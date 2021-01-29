#include "dxstdafx.h"
#include "PhysicsPoint.h"

void CPhysicsPoint::Init()
{
	contactType = K_COLLTYPE_NONE;
	bContacting = false;
	bIsStatic = false;
	bIsStaticZ = false;
	nFlagsCollision = K_LVL_PHYSP_COLLFLAG_ALL;
	bFlagRotationEnabled = false;
	bFlagPhysicsEnabled = false;
	fBounceF = K_LVL_PHYSPT_DEFAULT_FLOOR_BOUNCE;
	fFrictionF = K_LVL_PHYSPT_DEFAULT_FLOOR_FRICTION;
	pos = g_Vec3Zero;
	speed = g_Vec3Zero;
	accel = g_Vec3Zero;
	bIsDead = false;
}

