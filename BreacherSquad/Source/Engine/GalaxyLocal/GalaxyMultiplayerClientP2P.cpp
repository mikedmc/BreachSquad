#include "dxstdafx.h"

#ifdef ENABLE_GALAXY

#include "GalaxyMultiplayerClientP2P.h"

#include "galaxy/GalaxyApi.h"
#include "GalaxyUtils.h"

CGalaxyClientP2P s_networkClient;
INetwork* g_pNetwork = &s_networkClient;

using namespace galaxy::api;

CGalaxyClientP2P::CGalaxyClientP2P()
	: m_bRequestingLobbies(false)
	, m_bRegistered(false)
{
}

CGalaxyClientP2P::~CGalaxyClientP2P()
{
	Reset();

	if (ListenerRegistrar())
	{
		UnregisterAsGalaxyListener<ILobbyCreatedListener>(this);
		UnregisterAsGalaxyListener<ILobbyEnteredListener>(this);
		UnregisterAsGalaxyListener<IPersonaDataChangedListener>(this);
		UnregisterAsGalaxyListener<ILobbyDataUpdateListener>(this);
		UnregisterAsGalaxyListener<ILobbyMemberStateListener>(this);
		UnregisterAsGalaxyListener<ILobbyListListener>(this);
		UnregisterAsGalaxyListener<ILobbyDataRetrieveListener>(this);
		UnregisterAsGalaxyListener<IGameJoinRequestedListener>(this);
		UnregisterAsGalaxyListener<IGogServicesConnectionStateListener>(this);
		UnregisterAsGalaxyListener<IOperationalStateChangeListener>(this);
	}
}

bool CGalaxyClientP2P::Start()
{
	if (!NetworkBase::Start())
		return false;

	if (!m_bRegistered)
	{
		RegisterAsGalaxyListener<ILobbyCreatedListener>(this);
		RegisterAsGalaxyListener<ILobbyEnteredListener>(this);
		RegisterAsGalaxyListener<IPersonaDataChangedListener>(this);
		RegisterAsGalaxyListener<ILobbyDataUpdateListener>(this);
		RegisterAsGalaxyListener<ILobbyMemberStateListener>(this);
		RegisterAsGalaxyListener<ILobbyListListener>(this);
		RegisterAsGalaxyListener<ILobbyDataRetrieveListener>(this);
		RegisterAsGalaxyListener<IGameJoinRequestedListener>(this);
		RegisterAsGalaxyListener<IGogServicesConnectionStateListener>(this);
		RegisterAsGalaxyListener<IOperationalStateChangeListener>(this);
		m_bRegistered = true;
	}

	m_lobby.eState = INetwork::LOBBY_UNKNOWN;
	m_peerID = 0;

	if (IsUserConnected())
	{
		Friends()->SetRichPresence("status", "Idle");
	}

	m_bRequestingLobbies = false;
	return true;
}

void CGalaxyClientP2P::Reset()
{
	NetworkBase::Reset();

	m_bRequestingLobbies = false;
	m_keyValueLobbyParams.clear();
	m_listLobbies.clear();

	if (IsUserConnected())
	{
		Friends()->SetRichPresence("status", "Idle");
	}

	// reset connect flag
	UpdateRichPresenceConnectionInfo();
}

void CGalaxyClientP2P::Update()
{
	NetworkBase::Update();
}

bool CGalaxyClientP2P::Send(const void* pData, unsigned int dataSize)
{
	if (!m_peerID.IsValid())
		return false;

	if (dataSize > INetwork::MAX_UNRELIABLE_PACKET_SIZE)
		return false;

	if (SimulateLatencySend(pData, dataSize))
		return true;

	if (!Networking()->SendP2PPacket(m_peerID, pData, dataSize, P2P_SEND_UNRELIABLE, k_EClientConnectionChannel_Game))
	{
		g_pLog->Write("Network:: Failed sending data to client!\n");
		return false;
	}
	return true;
}

bool CGalaxyClientP2P::SendReliable(const void* pData, unsigned int dataSize)
{
	if (!m_peerID.IsValid())
		return false;

	if (dataSize > INetwork::MAX_RELIABLE_PACKET_SIZE)
		return false;

	if (!Networking()->SendP2PPacket(m_peerID, pData, dataSize, P2P_SEND_RELIABLE, k_EClientConnectionChannel_Game))
	{
		g_pLog->Write("Network:: Failed sending data to client!\n");
		return false;
	}
	return true;
}

bool CGalaxyClientP2P::Recv(void* pData, unsigned int uiDataAllocSize, unsigned int* pActualRecvSize)
{
	uint32_t uiAvailablePacketSize = 0;
	uint32_t uiReadPacketActualSize = 0;
	*pActualRecvSize = 0;

	// see if there is any data waiting on the socket
	if (Networking()->IsP2PPacketAvailable(&uiAvailablePacketSize, k_EClientConnectionChannel_Game))
	{
		assert(uiAvailablePacketSize <= uiDataAllocSize);
		if (uiAvailablePacketSize > uiDataAllocSize)
		{
			g_pLog->Write("Network:: Received packet size is larger than expected ! \n");
		}

		GalaxyID playerIDRemote;
		if (!Networking()->ReadP2PPacket(pData, uiDataAllocSize, &uiReadPacketActualSize, playerIDRemote, k_EClientConnectionChannel_Game))
			return false;

		assert(uiReadPacketActualSize == uiAvailablePacketSize);
		if (uiReadPacketActualSize != uiAvailablePacketSize)
		{
			g_pLog->Write("Network:: Got packet size '%d', expecting to have '%d' ! \n",
				uiReadPacketActualSize, uiDataAllocSize);
		}

		if (uiReadPacketActualSize > uiDataAllocSize)
		{
			g_pLog->Write("Network:: Got packet size '%d', only have room for '%d' - truncating ! \n",
				uiReadPacketActualSize, uiDataAllocSize);
		}

		*pActualRecvSize = uiReadPacketActualSize;
	}

	SimulateLatencyRecv(pData, &uiDataAllocSize, pActualRecvSize);

	return *pActualRecvSize != 0;
}

void CGalaxyClientP2P::CreateLobby(bool bFriendsOnly, const int iNumKeyValues /*= 0*/, const char* pKeyDataValues[][2] /*= NULL*/)
{
	if (!IsUserConnected())
	{
		AddEvent(INetwork::EV_CONNECTION_LOST, 1); // re-send event that connection is lost
		return;
	}

	if (m_lobby.eState != LOBBY_UNKNOWN)
	{
		LeaveLobby();
	}

	Matchmaking()->CreateLobby(bFriendsOnly ? LOBBY_TYPE_FRIENDS_ONLY : LOBBY_TYPE_PUBLIC, MAX_PLAYERS_PER_GAME, true, LOBBY_TOPOLOGY_TYPE_FCM);

	if (GetError())
	{
		g_pLog->Write("Error creating lobby: %s\n", GetError()->GetMsg());
		return;
	}

	// delete old params if present
	m_keyValueLobbyParams.clear();

	// copy params
	if (iNumKeyValues != 0)
	{
		m_keyValueLobbyParams.resize(iNumKeyValues * 2);
		for (size_t i(0), j(0); i < (size_t)iNumKeyValues; i++, j += 2)
		{
			m_keyValueLobbyParams[j] = pKeyDataValues[i][0];
			m_keyValueLobbyParams[j + 1] = pKeyDataValues[i][1];
		}
	}

	Friends()->SetRichPresence("status", "Creating a lobby");

	m_lobby.eState = LOBBY_CREATING;
}

void CGalaxyClientP2P::LeaveLobby()
{
	if (m_lobby.ullLobbyId == 0)
	{
		Reset(); // reset everything else
		return;
	}

	Matchmaking()->LeaveLobby(m_lobby.ullLobbyId);
	g_pLog->Write("Network:: Left lobby !\n");

	// if i left the lobby, and i had a a peer to talk with, close that connection
	if (m_peerID.IsValid())
	{
		g_pLog->Write("Network:: Closing session with user: '%llu'\n", m_peerID.GetRealID());
		
		m_peerID = GalaxyID::UNASSIGNED_VALUE;
	}

	Reset(); // // reset everything else
}

void CGalaxyClientP2P::JoinLobby(const LobbyID& ullLobbyId)
{
	if (!IsUserConnected())
	{
		AddEvent(INetwork::EV_CONNECTION_LOST, 1); // re-send event that connection is lost
		return;
	}

	if (!ullLobbyId.IsValid())
	{
		g_pLog->Write("Network:: JoinLobby - Invalid lobby id: %llu\n", ullLobbyId);
		return;
	}

	Matchmaking()->JoinLobby(ullLobbyId);

	m_lobby.eState = LOBBY_JOINING;
}

void CGalaxyClientP2P::UpdateLobbyMembers()
{
	if (m_lobby.ullLobbyId == 0)
		return;

	// list of users in lobby
	// iterate all the users in the lobby and show their details

	int iLobbyMembers = Matchmaking()->GetNumLobbyMembers(m_lobby.ullLobbyId);
	if (iLobbyMembers > INetwork::MAX_PLAYERS_PER_GAME)
	{
		iLobbyMembers = INetwork::MAX_PLAYERS_PER_GAME;
		g_pLog->Write("Network:: More then max player in the lobby! Capping at max! \n");
	}

	// first put the owner
	GalaxyID ownerID = Matchmaking()->GetLobbyOwner(m_lobby.ullLobbyId);

	m_lobby.iNumPlayers = iLobbyMembers;

	int currentArrID = 1;

	for (int i = 0; i < iLobbyMembers; i++)
	{
		GalaxyID lobbyMemberID = Matchmaking()->GetLobbyMemberByIndex(m_lobby.ullLobbyId, i);

		int arrID;
		if (lobbyMemberID == ownerID)
		{
			arrID = 0;
		}
		else
		{
			arrID = currentArrID;
			currentArrID++;
		}

		INetwork::LobbyUser& lobbyUser = m_lobby.players[arrID];


		// set user id 
		lobbyUser.m_netUserId = lobbyMemberID;

		// we get the details of a user from the SDK interface
		const char *pchName = Friends()->GetFriendPersonaName(lobbyMemberID);

		// we may not know the name of the other users in the lobby immediately; but we'll receive
		// a PersonaStateUpdate_t callback when they do, and we'll rebuild the list then
		if (pchName && *pchName)
		{
			// add this data as well, if ready
			strncpy(lobbyUser.m_rgchName, pchName, sizeof(lobbyUser.m_rgchName));
		}
	}
}

void CGalaxyClientP2P::OnPersonaDataChanged(GalaxyID userID, uint32_t personaStateChange)
{
	if (m_lobby.ullLobbyId == 0)
		return;

	(personaStateChange);

	bool userInLobby = false;
	for (int i = 0; i < m_lobby.iNumPlayers; i++)
	{
		if (m_lobby.players[i].m_netUserId == userID)
		{
			userInLobby = true;
			break;
		}
	}

	if (userInLobby)
		UpdateLobbyMembers();
}

void CGalaxyClientP2P::OnLobbyDataUpdateSuccess(const GalaxyID& lobbyID)
{
	// callbacks are broadcast to all listeners, so we'll get this for every lobby we're requesting
	if (m_lobby.ullLobbyId != lobbyID)
		return;

	g_pLog->Write("Network:: Got update data for my lobby !\n");

	UpdateLobbyMembers();
}

void CGalaxyClientP2P::OnLobbyDataUpdateFailure(const GalaxyID& lobbyID, ILobbyDataUpdateListener::FailureReason failureReason)
{
	if (m_lobby.ullLobbyId != lobbyID)
		return;

	g_pLog->Write("Network:: Failed getting data update for my lobby. Reason: %d !\n", failureReason);
}

void CGalaxyClientP2P::OnLobbyMemberStateChanged(const GalaxyID& lobbyID, const GalaxyID& memberID, LobbyMemberStateChange memberStateChange)
{
	if (m_lobby.ullLobbyId != lobbyID)
		return;

	UpdateLobbyMembers();

	switch (memberStateChange)
	{
	case LOBBY_MEMBER_STATE_CHANGED_LEFT:
	case LOBBY_MEMBER_STATE_CHANGED_DISCONNECTED:
	{
		if (memberID != User()->GetGalaxyID()) // just in case this gets signaled when we leave as well
		{
			AddEvent(INetwork::EV_LOBBY_PLAYER_LEFT);

			// check if lobby owner left, and if so, check if I am the new lobby owner, and mark myself as such in the data
			if (m_lobby.bIAmOwner == false)
			{
				GalaxyID lobbyOwnerID = Matchmaking()->GetLobbyOwner(m_lobby.ullLobbyId);
				if (lobbyOwnerID == User()->GetGalaxyID())
				{
					m_lobby.bIAmOwner = true;

					g_pLog->Write("Network:: I am now Lobby Owner !\n");
				}
			}

			// if my peer left, close p2p conn with him
			if (m_peerID.IsValid() && m_peerID == memberID)
			{
				g_pLog->Write("Network:: Closing session with user: '%llu'\n", m_peerID.GetRealID());
				m_peerID = GalaxyID::UNASSIGNED_VALUE;
			}

			// update internal steam status, so that ppl can join my lobby
			UpdateRichPresenceConnectionInfo();

			SetConnectedState(k_EClientNotConnected);
		}
	}
	break;
	case LOBBY_MEMBER_STATE_CHANGED_KICKED:
	case LOBBY_MEMBER_STATE_CHANGED_BANNED:
		break;

	case LOBBY_MEMBER_STATE_CHANGED_ENTERED:
		if (memberID != User()->GetGalaxyID()) // just in case this gets signaled when we join as well
		{
			AddEvent(INetwork::EV_LOBBY_PLAYER_JOINED);
			m_peerID = memberID;

			SetConnectedState(k_EClientConnected);
		}
		break;

	default:
		break;
	}
}


void CGalaxyClientP2P::RequestLobbyList(bool bOnlyNearbyLobbies, int iMaxResults, const int iNumFilterKeyValues /*= 0*/, const char* pFilterKeyDataValues[][2] /*= NULL*/)
{
	if (!IsUserConnected())
	{
		AddEvent(INetwork::EV_CONNECTION_LOST, 1); // re-send event that connection is lost
		return;
	}

	if (m_bRequestingLobbies)
		return;

	m_bRequestingLobbies = true;

	(bOnlyNearbyLobbies); //not applicable

	Matchmaking()->AddRequestLobbyListResultCountFilter(iMaxResults);

	for (int i(0); i < iNumFilterKeyValues; ++i)
	{
		// mb add more comparison flags somehow ?
		Matchmaking()->AddRequestLobbyListStringFilter(pFilterKeyDataValues[i][0], pFilterKeyDataValues[i][1], LobbyComparisonType::LOBBY_COMPARISON_TYPE_EQUAL);
	}

	//clear lobby list
	m_listLobbies.clear();
	// request all lobbies for this game
	Matchmaking()->RequestLobbyList();
}

int CGalaxyClientP2P::GetLobbyListEntriesCount()
{
	return m_listLobbies.size();
}

bool CGalaxyClientP2P::IsRequestingLobby()
{
	return m_bRequestingLobbies;
}

bool CGalaxyClientP2P::GetLobbyListEntry(int iLobbyIndex, uint64_t & out_ullLobbyId, char * out_szName)
{
	if ((iLobbyIndex < 0) || (iLobbyIndex >= m_listLobbies.size()))
		return false;

	LobbyBrowserItem& lobby = m_listLobbies[iLobbyIndex];
	out_ullLobbyId = lobby.lobbyID.ToUint64();
	sprintf(out_szName, "%s", lobby.szName);

	return true;
}

uint64_t CGalaxyClientP2P::GetLobbyIdByIndex(int iLobbyIndex)
{
	if (iLobbyIndex >= m_listLobbies.size())
		return 0;

	GalaxyID lobbyID = m_listLobbies[iLobbyIndex].lobbyID;
	return lobbyID.ToUint64();
}

const char* CGalaxyClientP2P::GetLobbyData(const uint64_t ullLobbyId, const char* pszKey)
{
	GalaxyID lobbyID(ullLobbyId);
	const char *pchLobbyData = Matchmaking()->GetLobbyData(lobbyID, pszKey);
	return pchLobbyData;
}

void CGalaxyClientP2P::ShowInviteToLobbyUI()
{
	char rgchConnectString[128];
	sprintf(rgchConnectString, "-connect-lobby-%llu", m_lobby.ullLobbyId);

	Friends()->ShowOverlayInviteDialog(rgchConnectString);
}

void CGalaxyClientP2P::OnLobbyList(uint32_t lobbyCount, LobbyListResult result)
{
	m_listLobbies.clear();
	m_bRequestingLobbies = false;

	if (result == LOBBY_LIST_RESULT_ERROR || result == LOBBY_LIST_RESULT_CONNECTION_FAILURE)
	{
		// we had a I/O failure - we probably timed out talking to the back-end servers
		// doesn't matter in this case, we can just act if no lobbies were received
		lobbyCount = 0;
	}

	m_listLobbies.resize(lobbyCount);

	if (lobbyCount == 0)
	{
		AddEvent(EV_LOBBY_LIST_RECEIVED, 0);
		g_pLog->Write("Network:: EV_LOBBY_LIST_RECEIVED - 0 lobbies\n");

		return;
	}

	for (uint32_t iLobby = 0; iLobby < lobbyCount; iLobby++)
	{
		GalaxyID lobbyID = Matchmaking()->GetLobbyByIndex(iLobby);

		// add the lobby to the list
		LobbyBrowserItem& lobby = m_listLobbies[iLobby];
		lobby.lobbyID = lobbyID;

		// pull the name from the lobby metadata
		const char *pchLobbyName = Matchmaking()->GetLobbyData(lobbyID, "name");
		if (pchLobbyName && pchLobbyName[0])
		{
			// set the lobby name
			sprintf(lobby.szName, "%s", pchLobbyName);

			// assume we have full info about the lobby at this point
			lobby.bDataAvailable = true;
		}
		else
		{
			// we don't have info about the lobby yet, request it
			Matchmaking()->RequestLobbyData(lobbyID);

			// results will be returned via LobbyDataUpdate_t callback
			sprintf(lobby.szName, "Lobby %llu", lobbyID.ToUint64());

			// data not ready
			lobby.bDataAvailable = false;
		}
	}

	CheckMatchmakingLobbiesData();
}

void CGalaxyClientP2P::CheckMatchmakingLobbiesData()
{
	if (m_listLobbies.size() == 0)
		return;

	// check if all lobbies have data available
	bool bGotAllData = true;

	std::vector< LobbyBrowserItem >::iterator iter;
	for (iter = m_listLobbies.begin(); iter != m_listLobbies.end(); ++iter)
		bGotAllData = bGotAllData && iter->bDataAvailable;

	if (bGotAllData)
	{
		// send event
		AddEvent(EV_LOBBY_LIST_RECEIVED, m_listLobbies.size());
		g_pLog->Write("Network:: EV_LOBBY_LIST_RECEIVED - %d lobbies\n", m_listLobbies.size());

		//no point in deleting the data either
		//m_listLobbies.clear(); // no point to keep this anymore
	}
}

void CGalaxyClientP2P::OnLobbyDataRetrieveSuccess(const GalaxyID& lobbyID)
{
	g_pLog->Write("Network:: Got updated lobbies data !\n");

	// find the lobby in our local list 
	std::vector< LobbyBrowserItem >::iterator iter;
	for (iter = m_listLobbies.begin(); iter != m_listLobbies.end(); ++iter)
	{
		// update the name of the lobby
		if (iter->lobbyID == lobbyID)
		{
			iter->bDataAvailable = true;

			// extract the display name from the lobby metadata
			const char *pchLobbyName = Matchmaking()->GetLobbyData(lobbyID, "name");
			if (pchLobbyName[0])
			{
				sprintf(iter->szName, "%s", pchLobbyName);
			}
			break;
		}
	}

	CheckMatchmakingLobbiesData();
}

void CGalaxyClientP2P::OnLobbyDataRetrieveFailure(const GalaxyID& lobbyID, ILobbyDataRetrieveListener::FailureReason failureReason)
{
	g_pLog->Write("Network:: Failed to retrieve lobby data ! Reason: %d \n", failureReason);

	// find the lobby in our local list 
	std::vector< LobbyBrowserItem >::iterator iter;
	for (iter = m_listLobbies.begin(); iter != m_listLobbies.end(); ++iter)
	{
		// update the name of the lobby
		if (iter->lobbyID == lobbyID)
		{
			m_listLobbies.erase(iter);
			break;
		}
	}

	CheckMatchmakingLobbiesData();
}

void CGalaxyClientP2P::OnLobbyCreated(const GalaxyID& lobbyID, LobbyCreateResult result)
{
	if (m_lobby.eState != INetwork::LOBBY_CREATING)
	{
		// if user quit quickly, just leave the newly create lobby
		Matchmaking()->LeaveLobby(lobbyID);
		g_pLog->Write("Network:: Lobby created, but immediately left after! User canceled! \n");
		return;
	}

	// record which lobby we're in
	if (result == LOBBY_CREATE_RESULT_SUCCESS)
	{
		// success
		m_lobby.ullLobbyId = lobbyID;
		m_lobby.iNumPlayers = 1;
		m_lobby.bIAmOwner = true;

		// set the name of the lobby if it's ours
		char rgchLobbyName[256];
		sprintf(rgchLobbyName, "%s's lobby", Friends()->GetPersonaName());
		Matchmaking()->SetLobbyData(lobbyID, "name", rgchLobbyName);

		// mark that we're in the lobby
		UpdateRichPresenceConnectionInfo();

		g_pLog->Write("Network:: Creating lobby '%s'\n", rgchLobbyName);
		AddEvent(EV_LOBBY_CREATE_SUCCESS);
		m_lobby.eState = LOBBY_IN_LOBBY;

		Friends()->SetRichPresence("status", "In lobby");

		// also set meta data on lobby
		if (m_keyValueLobbyParams.size())
		{
			for (size_t i(0); i < m_keyValueLobbyParams.size(); i += 2)
			{
				Matchmaking()->SetLobbyData(lobbyID, m_keyValueLobbyParams[i].c_str(), m_keyValueLobbyParams[i + 1].c_str());
				if (GetError())
				{
					g_pLog->Write("Network:: Unable to set key/value on lobby '%s': %s::%s\n",
						rgchLobbyName, m_keyValueLobbyParams[i].c_str(), m_keyValueLobbyParams[i + 1].c_str());
				}
			}
		}
	}
	else
	{
		// k_EResultNoConnection - your Steam client doesn't have a connection to the back-end
		// k_EResultTimeout - you the message to the Steam servers, but it didn't respond
		// k_EResultFail - the server responded, but with an unknown internal error
		// k_EResultAccessDenied - your game isn't set to allow lobbies, or your client does haven't rights to play the game
		// k_EResultLimitExceeded - your game client has created too many lobbies

		const char* pszReason = NULL;
		switch (result)
		{
		case LOBBY_CREATE_RESULT_ERROR:
			pszReason = "Galaxy error when creating a lobby";
			break;

		case LOBBY_CREATE_RESULT_CONNECTION_FAILURE:
			pszReason = "Galaxy connection failure when creating a lobby";
			break;
		default:
			pszReason = "UnknownInternalError: Galaxy";
			break;
		}

		AddEvent(EV_LOBBY_CREATE_FAILED);
		m_lobby.eState = LOBBY_UNKNOWN;

		char cBuffer[128];
		sprintf(cBuffer, "Failed to create lobby (%s).", pszReason);
		g_pLog->Write("Network:: Creating lobby failed '%s'\n", cBuffer);

		// failed, show error
		if (result == LOBBY_CREATE_RESULT_CONNECTION_FAILURE)
		{
			SetConnectionFailureState(cBuffer);

			Reset();
		}
	}
}

void CGalaxyClientP2P::OnLobbyEntered(const GalaxyID& lobbyID, LobbyEnterResult result)
{
	if (m_lobby.eState != LOBBY_JOINING)
		return;

	if (result != LOBBY_ENTER_RESULT_SUCCESS)
	{
		char cBuffer[128];
		sprintf(cBuffer, "Failed to enter lobby (reason: %d).", result);
		g_pLog->Write("Network:: '%s'\n", cBuffer);

		AddEvent(EV_LOBBY_JOIN_FAILED);
		m_lobby.eState = LOBBY_UNKNOWN;

		return;
	}

	// move forward the state
	m_lobby.ullLobbyId = lobbyID;
	m_lobby.bIAmOwner = false;
	m_lobby.eState = LOBBY_IN_LOBBY;

	// update lobby member data
	UpdateLobbyMembers();

	// get steam id for the other player; works only for 2 players
	for (int i(0); i < MAX_PLAYERS_PER_GAME; ++i)
	{
		if (m_lobby.players[i].m_netUserId != User()->GetGalaxyID())
		{
			m_peerID = m_lobby.players[i].m_netUserId;
			break;
		}
	}

	SetConnectedState(k_EClientConnected);

	Friends()->SetRichPresence("status", "In lobby");

	AddEvent(EV_LOBBY_JOIN_SUCCESS);

	// dbg
	const char* pszLobbyName = Matchmaking()->GetLobbyData(m_lobby.ullLobbyId, "name");
	g_pLog->Write("Network:: Joined lobby '%s'\n", pszLobbyName);
}


void CGalaxyClientP2P::SetConnectionFailureState(const char *pchErrorText)
{
	AddEvent(INetwork::EV_CONNECTION_LOST, 1);

	g_pLog->Write("Network:: ConnectionFailure:: Reason: %s\n", pchErrorText);
}

void CGalaxyClientP2P::OnGameJoinRequested(GalaxyID userID, const char* connectionString)
{
	g_pLog->Write("Network:: OnGameJoinRequested:: got string: '%s' from user %llu\n", connectionString, userID.ToUint64());

	// check if already in a lobby, and if so, leave it
	if (m_lobby.ullLobbyId != 0)
	{
		LeaveLobby();
	}

	// join the new lobby
	uint64_t ullLobbyID = 0;
	sscanf(connectionString, "-connect-lobby-%llu", &ullLobbyID);
	AddEvent(EV_LOBBY_INVITED, ullLobbyID);
}

void CGalaxyClientP2P::OnConnectionStateChange(GogServicesConnectionState connectionState)
{
	switch (connectionState)
	{
	case GOG_SERVICES_CONNECTION_STATE_UNDEFINED:
		g_pLog->Write("Network:: Connection state change - undefined \n");
		break;
	case GOG_SERVICES_CONNECTION_STATE_CONNECTED:
		g_pLog->Write("Network:: Connection state change - connected \n");
		break;
	case GOG_SERVICES_CONNECTION_STATE_DISCONNECTED:
		g_pLog->Write("Network:: Connection state change - disconnected \n");
		LeaveLobby();
		SetConnectionFailureState("GOG connection lost.");
		break;
	case GOG_SERVICES_CONNECTION_STATE_AUTH_LOST:
		g_pLog->Write("Network:: Connection state change - auth lost \n");
		LeaveLobby();
		SetConnectionFailureState("GOG Auth lost.");
		break;
	}
}

void CGalaxyClientP2P::OnOperationalStateChanged(uint32_t operationalState)
{
	g_pLog->Write("Network:: Operational state changed: %d \n", operationalState);

	if ((operationalState & (OPERATIONAL_STATE_SIGNED_IN | OPERATIONAL_STATE_LOGGED_ON)) != (OPERATIONAL_STATE_SIGNED_IN | OPERATIONAL_STATE_LOGGED_ON))
	{
		LeaveLobby();
		SetConnectionFailureState("User is not logged onto GOG servers anymore.");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates what we show to friends about what we're doing and how to connect
//-----------------------------------------------------------------------------
void CGalaxyClientP2P::UpdateRichPresenceConnectionInfo()
{
	// connect string that will come back to us on the command line	when a friend tries to join our game
	char rgchConnectString[128];
	rgchConnectString[0] = 0;

	if (m_lobby.ullLobbyId != 0 && m_lobby.iNumPlayers == 1)
	{
		// lobby connection method
		sprintf(rgchConnectString, "-connect-lobby-%llu", m_lobby.ullLobbyId);
	}

	if (IsUserConnected())
	{
		Friends()->SetRichPresence("connect", rgchConnectString);
	}
}

bool CGalaxyClientP2P::IsUserConnected()
{
	if (UTApp().m_Settings.galaxyFullyLoaded && User() && User()->SignedIn() && User()->IsLoggedOn())
		return true;

	return false;
}

#endif // ENABLE_GALAXY