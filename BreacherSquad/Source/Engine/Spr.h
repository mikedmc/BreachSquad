#pragma once

// Identifier for a frame in an animation
struct scFrameID
{
	int animIdx;
	int frameIdx;

	scFrameID() :
		animIdx(-1), frameIdx(0)
	{}

	void Init(int nAnimIdx, int nFrameIdx)
	{
		animIdx = nAnimIdx;
		frameIdx = nFrameIdx;
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
	CSpriteCollection*		pSprCol;					// Pointer to sprite collection
	int						animIdx;						// Animation index
	int						frameIdx;					// Frame index in sprite collection
	Vec2					pos;	
	float					rotation;
	Vec2					scale;
	DWORD					color;

	float					fTime;						// Animation time
	eSpriteAnimState		animStatus;

	CSpr();
	CSpr(const CSpr& sprite);
	CSpr(CSpriteCollection* pSpriteColl, int animIdx, float pX, float pY);
	CSpr(CSpriteCollection* pSpriteColl, int animIdx, Vec2 vPos);

	void					Init(CSpriteCollection *sprCollection, int nAnimIdx, Vec2 vPos = { 0.0f, 0.0f }, int nframeIdx = 0, DWORD nColor = 0xffffffff, float fRotation = 0.0f, Vec2 vScale = { 1.0f, 1.0f });
	void					Init(CSpriteCollection *sprCollection, CHAR* strAnimID, Vec2 vPos = { 0.0f, 0.0f }, int nframeIdx = 0, DWORD nColor = 0xffffffff, float fRotation = 0.0f, Vec2 vScale = { 1.0f, 1.0f });

	void					SetAnim(int nAnimIdx, int nFrameIdx = 0);
	void					SetAnim(CHAR* strAnimID, int nFrameIdx = 0);
	// Sets the animation only if it's not the current one
	void					SetAnimOnce(int nAnimIdx, int nFrameIdx = 0); 
	void					SetFrame(int nFrameIdx);
	
	// RETURNS: AFrame flag - returns it only once when entering the frame. Used for sending events from the editor on each frame like footsteps and such
	// updates position only if bUpdatePos is true
	UINT32					Update(float dTime, bool bUpdatePos = false);

	void					Paint();
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
	void PaintFrame( CSpriteCollection *sprCol, Vec2 vPos, int animID, int frameIdx, DWORD ncolor = 0xffffffff );
	// Paints a single frame from an animation, position as 2 floats
	void PaintFrame( CSpriteCollection *sprCol, float posX, float posY, int animID, int frameIdx, DWORD ncolor = 0xffffffff );
	// Paints a single frame, with transforms
	void PaintFrameEx( CSpriteCollection *sprCol, Vec2 vPos, int animID, int frameIdx, DWORD ncolor = 0xffffffff, float fRotZ = 0.0f, Vec2 vScale = { 1.0f, 1.0f }, UINT unFlags = 0 );
	// Paints single module
	void PaintFModule( CSpriteCollection *sprCol, Vec2 vPos, int animID, int frameIdx, int moduleIdx, DWORD ncolor = 0xffffffff );

	void PaintFrameClipped( CSpriteCollection *sprCol, float nX, float nY, int animID, int frameIdx, RECTLTRB_F& clip, DWORD ncolor = 0xffffffff );
	void PaintFModuleClipped( CSpriteCollection *sprCol, float nX, float nY, int animID, int frameIdx, int moduleIdx, RECTLTRB_F& clip, DWORD ncolor = 0xffffffff );
	// Stretches the module texture over the resized area replacing width and height with W and H (W/h < 0.0f means width and height stay unchanged)
	void PaintFModuleStretched( CSpriteCollection *sprCol, Vec2 vPos, int animID, int frameIdx, int moduleIdx, DWORD ncolor = 0xffffffff, float W = -1.0f, float H = -1.0f );

	/*
	//#TODO: maybe needed later
	void PaintFModuleTiled( CSpriteCollection *sprCol, Vec2 vPos, int animID, int frameIdx, int moduleIdx, DWORD ncolor = 0xffffffff, int W = -1, int H = -1 );
	void paintTiled(CSpriteCollection *sprCol, int W = -1, int H = -1);
	void paintTiledOffset(CSpriteCollection *sprCol, int W = -1, int H = -1, int offsX = 0, int offsY = 0);
	void paintTiledHOriented(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2);
	void paintTiledHOrientedOffset(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2, int offsetX);
	void PaintStretchedXOriented(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2);
	void PaintStretchedXOriented_texOverride(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2, int texIdxOffset);
	*/
}

