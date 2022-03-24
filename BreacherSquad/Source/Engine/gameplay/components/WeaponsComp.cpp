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
	//#TODO: ar putea sa-si ia singur din actor tot ce ii trebuie sa nu astepte actorul sa o updateze...
}

void CWeaponsComponent::Paint( CActor& act, ETexChannel eChannel /*= K_TEXCHAN_COLORMAP */ )
{
	sprite.pos = act.vHeart.xy_proj;
	sprite.Paint();
}

void CWeaponsComponent::AddWeapon( CWeaponTemplate * primary, CWeaponTemplate * altfire )
{
	CWeaponGroup* pWeapon = new CWeaponGroup();
	if ( primary )
	{
		pWeapon->modes[ K_WPNGRP_IDX_PRIMARY ].Init( primary );
	}

	if ( altfire )
	{
		pWeapon->modes[ K_WPNGRP_IDX_ALTFIRE ].Init( altfire );
	}

	return;
}

void CWeaponsComponent::Equip( int wpnIdx )
{
	if ( weaponIdx == wpnIdx )
		return;
	if ( weaponIdx < 0 || weaponIdx >= arrWeapons.Count() )
		return;
	// equip primary mode
	weapon = &arrWeapons[ wpnIdx ]->modes[ K_WPNGRP_IDX_PRIMARY ];
	weaponIdx = wpnIdx;
}
