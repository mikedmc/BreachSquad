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
	void Init(CHAR* animID, CSpriteCollection *sprCollection, float pX = 0.0f, float pY = 0.0f, int nframeIdx = 0, DWORD nColor = 0xffffffff);
	void setAnimation(int animIdx, int frameIdx = 0);
	void setAnimationOnce(int animIdx, int frameIdx = 0); //seteaza animatia doar daca e alta
	void setAnimationOnce_keepFrame(CSpriteCollection *sprCol, int animIdx); //seteaza animatia doar daca e alta setata si incearca sa pastreze frame-ul curent
	void setAnimation(CHAR* animID, CSpriteCollection *sprCollection);
	void SetFrame(int nFrameIdx);
	//RETURNS: AFrame flag - returns it only once when entering the frame.
	UINT32 Update(CSpriteCollection *sprCollection, float dTime, bool updatePos = false);

	void paint(CSpriteCollection *sprCol);
	void paint(CSpriteCollection *sprCol, RECT *cliprect);
	void paint(CSpriteCollection *sprCol, RECTXYWH *clipRct);
	void paintFlippedInPlace(CSpriteCollection *sprCol, bool flipX, bool flipY);
	// Optimized paint for the first module only
	void paint_firstModule(CSpriteCollection *sprCol);
	// Paints the first module of a frame forcing dwCol color
	void paint_firstModuleColorized(CSpriteCollection *sprCol, DWORD dwCol);
	//TODO: void paint_firstModule(CSpriteCollection *sprCol, RECTXYWH *clipRct);

	//deseneaza sprite-ul din alta textura din BSX (textura + texIdxOffset)
	void paint_texOverride(CSpriteCollection *sprCol, int texIdxOffset, float fOffX = 0.0f, float fOffy = 0.0f); //speciala pentru KnockJack
	//paint optimizat pt frames cu un singur modul - hardcodare pt viteza
	void paint_firstModule_texOverride(CSpriteCollection *sprCol, int texIdxOffset, float fOffX = 0.0f, float fOffy = 0.0f); //speciala pentru KnockJack

	//W=-1 inseamna ca pastreaza inaltimea originala
	void paintTiled(CSpriteCollection *sprCol, int W = -1, int H = -1);
	void paintTiledOffset(CSpriteCollection *sprCol, int W = -1, int H = -1, int offsX = 0, int offsY = 0);
	static void paintFrame(CSpriteCollection *sprCol, float nX, float nY, int animID, int frameID, DWORD ncolor = 0xffffffff);
	static void paintFrameModule(CSpriteCollection *sprCol, float nX, float nY, int animID, int frameID, int moduleID, DWORD ncolor = 0xffffffff);

	static void paintFrameClipped(CSpriteCollection *sprCol, float nX, float nY, int animID, int frameID, RECTXYWH& clip, DWORD ncolor = 0xffffffff);
	static void paintFrameModuleClipped(CSpriteCollection *sprCol, float nX, float nY, int animID, int frameID, int moduleID, RECTXYWH_F * clip, DWORD ncolor = 0xffffffff);
	static void paintFrameModuleTiled(CSpriteCollection *sprCol, float nX, float nY, int animID, int frameID, int moduleID, DWORD ncolor = 0xffffffff, int W = -1, int H = -1);
	
	//void paintTransformed(CSpriteMgr *sprManager, float scale=1.0f, float rot=0.0f);
	void paintTiledHOriented(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2);
	void paintTiledHOrientedOffset(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2, int offsetX);

	void PaintStretchedXOriented(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2);
	void PaintStretchedXOriented_texOverride(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2, int texIdxOffset);

	void paintModule_texOverride(CSpriteCollection *sprCol, int nModuleIdx, int texIdxOffset);

	static void paintFrameNEW(CSpriteCollection *sprCol, Vec3 vPos, int animID, int frameID, DWORD ncolor = 0xffffffff, float fRotZ = 0.0f, Vec2 vScale = { 1.0f, 1.0f });
	static void paintFrameModuleNEW(CSpriteCollection *sprCol, float nX, float nY, int animID, int frameID, int moduleID, DWORD ncolor = 0xffffffff);
};


extern DWORD GetFlags(CSpriteCollection* sprCol, int anim, int frame);