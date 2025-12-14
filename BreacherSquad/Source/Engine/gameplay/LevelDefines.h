#pragma once


// nickname for weapons sprlib in sprites multi library
#define K_LIBNICK_WEAPONS	L"SPRLIB_WEAPONS"
#define K_LIBNICK_LIGHTS	L"SPRLIB_LIGHTS"
#define K_LIBNICK_PROPS		L"SPRLIB_PROPS"
#define K_LIBNICK_BULLETS	L"SPRLIB_BULLETS"

// Specifies animation indexes for quick access (for actors)
enum EActorAnim
{
	K_ACT_ANIM_NOT_SET = -2,
	K_ACT_ANIM_EMPTY = -1,

	K_ACT_ANIM_REFPOSE,
	K_ACT_ANIM_IDLE,
	K_ACT_ANIM_CROUCH,
	K_ACT_ANIM_WALK,
	K_ACT_ANIM_RUN,
	K_ACT_ANIM_KICK,
	K_ACT_ANIM_AIM,
	K_ACT_ANIM_SHOOT,
	K_ACT_ANIM_SHOOT_ALT,
	K_ACT_ANIM_DIE,

	K_ACT_ANIMS_CNT
};
// must be same order as EAnims 
const CStringHash EActorAnimNames[K_ACT_ANIMS_CNT] =
{
	L"REFPOSE",
	L"IDLE",
	L"CROUCH",
	L"WALK",
	L"RUN",
	L"KICK",
	L"AIM",
	L"SHOOT",
	L"SHOOT_ALT",
	L"DIE",
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
// numarul maxim de seturi de versuri
#define K_LVL_ACT_VERSES_MAX_SETS 2
//timeout same verse
#define K_LVL_ACT_VERSES_TIMEOUT 4.0f

const CStringHash EActorSoundVerseNames[ K_LVL_ACT_VERSES_COUNT ] =
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
