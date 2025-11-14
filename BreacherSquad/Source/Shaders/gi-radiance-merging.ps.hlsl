struct PS_INPUT
{
	float4 UV0:            TEXCOORD0; //tex coords
};

sampler samp0: register(s0);
float in_CascadeExtent;   // Cascade Diagonal Resolution.
float in_CascadeAngular;  // Cascade angular resolution.
float in_CascadeCount;     // Total number of used cascades.
float in_CascadeIndex;    // Cascade index.
Texture2D <float4> gm_BaseTexture; // base sampler 0 texture (set from game maker with uniform_tx)
Texture2D <float4> in_CascadeAtlas; // Cascade Upper (N+1).

struct ProbeTexel {
	float count;
	float size;
	float index;
	float2 texel;
	float2 probe;
};

float mod_glsl_f1( float x, float y ) {
	return x - y * floor( x / y );
}


float2 mod_glsl_f2( float2 x, float2 y ) {
	return x - y * floor( x / y );
}

ProbeTexel cascadeProbeTexel( float2 coord, float cascade ) {
	float count = in_CascadeAngular * pow( 4.0, cascade );
	float size = sqrt( count );
	float2 texel = fmod( floor( coord ), float2( size, size ) );
	float index = (texel.y * size) + texel.x;
	float2 probe = floor( coord / float2( size, size ) );
	
	ProbeTexel texo;
	texo.count = count;
	texo.size = size;
	texo.index = index;
	texo.texel = texel;
	texo.probe = probe;
	return texo;
}

float4 cascadeFetch( ProbeTexel info, float2 texelIndex, float thetaIndex ) {
	float2 probeTexel = texelIndex * info.size;
	probeTexel += float2( fmod( thetaIndex, info.size ), thetaIndex / info.size );
	float2 cascadeTexelPosition = probeTexel / in_CascadeExtent;

	if ( cascadeTexelPosition.x < 0.0 || cascadeTexelPosition.y < 0.0 || cascadeTexelPosition.x >= 1.0 || cascadeTexelPosition.y >= 1.0 )
		return float4( 0.0, 0.0, 0.0, 0.0 );

	return in_CascadeAtlas.Sample( samp0, cascadeTexelPosition );
}

float4 ps_main( PS_INPUT pin ) : SV_Target 
{
	float2 cascadeCoord = pin.UV0.xy * in_CascadeExtent;
	ProbeTexel probeInfo = cascadeProbeTexel( cascadeCoord, in_CascadeIndex );
	ProbeTexel probeInfoN1 = cascadeProbeTexel( cascadeCoord, in_CascadeIndex + 1.0 );

	float2 texelIndexN1 = floor( (float2( probeInfo.probe ) - 1.0) / 2.0 );
	float2 texelIndexN1_N = floor( (texelIndexN1 * 2.0) + 1.0 );

	float4 radiance = gm_BaseTexture.Sample(samp0, pin.UV0.xy);
	radiance.a = 1.0 - radiance.a;

	if ( radiance.a != 0.0 && in_CascadeIndex < in_CascadeCount - 1.0 ) {
		float4 TL = float4( 0.0, 0.0, 0.0, 0.0 ), TR = float4( 0.0, 0.0, 0.0, 0.0 ),
			BL = float4( 0.0, 0.0, 0.0, 0.0 ), BR = float4( 0.0, 0.0, 0.0, 0.0 );

		// We always default to a 4x ray branch scaling between cascades.
		const float branch4 = 4.0;
		for ( float i = 0.0; i < branch4; i++ ) {
			float thetaIndexN1 = (probeInfo.index * branch4) + i;
			TL += cascadeFetch( probeInfoN1, texelIndexN1 + float2( 0.0, 0.0 ), thetaIndexN1 );
			TR += cascadeFetch( probeInfoN1, texelIndexN1 + float2( 1.0, 0.0 ), thetaIndexN1 );
			BL += cascadeFetch( probeInfoN1, texelIndexN1 + float2( 0.0, 1.0 ), thetaIndexN1 );
			BR += cascadeFetch( probeInfoN1, texelIndexN1 + float2( 1.0, 1.0 ), thetaIndexN1 );
		}

		// Per Specification:
		//float2 weight = float2(0.25) + (float2(probeInfo.probe) - texelIndexN1_N) * float2(0.5);

		// Smoother Weights:
		float2 weight = float2( 0.33, 0.33 ) + (float2( probeInfo.probe) - texelIndexN1_N) * float2( 0.33, 0.33 );

		float4 interpolated = lerp( lerp( TL, TR, weight.x ), lerp( BL, BR, weight.x ), weight.y ) / branch4;
		interpolated.a = 1.0 - interpolated.a;
		radiance += radiance.a * interpolated;
	}

	float4 gl_FragColor = float4( radiance.rgb, 1.0 );
	return gl_FragColor;
}
