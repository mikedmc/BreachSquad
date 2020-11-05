#pragma once

#define K_TBM_BLOCK_W		16
#define K_TBM_BLOCK_H		16

#define K_TBM_MAX_LAYERS	8

// single layered block
class CTileBlockMesh
{
public:
	CBufferedPainter			m_Painter;

	RECTXYWH					m_bboxTL;					// BBOX in tiles
	CAABB						m_bbox;						// BBOX in world coords

	int							m_arrMeshIdx[K_TBM_MAX_LAYERS]{};	// Array of mesh indexes per layer, or -1 for empty layers

public:
	CTileBlockMesh();
	~CTileBlockMesh();

	// Creates meshes for each layer
	// Receives pointer to map tiles
	OPRESULT					BuildBuffers(POINTXY_INT vBlockPos_TL, CTile** map, SIZEWH mapSizeTL, Vec2 vLevelOrigin);

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
public:
	CGrowableArray<CTileBlockMesh*> arrBlocks;

	CTileBlockMeshManager();
	~CTileBlockMeshManager();

	void						Release();
	// Builds all buffers for specified map
	OPRESULT					BuildBuffers(CTile** map, SIZEWH mapSizeTL, Vec2 vLevelOrigin);

	OPRESULT OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL, void* pUserContext = NULL);
	OPRESULT OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL, void* pUserContext = NULL);
	OPRESULT OnLostDevice(void* pUserContext = NULL);
	OPRESULT OnDestroyDevice(void* pUserContext = NULL);
};


