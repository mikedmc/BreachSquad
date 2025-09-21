struct PS_INPUT
{
	float4 UV0:            TEXCOORD0; //tex coords
};

// constants
uniform float PI = 3.141596;

// uniforms
float2 u_resolution = float2(512.0, 512.0);
static const float u_rays_per_pixel = 8;
Texture2D <float4> u_distance_data : register( t0 );
Texture2D <float4> u_scene_colour_data : register ( t1 );
Texture2D <float4> u_scene_emissive_data : register( t2 );
Texture2D <float4> u_last_frame_data : register( t3 );
Texture2D <float4> u_noise_data : register( t4 );
sampler samp0: register( s0 );
float u_dist_mod = 10.0;
bool u_bounce = true;
float u_emission_multi = 1.0;
float u_emission_range = 2.0;
float u_emission_dropoff = 2.0;
int u_max_raymarch_steps = 32;
float4 TIME : register ( c0 );
float4 v2pixelsize: register( c1 ); // inverse of RT resolution on both x and y


// ================================================================================
// return half a pixel size in UV space - used for some distance calculations to 
// determine if we're at a surface.
float epsilon()
{
	return 0.5 / max(u_resolution.x, u_resolution.y);
}

// ================================================================================
// return the surface data at a given location. 'uv' contains the hit location, while
// hit_data contains the distance data at that location which we already sampled from 
// the map() func.
void get_material(float2 uv, float4 hit_data, out float emissive, out float3 colour)
{	
	// if distance to nearest surface at this location is < epsilon (half pixel), we can
	// consider to be hitting that surface.
	if(hit_data.x / u_dist_mod < epsilon())
	{
		// convert uvs back to 0-1 range.
		float inv_aspect = u_resolution.y / u_resolution.x;
		uv.x *= inv_aspect;
		// read the surface data from emissive/colour maps. 
		// TODO: could probably be optimised by combining into one texture sample.
		float4 emissive_data = u_scene_emissive_data.SampleLevel(samp0, uv, 0);
		float4 colour_data = u_scene_colour_data.SampleLevel(samp0, uv, 0);
		emissive = emissive_data.r * u_emission_multi;
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
	float inv_aspect = u_resolution.y / u_resolution.x;
	uv.x *= inv_aspect;
	hit_data = u_distance_data.SampleLevel(samp0, uv, 0);
	float d = hit_data.x / u_dist_mod;
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
		if(step_dist < epsilon())
		{
			hit_pos = sample_point;
  			return true;
		}
		// if we didn't find a hit, step forward by the distance found in distance texture (min 1px).
		// since this distance is the distance to nearest surface, it guarantees we won't 'overstep'
		// and go past a surface. worst case is we are parallel and close to the surface, so we can't step
		// far but also won't reach the surface. this is where we have to make a trade-off in u_max_raymarch_steps.
		step_dist = max(step_dist, min(1.0 / u_resolution.x, 1.0 / u_resolution.y));
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
	last_emission = 0.0;
	for(int x = -1; x <= 1; x++)
	{
		for(int y = -1; y <= 1; y++)
		{
			float4 pixel = u_last_frame_data.SampleLevel(samp0, uv + pix * float2(float(x), float(y)), 0);
			if(pixel.a > last_emission)
			{
				last_emission = pixel.a;
				last_colour = pixel.rgb;
			}
		}
	}
}

// ================================================================================
// do the thing!
float4 ps_main(PS_INPUT pin) : SV_Target
{
	// since UVs are in 0-1 space, and our viewport could be non-square, we need to convert
	// UVs so our rays aren't skewed. i.e. if we're 1024x512 viewport, this will convert 0-1
	// x/y to 0-2 on x and 0-1 on y.
	// we will need to convert back when doing texture samples, which need 0-1 UV space.
	float2 uv = pin.UV0.xy;
	float aspect = u_resolution.x / u_resolution.y;
	float inv_aspect = u_resolution.y / u_resolution.x;
	uv.x *= aspect;
		
	float3 col = float3(0.0, 0.0, 0.0);
	float emis = 0.0;
	
	// get a random angle by sampling the noise texture and offsetting it by time (so we don't always sample
	// the same noise).
	float2 time = float2(TIME.x, TIME.y);
	float rand02pi = u_noise_data.SampleLevel(samp0, frac((uv + time) * 0.4), 0).r * 2.0 * PI; // noise sample
	float golden_angle = PI * 0.7639320225;
	
	for(float i = 0.0; i < u_rays_per_pixel; i++)
	{
		float2 hit_pos;
		float4 hit_data;
		float ray_dist;
		
		// get our ray dir by taking the random angle and adding golden_angle * ray number.
		
		//MIKE: aici in loc de cos sin ar trebui luat din blue noise vector si facuta normalizare optimizat: float invLen = inversesqrt(dot(dir, dir)); float2 approxDir = dir * invLen;
		float cur_angle = rand02pi + golden_angle * i;
		float2 rand_direction = float2(cos(cur_angle), sin(cur_angle));
		bool hit = raymarch(uv, rand_direction, hit_pos, hit_data, ray_dist);
		if(hit)
		{
			float mat_emissive;
			float3 mat_colour;
			get_material(hit_pos, hit_data, mat_emissive, mat_colour);
			
			// convert UVs back to 0-1 space.
			float2 st = hit_pos;
			st.x *= inv_aspect;
			
			float last_emission = 0.0;
			float3 last_colour = float3(0.0, 0.0, 0.0);
			if(u_bounce)
			{
				// we don't want emissive surfaces themselves to bounce light (we could, but it would probably blow
				// out the scene).
				if(mat_emissive < epsilon())
				{
					get_last_frame_data(st, v2pixelsize.xy, last_emission, last_colour);
				}
				// this is so light doesn't bounce off the surface it was emitted from.
				if(ray_dist < epsilon()) 
					last_emission = 0.0;
			}
			
			// calculate total emissive/colour values from direct and bounced (last frame) lighting.
			float emission = mat_emissive + last_emission;
			float r = u_emission_range;
			float drop = u_emission_dropoff;
			// attenuation calculation - very tweakable to get the correct sort of light range/dropoff.
			float att = pow(max(1.0 - (ray_dist * ray_dist) / (r * r), 0.0), u_emission_dropoff);
			emis += emission * att;
			col += (mat_emissive + last_emission) * (mat_colour + last_colour) * att;
		}
	}
	
	// right now, emis and col store the sum of contribution of all rays to this pixel, we need
	// to normalise it.
	emis *= (1.0 / u_rays_per_pixel);
	col *= (1.0 / u_rays_per_pixel);
	
	// colour data in rgb and emissive in alpha. this is important because when reading in the last frame data we
	// need colour and alpha to be separate. if we combined at this stage, the bounce calculations wouldn't work
	// properly.
	return float4(col, emis);
}