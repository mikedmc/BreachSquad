#pragma once

class CEventTypes
{
//EVENT TYPES
public:
	static CStringHash		evtT_SOUND;		 //sound event
	static CStringHash		evtT_CONTROLS;   //interface events - cele legate de ControlsManager
	static CStringHash		evtT_GAMESTATE;  //mesaje pentru schimbarea came state-ului
	static CStringHash		evtT_SYSTEM;     //mesaje de system. Poate fi rezolutie, mouse, etc
};

class CEventCommands
{
public:
	//Sound Commands
	static CStringHash evtC_SOUND_PLAY_IDX;				//int sndIdx, DWORD sndFlags
	static CStringHash evtC_SOUND_PLAY_NAME;				//WCHAR sndName, DWORD sndFlags
	static CStringHash evtC_SOUND_PLAY_HASH;				//UINT32 sndNameHash, DWORD sndFlags

	static CStringHash evtC_SOUND_STOP_IDX;				//int sndIdx, bool bFadeOut
	static CStringHash evtC_SOUND_STOP_NAME;				//WCHAR sndName, bool bFadeOut
	static CStringHash evtC_SOUND_STOP_HASH;				//UINT32 sndNameHash, bool bFadeOut

	static CStringHash evtC_SOUND_SET_GROUP_VOLUME;	//strGroupName, fVolume, bFade
	static CStringHash evtC_SOUND_STOP_GROUP;			//strGroupName, bFadeout, bResetSound

	//Controls/windows Commands
	static CStringHash evtC_CONTROLS_CLICK;				//UINT32 layerID, UINT32 ctrlID
	static CStringHash evtC_CONTROLS_SLIDER_CHANGED;	//UINT32 layerID, UINT32 ctrlID, float fSlidePercent
	static CStringHash evtC_CONTROLS_CHECK_CHANGED;		//UINT32 layerID, UINT32 ctrlID, bool bChecked
	static CStringHash evtC_CONTROLS_SELECTION_CHANGED;	//UINT32 layerID, UINT32 ctrlID, int selectedIdx, int oldIdx
	static CStringHash evtC_CONTROLS_PAGE_CHANGED;		//UINT32 layerID, UINT32 ctrlID, int nPageIdx, int nOldIdx

	//GAMESTATE
	static CStringHash evtC_GAMESTATE_CHANGE;				//UINT32 newGameState, int arg1, int arg2
	static CStringHash evtC_GAMESTATE_CHANGE_TRANSITION;	//UINT32 newGameState, int transitionType, int arg1, int arg2

	//SYSTEM
	static CStringHash evtC_SYSTEM_RESOLUTION_CHANGE;		//UINT32 width, UINT32 height
	static CStringHash evtC_SYSTEM_CONTROLLER_ADDED;		//int SDLinstanceID, WCHAR strName
	static CStringHash evtC_SYSTEM_CONTROLLER_REMOVED;		//int SDLinstanceID, WCHAR strName
};

