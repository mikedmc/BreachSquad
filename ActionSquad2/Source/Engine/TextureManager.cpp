#include "dxstdafx.h"

CTextureManager::CTextureManager(void)
{
	m_pd3dDevice = NULL;
}

CTextureManager::~CTextureManager(void)
{
	Release();
}


HRESULT CTextureManager::Release(void)
{
	HRESULT hr = S_OK;
	// Release textures and delete array of textures
	for (int i = 0; i < m_Texs.GetSize(); i++)
	{
		TexNode *pTN = m_Texs.GetAt(i);
		LOG(L"CTextureManager::Released %s", pTN->fileName);
		SAFE_RELEASE(pTN->pTexture);
		SAFE_DELETE(pTN);
	}
	m_Texs.RemoveAll();
	
	return hr;
}

LPDIRECT3DTEXTURE9 CTextureManager::GetTexture(int nTexIdx)
{
	if (nTexIdx < 0 || nTexIdx >= m_Texs.GetSize())
		return NULL;
	return m_Texs.GetAt(nTexIdx)->pTexture;
}


HRESULT	CTextureManager::AddTexture(const WCHAR* fileName, int *retTexIdx, D3DFORMAT format, DWORD filter, DWORD mipFilter, UINT nSetWidth, UINT nSetHeight)
{
	int texIdx = -1;
	if(retTexIdx != NULL)
		*retTexIdx = -1;

	if(wcslen(fileName) <= 0)
	{
		ErrorBox(K_ERR_WARNING, L"AddTexture:fileName is empty! No texture added.");
		return S_OK;
	}
	//--- check file exists ---
	if (FILE *file = OS_wfopen(fileName, L"r")) {
		OS_fclose(file);
	}
	else
	{
		ErrorBox(K_ERR_WARNING, L"AddTexture:file not found!\n%s", fileName);
		return E_FAIL;
	}

    for (int i = 0; i < m_Texs.GetSize(); i++)
	{
		TexNode* pTN = m_Texs.GetAt(i);

		//TODO: poate ar trebui sa reincarce textura daca difera filtrele?
		if(wcscmp(pTN->fileName, fileName) == 0)
		{
			// The texture already exists
			texIdx = i;
			if(retTexIdx != NULL)
				*retTexIdx = i;
			return S_OK;
		}
	}

	// Add the new texture
	TexNode *pNewTex = new TexNode();
	if (pNewTex == NULL)
		return E_OUTOFMEMORY;

	ZeroMemory(pNewTex, sizeof(TexNode));

	StringCchCopy(pNewTex->fileName, MAX_PATH, fileName);
	pNewTex->format = format;
	pNewTex->filter = filter;
	pNewTex->mipFilter = mipFilter;
	
	pNewTex->widthToLoad = nSetWidth;
	pNewTex->heightToLoad = nSetHeight;

	m_Texs.Add(pNewTex);
	
	texIdx = m_Texs.GetSize() - 1;
	if(retTexIdx != NULL)
		*retTexIdx = texIdx;

	// Try to create the new texture
	if (m_pd3dDevice != NULL)
		return LoadTexture(texIdx);
	return S_OK;
}


HRESULT CTextureManager::ReplaceTexture(int texIdx, const WCHAR* fileName, D3DFORMAT format, DWORD filter, DWORD mipFilter)
{
	if((texIdx < 0) || (texIdx >= m_Texs.GetSize()))
		return E_FAIL;

	if(wcslen(fileName) <= 0)
		return E_FAIL;

	TexNode* pTN = m_Texs.GetAt(texIdx);

	if(wcscmp(pTN->fileName, fileName) == 0)
	{
		// Texture is the same
		return S_OK;
	}

	// Reloads Texture
	LOG(L"CTextureManager::ReplaceTexture released %s", pTN->fileName);
	SAFE_RELEASE(pTN->pTexture);
	
	StringCchCopy(pTN->fileName, MAX_PATH, fileName);
	pTN->format = format;
	pTN->filter = filter;
	pTN->mipFilter = mipFilter;
	int nTexIdx = texIdx;

	// Try to create the new texture
	if (m_pd3dDevice != NULL)
		return LoadTexture(nTexIdx);

	return S_OK;
}



HRESULT CTextureManager::DeleteTexture(int nTexIdx)
{
	if (nTexIdx < 0 || nTexIdx >= m_Texs.GetSize())
		return E_INVALIDARG;
	HRESULT hr = S_OK;
	
	TexNode *pTN = m_Texs.GetAt(nTexIdx);
	LOG(L"CTextureManager::DeleteTexture released %s", pTN->fileName);
	SAFE_RELEASE(pTN->pTexture);
	pTN->fileName[0] = 0;
	//nu sterge nodul ca sa nu schimbe indecsii salvati
	return hr;
}

HRESULT CTextureManager::LoadTexture(const int nTexIdx)
{
	if (nTexIdx < 0 || nTexIdx >= m_Texs.GetSize())
		return E_INVALIDARG;

	HRESULT hr = S_OK;
	
	TexNode *pTN = m_Texs.GetAt(nTexIdx);
	
	// Make sure there's a texture to create
	if (wcslen(pTN->fileName) == 0)
		return S_OK;
	//daca e deja alocata, o dezaloca si o realoca
	if (pTN->pTexture != NULL)
	{
		LOG(L"CTextureManager::LoadTexture released %s", pTN->fileName);
		SAFE_RELEASE(pTN->pTexture);
	}

	// Create texture (managed)
	hr = D3DXCreateTextureFromFileEx(m_pd3dDevice, pTN->fileName, pTN->widthToLoad, pTN->heightToLoad, 
		1, 0, pTN->format, D3DPOOL_MANAGED, 
		pTN->filter, pTN->mipFilter, 0, 
		&pTN->info, NULL, &pTN->pTexture);

	if (FAILED(hr))
	{
		WCHAR wszMsg[512];
		StringCchPrintf(wszMsg, ARRAY_SIZE(wszMsg), L"[CTextureManager::LoadTexture] D3DXCreateTextureFromFileEx\n -Could not load texture %s\n", pTN->fileName);
		ErrorBox(K_ERR_WARNING, L"%s", wszMsg);
		return E_FAIL;
	}

	LOG(L"CTextureManager::Loaded %s", pTN->fileName);
	return S_OK;
}


TexNode* CTextureManager::GetTextureNode(int nTexIdx)
{
	if (nTexIdx < 0 || nTexIdx >= m_Texs.GetSize())
		return NULL;
	return m_Texs.GetAt(nTexIdx);
}

//-=-=-= SYSTEM / FRAMEWORK =-=-=-
HRESULT CTextureManager::OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr = S_OK;
	m_pd3dDevice = pd3dDevice;
	//le realoca daca s-a schimbat device-ul
	for(int kk=0; kk<m_Texs.GetSize(); kk++)
	{
		if(FAILED(hr = LoadTexture(kk)))
			return hr;
	}

	return hr;
}

HRESULT CTextureManager::OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	m_pd3dDevice = pd3dDevice;
	return S_OK;
}

HRESULT CTextureManager::OnLostDevice(void)
{
	HRESULT hr = S_OK;

	m_pd3dDevice = NULL;

	return hr;
}

HRESULT CTextureManager::OnDestroyDevice(void)
{
	HRESULT hr = S_OK;

	m_pd3dDevice = NULL;
	//dezaloca texturile cand se schimba device-ul
	for(int kk=0; kk<m_Texs.GetSize(); kk++)
	{
		LOG(L"CTextureManager::OnDestroyDevice released %s", m_Texs[kk]->fileName);
		SAFE_RELEASE(m_Texs[kk]->pTexture);
	}
	
	return hr;
}
