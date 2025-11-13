// just plain texturing multiplied by the vertex color
// one single texture level

sampler2D texColor : register(s0);  //texture

struct PS_INPUT
{
	float4 VertColor:       COLOR0;    //culoare lumina
	float2 Tex0:            TEXCOORD0; //tex RT normals_height
};

float4 ps_main( PS_INPUT Input ) : COLOR0
{
	float4 col = tex2D( texColor, Input.Tex0.xy );
	return col * Input.VertColor;
}
