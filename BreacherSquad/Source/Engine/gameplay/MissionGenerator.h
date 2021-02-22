#pragma once

// Area connectors
class CPlacedArea;
class CAreaConnector
{
public:
	Vec2i				pos;				// position of connector in local space (blocks coords)
	EDir				dir;				// direction of connection (K_DIR_...)
	CPlacedArea*		pConnectedArea;		// pointer to connected area

	CAreaConnector() : pos({ 0, 0 }), dir(EDir::EDIR_NONE), pConnectedArea(nullptr)
	{}

	CAreaConnector(Vec2i nPos, EDir nDir) : pos(nPos), dir(nDir), pConnectedArea(nullptr)
	{}
};

// contents of the inventory
class CInventoryArea
{
public:
	CAreaSpecs			areaSpecs;			// data copied from AreasInventory
	int					nAvailable;			// Number of available areas of this type
	int					nAreaConnDirFlags;  // Flags of all connections available for this area
	std::vector<CAreaConnector>		arrConnectors;	// List of available connectors (pos and dir)

	// Computes necessary connectors data and other necessary data
	CInventoryArea(CAreaSpecs as, int nTotalAvailable = 1);

	// Copies connector data into new array
	std::vector<CAreaConnector*> GetMatchingConnectors(EDir dir);
};

// A single block, used as return type mostly
struct CAreaBlock
{
	bool			bIsSet;
	EDir			eConnectionDir;
	CPlacedArea*	pParentArea;

	CAreaBlock() : bIsSet(false), eConnectionDir(EDIR_NONE), pParentArea(nullptr)
	{}
};

class CPlacedArea
{
public:
	RECTXYWH		AABB;				// world space rectangle in blocks positions
	CAreaSpecs		areaSpecs;
	int				nInventoryIdx;		// index in inventory. Could replace areaSpecs...
	std::vector<CAreaConnector> arrConnections;	// array of connections with neghboring areas

	int				nID;				// area ID needed for generating from story
	int				nGeneration;		// generation of placed area

	std::wstring	strAreaTags;
	std::wstring	strAreaName;		// not useful in final game

public:
	// returns list of available connectors
	std::vector<CAreaConnector*> GetAvailableConnectors(bool bShuffle)
	{
		std::vector<CAreaConnector*> arrConn;
		for (int ncon = 0; ncon < arrConnections.size(); ncon++)
		{
			// only add not connected connectors
			if (arrConnections[ncon].pConnectedArea == null)
				arrConn.push_back(&arrConnections[ncon]);
		}

		//#TODO: ar trebui sa foloseasca randomul sincronizat in retea. va face shuffle managerul
		//if (bShuffle)
			//m_rnd.ShuffleArray(arrConn.data(), arrConn.size(), arrConn.size() * 2);

		return arrConn;
	}

	CPlacedArea(CInventoryArea* area, Vec2i vPos)
	{
		AABB.Set(vPos.x, vPos.y, area->areaSpecs.sizeBL.x, area->areaSpecs.sizeBL.y);
		nGeneration = -1;
		nID = -1;
		strAreaTags = area->areaSpecs.strTags;
		strAreaName = area->areaSpecs.strFilename;
		areaSpecs = area->areaSpecs;
		arrConnections.clear();
		// copy connectors
		for (int kk = 0; kk < area->arrConnectors.size(); kk++)
		{
			CAreaConnector nc;
			nc.pConnectedArea = null;
			// bring connector position in placed area space:
			nc.pos.x += vPos.x;
			nc.pos.y += vPos.y;

			arrConnections.push_back(nc);
		}
	}

};

class CMissionGenerator
{
private:
	CRandom						m_rnd;				// RNG
	Vec2i						m_vStart;			// Level generation start point
	RECTXYWH					m_levelAABB;		// level AABB after generation
	
	std::vector<CPlacedArea>	m_arrPlaced;		// placed CPlacedArea elements

public:
	std::vector<CInventoryArea> m_arrInventory;

public:
	CMissionGenerator();
	~CMissionGenerator();
	// Builds the inventory from available areas
	void						BuildInventory(DWORD LevelRandSeed);
	// Releases all areas descriptors
	void						Release();

	// Filters available inventory areas and returns inventory areas pointers. Tags will be separated by commas.
	std::vector<CInventoryArea*> FilterAreas(int nMinConnectors, int nMaxConnectors, int dirFlags, 
											std::wstring strTagsAny = L"", std::wstring strTagsAll = L"", std::wstring strTagsNone = L"");

	// Returns block data and returns connection direction if it has a connection (EDIR_NONE if not)
	// Returns AreaBlock.filled=false 
	CAreaBlock GetPlacedBlockDescAt(Vec2i vPos);

	// Returns true is iarea can be placed and linked correctly with existing areas
	bool IsZoneClear(CInventoryArea* iarea, Vec2i vPos);
	// Removes all placed children of specified parent
	void RemoveChildrenOf(CPlacedArea* parent);
	// removes all areas of generation >= nMinGeneration
	void RemoveGenerations(int nMinGeneration);

	int GetInventoryIdx(CInventoryArea* iarea)
	{
		for (int kk = 0; kk < m_arrInventory.size(); kk++)
		{
			if (iarea == &m_arrInventory[kk])
				return kk;
		}
		ErrorBox(K_ERR_WARNING, L"Inventory entry not found! Should not happen!");
		return -1;
	}

	// gets a random area that fits the requirements and places it in the level returning reference to it
	CPlacedArea* PlaceStoryArea(CPlacedArea* parent, CAreaConnector* parentConn, int nGeneration, int nConnectionsMin, int nConnectionsMax, 
		std::wstring strTagsAny = L"", std::wstring strTagsAll = L"", std::wstring strTagsNone = L"")
	{
		int nDirFlag = K_DIRFLAG_ALL;
		if (parentConn->dir == K_DIR_LEFT) nDirFlag = K_DIRFLAG_RIGHT;
		if (parentConn->dir == K_DIR_UP) nDirFlag = K_DIRFLAG_DOWN;
		if (parentConn->dir == K_DIR_RIGHT) nDirFlag = K_DIRFLAG_LEFT;
		if (parentConn->dir == K_DIR_DOWN) nDirFlag = K_DIRFLAG_UP;

		Vec2i vDirOff = GetDirVec2i(parentConn->dir);
		Vec2i vStitchPt(parentConn->pos.x + parent->AABB.x, parentConn->pos.y + parent->AABB.y);
		vStitchPt.x += vDirOff.x; vStitchPt.y += vDirOff.y;

		auto availableList = FilterAreas(nConnectionsMin, nConnectionsMax, nDirFlag, strTagsAny, strTagsAll, strTagsNone);
		m_rnd.ShuffleArray(availableList.data(), availableList.size(), availableList.size() * 2);

		if (availableList.size() == 0)
		{
			LOG(L"Insufficient rooms in inventory! dirflag: %d", nDirFlag);
		}

		for(auto iarea : availableList)
		{
			EDir tryConnDir = GetDirInverse(parentConn->dir);
			//gets list of all connectors for a specified direction and shuffles them
			std::vector<CAreaConnector*> arrConn = iarea->GetMatchingConnectors(tryConnDir);
			// we have no connectors that way, try next
			if (arrConn.size() <= 0)
				continue;
			m_rnd.ShuffleArray(arrConn.data(), arrConn.size(), arrConn.size() * 2);
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
					CPlacedArea na(iarea, tryPos);
					na.nGeneration = nGeneration;
					// save reference so we can increase available items when removing the placed area
					na.nInventoryIdx = GetInventoryIdx(iarea);
					// point parent connection to this
					parentConn->pConnectedArea = &na;
					//make child point to parent too
					Vec2i vStitchLocal(vStitchPt.x - na.AABB.x, vStitchPt.y - na.AABB.y);
					for(CAreaConnector con : na.arrConnections)
					{
						//#TODO: check for random connections and stitch them! Remove following "break" if doing so or generalize...
						// IsAreaClear allows random connections but it could have a flag that would not allow that
						if (con.pos == vStitchLocal)
						{
							con.pConnectedArea = parent;
							break;
						}
					}

					m_arrPlaced.push_back(na);

					return &na;
				}
			}
		}
		// no area fits
		return nullptr;
	}


};


CMissionGenerator& UTGetMissionGen();
