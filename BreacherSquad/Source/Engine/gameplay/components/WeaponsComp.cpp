#include "dxstdafx.h"
#include "WeaponsComp.h"

CWeaponsComponent::CWeaponsComponent( CSpriteLib* pSpriteLib )
{
	pSprLib = pSpriteLib;

	weapon = nullptr;
	weaponIdx = -1;

	vAim = Vec2( 0.0f, 1.0f );
	vMuzzleVec = Vec2(1.0f, 0.0f);

	sprite.Init( pSprLib, 0 );
}

CWeaponsComponent::~CWeaponsComponent()
{
	SAFE_DELETE_CArray( arrWeapons );

	pSprLib = nullptr;
}

void CWeaponsComponent::Update( CActor& act, float dTime )
{
	MUVec2Norm(&vAim, &act.GetAimVec());
	for ( int kk = 0; kk < arrWeapons.Count(); kk++ )
	{
		arrWeapons[ kk ]->modes[ K_WPNGRP_IDX_PRIMARY ].Update( dTime );
		arrWeapons[ kk ]->modes[ K_WPNGRP_IDX_ALTFIRE ].Update( dTime );
	}
}

void CWeaponsComponent::Paint( CActor& act, ETexChannel eChannel /*= K_TEXCHAN_COLORMAP */ )
{
	float fang = UTMath::GetVectorAngle( vAim );
	VecProj vpMount = act.GetWeaponMountWorld( false );
	// weapons need flipping when animation gets flipped to the left if we want to keep the unified angle of rotation
	sprite.scale.y = (act.GetVisualFlipDirX() < 0) ? -1.0f : 1.0f;
	sprite.rotation = -fang;
	sprite.pos = vpMount.xy_proj;
	sprite.Paint();
}

void CWeaponsComponent::AddWeapon( CActor& act, CWeaponTemplate * primary, CWeaponTemplate * altfire )
{
	CWeaponGroup* pWeapon = new CWeaponGroup();
	if ( primary )
	{
		pWeapon->modes[ K_WPNGRP_IDX_PRIMARY ].Init( &act, primary );
	}

	if ( altfire )
	{
		pWeapon->modes[ K_WPNGRP_IDX_ALTFIRE ].Init( &act, altfire );
	}
	// add to weapons inventory
	arrWeapons.Add( pWeapon );

	return;
}

const CWeapon* CWeaponsComponent::Equip( int wpnIdx )
{
	if ( weaponIdx == wpnIdx )
		return weapon;
	if ( wpnIdx < 0 || wpnIdx >= arrWeapons.Count() )
		return weapon;
	// equip primary mode
	weaponIdx = wpnIdx;
	weapon = &arrWeapons[ wpnIdx ]->modes[ K_WPNGRP_IDX_PRIMARY ];
	// initialize sprite
	sprite.SetAnim( weapon->_template.animIdx_shoot );
	sprite.StopAnimation();
	// save/init muzzle point (frame 0 from shooting animation)
	Vec3i ptval;
	pSprLib->GetAFrameHitPointFlag( weapon->_template.animIdx_shoot, 0, 0, K_HITPTFLAG_MUZZLE, &ptval );
	vMuzzleVec.x = (float)ptval.x; vMuzzleVec.y = (float)ptval.y;

	return weapon;
}

void CWeaponsComponent::SetTriggerStates( bool bTriggerPushed, bool bReloadPushed )
{
	if ( !weapon ) 
		return;
	weapon->SetTriggerStates( bTriggerPushed, bReloadPushed );
}

void CWeaponsComponent::StopReloading()
{
	if ( !weapon )
		return;
	weapon->StopReloading();
}

void CWeaponsComponent::JamWeapon()
{
	if ( !weapon )
		return;
	weapon->Jam();
}

