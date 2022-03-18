#include "dxstdafx.h"
#include "VecProj.h"

VecProj::VecProj()
{
	xyz.x = 0.0f; xyz.y = 0.0f, xyz.z = 0.0f;
	xy.x = xy.y = 0.0f;
	proj_h = 0.0f;
	xy_proj = xy;
}

VecProj::VecProj(const VecProj & o)
{
	xyz = o.xyz;
	xy = o.xy;
	proj_h = o.proj_h;
	xy_proj = o.xy_proj;
}

VecProj::VecProj(const Vec3 & vec)
{
	xyz = vec;
	// compute other components
	xy.x = xyz.x; xy.y = xyz.y;
	proj_h = Z_TO_H(xyz.z);
	xy_proj = Vec2(xyz.x, xyz.y - proj_h);
}

VecProj::VecProj(const Vec2 & vec)
{
	xyz = Vec3(vec.x, vec.y, 0.0f);
	// compute other components
	xy.x = xyz.x; xy.y = xyz.y;
	proj_h = Z_TO_H(xyz.z);
	xy_proj = Vec2(xyz.x, xyz.y - proj_h);
}

void VecProj::Set(Vec3 & vec)
{
	xyz = vec;
	// compute other components
	xy.x = xyz.x; xy.y = xyz.y;
	proj_h = Z_TO_H(xyz.z);
	xy_proj = Vec2(xyz.x, xyz.y - proj_h);
}

void VecProj::Set()
{
	Vec3 v( 0.0f, 0.0f, 0.0f );
	Set( v );
}

void VecProj::Move(Vec3 & delta)
{
	xyz += delta;
	// compute other components
	xy.x = xyz.x; xy.y = xyz.y;
	proj_h = Z_TO_H(xyz.z);
	xy_proj = Vec2(xyz.x, xyz.y - proj_h);
}
