// shader used to turn voronoi into distance field

float4 udistmod : register( c0 ); // .x contains the distance modifier percentage
sampler2D texIn : register( s0 );  //input voronoi last pass RT

struct PS_INPUT
{
	float4 Tex0:            TEXCOORD0; //tex coords
};

float4 ps_main(PS_INPUT Input) : COLOR0
{
   // input is the voronoi output
   // we calculate the distance between the closest surface and this pixel
   float2 UV = Input.Tex0.xy;
   float4 tex = tex2D(texIn, UV);
   float dist = distance(tex.xy, UV);
   float mapped = clamp(dist * udistmod.x, 0.0, 1.0); // original pt versiunea cu raytracing
   //float mapped = dist;// *udistmod.x;
   return float4(mapped, mapped, mapped, 1.0);
}