Texture2D InputTexture : register(t0);
SamplerState SamplerLinear : register(s0);

// Uniform parameter for the reciprocal of the input texture width
cbuffer Parameters : register(b0)
{
    float TexelWidth; // 1.0 / InputTexture.Width
};


// Standard 9-tap Gaussian Weights (approximated, normalized sum should be ~1.0)
// This wider kernel provides a softer blur.
static const float weights[9] =
{
    0.00443, // Tap -8
    0.02100, // Tap -6
    0.08375, // Tap -4
    0.20788, // Tap -2
    0.36588, // Tap 0 (Center)
    0.20788, // Tap +2
    0.08375, // Tap +4
    0.02100, // Tap +6
    0.00443 // Tap +8
};

// Offsets are measured in terms of input texels. 
// For a 2x downscale, we sample at even pixel steps: -8, -6, -4, -2, 0, 2, 4, 6, 8.
/*
static const float offsets[9] =
{
    -8.0,
    -6.0,
    -4.0,
    -2.0,
     0.0,
     2.0,
     4.0,
     6.0,
     8.0
};
*/
// or between pixels so we take advantage of linear interpolation
static const float offsets[9] =
{
    -8.0,
    -6.0,
    -4.0,
    -2.0,
     0.0,
     2.0,
     4.0,
     6.0,
     8.0
};


 /*
// Standard 5-tap Gaussian Weights (Normalized sum should be ~1.0)
// These weights represent the contribution of each sample to the final pixel color.
static const float weights[5] =
{
    0.06136, // Tap -4
    0.24477, // Tap -2
    0.38774, // Tap 0 (Center)
    0.24477, // Tap +2
    0.06136  // Tap +4
};

// Offsets are measured in terms of input texels. 
// We use steps of 2 pixels to effectively sample a wider area, thus achieving the 2x downscale.
static const float offsets[5] = 
{
    -5.0, 
    -3.0, 
     0.0, 
     3.0, 
     5.0
};
   */

float4 ps_main(float4 position : SV_POSITION, float2 texCoord : TEXCOORD) : SV_TARGET
{
    float4 finalColor = float4(0.0, 0.0, 0.0, 0.0);
    float2 currentTexCoord = texCoord;

    // A low-resolution output pixel corresponds to a 2x2 area in the input texture.
    // By sampling at 2-pixel intervals (e.g., -4, -2, 0, 2, 4) relative to the 
    // center of the output pixel's UV coordinate, we cover the required spatial 
    // extent in the high-resolution texture.

    for (int i = 0; i < 9; i++)
    {
        // Calculate the horizontal offset in UV space (0.0 to 1.0)
        float offset = offsets[i] * TexelWidth;

        // Sample the input texture at the calculated coordinate
        float2 sampleUV = float2(currentTexCoord.x + offset, currentTexCoord.y);
        
        // Accumulate the weighted color
        finalColor += InputTexture.Sample(SamplerLinear, sampleUV) * weights[i];
    }

    return finalColor;
}




/*
// Standard 9-tap Gaussian Weights (approximated, normalized sum should be ~1.0)
// This wider kernel provides a softer blur.
static const float weights[9] =
{
    0.00443, // Tap -8
    0.02100, // Tap -6
    0.08375, // Tap -4
    0.20788, // Tap -2
    0.36588, // Tap 0 (Center)
    0.20788, // Tap +2
    0.08375, // Tap +4
    0.02100, // Tap +6
    0.00443  // Tap +8
};

// Offsets are measured in terms of input texels. 
// For a 2x downscale, we sample at even pixel steps: -8, -6, -4, -2, 0, 2, 4, 6, 8.
static const float offsets[9] = 
{
    -8.0, 
    -6.0, 
    -4.0, 
    -2.0, 
     0.0, 
     2.0, 
     4.0,
     6.0,
     8.0
};
*/