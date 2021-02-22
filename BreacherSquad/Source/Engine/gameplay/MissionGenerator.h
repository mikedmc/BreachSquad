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
	RECTXYWH AABB;    // world space rectangle in blocks positions
	CAreaSpecs areaSpecs;
	//public Form1.CGridCell[, ] blocks = null;    // blocks in matrix of AABB.w/h
	std::vector<CAreaConnector> arrConnections;

	//public CInventoryArea pInventoryArea; // pointer to inventory area needed to decrease inventory usage when removed. Should be index...
	int nID; // area ID needed for generating from story
	int nGeneration; // generation of placed area

	std::wstring strAreaTags;
	std::wstring strAreaName;

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

	/*
	CPlacedArea(Form1.CAreaDesc area, Point vPos)
	{
		AABB = area.AABB;
		AABB.X = vPos.X; AABB.Y = vPos.Y;
		nGeneration = 0;
		strAreaTags = area.strTags;
		strAreaName = area.strName;
		// allocate blocks
		blocks = new Form1.CGridCell[AABB.Width, AABB.Height];
		for (int yy = 0; yy < AABB.Height; yy++)
		{
			for (int xx = 0; xx < AABB.Width; xx++)
			{
				// copy active area
				blocks[xx, yy] = new Form1.CGridCell();
				blocks[xx, yy].bFilled = area.blocks[area.AABB.X + xx][area.AABB.Y + yy].bFilled;
				blocks[xx, yy].connectionDir = area.blocks[area.AABB.X + xx][area.AABB.Y + yy].connectionDir;
			}
		}
		// copy connectors
		for (int kk = 0; kk < area.arrConnectors.Count; kk++)
		{
			CAreaConnector nc = new CAreaConnector();
			nc.dir = area.blocks[area.arrConnectors[kk].X][area.arrConnectors[kk].Y].connectionDir;
			nc.pConnectedArea = null;
			// bring connector position in placed area space:
			nc.pos = area.arrConnectors[kk];
			nc.pos.X -= area.AABB.X; nc.pos.Y -= area.AABB.Y;

			arrConnections.Add(nc);
		}
	}
	*/

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

	bool IsZoneClear(CInventoryArea* iarea, Vec2i vPos)
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
						CAreaBlock pNeigh = GetPlacedBlockDescAt(Vec2i(xx + vPos.X + vOff.X, yy + vPos.Y + vOff.Y));
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


};


CMissionGenerator& UTGetMissionGen();
