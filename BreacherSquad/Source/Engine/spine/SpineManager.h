#pragma once

#include "SpineTexLoader.h"
#include "SpineDefines.h"
#include "SpineEventsListener.h"

#define K_SM_DEFAULT_MIX_DURATION 0.2f

// Loads all necessary spine atlasses and textures
class CSpineManager {
public:
	struct CAtlasContainer {
		CStringHash						shFilename;
		Atlas*							pAtlas;
	};

	// Stores all data for a skeleton template
	class CSkeletonTemplate {
	public:
		CStringHash						shID;						// filename works as ID

		CStringHash						shSkeletonFilename;			// name of Spine Skeleton file loaded (and main identifier of this object)
		CStringHash						shAtlasFilename;			// name of Spine Atlas file that the template skeleton is linked to
		spine::SkeletonData*			m_skeletonData;				// Pointer to Spine skeleton data
		spine::AnimationStateData*		m_animationStateData;		// Pointer to Spine anim state data

		std::string						arrBonesNames[K_SD_BONES_CNT];	//holds the Spine names for special bones
		std::string						arrEventsNames[K_SD_EVENTS_CNT];//holds the Spine names for events
		std::string						arrSlotsNames[K_SD_SLOTS_CNT];	//holds the Spine names for events

		CSkeletonTemplate();
		// SAFE BUT SLOW: Tells you if the skeleton contains the specified animation
		spine::Animation*				GetAnimation(const char * strAnimName);
	};
	
	// Helper structure for skeleton instances
	class CSkeletonInstance {
	public:
		UINT32							flagsLayer;				// Skeleton layer flags (if we want to split them into layers)
		bool							bVisible;				// Skeleton visible flag
		bool							bEnabled;				// Skeleton update enable flag

		Skeleton*						skel;					// Skeleton instance
		AnimationState*					anim;					// Animation state for current skeleton

		Bone*							arrBones[K_SD_BONES_CNT];	// Pointers to special bones
		Slot*							arrSlots[K_SD_SLOTS_CNT];	// Pointers to slots
		
		CSkeletonInstance();
	};

public:
	LPDIRECT3DDEVICE9					m_pDevice;				// pointer to GFX device

	CBufferedSpinePainter				m_Painter;				// buffered painter for skeleton meshes
	CSpineTexLoader						m_TexMgr;				// custom texture manager for spine

	CGrowableArray<CAtlasContainer*>	arrAtlasses;			// atlasses array

	CGrowableArray<CSkeletonTemplate*>	arrSkeletonTemplates;	// holds all data for skeletons templates
	CGrowableArray<CSkeletonInstance*>	arrSkeletonInstances;	// holds all skeleton/animState instance pairs

	CSpineManager();
	~CSpineManager();

	// Loads a Spine atlas and returns a pointer to it or null if it fails
	// wcsFolder + wcsFile should be the final path (2 path vars needed because Spine doesn't know WCHAR)
	CAtlasContainer*					LoadAtlas(WCHAR * wcsFolder, WCHAR * wcsFile);
	// Gets an atlas by it's path (ID). Returns null if not found.
	CAtlasContainer*					GetAtlasByPath(WCHAR * wcsPath);

	
	// Loads a skeleton template, adds it to the templates collection and returns a pointer to it
	CSkeletonTemplate*					LoadSkeletonTemplateXML(WCHAR* wcsXMLpath);
	// Gets a skeleton template by it's path (ID). Returns null if not found.
	CSkeletonTemplate*					GetSkeletonTemplateByPath(WCHAR* wcsXMLpath);


	// Returns a skeleton instance combo (skeleton and animState)
	CSkeletonInstance*					GetSkeletonInstance(CSkeletonTemplate* skelTemplate);
	// Sets an event listener for Spine events (anim end, anim start, custom events, etc)
	void								SetListenForEvents(CSkeletonInstance* skelInst, bool bListen);

	
	// Updates animation states (must be called before moving bones programatically)
	void								UpdateAnimationStates(float dTime, float fTimeLine);
	// Updates all necessary stuff for rendering (and IK, bust be called right before paint)
	void								Update(float dTime, float fTimeLine);
	// Paints all visible skeletons using a specified texture variation (normals or other surfaces)
	void								Paint(ETexChannel eChannel = K_TEXCHAN_COLORMAP);
	// Sends a skeleton to BufferedTexPainter for drawing (only buffers it, usually best done on Update)
	void								BatchSkeleton(Skeleton* skel);
	// Releases everything
	void								Release();
private:
	// Loads the skeleton data and initializes animation state data (on a specified Atlas file into a specified template)
	HRESULT								LoadSkeletonData(WCHAR * wcsPath, Atlas* pTargetAtlas, CSkeletonTemplate* pDestTemplate);
	HRESULT								LoadSkeletonDataJSON(WCHAR * wcsPath, Atlas* pTargetAtlas, CSkeletonTemplate* pDestTemplate);

public:
	//--- system framework ---
	HRESULT OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnLostDevice(void);
	HRESULT OnDestroyDevice(void);
};
