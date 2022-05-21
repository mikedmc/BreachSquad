#include "dxstdafx.h"
#include "ConstEvents.h"

///EVENT TYPES
const CStringHash CEventTypes::evtT_SOUND(L"EVTTYPE_SOUND");
const CStringHash CEventTypes::evtT_CONTROLS(L"EVTTYPE_CONTROLS");
const CStringHash CEventTypes::evtT_GAMESTATE(L"EVTTYPE_GAMESTATE");
const CStringHash CEventTypes::evtT_SYSTEM(L"EVTTYPE_SYSTEM");
const CStringHash CEventTypes::evtT_INFO( L"INFO" );
///EVENT COMMANDS
//--- SOUND ---
const CStringHash CEventCommands::evtC_SOUND_PLAY_IDX(L"SND_PLAY_IDX");				
const CStringHash CEventCommands::evtC_SOUND_PLAY_NAME(L"SND_PLAY_NAME");
const CStringHash CEventCommands::evtC_SOUND_PLAY_HASH(L"SND_PLAY_HASH");

const CStringHash CEventCommands::evtC_SOUND_STOP_IDX(L"SND_STOP_IDX");				
const CStringHash CEventCommands::evtC_SOUND_STOP_NAME(L"SND_STOP_NAME");				
const CStringHash CEventCommands::evtC_SOUND_STOP_HASH(L"SND_STOP_HASH");				

const CStringHash CEventCommands::evtC_SOUND_SET_GROUP_VOLUME(L"SND_SET_GROUP_VOLUME");	
const CStringHash CEventCommands::evtC_SOUND_STOP_GROUP(L"SND_STOP_GROUP");			

//--- CONTROLS ---
const CStringHash CEventCommands::evtC_CONTROLS_CLICK(L"CONTROLS_CLICK");
const CStringHash CEventCommands::evtC_CONTROLS_SLIDER_CHANGED(L"CONTROLS_SLIDER_CHANGED");
const CStringHash CEventCommands::evtC_CONTROLS_CHECK_CHANGED(L"CONTROLS_CHECK_CHANGED");
const CStringHash CEventCommands::evtC_CONTROLS_SELECTION_CHANGED(L"CONTROLS_SELECTION_CHANGED");
const CStringHash CEventCommands::evtC_CONTROLS_PAGE_CHANGED(L"CONTROLS_PAGE_CHANGED");

//--- GAMESTATE ---
const CStringHash CEventCommands::evtC_GAMESTATE_CHANGE(L"GAMESTATE_CHANGE");
const CStringHash CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION(L"GAMESTATE_CHANGE_TRANSITION");

//--- SYSTEM ---
const CStringHash CEventCommands::evtC_SYSTEM_RESOLUTION_CHANGE(L"SYS_RESOLUTION_CHANGE");
const CStringHash	CEventCommands::evtC_SYSTEM_CONTROLLER_ADDED(L"SYS_CONTROLLER_ADDED");
const CStringHash	CEventCommands::evtC_SYSTEM_CONTROLLER_REMOVED(L"SYS_CONTROLLER_REMOVED");
