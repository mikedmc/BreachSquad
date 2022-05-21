#pragma once

class CEventTypes
{
//EVENT TYPES
public:
	static const CStringHash		evtT_SOUND;			// sound commands
	static const CStringHash		evtT_CONTROLS;		// ControlsManager events
	static const CStringHash		evtT_GAMESTATE;		// gamestate change messages
	static const CStringHash		evtT_SYSTEM;		// system commands 
	static const CStringHash		evtT_INFO;			// informational events	- no actions should be done on this kind of event
};

class CEventCommands
{
public:
	//Sound Commands
	static const CStringHash evtC_SOUND_PLAY_IDX;				//int sndIdx, DWORD sndFlags
	static const CStringHash evtC_SOUND_PLAY_NAME;				//WCHAR sndName, DWORD sndFlags
	static const CStringHash evtC_SOUND_PLAY_HASH;				//UINT32 sndNameHash, DWORD sndFlags

	static const CStringHash evtC_SOUND_STOP_IDX;				//int sndIdx, bool bFadeOut
	static const CStringHash evtC_SOUND_STOP_NAME;				//WCHAR sndName, bool bFadeOut
	static const CStringHash evtC_SOUND_STOP_HASH;				//UINT32 sndNameHash, bool bFadeOut

	static const CStringHash evtC_SOUND_SET_GROUP_VOLUME;		//strGroupName, fVolume, bFade
	static const CStringHash evtC_SOUND_STOP_GROUP;				//strGroupName, bFadeout, bResetSound

	//Controls/windows Commands
	static const CStringHash evtC_CONTROLS_CLICK;				//UINT32 layerID, UINT32 ctrlID
	static const CStringHash evtC_CONTROLS_SLIDER_CHANGED;		//UINT32 layerID, UINT32 ctrlID, float fSlidePercent
	static const CStringHash evtC_CONTROLS_CHECK_CHANGED;		//UINT32 layerID, UINT32 ctrlID, bool bChecked
	static const CStringHash evtC_CONTROLS_SELECTION_CHANGED;	//UINT32 layerID, UINT32 ctrlID, int selectedIdx, int oldIdx
	static const CStringHash evtC_CONTROLS_PAGE_CHANGED;		//UINT32 layerID, UINT32 ctrlID, int nPageIdx, int nOldIdx

	//GAMESTATE
	static const CStringHash evtC_GAMESTATE_CHANGE;				//UINT32 newGameState, int arg1, int arg2
	static const CStringHash evtC_GAMESTATE_CHANGE_TRANSITION;	//UINT32 newGameState, int transitionType, int arg1, int arg2

	//SYSTEM
	static const CStringHash evtC_SYSTEM_RESOLUTION_CHANGE;		//UINT32 width, UINT32 height
	static const CStringHash evtC_SYSTEM_CONTROLLER_ADDED;		//int SDLinstanceID, WCHAR strName
	static const CStringHash evtC_SYSTEM_CONTROLLER_REMOVED;	//int SDLinstanceID, WCHAR strName
};

