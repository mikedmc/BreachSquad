#pragma once

#include "LevelDefines.h"
#include "ActorTypes.h"
#include "ActorTemplate.h"
#include "components/SpriteAnimComp.h"

// suspend flags used on actor->nSuspendedFlag 
#define K_LVL_SUSPENDFLAG_NONE 0
// player suspended - fallen offscreen
#define K_LVL_SUSPENDFLAG_OUTSIDE_SCREEN 1

class CActor : public IActiveInterface
{
	///--- COMPONENTS --- 
	/// Pointer components get deallocated by the actor, referenced ones are global so we don't touch them:
private:
	CSpriteAnimComponent*		c_graphics;		//graphics component that handles all painting and animation stuff

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
