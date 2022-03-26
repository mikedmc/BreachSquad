#pragma once

#include "LevelDefines.h"
#include "ActorTypes.h"
#include "ActorTemplate.h"
#include "components/SpriteActorComp.h"
#include "components/WeaponsComp.h"

// suspend flags used on actor->nSuspendedFlag 
#define K_LVL_SUSPENDFLAG_NONE 0
// player suspended - fallen offscreen
#define K_LVL_SUSPENDFLAG_OUTSIDE_SCREEN 1

class CActor : public IActiveInterface
{
	///--- COMPONENTS --- 
	/// Pointer components get deallocated by the actor, referenced ones are global so we don't touch them:
private:
	CSpriteActorComponent*	c_graphics;					// graphics component that handles all painting and animation stuff
	CWeaponsComponent*		c_weapons;					// graphics and logic component that handles the weapons

public:
	CActorTemplate			_template;				// holds data about each actor, copied from source templates (xml) and probably modified by enhancements
	CActorTemplate			_template_ini;			// holds initial template that we reset to when changing the weapon or adding non permanent enhancements

	//#TODO: convert to verse/voice component:
	EActorSoundVerse		eLastPlayedVerse;			//last played sound verse
	float					fVerseCooldown;				//don't play the same verse if cooldown > 0.0f
	int						nLastPlayedVerseSndIdx;		//last played sound idx

public:
	VecProj					vHeart;						// 3d heart position used for visibility tests and weapon direction
	UINT16					collisionFlags;				// tells us where we have collisions
	bool					bHasGravity;				// does it have gravity?

	Vec2 GetPosHeart(); //intoarce pozitia exacta a inimii in fn de starea curenta
	Vec3 GetPosWeapon();	//intoarce pozitia exacta a armei in fn de starea curenta


	Vec3		posHeart, posWeapon;	//pozitii absolute inima si arma presalvate (pentru viteza)
	Vec3		pos_last;		// position on last frame
	Vec2		speed;
	Vec2		vSpeedImpulse;	//viteza aplicata extern (cand e impuscat de exemplu). Se va atenua automat.
	Vec2		vMoveDirN;		//normalized movement direction

	float		fLife, fArmor; //cata viata are si cata armura
	float		fFOVPercent;   //field of view-ul personajului, intre 0 si 1 => 0.5 va fi FOV de 90 de grade. Reprezinta un fel de alertLevel si seteaza si hearing range

	CDamageOverTime		cDamageOverTime;	//#TODO: de schimbat in ceva si cu conotatii pozitive (healing, etc)
	EActorAttackState	nAttackStatus; //aici este statusul legat de arma (shooting, shootalt, reloading, etc)

	bool			bCrouched; //daca este crouched
	eGenericState	nRolling; //0-ready, 1-rolling, 2-finished and waiting reset (direction key up)
	//CMiscObjectRail* pRail;		//pointer catre un rail atunci cand merge pe tiroliana

	float		fStunTimer; //daca e diferit de 0 personajul este stunned
	UINT32		nLastDamageTakenFromUID;	//UID that gave actor last damage 

	UINT32		nSuspendedFlags;	//daca e suspended (AI), folosit de obicei la playerii principali ca sa le tai inputul (cand ies din ecran sau cand vrei sa nu poti sa-i mai controlezi). Flagul specifica motivul
	float		fSuspendedTimer;	//counts from when suspended flags is set
	bool		bSuspendInput;		//if set keyboard input is ignored

	IActiveInterface*			pClosestTouchable;			// currently focused interactible object
	eGenericState				eInteractState;				// state of interaction (NOTSET=not interacting, READY-selecting action, EXECUTING-started action, FINISHED-interact finished)
	int							nInteractOptionsSelIdx;		// index in arrInteractOptions
	CFixedArray<CScriptAction, 16>	arrInteractOptions;		// empty when not interacting. gathers all interaction options from object, character feats, inventory objects, etc

	//player control and controller data
	int			nPlayerOrdinal;	//player index (0-max_players_cnt)
	int			nControllerInstanceID; //player controller ID (-1 for empty)

	CAISensorInfo	m_AIsensorInfo;	// AI sensory information
	CAICommands		m_AIcommands;	// Commands issued by AI

	CAIState*		m_pAIcurrentState;
	int				m_nAIcurrentBehaviorIdx; // current behaviour index (in current state) or -1 when not set
	float			m_fAIbehaviorTimer;		// timer used for timed behaviors
	EAIBehaviorType GetCurrentBehavior();

	// CTOR. Allocate the components when calling the constructor. They will get deallocated by CActor.
	CActor( Vec2 vnPos, CActorTemplate* pActorTemplate, int nID, CSpriteActorComponent* pComGraphics, CWeaponsComponent* pComWpn );
	// DTOR
	~CActor();

	inline const eActiveInterfaceType	GetClassType() const { return K_LVL_IAI_TYPE_ACTOR; }
	// Gets pointer to weapons component
	inline CWeaponsComponent*			Weapons() { return c_weapons; }

	// tells if actor is alive and not hidden or deallocated, or inactive
	bool					IsAlive();

	void					SetPos( Vec3 newPos ) override;
	void					Move( Vec3 delta ) override;

	// Updates specified Actor AI. Returns busy state TRUE if actor has jobs to do or false if actor is still
	void					Update( float dTime );
	// Paints the actor on a specific color channel
	void					Paint( ETexChannel eChannel = K_TEXCHAN_COLORMAP );
	// returns aim vector
	inline Vec2				GetAimVec() { return m_AIcommands.vAimVec; };
	// returns animation direction on X (flipped or not, decided by the graphics component)
	inline int				GetVisualFlipDirX() { return c_graphics->GetFlipDirX(); }
	// Returns the gun mount position. bDualHanded tells if we are looking for the dual handed mount or single handed mount.
	// mountIndex is usually used when looking for the position of single handed weapons in dualWield mode (hand on the left)
	VecProj					GetWeaponMountWorld( bool bTwoHanded, int mountIndex = 0 );
	// Returns the gun muzzle position in World coordinates.
	// \param mountIndex - 0 for most weapons, 1 for second weapon in dual wielding stances
	VecProj					GetWeaponMuzzleWorld( bool bTwoHanded, int mountIndex /*= 0 */ );
	// sets graphics anim set
	void					SetAnimSet( int n_anim_set ) { c_graphics->SetAnimSet( n_anim_set ); }
	// Plays the actor verse from the template handling the positional attenuation
	void					PlaySoundVersePos( D3DXVECTOR2 vListenerPos, EActorSoundVerse sVerse, bool bPlayIfNotPlayingOnly = false );
	// Sets the actor's weapon and template upgrades and limitations generated by the weapon, adding them to actor initial template (after spawning, without weapons)
	void					ApplyWeaponTemplate( CWeapon * pWeapon );
	
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
