float4 fLightData : register(c0); //x:light intensity, y:light radius
//x: mul to convert H in Y offset (inverse 2D projection, world->screen), y: inv Z to H projection (screen -> world), z: light atten c1 gauss (usually 0.55)
float4 fWorldConstants : register(c1); 
// world coords light position
float3 vLightPosWorld : register(c2);   

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
	// bring height from 0..1 to -128..128 (has negative values too, relative to level floor)
	float nrmTexH = normal_h.z * 255.0f - 128.0f;
	float3 posWorld = float3(Input.Tex1.x, Input.Tex1.y + nrmTexH, nrmTexH * fWorldConstants.y);
	// invert projection (texture height gradient)
	//posWorld.y += normal_h.z * 4.0f;// fWorldConstants.y;

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
	float attenDist = lightDist / fLightData.y;

	// distance attenuation	 (not used, never gets down to 0, light clipping becomes visible on deferred shading making the light rectangle fullscreen)
	//float distAtten = 1.0f / (1.0f + fLightData.x * attenDist + fLightData.y * attenDist * attenDist);
	// attenuation based on distance, moved down so it approaces 0
	//float distAtten = (1.0f / (1.0f + fLightData.y * attenDist * attenDist) - 0.1) * 1.111;

	// attenuation based on gauss bell (fWorldConstants.z is the gauss bell base)
	float distAtten = exp(-((attenDist * attenDist) / (2.0f * fWorldConstants.z * fWorldConstants.z)));

	// original:		 //lg intensity
	float4 fvFinalColor = fLightData.x * distAtten * (Input.VertColor * dotN);
	
	// good smnooth effect if we ignore dotN and just use dist atten (gives illumination on other side too)
	//float4 fvFinalColor = distAtten * (Input.VertColor);

	return(fvFinalColor);
}
