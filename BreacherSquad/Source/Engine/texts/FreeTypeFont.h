#pragma once

// Free Type Font Anchor (can be combined)
#define FTFF_LEFT 1
#define FTFF_CENTER 2
#define FTFF_RIGHT 4
#define FTFF_TOP 8
#define FTFF_BOTTOM 16
#define FTFF_VCENTER 32
#define FTFF_TOPLEFT 9
#define FTFF_BOTTOMLEFT 17
#define FTFF_VCENTERLEFT 33
#define FTFF_TOPCENTER 10
#define FTFF_BOTTOMCENTER 18
#define FTFF_VCENTERHCENTER 34
#define FTFF_TOPRIGHT 12
#define FTFF_BOTTOMRIGHT 20
#define FTFF_VCENTERRIGHT 36

struct sGlyphInfo {
	int advanceX;								// cursor advance after glyph
	RECTLTRB_F			texRect;				// tex coords of module
	RECTLTRB_F			moduleRectOff;			// module rect offsetted by x_off and y_off. Origin in cursor point.
};

class CFreeTypeAtlas
{
public:
	PTEXTURE						pTex;
	SIZEWH							atlasSize;
	CArray<sGlyphInfo>				arrGlyphs;
	
	int								nMaxBearingY;	// max distance from baseline to top of all the characters

	CFreeTypeAtlas() : 
		pTex(nullptr), nMaxBearingY(0)
	{}

	~CFreeTypeAtlas()
	{
		Release();
	}

	void Release()
	{
		SAFE_RELEASE(pTex);
		arrGlyphs.RemoveAll();
	}
};

// Structure fed to CreateAtlas for advanced font rendering
struct sFreeTypeFontStyle
{
	std::string				strTexturePath;				// empty string for no texture. The specified PNG gets aligned with the center on the letter baseline.
	DWORD					dwOutlineColor;				// 0 for no outline, should always set alpha to non zero when enabled
	float					fShadowAlpha;				// 0 for no shadow
	int						shadowOffsetX;
	int						shadowOffsetY;
	float					fGeometryScale;				// 1.0f geometry is the same size as the texture (0.5 geometry half the size). Use it to give more detail to smaller text.

	sFreeTypeFontStyle() :
		dwOutlineColor(0), fShadowAlpha(0.0f),
		shadowOffsetX(0), shadowOffsetY(2),
		fGeometryScale(1.0f)
	{}

	sFreeTypeFontStyle(DWORD dwOutlineCol, float fShadowA, int nShadOffX, int nShadOffY, CHAR* strTexPath = nullptr) :
		dwOutlineColor(dwOutlineCol), fShadowAlpha(fShadowA),
		shadowOffsetX(nShadOffX), shadowOffsetY(nShadOffY)
	{
		if (strTexPath != nullptr)
			strTexturePath = strTexPath;
	}
};

class CFreeTypeFont
{
private:
	CSpritePainter*			m_pSP;

public:
	CFreeTypeAtlas			m_atlas;
	bool					bLoaded;
	CStringHash				shFontName;

public:
	int						letterSpacing;
	int						rowSpacing;
	int						spaceSize;
	
	int						rowHeight;					// Don't set manually, computed when loading the glyphs

	CFreeTypeFont();
	~CFreeTypeFont();


	// Creates texture atlas from font
	OPRESULT				CreateAtlas(PDEVICE pDevice, char* utf8Path, int nFontSize, WCHAR* wstrUniqueChars, sFreeTypeFontStyle *pStyle = nullptr);
	// Sets font style variables
	void					SetStyle(int nLetterSpacing, int nRowSpacing, int nSpaceSize);
	// Releases font and atlas
	void					Release();

	// Writes a line of texts without breaking it into multiple lines (no justify, no wrap)
	RECTXYWH				DrawStringLine(CStringDesc *strDesc, float X, float Y, UINT16 Flags = FTFF_BOTTOMLEFT, DWORD Color = 0xffffffff);
	RECTXYWH				DrawStringLine(int strID, float X, float Y, UINT16 Flags = FTFF_BOTTOMLEFT, DWORD Color = 0xffffffff);
};


