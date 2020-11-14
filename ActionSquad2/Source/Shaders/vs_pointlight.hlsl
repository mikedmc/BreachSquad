float4x4 matViewProjection;

struct VS_INPUT
{
	float4 Position : POSITION0;
	float3 Normal : NORMAL;       //unused
	float4 Color : COLOR0;
	float2 Tex0 : TEXCOORD0;      //RT coords for normals_height
	float3 Tex1 : TEXCOORD1;      //world position

};

struct VS_OUTPUT
{
	float4 Position :        POSITION0;
	float4 Color :           COLOR0;
	float2 Tex0 :            TEXCOORD0;
	float4 Tex1:             TEXCOORD1; // world coords
};

VS_OUTPUT vs_main(VS_INPUT Input)
{
	VS_OUTPUT Output;

	Output.Position = mul(Input.Position, matViewProjection);
	Output.Color = Input.Color; //culoarea luminii din C++
	Output.Tex0 = Input.Tex0; //RT coords for normals and height
	Output.Tex1 = Input.Position; // world position

	return(Output);

}
