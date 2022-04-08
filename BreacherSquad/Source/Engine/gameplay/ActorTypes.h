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
	K_ACT_ATTACK_IDLE,
	K_ACT_ATTACK_RELOADING,
	K_ACT_ATTACK_SHOOTING,
	K_ACT_ATTACK_SHOOTING_ALT,
	K_ACT_ATTACK_USING_GEAR,
	K_ACT_ATTACK_MELEE,
	K_ACT_ATTACK_BREACH,  //used mostly for breaching doors
};

