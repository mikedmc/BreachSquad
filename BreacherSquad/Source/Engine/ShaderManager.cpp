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
D3DVERTEXELEMENT9 _VERTEX_PCT4T4_ve[] =
{
	{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
	{ 0, 16, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	{ 0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
	D3DDECL_END()
};
D3DVERTEXELEMENT9 _VERTEX_PNCT4_ve[] =
{
	{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
	{0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
	{0, 28, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
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
OPRESULT CreateVS(PDEVICE pd3dDevice, WCHAR *szPath, PVERTEXSHADER *pVS)
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
			WCHAR wszErr[2048];
			GetErrorMessageW(GetLastError(), wszErr, ARRAY_SIZE(wszErr));
			return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"[C3DUtils::CreateVS] Unable to get vs filesize.\n\t\t%s\n", wszErr);
		}
	}	
	else
	{
		WCHAR wszErr[2048];
		GetErrorMessageW(GetLastError(), wszErr, ARRAY_SIZE(wszErr));
		return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"[C3DUtils::CreateVS] Unable to open vs file.\n\t\t%s\n", wszErr);
	}
	// maps a view of a file into the address space of the calling process
	pdwVS = (DWORD*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	if (FAILED(hr = pd3dDevice->CreateVertexShader(pdwVS, pVS)))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"[C3DUtils::CreateVS] Failed to create vertex shader.\n\t\tOPRESULT=%x\n", hr);
	}

	UnmapViewOfFile(pdwVS);
	CloseHandle(hMap);
	CloseHandle(hFile);

	return K_OP_OK;
}

OPRESULT CreatePS(PDEVICE pd3dDevice, WCHAR *szPath, PPIXELSHADER *pPS)
{
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
			WCHAR wszErr[2048];
			GetErrorMessageW(GetLastError(), wszErr, ARRAY_SIZE(wszErr));
			return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"[C3DUtils::CreatePS] Unable to get PS filesize.\n\t\t%s\n", wszErr);
		}
	}	
	else
	{
		WCHAR wszErr[2048];
		GetErrorMessageW(GetLastError(), wszErr, ARRAY_SIZE(wszErr));
		return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"[C3DUtils::CreatePS] Unable to open PS file.\n\t\t%s\n", wszErr);
	}
	// maps a view of a file into the address space of the calling process
	pdwPS = (DWORD*)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	HRESULT hr = S_OK;
	if (FAILED(hr = pd3dDevice->CreatePixelShader(pdwPS, pPS)))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"[C3DUtils::CreatePS] Failed to create vertex shader.\n\t\tOPRESULT=%x\n", hr);
	}

	UnmapViewOfFile(pdwPS);
	CloseHandle(hMap);
	CloseHandle(hFile);

	return K_OP_OK;
}
//=-=-=- pana aici  =-=-=-

CShaderManager::CShaderManager(void)
{
	_VERTEX_PNCT4T4_decl = null;
	_VERTEX_PNCT4_decl = null;
	_VERTEX_PCT4T4_decl = null;
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
OPRESULT CShaderManager::AddShadersFromXML(WCHAR* sXMLpath)
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
		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CShaderManager::Unable to load XML:%s\n", sXMLpath);
	}

	pugi::xml_node shadersNode = doc.root().child(L"Shaders");

	pugi::xml_node vsnodeparent = shadersNode.child(L"VertexShaders");
	if (vsnodeparent != null)
	{
		for (pugi::xml_node vsnode = vsnodeparent.first_child(); vsnode; vsnode = vsnode.next_sibling())
		{
			WCHAR wsPath[MAX_PATH];
			WCHAR wsName[MAX_PATH];
			StringCchPrintf(wsPath, MAX_PATH, L"%s%s", wsPathXML, vsnode.attribute(L"path").value());
			StringCchCopy(wsName, MAX_PATH, vsnode.attribute(L"name").value());
			if (OP_FAILED(AddVShader(wsPath, wsName)))
			{
				return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"Couldn't load vertex shader: %s", wsPath);
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
			if (FAILED(AddPShader(wsPath, wsName)))
			{
				return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"Couldn't load pixel shader: %s", wsPath);
			}
		}
	}

	return K_OP_OK;
}

OPRESULT CShaderManager::ClearAllVShaders(void)
{
	for(int ii=0; ii<VertexShaders.GetSize(); ii++)
	{
		SAFE_RELEASE(VertexShaders[ii]->pShader);
		SAFE_DELETE(VertexShaders[ii]);
	}
	VertexShaders.RemoveAll();
	return K_OP_OK;
}

OPRESULT CShaderManager::ClearAllPShaders(void)
{
	for(int ii=0; ii<PixelShaders.GetSize(); ii++)
	{
		SAFE_RELEASE(PixelShaders[ii]->pShader);
		SAFE_DELETE(PixelShaders[ii]);
	}
	PixelShaders.RemoveAll();
	return K_OP_OK;
}


OPRESULT CShaderManager::AddVShader(WCHAR *szPath, WCHAR * szFriendlyName, int * pnShaderIdx)
{
	if (szPath == NULL || wcscmp(szPath, L"") == 0)
		return K_OP_OK;

    for (int i = 0; i < VertexShaders.GetSize(); i++)
	{
		VSnode* pVSN = VertexShaders.GetAt(i);
		if ((pVSN->shName.IsEqual(szFriendlyName)) || (wcscmp(szPath, pVSN->szFilename) == 0))
		{
			// The shader already exists
			if (pnShaderIdx)
				*pnShaderIdx = i;
			return K_OP_OK;
		}
	}

	// Add the new shader
	VSnode *pNewVS = new VSnode();
	if (pNewVS == NULL)
		return K_OP_FAILED;

	ZeroMemory(pNewVS, sizeof(VSnode));
	StringCchCopy(pNewVS->szFilename, MAX_PATH, szPath);
	// shader name
	pNewVS->shName.Init(szFriendlyName);
	// creates shader
	HRESULT hr = S_OK;
	if (FAILED(hr = CreateVS(pDevice, szPath, &pNewVS->pShader)))
	{
		return K_OP_FAILED;
	}
	
	VertexShaders.Add(pNewVS);
	if (pnShaderIdx)
		*pnShaderIdx = VertexShaders.GetSize() - 1;
	return K_OP_OK;
}

OPRESULT CShaderManager::AddPShader(WCHAR * szPath, WCHAR * szFriendlyName, int * pnShaderIdx)
{
	if (szPath == NULL || wcscmp(szPath, L"") == 0)
		return K_OP_OK;

    for (int i = 0; i < PixelShaders.GetSize(); i++)
	{
		PSnode* pPSN = PixelShaders[i];
		if ((pPSN->shName.IsEqual(szFriendlyName)) || (wcscmp(szPath, pPSN->szFilename) == 0))
		{
			// The shader already exists
			if(pnShaderIdx)
				*pnShaderIdx = i;
			return K_OP_OK;
		}
	}

	// Add the new shader
	PSnode *pNewPS = new PSnode();
	if (pNewPS == NULL)
		return K_OP_FAILED;

	ZeroMemory(pNewPS, sizeof(PSnode));
	StringCchCopy(pNewPS->szFilename, MAX_PATH, szPath);
	//set friendly name
	pNewPS->shName.Init(szFriendlyName);
	
	HRESULT hr = S_OK;
	if (FAILED(hr = CreatePS(pDevice, szPath, &pNewPS->pShader)))
	{
		return K_OP_FAILED;
	}
	
	PixelShaders.Add(pNewPS);
	if (pnShaderIdx)
		*pnShaderIdx = PixelShaders.GetSize() - 1;
	return K_OP_OK;
}


PVERTEXSHADER CShaderManager::GetVShader(int nShaderIdx)
{
	if (nShaderIdx < 0 || nShaderIdx >= VertexShaders.Count())
		return nullptr;
	return VertexShaders[nShaderIdx]->pShader;
}

PVERTEXSHADER CShaderManager::GetVShaderByName(WCHAR * szName)
{
	if (szName == nullptr || wcscmp(szName, L"") == 0)
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
	return nullptr;
}

PVERTEXSHADER CShaderManager::GetVShaderByNameHash(UINT32 nNameHash)
{
	if (nNameHash == 0)
		return nullptr;

	for (int i = 0; i < VertexShaders.GetSize(); i++)
	{
		VSnode* pVS = VertexShaders[i];
		if (pVS->shName.textHash == nNameHash)
		{
			return pVS->pShader;
		}
	}

	return nullptr;
}

PPIXELSHADER CShaderManager::GetPShader(int nShaderIdx)
{
	if (nShaderIdx < 0 || nShaderIdx >= PixelShaders.Count())
		return nullptr;
	return PixelShaders[nShaderIdx]->pShader;
}

PPIXELSHADER CShaderManager::GetPShaderByName(WCHAR * szName)
{
	if (szName == nullptr || wcscmp(szName, L"") == 0)
	{
		return nullptr;
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
	return nullptr;
}

PPIXELSHADER CShaderManager::GetPShaderByNameHash(UINT32 nNameHash)
{
	if (nNameHash == 0)
		return nullptr;

	for (int i = 0; i < PixelShaders.GetSize(); i++)
	{
		PSnode* pPS = PixelShaders[i];
		if (pPS->shName.textHash == nNameHash)
		{
			return pPS->pShader;
		}
	}

	return nullptr;
}

void CShaderManager::ReloadAllShaders()
{
#if defined(_DEBUG) || defined(DEBUG)
	OnLostDevice(pDevice);
	OnResetDevice(pDevice, nullptr);
	LOG_DBG(L"-- ALL SHADERS RELOADED!");
#endif
}

//-=-=-= vertex declarations =-=-=-
OPRESULT CShaderManager::CreateVertexDeclarations()
{
	V_OP_HRFAILED(K_SEVERITY_CRITICAL, pDevice->CreateVertexDeclaration(_VERTEX_PNCT4T4_ve, &_VERTEX_PNCT4T4_decl));
	V_OP_HRFAILED(K_SEVERITY_CRITICAL, pDevice->CreateVertexDeclaration(_VERTEX_PNCT4_ve, &_VERTEX_PNCT4_decl));
	V_OP_HRFAILED(K_SEVERITY_CRITICAL, pDevice->CreateVertexDeclaration(_VERTEX_PCT4T4_ve, &_VERTEX_PCT4T4_decl));

	return K_OP_OK;
}
OPRESULT CShaderManager::ReleaseVertexDeclarations()
{
	SAFE_RELEASE(_VERTEX_PNCT4T4_decl);
	SAFE_RELEASE(_VERTEX_PNCT4_decl);
	SAFE_RELEASE(_VERTEX_PCT4T4_decl);

	return K_OP_OK;
}

OPRESULT CShaderManager::SetVS(PVERTEXSHADER pShader)
{
	if(FAILED(pDevice->SetVertexShader(pShader)))
		return OPRESULT(K_OP_FAILED, L"CShaderManager::Failed to set VS!", K_SEVERITY_WARNING);
	return K_OP_OK;
}

OPRESULT CShaderManager::SetVSByName(WCHAR* shaderName)
{
	if (shaderName == nullptr)
		return pDevice->SetVertexShader(nullptr);

	PVERTEXSHADER pVShader = GetVShaderByName(shaderName);
	if (pVShader)
		return pDevice->SetVertexShader(pVShader);

	return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CShaderManager::SetVSByName: VS not found: %s", shaderName);
}

OPRESULT CShaderManager::SetVSConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount)
{
	return pDevice->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

OPRESULT CShaderManager::SetVertexDeclaration(eVertexDeclarationType vtype)
{
	switch (vtype)
	{
		case K_SHM_PNCT4T4:
			return pDevice->SetVertexDeclaration(_VERTEX_PNCT4T4_decl);
			break;
		case K_SHM_PNCT:
			return pDevice->SetVertexDeclaration(_VERTEX_PNCT4_decl);
			break;
		case K_SHM_PT4T4:
			return pDevice->SetVertexDeclaration(_VERTEX_PNCT4T4_decl);
			break;
		default:
			return K_OP_FAILED;
			break;
	}
}

OPRESULT CShaderManager::SetPS(PPIXELSHADER pShader)
{
	return pDevice->SetPixelShader(pShader);
}

OPRESULT CShaderManager::SetPSByName(WCHAR* shaderName)
{
	if (shaderName == nullptr)
		return pDevice->SetPixelShader(nullptr);

	PPIXELSHADER pPShader = GetPShaderByName(shaderName);
	if (pPShader)
		return pDevice->SetPixelShader(pPShader);

	return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CShaderManager::SetPSByName: PS not found: %s", shaderName);
}

OPRESULT CShaderManager::SetPSConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount)
{
	return pDevice->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount);
}

//=-=-=- DEVICE FUNCTIONS -=-=-=
OPRESULT CShaderManager::OnCreateDevice( PDEVICE pDevice3d, const SURFACE_DESC* pBBDesc, void* pUserContext )
{
	pDevice = pDevice3d;
	return K_OP_OK;
}

OPRESULT CShaderManager::OnResetDevice( PDEVICE pDevice3d, const SURFACE_DESC* pBBDesc, void* pUserContext)
{
	pDevice = pDevice3d;
	OPRESULT opr = CreateVertexDeclarations();
	if (OP_FAILED(opr))
	{
		return OPRESULT(K_OP_FAILED, L"CShaderManager::OnResetDevice->Failed to createVertexDeclarations()", K_SEVERITY_CRITICAL);
	}

	HRESULT hr = S_OK;
	// reloads shaders
	for (int ii = 0; ii < VertexShaders.GetSize(); ii++)
	{
		if (FAILED(hr = CreateVS(pDevice, VertexShaders[ii]->szFilename, &VertexShaders[ii]->pShader)))
		{
			return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"Failed to re-create vertex shader (HR:%ld).\n\t\t%s\n", hr, VertexShaders[ii]->szFilename);
		}
	}
	for (int ii = 0; ii < PixelShaders.GetSize(); ii++)
	{
		if (FAILED(hr = CreatePS(pDevice, PixelShaders[ii]->szFilename, &PixelShaders[ii]->pShader)))
		{
			return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"Failed to re-create pixel shader (HR:%ld).\n\t\t%s\n", hr, PixelShaders[ii]->szFilename);
		}
	}

	return K_OP_OK;
}

OPRESULT CShaderManager::OnLostDevice( void* pUserContext )
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
	return K_OP_OK;
}

OPRESULT CShaderManager::OnDestroyDevice( void* pUserContext )
{
	return K_OP_OK;
}



///**************************************************************************************
/// Sigleton de acces
///**************************************************************************************

CShaderManager& UTGetShaderManager()
{
	static CShaderManager g_ShaderMgr;
	return g_ShaderMgr;
}

