float2 in_MixPercent; // x: percent of T1, y: percent of T2

sampler2D texT1 : register(s0); //texture
sampler2D texT2 : register(s1); //texture

struct PS_INPUT
{
    float4 VertColor : COLOR0; //culoare lumina
    float4 Tex0 : TEXCOORD0;
    float4 Tex1 : TEXCOORD1;
};

float4 ps_main(PS_INPUT Input) : COLOR0
{
    float3 col1 = tex2D(texT1, Input.Tex0.xy).rgb;
    float3 col2 = tex2D(texT2, Input.Tex1.xy).rgb;
    return saturate(float4(col1 * in_MixPercent.x + col2 * in_MixPercent.y, 1.0f));
}
