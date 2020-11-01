#include "dxstdafx.h"
#include "ShaderManager.h"
//vertex declarations
const DWORD _VERTEX_PNCT4T4::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2;
const DWORD _VERTEX_PT2T2::FVF = D3DFVF_XYZ | D3DFVF_TEX2;
const DWORD _VERTEX_PNCT::FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1;
const DWORD _VERTEX_PNT::FVF = D3DFVF_XYZ | D3DFVF_NORMAL |  D3DFVF_TEX1;
const DWORD _VERTEX_PTC::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
const DWORD _VERTEX_PC::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;

//P position, N normal, T texture, C color
//vertex elements
D3DVERTEXELEMENT9 _VERTEX_PNCT4T4_ve[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
	{0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
	{0, 28, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	{0, 44, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
	D3DDECL_END()
};
D3DVERTEXELEMENT9 _VERTEX_PT2T2_ve[] =
{
	{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	{ 0, 44, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
	D3DDECL_END()
};
D3DVERTEXELEMENT9 _VERTEX_PNCT_ve[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
	{0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
	{0, 28, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END()
};
D3DVERTEXELEMENT9 _VERTEX_PNT_ve[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
	{0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END()
};
D3DVERTEXELEMENT9 _VERTEX_PTC_ve[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	{0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
	D3DDECL_END()
};
D3DVERTEXELEMENT9 _VERTEX_PC_ve[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
	D3DDECL_END()
};


/*
	D3DVERTEXELEMENT9 tween_decl_ve[] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{1, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		{2, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 1},
		{2, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 1 },
		D3DDECL_END()
	};
	pDevice->CreateVertexDeclaration(tween_decl_ve, &tween_decl);
	//                         //
	/////////////////////////////
	if (!tween_decl)
	{
		MessageBox(0,L"Creating morph vertex declaration failed.",L"Fatal Error",0);
		PostQuitMessage(0);
	}

	if(g_bVSCapable)
	{
		CHAR szPath[MAX_PATH];
		StringCchPrintfA(szPath, MAX_PATH, "%sShaders\\tween.vso", g_szExePath);
		if (FAILED(hr = C3DUtils::CreateVS(pDevice, szPath, &m_pTweenShader)))
		{
			DXTRACE_ERR(L"[CMorph::InitDeviceObjects] Failed to create tween vertex shader", hr);
			DebugLogA("Failed to create tween vertex shader.\n\t\t%s\n", szPath);
			return hr;
		}
		StringCchPrintfA(szPath, MAX_PATH, "%sShaders\\tween_specular.vso", g_szExePath);
		if (FAILED(hr = C3DUtils::CreateVS(pDevice, szPath, &m_pTweenSpecular)))
		{
			DXTRACE_ERR(L"[CMorph::InitDeviceObjects] Failed to create tween vertex shader", hr);
			DebugLogA("Failed to create tween vertex shader.\n\t\t%s\n", szPath);
			return hr;
		}
	}
*/

//-=-=-= functii utile creare shadere =-=-=-
HRESULT CreateVS(LPDIRECT3DDEVICE9 pd3dDevice, WCHAR *szPath, LPDIRECT3DVERTEXSHADER9 *pVS)
{
	HRESULT hr = S_OK;
	HANDLE hFile, hMap;
	DWORD *pdwVS;
	hFile = CreateFile(szPath, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile != INVALID_HANDLE_VALUE) 
	{
		if (GetFileSize(hFile, 0) > 0)
		{
			hMap = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);
		}
		else
		{
			CloseHandle(hFile);
			CHAR szErr[2048];
			GetErrorMessageA(GetLastError(), szErr, ARRAY_SIZE(szErr));
			ErrorBox(K_ERR_CRITICAL, L"[C3DUtils::CreateVS] Unable to get vs filesize.\n\t\t%s\n", szErr);
			return E_FAIL;		
		}
	}	
	else
	{
		CHAR szErr[2048];
		GetErrorMessageA(GetLastError(), szErr, ARRAY_SIZE(szErr));
		ErrorBox(K_ERR_CRITICAL, L"[C3DUtils::CreateVS] Unable to open vs file.\n\t\t%s\n", szErr);
		return E_FAIL;	
	}
	// maps a view of a file into the address space of the calling process
	pdwVS = (DWORD*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	if (FAILED(hr = pd3dDevice->CreateVertexShader(pdwVS, pVS)))
	{
		ErrorBox(K_ERR_CRITICAL, L"[C3DUtils::CreateVS] Failed to create vertex shader.\n\t\thresult=%x\n", hr);
		return hr;
	}

	UnmapViewOfFile(pdwVS);
	CloseHandle(hMap);
	CloseHandle(hFile);

	return S_OK;
}

HRESULT CreatePS(LPDIRECT3DDEVICE9 pd3dDevice, WCHAR *szPath, LPDIRECT3DPIXELSHADER9 *pPS)
{
	HRESULT hr = S_OK;
	HANDLE hFile, hMap;
	DWORD *pdwPS;
	hFile = CreateFile(szPath, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile != INVALID_HANDLE_VALUE) 
	{
		if (GetFileSize(hFile, 0) > 0)
		{
			hMap = CreateFileMapping(hFile, 0, PAGE_READONLY, 0, 0, 0);
		}
		else
		{
			CloseHandle(hFile);
			CHAR szErr[2048];
			GetErrorMessageA(GetLastError(), szErr, ARRAY_SIZE(szErr));
			ErrorBox(K_ERR_CRITICAL, L"[C3DUtils::CreatePS] Unable to get PS filesize.\n\t\t%s\n", szErr);
			return E_FAIL;		
		}
	}	
	else
	{
		CHAR szErr[2048];
		GetErrorMessageA(GetLastError(), szErr, ARRAY_SIZE(szErr));
		ErrorBox(K_ERR_CRITICAL, L"[C3DUtils::CreatePS] Unable to open PS file.\n\t\t%s\n", szErr);
		return E_FAIL;	
	}
	// maps a view of a file into the address space of the calling process
	pdwPS = (DWORD*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	if (FAILED(hr = pd3dDevice->CreatePixelShader(pdwPS, pPS)))
	{
		ErrorBox(K_ERR_CRITICAL, L"[C3DUtils::CreatePS] Failed to create vertex shader.\n\t\thresult=%x\n", hr);
		return hr;
	}

	UnmapViewOfFile(pdwPS);
	CloseHandle(hMap);
	CloseHandle(hFile);

	return S_OK;
}
//=-=-=- pana aici  =-=-=-

CShaderManager::CShaderManager(void)
{
	_VERTEX_PNT_decl = NULL;
	_VERTEX_PNCT_decl = NULL;
	_VERTEX_PNCT4T4_decl = NULL;
}
CShaderManager::~CShaderManager(void)
{
	ClearAllVShaders();
	ClearAllPShaders();
	ReleaseVertexDeclarations();
}

/* Formatul incarcat:
<Shaders>
	<VertexShaders>
		<VextexShader name="VS_TEXspot" path="texSpot.vso" />
	</VertexShaders>
	<PixelShaders>
		<PixelShader name="PS_FOW" path="fow.pso" />
	</PixelShaders>
</Shaders>
*/
HRESULT CShaderManager::LoadShaders(WCHAR* sXMLpath)
{
	//gaseste calea fisierului XML
	WCHAR wsPathXML[MAX_PATH];
	StringCchCopy(wsPathXML, MAX_PATH, sXMLpath);
	int nIdx = (int)wcslen(wsPathXML);
	while (--nIdx > 0 && wsPathXML[nIdx] != '\\' && wsPathXML[nIdx] != '/');
	wsPathXML[nIdx + 1] = '\0';


	pugi::xml_document doc;
	if (!doc.load_file(sXMLpath))
	{
		ErrorBox(K_ERR_WARNING, L"CShaderManager::Unable to load XML:%s\n", sXMLpath);
		return(E_FAIL);
	}

	pugi::xml_node shadersNode = doc.root().child(L"Shaders");

	//ia parintele listei de strings
	pugi::xml_node vsnodeparent = shadersNode.child(L"VertexShaders");
	if (vsnodeparent != null)
	{
		for (pugi::xml_node vsnode = vsnodeparent.first_child(); vsnode; vsnode = vsnode.next_sibling())
		{
			WCHAR wsPath[MAX_PATH];
			WCHAR wsName[MAX_PATH];
			StringCchPrintf(wsPath, MAX_PATH, L"%s%s", wsPathXML, vsnode.attribute(L"path").value());
			StringCchCopy(wsName, MAX_PATH, vsnode.attribute(L"name").value());
			if (FAILED(UTGetShaderManager().AddVShader(wsPath, wsName)))
			{
				ErrorBox(K_ERR_CRITICAL, L"Couldn't load vertex shader: %s", wsPath);
			}
		}
	}

	pugi::xml_node psnodeparent = shadersNode.child(L"PixelShaders");
	if (psnodeparent != null)
	{
		for (pugi::xml_node psnode = psnodeparent.first_child(); psnode; psnode = psnode.next_sibling())
		{
			WCHAR wsPath[MAX_PATH];
			WCHAR wsName[MAX_PATH];
			StringCchPrintf(wsPath, MAX_PATH, L"%s%s", wsPathXML, psnode.attribute(L"path").value());
			StringCchCopy(wsName, MAX_PATH, psnode.attribute(L"name").value());
			if (FAILED(UTGetShaderManager().AddPShader(wsPath, wsName)))
			{
				ErrorBox(K_ERR_CRITICAL, L"Couldn't load pixel shader: %s", wsPath);
			}
		}
	}

	return S_OK;
}

HRESULT CShaderManager::ClearAllVShaders(void)
{
	for(int ii=0; ii<VertexShaders.GetSize(); ii++)
	{
		SAFE_RELEASE(VertexShaders[ii]->pShader);
		SAFE_DELETE(VertexShaders[ii]);
	}
	VertexShaders.RemoveAll();
	return S_OK;
}

HRESULT CShaderManager::ClearAllPShaders(void)
{
	for(int ii=0; ii<PixelShaders.GetSize(); ii++)
	{
		SAFE_RELEASE(PixelShaders[ii]->pShader);
		SAFE_DELETE(PixelShaders[ii]);
	}
	PixelShaders.RemoveAll();
	return S_OK;
}


HRESULT	CShaderManager::AddVShader(WCHAR *szPath, WCHAR * szFriendlyName, int * pnShaderIdx)
{
	HRESULT hr = S_OK;

	if (szPath == NULL || wcscmp(szPath, L"") == 0)
		return S_OK;

    for (int i = 0; i < VertexShaders.GetSize(); i++)
	{
		VSnode* pVSN = VertexShaders.GetAt(i);
		if ((pVSN->shName.IsEqual(szFriendlyName)) || (wcscmp(szPath, pVSN->szFilename) == 0))
		{
			// The shader already exists
			if (pnShaderIdx)
				*pnShaderIdx = i;
			return S_OK;
		}
	}

	// Add the new shader
	VSnode *pNewVS = new VSnode();
	if (pNewVS == NULL)
		return E_OUTOFMEMORY;

	ZeroMemory(pNewVS, sizeof(VSnode));
	StringCchCopy(pNewVS->szFilename, MAX_PATH, szPath);
	//shader name
	pNewVS->shName.Init(szFriendlyName);
	//face shaderul
	if (FAILED(hr = CreateVS(m_pd3dDevice, szPath, &pNewVS->pShader)))
	{
		ErrorBox(K_ERR_CRITICAL, L"Failed to create vertex shader.\n\t\t%s\n", szPath);
		return hr;
	}
	
	VertexShaders.Add(pNewVS);
	if (pnShaderIdx)
		*pnShaderIdx = VertexShaders.GetSize() - 1;
	return S_OK;
}

HRESULT	CShaderManager::AddPShader(WCHAR * szPath, WCHAR * szFriendlyName, int * pnShaderIdx)
{
	HRESULT hr = S_OK;

	if (szPath == NULL || wcscmp(szPath, L"") == 0)
		return S_OK;

    for (int i = 0; i < PixelShaders.GetSize(); i++)
	{
		PSnode* pPSN = PixelShaders[i];
		if ((pPSN->shName.IsEqual(szFriendlyName)) || (wcscmp(szPath, pPSN->szFilename) == 0))
		{
			// The shader already exists
			if(pnShaderIdx)
				*pnShaderIdx = i;
			return S_OK;
		}
	}

	// Add the new shader
	PSnode *pNewPS = new PSnode();
	if (pNewPS == NULL)
		return E_OUTOFMEMORY;

	ZeroMemory(pNewPS, sizeof(PSnode));
	StringCchCopy(pNewPS->szFilename, MAX_PATH, szPath);
	//set friendly name
	pNewPS->shName.Init(szFriendlyName);
	//face shaderul
	if (FAILED(hr = CreatePS(m_pd3dDevice, szPath, &pNewPS->pShader)))
	{
		ErrorBox(K_ERR_CRITICAL, L"Failed to create pixel shader.\n\t\t%s\n", szPath);
		return hr;
	}
	
	PixelShaders.Add(pNewPS);
	if (pnShaderIdx)
		*pnShaderIdx = PixelShaders.GetSize() - 1;
	return S_OK;
}


LPDIRECT3DVERTEXSHADER9 CShaderManager::GetVShader(int nShaderIdx)
{
	return VertexShaders[nShaderIdx]->pShader;
}

LPDIRECT3DVERTEXSHADER9 CShaderManager::GetVShaderByName(WCHAR * szName)
{
	if (szName == NULL || wcscmp(szName, L"") == 0)
	{
		return NULL;
	}

	UINT32 namehash = FastHash(szName);
	for (int i = 0; i < VertexShaders.GetSize(); i++)
	{
		VSnode* pVS = VertexShaders[i];
		if (pVS->shName.textHash == namehash)
		{
			return pVS->pShader;
		}
	}

	ErrorBox(K_ERR_WARNING, L"Vertex Shader not found!\n\t%s", szName);
	return NULL;
}

LPDIRECT3DVERTEXSHADER9 CShaderManager::GetVShaderByNameHash(UINT32 nNameHash)
{
	if (nNameHash == 0)
		return NULL;

	for (int i = 0; i < VertexShaders.GetSize(); i++)
	{
		VSnode* pVS = VertexShaders[i];
		if (pVS->shName.textHash == nNameHash)
		{
			return pVS->pShader;
		}
	}

	return NULL;
}

LPDIRECT3DPIXELSHADER9 CShaderManager::GetPShader(int nShaderIdx)
{
	return PixelShaders[nShaderIdx]->pShader;
}

LPDIRECT3DPIXELSHADER9 CShaderManager::GetPShaderByName(WCHAR * szName)
{
	if (szName == NULL || wcscmp(szName, L"") == 0)
	{
		return NULL;
	}

	UINT32 namehash = FastHash(szName);
	for (int i = 0; i < PixelShaders.GetSize(); i++)
	{
		PSnode* pPS = PixelShaders[i];
		if (pPS->shName.textHash == namehash)
		{
			return pPS->pShader;
		}
	}

	ErrorBox(K_ERR_WARNING, L"Pixel Shader not found!\n\t%s", szName);
	return NULL;
}

LPDIRECT3DPIXELSHADER9 CShaderManager::GetPShaderByNameHash(UINT32 nNameHash)
{
	if (nNameHash == 0)
		return NULL;

	for (int i = 0; i < PixelShaders.GetSize(); i++)
	{
		PSnode* pPS = PixelShaders[i];
		if (pPS->shName.textHash == nNameHash)
		{
			return pPS->pShader;
		}
	}

	return NULL;
}

//-=-=-= vertex declarations =-=-=-
HRESULT CShaderManager::CreateVertexDeclarations(LPDIRECT3DDEVICE9 pDevice)
{
	HRESULT hr = S_OK;
	V_RETURN(pDevice->CreateVertexDeclaration(_VERTEX_PNCT4T4_ve, &_VERTEX_PNCT4T4_decl));
	V_RETURN(pDevice->CreateVertexDeclaration(_VERTEX_PNCT_ve, &_VERTEX_PNCT_decl));
	V_RETURN(pDevice->CreateVertexDeclaration(_VERTEX_PNT_ve, &_VERTEX_PNT_decl));
	V_RETURN(pDevice->CreateVertexDeclaration(_VERTEX_PC_ve, &_VERTEX_PC_decl));
	V_RETURN(pDevice->CreateVertexDeclaration(_VERTEX_PT2T2_ve, &_VERTEX_PT2T2_decl));

	return S_OK;
}
HRESULT CShaderManager::ReleaseVertexDeclarations()
{
	SAFE_RELEASE(_VERTEX_PNCT4T4_decl);
	SAFE_RELEASE(_VERTEX_PNCT_decl);
	SAFE_RELEASE(_VERTEX_PNT_decl);
	SAFE_RELEASE(_VERTEX_PC_decl);
	SAFE_RELEASE(_VERTEX_PT2T2_decl);

	return S_OK;
}

//=-=-=- DEVICE FUNCTIONS -=-=-=
HRESULT CShaderManager::OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	m_pd3dDevice = pd3dDevice;
	return S_OK;
}

HRESULT CShaderManager::OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	m_pd3dDevice = pd3dDevice;
	HRESULT hr = CreateVertexDeclarations(pd3dDevice);
	if(FAILED(hr))
	{
		ErrorBox(K_ERR_CRITICAL, L"CShaderManager::OnResetDevice->Failed to createVertexDeclarations()\n");
		return   hr;
	}

	//reface shaderele
	for(int ii=0; ii<VertexShaders.GetSize(); ii++)
	{
		if (FAILED(hr = CreateVS(m_pd3dDevice, VertexShaders[ii]->szFilename, &VertexShaders[ii]->pShader)))
		{
			ErrorBox(K_ERR_CRITICAL, L"Failed to re-create vertex shader.\n\t\t%s\n", VertexShaders[ii]->szFilename);
			return hr;
		}
	}
	for(int ii=0; ii<PixelShaders.GetSize(); ii++)
	{
		if (FAILED(hr = CreatePS(m_pd3dDevice, PixelShaders[ii]->szFilename, &PixelShaders[ii]->pShader)))
		{
			ErrorBox(K_ERR_CRITICAL, L"Failed to re-create pixel shader.\n\t\t%s\n", PixelShaders[ii]->szFilename);
			return hr;
		}
	}

	if(FAILED(hr))
	{
		return   hr;
	}
	return S_OK;
}

HRESULT CShaderManager::OnLostDevice( void* pUserContext )
{
	ReleaseVertexDeclarations();
	for(int ii=0; ii<VertexShaders.GetSize(); ii++)
	{
		SAFE_RELEASE(VertexShaders[ii]->pShader);
	}
	for(int ii=0; ii<PixelShaders.GetSize(); ii++)
	{
		SAFE_RELEASE(PixelShaders[ii]->pShader);
	}
	return S_OK;
}

HRESULT CShaderManager::OnDestroyDevice( void* pUserContext )
{
	return S_OK;
}



///**************************************************************************************
/// Sigleton de acces
///**************************************************************************************

CShaderManager& UTGetShaderManager()
{
	static CShaderManager g_ShaderMgr;
	return g_ShaderMgr;
}

