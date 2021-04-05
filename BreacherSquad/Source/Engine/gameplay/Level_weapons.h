#pragma once

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

	float			fFireRateWait;			//fire rate  - cat timp trebuie sa treaca intre gloante
	bool			bResetFireRateOnTriggerUp;	//cand ridici de pe fire reseteaza fire rate
	bool			bUsesMainWeaponAmmo;		//for alt fire weapons: este doar un mod de tragere care foloseste aceeasi munitie ca si arma principala (aimed shot, double tap, etc)

	int				nBulletChamberSize;		//daca are bullet chamber sau nu (0 sau 1) - se aduna la bullets left. Nu poti seta chamber size mai mare
	bool			bCanShootFromCrouch, bCanShootFromAir, bCanShootFromLadders, bCanShootFromCover;
	int				nDropShellFrame;		//frame number of shell from SHELLS animation (-1 - no shell)
	int				nBurstSize;				//cate gloante trage intr-un burst (0 pt full automatic)
	float			fBurstCooldown;			//dupa cat timp de la burst poate trage din nou

	float			fJammedDuration;		//durata de blocare a armei cand ia damage
	float			fMuzzleLightSize;		//size of lighting effect when shooting
	bool			bHasLaserSight;			//daca are laser sight
	float			fShooterSpeedSlowingPercent; //procentul cu care scade viteza tragatorului daca se misca in timp ce trage
	//TODO: camera recoil se va face diferit
	float			fCameraRecoil;			//recul camera
	float			fSoundRadius;			//cat de departe se aude?
	bool			bPassive;				//arma pasiva, nu se foloseste ca si arma normala, se citesc doar proprietatile

	CStringHash		shScript_OnFire;		//called when shooting a weapon. If not set it just shoots the weapon.
	CStringHash		shScript_OnFireALT;		//called when shooting ALT mode for weapon. If not set it just shoots the ALT weapon.
	CStringHash		shScript_OnEmpty;		//called when weapon is empty

	//sounds - indexuri de sunete
	int		sndidxShoot, sndidxReload, sndidxEmpty;
	//alternatives
	int		sndidxShoot2, sndidxReload2, sndidxEmpty2;
	//bullets can trigger a sound action (verse) on the actor (grenades trigger "FIRE IN THE HOLE" verse)
	EActorSoundVerse	sndActorVerse;

	CWeaponTemplate() :
		//generic data
		fSpeedPenaltyPercent(0.0f),
		//other data
		nBulletsPerShot(5), 
		fFireRateWait(0.0f), fMuzzleLightSize(0.0f), nClipSize(10),
		fReloadTimePerUnit(1.0f), nReloadUnitSize(1),
		fAimErrorMaxFOV(0.0f), fAimErrorAddPerShot(0.0f), fAimErrorCooldownPerSecond(1.0f), fSpreadFOV(0.0f), fAimErrorMulPerShot(1.0f),
		bCanShootFromCrouch(true), bCanShootFromAir(false), bCanShootFromLadders(false), bCanShootFromCover(false), nDropShellFrame(-1),
		nBurstSize(0), bResetFireRateOnTriggerUp(false), bUsesMainWeaponAmmo(false), fShooterSpeedSlowingPercent(0.0f), nBulletChamberSize(0),
		sndidxShoot(-1), sndidxReload(-1), sndidxEmpty(-1), sndidxShoot2(-1), sndidxReload2(-1), sndidxEmpty2(-1),
		fJammedDuration(0.0f), fSoundRadius(128.0f), fBurstCooldown(0.0f), bHasLaserSight(false), bPassive(false),
		nHUD_AnimIdx(-1), nHUD_AnimIdxALT(-1), nMuzzleFlashAnim(-1), fAimFOV(0.0f),
		sndActorVerse(K_LVL_ACT_VERSE_EMPTY)
	{
		fCameraRecoil = 4.0f;
		name.Reset();
		shTemplateOverwrite.Reset();

		shScript_OnEmpty.Reset();
		shScript_OnFire.Reset();
		shScript_OnFireALT.Reset();
	}
};

enum EnumWeaponStatus {
	K_LVL_WPN_STATUS_UNKNOWN = -1,		//not initialized!

	K_LVL_WPN_STATUS_READY = 0,		//ready to shoot
	K_LVL_WPN_STATUS_COOLING,		//waiting between shots
	K_LVL_WPN_STATUS_JUST_SHOT,		//status setat dupa fiecare glont tras
	//--- de aici sunt stari in care CanShootWeapon intoarce false ---
	K_LVL_WPN_STATUS_RELOADING,		//reloading
	K_LVL_WPN_STATUS_JAMMED,		//cand se blocheaza arma (de ex cand isi ia stun parentul)
	K_LVL_WPN_STATUS_BURST_END,		//la capat de burst
	K_LVL_WPN_STATUS_NO_AMMO,		//cand ramane fara gloante
};

//consumable perks that can be set on weapons
struct CWeaponPerk {
	bool	bEnabled;			//is it enabled?
	float	fDamage_percAdd;	//damage that gets added from original (0.0f default)
	float	fROF_percAdd;		//rate of fire percent added (0.0f default)
	int		nDurationShots;		//how many shots is it active?
	float	fDurationTime;		//how much time is it active?

	CWeaponPerk() : fDamage_percAdd(0.0f), fROF_percAdd(0.0f), fDurationTime(0.0f), nDurationShots(0), bEnabled(false)
	{}

	void Reset()
	{
		bEnabled = false;
		fDamage_percAdd = 0.0f;
		fROF_percAdd = 0.0f;
		nDurationShots = 0;
		fDurationTime = 0.0f;
	}
};

class CWeapon
{
public:
	//#TODO #MAYBE:sa am un array de templates pentru modurile de tragere sau poate va fi in template-ul armei.
	CWeaponTemplate WeaponTemplate;
public:
	EnumWeaponStatus		status;					//status arma: ready, reloading
	EnumWeaponStatus		statusOld;				//status vechi arma: ca sa stim cand abia s-a schimbat
	//consumabile
	float	fAimErrorFOV;			//FOV-ul curent de eroare aim
	int		m_nBurstBulletsShot;	//cate gloante s-au tras din burst 
	int		m_nBulletsShotSinceCool; //how many bullets were shot in a burst since weapon was cool
	int		ammoLeft;				//-1 pt nr infinit de gloante
	float	fireRateTimer;			//timer de fire rate
	float	reloadTimer;			//timer reload
	float	fJammedTimer;			//timer jammed weapon
	int		nCanResetJamCount;		//can reset jam timer a few times (used usually when changing from one weapon to another so it doesn't shoot right away)
	bool	bPaintLaserSight;		//daca sa deseneze laser sight
	float	fTimeSinceShot;			//timpul de la ultimul glont tras
	//perks
	CWeaponPerk		m_activePerk;	//active weapon perk (could be an array if needed)
	//controls
	bool	bTriggerDown, bTriggerDownOld;	//e apasat tragaciul? (si starea anterioara)
	bool	bReloadDown;			//e apasat reload-ul?
	CActor*	pOwner;					//ownerul armei
	CSprite	m_sprMuzzleFlash;		//sprite pentru muzzle flash
	//ctor
	CWeapon();

	void SetTriggerStates(bool bTriggerPushed, bool bReloadPushed);
};
