#pragma once

#include "ComponentInterfaces.h"

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
	Vec3						vAim;						// aiming direction
	Vec2						vMuzzleVec;					// weapon muzzle vector (from weapon origin, local space)
	bool						bVisible;					// if not visible then it doesn't render

	CWeapon						arrWeapons[K_WPNSLOTS_CNT];	// array of weapon instances in current loadout
	EWpnSlot					eActiveSlot;				// currently active weapon slot

public:
	// receives pointer to global sprites library where resources are to be loaded
	CWeaponsComponent( CSpriteLib* pSpriteLib );
	~CWeaponsComponent();
	// Updates all weapons
	virtual void				Update( CActor& act, float dTime );
	// Paints weapons
	virtual void				Paint( CActor& act, ETexChannel eChannel = K_TEXCHAN_COLORMAP );
	// Adds a weapon to the inventory 
	void						AddWeapon( CActor& act, CWeaponTemplate * primary, EWpnSlot slot );
	// returns weapon on slot
	inline CWeapon*				GetWeapon( EWpnSlot slot ) { return &arrWeapons[ eActiveSlot ]; }
	// Equips new weapon by index and returns pointer to weapon
	const CWeapon*				Equip( EWpnSlot slot );
	// sets the triggers for currently used weapon
	void						SetTriggerStates( bool bTriggerPushed, bool bReloadPushed );
	// Returns current weapon
	inline CWeapon*				GetCurWeapon() { return &arrWeapons[eActiveSlot]; };
	// Returns current/active weapon slot
	inline EWpnSlot				GetCurWeaponSlot() { return eActiveSlot; }
	// Stops reloading current weapon		
	void						StopReloading();
	// Returns the local offset of the gun muzzle from the gun origin
	inline Vec2					GetWeaponMuzzlePoint() const { return vMuzzleVec; }
	// returns weapons aim vector
	inline Vec3					GetWeaponAimVec() const { return vAim; }
	// sets component visibility
	inline void					SetVisible( bool visible ) { bVisible = visible; }
	// is slot ready to shoot? returns true even if already shooting.
	bool						CanShoot( EWpnSlot slot );
};
