float2 in_MixPercent; // x: percent of T1, y: percent of T2

sampler2D texT1 : register(s0);  //texture
sampler2D texT2 : register(s1); //texture

struct PS_INPUT
{
	float4 VertColor:       COLOR0;    //culoare lumina
	float2 Tex0:            TEXCOORD0; //tex RT normals_height
};

float4 ps_main( PS_INPUT Input ) : COLOR0
{
	float4 col1 = tex2D( texT1, Input.Tex0.xy ) * in_MixPercent.x;
    float4 col2 = tex2D( texT2, Input.Tex0.xy) * in_MixPercent.y;
    return saturate(col1 * col2);
}
