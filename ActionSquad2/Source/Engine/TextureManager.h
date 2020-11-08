#pragma once

class TexNode
{
public:
	UINT				widthToLoad;	//forces texture loading to this width
	UINT				heightToLoad;

	WCHAR				fileName[MAX_PATH];
	LPDIRECT3DTEXTURE9	pTexture;
	D3DFORMAT			format;
	D3DXIMAGE_INFO		info;
	DWORD				filter, mipFilter;
	//CTOR
	TexNode() : widthToLoad(D3DX_DEFAULT), heightToLoad(D3DX_DEFAULT),
	filter(D3DX_DEFAULT),
	mipFilter(D3DX_DEFAULT)
	{}
};

//TODO: Poate ar fi bine sa fie adresabile si in functie de un hash atunci cand ai de gand sa stergi din ele
class CTextureManager
{
protected:
	LPDIRECT3DDEVICE9		m_pd3dDevice;
	HRESULT	LoadTexture(const int nTexIdx);

public:
	CTextureManager(void);
	~CTextureManager(void);

	/*!
	 *	Releases all allocated textures
	 */
	HRESULT Release(void);
	HRESULT DeleteTexture(int nTexIdx);
	HRESULT	AddTexture(const WCHAR* fileName, int *retTexIdx, D3DFORMAT format, DWORD filter, DWORD mipFilter, UINT nSetWidth = D3DX_DEFAULT, UINT nSetHeight = D3DX_DEFAULT);
	//Replaces a loaded texture with another one. Checks to see if it is the same.
	HRESULT ReplaceTexture(int texIdx, const WCHAR* fileName, D3DFORMAT format, DWORD filter = D3DX_DEFAULT, DWORD mipFilter = D3DX_DEFAULT);
	LPDIRECT3DTEXTURE9	GetTexture(int nTexIdx);
	Vec2 GetTextureSize(int nTexIdx);

	CGrowableArray<TexNode*>	m_Texs;
	TexNode* GetTextureNode(int nTexIdx);

	int		GetTextureCount(void)	{ return m_Texs.GetSize();	}

	//--- system framework ---
	HRESULT OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnLostDevice(void);
	HRESULT OnDestroyDevice(void);
};
