//--- exemplu bun dar nu merge, adica e economic ca si memorie dar nu am reusit sa-l fac sa afiseze ceva... orice....

struct PS_INPUT
{
	float4 UV0:            TEXCOORD0; //tex coords
};

sampler _samp0: register(s0);

Texture2D <float4> _MainTex;		// prev cascade
Texture2D <float4> _EmissiveTex;
Texture2D <float4> _ColorTex;
Texture2D <float4> _DistanceTex;

float2 _Aspect;             // x = aspect.x, y = aspect.y (same semantics as Unity float2)
float _RayRange;

float2 _CascadeResolution;  // resolution used for cascades (width, height)
int _CascadeLevel;        // current cascade level (int)
int _CascadeCount;        // total number of cascades

float _SkyRadiance;
float3 _SkyColor;
float3 _SunColor;
float _SunAngle;

static const float TAU = 6.2831853071795864769252867665590; // double PI

// ----------------- Helpers -----------------

float2 CalculateRayRange( int index, int count )
{
	// replicate logic in original (bit shifts). returns [start, end] scaled by _RayRange
	int maxValue = pow( 2, (count * 2) ) - 1; // (1 << (count * 2)) - 1; - mai optim e cu shift, si mai exact
	int nstart = pow( 2, (index * 2) ) - 1;// (1 << (index * 2)) - 1;
	int nend = pow( 2, (index * 2 + 2) ) - 1; //(1 << (index * 2 + 2)) - 1;
	float2 r = float2( float( nstart ), float( nend ) ) / float( maxValue );
	return r * _RayRange;
}

float3 SampleSkyRadiance( float a0, float a1 )
{
	// Sky integral formula from "Analytic Direct Illumination"
	const float SSunS = 8.0;
	const float ISSunS = 1.0 / SSunS;

	float3 SI = _SkyColor * (a1 - a0 - 0.5 * (cos( a1 ) - cos( a0 )));
	SI += _SunColor * (atan( SSunS * (_SunAngle - a0) ) - atan( SSunS * (_SunAngle - a1) )) * ISSunS;
	return SI * 0.16;
}

// Ray-marching over SDF stored in _DistanceTex, color in _ColorTex
float4 SampleRadianceSDF( float2 rayOrigin, float2 rayDirection, float2 rayRange )
{
	float t = rayRange.x;
	float4 hit = float4( 0.0, 1.0, 0.0, 1.0 );

	for ( int i = 0; i < 32; ++i )
	{
		float2 currentPosition = rayOrigin + t * rayDirection * _Aspect.yx;

		// if outside range, break
		if ( t > rayRange.y || currentPosition.x < 0.0 || currentPosition.y < 0.0 || currentPosition.x > 1.0 || currentPosition.y > 1.0 )
		{
			break;
		}

		float fdist = _DistanceTex.SampleLevel( _samp0, currentPosition, 0 ).r;

		if ( fdist < 0.001 )
		{
			float3 emission = _EmissiveTex.SampleLevel(_samp0, currentPosition, 0).rgb;

			if ( length( emission ) > 0.0 ) {
				hit = float4( emission, 1.0 ); // <-- Treat emission as radiance
			}
			else {
				float3 baseColor = _ColorTex.SampleLevel(_samp0, currentPosition, 0).rgb;
				hit = float4( baseColor, 0.0 ); // <-- normal surface
			}
			break;
		}

		t += fdist;
	}

	return hit;
}

// ----------------- Main -----------------

// glsl mod is slightly different from fmod
float2 mod_glsl( float2 x, float2 y ) {
	return x - y * floor( x / y );
}

float4 ps_main(PS_INPUT pin) : SV_Target
{
	// pixel index in cascade grid
	float2 fragTexCoord = pin.UV0.xy;
	float2 pixelIndex = floor( fragTexCoord * _CascadeResolution );

	int blockSqrtCount = pow( 2, _CascadeLevel ); // 1 << _CascadeLevel
	float2 blockDim = _CascadeResolution / float( blockSqrtCount );
	float2 block2DIndex = floor( pixelIndex / blockDim );
	float blockIndexF = block2DIndex.x + block2DIndex.y * float( blockSqrtCount );
	int blockIndex = int( blockIndexF + 0.5 );

	float2 coordsInBlock = mod_glsl( pixelIndex, blockDim );

	float4 finalResult = float4( 0.0, 0.0, 0.0, 0.0 );

	// ray origin is in some normalized coordinates relative to cascade grid
	float2 rayOrigin = (coordsInBlock + 0.5) * float( blockSqrtCount );
	float2 rayRange = CalculateRayRange( _CascadeLevel, _CascadeCount );

	for ( int i = 0; i < 4; ++i )
	{
		float angleStep = TAU / float( blockSqrtCount * blockSqrtCount * 4 );
		int angleIndex = blockIndex * 4 + i;
		float angle = (float( angleIndex ) + 0.5) * angleStep;

		float2 rayDirection = float2( cos( angle ), sin( angle ) );

		float4 radiance = SampleRadianceSDF( rayOrigin / _CascadeResolution, rayDirection, rayRange );

		if ( radiance.a != 0.0 )
		{
			if ( _CascadeLevel != (_CascadeCount - 1) )
			{
				// Merging with the Upper Cascade (_MainTex)
				// position logic from original shader
				float2 position = coordsInBlock * 0.5 + 0.25;
				float blockSqrtCountTimes2 = float( blockSqrtCount * 2 );
				float positionOffsetX = mod_glsl( float( angleIndex ), blockSqrtCountTimes2 );
				float positionOffsetY = floor( float( angleIndex ) / blockSqrtCountTimes2 );

				// clamp position between 0.5 and blockDim*0.5 - 0.5 (original clamps scalars; replicate)
				float2 minPos = float2( 0.5, 0.5 );
				float2 maxPos = blockDim * 0.5 - float2( 0.5, 0.5 );
				position = clamp( position, minPos, maxPos );

				float2 positionOffset = float2( positionOffsetX, positionOffsetY );

				float2 samplePos = (position + positionOffset * (blockDim * 0.5)) / _CascadeResolution;
				float4 rad = _MainTex.SampleLevel(_samp0, samplePos, 0);

				radiance.rgb += rad.rgb * radiance.a;
				radiance.a *= rad.a;
			}
			/*
			else
			{
				// top cascade: merge with sky radiance
				float3 sky = SampleSkyRadiance( angle, angle + angleStep ) * _SkyRadiance;
				radiance.rgb += (sky / angleStep) * 2.0;
			}*/
		}

		finalResult += radiance * 0.25;
	}

	return finalResult;
}
