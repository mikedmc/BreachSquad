// Directional shadowing light

 //x: intensity, y: height threshold, z: check vec length
float4 fLightData : register(c0);
//x: mul to convert H in Y offset (inverse 2D projection, world->screen), y: inv Z to H projection (screen -> world), z: light atten c1 gauss (usually 0.55)
//float4 fWorldConstants : register(c1); 
// 2d projected vector (xy) for the texture check (must be scaled to -1..1, relative to pixel position, size of a tile or similar
float4 vLightDirCheck : register(c1);   

sampler2D texNrmH : register(s0);  //textura render target (ul-diffuse, ur-normal+height, dl-specular power?)

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
	/*
	// bring height from 0..1 to 0..255
	float nrmTexH = normal_h.z * 255.0f;
	float3 posWorld = float3(Input.Tex1.x, Input.Tex1.y + nrmTexH, nrmTexH * fWorldConstants.y);
	// invert projection (texture height gradient)
	//posWorld.y += normal_h.z * 4.0f;// fWorldConstants.y;

	// convert normal XY in -1..1 (input normal is already normalized)
	normal_h = (normal_h * 2.0f) - 1.0f;
	// build normal Z
	float nrmZ = sqrt(1.0f - normal_h.x * normal_h.x - normal_h.y * normal_h.y);
	// compose final normal vector, normalized
	float3 normalN = float3(normal_h.xy, nrmZ);
	*/
	float pxHeight = normal_h.z;
	float2 vCheckUV = Input.Tex0.xy + vLightDirCheck.xy * fLightData.z;
	float4 check_normal_h = tex2D(texNrmH, vCheckUV);
	float checkHeight = check_normal_h.z;

	// TODO: should multiply by normal....

	// if check position has bigger height return 0 else return 1 (adds small threshold so it doesn't get shadow from same height
	// TODO: gradients at the end of the checked distance
	float diff = clamp((pxHeight + fLightData.y - checkHeight) * 100.0f, 0.0f, 1.0f);

	float4 fvFinalColor = fLightData.x * Input.VertColor * diff;
	
	return(fvFinalColor);
}
