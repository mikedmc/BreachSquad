#pragma once

struct sGlyphInfo {
	int x0, y0, x1, y1;
	int x_off, y_off;  
	int advanceX;       
};

class CFreeTypeAtlas
{
public:
	PTEXTURE						pTex;
	SIZEWH							atlasSize;
	CGrowableArray<sGlyphInfo>		arrGlyphs;

	CFreeTypeAtlas() : pTex(nullptr)
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
	//static CStringsManager			*m_pStrManager;
	//static void SetManagersPtr(CStringsManager *strManager);

public:
	CFreeTypeAtlas					m_atlas;
	bool							bLoaded;
	CStringHash						shFontName;

public:
	int letterSpacing;
	int rowSpacing;
	int rowHeight;
	int spaceSize;

	CFreeTypeFont();
	~CFreeTypeFont();


	OPRESULT CreateAtlas(PDEVICE pDevice, char* utf8Path, int nFontSize, WCHAR* wstrUniqueChars);
	void Release();

	//int DrawString(CStringDesc *strDesc, float X, float Y, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
};


