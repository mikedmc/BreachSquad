#pragma once

#include "ComponentInterfaces.h"

class CWeaponsComponent : public IBaseAnimComponent
{
private:
	CSpriteLib*					pSprLib;					// pointer to sprite library from pLib
	CStringHash					shParentName;				// Name of owner (optional)

	CSpr						sprite;						// Weapon sprite
	Vec2						vAim;						// aiming direction
	Vec2						vMount1;					// weapon mounting origin
	Vec2						vMount2;					// weapon mounting origin secondary weapon

public:
	// receives pointer to global sprites library where resources are to be loaded
	CWeaponsComponent( CSpriteLib* pSpriteLib );
	~CWeaponsComponent();
	// Updates all weapons
	virtual void				Update( CActor& act, float dTime );
	// Paints weapons
	virtual void				Paint( CActor& act, ETexChannel eChannel = K_TEXCHAN_COLORMAP );
	// saves pointers to animations from actor template
	//void						CacheAnimations( CActor& act );
	// Loads all necessary data for specified actor
	//OPRESULT					InitFromFile( CActor& act, WCHAR * Path );
	//void						SetAnimOnce( EActorAnim eAnim );
};
