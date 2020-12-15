#pragma once

#include "Network.h"

#include <vector>

enum EClientConnectionState
{
	k_EClientNotConnected,	// Initial state, not connected to a server
	k_EClientConnected,		// We've established communication with the other peer
};

class NetworkBase : public INetwork
{
public:
	virtual bool			Start();
	virtual void			Reset();
	virtual void			Update();
	virtual bool			PopEvent(sEvent& ev);

	virtual const sLobby&	GetCurrentLobby();

protected:
	// add new event
	void					AddEvent(eEvent evType, uint64_t _ullData = 0);

	void					SetConnectedState(EClientConnectionState eState) { m_eConnectedStatus = eState; }
	EClientConnectionState	GetConnectedState() { return m_eConnectedStatus; }

	bool					SimulateLatencySend(const void* pData, unsigned int dataSize);
	void					SimulateLatencyRecv(void* pData, unsigned int* pDataSize, unsigned int* pActualRecvSize);

protected:
	// Track whether we are connected to with a peer
	EClientConnectionState		m_eConnectedStatus;

	INetwork::sLobby			m_lobby;

	std::vector< sEvent >		m_stackEvents;			// used as a stack, but actually a vector :o

};