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

void CRTManager::AddRT(UINT32 dwID, UINT width, UINT height, UINT mipLevels, FORMAT3D texFormat, bool bDepthStencil /*= TRUE*/, FORMAT3D depthStencilFormat /*= D3DFMT_D24X8*/)
{
	CEngineRenderTarget * pRT = new CEngineRenderTarget();

	pRT->UID = dwID;
	pRT->nWidth = width;
	pRT->nHeight = height;
	pRT->dwTexFormat = texFormat;
	pRT->bDepthStencilBuffer = bDepthStencil;
	pRT->dwDepthStencilFormat = depthStencilFormat;
	pRT->nMipLevels = mipLevels;
	// create projection matrix specific for this RT
	// uses 0.5 because in DirectX9 UV of 0.0 means center of texel. Change this to 0.0f on OpenGL if blurry.
	MUMatOrthoOffCenterLH(&pRT->matProj, 0.5f, pRT->nWidth + 0.5f, pRT->nHeight + 0.5f, 0.5f, 0.0f, 1.0f);
	// Add RT to list 
	arrRT.Add(pRT);

	LOG_DBG(L"CRTManager: AddRT: Added RT ID:%d [W:%d H:%d texFmt:%d bDepth:%d depthFmt:%d]", dwID, width, height, texFormat, bDepthStencil, depthStencilFormat);
	// try to create the RT right now
	if (m_pDevice != null)
	{
		if (OP_SUCCESS(CreateRT(pRT)))
		{
			UTApp().g_gfxFlags |= K_UT_GFXFLAG_RTT;
		}
		else
		{
			UTApp().g_gfxFlags &= ~K_UT_GFXFLAG_RTT;
		}
	}
}

OPRESULT CRTManager::BeginSceneRT(UINT32 dwID)
{
	HRESULT hr = S_OK;
	CEngineRenderTarget* pTarget = GetRTbyUID(dwID);
	if ((pTarget != null) && (pTarget->bReady) && (pTarget->m_pRenderToSurface != null))
	{
		if (FAILED(pTarget->m_pRenderToSurface->BeginScene(pTarget->m_pRTSurface, NULL)))
		{
			return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"[WARNING]CRTManager: BeginScene failed on RT:%d", dwID);
		}
	}
	else
		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"[WARNING]CRTManager: BeginSceneRT: Target ID not found: %d", dwID);

	return K_OP_OK;
}

OPRESULT CRTManager::BeginSceneRT(CEngineRenderTarget* pRT)
{
	HRESULT hr = S_OK;
	if ((pRT != null) && (pRT->bReady == true) && (pRT->m_pRenderToSurface != null))
	{
		if (FAILED(pRT->m_pRenderToSurface->BeginScene(pRT->m_pRTSurface, NULL)))
		{
			return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CRTManager: BeginScene(p) failed on RT:%d", pRT->UID);
		}
	}
	else
		return OPRESULT(K_OP_FAILED, L"CRTManager: BeginSceneRT(p) failed!", K_SEVERITY_WARNING);

	return K_OP_OK;
}

OPRESULT CRTManager::EndSceneRT(UINT32 dwID)
{
	CEngineRenderTarget* pTarget = GetRTbyUID(dwID);
	//end scene paint/pass
	if ((pTarget != null) && (pTarget->m_pRenderToSurface != null))
	{
		pTarget->m_pRenderToSurface->EndScene(0);
	}
	else
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"[WARNING]CRTManager: EndSceneRT: Target ID not found: %d", dwID);
	}

	return K_OP_OK;
}

OPRESULT CRTManager::EndSceneRT(CEngineRenderTarget* pRT)
{
	//end scene paint/pass
	if ((pRT != null) && (pRT->m_pRenderToSurface != null))
	{
		pRT->m_pRenderToSurface->EndScene(0);
	}
	else
	{
		return OPRESULT(K_OP_FAILED, L"[WARNING]CRTManager: EndSceneRT: failed!", K_SEVERITY_WARNING);
	}

	return K_OP_OK;
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

	ErrorBox(K_ERR_WARNING, L"RTManager: RT not found ID:%d", dwID);
	return nullptr;
}

OPRESULT CRTManager::CreateRT(CEngineRenderTarget* pRT)
{
	if (m_pDevice == null)
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CRTManager: CreateRT: Device is null! pD:%d pRT:%d", m_pDevice, pRT);
	}
	if ((pRT == null) || (pRT->bReady))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CRTManager: CreateRT: Engine RT is null or already ready! pD:%d pRT:%d", m_pDevice, pRT);
	}

	//--- create RT texture ---
	HRESULT hr = S_OK;
	if (FAILED(D3DXCreateTexture(m_pDevice,
		pRT->nWidth,
		pRT->nHeight,
		pRT->nMipLevels,
		D3DUSAGE_RENDERTARGET,
		pRT->dwTexFormat,
		D3DPOOL_DEFAULT,
		&pRT->m_pRTTexture)))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CRTManager: CreateRT: Failed creating RT texture [ID:%d]. Setting CARD_FLAG_RTT to false.", pRT->UID);
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

			return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CRTManager: CreateRT: Failed creating RT surface [ID:%d]. Setting CARD_FLAG_RTT to false.", pRT->UID);
		}
	}

	pRT->bReady = true;
	LOG_DBG(L"CRTManager: CreateRT: Created RT [ID:%d] w:%d h:%d", pRT->UID, pRT->nWidth, pRT->nHeight);
	return K_OP_OK;
}


///----------------------------------------------------
/// DEVICE FUNCTIONS
///----------------------------------------------------
OPRESULT CRTManager::OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc)
{
	m_pDevice = pDevice;
	return K_OP_OK;
}

OPRESULT CRTManager::OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc)
{
	m_pDevice = pDevice;

	for (int kk = 0; kk < arrRT.nCount; kk++)
	{
		// try to create the RT right now
		CEngineRenderTarget * pRT = arrRT.m_pData[kk];
		if (OP_SUCCESS(CreateRT(pRT)))
		{
			UTApp().g_gfxFlags |= K_UT_GFXFLAG_RTT;
		}
		else
		{
			//#TODO: when it can't create the RTs we should exit the game or try with a smaller pixel size so surfaces are smaller
			UTApp().g_gfxFlags &= ~K_UT_GFXFLAG_RTT;
		}
	}

	return K_OP_OK;
}

OPRESULT CRTManager::OnLostDevice()
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

	return K_OP_OK;
}

OPRESULT CRTManager::OnDestroyDevice()
{
	m_pDevice = null;
	return K_OP_OK;
}


///**************************************************************************************
/// Sigleton 
///**************************************************************************************
CRTManager& UTGetRTManager()
{
	static CRTManager g_RTmgr;
	return g_RTmgr;
}

