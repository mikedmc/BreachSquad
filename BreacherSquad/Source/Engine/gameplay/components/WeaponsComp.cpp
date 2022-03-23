#include "dxstdafx.h"
#include "WeaponsComp.h"

CWeaponsComponent::CWeaponsComponent( CSpriteLib* pSpriteLib )
{
	pSprLib = pSpriteLib;

	vAim = Vec2( 0.0f, 1.0f );
	vMount1 = g_Vec2Zero;
	vMount2 = g_Vec2Zero;

	sprite.Init( pSprLib, 0 );
}

CWeaponsComponent::~CWeaponsComponent()
{
	pSprLib = nullptr;
}

void CWeaponsComponent::Update( CActor& act, float dTime )
{
	//#TODO: ar putea sa-si ia singur din actor tot ce ii trebuie sa nu astepte actorul sa o updateze...
}

void CWeaponsComponent::Paint( CActor& act, ETexChannel eChannel /*= K_TEXCHAN_COLORMAP */ )
{
	sprite.pos = act.vHeart.xy_proj;
}
