#include "dxstdafx.h"
#include "Prop.h"

///--- CACTIVE ---
void CProp::SetPos(Vec3 newPos)
{
	pos = newPos;
	bbox.Set(&bbox_ini, pos.xy_proj);
	bbox_floor.Set(&bbox_floor_ini, pos.xy);
	bbox_proj.Set(bbox_floor.vMin.x, bbox_floor.vMin.y - Z_TO_H(fHeight), bbox_floor.vMax.x, bbox_floor.vMax.y);
}

void CProp::Move(Vec3 delta)
{
	Vec3 npos = pos.xyz + delta;
	pos = npos;
	bbox.Set(&bbox_ini, pos.xy_proj);
	bbox_floor.Set(&bbox_floor_ini, pos.xy);
	bbox_proj.Set(bbox_floor.vMin.x, bbox_floor.vMin.y - Z_TO_H(fHeight), bbox_floor.vMax.x, bbox_floor.vMax.y);
}

void CProp::PostConstructionInit()
{
	// compute bboxes on init
	bbox.Set(&bbox_ini, pos.xy_proj);
	bbox_floor.Set(&bbox_floor_ini, pos.xy);
	bbox_proj.Set(bbox_floor.vMin.x, bbox_floor.vMin.y - Z_TO_H(fHeight), bbox_floor.vMax.x, bbox_floor.vMax.y);
}

void CProp::BeginPlay()
{

}

void CProp::EndPlay()
{

}
