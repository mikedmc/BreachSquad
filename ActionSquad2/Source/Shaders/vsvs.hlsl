float4x4 matViewProjection;
float4 RTTxywh;
float4 RTTtex_xywh;

struct VS_INPUT 
{
   float4 Position : POSITION0;
   float3 Normal : NORMAL;       //vector catre lumina  
   float4 Color : COLOR0;
   float2 Tex0 : TEXCOORD0;      //textura spotului
   float2 Tex1 : TEXCOORD1;      //textura din RTarget
   
};

struct VS_OUTPUT 
{
   float4 Position :        POSITION0;
   float4 Color :           COLOR0;
   float2 Tex0 :            TEXCOORD0;
   float2 Tex1:             TEXCOORD1;
   float3 dirToLight :      TEXCOORD2;
};

VS_OUTPUT vs_main( VS_INPUT Input )
{
   VS_OUTPUT Output;

   Output.Position         = mul( Input.Position, matViewProjection );
   Output.dirToLight       = Input.Normal;

   Output.Color = Input.Color; //light color
   Output.Tex0 = Input.Tex0; //spotlight texture (shape of the light)
   //compute tex coords in Render Target depending on vert coord. 
   //RT is a 1024x1024 texture divided in 4 quadrants (top left - color, top right - normals)
   float2 RTTxy, RTTwh;
   float2 RTTtex_xy, RTTtex_wh;
   RTTxy = RTTxywh.xy; RTTwh = RTTxywh.zw;
   RTTtex_xy = RTTtex_xywh.xy; RTTtex_wh = RTTtex_xywh.zw;
   Output.Tex1 = ((Input.Position.xy - RTTxy) / RTTwh) * RTTtex_wh + RTTtex_xy;

   return( Output );
   
}
