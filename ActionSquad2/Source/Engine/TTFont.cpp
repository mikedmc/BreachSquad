#include "dxstdafx.h"
#include ".\TTFont.h"

///------ statics ------
ID3DXSprite* CTTFontsManager::m_pSprite = NULL;
ID3DXSprite* CTTFont::m_pSprite = NULL;

void CTTFontsManager::SetGlobalSpritePtr(ID3DXSprite* pSprite)
{
	m_pSprite = pSprite;
	//set fonts sprite pointer too
	CTTFont::m_pSprite = pSprite;
}

///**************************************************************************************
/// TTF
///**************************************************************************************
CTTFont::CTTFont()
{
	pFont = null;
	shFontName.Reset();
	nFontSize = 0;
}

CTTFont::~CTTFont()
{
	SAFE_RELEASE(pFont);
	shFontName.Reset();
}

HRESULT CTTFont::DrawTextLine(const WCHAR* strMsg, int posX, int posY, UINT32 Flags, DWORD wCol, int nOriginalFontH)
{
	if (pFont == null)
	{
		LOG(L"[WARNING] TTF::DrawTextLine:Error: Font not ready! [%s]", shFontName.text);
		return E_FAIL;
	}
	if (m_pSprite == null)
	{
		LOG(L"[WARNING] TTF::DrawTextLine:Error: Sprite pointer is null!");
		return E_FAIL;
	}

	//set rectangle
	int scrW = CCameraTransform::GetActiveCamera()->GetViewport().w;

	HRESULT hr;
	RECT rc;
	SetRect(&rc, posX, posY, posX + scrW, posY + nFontSize);
	//horizontal
	if (Flags & DT_RIGHT)
	{
		rc.left = posX - scrW;
		rc.right = posX;
	}
	else if (Flags & DT_CENTER)
	{
		rc.left = posX - scrW / 2;
		rc.right = posX + scrW / 2;
	}
	//vertical
	int nOffY = 0;
	//top aligned by default:
	if (nOriginalFontH > 0)
		nOffY = (nOriginalFontH - nFontSize) / 2;
	if (Flags & DT_BOTTOM)
	{
		if (nOriginalFontH > 0)
			nOffY = -(nOriginalFontH - nFontSize) / 2;

		rc.top = posY - nFontSize;
		rc.bottom = posY;
	}
	else if (Flags & DT_VCENTER)
	{
		nOffY = 0;
		rc.top = posY - nFontSize / 2;
		rc.bottom = posY + nFontSize / 2;
	}
	//always draw the text vertically centered so it better aligns to bitmap font area
	rc.top += nOffY; rc.bottom += nOffY;
	Flags &= ~DT_BOTTOM; //remove valign flags
	Flags |= DT_VCENTER; //force vertical center


	hr = pFont->DrawText(m_pSprite, strMsg, -1, &rc, Flags, wCol);
	if (FAILED(hr))
	{
		LOG(L"[WARNING] TTF::DrawTextLine:Error: DrawText failed with code: %u", hr);
		return hr;
	}

	return S_OK;
}


///**************************************************************************************
/// TTF MANAGER
///**************************************************************************************
CTTFontsManager::CTTFontsManager()
{
	m_pSprite = null;
	m_pDevice = null;
	m_arrFonts.RemoveAll();
}

CTTFontsManager::~CTTFontsManager()
{
	Release();
}

HRESULT CTTFontsManager::LoadFont(const WCHAR* strFontNameID, WCHAR* strFontFace, WCHAR* strFontPath, int nFontSize)
{
	HRESULT hr = S_OK;

	if (m_pDevice == null)
	{
		LOG(L"TTF::Error: LoadFont failed! Device is null!");
		return E_FAIL;
	}
	
	CTTFont* pFont = new CTTFont();
	pFont->shFontName.Init(strFontNameID);
	pFont->nFontSize = nFontSize;

	//tell windows we have a custom font here
	AddFontResourceEx(strFontPath, FR_PRIVATE, 0);
	// Initialize the font
	if (FAILED(D3DXCreateFont(m_pDevice, nFontSize, 0, FW_BOLD, 1, FALSE, DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		strFontFace, &pFont->pFont)))
	{
		SAFE_DELETE(pFont);
		LOG(L"TTF::LoadFont:Error: Failed creating chat ttf font! Chat will be disabled!");
		return E_FAIL;
	}
		
	LOG(L"TTF::LoadFont [%s]: loaded OK.", strFontNameID);

	//calculates the rectangle
	//RECT rc;
	//SetRect(&rc, 0, 0, 0, 0);
	//pFont->pFont->DrawTextW(m_pSprite, L"A", -1, &rc, DT_NOCLIP | DT_CALCRECT, 0);

	m_arrFonts.Add(pFont);

	return S_OK;
}

void CTTFontsManager::Release()
{
	for (int kk = 0; kk < m_arrFonts.GetSize(); kk++)
	{
		LOG(L"TTF::Font Released [%s]", m_arrFonts[kk]->shFontName.text);
		SAFE_DELETE(m_arrFonts[kk]);
	}
	m_arrFonts.RemoveAll();
}

CTTFont* CTTFontsManager::GetFont(const WCHAR* strFontName)
{
	UINT32 unHash = FastHash(strFontName);
	for (int kk = 0; kk < m_arrFonts.GetSize(); kk++)
	{
		if (m_arrFonts[kk]->shFontName.textHash == unHash)
			return m_arrFonts[kk];
	}

	ErrorBox(K_ERR_WARNING, L"[WARNING]TTF::Font not found! [%s]", strFontName);
	return null;
}

CTTFont* CTTFontsManager::GetFont(UINT32 unFontNameHash)
{
	for (int kk = 0; kk < m_arrFonts.GetSize(); kk++)
	{
		if (m_arrFonts[kk]->shFontName.textHash == unFontNameHash)
			return m_arrFonts[kk];
	}

	ErrorBox(K_ERR_WARNING, L"[WARNING]TTF::GetFont(UINT32) Font not found! [%d]", unFontNameHash);
	return null;
}


///--- system framework ---
HRESULT CTTFontsManager::OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr = S_OK;
	m_pDevice = pd3dDevice;

	return hr;
}

HRESULT CTTFontsManager::OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr = S_OK;
	m_pDevice = pd3dDevice;

	for (int kk = 0; kk < m_arrFonts.GetSize(); kk++)
	{
		if (FAILED(hr = m_arrFonts[kk]->pFont->OnResetDevice()))
		{
			ErrorBox(K_ERR_WARNING, L"[WARNING] TTF::Font Failed ResetDevice! [%s]", m_arrFonts[kk]->shFontName.text);
		}
	}

	return S_OK;
}

HRESULT CTTFontsManager::OnLostDevice(void)
{
	m_pDevice = NULL;

	for (int kk = 0; kk < m_arrFonts.GetSize(); kk++)
	{
		m_arrFonts[kk]->pFont->OnLostDevice();
	}

	return S_OK;
}

HRESULT CTTFontsManager::OnDestroyDevice(void)
{
	HRESULT hr = S_OK;

	m_pDevice = NULL;
	return hr;
}

///**************************************************************************************
/// Sigleton de acces
///**************************************************************************************

CTTFontsManager& UTGetTTFManager()
{
	static CTTFontsManager g_TTFManager;
	return g_TTFManager;
}

