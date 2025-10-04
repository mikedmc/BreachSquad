float4x4 matViewProjection;
float4 RTTxywh;

struct VS_INPUT 
{
   float4 Position : POSITION0;
   float3 Normal : NORMAL;       //unused
   float4 Color : COLOR0;         
   float2 Tex0 : TEXCOORD0;      
   float3 Tex1 : TEXCOORD1; 
   
};

struct VS_OUTPUT 
{
   float4 Position :        POSITION0;
   float4 Color :           COLOR0;
   float2 Tex0 :            TEXCOORD0;
   float4 Tex1:             TEXCOORD1; // will contain world coords
};

VS_OUTPUT vs_main( VS_INPUT Input )
{
   VS_OUTPUT Output;

   Output.Position         = mul( Input.Position, matViewProjection );
   // Light color
   Output.Color = Input.Color; 
   // RT coords for normals and height
   //Output.Tex0 = Input.Tex0; 
   Output.Tex0.x = ((Input.Position.x - RTTxywh.x) / RTTxywh.z);
   Output.Tex0.y = ((Input.Position.y - RTTxywh.y) / RTTxywh.w);
   // world position
   Output.Tex1 = Input.Position; 

   return( Output );
   
}
