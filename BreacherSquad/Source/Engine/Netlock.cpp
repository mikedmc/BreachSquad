#include "dxstdafx.h"

#include <sstream>

void CNetLock::sPacketNetlock::SaveButtonsPressedPercents(int nFrame, float arrSrcPressedPercents[K_CM_COMMANDS_COUNT])
{
	//#TODO: de facut serializare buna si mica pentru trimitere si primire prin retea
	//save frame
	m_nFrame = nFrame;
	//save states
	for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
	{
		//save bool for now
		m_bButStates[kk] = (arrSrcPressedPercents[kk] > 0.0f);
	}
}

void CNetLock::sPacketNetlock::GetButtonsPressedPercents(float arrDest[K_CM_COMMANDS_COUNT])
{
	for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
	{
		arrDest[kk] = (m_bButStates[kk] == true) ? 1.0f : 0.0f;
	}
}

void CNetLock::sPacketNetlock::Serialize(BitPacker & streamDest)
{
	//encode keys (EControllerCommand order) on bit flags, starting from LSB
	UINT16 unButMasks = 0;
	for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
	{
		if (m_bButStates[kk])
			unButMasks |= (1 << kk);
	}
	UINT16 unFlagsMask = m_wFrameFlags << K_CM_COMMANDS_COUNT;
	//add frame flags to button masks
	unButMasks |= unFlagsMask;
	//and write it
	streamDest.WriteUShort(unButMasks);
}

void CNetLock::sPacketNetlock::Deserialize(BitPacker & streamSource)
{
	//unpack button states
	UINT16 unButFlagsMasks = streamSource.ReadUShort();
	for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
	{
		register UINT32 bFlag = (1 << kk);
		if (unButFlagsMasks & bFlag)
			m_bButStates[kk] = true;
		else
			m_bButStates[kk] = false;
	}
	// unpack frame flags
	m_wFrameFlags = unButFlagsMasks >> K_CM_COMMANDS_COUNT;
}

DWORD CNetLock::sPacketNetlock::SerializeToDW()
{
	UINT16 unButMasks = 0;
	for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
	{
		if (m_bButStates[kk])
			unButMasks |= (1 << kk);
	}
	UINT16 unFlagsMask = m_wFrameFlags << K_CM_COMMANDS_COUNT;
	//add frame flags to button masks
	unButMasks |= unFlagsMask;
	//and write it
	return unButMasks;
}

CNetLock::sPacketNetlock::sPacketNetlock()
{
	Reset();
}

void CNetLock::sPacketNetlock::Reset()
{
	m_nFrame = -1; //default value different from 0
	m_dwSyncCheck = 0;
	m_wFrameFlags = 0;
	for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
	{
		m_bButStates[kk] = false;
	}
}

CNetLock::CNetLock()
{
	m_nStep = 0;
	m_fTimeSinceLastRCV = 0.0f;

	m_nToSend_Head = 0;
	m_nToSend_Tail = -1;
	m_nReceived_Head = 0;
	m_nReceived_Tail = -1;

	m_nReceived_SyncFrame = -1;

	m_ullCurLobbyID = 0;
	m_bIsGamePrivate = false;
}

CNetLock::~CNetLock()
{
}

void CNetLock::Net_EnterNetlock()
{
	m_nStep = 0;

	m_fTimeSinceLastRCV = 0.0f;

	m_nToSend_Head = 0;
	m_nToSend_Tail = -1;
	m_nReceived_Head = 0;
	m_nReceived_Tail = -1;

	m_nReceived_SyncFrame = -1;

	//reset arrays
	for (int kk = 0; kk < K_NETLOCK_MAX_STATE_PACKAGES; kk++)
	{
		m_arrToSend[kk].Reset();
		m_arrReceived[kk].Reset();
	}
}

bool CNetLock::Net_SendFrameData(int nLastSyncedFrame, DWORD dwSyncCheck)
{
	//number of states
	UCHAR nStates = m_nToSend_Tail - m_nToSend_Head + 1;
	if ((m_nToSend_Tail < 0) || (m_nToSend_Head < 0) || (nStates <= 0))
	{
		LOG_DBG(L"CNetLock::Send - trying to send empty package. Ignored.");
		return true;
	}
	//buffer
	unsigned char buffer[INetwork::MAX_UNRELIABLE_PACKET_SIZE];
	BitPacker stream(buffer, sizeof(buffer));
	///--- write header ---
	sPacketHeader head(K_NETMSG_TYPE_GAME_FRAMES);
	head.Serialize(stream);
	///------ last synced frame ------
	stream.WriteInt(nLastSyncedFrame);
	//sync check hash 
	stream.WriteUInt(dwSyncCheck);
	///------ STATES ------
	if ((nStates < 0) || (nStates >= CNetLock::K_NETLOCK_MAX_STATE_PACKAGES))
	{
		LOG(L"CNetLock::Send - illegal number of ToSend messages!");
		return false;
	}
	//write head (or starting frame)
	stream.WriteInt(m_nToSend_Head);
	//number of states
	stream.WriteUChar(nStates);
	//write actual states, all of them
	if (nStates > 0)
	{
		for (int kk = 0; kk < nStates; kk++)
		{
			UINT32 idx = (m_nToSend_Head + kk) % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES;
			m_arrToSend[idx].Serialize(stream);
		}
	}
#ifdef K_NET_ENGINE_DBG_VERBOSE
	LOG_DBG_BUFF(L"-> Sent: sndBuff[H:%d T:%d] lastSync:%d", m_nToSend_Head, m_nToSend_Tail, nLastSyncedFrame);
#endif
#ifdef K_SYNC_ENGINE_DBG_VERBOSE
	Net_LogFrameData(5);
#endif
	//send actual packet
	bool bSuccess = false;
	bSuccess = g_pNetwork->Send(buffer, (UINT32)stream.GetBytesWritten());
	if (!bSuccess)
	{
		ErrorBox(K_ERR_WARNING, L"CNetLock::Send could not send packet!");
	}

	//LOG(L"---send start(%d) states(%d) syncFrame(%d)", m_nToSend_Head, nStates, nLastSyncedFrame);

	return bSuccess;
}

bool CNetLock::Net_ReceiveFrameData(int nLastSyncedFrame)
{
	if (g_pNetwork == null)
	{
		ErrorBox(K_ERR_WARNING, L"CNetLock::Receive - pNet is null!");
		return false;
	}
	//read packet
	unsigned int bufferSize = 0;
	unsigned char buffer[INetwork::MAX_UNRELIABLE_PACKET_SIZE];

	if (g_pNetwork->Recv(buffer, INetwork::MAX_UNRELIABLE_PACKET_SIZE, &bufferSize))
	{
		BitPacker stream(buffer, bufferSize);
		///--- read header ---
		sPacketHeader head;
		if (head.Deserialize(stream) == false)
		{
			//change game state
			CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE);
			nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
			nevent->AddNamedArgINT32(L"stateErrorStrIdx", STR_NETWORK_ERROR_WRONG_VERSION);
			__Events().TriggerEvent(nevent);

			LOG(L"sPacketHeader::Deserialize - Different game versions! Please update to the last version!");
			return false;
		}

		if (head.eMessageType != K_NETMSG_TYPE_GAME_FRAMES)
		{
			//we accept level result messages too because they might come before we finish simulating
			//#TODO: there should be only one method that handles incoming traffic during sync and level results
			if (head.eMessageType == K_NETMSG_TYPE_COMMAND)
			{
				//read type of command
				CNetLock::eNetCommand cmd = (CNetLock::eNetCommand)stream.ReadUChar();
				//handle command messages
				if (cmd == CNetLock::K_NETCMD_LEVEL_RESULTS)
				{
					sPacketLevelResults packRec;
					packRec.Deserialize(stream);
					LOG(L"Net:Net_ReceiveFrameData received LEVEL_RESULT command with state[%d]: %d", Net_GetOtherPlayerIndex(), (int)packRec.m_eCurrentState);
					//save peer state locally
					m_arrLvlResPeerStates[Net_GetOtherPlayerIndex()] = packRec.m_eCurrentState;
				}
				else if (cmd == K_NETCMD_GAMEPLAY_CMD)
				{
					unsigned int nCmd = stream.ReadUInt();
					eNetCommandGameplay eCmd = (eNetCommandGameplay)nCmd;

					unsigned int nPeerIdx = Net_GetOtherPlayerIndex();

					switch (eCmd)
					{
						case K_GAMPLAYCMD_LEVEL_LOADED:
						{
							m_nPlayerFlags[nPeerIdx] |= K_NETLOCK_PLAYERFLAG_STARTED_LEVEL;
							LOG(L"NET: Peer loaded the level!");
						}
						break;
					}
				}
				else if (cmd == CNetLock::K_NETCMD_CHAT_LINE)
				{
#ifdef ENABLE_CHAT_WINDOW
					//read len
					int nLen = stream.ReadUInt();
					//read bytes
					WCHAR strChatLine[MAX_PATH] = { 0 };
					memset(strChatLine, 0, MAX_PATH * sizeof(WCHAR));
					//read string into it
					stream.ReadBytes(strChatLine, nLen * sizeof(WCHAR));
					//send to chat
					g_ChatWnd.AddLine(strChatLine, m_sNames[Net_GetOtherPlayerIndex()].text, K_CW_REMOTE_COLOR);
#endif
				}
				else
				{
					LOG(L"CNetlock::Net_ReceiveFrameData: wrong command received: %d", cmd);
				}
			}
			else
			{
				LOG(L"CNetlock::Net_ReceiveFrameData: wrong package type received: %d", head.eMessageType);
				return true;
			}
		}

		if (head.eMessageType == K_NETMSG_TYPE_GAME_FRAMES)
		{
			///------ last synced frame ------
			int nLastSyncFrame = m_nReceived_SyncFrame;
			m_nReceived_SyncFrame = stream.ReadInt();
			//read sync check for frame
			DWORD dwSyncCheck = stream.ReadUInt();
			// save received check locally for each confirmed frame
			if (m_nReceived_SyncFrame >= 0)
			{
				m_arrReceived[m_nReceived_SyncFrame % K_NETLOCK_MAX_STATE_PACKAGES].m_dwSyncCheck = dwSyncCheck;
				// delete all sync data between old and new sync frame so we don't check with an old value (make sure we don't delete before)
				if (nLastSyncFrame < 0)
					nLastSyncFrame = 0;
				for (int kk = nLastSyncFrame + 1; kk < m_nReceived_SyncFrame; kk++)
					m_arrReceived[kk % K_NETLOCK_MAX_STATE_PACKAGES].m_dwSyncCheck = 0;
			}

			///------ states ------
			//read starting frame and number of states
			int nStartFrame = stream.ReadInt();
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
			//check and see if we have skipped a frame due to package loss
			if ((nStartFrame > m_nReceived_Tail + 1) && (m_nReceived_Tail >= 0))
			{
				LOG(L"[WARNING] CNetlock: Skipped frame! frame %d, skipped %d", nStartFrame, m_nReceived_Tail + 1);
			}
#endif
			UCHAR nFramesCnt = stream.ReadUChar();
			if (nFramesCnt <= 0)
			{
				LOG(L"[WARNING] CNetLock:: Received package with no frames! Discarding.");
				return true;
			}

			//older package?
			bool bUpdateTail = true;
			int nTailIdx = nStartFrame + nFramesCnt - 1;
			if (nTailIdx < m_nReceived_Tail)
			{
				LOG(L"[WARNING] CNetLock:: Received older package! Not updating head and tail.");
				bUpdateTail = false;
			}

			//only read what I can store
			if (nFramesCnt > K_NETLOCK_MAX_STATE_PACKAGES - 1)
				nFramesCnt = K_NETLOCK_MAX_STATE_PACKAGES - 1;
			//package received is newer than stored one
			if (nStartFrame >= m_nReceived_Head)
			{
				for (int kk = 0; kk < nFramesCnt; kk++)
				{
					UINT32 idx = (nStartFrame + kk) % K_NETLOCK_MAX_STATE_PACKAGES;
					//make sure we don't overwrite older frames when cycling over
					if ((nStartFrame + kk) >= nLastSyncedFrame + K_NETLOCK_MAX_STATE_PACKAGES - 1)
					{
						//received too much data! exit for and ignore the rest
						LOG_DBG(L"[INFO] Too much data received for frame:%d", nLastSyncedFrame);
						break;
					}

					if (bUpdateTail)
					{
						// on new packages overwrite all
						m_arrReceived[idx].Deserialize(stream);
					}
					else
					{
						//on older packages just fill in the missing frames
						if(m_arrReceived[idx].m_nFrame < (nStartFrame + kk))
							m_arrReceived[idx].Deserialize(stream);
					}
					//write new frame number
					m_arrReceived[idx].m_nFrame = (nStartFrame + kk);
				}
				//advance tail
				if (bUpdateTail)
				{
					m_nReceived_Head = nStartFrame;
					m_nReceived_Tail = m_nReceived_Head + nFramesCnt - 1;
				}

				//make sure receiving buffer always contains last synced frame
				if (m_nReceived_Head > nLastSyncedFrame - K_NETLOCK_PACKAGE_CURFRAME_SAFEGUARD)
					m_nReceived_Head = nLastSyncedFrame - K_NETLOCK_PACKAGE_CURFRAME_SAFEGUARD;
				if (m_nReceived_Head < 0)
					m_nReceived_Head = 0;

#ifdef K_NET_ENGINE_DBG_VERBOSE
				LOG_DBG_BUFF(L"<- Recv: rcvBuff[H:%d T:%d] rcvSync:%d dTsinceLast:%.4f", m_nReceived_Head, m_nReceived_Tail, m_nReceived_SyncFrame, m_fTimeSinceLastRCV);
#endif
#ifdef K_SYNC_ENGINE_DBG_VERBOSE
				Net_LogFrameData(5);
#endif
				m_fTimeSinceLastRCV = 0.0f;
			}
			return true;
		}
	}

	return false;
}



void CNetLock::Net_LogFrameData(int nCount /*= 10*/)
{
	LOG_DBG_BUFF(L"-- Last transferred data ---");
	WCHAR tmptxt[MAX_PATH], tmptxtsm[MAX_PATH];
	//received
	int nfrom = m_nReceived_Tail - nCount;
	if (nfrom < 0)
		nfrom = 0;
	StringCchPrintf(tmptxt, MAX_PATH, L"recv: %s", tmptxtsm);
	for (int kk = nfrom; kk <= m_nReceived_Tail; kk++)
	{
		StringCchPrintf(tmptxtsm, MAX_PATH, L"%d:%d ", kk, m_arrReceived[kk % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].SerializeToDW());
		StringCchCat(tmptxt, MAX_PATH, tmptxtsm);
	}
	LOG_DBG_BUFF(tmptxt);
	//sent
	nfrom = m_nToSend_Tail - nCount;
	if (nfrom < 0)
		nfrom = 0;
	StringCchPrintf(tmptxt, MAX_PATH, L"sent: %s", tmptxtsm);
	for (int kk = nfrom; kk <= m_nToSend_Tail; kk++)
	{
		StringCchPrintf(tmptxtsm, MAX_PATH, L"%d:%d ", kk, m_arrToSend[kk % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].SerializeToDW());
		StringCchCat(tmptxt, MAX_PATH, tmptxtsm);
	}
	LOG_DBG_BUFF(tmptxt);
}

void CNetLock::Net_UpdateEventLoop()
{
	INetwork::sEvent ev;
	while (g_pNetwork->PopEvent(ev))
	{
		g_pLog->Write("Game::Net_UpdateLobby() PopEvent %s, data=%llu\n", eLobbyEventStr[ev.eType], ev.ullData);
		switch (ev.eType)
		{
			case INetwork::EV_LOBBY_LIST_RECEIVED:
			{
				if (GameState::state != GAME_STATE_NET_LOBBY)
					break;
				//only if not in lobby
				const INetwork::sLobby& lobby = g_pNetwork->GetCurrentLobby();
				if (lobby.eState == INetwork::LOBBY_UNKNOWN)
				{
					if (ev.ullData == 0)
					{
						// didn't find any existing lobby, so create one ourselves
						Net_CreateLobby(false);

					}
					else
					{
						// join existing lobby
						assert(ev.ullData == 1);
						uint64_t lobbyId = g_pNetwork->GetLobbyIdByIndex(0);
						g_pNetwork->JoinLobby(lobbyId);
					}
				}
			}
			break;

			// failed to join, try searching again
			case INetwork::EV_LOBBY_JOIN_FAILED:
			{
				if (GameState::state == GAME_STATE_NET_LOBBY)
					g_pNetwork->RequestLobbyList(false, 1);
				else
					Net_QuitLobby();
			}
			break;

			// creation failed (?!), try again by first searching and then creating
			case INetwork::EV_LOBBY_CREATE_FAILED:
			{
				if (GameState::state == GAME_STATE_NET_LOBBY)
					g_pNetwork->RequestLobbyList(false, 1);
				else
					Net_QuitLobby();
			}
			break;

			// nothing to do, we're ready to go
			case INetwork::EV_LOBBY_JOIN_SUCCESS:
			{
				if (GameState::state == GAME_STATE_NET_LOBBY)
				{
				}
			}
			break;

				// nothing to do, we're waiting for another player to connect
			case INetwork::EV_LOBBY_CREATE_SUCCESS:
				break;

				// the other player in the lobby has left. we must leave the lobby as well as start over
			case INetwork::EV_LOBBY_PLAYER_LEFT:
			{
				LOG(L"Net:: Lost Connection to Peer! Player left!");

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE) || defined(K_SYNC_ENGINE_DBG_VERBOSE)
				// show last input and scene actors on disconnects
				Net_LogFrameData(10);
				LOG(L"-- scene actors %d --", __Sim().m_arrActors.Count());
				for ( auto node: __Sim().m_arrActors)
				{
					CActor* act = &node->m_data;
					LOG(L"%s ID %d pos(%.4f, %.4f)", act->_template.shID.text, act->ID, act->pos.xyz.x, act->pos.xyz.y);
				}
#endif

				Net_QuitLobby();

				if (GameState::state == GAME_STATE_NET_LOBBY)
				{
					//#TODO: could send you back to older state
					//change game state
					CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE);
					nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
					nevent->AddNamedArgINT32(L"stateErrorStrIdx", STR_NETWORK_ERROR_PLAYER_LEFT);
					__Events().TriggerEvent(nevent);
				}
				else if (UTApp().IsGameNetworked())
				{
					//change game state
					CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE);
					nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
					nevent->AddNamedArgINT32(L"stateErrorStrIdx", STR_NETWORK_ERROR_PLAYER_LEFT);
					__Events().TriggerEvent(nevent);
				}
				else
				{
					LOG(L"Net:: Player left but we are on single player. Ignoring.");
				}
			}
			break;

			case INetwork::EV_LOBBY_INVITED:
			{
				//triggered after you click accept
				assert(ev.ullData != 0);
				if (GameState::state <= GAME_STATE_LOADING)
				{
					LOG(L"Net:: Invitation to lobby ignored. It got accepted too early (loading screen or before)");
					break;
				}

				LOG(L"Net:: Invitation accepted to lobby: %llu", ev.ullData);
				//if we've been invited to a lobby from the Steam commandline then go directly into the lobby
				g_netlock.m_ullCurLobbyID = ev.ullData;
				//change state
				CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
				nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_NET_LOBBY);
				nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
				//setting join state
				nevent->AddNamedArgINT32(L"arg1", (int)CApplicationSettings::K_NETGAME_TYPE_QUICK_MATCH);
				__Events().QueueEvent(nevent);
			}
			break;

			case INetwork::EV_CONNECTION_LOST:
			{
				LOG(L"Net: EV_CONNECTION_LOST");

				Net_QuitLobby();
				if (UTApp().IsGameNetworked())
				{
					//change game state
					CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE);
					nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
					nevent->AddNamedArgINT32(L"stateErrorStrIdx", STR_NETWORK_ERROR_GENERIC);
					__Events().TriggerEvent(nevent);
				}
			}
			break;

			default:
				break;
		}
	}

	g_pNetwork->Update();
}

/************************************************************************/
/* LOBBY
/************************************************************************/
void CNetLock::Net_CreateLobby(bool bFriendsOnly)
{
	//write CRC string
	char strCRC[MAX_PATH] = { 0 };
	StringCchPrintfA(strCRC, MAX_PATH, "%u", UTApp().m_Settings.dev_unCurrentModsCRC);
	char strMode[MAX_PATH] = { 0 };
	//StringCchPrintfA(strMode, MAX_PATH, "%u", (int)g_gameMode);

	const char* pKeyDataValues[][2] =
	{
		{"version", _VERSION_CHARSTR_},
		{"CRC", strCRC}, 
		{"Mode", strMode}
	};
	//create it now
	g_pNetwork->CreateLobby(bFriendsOnly, sizeof(pKeyDataValues) / sizeof(pKeyDataValues[0]), pKeyDataValues);
}

void CNetLock::Net_RequestLobbyList(int nMaxLobbies /*,bool bMatchCRC*/)
{
	//write CRC string
	char strCRC[MAX_PATH] = { 0 };
	StringCchPrintfA(strCRC, MAX_PATH, "%u", UTApp().m_Settings.dev_unCurrentModsCRC);
	char strMode[MAX_PATH] = { 0 };
	//StringCchPrintfA(strMode, MAX_PATH, "%u", (int)g_gameMode);

	const char* pKeyDataValues[][2] =
	{
		{"version", _VERSION_CHARSTR_},
		{"CRC", strCRC},
		{"Mode", strMode}
	};

	g_pNetwork->RequestLobbyList(false, nMaxLobbies, sizeof(pKeyDataValues) / sizeof(pKeyDataValues[0]) /*- (bMatchCRC ? 0 : 1)*/, pKeyDataValues);
}

void CNetLock::Net_EnterLobby(bool bHostGame, bool bPrivateLobby)
{
	//save private lobby request
	m_bIsGamePrivate = bPrivateLobby;
	//default flags
	m_nStep = 0;
	//default lobby data
	m_ucSelChapter = 0;
	m_ucSelLevel = 0;
	//m_ucSelMode = GAME_MODE_CLASSIC;
	m_unRandomSeed = 6661;
	//names
	m_sNames[0].Reset();
	m_sNames[1].Reset();
	m_nPlayerFlags[0] = 0;
	m_nPlayerFlags[1] = 0;
	//downloaded level
	memset(m_csModID_DwnLvl, 0, sizeof(m_csModID_DwnLvl));
	//text shown on connection window
	__Texts().SetString(STR_CONNECTION_MSG, __Texts().strings[STR_WAITING_PEER]->sText);

	// request lobby ahead of time or join a lobby if invited
	if (m_ullCurLobbyID == 0) //create/join lobby
	{
		if (bHostGame)
		{
			Net_CreateLobby(bPrivateLobby);
		}
		else
		{
			Net_RequestLobbyList(1);
		}
	}
	else //join invited lobby
	{
			// join existing lobby (if invited by command line)
			assert(m_ullCurLobbyID != 0);
			g_pNetwork->JoinLobby(m_ullCurLobbyID);
			//reset invitation command
			m_ullCurLobbyID = 0;
	}
}

void CNetLock::Net_QuitLobby()
{
	g_pNetwork->LeaveLobby();
	//names
	m_sNames[0].Reset();
	m_sNames[1].Reset();
	m_nPlayerFlags[0] = 0;
	m_nPlayerFlags[1] = 0;
	//erase names from 
	__Texts().SetString(STR_NETWORK_HOST_NAME, L"host");
	__Texts().SetString(STR_NETWORK_PEER_NAME, L"peer");
}

void CNetLock::Net_UpdateLobby(float dTime)
{
	Net_UpdateEventLoop();

	// update and query current lobby status
	const INetwork::sLobby& lobby = g_pNetwork->GetCurrentLobby();
	if (lobby.eState != INetwork::LOBBY_IN_LOBBY || lobby.iNumPlayers != 2)
	{
		// waiting to establish a connection with a peer
		if (m_nStep == 0)
		{
			return;
		}
		else //make sure we still have a connection (should receive TIMEOUT or smthg in UpdateEventLoop)
		{
			//change game state
			CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE);
			nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
			nevent->AddNamedArgINT32(L"stateErrorStrIdx", STR_NETWORK_ERROR_GENERIC);
			__Events().TriggerEvent(nevent);

			LOG(L"[Error] Net_UpdateLobby - Peer left lobby or connection timeout!");
			return;
		}

	}

	//buffer
	unsigned int bufferSize = 0;
	unsigned char buffer[INetwork::MAX_UNRELIABLE_PACKET_SIZE];

	//talk to peer and setup common ground
	if (m_nStep == 0)
	{
		//set random seed
		m_unRandomSeed = GetTickCount();

		//decide map with 
		if (UTApp().m_Settings.devnet_eNetGameType == CApplicationSettings::K_NETGAME_TYPE_QUICK_MATCH)
		{
			//random level on quick match
			int nLevel = __Sim().GetNextRandomLevel();
			m_ucSelChapter = nLevel / K_GAME_LEVELS_PER_CHAPTER;
			m_ucSelLevel = nLevel % K_GAME_LEVELS_PER_CHAPTER;
		}
		else //hosted game, selected chapter and level already set from other screens
		{
			m_ucSelChapter = g_userData[K_MEMID_SELECTED_CHAPTER];
			m_ucSelLevel = g_userData[K_MEMID_SELECTED_LEVEL];
		}
		//save selected mode
		m_ucSelMode = (BYTE)0/*g_gameMode*/;
		
		//#MODDING: set data for modding handshake
		m_ucModData = 0;
		memset(m_csModID_DwnLvl, 0, sizeof(m_csModID_DwnLvl));
		//are we playing a custom level?
		if ((m_bIsGamePrivate) && (g_userData[K_MEMID_SELECTED_CHAPTER] >= UTGetChaptersList().GetChaptersCnt()))
		{
#ifdef ENABLE_STEAM_WORKSHOP
			int nModIdx = g_userData[K_MEMID_MOD_DWNLVL_SELECTED];
			CModsManager::CModDescriptor *mod = __Mods().GetModDescByIndex(nModIdx);
			if (mod != null)
			{
				m_ucModData = 1;
				//write mode ID
				std::ostringstream stream;
				stream << mod->uID;

				StringCchPrintfA(m_csModID_DwnLvl, MAX_PATH, "%s", stream.str().c_str());
			}
#endif // ENABLE_STEAM_WORKSHOP
		}

		if (lobby.bIAmOwner)
		{
			// owner sends lobby data
			BitPacker stream(buffer, sizeof(buffer));
			sPacketHeader head(K_NETMSG_TYPE_COMMAND);
			head.Serialize(stream);
			//write type of command
			stream.WriteUChar((BYTE)K_NETCMD_LOBBY_HANDSHAKE);

			stream.WriteUInt(m_unRandomSeed);
			stream.WriteUChar(m_ucSelMode);
			stream.WriteUChar(m_ucSelChapter);
			stream.WriteUChar(m_ucSelLevel);
			stream.WriteUInt(UTApp().m_Settings.dev_unCurrentModsCRC);
			//mod data
			stream.WriteUChar(m_ucModData);
			if (m_ucModData != 0)
			{
				LOG("Network:: Lobby: MASTER sent mod ID: %s", m_csModID_DwnLvl);
				stream.WriteString(m_csModID_DwnLvl);
			}

			LOG(L"Network:: Lobby: MASTER sent seed %d, mode %d, chapter %d, level %d, CRC[%08x]", m_unRandomSeed, m_ucSelMode, m_ucSelChapter, m_ucSelLevel, UTApp().m_Settings.dev_unCurrentModsCRC);
			g_pNetwork->SendReliable(stream.GetData(), stream.GetBytesWritten());
		}
		else
		{
			// client sends just the header and CRC
			BitPacker stream(buffer, sizeof(buffer));
			sPacketHeader head(K_NETMSG_TYPE_COMMAND);
			head.Serialize(stream);
			//write type of command
			stream.WriteUChar((BYTE)K_NETCMD_LOBBY_HANDSHAKE);
			stream.WriteUInt(UTApp().m_Settings.dev_unCurrentModsCRC);

			LOG(L"Network:: Lobby: SLAVE sent empty header and CRC[%08x]", UTApp().m_Settings.dev_unCurrentModsCRC);
			g_pNetwork->SendReliable(stream.GetData(), stream.GetBytesWritten());
		}
		//flag1 is used to send the messages only once
		m_nStep = 1;

		///--- save user names ---
		WCHAR szTemp[MAX_PATH] = { 0 };
		//host is always on idx 0 in lobby
		CStringsManager::UTF8toWCHAR(lobby.players[0].m_rgchName, szTemp, MAX_PATH);
		m_sNames[0].Init(szTemp);
		CStringsManager::UTF8toWCHAR(lobby.players[1].m_rgchName, szTemp, MAX_PATH);
		m_sNames[1].Init(szTemp);
		//write names in strings too
		__Texts().SetString(STR_NETWORK_HOST_NAME, m_sNames[0].text);
		__Texts().SetString(STR_NETWORK_PEER_NAME, m_sNames[1].text);

		//change interface text from waiting to connecting
		__Texts().ReplaceTokenString(STR_CONNECTION_MSG, STR_CONNECTING_PEER_X, 1, m_sNames[Net_GetOtherPlayerIndex()].text);

		//#TODO: disable CANCEL button (should have timeout)
		//CCtrlLayer* layer = UTGetControlsManager().GetTopmostInputLayer();
		//if (layer)
		//{
		//	layer->ControlSetDisableByName(true, "BUT_CANCEL_LOBBY");
		//}
	}

	// both the client and the server will wait until they receive a message from the other player
	if (m_nStep == 1)
	{
		bool bHandshaked = false;
		while (g_pNetwork->Recv(buffer, INetwork::MAX_UNRELIABLE_PACKET_SIZE, &bufferSize))
		{
			if (!bufferSize)
				continue; // decompression failed

			BitPacker stream(buffer, bufferSize);
			sPacketHeader packHead;
			if (packHead.Deserialize(stream) == false)
			{
				//change game state
				CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE);
				nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
				nevent->AddNamedArgINT32(L"stateErrorStrIdx", STR_NETWORK_ERROR_WRONG_VERSION);
				__Events().TriggerEvent(nevent);

				LOG(L"[Error] Net_UpdateLobby::sPacketHeader.Deserialize - Different game versions! Please update to the last version!");

				return;
			}

			//read type of command
			eNetCommand cmd = (eNetCommand)stream.ReadUChar();
			//ignore non commands or not handshake commands
			if ((packHead.eMessageType != K_NETMSG_TYPE_COMMAND) || (cmd != K_NETCMD_LOBBY_HANDSHAKE))
				continue; 

			bool bVersionDifferentErr = false;
			//non-owner peer reads data and saves it locally
			if (!lobby.bIAmOwner)
			{
				m_unRandomSeed = stream.ReadUInt();
				m_ucSelMode = stream.ReadUChar();
				m_ucSelChapter = stream.ReadUChar();
				m_ucSelLevel = stream.ReadUChar();
				UINT32 unPeerCRC = stream.ReadUInt();
				if (unPeerCRC != UTApp().m_Settings.dev_unCurrentModsCRC)
					bVersionDifferentErr = true;
				//read mod data
				m_ucModData = stream.ReadUChar();
				if (m_ucModData != 0)
				{
					int strl = stream.ReadString(m_csModID_DwnLvl, MAX_PATH);
					LOG("Network:: Lobby: SLAVE received mod ID: %s", m_csModID_DwnLvl);
					//check to see if we have the mod and that it is enabled
#ifdef ENABLE_STEAM_WORKSHOP
#ifdef WIN32
					UINT64 uiModID = _atoi64(m_csModID_DwnLvl);
#else
					UINT64 uiModID = atoll(m_csModID_DwnLvl);
#endif
					CModsManager::CModDescriptor* mod = __Mods().GetModDescByID(uiModID);
					if ((mod == null) || (mod->bActive == false))
					{
						bVersionDifferentErr = true;
					}
#endif // ENABLE_STEAM_WORKSHOP

				}

				LOG(L"Network:: Lobby: SLAVE received seed %d, mode %d, chapter %d, level %d, CRC[%08x]", m_unRandomSeed, m_ucSelMode, m_ucSelChapter, m_ucSelLevel, unPeerCRC);
			}
			else //owner reads only CRC from peer
			{
				UINT32 unPeerCRC = stream.ReadUInt();
				if (unPeerCRC != UTApp().m_Settings.dev_unCurrentModsCRC)
					bVersionDifferentErr = true;

				LOG(L"Network:: Lobby: MASTER received CRC[%08x]", unPeerCRC);
			}

			//check CRC match
			if (bVersionDifferentErr == true)
			{
				CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE);
				nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
				nevent->AddNamedArgINT32(L"stateErrorStrIdx", STR_NETWORK_ERROR_WRONG_VERSION);
				__Events().TriggerEvent(nevent);

				LOG(L"[Error] Net_UpdateLobby::peer CRC mismatch! Different mods activated!");
				return;
			}

			bHandshaked = true;
			m_nStep = 2; //next step
			break;
		}

		if (!bHandshaked)
			return;
	}

	if (m_nStep == 2)
	{
		//set loading levels (important for slave)
		//g_gameMode = (eGameMode)m_ucSelMode;
		g_userData[K_MEMID_SELECTED_CHAPTER] = m_ucSelChapter;
		g_userData[K_MEMID_SELECTED_LEVEL] = m_ucSelLevel;

		//#MODDING: set local vars on specified mod
		g_userData[K_MEMID_MOD_DWNLVL_SELECTED] = -1;
#ifdef ENABLE_STEAM_WORKSHOP
		if (m_ucModData != 0)
		{
#ifdef WIN32
			UINT64 uiModID = _atoi64(m_csModID_DwnLvl);
#else
			UINT64 uiModID = atoll(m_csModID_DwnLvl);
#endif
			int nSelModIdx = __Mods().GetModIndexByID(uiModID);
			g_userData[K_MEMID_MOD_DWNLVL_SELECTED] = nSelModIdx;
		}
#endif // ENABLE_STEAM_WORKSHOP


		CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE);
		nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_PLAYER_SELECTION);
		nevent->AddNamedArgINT32(L"arg1", 1); //reset player selection
		nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
		__Events().QueueEvent(nevent);

		m_nStep = 3; //next step
	}
}

void CNetLock::Net_ResetLevelResults()
{
	//reset peer state
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		m_arrLvlResPeerStates[kk] = sPacketLevelResults::K_LEVRES_STATE_UNDEFINED;
	}
}



void CNetLock::Net_EnterLevelResults()
{
	//send a reliable package to let peer know we finished the game
	CNetLock::sPacketLevelResults sPack(CNetLock::sPacketLevelResults::K_LEVRES_STATE_ACK_GAME_FINISHED);
	g_netlock.Net_SendLevelResultsCommand(sPack);
}

bool CNetLock::Net_SendLevelResultsCommand(sPacketLevelResults & playerState)
{
	LOG(L"Net:SendLevelResults sent state: %d", (int)playerState.m_eCurrentState);
	//buffer
	unsigned int bufferSize = 0;
	unsigned char buffer[INetwork::MAX_UNRELIABLE_PACKET_SIZE];
	// owner sends lobby data
	BitPacker stream(buffer, sizeof(buffer));

	//Send selection over the net
	sPacketHeader head(K_NETMSG_TYPE_COMMAND);
	head.Serialize(stream);
	//write type of command
	stream.WriteUChar((BYTE)K_NETCMD_LEVEL_RESULTS);
	//serialize
	playerState.Serialize(stream);

	//save player state locally
	m_arrLvlResPeerStates[Net_GetPlayerIndex()] = playerState.m_eCurrentState;

	return g_pNetwork->SendReliable(stream.GetData(), stream.GetBytesWritten());
}

void CNetLock::Net_UpdateLevelResults(float dTime)
{
	sPacketLevelResults packRec;

	// update and query current lobby status
	const INetwork::sLobby& lobby = g_pNetwork->GetCurrentLobby();
	if (lobby.eState != INetwork::LOBBY_IN_LOBBY || lobby.iNumPlayers != 2)
	{
		return;
	}

	//buffer
	unsigned int bufferSize = 0;
	unsigned char buffer[INetwork::MAX_UNRELIABLE_PACKET_SIZE];

	//read all messages 
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
				__Events().TriggerEvent(nevent);

				LOG(L"[Error] Net_UpdateLevelResults::sPacketHeader.Deserialize - Different game versions! Please update to the last version!");
			}

			if (packHead.eMessageType == CNetLock::K_NETMSG_TYPE_COMMAND)
			{
				//read type of command
				CNetLock::eNetCommand cmd = (CNetLock::eNetCommand)stream.ReadUChar();
				//tratam doar mesajele de tip comanda
				if (cmd == CNetLock::K_NETCMD_LEVEL_RESULTS)
				{
					packRec.Deserialize(stream);
					LOG(L"Net:UpdateLevelResults received state[%d]: %d", Net_GetOtherPlayerIndex(), (int)packRec.m_eCurrentState);
					//save peer state locally
					m_arrLvlResPeerStates[Net_GetOtherPlayerIndex()] = packRec.m_eCurrentState;
				}
				else
				{
					LOG(L"Net:UpdateLevelResults dump wrong packet type! head %d cmd %d", packHead.eMessageType, cmd);
				}
			}
		}
	}

	//all we do here is to write button presses into m_arrLvlResPeerStates. The array gets checked outside this class
}

int CNetLock::Net_LevelResultsCountStates(sPacketLevelResults::eLevelResultsState eState)
{
	int found = 0;
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if (m_arrLvlResPeerStates[kk] == eState)
			found++;
	}

	return found;
}

void CNetLock::Net_LevelResultsClearStates()
{
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		m_arrLvlResPeerStates[kk] = sPacketLevelResults::K_LEVRES_STATE_UNDEFINED;
	}
}

void CNetLock::Net_EnterPlayerSelScreen()
{
#ifdef K_SYNC_ENGINE_DBG_VERBOSE
	DebugLogClear();
	LOG(L"--- Cleared log on enter COOP player selection screen ---");
#endif
}

bool CNetLock::Net_SendPlayerSelection(sPacketPlayerSelection & playerSel)
{
	//buffer
	unsigned int bufferSize = 0;
	unsigned char buffer[INetwork::MAX_UNRELIABLE_PACKET_SIZE];
	// owner sends lobby data
	BitPacker stream(buffer, sizeof(buffer));

	//Send selection over the net
	sPacketHeader head(K_NETMSG_TYPE_COMMAND);
	head.Serialize(stream);
	//write type of command
	stream.WriteUChar((BYTE)K_NETCMD_PLAYER_SELECTION);
	//serialize received player selection struct
	playerSel.Serialize(stream);
	//send again random seed and selected level with every selection (only if hosting the game)
	if (Net_GetIAmHosting())
	{
		stream.WriteUInt(m_unRandomSeed);
		stream.WriteUChar(m_ucSelMode);
		stream.WriteUChar(m_ucSelChapter);
		stream.WriteUChar(m_ucSelLevel);

		LOG(L"Net::Net_SendPlayerSelection sent randseed(%d), mode(%d), lvl(%d.%d). playertype:%d bselected:%d", m_unRandomSeed, m_ucSelMode, m_ucSelChapter, m_ucSelLevel, playerSel.eType, playerSel.bSelected);
	}
	else
	{
		LOG(L"Net::Net_SendPlayerSelection sent playertype:%d bselected:%d", playerSel.eType, playerSel.bSelected);
	}

	return g_pNetwork->SendReliable(stream.GetData(), stream.GetBytesWritten());
}

void CNetLock::Net_ExitPlayerSelScreen()
{
	//reset all player flags
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
		m_nPlayerFlags[kk] = 0;
	//set needed flags, if any
}

bool CNetLock::Net_SendChatLine(WCHAR* strChatLine)
{
	if (strChatLine == null)
		return false;
	//buffer
	unsigned int bufferSize = 0;
	unsigned char buffer[INetwork::MAX_UNRELIABLE_PACKET_SIZE];
	// owner sends lobby data
	BitPacker stream(buffer, sizeof(buffer));

	//Send selection over the net
	sPacketHeader head(K_NETMSG_TYPE_COMMAND);
	head.Serialize(stream);
	//write type of command
	stream.WriteUChar((BYTE)K_NETCMD_CHAT_LINE);
	//measure string
	int nLen = wcsnlen_s(strChatLine, MAX_PATH);
	CLAMP(nLen, 0, MAX_PATH);
	if (nLen == 0)
		return false;
	//write length
	stream.WriteUInt(nLen);
	//write chars
	stream.WriteBytes(strChatLine, nLen * sizeof(WCHAR));
	
	//send it reliable
	return g_pNetwork->SendReliable(stream.GetData(), stream.GetBytesWritten());
}

bool CNetLock::Net_SendGameplayCommand(eNetCommandGameplay eCmd)
{
	if ((eCmd <= K_GAMEPLAYCMD_UNDEFINED) || (eCmd >= K_GAMPLAYCMDS_COUNT))
		return false;

	//buffer
	unsigned int bufferSize = 0;
	unsigned char buffer[INetwork::MAX_UNRELIABLE_PACKET_SIZE];
	// owner sends lobby data
	BitPacker stream(buffer, sizeof(buffer));

	//Send selection over the net
	sPacketHeader head(K_NETMSG_TYPE_COMMAND);
	head.Serialize(stream);
	//write type of command
	stream.WriteUChar((BYTE)K_NETCMD_GAMEPLAY_CMD);
	unsigned int unGamplayCmd = (UINT32)eCmd;
	stream.WriteUInt(unGamplayCmd);

	switch (eCmd)
	{
		case CNetLock::K_GAMPLAYCMD_LEVEL_LOADED:
		{
			//mark us as ready locally
			unsigned int nLocalIdx = Net_GetPlayerIndex();
			m_nPlayerFlags[nLocalIdx] |= K_NETLOCK_PLAYERFLAG_STARTED_LEVEL;
		}
		break;
		default:
			break;
	}

	//send it reliable
	return g_pNetwork->SendReliable(stream.GetData(), stream.GetBytesWritten());
}

