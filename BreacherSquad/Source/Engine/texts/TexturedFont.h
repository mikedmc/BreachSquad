#pragma once

#include "TTFont.h"
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

//declarare clasa
class CTexFontsManager;
	
//MODULE - folosit doar la incarcare
class tfModule 
{
public:
	UINT16 imgIdx;
	int X;
	int Y;
	int W;
	int H;
};
//FrameModule  - folosit doar la incarcare din XML
typedef struct _ntfFModule 
{
	//offsetul unde deseneaza modulul
	int ox;
	int oy;
	UINT32 flags;
	//datele luate din modul
	UINT16 imgIdx;
	RECT moduleRect;
	int moduleX;
	int moduleY;
	int moduleW;
	int moduleH;
} tfFModule;

class CTexFont
{
private:
	bool loaded;
	static CStringsManager			*m_pStrManager;
	static CTexFontsManager	*m_pFontsManager; 
	static ID3DXSprite*              s_pSprite; //sprite folosit la desenare

public:
	static LPDIRECT3DDEVICE9 pDevice;

	CTexNode*	pTexNode;						// pointer to managed texture node
	WCHAR		strLoadedTexture[MAX_PATH];
	WCHAR		strLoadedFile[MAX_PATH];
	/////////// modules /////////////
	int			moduleNo;

	int*		fmodule_ox;
	int*		fmodule_oy;
	RECT*		moduleRect;
	int*		moduleW;
	int*		moduleH;
	RECTXYWH*	frameBBox;

	CStringHash			shFontName;
	//replacement TTF font (used to replace the bitmap font with a TTF one)
	CTTFont*			pFontReplacementTTF;		//if not null use it to draw instead of using the bitmap
	CCameraTransform*	pFontReplacementCam;		//transform used for replacement font (maybe game has too low resolution and we need another transform)
	float				fFontReplacementCamScaling; //default scaling between game camera and Font camera
	bool				bFontReplacementOn;			//if on, use font replacement (used to turn off TTF ocasionally)

public:
	UINT32 ID;		//id font
	int letterSpacing;  //distanta dintre litere
	int rowSpacing;	   //distanta dintre 2 randuri (se adauga la rowAvgHeight)
	int rowHeight;  //marimea literelor majuscule
	int spaceSize;     //marimea caracterului spatiu

	CTexFont(void);
	~CTexFont(void);

	//se cheama o singura data ca sa se lege de managerul de stringuri
	static void SetManagersPtr(CStringsManager *strManager, CTexFontsManager *fontsManager);
	static void SetGlobalSpritePtr(ID3DXSprite* pSprite);

	//returns the row height
	int GetRowHeight(bool bIncludeSpacing);

	HRESULT LoadFontXML(WCHAR* XMLpath);
	void Release();
	void SetFontReplacementTTF(CTTFont* pReplacementTTF, CCameraTransform* pCamTransformTTF = null);

	//deseneaza string pe o linie si clamping dupa o dimensiune anume...
	int DrawStringClamped(CStringDesc *strDesc, int X, int Y, int maxW, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
	int DrawStringClamped(int strIdx, int X, int Y, int maxW, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);

	//deseneaza string pe o singura linie. Daca latimea e mai mare decat latimea maxima, scaleaza tot textul ca sa se incadreze
	int DrawStringScaleW(CStringDesc *strDesc, int X, int Y, int maxW, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
	int DrawStringScaleW(int strIdx, int X, int Y, int maxW, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
	
	int DrawStringScaleW(CStringDesc *strDesc, RECTXYWH rect, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
	int DrawStringScaleW(int strIdx, RECTXYWH rect, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
	//deseneaza textul si plimba o lumina pe el (este mai lent pt ca forteaza flush pt additive blending)
	//params: fLightPos - intre 0 si 1 reprezentand lungimea stringului; fLightRange - range lumina in pixeli
	int DrawStringLightened(int strIdx, int X, int Y, float fLightPos, float fLightRange, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
	int DrawStringLightened(CStringDesc *strDesc, int X, int Y, float fLightPos, float fLightRange, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);

	int DrawStringTransformed(CStringDesc *strDesc, int X, int Y, float scale, float rotation, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
	int DrawString(CStringDesc *strDesc, float X, float Y, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
	///--- functiile care primesc index gen STR_TITLE ---
	int DrawStringClipped(int strIdx, float X, float Y, RECTXYWH clipRect, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
	int DrawStringClipped(CStringDesc *strDesc, float X, float Y, RECTXYWH clipRect, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);

	int DrawString(int strIdx, float X, float Y, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
	void DrawString(int strIdx, RECTXYWH rect, UINT16 Flags = FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, DWORD Color = 0xffffffff);
	void DrawString(CStringDesc *strDesc, RECTXYWH rect, UINT16 Flags = FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, DWORD Color = 0xffffffff);
	//folosit ca sa poti face scroll pe Y in dreptunghiul rect
	//face automat WRAP la text
	void DrawStringOffsetY(int strIdx, RECTXYWH rect, int offsetY, UINT16 Flags = FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, DWORD Color = 0xffffffff);

	//masoara lungimea textului pt ca nu are param de latime
	SIZEWH MeasureString(int strIdx);
	SIZEWH MeasureString(CStringDesc *strDesc);
	//masoara latimea si inaltimea pe care se intinde textul
	SIZEWH MeasureString(CStringDesc* strDesc, int maxWidth);
	SIZEWH MeasureString(int strIdx, int maxWidth);
	
	///--- functiile care primesc HASH ID - folosite de obicei de controalele care incarca nume si salveaza hash ---
	int DrawHString(UINT32 strHash, int X, int Y, UINT16 Flags = FONTFLAG_ANCHOR_BOTTOMLEFT, DWORD Color = 0xffffffff);
	void DrawHString(UINT32 strHash, RECTXYWH rect, UINT16 Flags = FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, DWORD Color = 0xffffffff);
	void DrawHStringOffsetY(UINT32 strHash, RECTXYWH rect, int offsetY, UINT16 Flags = FONTFLAG_ANCHOR_TOPLEFT | FONTFLAG_WRAPTEXT, DWORD Color = 0xffffffff);
	SIZEWH MeasureHString(UINT32 strHash);
	SIZEWH MeasureHString(UINT32 strHash, int maxWidth);
	
	///--- system framework ---
	HRESULT OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnLostDevice(void);
	HRESULT OnDestroyDevice(void);

};


//***********************************************************
// Textured Font Manager
//***********************************************************
class CTexFontsManager 
{
private:
	LPDIRECT3DDEVICE9 pDevice;
	CStringsManager *globalStrManager;
public:
	CTextureManager m_texManager;
	CArray<CTexFont*> fonts;

	void SetManagersPtr(CStringsManager *pStrManager);
	//adauga cate un font din fisier XML
	HRESULT AddFontXML(WCHAR *XMLpath, int *retIdx = NULL);
	int GetFontIdx(const CHAR* fontID);
	int GetFontIdx(const WCHAR* fontID);

	CTexFont* operator[] (const CHAR* fontID);
	CTexFont* operator[] (const int fontIdx);

	CTexFontsManager();
	~CTexFontsManager();

	// Puts pause on using TTFonts (for text particles, small controller buttons, etc)
	void SetPauseOnTTFontsReplacement(bool bPaused);

	void Release();

	///--- system framework ---
	HRESULT OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnLostDevice(void);
	HRESULT OnDestroyDevice(void);
};

//declar singletonul
CTexFontsManager& __TexFonts();

