#pragma once
#include "ComponentInterfaces.h"

// declare used classes
//class CCollisionShape;
class CLevelArea;

#define K_PPC_DEFAULT_FLOOR_FRICTION		10.0f
#define K_PPC_DEFAULT_FLOOR_BOUNCE			0.5f

///--- collision flags ---
// collides with walls (tiles)
#define K_PPC_COLLFLAG_TILES	1
// collides with collision boxes
#define K_PPC_COLLFLAG_BOXES	2
// collides with everything
#define K_PPC_COLLFLAG_ALL	0xffff

class CPointPhysComponent : public IBasePointPhysComponent
{
public:
	CLevelArea*			pArea;								// Pointer to current area
	bool				bFlagPhysicsEnabled;				// Set to enable physics (only with FlagCollision Enabled) - bounce, friction etc. false-stops on collision
	int					nFlagsCollision;					// Collision checking flags

	Vec3				accel;								// Forces that act on the point. Set Z to 0 to skip floor collision. Set XY to 0 to skip tiles/boxes collisions.

	bool				bContacting;						// Is it contacting?
	bool				bContactStarted;					// Tells you for a frame that the contact has started
	bool				bIsStatic;							// Did it completely stop?
	bool				bIsStaticZ;							// Did it stop on Z axis?
	bool				bIsDead;							// Tells if we must kill it as it exited the play area. #TODO: necessary?

	ePPCContactType		contactType;						// Returns type of current/last collision (use it when bContacting is true)
	Vec3				contactNormal;						// Last contact normal
	Vec3				contactPos;							// Last contact pos
	//will use later: CCollisionShape*	pContactShape;						// Contacting shape/type #TODO: add collision type with additional data in it

	float				fBounceF;							// Floor bounce restitution factor
	float				fFrictionF;							// Floor friction

public:
	CPointPhysComponent( bool bPhysicsEnabled, Vec3 vAcceleration = g_Vec3Zero, int nCollFlags = K_PPC_COLLFLAG_ALL );
	~CPointPhysComponent();

	void				Update( VecProj& vPos, float dTime, CLevel & level ) override;

	void				SetSpeed( Vec3 vSpeed ) override;
};
