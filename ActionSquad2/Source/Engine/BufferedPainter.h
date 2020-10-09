#pragma once


///-------------------------------------------------------------
///	 buffered sprites 
///-------------------------------------------------------------

#define K_BS_MAX_QUAD_CNT 2000
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

	D3DXMATRIXA16 m_matWorld;
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

	HRESULT Flush();
	
	//--- old draw methods ---
	HRESULT DrawBuffered(LPDIRECT3DTEXTURE9 pTexture, RECTXYXY_F *pSrcRectUV, RECTXYWH_F *pSrcCoord, D3DXVECTOR3 *pCenter, D3DXVECTOR3 *pPosition, DWORD color = 0xffffffff);
	//aplica o matrice inainte sa aplice pozitia -> folositor la particule
	HRESULT DrawBufferedTransformed(LPDIRECT3DTEXTURE9 pTexture, RECTXYXY_F *pSrcRectUV, RECTXYWH_F *pSrcCoord, D3DXVECTOR3 *pCenter, D3DXMATRIXA16* coordtrans, D3DXVECTOR3 *pPosition, DWORD color = 0xffffffff);
	HRESULT Draw4VertsSpriteBuffered(LPDIRECT3DTEXTURE9 pTexture, _VERTEX_PNCT4T4 *verts, D3DXVECTOR3 *pos, DWORD color);

	void	SetTransformWorld(D3DXMATRIXA16 *matWrld);

	HRESULT OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL, void* pUserContext = NULL);
	HRESULT OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL, void* pUserContext = NULL);
	HRESULT OnLostDevice( void* pUserContext = NULL);
	HRESULT OnDestroyDevice( void* pUserContext = NULL);
};


///-----------------------------------------------------------------------------------------------
///	 BUFFERED PAINTER - tempalte class
///  - adaugi triunghiuri unul cate unul si la final face un VB si IB din care poti desena mesh-ul
///-----------------------------------------------------------------------------------------------
#define K_BP_SENTINEL 10

#define K_BP_MAX_TRIS_CNT 3000
#define K_BP_MAX_MESHES_CNT 100

class CBufferedPainter
{
private:
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

	//Se cheama inainte sa adaugi triunghiuri ca sa iei idx-ul meshului pe care il construiesti
	HRESULT BeginMesh(UINT32 &retMeshIdx);
	//Adauga un triunghi in meshul curent
	HRESULT AddTriangles(_VERTEX_PNCT4T4 *points, int trisCount);
	//Am terminat de editat un mesh ca sa stie sa treaca la urmatorul
	HRESULT EndMesh();
	//Se cheama dupa ce ai terminat treaba ca sa golesti bufferele
	HRESULT ClearBuffers();
	//Se cheama ca sa creeze VB si IB ca sa poti desena din ele
	HRESULT BuildBuffers();
	//Se cheama ca sa desenezi un mesh
	HRESULT DrawMesh(int meshIdx, bool setFVF = true);
	//Intoarce numarul de triunghiuri din mesh
	const int GetTrisCount(int meshIdx) const;

	HRESULT OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL, void* pUserContext = NULL);
	HRESULT OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL, void* pUserContext = NULL);
	HRESULT OnLostDevice(void* pUserContext = NULL);
	HRESULT OnDestroyDevice(void* pUserContext = NULL);
};
