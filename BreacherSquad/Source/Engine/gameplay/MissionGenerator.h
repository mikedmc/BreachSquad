#pragma once
#include "MissionStory.h"

// deadlock counter (sometimes it oscillates indifinetly between 2 generations)
#define K_LGEN_LOCK_WATCHDOG_COUNT  100
// times to try to place one generation
#define K_LGEN_TRIES_GENERATIONS	10
// times to try to place children on one node
#define K_LGEN_TRIES_CHILDREN		10

// all tags that CAN'T be used for randomly picked rooms
#define K_LGEN_TAGS_SPECIAL_AVOID		L"special,hall"
// any of these tags will mean it's a hallway
#define K_LGEN_TAGS_HALL_ANY			L"hall"

// announce classes
class CPlacedArea;

///----------------------------------------------------------------------------------
/// connector description for in-between relationships
///----------------------------------------------------------------------------------
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

///----------------------------------------------------------------------------------
/// Contains inventory data like area description, how many were used, etc
///----------------------------------------------------------------------------------
class CInventoryArea
{
public:
	CAreaSpecs			areaSpecs;					// data copied from AreasInventory
	int					nAvailable;					// Number of available areas of this type
	int					nAreaConnDirFlags;			// Flags of all connections available for this area
	std::vector<CAreaConnector>		arrConnectors;	// List of available connectors (pos and dir) translated from areaSpecs

	// Computes necessary connectors data and other necessary data
	CInventoryArea(CAreaSpecs as, int nTotalAvailable = 1);

	// Copies connector data into new array
	std::vector<CAreaConnector*> GetMatchingConnectors(EDir dir);
};

///----------------------------------------------------------------------------------
/// A single block, used as return type for different methods
///----------------------------------------------------------------------------------
struct CAreaBlock
{
	bool			bIsSet;
	EDir			eConnectionDir;
	CPlacedArea*	pParentArea;

	CAreaBlock() : bIsSet(false), eConnectionDir(EDIR_NONE), pParentArea(nullptr)
	{}
};

///----------------------------------------------------------------------------------
/// Area that has been placed in the level
///----------------------------------------------------------------------------------
class CPlacedArea
{
public:
	RECTXYWH			AABB;				// world space rectangle in blocks positions
	CAreaSpecs			areaSpecs;
	int					nInventoryIdx;		// index in inventory. Could replace areaSpecs...
	std::vector<CAreaConnector> arrConnections;	// array of connections with neghboring areas

	int					nID;				// area ID needed for generating from story
	int					nGeneration;		// generation of placed area

	std::wstring		strAreaTags;
	std::wstring		strAreaFile;		// Loads the actual area from here

public:
	// returns list of available connectors
	std::vector<CAreaConnector*> GetAvailableConnectors();

	// Constructs the placed area from an inventory area and positions it and connectors at vPos
	CPlacedArea(CInventoryArea* area, Vec2i vPos);

};

///----------------------------------------------------------------------------------
/// Generates random levels from an inventory of areas that are divided in blocks
/// Blocks are 8x8 tiles. See areasInventory.h
///----------------------------------------------------------------------------------
class CMissionGenerator
{
private:
	CRandom						m_rand;				// RNG
	RECTXYWH					m_levelAABB;		// level AABB after generation
	
	std::vector<CPlacedArea>	m_arrPlaced;		// placed CPlacedArea elements

public:
	std::vector<CInventoryArea> m_arrInventory;

public:
	CMissionGenerator();
	~CMissionGenerator();
	// Builds the inventory from available areas
	void						BuildInventory();
	// Releases all areas descriptors
	void						Release();

	// Filters available inventory areas and returns inventory areas pointers. Tags will be separated by commas.
	std::vector<CInventoryArea*> FilterAreas(int nMinConnectors, int nMaxConnectors, int dirFlags, 
											std::wstring strTagsAny = L"", std::wstring strTagsAll = L"", std::wstring strTagsNone = L"");

	// Returns block data and returns connection direction if it has a connection (EDIR_NONE if not)
	// Returns AreaBlock.filled=false 
	CAreaBlock					GetPlacedBlockDescAt(Vec2i vPos);

	// Returns true is iarea can be placed and linked correctly with existing areas
	bool						IsZoneClear(CInventoryArea* iarea, Vec2i vPos);
	
	// Removes all placed children of specified parent
	void						RemoveChildrenOf(CPlacedArea* parent);
	
	// Removes all areas of generation >= nMinGeneration
	void						RemoveGenerations(int nMinGeneration);
	
	// Returns the inventory idx for specified pointer to inventory area
	int							GetInventoryIdx(CInventoryArea* iarea);
	
	// returns array of all placed areas of specified generation
	std::vector<CPlacedArea*>	GetShuffledPlacedAreas(int nGeneration);

	// Gets a random area that fits the requirements and places it in the level returning reference to it
	CPlacedArea*				PlaceStoryArea(CPlacedArea* parent, CAreaConnector* parentConn, int nGeneration, int nConnectionsMin, int nConnectionsMax, 
												std::wstring strTagsAny = L"", std::wstring strTagsAll = L"", std::wstring strTagsNone = L"");

	// Only adds corridors when children can't be placed
	bool						GenerateWithCorridorsWhenNeeded(int maxDepth);

	// Only adds corridors when children can't be placed
	bool						GenerateFromStory(CMissionStory* story);
};

//--- SINGLETON ---
CMissionGenerator& UTGetMissionGen();
