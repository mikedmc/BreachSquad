float4x4 matViewProjection : register(c0);
// Render target rectangle
float4 RTTxywh : register(c4);	

struct VS_INPUT 
{
   float4 Position : POSITION0;		// world position of rectangle
   float3 Normal : NORMAL;			// empty
   float4 Color : COLOR0;         	// color
   float2 Tex0 : TEXCOORD0;      	// empty
   float2 Tex1 : TEXCOORD1; 		// empty
   
};

struct VS_OUTPUT 
{
   float4 Position :        POSITION0;
   float4 Color :           COLOR0;
   float2 Tex0 :            TEXCOORD0; // will be mapped to RT texture coords
   float2 Tex1 :            TEXCOORD1; // will contain world positions 
};

VS_OUTPUT vs_main( VS_INPUT Input )
{
   VS_OUTPUT Output;

   Output.Position		    = mul( Input.Position, matViewProjection );
   // Light color
   Output.Color				= Input.Color; 
   // RT coords for normals and height
   Output.Tex0.x			= ((Input.Position.x - RTTxywh.x) / RTTxywh.z);
   Output.Tex0.y			= ((Input.Position.y - RTTxywh.y) / RTTxywh.w);
   // world position
   Output.Tex1 = Input.Position;

   return( Output );
   
}
