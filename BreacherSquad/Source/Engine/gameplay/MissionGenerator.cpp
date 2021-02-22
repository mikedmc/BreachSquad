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

bool CMissionGenerator::IsZoneClear(CInventoryArea* iarea, Vec2i vPos)
{
	RECTXYWH AABBtest(vPos.x, vPos.y, iarea->areaSpecs.sizeBL.x, iarea->areaSpecs.sizeBL.y);
	// check overlapping blocks
	for (int xx = 0; xx < AABBtest.w; xx++)
	{
		for (int yy = 0; yy < AABBtest.h; yy++)
		{
			// check map occupation only on occupied blocks in current test area
			EDir blockSrcConDir = EDIR_NONE;
			bool bBlockSrcFilled = iarea->areaSpecs.GetBlockIsSet(Vec2i(xx, yy), blockSrcConDir);
			if (bBlockSrcFilled)
			{
				CAreaBlock blockDest = GetPlacedBlockDescAt(Vec2i(xx + vPos.x, yy + vPos.y));
				//Form1.CGridCell blockPlaced = GetPlacedBlockAt(new Point(xx + vPos.X, yy + vPos.Y));
				if (blockDest.bIsSet)
					return false;
				// check on all neighbours in all directions as blocks might overlap
				for (int kk = 0; kk < EDIRS_COUNT; kk++)
				{
					EDir dir = (EDir)kk;
					Vec2i vOff = GetDirVec2i(dir);
					CAreaBlock pNeigh = GetPlacedBlockDescAt(Vec2i(xx + vPos.x + vOff.y, yy + vPos.x + vOff.y));
					//Form1.CGridCell pNeigh = GetPlacedBlockAt(new Point(xx + vPos.X + vOff.X, yy + vPos.Y + vOff.Y));
					// for each neighbour that is alrady placed check if we have the correct connector for random loops
					if (pNeigh.bIsSet)
					{
						EDir dir_inv = GetDirInverse(dir);
						// remote blocked connection
						if ((pNeigh.eConnectionDir == dir_inv) && (blockSrcConDir != dir))
							return false;
						// local block blocked connection
						if (blockSrcConDir == dir)
						{
							// only allowed if remote block has matching connector
							if (pNeigh.eConnectionDir != dir_inv)
								return false;
							else
							{
								// check for random connection using the area generations or current stitch point
								LOG(L"IsZoneClear:: Random connection found!");
							}
						}
					}
				}
			}
		}
	}
	return true;
}

void CMissionGenerator::RemoveChildrenOf(CPlacedArea* parent)
{
	// unlink parent's children
	for (auto conn : parent->arrConnections)
	{
		// removes only the children (not parents and same generation areas)
		if ((conn.pConnectedArea != nullptr) && (conn.pConnectedArea->nGeneration > parent->nGeneration))
			conn.pConnectedArea = null;
	}

	for (int kk = m_arrPlaced.size() - 1; kk >= 0; kk--)
	{
		CPlacedArea* area = &m_arrPlaced[kk];
		// skip lower generation areas (parents)
		if (area->nGeneration <= parent->nGeneration)
			continue;

		for (auto pconn : area->arrConnections)
		{
			if (pconn.pConnectedArea == nullptr)
				continue;
			if (pconn.pConnectedArea == parent)
			{
				// put it back into inventory
				m_arrInventory[area->nInventoryIdx].nAvailable++;
				//remove and break
				m_arrPlaced.erase(m_arrPlaced.begin() + kk);
				break;
			}
		}
	}
}

void CMissionGenerator::RemoveGenerations(int nMinGeneration)
{
	LOG(L"Removing generations >= %d", nMinGeneration);
	// unlink remaining generations from useless ones
	for (auto area : m_arrPlaced)
	{
		if (area.nGeneration < nMinGeneration)
		{
			for (auto conn : area.arrConnections)
			{
				if ((conn.pConnectedArea != null) && (conn.pConnectedArea->nGeneration >= nMinGeneration))
					conn.pConnectedArea = null;
			}
		}
	}
	// delete useless generations
	for (int kk = m_arrPlaced.size() - 1; kk >= 0; kk--)
	{
		CPlacedArea* area = &m_arrPlaced[kk];
		if (area->nGeneration >= nMinGeneration)
		{
			// put it back into inventory
			m_arrInventory[area->nInventoryIdx].nAvailable++;
			//remove and break
			m_arrPlaced.erase(m_arrPlaced.begin() + kk);
		}
	}
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

std::vector<CAreaConnector*> CInventoryArea::GetMatchingConnectors(EDir dir)
{
	std::vector<CAreaConnector*> arrRet;
	for (auto conn : arrConnectors)
	{
		if (conn.dir == dir)
			arrRet.push_back(&conn);
	}
	return arrRet;
}

///----------------------------------------------------------------------------------
/// SINGLETON
///----------------------------------------------------------------------------------
CMissionGenerator& UTGetMissionGen()
{
	static CMissionGenerator g_MissionGen;
	return g_MissionGen;
}
