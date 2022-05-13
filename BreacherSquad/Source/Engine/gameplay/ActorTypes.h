#pragma once

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


// Class of the actor to define friends and enemies
//--- ORDER IS VERY IMPORTANT ---
enum EActorClass
{
	K_ACT_CLASS_NOT_SET = -2,	// usually not used (only in initializations)
	K_ACT_CLASS_ANY = -1,		// filter for GetClosestTarget or parameter type

	K_ACT_CLASS_PASSIVE = 0,	// barrels, actors that are not characters
	K_ACT_CLASS_TRAP = 1,		// mindless class for traps
	K_ACT_CLASS_EXPLOSION,		// class for explosions
	K_ACT_CLASS_HOSTAGE,
	//from here only human-blood-stun classes (sorted by love from near to far) that kill each other
	K_ACT_CLASS_PLAYER,			//clasa player
	K_ACT_CLASS_FRIENDLY,		//main player friendly class
	//from here on you get SP on kills and they get pushed when too close (usually enemies)
	K_ACT_CLASS_ENEMY,			//human enemies
	//count
	K_ACT_CLASSES_COUNT
};
// neutral classes are lower or equal to this
#define K_ACT_CLASSCHECKPOINT_NEUTRALS  K_ACT_CLASS_HOSTAGE

const CStringHash EActorClassNames[ K_ACT_CLASSES_COUNT ] =
{
	L"PASSIVE",
	L"TRAP",
	L"EXPLOSION",
	L"HOSTAGE",
	L"PLAYER",
	L"FRIENDLY",
	L"ENEMY",
};
