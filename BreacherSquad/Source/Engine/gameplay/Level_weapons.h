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
	//generic data:
	int				nHUD_AnimIdx;		//animatie INGAME HUD pentru grafica armei (vezi detalii frames in CCustomInterfaceIGM) sau -1 pt empty
	int				nHUD_AnimIdxALT;	//animatie INGAME HUD when shown as ALT weapon (painted just as small icon)
	int				nMuzzleFlashAnim;	//animatie muzzle flash sau -1 pt empty
	CStringHash		shTemplateOverwrite; //name of themplate that the weapon overwrites over the character template
	float			fSpeedPenaltyPercent;	//what percent of total movement speed is taken by this weapon
	CStringHash		shScript_OnFire;		//called when shooting a weapon. If not set it just shoots the weapon.
	CStringHash		shScript_OnFireALT;		//called when shooting ALT mode for weapon. If not set it just shoots the ALT weapon.
	CStringHash		shScript_OnEmpty;		//called when weapon is empty

	//fire modes data:
	CBulletTemplate	bulletTemplate;		//datele glontului tras de arma curenta
	int				nBulletsPerShot;	//nr de gloante trase pt un ammo

	//aiming data - toate FOV-urile de mai jos sunt half FOV de fapt
	float			fSpreadFOV;							//spread default arma
	float			fAimErrorMaxFOV;					//aiming error - FOV in radiani 
	float			fAimErrorAddPerShot;				//ce eroare de AIM se adauga dupa fiecare foc
	float			fAimErrorMulPerShot;				//factor multiplicare eroare FOV ca sa nu mai creasca liniar
	float			fAimErrorCooldownPerSecond;			//cooldown AIM error in functie de timp
	float			fAimFOV;							//angle in rad. can the weapon be aimed? (default 0.0f means only aim vec -1,0 or 1,0)

	int				nClipSize;				//-1 pt nr infinit de gloante
	//reload data
	int				nReloadUnitSize;		//cate gloante incarca odata sau 0 daca nu se poate incarca
	float			fReloadTimePerUnit;		//cat timp dureaza sa incarce o unitate de glont

	float			fFireRateWait;			//fire rate  - time between bullets
	bool			bResetFireRateOnTriggerUp;	// resets fire rate
	bool			bUsesMainWeaponAmmo;		// for alt fire weapons: este doar un mod de tragere care foloseste aceeasi munitie ca si arma principala (aimed shot, double tap, etc)

	int				nBulletChamberSize;		//daca are bullet chamber sau nu (0 sau 1) - se aduna la bullets left. Nu poti seta chamber size mai mare
	bool			bCanShootFromCrouch;
	bool			bCanShootFromCover;
	int				nDropShellFrame;		//frame number of shell from SHELLS animation (-1 - no shell)
	int				nBurstSize;				//cate gloante trage intr-un burst (0 pt full automatic)
	float			fBurstCooldown;			//dupa cat timp de la burst poate trage din nou

	float			fJammedDuration;		//durata de blocare a armei cand ia damage
	float			fMuzzleLightSize;		//size of lighting effect when shooting
	bool			bHasLaserSight;			//daca are laser sight
	float			fShooterSpeedSlowingPercent; //procentul cu care scade viteza tragatorului daca se misca in timp ce trage
	float			fSoundRadius;			//cat de departe se aude?
	bool			bPassive;				//arma pasiva, nu se foloseste ca si arma normala, se citesc doar proprietatile

	Vec2			vMountOffset;			// vector showing the offset from the mount to the gun rotating position
	bool			bSingleHanded;			// can be used with a single hand
	bool			bDualWielding;			// only for single handed weapons. if true it gets doubled in the second mount position.

	//sound indices to play
	int		sndidxShoot, sndidxReload, sndidxEmpty;
	//alternatives
	int		sndidxShoot2, sndidxReload2, sndidxEmpty2;
	
	EActorSoundVerse	sndActorVerse;		// bullets can trigger a sound action (verse) on the actor (grenades trigger "FIRE IN THE HOLE" verse)

	CWeaponTemplate() :
		fSpeedPenaltyPercent(0.0f),
		//other data
		nBulletsPerShot(5), 
		fFireRateWait(0.0f), fMuzzleLightSize(0.0f), nClipSize(10),
		fReloadTimePerUnit(1.0f), nReloadUnitSize(1),
		fAimErrorMaxFOV(0.0f), fAimErrorAddPerShot(0.0f), fAimErrorCooldownPerSecond(1.0f), fSpreadFOV(0.0f), fAimErrorMulPerShot(1.0f),
		bCanShootFromCrouch(true), bCanShootFromCover(false), nDropShellFrame(-1),
		nBurstSize(0), bResetFireRateOnTriggerUp(false), bUsesMainWeaponAmmo(false), fShooterSpeedSlowingPercent(0.0f), nBulletChamberSize(0),
		sndidxShoot(-1), sndidxReload(-1), sndidxEmpty(-1), sndidxShoot2(-1), sndidxReload2(-1), sndidxEmpty2(-1),
		fJammedDuration(0.0f), fSoundRadius(128.0f), fBurstCooldown(0.0f), bHasLaserSight(false), bPassive(false),
		nHUD_AnimIdx(-1), nHUD_AnimIdxALT(-1), nMuzzleFlashAnim(-1), fAimFOV(0.0f),
		sndActorVerse(K_LVL_ACT_VERSE_EMPTY)
	{
		bSingleHanded = true;
		bDualWielding = false;
		vMountOffset = Vec2( 0.0f, 0.0f );
	}
};

enum EWeaponStatus {
	K_LVL_WPN_STATUS_UNKNOWN = -1,	//not initialized!

	K_LVL_WPN_STATUS_READY = 0,		// ready to shoot
	K_LVL_WPN_STATUS_COOLING,		// waiting between shots
	K_LVL_WPN_STATUS_JUST_SHOT,		// status setat dupa fiecare glont tras
	//--- CanShootWeapon=false states from here on ---
	K_LVL_WPN_STATUS_RELOADING,		// reloading
	K_LVL_WPN_STATUS_JAMMED,		// jammed weapon (maybe stunned owner)
	K_LVL_WPN_STATUS_BURST_END,		// burst ended, we must wait cooldown
	K_LVL_WPN_STATUS_NO_AMMO,		// no more ammo
};

// weapon fire mode (usually we have 2 on a real weapon)
class CWeapon
{
public:
	CWeaponTemplate _template;
public:
	EWeaponStatus		status;					// weapon state: ready, reloading
	EWeaponStatus		statusOld;				// old status so we know when it changes
	//consumabile
	float	fAimErrorFOV;						// FOV-ul curent de eroare aim
	int		m_nBurstBulletsShot;				// cate gloante s-au tras din burst 
	int		m_nBulletsShotSinceCool;			// how many bullets were shot in a burst since weapon was cool
	int		ammoLeft;							// -1 pt nr infinit de gloante
	float	fireRateTimer;						// timer de fire rate
	float	reloadTimer;						// timer reload
	float	fJammedTimer;						// timer jammed weapon
	int		nCanResetJamCount;					// can reset jam timer a few times (used usually when changing from one weapon to another so it doesn't shoot right away)
	bool	bPaintLaserSight;					// daca sa deseneze laser sight
	float	fTimeSinceShot;						// timpul de la ultimul glont tras

	//controls
	bool	bTriggerDown, bTriggerDownOld;		// state of trigger and old state of trigger
	bool	bReloadDown;						// reload trigger state
	CActor*	pOwner;								// weapon owner
	//ctor
	CWeapon();

	// Initializez weapon from a weapon template
	void				Init( CActor* pOwnerActor, CWeaponTemplate * templ );
	// Updates weapon internal data
	EWeaponStatus		Update( float dTime );
	// Communicates the states of the trigger and reload trigger to the weapon
	void				SetTriggerStates(bool bTriggerPushed, bool bReloadPushed);
	// Resets the burst counter for weapons that shoot in bursts
	void				ResetBurst();
	// Jams the weapon (when receiving damage for example)
	bool				Jam();
	// Stops reloading current weapon
	void				StopReloading();
};

