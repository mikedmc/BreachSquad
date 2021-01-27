#pragma once

// declare used classes
class CCollisionShape;


///--------------------------------------------------------------------------
/// PHYSICS POINTS
///--------------------------------------------------------------------------

// calculeaza deplasare si coliziuni cu nivelul
///--- collision flags ---
//colizioneaza cu solide?
#define K_LVL_PHYSP_COLLFLAG_SOLID	1
//colizioneaza cu boxes?
#define K_LVL_PHYSP_COLLFLAG_BOX	2
//interactioneaza cu apa ? - daca pluteste sau alte efecte
#define K_LVL_PHYSP_COLLFLAG_WATER	4

class CPhysicsPoint2D {
public:
	enum eCollisionType {
		K_COLLTYPE_NONE = 0,	//no collision
		K_COLLTYPE_FAST,		//fast - collides with visible collshapes
		K_COLLTYPE_PRECISE,		//precise - collides with ALL collshapes (slower)
	};
	//flaguri de control
	bool		bFlagRotationEnabled;			//Set to enable rotation updates
	eCollisionType	eCollType;					//Set to enable collision detection
	bool		bFlagPhysicsEnabled;			//Set to enable physics (only with FlagCollision Enabled)

	int			nFlagsCollision;  //flagurile de coliziune (cu ce boxes colizioneaza)

	float		fAngle;
	float		fAngularSpeed;
	float		fAngularAccel;

	Vec2 pos, pos_last;
	Vec2 speed;
	Vec2 accel;
	//date frecare si bounce
	bool  bContacting; //spune daca este in contact
	bool  bContactStarted; //spune cand s-a initiat contactul, doar pentru un frame. Se poate pune sunet in fn de el
	bool  bIsStatic;	//spune daca nu se mai misca
	bool  bIsDead;		//spune daca e mort (afara din zona de joc)
	Vec2 contactNormal;  //normala ultimului contact
	Vec2 contactPos;		//pozitia ultimului contact
	CCollisionShape* pContactShape; //collision shape-ul cu care face contact

	float fBounceF;   //bounce restitution factor
	float fFrictionF; //default 10.0f

	CPhysicsPoint2D() : eCollType(K_COLLTYPE_NONE), bContacting(false), bContactStarted(false), bIsStatic(false), nFlagsCollision(0),
		bFlagRotationEnabled(false), fAngle(0.0f), fAngularSpeed(0.0f), fAngularAccel(0.0f),
		bFlagPhysicsEnabled(false), fBounceF(0.5f), fFrictionF(10.0f), bIsDead(false), pContactShape(NULL)
	{
		pos = pos_last = Vec2(0.0f, 0.0f);
		speed = Vec2(0.0f, 0.0f);
		accel = Vec2(0.0f, 0.0f);
		contactNormal = Vec2(0.0f, 0.0f);
		contactPos = Vec2(0.0f, 0.0f);
	};

	void Init();
	/*
	 * Forces a new position for the point
	 */
	void SetPosForced(Vec2 vecPos);
};



