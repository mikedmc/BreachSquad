#include "dxstdafx.h"
#include "WeaponsComp.h"

CWeaponsComponent::CWeaponsComponent( CSpriteLib* pSpriteLib )
{
	pSprLib = pSpriteLib;

	weapon = nullptr;
	weaponIdx = -1;

	vAim = Vec2( 0.0f, 1.0f );
	vMount1 = g_Vec2Zero;
	vMount2 = g_Vec2Zero;

	sprite.Init( pSprLib, 0 );
}

CWeaponsComponent::~CWeaponsComponent()
{
	SAFE_DELETE_CArray( arrWeapons );

	pSprLib = nullptr;
}

void CWeaponsComponent::Update( CActor& act, float dTime )
{
}

void CWeaponsComponent::Paint( CActor& act, ETexChannel eChannel /*= K_TEXCHAN_COLORMAP */ )
{
	Vec2 vAim = act.GetAimVec();
	float fang = UTMath::GetVectorAngle( vAim );
	VecProj vpMount = act.GetWeaponMountWorld( false );

	UINT tex_flip_flags = (act.GetVisualFlipDirX() < 0) ? K_SPRFLAG_FLIP_Y : 0;

	sprite.rotation = -fang;
	sprite.pos = vpMount.xy_proj;
	sprite.PaintEx(tex_flip_flags);
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
