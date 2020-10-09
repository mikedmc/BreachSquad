#pragma once

#include "gameplay/GameConstants.h"
#include "gameplay/ActiveInterface.h"
#include "gameplay/CollisionShape.h"
#include "gameplay/Light.h"

//--- RTT ---
//TODO: daca nu am nevoie de toate cadranele poate incerc 2 RT-uri de 512 sau unul de 1024x512
#define K_RTT_WIDTH	1024
#define K_RTT_HEIGHT 1024
//half sizes
#define K_RTT_H_WIDTH 512
#define K_RTT_H_HEIGHT 512

//--- occluders ---
#define K_LVL_MAX_OCCLUDERS_CNT 200
//more accurate aiming when crouched (percent multiplied with error)
#define K_LVL_CROUCH_ERROR_MULTIPLIER	0.7f
//more accurate aiming when in cover
#define K_LVL_COVER_ERROR_MULTIPLIER	0.5f
//minimum stun duration to get them dizzy
#define K_LVL_MIN_STUN_DIZZY_DURATION 0.2f
//how much it waits before teleporting outside of the screen players back
#define K_LVL_LOCALCOOP_TELEPORT_WAIT 3.0f
//RECON time for maximum aim
#define K_LVL_RECON_AIMING_DURATION 1.0f

//--- ENEMIES si personaj ---
#define K_LVL_ACTOR_MAX_FALL_SPEED_Y 500.0f
//max lateral speed
#define K_LVL_ACTOR_MAX_LATERAL_SPEED_X 400.0f
//max lateral impulse
#define K_LVL_ACTOR_MAX_LATERAL_IMPULSE_X 1200.0f
//cu ce viteza trebuie sa cada ca sa faca praf
#define K_LVL_MAX_FALL_SPEED_Y_DUST 300.0f
//cu ce viteza trebuie sa cada ca sa sparga geamul
#define K_LVL_MAX_FALL_SPEED_Y_BREAKGLASS 200.0f
//cat pierde din viata cand cade de sus
#define K_LVL_ACTOR_FALL_DAMAGE		25.0f
//la cat timp dupa ce cade de pe platforma poate sari din nou
#define K_LVL_ACTOR_JUMP_AFTER_PLATFORM_TIME 0.05f

//--- touch timer ---
#define K_LVL_TOUCH_TIMER_RESET_TIME	0.3f
//--- PLAYER death timeout wait ---
#define K_LVL_PLAYER_DEATH_TIMER 10.0f

#define K_LVL_GRAVITY 1000.0f
//distanta la care focalizeaza camera
#define K_LVL_CAM_LOOK_OFFSET 0.0f
#define K_LVL_CAM_FOLLOW_SPRING_KS 100.0f
#define K_LVL_CAM_FOLLOW_DAMPING_KD 20.0f
//impulse friction
#define K_LVL_AIR_DEFAULT_FRICTION 8.0f
#define K_LVL_GROUND_DEFAULT_FRICTION 10.0f
//multiplier de impuls pe cadavre
#define K_LVL_DEAD_BODY_BULLET_MOMENTUM_MULTIPLIER 2.0f
//la cati pixeli de penetrare face squash
#define K_LVL_MAX_PENETRATION 5

///--- AFRAMES flags ---
//flags set from editor and returned from Update(shoot, footsteps, etc)
#define K_LVL_ACTIVE_AFRAMEFLAG_ACTION 1
#define K_LVL_ACTIVE_AFRAMEFLAG_SOUND 2
//resets the rate of fire so we can shoot fast from the animation ACTION flags
#define K_LVL_ACTIVE_AFRAMEFLAG_RESET_FIRE_RATE 4
//when set the weapon shoots backwards (like ACTION flag but backwards)
#define K_LVL_ACTIVE_AFRAMEFLAG_ACTION_SYMMETRIC 8

//flaguri ale activilor explortate din editor pe 32 bits ca sa nu mai export valori separate pt fiecare. Se descompun la load
#define K_EDITOR_ACTIVE_FLAG_FLIPX 1
#define K_EDITOR_ACTIVE_FLAG_FLIPY 2
#define K_EDITOR_ACTIVE_FLAG_ANIMATED 4
#define K_EDITOR_ACTIVE_FLAG_IS_COVER 8

//level file format version from the Editor
#define K_EDITOR_LEVEL_FILE_FORMAT_VERSION 1015

///--- BACKGROUNDS ---
//pe cati biti e salvata valoarea flagurilor
#define K_LVL_BK_AFRAMEBITMASK_REPEAT_X 1
#define K_LVL_BK_AFRAMEBITMASK_REPEAT_Y 1
#define K_LVL_BK_AFRAMEBITMASK_XMOVE_MUL 255
#define K_LVL_BK_AFRAMEBITMASK_YMOVE_MUL 255
#define K_LVL_BK_AFRAMEBITMASK_XBYTIME   63
#define K_LVL_BK_AFRAMEBITMASK_XBYTIMENEG 1
#define K_LVL_BK_AFRAMEBITMASK_YBYTIME 63
#define K_LVL_BK_AFRAMEBITMASK_YBYTIMENEG 1
//si shifingul dinainte de mascare
#define K_LVL_BK_AFRAMESHIFT_REPEAT_X 0
#define K_LVL_BK_AFRAMESHIFT_REPEAT_Y 1
#define K_LVL_BK_AFRAMESHIFT_XMOVE_MUL 2
#define K_LVL_BK_AFRAMESHIFT_YMOVE_MUL 10
#define K_LVL_BK_AFRAMESHIFT_XBYTIME   18
#define K_LVL_BK_AFRAMESHIFT_XBYTIMENEG 24
#define K_LVL_BK_AFRAMESHIFT_YBYTIME	25
#define K_LVL_BK_AFRAMESHIFT_YBYTIMENEG 31

//--- TILE CLASS ---
#define K_LVL_LAYER_BACK	0
#define K_LVL_LAYER_MIDDLE	1
#define K_LVL_LAYER_FRONT	2
//total number of layers
#define K_LVL_LAYERS_CNT 3

class CTile {
public:
	int tileIDs[K_LVL_LAYERS_CNT];
	RECT srcRects[K_LVL_LAYERS_CNT];

	CTile()
	{
		for (int kk = 0; kk < K_LVL_LAYERS_CNT; kk++)
		{
			tileIDs[kk] = -1;
			SetRect(&srcRects[kk], 0, 0, 0, 0);
		}
	}
};

//this enum must be sincronizat with AI_states list (level.cpp)
enum AI_STATE 
{
	K_AI_STATE_UNDEFINED = -1,

	K_AI_STATE_FN_POS_ELLIPSE = 0,			//params: f_radX, f_radY, f_timeMul
	K_AI_STATE_FN_ANG_SIN_TIME = 1,			//params: f_min, f_max, f_timeMul, f_timeAdd
	K_AI_STATE_FN_ALPHA_SIN_TIME,			//params: f_min, f_max, f_timeMul, f_timeAdd
	K_AI_STATE_FN_GET_TARGET_POS,			//params: none
	K_AI_STATE_FN_GET_TARGET_ANG,			//params: none
	K_AI_STATE_FN_FOLLOW_TARGET_RAIL,		//params: n_dir=-1/1, f_speedPPS, f_pointPauseSec, b_autoChangeDirection, b_looping
	K_AI_STATE_FN_TOUCH_WHEN_SEE_PLAYER,	//params: f_angle, f_angleFOV, f_radius, f_cooldownSec
	//lights
	K_AI_STATE_FN_LIGHT_FLICKER1,		//params: f_timeMul, f_threshold
	K_AI_STATE_FN_LIGHT_ANG_CONE_XZ_TIME,	//params: f_coneHeight, f_coneRadius, f_timeMul, f_timeAdd
	//triggers
	K_AI_STATE_TRIGGER_IN_OUT,			//params: b_triggerPlayer, b_triggerActor, s_onOutScript
	//particle generators
	K_AI_STATE_PARTICLES_GENERATOR,		//params: s_type, s_layer="RT, back, back_light, etc" - tipul generatorului de particule
	//collision
	K_AI_STATE_COLL_FOG_OF_WAR,			//no params
	K_AI_STATE_COLL_BREAKABLE_DOOR,		//params: f_life-can be damaged by bullets, b_reinforced=1 only the SAW can break it
	K_AI_STATE_COLL_BREAKABLE_WINDOW,	//params: f_life - daca specifici life inseamna ca se poate sparge cu gloante
	K_AI_STATE_COLL_KILL_ACTORS,		//params: b_killPlayer, b_killOthers
	///--- actives ---
	K_AI_STATE_ACTIVE_SWINGING_FRONTOBJ,//no param - se balanseaza cand dai grenada langa ele
	K_AI_STATE_ACTIVE_EXPLO_TRAP,		//no param
	K_AI_STATE_ACTIVE_CHECKPOINT,		//param: n_isFirst(0/1) - default first spawn point
	K_AI_STATE_ACTIVE_TEAM_TELEPORTER_2FRAMES, //param: f_SlowTimeDuration, b_EnterHiddenRoom, b_DontChangeFrames, f_teleportDuration, s_openSnd, s_closeSnd
	K_AI_STATE_ACTIVE_DOORFACE_AUTOCLOSE,	//param: s_openSnd, s_closeSnd, b_DontChangeFrames
	K_AI_STATE_ACTIVE_DOOR_SECTION,			//param: n_locked, f_lockpickTime

	K_AI_STATE_ACTIVE_AMMO_BOX,			//param: n_ammoLeft
	K_AI_STATE_ACTIVE_HEALTH_BOX,		//param: n_healthLeft
	K_AI_STATE_ACTIVE_BOMB,				//param: f_explodeTimerSec
	K_AI_STATE_ACTIVE_ZOMBIE_SPAWNER,	//param: f_spawnFreq, n_maxSpawns
	///--- ACTORS ---
	//nu avem stari pentru actori - sunt tratate cu behaviors
	//states no
	K_AI_STATES_CNT
};


int GetAIStateByNameHash(UINT32 stateHash);




///--------------------------------------------------------------------------
///--- ACTIVES : obiectele din nivel, de decor sau scriptabile
///--------------------------------------------------------------------------
class CActive : public IActiveInterface
{
public:
	CSprite			sprite;
	int				nAnim_ini, nFrame_ini; //animatie si frame initial
	int				nLayer; //pe ce layer este obiectul de decor
	
	bool			flipX, flipY; //flip flags

	CActive() :
		flipX(false), flipY(false),
		nAnim_ini(-1), nFrame_ini(-1), nLayer(0)
	{}

	const int GetClassType() const {
		return K_LVL_IAI_TYPE_ACTIVE;
	}

	void SetPos(D3DXVECTOR2 newPos) override;
	void Move(D3DXVECTOR2 delta) override;
	void SetAngle(float fnAngle) override;

	// Initializes custom internal data (hardcodes usually)
	void InitInternalData();
};

///--------------------------------------------------------------------------
///--- ACTORS : clasa principala de inamici si personaje player
///--------------------------------------------------------------------------

//--> suspend flags used on actor->nSuspendedFlag 
#define K_LVL_SUSPENDFLAG_NONE 0
//player suspended - fallen offscreen
#define K_LVL_SUSPENDFLAG_OUTSIDE_SCREEN 1

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

		CAISensorInfo() :
			pTargetedActor(null), b_IsDead(false), m_lastInteractingActorUID(0), 
			m_bEnabled(true), fTargetOverlapX(0.0f), fTimeSinceHit(1000.0f)
		{
			m_AIlastEvent.Reset();
			m_AIcurrentEvent.Reset();
		}

		void Reset()
		{
			pTargetedActor = null;
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
		bool	bRunning;	//daca alearga
		bool	bThrustX, bThrustY;	//daca se deplaseaza sau nu
		bool	bCrouched;	//daca este crouch sau nu
		bool	bJump;		//comanda de jump
		bool	bClimb;		//comanda sa se catere
		int		nInteractKeyState;  //stare buton interact (just pressed, not pressed etc)
		int		nMoveDirX;	//directia de miscare ca si flaguri (-1,0,1)
		int		nMoveDirY;	//directia de miscare ca si flaguri (-1,0,1)
		int		nLookDirX;	//directia in care se uita -1/0/1
		D3DXVECTOR2		vAimDir;		//comanda de aiming pentru arma	(0.0f, 0.0f - for no aiming)
		EActorAnims		eOverrideAnim;	//if not empty, overrides actor animation

		EActorDeathCommand nDeathCommand; //0-not dead, 1-dead, 2-splat, 3-splat+explode
		EActorAttackState eAttackCommand;
		EActorAttackState eAttackCommand_last; //last attack command
		//set icon commands
		EActorIconTypes		nIconType;
		float	fIconDuration;	//daca setez
		//color command
		DWORD	nColor; //daca e diferit de 0 inseamna ca e comanda de culoare

		CAICommands() :
			bRunning(false), bThrustX(false), bThrustY(false), nMoveDirX(0), nMoveDirY(0), bJump(false),
			nLookDirX(0), bCrouched(false), bClimb(false), nInteractKeyState(K_CM_BUTSTATE_NOTPRESSED), nColor(0),
			eAttackCommand(K_LVL_ACT_ATTACK_IDLE), eAttackCommand_last(K_LVL_ACT_ATTACK_IDLE), nDeathCommand(K_LVL_ACT_DEATHCMD_NONE),
			nIconType(K_LVL_ACT_ICON_NONE), fIconDuration(0.0f), eOverrideAnim(K_LVL_ACT_ANIM_EMPTY)
		{
		}

		void Reset()
		{
			bThrustX = false;
			bThrustY = false;
			bRunning = false;
			nMoveDirX = 0;
			nMoveDirY = 0;
			nLookDirX = 0;
			vAimDir = D3DXVECTOR2(0.0f, 0.0f);

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
			bThrustY = false;
			bRunning = false;
			nMoveDirX = 0;
			nMoveDirY = 0;
			nLookDirX = 0;
			vAimDir = D3DXVECTOR2(0.0f, 0.0f);

			bCrouched = false;
			bJump = false;
			bClimb = false;
			nInteractKeyState = K_CM_BUTSTATE_NOTPRESSED;
			eAttackCommand = K_LVL_ACT_ATTACK_IDLE;
		}
	};

public:
	CActorTemplate			templateActor;				//datele generale din XML copiate in fiecare actor, datele curente
	CActorTemplate			templateActor_ini;			//datele initiale, imediat dupa loading si dupa customizarea initiala
	EActorAnims				eLastAnimSet, eLastAnimSet_feet;		//ultima animatie setata  pe actor prin SetActorAnimOnce() (torso si feet)

	EActorSoundVerse		eLastPlayedVerse;			//last played sound verse
	float					fVerseCooldown;				//don't play the same verse if cooldown > 0.0f
	int						nLastPlayedVerseSndIdx;		//last played sound idx
protected:
	int			nAnimSet;	//current animation set (-1 for RANDOM); Don't set directly!

public:
	//collision
	bool		bHasCollision;			//se calculeaza coliziunea cu nivelul
	UINT16		collisionFlags;		    //iti spune in ce directii are coliziune (K_DIRFLAG_)
	CCollisionShape*	standOnBox;		//cutia pe care sta, de la care ia miscarea. 
	float		fTimeAirborn;			//de cat timp este in aer, util pt jump after falling

	//puncte de interes	[3] - shooting, crouched, dead
	D3DXVECTOR2 vecWeapon_abs[3], vecHeart_abs[3], vecGroundCheck_abs[3]; //offseturi relative incarcate din REF_POSE
	RECTXYWH_F	stateBBoxes[3]; //bboxurile pentru idle, crouch, dead - salvate in InitActor
	D3DXVECTOR2 GetPosHeart(); //intoarce pozitia exacta a inimii in fn de starea curenta
	D3DXVECTOR2 GetPosWeapon();	//intoarce pozitia exacta a armei in fn de starea curenta

	D3DXVECTOR2	posHeart, posWeapon;	//pozitii absolute inima si arma presalvate (pentru viteza)
	void UpdateBBoxAndPoints();		//aduce la zi variabilele de bbox si posweapon, posheart

	D3DXVECTOR2	vecCamFollowPos;	//pozitia relativa in care se uita camera cand am in focus Actorul curent

	D3DXVECTOR2		speed;
	D3DXVECTOR2		vSpeedImpulse;	//viteza aplicata extern (cand e impuscat de exemplu). Se va atenua automat.

	int			lookDirXsign; //directia in care se uita pe X (-1 sau 1)
	D3DXVECTOR2	vAngleDir;		//directia efectiva a privirii in fn de fAngle (folosita doar de catre unele specii de actori)
	float		fLife, fArmor; //cata viata are si cata armura
	float		fFOVPercent;   //field of view-ul personajului, intre 0 si 1 => 0.5 va fi FOV de 90 de grade. Reprezinta un fel de alertLevel si seteaza si hearing range
	//flags
	bool		bHasGravity; //daca are gravitatie

	CDamageOverTime		cDamageOverTime;	//daca are efect de damage/heal over time
	EActorAttackState	nAttackStatus; //aici este statusul legat de arma (shooting, shootalt, reloading, etc)

	bool		bOnLadder; //daca este pe un ladder
	byte 		nInteractingState;	//0 - not interacting, 1 interacting, 2 lockpicking
	bool		bCrouched; //daca este crouched
	eGenericState	nRolling; //0-ready, 1-rolling, 2-finished and waiting reset (direction key up)
	CCollisionShape* pCover; //daca e diferit de null inseamna ca pe langa crouched e si covered
	//CMiscObjectRail* pRail;		//pointer catre un rail atunci cand merge pe tiroliana
	//bool		bInWater; //daca este in apa
	float		fStunTimer; //daca e diferit de 0 personajul este stunned
	int			nTookDamageFrames; //flag folosit pentru a desena frames aprinse cand lovesti inamic
	UINT32		nLastDamageTakenFromUID;	//UID that gave actor last damage 
	BYTE		nSkinIdx;					//player skin

	UINT32		nSuspendedFlags;	//daca e suspended (AI), folosit de obicei la playerii principali ca sa le tai inputul (cand ies din ecran sau cand vrei sa nu poti sa-i mai controlezi). Flagul specifica motivul
	float		fSuspendedTimer;	//counts from when suspended flags is set
	bool		bSuspendInput;		//if set keyboard input is ignored

	void SetIcon(EActorIconTypes iconType, float fDuration = 0.0f); //seteaza icon

	IActiveInterface*	pClosestTouchable; //cel mai apropiat activ cu can interact

	CSprite		sprite, sprite_feet;
	CSprite		m_sprOverheadIcon;	//icon shown when interacting with things (doors, objects) or in other circumstances too
	void		SetAnimSet(int newAnimSet); //sets current animation set (changes immediately)
	FORCEINLINE int GetAnimSet() const; 
	CWeapon		weapons[K_LVL_ACT_WEAPONS_CNT]; //colectia de arme posibile ale playerului. Efectiv armele care isi fac update.
	CWeapon*	pCurrentWeapon;				//arma cu care trage acum (poate fi gear, alt, primary, etc). Nu va fi niciodata NULL
	CWeapon*	pSelectedWeapon[K_LVL_ACT_WEAPONS_CNT]; //active weapons for each action - pointers to the weapons collection so we don't lose the original weapons
	//player control and controller data
	int			nPlayerOrdinal;	//player index (0-max_players_cnt)
	int			nControllerInstanceID; //player controller ID (-1 for empty)
	
	//--- elemente vizuale ---
	//icons desenate deasupra
	EActorIconTypes nIconType;
	float			fIconTimer;
	//--- AI ---
	CAISensorInfo	m_AIsensorInfo;	//informatii intrare AI
	CAICommands		m_AIcommands;	//commands issued by AI

	CAIState*		m_pAIcurrentState; //starea curenta de AI
	int				m_nAIcurrentBehaviorIdx; //indexul curent al behaviorului din state-ul curent (sau -1 cand nu e setat)
	float			m_fAIbehaviorTimer;		//timer used for timed behaviors
	EAIBehaviorType GetCurrentBehavior();	//intoarce behavior curent

	//CTOR
	CActor() :
		m_pAIcurrentState(null), m_nAIcurrentBehaviorIdx(-1), m_fAIbehaviorTimer(0.0f), nTookDamageFrames(0), nLastDamageTakenFromUID(0),
		pCurrentWeapon(null), nSkinIdx(0),
		eLastAnimSet(K_LVL_ACT_ANIM_EMPTY), nAnimSet(0), nSuspendedFlags(0), fSuspendedTimer(0.0f), bSuspendInput(false), fTimeAirborn(0.0f),
		eLastPlayedVerse(K_LVL_ACT_VERSE_EMPTY), fVerseCooldown(0.0f), nLastPlayedVerseSndIdx(-1)
	{
		bAnimated = true;
		nControllerInstanceID = -1;
	}

	const int GetClassType() const {
		return K_LVL_IAI_TYPE_ACTOR;
	}

	void SetPos(D3DXVECTOR2 newPos) override;
	void Move(D3DXVECTOR2 delta) override;
	void SetAngle(float fNewAngle) override; //seteaza unghiul si vAngleDir
};

///--- lista de obiecte vizibile/active ---
class CVisibilityLists {
//visual
public:
	//liste de pointeri activi
	CFixedArray<CLight*, 128> visible_lights;
	//lista de activi vizibili
	CFixedArray<CActive*, 512> visible_actives[K_LVL_LAYERS_CNT];
	//decals
	CFixedArray<CDecal*, 512> visible_decals[K_LVL_DECAL_LAYERS];
	//visible actors
	CFixedArray<CActor*, 256> visible_actors; 
	//collision shapes folosite la construierea volumelor de umbra
	CFixedArray<CCollisionShape*, 512> visible_colShapesLights;

//logic
public:
	//collision shapes (visible or closeby)
	CFixedArray<CCollisionShape*, 512>	logic_colShapes;
	//collision shapes from a larger area
	CFixedArray<CCollisionShape*, 1024>	logic_colShapesExtended; 

	CFixedArray<CActive*, 512> logic_actives_closeby[K_LVL_LAYERS_CNT]; //actives that can be interacted with
	CFixedArray<CActor*, 256> logic_actors_closeby; //closeby actors - bullets tests
	CFixedArray<CCollisionShape*, 256> logic_colShapesSpecial; //special collision shapes (water, triggers)
	//CTOR/DTOR
	CVisibilityLists() {}
};


enum ELevelState {
	K_LVL_STATE_PLAYING, 
	//next items only level finished states (see main.cpp network sync condition)
	K_LVL_STATE_MISSION_ACCOMPLISHED,
	K_LVL_STATE_MISSION_FAILED,
};

class CLevel : public IScriptable
{
public:
	//--- RTT nivel (color, normals, etc) ---
	LPD3DXRENDERTOSURFACE   m_pRenderToSurface;
	LPDIRECT3DTEXTURE9      m_pRTTexture;
	LPDIRECT3DSURFACE9      m_pRTSurface;
	//--- RTT composite - nivelul iluminat, inainte de efectele de deformare tip foc, apa, explozii ---
	LPD3DXRENDERTOSURFACE   m_pRT_final;
	LPDIRECT3DTEXTURE9      m_pRTTexture_final;
	LPDIRECT3DSURFACE9      m_pRTSurface_final;

	double			fLocalTimeline;
	CTimersArray	m_Timers;
	//network synced random generator
	CRandom			m_rand;
	//--- level states ---
	ELevelState m_levelState;	//masina de stari nivel
	int m_levelSubState; //substarile unei stari
	int	m_levelStateParam;	//parametru trimis schimbarii de stare (used only sometimes)
	float m_levelStateTimer; //timer folosit de catre stari
	void SetLevelState(ELevelState eNewState, int nLevelStateParam = 0);

	bool	m_bLoaded;							//is level loaded?
	int		m_nLoadedLevel, m_nLoadedChapter;	//nivelul si episodul incarcat
	int		m_nLoadedLevelType;					//type of loaded level
	
	int		m_unLoadedLevelFlags;				//K_LVL_LEVEL_FLAG_ set at level loading (usually prevent saving scores)

	bool m_bOneUpdateDone;		//#HACK: daca a facut cel putin un update ca sa am toate variabilele setate inainte de paint
	//--- time control ---
	float m_fTimeMultiplier_real;	//multiplicator timp - Don't set directly
	float m_fTimeMultiplier; //setare multiplicator dTime (valoare catre care tinde TimeMultiplier_real)
	float m_fTimeMultiplierDuration; //cat timp dureaza schimbarea de timp
	void SetTimeMultiplier(float fMultiplier, float fDuration);
public:
	LPDIRECT3DDEVICE9	m_pDevice; //pointer la device
	//pointer la game sprite class	
	ID3DXSprite*		m_pSprite; 
	void SetSpritePtr(ID3DXSprite* pSprite) {
		m_pSprite = pSprite;
	}

	CTextureManager m_texManager;

	CSpriteCollection m_sprLights;  //Animatiile de lumini au un format specific in fn de lumina (point:fr0-spot, fr1-glow)
	CSpriteCollection m_sprActives; //decorations
	CSpriteCollection m_sprActors;	//animations for the actors (main characters, enemies etc)
	CSpriteCollection m_sprBack;	//background graphics

	CScreenVignette	  m_screenVignette;	//darken screen vignette
	CScreenVignette	  m_screenVignetteDamage;	//damage vignette

	CVisibilityLists  m_visibleList; //lista de elemente vizibile sau active
	void BuildVisibilityLists();
	void ClearVisibilityLists();

	CBufferedPainter		m_bufferedPainter;	//folosit la desenarea de poligoane

	int						tileW, tileH;		//size of tiles
	SIZEWH					levelSizeTL;		//size of the level (in tiles)
	RECTXYWH_F				m_levelAABB;		//level AABB in pixels
	RECTXYWH				m_levelAABB_TL;		//level AABB in tiles (active tiles area, can be moved when generating random levels)

	CTile**					tiles;				//actual tilemap
	int						m_tilesTexBaseIdx, m_tilesTexNormIdx;		//indexuri la texturile folosite pt tileset
	D3DXVECTOR2				m_vLevelOrigin;		//originea fictiva a nivelului

	RECTXYWH_F				m_visibleArea;		//zona vizibila din BBuff in pixeli, coord world
	RECTXYWH				m_visibleAreaTL;	//zona vizibila din nivel, in tiles.
	///--- TEMPLATES ---
	CGrowableArray<CWeaponTemplate*>		m_arrTemplatesWeapon;
	CGrowableArray<CExplosionTemplate*>		m_arrTemplatesExplosion;
	CWeaponTemplate*		GetTemplateWeapon(WCHAR * templateName);
	CWeaponTemplate*		GetTemplateWeapon(DWORD templateNameHash);
	CExplosionTemplate*		GetTemplateExplosion(UINT32 templateNameHash);
	HRESULT					LoadWeaponTemplates(WCHAR * xmlPath);
	HRESULT					Weapon_Init(CWeapon* pWeapon, WCHAR* weaponTemplateName, CActor* pParent);

	CGrowableArray<CActorTemplate*> m_arrTemplatesActor;	//actor templates array
	CGrowableArray<CAITemplate*>	m_arrAItemplates;		//array folosit pentru salvarea template-urilor AI
	CActorTemplate*			GetTemplateActor(const WCHAR * templateName);
	CActorTemplate*			GetTemplateActor(const DWORD templateNameHash);
	// Randomizes the actor a little so they don't all have the exact same speeds
	void					RandomizeTemplateActor(CActorTemplate * actTemplate);
	HRESULT					LoadActorTemplates(WCHAR * xmlPath);
	// incarca bbox si hitpoints din animatie anume
	HRESULT					LoadActorBBoxAndPoints(CActor * destAct, EActorAnims eAnim, int nAnimSet = 0);
	// verifica daca se termina platforma pe care sta actorul, in directia ceruta
	bool					IsPlatformEnding(CActor* actor, int nDirSign);

	CGrowableArray<CCollisionShape*>	m_arrColShapes;
	// intoarce coliziunea unei drepte cu un collision shape. E cam ca AABB_Segment_Intersection_Arr dar cu coll shapes
	CCollisionShape*		ColShape_Segment_Intersection_Arr(D3DXVECTOR2 & start, D3DXVECTOR2 & end, CCollisionShape * arrBoxes[], int nBoxesCnt, D3DXVECTOR2 * retCollisionPoint, D3DXVECTOR2 * retNormal);
	// returns the first intersection of aabbSRC with a Collision Shape
	CCollisionShape*		ColShape_CAABB_Intersect_Arr(CAABB * aabbSrc, CCollisionShape * arrBoxes[], int nBoxesCnt);

	CGrowableArray<CActive*>	m_arrActives;				//obiectele din nivel (active sau nu)
	CFixedArray<CActive*, 256>	m_arrActivesPtrInteract;	//array containing objects that you can interact with (for speed checks)
	CGrowableArray<CActor*>		m_arrActors;				//actorii - inamici cu animatii
	// Initializes CActor with specified template and sets all the data it needs 
	HRESULT					InitActor(CActor* actor, CActorTemplate * actTemplate, D3DXVECTOR2 spawnPos);
	// Seteaza noua stare si are in vedere si incheierea starii precedente
	void					SetActorAIState(CActor * actor, CAIState* pNewState);
	//sets the current actor's weapon and template upgrades and limitations generated by the weapon	
	void					SetActorWeaponPerks(CActor * pActor, CWeapon * pWeapon);
	// Seteaza noua stare si are in vedere si incheierea starii precedente
	// \returns true:success false:state not found
	bool					SetActorAIState(CActor * actor, WCHAR * strStateName);
	// Sets behavior by idx, from current state behaviors array
	// \param: ret_bFinished - set to true if current behavior doesn't need an update (like set_animation or set_flag, etc)
	// \returns: true if set, false if error
	bool					SetActorAIBehaviorIdx(CActor * actor, int nBehaviorIdx, bool &ret_bFinished);
	// Sets damage over time
	void					SetActorDoT(CActor* act, CDamageOverTime::EDoTType eType, float fDuration, float fDamagePerSec, EActorClass eExcludedClass, EActorClass eFilterClass, DWORD dwOwnerUID);
	// Called when changing behaviors (to exit them gracefully)
	void					OnActorBehaviorFinished(CActor * actor, EAIBehaviorType eOldBehavior);
	// Seteaza animatia actorului o singura data, in functie de tipul animatiei cerute
	void					SetActorAnimationOnce(CActor* actor, EActorAnims nAnimType, EActorAnims nAnimTypeFeet = K_LVL_ACT_ANIM_EMPTY, bool bKeepFrame = false);
	// Gets the current type of animation that the actor is playing
	EActorAnims				GetActorAnimationType(CActor* actor);
	// Plays the actor verse from the template. 
	void					PlayActorSoundVerse(CActor* actor, EActorSoundVerse sVerse, bool bPlayIfNotPlayingOnly = false);
	// Tells you if the actor has said animation 
	FORCEINLINE bool		ActorHasAnimation(CActor* actor, EActorAnims nAnimType);
	//Kills the actor
	void					KillActor(CActor * actor, bool bSplatTarget = false);
	// Use it to damage enemies and player
	// @fHitPointsTaken - negative value - kills it immediately
	// RETURNS: damage made, type of material hit.
	CBulletHitReturnData	HitActor(CActor * actor, CBullet * pBullet, D3DXVECTOR2 * pvProjectileMomentum = NULL);
	
	//hits the actor with other things than bullets
	CBulletHitReturnData	HitActor(CActor * actor, float fDamage, UINT32 dwOwnerUID, EActorClass eOwnerClass, D3DXVECTOR2 *vDir = null, UINT32 dwBulletFlags = 0, int nArmorPiercingRating = 100, float fStunDuration = 0.0f);
	void					SetActorStun(CActor* actor, float fStunDuration);
	//shoots melee blows (bullets)
	int						MeleeBlow(int nBulletType, D3DXVECTOR2 vPos, D3DXVECTOR2 vDirection, UINT32 nOwnerUID, int nOwnerClass, float fRange, float fDamageActors, float fImpulse, float fStunDurationMax, EActorClass eIgnoredClass, float fRangeObjects, float fDamageObjects );
	// Spawns a player
	void					SpawnPlayer(D3DXVECTOR2 spawnPos, int nPlayerOrdinal, int nAnimset = 0);
	// Spawns an actor (NPC)
	CActor*					SpawnActor(D3DXVECTOR2 spawnPos, WCHAR* strTemplateName, int nLookDirSign, CStringHash* shStateOverride = null);
	// Spawns a new Active with empty properties
	CActive*				SpawnActive(D3DXVECTOR2 spawnPos, int nAnimIdx, int nFrameIdx, int nLayer = K_LVL_LAYER_BACK);
	// Spawns a light
	CLight*					SpawnLight(D3DXVECTOR3 spawnPos, int nType, int nAnimIdx, DWORD dwColor, float fScale = 1.0f, bool bCastShadows = false);
	// Gives a score for the user powerups placement 
	// \brief: used to move player spawned objects away from intersections with other interactibles and walls
	int						GetPowerupPlacingScore(CActive * active, D3DXVECTOR2 vPlacerPos);
	// Finds the best spawning rect for a proposed position
	// \returns false when can't be spawned safely
	// \param rectProposed_ret - the proposed placing rectangle
	bool					GetBestSpawningPos(D3DXVECTOR2 * vSpawn_ret, CAABB rectStart, CAABB * rectToAvoid = NULL);
	// Tells you if the bbox overlaps interactive elements (is relatively slow)
	bool					GetIsAreaNeutral(RECTXYWH_F rectArea);
	///--- LIGHTS ---
	CGrowableArray<CLight*> m_arrLights;				//array of lights
	DWORD					m_colAmbientGlobal;			//global ambient color
	float					m_fThunderTimer;			//pentru desenarea efectului de thunder/lightning (0.0f - stopped)
	
	///--- AI ---
	CGrowableArray<CAIEvent*> m_arrAIevents;
	/* Releases all dead objects (bReleaseIt flag set) on a separate step so they don't get deallocated when still in visibility lists */
	void					CleanupDeadObjects();
	/*--- updates all IActiveInterface implementations ---*/
	void					UpdateAI(float dTime);
	//updates AI for base class (common AIs)
	bool					UpdateAI_base(IActiveInterface* active, float dTime, double fTimeline);
	//updates AI for derived classes (particulare)
	void					UpdateAI_light(CLight* light, float dTime);
	void					UpdateAI_active(CActive* active, float dTime);
	void					UpdateAI_actor(CActor* actor, float dTime);
	void					UpdateAI_collshape(CCollisionShape * colshape, float dTime);
	//Seteaza AI si face toate setarile initiale din AI
	void					SetAI(IActiveInterface * active, int AIstate, CVariantCollection * params, INT32 targetID = -1);
	//Suprascrie parametrii din AI sau adauga params noi
	void					SetAIparams(IActiveInterface * active, CVariantCollection * params, bool bClearParams = false);
	//gaseste cel mai apropiat inamic vizibil
	CActor*					GetClosestTarget(CActor * sourceActor, EActorClass eTargetClassFilter1 = K_LVL_ACT_CLASS_ANY, EActorClass eTargetClassFilter2 = K_LVL_ACT_CLASS_ANY);
	// Finds closest visible actor of specified name (inside visibility radius)
	// @fMaxDistance - if greater than 0 then it overrides seeDistance
	CActor*					GetClosestActorByTemplateName(CActor * sourceActor, WCHAR * sTargetTemplateName, float fMaxDistance = 0.0f);
	// Finds the closest cover box from the level collision boxes list or null if none in range
	CCollisionShape*		GetClosestCover(D3DXVECTOR2 vPos, float fMaxDistance = 0.0f);
	//AI events (radius < 0.0f means infinite)
	void					AddAIEvent(EAIEventType eventType, UINT32 ownerUID, int ownerClass, D3DXVECTOR2 vPos, float radius, float duration = 0.6f, UINT32 targetUID = 0);
	// Deletes a targeted event
	// @targetUID - if not set it deletes all events of said type
	void					DeleteAITargetedEvent(EAIEventType eEvtType, UINT32 targetUID = 0);
	//Gaseste cel mai apropiat event (de tipul typeFilter daca il specific) cu linie directa de vedere
	CAIEvent*				GetMostImportantAIEvent(CActor * callerActor, EAIEventType eTypeFilter = K_LVL_AI_EVENT_ANY);
	///--- decals ---
	CGrowableArray<CDecal*> m_arrDecals;
	void					AddDecal(EDecalLayer nLayer, D3DXVECTOR2 pos, int animIdx, int frameIdx = 0, DWORD color = 0xffffffff, bool bIsAnimated = false);
	void					UpdateDecals(float dTime);
	//adds a blood decal (bLarge when enemy was splattered)
	void					AddDecal_BloodSplat(D3DXVECTOR2 pos, bool bLarge, EActorClass eVictimClass = K_LVL_ACT_CLASS_ANY);
	///--- physics points ---
	CLinkedPool<CPhysicsPoint2D>	m_poolPhysPts; //pool de obiecte fizice
	void					UpdatePhysicsPoints(float dTime);
	///--- bullets linked pool ---
	CLinkedPool<CBullet>	m_poolBullets;			//pool-ul de gloante
	int						m_bulletsMeshIdx;		//idx gloante
	CFixedArray<CBullet*, 128> m_arrBulletsTemp;	//Temporary bullets list used for misc checks

	///--- water ---
	int						m_waterMeshIdx;			//idx la meshul apelor vizibile in ecran
	int						m_waterTexIdx;			//idx textura normale apa in texManager
	int						m_waterAnimIdx;			//idx animatie apa din fisierul de fundal
	///--- room occluders ---
	int						m_fogofwarMeshIdx;		//idx mesh occludere

	
	// can we shoot the weapon? some weapons have pre-shoot conditions (maybe not working underWATER)
	bool					CanShootWeapon(CWeapon * weapon);
	// Trage cu arma specificata
	// \returns: true daca a putut sa traga sau false daca nu
	bool					ShootWeapon(CWeapon * weapon, D3DXVECTOR2 vDir);
	// \returns: weapon status
	EnumWeaponStatus		UpdateWeapon(CWeapon * weapon, float dTime);
	
	// \returns: true if started reloading, false if already full or no ammo
	bool					JamWeapon(CWeapon * weapon);
	// stops reloading the weapon
	void					StopReloadingWeapon(CWeapon * weapon);
	// seteaza comenzi arma
	void					ResetBurstWeapon(CWeapon * weapon);

	// Shoots a bullet and returns a pointer to the actual bullet. Don't deallocate or make any changes on said pointer.
	CBullet*				ShootBullet(CBulletTemplate * bulletTemplate, int actorClass, UINT32 nOwnerUID, D3DXVECTOR2 pos, D3DXVECTOR2 shootDir);
	// Returns the closest bullet (or null) of nBulletType under fMaxDistance
	CBullet*				GetClosestBullet(D3DXVECTOR2 vCheckPos, EBulletType nBulletType, float fMaxDistance = 0.0f, int dwOwnerUID = 0);
	// Releases all bullets of said type from specified owner
	void					ReleaseBullet(int nBulletType, UINT32 nOwnerUID);
	void					UpdateBullets(float dTime);
	void					PaintBullets(bool paintNormals = false);
	// Marks bullets as killed and returns how many were marked 
	// \param dwOwnerUID - specifies the owner UID filter or leave 0 to ignore the owner flag
	int						KillBulletsOfType(int nBulletType, UINT32 dwOwnerUID = 0);
	///--- level props pool ---
	int						m_propsLightsMeshIdx;	//id-ul meshului pentru desenarea luminii propsurilor
	CLinkedPool<CLevelProp>	m_poolProps;			//pool de props
	// Adds a generic prop (physical particle)
	// \param nSubType - secondary type of the added Prop, handled differently on every prop
	void					AddProp(EPropType type, D3DXVECTOR2 pos, D3DXVECTOR2 * speed, D3DXVECTOR2 * accel, int nSubType = 0);
	//adauga prop - o lumina provizorie (gunshots, etc)
	void					AddProp_Light(D3DXVECTOR2 pos, int nLightAnimIdx, float fDuration, float fFadeTime, DWORD color, float fScale = 1.0f);
	// \brief helper fn: adds an explosion (atat vizual cat si logic). 
	// \param vDir este pentru exploziile directionale. 
	// \param hash_EXPLO_name sunt constante predefinite
	void					AddProp_Explo(UINT32 hash_EXPLO_name, D3DXVECTOR2 pos, UINT32 dwOwnerUID, int exploOwnerClass = K_LVL_ACT_CLASS_PLAYER, D3DXVECTOR2 vExploDir = { 0.0f, 0.0f }, CAABB* exploAABB = null);
	void					UpdateProps(float dTime);
	void					PaintProps();	  //nu cred ca e nevoie de visibility lists pt ca sunt efemere
	///--- efecte speciale ---
	void					GenerateEffect(ELVLEffectType nEffectType, D3DXVECTOR2 pos, float fSize, DWORD color = 0xffffffff);
	void					GenerateEffect(CStringHash sEffectName, D3DXVECTOR2 pos, float fSize, DWORD color = 0xffffffff);
	///--- misc objects (rails, etc) ---
	CGrowableArray<CMiscObjectBase*>	m_arrMiscObjects;

	//pointer to player and controls
	int						m_nPlayers;			//cati playeri joaca jocul same screen
	int						m_nPlayersActive;	//cati players sunt activi, not in limbo
	CActor*					pPlayerActor[K_MAX_PLAYERS_CNT];				//direct pointers to player controllers
	int						m_arrPlayerControllersIIDs[K_MAX_PLAYERS_CNT];	//used to save player controllers IIDs for each player
	int						m_arrPlayerSelHotJoin[K_MAX_PLAYERS_CNT];		//hot join selection
	D3DXVECTOR2				m_arrPlayerLastSafePos[K_MAX_PLAYERS_CNT];
	///------ sync check ------
	DWORD					m_dwSyncCheckHash;		//used to sync network players by adding float actor data, hashing it and sending it over the network
	//strategic abilities
	//selectia strategic points (-1 for no selection)
	int						m_arrPlayerSelStrategic[K_MAX_PLAYERS_CNT]; 
	//strategic ability for each point (eg: body armor 1pt, extra life 8pts) - ultima abilitate se completeaza din playerSelScreen
	int						m_arrStrategicAbilities[K_MAX_PLAYERS_CNT][K_LVL_MAX_STRATEGIC_POINTS];
	//strategic ability names
	int						m_arrStrategicAbilitiesNames[K_MAX_PLAYERS_CNT][K_LVL_MAX_STRATEGIC_POINTS];
	//initializes strategic abilities arrays arrays
	void					InitializeStrategicAbilities(int nPlayerOrdinal);
	// Last valid spawning pos (level start flags or checkpoints)
	D3DXVECTOR2				vLastSpawnPoint; 
	///--- TEAM TELEPORTING DOORS ---
	int						m_nTeleportSlots;		//cate sloturi sunt pline pe usile de echipa (team doors)
	bool					m_bTeleportActivated;	//daca trebuie facuta teleportare
	//bool					m_bTeleportRequested;	//daca s-a cerut teleportarea inainte sa intre toata echipa
	float					m_fTeleportTimer;		//durata teleportului
	IActiveInterface*		m_pTeleportSource;		//pointer la obiectul care face teleport
	bool					m_bInsideHiddenRoom;	//specifica daca cel putin un player a intrat intr-o camera ascunsa (ca sa stiu sa scot din LIMBO personajele care nu au intrat)
	CAABB					m_HiddenRoomAABB;		//current hidden room trigger AABB
	bool					m_bPlayerInHiddenRoom[K_MAX_PLAYERS_CNT];	//tells you what player is inside the hidden room
	///--- STATISTICS ---
	int						m_arrStats[K_LVL_STATS_CNT];		//array that holds the statistics
	void					ResetLevelStatistics();
	void					IncreaseLevelStatistics(int K_LVL_STATS_n, int nValueToAdd = 1);
	// Gives strategic points for the strategic points bar 
	// \param: vPos - if set it adds a text particle with the value
	void					GiveStrategicPoints(float fPoints, D3DXVECTOR2 * vPos = null); 
	// Activates special ability, if possible
	// \param: nAbilityIdx - ability index in g_arrStrategicAbilities and price at the same time
	// \returns: true if success, false if failed
	bool					ActivateSpecialAbility(int nAbilityIdx, int nTargetPlayerOrdinal);
	// Vede daca am linie directa intre 2 puncte dar verifica coliziunea doar cu rect-urile probabile
	bool					UnobstructedLineOfSight(D3DXVECTOR2 pt1, D3DXVECTOR2 pt2, D3DXVECTOR2 * retVecCollisionPt = null, D3DXVECTOR2 * retVecCollisionNormal = null);

	UINT32					m_unLastID;				//Last loaded ID - used to assign unique IDs to runtime spawned elements
	//Generates a new ID and increments m_unLastID
	UINT32					GenerateNextID();		

	CLevel();
	~CLevel();
	// Loads a level from an absolute path
	HRESULT					LoadLevel(WCHAR * strPathAbs);
	// Loads a prefab at set position (given in tiles). It doesn't scale the level matrix.
	// \param nPosXtiles, nPosYtiles - absolute position in level tiles space
	// \param bAddOnly - if true it doesn't erase tiles if source tile is 0
	HRESULT					LoadPrefabAtPosition(WCHAR * strPathAbs, int nPosXtiles, int nPosYtiles, bool bAddOnly = false);
	// Releases all level data
	void					Release();
	//Gives you a random level from a shuffled list so you play all of them in random order
	int						GetNextRandomLevel();
	//intoarce pointer catre activul cu id-ul (din editor) respectiv - derivate din CActiveInterface
	IActiveInterface*		GetIActiveInterfacePtr(int ID);
	IActiveInterface*		GetIActiveInterfacePtr_byUID(UINT32 UID);
	//Intoarce pointer la CActive cu UID-ul respectiv
	CActive*				GetActiveByUID(UINT32 UID);
	CActor*					GetActorByUID(UINT32 UID);
	CLight*					GetLightByUID(UINT32 UID);
	// Gets the player with specified UID or NULL if not found
	CActor*					GetPlayerByUID(UINT32 UID);
	// Finds the closest player (visible or not)
	CActor*					GetClosestPlayer(CActor* sourceActor, bool bIgnoreDead = false);
	CActor*					GetClosestPlayer(D3DXVECTOR2 vSrcPos, bool bIgnoreDead = false);
	//tells if pPlayer is networked
	bool					IsNetworkPlayer(CActor* pPlayer);
	//intoarce collision shape-ul care contine punctul point si este de tipul collisionType
	CCollisionShape*		GetCollisionShapeAt(D3DXVECTOR2 point, int collisionType = -1);
	//gets a collision shape by UID
	CCollisionShape*		GetCollisionShapeByUID(UINT32 nUID);
	//spawn a new collision box
	CCollisionShape*		SpawnCollisionShape(int nType, D3DXVECTOR2 vMin, D3DXVECTOR2 vMax);
	///--- pt vizualizare ---
	IActiveInterface		*m_camTargetActive;		//la ce activ se uita camera sau null cand se uita la players
	IActiveInterface		*m_camTargetOld;		//tine minte pe ce a fost locked ca sa se poata intoarce
	CCameraTransform		m_camLevel;
	D3DXVECTOR2				m_vCamPosDefault;		//camera position when not locked on special actors (hidden rooms, etc)
///--- misc ---
	// Returns the number of XP points gained after current mission
	int						Local_ComputeMissionXP(int nStars);

///-- update/paint --	
	// Main level Update
	void					Update(float dTime_original);

	//deseneaza in back buffer toate elementele iluminabile
	HRESULT					PaintOffscreen();
	//just clears the render targets
	HRESULT					PaintOffscreen_nothing();
	//deseneaza in compositing RT varianta finala a jocului, inainte de efectele de apa, foc, explo
	HRESULT					PaintComposition();
	//just clears the compositing buffer
	HRESULT					PaintComposition_nothing();

	bool					m_bPaintBackground;		//daca sa deseneze sau nu nivelul
	int						m_BackAnimIdx;			//index animatie din BSX folosit la desenarea nivelului
    // paints level background
	void					PaintBackground();
	// level paint into composition texture 
	void					Paint();
	// paints final res effects (water, distortion, icons, etc)
	HRESULT					PaintUsingFinalRTT();
	
///--- interfaces ---
	CSpriteCollection		m_sprInterface;
	//interfata in sine
	CCustomInterfaceIGM		m_interfaceIGM;
	//controlul de ingame hints
	CCustomInterfaceTextBubble m_interfaceTextBubble;

///--- SCRIPT ---
	//activeaza cel mai apropiat obiect, primul gasit
	void					TouchClosestActive(CActor * pToucherAct, float dTime);
	//porneste un script si salveaza toate datele necesare in active. Trimite si lista de params din AI ca var locale in script
	void					StartScript(WCHAR* scriptName, IActiveInterface * active);
	void					StartScript(UINT32 scriptNameHash, IActiveInterface * active);
	//proceseaza local instructiunile venite din script
	//RETURNS: true - instr processed, false - not processed
	bool					ProcessScriptInstruction(CScriptInstruction *instr, UINT32 executorUID, UINT32 scriptUID);
	bool					OnScriptFinished(UINT32 executorUID, UINT32 scriptUID, CVariantCollection * pArrScriptVars);
	char const *			GetProcessorName(void) { return "CLevel"; }
	//functie ajutatoare pentru procesare instructiuni. Gaseste activ in fn de valoare parametru: SELF pt caller, TARGET pentru targetID si numar pt ID efectiv
	IActiveInterface*		ScriptGetActiveInterfaceByTargetParam(CVariantComplex* vcTarget, UINT32 executorUID);
private: //--- misc functions ---
	COccluder				m_occluders[K_LVL_MAX_OCCLUDERS_CNT];	 //stiva locala folosita de AddOccludersFromAABB
	int						m_occludersCnt;
	//cauta in lista de AABBs vizibile si le face clip la cel specificat (pt algoritmul de iluminare cu poly)
	COccluder*				GetVisibleAABBs_toOccluders(D3DXVECTOR2 viewPos, CAABB * viewRect, int & retOccludersCnt);
	void					AddOccludersFromAABB_stencil(D3DXVECTOR2 viewerPos, CAABB * aabb); //ver cu stencil
	int						BuildShadowVolume(CLight * light, CAABB * visibleAABB, COccluder * p_arrOccluders, int nOccludersCount, _VERTEX_PNCT4T4 *outVerts, int outVertsMaxCnt);

public: //--- framework methods ---
	HRESULT OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL, void* pUserContext = NULL);
	HRESULT OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL, void* pUserContext = NULL);
	HRESULT OnLostDevice( void* pUserContext = NULL);
	HRESULT OnDestroyDevice( void* pUserContext = NULL);
};
