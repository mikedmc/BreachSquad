#include "dxstdafx.h"
#include "WeaponsComp.h"

CWeaponsComponent::CWeaponsComponent( CSpriteLib* pSpriteLib )
{
	pSprLib = pSpriteLib;

	eActiveSlot = K_WPNSLOT_NONE;
	bVisible = true;

	vAim = Vec3( 0.0f, 1.0f, 0.0f );
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
	Vec2 v_aim;
	MUVec2Norm(&v_aim, &act.GetAimVec());
	vAim = Vec3( v_aim.x, v_aim.y, 0.0f );
	for ( int kk = 0; kk < K_WPNSLOTS_CNT; kk++ )
	{
		arrWeapons[ kk ].Update( dTime );
	}
}

void CWeaponsComponent::Paint( CActor& act, ETexChannel eChannel /*= K_TEXCHAN_COLORMAP */ )
{
	if ( !bVisible )
		return;

	float fang = UTMath::GetVectorAngle( Vec3XY(vAim) );
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

	///--- stop using old weapon
	CWeapon* old_weapon = &arrWeapons[ eActiveSlot ];
	old_weapon->SetTriggerStates( false, false );
	old_weapon->StopShootingCycle();
	old_weapon->StopReloading();

	///--- equip primary mode
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

bool CWeaponsComponent::CanShoot( EWpnSlot slot )
{
	EWpnStatus status = arrWeapons[ slot ].status;
	if ( status == K_WPN_STATUS_UNKNOWN )
		return false;
	if ( status > K_WPN_STATUSCHECKPOINT_CANNOT_SHOOT )
		return false;

	// by default return true
	return true;
}


