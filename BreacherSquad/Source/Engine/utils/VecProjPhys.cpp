#include "dxstdafx.h"
#include "VecProjPhys.h"

VecProjPhys::VecProjPhys()
{
	pos.Set();
	v = Vec3( 0.0f, 0.0f, 0.0f );
}

void VecProjPhys::Set( Vec3 & vPos, Vec3 & vSpeed )
{
	pos = vPos;
	v = vSpeed;
}
