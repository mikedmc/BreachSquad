#include "dxstdafx.h"
#include "AABBProj.h"

CAABBProj::CAABBProj( const CAABBProj& src )
{
	box_proj = src.box_proj;
	box_floor = src.box_floor;
	height = src.height;
}

CAABBProj::CAABBProj( Vec2 min, Vec2 max, float heightZ )
{
	box_floor.Set( min, max );
	height = heightZ;
	box_proj.Set( Vec2(min.x, min.y - Z_TO_H(height)), max );
}
