#pragma once

///----------------------------------------------------------------------------------
/// geometry layers - how many paint layers we have for each area block
///----------------------------------------------------------------------------------
enum eAreaLayer {
	K_AL_UNDER_FLOOR = 0,
	K_AL_FLOOR = 1,
	K_AL_WALLSHADOWS,
	K_AL_WALLS,
	K_AL_CEIL_DECO,
	K_AL_CEILINGS,

	K_ALS_COUNT
};

///----------------------------------------------------------------------------------
/// Level area descriptor
///----------------------------------------------------------------------------------
struct CLevelAreaDesc
{
	WCHAR			strFilename[MAX_PATH];			// area filename (no path)
	WCHAR			strTags[MAX_PATH];				// area tags (eg: "bedroom,dark,blood,any")
};

///--------------------------------------------------------------------------
/// A level is composed of many level areas, generated dinamically. 
/// Area keeps geometry data.
///--------------------------------------------------------------------------
class CLevelArea
{
private:
	PDEVICE							m_pDevice;
public:
	UINT32							ID;						// area ID used for finding the area and for references to it
	CTile**							tiles;					// actual tilemap
	CAABB							AABBbounds;				// bounding box in world space
	RectXYWHi						AABBbounds_TL;			// AABB in tiles, in world space
	SizeWHi							sizeTL;					// Area size in tiles
	bool							bVisible;
	bool							bActive;				// Was it activated? as soon as an area becomes "visible" it activates itself and first neighbours
	CFixedArray<CLevelArea*, 10>	arrNeighbours;

	CTileBlockMeshManager			areaMesh;				// Mesh manager for the map, handles painting and breaking the tiles in smaller patches

public:
	CArray<CProp*>					m_arrProps;				// list of props in this area

public:
	CLevelArea( UINT32 nID );
	~CLevelArea();

	void					Release();
	// returns null if x,y outside valid area. Coords in world space.
	CTile*					GetTile( int xTL, int yTL );
	// Updates the level area visibility and blocks visibility
	bool					UpdateVisibility( RectXYWH camRect );
	// Returns true if the mesh for specified layer is visible
	bool					IsLayerMeshVisible( eAreaLayer layer );
	// orders building of the buffers
	OPRESULT				BuildBuffers( PDEVICE pDevice );
	// intersection of segment with tiles (nullptr if not intersecting)
	// tileFlagsNonCollide - if tile has one of the flags then it's not colliding
	CTile*					SegmentTilesIntersection( Vec2 vStart, Vec2 vEnd, Vec2 & retPoint, Vec2 & retNormal, Vec2i *hitTilePosTL, DWORD tileFlagsNonCollide = K_TILEFLAG_WALKABLE | K_TILEFLAG_UNDER_FLOOR );
	// writes the tiles that collide with the player in the ret_arrAABBs array. Returns number of added elements. Starts from 0 overwriting the ret_arrAABBs elements.
	int						GetTilesCollisionBoxes( RectXYXYi srcBoxTL, CAABB* ret_arrAABBs, int nArrCapacity );
	// writes the bboxes of the props that collide with the srcBoxTL. Starts from 0 overwriting the ret_arrAABBs elements.
	int						GetPropsCollisionBoxes( CAABB srcBox, CAABB* ret_arrAABBs, int nArrCapacity );
	// gets all props belonging to area, that collide with a bbox
	int						GetPropsTouchingBox( CAABB srcBox, CProp* ret_arrProps[], int nArrCapacity, bool bOnlyInteractibles = false );
	int						GetPropsTouchingBox( CAABB srcBox, CArray<CProp*>& ret_arrProps, bool bOnlyInteractibles = false );
	// gets all the tiles that 
	int						GetTilesByFlag( RectXYXYi srcBoxTL, UINT32 dwFlagAny, CTile* ret_arrTiles, int nArrCapacity );
	// returns true if srcBox collides with tiles, props or collision boxes
	bool					IsBoxColliding( CAABB srcBox, bool bCheckProps = true );

public: //--- framework methods ---
	OPRESULT OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc = nullptr, void* pUserContext = nullptr );
	OPRESULT OnResetDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc = nullptr, void* pUserContext = nullptr );
	OPRESULT OnLostDevice( void* pUserContext = nullptr );
	OPRESULT OnDestroyDevice( void* pUserContext = nullptr );
};
