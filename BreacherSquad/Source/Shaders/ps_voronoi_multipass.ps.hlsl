// shader used to turn voronoi seed into complete voronoi map in the multipass voronoi sequence

float4 uoffset : register( c0 ); // .x contains the offset sent from the game code
float4 v2pixelsize: register( c1 ); // inverse of RT resolution on both x and y

Texture2D <float4> texIn;		// takes voronoi seed or last voronoi step
sampler samp0: register(s0);

struct PS_INPUT
{
	float4 Tex0:            TEXCOORD0; //tex coords
};

#define V2F16(v) ((v.y * float(0.0039215689)) + v.x)

float4 ps_main(PS_INPUT Input) : COLOR0
{	
	/*
	// tests data transport for float values
			float2 voffset = Input.Tex0.xy;
			float4 seed = texIn.SampleLevel( samp0, voffset, 0 );
			float2 fxy = float2(seed.x, seed.y);
			float2 fzw = float2(seed.z, seed.w);
			return float4(V2F16( fxy ), V2F16( fzw ), 0, 1.0);
			*/

	float closest_dist = 9999999.9;
	float4 closest_data = float4(0.0, 0.0, 0.0, 0.0); // keeps 2 floats encoded on 2 channels each (rg, ba)
	
	// uses Jump Flood Algorithm to do a fast voronoi generation.
	for(float x = -1.0; x <= 1.0; x += 1.0)
	{
		for(float y = -1.0; y <= 1.0; y += 1.0)
		{
			float2 voffset = Input.Tex0.xy;
			voffset += float2(x, y) * v2pixelsize.xy * uoffset.xy;

			float4 seed = texIn.SampleLevel(samp0, voffset, 0);
			// test just X on 2 vecs:
			//float2 seedpos = float2(V2F16( seed.xy ), seed.z );
			// full precision:
			//float2 seedpos = float2(V2F16( seed.xy ), V2F16( seed.zw ));
			// no precision:
			float2 seedpos = seed.xy;
			float dist = distance(seedpos, Input.Tex0.xy);
			
			if(seedpos.x != 0.0 && seedpos.y != 0.0 && dist <= closest_dist)
			{
				closest_dist = dist;
				closest_data = seed;
			}
		}
	}
	return closest_data;
}
