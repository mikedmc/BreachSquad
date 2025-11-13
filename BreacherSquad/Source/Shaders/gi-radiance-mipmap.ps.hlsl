uniform float in_MipMapExtent;     // Cascade MipMap Resolution.
uniform float in_CascadeExtent;    // Cascade Diagonal Resolution.
sampler2D in_CascadeAtlas : register(s0); // MipMap Source Cascade [N].

// Cascade index.
#define in_CascadeIndex  0
// Cascade angular resolution.
#define in_CascadeAngular  4

struct ProbeTexel {
	float count;
	float size;
	float probes;
};

ProbeTexel cascadeProbeTexel( float cascadeIndex ) {
	float cnt = in_CascadeAngular * pow( 4.0, cascadeIndex );
	float sze = sqrt( cnt );
	float probes = in_CascadeExtent / sze;
	
	ProbeTexel ptex;
	ptex.count = cnt;
	ptex.size = sze;
	ptex.probes = probes;
	return ptex;
}

// We fetch radiance intervals within the cascade by angle (thetaIndex) and probe (texelIndex).
float4 cascadeFetch( ProbeTexel info, float2 texelIndex, float thetaIndex ) {
	float2 probeTexel = texelIndex * info.size;
	probeTexel += float2( fmod( thetaIndex, info.size ), (thetaIndex / info.size) );
	float2 cascadeTexelPosition = probeTexel / in_CascadeExtent;
	return tex2D(in_CascadeAtlas, cascadeTexelPosition);
}

struct PS_INPUT
{
	float4 UV0:            TEXCOORD0; //tex coords
};

float4 ps_main( PS_INPUT pin ) : SV_Target
{
	// Get the mipmap's cascade texel info based on the cascade being rendered.
	ProbeTexel probeInfo = cascadeProbeTexel( in_CascadeIndex );
	float2 mipmapCoord = float2( pin.UV0.xy * in_MipMapExtent );

	// Loops through all of the radiance intervals for this mip-map and accumulate.
	float4 radiance = float4( 0.0, 0.0, 0.0, 0.0 );
	// SM3.0 supports loops, but count must be clamped to avoid dynamic loop limits
	//int maxCount = (int)min( probeInfo.count, 256 ); // DX9 hardware loop limit workaround

	for ( float i = 0.0; i < probeInfo.count; i++ ) {
		// cascadeFetch uses the probe's cell index, which is the same as the mipmap's pixel position.
		radiance += cascadeFetch( probeInfo, mipmapCoord, i );
	}
	float4 gl_FragColor = float4( radiance.rgb / probeInfo.count, 1.0 );
	return gl_FragColor;
}

