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
	// should remove these:
	int x0, y0, x1, y1;
	int x_off, y_off;  
	
	int advanceX;								// cursor advance after glyph

	RECTLTRB_F			texRect;				// tex coords of module
	RECTLTRB_F			moduleRectOff;			// module rect offsetted by x_off and y_off. Origin in cursor point.
};

class CFreeTypeAtlas
{
public:
	PTEXTURE						pTex;
	SIZEWH							atlasSize;
	CGrowableArray<sGlyphInfo>		arrGlyphs;
	
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
	OPRESULT				CreateAtlas(PDEVICE pDevice, char* utf8Path, int nFontSize, WCHAR* wstrUniqueChars);
	// Sets font style variables
	void					SetStyle(int nLetterSpacing, int nRowSpacing, int nSpaceSize);
	// Releases font and atlas
	void					Release();

	// Writes a line of texts without breaking it into multiple lines (no justify, no wrap)
	RECTXYWH				DrawStringLine(CStringDesc *strDesc, float X, float Y, UINT16 Flags = FTFF_BOTTOMLEFT, DWORD Color = 0xffffffff);
	RECTXYWH				DrawStringLine(int strID, float X, float Y, UINT16 Flags = FTFF_BOTTOMLEFT, DWORD Color = 0xffffffff);
};


