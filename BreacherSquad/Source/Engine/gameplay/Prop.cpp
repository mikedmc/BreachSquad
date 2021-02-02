#include "dxstdafx.h"
#include "Prop.h"

///--- CACTIVE ---
void CProp::SetPos(Vec2 newPos)
{
	pos = newPos;
	bbox.Set(&bbox_ini, pos);
	bbox_exported.Set(&bbox_exported_ini, pos);
}

void CProp::Move(Vec2 delta)
{
	pos += delta;
	bbox.Set(&bbox_ini, pos);
	bbox_exported.Set(&bbox_exported_ini, pos);
}

void CProp::SetAngle(float fnAngle)
{
	fAngle = fnAngle;
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
