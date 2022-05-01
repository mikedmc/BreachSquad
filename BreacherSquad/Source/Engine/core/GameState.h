#pragma once

/// GAME STATES 
enum EGameState {
	GAME_STATE_EMPTY = 0,
	GAME_STATE_PRELOAD,					// loads stuff that don't need painting (like the shaders and strings)
	GAME_STATE_DEVELOPER,
	GAME_STATE_LOADING,

	GAME_STATE_MAINMENU,
	//networking
	GAME_STATE_NET_LOBBY,
	GAME_STATE_JOIN_COOP_LIST,			//screen that shows and updates the available lobbies

	GAME_STATE_GAME_MODE_SELECTION,
	GAME_STATE_CHAPTER_SELECTION,
	GAME_STATE_LEVEL_SELECTION,
	GAME_STATE_PLAYER_SELECTION,
	GAME_STATE_GAME,
	//workshop - mod selection
	GAME_STATE_WORKSHOP,
	//controls editor
	GAME_STATE_CONTROLSED,
	//mod uploading to steam
	GAME_STATE_UPLOAD_MOD
};

/// Types of possible screen transitions
enum ETransitionType {
	TRANSITION_NONE,
	TRANSITION_SIMPLE,
	TRANSITION_PIXELATE,
};

class GameState {
private:
	static bool					bDuringTransition;			// Is it during transition?
	static int					nTransitionStep;			// step of current transition
	static float				fTransitionPercent;			// percent of current transition
	static EGameState			eNextState;					// state to set after transition
	static ETransitionType		nTransitionType;			// type of transition

public:
	static EGameState			state;						// main state of the app
	static int					substate;					// substate of the app
	static float				fTimer;						// timer sometimes used for the state

	// Changes current game state
	static void ChangeTo( EGameState newState, CVariantMap * args = nullptr );

	// Changes current game state playing a transition 
	static void ChangeTo_Transition( EGameState newState, ETransitionType transitionType, CVariantMap * args = nullptr );
	
	// Tells if state is transitioning to another state (usually animated)
	static bool isTransitioning() {
		return bDuringTransition;
	}

	// Updates transition
	static void UpdateTransition( float dTime );

	// Paints screen transition
	static void PaintTransition( float dTime, float fTimeline, PDEVICE pDevice );
};

