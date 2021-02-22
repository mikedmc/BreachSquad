#pragma once

#define K_LGEN_LOCK_WATCHDOG_COUNT  100
#define K_LGEN_TRIES_GENERATIONS	10
#define K_LGEN_TRIES_CHILDREN		10

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
	std::vector<CAreaConnector*> GetAvailableConnectors()
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

	int GetInventoryIdx(CInventoryArea* iarea);

	// returns array of all placed areas of specified generation
	std::vector<CPlacedArea*> GetShuffledPlacedAreas(int nGeneration);

	// gets a random area that fits the requirements and places it in the level returning reference to it
	CPlacedArea* PlaceStoryArea(CPlacedArea* parent, CAreaConnector* parentConn, int nGeneration, int nConnectionsMin, int nConnectionsMax, 
		std::wstring strTagsAny = L"", std::wstring strTagsAll = L"", std::wstring strTagsNone = L"");

	// Only adds corridors when children can't be placed
	bool GenerateWithCorridorsWhenNeeded(int maxDepth);
};


CMissionGenerator& UTGetMissionGen();
