#include "dxstdafx.h"
#include "Level_weapons.h"



///--------------------------------------------------------------------------
/// WEAPONS CLASS
///--------------------------------------------------------------------------

CWeapon::CWeapon() :	status(K_WPN_STATUS_UNKNOWN), statusOld(K_WPN_STATUS_UNKNOWN),
						fAimErrorFOV(0.0f), nBurstBulletsShot(0), ammoLeft(-1), 
						bTriggerDown(false), bReloadDown(false), bTriggerDownOld(false),
						pOwner(nullptr), bPaintLaserSight(false), fTimeSinceShot(0.0f),
						fStateT(0.0f)
{
}


void CWeapon::Init( CActor* pOwnerActor, CWeaponTemplate * templ )
{
	// copy template
	_template = *templ;
	// set owner
	pOwner = pOwnerActor;
	status = K_WPN_STATUS_READY;
	ammoLeft = _template.nClipSize + _template.nBulletChamberSize;
	// make sure infinite ammo is infinite
	if ( _template.nClipSize < 0 )
		ammoLeft = -1;
}

EWpnStatus CWeapon::Update( float dTime )
{
	// weapons that are not set should not update
	if ( status == K_WPN_STATUS_UNKNOWN )
		return K_WPN_STATUS_UNKNOWN;
	_ASSERT( pOwner != nullptr );

	statusOld = status;

	switch ( status )
	{
		case K_WPN_STATUS_READY:
		{
			if ( bTriggerDown )
			{
				fStateT = 0.0f;
				if ( _template.fChargeUpT > 0.0f )
					status = K_WPN_STATUS_CHARGING_UP;
				else
					status = K_WPN_STATUS_JUST_SHOT;
			}
			else if ( bReloadDown )
			{
				fStateT = 0.0f;
				status = K_WPN_STATUS_RELOADING;
			}
		}
		break;
		case K_WPN_STATUS_CHARGING_UP:
		{
			fStateT += dTime;
			if ( fStateT >= _template.fChargeUpT )
			{
				fStateT -= _template.fChargeUpT;
				// set this state for a single frame
				status = K_WPN_STATUS_JUST_SHOT;
			}
		}
		break;
		case K_WPN_STATUS_JUST_SHOT:
		{
			// JUST_SHOT only stays on for a single frame.
			// The classes above do the actual shooting when this state is reached
			fTimeSinceShot = 0.0f;
			if ( _template.fWindDownT > 0.0f )
				status = K_WPN_STATUS_WINDING_DOWN;
			else
				status = K_WPN_STATUS_COOLING;
			// update ammo and stuff
			//shooting sound (only if set). verific doar sndidx pentru ca vvarianta 2 contine cel putin valoarea primului
			if ( _template.sndidxShoot >= 0 )
			{
				SND_PLAY_POSITIONAL_RAND2( _template.sndidxShoot, _template.sndidxShoot2, pOwner->GetPosHeart() );
			}

			nBurstBulletsShot++;
			if ( ammoLeft > 0 )
				ammoLeft--;

			float fAimErrorMul = 1.0f;
			fAimErrorFOV += fabs( _template.fAimErrorAddPerShot ); //add aim error (can be negative too)
			fAimErrorFOV *= _template.fAimErrorMulPerShot; //add non linear error
			fAimErrorFOV *= fAimErrorMul; //scale aiming error from perks
			CLAMP( fAimErrorFOV, 0.0f, _template.fAimErrorMaxFOV ); //limit max error fov

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
		}
		break;
		case K_WPN_STATUS_WINDING_DOWN:
		{
			fStateT += dTime;
			if ( fStateT >= _template.fWindDownT )
			{
				fStateT -= _template.fWindDownT;
				status = K_WPN_STATUS_COOLING;
			}
		}
		break;
		case K_WPN_STATUS_COOLING:
		{
			fStateT += dTime;
			//reset timer on trigger up
			if ( (bTriggerDown == false) && (_template.bResetFireRateOnTriggerUp) )
				fStateT = _template.fCooldownT;

			if ( fStateT >= _template.fCooldownT )
			{
				fStateT = 0.0f;
				status = K_WPN_STATUS_READY;
				//burst lock
				if ( (_template.nBurstSize > 0) && (nBurstBulletsShot >= _template.nBurstSize) )
				{
					nBurstBulletsShot = 0;
					status = K_WPN_STATUS_BURST_END;
				}

				if ( ammoLeft == 0 )
					status = K_WPN_STATUS_NO_AMMO;
			}
		}
		break;
		case K_WPN_STATUS_RELOADING:
		{
			fStateT += dTime;

			if ( fStateT >= _template.fReloadTimePerUnit )
			{
				ammoLeft += _template.nReloadUnitSize;
				fStateT -= _template.fReloadTimePerUnit;

				int nMaxBullets = _template.nClipSize;
				//#HACK: shotguns load everything to the end
				if ( _template.nReloadUnitSize == 1 )
					nMaxBullets = _template.nClipSize + _template.nBulletChamberSize;
				if ( ammoLeft >= nMaxBullets )
				{
					CLAMP( ammoLeft, 0, _template.nClipSize + _template.nBulletChamberSize );
					fStateT = 0.0f;
					status = K_WPN_STATUS_READY;
				}
				else // we have more to load so we play the sound again
				{
					SND_PLAY_POSITIONAL_RAND2( _template.sndidxReload, _template.sndidxReload2, pOwner->GetPosHeart() );
				}
			}

			//stop reloading if you want to shoot (for shotgun type weapons)
			if ( bTriggerDown )
			{
				if ( (_template.nReloadUnitSize < _template.nClipSize + _template.nBulletChamberSize) && (ammoLeft > 0) )
				{
					status = K_WPN_STATUS_READY;
					fStateT = 0.0f;
				}
			}
		}
		break;
		case K_WPN_STATUS_BURST_END:
		{
			nBurstBulletsShot = 0;
			fStateT = 0.0f;
			status = K_WPN_STATUS_BURST_COOLDOWN;
		}
		break;
		case K_WPN_STATUS_BURST_COOLDOWN:
		{
			fStateT += dTime;
			if ( fStateT <= _template.fBurstCooldown )
			{
				fStateT = 0.0f;
				status = K_WPN_STATUS_READY;
				if ( ammoLeft == 0 )
					status = K_WPN_STATUS_NO_AMMO;
			}
		}
		break;
		case K_WPN_STATUS_NO_AMMO:
		{
			//empty clip sound
			if ( bTriggerDown && !bTriggerDownOld )
			{
				SND_PLAY_POSITIONAL_RAND2( _template.sndidxEmpty, _template.sndidxEmpty2, pOwner->GetPosHeart() );
			}
		}
		break;
		default:
			break;
	}
	
	fTimeSinceShot += dTime;
	// cooldown starts after a while if you stop shooting
	if ( fTimeSinceShot > 0.2f ) 
	{
		dec_limit( fAimErrorFOV, _template.fAimErrorCooldownPerSecond * dTime, 0.0f );
	}
	//#TODO: the reload when shooting should be moved on proper states
	if ( (status != K_WPN_STATUS_RELOADING) && (bReloadDown) && (!bTriggerDown) && (ammoLeft < _template.nClipSize + _template.nBulletChamberSize) )
	{
		SND_PLAY_POSITIONAL_RAND2( _template.sndidxReload, _template.sndidxReload2, pOwner->GetPosHeart() );

		fStateT = 0.0f;
		status = K_WPN_STATUS_RELOADING;
	}

	bTriggerDownOld = bTriggerDown;

	return status;
}

void CWeapon::SetTriggerStates(bool bTriggerPushed, bool bReloadPushed)
{
	//save old state - In Update se egaleaza din nou ca sa apara mesajul doar odata
	bTriggerDownOld = bTriggerDown;
	//set new state
	bTriggerDown = bTriggerPushed;
	//set reload trigger
	bReloadDown = bReloadPushed;
}

void CWeapon::ResetBurst()
{
	if ( status == K_WPN_STATUS_BURST_COOLDOWN )
	{
		if ( _template.bResetFireRateOnTriggerUp )
			fStateT = _template.fBurstCooldown;
	}
}

void CWeapon::StopReloading()
{
	if ( status != K_WPN_STATUS_RELOADING )
		return;

	bReloadDown = false;
	fStateT = 0.0f;
	if ( ammoLeft > 0 )
		status = K_WPN_STATUS_READY;
	else
		status = K_WPN_STATUS_NO_AMMO;
}

void CWeapon::StopShootingCycle()
{
	if ( IsShootingBullet() )
	{
		status = K_WPN_STATUS_READY;
		fStateT = 0.0f;
	}
}

bool CWeapon::IsShootingBullet()
{
	if ( status == K_WPN_STATUS_JUST_SHOT || status == K_WPN_STATUS_CHARGING_UP || status == K_WPN_STATUS_WINDING_DOWN )
		return true;
	// all other states mean that the bullet cycle is not on
	return false;
}

bool CWeapon::IsReadyToShoot()
{
	//#TODO: should account for the owner complying with the conditions (not jumping for example)
	//#TODO: should account for main weapon ammo if consuming from there
	if (( ammoLeft == 0) || (status != K_WPN_STATUS_READY ))
		return false;

	return true;
}


OPRESULT CLevel::LoadWeaponTemplates(WCHAR * xmlPath)
{
	pugi::xml_document doc;
	if (!doc.load_file(xmlPath))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"Unable to load Weapon Templates XML:%s\n", xmlPath);
	}

	///----------------------------------------------------------------------------------
	/// load weapons sprites
	///----------------------------------------------------------------------------------
	 
	int m_libidxWeapons;
	WCHAR wcsPath[ MAX_PATH ];
	WCHAR Path[ MAX_PATH ];
	swprintf_s( wcsPath, MAX_PATH, L"media/levels/data/weapons/%s", doc.root().child( L"WEAPONS" ).attribute( L"file" ).value());
	FileManager::GetMediaPath( wcsPath, Path );
	V_OP_RET( m_sprActors.AddSprites( Path, m_libidxWeapons, K_LIBNICK_WEAPONS ) );

	///----------------------------------------------------------------------------------
	/// LOAD EXPLOSION TEMPLATES
	///----------------------------------------------------------------------------------

	SAFE_DELETE_CArray(m_arrTemplatesExplosion);
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
		templ->eIgnoreActorClass = K_ACT_CLASS_ANY;
		if (!bnode.attribute(L"sIgnoredClass").empty())
			templ->eIgnoreActorClass = (EActorClass)GetListIndexByName(bnode.attribute(L"sIgnoredClass").value(), EActorClassNames, ARRAY_SIZE(EActorClassNames));

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
			templ->cDoT.eExcludedActClass = K_ACT_CLASS_ANY;
			if (!bnode.attribute(L"sDoTIgnoredClass").empty())
				templ->cDoT.eExcludedActClass = (EActorClass)GetListIndexByName(bnode.attribute(L"sDoTIgnoredClass").value(), EActorClassNames, ARRAY_SIZE(EActorClassNames));
			if (!bnode.attribute(L"sDoTClassFilter").empty())
				templ->cDoT.eFilteredActClass = (EActorClass)GetListIndexByName(bnode.attribute(L"sDoTClassFilter").value(), EActorClassNames, ARRAY_SIZE(EActorClassNames));
		}

		m_arrTemplatesExplosion.Add(templ);
	}

	///----------------------------------------------------------------------------------
	/// LOAD WEPAON TEMPLATES
	///----------------------------------------------------------------------------------

	//load weapon templates
	SAFE_DELETE_CArray(m_arrTemplatesWeapon);

	pugi::xml_node rootnode = doc.root().child(L"WEAPONS").child(L"WeaponTemplates");
	for (pugi::xml_node bnode = rootnode.first_child(); bnode; bnode = bnode.next_sibling())
	{
		CWeaponTemplate* templ = new CWeaponTemplate();
		//name
		const WCHAR* bType = bnode.name();
		templ->name.Init(bType);

		//#TODO: type of weapon should be string
		//if (!bnode.attribute(L"nType").empty())
//			templ->eType = (EWeaponType)bnode.attribute(L"nType").as_int();
		templ->bTwoHanded = bnode.attribute( L"twoHanded" ).as_bool();
		templ->bDualWielding = bnode.attribute( L"dualWielding" ).as_bool();
		templ->vMountOffset.x = bnode.attribute( L"mountOffX" ).as_int();
		templ->vMountOffset.y = bnode.attribute( L"mountOffY" ).as_int();

		templ->nHUD_AnimIdx = m_sprInterface.GetAnimationIdxByName( bnode.attribute( L"sHUDanimName" ).value() );
		templ->nHUD_AnimIdxALT = m_sprInterface.GetAnimationIdxByName( bnode.attribute( L"sHUDanimNameIcon" ).value() );
		// alt fire template
		templ->shAltFireTemplate.Init( bnode.attribute( L"altFire" ).value() );
		// get weapons animation idices
		templ->animIdx_reload = -1;
		templ->animIdx_shoot = -1;
		CSpriteLib* pSprWpn = m_sprActors.GetLibByNick( K_LIBNICK_WEAPONS );
		if ( pSprWpn )
		{
			if ( !bnode.attribute( L"animShoot" ).empty() ) {
				templ->animIdx_shoot = pSprWpn->GetAnimationIdxByName( bnode.attribute( L"animShoot" ).value() );
				if ( templ->animIdx_shoot == -1 )
					ErrorBox( K_ERR_WARNING, L"Could not find weapon shoot anim:%s", bnode.attribute( L"animShoot" ).value() );
			}
			if ( !bnode.attribute( L"animReload" ).empty() )
			{
				templ->animIdx_reload = pSprWpn->GetAnimationIdxByName( bnode.attribute( L"animReload" ).value() );
				if ( templ->animIdx_reload == -1 )
					ErrorBox( K_ERR_WARNING, L"Could not find weapon reload anim:%s", bnode.attribute( L"animReload" ).value() );
			}
		}

		if (!bnode.attribute(L"fSpeedPenaltyPercent").empty())
			templ->fSpeedPenaltyPercent = bnode.attribute(L"fSpeedPenaltyPercent").as_float();

		//muzzle flash anim
		templ->nMuzzleFlashAnim = -1;
		/*
		if (!bnode.attribute(L"sMuzzleFlashAnim").empty())
			templ->nMuzzleFlashAnim = m_sprActors.GetAnimationIdxByName(bnode.attribute(L"sMuzzleFlashAnim").value());
			*/
		//template overwrite sTemplateOverwrite - overwrites the actor default template (Adds to it)
		templ->shTemplateOverwrite.Init(bnode.attribute(L"sTemplateOverwrite").value());
		//weapon scripts
		templ->shScript_OnFire.Init(bnode.attribute(L"sScript_OnFire").value());
		templ->shScript_OnFireALT.Init(bnode.attribute(L"sScript_OnFireALT").value());
		templ->shScript_OnEmpty.Init(bnode.attribute(L"sScript_OnEmpty").value());

		{
			///--- bullet data ---
			//strings
			templ->bulletTemplate.nType = K_LVL_BULLET_UNKNOWN;
			if (!bnode.attribute(L"sBulletType").empty())
			{
				templ->bulletTemplate.nType = (EBulletType)GetListIndexByName(bnode.attribute(L"sBulletType").value(), EBulletTypeNames, K_LVL_BULLETS_COUNT);
			}
			//#TODO: bullet  group should be loaded from template
			templ->bulletTemplate.nGroup = K_LVL_BULLGROUP_BULLETS;
			//bullet explosion template hash (at the end of bullet life)
			templ->bulletTemplate.nExploTemplateHash = 0;
			if (!bnode.attribute(L"sBulletExploTemplate").empty())
			{
				templ->bulletTemplate.nExploTemplateHash = FastHash(bnode.attribute(L"sBulletExploTemplate").value());
			}
			//override bullet class
			templ->bulletTemplate.eClass = K_ACT_CLASS_ANY;
			if (!bnode.attribute(L"sBulletClass").empty())
			{
				templ->bulletTemplate.eClass = (EActorClass)GetListIndexByName(bnode.attribute(L"sBulletClass").value(), EActorClassNames, ARRAY_SIZE(EActorClassNames));
			}

			templ->bulletTemplate.fDamage = bnode.attribute(L"fBulletDamage").as_float();
			templ->bulletTemplate.fDamageLossPPx = bnode.attribute(L"fBulletDamageLossPPx").as_float();
			templ->bulletTemplate.fLife = bnode.attribute(L"fBulletLife").as_float();
			templ->bulletTemplate.fStunDuration = bnode.attribute(L"fBulletStunDuration").as_float();
			templ->bulletTemplate.fSpeed_ini = bnode.attribute(L"fBulletSpeed").as_float();
			templ->bulletTemplate.nArmorPiercingRating = bnode.attribute(L"nArmorPiercingRating").as_int();
			templ->bulletTemplate.fDamageObjects = bnode.attribute(L"fBulletDamageObjects").as_float();
			//bullet momentul (minimum not zero)
			templ->bulletTemplate.fMomentum = bnode.attribute(L"fBulletMomentum").as_float();
			if (templ->bulletTemplate.fMomentum == 0.0f)
				templ->bulletTemplate.fMomentum = 0.1f;
			//defaults
			templ->bulletTemplate.fSelfDamageMultiplier = 1.0f;
			if (!bnode.attribute(L"fBulletSelfDamageMultiplier").empty())
				templ->bulletTemplate.fSelfDamageMultiplier = bnode.attribute(L"fBulletSelfDamageMultiplier").as_float();
			templ->bulletTemplate.fCriticalHitChance = 0.0f;
			if (!bnode.attribute(L"fBulletCriticalChance").empty())
				templ->bulletTemplate.fCriticalHitChance = bnode.attribute(L"fBulletCriticalChance").as_float();
			//bullet flags
			templ->bulletTemplate.nFlags = K_LVL_BULLET_FLAG_NONE;
			if (templ->bulletTemplate.fDamageObjects > 0.0f)
				templ->bulletTemplate.nFlags |= K_LVL_BULLET_FLAG_BREAKS_DOORS;
			if (bnode.attribute(L"bBulletIgnoreArmor").as_bool())
				templ->bulletTemplate.nFlags |= K_LVL_BULLET_FLAG_IGNORE_ARMOR;
			if (bnode.attribute(L"bBulletIgnoreCover").as_bool())
				templ->bulletTemplate.nFlags |= K_LVL_BULLET_FLAG_IGNORE_COVER;
			if (bnode.attribute(L"bBulletDieOnImpact").as_bool())
				templ->bulletTemplate.nFlags |= K_LVL_BULLET_FLAG_DIE_ON_IMPACT;
			if (bnode.attribute(L"bBulletCanSplat").as_bool())
				templ->bulletTemplate.nFlags |= K_LVL_BULLET_FLAG_CAN_SPLAT;
			if (bnode.attribute(L"bBulletDirectional").as_bool())
				templ->bulletTemplate.nFlags |= K_LVL_BULLET_FLAG_DIRECTIONAL;
			///--- weapon data ---
			templ->fCooldownT = bnode.attribute(L"fCooldownT").as_float();
			templ->fChargeUpT = bnode.attribute( L"fChargeUpT" ).as_float();
			templ->fWindDownT = bnode.attribute( L"fWindDownT" ).as_float();

			//other constants
			templ->nBulletsPerShot = bnode.attribute(L"nBulletsPerShot").as_int();
			templ->fSpreadFOV = bnode.attribute(L"fSpreadFOV").as_float();
			templ->fAimFOV = bnode.attribute(L"fAimFOV").as_float();
			templ->fAimErrorMaxFOV = bnode.attribute(L"fAimErrorMaxFOV").as_float();
			templ->fAimErrorAddPerShot = bnode.attribute(L"fAimErrorAddPerShot").as_float();
			templ->fAimErrorCooldownPerSecond = bnode.attribute(L"fAimErrorCooldownPerSec").as_float();
			templ->nClipSize = bnode.attribute(L"nClipSize").as_int();
			templ->nReloadUnitSize = bnode.attribute(L"nReloadUnitSize").as_int();
			templ->fReloadTimePerUnit = bnode.attribute(L"fReloadTimePerUnit").as_float();
			templ->bResetFireRateOnTriggerUp = bnode.attribute(L"bCanResetFireRate").as_bool();
			templ->bUsesMainWeaponAmmo = bnode.attribute(L"bUsesMainWeaponAmmo").as_bool();
			templ->bCanShootFromCrouch = bnode.attribute(L"bCanShootFromCrouch").as_bool();
			templ->bCanShootFromCover = bnode.attribute(L"bCanShootFromCover").as_bool();
			templ->nBurstSize = bnode.attribute(L"nBurstSize").as_int();
			templ->fBurstCooldown = bnode.attribute(L"fBurstCooldown").as_float();
			templ->fMuzzleLightSize = bnode.attribute(L"fMuzzleLightSize").as_float();
			templ->bHasLaserSight = bnode.attribute(L"bHasLaserSight").as_bool();
			templ->fSoundRadius = bnode.attribute(L"fSoundRadius").as_float();
			//rectificate
			if (!bnode.attribute(L"fAimErrorMulPerShot").empty())
				templ->fAimErrorMulPerShot = bnode.attribute(L"fAimErrorMulPerShot").as_float();
			if (!bnode.attribute(L"fShooterSpeedSlowingPercent").empty())
				templ->fShooterSpeedSlowingPercent = bnode.attribute(L"fShooterSpeedSlowingPercent").as_float();
			if (!bnode.attribute(L"nDropShellFrame").empty())
				templ->nDropShellFrame = bnode.attribute(L"nDropShellFrame").as_int();

			templ->nBulletChamberSize = 0;
			if (!bnode.attribute(L"bBulletChamber").empty())
				templ->nBulletChamberSize = (bnode.attribute(L"bBulletChamber").as_bool() == true) ? 1 : 0;

			//actor verses for the bullet
			if (!bnode.attribute(L"sActorShootVerse").empty())
				templ->sndActorVerse = (EActorSoundVerse)GetListIndexByName(bnode.attribute(L"sActorShootVerse").value(), EActorSoundVerseNames, EActorSoundVerse::K_LVL_ACT_VERSES_COUNT);

			//sounds
			/*
			if(!bnode.attribute(L"sSndShoot").empty())
				templ->sndidxShoot = UTGetSoundManager().getSndIdxW(bnode.attribute(L"sSndShoot").value());
			if (!bnode.attribute(L"sSndReload").empty())
				templ->sndidxReload = UTGetSoundManager().getSndIdxW(bnode.attribute(L"sSndReload").value());
			if (!bnode.attribute(L"sSndEmpty").empty())
				templ->sndidxEmpty = UTGetSoundManager().getSndIdxW(bnode.attribute(L"sSndEmpty").value());
			//alternative sounds
			templ->sndidxShoot2 = templ->sndidxShoot;
			if (!bnode.attribute(L"sSndShoot2").empty())
				templ->sndidxShoot2 = UTGetSoundManager().getSndIdxW(bnode.attribute(L"sSndShoot2").value());
			templ->sndidxReload2 = templ->sndidxReload;
			if (!bnode.attribute(L"sSndReload2").empty())
				templ->sndidxReload2 = UTGetSoundManager().getSndIdxW(bnode.attribute(L"sSndReload2").value());
			templ->sndidxEmpty2 = templ->sndidxEmpty;
			if (!bnode.attribute(L"sSndEmpty2").empty())
				templ->sndidxEmpty2 = UTGetSoundManager().getSndIdxW(bnode.attribute(L"sSndEmpty2").value());
				*/
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
	pWeapon->status = K_WPN_STATUS_READY;
	pWeapon->ammoLeft = pWeapon->_template.nClipSize + pWeapon->_template.nBulletChamberSize;
	// make sure infinite ammo is infinite
	if (pWeapon->_template.nClipSize < 0)
		pWeapon->ammoLeft = -1;

	return pWeapon;
}
