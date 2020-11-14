//x=spec power, y=spec intens, z=light intensity
float4 fSpecularData : register(c0); 
//x=diffuse multiplier, y=spot color dodge layer opacity, z=spot linear dodge opacity
float4 fAtmosphericData : register(c1); 

//tex render target (ul-diffuse, ur-normal+height, dl-specular power?)
sampler2D texAll : register(s0);  
sampler2D texSpot : register(s1);

struct PS_INPUT 
{
   float4 VertColor:       COLOR0;    //light color
   float2 Tex0:            TEXCOORD0; //tex spot
   float2 Tex1:            TEXCOORD1; //tex render target
   float3 dirToLight :     TEXCOORD2; //direction to light
};

float4 ps_main( PS_INPUT Input ) : COLOR0
{
   float selfIllum          = tex2D(texAll, float2(Input.Tex1.x + 0.5f, Input.Tex1.y)).z;
   float4 fvBaseColor       = tex2D(texAll, Input.Tex1);

   float4 fvRealColor = (fvBaseColor * Input.VertColor) + (fvBaseColor * selfIllum);
   
   return( saturate(fvRealColor) );
}