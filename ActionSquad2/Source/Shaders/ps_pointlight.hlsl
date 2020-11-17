// distance attenuation formula: 1.0/(1.0 + c1*d + c2*d^2)
float4 fLightData : register(c0); //x-atten c1, y-atten c2, z-light radius
float4 fWorldConstants : register(c1); //x-mul to convert height in Y offset (2D projection)
float3 vLightPosWorld : register(c2);   // world coords light position

sampler2D texNrmH : register(s0);  //textura render target (ul-diffuse, ur-normal+height, dl-specular power?)
//sampler2D texSpot : register(s1);

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
	// bring height from 0..1 to 0..255
	float3 posWorld = float3(Input.Tex1.xy, normal_h.z * 255.0f);
	// inversez proiectia (cresc Y in fn de height-ul citit)
	//TODO: constanta 0.5 trebuie trimisa din cod ca proiectie: Y -= Z * 0.5 e in C++
	posWorld.y += posWorld.z * fWorldConstants.x;

	// convert normal XY in -1..1 (input normal is already normalized)
	normal_h = (normal_h * 2.0f) - 1.0f;
	// build normal Z
	float nrmZ = sqrt(1.0f - normal_h.x * normal_h.x - normal_h.y * normal_h.y);
	// compose final normal vector, normalized
	float3 normalN = float3(normal_h.xy, nrmZ);

	// direction pixel->light
	float3 lightRay = vLightPosWorld - posWorld;
	float lightDist = length(lightRay);
	// normalized light vector
	float3 lightRayN = lightRay / lightDist;
	float dotN = dot(lightRayN, normalN);
	dotN = clamp(dotN, 0.0f, 1.0f);
	// scales light distance to light radius because attenuation is based on light radius
	float attenDist = lightDist / fLightData.z;
	// distance attenuation
	//float distAtten = 1.0f / (1.0f + fLightData.x * attenDist + fLightData.y * attenDist * attenDist);

	// attenuation based on distance, moved down so it approaces 0
	//float distAtten = (1.0f / (1.0f + fLightData.y * attenDist * attenDist) - 0.1) * 1.111;

	// attenuation based on gauss bell (fLightData.x is the gauss bell base)
	float distAtten = exp(-((attenDist * attenDist) / (2.0f * fLightData.x * fLightData.x)));

	// original:
	float4 fvFinalColor = distAtten * (Input.VertColor * dotN);
	// good effect if we ignore dotN and just use dist atten
	//float4 fvFinalColor = distAtten * (Input.VertColor);

	return(fvFinalColor);
}
