#pragma once

// contents of the inventory
class CInventoryArea
{
public:
	CAreaSpecs			areaSpecs;			// data copied from AreasInventory
	int					nAvailable;			// Number of available areas of this type
};

// Area connectors
class CPlacedArea;
class CAreaConnector
{
public:
	Vec2i				pos;				// position of connector in local space (blocks coords)
	EDir				dir;				// direction of connection (K_DIR_...)
	CPlacedArea*		pConnectedArea;		// pointer to connected area
};

class CPlacedArea
{
public:
	RECTXYWH AABB;    // world space rectangle in blocks positions
	//public Form1.CGridCell[, ] blocks = null;    // blocks in matrix of AABB.w/h
	std::vector<CAreaConnector> arrConnections;

	//public CInventoryArea pInventoryArea; // pointer to inventory area needed to decrease inventory usage when removed. Should be index...
	int nID; // area ID needed for generating from story
	int nGeneration; // generation of placed area

	std::wstring strAreaTags;
	std::wstring strAreaName;

public:
	// returns list of available connectors
	std::vector<CAreaConnector*> get_available_connectors(bool bShuffle)
	{
		std::vector<CAreaConnector*> arrConn;
		for (int ncon = 0; ncon < arrConnections.size(); ncon++)
		{
			// only add not connected connectors
			if (arrConnections[ncon].pConnectedArea == null)
				arrConn.push_back(&arrConnections[ncon]);
		}

		//#TODO: ar trebui sa foloseasca randomul sincronizat in retea. Il poate primi la init.
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
	CRandom				m_rnd;				// RNG

public:
	std::vector<CInventoryArea> arrInventory;
public:
	// Builds the inventory from available areas
	void						BuildInventory();
	// Releases all areas descriptors
	void						Release();
};


CMissionGenerator& UTGetMissionGen();
