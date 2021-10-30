#pragma once

#include "ComponentInterfaces.h"

// number of maximum animations for one action (multiple idle animations or multiple death animations etc)
#define K_SACOMP_ANIM_MAX_SETS	2
// max no of animation tracks for the actors
#define K_SACOMP_MAX_ANIM_TRACKS 2

// Animation pointers buffer - keeps animation in sync with actor template declared animations
struct CAnimPtr {
	CStringHashA		animNamesA[K_SACOMP_ANIM_MAX_SETS];
	spine::Animation*	pAnim[K_SACOMP_ANIM_MAX_SETS];
	bool				bLooping;

	CAnimPtr()
	{
		bLooping = true;
		for (int kk = 0; kk < K_SACOMP_ANIM_MAX_SETS; kk++)
		{
			pAnim[kk] = null;
			animNamesA[kk].Reset();
		}
	}
};

// should be derived from a more complex interface with all the methods below (IRenderItemComponent)
// this way we can have all kinds of rendering spine, sprite, etc
class CSpineAnimComponent : public IBaseComponent
{
private:
	ESpineAnim					eLastAnim[K_SACOMP_MAX_ANIM_TRACKS];	// Last anim set on one track so we don't set it again if already set (for mixing)
	CStringHash					shParentName;							// Name of owner (optional)
	int							nAnimSet;								// Selected ainmation set

public:
	CSpineManager::CSkeletonTemplate*		pSkelTemplate;	// Pointer to the skeleton template (don't deallocate, managed)
	CSpineManager::CSkeletonInstance*		pSkeleton;		// Pointer to the skeleton instance (don't deallocate, managed)
	CAnimPtr								arrAnimsPtr[K_SD_ANIMS_CNT]; // Direct pointer structure to animations declared in actor template (rarely updated)

public:
	CSpineAnimComponent();
	~CSpineAnimComponent();
	// Updates all skeleton positions and processes needed animations
	virtual void				Update(CActor& act, float dTime);
	// Paints skeleton
	virtual void				Paint(CActor& act, ETexChannel eChannel = K_TEXCHAN_COLORMAP);
	// takes all position data from the actor and updates skeleton
	void						UpdateTransform(CActor& act);
	// saves pointers to animations from actor template
	void						SaveAnimationPointers(CActor& act);
	bool						SetSkin(const char * strSkinName);
	// Checks to see if specified animation is present (we should never have it on set 1 and not have it on set 0)
	bool						HasAnimation(ESpineAnim nAnimType, int nSet = 0);
	// Loads all necessary data for specified actor
	OPRESULT					InitFromFile(CActor& act, WCHAR * Path);

	void						SetAnimSet(int newAnimSet);
	FORCEINLINE int				GetAnimSet() const { return nAnimSet; }

	spine::TrackEntry*			SetAnimOnce(int nTrack, ESpineAnim eAnim);
	spine::TrackEntry*			AddAnimOnce(int nTrack, ESpineAnim eAnim, float fMixTime = K_SM_DEFAULT_MIX_DURATION, float fDelay = 0.0f);

	void						SetAimVecLocal(Vec2 vLocalAim);
	bool						GetGunPosWorld(Vec2 &retVec);
};
