#pragma once

// collision flags and masks (have more flags that combine to set the accessible flag)
#define COL_ACCESSIBLE		1
// flag that blocks movement
#define COL_MOVEMENT_BLOCK	2

// this will define how big the collision map will be
#define CELL_SIZE_METERS		0.25f // = width of a wall. for a 64x64 meters map we will have a 256x256 pathfinding map
#define CELL_SIZE_METERS_INV	(1.0f / CELL_SIZE_METERS)
#define CELL_RADIUS				(sqrtf(2.0f * CELL_SIZE_METERS * 0.5f * CELL_SIZE_METERS * 0.5f))

// either use this or the clearance params (this works better for our use case)
#define OBJECT_EXPANSION		(CELL_SIZE_METERS * 0.5f + 0.001f) // == 0.126f

#define MAX_OPEN_NODES			3000

//CRB: I moved the actual data outside this node stuff so data updates are much faster
// It didn't affect the pathfinding speed (with the most recent code), but it should be faster when it's all together
struct PathNode
{
	short					x;
	short					y;
	unsigned short			status;
	unsigned short			padding;
	int						parent; // use index instead of pointer, uses less memory on x64
	unsigned short			gcost;
	unsigned short			hcost;
};

class Pathfinder
{
public:
							Pathfinder();
							~Pathfinder();

	// when loading a map, call InitStart(), use AddObject() for each entity that blocks movement and InitEnd() after that.
	void					InitStart(int sourceWidthMeters, int sourceHeightMeters, unsigned int blockMask);
	void					InitEnd(unsigned int clearanceValueStartBit = 0, int numClearanceValues = 0);
	//void					MarkShapeCells(const Vec3* shapePts, int numPts, unsigned int blockMask); // adds given flags to all cells that are inside given shape

	void					MarkAsAccessible(Vec2 pos);
	//void					FloodfillAccessibleMask();
//	bool					AreSuspicionAreasUpdated() { return m_suspiciousAreasUpdated; }
//	void					SetSuspicionAreasUpdated(bool yes) { m_suspiciousAreasUpdated = yes; }

	enum eUpdateType
	{
		ADD_LOWORD_REPLACE_HIWORD,		// least sig. 16 bits are added, the most sig. 16 bits (which contain EntityId are replaced), see eCollisionFlags
		ADD_LOWORD,						// least sig. 16 bits are added, the most sig. 16 bits (the EntityId) are left unchanged
		REMOVE_ALL_IF_SAME_HIWORD,		// removes least sig 16 bits and the most sig. 16 bits (only if the the most sig. 16 bits are the same)
		REMOVE_LOWORD_AND_MAYBE_HIWORD, // removes least sig 16 bits, while the most sig 16 bits are only removed if they're the same
		REMOVE_LOWORD					// removes least sig 16 bits, the most sig. 16 bits are left unchanged
	};
	/*
	void					UpdateObject(const sCollisionShape& collision, const Matrix& parentTransform, unsigned int flags, eUpdateType update);
	void					UpdateCone(Vec2 s, Vec2 d, float coneWidth, unsigned blockFlag, unsigned flags, eUpdateType update);
	void					UpdateFOVCircleArea(Vec2 pos, float radius, unsigned blockMask, unsigned writeMask, eUpdateType update);
	*/
	// if 'bGetClosestPointIfBlocked' is set, we will always return a valid path, even if start/end are outside the map or inside a collision
	// uses the X/Z plane of the start/end points, but we accept Vec3 as a convenience
	// returns result in meters (game units)
	bool					GetPath(const Vec2& start, const Vec2& end, Vec2* pPath, int& numPathPoints, int maxPathPoints, unsigned int blockFlags, bool bGetClosestPointIfBlocked = true, unsigned int additionalCostFlags = 0);

	// !!! uses our own memory: not thread safe, memory is owned by this object and should not be referenced or released
	//bool					GetPath_Unsafe(const Vec3& start, const Vec3& end, const Vec3** ppPath, int& numPathPoints, unsigned int blockFlags, bool bGetClosestPointIfBlocked = true, unsigned int additionalCostFlags = 0);
	/*
	bool					LineHitsAny(Vec2 start, Vec2 end, unsigned blockFlags) const;
	bool					TraceLine(Vec2 start, Vec2 end, unsigned blockFlags, Vec2& out) const; // returns true when hitting something, out is set to hit point
	bool					TraceLineBlocked(Vec2 start, Vec2 end, unsigned blockFlags, Vec2& out) const; // returns true when NOT hitting something, out is set to hit point
	bool					IsPointClear(Vec2 pos, unsigned blockFlags) const;
	bool					ClosestCell(Vec2& out, Vec2 pos, unsigned blockFlags) const; // returns closest cell with given blockFlag
	bool					ClosestBlockedCellOutsideRadius(Vec2& out, Vec2 pos, unsigned blockFlags, const Vec2* occupiedPos, float occupiedRadius, int count) const;
	bool					ClosestFreeCellOutsideRadius(Vec2& out, Vec2 pos, unsigned blockFlags, const Vec2* occupiedPos, float occupiedRadius, int count) const;
	bool					ClosestEmptyCell(Vec2& out, Vec2 pos, unsigned blockFlags) const; // returns closest cell without given blockFlag
	*/
	unsigned int			GetRawData_Safe(Vec3 p) const; // checks bounds
	float					GetCellSizeMeters() const;
	int						GetWidth() const { return m_width; }
	int						GetHeight() const { return m_height; }
	//unsigned char*			GetPathfinderMipsRGBA(unsigned mask) const;
	//unsigned char*			GetPathfinderDataRGBA(unsigned mask) const;
	//unsigned char*			GetPathfinderDataR(unsigned mask) const;
	//unsigned char*			GetPathfinderLastSearchRGBA() const;
	bool					IsInsideMap(Vec2 p) const;
	Vec2					AdjustToInsideMap(const Vec2& start, const Vec2& end) const; // adjust endpoint so it's not outside the map and not touching the map edges
	Vec2					AdjustToInsideCell(const Vec2&) const; // clamp values so they aren't too close to cell edge (to avoid float errors)
	Vec2					AdjustToOutsideCollision(const Vec2&, unsigned mask) const; // snap position to the nearest adjacent unblocked cell
private:
	Vec2i					ConvertToPathfinderCoords(float x, float y) const;
	Vec2					ConvertToWorldCoords(int x, int y) const;
	bool					IsInsideMap(Vec2i p) const;

	// writing/deleting objects into the pathfinder
	/*
	void					UpdateSphere(const float radius, const Vec3& origin, unsigned int flags, eUpdateType update);
	void					UpdatePoly(const Vec2* pVerts, const int numVerts, unsigned int flags, eUpdateType update);
	void					UpdateMips(int xStart, int yStart, int xEnd, int yEnd);
	*/

	// if 'bGetClosestPointIfBlocked' is set, we will always return a valid path, even if start/end are outside the map or inside a collision
	enum eResult {
		RESULT_ALL_GOOD, // ok
		RESULT_FAILED, // couldn't find a path (start/end outside of map or inside collision)
		RESULT_CLOSEST_POINT, // end path was inside collision, but a point closest to the endpoint was returned (when using flag PF_CLOSEST_POINT)
	};
	eResult					GetPath(Vec2i start, Vec2i end, Vec2* pPath, int& numPathPoints, int maxPathPoints, unsigned int blockFlags, bool bGetClosestPointIfBlocked, unsigned int additionalCostFlags); // uses pathfinder coords

	void					AddNewToOpenList(PathNode* node, unsigned short gcost, int parentIdx, int destx, int desty);
	void					AddToOpenList(PathNode* node, int cost);
	PathNode*				PopBestOpenNode();
	unsigned int			GetRawData(int x, int y) const { return m_nodeData[x + y * m_width]; }
	
	//void					WriteFatBresenhamLine(const Vec2i& start, const Vec2i& end, unsigned int collidableMask, unsigned writeMask, eUpdateType add) const;
	bool					TraceBresenhamLine(const Vec2i& start, const Vec2i& end, unsigned int collidableMask, Vec2i* hitPoint = nullptr) const;
	//bool					TraceBresenhamLineBlocked(const Vec2i& start, const Vec2i& end, unsigned int collidableMask, Vec2i* hitPoint = nullptr) const;
	
	Vec2i					FindClosestEmptyCell(const Vec2i& start, int range, unsigned int collidableMask) const;

private:
	int						m_width;
	int						m_height;
	unsigned int			m_blockMask;
	unsigned int*			m_nodeData{}; // first 16 bits are various flags (e.g. cover/solid object), the other 16 are the entity's ID
	PathNode*				m_nodemap{};
	unsigned int			m_clearanceValueStartBit;
	//int*					m_nodeDataMips{};
	int						m_sniperUpdateIdx;
	//bool					m_suspiciousAreasUpdated = false;

	unsigned short			m_statusOpen;
	unsigned short			m_statusClosed;

	struct OpenNode {
		unsigned cost;
		unsigned idx;// indices into m_nodemap
	};
	OpenNode				m_openlist[MAX_OPEN_NODES];
	int						m_nOpenListSize;

	// filled in GetPath_Unsafe() with the path points.
	List<Vec3>				m_localGetPathBuffer;
};
