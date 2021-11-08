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
#define K_LVL_CAM_FOLLOW_SPRING_KS 1200.0f
#define K_LVL_CAM_FOLLOW_DAMPING_KD 40.0f
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

//#TODO: update them: The level editor exports these flags:
#define K_EDITOR_ACTIVE_FLAG_FLIPX 1
#define K_EDITOR_ACTIVE_FLAG_FLIPY 2
#define K_EDITOR_ACTIVE_FLAG_ANIMATED 4
#define K_EDITOR_ACTIVE_FLAG_IS_COVER 8

//level file format version from the Editor
#define K_EDITOR_LEVEL_FILE_FORMAT_VERSION 1015

// maximum number of allocated physics points
#define K_LVL_PHYSP_MAX_CNT 256


//maximum number of strategic points
#define K_LVL_MAX_STRATEGIC_POINTS 8
#define K_LVL_STRATEGIC_POINTS_ADDED_BY_PERK 2
