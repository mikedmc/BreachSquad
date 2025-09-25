#pragma once

//--- EDITOR duration per frame (1000 = 1 sec) -----
#define SPR_ED_TIMELINE 1000.0f

// Identifier for a frame in an animation
struct SprFrameId
{
	int animIdx;
	int frameIdx;

	SprFrameId() :
		animIdx( -1 ), frameIdx( 0 )
	{}

	void Init( int nAnimIdx, int nFrameIdx )
	{
		animIdx = nAnimIdx;
		frameIdx = nFrameIdx;
	}

	void Reset()
	{
		animIdx = -1;
		frameIdx = 0;
	}

	inline bool IsSet()
	{
		return ( animIdx >= 0 ) ? true : false;
	}

	void Set( int anim, int frame )
	{
		animIdx = anim;
		frameIdx = frame;
	}
};

// Identifier for a frame in an animation in a spriteLib. Use this instead of keeping separate anim and frame idx
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

	void Reset()
	{
		animIdx = -1;
		frameIdx = 0;
		spriteLibIdx = 0;
	}

	inline bool IsSet()
	{
		return ( animIdx >= 0 ) ? true : false;
	}
};


enum ESpriteAnimState {
	ANIM_JUST_STARTED = 1,
	ANIM_PLAYING,
	ANIM_PLAYING_FRAME_ADVANCED,
	ANIM_FRAMELOCK,		// animation reached end
	ANIM_LOOPRESET,
};

class CSpr
{
public:
	// Pointer to global sprite painter, for speed...
	static CSpritePainter*	s_pSP;						
private:
	UINT32					curFrameEvent;				// contains current frame animation flag for a single frame, when entering the frame. It contains the AFrameFlag but gets reset to 0 after a single frame.
	float					fTimeScale;					// used for scaling the timeline

public:
	CSpriteLib*				pSprCol;					// Pointer to sprite collection
	int						animIdx;					// Animation index
	int						frameIdx;					// Frame index in sprite collection
	Vec2					pos;	
	float					rotation;
	Vec2					scale;
	DWORD					color;
	float					fTime;						// Animation time
	int						animDirection;				// direction for animation playback
	ESpriteAnimState		animStatus;					// animation status

	CSpr();
	CSpr( const CSpr& sprite );
	CSpr( CSpriteLib* pSpriteColl );
	CSpr( CSpriteLib* pSpriteColl, int animationIdx, float pX, float pY );
	CSpr( CSpriteLib* pSpriteColl, int animationIdx, Vec2 vPos );

	void					Init(CSpriteLib *sprCollection, int nAnimIdx, Vec2 vPos = { 0.0f, 0.0f }, int nframeIdx = 0, DWORD nColor = 0xffffffff, float fRotation = 0.0f, Vec2 vScale = { 1.0f, 1.0f });
	void					Init(CSpriteLib *sprCollection, CHAR* strAnimID, Vec2 vPos = { 0.0f, 0.0f }, int nframeIdx = 0, DWORD nColor = 0xffffffff, float fRotation = 0.0f, Vec2 vScale = { 1.0f, 1.0f });

	void					SetAnim(int nAnimIdx, int nFrameIdx = 0);
	void					SetAnim(CHAR* strAnimID, int nFrameIdx = 0);
	// Sets the animation only if it's not the current one. Returns true if set, false if not set
	bool					SetAnimOnce(int nAnimIdx, int nFrameIdx = 0); 
	void					SetFrame(int nFrameIdx);
	// updates position only if bUpdatePos is true
	void					Update(float dTime, bool bUpdatePos = false);
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
	void					PaintFModule_texOverride(int moduleIdx, int texIdxOffset);
	// Stops the playing animation
	void					Stop();
	// scales the current animation duration to the desired target_duration_sec
	void					ScaleAnimTime( float target_duration_sec );
	// sets the animation speed multiplier. Gets reset when changing the animation.
	void					SetAnimSpeed( float time_multiplier = 1.0f );
	// Sets animation play direction. bRewind=true sets the frame on first or last frame depending on the direction.	
	void					SetAnimDirection( bool bReverseAnimation, bool bRewind = false );
	// resumes/restarts playing of animation
	void					Play( bool bReset = false );

	// returns current AFrame flag (only valid for one update loop, cleared if frame takes longer)
	FORCEINLINE UINT32		GetCurFrameEvent() { return curFrameEvent; }
	// returns current frame bbox
	RectXYWHi				GetAFrameBBox();
	// returns current frame flags
	UINT32					GetAframeFlags();

private:
	// resets everything to defaults (without resetting the lib pointers)
	void					Reset();
};

//#TODO: finish implementing all methods!
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

	void PaintFrameClipped( CSpriteLib *sprCol, Vec2 vPos, int animID, int frameIdx, RectXYWH& clip, DWORD ncolor = 0xffffffff );
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

