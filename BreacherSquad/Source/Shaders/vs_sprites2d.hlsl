float4x4 matProj;

//#ABOUT:
// transforms input with scale and rotation, and moves it to pos xy

struct VS_INPUT
{
	float4 Position : POSITION0;
	float3 Normal : NORMAL;			// position to add to mesh
	float4 Color : COLOR0;
	float2 Tex0 : TEXCOORD0;      	// tex coords
	float3 Tex1 : TEXCOORD1;		// x: scale x, y: scale y, z: rotation
};

struct VS_OUTPUT
{
	float4 Position :        POSITION0;
	float4 Color :           COLOR0;
	float2 Tex0 :            TEXCOORD0;
};

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output;

	float3 scale = float3(Input.Tex1.xy, 1.0f);
	float rotateZ = Input.Tex1.z;
	float3 translation = Input.Normal;
	// create sprite transform matrix
			
	float4x4 matScaleRotPos = float4x4(
		float4(scale.x * cos(rotateZ), scale.x * -sin(rotateZ), 0.0, 0.0),
		float4(scale.y * sin(rotateZ), scale.y *  cos(rotateZ), 0.0, 0.0),
		float4(0.0, 0.0, scale.z, 0.0),
		float4(translation.xyz, 1.0)
		);

	// multiply transform matrix with projection matrix
	float4x4 matWVP = mul(matScaleRotPos, matProj);
	// transform vertex
	Output.Position = mul(Input.Position, matWVP);
	//Output.Position = mul(Input.Position, matProj);
	Output.Color = Input.Color;
	Output.Tex0 = Input.Tex0;

	return(Output);

}
