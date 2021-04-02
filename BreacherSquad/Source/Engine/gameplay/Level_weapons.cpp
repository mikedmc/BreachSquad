#include "dxstdafx.h"
#include "Level_weapons.h"


///--------------------------------------------------------------------------
/// WEAPONS CLASS
///--------------------------------------------------------------------------
void CWeapon::Init()
{
	status = K_LVL_WPN_STATUS_UNKNOWN; //not initialized yet
	fAimErrorFOV = 0.0f;
	fireRateTimer = 0.0f;
	m_nBurstBulletsShot = 0;
	reloadTimer = 0.0f;
	ammoLeft = -1;
	fJammedTimer = 0.0f;
	bPaintLaserSight = false;
	m_sprMuzzleFlash.animationIdx = -1; //not set
	fTimeSinceShot = 0.0f;

	bTriggerDown = bTriggerDownOld = false;
	bReloadDown = false;
	pOwner = null;

	m_activePerk.Reset();
}

void CWeapon::SetTriggerStates(bool bTriggerPushed, bool bReloadPushed)
{
	//save old state - In Update se egaleaza din nou ca sa apara mesajul doar odata
	bTriggerDownOld = bTriggerDown;
	//set new state
	bTriggerDown = bTriggerPushed;
	//set reload trigger
	bReloadDown = bReloadPushed;

	//can reset jam timer
	if ((nCanResetJamCount > 0) && (fJammedTimer > 0.0f) && (bTriggerDown == false) && (bTriggerDownOld == false))
	{
		fJammedTimer = 0.0f;
		nCanResetJamCount--;
	}
}

///----------------------------------------------------------------------------------
/// LEVEL WEAPON METHODS
///----------------------------------------------------------------------------------


EnumWeaponStatus CLevel::UpdateWeapon(CWeapon * weapon, float dTime)
{
	//save old status
	weapon->statusOld = weapon->status;

	if ((weapon == null) || (weapon->status == K_LVL_WPN_STATUS_UNKNOWN))
		return K_LVL_WPN_STATUS_UNKNOWN;

	//wpn perks
	if (weapon->m_activePerk.bEnabled)
	{
		if (weapon->m_activePerk.fDurationTime > 0.0f)
			weapon->m_activePerk.fDurationTime -= dTime;
		//perk off if durations expired
		if ((weapon->m_activePerk.fDurationTime <= 0.0f) && (weapon->m_activePerk.nDurationShots <= 0))
			weapon->m_activePerk.bEnabled = false;
	}

	//update muzzle flash
	if (weapon->m_sprMuzzleFlash.animationIdx >= 0)
	{
		weapon->m_sprMuzzleFlash.Update(&m_sprActors, dTime);
	}

	//update aiming errors
	weapon->fTimeSinceShot += dTime;
	//face cooldown doar dupa ce a incetat sa traga de ceva timp:
	if (weapon->fTimeSinceShot > 0.1f) //approx 2 frames la 24 fps
	{
		dec_limit(weapon->fAimErrorFOV, weapon->WeaponTemplate.fAimErrorCooldownPerSecond * dTime, 0.0f);
	}
	//scade fire rate timer
	dec_limit(weapon->fireRateTimer, dTime, 0.0f);
	//reset burst and other data on trigger up
	if ((weapon->bTriggerDown == false) && (weapon->status == K_LVL_WPN_STATUS_BURST_END))
	{
		weapon->m_nBurstBulletsShot = 0;
		//jam weapon for burst cooldown
		weapon->fJammedTimer = weapon->WeaponTemplate.fBurstCooldown;
	}
	if ((weapon->bTriggerDown == false) && (weapon->fTimeSinceShot > 0.25f) && (weapon->fAimErrorFOV <= 0.0f))
	{
		weapon->m_nBulletsShotSinceCool = 0;
	}
	//reset timer on trigger up
	if ((weapon->bTriggerDown == false) && (weapon->WeaponTemplate.bResetFireRateOnTriggerUp))
		weapon->fireRateTimer = 0.0f;

	//on trigger down play emty sound 
	if ((weapon->bTriggerDownOld == false) && (weapon->bTriggerDown == true))
	{
		if ((weapon->ammoLeft == 0) && (weapon->WeaponTemplate.sndidxEmpty >= 0))
			SND_PLAY_POSITIONAL(weapon->WeaponTemplate.sndidxEmpty, weapon->pOwner->GetPosHeart());
	}
	//update old trigger state
	weapon->bTriggerDownOld = weapon->bTriggerDown;

	//if jammed can't reload, can't shoot
	if (weapon->fJammedTimer > 0.0f)
	{
		dec_limit(weapon->fJammedTimer, dTime, 0.0f);
		weapon->status = K_LVL_WPN_STATUS_JAMMED;

		return weapon->status;
	}

	if ((weapon->status != K_LVL_WPN_STATUS_RELOADING) && (weapon->bReloadDown) && (!weapon->bTriggerDown) && (weapon->ammoLeft < weapon->WeaponTemplate.nClipSize + weapon->WeaponTemplate.nBulletChamberSize))
	{
		SND_PLAY_POSITIONAL_RAND2(weapon->WeaponTemplate.sndidxReload, weapon->WeaponTemplate.sndidxReload2, weapon->pOwner->GetPosHeart());

		weapon->reloadTimer = 0.0f;
		weapon->status = K_LVL_WPN_STATUS_RELOADING;
	}

	//fire rate timer
	if (weapon->status != K_LVL_WPN_STATUS_RELOADING)
	{
		if (weapon->fireRateTimer <= 0.0f)
		{
			weapon->fireRateTimer = 0.0f;
			weapon->status = K_LVL_WPN_STATUS_READY;
		}
		else
		{
			weapon->status = K_LVL_WPN_STATUS_COOLING;
		}
	}
	//burst lock
	if ((weapon->WeaponTemplate.nBurstSize > 0) && (weapon->m_nBurstBulletsShot >= weapon->WeaponTemplate.nBurstSize))
	{
		weapon->status = K_LVL_WPN_STATUS_BURST_END;
	}

	if (weapon->bTriggerDown)
	{
		//stop reloading if possible (for shotgun type weapons)
		if ((weapon->status == K_LVL_WPN_STATUS_RELOADING) && (weapon->WeaponTemplate.nReloadUnitSize < weapon->WeaponTemplate.nClipSize + weapon->WeaponTemplate.nBulletChamberSize) &&
			(weapon->ammoLeft > 0) && (weapon->fireRateTimer <= 0.0f))
		{
			weapon->status = K_LVL_WPN_STATUS_READY;
			weapon->fireRateTimer = 0.0f;
			weapon->reloadTimer = 0.0f;
		}
	}
	//suntem inca pe reloading, facem reload
	if (weapon->status == K_LVL_WPN_STATUS_RELOADING)
	{
		weapon->reloadTimer += dTime;

		if (weapon->reloadTimer >= weapon->WeaponTemplate.fReloadTimePerUnit)
		{
			weapon->ammoLeft += weapon->WeaponTemplate.nReloadUnitSize;
			weapon->reloadTimer -= weapon->WeaponTemplate.fReloadTimePerUnit;

			int nMaxBullets = weapon->WeaponTemplate.nClipSize;
			//#HACK: la shotguns sa incarce automat pana la capat
			if (weapon->WeaponTemplate.nReloadUnitSize == 1)
				nMaxBullets = weapon->WeaponTemplate.nClipSize + weapon->WeaponTemplate.nBulletChamberSize;
			if (weapon->ammoLeft >= nMaxBullets)
			{
				CLAMP(weapon->ammoLeft, 0, weapon->WeaponTemplate.nClipSize + weapon->WeaponTemplate.nBulletChamberSize);
				weapon->reloadTimer = 0.0f;

				weapon->status = K_LVL_WPN_STATUS_READY;
			}
			else //daca incarca in mai multe secvente face play din nou la reload
			{
				SND_PLAY_POSITIONAL_RAND2(weapon->WeaponTemplate.sndidxReload, weapon->WeaponTemplate.sndidxReload2, weapon->pOwner->GetPosHeart());
			}
		}
	}
	//daca e cooling dar no ammo pun status pe no ammo
	if (weapon->ammoLeft == 0)
	{
		if (weapon->status <= K_LVL_WPN_STATUS_COOLING)
			weapon->status = K_LVL_WPN_STATUS_NO_AMMO;
	}

	return weapon->status;
}

void CLevel::ResetBurstWeapon(CWeapon * weapon)
{
	if (weapon == null)
		return;

	weapon->m_nBurstBulletsShot = 0;

	if (weapon->WeaponTemplate.bResetFireRateOnTriggerUp)
		weapon->fireRateTimer = 0.0f;
}

bool CLevel::JamWeapon(CWeapon * weapon)
{
	if (weapon == null)
		return false;
	if ((weapon->status == K_LVL_WPN_STATUS_RELOADING) || (weapon->status == K_LVL_WPN_STATUS_UNKNOWN))
		return false;
	if (weapon->WeaponTemplate.fJammedDuration <= 0.0f)
		return false;

	if (weapon->fJammedTimer < weapon->WeaponTemplate.fJammedDuration)
		weapon->fJammedTimer = weapon->WeaponTemplate.fJammedDuration;

	weapon->bTriggerDown = false;
	return true;
}

void CLevel::StopReloadingWeapon(CWeapon * weapon)
{
	if (weapon == null)
		return;
	if (weapon->status != K_LVL_WPN_STATUS_RELOADING)
		return;

	weapon->bReloadDown = false;
	weapon->fireRateTimer = 0.0f;
	weapon->reloadTimer = 0.0f;

	weapon->status = K_LVL_WPN_STATUS_READY;
}


bool CLevel::CanShootWeapon(CWeapon * weapon)
{
	//no weapon or empty weapon?
	if ((weapon == null) || (weapon->WeaponTemplate.name.IsEmpty()))
		return false;

	CActor* actor = weapon->pOwner;
	//shooting from the air?
	if ((!weapon->WeaponTemplate.bCanShootFromAir) && (actor != null) && ((actor->collisionFlags & K_DIRFLAG_DOWN) == 0))
	{
		return false;
	}

	return true;
}

bool CLevel::ShootWeapon(CWeapon * weapon, Vec3 vDir)
{
	if ((weapon == null) || (weapon->pOwner == null) || (weapon->status == K_LVL_WPN_STATUS_UNKNOWN))
		return false;
	//make sure we don't shoot a jammed weapon
	if (weapon->status == K_LVL_WPN_STATUS_JAMMED)
		return false;

	CActor* shooter = weapon->pOwner;
	Vec3 vFinalDir;
	MUVec3Norm(&vFinalDir, &vDir);
	Vec3 vShootPos = shooter->GetPosWeapon();

	int nFinalClass = shooter->actTemplate.actorClass;
	//bullet has template class, set it to final class
	if (weapon->WeaponTemplate.bulletTemplate.eClass != K_LVL_ACT_CLASS_ANY)
		nFinalClass = weapon->WeaponTemplate.bulletTemplate.eClass;

	//don't shoot too often
	if (weapon->fireRateTimer > 0.0f)
		return false;
	//ended burst => stop shooting
	if ((weapon->WeaponTemplate.nBurstSize > 0) && (weapon->m_nBurstBulletsShot >= weapon->WeaponTemplate.nBurstSize))
	{
		weapon->status = K_LVL_WPN_STATUS_BURST_END;
		return false;
	}

	//save local bullet template copy
	CBulletTemplate tmplBullet = weapon->WeaponTemplate.bulletTemplate;

	//init fire rate timer
	weapon->fireRateTimer = weapon->WeaponTemplate.fFireRateWait;
	if (weapon->m_activePerk.bEnabled)
	{
		weapon->fireRateTimer += weapon->WeaponTemplate.fFireRateWait * weapon->m_activePerk.fROF_percAdd;
	}
	//ammo (-1 infinite)
	int nAmmoReal = weapon->ammoLeft;
	//if weapon uses main weapon ammo check that ammo
	if (weapon->WeaponTemplate.bUsesMainWeaponAmmo)
		nAmmoReal = weapon->pOwner->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->ammoLeft;

	if (nAmmoReal != 0)
	{
		//shooting sound (only if set). verific doar sndidx pentru ca vvarianta 2 contine cel putin valoarea primului
		if (weapon->WeaponTemplate.sndidxShoot >= 0)
		{
			SND_PLAY_POSITIONAL_RAND2(weapon->WeaponTemplate.sndidxShoot, weapon->WeaponTemplate.sndidxShoot2, weapon->pOwner->GetPosHeart());
			weapon->fTimeSinceShot = 0.0f;
		}

		if (weapon->ammoLeft > 0)
			weapon->ammoLeft--;

		//set status
		weapon->status = K_LVL_WPN_STATUS_JUST_SHOT;

		weapon->m_nBurstBulletsShot++;
		weapon->m_nBulletsShotSinceCool++;
		//process aim error
		/*
		float fAimAng = UTMath::GetVectorAngle(vFinalDir);
		float fAimAngError = 0.0f;
		if (weapon->WeaponTemplate.fAimErrorAddPerShot < 0.0f)
		{
			fAimAngError = m_rand.RandFloatSgn(weapon->WeaponTemplate.fAimErrorMaxFOV - weapon->fAimErrorFOV);
		}
		else
		{
			fAimAngError = m_rand.RandFloatSgn(weapon->fAimErrorFOV);
		}
		//better aiming when crouched or in cover
		float fMul = 1.0f;
		if (shooter->bCrouched)
			fMul = K_LVL_CROUCH_ERROR_MULTIPLIER;

		fAimAngError *= fMul;
		//apply template aiming multiplier
		//fAimAngError *= shooter->actTemplate.fRecoilModifier;
		//add aiming error
		fAimAng += fAimAngError;
		*/
		//some weapons force the actor to play a verse when shooting
		//PlayActorSoundVerse(shooter, weapon->WeaponTemplate.sndActorVerse);

		//also shoot bullets
		for (int kk = 0; kk < weapon->WeaponTemplate.nBulletsPerShot; kk++)
		{
			//add weapon spread
			float fSpreadAng = m_rand.RandFloatSgn(weapon->WeaponTemplate.fSpreadFOV);

			//vFinalDir.x = cos(fAimAng + fSpreadAng);
			//vFinalDir.y = sin(fAimAng + fSpreadAng);
			//D3DXVec2Normalize(&vFinalDir, &vFinalDir);

			//apply weapon perk
			if (weapon->m_activePerk.bEnabled)
			{
				tmplBullet.fDamage += tmplBullet.fDamage * weapon->m_activePerk.fDamage_percAdd;
			}

			LOG_DBG_BUFF(L"= Shot:%s ID:%d =", weapon->WeaponTemplate.name.text, weapon->pOwner->ID);
			CBullet* bullet = ShootBullet(&tmplBullet, nFinalClass, shooter->GetUID(), vShootPos, vFinalDir);
			//--- statistics ---
			if ((bullet != NULL) && ((bullet->nFlags & K_LVL_BULLET_FLAG_NOT_BALLISTIC) == 0) && (weapon->pOwner->actTemplate.actorClass == K_LVL_ACT_CLASS_PLAYER))
			{
				//aici numara si grenadele dar nu prea conteaza pt ca tragi multe gloante in joc
				m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT + weapon->pOwner->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT]++;
			}
		}

		//adaug shell
		if (weapon->WeaponTemplate.nDropShellFrame >= 0)
		{
			AddDoofer(K_DOOFER_SHELL, weapon->pOwner->GetPosHeart(), &Vec2((40.0f + randfloat(30.0f)), -50.0f - randfloat(20.0f)), &g_vecGravityOld, weapon->WeaponTemplate.nDropShellFrame);
		}

		float fAimErrorMul = 1.0f;

		weapon->fAimErrorFOV += fabs(weapon->WeaponTemplate.fAimErrorAddPerShot); //add aim error (can be negative too)
		weapon->fAimErrorFOV *= weapon->WeaponTemplate.fAimErrorMulPerShot; //add non linear error
		weapon->fAimErrorFOV *= fAimErrorMul; //scale aiming error from perks
		CLAMP(weapon->fAimErrorFOV, 0.0f, weapon->WeaponTemplate.fAimErrorMaxFOV); //limit max error fov

		//make light
		if (weapon->WeaponTemplate.fMuzzleLightSize > 0.0f)
		{
			//prop - nozzle light
			float fPropAlpha = 0.8f * weapon->WeaponTemplate.fMuzzleLightSize;
			CLAMP(fPropAlpha, 0.0f, 1.0f);
			//			AddProp_Light(vShootPos, ANM_LIGHTS_SPR_POINT1, 0.05f, 0.0f, D3DCOLOR_COLORALPHA(0xffFDB727, fPropAlpha), weapon->WeaponTemplate.fMuzzleLightSize);
		}
		//adaug eventAI de sunet
		AddAIEvent(K_LVL_AI_EVENT_SOUND_THREAT, shooter->GetUID(), shooter->actTemplate.actorClass, shooter->GetPosHeart(), weapon->WeaponTemplate.fSoundRadius);
	}
	else
	{
		//empty clip sound
		SND_PLAY_POSITIONAL_RAND2(weapon->WeaponTemplate.sndidxEmpty, weapon->WeaponTemplate.sndidxEmpty2, weapon->pOwner->GetPosHeart());
		weapon->status = K_LVL_WPN_STATUS_NO_AMMO;

		return false;
	}

	//animate muzzle flash
	weapon->m_sprMuzzleFlash.SetFrame(0);

	//update perk (times shot)
	if (weapon->m_activePerk.bEnabled)
	{
		if (weapon->m_activePerk.nDurationShots > 0)
			weapon->m_activePerk.nDurationShots--;
	}

	return true;
}

