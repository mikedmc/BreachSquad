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
		
		PRENDERTOSURFACE		m_pRenderToSurface;
		PTEXTURE				m_pRTTexture;
		PSURFACE				m_pRTSurface;

		bool					bReady;					// Ready to be used
		UINT32					UID;					// ID of the resource

		bool					bDepthStencilBuffer;	// Do we need depth/stencil
		UINT					nWidth;
		UINT					nHeight;
		UINT					nMipLevels;
		FORMAT3D				dwTexFormat;
		FORMAT3D				dwDepthStencilFormat;

		Mat						matProj;				// Projection matrix specific to this RT

		CEngineRenderTarget() :
			bReady(false), nWidth(0), nHeight(0),
			dwTexFormat(D3DFMT_A8B8G8R8), UID(0), bDepthStencilBuffer(true),
			dwDepthStencilFormat(D3DFMT_D24X8), nMipLevels(1)
		{
			MUMatIdentity(&matProj);
		}
	};

private:
	PDEVICE						m_pDevice;
public:
	CFixedArray<CEngineRenderTarget*, 10> arrRT;
	
	//CTOR-DTOR
	CRTManager();
	~CRTManager();

	// Adds a new render target	to the RT collection
	void					AddRT(UINT32 dwID, UINT width, UINT height, UINT mipLevels, FORMAT3D texFormat, bool bDepthStencil = TRUE, FORMAT3D depthStencilFormat = D3DFMT_D24X8);
	// Called before drawing so the engine knows to draw to the specified RT
	OPRESULT				BeginSceneRT(UINT32 dwID);
	OPRESULT				BeginSceneRT(CEngineRenderTarget* pRT);
	// Called when drawing finished so we flush everything and announce that won't paint to the RT anylonger
	OPRESULT				EndSceneRT(UINT32 dwID);
	OPRESULT				EndSceneRT(CEngineRenderTarget* pRT);
	// Releases all allocated render targets and deletes them from the RT collection
	void					Release();
	// Returns pointer to RT or null if ID not found
	CEngineRenderTarget*	GetRTbyUID(UINT32 dwID);

private:
	// Creates a new render target texture and all associated surfaces and structures
	OPRESULT				CreateRT(CEngineRenderTarget* pRT);

public: //--- framework methods ---
	OPRESULT OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL);
	OPRESULT OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL);
	OPRESULT OnLostDevice();
	OPRESULT OnDestroyDevice();
};

///--- SINGLETON ---
CRTManager& __RTManager();
