#include "dxstdafx.h"
#include "Active.h"

///--- CACTIVE ---
void CActive::SetPos(D3DXVECTOR2 newPos)
{
	pos = newPos;
	bbox.Set(&bbox_ini, pos);
	bbox_exported.Set(&bbox_exported_ini, pos);
}

void CActive::Move(D3DXVECTOR2 delta)
{
	pos += delta;
	bbox.Set(&bbox_ini, pos);
	bbox_exported.Set(&bbox_exported_ini, pos);
}

void CActive::SetAngle(float fnAngle)
{
	fAngle = fnAngle;
}

void CActive::InitInternalData()
{

}
