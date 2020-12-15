//#TODO: Light would need UP and SIDE vectors to compute the projections correctly and to allow for rotations. Now it just projects on the X and Y axes, faster but uglier.

//x: light intensity, y: geometry half size W, z: geometry half size H
float4 fLightData : register(c0); 
//x: mul to convert H in Y offset (inverse 2D projection, world->screen), y: inv Z to H projection (screen -> world), z: light atten c1 gauss (usually 0.55)
float4 fWorldConstants : register(c1); 
// world coords light position
float3 vLightPosWorld : register(c2);
// xyz: direction of light, normalized
float3 vnLightDir : register(c3);
// xy: UV top left spot coords; zw: WH in texture coords
float4 bboxSpotCoords: register(c4);	    

sampler2D texNrmH : register(s0);   //normal and height texture
sampler2D texSpot : register(s1);	// spotlight texture

struct PS_INPUT
{
	float4 VertColor:       COLOR0;    // light color
	float2 Tex0:            TEXCOORD0; // tex RT normals_height
	float2 Tex1:            TEXCOORD1; // tex spot color
};

float4 ps_main(PS_INPUT Input) : COLOR0
{
	//XY normals, Z is height in world space. W is alpha
	float4 normal_h = tex2D(texNrmH, Input.Tex0.xy);
	// bring height from 0..1 to -128..128 (has negative values too, relative to level floor)
	float nrmTexH = normal_h.z * 255.0f - 128.0f;
	float3 posWorld = float3(Input.Tex1.x, Input.Tex1.y + nrmTexH, nrmTexH * fWorldConstants.y);

	// direction light->pixel
	float3 lightRay = posWorld - vLightPosWorld;
	// find projection on light axis
	float posdot = dot(lightRay, vnLightDir);
	float3 projpt = vLightPosWorld + vnLightDir * posdot;
	// vector from projection on axis to point
	lightRay = posWorld - projpt;
	
	// normalize to geometrysize
	lightRay.xy /= fLightData.yz;
	float2 spotCoords = (clamp(lightRay.xy, -1.0f, 1.0f) + 1.0f) / 2.0f;
	// use projection on axis as texture coordinates (distance from light direction)
	spotCoords = bboxSpotCoords.xy + spotCoords * bboxSpotCoords.zw;
	float4 spotColor = tex2D(texSpot, spotCoords);

	float4 fvFinalColor = fLightData.x * spotColor * Input.VertColor;

	return(fvFinalColor);
}
