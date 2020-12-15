#include "dxstdafx.h"
#include "NetworkBase.h"

//#define LATENCY_SIMULATOR
#ifdef LATENCY_SIMULATOR
#include "Kernel/Queue.h"
#include "Kernel/Random.h"
#include "os/os.h"
struct sLoopbackPacket
{
	unsigned char data[INetwork::MAX_RELIABLE_PACKET_SIZE];
	unsigned int dataSize;
	unsigned int time;
};
// this latency is added on top of the existent network latency
int g_latency[2] = { 40, 55 }; // min/max milliseconds. if we choose different min/max the latency is randomly chosen between the two values (=jitter)
int g_packetLoss = 30; // percent
StaticQueue<sLoopbackPacket, 512> g_latencySend;
StaticQueue<sLoopbackPacket, 512> g_latencyRecv;
Random g_latencyRandom(OS_GetTimeMS());
#endif

bool NetworkBase::Start()
{
	m_stackEvents.reserve(20);
	SetConnectedState(k_EClientNotConnected);

	return true;
}

void NetworkBase::Reset()
{
	SetConnectedState(k_EClientNotConnected);

	m_lobby.Reset();
	m_stackEvents.clear();

#ifdef LATENCY_SIMULATOR
	g_latencyRecv.Clear();
	g_latencySend.Clear();
#endif
}

void NetworkBase::Update()
{
	// empty events stack
	m_stackEvents.clear();
}

bool NetworkBase::PopEvent(sEvent& ev)
{
	if (m_stackEvents.empty())
		return false;

	ev = m_stackEvents[m_stackEvents.size() - 1];
	m_stackEvents.resize(m_stackEvents.size() - 1);
	return true;
}

void NetworkBase::AddEvent(eEvent evType, uint64_t _ullData)
{
	// add at begining, pop from end
	m_stackEvents.insert(m_stackEvents.begin(), INetwork::sEvent(evType, _ullData));
}

const INetwork::sLobby& NetworkBase::GetCurrentLobby()
{
	return m_lobby;
}

bool NetworkBase::SimulateLatencySend(const void* pData, unsigned int dataSize)
{
#ifdef LATENCY_SIMULATOR
	if (g_latencyRandom.RandomInt(100) < g_packetLoss)
		return true; // packet loss

	sLoopbackPacket* pNewPacket = g_latencySend.New();
	if (!pNewPacket)
		return true; // packet loss

	memcpy(pNewPacket->data, pData, dataSize);
	pNewPacket->dataSize = dataSize;
	pNewPacket->time = OS_GetTimeMS() + g_latency[0] + g_latencyRandom.RandomInt(g_latency[1] - g_latency[0]);

	// check the list if we need to send previously sent packets
	if (g_latencySend.GetCount() && g_latencySend.Get(0)->time <= OS_GetTimeMS())
	{
		const sLoopbackPacket* pPacket = g_latencySend.PopFirst();
		if ((OS_GetTimeMS() - pPacket->time) < 1000) // too much time has passed, we didn't call send fast enough, just discard packet
		{
			pData = pPacket->data;
			dataSize = pPacket->dataSize;
		}
	}
	else
		return true;
#endif

	return false;
}

void NetworkBase::SimulateLatencyRecv(void* pData, unsigned int* pDataSize, unsigned int* pActualRecvSize)
{
#ifdef LATENCY_SIMULATOR
	if (*pDataSize)
	{
		sLoopbackPacket* pNewPacket = g_latencyRecv.New();
		if (pNewPacket)
		{
			memcpy(pNewPacket->data, pData, *pActualRecvSize);
			pNewPacket->dataSize = *pActualRecvSize;
			pNewPacket->time = OS_GetTimeMS() + g_latency[0] + g_latencyRandom.RandomInt(g_latency[1] - g_latency[0]);
		}
		*pDataSize = 0;
	}

	if (g_latencyRecv.GetCount() && g_latencyRecv.Get(0)->time <= OS_GetTimeMS())
	{
		const sLoopbackPacket* pPacket = g_latencyRecv.PopFirst();
		if ((OS_GetTimeMS() - pPacket->time) < 1000) // too much time has passed, we didn't call send recv fast enough, just discard packet
		{
			memcpy(pData, pPacket->data, pPacket->dataSize);
			*pActualRecvSize = pPacket->dataSize;
		}
	}
#endif
}