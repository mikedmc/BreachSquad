#pragma once

// declare used classes
class CCollisionShape;

#define K_LVL_PHYSPT_DEFAULT_FLOOR_FRICTION		10.0f
#define K_LVL_PHYSPT_DEFAULT_FLOOR_BOUNCE		0.5f

///--------------------------------------------------------------------------
/// PHYSICS POINTS
/// used for simulationg bullets and objects that collide with the level
///--------------------------------------------------------------------------

// Type of returned collision
enum eRetContactType {
	K_COLLTYPE_NONE = 0,
	K_COLLTYPE_TILE,	
	K_COLLTYPE_BOX,
	K_COLLTYPE_FLOOR,
};


///--- collision flags ---
// collides with walls (tiles)
#define K_LVL_PHYSP_COLLFLAG_TILES	1
// collides with collision boxes
#define K_LVL_PHYSP_COLLFLAG_BOXES	2
// collides with everything
#define K_LVL_PHYSP_COLLFLAG_ALL	0xffff

class CPhysicsPoint {
public:
	
	bool				bFlagRotationEnabled;				// Set to enable rotation updates
	bool				bFlagPhysicsEnabled;				// Set to enable physics (only with FlagCollision Enabled) - bounce, friction etc. false-stops on collision

	int					nFlagsCollision;					// Collision checking flags

	float				fAngle;
	float				fAngularSpeed;
	float				fAngularAccel;

	Vec3				pos, pos_last;						// Position of the point
	Vec3				speed;								// Current speed of the point
	Vec3				accel;								// Forces that act on the point. Set Z to 0 to skip floor collision. Set XY to 0 to skip tiles/boxes collisions.
	
	bool				bContacting;						// Is it contacting?
	bool				bContactStarted;					// Tells you for a frame that the contact has started
	bool				bIsStatic;							// Did it completely stop?
	bool				bIsStaticZ;							// Did it stop on Z axis?
	bool				bIsDead;							// Tells if we must kill it as it exited the play area. #TODO: necessary?

	eRetContactType		contactType;						// returns type of current/last collision (use it when bContacting is true)
	Vec3				contactNormal;						// Last contact normal
	Vec3				contactPos;							// Last contact pos
	CCollisionShape*	pContactShape;						// Contacting shape/type #TODO: add collision type with additional data in it

	float				fBounceF;							// floor bounce restitution factor
	float				fFrictionF;							// floor friction

	CPhysicsPoint() : contactType(K_COLLTYPE_NONE), bContacting(false), bContactStarted(false), bIsStatic(false), nFlagsCollision(K_LVL_PHYSP_COLLFLAG_ALL),
		bFlagRotationEnabled(false), fAngle(0.0f), fAngularSpeed(0.0f), fAngularAccel(0.0f), bIsStaticZ(false),
		bFlagPhysicsEnabled(false), fBounceF(K_LVL_PHYSPT_DEFAULT_FLOOR_BOUNCE), fFrictionF(K_LVL_PHYSPT_DEFAULT_FLOOR_FRICTION),
		bIsDead(false), pContactShape(NULL)
	{
		pos = pos_last = Vec3(0.0f, 0.0f, 0.0f);
		speed = Vec3(0.0f, 0.0f, 0.0f);
		accel = Vec3(0.0f, 0.0f, 0.0f);
		contactNormal = Vec3(0.0f, 0.0f, 0.0f);
		contactPos = Vec3(0.0f, 0.0f, 0.0f);
	};

	void Init();
	
	// Forces a new position for the point, in case of collisions or other situations
	void SetPosForced(Vec3 vecPos);
};



