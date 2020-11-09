#include "dxstdafx.h"

CRTManager::CRTManager()
{
	m_pDevice = null;

	if (arrRT.Count() > 0)
		ErrorBox(K_ERR_WARNING, L"RTManager: CTor: Render targets collection not empty!");
}

CRTManager::~CRTManager()
{
	m_pDevice = null;

	Release();
}

void CRTManager::AddRT(UINT32 dwID, UINT width, UINT height, UINT mipLevels, D3DFORMAT texFormat, bool bDepthStencil /*= TRUE*/, D3DFORMAT depthStencilFormat /*= D3DFMT_D24X8*/)
{
	HRESULT hr = S_OK;

	CEngineRenderTarget * pRT = new CEngineRenderTarget();
	pRT->UID = dwID;
	pRT->nWidth = width;
	pRT->nHeight = height;
	pRT->dwTexFormat = texFormat;
	pRT->bDepthStencilBuffer = bDepthStencil;
	pRT->dwDepthStencilFormat = depthStencilFormat;
	pRT->nMipLevels = mipLevels;
	// Add RT to list 
	arrRT.Add(pRT);

	LOG_DBG(L"CRTManager: AddRT: Added RT ID:%d [W:%d H:%d texFmt:%d bDepth:%d depthFmt:%d]", dwID, width, height, texFormat, bDepthStencil, depthStencilFormat);
	// try to create the RT right now
	if (m_pDevice != null)
	{
		if (!FAILED(CreateRT(pRT)))
		{
			UTGetAppClass().g_gfxFlags |= K_UT_GFXFLAG_RTT;
		}
		else
		{
			UTGetAppClass().g_gfxFlags &= ~K_UT_GFXFLAG_RTT;
		}
	}
}

HRESULT CRTManager::BeginSceneRT(UINT32 dwID)
{
	HRESULT hr = S_OK;
	CEngineRenderTarget* pTarget = GetRTbyUID(dwID);
	if ((pTarget != null) && (pTarget->bReady) && (pTarget->m_pRenderToSurface != null))
	{
		if (FAILED(pTarget->m_pRenderToSurface->BeginScene(pTarget->m_pRTSurface, NULL)))
		{
			ErrorBox(K_ERR_WARNING, L"[WARNING]CRTManager: BeginScene failed on RT:%d", dwID);
		}
	}
	else
		ErrorBox(K_ERR_WARNING, L"[WARNING]CRTManager: BeginSceneRT: Target ID not found: %d", dwID);

	return hr;
}

HRESULT CRTManager::BeginSceneRT(CEngineRenderTarget* pRT)
{
	HRESULT hr = S_OK;
	if ((pRT != null) && (pRT->bReady == true) && (pRT->m_pRenderToSurface != null))
	{
		if (FAILED(pRT->m_pRenderToSurface->BeginScene(pRT->m_pRTSurface, NULL)))
		{
			ErrorBox(K_ERR_WARNING, L"[WARNING]CRTManager: BeginScene(p) failed on RT:%d", pRT->UID);
		}
	}
	else
		ErrorBox(K_ERR_WARNING, L"[WARNING]CRTManager: BeginSceneRT(p) failed!");

	return hr;
}

HRESULT CRTManager::EndSceneRT(UINT32 dwID)
{
	HRESULT hr = S_OK;
	CEngineRenderTarget* pTarget = GetRTbyUID(dwID);
	//end scene paint/pass
	if ((pTarget != null) && (pTarget->m_pRenderToSurface != null))
	{
		hr = pTarget->m_pRenderToSurface->EndScene(0);
	}
	else
		ErrorBox(K_ERR_WARNING, L"[WARNING]CRTManager: EndSceneRT: Target ID not found: %d", dwID);

	return hr;
}

HRESULT CRTManager::EndSceneRT(CEngineRenderTarget* pRT)
{
	HRESULT hr = S_OK;
	//end scene paint/pass
	if ((pRT != null) && (pRT->m_pRenderToSurface != null))
	{
		hr = pRT->m_pRenderToSurface->EndScene(0);
	}
	else
		ErrorBox(K_ERR_WARNING, L"[WARNING]CRTManager: EndSceneRT: failed!");

	return hr;
}

void CRTManager::Release()
{
	for (int kk = 0; kk < arrRT.nCount; kk++)
	{
		// release dynamically allocated data
		SAFE_RELEASE(arrRT.m_pData[kk]->m_pRenderToSurface);
		SAFE_RELEASE(arrRT.m_pData[kk]->m_pRTTexture);
		SAFE_RELEASE(arrRT.m_pData[kk]->m_pRTSurface);
		// delete element
		SAFE_DELETE(arrRT.m_pData[kk]);
	}

	LOG_DBG(L"RTManager: Release: %d render targets released!", arrRT.nCount);

	arrRT.Clear();
}

CRTManager::CEngineRenderTarget* CRTManager::GetRTbyUID(UINT32 dwID)
{
	for (int kk = 0; kk < arrRT.nCount; kk++)
	{
		if (arrRT.m_pData[kk]->UID == dwID)
			return arrRT.m_pData[kk];
	}
	return null;
}

HRESULT CRTManager::CreateRT(CEngineRenderTarget* pRT)
{
	if (m_pDevice == null)
	{
		LOG_DBG(L"CRTManager: CreateRT: Device is null! pD:%d pRT:%d", m_pDevice, pRT);
		return E_NOT_VALID_STATE;
	}
	if ((pRT == null) || (pRT->bReady))
	{
		LOG_DBG(L"CRTManager: CreateRT: Engine RT is null or already ready! pD:%d pRT:%d", m_pDevice, pRT);
		return E_NOT_VALID_STATE;
	}

	HRESULT hr = S_OK;
	//--- create RT texture ---
	if (FAILED(D3DXCreateTexture(m_pDevice,
		pRT->nWidth,
		pRT->nHeight,
		pRT->nMipLevels,
		D3DUSAGE_RENDERTARGET,
		pRT->dwTexFormat,
		D3DPOOL_DEFAULT,
		&pRT->m_pRTTexture)))
	{
		ErrorBox(K_ERR_WARNING, L"CRTManager: CreateRT: Failed creating RT texture [ID:%d]. Setting CARD_FLAG_RTT to false.", pRT->UID);
		return E_FAIL;
	}
	else //if NOT failed
	{
		// Create off-screen "render to" surfaces...
		D3DSURFACE_DESC desc;
		pRT->m_pRTTexture->GetSurfaceLevel(0, &pRT->m_pRTSurface);
		pRT->m_pRTSurface->GetDesc(&desc);

		if (FAILED(D3DXCreateRenderToSurface(m_pDevice,
			desc.Width,
			desc.Height,
			desc.Format,
			pRT->bDepthStencilBuffer,
			pRT->dwDepthStencilFormat, 
			&pRT->m_pRenderToSurface)))
		{
			SAFE_RELEASE(pRT->m_pRTTexture);
			SAFE_RELEASE(pRT->m_pRTSurface);
			ErrorBox(K_ERR_WARNING, L"CRTManager: CreateRT: Failed creating RT surface [ID:%d]. Setting CARD_FLAG_RTT to false.", pRT->UID);
			return E_FAIL;
		}
	}

	pRT->bReady = true;
	LOG_DBG(L"CRTManager: CreateRT: Created RT [ID:%d] w:%d h:%d", pRT->UID, pRT->nWidth, pRT->nHeight);
	return S_OK;
}


///----------------------------------------------------
/// DEVICE FUNCTIONS
///----------------------------------------------------
HRESULT CRTManager::OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	m_pDevice = pd3dDevice;
	return S_OK;
}

HRESULT CRTManager::OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	m_pDevice = pd3dDevice;

	HRESULT hr = S_OK;
	for (int kk = 0; kk < arrRT.nCount; kk++)
	{
		// try to create the RT right now
		CEngineRenderTarget * pRT = arrRT.m_pData[kk];
		if (!FAILED(CreateRT(pRT)))
		{
			UTGetAppClass().g_gfxFlags |= K_UT_GFXFLAG_RTT;
		}
		else
		{
			//#TODO: when it can't create the RTs we should exit the game or try with a smaller pixel size so surfaces are smaller
			UTGetAppClass().g_gfxFlags &= ~K_UT_GFXFLAG_RTT;
		}
	}

	return S_OK;
}

HRESULT CRTManager::OnLostDevice(void* pUserContext)
{
	m_pDevice = null;

	// release device objects
	for (int kk = 0; kk < arrRT.nCount; kk++)
	{
		SAFE_RELEASE(arrRT.m_pData[kk]->m_pRenderToSurface);
		SAFE_RELEASE(arrRT.m_pData[kk]->m_pRTTexture);
		SAFE_RELEASE(arrRT.m_pData[kk]->m_pRTSurface);
		arrRT.m_pData[kk]->bReady = false;
	}

	return S_OK;
}

HRESULT CRTManager::OnDestroyDevice(void* pUserContext)
{
	m_pDevice = null;
	return S_OK;
}


///**************************************************************************************
/// Sigleton 
///**************************************************************************************
CRTManager& UTGetRTManager()
{
	static CRTManager g_RTmgr;
	return g_RTmgr;
}

