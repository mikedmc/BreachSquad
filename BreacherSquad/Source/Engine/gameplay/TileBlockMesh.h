#pragma once

#define K_TBM_BLOCK_W		16
#define K_TBM_BLOCK_H		16

#define K_TBM_MAX_LAYERS	8

// single layered block
class CTileBlockMesh
{
public:
	CBufferedPainterQuads		m_Painter;

	RECTXYWH					m_mapAreaTL;				// map zone that this block renders
	CAABB						m_bbox;						// BBOX in world coords

	int							m_arrMeshIdx[K_TBM_MAX_LAYERS]{};	// Array of mesh indexes per layer, or -1 for empty layers

public:
	CTileBlockMesh();
	CTileBlockMesh(PDEVICE pDevice);
	~CTileBlockMesh();

	// Creates meshes for each layer
	// Receives pointer to map tiles
	OPRESULT					BuildBuffers(Vec2i vBlockPos_TL, CTile** map, SIZEWH mapSizeTL, Vec2 vOffset, CSpriteCollection* pLightsSpr);

	void						Clear();

	void						PaintLayer(int nLayer, bool bSetFVF = false);
};

// Keeps an array of tileblocks and manages them
class CTileBlockMeshManager
{
private:
	PDEVICE							m_pDevice;
public:
	CArray<CTileBlockMesh*>			arrBlocks;
	CFixedArray<CTileBlockMesh*, 8> arrVisible;				// array of visible blocks, computed in BuildVisibilityList

	CTileBlockMeshManager();
	~CTileBlockMeshManager();

	void						Init(PDEVICE pDevice);
	void						Release();

	// Builds all buffers for specified map, called after loading a level and when we have changes
	OPRESULT					BuildBuffers(CTile** map, SIZEWH mapSizeTL, Vec2 vOffset, CSpriteCollection* pLightsSpr);

	// Creates list of visible blocks. camRect is the XY plane of the AABB of the camera frustum.
	// Must be called before PaintLayer.
	int							UpdateVisibility(RECTXYWH_F camRect);

	// Paints tile layer for visible buffers (use idx from eAreaLayer)
	OPRESULT					PaintLayer(int layerIdx);

	OPRESULT OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL, void* pUserContext = NULL);
	OPRESULT OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL, void* pUserContext = NULL);
	OPRESULT OnLostDevice(void* pUserContext = NULL);
	OPRESULT OnDestroyDevice(void* pUserContext = NULL);
};


