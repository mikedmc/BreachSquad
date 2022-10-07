// water shader, works on color map, doesn't change normals

float4 c4WaterData : register( c0 ); //x:water murkyness, y: water texture scale (approx 0.005), z: water diffraction factor, w: water height offset
float4 c4WaterColor: register( c1 ); // water color (rgba)
float4 c4SurfaceData: register( c2 ); // x: surface color adder, y: specular alpha
float4 c4WaterAnim: register( c3 );  // x: water offX, y : water offY( .xy vector used for animating texture coords )

sampler2D texColor : register( s0 );		// tileset color
sampler2D texNrmH : register( s1 );			// tileset normals
sampler2D texWater : register( s2 );		// water texture (RG - normal, B-caustics)

// uses output from vs_sprites2d
struct PS_INPUT
{
	float4 VertColor:       COLOR0;    //vertex color
	float2 Tex0:            TEXCOORD0; //tex coord for tileset 
	float4 Tex1:            TEXCOORD1; //position in world space
};

/*
// version without specular, just deformations
float4 ps_main(PS_INPUT Input) : COLOR0
{
	//XY normals, Z is height in world space. W is alpha
	float4 normal_h = tex2D( texNrmH, Input.Tex0.xy );
	// bring depth from 0..1 to water depth (0..0.5 is under water where 0 is deepest)
	float nrmTexH = 1.0 - normal_h.z * 2.0f;
	float waterColorPerc = saturate( nrmTexH * c4WaterData.x );
	// get water coords scaled and animated
	float2 uvWater1 = Input.Tex1.xy * c4WaterData.y + c4WaterAnim.xy;
	float4 watercol1 = tex2D( texWater, uvWater1 );
	// get water diffraction (or diffraction offsets) scaled by water diffraction factor
	float2 waterDiff = ( ( watercol1.xy * 2.0f ) - 1.0f ) * c4WaterData.z * nrmTexH;
	// get diffracted color from tileset texture and add caustics color
	float4 texcol = tex2D( texColor, Input.Tex0.xy + waterDiff );

	float4 fvFinalColor = lerp( texcol, c4WaterColor, waterColorPerc );

	return(fvFinalColor);
}
*/

// uses water normal to get speculars
float4 ps_main( PS_INPUT Input ) : COLOR0
{
	//XY normals, Z is height in world space. W is alpha
	float4 normal_h = tex2D( texNrmH, Input.Tex0.xy );
	// bring depth from 0..1 to water depth 
	float nrmTexH = saturate( 1.0 - normal_h.z * 2.0f - c4WaterData.w );
	// get water coords scaled and animated
	float2 uvWater1 = Input.Tex1.xy * c4WaterData.y + c4WaterAnim.xy;
	float4 watercol1 = tex2D( texWater, uvWater1 );
	float2 uvWater2 = Input.Tex1.xy * c4WaterData.y + c4WaterAnim.zw;
	float4 watercol2 = tex2D( texWater, uvWater2 );
	// get water diffraction (or diffraction offsets) scaled by water diffraction factor
	float2 waterDiff = ( ( watercol1.xy + watercol2.xy ) - 1.0f ) * c4WaterData.z * nrmTexH;
	// get diffracted color from tileset texture and add caustics color
	float4 texcol = tex2D( texColor, Input.Tex0.xy + waterDiff );
	// gets the texture value from BLUE channel by normal coords
	float fWaterLimit = 1.0f;
	if ( ( 1.0 - normal_h.z * 2.0f ) < c4WaterData.w )
	{
		fWaterLimit = 0.0f;
	}
	float specularPerc = tex2D( texWater, ( ( watercol1.xy + watercol2.xy ) / 2.0f ) ).z * c4SurfaceData.y * fWaterLimit;
	

	float waterColorPerc = nrmTexH * c4WaterData.x;
	float4 fvFinalColor = specularPerc + lerp( texcol, c4WaterColor, waterColorPerc + c4SurfaceData.x * fWaterLimit );
	fvFinalColor.w = 1.0f;

	return( fvFinalColor );
}
