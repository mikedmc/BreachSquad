#include "dxstdafx.h"
#include "Level_weapons.h"



///--------------------------------------------------------------------------
/// WEAPONS CLASS
///--------------------------------------------------------------------------

CWeapon::CWeapon() :	status(K_LVL_WPN_STATUS_UNKNOWN), statusOld(K_LVL_WPN_STATUS_UNKNOWN),
						fAimErrorFOV(0.0f), fireRateTimer(0.0f), m_nBurstBulletsShot(0), m_nBulletsShotSinceCool(0),
						reloadTimer(0.0f), ammoLeft(-1), fJammedTimer(0.0f), nCanResetJamCount(0),
						bTriggerDown(false), bReloadDown(false), bTriggerDownOld(false),
						pOwner(nullptr), bPaintLaserSight(false), fTimeSinceShot(0.0f)
{
}


void CWeapon::Init( CWeaponTemplate * templ )
{
	// copy template
	_template = *templ;
	// set owner
	//#TODO: is parent needed??
	//pWeapon->pOwner = &act;
	status = K_LVL_WPN_STATUS_READY;
	ammoLeft = _template.nClipSize + _template.nBulletChamberSize;
	// make sure infinite ammo is infinite
	if ( _template.nClipSize < 0 )
		ammoLeft = -1;
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


EnumWeaponStatus CLevel::Weapon_Update(CWeapon * weapon, float dTime)
{
	//save old status
	weapon->statusOld = weapon->status;

	if ((weapon == null) || (weapon->status == K_LVL_WPN_STATUS_UNKNOWN))
		return K_LVL_WPN_STATUS_UNKNOWN;


	//update muzzle flash
	/*
	if (weapon->m_sprMuzzleFlash.animationIdx >= 0)
	{
		weapon->m_sprMuzzleFlash.Update(&m_sprActors, dTime);
	}
	*/

	//update aiming errors
	weapon->fTimeSinceShot += dTime;
	//face cooldown doar dupa ce a incetat sa traga de ceva timp:
	if (weapon->fTimeSinceShot > 0.1f) //approx 2 frames la 24 fps
	{
		dec_limit(weapon->fAimErrorFOV, weapon->_template.fAimErrorCooldownPerSecond * dTime, 0.0f);
	}
	//scade fire rate timer
	dec_limit(weapon->fireRateTimer, dTime, 0.0f);
	//reset burst and other data on trigger up
	if ((weapon->bTriggerDown == false) && (weapon->status == K_LVL_WPN_STATUS_BURST_END))
	{
		weapon->m_nBurstBulletsShot = 0;
		//jam weapon for burst cooldown
		weapon->fJammedTimer = weapon->_template.fBurstCooldown;
	}
	if ((weapon->bTriggerDown == false) && (weapon->fTimeSinceShot > 0.25f) && (weapon->fAimErrorFOV <= 0.0f))
	{
		weapon->m_nBulletsShotSinceCool = 0;
	}
	//reset timer on trigger up
	if ((weapon->bTriggerDown == false) && (weapon->_template.bResetFireRateOnTriggerUp))
		weapon->fireRateTimer = 0.0f;

	//on trigger down play emty sound 
	if ((weapon->bTriggerDownOld == false) && (weapon->bTriggerDown == true))
	{
		if ((weapon->ammoLeft == 0) && (weapon->_template.sndidxEmpty >= 0))
			SND_PLAY_POSITIONAL(weapon->_template.sndidxEmpty, weapon->pOwner->GetPosHeart());
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

	if ((weapon->status != K_LVL_WPN_STATUS_RELOADING) && (weapon->bReloadDown) && (!weapon->bTriggerDown) && (weapon->ammoLeft < weapon->_template.nClipSize + weapon->_template.nBulletChamberSize))
	{
		SND_PLAY_POSITIONAL_RAND2(weapon->_template.sndidxReload, weapon->_template.sndidxReload2, weapon->pOwner->GetPosHeart());

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
	if ((weapon->_template.nBurstSize > 0) && (weapon->m_nBurstBulletsShot >= weapon->_template.nBurstSize))
	{
		weapon->status = K_LVL_WPN_STATUS_BURST_END;
	}

	if (weapon->bTriggerDown)
	{
		//stop reloading if possible (for shotgun type weapons)
		if ((weapon->status == K_LVL_WPN_STATUS_RELOADING) && (weapon->_template.nReloadUnitSize < weapon->_template.nClipSize + weapon->_template.nBulletChamberSize) &&
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

		if (weapon->reloadTimer >= weapon->_template.fReloadTimePerUnit)
		{
			weapon->ammoLeft += weapon->_template.nReloadUnitSize;
			weapon->reloadTimer -= weapon->_template.fReloadTimePerUnit;

			int nMaxBullets = weapon->_template.nClipSize;
			//#HACK: la shotguns sa incarce automat pana la capat
			if (weapon->_template.nReloadUnitSize == 1)
				nMaxBullets = weapon->_template.nClipSize + weapon->_template.nBulletChamberSize;
			if (weapon->ammoLeft >= nMaxBullets)
			{
				CLAMP(weapon->ammoLeft, 0, weapon->_template.nClipSize + weapon->_template.nBulletChamberSize);
				weapon->reloadTimer = 0.0f;

				weapon->status = K_LVL_WPN_STATUS_READY;
			}
			else //daca incarca in mai multe secvente face play din nou la reload
			{
				SND_PLAY_POSITIONAL_RAND2(weapon->_template.sndidxReload, weapon->_template.sndidxReload2, weapon->pOwner->GetPosHeart());
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

void CLevel::Weapon_ResetBurst(CWeapon * weapon)
{
	if (weapon == null)
		return;

	weapon->m_nBurstBulletsShot = 0;

	if (weapon->_template.bResetFireRateOnTriggerUp)
		weapon->fireRateTimer = 0.0f;
}

bool CLevel::Weapon_Jam(CWeapon * weapon)
{
	if (weapon == null)
		return false;
	if ((weapon->status == K_LVL_WPN_STATUS_RELOADING) || (weapon->status == K_LVL_WPN_STATUS_UNKNOWN))
		return false;
	if (weapon->_template.fJammedDuration <= 0.0f)
		return false;

	if (weapon->fJammedTimer < weapon->_template.fJammedDuration)
		weapon->fJammedTimer = weapon->_template.fJammedDuration;

	weapon->bTriggerDown = false;
	return true;
}

void CLevel::Weapon_StopReloading(CWeapon * weapon)
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


bool CLevel::Weapon_CanShoot(CWeapon * weapon)
{
	//no weapon or empty weapon?
	if ((weapon == null) || (weapon->_template.name.IsEmpty()))
		return false;
	/*
	//#TODO: add more checkups or send this param to the AI input so he knows about it
	CActor* actor = weapon->pOwner;
	//shooting from the air?
	if ((!weapon->WeaponTemplate.bCanShootFromAir) && (actor != null) && ((actor->collisionFlags & K_DIRFLAG_DOWN) == 0))
	{
		return false;
	}
	*/

	return true;
}

bool CLevel::Weapon_Shoot(CWeapon * weapon, Vec3 vDir)
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
	if (weapon->_template.bulletTemplate.eClass != K_LVL_ACT_CLASS_ANY)
		nFinalClass = weapon->_template.bulletTemplate.eClass;

	//don't shoot too often
	if (weapon->fireRateTimer > 0.0f)
		return false;
	//ended burst => stop shooting
	if ((weapon->_template.nBurstSize > 0) && (weapon->m_nBurstBulletsShot >= weapon->_template.nBurstSize))
	{
		weapon->status = K_LVL_WPN_STATUS_BURST_END;
		return false;
	}

	//save local bullet template copy
	CBulletTemplate tmplBullet = weapon->_template.bulletTemplate;

	//init fire rate timer
	weapon->fireRateTimer = weapon->_template.fFireRateWait;
	//ammo (-1 infinite)
	int nAmmoReal = weapon->ammoLeft;
	//if weapon uses main weapon ammo check that ammo
	/*
	if (weapon->WeaponTemplate.bUsesMainWeaponAmmo)
		nAmmoReal = weapon->pOwner->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->ammoLeft;
		*/

	if (nAmmoReal != 0)
	{
		//shooting sound (only if set). verific doar sndidx pentru ca vvarianta 2 contine cel putin valoarea primului
		if (weapon->_template.sndidxShoot >= 0)
		{
			SND_PLAY_POSITIONAL_RAND2(weapon->_template.sndidxShoot, weapon->_template.sndidxShoot2, weapon->pOwner->GetPosHeart());
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
		for (int kk = 0; kk < weapon->_template.nBulletsPerShot; kk++)
		{
			//add weapon spread
			float fSpreadAng = m_rand.RandFloatSgn(weapon->_template.fSpreadFOV);

			//vFinalDir.x = cos(fAimAng + fSpreadAng);
			//vFinalDir.y = sin(fAimAng + fSpreadAng);
			//D3DXVec2Normalize(&vFinalDir, &vFinalDir);

			CBullet* bullet = ShootBullet(&tmplBullet, nFinalClass, shooter->GetUID(), vShootPos, vFinalDir);
		}

		//adaug shell
		if (weapon->_template.nDropShellFrame >= 0)
		{
			AddDoofer(K_DOOFER_SHELL, weapon->pOwner->GetPosHeart(), &Vec2((40.0f + randfloat(30.0f)), -50.0f - randfloat(20.0f)), &g_vecGravityOld, weapon->_template.nDropShellFrame);
		}

		float fAimErrorMul = 1.0f;

		weapon->fAimErrorFOV += fabs(weapon->_template.fAimErrorAddPerShot); //add aim error (can be negative too)
		weapon->fAimErrorFOV *= weapon->_template.fAimErrorMulPerShot; //add non linear error
		weapon->fAimErrorFOV *= fAimErrorMul; //scale aiming error from perks
		CLAMP(weapon->fAimErrorFOV, 0.0f, weapon->_template.fAimErrorMaxFOV); //limit max error fov

		//make light
		if (weapon->_template.fMuzzleLightSize > 0.0f)
		{
			//prop - nozzle light
			float fPropAlpha = 0.8f * weapon->_template.fMuzzleLightSize;
			CLAMP(fPropAlpha, 0.0f, 1.0f);
			//			AddProp_Light(vShootPos, ANM_LIGHTS_SPR_POINT1, 0.05f, 0.0f, D3DCOLOR_COLORALPHA(0xffFDB727, fPropAlpha), weapon->WeaponTemplate.fMuzzleLightSize);
		}
		//adaug eventAI de sunet
		AddAIEvent(K_LVL_AI_EVENT_SOUND_THREAT, shooter->GetUID(), shooter->actTemplate.actorClass, shooter->GetPosHeart(), weapon->_template.fSoundRadius);
	}
	else
	{
		//empty clip sound
		SND_PLAY_POSITIONAL_RAND2(weapon->_template.sndidxEmpty, weapon->_template.sndidxEmpty2, weapon->pOwner->GetPosHeart());
		weapon->status = K_LVL_WPN_STATUS_NO_AMMO;

		return false;
	}


	return true;
}


OPRESULT CLevel::LoadWeaponTemplates(WCHAR * xmlPath)
{
	pugi::xml_document doc;
	if (!doc.load_file(xmlPath))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"Unable to load Weapon Templates XML:%s\n", xmlPath);
	}

	// load weapons sprites
	int m_libidxWeapons;
	WCHAR wcsPath[ MAX_PATH ];
	WCHAR Path[ MAX_PATH ];
	swprintf_s( wcsPath, MAX_PATH, L"media/levels/data/%s", doc.root().child( L"WEAPONS" ).attribute( L"file" ).value());
	FileManager::GetMediaPath( wcsPath, Path );
	V_OP_RET( m_sprActors.AddSprites( Path, m_libidxWeapons, K_LIBNICK_WEAPONS ) );

	//load explosion templates
	SAFE_DELETE_GROWABLE_ARRAY(m_arrTemplatesExplosion);
	pugi::xml_node rootnodeexplo = doc.root().child(L"WEAPONS").child(L"ExplosionTemplates");
	for (pugi::xml_node bnode = rootnodeexplo.first_child(); bnode; bnode = bnode.next_sibling())
	{
		CExplosionTemplate* templ = new CExplosionTemplate();
		templ->cDoT.Set(CDamageOverTime::K_LVL_DoT_NONE);
		//name
		const WCHAR* bType = bnode.name();
		templ->name.Init(bType);

		templ->fDamage = bnode.attribute(L"fDamage").as_float();
		templ->fDamageRadius = bnode.attribute(L"fDamageRadius").as_float();
		templ->fStunDuration = bnode.attribute(L"fStunDuration").as_float();
		templ->fStunRadius = bnode.attribute(L"fStunRadius").as_float();
		templ->fSoundRadius = bnode.attribute(L"fSoundRadius").as_float();
		templ->nShrapnelCnt = bnode.attribute(L"nShrapnelCnt").as_int();
		templ->nNapalmCnt = bnode.attribute(L"nNapalmCnt").as_int();
		templ->fMaxImpulse = bnode.attribute(L"fMaxImpulse").as_float();
		if (!bnode.attribute(L"nArmorPiercingRating").empty())
			templ->nArmorPiercingRating = bnode.attribute(L"nArmorPiercingRating").as_int();
		if (!bnode.attribute(L"fDamageObjectsMultiplier").empty())
			templ->fDamageObjectsMultiplier = bnode.attribute(L"fDamageObjectsMultiplier").as_float();
		//excluded class 
		templ->eIgnoreActorClass = K_LVL_ACT_CLASS_ANY;
		if (!bnode.attribute(L"sIgnoredClass").empty())
			templ->eIgnoreActorClass = (EActorClass)GetListIndexByName(bnode.attribute(L"sIgnoredClass").value(), EActorClassNames, EActorClass::K_LVL_ACT_CLASSES_COUNT);

		//damage over time
		templ->cDoT.Set(CDamageOverTime::K_LVL_DoT_NONE);
		templ->fDoTRadius = 0.0f;
		if (!bnode.attribute(L"sDoTType").empty())
		{
			templ->cDoT.eType = (CDamageOverTime::EDoTType)GetListIndexByName(bnode.attribute(L"sDoTType").value(), EDoTTypeNames, CDamageOverTime::K_LVL_DoT_COUNT);
			//get radius
			templ->fDoTRadius = bnode.attribute(L"fDoTRadius").as_float();
			//get duration
			templ->cDoT.fDuration = bnode.attribute(L"fDoTDuration").as_float();
			//get damage per sec
			templ->cDoT.fDamagePerSec = bnode.attribute(L"fDoTDamagePerSec").as_float();
			//excluded class 
			templ->cDoT.eExcludedActClass = K_LVL_ACT_CLASS_ANY;
			if (!bnode.attribute(L"sDoTIgnoredClass").empty())
				templ->cDoT.eExcludedActClass = (EActorClass)GetListIndexByName(bnode.attribute(L"sDoTIgnoredClass").value(), EActorClassNames, EActorClass::K_LVL_ACT_CLASSES_COUNT);
			if (!bnode.attribute(L"sDoTClassFilter").empty())
				templ->cDoT.eFilteredActClass = (EActorClass)GetListIndexByName(bnode.attribute(L"sDoTClassFilter").value(), EActorClassNames, EActorClass::K_LVL_ACT_CLASSES_COUNT);
		}

		m_arrTemplatesExplosion.Add(templ);
	}
	//load weapon templates
	SAFE_DELETE_GROWABLE_ARRAY(m_arrTemplatesWeapon);

	pugi::xml_node rootnode = doc.root().child(L"WEAPONS").child(L"WeaponTemplates");
	for (pugi::xml_node bnode = rootnode.first_child(); bnode; bnode = bnode.next_sibling())
	{
		CWeaponTemplate* templ = new CWeaponTemplate();
		//name
		const WCHAR* bType = bnode.name();
		templ->name.Init(bType);

		//load generic weapon data
		if (!bnode.attribute(L"nType").empty())
			templ->eType = (EWeaponType)bnode.attribute(L"nType").as_int();
		templ->nHUD_AnimIdx = -1;
		if (!bnode.attribute(L"sHUDanimName").empty())
			templ->nHUD_AnimIdx = m_sprInterface.GetAnimationIdxByName(bnode.attribute(L"sHUDanimName").value());
		templ->nHUD_AnimIdxALT = -1;
		if (!bnode.attribute(L"sHUDanimNameIcon").empty())
			templ->nHUD_AnimIdxALT = m_sprInterface.GetAnimationIdxByName(bnode.attribute(L"sHUDanimNameIcon").value());
		if (!bnode.attribute(L"fSpeedPenaltyPercent").empty())
			templ->fSpeedPenaltyPercent = bnode.attribute(L"fSpeedPenaltyPercent").as_float();
		if (!bnode.attribute(L"bPassive").empty())
			templ->bPassive = bnode.attribute(L"bPassive").as_bool();

		//muzzle flash anim
		templ->nMuzzleFlashAnim = -1;
		/*
		if (!bnode.attribute(L"sMuzzleFlashAnim").empty())
			templ->nMuzzleFlashAnim = m_sprActors.GetAnimationIdxByName(bnode.attribute(L"sMuzzleFlashAnim").value());
			*/
		//template overwrite sTemplateOverwrite - overwrites the actor default template (Adds to it)
		if (!bnode.attribute(L"sTemplateOverwrite").empty())
		{
			templ->shTemplateOverwrite.Init(bnode.attribute(L"sTemplateOverwrite").value());
		}
		//weapon scripts
		if (!bnode.attribute(L"sScript_OnFire").empty())
		{
			templ->shScript_OnFire.Init(bnode.attribute(L"sScript_OnFire").value());
		}
		if (!bnode.attribute(L"sScript_OnFireALT").empty())
		{
			templ->shScript_OnFireALT.Init(bnode.attribute(L"sScript_OnFireALT").value());
		}
		if (!bnode.attribute(L"sScript_OnEmpty").empty())
		{
			templ->shScript_OnEmpty.Init(bnode.attribute(L"sScript_OnEmpty").value());
		}

		//load primary mode
		pugi::xml_node primnode = bnode.child(L"PRIMARY");
		if (!primnode.empty())
		{
			///--- bullet data ---
			//strings
			templ->bulletTemplate.nType = K_LVL_BULLET_UNKNOWN;
			if (!primnode.attribute(L"sBulletType").empty())
			{
				templ->bulletTemplate.nType = (EBulletType)GetListIndexByName(primnode.attribute(L"sBulletType").value(), EBulletTypeNames, K_LVL_BULLETS_COUNT);
			}
			//#TODO: bullet  group should be loaded from template
			templ->bulletTemplate.nGroup = K_LVL_BULLGROUP_BULLETS;
			//bullet explosion template hash (at the end of bullet life)
			templ->bulletTemplate.nExploTemplateHash = 0;
			if (!primnode.attribute(L"sBulletExploTemplate").empty())
			{
				templ->bulletTemplate.nExploTemplateHash = FastHash(primnode.attribute(L"sBulletExploTemplate").value());
			}
			//override bullet class
			templ->bulletTemplate.eClass = K_LVL_ACT_CLASS_ANY;
			if (!primnode.attribute(L"sBulletClass").empty())
			{
				templ->bulletTemplate.eClass = (EActorClass)GetListIndexByName(primnode.attribute(L"sBulletClass").value(), EActorClassNames, K_LVL_ACT_CLASSES_COUNT);
			}

			templ->bulletTemplate.fDamage = primnode.attribute(L"fBulletDamage").as_float();
			templ->bulletTemplate.fDamageLossPPx = primnode.attribute(L"fBulletDamageLossPPx").as_float();
			templ->bulletTemplate.fLife = primnode.attribute(L"fBulletLife").as_float();
			templ->bulletTemplate.fStunDuration = primnode.attribute(L"fBulletStunDuration").as_float();
			templ->bulletTemplate.fSpeed_ini = primnode.attribute(L"fBulletSpeed").as_float();
			templ->bulletTemplate.nArmorPiercingRating = primnode.attribute(L"nArmorPiercingRating").as_int();
			templ->bulletTemplate.fDamageObjects = primnode.attribute(L"fBulletDamageObjects").as_float();
			//bullet momentul (minimum not zero)
			templ->bulletTemplate.fMomentum = primnode.attribute(L"fBulletMomentum").as_float();
			if (templ->bulletTemplate.fMomentum == 0.0f)
				templ->bulletTemplate.fMomentum = 0.1f;
			//defaults
			templ->bulletTemplate.fSelfDamageMultiplier = 1.0f;
			if (!primnode.attribute(L"fBulletSelfDamageMultiplier").empty())
				templ->bulletTemplate.fSelfDamageMultiplier = primnode.attribute(L"fBulletSelfDamageMultiplier").as_float();
			templ->bulletTemplate.fCriticalHitChance = 0.0f;
			if (!primnode.attribute(L"fBulletCriticalChance").empty())
				templ->bulletTemplate.fCriticalHitChance = primnode.attribute(L"fBulletCriticalChance").as_float();
			//bullet flags
			templ->bulletTemplate.nFlags = K_LVL_BULLET_FLAG_NONE;
			if (templ->bulletTemplate.fDamageObjects > 0.0f)
				templ->bulletTemplate.nFlags |= K_LVL_BULLET_FLAG_BREAKS_DOORS;
			if (primnode.attribute(L"bBulletIgnoreArmor").as_bool())
				templ->bulletTemplate.nFlags |= K_LVL_BULLET_FLAG_IGNORE_ARMOR;
			if (primnode.attribute(L"bBulletIgnoreCover").as_bool())
				templ->bulletTemplate.nFlags |= K_LVL_BULLET_FLAG_IGNORE_COVER;
			if (primnode.attribute(L"bBulletDieOnImpact").as_bool())
				templ->bulletTemplate.nFlags |= K_LVL_BULLET_FLAG_DIE_ON_IMPACT;
			if (primnode.attribute(L"bBulletCanSplat").as_bool())
				templ->bulletTemplate.nFlags |= K_LVL_BULLET_FLAG_CAN_SPLAT;
			if (primnode.attribute(L"bBulletDirectional").as_bool())
				templ->bulletTemplate.nFlags |= K_LVL_BULLET_FLAG_DIRECTIONAL;
			///--- weapon data ---
			//calculam timpul intre gloante din fire rate per second
			templ->fFireRateWait = primnode.attribute(L"fFireRatePerSec").as_float();
			templ->fFireRateWait = 1.0f / templ->fFireRateWait;

			//other constants
			templ->nBulletsPerShot = primnode.attribute(L"nBulletsPerShot").as_int();
			templ->fSpreadFOV = primnode.attribute(L"fSpreadFOV").as_float();
			templ->fAimFOV = primnode.attribute(L"fAimFOV").as_float();
			templ->fAimErrorMaxFOV = primnode.attribute(L"fAimErrorMaxFOV").as_float();
			templ->fAimErrorAddPerShot = primnode.attribute(L"fAimErrorAddPerShot").as_float();
			templ->fAimErrorCooldownPerSecond = primnode.attribute(L"fAimErrorCooldownPerSec").as_float();
			templ->nClipSize = primnode.attribute(L"nClipSize").as_int();
			templ->nReloadUnitSize = primnode.attribute(L"nReloadUnitSize").as_int();
			templ->fReloadTimePerUnit = primnode.attribute(L"fReloadTimePerUnit").as_float();
			templ->bResetFireRateOnTriggerUp = primnode.attribute(L"bCanResetFireRate").as_bool();
			templ->bUsesMainWeaponAmmo = primnode.attribute(L"bUsesMainWeaponAmmo").as_bool();
			templ->bCanShootFromCrouch = primnode.attribute(L"bCanShootFromCrouch").as_bool();
			templ->bCanShootFromCover = primnode.attribute(L"bCanShootFromCover").as_bool();
			templ->nBurstSize = primnode.attribute(L"nBurstSize").as_int();
			templ->fBurstCooldown = primnode.attribute(L"fBurstCooldown").as_float();
			templ->fMuzzleLightSize = primnode.attribute(L"fMuzzleLightSize").as_float();
			templ->bHasLaserSight = primnode.attribute(L"bHasLaserSight").as_bool();
			templ->fJammedDuration = primnode.attribute(L"fJammedDuration").as_float();
			templ->fSoundRadius = primnode.attribute(L"fSoundRadius").as_float();
			//rectificate
			if (!primnode.attribute(L"fAimErrorMulPerShot").empty())
				templ->fAimErrorMulPerShot = primnode.attribute(L"fAimErrorMulPerShot").as_float();
			if (!primnode.attribute(L"fShooterSpeedSlowingPercent").empty())
				templ->fShooterSpeedSlowingPercent = primnode.attribute(L"fShooterSpeedSlowingPercent").as_float();
			if (!primnode.attribute(L"nDropShellFrame").empty())
				templ->nDropShellFrame = primnode.attribute(L"nDropShellFrame").as_int();

			templ->nBulletChamberSize = 0;
			if (!primnode.attribute(L"bBulletChamber").empty())
				templ->nBulletChamberSize = (primnode.attribute(L"bBulletChamber").as_bool() == true) ? 1 : 0;

			//actor verses for the bullet
			if (!primnode.attribute(L"sActorShootVerse").empty())
				templ->sndActorVerse = (EActorSoundVerse)GetListIndexByName(primnode.attribute(L"sActorShootVerse").value(), EActorSoundVerseNames, EActorSoundVerse::K_LVL_ACT_VERSES_COUNT);

			//sounds
			/*
			if(!primnode.attribute(L"sSndShoot").empty())
				templ->sndidxShoot = UTGetSoundManager().getSndIdxW(primnode.attribute(L"sSndShoot").value());
			if (!primnode.attribute(L"sSndReload").empty())
				templ->sndidxReload = UTGetSoundManager().getSndIdxW(primnode.attribute(L"sSndReload").value());
			if (!primnode.attribute(L"sSndEmpty").empty())
				templ->sndidxEmpty = UTGetSoundManager().getSndIdxW(primnode.attribute(L"sSndEmpty").value());
			//alternative sounds
			templ->sndidxShoot2 = templ->sndidxShoot;
			if (!primnode.attribute(L"sSndShoot2").empty())
				templ->sndidxShoot2 = UTGetSoundManager().getSndIdxW(primnode.attribute(L"sSndShoot2").value());
			templ->sndidxReload2 = templ->sndidxReload;
			if (!primnode.attribute(L"sSndReload2").empty())
				templ->sndidxReload2 = UTGetSoundManager().getSndIdxW(primnode.attribute(L"sSndReload2").value());
			templ->sndidxEmpty2 = templ->sndidxEmpty;
			if (!primnode.attribute(L"sSndEmpty2").empty())
				templ->sndidxEmpty2 = UTGetSoundManager().getSndIdxW(primnode.attribute(L"sSndEmpty2").value());
				*/
		}
		else
		{
			ErrorBox(K_ERR_WARNING, L"PRIMARY weapon mode not found in template!");
		}

		m_arrTemplatesWeapon.Add(templ);
	}

	return K_OP_OK;
}

CWeapon* CLevel::Weapon_Create(WCHAR* weaponTemplateName, CActor* pParent)
{
	CWeaponTemplate* wTempl = GetTemplateWeapon(weaponTemplateName);

	if (wTempl == nullptr)
		return nullptr;

	CWeapon* pWeapon = new CWeapon();
	// set owner
	pWeapon->pOwner = pParent;
	// copy data to local weapon template as we need it later on
	pWeapon->_template = *wTempl;
	// signal valid weapon
	pWeapon->status = K_LVL_WPN_STATUS_READY;
	pWeapon->ammoLeft = pWeapon->_template.nClipSize + pWeapon->_template.nBulletChamberSize;
	// make sure infinite ammo is infinite
	if (pWeapon->_template.nClipSize < 0)
		pWeapon->ammoLeft = -1;

	return pWeapon;
}
