#include "dxstdafx.h"
#include "PhysicsPoint.h"

void CPhysicsPoint::Init()
{
	eCollType = K_COLLTYPE_NONE;
	bContacting = false;
	bIsStatic = false;
	bIsStaticZ = false;
	nFlagsCollision = 0;
	bFlagRotationEnabled = false;
	fAngle = 0.0f;
	fAngularSpeed = 0.0f;
	fAngularAccel = 0.0f;
	bFlagPhysicsEnabled = false;
	fBounceF = 0.5f;
	fFrictionF = 10.0f;
	pos = Vec3(0.0f, 0.0f, 0.0f);
	speed = Vec3(0.0f, 0.0f, 0.0f);
	accel = Vec3(0.0f, 0.0f, 0.0f);
	bIsDead = false;
}

void CPhysicsPoint::SetPosForced(Vec3 vecPos)
{
	pos = vecPos;
}


