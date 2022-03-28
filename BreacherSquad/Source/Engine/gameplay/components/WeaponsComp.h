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
public:
	// hitpoint flags for frames
	enum EHitPtFlag {
		K_HITPTFLAG_MUZZLE = 1,								// frame hitpoint flag for muzzle
	};

private:
	CSpriteLib*					pSprLib;					// pointer to sprite library from pLib
	CStringHash					shParentName;				// Name of owner (optional)

	CSpr						sprite;						// Weapon sprite
	Vec2						vAim;						// aiming direction
	Vec2						vMuzzleVec;					// weapon muzzle vector (from weapon origin, local space)

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
	// Adds a weapon to the inventory 
	void						AddWeapon( CActor& act, CWeaponTemplate * primary, CWeaponTemplate * altfire );
	// Equips new weapon by index and returns pointer to weapon
	const CWeapon*				Equip( int weaponIdx );
	// sets the triggers for currently used weapon
	void						SetTriggerStates( bool bTriggerPushed, bool bReloadPushed );
	// Returns current weapon
	inline CWeapon*				GetCurrentWeapon() { return weapon; };
	// Stops reloading current weapon		
	void						StopReloading();
	// Jams current weapon
	void						JamWeapon();
	// Returns the local offset of the gun muzzle from the gun origin
	inline Vec2					GetWeaponMuzzlePoint() const { return vMuzzleVec; }
	// returns weapons aim vector
	inline Vec2					GetWeaponAimVec() const { return vAim; }
	//void						SetAnimOnce( EActorAnim eAnim );
};
