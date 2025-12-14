#include "dxstdafx.h"
#include "GameState.h"

///----------------------------------------------------------------------------------
/// Static Vars initialization
///----------------------------------------------------------------------------------
EGameState				GameState::state = GAME_STATE_EMPTY;
int						GameState::substate = 0;
float					GameState::fTimer = 0.0f;

bool					GameState::bDuringTransition = false;
int						GameState::nTransitionStep = 0;
float					GameState::fTransitionPercent = 0.0f;
EGameState				GameState::eNextState = GAME_STATE_EMPTY;
ETransitionType			GameState::nTransitionType = TRANSITION_NONE;


void GameState::SetState( EGameState newState, CVariantMap * args )
{
	LOG( L"System:: ChangeGameState(%d)", newState );
	EGameState oldGameState = GameState::state;

	///--- Release elements used in oldState ---
	ExitState( oldGameState, newState );

	///--- set new game state here ---
	GameState::state = newState;
	GameState::substate = 0;
	GameState::fTimer = 0.0f;

	switch ( newState )
	{
		case GAME_STATE_PRELOAD:
		{
			//change state to loading
			CEvent *nevent = new CEvent( CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION );

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
			nevent->AddNamedArgUINT32( L"newGameState", GAME_STATE_LOADING );
#else
			nevent->AddNamedArgUINT32( L"newGameState", GAME_STATE_DEVELOPER );
#endif

			nevent->AddNamedArgINT32( L"transitionType", TRANSITION_SIMPLE );
			__Events().QueueEvent( nevent );
		}
		break;
		case GAME_STATE_DEVELOPER:
		{
			UTApp().App_EnterState_Developer();
		}
		break;

		case GAME_STATE_LOADING:
		{
			g_bForceOneUpdatePerFrame = true;
			UTApp().App_EnterState_Loading();

			//on loading disable sync
			UTApp().m_Settings.devnet_eNetGameType = CApplicationSettings::K_NETGAME_TYPE_NO_NETWORK;
			UTApp().m_Settings.devnet_eSyncStatus = CApplicationSettings::K_NETGAME_SYNC_STOPPED;
		}
		break;

		case GAME_STATE_UPLOAD_MOD:
		{
			//some initial settings
			GameState::substate = 0;
			//force creating log window?
			UTApp().m_Settings.dev_bLogWindowShow = true;
		}
		break;

		case GAME_STATE_MAINMENU:
		{
			__Sim().GetNextRandomLevel();
#if defined(_DEBUG) || defined(DEBUG)
#if defined(ENABLE_ACHIEVEMENTS_RESET_ON_STARTUP)
			ErrorBox( K_ERR_ONSCREEN, L"---> [Achievements] Resetting achievements on startup!" );
			SteamUserStats()->ResetAllStats( true );
#endif
#endif
			//set menu state
			g_mainMenu.SetState( K_MM_STATE_MAINMENU );

			//set just started
			g_netlock.Net_QuitLobby();
			if ( g_bJustStarted )
			{
				g_bJustStarted = false;

				//analytics
				ANALYTICS_SCREENVIEW( "main_menu" );
#ifdef ENABLE_STEAM
				ANALYTICS_EVENT( "game_started_STEAM", _VERSION_CHARSTR_, "", 1 );
#endif
#ifdef ENABLE_GALAXY
				ANALYTICS_EVENT( "game_started_GOG", _VERSION_CHARSTR_, "", 1 );
#endif
			}

			//mark game as NON networked
			UTApp().m_Settings.devnet_eNetGameType = CApplicationSettings::K_NETGAME_TYPE_NO_NETWORK;
			UTApp().m_Settings.devnet_eSyncStatus = CApplicationSettings::K_NETGAME_SYNC_STOPPED;
		}
		break;

		case GAME_STATE_JOIN_COOP_LIST:
		{
			//set a state that has no update logic (just displays the background)
			g_mainMenu.SetState( K_MM_STATE_NET_LOBBY );

			//as soon as we enter we ask for the lobbies list and the state will read the lobbies a little later (on a timer job)
			g_netlock.Net_RequestLobbyList( 10 );

			__Texts().SetString( STR_LOBBIES_LIST_VAL, L"%s", __Texts().strings[ STR_PLEASE_HANG ]->sText );
			//add the window
			CCtrlLayer* lay = __GUI().ShowLayerOnce( "LAYER_ID_LOBBIES_LIST" );
			if ( lay != null )
			{
				CControl* ctrl = lay->GetControlByName( "BUT_JOIN_LOBBY" );
				if ( ctrl )
					ctrl->bDisabled = true;

				ctrl = lay->GetControlByName( "CTRL_LOBBIES_SELECTOR" );
				if ( ctrl )
				{
					ctrl->bDisabled = true;
					ctrl->paramsDict.SetVarINT32( L"nOptionsCnt", 1 );
				}

				ctrl = lay->GetControlByName( "BUT_REFRESH_LOBBIES" );
				if ( ctrl )
					ctrl->bDisabled = true;
			}
		}
		break;

		case GAME_STATE_NET_LOBBY:
		{
			GameState::substate = 0;

			//exit lobby if was in lobby
			if ( UTApp().m_Settings.devnet_eNetGameType != CApplicationSettings::K_NETGAME_TYPE_NO_NETWORK )
			{
				g_netlock.Net_QuitLobby();
			}

			//show window
#ifdef ENABLE_STEAM
			__GUI().ShowLayerOnce( "LAYER_ID_QUICK_MATCH_INVITE" );
#endif
#ifdef ENABLE_GALAXY
			__GUI().ShowLayerOnce( "LAYER_ID_QUICK_MATCH" );
#endif
			//change menu on net lobby background
			g_mainMenu.SetState( K_MM_STATE_NET_LOBBY );

			//set correct network game type
			UTApp().m_Settings.devnet_eNetGameType = ( CApplicationSettings::eNetGameTypes )0 /*param1*/; //param1 contains net match type 
			UTApp().m_Settings.devnet_eSyncStatus = CApplicationSettings::K_NETGAME_SYNC_STOPPED;
			//enter lobby
			switch ( UTApp().m_Settings.devnet_eNetGameType )
			{
				case CApplicationSettings::eNetGameTypes::K_NETGAME_TYPE_QUICK_MATCH:
				{
					g_netlock.Net_EnterLobby( false, false );
					LOG( L"[NET] Entered lobby Quick Match" );
				}
				break;
				case CApplicationSettings::eNetGameTypes::K_NETGAME_TYPE_HOST_PUBLIC:
				{
					g_netlock.Net_EnterLobby( true, false );
					LOG( L"[NET] Entered lobby Host Public" );
				}
				break;
				case CApplicationSettings::eNetGameTypes::K_NETGAME_TYPE_HOST_PRIVATE:
				{
					g_netlock.Net_EnterLobby( true, true );
					LOG( L"[NET] Entered lobby Host Private" );
				}
				break;
			}

			//analytics
			ANALYTICS_SCREENVIEW( "net_match" );
		}
		break;

		case GAME_STATE_WORKSHOP:
		{
			//save user data here, will load it when exiting the workshop
			App_SaveUserData();
			//set state
			g_mainMenu.SetState( K_MM_STATE_WORKSHOP );
		}
		break;

		case GAME_STATE_GAME_MODE_SELECTION:
		{
			//set state (and transmit arg for target game state)
			g_mainMenu.SetState( K_MM_STATE_GAME_MODE_SELECT, 0 /*param1*/ );
		}
		break;

		case GAME_STATE_CHAPTER_SELECTION:
		{
			g_mainMenu.SetState( K_MM_STATE_CHAPTER_SELECT );
		}
		break;

		case GAME_STATE_LEVEL_SELECTION:
		{
			g_mainMenu.SetState( K_MM_STATE_LEVEL_SELECT );
			//reset downloaded mod selection
			g_userData[ K_MEMID_MOD_DWNLVL_SELECTED ] = -1;
		}
		break;

		case GAME_STATE_PLAYER_SELECTION:
		{
			//release main menu sprites
			g_mainMenu.Release();
			//just to load them back in the player selection screen
			WCHAR xmlpath[ MAX_PATH ];
			FileManager::GetMediaPath( L"media/interfaces/menus.bsx", xmlpath );
			if ( FAILED( g_playerSelScr.InitSprites( xmlpath ) ) )
			{
				ErrorBox( K_ERR_CRITICAL, L"File not found:\n%s", xmlpath );
				break;
			}

			//param1: reset instanceIDs - On networked games don't reset instance ids so they appear already selected.
			int param1 = 0;
			if ( param1 != 0 )
			{
				g_playerSelScr.ResetSelection( true );
			}
			else
			{
				g_playerSelScr.ResetSelection( false );
			}
			//on networked games send local selection immediately so they sync levels
			if ( UTApp().IsGameNetworked() )
			{
				g_netlock.Net_EnterPlayerSelScreen();

				g_playerSelScr.SendSelectionByNetwork( g_netlock.Net_GetPlayerIndex() );
			}
		}
		break;
		case GAME_STATE_GAME:
		{
			// Create necessary render targets when device gets reset (created or reset)
			UINT fGameHpx = K_GAME_HEIGHT * K_RT_PIXEL_SIZE;
			UINT fGameWpx = K_GAME_WIDTH * K_RT_PIXEL_SIZE;

			// Create RTs
			__RTManager().AddRT( K_RTID_COLORDEPTHSTENCIL, fGameWpx, fGameHpx, 1, D3DFMT_A8R8G8B8, false );
			__RTManager().AddRT( K_RTID_FINAL, fGameWpx, fGameHpx, 1, D3DFMT_A8R8G8B8, false );
			__RTManager().AddRT( K_RTID_TEMP1, fGameWpx, fGameHpx, 1, D3DFMT_A8R8G8B8, false );

			// used for GI
			if ( UTApp().m_Settings.bEnableGI == true )
			{
				UINT radiance_render_extent = (UINT)K_GI_RENDER_EXTENT;
				__RTManager().AddRT( K_RTID_WORLDSCENE, radiance_render_extent, radiance_render_extent, 1, D3DFMT_A8R8G8B8, false );
				__RTManager().AddRT( K_RTID_TEMPORARY, radiance_render_extent, radiance_render_extent, 1, D3DFMT_A8R8G8B8, false );
				__RTManager().AddRT( K_RTID_STORAGE, radiance_render_extent, radiance_render_extent, 1, D3DFMT_A8R8G8B8, false );
				__RTManager().AddRT( K_RTID_STORAGE_HALF, radiance_render_extent / 2, radiance_render_extent / 2, 1, D3DFMT_A8R8G8B8, false );
				__RTManager().AddRT( K_RTID_STORAGE_HALF2, radiance_render_extent / 2, radiance_render_extent / 2, 1, D3DFMT_A8R8G8B8, false );
				__RTManager().AddRT( K_RTID_STORAGE_QUART, radiance_render_extent / 4, radiance_render_extent / 4, 1, D3DFMT_A8R8G8B8, false );
				__RTManager().AddRT( K_RTID_STORAGE_QUART2, radiance_render_extent / 4, radiance_render_extent / 4, 1, D3DFMT_A8R8G8B8, false );
				__RTManager().AddRT( K_RTID_STORAGE_EIGHTH, radiance_render_extent / 8, radiance_render_extent / 8, 1, D3DFMT_A8R8G8B8, false );
				__RTManager().AddRT( K_RTID_STORAGE_EIGHTH2, radiance_render_extent / 8, radiance_render_extent / 8, 1, D3DFMT_A8R8G8B8, false );
				__RTManager().AddRT( K_RTID_GI, radiance_render_extent, radiance_render_extent, 1, D3DFMT_A8R8G8B8, false );
			}


			//increase music counter
			g_userData[ K_MEMID_MUSIC_TRACK_COUNTER ]++;
			//save user data again
			App_SaveUserData();
			//release main menu class
			g_mainMenu.Release();
			// set the controller pointer normalization function (gets set to nullptr when not in game)
			__Controllers().SetNormalizeCoordsFunctionPtr( NormalizeIngameMouseCoords );

			//reset all scripts
			__Scripts().StopAllScripts();
			//clear global memory - nothing stays between levels
			__Scripts().ClearGlobalMemory();

			//loads the level
			if ( g_userData[ K_MEMID_MOD_DWNLVL_SELECTED ] < 0 )
			{
				// load generic textures
				// blue noise for GI:
				WCHAR texpath[MAX_PATH];
				StringCchPrintf( texpath, MAX_PATH, L"%s/levels/data/bluenoise512.png", UTApp().g_wszAppResDir );
				UTApp().g_texManager.AddTexture( texpath, D3DFMT_A8B8G8R8, D3DX_FILTER_NONE, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DX_DEFAULT, FastHash(L"BLUENOISE512"));
				// black texture for GI
				StringCchPrintf( texpath, MAX_PATH, L"%s/levels/data/black32.png", UTApp().g_wszAppResDir );
				UTApp().g_texManager.AddTexture( texpath, D3DFMT_A8B8G8R8, D3DX_FILTER_NONE, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DX_DEFAULT, FastHash( L"BLACK32" ) );
				StringCchPrintf( texpath, MAX_PATH, L"%s/levels/data/bayer8x8.png", UTApp().g_wszAppResDir );
				UTApp().g_texManager.AddTexture( texpath, D3DFMT_A8B8G8R8, D3DX_FILTER_NONE, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DX_DEFAULT, FastHash( L"BAYER8X8" ) );

				//classic levels
				///--- find chapter and level in levels.xml ---	
				WCHAR strLevelPath[ MAX_PATH_STD ] = { 0 };
				if ( g_startupCommand == GAME_STARTUP_LOAD_MAP )
				{
					std::wstring sProcessedPath = RemoveQuotationMarks( g_startupParam.text );
					//starting game with forced map
					swprintf_s( strLevelPath, MAX_PATH, L"%s", sProcessedPath.c_str() );
					//write current mission name and number
					__Texts().SetString( STR_CURRENT_MISSION_VAL, L"" );
				}
				else
				{
					int nChapterNumber = g_userData[ K_MEMID_SELECTED_CHAPTER ];
					int nLevelNumber = g_userData[ K_MEMID_SELECTED_LEVEL ];
					/*
					bool bLevelFound = UTGetChaptersList().GetMissionFilename( nChapterNumber, nLevelNumber, strLevelPath, MAX_PATH );
					if ( !bLevelFound )
					{
						ChangeTo_Transition( GAME_STATE_LEVEL_SELECTION, TRANSITION_SIMPLE );
						break;
					}
					//write current mission name and number
					int nStrIdxLevelName = UTGetChaptersList().m_arrChapters[ nChapterNumber ]->arrLevelNameStrIdx[ nLevelNumber ];
					__Texts().SetString( STR_CURRENT_MISSION_VAL, L"%d.%d %s", nChapterNumber + 1, nLevelNumber + 1, __Texts().strings[ nStrIdxLevelName ]->sText );
					*/
				}

				WCHAR tmppath[ MAX_PATH_STD ];
				swprintf_s( tmppath, MAX_PATH_STD, L"media/levels/missions/01_01_slow_starters_dmc.area");
				FileManager::GetMediaPath( tmppath, strLevelPath );
				if ( OP_FAILED( __Sim().LoadLevel_Static( strLevelPath ) ) )
				{
					ErrorBox( K_ERR_WARNING, L"Could not load static level [%s]!", strLevelPath );
					ChangeTo_Transition( GAME_STATE_LEVEL_SELECTION, TRANSITION_SIMPLE );
					break;
				}

				// LOAD RANDOM STORY
				/*
				if ( OP_FAILED( __Sim().LoadLevel_GenerateFromStory() ) )
				{
					ErrorBox( K_ERR_WARNING, L"Could not load level [%s]!", strLevelPath );
					ChangeTo_Transition( GAME_STATE_LEVEL_SELECTION, TRANSITION_SIMPLE );
					break;
				}
				*/
			}
			else
			{
				//modded custom levels
#ifdef ENABLE_STEAM_WORKSHOP
				CModsManager::CModDescriptor *mod = __Mods().GetModDescByIndex( g_userData[ K_MEMID_MOD_DWNLVL_SELECTED ] );
				if ( mod == nullptr )
				{
					ErrorBox( K_ERR_WARNING, L"Couldn't find custom level!" );
					ChangeTo_Transition( GAME_STATE_LEVEL_SELECTION, TRANSITION_SIMPLE );
					break;
				}

				WCHAR wcsLevelPath[ MAX_PATH ] = { 0 };
				mod->GetFullPathToAffectedFile( 0, wcsLevelPath, MAX_PATH );
				//write current mission name and number
				__Texts().SetString( STR_CURRENT_MISSION_VAL, L"%s", mod->shName.text );

				if ( OP_FAILED( __Sim().LoadLevel_GenerateFromStory( /* wcsLevelPath */ ) ) )
				{
					ErrorBox( K_ERR_WARNING, L"Could not load downloaded level [%s]!", wcsLevelPath );
					ChangeTo_Transition( GAME_STATE_LEVEL_SELECTION, TRANSITION_SIMPLE );
					break;
				}
#endif // ENABLE_STEAM_WORKSHOP
			}

			//start menu music
			SND_STOP_GROUP( "music", false, true );
			//SND_PLAY_ONCE( SNDIDX_THEME_MENU1, DSBPLAY_LOOPING );
			//analytics
			ANALYTICS_SCREENVIEW( "INGAME" );
		}
		break;

#ifdef K_CONTROLS_EDITOR
		case GAME_STATE_CONTROLSED:
		{
			__GUI().RemoveAllLayers( true );
			g_ControlsEditor.Launch();
			__ImGui().SetGlobalEnabled( true );
		}
		break;
#endif
	}

	// INFO: announce state changed
	CEvent *nevent = new CEvent( CEventTypes::evtT_INFO, CEventCommands::evtC_GAMESTATE_CHANGE);
	nevent->AddNamedArgUINT32( L"newGameState", state);
	nevent->AddNamedArgUINT32( L"oldGameState", oldGameState);
	__Events().QueueEvent( nevent );
}


void GameState::ChangeTo_Transition( EGameState newState, ETransitionType transitionType, CVariantMap * args /*= nullptr */ )
{
	//#TODO: save args and feed them to ChangeGameState:
	//vcArgs = *args;
	nTransitionStep = 0;
	fTransitionPercent = 0.0f;
	bDuringTransition = true;
	eNextState = newState;

	nTransitionType = transitionType;
}


void GameState::UpdateTransition( float dTime )
{
	if ( !bDuringTransition ) return;

	switch ( nTransitionType )
	{
		case TRANSITION_SIMPLE:
		{
			switch ( nTransitionStep )
			{
				case 0: //show full screen black poly
				{
					if ( fTransitionPercent >= 1.0f )
					{
						fTransitionPercent = 0.0f;
						nTransitionStep = 1;
						//full black, change state now
						GameState::SetState( eNextState );
					}

					fTransitionPercent += dTime * 6.0f;
					CLAMP( fTransitionPercent, 0.0f, 1.0f );
				}
				break;
				case 1:
				{
					fTransitionPercent += dTime * 6.0f;
					if ( fTransitionPercent >= 1.0f )
					{
						fTransitionPercent = 0.0f;
						bDuringTransition = false;
					}
				}
				break;
			}
		}
		break;
	}

}

void GameState::PaintTransition( float dTime, float fTimeline, PDEVICE pDevice )
{
	if ( !bDuringTransition ) return;

	switch ( nTransitionType )
	{
		//simple black transition
		case TRANSITION_SIMPLE:
		{
			//reset transforms
			pDevice->SetTransform( D3DTS_WORLD, &g_matIdentity );
			pDevice->SetTransform( D3DTS_VIEW, &g_matIdentity );

			RectLTRB rect( UTApp().g_rectRender.x, UTApp().g_rectRender.y, UTApp().g_rectRender.Right(), UTApp().g_rectRender.Bottom() );

			switch ( nTransitionStep )
			{
				case 0:
				{
					pDevice->SetTexture( 0, NULL ); 
					// keep black for half the transition
					float fAlpha = LIMIT( 1.5f * fTransitionPercent, 0.0f, 1.0f );
					UT3D::DrawRectUP_TL1T( pDevice, rect, RectLTRB( 0.0f, 0.0f, 1.0f, 1.0f ), DW_COLOR_XXXA( fAlpha ) );
					//#TODO: write "loading"
					if ( ( __GUI().m_sprCol.IsLoaded() ) && ( fAlpha >= 0.95f ) )
					{
					}
				}
				break;
				case 1:
				{
					// black rect
					pDevice->SetTexture( 0, NULL );
					float fAlpha = LIMIT( ( 1.5f - 1.5f * fTransitionPercent ), 0.0f, 1.0f );
					UT3D::DrawRectUP_TL1T( pDevice, rect, RectLTRB( 0.0f, 0.0f, 1.0f, 1.0f ), DW_COLOR_XXXA( fAlpha ) );
				}
				break;
			}
		}
		break;

	}
}

void GameState::ExitState( EGameState exitState, EGameState newState )
{
	switch ( exitState )
	{
		case GAME_STATE_PRELOAD:
		{
		}
		break;
		case GAME_STATE_DEVELOPER:
		{
			UTApp().App_ExitState_Developer();
		}
		break;
		case GAME_STATE_LOADING:
		{
			UTApp().App_ExitState_Loading();
			g_bForceOneUpdatePerFrame = false;
		}
		break;

		case GAME_STATE_PLAYER_SELECTION:
		{
			g_playerSelScr.ReleaseSprites();
			///--- load main menu ---
			WCHAR xmlpath[MAX_PATH], xmlpath2[MAX_PATH];
			FileManager::GetMediaPath( L"media/interfaces/menus.bsx", xmlpath );
			FileManager::GetMediaPath( L"media/interfaces/menus0.bsx", xmlpath2 );
			if ( FAILED( g_mainMenu.LoadSprites( xmlpath, xmlpath2 ) ) )
			{
				ErrorBox( K_ERR_CRITICAL, L"Main Menu file not found:\n%s", xmlpath );
			}

			__GUI().RemoveAllLayers( true );
		}
		break;
		case GAME_STATE_GAME:
		{

#ifdef ENABLE_CHAT_WINDOW
			//cancel current input if exited
			g_ChatWnd.CancelInput();
			g_ChatWnd.Clear();
#endif

			//push global scores to leaderboard when returning from the game
#ifdef ENABLE_LEADERBOARDS
			//upload multiplayer score
			if ( g_userData[K_MEMID_TOTAL_SCORE_COOP] > 0 )
				__Leaderboards().QueueJob( K_JOB_UPLOAD_SCORE, K_GAME_STR_LEADERBOARDS_GLOBAL_COOP, g_userData[K_MEMID_TOTAL_SCORE_COOP] );
			//upload single player score so that current leaderboard remains the single player one
			if ( g_userData[K_MEMID_TOTAL_SCORE_SOLO] > 0 )
				__Leaderboards().QueueJob( K_JOB_UPLOAD_SCORE, K_GAME_STR_LEADERBOARDS_GLOBAL_SP, g_userData[K_MEMID_TOTAL_SCORE_SOLO] );
#endif
			//must be called here to reset controller flags
			__Controllers().ResetAllControllersKeypresses();
			//stop all sounds
			__Audio().StopGroup( "sounds", false, true );
			__Audio().StopGroup( "ingame", false, true );

			SND_SET_GROUP_FREQUENCY( "ingame", 1.0f, false );
			UTApp().g_texManager.Release();
			// release level resources
			__Sim().Release();
			// level was unloaded, immediately set the controller pointer to null
			__Controllers().SetNormalizeCoordsFunctionPtr( nullptr );

			__GUI().RemoveAllLayers( true );

			__Audio().StopGroup( "music", false, true );
			if ( newState != GAME_STATE_GAME )
			{
				//SND_PLAY_ONCE( SNDIDX_THEME_MENU1, DSBPLAY_LOOPING );
			}

			//set volumes
			SND_SET_GROUP_VOLUME( "sounds", UTApp().m_Settings.fSoundsVolume, false );
			SND_SET_GROUP_VOLUME( "ingame", UTApp().m_Settings.fSoundsVolume, false );
			SND_SET_GROUP_VOLUME( "music", UTApp().m_Settings.fMusicVolume, false );

			///--- load main menu ---
			WCHAR xmlpath[MAX_PATH], xmlpath2[MAX_PATH];
			FileManager::GetMediaPath( L"media/interfaces/menus.bsx", xmlpath );
			FileManager::GetMediaPath( L"media/interfaces/menus0.bsx", xmlpath2 );
			if ( FAILED( g_mainMenu.LoadSprites( xmlpath, xmlpath2 ) ) )
			{
				ErrorBox( K_ERR_CRITICAL, L"Main Menu file not found:\n%s", xmlpath );
			}

			//-- release RTs
			__RTManager().ReleaseAll();
		}
		break;

		case GAME_STATE_WORKSHOP:
		{
			__Audio().StopGroup( "sounds", false, true );
			__GUI().RemoveAllLayers( true );
			//release used textures here:
			UTApp().g_texManager.Release();
			//make sure we reload everything that can be modded
			App_ReloadContentChanges();
			///compute mods CRC
			UINT32 unModsCRC = App_GetActiveModsCRC();

			UTApp().m_Settings.dev_unCurrentModsCRC = unModsCRC;
			LOG( L"--> CRC_BASE [%08x] CRC_MODS [%08x] <--", UTApp().m_Settings.dev_unCurrentCRC, UTApp().m_Settings.dev_unCurrentModsCRC );
			//when returning from the mods screen reload the main menu in case it changed
			g_mainMenu.Release();

			WCHAR xmlpath[MAX_PATH];
			WCHAR xmlpath2[MAX_PATH];
			FileManager::GetMediaPath( L"media/interfaces/menus.bsx", xmlpath );
			FileManager::GetMediaPath( L"media/interfaces/menus0.bsx", xmlpath2 );
			if ( FAILED( g_mainMenu.LoadSprites( xmlpath, xmlpath2 ) ) )
			{
				ErrorBox( K_ERR_CRITICAL, L"Main Menu file not found:\n%s", xmlpath );
			}
		}
		break;

		case GAME_STATE_JOIN_COOP_LIST:
		{
			__Audio().StopGroup( "sounds", false, true );
			__GUI().RemoveAllLayers( true );
		}
		break;

		case GAME_STATE_NET_LOBBY:
		case GAME_STATE_GAME_MODE_SELECTION:
		case GAME_STATE_CHAPTER_SELECTION:
		case GAME_STATE_LEVEL_SELECTION:
		case GAME_STATE_MAINMENU:
		{
			__Audio().StopGroup( "sounds", false, true );
			__GUI().RemoveAllLayers( true );
			//release used textures here:
			UTApp().g_texManager.Release();
		}
		break;
#ifdef K_CONTROLS_EDITOR
		case GAME_STATE_CONTROLSED:
		{
			__ImGui().SetGlobalEnabled( false );
			g_ControlsEditor.Close();
		}
		break;
#endif
	}

}

