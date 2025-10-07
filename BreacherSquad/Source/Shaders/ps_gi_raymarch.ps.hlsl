struct PS_INPUT
{
	float4 UV0:            TEXCOORD0; //tex coords
};

// constants
static const float PI = 3.141592;
static const float phi = 1.6180339887498948482045868343656381177203091798058;
static const float DOUBLE_PI = 6.283185307179586;
// uniforms
static const float u_rays_per_pixel = 6;
Texture2D <float4> u_distance_data;
Texture2D <float4> u_scene_colour_data;
Texture2D <float4> u_scene_emissive_data;
Texture2D <float4> u_last_frame_data;
Texture2D <float4> u_noise_data;
sampler samp0: register( s0 );
float3 u_emission = float3(1.0, 2.0, 2.0); //.x:multiplier=1.0 .y:range=2.0 .z:dropoff=2.0
static const int u_max_raymarch_steps = 32; // aici se mai poate umbla la final
float4 TIME : register ( c0 ); // .x .y different time scales
float4 rt_resolution : register( c1 ); //.x:RT_width, .y:RT_height, z: 1/RT_width, w: 1/RT_height -> zw=pixel size
float4 u_dist_mod : register(c2); //.x:distance modifier, .y:EPSILON half a pixel on longer axis, .z: EPSILON half pixel on short axix


//--------> ori randez luminile pe poligoanele lipsa din podea ca sa imi fac culori realiste ori fac bufferul de intersectie al luminii sa fie peretii si fug dinspre ei 1px si iau culoarea de pe podea
// - get last frame data ar trebui sa ia cel mai luminos din zona ca sa faca un fel de blur

// ================================================================================
// return half a pixel size in UV space - used for some distance calculations to 
// determine if we're at a surface.
/*
DMC: obsolete: comes from the code now
float epsilon()
{
	return 0.5 / max(rt_resolution.x, rt_resolution.y);
}
*/

float get_luminance( float3 colrgb )
{
	return (colrgb.r * 0.3) + (colrgb.g * 0.59) + (colrgb.b * 0.11);
}

// ================================================================================
// return the surface data at a given location. 'uv' contains the hit location, while
// hit_data contains the distance data at that location which we already sampled from 
// the map() func.
void get_material(float2 uv, float4 hit_data, out float emissive, out float3 colour)
{	
	// if distance to nearest surface at this location is < epsilon (half pixel), we can
	// consider to be hitting that surface.
	if(hit_data.x / u_dist_mod.x < u_dist_mod.y)
	{
		// read the surface data from emissive/colour maps. 
		// TODO: could probably be optimised by combining into one texture sample.
		float4 emissive_data = u_scene_emissive_data.SampleLevel(samp0, uv, 0);
		float4 colour_data = u_scene_colour_data.SampleLevel(samp0, uv, 0);
		emissive = emissive_data.r * u_emission.x;
		colour = colour_data.rgb;
	}
	// otherwise the raymarch reached max steps before finding a surface, so nothing is
	// contributed to the pixel brightness/colour.
	else
	{
		emissive = 0.0;
		colour = float3( 0.0, 0.0, 0.0 );
	}
}

// ================================================================================
// get distance data (to nearest surface) from given UV location.
float map(float2 uv, out float4 hit_data)
{
	float inv_aspect = rt_resolution.y / rt_resolution.x;
	uv.x *= inv_aspect;
	hit_data = u_distance_data.SampleLevel(samp0, uv, 0);
	float d = hit_data.x / u_dist_mod.x;
    return d;
}

// ================================================================================
// march a ray from a pixel in a given direction, until it hits a surface or runs out of
// steps. will return if a surface is hit, the hit location, hit data, and total length
// of the ray.
bool raymarch(float2 origin, float2 ray, out float2 hit_pos, out float4 hit_data, out float ray_dist)
{
	float t = 0.0;
	float prev_dist = 1.0;
	float step_dist = 1.0;
	float2 sample_point;
	for(int i = 0; i < u_max_raymarch_steps; i++)
	{
		sample_point = origin + ray * t;
		step_dist = map(sample_point, hit_data);
		
		// consider a hit if distance to surface is < epsilon (half pixel).
		if(step_dist <= u_dist_mod.y)
		{
			hit_pos = sample_point;
  			return true;
		}
		// if we didn't find a hit, step forward by the distance found in distance texture (min 1px).
		// since this distance is the distance to nearest surface, it guarantees we won't 'overstep'
		// and go past a surface. worst case is we are parallel and close to the surface, so we can't step
		// far but also won't reach the surface. this is where we have to make a trade-off in u_max_raymarch_steps.
		
		//step_dist = max(step_dist, min(1.0 / rt_resolution.x, 1.0 / rt_resolution.y));
		// Mike: using inverse of resolution
        step_dist = max(step_dist, min(rt_resolution.z, rt_resolution.w));
		t += step_dist;
		ray_dist = t;
	}
	return false;
}

// ================================================================================
// get emission/colour data at this location from the last frame. this allows 'infinite'
// bounces as sufaces that were lit in the last frame will now act as emissive surfaces.
// we need to sample a 3x3 grid around the hit pixel because the surface itself won't have
// emissive data (since it is rendered black, generally), sampling around it will find the
// closest emissive pixel though, which we can consider the surface's value.
void get_last_frame_data(float2 uv, float2 pix, out float last_emission, out float3 last_colour)
{
	
	float4 pixel = u_last_frame_data.SampleLevel( samp0, float2(uv.x, uv.y), 0 );
	last_emission = pixel.a;
	last_colour = pixel.rgb;
	
	/*
	float4 center_pixel = u_last_frame_data.SampleLevel( samp0, float2(uv.x, uv.y), 0 );
	float e = center_pixel.a;
	e = max( get_luminance( u_last_frame_data.SampleLevel( samp0, float2(uv.x - pix.x	, uv.y + pix.y), 0 )), e );
	e = max( get_luminance( u_last_frame_data.SampleLevel( samp0, float2(uv.x			, uv.y + pix.y), 0 )), e );
	e = max( get_luminance( u_last_frame_data.SampleLevel( samp0, float2(uv.x + pix.x	, uv.y + pix.y), 0 )), e );
	e = max( get_luminance( u_last_frame_data.SampleLevel( samp0, float2(uv.x - pix.x	, uv.y		  ), 0 )), e );
	e = max( get_luminance( u_last_frame_data.SampleLevel( samp0, float2(uv.x + pix.x	, uv.y		  ), 0 )), e );
	e = max( get_luminance( u_last_frame_data.SampleLevel( samp0, float2(uv.x - pix.x	, uv.y - pix.y), 0 )), e );
	e = max( get_luminance( u_last_frame_data.SampleLevel( samp0, float2(uv.x			, uv.y - pix.y), 0 )), e );
	e = max( get_luminance( u_last_frame_data.SampleLevel( samp0, float2(uv.x + pix.x	, uv.y - pix.y), 0 )), e );

	last_emission = e;
	last_colour = center_pixel.rgb;
	*/

		/*
	last_emission = 0.0; 
	//last_colour = float3(0.0, 0.0, 0.0); //DMC: I added this
	for(int x = -1; x <= 1; x++)
	{
		for(int y = -1; y <= 1; y++)
		{
			float4 pixel = u_last_frame_data.SampleLevel(samp0, float2(uv.x + pix.x * float(x), uv.y + pix.y * float(y)), 0);
			//if(pixel.a > last_emission)
			if(get_luminance(pixel.rgb) > last_emission)
			{
				last_emission = pixel.a;
				last_colour = pixel.rgb;
			}
		}
	}
	*/
}

float3 lin_to_srgb( float3 color )
{
	float3 x = color.rgb * 12.92;
	float3 y = 1.055 * pow( clamp( color.rgb, 0.0, 1.0 ), float3( 0.4166667, 0.4166667, 0.4166667) ) - 0.055;
	float3 clr = color.rgb;
	clr.r = (color.r < 0.0031308) ? x.r : y.r;
	clr.g = (color.g < 0.0031308) ? x.g : y.g;
	clr.b = (color.b < 0.0031308) ? x.b : y.b;
	return clr.rgb;
}


/*

#define USE_HASH 0
const float RAYS_PER_PIXEL = 16.;
const float phi = 1.6180339887498948482045868343656381177203091798058;
float tau = 6.283185307179586;
float hash(float p) { return fract(sin(p) * 43758.5453123);}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
	setupSurfaces();
	vec2 uv = fragCoord/iResolution.xy;
	vec2 originalUv = uv;
	uv = uv * 2.0 - 1.0;
	float aspect = iResolution.x / iResolution.y;
	float invAspect = iResolution.y / iResolution.x;
	uv.x *= aspect;

	vec3 col = vec3(0.);
	float emis = 0.;

	for(float i = 0.; i < RAYS_PER_PIXEL; i++)
	{
		vec2 hitPos;
		float dist;
		float curAngle = fract(texture(iChannel2, originalUv).r + i * phi) * tau;
		vec2 randDirection = vec2(cos(curAngle),sin(curAngle));
		bool hit = trace(uv, randDirection, hitPos, dist);
		if(hit)
		{
			Material mat = getMaterial(hitPos);
			float d = max(dist,0.);
			vec2 st = hitPos;
			st.x *=  invAspect;
			st = (st + 1.0)*0.5;

			float lastEmission = 0.;
			if(mat.emission <= EPSILON)
			{

				lastEmission = getEmissionFromBuffer(st);
				#ifdef WEIGHTED
				vec2 normal = estimateNormal(hitPos);
				float c = clamp(dot(-randDirection,normal),0.,1.);
				lastEmission *= c;
				#endif
			}
			if( iFrame==0 || d < EPSILON) lastEmission = 0.0;

			float emission = mat.emission + lastEmission;
			emis += emission*0.6;
			col += (mat.emission + lastEmission)*mat.color;
		}
	}
	col /= RAYS_PER_PIXEL;
	emis /= RAYS_PER_PIXEL;

	vec2 st = fragCoord/iResolution.xy;
	vec3 oldCol  = texture(iChannel0, st).rgb;
	vec3 newCol = col;

	if(iFrame==0) oldCol = vec3(0.0);

	float hysterisis = 0.9;
	fragColor = vec4(mix(newCol, oldCol, hysterisis), emis);
}*/

float hash( float p ) { return frac( sin( p ) * 43758.5453123 ); }
// ================================================================================
// do the thing!
float4 ps_main(PS_INPUT pin) : SV_Target
{
	// since UVs are in 0-1 space, and our viewport could be non-square, we need to convert
	// UVs so our rays aren't skewed. i.e. if we're 1024x512 viewport, this will convert 0-1
	// x/y to 0-2 on x and 0-1 on y.
	// we will need to convert back when doing texture samples, which need 0-1 UV space.
	float2 uv = pin.UV0.xy;

	float aspect = rt_resolution.x / rt_resolution.y;
	float inv_aspect = rt_resolution.y / rt_resolution.x;
	uv.x *= aspect;
		
	float3 colout = float3(0.0, 0.0, 0.0);
	float emis = 0.0;	
	
	// get a random angle by sampling the noise texture and offsetting it by time (so we don't always sample
	// the same noise).
	float2 time = float2(TIME.x, TIME.y);
	//float rand02pi = u_noise_data.SampleLevel( samp0, frac( (uv + time) * 0.4 ), 0 ).r * 2.0 * PI; // noise sample
	float rand02pi = u_noise_data.SampleLevel( samp0, pin.UV0.xy + TIME.xy, 0 ).r * DOUBLE_PI; // noise sample - good
	float golden_angle = PI * 0.7639320225;
	
	//float hittimes = 0.0;
	for ( float i = 0.0; i < u_rays_per_pixel; i++ )
	{
		float2 hit_pos;
		float4 hit_data;
		float ray_dist;

		// get our ray dir by taking the random angle and adding golden_angle * ray number.

		//DMC: sin is faster than lookups
		float cur_angle = frac(rand02pi + phi * i) * DOUBLE_PI;
		//float cur_angle = hash( rand02pi + float( i ) / float( u_rays_per_pixel ) ) * DOUBLE_PI;
		float2 rand_direction = float2(cos( cur_angle ), sin( cur_angle ));
		bool hit = raymarch( uv, rand_direction, hit_pos, hit_data, ray_dist );
		// if collision is inside the wall then colinwall will be 0
		//float colinwall = step( u_dist_mod.y, ray_dist );

		if(hit)
		{
			//hittimes += 1.0;
			//ray_dist = max( ray_dist, 0. );

			float2 uvst = hit_pos;
			// move back a few pixels to get the color:
			//uvst -= rand_direction * 0.5 * rt_resolution.zw;
			// convert uvs back to 0-1 range.
			uvst.x *= inv_aspect;
			//uvst = float2(hit_pos.x * inv_aspect, hit_pos.y);

			float mat_emissive;
			float3 mat_colour;
			get_material(uvst, hit_data, mat_emissive, mat_colour);
						
			float last_emission = 0.0;
			float3 last_colour = float3(0.0, 0.0, 0.0);
			
			
			//if(u_bounce) - DMC: ofc we want bounce
			{
				// we don't want emissive surfaces themselves to bounce light (we could, but it would probably blow
				// out the scene).

				if(mat_emissive < u_dist_mod.y)
				{
					// go back a pixel to get data
					float2 uvst2 = hit_pos;
					uvst2 -= rand_direction * 0.5 * rt_resolution.zw;
					uvst2.x *= inv_aspect;
					// using pixel size rt_resolution.zw
					get_last_frame_data(uvst2, rt_resolution.zw, last_emission, last_colour);
				}
				// this is so light doesn't bounce off the surface it was emitted from.
				
				if ( ray_dist < u_dist_mod.y ) {
					last_emission = 0.0;
					//last_colour = float3(0, 0, 0);
				}
			}
			
			
			// calculate total emissive/colour values from direct and bounced (last frame) lighting.
			float r = u_emission.y;
			float drop = u_emission.z;
			// attenuation calculation - very tweakable to get the correct sort of light range/dropoff.
			float att = pow( max( 1.0 - (ray_dist * ray_dist) / (r * r), 0.0 ), drop );
			//float emission = (mat_emissive + last_emission) * att;
			emis += 1;// emission;
			colout += (mat_colour * 0.6 + last_colour * 0.4) * att;// *colinwall;
			//colout += (mat_colour + last_colour) * emission;
			//ORIGINAL: colout += (mat_emissive + last_emission) * (mat_colour + last_colour) * att; 
		}
		
	}
	
	// right now, emis and col store the sum of contribution of all rays to this pixel, we need
	// to normalise it.
	emis = 1;// /= u_rays_per_pixel;
	colout /= u_rays_per_pixel;

	//emis = 1.0;
	//colout.g = hittimes / u_rays_per_pixel;
	
	// colour data in rgb and emissive in alpha. this is important because when reading in the last frame data we
	// need colour and alpha to be separate. if we combined at this stage, the bounce calculations wouldn't work
	// properly.
	//float3 curpx_last_color = u_last_frame_data.SampleLevel( samp0, pin.UV0.xy, 0 ).rgb;
	//return float4(lerp(colout, curpx_last_color, 0.5), 1.0); // temporal blur
	return float4(colout, emis);
}