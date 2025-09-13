// shader used to turn voronoi seed into complete voronoi map in the multipass voronoi sequence

float4 uoffset : register( c0 ); // .x contains the offset sent from the game code
float4 v2pixelsize: register( c1 ); // inverse of RT resolution on both x and y

sampler2D texIn : register(s0);  //input voronoi last pass RT

struct PS_INPUT
{
	float4 Tex0:            TEXCOORD0; //tex coords
};

float4 ps_main(PS_INPUT Input) : COLOR0
{
	float closest_dist = 9999999.9;
	float2 closest_pos = float2(0.0, 0.0);
	
	// uses Jump Flood Algorithm to do a fast voronoi generation.
	for(float x = -1.0; x <= 1.0; x += 1.0)
	{
		for(float y = -1.0; y <= 1.0; y += 1.0)
		{
			float2 voffset = Input.Tex0.xy;
			voffset += float2(x, y) * v2pixelsize.xy * uoffset.x;

			float2 pos = tex2D(texIn, voffset).xy;
			float dist = distance(pos.xy, Input.Tex0.xy);
			
			if(pos.x != 0.0 && pos.y != 0.0 && dist < closest_dist)
			{
				closest_dist = dist;
				closest_pos = pos;
			}
		}
	}
	return float4(closest_pos, 0.0, 1.0);
}
