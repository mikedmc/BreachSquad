float2 in_MixPercent; // x: percent of T1, y: percent of T2
float2 in_TexelSize; //x: texel T1, y:texel size T2

// Radiance Merging Pixel Shader
Texture2D fullResTex : register(t0);
Texture2D halfResTex : register(t1);

SamplerState samp : register(s0);

float4 ps_main(float2 uv : TEXCOORD) : SV_Target
{
    // Sample from different downscales
    float4 fullRes = fullResTex.SampleLevel(samp, uv, 0);

    float4 spreadcol = float4(0, 0, 0, 1.0f);
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float4 halfRes = halfResTex.SampleLevel(samp, uv + float2(x * in_TexelSize.y, y * in_TexelSize.y), 0);
            float dist = length(float2(x, y));
            spreadcol += halfRes * exp(-dist); // exponential falloff
        }
    }
    // Merge radiance
    float4 merged = float4(in_MixPercent.x * fullRes.rgb + in_MixPercent.y * (spreadcol.rgb / 9.0f), 1.0f);
    return saturate(merged);
    
    // Optional: apply tone mapping or gamma correction
    //merged = pow(merged, 1.0 / 2.2);

    //return float4(merged, 1.0);
    }



/*
// Radiance Merging Pixel Shader - 3 textures version
Texture2D fullResTex : register(t0);
Texture2D halfResTex : register(t1);
Texture2D quarterResTex : register(t2);

SamplerState samp : register(s0);

float4 ps_main(float2 uv : TEXCOORD) : SV_Target
{
    // Sample from different downscales
    float3 fullRes = fullResTex.SampleLevel(samp, uv, 0).rgb;
    float3 halfRes = halfResTex.SampleLevel(samp, uv, 0).rgb;
    float3 quarterRes = quarterResTex.SampleLevel(samp, uv, 0).rgb;

    // Weighting factors (tweakable)
    float w0 = 0.5; // full-res detail
    float w1 = 0.3; // mid-scale glow
    float w2 = 0.2; // large-scale radiance

    // Merge radiance
    float3 merged = w0 * fullRes + w1 * halfRes + w2 * quarterRes;

    // Optional: apply tone mapping or gamma correction
    //merged = pow(merged, 1.0 / 2.2);

    return float4(merged, 1.0);
}
*/