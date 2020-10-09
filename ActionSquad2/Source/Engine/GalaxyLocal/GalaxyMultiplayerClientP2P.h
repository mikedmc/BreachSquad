#pragma once

#ifdef ENABLE_GALAXY

#include "NetworkBase.h"

#include "galaxy/IListenerRegistrar.h"
#include "galaxy/GalaxyID.h"
#include "galaxy/GalaxyApi.h"

enum EClientConnectionChannel
{
	//k_EClientConnectionChannel_SteamStuff = 0,
	k_EClientConnectionChannel_Game = 1
};

struct LobbyBrowserItem
{
	LobbyBrowserItem()
		: lobbyID(0)
		, bDataAvailable(false)
	{
		szName[0] = '\0';
	}

	galaxy::api::GalaxyID	lobbyID;
	char		szName[256];
	bool		bDataAvailable;
};

class CGalaxyClientP2P : public NetworkBase, 
	galaxy::api::ILobbyCreatedListener, 
	galaxy::api::ILobbyEnteredListener, 
	galaxy::api::IPersonaDataChangedListener,
	galaxy::api::ILobbyDataUpdateListener,
	galaxy::api::ILobbyMemberStateListener,
	galaxy::api::ILobbyListListener,
	galaxy::api::ILobbyDataRetrieveListener,
	galaxy::api::IGameJoinRequestedListener,
	galaxy::api::IGogServicesConnectionStateListener,
	galaxy::api::IOperationalStateChangeListener
{

public:
	CGalaxyClientP2P();
	~CGalaxyClientP2P();

public:
	virtual bool Start() override;
	virtual void Reset() override;
	virtual void Update() override;

	virtual bool Send(const void* pData, unsigned int dataSize) override;
	virtual bool SendReliable(const void* pData, unsigned int dataSize) override;
	virtual bool Recv(void* pData, unsigned int uiDataAllocSize, unsigned int* pActualRecvSize) override;

	virtual void CreateLobby(bool bFriendsOnly, const int iNumKeyValues = 0, const char* pKeyDataValues[][2] = NULL) override;
	virtual void LeaveLobby() override;
	virtual void JoinLobby(const LobbyID& ullLobbyId) override;
	virtual void RequestLobbyList(bool bOnlyNearbyLobbies, int iMaxResults, const int iNumFilterKeyValues = 0, const char* pFilterKeyDataValues[][2] = NULL) override;
	virtual int	 GetLobbyListEntriesCount() override;
	virtual bool GetLobbyListEntry(int iLobbyIndex, uint64_t & out_ullLobbyId, char * out_szName) override;
	virtual bool IsRequestingLobby() override;

	virtual uint64_t GetLobbyIdByIndex(int iLobbyIndex) override;
	virtual const char* GetLobbyData(const uint64_t ullLobbyId, const char* pszKey) override;

	virtual void ShowInviteToLobbyUI() override;

	virtual bool IsUserConnected() override;

public: //Galaxy callbacks
	virtual void OnLobbyCreated(const galaxy::api::GalaxyID& lobbyID, galaxy::api::LobbyCreateResult result) override;
	virtual void OnLobbyEntered(const galaxy::api::GalaxyID& lobbyID, galaxy::api::LobbyEnterResult result) override;
	virtual void OnPersonaDataChanged(galaxy::api::GalaxyID userID, uint32_t personaStateChange) override;
	virtual void OnLobbyDataUpdateSuccess(const galaxy::api::GalaxyID& lobbyID) override;
	virtual void OnLobbyDataUpdateFailure(const galaxy::api::GalaxyID& lobbyID, galaxy::api::ILobbyDataUpdateListener::FailureReason failureReason) override;
	virtual void OnLobbyMemberStateChanged(const galaxy::api::GalaxyID& lobbyID, const galaxy::api::GalaxyID& memberID, galaxy::api::LobbyMemberStateChange memberStateChange) override;
	virtual void OnLobbyList(uint32_t lobbyCount, galaxy::api::LobbyListResult result) override;
	virtual void OnLobbyDataRetrieveSuccess(const galaxy::api::GalaxyID& lobbyID) override;
	virtual void OnLobbyDataRetrieveFailure(const galaxy::api::GalaxyID& lobbyID, galaxy::api::ILobbyDataRetrieveListener::FailureReason failureReason) override;
	virtual void OnGameJoinRequested(galaxy::api::GalaxyID userID, const char* connectionString) override;
	virtual void OnConnectionStateChange(galaxy::api::GogServicesConnectionState connectionState) override;
	virtual void OnOperationalStateChanged(uint32_t operationalState) override;

protected:
	// Updates what we show to friends about what we're doing and how to connect
	void		UpdateRichPresenceConnectionInfo();
	// checks if data is available for all the lobbies returned by RequestMatchmakingLobbies() func
	// and sends a EV_LOBBY_LIST_RECEIVED event with the number of received lobbies
	void		CheckMatchmakingLobbiesData();
	void		UpdateLobbyMembers();
	
	void		SetConnectionFailureState(const char *pchErrorText);

private:
	bool						m_bRequestingLobbies;
	bool						m_bRegistered;

	galaxy::api::GalaxyID		m_peerID;

	std::vector< std::string >	m_keyValueLobbyParams;	// key/value params; size/2
	std::vector< LobbyBrowserItem >	m_listLobbies;

};

#endif // ENABLE_GALAXY
