#include "dxstdafx.h"
#include "SpriteAnimComp.h"

CSpriteAnimComponent::CSpriteAnimComponent( CMultiSpriteLib* pSpriteLib )
{
	nAnimSet = 0;
	pLib = pSpriteLib;
	pSpriteLib = nullptr;
}

CSpriteAnimComponent::~CSpriteAnimComponent()
{
}

void CSpriteAnimComponent::Update(CActor& act, float dTime)
{
	sprite.pos = act.pos.xy_proj;
	sprite.Update( dTime );
	//#TODO: provide access to animation status and frame events (status through getter, events through callback)
}

void CSpriteAnimComponent::Paint(CActor& act, ETexChannel eChannel /*= K_TEXCHAN_COLORMAP*/)
{
	sprite.Paint();
}

void CSpriteAnimComponent::CacheAnimations(CActor& act)
{
	_ASSERT( pSprLib != nullptr );

	int nAnimsChanged = 0;
	for (int anm = 0; anm < K_ACT_ANIMS_CNT; anm++)
	{
		for (int kk = 0; kk < K_SPCOMP_ANIM_MAX_SETS; kk++)
		{
			for ( int ang = 0; ang < EANGS_CNT; ang++ )
			{
				CStringHashA *animname = &act.actTemplate.arrAnims[ anm ].animNamesA[ kk ][ ang ];
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
		}
	}

	LOG_DBG(L"CActor::UpdateAnimationPointers: Updates %d animations", nAnimsChanged);

}

void CSpriteAnimComponent::SetLayersVisibility( DWORD layersMask )
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
	dwLayersMask = act.actTemplate.arrSkins[ 0 ].layerVisibilityMask; //#0xffffffff maybe is better
	// set base animation
	SetAnimSet( 0 );
	SetAnimOnce( K_ACT_ANIM_IDLE, act.eAngle );

	return K_OP_OK;
}

void CSpriteAnimComponent::SetAnimSet(int newAnimSet)
{
	if (newAnimSet != nAnimSet)
	{
		nAnimSet = newAnimSet;
	}
}

void CSpriteAnimComponent::SetAnimOnce( EActorAnim eAnim, EAnimAngle eAngle )
{
	_ASSERT( (eAnim >= K_ACT_ANIM_EMPTY) && (eAnim < K_ACT_ANIMS_CNT) );
	sprite.SetAnimOnce( arrAnims[ (int)eAnim ].animIdx[ nAnimSet ][ (int)eAngle ] );
}


void CSpriteAnimComponent::SetAimVecLocal(Vec2 vLocalAim)
{
	//#TODO: vezi transformul asta ca sa muti din world space in skeleton space:
	//Vector2 ledgePointLocalSpace = skeletonAnimation.transform.InverseTransformPoint(ledgePoint); // your ledgePoint
	/*
	spine::Bone* b_aim = pSkeleton->arrBones[K_SD_BONE_AIM_IK];
	if (b_aim)
	{
		b_aim->setX(vLocalAim.x);
		b_aim->setY(vLocalAim.y);
	}
	*/
}

bool CSpriteAnimComponent::GetGunPosWorld(Vec2 &retVec)
{
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
