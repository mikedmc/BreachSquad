#pragma once

//#TODO: clasa folosita in actori si incapsuleaza toate variabilele necesare pentru definirea totala a unui state
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

// Actor attack states
//--- ORDER IS VERY IMPORTANT ---
enum EActorAttackState {
	K_LVL_ACT_ATTACK_IDLE,
	K_LVL_ACT_ATTACK_RELOADING,
	K_LVL_ACT_ATTACK_SHOOTING,
	K_LVL_ACT_ATTACK_SHOOTING_ALT,
	K_LVL_ACT_ATTACK_USING_GEAR,
	K_LVL_ACT_ATTACK_MELEE,
	K_LVL_ACT_ATTACK_BREACH,  //used mostly for breaching doors
};


// Holds all info that comes into the actor sensors
//#TODO: should include more enemies not just a focused one
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

// Commands that get sent to the AIs
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

