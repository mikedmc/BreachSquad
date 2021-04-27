#pragma once

//anunt clasele necesare
class CCollisionShape;

///--------------------------------------------------------------------------
///strategic abilities enum (trebuie sa corespunda iconurilor din IGM_STRATEGIC_BAR_ICONS)
///--------------------------------------------------------------------------
enum eStrategicAbility
{
	K_CI_STRATEGIC_NONE = -1,

	K_CI_STRATEGIC_BODY_ARMOR = 0,
	K_CI_STRATEGIC_GEAR_REFILL = 1,
	K_CI_STRATEGIC_MEDIKIT,
	K_CI_STRATEGIC_REINFORCEMENT,
	K_CI_STRATEGIC_EXTRA_LIFE,
	K_CI_STRATEGIC_SNIPER_SUPPORT,
	K_CI_STRATEGIC_ACTIVE_SNIPER_FIRE,
	K_CI_STRATEGIC_DRAGON_BREATH,
	K_CI_STRATEGIC_SUBMACHINEGUN,
	K_CI_STRATEGIC_BREACHER_SAW,
	K_CI_STRATEGIC_FBI_AKIMBO,
	K_CI_STRATEGIC_FBI_MP5K,
	K_CI_STRATEGIC_MK48MOD1,		//heavy weapon
	K_CI_STRATEGIC_RECON_MARKSMAN,
	K_CI_STRATEGIC_OFFDUTY_GARAND,	//huntingrifle
	K_CI_STRATEGIC_RECON_SIX12SD,   //shotgun for recon
	K_CI_STRATEGIC_HOMEMADE_PIE,

	K_CI_STRATEGIC_COUNT
};
//names must correspond to the ones used in gear_screen.xml
const CStringHash eStrategicAbilityNames[] = {
	L"SA_BODY_ARMOR",
	L"SA_GEAR_REFILL",
	L"SA_MEDIKIT",
	L"SA_REINFORCEMENT",
	L"SA_EXTRA_LIFE",
	L"SA_SNIPER_SUPPORT",
	L"SA_ACTIVE_SNIPER_FIRE",
	L"SA_DRAGON_BREATH",
	L"SA_SUBMACHINEGUN",
	L"SA_BREACHER_SAW",
	L"SA_FBI_AKIMBO",
	L"SA_FBI_MP5K",
	L"SA_MK48MOD1",
	L"SA_RECON_MARKSMAN",
	L"SA_OFFDUTY_GARAND",
	L"SA_RECON_SIX12SD",
	L"SA_HOMEMADE_PIE",
};


///--------------------------------------------------------------------------
/// EFFECT TYPES
///--------------------------------------------------------------------------
enum ELVLEffectType {
	K_LVL_EFFECT_EMPTY = -1,

	K_LVL_EFFECT_ELECTRIC_BREAK_SPARKS = 0,
	K_LVL_EFFECT_STARS_CONFETTI = 1,
	K_LVL_EFFECT_EXPLO_LARGE,
	K_LVL_EFFECT_STONE_BREAK,

	K_LVL_EFFECTS_CNT,
};

const CStringHash ELVLEffectTypeNames[] = {
	L"EFFECT_ELECTRIC_BREAK_SPARKS",
	L"EFFECT_STARS_CONFETTI",
	L"EFFECT_EXPLO_LARGE",
	L"EFFECT_STONE_BREAK", 
};

///----------------------------------------------------------------------------------
/// RENDER PASSES that the level needs/does
///----------------------------------------------------------------------------------
enum eLVLRenderPass {
	K_LVL_RP_NONE = -1,
	K_LVL_RP_COLORS = 0,
	K_LVL_RP_NORMALS_HEIGHT = 1,
	// renders all lights in a single surface (with shadows)
	K_LVL_RP_LIGHTS,
	// some elements need shadows to be painted (some bullets, props, actors)
	K_LVL_RP_SHADOWS,

	K_LVL_RP_COUNT
};

///--------------------------------------------------------------------------
/// MATERIAL TYPES
///--------------------------------------------------------------------------
enum EMaterialType
{
	K_LVL_MATERIAL_UNKNOWN = -1,

	K_LVL_MATERIAL_FLESH = 0,
	K_LVL_MATERIAL_METAL = 1,
	K_LVL_MATERIAL_WALL,

	K_LVL_MATERIALS_COUNT
};
const CStringHash EMaterialTypeNames[] =
{
	L"FLESH",
	L"METAL",
	L"WALL",
};


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
	L"AI_COLL_FOG_OF_WAR",	//pentru fog of war. Dispare cu alpha cand devine vizibila camera
	L"AI_COLL_BREAKABLE_DOOR", //pentru collShapes care se sparg de la charge si shotgun. va seta automat animatia usii pe cea de distrugere
	L"AI_COLL_BREAKABLE_WINDOW", //pentru collShapes care se sparg de la gloante. va seta automat frame-ul urmator al animatiei
	L"AI_COLL_KILL_ACTORS", //kills actors inside of it
	///--- PROPS ---
	L"AI_ACTIVE_SWINGING_FRONTOBJ",	//interactioneaza cu grenada si se balanseaza
	L"AI_ACTIVE_EXPLO_TRAP",	//capcana care explodeaza cand se intersecteaza bboxuul ei cu playerul
	L"AI_ACTIVE_CHECKPOINT",	//AI special pentru checkpoints - verifica intersectia cu personajul si lanseaza script
	L"AI_ACTIVE_TEAM_TELEPORTER_2FRAMES", //AI pentru usile de team teleport optional (pot intra toti sau doar cativa)
	L"AI_ACTIVE_DOORFACE_AUTOCLOSE", //AI pentru usile din fundal care stau deschise cat timp AItimer1>0.0f (ca si TELEPORTER_2FRAMES)
	L"AI_ACTIVE_DOOR_SECTION", //used for section doors (locked or unlocked)

	L"AI_ACTIVE_AMMO_BOX",	//AI pentru ammo boxes
	L"AI_ACTIVE_HEALTH_BOX",	//AI pentru ammo boxes
	L"AI_ACTIVE_BOMB",		//AI pentru bombele ce trebuiesc dezactivate
	L"AI_ACTIVE_ZOMBIE_SPAWNER", //AI for the zombie spawner
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
	AI_BEHAVIOR_TELEPORT_NEAR_ENEMY,//teleports in range of the enemy (with teleport_in and _out anims if available)
	AI_BEHAVIOR_SURPRISED,			//se blocheaza o perioada
	AI_BEHAVIOR_FIND_CLOSEST_PLAYER,	//se duce la pozitia celui mai apropiat player chiar daca nu-l vede. !trece si prin usi teleportoare daca nu e pe acelasi palier mesajul
	AI_BEHAVIOR_ATTACK,				//atac normal cu o singura arma
	AI_BEHAVIOR_ATTACK_BACKSTAB,	//atac normal dar daca te prinde cu spatele trage cu alt weapon (dedicat lui deadly machete)
	AI_BEHAVIOR_ATTACK_COVER,		//atac normal dar cauta si cover si sta in cover daca poate
	AI_BEHAVIOR_ATTACK_HITNRUN,		//atac normal dar fuge cat timp nu are arma pregatita
	AI_BEHAVIOR_BIGSHOT_ATTACK,		//attack for Bigshot
	AI_BEHAVIOR_JACKEDJONES_ATTACK,	//attack sequence of jacked up jones boss
	AI_BEHAVIOR_SHIELDBOSS_ATTACK,	//attack sequence for the shield boss
	AI_BEHAVIOR_TATTOOBOSS_ATTACK,	//attack sequence for the tattoo boss
	AI_BEHAVIOR_ESCAPE_ARREST,		//run away from player, not shooting, can open doors
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
	
	AI_BEHAVIOR_PLAYER_CONTROL,	 //actorul este controlat de player
	AI_BEHAVIOR_PLAYER_TEAM_TELEPORT, //actorul foloseste usa ca sa intre in alta camera (timer de asteptare pentru multiplayer)
	AI_BEHAVIOR_IN_LIMBO,	//actorul trece pe alpha 0 si fara input control si revine cand se iese din camera ascunsa
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
	L"AI_BEHAVIOR_TELEPORT_NEAR_ENEMY",
	L"AI_BEHAVIOR_SURPRISED",
	L"AI_BEHAVIOR_FIND_CLOSEST_PLAYER",
	L"AI_BEHAVIOR_ATTACK",
	L"AI_BEHAVIOR_ATTACK_BACKSTAB",
	L"AI_BEHAVIOR_ATTACK_COVER",
	L"AI_BEHAVIOR_ATTACK_HITNRUN",
	L"AI_BEHAVIOR_BIGSHOT_ATTACK",
	L"AI_BEHAVIOR_JACKEDJONES_ATTACK",
	L"AI_BEHAVIOR_SHIELDBOSS_ATTACK",
	L"AI_BEHAVIOR_TATTOOBOSS_ATTACK",
	L"AI_BEHAVIOR_ESCAPE_ARREST",
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
	L"AI_BEHAVIOR_PLAYER_TEAM_TELEPORT",
	L"AI_BEHAVIOR_IN_LIMBO",
	L"AI_BEHAVIOR_TELEPORT_TO_TOUCHABLE_TARGET",

	L"AI_BEHAVIOR_SET_STATE",

	L"AI_BEHAVIOR_FLEE",
	L"AI_BEHAVIOR_BLIND_RUN",
	L"AI_BEHAVIOR_SUICIDE",
	L"AI_BEHAVIOR_DEAD",
};

//!!! CAIBehavior needs to be copyable. CVariantCollection has copy constructor.
class CAIBehavior
{
public:
	EAIBehaviorType	nType;
	bool			bCanInterrupt;			// if false it keeps current behavior until finished. Doesn't check sensor info. Ai grija daca moare in timpul asta.
	bool			bIgnoreEvents;			// ignores AI events (apart from EVENT_DEAD)
	float			fBehaviorDuration;		// for timed states

	CVariantCollection	m_vcolParams;

	CAIBehavior() : nType(AI_BEHAVIOR_EMPTY), bCanInterrupt(true), bIgnoreEvents(false), fBehaviorDuration(-1.0f)
	{
		m_vcolParams.DeleteAll();
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
	UINT32	ownerUID;  //cel care a lansat eventul. 0 - invalid/not set;
	UINT32  targetUID;	// 0 -not set; folosit pentru eventuri targetate (GOT_SHOT, STUNNED, etc)
	int		ownerClass;	//clasa celui care a facut eventul

	EAIEventType	nType;		//tipul eventului, include si cat de grav este
	float			fRadius;
	float			fDuration;	//durata event

	Vec2		pos;		//pozitia eventului
	
	//CTOR/DTOR
	CAIEvent() :
		nType(K_LVL_AI_EVENT_NONE),
		ownerUID(0), ownerClass(-1), targetUID(0),
		fRadius(0.0f), fDuration(0.0f)
	{}

	CAIEvent(EAIEventType eventType, UINT32 evtOwnerUID, int evtOwnerClass, Vec2 vPos, float radius, float duration = 0.6f, UINT32 evtTargetUID = 0) :
		nType(eventType), ownerUID(evtOwnerUID), ownerClass(evtOwnerClass), pos(vPos), fRadius(radius), fDuration(duration), targetUID(evtTargetUID)
	{}

	void Set(EAIEventType eventType, UINT32 evtOwnerUID, int evtOwnerClass, Vec2 vPos, float radius, float duration = 0.6f, UINT32 evtTargetUID = 0)
	{
		nType = eventType; 
		ownerUID = evtOwnerUID; 
		ownerClass = evtOwnerClass; 
		pos = vPos; 
		fRadius = radius; 
		fDuration = duration; 
		targetUID = evtTargetUID;
	}
	void Reset()
	{
		nType = K_LVL_AI_EVENT_NONE;
		ownerUID = 0;
		ownerClass = 0;
		pos = Vec2(0.0f, 0.0f);
		fRadius = 0.0f;
		fDuration = 0.0f;
		targetUID = 0;
	}
};

///--- AI STATES ---
#define K_LVL_MAX_STATE_BEHAVIORS_CNT 32
#define K_LVL_MAX_STATE_EVENTS_CNT 32
//#TODO: starile AI pot fi mai multe intr-un AI group
class CAIState
{
public:
	CStringHash name;	//numele starii
	int	nPriority;  //prioritatea state-ului in cazul in care sunt mai multe activabile

	float fProbability; //probabilitatea state-ului cand sunt selectate mai multe de aceeasi prioritate
	CFixedArray<CAIBehavior, K_LVL_MAX_STATE_BEHAVIORS_CNT> m_arrBehaviors; //comportamentele din state, executate secvential
	CFixedArray<EAIEventType, K_LVL_MAX_STATE_EVENTS_CNT> m_arrTriggeringEventTypes; //tipurile de events care triggeruiesc starea curenta

	CAIState();
	~CAIState();
};

//TODO: de adaugat StateGroups cu probabilitati ca sa poti sa randomizezi AI (sa selecteze starea in fn de un random)

// AI template loaded from the AI xml
class CAITemplate
{
public:
	CGrowableArray<CAIState*>		m_arrStates;  //starile din care selecteaza 
	CGrowableArray<EAIEventType>	m_arrIgnoredEvents;	//list of ignored events
	//CTOR/DTOR
	~CAITemplate();
	//Finds best State based on input event and random numbers generator for states probabilities
	CAIState* GetHighestPriorityState(EAIEventType evtType, CRandom* pRandomGen);
	//Finds AI state with name
	CAIState* GetAIStateByName(CStringHash strName);
};

//clasa folosita in actori si incapsuleaza toate variabilele necesare pentru definirea totala a unui state
//aici se declara variabile locale necesare per actor, cum ar fi behaviorTimer 
//momentan sunt toate declarate in actor
/*
class CAIStateInstance
{
public:
	CAIState*		m_pAIcurrentState; //starea curenta de AI
	int				m_nAIcurrentBehaviorIdx; //indexul curent al behaviorului din state-ul curent (sau -1 cand nu e setat)
	float			m_fAIBehaviorTimer; //timer folosit la behaviors cu durata

	CAIStateInstance()
	{
		m_pAIcurrentState = null;
		m_nAIcurrentBehaviorIdx = -1;
		m_fAIBehaviorTimer = 0.0f;
	}
};
*/

///--------------------------------------------------------------------------
/// ACTORS
///--------------------------------------------------------------------------

///--- HITPOINT values filter ---
//pozitii relative heart, gun
#define K_LVL_ACTOR_HITPOINTFLAG_HEARTPOS 1
#define K_LVL_ACTOR_HITPOINTFLAG_GUNPOS 2
//ground sweep este punctul in care verifica daca are platforma in fatza (de obicei inamicii il au)
#define K_LVL_ACTOR_HITPOINTFLAG_GROUND_SWEEP 4

///--- constants ---
#define K_LVL_COVER_DAMAGE_ABSORBTION 0.8f

enum EActorDeathCommand {
	K_LVL_ACT_DEATHCMD_EMPTY = -1,

	K_LVL_ACT_DEATHCMD_NONE = 0,
	K_LVL_ACT_DEATHCMD_RESET_TO_ZERO = 1,
	K_LVL_ACT_DEATHCMD_RUNSCRIPT,
	K_LVL_ACT_DEATHCMD_SPLAT,
	K_LVL_ACT_DEATHCMD_DEALLOCATE,
	//list size
	K_LVL_ACT_DEATHCMD_CNT
};

const CStringHash EActorDeathCommandNames[] = {
	L"ACT_DEATHCMD_NONE",
	L"ACT_DEATHCMD_RESET_TO_ZERO",
	L"ACT_DEATHCMD_RUNSCRIPT",
	L"ACT_DEATHCMD_SPLAT",
	L"ACT_DEATHCMD_DEALLOCATE"
};

//ordinea e importanta:
enum EActorAttackState {
	K_LVL_ACT_ATTACK_IDLE,
	K_LVL_ACT_ATTACK_RELOADING,
	K_LVL_ACT_ATTACK_SHOOTING,
	K_LVL_ACT_ATTACK_SHOOTING_ALT,
	K_LVL_ACT_ATTACK_USING_GEAR,
	K_LVL_ACT_ATTACK_MELEE, 
	K_LVL_ACT_ATTACK_BREACH,  //used mostly for breaching doors
};

//iconurile afisate deasupra actorilor
enum EActorIconTypes
{
	K_LVL_ACT_ICON_REMOVE_ICON = -2, //folosit ca sa scoti iconul curent
	K_LVL_ACT_ICON_NONE = -1,

	K_LVL_ACT_ICON_SURPRISE = 0,
	K_LVL_ACT_ICON_QUESTION,
	K_LVL_ACT_ICON_THINKING,
	K_LVL_ACT_ICON_ARREST_ME,
	K_LVL_ACT_ICON_SCARED,

	K_LVL_ACT_ICONS_CNT
};


///--- ORDER IS VERY IMPORTANT ---
enum EActorClass
{
	K_LVL_ACT_CLASS_NOT_SET = -2,	//usually not used (only in initializations)
	K_LVL_ACT_CLASS_ANY = -1,		//folosita ca filtru la GetClosestTarget sau param gen NONE

	K_LVL_ACT_CLASS_PASSIVE = 0,	//butoaie, elemente care se sparg, etc
	K_LVL_ACT_CLASS_HOSTAGE = 1,	//special class for hostages
	K_LVL_ACT_CLASS_TRAP,		//clasa separata care distruge orice
	K_LVL_ACT_CLASS_EXPLOSION,		//clasa speciala de explozii pentru exploziile care distrug orice
	//from here only human-blood-stun classes (sorted by love from near to far) that kill each other
	K_LVL_ACT_CLASS_PLAYER,			//clasa player
	K_LVL_ACT_CLASS_FRIENDLY,		//main player friendly class
	//from here on you get SP on kills and they get pushed when too close (usually enemies)
	K_LVL_ACT_CLASS_HUMAN,			//human enemies
	K_LVL_ACT_CLASS_ZOMBIE,			//special zombie class
	//count
	K_LVL_ACT_CLASSES_COUNT
};

const CStringHash EActorClassNames[K_LVL_ACT_CLASSES_COUNT] =
{
	L"PASSIVE",
	L"HOSTAGE",
	L"TRAP",
	L"EXPLOSION",
	L"PLAYER",
	L"FRIENDLY",
	L"HUMAN",
	L"ZOMBIE"
};


///----- Similar to actor animations, sounds are loaded from actor templates and get called by action instead of sound names -----
enum EActorSoundVerse
{
	K_LVL_ACT_VERSE_EMPTY = -1,

	K_LVL_ACT_VERSE_START_GAME = 0, //first player to start the game
	K_LVL_ACT_VERSE_JOIN_GAME = 1,  //when joining the game or when playing as 2nd player
	K_LVL_ACT_VERSE_DIE,
	K_LVL_ACT_VERSE_USE_GRENADE,
	K_LVL_ACT_VERSE_USE_BREACHING_CHARGE,
	K_LVL_ACT_VERSE_USE_FLASHBANG,
	K_LVL_ACT_VERSE_USE_SMOKE,
	K_LVL_ACT_VERSE_RELOADING,
	K_LVL_ACT_VERSE_BOMB_LOCATED,
	K_LVL_ACT_VERSE_BOMB_DEFUSING,
	K_LVL_ACT_VERSE_BOMB_DEFUSED,
	K_LVL_ACT_VERSE_ROOM_CLEARED,		//room cleared
	K_LVL_ACT_VERSE_KILL_MADE,			//killed the enemy
	K_LVL_ACT_VERSE_TAKING_DAMAGE,
	K_LVL_ACT_VERSE_TAKE_HOSTAGE,
	K_LVL_ACT_VERSE_KILL_HOSTAGE,
	K_LVL_ACT_VERSE_THANK_YOU,
	K_LVL_ACT_VERSE_SURRENDER,
	K_LVL_ACT_VERSE_RUN_AWAY,
	K_LVL_ACT_VERSE_TAUNT,
	K_LVL_ACT_VERSE_TEASE,
	K_LVL_ACT_VERSE_LOCKPICK_START,			//lockpicking door

	K_LVL_ACT_VERSES_COUNT
};
//numarul maxim de seturi de versuri
#define K_LVL_ACT_VERSES_MAX_SETS 2
//timeout same verse
#define K_LVL_ACT_VERSES_TIMEOUT 4.0f

const CStringHash EActorSoundVerseNames[K_LVL_ACT_VERSES_COUNT] =
{
	L"START_GAME",
	L"JOIN_GAME",
	L"DIE",
	L"USE_GRENADE",
	L"USE_BREACHING_CHARGE",
	L"USE_FLASHBANG",
	L"USE_SMOKE",
	L"RELOADING",
	L"BOMB_LOCATED",
	L"BOMB_DEFUSING",
	L"BOMB_DEFUSED",
	L"ROOM_CLEARED",
	L"KILL_MADE",
	L"TAKING_DAMAGE",
	L"TAKE_HOSTAGE",
	L"KILL_HOSTAGE",
	L"THANK_YOU",
	L"SURRENDER",
	L"RUN_AWAY",
	L"TAUNT",
	L"TEASE",
	L"LOCKPICK_START"
};


///--------------------------------------------------------------------------
///--- DECALS : clasa folosita pentru afisarea urmelor pe pereti ---
///--------------------------------------------------------------------------
enum EDecalLayer
{
	K_LVL_DECAL_LAYER_FLOOR = 0,  
	K_LVL_DECAL_LAYER_WALLS = 1,  
	
	K_LVL_DECAL_LAYERS_CNT
};

//IMPORTANT: ca optimizare se pot face 2 arrays de decals in loc sa am param de layer pe fiecare decal!!!
class CDecal
{
public:
	EDecalLayer		layer;  //pe ce layer este
	CSprite			sprite; //sprite-ul animatiei (blood splats animate)
	CAABB			aabb;
	bool			bAnimated;	//for animated decals
};




const CStringHash EDoTTypeNames[] =
{
	L"DoT_NOEFFECT",
	L"DoT_CRIPPLED",
	L"DoT_INTIMIDATED",
	L"DoT_ZOMBIE_POISON",
	L"DoT_HEAL_LIFE",
	L"DoT_FIRE",
	L"DoT_TARGETED",
	L"DoT_TARGETED_ALLY",
	L"DoT_INVINCIBLE",
	L"DoT_SNIPER_TARGET",
};

class CDamageOverTime
{
public:
	enum EDoTType {
		//NONE always gets set to clear the effects
		K_LVL_DoT_NONE = -1,
		///DoT effects by priority: Sniper will overwrite fire/heal
		K_LVL_DoT_NOEFFECT = 0,
		K_LVL_DoT_CRIPPLED = 1,	//walks slower
		K_LVL_DoT_INTIMIDATED,	//intimidated - inflicts reduced damage
		K_LVL_DoT_ZOMBIE_POISON,		//hurts for a while
		K_LVL_DoT_HEAL_LIFE,			//heals for a while
		K_LVL_DoT_FIRE,
		K_LVL_DoT_TARGETED,		//used by the Recon class on enemies to lower their resistance (hardcoded: only works on enemies)
		K_LVL_DoT_TARGETED_ALLY,//used by the Recon class on friends to reduce damage
		K_LVL_DoT_INVINCIBLE,	//used on players after respawn
		K_LVL_DoT_SNIPER_TARGET,//sniper target effect, kills it when finished (ultimate effect)

		K_LVL_DoT_COUNT
	};

public:
	EDoTType	eType;			//folosit pentru efectul grafic
	float		fDuration;		//durata in secunde
	float		fDuration_ini;	//initial value for duration
	float		fDamagePerSec;	//damage sau heal pe secunda
	EActorClass	eExcludedActClass; //excluded class
	EActorClass	eFilteredActClass; //only this class counts
	UINT32		nOwnerUID;			//owner of the DoT (count as kill if not 0)

	float		fVar1;				//misc vars for each effect

	CDamageOverTime()
	{
		eType = K_LVL_DoT_NONE;
		fDuration = 0.0f;
		fDuration_ini = 0.0f;
		fDamagePerSec = 0.0f;
		eExcludedActClass = K_LVL_ACT_CLASS_ANY; //nu exclude nimic. Putin fortat ANY asta dar merge
		eFilteredActClass = K_LVL_ACT_CLASS_ANY; //don't filter anything
		nOwnerUID = 0;

		fVar1 = 0.0f;
	}

	bool Set(EDoTType eDoTType, float nfDuration = 0.0f, float nfDamagePerSec = 0.0f, EActorClass eExcludedClass = K_LVL_ACT_CLASS_ANY, EActorClass eFilterClass = K_LVL_ACT_CLASS_ANY, UINT32 unOwnerUID = 0)
	{
		if (nfDuration <= 0.0f)
		{
			eType = K_LVL_DoT_NONE;
			return true;
		}
		//reset old settings if necessary
		if (nfDuration <= 0.0f)
		{
			eType = K_LVL_DoT_NONE;
		}
		//don't set lower priority effect
		if (eType > eDoTType)
			return false;

		if ((eType != eDoTType) || (fDuration < nfDuration))
		{
			fDuration = nfDuration;
			fDuration_ini = nfDuration;
		}

		fVar1 = 0.0f;
		eType = eDoTType;
		fDamagePerSec = nfDamagePerSec;
		eExcludedActClass = eExcludedClass;
		eFilteredActClass = eFilterClass;
		nOwnerUID = unOwnerUID;

		return true;
	}

	void Reset()
	{
		eType = K_LVL_DoT_NONE;
		fDuration = 0.0f;
		fDuration_ini = 0.0f;
		fDamagePerSec = 0.0f;
		eExcludedActClass = K_LVL_ACT_CLASS_ANY;
		eFilteredActClass = K_LVL_ACT_CLASS_ANY; //don't filter anything
		nOwnerUID = 0;
	}
};


//numele predefinite ale exploziilor - pt particularizarile din AddProp_Explo
const UINT32 hash_EXPLO_CHARGE_INVISIBLE = FastHash(L"EXPLO_CHARGE_INVISIBLE");
const UINT32 hash_EXPLO_BARREL = FastHash(L"EXPLO_BARREL");
const UINT32 hash_EXPLO_GRENADE = FastHash(L"EXPLO_GRENADE");
const UINT32 hash_EXPLO_GRENADE_GROUND = FastHash(L"EXPLO_GRENADE_GROUND");
const UINT32 hash_EXPLO_LARGE = FastHash(L"EXPLO_LARGE");
const UINT32 hash_EXPLO_LARGE_XL = FastHash(L"EXPLO_LARGE_XL");
const UINT32 hash_EXPLO_FLASHBANG = FastHash(L"EXPLO_FLASHBANG");
const UINT32 hash_EXPLO_SHIELD_FLASH = FastHash(L"EXPLO_SHIELD_FLASH");
const UINT32 hash_EXPLO_CHARGE = FastHash(L"EXPLO_CHARGE");
const UINT32 hash_EXPLO_STUN_INVISIBLE = FastHash(L"EXPLO_STUN_INVISIBLE");
const UINT32 hash_EXPLO_BLOWUP_VEST = FastHash(L"EXPLO_BLOWUP_VEST");
const UINT32 hash_EXPLO_MOLOTOV = FastHash(L"EXPLO_MOLOTOV");
const UINT32 hash_EXPLO_BURN_DOT = FastHash(L"EXPLO_BURN_DOT");
const UINT32 hash_EXPLO_FAKE_SPY_CAMERA = FastHash(L"EXPLO_FAKE_SPY_CAMERA");
const UINT32 hash_EXPLO_FAKE_INTIMIDATE = FastHash(L"EXPLO_FAKE_INTIMIDATE");
const UINT32 hash_EXPLO_FAKE_SPY_CAMERA_HOSATAGE = FastHash(L"EXPLO_FAKE_SPY_CAMERA_HOSTAGES");
const UINT32 hash_EXPLO_FLAME_JET = FastHash(L"EXPLO_FLAME_JET");
const UINT32 hash_EXPLO_GREEN_GOO = FastHash(L"EXPLO_GREEN_GOO");
const UINT32 hash_EXPLO_GREEN_GOO_GROUND = FastHash(L"EXPLO_GREEN_GOO_GROUND");
const UINT32 hash_EXPLO_INVISIBLE_EXPLODING_ZOMBIE = FastHash(L"EXPLO_INVISIBLE_EXPLODING_ZOMBIE");
//camball - adds 1sec DoT "explosions" for the entire life then upon deallocation it adds the final one
const UINT32 hash_EXPLO_FAKE_CAM_BALL = FastHash(L"EXPLO_FAKE_CAM_BALL");
const UINT32 hash_EXPLO_FAKE_CAM_BALL_HOSTAGE = FastHash(L"EXPLO_FAKE_CAM_BALL_HOSTAGES");
const UINT32 hash_EXPLO_FAKE_CAM_BALL_1SEC = FastHash(L"EXPLO_FAKE_CAM_BALL_1SEC");
const UINT32 hash_EXPLO_FAKE_CAM_BALL_HOSTAGE_1SEC = FastHash(L"EXPLO_FAKE_CAM_BALL_HOSTAGES_1SEC");

class CExplosionTemplate
{
public:
	CStringHash	name;
	float fDamage, fDamageRadius;
	float fStunDuration, fStunRadius;
	float fSoundRadius;
	int		nShrapnelCnt;	//cate bucati de shrapnel arunca
	int		nNapalmCnt;		//cate flacari arunca
	float	fMaxImpulse;	//impulsul maxim transferat inamicilor
	int		nArmorPiercingRating;	//AP class - bullet vs shield logic (see actor's ArmorRating)
	float	fDamageObjectsMultiplier;		//explosion breaks doors too? (multiply fDamage by this)
	EActorClass eIgnoreActorClass;	//ignores specified class

	//damage over time
	CDamageOverTime	cDoT;	
	float			fDoTRadius; //raza de actiune DoT

	CExplosionTemplate() : 
		fDamage(0.0f), fDamageRadius(0.0f), fStunRadius(0.0f), fStunDuration(0.0f),	fDamageObjectsMultiplier(0.0f),
		fSoundRadius(64.0f), nShrapnelCnt(0), nNapalmCnt(0), fMaxImpulse(0.0f), fDoTRadius(0.0f), nArmorPiercingRating(1), eIgnoreActorClass(K_LVL_ACT_CLASS_ANY)
	{
		name.Reset();
		cDoT.Set(CDamageOverTime::K_LVL_DoT_NONE);
	}
};


///--------------------------------------------------------------------------
/// MISC OBJECTS - diverse obiecte speciale exportate din editor (RAILS, etc)
///--------------------------------------------------------------------------
enum EMiscObjectType
{
	K_LVL_MISC_UNDEFINED = -1,
	K_LVL_MISC_RAILS = 0,		//params: none
	K_LVL_MISC_FRONTLAYEROBJ,	//params: strAnim - animatia din m_sprBack, nFrame = frame-ul
	K_LVL_MISC_SCRIPT,	//params: str_script = SCRIPT_NAME

	//numarul de tipuri
	K_LVL_MISC_TYPES
};

//clasa generica de baza
class CMiscObjectBase
{
private:
	UINT32	UID;

public:
	int		ID; //id din editor
	EMiscObjectType type; //tipul obiectului

	CVariantCollection	varParams;	//parametrii primiti din editor, specifici fiecarui tip (vezi comentarii EMiscObjectTypes)

	//CTOR/DTOR
	CMiscObjectBase() : ID(-1), type(K_LVL_MISC_UNDEFINED)
	{
		UID = GenerateUID();
		varParams.DeleteAll();
	}
	virtual ~CMiscObjectBase() //virtual - cheama constructorul claselor derivate daca dezaloci prin pointer de baseClass
	{
		varParams.DeleteAll();
	}
};

//clase specifice fiecarui tip
class CMiscObjectRail : public CMiscObjectBase
{
public:
	CFixedArray<Vec2, 32> arrPoints; //maxim 32 de puncte pe un rail
	CFixedArray<float, 32> arrLenghts;		//lungimea parcursa pana in fiecare nod
	float fLength;							//lungimea totala

	//intoarce pozitia pe rail in functie de cursor intre 0 si 1
	Vec2 GetPosNormalized(float fCursorNormalized, Vec2 * retDir = NULL);
	//intoarce pozitia pe rail in fn de distanta parcursa (si directia normalizata)
	Vec2 GetPos(float fDistFromStart, Vec2 * retDir = NULL);
	//CTOR/DTOR
	CMiscObjectRail() : fLength(0.0f)
	{
		type = K_LVL_MISC_RAILS;
	}
	~CMiscObjectRail()
	{
		arrPoints.Clear();
		arrLenghts.Clear();
	}
};

#define K_LVL_FRONTLAYER_PARALLAX 1.5f
#define K_LVL_FRONTLAYER_SCALING 3.0f

class CMiscObject_FrontLayerObj : public CMiscObjectBase
{
public:
	Vec2 pos;
	CSprite sprite;
	CAABB aabb_ini;  //bbox initial

	CMiscObject_FrontLayerObj()
	{
		type = K_LVL_MISC_FRONTLAYEROBJ;
	}
};

///--- LEVEL STATISTICS -----------------------------------------------------------------------------
enum ELevelStats {
	K_LVL_STATS_LEVEL_START_SEC = 0,	//start time in seconds
	K_LVL_STATS_LEVEL_END_SEC = 1,		//end time in seconds
	
	K_LVL_STATS_TARGETS_TOTAL,		//contine numarul total initial de targets (inamici, ostateci, etc)
	K_LVL_STATS_TARGETS_LEFT,		//cate targets au mai ramas de rezolvat
	K_LVL_STATS_HOSTAGES_KILLED,	//total number killed hostages
	K_LVL_STATS_BOMBS_DISARMED,		//total number of disarmed bombs
	K_LVL_STATS_HOSTAGES_TOTAL,		//numarul total de ostateci
	K_LVL_STATS_CIVILIANS_TOTAL,	//total number of level civilians
	K_LVL_STATS_CIVILIANS_KILLED,	//total number of killed civilians
	K_LVL_STATS_CIVILIANS_ARRESTED, //total number of arrested civilians
	//bomb mode
	K_LVL_STATS_LEVEL_HAS_BOMBS,	//daca e diferit de 0 inseamna ca are bomba (provizoriu tin si numarulo bombelor aici)
	K_LVL_STATS_LEVEL_BOMB_ID,		//id-ul bombei
	K_LVL_STATS_LEVEL_BOMB_SEEN,	//daca a fost vazuta bomba
	//arrest warrant
	K_LVL_STATS_LEVEL_ARREST_TARGETS_TOTAL,		//not 0 means we have N enemies to arrest
	K_LVL_STATS_LEVEL_ARREST_TARGETS_ARRESTED,	//how many arrested?
	K_LVL_STATS_LEVEL_ARREST_TARGETS_KILLED,	//how many killed?
	//zombies
	K_LVL_STATS_ZOMBIES_TOTAL,
	K_LVL_STATS_ZOMBIE_PORTALS,
	K_LVL_STATS_ZOMBIE_PORTALS_DESTROYED,
	//misc
	K_LVL_STATS_LEVEL_SNIPER_VICTIMS,				//how many sniped 
	K_LVL_STATS_LEVEL_HEALTH_BOXES_USED,			//how many health boxes were used
	//infinite mode
	K_LVL_STATS_LEVEL_VINFINITE_FLOOR,				//current infinite mode floor

	//stats for player 1
	K_LVL_STATS_PL1START,///!!! don't move from here
	K_LVL_STATS_PL1_HAS_PLAYED,		//daca a jucat sau nu chiar daca acum e null (folosit la final de nivel la afisarea concluziilor)	
	K_LVL_STATS_PL1_KILLS,
	K_LVL_STATS_PL1_BULLETS_SHOT,	//cate gloante a tras
	K_LVL_STATS_PL1_BULLETS_HIT,	//cate gloante au lovit target uman
	K_LVL_STATS_PL1_HOSTAGES_SAVED,
	K_LVL_STATS_PL1_DEATHS,			//de cate ori a murit
	K_LVL_STATS_PL1_STRATEGIC_POINTS, //cate puncte strategice are in bara
	K_LVL_STATS_PL1_LIVES, 
	K_LVL_STATS_PL1_RESURRECT_PEER_CNT,		//de cate ori a facut resurrect colegului
	K_LVL_STATS_PL1_USE_EXTRA_LIFE_CNT,		//de cate ori a folosit extra life in nivel
	K_LVL_STATS_PL1_DAMAGE_TAKEN,			//number of hits taken

    //stats for player 2
	K_LVL_STATS_PL2START,///!!! don't move from here
	K_LVL_STATS_PL2_HAS_PLAYED,
	K_LVL_STATS_PL2_KILLS,
	K_LVL_STATS_PL2_BULLETS_SHOT,
	K_LVL_STATS_PL2_BULLETS_HIT,
	K_LVL_STATS_PL2_HOSTAGES_SAVED,
	K_LVL_STATS_PL2_DEATHS,		
	K_LVL_STATS_PL2_STRATEGIC_POINTS, 
	K_LVL_STATS_PL2_LIVES,
	K_LVL_STATS_PL2_RESURRECT_PEER_CNT,
	K_LVL_STATS_PL2_USE_EXTRA_LIFE_CNT,
	K_LVL_STATS_PL2_DAMAGE_TAKEN,		

	K_LVL_STATS_CNT
};

//numarul de stats per player ca sa le pot accesa in fn de ordinal
#define K_LVL_STATS_PLAYER_STATS_COUNT (K_LVL_STATS_PL2START - K_LVL_STATS_PL1START)

//level flags that get set when loading/starting a level
#define K_LVL_LEVEL_FLAG_NONE				0
#define K_LVL_LEVEL_FLAG_DOWNLOADED			1
#define K_LVL_LEVEL_FLAG_MODS_ON			2


///----------------------------------------------------------------------------------
/// SCRIPT ACTIONS - used on objects, gathered from object, weapons, inventory objects, actor properties, etc
/// !! must be copyable - implement copy constructor if needed
///----------------------------------------------------------------------------------
class CScriptAction {
public:
	CStringHash			shID;					// internal name of the action
	std::wstring		strTargetClasses;		// comma separated values with TARGETED IActive classes
	CStringHash			shScriptName;			// name of script to launch on targeted object
	int					strID_name;				// string ID of name to show when interacting (knock or break or etc)

	CScriptAction() :
		strID_name(-1)
	{
	}
};