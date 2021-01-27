#pragma once

// declare used classes
class CCollisionShape;


///--------------------------------------------------------------------------
/// PHYSICS POINTS
/// used for simulationg bullets and objects that collide with the level
///--------------------------------------------------------------------------

///--- collision flags ---
//colizioneaza cu solide?
#define K_LVL_PHYSP_COLLFLAG_SOLID	1
//colizioneaza cu boxes?
#define K_LVL_PHYSP_COLLFLAG_BOX	2
//interactioneaza cu apa ? - daca pluteste sau alte efecte
#define K_LVL_PHYSP_COLLFLAG_WATER	4

class CPhysicsPoint {
public:
	enum eCollisionType {
		K_COLLTYPE_NONE = 0,	// no collision
		K_COLLTYPE_FAST,		// fast - collides with visible collshapes
		K_COLLTYPE_PRECISE,		// precise - collides with ALL collshapes (slower)
	};
	
	bool				bFlagRotationEnabled;				// Set to enable rotation updates
	eCollisionType		eCollType;							// Set to enable collision detection
	bool				bFlagPhysicsEnabled;				// Set to enable physics (only with FlagCollision Enabled) - bounce, friction etc

	int					nFlagsCollision;					// Collision checking flags

	float				fAngle;
	float				fAngularSpeed;
	float				fAngularAccel;

	Vec3				pos, pos_last;
	Vec3				speed;
	Vec3				accel;
	
	bool				bContacting;						// Is it contacting?
	bool				bContactStarted;					// Tells you for a frame that the contact has started
	bool				bIsStatic;							// Did it completely stop?
	bool				bIsStaticZ;							// Did it stop on Z axis?
	bool				bIsDead;							// Tells if we must kill it as it exited the play area
	Vec3				contactNormal;						// Last contact normal
	Vec3				contactPos;							// Last contact pos
	CCollisionShape*	pContactShape;						// Contacting shape/type TODO

	float				fBounceF;							// bounce restitution factor
	float				fFrictionF;							// default 10.0f

	CPhysicsPoint() : eCollType(K_COLLTYPE_NONE), bContacting(false), bContactStarted(false), bIsStatic(false), nFlagsCollision(0),
		bFlagRotationEnabled(false), fAngle(0.0f), fAngularSpeed(0.0f), fAngularAccel(0.0f), bIsStaticZ(false),
		bFlagPhysicsEnabled(false), fBounceF(0.5f), fFrictionF(10.0f), bIsDead(false), pContactShape(NULL)
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



