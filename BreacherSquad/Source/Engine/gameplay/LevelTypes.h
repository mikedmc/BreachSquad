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
	K_LVL_FX_EMPTY = -1,

	K_FX_EXPLONICE_SM1 = 0,
	K_FX_EXPLONICE_BIG1,
	K_FX_ELECTRIC_BREAK_SPARKS,
	K_FX_EXPLO_LARGE,
	K_FX_STONE_BREAK,

	K_FXS_CNT,
};

const CStringHash ELVLEffectTypeNames[] = {
	L"FX_EXPLONICE_SM1",
	L"FX_EXPLONICE_BIG1",
	L"FX_ELECTRIC_BREAK_SPARKS",
	L"FX_STARS_CONFETTI",
	L"FX_EXPLO_LARGE",
	L"FX_STONE_BREAK", 
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
	// used more as a parameter for the rendering functions like paintBullets and not as a separate step/pass
	// might go away in the final version
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
		eExcludedActClass = K_ACT_CLASS_ANY; //nu exclude nimic. Putin fortat ANY asta dar merge
		eFilteredActClass = K_ACT_CLASS_ANY; //don't filter anything
		nOwnerUID = 0;

		fVar1 = 0.0f;
	}

	bool Set(EDoTType eDoTType, float nfDuration = 0.0f, float nfDamagePerSec = 0.0f, EActorClass eExcludedClass = K_ACT_CLASS_ANY, EActorClass eFilterClass = K_ACT_CLASS_ANY, UINT32 unOwnerUID = 0)
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
		eExcludedActClass = K_ACT_CLASS_ANY;
		eFilteredActClass = K_ACT_CLASS_ANY; //don't filter anything
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
	CStringHash	shFX;		// name of FX to be instantiated
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
		fSoundRadius(64.0f), nShrapnelCnt(0), nNapalmCnt(0), fMaxImpulse(0.0f), fDoTRadius(0.0f), nArmorPiercingRating(1), eIgnoreActorClass(K_ACT_CLASS_ANY)
	{
		name.Reset();
		shFX.Reset();
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

// base class for generic editor objects (rails and other stuff)
class CMiscObjectBase
{
private:
	UINT32	UID;

public:
	int		ID; //id din editor
	EMiscObjectType type; //tipul obiectului

	CVariantMap	varParams;	//parametrii primiti din editor, specifici fiecarui tip (vezi comentarii EMiscObjectTypes)

	//CTOR/DTOR
	CMiscObjectBase() : ID(-1), type(K_LVL_MISC_UNDEFINED)
	{
		UID = GenerateUID();
		varParams.Clear();
	}
	virtual ~CMiscObjectBase() //virtual - cheama constructorul claselor derivate daca dezaloci prin pointer de baseClass
	{
		varParams.Clear();
	}
};

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
/// All these actions are collected from all elements when action list gets created (when interacting)
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