#include "dxstdafx.h"


CGame::CGame()
{
	fTimeline = 0.0f;
	// starts maxed out so it immediately executes an update when starting
	fFixedStepTimer = K_GAME_FIXED_TIMESTEP_DTIME;
}

CGame::~CGame()
{
	Release();
}

void CGame::Update( float dTime, bool bSyncUpdate, int nUpdateFrame )
{
	float fElapsedTime = dTime;
	fTimeline += dTime;


	switch ( GameState::state )
	{
		case GAME_STATE_PRELOAD:
		{
		}
		break;
		case GAME_STATE_DEVELOPER:
		{
			UTApp().App_UpdateState_Developer( m_pDevice, fTimeline, fElapsedTime );
		}
		break;
		case GAME_STATE_LOADING:
		{
			UTApp().App_UpdateState_Loading( m_pDevice, fTimeline, fElapsedTime );
		}
		break;

		//utility mod uploading to Steam
		case GAME_STATE_UPLOAD_MOD:
		{
#ifdef ENABLE_STEAM_WORKSHOP
			switch ( GameState::substate )
			{
				case 0: //a few settings and checks
				{
					UTApp().m_Settings.dev_bDevMode = true;

					LOG( L"\nMod Upload/Update Started..." );
					GameState::substate++;
				}
				break;
				case 1:
				{
					LOG( L"[Workshop] Trying to update mod..." );
					//try update
					if ( true == Workshop_UpdatePublished( g_startupParam.text ) )
					{
						GameState::substate = 10; //exit
						LOG( L"Mod UPDATED successfully!" );
						MessageBox( null, L"Mod UPDATED successfully!", L"Info", MB_OK );
					}
					else
					{
						LOG( L"Mod not found! Uploading as new mod." );
						GameState::substate = 2;
					}
				}
				break;
				case 2:
				{
					LOG( L"--- PUBLISHING NEW MOD ---" );
					//try publish
					if ( true == Workshop_Publish( g_startupParam.text ) )
					{
						LOG( L"Mod UPLOADED successfully as new mod!" );
						MessageBox( null, L"Mod UPLOADED successfully as new mod!", L"Info", MB_OK );
					}
					else
					{
						LOG( L"Mod upload FAILED! See error.log for details!" );
						MessageBox( null, L"Mod upload FAILED! See error.log for details!", L"Error", MB_OK | MB_ICONERROR );
					}

					GameState::substate++;
				}
				break;

				//exit game at the end
				default:
				{
					PostQuitMessage( 0 );
				}
				break;
			}
			break;
#endif
		}
		break;

		case GAME_STATE_PLAYER_SELECTION:
		{
			if ( ( !GameState::isTransitioning() ) && ( !UTGetGUI().bIsBlocking ) )
				g_playerSelScr.Update( fElapsedTime );
		}
		break;

		case GAME_STATE_JOIN_COOP_LIST:
		{
			//update list on timer
			if ( g_timers.Tick( 2000 ) )
			{
				CCtrlLayer* lay = UTGetGUI().GetTopmostInputLayer();
				//disable the refresh button if still working
				if ( lay != null )
				{
					CControl* ctrl = lay->GetControlByName( "BUT_REFRESH_LOBBIES" );
					if ( ctrl )
					{
						ctrl->bDisabled = false;
						if ( g_pNetwork->IsRequestingLobby() )
							ctrl->bDisabled = true;
					}
				}

				int nLobbiesCnt = g_pNetwork->GetLobbyListEntriesCount();
				if ( nLobbiesCnt == 0 )
				{
					if ( !g_pNetwork->IsRequestingLobby() )
						UTLang().SetString( STR_LOBBIES_LIST_VAL, L"%s", UTLang().strings[ STR_NO_LOBBIES ]->sText );

					//disable controls (list, join)
					if ( lay != null )
					{
						CControl* ctrl = lay->GetControlByName( "BUT_JOIN_LOBBY" );
						if ( ctrl )
							ctrl->bDisabled = true;
						ctrl = lay->GetControlByName( "CTRL_LOBBIES_SELECTOR" );
						if ( ctrl )
						{
							ctrl->bDisabled = true;
							ctrl->paramsDict.SetNamedVarINT32( L"nOptionsCnt", 1 );
						}
					}
				}
				else
				{
					WCHAR	strLobbiesList[ 2048 ] = { 0 };
					for ( int kk = 0; kk < nLobbiesCnt; kk++ )
					{
						CStringDesc sdName;
						uint64_t iLobbyID = 0;
						char strLobbyName[ 250 ];

						g_pNetwork->GetLobbyListEntry( kk, iLobbyID, strLobbyName );
						UTLang().SetStringDescUTF8( &sdName, strLobbyName );

						StringCchCat( strLobbiesList, 2048, sdName.sText );
						if ( kk < nLobbiesCnt - 1 )
							StringCchCat( strLobbiesList, 2048, L"\n" );
					}

					UTLang().SetString( STR_LOBBIES_LIST_VAL, strLobbiesList );

					//enable controls (list, join)
					if ( lay != null )
					{
						CControl* ctrl = lay->GetControlByName( "BUT_JOIN_LOBBY" );
						if ( ctrl )
							ctrl->bDisabled = false;
						ctrl = lay->GetControlByName( "CTRL_LOBBIES_SELECTOR" );
						if ( ctrl )
						{
							ctrl->bDisabled = false;
							ctrl->paramsDict.SetNamedVarINT32( L"nOptionsCnt", nLobbiesCnt );
						}
					}
				}
			}
		}
		break;

		case GAME_STATE_WORKSHOP:
		case GAME_STATE_GAME_MODE_SELECTION:
		case GAME_STATE_LEVEL_SELECTION:
		case GAME_STATE_CHAPTER_SELECTION:
		{
			if ( ( !GameState::isTransitioning() ) && ( !UTGetGUI().bIsBlocking ) )
				g_mainMenu.Update( fElapsedTime );
		}
		break;

		case GAME_STATE_NET_LOBBY:
		{
			if ( GameState::isTransitioning() )
				break;
			//update background
			if ( !UTGetGUI().bIsBlocking )
				g_mainMenu.Update( fElapsedTime );

			//update lobby
			g_netlock.Net_UpdateLobby( fElapsedTime );
		}
		break;

		case GAME_STATE_MAINMENU:
		{
			//offer to reset the user data
			if ( g_userData[ K_MEMID_OFFER_RESET_USER_DATA ] != 0 )
			{
				UTGetGUI().ShowLayerOnce( "LAYER_ID_RESET_PROGRESS_EA" );
				g_userData[ K_MEMID_OFFER_RESET_USER_DATA ] = 0;
			}

			if ( ( !GameState::isTransitioning() ) && ( !UTGetGUI().bIsBlocking ) )
				g_mainMenu.Update( fElapsedTime );

			//always check to see if menu exists
#ifdef ENABLE_STEAM_WORKSHOP
			if ( UTGetGUI().GetLayerByName( "LAYER_ID_MAINMENU" ) == null )
			{
				UTGetGUI().ShowLayerOnce( "LAYER_ID_MAINMENU" );
			}
#else
			if ( UTGetGUI().GetLayerByName( "LAYER_ID_MAINMENU_NOWORKSHOP" ) == null )
			{
				UTGetGUI().ShowLayerOnce( "LAYER_ID_MAINMENU_NOWORKSHOP" );
			}
#endif

			for ( UINT kk = 0; kk < UTGetCtrlrMgr().m_arrControllers.size(); kk++ )
			{
				if ( UTGetCtrlrMgr().m_arrControllers[ kk ]->sCommands.keyState[ K_CM_COMMAND_BACK ] == K_CM_BUTSTATE_JUSTPRESSED )
				{
					CCtrlLayer* layer = UTGetGUI().GetLayerByName( "LAYER_ID_QUITGAME" );
					if ( ( layer == null ) && ( !UTGetGUI().bIsBlocking ) )
					{
						SND_PLAY( SNDIDX_CLICK );
						UTGetGUI().ShowLayerOnce( "LAYER_ID_QUITGAME" );
					}
					/*
					//windows close when pressing back
					else if ((layer != null) && (layer == UTGetControlsManager().GetTopmostInputLayer()))
					{
					SND_PLAY(SNDIDX_DENIED);
					UTGetControlsManager().RemoveLayer("LAYER_ID_QUITGAME");
					}
					*/
					break;
				}
			}
		}
		break;

		case GAME_STATE_GAME:
		{
			/// EXECUTE FIXED TIMESTEP BUSINESS
			fFixedStepTimer += dTime;
			int nFixedStepUpdates = 0;
			while ( fFixedStepTimer >= K_GAME_FIXED_TIMESTEP_DTIME )
			{
				float fFixedTime = K_GAME_FIXED_TIMESTEP_DTIME;
				//SPINE update animation states
				g_spineMgr.UpdateAnimationStates( fFixedTime );
				// Update level and all spine objects and bones
				g_level.UpdateFixedTimestep( fFixedTime );
				// SPINE update final skeleton world positions (no bone changes allowed after this)
				g_spineMgr.Update( fFixedTime );

				// remove from accumulator
				fFixedStepTimer -= K_GAME_FIXED_TIMESTEP_DTIME;
				nFixedStepUpdates++;
			}

			g_level.Update( fElapsedTime );

			/*
			//#DMC: comentat cat timp lucrez, functioneaza corect:
			if ( !bSyncUpdate ) //not networked or network sync finised even if still during gameplay
			{
				//ingame menu on ESC-back
				if ( g_level.m_levelState == K_LVL_STATE_PLAYING )
				{
					CCtrlLayer* layer = UTGetGUI().GetLayerByName( "LAYER_ID_IGM_MENU" );
					if ( ( layer == null ) && ( !UTGetGUI().bIsBlocking ) )
					{
						for ( UINT kk = 0; kk < UTGetCtrlrMgr().m_arrControllers.size(); kk++ )
						{
							//show menu
							if ( UTGetCtrlrMgr().m_arrControllers[ kk ]->sCommands.keyState[ K_CM_COMMAND_BACK ] == K_CM_BUTSTATE_JUSTPRESSED )
							{
								SND_PLAY( SNDIDX_CLICK );
								UTGetGUI().ShowLayerOnce( "LAYER_ID_IGM_MENU" );
								break;
							}
						}
					}
					else if ( ( layer != null ) && ( layer == UTGetGUI().GetTopmostInputLayer() ) )
					{
						for ( UINT kk = 0; kk < UTGetCtrlrMgr().m_arrControllers.size(); kk++ )
						{
							//remove onscreen menu
							if ( ( UTGetCtrlrMgr().m_arrControllers[ kk ]->sCommands.keyState[ K_CM_COMMAND_BACK ] == K_CM_BUTSTATE_JUSTPRESSED ) ||
								( UTGetCtrlrMgr().m_arrControllers[ kk ]->sCommands.keyState[ K_CM_COMMAND_RELOAD ] == K_CM_BUTSTATE_JUSTPRESSED ) )
							{
								SND_PLAY( SNDIDX_DENIED );
								UTGetGUI().RemoveLayer( "LAYER_ID_IGM_MENU" );
								break;
							}
						}
					}
				}

				//update game if no blocking window is shown
				if ( !UTGetGUI().bIsBlocking )
				{
					//SPINE update animation states
					g_spineMgr.UpdateAnimationStates( fElapsedTime, fTimeline );

					// Update level and all spine objects and bones
					g_level.UpdateFixedTimestep( fElapsedTime );
					g_level.Update( fElapsedTime );

					//SPINE update final skeleton world positions (no bone changes allowed after this)
					g_spineMgr.Update( fElapsedTime, fTimeline );

					g_bLevelNeedsUpdate = false;
				}
				//#HACK: update once after resolution changed so we adjust cameras
				if ( g_bLevelNeedsUpdate )
				{
					LOG_DBG( L"> Update called with dtime: 0.0" );
					g_level.UpdateFixedTimestep( 0.0f );
					g_bLevelNeedsUpdate = false;
				}
			}
			else  //networked, syncing update
			{
				//ingame menu on ESC-back (if chat is closed)
				bool bCanOpenMenu = true;
		#ifdef ENABLE_CHAT_WINDOW
				//because the chat window exits immediately we have to wait a little until we can bring the menu up
				if ( ( g_ChatWnd.IsReceivingInput() ) || ( g_ChatWnd.fTimeSinceLastInput < K_CW_MIN_TIME_BETWEEN_INPUTS ) )
					bCanOpenMenu = false;
		#endif
				if ( bCanOpenMenu )
				{
					//ingame menu on ESC-back
					if ( g_level.m_levelState == K_LVL_STATE_PLAYING )
					{
						CCtrlLayer* layer = UTGetGUI().GetLayerByName( "LAYER_ID_IGM_MENU_NET" );
						if ( ( layer == null ) && ( !UTGetGUI().bIsBlocking ) )
						{
							for ( UINT kk = 0; kk < UTGetCtrlrMgr().m_arrControllers.size(); kk++ )
							{
								CController* ctrlr = UTGetCtrlrMgr().m_arrControllers[ kk ];
								//ignore network controllers
								if ( ctrlr->eType == K_CM_CT_NET_FRAMELOCK )
									continue;
								//show menu
								if ( ctrlr->sCommands.keyState[ K_CM_COMMAND_BACK ] == K_CM_BUTSTATE_JUSTPRESSED )
								{
									SND_PLAY( SNDIDX_CLICK );
									UTGetGUI().ShowLayerOnce( "LAYER_ID_IGM_MENU_NET" );
									break;
								}
							}
						}
						else if ( ( layer != null ) && ( layer == UTGetGUI().GetTopmostInputLayer() ) && ( layer->alpha >= 1.0f ) )
						{
							for ( UINT kk = 0; kk < UTGetCtrlrMgr().m_arrControllers.size(); kk++ )
							{
								CController* ctrlr = UTGetCtrlrMgr().m_arrControllers[ kk ];
								//ignore network controllers
								if ( ctrlr->eType == K_CM_CT_NET_FRAMELOCK )
									continue;
								//remove onscreen menu
								if ( ( ctrlr->sCommands.keyState[ K_CM_COMMAND_BACK ] == K_CM_BUTSTATE_JUSTPRESSED ) ||
									( ctrlr->sCommands.keyState[ K_CM_COMMAND_RELOAD ] == K_CM_BUTSTATE_JUSTPRESSED ) )
								{
									SND_PLAY( SNDIDX_DENIED );
									UTGetGUI().RemoveLayer( "LAYER_ID_IGM_MENU_NET" );
									break;
								}
							}
						}
					}
				}
				//sync random seed again here (makes sure we don't get desynced between debug and release versions)
				//resets the number of random numbers requested
				g_level.m_rand.SetRandSeed( g_netlock.m_unRandomSeed + nUpdateFrame );
				//LOG(L"--update dT=%.6f T=%.6f rand:%d--", fElapsedTime, fTime, g_level.m_rand.GetRandomSeed());

				//SPINE update animation states
				g_spineMgr.UpdateAnimationStates( fElapsedTime, fTimeline );

				g_level.UpdateFixedTimestep( fElapsedTime );
				g_level.Update( fElapsedTime );
				//SPINE update animation states
				g_spineMgr.Update( fElapsedTime, fTimeline );
				g_bLevelNeedsUpdate = false;
			}
			 */
			
			///--- update editor after updating the game ---
			g_editor.Update( fElapsedTime );
		}
		break;

#ifdef K_CONTROLS_EDITOR
		case GAME_STATE_CONTROLSED:
			g_ControlsEditor.SetCameraTransform( &UTApp().g_cam360hScreen );
			g_ControlsEditor.Update( fElapsedTime );
			break;
#endif

	}





	//GC calls at the end
	fGCtimer += dTime;
	if ( fGCtimer >= K_GAME_GC_TIMER_S )
	{
		fGCtimer = 0.0f;
		GC();
	}
}

void CGame::BeforePaint()
{
	switch ( GameState::state )
	{
		case GAME_STATE_GAME:
		{
			//game is networked? Don't paint until we sync one frame (fixes bug that showed a frame from last coop game)
			/*
			if ((UTGetAppClass().IsGameNetworked()) && (g_nLastSyncedFrame <= 1))
			{
				break;
			}
			*/
			// Deferred buffers use their own begin and end for UTPainter();
			g_level.PaintDeferredBuffers(0.0f);
		}
		break;

		default:  //on all other states just clear the RTT for now
		{
			//g_level.PaintOffscreen_nothing();
			//g_level.PaintComposition_nothing();
		}
		break;
	}
}

void CGame::Paint( PDEVICE pDevice, ID3DXSprite* pSpr, float dTime )
{
	_ASSERT( pSpr != nullptr && pDevice != nullptr );

	//#INFO: it uses the fixed timestep remainder to extrapolate the positions into the future inside the paint functions
	// this allows the game to render smoothly and completely disconnect the update from the render pass

	switch ( GameState::state )
	{
		case GAME_STATE_PRELOAD:
		{
		}
		break;
		case GAME_STATE_DEVELOPER:
		{
			UTApp().App_PaintState_Developer( pDevice, pSpr, dTime );
		}
		break;

		case GAME_STATE_LOADING:
		{
			UTApp().App_PaintState_Loading( pDevice, pSpr, dTime );
		}
		break;

		case GAME_STATE_NET_LOBBY:
		{
			g_mainMenu.Paint();
		}
		break;

		case GAME_STATE_JOIN_COOP_LIST:
		case GAME_STATE_WORKSHOP:
		case GAME_STATE_GAME_MODE_SELECTION:
		case GAME_STATE_CHAPTER_SELECTION:
		case GAME_STATE_LEVEL_SELECTION:
		case GAME_STATE_MAINMENU:
		{
			g_mainMenu.Paint();

			/*
			// show font image
			if (DXUTIsKeyDown('6'))
			{
				if (g_font1.m_atlas.pTex != nullptr)
				{
					pSpr->Flush();
					CCameraTransform::SetActiveCameraIdentity(pDevice);
					RECT src;
					SetRect(&src, 0, 0, g_font1.m_atlas.atlasSize.w, g_font1.m_atlas.atlasSize.h);
					pSpr->SetTransform(&g_matIdentity);
					pSpr->Draw(g_font1.m_atlas.pTex, &src, NULL, &D3DXVECTOR3(UTGetAppClass().g_rectRender.x, 0.0f, 0.0f), 0xffffffff);
					pSpr->Flush();
				}
			}

			UTLang().SetString(STR_TEMP1, L"Play Game now!");

			PVERTEXSHADER pSprVS = UTGetShaderManager().GetVShaderByName(L"VS_SPRITES2D");
			if (pSprVS)
				UTPainter().Begin(pSprVS, UTGetAppClass().g_matProj);

			g_font1.DrawStringLine(UTLang().strings[STR_TEMP1], 400.0f, 200.0f, FTFF_LEFT | FTFF_VCENTER, 0xffffffff);
			g_font1.DrawStringLine(UTLang().strings[STR_TEMP1], 400.0f, 200.0f + g_font1.rowHeight, FTFF_RIGHT, 0xff88ff88);
			g_font1.DrawStringLine(UTLang().strings[STR_TEMP1], 400.0f, 200.0f + 2 * g_font1.rowHeight, FTFF_CENTER, 0xff8888ff);

			UTPainter().End();
			*/
		}
		break;
		case GAME_STATE_PLAYER_SELECTION:
		{
			g_playerSelScr.Paint( pSpr );
		}
		break;
		case GAME_STATE_GAME:
		{
			//if level not loaded just skip paint
			if ( !g_level.m_bLoaded )
				break;

			// paint game elements above RTT content
			g_level.Paint();

			//final flush
			pSpr->Flush();

			/*
			pSpr->Flush();
			PVERTEXSHADER vsspr = UTGetShaderManager().GetVShaderByName(L"VS_SPRITES2D");
			if (vsspr)
			{
				g_SprPainter.Begin(vsspr, UTGetAppClass().g_matProj);

				for (int kk = 0; kk < 5; kk++)
					CSprite::paintFrameNEW(&g_level.m_sprInterface, Vec3(100.0f + 30.0f * kk, 100.0f + 30.0f * kk, 0.0f), ANM_IGM_INTERFACE_SPR_PORTRAITS, kk,
						0xffffffff, fTime, Vec2(1.0f + 0.4f * sin(fTime), 1.0f - 0.4f * sin(fTime)));


				g_SprPainter.End();
			}
			*/


			///--- level editor ---
			g_editor.Paint( pSpr );

			///--- string dummies ---
			pSpr->SetTransform( &g_matIdentity );
			//paint string dummies
			/*
			CCameraTransform::SetActiveCamera(pDevice, &UTGetAppClass().g_cam240hScreen);
			g_particlesMgr.PaintStringDummies();
			pSpr->Flush();
			 */
			 //debug stuff
#if defined(_DEBUG) || defined(DEBUG)
				//game screen space
			CCameraTransform::SetActiveCamera( pDevice, &UTApp().g_camRTScreen );

			if ( DXUTIsKeyDown( '9' ) )
			{
				CRTManager::CEngineRenderTarget* pRT = UTGetRTManager().GetRTbyUID( K_RTID_COLORDEPTHSTENCIL );
				if ( pRT != null )
				{
					pSpr->Flush();
					CCameraTransform::SetActiveCameraIdentity( pDevice );
					RECT src;
					SetRect( &src, 0, 0, pRT->nWidth, pRT->nHeight );
					pSpr->SetTransform( &g_matIdentity );
					pSpr->Draw( pRT->m_pRTTexture, &src, NULL, &D3DXVECTOR3( UTApp().g_rectRender.x, 0.0f, 0.0f ), 0xffffffff );
					pSpr->Flush();
				}
			}
			if ( DXUTIsKeyDown( '8' ) )
			{
				CRTManager::CEngineRenderTarget* pRT = UTGetRTManager().GetRTbyUID( K_RTID_TEMP1 );
				if ( pRT != null )
				{
					pSpr->Flush();
					CCameraTransform::SetActiveCameraIdentity( pDevice );
					RECT src;
					SetRect( &src, 0, 0, pRT->nWidth, pRT->nHeight );
					pSpr->SetTransform( &g_matIdentity );
					pSpr->Draw( pRT->m_pRTTexture, &src, NULL, &D3DXVECTOR3( UTApp().g_rectRender.x, 0.0f, 0.0f ), 0xffffffff );
					pSpr->Flush();
				}
			}
			if ( DXUTIsKeyDown( '0' ) )
			{
				CRTManager::CEngineRenderTarget* pRT = UTGetRTManager().GetRTbyUID( K_RTID_FINAL );
				if ( pRT != null )
				{
					pSpr->Flush();
					CCameraTransform::SetActiveCameraIdentity( pDevice );
					RECT src;
					SetRect( &src, 0, 0, pRT->nWidth, pRT->nHeight );
					pSpr->SetTransform( &g_matIdentity );
					pSpr->Draw( pRT->m_pRTTexture, &src, NULL, &D3DXVECTOR3( UTApp().g_rectRender.x, 0.0f, 0.0f ), 0xffffffff );
					pSpr->Flush();
				}
			}
			//if (DXUTIsKeyDown('9'))
			//{
			//	pSpr->Flush();
			//	CCameraTransform::SetActiveCameraIdentity(pDevice);
			//	RECT src;
			//	SetRect(&src, 0, 0, 512, 512);
			//	pSpr->SetTransform(&g_matIdentity);
			//	pSpr->Draw(g_level.m_pRTTexture, &src, NULL, &Vec3(UTGetAppClass().g_rectRender.x, 0.0f, 0.0f), 0xffffffff);
			//	pSpr->Flush();
			//}
			//if (DXUTIsKeyDown('0'))
			//{
			//	pSpr->Flush();
			//	CCameraTransform::SetActiveCameraIdentity(pDevice);
			//	RECT src;
			//	SetRect(&src, 512, 0, 1024, 512);
			//	pSpr->SetTransform(&g_matIdentity);
			//	pSpr->Draw(g_level.m_pRTTexture, &src, NULL, &Vec3(UTGetAppClass().g_rectRender.x, 0.0f, 0.0f), 0xffffffff);
			//	pSpr->Flush();
			//}
#endif
		}
		break;

	}

}

void CGame::Release()
{

}

void CGame::GC()
{
	g_level.GC();
}


///----------------------------------------------------------------------------------
/// DEVICE CALLBACKS
///----------------------------------------------------------------------------------
OPRESULT CGame::OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc )
{
	m_pDevice = pDevice;
	return K_OP_OK;
}

OPRESULT CGame::OnResetDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc )
{
	m_pDevice = pDevice;
	return K_OP_OK;
}

OPRESULT CGame::OnLostDevice()
{
	m_pDevice = nullptr;
	return K_OP_OK;
}

OPRESULT CGame::OnDestroyDevice()
{
	m_pDevice = nullptr;
	return K_OP_OK;
}
