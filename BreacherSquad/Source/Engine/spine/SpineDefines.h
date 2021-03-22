#pragma once


///--------------------------------------------------------------------------
/// ANIMATIONS
///--------------------------------------------------------------------------

// Specifies animation indexes for quick access
enum ESpineAnim
{
	K_SD_ANIM_NOT_SET = -2,
	K_SD_ANIM_EMPTY = -1,

	K_SD_ANIM_IDLE,
	K_SD_ANIM_IDLE_CROUCH,
	K_SD_ANIM_MOVE,
	K_SD_ANIM_MOVE_FAST,
	K_SD_ANIM_MOVE_BK,
	K_SD_ANIM_JUMP,
	K_SD_ANIM_FALL,
	K_SD_ANIM_AIM,
	K_SD_ANIM_SHOOT,
	K_SD_ANIM_SHOOT_ALT,
	K_SD_ANIM_CLIMB_UP,
	K_SD_ANIM_DIE,

	K_SD_ANIMS_CNT
};
// must be same order as EAnims 
const CStringHash ESpineAnimNames[K_SD_ANIMS_CNT] =
{
	L"IDLE",
	L"IDLE_CROUCH",
	L"MOVE",
	L"MOVE_FAST",
	L"MOVE_BK",
	L"JUMP",
	L"FALL",
	L"AIM",
	L"SHOOT",
	L"SHOOT_ALT",
	L"CLIMB_UP",
	L"DIE",
};


///----------------------------------------------------
/// SPINE EVENTS
///----------------------------------------------------
enum ESpineEvent
{
	K_SD_EVENT_EMPTY = -1,

	K_SD_EVENT_SHOOT,
	K_SD_EVENT_FOOTSTEP,

	K_SD_EVENTS_CNT
};
// specifies xml node names, must be same order as above
const CStringHash ESpineEventNames[K_SD_EVENTS_CNT] =
{
	L"EVENT_SHOOT",
	L"EVENT_FOOTSTEP",
};


///----------------------------------------------------
/// SPINE SPECIAL BONES
///----------------------------------------------------
enum ESpineSpecialBone
{
	K_SD_BONE_EMPTY = -1,

	K_SD_BONE_AIM_IK,
	K_SD_BONE_LEFT_FOOT_IK,
	K_SD_BONE_RIGHT_FOOT_IK,

	K_SD_BONES_CNT
};
// specifies xml node names, must be same order as above
const CStringHash ESpineSpecialBoneNames[K_SD_BONES_CNT] =
{
	L"BONE_AIM_IK",
	L"BONE_HANDBK_GUN",
	L"BONE_HANDFR_GUN"
};


///----------------------------------------------------
/// SLOTS NAMES
///----------------------------------------------------
enum ESpineSlot
{
	K_SD_SLOT_EMPTY = -1,

	K_SD_SLOT_LEFT_HAND,
	K_SD_SLOT_LEFT_HAND_TOP,
	K_SD_SLOT_RIGHT_HAND,
	K_SD_SLOT_RIGHT_HAND_TOP,

	K_SD_SLOTS_CNT
};
// specifies xml node names, must be same order as above
const CStringHash ESpineSlotNames[K_SD_SLOTS_CNT] =
{
	L"SLOT_LEFT_HAND",
	L"SLOT_LEFT_HAND_TOP",
	L"SLOT_RIGHT_HAND",
	L"SLOT_RIGHT_HAND_TOP"
};

