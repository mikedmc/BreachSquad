#pragma once

class CTTFont
{
public:
	//static pointer to target sprite
	static ID3DXSprite* m_pSprite;
public:
	CStringHash shFontName;
	ID3DXFont*	pFont;
	int			nFontSize;

	CTTFont();
	~CTTFont();
	// Draws a simple line of text
	HRESULT DrawTextLine(const WCHAR* strMsg, int posX, int posY, UINT32 Flags = DT_LEFT | DT_BOTTOM, DWORD wCol = 0xffffffff, int nOriginalFontH = -1);
};

//***********************************************************
// TrueType Font Manager
//***********************************************************
class CTTFontsManager 
{
public:
	//static pointer to target sprite
	static ID3DXSprite* m_pSprite;
	//sets static pointer to taget sprite
	static void SetGlobalSpritePtr(ID3DXSprite* pSprite);
private:
	LPDIRECT3DDEVICE9 m_pDevice;
	
	CArray<CTTFont*> m_arrFonts;
public:
	CTTFontsManager();
	~CTTFontsManager();

	//loads a font and stores it locally with the ID:strFontNameID
	HRESULT LoadFont(const WCHAR* strFontNameID, WCHAR* strFontFace, WCHAR* strFontPath, int nFontSize);
	void Release();

	CTTFont* GetFont(const WCHAR* strFontName);
	CTTFont* GetFont(UINT32 unFontNameHash);

	///--- system framework ---
	HRESULT OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnLostDevice(void);
	HRESULT OnDestroyDevice(void);
};

//declar singletonul
CTTFontsManager& UTGetTTFManager();

