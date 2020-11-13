#pragma once

//anunt clasele necesare
class CCollisionShape;

///--------------------------------------------------------------------------
/// EFFECT TYPES
///--------------------------------------------------------------------------
enum ELVLEffectType {
	K_LVL_EFFECT_EMPTY = -1,

	K_LVL_EFFECT_ELECTRIC_BREAK_SPARKS = 0,
	K_LVL_EFFECT_STARS_CONFETTI = 1,
	K_LVL_EFFECT_EXPLO_LARGE,
	K_LVL_EFFECT_EXPLODING_ZOMBIE_DIE,
	K_LVL_EFFECT_STONE_BREAK,

	K_LVL_EFFECTS_CNT,
};

const CStringHash ELVLEffectTypeNames[] = {
	L"EFFECT_ELECTRIC_BREAK_SPARKS",
	L"EFFECT_STARS_CONFETTI",
	L"EFFECT_EXPLO_LARGE",
	L"EFFECT_EXPLODING_ZOMBIE_DIE",
	L"EFFECT_STONE_BREAK", 
};

///----------------------------------------------------------------------------------
/// RENDER PASSES that the level needs/does
///----------------------------------------------------------------------------------
enum eLVLRenderPass {
	K_LVL_RP_NONE = -1,
	K_LVL_RP_COLORS = 0,
	K_LVL_RP_NORMALS_HEIGHT = 1,

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
/// PHYSICS POINTS
///--------------------------------------------------------------------------

#define K_LVL_PHYSP_MAX_CNT 256
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
//TODO: ca sa testezi coliziunea unui cerc trebuiesc marite toate dreptunghiurile cu o raza...
//float		fRadius;	//raza de coliziune (de exemplu grenade si alte obiecte rotunde)

	float		fAngle;
	float		fAngularSpeed;
	float		fAngularAccel;

	D3DXVECTOR2 pos, pos_last;
	D3DXVECTOR2 speed;
	D3DXVECTOR2 accel;
	//date frecare si bounce
	bool  bContacting; //spune daca este in contact
	bool  bContactStarted; //spune cand s-a initiat contactul, doar pentru un frame. Se poate pune sunet in fn de el
	bool  bIsStatic;	//spune daca nu se mai misca
	bool  bIsDead;		//spune daca e mort (afara din zona de joc)
	D3DXVECTOR2 contactNormal;  //normala ultimului contact
	D3DXVECTOR2 contactPos;		//pozitia ultimului contact
	CCollisionShape* pContactShape; //collision shape-ul cu care face contact

	float fBounceF;   //bounce restitution factor
	float fFrictionF; //default 10.0f

	CPhysicsPoint2D() : eCollType(K_COLLTYPE_NONE), bContacting(false), bContactStarted(false) , bIsStatic(false), nFlagsCollision(0),
		bFlagRotationEnabled(false), fAngle(0.0f), fAngularSpeed(0.0f), fAngularAccel(0.0f),
		bFlagPhysicsEnabled(false), fBounceF(0.5f), fFrictionF(10.0f), bIsDead(false), pContactShape(NULL)
	{
		pos = pos_last = D3DXVECTOR2(0.0f, 0.0f);
		speed = D3DXVECTOR2(0.0f, 0.0f);
		accel = D3DXVECTOR2(0.0f, 0.0f);
		contactNormal = D3DXVECTOR2(0.0f, 0.0f);
		contactPos = D3DXVECTOR2(0.0f, 0.0f);
	};

	void Init();
	/*
	 * Forces a new position for the point
	 */
	void SetPosForced(D3DXVECTOR2 vecPos);
};



///--------------------------------------------------------------------------
/// BEHAVIORS and AI STATES
///--------------------------------------------------------------------------
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

//!!! CLASA CAIBehavior TREBUIE SA POATA FI COPIATA CA VALOARE (vezi initActor unde copiaza template actor in instanta actor)
class CAIBehavior
{
public:
	EAIBehaviorType	nType;
	bool			bCanInterrupt; //if false it keeps current behavior until finished. Doesn't check sensor info. Ai grija daca moare in timpul asta.
	bool			bDetectPlatforms; //daca este setat tine cont de capetele platformelor ca sa se intoarca
	bool			bIgnoreEvents;		//sa ignore eventurile AI (mai putin EVENT_DEAD)
	float			fBehaviorDuration;	//cat timp dureaza starea, dupa care ii da "finished"; <0.0f means infinite

	CVariantCollection	m_vcolParams;

	CAIBehavior() : nType(AI_BEHAVIOR_EMPTY), bCanInterrupt(true), bDetectPlatforms(true), bIgnoreEvents(false), fBehaviorDuration(-1.0f)
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

	D3DXVECTOR2		pos;		//pozitia eventului
	
	//CTOR/DTOR
	CAIEvent() :
		nType(K_LVL_AI_EVENT_NONE),
		ownerUID(0), ownerClass(-1), targetUID(0),
		fRadius(0.0f), fDuration(0.0f)
	{}

	CAIEvent(EAIEventType eventType, UINT32 evtOwnerUID, int evtOwnerClass, D3DXVECTOR2 vPos, float radius, float duration = 0.6f, UINT32 evtTargetUID = 0) :
		nType(eventType), ownerUID(evtOwnerUID), ownerClass(evtOwnerClass), pos(vPos), fRadius(radius), fDuration(duration), targetUID(evtTargetUID)
	{}

	void Set(EAIEventType eventType, UINT32 evtOwnerUID, int evtOwnerClass, D3DXVECTOR2 vPos, float radius, float duration = 0.6f, UINT32 evtTargetUID = 0)
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
		pos = D3DXVECTOR2(0.0f, 0.0f);
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

//Template-ul de AI este cel incarcat din xml-ul de AI
class CAITemplate
{
public:
	CGrowableArray<CAIState*> m_arrStates;  //starile din care selecteaza 
	CGrowableArray<EAIEventType> m_arrIgnoredEvents;	//list of ignored events
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

enum EActorWeaponsIdx {
	K_LVL_ACT_WEAPON_PRIMARY = 0,
	K_LVL_ACT_WEAPON_SECONDARY = 1,
	K_LVL_ACT_WEAPON_GEAR,
	K_LVL_ACT_WEAPON_MELEE,

	K_LVL_ACT_WEAPON_TEMPORARY,  //used for special abilities and temporary weapons (can't reload)
	K_LVL_ACT_WEAPON_TEMPORARY_ALT,  //alt fire for temp

	K_LVL_ACT_WEAPON_NO_WEAPON,  //used for empty weapons so we don't use NULL
	///--- hardcoded ---
	K_LVL_ACT_WEAPON_BREACH,	//default breach weapon used when breaching doors

	K_LVL_ACT_WEAPONS_CNT
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


///--------------------------------------------------------------------------
/// ACTOR TEMPLATES de inamici si arme din XML
///--------------------------------------------------------------------------
//enum-ul actorAnims specifica indecsii in array-ul de definitii de animatii pt acces rapid
enum EActorAnims
{
	K_LVL_ACT_ANIM_NOT_SET = -2,
	K_LVL_ACT_ANIM_EMPTY = -1,

	K_LVL_ACT_ANIM_REF_POSE = 0, //de aici isi ia collision points si bbox (frame 0-idle, 1-crouched, 2-dead)
	K_LVL_ACT_ANIM_IDLE,	   
	//move
	K_LVL_ACT_ANIM_MOVE,		//walk. importanta
	K_LVL_ACT_ANIM_MOVE_FAST,	//run - secundara
	K_LVL_ACT_ANIM_MOVE_BK,		//se misca cu spatele
	//saritura (3 anims)
	K_LVL_ACT_ANIM_JUMP,
	K_LVL_ACT_ANIM_JUMP_STILL,
	K_LVL_ACT_ANIM_FALL,
	//shoot
	K_LVL_ACT_ANIM_SHOOT,		//animatie shoot primary
	K_LVL_ACT_ANIM_SHOOT_ALT,	//shoot alternativ (grenada)
	K_LVL_ACT_ANIM_USE_GEAR,	//use gear (grenades, mines, etc)
	K_LVL_ACT_ANIM_MELEE,		//melee attack
	//die
	K_LVL_ACT_ANIM_DIE,			//moarte normala
	K_LVL_ACT_ANIM_DIE_ALT,		//moarte alternativa, if needed

	//speciale - pentru cei care au animatii separate cu arma scoasa
	K_LVL_ACT_ANIM_DRAW_GUN,
	K_LVL_ACT_ANIM_HOLSTER_GUN,
	K_LVL_ACT_ANIM_IDLE_GUN,			//IDLE with gun out
	K_LVL_ACT_ANIM_MOVE_GUN,			//move with gun pulled out
	K_LVL_ACT_ANIM_MOVE_FAST_GUN,		//move fast with gun pulled out
	K_LVL_ACT_ANIM_MOVE_BK_GUN,			//move back with gun pulled out
	
	K_LVL_ACT_GUNPOINT_DRAW_GUN,		//folosite de exemplu cand scoate pistolul si executa ostatec
	K_LVL_ACT_GUNPOINT_IDLE_GUN,
	K_LVL_ACT_GUNPOINT_SHOOT,

	K_LVL_ACT_ANIM_CLIMB_LADDER,		//animatie de urcare pe ladder
	K_LVL_ACT_ANIM_RELOAD,				//reload weapon
	K_LVL_ACT_ANIM_MOVE_SHOOTING,		//shoot while moving
	K_LVL_ACT_ANIM_MOVE_BK_SHOOTING,	//shoot while moving backwards

	K_LVL_ACT_ANIM_CROUCH,
	K_LVL_ACT_ANIM_CROUCH_SHOOT,
	K_LVL_ACT_ANIM_CROUCH_RELOAD,
	K_LVL_ACT_ANIM_GET_UP,				//folosita daca am animatie sa se ridice din crouch de exemplu
	K_LVL_ACT_ANIM_STUNNED,
	K_LVL_ACT_ANIM_BREACH_DOOR,			//cand sparge o usa cu ranga
	K_LVL_ACT_ANIM_INTERACT,			//cand interactioneaza cu obiecte din spate (fiddle)
	K_LVL_ACT_ANIM_LOCKPICK,			//lockpicking door
	K_LVL_ACT_ANIM_ROLL,				//roll animation

	K_LVL_ACT_ANIM_WALK_IN,				//cand intra pe o usa
	K_LVL_ACT_ANIM_WALK_OUT,			//cand iese dintr-o usa
	K_LVL_ACT_ANIM_TAUNT,	
	K_LVL_ACT_ANIM_TEASE,
	K_LVL_ACT_ANIM_ARRESTED,			//arrested animation
	K_LVL_ACT_ANIM_SURRENDER,			//surrender animation (I give up!)
	K_LVL_ACT_ANIM_TELEPORT_IN,			//beam me up Scotty!
	K_LVL_ACT_ANIM_TELEPORT_OUT,		//beam me out Scotty!
	K_LVL_ACT_ANIM_FLY,
	//--- feet anims for composed animations ---
	K_LVL_ACT_ANIM_FEET_STILL,
	K_LVL_ACT_ANIM_FEET_IDLE,
	K_LVL_ACT_ANIM_FEET_MOVE,
	K_LVL_ACT_ANIM_FEET_MOVE_FAST,
	K_LVL_ACT_ANIM_FEET_CROUCH,
	K_LVL_ACT_ANIM_FEET_JUMP,
	K_LVL_ACT_ANIM_FEET_MOVE_BACK,

	//ultima linie specifica nr maxim de animatii
	K_LVL_ACT_ANIMS_CNT
};

//numarul maxim de seturi de animatii
#define K_LVL_ACT_ANIM_MAX_SETS 2

//numele indecsilor de animatii pentru cautare in XML. Trebuie sa corespunda cu _tag_actorAnims	din levelTypes.h
const CStringHash EActorAnimNames[K_LVL_ACT_ANIMS_CNT] =
{
	L"REF_POSE",
	L"IDLE",
	L"MOVE",
	L"MOVE_FAST",
	L"MOVE_BK",
	L"JUMP",
	L"JUMP_STILL",
	L"FALL",

	L"SHOOT",
	L"SHOOT_ALT",
	L"USE_GEAR",
	L"MELEE",

	L"DIE",
	L"DIE_ALT",

	L"DRAW_GUN",
	L"HOLSTER_GUN",
	L"IDLE_GUN",
	L"MOVE_GUN",
	L"MOVE_FAST_GUN",
	L"MOVE_BK_GUN",

	L"GUNPOINT_DRAW_GUN",
	L"GUNPOINT_IDLE_GUN",
	L"GUNPOINT_SHOOT",

	L"CLIMB_LADDER",
	L"RELOAD",
	L"MOVE_SHOOTING",
	L"MOVE_BK_SHOOTING",
	L"CROUCH",
	L"CROUCH_SHOOT",
	L"CROUCH_RELOAD",
	L"GET_UP",
	L"STUNNED",
	L"BREACH_DOOR",
	L"INTERACT",
	L"LOCKPICK",
	L"ROLL",

	L"WALK_IN",
	L"WALK_OUT",
	L"TAUNT",
	L"TEASE",
	L"ARRESTED",
	L"SURRENDER",
	L"TELEPORT_IN",
	L"TELEPORT_OUT",
	L"FLY",
	//--- feet anims for composed animations ---
	L"FEET_STILL",
	L"FEET_IDLE",
	L"FEET_MOVE",
	L"FEET_MOVE_FAST",
	L"FEET_CROUCH",
	L"FEET_JUMP",
	L"FEET_MOVE_BACK"
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

class CActorTemplate
{
public:
	enum EActorCapabilitiesFlags {
		K_ACT_CAPS_NONE = 0,
		K_ACT_CAPS_CAN_JUMP = 1,
		K_ACT_CAPS_CAN_CROUCH = 2,
		K_ACT_CAPS_CAN_COVER = 4,
		K_ACT_CAPS_CAN_CLIMB = 8,
		K_ACT_CAPS_CAN_ROLL = 16,
		K_ACT_CAPS_CAN_INTERACT = 32,
		//flaguri diverse
		K_ACT_CAPS_NOT_A_TARGET = 128,		//daca e setat inseamna ca nu poate fi impuscat sau explodat
		//has explosive vest?
		K_ACT_CAPS_HAS_EXPLOSIVE_VEST = 256,
		//can be remote detonated?
		K_ACT_CAPS_CAN_BE_DETONATED = 512,
		//can the view be rotated? (High precision FOV if on, rectangle vision if off)
		K_ACT_CAPS_CAN_ROTATE_VIEW = 1024,
		//was biten by zombies so it turns upon death
		K_ACT_CAPS_TURN_TO_ZOMBIE = 2048
	};


	CStringHash		shName;				//template name

	int				animIDs[K_LVL_ACT_ANIMS_CNT][K_LVL_ACT_ANIM_MAX_SETS];	//array cu numele animatiilor pentru fiecare dintre actiunile din _tag_actorAnims (default dir: Right) cu maxim o variatie
	int				soundIDs[K_LVL_ACT_VERSES_COUNT][K_LVL_ACT_VERSES_MAX_SETS]; //contains sound ids-s mapped on different actions (called verses, see EActorSoundVerse)
	///--- GENERICS: !!! when adding new generics don't forget to edit OverwriteGenericDataFromTemplate !!!
	EActorClass		actorClass; //clasa din care face parte (tip _tag_actorClass)
	EActorClass		foeClassFilter1, foeClassFilter2;	//if other than K_LVL_ACT_CLASS_ANY ignores all other classes but this when searching for enemies
	EMaterialType	eMaterial;	//type of material
	int				nHUDportraitFrameIdx; //index of portrait frame in IGM_INTERFACE PORTRAITS animation
	
	UINT32			eCaps;				//see EActorCapabilitiesFlags

	float			fLife;				//actor life
	float			fArmor;				//actor armor
	int				nArmorDir;			//0-all around, -1 back shield, 1 front shield
	int				nArmorRating;		//clasa armurii - in legatura cu Armor Piercing Rating. AR=1, bullet AP=2 inseamna ca trece glontul prin armura
	float			fArmorMPP;			//Melee Protection Percentage of melee protection (0..1)

	float			jumpSpeed;
	float			moveMaxSpeed;		//run speed
	float			moveMinSpeed;		//walk speed
	float			moveBackSpeed;		//walk backwards
	float			fDexterity;			//(default 1.0f) - weapon reload
	float			fRecoilModifier;	//(default 1.0f) - gets multiplied with recoil aiming error (lower means better aim)
	float			climbSpeed;			//viteza cand se catara
	float			distSee;			//distanta la care vede
	float			distHear;			//distanta la care te aude (fiind cu spatele)
	float			distAttackMax;		//distanta de la care ataca
	float			distAttackMin;		//distanta minima de tinut pana in inamic
	float			fMass;				//masa actorului (scalre impuls gloante)
	float			fFOVpercent;		//FOV-ul actorului in starea idle

	CStringHash		shScript_OnSpawn;				//if set it gets executed on character spawning (used mainly to add perks to weapons)

	///--- CAN'T BE CHANGED:
	bool			bComposedAnimation; //daca animatia este compusa din torso si feet
	float			fStrategicPoints;	//cate puncte strategice iei cand ii ucizi/salvezi

	///--- WEAPONS:
	CStringHash		weaponType;  //tipul armei WPN_etc
	CStringHash		weaponTypeAlt; //tipul armei secundare
	CStringHash		weaponTypeGear; //tipul armei GEAR
	CStringHash		weaponTypeMelee; //tipul armei secundare pe care poti sa schimbi
	CStringHash		weaponTypeBreach; //tipul armei folosite la door breaching

	///--- AI: pointer catre template-ul de AI, salvat separat in growableArray ca sa pot copia structura asta in actori
	CAITemplate*	AItemplate;
	CStringHash		AIdefaultStateName;  //numele state-ului initial din AI

	CActorTemplate() : 
		//generic params
		jumpSpeed(K_NOT_SET), moveMaxSpeed(K_NOT_SET), moveMinSpeed(K_NOT_SET), moveBackSpeed(K_NOT_SET), fDexterity(K_NOT_SET), fRecoilModifier(K_NOT_SET),
		nHUDportraitFrameIdx(K_NOT_SET), distSee(K_NOT_SET), distHear(K_NOT_SET), distAttackMax(K_NOT_SET), 
		distAttackMin(K_NOT_SET), fLife(K_NOT_SET), fArmor(K_NOT_SET), nArmorDir(K_NOT_SET), fArmorMPP(K_NOT_SET),
		nArmorRating(K_NOT_SET), fMass(K_NOT_SET), climbSpeed(K_NOT_SET), fFOVpercent(K_NOT_SET), 
		//more important values
		actorClass(K_LVL_ACT_CLASS_NOT_SET), foeClassFilter1(K_LVL_ACT_CLASS_ANY), foeClassFilter2(K_LVL_ACT_CLASS_ANY),
		bComposedAnimation(false), eMaterial(K_LVL_MATERIAL_UNKNOWN), eCaps(K_ACT_CAPS_NONE), AItemplate(null),
		fStrategicPoints(0.0f)
	{
		shName.Reset();
		AIdefaultStateName.Reset(); //TODO: ar trebui initializat pe un AI static care nu face nimic

		weaponType.Reset();
		weaponTypeAlt.Reset();
		weaponTypeGear.Reset();
		weaponTypeMelee.Reset();
		weaponTypeBreach.Reset();

		//reset anim IDs
		for (int kk = 0; kk < K_LVL_ACT_ANIMS_CNT; kk++)
		{
			for (int jj = 0; jj < K_LVL_ACT_ANIM_MAX_SETS; jj++)
			{
				animIDs[kk][jj] = -1; //default value for missing anim
			}
		}
		//reset verses
		for (int kk = 0; kk < K_LVL_ACT_VERSES_COUNT; kk++)
		{
			for (int jj = 0; jj < K_LVL_ACT_VERSES_MAX_SETS; jj++)
			{
				soundIDs[kk][jj] = -1; //default value for missing anim
			}
		}
	}

	// Fills variables with default values if not set (some templates are missing values)
	// \brief: We need this because of the upgrade templates that must have a lot of values on K_NOT_SET (0xDEADBEEF)
	void FillDefaultValuesIfNotSet()
	{
		if (actorClass == K_LVL_ACT_CLASS_NOT_SET) { actorClass = K_LVL_ACT_CLASS_HUMAN; }
		if (eMaterial == K_LVL_MATERIAL_UNKNOWN) { eMaterial = K_LVL_MATERIAL_FLESH; }

		if (nHUDportraitFrameIdx == K_NOT_SET) { nHUDportraitFrameIdx = 0; }
		if (fLife == K_NOT_SET) { fLife = 100.0f; }
		if (fArmor == K_NOT_SET) { fArmor = 0.0f; }
		if (nArmorDir == K_NOT_SET) { nArmorDir = 1; }
		if (nArmorRating == K_NOT_SET) { nArmorRating = 0; }
		if (jumpSpeed == K_NOT_SET) { jumpSpeed = 0.0f; }
		if (moveMaxSpeed == K_NOT_SET) { moveMaxSpeed = 0.0f; }
		if (moveMinSpeed == K_NOT_SET) { moveMinSpeed = 0.0f; }
		if (moveBackSpeed == K_NOT_SET) { moveBackSpeed = 0.0f; }
		if (fDexterity == K_NOT_SET) { fDexterity = 1.0f; }
		if (fRecoilModifier == K_NOT_SET) { fRecoilModifier = 1.0f; }
		if (climbSpeed == K_NOT_SET) { climbSpeed = 0.0f; }
		if (distSee == K_NOT_SET) { distSee = 64.0f; }
		if (distHear == K_NOT_SET) { distHear = 16.0f; }
		if (distAttackMax == K_NOT_SET) { distAttackMax = 64.0f; }
		if (distAttackMin == K_NOT_SET) { distAttackMin = 0.0f; }
		if (fMass == K_NOT_SET) { fMass = 10.0f; }
		if (fFOVpercent == K_NOT_SET) { fFOVpercent = 0.7f; }
		if (fArmorMPP == K_NOT_SET) { fArmorMPP = 0.0f; }
	}

	// Overwrites the current animations with hte ones that are set in pTemplate
	void OverwriteAnimsFromTemplate(CActorTemplate* pTemplate, bool bEraseOldAnimations = false)
	{
		if (pTemplate == null)
		{
			return;
		}

		for (int kk = 0; kk < K_LVL_ACT_ANIMS_CNT; kk++)
		{
			for (int jj = 0; jj < K_LVL_ACT_ANIM_MAX_SETS; jj++)
			{
				if (bEraseOldAnimations)
					animIDs[kk][jj] = -1;
				//overwrite if existing
				if (pTemplate->animIDs[kk][jj] != -1)
					animIDs[kk][jj] = pTemplate->animIDs[kk][jj];
			}
		}
	}

	//Overwrites "generics" with the ones that are set in pTemplate (only if not K_NOT_SET)
	void OverwriteGenericDataFromTemplate(CActorTemplate* pTemplate)
	{
		if (pTemplate == null)
		{
			return;
		}
		if (pTemplate->actorClass != K_LVL_ACT_CLASS_NOT_SET) { actorClass = pTemplate->actorClass; }
		if (pTemplate->foeClassFilter1 != K_LVL_ACT_CLASS_ANY) { foeClassFilter1 = pTemplate->foeClassFilter1; }
		if (pTemplate->foeClassFilter2 != K_LVL_ACT_CLASS_ANY) { foeClassFilter2 = pTemplate->foeClassFilter2; }
		if (pTemplate->eMaterial != K_LVL_MATERIAL_UNKNOWN) { eMaterial = pTemplate->eMaterial; }
		//add new caps (should XOR if need to disable them)
		if(pTemplate->eCaps != 0) eCaps = pTemplate->eCaps;

		if (pTemplate->nHUDportraitFrameIdx != K_NOT_SET) { nHUDportraitFrameIdx = pTemplate->nHUDportraitFrameIdx; }
		if (pTemplate->fLife != K_NOT_SET) { fLife = pTemplate->fLife; }
		if (pTemplate->fArmor != K_NOT_SET) { fArmor = pTemplate->fArmor; }
		if (pTemplate->nArmorDir != K_NOT_SET) { nArmorDir = pTemplate->nArmorDir; }
		if (pTemplate->nArmorRating != K_NOT_SET) { nArmorRating = pTemplate->nArmorRating; }
		if (pTemplate->jumpSpeed != K_NOT_SET) { jumpSpeed = pTemplate->jumpSpeed; }
		if (pTemplate->moveMaxSpeed != K_NOT_SET) { moveMaxSpeed = pTemplate->moveMaxSpeed; }
		if (pTemplate->moveMinSpeed != K_NOT_SET) { moveMinSpeed = pTemplate->moveMinSpeed; }
		if (pTemplate->moveBackSpeed != K_NOT_SET) { moveBackSpeed = pTemplate->moveBackSpeed; }
		if (pTemplate->fDexterity != K_NOT_SET) { fDexterity = pTemplate->fDexterity; }
		if (pTemplate->fRecoilModifier != K_NOT_SET) { fRecoilModifier = pTemplate->fRecoilModifier; }
		if (pTemplate->climbSpeed != K_NOT_SET) { climbSpeed = pTemplate->climbSpeed; }
		if (pTemplate->distSee != K_NOT_SET) { distSee = pTemplate->distSee; }
		if (pTemplate->distHear != K_NOT_SET) { distHear = pTemplate->distHear; }
		if (pTemplate->distAttackMax != K_NOT_SET) { distAttackMax = pTemplate->distAttackMax; }
		if (pTemplate->distAttackMin != K_NOT_SET) { distAttackMin = pTemplate->distAttackMin; }
		if (pTemplate->fMass != K_NOT_SET) { fMass = pTemplate->fMass; }
		if (pTemplate->fFOVpercent != K_NOT_SET) { fFOVpercent = pTemplate->fFOVpercent; }
		if (pTemplate->fArmorMPP != K_NOT_SET) { fArmorMPP = pTemplate->fArmorMPP; }
		//scripts
		if (!pTemplate->shScript_OnSpawn.IsEmpty()) { shScript_OnSpawn = pTemplate->shScript_OnSpawn; }
	}

	//Adds "GENERIC" data from pTemplate to current template (weapon upgrades and such)
	void AddGenericDataFromTemplate(CActorTemplate* pTemplate)
	{
		if (pTemplate == null)
		{
			return;
		}
		if (pTemplate->actorClass != K_LVL_ACT_CLASS_NOT_SET) { actorClass = pTemplate->actorClass; }
		if (pTemplate->foeClassFilter1 != K_LVL_ACT_CLASS_ANY) { foeClassFilter1 = pTemplate->foeClassFilter1; }
		if (pTemplate->foeClassFilter2 != K_LVL_ACT_CLASS_ANY) { foeClassFilter2 = pTemplate->foeClassFilter2; }
		if (pTemplate->eMaterial != K_LVL_MATERIAL_UNKNOWN) { eMaterial = pTemplate->eMaterial; }
		//XOR in new caps:
		eCaps ^= pTemplate->eCaps;

		if (pTemplate->nHUDportraitFrameIdx != K_NOT_SET) { nHUDportraitFrameIdx = pTemplate->nHUDportraitFrameIdx; }
		if (pTemplate->fLife != K_NOT_SET) { fLife += pTemplate->fLife; }
		if (pTemplate->fArmor != K_NOT_SET) { fArmor += pTemplate->fArmor; }
		if (pTemplate->nArmorDir != K_NOT_SET) { nArmorDir = pTemplate->nArmorDir; }  //exception: armor dir can't be added so it gets written over
		if (pTemplate->nArmorRating != K_NOT_SET) { nArmorRating += pTemplate->nArmorRating; }
		if (pTemplate->jumpSpeed != K_NOT_SET) { jumpSpeed += pTemplate->jumpSpeed; }
		if (pTemplate->moveMaxSpeed != K_NOT_SET) { moveMaxSpeed += pTemplate->moveMaxSpeed; }
		if (pTemplate->moveMinSpeed != K_NOT_SET) { moveMinSpeed += pTemplate->moveMinSpeed; }
		if (pTemplate->moveBackSpeed != K_NOT_SET) { moveBackSpeed += pTemplate->moveBackSpeed; }
		if (pTemplate->fDexterity != K_NOT_SET) { fDexterity += pTemplate->fDexterity; }
		if (pTemplate->fRecoilModifier != K_NOT_SET) { fRecoilModifier += pTemplate->fRecoilModifier; }
		if (pTemplate->climbSpeed != K_NOT_SET) { climbSpeed += pTemplate->climbSpeed; }
		if (pTemplate->distSee != K_NOT_SET) { distSee += pTemplate->distSee; }
		if (pTemplate->distHear != K_NOT_SET) { distHear += pTemplate->distHear; }
		if (pTemplate->distAttackMax != K_NOT_SET) { distAttackMax += pTemplate->distAttackMax; }
		if (pTemplate->distAttackMin != K_NOT_SET) { distAttackMin += pTemplate->distAttackMin; }
		if (pTemplate->fMass != K_NOT_SET) { fMass += pTemplate->fMass; }
		if (pTemplate->fFOVpercent != K_NOT_SET) { fFOVpercent += pTemplate->fFOVpercent; }
		if (pTemplate->fArmorMPP != K_NOT_SET) { fArmorMPP += pTemplate->fArmorMPP; }
		//weapon scripts
		if (!pTemplate->shScript_OnSpawn.IsEmpty()) { shScript_OnSpawn = pTemplate->shScript_OnSpawn; }

		//normalize some values
		if (fLife < 0.0f) fLife = 0.0f;
		if (fArmor < 0.0f) fArmor = 0.0f;
		CLAMP(fArmorMPP, 0.0f, 1.0f);
		if (nArmorRating < 0) nArmorRating = 0;
		if (fDexterity < 0.0f) fDexterity = 0.0f;
		if (fRecoilModifier < 0.0f) fRecoilModifier = 0.0f;
		if (fMass < 0.1f) fMass = 0.1f;
		if (moveMinSpeed < 0.0f) moveMinSpeed = 0.0f;
		if (moveMaxSpeed < 0.0f) moveMaxSpeed = 0.0f;
	}

};




///--------------------------------------------------------------------------
///--- BULLETS ---
///--------------------------------------------------------------------------
#define K_LVL_BULLETS_MAX_CNT 256

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

/*!
 *	Structure used to return data from HitActor function
 */
struct CBulletHitReturnData {
	float fPointsTaken;
	EMaterialType eMaterial;
	bool	bPenetratedShield;
	bool	bArmorHit;			//kills shotgun penetrating bullets
	bool	bKilledTarget;
};

//clasa de bullet template este folosita in Weapon Template cand incarca datele fiecarei arme iar apoi in ShootBullet ca sa stie ce trage
class CBulletTemplate {
public:
	EBulletType		nType;					//tipul glontului din cele predefinite (-1 = not set)
	EBulletGroup	nGroup;					//bullet group (classic, grenade, etc hardcoded)

	float	fDamage;				//cat damage poarta glontul
	float	fDamageObjects;			//bullet damage for objects (mixed with BULLET_FLAG_CAN_BREAK_DOORS)
	float	fDamageLossPPx;			//cat damage pierde in functie de distanta parcursa in pixeli (0.0f - nu pierde din damage)
	float	fSelfDamageMultiplier;	//cat damage din damage-ul dat victimei se scade din damage-ul glontului (default 1.0f adica tot)
	float	fCriticalHitChance;		//how probable it is to do a critical hit
	float	fLife;					//cat timp traieste, life-ul initial
	float	fMomentum;				//cat procent din viteza imprima in viteza inamicului
	float	fStunDuration;			//face stun?
	int		nArmorPiercingRating;	//AP class - bullet vs shield logic (see actor's ArmorRating)
	UINT32  nExploTemplateHash;	//template of explosion
	UINT32	nFlags;					//flaguri diverse K_LVL_BULLET_FLAG_*

	float	fSpeed_ini;				//viteza liniara initala glont
	EActorClass eClass;				//shooter class


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
	int		actorClass;				//clasa din care face parte (cine trage ca sa nu se loveasca intre ei inamici de aceeasi clasa)
	UINT32	ownerUID;				//uneori am nevoie de UID-ul ownerului
	UINT32	dwLastTargetUID;		//used in order to hit targets only once (penetrating) no matter the framerate

public:
	int		type;					//tip glont: racheta, grenada, glont de shotgun, sniper, etc
	float	fDamage;				//cat damage poarta glontul
	float	fDamage_ini;			//initial damage (readonly please)
	float	fDamageLossPPx;			//cat damage pierde in functie de distanta parcursa in pixeli (0.0f - nu pierde din damage)
	float	fSelfDamageMultiplier;	//cat damage din damage-ul dat victimei se scade din damage-ul glontului (default 1.0f adica tot)
	float   fCriticalHitChance;		//critical hit chance

	float	fLife;					//cat timp traieste
	float	fLife_ini;				//initial fLife value (readonly please)

	float	fMomentum;				//cat procent din viteza imprima in viteza inamicului
	float	fStunDuration;			//face stun?
	int		nArmorPiercingRating;	//AP class - bullet vs shield logic (see actor's ArmorRating)
	UINT32  nExploTemplateHash;		//hash of explosion template at the end or 0 if none
	D3DXVECTOR2	vSpawnPos;			//pozitie spawnare
	
	SIZEWH_F 		szTailSize;		//bullets that have tails save the tail length here
	RECTLTRB_F		rectTailTex;	//texture rectangle

	UINT32	nFlags;					//flaguri diverse
	int		nSubstate;				//bullet state used by some bullet types

	CLinkedPool<CPhysicsPoint2D>::CLinkedPoolNode *physPt; //punctul fizic (coliziune, pozitie, etc)
    //grafica
	CSprite		sprBullet;	//grafica glont

	CBullet() : type(K_LVL_BULLET_SHOTGUN), fDamage(1.0f), fDamage_ini(1.0f), fDamageLossPPx(0.0f),
		fLife(1.0f), fLife_ini(1.0f), actorClass(K_LVL_ACT_CLASS_PLAYER), nSubstate(0),
		fMomentum(0.0f), nFlags(0), fStunDuration(0.0f), ownerUID(0), dwLastTargetUID(0),
		nArmorPiercingRating(0), nExploTemplateHash(0), fSelfDamageMultiplier(1.0f), fCriticalHitChance(0.0f)
	{
		physPt = null;
		vSpawnPos = D3DXVECTOR2(0.0f, 0.0f);

		szTailSize.w = szTailSize.h = 0.0f;
	}
};



///--------------------------------------------------------------------------
///--- weapon class ---
///--------------------------------------------------------------------------

class CWeaponTemplate
{
public:
	CStringHash		name;
	//generic data:
	int				nHUD_AnimIdx;		//animatie INGAME HUD pentru grafica armei (vezi detalii frames in CCustomInterfaceIGM) sau -1 pt empty
	int				nHUD_AnimIdxALT;	//animatie INGAME HUD when shown as ALT weapon (painted just as small icon)
	int				nMuzzleFlashAnim;	//animatie muzzle flash sau -1 pt empty
	CStringHash		shTemplateOverwrite; //name of themplate that the weapon overwrites over the character template
	float			fSpeedPenaltyPercent;	//what percent of total movement speed is taken by this weapon
	//fire modes data:
	CBulletTemplate	bulletTemplate;		//datele glontului tras de arma curenta
	int				nBulletsPerShot;	//nr de gloante trase pt un ammo
	
	//aiming data - toate FOV-urile de mai jos sunt half FOV de fapt
	float			fSpreadFOV;							//spread default arma
	float			fAimErrorMaxFOV;					//aiming error - FOV in radiani 
	float			fAimErrorAddPerShot;				//ce eroare de AIM se adauga dupa fiecare foc
	float			fAimErrorMulPerShot;				//factor multiplicare eroare FOV ca sa nu mai creasca liniar
	float			fAimErrorCooldownPerSecond;			//cooldown AIM error in functie de timp
	float			fAimFOV;							//angle in rad. can the weapon be aimed? (default 0.0f means only aim vec -1,0 or 1,0)

	int				nClipSize;				//-1 pt nr infinit de gloante
	//reload data
	int				nReloadUnitSize;		//cate gloante incarca odata sau 0 daca nu se poate incarca
	float			fReloadTimePerUnit;		//cat timp dureaza sa incarce o unitate de glont

	float			fFireRateWait;			//fire rate  - cat timp trebuie sa treaca intre gloante
	bool			bResetFireRateOnTriggerUp;	//cand ridici de pe fire reseteaza fire rate
	bool			bUsesMainWeaponAmmo;		//for alt fire weapons: este doar un mod de tragere care foloseste aceeasi munitie ca si arma principala (aimed shot, double tap, etc)

	int				nBulletChamberSize;		//daca are bullet chamber sau nu (0 sau 1) - se aduna la bullets left. Nu poti seta chamber size mai mare
	bool			bAnimSync;				//poate trage doar pe animatii non looping, pe frame cu action flag
	bool			bCanShootFromCrouch, bCanShootFromAir, bCanShootFromLadders, bCanShootFromCover;
	int				nDropShellFrame;		//frame number of shell from SHELLS animation (-1 - no shell)
	int				nBurstSize;				//cate gloante trage intr-un burst (0 pt full automatic)
	float			fBurstCooldown;			//dupa cat timp de la burst poate trage din nou

	float			fJammedDuration;		//durata de blocare a armei cand ia damage
	float			fMuzzleLightSize;		//size of lighting effect when shooting
	bool			bHasLaserSight;			//daca are laser sight
	float			fShooterSpeedSlowingPercent; //procentul cu care scade viteza tragatorului daca se misca in timp ce trage
	//TODO: camera recoil se va face diferit
	float			fCameraRecoil;			//recul camera
	float			fSoundRadius;			//cat de departe se aude?
	bool			bPassive;				//arma pasiva, nu se foloseste ca si arma normala, se citesc doar proprietatile

	CStringHash		shScript_OnFire;		//called when shooting a weapon. If not set it just shoots the weapon.
	CStringHash		shScript_OnFireALT;		//called when shooting ALT mode for weapon. If not set it just shoots the ALT weapon.
	CStringHash		shScript_OnEmpty;		//called when weapon is empty

	//sounds - indexuri de sunete
	int		sndidxShoot, sndidxReload, sndidxEmpty;
	//alternatives
	int		sndidxShoot2, sndidxReload2, sndidxEmpty2;
	//bullets can trigger a sound action (verse) on the actor (grenades trigger "FIRE IN THE HOLE" verse)
	EActorSoundVerse	sndActorVerse;

	CWeaponTemplate() :
		//generic data
		fSpeedPenaltyPercent(0.0f),
		//other data
		nBulletsPerShot(5), bAnimSync(false),
		fFireRateWait(0.0f), fMuzzleLightSize(0.0f), nClipSize(10),
		fReloadTimePerUnit(1.0f), nReloadUnitSize(1),
		fAimErrorMaxFOV(0.0f), fAimErrorAddPerShot(0.0f), fAimErrorCooldownPerSecond(1.0f), fSpreadFOV(0.0f), fAimErrorMulPerShot(1.0f),
		bCanShootFromCrouch(true), bCanShootFromAir(false), bCanShootFromLadders(false), bCanShootFromCover(false), nDropShellFrame(-1),
		nBurstSize(0), bResetFireRateOnTriggerUp(false), bUsesMainWeaponAmmo(false), fShooterSpeedSlowingPercent(0.0f), nBulletChamberSize(0),
		sndidxShoot(-1), sndidxReload(-1), sndidxEmpty(-1), sndidxShoot2(-1), sndidxReload2(-1), sndidxEmpty2(-1),
		fJammedDuration(0.0f), fSoundRadius(128.0f), fBurstCooldown(0.0f), bHasLaserSight(false), bPassive(false),
		nHUD_AnimIdx(-1), nHUD_AnimIdxALT(-1), nMuzzleFlashAnim(-1), fAimFOV(0.0f),
		sndActorVerse(K_LVL_ACT_VERSE_EMPTY)
	{
		fCameraRecoil = 4.0f;
		name.Reset();
		shTemplateOverwrite.Reset();
		
		shScript_OnEmpty.Reset();
		shScript_OnFire.Reset();
		shScript_OnFireALT.Reset();
	}
};

enum EnumWeaponStatus {	
	K_LVL_WPN_STATUS_UNKNOWN = -1,		//not initialized!

	K_LVL_WPN_STATUS_READY = 0,		//ready to shoot
	K_LVL_WPN_STATUS_COOLING,		//waiting between shots
	K_LVL_WPN_STATUS_JUST_SHOT,		//status setat dupa fiecare glont tras
	//--- de aici sunt stari in care CanShootWeapon intoarce false ---
	K_LVL_WPN_STATUS_RELOADING,		//reloading
	K_LVL_WPN_STATUS_JAMMED,		//cand se blocheaza arma (de ex cand isi ia stun parentul)
	K_LVL_WPN_STATUS_BURST_END,		//la capat de burst
	K_LVL_WPN_STATUS_NO_AMMO,		//cand ramane fara gloante
};

//consumable perks that can be set on weapons
struct CWeaponPerk {
	bool	bEnabled;			//is it enabled?
	float	fDamage_percAdd;	//damage that gets added from original (0.0f default)
	float	fROF_percAdd;		//rate of fire percent added (0.0f default)
	int		nDurationShots;		//how many shots is it active?
	float	fDurationTime;		//how much time is it active?

	CWeaponPerk() : fDamage_percAdd(0.0f), fROF_percAdd(0.0f), fDurationTime(0.0f), nDurationShots(0), bEnabled(false)
	{}

	void Reset()
	{
		bEnabled = false;
		fDamage_percAdd = 0.0f;
		fROF_percAdd = 0.0f;
		nDurationShots = 0;
		fDurationTime = 0.0f;
	}
};

class CWeapon
{
public:
	//#TODO #MAYBE:sa am un array de templates pentru modurile de tragere sau poate va fi in template-ul armei.
	CWeaponTemplate WeaponTemplate;
public:
	EnumWeaponStatus		status;					//status arma: ready, reloading
	EnumWeaponStatus		statusOld;				//status vechi arma: ca sa stim cand abia s-a schimbat
	//consumabile
	float	fAimErrorFOV;			//FOV-ul curent de eroare aim
	int		m_nBurstBulletsShot;	//cate gloante s-au tras din burst 
	int		m_nBulletsShotSinceCool; //how many bullets were shot in a burst since weapon was cool
	int		ammoLeft;				//-1 pt nr infinit de gloante
	float	fireRateTimer;			//timer de fire rate
	float	reloadTimer;			//timer reload
	float	fJammedTimer;			//timer jammed weapon
	int		nCanResetJamCount;		//can reset jam timer a few times (used usually when changing from one weapon to another so it doesn't shoot right away)
	bool	bPaintLaserSight;		//daca sa deseneze laser sight
	float	fTimeSinceShot;			//timpul de la ultimul glont tras
	//perks
	CWeaponPerk		m_activePerk;	//active weapon perk (could be an array if needed)
	//controls
	bool	bTriggerDown, bTriggerDownOld;	//e apasat tragaciul? (si starea anterioara)
	bool	bReloadDown;			//e apasat reload-ul?
	CActor*	pOwner;					//ownerul armei
	CSprite	m_sprMuzzleFlash;		//sprite pentru muzzle flash
	//ctor
	CWeapon() : status(K_LVL_WPN_STATUS_UNKNOWN), statusOld(K_LVL_WPN_STATUS_UNKNOWN),
		fAimErrorFOV(0.0f), fireRateTimer(0.0f), m_nBurstBulletsShot(0), m_nBulletsShotSinceCool(0),
		reloadTimer(0.0f), ammoLeft(-1), fJammedTimer(0.0f), nCanResetJamCount(0),
		bTriggerDown(false), bReloadDown(false), bTriggerDownOld(false),
		pOwner(null), bPaintLaserSight(false), fTimeSinceShot(0.0f)
	{}

	void Init();
	void SetTriggerStates(bool bTriggerPushed, bool bReloadPushed);
};

///--------------------------------------------------------------------------
///--- DECALS : clasa folosita pentru afisarea urmelor pe pereti ---
///--------------------------------------------------------------------------
enum EDecalLayer
{
	K_LVL_DECAL_LAYER_BACKWALLS = 0,    //blood and stains (apar clipuite la pereti)
	K_LVL_DECAL_LAYER_BACKOBJECTS = 1,  //shells and such (apar intregi)
	//ultimul din enum
	K_LVL_DECAL_LAYERS
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




///--------------------------------------------------------------------------
///--- PHYSICS PARTICLES ---
///--------------------------------------------------------------------------
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

//pool de obiecte speciale cu fizica sau fara, de tipul cartuse cu coliziune, bucati de carne care genereaza sange si splaturi pe pereti cand se lovesc
#define K_LVL_PROPS_MAX_CNT 256
//types
enum ESpecialPropType {
	K_SPROP_NOT_SET = -1,

	K_SPROP_SHELL = 0,	//cartusele jucatorului
	K_SPROP_MEAT = 1,	//carnea care sare din oameni
	K_SPROP_SHRAPNEL_SMOKING, //bucati de bomba care lasa fum in urma
	K_SPROP_LIGHT,		//lumina temporara pentru arme, explozii, etc. Deseneaza din m_sprLights.
	K_SPROP_EXPLOSION,	//explozie care deformeaza ecranul (si deseneaza si explozia (cu particule))
	K_SPROP_FIRE_SOURCE, //o bucata de foc care moare dupa un timp dar loveste toti oamenii
	K_SPROP_GOO,			//green goo
};

class CSpecialProp {
public:
	ESpecialPropType	type;		
	CLinkedPool<CPhysicsPoint2D>::CLinkedPoolNode *physPt; //punctul fizic (coliziune, pozitie, etc)

	int			nSubType;	//folosit de fiecare tip in mod diferit
	float		fTimer;		//timer care porneste de la 0
	bool		bAnimated;	//daca e animat sprite-ul
	CSprite		spr, spr2;	//grafica din Props (unele au nevoie de 2 sprites)
	float		fSize;		
	//--- variabile lumini ---
	bool		bMakesLight;
	CSprite		sprLight;
	float		fLightDuration, fLightFadeOut;	//durata totala a luminii si durata de fade out
	float		fLightScaling;
	float		fLightTimer;		//timer-ul de viata al luminii
	//--- diverse variabile ---
	bool		bVar1;
	int			nIntVar1;

	CSpecialProp() : physPt(null), type(K_SPROP_NOT_SET), nSubType(0), fTimer(0.0f), bAnimated(false), fSize(1.0f),
		bMakesLight(false), fLightDuration(0.0f), fLightFadeOut(0.0f), fLightScaling(1.0f), fLightTimer(0.0f), bVar1(false), nIntVar1(0)
	{
	}

	void Reset()
	{
		physPt = null; 
		type = K_SPROP_NOT_SET; nSubType = 0; fTimer = 0.0f; bAnimated = false; fSize = 1.0f;
		bMakesLight = false; fLightDuration = 0.0f; fLightFadeOut = 0.0f; fLightScaling = 1.0f; fLightTimer = 0.0f;
	}
};




///--------------------------------------------------------------------------
/// MISC OBJECTS - diverse obiecte speciale exportate din editor (RAILS, etc)
///--------------------------------------------------------------------------
enum EMiscObjectType
{
	K_LVL_MISC_UNDEFINED = -1,
	K_LVL_MISC_RAILS = 0,		//params: none
	K_LVL_MISC_BACKGROUND = 1,	//params: str_bsx = night.bsx (seteaza fundalul nivelului, apare unul singur pe nivel)
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
	CFixedArray<D3DXVECTOR2, 32> arrPoints; //maxim 32 de puncte pe un rail
	CFixedArray<float, 32> arrLenghts;		//lungimea parcursa pana in fiecare nod
	float fLength;							//lungimea totala

	//intoarce pozitia pe rail in functie de cursor intre 0 si 1
	D3DXVECTOR2 GetPosNormalized(float fCursorNormalized, D3DXVECTOR2 * retDir = NULL);
	//intoarce pozitia pe rail in fn de distanta parcursa (si directia normalizata)
	D3DXVECTOR2 GetPos(float fDistFromStart, D3DXVECTOR2 * retDir = NULL);
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
	D3DXVECTOR2 pos;
	CSprite sprite;
	CAABB aabb_ini;  //bbox initial

	CMiscObject_FrontLayerObj()
	{
		type = K_LVL_MISC_FRONTLAYEROBJ;
	}
};



///--------------------------------------------------------------------------
/// SCREEN VIGNETTE
///--------------------------------------------------------------------------

class CScreenVignette
{
public:
	CSprite sprite;
	DWORD colorBase;
	float fDuration; //durata totala de afisare
	float fFadeInTime, fFadeOutTime;
	float fLife;	//viata curenta (0.0f inseamna ca nu deseneaza)
	float fAlpha, fMaxAlpha;	//alpha calculata in update

	CScreenVignette() : colorBase(0x00000000), fDuration(0.0f), fLife(0.0f), fFadeInTime(0.0f), fFadeOutTime(0.0f), fAlpha(0.0f), fMaxAlpha(1.0f)
	{
		sprite.Init(ANM_CONTROLS_SPR_VIGNETTES, 0.0f, 0.0f, 0x00000000);
	}

	void Init(float nfLife, DWORD nColor = 0xff000000, float nfFadeInTime = 0.0f, float nfFadeOutTime = 0.0f, float nfMaxAlpha = 1.0f)
	{
		fLife = fDuration = nfLife;
		colorBase = nColor;
		fFadeInTime = nfFadeInTime;
		fFadeOutTime = nfFadeOutTime;
		fMaxAlpha = nfMaxAlpha;

		//daca am fade in pornesc de la alpha 0
		if (nfFadeInTime > 0.0f)
		{
			fAlpha = 0.0f;
		}
		else
		{
			fAlpha = 1.0f;
		}

		sprite.color = D3DCOLOR_COLORALPHA(colorBase, fAlpha * fMaxAlpha);
	}

	void Update(float dTime)
	{
		if (fLife <= 0.0f)
			return;

		fLife -= dTime;
		//daca sunt in zona de fade in
		if ((fDuration - fLife) < fFadeInTime)
		{
			fAlpha = (fDuration - fLife) / fFadeInTime;
		}
		else if (fLife < fFadeOutTime) //fade out
		{
			fAlpha = fLife / fFadeOutTime;
		}
		else //full color
		{
			fAlpha = 1.0f;
		}

		if (fLife <= 0.0f)
		{
			fLife = 0.0f;
			fAlpha = 0.0f;
		}

		sprite.color = D3DCOLOR_COLORALPHA(colorBase, fAlpha * fMaxAlpha);
	}

	void Paint(ID3DXSprite* pSprite, CSpriteCollection* sprCol)
	{
		if (fLife <= 0.0f)
			return;
		D3DXMATRIXA16 mattrans;
		RECTXYWH_F bbox = sprCol->GetAFrameBBox_real(sprite.animationIdx, 0);
		D3DXMatrixAffineTransformation2D(&mattrans, UTGetAppClass().g_rectRender.h / bbox.h, NULL, 0.0f, &UTGetAppClass().g_rectRender.Center());
		pSprite->SetTransform(&mattrans);
		sprite.paint(sprCol);
		pSprite->Flush();
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


