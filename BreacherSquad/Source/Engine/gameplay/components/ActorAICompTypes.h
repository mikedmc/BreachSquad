#pragma once

#include "../ActorTypes.h"

class CActor;
///--------------------------------------------------------------------------
/// BEHAVIORS and AI STATES
///--------------------------------------------------------------------------

//this enum must be sincronizat with AI_states list (level.cpp)
enum EAIstate
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
	K_AI_STATE_COLL_BREAKABLE_DOOR,		//params: f_life-can be damaged by bullets, b_reinforced=1 only the SAW can break it
	K_AI_STATE_COLL_BREAKABLE_WINDOW,	//params: f_life - daca specifici life inseamna ca se poate sparge cu gloante
	K_AI_STATE_COLL_KILL_ACTORS,		//params: b_killPlayer, b_killOthers
	///--- actives ---
	K_AI_STATE_ACTIVE_EXPLO_TRAP,		//no param
	K_AI_STATE_ACTIVE_CHECKPOINT,		//param: n_isFirst(0/1) - default first spawn point
	K_AI_STATE_ACTIVE_TEAM_TELEPORTER_2FRAMES, //param: f_SlowTimeDuration, b_EnterHiddenRoom, b_DontChangeFrames, f_teleportDuration, s_openSnd, s_closeSnd
	K_AI_STATE_ACTIVE_DOORFACE_AUTOCLOSE,	//param: s_openSnd, s_closeSnd, b_DontChangeFrames
	K_AI_STATE_ACTIVE_DOOR_SECTION,			//param: n_locked, f_lockpickTime

	K_AI_STATE_ACTIVE_AMMO_BOX,			//param: n_ammoLeft
	K_AI_STATE_ACTIVE_HEALTH_BOX,		//param: n_healthLeft
	K_AI_STATE_ACTIVE_BOMB,				//param: f_explodeTimerSec
	///--- ACTORS ---
	//nu avem stari pentru actori - sunt tratate cu behaviors
	//states no
	K_AI_STATES_CNT
};


///--- AI STATES/FUNCTIONS ---
//possible states - must be in editor/Data/behaviors.txt too
static const CStringHash EAIstate_names[] = {
	///--- GENERIC FUNCTIONS ---
	L"AI_FN_POS_ELLIPSE",
	L"AI_FN_ANG_SIN_TIME",
	L"AI_FN_ALPHA_SIN_TIME", //face alpha intre min si max in fn de sin(t + dt)
	L"AI_FN_GET_TARGET_POS",
	L"AI_FN_GET_TARGET_ANG", //ia unghiul targetului (relativ la cel actual al lui) si pastraza pozitia fata de originea lui
	L"AI_FN_FOLLOW_TARGET_RAIL",
	L"AI_FN_TOUCH_WHEN_SEE_PLAYER", //cand vede playerul in unghiul solid setat din params face touch la target
	///--- LIGHTS ---
	L"AI_FN_LIGHT_FLICKER1",
	L"AI_FN_LIGHT_ANG_CONE_XZ_TIME", //roteste directia luminii pe un con cu varful pe Y si baza pe XZ (doar luminile au directie 3D si doar cele IES o folosesc)
	///--- TRIGGERS ---
	L"AI_TRIGGER_IN_OUT", //executa script name pe intrare si pe iesire (face touch la target)
	///--- PARTICLE SYSTEM ---
	L"AI_PARTICLES_GENERATOR", //pentru generatoarele de particule
	///--- COLLISION BOXES ---
	L"AI_COLL_BREAKABLE_DOOR", //pentru collShapes care se sparg de la charge si shotgun. va seta automat animatia usii pe cea de distrugere
	L"AI_COLL_BREAKABLE_WINDOW", //pentru collShapes care se sparg de la gloante. va seta automat frame-ul urmator al animatiei
	L"AI_COLL_KILL_ACTORS", //kills actors inside of it
	///--- PROPS ---
	L"AI_ACTIVE_EXPLO_TRAP",	//capcana care explodeaza cand se intersecteaza bboxuul ei cu playerul
	L"AI_ACTIVE_CHECKPOINT",	//AI special pentru checkpoints - verifica intersectia cu personajul si lanseaza script
	L"AI_ACTIVE_TEAM_TELEPORTER_2FRAMES", //AI pentru usile de team teleport optional (pot intra toti sau doar cativa)
	L"AI_ACTIVE_DOORFACE_AUTOCLOSE", //AI pentru usile din fundal care stau deschise cat timp AItimer1>0.0f (ca si TELEPORTER_2FRAMES)
	L"AI_ACTIVE_DOOR_SECTION", //used for section doors (locked or unlocked)

	L"AI_ACTIVE_AMMO_BOX",	//AI pentru ammo boxes
	L"AI_ACTIVE_HEALTH_BOX",	//AI pentru ammo boxes
	L"AI_ACTIVE_BOMB",		//AI pentru bombele ce trebuiesc dezactivate
	///--- ACTORS ---
	//no ACTOR states (they have special AI class)
};
//Don't forget to add the state in AI_STATE enum too (level.h)

enum EAIBehaviorType
{
	AI_BEHAVIOR_EMPTY = -1,

	AI_BEHAVIOR_IDLE = 0,
	AI_BEHAVIOR_IDLE_CROUCHED = 1,  //ca si idle dar cu comanda de crouch
	AI_BEHAVIOR_HOSTAGE,			//hostage behavior
	AI_BEHAVIOR_SET_ANIMSET,		//seteaza setul curent de animatii
	AI_BEHAVIOR_SET_CAPS,			//seteaza capabilitatile din EActorCapabilitiesFlags
	AI_BEHAVIOR_PATROL,				//patrols around
	AI_BEHAVIOR_PATROL_BREAK_DOORS,	//patrols around but attacks doors if weapon permits it
	AI_BEHAVIOR_RUN_AWAY,			//runs from triggering event (can open unlocked doors)
	AI_BEHAVIOR_HOLD_POSITION,		//alert but not moving
	AI_BEHAVIOR_SURPRISED,			//se blocheaza o perioada
	AI_BEHAVIOR_ATTACK,				//atac normal cu o singura arma
	AI_BEHAVIOR_ATTACK_BACKSTAB,	//atac normal dar daca te prinde cu spatele trage cu alt weapon (dedicat lui deadly machete)
	AI_BEHAVIOR_ATTACK_COVER,		//atac normal dar cauta si cover si sta in cover daca poate
	AI_BEHAVIOR_ATTACK_HITNRUN,		//atac normal dar fuge cat timp nu are arma pregatita
	AI_BEHAVIOR_WAIT_FOR_ACTION,	//folosit cand are loc eventul (de obicei breach). se pozitioneaza corect si asteapta sa apara playerul sau da timeout.
	AI_BEHAVIOR_DETONATE_NEARBY,	//detonates a nearby target when animation reaches action flag frame
	AI_BEHAVIOR_WAIT,				//asteapta un timp (folosit pt surprizeTimer de exemplu)
	AI_BEHAVIOR_CHANGE_COLOR,		//changes color
	AI_BEHAVIOR_BARREL_EXPLODING,
	AI_BEHAVIOR_SHOW_ENEMY,			//shows enemy to nearby foes
	AI_BEHAVIOR_HUMAN_SHIELD_ATTACK, //special pentru LEADER LEMMY - poate lua ostatec ca human shield
	AI_BEHAVIOR_GUNPOINT_HOSTAGE,	//cauta ostatec, se duce la el si il tine at gunpoint
	AI_BEHAVIOR_GET_IN_COVER,		//looks for cover and goes there. 

	AI_BEHAVIOR_FLY_AWAY,			//crow flies away

	AI_BEHAVIOR_PLAY_ANIM,			//face play la animatie pana se termina animatia sau fBehaviorDuration
	AI_BEHAVIOR_RUN_SCRIPT,			//ruleaza scriptul setat in AI-ul starii, nu asteapta sa se termine
	AI_BEHAVIOR_GENERATE_EFFECT,	//genereaza efect din lista de efecte ELVLEffectType
	AI_BEHAVIOR_PLAY_VERSE,			//face play unui vers si iese imediat

	AI_BEHAVIOR_PLAYER_CONTROL,		 //actorul este controlat de player
	AI_BEHAVIOR_IN_LIMBO,			//actorul trece pe alpha 0 si fara input control si revine cand se iese din camera ascunsa
	AI_BEHAVIOR_TELEPORT_TO_TOUCHABLE_TARGET, //folosit la teleportarea solo: se teleporteaza la targetul obiectului curent atins (activat cu UP)

	AI_BEHAVIOR_SET_STATE, //schimba pe o stare anume

	AI_BEHAVIOR_FLEE,		//runs away from target or last event if target not visible
	AI_BEHAVIOR_BLIND_RUN,  //alearga haotic
	AI_BEHAVIOR_SUICIDE,	//se sinucide.
	AI_BEHAVIOR_DEAD,
	//count them
	AI_BEHAVIORS_CNT
};
//stringurile de aici corespund cu valorile de deasupra (necesare la parsare)
const CStringHash EAIBehaviorTypeNames[] = {
	L"AI_BEHAVIOR_IDLE",
	L"AI_BEHAVIOR_IDLE_CROUCHED",
	L"AI_BEHAVIOR_HOSTAGE",
	L"AI_BEHAVIOR_SET_ANIMSET",
	L"AI_BEHAVIOR_SET_CAPS",
	L"AI_BEHAVIOR_PATROL",
	L"AI_BEHAVIOR_PATROL_BREAK_DOORS",
	L"AI_BEHAVIOR_RUN_AWAY",
	L"AI_BEHAVIOR_HOLD_POSITION",
	L"AI_BEHAVIOR_SURPRISED",
	L"AI_BEHAVIOR_ATTACK",
	L"AI_BEHAVIOR_ATTACK_BACKSTAB",
	L"AI_BEHAVIOR_ATTACK_COVER",
	L"AI_BEHAVIOR_ATTACK_HITNRUN",
	L"AI_BEHAVIOR_WAIT_FOR_ACTION",
	L"AI_BEHAVIOR_DETONATE_NEARBY",
	L"AI_BEHAVIOR_WAIT",
	L"AI_BEHAVIOR_CHANGE_COLOR",
	L"AI_BEHAVIOR_BARREL_EXPLODING",
	L"AI_BEHAVIOR_SHOW_ENEMY",
	L"AI_BEHAVIOR_HUMAN_SHIELD_ATTACK",
	L"AI_BEHAVIOR_GUNPOINT_HOSTAGE",
	L"AI_BEHAVIOR_GET_IN_COVER",

	L"AI_BEHAVIOR_FLY_AWAY",

	L"AI_BEHAVIOR_PLAY_ANIM",
	L"AI_BEHAVIOR_RUN_SCRIPT",
	L"AI_BEHAVIOR_GENERATE_EFFECT",
	L"AI_BEHAVIOR_PLAY_VERSE",

	L"AI_BEHAVIOR_PLAYER_CONTROL",
	L"AI_BEHAVIOR_IN_LIMBO",
	L"AI_BEHAVIOR_TELEPORT_TO_TOUCHABLE_TARGET",

	L"AI_BEHAVIOR_SET_STATE",

	L"AI_BEHAVIOR_FLEE",
	L"AI_BEHAVIOR_BLIND_RUN",
	L"AI_BEHAVIOR_SUICIDE",
	L"AI_BEHAVIOR_DEAD",
};

//!!! CAIBehavior needs to be copyable. CVariantMap has copy constructor.
class CAIBehavior
{
public:
	EAIBehaviorType	nType;
	bool			bCanInterrupt;			// if false it keeps current behavior until finished. Doesn't check sensor info. Ai grija daca moare in timpul asta.
	bool			bIgnoreEvents;			// ignores AI events (apart from EVENT_DEAD)
	float			fBehaviorDuration;		// for timed states

	CVariantMap	m_vcolParams;

	CAIBehavior() : nType( AI_BEHAVIOR_EMPTY ), bCanInterrupt( true ), bIgnoreEvents( false ), fBehaviorDuration( -1.0f )
	{
		m_vcolParams.Clear();
	}
};


///--------------------------------------------------------------------------
/// AI events
/// - sunetele de exemplu si alte eventuri care alerteaza AI-urile
///--------------------------------------------------------------------------

//lista este in functie de prioritati
enum EAIEventType
{
	//folosit la filtre
	K_LVL_AI_EVENT_ANY = -2,
	//empty event. folosit la filtru la GetClosestAIEvent
	K_LVL_AI_EVENT_NONE = -1,
	//event folosit pentru schimbarea starilor (Cand nu e nici un input interesant)
	//nu e inclus in eventurile ANY
	K_LVL_AI_EVENT_IDLE_TICK = 0,
	///--- De aici incep eventurile propriuzise (ANY filter) ---
	//event trimis de obicei din script, cu raza mica (pentru hostages mai ales)
	K_LVL_AI_EVENT_SCRIPT_TRIGGER,
	//lumini care se aprind si se sting, etc
	K_LVL_AI_EVENT_STRANGE,
	K_LVL_AI_EVENT_SOUND_THREAT,
	K_LVL_AI_EVENT_SOUND_EXPLOSION,
	//players shooting doors usually
	K_LVL_AI_EVENT_SOUND_BEHIND_DOOR,
	//grav, atunci cand se aude o usa
	K_LVL_AI_EVENT_SOUND_DOOR_BREACHING,
	//f grav, atunci cand este impuscat
	K_LVL_AI_EVENT_GOT_HIT,
	//ff grav, cand isi ia melee
	K_LVL_AI_EVENT_GOT_MELEED,

	//urmeaza eventuri care nu prea se adauga in lista ci sunt intoarse de senzorii actorului
	K_LVL_AI_EVENT_LOST_ENEMY,	//cand pierde inamicul
	K_LVL_AI_EVENT_TOUCH_ENEMY, //cand atinge inamicul pe care il vede
	K_LVL_AI_EVENT_SEE_ENEMY,	//cand vede inamicul
	K_LVL_AI_EVENT_LOW_HEALTH,	//cand mai ramane doar putin din energie (trimis o singura data)
	K_LVL_AI_EVENT_DEAD,		//cand moare
	//count them
	K_LVL_AI_EVENTS_CNT,
};
const CStringHash EAIEventTypeNames[] = {
	L"AI_EVENT_IDLE_TICK",
	L"AI_EVENT_SCRIPT_TRIGGER",
	L"AI_EVENT_STRANGE",
	L"AI_EVENT_SOUND_THREAT",
	L"AI_EVENT_SOUND_EXPLOSION",
	L"AI_EVENT_SOUND_BEHIND_DOOR",
	L"AI_EVENT_SOUND_DOOR_BREACHING",
	L"AI_EVENT_GOT_HIT",
	L"AI_EVENT_GOT_MELEED",

	L"AI_EVENT_LOST_ENEMY",
	L"AI_EVENT_TOUCH_ENEMY",
	L"AI_EVENT_SEE_ENEMY",
	L"AI_EVENT_LOW_HEALTH",
	L"AI_EVENT_DEAD",
};

class CAIEvent
{
public:
	UINT32			ownerUID;   // UID of event raiser. 0 - generic/not set;
	int				ownerClass;	// class of event raiser
	EAIEventType	nType;		// event type
	float			fRadius;
	float			fDuration;
	Vec2			pos;

	CAIEvent() :
		nType( K_LVL_AI_EVENT_NONE ),
		ownerUID( 0 ), ownerClass( -1 ),
		fRadius( 0.0f ), fDuration( 0.0f )
	{}

	CAIEvent( EAIEventType eventType, UINT32 evtOwnerUID, int evtOwnerClass, Vec2 vPos, float radius, float duration = 0.6f ) :
		nType( eventType ), ownerUID( evtOwnerUID ), ownerClass( evtOwnerClass ), pos( vPos ), fRadius( radius ), fDuration( duration )
	{}

	inline bool operator!=( const CAIEvent& rhs )
	{
		// checks just a few of the main properties of the event
		return nType != rhs.nType || ownerUID != rhs.ownerUID || ownerClass != rhs.ownerClass;
	}

	void Set( EAIEventType eventType, UINT32 evtOwnerUID, int evtOwnerClass, Vec2 vPos, float radius, float duration = 0.5f )
	{
		nType = eventType;
		ownerUID = evtOwnerUID;
		ownerClass = evtOwnerClass;
		pos = vPos;
		fRadius = radius;
		fDuration = duration;
	}

	void Reset()
	{
		nType = K_LVL_AI_EVENT_NONE;
		ownerUID = 0;
		ownerClass = 0;
		pos = Vec2( 0.0f, 0.0f );
		fRadius = 0.0f;
		fDuration = 0.0f;
	}
};

///--- AI STATES ---
#define K_LVL_MAX_STATE_BEHAVIORS_CNT 32
#define K_LVL_MAX_STATE_EVENTS_CNT 32
class CAIState
{
public:
	CStringHash		name;				// name of state
	int				nPriority;			// state priority if more states get triggered by the same events
	float			fProbability;		// state probability if we have state variation on same priority

	CFixedArray<CAIBehavior, K_LVL_MAX_STATE_BEHAVIORS_CNT> m_arrBehaviors;				// state behaviors, executed in a loop
	CFixedArray<EAIEventType, K_LVL_MAX_STATE_EVENTS_CNT> m_arrTriggeringEventTypes;	// types of events triggering the current state

	CAIState();
	~CAIState();
};

// AI template loaded from the AI xml
class CAITemplate
{
public:
	CArray<CAIState*>			m_arrStates;			// all states in AI
	CArray<EAIEventType>		m_arrIgnoredEvents;		// list of ignored events
	//CTOR/DTOR
	~CAITemplate();
	// Finds best State based on input event and random numbers generator for states probabilities
	CAIState* GetHighestPriorityState( EAIEventType evtType, CRandom* pRandomGen );
	// Finds AI state with name
	CAIState* GetAIStateByName( CStringHash strName );
};


// Holds all info that comes into the actor sensors
class CAISensorInfo
{
public:
	// internal weapon state
	enum EWpnSensorState {
		UNAVAILABLE = 0,
		CAN_SHOOT = 1,
		NEEDS_RELOAD = 2,
	};
public:
	bool				m_bEnabled;			// sensors are enabled or disabled?
	//external sensors
	CActor*				pTargetedActor;		// visible enemy, set by internal sensors
	UINT32				m_lastInteractingActorUID;	//0-not set or UID for last actor that he interacted with
	float				fTimeSinceHit;		//time passed since got hit
	CAIEvent			evtInternal;		// internal event given by sensors (see enemy, got shot etc). Don't use for decisions, updated by sensors.
	// weapon status sensors
	EWpnSensorState		WpnStatePrimary;	// status of primary weapon
	EWpnSensorState		WpnStateSecondary;  // status of secondary weapon (usually grenades)
	Vec2				vGoTo;				// target destination for current actor ({0.0, 0.0} means not set)

	CAIEvent			evt;				// current event on which actor is making decisions (chosen between evtInternal and level AI events)

	CAISensorInfo();

	void Reset();
};

// Commands that get sent to the AIs
class CAICommands
{
public:
	bool				bThrust;  
	Vec2				vMoveDir;
	Vec2				vAimVec;	// set on 0.0 for no aim command (old aim vec will be kept)

	bool				bRunning;
	bool				bCrouched;
	bool				bJump;
	bool				bInteract;				// interact command
	//EActorAnims			eOverrideAnim;	//if not empty, overrides actor animation

	EActorDeathCommand	nDeathCommand; //0-not dead, 1-dead, 2-splat, 3-splat+explode
	EActorAttackState	eAttackCommand;
	EActorAttackState	eAttackCommand_last; //last attack command
	//color command: !=0 means color command is active
	DWORD				nColor;

	CAICommands()
	{
		Reset();
	}

	void Reset()
	{
		bThrust = false;
		vMoveDir = { 0.0f, 0.0f };
		vAimVec = { 0.0f, 0.0f };

		bRunning = false;

		bCrouched = false;
		bJump = false;
		bInteract = false;
		nColor = 0;

		nDeathCommand = K_LVL_ACT_DEATHCMD_NONE;
		eAttackCommand = K_ACT_ATTACK_IDLE;
		eAttackCommand_last = K_ACT_ATTACK_IDLE;
		//eOverrideAnim = K_LVL_ACT_ANIM_EMPTY;
	}

	void ResetMoveCommands()
	{
		bThrust = false;
		bRunning = false;
		vMoveDir = { 0.0f, 0.0f };

		bCrouched = false;
		bJump = false;
		bInteract = false;
		eAttackCommand = K_ACT_ATTACK_IDLE;
	}
};


