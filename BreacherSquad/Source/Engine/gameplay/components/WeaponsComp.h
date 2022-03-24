#pragma once

#include "ComponentInterfaces.h"


// index in weapon group
#define K_WPNGRP_IDX_PRIMARY 0
#define K_WPNGRP_IDX_ALTFIRE 1
// weapons with 2 firing modes used by actors
class CWeaponGroup
{
public:
	CWeapon modes[ 2 ];

	//#TODO: add stuff like switching primary with alt fire and so on
};


class CWeaponsComponent : public IBaseAnimComponent
{
private:
	CSpriteLib*					pSprLib;					// pointer to sprite library from pLib
	CStringHash					shParentName;				// Name of owner (optional)

	CSpr						sprite;						// Weapon sprite
	Vec2						vAim;						// aiming direction
	Vec2						vMount1;					// weapon mounting origin
	Vec2						vMount2;					// weapon mounting origin secondary weapon

	CArray<CWeaponGroup*>		arrWeapons;					// array of weapon instances in current loadout
	CWeapon*					weapon;						// currently equipped weapon mode
	int							weaponIdx;					// currently weapon goup

public:
	// receives pointer to global sprites library where resources are to be loaded
	CWeaponsComponent( CSpriteLib* pSpriteLib );
	~CWeaponsComponent();
	// Updates all weapons
	virtual void				Update( CActor& act, float dTime );
	// Paints weapons
	virtual void				Paint( CActor& act, ETexChannel eChannel = K_TEXCHAN_COLORMAP );
	// adds a weapon to the inventory 
	void						AddWeapon( CWeaponTemplate * primary, CWeaponTemplate * altfire );
	// equips new weapon by index
	void						Equip( int weaponIdx );
	// sets the triggers for currently used weapon
	void						SetTriggerStates( bool bTriggerPushed, bool bReloadPushed );
	// returns current weapon
	const inline CWeapon*		GetCurrentWeapon() const { return weapon; };
	
	//addweapon(template)
	//equipWeapon

	// saves pointers to animations from actor template
	//void						CacheAnimations( CActor& act );
	// Loads all necessary data for specified actor
	//OPRESULT					InitFromFile( CActor& act, WCHAR * Path );
	//void						SetAnimOnce( EActorAnim eAnim );
};
