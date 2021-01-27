#include "dxstdafx.h"
#include "PhysicsPoint.h"

void CPhysicsPoint2D::Init()
{
	eCollType = K_COLLTYPE_NONE;
	bContacting = false;
	bIsStatic = false;
	nFlagsCollision = 0;
	bFlagRotationEnabled = false;
	fAngle = 0.0f;
	fAngularSpeed = 0.0f;
	fAngularAccel = 0.0f;
	bFlagPhysicsEnabled = false;
	fBounceF = 0.5f;
	fFrictionF = 10.0f;
	pos = Vec2(0.0f, 0.0f);
	speed = Vec2(0.0f, 0.0f);
	accel = Vec2(0.0f, 0.0f);
	bIsDead = false;
}

void CPhysicsPoint2D::SetPosForced(Vec2 vecPos)
{
	pos = vecPos;
}


