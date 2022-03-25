#pragma once

//--- EDITOR duration per frame (1000 = 1 sec) -----
#define SPR_ED_TIMELINE 1000.0f

// Identifier for a frame in an animation
struct SprFrameId
{
	int animIdx;
	int frameIdx;

	SprFrameId() :
		animIdx(-1), frameIdx(0)
	{}

	void Init(int nAnimIdx, int nFrameIdx)
	{
		animIdx = nAnimIdx;
		frameIdx = nFrameIdx;
	}
};

// Identifier for a frame in an animation. Use this instead of keeping separate anim and frame idx
struct SprFrameIdEx
{
	int				spriteLibIdx;				// index of sprite lib in multi sprite lib configuration
	int				animIdx;
	int				frameIdx;

	SprFrameIdEx() :
		animIdx( -1 ), frameIdx( 0 ), spriteLibIdx( 0 )
	{}

	void Init( int nAnimIdx, int nFrameIdx, int nSpriteLibIdx = 0 )
	{
		animIdx = nAnimIdx;
		frameIdx = nFrameIdx;
		spriteLibIdx = nSpriteLibIdx;
	}
};


enum eSpriteAnimState {
	ANIM_STOPPED = 0,
	ANIM_JUST_STARTED = 1,
	ANIM_PLAYING,
	ANIM_PLAYING_FRAME_ADVANCED,
	ANIM_FRAMELOCK,
	ANIM_LOOPRESET,
};

class CSpr
{
public:
	// Pointer to global sprite painter, for speed...
	static CSpritePainter*	s_pSP;						

public:
	CSpriteLib*				pSprCol;					// Pointer to sprite collection
	int						animIdx;					// Animation index
	int						frameIdx;					// Frame index in sprite collection
	Vec2					pos;	
	float					rotation;
	Vec2					scale;
	DWORD					color;

	float					fTime;						// Animation time
	eSpriteAnimState		animStatus;

	CSpr();
	CSpr(const CSpr& sprite);
	CSpr(CSpriteLib* pSpriteColl, int animIdx, float pX, float pY);
	CSpr(CSpriteLib* pSpriteColl, int animIdx, Vec2 vPos);

	void					Init(CSpriteLib *sprCollection, int nAnimIdx, Vec2 vPos = { 0.0f, 0.0f }, int nframeIdx = 0, DWORD nColor = 0xffffffff, float fRotation = 0.0f, Vec2 vScale = { 1.0f, 1.0f });
	void					Init(CSpriteLib *sprCollection, CHAR* strAnimID, Vec2 vPos = { 0.0f, 0.0f }, int nframeIdx = 0, DWORD nColor = 0xffffffff, float fRotation = 0.0f, Vec2 vScale = { 1.0f, 1.0f });

	void					SetAnim(int nAnimIdx, int nFrameIdx = 0);
	void					SetAnim(CHAR* strAnimID, int nFrameIdx = 0);
	// Sets the animation only if it's not the current one
	void					SetAnimOnce(int nAnimIdx, int nFrameIdx = 0); 
	void					SetFrame(int nFrameIdx);
	
	// RETURNS: AFrame flag - returns it only once when entering the frame. Used for sending events from the editor on each frame like footsteps and such
	// updates position only if bUpdatePos is true
	UINT32					Update(float dTime, bool bUpdatePos = false);
	// Paints current frame
	void					Paint();
	// Paints current frame clipped to clip rectangle
	void					Paint( RectLTRB& clip );
	// Paints current frame flipping the texture coords if instructed to do so (K_SPRFLAG_FLIP_X, K_SPRFLAG_FLIP_Y)
	void					PaintEx( UINT texFlipFlags );
	// Optimized paint for a single module when we don't have more modules per frame (skips a for)
	void					PaintFModule(int moduleIdx);
	// Optimized paint for a single module when we don't have more modules per frame (skips a for)
	// Adds texIdxOffset to the texture index (used when loading normals and other textures in the same sprite collection)
	void					PaintModule_texOverride(int moduleIdx, int texIdxOffset);
};

// generic data
namespace UTSprite
{
	// Paints a single frame from an animation	
	void PaintFrame( CSpriteLib *sprCol, Vec2 vPos, int animID, int frameIdx, DWORD ncolor = 0xffffffff );
	// Paints a single frame from an animation, position as 2 floats
	void PaintFrame( CSpriteLib *sprCol, float posX, float posY, int animID, int frameIdx, DWORD ncolor = 0xffffffff );
	// Paints a single frame, with transforms
	void PaintFrameEx( CSpriteLib *sprCol, Vec2 vPos, int animID, int frameIdx, DWORD ncolor = 0xffffffff, float fRotZ = 0.0f, Vec2 vScale = { 1.0f, 1.0f }, UINT unFlags = 0 );
	// Paints single module
	void PaintFModule( CSpriteLib *sprCol, Vec2 vPos, int animID, int frameIdx, int moduleIdx, DWORD ncolor = 0xffffffff );

	void PaintFrameClipped( CSpriteLib *sprCol, Vec2 vPos, int animID, int frameIdx, RectLTRB& clip, DWORD ncolor = 0xffffffff );
	void PaintFModuleClipped( CSpriteLib *sprCol, Vec2 vPos, int animID, int frameIdx, int moduleIdx, RectLTRB& clip, DWORD ncolor = 0xffffffff );
	// Stretches the module texture over the resized area replacing width and height with W and H (W/h < 0.0f means width and height stay unchanged)
	void PaintFModuleStretched( CSpriteLib *sprCol, Vec2 vPos, int animID, int frameIdx, int moduleIdx, DWORD ncolor = 0xffffffff, float W = -1.0f, float H = -1.0f );
	// fills the rect defined by vPos, W and H by tiling the specified module
	// assumes that the module is placed in 0,0 (no offset ox,oy)
	void PaintFModuleTiled( CSpriteLib *sprCol, Vec2 vPos, int animID, int frameIdx, int moduleIdx, DWORD ncolor = 0xffffffff, float W = -1.0f, float H = -1.0f );

	/*
	void paintTiled(CSpriteCollection *sprCol, int W = -1, int H = -1);
	void paintTiledOffset(CSpriteCollection *sprCol, int W = -1, int H = -1, int offsX = 0, int offsY = 0);
	void paintTiledHOriented(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2);
	void paintTiledHOrientedOffset(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2, int offsetX);
	void PaintStretchedXOriented(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2);
	void PaintStretchedXOriented_texOverride(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2, int texIdxOffset);
	*/
}

