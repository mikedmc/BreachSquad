#include "dxstdafx.h"

CSpineTexLoader::CSpineTexLoader(void)
{
	m_pDevice = NULL;
	memset(m_wcsFilesPrefix, 0, sizeof(WCHAR) * MAX_PATH);
}

CSpineTexLoader::~CSpineTexLoader(void)
{
	Release();
}


void CSpineTexLoader::load(AtlasPage& page, const String& path)
{
	WCHAR wcsFilename[MAX_PATH];
	mbstowcs(wcsFilename, path.buffer(), MAX_PATH);

	CSpineTex* texture = AddTexture(wcsFilename, D3DFMT_A8R8G8B8, D3DX_FILTER_NONE, D3DX_FILTER_NONE);

	// if texture loading failed, we simply return.
	if (!texture) return;

	// store the Texture on the rendererObject so we can
	// retrieve it later for rendering.
	page.setRendererObject(texture);

	// store the texture width and height on the spAtlasPage
	// so spine-c can calculate texture coordinates for
	// rendering.
	page.width = texture->info.Width;
	page.height = texture->info.Height;
}

void CSpineTexLoader::unload(void* texture)
{
	CSpineTex* pTex = static_cast<CSpineTex*>(texture);
	DisposeTexture(pTex);
}

HRESULT CSpineTexLoader::Release(void)
{
	HRESULT hr = S_OK;
	// Release textures and delete array of textures
	for (int i = 0; i < m_Texs.GetSize(); i++)
	{
		CSpineTex *pTN = m_Texs.GetAt(i);
		LOG(L"CSpineTexLoader::Released %s", pTN->fileName);
		SAFE_RELEASE(pTN->pTexture);
		SAFE_RELEASE(pTN->pTexture_N);
		SAFE_RELEASE(pTN->pTexture_S);
		SAFE_DELETE(pTN);
	}
	m_Texs.RemoveAll();
	
	return hr;
}

HRESULT CSpineTexLoader::DisposeTexture(CSpineTex* tex)
{
	if (tex == null)
		return S_OK;

	int nIdx = m_Texs.IndexOf(tex);

	if (nIdx < 0)
	{
		LOG(L"CSpineTexLoader::Couldn't find [%s]", tex->fileName);
	}
	else
	{
		LOG(L"CSpineTexLoader::Disposed [%s]", tex->fileName);
	}

	SAFE_RELEASE(tex->pTexture);
	SAFE_RELEASE(tex->pTexture_N);
	SAFE_RELEASE(tex->pTexture_S);
	SAFE_DELETE(tex);

	m_Texs.Remove(nIdx);

	return S_OK;
}


CSpineTex* CSpineTexLoader::AddTexture(const WCHAR* fileName, D3DFORMAT format, DWORD filter, DWORD mipFilter, UINT nSetWidth, UINT nSetHeight)
{
	WCHAR wcsFinalPath[MAX_PATH];
	StringCchPrintf(wcsFinalPath, MAX_PATH, L"%s%s", m_wcsFilesPrefix, fileName);

	HRESULT hr = S_OK;
	if(wcslen(fileName) <= 0)
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING] CSpineTexLoader::AddTexture:fileName is empty! No texture added.");
		return null;
	}
	//--- check file exists ---
	if (FILE *file = OS_wfopen(wcsFinalPath, L"r")) 
	{
		OS_fclose(file);
	}
	else
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING] CSpineTexLoader::AddTexture:file not found!\n%s", fileName);
		return null;
	}

	// Add the new texture
	CSpineTex * pNewTex = new CSpineTex();

	ZeroMemory(pNewTex, sizeof(TexNode));
	//adds the default prefix before loading
	StringCchPrintf(pNewTex->fileName, MAX_PATH, L"%s", wcsFinalPath);

	pNewTex->format = format;
	pNewTex->filter = filter;
	pNewTex->mipFilter = mipFilter;
	
	pNewTex->nSetWidth = nSetWidth;
	pNewTex->nSetHeight = nSetHeight;

	m_Texs.Add(pNewTex);
	
	int texIdx = m_Texs.GetSize() - 1;

	// Try to create the new texture
	if (m_pDevice != NULL)
	{
		if (FAILED(hr = LoadTextureNow(texIdx)))
		{
			SAFE_DELETE(pNewTex);
			m_Texs.Remove(texIdx);

			ErrorBox(K_ERR_WARNING, L"[WARNING] CSpineTexLoader::AddTexture: Couldn't load texture! \n%s", fileName);

			return null;
		}
	}

	return pNewTex;
}


void CSpineTexLoader::SetFilesPrefix(const WCHAR * wcsPrefix)
{
	StringCchCopy(m_wcsFilesPrefix, MAX_PATH, wcsPrefix);
}

HRESULT CSpineTexLoader::LoadTextureNow(const int nTexIdx)
{
	if (nTexIdx < 0 || nTexIdx >= m_Texs.GetSize())
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	CSpineTex* pTN = m_Texs[nTexIdx];

	// Make sure there's a texture to create
	if (wcslen(pTN->fileName) == 0)
		return S_OK;
	// Already allocated
	SAFE_RELEASE(pTN->pTexture);
	SAFE_RELEASE(pTN->pTexture_N);
	SAFE_RELEASE(pTN->pTexture_S);

	// Create texture (managed)
	hr = D3DXCreateTextureFromFileEx(m_pDevice, pTN->fileName, pTN->nSetWidth, pTN->nSetHeight,
		1, 0, pTN->format, D3DPOOL_MANAGED,
		pTN->filter, pTN->mipFilter, 0,
		&pTN->info, NULL, &pTN->pTexture);

	if (FAILED(hr))
	{
		WCHAR wszMsg[512];
		StringCchPrintf(wszMsg, ARRAY_SIZE(wszMsg), L"[CSpineTexLoader::LoadTexture] D3DXCreateTextureFromFileEx\n -Could not load texture %s\n", pTN->fileName);
		ErrorBox(K_ERR_WARNING, L"%s", wszMsg);
		return hr;
	}

	LOG(L"CSpineTexLoader::Loaded [%s]", pTN->fileName);

#ifdef K_TL_LOAD_NORMAL_MAPS_SUFFIX
	{
		WCHAR strPathNoExt[MAX_PATH] = { 0 };
		WCHAR strExt[MAX_PATH] = { 0 };
		WCHAR strFinalPath[MAX_PATH] = { 0 };

		if ((OS_GetFileNameWithoutExtension(strPathNoExt, MAX_PATH, pTN->fileName) > 0) &&
			(OS_GetFileNameExtension(strExt, MAX_PATH, pTN->fileName)))
		{
			StringCchPrintf(strFinalPath, MAX_PATH, L"%s%s.%s", strPathNoExt, K_TL_LOAD_NORMAL_MAPS_SUFFIX, strExt);

			// Create texture (managed)
			hr = D3DXCreateTextureFromFileEx(m_pDevice, strFinalPath, pTN->nSetWidth, pTN->nSetHeight,
				1, 0, pTN->format, D3DPOOL_MANAGED,
				pTN->filter, pTN->mipFilter, 0,
				&pTN->info, NULL, &pTN->pTexture_N);

			if (FAILED(hr))
			{
				WCHAR wszMsg[512];
				StringCchPrintf(wszMsg, ARRAY_SIZE(wszMsg), L"[CSpineTexLoader::LoadTexture] D3DXCreateTextureFromFileEx\n -Could not load NORMAL MAP texture %s\n", strFinalPath);
				ErrorBox(K_ERR_WARNING, L"%s", wszMsg);
				return hr;
			}
		}
		else
		{
			ErrorBox(K_ERR_WARNING, L"[CSpineTexLoader::LoadTexture] Could not parse NORMAL MAP path %s\n", pTN->fileName);
			return E_INVALIDARG;
		}

		LOG(L"CSpineTexLoader::Loaded normal map [%s]", strFinalPath);
	}
#endif // K_TL_LOAD_NORMAL_MAPS_SUFFIX


#ifdef K_TL_LOAD_SPECULAR_MAPS_SUFFIX
	{
		WCHAR strPathNoExt[MAX_PATH] = { 0 };
		WCHAR strExt[MAX_PATH] = { 0 };
		WCHAR strFinalPath[MAX_PATH] = { 0 };

		if ((OS_GetFileNameWithoutExtension(strPathNoExt, MAX_PATH, pTN->fileName) > 0) &&
			(OS_GetFileNameExtension(strExt, MAX_PATH, pTN->fileName)))
		{
			StringCchPrintf(strFinalPath, MAX_PATH, L"%s%s.%s", strPathNoExt, K_TL_LOAD_SPECULAR_MAPS_SUFFIX, strExt);

			// Create texture (managed)
			hr = D3DXCreateTextureFromFileEx(m_pDevice, strFinalPath, pTN->nSetWidth, pTN->nSetHeight,
				1, 0, pTN->format, D3DPOOL_MANAGED,
				pTN->filter, pTN->mipFilter, 0,
				&pTN->info, NULL, &pTN->pTexture_S);

			if (FAILED(hr))
			{
				WCHAR wszMsg[512];
				StringCchPrintf(wszMsg, ARRAY_SIZE(wszMsg), L"[CSpineTexLoader::LoadTexture] D3DXCreateTextureFromFileEx\n -Could not load SPECULAR MAP texture %s\n", strFinalPath);
				ErrorBox(K_ERR_WARNING, L"%s", wszMsg);
				return hr;
			}
		}
		else
		{
			ErrorBox(K_ERR_WARNING, L"[CSpineTexLoader::LoadTexture] Could not parse SPECULAR MAP path %s\n", pTN->fileName);
			return E_INVALIDARG;
		}

		LOG(L"CSpineTexLoader::Loaded specular map [%s]", strFinalPath);
	}
#endif // K_TL_LOAD_SPECULAR_MAPS_SUFFIX


	return S_OK;
}


//-=-=-= SYSTEM / FRAMEWORK =-=-=-
HRESULT CSpineTexLoader::OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr = S_OK;
	m_pDevice = pd3dDevice;
	// Reallocate when changing the device
	for(int kk=0; kk<m_Texs.GetSize(); kk++)
	{
		if (FAILED(hr = LoadTextureNow(kk)))
		{
			ErrorBox(K_ERR_WARNING, L"CSpineTexLoader::OnCreateDevice load failed on: %s", m_Texs[kk]->fileName);
		}
	}

	return hr;
}

HRESULT CSpineTexLoader::OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	m_pDevice = pd3dDevice;
	return S_OK;
}

HRESULT CSpineTexLoader::OnLostDevice(void)
{
	HRESULT hr = S_OK;

	m_pDevice = NULL;

	return hr;
}

HRESULT CSpineTexLoader::OnDestroyDevice(void)
{
	HRESULT hr = S_OK;

	m_pDevice = NULL;
	// Deallocate only texture objects
	for (int kk = 0; kk < m_Texs.GetSize(); kk++)
	{
		LOG(L"CSpineTexLoader::OnDestroyDevice released %s", m_Texs[kk]->fileName);
		SAFE_RELEASE(m_Texs[kk]->pTexture);
		SAFE_RELEASE(m_Texs[kk]->pTexture_N);
		SAFE_RELEASE(m_Texs[kk]->pTexture_S);
	}
	
	return hr;
}
