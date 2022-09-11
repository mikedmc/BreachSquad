#pragma once

///----------------------------------------------------------------------------------
/// Vector class that encapsulates fake projection computation for fake 3d games
///----------------------------------------------------------------------------------
class VecProj
{
public:
	Vec3		xyz;			// 3d component
	Vec2		xy;				// xy plane component (floor). Don't set directly!
	float		proj_h;			// height after projection. Don't set directly!
	Vec2		xy_proj;		// projected to 2d space where z adds to y. Don't set directly!

	VecProj();
	VecProj( const VecProj & o );
	VecProj( const Vec3 & vec );
	VecProj( const Vec2 & vec );
	VecProj( float x, float y, float z );

	void		Set( Vec3 & vec );
	void		Set( float x, float y, float z );
	// Resets vector components to 0.0f
	void		Set();
	void		Move( const Vec3 delta );
};
