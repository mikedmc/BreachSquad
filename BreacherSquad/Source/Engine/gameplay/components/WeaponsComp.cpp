#include "dxstdafx.h"
#include "WeaponsComp.h"

CWeaponsComponent::CWeaponsComponent( CSpriteLib* pSpriteLib )
{
	pSprLib = pSpriteLib;

	weapon = nullptr;
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

void CWeaponsComponent::AddWeapon( CWeaponTemplate & wpnTemplate )
{
	CWeapon* pWeapon = new CWeapon();
	// set owner
	//#TODO: is parent needed??
	//pWeapon->pOwner = &act;
	// copy data to local weapon template as we need it later on
	pWeapon->WeaponTemplate = wpnTemplate;
	// signal valid weapon
	pWeapon->status = K_LVL_WPN_STATUS_READY;
	pWeapon->ammoLeft = pWeapon->WeaponTemplate.nClipSize + pWeapon->WeaponTemplate.nBulletChamberSize;
	// make sure infinite ammo is infinite
	if ( pWeapon->WeaponTemplate.nClipSize < 0 )
		pWeapon->ammoLeft = -1;

	return;
}

void CWeaponsComponent::Equip( int weaponIdx )
{

}
