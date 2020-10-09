#pragma once

#ifdef ENABLE_STEAM

#ifdef __linux__
#include <stddef.h>
#endif //__linux__ 

#include "steam_api.h"
#include "isteammatchmaking.h"

#include "../NetworkBase.h"

#include <string>
#include <vector>


enum EClientConnectionChannel
{
	k_EClientConnectionChannel_SteamStuff = 0,
	k_EClientConnectionChannel_Game		  = 1
};

// Enum for possible game states on the client
enum EClientState
{
	k_EClientNotReady = 0x00,
	k_EClientReady,

	k_EClientCreatingLobby,
	k_EClientInLobby,
	k_EClientJoiningLobby,

	k_EClientConnectionFailure,
};

// an item in the list of lobbies we've found
struct LobbyBrowserItem
{
	LobbyBrowserItem()
		: ullLobbyId(0)
		, bDataAvailable(false)
	{
		szName[0] = '\0';
	}

	uint64_t	ullLobbyId;
	char		szName[256];
	bool		bDataAvailable;
};

class CSteamClientP2P : public NetworkBase
{
public:
	// Constructor
	CSteamClientP2P();

	// Destructor
	~CSteamClientP2P();

public: // INetwork inheritance
	virtual bool			Start();
	virtual void			Reset();

	virtual void			Update();

	virtual bool			Send( const void* pData, unsigned int dataSize );
	virtual bool			SendReliable(const void* pData, unsigned int dataSize);
	virtual bool			Recv( void* pData, unsigned int uiDataAllocSize, unsigned int* pActualRecvSize );

	virtual void			CreateLobby( bool bFriendsOnly, const int iNumKeyValues = 0, const char* pKeyDataValues[][2] = NULL ); //, const char* pKeyDataValues[][2], int numKeyDataValues) = 0;
	virtual void			LeaveLobby();
	
	// lobbyId is received either through the command line or through the EV_LOBBY_INVITED event
	// the result is returned with either EV_LOBBY_JOIN_FAILED or EV_LOBBY_JOIN_SUCCESS
	// -> will transition to 'k_EClientInLobby' when done or 'k_EClientConnectionFailure' if timeout
	virtual void			JoinLobby( const LobbyID& ullLobbyId );

	virtual void			RequestLobbyList( bool bOnlyNearbyLobbies, int iMaxResults, const int iNumFilterKeyValues = 0, const char* pFilterKeyDataValues[][2] = NULL );
	virtual int				GetLobbyListEntriesCount();
	virtual bool			GetLobbyListEntry(int iLobbyIndex, uint64_t	& out_ullLobbyId, char * out_szName);
	virtual bool			IsRequestingLobby();

	// after calling RequestLobbyList(), use this to get the lobbyId, after which we can call JoinLobby(lobbyId)
	virtual uint64_t		GetLobbyIdByIndex( int iLobbyIndex );
	virtual const char*		GetLobbyData( const uint64_t ullLobbyId, const char* pszKey );

	virtual void			ShowInviteToLobbyUI();

	virtual bool			IsUserConnected() override { return true; };

private:
	// general funcs
	//

	// Get the steam id for the local user at this client
	CSteamID				GetLocalSteamID() { return m_steamIDLocalUser; }

	// Get the local players name
	const char*				GetLocalPlayerName() { return SteamFriends()->GetFriendPersonaName( m_steamIDLocalUser ); }

	// Set game state
	EClientState			GetGameState() { return m_eGameState; }

	// set failure text
	const char*				GetConnectionFailureText() { return m_rgchErrorText; }

private:
	// Shared init for all constructors
	void					Init();

	// internal func to treat traffic received meant for internal network stuff
	void					ReceiveNetworkData();

	// internal use only
	void					SetGameState( EClientState eState );
	void					SetConnectionFailureState( const char *pchErrorText, const EConnectionLostState eConnLostState );

	// Updates what we show to friends about what we're doing and how to connect
	void					UpdateRichPresenceConnectionInfo();

	// checks if data is available for all the lobbies returned by RequestMatchmakingLobbies() func
	// and sends a EV_LOBBY_LIST_RECEIVED event with the number of received lobbies
	void					CheckMatchmakingLobbiesData();

	// debug info
	void					Debug_DumpConnectionInfo( const CSteamID& connectedSteamID );

private: // inside lobby callbacks / funcs

	void GetMembers();

	// user state change handler
	STEAM_CALLBACK( CSteamClientP2P, OnPersonaStateChange, PersonaStateChange_t, m_CallbackPersonaStateChange );

	// lobby state change handler
	STEAM_CALLBACK( CSteamClientP2P, OnLobbyDataUpdate, LobbyDataUpdate_t, m_CallbackLobbyDataUpdate );
	STEAM_CALLBACK( CSteamClientP2P, OnLobbyChatUpdate, LobbyChatUpdate_t, m_CallbackChatDataUpdate );

private: // lobby browser

	std::vector< LobbyBrowserItem >			m_listLobbies; 

	const std::vector< LobbyBrowserItem >&	GetLobbyList() { return m_listLobbies; }
	bool									IsRequestingLobbies() { return m_bRequestingLobbies; }

	CCallResult<CSteamClientP2P, LobbyMatchList_t> m_SteamCallResultLobbyMatchList;

	void OnLobbyMatchListCallback( LobbyMatchList_t *pLobbyMatchList, bool bIOFailure );
	STEAM_CALLBACK( CSteamClientP2P, OnLobbiesDataUpdatedCallback, LobbyDataUpdate_t, m_CallbackLobbiesDataUpdated );

private:
	// steam callbacks
	STEAM_CALLBACK( CSteamClientP2P, OnP2PSessionRequest, P2PSessionRequest_t, m_CallbackP2PSessionRequest );
	STEAM_CALLBACK( CSteamClientP2P, OnP2PSessionConnectFail, P2PSessionConnectFail_t, m_CallbackP2PSessionConnectFail );

private:
	// callback for when we're creating a new lobby
	void OnLobbyCreated( LobbyCreated_t *pCallback, bool bIOFailure );
	CCallResult<CSteamClientP2P, LobbyCreated_t> m_SteamCallResultLobbyCreated;

	// callback for when we've joined a lobby
	void OnLobbyEntered( LobbyEnter_t *pCallback, bool bIOFailure );
	CCallResult<CSteamClientP2P, LobbyEnter_t> m_SteamCallResultLobbyEntered;

	// callback for when the lobby game server has started
	STEAM_CALLBACK( CSteamClientP2P, OnLobbyGameCreated, LobbyGameCreated_t );

	STEAM_CALLBACK( CSteamClientP2P, OnGameJoinRequested, GameRichPresenceJoinRequested_t );
	STEAM_CALLBACK( CSteamClientP2P, OnGameLobbyJoinRequested, GameLobbyJoinRequested_t );

	// callbacks for Steam connection state
	STEAM_CALLBACK( CSteamClientP2P, OnSteamServersConnected, SteamServersConnected_t );
	STEAM_CALLBACK( CSteamClientP2P, OnSteamServersDisconnected, SteamServersDisconnected_t );
	STEAM_CALLBACK( CSteamClientP2P, OnSteamServerConnectFailure, SteamServerConnectFailure_t );
	
	// general steam callbacks; mb move this somewhere else ?
	//

	// ipc failure handler
	STEAM_CALLBACK( CSteamClientP2P, OnIPCFailure, IPCFailure_t );

	// Steam wants to shut down, Game for Windows applications should shutdown too
	STEAM_CALLBACK( CSteamClientP2P, OnSteamShutdown, SteamShutdown_t );

private:
	// Track whether we are in the middle of a refresh or not
	bool						m_bRequestingLobbies;

	CSteamID					m_steamIDLocalUser;	// SteamID for the local user on this client
	CSteamID					m_steamIDPeer;		// SteamID for the user with which I am playing

//	CVoiceChat*					m_pVoiceChat;		// p2p voice chat

	std::vector< std::string >	m_keyValueLobbyParams;	// key/value params; size/2

	// Current game state
	EClientState				m_eGameState;

	// Time the last state transition occurred (so we can count-down round restarts)
	uint64						m_ulStateTransitionTime;

	// Time we last got data from the server
	uint64						m_ulLastNetworkDataReceivedTime;

	// Keeps information about how the connection was lost, or what it was
	EConnectionLostState		m_eConnectionLostState;

	// Text to display if we are in an error state
	char						m_rgchErrorText[256];

	bool						m_bShowConnectionInfoOnce;	
};

#endif // ENABLE_STEAM