#pragma once

///---------------------------------------------------------------------------------------------
///		SCRIPT PROCESSING
///---------------------------------------------------------------------------------------------
//#TODO: de schimbat parametrul "target" in "who" ca sa nu fie confuzie cu targetul elementelor
//SCRIPT INSTRUCTIONS
enum eLVLScriptInstruction {
	instr_IACTIVE_TOGGLE_HIDDEN,
	instr_IACTIVE_SET_AI,
	instr_IACTIVE_SET_AI_PARAMS,
	instr_IACTIVE_GET_AI_PARAM,
	instr_IACTIVE_SET_SCRIPT,
	instr_IACTIVE_RAIL_CHANGEDIR,
	instr_IACTIVE_CAMERA_SET_TARGET,
	instr_IACTIVE_SET_CAN_INTERACT,
	instr_IACTIVE_SET_TARGETPTR,
	instr_IACTIVE_ADD_AI_EVENT,
	instr_IACTIVE_SAVE_TOUCHER_UID,
	instr_IACTIVE_ADD_NOTIFICATION,
	instr_IACTIVE_ADD_NOTIFICATION_LOCKED_DOOR,
	instr_IACTIVE_REMOVE_NOTIFICATION,
	instr_IACTIVE_GENERATE_EFFECT,

	instr_ACTIVE_INC_FRAME,
	instr_ACTIVE_SET_ANIM,
	instr_ACTIVE_DOORFACE_SET_OPEN_TIMER,
	instr_ACTIVE_AMMOBOX_GIVE_AMMO,
	instr_ACTIVE_HEALTHBOX_GIVE_HEALTH,

	instr_ACTOR_TOUCHER_TELEPORT,
	instr_ACTOR_SET_COMMAND,
	instr_ACTOR_SPAWN,
	instr_ACTOR_SET_AI_STATE,
	instr_ACTOR_SET_TEMPLATE,
	instr_ACTOR_SET_TEMPLATE_RANDOM,
	instr_ACTOR_PERK_MODIFIER,
	instr_ACTOR_SWITCH_WEAPONS,
	instr_ACTOR_SET_WEAPON,
	instr_ACTOR_EQUIP_WEAPONS,
	instr_ACTOR_SHOOT_WEAPON,
	instr_ACTOR_JAM_WEAPON,
	instr_ACTOR_HIT,
	instr_ACTOR_SET_DOT,
	instr_ACTOR_SPAWN_AT,  // spawns actor at specific coords

	instr_COLL_ENTER_HIDDEN_ROOM,
	instr_COLL_CHECK_HIDDEN_ROOM_CLEARED,

	instr_LEVEL_HIDE_BACKGROUND,
	instr_LEVEL_SHOW_BACKGROUND,
	instr_LEVEL_TOGGLE_BACKGROUND,
	instr_LEVEL_CAMERA_RESTORE_LAST_TARGET,
	instr_LEVEL_SET_TIME_MULTIPLIER,
	instr_LEVEL_NOTIFY_ENGINE,
	instr_LEVEL_ENABLE_LIGHTNING,
	instr_LEVEL_GIVE_STRATEGIC_POINTS,

	//count
	instr_COUNT
};

//SCRIPT INSTRUCTIONS NAMES
const CStringHash eLVLScriptInstructionNames[] = {
	L"IACTIVE_TOGGLE_HIDDEN",
	L"IACTIVE_SET_AI",  //args: [target="SELF/target/id"] aiName="AI_UNDEFINED" aiparam1="val1" aiparam2="val2" ...
	L"IACTIVE_SET_AI_PARAMS", //args: [target="SELF/target/id"] aiparam1="val1" aiparam2="val2" ...
	L"IACTIVE_GET_AI_PARAM",	//args:[target = "SELF/target/id"] paramName = "numeParametruAI" destLocalVarName = "varLocalaScript"
	L"IACTIVE_SET_SCRIPT",  //args: [target="SELF/target/id"] scriptName="SCRIPT_NAME"
	L"IACTIVE_RAIL_CHANGEDIR", //args: [target="SELF/target/id"]
	L"IACTIVE_CAMERA_SET_TARGET",	//args: [target = "SELF/target/id"][teleport = "0/1"]
	L"IACTIVE_SET_CAN_INTERACT", //args: [target = "SELF/target/id"] bCanInteract = "0/1"
	L"IACTIVE_SET_TARGETPTR", //args: [target = "SELF/target/id"] targetID="ID/-1"
	L"IACTIVE_ADD_AI_EVENT",//args: [target = "SELF/target/id"] sEventType="AI_EVENT_STRANGE" fRange="128.0" fDuration="1.0"
	L"IACTIVE_SAVE_TOUCHER_UID",//args: sAIvarName="varName"
	L"IACTIVE_ADD_NOTIFICATION",//args: [target = "SELF/target/id"] sFontID="FONT_7_B1" sStringID="STR_ID" fDuration="0.0f" / >
	L"IACTIVE_ADD_NOTIFICATION_LOCKED_DOOR",//args: [target = "SELF/target/id"] fDuration="0.0f" / >
	L"IACTIVE_REMOVE_NOTIFICATION",
	L"IACTIVE_GENERATE_EFFECT",	//args: [target = "SELF/target/id"] sEffectType="EFFECT_TYPE_NAME"

	L"ACTIVE_INC_FRAME",  //args: [target="SELF/target/id"] step="n/-n" loop="0/1"
	L"ACTIVE_SET_ANIM",  //args: [target="SELF/target/id"] [anim="ANIM_NAME_FROM_BSX"] [frame="0"] [animated="0/1"]
	L"ACTIVE_DOORFACE_SET_OPEN_TIMER",  //args: [target="SELF/target/id"] fDuration="0.0"
	L"ACTIVE_AMMOBOX_GIVE_AMMO", //no args
	L"ACTIVE_HEALTHBOX_GIVE_HEALTH", //no args

	L"ACTOR_TOUCHER_TELEPORT",  //args: [where="SELF/target/id"] [offX="10"] [offY="10"]
	L"ACTOR_SET_COMMAND",  //args: [who="..."] [sCommand="COMMAND_MELEE from EControllerCommandNames"] 
	L"ACTOR_SPAWN",  //args: [where="SELF/target/id"]  template="TEMPLATE_NAME" AIstate="AI_STATE_NAME" [direction="-1/0/1/player"] [offX="10"] [offY="10"]
	L"ACTOR_SET_AI_STATE",  //args: [where="SELF/target/id"] AIstate="AI_STATE_NAME"
	L"ACTOR_SET_TEMPLATE",  //args: who = "SELF/target/targets_target/toucher/id" sTemplateName = "ACTOR_TEMPLATE"
	L"ACTOR_SET_TEMPLATE_RANDOM",  //args: who = "SELF/target/targets_target/toucher/id" sTemplateName1="ACTOR_TEMPLATE" fProbability1="10.0" [sTemplateName2="ACTOR2"] fProbability2="20.0"
	L"ACTOR_PERK_MODIFIER", //args: who sPerkName fQtyAdded
	L"ACTOR_SWITCH_WEAPONS", //args: who="..." nWpnIdx_src="X" nWpnIdx_dest="Y" nKeepAmmoFromSrc="0/1" nRefillAmmo="0/1"
	L"ACTOR_SET_WEAPON", //args: who="SELF/target/targets_target/toucher/id" nWpnIdx="X" sWpnTemplate="WPN_TEMPLATE"
	L"ACTOR_EQUIP_WEAPONS",//args: who="SELF/target/targets_target/toucher/id"  nPrimaryIdx="X" nSecondaryIdx="Y" nGearIdx="Z" 
	L"ACTOR_SHOOT_WEAPON", //args: who="SELF/target/targets_target/toucher/id"  [nWeaponIdx="X"]
	L"ACTOR_JAM_WEAPON",	//args: who="..." nWeaponIdx="X" nCanResetJam="Y"
	L"ACTOR_HIT",			//args: who="SELF/target/targets_target/toucher/id" fDamage="100.0"
	L"ACTOR_SET_DOT",		//args: who="..." sDoT="DoT_INVINCIBLE" [fDuration="1.0"]
	L"ACTOR_SPAWN_AT",		//args: template="" posX="world coord" posY="world coord"

	L"COLL_ENTER_HIDDEN_ROOM", //args: [target="SELF/target/id"] [lockCamera="1"] [hideOutside="1"]
	L"COLL_CHECK_HIDDEN_ROOM_CLEARED", //args: [target="SELF/target/id"]

	L"LEVEL_HIDE_BACKGROUND",  //args: none
	L"LEVEL_SHOW_BACKGROUND",  //args: none
	L"LEVEL_TOGGLE_BACKGROUND",  //args: none
	L"LEVEL_CAMERA_RESTORE_LAST_TARGET",  //args: none
	L"LEVEL_SET_TIME_MULTIPLIER",  //args: fValue="1.0" fDuration="5.0"
	L"LEVEL_NOTIFY_ENGINE",  //args: varName1="val1" varName2="val2" ...
	L"LEVEL_ENABLE_LIGHTNING",  //args: none
	L"LEVEL_GIVE_STRATEGIC_POINTS",  //args: [fPoints="1.0"]

};


