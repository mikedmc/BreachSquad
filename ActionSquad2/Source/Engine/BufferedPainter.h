#pragma once


///-------------------------------------------------------------
///	 buffered sprites - deprecated class for drawing sprites (should be remade)
///-------------------------------------------------------------

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
	UINT32 m_nVertexCursor;

	LPDIRECT3DVERTEXBUFFER9 m_vb;
	LPDIRECT3DINDEXBUFFER9 m_ib;

	LPDIRECT3DDEVICE9 pDevice;
	//TODO: Poate ar fi bine sa il fac cu un template class ca sa ii specific in constructor ce fel de verts folosim. Sau ceva generic cu void*
	_VERTEX_PNCT4T4 *m_verts; //aici scrie tot si abia la flush face VB-ul
	//texture changes
	UINT32 m_nTexChangesCursor;
	LPDIRECT3DTEXTURE9 m_texPtrs[K_BS_MAX_TEXCHANGES_CNT];
	UINT32	m_nTrisPerTexture[K_BS_MAX_TEXCHANGES_CNT];
	UINT32  m_nTrisOffsets[K_BS_MAX_TEXCHANGES_CNT];
public:
	CBufferedSprites(void);
	~CBufferedSprites(void);

	HRESULT Begin(UINT32 flags = K_BS_ALPHABLENDING | K_BS_ALPHATEST | K_BS_MODULATE_COLORS);
	HRESULT End();
	HRESULT DrawBuffered(LPDIRECT3DTEXTURE9 pTexture, RECTXYXY_F *pSrcRectUV, RECTXYWH_F *pSrcCoord, D3DXVECTOR3 *pCenter, D3DXVECTOR3 *pPosition, DWORD color = 0xffffffff);
	HRESULT Flush();

	HRESULT OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL, void* pUserContext = NULL);
	HRESULT OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL, void* pUserContext = NULL);
	HRESULT OnLostDevice( void* pUserContext = NULL);
	HRESULT OnDestroyDevice( void* pUserContext = NULL);
};


///-----------------------------------------------------------------------------------------------
///	 BUFFERED PAINTER - well tested
///  Adds geometry to meshes identified by index and then builds VB and IB and draws them
///-----------------------------------------------------------------------------------------------
#define K_BP_SENTINEL 10

#define K_BP_MAX_TRIS_CNT 4000
#define K_BP_MAX_MESHES_CNT 100

class CBufferedPainter
{
protected:
	LPDIRECT3DVERTEXBUFFER9 m_vb;
	LPDIRECT3DINDEXBUFFER9 m_ib;

	LPDIRECT3DDEVICE9 pDevice;

	UINT32  m_nVertexCursor; //la ce vertex suntem in buffer
	_VERTEX_PNCT4T4 *m_verts; //aici scrie tot si abia la final face VB-ul
	
	UINT32	m_nTrisPerMesh[K_BP_MAX_MESHES_CNT]; //cate triunghiuri are fiecare mesh
	UINT32  m_nTrisOffsets[K_BP_MAX_MESHES_CNT]; //offsetul in VB pana la inceputul meshului
	int		m_nMeshesCnt;	//cate meshuri sunt
	bool	m_bMeshStarted;  //daca am inceput editarea unui mesh

public:
	CBufferedPainter(void);
	~CBufferedPainter(void);

	//Announce mesh editing start
	HRESULT BeginMesh(int &retMeshIdx);
	//Add triangle to current mesh
	HRESULT AddTriangles(_VERTEX_PNCT4T4 *points, int trisCount);
	//Announce mesh editing ended
	HRESULT EndMesh();
	//Empties all buffers
	HRESULT ClearBuffers();
	//Builds vertex and index buffers. Call this before DrawMesh .
	HRESULT BuildBuffers();
	//Draws a mesh by index
	HRESULT DrawMesh(int meshIdx, bool setFVF = true);
	//Returns number of triangles in mesh
	const int GetTrisCount(int meshIdx) const;

	HRESULT OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL, void* pUserContext = NULL);
	HRESULT OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL, void* pUserContext = NULL);
	HRESULT OnLostDevice(void* pUserContext = NULL);
	HRESULT OnDestroyDevice(void* pUserContext = NULL);
};


///----------------------------------------------------
/// Class that paints buffered meshes
/// Keeps track of texture and mode changes
/// Intended for small and fast meshes
///----------------------------------------------------
class CSpineTex;
class CBufferedTexPainter : private CBufferedPainter {
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

	int					passesCnt;							// total number of "passes" or "mode changes" needed to paint the meshes
	CPaintPassData		arrPasses[K_BP_MAX_MESHES_CNT];

public:
	CBufferedTexPainter();
	~CBufferedTexPainter();
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


