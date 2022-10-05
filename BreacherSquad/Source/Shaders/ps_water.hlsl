// water shader, works on color map, doesn't change normals
float4 c4WaterData : register(c0); //x:water murkyness, y:
//water color
float4 c4WaterColor: register(c1); 

sampler2D texColor : register( s0 );		// tileset color
sampler2D texNrmH : register( s1 );			// tileset normals
//sampler2D texWater : register( s2 );		// water texture (RG - normal, B-caustics)

// uses output from vs_sprites2d
struct PS_INPUT
{
	float4 VertColor:       COLOR0;    //vertex color
	float2 Tex0:            TEXCOORD0; //tex coord for tileset 
	float1 Tex1:            TEXCOORD1; //position in world space
};

float4 ps_main(PS_INPUT Input) : COLOR0
{
	//XY normals, Z is height in world space. W is alpha
	float4 normal_h = tex2D( texNrmH, Input.Tex0.xy );
	float4 texcol = tex2D( texColor, Input.Tex0.xy );
	// bring height from 0..1 to water depth (0..0.5 is under water where 0 is deepest)
	float nrmTexH = 1.0 - normal_h.z * 2.0f;
	float waterColorPerc = saturate( nrmTexH * c4WaterData.x );

	//float4 fvFinalColor = float4( Input.Tex1.x, Input.Tex1.y, 0.0, 1.0 );////normal_h;// texcol;// *( Input.VertColor );// +nrmTexH;
	float4 fvFinalColor = lerp(texcol, c4WaterColor, waterColorPerc);

	return(fvFinalColor);
}
