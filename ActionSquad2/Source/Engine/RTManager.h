#pragma once

///----------------------------------------------------
/// RENDER TARGET MANAGER
/// - class for creating render targets
///----------------------------------------------------

// max number of render targets
#define K_RTT_MAX_RT_CNT	10

class CRTManager {
public:
	struct CEngineRenderTarget {
		// DirectX specific stuff
		LPD3DXRENDERTOSURFACE   m_pRenderToSurface;
		LPDIRECT3DTEXTURE9      m_pRTTexture;
		LPDIRECT3DSURFACE9      m_pRTSurface;

		bool					bReady;					// Ready to be used
		UINT32					UID;					// ID of the resource

		bool					bDepthStencilBuffer;	// Do we need depth/stencil
		UINT					nWidth;
		UINT					nHeight;
		UINT					nMipLevels;
		D3DFORMAT				dwTexFormat;
		D3DFORMAT				dwDepthStencilFormat;

		CEngineRenderTarget() :
			bReady(false), nWidth(0), nHeight(0),
			dwTexFormat(D3DFMT_A8B8G8R8), UID(0), bDepthStencilBuffer(true),
			dwDepthStencilFormat(D3DFMT_D24X8), nMipLevels(1)
		{
		}
	};

private:
	LPDIRECT3DDEVICE9	m_pDevice;
public:
	CFixedArray<CEngineRenderTarget*, 10> arrRT;
	
	//CTOR-DTOR
	CRTManager();
	~CRTManager();

	// Adds a new render target	to the RT collection
	void					AddRT(UINT32 dwID, UINT width, UINT height, UINT mipLevels, D3DFORMAT texFormat, bool bDepthStencil = TRUE, D3DFORMAT depthStencilFormat = D3DFMT_D24X8);
	// Called before drawing so the engine knows to draw to the specified RT
	HRESULT					BeginSceneRT(UINT32 dwID);
	HRESULT					BeginSceneRT(CEngineRenderTarget* pRT);
	// Called when drawing finished so we flush everything and announce that won't paint to the RT anylonger
	HRESULT					EndSceneRT(UINT32 dwID);
	// Releases all allocated render targets and deletes them from the RT collection
	void					Release();
	// Returns pointer to RT or null if ID not found
	CEngineRenderTarget*	GetRTbyUID(UINT32 dwID);

private:
	// Creates a new render target texture and all associated surfaces and structures
	HRESULT					CreateRT(CEngineRenderTarget* pRT);

public: //--- framework methods ---
	HRESULT OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL, void* pUserContext = NULL);
	HRESULT OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL, void* pUserContext = NULL);
	HRESULT OnLostDevice(void* pUserContext = NULL);
	HRESULT OnDestroyDevice(void* pUserContext = NULL);
};

///--- SINGLETON ---
CRTManager& UTGetRTManager();
