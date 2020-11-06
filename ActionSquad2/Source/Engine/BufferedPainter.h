#pragma once


///-------------------------------------------------------------
///	 buffered sprites - deprecated class for drawing sprites (should be remade)
///-------------------------------------------------------------

/*
#define K_BS_MAX_QUAD_CNT 4000
#define K_BS_MAX_TEXCHANGES_CNT 50
//--- begin flags ---
//enables alpha bleding
#define K_BS_ALPHABLENDING 1
//enables alpha testing
#define K_BS_ALPHATEST 2
//set = modulates vertex and texture colors / not set = uses texture colors
#define K_BS_MODULATE_COLORS 4

class CBufferedSprites
{
private:
	UINT32				m_nVertexCursor;

	PVERTEXBUFFER		m_vb;
	PINDEXBUFFER		m_ib;

	PDEVICE				m_pDevice;
	_VERTEX_PNCT4T4 *m_verts; //aici scrie tot si abia la flush face VB-ul
	//texture changes
	UINT32 m_nTexChangesCursor;
	PTEXTURE m_texPtrs[K_BS_MAX_TEXCHANGES_CNT];
	UINT32	m_nTrisPerTexture[K_BS_MAX_TEXCHANGES_CNT];
	UINT32  m_nTrisOffsets[K_BS_MAX_TEXCHANGES_CNT];
public:
	CBufferedSprites(void);
	~CBufferedSprites(void);

	HRESULT Begin(UINT32 flags = K_BS_ALPHABLENDING | K_BS_ALPHATEST | K_BS_MODULATE_COLORS);
	HRESULT End();
	HRESULT DrawBuffered(PTEXTURE pTexture, RECTXYXY_F *pSrcRectUV, RECTXYWH_F *pSrcCoord, Vec3 *pCenter, Vec3 *pPosition, DWORD color = 0xffffffff);
	HRESULT Flush();

	HRESULT OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL, void* pUserContext = NULL);
	HRESULT OnResetDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL, void* pUserContext = NULL);
	HRESULT OnLostDevice( void* pUserContext = NULL);
	HRESULT OnDestroyDevice( void* pUserContext = NULL);
};
 */

///-----------------------------------------------------------------------------------------------
///	 BUFFERED PAINTER TRIANGLES
///  Adds geometry to meshes identified by index and then builds VB and IB and draws them
///-----------------------------------------------------------------------------------------------
#define K_BP_SENTINEL_TRIS		10
#define K_BP_MAX_MESHES_CNT		100

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
	};

	int						passesCnt;								// total number of "passes" or "mode changes" needed to paint the meshes
	CPaintPassData			arrPasses[K_BP_MAX_MESHES_CNT]{};

public:
	CBufferedSpinePainter();
	~CBufferedSpinePainter();

	using				CBufferedPainter::Init;

	// Adds triangles to the paint buffer (CSpineTex can be replaced with texture pointer)
	void				BufferMesh(_VERTEX_PNCT4T4 *points, int trisCount, CSpineTex* pTex, EBlendMode eMode);

	// Creates the actual vertex buffer with the data
	// Could be called inside Paint, left public for more granular control
	using				CBufferedPainter::BuildBuffers;

	// Clears all buffers
	void				Clear();

	// Paints all buffered meshes
	void				Paint(bool setFVF = true, ETexChannel eChannel = K_TEXCHAN_COLORMAP);

	//make parent framework methods public with "using"
	using				CBufferedPainter::OnCreateDevice;
	using				CBufferedPainter::OnResetDevice;
	using				CBufferedPainter::OnLostDevice;
	using				CBufferedPainter::OnDestroyDevice;
};


