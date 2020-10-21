#include "dxstdafx.h"

CMainMenu::CMainMenu()
{
	m_pDevice = null;
	m_pSprite = null;
	m_eTargetGameState = GAME_STATE_EMPTY;

	fLocalTimeline = 0.0f;
	//initialize on invalid state
	m_eState = K_MM_STATE_INVALID;
	
	memset(m_arrSelItems, 0, sizeof(m_arrSelItems));
	m_nSelection = 0;
	m_nSelectionOld = 0;
	m_nSelElements = 0;
	m_nSelPage = 0;
	m_fSelTimer = 0.0f;
}

CMainMenu::~CMainMenu()
{
	m_pSprite = null;
	m_pDevice = null;
	m_eState = K_MM_STATE_INVALID;

	Release();
}

HRESULT CMainMenu::LoadSprites(WCHAR * strSpritePath)
{
	HRESULT hr = S_OK;
	if (FAILED(m_sprCol.LoadSprites(strSpritePath)))
	{
		return hr;
	}
	return hr;
}

void CMainMenu::SetSpritePtr(ID3DXSprite* pSprite)
{
	m_pSprite = pSprite;
}

void CMainMenu::SetState(EMM_State neState, int nArg1 /*= 0*/)
{
	fLocalTimeline = 0.0f;
	m_nSubstate = 0;
	m_bSelectionMade = false;
	m_rectSel.Set(0, 0, 0, 0);
	m_rectSelTarget.Set(0, 0, 0, 0);
	m_fSelTimer = 0.0f;

	//set new state
	m_eState = neState;

#ifdef ENABLE_STEAM_WORKSHOP
	//force downloaded content menu selection
	if ((m_eState == K_MM_STATE_LEVEL_SELECT) && (g_userData[K_MEMID_SELECTED_CHAPTER] >= UTGetChaptersList().GetChaptersCnt()))
	{
		m_eState = K_MM_STATE_DOWNLOADED_LEVEL_SELECT;
	}
#endif

	switch (m_eState)
	{
		case K_MM_STATE_WORKSHOP:
		{
		}
		break;

		case K_MM_STATE_NET_LOBBY:
		{
		}
		break;

		case K_MM_STATE_MAINMENU:
		{
#ifdef ENABLE_STEAM_WORKSHOP
			if (UTGetControlsManager().GetLayerByName("LAYER_ID_MAINMENU") == null)
			{
				UTGetControlsManager().ShowLayerOnce("LAYER_ID_MAINMENU");
			}
#else
			if (UTGetControlsManager().GetLayerByName("LAYER_ID_MAINMENU_NOWORKSHOP") == null)
			{
				UTGetControlsManager().ShowLayerOnce("LAYER_ID_MAINMENU_NOWORKSHOP");
			}
#endif
			//timer used for highlighting of DK2 ad
			m_fSelTimer = 0.0f;
		}
		break;

		case K_MM_STATE_GAME_MODE_SELECT:
		{
			//set final target
			m_eTargetGameState = (eGameState)nArg1;
			//special screen mode for quick match online coop
			//bool bIsCoopQM = (UTGetAppClass().m_Settings.devnet_eNetGameType == CApplicationSettings::K_NETGAME_TYPE_QUICK_MATCH);
			m_arrSelItems[0] = K_MM_MODE_VINFINITE;
			m_arrSelItems[1] = K_MM_MODE_CLASSIC;
			m_arrSelItems[2] = K_MM_MODE_ZOMBIE_INVASION;
			m_nSelElements = 3;

			m_nSelection = 1;
			
			m_nSelectionOld = m_nSelection;
			m_fSelPageCursor = m_nSelection;
		}
		break;
		
		case K_MM_STATE_CHAPTER_SELECT:
		{
			//load modded chapters every time we enter chapter selection screen in case a mod changed them
			WCHAR wcsPath[MAX_PATH];
			FileManager::GetMediaPath(L"media/levels/missions/missions.xml", wcsPath);
			UTGetChaptersList().LoadChapters(wcsPath);

			m_nSelection = g_userData[K_MEMID_SELECTED_CHAPTER];
			m_nSelElements = UTGetChaptersList().GetChaptersCnt();
#ifdef ENABLE_STEAM_WORKSHOP
			//add the DOWNLOADED LEVELS chapter
			m_nSelElements += 1;
			//remove workshop chapter if we're not on HOST PRIVATE
			bool bIsOnline = (UTGetAppClass().m_Settings.devnet_eNetGameType != CApplicationSettings::K_NETGAME_TYPE_NO_NETWORK);
			bool bIsHostPrivate = (UTGetAppClass().m_Settings.devnet_eNetGameType == CApplicationSettings::K_NETGAME_TYPE_HOST_PRIVATE);
			if ((bIsOnline) && (!bIsHostPrivate))
			{
				m_nSelElements -= 1;
				//move selection back a little
				if (m_nSelection == UTGetChaptersList().GetChaptersCnt())
					m_nSelection--;
			}
#endif			

			m_nSelPage = m_nSelection;
			m_fSelPageCursor = m_nSelPage;
		}
		break;

		case K_MM_STATE_DOWNLOADED_LEVEL_SELECT:
		{
#ifdef ENABLE_STEAM_WORKSHOP
			UTGetAppClass().g_texManager.Release();
			memset(m_arrSelItems, -1, sizeof(m_arrSelItems));
			m_nSelElements = 0;
			//we'll save data in m_arrSelItems like this: 
			//foreach mod in SINGLE_LEVEL_MODS: imageIndex in texManager, mission type(hostage, bomb, etc), mod index in arrMods
			for (int kk = 0; kk < UTGetModsManager().m_arrMods.GetSize(); kk++)
			{
				CModsManager::CModDescriptor* mod = UTGetModsManager().m_arrMods[kk];
				if (mod->eType != CModsManager::K_MOD_TYPE_SINGLE_LEVEL)
					continue;
				if (mod->bActive == false)
					continue;

				WCHAR strPath[MAX_PATH];
				//see if mod has affected level
				if (!mod->GetFullPathToAffectedFile(0, strPath, MAX_PATH))
				{
					ErrorBox(K_ERR_WARNING, L"[WARNING] SetState::Mod is missing level file!");
					continue;
				}
				//save mission type
				m_arrSelItems[m_nSelElements * 3 + 1] = App_GetMissionType(strPath);
				//load image and save index to it
				if (mod->GetFullPathToModImage(strPath, MAX_PATH))
				{
					//forcing image to be loaded at 16:9 fixed size 
					HRESULT hr = UTGetAppClass().g_texManager.AddTexture(strPath, &m_arrSelItems[m_nSelElements * 3], D3DFMT_A8R8G8B8, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 96, 54);
					if (FAILED(hr))
					{
						ErrorBox(K_ERR_WARNING, L"[WARNING] SetState::Couldn't load mod image. Mod name: [%s] Image path: [%s]", mod->shName.text, strPath);
					}
				}

				//save mod index in mods list
				m_arrSelItems[m_nSelElements * 3 + 2] = kk;
				//add selection element
				m_nSelElements++;
			}

			m_nSelection = 0;
			//no items to display, default on BACK button
			if (m_nSelElements == 0)
				m_nSelection = -1;

			m_nSelectionOld = m_nSelection;
			m_nSelRows = 2; m_nSelColumns = 5;
			m_nSelPagesCnt = (int)ceil((float)m_nSelElements / (float)(m_nSelRows * m_nSelColumns));
			m_nSelPage = 0;
			m_fSelPageCursor = 0.0f;
#endif		
		}
		break;

		case K_MM_STATE_LEVEL_SELECT:
		{
			//selected level index is 0 based and relative to selected chapter
			m_nSelection = g_userData[K_MEMID_SELECTED_LEVEL];
			CLAMP(m_nSelection, 0, K_GAME_LEVELS_PER_CHAPTER - 1);

			m_nSelElements = K_GAME_LEVELS_PER_CHAPTER;
			m_nSelRows = 3; m_nSelColumns = 4;

			m_nSelPagesCnt = (int)ceil((float)m_nSelElements / (float)(m_nSelRows * m_nSelColumns));
			m_nSelPage = m_nSelection / (m_nSelRows * m_nSelColumns);
			m_fSelPageCursor = m_nSelPage;

			m_rectSel.Set(0, 0, 0, 0);
			m_rectSelTarget.Set(0, 0, 0, 0);

			m_bSelectionMade = false;

			//#ACHIEVEMENTS: award achievements for chapter unlocking
			int nSelChapter = g_userData[K_MEMID_SELECTED_CHAPTER];
			int arrAchiev[] = { -1, ACH_TROUBLEMAKERS_ARRIVING, ACH_THINGS_HEATING_UP, ACH_METRO_CALLING, ACH_HELL_IS_COMING, -1, ACH_GOING_HOME, -1, -1, -1 };
			if ((arrAchiev[nSelChapter] >= 0) && (UTGetChaptersList().IsChapterUnlocked(nSelChapter, g_userData[K_MEMID_MISSIONS_COMPLETED])))
			{
				UTGetAchievementManager().UnlockAchievement((EGameAchievements)arrAchiev[nSelChapter]);
			}
		}
		break;
	}

}

bool CMainMenu::RequestLeaderboardsUpdate(bool bCoop)
{
#ifdef ENABLE_LEADERBOARDS
	int nLevelNumber = m_nSelection;
	int nChapterNumber = g_userData[K_MEMID_SELECTED_CHAPTER];

	if ((nChapterNumber < 0) || (nChapterNumber >= UTGetChaptersList().GetChaptersCnt()))
		return false;
	if ((nLevelNumber < 0) || (nLevelNumber >= K_GAME_LEVELS_PER_CHAPTER))
		return false;

	char pszBoardName[MAX_PATH];

#ifdef ENABLE_STEAM
	char strFormat[] = "%s%d.%d";
#endif
#ifdef ENABLE_GALAXY
	char strFormat[] = "%s%d_%d";
#endif

	if (!bCoop)
		StringCchPrintfA(pszBoardName, MAX_PATH, strFormat, K_GAME_STR_LEADERBOARDS_PREFIX_SP, nChapterNumber + 1, nLevelNumber + 1);
	else
		StringCchPrintfA(pszBoardName, MAX_PATH, strFormat, K_GAME_STR_LEADERBOARDS_PREFIX_COOP, nChapterNumber + 1, nLevelNumber + 1);
	//vertical infinite mode? (not really necessary because this function is only called from menus therefore not available for infinite tower)
	if (g_gameMode == GAME_MODE_INFINITE_TOWER)
	{
		if (!bCoop)
			StringCchPrintfA(pszBoardName, MAX_PATH, "%s", K_GAME_STR_LEADERBOARDS_VINFINITE_PREFIX_SP);
		else
			StringCchPrintfA(pszBoardName, MAX_PATH, "%s", K_GAME_STR_LEADERBOARDS_VINFINITE_PREFIX_COOP);
	}
	//on zombie mode leaderboards have an appendix
	if (g_gameMode == GAME_MODE_ZOMBIE_INVASION)
		StringCchCatA(pszBoardName, MAX_PATH, "_zm");
	//reset strings too
	g_stringsMgr.SetString(STR_LEADERBOARDS_NAMES_VAL, L"...");
	g_stringsMgr.SetString(STR_LEADERBOARDS_SCORES_VAL, L"...");
	g_stringsMgr.SetString(STR_LEADERBOARDS_PLAYERSCORE_VAL, L"...");
	//request downloading of scores
	UTGetLeaderboards().QueueJob(K_JOB_GET_SCORES_GLOBAL, pszBoardName, 1);
	//request downloading of your own score
	UTGetLeaderboards().QueueJob(K_JOB_GET_SCORE_FOR_CURRENT_USER, pszBoardName, 0);
#endif

	return true;
}

void CMainMenu::Update(float dTime)
{
	//tinem si old value for timeline
	double fOldLocalTimeline = fLocalTimeline;
	//update local timeline
	fLocalTimeline += dTime;

	ECtrlMgrCommandType eCommand = K_CCTRLMGR_COMMAND_NONE;
	//aleg din controllere doar comenzile necesare clasei
	for (int kk = 0; kk < UTGetCtrlrMgr().m_arrControllers.size(); kk++)
	{
		CController* ctrlr = UTGetCtrlrMgr().m_arrControllers[kk];
		if ((ctrlr->GetButState(K_CM_COMMAND_MOVE_X) == K_CM_BUTSTATE_JUSTPRESSED) && (ctrlr->GetAxisVal(K_CM_COMMAND_MOVE_X) < 0.0f))
			eCommand = K_CCTRLMGR_COMMAND_LEFT;
		if ((ctrlr->GetButState(K_CM_COMMAND_MOVE_X) == K_CM_BUTSTATE_JUSTPRESSED) && (ctrlr->GetAxisVal(K_CM_COMMAND_MOVE_X) > 0.0f))
			eCommand = K_CCTRLMGR_COMMAND_RIGHT;
		if ((ctrlr->GetButState(K_CM_COMMAND_MOVE_Y) == K_CM_BUTSTATE_JUSTPRESSED) && (ctrlr->GetAxisVal(K_CM_COMMAND_MOVE_Y) < 0.0f))
			eCommand = K_CCTRLMGR_COMMAND_UP;
		if ((ctrlr->GetButState(K_CM_COMMAND_MOVE_Y) == K_CM_BUTSTATE_JUSTPRESSED) && (ctrlr->GetAxisVal(K_CM_COMMAND_MOVE_Y) > 0.0f))
			eCommand = K_CCTRLMGR_COMMAND_DOWN;

		if ((ctrlr->sCommands.keyState[K_CM_COMMAND_FIRE1] == K_CM_BUTSTATE_JUSTPRESSED) ||
			(ctrlr->sCommands.keyState[K_CM_COMMAND_JUMP] == K_CM_BUTSTATE_JUSTPRESSED) ||
			(ctrlr->sCommands.keyState[K_CM_COMMAND_SELECT] == K_CM_BUTSTATE_JUSTPRESSED))
			eCommand = K_CCTRLMGR_COMMAND_SELECT;

		if ((ctrlr->sCommands.keyState[K_CM_COMMAND_BACK] == K_CM_BUTSTATE_JUSTPRESSED) ||
			(ctrlr->sCommands.keyState[K_CM_COMMAND_RELOAD] == K_CM_BUTSTATE_JUSTPRESSED) /*||
			(ctrlr->sCommands.keyState[K_CM_COMMAND_MELEE] == K_CM_BUTSTATE_JUSTPRESSED)*/)
			eCommand = K_CCTRLMGR_COMMAND_BACK;

		if (eCommand != K_CCTRLMGR_COMMAND_NONE)
			break;
	}

	//save local mouse coords
	D3DXVECTOR2 vLocalMousePos = UTGetAppClass().g_cam240hScreen.ScreenToWorld(g_mouse.pos);

	switch (m_eState)
	{
		case K_MM_STATE_WORKSHOP:
		{
#ifdef ENABLE_STEAM_WORKSHOP
			switch (m_nSubstate)
			{
				//showing "updating mods screen"
				case 0: 
				{
					//we'll need textures for the mods
					UTGetAppClass().g_texManager.Release();

					m_nSubstate++;
				}
				break;
				//actual mods update
				case 1:
				{
					//update mods (download new, delete old)
					Workshop_CheckSubscriptions();

					//load mods images
					for (int ll = 0; ll < UTGetModsManager().m_arrMods.GetSize(); ll++)
					{
						m_arrSelItems[ll] = -1;
						CModsManager::CModDescriptor* nmod = UTGetModsManager().m_arrMods[ll];
						WCHAR strImgPath[MAX_PATH];
						if (nmod->GetFullPathToModImage(strImgPath, MAX_PATH))
						{
							//forcing image to be loaded at 16:9 fixed size 
							HRESULT hr = UTGetAppClass().g_texManager.AddTexture(strImgPath, &m_arrSelItems[ll], D3DFMT_A8R8G8B8, D3DX_FILTER_LINEAR, D3DX_FILTER_LINEAR, 96, 54);
							if (FAILED(hr))
							{
								ErrorBox(K_ERR_WARNING, L"Couldn't load mod image. Mod name: [%s] Image path: [%s]", nmod->shName.text, strImgPath);
							}
						}
					}

					//count mods
					m_nSelElements = UTGetModsManager().m_arrMods.GetSize();
					//initialize paging data
					m_bSelectionMade = false;
					m_nSelection = 0;
					if (m_nSelElements == 0)
						m_nSelection = -1;
					m_nSelectionOld = m_nSelection;
					m_nSelPage = 0;
					m_fSelPageCursor = 0.0f;
					m_nSelRows = 5;
					//displaying mods as rows, single column
					m_nSelPagesCnt = m_nSelElements / m_nSelRows;
					if ((m_nSelElements % m_nSelRows) > 0)
						m_nSelPagesCnt++;

					m_nSubstate++;
				}
				break;
				//mods selector screen
				case 2:
				{
					m_fSelTimer += dTime;

					RECTXYWH_F worldrect = UTGetAppClass().g_rect240hWorld;
					const int nItemSpacing = 10;
					SIZEWH szItem(200, 10);
					RECTXYWH rectItemsList(worldrect.CenterX() - szItem.w / 2, worldrect.Bottom() - 26 - m_nSelRows * (szItem.h + nItemSpacing), szItem.w, m_nSelRows * (szItem.h + nItemSpacing) - nItemSpacing);

					if (eCommand == K_CCTRLMGR_COMMAND_LEFT)
					{
						if ((m_nSelPage > 0) && (m_nSelection >= 0))
						{
							m_nSelPage--;
							m_nSelection -= m_nSelRows;
							if (m_nSelection < 0)
								m_nSelection = 0;
						}
						m_nSelectionOld = m_nSelection;
						m_fSelTimer = 0.0f;
					}
					else if (eCommand == K_CCTRLMGR_COMMAND_RIGHT)
					{
						if ((m_nSelPage < m_nSelPagesCnt - 1) && (m_nSelection >= 0))
						{
							m_nSelPage++;
							m_nSelection += m_nSelRows;
							if (m_nSelection > m_nSelElements - 1)
								m_nSelection = m_nSelElements - 1;
						}
						m_nSelectionOld = m_nSelection;
						m_fSelTimer = 0.0f;
					}
					else if (eCommand == K_CCTRLMGR_COMMAND_DOWN)
					{
						if (m_nSelection >= 0) //don't come back to list from BACK button by pressing down
						{
							int nLocalSelIdx = m_nSelection % m_nSelRows;
							if (nLocalSelIdx < m_nSelRows - 1)
							{
								m_nSelection++;
								//special case for short lists
								if (m_nSelection >= m_nSelElements)
									m_nSelection = -1;
							}
							else
								m_nSelection = -1;
						}
						m_nSelectionOld = m_nSelection;
						m_fSelTimer = 0.0f;
					}
					else if (eCommand == K_CCTRLMGR_COMMAND_UP)
					{
						if (m_nSelection == -1) //back from BACK button
						{
							if (m_nSelElements > 0)
							{
								m_nSelection = m_nSelPage * m_nSelRows + (m_nSelRows - 1);
								if (m_nSelection > m_nSelElements - 1)
									m_nSelection = m_nSelElements - 1;
								m_nSelectionOld = m_nSelection;
								m_fSelTimer = 0.0f;
							}
						}
						else //scroll up
						{
							int nLocalSelIdx = m_nSelection % m_nSelRows;
							if (nLocalSelIdx > 0)
							{
								m_nSelection--;
								m_nSelectionOld = m_nSelection;
								m_fSelTimer = 0.0f;
							}
						}
					}
					else if (eCommand == K_CCTRLMGR_COMMAND_BACK)
					{
						SND_PLAY(SNDIDX_DENIED);

						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
						nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
						UTGetEventManager().QueueEvent(nevent);
					}

					//back button selection rectangle
					RECTXYWH rectButBack(worldrect.CenterX() - 35, worldrect.Bottom() - 25, 70, 16);
					//rectangle of currently selected mod
					int nIdx = m_nSelection - m_nSelPage * m_nSelRows;
					RECTXYWH rectItem(rectItemsList.x, rectItemsList.y + (szItem.h + nItemSpacing) * nIdx, szItem.w, szItem.h);

					//--- mouse selection on mouse click ---
					if (g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED)
					{
						//check chapters click
						RECTXYWH wndrectL = rectItemsList;
						//adjust for difference between graphics and clicks:
						wndrectL.y -= 5; wndrectL.h += 10;

						if (PointInRect(vLocalMousePos, wndrectL))
						{
							int nSel = m_nSelPage * m_nSelRows + (vLocalMousePos.y - wndrectL.y) / (szItem.h + nItemSpacing);
							if ((nSel < 0) || (nSel >= m_nSelElements))
								break;

							m_nSelection = nSel;
							m_fSelTimer = 0.0f;
							if (m_nSelection >= 0)
							{
								if(m_nSelectionOld == m_nSelection)
									eCommand = K_CCTRLMGR_COMMAND_SELECT;
								else
									m_nSelectionOld = m_nSelection;
							}
						}
						else if ((vLocalMousePos.x > wndrectL.Right()) && (vLocalMousePos.y > wndrectL.y) && (vLocalMousePos.y < wndrectL.Bottom()))
						{
							if (m_nSelPage < m_nSelPagesCnt - 1)
							{
								if (m_nSelection < 0)
									m_nSelection = m_nSelPage * m_nSelRows;
								m_nSelPage++;
								m_nSelection += m_nSelRows;
								if (m_nSelection > m_nSelElements - 1)
									m_nSelection = m_nSelElements - 1;
							}
							m_nSelectionOld = m_nSelection;
							m_fSelTimer = 0.0f;
						}
						else if ((vLocalMousePos.x < wndrectL.x) && (vLocalMousePos.y > wndrectL.y) && (vLocalMousePos.y < wndrectL.Bottom()))
						{
							if (m_nSelPage > 0)
							{
								if (m_nSelection < 0)
									m_nSelection = m_nSelPage * m_nSelRows;
								m_nSelPage--;
								m_nSelection -= m_nSelRows;
								if (m_nSelection < 0)
									m_nSelection = 0;
							}
							m_nSelectionOld = m_nSelection;
							m_fSelTimer = 0.0f;
						}
						//check buttons
						if (PointInRect(vLocalMousePos, rectButBack))
						{
							if (m_nSelection == -1)
								eCommand = K_CCTRLMGR_COMMAND_SELECT;

							m_nSelection = -1;
							m_nSelectionOld = m_nSelection;
							m_fSelTimer = 0.0f;
						}
					}

					//--- SELECTION COMMAND ---
					if (eCommand == K_CCTRLMGR_COMMAND_SELECT)
					{
						//back button
						if (m_nSelection == -1)
						{
							m_bSelectionMade = true;

							//save mods status
							UTGetModsManager().SaveModsToCacheFile();

							CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
							nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
							nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
							UTGetEventManager().QueueEvent(nevent);

							SND_PLAY(SNDIDX_DENIED);
							break;
						}

						//act on command
						if ((m_nSelection >= 0) && (m_nSelection < m_nSelElements))
						{
							CModsManager::CModDescriptor *mod = UTGetModsManager().GetModDescByIndex(m_nSelection);
							if (mod != null)
							{
								SND_PLAY(SNDIDX_CLICK);
								bool bAllGood = true;
								// Is mod still compatible (when activating, is inactive now) ?
								if (mod->bActive == false)
								{
									if (!UTGetModsManager().IsCompatibleWithCurrentVersion(mod))
									{
										UTGetControlsManager().MessageBoxOK(STR_WARNING, STR_MOD_INCOMPATIBLE_MSG);
										//SND_PLAY(SNDIDX_DENIED);
										//bAllGood = false;
									}
								}
								// try to activate the mod, checking for conflicts
								if (bAllGood)
								{
									CModsManager::CModDescriptor* pConflicting = UTGetModsManager().SetModActive(mod, !mod->bActive);

									if (pConflicting != null)
									{
										UTGetControlsManager().MessageBoxOK(STR_WARNING, STR_MOD_CONFLICTING_MSG);
										SND_PLAY(SNDIDX_DENIED);
										bAllGood = false;
									}
								}
							}
						}
					}


					//float selection cursor
					if (m_nSelection >= 0)
						MATH_EaseTo_quadratic(&m_fSelPageCursor, (float)m_nSelection, 10.0f * dTime, 1.0f * dTime);
					//set selection rect
					if (m_nSelection >= 0)
					{
						m_rectSelTarget = rectItem;
						m_rectSelTarget.Inflate(-1.0f);
					}
					else if (m_nSelection == -1) //buton back
					{
						m_rectSelTarget = rectButBack;
						m_rectSelTarget.Inflate(-6.0f);
					}
					//first time entry check for selection cursor
					if ((m_rectSel.w == 0.0f) || (m_rectSel.h == 0.0f))
					{
						m_rectSel = m_rectSelTarget;
					}

					//morph current selection into target selection
					CAABB selaabb_from, selaabb_to;
					selaabb_from.Set(m_rectSel);
					selaabb_to.Set(m_rectSelTarget);

					AABB_MorphInto_quadratic(&selaabb_from, &selaabb_to, 20.0f * dTime, 60.0f * dTime);

					m_rectSel.Set(selaabb_from.vMin.x, selaabb_from.vMin.y, selaabb_from.vSize.x, selaabb_from.vSize.y);
				}
				break;
			}
#endif
		}
		break;

		case K_MM_STATE_NET_LOBBY:
		{
		}
		break;

		case K_MM_STATE_MAINMENU:
		{
			RECTXYWH_F worldrect = UTGetAppClass().g_rect240hWorld;

			//generare particule bokeh
			if (g_timers.Tick(400))
			{
				g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_BOKEH_L, false, randint(2), &D3DXVECTOR2(worldrect.Right() + randfloatsgn(50.0f), worldrect.h + 30.0f),
					&D3DXVECTOR2(-10.0f, -5.0f), &D3DXVECTOR2(-randfloat(20.0f), -20.0f - randfloat(10.0f)), 3.0f + randfloat(2.0f), 0.6f, 0.0f, 0.0f, 0.0f, 0.1f, 1.0f, 0x88ffffff, K_PART_LAYER_FRONT_LIGHT);
			}
			//particule foc
			if (g_timers.Tick(90))
			{
				g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRE_REAL, false, randint(12), &D3DXVECTOR2(worldrect.Right() + 10.0f, randfloat(worldrect.h / 2.0f) + worldrect.h * 0.25f),
					&D3DXVECTOR2(-4.0f, -8.0f), &D3DXVECTOR2(-40.0f - randfloat(20.0f), -10.0f - randfloat(10.0f)), 3.0f + randfloat(2.0f), 0.6f - randfloat(0.2f), -0.05f, 0.0f, randfloatsgn(PI), 0.1f, 1.0f, 0xffffffff, K_PART_LAYER_FRONT_LIGHT);
			}
			//particule foc spate
			if (g_timers.Tick(60))
			{
				g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRE_REAL, false, randint(12), &D3DXVECTOR2(worldrect.Right() + 10.0f, randfloat(worldrect.h / 2.0f) + worldrect.h * 0.25f),
					&D3DXVECTOR2(-8.0f, -4.0f), &D3DXVECTOR2(-40.0f - randfloat(20.0f), -10.0f - randfloat(10.0f)), 3.0f + randfloat(2.0f), 0.5f - randfloat(0.2f), -0.05f, 0.0f, randfloatsgn(PI), 0.1f, 1.0f, 0xaaffffff, K_PART_LAYER_NORMAL_LIGHT);
			}

		}
		break;

		case K_MM_STATE_GAME_MODE_SELECT:
		{
			//special screen mode for quick match online coop
			bool bIsCoopQM = (UTGetAppClass().m_Settings.devnet_eNetGameType == CApplicationSettings::K_NETGAME_TYPE_QUICK_MATCH);

			RECTXYWH_F worldrect = UTGetAppClass().g_rect240hWorld;
			RECTXYWH picrect = m_sprCol.GetAFrameBBox(ANM_MENUS_SPR_GAME_MODE_SPLASHES, 0);
			picrect.h += 32;
			int wndSpacing = 24;

			D3DXVECTOR2 vpos = worldrect.Center();

			if (eCommand == K_CCTRLMGR_COMMAND_LEFT)
			{
				if (m_nSelection > 0)
					m_nSelection--;
			}
			else if (eCommand == K_CCTRLMGR_COMMAND_RIGHT)
			{
				if ((m_nSelection < m_nSelElements - 1) && (m_nSelection >= 0))
					m_nSelection++;
			}
			else if (eCommand == K_CCTRLMGR_COMMAND_DOWN)
			{
				if(m_nSelection >= 0)
					m_nSelectionOld = m_nSelection;
				m_nSelection = -1;
			}
			else if ((eCommand == K_CCTRLMGR_COMMAND_UP) && (m_nSelection == -1))
			{
				m_nSelection = m_nSelectionOld;
			}
			else if (eCommand == K_CCTRLMGR_COMMAND_BACK)
			{
				SND_PLAY(SNDIDX_DENIED);

				CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
				nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
				nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
				UTGetEventManager().QueueEvent(nevent);
			}

			//back button selection rectangle
			RECTXYWH rectButBack(worldrect.CenterX() - 35, worldrect.Bottom() - 30, 70, 16);

			//--- mouse selection on mouse click ---
			if (g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED)
			{
				//check chapters click
				RECTXYWH wndrectL = picrect;
				wndrectL.x += vpos.x - picrect.w / 2.0f;
				wndrectL.y += vpos.y - picrect.h / 2.0f;
				if (PointInRect(vLocalMousePos, wndrectL))
				{
					if (m_nSelection < 0)
						m_nSelection = m_nSelectionOld;
					else
						eCommand = K_CCTRLMGR_COMMAND_SELECT;
				}
				else if (vLocalMousePos.x > wndrectL.Right())
				{
					if (m_nSelection < 0)
						m_nSelection = m_nSelectionOld;
					else if ((m_nSelection < m_nSelElements - 1) && (m_nSelection >= 0))
						m_nSelection++;
				}
				else if (vLocalMousePos.x < wndrectL.x)
				{
					if (m_nSelection < 0)
						m_nSelection = m_nSelectionOld;
					else if (m_nSelection > 0)
						m_nSelection--;
				}
				//check buttons
				if (PointInRect(vLocalMousePos, rectButBack))
				{
					if (m_nSelection == -1)
						eCommand = K_CCTRLMGR_COMMAND_SELECT;
					else
						m_nSelectionOld = m_nSelection;

					m_nSelection = -1;
				}
			}

			//--- SELECTION COMMAND ---
			if (eCommand == K_CCTRLMGR_COMMAND_SELECT)
			{
				m_bSelectionMade = true;

				//back button
				if (m_nSelection == -1)
				{
					CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
					nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
					nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
					UTGetEventManager().QueueEvent(nevent);

					SND_PLAY(SNDIDX_DENIED);
					break;
				}

				//act on command
				if ((m_nSelection >= 0) && (m_nSelection < m_nSelElements))
				{
					switch (m_arrSelItems[m_nSelection])
					{
						//weekly challenge
						case K_MM_MODE_WEEKLY_CHALLENGE:
						{
							if (bIsCoopQM) //networked quick match
								break;

							g_gameMode = GAME_MODE_CLASSIC;
							g_userData[K_MEMID_SELECTED_CHAPTER] = K_GAME_WEEKLY_CHALLENGE_CHAPTER_NO; 

							CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
							nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_LEVEL_SELECTION);
							nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
							UTGetEventManager().QueueEvent(nevent);

							SND_PLAY(SNDIDX_CLICK);
						}
						break;

						//zombie mode
						case K_MM_MODE_ZOMBIE_INVASION:
						{
							SND_PLAY(SNDIDX_CLICK);

							g_gameMode = GAME_MODE_ZOMBIE_INVASION;
							//special case for target game modes
							if (m_eTargetGameState == GAME_STATE_JOIN_COOP_LIST)
							{
								CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
								nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_JOIN_COOP_LIST);
								nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
								UTGetEventManager().QueueEvent(nevent);
								break;
							}


							if (!bIsCoopQM)
							{
								CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
								nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_CHAPTER_SELECTION);
								nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
								UTGetEventManager().QueueEvent(nevent);
							}
							else  //networked game
							{
								CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
								nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_NET_LOBBY);
								nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
								nevent->AddNamedArgINT32(L"arg1", (int)UTGetAppClass().m_Settings.devnet_eNetGameType);
								UTGetEventManager().QueueEvent(nevent);
							}
						}
						break;

						case K_MM_MODE_VINFINITE:
						{
							SND_PLAY(SNDIDX_CLICK);

							g_gameMode = GAME_MODE_INFINITE_TOWER;
							//special case for target game modes
							if (m_eTargetGameState == GAME_STATE_JOIN_COOP_LIST)
							{
								CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
								nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_JOIN_COOP_LIST);
								nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
								UTGetEventManager().QueueEvent(nevent);
								break;
							}

							if (!UTGetAppClass().IsGameNetworked())
							{
								CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
								nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_PLAYER_SELECTION);
								nevent->AddNamedArgINT32(L"arg1", 1); //reset player selection
								nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
								UTGetEventManager().QueueEvent(nevent);
							}
							else  //networked quick match
							{
								CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
								nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_NET_LOBBY);
								nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
								nevent->AddNamedArgINT32(L"arg1", (int)UTGetAppClass().m_Settings.devnet_eNetGameType);
								UTGetEventManager().QueueEvent(nevent);
							}
						}
						break;

						//classic mode
						default:
						{
							SND_PLAY(SNDIDX_CLICK);
							g_gameMode = GAME_MODE_CLASSIC;
							//special case for target game modes
							if (m_eTargetGameState == GAME_STATE_JOIN_COOP_LIST)
							{
								CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
								nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_JOIN_COOP_LIST);
								nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
								UTGetEventManager().QueueEvent(nevent);
								break;
							}

							if (!bIsCoopQM)
							{
								CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
								nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_CHAPTER_SELECTION);
								nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
								UTGetEventManager().QueueEvent(nevent);
							}
							else  //networked quick match
							{
								CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
								nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_NET_LOBBY);
								nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
								nevent->AddNamedArgINT32(L"arg1", (int)UTGetAppClass().m_Settings.devnet_eNetGameType);
								UTGetEventManager().QueueEvent(nevent);
							}
						}
						break;
					}
				}
			}


			//float selection cursor
			if(m_nSelection >= 0)
				MATH_EaseTo_quadratic(&m_fSelPageCursor, (float)m_nSelection, 10.0f * dTime, 1.0f * dTime);
			//set selection rect
			if (m_nSelection >= 0)
			{
				RECTXYWH wndrectL = picrect; //local wndrect
				wndrectL.x += vpos.x - picrect.w / 2.0f;
				wndrectL.y += vpos.y - picrect.h / 2.0f;
				wndrectL.y += 5; wndrectL.h -= 4;
				wndrectL.x += 3; wndrectL.w -= 6;

				m_rectSelTarget = wndrectL;

				//first time entry check
				if ((m_rectSel.w == 0.0f) || (m_rectSel.h == 0.0f))
				{
					m_rectSel = m_rectSelTarget;
				}

			}
			else if (m_nSelection == -1) //buton back
			{
				m_rectSelTarget.Set(worldrect.CenterX() - 29, worldrect.Bottom() - 24, 58, 4); //hardcoded rect (smaller)
			}

			//morph current selection into target selection
			CAABB selaabb_from, selaabb_to;
			selaabb_from.Set(m_rectSel);
			selaabb_to.Set(m_rectSelTarget);

			AABB_MorphInto_quadratic(&selaabb_from, &selaabb_to, 20.0f * dTime, 60.0f * dTime);

			m_rectSel.Set(selaabb_from.vMin.x, selaabb_from.vMin.y, selaabb_from.vSize.x, selaabb_from.vSize.y);
		}
		break;

		case K_MM_STATE_CHAPTER_SELECT:
		{
			if (eCommand == K_CCTRLMGR_COMMAND_LEFT)
			{
				if (m_nSelection > 0)
					m_nSelection--;
			}
			else if (eCommand == K_CCTRLMGR_COMMAND_RIGHT)
			{
				if ((m_nSelection < m_nSelElements - 1) && (m_nSelection >= 0))
					m_nSelection++;
			}
			else if (eCommand == K_CCTRLMGR_COMMAND_DOWN)
			{
				m_nSelection = -1;
			}
			else if ((eCommand == K_CCTRLMGR_COMMAND_UP) && (m_nSelection == -1))
			{
				m_nSelection = m_nSelPage;
			}
			else if (eCommand == K_CCTRLMGR_COMMAND_BACK)
			{
				SND_PLAY(SNDIDX_DENIED);

				CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
				nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_GAME_MODE_SELECTION);
				nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
				UTGetEventManager().QueueEvent(nevent);
			}

			//--- MOUSE INPUT ---
			RECTXYWH_F worldrect = UTGetAppClass().g_rect240hWorld;
			RECTXYWH picrect = m_sprCol.GetAFrameBBox(ANM_MENUS_SPR_CHAPTER_SPLASHES_LG, 0);
			//facem loc pentru mesajele de sub imagine (trebuie sa corespunda cu cele din paint)
			RECTXYWH wndrect = picrect;
			wndrect.Inflate(-2, -2);
			wndrect.h += 24;
			int	wndSpacing = 24;

			D3DXVECTOR2 vpos = worldrect.Center();
			//dreptunghiul selectiei de buton back
			RECTXYWH rectButBack(worldrect.CenterX() - 35, worldrect.Bottom() - 30, 70, 16);

			//--- mouse selection on mouse click ---
			if (g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED)
			{
				//check chapters click
				RECTXYWH wndrectL = wndrect;
				wndrectL.x += vpos.x - picrect.w / 2.0f;
				wndrectL.y += vpos.y - wndrect.h / 2.0f;
				if (PointInRect(vLocalMousePos, wndrectL))
				{
					if (m_nSelection < 0)
					{
						m_nSelection = m_nSelPage;
					}
					else
					{
						eCommand = K_CCTRLMGR_COMMAND_SELECT;
					}
				}
				else if (vLocalMousePos.x > wndrectL.Right())
				{
					if (m_nSelection < 0)
					{
						m_nSelection = m_nSelPage;
					}
					if ((m_nSelection < m_nSelElements - 1) && (m_nSelection >= 0))
						m_nSelection++;
				}
				else if (vLocalMousePos.x < wndrectL.x)
				{
					if (m_nSelection < 0)
					{
						m_nSelection = m_nSelPage;
					}
					if (m_nSelection > 0)
						m_nSelection--;
				}
				//check buttons
				if (PointInRect(vLocalMousePos, rectButBack))
				{
					if (m_nSelection == -1)
					{
						eCommand = K_CCTRLMGR_COMMAND_SELECT;
					}
					m_nSelection = -1;
				}
			}

			//--- SELECTION COMMAND ---
			if (eCommand == K_CCTRLMGR_COMMAND_SELECT)
			{
				m_bSelectionMade = true;

				//act on command
				switch (m_nSelection)
				{
					case -1:  //back button
					{
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_GAME_MODE_SELECTION);
						nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
						UTGetEventManager().QueueEvent(nevent);

						SND_PLAY(SNDIDX_DENIED);
					}
					break;
					default:
					{
						//not enough missions played so you can't enter (arrChapterData[n*3+2] is number of necessary missions
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
						if (m_nSelection != g_userData[K_MEMID_SELECTED_CHAPTER])
						{
							g_userData[K_MEMID_SELECTED_LEVEL] = 0;
							g_userData[K_MEMID_SELECTED_CHAPTER] = m_nSelection;
						}

						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_LEVEL_SELECTION);
						nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
						UTGetEventManager().QueueEvent(nevent);

						SND_PLAY(SNDIDX_CLICK);
#else
						if (!UTGetChaptersList().IsChapterUnlocked(m_nSelection, g_userData[K_MEMID_MISSIONS_COMPLETED]))
						{
							SND_PLAY(SNDIDX_DENIED);
							break;
						}

						//default to first level when changing chapters
						if (m_nSelection != g_userData[K_MEMID_SELECTED_CHAPTER])
						{
							g_userData[K_MEMID_SELECTED_LEVEL] = 0;
							g_userData[K_MEMID_SELECTED_CHAPTER] = m_nSelection;
						}

						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_LEVEL_SELECTION);
						nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
						UTGetEventManager().QueueEvent(nevent);

						SND_PLAY(SNDIDX_CLICK);
#endif
					}
					break;
				}
			}

			//--- selection page ---
			if (m_nSelection >= 0)
			{
				m_nSelPage = m_nSelection;
			}
			MATH_EaseTo_quadratic(&m_fSelPageCursor, (float)m_nSelPage, 10.0f * dTime, 1.0f * dTime);

			//set selection rect
			if (m_nSelection >= 0)
			{
				RECTXYWH wndrectL = wndrect; //local wndrect
				wndrectL.x += worldrect.Center().x - picrect.w / 2.0f; // vpos.x + m_nSelection * (picrect.w + wndSpacing)*/ - picrect.w / 2.0f;
				wndrectL.y += vpos.y - wndrect.h / 2.0f;

				m_rectSelTarget = wndrectL;

				//first time entry check
				if ((m_rectSel.w == 0.0f) || (m_rectSel.h == 0.0f))
				{
					m_rectSel = m_rectSelTarget;
				}

			}
			else if (m_nSelection == -1) //buton back
			{
				m_rectSelTarget.Set(worldrect.CenterX() - 30, worldrect.Bottom() - 25, 60, 6); //hardcoded rect (smaller)
			}

			//morph current selection into target selection
			CAABB selaabb_from, selaabb_to;
			selaabb_from.Set(m_rectSel);
			selaabb_to.Set(m_rectSelTarget);

			AABB_MorphInto_quadratic(&selaabb_from, &selaabb_to, 20.0f * dTime, 60.0f * dTime);

			m_rectSel.Set(selaabb_from.vMin.x, selaabb_from.vMin.y, selaabb_from.vSize.x, selaabb_from.vSize.y);
		}
		break;

		case K_MM_STATE_LEVEL_SELECT:
		{
			//--- compute useful data ---
			RECTXYWH_F worldrect = UTGetAppClass().g_rect240hWorld;
			int nSelPerPage = m_nSelRows * m_nSelColumns;
			//dimensiuni ferestre nivel si spacing intre ele
			SIZEWH wndSz(48, 36), wndSpacing(8, 8), wndSzTotal(48 + 8, 36 + 8);
			int nPage = m_nSelection / nSelPerPage;
			int nRow = (m_nSelection % nSelPerPage) / m_nSelColumns;
			int nColumn = (m_nSelection % nSelPerPage) % m_nSelColumns;
			RECTXYWH rectRightPanel(worldrect.CenterX() - 160 + 100, 20, 220, worldrect.h - 20);

			//--- mouse input ---
			if (g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED)
			{
				RECTXYWH pageRect(rectRightPanel.CenterX() - 0.5f * m_nSelColumns * wndSzTotal.w, rectRightPanel.CenterY() - 0.5f * m_nSelRows * wndSzTotal.h - 8, m_nSelColumns * wndSzTotal.w, m_nSelRows * wndSzTotal.h);
				D3DXVECTOR2 vRefPos(pageRect.x, pageRect.y);
				vRefPos.x -= m_fSelPageCursor * pageRect.w;
				//verifica nivelele
				for (int kk = 0; kk < m_nSelElements; kk++)
				{
					int nPage = kk / nSelPerPage;
					int nRow = (kk % nSelPerPage) / m_nSelColumns;
					int nColumn = (kk % nSelPerPage) % m_nSelColumns;

					float wndAlpha = 1.0f;

					wndAlpha = 1.0f - fabs((float)nPage - m_fSelPageCursor);
					CLAMP(wndAlpha, 0.0f, 1.0f);
					//daca nu se vede nu deseneazza
					if (wndAlpha <= 0.0f)
						continue;

					D3DXVECTOR2 vRefPos(pageRect.x, pageRect.y);
					vRefPos.x -= m_fSelPageCursor * pageRect.w;
					RECTXYWH lvlrect(vRefPos.x + nColumn * wndSzTotal.w + nPage * pageRect.w + wndSpacing.w / 2, vRefPos.y + nRow * wndSzTotal.h + wndSpacing.h / 2, wndSz.w, wndSz.h);

					if (PointInRect(vLocalMousePos, lvlrect))
					{
						if (m_nSelection == kk)
						{
							eCommand = K_CCTRLMGR_COMMAND_SELECT;
						}

						m_nSelection = kk;
						break;
					}
				}
				//page left
				if (vLocalMousePos.x < pageRect.x)
				{
					if (m_nSelPage > 0)
					{
						m_nSelection -= nSelPerPage;
					}
				}
				//page right
				else if (vLocalMousePos.x > pageRect.Right())
				{
					if (m_nSelPage < m_nSelPagesCnt - 1)
					{
						m_nSelection += nSelPerPage;
					}
				}
				//back button
				RECTXYWH recttemp(rectRightPanel.CenterX() - 35, rectRightPanel.Bottom() - 30, 70, 16);
				if (PointInRect(vLocalMousePos, recttemp))
				{
					if (m_nSelection == -1)
					{
						eCommand = K_CCTRLMGR_COMMAND_SELECT;
					}
					m_nSelection = -1;
				}
			}


			//--- handle keyboard input ---
			if (eCommand == K_CCTRLMGR_COMMAND_LEFT)
			{
				if (m_nSelection >= 0)
				{
					m_nSelection--;
					if (m_nSelection < 0)
						m_nSelection = m_nSelElements - 1;
					//multipage:
					/*
					if (nColumn == 0)
					{
						if (m_nSelPage > 0)
							m_nSelection -= nSelPerPage - 1;
					}
					else
						m_nSelection--;
						*/
				}
			}
			else if (eCommand == K_CCTRLMGR_COMMAND_RIGHT)
			{
				if (m_nSelection >= 0)
				{
					m_nSelection++;
					if (m_nSelection >= m_nSelElements)
						m_nSelection = 0;
				}
				//multipage:
				/*
				if ((m_nSelection >= 0) && (m_nSelection < m_nSelElements - 1))
				{
					if (nColumn == m_nSelColumns - 1)
					{
						if(m_nSelPage < m_nSelPagesCnt - 1)
							m_nSelection += nSelPerPage - 1;
					}
					else
						m_nSelection++;
				}
				*/
			}
			else if (eCommand == K_CCTRLMGR_COMMAND_DOWN)
			{
				if (m_nSelection >= 0)
				{
					if (m_nSelection % nSelPerPage < nSelPerPage - m_nSelColumns)
					{
						m_nSelection += m_nSelColumns;
					}
					else
					{
						m_nSelectionOld = m_nSelection;
						m_nSelection = -1; //buton back
					}
				}
			}
			else if (eCommand == K_CCTRLMGR_COMMAND_UP)
			{
				if (m_nSelection == -1) //buton back
				{
					m_nSelection = m_nSelectionOld;
				}
				else
				{
					if (m_nSelection % nSelPerPage > m_nSelColumns - 1)
					{
						m_nSelection -= m_nSelColumns;
					}
				}
			}
			else if (eCommand == K_CCTRLMGR_COMMAND_BACK)
			{
				SND_PLAY(SNDIDX_DENIED);

				CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
				nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_CHAPTER_SELECTION);
				nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
				UTGetEventManager().QueueEvent(nevent);
			}
			else if (eCommand == K_CCTRLMGR_COMMAND_SELECT)
			{
				m_bSelectionMade = true;

				//back button
				if (m_nSelection == -1)
				{
					SND_PLAY(SNDIDX_DENIED);

					CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
					nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_CHAPTER_SELECTION);
					nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
					UTGetEventManager().QueueEvent(nevent);
				}
				else
				{
					//see if we can select this level
					int nRealLevelIdx = m_nSelection + g_userData[K_MEMID_SELECTED_CHAPTER] * K_GAME_LEVELS_PER_CHAPTER;
					//do we have a level?
					bool bCanStart = UTGetChaptersList().IsValidLevel(g_userData[K_MEMID_SELECTED_CHAPTER], m_nSelection);
					//check scanned type of level too (missing?)
					int nLevelType = g_levelStats[nRealLevelIdx].nLevelType;
					if (nLevelType < 0)
						bCanStart = false;

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
					//on debug or developer mode always enter level
					bCanStart = true;
#endif
					if (!bCanStart)
					{
						SND_PLAY(SNDIDX_DENIED);
					}
					else
					{
						SND_PLAY(SNDIDX_CLICK);
						//save selected level
						g_userData[K_MEMID_SELECTED_LEVEL] = m_nSelection;

						if (!UTGetAppClass().IsGameNetworked())
						{
							CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
							nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_PLAYER_SELECTION);
							nevent->AddNamedArgINT32(L"arg1", 1); //reset player selection
							nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
							UTGetEventManager().QueueEvent(nevent);
						}
						else  //networked game is hosted 
						{
							CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
							nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_NET_LOBBY);
							nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
							nevent->AddNamedArgINT32(L"arg1", (int)UTGetAppClass().m_Settings.devnet_eNetGameType);
							UTGetEventManager().QueueEvent(nevent);
						}
					}
				}
			}

			//#HACK: info button - should be on commands
#ifdef ENABLE_LEADERBOARDS
			//show leaderboard when pressing melee key (any controller)
			if (UTGetCtrlrMgr().KeyPressed(K_CM_COMMAND_MELEE))
			{
				CCtrlLayer* lay = UTGetControlsManager().GetLayerByName("LAYER_ID_LEADERBOARDS_LVL");
				if (lay == null)
				{
					int nChapter = g_userData[K_MEMID_SELECTED_CHAPTER];
					int nLevel = m_nSelection;

					if (RequestLeaderboardsUpdate(false))
					{
						///--- show layer ---
						lay = UTGetControlsManager().ShowLayerOnce("LAYER_ID_LEADERBOARDS_LVL");
						if (lay)
						{
							//level name in STR_TEMP10
							if (UTGetChaptersList().IsValidLevel(nChapter, nLevel))
							{
								int nStrIdxLevelName = UTGetChaptersList().m_arrChapters[nChapter]->arrLevelNameStrIdx[nLevel];
								g_stringsMgr.SetString(STR_TEMP10, L"%d.%d %s", nChapter + 1, nLevel + 1, g_stringsMgr.strings[nStrIdxLevelName]->sText);
							}
							else
								g_stringsMgr.SetString(STR_TEMP10, L"");

							//reset scroll page and save leaderboard index as a payload in this control
							// set scores list on empty
							CControl *ctrl = lay->GetControlByName("CTRL_SCORESLIST_TT");
							if (ctrl != null)
							{
								ctrl->paramsDict.SetNamedVarINT32(L"nOptionsCnt", 0);
								ctrl->paramsDict.SetNamedVarINT32(L"nPage", 0);
								ctrl->paramsDict.SetNamedVarINT32(L"nLeaderboardID", 1); //Multiplayer
#ifndef ENABLE_LEADERBOARDS_NAMES_SELECTION
								ctrl->bCanHaveFocus = false;
								ctrl->paramsDict.SetNamedVarBool(L"bUserCanSelect", false);
#endif
							}
						}
					}
					else
					{
						SND_PLAY_ONCE(SNDIDX_DENIED, 0);
					}
				}
			}
#endif

			//--- update page scroller cursor ---
			if (m_nSelection >= 0)
			{
				m_nSelPage = m_nSelection / nSelPerPage;
				//update data after selection
				nPage = m_nSelection / nSelPerPage;
				nRow = (m_nSelection % nSelPerPage) / m_nSelColumns;
				nColumn = (m_nSelection % nSelPerPage) % m_nSelColumns;
			}

			MATH_EaseTo_quadratic(&m_fSelPageCursor, (float)m_nSelPage, 10.0f * dTime, 1.0f * dTime);
			//--- update selection ---
			if (m_nSelection >= 0)
			{
				RECTXYWH pageRect(rectRightPanel.CenterX() - 0.5f * m_nSelColumns * wndSzTotal.w, rectRightPanel.CenterY() - 0.5f * m_nSelRows * wndSzTotal.h - 8, m_nSelColumns * wndSzTotal.w, m_nSelRows * wndSzTotal.h);
				D3DXVECTOR2 vRefPos(pageRect.x, pageRect.y);
				vRefPos.x -= m_fSelPageCursor * pageRect.w;

				m_rectSelTarget.Set(vRefPos.x + nColumn * wndSzTotal.w + nPage * pageRect.w + wndSpacing.w / 2, vRefPos.y + nRow * wndSzTotal.h + wndSpacing.h / 2, wndSz.w, wndSz.h);
				//first time entry check
				if ((m_rectSel.w == 0.0f) || (m_rectSel.h == 0.0f))
				{
					m_rectSel = m_rectSelTarget;
				}
			}
			else  //back button
			{
				RECTXYWH recttemp(rectRightPanel.CenterX() - 34, rectRightPanel.Bottom() - 29, 68, 14);
				m_rectSelTarget = recttemp;
			}
			//--- morph current selection into target selection ---
			CAABB selaabb_from, selaabb_to;
			selaabb_from.Set(m_rectSel);
			selaabb_to.Set(m_rectSelTarget);
			AABB_MorphInto_quadratic(&selaabb_from, &selaabb_to, 20.0f * dTime, 60.0f * dTime);

			m_rectSel.Set(selaabb_from.vMin.x, selaabb_from.vMin.y, selaabb_from.vSize.x, selaabb_from.vSize.y);
		}
		break;

		case K_MM_STATE_DOWNLOADED_LEVEL_SELECT:
		{
			//--- compute useful data ---
			RECTXYWH_F worldrect = UTGetAppClass().g_rect240hWorld;
			int nSelPerPage = m_nSelRows * m_nSelColumns;
			//dimensiuni ferestre nivel si spacing intre ele
			SIZEWH wndSz(48, 36), wndSpacing(8, 8), wndSzTotal(48 + 8, 36 + 8);
			int nPage = m_nSelection / nSelPerPage;
			int nRow = (m_nSelection % nSelPerPage) / m_nSelColumns;
			int nColumn = (m_nSelection % nSelPerPage) % m_nSelColumns;
			RECTXYWH rectItemsPanel(worldrect.CenterX() - 160, 110, 320, worldrect.h - 110);

			m_fSelTimer += dTime;

			//--- mouse input ---
			if (g_mouse.Lbut == K_MOUSE_BUTT_JUSTPRESSED)
			{
				RECTXYWH pageRect(rectItemsPanel.CenterX() - 0.5f * m_nSelColumns * wndSzTotal.w, rectItemsPanel.y, m_nSelColumns * wndSzTotal.w, m_nSelRows * wndSzTotal.h);
				D3DXVECTOR2 vRefPos(pageRect.x, pageRect.y);
				vRefPos.x -= m_fSelPageCursor * pageRect.w;
				//verifica nivelele
				for (int kk = 0; kk < m_nSelElements; kk++)
				{
					int nPage = kk / nSelPerPage;
					int nRow = (kk % nSelPerPage) / m_nSelColumns;
					int nColumn = (kk % nSelPerPage) % m_nSelColumns;

					float wndAlpha = 1.0f;

					wndAlpha = 1.0f - fabs((float)nPage - m_fSelPageCursor);
					CLAMP(wndAlpha, 0.0f, 1.0f);
					//daca nu se vede nu deseneazza
					if (wndAlpha <= 0.0f)
						continue;

					D3DXVECTOR2 vRefPos(pageRect.x, pageRect.y);
					vRefPos.x -= m_fSelPageCursor * pageRect.w;
					RECTXYWH lvlrect(vRefPos.x + nColumn * wndSzTotal.w + nPage * pageRect.w + wndSpacing.w / 2, vRefPos.y + nRow * wndSzTotal.h + wndSpacing.h / 2, wndSz.w, wndSz.h);

					if (PointInRect(vLocalMousePos, lvlrect))
					{
						if (m_nSelection == kk)
						{
							eCommand = K_CCTRLMGR_COMMAND_SELECT;
						}

						m_nSelection = kk;
						m_fSelTimer = 0.0f;
						break;
					}
				}
				//page left
				if ((vLocalMousePos.x < pageRect.x) && (vLocalMousePos.y > pageRect.y) && (vLocalMousePos.y < pageRect.Bottom()))
				{
					if (m_nSelPage > 0)
					{
						m_nSelection -= nSelPerPage;
						m_fSelTimer = 0.0f;
					}
				}
				//page right
				else if ((vLocalMousePos.x > pageRect.Right()) && (vLocalMousePos.y > pageRect.y) && (vLocalMousePos.y < pageRect.Bottom()))
				{
					if (m_nSelPage < m_nSelPagesCnt - 1)
					{
						m_nSelection += nSelPerPage;
						if (m_nSelection > m_nSelElements - 1)
							m_nSelection = m_nSelElements - 1;
						m_fSelTimer = 0.0f;
					}
				}
				//back button
				RECTXYWH recttemp(rectItemsPanel.CenterX() - 35, rectItemsPanel.Bottom() - 30, 70, 16);
				if (PointInRect(vLocalMousePos, recttemp))
				{
					if (m_nSelection == -1)
					{
						eCommand = K_CCTRLMGR_COMMAND_SELECT;
					}
					m_nSelection = -1;
				}
			}


			//--- handle keyboard input ---
			if (eCommand == K_CCTRLMGR_COMMAND_LEFT)
			{
				if (m_nSelection >= 0)
				{
					if (nColumn == 0)
					{
						if (m_nSelPage > 0)
							m_nSelection -= nSelPerPage - 1;
					}
					else
					{
						m_nSelection--;
					}
					m_fSelTimer = 0.0f;
				}
			}
			else if (eCommand == K_CCTRLMGR_COMMAND_RIGHT)
			{
				if ((m_nSelection >= 0) && (m_nSelection < m_nSelElements - 1))
				{
					if (nColumn == m_nSelColumns - 1)
					{
						if (m_nSelPage < m_nSelPagesCnt - 1)
						{
							m_nSelection += nSelPerPage - 1;
							if (m_nSelection > m_nSelElements - 1)
								m_nSelection = m_nSelElements - 1;
						}
					}
					else
					{
						m_nSelection++;
					}
					m_fSelTimer = 0.0f;
				}
			}
			else if (eCommand == K_CCTRLMGR_COMMAND_DOWN)
			{
				if (m_nSelection >= 0)
				{
					//on the bottom row or when pressing down has no corresponding element
					if ((m_nSelection % nSelPerPage < nSelPerPage - m_nSelColumns) && (m_nSelection + m_nSelColumns < m_nSelElements - 1))
					{
						m_nSelection += m_nSelColumns;
						if (m_nSelection > m_nSelElements - 1)
							m_nSelection = m_nSelElements - 1;
					}
					else
					{
						m_nSelectionOld = m_nSelection;
						m_nSelection = -1; //buton back
					}
					m_fSelTimer = 0.0f;
				}
			}
			else if (eCommand == K_CCTRLMGR_COMMAND_UP)
			{
				//back button
				if (m_nSelection == -1)
				{
					if(m_nSelElements > 0) 
						m_nSelection = m_nSelectionOld;
				}
				else
				{
					if (m_nSelection % nSelPerPage > m_nSelColumns - 1)
					{
						m_nSelection -= m_nSelColumns;
					}
				}
				m_fSelTimer = 0.0f;
			}
			else if (eCommand == K_CCTRLMGR_COMMAND_BACK)
			{
				SND_PLAY(SNDIDX_DENIED);

				CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
				nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_CHAPTER_SELECTION);
				nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
				UTGetEventManager().QueueEvent(nevent);
			}
			else if (eCommand == K_CCTRLMGR_COMMAND_SELECT)
			{
				m_bSelectionMade = true;

				//back button
				if (m_nSelection == -1)
				{
					SND_PLAY(SNDIDX_DENIED);

					CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
					nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_CHAPTER_SELECTION);
					nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
					UTGetEventManager().QueueEvent(nevent);
					//reset mod selection
					g_userData[K_MEMID_MOD_DWNLVL_SELECTED] = -1;
				}
				else
				{
					SND_PLAY(SNDIDX_CLICK);
					//save selected mod level index
					g_userData[K_MEMID_MOD_DWNLVL_SELECTED] = m_arrSelItems[m_nSelection * 3 + 2];

					//play selected level
					if (!UTGetAppClass().IsGameNetworked())
					{
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_PLAYER_SELECTION);
						nevent->AddNamedArgINT32(L"arg1", 1); //reset player selection
						nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
						UTGetEventManager().QueueEvent(nevent);
					}
					else  //networked game is hosted 
					{
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_NET_LOBBY);
						nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
						nevent->AddNamedArgINT32(L"arg1", (int)UTGetAppClass().m_Settings.devnet_eNetGameType);
						UTGetEventManager().QueueEvent(nevent);
					}
				}
			}

			//--- update page scroller cursor ---
			if (m_nSelection >= 0)
			{
				m_nSelPage = m_nSelection / nSelPerPage;
				//update data after selection
				nPage = m_nSelection / nSelPerPage;
				nRow = (m_nSelection % nSelPerPage) / m_nSelColumns;
				nColumn = (m_nSelection % nSelPerPage) % m_nSelColumns;
			}

			MATH_EaseTo_quadratic(&m_fSelPageCursor, (float)m_nSelPage, 10.0f * dTime, 1.0f * dTime);
			//--- update selection ---
			if (m_nSelection >= 0)
			{
				RECTXYWH pageRect(rectItemsPanel.CenterX() - 0.5f * m_nSelColumns * wndSzTotal.w, rectItemsPanel.y, m_nSelColumns * wndSzTotal.w, m_nSelRows * wndSzTotal.h);
				D3DXVECTOR2 vRefPos(pageRect.x, pageRect.y);
				vRefPos.x -= m_fSelPageCursor * pageRect.w;

				m_rectSelTarget.Set(vRefPos.x + nColumn * wndSzTotal.w + nPage * pageRect.w + wndSpacing.w / 2, vRefPos.y + nRow * wndSzTotal.h + wndSpacing.h / 2, wndSz.w, wndSz.h);
			}
			else  //back button
			{
				RECTXYWH recttemp(rectItemsPanel.CenterX() - 34, rectItemsPanel.Bottom() - 29, 68, 14);
				m_rectSelTarget = recttemp;
			}
			//first selection size time entry check
			if ((m_rectSel.w == 0.0f) || (m_rectSel.h == 0.0f))
			{
				m_rectSel = m_rectSelTarget;
			}
			//--- morph current selection into target selection ---
			CAABB selaabb_from, selaabb_to;
			selaabb_from.Set(m_rectSel);
			selaabb_to.Set(m_rectSelTarget);
			AABB_MorphInto_quadratic(&selaabb_from, &selaabb_to, 20.0f * dTime, 60.0f * dTime);

			m_rectSel.Set(selaabb_from.vMin.x, selaabb_from.vMin.y, selaabb_from.vSize.x, selaabb_from.vSize.y);
		}
		break;

	}

	//now update particles
	g_particlesMgr.Update(dTime);
}


void CMainMenu::Paint()
{
	//setam ecranul standard de 240h inaltime
	CCameraTransform::SetActiveCamera(m_pDevice, &UTGetAppClass().g_cam240hScreen);
	App_SetWorldTransform(m_pDevice, &g_matIdentity);

	RECTXYWH_F scrrect = UTGetAppClass().g_cam240hScreen.GetCamWorldAABB();
	RECTXYWH_F worldrect = UTGetAppClass().g_rect240hWorld;

	switch (m_eState)
	{
		case K_MM_STATE_WORKSHOP:
		{
			PaintMainBackground(worldrect, 0xff4444dd);

#ifdef ENABLE_STEAM_WORKSHOP
			switch (m_nSubstate)
			{
				//show "updating mods"
				case 0:
				case 1:
				{
					//title bar
					RECTXYWH rRect(0, 20, 100, 18);
					CtrlMgrDrawWidebar(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_WIDEBAR2, rRect, 0xffffffff);
					g_font8bs1->DrawString(STR_UPDATING_MODS, worldrect.CenterX(), worldrect.y + 33.0f, FONTFLAG_ANCHOR_BOTTOMCENTER, K_COLOR_SELECTED_TEXT);
					//please wait
					g_font8bs1->DrawString(STR_PLEASE_HANG, worldrect.CenterX(), worldrect.CenterY(), FONTFLAG_ANCHOR_VCENTERHCENTER, K_COLOR_DEFAULT_TEXT);
				}
				break;
				//mods selection
				case 2:
				{
					//CTTFont* pTTFont = UTGetTTFManager().GetFont(shTTFID_SZ20.textHash);
					//if ((pTTFont == null) || (pTTFont->pFont == null))
					//{
					//	ErrorBox(K_ERR_ONSCREEN, L"True type font not loaded!");

					//	CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
					//	nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
					//	nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
					//	UTGetEventManager().QueueEvent(nevent);

					//	break;
					//}

					const int nItemSpacing = 10;
					SIZEWH szItem(200, 10);
					//items list area
					RECTXYWH rectItemsList(worldrect.CenterX() - szItem.w / 2, worldrect.Bottom() - 26 - m_nSelRows * (szItem.h + nItemSpacing), szItem.w, m_nSelRows * (szItem.h + nItemSpacing) - nItemSpacing);
					//selected mod details area
					RECTXYWH rectSelMod(worldrect.CenterX() - 150, worldrect.y + 50, 300, 52);
					//title
					RECTXYWH rRect(0, 20, 100, 18);
					CtrlMgrDrawWidebar(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_WIDEBAR2, rRect, 0xffffffff);
					g_font8bs1->DrawString(STR_AVAILABLE_MODS, worldrect.CenterX(), worldrect.y + 33.0f, FONTFLAG_ANCHOR_BOTTOMCENTER, K_COLOR_SELECTED_TEXT);

					//mod list background
					RECTXYWH rectTemp = rectItemsList;
					rectTemp.Inflate(2, 2);
					CtrlMgrDrawFrame(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_FRAME1, rectTemp, 0x88888888);

					//paint selection cursor
					RECTXYWH selrect;
					if ((m_rectSel.w > 0.0f) && (m_rectSel.h > 0.0f))
					{
						selrect.x = m_rectSel.x; selrect.y = m_rectSel.y;
						selrect.w = m_rectSel.w; selrect.h = m_rectSel.h;
						selrect.Inflate(5, 5);
						CtrlMgrDrawFrame(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_FRAME5, selrect, 0xffffffff);
					}

					///--- SELECTED MOD ---
					CtrlMgrDrawFrame(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_FRAME1, rectSelMod, 0x88888888);
					//image
					if ((m_nSelection >= 0) && (m_nSelection < m_nSelElements))
					{
						if (m_arrSelItems[m_nSelection] >= 0)
						{
							RECT rcImg;
							TexNode* pTex = UTGetAppClass().g_texManager.GetTextureNode(m_arrSelItems[m_nSelection]);
							if (pTex)
							{
								SetRect(&rcImg, 0, 0, pTex->widthToLoad, pTex->heightToLoad);
								m_pSprite->Draw(pTex->pTexture, &rcImg, NULL, &D3DXVECTOR3(rectSelMod.x - 1, rectSelMod.y - 1, 0.0f), 0xffffffff);
							}
						}

						CModsManager::CModDescriptor *mod = UTGetModsManager().m_arrMods[m_nSelection];
						//title again
						CStringDesc sdModName;
						g_stringsMgr.SetStringDesc(&sdModName, mod->shName.text);
						WCHAR strCompleteDesc[2048];
						StringCchPrintf(strCompleteDesc, 2048, L"%s\n- %s", mod->strDescription, mod->strAuthor);
						g_stringsMgr.SetString(STR_TEMP12, strCompleteDesc);
						rectTemp = rectSelMod; rectTemp.x += 100; rectTemp.w -= 100; rectTemp.y += 20; rectTemp.h -= 18;
						g_font8bs1->DrawString(&sdModName, rectTemp.x, rectTemp.y - 10, FONTFLAG_ANCHOR_VCENTERLEFT, K_COLOR_SELECTED_TEXT);

						int texth = g_font6n1->MeasureString(STR_TEMP12, rectTemp.w).h;
						if (texth <= rectTemp.h)
						{
							g_font6n1->DrawString(STR_TEMP12, rectTemp, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
						}
						else
						{
							float fTmMul = 8.0f / (float)(texth - rectTemp.h);
							float offy = LIMIT((float)sin((-1.0f + m_fSelTimer) * fTmMul), 0.0f, 1.0f);
							offy *= -(texth - rectTemp.h);
							g_font6n1->DrawStringOffsetY(STR_TEMP12, rectTemp, (int)floor(offy), FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT | FONTFLAG_CLIPTEXT, K_COLOR_DEFAULT_TEXT);
						}
					}
					///--- MODS LIST ---

					//paint mods
					for (int kk = 0; kk < m_nSelRows; kk++)
					{
						int nModIdx = kk + m_nSelPage * m_nSelRows;
						if (nModIdx >= m_nSelElements)
							continue;
						CModsManager::CModDescriptor *mod = UTGetModsManager().GetModDescByIndex(nModIdx);
						if (mod == null)
							continue;

						float fColor = 1.0f;

						RECTXYWH rectItem(rectItemsList.x, rectItemsList.y + (szItem.h + nItemSpacing) * kk, szItem.w, szItem.h);

						float fDark = 1.0f - fabs((float)kk - m_fSelPageCursor);
						CLAMP(fDark, 0.0f, 1.0f);
						
						CtrlMgrDrawFrame(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_FRAME1, rectItem, D3DCOLOR_COLORVALUE(fColor, fColor, fColor, 1.0f));
						//mod name
						RECT txtrect;
						SetRect(&txtrect, rectItem.x, rectItem.y, rectItem.Right(), rectItem.Bottom());
						//icon
						if(mod->eType == CModsManager::K_MOD_TYPE_SINGLE_LEVEL)
							CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectItem.x - 2, rectItem.CenterY(), ANM_CONTROLS_SPR_ICONS_MISC, 4, D3DCOLOR_FFFA(fColor));
						else
							CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectItem.x - 2, rectItem.CenterY(), ANM_CONTROLS_SPR_ICONS_MISC, 5, D3DCOLOR_FFFA(fColor));

						CStringDesc sdModName;
						g_stringsMgr.SetStringDesc(&sdModName, mod->shName.text);
						RECTXYWH recttemp(rectItem.x, rectItem.y - 10, rectItem.w - 30, rectItem.h + 20);
						g_font8bs1->DrawStringClipped(&sdModName, rectItem.x + 14, rectItem.CenterY(), recttemp, FONTFLAG_ANCHOR_VCENTERLEFT, D3DCOLOR_COLORALPHA(K_COLOR_DEFAULT_TEXT, fColor));
						//enabled?
						if(mod->bActive)
							g_font8bs1->DrawString(STR_ON, rectItem.Right(), rectItem.CenterY(), FONTFLAG_ANCHOR_VCENTERRIGHT, 0xff66ff66);
						else
							g_font8bs1->DrawString(STR_OFF, rectItem.Right(), rectItem.CenterY(), FONTFLAG_ANCHOR_VCENTERRIGHT, 0xffff6666);
					}
					//no mods...
					if (m_nSelElements == 0)
					{
						g_font8bs1->DrawString(STR_NO_MODS_SUBSCRIBED, rectItemsList, FONTFLAG_ANCHOR_VCENTERHCENTER | FONTFLAG_WRAPTEXT, K_COLOR_SELECTED_TEXT);
					}
					//paint scroll arrows - when not on back button
					if (m_nSelPagesCnt > 1)
					{
						float offx = 10.0f + sin(fLocalTimeline * 5.0f);
						if (m_nSelPage > 0)
							CSprite::paintFrame(&m_sprCol, rectItemsList.CenterX() - 0.5f * rectItemsList.w - offx, rectItemsList.CenterY(), ANM_MENUS_SPR_ARROWS_LARGE, 0, 0xffffffff);
						if (m_nSelPage < m_nSelPagesCnt - 1)
							CSprite::paintFrame(&m_sprCol, rectItemsList.CenterX() + 0.5f * rectItemsList.w + offx, rectItemsList.CenterY(), ANM_MENUS_SPR_ARROWS_LARGE, 1, 0xffffffff);
					}
					//paint paging
					if (m_nSelPagesCnt > 1)
					{
						rectTemp = rectItemsList;
						rectTemp.h += 7; rectTemp.w += 7;
						CtrlMgrDrawPageSelector(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_PAGE_SELECTOR_L, rectTemp, m_nSelPagesCnt, m_nSelPage, 0xffffffff, 1);
					}

					//buton back
					RECTXYWH recttemp(worldrect.CenterX() - 35, worldrect.Bottom() - 25, 70, 16);
					int butframe = 0;
					if (m_nSelection == -1) //daca am hover pe buton BACK
					{
						butframe = 3;
						if (m_bSelectionMade)
							butframe = 6;
					}
					CtrlMgrDrawHTilingAnim(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_BUTTON1, butframe, recttemp, 0xffffffff);
					//textul apasat pe butonul de back
					if ((!m_bSelectionMade) && (m_nSelection == -1))
						recttemp.y -= 1;
					g_font8bs1->DrawString(STR_BACK, recttemp, FONTFLAG_ANCHOR_VCENTERHCENTER, 0xffffffff);
				}
				break;
			}
#endif
		}
		break;

		case K_MM_STATE_NET_LOBBY:
		{
			PaintMainBackground(worldrect, 0xff4444dd);

			//show mod enabled message
			if (UTGetAppClass().IsGameModded())
			{
				g_font10bs1->DrawString(STR_MOD_ENABLED, scrrect.CenterX(), 15.0f, FONTFLAG_ANCHOR_TOPCENTER, K_COLOR_DEFAULT_TEXT);
				g_font6ns1->DrawString(STR_MOD_ENABLED_WARNING, scrrect.CenterX(), 27.0f, FONTFLAG_ANCHOR_TOPCENTER, K_COLOR_DEFAULT_TEXT);
			}
		}
		break;

		case K_MM_STATE_MAINMENU:
		{
			PaintMainBackground(worldrect, 0xffffffff, true);

			//linear sampling
			m_pSprite->Flush();
			m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
			m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

			//particule foc
			g_particlesMgr.PaintLayer(K_PART_LAYER_FRONT_LIGHT, true);

			m_pSprite->Flush();
			m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

			//paint logo
			CSprite::paintFrame(&m_sprCol, scrrect.Right(), scrrect.y, ANM_MENUS_SPR_TITLE, 0);
			//paint logo bling
			CSprite spr(ANM_MENUS_SPR_TITLE, scrrect.Right(), scrrect.y);
			spr.currentFrame = 1;
			CSprite spr2 = spr;

			//float fAlpha = (2.0f * PerlinNoise1D(fLocalTimeline, 5.0f, 2.0f, 0.6f, 0.5f, 2)) - 1.0f;
			float fAlpha = (((1.0f + sin(fLocalTimeline)) * 5.0f) - 9.0f);
			CLAMP(fAlpha, 0.0f, 1.0f);
			spr2.color = D3DCOLOR_FFFA(fAlpha * 0.7f);
			spr2.paint(&m_sprCol);
			AdditiveBlendingOFF(m_pDevice, m_pSprite);
		}
		break;

		case K_MM_STATE_GAME_MODE_SELECT:
		{
			PaintMainBackground(worldrect, 0xff4444dd);
			//title
			RECTXYWH rRect(0, 22, 100, 18);
			CtrlMgrDrawWidebar(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_WIDEBAR2, rRect, 0xffffffff);
			g_font8bs1->DrawString(STR_SELECT_GAME_MODE, worldrect.CenterX(), worldrect.y + 35.0f, FONTFLAG_ANCHOR_BOTTOMCENTER, K_COLOR_SELECTED_TEXT);
			//coop string if online game
			if (UTGetAppClass().IsGameNetworked())
				g_font8b1->DrawString(STR_COOP, worldrect.Right() - 3.0f, worldrect.y + 13.0f, FONTFLAG_ANCHOR_BOTTOMRIGHT, K_COLOR_DEFAULT_TEXT_HALFALPHA);
			else
				g_font8b1->DrawString(STR_LOCAL_GAME, worldrect.Right() - 3.0f, worldrect.y + 13.0f, FONTFLAG_ANCHOR_BOTTOMRIGHT, K_COLOR_DEFAULT_TEXT_HALFALPHA);

			RECTXYWH picrect = m_sprCol.GetAFrameBBox(ANM_MENUS_SPR_GAME_MODE_SPLASHES, 0);
			//facem loc pentru mesajele de sub imagine
			RECTXYWH wndrect = picrect;
			wndrect.Inflate(-2, -2);
			wndrect.h += 32;
			int	wndSpacing = 24;

			//paint selection cursor
			RECTXYWH selrect;
			if ((m_rectSel.w > 0.0f) && (m_rectSel.h > 0.0f))
			{
				selrect.x = m_rectSel.x; selrect.y = m_rectSel.y;
				selrect.w = m_rectSel.w; selrect.h = m_rectSel.h;
				selrect.Inflate(5, 5);
				CtrlMgrDrawFrame(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_FRAME5, selrect, 0xffffffff);
			}
			//paint chapters
			D3DXVECTOR2 vpos = worldrect.Center();

			for (int kk = 0; kk < m_nSelElements; kk++)
			{
				float fDark = 1.0f - fabs((float)kk - m_fSelPageCursor);
				CLAMP(fDark, 0.0f, 1.0f);
				PaintGameModeWindow(D3DXVECTOR2(vpos.x + kk * (picrect.w + wndSpacing) - m_fSelPageCursor * (picrect.w + wndSpacing), vpos.y), m_arrSelItems[kk], 0.6f + 0.4f * fDark);
			}
			//paint scroll arrows - when not on back button
			if (m_nSelection >= 0)
			{
				float offx = 10.0f + sin(fLocalTimeline * 5.0f);
				if (m_nSelection > 0)
					CSprite::paintFrame(&m_sprCol, worldrect.CenterX() - 0.5f * wndrect.w - offx, worldrect.CenterY(), ANM_MENUS_SPR_ARROWS_LARGE, 0, 0xffffffff);
				if (m_nSelection < m_nSelElements - 1)
					CSprite::paintFrame(&m_sprCol, worldrect.CenterX() + 0.5f * wndrect.w + offx, worldrect.CenterY(), ANM_MENUS_SPR_ARROWS_LARGE, 1, 0xffffffff);
			}
			//--- page selector cursor ---
			RECTXYWH tmprect(worldrect.CenterX() - 20, worldrect.Bottom() - 24, 40, 4);
			tmprect.h -= 28;
			if ((m_nSelElements > 1) && (tmprect.w > 0))
			{
				int nSelPt = m_nSelection;
				if (m_nSelection < 0)
					nSelPt = m_nSelectionOld;
				CtrlMgrDrawPageSelector(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_PAGE_SELECTOR_L, tmprect, m_nSelElements, nSelPt, 0xffffffff, 0);
			}

			//buton back
			RECTXYWH recttemp(worldrect.CenterX() - 35, worldrect.Bottom() - 30, 70, 16);
			int butframe = 0;
			if (m_nSelection == -1) //daca am hover pe buton BACK
			{
				butframe = 3;
				if (m_bSelectionMade)
					butframe = 6;
			}
			CtrlMgrDrawHTilingAnim(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_BUTTON1, butframe, recttemp, 0xffffffff);
			//textul apasat pe butonul de back
			if ((!m_bSelectionMade) && (m_nSelection == -1))
				recttemp.y -= 1;
			g_font8bs1->DrawString(STR_BACK, recttemp, FONTFLAG_ANCHOR_VCENTERHCENTER, 0xffffffff);
		}
		break;

		case K_MM_STATE_CHAPTER_SELECT:
		{
			PaintMainBackground(worldrect, 0xff4444dd);
			//title
			RECTXYWH rRect(0, 22, 100, 18);
			CtrlMgrDrawWidebar(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_WIDEBAR2, rRect, 0xffffffff);
			g_font8bs1->DrawString(STR_SELECT_EPISODE, worldrect.CenterX(), worldrect.y + 35.0f, FONTFLAG_ANCHOR_BOTTOMCENTER, K_COLOR_SELECTED_TEXT);
			//game mode name (top left)
			if (g_gameMode == GAME_MODE_ZOMBIE_INVASION)
				g_font8b1->DrawString(STR_ZOMBIE_INVASION, 3.0f, worldrect.y + 13.0f, FONTFLAG_ANCHOR_BOTTOMLEFT, K_COLOR_DEFAULT_TEXT_HALFALPHA);
			else //classic mode
				g_font8b1->DrawString(STR_CLASSIC_MODE, 3.0f, worldrect.y + 13.0f, FONTFLAG_ANCHOR_BOTTOMLEFT, K_COLOR_DEFAULT_TEXT_HALFALPHA);
			//coop string if online game
			if (UTGetAppClass().IsGameNetworked())
				g_font8b1->DrawString(STR_COOP, worldrect.Right() - 3.0f, worldrect.y + 13.0f, FONTFLAG_ANCHOR_BOTTOMRIGHT, K_COLOR_DEFAULT_TEXT_HALFALPHA);
			else
				g_font8b1->DrawString(STR_LOCAL_GAME, worldrect.Right() - 3.0f, worldrect.y + 13.0f, FONTFLAG_ANCHOR_BOTTOMRIGHT, K_COLOR_DEFAULT_TEXT_HALFALPHA);

			RECTXYWH picrect = m_sprCol.GetAFrameBBox(ANM_MENUS_SPR_CHAPTER_SPLASHES_LG, 0);
			//messages under the image
			RECTXYWH wndrect = picrect;
			wndrect.Inflate(-2, -2);
			wndrect.h += 24;
			int	wndSpacing = 24;

			//paint selection cursor
			RECTXYWH selrect;
			if ((m_rectSel.w > 0.0f) && (m_rectSel.h > 0.0f))
			{
				selrect.x = m_rectSel.x; selrect.y = m_rectSel.y;
				selrect.w = m_rectSel.w; selrect.h = m_rectSel.h;
				selrect.Inflate(4, 4);
				CtrlMgrDrawFrame(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_FRAME5, selrect, 0xffffffff);
			}
			//paint chapters
			D3DXVECTOR2 vpos = worldrect.Center();

			for (int kk = 0; kk < m_nSelElements; kk++)
			{
				float fDark = 1.0f - fabs((float)kk - m_fSelPageCursor);
				CLAMP(fDark, 0.0f, 1.0f);
				PaintChapterWindowLarge(D3DXVECTOR2(vpos.x + kk * (picrect.w + wndSpacing) - m_fSelPageCursor * (picrect.w + wndSpacing), vpos.y), kk, 0.6f + 0.4f * fDark);
			}
			//paint scroll arrows - when not on back button
			if (m_nSelection >= 0)
			{
				float offx = 10.0f + sin(fLocalTimeline * 5.0f);
				if (m_nSelection > 0)
					CSprite::paintFrame(&m_sprCol, worldrect.CenterX() - 0.5f * wndrect.w - offx, worldrect.CenterY(), ANM_MENUS_SPR_ARROWS_LARGE, 0, 0xffffffff);
				if (m_nSelection < m_nSelElements - 1)
					CSprite::paintFrame(&m_sprCol, worldrect.CenterX() + 0.5f * wndrect.w + offx, worldrect.CenterY(), ANM_MENUS_SPR_ARROWS_LARGE, 1, 0xffffffff);
			}

			//buton back
			RECTXYWH recttemp(worldrect.CenterX() - 35, worldrect.Bottom() - 30, 70, 16);
			int butframe = 0;
			if (m_nSelection == -1) //daca am hover pe buton BACK
			{
				butframe = 3;
				if (m_bSelectionMade)
					butframe = 6;
			}
			CtrlMgrDrawHTilingAnim(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_BUTTON1, butframe, recttemp, 0xffffffff);
			//textul apasat pe butonul de back
			if ((!m_bSelectionMade) && (m_nSelection == -1))
				recttemp.y -= 1;
			g_font8bs1->DrawString(STR_BACK, recttemp, FONTFLAG_ANCHOR_VCENTERHCENTER, 0xffffffff);

			//--- total stars ---
			/*
			g_font6ns1->DrawString(STR_TOTAL_STARS, worldrect.CenterX() - 150, worldrect.Bottom() - 20, FONTFLAG_ANCHOR_BOTTOMLEFT, K_COLOR_DEFAULT_TEXT);
			CSprite::paintFrame(&m_sprCol, worldrect.CenterX() - 150, worldrect.Bottom() - 15, ANM_MENUS_SPR_LEVEL_STARS, 1, 0xffffffff);
			g_font8bs1->DrawString(STR_TOTAL_STARS_VAL, worldrect.CenterX() - 140, worldrect.Bottom() - 10, FONTFLAG_ANCHOR_BOTTOMLEFT, K_COLOR_SELECTED_TEXT);
			*/
		}
		break;

		case K_MM_STATE_LEVEL_SELECT:
		{
			//incadram 2 panels in rezolutia de baza de 320x240
			RECTXYWH rectLeftPanel(worldrect.CenterX() - 160, 0, 100, worldrect.h);
			RECTXYWH rectRightPanel(worldrect.CenterX() - 160 + 100, 20, 220, worldrect.h - 20);
			RECTXYWH tmprect; 

			int nSelectedChapter = g_userData[K_MEMID_SELECTED_CHAPTER];

			PaintMainBackground(worldrect, 0xff4444dd);
			//game mode name (top left)
			if (g_gameMode == GAME_MODE_ZOMBIE_INVASION)
				g_font8b1->DrawString(STR_ZOMBIE_INVASION, 3.0f, worldrect.y + 13.0f, FONTFLAG_ANCHOR_BOTTOMLEFT, K_COLOR_DEFAULT_TEXT_HALFALPHA);
			else //classic mode
				g_font8b1->DrawString(STR_CLASSIC_MODE, 3.0f, worldrect.y + 13.0f, FONTFLAG_ANCHOR_BOTTOMLEFT, K_COLOR_DEFAULT_TEXT_HALFALPHA);
			//coop string if online game
			if (UTGetAppClass().IsGameNetworked())
				g_font8b1->DrawString(STR_COOP, worldrect.Right() - 3.0f, worldrect.y + 13.0f, FONTFLAG_ANCHOR_BOTTOMRIGHT, K_COLOR_DEFAULT_TEXT_HALFALPHA);
			else
				g_font8b1->DrawString(STR_LOCAL_GAME, worldrect.Right() - 3.0f, worldrect.y + 13.0f, FONTFLAG_ANCHOR_BOTTOMRIGHT, K_COLOR_DEFAULT_TEXT_HALFALPHA);

			//--- paint levels ---
			int nSelPerPage = m_nSelRows * m_nSelColumns;
			//dimensiuni ferestre nivel si spacing intre ele
			SIZEWH wndSz(48, 36), wndSpacing(8, 8), wndSzTotal(48 + 8, 36 + 8);
			//page rect este offsetat putin (8px) in sus ca sa incapa butonul de back
			RECTXYWH pageRect(rectRightPanel.CenterX() - 0.5f * m_nSelColumns * wndSzTotal.w, rectRightPanel.CenterY() - 0.5f * m_nSelRows * wndSzTotal.h - 8, m_nSelColumns * wndSzTotal.w, m_nSelRows * wndSzTotal.h);

			//--- paint selection ---
			RECTXYWH selrect;
			if ((m_rectSel.w > 0.0f) && (m_rectSel.h > 0.0f))
			{
				selrect.x = m_rectSel.x; selrect.y = m_rectSel.y;
				selrect.w = m_rectSel.w; selrect.h = m_rectSel.h;
				CtrlMgrDrawFrame(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_FRAME5, selrect, 0xffffffff);
			}

			for (int kk = 0; kk < m_nSelElements; kk++)
			{
				int nPage = kk / nSelPerPage;
				int nRow = (kk % nSelPerPage) / m_nSelColumns;
				int nColumn = (kk % nSelPerPage) % m_nSelColumns;

				float wndAlpha = 1.0f;

				wndAlpha = 1.0f - fabs((float)nPage - m_fSelPageCursor);
				CLAMP(wndAlpha, 0.0f, 1.0f);
				//daca nu se vede nu deseneazza
				if (wndAlpha <= 0.0f)
					continue;

				DWORD wndCol = D3DCOLOR_FFFA(wndAlpha);

				D3DXVECTOR2 vRefPos(pageRect.x, pageRect.y);
				vRefPos.x -= m_fSelPageCursor * pageRect.w;

				RECTXYWH lvlrect(vRefPos.x + nColumn * wndSzTotal.w + nPage * pageRect.w + wndSpacing.w / 2, vRefPos.y + nRow * wndSzTotal.h + wndSpacing.h / 2, wndSz.w, wndSz.h);
				int nRealLevelIdx = nSelectedChapter * K_GAME_LEVELS_PER_CHAPTER + kk;

				if (UTGetChaptersList().IsValidLevel(nSelectedChapter, kk))
				{
					CSprite::paintFrame(&m_sprCol, lvlrect.CenterX(), lvlrect.CenterY(), ANM_MENUS_SPR_LEVEL_PANEL_STARS, g_levelStats[nRealLevelIdx].nStars, wndCol);
					//level type icon
					int nLevelType = g_levelStats[nRealLevelIdx].nLevelType + 1;
					if ((nLevelType <= 0) || (nLevelType >= m_sprCol.GetAFramesCnt(ANM_MENUS_SPR_LEVEL_TYPE_ICONS)))
					{
						//yellow bands on missing levels
						CSprite::paintFrame(&m_sprCol, lvlrect.CenterX(), lvlrect.CenterY(), ANM_MENUS_SPR_LEVEL_PANEL_LOCKED, 0, wndCol);
						//nLevelType = 0;
						//CSprite::paintFrame(&m_sprCol, lvlrect.CenterX(), lvlrect.CenterY(), ANM_MENUS_SPR_LEVEL_TYPE_ICONS, nLevelType, wndCol);
					}
					else
					{
						CSprite::paintFrame(&m_sprCol, lvlrect.CenterX(), lvlrect.CenterY(), ANM_MENUS_SPR_LEVEL_TYPE_ICONS, nLevelType, wndCol);
						//numar nivel
						CStringDesc strdesc;
						g_stringsMgr.SetStringDesc(&strdesc, L"%d", kk + 1);
						g_font6n1->DrawString(&strdesc, lvlrect.x + 7, lvlrect.y + 7, FONTFLAG_ANCHOR_VCENTERHCENTER, D3DCOLOR_COLORALPHA(K_COLOR_DEFAULT_TEXT, wndAlpha));
					}
				}
				else
				{
					CSprite::paintFrame(&m_sprCol, lvlrect.CenterX(), lvlrect.CenterY(), ANM_MENUS_SPR_LEVEL_PANEL_LOCKED, 0, wndCol);
				}
			}
			//--- string melee = leaderboards
			if(m_nSelection >= 0)
				g_font6ns1->DrawString(STR_MELEE_LEADERBOARDS, pageRect.Right(), pageRect.Bottom() + 6, FONTFLAG_ANCHOR_BOTTOMRIGHT, K_COLOR_SELECTED_TEXT);

			//--- mission name ---
			RECTXYWH rRect(0, 19, 400, 30);
			CtrlMgrDrawWidebar(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_WIDEBAR2, rRect, 0xffffffff);

			g_font6ns1->DrawString(STR_SELECT_MISSION, rectRightPanel.CenterX(), rectRightPanel.y + 6, FONTFLAG_ANCHOR_BOTTOMCENTER, K_COLOR_DEFAULT_TEXT);
			if (m_nSelection >= 0)
			{
				int nRealLevelIdx = nSelectedChapter * K_GAME_LEVELS_PER_CHAPTER + m_nSelection;
				CStringDesc sdMission;
				int nStrIdxLevelName = UTGetChaptersList().m_arrChapters[nSelectedChapter]->arrLevelNameStrIdx[m_nSelection];
				if(nStrIdxLevelName >= 0)
					g_stringsMgr.SetStringDesc(&sdMission, L"%d. %s", m_nSelection + 1, g_stringsMgr.strings[nStrIdxLevelName]->sText);
				else
					g_stringsMgr.SetStringDesc(&sdMission, L"%d. %s", m_nSelection + 1, g_stringsMgr.strings[STR_NOT_AVAILABLE]->sText);

				g_font8bs1->DrawString(&sdMission, rectRightPanel.CenterX(), rectRightPanel.y + 18, FONTFLAG_ANCHOR_BOTTOMCENTER, K_COLOR_SELECTED_TEXT);
				
				int nLevelType = g_levelStats[nRealLevelIdx].nLevelType;
				if ((nLevelType >= 0) && (nLevelType < K_GAME_LSTYPES_COUNT))
				{
					g_font6ns1->DrawString(STR_MISSION_TYPE1 + g_levelStats[nRealLevelIdx].nLevelType, rectRightPanel.CenterX(), rectRightPanel.y + 28, FONTFLAG_ANCHOR_BOTTOMCENTER, K_COLOR_DEFAULT_TEXT);
				}
			}

			//--- page selector cursor ---
			tmprect = pageRect; tmprect.h += 3;
			if (m_nSelPagesCnt > 1)
			{
				CtrlMgrDrawPageSelector(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_PAGE_SELECTOR_L, tmprect, m_nSelPagesCnt, m_nSelPage, 0xffffffff, 1);
			}
			//paint scroll arrows - when not on back button
			if (m_nSelPage >= 0)
			{
				float offx = sin(fLocalTimeline * 5.0f);
				if (m_nSelPage > 0)
					CSprite::paintFrame(&UTGetControlsManager().m_sprCol, pageRect.x - offx, pageRect.CenterY(), ANM_CONTROLS_SPR_ARROWS1, K_DIR_LEFT, 0xffffffff);
				if (m_nSelPage < m_nSelPagesCnt - 1)
					CSprite::paintFrame(&UTGetControlsManager().m_sprCol, pageRect.Right() + offx, pageRect.CenterY(), ANM_CONTROLS_SPR_ARROWS1, K_DIR_RIGHT, 0xffffffff);
			}

			//buton back
			RECTXYWH recttemp(rectRightPanel.CenterX() - 35, rectRightPanel.Bottom() - 30, 70, 16);
			int butframe = 0;
			if (m_nSelection == -1) //daca am hover pe buton BACK
			{
				butframe = 3;
				if (m_bSelectionMade)
					butframe = 6;
			}
			CtrlMgrDrawHTilingAnim(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_BUTTON1, butframe, recttemp, 0xffffffff);
			//textul apasat pe butonul de back
			if ((!m_bSelectionMade) && (m_nSelection == -1))
				recttemp.y -= 1;
			g_font8bs1->DrawString(STR_BACK, recttemp, FONTFLAG_ANCHOR_VCENTERHCENTER, 0xffffffff);


			//title and chapter window
			tmprect = rectLeftPanel;
			tmprect.h = 45;

			PaintChapterWindow(D3DXVECTOR2(rectLeftPanel.CenterX(), rectLeftPanel.CenterY()), nSelectedChapter, 0xffffffff);
			//#WEEKLY: paint medals - temp
			if (nSelectedChapter == K_GAME_WEEKLY_CHALLENGE_CHAPTER_NO)
			{
				switch (m_nSelection)
				{
					//week 1
					case 0:
					{
						//medal
						int nFrame = ((g_userData[K_MEMID_WEEKLY1_TASKS_FLAGS] & 1) != 0) ? 1 : 0;
						CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectLeftPanel.x + 10, rectLeftPanel.Bottom() - 95, ANM_CONTROLS_SPR_MEDAL1, nFrame);
						//text
						RECTXYWH txtrct(rectLeftPanel.x + 22, rectLeftPanel.Bottom() - 97, 85, 50);
						g_font6ns1->DrawString(STR_WEEK1_TASK, txtrct, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
					}
					break;
					//week 2
					case 1:
					{
						//medal
						int nFrame = ((g_userData[K_MEMID_WEEKLY2_TASKS_FLAGS] & 1) != 0) ? 1 : 0;
						CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectLeftPanel.x + 10, rectLeftPanel.Bottom() - 95, ANM_CONTROLS_SPR_MEDAL1, nFrame);
						//text
						RECTXYWH txtrct(rectLeftPanel.x + 22, rectLeftPanel.Bottom() - 97, 85, 50);
						g_font6ns1->DrawString(STR_WEEK2_TASK1, txtrct, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
						//medal
						nFrame = ((g_userData[K_MEMID_WEEKLY2_TASKS_FLAGS] & 2) != 0) ? 1 : 0;
						CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectLeftPanel.x + 10, rectLeftPanel.Bottom() - 83, ANM_CONTROLS_SPR_MEDAL1, nFrame);
						//text
						txtrct.Set(rectLeftPanel.x + 22, rectLeftPanel.Bottom() - 85, 85, 50);
						g_font6ns1->DrawString(STR_WEEK2_TASK2, txtrct, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
					}
					break;
					//week 3 - zombies - no sniper
					case 2:
					{
						//medal
						int nFrame = ((g_userData[K_MEMID_WEEKLY3_TASKS_FLAGS] & 1) != 0) ? 1 : 0;
						CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectLeftPanel.x + 10, rectLeftPanel.Bottom() - 95, ANM_CONTROLS_SPR_MEDAL1, nFrame);
						//text
						RECTXYWH txtrct(rectLeftPanel.x + 22, rectLeftPanel.Bottom() - 97, 85, 50);
						g_font6ns1->DrawString(STR_WEEK2_TASK2, txtrct, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
					}
					break;
					//week 4 - no health, finish as shield
					case 3:
					{
						//medal
						int nFrame = ((g_userData[K_MEMID_WEEKLY4_TASKS_FLAGS] & 1) != 0) ? 1 : 0;
						CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectLeftPanel.x + 10, rectLeftPanel.Bottom() - 95, ANM_CONTROLS_SPR_MEDAL1, nFrame);
						//text
						RECTXYWH txtrct(rectLeftPanel.x + 22, rectLeftPanel.Bottom() - 97, 85, 50);
						g_font6ns1->DrawString(STR_WEEK4_TASK1, txtrct, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
						//medal
						nFrame = ((g_userData[K_MEMID_WEEKLY4_TASKS_FLAGS] & 2) != 0) ? 1 : 0;
						CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectLeftPanel.x + 10, rectLeftPanel.Bottom() - 83, ANM_CONTROLS_SPR_MEDAL1, nFrame);
						//text
						txtrct.Set(rectLeftPanel.x + 22, rectLeftPanel.Bottom() - 85, 85, 50);
						g_font6ns1->DrawString(STR_WEEK4_TASK2, txtrct, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
					}
					break;
					//week 5 - under 3 min, win as breacher
					case 4:
					{
						//medal
						int nFrame = ((g_userData[K_MEMID_WEEKLY5_TASKS_FLAGS] & 1) != 0) ? 1 : 0;
						CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectLeftPanel.x + 10, rectLeftPanel.Bottom() - 95, ANM_CONTROLS_SPR_MEDAL1, nFrame);
						//text
						RECTXYWH txtrct(rectLeftPanel.x + 22, rectLeftPanel.Bottom() - 97, 85, 50);
						g_font6ns1->DrawString(STR_WEEK1_TASK, txtrct, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
						//medal
						nFrame = ((g_userData[K_MEMID_WEEKLY5_TASKS_FLAGS] & 2) != 0) ? 1 : 0;
						CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectLeftPanel.x + 10, rectLeftPanel.Bottom() - 83, ANM_CONTROLS_SPR_MEDAL1, nFrame);
						//text
						txtrct.Set(rectLeftPanel.x + 22, rectLeftPanel.Bottom() - 85, 85, 50);
						g_font6ns1->DrawString(STR_WEEK5_TASK1, txtrct, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
					}
					break;
					//week 6 - win as fergie
					case 5:
					{
						//medal
						int nFrame = ((g_userData[K_MEMID_WEEKLY6_TASKS_FLAGS] & 1) != 0) ? 1 : 0;
						CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectLeftPanel.x + 10, rectLeftPanel.Bottom() - 95, ANM_CONTROLS_SPR_MEDAL1, nFrame);
						//text
						RECTXYWH txtrct(rectLeftPanel.x + 22, rectLeftPanel.Bottom() - 97, 85, 50);
						g_font6ns1->DrawString(STR_WEEK6_TASK1, txtrct, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
					}
					break;
					//week 7
					case 6:
					{
						//medal
						int nFrame = ((g_userData[K_MEMID_WEEKLY7_TASKS_FLAGS] & 1) != 0) ? 1 : 0;
						CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectLeftPanel.x + 10, rectLeftPanel.Bottom() - 95, ANM_CONTROLS_SPR_MEDAL1, nFrame);
						//text
						RECTXYWH txtrct(rectLeftPanel.x + 22, rectLeftPanel.Bottom() - 97, 85, 50);
						g_font6ns1->DrawString(STR_WEEK7_TASK1, txtrct, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
					}
					break;
					// week 8: no healing, no snipers
					case 7:
					{
						//medal
						int nFrame = ((g_userData[K_MEMID_WEEKLY8_TASKS_FLAGS] & 1) != 0) ? 1 : 0;
						CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectLeftPanel.x + 10, rectLeftPanel.Bottom() - 95, ANM_CONTROLS_SPR_MEDAL1, nFrame);
						//text
						RECTXYWH txtrct(rectLeftPanel.x + 22, rectLeftPanel.Bottom() - 97, 85, 50);
						g_font6ns1->DrawString(STR_WEEK4_TASK1, txtrct, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
						//medal
						nFrame = ((g_userData[K_MEMID_WEEKLY8_TASKS_FLAGS] & 2) != 0) ? 1 : 0;
						CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectLeftPanel.x + 10, rectLeftPanel.Bottom() - 83, ANM_CONTROLS_SPR_MEDAL1, nFrame);
						//text
						txtrct.Set(rectLeftPanel.x + 22, rectLeftPanel.Bottom() - 85, 85, 50);
						g_font6ns1->DrawString(STR_WEEK2_TASK2, txtrct, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
					}
					break;
					//week 9 - win as assaulter, no snipers
					case 8:
					{
						//medal
						int nFrame = ((g_userData[K_MEMID_WEEKLY9_TASKS_FLAGS] & 1) != 0) ? 1 : 0;
						CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectLeftPanel.x + 10, rectLeftPanel.Bottom() - 95, ANM_CONTROLS_SPR_MEDAL1, nFrame);
						//text
						RECTXYWH txtrct(rectLeftPanel.x + 22, rectLeftPanel.Bottom() - 97, 85, 50);
						g_font6ns1->DrawString(STR_WEEK7_TASK1, txtrct, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
						//medal
						nFrame = ((g_userData[K_MEMID_WEEKLY9_TASKS_FLAGS] & 2) != 0) ? 1 : 0;
						CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectLeftPanel.x + 10, rectLeftPanel.Bottom() - 83, ANM_CONTROLS_SPR_MEDAL1, nFrame);
						//text
						txtrct.Set(rectLeftPanel.x + 22, rectLeftPanel.Bottom() - 85, 85, 50);
						g_font6ns1->DrawString(STR_WEEK2_TASK2, txtrct, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
					}
					break;
					//week 10
					case 9:
					{
						//medal
						int nFrame = ((g_userData[K_MEMID_WEEKLY10_TASKS_FLAGS] & 1) != 0) ? 1 : 0;
						CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectLeftPanel.x + 10, rectLeftPanel.Bottom() - 95, ANM_CONTROLS_SPR_MEDAL1, nFrame);
						//text
						RECTXYWH txtrct(rectLeftPanel.x + 22, rectLeftPanel.Bottom() - 97, 85, 50);
						g_font6ns1->DrawString(STR_WEEK10_TASK1, txtrct, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
					}
					break;
					//week 11 - win as fergie
					case 10:
					{
						//medal
						int nFrame = ((g_userData[K_MEMID_WEEKLY11_TASKS_FLAGS] & 1) != 0) ? 1 : 0;
						CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectLeftPanel.x + 10, rectLeftPanel.Bottom() - 95, ANM_CONTROLS_SPR_MEDAL1, nFrame);
						//text
						RECTXYWH txtrct(rectLeftPanel.x + 22, rectLeftPanel.Bottom() - 97, 85, 50);
						g_font6ns1->DrawString(STR_WEEK6_TASK1, txtrct, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
					}
					break;
					//week 12 - under 3 min
					case 11:
					{
						//medal
						int nFrame = ((g_userData[K_MEMID_WEEKLY12_TASKS_FLAGS] & 1) != 0) ? 1 : 0;
						CSprite::paintFrame(&UTGetControlsManager().m_sprCol, rectLeftPanel.x + 10, rectLeftPanel.Bottom() - 95, ANM_CONTROLS_SPR_MEDAL1, nFrame);
						//text
						RECTXYWH txtrct(rectLeftPanel.x + 22, rectLeftPanel.Bottom() - 97, 85, 50);
						g_font6ns1->DrawString(STR_WEEK1_TASK, txtrct, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
					}
					break;
				}
			}

			//--- total stars ---
			recttemp = rectLeftPanel; recttemp.y = 200; recttemp.h = 30; recttemp.x += 5; recttemp.w -= 10;
			recttemp.Inflate(-6, -6);
			CSprite::paintFrame(&m_sprCol, recttemp.x, recttemp.CenterY(), ANM_MENUS_SPR_STARS_BOX, 0, 0xffffffff);

			g_font6ns1->DrawString(STR_TOTAL_STARS, recttemp.CenterX(), recttemp.y - 1, FONTFLAG_ANCHOR_TOPCENTER, K_COLOR_DEFAULT_TEXT);
			g_font8bs1->DrawString(STR_TOTAL_STARS_VAL, recttemp.CenterX(), recttemp.y + 9, FONTFLAG_ANCHOR_TOPCENTER, K_COLOR_SELECTED_TEXT);

		}
		break;

		case K_MM_STATE_DOWNLOADED_LEVEL_SELECT:
		{
#ifdef ENABLE_STEAM_WORKSHOP

			int nSelectedChapter = g_userData[K_MEMID_SELECTED_CHAPTER];

			//selected level details
			RECTXYWH rectSelMod(worldrect.CenterX() - 150, worldrect.y + 50, 300, 52);
			RECTXYWH rectItemsPanel(worldrect.CenterX() - 160, 110, 320, worldrect.h - 110);
			RECTXYWH tmprect; 

			PaintMainBackground(worldrect, 0xff4444dd);
			//game mode name (top left)
			if (g_gameMode == GAME_MODE_ZOMBIE_INVASION)
				g_font8b1->DrawString(STR_ZOMBIE_INVASION, 3.0f, worldrect.y + 13.0f, FONTFLAG_ANCHOR_BOTTOMLEFT, K_COLOR_DEFAULT_TEXT_HALFALPHA);
			else //classic mode
				g_font8b1->DrawString(STR_CLASSIC_MODE, 3.0f, worldrect.y + 13.0f, FONTFLAG_ANCHOR_BOTTOMLEFT, K_COLOR_DEFAULT_TEXT_HALFALPHA);
			//coop string if online game
			if (UTGetAppClass().IsGameNetworked())
				g_font8b1->DrawString(STR_COOP, worldrect.Right() - 3.0f, worldrect.y + 13.0f, FONTFLAG_ANCHOR_BOTTOMRIGHT, K_COLOR_DEFAULT_TEXT_HALFALPHA);
			else
				g_font8b1->DrawString(STR_LOCAL_GAME, worldrect.Right() - 3.0f, worldrect.y + 13.0f, FONTFLAG_ANCHOR_BOTTOMRIGHT, K_COLOR_DEFAULT_TEXT_HALFALPHA);

			///--- paint levels ---
			int nSelPerPage = m_nSelRows * m_nSelColumns;
			//dimensiuni ferestre nivel si spacing intre ele
			SIZEWH wndSz(48, 36), wndSpacing(8, 8), wndSzTotal(48 + 8, 36 + 8);
			
			RECTXYWH pageRect(rectItemsPanel.CenterX() - 0.5f * m_nSelColumns * wndSzTotal.w, rectItemsPanel.y, m_nSelColumns * wndSzTotal.w, m_nSelRows * wndSzTotal.h);

			//--- paint selection ---
			RECTXYWH selrect;
			if ((m_rectSel.w > 0.0f) && (m_rectSel.h > 0.0f))
			{
				selrect.x = m_rectSel.x; selrect.y = m_rectSel.y;
				selrect.w = m_rectSel.w; selrect.h = m_rectSel.h;
				CtrlMgrDrawFrame(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_FRAME5, selrect, 0xffffffff);
			}

			for (int kk = 0; kk < m_nSelElements; kk++)
			{
				int nPage = kk / nSelPerPage;
				int nRow = (kk % nSelPerPage) / m_nSelColumns;
				int nColumn = (kk % nSelPerPage) % m_nSelColumns;

				float wndAlpha = 1.0f;

				wndAlpha = 1.0f - fabs((float)nPage - m_fSelPageCursor);
				CLAMP(wndAlpha, 0.0f, 1.0f);
				//daca nu se vede nu deseneazza
				if (wndAlpha <= 0.0f)
					continue;

				DWORD wndCol = D3DCOLOR_FFFA(wndAlpha);

				D3DXVECTOR2 vRefPos(pageRect.x, pageRect.y);
				vRefPos.x -= m_fSelPageCursor * pageRect.w;

				RECTXYWH lvlrect(vRefPos.x + nColumn * wndSzTotal.w + nPage * pageRect.w + wndSpacing.w / 2, vRefPos.y + nRow * wndSzTotal.h + wndSpacing.h / 2, wndSz.w, wndSz.h);
				//BASE PANEL (should not contain stars)
				CSprite::paintFrame(&m_sprCol, lvlrect.CenterX(), lvlrect.CenterY(), ANM_MENUS_SPR_LEVEL_PANEL_LOCKED, 1, wndCol);
				//level type icon
				int nLevelType = 1 + m_arrSelItems[kk * 3 + 1]; //saved when initializing state
				if ((nLevelType <= 0) || (nLevelType >= m_sprCol.GetAFramesCnt(ANM_MENUS_SPR_LEVEL_TYPE_ICONS)))
				{
					//yellow bands on missing levels
					CSprite::paintFrame(&m_sprCol, lvlrect.CenterX(), lvlrect.CenterY(), ANM_MENUS_SPR_LEVEL_PANEL_LOCKED, 0, wndCol);
				}
				else
				{
					CSprite::paintFrame(&m_sprCol, lvlrect.CenterX() - 3, lvlrect.CenterY() + 1, ANM_MENUS_SPR_LEVEL_TYPE_ICONS, nLevelType, wndCol);
				}
			}

			//nothing to show
			if (m_nSelElements == 0)
			{
				g_font8bs1->DrawString(STR_NO_DOWNLOADED_LEVELS, pageRect, FONTFLAG_ANCHOR_VCENTERHCENTER | FONTFLAG_WRAPTEXT, K_COLOR_SELECTED_TEXT);
			}

			///--- SELECTED MOD details ---
			CtrlMgrDrawFrame(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_FRAME1, rectSelMod, 0x88888888);
			//image
			if ((m_nSelection >= 0) && (m_nSelection < m_nSelElements))
			{
				if (m_arrSelItems[m_nSelection * 3] >= 0)
				{
					RECT rcImg;
					TexNode* pTex = UTGetAppClass().g_texManager.GetTextureNode(m_arrSelItems[m_nSelection * 3]);
					if (pTex)
					{
						SetRect(&rcImg, 0, 0, pTex->widthToLoad, pTex->heightToLoad);
						m_pSprite->Draw(pTex->pTexture, &rcImg, NULL, &D3DXVECTOR3(rectSelMod.x - 1, rectSelMod.y - 1, 0.0f), 0xffffffff);
					}
				}

				CModsManager::CModDescriptor *mod = UTGetModsManager().GetModDescByIndex(m_arrSelItems[m_nSelection * 3 + 2]);
				if (mod != null)
				{
					CStringDesc sdModName, sdModDesc;
					g_stringsMgr.SetStringDesc(&sdModName, mod->shName.text);
					WCHAR strCompleteDesc[2048];
					StringCchPrintf(strCompleteDesc, 2048, L"%s\n- %s", mod->strDescription, mod->strAuthor);
					g_stringsMgr.SetString(STR_TEMP12, strCompleteDesc);
					tmprect = rectSelMod; tmprect.x += 100; tmprect.w -= 100; tmprect.y += 20; tmprect.h -= 18;
					g_font8bs1->DrawString(&sdModName, tmprect.x, tmprect.y - 10, FONTFLAG_ANCHOR_VCENTERLEFT, K_COLOR_SELECTED_TEXT);

					int texth = g_font6n1->MeasureString(STR_TEMP12, tmprect.w).h;
					if (texth <= tmprect.h)
					{
						g_font6n1->DrawString(STR_TEMP12, tmprect, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
					}
					else
					{
						float fTmMul = 8.0f / (float)(texth - tmprect.h);
						float offy = LIMIT((float)sin((-1.0f + m_fSelTimer) * fTmMul), 0.0f, 1.0f);
						offy *= -(texth - tmprect.h);
						g_font6n1->DrawStringOffsetY(STR_TEMP12, tmprect, (int)floor(offy), FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT | FONTFLAG_CLIPTEXT, K_COLOR_DEFAULT_TEXT);
					}
				}
			}


			//--- title ---
			RECTXYWH rRect(0, 20, 100, 18);
			CtrlMgrDrawWidebar(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_WIDEBAR2, rRect, 0xffffffff);
			g_font8bs1->DrawString(STR_DOWNLOADED_LEVELS, worldrect.CenterX(), worldrect.y + 33.0f, FONTFLAG_ANCHOR_BOTTOMCENTER, K_COLOR_SELECTED_TEXT);


			//if (m_nSelection >= 0)
			//{
			//	int nLevelType = g_levelStats[nRealLevelIdx].nLevelType;
			//	if ((nLevelType >= 0) && (nLevelType < K_GAME_LSTYPES_COUNT))
			//	{
			//		g_font6ns1->DrawString(STR_MISSION_TYPE1 + g_levelStats[nRealLevelIdx].nLevelType, rRect.CenterX(), rRect.y + 28, FONTFLAG_ANCHOR_BOTTOMCENTER, K_COLOR_DEFAULT_TEXT);
			//	}
			//}

			//--- page selector cursor ---
			tmprect = pageRect; tmprect.h += 1; tmprect.w -= 2;
			if (m_nSelPagesCnt > 1)
			{
				CtrlMgrDrawPageSelector(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_PAGE_SELECTOR_L, tmprect, m_nSelPagesCnt, m_nSelPage, 0xffffffff, 1);
			}
			//paint scroll arrows - when not on back button
			if (m_nSelPage >= 0)
			{
				float offx = sin(fLocalTimeline * 5.0f);
				if (m_nSelPage > 0)
					CSprite::paintFrame(&UTGetControlsManager().m_sprCol, pageRect.x - offx, pageRect.CenterY(), ANM_CONTROLS_SPR_ARROWS1, K_DIR_LEFT, 0xffffffff);
				if (m_nSelPage < m_nSelPagesCnt - 1)
					CSprite::paintFrame(&UTGetControlsManager().m_sprCol, pageRect.Right() + offx, pageRect.CenterY(), ANM_CONTROLS_SPR_ARROWS1, K_DIR_RIGHT, 0xffffffff);
			}

			//buton back
			RECTXYWH recttemp(rectItemsPanel.CenterX() - 35, rectItemsPanel.Bottom() - 30, 70, 16);
			int butframe = 0;
			if (m_nSelection == -1) //daca am hover pe buton BACK
			{
				butframe = 3;
				if (m_bSelectionMade)
					butframe = 6;
			}
			CtrlMgrDrawHTilingAnim(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_BUTTON1, butframe, recttemp, 0xffffffff);
			//textul apasat pe butonul de back
			if ((!m_bSelectionMade) && (m_nSelection == -1))
				recttemp.y -= 1;
			g_font8bs1->DrawString(STR_BACK, recttemp, FONTFLAG_ANCHOR_VCENTERHCENTER, 0xffffffff);

#endif
		}
		break;

	}

	//final flush
	m_pSprite->SetTransform(&g_matIdentity);
	m_pSprite->Flush();
}

void CMainMenu::PaintMainBackground(RECTXYWH_F worldRect, DWORD dwColor, bool bPaintParticles)
{
	//paint back
	float fbackOffX = 10.0f + 8.0f * sin(fLocalTimeline * 0.4f + M_PI);
	CSprite::paintFrame(&m_sprCol, worldRect.CenterX() + fbackOffX, worldRect.CenterY(), ANM_MENUS_SPR_BACK_LAYERS, 0, dwColor);
	//particles
	if (bPaintParticles)
	{
		//linear sampling
		m_pSprite->Flush();
		m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

		//particule foc
		g_particlesMgr.PaintLayer(K_PART_LAYER_NORMAL_LIGHT, true);

		m_pSprite->Flush();
		m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	}
	//paint chars
	fbackOffX = 4.0f + 4.0f * sin(fLocalTimeline * 0.4f + M_PI);
	CSprite::paintFrame(&m_sprCol, worldRect.CenterX() + fbackOffX, worldRect.CenterY(), ANM_MENUS_SPR_BACK_LAYERS, 1, dwColor);
	//paint character flickering orange light
	AdditiveBlendingON(m_pDevice, m_pSprite);
	float alpha = 0.4f + PerlinNoise1D(fLocalTimeline, 5.0f, 2.0f, 0.4f, 0.5f, 2);
	CSprite::paintFrame(&m_sprCol, worldRect.CenterX() + fbackOffX, worldRect.CenterY(), ANM_MENUS_SPR_BACK_LAYERS, 2, D3DCOLOR_COLORALPHA(dwColor, alpha));

	//paint flickering right glow
	CSprite::paintFrame(&m_sprCol, worldRect.Right(), worldRect.CenterY(), ANM_MENUS_SPR_BACK_LAYERS, 4, D3DCOLOR_COLORALPHA(dwColor, alpha));

	AdditiveBlendingOFF(m_pDevice, m_pSprite);


	//paint fog layer (w:375)
	float fogoffx = -375.0f * FLOAT_FRAC(fLocalTimeline * 2.0f / 375.0f) + (2.0f * sin(fLocalTimeline * 0.4f + M_PI));
	CSprite::paintFrame(&m_sprCol, worldRect.CenterX() + fogoffx, worldRect.CenterY(), ANM_MENUS_SPR_FOG, 0, dwColor);
	CSprite::paintFrame(&m_sprCol, worldRect.CenterX() + fogoffx + 375.0f, worldRect.CenterY(), ANM_MENUS_SPR_FOG, 0, dwColor);

	fogoffx = -375.0f * FLOAT_FRAC(fLocalTimeline * 5.0f / 375.0f) + (2.0f * sin(fLocalTimeline * 0.4f + M_PI));
	CSprite::paintFrame(&m_sprCol, worldRect.CenterX() + fogoffx, worldRect.CenterY(), ANM_MENUS_SPR_FOG, 1, dwColor);
	CSprite::paintFrame(&m_sprCol, worldRect.CenterX() + fogoffx + 375.0f, worldRect.CenterY(), ANM_MENUS_SPR_FOG, 1, dwColor);
	//paint shotgun
	CSprite::paintFrame(&m_sprCol, worldRect.CenterX() - 12.0f + (6.0f * sin(fLocalTimeline * 0.4f)), worldRect.CenterY(), ANM_MENUS_SPR_BACK_LAYERS, 3, dwColor);
}

void CMainMenu::PaintChapterWindow(D3DXVECTOR2 vCenter, int nChapterIdx, DWORD dwColor)
{
	bool bChapterUnlocked = UTGetChaptersList().IsChapterUnlocked(nChapterIdx, g_userData[K_MEMID_MISSIONS_COMPLETED]);

	RECTXYWH picrect = m_sprCol.GetAFrameBBox(ANM_MENUS_SPR_CHAPTER_SPLASHES, 0);
	//facem loc pentru mesajele de sub imagine
	RECTXYWH wndrect = picrect;
	wndrect.Inflate(-2, -2);
	wndrect.h += 24;
	int	wndSpacing = 24;

	RECTXYWH wndrectL = wndrect; //local wndrect
	wndrectL.x += vCenter.x - picrect.w / 2.0f;
	wndrectL.y += vCenter.y - wndrect.h / 2.0f;

	float wndAlpha = D3DCOLOR_GETFALPHA(dwColor);
	//Paint Chapter Window
	CtrlMgrDrawFrame(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_FRAME1, wndrectL, dwColor);
	//mission image
	CSprite::paintFrame(&m_sprCol, wndrectL.x - 2, wndrectL.y - 2, ANM_MENUS_SPR_CHAPTER_SPLASHES, UTGetChaptersList().m_arrChapters[nChapterIdx]->nChapterImgFrame, dwColor);
	
	//mission name frame and string
	RECTXYWH titlerect(wndrectL.x, wndrectL.y + picrect.h + 1, wndrectL.w, 9);
	CtrlMgrDrawFrame(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_FRAME4, titlerect, dwColor);
	titlerect.Inflate(1, 1);
	if(UTGetChaptersList().m_arrChapters[nChapterIdx]->nChapterNameStrIdx >= 0)
		g_font6n1->DrawStringClamped(UTGetChaptersList().m_arrChapters[nChapterIdx]->nChapterNameStrIdx, titlerect.CenterX(), titlerect.y + 13, titlerect.w + 6, FONTFLAG_ANCHOR_TOPCENTER, D3DCOLOR_COLORALPHA(K_COLOR_DEFAULT_TEXT, wndAlpha));

	g_stringsMgr.ReplaceTokenInt(STR_TEMP1, STR_CHAPTER_N, 1, nChapterIdx + 1);
	g_font6n1->DrawString(STR_TEMP1, titlerect.CenterX(), titlerect.Bottom() - 3, FONTFLAG_ANCHOR_BOTTOMCENTER, D3DCOLOR_COLORALPHA(K_COLOR_DEFAULT_TEXT, wndAlpha));
}

void CMainMenu::PaintChapterWindowLarge(D3DXVECTOR2 vCenter, int nChapterIdx, float fAlpha)
{
	bool bChapterUnlocked = UTGetChaptersList().IsChapterUnlocked(nChapterIdx, g_userData[K_MEMID_MISSIONS_COMPLETED]);
	//coming soon chapter?
	bool bWorkshopChapter = false;

#ifdef ENABLE_STEAM_WORKSHOP
	bWorkshopChapter = (nChapterIdx >= UTGetChaptersList().GetChaptersCnt());
	if (bWorkshopChapter)
		bChapterUnlocked = true;
#endif

	RECTXYWH picrect = m_sprCol.GetAFrameBBox(ANM_MENUS_SPR_CHAPTER_SPLASHES_LG, 0);
	//facem loc pentru mesajele de sub imagine
	RECTXYWH wndrect = picrect;
	wndrect.Inflate(-2, -2);
	wndrect.h += 24;
	int	wndSpacing = 24;

	RECTXYWH wndrectL = wndrect; //local wndrect
	wndrectL.x += vCenter.x - picrect.w / 2.0f;
	wndrectL.y += vCenter.y - wndrect.h / 2.0f;

	//float wndAlpha = D3DCOLOR_GETFALPHA(dwColor);
	//Paint Chapter Window
	CtrlMgrDrawFrame(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_FRAME1, wndrectL, D3DCOLOR_COLORVALUE(fAlpha, fAlpha, fAlpha, 1.0f));
	//mission image
	if(!bWorkshopChapter)
		CSprite::paintFrame(&m_sprCol, wndrectL.x - 2, wndrectL.y - 2, ANM_MENUS_SPR_CHAPTER_SPLASHES_LG, UTGetChaptersList().m_arrChapters[nChapterIdx]->nChapterImgFrame, D3DCOLOR_COLORVALUE(fAlpha, fAlpha, fAlpha, 1.0f));
	else
		CSprite::paintFrame(&m_sprCol, wndrectL.x - 2, wndrectL.y - 2, ANM_MENUS_SPR_WORKSHOP_ART, 0, D3DCOLOR_COLORVALUE(fAlpha, fAlpha, fAlpha, 1.0f));

	int nChapterStars = 0;
	int nCompletedLevels = 0;
	//if unlocked
	if ((bChapterUnlocked) && (!bWorkshopChapter))
	{
		//mission stars
		CSprite::paintFrame(&m_sprCol, wndrectL.x + picrect.w - 10, wndrectL.y + picrect.h - 10, ANM_MENUS_SPR_LEVEL_STARS, 1, D3DCOLOR_COLORVALUE(fAlpha, fAlpha, fAlpha, 1.0f));
		//count stars
		for (int kk = 0; kk < K_GAME_LEVELS_PER_CHAPTER; kk++)
		{
			nChapterStars += g_levelStats[kk + nChapterIdx * K_GAME_LEVELS_PER_CHAPTER].nStars;
			if (g_levelStats[kk + nChapterIdx * K_GAME_LEVELS_PER_CHAPTER].nStars > 0)
				nCompletedLevels++;
		}

		CStringDesc strdesc;
		g_stringsMgr.SetStringDesc(&strdesc, L"%d/%d", nChapterStars, K_GAME_LEVELS_PER_CHAPTER * 3);
		g_font6ns1->DrawString(&strdesc, wndrectL.x + picrect.w - 15, wndrectL.y + picrect.h - 6, FONTFLAG_ANCHOR_BOTTOMRIGHT, D3DCOLOR_COLORALPHA(K_COLOR_DEFAULT_TEXT, fAlpha));
	}
	//mission name frame and string
	RECTXYWH titlerect(wndrectL.x, wndrectL.y + picrect.h + 1, wndrectL.w, 9);
	CtrlMgrDrawFrame(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_FRAME4, titlerect, D3DCOLOR_COLORVALUE(fAlpha, fAlpha, fAlpha, 1.0f));
	titlerect.Inflate(1, 1);
	if (!bWorkshopChapter)
	{
		if (UTGetChaptersList().m_arrChapters[nChapterIdx]->nChapterNameStrIdx >= 0)
			g_font8bs1->DrawStringScaleW(UTGetChaptersList().m_arrChapters[nChapterIdx]->nChapterNameStrIdx, titlerect.CenterX(), titlerect.Bottom() - 3, titlerect.w, FONTFLAG_ANCHOR_BOTTOMCENTER, D3DCOLOR_COLORALPHA(K_COLOR_DEFAULT_TEXT, fAlpha));
	}
	else
	{
		g_font8bs1->DrawStringScaleW(STR_WORKSHOP, titlerect.CenterX(), titlerect.Bottom() - 3, titlerect.w, FONTFLAG_ANCHOR_BOTTOMCENTER, D3DCOLOR_COLORALPHA(K_COLOR_DEFAULT_TEXT, fAlpha));
	}
	//second text: completed or necessary stars
	if (bChapterUnlocked)
	{
		if (!bWorkshopChapter)
		{
			g_stringsMgr.ReplaceTokenInt(STR_TEMP1, STR_MISSIONS_COMPLETED_N, 1, nCompletedLevels);
			g_stringsMgr.ReplaceTokenInt(STR_TEMP1, STR_TEMP1, 2, K_GAME_LEVELS_PER_CHAPTER);
			g_font6n1->DrawString(STR_TEMP1, titlerect.CenterX(), titlerect.y + 20, FONTFLAG_ANCHOR_BOTTOMCENTER, D3DCOLOR_COLORALPHA(K_COLOR_DEFAULT_TEXT, fAlpha));
		}
		else
		{
			g_font6n1->DrawString(STR_DOWNLOADED_LEVELS, titlerect.CenterX(), titlerect.y + 20, FONTFLAG_ANCHOR_BOTTOMCENTER, D3DCOLOR_COLORALPHA(K_COLOR_DEFAULT_TEXT, fAlpha));
		}
	}
	else //show necessary levels to unlock
	{
		if (!bWorkshopChapter)
		{
			g_stringsMgr.ReplaceTokenInt(STR_TEMP1, STR_COMPLETE_N_TO_UNLOCK_N, 1, UTGetChaptersList().m_arrChapters[nChapterIdx]->nChapterMissionsToUnlock);
			g_stringsMgr.ReplaceTokenInt(STR_TEMP1, STR_TEMP1, 2, g_userData[K_MEMID_MISSIONS_COMPLETED]);
			g_stringsMgr.ReplaceTokenInt(STR_TEMP1, STR_TEMP1, 3, UTGetChaptersList().m_arrChapters[nChapterIdx]->nChapterMissionsToUnlock);

			g_font6n1->DrawString(STR_TEMP1, titlerect.CenterX(), titlerect.y + 20, FONTFLAG_ANCHOR_BOTTOMCENTER, D3DCOLOR_COLORALPHA(0xffff8888, fAlpha));
		}
	}
}

void CMainMenu::PaintGameModeWindow(D3DXVECTOR2 vCenter, int nGameModeIdx, float fAlpha)
{
	RECTXYWH picrect = m_sprCol.GetAFrameBBox(ANM_MENUS_SPR_GAME_MODE_SPLASHES, 0);
	//facem loc pentru mesajele de sub imagine
	RECTXYWH wndrect = picrect;
	wndrect.Inflate(-2, -2);
	wndrect.h += 32;

	RECTXYWH wndrectL = wndrect; //local wndrect
	wndrectL.x += vCenter.x - picrect.w / 2.0f;
	wndrectL.y += vCenter.y - wndrect.h / 2.0f;

	DWORD wcol = D3DCOLOR_FFFA(fAlpha);

	RECTXYWH wndrectL2 = wndrectL;
	wndrectL2.Inflate(1, 1);
	CtrlMgrDrawFrame(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_FRAME_BLACK1, wndrectL2, D3DCOLOR_COLORVALUE(fAlpha, fAlpha, fAlpha, 1.0f));

	CSprite::paintFrame(&m_sprCol, wndrectL.x - 2, wndrectL.y - 2, ANM_MENUS_SPR_GAME_MODE_SPLASHES, nGameModeIdx, D3DCOLOR_COLORVALUE(fAlpha, fAlpha, fAlpha, 1.0f));

	int nTitleStrIdx = -1;
	int nSubtitleStrIdx = -1;

	switch (nGameModeIdx)
	{
		case K_MM_MODE_CLASSIC:
		{
			nTitleStrIdx = STR_CLASSIC_MODE;
			nSubtitleStrIdx = STR_CLASSIC_MODE_DESC;
		}
		break;

		case K_MM_MODE_WEEKLY_CHALLENGE:
		{
			nTitleStrIdx = STR_WEEKLY_CHALLENGE;
			nSubtitleStrIdx = STR_WEEKLY_CHALLENGE_DESC;
		}
		break;

		case K_MM_MODE_ZOMBIE_INVASION:
		{
			nTitleStrIdx = STR_ZOMBIE_INVASION;
			nSubtitleStrIdx = STR_ZOMBIE_INVASION_DESC;
		}
		break;

		case K_MM_MODE_VINFINITE:
		{
			nTitleStrIdx = STR_VINFINITE_MODE;
			nSubtitleStrIdx = STR_VINFINITE_MODE_DESC;
		}
		break;
	}
	//draw titles
	if(nTitleStrIdx >= 0)
		g_font8bs1->DrawStringScaleW(nTitleStrIdx, wndrectL.CenterX(), wndrectL.y + picrect.h + 8, wndrect.w, FONTFLAG_ANCHOR_VCENTERHCENTER, wcol);
	if (nSubtitleStrIdx >= 0)
		g_font6ns1->DrawStringScaleW(nSubtitleStrIdx, wndrectL.CenterX(), wndrectL.y + picrect.h + 20, wndrect.w, FONTFLAG_ANCHOR_VCENTERHCENTER, D3DCOLOR_COLORALPHA(K_COLOR_DEFAULT_TEXT, fAlpha));
}

void CMainMenu::Release()
{
	m_sprCol.Release();
}

/*----------------------------------*\
*  System/Framework
\*----------------------------------*/
HRESULT CMainMenu::OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	m_pDevice = pd3dDevice;
	HRESULT hr = S_OK;

	V_RETURN(m_sprCol.OnCreateDevice(pd3dDevice));
	return hr;
}

HRESULT CMainMenu::OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	m_pDevice = pd3dDevice;
	HRESULT hr = S_OK;
	V_RETURN(m_sprCol.OnResetDevice(pd3dDevice));
	return hr;
}

HRESULT CMainMenu::OnLostDevice(void)
{
	HRESULT hr = S_OK;
	m_pDevice = NULL;
	V_RETURN(m_sprCol.OnLostDevice());
	return hr;
}

HRESULT CMainMenu::OnDestroyDevice(void)
{
	m_pDevice = NULL;
	HRESULT hr = S_OK;
	V_RETURN(m_sprCol.OnDestroyDevice());
	return hr;
}
