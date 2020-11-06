#pragma once

//--- ORDINEA NORMALA DE ADAUGARE IN STRUCTURA A DATELOR ---
//D3DFVF_XYZ       - position in 3D space
//D3DFVF_XYZRHW      - already transformed co-ordinate (2D space)
//D3DFVF_NORMAL      - normal
//D3DFVF_DIFFUSE     - diffuse colour
//D3DFVF_SPECULAR     - specular colour
//D3DFVF_TEX1       - one texture co-ordinate
//D3DFVF_TEX2       - two texture co-ordinates
//--- pt fixced pipeline nu schimba ordinea !!!! ---

//-=-=-= VERTEX TYPES =-=-=-
typedef struct _tagVERTEX_PNCT4T4
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 n;
	DWORD color;
	D3DXVECTOR4 tex1;
	D3DXVECTOR4 tex2;

	static const DWORD FVF;
} _VERTEX_PNCT4T4;

typedef struct _tagVERTEX_PT2T2
{
	D3DXVECTOR3 pos;
	D3DXVECTOR2 tex1;
	D3DXVECTOR2 tex2;

	static const DWORD FVF;
} _VERTEX_PT2T2;

typedef struct _tagVERTEX_PNCT
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 n;
	DWORD color;
	float tu, tv;

	static const DWORD FVF;
} _VERTEX_PNCT;

typedef struct _tagVERTEX_PNT
{
	D3DXVECTOR3 pos;
	D3DXVECTOR3 n;
	float tu, tv;

	static const DWORD FVF;
} _VERTEX_PNT;

typedef struct _tagVERTEX_PTC
{
	D3DXVECTOR3 pos;
	DWORD color;
	float tu, tv;

	static const DWORD FVF;
} _VERTEX_PTC;

typedef struct _tagVERTEX_PC
{
	D3DXVECTOR3 pos;
	DWORD color;

	static const DWORD FVF;
} _VERTEX_PC;

//--- setters ---
inline void SET_PNCT4T4(_VERTEX_PNCT4T4 * vert, Vec3 pos, Vec3 n, DWORD color, Vec4 tex1, Vec4 tex2)
{
	assert(vert != nullptr);
	vert->pos = pos;
	vert->n = n;
	vert->color = color;
	vert->tex1 = tex1;
	vert->tex2 = tex2;
}

//-=-=-= PS/VS nodes =-=-=-
struct VSnode
{
	WCHAR				szFilename[MAX_PATH];
	CStringHash			shName;	//friendly name
	LPDIRECT3DVERTEXSHADER9		pShader;

	VSnode() : pShader(null)
	{
		szFilename[0] = 0;
		shName.Reset();
	}
};

struct PSnode
{
	WCHAR				szFilename[MAX_PATH];
	CStringHash			shName; //friendly name
	LPDIRECT3DPIXELSHADER9		pShader;

	PSnode() : pShader(null)
	{
		szFilename[0] = 0;
		shName.Reset();
	}
};

class CShaderManager
{
public:
	//all vertex declarations are kept here
	LPDIRECT3DVERTEXDECLARATION9 _VERTEX_PNCT4T4_decl;
	LPDIRECT3DVERTEXDECLARATION9 _VERTEX_PNCT_decl;
	LPDIRECT3DVERTEXDECLARATION9 _VERTEX_PNT_decl;
	LPDIRECT3DVERTEXDECLARATION9 _VERTEX_PC_decl;
	LPDIRECT3DVERTEXDECLARATION9 _VERTEX_PT2T2_decl;

public:
	LPDIRECT3DDEVICE9			m_pd3dDevice;
	CGrowableArray<VSnode*>		VertexShaders;
	CGrowableArray<PSnode*>		PixelShaders;

	CShaderManager(void);
	~CShaderManager(void);
				 
	HRESULT LoadShaders(WCHAR* sXMLpath);

	HRESULT ClearAllVShaders(void);
	HRESULT	AddVShader(WCHAR * szPath, WCHAR * szFriendlyName, int * pnShaderIdx = null);

	HRESULT ClearAllPShaders(void);
	HRESULT	AddPShader(WCHAR * szPath, WCHAR * szFriendlyName, int * pnShaderIdx = null);

	LPDIRECT3DVERTEXSHADER9 GetVShader(int nShaderIdx);
	LPDIRECT3DVERTEXSHADER9 GetVShaderByName(WCHAR * szName);
	LPDIRECT3DVERTEXSHADER9 GetVShaderByNameHash(UINT32 nNameHash);

	LPDIRECT3DPIXELSHADER9 GetPShader(int nShaderIdx);
	LPDIRECT3DPIXELSHADER9 GetPShaderByName(WCHAR * szName);
	LPDIRECT3DPIXELSHADER9 GetPShaderByNameHash(UINT32 nNameHash);

	int		GetVShadersCount(void)	{ return VertexShaders.GetSize();	}
	int		GetPShadersCount(void)	{ return PixelShaders.GetSize();	}

	//vertex declarations are initialised here
	HRESULT CreateVertexDeclarations(LPDIRECT3DDEVICE9 pDevice);
	HRESULT ReleaseVertexDeclarations();

	HRESULT OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext = NULL);
	HRESULT OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext = NULL);
	HRESULT OnLostDevice( void* pUserContext = NULL);
	HRESULT OnDestroyDevice( void* pUserContext = NULL);
};


//declar singletonul
CShaderManager& UTGetShaderManager();
