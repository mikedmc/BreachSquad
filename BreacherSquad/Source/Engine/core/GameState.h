#pragma once

///--- GAME STATES ---
enum eGameState {
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

class GameState {
public:
	static eGameState			state;						// main state of the app
	static int					substate;					// substate of the app

	// Changes current game state
	static void ChangeTo( eGameState newState, CVariantCollection * args = nullptr );
};

