struct PS_INPUT
{
	float4 UV0:            TEXCOORD0; //tex coords
};
sampler samp0: register(s0);

float     in_RenderExtent;  // Scren Space Resolution.
Texture2D <float4> in_DistanceField; // World Input Distance Field.
Texture2D <float4> in_WorldScene;    // World Input Raymarch Scene.
float in_RenderDecayRate;   // How quickly 

float in_CascadeExtent;   // Cascade Diagonal Resolution.
float in_CascadeSpacing;  // Cascade 0 probe spacing.
float in_CascadeInterval; // Cascade 0 radiance interval.
float in_CascadeAngular;  // Cascade angular resolution.
float in_CascadeIndex;    // Cascade index.

#define EPSILON       0.0001
#define TAU           6.283185
#define V2F16(v) ((v.y * float(0.0039215689)) + v.x)

struct ProbeTexel {
	float count;   // number of ray-directions in this probe.
	float2 probe;   // the cell index of this probe.
	float2 spacing; // spacing between radiance probes.
	float index;   // the theta-index of this texel in it's probe.
	float minimum; // minimum interval-range.
	float maximum; // maximum interval-range.
	float range;   // maximum - minimum intervals.
	float texel;   // texel size: 1.0 / cascadeExtent;
	//float2 position; // cascade texel probe position.
};

float2 mod_glsl( float2 x, float2 y ) {
	return x - y * floor( x / y );
}

ProbeTexel cascadeProbeTexel( float2 coord, float cascade ) {
	float count = in_CascadeAngular * pow( 4.0, cascade );
	float size = sqrt( count );
	float2 probe = floor( coord / size );
	float2 spacing = in_CascadeSpacing * pow( 2.0, cascade );

	float2  probePos = mod_glsl( floor( coord ), float2( size, size ) );
	float index = (probePos.y * size) + probePos.x;

	// Quadruples the Interval Range: (per specification, but not as smooth)
	float minimum = (in_CascadeInterval  * (1.0 - pow( 4.0, cascade ))) / (1.0 - 4.0);
	float range = in_CascadeInterval * pow( 4.0, cascade );
	float maximum = minimum + range;

	// Quadruples the Interval Range End-Points: (typical implementation)
	//float minimum = in_CascadeInterval * pow(4.0, cascade - 1.0) * sign(cascade);
	//float maximum = in_CascadeInterval * pow(4.0, cascade);
	//float range = maximum - minimum;

	float texel = 1.0 / in_RenderExtent;

	//return ProbeTexel( count, probe, spacing, index, minimum, maximum, range, texel/*, probePos / float2(size)*/ );
	ProbeTexel rettex;
	rettex.count = count;
	rettex.probe = probe;
	rettex.spacing = spacing;
	rettex.index = index;
	rettex.minimum = minimum;
	rettex.maximum = maximum;
	rettex.range = range;
	rettex.texel = texel;
	//rettex.position = probePos / float2(size, size);
	return rettex;
}

float4 marchInterval( ProbeTexel probeInfo ) {
	float2 probe = float2( (probeInfo.probe + 0.5) * probeInfo.spacing );
	probe *= probeInfo.texel;

	float theta = TAU * ((probeInfo.index + 0.5) / probeInfo.count);
	float2 delta = float2( cos( theta ), -sin( theta ) );
	float2 interval = probe + ((delta * probeInfo.minimum) * probeInfo.texel);

	//
	// Ray Visibility Term: The A (Alpha Component) returns the transparency of this ray.
	//	* A visibility term of 0.0 means this ray is fully opaque (object hit).
	//	* A visibility term of 1.0 means this ray is transparent (no hit).
	//		When merging cascade rays with NO hits (1.0 visibility term) are the only
	//		rays which will merge with above cascades. This applies merging/smoothing
	//		of rays between cascade ranges to create those smooth shadows.
	//
	//	Interval Raymarching (raymarches a specific range away from probe):
	//
	float decay = min( max( 0.0, in_RenderDecayRate ), 1.0 );
	for ( float ii = 0.0, dd = 0.0, rd = 0.0, rt = probeInfo.range * probeInfo.texel; ii < probeInfo.range; ii++ ) {
		float2 ray = interval + delta * min( rd, rt );
		float4 texread = in_DistanceField.SampleLevel( samp0, ray, 0 );
		rd += dd = V2F16( texread.rg );

		// End of Interval Range or Out of Bounds:
		if ( rd >= rt || ray.x < 0.0 || ray.y < 0.0 || ray.x >= 1.0 || ray.y >= 1.0 ) return float4( 0.0, 0.0, 0.0, 0.0 );

		// Surface/Object collision:
		//if (dd < EPSILON) return max(float4(texture2D(in_WorldScene, ray).rgb, 1.0), float4(texture2D(in_WorldScene, ray - (delta * probeInfo.texel)).rgb, 1.0) * decay);
		if ( dd < EPSILON ) return float4( in_WorldScene.SampleLevel(samp0, ray, 0).rgb, 1.0 );
	}

	return float4( 0.0, 0.0, 0.0, 0.0 );
}

float4 ps_main( PS_INPUT pin ) : SV_Target
{
	float2 texel = pin.UV0.xy * float2( in_CascadeExtent, in_CascadeExtent );
	ProbeTexel probeInfo = cascadeProbeTexel( texel, in_CascadeIndex );
	float4 gl_FragColor = marchInterval( probeInfo );
	return gl_FragColor;
}

//
// Cascade Radiance:
//		gl_FragColor = float4(intervalRayMarch(probeInfo).rgb, 1.0);
//
// Cascade Prob Texel Space:
//		gl_FragColor = float4(probeInfo.position, 0.0, 1.0);
//
