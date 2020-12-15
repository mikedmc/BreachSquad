float4 fCompData : register(c0); //x:gamma, y:1/gamma, z:final light multiplier, w:color dodge

sampler2D texColor : register(s0);  //color RT texture (diffuse color)
sampler2D texLights : register(s1);  //lightmap RT

struct PS_INPUT
{
	float4 VertColor:       COLOR0;    //culoare lumina
	float2 Tex0:            TEXCOORD0; //tex RT normals_height
	float2 Tex1:            TEXCOORD1; //world vertex position
};

float4 ps_main(PS_INPUT Input) : COLOR0
{
	float3 vCol = tex2D(texColor, Input.Tex0.xy).rgb;
	float3 vLight = tex2D(texLights, Input.Tex0.xy).rgb;
	// gamma correct light
	vLight = pow(vLight, fCompData.yyy);
	
	float3 fvFinal = vLight * vCol * fCompData.z;
	// color dodge: Composite = Background / (1 - foreground * effect_alpha)
	fvFinal /= (1.0f - vLight * fCompData.w); //0.4f default
	// linear dodge for light volume (try it!)
	//fvFinal += vLight.rgb * 0.2f;

	return float4(fvFinal, 1.0f);
}
