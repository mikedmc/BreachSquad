#include "dxstdafx.h"
#include "PhysPtComp.h"

CPointPhysComponent::CPointPhysComponent( bool bEnableBounce, Vec3 vAcceleration, int nCollFlags )
{
	Reset();

	nFlagsCollision = nCollFlags;
	bFlagBounceEnabled = bEnableBounce;
	accel = vAcceleration;
}

CPointPhysComponent::~CPointPhysComponent()
{
	pArea = nullptr;
}

void CPointPhysComponent::Reset()
{
	pArea = nullptr;

	accel = g_Vec3Zero;
	speed = g_Vec3Zero;
	contactNormal = g_Vec3Zero;
	contactPos = g_Vec3Zero;

	contactType = PCT_NONE;
	bContacting = false;
	bContactStarted = false; 
	bIsStatic = false;
	bIsStaticZ = false; 
	fBounceF = K_PPC_DEFAULT_FLOOR_BOUNCE;
	fFrictionF = K_PPC_DEFAULT_FLOOR_FRICTION;
	bIsDead = false; 
	pArea = nullptr;
	nFlagsCollision = K_PPC_COLLFLAG_ALL;
	bFlagBounceEnabled = false;
}

void CPointPhysComponent::Update( VecProj& vPos, float dTime, CLevel & level )
{
	if ( bIsDead )
		return;
	// kill it when it gets outside the play area
	if ( !Rects::PointInRect( vPos.xy, level.m_levelAABB ) )
	{
		bIsDead = true;
		bIsStatic = true;
		return;
	}

	// what forces act on the point
	Vec3			vecForces = accel;
	bool			bWasContacting = bContacting;

	contactType = PCT_NONE;
	bContacting = false;
	//pContactShape = nullptr;
	bContactStarted = false;
	//save last pos
	Vec3 vLastPos = vPos.xyz;
	if ( pArea == nullptr )
	{
		pArea = level.Areas_GetAt( vPos.xy );
		if ( pArea == nullptr )
		{
			bIsDead = true;
			bIsStatic = true;
			return;
		}
	}
	_ASSERT( pArea != nullptr );
	///--- integrator
	if ( bIsStatic )
	{
		vecForces = g_Vec3Zero;
		speed = g_Vec3Zero;
	}
	if ( bIsStaticZ )
	{
		vecForces.z = 0.0f;
		speed.z = 0.0f;
	}

	speed += vecForces * dTime;
	Vec3 pos = vLastPos + speed * dTime;

	///--- check collisions
	if(!bIsStatic)
	{
		Vec2 collisionPoint, collisionNormal;
		Vec2 vFrom = Vec3XY( vLastPos);
		Vec2 vTo = Vec3XY( pos );
		Vec2 vMove = vTo - vFrom;

		ePPCContactType contactT = PCT_NONE;
		CCollisionShape* colShape = nullptr;

		// XY plane collision
		bool bCollided = false;
		float fMinContactDistance = 100000.0f;
		if ( ( vMove.x != 0.0f ) && ( vMove.y != 0.0f ) )
		{
			// tiles collision
			if ( nFlagsCollision & K_PPC_COLLFLAG_TILES )
			{
				Vec2i ptHitTilePos( 0, 0 );
				CTile* pColTile = level.SegmentTilesIntersectionEx( vFrom, vTo, collisionPoint, collisionNormal, &ptHitTilePos, pArea );
				if ( pColTile )
				{
					bCollided = true;
					contactT = PCT_TILE;
					fMinContactDistance = MUVec2Len( &( vFrom - collisionPoint ) );
				}
			}

			// collision with shapes 
			if ( nFlagsCollision & K_PPC_COLLFLAG_BOXES )
			{
				// (overwrite collpoint ONLY if closer and make other optimizations to see if we CAN collide with anything)
			// get only the boxes in vMove box
			//colShape = ColShape_Segment_Intersection_Arr(pos_last, pos, m_visibleList.logic_colShapes.m_pData, m_visibleList.logic_colShapes.Count(), &collisionPoint, &collisionNormal);
			}
		}

		if ( bCollided )
		{
			pos.x = collisionPoint.x + collisionNormal.x;
			pos.y = collisionPoint.y + collisionNormal.y;

			bContacting = true;
			//pContactShape = colShape;
			contactNormal = Vec2ToVec3XY0( collisionNormal );
			contactPos = Vec3( collisionPoint.x, collisionPoint.y, pos.z );
			contactType = contactT;

			if ( bFlagBounceEnabled )
			{
				float fDot = MUVec3Dot( &speed, &contactNormal );
				Vec3 Vn = contactNormal * fDot;
				Vec3 Vt = speed - Vn;
				// compute final speed
				speed = -Vn + Vt;
			}
			else
			{
				speed = g_Vec3Zero;
			}
		}

		// Minimum speed on Z when we consider the point stopped
		const float fMinSpeedZ = 0.1f;
		// Current floor height. #MAYBE: should get it from each tile
		float fFloorH = 0.0f;

		// Z floor collision at the end to bring it back up
		// Only compute this part if we have vertical acceleration and speed
		//#MAYBE: should check collision with ceiling too
		if ( ( accel.z != 0.0f ) && ( speed.z != 0.0f ) && ( pos.z <= fFloorH ) )
		{
			bContacting = true;
			// walls collisions have priority so only set normals if no other collision happened
			if ( !bCollided )
			{
				contactNormal = Vec3( 0.0f, 0.0f, -1.0f );
				contactPos = pos;
				//pContactShape = nullptr;
				contactType = PCT_FLOOR;
			}
			// get the point back above the floor
			pos.z = fFloorH - pos.z;

			if ( bFlagBounceEnabled )
			{
				// make sure it always ricochets upwards
				speed.z = fabs( speed.z * fBounceF );

				if ( fabs( speed.z * dTime ) < fMinSpeedZ )
				{
					speed.z = 0.0f;
					pos.z = fFloorH;
					bIsStaticZ = true;
				}
				// apply friction
				speed.x -= speed.x * fFrictionF * dTime;
				speed.y -= speed.y * fFrictionF * dTime;
			}
			else
			{
				speed = g_Vec3Zero;
			}
		}

		//check bounce or first contact - mainly for sounds and particles
		if ( bWasContacting == false )
		{
			bContactStarted = true;
		}

		//update current area (change only if not static)
		Vec2 vpos2d = Vec3XY( pos );
		if ( ( pArea == nullptr ) || ( !pArea->AABBbounds.PointIn( vpos2d ) ) )
		{
			pArea = level.Areas_GetAt( vpos2d );
			if ( pArea == nullptr )
			{
				bIsStatic = true;
				bIsDead = true;
				return;
			}
		}

		// is it almost stopped?
		if ( UTMath::Vec3AlmostZero( speed * dTime, 0.5f ) )
		{
			bIsStatic = true;
			speed = g_Vec3Zero;
		}
		else
		{
			bIsStatic = false;
		}
	}
	// save final position and convert to projected value
	vPos.Set( pos );
}

void CPointPhysComponent::SetAccel( Vec3 vAcceleration )
{
	accel = vAcceleration;
}

void CPointPhysComponent::SetBounceEnabled( bool bEnableBounce )
{
	bFlagBounceEnabled = bEnableBounce;
}

void CPointPhysComponent::SetCollisionFlags( int nCollFlags )
{
	nFlagsCollision = nCollFlags;
}

void CPointPhysComponent::SetSpeed( Vec3 vSpeed )
{
	speed = vSpeed;
}
