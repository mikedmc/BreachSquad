#pragma once

// max no of anim sets
#define K_ACT_ANIM_MAX_SETS 2
// max no of verse sets
#define K_ACT_VERSES_MAX_SETS 2
// max no of animation tracks for the actors
#define K_ACT_MAX_ANIM_TRACKS 2

///--------------------------------------------------------------------------
///--- ACTORS : clasa principala de inamici si personaje player
///--------------------------------------------------------------------------

//--> suspend flags used on actor->nSuspendedFlag 
#define K_LVL_SUSPENDFLAG_NONE 0
//player suspended - fallen offscreen
#define K_LVL_SUSPENDFLAG_OUTSIDE_SCREEN 1


class CActorTemplate
{
public:
	enum EActorCapabilitiesFlags {
		K_ACT_CAPS_NONE = 0,
		K_ACT_CAPS_CAN_COVER = 1,
		K_ACT_CAPS_CAN_INTERACT,
		K_ACT_CAPS_CAN_ROLL,
		K_ACT_CAPS_CAN_CROUCH,
		//misc flags
		K_ACT_CAPS_NOT_A_TARGET = 128,		// Literally not a target
	};

	// Animations descriptor
	struct CAnimDesc {
		bool				bLooping;
		CStringHashA		animNamesA[K_ACT_ANIM_MAX_SETS];

		CAnimDesc() : bLooping(true)
		{};

		void Reset()
		{
			bLooping = true;
			for (int kk = 0; kk < K_ACT_ANIM_MAX_SETS; kk++)
			{
				animNamesA[kk].Reset();
			}
		}
	};

	CStringHash		shID;				// ID: actor template file used as template ID
	CStringHash		shSkeletonXML;		// skeleton xml file name (not full path)
	CStringHashA	shSkinName;			// skeleton skin name
	CStringHash		shAIState_ini;		// initial AI state

	CAnimDesc		arrAnims[K_SD_ANIMS_CNT];										// Array that keeps animation data from actor.xml
	int				soundIDs[K_LVL_ACT_VERSES_COUNT][K_ACT_VERSES_MAX_SETS];	// Contains sound ids-s mapped on different actions (called verses, see EActorSoundVerse)

	///--- GENERICS: !!! when adding new generics don't forget to edit OverwriteGenericDataFromTemplate !!!
	EMaterialType	eMaterial;	//type of material
	EActorClass		actorClass; //class of actor
	CAITemplate*	AItemplate;	

	UINT32			eCaps;				//see EActorCapabilitiesFlags

	float			fMass;
	float			fLife;		
	float			fArmor;		
	float			fSpeedMove;

	CAABB			aabb;
	float			fHeight;
	float			fShootHeight;
	CStringHash		shWeaponDefault;

	CActorTemplate();

	// Fills variables with default values if not set (some templates are missing values)
	// \brief: We need this because of the upgrade templates that must have a lot of values on K_NOT_SET (0xDEADBEEF)
	void FillDefaultValuesIfNotSet();

	// Overwrites the current animations with the ones that are set in pTemplate
	// \returns: true if animations have been changed
	bool OverwriteAnimsFromTemplate(CActorTemplate* pTemplate, bool bEraseOldAnimations = false);

	//Overwrites "generics" with the ones that are set in pTemplate (only if not K_NOT_SET)
	void OverwriteGenericDataFromTemplate(CActorTemplate* pTemplate);

	//Adds "GENERIC" data from pTemplate to current template (weapon upgrades and such)
	void AddGenericDataFromTemplate(CActorTemplate* pTemplate);

};


class CActor : public IActiveInterface
{
public:
	//ce info primeste de la senzori
	class CAISensorInfo
	{
	public:
		bool		m_bEnabled;			//senzorii sunt enabled sau disabled
		//external sensors
		CActor*		pTargetedActor;		//inamicul vizibil
		float		fTargetOverlapX;	//#HACK: cu cat se suprapune peste target (ca sa pot sa-l imping in spate)
		CAIEvent	m_AIcurrentEvent;	//eventul curent, cel mai actual. Se salveaza si in lastAIevent automat.
		UINT32		m_lastInteractingActorUID;	//0-not set sau UID pentru ultimul actor cu care a interactionat
		float		fTimeSinceHit;		//time passed since got hit
		//internal sensors
		bool		b_IsDead;			//daca a murit

		//sensor memory
		CAIEvent	m_AIlastEvent;		//eventul cel mai important, ultimul primit. Asta este memoria actorului, deci raman setate pentru o durata mai mare sau pana cand sunt suprascrise

		CAISensorInfo()
		{
			Reset();
		}

		void Reset()
		{
			pTargetedActor = nullptr;
			b_IsDead = false;
			m_lastInteractingActorUID = 0;
			m_bEnabled = true;
			fTargetOverlapX = 0.0f;
			fTimeSinceHit = 1000.0f;

			m_AIlastEvent.Reset();
			m_AIcurrentEvent.Reset();
		}
	};
	//ce comenzi trimite AI-ul mai departe
	class CAICommands
	{
	public:
		bool				bThrust;
		Vec2			vMoveDir;
		Vec2			vAimVec;		

		bool				bRunning;	//daca alearga
		bool				bThrustX;	//should be a float (0..1) to replace bRunning
		bool				bCrouched;	//daca este crouch sau nu
		bool				bJump;		//comanda de jump
		bool				bClimb;		//comanda sa se catere
		int					nInteractKeyState;  //stare buton interact (just pressed, not pressed etc)
		int					nMoveDirX;	//directia de miscare ca si flaguri (-1,0,1)
		int					nMoveDirY;	//directia de miscare ca si flaguri (-1,0,1)
		EActorAnims			eOverrideAnim;	//if not empty, overrides actor animation

		EActorDeathCommand	nDeathCommand; //0-not dead, 1-dead, 2-splat, 3-splat+explode
		EActorAttackState	eAttackCommand;
		EActorAttackState	eAttackCommand_last; //last attack command
		//set icon commands
		EActorIconTypes		nIconType;
		float				fIconDuration;	//daca setez
		//color command: !=0 means color command is active
		DWORD				nColor; 

		CAICommands()
		{
			Reset();
		}

		void Reset()
		{
			bThrust = false;
			vMoveDir = Vec2(0.0f, 0.0f);
			
			bThrustX = false;
			bRunning = false;
			nMoveDirX = 0;
			nMoveDirY = 0;
			vAimVec = Vec2(0.0f, 0.0f);

			bCrouched = false;
			bJump = false;
			bClimb = false;
			nInteractKeyState = K_CM_BUTSTATE_NOTPRESSED;
			nColor = 0;

			nDeathCommand = K_LVL_ACT_DEATHCMD_NONE;
			eAttackCommand = K_LVL_ACT_ATTACK_IDLE;
			eAttackCommand_last = K_LVL_ACT_ATTACK_IDLE;
			eOverrideAnim = K_LVL_ACT_ANIM_EMPTY;

			nIconType = K_LVL_ACT_ICON_NONE;
			fIconDuration = 0.0f;
		}

		void ResetMoveCommands()
		{
			bThrustX = false;
			bRunning = false;
			nMoveDirX = 0;
			nMoveDirY = 0;
			vAimVec = Vec2(0.0f, 0.0f);

			bCrouched = false;
			bJump = false;
			bClimb = false;
			nInteractKeyState = K_CM_BUTSTATE_NOTPRESSED;
			eAttackCommand = K_LVL_ACT_ATTACK_IDLE;
		}
	};

	// Animation pointers buffer - keeps animation in sync with actor template declared animations
	struct CAnimPtr {
		CStringHashA		animNamesA[K_ACT_ANIM_MAX_SETS];
		spine::Animation*	pAnim[K_ACT_ANIM_MAX_SETS];
		bool				bLooping;

		CAnimPtr() : bLooping(true)
		{
			for (int kk = 0; kk < K_ACT_ANIM_MAX_SETS; kk++)
			{
				pAnim[kk] = null;
				animNamesA[kk].Reset();
			}
		}
	};

public:
	CSpineManager::CSkeletonTemplate*		pSkelTemplate;	// Pointer to the skeleton template (don't deallocate, managed)
	CSpineManager::CSkeletonInstance*		pSkeleton;		// Pointer to the skeleton instance (don't deallocate, managed)
	CAnimPtr								arrAnimsPtr[K_SD_ANIMS_CNT]; // Direct pointer structure to animations declared in actor template (rarely updated)

	CActorTemplate			actTemplate;				//datele generale din XML copiate in fiecare actor, datele curente
	CActorTemplate			actTemplate_ini;			//datele initiale, imediat dupa loading si dupa customizarea initiala
	EActorAnims				eLastAnimSet, eLastAnimSet_feet;		//ultima animatie setata  pe actor prin SetActorAnimOnce() (torso si feet)

	EActorSoundVerse		eLastPlayedVerse;			//last played sound verse
	ESpineAnim				eLastAnim[K_ACT_MAX_ANIM_TRACKS];	// Last anim set with SetActorAnimOnce() (torso and feet)
	float					fVerseCooldown;				//don't play the same verse if cooldown > 0.0f
	int						nLastPlayedVerseSndIdx;		//last played sound idx

	int						nAnimSet;	//current animation set (-1 for RANDOM); Don't set directly!

public:
	//collision
	bool		bHasCollision;			//se calculeaza coliziunea cu nivelul
	UINT16		collisionFlags;		    //iti spune in ce directii are coliziune (K_DIRFLAG_)

	Vec2 GetPosHeart(); //intoarce pozitia exacta a inimii in fn de starea curenta
	Vec3 GetPosWeapon();	//intoarce pozitia exacta a armei in fn de starea curenta

	Vec3	posHeart, posWeapon;	//pozitii absolute inima si arma presalvate (pentru viteza)

	Vec2		pos_last;		// position on last frame
	Vec2		speed;
	Vec2		vSpeedImpulse;	//viteza aplicata extern (cand e impuscat de exemplu). Se va atenua automat.

	Vec2		vMoveDirN;		//normalized movement direction

	EAnimAngle	eAnimAngle;		// animation angle (6 possible ways)
	bool		bAnimFlipX;		// do we need to flip the animation on X?

	float		fLife, fArmor; //cata viata are si cata armura
	float		fFOVPercent;   //field of view-ul personajului, intre 0 si 1 => 0.5 va fi FOV de 90 de grade. Reprezinta un fel de alertLevel si seteaza si hearing range
	//flags
	bool		bHasGravity; //daca are gravitatie

	CDamageOverTime		cDamageOverTime;	//daca are efect de damage/heal over time
	EActorAttackState	nAttackStatus; //aici este statusul legat de arma (shooting, shootalt, reloading, etc)

	byte 		nInteractingState;	//0 - not interacting, 1 interacting, 2 lockpicking
	bool		bCrouched; //daca este crouched
	eGenericState	nRolling; //0-ready, 1-rolling, 2-finished and waiting reset (direction key up)
	CCollisionShape* pCover; //daca e diferit de null inseamna ca pe langa crouched e si covered
	//CMiscObjectRail* pRail;		//pointer catre un rail atunci cand merge pe tiroliana

	float		fStunTimer; //daca e diferit de 0 personajul este stunned
	int			nTookDamageFrames; //flag folosit pentru a desena frames aprinse cand lovesti inamic
	UINT32		nLastDamageTakenFromUID;	//UID that gave actor last damage 
	BYTE		nSkinIdx;					//player skin

	UINT32		nSuspendedFlags;	//daca e suspended (AI), folosit de obicei la playerii principali ca sa le tai inputul (cand ies din ecran sau cand vrei sa nu poti sa-i mai controlezi). Flagul specifica motivul
	float		fSuspendedTimer;	//counts from when suspended flags is set
	bool		bSuspendInput;		//if set keyboard input is ignored

	void SetIcon(EActorIconTypes iconType, float fDuration = 0.0f); //seteaza icon

	IActiveInterface*	pClosestTouchable; //cel mai apropiat activ cu can interact

	CSprite		m_sprOverheadIcon;	//icon shown when interacting with things (doors, objects) or in other circumstances too

	// Sets a Spine skin and returns true if successfull
	bool					Spine_SetSkin(const char * strSkinName);
	// Tells you if the actor has said animation 
	bool					HasAnimation(ESpineAnim nAnimType, int nSet = 0);

	// Sets current animation set (changes immediately)
	void					SetAnimSet(int newAnimSet);
	FORCEINLINE int			GetAnimSet() const { return nAnimSet; }

	CGrowableArray<CWeapon*>	arrWeapons;		// Collection of weapons available for current actor
	CWeapon*					pWeaponMain;	// currently selected main weapon (points to arrWeapons)
	// Adds a weapon to actor's arsenal
	void						AddWeapon(CWeapon* wpn, bool bEquip);
	// Equips weapon from arsenal
	void						EquipWeapon(int nWeaponIdx);
	
	//player control and controller data
	int			nPlayerOrdinal;	//player index (0-max_players_cnt)
	int			nControllerInstanceID; //player controller ID (-1 for empty)

	EActorIconTypes nIconType;
	float			fIconTimer;

	CAISensorInfo	m_AIsensorInfo;	// AI sensory information
	CAICommands		m_AIcommands;	// Commands issued by AI

	CAIState*		m_pAIcurrentState; 
	int				m_nAIcurrentBehaviorIdx; // current behaviour index (in current state) or -1 when not set
	float			m_fAIbehaviorTimer;		// timer used for timed behaviors
	EAIBehaviorType GetCurrentBehavior();	

	//CTOR
	CActor(Vec2 vPos, CActorTemplate* pActorTemplate, int nID = -1);
	~CActor();

	const eActiveInterfaceType GetClassType() const {
		return K_LVL_IAI_TYPE_ACTOR;
	}

	void SetPos(Vec2 newPos) override;
	void Move(Vec2 delta) override;
	void SetAngle(float fNewAngle) override; //seteaza unghiul si vAngleDir

	// Updates specified Actor AI. Returns busy state TRUE if actor has jobs to do or false if actor is still
	void					Update(float dTime);
	// Sets pointers to spine animations from skeleton template (optimization)
	void					Spine_SaveAnimPointers();
	// Set actor animation checking if not already set
	spine::TrackEntry*		SetAnimOnce(int nTrack, ESpineAnim eAnim);
	// Adds an animation to a track
	spine::TrackEntry*		AddAnimOnce(int nTrack, ESpineAnim eAnim, float fMixTime = K_SM_DEFAULT_MIX_DURATION, float fDelay = 0.0f);
	// Plays the actor verse from the template handling the positional attenuation
	void					PlaySoundVersePos(D3DXVECTOR2 vListenerPos, EActorSoundVerse sVerse, bool bPlayIfNotPlayingOnly = false);
	// Equips specified weapon and sets template
	void					EquipWpn(CWeapon * pWeapon);
	// Sets the actor's weapon and template upgrades and limitations generated by the weapon, adding them to actor initial template (after spawning, without weapons)
	void					AddWpnTemplate(CWeapon * pWeapon);


	void PostConstructionInit() override;
	void BeginPlay() override;
	void EndPlay() override;

private:
	// Initializes CActor with specified template and sets all the data it needs. Returns false if it fails
	bool					InitFromTemplate(CActorTemplate * pActorTemplate);
};
