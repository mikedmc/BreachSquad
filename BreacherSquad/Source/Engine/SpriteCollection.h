#pragma once

#define K_EDITOR_ANIMATION_FLAG_LOOPED 0x1

//----------------------------------------------------------	
// bsx FILE FORMAT 
//<?xml version="1.0"?>
//<SpriteCollection Version="2.0">
//  <Image>intro1.png</Image>
//  <Modules>
//    <Module ImageIdx="0" X="704" Y="656" W="80" H="60" />
//  </Modules>
//  <FrameModules>
//    <FrameModule ModuleIdx="8" OX="0" OY="0" Flags="0" />
//  </FrameModules>
//  <Frames>
//    <Frame BBoxX="-275" BBoxY="-160" BBoxW="549" BBoxH="319">
//      <FModules>
//        <FModule Idx="0" />
//      </FModules>
//      <Points>
//        <Point X="159" Y="-147" Flags="12" />
//        <Point X="223" Y="-93" Flags="0" />
//      </Points>
//    </Frame>
//  </Frames>
//  <AnimationFrames>
//    <AnimationFrame FrameIdx="3" Duration="1" DX="0" DY="0" Flags="0" />
//  </AnimationFrames>
//  <Animations>
//    <Animation ID="OCCLUDERS" Flags="0">
//      <AFrame Idx="0" />
//      <AFrame Idx="1" />
//    </Animation>
//  </Animations>
//</SpriteCollection>
//-------------------------------------------------------------------------	

#define BSX_VERSION 2.0

//Texture class
class scTexture
{
public:
	PTEXTURE			pTex;
	TEXTURE_INFO		info;
	WCHAR				imagePath[MAX_PATH];
	//CTOR
	scTexture() :
		pTex(NULL)
	{
	}
};

//MODULE - used only when loading
struct scModule 
{
	UINT16				imgIdx;
	int					X;
	int					Y;
	int					W;
	int					H;
};

//FrameModule
struct scFModule 
{
	int					ox;						// offset where module is to be painted
	int					oy;						// offset where module is to be painted
	UINT32				flags;
	UINT16				imgIdx;					// image index in texture array
	scTexture*			pImg;					// pointer to texture for fast access
	RECT				moduleRect;				// RECT of module
	RECTXYWH			moduleXYWH;				// XYWH of module
	
	RECTLTRB_F			texRect;				// tex coords of module
	RECTLTRB_F			moduleRectOff;			// module rect offsetted by ox and oy
};

//Frame - only used when loading from file (doubled in AFrame)
struct scFrame 
{
	RECTXYWH			BBox;
	int					frame_hitPtsNo;
	int*				frame_hitPtsPosXYF;		//written like this: [frame][x1,y1,flag1,x2,y2...]
	UCHAR				frame_fmodulesNo;  
	int*				frame_fmodulesIdx;
};
//AnimationFrame
struct scAFrame
{
	int					dx;
	int					dy;
	UINT32				flags;
	int					duration;
	RECTXYWH			BBox;					// BBox loaded from editor
	int					PointsNo;				// hitpoints no
	int*				PointsXYFlag;			// [frame][x1,y1,flag1,x2,y2...]
	int					fmodulesNo;  
	int*				fmodulesIdx;
	
	RECTXYWH			BBox_real;				// bounding box real, computed from modules
};

//Animation
#define K_SPRITECOLLECTION_MAX_ANIM_NAME 128
class scAnimation
{
public:
	int					aframesNo;
	int*				aframesIdx;				// [A-frameid1, A-frameid2, ...]
	UINT32				flags;
	
	CStringHash			animName;				// save anim name and hash
};


/*!
 * \class CSpriteCollection
 *
 * \brief Loads a bsx sprites file including textures
 */
class CSpriteCollection
{
private:
	bool							bIsLoaded;
	PDEVICE							m_pDevice;

public:
	WCHAR							wcsLoadedFile[MAX_PATH];	// Path of currently loaded file
	short							imageNo;
	CArray<scTexture*>				Textures;
	int								fmoduleNo;
	CArray<scFModule*>				FModules;
	int								aframesNo;
	CArray<scAFrame*>				AFrames;
	int								animationNo;
	CArray<scAnimation*>			Animations;

	CSpriteCollection(void);
	~CSpriteCollection(void);

public:
	// Loads the specified sprites collection.
	// \param wcsImageFolderOverride - images get searched here. If null they get loaded from the wcsFullPath folder
	OPRESULT						LoadSprites(WCHAR* wcsFullPath);
	// Releases currently loaded collection
	void							Release();
	inline bool						IsLoaded() const { return bIsLoaded; }

	int								GetAnimationIdxByName(const WCHAR* animName);
	int								GetAnimationIdxByName(const CHAR* animName);
	int								GetAnimationIdxByNameHash(const UINT32 animNameHash);

	// Returns BBOX set from editor
	inline RECTXYWH					GetAFrameBBox(int animIdx, int frameIdx) { return AFrames[Animations[animIdx]->aframesIdx[frameIdx]]->BBox; }
	inline UINT32					GetAFrameFlags(int animIdx, int frameIdx) { return AFrames[Animations[animIdx]->aframesIdx[frameIdx]]->flags; }
	inline UINT32					GetAnimFlags(int animIdx) { return Animations[animIdx]->flags; }
	// Returns real BBOX computed at load time
	inline RECTXYWH					GetAFrameBBox_real(int animIdx, int frameIdx) { return AFrames[Animations[animIdx]->aframesIdx[frameIdx]]->BBox_real; }
	RECTXYWH						GetModuleRect(int animIdx, int frameIdx, int moduleIdx);
	RECTLTRB_F						GetModuleRect_TexCoords(int animIdx, int frameIdx, int moduleIdx);
	SIZEWH							GetTextureSizeByAnim(int animIdx);
	scTexture*						GetTextureByAnim(int animIdx, int frameIdx, int moduleIdx);
	// Returns no of aframe hitpoints
	int								GetAFrameHitPointsCnt(int animIdx, int frameIdx);
	// Returns no of hitpoints where (flag & flagFilter != 0)
	int								GetAFrameHitPointsCntFlag(int animIdx, int frameIdx, DWORD flagFilter = 0xffffffff);
	// Get actual hitpoint data. Returns TRUE if success
	bool							GetAFrameHitPoint(int animIdx, int frameIdx, int pointIdx, PointXYZi *outvar);
	// Returns Nth hitpoint where (flag & flagFilter != 0)
	bool							GetAFrameHitPointFlag(int animIdx, int frameIdx, int pointIdx, DWORD flagFilter, PointXYZi *outvar);
	// Returns no of frames from an anim
	inline const int				GetAFramesCnt(int anmIdx) const { return Animations[anmIdx]->aframesNo; }
	
	inline bool IsLooping(int animIdx)
	{
		return ((Animations[animIdx]->flags & K_EDITOR_ANIMATION_FLAG_LOOPED) != 0);
	}

	OPRESULT OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL);
	OPRESULT OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL);
	OPRESULT OnLostDevice(void);
	OPRESULT OnDestroyDevice(void);
};
