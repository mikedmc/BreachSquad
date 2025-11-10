// Voronoi seed 
sampler2D texIn : register(s0);  //render target texture

#define F16V2(f) float2(floor(f * 255.0) * float(0.0039215689), frac(f * 255.0))
#define V2F16(v) ((v.y * float(0.0039215689)) + v.x)

struct PS_INPUT
{
	float4 Tex0:            TEXCOORD0; //tex coords
};

float4 ps_main(PS_INPUT Input) : COLOR0
{
	// for the voronoi seed texture we just store the UV of the pixel if the pixel is part
	// of an object (emissive or occluding), or black otherwise.

	float4 scene_col = tex2D(texIn, Input.Tex0.xy);
	
	if(scene_col.a == 0.0)
	{
		return float4(0.0, 0.0, 0.0, 0.0);
	}
	return float4(Input.Tex0.xy, 0.0, 1.0); // DX9 it only works with alpha 1, use float textures...
	//test encoding just X
	//return float4(F16V2( Input.Tex0.x ), Input.Tex0.y, 1.0);
	// full precision
	//return float4(F16V2(Input.Tex0.x), F16V2(Input.Tex0.y));
}
