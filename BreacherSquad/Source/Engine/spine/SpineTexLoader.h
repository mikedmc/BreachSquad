#pragma once
#include "spine/TextureLoader.h"

// Comment this line if you don't need normal maps. The string gets added to the filename, before the extension
#define K_TL_LOAD_NORMAL_MAPS_SUFFIX L"_n"
// Comment this line if you don't need specular data maps. The string gets added to the filename, before the extension
//#define K_TL_LOAD_SPECULAR_MAPS_SUFFIX L"_s"

class CSpineTex
{
public:
	int					nSetWidth, nSetHeight;
	WCHAR				fileName[MAX_PATH];
	PTEXTURE			pTexture;				// Texture
	PTEXTURE			pTexture_N;				// Normal map
	PTEXTURE			pTexture_S;				// Specular map
	FORMAT3D			format;
	IMAGE_INFO			info;
	DWORD				filter, mipFilter;
	//CTOR
	CSpineTex() :
		filter(D3DX_DEFAULT), mipFilter(D3DX_DEFAULT), 
		pTexture(null), pTexture_N(null), pTexture_S(null), format(D3DFMT_UNKNOWN),
		nSetWidth(D3DX_DEFAULT), nSetHeight(D3DX_DEFAULT)
	{
		fileName[0] = 0;
	}
};

class CSpineTexLoader : public TextureLoader
{
protected:
	PDEVICE						m_pDevice;
	WCHAR						m_wcsFilesPrefix[MAX_PATH];			//prefix that gets put before file paths

public:
	CArray<CSpineTex*>			m_Texs;

	CSpineTexLoader();
	~CSpineTexLoader();


	// Called when the atlas loads the texture of a page.
	virtual void			load(AtlasPage& page, const String& path);
	// Called when the atlas is disposed and itself disposes its atlas pages.
	virtual void			unload(void* texture);

	//Releases everything
	HRESULT					Release(void);
	HRESULT					DisposeTexture(CSpineTex* tex);
	CSpineTex*				AddTexture(const WCHAR* fileName, D3DFORMAT format, DWORD filter, DWORD mipFilter, UINT nSetWidth = D3DX_DEFAULT, UINT nSetHeight = D3DX_DEFAULT);
	// Sets a prefix to be used for all following texture loads. Needed because Spine dowsn't support WCHAR.
	void					SetFilesPrefix(const WCHAR * wcsPrefix);

private:
	// Loads the texture object from the texture node
	HRESULT					LoadTextureNow(const int nTexIdx);

public:
	//--- system framework ---
	HRESULT OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnLostDevice(void);
	HRESULT OnDestroyDevice(void);
};
