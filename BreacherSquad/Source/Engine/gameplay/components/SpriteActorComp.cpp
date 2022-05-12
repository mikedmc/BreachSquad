#include "dxstdafx.h"
#include "SpriteActorComp.h"

CSpriteActorComponent::CSpriteActorComponent( CMultiSpriteLib* pSpriteLib )
{
	eAngle = EDIR6_S;
	nFlipDirX = 1;
	nAnimSet = 0;
	pLib = pSpriteLib;
	pSpriteLib = nullptr;
	nSkinIdx = 0;
}

CSpriteActorComponent::~CSpriteActorComponent()
{
	pLib = nullptr;
	pSprLib = nullptr;
}

void CSpriteActorComponent::Update(CActor& act, float dTime)
{
	// get necessary data from the actor
	Vec2 vSpeed = act.GetSpeedVec();
	MUVec2Norm(&vAim, &act.GetAimVec());
	eAngle = GetDir6FromVec( vAim );
	Vec2 vAimN( 0.0f, 0.0f );
	MUVec2Norm( &vAimN, &vAim );
	// see if he's walking backwards
	float fSpeedDot = MUVec2Dot( &vAim, &vSpeed );
	// flips a little later on the angle so we don't get jitter when looking N and S
	if ( nFlipDirX > 0 ) {
		if ( vAimN.x < -0.2f ) nFlipDirX = -1;
	}
	else {
		if ( vAimN.x > 0.2f ) nFlipDirX = 1;
	}
	sprite.pos = act.pos.xy_proj;
	// round up to eliminate visual artefacts
	UTMath::RoundVec2( sprite.pos );
	// change animation duration if walking back
	if ( fSpeedDot < 0.0f )
		sprite.SetAnimDirection( true );
	else
		sprite.SetAnimDirection( false );
	// now update the sprite
	sprite.Update( dTime );
}

void CSpriteActorComponent::Paint( CActor& act, ETexChannel eChannel /*= K_TEXCHAN_COLORMAP*/ )
{
	Vec2 scale( (float)nFlipDirX, 1.0f );
	// CUSTOM MASKED SPRITE PAINTER
	//sprite.Paint();
	CSpriteLib* pSpr = sprite.pSprCol;
	_ASSERT( sprite.animIdx < pSpr->Animations.Count() );
	_ASSERT( sprite.frameIdx < pSpr->Animations[ sprite.animIdx ]->aframesNo );

	int aframeIdx = pSpr->Animations[ sprite.animIdx ]->aframesIdx[ sprite.frameIdx ];
	for ( int ii = 0; ii < pSpr->AFrames[ aframeIdx ]->fmodulesNo; ii++ )
	{
		scFModule* mod = pSpr->FModules[ pSpr->AFrames[ aframeIdx ]->fmodulesIdx[ ii ] ];
		// shift module flag 2 bits to the right to erase the flipX and flipY flags
		// find layer index 1..N = (flags>>2), convert to layer mask by shifting to the left with index-1
		UINT32 flagmask = 1 << ((mod->flags >> 2) - 1);
		if ( FLAG_NONE( flagmask, dwLayersMask ) )
			continue;

		CSpr::s_pSP->Draw( mod->pImg->pTex,
			mod->texRect,
			mod->moduleRectOff,
			sprite.pos,
			sprite.color, 0.0f, scale );
	}
}

void CSpriteActorComponent::CacheAnimations( CActor& act )
{
	_ASSERT( pSprLib != nullptr );

	int nAnimsChanged = 0;
	for ( int anm = 0; anm < K_ACT_ANIMS_CNT; anm++ )
	{
		for ( int kk = 0; kk < K_SPCOMP_ANIM_MAX_SETS; kk++ )
		{
			for ( int ang = 0; ang < EDIR6S_CNT; ang++ )
			{
				CStringHash *animname = &act._template.arrAnims[ anm ].animNamesA[ kk ][ ang ];
				if ( animname->IsSet() )
				{
					int anmidx = pSprLib->GetAnimationIdxByName( animname->text );
					arrAnims[ anm ].animIdx[ kk ][ ang ] = anmidx;
					if ( anmidx < 0 )
					{
						ErrorBox( K_ERR_WARNING, L"SpriteAnimComponent::CacheAnimations: Could not find anim:%s in template: %s", animname->text, act._template.shID.text );
					}
				}
				else
					arrAnims[ anm ].animIdx[ kk ][ ang ] = -1;
			}
			// overwrite SW and NW with SE and NE as we use the same anims (if not specifically set in template)
			if ( arrAnims[ anm ].animIdx[ kk ][ EDIR6_NW ] < 0 )
				arrAnims[ anm ].animIdx[ kk ][ EDIR6_NW ] = arrAnims[ anm ].animIdx[ kk ][ EDIR6_NE ];
			if ( arrAnims[ anm ].animIdx[ kk ][ EDIR6_SW ] < 0 )
				arrAnims[ anm ].animIdx[ kk ][ EDIR6_SW ] = arrAnims[ anm ].animIdx[ kk ][ EDIR6_SE ];
		}
	}

	LOG_DBG( L"CActor::UpdateAnimationPointers: Updates %d animations", nAnimsChanged );
}

void CSpriteActorComponent::SetSkin( CActor& act, WCHAR* skinName, bool bShowPrimaryHand, bool bShowSecondaryHand )
{
	nSkinIdx = 0;
	CStringHash skinNamesh;
	if ( skinName == nullptr )
	{
		skinNamesh = act._template.arrSkins[ 0 ].name;
	}
	else
	{
		skinNamesh.Init( skinName );
	}

	for ( int kk = 0; kk < act._template.arrSkinsCnt; kk++ ) {
		if ( act._template.arrSkins[ kk ].name == skinNamesh )
		{
			// only save skin index if found
			nSkinIdx = kk;

			dwLayersMask = act._template.arrSkins[ kk ].layersVisMask;
			skinNamesh = act._template.arrSkins[ kk ].name;
			// show hands
			if ( bShowPrimaryHand )
				dwLayersMask |= act._template.arrSkins[ kk ].hand1Mask;
			else 
				dwLayersMask &= ~act._template.arrSkins[ kk ].hand1Mask;

			if ( bShowSecondaryHand )
				dwLayersMask |= act._template.arrSkins[ kk ].hand2Mask;
			else
				dwLayersMask &= ~act._template.arrSkins[ kk ].hand2Mask;

			return;
			
		}
	}
	ErrorBox( K_ERR_WARNING, L"Couldn't find skin named: %s", skinName );
}

void CSpriteActorComponent::SetSkinFlags( CActor& act, bool bShowPrimaryHand, bool bShowSecondaryHand )
{
	if ( nSkinIdx < 0 || nSkinIdx >= act._template.arrSkinsCnt )
	{
		ErrorBox( K_ERR_WARNING, L"SetSkinFlags:: Illegal skin index! Resetting to 0" );
		nSkinIdx = 0;
	}
	// show hands
	if ( bShowPrimaryHand )
		dwLayersMask |= act._template.arrSkins[ nSkinIdx ].hand1Mask;
	else
		dwLayersMask &= ~act._template.arrSkins[ nSkinIdx ].hand1Mask;

	if ( bShowSecondaryHand )
		dwLayersMask |= act._template.arrSkins[ nSkinIdx ].hand2Mask;
	else
		dwLayersMask &= ~act._template.arrSkins[ nSkinIdx ].hand2Mask;
}

bool CSpriteActorComponent::HasAnimation(EActorAnim nAnimType, int nSet)
{
	if ((nSet < 0) || (nSet >= K_SPCOMP_ANIM_MAX_SETS))
		return false;

	bool bHasIt = (this->arrAnims[nAnimType].animIdx[nSet] >= 0);
	return bHasIt;
}

OPRESULT CSpriteActorComponent::InitFromFile(CActor& act, WCHAR * Path)
{
	int nLibIdx = 0;
	if ( OP_FAILED( pLib->AddSprites( Path, nLibIdx ) ) )
	{
		return OPRESULT( K_OP_FAILED, K_SEVERITY_CRITICAL, L"CSpriteAnimComponent:: Could not load spriteLib: %s", Path );
	}

	pSprLib = pLib->GetLib( nLibIdx );
	_ASSERT( pSprLib != nullptr );
	// cache animations for fast access
	CacheAnimations( act );
	// save parent name
	shParentName = act._template.shID;

	// init sprites
	sprite.Init( pSprLib, 0 );
	//Set skin (first skin by default)
	SetSkin( act, nullptr, true, true);
	// set base animation
	SetAnimSet( 0 );
	SetAnimOnce( K_ACT_ANIM_IDLE );

	return K_OP_OK;
}

void CSpriteActorComponent::SetAnimSet(int newAnimSet)
{
	if (newAnimSet != nAnimSet)
	{
		nAnimSet = newAnimSet;
	}
}

void CSpriteActorComponent::SetAnimOnce( EActorAnim eAnim )
{
	_ASSERT( (eAnim >= K_ACT_ANIM_EMPTY) && (eAnim < K_ACT_ANIMS_CNT) );
	sprite.SetAnimOnce( arrAnims[ (int)eAnim ].animIdx[ nAnimSet ][ (int)eAngle ] );
}


Vec2 CSpriteActorComponent::GetMountPoint( bool bTwoHanded, int mountIndex /*= 0 */ )
{
	// decide flag for received settings
	EHitPtFlag pointflag = K_HITPTFLAG_MOUNT_TWOHANDED;
	if ( !bTwoHanded )
	{
		pointflag = (mountIndex == 0) ? K_HITPTFLAG_MOUNT_PRIMARY : K_HITPTFLAG_MOUNT_SECONDARY;
	}


	int anmidx = arrAnims[ K_ACT_ANIM_REFPOSE ].animIdx[ nAnimSet ][ (int)eAngle ];
	_ASSERT( anmidx >= 0 );

	Vec3i ptval( 0, 0, 0 );
	if ( pSprLib->GetAFrameHitPointFlag( anmidx, 0, 0, pointflag, &ptval ) )
	{
		// when graphics flip then we flip the mount points too
		return Vec2( (float)(ptval.x * nFlipDirX), (float)ptval.y );
	}
	return Vec2(0.0f, 0.0f);
}

EAnimEvent CSpriteActorComponent::GetAnimFrameEvent()
{
	// event flags from the editor
	const UINT32 FLAG_SOUND_EVENT = 0x1;
	const UINT32 FLAG_SHOOT_EVENT = 0x2;

	UINT32 aframef = sprite.GetCurFrameEvent();
	if ( aframef & FLAG_SOUND_EVENT )
		return FEVT_SOUND;
	if ( aframef & FLAG_SHOOT_EVENT)
		return FEVT_SHOOT;

	return FEVT_NONE;
}

