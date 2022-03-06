#pragma once

#define ANIMATION_FLAG_LOOPED 0x1

#define ANIM_STATUS_STOPPED 0
#define ANIM_STATUS_JUST_STARTED 1
#define ANIM_STATUS_PLAYING 2
#define ANIM_STATUS_PLAYING_FRAME_ADVANCED 3
#define ANIM_STATUS_FRAMELOCK 4
#define ANIM_STATUS_LOOPRESET 5

//setup macros
#define SPRITE_SETPOS(sprite, nX, nY) {sprite.X = nX; sprite.Y = nY;}

class CSprite
{
public:
	static ID3DXSprite*		s_pSprite;				
	static CSpritePainter*	s_pSP;						// Pointer to sprite painter
	// Sets the sprite used for painting (s_pSprite)
	static void SetGlobalSpritePtr(ID3DXSprite* pSprite, CSpritePainter* pSP);	
public:
	int		animationIdx;
	D3DXVECTOR2 pos;

	float	timePassed;
	int		currentFrame;
	int		animStatus;									//ANIM_FRAMELOCK, etc
	DWORD	color;

	CSprite(void);
	CSprite(int animIdx, float pX, float pY);
	CSprite(const CSprite& sprite);
	CSprite(int animIdx, D3DXVECTOR2 vPos);

	void Init(int animIdx, float pX = 0.0f, float pY = 0.0f, int nframeIdx = 0, DWORD nColor = 0xffffffff);
	void Init(CHAR* animID, CSpriteLib *sprCollection, float pX = 0.0f, float pY = 0.0f, int nframeIdx = 0, DWORD nColor = 0xffffffff);
	void setAnimation(int animIdx, int frameIdx = 0);
	void setAnimationOnce(int animIdx, int frameIdx = 0); //seteaza animatia doar daca e alta
	void setAnimationOnce_keepFrame(CSpriteLib *sprCol, int animIdx); //seteaza animatia doar daca e alta setata si incearca sa pastreze frame-ul curent
	void setAnimation(CHAR* animID, CSpriteLib *sprCollection);
	void SetFrame(int nFrameIdx);
	//RETURNS: AFrame flag - returns it only once when entering the frame.
	UINT32 Update(CSpriteLib *sprCollection, float dTime, bool updatePos = false);

	void paint(CSpriteLib *sprCol);
	void paint(CSpriteLib *sprCol, RECT *cliprect);
	void paint(CSpriteLib *sprCol, RectXYWHi *clipRct);
	void paintFlippedInPlace(CSpriteLib *sprCol, bool flipX, bool flipY);
	// Optimized paint for the first module only
	void paint_firstModule(CSpriteLib *sprCol);
	// Paints the first module of a frame forcing dwCol color
	void paint_firstModuleColorized(CSpriteLib *sprCol, DWORD dwCol);
	//TODO: void paint_firstModule(CSpriteCollection *sprCol, RECTXYWH *clipRct);

	//deseneaza sprite-ul din alta textura din BSX (textura + texIdxOffset)
	void paint_texOverride(CSpriteLib *sprCol, int texIdxOffset, float fOffX = 0.0f, float fOffy = 0.0f); //speciala pentru KnockJack
	//paint optimizat pt frames cu un singur modul - hardcodare pt viteza
	void paint_firstModule_texOverride(CSpriteLib *sprCol, int texIdxOffset, float fOffX = 0.0f, float fOffy = 0.0f); //speciala pentru KnockJack

	//W=-1 inseamna ca pastreaza inaltimea originala
	void paintTiled(CSpriteLib *sprCol, int W = -1, int H = -1);
	void paintTiledOffset(CSpriteLib *sprCol, int W = -1, int H = -1, int offsX = 0, int offsY = 0);
	static void paintFrame(CSpriteLib *sprCol, float nX, float nY, int animID, int frameID, DWORD ncolor = 0xffffffff);
	static void paintFrameModule(CSpriteLib *sprCol, float nX, float nY, int animID, int frameID, int moduleID, DWORD ncolor = 0xffffffff);

	static void paintFrameClipped(CSpriteLib *sprCol, float nX, float nY, int animID, int frameID, RectXYWHi& clip, DWORD ncolor = 0xffffffff);
	static void paintFrameModuleClipped(CSpriteLib *sprCol, float nX, float nY, int animID, int frameID, int moduleID, RectXYWH * clip, DWORD ncolor = 0xffffffff);
	static void paintFrameModuleTiled(CSpriteLib *sprCol, float nX, float nY, int animID, int frameID, int moduleID, DWORD ncolor = 0xffffffff, int W = -1, int H = -1);
	
	//void paintTransformed(CSpriteMgr *sprManager, float scale=1.0f, float rot=0.0f);
	void paintTiledHOriented(CSpriteLib *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2);
	void paintTiledHOrientedOffset(CSpriteLib *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2, int offsetX);

	void PaintStretchedXOriented(CSpriteLib *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2);
	void PaintStretchedXOriented_texOverride(CSpriteLib *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2, int texIdxOffset);

	void paintModule_texOverride(CSpriteLib *sprCol, int nModuleIdx, int texIdxOffset);

	static void paintFrameNEW(CSpriteLib *sprCol, Vec2 vPos, int animID, int frameID, DWORD ncolor = 0xffffffff, float fRotZ = 0.0f, Vec2 vScale = { 1.0f, 1.0f });
	static void paintFrameModuleNEW(CSpriteLib *sprCol, float nX, float nY, int animID, int frameID, int moduleID, DWORD ncolor = 0xffffffff);
};


extern DWORD GetFlags(CSpriteLib* sprCol, int anim, int frame);