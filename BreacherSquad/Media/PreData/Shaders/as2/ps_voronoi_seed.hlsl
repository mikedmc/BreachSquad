// Voronoi seed 

sampler2D texIn : register(s0);  //render target texture

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
		return float4(0.0, 0.0, 0.0, 1.0);
	}
	return float4(Input.Tex0.x, Input.Tex0.y, 0.0, 1.0);
}
