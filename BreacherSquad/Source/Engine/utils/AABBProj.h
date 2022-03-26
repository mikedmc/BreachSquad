#pragma once

///----------------------------------------------------------------------------------
/// Vector class that encapsulates fake projection computation for fake 3d games
///----------------------------------------------------------------------------------
class CAABBProj
{
public:
	CAABB		box_proj;
	CAABB		box_floor;
	float		height;

	CAABBProj( Vec2 min, Vec2 max, float heightZ );

	CAABBProj( const CAABBProj& src );
};
