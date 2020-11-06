#pragma once
///-----------------------------------------------------------------------------------------------
///	 BUFFERED PAINTER QUADS
///  Adds geometry to meshes identified by index and then builds VB and IB and draws them
///-----------------------------------------------------------------------------------------------
#define K_BP_SENTINEL_QUADS		10
#define K_BP_MAX_MESHES_CNT		100

class CBufferedPainterQuads
{
protected:
	PVERTEXBUFFER				m_vb;
	PINDEXBUFFER				m_ib;

	PDEVICE						m_pDevice;

	UINT32						m_nVertexCursor;						// Current vertex index in buffer (writing position)
	_VERTEX_PNCT4T4				*m_verts;								// temp buffer for storing vertices
	
	UINT32						m_nQuadsPerMesh[K_BP_MAX_MESHES_CNT]{};	// Triangle count per mesh
	UINT32						m_nQuadsOffsets[K_BP_MAX_MESHES_CNT]{};	// VB offset per mesh
	int							m_nMeshesCnt;							// Total no of pending meshes

	bool						m_bMeshStarted;							// Did mesh editing start?
	int							m_nMaxQuadsCnt;							// Max no of quads

public:
	CBufferedPainterQuads();
	~CBufferedPainterQuads(void);


	// Allocates temp buffer of verts
	void						Init(int nMaxQuadsCnt);

	// Announce mesh editing start
	OPRESULT					BeginMesh(int &retMeshIdx);

	// Add quad to current mesh. Vertices must be in clockwise order
	OPRESULT					AddQuads(_VERTEX_PNCT4T4 *points, int quadsCount);

	// Announce mesh editing ended
	int							EndMesh();

	// Empties all buffers
	OPRESULT					ClearBuffers();

	// Builds vertex and index buffers. Call this before DrawMesh .
	OPRESULT					BuildBuffers();

	// Draws a mesh by index
	OPRESULT					DrawMesh(int meshIdx, bool setFVF = true);

	// Returns number of triangles in mesh
	const int					GetTrisCount(int meshIdx) const;

	OPRESULT OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL, void* pUserContext = NULL);
	OPRESULT OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL, void* pUserContext = NULL);
	OPRESULT OnLostDevice(void* pUserContext = NULL);
	OPRESULT OnDestroyDevice(void* pUserContext = NULL);
};


