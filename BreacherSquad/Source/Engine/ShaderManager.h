#pragma once

//--- DX order of elements in fixed pipeline structures ---
//D3DFVF_XYZ			- position in 3D space
//D3DFVF_XYZRHW			- already transformed co-ordinate (2D space)
//D3DFVF_NORMAL			- normal
//D3DFVF_DIFFUSE		- diffuse colour
//D3DFVF_SPECULAR		- specular colour
//D3DFVF_TEX1			- one texture co-ordinate
//D3DFVF_TEX2			- two texture co-ordinates
//--- for fixed pipeline don't change order in structs ---

//-=-=-= VERTEX TYPES =-=-=-
typedef struct _VERTEX_PNCT4T4
{
	Vec3 pos;
	Vec3 n;
	DWORD color;
	Vec4 tex1;
	Vec4 tex2;

	static const DWORD FVF;
};

typedef struct _tagVERTEX_PT2T2
{
	Vec3 pos;
	Vec2 tex1;
	Vec2 tex2;

	static const DWORD FVF;
} _VERTEX_PT2T2;

typedef struct _tagVERTEX_PNCT
{
	Vec3 pos;
	Vec3 n;
	DWORD color;
	float tu, tv;

	static const DWORD FVF;
} _VERTEX_PNCT;

typedef struct _tagVERTEX_PNT
{
	Vec3 pos;
	Vec3 n;
	float tu, tv;

	static const DWORD FVF;
} _VERTEX_PNT;

typedef struct _tagVERTEX_PTC
{
	Vec3 pos;
	DWORD color;
	float tu, tv;

	static const DWORD FVF;
} _VERTEX_PTC;

typedef struct _tagVERTEX_PC
{
	Vec3 pos;
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
	PVERTEXSHADER		pShader;

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
	PPIXELSHADER		pShader;

	PSnode() : pShader(null)
	{
		szFilename[0] = 0;
		shName.Reset();
	}
};

enum eVertexDeclarationType {
	K_SHM_PNCT4T4,
	K_SHM_PNCT,
	K_SHM_PT4T4,
};

class CShaderManager
{
public:
	//all vertex declarations are kept here
	PVERTEXDECL _VERTEX_PNCT4T4_decl;
	PVERTEXDECL _VERTEX_PNCT4_decl;
	PVERTEXDECL _VERTEX_PCT4T4_decl;

public:
	PDEVICE					pDevice;
	CArray<VSnode*>			VertexShaders;
	CArray<PSnode*>			PixelShaders;

	CShaderManager(void);
	~CShaderManager(void);
				 
	OPRESULT				AddShadersFromXML(WCHAR* sXMLpath);

	OPRESULT				ClearAllVShaders(void);
	OPRESULT				AddVShader(WCHAR * szPath, WCHAR * szFriendlyName, int* pnShaderIdx = nullptr);

	OPRESULT				ClearAllPShaders(void);
	OPRESULT				AddPShader(WCHAR * szPath, WCHAR * szFriendlyName, int* pnShaderIdx = nullptr);

	PVERTEXSHADER			GetVShader(int nShaderIdx);
	PVERTEXSHADER			GetVShaderByName(WCHAR * szName);
	PVERTEXSHADER			GetVShaderByNameHash(UINT32 nNameHash);

	PPIXELSHADER			GetPShader(int nShaderIdx);
	PPIXELSHADER			GetPShaderByName(WCHAR * szName);
	PPIXELSHADER			GetPShaderByNameHash(UINT32 nNameHash);

	int						GetVShadersCount(void)	{ return VertexShaders.GetSize();	}
	int						GetPShadersCount(void)	{ return PixelShaders.GetSize();	}

	// Reloads all shaders instantly (debug only)
	void					ReloadAllShaders();

	//vertex declarations are initialised here
	OPRESULT				CreateVertexDeclarations();
	OPRESULT				ReleaseVertexDeclarations();

	// sets a VS by name or nullptr if name is null
	OPRESULT				SetVS(PVERTEXSHADER pShader);
	OPRESULT				SetVSByName(WCHAR* shaderName);
	OPRESULT				SetVSConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount);
	OPRESULT				SetVertexDeclaration(eVertexDeclarationType vtype);

	// sets a PS by name or nullptr if name is null
	OPRESULT				SetPS(PPIXELSHADER pShader);
	OPRESULT				SetPSByName(WCHAR* shaderName);
	OPRESULT				SetPSConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount);


	OPRESULT				OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc, void* pUserContext = NULL);
	OPRESULT				OnResetDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc, void* pUserContext = NULL);
	OPRESULT				OnLostDevice( void* pUserContext = NULL);
	OPRESULT				OnDestroyDevice( void* pUserContext = NULL);
};


// Shader Manager singleton. Handles loading for all VS and PS
CShaderManager& __Shaders();
