#pragma once

///----------------------------------------------------------------------------------
/// each story generation entry is a room 
///----------------------------------------------------------------------------------
class CStoryRoom
{
public:
	
	int					nChildren;			// number of children of the room (connections will be children + 1)
	std::wstring		tags_any;			// any of the tags will add it
	std::wstring		tags_all;			// will add it only if contains ALL tags (doesn't exclude ANY filter)
	std::wstring		tags_none;			// will remove it if it contains any of the tags here

public:	
	bool				bUsed;				// usage flag used while generating the level (used rooms show up as true)
	int					nID;				// ID of room for graph building
	int					nParentID;			// ID of parent for graph building

public:
	CStoryRoom()
	{
		nChildren = 0;
		bUsed = false;
		nID = -1;
		nParentID = -1;
	}
};

///----------------------------------------------------------------------------------
/// stories are composed of multiple generations (graph depth)
///----------------------------------------------------------------------------------
class CStoryGeneration
{
public: 
	std::vector<CStoryRoom>		arrRooms;			// Array of rooms for current generation

	~CStoryGeneration()
	{
		arrRooms.clear();
	}
};

///--------------------------------------------------------------------------
/// MISSION STORY - graph representing the structure of a level
///--------------------------------------------------------------------------
class CMissionStory
{
public: 
	std::vector<CStoryGeneration>		arrGenerations;		// Array of generations in story

public:
	CMissionStory();
	~CMissionStory();

	// Loads a story from file
	OPRESULT					LoadStory(WCHAR* strXMLPath);
	// Releases everything
	void						Release();
	// Tells if story is loaded
	bool						IsLoaded();
	// Returns a StoryRoom for specified nGeneration, with sepcified nParentID
	// Used during level generation, uses bUsed flag
	CStoryRoom*					GetNextAvailableRoom(int nGeneration, int nParentID, bool bMarkAsUsed = true);
	// Computes IDs, parent IDs and trims loose children
	void						ComputeRelationshipsGraph();
	// clear "used" flag from a generation, for a specific parentID
	void						ClearChildEntries(int nGeneration, int nParentID);
	// clear "used" flag fr a full generation and children ones
	void						ClearEntriesFromGeneration(int nGeneration);

};


CAreasInventory& UTGetAreasInv();
