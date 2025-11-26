// Radiance Merging Pixel Shader
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