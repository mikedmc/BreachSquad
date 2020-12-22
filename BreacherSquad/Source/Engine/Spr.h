#pragma once

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
	int						animID;						// Animation index
	int						frameID;					// Frame index in sprite collection
	Vec2					pos;		

	float					fTime;						// Animation time
	eSpriteAnimState		animStatus;
	DWORD					color;

	CSpr();
	CSpr(int animIdx, float pX, float pY);
	CSpr(const CSpr& sprite);
	CSpr(int animIdx, Vec2 vPos);

	void					Init(int animIdx, Vec2 vPos = { 0.0f, 0.0f }, int nframeIdx = 0, DWORD nColor = 0xffffffff);
	void					Init(CHAR* strAnimID, CSpriteCollection *sprCollection, Vec2 vPos = { 0.0f, 0.0f }, int nframeIdx = 0, DWORD nColor = 0xffffffff);

	void					SetAnim(int animIdx, int frameIdx = 0);
	void					SetAnim(CSpriteCollection *sprCollection, CHAR* strAnimID);
	// Sets the animation only if it's not the current one
	void					SetAnimOnce(int animIdx, int frameIdx = 0); 
	void					SetFrame(int nFrameIdx);
	
	//RETURNS: AFrame flag - returns it only once when entering the frame. Used for sending events from the editor on each frame like footsteps and such
	UINT32					Update(CSpriteCollection *sprCollection, float dTime, bool bUpdatePos = false);

	void					Paint(CSpriteCollection *sprCol);
	// Optimized paint for a single module when we don't have more modules per frame (skips a for)
	void					PaintModule(CSpriteCollection *sprCol, int moduleIdx);
	// Optimized paint for a single module when we don't have more modules per frame (skips a for)
	// Adds texIdxOffset to the texture index (used when loading normals and other textures in the same sprite collection)
	void					PaintModule_texOverride(CSpriteCollection *sprCol, int moduleIdx, int texIdxOffset);
};

// generic data
namespace UTSprite
{
	DWORD GetFrameFlags(CSpriteCollection* sprCol, int animID, int frameID);
	/*
	void PaintFrame(CSpriteCollection *sprCol, float nX, float nY, int animID, int frameID, DWORD ncolor = 0xffffffff);
	void PaintFrameModule(CSpriteCollection *sprCol, float nX, float nY, int animID, int frameID, int moduleID, DWORD ncolor = 0xffffffff);
	void PaintFrameClipped(CSpriteCollection *sprCol, float nX, float nY, int animID, int frameID, RECTXYWH& clip, DWORD ncolor = 0xffffffff);
	void PaintFrameModuleClipped(CSpriteCollection *sprCol, float nX, float nY, int animID, int frameID, int moduleID, RECTXYWH_F * clip, DWORD ncolor = 0xffffffff);
	void PaintFrameModuleTiled(CSpriteCollection *sprCol, float nX, float nY, int animID, int frameID, int moduleID, DWORD ncolor = 0xffffffff, int W = -1, int H = -1);
	*/
	void PaintFrameEx(CSpriteCollection *sprCol, Vec2 vPos, int animID, int frameID, DWORD ncolor = 0xffffffff, float fRotZ = 0.0f, Vec2 vScale = { 1.0f, 1.0f });
	void PaintFrameModule(CSpriteCollection *sprCol, Vec2 vPos, int animID, int frameID, int moduleID, DWORD ncolor = 0xffffffff);

	/*
	//W=-1 inseamna ca pastreaza inaltimea originala
	void paintTiled(CSpriteCollection *sprCol, int W = -1, int H = -1);
	void paintTiledOffset(CSpriteCollection *sprCol, int W = -1, int H = -1, int offsX = 0, int offsY = 0);

	void paintTiledHOriented(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2);
	void paintTiledHOrientedOffset(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2, int offsetX);

	void PaintStretchedXOriented(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2);
	void PaintStretchedXOriented_texOverride(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2, int texIdxOffset);
	*/
}

