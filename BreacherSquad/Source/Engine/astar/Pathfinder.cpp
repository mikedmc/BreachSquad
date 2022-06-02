#include "dxstdafx.h"
#include "Pathfinder.h"
#include <algorithm>

//#define PATHFINDING_PERF

constexpr int MetersToCells(float m) {
	return int(m * CELL_SIZE_METERS_INV);
}

#define HALF_COST	5
#define MIN_COST	(2*HALF_COST)
FORCEINLINE unsigned short GetGCostForParent(short nodex, short nodey, unsigned short parentGCost, short parentx, short parenty)
{
	//NOTE: CRB very important that this is the same or lower than the heuristic when using the sorted open list
	return parentGCost + ((nodex == parentx || nodey == parenty) ? MIN_COST : (MIN_COST + HALF_COST)); 
}

FORCEINLINE unsigned short CalculateH(int nodex, int nodey, int destx, int desty)
{
	//https://www.redblobgames.com/pathfinding/posts/reprioritize.html
	//CRB: Octile distance. Fudge of 1 makes it a consistent heuristic
	//If the heuristic is consistent, when a node is removed from openSet the path to it is guaranteed to be optimal
	//This just means it guarantees optimal paths.
	unsigned short dx = (unsigned short)abs(nodex - destx);
	unsigned short dy = (unsigned short)abs(nodey - desty);
	constexpr unsigned short fudge = 1; //NOTE: CRB fudge 0 gives guaranteed optimal paths, but is slower by about 30%
	return (HALF_COST + fudge) * (max(dx,dy)*2 + min(dx, dy)); // assumes a 1.5 cost for diagonals (works better with integer math)

	// use (MIN_COST*1.5) if we want the heuristic to be a bit forgiving by overestimating a bit. By overestimating heuristics we also have better performance since we don't need the smallest super optimal path.
	// Manhattan distance
	//return MIN_COST * (unsigned short)(dx + dy);
}

#define MIN_COST_UNADM (int)(MIN_COST * 3)
FORCEINLINE unsigned short CalculateH_Unadmissible(int nodex, int nodey, int destx, int desty)
{
	return MIN_COST_UNADM * (unsigned short)(abs(destx - nodex) + abs(desty - nodey));
}

FORCEINLINE unsigned short GetAdditionalCost(const unsigned int nodeData, const unsigned int additionalCostFlags)
{
	if (additionalCostFlags == 0)
		return 0;
	unsigned short extraCost = 0;
	unsigned maskedCost = nodeData & additionalCostFlags;
	extraCost += (unsigned short)((MIN_COST * 2) * bool(maskedCost & (COL_CLEARANCE0 | COL_CLEARANCE1)));
	//DMC commented out: extraCost += (unsigned short)((MIN_COST * 20) * bool(maskedCost & (COL_SCATTER | COL_DANGER_AREA))); //NOTE: CRB adding this costs an extra 7% perf on my PC
	return extraCost;
	// use this when using variable additional costs
	//return (unsigned short)(MIN_COST * 10.0f * (1.0f / (float)Math::GetNumberOfBitsSet(additionalCostFlags)));
}

/*
//DMC: this was already commented out
FORCE_INLINE void UpdatePathfinderCell(Pathfinder::eUpdateType updateType, unsigned int incoming, unsigned int& target)
{
	// mask out the bits we don't use for collisions
	const unsigned int flagsMask = ((_COL_MASK_START - 1) << 1) - 1; // basically sets all lower bits. So 17 0b00010001 -> 31 0b00011111

	if (updateType == Pathfinder::ADD)
	{
		// do not overwrite existing entities?
		//if ((target & clearanceBitsMask) == 0)
		{
			//NOTE: CRB this also replaced special flags in the area
			// entity ID is replaced, flags are added on top of the existing ones
			target = ((incoming | target) & flagsMask) | (incoming & (~flagsMask));
		}
	}
	else
	if (updateType == Pathfinder::REMOVE)
	{
		// only delete if it's us that wrote this cell. But then only delete the ID and our own flags, leave the remaining flags alone (since flags were additively added above)
		if ((target & 0xffff0000) == (incoming & 0xffff0000))
		{
			//NOTE: CRB this also removed special flags in the area
			target &= (~incoming & flagsMask); // target & (~clearanceBitsMask);
		}
	}
}
//*/

FORCEINLINE void UpdatePathfinderCell(const Pathfinder::eUpdateType updateType, const unsigned char incoming, unsigned char& target)
{
	if (updateType == Pathfinder::ADD_LOWORD)
	{
		// entity ID is kept, flags are added on top
		target = (incoming | target);
	}
	else
	if (updateType == Pathfinder::REMOVE_LOWORD)
	{
		target = (target & ~incoming);
	}
}

/*
#define MIPS_DIM(dim)	(((dim) + 7) / 8)
#define MIPS_FROM(x)	((x) / 8)
#define MIPS_TO(x)		((x) * 8)
*/

//////////////////////////////////////////////////////////////////////////

Pathfinder::Pathfinder()
{
	m_width						= 0;
	m_height					= 0;
	m_statusOpen				= 0;
	m_statusClosed				= 1;
	m_nOpenListSize				= 0;
	//m_clearanceValueStartBit	= 0;
	//m_sniperUpdateIdx			= 0;
	memset(m_openlist, 0, sizeof(m_openlist));
}

Pathfinder::~Pathfinder()
{
	Release();
}

float Pathfinder::GetCellSizeMeters() const
{
	return CELL_SIZE_METERS;
}

void Pathfinder::Init(int sourceWidthMeters, int sourceHeightMeters, unsigned char blockMask)
{
	const int newWidth = (int)(sourceWidthMeters / CELL_SIZE_METERS);
	const int newHeight = (int)(sourceHeightMeters / CELL_SIZE_METERS);

	// try to preserve memory
	//if ((newWidth * newHeight) > (m_width * m_height))
	{
		SAFE_DELETE_ARRAY(m_nodeData);
		SAFE_DELETE_ARRAY(m_nodemap);
		//SAFE_DELETE_ARRAY(m_nodeDataMips);
		m_nodeData = new unsigned char [newWidth * newHeight];
		m_nodemap = new PathNode[newWidth * newHeight];
		//m_nodeDataMips = new int[ MIPS_DIM(newWidth) * MIPS_DIM(newHeight)];
	}

	m_width = newWidth;
	m_height = newHeight;
	m_blockMask = blockMask;

	memset(m_nodeData, 0, sizeof(m_nodeData[0]) * m_width * m_height);
	memset(m_nodemap, 0, sizeof(m_nodemap[0]) * m_width * m_height);
	//memset(m_nodeDataMips, 0, sizeof(int) * MIPS_DIM(m_width) * MIPS_DIM(m_height));

	// calculate maximum needed points (estimate)
	//int maxPoints = (int)sqrtf((float)(m_width * m_width + m_height * m_height)) + m_width * 2;
	//if (m_localGetPathBuffer.GetCapacity() < maxPoints)
	//{
	//	m_localGetPathBuffer.Resize(maxPoints);
	//}

	//UPDATE FLAGS: fill with border blocker value
	for ( int i = 0; i < m_width; ++i )
		m_nodeData[i] = m_blockMask; // top row

	for ( int i = 0; i < m_width; ++i )
		m_nodeData[m_width * ( m_height - 1 ) + i] = m_blockMask; // bottom row

	for ( int i = 0; i < m_height; ++i )
	{
		m_nodeData[m_width * i] = m_blockMask; // left column
		m_nodeData[m_width * ( i + 1 ) - 1] = m_blockMask; // right column
	}

	// initialize positions in the nodemap
	for ( int y = 0; y < m_height; ++y )
	{
		int index = y * m_width;
		for ( int x = 0; x < m_width; ++x, ++index )
		{
			PathNode* pNode = &m_nodemap[index];
			pNode->x = ( short ) x;
			pNode->y = ( short ) y;
		}
	}
}

void Pathfinder::Release()
{
	SAFE_DELETE_ARRAY( m_nodeData );
	SAFE_DELETE_ARRAY( m_nodemap );
	//SAFE_DELETE_ARRAY(m_nodeDataMips);
	//m_localGetPathBuffer.Free();
}

void Pathfinder::ComputeClearance(/*unsigned int clearanceValueStartBit, int numClearanceValues*/)
{
	//m_suspiciousAreasUpdated = true;

	//m_clearanceValueStartBit = clearanceValueStartBit; // save for later
	//_ASSERT(m_clearanceValueStartBit == 0 || m_clearanceValueStartBit == 8192); // hack warning. Make sure to edit UpdatePathfinderCell() if this changes, it makes the assumption that we only use up to 8192 bits for flags


	// DMC: next part automatically calculates clearance flags (around objects)

//	if (!numClearanceValues || !clearanceValueStartBit)
//		return;
//
//
//	// hardcoded for 2 levels
//	_ASSERT(numClearanceValues == 2);
//	unsigned int clearanceBits[2] = { clearanceValueStartBit, clearanceValueStartBit << 1 };// , clearanceValueStartBit << 2};
//
//	for(int y = 0; y < m_height; y++)
//	{
//		for (int x = 0; x < m_width; ++x)
//		{
//			unsigned* pData = &m_nodeData[x + y * m_width];
//			const unsigned int data = *pData;
//			if (data & m_blockMask)
//				continue;
//
//			//
//			// clearance level 0 - this is a free cell. Same as non-blocking cell (no movement block). We only need it as an extra cost when trying to avoid the shortest path.
//			*pData = data | clearanceBits[0];
//
//			//
//			// clearance level 1
//			bool bClear1 = true;
//			const Vec2i lvl2[] = {
//				{x-1, y-1},	{x  , y-1},	{x+1, y-1},
//				{x-1, y  },				{x+1, y  },
//				{x-1, y+1},	{x  , y+1},	{x+1, y+1},
//			};
//			for (int i = 0; i < ARRAY_SIZE(lvl2) && bClear1; ++i)
//			{
//				_ASSERT(lvl2[i].x >= 0 && lvl2[i].x < m_width && lvl2[i].y >= 0 && lvl2[i].y < m_height); // with the map edges blocked out, there's no way we can get outside the map
//				bClear1 &= (m_nodeData[lvl2[i].y * m_width + lvl2[i].x] & m_blockMask) == 0;
//			}
//
//			if (bClear1)
//				*pData = data | clearanceBits[1];
//			else
//				continue;
//
//			//
//			// clearance level 2
//			bool bClear2 = true;
//			const Vec2i lvl3[] = {
//				{x-2, y-2},	{x-1, y-2},	{x  , y-2},	{x+1, y-2},	{x+2, y-2},
//				{x-2, y-1},	{x-1, y-1},	{x  , y-1},	{x+1, y-1},	{x+2, y-1},
//				{x-2, y  },	{x-1, y  },				{x+1, y  },	{x+2, y  },
//				{x-2, y+1},	{x-1, y+1},	{x  , y+1},	{x+1, y+1},	{x+2, y+1},
//				{x-2, y+2},	{x-1, y+2},	{x  , y+2},	{x+1, y+2},	{x+2, y+2},
//			};
//			for (int i = 0; i < ARRAY_SIZE(lvl3) && bClear2; ++i)
//			{
//				_ASSERT(lvl3[i].x >= 0 && lvl3[i].x < m_width && lvl3[i].y >= 0 && lvl3[i].y < m_height); // with the map edges blocked out, there's no way we can get outside the map
//				bClear2 &= (m_nodeData[lvl3[i].y * m_width + lvl3[i].x] & m_blockMask) == 0;
//			}
//
//			//if (bClear2)
//			//	pNode->data = data | clearanceBits[2];
//			//else
//			//	continue;
//
//			// this will just clear up the previous cells, meaning we leave spaces that have more than 'numClearanceValues' free around them with no extra cost
//			if (bClear2)
//				*pData = data; // reset
//
// /*
//			// clearance level 3 - this will just clear up the previous cells DMC: already commented out
//			bool bClear3 = true;
//			const Vec2i lvl4[] = {
//				{x-3, y-3},	{x-2, y-3},	{x-1, y-3},	{x  , y-3},	{x+1, y-3},	{x+2, y-3},	{x+3, y-3},
//				{x-3, y-2},	{x-2, y-2},	{x-1, y-2},	{x  , y-2},	{x+1, y-2},	{x+2, y-2},	{x+3, y-2},
//				{x-3, y-1},	{x-2, y-1},	{x-1, y-1},	{x  , y-1},	{x+1, y-1},	{x+2, y-1},	{x+3, y-1},
//				{x-3, y  },	{x-2, y  },	{x-1, y  },				{x+1, y  },	{x+2, y  },	{x+3, y  },
//				{x-3, y+1},	{x-2, y+1},	{x-1, y+1},	{x  , y+1},	{x+1, y+1},	{x+2, y+1},	{x+3, y+1},
//				{x-3, y+2},	{x-2, y+2},	{x-1, y+2},	{x  , y+2},	{x+1, y+2},	{x+2, y+2},	{x+3, y+2},
//				{x-3, y+3},	{x-2, y+3},	{x-1, y+3},	{x  , y+3},	{x+1, y+3},	{x+2, y+3},	{x+3, y+3},
//			};
//			for (int i = 0; i < COUNT_OF(lvl4) && bClear3; ++i)
//			{
//				ASSERT(lvl4[i].x >= 0 && lvl4[i].x < m_width && lvl4[i].y >= 0 && lvl4[i].y < m_height); // with the map edges blocked out, there's no way we can get outside the map
//				bClear3 &= (m_nodeData[lvl4[i].y * m_width + lvl4[i].x] & m_blockMask) == 0;
//			}
//
//			if (bClear3)
//				pNode->data = data; // reset
// */
//		}
//	}
}

void Pathfinder::SetNodeFlags( int xTL, int yTL, unsigned char flag )
{
	_ASSERT( xTL >= 0 && xTL < m_width && yTL >= 0 && yTL < m_height );
	m_nodeData[xTL + yTL * m_width] |= flag;
}

void Pathfinder::ClearNodeFlags( int xTL, int yTL, unsigned char flag )
{
	_ASSERT( xTL >= 0 && xTL < m_width && yTL >= 0 && yTL < m_height );
	m_nodeData[xTL + yTL * m_width] &= ~flag;
}

/*
namespace AI {
	extern bool CanSniperSeeCommon(Vec3 from, Vec2 to);
};
void Pathfinder::InitSniperLOS(Vec3 sniperPos)
{
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; ++x)
		{
			auto pNode = &m_nodeData[x + y * m_width];
			if (*pNode & m_blockMask)
				continue;

			//TODO: This won't work with multiple snipers
			Vec2 nodePos = ConvertToWorldCoords(x, y).GetXZ();
			if (!AI::CanSniperSeeCommon(sniperPos, nodePos))
				*pNode |= COL_NO_SNIPER_LOS;
		}
	}
	UpdateMips(0, 0, m_width, m_height);
}

void Pathfinder::UpdateSniperLOS(Vec3 sniperPos)
{
	int mipWidth = MIPS_DIM(m_width);
	int mipHeight = MIPS_DIM(m_height);
	m_sniperUpdateIdx = (m_sniperUpdateIdx + 1) % (mipWidth * mipHeight);
	int mx = m_sniperUpdateIdx % mipWidth;
	int my = (m_sniperUpdateIdx / mipWidth) % mipHeight;
	int sx = MIPS_TO(mx);
	int sy = MIPS_TO(my);
	int sxm = Min(MIPS_TO(mx + 1), m_width);
	int sym = Min(MIPS_TO(my + 1), m_height);

	for (int y = sy; y < sym; y++)
	{
		for (int x = sx; x < sxm; ++x)
		{
			ASSERT((y*m_width + x) < m_width * m_height);
			auto pNode = &m_nodeData[x + y * m_width];
			if (*pNode & m_blockMask)
				continue;

			//TODO: This won't work with multiple snipers
			Vec2 nodePos = ConvertToWorldCoords(x, y).GetXZ();
			if (!AI::CanSniperSeeCommon(sniperPos, nodePos))
				*pNode |= COL_NO_SNIPER_LOS;
			else 
				*pNode &= ~COL_NO_SNIPER_LOS;
		}
	}
	UpdateMips(mx, my, mx+1, my+1);
}
*/
/*
void Pathfinder::MarkShapeCells(const Vec3* shapePts, int numPts, unsigned int blockMask)
{
	if (!shapePts || numPts == 0)
		return;

	Vec3 min,max;
	Math::ComputeAABB(shapePts, numPts, min, max);
	Vec2i start = ConvertToPathfinderCoords(min.x, min.z);
	Vec2i end = ConvertToPathfinderCoords(max.x, max.z);
	start.x = Clamp(0, m_width - 1, start.x);
	start.y = Clamp(0, m_height - 1, start.y);
	end.x = Clamp(0, m_width - 1, end.x);
	end.y = Clamp(0, m_height - 1, end.y);

	for (int y = start.y; y < end.y; y++)
	{
		for (int x = start.x; x < end.x; x++)
		{
			if (Math::PointInPoly_XZ(ConvertToWorldCoords(x, y), shapePts, numPts))
				m_nodeData[x + y * m_width] |= blockMask;
		}
	}
}
*/
void Pathfinder::MarkAsAccessible(Vec2 pos)
{
	auto posi = ConvertToPathfinderCoords(pos.x, pos.y);
	if (!IsInsideMap(posi) || (GetRawData(posi.x, posi.y) & COL_MOVEMENT_BLOCK))
		return;

	m_nodeData[posi.x + posi.y * m_width] |= COL_ACCESSIBLE;
}

/*
void Pathfinder::FloodfillAccessibleMask()
{
	struct P {
		unsigned short x;
		unsigned short y;
	};
	List<P> toVisit;
	toVisit.Reserve(1024);
	for (int y = 0; y < m_height; y++)
		for (int x = 0; x < m_width; x++)
			if (m_nodeData[y* m_width + x] & COL_ACCESSIBLE) {
				toVisit.Add({ (unsigned short)x, (unsigned short)y });
				break;
			}

	while (toVisit.GetNumElements()) {
		auto x = toVisit[0].x;
		auto y = toVisit[0].y;
		toVisit.Remove(0);
		if (x != 0				&& !(m_nodeData[y*m_width + x - 1] & (COL_ACCESSIBLE | COL_MOVEMENT_BLOCK))) {
			ASSERT(!(m_nodeData[y*m_width + x - 1] & COL_MOVEMENT_BLOCK));
			m_nodeData[y*m_width + x - 1] |= COL_ACCESSIBLE;
			toVisit.Add({ (unsigned short)(x - 1), y });
		}
		if (x != (m_width-1)	&& !(m_nodeData[y*m_width + x + 1] & (COL_ACCESSIBLE | COL_MOVEMENT_BLOCK))) {
			m_nodeData[y*m_width + x + 1] |= COL_ACCESSIBLE;
			ASSERT(!(m_nodeData[y*m_width + x + 1] & COL_MOVEMENT_BLOCK));
			toVisit.Add({ (unsigned short)(x + 1), y });
		}
		if (y != 0				&& !(m_nodeData[(y - 1)*m_width + x] & (COL_ACCESSIBLE | COL_MOVEMENT_BLOCK))) {
			m_nodeData[(y -1)*m_width + x] |= COL_ACCESSIBLE;
			toVisit.Add({ x, (unsigned short)(y - 1) });
		}
		if (y != (m_height-1)	&& !(m_nodeData[(y + 1)*m_width + x] & (COL_ACCESSIBLE | COL_MOVEMENT_BLOCK))) {
			m_nodeData[(y + 1)*m_width + x] |= COL_ACCESSIBLE;
			toVisit.Add({ x, (unsigned short)(y + 1) });
		}
	}
}
*/

//NOTE: CRB This takes 0.008 to 0.010 ms on DEBUG on my PC. 0.002 on Release
// Don't bother optimising, costs nothing
/*
void Pathfinder::UpdateFOVCircleArea(Vec2 pos, float radius, unsigned blockMask, unsigned writeMask, eUpdateType update)
{
	//uint64_t timeBefore = OS_GetTimeMicroSec();

	const Vec2i startP = ConvertToPathfinderCoords(pos.x, pos.y);
	if (!IsInsideMap(startP))
		return;

	for (float angle = 0.f; angle < Math::HALF_PI; angle += Math::HALF_PI * 0.1f)
	{
		Vec2 dir = Math::AngleToVector_Radians(angle) * radius;
		auto endP = ConvertToPathfinderCoords(pos.x + dir.x, pos.y + dir.y);
		endP.x = Clamp(1, m_width - 1, endP.x);
		endP.y = Clamp(1, m_height - 1, endP.y);
		WriteFatBresenhamLine(startP, endP, blockMask, writeMask, update);
		endP = ConvertToPathfinderCoords(pos.x - dir.x, pos.y + dir.y);
		endP.x = Clamp(1, m_width - 1, endP.x);
		endP.y = Clamp(1, m_height - 1, endP.y);
		WriteFatBresenhamLine(startP, endP, blockMask, writeMask, update);
		endP = ConvertToPathfinderCoords(pos.x + dir.x, pos.y - dir.y);
		endP.x = Clamp(1, m_width - 1, endP.x);
		endP.y = Clamp(1, m_height - 1, endP.y);
		WriteFatBresenhamLine(startP, endP, blockMask, writeMask, update);
		endP = ConvertToPathfinderCoords(pos.x - dir.x, pos.y - dir.y);
		endP.x = Clamp(1, m_width - 1, endP.x);
		endP.y = Clamp(1, m_height - 1, endP.y);
		WriteFatBresenhamLine(startP, endP, blockMask, writeMask, update);
	}

	//float numMS = (OS_GetTimeMicroSec() - timeBefore) * 0.001f;
	//LOG("[CRB] Mark took %.3f ms\n", numMS);
}
*/


//unsigned char* Pathfinder::GetPathfinderMipsRGBA(unsigned /*mask*/) const
//{
//	return (unsigned char*)m_nodeDataMips;
//}
//
//unsigned char* Pathfinder::GetPathfinderDataRGBA(unsigned mask) const
//{
//	unsigned char* pData = new unsigned char[m_width * m_height * 4];
//	int pixel = 0;
//	for(int y = 0; y < m_height; y++)
//	{
//		for (int x = 0; x < m_width; ++x)
//		{
//			auto val = m_nodeData[x + y * m_width];
//			unsigned int pf = (mask == unsigned(-1)) ? val : ((val & mask) ? 0xffff00ff : 0);
//			pData[pixel+0] = (unsigned char)(pf & 0xff);
//			pData[pixel+1] = (unsigned char)((pf >> 8) & 0xff);
//			pData[pixel+2] = (unsigned char)((pf >> 16) & 0xff);
//			pData[pixel+3] = (unsigned char)((pf >> 24) & 0xff);
//			pData += 4;
//		}
//	}
//	pData -= m_width * m_height * 4;
//	return pData; // needs to be flipped if we want to save as tga
//}
//
//unsigned char* Pathfinder::GetPathfinderDataR(unsigned mask) const
//{
//	unsigned char* pRet = new unsigned char[m_width * m_height];
//	unsigned char* pData = pRet;
//	for (int y = 0; y < m_height; y++)
//	{
//		for (int x = 0; x < m_width; ++x)
//		{
//			auto val = m_nodeData[x + y * m_width];
//			*pData = (mask & val) ? 255 : 0;
//			pData++;
//		}
//	}
//	return pRet;
//}
//
//unsigned char* Pathfinder::GetPathfinderLastSearchRGBA() const
//{
//	int bestNodeIndex = -1;
//	unsigned int mincost = 0xffffffff;
//	for (int i = 0; i < m_nOpenListSize; i++)
//	{
//		if (m_openlist[i].cost < mincost)
//		{
//			bestNodeIndex = m_openlist[i].idx;
//			mincost = m_openlist[i].cost;
//		}
//	}
//
//	unsigned char* pData = new unsigned char[m_width * m_height * 4];
//	int pixel = 0;
//	for (int y = 0; y < m_height; y++)
//	{
//		for (int x = 0; x < m_width; ++x)
//		{
//			int status = m_nodemap[x + y * m_width].status;
//
//			// ABGR
//			unsigned val = status == m_statusOpen ? 0xFF00FFFF : 0xFFAA0000;
//			if (bestNodeIndex == (x + y * m_width))
//				val = 0xFFFFFFFF;
//			if (status != m_statusOpen && status != m_statusClosed)
//				val = 0;
//			//unsigned int pf = (mask == unsigned(-1)) ? val : ((val & mask) ? 0xffff00ff : 0);
//			pData[pixel + 0] = (unsigned char)(val & 0xff);
//			pData[pixel + 1] = (unsigned char)((val >> 8) & 0xff);
//			pData[pixel + 2] = (unsigned char)((val >> 16) & 0xff);
//			pData[pixel + 3] = (unsigned char)((val >> 24) & 0xff);
//			pData += 4;
//		}
//	}
//	pData -= m_width * m_height * 4;
//	return pData; // needs to be flipped if we want to save as tga
//}

/*
A potential issue comes from having overlapped objects, for example a table and a wall (though it could be any number of objects overlapping).
If we have to remove that object later (because it gets destroyed), we shouldn't remove the part that is also covered by a wall.
A simple solution would be to keep the entity ID stored in the upper part of the pathfinder unsigned int array. Only the object which was last written would show up in the list, but we will make sure to add the walls in last.
When deleting an object, we would check which cells it touches and only remove it from those cells that have its ID.

Another solution would be to have a separate array of lists, keeping entity IDs: List<unsigned int> cells[width][height], which would only be accessed when writing/deleting objects,
	but I don't think there's a need for this (wasn't a problem in DK1).
*/
//void Pathfinder::UpdateObject(const sCollisionShape& collision, const Matrix& parentTransform, unsigned int flags, eUpdateType update)
//{
//	switch (collision.type)
//	{
//		case sCollisionShape::SPHERE:
//		{
//			float radius = collision.params.sphere.radius;
//			Vec3 origin = (parentTransform * collision.transform).GetTransVec();
//			UpdateSphere(radius, origin, flags, update);
//		}break;
//
//		case sCollisionShape::CAPSULE:
//		case sCollisionShape::BOX:
//		case sCollisionShape::CONVEX_MESH:
//		{
//			int numOutlineVerts = 0;
//			Vec2 outline[128];
//			collision.GetProjectionOutline(parentTransform, outline, COUNT_OF(outline), &numOutlineVerts, OBJECT_EXPANSION);
//			UpdatePoly(outline, numOutlineVerts, flags, update);
//		}break;
//
//		default:
//			DEBUG_BREAK();
//			break;
//	}
//}


//static FORCEINLINE bool PtInCone(Vec2 p, Vec2 s, Vec2 d, float width)
//{
//	Vec2 dirToHuman = p - s;
//	dirToHuman.Normalize();
//	float angle = dirToHuman * d;
//	return (angle >= width);
//}
//
//void Pathfinder::UpdateCone(Vec2 s, Vec2 d, float coneWidth, unsigned blockFlags, unsigned flags, eUpdateType add)
//{
//	const Vec2 fstart = ConvertToWorldCoords(1, 1).GetXZ();
//	const Vec2i ps = ConvertToPathfinderCoords(s.x, s.y);
//	if (!IsInsideMap(ps))
//		return;
//
//	// brute-force, ~100x slower than the version below
//	/*
//	float fy = fstart.y;
//	for (int y = 1; y < m_height - 1; y++, fy += CELL_SIZE_METERS) {
//		float fx = fstart.x;
//		for (int x = 1; x < m_width - 1; x++, fx += CELL_SIZE_METERS)
//		{
//			Vec2 p = { fx, fy };//ConvertToWorldCoords(x, y).GetXZ();
//			if (m_nodeData[x + y * m_width] & blockFlags)
//				continue;
//			if (!PtInCone( p, s, d, coneWidth))
//				continue;
//
//			if (!TraceBresenhamLine(ps, {x,y}, blockFlags))
//				m_nodeData[x + y * m_width] |= flags;
//		}
//	}
//	//*/
//
//	//*
//	// draw lines from cone start towards the map edge
//	//CRB: 760 cones in a 100x100m map took 6ms on my PC
//	//NOTE: We write a 1-cell wider line so that it's conservative and we don't have false negatives
//	float fy, fx;
//	fy = fstart.y;
//	for (int y = 1; y < m_height - 1; y++, fy += CELL_SIZE_METERS)
//	{
//		fx = fstart.x;
//		int x = 1;
//		if (PtInCone({ fx, fy }, s, d, coneWidth))
//			WriteFatBresenhamLine(ps, { x,y }, blockFlags, flags, add);
//		fx = fstart.x + CELL_SIZE_METERS * (m_width - 1);
//		x = m_width - 2;
//		if (PtInCone({ fx, fy }, s, d, coneWidth))
//			WriteFatBresenhamLine(ps, { x,y }, blockFlags, flags, add);
//	}
//	fx = fstart.x;
//	for (int x = 1; x < m_width - 1; x++, fx += CELL_SIZE_METERS)
//	{
//		fy = fstart.y;
//		int y = 1;
//		if (PtInCone({ fx, fy }, s, d, coneWidth))
//			WriteFatBresenhamLine(ps, { x,y }, blockFlags, flags, add);
//		fy = fstart.y + CELL_SIZE_METERS * (m_height - 1);
//		y = m_height - 2;
//		if (PtInCone({ fx, fy }, s, d, coneWidth))
//			WriteFatBresenhamLine(ps, { x,y }, blockFlags, flags, add);
//	}
//	//*/
//}

static FORCEINLINE bool IsEdge(int x, int y, int w, int h)
{
	return x == 0 || x == (w - 1) || y == 0 || y == (h - 1);
}

//void Pathfinder::UpdateSphere(const float radius, const Vec3& origin, unsigned int flags, eUpdateType update)
//{
//	Vec2i pfBoxAABB[2] =
//	{
//		ConvertToPathfinderCoords(origin.x - radius - OBJECT_EXPANSION, origin.z - radius - OBJECT_EXPANSION),
//		ConvertToPathfinderCoords(origin.x + radius + OBJECT_EXPANSION, origin.z + radius + OBJECT_EXPANSION),
//	};
//
//	// clamp to inside the map
//	for (int i = 0; i < 2; ++i)
//	{
//		pfBoxAABB[i].x = Clamp(1, m_width - 2, pfBoxAABB[i].x);
//		pfBoxAABB[i].y = Clamp(1, m_height - 2, pfBoxAABB[i].y);
//	}
//
//	Vec2i pfOrigin = ConvertToPathfinderCoords(origin.x, origin.z);
//	Vec2 fBox = ConvertToWorldCoords(pfBoxAABB[0].x, pfBoxAABB[0].y).GetXZ();
//	Vec2 fOrigin = origin.GetXZ();
//	const float limitSq = (radius + CELL_RADIUS) * (radius + CELL_RADIUS);
//
//	// update collision map area related to the box, in the pathfinder
//	float fy = fBox.y, fx;
//	for(int y = pfBoxAABB[0].y; y <= pfBoxAABB[1].y; y++, fy += CELL_SIZE_METERS)
//	{
//		fx = fBox.x;
//		for (int x = pfBoxAABB[0].x; x <= pfBoxAABB[1].x; ++x, fx += CELL_SIZE_METERS)
//		{
//			if (x != pfOrigin.x && y != pfOrigin.y)
//			{
//				float dist = Vec2::DistanceSq(fOrigin, { fx, fy });
//				if (dist >= limitSq)
//					continue;
//			}
//
//			UpdatePathfinderCell(update, flags, m_nodeData[x + y * m_width]);
//		}
//	}
//}

// used for box/capsule/mesh
//void Pathfinder::UpdatePoly(const Vec2* pVerts, const int numVerts, unsigned int flags, eUpdateType update)
//{
//	Vec2 bbox[2];
//	Math::ComputeAABB(pVerts, numVerts, bbox[0], bbox[1]);
//	Vec2i bboxi[2];
//	for (int i = 0; i < 2; ++i)
//	{
//		bboxi[i] = ConvertToPathfinderCoords(bbox[i].x, bbox[i].y);
//		bboxi[i].x = Clamp(1, m_width - 2, bboxi[i].x);
//		bboxi[i].y = Clamp(1, m_height - 2, bboxi[i].y);
//	}
//
//	// update collision map area related to the box, in the pathfinder
//	for(int y = bboxi[0].y; y <= bboxi[1].y; ++y)
//	{
//		for (int x = bboxi[0].x; x <= bboxi[1].x; ++x)
//		{
//			Vec3 center = ConvertToWorldCoords(x, y);
//			if (!Math::PointInPoly(Vec2(center.x, center.z), pVerts, numVerts))
//			{
//				// cell's center point was not inside the poly, but maybe we touch the corners...
//
//				const float radius = CELL_SIZE_METERS * 0.5f - 0.0001f; // offset removes collinearity, which would unnecessarily expand the border too much
//				Vec2 cellBB[] = {
//					{center.x - radius, center.z - radius},
//					{center.x + radius, center.z + radius},
//				};
//
//				bool bCrossed = false;
//				for (int i = 0; i < numVerts && !bCrossed; ++i)
//				{
//					if (Math::SegmentVsAABB(pVerts[i], pVerts[(i + 1) % numVerts], cellBB[0], cellBB[1]))
//						bCrossed = true;
//				}
//
//				if (!bCrossed)
//					continue;
//			}
//
//			UpdatePathfinderCell(update, flags, m_nodeData[x + y * m_width]);
//		}
//	}
//}
//
//void Pathfinder::UpdateMips(int xStart, int yStart, int xEnd, int yEnd)
//{
//	int mx0 = MIPS_FROM(xStart);
//	int my0 = MIPS_FROM(yStart);
//	int mx1 = MIPS_FROM(xEnd);
//	int my1 = MIPS_FROM(yEnd);
//	int mipWidth = MIPS_DIM(m_width);
//	//int mipHeight = MIPS_DIM(m_height);
//	for (int my = my0; my < my1; my++)
//		for (int mx = mx0; mx < mx1; mx++) {
//			int mipData = 0;
//			int sx = MIPS_TO(mx);
//			int sy = MIPS_TO(my);
//			int ex = Min(MIPS_TO(mx + 1), m_width);
//			int ey = Min(MIPS_TO(my + 1), m_height);
//
//			for (int y = sy; y < ey; y++)
//				for (int x = sx; x < ex; x++)
//					mipData |= m_nodeData[y * m_width + x];
//
//			m_nodeDataMips[ my*mipWidth + mx ] = mipData;
//		}
//}
//
//bool Pathfinder::LineHitsAny(Vec2 start, Vec2 end, unsigned blockFlags) const
//{
//	Vec2i starti = ConvertToPathfinderCoords(start.x, start.y);
//	Vec2i endi = ConvertToPathfinderCoords(end.x, end.y);
//
//	if (!IsInsideMap(starti))
//	{
//		DEBUG_BREAK();
//		return false;
//	}
//
//	if (!IsInsideMap(endi))
//	{
//		// Note: this is already clamped to inside the map when function is called, but it has failed in the past, due to unexplained error in AdjustToInsideMap(), see comment there
//		DEBUG_BREAK();
//		return false;
//	}
//
//	return TraceBresenhamLine(starti, endi, blockFlags);
//}
//
//bool Pathfinder::TraceLine(Vec2 start, Vec2 end, unsigned blockFlags, Vec2& outWS) const
//{
//	Vec2i starti = ConvertToPathfinderCoords(start.x, start.y);
//	Vec2i endi = ConvertToPathfinderCoords(end.x, end.y);
//	if (!IsInsideMap(starti))
//	{
//		DEBUG_BREAK();
//		return false;
//	}
//	if (!IsInsideMap(endi))
//	{
//		DEBUG_BREAK();
//		return false;
//	}
//
//	Vec2i out;
//	const bool hit = TraceBresenhamLine(starti, endi, blockFlags, &out);
//	if(hit)
//		outWS = ConvertToWorldCoords(out.x, out.y).GetXZ();
//	return hit;
//}
//
//bool Pathfinder::TraceLineBlocked(Vec2 start, Vec2 end, unsigned blockFlags, Vec2& outWS) const
//{
//	Vec2i starti = ConvertToPathfinderCoords(start.x, start.y);
//	Vec2i endi = ConvertToPathfinderCoords(end.x, end.y);
//	if (!IsInsideMap(starti))
//	{
//		DEBUG_BREAK();
//		return false;
//	}
//	if (!IsInsideMap(endi))
//	{
//		DEBUG_BREAK();
//		return false;
//	}
//
//	Vec2i out;
//	const bool hit = TraceBresenhamLineBlocked(starti, endi, blockFlags, &out);
//	if (hit)
//		outWS = ConvertToWorldCoords(out.x, out.y).GetXZ();
//	return hit;
//}
//
//bool Pathfinder::IsPointClear(Vec2 pos, unsigned blockFlags) const
//{
//	unsigned data = GetRawData_Safe(pos.ToX0Y());
//	return (data & blockFlags) == 0;
//}

FORCEINLINE int spiralIdx2offset(int radius, int i)
{
	int h = 2 << radius;
	int hm = h - 1;
	int xo = 1 << radius;
	int dy = i & hm;
	dy = dy <= xo ? dy : (h - dy);
	dy = dy > radius ? radius : dy;
	return i < h ? dy : -dy;
}

struct NearestIdx
{
	int width, height;
	int i = 0;
	int l = 0;
	int endl;

	NearestIdx(int _width, int _height) {
		width = _width, height = _height;
		endl = 3;
	}

	operator bool() {
		i++;
		int maxI = 8 * l;
		if (i >= maxI || l == 0)
		{
			l++;
			i = 0;
		}
		return l < endl;
	}

	void setRange(int cellRadius) { endl = cellRadius + 1; }

	bool getIdx(int sx, int sy, int pd, int& px, int& py) const
	{
		int w = 4 << l;
		int wm = w - 1;
		int xo = 1 << l;


		int si = pd * xo;// start index
		// starting from preferred direction, search in alternating left/right
		int ii = ((i + 1) / 2) * ((i % 2) == 0 ? -1 : 1);
		ii = (w + si + ii) & wm;

		int dx = spiralIdx2offset(l, (w + (ii - xo)) & wm);
		int dy = spiralIdx2offset(l, ii);

		px = sx + dx;
		py = sy + dy;
		return (px >= 0 && px < width && py >= 0 && py < height);
	}
};

//helper for indexing in a 2D spiral pattern
struct SpiralIdx
{
	int w, h;
	int x = 0;
	int y = 0;
	int dx = 0;
	int dy = -1;
	int i = 0;
	int maxI;

	SpiralIdx(int width, int height) {
		w = width, h = height;
		maxI = w * h * 4;
	}

	operator bool() {
		i++;
		if ((x == y) || ((x < 0) && (x == -y)) || ((x > 0) && (x == 1 - y)))
		{
			int t = dx;
			dx = -dy;
			dy = t;
		}

		x += dx;
		y += dy;

		return i < maxI;
	}

	void setRange(int cellRadius) { maxI = cellRadius * cellRadius * 4; }

	bool getIdx(int sx, int sy, int& px, int& py) const
	{
		px = sx + x;
		py = sy + y;
		return (px >= 0 && px < w && py >= 0 && py < h);
	}
};
/*
bool Pathfinder::ClosestCell(Vec2& out, Vec2 pos, unsigned blockFlags)
{
	Vec2i start = ConvertToPathfinderCoords(pos.x, pos.y);

	// use mips first
	int mx = MIPS_FROM(start.x), my = MIPS_FROM(start.y);
	int mw = MIPS_DIM(m_width);
	int mh = MIPS_DIM(m_height);
	SpiralIdx spiralMip{ mw, mh};
	do {
		int px, py;
		if (spiralMip.getIdx(mx, my, px, py) &&
			(m_nodeDataMips[py * mw + px] & blockFlags) != 0)
		{
			mx = px; my = py;
			break;
		}
	} while (spiralMip);
	if (!spiralMip)
		return false; // no cell matched

	// look for nearest cell in mip
	int cx = Clamp(MIPS_TO(mx), MIPS_TO(mx + 1) - 1, start.x);
	int cy = Clamp(MIPS_TO(my), MIPS_TO(my + 1) - 1, start.y);
	mx = MIPS_TO(mx); my = MIPS_TO(my);
	cx -= mx; cy -= my;
	SpiralIdx spiralCell{ 8, 8 };
	do {
		int px, py;
		if ( spiralCell.getIdx(cx, cy, px, py) && 
			(m_nodeData[(my + py) * m_width + (mx + px)] & blockFlags) != 0)
		{
			out = ConvertToWorldCoords(mx + px, my + py).GetXZ();
			return true;
		}
	} while (spiralCell);

	return false;
}

bool Pathfinder::ClosestBlockedCellOutsideRadius(Vec2& out, Vec2 pos, unsigned blockFlags, const Vec2* occupiedPos, float occupiedRadius, int count) const
{
	Vec2i start = ConvertToPathfinderCoords(pos.x, pos.y);
	// look for nearest cell in mip
	SpiralIdx spiralCell{ m_width, m_height };
	spiralCell.setRange(MetersToCells(3.f)); // look in 3m range
	do {
		int px, py;
		if (spiralCell.getIdx(start.x, start.y, px, py) &&
			(m_nodeData[py * m_width + px] & blockFlags) != 0)
		{
			Vec2 p = ConvertToWorldCoords( px, py).GetXZ();
			bool occupied = false;
			for (int i = 0; i < count; i++)
				if (Vec2::DistanceSq(occupiedPos[i], p) < occupiedRadius*occupiedRadius) {
					occupied = true;
					break;
				}
			if (!occupied) {
				out = p;
				return true;
			}
		}
	} while (spiralCell);

	return false;
}

bool Pathfinder::ClosestFreeCellOutsideRadius(Vec2& out, Vec2 pos, unsigned blockFlags, const Vec2* occupiedPos, float occupiedRadius, int count) const
{
	Vec2i start = ConvertToPathfinderCoords(pos.x, pos.y);
	// look for nearest cell in mip
	SpiralIdx spiralCell{ m_width, m_height };
	spiralCell.setRange(MetersToCells(3.f)); // look in 3m range
	do {
		int px, py;
		if (spiralCell.getIdx(start.x, start.y, px, py) &&
			(m_nodeData[py * m_width + px] & blockFlags) == 0)
		{
			Vec2 p = ConvertToWorldCoords(px, py).GetXZ();
			bool occupied = false;
			for (int i = 0; i < count; i++)
				if (Vec2::DistanceSq(occupiedPos[i], p) < occupiedRadius*occupiedRadius) {
					occupied = true;
					break;
				}
			if (!occupied) {
				out = p;
				return true;
			}
		}
	} while (spiralCell);

	return false;
}

bool Pathfinder::ClosestEmptyCell(Vec2& out, Vec2 pos, unsigned blockFlags) const
{
	const Vec2i pf = ConvertToPathfinderCoords(pos.x, pos.y);
	
	//nearest tile direction
	int bestDir = 0;
	Vec2 dp = pos - ConvertToWorldCoords(pf.x, pf.y).GetXZ();
	if (abs(dp.x) > abs(dp.y))
		if (dp.x < 0.f)
			bestDir = 0;
		else
			bestDir = 2;
	else
		if (dp.y < 0.f)
			bestDir = 3;
		else
			bestDir = 1;

	// searches in a spiral
	NearestIdx spiral{ m_width, m_height };
	spiral.setRange( MetersToCells(1.6f));//NOTE: keep this bigger than 0.6m so sitting enemies find a new position
	do {
		int px, py;
		if (spiral.getIdx(pf.x, pf.y, bestDir, px, py)
			&& ((m_nodeData[py * m_width + px] & blockFlags) == 0))
		{
			out = ConvertToWorldCoords(px, py).GetXZ();
			return true;
		}
	} while (spiral);

	return false;
}
*/
unsigned int Pathfinder::GetRawData_Safe(Vec3 p) const
{
	const Vec2i pf = ConvertToPathfinderCoords(p.x, p.z);

	//NOTE: CRB if this is meant to be used safely, then its not an error to be out-of-bounds
	//ASSERT(pf.x >= 0 && pf.x < m_width);
	//ASSERT(pf.y >= 0 && pf.y < m_height);
	
	if (pf.x < 0 || pf.y < 0 || pf.x >= m_width || pf.y >= m_height)
		return m_blockMask;
	return m_nodeData[pf.x + pf.y * m_width];
}

FORCEINLINE Vec2i Pathfinder::ConvertToPathfinderCoords(float x, float y) const
{
	return { ( int ) floor( x / K_TILE_SIZE_F ), ( int ) floor( y / K_TILE_SIZE_F ) };
}

FORCEINLINE Vec2 Pathfinder::ConvertToWorldCoords(int x, int y) const
{
	// we return the center of the cell
	return nsTiles::GetTileCenter( { x, y } );
}

bool Pathfinder::IsInsideMap(Vec2 wp) const
{
	auto p = ConvertToPathfinderCoords(wp.x, wp.y);
	return (p.x >= 0 && p.x < m_width && p.y >= 0 && p.y < m_height);
}

bool Pathfinder::IsInsideMap(Vec2i p) const
{
	return (p.x >= 0 && p.x < m_width && p.y >= 0 && p.y < m_height);
}

Vec2 Pathfinder::AdjustToInsideMap(const Vec2& start, const Vec2& end) const
{
	float width = CELL_SIZE_METERS *  (m_width / 2) - 0.1f;
	float height = CELL_SIZE_METERS * (m_height / 2) - 0.1f;

	Vec2 diff = end - start;
	Vec2 edge = -start;
	edge.x += diff.x > 0.f ? width : -width;
	edge.y += diff.y > 0.f ? height : -height;

	// TODO: sometimes this returns huge values, which are not inside map and breaks the pathfinder in functions that don't check bounds, see LineHitsAny()
	//   crashdumps are unclear, but results in an output like {x=-32.9000015 y=-14177.7666} using an input like {x=-32.9012413 y=3.62514806}. Could not reproduce, needs investigating.
	float xt = diff.x != 0.f ? edge.x / diff.x : INFINITY;
	float yt = diff.y != 0.f ? edge.y / diff.y : INFINITY;
	float t = min(min(xt, yt), 1.f);

	return start + (diff * t);
}

Vec2 Pathfinder::AdjustToInsideCell(const Vec2& p) const
{
	auto c = ConvertToPathfinderCoords(p.x, p.y);
	auto celled = ConvertToWorldCoords(c.x, c.y);
	//NOTE: Adjust to within a valid cell so float rounding errors don't happen
	constexpr float factor = 0.495f;
	celled.x = celled.x + LIMIT( p.x - celled.x, CELL_SIZE_METERS * -factor, CELL_SIZE_METERS * factor);
	celled.y = celled.y + LIMIT( p.y - celled.y, CELL_SIZE_METERS * -factor, CELL_SIZE_METERS * factor);
	return celled;
}

Vec2 Pathfinder::AdjustToOutsideCollision(const Vec2& p, unsigned char mask) const
{
	auto c = ConvertToPathfinderCoords(p.x, p.y);
	if (m_nodeData[c.x + c.y * m_width] & mask)
	{
		float fdx = (float(c.x - (m_width / 2)) + 0.5f) * -CELL_SIZE_METERS + p.x;
		float fdy = (float(c.y - (m_height / 2)) + 0.5f) * -CELL_SIZE_METERS + p.y;
		int dx = fdx > 0.f ? 1 : -1;
		int dy = fdy > 0.f ? 1 : -1;

		Vec2i check[3];
		check[0] = { c.x +dx, c.y };
		check[1] = { c.x +dx, c.y +dy };
		check[2] = { c.x	, c.y +dy };
		if (fdx < fdy) 
			std::swap(check[0], check[2]);
		int clearIdx = -1;
		for (int i = 0; i < 3; i++)
			if (!(m_nodeData[check[i].x + check[i].y * m_width] & mask))
			{
				clearIdx = i;
				break;
			}
		if (clearIdx == -1)
			return p; // both the starting position and direct neighbors are blocked, there's no hope

		c = check[clearIdx];
	}

	//NOTE: Adjust to within a valid cell so float rounding errors don't happen
	auto celled = ConvertToWorldCoords(c.x, c.y);
	constexpr float factor = 0.495f;
	celled.x = celled.x + LIMIT( p.x - celled.x, CELL_SIZE_METERS * -factor, CELL_SIZE_METERS * factor );
	celled.y = celled.y + LIMIT( p.y - celled.y, CELL_SIZE_METERS * -factor, CELL_SIZE_METERS * factor );
	return celled;
}

//bool Pathfinder::GetPath_Unsafe(const Vec3& start, const Vec3& end, const Vec3** ppPath, int& numPathPoints, unsigned int blockFlags, bool bGetClosestPointIfBlocked /*= true*/, unsigned int additionalCostFlags /*= 0*/)
//{
//	numPathPoints = 0;
//	*ppPath = m_localGetPathBuffer.GetListPtr();
//	Vec2i endInt = ConvertToPathfinderCoords(end.x, end.z);
//	eResult result = GetPath(ConvertToPathfinderCoords(start.x, start.z), endInt, m_localGetPathBuffer.GetListPtr(), numPathPoints, m_localGetPathBuffer.GetCapacity(), blockFlags, bGetClosestPointIfBlocked, additionalCostFlags);
//	if (result == RESULT_ALL_GOOD)
//	{
//		//TODO: there are still cases of "start point inside collision"
//		// replace last point with the more precise end point
//		m_localGetPathBuffer.Resize(Max(1, numPathPoints));
//		Vec3 endClamped = ConvertToWorldCoords(endInt.x, endInt.y);
//		m_localGetPathBuffer[numPathPoints - 1] = endClamped;
//		//NOTE: Converting back from endInt, but adjust to within a valid cell so float rounding errors don't happen
//		Vec3 diff = end - endClamped;
//		diff.x = Clamp(CELL_SIZE_METERS * -0.49f, CELL_SIZE_METERS * 0.49f, diff.x);
//		diff.z = Clamp(CELL_SIZE_METERS * -0.49f, CELL_SIZE_METERS * 0.49f, diff.z);
//		m_localGetPathBuffer[numPathPoints - 1] = endClamped + diff;
//	}
//
//	return (result != RESULT_FAILED);
//}

bool Pathfinder::GetPath(const Vec2& start, const Vec2& end, Vec2* pPath, int maxPathPoints, int& numPathPoints, unsigned char blockFlags, bool bGetClosestPointIfBlocked /*= true*/, unsigned char additionalCostFlags /*= 0*/)
{
	eResult result = GetPath(ConvertToPathfinderCoords(start.x, start.y), ConvertToPathfinderCoords(end.x, end.y), pPath, maxPathPoints, numPathPoints, blockFlags, bGetClosestPointIfBlocked, additionalCostFlags);
	if (result == RESULT_ALL_GOOD)
	{
		// replace last point (which is center-cell) with the exact end point
		numPathPoints = max(1, numPathPoints);
		pPath[numPathPoints - 1] = end;
	}

	// should we fill in the height?
	//for (int i = 0; i < numPathPoints; ++i)
	//	pPath[i].y = start.y;

	return (result != RESULT_FAILED);
}

Vec2i Pathfinder::FindClosestEmptyCell(const Vec2i& start, int range, unsigned char collidableMask) const
{
	// searches in a spiral
	SpiralIdx spiral{m_width, m_height};
	spiral.setRange(range);
	do {
		int px, py;
		if (spiral.getIdx(start.x, start.y, px, py)
			&& ((m_nodeData[py * m_width + px] & collidableMask) == 0))
		{
			return {px, py};
		}
	} while (spiral);

	return start;
}

// if 'bGetClosestPointIfBlocked' is set, we will always return a valid path, even if start/end are outside the map or inside a collision
Pathfinder::eResult Pathfinder::GetPath(const Vec2i start, Vec2i end, Vec2* pPath, int maxPathPoints, int& numPathPoints, unsigned char blockFlags, bool bGetClosestPointIfBlocked, unsigned char additionalCostFlags)
{
	_ASSERT(blockFlags);
	if (!blockFlags)
		blockFlags = m_blockMask;
	numPathPoints = 0;

	const bool startOutsideMap = start.x < 0 || start.x >= m_width || start.y < 0 || start.y >= m_height;
	const bool endOutsideMap = end.x < 0 || end.x >= m_width || end.y < 0 || end.y >= m_height;
	if (startOutsideMap)
	{
		// get me inside...
		pPath[0] = ConvertToWorldCoords(end.x, end.y);
		numPathPoints = 1;
		LOG(L"[Error] Pathfinder::GetPath() start point outside of map\n");
		return RESULT_FAILED;
	}

	if (endOutsideMap)
	{
		g_pLog->Write("[Warning] Pathfinder::GetPath() end point outside of map\n");
		return RESULT_FAILED;
/*
DMC: was already commented out
		if (!bGetClosestPointIfBlocked)
			return RESULT_FAILED;

		// we will return the closest path to the point even if outside, but help a bit by moving the endpoit inside a valid cell, so that we don't have to traverse the ENTIRE map

		// project end point onto the map's edge and into the map until we find a non-collideable cell
		Vec2i mapEdge(Clamp(1, m_width - 2, end.x), Clamp(1, m_height - 2, end.y)); // -2 because we know the map edges to be collideable
		while ((m_nodeData[mapEdge.y * m_width + mapEdge.x] & blockFlags) != 0)
		{		
				mapEdge.x += Math::SignOf(mapEdge.x - end.x);
				mapEdge.y += Math::SignOf(mapEdge.y - end.y);

				if (mapEdge.x < 0 || mapEdge.x >= m_width || mapEdge.y < 0 || mapEdge.y >= m_height)
				{
					mapEdge = end;
					break; // couldn't find a way
				}
		}
		end = mapEdge;
*/
	}

	bool startInsideCollision = (GetRawData(start.x, start.y) & blockFlags) != 0;
	bool endInsideCollision = (GetRawData(end.x, end.y) & blockFlags) != 0;
	if (startInsideCollision)
	{
		// get me outside...
		//   this has consequences, don't use it? (if a human is walking along a path and he just barely goes through a wall and we click for a path, he will get a straight line to wherever that is)
		//pPath[0] = ConvertToWorldCoords(end.x, end.y);
		//numPathPoints = 1;
		g_pLog->Write("[Error] Pathfinder::GetPath() start point inside collision\n");
		return RESULT_FAILED;
	}

	if (endInsideCollision)
	{
		if (!bGetClosestPointIfBlocked)
		{
			g_pLog->Write("[Warning] Pathfinder::GetPath() end point inside collision\n");
			return RESULT_FAILED;
		}

		// we will return the closest path to the point even if outside, but help a bit by moving the endpoint inside a valid cell, so that we don't have to traverse the ENTIRE map

		// search for a valid end point around our target (expand in a spiral and get the free point that's closest to the collidable point)
		Vec2i empty = FindClosestEmptyCell(end, 25, blockFlags);
		_ASSERT((GetRawData(empty.x, empty.y) & blockFlags) == 0);
		//#ifdef PATHFINDING_PERF //NOTE: This doesn't seem to take any time at all
		//	float deltaMS = (OS_GetTimeMicroSec() - timeBefore) / 1000.f;
		//	g_pLog->Write("[Info] FindClosestEmptyCell %.3f ms | flags %d \n", deltaMS, blockFlags);
		//#endif

		_ASSERT(empty != end && "Can't find an empty cell around this point. Big performance warning: even though the query will not fail, we will need to search through the entire map. You shouldn't get here in normal gameplay.");
		end.x = empty.x;
		end.y = empty.y;
	}

	// go
	m_statusOpen += 2; // these being unsigned shorts, we can call the function 32767 times until we have to wrap around
	m_statusClosed += 2;

	m_nOpenListSize = 0;
	unsigned int startNodeIdx = start.y * m_width + start.x;
	PathNode* startnode = &m_nodemap[startNodeIdx];
	AddNewToOpenList(startnode, 0, -1, end.x, end.y);

	// in case the end point is inside a collision, we'll backtrack from the closest one found
	PathNode* minDistNode = startnode;

	PathNode* resultNode = NULL;
	while (m_nOpenListSize)
	{
		PathNode* bestnode = PopBestOpenNode();
		if (bestnode->status == m_statusClosed)
			continue;
		const unsigned short bestnodeGCost = bestnode->gcost;
		//const unsigned short bestnodeCost = bestnode->gcost + bestnode->hcost;
		const short bestnodeX = bestnode->x;
		const short bestnodeY = bestnode->y;
		bestnode->status = m_statusClosed;
		const int bestnodeIdx = bestnodeY * m_width + bestnodeX;
		if (bestnodeX == end.x && bestnodeY == end.y)
		{
			resultNode = bestnode; // we're done
			break;
		}

		// don't need to check bounds, since we block borders when building the map
		const int xll = (bestnodeX - 1);
		const int xul = (bestnodeX + 1);
		const int yll = (bestnodeY - 1) * m_width;
		const int yul = (bestnodeY + 1) * m_width;

		// top/left/right/bottom are mandatory, but diagonals are checked only if they don't cut through a collision corner
		//{xll + bestnodeY},	// left
		//{xul + bestnodeY},	// right
		//{bestnodeX + yll},	// top
		//{bestnodeX + yul},	// bottom
		//{xll + yll},			// top left
		//{xul + yll},			// top right
		//{xll + yul},			// bottom left
		//{xul + yul},			// bottom right

		unsigned neighbors[8] = {
			m_nodeData[yll + xll],//tl
			m_nodeData[yll + xul],//tr
			m_nodeData[yll + bestnodeX],// top
			m_nodeData[yul + xll],//bl
			m_nodeData[yul + xul],//br
			m_nodeData[yul + bestnodeX],//bottom
			m_nodeData[xll + bestnodeY * m_width],// left
			m_nodeData[xul + bestnodeY * m_width],// right
		};
		int numNeighbors = 0;
		PathNode* pNeighbors[8] = {};
		unsigned short pNeighborAdditionalCost[8] = {};
		if (!(neighbors[2] & blockFlags)) { // top
			pNeighborAdditionalCost[numNeighbors] = GetAdditionalCost(neighbors[2], additionalCostFlags);
			pNeighbors[numNeighbors++] = &m_nodemap[yll + bestnodeX];
		}
		if (!(neighbors[5] & blockFlags)) {
			pNeighborAdditionalCost[numNeighbors] = GetAdditionalCost(neighbors[5], additionalCostFlags);
			pNeighbors[numNeighbors++] = &m_nodemap[yul + bestnodeX];
		}
		if (!(neighbors[6] & blockFlags)) {// left
			pNeighborAdditionalCost[numNeighbors] = GetAdditionalCost(neighbors[6], additionalCostFlags);
			pNeighbors[numNeighbors++] = &m_nodemap[xll + bestnodeY * m_width];
		}
		if (!(neighbors[7] & blockFlags)) {
			pNeighborAdditionalCost[numNeighbors] = GetAdditionalCost(neighbors[7], additionalCostFlags);
			pNeighbors[numNeighbors++] = &m_nodemap[xul + bestnodeY * m_width];
		}
		// top left
		if (!(neighbors[2] & blockFlags) && !(neighbors[6] & blockFlags) && !(neighbors[0] & blockFlags)) {
			pNeighborAdditionalCost[numNeighbors] = GetAdditionalCost(neighbors[0], additionalCostFlags);
			pNeighbors[numNeighbors++] = &m_nodemap[yll + xll];
		}
		// top right
		if (!(neighbors[2] & blockFlags) && !(neighbors[7] & blockFlags) && !(neighbors[1] & blockFlags)) {
			pNeighborAdditionalCost[numNeighbors] = GetAdditionalCost(neighbors[1], additionalCostFlags);
			pNeighbors[numNeighbors++] = &m_nodemap[yll + xul];
		}
		// bottom left
		if (!(neighbors[5] & blockFlags) && !(neighbors[6] & blockFlags) && !(neighbors[3] & blockFlags)) {
			pNeighborAdditionalCost[numNeighbors] = GetAdditionalCost(neighbors[3], additionalCostFlags);
			pNeighbors[numNeighbors++] = &m_nodemap[yul + xll];
		}
		// bottom right
		if (!(neighbors[5] & blockFlags) && !(neighbors[7] & blockFlags) && !(neighbors[4] & blockFlags)) {
			pNeighborAdditionalCost[numNeighbors] = GetAdditionalCost(neighbors[4], additionalCostFlags);
			pNeighbors[numNeighbors++] = &m_nodemap[yul + xul];
		}

		for (int i = 0; i < numNeighbors; ++i)
		{
			PathNode* neighbor = pNeighbors[i];
			const int status = neighbor->status;

			if (status == m_statusClosed)
				continue; // closed

			const unsigned short currentgcost = GetGCostForParent(neighbor->x, neighbor->y, bestnodeGCost, bestnodeX, bestnodeY) + pNeighborAdditionalCost[i];
			if (status == m_statusOpen)
			{
				const unsigned short oldgcost = neighbor->gcost;
				// if this path is easier, update the parent and our position in the list
				if (currentgcost < oldgcost)
				{
					neighbor->parent = bestnodeIdx;
					neighbor->gcost = currentgcost;
					
					//add to open list again, don't bother removing it
					AddToOpenList(neighbor, currentgcost + neighbor->hcost);
				}
			}
			else
			{
				AddNewToOpenList(neighbor, currentgcost, bestnodeIdx, end.x, end.y);

				if (neighbor->hcost < minDistNode->hcost)
					minDistNode = neighbor;
			}
		}
	}

	if (resultNode == NULL && bGetClosestPointIfBlocked)
	{
		resultNode = minDistNode;
	}

	// perform manipulation on the resulting path

	//#DMC: path smoothing can be taken from my version where it announces collision when crossing higher cost tiles too to avoid corners

	int points = 0;
	PathNode* node = resultNode;
	//PathNode* prevAddedNode = node;
	Vec2i prevAddedPoint;
	Vec2i prevNodePoint;

	//int idx = 0;
	//auto drawNode = resultNode;
	//while (drawNode) {
	//	g_debugCircles[idx] = ConvertToWorldCoords(drawNode->x, drawNode->y);
	//	g_debugCircleCount = Min(idx, (int)COUNT_OF(g_debugCircles));
	//	idx = (idx + 1) % (int)COUNT_OF(g_debugCircles);
	//	drawNode = drawNode->parent;
	//}

	// add the first point
	if (node)
	{
		pPath[points++] = ConvertToWorldCoords(node->x, node->y);
		prevAddedPoint.x = node->x;
		prevAddedPoint.y = node->y;
		prevNodePoint.x = node->x;
		prevNodePoint.y = node->y;
		node = node->parent > -1 ? &m_nodemap[node->parent] : NULL;
	}

	// backtrack while removing unnecessary points if there's no collision between
	while (node != NULL)
	{
		if (points >= maxPathPoints)
		{
			ErrorBox(K_ERR_WARNING, L"[Error] Pathfinding failed with insufficient number of path points (%d and we space enough space for %d)\n", points, maxPathPoints);
			return RESULT_FAILED;
		}

		const bool bCollided = TraceBresenhamLine(prevAddedPoint, Vec2i(node->x, node->y), blockFlags);
		if (bCollided)
		{
			// found a non-straightline node
			/*
			// check backwards to find first node that can see current node and add it
			PathNode* nodeToAdd = prevAddedNode;
			while (nodeToAdd != node) {
				if (!TraceBresenhamLine({ nodeToAdd->x, nodeToAdd->y }, Vec2i(node->x, node->y), blockFlags))
					break;
				nodeToAdd = nodeToAdd->parent;
			}
			if (prevAddedPoint != prevNodePoint)
				pPath[points++] = ConvertToWorldCoords(nodeToAdd->x, nodeToAdd->y);
			prevAddedPoint.x = nodeToAdd->x;
			prevAddedPoint.y = nodeToAdd->y;
			prevAddedNode = nodeToAdd;
			//*/

			if (prevAddedPoint != prevNodePoint)
				pPath[points++] = ConvertToWorldCoords(prevNodePoint.x, prevNodePoint.y);
			prevAddedPoint.x = prevNodePoint.x;
			prevAddedPoint.y = prevNodePoint.y;
			
		}

		prevNodePoint.x = node->x;
		prevNodePoint.y = node->y;
		node = node->parent > -1 ? &m_nodemap[node->parent] : NULL;

	}

	// reverse order
	numPathPoints = points;
	for (int i = 0; i < points / 2; ++i)
	{
		Vec2 temp = pPath[i];
		pPath[i] = pPath[numPathPoints - 1 - i];
		pPath[numPathPoints - 1 - i] = temp;
	}

	const bool bClosestPoint = (endInsideCollision || endOutsideMap || !resultNode || resultNode->x != end.x || resultNode->y != end.y);

	if (!resultNode)
		return RESULT_FAILED;

	if (bClosestPoint)
		return RESULT_CLOSEST_POINT;

	return RESULT_ALL_GOOD;
}

//void Pathfinder::WriteFatBresenhamLine(const Vec2i & start, const Vec2i & end, unsigned int collidableMask, unsigned writeMask, eUpdateType add) const
//{
//	// Note: start needs to be inside the map
//
//	int x = start.x;
//	int y = start.y;
//
//	if (m_nodeData[x + y * m_width] & collidableMask)
//		return;
//
//	int dx = abs(end.x - start.x);
//	int dy = abs(end.y - start.y);
//	int x_inc = (end.x > start.x) ? 1 : -1;
//	int y_inc = (end.y > start.y) ? 1 : -1;
//	int error = dx - dy;
//	dx *= 2;
//	dy *= 2;
//
//	const int maxNodeIndex = m_width * m_height - 1;
//
//	while (x != end.x || y != end.y)
//	{
//		if (error > 0)
//		{
//			x += x_inc;
//			error -= dy;
//		}
//		else
//		{
//			y += y_inc;
//			error += dx;
//		}
//
//		_ASSERT((x + y * m_width) < (m_width * m_height) && (x + y * m_width) >= 0);
//		unsigned int value = m_nodeData[x + y * m_width];
//		if (value & collidableMask) {
//			return;
//		}
//		else if (add == ADD_LOWORD) {
//			writeMask &= 0x0000ffff;
//			m_nodeData[x + y * m_width]								|= writeMask;
//			m_nodeData[Min(maxNodeIndex, x + 1 + y * m_width)]		|= writeMask;
//			m_nodeData[Max(0, x - 1 + y * m_width)]					|= writeMask;
//			m_nodeData[Min(maxNodeIndex, x + (y + 1) * m_width)]	|= writeMask;
//			m_nodeData[Max(0, x + (y - 1) * m_width)]				|= writeMask;
//		}
//		else if (add == REMOVE_LOWORD) {
//			writeMask &= 0x0000ffff;
//			m_nodeData[x + y * m_width]								&= ~writeMask;
//			m_nodeData[Min(maxNodeIndex, x + 1 + y * m_width)]		&= ~writeMask;
//			m_nodeData[Max(0, x - 1 + y * m_width)]					&= ~writeMask;
//			m_nodeData[Min(maxNodeIndex, x + (y + 1) * m_width)]	&= ~writeMask;
//			m_nodeData[Max(0, x + (y - 1) * m_width)]				&= ~writeMask;
//		}
//		else
//		{
//			// unsupported / not needed for this use case
//			DEBUG_BREAK();
//		}
//	}
//}

bool Pathfinder::TraceBresenhamLine(const Vec2i& start, const Vec2i& end, unsigned char collidableMask, Vec2i* hitPoint) const
{
	// Note: 'start' needs to be inside the map

	int x = start.x;
	int y = start.y;

	if (m_nodeData[x + y * m_width] & collidableMask)
	{
		if (hitPoint)
			*hitPoint = { x, y };
		return true;
	}

	int dx = abs(end.x - start.x);
	int dy = abs(end.y - start.y);
	int x_inc = (end.x > start.x) ? 1 : -1;
	int y_inc = (end.y > start.y) ? 1 : -1;
	int error = dx - dy;
	dx *= 2;
	dy *= 2;

	// check 3 grid points around the origin, depending on the direction (e.g.: left, bottom left and bottom.  e.g.2: right, top right and top)
	//NOTE: Crb disabled this because it's causing wrong results along 
	bool bCheckCorners = false;// start.x != end.x && start.y != end.y;
	Vec2i offsets[3] = {
		{x_inc, 0},
		{x_inc, y_inc},
		{0, y_inc}
	};
	const int maxNodeIndex = m_width * m_height;

	while (x != end.x || y != end.y)
    {
		if (bCheckCorners)
		{
			// search around this cell
			unsigned int d;
			int idx;
			for (int off = 0; off < ARRAY_SIZE(offsets); off++) {
				//TODO: BUG! x isn't range-checked here, and will read from the wrong lines on edges (x==0 & x==width-1) !
				idx = x + offsets[off].x + (y + offsets[off].y) * m_width;
				if (idx >= 0 && idx < maxNodeIndex) {
					d = m_nodeData[idx];
					if (d & collidableMask) {
						if (hitPoint)
							*hitPoint = { x + offsets[off].x, y + offsets[off].y };
						return true;
					}
				}
			}
		}

        if (error > 0)
        {
            x += x_inc;
            error -= dy;
        }
		else
        {
            y += y_inc;
            error += dx;
        }

		_ASSERT((x + y * m_width) < (m_width * m_height) && (x + y * m_width) >= 0);
		unsigned int value = m_nodeData[x + y * m_width];
		if (value & collidableMask)
		{
			if (hitPoint)
				*hitPoint = { x,y };
			return true;
		}
    }

	return false;
}

//bool Pathfinder::TraceBresenhamLineBlocked(const Vec2i & start, const Vec2i & end, unsigned int collidableMask, Vec2i * hitPoint) const
//{
//	// Note: 'start' needs to be inside the map
//	int x = start.x;
//	int y = start.y;
//	if ( (m_nodeData[x + y * m_width] & collidableMask) == 0)
//	{
//		if (hitPoint)
//			*hitPoint = { x, y };
//		return true;
//	}
//
//	int dx = abs(end.x - start.x);
//	int dy = abs(end.y - start.y);
//	int x_inc = (end.x > start.x) ? 1 : -1;
//	int y_inc = (end.y > start.y) ? 1 : -1;
//	int error = dx - dy;
//	dx *= 2;
//	dy *= 2;
//
//	// check 3 grid points around the origin, depending on the direction (e.g.: left, bottom left and bottom.  e.g.2: right, top right and top)
//	//NOTE: Crb disabled this because it's causing wrong results along 
//	bool bCheckCorners = false;// start.x != end.x && start.y != end.y;
//	Vec2i offsets[3] = {
//		{x_inc, 0},
//		{x_inc, y_inc},
//		{0, y_inc}
//	};
//	const int maxNodeIndex = m_width * m_height;
//
//	while (x != end.x || y != end.y)
//	{
//		if (bCheckCorners)
//		{
//			// search around this cell
//			unsigned int d;
//			int idx;
//			for (int off = 0; off < COUNT_OF(offsets); off++) {
//				//TODO: BUG! x isn't range-checked here, and will read from the wrong lines on edges (x==0 & x==width-1) !
//				idx = x + offsets[off].x + (y + offsets[off].y) * m_width;
//				if (idx >= 0 && idx < maxNodeIndex) {
//					d = m_nodeData[idx];
//					if ((d & collidableMask) == 0) {
//						if (hitPoint)
//							*hitPoint = { x + offsets[off].x, y + offsets[off].y };
//						return true;
//					}
//				}
//			}
//		}
//
//		if (error > 0)
//		{
//			x += x_inc;
//			error -= dy;
//		}
//		else
//		{
//			y += y_inc;
//			error += dx;
//		}
//
//		ASSERT((x + y * m_width) < (m_width * m_height) && (x + y * m_width) >= 0);
//		unsigned int value = m_nodeData[x + y * m_width];
//		if ( (value & collidableMask) == 0)
//		{
//			if (hitPoint)
//				*hitPoint = { x,y };
//			return true;
//		}
//	}
//
//	return false;
//}

inline void Pathfinder::AddToOpenList(PathNode* node, int cost)
{
	unsigned nodeIdx = (int)(node - m_nodemap);

	_ASSERT(m_nOpenListSize < MAX_OPEN_NODES && "Too many open nodes!");
	m_openlist[m_nOpenListSize] = { unsigned(cost), nodeIdx };
	++m_nOpenListSize;
	std::push_heap(m_openlist, m_openlist + m_nOpenListSize, [](const Pathfinder::OpenNode& a, const Pathfinder::OpenNode& b) -> bool
	{
		return a.cost > b.cost;
	});
}

//CRB: Using a repeatable stress test (a LOT of varied length queries) the push_heap version takes 580ms vs the 2200 in the old linear search version
//If we also use the additionalCost thing the difference increases. New version is 870ms and old is 3800ms
//Shorter paths have a smaller speedup, but the longer a path is the more we gain. Which is what we want
inline void Pathfinder::AddNewToOpenList(PathNode* node, unsigned short gcost, int parentIdx, int destx, int desty)
{
	node->parent = parentIdx;
	node->gcost = gcost;
	auto hcost = CalculateH(node->x, node->y, destx, desty);;
	node->hcost = hcost;
	node->status = m_statusOpen;

	AddToOpenList(node, gcost + hcost);
}


PathNode* Pathfinder::PopBestOpenNode()
{
	/* OLD, linear search, slower
	int index = -1;
	unsigned int mincost = 0xffffffff;
	for (int i = 0; i < m_nOpenListSize; i++)
	{
		auto& pNode = m_nodemap[m_openlist[i].idx];
		//NOTE: We use the actual node cost instead of the stored one so we can check for correctness vs the fast method
		unsigned cost = pNode.gcost + pNode.hcost;
		if (cost < mincost)
		{
			mincost = cost;
			index = i;
		}
	}
	PathNode* best = &m_nodemap[m_openlist[index].idx];
	m_openlist[index] = m_openlist[--m_nOpenListSize];
	//*/

	int bestIdx = -1;
	std::pop_heap(m_openlist, m_openlist + m_nOpenListSize, [](const Pathfinder::OpenNode& a, const Pathfinder::OpenNode& b) -> bool
	{
		return a.cost > b.cost;
	});
	--m_nOpenListSize;
	bestIdx = m_openlist[m_nOpenListSize].idx;
	PathNode* best = &m_nodemap[bestIdx];

	return best;
}