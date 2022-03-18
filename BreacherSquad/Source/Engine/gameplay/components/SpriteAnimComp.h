#pragma once

#include "ComponentInterfaces.h"

// number of maximum animations for one action (multiple idle animations or multiple death animations etc)
// must be the same as K_ACT_ANIM_MAX_SETS
#define K_SPCOMP_ANIM_MAX_SETS	2

struct CAnimData {
	// keeps animation indexes for each animation [set][angle]
	int				animIdx[ K_SPCOMP_ANIM_MAX_SETS ][ EDIR6S_CNT ];

	CAnimData()
	{
		for ( int kk = 0; kk < K_SPCOMP_ANIM_MAX_SETS; kk++ )
			for ( int ang = 0; ang < EDIR6S_CNT; ang++ )
				animIdx[ kk ][ ang ] = -1;
	}
};

//#TODO: should be named SpriteActorComponent
class CSpriteAnimComponent : public IBaseAnimComponent
{
private:
	CMultiSpriteLib*			pLib;									// pointer to sprites library
	CSpriteLib*					pSprLib;								// pointer to sprite library from pLib

	CStringHash					shParentName;							// Name of owner (optional)
	int							nAnimSet;								// Selected ainmation set
	CSpr						sprite;									// sprite used for frames keeping. Don't use it for painting directly (does not support layers)
	UINT32						dwLayersMask;							// least important byte is layer id 1 from FModule flags. Use this to hise or to show parts depending on skin
	CStringHashA				shSkin;									// currently set skin (useless?)
	Vec2						vAim;									// aim vector

public:
	CAnimData					arrAnims[ K_ACT_ANIMS_CNT ];			// data about animations

public:
	// receives pointer to global sprites library where resources are to be loaded
	CSpriteAnimComponent( CMultiSpriteLib* pSpriteLib );
	~CSpriteAnimComponent();
	// Updates all skeleton positions and processes needed animations
	virtual void				Update( CActor& act, float dTime );
	// Paints skeleton
	virtual void				Paint( CActor& act, ETexChannel eChannel = K_TEXCHAN_COLORMAP );
	// saves pointers to animations from actor template
	void						CacheAnimations( CActor& act );
	// sets a skin and hides or shows the hands. If skin isn't found it defaults to first skin in template.
	// use skinName=nullptr to set the default first skin
	void						SetSkin( CActor& act, WCHAR* skinName, bool bShowPrimaryHand = false, bool bShowSecondaryHand = true );
	// sets the internal layer visibility flags (see dwLayersMask)
	void						SetLayersVisibilityMask( DWORD layersMask );
	// Checks to see if specified animation is present (we should never have it on set 1 and not have it on set 0)
	bool						HasAnimation( EActorAnim nAnimType, int nSet = 0 );
	// Loads all necessary data for specified actor
	OPRESULT					InitFromFile( CActor& act, WCHAR * Path );

	void						SetAnimSet( int newAnimSet );
	inline int					GetAnimSet() const { return nAnimSet; }

	void						SetAnimOnce( EActorAnim eAnim, EDir6 eAngle );

	void						SetAimVecLocal( Vec2 vLocalAim );
	bool						GetGunPosWorld( Vec2 &retVec );
};
