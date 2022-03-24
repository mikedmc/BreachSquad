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

	CArray<CWeapon*>			arrWeapons;					// array of weapon instances in current loadout
	CWeapon*					weapon;						// currently equipped weapon

public:
	// receives pointer to global sprites library where resources are to be loaded
	CWeaponsComponent( CSpriteLib* pSpriteLib );
	~CWeaponsComponent();
	// Updates all weapons
	virtual void				Update( CActor& act, float dTime );
	// Paints weapons
	virtual void				Paint( CActor& act, ETexChannel eChannel = K_TEXCHAN_COLORMAP );
	// adds a weapon to the inventory and returns a pointer to it
	void						AddWeapon( CWeaponTemplate & wpnTemplate );
	// equips new weapon by index
	void						Equip( int weaponIdx );
	// sets the triggers for currently used weapon
	void						SetTriggerStates( bool bTriggerPushed, bool bReloadPushed );
	
	
	//addweapon(template)
	//equipWeapon

	// saves pointers to animations from actor template
	//void						CacheAnimations( CActor& act );
	// Loads all necessary data for specified actor
	//OPRESULT					InitFromFile( CActor& act, WCHAR * Path );
	//void						SetAnimOnce( EActorAnim eAnim );
};
