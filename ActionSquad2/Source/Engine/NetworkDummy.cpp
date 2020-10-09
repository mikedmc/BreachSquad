#include "dxstdafx.h"

//gets enabled when not wanting networking
#ifndef ENABLE_NETWORKING

CNetworkDummy g_netDummy;
INetwork* g_pNetwork = &g_netDummy;

#endif