#pragma once

// declare classes
class CActor;

///--------------------------------------------------------------------------
///--- weapon class ---
///--------------------------------------------------------------------------

class CWeaponTemplate
{
public:
	CStringHash		name;
	//CStringHash		filePath;					// path to bsx file to load (each weapon has it's own image)
	//generic data:
	CStringHash		shAltFireTemplate;			// weapon template for alt fire mode of the weapon
	CStringHash		shTemplateOverwrite;		// name of themplate that the weapon overwrites over the character template
	float			fSpeedPenaltyPercent;		// what percent of total movement speed is taken by this weapon
	CStringHash		shScript_OnFire;			// called when shooting a weapon. If not set it just shoots the weapon.
	CStringHash		shScript_OnFireALT;			// called when shooting ALT mode for weapon. If not set it just shoots the ALT weapon.
	CStringHash		shScript_OnEmpty;			// called when weapon is empty
	int				animIdx_shoot;
	int				animIdx_reload;

	int				nHUD_AnimIdx;			//animatie INGAME HUD pentru grafica armei (vezi detalii frames in CCustomInterfaceIGM) sau -1 pt empty
	int				nHUD_AnimIdxALT;		//animatie INGAME HUD when shown as ALT weapon (painted just as small icon)
	int				nMuzzleFlashAnim;		//animatie muzzle flash sau -1 pt empty

	//fire modes data:
	CBulletTemplate	bulletTemplate;					// bullet data for current weapon
	int				nBulletsPerShot;				// number of bullets per shot

	float			fSpreadFOV;							//spread default arma
	float			fAimErrorMaxFOV;					//aiming error - FOV in radiani 
	float			fAimErrorAddPerShot;				//ce eroare de AIM se adauga dupa fiecare foc
	float			fAimErrorMulPerShot;				//factor multiplicare eroare FOV ca sa nu mai creasca liniar
	float			fAimErrorCooldownPerSecond;			//cooldown AIM error in functie de timp
	float			fAimFOV;							// angle in rad. can the weapon be aimed? (default 0.0f means only aim vec -1,0 or 1,0)

	int				nClipSize;						// -1 infinite clip
	
	int				nReloadUnitSize;				// bullets loaded at once (0 if it can't be reloaded)
	float			fReloadTimePerUnit;				// time it takes to load a bullets unit

	float			fCooldownT;						// fire rate  - time between bullets
	float			fChargeUpT;						// charge up duration (before shooting) - can't be reset
	float			fWindDownT;						// wind down duration (after shooting) - can't be reset
	bool			bResetFireRateOnTriggerUp;		// resets fire rate when you release the button (fast tap = fast shoot)
	bool			bUsesMainWeaponAmmo;			// for alt fire weapons: aimed shot uses ammo from main weapon

	int				nBulletChamberSize;				// bullet chamber (0 or 1) - adds to bullets left.
	bool			bCanShootFromCrouch;
	bool			bCanShootFromCover;
	int				nDropShellFrame;				// frame number of shell from SHELLS animation (-1 - no shell)
	int				nBurstSize;						// bullets per burst (0 = full automatic)
	float			fBurstCooldown;					// time between bursts (cooldown is between bullets)
	float			fShooterSpeedSlowingPercent;	// procentul cu care scade viteza tragatorului daca se misca in timp ce trage

	float			fMuzzleLightSize;				// size of lighting effect when shooting
	bool			bHasLaserSight;			
	float			fSoundRadius;			// how far can the weapon be heared (in pixels)

	Vec2			vMountOffset;			// vector showing the offset from the mount to the gun rotating position
	bool			bTwoHanded;				// can be used with a single hand
	bool			bDualWielding;			// only for single handed weapons. if true it gets doubled in the second mount position.

	//sound indices to play
	int		sndidxShoot, sndidxReload, sndidxEmpty;
	//alternatives
	int		sndidxShoot2, sndidxReload2, sndidxEmpty2;
	
	EActorSoundVerse	sndActorVerse;		// bullets can trigger a sound action (verse) on the actor (grenades trigger "FIRE IN THE HOLE" verse)

	CWeaponTemplate() :
		fSpeedPenaltyPercent(0.0f),
		//other data
		nBulletsPerShot(5), fWindDownT(0.0f), fChargeUpT(0.0f), fCooldownT(0.0f), 
		fMuzzleLightSize(0.0f), nClipSize(10),
		fReloadTimePerUnit(1.0f), nReloadUnitSize(1),
		fAimErrorMaxFOV(0.0f), fAimErrorAddPerShot(0.0f), fAimErrorCooldownPerSecond(1.0f), fSpreadFOV(0.0f), fAimErrorMulPerShot(1.0f),
		bCanShootFromCrouch(true), bCanShootFromCover(false), nDropShellFrame(-1),
		nBurstSize(0), bResetFireRateOnTriggerUp(false), bUsesMainWeaponAmmo(false), fShooterSpeedSlowingPercent(0.0f), nBulletChamberSize(0),
		sndidxShoot(-1), sndidxReload(-1), sndidxEmpty(-1), sndidxShoot2(-1), sndidxReload2(-1), sndidxEmpty2(-1),
		fSoundRadius(128.0f), fBurstCooldown(0.0f), bHasLaserSight(false), fAimFOV( 0.0f ),
		nHUD_AnimIdx(-1), nHUD_AnimIdxALT(-1), nMuzzleFlashAnim(-1), animIdx_shoot(-1), animIdx_reload(-1),
		sndActorVerse(K_LVL_ACT_VERSE_EMPTY)
	{
		bTwoHanded = true;
		bDualWielding = false;
		vMountOffset = Vec2( 0.0f, 0.0f );
	}
};

// States logic:
// |-------------|JUST_SHOT|------------|----------------|READY|
//   charging up			  wind down	     cool down
enum EWpnStatus {
	K_WPN_STATUS_UNKNOWN = -1,	//not initialized!
	// NONSTATE	checkpoint for CAN SHOOT states
	K_WPN_STATUSCHECKPOINT_CAN_SHOOT = 0,
	// the following states mean the weapon can shoot
	K_WPN_STATUS_READY,		// ready to shoot
	K_WPN_STATUS_CHARGING_UP,	// optional. some weapons have a period of aiming before shooting
	K_WPN_STATUS_JUST_SHOT,		// just spawned a bullet, started playing animation
	K_WPN_STATUS_WINDING_DOWN,	// optional. some weapons need a period of winding down after shooting, before starting to cool down
	K_WPN_STATUS_COOLING,		// waiting between shots (added to charding up and winding down)
	// NONSTATE checkpoint for CANNOT SHOOT states
	K_WPN_STATUSCHECKPOINT_CANNOT_SHOOT,
	// the following states return canShoot false (signals that trigger should be released and action taken)
	K_WPN_STATUS_RELOADING,		// reloading
	K_WPN_STATUS_BURST_END,		// burst ended, we must wait cooldown (release trigger)
	K_WPN_STATUS_BURST_COOLDOWN,// burst cooling down
	K_WPN_STATUS_NO_AMMO,		// no more ammo
};

// weapon fire mode (usually we have 2 on a real weapon)
class CWeapon
{
public:
	CWeaponTemplate _template;
public:
	EWpnStatus		status;					// weapon state: ready, reloading
	EWpnStatus		statusOld;				// old status so we know when it changes
	//consumabile
	float	fAimErrorFOV;						// FOV-ul curent de eroare aim
	int		nBurstBulletsShot;					// cate gloante s-au tras din burst 
	int		ammoLeft;							// -1 pt nr infinit de gloante
	float	fStateT;							// state timer
	bool	bPaintLaserSight;					// daca sa deseneze laser sight
	float	fTimeSinceShot;						// timpul de la ultimul glont tras

	//controls
	bool	bTriggerDown, bTriggerDownOld;		// state of trigger and old state of trigger
	bool	bReloadDown;						// reload trigger state
	CActor*	pOwner;								// weapon owner
	//ctor
	CWeapon();

	// Initializez weapon from a weapon template
	void					Init( CActor* pOwnerActor, CWeaponTemplate * templ );
	// returns weapon state
	inline EWpnStatus		GetState() { return status; }
	// Updates weapon internal data
	EWpnStatus				Update( float dTime );
	// Communicates the states of the trigger and reload trigger to the weapon
	void					SetTriggerStates(bool bTriggerPushed, bool bReloadPushed);
	// Resets the burst counter for weapons that shoot in bursts
	void					ResetBurst();
	// Stops reloading current weapon
	void					StopReloading();
	// Stops shooting if during shooting cycle
	void					StopShootingCycle();
	// returns true if it is during the shooting cycle
	bool					IsShootingBullet();
	// returns true if weapon will shoot a bullet (ALL CONDITIONS)
	bool					IsReadyToShoot();
};

