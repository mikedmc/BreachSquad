#include "dxstdafx.h"


//pozitii relative ale dreptunghiului de selectie pe fiecare fereastra 
//[K_PSS_CURPOS_CNT][x,y,w,h] - relative to the window(topX, topY)
int arrCursorRects[K_PSS_CURPOS_CNT][4] = {
	{44,4,K_PSS_PLAYER_WINDOW_WIDTH - 46,16},		//class
	{44,24,K_PSS_PLAYER_WINDOW_WIDTH - 46,32},		//XP Points
	{2,59,K_PSS_PLAYER_WINDOW_WIDTH - 4,47},		//primary weapon
	{2,109,K_PSS_PLAYER_WINDOW_WIDTH / 2 - 4, 33},		//equipment (left)
	{K_PSS_PLAYER_WINDOW_WIDTH / 2 + 2, 109, K_PSS_PLAYER_WINDOW_WIDTH / 2 - 4, 33},		//gear (right)
	{2,145,K_PSS_PLAYER_WINDOW_WIDTH - 4, 44},		//ultimate
	{29,194,K_PSS_PLAYER_WINDOW_WIDTH - 58,16}		//ready
};

FORCEINLINE int CPlayerSelScr::GetItemsCount(EPSSPlayerClass ePlayerClass, ePSSItemCategory eItemsCategory)
{
	return arrItemsByClass[ePlayerClass].matOptionsByItemType[eItemsCategory].nCount;
}

sPSSItemData* CPlayerSelScr::GetItem(EPSSPlayerClass ePlayerClass, ePSSItemCategory eItemsCategory, int nIndex)
{
	if ((nIndex < 0) || (nIndex >= arrItemsByClass[ePlayerClass].matOptionsByItemType[eItemsCategory].nCount))
	{
		ErrorBox(K_ERR_WARNING, L"CPlayerSelScr::GetItem invalid index[%d] for cat[%d]. Defaulting on 0", nIndex, (int)eItemsCategory);
		//default on first option
		return &arrItemsByClass[ePlayerClass].matOptionsByItemType[eItemsCategory].m_pData[0];	
	}
	return &arrItemsByClass[ePlayerClass].matOptionsByItemType[eItemsCategory].m_pData[nIndex];	
}

void CPlayerSelScr::ResetSelection(bool bResetInstanceIDs)
{
	//face doar o resetare partiala ca sa nu apara selectati players
	m_nPlayersCnt = 0;
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if (bResetInstanceIDs)
		{
			m_arrPlayers[kk].nInstanceID = -1;
		}
		//counts the active players and see if the controller is still present
		if (m_arrPlayers[kk].nInstanceID != -1)
		{
			if (null != UTGetCtrlrMgr().GetControllerByInstanceID(m_arrPlayers[kk].nInstanceID))
				m_nPlayersCnt++;  //controller still there
			else
				m_arrPlayers[kk].nInstanceID = -1; //must press fire again to select new controller
		}

		m_arrPlayers[kk].bSelected = false;
		m_arrPlayers[kk].nCursorPosReal = K_PSS_CURPOS_READY;
		//set verse timer
		m_arrPlayers[kk].fVerseReadyTimer = K_PSS_WAIT_BEFORE_VERSE_SEC - EPS;
		//details window
		m_arrPlayers[kk].nCursorMoreReal = -1;
		m_arrPlayers[kk].fAnimCursor = 0.0f;

		///--- load selection from user data for local players ---
		if ((m_arrPlayers[kk].nInstanceID != -1) && (!m_arrPlayers[kk].bIsNetworkPlayer))
		{
			LoadSelectionForPanel(kk);
		}
		//net players keep old setting, they just update weapon options until we receive a valid network message
		if (m_arrPlayers[kk].bIsNetworkPlayer)
		{
			SetWeaponsOptions(&m_arrPlayers[kk], false);
			//set the rest of the selection on empty until updated or they will appear unlocked before we receive the message
			//#TODO: ar trbeui sa fie un flag (selection on -1) care sa imi spuna ca nu am primit update din retea si sa nu desenez nimic la arme (sau sa desenez fereastra inchisa)
			//acum se deseneaza default selection fara arme si updates pana cand primeste mesajul de pe retea.
			for (int ll = 0; ll < PSS_ITEMCATS_COUNT; ll++)
			{
				m_arrPlayers[kk].nSelection[ll] = 0;
			}
			//load local points invested into upgrades
			m_arrPlayers[kk].nPlayerXPPts = 0;
			for (int ll = 0; ll < K_PSS_UPGRADE_BARS_CNT; ll++)
			{
				m_arrPlayers[kk].arrUpgradeBarsPts[ll] = 0;
			}
		}
		//update lock flag on selections
		SetSelectionPrices(&m_arrPlayers[kk]);
	}
}

void CPlayerSelScr::OnControllerRemoved(int ctrlrInstanceID)
{
	int nAffectedIdx = -1;
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if (m_arrPlayers[kk].nInstanceID == ctrlrInstanceID)
		{
			nAffectedIdx = kk;
			break;
		}
	}

	//remove selected characters on local game
	if (nAffectedIdx >= 0)
	{
		if (!UTApp().IsGameNetworked())
		{
			g_playerSelScr.ResetSelection(true);
		}
		else
		{
			// on networked games just move it to default keyboard so we don't break the flow
			m_arrPlayers[nAffectedIdx].nInstanceID = K_CM_IID_KBM1;
		}
	}
}

int CPlayerSelScr::GetUpgradeBarIdx(const WCHAR* sBarName)
{
	CStringHash strh;
	strh.Init(sBarName);
	for (int kk = 0; kk < m_arrUpgradeBars.GetSize(); kk++)
	{
		if (m_arrUpgradeBars[kk]->shUID.getHash() == strh.getHash())
			return kk;
	}

	ErrorBox(K_ERR_WARNING, L"[WARNING] CPlayerSelScr::GetUpgradeBarIdx: Couldn't find upgrade bar with name [%s]", strh.text);
	return -1;
}

CPlayerSelScr::CPlayerSelScr()
{
	m_pDevice = null;
	fLocalTimeline = 0.0f;
	//resetare completa la prima initializare	
	m_nPlayersCnt = 0;
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		m_arrPlayers[kk].Init();
	}
}

CPlayerSelScr::~CPlayerSelScr()
{
	ReleaseSprites();
	//release upgrade bars collection 
	SAFE_DELETE_GROWABLE_ARRAY(m_arrUpgradeBars);
}

HRESULT CPlayerSelScr::InitSprites(WCHAR * strPath)
{
	fLocalTimeline = 0.0f;
	V_OP_RETHR(m_sprCol.LoadSprites(strPath));
	return S_OK;
}

void CPlayerSelScr::Update(float dTime)
{
	//update local timeline
	fLocalTimeline += dTime;

	//pentru jocurile multiplayer asteptam de pe retea date de selectie
	if (UTApp().IsGameNetworked())
	{
		CNetLock::sPacketPlayerSelection netSel;

		// update and query current lobby status
		const INetwork::sLobby& lobby = g_pNetwork->GetCurrentLobby();
		if (lobby.eState != INetwork::LOBBY_IN_LOBBY || lobby.iNumPlayers != 2)
		{
			return;
		}

		//buffer
		unsigned int bufferSize = 0;
		unsigned char buffer[INetwork::MAX_UNRELIABLE_PACKET_SIZE];

		//read one by one and process network input
		while (g_pNetwork->Recv(buffer, INetwork::MAX_UNRELIABLE_PACKET_SIZE, &bufferSize))
		{
			if (bufferSize > 0)
			{
				//decomprimam datele din pachet si verificam autenticitatea
				BitPacker stream(buffer, bufferSize);
				
				CNetLock::sPacketHeader packHead;
				if (packHead.Deserialize(stream) == false)
				{
					//change game state
					CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE);
					nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
					nevent->AddNamedArgINT32(L"stateErrorStrIdx", STR_NETWORK_ERROR_WRONG_VERSION);
					UTGetEventManager().TriggerEvent(nevent);

					LOG(L"[Error] PlayerSelScr.Update::sPacketHeader.Deserialize - Different game versions! Please update to the last version!");
					return;
				}

				//read type of command
				CNetLock::eNetCommand cmd = (CNetLock::eNetCommand)stream.ReadUChar();
				//tratam doar mesajele de tip comanda
				if ((packHead.eMessageType == CNetLock::K_NETMSG_TYPE_COMMAND) && (cmd == CNetLock::K_NETCMD_PLAYER_SELECTION))
				{
					//right kind of message, decode it
					netSel.Deserialize(stream);
					LOG(L"Net:playerSelScreen received: playerType:%d bSelected:%d", netSel.eType, netSel.bSelected);
					//if not hosting the game read random seed and selected chapter/level and save back into CNetlock class
					if (!g_netlock.Net_GetIAmHosting())
					{
						g_netlock.m_unRandomSeed = stream.ReadUInt();
						g_netlock.m_ucSelMode = stream.ReadUChar();
						g_netlock.m_ucSelChapter = stream.ReadUChar();
						g_netlock.m_ucSelLevel = stream.ReadUChar();
						LOG(L"Net:playerSelScreen slave received randseed(%d) mode(%d) chapter(%d) and level(%d).", g_netlock.m_unRandomSeed, g_netlock.m_ucSelMode, g_netlock.m_ucSelChapter, g_netlock.m_ucSelLevel);
					}

					//find correct position for net player
					int nPeerOrdinal = g_netlock.Net_GetOtherPlayerIndex();
					//daca nu exista il initializam acum
					if (m_arrPlayers[nPeerOrdinal].nInstanceID == -1)
					{
						if (m_arrPlayers[nPeerOrdinal].eType == K_PSS_CLASS_NOT_SELECTED)
						{
							m_arrPlayers[nPeerOrdinal].Init();
							m_arrPlayers[nPeerOrdinal].eType = K_PSS_CLASS_ASSAULTER;
							SetWeaponsOptions(&m_arrPlayers[nPeerOrdinal]);
							m_arrPlayers[nPeerOrdinal].fVerseReadyTimer = K_PSS_WAIT_BEFORE_VERSE_SEC - EPS;
						}
						//setam instance ID ca sa legam user de controller
						m_arrPlayers[nPeerOrdinal].nInstanceID = K_CM_IID_NET1;
						m_arrPlayers[nPeerOrdinal].bIsNetworkPlayer = true;
						//increase players cnt
						m_nPlayersCnt++;
						assert((m_nPlayersCnt >= 0) && (m_nPlayersCnt <= K_MAX_PLAYERS_CNT));
					}

					//save data from message
					if ((EPSSPlayerClass)netSel.eType != m_arrPlayers[nPeerOrdinal].eType)
					{
						m_arrPlayers[nPeerOrdinal].eType = (EPSSPlayerClass)netSel.eType;
						SetWeaponsOptions(&m_arrPlayers[nPeerOrdinal]);
						m_arrPlayers[nPeerOrdinal].fVerseReadyTimer = 0.0f;
					}
					m_arrPlayers[nPeerOrdinal].bSelected = (bool)netSel.bSelected;
					m_arrPlayers[nPeerOrdinal].nCursorPosReal = (int)netSel.nCursorPos;
					for (int kk = 0; kk < PSS_ITEMCATS_COUNT; kk++)
					{
						m_arrPlayers[nPeerOrdinal].nSelection[kk] = netSel.nWpnSelection[kk];
						m_arrPlayers[nPeerOrdinal].nPrice[kk] = netSel.nWpnSelectionPrice[kk];
					}
					//read XP data
					m_arrPlayers[nPeerOrdinal].nPlayerXPPts = (int)netSel.nPlayerXPpoints;
					for (int kk = 0; kk < K_PSS_UPGRADE_BARS_CNT; kk++)
					{
						m_arrPlayers[nPeerOrdinal].arrUpgradeBarsPts[kk] = (int)netSel.arrUpgradeBarsPts[kk];
					}
					//save networked stars number
					UTLang().SetString(STR_PEER_STARS_VAL, L"%d", (int)netSel.nPlayerStars);
				}
				else
				{
					LOG(L"Net:playerSelScreen wrong packet type(%d) cmd(%d)", packHead.eMessageType, cmd);
				}
			}
		}
	}

	//aleg din controllere doar comenzile necesare clasei
	bool bBackProcessed = false;
	bool bBackPressed = false;
	for (int kk = 0; kk < UTGetCtrlrMgr().m_arrControllers.size(); kk++)
	{
		//pt fiecare controller resetez comanda si instanceID
		EPSSInputCommand eCommand = K_PSS_COMMAND_NONE;
		int	nInstanceID = -1;

		CController* ctrlr = UTGetCtrlrMgr().m_arrControllers[kk];

		if ((ctrlr->GetButState(K_CM_COMMAND_MOVE_X) == K_CM_BUTSTATE_JUSTPRESSED) && (ctrlr->GetAxisVal(K_CM_COMMAND_MOVE_X) < 0.0f))
			eCommand = K_PSS_COMMAND_LEFT;
		if ((ctrlr->GetButState(K_CM_COMMAND_MOVE_X) == K_CM_BUTSTATE_JUSTPRESSED) && (ctrlr->GetAxisVal(K_CM_COMMAND_MOVE_X) > 0.0f))
			eCommand = K_PSS_COMMAND_RIGHT;
		if ((ctrlr->GetButState(K_CM_COMMAND_MOVE_Y) == K_CM_BUTSTATE_JUSTPRESSED) && (ctrlr->GetAxisVal(K_CM_COMMAND_MOVE_Y) < 0.0f))
			eCommand = K_PSS_COMMAND_UP;
		if ((ctrlr->GetButState(K_CM_COMMAND_MOVE_Y) == K_CM_BUTSTATE_JUSTPRESSED) && (ctrlr->GetAxisVal(K_CM_COMMAND_MOVE_Y) > 0.0f))
			eCommand = K_PSS_COMMAND_DOWN;
		if ((ctrlr->sCommands.keyState[K_CM_COMMAND_FIRE1] == K_CM_BUTSTATE_JUSTPRESSED) || 
			(ctrlr->sCommands.keyState[K_CM_COMMAND_JUMP] == K_CM_BUTSTATE_JUSTPRESSED))
			eCommand = K_PSS_COMMAND_SELECT;
		//cand apesi back se duce inapoi in ecranul de mission select
		if ((ctrlr->sCommands.keyState[K_CM_COMMAND_BACK] == K_CM_BUTSTATE_JUSTPRESSED) ||
			(ctrlr->sCommands.keyState[K_CM_COMMAND_RELOAD] == K_CM_BUTSTATE_JUSTPRESSED)	||
			(ctrlr->sCommands.keyState[K_CM_COMMAND_MELEE] == K_CM_BUTSTATE_JUSTPRESSED) )
		{
			eCommand = K_PSS_COMMAND_BACK;
			bBackPressed = true;
		}
		//daca am comanda salvez intanceID
		if (eCommand != K_PSS_COMMAND_NONE)
			nInstanceID = ctrlr->nSDLInstanceId;

		//daca am comanda vad ce player primeste comanda sau adauga player
		if ((eCommand != K_PSS_COMMAND_NONE) && (nInstanceID != -1))
		{
			int nPlayerIdx = -1;
			//trec prin toti playerii pentru ca pe retea pot fi adaugat pe index [1] desi e doar un player adaugat (adica eu ca si SLAVE)
			for (int ll = 0; ll < K_MAX_PLAYERS_CNT; ll++)
			{
				if (m_arrPlayers[ll].nInstanceID == nInstanceID)
				{
					nPlayerIdx = ll; //am gasit playerul
					break;
				}
			}

			///--- ADDING NEW PLAYER ---
			//networked playerul local il adaugam mereu acolo unde este necesar (0-master, 1-slave)
			if (UTApp().IsGameNetworked())
			{
				if ((nPlayerIdx == -1) && (eCommand == K_PSS_COMMAND_SELECT))
				{
					int nPlayerOrdinal = g_netlock.Net_GetPlayerIndex();
					//daca nu e deja adaugat player, adaugam acum o singura data, primul controller care face SELECT
					if (m_arrPlayers[nPlayerOrdinal].nInstanceID == -1)
					{
						//set player idx to check
						int nPlayerIdx = nPlayerOrdinal;

						if (m_arrPlayers[nPlayerIdx].eType == K_PSS_CLASS_NOT_SELECTED)
						{
							m_arrPlayers[nPlayerIdx].Init();
							m_arrPlayers[nPlayerIdx].eType = K_PSS_CLASS_ASSAULTER;
						}

						//load last selection ALWAYS
						m_arrPlayers[nPlayerIdx].eType = (EPSSPlayerClass)g_userData[K_MEMID_PANEL1_CLASS + nPlayerIdx * (K_MEMID_PANEL2_CLASS - K_MEMID_PANEL1_CLASS)];
						//set the rest of the selection
						for (int ll = 0; ll < PSS_ITEMCATS_COUNT; ll++)
						{
							int nDataOff = K_MEMID_PANEL1_CLASSDATA_START + nPlayerIdx * (K_MEMID_PANEL2_CLASS - K_MEMID_PANEL1_CLASS) + m_arrPlayers[nPlayerIdx].eType * 5;
							m_arrPlayers[nPlayerIdx].nSelection[ll] = g_userData[ll + nDataOff];
							//make sure selection fits data (for modding)
							int nItemsCnt = arrItemsByClass[m_arrPlayers[nPlayerIdx].eType].matOptionsByItemType[ll].nCount;
							if ((m_arrPlayers[nPlayerIdx].nSelection[ll] < 0) || (m_arrPlayers[nPlayerIdx].nSelection[ll] >= nItemsCnt))
							{
								m_arrPlayers[nPlayerIdx].nSelection[ll] = 0;
							}
						}

						//setam instance ID ca sa legam user de controller
						m_arrPlayers[nPlayerIdx].nInstanceID = nInstanceID;
						m_arrPlayers[nPlayerIdx].bIsNetworkPlayer = false;
						m_arrPlayers[nPlayerIdx].fVerseReadyTimer = K_PSS_WAIT_BEFORE_VERSE_SEC - EPS;

						//load local points invested into upgrades
						InitUpgradeBars(&m_arrPlayers[nPlayerIdx]);
						//update lock flag on selections
						SetSelectionPrices(&m_arrPlayers[nPlayerIdx]);
						//reset command
						eCommand = K_PSS_COMMAND_NONE;
						//send command
						SendSelectionByNetwork(nPlayerIdx);
						
						m_nPlayersCnt++;
						assert((m_nPlayersCnt >= 0) && (m_nPlayersCnt <= K_MAX_PLAYERS_CNT));
					}
				}
			}
			else //pe modul NOT networked se aloca dinamic first come first served
			{
				//daca nu avem playerul in lista il adaugam daca mai este loc
				if ((nPlayerIdx == -1) && (eCommand == K_PSS_COMMAND_SELECT) && (m_nPlayersCnt < K_MAX_PLAYERS_CNT))
				{
					//player index to check
					int nPlayerIdx = m_nPlayersCnt;
					//sunt deja initializate dinainte?
					if (m_arrPlayers[nPlayerIdx].eType == K_PSS_CLASS_NOT_SELECTED)
					{
						m_arrPlayers[nPlayerIdx].Init();
						m_arrPlayers[nPlayerIdx].eType = K_PSS_CLASS_ASSAULTER;
					}

					//load last selection ALWAYS
					m_arrPlayers[nPlayerIdx].eType = (EPSSPlayerClass)g_userData[K_MEMID_PANEL1_CLASS + nPlayerIdx * (K_MEMID_PANEL2_CLASS - K_MEMID_PANEL1_CLASS)];
					//set the rest of the selection
					for (int ll = 0; ll < PSS_ITEMCATS_COUNT; ll++)
					{
						int nDataOff = K_MEMID_PANEL1_CLASSDATA_START + nPlayerIdx * (K_MEMID_PANEL2_CLASS - K_MEMID_PANEL1_CLASS) + m_arrPlayers[nPlayerIdx].eType * 5;
						m_arrPlayers[nPlayerIdx].nSelection[ll] = g_userData[ll + nDataOff];
						//make sure selection fits data (for modding)
						int nItemsCnt = arrItemsByClass[m_arrPlayers[nPlayerIdx].eType].matOptionsByItemType[ll].nCount;
						if ((m_arrPlayers[nPlayerIdx].nSelection[ll] < 0) || (m_arrPlayers[nPlayerIdx].nSelection[ll] >= nItemsCnt))
						{
							m_arrPlayers[nPlayerIdx].nSelection[ll] = 0;
						}
					}

					//setam instance ID ca sa legam user de controller
					m_arrPlayers[nPlayerIdx].nInstanceID = nInstanceID;
					m_arrPlayers[nPlayerIdx].bIsNetworkPlayer = false;
					m_arrPlayers[nPlayerIdx].fVerseReadyTimer = K_PSS_WAIT_BEFORE_VERSE_SEC - EPS;
					
					InitUpgradeBars(&m_arrPlayers[nPlayerIdx]);
					//update lock flag on selections
					SetSelectionPrices(&m_arrPlayers[nPlayerIdx]);
					//reset command
					eCommand = K_PSS_COMMAND_NONE;

					//advance player
					m_nPlayersCnt++;
					assert((m_nPlayersCnt >= 0) && (m_nPlayersCnt <= K_MAX_PLAYERS_CNT));
				}
			}

			//daca avem playerul procesam comanda
			if ((nPlayerIdx != -1) && (eCommand != K_PSS_COMMAND_NONE))
			{
				//only allow CANCEL if selected
				if ((m_arrPlayers[nPlayerIdx].bSelected) && (eCommand != K_PSS_COMMAND_BACK))
					eCommand = K_PSS_COMMAND_NONE;

				CPlayerCharSelection* playersel = &m_arrPlayers[nPlayerIdx];
				bool bDetailsWndOpen = (playersel->nCursorMoreReal < 0) ? false : true;
				//command pressed
				playersel->fTimeSinceCursorMoved = 0.0f;

				//switch (playersel->nCursorPosReal)
				//{
				//	case K_PSS_CURPOS_CLASS: //class change
				//	{
				//		int pltype = playersel->eType;
				//		if ((seldir == 0) || ((seldir < 0) && (pltype == 0)) || ((seldir > 0) && (pltype >= K_PSS_CLASSES_COUNT - 1)))
				//			break;
				//		
				//		//save the selection in memory before changing the class
				//		for (int ll = 0; ll < PSS_ITEMCATS_COUNT; ll++)
				//		{
				//			int nDataOff = K_MEMID_PANEL1_CLASSDATA_START + nPlayerIdx * (K_MEMID_PANEL2_CLASS - K_MEMID_PANEL1_CLASS) + m_arrPlayers[nPlayerIdx].eType * 5;
				//			g_userData[ll + nDataOff] = m_arrPlayers[nPlayerIdx].nSelection[ll];
				//		}

				//		pltype += seldir;
				//		CLAMP(pltype, 0, (int)K_PSS_CLASSES_COUNT - 1);
				//		m_arrPlayers[nPlayerIdx].eType = (EPSSPlayerClass)pltype;
				//		//set the rest of the selection from memory
				//		for (int ll = 0; ll < PSS_ITEMCATS_COUNT; ll++)
				//		{
				//			int nDataOff = K_MEMID_PANEL1_CLASSDATA_START + nPlayerIdx * (K_MEMID_PANEL2_CLASS - K_MEMID_PANEL1_CLASS) + m_arrPlayers[nPlayerIdx].eType * 5;
				//			m_arrPlayers[nPlayerIdx].nSelection[ll] = g_userData[ll + nDataOff];
				//		}

				//		InitUpgradeBars(&m_arrPlayers[nPlayerIdx]);

				//		SetSelectionPrices(&m_arrPlayers[nPlayerIdx]);
				//		m_arrPlayers[nPlayerIdx].fVerseReadyTimer = 0.0f;

				//		//when networked play send all commands
				//		if (UTGetAppClass().IsGameNetworked())
				//			SendSelectionByNetwork(nPlayerIdx);
				//	}
				//	break;

				//	case K_PSS_CURPOS_ULTIMATE:
				//	case K_PSS_CURPOS_WEAPON: 
				//	case K_PSS_CURPOS_EQUIPMENT:
				//	case K_PSS_CURPOS_GEAR: 
				//	{
				//		ePSSItemCategory itemtype = (ePSSItemCategory)(playersel->nCursorPosReal - 1);
				//		playersel->nSelection[itemtype] += seldir;
				//		CLAMP(playersel->nSelection[itemtype], 0, arrItemsByClass[playersel->eType].matOptionsByItemType[itemtype].Count() - 1);
				//		//update lock flag on selections
				//		SetSelectionPrices(&m_arrPlayers[nPlayerIdx]);
				//		//when networked play send all commands
				//		if (UTGetAppClass().IsGameNetworked())
				//			SendSelectionByNetwork(nPlayerIdx);
				//	}
				//	break;
				//}

				if (!bDetailsWndOpen)
				{
					//treat back command
					if (eCommand == K_PSS_COMMAND_BACK)
					{
						if ((nPlayerIdx != -1) && (m_arrPlayers[nPlayerIdx].bSelected == true))
						{
							m_arrPlayers[nPlayerIdx].bSelected = false;
							//when networked play send all commands
							if (UTApp().IsGameNetworked())
								SendSelectionByNetwork(nPlayerIdx);

							bBackProcessed = true;
						}
					}
					else if (eCommand == K_PSS_COMMAND_LEFT)
					{
						if (playersel->nCursorPosReal == K_PSS_CURPOS_GEAR)
							playersel->nCursorPosReal = K_PSS_CURPOS_EQUIPMENT;
						//when networked play send all commands
						if (UTApp().IsGameNetworked())
							SendSelectionByNetwork(nPlayerIdx);
					}
					else if (eCommand == K_PSS_COMMAND_RIGHT)
					{
						if (playersel->nCursorPosReal == K_PSS_CURPOS_EQUIPMENT)
							playersel->nCursorPosReal = K_PSS_CURPOS_GEAR;
						//when networked play send all commands
						if (UTApp().IsGameNetworked())
							SendSelectionByNetwork(nPlayerIdx);
					}
					else if (eCommand == K_PSS_COMMAND_DOWN)
					{
						if (playersel->nCursorPosReal == K_PSS_CURPOS_EQUIPMENT)
							playersel->nCursorPosReal = K_PSS_CURPOS_ULTIMATE;
						else if (playersel->nCursorPosReal >= K_PSS_CURPOS_CNT - 1)
						{
							playersel->nCursorPosReal = 0;
						}
						else if (playersel->nCursorPosReal < K_PSS_CURPOS_CNT - 1)
						{
							playersel->nCursorPosReal++;
						}
						//when networked play send all commands
						if (UTApp().IsGameNetworked())
							SendSelectionByNetwork(nPlayerIdx);
					}
					else if (eCommand == K_PSS_COMMAND_UP)
					{
						if (playersel->nCursorPosReal == K_PSS_CURPOS_ULTIMATE)
							playersel->nCursorPosReal = K_PSS_CURPOS_EQUIPMENT;
						else if (playersel->nCursorPosReal == K_PSS_CURPOS_GEAR)
							playersel->nCursorPosReal = K_PSS_CURPOS_WEAPON;
						else if (playersel->nCursorPosReal == 0)
						{
							playersel->nCursorPosReal = K_PSS_CURPOS_CNT - 1;
						}
						else if (playersel->nCursorPosReal > 0)
						{
							playersel->nCursorPosReal--;
						}
						//when networked play send all commands
						if (UTApp().IsGameNetworked())
							SendSelectionByNetwork(nPlayerIdx);
					}
					//select command
					else if (eCommand == K_PSS_COMMAND_SELECT)
					{
						SND_PLAY(SNDIDX_CLICK);

						switch (playersel->nCursorPosReal)
						{
							case K_PSS_CURPOS_CLASS:
							{
								playersel->nCursorMoreReal = (int)playersel->eType;
							}
							break;
							case K_PSS_CURPOS_XP_POINTS:
							{
								CCtrlLayer* layer = UTGetGUI().ShowLayerOnce("LAYER_ID_PLAYER_UPGRADE");
								if (layer)
								{
									int nPlayerType = (int)playersel->eType;
									int nXPPoints = App_GetAvailableXPPoints(playersel->eType);
									//write points to string
									UTLang().SetString(STR_UNUSED_POINTS_VAL, L"%d", nXPPoints);
									//erase description
									UTLang().SetString(STR_SELECTED_PERK_DESC_VAL, L" ");
									//portrait
									CControl* ctrl = null;
									ctrl = layer->GetControlByName("CTRL_ANIM_PORTRAIT");
									if (ctrl)
									{
										ctrl->paramsDict.SetNamedVarINT32(L"setFrame", nPlayerType);
										//change portrait animation depending on player slot (left vs right)
										if(nPlayerIdx == 0)
											ctrl->paramsDict.SetNamedVarINT32(L"animID", ANM_CONTROLS_SPR_PLAYER_PORTRAITS);
										else
											ctrl->paramsDict.SetNamedVarINT32(L"animID", ANM_CONTROLS_SPR_PLAYER_PORTRAITS_R);
									}
									//class name
									ctrl = layer->GetControlByName("CTRL_LABEL_CLASS");
									if (ctrl)
									{
										ctrl->paramsDict.SetNamedVarINT32(L"stringID", STR_PLAYER_CLASS_ASSAULTER + nPlayerType);
									}
									
									//XP bar to fixed value
									ctrl = layer->GetControlByName("CTRL_XPBAR_XP");
									if (ctrl)
									{
										int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + nPlayerType;
										ctrl->paramsDict.SetNamedVarINT32(L"nOldValue", g_userData[nPlBaseIdx]);
										ctrl->paramsDict.SetNamedVarINT32(L"nNewValue", g_userData[nPlBaseIdx]);
									}
									//Upgrade ctrl
									ctrl = layer->GetControlByName("CTRLID_UPGRADE_PLAYER");
									if (ctrl)
									{
										int nSelectedLine = K_PSS_UPGRADE_BARS_CNT - 1;
										int nBarIdx = g_playerSelScr.arrItemsByClass[nPlayerType].arrUpgradeBarsIdx[nSelectedLine];
										int nFilled = g_userData[K_MEMID_UPGRADE_BAR_POINTS_START + g_playerSelScr.m_arrUpgradeBars[nBarIdx]->nMemSlot];
										int nDots = g_playerSelScr.m_arrUpgradeBars[nBarIdx]->nTotalPoints;
										int nSelectedPoint = nFilled - 1;
										CLAMP(nSelectedPoint, 0, nDots - 1);

										ctrl->paramsDict.SetNamedVarINT32(L"nPlayerOrdinal", nPlayerIdx);
										ctrl->paramsDict.SetNamedVarINT32(L"nSelectedLine", nSelectedLine);
										ctrl->paramsDict.SetNamedVarINT32(L"nSelectedPoint", nSelectedPoint);
									}
								}
							}
							break;
							case K_PSS_CURPOS_WEAPON:
							{
								playersel->nCursorMoreReal = playersel->nSelection[PSS_ITEMCAT_WEAPON];
								playersel->fAnimCursor = 0.0f;
							}
							break;
							case K_PSS_CURPOS_EQUIPMENT:
							{
								playersel->nCursorMoreReal = playersel->nSelection[PSS_ITEMCAT_EQUIPMENT];
								playersel->fAnimCursor = 0.0f;
							}
							break;
							case K_PSS_CURPOS_GEAR:
							{
								playersel->nCursorMoreReal = playersel->nSelection[PSS_ITEMCAT_GEAR];
								playersel->fAnimCursor = 0.0f;
							}
							break;
							case K_PSS_CURPOS_ULTIMATE:
							{
								playersel->nCursorMoreReal = playersel->nSelection[PSS_ITEMCAT_ULTIMATE];
								playersel->fAnimCursor = 0.0f;
							}
							break;
							case K_PSS_CURPOS_READY:
							{
								bool bCancelSelection = false;
								//make sure we can't start the game if we didn't buy the selected weapons
								int wpnPrice = 0;
								UINT32 wpnNameHash = 0;
								for (int kk = 0; kk < PSS_ITEMCATS_COUNT; kk++)
								{
									sPSSItemData* pItem = &arrItemsByClass[playersel->eType].matOptionsByItemType[kk].m_pData[playersel->nSelection[kk]];
									wpnNameHash = pItem->shName.getHash();
									wpnPrice = UTGetShop().GetItemPrice(wpnNameHash);
									if (wpnPrice > 0)
									{
										//FAILSAFE: should never get here (you can only select already unlocked items). Just select the first one from the list.
										LOG(L"[WARNING] Locked weapon was selected! nSelection[%d]=%d price:%d", kk, playersel->nSelection[kk], wpnPrice);
										playersel->nSelection[kk] = 0;
										SetSelectionPrices(&m_arrPlayers[nPlayerIdx]);
										//when networked play send all commands
										if (UTApp().IsGameNetworked())
											SendSelectionByNetwork(nPlayerIdx);
										break;
									}
								}

								if (wpnPrice > 0)
								{
									bCancelSelection = true;
									SND_PLAY(SNDIDX_DENIED);
								}

								//3. no windows shown, select gear
								if (!bCancelSelection)
								{
									m_arrPlayers[nPlayerIdx].bSelected = true;

									//when networked play send all commands
									if (UTApp().IsGameNetworked())
										SendSelectionByNetwork(nPlayerIdx);
								}
							}
							break;
						}
					}

				}
				else  //details window open
				{
					//switch by open details window
					ePSSItemCategory itemtype = PSS_ITEMCAT_NOT_SET;
					switch (playersel->nCursorPosReal)
					{
						case K_PSS_CURPOS_CLASS:
						{
							int seldir = 0;
							if (eCommand == K_PSS_COMMAND_UP)
							{
								seldir = -1;
							}
							else if (eCommand == K_PSS_COMMAND_DOWN)
							{
								seldir = 1;
							}
							else if (eCommand == K_PSS_COMMAND_BACK)
							{
								SND_PLAY(SNDIDX_DENIED);
								//compute selection delta when canceling so it returns to prev selection saved in nCursorMoreReal
								seldir = playersel->nCursorMoreReal - (int)playersel->eType;
								//close MORE window
								playersel->nCursorMoreReal = -1;
								playersel->fAnimCursor = 0.0f;

								bBackProcessed = true;
							}

							//made a change
							if (seldir != 0)
							{
								SND_PLAY(SNDIDX_CLICK);

								int pltype = playersel->eType;
								//save the selection in memory before changing the class
								for (int ll = 0; ll < PSS_ITEMCATS_COUNT; ll++)
								{
									int nDataOff = K_MEMID_PANEL1_CLASSDATA_START + nPlayerIdx * (K_MEMID_PANEL2_CLASS - K_MEMID_PANEL1_CLASS) + m_arrPlayers[nPlayerIdx].eType * 5;
									g_userData[ll + nDataOff] = m_arrPlayers[nPlayerIdx].nSelection[ll];
								}

								pltype += seldir;
								if (pltype < 0) pltype = (int)K_PSS_CLASSES_COUNT - 1;
								if (pltype > (int)K_PSS_CLASSES_COUNT - 1) pltype = 0;

								playersel->fAnimCursor = (float)seldir;

								m_arrPlayers[nPlayerIdx].eType = (EPSSPlayerClass)pltype;
								//set the rest of the selection from memory
								for (int ll = 0; ll < PSS_ITEMCATS_COUNT; ll++)
								{
									int nDataOff = K_MEMID_PANEL1_CLASSDATA_START + nPlayerIdx * (K_MEMID_PANEL2_CLASS - K_MEMID_PANEL1_CLASS) + m_arrPlayers[nPlayerIdx].eType * 5;
									m_arrPlayers[nPlayerIdx].nSelection[ll] = g_userData[ll + nDataOff];
								}

								InitUpgradeBars(&m_arrPlayers[nPlayerIdx]);

								SetSelectionPrices(&m_arrPlayers[nPlayerIdx]);
								//m_arrPlayers[nPlayerIdx].fVerseReadyTimer = 0.0f;

								//when networked play send all commands
								if (UTApp().IsGameNetworked())
									SendSelectionByNetwork(nPlayerIdx);
							}

							//close details and send selection again
							else if (eCommand == K_PSS_COMMAND_SELECT)
							{
								//SND_PLAY(SNDIDX_RELOAD_TACTICAL);

								playersel->nCursorMoreReal = -1;

								InitUpgradeBars(&m_arrPlayers[nPlayerIdx]);

								SetSelectionPrices(&m_arrPlayers[nPlayerIdx]);
								m_arrPlayers[nPlayerIdx].fVerseReadyTimer = 0.0f;

								//when networked play send all commands
								if (UTApp().IsGameNetworked())
									SendSelectionByNetwork(nPlayerIdx);
							}
						}
						break;

						case K_PSS_CURPOS_WEAPON:
							itemtype = PSS_ITEMCAT_WEAPON;
							break;

						case K_PSS_CURPOS_EQUIPMENT:
							itemtype = PSS_ITEMCAT_EQUIPMENT;
							break;

						case K_PSS_CURPOS_GEAR:
							itemtype = PSS_ITEMCAT_GEAR;
							break;

						case K_PSS_CURPOS_ULTIMATE:
							itemtype = PSS_ITEMCAT_ULTIMATE;
							break;

						default:
							playersel->nCursorMoreReal = -1;
							break;
					}

					//execute common commands
					if (itemtype != PSS_ITEMCAT_NOT_SET)
					{
						if (eCommand == K_PSS_COMMAND_UP)
						{
							SND_PLAY(SNDIDX_CLICK);

							playersel->nSelection[itemtype]--;
							if (playersel->nSelection[itemtype] < 0)
								playersel->nSelection[itemtype] = arrItemsByClass[playersel->eType].matOptionsByItemType[itemtype].Count() - 1;
							playersel->fAnimCursor = -1.0f;
							//update lock flag on selections
							SetSelectionPrices(&m_arrPlayers[nPlayerIdx]);
						}
						else if (eCommand == K_PSS_COMMAND_DOWN)
						{
							SND_PLAY(SNDIDX_CLICK);

							playersel->nSelection[itemtype]++;
							playersel->fAnimCursor = 1.0f;
							if (playersel->nSelection[itemtype] > arrItemsByClass[playersel->eType].matOptionsByItemType[itemtype].Count() - 1)
								playersel->nSelection[itemtype] = 0;
							//update lock flag on selections
							SetSelectionPrices(&m_arrPlayers[nPlayerIdx]);
						}
						else if (eCommand == K_PSS_COMMAND_BACK)
						{
							SND_PLAY(SNDIDX_DENIED);
							//reset selection
							playersel->nSelection[itemtype] = playersel->nCursorMoreReal;
							CLAMP(playersel->nSelection[itemtype], 0, arrItemsByClass[playersel->eType].matOptionsByItemType[itemtype].Count() - 1);
							//update lock flag on selections
							SetSelectionPrices(&m_arrPlayers[nPlayerIdx]);
							//when networked play send all commands
							if (UTApp().IsGameNetworked())
								SendSelectionByNetwork(nPlayerIdx);
							//close details wnd
							playersel->nCursorMoreReal = -1;
							playersel->fAnimCursor = 0.0f;

							bBackProcessed = true;
						}
						else if (eCommand == K_PSS_COMMAND_SELECT)
						{
							sPSSItemData* pItem = &arrItemsByClass[playersel->eType].matOptionsByItemType[itemtype].m_pData[playersel->nSelection[itemtype]];
							UINT32 wpnNameHash = pItem->shName.getHash();
							int wpnNameIdx = pItem->strIdxScreenName;
							int wpnPrice = UTGetShop().GetItemPrice(wpnNameHash);
							if (wpnPrice <= 0)
							{
								//SND_PLAY(SNDIDX_RELOAD_TACTICAL);
								playersel->nCursorMoreReal = -1;

								//when networked play send selection
								if (UTApp().IsGameNetworked())
									SendSelectionByNetwork(nPlayerIdx);
							}
							else
							{
								SND_PLAY(SNDIDX_DENIED);
								
								if (g_userData[K_MEMID_STARS_TOTAL] - g_userData[K_MEMID_STARS_SPENT] < wpnPrice)
								{
									UTGetGUI().MessageBoxOK(STR_UNLOCK_ITEM, STR_NOT_ENOUGH_STARS);
								}
								else
								{
									CCtrlLayer* layer = UTGetGUI().ShowLayerOnce("LAYER_ID_BUY_WEAPON");
									CControl* ctrl = null;
									if (layer != null)
									{
										//set prices strings
										UTLang().SetString(STR_TEMP1, L"%d", g_userData[K_MEMID_STARS_TOTAL] - g_userData[K_MEMID_STARS_SPENT]);
										UTLang().SetString(STR_TEMP2, L"%d", wpnPrice);

										if (ctrl = layer->GetControlByName("LABEL_WPN_NAME"))
										{
											ctrl->paramsDict.SetNamedVarINT32(L"stringID", wpnNameIdx);
										}
										//weapon picture
										if (ctrl = layer->GetControlByName("CP_WEAPON_ICON"))
										{
											int nAnmIdx = ANM_MENUS_SPR_ICONS_WEAPONS;
											if (itemtype == PSS_ITEMCAT_GEAR)
												nAnmIdx = ANM_MENUS_SPR_ICONS_GEAR;
											else if (itemtype == PSS_ITEMCAT_EQUIPMENT)
												nAnmIdx = ANM_MENUS_SPR_ICONS_EQUIPMENT;
											else if (itemtype == PSS_ITEMCAT_ULTIMATE)
												nAnmIdx = ANM_MENUS_SPR_ICONS_ULTIMATE;

											ctrl->paramsDict.SetNamedVarINT32(L"nAnimIdx", nAnmIdx);
											ctrl->paramsDict.SetNamedVarINT32(L"nFrameIdx", pItem->iconIdx);
										}
										//save weapon name hash as button param
										if (ctrl = layer->GetControlByName("BUT_UNLOCK_WEAPON"))
										{
											ctrl->paramsDict.SetNamedVarUINT32(L"nMsgParamUINT32", wpnNameHash);
										}
										//disable UNLOCK button
										if (wpnPrice > g_userData[K_MEMID_STARS_TOTAL] - g_userData[K_MEMID_STARS_SPENT])
										{
											if (ctrl = layer->GetControlByName("BUT_UNLOCK_WEAPON"))
											{
												ctrl->bDisabled = true;
											}
											//red price when no money
											if (ctrl = layer->GetControlByName("CTRL_LABEL_PRICE"))
											{
												ctrl->paramsDict.SetNamedVarString(L"fontColor", L"0xffff0000");
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	//treat exit command if command wasn't erased
	if ((bBackPressed) && (!bBackProcessed))
	{
		SND_PLAY(SNDIDX_DENIED);
		UTGetGUI().ShowLayerOnce("LAYER_ID_QUIT_PLAYERSEL");
	}

	//update cursor things and other time dependent stuff
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		CPlayerCharSelection* playsel = &m_arrPlayers[kk];
		if (playsel->eType == K_PSS_CLASS_NOT_SELECTED)
			continue;

		//update cursor moved
		playsel->fTimeSinceCursorMoved += dTime;

		//animate cursor on selected window
		REACH_VALUE_LINEAR(playsel->fAnimCursor, 0, dTime * 10.0f);

		//update selection cursors
		CAABB aabbCursorTarget;
		aabbCursorTarget.Set(arrCursorRects[playsel->nCursorPosReal][0], arrCursorRects[playsel->nCursorPosReal][1],
			arrCursorRects[playsel->nCursorPosReal][0] + arrCursorRects[playsel->nCursorPosReal][2], 
			arrCursorRects[playsel->nCursorPosReal][1] + arrCursorRects[playsel->nCursorPosReal][3]);
		//set selection cursor if not already set
		if ((playsel->aabbCursor.vSize.x <= 0.0f) || (playsel->aabbCursor.vSize.y <= 0.0f))
			playsel->aabbCursor = aabbCursorTarget;
		//morph to target cursor pos
		AABB::MorphInto_Quadratic(&playsel->aabbCursor, &aabbCursorTarget, 20.0f * dTime, 60.0f * dTime);

		//play verse on changing class
		playsel->fVerseReadyTimer += dTime;
		if ((!playsel->bIsNetworkPlayer) && (playsel->nInstanceID >= 0) && (playsel->fVerseReadyTimer > K_PSS_WAIT_BEFORE_VERSE_SEC) && (playsel->fVerseReadyTimer - dTime <= K_PSS_WAIT_BEFORE_VERSE_SEC))
		{
			/*
			switch (playsel->eType)
			{
				case K_PSS_CLASS_ASSAULTER:
				{
					if (randompercent(50.0f))
						SND_PLAY(SNDIDX_VOICE_ASSAULTER_READY1);
					else
						SND_PLAY(SNDIDX_VOICE_ASSAULTER_READY2);
				}
				break;
				case K_PSS_CLASS_BREACHER:
				{
					if (randompercent(50.0f))
						SND_PLAY(SNDIDX_VOICE_BREACHER_READY);
					else
						SND_PLAY(SNDIDX_VOICE_BREACHER_YOURANG);
				}
				break;
				case K_PSS_CLASS_SHIELD:
				{
					if (randompercent(50.0f))
						SND_PLAY(SNDIDX_VOICE_SHIELD_SHIELDUP);
					else
						SND_PLAY(SNDIDX_VOICE_SHIELD_READY);
				}
				break;
				case K_PSS_CLASS_FBI_AGENT:
				{
					if (randompercent(50.0f))
						SND_PLAY(SNDIDX_VOICE_FBI_IGOTTHIS);
					else
						SND_PLAY(SNDIDX_VOICE_FBI_FBI);
				}
				break;
				case K_PSS_CLASS_RECON:
				{
					if (randompercent(50.0f))
						SND_PLAY(SNDIDX_VOICE_RECON_READYTOROLL);
					else
						SND_PLAY(SNDIDX_VOICE_RECON_RECONREADY);
				}
				break;
				case K_PSS_CLASS_OFFDUTYGUY:
				{
					if (randompercent(50.0f))
						SND_PLAY(SNDIDX_VOICE_OFFDUTY_OFFDUTY);
					else
						SND_PLAY(SNDIDX_VOICE_OFFDUTY_FISHING);
				}
				break;
			}
			*/
		}

	}



	//restul de verificari
	//au apasat toti pe select?
	bool bAllSelected = true;
	//daca nu e nici un player inseamna ca nu e selectie facuta
	if (m_nPlayersCnt <= 0)
		bAllSelected = false;
	//on networked games we need both players to press
	if (UTApp().IsGameNetworked())
	{
		if (m_nPlayersCnt < K_MAX_PLAYERS_CNT)
			bAllSelected = false;
	}

	for (int kk = 0; kk < m_nPlayersCnt; kk++)
	{
		if (!m_arrPlayers[kk].bSelected)
		{
			bAllSelected = false;
			break;
		}
	}

	if (bAllSelected)
	{
		SaveSelection();

		if (UTApp().IsGameNetworked())
		{
			//g_gameMode = (eGameMode)g_netlock.m_ucSelMode;
			g_userData[K_MEMID_SELECTED_CHAPTER] = (int)g_netlock.m_ucSelChapter;
			g_userData[K_MEMID_SELECTED_LEVEL] = (int)g_netlock.m_ucSelLevel;
			//signal exiting player selection
			g_netlock.Net_ExitPlayerSelScreen();
		}

		//--- ANALYTICS EVENT ---
		CHAR ctxt[MAX_PATH];
		WCHAR wtxt[MAX_PATH];
		//save network received level selection
		if (UTApp().IsGameNetworked())
		{
			StringCchPrintf(wtxt, MAX_PATH, L"%s_%s", EPSSPlayerClassNames[(int)m_arrPlayers[0].eType], EPSSPlayerClassNames[(int)m_arrPlayers[1].eType].text);
			wcstombs(ctxt, wtxt, MAX_PATH);
			ANALYTICS_EVENT("player_sel_coop", ctxt, "coop", 1);
		}
		else
		{
			if (m_nPlayersCnt == 1)
			{
				StringCchPrintf(wtxt, MAX_PATH, L"%s", EPSSPlayerClassNames[(int)m_arrPlayers[0].eType].text);
				wcstombs(ctxt, wtxt, MAX_PATH);
				ANALYTICS_EVENT("player_sel_1p", ctxt, "local", 1);
			}
			else
			{
				StringCchPrintf(wtxt, MAX_PATH, L"%s_%s", EPSSPlayerClassNames[(int)m_arrPlayers[0].eType].text, EPSSPlayerClassNames[(int)m_arrPlayers[1].eType].text);
				wcstombs(ctxt, wtxt, MAX_PATH);
				ANALYTICS_EVENT("player_sel_2p", ctxt, "local", 1);
			}
		}

		CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
		nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_GAME);
		nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
		UTGetEventManager().QueueEvent(nevent);
	}
}


void CPlayerSelScr::Paint(ID3DXSprite* pSprite)
{
	//setam ecranul standard de 240h inaltime
	CCameraTransform::SetActiveCamera(m_pDevice, &UTApp().g_cam360hScreen);
	App_SetWorldTransform(m_pDevice, &g_matIdentity);

	RECTXYWH_F scrrect = UTApp().g_cam360hScreen.GetCamWorldAABB();
	///--- paint background (from mainmenu.cpp, easily changed)

	//#MAYBE: poate ar trebui ca desenarea asta sa fie intr-o functie generica (ca sa nu mai fie in 2 locuri)
	DWORD dwColor = 0xff4444dd;
	RECTXYWH_F worldrect = UTApp().g_rect360hWorld;
	//paint back
	float fbackOffX = 10.0f + 8.0f * sin(fLocalTimeline * 0.4f + M_PI);
	CSprite::paintFrame(&m_sprCol, worldrect.CenterX() + fbackOffX, worldrect.CenterY(), ANM_MENUS_SPR_BACK_LAYERS, 0, dwColor);
	//paint chars
	fbackOffX = 4.0f + 4.0f * sin(fLocalTimeline * 0.4f + M_PI);
	CSprite::paintFrame(&m_sprCol, worldrect.CenterX() + fbackOffX, worldrect.CenterY(), ANM_MENUS_SPR_BACK_LAYERS, 1, dwColor);
	//paint character flickering orange light
	AdditiveBlendingON(m_pDevice, pSprite);
	float alpha = 0.4f + UTPerlin::PerlinNoise1D(fLocalTimeline, 5.0f, 2.0f, 0.4f, 0.5f, 2);
	CSprite::paintFrame(&m_sprCol, worldrect.CenterX() + fbackOffX, worldrect.CenterY(), ANM_MENUS_SPR_BACK_LAYERS, 2, DW_COLORALPHA(dwColor, alpha));
	AdditiveBlendingOFF(m_pDevice, pSprite);

	//paint fog layer (w:375)
	float fogoffx = -375.0f * FLOAT_FRAC(fLocalTimeline * 2.0f / 375.0f) + (2.0f * sin(fLocalTimeline * 0.4f + M_PI));
	CSprite::paintFrame(&m_sprCol, worldrect.CenterX() + fogoffx, worldrect.CenterY(), ANM_MENUS_SPR_FOG, 0, dwColor);
	CSprite::paintFrame(&m_sprCol, worldrect.CenterX() + fogoffx + 375.0f, worldrect.CenterY(), ANM_MENUS_SPR_FOG, 0, dwColor);

	fogoffx = -375.0f * FLOAT_FRAC(fLocalTimeline * 5.0f / 375.0f) + (2.0f * sin(fLocalTimeline * 0.4f + M_PI));
	CSprite::paintFrame(&m_sprCol, worldrect.CenterX() + fogoffx, worldrect.CenterY(), ANM_MENUS_SPR_FOG, 1, dwColor);
	CSprite::paintFrame(&m_sprCol, worldrect.CenterX() + fogoffx + 375.0f, worldrect.CenterY(), ANM_MENUS_SPR_FOG, 1, dwColor);
	//paint shotgun
	CSprite::paintFrame(&m_sprCol, worldrect.CenterX() - 12.0f + (6.0f * sin(fLocalTimeline * 0.4f)), worldrect.CenterY(), ANM_MENUS_SPR_BACK_LAYERS, 3, dwColor);

	///--- paint selection windows ---
	if (!UTApp().IsGameNetworked())
	{
		PaintPlayerSelectionWindow(0, D3DXVECTOR2(scrrect.CenterX() - K_PSS_PLAYER_WINDOW_WIDTH - 10.0f, 18.0f));
		PaintPlayerSelectionWindow(1, D3DXVECTOR2(scrrect.CenterX() + 10.0f, 18.0f));
	}
	else
	{
		//host always on left
		PaintPlayerSelectionWindow(0, D3DXVECTOR2(scrrect.CenterX() - K_PSS_PLAYER_WINDOW_WIDTH - 10.0f, 18.0f), 1.0f, &g_netlock.m_sNames[0]);
		//peer always on right
		PaintPlayerSelectionWindow(1, D3DXVECTOR2(scrrect.CenterX() + 10.0f, 18.0f), 1.0f, &g_netlock.m_sNames[1]);
	}

	///--- paint details windows ---
	PaintDetailsWindow(0, D3DXVECTOR2(scrrect.CenterX() - K_PSS_PLAYER_WINDOW_WIDTH - 10.0f, 18.0f));
	PaintDetailsWindow(1, D3DXVECTOR2(scrrect.CenterX() + 10.0f, 18.0f));

	pSprite->Flush();
}

HRESULT CPlayerSelScr::LoadItems()
{
	//reset stuff
	ReleaseItems();
	for (int kk = 0; kk < K_PSS_CLASSES_COUNT; kk++)
	{
		arrItemsByClass[kk].Reset();
	}
	//load document
	WCHAR xmlPath[MAX_PATH];
	FileManager::GetMediaPath(L"media/levels/data/gear_screen.xml", xmlPath);

	pugi::xml_document doc;
	if (!doc.load_file(xmlPath))
	{
		ErrorBox(K_ERR_CRITICAL, L"Unable to load Player Selection Screen items XML:%s\n", xmlPath);
		return E_FAIL;
	}

	///--- Load upgrade bars ---
	pugi::xml_node xbarsnode = doc.root().child(L"GEAR_DATA").child(L"UPGRADE_BARS");
	for (pugi::xml_node xbar = xbarsnode.first_child(); xbar; xbar = xbar.next_sibling())
	{
		CUpgradeBar *nbar = new CUpgradeBar();
		if (!xbar.attribute(L"strUID").empty())
			nbar->shUID.Init(xbar.attribute(L"strUID").value());
		if (!xbar.attribute(L"strID_name").empty())
			nbar->nStrIdx_name = UTLang().GetStrIdx(xbar.attribute(L"strID_name").value());
		if (!xbar.attribute(L"strID_desc").empty())
			nbar->nStrIdx_desc = UTLang().GetStrIdx(xbar.attribute(L"strID_desc").value());

		nbar->nTotalPoints = xbar.attribute(L"nTotalPoints").as_int();
		if ((nbar->nTotalPoints <= 0) || (nbar->nTotalPoints >= K_PSS_UPGRADE_BAR_MAX_POINTS))
		{
			ErrorBox(K_ERR_WARNING, L"[WARNING] PlayerSelScreen::LoadItems: Upgrade bar [%s] - illegal param nTotalPoints!", nbar->shUID.text);
		}
		//memory slot: data gets saved at g_userData[K_MEMID_UPGRADE_BAR_POINTS_START + nMemSlot]
		if (!xbar.attribute(L"nMemorySlot").empty())
		{
			nbar->nMemSlot = xbar.attribute(L"nMemorySlot").as_int();
		}
		else
		{
			ErrorBox(K_ERR_WARNING, L"[WARNING] PlayerSelScreen::LoadItems: Upgrade bar [%s] - nMemSlot not specified! Defaulting to 0!", nbar->shUID.text);
		}
		//fill with empty perks for easy drawing
		for (int ll = 0; ll < nbar->nTotalPoints; ll++)
		{
			CUpgradePerk perk;
			nbar->m_arrPerks.Add(perk);
		}
		//load perks on price points
		for (pugi::xml_node xperk = xbar.first_child(); xperk; xperk = xperk.next_sibling())
		{
			CUpgradePerk perk;
			perk.shUID.Init(xperk.name());
			perk.nIconIdx = xperk.attribute(L"nIconIdx").as_int();
			perk.nPointPrice = xperk.attribute(L"nPointsPrice").as_int();
			if (!xperk.attribute(L"strID_desc").empty())
				perk.nStrIdx_desc = UTLang().GetStrIdx(xperk.attribute(L"strID_desc").value());

			int nIdx = perk.nPointPrice - 1;
			if ((nIdx >= 0) && (nIdx < nbar->nTotalPoints))
			{
				nbar->m_arrPerks.m_pData[nIdx] = perk;
			}
			else
			{
				ErrorBox(K_ERR_WARNING, L"[WARNING] PlayerSelScreen::LoadItems: Upgrade bar [%s] - illegal price point on perk: [%s]", nbar->shUID.text, perk.shUID.text);
			}
		}
		//save bar
		m_arrUpgradeBars.Add(nbar);
	}

	///--- Load player classes and items and gear ---
	pugi::xml_node rootnode = doc.root().child(L"GEAR_DATA").child(L"ITEMS");
	for (pugi::xml_node bclass = rootnode.first_child(); bclass; bclass = bclass.next_sibling())
	{
		//class
		int idx = GetListIndexByName(bclass.attribute(L"type").value(), EPSSPlayerClassNames, EPSSPlayerClass::K_PSS_CLASSES_COUNT);
		if (idx < 0)
		{
			ErrorBox(K_ERR_WARNING, L"[WARNING] CPlayerSelScr::LoadItems: Unknown class type [%s]!", bclass.attribute(L"type").value());
			continue;
		}
		
		ePSSPlayerOptions* itm = &arrItemsByClass[idx];
		itm->ePlayerType = (EPSSPlayerClass)idx;
		//load descriptive texts
		if (!bclass.attribute(L"strID_desc").empty())
			itm->nStrIdx_desc = UTLang().GetStrIdx(bclass.attribute(L"strID_desc").value());
		if (!bclass.attribute(L"strID_difficulty").empty())
			itm->nStrIdx_difficulty= UTLang().GetStrIdx(bclass.attribute(L"strID_difficulty").value());
		//load upgrade bars
		//TEAM BARS
		itm->arrUpgradeBarsIdx[0] = GetUpgradeBarIdx(bclass.attribute(L"strXPBarTeam1").value());
		itm->arrUpgradeBarsIdx[1] = GetUpgradeBarIdx(bclass.attribute(L"strXPBarTeam2").value());
		//CLASS BARS
		itm->arrUpgradeBarsIdx[2] = GetUpgradeBarIdx(bclass.attribute(L"strXPBarClass1").value());
		itm->arrUpgradeBarsIdx[3] = GetUpgradeBarIdx(bclass.attribute(L"strXPBarClass2").value());
		itm->arrUpgradeBarsIdx[4] = GetUpgradeBarIdx(bclass.attribute(L"strXPBarClass3").value());

		//ultimates
		for (pugi::xml_node btype = bclass.first_child(); btype; btype = btype.next_sibling())
		{
			ePSSItemCategory eType = PSS_ITEMCAT_NOT_SET;
			////fid correct category based on node name
			const WCHAR* sType = btype.name();
			if (wcscmp(sType, L"ULTIMATE") == 0)
				eType = PSS_ITEMCAT_ULTIMATE;
			else if (wcscmp(sType, L"WEAPONS") == 0)
				eType = PSS_ITEMCAT_WEAPON;
			else if (wcscmp(sType, L"EQUIPMENT") == 0)
				eType = PSS_ITEMCAT_EQUIPMENT;
			else if (wcscmp(sType, L"GEAR") == 0)
				eType = PSS_ITEMCAT_GEAR;

			if (eType == PSS_ITEMCAT_NOT_SET)
			{
				ErrorBox(K_ERR_WARNING, L"[WARNING] CPlayerSelScr::LoadItems: Skipped unknown group name [%s]!", sType);
				continue;
			}

			for (pugi::xml_node bnode = btype.first_child(); bnode; bnode = bnode.next_sibling())
			{
				const WCHAR* sName = bnode.name();
				sPSSItemData itdata;

				itdata.shName.Init(sName);
				//special value "EMPTY"
				if (itdata.shName.IsEqual(L"EMPTY"))
					itdata.shName.Reset();

				if (!bnode.attribute(L"strID_name").empty())
					itdata.strIdxScreenName = UTLang().GetStrIdx(bnode.attribute(L"strID_name").value());
				if (!bnode.attribute(L"nIconIdx").empty())
					itdata.iconIdx = bnode.attribute(L"nIconIdx").as_int();

				if (!bnode.attribute(L"strALTFireWeaponTemplate").empty())
					itdata.shALTweaponTemplate.Init(bnode.attribute(L"strALTFireWeaponTemplate").value());
				if (!bnode.attribute(L"strID_ALTname").empty())
					itdata.strIdxALTscreenName = UTLang().GetStrIdx(bnode.attribute(L"strID_ALTname").value());
				if (!bnode.attribute(L"nIconIdxALT").empty())
					itdata.iconALTidx = bnode.attribute(L"nIconIdxALT").as_int();

				//read stats (max 4)
				for (int kk = 0; kk < 4; kk++)
				{
					WCHAR strKey[MAX_PATH];
					StringCchPrintf(strKey, MAX_PATH, L"strID_stat%d", kk + 1);
					if (!bnode.attribute(strKey).empty())
						itdata.nStatsData[kk * 2] = UTLang().GetStrIdx(bnode.attribute(strKey).value());
					StringCchPrintf(strKey, MAX_PATH, L"fPercent_stat%d", kk + 1);
					if (!bnode.attribute(strKey).empty())
						itdata.nStatsData[kk * 2 + 1] = (int)ceil(bnode.attribute(strKey).as_float() * 100.0f);
				}

				if (!bnode.attribute(L"strID_longDesc").empty())
					itdata.strIdxLongDescription = UTLang().GetStrIdx(bnode.attribute(L"strID_longDesc").value());
				if (!bnode.attribute(L"strTemplateModifier").empty())
					itdata.shModifierTemplate.Init(bnode.attribute(L"strTemplateModifier").value());

				//and add it to the right category
				itm->matOptionsByItemType[eType].Add(itdata);
			}
		}
	}

	LOG(L"Player Selection Screen: Loaded Items from xml.");

	//check if they transferred points from a class to another (to counteract old issue with transferring points from a class to another from old versions)
	for (int kk = 0; kk < K_PSS_CLASSES_COUNT; kk++)
	{
		int nPlayerClass = kk;
		//compute number of points spent on own bars (first 2 are the team bars, always)
		int nOwnBarsSpent = 0;
		int nXPPointsReal = App_GetAvailableXPPoints((EPSSPlayerClass)nPlayerClass); //real number of XP points to spend
		for (int ll = 2; ll < K_PSS_UPGRADE_BARS_CNT; ll++)
		{
			int nBarIdx = g_playerSelScr.arrItemsByClass[nPlayerClass].arrUpgradeBarsIdx[ll];
			int nFilledReal = g_userData[K_MEMID_UPGRADE_BAR_POINTS_START + m_arrUpgradeBars[nBarIdx]->nMemSlot];
			nOwnBarsSpent += nFilledReal;
		}

		int nCurrentLevel = App_GetXPLevel(g_userData[K_MEMID_TOTALXP_PER_CLASS_START + kk]);
		int nTotalPoints = K_GAME_XPPOINTS_PER_XPLEVEL * nCurrentLevel;
		int nTeamSpentPoints = nTotalPoints - nXPPointsReal - nOwnBarsSpent;

		if (nTeamSpentPoints < 0)
		{
			LOG(L"[WARNING] Class %s with wrong team points! Moving points from another class through the team bars isn't allowed anymore!", EPSSPlayerClassNames[kk].text);
			//reset actual points
			for (int kk = K_MEMID_UPGRADE_BAR_POINTS_START; kk <= K_MEMID_UPGRADE_BAR_POINTS_END; kk++)
				g_userData[kk] = 0;
			for (int kk = K_MEMID_POINTS_SPENT_PER_CLASS_START; kk <= K_MEMID_POINTS_SPENT_PER_CLASS_END; kk++)
				g_userData[kk] = 0;
		}
	}


	return S_OK;
}

void CPlayerSelScr::ReleaseItems()
{
	//release upgrade bars collection 
	SAFE_DELETE_GROWABLE_ARRAY(m_arrUpgradeBars);
	//resetare completa la prima initializare	
	m_nPlayersCnt = 0;
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		m_arrPlayers[kk].Init();
	}
}

void CPlayerSelScr::ReleaseSprites()
{
	m_sprCol.Release();
}


bool CPlayerSelScr::SendSelectionByNetwork(int nPlayerOrdinal)
{
	assert((nPlayerOrdinal >= 0) && (nPlayerOrdinal < K_MAX_PLAYERS_CNT));
	CPlayerCharSelection* playerSel = &m_arrPlayers[nPlayerOrdinal];
	//don't send empty
	if ((playerSel == null) || (playerSel->nInstanceID < 0) || (playerSel->bIsNetworkPlayer))
		return false;

	CNetLock::sPacketPlayerSelection plsel;

	plsel.eType = (BYTE)playerSel->eType;
	plsel.bSelected = (BYTE)playerSel->bSelected;
	plsel.nCursorPos = (BYTE)playerSel->nCursorPosReal;
	plsel.nPlayerStars = g_userData[K_MEMID_STARS_TOTAL] - g_userData[K_MEMID_STARS_SPENT];
	for (int kk = 0; kk < PSS_ITEMCATS_COUNT; kk++)
	{
		plsel.nWpnSelection[kk] = (BYTE)playerSel->nSelection[kk];
		plsel.nWpnSelectionPrice[kk] = (BYTE)playerSel->nPrice[kk];
	}
	//XP
	plsel.nPlayerXPpoints = playerSel->nPlayerXPPts;
	for (int kk = 0; kk < K_PSS_UPGRADE_BARS_CNT; kk++)
	{
		plsel.arrUpgradeBarsPts[kk] = (BYTE)playerSel->arrUpgradeBarsPts[kk];
	}

	return g_netlock.Net_SendPlayerSelection(plsel);
}

void CPlayerSelScr::PaintPlayerSelectionWindow(int nPlayerOrdinal, D3DXVECTOR2 pos, float fAlpha, CStringHash * sPlayerName)
{
	if ((nPlayerOrdinal < 0) || (nPlayerOrdinal >= K_MAX_PLAYERS_CNT))
		return;
	CPlayerCharSelection* playersel = &m_arrPlayers[nPlayerOrdinal];
	RECTXYWH winbox(pos.x, pos.y, K_PSS_PLAYER_WINDOW_WIDTH, K_PSS_PLAYER_WINDOW_HEIGHT);

	DWORD dwWinColor = DW_COLOR_FFFA(fAlpha);
	//if selection was made or we have shown a details window darken main window
	if ((playersel->bSelected) || (playersel->nCursorMoreReal >= 0))
	{
		dwWinColor = DW_COLORALPHA(0xff333333, fAlpha);
		fAlpha *= 0.4f;
	}
	DWORD dwTextColor = DW_COLORALPHA(K_COLOR_DEFAULT_TEXT, fAlpha);
	DWORD dwTitleColor = DW_COLORALPHA(K_COLOR_WINDOW_TITLE, fAlpha);

	if (playersel->nInstanceID < 0) //empty slot
	{
		RECTXYWH tempbox(winbox.x, winbox.CenterY() - 25, winbox.w, 50);
		CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME3, tempbox, dwWinColor);

		CStringDesc strPlayer;
		if (sPlayerName == null)
			UTLang().ReplaceTokenInt(&strPlayer, STR_PLAYER_N, 1, nPlayerOrdinal + 1);
		else
			UTLang().SetStringDesc(&strPlayer, sPlayerName->text);

		g_font10bs1->DrawStringClamped(&strPlayer, winbox.CenterX(), winbox.CenterY() - 4, winbox.w, FONTFLAG_ANCHOR_BOTTOMCENTER, K_COLOR_DEFAULT_TEXT);

		if (g_timers.GetTimerValue(1000) > 0.2f)
		{
			RECTXYWH tempbox2(winbox.x, winbox.CenterY() + 4, winbox.w, 35);
			g_font8bs1->DrawString(STR_PRESS_FIRE_TO_JOIN, tempbox2, FONTFLAG_ANCHOR_TOPCENTER | FONTFLAG_WRAPTEXT, K_COLOR_DEFAULT_TEXT);
		}
	}
	else
	{
		CStringDesc strTemp;
		///--- fereastra principala
		if (sPlayerName == null)
			UTLang().ReplaceTokenInt(&strTemp, STR_PLAYER_N, 1, nPlayerOrdinal + 1);
		else
			UTLang().SetStringDesc(&strTemp, sPlayerName->text);

		CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_WINDOW2, winbox, dwWinColor);
		//player names
		RECTXYWH clipper;
		clipper.Set(winbox.x + 3, winbox.y - 12, winbox.w - 20, 20);
		g_font8b1->DrawStringClamped(&strTemp, clipper.x, clipper.y, clipper.w, FONTFLAG_ANCHOR_TOPLEFT, dwTitleColor);
		//icon controller
		CController* ctrlr = UTGetCtrlrMgr().GetControllerByInstanceID(playersel->nInstanceID);
		if (ctrlr != null)
		{
			if (ctrlr->eType == K_CM_CT_KBM_SDL)
				CSprite::paintFrame(&UTGetGUI().m_sprCol, winbox.Right(), winbox.y, ANM_CONTROLS_SPR_ICONS_CONTROLLER, 0, dwTitleColor);
			else if (ctrlr->eType == K_CM_CT_JOYSTICK_SDL)
				CSprite::paintFrame(&UTGetGUI().m_sprCol, winbox.Right(), winbox.y, ANM_CONTROLS_SPR_ICONS_CONTROLLER, 1, dwTitleColor);
			else //network
				CSprite::paintFrame(&UTGetGUI().m_sprCol, winbox.Right(), winbox.y, ANM_CONTROLS_SPR_ICONS_CONTROLLER, 2, dwTitleColor);
		}
		//placeholder portraits
		CSprite::paintFrame(&m_sprCol, winbox.x, winbox.y, ANM_MENUS_SPR_CHARSEL_WND_DECO, 0, dwWinColor);

		///--- selection cursor 
		RECTXYWH rcCursor = playersel->aabbCursor.to_RECTXYWH();
		rcCursor.Move(winbox.x, winbox.y);
		if ((!playersel->bSelected) && (playersel->nCursorMoreReal < 0))
		{
			//paint actual scaling cursor
			CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME5, rcCursor, 0xffffffff);
		}
		
		///--- poza si frame player
		RECTXYWH recttemp(winbox.x, winbox.y, 42, 52);
		RECTXYWH recttemp2;
		int nPortraitAnm = (nPlayerOrdinal == 0) ? ANM_MENUS_SPR_CHAR_PORTRAITS_LARGE : ANM_MENUS_SPR_CHAR_PORTRAITS_LARGE_R;
		CSprite::paintFrame(&m_sprCol, recttemp.CenterX(), recttemp.y + 3, nPortraitAnm, (int)playersel->eType, dwWinColor);
		//player types page dots under the portrait
		recttemp2 = recttemp; recttemp2.h = 38;
		//CtrlMgrDrawPageSelector(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_PAGE_SELECTOR, recttemp2, K_PSS_CLASSES_COUNT, (int)playersel->eType, dwWinColor);
		//available stars
		CSprite::paintFrame(&m_sprCol, recttemp.CenterX(), recttemp.Bottom() - 2, ANM_MENUS_SPR_CHARSEL_WND_DECO, 1, dwWinColor); //stars placeholder
		//paint network stars or local stars
		if((UTApp().IsGameNetworked()) && (playersel->bIsNetworkPlayer))
			g_font5ns2->DrawString(STR_PEER_STARS_VAL, recttemp.CenterX(), recttemp.Bottom() - 2, FONTFLAG_ANCHOR_BOTTOMLEFT, DW_COLORALPHA(0xfffdb727, fAlpha));
		else
			g_font5ns2->DrawString(STR_TOTAL_STARS_VAL, recttemp.CenterX(), recttemp.Bottom() - 2, FONTFLAG_ANCHOR_BOTTOMLEFT, DW_COLORALPHA(0xfffdb727, fAlpha));

		///--- player class
		recttemp.Set(winbox.x + 52, recttemp.y + 4, winbox.w - 62, 16);
		CtrlMgrDrawHTilingAnim_HeadsOutside(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_INPUT_BOX2, 0, recttemp, dwWinColor);
		recttemp.Set(winbox.x + 44, recttemp.y, winbox.w - 46, 16);
		g_font9b1->DrawString(STR_PLAYER_CLASS_ASSAULTER + (int)playersel->eType, recttemp.CenterX(), recttemp.CenterY(), FONTFLAG_ANCHOR_VCENTERHCENTER, DW_COLORALPHA(K_COLOR_DEFAULT_TEXT_LIGHTER, fAlpha));
		//player types page dots under the class name
		//recttemp2 = recttemp; recttemp2.h -= 2;
		//CtrlMgrDrawPageSelector(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_PAGE_SELECTOR, recttemp2, K_PSS_TYPES_COUNT, (int)playersel->eType, dwWinColor);

		///--- upgrade bar ---
		int nTotalXP = (int)playersel->nPlayerXPPts;
		int nLevel = App_GetXPLevel(nTotalXP);
		int nMaxXP = App_GetMaxXP(nLevel);
		int nMinXP = App_GetMaxXP(nLevel - 1);
		int nUpgradePoints = App_GetAvailableXPPoints(playersel->eType);

		recttemp.y += recttemp.h + 4;
		recttemp.h = 32;
		CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME1, recttemp, dwWinColor, -4);
		//XP progress and numbers
		recttemp2.Set(recttemp.x + 17, recttemp.y + 3, recttemp.w - 22, 12);
		CStringDesc strDesc;
		if (nLevel < K_GAME_MAX_UPGRADE_LEVELS)
		{
			float fPercFill = (float)(nTotalXP - nMinXP) / (float)(nMaxXP - nMinXP);
			CtrlMgrDrawProgress_HeadsOutside(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_PROGRESS_SM_XP, recttemp2, fPercFill, dwWinColor);
			UTLang().SetStringDesc(&strDesc, L"%d/%d", nTotalXP - nMinXP, nMaxXP - nMinXP);
			g_font6nc1->DrawString(&strDesc, recttemp2, FONTFLAG_ANCHOR_VCENTERHCENTER, dwWinColor);
		}
		else
		{
			CtrlMgrDrawProgress_HeadsOutside(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_PROGRESS_SM_XP, recttemp2, 1.0f, dwWinColor);
			g_font6nc1->DrawString(STR_MAX_LEVEL, recttemp2, FONTFLAG_ANCHOR_VCENTERHCENTER, dwWinColor);
		}
		//shieldicon and level
		CSprite::paintFrame(&UTGetGUI().m_sprCol, recttemp2.x, recttemp2.CenterY(), ANM_CONTROLS_SPR_PROGRESS_XP_UPGRADE, 9, dwWinColor);
		UTLang().SetStringDesc(&strDesc, L"%d", nLevel + 1);
		g_font6nc1->DrawString(&strDesc, recttemp2.x - 8, recttemp2.CenterY(), FONTFLAG_ANCHOR_VCENTERHCENTER, DW_COLORALPHA(0xfffdb727, fAlpha));

		//frame nume abilitate
		recttemp2.Set(recttemp.x + 4, recttemp.y + 20, recttemp.w - 8, 8);
		CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME4, recttemp2, dwWinColor);
		g_font8b1->DrawString(STR_SKILLS, recttemp2.CenterX(), recttemp2.y, FONTFLAG_ANCHOR_TOPCENTER, dwTextColor);
		//upgrade chevron
		if ((!playersel->bIsNetworkPlayer) && (nUpgradePoints > 0) && (g_timers.GetTimerValue(600) < 0.4f))
		{
			CSprite::paintFrame(&UTGetGUI().m_sprCol, recttemp2.x + 4, recttemp2.CenterY(), ANM_CONTROLS_SPR_ICONS_MISC, 0, dwWinColor);
			CSprite::paintFrame(&UTGetGUI().m_sprCol, recttemp2.Right() - 5, recttemp2.CenterY(), ANM_CONTROLS_SPR_ICONS_MISC, 0, dwWinColor);
		}

		///--- Primary weapon
		sPSSItemData* pItem = GetItem(playersel->eType, PSS_ITEMCAT_WEAPON, playersel->nSelection[PSS_ITEMCAT_WEAPON]);
		recttemp.Set(winbox.x + 6, recttemp.Bottom() + 17, winbox.w - 12, 29);
		CtrlMgrDrawWindow(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_WINDOW1, recttemp, dwWinColor, FONTIDX_8_B1, STR_PRIMARY_WEAPON, dwTextColor);
		//weapon icon
		recttemp2.Set(recttemp.x, recttemp.y + 2, 44, 14);
		CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME6_DARK, recttemp2, dwWinColor);
		DWORD dwWCol = dwWinColor;
		if (playersel->nPrice[PSS_ITEMCAT_WEAPON] <= 0)
		{
			CSprite::paintFrame(&m_sprCol, recttemp2.CenterX(), recttemp2.CenterY(), ANM_MENUS_SPR_ICONS_WEAPONS, pItem->iconIdx, dwWCol);
		}
		else 
		{
			int nWeaponPrice = playersel->nPrice[PSS_ITEMCAT_WEAPON];
			dwWCol = DW_COLORALPHA(0xff888888, fAlpha);
			//paint lock on locked weapons
			CSprite::paintFrame(&m_sprCol, recttemp2.CenterX(), recttemp2.CenterY(), ANM_MENUS_SPR_ICONS_WEAPONS, pItem->iconIdx, dwWCol);
			CSprite::paintFrame(&UTGetGUI().m_sprCol, recttemp2.x + 4, recttemp2.Bottom() - 3, ANM_CONTROLS_SPR_LOCKS, 0, dwWinColor);
			CSprite::paintFrame(&UTGetGUI().m_sprCol, recttemp2.Right() - 4, recttemp2.Bottom() - 3, ANM_CONTROLS_SPR_STARS_2F_SM, 1, dwWinColor);
			//paint cost
			CStringDesc sdPrice;
			UTLang().SetStringDesc(&sdPrice, L"%d", nWeaponPrice);
			g_font6n1->DrawString(&sdPrice, recttemp2.Right() - 8, recttemp2.Bottom(), FONTFLAG_ANCHOR_BOTTOMRIGHT, dwTextColor);
		}

		//weapon name on the right
		recttemp2.Set(recttemp.Right() - 72, recttemp.y + 1, 74, 18);
		g_font6n1->DrawString(pItem->strIdxScreenName, recttemp2, FONTFLAG_ANCHOR_TOPRIGHT | FONTFLAG_WRAPTEXT, dwTextColor);
		
		//special
		recttemp2.Set(recttemp.x, recttemp.y + 21, recttemp.w, 8);
		CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME4, recttemp2, dwWinColor);
		CSprite::paintFrame(&m_sprCol, recttemp2.x, recttemp2.y, ANM_MENUS_SPR_ICONS_SPECIALS, pItem->iconALTidx, dwWinColor);
		g_font6n1->DrawStringClamped(pItem->strIdxALTscreenName, recttemp2.x + 10, recttemp2.y + 1, recttemp2.w, FONTFLAG_ANCHOR_TOPLEFT, dwTextColor);
		//page dots
		recttemp2 = recttemp; recttemp2.h += 2;
		//CtrlMgrDrawPageSelector(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_PAGE_SELECTOR, recttemp2, GetItemsCount(playersel->eType, PSS_ITEMCAT_WEAPON), playersel->nSelection[PSS_ITEMCAT_WEAPON], dwWinColor);

		///--- Equipment slots
		///slot 1 left
		pItem = GetItem(playersel->eType, PSS_ITEMCAT_EQUIPMENT, playersel->nSelection[PSS_ITEMCAT_EQUIPMENT]);
		recttemp.Set(winbox.x + 6, recttemp.Bottom() + 21, winbox.w / 2 - 12, 15);
		CtrlMgrDrawWindow(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_WINDOW1, recttemp, dwWinColor, FONTIDX_8_B1, STR_GEAR, dwTextColor);
		//icon
		recttemp2.Set(recttemp.x, recttemp.y + 2, recttemp.w, 13);
		CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME6_DARK, recttemp2, dwWinColor);
		dwWCol = dwWinColor;
		CSprite::paintFrame(&m_sprCol, recttemp2.CenterX(), recttemp2.CenterY(), ANM_MENUS_SPR_ICONS_EQUIPMENT, pItem->iconIdx, dwWCol);
		//special ability icon - only paint if we have a valid icon
		if (pItem->iconALTidx > 1)
		{
			CSprite::paintFrame(&m_sprCol, recttemp2.x, recttemp2.Bottom() - 7, ANM_MENUS_SPR_CHARSEL_WND_DECO, 2, dwWinColor); //empty dark frame
			CSprite::paintFrame(&m_sprCol, recttemp2.x, recttemp2.Bottom() - 7, ANM_MENUS_SPR_ICONS_SPECIALS, pItem->iconALTidx, dwWinColor); //actual icon
		}
		//page dots
		recttemp2 = recttemp; recttemp2.h += 2;
		//CtrlMgrDrawPageSelector(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_PAGE_SELECTOR, recttemp2, GetItemsCount(playersel->eType, PSS_ITEMCAT_EQUIPMENT), playersel->nSelection[PSS_ITEMCAT_EQUIPMENT], dwWinColor);
		///slot 2 right (no special ability)
		pItem = GetItem(playersel->eType, PSS_ITEMCAT_GEAR, playersel->nSelection[PSS_ITEMCAT_GEAR]);
		recttemp.x = winbox.x + winbox.w / 2 + 6;
		CtrlMgrDrawWindow(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_WINDOW1, recttemp, dwWinColor, FONTIDX_8_B1, STR_GEAR, dwTextColor);
		//icon
		recttemp2.Set(recttemp.x, recttemp.y + 2, recttemp.w, 13);
		CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME6_DARK, recttemp2, dwWinColor);
		dwWCol = dwWinColor;
		CSprite::paintFrame(&m_sprCol, recttemp2.CenterX(), recttemp2.CenterY(), ANM_MENUS_SPR_ICONS_GEAR, pItem->iconIdx, dwWCol);
		//special ability icon - only paint if we have a valid icon
		if (pItem->iconALTidx > 1)
		{
			CSprite::paintFrame(&m_sprCol, recttemp2.x, recttemp2.Bottom() - 7, ANM_MENUS_SPR_CHARSEL_WND_DECO, 2, dwWinColor); //empty dark frame
			CSprite::paintFrame(&m_sprCol, recttemp2.x, recttemp2.Bottom() - 7, ANM_MENUS_SPR_ICONS_SPECIALS, pItem->iconALTidx, dwWinColor); //actual icon
		}
		//page dots
		recttemp2 = recttemp; recttemp2.h += 2;
		//CtrlMgrDrawPageSelector(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_PAGE_SELECTOR, recttemp2, GetItemsCount(playersel->eType, PSS_ITEMCAT_GEAR), playersel->nSelection[PSS_ITEMCAT_GEAR], dwWinColor);

		///--- ULTIMATE
		pItem = GetItem(playersel->eType, PSS_ITEMCAT_ULTIMATE, playersel->nSelection[PSS_ITEMCAT_ULTIMATE]);
		recttemp.Set(winbox.x + 6, recttemp.Bottom() + 21, winbox.w - 12, 25);
		CtrlMgrDrawWindow(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_WINDOW1, recttemp, dwWinColor, FONTIDX_8_B1, STR_ULTIMATE, dwTextColor);
		//frame si icon abilitate
		recttemp2.Set(recttemp.CenterX() - 22, recttemp.y + 2, 44, 10);
		CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME6_DARK, recttemp2, dwWinColor);
		CSprite::paintFrame(&m_sprCol, recttemp2.CenterX(), recttemp2.CenterY(), ANM_MENUS_SPR_ICONS_ULTIMATE, pItem->iconIdx, dwWinColor);
		//window deco
		CSprite::paintFrame(&m_sprCol, recttemp2.x - 10, recttemp2.CenterY(), ANM_MENUS_SPR_MISC_ICONS, 0, dwWinColor);
		CSprite::paintFrame(&m_sprCol, recttemp2.Right() + 10, recttemp2.CenterY(), ANM_MENUS_SPR_MISC_ICONS, 1, dwWinColor);
		//frame ability name
		recttemp2.Set(recttemp.x, recttemp.y + 17, recttemp.w, 8);
		CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME4, recttemp2, dwWinColor);
		g_font6n1->DrawString(pItem->strIdxScreenName, recttemp2.CenterX(), recttemp2.CenterY(), FONTFLAG_ANCHOR_VCENTERHCENTER, dwTextColor);
		//abilities page dots
		recttemp2 = recttemp; recttemp2.h += 2;
		//CtrlMgrDrawPageSelector(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_PAGE_SELECTOR, recttemp2, GetItemsCount(playersel->eType, PSS_ITEMCAT_ULTIMATE), playersel->nSelection[PSS_ITEMCAT_ULTIMATE], dwWinColor);


		///--- BUTON READY
		recttemp.Set(winbox.x + 29, recttemp.Bottom() + 10, winbox.w - 58, 16);
		int butframe = 0;
		if (playersel->nCursorPosReal == K_PSS_CURPOS_READY) //daca am hover pe buton
			butframe = 3;
		if (playersel->bSelected)
			butframe = 6;

		CtrlMgrDrawHTilingAnim(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_BUTTON1, butframe, recttemp, dwWinColor);
		//textul
		if(!playersel->bSelected)
			recttemp.y -= 1;
		g_font8bs1->DrawString(STR_READY, recttemp, FONTFLAG_ANCHOR_VCENTERHCENTER, dwWinColor);


		///--- paints controller command for selection ---
		if ((!playersel->bSelected) && (!playersel->bIsNetworkPlayer) && (playersel->nCursorMoreReal < 0) && (playersel->nCursorPosReal < K_PSS_CURPOS_READY) && (playersel->fTimeSinceCursorMoved > K_PSS_HINT_WAIT_TIMER))
		{
			CController* ctrlr = UTGetCtrlrMgr().GetControllerByInstanceID(playersel->nInstanceID);
			EControllerCommand eCmd = K_CM_COMMAND_FIRE1;
			if (ctrlr->eType == K_CM_CT_JOYSTICK_SDL)
				eCmd = K_CM_COMMAND_JUMP;
			App_PaintControllerKey(ctrlr, eCmd, D3DXVECTOR2(rcCursor.Right() - 1.0f, rcCursor.Bottom() - 5.0f), ((g_timers.GetTimerValue(600) < 0.3f) ? true : false), -1);
		}
	}

}

void CPlayerSelScr::PaintDetailsWindow(int nPlayerOrdinal, D3DXVECTOR2 pos, float fAlpha /*= 1.0f*/)
{
	if ((nPlayerOrdinal < 0) || (nPlayerOrdinal >= K_MAX_PLAYERS_CNT))
		return;

	CPlayerCharSelection* playersel = &m_arrPlayers[nPlayerOrdinal];
	if ((playersel->nCursorMoreReal < 0) || (playersel->nInstanceID < 0))
		return;

	RECTXYWH winbox(pos.x, pos.y, K_PSS_PLAYER_WINDOW_WIDTH, K_PSS_PLAYER_WINDOW_HEIGHT);

	DWORD dwTextColor = DW_COLORALPHA(K_COLOR_DEFAULT_TEXT, fAlpha);
	DWORD dwTitleColor = DW_COLORALPHA(K_COLOR_WINDOW_TITLE, fAlpha);
	DWORD dwWinColor = DW_COLOR_FFFA(fAlpha);

	//each detail window is treated here:
	switch (playersel->nCursorPosReal)
	{
		case K_PSS_CURPOS_CLASS:
		{
			int nItemsCnt = K_PSS_CLASSES_COUNT;
			//icon and frame
			CStringDesc strDesc;
			RECTXYWH recttemp, recttemp2;
			int nTotalXP = playersel->nPlayerXPPts;
			int nLevel = App_GetXPLevel(nTotalXP);
			int nMaxXP = App_GetMaxXP(nLevel);
			int nMinXP = App_GetMaxXP(nLevel - 1);
			//frame 
			RECTXYWH localbox(winbox.x + 4, winbox.y + 4 + 4, winbox.w - 8, 62);
			RECTXYWH tempbox(winbox.x, winbox.y + 4, winbox.w, 62 + 8);
			
			///--- the other items (small portraits) ---
			for (int kk = 0; kk < nItemsCnt; kk++)
			{
				int nSelectedIdx = (int)playersel->eType;
				float posY = localbox.y - 20.0f * (nSelectedIdx - kk) - 1.0f + 20.0f * playersel->fAnimCursor;
				if (kk == nSelectedIdx)
				{
					if (playersel->fAnimCursor < 0.0f)
						posY = localbox.y - 1.0f + 20.0f * playersel->fAnimCursor;
					else if (playersel->fAnimCursor > 0.0f)
						posY = localbox.Bottom() - 12.0f + 20.0f * playersel->fAnimCursor;
					else
						continue;
				}
				if (kk > nSelectedIdx)
					posY = localbox.Bottom() + (kk - nSelectedIdx) * 20.0f - 9.0f + 20.0f * playersel->fAnimCursor;

				recttemp.Set(localbox.x, posY, localbox.w, 10);
				CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME1, recttemp, dwWinColor);
				//icon
				recttemp2.Set(recttemp.x, recttemp.y, 32, 12);
				int nPortraitAnm = (nPlayerOrdinal == 0) ? ANM_MENUS_SPR_CHAR_PORTRAITS_SM: ANM_MENUS_SPR_CHAR_PORTRAITS_SM_R;
				CSprite::paintFrame(&m_sprCol, recttemp2.CenterX() + 3, recttemp2.y - 1, nPortraitAnm, kk, dwWinColor);
				//name
				recttemp2.Set(recttemp.Right() - 85, recttemp.y, 85, recttemp.h);
				CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME4, recttemp2, dwWinColor);
				g_font9b1->DrawString(STR_PLAYER_CLASS_ASSAULTER + kk, recttemp2.CenterX(), recttemp2.CenterY(), FONTFLAG_ANCHOR_VCENTERHCENTER, dwTextColor);
				//shieldicon and level
				CSprite::paintFrame(&UTGetGUI().m_sprCol, recttemp.x + 12, recttemp.Bottom() - 5, ANM_CONTROLS_SPR_PROGRESS_XP_UPGRADE, 9, dwWinColor);
				int nxppts = g_userData[K_MEMID_TOTALXP_PER_CLASS_START + kk];
				int nlvllocal = App_GetXPLevel(nxppts);
				UTLang().SetStringDesc(&strDesc, L"%d", nlvllocal + 1);
				g_font6nc1->DrawString(&strDesc, recttemp.x + 12 - 8, recttemp.Bottom() - 5, FONTFLAG_ANCHOR_VCENTERHCENTER, DW_COLORALPHA(0xfffdb727, fAlpha));
			}

			///--- selected class ---
			CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME5, tempbox, 0xffffffff);
			CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME1, localbox, dwWinColor);

			//icon
			recttemp.Set(localbox.x - 2, localbox.y - 2, 38, 38);
			CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME_WHITE1, recttemp, DW_COLORALPHA(0xff2A3946, fAlpha));
			int nPortraitAnm = (nPlayerOrdinal == 0) ? ANM_MENUS_SPR_CHAR_PORTRAITS_LARGE : ANM_MENUS_SPR_CHAR_PORTRAITS_LARGE_R;
			CSprite::paintFrame(&m_sprCol, recttemp.CenterX(), recttemp.y, nPortraitAnm, (int)playersel->eType, dwWinColor);
			//shieldicon and level
			CSprite::paintFrame(&UTGetGUI().m_sprCol, recttemp.x + 14, recttemp.Bottom() - 7, ANM_CONTROLS_SPR_PROGRESS_XP_UPGRADE, 9, dwWinColor);
			UTLang().SetStringDesc(&strDesc, L"%d", nLevel + 1);
			g_font6nc1->DrawString(&strDesc, recttemp.x + 14 - 8, recttemp.Bottom() - 7, FONTFLAG_ANCHOR_VCENTERHCENTER, DW_COLORALPHA(0xfffdb727, fAlpha));
			//class name
			recttemp2.Set(localbox.x + 48, localbox.y - 1, winbox.w - 62, 16);
			CtrlMgrDrawHTilingAnim_HeadsOutside(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_INPUT_BOX2, 0, recttemp2, dwWinColor);
			g_font9b1->DrawString(STR_PLAYER_CLASS_ASSAULTER + (int)playersel->eType, recttemp2.CenterX(), recttemp2.CenterY(), FONTFLAG_ANCHOR_VCENTERHCENTER, DW_COLORALPHA(K_COLOR_DEFAULT_TEXT_LIGHTER, fAlpha));
			//difficulty and XP
			recttemp2.Set(recttemp.Right() + 6, recttemp.y + 17, localbox.w - recttemp.w - 4, 8);
			if (nLevel < K_GAME_MAX_UPGRADE_LEVELS)
			{
				float fPercFill = (float)(nTotalXP - nMinXP) / (float)(nMaxXP - nMinXP);
				CtrlMgrDrawProgress_HeadsOutside(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_PROGRESS_SM_XP, recttemp2, fPercFill, dwWinColor);
				UTLang().SetStringDesc(&strDesc, L"%d/%d", nTotalXP - nMinXP, nMaxXP - nMinXP);
				g_font6nc1->DrawString(&strDesc, recttemp2, FONTFLAG_ANCHOR_VCENTERHCENTER, dwWinColor);
			}
			else
			{
				CtrlMgrDrawProgress_HeadsOutside(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_PROGRESS_SM_XP, recttemp2, 1.0f, dwWinColor);
				g_font6nc1->DrawString(STR_MAX_LEVEL, recttemp2, FONTFLAG_ANCHOR_VCENTERHCENTER, dwWinColor);
			}
			//difficulty
			recttemp2.Set(recttemp.Right() + 6, recttemp.y + 30, localbox.w - recttemp.w - 4, recttemp.h - 30);
			//check area://CtrlMgrDrawFrame(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_FRAME_WHITE1, recttemp2, 0xffffffff);
			int nStrDiff = arrItemsByClass[playersel->eType].nStrIdx_difficulty;
			if (nStrDiff >= 0)
			{
				g_font6n1->DrawString(STR_DIFFICULTY_COLON, recttemp2.CenterX() + 6, recttemp2.CenterY(), FONTFLAG_ANCHOR_VCENTERRIGHT, dwTextColor);
				g_font6n1->DrawString(nStrDiff, recttemp2.CenterX() + 8, recttemp2.CenterY(), FONTFLAG_ANCHOR_VCENTERLEFT, dwWinColor);
			}

			//description
			recttemp2.Set(localbox.x, recttemp.Bottom() + 8, localbox.w, 18);
			CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME_WHITE1, recttemp2, DW_COLORALPHA(0xff1C252E, fAlpha));
			if (arrItemsByClass[playersel->eType].nStrIdx_desc >= 0)
			{
				int texth = g_font6n1->MeasureString(arrItemsByClass[playersel->eType].nStrIdx_desc, recttemp2.w).h;
				if (texth <= recttemp2.h)
				{
					g_font6n1->DrawString(arrItemsByClass[playersel->eType].nStrIdx_desc, recttemp2, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, dwTextColor);
				}
				else
				{
					float fTmMul = 8.0f / (float)(texth - recttemp2.h);
					float offy = LIMIT((float)sin((-1.0f + playersel->fTimeSinceCursorMoved) * fTmMul), 0.0f, 1.0f);
					offy *= -(texth - recttemp2.h);
					g_font6n1->DrawStringOffsetY(arrItemsByClass[playersel->eType].nStrIdx_desc, recttemp2, (int)floor(offy), FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT | FONTFLAG_CLIPTEXT, dwTextColor);
				}
			}

			float off = sin(fLocalTimeline * 5.0f);
			//if (playersel->eType > 0)
			CSprite::paintFrame(&UTGetGUI().m_sprCol, tempbox.CenterX(), tempbox.y - off, ANM_CONTROLS_SPR_ARROWS1, K_DIR_UP, dwWinColor);
			//if (playersel->eType < K_PSS_CLASSES_COUNT - 1)
			CSprite::paintFrame(&UTGetGUI().m_sprCol, tempbox.CenterX(), tempbox.Bottom() + off, ANM_CONTROLS_SPR_ARROWS1, K_DIR_DOWN, dwWinColor);

			//page dots
			//recttemp2 = localbox; recttemp2.h += 2;
			//CtrlMgrDrawPageSelector(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_PAGE_SELECTOR, recttemp2, nItemsCnt, (int)playersel->eType, dwWinColor);

			///--- paints controller command for selection ---
			if ((!playersel->bSelected) && (!playersel->bIsNetworkPlayer) && (playersel->fTimeSinceCursorMoved > K_PSS_HINT_WAIT_TIMER))
			{
				CController* ctrlr = UTGetCtrlrMgr().GetControllerByInstanceID(playersel->nInstanceID);
				//back
				EControllerCommand eCmdBak = K_CM_COMMAND_RELOAD;
				App_PaintControllerKey(ctrlr, eCmdBak, D3DXVECTOR2(tempbox.Right() - 1.0f, tempbox.Bottom() - 15.0f), ((g_timers.GetTimerValue(600) >= 0.3f) ? true : false), -1, 0xffff9999);
				//accept
				EControllerCommand eCmd = K_CM_COMMAND_FIRE1;
				if (ctrlr->eType == K_CM_CT_JOYSTICK_SDL)
					eCmd = K_CM_COMMAND_JUMP;
				App_PaintControllerKey(ctrlr, eCmd, D3DXVECTOR2(tempbox.Right() - 1.0f, tempbox.Bottom() - 5.0f), ((g_timers.GetTimerValue(600) < 0.3f) ? true : false), -1);
			}
		}
		break;
		case K_PSS_CURPOS_WEAPON:
		{
			ePSSItemCategory eItemCat = PSS_ITEMCAT_WEAPON;
			sPSSItemData* pItem = GetItem(playersel->eType, eItemCat, playersel->nSelection[(int)eItemCat]);
			int nItemsCnt = GetItemsCount(playersel->eType, eItemCat);
			//weapon icon and frame
			RECTXYWH recttemp, recttemp2;
			//frame 
			RECTXYWH localbox(winbox.x + 6, winbox.y + 73, winbox.w - 12, 48);
			RECTXYWH tempbox(winbox.x + 2, winbox.y + 59, winbox.w - 4, 48 + 18); //selection

			DWORD dwWColDenied = DW_COLORALPHA(0xff887777, fAlpha);

			///--- the other items (small portraits) ---
			for (int kk = 0; kk < nItemsCnt; kk++)
			{
				int nSelectedIdx = playersel->nSelection[(int)eItemCat];
				float posY = localbox.y - 24.0f * (nSelectedIdx - kk) - 12.0f + 24.0f * playersel->fAnimCursor;
				if (kk == nSelectedIdx)
				{
					if (playersel->fAnimCursor < 0.0f)
						posY = localbox.y - 12.0f + 24.0f * playersel->fAnimCursor;
					else if (playersel->fAnimCursor > 0.0f)
						posY = localbox.Bottom() - 12.0f + 24.0f * playersel->fAnimCursor;
					else 
						continue;
				}
				if (kk > nSelectedIdx)
					posY = localbox.Bottom() + (kk - nSelectedIdx) * 24.0f - 12.0f + 24.0f * playersel->fAnimCursor;

				sPSSItemData* pItemSm = GetItem(playersel->eType, eItemCat, kk);
				int nPrice = UTGetShop().GetItemPrice(pItemSm->shName.getHash());
				
				recttemp.Set(localbox.x, posY, localbox.w, 14);
				CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME1, recttemp, dwWinColor);
				if (nPrice <= 0) //owned
				{
					//icon
					recttemp2.Set(recttemp.x, recttemp.y, 44, 14);
					CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME6_DARK, recttemp2, dwWinColor);
					CSprite::paintFrame(&m_sprCol, recttemp2.CenterX(), recttemp2.CenterY(), ANM_MENUS_SPR_ICONS_WEAPONS, pItemSm->iconIdx, dwWinColor);
					//name
					recttemp2.Set(recttemp.Right() - 72, recttemp.y - 1, 74, 18);
					g_font6n1->DrawString(pItemSm->strIdxScreenName, recttemp2, FONTFLAG_ANCHOR_TOPRIGHT | FONTFLAG_WRAPTEXT, dwTextColor);
				}
				else //locked
				{
					//icon
					recttemp2.Set(recttemp.x, recttemp.y, 44, 14);
					CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME6_DARK, recttemp2, dwWinColor);
					CSprite::paintFrame(&m_sprCol, recttemp2.CenterX(), recttemp2.CenterY(), ANM_MENUS_SPR_ICONS_WEAPONS, pItemSm->iconIdx, dwWColDenied);
					//lock
					CSprite::paintFrame(&UTGetGUI().m_sprCol, recttemp2.x + 2, recttemp2.Bottom() - 3, ANM_CONTROLS_SPR_LOCKS, 3, dwWinColor);
					//name
					recttemp2.Set(recttemp.Right() - 72, recttemp.y - 1, 74, 18);
					g_font6n1->DrawString(pItemSm->strIdxScreenName, recttemp2, FONTFLAG_ANCHOR_TOPRIGHT | FONTFLAG_WRAPTEXT, dwTextColor);
				}
			}

			///--- selected item ---
			CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME5, tempbox, 0xffffffff);
			CtrlMgrDrawWindow(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_WINDOW1, localbox, dwWinColor, FONTIDX_8_B1, pItem->strIdxScreenName, dwTextColor);
			//weapon icon
			recttemp2.Set(localbox.x, localbox.y + 2, 44, 31);
			CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME_WHITE2, recttemp2, DW_COLORALPHA(0xff2A3946, fAlpha));
			recttemp.Set(localbox.x, localbox.y + 2, 44, 14);
			CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME6_DARK, recttemp, dwWinColor);
			if (playersel->nPrice[(int)eItemCat] <= 0)
			{
				CSprite::paintFrame(&m_sprCol, recttemp.CenterX(), recttemp.CenterY(), ANM_MENUS_SPR_ICONS_WEAPONS, pItem->iconIdx, dwWinColor);
				g_font6n1->DrawString(STR_AVAILABLE, recttemp2.CenterX(), recttemp2.Bottom() - 4, FONTFLAG_ANCHOR_BOTTOMCENTER, dwTextColor);
			}
			else
			{
				int nWeaponPrice = playersel->nPrice[(int)eItemCat];
				//paint lock on locked weapons
				CSprite::paintFrame(&m_sprCol, recttemp.CenterX(), recttemp.CenterY(), ANM_MENUS_SPR_ICONS_WEAPONS, pItem->iconIdx, dwWColDenied);
				CSprite::paintFrame(&UTGetGUI().m_sprCol, recttemp2.x + 8, recttemp2.Bottom() - 7, ANM_CONTROLS_SPR_LOCKS, 2, dwWinColor);
				CSprite::paintFrame(&UTGetGUI().m_sprCol, recttemp2.Right() - 12, recttemp2.Bottom() - 4, ANM_CONTROLS_SPR_ICONS_MISC, 1, dwWinColor);
				//paint cost
				CStringDesc sdPrice;
				UTLang().SetStringDesc(&sdPrice, L"%d", nWeaponPrice);
				g_font6n1->DrawString(&sdPrice, recttemp2.Right() - 7, recttemp2.Bottom() - 3, FONTFLAG_ANCHOR_BOTTOMCENTER, dwTextColor);
			}
			//stats (max 4)
			recttemp2.Set(localbox.Right() - 25, localbox.y, 24, 8);
			for (int oo = 0; oo < 4; oo++)
			{
				int nStrIdx = pItem->nStatsData[oo * 2];
				if (nStrIdx >= 0)
				{
					float fPerc = (float)pItem->nStatsData[oo * 2 + 1] / 100.0f;
					CtrlMgrDrawProgress_HeadsOutside(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_PROGRESS, recttemp2, fPerc, dwWinColor);
					g_font6n1->DrawString(nStrIdx, recttemp2.x - 3, recttemp2.y + 8, FONTFLAG_ANCHOR_BOTTOMRIGHT, dwTextColor);
					recttemp2.y += 9;
				}
			}
			//special ability
			recttemp2.Set(localbox.x, localbox.Bottom() - 8, localbox.w, 8);
			CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME4, recttemp2, dwWinColor);
			CSprite::paintFrame(&m_sprCol, recttemp2.x, recttemp2.y, ANM_MENUS_SPR_ICONS_SPECIALS, pItem->iconALTidx, dwWinColor);
			if (pItem->strIdxALTscreenName >= 0)
				g_font6n1->DrawStringClamped(pItem->strIdxALTscreenName, recttemp2.x + 10, recttemp2.y + 1, recttemp2.w, FONTFLAG_ANCHOR_TOPLEFT, dwTextColor);
			
			//page dots
			//recttemp2 = localbox; recttemp2.h += 2;
			//CtrlMgrDrawPageSelector(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_PAGE_SELECTOR, recttemp2, GetItemsCount(playersel->eType, eItemCat), playersel->nSelection[(int)eItemCat], dwWinColor);

			//arrows
			float off = sin(fLocalTimeline * 5.0f);
			//if (playersel->nSelection[(int)eItemCat] > 0)
			CSprite::paintFrame(&UTGetGUI().m_sprCol, tempbox.CenterX(), tempbox.y - off, ANM_CONTROLS_SPR_ARROWS1, K_DIR_UP, dwWinColor);
			//if (playersel->nSelection[(int)eItemCat] < nItemsCnt - 1)
			CSprite::paintFrame(&UTGetGUI().m_sprCol, tempbox.CenterX(), tempbox.Bottom() + off, ANM_CONTROLS_SPR_ARROWS1, K_DIR_DOWN, dwWinColor);

			///--- paints controller command for selection ---
			if ((!playersel->bSelected) && (!playersel->bIsNetworkPlayer) && (playersel->fTimeSinceCursorMoved > K_PSS_HINT_WAIT_TIMER))
			{
				CController* ctrlr = UTGetCtrlrMgr().GetControllerByInstanceID(playersel->nInstanceID);
				//back
				EControllerCommand eCmdBak = K_CM_COMMAND_RELOAD;
				App_PaintControllerKey(ctrlr, eCmdBak, D3DXVECTOR2(tempbox.Right() - 1.0f, tempbox.Bottom() - 15.0f), ((g_timers.GetTimerValue(600) >= 0.3f) ? true : false), -1, 0xffff9999);
				//accept
				EControllerCommand eCmd = K_CM_COMMAND_FIRE1;
				if (ctrlr->eType == K_CM_CT_JOYSTICK_SDL)
					eCmd = K_CM_COMMAND_JUMP;
				App_PaintControllerKey(ctrlr, eCmd, D3DXVECTOR2(tempbox.Right() - 1.0f, tempbox.Bottom() - 5.0f), ((g_timers.GetTimerValue(600) < 0.3f) ? true : false), -1);
			}
		}
		break;
		case K_PSS_CURPOS_EQUIPMENT:
		case K_PSS_CURPOS_GEAR:
		{
			ePSSItemCategory eItemCat = PSS_ITEMCAT_EQUIPMENT;
			int nIconsAnim = ANM_MENUS_SPR_ICONS_EQUIPMENT;
			if (playersel->nCursorPosReal == K_PSS_CURPOS_GEAR)
			{
				eItemCat = PSS_ITEMCAT_GEAR;
				nIconsAnim = ANM_MENUS_SPR_ICONS_GEAR;
			}

			sPSSItemData* pItem = GetItem(playersel->eType, eItemCat, playersel->nSelection[(int)eItemCat]);
			int nItemsCnt = GetItemsCount(playersel->eType, eItemCat);
			//frame 
			RECTXYWH localbox(winbox.x + 6, winbox.y + 109 + 14, winbox.w - 12, 53);
			RECTXYWH tempbox(winbox.x + 2, winbox.y + 109, winbox.w - 4, 53 + 18);
			
			RECTXYWH recttemp, recttemp2, recttemp3;
			DWORD dwWColDenied = DW_COLORALPHA(0xff887777, fAlpha);

			///--- the other items (small portraits) ---
			for (int kk = 0; kk < nItemsCnt; kk++)
			{
				int nSelectedIdx = playersel->nSelection[(int)eItemCat];
				float posY = localbox.y - 24.0f * (nSelectedIdx - kk) - 12.0f + 24.0f * playersel->fAnimCursor;
				if (kk == nSelectedIdx)
				{
					if (playersel->fAnimCursor < 0.0f)
						posY = localbox.y - 12.0f + 24.0f * playersel->fAnimCursor;
					else if (playersel->fAnimCursor > 0.0f)
						posY = localbox.Bottom() - 12.0f + 24.0f * playersel->fAnimCursor;
					else
						continue;
				}
				if (kk > nSelectedIdx)
					posY = localbox.Bottom() + (kk - nSelectedIdx) * 24.0f - 12.0f + 24.0f * playersel->fAnimCursor;

				sPSSItemData* pItemSm = GetItem(playersel->eType, eItemCat, kk);
				int nPrice = UTGetShop().GetItemPrice(pItemSm->shName.getHash());

				recttemp.Set(localbox.x, posY, localbox.w, 14);
				CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME1, recttemp, dwWinColor);

				UINT16 ualign = 0;
				if (playersel->nCursorPosReal == K_PSS_CURPOS_EQUIPMENT)
				{
					recttemp2.Set(recttemp.x, recttemp.y, 44, 14);
					recttemp3.Set(recttemp.Right() - 72, recttemp.y - 1, 74, 18);
					ualign = FONTFLAG_ANCHOR_TOPRIGHT | FONTFLAG_WRAPTEXT;
				}
				else //icon on right
				{
					recttemp2.Set(recttemp.Right() - 44, recttemp.y, 44, 14);
					recttemp3.Set(recttemp.x, recttemp.y - 1, 74, 18);
					ualign = FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT;
				}

				if (nPrice <= 0) //owned
				{
					//icon
					CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME6_DARK, recttemp2, dwWinColor);
					CSprite::paintFrame(&m_sprCol, recttemp2.CenterX(), recttemp2.CenterY(), nIconsAnim, pItemSm->iconIdx, dwWinColor);
					//name
					g_font6n1->DrawString(pItemSm->strIdxScreenName, recttemp3, ualign, dwTextColor);
				}
				else //locked
				{
					//icon
					CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME6_DARK, recttemp2, dwWinColor);
					CSprite::paintFrame(&m_sprCol, recttemp2.CenterX(), recttemp2.CenterY(), nIconsAnim, pItemSm->iconIdx, dwWColDenied);
					//lock
					CSprite::paintFrame(&UTGetGUI().m_sprCol, recttemp2.x + 2, recttemp2.Bottom() - 3, ANM_CONTROLS_SPR_LOCKS, 3, dwWinColor);
					//name
					g_font6n1->DrawString(pItemSm->strIdxScreenName, recttemp3, ualign, dwTextColor);
				}
			}

			///--- selected item ---
			CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME5, tempbox, 0xffffffff);
			CtrlMgrDrawWindow(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_WINDOW1, localbox, dwWinColor, FONTIDX_8_B1, pItem->strIdxScreenName, dwTextColor);

			//icon
			if (playersel->nCursorPosReal == K_PSS_CURPOS_EQUIPMENT)
			{
				recttemp2.Set(localbox.x + 54, localbox.y + 2, localbox.w - 54, 13);
				recttemp.Set(localbox.x, localbox.y + 2, 54, 13);
			}
			else
			{
				recttemp2.Set(localbox.x, localbox.y + 2, localbox.w - 54, 13);
				recttemp.Set(localbox.Right() - 54, localbox.y + 2, 54, 13);
			}

			CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME_WHITE2, recttemp2, DW_COLORALPHA(0xff2A3946, fAlpha));
			CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME6_DARK, recttemp, dwWinColor);
			DWORD dwWCol = dwWinColor;
			if (playersel->nPrice[(int)eItemCat] <= 0)
			{
				CSprite::paintFrame(&m_sprCol, recttemp.CenterX(), recttemp.CenterY(), nIconsAnim, pItem->iconIdx, dwWCol);
				g_font6n1->DrawString(STR_AVAILABLE, recttemp2.CenterX(), recttemp2.Bottom() - 4, FONTFLAG_ANCHOR_BOTTOMCENTER, dwTextColor);
			}
			else
			{
				int nWeaponPrice = playersel->nPrice[(int)eItemCat];
				//paint lock on locked weapons
				CSprite::paintFrame(&m_sprCol, recttemp.CenterX(), recttemp.CenterY(), nIconsAnim, pItem->iconIdx, dwWColDenied);
				CSprite::paintFrame(&UTGetGUI().m_sprCol, recttemp2.CenterX() - 14, recttemp2.Bottom() - 7, ANM_CONTROLS_SPR_LOCKS, 2, dwWinColor);
				CSprite::paintFrame(&UTGetGUI().m_sprCol, recttemp2.CenterX() + 10, recttemp2.Bottom() - 4, ANM_CONTROLS_SPR_ICONS_MISC, 1, dwWinColor);
				//paint cost
				CStringDesc sdPrice;
				UTLang().SetStringDesc(&sdPrice, L"%d", nWeaponPrice);
				g_font6n1->DrawString(&sdPrice, recttemp2.CenterX() + 14, recttemp2.Bottom() - 3, FONTFLAG_ANCHOR_BOTTOMCENTER, dwTextColor);
			}

			//description frame and text
			recttemp.Set(localbox.x, recttemp2.Bottom() + 6, localbox.w, 18);
			CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME_WHITE1, recttemp, DW_COLORALPHA(0xff1C252E, fAlpha));
			if (pItem->strIdxLongDescription >= 0)
			{
				int texth = g_font6n1->MeasureString(pItem->strIdxLongDescription, recttemp.w).h;
				if (texth <= recttemp.h)
				{
					g_font6n1->DrawString(pItem->strIdxLongDescription, recttemp, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, dwTextColor);
				}
				else
				{
					float fTmMul = 8.0f / (float)(texth - recttemp.h);
					float offy = LIMIT((float)sin((-1.0f + playersel->fTimeSinceCursorMoved) * fTmMul), 0.0f, 1.0f);
					offy *= -(texth - recttemp.h);
					g_font6n1->DrawStringOffsetY(pItem->strIdxLongDescription, recttemp, (int)floor(offy), FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT | FONTFLAG_CLIPTEXT, dwTextColor);
				}
			}
			//stats (max 4)
			recttemp2.Set(recttemp.x + 3, recttemp.y, 24, 8);
			for (int oo = 0; oo < 4; oo++)
			{
				int nStrIdx = pItem->nStatsData[oo * 2];
				if (nStrIdx >= 0)
				{
					float fPerc = (float)pItem->nStatsData[oo * 2 + 1] / 100.0f;
					CtrlMgrDrawProgress_HeadsOutside(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_PROGRESS, recttemp2, fPerc, dwWinColor);
					g_font6n1->DrawString(nStrIdx, recttemp2.Right() + 5, recttemp2.y + 2, FONTFLAG_ANCHOR_TOPLEFT, dwTextColor);
					recttemp2.y += 9;
				}
			}
			//special ability
			recttemp2.Set(localbox.x, localbox.Bottom() - 8, localbox.w, 8);
			CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME4, recttemp2, dwWinColor);
			CSprite::paintFrame(&m_sprCol, recttemp2.x, recttemp2.y, ANM_MENUS_SPR_ICONS_SPECIALS, pItem->iconALTidx, dwWinColor);
			if (pItem->strIdxALTscreenName >= 0)
				g_font6n1->DrawStringClamped(pItem->strIdxALTscreenName, recttemp2.x + 10, recttemp2.y + 1, recttemp2.w, FONTFLAG_ANCHOR_TOPLEFT, dwTextColor);
			//page dots
			//recttemp2 = localbox; recttemp2.h += 2;
			//CtrlMgrDrawPageSelector(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_PAGE_SELECTOR, recttemp2, GetItemsCount(playersel->eType, eItemCat), playersel->nSelection[(int)eItemCat], dwWinColor);
			float off = sin(fLocalTimeline * 5.0f);
			//if (playersel->nSelection[(int)eItemCat] > 0)
			CSprite::paintFrame(&UTGetGUI().m_sprCol, tempbox.CenterX(), tempbox.y - off, ANM_CONTROLS_SPR_ARROWS1, K_DIR_UP, dwWinColor);
			//if (playersel->nSelection[(int)eItemCat] < nItemsCnt - 1)
			CSprite::paintFrame(&UTGetGUI().m_sprCol, tempbox.CenterX(), tempbox.Bottom() + off, ANM_CONTROLS_SPR_ARROWS1, K_DIR_DOWN, dwWinColor);
			///--- paints controller command for selection ---
			if ((!playersel->bSelected) && (!playersel->bIsNetworkPlayer) && (playersel->fTimeSinceCursorMoved > K_PSS_HINT_WAIT_TIMER))
			{
				CController* ctrlr = UTGetCtrlrMgr().GetControllerByInstanceID(playersel->nInstanceID);
				//back
				EControllerCommand eCmdBak = K_CM_COMMAND_RELOAD;
				App_PaintControllerKey(ctrlr, eCmdBak, D3DXVECTOR2(tempbox.Right() - 1.0f, tempbox.Bottom() - 15.0f), ((g_timers.GetTimerValue(600) >= 0.3f) ? true : false), -1, 0xffff9999);
				//accept
				EControllerCommand eCmd = K_CM_COMMAND_FIRE1;
				if (ctrlr->eType == K_CM_CT_JOYSTICK_SDL)
					eCmd = K_CM_COMMAND_JUMP;
				App_PaintControllerKey(ctrlr, eCmd, D3DXVECTOR2(tempbox.Right() - 1.0f, tempbox.Bottom() - 5.0f), ((g_timers.GetTimerValue(600) < 0.3f) ? true : false), -1);
			}
		}
		break;
		case K_PSS_CURPOS_ULTIMATE:
		{
			ePSSItemCategory eItemCat = PSS_ITEMCAT_ULTIMATE;

			sPSSItemData* pItem = GetItem(playersel->eType, eItemCat, playersel->nSelection[(int)eItemCat]);
			int nItemsCnt = GetItemsCount(playersel->eType, eItemCat);
			//frame 
			RECTXYWH localbox(winbox.x + 6, winbox.y + 145 + 14, winbox.w - 12, 35);
			RECTXYWH tempbox(winbox.x + 2, winbox.y + 145, winbox.w - 4, 35 + 18);
			//weapon icon and frame
			RECTXYWH recttemp, recttemp2;

			DWORD dwWColDenied = DW_COLORALPHA(0xff887777, fAlpha);

			///--- the other items (small portraits) ---
			for (int kk = 0; kk < nItemsCnt; kk++)
			{
				int nSelectedIdx = playersel->nSelection[(int)eItemCat];
				float posY = localbox.y - 20.0f * (nSelectedIdx - kk) - 12.0f + 24.0f * playersel->fAnimCursor;
				if (kk == nSelectedIdx)
				{
					if (playersel->fAnimCursor < 0.0f)
						posY = localbox.y - 12.0f + 20.0f * playersel->fAnimCursor;
					else if (playersel->fAnimCursor > 0.0f)
						posY = localbox.Bottom() - 12.0f + 20.0f * playersel->fAnimCursor;
					else
						continue;
				}
				if (kk > nSelectedIdx)
					posY = localbox.Bottom() + (kk - nSelectedIdx) * 24.0f - 12.0f + 24.0f * playersel->fAnimCursor;

				sPSSItemData* pItemSm = GetItem(playersel->eType, eItemCat, kk);
				int nPrice = UTGetShop().GetItemPrice(pItemSm->shName.getHash());

				recttemp.Set(localbox.x, posY, localbox.w, 10);
				CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME1, recttemp, dwWinColor);
				if (nPrice <= 0) //owned
				{
					//icon
					recttemp2.Set(recttemp.x, recttemp.y, 44, 10);
					CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME6_DARK, recttemp2, dwWinColor);
					CSprite::paintFrame(&m_sprCol, recttemp2.CenterX(), recttemp2.CenterY(), ANM_MENUS_SPR_ICONS_ULTIMATE, pItemSm->iconIdx, dwWinColor);
					//name
					recttemp2.Set(recttemp.Right() - 72, recttemp.y - 1, 74, 18);
					g_font6n1->DrawString(pItemSm->strIdxScreenName, recttemp2, FONTFLAG_ANCHOR_TOPRIGHT | FONTFLAG_WRAPTEXT, dwTextColor);
				}
				else //locked
				{
					//icon
					recttemp2.Set(recttemp.x, recttemp.y, 44, 10);
					CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME6_DARK, recttemp2, dwWinColor);
					CSprite::paintFrame(&m_sprCol, recttemp2.CenterX(), recttemp2.CenterY(), ANM_MENUS_SPR_ICONS_ULTIMATE, pItemSm->iconIdx, dwWColDenied);
					//lock
					CSprite::paintFrame(&UTGetGUI().m_sprCol, recttemp2.x + 2, recttemp2.Bottom() - 3, ANM_CONTROLS_SPR_LOCKS, 3, dwWinColor);
					//name
					recttemp2.Set(recttemp.Right() - 72, recttemp.y - 1, 74, 18);
					g_font6n1->DrawString(pItemSm->strIdxScreenName, recttemp2, FONTFLAG_ANCHOR_TOPRIGHT | FONTFLAG_WRAPTEXT, DW_COLORALPHA(K_COLOR_DEFAULT_TEXT_HALFALPHA, 0.4f));
				}
			}

			///--- selected item ---
			CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME5, tempbox, 0xffffffff);
			CtrlMgrDrawWindow(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_WINDOW1, localbox, dwWinColor, FONTIDX_8_B1, pItem->strIdxScreenName, dwTextColor);

			//icon
			recttemp2.Set(localbox.x, localbox.y + 2, localbox.w - 4, 10);
			recttemp.Set(localbox.CenterX() - 22, localbox.y + 2, 44, 10);

			//CtrlMgrDrawFrame(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_FRAME_WHITE2, recttemp2, D3DCOLOR_COLORALPHA(0xff2A3946, fAlpha));
			CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME6_DARK, recttemp, dwWinColor);

			if (playersel->nPrice[(int)eItemCat] <= 0)
			{
				CSprite::paintFrame(&m_sprCol, recttemp.CenterX(), recttemp.CenterY(), ANM_MENUS_SPR_ICONS_ULTIMATE, pItem->iconIdx, dwWinColor);
				//g_font6n1->DrawString(STR_AVAILABLE, recttemp2.CenterX(), recttemp2.Bottom() - 4, FONTFLAG_ANCHOR_BOTTOMCENTER, dwTextColor);
			}
			else
			{
				int nWeaponPrice = playersel->nPrice[(int)eItemCat];
				//paint lock on locked weapons
				CSprite::paintFrame(&m_sprCol, recttemp.CenterX(), recttemp.CenterY(), ANM_MENUS_SPR_ICONS_ULTIMATE, pItem->iconIdx, dwWColDenied);
				CSprite::paintFrame(&UTGetGUI().m_sprCol, recttemp2.CenterX() - 14, recttemp2.Bottom() - 7, ANM_CONTROLS_SPR_LOCKS, 1, dwWinColor);
				CSprite::paintFrame(&UTGetGUI().m_sprCol, recttemp2.CenterX() + 10, recttemp2.Bottom() - 4, ANM_CONTROLS_SPR_ICONS_MISC, 1, dwWinColor);
				//paint cost
				CStringDesc sdPrice;
				UTLang().SetStringDesc(&sdPrice, L"%d", nWeaponPrice);
				g_font6n1->DrawString(&sdPrice, recttemp2.CenterX() + 14, recttemp2.Bottom() - 3, FONTFLAG_ANCHOR_BOTTOMCENTER, dwTextColor);
			}
			//window deco
			CSprite::paintFrame(&m_sprCol, recttemp.x - 10, recttemp.CenterY(), ANM_MENUS_SPR_MISC_ICONS, 0, dwWinColor);
			CSprite::paintFrame(&m_sprCol, recttemp.Right() + 10, recttemp.CenterY(), ANM_MENUS_SPR_MISC_ICONS, 1, dwWinColor);

			//description frame and text
			recttemp.Set(localbox.x, recttemp2.Bottom() + 5, localbox.w, 18);
			CtrlMgrDrawFrame(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_FRAME_WHITE1, recttemp, DW_COLORALPHA(0xff1C252E, fAlpha));
			if (pItem->strIdxLongDescription >= 0)
			{
				//g_font6n1->DrawString(pItem->strIdxLongDescription, recttemp, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, dwTextColor);
				int texth = g_font6n1->MeasureString(pItem->strIdxLongDescription, recttemp.w).h;
				if (texth <= recttemp.h)
				{
					g_font6n1->DrawString(pItem->strIdxLongDescription, recttemp, FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, dwTextColor);
				}
				else
				{
					float fTmMul = 8.0f / (float)(texth - recttemp.h);
					float offy = LIMIT((float)sin((-1.0f + playersel->fTimeSinceCursorMoved) * fTmMul), 0.0f, 1.0f);
					offy *= -(texth - recttemp.h);
					g_font6n1->DrawStringOffsetY(pItem->strIdxLongDescription, recttemp, (int)floor(offy), FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT | FONTFLAG_CLIPTEXT, dwTextColor);
				}
			}
			//stats (max 4)
			recttemp2.Set(recttemp.x + 3, recttemp.y, 24, 8);
			for (int oo = 0; oo < 4; oo++)
			{
				int nStrIdx = pItem->nStatsData[oo * 2];
				if (nStrIdx >= 0)
				{
					float fPerc = (float)pItem->nStatsData[oo * 2 + 1] / 100.0f;
					CtrlMgrDrawProgress_HeadsOutside(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_PROGRESS, recttemp2, fPerc, dwWinColor);
					g_font6n1->DrawString(nStrIdx, recttemp2.Right() + 5, recttemp2.y + 2, FONTFLAG_ANCHOR_TOPLEFT, dwTextColor);
					recttemp2.y += 9;
				}
			}
			//page dots
			//recttemp2 = localbox; recttemp2.h += 2;
			//CtrlMgrDrawPageSelector(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_PAGE_SELECTOR, recttemp2, GetItemsCount(playersel->eType, eItemCat), playersel->nSelection[(int)eItemCat], dwWinColor);
			float off = sin(fLocalTimeline * 5.0f);
			//if (playersel->nSelection[(int)eItemCat] > 0)
			CSprite::paintFrame(&UTGetGUI().m_sprCol, tempbox.CenterX(), tempbox.y - off, ANM_CONTROLS_SPR_ARROWS1, K_DIR_UP, dwWinColor);
			//if (playersel->nSelection[(int)eItemCat] < nItemsCnt - 1)
			CSprite::paintFrame(&UTGetGUI().m_sprCol, tempbox.CenterX(), tempbox.Bottom() + off, ANM_CONTROLS_SPR_ARROWS1, K_DIR_DOWN, dwWinColor);
			///--- paints controller command for selection ---
			if ((!playersel->bSelected) && (!playersel->bIsNetworkPlayer) && (playersel->fTimeSinceCursorMoved > K_PSS_HINT_WAIT_TIMER))
			{
				CController* ctrlr = UTGetCtrlrMgr().GetControllerByInstanceID(playersel->nInstanceID);
				//back
				EControllerCommand eCmdBak = K_CM_COMMAND_RELOAD;
				App_PaintControllerKey(ctrlr, eCmdBak, D3DXVECTOR2(tempbox.Right() - 1.0f, tempbox.Bottom() - 15.0f), ((g_timers.GetTimerValue(600) >= 0.3f) ? true : false), -1, 0xffff9999);
				//accept
				EControllerCommand eCmd = K_CM_COMMAND_FIRE1;
				if (ctrlr->eType == K_CM_CT_JOYSTICK_SDL)
					eCmd = K_CM_COMMAND_JUMP;
				App_PaintControllerKey(ctrlr, eCmd, D3DXVECTOR2(tempbox.Right() - 1.0f, tempbox.Bottom() - 5.0f), ((g_timers.GetTimerValue(600) < 0.3f) ? true : false), -1);
			}
		}
		break;
	}
}

float CPlayerSelScr::GetUpgradeBarPercent(CPlayerCharSelection* pSel, WCHAR* strBarName)
{
	UINT32 dwBarUID = FastHash(strBarName);
	CActorTemplate templ;
	if (pSel == NULL)
	{
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
		ErrorBox(K_ERR_WARNING, L"[Warning] GetUpgradeBarPercent: Player Selection is empty!");
#endif
		return 0.0f;
	}

	for (int kk = 0; kk < K_PSS_UPGRADE_BARS_CNT; kk++)
	{
		int nBarIdx = arrItemsByClass[(int)pSel->eType].arrUpgradeBarsIdx[kk];
		UINT32 shBarUID = m_arrUpgradeBars[nBarIdx]->shUID.getHash();
		if (shBarUID != dwBarUID)
			continue;

		int nTotalDots = m_arrUpgradeBars[nBarIdx]->nTotalPoints;
		int nFilledDots = pSel->arrUpgradeBarsPts[kk];
		float fFilledPercent = (float)nFilledDots / (float)nTotalDots;

		return fFilledPercent;
	}

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	ErrorBox(K_ERR_WARNING, L"[Warning] GetUpgradeBarPercent: Bar not found! %s", strBarName);
#endif

	return 0.0f;
}

int CPlayerSelScr::GetAllActivePerks(CPlayerCharSelection* pSel, UINT32 arrRetValues[], int nRetValuesArrSize)
{
	int nCount = 0;
	for (int kk = 0; kk < K_PSS_UPGRADE_BARS_CNT; kk++)
	{
		int nBarIdx = arrItemsByClass[(int)pSel->eType].arrUpgradeBarsIdx[kk];
		int nTotalDots = m_arrUpgradeBars[nBarIdx]->nTotalPoints;
		int nFilledDots = pSel->arrUpgradeBarsPts[kk];
		for (int ll = 0; ll < nTotalDots; ll++)
		{
			if ((!m_arrUpgradeBars[nBarIdx]->m_arrPerks[ll].shUID.IsEmpty()) && (ll < nFilledDots))
			{
				arrRetValues[nCount++] = m_arrUpgradeBars[nBarIdx]->m_arrPerks[ll].shUID.textHash;
				if (nCount >= nRetValuesArrSize)
				{
					ErrorBox(K_ERR_WARNING, L"CPlayerSelScr::GetAllActivePerks - array too small!");
					return nCount;
				}
			}
		}
	}
	return nCount;
}

bool CPlayerSelScr::IsPerkEnabled(int nPlayerOrdinal, const CStringHash * strPerkName)
{
	if ((nPlayerOrdinal < 0) || (nPlayerOrdinal >= K_MAX_PLAYERS_CNT)) 
	{
		ErrorBox(K_ERR_WARNING, L"IsPerkEnabled: Illegal player ordinal!");
		return false;
	}
	if (strPerkName == null)
		return false;

	CPlayerCharSelection* pSel = &m_arrPlayers[nPlayerOrdinal];
	UINT32 dwPerkUID = strPerkName->textHash;
	for (int kk = 0; kk < K_PSS_UPGRADE_BARS_CNT; kk++)
	{
		int nBarIdx = arrItemsByClass[(int)pSel->eType].arrUpgradeBarsIdx[kk];
		int nTotalDots = m_arrUpgradeBars[nBarIdx]->nTotalPoints;
		int nFilledDots = pSel->arrUpgradeBarsPts[kk];
		for (int ll = 0; ll < nTotalDots; ll++)
		{
			if (m_arrUpgradeBars[nBarIdx]->m_arrPerks[ll].shUID.textHash == dwPerkUID)
			{
				return (ll < nFilledDots); 
			}
		}
	}

	return false;
}

/*----------------------------------*\
*  System/Framework
\*----------------------------------*/
HRESULT CPlayerSelScr::OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	m_pDevice = pd3dDevice;
	HRESULT hr = S_OK;

	V_RETURN(m_sprCol.OnCreateDevice(pd3dDevice));
	return hr;
}

HRESULT CPlayerSelScr::OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	m_pDevice = pd3dDevice;
	HRESULT hr = S_OK;
	V_RETURN(m_sprCol.OnResetDevice(pd3dDevice));
	return hr;
}

HRESULT CPlayerSelScr::OnLostDevice(void)
{
	HRESULT hr = S_OK;
	m_pDevice = NULL;
	V_RETURN(m_sprCol.OnLostDevice());
	return hr;
}

HRESULT CPlayerSelScr::OnDestroyDevice(void)
{
	m_pDevice = NULL;
	HRESULT hr = S_OK;
	V_RETURN(m_sprCol.OnDestroyDevice());

	return hr;
}

#pragma region HELPER_FUNCTIONS

UINT32 CPlayerSelScr::GetPrimaryWeaponNameHash(CPlayerCharSelection* pSel)
{
	return arrItemsByClass[pSel->eType].matOptionsByItemType[PSS_ITEMCAT_WEAPON].m_pData[pSel->nSelection[PSS_ITEMCAT_WEAPON]].shName.getHash();
}

UINT32 CPlayerSelScr::GetALTWeaponNameHash(CPlayerCharSelection* pSel)
{
	//ia alternativa de la arma principala
	UINT32 altNameHash = arrItemsByClass[pSel->eType].matOptionsByItemType[PSS_ITEMCAT_WEAPON].m_pData[pSel->nSelection[PSS_ITEMCAT_WEAPON]].shALTweaponTemplate.getHash();

	//daca nu e setata ia arma alternativa de la echipament (poate fi shioeld flash-ul sau altele)
	if(altNameHash == 0)
		altNameHash = arrItemsByClass[pSel->eType].matOptionsByItemType[PSS_ITEMCAT_EQUIPMENT].m_pData[pSel->nSelection[PSS_ITEMCAT_EQUIPMENT]].shALTweaponTemplate.getHash();

	return altNameHash;
}

UINT32 CPlayerSelScr::GetEquipmentTemplateModifierHash(CPlayerCharSelection* pSel)
{
	return arrItemsByClass[pSel->eType].matOptionsByItemType[PSS_ITEMCAT_EQUIPMENT].m_pData[pSel->nSelection[PSS_ITEMCAT_EQUIPMENT]].shModifierTemplate.getHash();
}

UINT32 CPlayerSelScr::GetGearNameHash(CPlayerCharSelection* pSel)
{
	return arrItemsByClass[pSel->eType].matOptionsByItemType[PSS_ITEMCAT_GEAR].m_pData[pSel->nSelection[PSS_ITEMCAT_GEAR]].shName.getHash();
}

bool CPlayerSelScr::GetUltimateAbility(CPlayerCharSelection* pSel, eStrategicAbility & eRetAbility, int & nRetNameStrIdx)
{
	sPSSItemData* pItem = GetItem(pSel->eType, PSS_ITEMCAT_ULTIMATE, pSel->nSelection[PSS_ITEMCAT_ULTIMATE]);
	if (pItem == null)
		return false;
	
	eRetAbility = (eStrategicAbility)GetListIndexByNameHash(pItem->shName.getHash(), eStrategicAbilityNames, K_CI_STRATEGIC_COUNT);
	nRetNameStrIdx = pItem->strIdxScreenName;
	return true;
}

///--- SELECTION CLASS ---

void CPlayerSelScr::SetWeaponsOptions(CPlayerCharSelection* pSel, bool bResetWeaponsSelection)
{
	for (int kk = 0; kk < PSS_ITEMCATS_COUNT; kk++)
	{
		//set default selection
		if(bResetWeaponsSelection)
			pSel->nSelection[kk] = 0;

		pSel->nPrice[kk] = 0;
	}
	//update selection locked flag
	SetSelectionPrices(pSel);
}


void CPlayerSelScr::SetSelectionPrices(CPlayerCharSelection* pSel)
{
	if (pSel == null)
		return;
	//player not selected yet so set prices on 0
	if (pSel->eType == K_PSS_CLASS_NOT_SELECTED)
	{
		for (int kk = 0; kk < PSS_ITEMCATS_COUNT; kk++)
		{
			pSel->nPrice[kk] = 0;
		}
		return;
	}
	//set actual prices
	for (int kk = 0; kk < PSS_ITEMCATS_COUNT; kk++)
	{
		UINT32 wpnhash = arrItemsByClass[pSel->eType].matOptionsByItemType[kk].m_pData[pSel->nSelection[kk]].shName.getHash();
		//set price
		pSel->nPrice[kk] = UTGetShop().GetItemPrice(wpnhash);
	}
}

void CPlayerSelScr::InitUpgradeBars(CPlayerCharSelection* pSel)
{
	if (pSel->bIsNetworkPlayer)
		return;
	//load XP
	pSel->nPlayerXPPts = g_userData[K_MEMID_TOTALXP_PER_CLASS_START + (int)pSel->eType];
	//load local points invested into upgrades
	for (int ll = 0; ll < K_PSS_UPGRADE_BARS_CNT; ll++)
	{
		int nbaridx = arrItemsByClass[(int)pSel->eType].arrUpgradeBarsIdx[ll];
		if (nbaridx >= 0)
		{
			pSel->arrUpgradeBarsPts[ll] = g_userData[K_MEMID_UPGRADE_BAR_POINTS_START + m_arrUpgradeBars[nbaridx]->nMemSlot];
			//make sure we don't have negative values
			if ((pSel->arrUpgradeBarsPts[ll] < 0) || (pSel->arrUpgradeBarsPts[ll] > m_arrUpgradeBars[nbaridx]->nTotalPoints))
			{
				//make sure we don't spend more than we have
				pSel->arrUpgradeBarsPts[ll] = 0;
			}
		}
	}
}

void CPlayerSelScr::SaveSelection()
{
	//save player selection only for local players
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if ((m_arrPlayers[kk].bSelected) && (m_arrPlayers[kk].eType != K_PSS_CLASS_NOT_SELECTED) && (m_arrPlayers[kk].nInstanceID != -1) && (m_arrPlayers[kk].bIsNetworkPlayer == false))
		{
			g_userData[K_MEMID_PANEL1_CLASS + kk * (K_MEMID_PANEL2_CLASS - K_MEMID_PANEL1_CLASS)] = (int)m_arrPlayers[kk].eType;
			for (int ll = 0; ll < PSS_ITEMCATS_COUNT; ll++)
			{
				int nDataOff = K_MEMID_PANEL1_CLASSDATA_START + kk * (K_MEMID_PANEL2_CLASS - K_MEMID_PANEL1_CLASS) + m_arrPlayers[kk].eType * 5;
				g_userData[ll + nDataOff] = m_arrPlayers[kk].nSelection[ll];
			}
		}
	}
}

void CPlayerSelScr::LoadSelectionForPanel(int nPlayerOrdinal)
{
	assert((nPlayerOrdinal >= 0) && (nPlayerOrdinal <= 1));
	if (m_arrPlayers[nPlayerOrdinal].bIsNetworkPlayer == true)
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING] Can't load data for network player! idx:%d", nPlayerOrdinal);
		return;
	}
	//load last selection from the local repo unless player is networked
	m_arrPlayers[nPlayerOrdinal].eType = (EPSSPlayerClass)g_userData[K_MEMID_PANEL1_CLASS + nPlayerOrdinal * (K_MEMID_PANEL2_CLASS - K_MEMID_PANEL1_CLASS)];
	//set the rest of the selection
	for (int ll = 0; ll < PSS_ITEMCATS_COUNT; ll++)
	{
		int nDataOff = K_MEMID_PANEL1_CLASSDATA_START + nPlayerOrdinal * (K_MEMID_PANEL2_CLASS - K_MEMID_PANEL1_CLASS) + m_arrPlayers[nPlayerOrdinal].eType * 5;
		m_arrPlayers[nPlayerOrdinal].nSelection[ll] = g_userData[ll + nDataOff];
		//make sure selection fits data (for modding)
		int nItemsCnt = arrItemsByClass[m_arrPlayers[nPlayerOrdinal].eType].matOptionsByItemType[ll].nCount;
		if ((m_arrPlayers[nPlayerOrdinal].nSelection[ll] < 0) || (m_arrPlayers[nPlayerOrdinal].nSelection[ll] >= nItemsCnt))
		{
			m_arrPlayers[nPlayerOrdinal].nSelection[ll] = 0;
		}
	}
	//load local points invested into upgrades
	InitUpgradeBars(&m_arrPlayers[nPlayerOrdinal]);

	SetSelectionPrices(&m_arrPlayers[nPlayerOrdinal]);
}

#pragma endregion HELPER_FUNCTIONS