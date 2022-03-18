#pragma once

#include "LevelDefines.h"
#include "components/SpriteAnimComp.h"

// max no of anim sets
#define K_ACT_ANIM_MAX_SETS 2
// max no of verse sets
#define K_ACT_VERSES_MAX_SETS 2
// max number of skins
#define K_ACT_SKINS_MAX_SETS 20

// suspend flags used on actor->nSuspendedFlag 
#define K_LVL_SUSPENDFLAG_NONE 0
// player suspended - fallen offscreen
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

	// descriptor for skins array
	struct CSkinDesc {
		CStringHash			name;
		DWORD				layersVisMask;			// layer visibility bit mask that gets applied (less important bit is layer index 0)
		DWORD				hand2Mask;				// secondary hand layer mask
		DWORD				hand1Mask;				// main hand layer mask

		CSkinDesc() : layersVisMask( 0xffffffff ), hand2Mask( 0 ), hand1Mask( 0 )
		{}
	};

	// Animations descriptor (keeps animation names for each of the 6 angles, for every set)
	// We only use N, NE, SE, S and flip them for the left quadrant but still we try and load all directions
	struct CAnimDesc {
		// Holds animation names [animSet][angle]
		CStringHash			animNamesA[ K_ACT_ANIM_MAX_SETS ][ EDIR6S_CNT ];

		CAnimDesc()
		{
			Reset();
		};

		void Reset()
		{
			for ( int sets = 0; sets < K_ACT_ANIM_MAX_SETS; sets++ )
				for ( int ang = 0; ang < EDIR6S_CNT; ang++ )
					animNamesA[ sets ][ ang ].Reset(); //not set
		}
	};

	CStringHash		shID;				// ID: actor template file used as template ID
	CStringHash		shSourceXML;		// skeleton xml file name (not full path)
	CStringHash		shAIState_ini;		// initial AI state

	CSkinDesc		arrSkins[ K_ACT_SKINS_MAX_SETS ];								// Array that contains skin names and descriptions
	int				arrSkinsCnt;
	CAnimDesc		arrAnims[ K_ACT_ANIMS_CNT ];									// Array that keeps animation data from actor.xml
	int				soundIDs[ K_LVL_ACT_VERSES_COUNT ][ K_ACT_VERSES_MAX_SETS ];	// Contains sound ids-s mapped on different actions (called verses, see EActorSoundVerse)

	///--- GENERICS: !!! when adding new generics don't forget to edit OverwriteGenericDataFromTemplate !!!
	EMaterialType	eMaterial;			// type of material
	EActorClass		actorClass;			// class of actor
	CAITemplate*	AItemplate;

	UINT32			eCaps;				// see EActorCapabilitiesFlags

	float			fMass;
	float			fLife;
	float			fArmor;
	float			fSpeedMove;

	CAABB			bbox;				// 2d bbox on floor plane defined around the character origin (not always centered)
	float			fHeight;			// height of character
	CStringHash		shWeaponDefault;

	CActorTemplate();

	// Fills variables with default values if not set (some templates are missing values)
	// \brief: We need this because of the upgrade templates that must have a lot of values on K_NOT_SET (0xDEADBEEF)
	void FillDefaultValuesIfNotSet();

	// Overwrites the current animations with the ones that are set in pTemplate
	// \returns: true if animations have been changed
	bool OverwriteAnimsFromTemplate( CActorTemplate* pTemplate, bool bEraseOldAnimations = false );

	//Overwrites "generics" with the ones that are set in pTemplate (only if not K_NOT_SET)
	void OverwriteGenericDataFromTemplate( CActorTemplate* pTemplate );

	//Adds "GENERIC" data from pTemplate to current template (weapon upgrades and such)
	void AddGenericDataFromTemplate( CActorTemplate* pTemplate );
	
	// returns skin layer visibility flag or 0xffffffff if skin not found
	UINT32 GetSkinMaskValue( WCHAR* skinName );
};


class CActor : public IActiveInterface
{
	///--- COMPONENTS --- 
	/// Pointer components get deallocated by the actor, referenced ones are global so we don't touch them:
private:
	CSpriteAnimComponent*		c_graphics;		//graphics component that handles all painting and animation stuff
public:
	//ce info primeste de la senzori
	class CAISensorInfo
	{
	public:
		bool		m_bEnabled;			// sensors are enabled or disabled?
		//external sensors
		CActor*		pTargetedActor;		//inamicul vizibil
		CAIEvent	m_AIcurrentEvent;	//eventul curent, cel mai actual. Se salveaza si in lastAIevent automat.
		UINT32		m_lastInteractingActorUID;	//0-not set or UID for last actor that he interacted with
		float		fTimeSinceHit;		//time passed since got hit
		//internal sensors
		bool		b_IsDead;			// did I die?

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
			fTimeSinceHit = 1000.0f;

			m_AIlastEvent.Reset();
			m_AIcurrentEvent.Reset();
		}
	};
	//ce comenzi trimite AI-ul mai departe
	class CAICommands
	{
	public:
		bool				bThrust;  //#TODO: thrust might as well be a float (low precision float) and remove bRunning
		Vec2				vMoveDir;
		Vec2				vAimVec;

		bool				bRunning;
		bool				bCrouched;
		bool				bJump;
		bool				bInteract;				// interact command
		//EActorAnims			eOverrideAnim;	//if not empty, overrides actor animation

		EActorDeathCommand	nDeathCommand; //0-not dead, 1-dead, 2-splat, 3-splat+explode
		EActorAttackState	eAttackCommand;
		EActorAttackState	eAttackCommand_last; //last attack command
		//set icon commands
		EActorIconTypes		nIconType;
		float				fIconDuration;
		//color command: !=0 means color command is active
		DWORD				nColor;

		CAICommands()
		{
			Reset();
		}

		void Reset()
		{
			bThrust = false;
			vMoveDir = Vec2( 0.0f, 0.0f );

			bRunning = false;
			vAimVec = Vec2( 0.0f, 0.0f );

			bCrouched = false;
			bJump = false;
			bInteract = false;
			nColor = 0;

			nDeathCommand = K_LVL_ACT_DEATHCMD_NONE;
			eAttackCommand = K_LVL_ACT_ATTACK_IDLE;
			eAttackCommand_last = K_LVL_ACT_ATTACK_IDLE;
			//eOverrideAnim = K_LVL_ACT_ANIM_EMPTY;

			nIconType = K_LVL_ACT_ICON_NONE;
			fIconDuration = 0.0f;
		}

		void ResetMoveCommands()
		{
			bThrust = false;
			bRunning = false;
			vAimVec = Vec2( 0.0f, 0.0f );

			bCrouched = false;
			bJump = false;
			bInteract = false;
			eAttackCommand = K_LVL_ACT_ATTACK_IDLE;
		}
	};


public:
	CActorTemplate			actTemplate;				// holds data about each actor, copied from source templates (xml) and probably modified by enhancements
	CActorTemplate			actTemplate_ini;			// holds initial template that we reset to when changing the weapon or adding non permanent enhancements

	EActorSoundVerse		eLastPlayedVerse;			//last played sound verse
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

	Vec3		pos_last;		// position on last frame
	Vec2		speed;
	Vec2		vSpeedImpulse;	//viteza aplicata extern (cand e impuscat de exemplu). Se va atenua automat.

	Vec2		vMoveDirN;		//normalized movement direction

	EDir6	eAngle;			// animation angle (6 possible ways)
	bool		bAnimFlipX;		// do we need to flip the animation on X?

	float		fLife, fArmor; //cata viata are si cata armura
	float		fFOVPercent;   //field of view-ul personajului, intre 0 si 1 => 0.5 va fi FOV de 90 de grade. Reprezinta un fel de alertLevel si seteaza si hearing range
	//flags
	bool		bHasGravity; //daca are gravitatie

	CDamageOverTime		cDamageOverTime;	//#TODO: de schimbat in ceva si cu conotatii pozitive (healing, etc)
	EActorAttackState	nAttackStatus; //aici este statusul legat de arma (shooting, shootalt, reloading, etc)

	bool		bCrouched; //daca este crouched
	eGenericState	nRolling; //0-ready, 1-rolling, 2-finished and waiting reset (direction key up)
	//CMiscObjectRail* pRail;		//pointer catre un rail atunci cand merge pe tiroliana

	float		fStunTimer; //daca e diferit de 0 personajul este stunned
	UINT32		nLastDamageTakenFromUID;	//UID that gave actor last damage 

	UINT32		nSuspendedFlags;	//daca e suspended (AI), folosit de obicei la playerii principali ca sa le tai inputul (cand ies din ecran sau cand vrei sa nu poti sa-i mai controlezi). Flagul specifica motivul
	float		fSuspendedTimer;	//counts from when suspended flags is set
	bool		bSuspendInput;		//if set keyboard input is ignored

	void SetIcon( EActorIconTypes iconType, float fDuration = 0.0f ); //seteaza icon

	IActiveInterface*			pClosestTouchable;			// currently focused interactible object
	eGenericState				eInteractState;				// state of interaction (NOTSET=not interacting, READY-selecting action, EXECUTING-started action, FINISHED-interact finished)
	int							nInteractOptionsSelIdx;		// index in arrInteractOptions
	CFixedArray<CScriptAction, 16>	arrInteractOptions;		// empty when not interacting. gathers all interaction options from object, character feats, inventory objects, etc

	CArray<CWeapon*>			arrWeapons;		// Collection of weapons available for current actor
	CWeapon*					pWeaponMain;	// currently selected main weapon (points to arrWeapons)
	// Adds a weapon to actor's arsenal
	void						AddWeapon( CWeapon* wpn, bool bEquip );
	// Equips weapon from arsenal
	void						EquipWeapon( int nWeaponIdx );

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
	CActor( Vec2 vnPos, CActorTemplate* pActorTemplate, int nID, CSpriteAnimComponent* pComGraphics );
	~CActor();

	const eActiveInterfaceType GetClassType() const {
		return K_LVL_IAI_TYPE_ACTOR;
	}

	// tells if actor is alive and not hidden or deallocated, or inactive
	bool					IsAlive();

	void SetPos( Vec3 newPos ) override;
	void Move( Vec3 delta ) override;

	// Updates specified Actor AI. Returns busy state TRUE if actor has jobs to do or false if actor is still
	void					Update( float dTime );
	// Paints the actor on a specific color channel
	FORCEINLINE void		Paint( ETexChannel eChannel = K_TEXCHAN_COLORMAP ) { c_graphics->Paint( *this, eChannel ); };
	// sets graphics anim set
	void					SetAnimSet( int n_anim_set ) { c_graphics->SetAnimSet( n_anim_set ); }
	// Plays the actor verse from the template handling the positional attenuation
	void					PlaySoundVersePos( D3DXVECTOR2 vListenerPos, EActorSoundVerse sVerse, bool bPlayIfNotPlayingOnly = false );
	// Equips specified weapon and sets template
	void					EquipWpn( CWeapon * pWeapon );
	// Sets the actor's weapon and template upgrades and limitations generated by the weapon, adding them to actor initial template (after spawning, without weapons)
	void					AddWpnTemplate( CWeapon * pWeapon );
	// Updates possible actions list when interacting with something
	// Looks into the inventory, the touchable and the actor specs/template for specific actions
	void					BuildActionsList();
	// Clears the current actions list
	void					ClearActionsList();

	void PostConstructionInit() override;
	void BeginPlay() override;
	void EndPlay() override;

private:
	// Initializes CActor with specified template and sets all the data it needs. Returns false if it fails
	bool					InitFromTemplate( CActorTemplate * pActorTemplate );
};
