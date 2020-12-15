float4x4 matViewProjection;

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

   Output.Position		= mul(Input.Position, matViewProjection);
   Output.Color 		= Input.Color;
   Output.Tex0			= Input.Tex0;
   Output.Tex1			= Input.Tex1; 

   return( Output );
   
}
