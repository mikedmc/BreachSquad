// shader used to turn voronoi into distance field

float4 udistmod : register( c0 ); // .x contains the distance modifier percentage
sampler2D texIn : register( s0 );  //input voronoi last pass RT

struct PS_INPUT
{
	float4 Tex0:            TEXCOORD0; //tex coords
};

#define V2F16(v) ((v.y * float(0.0039215689)) + v.x)
#define F16V2(f) float2(floor(f * 255.0) * float(0.0039215689), frac(f * 255.0))

// packs float to 2 vector components for higher precision
float2 fnFloatPackToV2( float f ) {
	return float2(floor( f * 255.0 ) * float( 0.0039215689 ), frac( f * 255.0 ));
}

float4 ps_main(PS_INPUT Input) : COLOR0
{
   // input is the voronoi output
   // we calculate the distance between the closest surface and this pixel
   float2 UV = Input.Tex0.xy;
   float4 tex = tex2D(texIn, UV);
   //float2 jumpflood = float2(V2F16( tex.xy ), V2F16( tex.zw )); // high precision on alpha too (not working in dx9)
   //float2 jumpflood = float2(V2F16( tex.xy ), tex.z );// precision test on X (works)
   float2 jumpflood = tex.xy; // without high precision - V2F16 not working on dx9, try float textures
   float dist = distance(UV, jumpflood);
   dist = saturate( dist ); 
//   float mapped = clamp(dist * udistmod.x, 0.0, 1.0); // original pt versiunea cu raytracing
   //float mapped = dist;// *udistmod.x;
   
   //return float4(fnFloatPackToV2(dist), 0.0, 1.0); 
   return float4(dist, dist, dist, 1.0);
    //return dist;
   //return float4(F16V2( dist ), 0.0, 1.0);



   /*
   vec4 jfuv = texture2D(gm_BaseTexture, in_TextCoord);
	vec2 jumpflood = vec2(V2F16(jfuv.rg),V2F16(jfuv.ba));
	float dist = distance(in_TextCoord, jumpflood);
	gl_FragColor = vec4(F16V2(dist), 0.0, 1.0);
	*/
}