#include "dxstdafx.h"
#include "Prop.h"

///--- CACTIVE ---
void CProp::SetPos(Vec3 newPos)
{
	pos = newPos;
	bbox.Set(&bbox_ini, pos.xy);
	bbox_exported.Set(&bbox_exported_ini, pos.xy);
}

void CProp::Move(Vec3 delta)
{
	Vec3 npos = pos.xyz + delta;
	pos = npos;
	bbox.Set(&bbox_ini, pos.xy);
	bbox_exported.Set(&bbox_exported_ini, pos.xy);
}

void CProp::PostConstructionInit()
{

}

void CProp::BeginPlay()
{

}

void CProp::EndPlay()
{

}
