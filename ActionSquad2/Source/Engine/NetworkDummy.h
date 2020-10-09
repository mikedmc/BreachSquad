#pragma once

#include "Network.h"

// empty implementation of INetwork interface
class CNetworkDummy : public INetwork
{
private: 
	sLobby			lobby;
public:
	bool			Start() { return true; }
	void			Update() {};
	bool			PopEvent(sEvent& ev) { return false; }
	bool			Send(const void* pData, unsigned int dataSize) { return false; }
	bool			SendReliable(const void* pData, unsigned int dataSize) { return false; }
	bool			Recv(void* pData, unsigned int* pDataSize) { return false; }
	void			CreateLobby(bool bFriendsOnly, const int iNumKeyValues = 0, const char* pKeyDataValues[][2] = NULL) {};
	void			LeaveLobby() {};
	void			JoinLobby(const uint64_t& ullLobbyId) {};
	void			RequestLobbyList(bool bOnlyNearbyLobbies, int iMaxResults, const int iNumFilterKeyValues = 0, const char* pFilterKeyDataValues[][2] = NULL) {};
	uint64_t		GetLobbyIdByIndex(int iLobbyIndex) { return 0; }
	const char*		GetLobbyData(const uint64_t ullLobbyId, const char* pszKey) { return null; }
	const sLobby&	GetCurrentLobby() { return lobby; }
	void			ShowInviteToLobbyUI() {};
};
