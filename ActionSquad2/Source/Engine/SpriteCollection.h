#pragma once

//----------------------------------------------------------	
//FORMAT FISER
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

//////////////////////////////////////////////////////////////////////////////////////////
//
//   Structurile de date pentru afisare
//
//////////////////////////////////////////////////////////////////////////////////////////
//MODULE - folosit doar la incarcare
class scModule 
{
public:
	UINT16 imgIdx;
	int X;
	int Y;
	int W;
	int H;
};
//FrameModule
typedef struct _nscFModule 
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
	//coordonatele in textura in functie de dimensiunea texturii
	D3DXVECTOR2 texUL;
	D3DXVECTOR2 texDR;
} scFModule;
//Frame - folosit doar la incarcare
class scFrame 
{
public:
	RECTXYWH BBox;
	int frame_hitPtsNo;
	int* frame_hitPtsPosXYF; //sunt scrise la rand: [frame][x1,y1,flag1,x2,y2...]
	UCHAR frame_fmodulesNo;  
	int* frame_fmodulesIdx;
};
//AnimationFrame
typedef struct _nscAFrame
{
	int dx;
	int dy;
	UINT32 flags;
	int duration;
	//date frame
	RECTXYWH BBox; //BBox incarcat din editor
	int PointsNo;
	int* PointsXYFlag; //sunt scrise la rand: [frame][x1,y1,flag1,x2,y2...]
	UCHAR fmodulesNo;  
	int* fmodulesIdx;
	
	RECTXYWH BBox_real; //bounding box real, calculat din module, folosit in paint de obicei
} scAFrame;

//Animation
#define K_SPRITECOLLECTION_MAX_ANIM_NAME 128
class scAnimation
{
public:
	int aframesNo;
	int* aframesIdx;// [A-frameid1, A-frameid2, ...]
	UINT32 flags;
	//salveaza numele animatiei si hash-ul ei
	CStringHash animName;
};
//Textures
class scTexture
{
public:
	LPDIRECT3DTEXTURE9	pTex;
	D3DXIMAGE_INFO		info;
	WCHAR				imagePath[MAX_PATH];
	//CTOR
	scTexture() :
	pTex(NULL)
	{
	}
};

/*!
 * \class CSpriteCollection
 *
 * \brief Loads a bsx sprites file including textures
 */
class CSpriteCollection
{
private:
	bool bIsLoaded;
public:
	WCHAR wcsLoadedFile[MAX_PATH];
// images ////////////// 
	CGrowableArray<scTexture*> Textures;
	short imageNo;
// fmodules /////////////
	int fmoduleNo;
	CGrowableArray<scFModule*> FModules;
// Animation frames//////////
	int aframesNo;
	CGrowableArray<scAFrame*> AFrames;
// animations ////
	int animationNo;
	CGrowableArray<scAnimation*> Animations;

	CSpriteCollection(void);
	~CSpriteCollection(void);

public:
	LPDIRECT3DDEVICE9	pDevice;
	// Loads the specified sprites collection.
	// \param wcsImageFolderOverride - images get searched here. If null they get loaded from the wcsFullPath folder
	HRESULT				LoadSprites(WCHAR* wcsFullPath);
	// Releases currently loaded collection
	void				Release();
	inline bool			IsLoaded() const { return bIsLoaded; }

	int					getAnimationIdxByName(const WCHAR* animName);
	int					getAnimationIdxByName(const CHAR* animName);
	int					getAnimationIdxByNameHash(const UINT32 animNameHash);

	//intoarce BBOX-ul frame-ului setat din editor
	FORCEINLINE RECTXYWH GetAFrameBBox(int animIdx, int frameIdx) { return AFrames[Animations[animIdx]->aframesIdx[frameIdx]]->BBox; }
	FORCEINLINE UINT32   GetAFrameFlag(int animIdx, int frameIdx) { return AFrames[Animations[animIdx]->aframesIdx[frameIdx]]->flags; }
	//intoarce bbox-ul frame-ului, calculat la load
	FORCEINLINE RECTXYWH GetAFrameBBox_real(int animIdx, int frameIdx) { return AFrames[Animations[animIdx]->aframesIdx[frameIdx]]->BBox_real; }
	FORCEINLINE RECTLTRB_F GetAFrameBBox_real_LTRB(int animIdx, int frameIdx) { return AFrames[Animations[animIdx]->aframesIdx[frameIdx]]->BBox_real; }
	//intoarce dreptunghiul din textura al unui modul
	RECTXYWH GetModuleRect(int animIdx, int frameIdx, int moduleIdx);
	RECTLTRB_F GetModuleRect_TexCoords(int animIdx, int frameIdx, int moduleIdx);
	SIZEWH GetTextureSizeByAnim(int animIdx);
	LPDIRECT3DTEXTURE9 GetTextureByAnim(int animIdx, int frameIdx, int moduleIdx);
	//intoarce nr de hitpoints (sau 0 daca e eroare sau nu are)
	int GetAFrameHitPointsCnt(int animIdx, int frameIdx);
	//cate sunt care respecta filtrul de flag (flag & flagFilter != 0)
	int GetAFrameHitPointsCntFlag(int animIdx, int frameIdx, DWORD flagFilter = 0xffffffff);
	//intoarce hitpoint cu indexul corespunzator din animatie, frame (filtreaza punctele dupa masca flagfilter)
	HRESULT GetAFrameHitPoint(int animIdx, int frameIdx, int pointIdx, POINTXYZ_INT *outvar);
	//intoarce al N-lea hitpoint al carui flag & flagFilter != 0
	HRESULT GetAFrameHitPointFlag(int animIdx, int frameIdx, int pointIdx, DWORD flagFilter, POINTXYZ_INT *outvar);
	//intoarce nr de frames dintr-o animatie
	const int	GetAFramesCnt(int anmIdx) const { return Animations[anmIdx]->aframesNo; }
	
	bool IsLooping(int animIdx);

	HRESULT OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnLostDevice(void);
	HRESULT OnDestroyDevice(void);
};
