#pragma once

//#TODO: Cand face wrap, daca e un cuvand mai lung decat spatiul acordat, se blocheaza in paint in while
//#TODO: La alinierea pe verticala sa nu mai ia in seama inaltimea efectiva adunata a literelor ci sa se ia dupa rowHeight si rowSpacing doar

///--- FORMAT ---
// este acelasi format ca si la sprites dar are o linie in plus, cea cu datele despre font
//<?xml version="1.0"?>
//<SpriteCollection Version="2.0">
///<FontData ID="FONTID_MED" LetterSpacing="-1" RowSpacing="4" RowHeight="18" SpaceSize="6" />
//  <Image>fontmediu.png</Image>
//  <Modules>
//    <Module ImageIdx="0" X="1" Y="3" W="17" H="18" />
//  </Modules>
//  <FrameModules>
//    <FrameModule ModuleIdx="0" OX="0" OY="-16" Flags="0" />
//  </FrameModules>
//  <AnimationFrames />
//  <Animations />
//</SpriteCollection>

/*
///--- font anchors ---
#define FONTFLAG_ANCHOR_LEFT 1
#define FONTFLAG_ANCHOR_CENTER 2
#define FONTFLAG_ANCHOR_RIGHT 4
#define FONTFLAG_ANCHOR_TOP 8
#define FONTFLAG_ANCHOR_BOTTOM 16
#define FONTFLAG_ANCHOR_VCENTER 32
//combinatii de flags
#define FONTFLAG_ANCHOR_TOPLEFT 9
#define FONTFLAG_ANCHOR_BOTTOMLEFT 17
#define FONTFLAG_ANCHOR_VCENTERLEFT 33
#define FONTFLAG_ANCHOR_TOPCENTER 10
#define FONTFLAG_ANCHOR_BOTTOMCENTER 18
#define FONTFLAG_ANCHOR_VCENTERHCENTER 34
#define FONTFLAG_ANCHOR_TOPRIGHT 12
#define FONTFLAG_ANCHOR_BOTTOMRIGHT 20
#define FONTFLAG_ANCHOR_VCENTERRIGHT 36

//cand e setat flagul de wrap, pe functia care primeste dreptunghi ca parametru face wrap
#define FONTFLAG_WRAPTEXT 64
//daca e setat flagul de clip taie din cuvinte ca sa incapa fix in dreptunghiul delimitator
//daca depaseste in jos, taie si din linii
#define FONTFLAG_CLIPTEXT 128
//format justify la text
#define FONTFLAG_JUSTIFY 256
/////////////
#define FONT_MIN_LETTER_SPACING 0
#define FONT_MIN_ROW_SPACING 15
#define FONT_MIN_SPACE_SIZE 5
#define FONT_MIN_ROW_HEIGHT 20
  */
class CTexFontsManager;
	
//MODULE - used to load modules data
struct CTFModule 
{
	UINT16 imgIdx;
	int X;
	int Y;
	int W;
	int H;
};
//FrameModule - used to load frame module data
struct CTFFModule 
{
	int ox;
	int oy;
	UINT32 flags;
	UINT16 imgIdx;
	RECT moduleRect;
	int moduleX;
	int moduleY;
	int moduleW;
	int moduleH;
};

class CTexFont
{
private:
	bool loaded;

public:
	int			nFontsMgrTexManagerIDX;		
	WCHAR		strLoadedTexture[MAX_PATH];
	WCHAR		strLoadedFile[MAX_PATH];

	int			moduleNo;
	RECTLTRB_F*	moduleUV;		// UV coords 
	RECTLTRB_F* moduleRect;		// default position and size from editor
	RECTXYWH*	frameBBox;

	CStringHash			shFontName;
public:						
	UINT32		ID;					// font name hash
	int			letterSpacing;		
	int			rowSpacing;	   
	int			rowHeight;  
	int			spaceSize;     

	CTexFont(void);
	~CTexFont(void);

	//returns the row height
	int GetRowHeight(bool bIncludeSpacing);

	OPRESULT LoadFontXML(WCHAR* XMLpath);
	void Release();

	// Single line text, displays ... (three dots) if length > maxW
	int DrawStringClamped(CStringDesc *strDesc, int X, int Y, int maxW, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
	int DrawStringClamped(int strIdx, int X, int Y, int maxW, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);

	//deseneaza string pe o singura linie. Daca latimea e mai mare decat latimea maxima, scaleaza tot textul ca sa se incadreze
	int DrawStringScaleW(CStringDesc *strDesc, int X, int Y, int maxW, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
	int DrawStringScaleW(int strIdx, int X, int Y, int maxW, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
	
	int DrawStringScaleW(CStringDesc *strDesc, RECTXYWH rect, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
	int DrawStringScaleW(int strIdx, RECTXYWH rect, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);

	int DrawString(int strIdx, float X, float Y, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
	void DrawString(int strIdx, RECTXYWH rect, UINT16 Flags = FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, DWORD Color = 0xffffffff);
	void DrawString(CStringDesc *strDesc, RECTXYWH rect, UINT16 Flags = FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, DWORD Color = 0xffffffff);
	int DrawString( CStringDesc *strDesc, float X, float Y, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff );
	//folosit ca sa poti face scroll pe Y in dreptunghiul rect
	//face automat WRAP la text
	//void DrawStringOffsetY(int strIdx, RECTXYWH rect, int offsetY, UINT16 Flags = FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, DWORD Color = 0xffffffff);

	//masoara lungimea textului pt ca nu are param de latime
	SIZEWH MeasureString(int strIdx);
	SIZEWH MeasureString(CStringDesc *strDesc);
	//masoara latimea si inaltimea pe care se intinde textul
	SIZEWH MeasureString(CStringDesc* strDesc, int maxWidth);
	SIZEWH MeasureString(int strIdx, int maxWidth);
	
	int DrawHString(UINT32 strHash, int X, int Y, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
	void DrawHString(UINT32 strHash, RECTXYWH rect, UINT16 Flags = FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, DWORD Color = 0xffffffff);
	//void DrawHStringOffsetY(UINT32 strHash, RECTXYWH rect, int offsetY, UINT16 Flags = FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, DWORD Color = 0xffffffff);
	SIZEWH MeasureHString(UINT32 strHash);
	SIZEWH MeasureHString(UINT32 strHash, int maxWidth);
};


//***********************************************************
// Textured Font Manager
//***********************************************************
class CTexFontsManager 
{
private:
	PDEVICE					m_pDevice;

public:
	CTextureManager			m_texManager;
	CArray<CTexFont*>		fonts;

public:
	CTexFontsManager();
	~CTexFontsManager();

	// Adds one front from file
	OPRESULT				AddFontXML(WCHAR *XMLpath, int *retIdx = NULL);
	int						GetFontIdx(const CHAR* fontID);
	int						GetFontIdx(const WCHAR* fontID);

	CTexFont* operator[] (const CHAR* fontID);
	CTexFont* operator[] (const int fontIdx);

	void					Release();

	///--- system framework ---
	OPRESULT OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc = nullptr );
	OPRESULT OnResetDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc = nullptr );
	OPRESULT OnLostDevice();
	OPRESULT OnDestroyDevice();
};

//declar singletonul
CTexFontsManager& __TexFonts();

