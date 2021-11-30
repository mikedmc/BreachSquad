#include "dxstdafx.h"

CGame::CGame()
{
	fTimeline = 0.0f;
}

CGame::~CGame()
{

}

void CGame::Update( float dTime )
{
	fTimeline += dTime;
	fGCtimer += dTime;
	// game calls


	//GC calls at the end
	if ( fGCtimer >= K_GAME_GC_TIMER_S )
	{
		fGCtimer = 0.0f;
		GC();
	}
}

void CGame::BeforePaint()
{
	switch ( g_gameState )
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
			g_level.PaintDeferredBuffers();
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

	switch ( g_gameState )
	{
		case GAME_STATE_PRELOAD:
		{
		}
		break;
		case GAME_STATE_DEVELOPER:
		{
			UTGetAppClass().App_PaintState_Developer( pDevice, pSpr, dTime );
		}
		break;

		case GAME_STATE_LOADING:
		{
			UTGetAppClass().App_PaintState_Loading( pDevice, pSpr, dTime );
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
			CCameraTransform::SetActiveCamera( pDevice, &UTGetAppClass().g_camRTScreen );

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
					pSpr->Draw( pRT->m_pRTTexture, &src, NULL, &D3DXVECTOR3( UTGetAppClass().g_rectRender.x, 0.0f, 0.0f ), 0xffffffff );
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
					pSpr->Draw( pRT->m_pRTTexture, &src, NULL, &D3DXVECTOR3( UTGetAppClass().g_rectRender.x, 0.0f, 0.0f ), 0xffffffff );
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
					pSpr->Draw( pRT->m_pRTTexture, &src, NULL, &D3DXVECTOR3( UTGetAppClass().g_rectRender.x, 0.0f, 0.0f ), 0xffffffff );
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
