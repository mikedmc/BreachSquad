#pragma once

// Events coming from the editor, on actor animations
enum EAnimEvent {
	FEVT_NONE = 0,

	FEVT_SOUND = 1,
	FEVT_SHOOT = 2,

	FEVTS_COUNT
};

// available active weapon slots on current actor
enum EWpnSlot {
	K_WPNSLOT_NONE = -1,

	K_WPNSLOT_PRIMARY = 0,
	K_WPNSLOT_ALTFIRE = 1,
	K_WPNSLOT_GEAR,
	K_WPNSLOT_MELEE,
	K_WPNSLOT_EMPTYHANDS,		// Always empty slot for when we need to holster all weapons	

	K_WPNSLOTS_CNT
};
