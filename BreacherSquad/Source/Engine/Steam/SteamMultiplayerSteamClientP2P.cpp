#include "dxstdafx.h"

#ifdef ENABLE_STEAM

#if defined( _WIN32 )
	#include <windows.h>
#endif

#include "SteamMultiplayerSteamClientP2P.h"

#include <time.h> // for random num generator; all clients eventually need to have same seed ?

//****************************************************************************************

CSteamClientP2P g_netSteamInstance;
INetwork* g_pNetwork = &g_netSteamInstance;

//****************************************************************************************

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CSteamClientP2P::CSteamClientP2P() :
		m_CallbackPersonaStateChange( this, &CSteamClientP2P::OnPersonaStateChange ),
		m_CallbackLobbyDataUpdate( this, &CSteamClientP2P::OnLobbyDataUpdate ),
		m_CallbackChatDataUpdate( this, &CSteamClientP2P::OnLobbyChatUpdate ),
		m_CallbackLobbiesDataUpdated( this, &CSteamClientP2P::OnLobbiesDataUpdatedCallback ),
		m_CallbackP2PSessionRequest( this, &CSteamClientP2P::OnP2PSessionRequest ),
		m_CallbackP2PSessionConnectFail( this, &CSteamClientP2P::OnP2PSessionConnectFail )
{
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CSteamClientP2P::~CSteamClientP2P()
{
	// prob not needed anymore here
	// todo: remove later if ok
	Reset();
}

//****************************************************************************************
// INetwork inheritance
//****************************************************************************************

bool CSteamClientP2P::Start()
{
	if (!NetworkBase::Start())
		return false;

	Init();
	
	return true;
}

void CSteamClientP2P::Update()
{
	NetworkBase::Update();

	// Get any new data off the network for voice chat
//	ReceiveNetworkData();

//	m_pVoiceChat->RunFrame();

	// Update state for everything
	switch ( m_eGameState )
	{
		case k_EClientCreatingLobby:
		case k_EClientInLobby:
		case k_EClientNotReady:
		case k_EClientReady:
		break;

		case k_EClientJoiningLobby:
		{
			// Check if we've waited too long and should time out the connection
			if ( OS_GetTimeMS() - m_ulStateTransitionTime > MILLISECONDS_CONNECTION_TIMEOUT )
			{
				g_pLog->Write( "Network:: Timed out connecting to lobby.\n" );
				
				// mb also change internal state etc ? like check reason for fail, and change internal state ?
			}
		}
		break;

		case k_EClientConnectionFailure:
		{
			// do something here ? or rely on steam callback to let us know when we have steam servers connection back ?
		}
		break;

		default:
		g_pLog->Write( "Network:: Unhandled game state in CSteamClientP2P::RunFrame\n" );
	}
}

bool CSteamClientP2P::Send(const void* pData, unsigned int dataSize)
{
	if( !m_steamIDPeer.IsValid() )
		return false;

	if( dataSize > INetwork::MAX_UNRELIABLE_PACKET_SIZE )
		return false;

	if (SimulateLatencySend(pData, dataSize))
		return true;

	if ( !SteamNetworking()->SendP2PPacket( m_steamIDPeer, pData, dataSize, k_EP2PSendUnreliableNoDelay, k_EClientConnectionChannel_Game ) )
	{
		g_pLog->Write( "Network:: Failed sending data to client!\n" );
		return false;
	}
	return true;
}

bool CSteamClientP2P::SendReliable(const void* pData, unsigned int dataSize)
{
	if( !m_steamIDPeer.IsValid() )
		return false;

	if( dataSize > INetwork::MAX_RELIABLE_PACKET_SIZE )
		return false;

	if ( !SteamNetworking()->SendP2PPacket( m_steamIDPeer, pData, dataSize, k_EP2PSendReliable, k_EClientConnectionChannel_Game ) )
	{
		g_pLog->Write( "Network:: Failed sending data to client!\n" );
		return false;
	}
	return true;
}

void CSteamClientP2P::Debug_DumpConnectionInfo( const CSteamID& connectedSteamID )
{
	if( m_bShowConnectionInfoOnce )
		m_bShowConnectionInfoOnce = false;
	else
		return;

	P2PSessionState_t p2pSessionState;
	bool bState = SteamNetworking()->GetP2PSessionState( connectedSteamID, &p2pSessionState );
	if( bState )
	{
		g_pLog->Write( "Network:: P2P Connection State: relay(%s), connecting(%s), connActive(%s), error?(%s)\n", 
			p2pSessionState.m_bUsingRelay ? "yes" : "no",
			p2pSessionState.m_bConnecting ? "yes" : "no",
			p2pSessionState.m_bConnectionActive ? "yes" : "no",
			p2pSessionState.m_eP2PSessionError == 0 ? "NOERR" : "ERROR" );
	}
	else
	{
		g_pLog->Write( "Network:: P2P Connection State: no connection yet, even if we accepted !\n" );
	}
}

bool CSteamClientP2P::Recv(void* pData, unsigned int uiDataAllocSize, unsigned int* pActualRecvSize)
{
	uint32 uiAvailablePacketSize = 0;
	uint32 uiReadPacketActualSize = 0;
	*pActualRecvSize = 0;

	// see if there is any data waiting on the socket
	if ( SteamNetworking()->IsP2PPacketAvailable( &uiAvailablePacketSize, k_EClientConnectionChannel_Game ) )
	{
		assert( uiAvailablePacketSize <= uiDataAllocSize );
		if ( uiAvailablePacketSize > uiDataAllocSize )
		{
			g_pLog->Write( "Network:: Received packet size is larger than expected ! \n" );
		}
	
		CSteamID steamIDRemote;
		if ( !SteamNetworking()->ReadP2PPacket( pData, uiDataAllocSize, &uiReadPacketActualSize, &steamIDRemote, k_EClientConnectionChannel_Game ) )
			return false;

		m_ulLastNetworkDataReceivedTime = OS_GetTimeMS();
		assert( uiReadPacketActualSize == uiAvailablePacketSize );
		if( uiReadPacketActualSize != uiAvailablePacketSize )
		{
			g_pLog->Write( "Network:: Got packet size '%d', expecting to have '%d' ! \n",
				uiReadPacketActualSize, uiDataAllocSize );
		}

		if( uiReadPacketActualSize > uiDataAllocSize )
		{
			g_pLog->Write( "Network:: Got packet size '%d', only have room for '%d' - truncating ! \n",
				uiReadPacketActualSize, uiDataAllocSize );
		}
	
		*pActualRecvSize = uiReadPacketActualSize;

		// dump connection state; debug info
		Debug_DumpConnectionInfo( steamIDRemote );
	}

	SimulateLatencyRecv(pData, &uiDataAllocSize, pActualRecvSize);

	return *pActualRecvSize != 0;
}

//-----------------------------------------------------------------------------
// Purpose: create lobby; will transition to 'k_EClientInLobby'
//-----------------------------------------------------------------------------
void CSteamClientP2P::CreateLobby( bool bFriendsOnly, const int iNumKeyValues, const char* pKeyDataValues[][2] )
{
	if( m_eGameState == k_EClientConnectionFailure || 
		m_eGameState == k_EClientNotReady )
	{
		AddEvent( INetwork::EV_CONNECTION_LOST, (uint64)m_eConnectionLostState ); // re-send event that connection is lost
		return;
	}

	// start creating the lobby
	if ( !m_SteamCallResultLobbyCreated.IsActive() )
	{
		// ask steam to create a lobby
		SteamAPICall_t hSteamAPICall = SteamMatchmaking()->CreateLobby( bFriendsOnly ? k_ELobbyTypeFriendsOnly : k_ELobbyTypePublic, MAX_PLAYERS_PER_GAME );

		// set the function to call when this completes
		m_SteamCallResultLobbyCreated.Set( hSteamAPICall, this, &CSteamClientP2P::OnLobbyCreated );

		// delete old params if present
		m_keyValueLobbyParams.clear();

		// copy params
		if( iNumKeyValues != 0 )
		{
			m_keyValueLobbyParams.resize( iNumKeyValues * 2 );
			for( size_t i(0), j(0); i < (size_t)iNumKeyValues; i++, j+=2 )
			{
				m_keyValueLobbyParams[ j ] = pKeyDataValues[ i ][ 0 ];
				m_keyValueLobbyParams[ j +1 ] = pKeyDataValues[ i ][ 1 ];
			}
		}
	}

	SteamFriends()->SetRichPresence( "status", "Creating a lobby" );

	// set correct state
	SetGameState( k_EClientCreatingLobby );

	m_lobby.eState = LOBBY_CREATING;
}

//-----------------------------------------------------------------------------
// Purpose: leave lobby
//-----------------------------------------------------------------------------
void CSteamClientP2P::LeaveLobby()
{
	if( m_lobby.ullLobbyId == 0 )
	{
		Reset(); // reset everything else
		return;
	}

	// if no SteamMatchmaking() then it means this gets called in a static-memory-destruct, so nothing allocd on heap will be valid ( like steam stuff, log .. )
	if( SteamMatchmaking() )
	{
		SteamMatchmaking()->LeaveLobby( CSteamID( m_lobby.ullLobbyId ) );
		g_pLog->Write( "Network:: Left lobby !\n" );
	}

	// if i left the lobby, and i had a a peer to talk with, close that connection
	if( m_steamIDPeer.IsValid() )
	{
		// if no SteamNetworking() then it means this gets called in a static-memory-destruct, so nothing allocd on heap will be valid ( like steam stuff, log .. )
		if( SteamNetworking() )
		{
			SteamNetworking()->CloseP2PSessionWithUser( m_steamIDPeer );
			g_pLog->Write( "Network:: Closing session with user: '%d'\n", m_steamIDPeer.GetAccountID() );
		}

		m_steamIDPeer.Clear();
	}

	Reset(); // // reset everything else
}

//-----------------------------------------------------------------------------
// Purpose: join lobby
//-----------------------------------------------------------------------------
void CSteamClientP2P::JoinLobby( const LobbyID& ullLobbyId )
{
	if( m_eGameState == k_EClientConnectionFailure ||
		m_eGameState == k_EClientNotReady )
	{
		AddEvent( INetwork::EV_CONNECTION_LOST, (uint64)m_eConnectionLostState ); // re-send event that connection is lost
		return;
	}

	// normal join code
	CSteamID steamIDLobby( ullLobbyId );

	if( !steamIDLobby.IsValid() )
	{
		g_pLog->Write( "Network:: JoinLobby - Invalid lobby steam id: %llu\n", ullLobbyId );
		return;
	}

	SteamAPICall_t hSteamAPICall = SteamMatchmaking()->JoinLobby( steamIDLobby );

	// set the function to call when this API completes
	m_SteamCallResultLobbyEntered.Set( hSteamAPICall, this, &CSteamClientP2P::OnLobbyEntered );

	// set correct state
	SetGameState( k_EClientJoiningLobby );
	m_lobby.eState = LOBBY_JOINING;
}

//-----------------------------------------------------------------------------
// Purpose: Gets the members of the lobby
//-----------------------------------------------------------------------------
void CSteamClientP2P::GetMembers()
{
	if ( m_lobby.ullLobbyId == 0 )
		return;

	// list of users in lobby
	// iterate all the users in the lobby and show their details

	CSteamID steamIDLobby( m_lobby.ullLobbyId );

	int iLobbyMembers = SteamMatchmaking()->GetNumLobbyMembers( steamIDLobby );
	if( iLobbyMembers > INetwork::MAX_PLAYERS_PER_GAME )
	{
		iLobbyMembers = INetwork::MAX_PLAYERS_PER_GAME;
		g_pLog->Write( "Network:: More then max player in the lobby! Capping at max! \n" );
	}

	m_lobby.iNumPlayers = iLobbyMembers;
	for ( int i = 0; i < iLobbyMembers; i++ )
	{
		INetwork::LobbyUser& lobbyUser = m_lobby.players[ i ];

		CSteamID steamIDLobbyMember = SteamMatchmaking()->GetLobbyMemberByIndex( steamIDLobby, i ) ;
		
		// set steam user id 
		lobbyUser.m_netUserId = steamIDLobbyMember.ConvertToUint64();

		// we get the details of a user from the ISteamFriends interface
		const char *pchName = SteamFriends()->GetFriendPersonaName( steamIDLobbyMember );

		// we may not know the name of the other users in the lobby immediately; but we'll receive
		// a PersonaStateUpdate_t callback when they do, and we'll rebuild the list then
		if ( pchName && *pchName )
		{
			// add this data as well, if ready
			strncpy( lobbyUser.m_rgchName, pchName, sizeof( lobbyUser.m_rgchName ) );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handles a user in the lobby changing their name or details
//			( note: joining and leaving is handled below by CSteamClientP2P::OnLobbyChatUpdate() )
//-----------------------------------------------------------------------------
void CSteamClientP2P::OnPersonaStateChange( PersonaStateChange_t *pCallback )
{
	// callbacks are broadcast to all listeners, so we'll get this for every friend who changes state
	// so make sure the user is in the lobby before acting
	if ( !SteamFriends()->IsUserInSource( pCallback->m_ulSteamID, CSteamID( m_lobby.ullLobbyId ) ) )
		return;

	// rebuild members list
	GetMembers();
}


//-----------------------------------------------------------------------------
// Purpose: Handles lobby / user data changing ( if lobby - meta data; if user - name )
//          Only called when RequestLobbyData() was previously called
//-----------------------------------------------------------------------------
void CSteamClientP2P::OnLobbyDataUpdate( LobbyDataUpdate_t *pCallback )
{
	// callbacks are broadcast to all listeners, so we'll get this for every lobby we're requesting
	if ( m_lobby.ullLobbyId != pCallback->m_ulSteamIDLobby )
		return;

	g_pLog->Write( "Network:: Got update data for my lobby !\n" );

	// get lobby name
//	const char* pszLobbyName = SteamMatchmaking()->GetLobbyData( CSteamID( m_lobby.ullLobbyId ), "name" );
//	if( pszLobbyName )
//		strncpy( m_lobby.szName, pszLobbyName, Math::Min( strlen( pszLobbyName ), sizeof( m_lobby.szName ) ) );

	// rebuild the menu - don't do it here
	GetMembers();
}


//-----------------------------------------------------------------------------
// Purpose: Handles users in the lobby joining or leaving
//-----------------------------------------------------------------------------
void CSteamClientP2P::OnLobbyChatUpdate( LobbyChatUpdate_t *pCallback )
{
	// callbacks are broadcast to all listeners, so we'll get this for every lobby we're requesting
	if ( m_lobby.ullLobbyId != pCallback->m_ulSteamIDLobby )
		return;

	// rebuild the lobby users list
	GetMembers();

	switch( pCallback->m_rgfChatMemberStateChange )
	{
		case k_EChatMemberStateChangeLeft:
		case k_EChatMemberStateChangeDisconnected:
		{
			if( pCallback->m_ulSteamIDMakingChange != GetLocalSteamID().ConvertToUint64() ) // just in case this gets signaled when we leave as well
			{
				AddEvent( INetwork::EV_LOBBY_PLAYER_LEFT );

				// check if lobby owner left, and if so, check if I am the new lobby owner, and mark myself as such in the data
				if( m_lobby.bIAmOwner == false )
				{
					CSteamID lobbySteamID( m_lobby.ullLobbyId );
					CSteamID lobbyOwnerSteamID = SteamMatchmaking()->GetLobbyOwner( lobbySteamID );
					if( lobbyOwnerSteamID == GetLocalSteamID() )
					{
						m_lobby.bIAmOwner = true;

						g_pLog->Write( "Network:: I am now Lobby Owner !\n" );
					}
				}

				// if my peer left, close p2p conn with him
				if( m_steamIDPeer.IsValid() &&
					m_steamIDPeer.ConvertToUint64() == pCallback->m_ulSteamIDMakingChange )
				{
					SteamNetworking()->CloseP2PSessionWithUser( m_steamIDPeer );
					g_pLog->Write( "Network:: Closing session with user: '%d'\n", m_steamIDPeer.GetAccountID() );
					m_steamIDPeer.Clear();
				}

				// update internal steam status, so that ppl can join my lobby
				UpdateRichPresenceConnectionInfo();

				m_eConnectedStatus = k_EClientNotConnected;
			}
		}
		break;

		case k_EChatMemberStateChangeKicked:
		case k_EChatMemberStateChangeBanned:
			break;

		case k_EChatMemberStateChangeEntered:
		{
			if( pCallback->m_ulSteamIDMakingChange != GetLocalSteamID().ConvertToUint64() ) // just in case this gets signaled when we leave as well
			{
				AddEvent( INetwork::EV_LOBBY_PLAYER_JOINED );
				m_steamIDPeer = pCallback->m_ulSteamIDMakingChange;
				SteamFriends()->SetPlayedWith( m_steamIDPeer );
				m_eConnectedStatus = k_EClientConnected;
			}

		} break;

		default : break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Starts rebuilding the lobby list
//			Will take a while, show searching .. msg
//-----------------------------------------------------------------------------
void CSteamClientP2P::RequestLobbyList( bool bOnlyNearbyLobbies, int iMaxResults, const int iNumFilterKeyValues, const char* pFilterKeyDataValues[][2] )
{
	if( m_eGameState == k_EClientConnectionFailure ||
		m_eGameState == k_EClientNotReady )
	{
		AddEvent( INetwork::EV_CONNECTION_LOST, (uint64)m_eConnectionLostState ); // re-send event that connection is lost
		return;
	}

	if ( !m_bRequestingLobbies )
	{
		m_bRequestingLobbies = true;

		// if/else because flag might remain set by a previos call to RequestLobbyList()
		if( !bOnlyNearbyLobbies )
		{
			// // for games that don't have many latency requirements, will return lobbies about half-way around the globe
			SteamMatchmaking()->AddRequestLobbyListDistanceFilter( ELobbyDistanceFilter::k_ELobbyDistanceFilterFar );
		}
		else
		{
			// only lobbies in the same region or near by regions
			SteamMatchmaking()->AddRequestLobbyListDistanceFilter( ELobbyDistanceFilter::k_ELobbyDistanceFilterDefault );
		}
		SteamMatchmaking()->AddRequestLobbyListResultCountFilter( iMaxResults );

		for( int i(0); i < iNumFilterKeyValues; ++i )
		{
			// mb add more comparison flags somehow ?
			SteamMatchmaking()->AddRequestLobbyListStringFilter( pFilterKeyDataValues[ i ][ 0 ], pFilterKeyDataValues[ i ][ 1 ], ELobbyComparison::k_ELobbyComparisonEqual );
		}

		//clear lobby list
		m_listLobbies.clear();
		// request all lobbies for this game
		SteamAPICall_t hSteamAPICall = SteamMatchmaking()->RequestLobbyList();

		// set the function to call when this API call has completed
		m_SteamCallResultLobbyMatchList.Set( hSteamAPICall, this, &CSteamClientP2P::OnLobbyMatchListCallback );
	}
}

int CSteamClientP2P::GetLobbyListEntriesCount()
{
	return m_listLobbies.size();
}

bool CSteamClientP2P::GetLobbyListEntry(int iLobbyIndex, uint64_t & out_ullLobbyId, char * out_szName)
{
	if ((iLobbyIndex < 0) || (iLobbyIndex >= m_listLobbies.size()))
		return false;

	LobbyBrowserItem& lobby = m_listLobbies[iLobbyIndex];
	out_ullLobbyId = lobby.ullLobbyId;
	sprintf(out_szName, "%s", lobby.szName);

	return true;
}

bool CSteamClientP2P::IsRequestingLobby()
{
	return m_bRequestingLobbies;
}

uint64_t CSteamClientP2P::GetLobbyIdByIndex( int iLobbyIndex )
{
	CSteamID steamIDLobby = SteamMatchmaking()->GetLobbyByIndex( iLobbyIndex );
	return steamIDLobby.ConvertToUint64();
}

const char*	CSteamClientP2P::GetLobbyData( const uint64_t ullLobbyId, const char* pszKey )
{
	CSteamID lobbySteamID( ullLobbyId );
	const char *pchLobbyData = SteamMatchmaking()->GetLobbyData( lobbySteamID, pszKey );
	return pchLobbyData;
}

void CSteamClientP2P::ShowInviteToLobbyUI()
{
	SteamFriends()->ActivateGameOverlayInviteDialog( CSteamID( m_lobby.ullLobbyId ) );
}

//-----------------------------------------------------------------------------
// Purpose: Callback, on a list of lobbies being received from the Steam back-end
//-----------------------------------------------------------------------------
void CSteamClientP2P::OnLobbyMatchListCallback( LobbyMatchList_t *pCallback, bool bIOFailure )
{
	m_listLobbies.clear();
	m_bRequestingLobbies = false;

	if ( bIOFailure )
	{
		// we had a Steam I/O failure - we probably timed out talking to the Steam back-end servers
		// doesn't matter in this case, we can just act if no lobbies were received
	}

	// lobbies are returned in order of closeness to the user, so add them to the list in that order
	m_listLobbies.resize( pCallback->m_nLobbiesMatching );

	// assume we have full info about the lobby at this point
	if( pCallback->m_nLobbiesMatching == 0 )
	{
		AddEvent( EV_LOBBY_LIST_RECEIVED, 0 );
		g_pLog->Write( "Network:: EV_LOBBY_LIST_RECEIVED - 0 lobbies\n" );

		return;
	}

	for ( uint32 iLobby = 0; iLobby < pCallback->m_nLobbiesMatching; iLobby++ )
	{
		CSteamID steamIDLobby = SteamMatchmaking()->GetLobbyByIndex( iLobby );

		// add the lobby to the list
		LobbyBrowserItem& lobby = m_listLobbies[ iLobby ];
		lobby.ullLobbyId = steamIDLobby.ConvertToUint64();

		// pull the name from the lobby metadata
		const char *pchLobbyName = SteamMatchmaking()->GetLobbyData( steamIDLobby, "name" );
		if ( pchLobbyName && pchLobbyName[0] )
		{
			// set the lobby name
			sprintf( lobby.szName, "%s", pchLobbyName );

			// assume we have full info about the lobby at this point
			lobby.bDataAvailable = true;
		}
		else
		{
			// we don't have info about the lobby yet, request it
			SteamMatchmaking()->RequestLobbyData( steamIDLobby );

			// results will be returned via LobbyDataUpdate_t callback
			sprintf( lobby.szName, "Lobby %d", steamIDLobby.GetAccountID() );

			// data not ready
			lobby.bDataAvailable = false;
		}
	}

	CheckMatchmakingLobbiesData();
}

void CSteamClientP2P::CheckMatchmakingLobbiesData()
{
	if( m_listLobbies.size() == 0 )
		return;

	// check if all lobbies have data available
	bool bGotAllData = true;

	std::vector< LobbyBrowserItem >::iterator iter;
	for( iter = m_listLobbies.begin(); iter != m_listLobbies.end(); ++iter )
		bGotAllData = bGotAllData && iter->bDataAvailable;

	if( bGotAllData )
	{
		// send event
		AddEvent( EV_LOBBY_LIST_RECEIVED, m_listLobbies.size() );
		g_pLog->Write( "Network:: EV_LOBBY_LIST_RECEIVED - %d lobbies\n", m_listLobbies.size() );

		//no point in deleting the data either
		//m_listLobbies.clear(); // no point to keep this anymore
	}
}

//-----------------------------------------------------------------------------
// Purpose: Callback, on a list of lobbies being received from the Steam back-end
//-----------------------------------------------------------------------------
void CSteamClientP2P::OnLobbiesDataUpdatedCallback( LobbyDataUpdate_t *pCallback )
{
	g_pLog->Write( "Network:: Got updated lobbies data !\n" );

	// find the lobby in our local list 
	std::vector< LobbyBrowserItem >::iterator iter;
	for( iter = m_listLobbies.begin(); iter != m_listLobbies.end(); ++iter )
	{
		// update the name of the lobby
		if ( iter->ullLobbyId == pCallback->m_ulSteamIDLobby )
		{
			iter->bDataAvailable = true;

			// extract the display name from the lobby metadata
			const char *pchLobbyName = SteamMatchmaking()->GetLobbyData( CSteamID( iter->ullLobbyId ), "name" );
			if ( pchLobbyName[0] )
			{
				sprintf( iter->szName, "%s", pchLobbyName );
			}
			break;
		}
	}

	CheckMatchmakingLobbiesData();
}

//****************************************************************************************
// Purpose: another user has sent us a packet - do we accept?
//****************************************************************************************
void CSteamClientP2P::OnP2PSessionRequest( P2PSessionRequest_t *pP2PSessionRequest )
{
	// only accept p2p requests when in lobby ( either from create or from join )
	if( m_eConnectedStatus == EClientConnectionState::k_EClientNotConnected )
		return;

	// always accept packets
	// the packet itself will come through when you call SteamNetworking()->ReadP2PPacket()
	SteamNetworking()->AcceptP2PSessionWithUser( pP2PSessionRequest->m_steamIDRemote );

	g_pLog->Write( "Network:: Accepting session with user: '%d'\n", pP2PSessionRequest->m_steamIDRemote.GetAccountID() );
}

//-----------------------------------------------------------------------------
// Purpose: steam callback, triggered when our connection to another client fails
//-----------------------------------------------------------------------------
void CSteamClientP2P::OnP2PSessionConnectFail( P2PSessionConnectFail_t *pCallback )
{
	g_pLog->Write( "Network:: Failed to make P2P connection with '%d'\n", pCallback->m_steamIDRemote.GetAccountID() );

	// wait for 20 sec before setting this as a failed connection ( until the first connect attempt finishes )

/*
	// list of possible errors returned by SendP2PPacket() API
	// these will be posted in the P2PSessionConnectFail_t callback
	enum EP2PSessionError
	{
		k_EP2PSessionErrorNone = 0,
		k_EP2PSessionErrorNotRunningApp = 1,			// target is not running the same game
		k_EP2PSessionErrorNoRightsToApp = 2,			// local user doesn't own the app that is running
		k_EP2PSessionErrorDestinationNotLoggedIn = 3,	// target user isn't connected to Steam
		k_EP2PSessionErrorTimeout = 4,					// target isn't responding, perhaps not calling AcceptP2PSessionWithUser()
														// corporate firewalls can also block this (NAT traversal is not firewall traversal)
														// make sure that UDP ports 3478, 4379, and 4380 are open in an outbound direction
		k_EP2PSessionErrorMax = 5
	};
*/

	const char* pszExplicitErrorString = "Unknown Error";
	switch( pCallback->m_eP2PSessionError )
	{
		case k_EP2PSessionErrorNone:
			break;

		case k_EP2PSessionErrorNotRunningApp:
			pszExplicitErrorString = "k_EP2PSessionErrorNotRunningApp";
			break;

		case k_EP2PSessionErrorNoRightsToApp:
			pszExplicitErrorString = "k_EP2PSessionErrorNoRightsToApp";
			break;

		case k_EP2PSessionErrorDestinationNotLoggedIn:
			pszExplicitErrorString = "k_EP2PSessionErrorDestinationNotLoggedIn";
			break;

		case k_EP2PSessionErrorTimeout:
			pszExplicitErrorString = "k_EP2PSessionErrorTimeout";
			break;

		default : 
			break;
	}

	char szErrorBuffer[256];
	sprintf( szErrorBuffer, "Failed to make P2P connection ! Reason: %s", pszExplicitErrorString );
	SetConnectionFailureState( szErrorBuffer, INetwork::CONNECTION_LOST_P2P );
	
	LeaveLobby();
}

//****************************************************************************************

//-----------------------------------------------------------------------------
// Purpose: initialize our client for use
//-----------------------------------------------------------------------------
void CSteamClientP2P::Init()
{
	SetGameState( k_EClientNotReady );

	// On PC/OSX we always know the user has a SteamID and is logged in already,
	// as Steam enforces this before game launch.  On PS3 however the game must
	// initiate the logon and we need to check the state here and block the user
	// while Steam connects.
	if ( SteamUser()->BLoggedOn() )
	{
		m_steamIDLocalUser = SteamUser()->GetSteamID();
		SetGameState( k_EClientReady );

		SteamFriends()->SetRichPresence( "status", "Idle" );
	}

	m_ulStateTransitionTime = OS_GetTimeMS();
	m_ulLastNetworkDataReceivedTime = 0;
	m_rgchErrorText[0] = 0;

	m_bRequestingLobbies = false;
	m_bShowConnectionInfoOnce = true;

	// P2P voice chat 
//	m_pVoiceChat = new CVoiceChat();
}

//-----------------------------------------------------------------------------
// Purpose: deinitialize our client for use
//-----------------------------------------------------------------------------
void CSteamClientP2P::Reset()
{
	NetworkBase::Reset();

	m_listLobbies.clear();

	m_rgchErrorText[0] = 0;
	m_ulStateTransitionTime = OS_GetTimeMS();
	m_ulLastNetworkDataReceivedTime = 0;
	
	m_bRequestingLobbies = false;
	m_bShowConnectionInfoOnce = true;

	// delete old params if present
	m_keyValueLobbyParams.clear();

	SetGameState( k_EClientNotReady );
	if ( SteamUser() && SteamUser()->BLoggedOn() )
	{
		SteamFriends()->SetRichPresence( "status", "Idle" );
		SetGameState( k_EClientReady );
	}

	// reset +connect flag
	UpdateRichPresenceConnectionInfo();


/*	if ( m_pVoiceChat )
	{
		m_pVoiceChat->StopVoiceChat();
		delete m_pVoiceChat;
	} */
}

//-----------------------------------------------------------------------------
// Purpose: Receives incoming network data
//-----------------------------------------------------------------------------
void CSteamClientP2P::ReceiveNetworkData()
{
	char rgchRecvBuf[1200]; // magic value; min MRT; will prevent at least one malloc for the first msg
	char *pchRecvBuf = rgchRecvBuf;
	uint32 cubMsgSize;
	for (;;)
	{
		// reset the receive buffer
		if ( pchRecvBuf != rgchRecvBuf )
		{
			free( pchRecvBuf );
			pchRecvBuf = rgchRecvBuf;
		}

		// see if there is any data waiting on the socket
		if ( !SteamNetworking()->IsP2PPacketAvailable( &cubMsgSize ) ) // 0 channel
			break;

		// not enough space in default buffer
		// alloc custom size and try again
		if ( cubMsgSize > sizeof(rgchRecvBuf) )
		{
			pchRecvBuf = (char *)malloc( cubMsgSize );
		}

		CSteamID steamIDRemote;
		if ( !SteamNetworking()->ReadP2PPacket( pchRecvBuf, cubMsgSize, &cubMsgSize, &steamIDRemote ) ) // 0 channel
			break;

		m_ulLastNetworkDataReceivedTime = OS_GetTimeMS();

		// the message is from another player; cand be voice msg
//		EMessage eMsg = (EMessage)*(unsigned long*)pchRecvBuf;

		// process voice stuff
//		if ( m_pVoiceChat->HandleMessage( steamIDRemote, eMsg, pchRecvBuf ) )
//			continue;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Finishes up entering a lobby of our own creation
//-----------------------------------------------------------------------------
void CSteamClientP2P::OnLobbyCreated( LobbyCreated_t *pCallback, bool /* bIOFailure */ )
{
	if ( m_eGameState != k_EClientCreatingLobby )
	{
		// if user quit quickly, just leave the newly create lobby
		SteamMatchmaking()->LeaveLobby( CSteamID( pCallback->m_ulSteamIDLobby ) );
		g_pLog->Write( "Network:: Lobby created, but immediately left after! User canceled! \n" );
		return;
	}

	// record which lobby we're in
	if ( pCallback->m_eResult == k_EResultOK )
	{
		CSteamID lobbySteamID( pCallback->m_ulSteamIDLobby );

		// success
		m_lobby.ullLobbyId = pCallback->m_ulSteamIDLobby;
		m_lobby.iNumPlayers = 1;
		m_lobby.bIAmOwner = true;

		// set the name of the lobby if it's ours
		char rgchLobbyName[256];
		sprintf( rgchLobbyName, "%s's lobby", SteamFriends()->GetPersonaName() );
		SteamMatchmaking()->SetLobbyData( lobbySteamID, "name", rgchLobbyName );

		// mark that we're in the lobby
		SetGameState( k_EClientInLobby );
		UpdateRichPresenceConnectionInfo();

		g_pLog->Write( "Network:: Creating lobby '%s'\n", rgchLobbyName );
		AddEvent( EV_LOBBY_CREATE_SUCCESS );
		m_lobby.eState = LOBBY_IN_LOBBY;

		SteamFriends()->SetRichPresence( "status", "In lobby" );

		// also set meta data on lobby
		if( m_keyValueLobbyParams.size() )
		{
			for( size_t i(0); i < m_keyValueLobbyParams.size(); i += 2 )
			{
				bool bResult = SteamMatchmaking()->SetLobbyData( lobbySteamID, m_keyValueLobbyParams[ i ].c_str(), m_keyValueLobbyParams[ i +1 ].c_str() );
				if( !bResult )
				{
					g_pLog->Write( "Network:: Unable to set key/value on lobby '%s': %s::%s\n",
						rgchLobbyName, m_keyValueLobbyParams[ i ].c_str(), m_keyValueLobbyParams[ i +1 ].c_str() );
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
		switch( pCallback->m_eResult )
		{
			case k_EResultNoConnection:
				pszReason = "NoConnection: Steam client doesn't have a connection to the back-end";
				break;

			case k_EResultTimeout:
				pszReason = "Timeout: Steam servers didn't respond";
				break;

			case k_EResultAccessDenied:
				pszReason = "AccessDenied: Game not allowed to set-up lobbies; or client not allowed to play game";
				break;

			case k_EResultLimitExceeded:
				pszReason = "LimitExceeded: Too many lobbies created";
				break;

			case k_EResultFail:
			default:
				pszReason = "UnknownInternalError: Steam";
				break;
		}

		AddEvent( EV_LOBBY_CREATE_FAILED );
		m_lobby.eState = LOBBY_UNKNOWN;

		char cBuffer[128];
		sprintf( cBuffer, "Failed to create lobby (%s).", pszReason );
		g_pLog->Write( "Network:: Creating lobby failed '%s'\n", cBuffer );

		// failed, show error
		if( pCallback->m_eResult == k_EResultNoConnection )
		{
			SetConnectionFailureState( cBuffer, INetwork::CONNECTION_LOST_STEAM );

			Reset();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Finishes up entering a lobby
//-----------------------------------------------------------------------------
void CSteamClientP2P::OnLobbyEntered( LobbyEnter_t *pCallback, bool /* bIOFailure */ )
{
	if ( m_eGameState != k_EClientJoiningLobby )
		return;

	if ( pCallback->m_EChatRoomEnterResponse != k_EChatRoomEnterResponseSuccess )
	{
		char cBuffer[128];
		sprintf( cBuffer, "Failed to enter lobby (reason: %d).", pCallback->m_EChatRoomEnterResponse );
		g_pLog->Write( "Network:: '%s'\n", cBuffer );

		AddEvent( EV_LOBBY_JOIN_FAILED );
		m_lobby.eState = LOBBY_UNKNOWN;
		SetGameState( k_EClientReady ); // ignore; let the code try again

		return;
	}

	// move forward the state
	m_lobby.ullLobbyId = pCallback->m_ulSteamIDLobby;
	m_lobby.bIAmOwner = false;
	m_lobby.eState = LOBBY_IN_LOBBY;

	// update lobby member data
	GetMembers();

	// get steam id for the other player; works only for 2 players
	for( int i(0); i < MAX_PLAYERS_PER_GAME; ++i )
	{
		if( m_lobby.players[ i ].m_netUserId != GetLocalSteamID().ConvertToUint64() )
		{
			m_steamIDPeer = CSteamID( m_lobby.players[ i ].m_netUserId );
			SteamFriends()->SetPlayedWith( m_steamIDPeer );
			break;
		}
	}

	SetGameState( k_EClientInLobby );
	m_eConnectedStatus = k_EClientConnected;

	SteamFriends()->SetRichPresence( "status", "In lobby" );

	AddEvent( EV_LOBBY_JOIN_SUCCESS );

	// dbg
	const char* pszLobbyName = SteamMatchmaking()->GetLobbyData( CSteamID( m_lobby.ullLobbyId ), "name" );
	g_pLog->Write( "Network:: Joined lobby '%s'\n", pszLobbyName );
}

//-----------------------------------------------------------------------------
// Purpose: Joins a game from a lobby
//-----------------------------------------------------------------------------
void CSteamClientP2P::OnLobbyGameCreated( LobbyGameCreated_t* /* pCallback */ )
{
	if ( m_eGameState != k_EClientInLobby )
		return;

	// not used anymore
}

//-----------------------------------------------------------------------------
// Purpose: Steam is asking us to join a game, based on the user selecting
//			'join game' on a friend in their friends list 
//			the string comes from the "connect" field set in the friends' rich presence
//-----------------------------------------------------------------------------
void CSteamClientP2P::OnGameJoinRequested( GameRichPresenceJoinRequested_t* pCallback )
{
	g_pLog->Write( "Network:: OnGameJoinRequested:: got string: '%s'\n",
		pCallback->m_rgchConnect );

	// check if already in a lobby, and if so, leave it
	if( m_lobby.ullLobbyId != 0 )
	{
		LeaveLobby();
	}

	// join the new lobby
	uint64_t ullLobbyID = 0;
	sscanf( pCallback->m_rgchConnect, "+connect_lobby %llu", &ullLobbyID );
	AddEvent( EV_LOBBY_INVITED, ullLobbyID );
}

//-----------------------------------------------------------------------------
// Purpose: Another user is asking us to join a lobby, based on the user selecting
//			'invite' on a friend in the invite friends list
//			the string comes from the "connect" field set in the friends' rich presence
//-----------------------------------------------------------------------------
void CSteamClientP2P::OnGameLobbyJoinRequested( GameLobbyJoinRequested_t* pCallback )
{
	g_pLog->Write( "Network:: OnGameLobbyJoinRequested:: got lobby id: '%llu', from user '%llu'\n",
		pCallback->m_steamIDLobby.ConvertToUint64(), pCallback->m_steamIDFriend.ConvertToUint64() );

	// check if already in a lobby, and if so, leave it
	if( m_lobby.ullLobbyId != 0 )
	{
		LeaveLobby();
	}

	// join the new lobby
	AddEvent( EV_LOBBY_INVITED, pCallback->m_steamIDLobby.ConvertToUint64() );
}

//-----------------------------------------------------------------------------
// Purpose: Handles notification that we are now connected to Steam
//-----------------------------------------------------------------------------
void CSteamClientP2P::OnSteamServersConnected( SteamServersConnected_t* /* callback */ )
{
	if ( SteamUser()->BLoggedOn() )
	{
		SetGameState( k_EClientReady );
		g_pLog->Write( "Network:: SteamServersConnected_t, also logged in!\n" );
	}
	else
	{
		SetGameState( k_EClientNotReady );
		g_pLog->Write( "Network:: SteamServersConnected_t, but not logged on?\n" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handles notification that we are now connected to Steam
//-----------------------------------------------------------------------------
void CSteamClientP2P::OnSteamServersDisconnected( SteamServersDisconnected_t* /* callback */ )
{
	// try to clean-up
	LeaveLobby(); // only does a local Reset() since connection with steam was lost
	SetConnectionFailureState( "Steam connection lost.", INetwork::CONNECTION_LOST_STEAM );

	g_pLog->Write( "Network:: SteamServersDisconnected_t\n" );
}

//-----------------------------------------------------------------------------
// Purpose: Handles notification that we are failed to connected to Steam
//-----------------------------------------------------------------------------
void CSteamClientP2P::OnSteamServerConnectFailure( SteamServerConnectFailure_t *callback )
{
	g_pLog->Write( "Network:: SteamServerConnectFailure_t: %d\n", callback->m_eResult );
}

//-----------------------------------------------------------------------------
// Purpose: Handles notification of a steam ipc failure
// we may get multiple callbacks, one for each IPC operation we attempted
// since the actual failure, so protect ourselves from alerting more than once.
//-----------------------------------------------------------------------------
void CSteamClientP2P::OnIPCFailure( IPCFailure_t* /* failure */ )
{
	static bool bExiting = false;
	if ( !bExiting )
	{
		g_pLog->Write( "Network:: Steam IPC Failure, shutting down - callback system is in error state ( Steam fatal error )\n" );
		g_pLog->Write( "Network:: Disconnect from Steam, reset any stored Steam state and reconnect.\n" );

		// what to do in p2p network, when steam will close ?

		// try to clean-up
		LeaveLobby(); // only does a local Reset() since connection with steam was lost
		SetConnectionFailureState( "Steam shutdown.", INetwork::CONNECTION_LOST_STEAM );

		bExiting = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Handles notification of a Steam shutdown request since a Windows
// user in a second concurrent session requests to play this game. Shutdown
// this process immediately if possible.
//-----------------------------------------------------------------------------
void CSteamClientP2P::OnSteamShutdown( SteamShutdown_t* /* callback */ )
{
	static bool bExiting = false;
	if ( !bExiting )
	{
		g_pLog->Write( "Network:: Steam shutdown request, shutting down\n" );

		// what to do in p2p network, when steam will close ?

		// try to clean-up
		LeaveLobby(); // only does a local Reset() since connection with steam was lost
		SetConnectionFailureState( "Steam shutdown.", INetwork::CONNECTION_LOST_STEAM );

		bExiting = true;
	}
}

//****************************************************************************************

//-----------------------------------------------------------------------------
// Purpose: Used to transition game state
//-----------------------------------------------------------------------------
void CSteamClientP2P::SetGameState( EClientState eState )
{
	if ( m_eGameState == eState )
		return;

	m_ulStateTransitionTime = OS_GetTimeMS();
	m_eGameState = eState;
}

//-----------------------------------------------------------------------------
// Purpose: set the error string to display in the UI
//-----------------------------------------------------------------------------
void CSteamClientP2P::SetConnectionFailureState( const char *pchErrorText, const EConnectionLostState eConnLostState )
{
	SetGameState( k_EClientConnectionFailure );

	// send conn lost event
	m_eConnectionLostState = eConnLostState;
	AddEvent( INetwork::EV_CONNECTION_LOST, (uint64)m_eConnectionLostState );

	sprintf( m_rgchErrorText, "%s", pchErrorText );
	g_pLog->Write( "Network:: ConnectionFailure:: Reason: %s\n", pchErrorText );
}

//-----------------------------------------------------------------------------
// Purpose: Updates what we show to friends about what we're doing and how to connect
//-----------------------------------------------------------------------------
void CSteamClientP2P::UpdateRichPresenceConnectionInfo()
{
	// connect string that will come back to us on the command line	when a friend tries to join our game
	char rgchConnectString[128];
	rgchConnectString[0] = 0;

	if( m_lobby.ullLobbyId != 0 && m_lobby.iNumPlayers == 1 )
	{
		// lobby connection method
		sprintf( rgchConnectString, "+connect_lobby %llu", m_lobby.ullLobbyId );
	}

	if( SteamFriends() )
		SteamFriends()->SetRichPresence( "connect", rgchConnectString );
}

//****************************************************************************************

#endif // ENABLE_STEAM
