#pragma once


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
//max allowed speed
#define K_LVL_ACTOR_MAX_SPEED 400.0f
//max lateral impulse
#define K_LVL_ACTOR_MAX_IMPULSE 1200.0f
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

//maximum number of strategic points
#define K_LVL_MAX_STRATEGIC_POINTS 8
#define K_LVL_STRATEGIC_POINTS_ADDED_BY_PERK 2
