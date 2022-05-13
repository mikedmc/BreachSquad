#include "dxstdafx.h"
///**************************************************************************************
/// game update states
///**************************************************************************************


///----- GAME_STATE_LOADING -----
#define K_CS_LOADING_WEAPONS 13
#define K_CS_LOADING_WEAPONS_PER_COL 10
int nLoadingFrame = 0;

void CApplication::App_EnterState_Loading()
{
	// Create necessary render targets when device gets reset (created or reset)
	UINT fGameHpx = K_GAME_HEIGHT * K_RT_PIXEL_SIZE;
	UINT fGameWpx = K_GAME_WIDTH * K_RT_PIXEL_SIZE;
	// Create RTs
	__RTManager().AddRT( K_RTID_COLORDEPTHSTENCIL, fGameWpx, fGameHpx, 1, D3DFMT_A8R8G8B8, false );
	__RTManager().AddRT( K_RTID_TEMP1, fGameWpx, fGameHpx, 1, D3DFMT_A8R8G8B8, false );
	__RTManager().AddRT( K_RTID_FINAL, fGameWpx, fGameHpx, 1, D3DFMT_A8R8G8B8, false );


	GameState::substate = 0;
	GameState::fTimer = 0.0f;
	//make sure we release everything
	g_texManager.Release();
	WCHAR wcsPath[MAX_PATH];
	FileManager::GetMediaPath(L"media/interfaces/loading.png", wcsPath, true);
	g_texManager.AddTexture(wcsPath, D3DFMT_A8R8G8B8, D3DX_FILTER_NONE, D3DX_FILTER_NONE);
	//decide which weapon to show from available 13
	nLoadingFrame = randint(K_CS_LOADING_WEAPONS);
}

void CApplication::App_UpdateState_Loading(LPDIRECT3DDEVICE9 pDevice, double fTimeline, float dTime)
{
	if (GameState::isTransitioning())
		return;

	switch ( GameState::substate )
	{
		//crc check and splash load
		case 0:
		{
			//next state
			GameState::substate++;
			GameState::fTimer = 0.0f;
		}
		break;
		//animating background
		case 1:
		{
			GameState::substate++;
		}
		break;
		//misc
		case 2:
		{
#ifdef ENABLE_STEAM_WORKSHOP
			///--- mods subscriptions checked on loading ---
			//load mods before anything else !!
			Workshop_CheckSubscriptions();
			//reload strings too
			App_LocaLoadStrings();
			//load modded chapters again after modding
			WCHAR wcsPath[MAX_PATH];
			FileManager::GetMediaPath(L"media/levels/missions/missions.xml", wcsPath);
			UTGetChaptersList().LoadChapters(wcsPath);
			//initialize vertical mode after modding
			//g_verticalMode.Init(&g_level, L"media/levels/mod_prefabs/infinite_tower.xml");
			//compute mods CRC
			UINT32 unModsCRC = App_GetActiveModsCRC();
			//add vertical mode CRC
			//unModsCRC += g_verticalMode.GetFilesCRC(false);
			UTApp().m_Settings.dev_unCurrentModsCRC = unModsCRC;
			LOG(L"--> CRC_BASE [%08x] CRC_MODS [%08x] <--", UTApp().m_Settings.dev_unCurrentCRC, UTApp().m_Settings.dev_unCurrentModsCRC);
#endif
			//chapter limiting (last chapter is the modding chapter)
			if ((g_userData[K_MEMID_SELECTED_CHAPTER] < 0) || (g_userData[K_MEMID_SELECTED_CHAPTER] > UTGetChaptersList().GetChaptersCnt()))
			{
				g_userData[K_MEMID_SELECTED_CHAPTER] = 0;
				g_userData[K_MEMID_SELECTED_LEVEL] = 0;
			}
			//parse all levels again for presence and level type if we want to
			App_ParseAllLevelsForData(false);
			App_UpdateLevelStats();
			///--- player selection screen items ---
			g_playerSelScr.LoadItems();

			if (UTApp().m_Settings.dev_unCurrentCRC != K_GAME_CRC)
			{
				char strcrc[MAX_PATH];
				StringCchPrintfA(strcrc, MAX_PATH, "CRC%08x", UTApp().m_Settings.dev_unCurrentCRC);
				ANALYTICS_EVENT("loading_modded", _VERSION_CHARSTR_, strcrc, 1);
			}
			else
			{
				char strcrc[MAX_PATH];
				StringCchPrintfA(strcrc, MAX_PATH, "CRC%08x", UTApp().m_Settings.dev_unCurrentCRC);
				ANALYTICS_EVENT("loading_vanilla", _VERSION_CHARSTR_, strcrc, 1);
			}

			//language
			CHAR langtxt[MAX_PATH];
			wcstombs(langtxt, g_Language.shLangAlias.text, MAX_PATH);
			ANALYTICS_EVENT("language", langtxt, "", 0);

			GameState::substate++;
		}
		break;
		//load fonts
		case 3:
		{
			GameState::substate++;

			///--- FONTS ---
			//load fonts based on selected language
			if (OP_FAILED(App_LocaLoadFonts(g_Language.bUseTTFonts)))
			{
				ErrorBox(K_ERR_CRITICAL, L"[ERROR] Error loading language fonts!");
				return;
			}

			// strings are already loaded
			/*
			sFreeTypeFontStyle fstyle;
			fstyle.strTexturePath = "media/fonts/rust64.png";
			fstyle.fShadowAlpha = 0.6f;
			fstyle.shadowOffsetX = 2;
			fstyle.shadowOffsetY = 4;
			fstyle.dwOutlineColor = 0x88000000;
			g_font1.CreateAtlas(pDevice, "media/fonts/inky_thin_pixels.ttf", 24, UTLang().alphabet, &fstyle);
			g_font1.SetStyle(-1, 2, 8);
			*/

			///--- CONTROLS ---
			UTGetGUI().Init();

			WCHAR xmlpath[MAX_PATH];
			FileManager::GetMediaPath(L"media/interfaces/interfaces.xml", xmlpath);
			if (OP_FAILED(UTGetGUI().LoadControlsXML(xmlpath)))
			{
				ErrorBox(K_ERR_CRITICAL, L"Couldn't load interfaces sprites!");
				return;
			}

			///--- level areas inventory ---
			FileManager::GetMediaPath(L"media/levels/areas/areas_list.xml", xmlpath);
			if (OP_FAILED(UTGetAreasInv().LoadAreasSpecs(xmlpath)))
				return;
		}
		break;
		case 4:
		{
			GameState::substate++;

			HRESULT hr = S_OK;
			WCHAR xmlpath[MAX_PATH];
			///--- SOUNDS ---
			FileManager::GetMediaPath(L"media/sounds/sounds.xml", xmlpath);
			if (FAILED(hr = UTGetSoundManager().LoadSoundsXML(xmlpath)))
			{
				ErrorBox(K_ERR_WARNING, L"Failed INITSOUND->LoadSoundsXML()\n");
			}
			//set volumes
			SND_SET_GROUP_VOLUME("sounds", UTApp().m_Settings.fSoundsVolume, false);
			SND_SET_GROUP_VOLUME("ingame", UTApp().m_Settings.fSoundsVolume, false);
			SND_SET_GROUP_VOLUME("music", UTApp().m_Settings.fMusicVolume, false);
			SND_SET_GROUP_FREQUENCY("ingame", 1.0f, false);
		}
		break;
		//other technical stuff
		case 5:
		{
			GameState::substate++;

			// load editor sprites
			g_editor.Init();

			WCHAR xmlpath[MAX_PATH];
			FileManager::GetMediaPath(L"media/scripts.xml", xmlpath);
			UTGetScriptManager().AddScripts(xmlpath);
			///--- SHADERS ---
			// no modding support on shaders!
			WCHAR mszPath[MAX_PATH];
			StringCchPrintf(mszPath, MAX_PATH, L"%s/shaders/shaders.xml", UTApp().g_wszAppResDir);
			if (OP_FAILED(UTGetShaderManager().AddShadersFromXML(mszPath)))
			{
				ErrorBox(K_ERR_CRITICAL, L"Couldn't load shaders XML: %s", mszPath);
				return;
			}
			///--- particles ---
			FileManager::GetMediaPath(L"media/particles/particles.bsx", xmlpath);
			if (OP_FAILED(__Particles().Init(xmlpath, 5000)))
			{
				ErrorBox(K_ERR_CRITICAL, L"Couldn't load particles!");
				return;
			}
			///--- load main menu ---
			WCHAR xmlpath2[MAX_PATH];
			FileManager::GetMediaPath(L"media/interfaces/menus.bsx", xmlpath);
			FileManager::GetMediaPath(L"media/interfaces/menus0.bsx", xmlpath2);
			if (FAILED(g_mainMenu.LoadSprites(xmlpath, xmlpath2)))
			{
				ErrorBox(K_ERR_CRITICAL, L"Main Menu file not found:\n%s", xmlpath);
			}

			V_OP_RET_VOID( __Game().gMenus.Init() );
		}
		break;
		//wait keypress
		case 6:
		{
			GameState::substate++;
			Sleep(100);
		}
		break;
		//change gamestate
		case 7:
		{
			GameState::substate++;

			//start with specified map if requested by editor
			if (g_startupCommand == GAME_STARTUP_LOAD_MAP)
			{
				//when starting the game it will check for the startup command and load specified level 
				CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
				nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_PLAYER_SELECTION);
				nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
				UTGetEventManager().QueueEvent(nevent);
				break;
			}

			//if we've been invited to a lobby from the Steam commandline then go directly into the lobby
#if defined(ENABLE_STEAM) || defined(ENABLE_GALAXY)
			//command: GAME_STARTUP_JOIN_LOBBY
			if (g_netlock.m_ullCurLobbyID != 0)
			{
				LOG(L"Game:: Accepting Join Lobby invitation LobbyID: %llu", g_netlock.m_ullCurLobbyID);
				//change state
				CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
				nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_NET_LOBBY);
				nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
				//set joining state
				nevent->AddNamedArgINT32(L"arg1", (int)CApplicationSettings::K_NETGAME_TYPE_QUICK_MATCH);
				UTGetEventManager().QueueEvent(nevent);
			}
			else //not invited, go to splash
#endif
			{
				CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
				nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
				nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
				UTGetEventManager().QueueEvent(nevent);
			}
			//from now on we can pause the game
			g_bCanPause = true;
		}
		break;
		//wait for transition
		default:
		{
		}
		break;
	}
}

void CApplication::App_PaintState_Loading(LPDIRECT3DDEVICE9 pDevice, ID3DXSprite* pSprite, double fTimeline)
{
	//setam ecranul standard de 240h inaltime
	CCameraTransform::SetActiveCamera(pDevice, &UTApp().g_cam360hScreen);
	App_SetWorldTransform(pDevice, &g_matIdentity);

	RectXYWH scrrect = UTApp().g_cam360hScreen.GetCamWorldAABB();
	RectXYWH worldrect = UTApp().g_rect360hWorld;

	//fonts loaded so write "loading" 
	if ((GameState::substate > 3) && (GameState::substate < 6))
	{
		RectXYWHi rct(scrrect.x + 25, scrrect.CenterY() - 10, scrrect.w - 50, 15);
		g_font6ns1->DrawString(STR_LOADING, rct, FONTFLAG_ANCHOR_TOPCENTER, 0xff186582);
	}

	if (GameState::substate > 3)
	{
		//write title window text
		if (UTApp().m_Settings.dev_unCurrentCRC != K_GAME_CRC)
		{
			g_font10bs1->DrawString(STR_CHANGE_DETECTED, scrrect.CenterX(), 15.0f, FONTFLAG_ANCHOR_TOPCENTER, 0xff963500);
			g_font6ns1->DrawString(STR_CHANGE_DETECTED_WARNING, scrrect.CenterX(), 27.0f, FONTFLAG_ANCHOR_TOPCENTER, K_COLOR_DEFAULT_TEXT);
		}
	}
	//progress
	if ((GameState::substate >= 0) && (GameState::substate < 7))
	{
		int nlX = 48 * (nLoadingFrame / K_CS_LOADING_WEAPONS_PER_COL) * 2;
		int nlY = 24 * (nLoadingFrame % K_CS_LOADING_WEAPONS_PER_COL);

		RECT rctSrcEmpty;
		SetRect(&rctSrcEmpty, nlX + 48, nlY, nlX + 48 + 48, nlY + 24);
		RECT rctSrcFull;
		SetRect(&rctSrcFull, nlX, nlY, nlX + 1 + (int)ceil(48 * ((float)GameState::substate / 7.0f)), nlY + 24);
		
		CTexNode* pTN = g_texManager.GetTextureByIndex( 0 );
		if ( pTN && pTN->isLoaded() )
		{
			pSprite->Draw( pTN->pTexture, &rctSrcEmpty, NULL, &D3DXVECTOR3( scrrect.CenterX() - 24.0f, scrrect.CenterY(), 0.0f ), 0xffffffff );
			pSprite->Draw( pTN->pTexture, &rctSrcFull, NULL, &D3DXVECTOR3( scrrect.CenterX() - 24.0f, scrrect.CenterY(), 0.0f ), 0xffffffff );
		}
	}
}

void CApplication::App_ExitState_Loading()
{
	g_texManager.Release();
	g_sprMgrGlobal.Release();
	//push global scores to leaderboard and request single player leaderboard
#ifdef ENABLE_LEADERBOARDS
	//start initialize job
	UTGetLeaderboards().QueueJob(K_JOB_INITIALIZE, K_GAME_STR_LEADERBOARDS_GLOBAL_SP, 0);
	//reset strings for scores
	__Texts().SetString(STR_LEADERBOARDS_NAMES_VAL, L"...");
	__Texts().SetString(STR_LEADERBOARDS_SCORES_VAL, L"...");
	//reset old scores
	UTGetLeaderboards().ResetScoresList();
	//upload multiplayer score
	if (g_userData[K_MEMID_TOTAL_SCORE_COOP] > 0)
		UTGetLeaderboards().QueueJob(K_JOB_UPLOAD_SCORE, K_GAME_STR_LEADERBOARDS_GLOBAL_COOP, g_userData[K_MEMID_TOTAL_SCORE_COOP]);
	//upload single player score so that current leaderboard remains the single player one
	if (g_userData[K_MEMID_TOTAL_SCORE_SOLO] > 0)
		UTGetLeaderboards().QueueJob(K_JOB_UPLOAD_SCORE, K_GAME_STR_LEADERBOARDS_GLOBAL_SP, g_userData[K_MEMID_TOTAL_SCORE_SOLO]);
	//request single player scores
	UTGetLeaderboards().QueueJob(K_JOB_GET_SCORES_GLOBAL, K_GAME_STR_LEADERBOARDS_GLOBAL_SP, 1);
#endif

}



///----- GAME_STATE_DEVELOPER -----

void CApplication::App_EnterState_Developer()
{
	GameState::substate = 0;
	GameState::fTimer = K_GAME_SPLASH_SHOW_TIMER;
	//make sure we release everything
	UTApp().g_texManager.Release();
	//load the texture
	WCHAR texpath[MAX_PATH];
	StringCchPrintf(texpath, MAX_PATH, L"%s/interfaces/pixelshard.png", UTApp().g_wszAppResDir);
	UTApp().g_texManager.AddTexture(texpath, D3DFMT_A8B8G8R8, D3DX_FILTER_NONE, D3DX_FILTER_NONE);
}

void CApplication::App_UpdateState_Developer(LPDIRECT3DDEVICE9 pDevice, double fTimeline, float dTime)
{
	if ((!GameState::isTransitioning()) && (GameState::fTimer > 0.5f))
	{
		if ((g_texManager.GetTextureByIndex(0) == null) || (g_mouse.Lbut != K_MOUSE_BUTT_NOTPRESSED) || (g_mouse.Rbut != K_MOUSE_BUTT_NOTPRESSED) || (UTGetCtrlrMgr().KeyPressed()))
		{
			GameState::fTimer = 0.5f;
		}
	}

	if (GameState::substate == 0)
	{
		GameState::fTimer -= dTime;
		if ( GameState::fTimer <= 0.0f)
		{
			GameState::substate = 1;
			//change state to loading
			CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
			nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_LOADING);
			nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
			UTGetEventManager().QueueEvent(nevent);
		}
	}
}

void CApplication::App_PaintState_Developer(LPDIRECT3DDEVICE9 pDevice, ID3DXSprite* pSprite, double fTimeline)
{
	//setam ecranul standard de 240h inaltime
	CCameraTransform::SetActiveCamera(pDevice, &UTApp().g_cam360hScreen);
	App_SetWorldTransform(pDevice, &g_matIdentity);

	RectXYWH scrrect = UTApp().g_cam360hScreen.GetCamWorldAABB();
	RectXYWH worldrect = UTApp().g_rect360hWorld;
	RECT src;
	//logo
	CTexNode* pTN = g_texManager.GetTextureByIndex( 0 );
	if (pTN != nullptr)
	{
		//7x3 frames 71x86px
		int nAnimFrames = 21; 
		SizeWH recsz(71, 86);
		int nFrame = (int)floor(nAnimFrames * (1.0f - (GameState::fTimer / K_GAME_SPLASH_SHOW_TIMER)));
		int nx = nFrame % 7, ny = nFrame / 7;

		SetRect(&src, nx * recsz.w, ny * recsz.h, (nx + 1) * recsz.w, (ny + 1) * recsz.h);
		pSprite->Draw(pTN->pTexture, &src, &D3DXVECTOR3((src.right - src.left) / 2.0f, (src.bottom - src.top) / 2.0f, 0.0f), &D3DXVECTOR3(scrrect.CenterX(), scrrect.CenterY(), 0.0f), 0xffffffff);
	}
}

void CApplication::App_ExitState_Developer()
{
	g_texManager.Release();
}
