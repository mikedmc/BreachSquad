#pragma once



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
	K_SD_BONE_GUN_MOUNT,

	K_SD_BONES_CNT
};
// specifies xml node names, must be same order as above
const CStringHash ESpineSpecialBoneNames[K_SD_BONES_CNT] =
{
	L"BONE_AIM_IK",
	L"BONE_GUN_MOUNT",
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

