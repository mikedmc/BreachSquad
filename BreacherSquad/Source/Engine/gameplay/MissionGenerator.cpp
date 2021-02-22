#include "dxstdafx.h"
#include "MissionGenerator.h"


CMissionGenerator::CMissionGenerator()
{

}

CMissionGenerator::~CMissionGenerator()
{
	Release();
}

void CMissionGenerator::Release()
{
}


std::vector<CInventoryArea*> CMissionGenerator::FilterAreas(int nMinConnectors, int nMaxConnectors, int dirFlags, std::wstring strTagsAny /*= L""*/, std::wstring strTagsAll /*= L""*/, std::wstring strTagsNone /*= L""*/)
{
	std::vector<CInventoryArea*> retList;
	for (auto iarea : m_arrInventory)
	{
		if ((iarea.nAvailable > 0) &&
			(iarea.arrConnectors.size() >= nMinConnectors) &&
			(iarea.arrConnectors.size() <= nMaxConnectors) &&
			((iarea.nAreaConnDirFlags & dirFlags) != 0))
		{
			bool bAdd = true;
			// check tags
			if ((strTagsAny.length() > 0) && (!StringContainsAnyToken(iarea.areaSpecs.strTags, strTagsAny, L",")))
				bAdd = false;
			if ((strTagsAll.length() > 0) && (!StringContainsAllTokens(iarea.areaSpecs.strTags, strTagsAll, L",")))
				bAdd = false;
			if ((strTagsNone.length() > 0) && (StringContainsAnyToken(iarea.areaSpecs.strTags, strTagsNone, L",")))
				bAdd = false;

			if (bAdd)
				retList.push_back(&iarea);
		}
	}

	return retList;
}


CAreaBlock CMissionGenerator::GetPlacedBlockDescAt(Vec2i vPos)
{
	CAreaBlock retab;

	for (auto pa : m_arrPlaced)
	{
		if (!pa.AABB.Contains(vPos))
			continue;
		// see if block is set
		Vec2i vLocal(vPos.x - pa.AABB.x, vPos.y - pa.AABB.y);
		EDir retdir = EDIR_NONE;
		bool bIsSet = pa.areaSpecs.GetBlockIsSet(vLocal, retdir);
		// return true if block is set or check all blocks
		if (bIsSet)
		{
			retab.eConnectionDir = retdir;
			retab.bIsSet = true;
			retab.pParentArea = &pa;
			return retab;
		}
	}

	return retab;
}

CInventoryArea::CInventoryArea(CAreaSpecs as, int nTotalAvailable)
{
	areaSpecs = as;
	nAvailable = nTotalAvailable;
	nAreaConnDirFlags = 0;
	for (int kk = 0; kk < as.strSpecs.length(); kk++)
	{
		if (as.strSpecs[kk] == 'L')
		{
			nAreaConnDirFlags |= K_DIRFLAG_LEFT;
			arrConnectors.push_back(CAreaConnector(Vec2i(kk / as.sizeBL.x, kk % as.sizeBL.y), EDIR_LEFT));
		}
		else if (as.strSpecs[kk] == 'U')
		{
			nAreaConnDirFlags |= K_DIRFLAG_UP;
			arrConnectors.push_back(CAreaConnector(Vec2i(kk / as.sizeBL.x, kk % as.sizeBL.y), EDIR_UP));
		}
		else if (as.strSpecs[kk] == 'R')
		{
			nAreaConnDirFlags |= K_DIRFLAG_RIGHT;
			arrConnectors.push_back(CAreaConnector(Vec2i(kk / as.sizeBL.x, kk % as.sizeBL.y), EDIR_RIGHT));
		}
		else if (as.strSpecs[kk] == 'D')
		{
			nAreaConnDirFlags |= K_DIRFLAG_DOWN;
			arrConnectors.push_back(CAreaConnector(Vec2i(kk / as.sizeBL.x, kk % as.sizeBL.y), EDIR_DOWN));
		}
	}
}

///----------------------------------------------------------------------------------
/// SINGLETON
///----------------------------------------------------------------------------------
CMissionGenerator& UTGetMissionGen()
{
	static CMissionGenerator g_MissionGen;
	return g_MissionGen;
}
