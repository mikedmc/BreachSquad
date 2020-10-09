#pragma once

class INetwork
{
public:
	static const int MAX_UNRELIABLE_PACKET_SIZE		 = 1200; // any message sent/received must not exceed this
	static const int MAX_RELIABLE_PACKET_SIZE		 = (1024 * 1024); // 1 MB
	static const int MAX_PLAYERS_PER_GAME		     = 2; // maximum number of players that can join a lobby
	static const int MILLISECONDS_CONNECTION_TIMEOUT = 30000; // timeout for a lobby join attempt

	enum eEvent
	{
		EV_UNKNOWN,

		EV_LOBBY_CREATE_SUCCESS, // received after calling CreateLobby()
		EV_LOBBY_CREATE_FAILED, // received after calling CreateLobby()

		// received when someone invites us in a lobby. the lobbyId is passed through "data". We must call JoinLobby().
		EV_LOBBY_INVITED,

		// when a lobby was successfully joined after calling JoinLobby(). The lobby status is set to LOBBY_IN_LOBBY, no action is needed
		EV_LOBBY_JOIN_SUCCESS,

		// join failed after calling JoinLobby(). The lobby state is set to LOBBY_UNKNOWN, no action is needed
		EV_LOBBY_JOIN_FAILED,

		// the other player in the lobby has left. The lobby state is set to LOBBY_UNKNOWN, no action is needed.
		EV_LOBBY_PLAYER_LEFT,

		// a player has joined our lobby. The number of players in the current lobby will be increased to 2
		EV_LOBBY_PLAYER_JOINED,

		// received in response to calling RequestLobbyList(). the number of lobbies will be in sEvent::data. We can call GetLobbyIdByIndex() to retrieve the actuall lobbyId
		EV_LOBBY_LIST_RECEIVED,

		// received when connection is lost, in these cases:
		// 0 - connection lost with the other player
		// 1 - connection lost with steam servers / steam disconnected
		EV_CONNECTION_LOST,

		EV_NUM_EVENTS
	};

	enum EConnectionLostState
	{
		CONNECTION_LOST_P2P = 0,
		CONNECTION_LOST_STEAM
	};

	struct sEvent
	{
	public:
		sEvent()
		{
			eType = EV_UNKNOWN;
		}

		sEvent( eEvent _eType )
		{
			eType = _eType;
		}

		sEvent( eEvent _eType, const uint64_t& _ullData )
		{
			eType = _eType;
			ullData = _ullData;
		}

	public:
		eEvent eType;

		// if EV_LOBBY_INVITED, this is the lobbyId, which we can use to call JoinLobby()
		// if EV_LOBBY_LIST_RECEIVED, this is the number of lobbies found, which we can iterate through
		// if EV_CONNECTION_LOST, this is: 0 - p2p connection lost, 1 - connection lost with steam, 2 - timeout connecting to lobby
		uint64_t ullData;
	};


	// an item in the list of lobbies we've found to display
	struct LobbyUser
	{
		LobbyUser()
		{
			m_rgchName[0] = '\0';
		}

		NetID	m_netUserId;
		char	m_rgchName[256];
	};

	enum eLobbyState
	{
		LOBBY_UNKNOWN, // lobby does not exist. we haven't called CreateLobby() nor JoinLobby()
		LOBBY_CREATING, // after calling CreateLobby() and until we get a EV_LOBBY_CREATE_* message
		LOBBY_JOINING, // after calling JoinLobby() and until we get a EV_LOBBY_JOIN_* message
		LOBBY_IN_LOBBY, // the lobby has been successfully created and there's at least 1 person in the lobby. Query Peer/Lobby to see when two people joined. After that we can start communication with send/recv
	};

	struct sLobby
	{
	public:
		sLobby()
		{
			Reset();
		}

		void Reset()
		{
			eState = LOBBY_UNKNOWN;
			ullLobbyId = 0;
			bIAmOwner = false;
			iNumPlayers = 0;

#ifdef ENABLE_STEAM
			for( int i(0); i < MAX_PLAYERS_PER_GAME; ++i )
			{
				players[i].m_rgchName[0] = '\0';
				players[i].m_netUserId.Clear();
			}
#endif
#ifdef ENABLE_GALAXY
			for (int i(0); i < MAX_PLAYERS_PER_GAME; ++i)
			{
				players[i].m_rgchName[0] = '\0';
				players[i].m_netUserId = 0;
			}
#endif
		}

	public:
		eLobbyState		eState;
		LobbyID			ullLobbyId;
		bool			bIAmOwner; // true for the one that called CreateLobby. he is also player[0]. If the owner leaves the lobby, the other peer becomes the owner and player[0]

		int				iNumPlayers;
		LobbyUser		players[ MAX_PLAYERS_PER_GAME ];

		// metadata stuff (key/value pairs)
	};

	// boots up the networking system. Must be called after Steam/whatever the underlying system was initialized.
	virtual bool			Start() = 0;

	// runs the message loop. resets the previous event list when called, so make sure to use PopEvent() to parse the events before/after you call Update
	virtual void			Update() = 0;

	// PopEvent() retrieves events in FIFO order. must be called in a loop until it returns false
	virtual bool			PopEvent( sEvent& ev ) = 0;

	// Send/Recv can be called once there are 2 players in a lobby

	// returns false when you are not in a lobby with 2 players, or dataSize > MAX_PACKET_SIZE
	virtual bool			Send(const void* pData, unsigned int dataSize) = 0;

	// returns false when you are not in a lobby with 2 players, or dataSize > MAX_RELIABLE_PACKET_SIZE
	virtual bool			SendReliable(const void* pData, unsigned int dataSize) = 0;

	// returns false when there's nothing else to receive. Call in a loop until it returns false
	// uiDataAllocSize - the size of pData
	// pActualRecvSize - holds the actual size of the received package; check if this is larger than uiDataAllocSize and discard
	virtual bool			Recv( void* pData, unsigned int uiDataAllocSize, unsigned int* pActualRecvSize ) = 0;

	/*
	Can only create 1 lobby per user. Calling CreateLobby repeatedly will not do anything if we already created one or if we're in another lobby.
	The result will be returned through an event: EV_LOBBY_CREATE_SUCCESS or EV_LOBBY_CREATE_FAILED
	Can be created with public visibility or friends-only.
	Maximum number of people in a lobby is 2 (owner + joiner).

	-Lobby metadata Key/value usage:
	const char* pKeyDataValues[][2] = 
	{
	{"map", "bla.xml"},
	{"muie", "cu menta"},
	};
	g_pNetwork->CreateLobby(true, pKeyDataValues, sizeof(pKeyDataValues) / sizeof(pKeyDataValues[0]));
	*/

	virtual void			CreateLobby( bool bFriendsOnly, const int iNumKeyValues = 0, const char* pKeyDataValues[][2] = NULL ) = 0;

	// if there's someone else in the lobby, they will be notified with EV_LOBBY_PLAYER_LEFT
	// if the lobby owner leaves the lobby, the other player automatically becomes the new owner, and his lobby status sets bIAmOwner to "true"
	// depending on the current game state, the remaining player could either call LeaveLobby() to completely close the lobby or continue to wait for a player to join
	virtual void			LeaveLobby() = 0;

	// lobbyId is received either through the command line or through the EV_LOBBY_INVITED event
	// the result is returned with either EV_LOBBY_JOIN_FAILED or EV_LOBBY_JOIN_SUCCESS
	virtual void			JoinLobby( const LobbyID& ullLobbyId ) = 0;

	// this call can take from 300ms to 5 seconds to complete, and has a timeout of 20 seconds. the result is sent through EV_LOBBY_LIST_RECEIVED
	// bOnlyNearbyLobbies - if true, only search for lobbies in the same or nearby region (aka k_ELobbyDistanceFilterDefault in Steam). If False, also include "far" regions, but not worldwide (aka k_ELobbyDistanceFilterFar)
	// maxResults - when quickplaying use "1"
	virtual void			RequestLobbyList( bool bOnlyNearbyLobbies, int iMaxResults, const int iNumFilterKeyValues = 0, const char* pFilterKeyDataValues[][2] = NULL ) = 0;
	// returns number of lobbies in last received list
	virtual int				GetLobbyListEntriesCount() = 0;
	// returns lobby[iLobbyIndex] name and ID in out_... vars 
	virtual bool			GetLobbyListEntry(int iLobbyIndex, uint64_t	& out_ullLobbyId, char * out_szName) = 0;
	// tells if getting the lobbies list is processing
	virtual bool			IsRequestingLobby() = 0;

	// after calling RequestLobbyList(), use this to get the lobbyId, after which we can call JoinLobby(lobbyId)
	virtual uint64_t		GetLobbyIdByIndex( int iLobbyIndex ) = 0;

	// key-based data for lobby 'ullLobbyId'
	virtual const char*		GetLobbyData( const uint64_t ullLobbyId, const char* pszKey ) = 0;

	// current lobby
	virtual const sLobby&	GetCurrentLobby() = 0;

	// invite a user to current lobby
	virtual void			ShowInviteToLobbyUI() = 0;

	// returns true if the user is connected and ready to access network features
	virtual bool			IsUserConnected() = 0;
};

static const char* eLobbyEventStr[INetwork::EV_NUM_EVENTS] =
{
	"EV_UNKNOWN",
	"EV_LOBBY_CREATE_SUCCESS",
	"EV_LOBBY_CREATE_FAILED",
	"EV_LOBBY_INVITED",
	"EV_LOBBY_JOIN_SUCCESS",
	"EV_LOBBY_JOIN_FAILED",
	"EV_LOBBY_PLAYER_LEFT",
	"EV_LOBBY_PLAYER_JOINED",
	"EV_LOBBY_LIST_RECEIVED",
	"EV_CONNECTION_LOST"
};

extern INetwork* g_pNetwork;
