#include "dxstdafx.h"
#include "Prop.h"

///--- CACTIVE ---
void CProp::SetPos(Vec3 newPos)
{
	pos = newPos;
	bbox.Set(&bbox_ini, pos.xy_proj);
	bbox_floor.Set(&bbox_floor_ini, pos.xy);
}

void CProp::Move(Vec3 delta)
{
	Vec3 npos = pos.xyz + delta;
	pos = npos;
	bbox.Set(&bbox_ini, pos.xy_proj);
	bbox_floor.Set(&bbox_floor_ini, pos.xy);
}

void CProp::PostConstructionInit()
{
	// compute bboxes on init
	bbox.Set(&bbox_ini, pos.xy_proj);
	bbox_floor.Set(&bbox_floor_ini, pos.xy);
}

void CProp::BeginPlay()
{

}

void CProp::EndPlay()
{

}
