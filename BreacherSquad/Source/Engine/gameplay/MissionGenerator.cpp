#include "dxstdafx.h"
#include "MissionGenerator.h"


std::vector<CAreaConnector*> CPlacedArea::GetAvailableConnectors()
{
	std::vector<CAreaConnector*> arrConn;
	for (int ncon = 0; ncon < arrConnections.size(); ncon++)
	{
		// only add not connected connectors
		if (arrConnections[ncon].pConnectedArea == null)
			arrConn.push_back(&arrConnections[ncon]);
	}

	return arrConn;
}

CPlacedArea::CPlacedArea(CInventoryArea* area, Vec2i vPos)
{
	AABB.Set(vPos.x, vPos.y, area->areaSpecs.sizeBL.x, area->areaSpecs.sizeBL.y);
	nGeneration = -1;
	nID = -1;
	nInventoryIdx = -1;
	strAreaTags = area->areaSpecs.strTags;
	strAreaFile = area->areaSpecs.strFilename;
	areaSpecs = area->areaSpecs;
	arrConnections.clear();
	// copy connectors
	for (int kk = 0; kk < area->arrConnectors.size(); kk++)
	{
		CAreaConnector nc;
		nc.dir = area->arrConnectors[kk].dir;
		nc.pConnectedArea = null;
		// bring connector position in placed area space:
		nc.pos.x = area->arrConnectors[kk].pos.x;
		nc.pos.y = area->arrConnectors[kk].pos.y;

		arrConnections.push_back(nc);
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
			arrConnectors.push_back(CAreaConnector(Vec2i(kk % as.sizeBL.x, kk / as.sizeBL.x), EDIR_LEFT));
		}
		else if (as.strSpecs[kk] == 'U')
		{
			nAreaConnDirFlags |= K_DIRFLAG_UP;
			arrConnectors.push_back(CAreaConnector(Vec2i(kk % as.sizeBL.x, kk / as.sizeBL.x), EDIR_UP));
		}
		else if (as.strSpecs[kk] == 'R')
		{
			nAreaConnDirFlags |= K_DIRFLAG_RIGHT;
			arrConnectors.push_back(CAreaConnector(Vec2i(kk % as.sizeBL.x, kk / as.sizeBL.x), EDIR_RIGHT));
		}
		else if (as.strSpecs[kk] == 'D')
		{
			nAreaConnDirFlags |= K_DIRFLAG_DOWN;
			arrConnectors.push_back(CAreaConnector(Vec2i(kk % as.sizeBL.x, kk / as.sizeBL.x), EDIR_DOWN));
		}
	}
}


std::vector<CAreaConnector*> CInventoryArea::GetMatchingConnectors(EDir dir)
{
	std::vector<CAreaConnector*> arrRet;
	for (int kk = 0; kk < arrConnectors.size(); kk++)
	{
		CAreaConnector* conn = &arrConnectors[kk];
		if (conn->dir == dir)
			arrRet.push_back(conn);
	}
	return arrRet;
}


CMissionGenerator::CMissionGenerator()
{
	m_rand.SetRandSeedTime();
	SAFE_DELETE_STDVEC(m_arrPlaced);
	m_arrInventory.clear();
	m_levelAABB.Set(0, 0, 0, 0);
}


CMissionGenerator::~CMissionGenerator()
{
	Release();
}

void CMissionGenerator::BuildInventory(std::vector<CAreaSpecs>& arrAreas)
{
	m_arrInventory.clear();
	m_arrInventory.reserve(arrAreas.size());
	for (auto area : arrAreas)
	{
		m_arrInventory.push_back(CInventoryArea(area, 20));
	}
}

void CMissionGenerator::Release()
{
	SAFE_DELETE_STDVEC(m_arrPlaced);
	m_arrInventory.clear();
}


std::vector<CInventoryArea*> CMissionGenerator::FilterAreas(int nMinConnectors, int nMaxConnectors, int dirFlags, std::wstring strTagsAny /*= L""*/, std::wstring strTagsAll /*= L""*/, std::wstring strTagsNone /*= L""*/)
{
	std::vector<CInventoryArea*> retList;
	for (int kk = 0; kk < m_arrInventory.size() ; kk++)
	{
		CInventoryArea* iarea = &m_arrInventory[kk];
		if ((iarea->nAvailable > 0) &&
			(iarea->arrConnectors.size() >= nMinConnectors) &&
			(iarea->arrConnectors.size() <= nMaxConnectors) &&
			((iarea->nAreaConnDirFlags & dirFlags) != 0))
		{
			bool bAdd = true;
			// check tags
			if ((strTagsAny.length() > 0) && (!StringContainsAnyToken(iarea->areaSpecs.strTags, strTagsAny, L",")))
				bAdd = false;
			if ((strTagsAll.length() > 0) && (!StringContainsAllTokens(iarea->areaSpecs.strTags, strTagsAll, L",")))
				bAdd = false;
			if ((strTagsNone.length() > 0) && (StringContainsAnyToken(iarea->areaSpecs.strTags, strTagsNone, L",")))
				bAdd = false;

			if (bAdd)
				retList.push_back(iarea);
		}
	}

	return retList;
}


CAreaBlock CMissionGenerator::GetPlacedBlockDescAt(Vec2i vPos)
{
	CAreaBlock retab;

	for (auto pa : m_arrPlaced)
	{
		if (!pa->AABB.Contains(vPos))
			continue;
		// see if block is set
		Vec2i vLocal(vPos.x - pa->AABB.x, vPos.y - pa->AABB.y);
		EDir retdir = EDIR_NONE;
		bool bIsSet = pa->areaSpecs.GetBlockIsSet(vLocal, retdir);
		// return true if block is set or check all blocks
		if (bIsSet)
		{
			retab.eConnectionDir = retdir;
			retab.bIsSet = true;
			retab.pParentArea = pa;
			return retab;
		}
	}

	return retab;
}


bool CMissionGenerator::IsZoneClear(CInventoryArea* iarea, Vec2i vPos)
{
	RectXYWHi AABBtest(vPos.x, vPos.y, iarea->areaSpecs.sizeBL.x, iarea->areaSpecs.sizeBL.y);
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
	LOG(L"Remove children of %s", parent->strAreaFile.c_str());
	// unlink parent's children
	for (int kk = 0; kk < parent->arrConnections.size() ; kk++)
	{
		auto conn = &parent->arrConnections[kk];
		// removes only the children (not parents and same generation areas)
		if ((conn->pConnectedArea != nullptr) && (conn->pConnectedArea->nGeneration > parent->nGeneration))
			conn->pConnectedArea = nullptr;
	}

	for (int kk = m_arrPlaced.size() - 1; kk >= 0; kk--)
	{
		CPlacedArea* area = m_arrPlaced[kk];
		// skip lower generation areas (parents)
		if (area->nGeneration <= parent->nGeneration)
			continue;

		for (int ll = 0; ll < area->arrConnections.size(); ll++)
		{
			auto pconn = &area->arrConnections[ll];
			if (pconn->pConnectedArea == nullptr)
				continue;
			if (pconn->pConnectedArea == parent)
			{
				// put it back into inventory
				_ASSERT((area->nInventoryIdx >= 0) && (area->nInventoryIdx < m_arrInventory.size()));
				m_arrInventory[area->nInventoryIdx].nAvailable++;
				//remove and break
				SAFE_DELETE(m_arrPlaced[kk]);
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
		if (area->nGeneration < nMinGeneration)
		{
			for (int kk = 0; kk < area->arrConnections.size(); kk++)
			{
				CAreaConnector* conn = &area->arrConnections[kk];
				if ((conn->pConnectedArea != null) && (conn->pConnectedArea->nGeneration >= nMinGeneration))
					conn->pConnectedArea = null;
			}
		}
	}
	// delete useless generations
	for (int kk = m_arrPlaced.size() - 1; kk >= 0; kk--)
	{
		CPlacedArea* area = m_arrPlaced[kk];
		if (area->nGeneration >= nMinGeneration)
		{
			// put it back into inventory
			_ASSERT((area->nInventoryIdx >= 0) && (area->nInventoryIdx < m_arrInventory.size()));
			m_arrInventory[area->nInventoryIdx].nAvailable++;
			//remove and break
			SAFE_DELETE(m_arrPlaced[kk]);
			m_arrPlaced.erase(m_arrPlaced.begin() + kk);
		}
	}
}


int CMissionGenerator::GetInventoryIdx(CInventoryArea* iarea)
{
	for (int kk = 0; kk < m_arrInventory.size(); kk++)
	{
		if (iarea == &m_arrInventory[kk])
			return kk;
	}
	ErrorBox(K_ERR_WARNING, L"Inventory entry not found! Should not happen!");
	return -1;
}


std::vector<CPlacedArea*> CMissionGenerator::GetShuffledPlacedAreas(int nGeneration)
{
	std::vector<CPlacedArea*> retArr;
	for (int kk = 0; kk < m_arrPlaced.size(); kk++)
	{
		CPlacedArea* placed = m_arrPlaced[kk];
		if (placed->nGeneration == nGeneration)
			retArr.push_back(placed);
	}

	m_rand.ShuffleStdVector(retArr, retArr.size() * 2);

	return retArr;
}


CPlacedArea* CMissionGenerator::PlaceStoryArea(CPlacedArea* parent, CAreaConnector* parentConn, int nGeneration, int nConnectionsMin, int nConnectionsMax, std::wstring strTagsAny /*= L""*/, std::wstring strTagsAll /*= L""*/, std::wstring strTagsNone /*= L""*/)
{
	int nDirFlag = K_DIRFLAG_ALL;
	if (parentConn->dir == EDIR_LEFT) nDirFlag = K_DIRFLAG_RIGHT;
	else if (parentConn->dir == EDIR_UP) nDirFlag = K_DIRFLAG_DOWN;
	else if (parentConn->dir == EDIR_RIGHT) nDirFlag = K_DIRFLAG_LEFT;
	else if (parentConn->dir == EDIR_DOWN) nDirFlag = K_DIRFLAG_UP;

	Vec2i vDirOff = GetDirVec2i(parentConn->dir);
	Vec2i vStitchPt(parentConn->pos.x + parent->AABB.x, parentConn->pos.y + parent->AABB.y);
	vStitchPt.x += vDirOff.x; vStitchPt.y += vDirOff.y;

	auto availableList = FilterAreas(nConnectionsMin, nConnectionsMax, nDirFlag, strTagsAny, strTagsAll, strTagsNone);
	m_rand.ShuffleStdVector(availableList, availableList.size() * 2);

	if (availableList.size() == 0)
	{
		LOG(L"Insufficient rooms in inventory! dirflag: %d", nDirFlag);
	}

	for (auto iarea : availableList)
	{
		EDir tryConnDir = GetDirInverse(parentConn->dir);
		//gets list of all connectors for a specified direction and shuffles them
		std::vector<CAreaConnector*> arrConn = iarea->GetMatchingConnectors(tryConnDir);
		// we have no connectors that way, try next
		if (arrConn.size() <= 0)
			continue;
		m_rand.ShuffleStdVector(arrConn, arrConn.size() * 2);
		for (auto pconnector : arrConn)
		{
			// find position of connection point
			// then find origin for area to place
			Vec2i tryPos(vStitchPt.x - pconnector->pos.x, vStitchPt.y - pconnector->pos.y);
			// see if area is clear 
			if (IsZoneClear(iarea, tryPos))
			{
				// consume from inventory
				iarea->nAvailable--;
				// all good, add new area
				CPlacedArea* na = new CPlacedArea(iarea, tryPos);
				// point parent connection to this after adding it to the array
				parentConn->pConnectedArea = na;
				na->nGeneration = nGeneration;
				// save reference so we can increase available items when removing the placed area
				na->nInventoryIdx = GetInventoryIdx(iarea);
				//make child point to parent too
				Vec2i vStitchLocal(vStitchPt.x - na->AABB.x, vStitchPt.y - na->AABB.y);
				for (int kk = 0; kk < na->arrConnections.size(); kk++)
				{
					CAreaConnector* con = &na->arrConnections[kk];
					//#TODO: check for random connections and stitch them! Remove following "break" if doing so or generalize...
					// IsAreaClear allows random connections but it could have a flag that would not allow that
					if (con->pos == vStitchLocal)
					{
						con->pConnectedArea = parent;
						break;
					}
				}

				m_arrPlaced.push_back(na);

				return na;
			}
		}
	}
	// no area fits
	return nullptr;
}


bool CMissionGenerator::GenerateLevelRandomly(int maxDepth)
{
	LOG(L"Generating level - corridors when needed...");
	Vec2i posStart(10000, 10000);
	SAFE_DELETE_STDVEC(m_arrPlaced);
	m_arrPlaced.reserve(100);

	bool bLevelGenerated = true;

	auto availableList = FilterAreas(1, 1, K_DIRFLAG_ALL, L"start");
	if (availableList.size() > 0)
	{
		m_rand.ShuffleStdVector(availableList, availableList.size() * 2);
		// place starting area:
		CInventoryArea* selarea = availableList[0];
		CPlacedArea* pa = new CPlacedArea(selarea, posStart);
		selarea->nAvailable--;
		pa->nInventoryIdx = GetInventoryIdx(selarea);
		pa->nGeneration = 0;
		pa->nID = 0;
		m_arrPlaced.push_back(pa);

		int nMaxDepth = maxDepth;

		int nLockWatchdog = K_LGEN_LOCK_WATCHDOG_COUNT;     // fails the level generation if it tries too many times
		int nCurrGeneration = 0;
		while (nCurrGeneration < nMaxDepth)
		{
			nLockWatchdog--;
			if (nLockWatchdog < 0)
			{
				SAFE_DELETE_STDVEC(m_arrPlaced);
				bLevelGenerated = false;
				ErrorBox(K_ERR_WARNING, L"Could not generate level! Deadlock!");
				break;
			}
			///--- place actual rooms (corridors must be excluded)
			_ASSERT(nCurrGeneration < 50);

			int generationTries = K_LGEN_TRIES_GENERATIONS;
			bool bGenerationPlaced = false;
			while ((generationTries > 0) && (bGenerationPlaced == false))
			{
				bGenerationPlaced = true;
				// for each placed area of current generation:
				auto arrGenAreas2 = GetShuffledPlacedAreas(nCurrGeneration);
				for (int kk = 0; kk < arrGenAreas2.size(); kk++)
				{
					// find placed area
					CPlacedArea* placed = arrGenAreas2[kk];
					if (placed->nGeneration != nCurrGeneration)
						continue;
					// for each area try connecting the children N times
					int childTries = K_LGEN_TRIES_CHILDREN;
					bool bChildrenPlaced = false;
					while ((childTries > 0) && (bChildrenPlaced == false))
					{
						bChildrenPlaced = true;
						// get the shuffled connectors
						auto arrConn = placed->GetAvailableConnectors();
						m_rand.ShuffleStdVector(arrConn, arrConn.size() * 2);

						// take connectors one by one and try to place random children
						for (int ncon = 0; ncon < arrConn.size(); ncon++)
						{
							auto curcon = arrConn[ncon];
							//Place random area tries to place all available items with future depth
							//#TODO: better min/max connections heuristic (distance from last split, add more splits, weighted randoms)
							int nMinConn = 2, nMaxConn = 4;
							if (placed->nGeneration + 1 == nMaxDepth)
							{
								nMinConn = 1;
								nMaxConn = 1;
							}

							CPlacedArea* plarea = PlaceStoryArea(placed, curcon, placed->nGeneration + 1, nMinConn, nMaxConn, L"", L"", K_LGEN_TAGS_SPECIAL_AVOID);
							if (plarea == null)
							{
								LOG(L"Could not place children! Removing them! try: %d", childTries);
								bChildrenPlaced = false;
								//remove already placed children of this parent area
								RemoveChildrenOf(placed);

								// add corridor on this connection
								CPlacedArea* plhall = PlaceStoryArea(placed, curcon, placed->nGeneration, 2, 2, K_LGEN_TAGS_HALL_ANY);
								if (plhall != nullptr)
								{
									LOG(L"Corridor placed.");
									// add corridor as level 6 area too so it gets completed on next pass
									arrGenAreas2.push_back(curcon->pConnectedArea);
								}

								// exit for
								break;
							}
						}

						childTries--;
					}
					// failed to place children after many tries:
					if (bChildrenPlaced == false)
					{
						LOG(L"Generation failed! try: %d", generationTries);
						bGenerationPlaced = false;
						//remove parent generations and all of their children
						if (nCurrGeneration > 0)
						{
							RemoveGenerations(nCurrGeneration);
							nCurrGeneration--;
						}
						else
						{
							// returned to starting point, failed generating level!
							generationTries = 0;
						}
						// exit generations for
						break;
					}
				}

				generationTries--;
			}

			// AL GOOD, prepare next generation
			if (bGenerationPlaced == true)
			{
				LOG(L"-- Generation placed! gen: %d", nCurrGeneration + 1);
				nCurrGeneration++;
			}
			else
			{
				SAFE_DELETE_STDVEC(m_arrPlaced);
				bLevelGenerated = false;
				nCurrGeneration = maxDepth; //force exit while
				ErrorBox(K_ERR_WARNING, L"Could not generate level!");
				break;
			}
		}
	}

	m_levelAABB.Set(0, 0, 0, 0);
	if (bLevelGenerated)
	{
		// find level AABB
		Vec2i vMin(1000000, 1000000);
		Vec2i vMax(-1000000, -1000000);
		for (auto pa : m_arrPlaced)
		{
			if (pa->AABB.x < vMin.x) vMin.x = pa->AABB.x;
			if (pa->AABB.y < vMin.y) vMin.y = pa->AABB.y;
			if (pa->AABB.Right() > vMax.x) vMax.x = pa->AABB.Right();
			if (pa->AABB.Bottom() > vMax.y) vMax.y = pa->AABB.Bottom();
		}
		// move all blocks to 0
		for (auto pa : m_arrPlaced)
		{
			pa->AABB.Move(-vMin.x, -vMin.y);
		}
		vMax.x -= vMin.x; vMax.y -= vMin.y;
		vMin.x = vMin.y = 0;
		// compute final level aabb in tiles
		m_levelAABB.Set(vMin.x * K_LGEN_BLOCK_W, vMin.y * K_LGEN_BLOCK_H, (vMax.x - vMin.x + 1) * K_LGEN_BLOCK_W, (vMax.y - vMin.y + 1) * K_LGEN_BLOCK_H);
		LOG(L"-- Level generated OK!");
	}

	m_arrPlaced.shrink_to_fit();

	return bLevelGenerated;
}


bool CMissionGenerator::GenerateLevelFromStory(CMissionStory* story)
{
	_ASSERT(story != nullptr);
	Vec2i posStart(10000, 10000);
	LOG(L"Generating level from story..");
	SAFE_DELETE_STDVEC(m_arrPlaced);
	//#TODO: reserve up to 100 rooms
	m_arrPlaced.reserve(100);

	bool bLevelGenerated = true;

	CStoryRoom* pRoomStart = &story->arrGenerations[0].arrRooms[0];
	// WARNING! story must have a single starting point!
	int nStartChildren = pRoomStart->nChildren;
	//get "start" flags... all of them
	std::wstring strStartFlags = pRoomStart->tags_any;

	auto availableList = FilterAreas(nStartChildren, nStartChildren, K_DIRFLAG_ALL, strStartFlags);
	if (availableList.size() > 0)
	{
		m_rand.ShuffleStdVector(availableList, availableList.size() * 2);
		// place starting area:
		CInventoryArea* selarea = availableList[0];
		CPlacedArea* pa = new CPlacedArea(selarea, posStart);
		selarea->nAvailable--;
		pa->nInventoryIdx = GetInventoryIdx(selarea);
		pa->nGeneration = 0;
		pa->nID = pRoomStart->nID; //usually ID:0 for first room
		m_arrPlaced.push_back(pa);

		int nLockWatchdog = K_LGEN_LOCK_WATCHDOG_COUNT;
		int nCurrGeneration = 0;
		while (nCurrGeneration < story->arrGenerations.size())
		{
			nLockWatchdog--;
			if (nLockWatchdog < 0)
			{
				SAFE_DELETE_STDVEC(m_arrPlaced);
				bLevelGenerated = false;
				ErrorBox(K_ERR_WARNING, L"Could not generate level! Deadlock!");
				break;
			}
			///--- place actual rooms (corridors must be excluded)
			_ASSERT(nCurrGeneration < 50);

			int generationTries = K_LGEN_TRIES_GENERATIONS;
			bool bGenerationPlaced = false;
			while ((generationTries > 0) && (bGenerationPlaced == false))
			{
				bGenerationPlaced = true;
				// for each placed area of current generation:
				auto arrGenAreas = GetShuffledPlacedAreas(nCurrGeneration);
				for (int kk = 0; kk < arrGenAreas.size(); kk++)
				{
					// find placed area
					CPlacedArea* placed = arrGenAreas[kk];
					if (placed->nGeneration != nCurrGeneration)
						continue;
					// for each area try connecting the children N times
					int childTries = K_LGEN_TRIES_CHILDREN;
					bool bChildrenPlaced = false;
					while ((childTries > 0) && (bChildrenPlaced == false))
					{
						bChildrenPlaced = true;
						// get the shuffled connectors
						auto arrConn = placed->GetAvailableConnectors();
						m_rand.ShuffleStdVector(arrConn, arrConn.size() * 2);
						// take connectors one by one and try to place random children according to story
						for (int ncon = 0; ncon < arrConn.size(); ncon++)
						{
							auto curcon = arrConn[ncon];
							// get data from story
							CStoryRoom* entry = story->GetNextAvailableRoom(placed->nGeneration + 1, placed->nID, false);
							if (entry == nullptr)
							{
								ErrorBox(K_ERR_WARNING, L"Failed to get story area! generation=%d", placed->nGeneration + 1);
								//#TODO: should break level generation...?
								continue;
							}
							// if area does not specify any kind of flag then we avoid special areas by default
							std::wstring strTagsAvoid = entry->tags_none;
							if ((entry->tags_none.length() == 0) && (entry->tags_any.length() == 0) && (entry->tags_all.length() == 0))
								strTagsAvoid = K_LGEN_TAGS_SPECIAL_AVOID;

							// Place random area tries to place all available items with future generation depth
							CPlacedArea* parea = PlaceStoryArea(placed, curcon, placed->nGeneration + 1, entry->nChildren + 1, entry->nChildren + 1, entry->tags_any, entry->tags_all, strTagsAvoid);
							if (parea == nullptr)
							{
								LOG(L"Could not place children! Removing them! try: %d", childTries);
								bChildrenPlaced = false;
								//remove already placed children of this parent area
								RemoveChildrenOf(placed);
								// clear "used" flag in story
								story->ClearChildEntries(placed->nGeneration + 1, placed->nID);

								// add corridor on this connection
								CPlacedArea* pcorridor = PlaceStoryArea(placed, curcon, placed->nGeneration, 2, 2, K_LGEN_TAGS_HALL_ANY, L"", L"");
								if (pcorridor != nullptr)
								{
									LOG(L"Corridor placed.");
									// set same ID to corridor as room he's coming from
									pcorridor->nID = placed->nID;
									// add corridor as level 6 area too so it gets completed on next pass
									arrGenAreas.push_back(curcon->pConnectedArea);
								}

								// exit for
								break;
							}
							else
							{
								// set story id to room
								parea->nID = entry->nID;
								entry->bUsed = true;
							}
						}

						childTries--;
					}
					// failed to place children after many tries:
					if (bChildrenPlaced == false)
					{
						LOG(L"Generation failed! try: %d", generationTries);
						bGenerationPlaced = false;
						//remove parent generations and all of their children
						if (nCurrGeneration > 0)
						{
							RemoveGenerations(nCurrGeneration);
							// clear "used" flags for current generation and all children
							story->ClearEntriesFromGeneration(nCurrGeneration);
							nCurrGeneration--;
						}
						else
						{
							// returned to starting point, failed generating level!
							generationTries = 0;
						}
						// exit generations for
						break;
					}
				}

				generationTries--;
			}

			// AL GOOD, prepare next generation
			if (bGenerationPlaced == true)
			{
				LOG(L"-- Generation placed! gen: %d", nCurrGeneration + 1);
				nCurrGeneration++;
			}
			else
			{
				SAFE_DELETE_STDVEC(m_arrPlaced);
				story->ClearEntriesFromGeneration(0);
				bLevelGenerated = false;
				nCurrGeneration = story->arrGenerations.size(); //force exit while
				ErrorBox(K_ERR_WARNING, L"Could not generate level from story!");
				break;
			}
		}
	}

	m_levelAABB.Set(0, 0, 0, 0);
	if (bLevelGenerated)
	{
		// find level min/max coords
		Vec2i vMin(1000000, 1000000);
		Vec2i vMax(-1000000, -1000000);
		for (auto pa : m_arrPlaced)
		{
			if (pa->AABB.x < vMin.x) vMin.x = pa->AABB.x;
			if (pa->AABB.y < vMin.y) vMin.y = pa->AABB.y;
			if (pa->AABB.Right() > vMax.x) vMax.x = pa->AABB.Right();
			if (pa->AABB.Bottom() > vMax.y) vMax.y = pa->AABB.Bottom();
		}
		// move all blocks to 0
		for (auto pa : m_arrPlaced) 
		{
			pa->AABB.Move(-vMin.x, -vMin.y);
		}
		vMax.x -= vMin.x; vMax.y -= vMin.y;
		vMin.x = vMin.y = 0;
		// compute final level aabb in tiles
		m_levelAABB.Set(vMin.x * K_LGEN_BLOCK_W, vMin.y * K_LGEN_BLOCK_H, (vMax.x - vMin.x + 1) * K_LGEN_BLOCK_W, (vMax.y - vMin.y + 1) * K_LGEN_BLOCK_H);
		LOG(L"-- Level generated from story OK!");
	}

	m_arrPlaced.shrink_to_fit();

	return bLevelGenerated;
}



///----------------------------------------------------------------------------------
/// SINGLETON
///----------------------------------------------------------------------------------
CMissionGenerator& UTGetMissionGen()
{
	static CMissionGenerator g_MissionGen;
	return g_MissionGen;
}

