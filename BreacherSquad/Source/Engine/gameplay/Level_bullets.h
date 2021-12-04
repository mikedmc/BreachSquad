#pragma once

///--------------------------------------------------------------------------
///--- BULLETS ---
///--------------------------------------------------------------------------
#define K_LVL_BULLETS_MAX_CNT 256
// hardcoded bullet height based on average main character height
#define K_BULLET_DEFAULT_Z			24.0f

#define K_LVL_BULLET_FLAG_NONE 0
#define K_LVL_BULLET_FLAG_IGNORE_COVER 1
#define K_LVL_BULLET_FLAG_IGNORE_ARMOR 2
//gloantele care pot sparge usile ce se deschid cu charges
#define K_LVL_BULLET_FLAG_BREAKS_DOORS 4
//flagul kill it now se seteaza cand vrei sa scapi de un glont
#define K_LVL_BULLET_FLAG_KILLITNOW 8
//flag de kill on impact chiar daca mai are energie 
#define K_LVL_BULLET_FLAG_DIE_ON_IMPACT 16
//sa nu faca efecte de particule unde loveste
#define K_LVL_BULLET_FLAG_NO_IMPACT_PARTICLES 32
//daca poate sa sparga inamicul in bucati
#define K_LVL_BULLET_FLAG_CAN_SPLAT 64
//daca explozia finala este directionala
#define K_LVL_BULLET_FLAG_DIRECTIONAL 128
//bullet isn't (classic) bullet so we don't count it in accuracy computation. It differentiates between grenades, melee and standard bullets
#define K_LVL_BULLET_FLAG_NOT_BALLISTIC 256
//melee "bullet"
#define K_LVL_BULLET_FLAG_MELEE 512
//don't add blood decals
#define K_LVL_BULLET_FLAG_NO_DECALS 1024
//gives critical hit if target is surprised
#define K_LVL_BULLET_FLAG_CRITICAL_IF_SCARED 2048
//can cripple target (add dot effect)
#define K_LVL_BULLET_FLAG_CAN_CRIPPLE 4096
//surgeon bullet on highlighted targets
#define K_LVL_BULLET_FLAG_SURGEON_IF_TARGETED 8192
//probability of critical shot when shot from behind
#define K_LVL_BULLET_FLAG_CRITICAL_FROM_BEHIND 16384
//no sound on impact
//#define K_LVL_BULLET_FLAG_ ... 32768

enum EBulletGroup
{
	K_LVL_BULLGROUP_NONE = -1,		//not set

	K_LVL_BULLGROUP_BULLETS = 0,
	K_LVL_BULLGROUP_THROWABLES,		//grenades 
	K_LVL_BULLGROUP_MELEE,
	K_LVL_BULLGROUP_SPECIAL,		//special ones like spy cameras and such
};

enum EBulletType
{
	K_LVL_BULLET_UNKNOWN = -1,

	K_LVL_BULLET_INVISIBLE = 0,		//invisible bullets that don't need drawing
	K_LVL_BULLET_DULL = 1,			//bullets that don't get actually shot
	K_LVL_BULLET_SHOTGUN,
	K_LVL_BULLET_SHOTGUN_INCENDIARY,
	K_LVL_BULLET_GRENADE,
	K_LVL_BULLET_GRENADE_ROUND,
	K_LVL_BULLET_FLASHBANG,
	K_LVL_BULLET_BREACHING_CHARGE,
	K_LVL_BULLET_CAM_BALL,
	K_LVL_BULLET_MOLOTOV,
	K_LVL_BULLET_MELEE,
	K_LVL_BULLET_MELEE_SAW,
	K_LVL_BULLET_TRACER1,  //linie continua pentru mitraliera
	K_LVL_BULLET_TRACER_AIMED_SHOT, //glont putere mare pentru aimed shot
	K_LVL_BULLET_TRACER_RECON, //special recont alt fire bullet
	K_LVL_BULLET_SHOTGUN_SLUG, //glont care trece prin mai multi
	K_LVL_BULLET_SMOKE_GRENADE, //folosita la spawning coleg, scoate fum o perioada
	K_LVL_BULLET_FIRE_JET,		//jet of fire
	K_LVL_BULLET_GOO,			//green goo

	K_LVL_BULLETS_COUNT
};

const CStringHash EBulletTypeNames[] =
{
	L"BULLET_INVISIBLE",
	L"BULLET_DULL",
	L"BULLET_SHOTGUN",
	L"BULLET_SHOTGUN_INCENDIARY",
	L"BULLET_GRENADE",
	L"BULLET_GRENADE_ROUND",
	L"BULLET_FLASHBANG",
	L"BULLET_BREACHING_CHARGE",
	L"BULLET_CAM_BALL",
	L"BULLET_MOLOTOV",
	L"BULLET_MELEE",
	L"BULLET_MELEE_SAW",
	L"BULLET_TRACER1",
	L"BULLET_TRACER_AIMED_SHOT",
	L"BULLET_TRACER_RECON",
	L"BULLET_SHOTGUN_SLUG",
	L"BULLET_SMOKE_GRENADE",
	L"BULLET_FIRE_JET",
	L"BULLET_GOO"
};

//	Structure used to return data from HitActor function
struct CBulletHitReturnData {
	float			fPointsTaken;
	EMaterialType	eMaterial;
	bool			bPenetratedShield;
	bool			bArmorHit;			// kills shotgun penetrating bullets
	bool			bKilledTarget;
};

class CBulletTemplate {
public:
	EBulletType		nType;			
	EBulletGroup	nGroup;			// bullet group (classic, grenade, etc hardcoded)

	float	fDamage;				// bullet total damage
	float	fDamageObjects;			// bullet damage for objects (mixed with BULLET_FLAG_CAN_BREAK_DOORS)
	float	fDamageLossPPx;			// cat damage pierde in functie de distanta parcursa in pixeli (0.0f - nu pierde din damage)
	float	fSelfDamageMultiplier;	// cat damage din damage-ul dat victimei se scade din damage-ul glontului (default 1.0f adica tot)
	float	fCriticalHitChance;		// how probable it is to do a critical hit
	float	fLife;					// total lifetime of the bullet (0.0 for infinite)
	float	fMomentum;				// speed * fMomentum transferred to enemy on impact
	float	fStunDuration;			// time it stuns enemies
	int		nArmorPiercingRating;	// AP class - bullet vs shield logic (see actor's ArmorRating)
	UINT32  nExploTemplateHash;		// Template of explosion if bullet explodes when dying
	UINT32	nFlags;					// misc flags from K_LVL_BULLET_FLAG_*

	float	fSpeed_ini;				// initial bullet speed
	EActorClass eClass;				// shooter's class


	CBulletTemplate() :
		fDamage(1.0f), fDamageObjects(0.0f), fDamageLossPPx(0.0f), fLife(1.0f), fMomentum(0.0f), nArmorPiercingRating(0),
		fStunDuration(0.0f), nFlags(K_LVL_BULLET_FLAG_NONE), nType(K_LVL_BULLET_UNKNOWN), nGroup(K_LVL_BULLGROUP_NONE),
		fSpeed_ini(0.0f), nExploTemplateHash(0), eClass(K_LVL_ACT_CLASS_ANY),
		fSelfDamageMultiplier(1.0f), fCriticalHitChance(0.0f)
	{
	}
};

class CBullet {
public:
	int					actorClass;				// shooter class
	UINT32				ownerUID;				
	UINT32				dwLastTargetUID;		// used in order to hit targets only once (penetrating) no matter the framerate
	CLevelArea*			pArea;					// current bullet area (collision optimization)

public:
	EBulletType			eType;					// bullet type: rocket, grenade, etc
	float				fDamage;				// current damage
	float				fDamage_ini;			// initial damage (readonly please)
	float				fDamageLossPPx;			// damage lost per pixel of movement (0.0f - no damage lost)
	float				fSelfDamageMultiplier;	// damage subtracted from bullet when it hits enemy (default 1.0f - loses all)
	float				fCriticalHitChance;		// critical hit chance (0..1)

	float				fLife;					//cat timp traieste
	float				fLife_ini;				//initial fLife value (readonly please)

	float				fMomentum;				//cat procent din viteza imprima in viteza inamicului
	float				fStunDuration;			//face stun?
	int					nArmorPiercingRating;	//AP class - bullet vs shield logic (see actor's ArmorRating)
	UINT32				nExploTemplateHash;		//hash of explosion template at the end or 0 if none

	VecProjPhys			posV;					// position and speed (used by collision components)

	UINT32				nFlags;					// misc flags
	int					nSubstate;				// bullet state used by some bullet types

	CLinkedPool<CPhysicsPoint>::CLinkedPoolNode *physPt; // attached PhysPoint for collision detection and bounce
	
	CSpr				sprBullet;				// bullet sprite
	bool				bAnimated;				// Sets itself when initialized. If animation is looped then we consider it to be animated.
	scFrameID			fidLight;				// anim and frame for the light sprite

	CBullet() : eType(K_LVL_BULLET_SHOTGUN), fDamage(1.0f), fDamage_ini(1.0f), fDamageLossPPx(0.0f),
		fLife(1.0f), fLife_ini(1.0f), actorClass(K_LVL_ACT_CLASS_PLAYER), nSubstate(0),
		fMomentum(0.0f), nFlags(0), fStunDuration(0.0f), ownerUID(0), dwLastTargetUID(0),
		nArmorPiercingRating(0), nExploTemplateHash(0), fSelfDamageMultiplier(1.0f), fCriticalHitChance(0.0f),
		bAnimated(false), pArea(nullptr)
	{
		physPt = nullptr;
	}
};


