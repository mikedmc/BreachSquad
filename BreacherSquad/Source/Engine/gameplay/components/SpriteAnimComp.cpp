#include "dxstdafx.h"
#include "SpriteAnimComp.h"

CSpriteAnimComponent::CSpriteAnimComponent( CMultiSpriteLib* pSpriteLib )
{
	eAngle = EDIR6_S;
	nAnimSet = 0;
	pLib = pSpriteLib;
	pSpriteLib = nullptr;
}

CSpriteAnimComponent::~CSpriteAnimComponent()
{
	pLib = nullptr;
	pSprLib = nullptr;
}

void CSpriteAnimComponent::Update(CActor& act, float dTime)
{
	sprite.pos = act.pos.xy_proj;
	// round up to eliminate viual artefacts
	sprite.pos.x = ROUND_FLOAT( sprite.pos.x );
	sprite.pos.y = ROUND_FLOAT( sprite.pos.y );

	sprite.Update( dTime );
	//#TODO: provide access to animation status and frame events (status through getter, events through callback)
}

void CSpriteAnimComponent::Paint( CActor& act, ETexChannel eChannel /*= K_TEXCHAN_COLORMAP*/ )
{
	Vec2 scale( 1.0f, 1.0f );
	if ( vAim.x < 0.0f )
		scale.x = -1.0f;
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
		if ( NIS_FLAG_ANY( flagmask, dwLayersMask ) )
			continue;

		CSpr::s_pSP->Draw( mod->pImg->pTex,
			mod->texRect,
			mod->moduleRectOff,
			sprite.pos,
			sprite.color, 0.0f, scale );
	}
}

void CSpriteAnimComponent::CacheAnimations( CActor& act )
{
	_ASSERT( pSprLib != nullptr );

	int nAnimsChanged = 0;
	for ( int anm = 0; anm < K_ACT_ANIMS_CNT; anm++ )
	{
		for ( int kk = 0; kk < K_SPCOMP_ANIM_MAX_SETS; kk++ )
		{
			for ( int ang = 0; ang < EDIR6S_CNT; ang++ )
			{
				CStringHash *animname = &act.actTemplate.arrAnims[ anm ].animNamesA[ kk ][ ang ];
				if ( animname->IsSet() )
				{
					int anmidx = pSprLib->GetAnimationIdxByName( animname->text );
					arrAnims[ anm ].animIdx[ kk ][ ang ] = anmidx;
					if ( anmidx < 0 )
					{
						ErrorBox( K_ERR_WARNING, L"SpriteAnimComponent::CacheAnimations: Could not find anim:%s in template: %s", animname->text, act.actTemplate.shID.text );
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

void CSpriteAnimComponent::SetSkin( CActor& act, WCHAR* skinName, bool bShowPrimaryHand, bool bShowSecondaryHand )
{
	CStringHash skinNamesh;
	if ( skinName == nullptr )
		skinNamesh = act.actTemplate.arrSkins[ 0 ].name;
	else
		skinNamesh.Init( skinName );

	for ( int kk = 0; kk < act.actTemplate.arrSkinsCnt; kk++ ) {
		if ( act.actTemplate.arrSkins[ kk ].name == skinNamesh )
		{
			dwLayersMask = act.actTemplate.arrSkins[ kk ].layersVisMask;
			skinNamesh = act.actTemplate.arrSkins[ kk ].name;
			// show hands
			if ( bShowPrimaryHand )
				dwLayersMask |= act.actTemplate.arrSkins[ kk ].hand1Mask;
			else 
				dwLayersMask &= ~act.actTemplate.arrSkins[ kk ].hand1Mask;

			if ( bShowSecondaryHand )
				dwLayersMask |= act.actTemplate.arrSkins[ kk ].hand2Mask;
			else
				dwLayersMask &= ~act.actTemplate.arrSkins[ kk ].hand2Mask;

			return;
			
		}
	}
	ErrorBox( K_ERR_WARNING, L"Couldn't find skin named: %s", skinName );
}

void CSpriteAnimComponent::SetLayersVisibilityMask( DWORD layersMask )
{
	dwLayersMask = layersMask;
}

bool CSpriteAnimComponent::HasAnimation(EActorAnim nAnimType, int nSet)
{
	if ((nSet < 0) || (nSet >= K_SPCOMP_ANIM_MAX_SETS))
		return false;

	bool bHasIt = (this->arrAnims[nAnimType].animIdx[nSet] >= 0);
	return bHasIt;
}

OPRESULT CSpriteAnimComponent::InitFromFile(CActor& act, WCHAR * Path)
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
	shParentName = act.actTemplate.shID;

	// init sprites
	sprite.Init( pSprLib, 0 );
	//Set skin (first skin by default)
	SetSkin( act, nullptr, true, true);
	// set base animation
	SetAnimSet( 0 );
	SetAnimOnce( K_ACT_ANIM_IDLE );

	return K_OP_OK;
}

void CSpriteAnimComponent::SetAnimSet(int newAnimSet)
{
	if (newAnimSet != nAnimSet)
	{
		nAnimSet = newAnimSet;
	}
}

void CSpriteAnimComponent::SetAnimOnce( EActorAnim eAnim )
{
	_ASSERT( (eAnim >= K_ACT_ANIM_EMPTY) && (eAnim < K_ACT_ANIMS_CNT) );
	sprite.SetAnimOnce( arrAnims[ (int)eAnim ].animIdx[ nAnimSet ][ (int)eAngle ] );
}


Vec2 CSpriteAnimComponent::GetMountPoint( EHitPtFlag pointflag )
{
	int anmidx = arrAnims[ K_ACT_ANIM_REFPOSE ].animIdx[ nAnimSet ][ (int)eAngle ];
	_ASSERT( anmidx >= 0 );

	Vec3i ptval( 0, 0, 0 );
	if ( pSprLib->GetAFrameHitPointFlag( anmidx, 0, 0, pointflag, &ptval ) )
	{
		return Vec2( (float)ptval.x, (float)ptval.y );
	}
	return Vec2(0.0f, 0.0f);
}

void CSpriteAnimComponent::SetAimVecLocal(Vec2 vLocalAim)
{
	vAim = vLocalAim;
	eAngle = GetDir6FromVec( vAim );
}

bool CSpriteAnimComponent::GetGunPosWorld( CActor& act, Vec2 &retVec )
{
	//#TODO: functia asta e inutila si ar trebui inlocuita cu ceva care iti da weapon mount (left, right, 2handed)
	// componenta de weapons ar trebui sa aiba paint separat


	retVec = sprite.pos;
	/*
	spine::Bone* bone = pSkeleton->arrBones[K_SD_BONE_GUN_MOUNT];
	if (bone)
	{
		retVec = { bone->getWorldX(), bone->getWorldY() };
		return true;
	}
	return false;
	*/
	return true;
}
