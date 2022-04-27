#include "dxstdafx.h"
#include "AABBProj.h"

CAABBProj::CAABBProj() : 
	vHalfSize( 0.0f, 0.0f, 0.0f ), vCenter( 0.0f, 0.0f, 0.0f ), 
	vMin( 0.0f, 0.0f, 0.0f ), vSize( 0.0f, 0.0f, 0.0f ), vMax( 0.0f, 0.0f, 0.0f )
{
}

CAABBProj::CAABBProj( Vec3 min, Vec3 max )
{
	Set( min, max );
}

CAABBProj::CAABBProj( CAABBProj& src )
{
	Set( src.vMin, src.vMax );
}

void CAABBProj::Set( Vec3 min, Vec3 max )
{
	vMin = min;
	vMax = max;

	vSize = vMax - vMin;
	vHalfSize = vSize / 2.0f;
	vCenter = vMin + vHalfSize;

	box_xy.Set( vMin.x, vMin.y, vMax.x, vMax.y );
}



CAABBProjEx::CAABBProjEx( const CAABBProj & box )
{
	Set( box.vMin, box.vMax );
	vMin_ini = box.vMin;
	vMax_ini = box.vMax;
}

CAABBProjEx::CAABBProjEx()
{
	vMin_ini = { 0.0f, 0.0f, 0.0f };
	vMax_ini = { 0.0f, 0.0f, 0.0f };
}

void CAABBProjEx::SaveSnapshot()
{
	vMin_ini = vMin;
	vMax_ini = vMax;
}

void CAABBProjEx::SetSnapshot( Vec3 min, Vec3 max )
{
	vMin_ini = min;
	vMax_ini = max;
}

void CAABBProjEx::RestoreSnapshot( Vec3 vOffset /*= { 0.0f, 0.0f, 0.0f } */ )
{
	Set( vMin_ini + vOffset, vMax_ini + vOffset );
}
