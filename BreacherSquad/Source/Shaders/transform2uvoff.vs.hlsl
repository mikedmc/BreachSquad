float4x4 matViewProjection;
float4 texOff; // .xy - offset of UV0 channel, .zw - offset of UV1 channel

struct VS_INPUT
{
   float4 Position : POSITION0;
   float3 Normal : NORMAL;       //unused
   float4 Color : COLOR0;         
   float2 Tex0 : TEXCOORD0;      //RT coords for corners
   float3 Tex1 : TEXCOORD1; 	// same coords for corners
   
};

struct VS_OUTPUT 
{
   float4 Position :        POSITION0;
   float4 Color :           COLOR0;
   float2 Tex0 :            TEXCOORD0;
   float2 Tex1 :            TEXCOORD1;
};

VS_OUTPUT vs_main( VS_INPUT Input )
{
   VS_OUTPUT Output;

	//- mul(matrix,vector) expects column-major
	//- mul(vector,matrix) expects row-major
 
   Output.Position = mul(matViewProjection, Input.Position);
   Output.Color 		= Input.Color;
   Output.Tex0			= Input.Tex0.xy + texOff.xy;
   Output.Tex1			= Input.Tex1.xy + texOff.zw; 

   return( Output );
   
}
