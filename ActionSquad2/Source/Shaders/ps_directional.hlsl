// Directional light, no shadows

 //x: intensity
float4 fLightData : register(c0);
// xyz: light normalized direction
float4 vLightDir: register(c1);   

sampler2D texNrmH : register(s0);  //render target texture (normals and height)

struct PS_INPUT
{
	float4 VertColor:       COLOR0;    //culoare lumina
	float2 Tex0:            TEXCOORD0; //tex RT normals_height
	float4 Tex1:            TEXCOORD1; //world vertex position
};

float4 ps_main(PS_INPUT Input) : COLOR0
{
	//XY normals, Z is height in world space. W is alpha
	float4 normal_h = tex2D(texNrmH, Input.Tex0.xy);
	// convert normal XY in -1..1 (input normal is already normalized)
	normal_h = (normal_h * 2.0f) - 1.0f;
	// build normal Z
	float nrmZ = sqrt(1.0f - normal_h.x * normal_h.x - normal_h.y * normal_h.y);
	// compose final normal vector, normalized
	float3 normalN = float3(normal_h.xy, nrmZ);
	float dotN = dot(vLightDir.xyz, normalN);
	dotN = clamp(dotN, 0.0f, 1.0f);

	float4 fvFinalColor = fLightData.x * (Input.VertColor * dotN);
	
	return(fvFinalColor);
}
