#pragma once

#define K_TBM_BLOCK_W		16
#define K_TBM_BLOCK_H		16

#define K_TBM_MAX_LAYERS	8

// single layered block
class CTileBlockMesh
{
public:
	CBufferedPainterQuads		m_Painter;

	RECTXYWH					m_bboxTL;					// BBOX in tiles
	CAABB						m_bbox;						// BBOX in world coords

	int							m_arrMeshIdx[K_TBM_MAX_LAYERS]{};	// Array of mesh indexes per layer, or -1 for empty layers

public:
	CTileBlockMesh();
	~CTileBlockMesh();

	// Creates meshes for each layer
	// Receives pointer to map tiles
	OPRESULT					BuildBuffers(POINTXY_INT vBlockPos_TL, CTile** map, SIZEWH mapSizeTL, Vec2 vLevelOrigin);

	void						Clear();

	inline void					PaintLayer(int nLayer, bool bSetFVF = false) 
	{
		assert((nLayer >= 0) && (nLayer < K_TBM_MAX_LAYERS));
		m_Painter.DrawMesh(m_arrMeshIdx[nLayer], bSetFVF);
	}
};

// Keeps an array of tileblocks and manages them
class CTileBlockMeshManager
{
private:
	PDEVICE							m_pDevice;
	// array of visible blocks
	CFixedArray<CTileBlockMesh*, 8> arrVisible;
public:
	CGrowableArray<CTileBlockMesh*> arrBlocks;

	CTileBlockMeshManager();
	~CTileBlockMeshManager();

	void						Release();
	// Builds all buffers for specified map, called after loading a level and when we have changes
	OPRESULT					BuildBuffers(CTile** map, SIZEWH mapSizeTL, Vec2 vLevelOrigin);
	// Creates list of visible blocks. camRect is the XY plane of the AABB of the camera frustum.
	// Must be called before PaintLayer.
	int							BuildVisibilityList(RECTXYWH_F camRect);
	// Paints tile layer for visible buffers
	OPRESULT					PaintLayer(int layerIdx);

	OPRESULT OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL, void* pUserContext = NULL);
	OPRESULT OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL, void* pUserContext = NULL);
	OPRESULT OnLostDevice(void* pUserContext = NULL);
	OPRESULT OnDestroyDevice(void* pUserContext = NULL);
};


