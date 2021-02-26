#pragma once

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
	PDEVICE					m_pDevice;
public:
	UINT32					ID;								// area ID used for finding the area and for references to it
	CTile**					tiles;							// actual tilemap
	CAABB					AABBbounds;						// bounding box in world space
	RECTXYWH				AABBbounds_TL;					// AABB in tiles, in world space
	SIZEWH					sizeTL;							// Area size in tiles
	bool					bVisible;
	//bool bDiscovered - was it activated?

	CTileBlockMeshManager	areaMesh;						// Mesh manager for the map, handles painting and breaking the tiles in smaller patches

public:
	CLevelArea(UINT32 nID);
	~CLevelArea();

	void					Release();
	// returns null if x,y outside valid area. Coords in world space.
	CTile*					GetTile(int xTL, int yTL);
	// Updates the level area visibility and blocks visibility
	bool					UpdateVisibility(RECTXYWH_F camRect);
	// orders building of the buffers
	OPRESULT				BuildBuffers(PDEVICE pDevice, CSpriteCollection* pLightsSprCol);
	// intersection of segment with tiles (nullptr if not intersecting)
	CTile*					SegmentTilesIntersection(Vec2 vStart, Vec2 vEnd, Vec2 & retPoint, Vec2 & retNormal, Vec2i *hitTilePosTL);
	// writes the tiles that collide with the player in the ret_arrAABBs array. Returns number of added elements.
	int						GetTilesCollisionBoxes(RECTXYXY srcBoxTL, CAABB* ret_arrAABBs, int nArrCapacity);

public: //--- framework methods ---
	OPRESULT OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL, void* pUserContext = NULL);
	OPRESULT OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL, void* pUserContext = NULL);
	OPRESULT OnLostDevice(void* pUserContext = NULL);
	OPRESULT OnDestroyDevice(void* pUserContext = NULL);
};
