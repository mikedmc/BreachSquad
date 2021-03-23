#pragma once


///-----------------------------------------------------------------------------------------------
///	 BUFFERED PAINTER TRIANGLES
///  Adds geometry to meshes identified by index and then builds VB and IB and draws them
///-----------------------------------------------------------------------------------------------
#define K_BP_SENTINEL_TRIS		10
#define K_BP_MAX_MESHES_CNT		200

class CBufferedPainter
{
protected:
	PVERTEXBUFFER				m_vb;
	PINDEXBUFFER				m_ib;

	PDEVICE						m_pDevice;

	UINT32						m_nVertexCursor;						// Current vertex index in buffer (writing position)
	_VERTEX_PNCT4T4				*m_verts;								// temp buffer for storing vertices
	
	UINT32						m_nTrisPerMesh[K_BP_MAX_MESHES_CNT]{};	// Triangle count per mesh
	UINT32						m_nTrisOffsets[K_BP_MAX_MESHES_CNT]{};	// VB offset per mesh
	int							m_nMeshesCnt;							// Total no of pending meshes

	bool						m_bMeshStarted;							// Did mesh editing start?
	int							m_nMaxTrisCnt;							// Max no of tris

public:
	CBufferedPainter();
	~CBufferedPainter(void);


	// Allocates temp buffer of verts
	void						Init(int nMaxTrisCnt);

	// Announce mesh editing start
	OPRESULT					BeginMesh(int &retMeshIdx);

	// Add triangle to current mesh
	OPRESULT					AddTriangles(_VERTEX_PNCT4T4 *points, int trisCount);

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


///----------------------------------------------------
/// Class that paints buffered meshes
/// Keeps track of texture and mode changes
/// Intended for small and fast meshes
///----------------------------------------------------
class CSpineTex;
class CBufferedSpinePainter : private CBufferedPainter {
public:
	enum EBlendMode {
		BLEND_NORMAL = 0,
		BLEND_ADDITIVE = 1,
		BLEND_MULTIPLY,
		BLEND_SCREEN
	};

private:
	// It holds data about each pass (mode change) in the buffer
	struct CPaintPassData {
		EBlendMode			eMode;
		CSpineTex*			pTex;
		int					nMeshIdx;
		UINT				dwSkelUID;			// UID of current pass, usually corresponds to skeleton UID but it can be 0 if we don't need to split meshes by UID
	};

	int						passesCnt;								// total number of "passes" or "mode changes" needed to paint the meshes
	CPaintPassData			arrPasses[K_BP_MAX_MESHES_CNT]{};

public:
	CBufferedSpinePainter();
	~CBufferedSpinePainter();

	using				CBufferedPainter::Init;

	// Adds triangles to the paint buffer (CSpineTex can be replaced with texture pointer)
	// Returns pass index when starting a new mesh or -1 if using already started mesh
	int					BufferMesh(_VERTEX_PNCT4T4 *points, int trisCount, CSpineTex* pTex, EBlendMode eMode, UINT dwUID = 0);

	// Creates the actual vertex buffer with the data
	// Could be called inside Paint, left public for more granular control
	using				CBufferedPainter::BuildBuffers;

	// Clears all buffers
	void				Clear();

	// Paints all buffered meshes
	void				Paint(bool setFVF = true, ETexChannel eChannel = K_TEXCHAN_COLORMAP);
	// Paints one single pass from the passes array, by index
	OPRESULT			PaintPass(int nPassIdx, bool setFVF = true, ETexChannel eChannel = K_TEXCHAN_COLORMAP);

	//make parent framework methods public with "using"
	using				CBufferedPainter::OnCreateDevice;
	using				CBufferedPainter::OnResetDevice;
	using				CBufferedPainter::OnLostDevice;
	using				CBufferedPainter::OnDestroyDevice;
};


