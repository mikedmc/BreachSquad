// Composition shader that takes GI mipmap mapped on Tex1 input

float4 fCompData : register(c0); //x:gamma, y:1/gamma, z:final light multiplier, w:color dodge
float4 fGIData : register(c1); //x:GI multiplier, y: 1/gamma

sampler2D texColor : register(s0);  //color RT texture (diffuse color)
sampler2D texLights : register(s1);  //lightmap RT
sampler2D texGI : register(s2);  //GI mipmap

struct PS_INPUT
{
	float4 VertColor:       COLOR0;    //culoare lumina
	float2 Tex0:            TEXCOORD0; //tex RT normals_height
	float2 Tex1:            TEXCOORD1; //world vertex position
};

float3 lin_to_srgb(float3 color)
{
   float3 x = color.rgb * 12.92;
   float3 y = 1.055 * pow(clamp(color.rgb, 0.0, 1.0), float3(0.4166667, 0.4166667, 0.4166667)) - 0.055;
   float3 clr = color.rgb;
   clr.r = (color.r < 0.0031308) ? x.r : y.r;
   clr.g = (color.g < 0.0031308) ? x.g : y.g;
   clr.b = (color.b < 0.0031308) ? x.b : y.b;
   return clr.rgb;
}

float4 ps_main(PS_INPUT Input) : COLOR0
{
	float3 vCol = tex2D(texColor, Input.Tex0.xy).rgb;
	float3 vLight = tex2D(texLights, Input.Tex0.xy).rgb;
    float3 vGI = tex2D(texGI, Input.Tex1.xy).rgb;
	//older: gamma correct light (fast alternative, not perfect)
	vLight = pow(vLight, fCompData.yyy);
	// try this slower but better version (looks a little too bright)
	//vLight = lin_to_srgb(vLight);
	
	// gamma correct GI - not useful but makes it more visible
    vGI = lin_to_srgb(vGI);
    //vGI = pow(vGI, fGIData.yyy);
	
    float3 f_total_light = saturate(vLight * fCompData.z + vGI * fGIData.x);
    float3 fvFinal = f_total_light * vCol;
	// color dodge: Composite = Background / (1 - foreground * effect_alpha)
	fvFinal /= (1.0f - vLight * fCompData.w); //0.4f default
	// linear dodge for light volume (try it!)
	//fvFinal += vLight.rgb * 0.2f;

	return float4(fvFinal, 1.0f);
}
