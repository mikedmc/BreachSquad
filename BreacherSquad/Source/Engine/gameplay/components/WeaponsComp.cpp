#include "dxstdafx.h"
#include "WeaponsComp.h"

CWeaponsComponent::CWeaponsComponent( CSpriteLib* pSpriteLib )
{
	pSprLib = pSpriteLib;

	eActiveSlot = K_WPNSLOT_NONE;
	bVisible = true;

	vAim = Vec2( 0.0f, 1.0f );
	vMuzzleVec = Vec2(1.0f, 0.0f);

	sprite.Init( pSprLib, 0 );
}

CWeaponsComponent::~CWeaponsComponent()
{
	for ( int kk = 0; kk < K_WPNSLOTS_CNT; kk++ )
	{
		arrWeapons[ kk ].SetTriggerStates( false, false );
	}

	pSprLib = nullptr;
}

void CWeaponsComponent::Update( CActor& act, float dTime )
{
	MUVec2Norm(&vAim, &act.GetAimVec());
	for ( int kk = 0; kk < K_WPNSLOTS_CNT; kk++ )
	{
		arrWeapons[ kk ].Update( dTime );
	}
}

void CWeaponsComponent::Paint( CActor& act, ETexChannel eChannel /*= K_TEXCHAN_COLORMAP */ )
{
	if ( !bVisible )
		return;

	float fang = UTMath::GetVectorAngle( vAim );
	VecProj vpMount = act.GetWeaponMountWorld( false );
	// weapons need flipping when animation gets flipped to the left if we want to keep the unified angle of rotation
	sprite.scale.y = (act.GetVisualFlipDirX() < 0) ? -1.0f : 1.0f;
	sprite.rotation = -fang;
	sprite.pos = vpMount.xy_proj;
	sprite.Paint();
}

void CWeaponsComponent::AddWeapon( CActor& act, CWeaponTemplate * primary, EWpnSlot slot )
{
	if ( primary )
	{
		arrWeapons[slot].Init( &act, primary );
	}

	return;
}

const CWeapon* CWeaponsComponent::Equip( EWpnSlot slot )
{
	if ( slot == eActiveSlot )
		return &arrWeapons[eActiveSlot];
	// equip primary mode
	eActiveSlot = slot;
	CWeapon* weapon = &arrWeapons[ eActiveSlot ];
	// initialize sprite
	sprite.SetAnim( weapon->_template.animIdx_shoot );
	sprite.Stop();
	// save/init muzzle point (frame 0 from shooting animation)
	Vec3i ptval;
	pSprLib->GetAFrameHitPointFlag( weapon->_template.animIdx_shoot, 0, 0, K_HITPTFLAG_MUZZLE, &ptval );
	vMuzzleVec.x = (float)ptval.x; vMuzzleVec.y = (float)ptval.y;

	return weapon;
}

void CWeaponsComponent::SetTriggerStates( bool bTriggerPushed, bool bReloadPushed )
{
	arrWeapons[eActiveSlot].SetTriggerStates( bTriggerPushed, bReloadPushed );
}

void CWeaponsComponent::StopReloading()
{
	arrWeapons[eActiveSlot].StopReloading();
}

void CWeaponsComponent::JamWeapon()
{
	arrWeapons[eActiveSlot].Jam();
}

