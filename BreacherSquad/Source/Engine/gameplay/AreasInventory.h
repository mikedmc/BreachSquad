#pragma once

// Area specifications loaded from file
// Each area is divided in logical blocks of 8x8 tiles 
// Each block can have "connections" with neighbors. 
// Continuity of inter-areas connectors must be safe-guarded by the level designer
class CAreaSpecs
{
public:
	Vec2i						sizeBL;					// Number of blocks on each side
	std::wstring				strSpecs;				// contains one char for each block (0-empty, 1-filled, LURD-connection direction on filled block)
	std::wstring				strTags;				// area comma separated tags (hall,special,etc)
	std::wstring				strFilename;			// filename of matching level area file

public:
	CAreaSpecs()
	{
		sizeBL.x = sizeBL.y = 0;
	}

	~CAreaSpecs()
	{
	}

	bool GetBlockIsSet(Vec2i pos, EDir & retConnDir)
	{
		retConnDir = EDIR_NONE;
		if ((pos.x < 0) || (pos.y < 0) || (pos.x >= sizeBL.x) || (pos.y >= sizeBL.y))
		{
			return false;
		}
		WCHAR retchar = strSpecs[pos.y * sizeBL.x + pos.x];
		if (retchar == '0')
			return false;
		if (retchar == '1')
			return true;

		if (retchar == 'L')
			retConnDir = EDIR_LEFT;
		else if (retchar == 'U')
			retConnDir = EDIR_UP;
		else if (retchar == 'R')
			retConnDir = EDIR_RIGHT;
		else if (retchar == 'D')
			retConnDir = EDIR_DOWN;

		return true;
	}
};

///--------------------------------------------------------------------------
/// AREAS INVENTORY - keeps track of all level parts (areas)
///  - Provides inventory of parts for the levels generator
///--------------------------------------------------------------------------
class CAreasInventory 
{
public: 
	std::vector<CAreaSpecs>		arrAreas;

public:
	CAreasInventory();
	~CAreasInventory();

	// Loads the available areas specs from the specs file
	// FORMAT: <Area File="area0" BlocksW="2" BlocksH="2" ConnectorsDesc="1U1R" Tags="" />
	OPRESULT					LoadAreasSpecs(WCHAR* strXMLPath);

	// Releases all areas descriptors
	void						Release();

	// Returns a copy of the areas array
	std::vector<CAreaSpecs>		GetAreas();
};


///----------------------------------------------------------------------------------
/// Loads the areas definitions file
///----------------------------------------------------------------------------------
CAreasInventory& UTGetAreasInv();
