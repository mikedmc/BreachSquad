#include "dxstdafx.h"
#include "MissionGenerator.h"


void CMissionGenerator::Release()
{
}


///----------------------------------------------------------------------------------
/// SINGLETON
///----------------------------------------------------------------------------------
CMissionGenerator& UTGetMissionGen()
{
	static CMissionGenerator g_MissionGen;
	return g_MissionGen;
}

