#include "dxstdafx.h"

//saves warnings stack
#pragma warning(push)
//disable warning
//#pragma warning(disable : 4706)  //assignment within conditional expression

///--- AI STATES/FUNCTIONS ---
//possible states - must be in editor/Data/behaviors.txt too
static const CStringHash AI_states[] = {
	///--- GENERIC FUNCTIONS ---
	L"AI_FN_POS_ELLIPSE",
	L"AI_FN_ANG_SIN_TIME",
	L"AI_FN_ALPHA_SIN_TIME", //face alpha intre min si max in fn de sin(t + dt)
	L"AI_FN_GET_TARGET_POS",
	L"AI_FN_GET_TARGET_ANG", //ia unghiul targetului (relativ la cel actual al lui) si pastraza pozitia fata de originea lui
	L"AI_FN_FOLLOW_TARGET_RAIL",
	L"AI_FN_TOUCH_WHEN_SEE_PLAYER", //cand vede playerul in unghiul solid setat din params face touch la target
	///--- LIGHTS ---
	L"AI_FN_LIGHT_FLICKER1", 
	L"AI_FN_LIGHT_ANG_CONE_XZ_TIME", //roteste directia luminii pe un con cu varful pe Y si baza pe XZ (doar luminile au directie 3D si doar cele IES o folosesc)
	///--- TRIGGERS ---
	L"AI_TRIGGER_IN_OUT", //executa script name pe intrare si pe iesire (face touch la target)
	///--- PARTICLE SYSTEM ---
	L"AI_PARTICLES_GENERATOR", //pentru generatoarele de particule
	///--- COLLISION BOXES ---
	L"AI_COLL_FOG_OF_WAR",	//pentru fog of war. Dispare cu alpha cand devine vizibila camera
	L"AI_COLL_BREAKABLE_DOOR", //pentru collShapes care se sparg de la charge si shotgun. va seta automat animatia usii pe cea de distrugere
	L"AI_COLL_BREAKABLE_WINDOW", //pentru collShapes care se sparg de la gloante. va seta automat frame-ul urmator al animatiei
	L"AI_COLL_KILL_ACTORS", //kills actors inside of it
	///--- ACTIVES ---
	L"AI_ACTIVE_SWINGING_FRONTOBJ",	//interactioneaza cu grenada si se balanseaza
	L"AI_ACTIVE_EXPLO_TRAP",	//capcana care explodeaza cand se intersecteaza bboxuul ei cu playerul
	L"AI_ACTIVE_CHECKPOINT",	//AI special pentru checkpoints - verifica intersectia cu personajul si lanseaza script
	L"AI_ACTIVE_TEAM_TELEPORTER_2FRAMES", //AI pentru usile de team teleport optional (pot intra toti sau doar cativa)
	L"AI_ACTIVE_DOORFACE_AUTOCLOSE", //AI pentru usile din fundal care stau deschise cat timp AItimer1>0.0f (ca si TELEPORTER_2FRAMES)
	L"AI_ACTIVE_DOOR_SECTION", //used for section doors (locked or unlocked)

	L"AI_ACTIVE_AMMO_BOX",	//AI pentru ammo boxes
	L"AI_ACTIVE_HEALTH_BOX",	//AI pentru ammo boxes
	L"AI_ACTIVE_BOMB",		//AI pentru bombele ce trebuiesc dezactivate
	L"AI_ACTIVE_ZOMBIE_SPAWNER", //AI for the zombie spawner
	///--- ACTORS ---
	//no ACTOR states (they have special AI class)
};
//Don't forget to add the state in AI_STATE enum too (level.h)

//intoarce index stare AI enum in fn de nume string din editor sau STATE_UNDEFINED daca nu gaseste starea specificata
int GetAIStateByNameHash(UINT32 stateHash)
{
	for (int kk = 0; kk < K_AI_STATES_CNT; kk++)
	{
		if (stateHash == AI_states[kk].textHash)
			return kk;
	}

	if (stateHash != 0)
	{
		ErrorBox(K_ERR_WARNING, L"GetAIByStateHash::Unknown AI state!");
	}

	return K_AI_STATE_UNDEFINED;
}

//used only for the level intro starting verses
static int nIntroVerseState = 0;


HRESULT CLevel::InitActor(CActor * actor, CActorTemplate * actTemplate, D3DXVECTOR2 spawnPos)
{
	if (actTemplate == NULL)
	{
		ErrorBox(K_ERR_CRITICAL, L"Actor template is null for ID:%d!", actor->ID);
		return E_FAIL;
	}
	//copy template data
	actor->templateActor = *actTemplate;
	///!!! DON'T USE actTemplate from now on !!!

	actor->templateActor.FillDefaultValuesIfNotSet();
	RandomizeTemplateActor(&actor->templateActor);
	//save a copy
	actor->templateActor_ini = actor->templateActor;

	actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
	actor->cDamageOverTime.Reset();
	
	actor->nIconType = K_LVL_ACT_ICON_NONE;
	actor->fIconTimer = K_LVL_ACT_ICON_NONE;

	actor->eLastAnimSet = K_LVL_ACT_ANIM_EMPTY;
	actor->eLastAnimSet_feet = K_LVL_ACT_ANIM_EMPTY;

	actor->eLastPlayedVerse = K_LVL_ACT_VERSE_EMPTY;
	actor->fVerseCooldown = 0.0f;

	actor->bOnLadder = false;
	actor->nInteractingState = 0;
	actor->collisionFlags = K_DIRFLAG_NONE;
	actor->fStunTimer = 0.0f;

	actor->nSuspendedFlags = 0;
	actor->fSuspendedTimer = 0.0f;
	actor->bSuspendInput = false;

	actor->nPlayerOrdinal = -1;
	actor->nControllerInstanceID = -1;

	actor->bCrouched = false;
	actor->nRolling = K_STATE_READY;
	actor->pCover = null;
	actor->nTookDamageFrames = 0;
	actor->nLastDamageTakenFromUID = 0;
	actor->nSkinIdx = 0;

	actor->AItimerDecision = K_LVL_AI_DECISION_INTERVAL;
	actor->SetAngle(0.0f);

	actor->bHasCollision = true;
	actor->bHasGravity = true;
	actor->bSkipRender = false;

	actor->vMoveDirN = D3DXVECTOR2(0.0f, 0.0f);

	actor->bReleaseIt = false;
	actor->speed = D3DXVECTOR2(0.0f, 0.0f);
	actor->vSpeedImpulse = D3DXVECTOR2(0.0f, 0.0f);
	actor->vecCamFollowPos = D3DXVECTOR2(0.0f, 0.0f);

	actor->m_sprOverheadIcon.Init(-1, 0.0f, 0.0f);
	//set hue
	byte collvl = 255;
	actor->color_ini = D3DCOLOR_ARGB(255, collvl, collvl, collvl);
	actor->color = actor->color_ini;
	//no closest touchable
	actor->pClosestTouchable = null;

	actor->fLife = actor->templateActor.fLife;
	actor->fArmor = actor->templateActor.fArmor;
	actor->fFOVPercent = actor->templateActor.fFOVpercent;

	Weapon_Init(&actor->weapons[K_LVL_ACT_WEAPON_PRIMARY], actor->templateActor.weaponType.text, actor);
	Weapon_Init(&actor->weapons[K_LVL_ACT_WEAPON_SECONDARY], actor->templateActor.weaponTypeAlt.text, actor);
	Weapon_Init(&actor->weapons[K_LVL_ACT_WEAPON_GEAR], actor->templateActor.weaponTypeGear.text, actor);
	Weapon_Init(&actor->weapons[K_LVL_ACT_WEAPON_MELEE], actor->templateActor.weaponTypeMelee.text, actor);
	//set breach weapon
	Weapon_Init(&actor->weapons[K_LVL_ACT_WEAPON_BREACH], actor->templateActor.weaponTypeBreach.text, actor);
	//clear temp weapons
	actor->weapons[K_LVL_ACT_WEAPON_TEMPORARY].Init();
	actor->weapons[K_LVL_ACT_WEAPON_TEMPORARY_ALT].Init();
	//clear no weapon weapon
	actor->weapons[K_LVL_ACT_WEAPON_NO_WEAPON].Init();

	actor->pCurrentWeapon = &actor->weapons[K_LVL_ACT_WEAPON_PRIMARY];
	//set selected weapons
	actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY] = &actor->weapons[K_LVL_ACT_WEAPON_PRIMARY];
	actor->pSelectedWeapon[K_LVL_ACT_WEAPON_SECONDARY] = &actor->weapons[K_LVL_ACT_WEAPON_SECONDARY];
	actor->pSelectedWeapon[K_LVL_ACT_WEAPON_GEAR] = &actor->weapons[K_LVL_ACT_WEAPON_GEAR];
	actor->pSelectedWeapon[K_LVL_ACT_WEAPON_MELEE] = &actor->weapons[K_LVL_ACT_WEAPON_MELEE];
	actor->pSelectedWeapon[K_LVL_ACT_WEAPON_BREACH] = &actor->weapons[K_LVL_ACT_WEAPON_BREACH];
	//others are null:
	actor->pSelectedWeapon[K_LVL_ACT_WEAPON_TEMPORARY] = null;
	actor->pSelectedWeapon[K_LVL_ACT_WEAPON_TEMPORARY_ALT] = null;
	actor->pSelectedWeapon[K_LVL_ACT_WEAPON_NO_WEAPON] = null;

	SetActorWeaponPerks(actor, &actor->weapons[K_LVL_ACT_WEAPON_PRIMARY]);

	//set position
	actor->pos = spawnPos;
	//actor->lookDirXsign = 1;
	if(actor->templateActor.bComposedAnimation)
		SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_IDLE, K_LVL_ACT_ANIM_FEET_IDLE);
	else
		SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_IDLE);
	//randomize frames (advance for a random period)
	if (m_sprActors.IsLooping(actor->sprite.animationIdx))
	{
		int nTimes = 10 + m_rand.RandInt(60);
		for (int nn = 0; nn < nTimes; nn++)
		{
			actor->sprite.Update(&m_sprActors, 1.0f / 24.0f);
			if (actor->templateActor.bComposedAnimation)
				actor->sprite_feet.Update(&m_sprActors, 1.0f / 24.0f);
		}
	}

	actor->sprite.pos = actor->pos; 
	actor->sprite_feet.pos = actor->pos;
	//setul de animatii selectat
	actor->SetAnimSet(0);
	///--- hitpoints initialization ---
	LoadActorBBoxAndPoints(actor, K_LVL_ACT_ANIM_REF_POSE, 0);
	//set bbox ini
	actor->UpdateBBoxAndPoints();

	//AI-ul clasic se seteaza pe UNDEFINED dar seteaza celelalte variabile pe 0
	SetAI(actor, K_AI_STATE_UNDEFINED, null);
	//seteaza AI-ul pt inference machine din template
	SetActorAIState(actor, actor->templateActor.AItemplate->GetAIStateByName(actor->templateActor.AIdefaultStateName));

	//update all relative data
	actor->SetPos(actor->pos);

	//clear AI input
	actor->m_AIcommands.Reset();
	actor->m_AIsensorInfo.Reset();

	return S_OK;
}


CBulletHitReturnData CLevel::HitActor(CActor* actor, CBullet *pBullet, D3DXVECTOR2* pvProjectileMomentum)
{
	CBulletHitReturnData retData;
 	retData.eMaterial = actor->templateActor.eMaterial;
	retData.bPenetratedShield = false;
	retData.bKilledTarget = false;
	retData.bArmorHit = false;

	if ((actor == null) || (pBullet == null))
	{
		ErrorBox(K_ERR_WARNING, L"CLevel::HitActor invalid params!");
		return retData;
	}

	bool bGoreEnabled = UTGetAppClass().m_Settings.bGoreEnabled;

	float fHitPointsTaken = pBullet->fDamage;
	float fActorInitialLife = actor->fLife;
	//recon targeted enemies die 30% faster
	if (actor->cDamageOverTime.eType == CDamageOverTime::K_LVL_DoT_TARGETED)
	{
		if ((pBullet->actorClass == K_LVL_ACT_CLASS_PLAYER) || (pBullet->actorClass == K_LVL_ACT_CLASS_EXPLOSION))
		{
			//fVar1 contains the actual damage multiplier
			fHitPointsTaken += fHitPointsTaken * actor->cDamageOverTime.fVar1;
		}
	}
	//recon targeted allies take less damage
	if (actor->cDamageOverTime.eType == CDamageOverTime::K_LVL_DoT_TARGETED_ALLY)
	{
		if(pBullet->actorClass == K_LVL_ACT_CLASS_PLAYER)
			fHitPointsTaken -= fHitPointsTaken * 0.5f;
	}
	//#PERK: FORTIFIED - reduced damage for front hits when crouching
	if ((actor->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER) && (actor->bCrouched) && (pvProjectileMomentum != null) &&
		((actor->templateActor.nArmorDir == 0) || (SIGN(actor->lookDirXsign * actor->templateActor.nArmorDir) != SIGN(pvProjectileMomentum->x))) )
	{
		if (g_playerSelScr.IsPerkEnabled(actor->nPlayerOrdinal, &shPerk_FORTIFIED))
			fHitPointsTaken *= 0.5f;
	}

	float fOldLife = actor->fLife;
	if (fOldLife > 0.0f)
	{
		//signal damage made by coloring them in red
		actor->nTookDamageFrames = 4;
	}

	float fBulletLostEnergy = 0.0f;
	float fLifeTaken = 0.0f; //cata viata ia din actor. Se foloseste doar local.
	float fShieldPointsTaken = 0.0f; //shield taken
	if (fHitPointsTaken < 0.0f)	//kill actor command
	{
		fBulletLostEnergy = actor->fLife + actor->fArmor;
		fLifeTaken = fBulletLostEnergy;
		//daca am valoare negativa la hitpoints setam direct viata la valoarea respectiva
		actor->fLife = fHitPointsTaken;
		actor->varAIparams.SetNamedVarINT32(L"nDeathCommand", K_LVL_ACT_DEATHCMD_SPLAT);

		actor->fArmor = 0.0f;
	}
	else
	{
		fLifeTaken = fHitPointsTaken;
		bool bBulletStopped = false;
		//decidere directie shield vs directie projectileMomentum daca avem directie pe shield (sau shield all around)		
		if (((pBullet->nFlags & K_LVL_BULLET_FLAG_IGNORE_ARMOR) == 0) && (pvProjectileMomentum != null) && (actor->fArmor > 0.0f) && 
			((actor->templateActor.nArmorDir == 0) || (SIGN(actor->lookDirXsign * actor->templateActor.nArmorDir) != SIGN(pvProjectileMomentum->x))) ) 
		{
			int nActorAR = actor->templateActor.nArmorRating;
			//melee damage is treated differently
			if (pBullet->nFlags & K_LVL_BULLET_FLAG_MELEE)
			{
				//melee ignores armor usually but if armor hase melee resistance then it takes first from the armor and then from life
				float fDmgToArmor = fHitPointsTaken * actor->templateActor.fArmorMPP;
				fShieldPointsTaken = min(fDmgToArmor, actor->fArmor);
				fLifeTaken = fHitPointsTaken - fShieldPointsTaken;

				bBulletStopped = true;
				retData.bPenetratedShield = true;
				retData.eMaterial = K_LVL_MATERIAL_FLESH;
			}
			else
			{
				if (nActorAR < 0) //special case for human shield (hostage)
				{
					fShieldPointsTaken = min(actor->fArmor, fHitPointsTaken);
					fLifeTaken = max(0.0f, fHitPointsTaken - fShieldPointsTaken);
					bBulletStopped = false;
					retData.eMaterial = K_LVL_MATERIAL_FLESH;
				}
				else if (pBullet->nArmorPiercingRating < nActorAR)
				{
					fLifeTaken = 0.0f;

					float fShieldPerc = max(0.25f, 1.0f - (nActorAR - pBullet->nArmorPiercingRating) * 0.25f);
					fShieldPointsTaken = fHitPointsTaken * fShieldPerc;
					bBulletStopped = true;
					retData.eMaterial = K_LVL_MATERIAL_METAL;
					retData.bArmorHit = true;
				}
				else if (pBullet->nArmorPiercingRating == nActorAR)
				{
					fLifeTaken = 0.0f;
					fShieldPointsTaken = fHitPointsTaken;
					bBulletStopped = true;
					retData.bPenetratedShield = true;
					retData.eMaterial = K_LVL_MATERIAL_METAL;
				}
				else
				{
					float fLifePerc = max(1.0f, ((pBullet->nArmorPiercingRating - nActorAR) * 0.25f));
					fLifeTaken = fHitPointsTaken * fLifePerc;
					fShieldPointsTaken = fHitPointsTaken;
					bBulletStopped = false;
					retData.bPenetratedShield = true;
					retData.eMaterial = K_LVL_MATERIAL_FLESH;
				}
			}
			//#HACK: pentru human shield schimb materialul pe FLESH. Ar trebui sa am si materialul scutului dar nu are rost momentan
			if ((retData.eMaterial == K_LVL_MATERIAL_METAL) && (actor->GetCurrentBehavior() == AI_BEHAVIOR_HUMAN_SHIELD_ATTACK))
				retData.eMaterial = K_LVL_MATERIAL_FLESH;

			//set armor icons (on and off)
			if ((actor->m_sprOverheadIcon.animationIdx == -1) && (actor->fArmor > 0.0f) && (actor->fArmor == actor->templateActor.fArmor))
			{
				actor->m_sprOverheadIcon.setAnimationOnce(ANM_IGM_INTERFACE_SPR_ICON_ARMOR_APPEAR);
			}

			//scade shield points din armor
			actor->fArmor -= fShieldPointsTaken;
			//took too much armor? get extra armor taken from life
			if (actor->fArmor <= 0.0f)
			{
				fLifeTaken += -actor->fArmor;
				actor->fArmor = 0.0f;
				//armor icon off
				if ((actor->templateActor.fArmor > 0.0f) && (actor->templateActor.nArmorRating > 0))
				{
					actor->m_sprOverheadIcon.setAnimationOnce(ANM_IGM_INTERFACE_SPR_ICON_ARMOR_DISAPPEAR);
				}
			}

			//--- calculam energia ramasa in glont ---
			if (bBulletStopped)
			{
				//bullet loses all its energy so it dies
				fBulletLostEnergy = pBullet->fDamage;
			}
			else
			{
				fBulletLostEnergy = fShieldPointsTaken + min(fLifeTaken, max(actor->fLife, 0.0f));
			}
		}
		else //no shield
		{
			//already dead bodies stop bullets
			if ((actor->fLife <= 0.0f) && (actor->GetCurrentBehavior() == AI_BEHAVIOR_DEAD))
				bBulletStopped = true;

			fBulletLostEnergy = fShieldPointsTaken + min(fLifeTaken, max(actor->fLife, 0.0f));
		}
		//when shooting a dead body take a maximum of 10% energy from the bullet
		//daca nu luam energia asta in momentul in care glontul tras se duce in cadavru nu il strapunge si timp de mai multe frames sta pe loc si face zgomot de damage
		if ((actor->fLife <= 0.0f) && (fBulletLostEnergy <= 0.0f))
			fBulletLostEnergy = actor->templateActor.fLife * 0.1f;

		//transmit bullet momentum daca nu sunt under cover
		if ((actor->templateActor.fMass > 0.0f) && (pvProjectileMomentum) && (actor->pCover == null))
		{
			actor->vSpeedImpulse += *pvProjectileMomentum / actor->templateActor.fMass;
		}
		
		//subtract life	if no invincibility
		if (actor->cDamageOverTime.eType == CDamageOverTime::K_LVL_DoT_INVINCIBLE)
			fLifeTaken = 0.0f;

		if (actor->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER)
		{
			float fDecLife = fLifeTaken;
			//#PERK: DIE HARD
			if ((actor->fLife < actor->templateActor.fLife * 0.1f) && (actor->fLife > 0.0f))
			{
				if (g_playerSelScr.IsPerkEnabled(actor->nPlayerOrdinal, &shPerk_DIE_HARD))
					fDecLife = 0.1f * fLifeTaken;
				//without the perk you take half the damage (attenuated die hard for the hack of it)
				else if (actor->fLife < actor->templateActor.fLife * 0.1f)
					fDecLife = 0.5f * fLifeTaken;
			}
			//#PERK: IRON MAN - stopped bullet increases shooter panic
			if (retData.bArmorHit)
			{
				if (g_playerSelScr.IsPerkEnabled(actor->nPlayerOrdinal, &shPerk_IRON_MAN))
				{
					CActor* pShooter = GetActorByUID(pBullet->ownerUID);
					SetActorDoT(pShooter, CDamageOverTime::K_LVL_DoT_INTIMIDATED, 0.5f, 0.0f, K_LVL_ACT_CLASS_PLAYER, K_LVL_ACT_CLASS_HUMAN, actor->GetUID());
				}
			}

			//#HACK: when rolling only take half the damage
			if (actor->nRolling == K_STATE_EXECUTING)
			{
				fDecLife = 0.5f * fLifeTaken;
				//#PERK: EVASION - fbi lower damage when rolling
				if (g_playerSelScr.IsPerkEnabled(actor->nPlayerOrdinal, &shPerk_EVASION))
					fDecLife = 0.35f * fLifeTaken;
			}

#if defined(ENABLE_PLAYER_INVINCIBILITY)
			fDecLife = 0.0f;
#endif

			actor->fLife -= fDecLife;
			//analytics
			m_arrStats[K_LVL_STATS_PL1_DAMAGE_TAKEN + actor->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT] += (int)ceil(fDecLife);
		}
		else
		{
			actor->fLife -= fLifeTaken;
			

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
			//LOG(L"--> Damaged %s: fLifeTaken:%.2f fArmorTaken:%.2f(AR:%d) bIgnoreArmor:%d <--", actor->templateActor.shName.text, fLifeTaken, fShieldPointsTaken, actor->templateActor.nArmorRating, (pBullet->nFlags & K_LVL_BULLET_FLAG_IGNORE_ARMOR));
#endif
		}
		//save last damager UID
		actor->nLastDamageTakenFromUID = pBullet->ownerUID;
		//if he's still alive and you took enough of it's life say verse
		if ((actor->fLife > 0.0f) && (fLifeTaken >= actor->templateActor.fLife * 0.1f))
			PlayActorSoundVerse(actor, K_LVL_ACT_VERSE_TAKING_DAMAGE, true);

		//life left in it?
		if (actor->fLife > 0.0f)
		{
			//mesaj LOW_HEALTH - la 10% din viata originala
			float fLifeLowLimit = actor->templateActor.fLife * 0.1f;
			if ((actor->fLife < fLifeLowLimit) && (actor->fLife + fLifeTaken >= fLifeLowLimit))
			{
				AddAIEvent(K_LVL_AI_EVENT_LOW_HEALTH, 0, pBullet->actorClass, actor->posHeart, 10000.0f, 0.6f, actor->GetUID());
			}

			//adaugam si stun
			if (actor->fStunTimer < pBullet->fStunDuration)
			{
				SetActorStun(actor, pBullet->fStunDuration);
			}
		}
	}

	//player hit
	if ((actor->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER) && (fActorInitialLife > 0.0f))
	{
		//only on non networked players
		if (!IsNetworkPlayer(actor))
		{
			float fDmgPerc = 0.0f;
			if (fLifeTaken > 0.0f) fDmgPerc += 0.4f;
			if (fShieldPointsTaken > 0.0f) fDmgPerc += 0.2f;
			m_screenVignetteDamage.Init(0.32f, 0xffff0000, 0.0f, 0.32f, fDmgPerc);
		}
	}
	else if ((actor->templateActor.actorClass >= K_LVL_ACT_CLASS_HUMAN) && 
			((pBullet->actorClass == K_LVL_ACT_CLASS_PLAYER) || (pBullet->actorClass == K_LVL_ACT_CLASS_EXPLOSION)) )
	{
		if ((actor->fLife <= 0.0f) && (fOldLife > 0.0f))
		{
			//daca moare inamicul pun frag pentru playerul care a lovit
			CActor* pPlayer = GetPlayerByUID(pBullet->ownerUID);
			if (pPlayer)
			{
				//only count actors that give you strategic points
				if (actor->templateActor.fStrategicPoints > 0.0f)
					m_arrStats[K_LVL_STATS_PL1_KILLS + pPlayer->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT]++;
				
				if ((g_playerSelScr.m_arrPlayers[pPlayer->nPlayerOrdinal].eType == K_PSS_CLASS_BREACHER) && (pBullet->fDamageLossPPx > 0.0f))
				{
					//#PERK: TWIN SHOT - shotguns get inc ROF after hit from close quarters
					if (g_playerSelScr.IsPerkEnabled(pPlayer->nPlayerOrdinal, &shPerk_TWIN_SHOT))
					{
						pPlayer->pCurrentWeapon->m_activePerk.fROF_percAdd = -0.3f;
						pPlayer->pCurrentWeapon->m_activePerk.nDurationShots = 1;
						pPlayer->pCurrentWeapon->m_activePerk.bEnabled = true;
					}
					//#PERK: KILLING SPREE - increase damage for next shot
					if (g_playerSelScr.IsPerkEnabled(pPlayer->nPlayerOrdinal, &shPerk_KILLING_SPREE))
					{
						pPlayer->pCurrentWeapon->m_activePerk.fDamage_percAdd = 0.25f;
						pPlayer->pCurrentWeapon->m_activePerk.nDurationShots = 1;
						pPlayer->pCurrentWeapon->m_activePerk.bEnabled = true;
					}
				}

			}
		}
	}

	//event got_hit
	if ((actor->fLife > 0.0f) && (actor->templateActor.actorClass > K_LVL_ACT_CLASS_PLAYER))
	{
		//adaug eventuri de GOT_HIT doar pe clasele HUMAN, cand sunt lovite de catre player
		//find shooter pos. defaults on pos based on bullet speed
		D3DXVECTOR2 evtpos = actor->posHeart;
		if (pvProjectileMomentum != null)
			evtpos -= *pvProjectileMomentum;

		CActor* pPlayer = GetPlayerByUID(pBullet->ownerUID);
		if (pPlayer)
			evtpos = pPlayer->posHeart;
		
		//only add "got hit" events for enemy classes
		if (pBullet->actorClass >= K_LVL_ACT_CLASS_EXPLOSION)
		{
			AddAIEvent(K_LVL_AI_EVENT_GOT_HIT, pBullet->ownerUID, pBullet->actorClass, evtpos, -1.0f, 1.2f, actor->GetUID());
		}
	}

	//set dead AI on humans
	if ((actor->fLife <= 0.0f) && (actor->templateActor.eMaterial == K_LVL_MATERIAL_FLESH))
	{
		//give strategic points on death
		if ((fOldLife > 0.0f) && (actor->templateActor.actorClass >= K_LVL_ACT_CLASS_HUMAN))
		{
			//you get points if enemy killed by player or explo
			if (((pBullet->actorClass == K_LVL_ACT_CLASS_PLAYER) || (pBullet->actorClass == K_LVL_ACT_CLASS_EXPLOSION)) && (actor->templateActor.actorClass != K_LVL_ACT_CLASS_PLAYER))
			{
				if (actor->UID != pBullet->ownerUID)
				{
					//on infinite mode don't give SP points for enemies
					if (g_gameMode != GAME_MODE_INFINITE_TOWER)
						GiveStrategicPoints(actor->templateActor.fStrategicPoints, &D3DXVECTOR2(actor->bbox.vCenter.x, actor->bbox.vMin.y));
				}
			}
		}

		//cadavers get pushed more by kicking them
		if ((fOldLife > 0.0f) && (actor->templateActor.fMass > 0.0f) && (pvProjectileMomentum != null))
			actor->vSpeedImpulse += K_LVL_DEAD_BODY_BULLET_MOMENTUM_MULTIPLIER * (*pvProjectileMomentum / actor->templateActor.fMass);

		//erase shooting flags
		actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;

		bool bSplatActor = false;
		
		//very low life from the first hit? splat!
		if ((pBullet->nFlags & K_LVL_BULLET_FLAG_CAN_SPLAT) && (actor->GetCurrentBehavior() != AI_BEHAVIOR_DEAD) && (pBullet->actorClass == K_LVL_ACT_CLASS_PLAYER) && (actor->fLife < -actor->templateActor.fLife * 0.5f))
		{
			bSplatActor = true;
			//if bullets lose power then only splat from close quarters
			if ((pBullet->fDamageLossPPx > 0.0f) && ((pBullet->fLife / pBullet->fLife_ini) < 0.9f))
				bSplatActor = false;
		}
		//grenades splat dead bodies
		if ((actor->GetCurrentBehavior() == AI_BEHAVIOR_DEAD) && (pBullet->actorClass == K_LVL_ACT_CLASS_EXPLOSION) && (fLifeTaken >= actor->templateActor.fLife))
				bSplatActor = true;
		//if dead but you keep kicking him it explodes
		if ((actor->GetCurrentBehavior() == AI_BEHAVIOR_DEAD) && (pBullet->nFlags & K_LVL_BULLET_FLAG_CAN_SPLAT) && (actor->fLife < -actor->templateActor.fLife))
			bSplatActor = true;
		//if lucky cancel splat
		if (m_rand.RandInt(100) <= 10)
		{
			bSplatActor = false;
			actor->fLife = 0.0f;
		}

		//--- generate blood splats on death ---
		if ((fOldLife > 0.0f) && (actor->templateActor.eMaterial == K_LVL_MATERIAL_FLESH))
		{
			//splaturile sunt sortate in fn de marime (folosesc posHeart in log de GetPosHeart() pentru ca altfel imi da deja pozitia de dupa moarte, adica prea jos)
			//splaturile sunt sortate in functie de dimensiune (crescator)
			if (bGoreEnabled)
			{
				if ((pBullet->nFlags & K_LVL_BULLET_FLAG_NO_DECALS) == 0)
				{
					AddDecal_BloodSplat(actor->posHeart, true, actor->templateActor.actorClass);
				}
			}

			retData.bKilledTarget = true;
			//say shooter verse
			CActor* pShooter = GetActorByUID(pBullet->ownerUID);
			if (pShooter != null)
			{
				PlayActorSoundVerse(pShooter, K_LVL_ACT_VERSE_KILL_MADE);
			}
		}

		//wear explosive vest? if damage is important the explode it too
		if ((actor->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_HAS_EXPLOSIVE_VEST) && (fLifeTaken > 10.0f))
		{
			if (pBullet->actorClass == K_LVL_ACT_CLASS_EXPLOSION)
			{
				//suicide?
				if (pBullet->ownerUID != actor->GetUID())
				{
					//add explosion
					actor->varAIparams.SetNamedVarUINT32(L"nExplode", hash_EXPLO_BLOWUP_VEST);
				}
				//remove vest flags
				actor->templateActor.eCaps &= ~CActorTemplate::K_ACT_CAPS_HAS_EXPLOSIVE_VEST;
				actor->templateActor.eCaps &= ~CActorTemplate::K_ACT_CAPS_CAN_BE_DETONATED;
				//command splat!
				if (actor->templateActor.actorClass != K_LVL_ACT_CLASS_PLAYER)
					actor->varAIparams.SetNamedVarINT32(L"nDeathCommand", K_LVL_ACT_DEATHCMD_SPLAT);
				//early exit
				retData.fPointsTaken = fBulletLostEnergy;
				return retData;
			}
		}

		//set splat command
		if (bSplatActor)
		{
			actor->varAIparams.SetNamedVarINT32(L"nDeathCommand", K_LVL_ACT_DEATHCMD_SPLAT);
		}
	}

	//daca are coliziuni laterale anulez impulsul ca sa nu intre prin geometrie
	if (((actor->collisionFlags & K_DIRFLAG_RIGHT) && (actor->vSpeedImpulse.x > 0.0f)) ||
		((actor->collisionFlags & K_DIRFLAG_LEFT) && (actor->vSpeedImpulse.x < 0.0f)))
	{
		actor->vSpeedImpulse.x = 0.0f;
	}

	retData.fPointsTaken = fBulletLostEnergy;
	return retData;
}

CBulletHitReturnData CLevel::HitActor(CActor * actor, float fDamage, UINT32 dwOwnerUID, EActorClass eOwnerClass, D3DXVECTOR2 *vDir /*= null*/, UINT32 dwBulletFlags /*= 0*/, int nArmorPiercingRating /*= 100*/, float fStunDuration /*= 0.0f*/)
{
	CBullet bullet;

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	//LOG(L"-Damaged %s with %.2f", actor->templateActor.shName.text, fDamage);
#endif

	bullet.fDamage = fDamage;
	bullet.ownerUID = dwOwnerUID;
	bullet.actorClass = eOwnerClass;
	bullet.nFlags = dwBulletFlags;
	//always pierce armor (by default)
	bullet.nArmorPiercingRating = nArmorPiercingRating;
	bullet.fStunDuration = fStunDuration;

	return HitActor(actor, &bullet, vDir);
}

/*
* Sets STUN timer
*/
void CLevel::SetActorStun(CActor* actor, float fStunDuration)
{
	if ((actor->templateActor.actorClass != K_LVL_ACT_CLASS_HUMAN) && (actor->templateActor.actorClass != K_LVL_ACT_CLASS_FRIENDLY))
		return;

	if ((actor->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET) != 0)
		return;

	//some don't get stunned
	if ((actor->GetCurrentBehavior() == AI_BEHAVIOR_SHIELDBOSS_ATTACK) || (actor->GetCurrentBehavior() == AI_BEHAVIOR_TATTOOBOSS_ATTACK))
		return;

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	//LOG(L"- Stunned %s for %.4f", actor->templateActor.shName.text, fStunDuration);
#endif

	if (fStunDuration > actor->fStunTimer)
	{
		actor->fStunTimer = fStunDuration;
	}

	bool bInterrupting = false;
	//reset actions
	if (actor->fStunTimer >= K_LVL_MIN_STUN_DIZZY_DURATION)
	{
		StopReloadingWeapon(actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]);
		bInterrupting = true;

		//only count stunned enemies
		if (actor->templateActor.actorClass == K_LVL_ACT_CLASS_HUMAN)
			App_IncreaseGamestat(K_MEMID_GAMESTATS_ENEMIES_STUNNED);
	}
	//reset actions
	actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
	//blochez arma principala la orice fel de stun (ca sa nu traga nici cand are stun mic)
	JamWeapon(actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]);
	//stop moving
	if (actor->collisionFlags & K_DIRFLAG_DOWN)
		actor->speed.x = 0.0f;

	//custom stun responses
	/*
	switch (actor->GetCurrentBehavior())
	{
		//don't stun cadavers
		case AI_BEHAVIOR_DEAD:
		{
			actor->fStunTimer = 0.0f;
		}
		break;
	}
	*/
}


/* 
* Executes a melee blow and returns the number of actors you've hit
* \param fDamageObjects - >0.0f to damage doors and windows
*/	
int CLevel::MeleeBlow(int nBulletType, D3DXVECTOR2 vPos, D3DXVECTOR2 vDirection, UINT32 nOwnerUID, int nOwnerClass, float fRange, float fDamageActors, float fImpulse, float fStunDurationMax, EActorClass eIgnoredClass, float fRangeObjects, float fDamageObjects)
{
	//MeleeHit
	CFixedArray<CActor*, 50> arrAffectedActors;
	CActor* pClosestActor = null;
	CActor* pClosestActorAlive = null;
	float fMinDistSq = 100000.0f;
	float fMinDistSqAlive = 100000.0f;

	D3DXVECTOR2 vDirN;
	D3DXVec2Normalize(&vDirN, &vDirection);
	//get shooter bbox
	CActor* pShooter = GetActorByUID(nOwnerUID);
	///--- check doors and windows vs melee ---
	if (fDamageObjects > 0.0f)
	{
		//coliziunea cu nivelul
		D3DXVECTOR2 collisionPoint, collisionNormal;
		D3DXVECTOR2 vEnd = (vPos + vDirN * fRangeObjects);
		CCollisionShape* colShape = ColShape_Segment_Intersection_Arr(vPos, vEnd, m_visibleList.logic_colShapesExtended.m_pData, m_visibleList.logic_colShapesExtended.Count(), &collisionPoint, &collisionNormal);
		if (colShape != null)
		{
			switch (colShape->AIstate)
			{
				case K_AI_STATE_COLL_BREAKABLE_WINDOW:
				case K_AI_STATE_COLL_BREAKABLE_DOOR:
				{
					bool bImmune = false;
					//is it reinforced? only the SAW can breach it
					if (nBulletType != K_LVL_BULLET_MELEE_SAW)
					{
						if (colShape->varAIparams.GetVariantByName(L"b_reinforced")->m_asINT32 != 0)
							bImmune = true;
					}
					else
					{
						//generate sparks
						D3DXVECTOR2 dir = collisionNormal;
						dir.y -= 1.0f;
						D3DXVec2Normalize(&dir, &dir);
						for (int kk = 0; kk < 12; kk++)
						{
							g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRESPARK2, true, 2 + randint(2), &collisionPoint, &g_vecGravity, 
								&(D3DXVECTOR2(dir.x + randfloatsgn(0.4f), dir.y + randfloatsgn(0.4f)) * (40.0f + randfloat(20.0f))), 
								0.4f + randfloat(0.4f), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_FRONT_LIGHT, 2.0f);
						}
					}

					//scadem viata
					if (!bImmune)
					{
						colShape->AIfvar1 -= fDamageObjects;
					}
					
					//mark hit
					colShape->AIvarBool1 = true;
					//salvam directia fortei aplicata de glont ca sa stie ca a fost lovita
					colShape->varAIparams.SetNamedVarFloat(L"fForceDirX", vDirN.x * max(1.0f, fImpulse));
					colShape->varAIparams.SetNamedVarINT32(L"bExploded", 0);
					//add sound event on player's side of the door
					D3DXVECTOR2 sndpos1 = D3DXVECTOR2(colShape->bbox.vCenter.x - vDirN.x * (colShape->bbox.vHalfSize.x + 2.0f), colShape->bbox.vCenter.y);
					AddAIEvent(K_LVL_AI_EVENT_SOUND_THREAT, nOwnerUID, nOwnerClass, sndpos1, 200.0f, 1.0f);

					//add events behind door
					sndpos1 = D3DXVECTOR2(colShape->bbox.vCenter.x + vDirN.x * (colShape->bbox.vHalfSize.x + 2.0f), colShape->bbox.vCenter.y);
					AddAIEvent(K_LVL_AI_EVENT_SOUND_THREAT, 0, 0, sndpos1, 200.0f, 1.0f);
					//breaching event behind door
					AddAIEvent(K_LVL_AI_EVENT_SOUND_DOOR_BREACHING, 0, nOwnerClass, sndpos1, 160.0f, 1.0f);
					//shake
					if(nOwnerClass == K_LVL_ACT_CLASS_PLAYER)
						m_camLevel.ShakeScreen(2.0f, 8.0f, &colShape->pos);

				}
				break;
			}
		}
	}

	///--- actors ---
	float fRangeSq = fRange * fRange;
	//find all affected actors
	for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
	{
		CActor* act = m_arrActors[kk];
		D3DXVECTOR2 vTo = act->posHeart - vPos;
		//ignored class
		if (act->templateActor.actorClass == eIgnoredClass)
			continue;
		if ((act->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET) != 0)
			continue;
		//not in front of player and point not in bbox, skip it
		if ((SIGN(vTo.x) != SIGN(vDirection.x)) && (!act->bbox.PointIn(vPos)))
			continue;
		//too far?
		float fDistSq = D3DXVec2LengthSq(&vTo);
		if (fDistSq > fRangeSq)
			continue;
		//too high or too low
		if (pShooter != null)
		{
			if ((pShooter->bbox.vMin.y > act->bbox.vMax.y) || (pShooter->bbox.vMax.y < act->bbox.vMin.y))
				continue;
		}
		//direct line of sight
		if (!IsLineOfSight(act->posHeart, vPos))
			continue;
		//add it to the list
		arrAffectedActors.Add(act);
		//save closest dead or alive
		if (fDistSq < fMinDistSq)
		{
			fMinDistSq = fDistSq;
			pClosestActor = act;
		}
		//closest alive
		if ((fDistSq < fMinDistSqAlive) && (act->fLife > 0.0f))
		{
			fMinDistSqAlive = fDistSq;
			pClosestActorAlive = act;
		}
	}
	//no closest actor alive? get closest dead
	if (pClosestActorAlive == null)
		pClosestActorAlive = pClosestActor;

	//hit and push actors
	if (pClosestActorAlive != null)
	{
		UINT32 unBulFlags = K_LVL_BULLET_FLAG_CAN_SPLAT | K_LVL_BULLET_FLAG_MELEE | K_LVL_BULLET_FLAG_NOT_BALLISTIC;
		for (int kk = 0; kk < arrAffectedActors.Count(); kk++)
		{
			CBulletHitReturnData hitdata;
			CActor* act = arrAffectedActors.m_pData[kk];
			//damage closest actor
			if (act == pClosestActorAlive)
			{
				//#HARDCODE: special melee things for special classes
				//barrel kick
				if (act->GetCurrentBehavior() == AI_BEHAVIOR_BARREL_EXPLODING)
				{
					//around 10 hits breaks it
					hitdata = HitActor(act, fDamageActors * 0.5f, nOwnerUID, (EActorClass)nOwnerClass, &(vDirN * fImpulse * 6.0f), unBulFlags, 5, fStunDurationMax);
				}
				else
				{
					//hit wasn't processed so do it now
					float fDamage = fDamageActors;
					//damage dead bodies more (usually object damage is larger)
					if (act->fLife <= 0.0f)
						fDamage = fDamageObjects;
					
					if (nOwnerClass >= K_LVL_ACT_CLASS_HUMAN)
					{
						//#PERK: offduty fitness
						if ((act->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER) &&
							(g_playerSelScr.m_arrPlayers[act->nPlayerOrdinal].eType == K_PSS_CLASS_OFFDUTYGUY))
						{
							float fMeleeResist = g_playerSelScr.GetUpgradeBarPercent(&g_playerSelScr.m_arrPlayers[act->nPlayerOrdinal], L"O2_FITNESS");
							//remove damage due to armor melee resistance
							fDamage *= (1.0f - fMeleeResist * 0.4f);
						}
						//#PERK: MELEE DEFENSE - melee damage from front takes only 70%
						if ((act->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER) && (SIGN(vDirection.x) != act->lookDirXsign))
							if (g_playerSelScr.IsPerkEnabled(act->nPlayerOrdinal, &shPerk_MELEE_DEFENSE))
								fDamage *= 0.8f;
						//#PERK: COMBATIVES - melee further received for FBI
						if (act->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER)
							if (g_playerSelScr.IsPerkEnabled(act->nPlayerOrdinal, &shPerk_COMBATIVES))
								fDamage *= 0.6f;
					}
					else if (nOwnerClass == K_LVL_ACT_CLASS_PLAYER)
					{
						//#PERK: FEDERAL JUSTICE - bonus for back stabbing
						if (SIGN(vDirection.x) == act->lookDirXsign)
						{
							CActor* ownerAct = GetPlayerByUID(nOwnerUID);
							if (ownerAct != NULL) {
								if (g_playerSelScr.IsPerkEnabled(ownerAct->nPlayerOrdinal, &shPerk_FEDERAL_JUSTICE))
									fDamage *= 1.5f;
							}
						}
					}

					//#TODO: armor piercing ar trebui transmis aici de deasupra prin bullet template
					int nArmorPiercing = 5;

					hitdata = HitActor(act, fDamage, nOwnerUID, (EActorClass)nOwnerClass, &(vDirN * fImpulse), unBulFlags, nArmorPiercing, fStunDurationMax);
					//main melee target gets MELEE AI event
					if ((nOwnerClass > K_LVL_ACT_CLASS_EXPLOSION) && (hitdata.fPointsTaken > 0.0f))
					{
						AddAIEvent(K_LVL_AI_EVENT_GOT_MELEED, nOwnerUID, nOwnerClass, vPos, -1.0f, 1.0f, act->GetUID());
					}
				}
			}
			else
			{
				float fDamage = 1.0f;
				//damage dead bodies more (usually object damage is larger)
				if (act->fLife <= 0.0f)
					fDamage = fDamageObjects;

				hitdata = HitActor(act, fDamage, nOwnerUID, (EActorClass)nOwnerClass, &(vDirN * fImpulse), unBulFlags, 5, fStunDurationMax);
			}

			//-- now play sounds ---
			if ((nOwnerClass >= K_LVL_ACT_CLASS_HUMAN) || (nOwnerClass == K_LVL_ACT_CLASS_PLAYER))
			{
				if (hitdata.eMaterial == K_LVL_MATERIAL_FLESH)
				{
					//SND_PLAY_POSITIONAL_RAND2(SNDIDX_BULLET_HIT_BODY_GENERIC_01, SNDIDX_BULLET_HIT_BODY_GENERIC_02, act->posHeart);
				}
				else if (hitdata.eMaterial == K_LVL_MATERIAL_METAL)
				{
					//SND_PLAY_POSITIONAL_RAND2(SNDIDX_BULLET_HIT_SHIELD_01, SNDIDX_BULLET_HIT_SHIELD_02, act->posHeart);
				}
			}
		}
	}

	
	//number of meleed actors
	return arrAffectedActors.Count();
}

/* \brief Spawns a new player at spawnPos
* Takes all the necessary spawn data from the "Gear selection screen" object. Optimized for DoorKickers.
* \param nPlayerOrdinal - 0-player1 or 1-player2
* \param nAnimset: -1 to skip spawn animation, 0 first animation, 1 second animation
*/
void CLevel::SpawnPlayer(D3DXVECTOR2 spawnPos, int nPlayerOrdinal, int nAnimset)
{
	if ((nPlayerOrdinal < 0) || (nPlayerOrdinal >= K_MAX_PLAYERS_CNT))
	{
		ErrorBox(K_ERR_WARNING, L"SpawnPlayer::Wrong Player Ordinal!");
		return;
	}

	if (m_arrPlayerControllersIIDs[nPlayerOrdinal] < 0)
	{
		ErrorBox(K_ERR_WARNING, L"SpawnPlayer::Invalid controller UID! Probably controller was removed.");
		return;
	}

	//find selected template
	CPlayerSelScr::CPlayerCharSelection* playersel = &g_playerSelScr.m_arrPlayers[nPlayerOrdinal];

	if (playersel->eType == K_PSS_CLASS_NOT_SELECTED)
	{
		ErrorBox(K_ERR_WARNING, L"SpawnPlayer::Player type not selected!");
		return;
	}

	//--- see if the selected weapons requested a template overwrite ---
	CActorTemplate* acttemplate = null;
	acttemplate = GetTemplateActor(EPSSPlayerTypeTemplate[(int)playersel->eType].text);
	if (acttemplate == null)
	{
		ErrorBox(K_ERR_WARNING, L"SpawnPlayer::Template [%s] not found!", EPSSPlayerTypeTemplate[(int)playersel->eType].text);
		return;
	}
	//copy template locally and customize it based on gear selection
	CActorTemplate templateLocal = *acttemplate;
	templateLocal.FillDefaultValuesIfNotSet();
	///--- set weapons and gear ---
	UINT32 namehash = 0;
	//equipment	- add equipment template
	namehash = g_playerSelScr.GetEquipmentTemplateModifierHash(playersel);
	if (namehash != 0)
	{
		///ADD EQUIPMENT TEMPLATE
		CActorTemplate* acttempl = GetTemplateActor(namehash);
		if (acttempl != null)
		{
			templateLocal.AddGenericDataFromTemplate(acttempl);
			templateLocal.OverwriteAnimsFromTemplate(acttempl);
		}
	}
	//gear
	namehash = g_playerSelScr.GetGearNameHash(playersel);
	CWeaponTemplate* wGear = GetTemplateWeapon(namehash);
	if ((wGear != null) && (!wGear->bPassive))
	{
		templateLocal.weaponTypeGear = wGear->name;
	}
	if ((wGear != null) && (!wGear->shTemplateOverwrite.IsEmpty()))
	{
		///ADD GEAR TEMPLATE
		CActorTemplate* acttempl = GetTemplateActor(wGear->shTemplateOverwrite.getHash());
		if (acttempl != null)
		{
			templateLocal.AddGenericDataFromTemplate(acttempl);
			templateLocal.OverwriteAnimsFromTemplate(acttempl);
		}
	}
	
	///LAST! PRIMARY WEAPON TEMPLATE GETS ADDED WHEN CHANGING WEAPONS (equiping main weapon)
	/// We don't add it here so it saves the actor->template_ini without the equipped weapons
	//alt fire from primary weapon
	namehash = g_playerSelScr.GetALTWeaponNameHash(playersel);
	CWeaponTemplate* wPrimaryALT = GetTemplateWeapon(namehash);
	if ((wPrimaryALT != null) && (!wPrimaryALT->bPassive))
	{
		templateLocal.weaponTypeAlt = wPrimaryALT->name;
	}
	//get selected primary weapon
	namehash = g_playerSelScr.GetPrimaryWeaponNameHash(playersel);
	CWeaponTemplate* wPrimary = GetTemplateWeapon(namehash);
	if ((wPrimary != null) && (!wPrimary->bPassive))
	{
		templateLocal.weaponType = wPrimary->name;
	}


	//overwrite player if already there
	if ((pPlayerActor[nPlayerOrdinal] != null) && (pPlayerActor[nPlayerOrdinal]->fLife > 0.0f))
	{
		//change player
		InitActor(pPlayerActor[nPlayerOrdinal], &templateLocal, pPlayerActor[nPlayerOrdinal]->pos);
		//set controller
		pPlayerActor[nPlayerOrdinal]->nPlayerOrdinal = nPlayerOrdinal;
		pPlayerActor[nPlayerOrdinal]->nControllerInstanceID = m_arrPlayerControllersIIDs[nPlayerOrdinal];
	}
	else //spawn new player
	{
		CActor* nact = new CActor();
		InitActor(nact, &templateLocal, spawnPos);
		m_arrActors.Add(nact);

		pPlayerActor[nPlayerOrdinal] = nact;

		nact->lookDirXsign = 1;
		//set angle
		if (nact->lookDirXsign == -1)
			nact->SetAngle(PI);
		else
			nact->SetAngle(0.0f);
		
		//set controller
		pPlayerActor[nPlayerOrdinal]->nPlayerOrdinal = nPlayerOrdinal;
		pPlayerActor[nPlayerOrdinal]->nControllerInstanceID = m_arrPlayerControllersIIDs[nPlayerOrdinal];
	}

	CActor* nact = pPlayerActor[nPlayerOrdinal];
	//save last safe position as spawn position
	m_arrPlayerLastSafePos[nPlayerOrdinal] = spawnPos;

	///UPGRADE BARS
	g_playerSelScr.ApplyUpgradesOnActor(playersel, nact);
	//update backup template
	nact->templateActor_ini = nact->templateActor;

	//run ON_SPAWN script
	if (!templateLocal.shScript_OnSpawn.IsEmpty())
	{
		StartScript(templateLocal.shScript_OnSpawn.getHash(), nact);
	}

	//animate player on spawn (only if told otherwise by nAnimset=-1)
	if (nAnimset >= 0)
	{
		nact->SetAnimSet(nAnimset);
		SetActorAIState(nact, L"JOIN_GAME");
		AddProp_Light(nact->GetPosHeart(), ANM_LIGHTS_SPR_POINT1, 0.5f, 0.1f, 0x8888ff00, 1.0f);

		//set invulnerability
		SetActorDoT(nact, CDamageOverTime::K_LVL_DoT_INVINCIBLE, 2.0f, 0.0f, K_LVL_ACT_CLASS_ANY, K_LVL_ACT_CLASS_ANY, 0);

		//SND_PLAY(SNDIDX_UI_PLAYER_JOIN);
	}

	//set skin
	nact->nSkinIdx = nPlayerOrdinal;

	//set interface pointers
	m_interfaceIGM.Init(&m_sprInterface, pPlayerActor[0], pPlayerActor[1]);
	
	//resetam numarul de puncte strategice si scoatem selectia
	m_arrStats[K_LVL_STATS_PL1_STRATEGIC_POINTS + nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT] = 0;
	m_interfaceIGM.SetStrategicPoints(m_arrStats[K_LVL_STATS_PL1_STRATEGIC_POINTS] / 1000.0f, m_arrStats[K_LVL_STATS_PL2_STRATEGIC_POINTS] / 1000.0f);
	m_interfaceIGM.SetStrategicSelection(nPlayerOrdinal, -1);
	//initialize arrays
	InitializeStrategicAbilities(nPlayerOrdinal);

	//count players again
	m_nPlayers = 0;
	m_nPlayersActive = 0;
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if (pPlayerActor[kk] != null)
		{
			m_nPlayers++;
			//HAS_PLAYED needs to be 0 or 1
			m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED + kk * K_LVL_STATS_PLAYER_STATS_COUNT] = 1;

			EAIBehaviorType curbeh = pPlayerActor[kk]->GetCurrentBehavior();
			if (curbeh != AI_BEHAVIOR_IN_LIMBO)
				m_nPlayersActive++;
		}
	}
}

CActor* CLevel::SpawnActor(D3DXVECTOR2 spawnPos, WCHAR* strTemplateName, int nLookDirSign, CStringHash* shStateOverride)
{
	CActor * nact = new CActor();
	CActorTemplate* ntempl = GetTemplateActor(strTemplateName);
	InitActor(nact, ntempl, spawnPos);

	//setam AI model vechi doar de siguranta
	//SetAI(nact, K_AI_STATE_UNDEFINED, null);
	CAIState* pStateOver = null;
	if (shStateOverride != null)
		pStateOver = nact->templateActor.AItemplate->GetAIStateByName(*shStateOverride);
	//set the override state only if we did find it
	if (pStateOver == null)
	{
		SetActorAIState(nact, nact->templateActor.AItemplate->GetAIStateByName(nact->templateActor.AIdefaultStateName));
	}
	else
	{
		SetActorAIState(nact, pStateOver);
	}

	//destination offsets
	nact->lookDirXsign = m_rand.RandSign();
	if (nLookDirSign != 0)
		nact->lookDirXsign = SIGN(nLookDirSign);

	//set angle after spawning
	if (nact->lookDirXsign == 1)
		nact->SetAngle(0.0f);
	else
		nact->SetAngle(PI);

	nact->ID = GenerateNextID();

	m_arrActors.Add(nact);

	//--- update statistics ---
	bool bCountEnemy = false;
	//add actor as target only if not spawned already dead (DEAD behavior)
	if (nact->GetCurrentBehavior() != AI_BEHAVIOR_DEAD)
	{
		if (nact->templateActor.actorClass == K_LVL_ACT_CLASS_HOSTAGE)
		{
			//add actor as target only if not spawned already dead (DEAD behavior)
			m_arrStats[K_LVL_STATS_TARGETS_TOTAL]++;
			m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]++;
			//forteaza scriptul de save hostage chiar daca nu l-ai setat din editor
			nact->bCanInteract = true;
			if (nact->script_hash.IsEmpty())
				nact->script_hash.Init(L"SAVE_HOSTAGE");
		}
		else if (nact->templateActor.actorClass == K_LVL_ACT_CLASS_HUMAN)
		{
			bCountEnemy = true;
		}
		else if (nact->templateActor.actorClass == K_LVL_ACT_CLASS_ZOMBIE)
		{
			m_arrStats[K_LVL_STATS_ZOMBIES_TOTAL]++;
			bCountEnemy = true;
		}
	}

	if (bCountEnemy)
	{
		m_arrStats[K_LVL_STATS_TARGETS_TOTAL]++;
		m_arrStats[K_LVL_STATS_TARGETS_LEFT]++;
	}

	return nact;
}

CActive* CLevel::SpawnActive(D3DXVECTOR2 spawnPos, int nAnimIdx, int nFrameIdx, int nLayer)
{
	CActive* obj = new CActive();

	obj->ID = GenerateNextID();
	obj->nLayer = nLayer;
	//pozitia
	obj->pos = spawnPos;
	obj->pos_ini = obj->pos;
	//anim
	int animIdx = nAnimIdx;
	int frameIdx = nFrameIdx;
	obj->sprite.Init(animIdx, obj->pos.x, obj->pos.y, frameIdx);
	obj->nAnim_ini = animIdx;
	obj->nFrame_ini = frameIdx;
	obj->color = 0xffffffff;
	obj->sprite.color = obj->color;
	//angle
	obj->fAngle = obj->fAngle_ini = 0.0f;
	//load flags and split
	UINT32 activFlags = 0;
	//flip xy
	obj->flipX = ((activFlags & K_EDITOR_ACTIVE_FLAG_FLIPX) != 0);
	obj->flipY = ((activFlags & K_EDITOR_ACTIVE_FLAG_FLIPY) != 0);
	//animated
	obj->bAnimated = ((activFlags & K_EDITOR_ACTIVE_FLAG_ANIMATED) != 0);
	obj->bReleaseIt = false;
	//cand e animat selecteaza random frame-ul de pornire
	if (obj->bAnimated)
	{
		obj->sprite.currentFrame = m_rand.RandInt(m_sprActives.GetAFramesCnt(obj->sprite.animationIdx));
	}
	//bbox
	RECTXYWH bbox_set = m_sprActives.GetAFrameBBox(animIdx, frameIdx);
	RECTXYWH objbox = m_sprActives.GetAFrameBBox_real(animIdx, frameIdx);
	obj->bbox_ini.Set(objbox);
	obj->bbox_exported_ini.Set(bbox_set);
	//daca e flipat pe X flipez si bbox. Pe Y nu e cazul pt ca se pastreaza in acelasi bbox in paint
	if (obj->flipX)
	{
		obj->bbox_ini.Move(D3DXVECTOR2(-2.0f * obj->bbox_ini.vCenter.x, 0.0f));
		obj->bbox_exported_ini.Move(D3DXVECTOR2(-2.0f * obj->bbox_exported_ini.vCenter.x, 0.0f));
	}
	obj->bbox = obj->bbox_ini;
	obj->bbox.Move(obj->pos);

	obj->bbox_exported = obj->bbox_exported_ini;
	obj->bbox_exported.Move(obj->pos);

	//load logic
	obj->bCanInteract = false;
	obj->bHideInteractIcon = false;
	obj->bStandsOut = false;
	//interact timer
	obj->fTouchDuration = 0.0f;
	//start hidden
	obj->bHidden = obj->bSetHidden = false;

	obj->targetID_ini = -1;
	obj->script_hash.Reset();
	obj->AIstate = K_AI_STATE_UNDEFINED;

	m_arrActives.Add(obj);

	return obj;
}

CLight* CLevel::SpawnLight(D3DXVECTOR3 spawnPos, int nType, int nAnimIdx, DWORD dwColor, float fScale, bool bCastShadows)
{
	CLight *nl = new CLight();
	nl->m_nLightMeshIdx = -1;
	nl->m_nShadowMeshIdx = -1;

	nl->ID = GenerateNextID();
	nl->type = nType;
	nl->fVolumeAlpha = 1.0f;
	nl->fIntensity = 1.0f;
	nl->pos3D = spawnPos;
	//can't be 0.0f - same plane as background
	if (nl->pos3D.z == 0.0f)
		nl->pos3D.z = 0.1f;

	nl->pos = D3DXVECTOR2(nl->pos3D.x, nl->pos3D.y);
	nl->pos_ini = nl->pos;
	//animID
	nl->animID = nAnimIdx;
	if ((nl->animID < 0) && (nl->type != K_LVL_LIGHT_AMBIENTAL))
		ErrorBox(K_ERR_WARNING, L"[WARNING] SpawnLight::Light ID:%d doesn't have animID!!", nl->ID);
	//color
	nl->color = dwColor;
	nl->color_ini = nl->color;
	//get anim bbox
	RECTXYWH rectAnim = m_sprLights.GetAFrameBBox_real(nl->animID, 0);
	D3DXVECTOR2 bbmin, bbmax;
	float fLocalScale = fScale * K_LVL_LIGHTRENDER_BSX_SCALING;
	bbmin.x = nl->pos.x + rectAnim.x * fLocalScale;
	bbmin.y = nl->pos.y + rectAnim.y * fLocalScale;
	bbmax.x = bbmin.x + rectAnim.w * fLocalScale;
	bbmax.y = bbmin.y + rectAnim.h * fLocalScale;
	nl->bbox.Set_Corrected(bbmin, bbmax);
	nl->bbox_ini = nl->bbox;
	nl->bbox_ini.Move(-nl->pos);
	nl->fMaxRadius = max(nl->bbox.vSize.x, nl->bbox.vSize.y);
	//read angle and convert to radians
	nl->fAngle = 0.0f;
	nl->fAngle = DEG_TO_RAD(nl->fAngle);
	nl->fAngle_ini = nl->fAngle;
	//casts shadows
	nl->castShadows = bCastShadows;

	//setam buffer intern in fn de dreptunghiul frame-ului si animatiei ca sa nu le mai calculez pe paint
	switch (nl->type)
	{
		case K_LVL_LIGHT_POINT:
		{
			nl->fVolumeAlpha = 1.0f;
			//coord spotului in planul 0, relativ la lumina
			nl->lCorners[0] = D3DXVECTOR3(nl->bbox.vMin.x - nl->pos.x, nl->bbox.vMin.y - nl->pos.y, 0.0f);
			nl->lCorners[1] = D3DXVECTOR3(nl->bbox.vMax.x - nl->pos.x, nl->bbox.vMin.y - nl->pos.y, 0.0f);
			nl->lCorners[2] = D3DXVECTOR3(nl->bbox.vMin.x - nl->pos.x, nl->bbox.vMax.y - nl->pos.y, 0.0f);
			nl->lCorners[3] = D3DXVECTOR3(nl->bbox.vMax.x - nl->pos.x, nl->bbox.vMax.y - nl->pos.y, 0.0f);
			//coord in textura
			if (nl->animID >= 0)
				nl->lTexRect = m_sprLights.GetModuleRect_TexCoords(nl->animID, 0, 0);
			//daca lumina este descentrata luam distanta maxima de la lumina la colturi si facem bbox-ul maxim in fn de ea
			float d1 = D3DXVec2Length(&D3DXVECTOR2(nl->pos.x - nl->bbox.vMin.x, nl->pos.y - nl->bbox.vMin.y));
			float d2 = max(d1, D3DXVec2Length(&D3DXVECTOR2(nl->pos.x - nl->bbox.vMax.x, nl->pos.y - nl->bbox.vMin.y)));
			float d3 = max(d2, D3DXVec2Length(&D3DXVECTOR2(nl->pos.x - nl->bbox.vMin.x, nl->pos.y - nl->bbox.vMax.y)));
			float dmax = max(d3, D3DXVec2Length(&D3DXVECTOR2(nl->pos.x - nl->bbox.vMax.x, nl->pos.y - nl->bbox.vMax.y)));
			nl->fMaxRadius = dmax;
		}
		break;
		case K_LVL_LIGHT_REALISTIC_IES_OBSOLETE:
		{
			ErrorBox(K_ERR_WARNING, L"[WARNING] SpawnLight:: Illegal light type (IES LIGHT)!");
		}
		break;
		case K_LVL_LIGHT_AMBIENTAL:
		{
			nl->castShadows = false;
			nl->fVolumeAlpha = 0.0f;
			m_colAmbientGlobal = nl->color;
			nl->bbox_ini.Set(D3DXVECTOR2(0.0f, 0.0f), D3DXVECTOR2(0.0f, 0.0f));
			nl->bbox = nl->bbox_ini;
		}
		break;
		case K_LVL_LIGHT_AREA:
		{
			nl->castShadows = false;
			nl->fVolumeAlpha = 0.0f;
			nl->lCorners[0] = D3DXVECTOR3(-nl->bbox.vHalfSize.x, -nl->bbox.vHalfSize.y, 0.0f);
			nl->lCorners[1] = D3DXVECTOR3(nl->bbox.vHalfSize.x, -nl->bbox.vHalfSize.y, 0.0f);
			nl->lCorners[2] = D3DXVECTOR3(-nl->bbox.vHalfSize.x, nl->bbox.vHalfSize.y, 0.0f);
			nl->lCorners[3] = D3DXVECTOR3(nl->bbox.vHalfSize.x, nl->bbox.vHalfSize.y, 0.0f);
			if (nl->animID >= 0)
				nl->lTexRect = m_sprLights.GetModuleRect_TexCoords(nl->animID, 0, 0);
		}
		break;
		case K_LVL_LIGHT_DIRECTIONAL:
		{
			nl->castShadows = false;
			nl->fVolumeAlpha = 0.0f;
			nl->lCorners[0] = D3DXVECTOR3(-nl->bbox.vHalfSize.x, -nl->bbox.vHalfSize.y, 0.0f);
			nl->lCorners[1] = D3DXVECTOR3(nl->bbox.vHalfSize.x, -nl->bbox.vHalfSize.y, 0.0f);
			nl->lCorners[2] = D3DXVECTOR3(-nl->bbox.vHalfSize.x, nl->bbox.vHalfSize.y, 0.0f);
			nl->lCorners[3] = D3DXVECTOR3(nl->bbox.vHalfSize.x, nl->bbox.vHalfSize.y, 0.0f);
			if (nl->animID >= 0)
				nl->lTexRect = m_sprLights.GetModuleRect_TexCoords(nl->animID, 0, 0);
		}
		break;
	}

	//add light and return it
	m_arrLights.Add(nl);
	return nl;
}

int CLevel::GetPowerupPlacingScore(CActive* active, D3DXVECTOR2 vPlacerPos)
{
	//find a new position if necessary
	int nScore = 0;

	//no lign of sight
	if (!IsLineOfSight(vPlacerPos, active->bbox.vCenter))
	{
		nScore -= 100;
		return nScore;
	}

	//is it floating?
	bool bFloating = true;
	CCollisionShape* pCol = GetCollisionShapeAt(active->pos);
	if(pCol != null)
	{
		if ((pCol->type == K_LVL_COLL_TYPE_SOLID) || (pCol->type == K_LVL_COLL_TYPE_BOX) || (pCol->type == K_LVL_COLL_TYPE_LADDER))
		{
			bFloating = false;
		}
	}
	if (bFloating)
	{
		nScore -= 100;
		return nScore;
	}
	//interactible active
	for (int kk = 0; kk < m_visibleList.logic_actives_closeby[K_LVL_LAYER_BACK].nCount; kk++)
	{
		CActive* pActiv = m_visibleList.logic_actives_closeby[K_LVL_LAYER_BACK].m_pData[kk];
		if (!pActiv->bCanInteract)
			continue;
		if (pActiv == active)
			continue;

		if (pActiv->bbox.Intersects(&active->bbox))
			nScore--;
	}
	//mid layer
	for (int kk = 0; kk < m_visibleList.logic_actives_closeby[K_LVL_LAYER_MIDDLE].nCount; kk++)
	{
		CActive* pActiv = m_visibleList.logic_actives_closeby[K_LVL_LAYER_MIDDLE].m_pData[kk];
		if (!pActiv->bCanInteract)
			continue;
		if (pActiv == active)
			continue;

		if (pActiv->bbox.Intersects(&active->bbox))
			nScore--;
	}
	//interactible actors
	for (int kk = 0; kk < m_visibleList.logic_actors_closeby.nCount; kk++)
	{
		CActor* pAct = m_visibleList.logic_actors_closeby.m_pData[kk];
		if (!pAct->bCanInteract)
			continue;

		if (pAct->bbox.Intersects(&active->bbox))
			nScore--;
	}
	//#TODO: check intersection with ladders and walls too

	return nScore;
}

bool CLevel::GetBestSpawningPos(D3DXVECTOR2 * vSpawn_ret, CAABB rectStart, CAABB * rectToAvoid)
{
	if (vSpawn_ret == null)
		return false;

	//#TODO: make sure we don't spawn under an elevator and return false if all spawn positions return under the elevator

	///--- find best spawn position ---
	D3DXVECTOR2 vSpawnFinal(rectStart.vCenter.x, rectStart.vMax.y);
	D3DXVECTOR2 spawnPos = vSpawnFinal;
	//try a few times to the left and right and compute score
	int nPlaceScore = -100000;
	for (int kk = 0; kk < 8; kk++)
	{
		int nScore = 0;
		int offx = ((kk / 2) * (((kk % 2) * 2) - 1)) * K_TILE_HSIZE;
		D3DXVECTOR2 vCheck(spawnPos.x + (float)offx, spawnPos.y);
		CAABB rectCheck = rectStart;
		rectCheck.Move(D3DXVECTOR2((float)offx, 0.0f));
		//deform it a little
		rectCheck.Inflate(4.0f, -2.0f);

		//not direct line of sight? fail
		if (!IsLineOfSight(rectCheck.vCenter, rectStart.vCenter))
			continue;

		//try to avoid the other box
		if ((rectToAvoid != null) && (rectToAvoid->Intersects(&rectCheck)))
			nScore -= 25;

		CCollisionShape* col = null;
		//prefer both feet on ground
		col = GetCollisionShapeAt(D3DXVECTOR2(vCheck.x + 6.0f, vCheck.y + 1.0f));
		if (col == null)
			nScore -= 50;
		col = GetCollisionShapeAt(D3DXVECTOR2(vCheck.x - 6.0f, vCheck.y + 1.0f));
		if (col == null)
			nScore -= 50;
		//prefer not intersecting geometry
		if (ColShape_CAABB_Intersect_Arr(&rectCheck, m_visibleList.logic_colShapes.m_pData, m_visibleList.logic_colShapes.Count()) != null)
			nScore -= 100;

		if (nScore > nPlaceScore)
		{
			nPlaceScore = nScore;
			vSpawnFinal = vCheck;
		}
	}

	//make sure we have both feet on solid ground
	CCollisionShape* col = GetCollisionShapeAt(D3DXVECTOR2(vSpawnFinal.x + 5.0f, vSpawnFinal.y + 1.0f));
	if (col == null)
		col = GetCollisionShapeAt(D3DXVECTOR2(vSpawnFinal.x - 5.0f, vSpawnFinal.y + 1.0f));
	if ((col != null) && (col->type == K_LVL_COLL_TYPE_SOLID))
	{
		//very narrow bbox, center on it
		if (col->bbox.vSize.x < K_TILE_SIZE)
			vSpawnFinal.x = col->bbox.vCenter.x;
		else
		{
			if (vSpawnFinal.x < col->bbox.vMin.x + 5.0f)
				vSpawnFinal.x = col->bbox.vMin.x + 5.0f;
			else if (vSpawnFinal.x > col->bbox.vMax.x - 5.0f)
				vSpawnFinal.x = col->bbox.vMax.x - 5.0f;
		}
	}
	//return position
	*vSpawn_ret = vSpawnFinal;
	return true;
}

bool CLevel::GetIsAreaNeutral(RECTXYWH_F rectArea)
{
	CAABB bbox(rectArea);
	//interactible active
	for (int kk = 0; kk < m_arrActives.GetSize(); kk++)
	{
		CActive* pActiv = m_arrActives[kk];
		if (!pActiv->bCanInteract)
			continue;

		if (pActiv->bbox.Intersects(&bbox))
			return false;
	}
	//interactible actors
	for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
	{
		CActor* pAct = m_arrActors[kk];
		if (!pAct->bCanInteract)
			continue;

		if (pAct->bbox.Intersects(&bbox))
			return false;
	}
	//intersection with bboxes
	for (int kk = 0; kk < m_arrColShapes.GetSize(); kk++)
	{
		CCollisionShape* pColl = m_arrColShapes[kk];
		if ((pColl->type != K_LVL_COLL_TYPE_SOLID) && (pColl->type != K_LVL_COLL_TYPE_LADDER) && (pColl->type != K_LVL_COLL_TYPE_MOVING_PLATFORM))
			continue;

		if (pColl->bbox.Intersects(&bbox))
			return false;
	}

	return true;
}





UINT32 CLevel::GenerateNextID()
{
	m_unLastID++; //last ID always stays on a new ID
	return (m_unLastID - 1);
}

CLevel::CLevel()
{
	m_bLoaded = false;
	m_bOneUpdateDone = false;

	m_dwSyncCheckHash = 0;
	//init render targets
	m_pRenderToSurface = NULL;
	m_pRTTexture = NULL;
	m_pRTSurface = NULL;

	m_pRT_final = NULL;
	m_pRTTexture_final = NULL;
	m_pRTSurface_final = NULL;

	fLocalTimeline = 0.0f;

	tiles = NULL;
	m_pDevice = NULL;
	levelSizeTL.w = levelSizeTL.h = 0;
	tileW = tileH = 0;

	m_levelAABB.Set(0.0f, 0.0f, 0.0f, 0.0f);
	m_levelAABB_TL.Set(0, 0, 0, 0);
	//init visible area
	m_visibleAreaTL.Set(0, 0, 0, 0);
	m_visibleArea.Set(0.0f, 0.0f, 0.0f, 0.0f);
	//bullets
	m_propsLightsMeshIdx = -1;

	//indexuri texturi
	m_tilesTexBaseIdx = -1;
	m_tilesTexNormIdx = -1;
	//water
	m_waterMeshIdx = -1;
	m_waterTexIdx = -1;
	m_waterAnimIdx = -1;
	//fog of war
	m_fogofwarMeshIdx = -1;
	m_bulletsMeshIdx = -1;
	//level states
	m_levelState = K_LVL_STATE_PLAYING;
	m_levelSubState = 0;
	m_levelStateTimer = 0.0f;

	m_nPlayers = 0;
	m_nPlayersActive = 0;
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		pPlayerActor[kk] = null;
		m_arrPlayerControllersIIDs[kk] = -1; //init player controllers array on no controller
		m_arrPlayerSelHotJoin[kk] = -1;
		m_arrPlayerSelStrategic[kk] = -1;
		m_arrPlayerLastSafePos[kk] = D3DXVECTOR2(0.0f, 0.0f);
	}
	//init interfaces
	m_interfaceIGM.Init(&m_sprInterface, pPlayerActor[0], pPlayerActor[1]);
	m_interfaceTextBubble.Init(&UTGetControlsManager().m_sprCol);

	vLastSpawnPoint = D3DXVECTOR2(0.0f, 0.0f);
	m_vCamPosDefault = D3DXVECTOR2(0.0f, 0.0f);
	//time control
	m_fTimeMultiplier = m_fTimeMultiplier_real = 1.0f;
	m_fTimeMultiplierDuration = 0.0f;
}

CLevel::~CLevel()
{
	Release();
}

///------------- TEMPLATES ----------------
//WEAPONS
HRESULT CLevel::LoadWeaponTemplates(WCHAR * xmlPath)
{
	HRESULT hr = S_OK;

	pugi::xml_document doc;
	if (!doc.load_file(xmlPath))
	{
		ErrorBox(K_ERR_CRITICAL, L"Unable to load Weapon Templates XML:%s\n", xmlPath);
		return E_FAIL;
	}

	//load explosion templates
	SAFE_DELETE_GROWABLE_ARRAY(m_arrTemplatesExplosion);
	pugi::xml_node rootnodeexplo = doc.root().child(L"WEAPONRY").child(L"ExplosionTemplates");
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

	pugi::xml_node rootnode = doc.root().child(L"WEAPONRY").child(L"WeaponTemplates");
	for (pugi::xml_node bnode = rootnode.first_child(); bnode; bnode = bnode.next_sibling())
	{
		CWeaponTemplate* templ = new CWeaponTemplate();
		//name
		const WCHAR* bType = bnode.name();
		templ->name.Init(bType);

		//load generic weapon data
		templ->nHUD_AnimIdx = -1;
		if (!bnode.attribute(L"sHUDanimName").empty())
			templ->nHUD_AnimIdx = m_sprInterface.getAnimationIdxByName(bnode.attribute(L"sHUDanimName").value());
		templ->nHUD_AnimIdxALT = -1;
		if (!bnode.attribute(L"sHUDanimNameIcon").empty())
			templ->nHUD_AnimIdxALT = m_sprInterface.getAnimationIdxByName(bnode.attribute(L"sHUDanimNameIcon").value());
		if (!bnode.attribute(L"fSpeedPenaltyPercent").empty())
			templ->fSpeedPenaltyPercent = bnode.attribute(L"fSpeedPenaltyPercent").as_float();
		if (!bnode.attribute(L"bPassive").empty())
			templ->bPassive = bnode.attribute(L"bPassive").as_bool();

		//muzzle flash anim
		templ->nMuzzleFlashAnim = -1;
		if (!bnode.attribute(L"sMuzzleFlashAnim").empty())
			templ->nMuzzleFlashAnim = m_sprActors.getAnimationIdxByName(bnode.attribute(L"sMuzzleFlashAnim").value());
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

				//#TODO: bullet groups should not be hardcoded in this way
				switch (templ->bulletTemplate.nType)
				{
					case K_LVL_BULLET_INVISIBLE:
					case K_LVL_BULLET_DULL:
					case K_LVL_BULLET_CAM_BALL:
					case K_LVL_BULLET_FIRE_JET:
						templ->bulletTemplate.nGroup = K_LVL_BULLGROUP_SPECIAL;
						break;

					case K_LVL_BULLET_GRENADE:
					case K_LVL_BULLET_GRENADE_ROUND:
					case K_LVL_BULLET_FLASHBANG:
					case K_LVL_BULLET_BREACHING_CHARGE:
					case K_LVL_BULLET_MOLOTOV:
					case K_LVL_BULLET_SMOKE_GRENADE:
					case K_LVL_BULLET_GOO:
						templ->bulletTemplate.nGroup = K_LVL_BULLGROUP_THROWABLES;
						break;

					case K_LVL_BULLET_SHOTGUN:
					case K_LVL_BULLET_SHOTGUN_INCENDIARY:
					case K_LVL_BULLET_TRACER1:
					case K_LVL_BULLET_TRACER_AIMED_SHOT:
					case K_LVL_BULLET_TRACER_RECON:
					case K_LVL_BULLET_SHOTGUN_SLUG:
						templ->bulletTemplate.nGroup = K_LVL_BULLGROUP_BULLETS;
						break;

					case K_LVL_BULLET_MELEE:
					case K_LVL_BULLET_MELEE_SAW:
						templ->bulletTemplate.nGroup = K_LVL_BULLGROUP_MELEE;
						break;

					default:
						templ->bulletTemplate.nGroup = K_LVL_BULLGROUP_BULLETS;
						break;

				}
			}
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
			if(!primnode.attribute(L"fBulletSelfDamageMultiplier").empty())
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
			templ->bAnimSync = primnode.attribute(L"bAnimSync").as_bool();
			templ->bCanShootFromCrouch = primnode.attribute(L"bCanShootFromCrouch").as_bool();
			templ->bCanShootFromAir = primnode.attribute(L"bCanShootFromAir").as_bool();
			templ->bCanShootFromCover = primnode.attribute(L"bCanShootFromCover").as_bool();
			templ->bCanShootFromLadders = primnode.attribute(L"bCanShootFromLadders").as_bool();
			templ->nBurstSize = primnode.attribute(L"nBurstSize").as_int();
			templ->fBurstCooldown = primnode.attribute(L"fBurstCooldown").as_float();
			templ->fMuzzleLightSize = primnode.attribute(L"fMuzzleLightSize").as_float();
			templ->bHasLaserSight = primnode.attribute(L"bHasLaserSight").as_bool();
			templ->fJammedDuration = primnode.attribute(L"fJammedDuration").as_float();
			templ->fSoundRadius = primnode.attribute(L"fSoundRadius").as_float();
			//rectificate
			if(!primnode.attribute(L"fAimErrorMulPerShot").empty())
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

	return hr;
}

CWeaponTemplate* CLevel::GetTemplateWeapon(WCHAR * templateName)
{
	UINT32 nameHash = FastHash(templateName);
	for (int kk = 0; kk < m_arrTemplatesWeapon.GetSize(); kk++)
	{
		if (m_arrTemplatesWeapon[kk]->name.getHash() == nameHash)
			return m_arrTemplatesWeapon[kk];
	}

#if defined(_DEBUG) || defined(DEBUG)
	if (wcslen(templateName) > 0)
		ErrorBox(K_ERR_WARNING, L"Weapon template not found! %s", templateName);
#endif

	return NULL;
}

CWeaponTemplate* CLevel::GetTemplateWeapon(DWORD templateNameHash)
{
	for (int kk = 0; kk < m_arrTemplatesWeapon.GetSize(); kk++)
	{
		if (m_arrTemplatesWeapon[kk]->name.getHash() == templateNameHash)
			return m_arrTemplatesWeapon[kk];
	}

#if defined(_DEBUG) || defined(DEBUG)
	if (templateNameHash != 0)
		ErrorBox(K_ERR_WARNING, L"Weapon template (hash) not found!");
#endif

	return NULL;
}

CExplosionTemplate* CLevel::GetTemplateExplosion(UINT32 templateNameHash)
{
	for (int kk = 0; kk < m_arrTemplatesExplosion.GetSize(); kk++)
	{
		if (m_arrTemplatesExplosion[kk]->name.getHash() == templateNameHash)
			return m_arrTemplatesExplosion[kk];
	}

#if defined(_DEBUG) || defined(DEBUG)
	ErrorBox(K_ERR_WARNING, L"Explosion template not found!");
#endif

	return NULL;
}


HRESULT	CLevel::Weapon_Init(CWeapon* pWeapon, WCHAR* weaponTemplateName, CActor* pParent)
{
	if (pWeapon == null)
		return E_FAIL;

	pWeapon->Init();
	CWeaponTemplate* wTempl = GetTemplateWeapon(weaponTemplateName);
	if (wTempl != null)
	{
		//copy data to local weapon template
		pWeapon->WeaponTemplate = *wTempl;
		//signal valid weapon
		pWeapon->status = K_LVL_WPN_STATUS_READY;
		pWeapon->ammoLeft = pWeapon->WeaponTemplate.nClipSize + pWeapon->WeaponTemplate.nBulletChamberSize;
		//make sure infinite ammo is infinite
		if (pWeapon->WeaponTemplate.nClipSize < 0)
			pWeapon->ammoLeft = -1;

		pWeapon->pOwner = pParent;
		if (wTempl->nMuzzleFlashAnim >= 0)
		{
			pWeapon->m_sprMuzzleFlash.Init(wTempl->nMuzzleFlashAnim, 0, 0);
			//ma asigur ca nu se afiseaza
			pWeapon->m_sprMuzzleFlash.animStatus = ANIM_STATUS_FRAMELOCK; 
		}
	}

	return S_OK;
}

HRESULT CLevel::LoadActorTemplates(WCHAR * xmlPath)
{
	HRESULT hr = S_OK;

	pugi::xml_document doc;
	if (!doc.load_file(xmlPath))
	{
		ErrorBox(K_ERR_CRITICAL, L"Unable to load Templates XML:%s\n", xmlPath);
		return E_FAIL;
	}

	SAFE_DELETE_GROWABLE_ARRAY(m_arrTemplatesActor);
	SAFE_DELETE_GROWABLE_ARRAY(m_arrAItemplates);

	//load actor templates
	pugi::xml_node rootnode = doc.root().child(L"ActorTemplates");
	for (pugi::xml_node bnode = rootnode.first_child(); bnode; bnode = bnode.next_sibling())
	{
		CActorTemplate* templ = new CActorTemplate();
		//name
		const WCHAR* bType = bnode.name();
		templ->shName.Init(bType);
		//constants
		if (!bnode.attribute(L"jumpSpeed").empty())			{ templ->jumpSpeed = bnode.attribute(L"jumpSpeed").as_float(); }
		if (!bnode.attribute(L"moveMaxSpeed").empty())		{ templ->moveMaxSpeed = bnode.attribute(L"moveMaxSpeed").as_float(); }
		if (!bnode.attribute(L"moveBackSpeed").empty())		{ templ->moveBackSpeed = bnode.attribute(L"moveBackSpeed").as_float(); }
		if (!bnode.attribute(L"moveMinSpeed").empty())		{ templ->moveMinSpeed = bnode.attribute(L"moveMinSpeed").as_float(); }
		if (!bnode.attribute(L"climbSpeed").empty())		{ templ->climbSpeed = bnode.attribute(L"climbSpeed").as_float(); }
		if (!bnode.attribute(L"distSee").empty())			{ templ->distSee = bnode.attribute(L"distSee").as_float(); }
		if (!bnode.attribute(L"distHear").empty())			{ templ->distHear = bnode.attribute(L"distHear").as_float(); }
		if (!bnode.attribute(L"distAttackMax").empty())		{ templ->distAttackMax = bnode.attribute(L"distAttackMax").as_float(); }
		if (!bnode.attribute(L"distAttackMin").empty())		{ templ->distAttackMin = bnode.attribute(L"distAttackMin").as_float(); }
		if (!bnode.attribute(L"fMass").empty())				{ templ->fMass = bnode.attribute(L"fMass").as_float(); }
		if (!bnode.attribute(L"nHUDPortraitFrame").empty()) { templ->nHUDportraitFrameIdx = bnode.attribute(L"nHUDPortraitFrame").as_int(); }
		templ->fStrategicPoints = 0.0f;
		if (!bnode.attribute(L"fStrategicPoints").empty())	{ templ->fStrategicPoints = bnode.attribute(L"fStrategicPoints").as_float(); }
		//life
		if (!bnode.attribute(L"Life").empty())
			templ->fLife = bnode.attribute(L"Life").as_float();
		if (!bnode.attribute(L"Armor").empty())
			templ->fArmor = bnode.attribute(L"Armor").as_float();
		if (!bnode.attribute(L"fArmorMeleeProtectionPercent").empty())
			templ->fArmorMPP = bnode.attribute(L"fArmorMeleeProtectionPercent").as_float();
		//default values
		if(!bnode.attribute(L"fFOVpercent").empty())
			templ->fFOVpercent = bnode.attribute(L"fFOVpercent").as_float();
		
		if (!bnode.attribute(L"fDexterity").empty())
			templ->fDexterity = bnode.attribute(L"fDexterity").as_float();
		if (!bnode.attribute(L"fRecoilModifier").empty())
			templ->fRecoilModifier = bnode.attribute(L"fRecoilModifier").as_float();

		if (!bnode.attribute(L"nArmorDir").empty())
			templ->nArmorDir = bnode.attribute(L"nArmorDir").as_int();

		if (!bnode.attribute(L"nArmorRating").empty())
			templ->nArmorRating = bnode.attribute(L"nArmorRating").as_float();
		//composed animation
		templ->bComposedAnimation = false;
		if (!bnode.attribute(L"bComposedAnimation").empty())
			templ->bComposedAnimation = bnode.attribute(L"bComposedAnimation").as_bool();
		//caps
		templ->eCaps = 0;
		if (bnode.attribute(L"canJump").as_bool())
			templ->eCaps |= CActorTemplate::K_ACT_CAPS_CAN_JUMP;
		if (bnode.attribute(L"canCrouch").as_bool())
			templ->eCaps |= CActorTemplate::K_ACT_CAPS_CAN_CROUCH;
		if (bnode.attribute(L"canCover").as_bool())
			templ->eCaps |= CActorTemplate::K_ACT_CAPS_CAN_COVER;
		if (bnode.attribute(L"canClimb").as_bool())
			templ->eCaps |= CActorTemplate::K_ACT_CAPS_CAN_CLIMB;
		if (bnode.attribute(L"canInteract").as_bool())
			templ->eCaps |= CActorTemplate::K_ACT_CAPS_CAN_INTERACT;
		if (bnode.attribute(L"canRoll").as_bool())
			templ->eCaps |= CActorTemplate::K_ACT_CAPS_CAN_ROLL;
		if (bnode.attribute(L"canRotateView").as_bool())
			templ->eCaps |= CActorTemplate::K_ACT_CAPS_CAN_ROTATE_VIEW;

		if (bnode.attribute(L"canBeDetonated").as_bool())
			templ->eCaps |= CActorTemplate::K_ACT_CAPS_CAN_BE_DETONATED;
		if (bnode.attribute(L"hasExplosiveVest").as_bool())
			templ->eCaps |= CActorTemplate::K_ACT_CAPS_HAS_EXPLOSIVE_VEST;

		//weapons
		if (!bnode.attribute(L"weapon").empty())
		{
			templ->weaponType.Init(bnode.attribute(L"weapon").value());
		}
		if (!bnode.attribute(L"weaponAlt").empty())
		{
			templ->weaponTypeAlt.Init(bnode.attribute(L"weaponAlt").value());
		}
		if (!bnode.attribute(L"weaponGear").empty())
		{
			templ->weaponTypeGear.Init(bnode.attribute(L"weaponGear").value());
		}
		if (!bnode.attribute(L"weaponMelee").empty())
		{
			templ->weaponTypeMelee.Init(bnode.attribute(L"weaponMelee").value());
		}
		if (!bnode.attribute(L"weaponBreach").empty())
		{
			templ->weaponTypeBreach.Init(bnode.attribute(L"weaponBreach").value());
		}

		///--- actor scripts ---
		if (!bnode.attribute(L"sScript_OnSpawn").empty())
		{
			templ->shScript_OnSpawn.Init(bnode.attribute(L"sScript_OnSpawn").value());
		}

		if (!bnode.attribute(L"AIstate").empty())
		{
			templ->AIdefaultStateName.Init(bnode.attribute(L"AIstate").value());
		}
		if (!bnode.attribute(L"class").empty())
		{
			templ->actorClass = (EActorClass)GetListIndexByName(bnode.attribute(L"class").value(), EActorClassNames, K_LVL_ACT_CLASSES_COUNT);
		}
		if (!bnode.attribute(L"foeClassFilter1").empty())
			templ->foeClassFilter1 = (EActorClass)GetListIndexByName(bnode.attribute(L"foeClassFilter1").value(), EActorClassNames, K_LVL_ACT_CLASSES_COUNT);
		if (!bnode.attribute(L"foeClassFilter2").empty())
			templ->foeClassFilter2 = (EActorClass)GetListIndexByName(bnode.attribute(L"foeClassFilter2").value(), EActorClassNames, K_LVL_ACT_CLASSES_COUNT);
		if (!bnode.attribute(L"sMaterial").empty())
		{
			templ->eMaterial = (EMaterialType)GetListIndexByName(bnode.attribute(L"sMaterial").value(), EMaterialTypeNames, K_LVL_MATERIALS_COUNT);
		}

		//anims
		pugi::xml_node anmnode = bnode.child(L"ANIMS");
		if (anmnode != NULL)
		{
			for (int kk = 0; kk < K_LVL_ACT_ANIMS_CNT; kk++)
			{
				pugi::xml_node nmnode = anmnode.child(EActorAnimNames[kk].text);
				if (nmnode != NULL)
				{
					//main animation
					templ->animIDs[kk][0] = m_sprActors.getAnimationIdxByName(nmnode.attribute(L"set0").value());
					if (templ->animIDs[kk][0] == -1)
					{
						ErrorBox(K_ERR_WARNING, L"Template set0 animation not found!\n%s", nmnode.attribute(L"set0").value());
					}
					//next sets aren't mandatory
					if (!nmnode.attribute(L"set1").empty())
					{
						templ->animIDs[kk][1] = m_sprActors.getAnimationIdxByName(nmnode.attribute(L"set1").value());
						if (templ->animIDs[kk][1] == -1)
						{
							ErrorBox(K_ERR_WARNING, L"Template variation animation not found!\n%s", nmnode.attribute(L"set1").value());
						}
					}
				}
			}
		}

		//sound verses
		pugi::xml_node versenode = bnode.child(L"VERSES");
		if (versenode != NULL)
		{
			for (int kk = 0; kk < K_LVL_ACT_VERSES_COUNT; kk++)
			{
				pugi::xml_node nmnode = versenode.child(EActorSoundVerseNames[kk].text);
				if (nmnode != NULL)
				{
					if (!nmnode.attribute(L"set0").empty())
					{
						/*
						templ->soundIDs[kk][0] = UTGetSoundManager().getSndIdxW(nmnode.attribute(L"set0").value());
						if ((!nmnode.attribute(L"set0").empty()) && (templ->soundIDs[kk][0] == -1))
						{
							ErrorBox(K_ERR_WARNING, L"Template set0 sound not found!\n%s", nmnode.attribute(L"set0").value());
						}
						*/
					}
					//variation
					if (!nmnode.attribute(L"set1").empty())
					{
						/*
						templ->soundIDs[kk][1] = UTGetSoundManager().getSndIdxW(nmnode.attribute(L"set1").value());
						if ((!nmnode.attribute(L"set1").empty()) && (templ->soundIDs[kk][1] == -1))
						{
							ErrorBox(K_ERR_WARNING, L"Template set1 sound not found!\n%s", nmnode.attribute(L"set1").value());
						}
						*/
					}
				}
			}
		}

		//create local AI template copy
		CAITemplate* aitemplate = new CAITemplate();

		//AI ignored events
		pugi::xml_node aiignorenode = bnode.child(L"AI_IGNORE_EVENTS");
		if (aiignorenode != NULL)
		{
			//parcurg nodurile de stari
			for (pugi::xml_node statenode = aiignorenode.first_child(); statenode; statenode = statenode.next_sibling())
			{
				EAIEventType nevttype = (EAIEventType)GetListIndexByName(statenode.attribute(L"type").value(), EAIEventTypeNames, K_LVL_AI_EVENTS_CNT);
				if (nevttype >= 0)
				{
					aitemplate->m_arrIgnoredEvents.Add(nevttype);
				}
			}
		}

		//AI template
		pugi::xml_node ainode = bnode.child(L"AI");
		if (ainode != NULL)
		{
			//parcurg nodurile de stari
			for (pugi::xml_node statenode = ainode.first_child(); statenode; statenode = statenode.next_sibling())
			{
				CAIState * nstate = new CAIState();
				nstate->name.Init(statenode.attribute(L"name").value());
				nstate->nPriority = statenode.attribute(L"nPriority").as_int();
				//read probability and set to 100.0 if missing
				nstate->fProbability = statenode.attribute(L"fProbability").as_float();
				if (nstate->fProbability == 0.0f)
					nstate->fProbability = 100.0f;
				//find triggers
				pugi::xml_node triggersparent = statenode.child(L"TRIGGERING_EVENTS");
				if (triggersparent != null)
				{
					for (pugi::xml_node eventnode = triggersparent.first_child(); eventnode; eventnode = eventnode.next_sibling())
					{
						CStringHash evtTypeStr(eventnode.attribute(L"type").value());
						EAIEventType nevt = K_LVL_AI_EVENT_NONE;
						//trateaza keyword "ANY"
						if (evtTypeStr.textHash == FastHash(L"any"))
							nevt = K_LVL_AI_EVENT_ANY;
						else
							nevt = (EAIEventType)GetListIndexByName(eventnode.attribute(L"type").value(), EAIEventTypeNames, K_LVL_AI_EVENTS_CNT);

						nstate->m_arrTriggeringEventTypes.Add(nevt);
					}
				}
				//find behaviors
				//#TODO: aici ar trebui sa fie un nod de grup de behaviors iar copiii sa contina behaviors, cu probabilitati pe fiecare copil ca sa pot varia AI-ul random
				pugi::xml_node behaviorsparent = statenode.child(L"BEHAVIORS");
				if (behaviorsparent != null)
				{
					for (pugi::xml_node behnode = behaviorsparent.first_child(); behnode; behnode = behnode.next_sibling())
					{
						CAIBehavior nbeh;
						nbeh.nType = (EAIBehaviorType)GetListIndexByName(behnode.attribute(L"name").value(), EAIBehaviorTypeNames, AI_BEHAVIORS_CNT);
						//salvam cativa params generici
						if (!behnode.attribute(L"bCanInterrupt").empty())
							nbeh.bCanInterrupt = behnode.attribute(L"bCanInterrupt").as_bool();
						if (!behnode.attribute(L"bDetectPlatforms").empty())
							nbeh.bDetectPlatforms = behnode.attribute(L"bDetectPlatforms").as_bool();
						if (!behnode.attribute(L"bIgnoreEvents").empty())
							nbeh.bIgnoreEvents = behnode.attribute(L"bIgnoreEvents").as_bool();
						if (!behnode.attribute(L"fBehaviorDuration").empty())
							nbeh.fBehaviorDuration = behnode.attribute(L"fBehaviorDuration").as_float();
						//read all other behavior specific attributes
						for (pugi::xml_attribute_iterator ait = behnode.attributes_begin(); ait != behnode.attributes_end(); ++ait)
						{
							//sarim peste params generici (name, bCanInterrupt)
							if (ait->internal_object() == behnode.attribute(L"name").internal_object())
								continue;
							if (ait->internal_object() == behnode.attribute(L"bCanInterrupt").internal_object())
								continue;
							if (ait->internal_object() == behnode.attribute(L"bDetectPlatforms").internal_object())
								continue;
							if (ait->internal_object() == behnode.attribute(L"bIgnoreEvents").internal_object())
								continue;
							if (ait->internal_object() == behnode.attribute(L"fBehaviorDuration").internal_object())
								continue;

							WCHAR wval[MAX_PATH];
							StringCchCopy(wval, MAX_PATH, ait->value());
							nbeh.m_vcolParams.SetNamedVarAUTO(ait->name(), wval);
						}

						nstate->m_arrBehaviors.Add(nbeh);
					}
				}
				//adaug state-ul al ai template-ul curent alocat
				aitemplate->m_arrStates.Add(nstate);
			}
		}
		//adaug in lista si salvez pointer in template-ul actorului catre template-ul de AI
		m_arrAItemplates.Add(aitemplate);
		templ->AItemplate = aitemplate;
		//finished loading template: adaug tempalte-ul actorului in lista
		m_arrTemplatesActor.Add(templ);
	}

	return hr;
}

HRESULT CLevel::LoadActorBBoxAndPoints(CActor * destAct, EActorAnims eAnim, int nAnimSet /*= 0*/)
{
	int animSet = LIMIT(nAnimSet, 0, K_LVL_ACT_ANIM_MAX_SETS);

	//sunt 3 frames in REF_POSE: idle, crouch, dead; ia mereu din primul set momentan
	int refposeAnim = destAct->templateActor.animIDs[eAnim][animSet];
	int refposeframes = 0;
	if (refposeAnim >= 0)
	{
		refposeframes = m_sprActors.GetAFramesCnt(refposeAnim);
	}
	else
	{
		ErrorBox(K_ERR_WARNING, L"InitActor::actor %s missing REF_POSE animation set %d!", destAct->templateActor.shName.text, animSet);
		return E_INVALIDARG;
	}

	for (int ll = 0; ll < 3; ll++)
	{
		destAct->vecWeapon_abs[ll] = D3DXVECTOR2(0.0f, -1.0f); //setez pe -1 ca sa iasa din podea
		destAct->vecHeart_abs[ll] = D3DXVECTOR2(0.0f, -1.0f);
		destAct->vecGroundCheck_abs[ll] = D3DXVECTOR2(0.0f, 0.0f);
		destAct->stateBBoxes[ll].Set(0.0f, 0.0f, 0.0f, 0.0f);

		if (ll >= refposeframes)
			continue;

		if (refposeAnim >= 0)
		{
			POINTXYZ_INT pt;
			//cautam punct arma
			if (SUCCEEDED(m_sprActors.GetAFrameHitPointFlag(refposeAnim, ll, 0, K_LVL_ACTOR_HITPOINTFLAG_GUNPOS, &pt)))
			{
				destAct->vecWeapon_abs[ll] = D3DXVECTOR2(pt.x, pt.y);
			}
			//cautam punct inima
			if (SUCCEEDED(m_sprActors.GetAFrameHitPointFlag(refposeAnim, ll, 0, K_LVL_ACTOR_HITPOINTFLAG_HEARTPOS, &pt)))
			{
				destAct->vecHeart_abs[ll] = D3DXVECTOR2(pt.x, pt.y);
			}
			//cautam punct ground check
			if (SUCCEEDED(m_sprActors.GetAFrameHitPointFlag(refposeAnim, ll, 0, K_LVL_ACTOR_HITPOINTFLAG_GROUND_SWEEP, &pt)))
			{
				destAct->vecGroundCheck_abs[ll] = D3DXVECTOR2(pt.x, pt.y);
			}
			//BBOX
			RECTXYWH playerbbox = m_sprActors.GetAFrameBBox(refposeAnim, ll);
			destAct->stateBBoxes[ll].Set(playerbbox.x, playerbbox.y, playerbbox.w, playerbbox.h);
		}
	}

	return S_OK;
}

void CLevel::SetActorAnimationOnce(CActor* actor, EActorAnims nAnimType, EActorAnims nAnimTypeFeet, bool bKeepFrame /*= false*/)
{
	//daca cer aceeasi animatie ies direct
	if (actor == null)
		return;

	if (actor->eLastAnimSet != nAnimType)
	{
		//default animation set0 - primul set este obligatoriu
		int anmidx = actor->templateActor.animIDs[nAnimType][0];
		//select animation based on nAnimSet
		int nAnimSet = actor->GetAnimSet();
		if ((nAnimSet < 0) || (nAnimSet >= K_LVL_ACT_ANIM_MAX_SETS)) //random
		{
			//decid animatia efectiva dintre cea principala si cea alternativa
			if ((actor->templateActor.animIDs[nAnimType][1] >= 0) && (m_rand.RandFloat(100.0f) <= 50.0f))
				anmidx = actor->templateActor.animIDs[nAnimType][1];
		}
		else //setul de animatii selectat
		{
			//daca am animatie alternativa o setez altfel ramane seul 0
			if(actor->templateActor.animIDs[nAnimType][nAnimSet] >= 0)
				anmidx = actor->templateActor.animIDs[nAnimType][nAnimSet];
		}

		if (anmidx >= 0)
		{
			if (bKeepFrame)
			{
				actor->sprite.setAnimationOnce_keepFrame(&m_sprActors, anmidx);
			}
			else
			{
				actor->sprite.setAnimationOnce(anmidx);
				//randomizare animatii looping care nu sunt compuse (ostateci, etc)
				if ((!actor->templateActor.bComposedAnimation) && (m_sprActors.IsLooping(anmidx)))
				{
					//LOG(L"SetAnimOnce: %s ID %d to anim %d from %d", actor->templateActor.name.text, actor->ID, nAnimType, actor->eLastAnimSet);
					actor->sprite.currentFrame = m_rand.RandInt(m_sprActors.GetAFramesCnt(anmidx));
				}
			}
		}
		//salvez animatia setata acum
		actor->eLastAnimSet = nAnimType;

		//VERSES by animations
		switch (nAnimType)
		{
			case K_LVL_ACT_ANIM_RELOAD:
			{
				//only say reloading when out of ammo
				if((actor->pCurrentWeapon != null) && (actor->pCurrentWeapon->ammoLeft == 0))
					PlayActorSoundVerse(actor, K_LVL_ACT_VERSE_RELOADING);
			}
			break;
		}

	}
	//la animatia compusa setam feet animation separat
	if (actor->templateActor.bComposedAnimation)
	{
		if (nAnimTypeFeet == K_LVL_ACT_ANIM_EMPTY)
		{
			actor->sprite_feet.animationIdx = -1;
			actor->eLastAnimSet_feet = nAnimTypeFeet;
		}
		else if (actor->eLastAnimSet_feet != nAnimTypeFeet)
		{
			//salvez animatia setata acum
			actor->eLastAnimSet_feet = nAnimTypeFeet;
			//decid animatia efectiva dintre cea principala si cea alternativa
			int anmidxfeet = actor->templateActor.animIDs[nAnimTypeFeet][0];
			//daca am animatie alternativa o alege random
			if ((actor->templateActor.animIDs[nAnimTypeFeet][1] >= 0) && (m_rand.RandFloat(100.0f) <= 50.0f))
				anmidxfeet = actor->templateActor.animIDs[nAnimTypeFeet][1];

			if (anmidxfeet >= 0)
			{
				if (bKeepFrame)
					actor->sprite_feet.setAnimationOnce_keepFrame(&m_sprActors, anmidxfeet);
				else
					actor->sprite_feet.setAnimationOnce(anmidxfeet);
			}
		}
	}

}

EActorAnims CLevel::GetActorAnimationType(CActor* actor)
{
	return actor->eLastAnimSet;
}

void CLevel::PlayActorSoundVerse(CActor* actor, EActorSoundVerse sVerse, bool bPlayIfNotPlayingOnly)
{
	if ((actor == null) || (sVerse == K_LVL_ACT_VERSE_EMPTY))
		return;
	//timeout between same verses
	if ((actor->fVerseCooldown > 0.0f) && (sVerse == actor->eLastPlayedVerse))
		return;

	int nVariation = -1;
	if (actor->templateActor.soundIDs[(int)sVerse][0] >= 0)
		nVariation = 0;
	if (actor->templateActor.soundIDs[(int)sVerse][1] >= 0)
		nVariation = randint(2);

	if (nVariation < 0)
		return;

	//play only once
	if (bPlayIfNotPlayingOnly)
	{
		if (SND_IS_PLAYING(actor->templateActor.soundIDs[(int)sVerse][nVariation]))
			return;
	}
	//actually play the sound
	//play only nearby sounds
	D3DXVECTOR2 vDist(actor->pos.x - m_camLevel.GetCamPos().x, actor->pos.y - m_camLevel.GetCamPos().y);
	if (D3DXVec2Length(&vDist) < K_GAME_HALF_WIDTH * 1.5f)
	{
		SND_PLAY_POSITIONAL(actor->templateActor.soundIDs[(int)sVerse][nVariation], actor->posHeart);
	}

	actor->nLastPlayedVerseSndIdx = actor->templateActor.soundIDs[(int)sVerse][nVariation];
	actor->eLastPlayedVerse = sVerse;
	actor->fVerseCooldown = K_LVL_ACT_VERSES_TIMEOUT;
}

FORCEINLINE bool CLevel::ActorHasAnimation(CActor* actor, EActorAnims nAnimType)
{
	return ((actor->templateActor.animIDs[nAnimType][0] >= 0) || (actor->templateActor.animIDs[nAnimType][1] >= 0));
}

void CLevel::KillActor(CActor * actor, bool bSplatTarget)
{
	CBullet bullet;
	bullet.fDamage = actor->templateActor.fLife;
	bullet.nFlags |= K_LVL_BULLET_FLAG_IGNORE_ARMOR | K_LVL_BULLET_FLAG_IGNORE_COVER | K_LVL_BULLET_FLAG_NO_IMPACT_PARTICLES | K_LVL_BULLET_FLAG_NOT_BALLISTIC | K_LVL_BULLET_FLAG_NO_DECALS;
	bullet.actorClass = K_LVL_ACT_CLASS_TRAP;

	if (bSplatTarget)
	{
		bullet.nFlags |= K_LVL_BULLET_FLAG_CAN_SPLAT;
		bullet.nFlags &= ~K_LVL_BULLET_FLAG_NO_DECALS;
		//very large damage
		bullet.fDamage = -actor->templateActor.fLife;
	}

	HitActor(actor, &bullet);
}

bool CLevel::IsPlatformEnding(CActor* actor, int nDirSign)
{
	if (nDirSign == 0)
		return false;
	//presupunem ca ai-ul de dead nu cauta platform ends si ca pe crouch nu se misca asa ca folosim mereu datele din standing adica vecGroundCheck[0]
	D3DXVECTOR2 vChkPos = actor->pos;
	vChkPos.y += actor->vecGroundCheck_abs[0].y;
	if(nDirSign < 0)
		vChkPos.x -= actor->vecGroundCheck_abs[0].x;
	else
		vChkPos.x += actor->vecGroundCheck_abs[0].x;


	return false;
}

CActorTemplate* CLevel::GetTemplateActor(const WCHAR * templateName)
{
	UINT32 nameHash = FastHash(templateName);
	//get template now
	for (int kk = 0; kk < m_arrTemplatesActor.GetSize(); kk++)
	{
		if (m_arrTemplatesActor[kk]->shName.getHash() == nameHash)
			return m_arrTemplatesActor[kk];
	}
	return NULL;
}

CActorTemplate* CLevel::GetTemplateActor(const DWORD templateNameHash)
{
	if (templateNameHash == 0)
		return NULL;

	for (int kk = 0; kk < m_arrTemplatesActor.GetSize(); kk++)
	{
		if (m_arrTemplatesActor[kk]->shName.getHash() == templateNameHash)
			return m_arrTemplatesActor[kk];
	}

	return NULL;
}

void CLevel::RandomizeTemplateActor(CActorTemplate * actTemplate)
{			   
	//nu randomizam playerul
	if (actTemplate->actorClass == K_LVL_ACT_CLASS_PLAYER)
		return;
	//randomizeaza vitezele cu 10%
	if(actTemplate->moveMaxSpeed > 0.0f)
		actTemplate->moveMaxSpeed += m_rand.RandFloatSgn(actTemplate->moveMaxSpeed * 0.1f);
	if (actTemplate->moveMinSpeed > 0.0f)
		actTemplate->moveMinSpeed += m_rand.RandFloatSgn(actTemplate->moveMinSpeed * 0.1f);
}

///--- IACTIVE ---
IActiveInterface* CLevel::GetIActiveInterfacePtr(int ID)
{
	if (ID < 0)
		return null;
	//check actives
	for (int kk = 0; kk < m_arrActives.GetSize(); kk++)
	{
		if (m_arrActives[kk]->ID == ID)
			return m_arrActives[kk];
	}
	//check lights
	for (int kk = 0; kk < m_arrLights.GetSize(); kk++)
	{
		if (m_arrLights[kk]->ID == ID)
			return m_arrLights[kk];
	}
	//check collision boxes
	for (int kk = 0; kk < m_arrColShapes.GetSize(); kk++)
	{
		if (m_arrColShapes[kk]->ID == ID)
			return m_arrColShapes[kk];
	}
	//verifica si actorii
	for (int kk = 0; kk < m_arrActors.Count(); kk++)
	{
		if (m_arrActors[kk]->ID == ID)
			return m_arrActors[kk];
	}

	return NULL;
}

IActiveInterface* CLevel::GetIActiveInterfacePtr_byUID(UINT32 UID)
{
	if (UID == 0)
		return null;
	//check actives
	for (int kk = 0; kk < m_arrActives.GetSize(); kk++)
	{
		if (m_arrActives[kk]->GetUID() == UID)
			return m_arrActives[kk];
	}
	//verifica si actorii
	for (int kk = 0; kk < m_arrActors.Count(); kk++)
	{
		if (m_arrActors[kk]->GetUID() == UID)
			return m_arrActors[kk];
	}
	//check lights - mai putin probabil
	for (int kk = 0; kk < m_arrLights.GetSize(); kk++)
	{
		if (m_arrLights[kk]->GetUID() == UID)
			return m_arrLights[kk];
	}
	//check collision boxes - foarte putin probabil
	for (int kk = 0; kk < m_arrColShapes.GetSize(); kk++)
	{
		if (m_arrColShapes[kk]->GetUID() == UID)
			return m_arrColShapes[kk];
	}

	return NULL;
}

CActor* CLevel::GetActorByUID(UINT32 UID)
{
	if (UID == 0)
		return NULL;
	for (int kk = 0; kk < m_arrActors.Count(); kk++)
	{
		if (m_arrActors[kk]->GetUID() == UID)
			return m_arrActors[kk];
	}
	return NULL;
}

CLight* CLevel::GetLightByUID(UINT32 UID)
{
	if (UID == 0)
		return NULL;
	for (int kk = 0; kk < m_arrLights.Count(); kk++)
	{
		if (m_arrLights[kk]->GetUID() == UID)
			return m_arrLights[kk];
	}
	return NULL;
}

CActor* CLevel::GetPlayerByUID(UINT32 UID)
{
	if (UID == 0)
		return NULL;
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if (pPlayerActor[kk] == null)
			continue;
		if (pPlayerActor[kk]->UID == UID)
		{
			return pPlayerActor[kk];
		}
	}
	return NULL;
}

CActor* CLevel::GetClosestPlayer(CActor* sourceActor, bool bIgnoreDead)
{
	float fMinDist = 100000.0f;
	CActor* plact = null;
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if (pPlayerActor[kk] == null)
			continue;
		EAIBehaviorType beh = pPlayerActor[kk]->GetCurrentBehavior();
		if ((bIgnoreDead) && (beh == AI_BEHAVIOR_DEAD))
			continue;
		float fDist = D3DXVec2Length(&(pPlayerActor[kk]->pos - sourceActor->pos));
		if (fDist < fMinDist)
		{
			plact = pPlayerActor[kk];
			fMinDist = fDist;
		}
	}
	return plact;
}

CActor* CLevel::GetClosestPlayer(D3DXVECTOR2 vSrcPos, bool bIgnoreDead)
{
	float fMinDist = 100000.0f;
	CActor* plact = null;
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if (pPlayerActor[kk] == null)
			continue;
		if ((bIgnoreDead) && (pPlayerActor[kk]->GetCurrentBehavior() == AI_BEHAVIOR_DEAD))
			continue;
		float fDist = D3DXVec2Length(&(pPlayerActor[kk]->pos - vSrcPos));
		if (fDist < fMinDist)
		{
			plact = pPlayerActor[kk];
			fMinDist = fDist;
		}
	}
	return plact;
}

bool CLevel::IsNetworkPlayer(CActor* pPlayer)
{
	if (pPlayer == null)
		return false;

	return (pPlayer->nControllerInstanceID == K_CM_IID_NET1);
}

CActive* CLevel::GetActiveByUID(UINT32 UID)
{
	if (UID == 0)
		return NULL;
	for (int kk = 0; kk < m_arrActives.Count(); kk++)
	{
		if (m_arrActives[kk]->GetUID() == UID)
			return m_arrActives[kk];
	}
	return NULL;
}

HRESULT CLevel::LoadLevel(WCHAR * strPathAbs)
{
	HRESULT hr = S_OK;
	//offset the level down 
	int nLevelOffsetY_TL = 1000;
	int nLevelOffsetY = 1000 * K_TILE_SIZE;

	//set last ID on a number that will never get reached from the editor
	m_unLastID = 100000; 
	int nLayersCnt = K_LVL_LAYERS_CNT;
	int nChapterNumber = g_userData[K_MEMID_SELECTED_CHAPTER];
	int nLevelNumber = g_userData[K_MEMID_SELECTED_LEVEL];
	//--- set loaded level flags
	//are we loading a downloaded level?
	int nModIdx_SelectedContent = g_userData[K_MEMID_MOD_DWNLVL_SELECTED];
	m_unLoadedLevelFlags = K_LVL_LEVEL_FLAG_NONE;
	if (nModIdx_SelectedContent >= 0)
		m_unLoadedLevelFlags |= K_LVL_LEVEL_FLAG_DOWNLOADED;
	if (UTGetAppClass().IsGameModded())
		m_unLoadedLevelFlags |= K_LVL_LEVEL_FLAG_MODS_ON;
	if (g_gameMode == GAME_MODE_INFINITE_TOWER)
	{
		m_unLoadedLevelFlags |= K_LVL_LEVEL_FLAG_VINFINITE_MODE;
		g_stringsMgr.ReplaceTokenInt(STR_FLOOR_X_VALUE, STR_FLOOR_X, 1, 0);
	}

	WCHAR Path[MAX_PATH] = { 0 };

	if (UTGetAppClass().IsGameNetworked())
	{
		m_rand.SetRandomSeed(g_netlock.m_unRandomSeed);
	}
	else
	{
		//randomize seed
		m_rand.SetRandomSeed(GetTickCount());
	}
	//reset local timeline
	fLocalTimeline = 0.0f;
	vLastSpawnPoint = D3DXVECTOR2(0.0f, 0.0f);

	//realease level if loaded
	Release();
	
	//reset shakes
	m_camLevel.ShakeScreen(0.0f, 0.0f);
	//reset all timers
	m_Timers.ResetTimers();

	m_bLoaded = false;
	m_bOneUpdateDone = false;

	m_dwSyncCheckHash = 0;
	//get rid of all particles
	g_particlesMgr.RemoveAll();
	//--- setari initiale ---
	ResetLevelStatistics();

	m_colAmbientGlobal = 0xffffffff;
	m_fThunderTimer = 0.0f;

	m_waterAnimIdx = -1;
	//team doors
	m_nTeleportSlots = 0;
	m_bTeleportActivated = false;
//	m_bTeleportRequested = false;
	m_bInsideHiddenRoom = false;
	m_bPlayerInHiddenRoom[0] = m_bPlayerInHiddenRoom[1] = false;
	m_pTeleportSource = null;

	m_interfaceIGM.Reset();
	m_interfaceIGM.SetBombTimer(-1.0f);
	//load interface sprites
	FileManager::GetMediaPath(L"media/interfaces/igm_interface.bsx", Path);
	if (FAILED(m_sprInterface.LoadSprites(Path)))
	{
		return E_FAIL;
	}

	//load level
	FILE *fl = NULL;
	int err = OS_wfopen_s(&fl, strPathAbs, L"rb");

	if (fl == NULL || err != 0)
	{
		return E_FAIL;
	}

	//read int array
	//#IMPORTANT: !!!If you change this part make sure you update it in App_ResetUserData() too!!!
	UINT32 arrInts[10];
	OS_fread(arrInts, sizeof(UINT32), 10, fl);
	if (arrInts[0] != K_EDITOR_LEVEL_FILE_FORMAT_VERSION)
	{
		if (arrInts[0] == 1014)
		{
			LOG(L"LoadLevel:: Old level format found [1014]! Loading and converting lights to new format.");
		}
		else if (arrInts[0] < 1014) //last version files didn't have light volumes alpha
		{
			ErrorBox(K_ERR_WARNING, L"[Error] LoadLevel(%s)::Wrong file version found: %d !", strPathAbs, arrInts[0]);
			return E_FAIL;
		}
	}

	//tip misiune
	byte missionType = OS_freadByte(fl);
	//save the level type again if we're playing a standard level
	if (m_unLoadedLevelFlags != K_LVL_LEVEL_FLAG_NONE)
	{
		g_levelStats[nLevelNumber + nChapterNumber * K_GAME_LEVELS_PER_CHAPTER].nLevelType = missionType;
	}
	//tileset name
	CHAR charArr[MAX_PATH];
	WCHAR wcharArr[MAX_PATH];
	WCHAR wcsMediaAddr[MAX_PATH];

	OS_freadString(fl, charArr);
	size_t converted;
	mbstowcs_s(&converted, wcharArr, charArr, MAX_PATH);

	StringCchPrintf(wcsMediaAddr, MAX_PATH, L"media/levels/data/%s", wcharArr);
	FileManager::GetMediaPath(wcsMediaAddr, Path);
	if (FAILED(m_texManager.AddTexture(Path, &m_tilesTexBaseIdx, D3DFMT_A8R8G8B8, D3DX_FILTER_NONE, D3DX_FILTER_NONE)))
	{
		ErrorBox(K_ERR_CRITICAL, L"Couldn't load tileset texture: %s", Path);
		return E_FAIL;
	}
	//#TODO: deletes the last 4 characters (.png) and adds another ending... should be handled differently (from the editor)
	wcsMediaAddr[wcslen(wcsMediaAddr) - 4] = 0;
	StringCchCat(wcsMediaAddr,MAX_PATH, L"_n.png");
	FileManager::GetMediaPath(wcsMediaAddr, Path);
	if (FAILED(m_texManager.AddTexture(Path, &m_tilesTexNormIdx, D3DFMT_A8R8G8B8, D3DX_FILTER_NONE, D3DX_FILTER_NONE)))
	{
		ErrorBox(K_ERR_CRITICAL, L"Couldn't load tileset normals texture: %s", Path);
		return E_FAIL;
	}
	//#TODO: water texture should be loaded from the editor also (not hardcoded)
	FileManager::GetMediaPath(L"media/levels/data/water1_n.png", Path);
	if (FAILED(m_texManager.AddTexture(Path, &m_waterTexIdx, D3DFMT_A8R8G8B8, D3DX_FILTER_NONE, D3DX_FILTER_NONE)))
	{
		ErrorBox(K_ERR_CRITICAL, L"Couldn't load water texture: %s", Path);
		return E_FAIL;
	}

	int tilesetColumns;
	//load tile size
	tileW = OS_freadByte(fl); 
	tileH = OS_freadByte(fl);
	tilesetColumns = OS_freadUInt16(fl);
	//level size
	int levelW = OS_freadUInt16(fl);
	int levelH = OS_freadUInt16(fl);

	//level origin - in pixels
	int originY = OS_freadInt16(fl);
	int originX = OS_freadInt16(fl);

	levelSizeTL.w = levelW;
	levelSizeTL.h = levelH;

	//set level size
	m_levelAABB_TL.Set(0, nLevelOffsetY_TL, levelSizeTL.w, levelSizeTL.h);
	m_levelAABB.Set(m_levelAABB_TL.x * tileW, m_levelAABB_TL.y * tileH, levelSizeTL.w * tileW, levelSizeTL.h * tileH);
	m_vLevelOrigin.x = (float)originX + m_levelAABB.x;
	m_vLevelOrigin.y = (float)originY + m_levelAABB.y;

	tiles = new CTile*[levelSizeTL.w];
	for (int kk = 0; kk < levelSizeTL.w; kk++)
	{
		tiles[kk] = new CTile[levelSizeTL.h];
	}
	//read tiles, not a rare matrix
	for (int yy = 0; yy < levelSizeTL.h; yy++)
	{
		for (int xx = 0; xx < levelSizeTL.w; xx++)
		{
			for (int kk = 0; kk < nLayersCnt; kk++)
			{
				//nivel
				int tileID = OS_freadInt32(fl);
				tiles[xx][yy].tileIDs[kk] = tileID;
				if (tileID >= 0)
				{
					SetRect(&tiles[xx][yy].srcRects[kk], (tileID % tilesetColumns) * tileW, (tileID / tilesetColumns) * tileH, (tileID % tilesetColumns) * tileW + tileW, (tileID / tilesetColumns) * tileH + tileH);
				}
			}
		}
	}

	///--- lights ---
	SAFE_DELETE_GROWABLE_ARRAY(m_arrLights);
	//read path
	OS_freadString(fl, charArr);
	mbstowcs_s(&converted, wcharArr, charArr, MAX_PATH);
	//load bsx
	StringCchPrintf(wcsMediaAddr, MAX_PATH, L"media/levels/data/%s", wcharArr);
	FileManager::GetMediaPath(wcsMediaAddr, Path);
	if (FAILED(m_sprLights.LoadSprites(Path)))
	{
		return E_FAIL;
	}

	int lightsCnt = (int)OS_freadUInt32(fl);
	//date fiecare 
	for (int kk = 0; kk < lightsCnt; kk++)
	{
		CLight *nl = new CLight();
		nl->m_nLightMeshIdx = -1;
		nl->m_nShadowMeshIdx = -1;

		nl->ID = OS_freadUInt32(fl);
		nl->type = OS_freadByte(fl); //tip lumina
		int nVolumeAttenuationPerc = (int)OS_freadUInt32(fl);
		nl->fVolumeAlpha = 1.0f - (float)nVolumeAttenuationPerc / 100.0f;
		nl->fIntensity = OS_freadFloat32(fl);
		CLAMP(nl->fIntensity, 0.0f, 1.0f);
		nl->pos3D.x = (float)OS_freadInt32(fl);
		nl->pos3D.y = (float)OS_freadInt32(fl);
		nl->pos3D.z = (float)OS_freadInt32(fl);
		//z nu are voie sa fie in acelasi plan cu fundalul
		if (nl->pos3D.z == 0.0f)
			nl->pos3D.z = 0.1f;
		nl->pos3D.y += (float)nLevelOffsetY;

		nl->pos_ini = nl->pos = D3DXVECTOR2(nl->pos3D.x, nl->pos3D.y);
		//animID
		CHAR charAnmName[MAX_PATH];
		OS_freadString(fl, charAnmName);

		nl->animID = m_sprLights.getAnimationIdxByName(charAnmName);
		if ((nl->animID < 0) && (nl->type != K_LVL_LIGHT_AMBIENTAL))
			ErrorBox(K_ERR_WARNING, L"Light ID:%d doesn't have animID!!", nl->ID);
		//color
		BYTE ca, cr, cg, cb;
		ca = OS_freadUByte(fl); cr = OS_freadUByte(fl); cg = OS_freadUByte(fl); cb = OS_freadUByte(fl);
		nl->color = D3DCOLOR_ARGB(ca, cr, cg, cb);
		nl->color_ini = nl->color;

		D3DXVECTOR2 bbmin, bbmax;
		bbmin.x = (float)OS_freadInt32(fl);
		bbmin.y = (float)OS_freadInt32(fl);
		bbmin.y += (float)nLevelOffsetY;
		bbmax.x = bbmin.x + (float)OS_freadInt32(fl);
		bbmax.y = bbmin.y + (float)OS_freadInt32(fl);
		//set loaded size (default)
		nl->bbox.Set_Corrected(bbmin, bbmax);
		nl->bbox_ini = nl->bbox;
		nl->bbox_ini.Move(-nl->pos);
		nl->fMaxRadius = max(nl->bbox.vSize.x, nl->bbox.vSize.y);
		//re-arrange spots (maybe lights image changed)
		if (nl->animID >= 0)
		{
			RECTXYWH lrect = m_sprLights.GetAFrameBBox_real(nl->animID, 0);
			float fScaleX = nl->bbox.vSize.x / lrect.w;
			float fScaleY = nl->bbox.vSize.y / lrect.h;

			bbmin.x = lrect.x * fScaleX; bbmin.y = lrect.y * fScaleY;
			bbmax.x = lrect.w * fScaleX; bbmax.y = lrect.h * fScaleY;
			bbmin += nl->pos;
			bbmax += bbmin;
			//set bbox
			nl->bbox.Set_Corrected(bbmin, bbmax);
			nl->bbox_ini = nl->bbox;
			nl->bbox_ini.Move(-nl->pos);
			nl->fMaxRadius = max(nl->bbox.vSize.x, nl->bbox.vSize.y);
		}

		//read angle and convert to radians
		nl->fAngle = (float)OS_freadInt16(fl);
		nl->fAngle = DEG_TO_RAD(nl->fAngle);
		nl->fAngle_ini = nl->fAngle;
		//casts shadows
		UINT16 u2b = OS_freadUInt16(fl);
		nl->castShadows = ((u2b & K_EDITOR_LIGHT_FLAG_CAST_SHADOWS) != 0);
		
		//save global ambient light color
		if(nl->type == K_LVL_LIGHT_AMBIENTAL)
			m_colAmbientGlobal = nl->color;

		//set all internal light data needed for rendering
		nl->InitGeometry(&m_sprLights);

		//load logic
		nl->LoadLogic(fl);

		m_arrLights.Add(nl);
	}

	///--- collision elements ---
	SAFE_DELETE_GROWABLE_ARRAY(m_arrColShapes);
	//add level limits
	CCollisionShape* cc = null;
	//left
	cc = new CCollisionShape();
	cc->bbox.Set(D3DXVECTOR2(m_levelAABB.x - 32.0f, m_levelAABB.y), D3DXVECTOR2(m_levelAABB.x + 2.0f, m_levelAABB.Bottom()));
	cc->ubFlags = K_LVL_COLLFLAG_LEVEL_BOUNDS;
	m_arrColShapes.Add(cc);
	//right
	cc = new CCollisionShape();
	cc->bbox.Set(D3DXVECTOR2(m_levelAABB.Right() - 2.0f, m_levelAABB.y), D3DXVECTOR2(m_levelAABB.Right() + 32.0f, m_levelAABB.Bottom()));
	cc->ubFlags = K_LVL_COLLFLAG_LEVEL_BOUNDS;
	m_arrColShapes.Add(cc);
	//bottom
	cc = new CCollisionShape();
	cc->bbox.Set(D3DXVECTOR2(m_levelAABB.x, m_levelAABB.Bottom() - 2.0f), D3DXVECTOR2(m_levelAABB.Right(), m_levelAABB.Bottom() + 32.0f));
	cc->ubFlags = K_LVL_COLLFLAG_LEVEL_BOUNDS;
	m_arrColShapes.Add(cc);
	//top
	cc = new CCollisionShape();
	cc->bbox.Set(D3DXVECTOR2(m_levelAABB.x, m_levelAABB.y - 32.0f), D3DXVECTOR2(m_levelAABB.Right(), m_levelAABB.y + 2.0f));
	cc->ubFlags = K_LVL_COLLFLAG_LEVEL_BOUNDS;
	m_arrColShapes.Add(cc);

	//read level collision boxes
	int colCnt = (int)OS_freadUInt32(fl);
	//date fiecare element
	for (int kk = 0; kk < colCnt; kk++)
	{
		CCollisionShape* colobj = new CCollisionShape();
		colobj->ID = OS_freadUInt32(fl);

		D3DXVECTOR2 cmin, cmax;
		cmin.x = (float)OS_freadInt32(fl); cmin.y = (float)OS_freadInt32(fl); //XY
		cmax.x = (float)OS_freadUInt32(fl); cmax.y = (float)OS_freadUInt32(fl); //WH
		cmin.y += (float)nLevelOffsetY;
		cmax += cmin;
		colobj->bbox.Set(cmin, cmax);
		//bbox safeguarding
		if ((colobj->bbox.vSize.x <= 0.0f) || (colobj->bbox.vSize.y <= 0.0f))
			colobj->bbox.Set(D3DXVECTOR2(0.0f, 0.0f), D3DXVECTOR2(16.0f, 16.0f));
		colobj->bbox_ini = colobj->bbox;
		//set exported bboxes too
		colobj->bbox_exported = colobj->bbox;
		colobj->bbox_exported_ini = colobj->bbox_ini;
		//setam pos on center
		colobj->pos = colobj->bbox_ini.vCenter;
		//type (ub)
		colobj->type = OS_freadUByte(fl);
		//cast shadows
		colobj->castShadows = (OS_freadByte(fl) != 0) ? true : false;

		//load logic and init custom data
		colobj->LoadLogic(fl);
		colobj->InitInternalData();

		m_arrColShapes.Add(colobj);
	}


	///--- objects - decorations ---
	SAFE_DELETE_GROWABLE_ARRAY(m_arrActives);
	m_arrActivesPtrInteract.Clear();
	//read path
	OS_freadString(fl, charArr);
	mbstowcs_s(&converted, wcharArr, charArr, MAX_PATH);
	//load bsx
	StringCchPrintf(wcsMediaAddr, MAX_PATH, L"media/levels/data/%s", wcharArr);
	FileManager::GetMediaPath(wcsMediaAddr, Path);
	if (FAILED(m_sprActives.LoadSprites(Path)))
	{
		return E_FAIL;
	}

	//--- actives ---
	CFixedArray<int, 20> arrLocalBombIDs;

	int decocnt = (int)OS_freadUInt32(fl);
	for (int kk = 0; kk < decocnt; kk++)
	{
		CActive* obj = new CActive();

		obj->ID = OS_freadUInt32(fl);
		//convert layer from editor values to game values (editor misses MID layer):
		byte nLayer = OS_freadByte(fl);
		obj->nLayer = nLayer;
		//position (used to load UINT32)
		obj->pos.x = (float)OS_freadInt32(fl);
		obj->pos.y = (float)OS_freadInt32(fl);
		obj->pos.y += (float)nLevelOffsetY;
		obj->pos_ini = obj->pos;
		//animation
		CHAR charAnmName[MAX_PATH];
		OS_freadString(fl, charAnmName);
		int animIdx = m_sprActives.getAnimationIdxByName(charAnmName);
		if (animIdx < 0)
			ErrorBox(K_ERR_WARNING, L"Active ID:%d without animation!", obj->ID);
		//frame
		int frameIdx = OS_freadUInt16(fl);
		obj->sprite.Init(animIdx, obj->pos.x, obj->pos.y, frameIdx);
		obj->nAnim_ini = animIdx;
		obj->nFrame_ini = frameIdx;
		obj->color = 0xffffffff;
		obj->sprite.color = obj->color;
		obj->bStandsOut = false;
		//angle
		obj->fAngle = 0.0f;
		obj->fAngle_ini = 0.0f;
		//load flags and split
		UINT32 activFlags = OS_freadUInt32(fl);
		//flip xy
		obj->flipX = ((activFlags & K_EDITOR_ACTIVE_FLAG_FLIPX) != 0);
		obj->flipY = ((activFlags & K_EDITOR_ACTIVE_FLAG_FLIPY) != 0);
		//animated
		obj->bAnimated = ((activFlags & K_EDITOR_ACTIVE_FLAG_ANIMATED) != 0);
		obj->bReleaseIt = false;
		//animated? select different start frame
		if (obj->bAnimated)
		{
			obj->sprite.currentFrame = m_rand.RandInt(m_sprActives.GetAFramesCnt(obj->sprite.animationIdx));
		}
		//bbox
		RECTXYWH bbox_set = m_sprActives.GetAFrameBBox(animIdx, frameIdx);
		RECTXYWH objbox = m_sprActives.GetAFrameBBox_real(animIdx, frameIdx);
		obj->bbox_ini.Set(objbox);
		obj->bbox_exported_ini.Set(bbox_set);
		//daca e flipat pe X flipez si bbox. Pe Y nu e cazul pt ca se pastreaza in acelasi bbox in paint
		if (obj->flipX)
		{
			obj->bbox_ini.Move(D3DXVECTOR2(-2.0f * obj->bbox_ini.vCenter.x, 0.0f));
			obj->bbox_exported_ini.Move(D3DXVECTOR2(-2.0f * obj->bbox_exported_ini.vCenter.x, 0.0f));
		}
		obj->bbox = obj->bbox_ini;
		obj->bbox.Move(obj->pos);
		
		obj->bbox_exported = obj->bbox_exported_ini;
		obj->bbox_exported.Move(obj->pos);

		//load logic and init data
		obj->LoadLogic(fl);
		obj->InitInternalData();

		m_arrActives.Add(obj);

		//mark and save interactibles
		if (obj->bCanInteract)
			m_arrActivesPtrInteract.Add(obj);

		///--- setari speciale ---
		//#HARDCODE: is cover? - add bbox as cover box
		if (activFlags & K_EDITOR_ACTIVE_FLAG_IS_COVER)
		{
			//make the object stand out
			obj->bStandsOut = true;

			CCollisionShape* colobj = new CCollisionShape();
			colobj->ID = GenerateNextID(); 

			RECTXYWH objbox = m_sprActives.GetAFrameBBox(obj->sprite.animationIdx, obj->sprite.currentFrame);
			//objbox.Move(obj->pos.x, obj->pos.y);
			
			colobj->bbox.Set(objbox);
			colobj->bbox.Flip(obj->flipX, false);
			colobj->Move(obj->pos);

			colobj->bbox_ini = colobj->bbox;
			//setam si pos pe centrul ei
			colobj->pos = colobj->bbox_ini.vCenter;
			colobj->type = K_LVL_COLL_TYPE_COVER;
			//cast shadows - off by default
			colobj->castShadows = false;
			//load logic
			colobj->bCanInteract = false;
			colobj->bHideInteractIcon = false;
			colobj->bHidden = false;

			colobj->targetID_ini = -1; //save for later
			colobj->AIstate = K_AI_STATE_UNDEFINED;
			colobj->collFlags = K_DIRFLAG_NONE;
			colobj->stairSize = 0;

			colobj->bReleaseIt = false;

			m_arrColShapes.Add(colobj);
		}
		//#HACK: is it a door? set special AI
		CStringHash shAnimName(charAnmName);
		const CStringHash shAnimDoors(L"DOORS_SECTION");
		if (shAnimName == shAnimDoors)
		{
			obj->AIstate = K_AI_STATE_ACTIVE_DOOR_SECTION;
			if (obj->script_hash.IsEqual(L"ACTIVE_LOCKED_BREAKABLE"))
			{
				obj->varAIparams.SetNamedVarINT32(L"n_locked", 1);
				obj->varAIparams.SetNamedVarFloat(L"f_lockpickTime", 2.5f);
			}
		}
		//--- bomb defusal mode ---
		if (obj->AIstate == K_AI_STATE_ACTIVE_BOMB)
		{
			arrLocalBombIDs.Add(obj->ID);
		}
		//--- zombie spawners ---
		if (obj->AIstate == K_AI_STATE_ACTIVE_ZOMBIE_SPAWNER)
		{
			m_arrStats[K_LVL_STATS_ZOMBIE_PORTALS]++;
		}
	}
	//--- init bomb mode ---
	if (arrLocalBombIDs.Count() > 0)
	{
		m_arrStats[K_LVL_STATS_LEVEL_HAS_BOMBS] = 1;
		m_arrStats[K_LVL_STATS_LEVEL_BOMB_SEEN] = 0;
		//select random bomb
		int nFinalBombIdx = m_rand.RandInt(arrLocalBombIDs.Count());
		m_arrStats[K_LVL_STATS_LEVEL_BOMB_ID] = arrLocalBombIDs[nFinalBombIdx];
		//hide-deallocate other bombs
		for (int kk = 0; kk < arrLocalBombIDs.Count(); kk++)
		{
			if (kk != nFinalBombIdx)
			{
				CActive* bombact = dynamic_cast<CActive*>(GetIActiveInterfacePtr(arrLocalBombIDs[kk]));
				if (bombact != null)
				{
					bombact->bSetHidden = true;
					bombact->bHidden = true;
					bombact->bReleaseIt = true;
				}
			}
		}
	}

	///--- load actors ---
	SAFE_DELETE_GROWABLE_ARRAY(m_arrActors);
	//read path
	OS_freadString(fl, charArr);
	mbstowcs_s(&converted, wcharArr, charArr, MAX_PATH);
	//load bsx
	StringCchPrintf(wcsMediaAddr, MAX_PATH, L"media/levels/data/%s", wcharArr);
	FileManager::GetMediaPath(wcsMediaAddr, Path);
	if (FAILED(m_sprActors.LoadSprites(Path)))
	{
		return E_FAIL;
	}

	//--- load actors templates and weaponry right after actives sprite ---
	FileManager::GetMediaPath(L"media/levels/data/weapons_data.xml", Path);
	if (FAILED(LoadWeaponTemplates(Path)))
	{
		return E_FAIL;
	}

	FileManager::GetMediaPath(L"media/levels/data/actors_data.xml", Path);
	if (FAILED(LoadActorTemplates(Path)))
	{
		return E_FAIL;
	}


	int actorscnt = (int)OS_freadUInt32(fl);
	for (int kk = 0; kk < actorscnt; kk++)
	{
		UINT32 actID = OS_freadUInt32(fl);
		//pozitia
		D3DXVECTOR2 actPos;
		actPos.x = (float)OS_freadInt32(fl);
		actPos.y = (float)OS_freadInt32(fl);
		actPos.y += nLevelOffsetY;
		//boolean SetAngle si unghi
		bool bSetActorAngle = (OS_freadByte(fl) != 0) ? true : false;
		float fActorAngle = DEG_TO_RAD(OS_freadInt16(fl));
		//read template name
		CHAR readstr[MAX_PATH];
		WCHAR templateNameW[MAX_PATH];
		OS_freadString(fl, readstr);
		mbstowcs(templateNameW, readstr, MAX_PATH);
		//read selected AI state from editor
		WCHAR stateNameW[MAX_PATH];
		OS_freadString(fl, readstr);
		mbstowcs(stateNameW, readstr, MAX_PATH);
		//direction
		bool bactLookleft = (OS_freadByte(fl) != 0) ? true : false;
		bool bactCollision = (OS_freadByte(fl) != 0) ? true : false;
		bool bactGravity = (OS_freadByte(fl) != 0) ? true : false;
		//logic
		byte n1b = OS_freadByte(fl);
		bool bactCanInteract = (n1b & 0x1);
		bool bactHideInteract = (n1b & 0x2);
		float factTouchDuration = (float)OS_freadInt32(fl);
		bool bactStartHidden = (OS_freadByte(fl) != 0) ? true : false;
		INT32 nactTargetID = OS_freadInt32(fl);
		//read script AI name
		CHAR strScriptName[MAX_PATH];
		CHAR strAIname[MAX_PATH];
		OS_freadString(fl, strScriptName);
		OS_freadString(fl, strAIname);
		//read AI params
		CVariantCollection arrParams;
		int nAIparamsCnt = OS_freadByte(fl); //nr params
		if (nAIparamsCnt > 0)
		{
			for (int i = 0; i < nAIparamsCnt; i++)
			{
				CHAR varname[MAX_PATH] = { 0 };
				WCHAR wvarname[MAX_PATH] = { 0 };
				CHAR varval[MAX_PATH];
				WCHAR wvarval[MAX_PATH];

				OS_freadString(fl, varname);
				OS_freadString(fl, varval);

				size_t convnr;
				mbstowcs_s(&convnr, wvarname, varname, MAX_PATH);
				mbstowcs_s(&convnr, wvarval, varval, MAX_PATH);

				arrParams.SetNamedVarAUTO(wvarname, wvarval);
			}
		}
		///--- FINISHED READING DATA ---

		///--- RANDOM ENEMIES HERE ---
		CStringHash shTemplateNameHash(templateNameW);
		if (shTemplateNameHash.textHash == FastHash(L"ACTOR_RANDOM_ENEMY"))
		{
			//random enemy should have multiple AI params lines with different templates on them.
			//will select a random character from the AI specified lines
			int nTemplatesCnt = arrParams.GetVariantCount();
			if (nTemplatesCnt == 0)
			{
				ErrorBox(K_ERR_CRITICAL, L"Random Enemy (ID:%d) should have more templates! Please specify templates in AI params as strings!", actID);
			}
			else
			{
				int nRandTemplate = m_rand.RandInt(nTemplatesCnt);
				shTemplateNameHash = arrParams[nRandTemplate]->m_strArg;
				if (shTemplateNameHash.textHash == 0)
				{
					shTemplateNameHash.Init(L"ACTOR_RANDOM_ENEMY");
					ErrorBox(K_ERR_CRITICAL, L"Random Enemy (ID:%d) illegal template name: [%s]", actID, arrParams[nRandTemplate]->m_strArg.text);
				}
			}
			//erase AI params
			arrParams.DeleteAll();
		}

		//add actor
		CActor* nact = new CActor();

		nact->ID = actID; //save actor ID
		nact->bAnimated = true;  //animated by default

		CActorTemplate* acttempl = GetTemplateActor(shTemplateNameHash.textHash);
		if (acttempl == null)
		{
			ErrorBox(K_ERR_WARNING, L"LoadLevel::GetTemplateActor - invalid template name: %s", shTemplateNameHash.text);
		}
		InitActor(nact, acttempl, actPos);
		
		nact->lookDirXsign = (bactLookleft) ? -1 : 1;
		nact->fAngle = nact->fAngle_ini = fActorAngle;
		//daca unghiul e setat din editor il las asa cum e, altfel il sincronizez cu lookdirXsign
		//Unghiul trebuie setat corect pentru ca e folosit la gasirea inamicilor
		if (bSetActorAngle)
		{
			nact->SetAngle(fActorAngle);
		}
		else
		{
			if (nact->lookDirXsign == -1)
				nact->SetAngle(PI);
			else
				nact->SetAngle(0.0f);
		}

		nact->bHasCollision = bactCollision;
		nact->bHasGravity = bactGravity;

		//logic
		nact->bCanInteract = bactCanInteract;
		nact->bHideInteractIcon = bactHideInteract;
		//interact timer
		nact->fTouchDuration = factTouchDuration;
		//start hidden
		nact->bHidden = nact->bSetHidden = bactStartHidden;

		nact->targetID_ini = nactTargetID; //save for later when we have loaded all the objects
		//script name
		nact->script_hash.Init(strScriptName);

		int nNewAIstate = nact->AIstate; //default state is old state
		//if (strout[0] != 0) //if not empty override template AI state
		//{
		//	nNewAIstate = GetAIStateByNameHash(FastHash(strout));
		//}
		
		//append the editor ai params as some of them are set from the AI function
		nact->varAIparams.AppendCollection(arrParams);

		//set AI
		//#TODO: next line is useless
		SetAI(nact, nNewAIstate, &nact->varAIparams, nact->targetID_ini);
		//set state that was set from the editor
		if (wcslen(stateNameW) > 0)
		{
			CAIState* nState = nact->templateActor.AItemplate->GetAIStateByName(stateNameW);
			if (nState == null)
			{
				ErrorBox(K_ERR_WARNING, L"[WARNING] LoadLevel: State %s not found on ID:%d", stateNameW, nact->ID);
			}
			SetActorAIState(nact, nState);
		}

		m_arrActors.Add(nact);

		//--- update statistics ---
		bool bCountEnemy = false;
		if (nact->GetCurrentBehavior() != AI_BEHAVIOR_DEAD)
		{
			//#HACK: set script on hostage and count them as targets
			if (nact->templateActor.actorClass == K_LVL_ACT_CLASS_HOSTAGE)
			{
				bCountEnemy = false;
				//add actor as target only if not spawned already dead (DEAD behavior)
				m_arrStats[K_LVL_STATS_TARGETS_TOTAL]++;
				m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]++;
				//forteaza scriptul de save hostage chiar daca nu l-ai setat din editor
				nact->bCanInteract = true;
				if (nact->script_hash.IsEmpty())
					nact->script_hash.Init(L"SAVE_HOSTAGE");
			}
			else if (nact->templateActor.actorClass == K_LVL_ACT_CLASS_HUMAN)
			{
				//add actor as target only if not spawned already dead (DEAD behavior)
				bCountEnemy = true;
			}
			else if (nact->templateActor.actorClass == K_LVL_ACT_CLASS_ZOMBIE)
			{
				m_arrStats[K_LVL_STATS_ZOMBIES_TOTAL]++;
				bCountEnemy = true;
			}
		}
		//now count enemy targets
		if (bCountEnemy)
		{
			m_arrStats[K_LVL_STATS_TARGETS_TOTAL]++;
		}
	}

	//initialize targets left
	m_arrStats[K_LVL_STATS_TARGETS_LEFT] = m_arrStats[K_LVL_STATS_TARGETS_TOTAL];

	///--- incarca elementele speciale ---
	SAFE_DELETE_GROWABLE_ARRAY(m_arrMiscObjects);
	
	UINT32 miscCnt = OS_freadUInt32(fl);
	for (UINT32 kk = 0; kk < miscCnt; kk++)
	{
		byte type = OS_freadByte(fl);

		switch (type)
		{
			case K_LVL_MISC_FRONTLAYEROBJ:
			{
				CMiscObject_FrontLayerObj * frontobj = new CMiscObject_FrontLayerObj();
				//generic data
				frontobj->ID = OS_freadUInt32(fl);
				//read params
				int nparamsCnt = OS_freadByte(fl); //nr params
				if (nparamsCnt > 0)
				{
					for (int i = 0; i < nparamsCnt; i++)
					{
						CHAR varname[MAX_PATH] = { 0 };
						CHAR varval[MAX_PATH];
						WCHAR wvarval[MAX_PATH], wvarname[MAX_PATH];

						OS_freadString(fl, varname);
						OS_freadString(fl, varval);

						size_t convnr;
						mbstowcs_s(&convnr, wvarval, varval, MAX_PATH);
						mbstowcs_s(&convnr, wvarname, varname, MAX_PATH);

						frontobj->varParams.SetNamedVarAUTO(wvarname, wvarval);
					}
				}
				//specific data 
				//pozitia o citesc si nu o folosesc
				frontobj->pos.x = (float)OS_freadInt32(fl);
				frontobj->pos.y = (float)OS_freadInt32(fl);
				frontobj->pos.y += (float)nLevelOffsetY;
				//set color
				frontobj->sprite.color = m_colAmbientGlobal;

				m_arrMiscObjects.Add(frontobj);
			}
			break;
			case K_LVL_MISC_SCRIPT:
			{
				//generic data
				UINT32 ID = OS_freadUInt32(fl);
				//read params
				int nparamsCnt = OS_freadByte(fl); //nr params
				if (nparamsCnt > 0)
				{
					for (int i = 0; i < nparamsCnt; i++)
					{
						CHAR varname[MAX_PATH] = { 0 };
						CHAR varval[MAX_PATH];
						WCHAR wvarname[MAX_PATH];
						WCHAR wvarval[MAX_PATH];

						OS_freadString(fl, varname);
						OS_freadString(fl, varval);

						size_t convnr;
						mbstowcs_s(&convnr, wvarval, varval, MAX_PATH);
						mbstowcs_s(&convnr, wvarname, varname, MAX_PATH);
						
						if (wcscmp(wvarname, L"str_script") == 0)
						{
							//am citit primul parametru iar valoarea lui este bsx-ul fundalului deci incarc fundalul
							UTGetScriptManager().StartScript(wvarval);
						}
					}
				}
				//pozitia o citesc si nu o folosesc
				OS_freadUInt32(fl); OS_freadUInt32(fl);
			}
			break;
			case K_LVL_MISC_BACKGROUND:
			{
				//generic data
				UINT32 ID = OS_freadUInt32(fl);
				//read params
				int nparamsCnt = OS_freadByte(fl); //nr params
				if (nparamsCnt > 0)
				{
					for (int i = 0; i < nparamsCnt; i++)
					{
						CHAR varname[MAX_PATH] = { 0 };
						CHAR varval[MAX_PATH];
						WCHAR wvarname[MAX_PATH];
						WCHAR wvarval[MAX_PATH];

						OS_freadString(fl, varname);
						OS_freadString(fl, varval);

						size_t convnr;
						mbstowcs_s(&convnr, wvarval, varval, MAX_PATH);
						mbstowcs_s(&convnr, wvarname, varname, MAX_PATH);
						//HARDCODE: de scos hardcodarea dupa i
						if (wcscmp(wvarname, L"str_bsx") == 0) 
						{
							//first param is the bsx-ul for the background so load it
							StringCchPrintf(wcsMediaAddr, MAX_PATH, L"media/back/%s", wvarval);
							FileManager::GetMediaPath(wcsMediaAddr, Path);
							//m_sprBack.LoadSprites(Path);
							//defaults on first anim
							//m_BackAnimIdx = 0;
						}
						else if (wcscmp(wvarname, L"str_anim") == 0) 
						{
							//m_BackAnimIdx = m_sprBack.getAnimationIdxByName(wvarval);
						}
						else if (wcscmp(wvarname, L"str_water_anim") == 0)
						{
							//m_waterAnimIdx = m_sprBack.getAnimationIdxByName(wvarval);
						}
					}
				}
				//specific data 
				//pozitia o citesc si nu o folosesc
				OS_freadUInt32(fl); OS_freadUInt32(fl);

				//m_bPaintBackground = true;
			}
			break;
			case K_LVL_MISC_RAILS:
			{
				CMiscObjectRail* rail = new CMiscObjectRail();
				//generic data
				rail->ID = OS_freadUInt32(fl);
				//read params
				int nparamsCnt = OS_freadByte(fl); //nr params
				if (nparamsCnt > 0)
				{
					for (int i = 0; i < nparamsCnt; i++)
					{
						CHAR varname[MAX_PATH] = { 0 };
						CHAR varval[MAX_PATH] = { 0 };
						WCHAR wvarname[MAX_PATH];
						WCHAR wvarval[MAX_PATH];

						OS_freadString(fl, varname);
						OS_freadString(fl, varval);

						size_t convnr;
						mbstowcs_s(&convnr, wvarname, varval, MAX_PATH);
						mbstowcs_s(&convnr, wvarval, varval, MAX_PATH);

						rail->varParams.SetNamedVarAUTO(wvarname, wvarval);
					}
				}
				//specific data 
				//pozitia punctelor
				UINT16 ptscnt = OS_freadUInt16(fl);
				float totalLength = 0.0f;
				//coordonate puncte
				for (int i = 0; i < ptscnt; i++)
				{
					D3DXVECTOR2 pt;
					pt.x = OS_freadInt32(fl);
					pt.y = OS_freadInt32(fl);
					pt.y += nLevelOffsetY;
					rail->arrPoints.Add(pt);
					//lungimile
					if (i == 0)									
					{
						totalLength = 0.0f;
						rail->arrLenghts.Add(totalLength);
					}
					else
					{
						D3DXVECTOR2 dist = rail->arrPoints.m_pData[i] - rail->arrPoints.m_pData[i - 1];
						float ldist = D3DXVec2Length(&dist);
						totalLength += ldist;
						rail->arrLenghts.Add(totalLength);
					}
				}
				rail->fLength = totalLength;
				//check total len
				if (totalLength <= 0.0f)
				{
					ErrorBox(K_ERR_WARNING, L"Zero length rail! ID:%d", rail->ID);
					SAFE_DELETE(rail);
					break;
				}
				//add rail to list if everything ok
				m_arrMiscObjects.Add(rail);
			}
			break;
		}
	}
	//set animation data at the end (some front objs need bg to be loaded)
	for (UINT32 kk = 0; kk < m_arrMiscObjects.GetSize(); kk++)
	{
		CMiscObjectBase* mob = m_arrMiscObjects[kk];
		if (mob->type == K_LVL_MISC_FRONTLAYEROBJ)
		{
			CMiscObject_FrontLayerObj *frontobj = dynamic_cast<CMiscObject_FrontLayerObj*>(mob);
			if (frontobj != null)
			{
				//anim name
				UINT32 animHash = frontobj->varParams.GetVariantByName(L"strAnim")->m_strArg.getHash();
				frontobj->sprite.animationIdx = -1;// m_sprBack.getAnimationIdxByNameHash(animHash);
				frontobj->sprite.currentFrame = frontobj->varParams.GetVariantByName(L"nFrame")->m_asUINT32;
				//set bbox
				//frontobj->aabb_ini.Set(m_sprBack.GetAFrameBBox(frontobj->sprite.animationIdx, frontobj->sprite.currentFrame));
			}
		}
	}

	OS_fclose(fl);


	//#ZOMBIE: place spawners in level only if level doesn't have zombies. Leave as it is if it has.
	if ((g_gameMode == GAME_MODE_ZOMBIE_INVASION) && (m_arrStats[K_LVL_STATS_ZOMBIES_TOTAL] <= 0))
	{
		//decide number of spawners based on number of enemies/targets
		int nTotalTargets = m_arrStats[K_LVL_STATS_TARGETS_TOTAL];
		int nZombiesToSpawn = nTotalTargets / 3;
		if (nZombiesToSpawn < 5)
			nZombiesToSpawn = 5;
		int nMediumSpawnCount = 4 + m_rand.RandInt(3);
		int nSpawnersCnt = nZombiesToSpawn / nMediumSpawnCount;
		if (nSpawnersCnt < 1)
			nSpawnersCnt = 1;

		LOG(L"ZOMBIE MODE: Adding %d spawners (spawn count:%d)", nSpawnersCnt, nMediumSpawnCount);

		//min distance between spawners
		const float fMinSpawnersDistance = 300.0f;

		CFixedArray<D3DXVECTOR2, 50> arrLocalSpawnersPos;
		for (int kk = 0; kk < nSpawnersCnt; kk++)
		{
			int nTries = 0;
			int nSelectedActorIdx = -1;
			bool bFound = false;
			while ((nTries < 50) && (bFound == false))
			{
				nSelectedActorIdx = m_rand.RandInt(m_arrActors.GetSize());
				assert((nSelectedActorIdx >= 0) && (nSelectedActorIdx < m_arrActors.GetSize()));

				D3DXVECTOR2 vPos = m_arrActors[nSelectedActorIdx]->pos;
			    //check valid position
				bool bFailTest = false;
				//spawn only where enemies are present
				if (m_arrActors[nSelectedActorIdx]->templateActor.actorClass != K_LVL_ACT_CLASS_HUMAN)
					bFailTest = true;
				
				//check distance from other spawners
				if (!bFailTest)
				{
					for (int ll = 0; ll < arrLocalSpawnersPos.Count(); ll++)
					{
						if (D3DXVec2Length(&(arrLocalSpawnersPos.m_pData[ll] - vPos)) < fMinSpawnersDistance)
						{
							bFailTest = true;
							break;
						}
					}
				}

				//check distance from walls and active elements
				if (!bFailTest)
				{
					RECTXYWH_F objrect = m_sprActives.GetAFrameBBox(ANM_ACTIVES_SPR_ZOMBIE_SPAWNER_APPEAR, 0);
					objrect.Move(vPos.x, vPos.y - 2.0f);
					//find a random placing spot
					float fOffX = 0.0f;
					RECTXYWH_F placerect = objrect;
					int nLocTries = 30;
					bool bPlaced = false;
					while ((nLocTries > 0) && (bPlaced == false))
					{
						// call randoms on separate lines so they don't switch order on dbg/release
						int nSgn = m_rand.RandSign();
						float fOffXf = m_rand.RandFloat(objrect.w);
						fOffX = nSgn * ((objrect.w / 2.0f) + fOffXf);
						placerect = objrect;
						placerect.Move(fOffX, 0.0f);
						bPlaced = GetIsAreaNeutral(placerect);
						//is spawner placed on ground?
						if (bPlaced)
						{
							CCollisionShape* colshape = GetCollisionShapeAt(D3DXVECTOR2(vPos.x + fOffX, vPos.y + 2.0f));
							if (colshape == null)
								bPlaced = false;
						}
						//line of sight between start and end pos
						if (bPlaced)
						{
							if (!IsLineOfSight(objrect.Center(), placerect.Center()))
								bPlaced = false;
						}

						nLocTries--;
					}

					if (!bPlaced)
						bFailTest = true;
					else
						vPos.x += fOffX;
				}

				//All good, save spawn pos
				if (!bFailTest)
				{
					bFound = true;
					arrLocalSpawnersPos.Add(vPos);
				}

				nTries++;
				if (nTries >= 50)
				{
					ErrorBox(K_ERR_WARNING, L"ZOMBIE MODE: Could not place spawner!");
				}
			}
		}

		for (int kk = 0; kk < arrLocalSpawnersPos.Count(); kk++)
		{
			D3DXVECTOR2 vPos = arrLocalSpawnersPos.m_pData[kk];
			//add green flickering light
			CLight* pLight = SpawnLight(D3DXVECTOR3(vPos.x, vPos.y - 20.0f, 100.0f), K_LVL_LIGHT_POINT, ANM_LIGHTS_SPR_POINT1, 0xff3bff3b, 1.0f, false);
			pLight->AIstate = K_AI_STATE_FN_LIGHT_FLICKER1;
			pLight->bHidden = true;
			pLight->bSetHidden = true;
			//AI data
			pLight->varAIparams.SetNamedVarFloat(L"f_timeMul", 4.0f);
			pLight->varAIparams.SetNamedVarFloat(L"f_threshold", 0.5f);

			CActive* pSpawner = SpawnActive(vPos, ANM_ACTIVES_SPR_ZOMBIE_SPAWNER_APPEAR, 0, K_LVL_LAYER_MIDDLE);
			pSpawner->bAnimated = false;
			pSpawner->nLayer = K_LVL_LAYER_MIDDLE; //so it doesn't get dirty
			pSpawner->bCanInteract = true;
			pSpawner->fTouchDuration = 4.0f;
			pSpawner->script_hash.Init(L"DISABLE_ZOMBIE_SPAWNER");
			//AI data
			pSpawner->AIstate = K_AI_STATE_ACTIVE_ZOMBIE_SPAWNER;
			pSpawner->targetID_ini = pLight->ID;
			pSpawner->varAIparams.SetNamedVarFloat(L"f_spawnFreq", 4.0f + m_rand.RandFloat(2.0f));
			pSpawner->varAIparams.SetNamedVarINT32(L"n_maxSpawns", nMediumSpawnCount - 1 + m_rand.RandInt(3));

			//save stats
			m_arrStats[K_LVL_STATS_ZOMBIE_PORTALS]++;
		}

		//STEP 2: replace a few hostages with fake ones
		int nFakeHostages = m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL] / 4;
		if (nFakeHostages >= 2)
		{
			nFakeHostages += m_rand.RandInt(3) - 1;
			if (nFakeHostages < 0)
				nFakeHostages = 0;
		}

		CFixedArray<int, 50> arrLocalHostageIdx;
		for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
		{
			if (m_arrActors[kk]->templateActor.actorClass == K_LVL_ACT_CLASS_HOSTAGE)
				arrLocalHostageIdx.Add(kk);
		}

		CActorTemplate* pTemplate = GetTemplateActor(L"ACTOR_ZOMBIE_HOSTAGE_CROUCHED");
		if (pTemplate != null)
		{
			int nHostageIdx = m_rand.RandInt(100);
			for (int kk = 0; kk < nFakeHostages; kk++)
			{
				int nActIdx = arrLocalHostageIdx.m_pData[nHostageIdx % arrLocalHostageIdx.Count()];
				nHostageIdx += m_rand.RandInt(100);

				CActor* act = m_arrActors[nActIdx];
				//set new template and make sure it wasn't already converted
				if ((act->templateActor.actorClass == K_LVL_ACT_CLASS_HOSTAGE) && (!act->script_hash.IsEqual(L"HOSTAGE_TO_ZOMBIE")))
				{
					InitActor(act, pTemplate, act->pos);
					act->script_hash.Init(L"HOSTAGE_TO_ZOMBIE");
					act->bCanInteract = true;
					//not necessary to end the level
					m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]--;
					m_arrStats[K_LVL_STATS_TARGETS_TOTAL]--;
					//has to be killed
					m_arrStats[K_LVL_STATS_TARGETS_TOTAL]++;
				}
			}
		}
	}



	///--- everything loaded, SetAI here ---
	//setez ai-ul la final ca sa execute functiile de initializare cand avem toate array-urile incarcate (ca sa ma asigur ca gaseste target ID-urile)
	for (int kk = 0; kk < m_arrLights.GetSize(); kk++)
	{
		CLight * light = m_arrLights[kk];
		SetAI(light, light->AIstate, &light->varAIparams, light->targetID_ini);
	}
	for (int kk = 0; kk < m_arrColShapes.GetSize(); kk++)
	{
		CCollisionShape * shape = m_arrColShapes[kk];
		SetAI(shape, shape->AIstate, &shape->varAIparams, shape->targetID_ini);
	}
	for (int kk = 0; kk < m_arrActives.GetSize(); kk++)
	{
		CActive * activ = m_arrActives[kk];
		SetAI(activ, activ->AIstate, &activ->varAIparams, activ->targetID_ini);
	}
	//ma asigur ca toti actorii au pointerii setati bine chemand inca odata setAI
	for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
	{
		CActor* actor = m_arrActors[kk];
		SetAI(actor, actor->AIstate, &actor->varAIparams, actor->targetID_ini);
	}

	//other settings:
	SAFE_DELETE_GROWABLE_ARRAY(m_arrDecals);

	//set interfaces ptrs
	m_interfaceIGM.Init(&m_sprInterface, pPlayerActor[0], pPlayerActor[1]);
	m_interfaceTextBubble.Init(&UTGetControlsManager().m_sprCol);
	///--- camera ---
	//target
	m_camTargetActive = null; //cand nu am target se uita dupa players
	m_camTargetOld = null;
	//cam settings
	m_camLevel.SetWorldBounds(m_levelAABB, true, K_CAMTRANS_AXIS_NONE);
	m_HiddenRoomAABB.Set(0.0f, 0.0f, 0.0f, 0.0f);
	//for the render targets we render 1:1
	m_vCamPosDefault = vLastSpawnPoint; //spawn pointul este initializat in setAI cand gaseste checkpoint cu bIsFirst
	m_camLevel.InitCamera(UTGetAppClass().g_rectRender, K_GAME_WIDTH, K_CAMTRANS_AXIS_H, m_vCamPosDefault); //initializam pe primul spawn point
	m_camLevel.SetCamAnimationSpring(K_LVL_CAM_FOLLOW_SPRING_KS, K_LVL_CAM_FOLLOW_DAMPING_KD);
	//chemam un update ca sa ne asiguram ca am initializat toate variabilele camerei
	m_camLevel.Update(0.0f);
	//pools
	m_poolPhysPts.Init(K_LVL_PHYSP_MAX_CNT);
	m_poolBullets.Init(K_LVL_BULLETS_MAX_CNT);
	m_poolProps.Init(K_LVL_PROPS_MAX_CNT);

	//spawn selected players
	m_nPlayers = 0;
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		//trecem controller instanceIDs in arr local din level
		m_arrPlayerControllersIIDs[kk] = g_playerSelScr.m_arrPlayers[kk].nInstanceID;
		//set selected 
		m_arrPlayerSelHotJoin[kk] = -1;
		m_arrPlayerSelStrategic[kk] = -1;
		//daca am selectat player
		if (g_playerSelScr.m_arrPlayers[kk].bSelected)
		{
			//spawn Player aloca si controllerul potrivit
			int offx = ((kk * 2) - 1) * K_TILE_HSIZE;
			SpawnPlayer(vLastSpawnPoint + D3DXVECTOR2((float)offx, 0.0f), kk, -1);
			//resolve selection
			m_arrPlayerSelHotJoin[kk] = (int)g_playerSelScr.m_arrPlayers[kk].eType;
		}
		else
		{
			m_arrPlayerControllersIIDs[kk] = -1; //allow hot join
		}
	}

	//clear global script memory (per level instance)
	UTGetScriptManager().ClearGlobalMemory();

	//reset time multiplier
	SetTimeMultiplier(1.0f, 0.0f);
	//facem un build visibility lists
	BuildVisibilityLists();

	//show level type																																							 
	if ((m_unLoadedLevelFlags & (K_LVL_LEVEL_FLAG_DOWNLOADED | K_LVL_LEVEL_FLAG_VINFINITE_MODE)) == 0)
	{
		int nStrIdxLevelName = UTGetChaptersList().m_arrChapters[nChapterNumber]->arrLevelNameStrIdx[nLevelNumber];
		if (nStrIdxLevelName >= 0)
		{
			int idx = g_particlesMgr.AddStringDummy(K_PDUMMY_STRING_WIDEBAR, D3DXVECTOR2(0.0f, -50.0f), nStrIdxLevelName, FONTIDX_12_WOW, 1.0f, 2.0f, K_COLOR_SELECTED_TEXT);
			//add second line of text
			CStringDummy* dum = g_particlesMgr.m_vDummies[idx];
			dum->intParam3 = STR_MISSION_TYPE1 + missionType;
			dum->intParam4 = FONTIDX_8_BS1;
		}

		m_nLoadedChapter = nChapterNumber;
		m_nLoadedLevel = nLevelNumber;
	}
	else
	{
		if (m_unLoadedLevelFlags & K_LVL_LEVEL_FLAG_DOWNLOADED)
		{
			g_particlesMgr.AddStringDummy(K_PDUMMY_STRING_WIDEBAR, D3DXVECTOR2(0.0f, -50.0f), STR_MISSION_TYPE1 + missionType, FONTIDX_12_WOW, 1.0f, 2.0f, K_COLOR_SELECTED_TEXT);
			m_nLoadedChapter = 1000;
			m_nLoadedLevel = nModIdx_SelectedContent;
		}
		else if (m_unLoadedLevelFlags & K_LVL_LEVEL_FLAG_VINFINITE_MODE)
		{
			int idx = g_particlesMgr.AddStringDummy(K_PDUMMY_STRING_WIDEBAR, D3DXVECTOR2(0.0f, -50.0f), STR_CURRENT_MISSION_VAL, FONTIDX_12_WOW, 1.0f, 2.0f, K_COLOR_SELECTED_TEXT);
			//add second line of text
			CStringDummy* dum = g_particlesMgr.m_vDummies[idx];
			dum->intParam3 = STR_MISSION_TYPE1;  //kill'em all by default
			dum->intParam4 = FONTIDX_8_BS1;
		}
	}
	//save type of loaded mission
	m_nLoadedLevelType = missionType;
	///--- LAST THINGS ---
	//called after characters spawning
	SetLevelState(K_LVL_STATE_PLAYING);

	m_bLoaded = true;

	LOG(L"Game:: Level loaded:[%s] net.randcheck[%d]", strPathAbs, m_rand.RandInt(60000));
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	LOG(L"Game:: Total Targets:[%d] Hostages:[%d]", m_arrStats[K_LVL_STATS_TARGETS_TOTAL], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);
#endif

	return hr;
}


HRESULT CLevel::LoadPrefabAtPosition(WCHAR * strPathAbs, int nPosXtiles, int nPosYtiles, bool bAddOnly)
{
	if (!m_bLoaded)
	{
		ErrorBox(K_ERR_WARNING, L"[Error] LoadPrefab(%s)::Load a level first!");
		return E_FAIL;
	}
	//load level
	FILE *fl = NULL;
	int err = OS_wfopen_s(&fl, strPathAbs, L"rb");

	if (fl == NULL || err != 0)
	{
		return E_FAIL;
	}

	//all elements start from this ID
	UINT32 dwBaseID = GenerateNextID();
	UINT32 dwMaxIDlocal = 0;
	//read int array
	UINT32 arrInts[10];
	OS_fread(arrInts, sizeof(UINT32), 10, fl);
	if (arrInts[0] != K_EDITOR_LEVEL_FILE_FORMAT_VERSION)
	{
		if (arrInts[0] == 1014)
		{
			LOG(L"LoadPrefabAtPosition(%s):: Old level format found [1014]!", strPathAbs);
		}
		else if (arrInts[0] < 1014) //last version files didn't have light volumes alpha
		{
			ErrorBox(K_ERR_WARNING, L"[Error] LoadPrefabAtPosition(%s)::Wrong file version found: %d !", strPathAbs, arrInts[0]);
			return E_FAIL;
		}
	}

	byte missionType = OS_freadByte(fl);
	//tileset name
	CHAR charArr[MAX_PATH];
	OS_freadString(fl, charArr); 

	//load tile size
	int tlW = OS_freadByte(fl);
	int tlH = OS_freadByte(fl);
	int tilesetColumns = OS_freadUInt16(fl);
	//prefab size in tiles (multiple of blocks)
	int levelW = OS_freadUInt16(fl);
	int levelH = OS_freadUInt16(fl);
	//level origin - in pixels
	int nPrefabOriginY = OS_freadInt16(fl);
	int nPrefabOriginX = OS_freadInt16(fl);

	//read tiles, not a rare matrix
	for (int yy = 0; yy < levelH; yy++)
	{
		for (int xx = 0; xx < levelW; xx++)
		{
			for (int kk = 0; kk < K_LVL_LAYERS_CNT; kk++)
			{
				int nTLx = nPosXtiles - (nPrefabOriginX / tileW) + xx - m_levelAABB_TL.x;
				int nTLy = nPosYtiles - (nPrefabOriginY / tileH) + yy - m_levelAABB_TL.y;
				//read tile
				int tileID = OS_freadInt32(fl);
				//check limits
				if ((nTLx < 0) || (nTLy < 0) || (nTLx >= levelSizeTL.w) || (nTLy >= levelSizeTL.h))
					continue;
				if ((bAddOnly) && (tileID < 0))
					continue;
				tiles[nTLx][nTLy].tileIDs[kk] = tileID;
				if (tileID >= 0)
					SetRect(&tiles[nTLx][nTLy].srcRects[kk], (tileID % tilesetColumns) * tileW, (tileID / tilesetColumns) * tileH, (tileID % tilesetColumns) * tileW + tileW, (tileID / tilesetColumns) * tileH + tileH);
				else
					SetRect(&tiles[nTLx][nTLy].srcRects[kk], 0, 0, 0, 0);
			}
		}
	}

	///--- lights ---
	//read path
	OS_freadString(fl, charArr);

	int lightsCnt = (int)OS_freadUInt32(fl);
	int nLightsCntOld = m_arrLights.GetSize();
	//date fiecare 
	for (int kk = 0; kk < lightsCnt; kk++)
	{
		D3DXVECTOR2 vOffset(nPosXtiles * tileW - nPrefabOriginX, nPosYtiles * tileH - nPrefabOriginY);

		CLight *nl = new CLight();
		nl->m_nLightMeshIdx = -1;
		nl->m_nShadowMeshIdx = -1;

		nl->ID = OS_freadUInt32(fl) + dwBaseID;
		if (nl->ID > dwMaxIDlocal)
			dwMaxIDlocal = nl->ID;
		nl->type = OS_freadByte(fl); //tip lumina
		int nVolumeAttenuationPerc = (int)OS_freadUInt32(fl);
		nl->fVolumeAlpha = 1.0f - (float)nVolumeAttenuationPerc / 100.0f;
		nl->fIntensity = OS_freadFloat32(fl);
		CLAMP(nl->fIntensity, 0.0f, 1.0f);
		nl->pos3D.x = (float)OS_freadInt32(fl);
		nl->pos3D.y = (float)OS_freadInt32(fl);
		nl->pos3D.z = (float)OS_freadInt32(fl);
		//z can't be in the same plane as the background
		if (nl->pos3D.z == 0.0f)
			nl->pos3D.z = 0.1f;
		//move light by spawn pos
		nl->pos3D.x += vOffset.x;
		nl->pos3D.y += vOffset.y;

		nl->pos_ini = nl->pos = D3DXVECTOR2(nl->pos3D.x, nl->pos3D.y);
		//animID
		CHAR charAnmName[MAX_PATH];
		OS_freadString(fl, charAnmName);

		nl->animID = m_sprLights.getAnimationIdxByName(charAnmName);
		if ((nl->animID < 0) && (nl->type != K_LVL_LIGHT_AMBIENTAL))
			ErrorBox(K_ERR_WARNING, L"[WARNING]Prefab::Light ID:%d doesn't have animID!!", nl->ID);
		//color
		BYTE ca, cr, cg, cb;
		ca = OS_freadUByte(fl); cr = OS_freadUByte(fl); cg = OS_freadUByte(fl); cb = OS_freadUByte(fl);
		nl->color = D3DCOLOR_ARGB(ca, cr, cg, cb);
		nl->color_ini = nl->color;

		D3DXVECTOR2 bbmin, bbmax;
		bbmin.x = (float)OS_freadInt32(fl);
		bbmin.y = (float)OS_freadInt32(fl);
		bbmax.x = bbmin.x + (float)OS_freadInt32(fl);
		bbmax.y = bbmin.y + (float)OS_freadInt32(fl);
		//move exported bbox too
		bbmin += vOffset; bbmax += vOffset;
		nl->bbox.Set_Corrected(bbmin, bbmax);
		nl->bbox_ini = nl->bbox;
		nl->bbox_ini.Move(-nl->pos);
		nl->fMaxRadius = max(nl->bbox.vSize.x, nl->bbox.vSize.y);
		//read angle and convert to radians
		nl->fAngle = (float)OS_freadInt16(fl);
		nl->fAngle = DEG_TO_RAD(nl->fAngle);
		nl->fAngle_ini = nl->fAngle;
		//casts shadows
		UINT16 u2b = OS_freadUInt16(fl);
		nl->castShadows = ((u2b & K_EDITOR_LIGHT_FLAG_CAST_SHADOWS) != 0);

		//set all internal light data needed for rendering
		nl->InitGeometry(&m_sprLights);
		//load logic
		nl->LoadLogic(fl);
		if (nl->targetID_ini >= 0)
			nl->targetID_ini += dwBaseID;

		m_arrLights.Add(nl);
	}

	///--- collision elements ---
	//read level collision boxes
	int colCnt = (int)OS_freadUInt32(fl);
	int nCollisionsCntOld = m_arrColShapes.GetSize();
	//date fiecare element
	for (int kk = 0; kk < colCnt; kk++)
	{
		D3DXVECTOR2 vOffset(nPosXtiles * tileW - nPrefabOriginX, nPosYtiles * tileH - nPrefabOriginY);

		CCollisionShape* colobj = new CCollisionShape();
		colobj->ID = OS_freadUInt32(fl) + dwBaseID;
		if (colobj->ID > dwMaxIDlocal)
			dwMaxIDlocal = colobj->ID;

		D3DXVECTOR2 cmin, cmax;
		cmin.x = (float)OS_freadInt32(fl); cmin.y = (float)OS_freadInt32(fl); //XY
		cmax.x = (float)OS_freadUInt32(fl); cmax.y = (float)OS_freadUInt32(fl); //WH
		cmax += cmin;
		//move them by spawn pos
		cmin += vOffset;
		cmax += vOffset;
		colobj->bbox.Set(cmin, cmax);
		//bbox safeguarding
		if ((colobj->bbox.vSize.x <= 0.0f) || (colobj->bbox.vSize.y <= 0.0f))
			colobj->bbox.Set(D3DXVECTOR2(0.0f, 0.0f), D3DXVECTOR2(16.0f, 16.0f));
		colobj->bbox_ini = colobj->bbox;
		//set exported bboxes too
		colobj->bbox_exported = colobj->bbox;
		colobj->bbox_exported_ini = colobj->bbox_ini;
		//set pos on center
		colobj->pos = colobj->bbox_ini.vCenter;

		//type (ub)
		colobj->type = OS_freadUByte(fl);
		//cast shadows
		colobj->castShadows = (OS_freadByte(fl) != 0) ? true : false;
		//load logic and internal data
		colobj->LoadLogic(fl);
		if (colobj->targetID_ini >= 0)
			colobj->targetID_ini += dwBaseID;
		colobj->InitInternalData();

		m_arrColShapes.Add(colobj);
	}

	///--- ACTIVES - decorations ---
	//read path
	OS_freadString(fl, charArr);
	//--- actives ---
	int decocnt = (int)OS_freadUInt32(fl);
	int nDecoCntOld = m_arrActives.GetSize();
	for (int kk = 0; kk < decocnt; kk++)
	{
		D3DXVECTOR2 vOffset(nPosXtiles * tileW - nPrefabOriginX, nPosYtiles * tileH - nPrefabOriginY);
		CActive* obj = new CActive();

		obj->ID = OS_freadUInt32(fl) + dwBaseID;
		if (obj->ID > dwMaxIDlocal)
			dwMaxIDlocal = obj->ID;
		//convert layer from editor values to game values (editor misses MID layer):
		byte nLayer = OS_freadByte(fl);
		obj->nLayer = nLayer;
		//position (used to load UINT32)
		obj->pos.x = (float)OS_freadInt32(fl);
		obj->pos.y = (float)OS_freadInt32(fl);
		//move by spawn pos
		obj->pos += vOffset;

		obj->pos_ini = obj->pos;
		//animation
		CHAR charAnmName[MAX_PATH];
		OS_freadString(fl, charAnmName);
		int animIdx = m_sprActives.getAnimationIdxByName(charAnmName);
		if (animIdx < 0)
			ErrorBox(K_ERR_WARNING, L"Active ID:%d without animation!", obj->ID);
		//frame
		int frameIdx = OS_freadUInt16(fl);
		obj->sprite.Init(animIdx, obj->pos.x, obj->pos.y, frameIdx);
		obj->nAnim_ini = animIdx;
		obj->nFrame_ini = frameIdx;
		obj->color = 0xffffffff;
		obj->sprite.color = obj->color;
		obj->bStandsOut = false;
		//angle
		obj->fAngle = 0.0f;
		obj->fAngle_ini = 0.0f;
		//load flags and split
		UINT32 activFlags = OS_freadUInt32(fl);
		//flip xy
		obj->flipX = ((activFlags & K_EDITOR_ACTIVE_FLAG_FLIPX) != 0);
		obj->flipY = ((activFlags & K_EDITOR_ACTIVE_FLAG_FLIPY) != 0);
		//animated
		obj->bAnimated = ((activFlags & K_EDITOR_ACTIVE_FLAG_ANIMATED) != 0);
		obj->bReleaseIt = false;
		//animated? select different start frame
		if (obj->bAnimated)
		{
			obj->sprite.currentFrame = m_rand.RandInt(m_sprActives.GetAFramesCnt(obj->sprite.animationIdx));
		}
		//bbox
		RECTXYWH bbox_set = m_sprActives.GetAFrameBBox(animIdx, frameIdx);
		RECTXYWH objbox = m_sprActives.GetAFrameBBox_real(animIdx, frameIdx);
		obj->bbox_ini.Set(objbox);
		obj->bbox_exported_ini.Set(bbox_set);
		//daca e flipat pe X flipez si bbox. Pe Y nu e cazul pt ca se pastreaza in acelasi bbox in paint
		if (obj->flipX)
		{
			obj->bbox_ini.Move(D3DXVECTOR2(-2.0f * obj->bbox_ini.vCenter.x, 0.0f));
			obj->bbox_exported_ini.Move(D3DXVECTOR2(-2.0f * obj->bbox_exported_ini.vCenter.x, 0.0f));
		}
		obj->bbox = obj->bbox_ini;
		obj->bbox.Move(obj->pos);

		obj->bbox_exported = obj->bbox_exported_ini;
		obj->bbox_exported.Move(obj->pos);

		//load logic and init data
		obj->LoadLogic(fl);
		//change target ids
		if(obj->targetID_ini >= 0)
			obj->targetID_ini += dwBaseID;
		obj->InitInternalData();

		m_arrActives.Add(obj);

		///--- special settings ---
		//#HACK: is it a door? set special AI
		CStringHash shAnimName(charAnmName);
		const CStringHash shAnimDoors(L"DOORS_SECTION");
		if (shAnimName == shAnimDoors)
		{
			obj->AIstate = K_AI_STATE_ACTIVE_DOOR_SECTION;
			if (obj->script_hash.IsEqual(L"ACTIVE_LOCKED_BREAKABLE"))
			{
				obj->varAIparams.SetNamedVarINT32(L"n_locked", 1);
				obj->varAIparams.SetNamedVarFloat(L"f_lockpickTime", 2.5f);
			}
		}
		//--- zombie spawners ---
		if (obj->AIstate == K_AI_STATE_ACTIVE_ZOMBIE_SPAWNER)
		{
			m_arrStats[K_LVL_STATS_ZOMBIE_PORTALS]++;
		}
	}

	///--- load actors ---
	//read path
	OS_freadString(fl, charArr);
	int actorscnt = (int)OS_freadUInt32(fl);
	int nActorsCntOld = m_arrActors.GetSize();
	for (int kk = 0; kk < actorscnt; kk++)
	{
		D3DXVECTOR2 vOffset(nPosXtiles * tileW - nPrefabOriginX, nPosYtiles * tileH - nPrefabOriginY);
		UINT32 actID = OS_freadUInt32(fl) + dwBaseID;
		if (actID > dwMaxIDlocal)
			dwMaxIDlocal = actID;
		//pozitia
		D3DXVECTOR2 actPos;
		actPos.x = (float)OS_freadInt32(fl);
		actPos.y = (float)OS_freadInt32(fl);
		//add spawning offset
		actPos += vOffset;
		//boolean SetAngle si unghi
		bool bSetActorAngle = (OS_freadByte(fl) != 0) ? true : false;
		float fActorAngle = DEG_TO_RAD(OS_freadInt16(fl));
		//read template name
		CHAR readstr[MAX_PATH];
		WCHAR templateNameW[MAX_PATH];
		OS_freadString(fl, readstr);
		mbstowcs(templateNameW, readstr, MAX_PATH);
		//read selected AI state from editor
		WCHAR stateNameW[MAX_PATH];
		OS_freadString(fl, readstr);
		mbstowcs(stateNameW, readstr, MAX_PATH);
		//direction
		bool bactLookleft = (OS_freadByte(fl) != 0) ? true : false;
		bool bactCollision = (OS_freadByte(fl) != 0) ? true : false;
		bool bactGravity = (OS_freadByte(fl) != 0) ? true : false;
		//logic
		byte n1b = OS_freadByte(fl);
		bool bactCanInteract = (n1b & 0x1);
		bool bactHideInteract = (n1b & 0x2);
		float factTouchDuration = (float)OS_freadInt32(fl);
		bool bactStartHidden = (OS_freadByte(fl) != 0) ? true : false;
		INT32 nactTargetID = OS_freadInt32(fl);
		if(nactTargetID >= 0)
			nactTargetID += dwBaseID;
		//read script AI name
		CHAR strScriptName[MAX_PATH];
		CHAR strAIname[MAX_PATH];
		OS_freadString(fl, strScriptName);
		OS_freadString(fl, strAIname);
		//read AI params
		CVariantCollection arrParams;
		int nAIparamsCnt = OS_freadByte(fl); //nr params
		if (nAIparamsCnt > 0)
		{
			for (int i = 0; i < nAIparamsCnt; i++)
			{
				CHAR varname[MAX_PATH] = { 0 };
				WCHAR wvarname[MAX_PATH] = { 0 };
				CHAR varval[MAX_PATH];
				WCHAR wvarval[MAX_PATH];

				OS_freadString(fl, varname);
				OS_freadString(fl, varval);

				size_t convnr;
				mbstowcs_s(&convnr, wvarname, varname, MAX_PATH);
				mbstowcs_s(&convnr, wvarval, varval, MAX_PATH);

				arrParams.SetNamedVarAUTO(wvarname, wvarval);
			}
		}
		///--- FINISHED READING DATA ---
		CActor* nact = null;
		CStringHash shState(stateNameW);
		if(!shState.IsEmpty())
			nact = SpawnActor(actPos, templateNameW, ((bactLookleft) ? -1 : 1), &shState);
		else
			nact = SpawnActor(actPos, templateNameW, ((bactLookleft) ? -1 : 1));
		//force local ID
		nact->ID = actID;
		//append the editor AI params as some of them are set from the AI function
		nact->varAIparams.AppendCollection(arrParams);
	}

	//#TODO: handle misc objects too if needed

	OS_fclose(fl);

	//! Set global ID to safe value
	m_unLastID = dwMaxIDlocal + 1;

	///--- everything loaded, SetAI here ---
	for (int kk = nLightsCntOld; kk < m_arrLights.GetSize(); kk++)
	{
		CLight * light = m_arrLights[kk];
		SetAI(light, light->AIstate, &light->varAIparams, light->targetID_ini);
	}
	for (int kk = nCollisionsCntOld; kk < m_arrColShapes.GetSize(); kk++)
	{
		CCollisionShape * shape = m_arrColShapes[kk];
		SetAI(shape, shape->AIstate, &shape->varAIparams, shape->targetID_ini);
	}
	for (int kk = nDecoCntOld; kk < m_arrActives.GetSize(); kk++)
	{
		CActive * activ = m_arrActives[kk];
		SetAI(activ, activ->AIstate, &activ->varAIparams, activ->targetID_ini);
	}
	//ma asigur ca toti actorii au pointerii setati bine chemand inca odata setAI
	for (int kk = nActorsCntOld; kk < m_arrActors.GetSize(); kk++)
	{
		CActor* actor = m_arrActors[kk];
		SetAI(actor, actor->AIstate, &actor->varAIparams, actor->targetID_ini);
	}

	return S_OK;
}

void CLevel::ClearVisibilityLists()
{
	for (int jj = 0; jj < K_LVL_LAYERS_CNT; jj++)
	{
		m_visibleList.visible_actives[jj].Clear();
		m_visibleList.logic_actives_closeby[jj].Clear();
	}

	for (int kk = 0; kk < K_LVL_DECAL_LAYERS; kk++)
		m_visibleList.visible_decals[kk].Clear();

	m_visibleList.visible_actors.Clear();
	m_visibleList.visible_lights.Clear();
	m_visibleList.visible_colShapesLights.Clear();

	m_visibleList.logic_actors_closeby.Clear();
	m_visibleList.logic_colShapes.Clear();
	m_visibleList.logic_colShapesExtended.Clear();
	m_visibleList.logic_colShapesSpecial.Clear();
}

void CLevel::SetLevelState(ELevelState eNewState, int nLevelStateParam)
{
	//set actual state
	m_levelState = eNewState;
	switch (m_levelState)
	{
		case K_LVL_STATE_PLAYING:
		{
			// save level start time
			m_arrStats[K_LVL_STATS_LEVEL_START_SEC] = (int)floor(fLocalTimeline);

			m_levelSubState = 0;
			m_levelStateTimer = 0.0f;
			//intro chars verses
			nIntroVerseState = 0;

			CHAR ctxt[MAX_PATH];
			int nLevelIdx = g_userData[K_MEMID_SELECTED_LEVEL] + g_userData[K_MEMID_SELECTED_CHAPTER] * K_GAME_LEVELS_PER_CHAPTER;
			StringCchPrintfA(ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1);
			//add zombie mode to level name
			if (g_gameMode == GAME_MODE_ZOMBIE_INVASION)
				StringCchCatA(ctxt, MAX_PATH, "_zm");

			if (UTGetAppClass().IsGameNetworked())
			{
				//mark sync start here, after loading the game
				UTGetAppClass().m_Settings.devnet_eSyncStatus = CApplicationSettings::K_NETGAME_SYNC_GET_READY;
				//send loaded level confirmation
				g_netlock.Net_SendGameplayCommand(g_netlock.K_GAMPLAYCMD_LEVEL_LOADED);

#ifdef ENABLE_CHAT_WINDOW
				//say: "press ENTER to chat"
				g_ChatWnd.AddLine(g_stringsMgr.strings[STR_ENTER_TO_CHAT]->sText, L"SYSTEM", K_CW_SYSTEM_COLOR);
#endif
				LOG(L"Level::SetLevelState - Started networked game!");
				if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
					ANALYTICS_EVENT("level_start_net", ctxt, "playedTimes", g_levelStats[nLevelIdx].nPlayedTimes);
				else //custom level
				{
					StringCchPrintfA(ctxt, MAX_PATH, "lvlflag_%d", m_unLoadedLevelFlags);
					ANALYTICS_EVENT("level_start_net_custom", ctxt, "val", 0);
				}
			}
			else
			{
				LOG(L"Level::SetLevelState - Started game!");
				if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
					ANALYTICS_EVENT("level_start", ctxt, "playedTimes", g_levelStats[nLevelIdx].nPlayedTimes);
				else //custom level
				{
					StringCchPrintfA(ctxt, MAX_PATH, "lvlflag_%d", m_unLoadedLevelFlags);
					ANALYTICS_EVENT("level_start_custom", ctxt, "val", 0);
				}
			}

		}
		break;
		case K_LVL_STATE_MISSION_ACCOMPLISHED:
		{
#ifdef ENABLE_CHAT_WINDOW
			g_ChatWnd.CancelInput();
#endif
			//save level finished time
			m_arrStats[K_LVL_STATS_LEVEL_END_SEC] = (int)floor(fLocalTimeline);
			//remove any interfaces that might be shown
			UTGetControlsManager().RemoveAllLayers();

			SND_STOP_GROUP("music", false, true);

			//SND_PLAY_ONCE(SNDIDX_ANNOUNCER_WIN, 0);
			//SND_PLAY_ONCE(SNDIDX_STINGER_WIN, 0);

			m_levelSubState = 0;
			m_levelStateTimer = 0.0f;

			g_particlesMgr.AddStringDummy(K_PDUMMY_STRING_WIDEBAR, D3DXVECTOR2(0.0f, -50.0f), STR_MISSION_ACCOMPLISHED, FONTIDX_12_WOW, 1.0f, 1.8f, K_COLOR_SELECTED_TEXT);
			//enter level results sync
			if (UTGetAppClass().IsGameNetworked())
			{
				LOG(L"Net::Level: Signal mission accomplished.");
				g_netlock.Net_EnterLevelResults();
			}

			// activate coop achievement on local matches too
			int nPlayers = m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED] + m_arrStats[K_LVL_STATS_PL2_HAS_PLAYED];
			if (nPlayers > 1)
			{
				App_IncreaseGamestat(K_MEMID_GAMESTATS_COOP_GAMES_WON);
			}
			
			//remove hot join
			for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
			{
				if (pPlayerActor[kk] != null)
				{
					m_arrPlayerSelStrategic[kk] = -1;
					m_interfaceIGM.SetStrategicSelection(kk, -1);
					pPlayerActor[kk]->SetIcon(K_LVL_ACT_ICON_NONE);

				}

				if ((m_arrPlayerSelHotJoin[kk] != -1) && (pPlayerActor[kk] == null))
				{
					m_arrPlayerControllersIIDs[kk] = -1;
					m_arrPlayerSelHotJoin[kk] = -1;
					g_playerSelScr.m_arrPlayers[kk].nInstanceID = -1;
					m_interfaceIGM.SetHotJoinSelection(kk, m_arrPlayerSelHotJoin[kk]);
				}
			}

			//#ACHIEVEMENTS: check achievement ninja
			for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
			{
				if ((pPlayerActor[kk] != null) && (!IsNetworkPlayer(pPlayerActor[kk])) && 
					(m_arrStats[K_LVL_STATS_PL1_DAMAGE_TAKEN + kk * K_LVL_STATS_PLAYER_STATS_COUNT] == 0) && 
					(m_arrStats[K_LVL_STATS_PL1_DEATHS + kk * K_LVL_STATS_PLAYER_STATS_COUNT] == 0))
				{
					UTGetAchievementManager().UnlockAchievement(ACH_NINJA);
					break;
				}
			}
		}
		break;
		case K_LVL_STATE_MISSION_FAILED:
		{
#ifdef ENABLE_CHAT_WINDOW
			g_ChatWnd.CancelInput();
#endif
			//save level finished time
			m_arrStats[K_LVL_STATS_LEVEL_END_SEC] = (int)floor(fLocalTimeline);
			//remove any interfaces that might be shown
			UTGetControlsManager().RemoveAllLayers();

			SND_STOP_GROUP("music", false, true);

			//SND_PLAY_ONCE(SNDIDX_ANNOUNCER_FAIL, 0);
			//SND_PLAY_ONCE(SNDIDX_STINGER_LOSE, 0);

			m_levelStateParam = nLevelStateParam; //reason why failed - stringIDX
			m_levelSubState = 0;
			m_levelStateTimer = 0.0f;

			if(g_gameMode == GAME_MODE_INFINITE_TOWER)
				g_particlesMgr.AddStringDummy(K_PDUMMY_STRING_WIDEBAR, D3DXVECTOR2(0.0f, -50.0f), STR_YOU_DIED, FONTIDX_12_WOW, 1.0f, 1.8f, K_COLOR_SELECTED_TEXT);
			else
				g_particlesMgr.AddStringDummy(K_PDUMMY_STRING_WIDEBAR, D3DXVECTOR2(0.0f, -50.0f), STR_MISSION_FAILED, FONTIDX_12_WOW, 1.0f, 1.8f, K_COLOR_SELECTED_TEXT);
			//enter level results sync
			if (UTGetAppClass().IsGameNetworked())
			{
				LOG(L"Net::Level: Signal mission failed.");
				g_netlock.Net_EnterLevelResults();
			}

			//remove hot join and strategic menu
			for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
			{
				if (pPlayerActor[kk] != null)
				{
					m_arrPlayerSelStrategic[kk] = -1;
					m_interfaceIGM.SetStrategicSelection(kk, -1);
					pPlayerActor[kk]->SetIcon(K_LVL_ACT_ICON_NONE);

				}
				if ((m_arrPlayerSelHotJoin[kk] != -1) && (pPlayerActor[kk] == null))
				{
					m_arrPlayerControllersIIDs[kk] = -1;
					m_arrPlayerSelHotJoin[kk] = -1;
					g_playerSelScr.m_arrPlayers[kk].nInstanceID = -1;
					m_interfaceIGM.SetHotJoinSelection(kk, m_arrPlayerSelHotJoin[kk]);
				}
			}
		}
		break;
		
		default:
			LOG(L"[WARNING] Net::Level - SetLevelState state %d not handled!", eNewState);
			break;
	}
}

void CLevel::SetTimeMultiplier(float fMultiplier, float fDuration)
{
	m_fTimeMultiplier = fMultiplier;
	m_fTimeMultiplierDuration = fDuration;
	//la reset nu am durata
	if (fMultiplier == 1.0f)
		m_fTimeMultiplierDuration = 0.0f;

	//play sound
	if (fMultiplier < 1.0f)
	{
		SND_PLAY_ONCE(SNDIDX_TIME_SLOW, 0);
		SND_PLAY_ONCE(SNDIDX_HEARTBEAT, DSBPLAY_LOOPING);
	}
	else if (fMultiplier >= 1.0f)
	{
		SND_STOP(SNDIDX_HEARTBEAT, true);
	}
}

//used to save last player positions
static D3DXVECTOR2 s_vLastPlayerPos[K_MAX_PLAYERS_CNT];

bool CLevel::NormalizeMouseCoords(int ControllerIID, float fAxisValue, bool bIsHorizontalAxis, float & ret_fAxisValue)
{
	ret_fAxisValue = fAxisValue;
	// level not loaded? return same coordinates
	if (!m_bLoaded)
		return false;

	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if (m_arrPlayerControllersIIDs[kk] == ControllerIID) {
			CActor* pPlayer = pPlayerActor[kk];
			if (pPlayerActor == null)
			{
				ErrorBox(K_ERR_WARNING, L"NormalizeMouseCoords player pointer is missing! idx:", kk);
				return false;
			}

			if (bIsHorizontalAxis)
			{
				D3DXVECTOR2 retpt = m_camLevel.ScreenToWorld(D3DXVECTOR2(fAxisValue, 0.0f));
				// make coords relative to player
				retpt.x -= pPlayer->pos.x;
				// set final coords
				ret_fAxisValue = retpt.x;
				return true;
			}
			else
			{
				D3DXVECTOR2 retpt = m_camLevel.ScreenToWorld(D3DXVECTOR2(0.0f, fAxisValue));
				// make coords relative to player
				retpt.y -= pPlayer->pos.y;
				// set final coords
				ret_fAxisValue = retpt.y;
				return true;
			}
		}
	}

	ErrorBox(K_ERR_WARNING, L"NormalizeMouseCoords couldn't find player with ControllerIID:%d", ControllerIID);
	return false;
}

void CLevel::BuildVisibilityLists()
{
	//level aabb
	CAABB lvlAABB;
	lvlAABB.Set(m_levelAABB);
	///--- visual stuff - depends only on camaabb ---
	RECTXYWH_F camrect_old = m_camLevel.GetCamWorldAABB();
	//build a camera view rectangle constant across different resolutions so it doesn't desync when on multiplayer
	//it will need intervention if camera constraint changes axis in order to maintain maximum visible area
	SIZEWH_F camrectsz(K_GAME_WIDTH, K_GAME_HEIGHT_MAX);
	RECTXYWH_F camrect(camrect_old.CenterX() - camrectsz.w * 0.5f, camrect_old.CenterY() - camrectsz.h * 0.5f, camrectsz.w, camrectsz.h);
	//maximize camrect vertically
	CAABB camaabb(D3DXVECTOR2(camrect.x, camrect.y), D3DXVECTOR2(camrect.Right(), camrect.Bottom()));
	//union of all visible lights AABBs
	CAABB lightsCommonAABB(D3DXVECTOR2(-1000.0f, -1000.0f), D3DXVECTOR2(-1000.0f, -1000.0f)); 
	//for detecting visible actives (objects)
	CAABB activesPaintAABB = camaabb; 
	//for detecting visible actors
	CAABB actorsPaintAABB = camaabb; //box-ul care zice daca actorul e vizibil sau nu
	actorsPaintAABB.Inflate(D3DXVECTOR2(K_TILE_SIZE, K_TILE_SIZE)); //maresc putin bboxul actorilor pt ca cei morti au bbox mai mic

	ClearVisibilityLists();

	///----- logical stuff - depends on both players -----
	//filter only useful collision boxes here (bullets intersections and such)
	CAABB collisionAreaAABBs[K_MAX_PLAYERS_CNT];
	//larger boxes
	CAABB collisionAreaAABBs_extended[K_MAX_PLAYERS_CNT];
	//filter only closeby actors (events triggering, bullets collisions etc)
	CAABB actorsNearbyAABBs[K_MAX_PLAYERS_CNT];
	//filter closeby actives
	CAABB activesNearbyAABBs[K_MAX_PLAYERS_CNT];
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		//saves the last position so it doesn't go crazy when the players get freed
		if (pPlayerActor[kk] != null)
		{
			s_vLastPlayerPos[kk] = pPlayerActor[kk]->pos;
		}
		//get a 2 screen area around each player's last pos (could be smaller, yes)
		collisionAreaAABBs[kk].Set(s_vLastPlayerPos[kk] - camaabb.vSize, s_vLastPlayerPos[kk] + camaabb.vSize);
		AABB_KeepInside(collisionAreaAABBs[kk], lvlAABB);
		//double that area to be sure (needed sometimes)
		collisionAreaAABBs_extended[kk].Set(s_vLastPlayerPos[kk] - camaabb.vSize * 2.0f, s_vLastPlayerPos[kk] + camaabb.vSize * 2.0f);
		AABB_KeepInside(collisionAreaAABBs_extended[kk], lvlAABB);
		//a larger area for events activation and such (slightly smaller area)
		actorsNearbyAABBs[kk].Set(s_vLastPlayerPos[kk] - camaabb.vHalfSize * 1.5f, s_vLastPlayerPos[kk] + camaabb.vHalfSize * 1.5f);
		AABB_KeepInside(actorsNearbyAABBs[kk], lvlAABB);

		//filter actives area
		activesNearbyAABBs[kk] = actorsNearbyAABBs[kk];
	}

	//flag used for rendering:
	bool bFirstShadowingLightSet = false;
	//select lights that have AABBs that touch the visible area
	m_visibleList.visible_lights.Clear();
	for (int kk = 0; kk < m_arrLights.GetSize(); kk++)
	{
		const CLight* light = m_arrLights[kk];
		//daca e ascunsa o sare
		if (light->bHidden)
			continue;

		if (light->type == K_LVL_LIGHT_AMBIENTAL)
		{
			m_visibleList.visible_lights.Add(m_arrLights[kk]);
			continue;
		}

		if (camaabb.IntersectsCircle(light->pos, light->fMaxRadius))
		{
			//if we have hidden room clip area on, ignore the lights outside
			if ((m_HiddenRoomAABB.vSize.x > 0.0f) && (m_HiddenRoomAABB.vSize.y >= 0.0f))
				if (!m_HiddenRoomAABB.PointIn(light->pos))
					continue;

			if (m_visibleList.visible_lights.Add(m_arrLights[kk]) < 0)
				break;
			if (m_arrLights[kk]->castShadows)
			{
				CAABB bbox_max(D3DXVECTOR2(light->pos.x - light->fMaxRadius, light->pos.y - light->fMaxRadius), D3DXVECTOR2(light->pos.x + light->fMaxRadius, light->pos.y + light->fMaxRadius));
				//la prima lumina cu shadow seteaza lightsCommonAABB fix pe bbox-ul luminii
				if (bFirstShadowingLightSet == false)
				{
					bFirstShadowingLightSet = true;
					lightsCommonAABB = bbox_max;
				}
				else
				{
					lightsCommonAABB = AABB_Union(lightsCommonAABB, bbox_max);
				}
			}
		}
	}
	//toate bbox-urile care intra in actiunea luminilor care fac shadow casting
	m_visibleList.visible_colShapesLights.Clear();
	m_visibleList.logic_colShapes.Clear();
	m_visibleList.logic_colShapesExtended.Clear();
	m_visibleList.logic_colShapesSpecial.Clear();
	for (int kk = 0; kk < m_arrColShapes.GetSize(); kk++)
	{
		//selectez bboxurile pt coliziune (cele din ecran momentan)
		if (m_arrColShapes[kk]->bHidden)
			continue;
		
		switch (m_arrColShapes[kk]->type)
		{
			//case K_LVL_COLL_TYPE_TRIGGER:
			case K_LVL_COLL_TYPE_FOG_OF_WAR:
			case K_LVL_COLL_TYPE_WATER:
			case K_LVL_COLL_TYPE_COVER:
			{
				//le selectez doar pe cele din ecran
				if ((collisionAreaAABBs[0].Intersects(&m_arrColShapes[kk]->bbox)) || (collisionAreaAABBs[1].Intersects(&m_arrColShapes[kk]->bbox)))
					m_visibleList.logic_colShapesSpecial.Add(m_arrColShapes[kk]);
			}
			break;
			case K_LVL_COLL_TYPE_LEDGE:
			case K_LVL_COLL_TYPE_SOLID:
			{

				//commented intersection with larger area so we add them all
				//if ((collisionAreaAABBs_extended[0].Intersects(&m_arrColShapes[kk]->bbox)) || (collisionAreaAABBs_extended[1].Intersects(&m_arrColShapes[kk]->bbox)))
				{
					//#TODO: if extended colshapes contains ALL collision shapes we could build it only once
					m_visibleList.logic_colShapesExtended.Add(m_arrColShapes[kk]);
					//add all doors/windows to a special list so we can check them rapidly when breaking them with explosions
					if ((m_arrColShapes[kk]->AIstate == K_AI_STATE_COLL_BREAKABLE_WINDOW) ||
						(m_arrColShapes[kk]->AIstate == K_AI_STATE_COLL_BREAKABLE_DOOR))
					{
						m_visibleList.logic_colShapesSpecial.Add(m_arrColShapes[kk]);
					}
					//gather all important collision boxes (nearby) - check only if included in extended area which is larger but still centered
					if ((collisionAreaAABBs[0].Intersects(&m_arrColShapes[kk]->bbox)) || (collisionAreaAABBs[1].Intersects(&m_arrColShapes[kk]->bbox)))
					{
						m_visibleList.logic_colShapes.Add(m_arrColShapes[kk]);
					}
				}
			}
			break;
		}
		//find bboxes that can cast shadows
		if (bFirstShadowingLightSet)
		{
			//bagam doar pe cele care fac umbra
			if (!m_arrColShapes[kk]->castShadows)
				continue;
			if (lightsCommonAABB.Intersects(&m_arrColShapes[kk]->bbox))
			{
				m_visibleList.visible_colShapesLights.Add(m_arrColShapes[kk]);
			}
		}
	}
	//actorii vizibili
	m_visibleList.visible_actors.Clear();
	m_visibleList.logic_actors_closeby.Clear();
	for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
	{
		CActor* actor = m_arrActors[kk];
		if ((actor->bHidden) || (actor->bSkipRender))
			continue;
		//is it nearby?
		if ((actorsNearbyAABBs[0].Intersects(&actor->bbox)) || (actorsNearbyAABBs[1].Intersects(&actor->bbox)))
		{
			m_visibleList.logic_actors_closeby.Add(actor);
		}
		//must be painted?
		if (actorsPaintAABB.Intersects(&actor->bbox))
		{
			//#PERSONALIZARE: don't draw actors under FOW
			bool bUnderFOW = false;
			for (int jj = 0; jj < m_visibleList.logic_colShapesSpecial.Count(); jj++)
			{
				CCollisionShape* colshape = m_visibleList.logic_colShapesSpecial.m_pData[jj];
				//daca e alt tip de collision sau daca a fost descoperit
				if (colshape->type != K_LVL_COLL_TYPE_FOG_OF_WAR)
					continue;
				else if (colshape->AIfvar1 < 1.0f)
					continue;

				if (m_visibleList.logic_colShapesSpecial.m_pData[jj]->bbox.PointIn(actor->GetPosHeart()))
				{
					bUnderFOW = true;
				}
			}

			//adaug in lista de paint doar daca nu sunt sub FOW
			if (!bUnderFOW)
				m_visibleList.visible_actors.Add(actor);
		}
	}
	//all actives onscreen for rendering
	for (int jj = 0; jj < K_LVL_LAYERS_CNT; jj++)
	{
		m_visibleList.visible_actives[jj].Clear();
		m_visibleList.logic_actives_closeby[jj].Clear();
	}

	for (int kk = 0; kk < m_arrActives.GetSize(); kk++)
	{
		if (m_arrActives[kk]->bHidden)
			continue;
		CActive* active = m_arrActives[kk];
		//visible actives
		if (activesPaintAABB.Intersects(&active->bbox))
		{
			m_visibleList.visible_actives[(int)active->nLayer].Add(active);
			//#PERSONALIZARE: bomb was seen? let us know
			if ((m_arrStats[K_LVL_STATS_LEVEL_HAS_BOMBS] != 0) && (m_arrStats[K_LVL_STATS_LEVEL_BOMB_SEEN] == 0))
			{
				if (m_arrActives[kk]->ID == m_arrStats[K_LVL_STATS_LEVEL_BOMB_ID])
				{
					m_arrStats[K_LVL_STATS_LEVEL_BOMB_SEEN] = 1;
					//find first valid player
					CActor* pPlayer = GetClosestPlayer(active->pos);
					if(pPlayer != null)
						PlayActorSoundVerse(pPlayer, K_LVL_ACT_VERSE_BOMB_LOCATED);
				}
			}
		}
		//logical closeby actives
		if ((activesNearbyAABBs[0].Intersects(&active->bbox)) || (activesNearbyAABBs[1].Intersects(&active->bbox)))
		{
			m_visibleList.logic_actives_closeby[(int)active->nLayer].Add(active);
		}
	}
	//clear all decals layers
	for (int kk = 0; kk < K_LVL_DECAL_LAYERS; kk++)
	{
		m_visibleList.visible_decals[kk].Clear();
	}

	for (int kk = 0; kk < m_arrDecals.GetSize(); kk++)
	{
		if (activesPaintAABB.Intersects(&m_arrDecals[kk]->aabb))
		{
			m_visibleList.visible_decals[m_arrDecals[kk]->layer].Add(m_arrDecals[kk]);
		}
	}
}

///--------------------------------------------------------------------------------
///--- AI UPDATES ---
///--------------------------------------------------------------------------------
//updates AI for base class (common AIs) - se cheama pe default in updateAI pt fiecare clasa
//aici sunt implementate functiile comune tuturor claselor
bool CLevel::UpdateAI_base(IActiveInterface* active, float dTime, double fTimeline)
{
	bool bProcessedState = true;

	///--- check AI states ---
	switch (active->AIstate)
	{
		case K_AI_STATE_FN_TOUCH_WHEN_SEE_PLAYER:
		{
			dec_limit(active->AItimer1, dTime, 0.0f);
			if ((active->AItimer1 <= 0.0f) && (active->pTarget != null))
			{
				for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
				{
					if (pPlayerActor[kk] == null)
						continue;

					//verifica sa fie in raza vizuala
					D3DXVECTOR2 vActPl = pPlayerActor[kk]->posHeart - active->pos;
					float fActPlLen = D3DXVec2Length(&vActPl);
					if (fActPlLen > active->AIfvar3) //radius
						break;
					//normalize
					//vActPl /= fActPlLen;
					float fPlayerAng = Math_GetVectorAngle(vActPl);
					fPlayerAng -= active->AIfvar1; //angle
					//#TODO: is direct line of sight?
					//is in fov?
					if (fabs(fPlayerAng) < active->AIfvar2)
					{
						active->AItimer1 = active->varAIparams.GetVariantByName(L"f_cooldownSec")->m_asFloat;
						//touch itself
						//active->pTarget->Touch(active->GetUID());
						active->Touch(pPlayerActor[kk]->GetUID(), dTime);
						break;
					}

				}
			}
		}
		break;
		case K_AI_STATE_TRIGGER_IN_OUT:
		{

#if defined(_DEBUG) || defined(DEBUG)
			if (active->AIstate == K_AI_STATE_UNDEFINED)
			{
				ErrorBox(K_ERR_WARNING, L"Trigger without AI. Please set AI state!");
				break;
			}
#endif

			bool bTrigger = false;
			//triggered by players
			if (active->AIfvar1 != 0.0f)
			{
				for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
				{
					if ((pPlayerActor[kk] != NULL) && (!pPlayerActor[kk]->bHidden) && (pPlayerActor[kk]->bbox.Intersects(&active->bbox)))
					{
						bTrigger = true;
						break;
					}
				}
			}
			//triggered by enemies
			if (active->AIfvar2 != 0.0f)
			{
				for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
				{
					//skip actors that are: hidden, dead, players or not a target
					if ((m_arrActors[kk]->bHidden) || (m_arrActors[kk]->fLife <= 0.0f) || (m_arrActors[kk]->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER) ||
						((m_arrActors[kk]->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET) != 0))
							continue;
					if (m_arrActors[kk]->bbox.Intersects(&active->bbox))
					{
						bTrigger = true;
						break;
					}
				}
			}
			//see if toggled and run scripts
			if ((active->AIvar1 == 0) && (bTrigger))
			{
				//On Enter
				active->Touch(active->GetUID(), 0.0f);
				active->AIvar1 = 1;
			}
			else if ((active->AIvar1 != 0) && (!bTrigger) && (active->AIstrvar1.textHash != 0))
			{
				//On Leave if script present
				active->Touch(active->GetUID(), 0.0f, active->AIstrvar1.textHash);
				active->AIvar1 = 0;
			}
		}
		break;

		case K_AI_STATE_FN_POS_ELLIPSE:	//f_radX, f_radY, f_timeMul
		{
			//check number of params
			if (active->varAIparams.GetVariantCount() < 3)
			{
				LOG(L"UpdateAI_base::ID:%d class:%d needs more AI params", active->ID, active->GetClassType());
				break;
			}
			float radX = active->varAIparams[0]->asFloat();
			float radY = active->varAIparams[1]->asFloat();
			float timeMul = active->varAIparams[2]->asFloat();

			D3DXVECTOR2 delta = D3DXVECTOR2(radX * cos(timeMul * fTimeline), radY * sin(timeMul * fTimeline));
			active->SetPos(active->pos_ini + delta);
		}
		break;
		case K_AI_STATE_FN_ANG_SIN_TIME:
		{
			//check number of params
			if (active->varAIparams.GetVariantCount() < 4)
			{
				LOG(L"UpdateAI_base::ID:%d class:%d needs more AI params", active->ID, active->GetClassType());
				break;
			}
			float fmin = active->varAIparams[0]->asFloat();
			float fmax = active->varAIparams[1]->asFloat();
			float timeMul = active->varAIparams[2]->asFloat();
			float timeAdd = active->varAIparams[3]->asFloat();

			float dangle = fmin + (fmax - fmin) * ((sin(fLocalTimeline * timeMul + timeAdd) + 1.0f) / 2.0f);
			active->fAngle = active->fAngle_ini + dangle;
		}
		break;
		case K_AI_STATE_FN_ALPHA_SIN_TIME:
		{
			//check number of params
			if (active->varAIparams.GetVariantCount() < 3)
			{
				LOG(L"UpdateAI_base::ID:%d class:%d needs more AI params", active->ID, active->GetClassType());
				break;
			}
			float fmin = active->varAIparams[0]->asFloat();
			float fmax = active->varAIparams[1]->asFloat();
			float timeMul = active->varAIparams[2]->asFloat();
			float timeAdd = active->varAIparams[3]->asFloat();

			float falpha = fmin + (fmax - fmin) * ((sin(fLocalTimeline * timeMul + timeAdd) + 1.0f) / 2.0f);
			active->color = D3DCOLOR_COLORALPHA(active->color_ini, falpha);
		}
		break;
		case K_AI_STATE_FN_GET_TARGET_POS:
		{
			if (active->pTarget != NULL)
			{
				D3DXVECTOR2 targetDelta = active->pTarget->pos - active->pTarget->pos_ini;
				//mut obiectul cu delta totala a targetului
				active->SetPos(active->pos_ini + targetDelta);
			}
		}
		break;
		case K_AI_STATE_FN_GET_TARGET_ANG:
		{
			if (active->pTarget != NULL)
			{
				D3DXVECTOR2 targetVec = active->pos_ini - active->pTarget->pos_ini;
				D3DXMATRIXA16 matrot;
				D3DXMatrixRotationZ(&matrot, active->pTarget->fAngle - active->pTarget->fAngle_ini);
				D3DXVec2TransformCoord(&targetVec, &targetVec, &matrot);
				//mut obiectul cu delta totala a targetului
				active->SetPos(active->pTarget->pos_ini + targetVec);
				active->SetAngle(active->fAngle_ini + (active->pTarget->fAngle - active->pTarget->fAngle_ini));
			}
		}
		break;
		case K_AI_STATE_FN_FOLLOW_TARGET_RAIL:
		{
			//declar statice variabilele cu prescurtarea parametrilor si le accesez din variantcollection mereu sau poate chiar pointeri CVariantComplex
			static const UINT32 hash_v_railPtr = FastHash(L"railPtr");
			static const UINT32 hash_n_dir = FastHash(L"n_dir");
			static const UINT32 hash_f_pointPauseSec = FastHash(L"f_pointPauseSec");
			static const UINT32 hash_b_autoChangeDirection = FastHash(L"b_autoChangeDirection");
			//get params
			CMiscObjectRail* rail = null;
			//continuam cu procesarea
			CVariantComplex* railvc = active->varAIparams.GetVariantByNameHash(hash_v_railPtr);
			if (railvc->m_type == CVariantComplex::K_ARGTYPE_NONE)
			{
				ErrorBox(K_ERR_WARNING, L"Rail pointer not found!", active->targetID_ini);
				break;
			}
			//get rail pointer
			rail = static_cast<CMiscObjectRail*>(railvc->m_asVoid);
			//avanseaza	(fara pauza la capete momentan)
			if (active->AItimer2 > 0.0f)
			{
				active->AItimer2 -= dTime;
			}
			else
			{
				//move 
				int movedir = active->varAIparams.GetVariantByNameHash(hash_n_dir)->m_asINT32;
				if (movedir != 0) //movedir == 0 inseamna ca sta pe loc
				{
					  //pozitie rail	    //speed
					active->AItimer1 += active->AIfvar1 * dTime * movedir;

					if (active->AItimer1 >= rail->fLength)
					{
						active->AItimer1 = rail->fLength;
						//is looping? go to the other side of the rail
						if (active->AIvarBool1)
						{
							active->AItimer1 = 0.0f;
						}
						else
						{
							//inverseaza directia daca e pe auto
							if (active->varAIparams.GetVariantByNameHash(hash_b_autoChangeDirection)->m_asINT32 != 0)
							{
								active->varAIparams.SetNamedVarINT32(L"n_dir", -movedir);
							}
						}

						CVariantComplex* waitTimer = active->varAIparams.GetVariantByNameHash(hash_f_pointPauseSec);
						active->AItimer2 = waitTimer->asFloat();
					}
					else if (active->AItimer1 <= 0.0f)
					{
						active->AItimer1 = 0.0f;
						if (active->AIvarBool1)
						{
							active->AItimer1 = rail->fLength;
						}
						else
						{
							//reverse direction only if not looping
							if (active->varAIparams.GetVariantByNameHash(hash_b_autoChangeDirection)->m_asINT32 != 0)
								active->varAIparams.SetNamedVarINT32(L"n_dir", -movedir); //inversam directia
						}
						//reset wait timer
						CVariantComplex* waitTimer = active->varAIparams.GetVariantByNameHash(hash_f_pointPauseSec);
						active->AItimer2 = waitTimer->asFloat();
					}
				}
			}
			D3DXVECTOR2 newpos = rail->GetPos(active->AItimer1);
			active->SetPos(newpos);
		}
		break;
		default:
			bProcessedState = false;
			break;
	}

	return bProcessedState;
}

//updates AI for derived classes (particulare)
//aici sunt implementate functiile particulare fiecarei clase
void CLevel::UpdateAI_collshape(CCollisionShape * colshape, float dTime)
{
	//touch timer reset (nu este necesar)
	//colshape->UpdateTouchTimerReset(dTime);

	//daca am schimbat vizibilitatea
	colshape->bHidden = colshape->bSetHidden;
	//daca este hidden nu mai verifica AI
	if (colshape->bHidden)
		return;

	//update timeline
	colshape->fTimelineAI += dTime;

	///--- generic water mechanics ---
	if (m_Timers.Tick(500))
	{
		if (colshape->type == K_LVL_COLL_TYPE_WATER)
		{
			//players
			for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
			{
				if ((pPlayerActor[kk] == NULL) || (pPlayerActor[kk]->bHidden) || (pPlayerActor[kk]->fLife <= 0.0f) ||
					((pPlayerActor[kk]->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET) != 0) )
					continue;
				D3DXVECTOR2 vCheckPt(pPlayerActor[kk]->bbox.vCenter.x, pPlayerActor[kk]->bbox.vMin.y);
				if (colshape->bbox.PointIn(vCheckPt))
				{
					HitActor(pPlayerActor[kk], 50.0f, 0, K_LVL_ACT_CLASS_TRAP, null,
						K_LVL_BULLET_FLAG_NO_IMPACT_PARTICLES | K_LVL_BULLET_FLAG_NOT_BALLISTIC | K_LVL_BULLET_FLAG_IGNORE_ARMOR | K_LVL_BULLET_FLAG_IGNORE_COVER | K_LVL_BULLET_FLAG_NO_DECALS, 10, 0.0f);
				}
			}
			//other enemies
			for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
			{
				//skip actors that are: hidden, dead, players or not a target
				if ((m_arrActors[kk]->bHidden) || (m_arrActors[kk]->fLife <= 0.0f) || (m_arrActors[kk]->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER) ||
					(m_arrActors[kk]->fLife <= 0.0f) ||
					((m_arrActors[kk]->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET) != 0))
					continue;
				D3DXVECTOR2 vCheckPt(m_arrActors[kk]->bbox.vCenter.x, m_arrActors[kk]->bbox.vMin.y);
				if (colshape->bbox.PointIn(vCheckPt))
				{
					HitActor(m_arrActors[kk], 50.0f, 0, K_LVL_ACT_CLASS_TRAP, null,
						K_LVL_BULLET_FLAG_NO_IMPACT_PARTICLES | K_LVL_BULLET_FLAG_NOT_BALLISTIC | K_LVL_BULLET_FLAG_IGNORE_ARMOR | K_LVL_BULLET_FLAG_IGNORE_COVER | K_LVL_BULLET_FLAG_NO_DECALS, 10, 0.0f);
				}
			}
		}
	}
	//AI states
	if (colshape->AIstate != K_AI_STATE_UNDEFINED)
	{
		//stari particulare collision shapes
		switch (colshape->AIstate)
		{
			case K_AI_STATE_COLL_KILL_ACTORS:
			{
				//check only a few times per second
				if (m_Timers.Tick(200))
				{
					bool bKillPlayer = (int)colshape->varAIparams.GetVariantByName(L"b_killPlayer")->m_asINT32;
					bool bKillOthers = (int)colshape->varAIparams.GetVariantByName(L"b_killOthers")->m_asINT32;
					bool bSplat = (int)colshape->varAIparams.GetVariantByName(L"b_splat")->m_asINT32;

					if (bKillPlayer)
					{
						for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
						{
							if ((pPlayerActor[kk] != NULL) && (!pPlayerActor[kk]->bHidden) && (pPlayerActor[kk]->fLife > 0.0f) &&
								(colshape->bbox.PointIn(pPlayerActor[kk]->GetPosHeart())))
							{
								KillActor(pPlayerActor[kk], bSplat);
							}
						}
					}

					if (bKillOthers)
					{
						for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
						{
							//skip actors that are: hidden, dead, players or not a target
							if ((m_arrActors[kk]->bHidden) || (m_arrActors[kk]->fLife <= 0.0f) || (m_arrActors[kk]->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER) ||
								(m_arrActors[kk]->fLife <= 0.0f) ||
								((m_arrActors[kk]->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET) != 0))
								continue;
							if (colshape->bbox.PointIn(m_arrActors[kk]->GetPosHeart()))
							{
								KillActor(m_arrActors[kk], bSplat);
							}
						}
					}
				}
			}
			break;

			case K_AI_STATE_COLL_BREAKABLE_WINDOW:
			{
				if (colshape->AIfvar1 <= 0.0f)
				{
					//seteaza animatia de usa sparta
					if (colshape->pTarget != NULL)
					{
						//trebuie sa pointeze spre un CActive neaparat
						CActive* winact = dynamic_cast<CActive*>(colshape->pTarget);
						if (winact == null)
						{
							ErrorBox(K_ERR_WARNING, L"K_AI_STATE_COLL_BREAKABLE_WINDOW bad cast to CActive");
							break;
						}

						winact->sprite.currentFrame++;
						//reset object script and interact
						winact->script_hash.Reset();

						//generate particles
						CVariantComplex* cvar = colshape->varAIparams.GetVariantByName(L"fForceDirX");
						float dirx = SIGN(cvar->m_asFloat);
						for (int ll = 0; ll < 20; ll++)
						{
							D3DXVECTOR2 ppos = AABB_GetRandomPointInBox(colshape->bbox);
							g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLASS_SHARDS, false, randint(5), &ppos, &g_vecGravity, &D3DXVECTOR2(dirx * (60.0f + randfloat(60.0f)), -40.0f + randfloatsgn(50.0f)), 0.3f + randfloat(0.2f), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
						}
						//sound
						//SND_PLAY_POSITIONAL_RAND2(SNDIDX_WINDOWBREAK1, SNDIDX_WINDOWBREAK2, colshape->bbox.vCenter);
					}
					//destroy collision box
					colshape->bReleaseIt = true;
					colshape->bSetHidden = true;
					//force hidden here to avoid collisions after death
					colshape->bHidden = true;
				}
			}
			break;
			case K_AI_STATE_COLL_BREAKABLE_DOOR:
			{
				float fForceDirX = 0.0f;
				if (colshape->AIvarBool1) //was hit?
				{
					//erase hit flag  (speed optimization)
					colshape->AIvarBool1 = false;
					//set shake timer
					colshape->AItimer1 = 1.0f;
					//just set fForeceDirX to something in order to make it get hit
					CVariantComplex* cvar = colshape->varAIparams.GetVariantByName(L"fForceDirX");
					if (cvar->m_type == CVariantComplex::K_ARGTYPE_FLOAT)
					{
						fForceDirX = cvar->m_asFloat;
						colshape->varAIparams.DeleteVar(L"fForceDirX");
					}
				}
				float dirx = SIGN(fForceDirX);
				//if hit
				if (fForceDirX != 0.0f)
				{
					//generate particles
					for (int ll = 0; ll < 30; ll++)
					{
						D3DXVECTOR2 ppos = AABB_GetRandomPointInBox(colshape->bbox);
						g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_WOODEN_SPLINTERS, false, randint(6), &ppos, &g_vecGravity, &D3DXVECTOR2(dirx * (100.0f + randfloat(60.0f)), -40.0f + randfloatsgn(50.0f)), 0.3f + randfloat(0.2f), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
					}
					//Adauga events de zgomot dincolo de usa
					D3DXVECTOR2 sndpos1 = D3DXVECTOR2(colshape->bbox.vCenter.x + dirx * (colshape->bbox.vHalfSize.x + 2.0f), colshape->bbox.vCenter.y);
					//SND_PLAY_POSITIONAL_RAND2(SNDIDX_DOOR_HIT1, SNDIDX_DOOR_HIT2, sndpos1);
				}

				//if dead
				if (colshape->AIfvar1 <= 0.0f)
				{
					//seteaza animatia de usa sparta
					if (colshape->pTarget != NULL)
					{
						//trebuie sa pointeze spre un CActive neaparat
						CActive* dooract = dynamic_cast<CActive*>(colshape->pTarget);
						if (dooract == null)
						{
							ErrorBox(K_ERR_WARNING, L"K_AI_STATE_COLL_BREAKABLE_DOOR bad cast to CActive");
							break;
						}

						//centram pe bboxul initial
						D3DXVECTOR2 vcenter = dooract->bbox.vCenter;
						dooract->sprite.setAnimation(ANM_ACTIVES_SPR_DOOR_BREAKING);
						dooract->bAnimated = true;
						dooract->pos.x = vcenter.x;
						//reset object script and interact
						dooract->script_hash.Reset();
						dooract->bCanInteract = false;
						dooract->bStandsOut = false;

						//vedem daca flipam animatia in fn de forta aplicata
						if (fForceDirX < 0.0f)
						{
							dooract->flipX = true;

							D3DXVECTOR2 secondExploPos(colshape->bbox.vCenter.x - (colshape->bbox.vHalfSize.x + 1.0f), colshape->bbox.vCenter.y);
							AddProp_Explo(hash_EXPLO_STUN_INVISIBLE, secondExploPos, 0, K_LVL_ACT_CLASS_PLAYER);
						}
						else
						{
							//trebuie setat si pe else pentru ca poate veni deja flipat din editor
							dooract->flipX = false;

							D3DXVECTOR2 secondExploPos(colshape->bbox.vCenter.x + (colshape->bbox.vHalfSize.x + 1.0f), colshape->bbox.vCenter.y);
							AddProp_Explo(hash_EXPLO_STUN_INVISIBLE, secondExploPos, 0, K_LVL_ACT_CLASS_PLAYER);
						}

						g_particlesMgr.GenerateDoorBreak(colshape->bbox.vCenter, D3DXVECTOR2(dirx, 0.0f), K_PART_LAYER_RT_FRONT_NRM);
						//sound
						//SND_PLAY_POSITIONAL(SNDIDX_DOOR_BREAK, colshape->bbox.vCenter);
						//analytics locale
						CVariantComplex* cvexploded = colshape->varAIparams.GetVariantByName(L"bExploded");
						if ((cvexploded->m_type == CVariantComplex::K_ARGTYPE_INT32) && (cvexploded->m_asINT32 != 0))
						{
							App_IncreaseGamestat(K_MEMID_GAMESTATS_DOORS_EXPLODED);
						}
						else
						{
							App_IncreaseGamestat(K_MEMID_GAMESTATS_DOORS_BREACHED);
						}
					}
					//hide or destroy collision box
					colshape->bSetHidden = true;
				}
				else //not dead
				{
					if (colshape->AItimer1 > 0.0f)
					{
						if (m_Timers.Tick(25))
						{
							dec_limit(colshape->AItimer1, 0.05f, 0.0f);
							//trebuie sa pointeze spre un CActive neaparat
							CActive* dooract = dynamic_cast<CActive*>(colshape->pTarget);
							if (dooract == null)
							{
								ErrorBox(K_ERR_WARNING, L"K_AI_STATE_COLL_BREAKABLE_DOOR bad cast to CActive (doorshake)");
								break;
							}
							//shake door
							dooract->pos.x = dooract->pos_ini.x + (2.0f * colshape->AItimer1) * sin(colshape->AItimer1 * 40.0f);
						}
					}
				}
			}
			break;
			case K_AI_STATE_COLL_FOG_OF_WAR:
			{
				//verifica daca playerul o vede (de cateva ori pe secunda)
				if (g_timers.Tick(100))
				{
					for (int npl = 0; npl < K_MAX_PLAYERS_CNT; npl++)
					{
						if (pPlayerActor[npl] == null)
							continue;

						CActor* pPlayer = pPlayerActor[npl];
						CAABB aabbPlayerView(D3DXVECTOR2(pPlayer->posHeart.x - pPlayer->templateActor.distSee, pPlayer->posHeart.y - pPlayer->templateActor.distSee),
							D3DXVECTOR2(pPlayer->posHeart.x + pPlayer->templateActor.distSee, pPlayer->posHeart.y + pPlayer->templateActor.distSee));

						//didn't decide to remove FOW:
						if (colshape->AIfvar1 >= 1.0f)
						{
							if (!aabbPlayerView.Intersects(&colshape->bbox))
								continue;
							//vad daca e fereastra verticala sau orizontala
							bool bCheckH = true;
							//daca plaeyrul este deasupra sau sub 
							if ((pPlayer->posHeart.x >= colshape->bbox.vMin.x) && (pPlayer->posHeart.x <= colshape->bbox.vMax.x))
							{
								bCheckH = false;
							}

							D3DXVECTOR2 vfrom = pPlayer->posHeart;
							D3DXVECTOR2 vto = vfrom;
							if (bCheckH)
							{
								vto.x += SIGN(colshape->bbox.vCenter.x - pPlayer->posHeart.x) * pPlayer->templateActor.distSee;
							}
							else
							{
								//search on diagonal
								vto.x += SIGN(colshape->bbox.vCenter.x - pPlayer->posHeart.x) * pPlayer->templateActor.distSee;
								vto.y += SIGN(colshape->bbox.vCenter.y - pPlayer->posHeart.y) * pPlayer->templateActor.distSee;
							}

							//do we have FOW collision?
							D3DXVECTOR2 fowColPt;
							float fowT = -1.0f;
							if (AABB_Segment_IntersectionEx(vfrom, vto, colshape->bbox, &fowColPt, fowT))
							{
								bool bHasCollidedBefore = false;
								for (int kk = 0; kk < m_visibleList.logic_colShapesExtended.Count(); kk++)
								{
									CCollisionShape *shape = m_visibleList.logic_colShapesExtended.m_pData[kk];
									if ((shape->type != K_LVL_COLL_TYPE_SOLID) || (!shape->castShadows))
										continue;
									float fshapeT = 100.0f;
									if (AABB_Segment_IntersectionEx(vfrom, vto, shape->bbox, NULL, fshapeT))
									{
										if ((fowT >= 0.0f) && (fowT <= 1.0f) && (fshapeT < fowT))
										{
											bHasCollidedBefore = true;
											break;
										}
									}
								}
								//intersection before?
								if (!bHasCollidedBefore)
								{
									colshape->AIfvar1 = 1.0f - EPS;
								}
							}
						}

					}
				}

				if (colshape->AIfvar1 < 1.0f)
					colshape->AIfvar1 -= dTime * 10.0f;
				if (colshape->AIfvar1 <= 0.0f)
				{
					colshape->AIfvar1 = 0.0f;
					colshape->bSetHidden = true;
					colshape->bReleaseIt = true;
				}
				//set color
				colshape->color = D3DCOLOR_COLORALPHA(K_LVL_COLL_FOW_COLOR, colshape->AIfvar1);
			}
			break;
			case K_AI_STATE_PARTICLES_GENERATOR:
			{
				//#TODO: aici pune pe pauza emitoarele sau seteaza sa genereze doar in zona vizibila?...
				/*
				static const UINT32 hash_v_emitterPtr = FastHash(L"emitterPtr");
				//get params
				CParticleEmitter* pe = null;
				//continuam cu procesarea
				CVariantComplex* emittervc = colshape->varAIparams.GetVariantByNameHash(hash_v_emitterPtr);
				if (emittervc->m_type == CVariantComplex::K_ARGTYPE_NONE)
				{
					ErrorBox(K_ERR_WARNING, L"ParticleEmitter pointer not found!");
					break;
				}
				//get rail pointer
				pe = static_cast<CParticleEmitter*>(emittervc->m_asVoid);
				//ii da pause cand iese din ecran
				CAABB camAABB;
				camAABB.Set(m_camLevel.GetCamWorldAABB());

				if (camAABB.Intersects(&colshape->bbox))
				{
					pe->bPauseUpdate = false;
				}
				else
				{
					pe->bPauseUpdate = true;
				}
				*/
			}
			break;
			default:
			{
				if (!UpdateAI_base(colshape, dTime, colshape->fTimelineAI))
				{
					ErrorBox(K_ERR_WARNING, L"CLevel::UpdateAI_collshape- AIstate not handled: %d", colshape->AIstate);
				}
			}
			break;
		}
	}
}

void CLevel::UpdateAI_light(CLight* light, float dTime)
{
	//touch timer reset (nu e necesar pe lights)
	//light->UpdateTouchTimerReset(dTime);

	//daca am schimbat vizibilitatea
	light->bHidden = light->bSetHidden;
	//daca este hidden nu mai verifica AI
	if (light->bHidden)
		return;

	//update timeline
	light->fTimelineAI += dTime;

	//daca nu a fost tratata starea curenta inseamna ca este particulara pt clasa asta
	if (light->AIstate != K_AI_STATE_UNDEFINED)
	{
		//stari particulare lumini (se pot suprascrie cele default)
		switch (light->AIstate)
		{
			case K_AI_STATE_FN_LIGHT_FLICKER1:
			{
				//params: f_timeMul, f_threshold
				float timeMul = light->varAIparams.GetVariantByName(L"f_timeMul")->m_asFloat;
				float fThreshold = light->varAIparams.GetVariantByName(L"f_threshold")->asFloat();
				float falpha = PerlinNoise1D(light->fTimelineAI * timeMul, 2.0f, 3.0f, 0.8f, 0.25f, 2);
				if (falpha > fThreshold)
					falpha = 1.0f;
				else
					falpha = falpha / fThreshold;
				//falpha = (falpha < fThreshold) ? 0.0f : 1.0f;
				light->color = D3DCOLOR_COLORALPHA(light->color_ini, falpha);
			}
			break;
			case K_AI_STATE_FN_LIGHT_ANG_CONE_XZ_TIME:
			{
				//fvar1 - height, fvar2 - radius, timer1 - timeMul, timer2 - timeAdd
				D3DXVECTOR3 conepoint(0.0f, -light->AIfvar1, 0.0f);
				D3DXVECTOR3 ppos = D3DXVECTOR3(light->AIfvar2 * sin((light->fTimelineAI + light->AItimer2) * light->AItimer1), 0.0f, light->AIfvar2 * cos((light->fTimelineAI + light->AItimer2) * light->AItimer1));
				D3DXVec3Normalize(&light->vnDirection, &(ppos - conepoint));
			}
			break;
			default:
			{
				if (!UpdateAI_base(light, dTime, light->fTimelineAI))
				{
					ErrorBox(K_ERR_WARNING, L"CLevel::UpdateAI_light - AIstate not handled: %d", light->AIstate);
				}
			}
			break;
		}
	}

	//update-uri finale
	light->SetPos(light->pos);
}

void CLevel::UpdateAI_active(CActive* active, float dTime)
{
	//touch timer reset
	//daca trebuie actionat de toata echipa verific aici (doar pentru active pentru ca nu voi actiona pe actori sau collisions)
	bool bResetTouchTimer = false;
	if (active->fTouchDuration < 0.0f)
	{
		for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
		{
			if (pPlayerActor[kk] != null)
			{
				//sa fiu sigur ca interactioneaza pe acelasi obiect
				if ((pPlayerActor[kk]->nInteractingState == 0) || (pPlayerActor[kk]->pClosestTouchable != active))
				{
					active->fTouchTimer = 0.0f;
					break;
				}
			}
		}
	}

	active->UpdateTouchTimerReset(dTime);
	//daca am schimbat vizibilitatea
	active->bHidden = active->bSetHidden;
	//daca este hidden nu mai verifica AI
	if (active->bHidden)
		return;

	//update timeline
	active->fTimelineAI += dTime;

	//update sprite if animated
	if (active->bAnimated)
	{
		UINT32 aframeFlag = active->sprite.Update(&m_sprActives, dTime);
		//cand ajunge la capatul animatiei scoate flagul de animated
		if (active->sprite.animStatus == ANIM_STATUS_FRAMELOCK)
			active->bAnimated = false;
		//la obiectele animate luam bbox-ul la fiecare frame
		if ((active->sprite.animStatus == ANIM_STATUS_PLAYING_FRAME_ADVANCED) || (active->sprite.animStatus == ANIM_STATUS_FRAMELOCK))
		{
			RECTXYWH frrect = m_sprActives.GetAFrameBBox(active->sprite.animationIdx, active->sprite.currentFrame);
			active->bbox_ini.Set(frrect);
			//nu pastreaza acelasi bbox la flip deci flipam bboxul
			if (active->flipX)
			{
				active->bbox_ini.Flip(true, false);
			}
		}
	}

	//daca nu a fost tratata starea curenta inseamna ca este particulara pt clasa asta
	if (active->AIstate != K_AI_STATE_UNDEFINED)
	{
		//stari particulare obiectelor (se pot suprascrie cele default)
		switch (active->AIstate)
		{
			case K_AI_STATE_ACTIVE_ZOMBIE_SPAWNER:
			{
				switch (active->AIsubState)
				{
					case 0: //not enabled yet
					{
						//make sure we don't animate
						active->bAnimated = false;

						for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
						{
							CActor* pPlayer = pPlayerActor[kk];
							if (pPlayer == null)
								continue;
							float fDist = D3DXVec2Length(&(pPlayer->GetPosHeart() - active->pos));
							const float fActivationDistance = 150.0f;
							if (fDist <= fActivationDistance)
							{
								active->AIsubState = 1;
								active->sprite.setAnimation(ANM_ACTIVES_SPR_ZOMBIE_SPAWNER_APPEAR);
								active->bAnimated = true;

								m_camLevel.ShakeScreen(2.0f, 4.0f, &active->pos);
								//SND_PLAY_POSITIONAL(SNDIDX_STONE_MOVE1, active->pos);
								break;
							}
						}
					}
					break;
					case 1: //appearing
					{
						//wait for it to finish the animation
						if (active->sprite.animStatus == ANIM_STATUS_FRAMELOCK)
						{
							//change to spawning
							active->AIsubState = 2;
							//spawn quickly after activation
							active->AItimer1 = active->AItimer2 - 1.0f - m_rand.RandFloat(1.0f);
							//change on looping animation
							active->sprite.setAnimation(ANM_ACTIVES_SPR_ZOMBIE_SPAWNER_ACTIVE);
							active->bAnimated = true;
							//enable target light
							if (active->pTarget != null)
							{
								active->pTarget->bSetHidden = false;
							}
						}
					}
					break;
					case 2: //active
					{
						//check spawn count
						if (active->AIvar1 < active->AIvar2)
						{
							active->AItimer1 += dTime;
							if (active->AItimer1 >= active->AItimer2)
							{
								active->AIvar1++;
								active->AItimer1 = 0.0f;
								//spawn zombie
								CStringHash shTemplate;
								//#ZOMBIE: generate zombies
								int prob = m_rand.RandInt(100);
								if (prob < 12)
									shTemplate.Init(L"ACTOR_ZOMBIE_TELEPORT1"); //12%
								else if (prob < 24)
									shTemplate.Init(L"ACTOR_ZOMBIE_RANGED1"); //12%
								else if (prob < 40)
									shTemplate.Init(L"ACTOR_ZOMBIE_EXPLODING"); //16%
								else if (prob < 66)
									shTemplate.Init(L"ACTOR_ZOMBIE_FAST1"); //26%
								else
									shTemplate.Init(L"ACTOR_ZOMBIE_SLOW1"); //34%
								//spawn
								CStringHash shState(L"AWARE");
								SpawnActor(active->pos, shTemplate.text, 0, &shState);

								g_particlesMgr.GenerateZombieSpawn(active->pos, 0x8800ff00, K_PART_LAYER_RT_FRONT_NRM);
							}
						}
					}
					break;
				}
			}
			break;

			case K_AI_STATE_ACTIVE_BOMB:
			{
				//daca nu esti pe playing nu mai scade counterul la bomba
				if (m_levelState != K_LVL_STATE_PLAYING)
					break;

				float fOldTimer = active->AItimer1;
				active->AItimer1 -= dTime;
				m_interfaceIGM.SetBombTimer(active->AItimer1);

				//--- sounds ---
				if (active->AItimer1 > 15.0f)
				{
					if (floor(fOldTimer) > floor(active->AItimer1))
					{
						//SND_PLAY(SNDIDX_BOMBBEEP);
					}
				}
				else
				{
					if (m_Timers.Tick(250))
					{
						//SND_PLAY(SNDIDX_BOMBBEEP);
					}
				}

				if (active->AItimer1 <= 0.0f)
				{
					m_interfaceIGM.SetBombTimer(0.0f);
					//add some explosions so everybody will die
					AddProp_Explo(hash_EXPLO_LARGE_XL, active->pos, active->UID, K_LVL_ACT_CLASS_EXPLOSION);
					AddProp_Explo(hash_EXPLO_LARGE_XL, active->pos + D3DXVECTOR2(32.0f, 0.0f), active->UID, K_LVL_ACT_CLASS_EXPLOSION);
					AddProp_Explo(hash_EXPLO_LARGE_XL, active->pos - D3DXVECTOR2(32.0f, 0.0f), active->UID, K_LVL_ACT_CLASS_EXPLOSION);

					g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_ROUND_XL, true, 0, &D3DXVECTOR2(active->pos.x, active->pos.y - 15.0f), NULL, NULL, 1.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);

					active->sprite.setAnimation("BOMB_EXPLODED", &m_sprActives);

					SetLevelState(K_LVL_STATE_MISSION_FAILED, STR_BOMB_EXPLODED);
				}
			}
			break;
			case K_AI_STATE_ACTIVE_AMMO_BOX:
			{
				int nAmmoLeft = active->varAIparams.GetVariantByName(L"n_ammoLeft")->m_asINT32;
				active->sprite.currentFrame = nAmmoLeft;

				//fade out
				if (nAmmoLeft <= 0)
				{
					active->AItimer1 -= dTime;
					if (active->AItimer1 <= 0.0f)
					{
						active->bReleaseIt = true;
					}
					//color
					float fAlpha = LIMIT(active->AItimer1, 0.0f, 1.0f);
					active->color = D3DCOLOR_COLORALPHA(active->color_ini, fAlpha);
				}
			}
			break;
			case K_AI_STATE_ACTIVE_HEALTH_BOX:
			{
				int nHealthLeft = active->varAIparams.GetVariantByName(L"n_healthLeft")->m_asINT32;
				active->sprite.currentFrame = nHealthLeft;

				//fade out
				if (nHealthLeft <= 0)
				{
					active->AItimer1 -= dTime;
					if (active->AItimer1 <= 0.0f)
					{
						active->bReleaseIt = true;
					}
					//color
					float fAlpha = LIMIT(active->AItimer1, 0.0f, 1.0f);
					active->color = D3DCOLOR_COLORALPHA(active->color_ini, fAlpha);
				}
			}
			break;
			case K_AI_STATE_ACTIVE_TEAM_TELEPORTER_2FRAMES:
			{
				//keep door open (AIvar1 contine frame-ul default) - set frame
				active->sprite.currentFrame = active->nFrame_ini;
				if (active->AItimer1 > 0.0f)
				{
					active->AItimer1 -= dTime;
					bool bDontChangeFrames = (bool)(active->varAIparams.GetVariantByName(L"b_DontChangeFrames")->m_asINT32);
					if (!bDontChangeFrames)
					{
						active->sprite.currentFrame++;
					}
					if (active->AItimer1 < 0.0f)
						active->AItimer1 = 0.0f;
				}

				//daca primesc parametru de toucher inseamna ca a fost activata usa si o tin deschisa pana cand actorul activator intra in behavior de TEAM_TELEPORT
				UINT32 nCurrentToucher = active->varAIparams.GetVariantByName(L"nToucherUID")->m_asUINT32;
				if (nCurrentToucher != 0)
				{
					//Teleport logic (merge doar pentru actori)
					CActor* toucher = GetActorByUID(nCurrentToucher);
					//verifica daca e deja un player intr-un teleporter si daca este nu iti da voie sa intri in altul
					if ((m_pTeleportSource != null) && (m_pTeleportSource != active))
					{
						active->varAIparams.SetNamedVarUINT32(L"nToucherUID", 0);
						SND_PLAY_POSITIONAL(SNDIDX_DENIED, active->pos);
						break;
					}

					//set toucher teleport state - enter once
					if ((toucher != null) && (toucher->m_pAIcurrentState != null) && (!toucher->m_pAIcurrentState->name.IsEqual(L"TEAM_TELEPORT")) && (m_fTeleportTimer <= 0.0f))
					{
						//setam starea TEAM_TELEPORT pentru actorul toucher
						SetActorAIState(toucher, L"TEAM_TELEPORT");
						//tinem usa deschisa o perioada	daca nu e setat flagul de don't change frames
						bool bDontChangeFrames = (bool)(active->varAIparams.GetVariantByName(L"b_DontChangeFrames")->m_asINT32);
						if(!bDontChangeFrames)
							active->AItimer1 = 1.0f;
						//centram player
						toucher->pos = active->pos;
						toucher->speed.x = 0.0f;
						//setam si pointerul la teleporter
						if (m_pTeleportSource == null)
						{
							m_pTeleportSource = active;
						}
					}
					//reset touch command
					active->varAIparams.SetNamedVarUINT32(L"nToucherUID", 0);
				}

				//#PERSONALIZARE: player in limbo? keep door open
				if ((m_nTeleportSlots > 0) && (m_pTeleportSource == active) && (m_fTeleportTimer <= 0.0f))
				{
					bool bDontChangeFrames = (bool)(active->varAIparams.GetVariantByName(L"b_DontChangeFrames")->m_asINT32);
					if (!bDontChangeFrames)
						active->AItimer1 = 1.0f;
				}

				//daca am activat teleportul face teleport
				if ((/*(m_bTeleportRequested) || */(m_nTeleportSlots == m_nPlayersActive)) && (m_pTeleportSource == active))
				{
					bool bCanTeleport = true;
					//verificare finala daca pot face teleport request de unul dintre players
					/*
					if (m_bTeleportRequested)
					{
						for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
						{
							if (pPlayerActor[kk] != null)
							{
								//daca unul dintre ei este in TEAM TELEPORT dar nu e inca pe behavior TEAM_TELEPORT atunci da cancel la request
								//fara verificarea asta aparea un bug atunci cand unul apasa sus in timp ce celalalt intra pe usa
								if ((pPlayerActor[kk]->m_pAIcurrentState->name.IsEqual(L"TEAM_TELEPORT")) && (pPlayerActor[kk]->GetCurrentBehavior() != AI_BEHAVIOR_PLAYER_TEAM_TELEPORT))
								{
									m_bTeleportRequested = false;
									bCanTeleport = false;
								}
							}
						}
					}
					*/
					//teleport players
					if ((active->pTarget != null) && (bCanTeleport))
					{
						bool bTeleported[K_MAX_PLAYERS_CNT] = { false, false };

						if (m_fTeleportTimer <= 0.0f)
						{
							for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
							{
								if (pPlayerActor[kk] != null)
								{
									if (pPlayerActor[kk]->GetCurrentBehavior() == AI_BEHAVIOR_PLAYER_TEAM_TELEPORT)
									{
										pPlayerActor[kk]->pos = active->pTarget->pos;
										bTeleported[kk] = true;
										//close source door after teleport (ca sa nu se vada deschis liftul pe 2 paliere)
										active->AItimer1 = 0.0f;
										//set duration timer
										m_fTeleportTimer = EPS + active->varAIparams.GetVariantByName(L"f_teleportDuration")->m_asFloat;
									}
									else																  
									{
										//only on same PC multiplayer
										if (!UTGetAppClass().IsGameNetworked())
										{
											//if we have player control (state DEFAULT) put him into LIMBO 
											if (pPlayerActor[kk]->m_pAIcurrentState->name.IsEqual(L"DEFAULT"))
												SetActorAIState(pPlayerActor[kk], L"IN_LIMBO");
										}
									}
								}
							}
						}

						//open destination door (SAME AI)
						if (m_fTeleportTimer > 0.0f)
						{
							m_fTeleportTimer -= dTime;
							if (m_fTeleportTimer <= 0.0f)
							{
								m_fTeleportTimer = 0.0f;

								//tinem usa destinatie deschisa o perioada daca nu e setat flagul de don't change frames
								bool bDontChangeFrames = (bool)(active->pTarget->varAIparams.GetVariantByName(L"b_DontChangeFrames")->m_asINT32);
								if (!bDontChangeFrames)
								{
									active->pTarget->AItimer1 = 1.0f;
								}

								m_bTeleportActivated = true;

								//door takes to hidden room
								bool bHiddenRoom = (bool)(active->varAIparams.GetVariantByName(L"b_EnterHiddenRoom")->m_asINT32);
								//black out screen for a bit
								//when on multiplayer, darken only if teleported to hidden room
								if (bHiddenRoom)
								{
									if ((!UTGetAppClass().IsGameNetworked()) || (bTeleported[g_netlock.Net_GetPlayerIndex()]))
									{
										m_screenVignette.Init(0.5f, 0xff000000, 0.0f, 0.5f);
									}
								}
								//daca avem slow time facem acum
								float fSlowTimeDuration = active->varAIparams.GetVariantByName(L"f_SlowTimeDuration")->m_asFloat;
								if (fSlowTimeDuration > 0.0f)
								{
									if (!UTGetAppClass().IsGameNetworked())
									{
										SetTimeMultiplier(0.5f, fSlowTimeDuration);
									}
									else //on networked games only slow down time if both players enter
									{
										if ((bTeleported[0] == true) && (bTeleported[1] == true))
											SetTimeMultiplier(0.5f, fSlowTimeDuration);
									}
									//ca sa faca doar prima data slowdown stergem variabila
									active->varAIparams.SetNamedVarFloat(L"f_SlowTimeDuration", 0.0f);
								}
								//set hidden room flag
								m_bInsideHiddenRoom = bHiddenRoom;

								if (bTeleported[0])
									m_bPlayerInHiddenRoom[0] = bHiddenRoom;
								if (bTeleported[1])
									m_bPlayerInHiddenRoom[1] = bHiddenRoom;
								//get in hidden room?
								if (m_bInsideHiddenRoom)
								{
									//m_bPaintBackground = false;
								}
								else //get out of hidden room
								{
									SetTimeMultiplier(1.0f, 0.0f);
									m_HiddenRoomAABB.Set(0.0f, 0.0f, 0.0f, 0.0f);
									//reset camera target
									m_camTargetActive = m_camTargetOld;
									if (m_camTargetActive == null)
										m_camLevel.SetCamPos(&m_vCamPosDefault, 1.0f, true);
									else
										m_camLevel.SetCamPos(&m_camTargetActive->pos, 1.0f, true);
									//start painting the background
									//m_bPaintBackground = true;
									
									//black out screen
									RECTXYWH_F camrect = m_camLevel.GetCamWorldAABB();
									CAABB camAABB(D3DXVECTOR2(camrect.x, camrect.y), D3DXVECTOR2(camrect.Right(), camrect.Bottom()));
									//black out screen only if teleporting outside the screen
									if(!camAABB.Intersects(&active->pTarget->bbox_exported))
										m_screenVignette.Init(0.5f, 0xff000000, 0.0f, 0.5f);
								}
							}
						}
					}
				}

				//open/close sounds
				if ((active->AIvarBool1 == false) && (active->AItimer1 > 0.0f))
				{
					//just opened
					CVariantComplex* cvc = active->varAIparams.GetVariantByName(L"s_openSnd");
					if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
					{
						int sndidx = UTGetSoundManager().getSndIdx(cvc->m_strArg.textHash);
						SND_PLAY_POSITIONAL(sndidx, active->pos);
					}
					//on open script
					cvc = active->varAIparams.GetVariantByName(L"s_ScriptOnOpen");
					if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
					{
						UTGetScriptManager().StartScript(cvc->m_strArg.textHash, active->UID);
					}

					active->AIvarBool1 = true;
				}
				else if ((active->AIvarBool1 == true) && (active->AItimer1 <= 0.0f))
				{
					//just closed
					CVariantComplex* cvc = active->varAIparams.GetVariantByName(L"s_closeSnd");
					if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
					{
						int sndidx = UTGetSoundManager().getSndIdx(cvc->m_strArg.textHash);
						SND_PLAY_POSITIONAL(sndidx, active->pos);
					}
					//on close script
					cvc = active->varAIparams.GetVariantByName(L"s_ScriptOnClose");
					if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
					{
						UTGetScriptManager().StartScript(cvc->m_strArg.textHash, active->UID);
					}
					//save state
					active->AIvarBool1 = false;
				}
			}
			break;

			case K_AI_STATE_ACTIVE_DOOR_SECTION:
			{
				active->AItimer1 = 0.0f;
			}
			break;

			case K_AI_STATE_ACTIVE_DOORFACE_AUTOCLOSE:
			{
				//keep door open (AIvar1 contine frame-ul default) - set frame
				active->sprite.currentFrame = active->nFrame_ini;
				if (active->AItimer1 > 0.0f)
				{
					active->AItimer1 -= dTime;
					
					bool bDontChangeFrames = (bool)(active->varAIparams.GetVariantByName(L"b_DontChangeFrames")->m_asINT32);
					if (!bDontChangeFrames)
					{
						active->sprite.currentFrame++;
					}

					if (active->AItimer1 < 0.0f)
						active->AItimer1 = 0.0f;
				}

				//open/close sounds
				if ((active->AIvarBool1 == false) && (active->AItimer1 > 0.0f))
				{
					//just opened
					CVariantComplex* cvc = active->varAIparams.GetVariantByName(L"s_openSnd");
					if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
					{
						int sndidx = UTGetSoundManager().getSndIdx(cvc->m_strArg.textHash);
						SND_PLAY_POSITIONAL(sndidx, active->pos);
					}
					//on open script
					cvc = active->varAIparams.GetVariantByName(L"s_ScriptOnOpen");
					if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
					{
						UTGetScriptManager().StartScript(cvc->m_strArg.textHash, active->UID);
					}

					active->AIvarBool1 = true;
				}
				else if ((active->AIvarBool1 == true) && (active->AItimer1 <= 0.0f))
				{
					//just closed
					CVariantComplex* cvc = active->varAIparams.GetVariantByName(L"s_closeSnd");
					if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
					{
						int sndidx = UTGetSoundManager().getSndIdx(cvc->m_strArg.textHash);
						SND_PLAY_POSITIONAL(sndidx, active->pos);
					}
					//on close script
					cvc = active->varAIparams.GetVariantByName(L"s_ScriptOnClose");
					if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
					{
						UTGetScriptManager().StartScript(cvc->m_strArg.textHash, active->UID);
					}
					active->AIvarBool1 = false;
				}

			}
			break;

			case K_AI_STATE_ACTIVE_SWINGING_FRONTOBJ:
			{
				//implementare balans
				float fAng = active->fAngle;
				float angDelta = active->fAngle - active->fAngle_ini;
				
				float fFriction = 0.4f;
				//ca sa se miste incet scot frecarea la viteze mici
				if (fabs(active->AIfvar1) <= 0.04f)
					fFriction = 0.0f;
				active->AIfvar1 -= angDelta * dTime * 20.0f + active->AIfvar1 * dTime * fFriction;
				fAng += active->AIfvar1 * dTime;
				CLAMP(fAng, active->fAngle_ini - 1.4f, active->fAngle_ini + 1.4f);
				
				active->SetAngle(fAng);
			}
			break;

			case K_AI_STATE_ACTIVE_EXPLO_TRAP:
			{
				for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
				{
					if (pPlayerActor[kk] == null)
						continue;
					if (pPlayerActor[kk]->bbox.Intersects(&active->bbox))
					{
						//generate explo
						AddProp_Explo(hash_EXPLO_GRENADE_GROUND, active->pos, active->GetUID(), K_LVL_ACT_CLASS_EXPLOSION);
						//decal explo mark
						AddDecal(K_LVL_DECAL_LAYER_BACKWALLS, active->pos, ANM_ACTIVES_SPR_DECAL_EXPLOMARKS, randint(3), 0xffffffff);

						active->bReleaseIt = true;
						break;
					}
				}
			}
			break;
			case K_AI_STATE_ACTIVE_CHECKPOINT:
			{
				for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
				{
					if (pPlayerActor[kk] == null)
						continue;
					if (pPlayerActor[kk]->bbox.Intersects(&active->bbox))
					{
						active->Touch(pPlayerActor[kk]->GetUID(), dTime);
						//save checkpoint
						vLastSpawnPoint = active->pos;
						break;
					}
				}
			}
			break;
			default:
			{
				if (!UpdateAI_base(active, dTime, active->fTimelineAI))
				{
					ErrorBox(K_ERR_WARNING, L"CLevel::UpdateAI_active - AIstate not handled: %d", active->AIstate);
				}
			}
			break;
		}
	}

	active->SetPos(active->pos);
	//update-uri finale
	active->sprite.pos = active->pos;
	active->sprite.color = active->color;
}


void CLevel::SetActorAIState(CActor * actor, CAIState* pNewState)
{
	//daca e aceeasi stare sau null nu o mai setez
	if ((actor->m_pAIcurrentState == pNewState) || (pNewState == null) || (actor == null))
 		return;

	///1. clean exit old state:
	OnActorBehaviorFinished(actor, actor->GetCurrentBehavior());
	///2. sets the new behavior
	actor->m_AIsensorInfo.m_bEnabled = true; //enable sensors on new state
	actor->m_pAIcurrentState = pNewState;
	int newBehaviorIdx = -1; //defaults on no behavior
							 //daca am stare not null si are behaviors il setez pe primul
	if ((actor->m_pAIcurrentState != null) && (actor->m_pAIcurrentState->m_arrBehaviors.nCount > 0))
		newBehaviorIdx = 0;

	bool bShortBehavior = false;
	do 
	{
		SetActorAIBehaviorIdx(actor, newBehaviorIdx, bShortBehavior);
		if (bShortBehavior)
		{
			OnActorBehaviorFinished(actor, actor->GetCurrentBehavior());
			newBehaviorIdx++;
		}
	} while (bShortBehavior);

}

bool CLevel::SetActorAIState(CActor * actor, WCHAR * strStateName)
{
	CAIState* newstate = actor->templateActor.AItemplate->GetAIStateByName(strStateName);
	if (newstate == null)
	{
		LOG(L"CLevel::SetActorAIState - state not found! %s\n", strStateName);
		return false;
	}
	//everything ok, set state
	SetActorAIState(actor, newstate);
	return true;
}

void CLevel::SetActorWeaponPerks(CActor * pActor, CWeapon * pWeapon)
{
	assert((pWeapon != null) && (pActor != null));

	//reset actor template to initial one
	pActor->templateActor = pActor->templateActor_ini;

	if (!pWeapon->WeaponTemplate.shTemplateOverwrite.IsEmpty())
	{
		CActorTemplate* updateTemplate = GetTemplateActor(pWeapon->WeaponTemplate.shTemplateOverwrite.textHash);
		if (updateTemplate == null)
		{
			ErrorBox(K_ERR_WARNING, L"SetActorCurrentWeapon failed! Template %s not found for weapon %s!", pWeapon->WeaponTemplate.shTemplateOverwrite.text, pWeapon->WeaponTemplate.name.text);
		}
		//set animations from new template
		pActor->templateActor.AddGenericDataFromTemplate(updateTemplate);
		pActor->templateActor.OverwriteAnimsFromTemplate(updateTemplate);

		//reset animations (make sure they get set)
		pActor->eLastAnimSet = K_LVL_ACT_ANIM_EMPTY;
		pActor->eLastAnimSet_feet = K_LVL_ACT_ANIM_EMPTY;
	}
	//set the heart and gun vectors again
	LoadActorBBoxAndPoints(pActor, K_LVL_ACT_ANIM_REF_POSE, 0);

	///--- PERKS ---
	//apply perks that change current weapon
	if ((pActor->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER) && (pActor->nPlayerOrdinal >= 0))
	{
		switch (g_playerSelScr.m_arrPlayers[pActor->nPlayerOrdinal].eType)
		{
			case K_PSS_CLASS_ASSAULTER:
			{
				if (pWeapon->WeaponTemplate.bulletTemplate.nGroup == K_LVL_BULLGROUP_BULLETS)
				{
					float fAccuracy = g_playerSelScr.GetUpgradeBarPercent(&g_playerSelScr.m_arrPlayers[pActor->nPlayerOrdinal], L"A1_ACCURACY");
					pWeapon->WeaponTemplate.fSpreadFOV *= 1.0f - fAccuracy * 0.3f;
				}
				//#PERK: QUICK AIM - faster aiming for aimed shot
				if (g_playerSelScr.IsPerkEnabled(pActor->nPlayerOrdinal, &shPerk_QUICK_AIM))
				{
					CActorTemplate* updateTemplate = GetTemplateActor(L"UPGRADE_ASSAULTER_AIMED_SHORTER");
					//set animations from new template
					pActor->templateActor.AddGenericDataFromTemplate(updateTemplate);
					pActor->templateActor.OverwriteAnimsFromTemplate(updateTemplate);
				}
			}
			break;
			case K_PSS_CLASS_RECON:
			{
				if (pWeapon->WeaponTemplate.bulletTemplate.nGroup == K_LVL_BULLGROUP_BULLETS)
				{
					float fAccuracy = g_playerSelScr.GetUpgradeBarPercent(&g_playerSelScr.m_arrPlayers[pActor->nPlayerOrdinal], L"R1_GUNPLAY");
					pWeapon->WeaponTemplate.fSpreadFOV *= 1.0f - fAccuracy * 0.3f;
				}
			}
			break;
			case K_PSS_CLASS_FBI_AGENT:
			{
				if (pWeapon->WeaponTemplate.bulletTemplate.nGroup == K_LVL_BULLGROUP_BULLETS)
				{
					float fAccuracy = g_playerSelScr.GetUpgradeBarPercent(&g_playerSelScr.m_arrPlayers[pActor->nPlayerOrdinal], L"F1_HANDGUN");
					pWeapon->WeaponTemplate.fSpreadFOV *= 1.0f - fAccuracy * 0.3f;
				}
			}
			break;
			case K_PSS_CLASS_SHIELD:
			{
				if (pWeapon->WeaponTemplate.bulletTemplate.nGroup == K_LVL_BULLGROUP_BULLETS)
				{
					float fAccuracy = g_playerSelScr.GetUpgradeBarPercent(&g_playerSelScr.m_arrPlayers[pActor->nPlayerOrdinal], L"S1_HANDGUN");
					pWeapon->WeaponTemplate.fSpreadFOV *= 1.0f - fAccuracy * 0.3f;
				}
				//#PERK: DEVASTATOR - better smg ability 
				if (pWeapon->WeaponTemplate.name.IsEqual(L"WPN_SA_UZZI"))
				{
					if (g_playerSelScr.IsPerkEnabled(pActor->nPlayerOrdinal, &shPerk_DEVASTATOR))
					{
						pWeapon->WeaponTemplate.bulletTemplate.fCriticalHitChance += 0.2f;
					}
				}
			}
			break;
			case K_PSS_CLASS_BREACHER:
			{
			}
			break;
			case K_PSS_CLASS_OFFDUTYGUY:
			{
				if (pWeapon->WeaponTemplate.bulletTemplate.nGroup == K_LVL_BULLGROUP_BULLETS)
				{
					float fAccuracy = g_playerSelScr.GetUpgradeBarPercent(&g_playerSelScr.m_arrPlayers[pActor->nPlayerOrdinal], L"O1_SHOOTING");
					pWeapon->WeaponTemplate.fSpreadFOV *= 1.0f - fAccuracy * 0.3f;
				}
				//#PERK: AP_AMMO GARAND
				if (pWeapon->WeaponTemplate.name.IsEqual(L"WPN_ULTIMATE_GARAND"))
				{
					if (g_playerSelScr.IsPerkEnabled(pActor->nPlayerOrdinal, &shPerk_GARAND_AP_AMMO))
					{
						pWeapon->WeaponTemplate.bulletTemplate.nArmorPiercingRating = pWeapon->WeaponTemplate.bulletTemplate.nArmorPiercingRating + 1;
					}
				}
			}
			break;
		}
	}
}


bool CLevel::SetActorAIBehaviorIdx(CActor * actor, int nBehaviorIdx, bool &ret_bFinished)
{
	//by default all states need update
	ret_bFinished = false;
	//daca starea e null sau index negativ, sau daca starea nu are behaviors
	if ((nBehaviorIdx < 0) || (actor->m_pAIcurrentState == null) || (actor->m_pAIcurrentState->m_arrBehaviors.nCount <= 0))
	{
		actor->m_nAIcurrentBehaviorIdx = -1;
		ret_bFinished = true;
		return false;
	}

	//daca e valida
	actor->m_nAIcurrentBehaviorIdx = nBehaviorIdx % actor->m_pAIcurrentState->m_arrBehaviors.nCount;
	//setari initiale behavior
	CAIBehavior* pNewBehavior = &actor->m_pAIcurrentState->m_arrBehaviors[actor->m_nAIcurrentBehaviorIdx];
	//reset behavior timer
	actor->m_fAIbehaviorTimer = 0.0f;
	
	actor->m_AIcommands.Reset();
	actor->fFOVPercent = actor->templateActor.fFOVpercent;

	switch (pNewBehavior->nType)
	{
		case AI_BEHAVIOR_EMPTY:
		{
			//state doesn't need update
			ret_bFinished = true;
		}
		break;
		case AI_BEHAVIOR_SHOW_ENEMY:
		{	
			bool bHasFriends = false;
			for (int kk = 0; kk < m_visibleList.logic_actors_closeby.Count(); kk++)
			{
				CActor* pFriend = m_visibleList.logic_actors_closeby.m_pData[kk];
				if (pFriend == actor)
					continue;
				if ((pFriend->templateActor.actorClass == actor->templateActor.actorClass) && (pFriend->fLife > 0.0f))
				{
					if (IsLineOfSight(pFriend->GetPosHeart(), actor->GetPosHeart()))
					{
						bHasFriends = true;
						break;
					}
				}
			}

			if (!bHasFriends)
			{
				ret_bFinished = true;
				break;
			}

			//look to the event
			if (actor->m_AIsensorInfo.m_AIcurrentEvent.nType != K_LVL_AI_EVENT_NONE)
			{
				actor->m_AIcommands.nLookDirX = SIGN(actor->m_AIsensorInfo.m_AIcurrentEvent.pos.x - actor->pos.x);
			}
			//add sound threat event
			D3DXVECTOR2 vPos = actor->GetPosHeart();
			if (actor->m_AIsensorInfo.pTargetedActor != NULL)
			{
				vPos = actor->m_AIsensorInfo.pTargetedActor->GetPosHeart();
			}
			else if (actor->m_AIsensorInfo.m_AIcurrentEvent.nType > K_LVL_AI_EVENT_IDLE_TICK)
			{
				vPos = actor->m_AIsensorInfo.m_AIcurrentEvent.pos;
			}
			AddAIEvent(K_LVL_AI_EVENT_SOUND_THREAT, actor->GetUID(), K_LVL_ACT_CLASS_PLAYER, vPos, 256.0f, 1.0f);

			//play verse
			CVariantComplex* cvc = pNewBehavior->m_vcolParams.GetVariantByName(L"sVerseName");
			if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
			{
				EActorSoundVerse eVerse = (EActorSoundVerse)GetListIndexByNameHash(cvc->m_strArg.getHash(), EActorSoundVerseNames, K_LVL_ACT_VERSES_COUNT);
				if (eVerse != K_LVL_ACT_VERSE_EMPTY)
				{
					PlayActorSoundVerse(actor, eVerse);
				}
				else
				{
					ErrorBox(K_ERR_WARNING, L"BEHAVIOR_SHOW_ENEMY - Verse name not found!");
				}
			}
			else
			{
				ErrorBox(K_ERR_WARNING, L"BEHAVIOR_SHOW_ENEMY - sVerseName param not found!");
			}
			
			//save animation identifier
			cvc = pNewBehavior->m_vcolParams.GetVariantByName(L"sAnimIdentifier");
			if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
			{
				actor->AIvar1 = GetListIndexByName(cvc->m_strArg.text, EActorAnimNames, K_LVL_ACT_ANIMS_CNT);
			}
			else
			{
				//no animation to wait for, then return instantly
				ret_bFinished = true;
			}

			actor->fFOVPercent = 1.0f; //disable FOV check while attacking
		}
		break;
		case AI_BEHAVIOR_SET_STATE:
		{
			CVariantComplex* vc = pNewBehavior->m_vcolParams.GetVariantByName(L"sState");
			if (vc->m_type != CVariantComplex::K_ARGTYPE_STRING)
			{
				ErrorBox(K_ERR_WARNING, L"AI_BEHAVIOR_SET_STATE: sState arg not set or wrong type!");
				break;
			}

			CAIState* newstate = actor->templateActor.AItemplate->GetAIStateByName(vc->m_strArg);
			if (newstate == null)
			{
				LOG(L"AI_BEHAVIOR_SET_STATE - state not found! %s\n", vc->m_strArg.text);
				break;
			}
			//everything ok, set state
			SetActorAIState(actor, newstate);
			//!!! make sure we stay:
			ret_bFinished = false;
		}
		break;
		case AI_BEHAVIOR_TELEPORT_TO_TOUCHABLE_TARGET:
		{
			if ((actor->pClosestTouchable == null) || (actor->pClosestTouchable->pTarget == null))
			{
				LOG(L"AI_BEHAVIOR_TELEPORT_TO_TOUCHABLE_TARGET::touchable is null or touchable target is null!");
				break;
			}

			//teleportam playerul pe targetul lui closest touchable. Se presupune ca scriptul de pe touchable ii activeaza starea de SOLO_TELEPORT
			actor->SetPos(actor->pClosestTouchable->pTarget->pos);
			//set open frame (if necessary) on target
			IActiveInterface* active = actor->pClosestTouchable->pTarget;
			if ((active->AIstate == K_AI_STATE_ACTIVE_DOORFACE_AUTOCLOSE) || (active->AIstate == K_AI_STATE_ACTIVE_TEAM_TELEPORTER_2FRAMES))
			{
				bool bDontChangeFrames = (bool)(active->varAIparams.GetVariantByName(L"b_DontChangeFrames")->m_asINT32);
				if (!bDontChangeFrames)
					active->AItimer1 = 1.0f;
			}
			//state doesn't need update
			ret_bFinished = true;
		}
		break;
		case AI_BEHAVIOR_PLAYER_TEAM_TELEPORT:
		{
			actor->AIsubState = 0;
			//setez teleport slots
			m_nTeleportSlots++;
			m_bTeleportActivated = false;
			m_fTeleportTimer = 0.0f;
			//cand intra in stare ma reasigur ca e setat pointerul de teleport source
			if (m_pTeleportSource == null)
			{
				m_pTeleportSource = actor->pClosestTouchable;
			}
		}
		break;
		case AI_BEHAVIOR_IDLE_CROUCHED:
		{
			//save fadeout duration
			actor->AIfvar1 = pNewBehavior->m_vcolParams.GetVariantByName(L"fFadeOutDuration")->asFloat();
		}
		break;
		case AI_BEHAVIOR_HOSTAGE:
		{
			actor->AIsubState = 0;
			//actor doesn't try to escape:
			actor->AIvarBool1 = false;
			//can hostage escape?
			float fProbability = pNewBehavior->m_vcolParams.GetVariantByName(L"fRunProbability")->asFloat();
			if (m_rand.RandFloat(100.0f) < fProbability * 100.0f)
			{
				//we have a runner!
				actor->AIvarBool1 = true;
			}
		}
		break;
		case AI_BEHAVIOR_FLY_AWAY:
		{
			actor->bHasGravity = false;
		}
		break;
		case AI_BEHAVIOR_IDLE:
		{
		}
		break;
		case AI_BEHAVIOR_SET_ANIMSET:
		{
			actor->SetAnimSet(pNewBehavior->m_vcolParams.GetVariantByName(L"nSet")->m_asINT32);
			//state doesn't need update
			ret_bFinished = true;
		}
		break;
		case AI_BEHAVIOR_SET_CAPS:
		{
			CVariantComplex* cvNotATarget = pNewBehavior->m_vcolParams.GetVariantByName(L"nNotATarget");
			if (cvNotATarget->m_type != CVariantComplex::K_ARGTYPE_NONE)
			{
				bool bVal = (cvNotATarget->m_asINT32 != 0);
				if (bVal)
					actor->templateActor.eCaps |= CActorTemplate::K_ACT_CAPS_NOT_A_TARGET;
				else
					actor->templateActor.eCaps &= ~CActorTemplate::K_ACT_CAPS_NOT_A_TARGET;
			}

			CVariantComplex* cvCanBeDetonated = pNewBehavior->m_vcolParams.GetVariantByName(L"nCanBeDetonated");
			if (cvCanBeDetonated->m_type != CVariantComplex::K_ARGTYPE_NONE)
			{
				bool bVal = (cvCanBeDetonated->m_asINT32 != 0);
				if (bVal)
					actor->templateActor.eCaps |= CActorTemplate::K_ACT_CAPS_CAN_BE_DETONATED;
				else
					actor->templateActor.eCaps &= ~CActorTemplate::K_ACT_CAPS_CAN_BE_DETONATED;
			}

			CVariantComplex* cvHasExplosiveVest = pNewBehavior->m_vcolParams.GetVariantByName(L"nHasExplosiveVest");
			if (cvHasExplosiveVest->m_type != CVariantComplex::K_ARGTYPE_NONE)
			{
				bool bVal = (cvHasExplosiveVest->m_asINT32 != 0);
				if (bVal)
					actor->templateActor.eCaps |= CActorTemplate::K_ACT_CAPS_HAS_EXPLOSIVE_VEST;
				else
					actor->templateActor.eCaps &= ~CActorTemplate::K_ACT_CAPS_HAS_EXPLOSIVE_VEST;
			}
			//state doesn't need update
			ret_bFinished = true;
		}
		break;
		case AI_BEHAVIOR_BARREL_EXPLODING:
		{
			//burns with flame?
			CVariantComplex* cve = pNewBehavior->m_vcolParams.GetVariantByName(L"nCanBurn");
			actor->AIvarBool1 = true;
			if ((cve->m_type != CVariantComplex::K_ARGTYPE_NONE) && (cve->asInt32() == 0))
				actor->AIvarBool1 = false;

			actor->AIsubState = 0;
			//setez din start comanda de explode ca atunci cand trece in dead sa explodeze
			actor->varAIparams.SetNamedVarUINT32(L"nExplode", hash_EXPLO_BARREL);
			//special value that tells the engine that the explosion will have the last damager's UID so we can transmit barrel kills to players
			actor->varAIparams.SetNamedVarBool(L"bUseDamagerUID", true);
			//barrels aren't animated if IDLE animation doesn't loop
			if (!m_sprActors.IsLooping(actor->templateActor.animIDs[K_LVL_ACT_ANIM_IDLE][0]))
			{
				actor->bAnimated = false;
				actor->sprite.currentFrame = m_rand.RandInt(m_sprActors.GetAFramesCnt(actor->templateActor.animIDs[K_LVL_ACT_ANIM_IDLE][0]));
			}
		}
		break;
		case AI_BEHAVIOR_FLEE:
		{
			actor->AItimer1 = 0.0f;
			//distance to run at
			actor->AIfvar1 = pNewBehavior->m_vcolParams.GetVariantByName(L"fFleeDistance")->m_asFloat;
			if (actor->AIfvar1 <= 0.0f)
				actor->AIfvar1 = actor->templateActor.distHear;

			actor->AIsubState = 0;
		}
		break;
		case AI_BEHAVIOR_BLIND_RUN:
		{
			actor->AItimer1 = 0.0f;
			//save fadeout duration
			actor->AIfvar1 = pNewBehavior->m_vcolParams.GetVariantByName(L"fFadeOutDuration")->asFloat();
		}
		break;
		case AI_BEHAVIOR_HOLD_POSITION:
		{
		}
		break;
		case AI_BEHAVIOR_PATROL:
		{
			//save wait timer
			actor->AIfvar1 = pNewBehavior->m_vcolParams.GetVariantByName(L"fWaitTimer")->asFloat();
			actor->AItimer1 = 0.0f;
			//patrol faster?
			actor->AIvarBool1 = (pNewBehavior->m_vcolParams.GetVariantByName(L"nRun")->asInt32() != 0);
			//can he open doors?
			actor->AIvarBool2 = (pNewBehavior->m_vcolParams.GetVariantByName(L"nOpenUnlockedDoors")->asInt32() != 0);
		}
		break;
		case AI_BEHAVIOR_PATROL_BREAK_DOORS:
		{
			//save wait timer
			actor->AIfvar1 = pNewBehavior->m_vcolParams.GetVariantByName(L"fWaitTimer")->asFloat();
			actor->AItimer1 = 0.0f;
			//patrol faster?
			actor->AIvarBool1 = (pNewBehavior->m_vcolParams.GetVariantByName(L"nRun")->asInt32() != 0);
			//set on patroling
			actor->AIsubState = 0;
			//break door probability
			actor->AIfvar2 = pNewBehavior->m_vcolParams.GetVariantByName(L"fBreakProb")->asFloat();
		}
		break;
		case AI_BEHAVIOR_RUN_AWAY:
		{
			//running direction - to be set later on
			actor->AIvar1 = 0;
			//can he open doors?
			actor->AIvarBool2 = (pNewBehavior->m_vcolParams.GetVariantByName(L"nOpenUnlockedDoors")->asInt32() != 0);
		}
		break;
		case AI_BEHAVIOR_TELEPORT_NEAR_ENEMY:
		{
			actor->fFOVPercent = 1.0f; //disable FOV check while attacking
			actor->AIsubState = 0;
			//save max distance to target
			actor->AIvar1 = pNewBehavior->m_vcolParams.GetVariantByName(L"nTargetMaxDist")->asInt32();
		}
		break;
		case AI_BEHAVIOR_WAIT_FOR_ACTION:
		{
			actor->fFOVPercent = 1.0f;
			//var that tells the enemy when he can shoot
			actor->AIfvar1 = pNewBehavior->m_vcolParams.GetVariantByName(L"fShootPeriod")->asFloat();
			//timer that keeps actual time
			actor->AItimer1 = actor->AIfvar1;
		}
		break;
		case AI_BEHAVIOR_DETONATE_NEARBY:
		{
			actor->AIvarBool1 = false; //not detonated
			actor->AIvar1 = -1; //no anim set
			//salvez identificatorul animatiei
			CVariantComplex* cvc = pNewBehavior->m_vcolParams.GetVariantByName(L"sAnimIdentifier");
			if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
			{
				actor->AIvar1 = GetListIndexByName(cvc->m_strArg.text, EActorAnimNames, K_LVL_ACT_ANIMS_CNT);
			}
			//get nearby target

			actor->AItargetUID = 0;
			float fDistCurrent = 1000000.0f;
			//get closest detonation target
			for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
			{
				CActor* tact = m_arrActors[kk];
				if ((tact->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_CAN_BE_DETONATED) == 0)
					continue;
				D3DXVECTOR2 vDelta = tact->GetPosHeart() - actor->GetPosHeart();
				float fDist = D3DXVec2Length(&vDelta);
				//too far?
				if (fDist > actor->templateActor.distSee)
					continue;

				if (fDist < fDistCurrent)
				{
					if (IsLineOfSight(actor->GetPosHeart(), tact->GetPosHeart()))
					{
						fDistCurrent = fDist;
						actor->AItargetUID = tact->GetUID();
					}
				}
			}
		}
		break;
		case AI_BEHAVIOR_CHANGE_COLOR:
		{
			float fDuration = pNewBehavior->m_vcolParams.GetVariantByName(L"fTotalDuration")->m_asFloat;
			CLAMP(fDuration, 0.0f, 60.0f);

			float fAlpha = 0.0f;
			CVariantComplex* cvc = pNewBehavior->m_vcolParams.GetVariantByName(L"fAlpha");
			if (cvc->m_type == CVariantComplex::K_ARGTYPE_FLOAT)
				fAlpha = cvc->m_asFloat;
			CLAMP(fAlpha, 0.0f, 1.0f);

			actor->AItimer1 = 0.0f;
			actor->AItimer2 = fDuration;
			actor->AIfvar1 = fAlpha;
		}
		break;
		case AI_BEHAVIOR_PLAY_ANIM:
		{
			//salvez identificatorul animatiei
			CVariantComplex* cvc = pNewBehavior->m_vcolParams.GetVariantByName(L"sAnimIdentifier");
			if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
			{										  
				actor->AIvar1 = GetListIndexByName(cvc->m_strArg.text, EActorAnimNames, K_LVL_ACT_ANIMS_CNT);
			}
			else
			{
				ErrorBox(K_ERR_WARNING, L"Behavior PLAY ANIM sAnimIdentifier not set!");
			}

			//save dest alpha param (defaults on 1.0)
			actor->AIfvar1 = 1.0f;
			if (pNewBehavior->m_vcolParams.GetVariantByName(L"fDestAlpha")->m_type != CVariantComplex::K_ARGTYPE_NONE)
			{
				actor->AIfvar1 = pNewBehavior->m_vcolParams.GetVariantByName(L"fDestAlpha")->asFloat();
			}
			//save actual alpha
			actor->AIfvar2 = D3DCOLOR_GETFALPHA(actor->color);
		}
		break;
		case AI_BEHAVIOR_RUN_SCRIPT:
		{
			bool bWaitScriptEnd = false;
			CVariantComplex* cve = pNewBehavior->m_vcolParams.GetVariantByName(L"bWaitScriptEnd");
			if (cve->m_type != CVariantComplex::K_ARGTYPE_NONE)
				bWaitScriptEnd = cve->m_asBool;
			actor->AIvar1 = 0;
			if (bWaitScriptEnd)
				actor->AIvar1 = 1;

 			bool bTouchTarget = false;
			CVariantComplex* cvb = pNewBehavior->m_vcolParams.GetVariantByName(L"bTouchTarget");
			if (cvb->m_type != CVariantComplex::K_ARGTYPE_NONE)
				bTouchTarget = cvb->m_asBool;

			UINT32 nScriptOverride = 0;
			CVariantComplex* cvc = pNewBehavior->m_vcolParams.GetVariantByName(L"sScriptOverride");
			if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
			{
				nScriptOverride = cvc->m_strArg.textHash;
			}
			//Run script
			actor->Touch(actor->GetUID(), 0.0f, nScriptOverride, bTouchTarget);
		}
		break;
		case AI_BEHAVIOR_PLAY_VERSE:
		{
			CVariantComplex* cvc = pNewBehavior->m_vcolParams.GetVariantByName(L"sVerseName");
			if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
			{
				EActorSoundVerse eVerse = (EActorSoundVerse)GetListIndexByNameHash(cvc->m_strArg.getHash(), EActorSoundVerseNames, K_LVL_ACT_VERSES_COUNT);
				if (eVerse != K_LVL_ACT_VERSE_EMPTY)
				{
					PlayActorSoundVerse(actor, eVerse);
				}
				else
				{
					ErrorBox(K_ERR_WARNING, L"BEHAVIOR_PLAY_VERSE- Verse name not found!");
				}
			}
			else
			{
				ErrorBox(K_ERR_WARNING, L"BEHAVIOR_PLAY_VERSE- sVerseName param not found!");
			}
			//state doesn't need update
			ret_bFinished = true;
		}
		break;
		case AI_BEHAVIOR_GENERATE_EFFECT:
		{
			float fSize = 1.0f;
			CVariantComplex* cvb = pNewBehavior->m_vcolParams.GetVariantByName(L"fSize");
			if (cvb->m_type != CVariantComplex::K_ARGTYPE_NONE)
				fSize = cvb->m_asFloat;

			CVariantComplex* cvc = pNewBehavior->m_vcolParams.GetVariantByName(L"sEffectType");
			if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
			{
				GenerateEffect(cvc->m_strArg, actor->posHeart, fSize);
			}
			else
			{
				ErrorBox(K_ERR_WARNING, L"BEHAVIOR_GENERATE_EFFECT - sEffectType parameter not found!");
			}
			//state doesn't need update
			ret_bFinished = true;
		}
		break;

		case AI_BEHAVIOR_FIND_CLOSEST_PLAYER:
		{
			//se intoarce catre player
			CActor* plAct = GetClosestPlayer(actor);
			if (plAct != null)
			{
				actor->m_AIcommands.nLookDirX = SIGN(plAct->GetPosHeart().x - actor->GetPosHeart().x);
			}

			actor->fFOVPercent = 1.0f;  //disable FOV check while attacking
			actor->AIfvar1 = 0.0f;		//teleporter timer
			actor->AItimer1 = 0.0f;		//forget about target timer (usually 10 sec)
		}
		break;

		case AI_BEHAVIOR_SURPRISED:
		{
			//se intoarce catre event
			if (actor->m_AIsensorInfo.m_AIcurrentEvent.nType > K_LVL_AI_EVENT_IDLE_TICK)
			{
				actor->m_AIcommands.nLookDirX = SIGN(actor->m_AIsensorInfo.m_AIcurrentEvent.pos.x - actor->pos.x);
			}
			//set wait timer
			actor->AItimer1 = pNewBehavior->m_vcolParams.GetVariantByName(L"fWaitTimer")->asFloat();
			actor->AIsubState = 0;

			actor->fFOVPercent = 1.0f; //disable FOV check while attacking

			//#PERK: INVISIBLE - HUMAN enemies react slower when seeing you
			if ((actor->m_AIsensorInfo.m_AIcurrentEvent.nType == K_LVL_AI_EVENT_SEE_ENEMY) &&
				(actor->m_AIsensorInfo.pTargetedActor != null) &&
				(actor->templateActor.actorClass == K_LVL_ACT_CLASS_HUMAN) &&
				(actor->m_AIsensorInfo.pTargetedActor->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER) &&
				(g_playerSelScr.IsPerkEnabled(actor->m_AIsensorInfo.pTargetedActor->nPlayerOrdinal, &shPerk_INVISIBLE)))
			{
				actor->AItimer1 += 0.5f;
			}
		}
		break;

		case AI_BEHAVIOR_HUMAN_SHIELD_ATTACK:
		{
			actor->fFOVPercent = 1.0f; //disable FOV check while attacking
			actor->AItargetUID = 0; //unset target ID (this will be the hostage UID)
			actor->AIsubState = 0; //0-looking for hostage, 1-normal attack

			actor->AIvarBool1 = true;	//decide movement helper var

			actor->AItimer1 = 0.0f;		
		}
		break;
		case AI_BEHAVIOR_GET_IN_COVER:
		{
			actor->fFOVPercent = 1.0f; //disable FOV check while attacking
			actor->AIsubState = 0;
		}
		break;
		case AI_BEHAVIOR_GUNPOINT_HOSTAGE:
		{
			actor->fFOVPercent = 1.0f; //disable FOV check while attacking
			actor->AIsubState = 0;
			//save execute delay
			actor->AIfvar1 = pNewBehavior->m_vcolParams.GetVariantByName(L"fExecuteDelay")->asFloat();
			if (actor->AIfvar1 <= 0.0f)
				actor->AIfvar1 = 2.0f; //defaults on 0
		}
		break;
		case AI_BEHAVIOR_ATTACK_COVER:
		{
			actor->fFOVPercent = 1.0f; //disable FOV check while attacking
			actor->AIvarBool1 = true; //decide movement helper var
			actor->AIsubState = 0;
			actor->AItimer1 = 0.0f;	//generic timer for decision making
		}
		break;
		case AI_BEHAVIOR_BIGSHOT_ATTACK:
		{
			actor->fFOVPercent = 1.0f; //disable FOV check while attacking
			actor->AIvarBool1 = true; //decide movement helper var
			actor->AIsubState = 0;	//decision state
			actor->AItimer1 = 0.0f;	//generic timer for decision making
			//make sure we can't interact with him until he gives up
			actor->bCanInteract = false;
			actor->AIvar1 = 0; //surrender times
		}
		break;
		case AI_BEHAVIOR_ESCAPE_ARREST:
		{
			actor->fFOVPercent = 1.0f; //disable FOV check while attacking
			actor->AIsubState = 0;	//decision state
			actor->AItimer1 = 0.0f;	//generic timer for decision making
			//make sure we can't interact with him until he gives up
			actor->bCanInteract = false;
			actor->AIvar1 = 0; //run direction

		    //can he open doors?
			actor->AIvarBool2 = (pNewBehavior->m_vcolParams.GetVariantByName(L"nOpenUnlockedDoors")->asInt32() != 0);
		}
		break;
		case AI_BEHAVIOR_ATTACK_BACKSTAB:
		case AI_BEHAVIOR_ATTACK_HITNRUN:
		case AI_BEHAVIOR_ATTACK:
		{
			actor->fFOVPercent = 1.0f; //disable FOV check while attacking
			actor->AIvarBool1 = true; //decide movement helper var
			actor->AIsubState = 0;
			actor->AItimer1 = 0.0f;	//generic timer for decision making
		}
		break;
		case AI_BEHAVIOR_SHIELDBOSS_ATTACK:
		{
			actor->fFOVPercent = 1.0f; //disable FOV check while attacking
			actor->AIvarBool1 = true; //decide movement helper var
			actor->AIsubState = 0;
			actor->AItimer1 = 0.0f; //timer for charges
			actor->AItimer2 = 0.0f; //timer for molotov
			actor->AIfvar1 = 0.0f; //normal attack duration
		}
		break;
		case AI_BEHAVIOR_TATTOOBOSS_ATTACK:
		{
			actor->fFOVPercent = 1.0f; //disable FOV check while attacking
			actor->AIvarBool1 = true;
			actor->AIsubState = 0;
			actor->AItimer1 = 10.0f;	//drink potion timer
			actor->AItimer2 = 0.0f;		//berserk timer (only stabs)
			actor->AIfvar1 = 0.0f; 
		}
		break;
		case AI_BEHAVIOR_JACKEDJONES_ATTACK:
		{
			actor->fFOVPercent = 1.0f; //disable FOV check while attacking
			actor->AIvarBool1 = true; //decide movement helper var
			actor->AIsubState = 0;
			actor->AItimer1 = 0.0f;	//generic timer for decision making
		}
		break;
		case AI_BEHAVIOR_SUICIDE:
		{
			actor->bOnLadder = false;
			actor->bCrouched = false;
			actor->pCover = null;
			actor->fStunTimer = 0.0f;

			actor->m_AIcommands.nIconType = K_LVL_ACT_ICON_REMOVE_ICON;
			actor->m_AIcommands.fIconDuration = -1.0f;

			actor->fLife = 0.0f; //kill it
			//trateaza death commands din script
			EActorDeathCommand dcmd = K_LVL_ACT_DEATHCMD_NONE;
			CVariantComplex* cvc = pNewBehavior->m_vcolParams.GetVariantByName(L"sDeathCommand");
			if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
			{
				int dcmd = GetListIndexByName(cvc->m_strArg.text, EActorDeathCommandNames, K_LVL_ACT_DEATHCMD_CNT);
				//daca avem comanda de death o trimitem mai departe
				if(dcmd >= 0)
					actor->varAIparams.SetNamedVarINT32(L"nDeathCommand", dcmd);
			}
			//state doesn't need update
			ret_bFinished = true;
		}
		break;
		case AI_BEHAVIOR_DEAD:
		{
			//make sure he's dead!
			bool bSpawnedDead = false;
			if (actor->fLife > 0.0f)
			{
				actor->fLife = 0.0f;
				bSpawnedDead = true;
			}
			actor->fArmor = 0.0f;

			actor->bOnLadder = false;
			actor->bCrouched = false;
			actor->pCover = null;
			actor->fStunTimer = 0.0f;
			//death timer for players or splat timer for others
			actor->AItimer1 = 0.0f;
			if (actor->templateActor.actorClass != K_LVL_ACT_CLASS_PLAYER) 
			{
				CVariantComplex* cvt = pNewBehavior->m_vcolParams.GetVariantByName(L"fSplatTimer");
				if (cvt->m_type == CVariantComplex::K_ARGTYPE_FLOAT)
					actor->AItimer1 = cvt->asFloat();
				//#ZOMBIE: was biten? make victim explode and turn
				if ((actor->AItimer1 <= 0.0f) && ((actor->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_TURN_TO_ZOMBIE) != 0) )
				{
					actor->AItimer1 = 3.0f + m_rand.RandFloat(3.0f);
				}
			}

			//remove icons
			actor->m_AIcommands.nIconType = K_LVL_ACT_ICON_REMOVE_ICON;
			actor->m_AIcommands.fIconDuration = -1.0f;
			actor->m_AIcommands.ResetMoveCommands();
			//reset color
			actor->m_AIcommands.nColor = actor->color_ini;
			//reset overhead icons
			actor->m_sprOverheadIcon.animationIdx = -1;
			//stop weapons
			for (int kk = 0; kk < K_LVL_ACT_WEAPONS_CNT; kk++)
			{
				StopReloadingWeapon(actor->pSelectedWeapon[kk]);
				JamWeapon(actor->pSelectedWeapon[kk]);
			}
			//trateaza death commands din script
			EActorDeathCommand dcmd = K_LVL_ACT_DEATHCMD_NONE;
			CVariantComplex* cvc = pNewBehavior->m_vcolParams.GetVariantByName(L"sDeathCommand");
			if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
			{
				dcmd = (EActorDeathCommand)GetListIndexByName(cvc->m_strArg.text, EActorDeathCommandNames, K_LVL_ACT_DEATHCMD_CNT);
				//daca avem comanda de death o trimitem mai departe
				if (dcmd >= K_LVL_ACT_DEATHCMD_NONE)
					actor->varAIparams.SetNamedVarINT32(L"nDeathCommand", (int)dcmd);
			}
			//trateaza death script
			CVariantComplex* cvs = pNewBehavior->m_vcolParams.GetVariantByName(L"sDeathScript");
			if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
			{
				actor->varAIparams.AddVariant(cvs);
			}

			if (actor->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER)
			{
				//timerul este folosit ca sa nu sara camera de pe cadavru prea repede
				actor->AItimer1 = K_LVL_PLAYER_DEATH_TIMER;
				//daca nu mai are vieti pun un timer mai mic dar il pun totusi ca sa nu sara camera prea repede
				if (m_arrStats[K_LVL_STATS_PL1_LIVES + actor->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT] <= 0)
					actor->AItimer1 = K_LVL_PLAYER_DEATH_TIMER * 0.25f;

				//actor->varAIparams.SetNamedVarINT32(L"nDeathCommand", K_LVL_ACT_DEATHCMD_RESET_TO_ZERO);
				//--- STATISTICS ---
				m_arrStats[K_LVL_STATS_PL1_DEATHS + actor->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT]++;

				m_arrPlayerSelStrategic[actor->nPlayerOrdinal] = -1;
				m_interfaceIGM.SetStrategicSelection(actor->nPlayerOrdinal, -1);
				//remove icon
				actor->SetIcon(K_LVL_ACT_ICON_NONE);
				//dam remove la particles de pe interfata cand moare un player
				g_particlesMgr.RemoveAllFromLayer(K_PART_LAYER_INTERFACE_LIGHT);
			}
			else if (actor->templateActor.actorClass >= K_LVL_ACT_CLASS_HUMAN)
			{
				if (!bSpawnedDead)
				{
					//counts online coop victims too but keeps achievements separated (steam counter)
					App_IncreaseGamestat(K_MEMID_GAMESTATS_ENEMIES_KILLED, 1);
					//statistics for each class
					CActor* pPlayer = g_level.GetPlayerByUID(actor->m_AIsensorInfo.m_lastInteractingActorUID);
					if ((pPlayer != null) && (!IsNetworkPlayer(pPlayer)))
					{
						switch (g_playerSelScr.m_arrPlayers[pPlayer->nPlayerOrdinal].eType)
						{
							case K_PSS_CLASS_ASSAULTER:
								App_IncreaseGamestat(K_MEMID_GAMESTATS_KILLS_ASSAULTER);
								break;
							case K_PSS_CLASS_BREACHER:
								App_IncreaseGamestat(K_MEMID_GAMESTATS_KILLS_BREACHER);
								break;
							case K_PSS_CLASS_SHIELD:
								App_IncreaseGamestat(K_MEMID_GAMESTATS_KILLS_SHIELD);
								break;
							case K_PSS_CLASS_FBI_AGENT:
								App_IncreaseGamestat(K_MEMID_GAMESTATS_KILLS_FBI);
								break;
							case K_PSS_CLASS_RECON:
								App_IncreaseGamestat(K_MEMID_GAMESTATS_KILLS_RECON);
								break;
							case K_PSS_CLASS_OFFDUTYGUY:
								App_IncreaseGamestat(K_MEMID_GAMESTATS_KILLS_OFFDUTY);
								break;
						}
					}

				}

			}

			//play death verses
			if ((dcmd == K_LVL_ACT_DEATHCMD_NONE) && (!bSpawnedDead))
				PlayActorSoundVerse(actor, K_LVL_ACT_VERSE_DIE);

			//hostages specials
			if (actor->templateActor.actorClass == K_LVL_ACT_CLASS_HOSTAGE)
			{
				//save stats for saviour only if deallocating by itself (not killed)
				if (dcmd == K_LVL_ACT_DEATHCMD_DEALLOCATE)
				{
					UINT32 nToucherUID = actor->varAIparams.GetVariantByName(L"nToucherUID")->m_asUINT32;
					CActor* pact = GetPlayerByUID(nToucherUID);
					if (pact)
					{
						m_arrStats[K_LVL_STATS_PL1_HOSTAGES_SAVED + pact->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT]++;
					}
					else
					{
						ErrorBox(K_ERR_WARNING, L"Hostage save went unnoticed! level %d chapter %d", m_nLoadedLevel + 1, m_nLoadedChapter + 1);
					}
					
					App_IncreaseGamestat(K_MEMID_GAMESTATS_HOSTAGES_SAVED);
					GiveStrategicPoints(actor->templateActor.fStrategicPoints, &D3DXVECTOR2(actor->bbox.vCenter.x, actor->bbox.vMin.y));
					//make sure we release it on the next frame
					actor->bReleaseIt = true;
				}
				else //hostage killed
				{
					CActor* pPlayer = GetPlayerByUID(actor->nLastDamageTakenFromUID);
					if (pPlayer != null)
					{
						HitActor(pPlayer, pPlayer->fLife * 0.25f, 0, K_LVL_ACT_CLASS_TRAP, null,
							K_LVL_BULLET_FLAG_NO_IMPACT_PARTICLES | K_LVL_BULLET_FLAG_NOT_BALLISTIC | K_LVL_BULLET_FLAG_IGNORE_ARMOR | K_LVL_BULLET_FLAG_IGNORE_COVER | K_LVL_BULLET_FLAG_NO_DECALS, 10, 0.0f);
						if (!IsNetworkPlayer(pPlayer))
						{
							//count only hostages killed by local players
							App_IncreaseGamestat(K_MEMID_GAMESTATS_HOSTAGES_KILLED);
						}
					}
					//--- level stats ---
					if (!bSpawnedDead)
					{
						m_arrStats[K_LVL_STATS_HOSTAGES_KILLED]++;
					}
				}
			}
			//make sure we release it on the next frame
			if (dcmd == K_LVL_ACT_DEATHCMD_DEALLOCATE)
			{
				actor->bReleaseIt = true;
			}
		}
		break;
		default:
			break;
	}

	return true;
}

void CLevel::SetActorDoT(CActor* act, CDamageOverTime::EDoTType eType, float fDuration, float fDamagePerSec, EActorClass eExcludedClass, EActorClass eFilterClass, DWORD dwOwnerUID)
{
	if (act == null)
		return;
	if ((eFilterClass > K_LVL_ACT_CLASS_ANY) && (act->templateActor.actorClass != eFilterClass))
		return;
	if ((eExcludedClass > K_LVL_ACT_CLASS_ANY) && (act->templateActor.actorClass == eExcludedClass))
		return;

	if ((eType == CDamageOverTime::K_LVL_DoT_INTIMIDATED) && (act->fLife <= 0.0f))
		return;

	//#HARDCODE: DoT_TARGETED only works on enemies
	if ((eType == CDamageOverTime::K_LVL_DoT_TARGETED) && (act->templateActor.actorClass < K_LVL_ACT_CLASS_HUMAN))
		return;

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	//LOG(L"- SetDoT %s for %.4f", act->templateActor.shName.text, eType);
#endif

	if (act->cDamageOverTime.Set(eType, fDuration, fDamagePerSec, eExcludedClass, eFilterClass, dwOwnerUID))
	{
		//pointer to player owner or null if not a player
		CActor* pPlayerOwner = GetPlayerByUID(dwOwnerUID);
		//special statistics
		if ((eType == CDamageOverTime::K_LVL_DoT_FIRE) && (act->templateActor.actorClass >= K_LVL_ACT_CLASS_HUMAN))
		{
			if((pPlayerOwner != null) && (!IsNetworkPlayer(pPlayerOwner)))
				App_IncreaseGamestat(K_MEMID_GAMESTATS_ENEMIES_SET_ON_FIRE);
		}
		//intimidated icon
		if (eType == CDamageOverTime::K_LVL_DoT_INTIMIDATED)
			act->SetIcon(K_LVL_ACT_ICON_SCARED, fDuration);

		//set some perks again
		if (pPlayerOwner != null)
		{
			if ((eType == CDamageOverTime::K_LVL_DoT_TARGETED) || (eType == CDamageOverTime::K_LVL_DoT_TARGETED_ALLY))
			{
				//#PERK: R2_RECON Bar percentage
				float fReconPerc = g_playerSelScr.GetUpgradeBarPercent(&g_playerSelScr.m_arrPlayers[pPlayerOwner->nPlayerOrdinal], L"R2_RECON");
				//set damage adder percent fVar1 here! (targeted enemies damage multiplier)
				act->cDamageOverTime.fVar1 = 0.3f + fReconPerc * 0.2f;

				//#PERK: DURACELLS - recon effects last longer
				if (g_playerSelScr.IsPerkEnabled(pPlayerOwner->nPlayerOrdinal, &shPerk_DURACELLS))
				{
					act->cDamageOverTime.fDuration_ini *= 1.5f;
					act->cDamageOverTime.fDuration = act->cDamageOverTime.fDuration_ini;
				}
			}
		}

	}
}

void CLevel::OnActorBehaviorFinished(CActor * actor, EAIBehaviorType eOldBehavior)
{
	switch (eOldBehavior)
	{
		case AI_BEHAVIOR_BIGSHOT_ATTACK:
		{
			actor->bCanInteract = false;
		}
		break;
		case AI_BEHAVIOR_ESCAPE_ARREST:
		{
			actor->bCanInteract = false;
		}
		break;

		case AI_BEHAVIOR_HUMAN_SHIELD_ATTACK:
		{
			//verifica sa dea release la ostatec daca l-ai omorat pe posesorul lui moare si ostatecul
			if (actor->AItargetUID != 0)
			{
				CActor * pHostage = GetActorByUID(actor->AItargetUID);
				if ((pHostage != null) && (pHostage->bHidden))
				{
					pHostage->bSetHidden = false;
					//daca mai are armura inseamna ca mai traieste ostatecul. daca nu, inseamna ca a murit si el
					if (actor->fArmor > 0.0f)
					{
						pHostage->fLife = actor->fArmor;
						actor->fArmor = 0.0f;
					}
					else
					{
						pHostage->fLife = 0.0f;
						pHostage->nLastDamageTakenFromUID = actor->nLastDamageTakenFromUID;
					}
				}
			}

			actor->AItargetUID = 0;
			actor->SetAnimSet(0); //revin la animatiile normale
			actor->fArmor = 0.0f;//resetez armura
		}
		break;
	}

}

//lista temporara de collision shapes folosita la coliziuni
CFixedArray<CCollisionShape*, 200> tempCollBoxList;

void CLevel::UpdateAI_actor(CActor* actor, float dTime)
{
	//touch timer reset
	actor->UpdateTouchTimerReset(dTime);

	//daca am schimbat vizibilitatea
	actor->bHidden = actor->bSetHidden;
	//daca este hidden nu mai verifica AI
	if (actor->bHidden)
		return;

	//update timeline
	actor->fTimelineAI += dTime;
	//update icon timers
	if (actor->fIconTimer > 0.0f)
	{
		actor->fIconTimer -= dTime;
		if (actor->fIconTimer <= 0.0f)
		{
			actor->SetIcon(K_LVL_ACT_ICON_NONE);
		}
	}

	//--- UPDATE ANIMATION ---
	//get displacement from anim moves
	D3DXVECTOR2 vAnimMove(0.0f, 0.0f);
	actor->sprite.pos.x = actor->sprite.pos.y = 0.0f; //resetez pozitia ca oricum se suprascrie la final de update
	UINT32 aframeFlag = 0; //flagul aframe-ului resetat
	UINT32 aframeFlag_feet = 0; //flagul aframe-ului resetat
	//update only if animated flag set
	if (actor->bAnimated)
	{
		aframeFlag = actor->sprite.Update(&m_sprActors, dTime, true);
		//animate feet if we have animation
		if (actor->sprite_feet.animationIdx >= 0)
		{
			//slow down feet anim too if weapon slows us down
			float fTimeAdv = dTime;
			//if ((actor->nAttackStatus > K_LVL_ACT_ATTACK_RELOADING) && (actor->collisionFlags & K_DIRFLAG_DOWN))
			//{
			//	fTimeAdv = dTime * (1.0f - actor->pCurrentWeapon->WeaponTemplate.fShooterSpeedSlowingPercent);
			//}

			aframeFlag_feet = actor->sprite_feet.Update(&m_sprActors, fTimeAdv, false);
		}
	}
	

	//------------------------------------------------------------------------------------------
	//	INTEGRATOR - physics
	//------------------------------------------------------------------------------------------
	//#TODO: check speed limits - should be done on the speed vector, normalized
	CLAMP(actor->speed.x, -K_LVL_ACTOR_MAX_SPEED, K_LVL_ACTOR_MAX_SPEED);
	CLAMP(actor->speed.y, -K_LVL_ACTOR_MAX_SPEED, K_LVL_ACTOR_MAX_SPEED);
	//update impulse
	D3DXVECTOR2 impFriction(K_LVL_GROUND_DEFAULT_FRICTION, K_LVL_GROUND_DEFAULT_FRICTION);
	//limit impulse
	CLAMP(actor->vSpeedImpulse.y, -K_LVL_ACTOR_MAX_IMPULSE, K_LVL_ACTOR_MAX_IMPULSE);
	CLAMP(actor->vSpeedImpulse.x, -K_LVL_ACTOR_MAX_IMPULSE, K_LVL_ACTOR_MAX_IMPULSE);
	//ATENTIE!!! daca trece prin usi inseamna ca bboxul din starea dead e mai lat decat cel din normal.

	actor->vSpeedImpulse.x -= actor->vSpeedImpulse.x * impFriction.x * dTime;
	actor->vSpeedImpulse.y -= actor->vSpeedImpulse.y * impFriction.y * dTime;
	//reset vertical impulse so we don't get pushed up (seems to jump for a frame)
	if (actor->vSpeedImpulse.y < 0.0f)
		actor->vSpeedImpulse.y = 0.0f;
	///--- move actor ---
	//setez viteza
	D3DXVECTOR2 movevec = actor->speed * dTime;
	
	//movevec += vAnimMove;//adaug si animatia exportata din editor (nu e in fn de dTime)
	D3DXVECTOR2 actorOldPos = actor->pos;
	//move actor to next position
	actor->pos += movevec; 

	if (actor->bHasCollision)
	{
		//1. fine bbox start and end union that includes all collisions when moving at high speeds
		CAABB destbox, oldbox;
		destbox = actor->bbox_ini;
		destbox.Move(actor->pos);
		oldbox = actor->bbox_ini;
		oldbox.Move(actorOldPos);
		//uniunea lor
		CAABB boxUnion = AABB_Union(destbox, oldbox);
		//optional
		boxUnion.Inflate(K_TILE_HSIZE, K_TILE_HSIZE);

		//#TODO: if it gets getting more expensive just use a quad tree on the collision boxes
		//array care tine pointeri la boxurile cu care e contact pe boxul final
		tempCollBoxList.Clear();

		for (int kk = 0; kk < m_arrColShapes.GetSize(); kk++)
		{
			if (m_arrColShapes[kk]->bHidden)
				continue;

			//nu am intersectie probabils - trec mai departe
			if (!boxUnion.Intersects(&m_arrColShapes[kk]->bbox))
				continue;

			//adauga bbox in lista de probabile pt intersectie
			if (m_arrColShapes[kk]->collFlags != K_DIRFLAG_NONE)
			{
				tempCollBoxList.Add(m_arrColShapes[kk]);

				//ne asiguram ca nu trecem prin cutii mici (solide) la viteze foarte mari
				//#TODO: implement other collision types here
				if (m_arrColShapes[kk]->type == K_LVL_COLL_TYPE_SOLID)
				{
					CAABB* box = &m_arrColShapes[kk]->bbox;
					if ((box->vSize.x <= fabs(movevec.x)) || (box->vSize.y <= fabs(movevec.y)))
					{
						//sweep test - enlarge bbox and check intersections between centers vector
						CAABB staticBoxGrown = *box;
						staticBoxGrown.Inflate(oldbox.vHalfSize.x, oldbox.vHalfSize.y);

						D3DXVECTOR2 vColPt;
						if (AABB_Segment_Intersection_NoHeads(oldbox.vCenter, destbox.vCenter, staticBoxGrown, &vColPt))
						{
							//repozitionam
							movevec = vColPt - oldbox.vCenter;
							actor->pos = actorOldPos + movevec;
							//destbox set
							destbox = actor->bbox_ini;
							destbox.Move(actor->pos);
						}
					}
				}

			}

		}

		//reset coll flags
		UINT16 unTotalFlags = 0;

		bool bSquashPlayer = false;
		if (tempCollBoxList.Count() > 0)
		{
			for (int kk = 0; kk < tempCollBoxList.Count(); kk++)
			{
				CCollisionShape *colshape = tempCollBoxList[kk];
				CAABB box = colshape->bbox; //copy box to edit it

				CAABB minkAABB = AABB_GetMinkowskiDifference(destbox, box);
				//verificam coliziune: daca nu contine originea nu e coliziune
				if ((minkAABB.vMin.x > 0.0f) || (minkAABB.vMin.y > 0.0f) || (minkAABB.vMax.x < 0.0f) || (minkAABB.vMax.y < 0.0f))
					continue;
				//daca avem coliziune gasim vectorul de penetrare adica distanta minima de la origine la margini
				float minx = fabs(minkAABB.vMin.x);
				float maxx = fabs(minkAABB.vMax.x);
				float miny = fabs(minkAABB.vMin.y);
				float maxy = fabs(minkAABB.vMax.y);

				int retDirFlag = K_DIRFLAG_NONE;
				D3DXVECTOR2 vPenetrate(0.0f, 0.0f);
				float mindist = 1000000.0f;

				if (colshape->collFlags & K_DIRFLAG_LEFT)
				{
					if (actor->speed.x == 0.0f)
					{
						mindist = minx;
						vPenetrate.x = -minx; vPenetrate.y = 0.0f;
						retDirFlag = K_DIRFLAG_LEFT;
					}
					else //ignore left collisions if difference is small (small stairs)
					{
						float fDY = actor->bbox.vMax.y - colshape->bbox.vMin.y;
						//don't ignore collision if stairs too high or feet not on ground
						if ((fDY > 4.0f) || (fDY <= 0.0f) || ((actor->collisionFlags & K_DIRFLAG_DOWN) == 0))
						{
							mindist = minx;
							vPenetrate.x = -minx; vPenetrate.y = 0.0f;
							retDirFlag = K_DIRFLAG_LEFT;
						}
					}
				}
				if ((maxx < mindist) && (colshape->collFlags & K_DIRFLAG_RIGHT))
				{
					if (actor->speed.x == 0.0f)
					{
						mindist = maxx;
						vPenetrate.x = maxx; vPenetrate.y = 0.0f;
						retDirFlag = K_DIRFLAG_RIGHT;
					}
					else //ignore right collisions if difference is small (small stairs)
					{
						float fDY = actor->bbox.vMax.y - colshape->bbox.vMin.y;
						if ((fDY > 4.0f) || (fDY <= 0.0f) || ((actor->collisionFlags & K_DIRFLAG_DOWN) == 0))
						{
							mindist = maxx;
							vPenetrate.x = maxx; vPenetrate.y = 0.0f;
							retDirFlag = K_DIRFLAG_RIGHT;
						}
					}
				}
				if ((maxy < mindist) && (colshape->collFlags & K_DIRFLAG_DOWN))
				{ 
					if (movevec.y >= 0.0f) //this if is optional but it helps when jumping near an interactible so we don't get the interact icon shown
					{
						mindist = maxy;
						vPenetrate.x = 0.0f; vPenetrate.y = maxy;
						retDirFlag = K_DIRFLAG_DOWN;
					}
				}
				if ((miny < mindist) && (colshape->collFlags & K_DIRFLAG_UP))
				{
					mindist = miny;
					vPenetrate.x = 0.0f; vPenetrate.y = -miny;
					retDirFlag = K_DIRFLAG_UP;
				}
				//daca nu da coliziune din cauza flagurilor ignor boxul
				if (retDirFlag == K_DIRFLAG_NONE)
					continue;

				//set collision flags
				unTotalFlags |= retDirFlag;
				//change actor placement
				actor->pos -= vPenetrate;
				movevec = actor->pos - actorOldPos;
				//destbox set
				destbox = actor->bbox_ini;
				destbox.Move(actor->pos);
				//still penetrating? SQUASH!
				if ((fabs(vPenetrate.x) > K_LVL_MAX_PENETRATION) || (fabs(vPenetrate.y) > K_LVL_MAX_PENETRATION))
				{
					bSquashPlayer = true;
				}
			}

			//set actor current collision flags
			actor->collisionFlags = unTotalFlags;

			//final check
			//squash - cand esti strivit moare cu splat
			if ((bSquashPlayer) &&
				( ((actor->collisionFlags & K_DIRFLAG_UP_DOWN) == K_DIRFLAG_UP_DOWN) || ((actor->collisionFlags & K_DIRFLAG_LEFT_RIGHT) == K_DIRFLAG_LEFT_RIGHT) ) )
			{
				HitActor(actor, -500.0f, actor->GetUID(), K_LVL_ACT_CLASS_EXPLOSION);
			}
			//DOWN collision
			if ((actor->speed.y > 0.0f) && (actor->collisionFlags & K_DIRFLAG_DOWN))
			{
				//reset speed to 0 !!!
				actor->speed.y = 0.0f;
			}
			//UP collision
			else if ((actor->speed.y < 0.0f) && (actor->collisionFlags & K_DIRFLAG_UP))
			{
				actor->speed.y = 0.0f;
			}
			//LEFT collision
			if ((actor->speed.x < 0.0f) && (actor->collisionFlags & K_DIRFLAG_LEFT))
			{
				actor->speed.x = 0.0f;
			}
			//RIGHT collision
			if ((actor->speed.x > 0.0f) && (actor->collisionFlags & K_DIRFLAG_RIGHT))
			{
				actor->speed.x = 0.0f;
			}
			//LEFT RIGHT collision - impulse
			if (((actor->vSpeedImpulse.x < 0.0f) && (actor->collisionFlags & K_DIRFLAG_LEFT)) ||
				((actor->vSpeedImpulse.x > 0.0f) && (actor->collisionFlags & K_DIRFLAG_RIGHT)) || 
				((actor->vSpeedImpulse.y < 0.0f) && (actor->collisionFlags & K_DIRFLAG_UP)) || 
				((actor->vSpeedImpulse.y > 0.0f) && (actor->collisionFlags & K_DIRFLAG_DOWN)))
			{
				actor->vSpeedImpulse.x = 0.0f;
			}
		}
		else
		{
			actor->collisionFlags = 0;
		}

	}
	//end phys

	//check world bounds for each actor - kill if out
	if (!PointInRect(actor->pos, m_levelAABB))
	{
		KillActor(actor);
	}

	///--- ACTOR CAPS ---
	//check if fall off ladder
	/*
	if (actor->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_CAN_CLIMB) 
	{
		//check bottom center of bbox
		D3DXVECTOR2 feetpos = D3DXVECTOR2(actor->bbox.vMin.x + actor->bbox.vHalfSize.x, actor->bbox.vMax.y);
		//ca sa iasa de pe ladder
		if (actor->bOnLadder)
		{
			bool bFeetOff = false;
			//verific caderea de pe scara doar cand ma misc
			if (actor->m_AIcommands.nMoveDirY != 0)
			{
				bFeetOff = true;
				for (int kk = 0; kk < tempCollBoxList.Count(); kk++)
				{
					if (((tempCollBoxList[kk]->collFlags & K_DIRFLAG_DOWN) == 0) || (tempCollBoxList[kk]->type != K_LVL_COLL_TYPE_LADDER))
						continue;

					if (actor->m_AIcommands.nMoveDirY < 0)
					{
						if ((tempCollBoxList[kk]->bbox.PointIn(feetpos.x, feetpos.y)) || (tempCollBoxList[kk]->bbox.PointIn(actor->posHeart.x, actor->posHeart.y)))
						{
							bFeetOff = false;
							break; //exit loop
						}
					}
					else if (actor->m_AIcommands.nMoveDirY > 0)
					{
						//daca am coliziune in partea de jos coboara de pe scara
						if (actor->collisionFlags & K_DIRFLAG_DOWN)
						{
							bFeetOff = true;
							break;
						}
						else if ((tempCollBoxList[kk]->bbox.PointIn(feetpos.x, feetpos.y)) || (tempCollBoxList[kk]->bbox.PointIn(actor->posHeart.x, actor->posHeart.y)))
						{
							bFeetOff = false;
							break; //exit loop
						}
					}
				}
			}
			//coboara de pe scara doar daca tii apasat pe jos
			if (bFeetOff)
			{
				actor->bOnLadder = false;
			}
		}
	}
	*/
	//--- find closest touchable ---
	if (actor->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_CAN_INTERACT)
	{
		IActiveInterface* pLowPrioTouch = null;
		actor->pClosestTouchable = null;
		//find the active that has the biggest bbox intersection surface with our player
		float fSurface = 0.0f;
		for (int kk = 0; kk < m_arrActivesPtrInteract.Count(); kk++)
		{
			CActive* activ = m_arrActivesPtrInteract.m_pData[kk];
			if ((activ->bHidden) || (!activ->bCanInteract))
				continue;
			//aici verificam cu bboxul setat in editor
			CAABB retAABB;
			if (AABB_Intersection(actor->bbox, activ->bbox_exported, retAABB))
			{
				//save low priority toucher if interact icon is hidden
				if (activ->bHideInteractIcon)
				{
					pLowPrioTouch = activ;
				}
				else
				{
					float fs = retAABB.vSize.x * retAABB.vSize.y;
					//prioritize section doors
					if ((activ->pTarget != null) && (activ->pTarget->AIstate == K_AI_STATE_COLL_BREAKABLE_DOOR))
						fs += 1000.0f;

					if (fs >= fSurface)
					{
						actor->pClosestTouchable = activ;
						fSurface = fs;
					}
				}
			}
		}
		//can touch actors too
		if (actor->pClosestTouchable == null)
		{
			for (int kk = m_visibleList.logic_actors_closeby.Count() - 1; kk >= 0; kk--)
			{
				CActor* act = m_visibleList.logic_actors_closeby.m_pData[kk];
				if ((act->bHidden) || (!act->bCanInteract) || (act->fLife <= 0.0f))
					continue;
				//aici verificam cu bboxul setat in editor
				if (act->bbox.Intersects(&actor->bbox))
				{
					actor->pClosestTouchable = act;
					break; //exit for loop
				}
			}
		}

		//lowest priority
		if (actor->pClosestTouchable == null)
			actor->pClosestTouchable = pLowPrioTouch;
	}

	//Stun Timer 
	if (actor->fLife > 0.0f)
	{
		if (actor->fStunTimer > 0.0f)
		{
			actor->fStunTimer -= dTime;
			//#TODO: cand a terminat stun il anunt ca a fost lovit
		}
	}
	else
	{
		actor->fStunTimer = 0.0f;
	}
	
	//verse timer
	dec_limit(actor->fVerseCooldown, dTime, 0.0f);

	//reset previous commands
	actor->m_AIcommands.Reset();
	//----------------------------------------
	//			PERCEIVE			
	//----------------------------------------
	
	//vedem daca a expirat durata behavior curent si daca da fortam un pas de decizie AI
	bool bBehaviorDurationFinished = false;
	if (actor->m_pAIcurrentState->m_arrBehaviors[actor->m_nAIcurrentBehaviorIdx].fBehaviorDuration > 0.0f)
	{
		actor->m_fAIbehaviorTimer += dTime;
		if (actor->m_fAIbehaviorTimer >= actor->m_pAIcurrentState->m_arrBehaviors[actor->m_nAIcurrentBehaviorIdx].fBehaviorDuration)
		{
 			bBehaviorDurationFinished = true;
			actor->AItimerDecision = 0.0f;
		}
	}

	//dead
	if (actor->fLife <= 0.0f)
	{
		//ca sa intre doar o singura data:
		if (actor->m_AIsensorInfo.m_AIcurrentEvent.nType != K_LVL_AI_EVENT_DEAD)
		{
			actor->m_AIsensorInfo.b_IsDead = true;
			actor->m_AIsensorInfo.m_AIcurrentEvent.Set(K_LVL_AI_EVENT_DEAD, actor->GetUID(), actor->templateActor.actorClass, actor->posHeart, -1.0f, 1.0f, actor->GetUID());
			//save in memory
			actor->m_AIsensorInfo.m_AIlastEvent = actor->m_AIsensorInfo.m_AIcurrentEvent;
			//reset targeted actor
			actor->m_AIsensorInfo.pTargetedActor = null;
			///THINK: force state decision
			CAIState* newState = actor->templateActor.AItemplate->GetHighestPriorityState(K_LVL_AI_EVENT_DEAD, &m_rand);
			SetActorAIState(actor, newState);
		}
	}
	else //process low freq sensors only if no message from realtime sensors (more important)
	{
		bool bIgnoreAIEvents = false;
		if((actor->m_pAIcurrentState != null) && (actor->m_nAIcurrentBehaviorIdx >= 0))
			bIgnoreAIEvents = actor->m_pAIcurrentState->m_arrBehaviors[actor->m_nAIcurrentBehaviorIdx].bIgnoreEvents;
		
		///HIGH FREQUENCY SENSORS
		//hit timer (used in some behaviors)
		actor->m_AIsensorInfo.fTimeSinceHit += dTime;
		//remove target overlap
		actor->m_AIsensorInfo.fTargetOverlapX = 0.0f;
		//did he get hit? reset time since hit 
		if (actor->nTookDamageFrames > 0)
			actor->m_AIsensorInfo.fTimeSinceHit = 0.0f;

		///LOW FREQUENCY SENSORS
		actor->AItimerDecision -= dTime;
		if ((actor->AItimerDecision <= 0.0f) && (!bIgnoreAIEvents) && (actor->m_AIsensorInfo.m_bEnabled))
		{
			//save previous event in memory only if not IDLE_TICK
			if(actor->m_AIsensorInfo.m_AIcurrentEvent.nType > K_LVL_AI_EVENT_IDLE_TICK)
				actor->m_AIsensorInfo.m_AIlastEvent = actor->m_AIsensorInfo.m_AIcurrentEvent;

			//check for targets or other AI events
			CActor* targetActor = GetClosestTarget(actor, actor->templateActor.foeClassFilter1, actor->templateActor.foeClassFilter2);
			if (targetActor != null)
			{
				float enemyDst = D3DXVec2Length(&(targetActor->posHeart - actor->posHeart));
				AddAIEvent(K_LVL_AI_EVENT_SEE_ENEMY, targetActor->GetUID(), targetActor->templateActor.actorClass, targetActor->posHeart, enemyDst, 1.0f, actor->GetUID());
				//#HACK: alerts the other enemies only if enemy class
				if(actor->templateActor.actorClass >= K_LVL_ACT_CLASS_HUMAN)
					AddAIEvent(K_LVL_AI_EVENT_SOUND_THREAT, targetActor->GetUID(), targetActor->templateActor.actorClass, targetActor->GetPosHeart(), 200.0f, 0.6f);
				//set target pointer
				actor->m_AIsensorInfo.pTargetedActor = targetActor;
				//vede daca face overlap
				if (actor->bbox.Intersects(&targetActor->bbox))
				{
					actor->m_AIsensorInfo.fTargetOverlapX = SIGN(actor->pos.x - targetActor->pos.x) * ((actor->bbox.vHalfSize.x + targetActor->bbox.vHalfSize.x) - fabs(actor->pos.x - targetActor->pos.x));
				}
				//daca se ating trimit si event de touch enemy, doar daca vede inamicul
				if (actor->bbox.Intersects(&targetActor->bbox))
				{
					AddAIEvent(K_LVL_AI_EVENT_TOUCH_ENEMY, targetActor->GetUID(), targetActor->templateActor.actorClass, targetActor->posHeart, enemyDst, 1.0f, actor->GetUID());
				}
				//scrie ultimul actor cu care a interactionat (nu este vital)
 				actor->m_AIsensorInfo.m_lastInteractingActorUID = targetActor->GetUID();
			}
			else
			{
				//reset targeted actor
				if (actor->m_AIsensorInfo.pTargetedActor != null)
				{
					//sterg mesaj de see enemy pt actorul curent
					DeleteAITargetedEvent(K_LVL_AI_EVENT_SEE_ENEMY, actor->GetUID());
					//Trimit mesaj de LOST_ENEMY
					AddAIEvent(K_LVL_AI_EVENT_LOST_ENEMY, 0, K_LVL_ACT_CLASS_ANY, actor->posHeart + D3DXVECTOR2(16.0f * actor->lookDirXsign, 0.0f), 16.0f, 1.0f, actor->GetUID());
					//reset targeting actor
					actor->m_AIsensorInfo.pTargetedActor = null;
				}

				//#HACK: uneori e lovit dar nu apuca sa vada inamicul si ramane blocat ca nu primeste LOST_ENEMY asa ca il trimitem acum
				if ((actor->m_AIsensorInfo.pTargetedActor == null) && (actor->m_AIsensorInfo.m_AIlastEvent.nType == K_LVL_AI_EVENT_GOT_HIT))
				{
					//put event behind him
					AddAIEvent(K_LVL_AI_EVENT_LOST_ENEMY, 0, K_LVL_ACT_CLASS_ANY, actor->posHeart - D3DXVECTOR2(16.0f * actor->lookDirXsign, 0.0f), 16.0f, 0.5f, actor->GetUID());
				}
			}

			///--- select best event ---
			CAIEvent* evt = GetMostImportantAIEvent(actor);

			if (evt != null)
			{
				actor->m_AIsensorInfo.m_AIcurrentEvent = *evt;
			}
			else
			{
				//nothing important, set idle tick
				actor->m_AIsensorInfo.m_AIcurrentEvent.Set(K_LVL_AI_EVENT_IDLE_TICK, 0, 0, D3DXVECTOR2(0.0f, 0.0f), -1.0f, 1.0f);
			}
			//----------------------------------------
			//	THINK - decide best behavior
			//----------------------------------------
			//daca nu am behavior sau daca behaviorul imi permite sa il intrerup.
			//am comentat verificarea pe behaviorDurationFinished pentru ca mesajul de IDLE_TICK ma scotea dintre behaviors care nu pot fi intrerupte. Ca sa pot intrerupe cand vreau bag un behavior IDLE
			if ((actor->m_nAIcurrentBehaviorIdx < 0) || (actor->m_pAIcurrentState->m_arrBehaviors[actor->m_nAIcurrentBehaviorIdx].bCanInterrupt) /*|| (bBehaviorDurationFinished)*/)
			{
				CAIState* newState = actor->templateActor.AItemplate->GetHighestPriorityState(actor->m_AIsensorInfo.m_AIcurrentEvent.nType, &m_rand);
				
				//daca vechea stare a fost setata de acelasi mesaj ca si acum si nu are prioritate mai mica nu ar mai trebui setata alta stare ci cel mult dat restart la starea curenta
				if ((newState != null) && (actor->m_AIsensorInfo.m_AIlastEvent.nType == actor->m_AIsensorInfo.m_AIcurrentEvent.nType) && (newState->nPriority == actor->m_pAIcurrentState->nPriority))
				{
					//#MAYBE: reset current behavior if it's the same state?
				}
				else
				{
					//if (newState != null)
					//	DebugPrintW(L"evttype:%d set_state: %s\n", actor->m_AIsensorInfo.m_AIcurrentEvent.nType, newState->name.text);

					//state may also be null when no state is associated with an event
					SetActorAIState(actor, newState);
				}
			}
		}
	}


	//------------------------------------------------------------------------------------------
	//	THINK - run behavior - realtime	- proceseaza AIsensors si scrie doar in AIcommands
	//------------------------------------------------------------------------------------------
	bool bBehaviorFinished = false;

	bool bSkipAI = false;
	if ((actor->m_pAIcurrentState == null) || (actor->m_nAIcurrentBehaviorIdx < 0) || (actor->fStunTimer > 0.0f))
		bSkipAI = true;

	if (bBehaviorDurationFinished)
	{
		bBehaviorFinished = true;
		bSkipAI = true;
	}
	//simple way to check if it's time to decide
	bool bTimeToDecide = (actor->AItimerDecision <= 0.0f);

	if (!bSkipAI) //daca nu am skip AI procesez switch-ul
	{
		switch (actor->m_pAIcurrentState->m_arrBehaviors[actor->m_nAIcurrentBehaviorIdx].nType)
		{
			//------------------------------------------------------------------------------------------
			//	this switch only progesses AIcommands for movement
			//------------------------------------------------------------------------------------------
			
			//make sure we exit immediately from these
			case AI_BEHAVIOR_TELEPORT_TO_TOUCHABLE_TARGET:
			case AI_BEHAVIOR_EMPTY:
			case AI_BEHAVIOR_PLAY_VERSE:
			case AI_BEHAVIOR_GENERATE_EFFECT:
			case AI_BEHAVIOR_SET_ANIMSET:
			case AI_BEHAVIOR_SET_CAPS:
			case AI_BEHAVIOR_SUICIDE:
			{
				bBehaviorFinished = true;
			}
			break;

			//waits animation to play out before exiting
			case AI_BEHAVIOR_SHOW_ENEMY:
			{
				//seteaza comanda de override anim cu valoarea salvata in SetActorAIBehavior din params behavior
				actor->m_AIcommands.eOverrideAnim = (EActorAnims)actor->AIvar1;
				//conditii final (sfarsit animatie)
				if (actor->sprite.animStatus == ANIM_STATUS_FRAMELOCK)
					bBehaviorFinished = true;
			}
			break;

			//IN_LIMBO se termina cand se revine din hidden room
			case AI_BEHAVIOR_IN_LIMBO:
			{
				//set transparency to max
				actor->m_AIcommands.nColor = D3DCOLOR_COLORALPHA(actor->color_ini, 0.0f);
				//exit state when exiting hidden room
				if (!m_bInsideHiddenRoom)
				{
					bBehaviorFinished = true;
					//dupa behavior finished se reseteaza comenzile AI asa ca setam direct culoarea
					actor->color = actor->color_ini;
				}
			}
			break;
			//acest AI e folosit la teleportarea intre usi in multiplayer si singleplayer ca sa se astepte playerii intre ei (timer si time slowdown)
			case AI_BEHAVIOR_PLAYER_TEAM_TELEPORT:
			{
				CController* pController = UTGetCtrlrMgr().GetControllerByInstanceID(actor->nControllerInstanceID);
				if (pController == null) //controller not set or removed, skipping AI
				{
					break;
				}
				//daca apasa pe UP cand sunt in teleporter cer teleportare fortata (doar cand nu sunt in camera ascunsa)
				/*
				if ((pController->sCommands.keyState[K_CM_COMMAND_UP] == K_CM_BUTSTATE_JUSTPRESSED) && (!m_bInsideHiddenRoom))
				{
					//can request teleportation by itself only on local coop
					if(!UTGetAppClass().IsGameNetworked())
						m_bTeleportRequested = true;
				}
				*/
				//daca apas alt buton iese din teleport
				/*
				if ((pController->sCommands.keyState[K_CM_COMMAND_DOWN] == K_CM_BUTSTATE_JUSTPRESSED) && (m_nTeleportSlots < m_nPlayersActive))
				{
					//termina behavior
					bBehaviorFinished = true;
					//trimite info de deschidere usa la teleportor (timer)
					if (m_pTeleportSource != null)
					{
						if (m_pTeleportSource->varAIparams.GetVariantByName(L"b_DontChangeFrames")->m_asINT32 == 0)
						{
							m_pTeleportSource->AItimer1 = 1.0f;
						}
					}
				}
				*/
				//daca apesi st/dr se intoarce cu fatza in directia respectiva
				bool bPressedRight = (pController->GetAxisVal(K_CM_COMMAND_MOVE_X) > 0.0f);
				bool bPressedLeft = (pController->GetAxisVal(K_CM_COMMAND_MOVE_X) < 0.0f);
				//daca apasa ambele butoane nu se misca
				if (bPressedLeft && bPressedRight)
					bPressedLeft = bPressedRight = false;

				if (!bPressedLeft && bPressedRight)
				{
					actor->m_AIcommands.bThrustX = false;
					actor->m_AIcommands.nLookDirX = 1;
					actor->m_AIcommands.nMoveDirX = 1;
				}
				if (bPressedLeft && !bPressedRight)
				{
					actor->m_AIcommands.bThrustX = false;
					actor->m_AIcommands.nLookDirX = -1;
					actor->m_AIcommands.nMoveDirX = -1;
				}

				//iese din behavior dupa ce au fost teleportati
				if (m_bTeleportActivated)
				{
					bBehaviorFinished = true;
				}

				if (bBehaviorFinished)
				{
					//scad numarul playerilor teleportati
					m_nTeleportSlots--;
					if (m_nTeleportSlots <= 0)
					{
						m_pTeleportSource = null;
						m_nTeleportSlots = 0;
						//m_bTeleportRequested = false;
						m_bTeleportActivated = false;
					}
				}
				//make sure it isn't invisible
				actor->m_AIcommands.nColor = 0x00ffffff;
			}
			break;
			case AI_BEHAVIOR_PLAYER_CONTROL:
			{
				CController* pController = UTGetCtrlrMgr().GetControllerByInstanceID(actor->nControllerInstanceID);
				//controller not set or removed, skipping AI
				if ((pController == null) || (pController->nFlags & K_CM_CTRLR_FLAG_PAUSED) || (actor->bSuspendInput))
				{
					//HitActor(actor, -1.0f, 0, 100, K_LVL_ACT_CLASS_TRAP);
					break;
				}
				//if suspended or other don't process input
				if ((actor->nSuspendedFlags != K_LVL_SUSPENDFLAG_NONE) || (m_levelState != K_LVL_STATE_PLAYING))
					break;

				//switch to Strategic Ability selection
				if (pController->sCommands.keyState[K_CM_COMMAND_STRATEGIC_MENU] == K_CM_BUTSTATE_JUSTPRESSED)
				{
					int nStatIdx = K_LVL_STATS_PL1_STRATEGIC_POINTS + actor->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT;

					//set icon for long duration
					actor->SetIcon(K_LVL_ACT_ICON_THINKING, 120.0f);
					//aici se seteaza pleayerselStrategic pe cea mai mare optiune
					//float fPoints = m_arrStats[nStatIdx] / 1000.0f;
					//set selection on first valid
					int nSel = 0;
					while ((m_arrStrategicAbilities[actor->nPlayerOrdinal][nSel] < 0) && (nSel < K_LVL_MAX_STRATEGIC_POINTS))
						nSel++;

					m_arrPlayerSelStrategic[actor->nPlayerOrdinal] = nSel;
					m_interfaceIGM.SetStrategicSelection(actor->nPlayerOrdinal, nSel);
					//play a sound on opening the interface
					//SND_PLAY(SNDIDX_CLICK_DENIED);
				}

				D3DXVECTOR2 vMoveDir = pController->GetDoubleAxisVectorN(K_CM_COMMAND_MOVE_X, K_CM_COMMAND_MOVE_Y);
				if (D3DXVec2LengthSq(&vMoveDir) > 0.0f)
				{
					actor->m_AIcommands.bThrust = true;
					actor->m_AIcommands.vMoveDir = vMoveDir;
					actor->m_AIcommands.bRunning = true;
				}

				//reset roll status
				/*
				if (actor->nRolling == K_STATE_FINISHED)
				{
					//reset only when thrustX off
					if ((!bPressedLeft) && (!bPressedRight))
						actor->nRolling = K_STATE_READY;
				}
				*/

				//climb ladders and interact
				/* //#DMC: commented 13 oct 2020
				if (pController->sCommands.keyState[K_CM_COMMAND_UP] != K_CM_BUTSTATE_NOTPRESSED)
				{
					if (actor->bOnLadder)
					{
						actor->m_AIcommands.nMoveDirY = -1;
						//actor->m_AIcommands.bThrustY = true;
					}
					else
					{
						actor->m_AIcommands.bClimb = true;
						actor->m_AIcommands.nMoveDirY = -1;
					}
				}
				//down (crouch, cover, climb down)
				else if (pController->sCommands.keyState[K_CM_COMMAND_DOWN] != K_CM_BUTSTATE_NOTPRESSED)
				{
					if (actor->bOnLadder)
					{
						actor->m_AIcommands.nMoveDirY = 1;
						//actor->m_AIcommands.bThrustY = true;
					}
					else
					{
						actor->m_AIcommands.bClimb = true;
						actor->m_AIcommands.nMoveDirY = 1;
						//setez si comanda de crouch
						actor->m_AIcommands.bCrouched = true;
					}
				}
				//touch
				actor->m_AIcommands.nInteractKeyState = pController->sCommands.keyState[K_CM_COMMAND_UP];
				*/
				actor->m_AIcommands.nInteractKeyState = K_CM_BUTSTATE_NOTPRESSED;
				//FIRE SHOOT
				if (pController->sCommands.bKeyDown[K_CM_COMMAND_FIRE1])
				{
					actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_SHOOTING;
				}
				else if (pController->sCommands.keyState[K_CM_COMMAND_RELOAD] == K_CM_BUTSTATE_JUSTPRESSED)
				{
					actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_RELOADING;
				}
				else if (pController->sCommands.bKeyDown[K_CM_COMMAND_FIRE2])
				{
					actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_SHOOTING_ALT;
				}
				//lets you use MELEE while holding fire or reloading
				if (pController->sCommands.keyState[K_CM_COMMAND_MELEE] == K_CM_BUTSTATE_JUSTPRESSED)
				{
					actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_MELEE;
				}
				//let you USE GEAR while holding FIRE (only if you have GEAR)
				if ((pController->sCommands.keyState[K_CM_COMMAND_USE_GEAR] == K_CM_BUTSTATE_JUSTPRESSED) && (actor->pSelectedWeapon[K_LVL_ACT_WEAPON_GEAR]->status != K_LVL_WPN_STATUS_UNKNOWN))
				{
					actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_USING_GEAR;
				}
				//RELOAD ON SHOOT - overwrites previous commands
				if ((pController->sCommands.keyState[K_CM_COMMAND_FIRE1] == K_CM_BUTSTATE_JUSTPRESSED) && (actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->ammoLeft == 0))
				{
					actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_RELOADING;
				}

				//when selecting strategic ability only crouch
				if (m_arrPlayerSelStrategic[actor->nPlayerOrdinal] >= 0)
				{
					actor->m_AIcommands.ResetMoveCommands();
					if (actor->collisionFlags & K_DIRFLAG_DOWN)
					{
						actor->m_AIcommands.bCrouched = true;
					}
					break;
				}

			}
			break;
			
			case AI_BEHAVIOR_BARREL_EXPLODING:
			{
				//when moving it attracts attention
				if ((m_Timers.Tick(100.0f)) && (fabs(actor->speed.x + actor->vSpeedImpulse.x) > 10.0f))
				{
					AddAIEvent(K_LVL_AI_EVENT_SOUND_THREAT, actor->GetUID(), actor->templateActor.actorClass, actor->GetPosHeart(), 32.0f, 0.5f);
				}

				if (actor->AIvarBool1) //can it burn?
				{
					//when it catches fire it burns 'till the end
					if (actor->cDamageOverTime.eType == CDamageOverTime::K_LVL_DoT_FIRE)
					{
						actor->cDamageOverTime.fDuration = actor->cDamageOverTime.fDuration_ini;
					}
					//daca are viata mai mica de 90% din viata initiala in mai putin de o secunda explodeaza
					if ((actor->fLife <= actor->templateActor.fLife * 0.9f) && (actor->AIsubState == 0))
					{
						//starts to shake
						actor->AIsubState = 1; 
						actor->AItimer1 = 2.0f;
					}

					//explodes after a while
					if (actor->AIsubState == 1)
					{
						//when flaming it takes 10% of life
						actor->fLife -= dTime * actor->templateActor.fLife * 0.5f;
						if (actor->fLife > 0.0f) //pana sa moara genereaza particule de foc si vibreaza
						{
							//generate particles too
							if (m_Timers.Tick(40.0f))
							{
								D3DXVECTOR2 ppos(actor->bbox.vMin.x + randfloat(actor->bbox.vSize.x), actor->bbox.vMin.y + randfloat(actor->bbox.vSize.y));
								g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRESPARK1, true, randint(2), &ppos, NULL, &D3DXVECTOR2(randfloatsgn(1.0f), -10.0f - randfloat(5.0f)), 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.5f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
								ppos = D3DXVECTOR2(actor->bbox.vMin.x + randfloat(actor->bbox.vSize.x), actor->bbox.vMin.y + randfloat(actor->bbox.vSize.y));
								g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRESPARK2, true, 0, &ppos, NULL, &D3DXVECTOR2(randfloatsgn(1.0f), -10.0f - randfloat(5.0f)), 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.5f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
							}

							actor->m_AIcommands.bThrustX = true;
							actor->m_AIcommands.nMoveDirX = m_rand.RandSign();
							actor->m_AIcommands.nLookDirX = 1;
						}
					}
				}
			}
			break;
			case AI_BEHAVIOR_HOSTAGE:
			{
				//always set crouched command if actor can crouch
				if (actor->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_CAN_CROUCH)
					actor->m_AIcommands.bCrouched = true;
				//can he follow targets? does it only once
				if (actor->AIvarBool1)
				{
					if (actor->m_AIsensorInfo.pTargetedActor != null)
					{
						//play the verse only once
						if (actor->AIsubState == 0)
						{
							PlayActorSoundVerse(actor, K_LVL_ACT_VERSE_TAUNT);
						}

						D3DXVECTOR2 vDelta = actor->m_AIsensorInfo.pTargetedActor->GetPosHeart() - actor->GetPosHeart();
						float fDist = fabs(vDelta.x);
						float fDistMin = max(K_TILE_SIZE, actor->templateActor.distAttackMin);
						//see if target is already too close
						if (fDist <= fDistMin)
						{
							actor->AIvarBool1 = false;
							break;
						}

						actor->AIsubState = 1; //followed target
						actor->m_AIcommands.bCrouched = false;
						actor->m_AIcommands.nIconType = K_LVL_ACT_ICON_SURPRISE;
						actor->m_AIcommands.fIconDuration = 0.5f;
						//run to target
						actor->m_AIcommands.bThrustX = true;
						actor->m_AIcommands.nLookDirX = SIGN(vDelta.x);
						actor->m_AIcommands.bRunning = true;
						//gets too close
						bool bHasLateralCollisions = ((actor->collisionFlags & (K_DIRFLAG_RIGHT | K_DIRFLAG_LEFT)) != 0);
						if ((fDist <= fDistMin) || (bHasLateralCollisions))
						{
							actor->AIvarBool1 = false;
						}
					}
					else
					{
						if (actor->AIsubState == 1) //already followed but lost him
						{
							actor->AIvarBool1 = false;
						}
					}
				}
			}
			break;
			case AI_BEHAVIOR_IDLE_CROUCHED:
			{
				actor->m_AIcommands.bCrouched = true;
				//handle fade out duration
				if ((actor->m_pAIcurrentState->m_arrBehaviors[actor->m_nAIcurrentBehaviorIdx].fBehaviorDuration > 0.0f) && (actor->AIfvar1 > 0.0f))
				{
					float fLeftTime = actor->m_pAIcurrentState->m_arrBehaviors[actor->m_nAIcurrentBehaviorIdx].fBehaviorDuration - actor->m_fAIbehaviorTimer;
					if (fLeftTime <= actor->AIfvar1)
					{
						//setam comanda de culoare
						actor->m_AIcommands.nColor = D3DCOLOR_COLORALPHA(actor->color_ini, fLeftTime / actor->AIfvar1);
					}
				}
			}
			break;
			case AI_BEHAVIOR_PLAY_ANIM:
			{
				//playerii pot schimba directia si pe play anim
				if (actor->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER)
				{
					CController* pController = UTGetCtrlrMgr().GetControllerByInstanceID(actor->nControllerInstanceID);
					if (pController != null)
					{
						//daca apesi st/dr se intoarce cu fatza in directia respectiva
						bool bPressedRight = (pController->GetAxisVal(K_CM_COMMAND_MOVE_X) > 0.0f);
						bool bPressedLeft = (pController->GetAxisVal(K_CM_COMMAND_MOVE_X) < 0.0f);
						//daca apasa ambele butoane nu se misca
						if (bPressedLeft && bPressedRight)
							bPressedLeft = bPressedRight = false;

						if (!bPressedLeft && bPressedRight)
						{
							actor->m_AIcommands.bThrustX = false;
							actor->m_AIcommands.nLookDirX = 1;
							actor->m_AIcommands.nMoveDirX = 1;
						}
						if (bPressedLeft && !bPressedRight)
						{
							actor->m_AIcommands.bThrustX = false;
							actor->m_AIcommands.nLookDirX = -1;
							actor->m_AIcommands.nMoveDirX = -1;
						}
					}
				}
				//seteaza comanda de override anim cu valoarea salvata in SetActorAIBehavior din params behavior
				actor->m_AIcommands.eOverrideAnim = (EActorAnims)actor->AIvar1;
				//conditii final (sfarsit animatie)
				if (actor->sprite.animStatus == ANIM_STATUS_FRAMELOCK)
					bBehaviorFinished = true;
				//modificare alpha daca e setat duration
				if (actor->m_pAIcurrentState->m_arrBehaviors[actor->m_nAIcurrentBehaviorIdx].fBehaviorDuration > 0.0f)
				{
					float fPerc = actor->m_fAIbehaviorTimer / actor->m_pAIcurrentState->m_arrBehaviors[actor->m_nAIcurrentBehaviorIdx].fBehaviorDuration;
					//setam comanda de culoare
					actor->m_AIcommands.nColor = D3DCOLOR_COLORALPHA(actor->color_ini, (1.0f - fPerc) * actor->AIfvar2 + fPerc * actor->AIfvar1);
				}
			}
			break;
			

			case AI_BEHAVIOR_SURPRISED:
			{
				if (actor->AIsubState == 0)
				{
					actor->AIsubState = 1;
					actor->m_AIcommands.nIconType = K_LVL_ACT_ICON_SURPRISE;
					actor->m_AIcommands.fIconDuration = 0.5f + actor->AItimer1;
					
					//#PERK: INTIMIDATING - surprised enemies inflict lower damage
					if (actor->m_AIsensorInfo.pTargetedActor != null)
					{
						CActor *pPlayer = GetPlayerByUID(actor->m_AIsensorInfo.pTargetedActor->GetUID());
						if ((pPlayer != null) && (g_playerSelScr.IsPerkEnabled(pPlayer->nPlayerOrdinal, &shPerk_INTIMIDATING)))
						{
							SetActorDoT(actor, CDamageOverTime::K_LVL_DoT_INTIMIDATED, 2.0f, 0.0f, K_LVL_ACT_CLASS_PLAYER, K_LVL_ACT_CLASS_HUMAN, pPlayer->GetUID());
							//overwrite previous
							actor->m_AIcommands.nIconType = K_LVL_ACT_ICON_SCARED;
							actor->m_AIcommands.fIconDuration = 2.0f;
						}
					}
				}

				actor->AItimer1 -= dTime;
				if (actor->AItimer1 <= 0.0f)
				{
					bBehaviorFinished = true;
				}
			}
			break;

			case AI_BEHAVIOR_WAIT:
			{
				//keep old crouch state
				actor->m_AIcommands.bCrouched = actor->bCrouched;
			}
			break;
			case AI_BEHAVIOR_RUN_SCRIPT:					   
			{
				//se termina behavior-ul cand s-a terminat de rulat scriptul
				if (actor->AIvar1 == 1) //wait script end
				{
					if (actor->nRunningScriptUID == 0)
						bBehaviorFinished = true;
				}
				else
				{
					bBehaviorFinished = true;
				}
			}
			break;
			case AI_BEHAVIOR_DEAD:
			{
				//get inherited death command from other states
				CVariantComplex* cvdeath = actor->varAIparams.GetVariantByName(L"nDeathCommand");

				if (cvdeath->m_type != CVariantComplex::K_ARGTYPE_NONE)
				{
					actor->m_AIcommands.nDeathCommand = (EActorDeathCommand)cvdeath->m_asINT32;
					//delete the death value after saving it to local var
					actor->varAIparams.DeleteVar(L"nDeathCommand");
				}

				///--- enforce death commands ---
				if (actor->m_AIcommands.nDeathCommand == K_LVL_ACT_DEATHCMD_NONE)
				{
					//daca nu am animatie de dead face direct splat daca poate (sau daca am primit param de bSplat din Hit Actor)
					if ((actor->templateActor.animIDs[K_LVL_ACT_ANIM_DIE][0] == -1) || (actor->varAIparams.GetVariantByName(L"bSplat")->m_asBool))
					{
						actor->m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_SPLAT;
					}
				}
				//cauta params particulari de AI setati din Hit Actor
				CVariantComplex* cvc = actor->varAIparams.GetVariantByName(L"nExplode");
				if (cvc->m_type != CVariantComplex::K_ARGTYPE_NONE)
				{
					//comanda splat on explode daca e clasa care trebuie
					if ((actor->templateActor.actorClass == K_LVL_ACT_CLASS_HUMAN) || (actor->templateActor.actorClass == K_LVL_ACT_CLASS_HOSTAGE))
						actor->m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_SPLAT;
					//get explo class
					UINT32 unExploUID = actor->GetUID();
					if (actor->varAIparams.GetVariantByName(L"bUseDamagerUID")->m_asBool)
						unExploUID = actor->nLastDamageTakenFromUID;
					//generate explo
					AddProp_Explo(cvc->m_asUINT32, actor->posHeart, unExploUID, K_LVL_ACT_CLASS_EXPLOSION, D3DXVECTOR2(0.0f, 0.0f), &actor->bbox);

					//decal explo mark
					AddDecal(K_LVL_DECAL_LAYER_BACKWALLS, actor->posHeart, ANM_ACTIVES_SPR_DECAL_EXPLOMARKS, randint(3), 0xffffffff);
				}

				///- when the player dies -
				if (actor->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER)
				{
					//actor->m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_RESET_TO_ZERO;
					//actor->varAIparams.SetNamedVarINT32(L"nDeathCommand", K_LVL_ACT_DEATHCMD_RESET_TO_ZERO);
					//#HACK: death timer - waits for the timer before executing the state, only for players
					//press fire to reset timer
					CController* pController = UTGetCtrlrMgr().GetControllerByInstanceID(actor->nControllerInstanceID);
					//daca apesi fire dupa o secunda scursa nu mai asteapta timerul
					bool bContinue = false;
					if ((pController != null) && (actor->AItimer1 < K_LVL_PLAYER_DEATH_TIMER - 1.0f) && 
						(m_arrStats[K_LVL_STATS_PL1_LIVES + actor->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT] > 0))
					{
						if ((pController->sCommands.keyState[K_CM_COMMAND_FIRE1] == K_CM_BUTSTATE_JUSTRELEASED) ||
							(pController->sCommands.keyState[K_CM_COMMAND_JUMP] == K_CM_BUTSTATE_JUSTRELEASED))
						{
							actor->AItimer1 = 0.0f;
							//continue only on keypress
							//bContinue = true;
						}
					}

					if ((actor->AItimer1 > 0.0f) && (m_levelState == K_LVL_STATE_PLAYING))
					{
						actor->AItimer1 -= dTime;
						break;
					}

					//continue if we have enough lives
					if (m_arrStats[K_LVL_STATS_PL1_LIVES + actor->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT] > 0)
						bContinue = true;
					//setam clasa pasiva ca sa putem sa distrugem cadavrul
					actor->templateActor.actorClass = K_LVL_ACT_CLASS_HUMAN;

					//daca avem breaching charges aruncate in nivel le dezalocam
					ReleaseBullet(K_LVL_BULLET_BREACHING_CHARGE, actor->GetUID());

					//release camera
					pPlayerActor[actor->nPlayerOrdinal] = null;
					if (bContinue)
					{
						m_arrPlayerSelHotJoin[actor->nPlayerOrdinal] = (int)g_playerSelScr.m_arrPlayers[actor->nPlayerOrdinal].eType;
					}
					else
					{
						m_arrPlayerSelHotJoin[actor->nPlayerOrdinal] = -1; //remove old selection so it doesn't show the hot join icon
					}

					m_nPlayers--;
					m_nPlayersActive--;  

					m_interfaceIGM.Init(&m_sprInterface, pPlayerActor[0], pPlayerActor[1]);
					m_interfaceIGM.SetHotJoinSelection(actor->nPlayerOrdinal, -1);

					//on local play check if other player is suspended and move camera on him
					if ((!UTGetAppClass().IsGameNetworked()) && (!bContinue))
					{
						for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
						{
							if ((pPlayerActor[kk] != null) && (pPlayerActor[kk]->nSuspendedFlags & K_LVL_SUSPENDFLAG_OUTSIDE_SCREEN))
							{
								m_vCamPosDefault = pPlayerActor[kk]->GetPosHeart();
								break;
							}
						}
					}
				}
				else //splat timer - splat corpse if timer is set
				{
					if ((actor->templateActor.eMaterial == K_LVL_MATERIAL_FLESH) && (actor->AItimer1 > 0.0f))
					{
						actor->AItimer1 -= dTime;
						if (actor->AItimer1 <= 0.0f)
						{
							actor->AItimer1 = 0.0f;
							actor->m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_SPLAT;
						}
						//#ZOMBIE: if turning into zombie show it by generating some particles
						if ((actor->AItimer1 < 2.0f) && ((actor->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_TURN_TO_ZOMBIE) != 0))
						{
							if (m_Timers.Tick(200))
							{
								if (UTGetAppClass().m_Settings.bGoreEnabled)
								{
									g_particlesMgr.GenerateBulletHitEnemy(D3DXVECTOR2(actor->posHeart.x + randfloatsgn(5.0f), actor->posHeart.y),
										D3DXVECTOR2((float)randsign(), -1.0f), K_LVL_ACT_CLASS_HUMAN, K_PART_LAYER_RT_FRONT_NRM);
								}
								else
								{
									//on gore off generate some stars
									g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_CROSS_SM, true, 0, &D3DXVECTOR2(actor->posHeart.x + randfloatsgn(8.0f), actor->posHeart.y), NULL,
										&D3DXVECTOR2(0.0f, -30.0f - randfloat(10.0f)), 0.6f, 0.7f, 0.0f, 0.0f, 0.0f, 0.2f, 0.2f, 0xff32a7fa, K_PART_LAYER_RT_FRONT_NRM);

								}
							}
						}
					}
				}
				//only flesh can splat
				if ((actor->m_AIcommands.nDeathCommand == K_LVL_ACT_DEATHCMD_SPLAT) && (actor->templateActor.eMaterial != K_LVL_MATERIAL_FLESH))
				{
					actor->m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_DEALLOCATE;
				}
				//execute script on death if no other important command issued
				if (actor->m_AIcommands.nDeathCommand == K_LVL_ACT_DEATHCMD_RUNSCRIPT)
				{
					CVariantComplex* cvc = actor->varAIparams.GetVariantByName(L"sDeathScript");
					if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
					{
						StartScript(cvc->m_strArg.text, actor);
						//clear script and death command
						actor->varAIparams.DeleteVar(L"sDeathScript");
						actor->m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_NONE;
					}
				}
			}
			break;
			default:
				break;
		}

		///update behavior timer - signal behavior finished when timer expires
		if (bBehaviorDurationFinished == true)
			bBehaviorFinished = bBehaviorDurationFinished;
	}
	else //skipping AI
	{
		//!!! keep old crouched state if short stun so it doesn't jitter when shot
		if (actor->fStunTimer < K_LVL_MIN_STUN_DIZZY_DURATION)
		{
			actor->m_AIcommands.bCrouched = actor->bCrouched;
		}
	}

	///reset decision timer - dupa THINK ca sa pot controla timer-ul direct din AI
	if (actor->AItimerDecision <= 0.0f)
	{
		actor->AItimerDecision = K_LVL_AI_DECISION_INTERVAL + m_rand.RandFloatSgn(K_LVL_AI_DECISION_INTERVAL_VARIATION);
	}
	///if behavior ended get on to next one
	if (bBehaviorFinished)
	{
		bool bShortBehavior = false;
		do
		{
			OnActorBehaviorFinished(actor, actor->GetCurrentBehavior());
			SetActorAIBehaviorIdx(actor, actor->m_nAIcurrentBehaviorIdx + 1, bShortBehavior);
		} while (bShortBehavior);
	}
	

	//----------------------------------------
	//	EXECUTE - process AI output - generalizare/executie comenzi AI
	//----------------------------------------
	///--- AI commands ---
	actor->nInteractingState = 0;


	//save old crouch state
	bool bCrouchedOldState = actor->bCrouched;
	//daca actorul se poate catara si are comanda de climb si directia verifica daca se urca pe scari
	if ((actor->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_CAN_CLIMB) && (!actor->bOnLadder))
	{
		if ((actor->m_AIcommands.bClimb) && (	//daca e pe pamant poti face climb cu orice tasta, in aer doar cu sus ca sa poti sa iti dai drumul de pe scara
				((actor->m_AIcommands.nMoveDirY != 0) && ((actor->collisionFlags & K_DIRFLAG_DOWN) != 0)) || 
				((actor->m_AIcommands.nMoveDirY == -1) && ((actor->collisionFlags & K_DIRFLAG_DOWN) == 0)) 
			))
		{
			//check actor bottom center of bbox by default
			D3DXVECTOR2 checkpos = D3DXVECTOR2(actor->bbox.vMin.x + actor->bbox.vHalfSize.x, actor->bbox.vMax.y);
			if (actor->m_AIcommands.nMoveDirY == -1)
				checkpos = actor->posHeart;

			actor->bOnLadder = false;
			for (int kk = 0; kk < tempCollBoxList.Count(); kk++)
			{
				if (((tempCollBoxList[kk]->collFlags & K_DIRFLAG_DOWN) == 0) || (tempCollBoxList[kk]->type != K_LVL_COLL_TYPE_LADDER))
					continue;

				if (tempCollBoxList[kk]->bbox.PointIn(checkpos))
				{
					actor->bOnLadder = true;
					actor->bCrouched = false;
					actor->m_AIcommands.bCrouched = false;
					actor->pCover = null;
					//centram X pe scara
					actor->pos.x = tempCollBoxList[kk]->bbox.vCenter.x;
					//stop reloading
					StopReloadingWeapon(actor->pCurrentWeapon);
					actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_IDLE;
					break; //exit for loop
				}
			}
		}
	}
	//hold still on ladder
	if (actor->bOnLadder)
	{
		actor->m_AIcommands.bThrustX = false;
		actor->m_AIcommands.nMoveDirX = 0;
	}

	//still in cover if can't roll
	if (actor->pCover != null)
	{
		//can't roll so stay still
		if ((actor->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_CAN_ROLL) == 0)
		{
			actor->m_AIcommands.bThrustX = false;
			actor->m_AIcommands.nMoveDirX = 0;
		}
		//can roll but looking in the other direction
		else
		{
			if ((actor->m_AIcommands.bThrustX) && (actor->m_AIcommands.nMoveDirX != actor->lookDirXsign))
			{
				actor->m_AIcommands.bThrustX = false;
				actor->m_AIcommands.nMoveDirX = 0;
				actor->nRolling = K_STATE_FINISHED;
			}
		}
	}

	//set crouch (not on ladder)
	if (!actor->bOnLadder)
	{
		actor->bCrouched = actor->m_AIcommands.bCrouched;
	}

	//check crouch conditions when falling or jumping
	if (fabs(actor->speed.y) > 1.0f)
		actor->bCrouched = false;

	if (actor->bCrouched)
	{
		//can he roll?
		if ((actor->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_CAN_ROLL) && 
			(actor->m_AIcommands.bThrustX == true) && (actor->nAttackStatus == K_LVL_ACT_ATTACK_IDLE))
		{
			if (actor->nRolling == K_STATE_READY)
			{
				actor->nRolling = K_STATE_EXECUTING;
			}
			else if (actor->nRolling == K_STATE_FINISHED)
			{
				actor->m_AIcommands.bThrustX = false;
				actor->m_AIcommands.nMoveDirX = 0;
			}
		}
		else //can't roll so just disable moving when crouched
		{
			actor->m_AIcommands.bThrustX = false;
			actor->m_AIcommands.nMoveDirX = 0;
		}
	}

	if ((actor->nRolling == K_STATE_EXECUTING) && (actor->m_AIcommands.bCrouched == true))
	{
		actor->m_AIcommands.bThrustX = true;
	}

	//roll mechanics removed if not crouched
	if (actor->nRolling == K_STATE_EXECUTING)
	{
		bool bCanceledByKeys = (actor->m_AIcommands.bThrustX == false) && (actor->m_AIcommands.bCrouched == false);
		if ((bCanceledByKeys) || (actor->sprite.animStatus == ANIM_STATUS_FRAMELOCK) ||
			(actor->nAttackStatus != K_LVL_ACT_ATTACK_IDLE) || 
			((actor->collisionFlags & K_DIRFLAG_DOWN) == 0))
		{
			actor->nRolling = K_STATE_FINISHED;
			//check for cover when ending the roll
			if(actor->bCrouched)
				bCrouchedOldState = false;
		}
		//roll not finished so keep crouched
		if (actor->nRolling == K_STATE_EXECUTING)
			actor->bCrouched = true;
	}
	if ((actor->nRolling == K_STATE_FINISHED) && (actor->bCrouched == false))
		actor->nRolling = K_STATE_READY;


	//look for cover when entering crouched state
	if (actor->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_CAN_COVER)
	{
		//entering crouch state
		if ((actor->bCrouched) && (!bCrouchedOldState) && (actor->pCover == null))
		{
			int defaultCheckAttackStatus = K_LVL_ACT_ATTACK_SHOOTING;
			//can crouch?
			if ((actor->nAttackStatus <= defaultCheckAttackStatus) && (!actor->bOnLadder) && (actor->collisionFlags & K_DIRFLAG_DOWN))
			{
				//look for cover, find cover
				for (int ll = 0; (ll < m_visibleList.logic_colShapesSpecial.Count()) && (actor->pCover == null); ll++)
				{
					if (m_visibleList.logic_colShapesSpecial[ll]->type != K_LVL_COLL_TYPE_COVER)
						continue;

					CAABB * coverbox = &m_visibleList.logic_colShapesSpecial.m_pData[ll]->bbox;
					if (actor->bbox.Intersects(coverbox))
					{
						actor->pCover = m_visibleList.logic_colShapesSpecial.m_pData[ll];
						//pozitionare actor in functie de centrele boxurilor
						if (actor->bbox.vCenter.x < coverbox->vCenter.x)
						{
							actor->pos.x -= actor->bbox.vCenter.x - coverbox->vMin.x + 2.0f;
							//se uita spre cover mereu chiar daca animatia este invers pt ca atunci cand iesi din cover sa fie cu fatza spre inamic
							/* actor->lookDirXsign = 1; */
						}
						else
						{
							actor->pos.x += coverbox->vMax.x - actor->bbox.vCenter.x + 2.0f;
							/* actor->lookDirXsign = -1; */
						}
					}
				}
			}
		}
		//exiting crouch state
		else if (!actor->bCrouched)
		{
			actor->pCover = null;
		}

		//daca esti in cover si te-a miscat ceva te scoate automat
		if ((actor->pCover != null) && (!actor->bbox.Intersects(&actor->pCover->bbox)))
		{
			actor->pCover = null;
		}
	}

	
	///--- Look direction ---
	//trebuie sa avem pointerul mereu setat
	assert(actor->pCurrentWeapon != null);
	///--- Shooting and reloading ---
	if (actor->pCurrentWeapon->status == K_LVL_WPN_STATUS_RELOADING)
	{
		//can't shoot until you reload on weapons with bullets clip
		if (actor->pCurrentWeapon->WeaponTemplate.nReloadUnitSize >= actor->pCurrentWeapon->WeaponTemplate.nClipSize)
		{
			//poti intrerupe reload cu urmatoarele comenzi:
			if (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_MELEE)
			{
				StopReloadingWeapon(actor->pCurrentWeapon);
				actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
			}
			else
			{
				actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_RELOADING;
			}
		}
	}

	//daca nu am foc alternativ anuleaza comanda
	if ((actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_SHOOTING_ALT) && (actor->pSelectedWeapon[K_LVL_ACT_WEAPON_SECONDARY]->status == K_LVL_WPN_STATUS_UNKNOWN))
	{
		actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_IDLE;
	}

	///--- keep players together, limits movement on couch multiplayer but not on net multiplayer
	if ((actor->GetCurrentBehavior() == AI_BEHAVIOR_PLAYER_CONTROL) && (!UTGetAppClass().IsGameNetworked()))
	{
		//increase suspended timer
		if (actor->nSuspendedFlags != K_LVL_SUSPENDFLAG_NONE)
		{
			actor->fSuspendedTimer += dTime;
			//outside the screen for too long? teleport PLAYER back to his friend
			if (actor->fSuspendedTimer >= K_LVL_LOCALCOOP_TELEPORT_WAIT)
			{
				CActor* pOther = pPlayerActor[(actor->nPlayerOrdinal + 1) % K_MAX_PLAYERS_CNT];
				if((pOther != null) && (pOther->GetCurrentBehavior() != AI_BEHAVIOR_IN_LIMBO) &&
					(pOther->nSuspendedFlags == K_LVL_SUSPENDFLAG_NONE) && (pOther->collisionFlags & K_DIRFLAG_DOWN))
				{
					D3DXVECTOR2 vTeleportPos = pOther->pos;
					if (GetBestSpawningPos(&vTeleportPos, pOther->bbox, &pOther->bbox))
					{
						actor->SetPos(vTeleportPos);
						//animate player on spawn (only if told otherwise by nAnimset=-1)
						actor->SetAnimSet(0);
						SetActorAIState(actor, L"JOIN_GAME");
						AddProp_Light(actor->GetPosHeart(), ANM_LIGHTS_SPR_POINT1, 0.5f, 0.1f, 0x8888ff00, 1.0f);
					}

				}
			}
		}
		else
			actor->fSuspendedTimer = 0.0f;
		//daca am mai multi players intra mereu sau daca playerul curent este suspended (in caz ca celalalt a murit)
		if ((m_nPlayersActive > 1) || (actor->nSuspendedFlags & K_LVL_SUSPENDFLAG_OUTSIDE_SCREEN))
		{
			RECTXYWH_F camrect = m_camLevel.GetCamWorldAABB();
			CAABB camAABB(D3DXVECTOR2(camrect.x, camrect.y), D3DXVECTOR2(camrect.Right(), camrect.Bottom()));

			camrect.Inflate(-16.0f);
			//players midpoint
			D3DXVECTOR2 avg(0.0f, 0.0f);
			int plcnt = 0;
			for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
			{
				if ((pPlayerActor[kk] != NULL) && (pPlayerActor[kk]->GetCurrentBehavior() != AI_BEHAVIOR_IN_LIMBO) && (pPlayerActor[kk]->nSuspendedFlags == K_LVL_SUSPENDFLAG_NONE))
				{
					avg += pPlayerActor[kk]->posHeart + pPlayerActor[kk]->vecCamFollowPos;
					plcnt++;
				}
			}
			if (plcnt > 0)
			{
				avg /= plcnt;
			}
			//--- gaseste zona in care ai voie sa te misti, limitare miscare multiplayer ---
			RECTXYWH_F lockrect(avg.x - camrect.w / 2.0f, avg.y - camrect.h / 2.0f, camrect.w, camrect.h);

			if (((actor->bbox.vMin.x < lockrect.x) && (actor->m_AIcommands.nMoveDirX < 0)) ||
				((actor->bbox.vMax.x > lockrect.Right()) && (actor->m_AIcommands.nMoveDirX > 0)))
			{
				actor->m_AIcommands.bThrustX = false;
			}
			if (((actor->bbox.vMin.y < lockrect.y) && (actor->m_AIcommands.nMoveDirY < 0)) ||
				((actor->bbox.vMax.y > lockrect.Bottom()) && (actor->m_AIcommands.nMoveDirY > 0)))
			{
//				actor->m_AIcommands.bThrustY = false;
			}
			//daca actorul a iesit din ecran ii da suspend
			if (camAABB.Intersects(&actor->bbox))
			{
				actor->nSuspendedFlags &= ~K_LVL_SUSPENDFLAG_OUTSIDE_SCREEN;
			}
			else
			{
				actor->nSuspendedFlags |= K_LVL_SUSPENDFLAG_OUTSIDE_SCREEN;
			}
		}
	}

	//daca arma curenta nu a terminat de tras nu o setez
	CWeapon* pNewWeapon = actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]; //defaults on primary default weapon
	if ((actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_SHOOTING) || (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_RELOADING))
		pNewWeapon = actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY];
	else if (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_SHOOTING_ALT)
		pNewWeapon = actor->pSelectedWeapon[K_LVL_ACT_WEAPON_SECONDARY];
	else if (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_USING_GEAR)
		pNewWeapon = actor->pSelectedWeapon[K_LVL_ACT_WEAPON_GEAR];
	else if (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_MELEE) 
		pNewWeapon = actor->pSelectedWeapon[K_LVL_ACT_WEAPON_MELEE];
	else if (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_BREACH)
		pNewWeapon = actor->pSelectedWeapon[K_LVL_ACT_WEAPON_BREACH];

	//daca sunt cu arma care incarca glont cu glont pot schimba si in timp ce incarca
	if ((pNewWeapon != null) && (pNewWeapon != actor->pCurrentWeapon) &&
		(actor->pCurrentWeapon->WeaponTemplate.nReloadUnitSize < actor->pCurrentWeapon->WeaponTemplate.nClipSize) && 
		(actor->nAttackStatus == K_LVL_ACT_ATTACK_RELOADING))
	{
		StopReloadingWeapon(actor->pCurrentWeapon);
		actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
	}

	//#HACK: sa poti trage cu GEAR cat timp tragi cu arma principala
	if ((actor->nAttackStatus != K_LVL_ACT_ATTACK_IDLE) && 
		(actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_USING_GEAR) &&
		(actor->pCurrentWeapon == actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]) && 
		(pNewWeapon == actor->pSelectedWeapon[K_LVL_ACT_WEAPON_GEAR]) && (pNewWeapon->status == K_LVL_WPN_STATUS_READY) && (pNewWeapon->ammoLeft > 0))
	{
		StopReloadingWeapon(actor->pCurrentWeapon);
		actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
	}

	//change weapon
	if ((pNewWeapon != null) && (actor->nAttackStatus == K_LVL_ACT_ATTACK_IDLE) && (pNewWeapon != actor->pCurrentWeapon) &&
		((actor->pCurrentWeapon->status <= K_LVL_WPN_STATUS_COOLING) || (actor->pCurrentWeapon->status == K_LVL_WPN_STATUS_NO_AMMO)) )
	{
		//raise triggers
		actor->pCurrentWeapon->SetTriggerStates(false, false);
		//stop reloading if was reloading
		StopReloadingWeapon(actor->pCurrentWeapon);
		//switch to new weapon
		actor->pCurrentWeapon = pNewWeapon;
		SetActorWeaponPerks(actor, pNewWeapon);
	}
	else
	{
		//daca schimbarea armei nu e valida anulez comanda data de AI
		if (pNewWeapon != actor->pCurrentWeapon)
			actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_IDLE;
	}
	//LADDER: daca am comandat sa trag dar arma nu poate trage de pe scara schimb comanda
	if ((actor->m_AIcommands.eAttackCommand != K_LVL_ACT_ATTACK_IDLE) && (actor->bOnLadder) && (!actor->pCurrentWeapon->WeaponTemplate.bCanShootFromLadders))
	{
		//pulls enemy down from ladders (grab'em by the pussy)
		/*
		if (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_MELEE)
		{
			for (int jj = 0; jj < m_visibleList.logic_actors_closeby.Count(); jj++)
			{
				CActor* act = m_visibleList.logic_actors_closeby.m_pData[jj];
				//daca actorul are un target (te vede) sau e de clasa gresita trece mai departe
				if ((act->templateActor.actorClass != K_LVL_ACT_CLASS_HUMAN) || (act->m_AIsensorInfo.pTargetedActor != null))
					continue;
				//daca inamicul este unde trebuie il agat si il arunc jos
				D3DXVECTOR2 vChkPos = actor->GetPosHeart();
				//verifica cu jumatate de tile mai sus
				vChkPos.y -= 16.0f;
				if (act->bbox.PointIn(vChkPos))
				{
					AddAIEvent(K_LVL_AI_EVENT_SEE_ENEMY, actor->GetUID(), actor->templateActor.actorClass, act->posHeart, 300.0f, 1.0f);
					act->fLife = 0.0f;
					act->fArmor = 0.0f;
					//il trag jos pe scara
					act->pos.x = actor->pos.x;
					float fOldPosY = act->pos.y;
					act->pos.y += 10.0f;

					actor->pos.y = fOldPosY;

					break;
				}
			}
		}
		*/
		//reset attack command
		actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_IDLE;
	}

	//verificari diverse ex. daca esti in aer si tragi cu o arma ce nu poate fi trasa din aer se intrerupe
	if ((actor->m_AIcommands.eAttackCommand >= K_LVL_ACT_ATTACK_SHOOTING) || (actor->nAttackStatus >= K_LVL_ACT_ATTACK_SHOOTING))
	{
		if (!CanShootWeapon(actor->pCurrentWeapon))
		{
			actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_IDLE;
			actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
		}
	}

	//command weapon
	if (actor->m_AIcommands.eAttackCommand >= K_LVL_ACT_ATTACK_SHOOTING)
	{
		actor->pCurrentWeapon->SetTriggerStates(true, false);
	}
	else if (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_RELOADING)
	{
		if(actor->pCurrentWeapon->WeaponTemplate.nReloadUnitSize != 0)
			actor->pCurrentWeapon->SetTriggerStates(false, true);
	}
	else
	{
		actor->pCurrentWeapon->SetTriggerStates(false, false);
	}

	///--- update weapons ---
	for (auto& weapon : actor->weapons)
	{
		UpdateWeapon(&weapon, dTime);
	}

	// anim synced weapons only shoot when anim ready
	if (actor->pCurrentWeapon->WeaponTemplate.bAnimSync)
	{
		if (actor->pCurrentWeapon->WeaponTemplate.shScript_OnFireALT.IsEmpty())
		{
			if ((actor->pCurrentWeapon->status != K_LVL_WPN_STATUS_READY) && (actor->m_AIcommands.eAttackCommand >= K_LVL_ACT_ATTACK_SHOOTING))
				actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_IDLE;
		}
		else
		{
			//daca ai script il lanseaza si daca arma e pe empty dar daca nu ai script trage doar cand e arma gata
			if ((actor->pCurrentWeapon->status == K_LVL_WPN_STATUS_NO_AMMO) && (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_SHOOTING_ALT))
			{
				actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_SHOOTING_ALT;
			}
			else if ((actor->pCurrentWeapon->status != K_LVL_WPN_STATUS_READY) && (actor->m_AIcommands.eAttackCommand >= K_LVL_ACT_ATTACK_SHOOTING))
			{
				actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_IDLE;
			}
		}
	}
	else //lightweight weapons (non sync), as soon as weapon is ready reset the status
	{
		//check before ShootWeapon
		if (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_IDLE)
		{
			if(actor->pCurrentWeapon->status != K_LVL_WPN_STATUS_COOLING)
				actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
		}
	}


	//daca are laser sight o activeaza acum, o singura data cand se da comanda de shoot
	if ((actor->pCurrentWeapon->WeaponTemplate.bHasLaserSight) && (actor->nAttackStatus != actor->m_AIcommands.eAttackCommand) && (actor->m_AIcommands.eAttackCommand >= K_LVL_ACT_ATTACK_SHOOTING))
	{
		actor->pCurrentWeapon->bPaintLaserSight = true;
	}

	///--- set actor attack status from command
	if(actor->m_AIcommands.eAttackCommand != K_LVL_ACT_ATTACK_IDLE)
		actor->nAttackStatus = actor->m_AIcommands.eAttackCommand;

	//COVER: exit from crouch if forced by weapon
	if ((actor->pCover != null) && (actor->bCrouched == true) && (!actor->pCurrentWeapon->WeaponTemplate.bCanShootFromCover))
	{
		if (actor->nAttackStatus >= K_LVL_ACT_ATTACK_SHOOTING)
		{
			bool bWillRunScript = false;
			//shooting weapon that has shoot script
			if (actor->pCurrentWeapon->WeaponTemplate.shScript_OnFire.IsSet())
				bWillRunScript = true;
			//Shooting ALT fire and primary weapon ALTfire script is set? then run it.
			else if ((actor->nAttackStatus == K_LVL_ACT_ATTACK_SHOOTING_ALT) && (actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->WeaponTemplate.shScript_OnFireALT.IsSet()))
				bWillRunScript = true;

			//#HACK: scripts can always be run from cover
			if(bWillRunScript == false)
			{
				actor->bCrouched = false;
			}
		}
	}
	//daca arma curenta nu poate trage din crouch scot crouch
	if ((actor->bCrouched == true) && (!actor->pCurrentWeapon->WeaponTemplate.bCanShootFromCrouch))
	{
		if (actor->nAttackStatus >= K_LVL_ACT_ATTACK_SHOOTING)
			actor->bCrouched = false;
	}

	//nu se misca in timp ce trage cu arme care te opresc din mers
	if (actor->pCurrentWeapon->WeaponTemplate.fShooterSpeedSlowingPercent >= 1.0f)
	{
		if ((actor->m_AIcommands.eAttackCommand != K_LVL_ACT_ATTACK_IDLE) || (actor->nAttackStatus != K_LVL_ACT_ATTACK_IDLE))
		{
			actor->m_AIcommands.bThrustX = false;
			actor->m_AIcommands.nMoveDirX = 0;
		}
	}

	//#HACK: nu se misca pe reload daca nu are animatiile necesare (pe personajele fara animatii compuse sigur nu am reload while moving)
	if ((!actor->templateActor.bComposedAnimation) && (actor->nAttackStatus == K_LVL_ACT_ATTACK_RELOADING))
	{
		actor->m_AIcommands.bThrustX = false;
		actor->m_AIcommands.nMoveDirX = 0;
		actor->m_AIcommands.nLookDirX = 0;
	}

	//#TODO: starile playerului ar trebui separate in stare arma si index arma ca sa nu mai verific mai jos cu >= SHOOTING
	///--- setam directie look in fn de comanda AI inainte sa tragem cu arma ---
	if (actor->m_AIcommands.nLookDirX != 0)
	{
		actor->lookDirXsign = actor->m_AIcommands.nLookDirX;
		//daca primesc comanda de lookDir setez si unghiul. Altfel unghiul ramane cel setat in LoadLevel sau se schimba prin alta comanda de schimbare unghi
		if (actor->lookDirXsign == 1)
			actor->SetAngle(0.0f);
		else
			actor->SetAngle(PI);
	}

	///--- set bbox and cover when crouched or dead ---	 
	actor->UpdateBBoxAndPoints();

	///--- find and save last safe pos for respawn ---
	/*
	if (((actor->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER) && (actor->GetCurrentBehavior() == AI_BEHAVIOR_PLAYER_CONTROL) && 
		(actor->collisionFlags & K_DIRFLAG_DOWN) != 0) &&
		(actor->standOnBox != null) && (actor->fLife > 0.0f))
	{
		if (((actor->standOnBox->ubFlags & K_LVL_COLLFLAG_SOLID) != 0) && (actor->nPlayerOrdinal >= 0) && (actor->nPlayerOrdinal < K_MAX_PLAYERS_CNT))
			m_arrPlayerLastSafePos[actor->nPlayerOrdinal] = actor->pos;
		//don't go under the collision box
		if((actor->standOnBox != null) && (actor->pos.y > actor->standOnBox->bbox.vMin.y))
			m_arrPlayerLastSafePos[actor->nPlayerOrdinal].y = actor->standOnBox->bbox.vMin.y;
	}
	*/

	//final shoot precheck
	bool bRunScriptOnEmpty = false;
	int nWeaponShots = 0;
	//limitari stari atac (revenire in starea de IDLE) si movement
	if (actor->nAttackStatus >= K_LVL_ACT_ATTACK_SHOOTING)
	{
		UINT32 dwShootScriptUID = 0;
		//shooting weapon that has shoot script
		if (actor->pCurrentWeapon->WeaponTemplate.shScript_OnFire.IsSet())
		{
			dwShootScriptUID = actor->pCurrentWeapon->WeaponTemplate.shScript_OnFire.getHash();
		}
		//Shooting ALT fire and primary weapon ALTfire script is set? then run it.
		else if ((actor->nAttackStatus == K_LVL_ACT_ATTACK_SHOOTING_ALT) && (actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->WeaponTemplate.shScript_OnFireALT.IsSet()))
		{
			dwShootScriptUID = actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->WeaponTemplate.shScript_OnFireALT.getHash();
		}
		
		//no shoot script
		if (dwShootScriptUID == 0)
		{
			//#PERSONALIZARE: check animation firerate reset and other weapon info
			if (aframeFlag & K_LVL_ACTIVE_AFRAMEFLAG_RESET_FIRE_RATE)
				actor->pCurrentWeapon->fireRateTimer = 0.0f;
			
			//#PERSONALIZARE: ca sa traga si in spate uneori (AKIMBO DUAL YELD)
			//#TODO: frame-urile in care trage personajul ar trebui marcate cu hit points cu diverse flaguri (ca sa poti seta mai multe puncte de shoot)
			bool bShoot = ((aframeFlag & K_LVL_ACTIVE_AFRAMEFLAG_ACTION) != 0);
			bool bShootSymmetric = ((aframeFlag & K_LVL_ACTIVE_AFRAMEFLAG_ACTION_SYMMETRIC) != 0);

			//can we shoot?
			if (!actor->pCurrentWeapon->WeaponTemplate.bAnimSync)
			{
				D3DXVECTOR2 vShootDir;
				if (actor->m_AIcommands.vAimDir.x != 0.0f)
					vShootDir = actor->m_AIcommands.vAimDir;
				else
					vShootDir = D3DXVECTOR2(actor->lookDirXsign, 0.0f);

				//shoot without waiting for a flag (forward or back)
				if (ShootWeapon(actor->pCurrentWeapon, vShootDir))
					nWeaponShots++;

				if (bShootSymmetric)
				{
					actor->lookDirXsign = -actor->lookDirXsign;
					//actor->pCurrentWeapon->fireRateTimer = 0.0f;
					vShootDir.x *= -1.0f; //mirror shoot dir
					if (ShootWeapon(actor->pCurrentWeapon, vShootDir))
						nWeaponShots++;
					actor->lookDirXsign = -actor->lookDirXsign;
				}
			}
			else //daca e sincronizata cu animatia trage cand ajunge pe frame de action
			{
				//#HACK: ca sa poti da grenada de pe scara fara animatie am pus si conditia de bOnLadder
				bool bActionSignal = (aframeFlag & (K_LVL_ACTIVE_AFRAMEFLAG_ACTION | K_LVL_ACTIVE_AFRAMEFLAG_ACTION_SYMMETRIC));
				if ((bActionSignal) || (actor->bOnLadder))
				{
					//#HACK: double hack! sa poti da grenada de pe scara. imi trebuie animatie pt grenada de pe scara!!!
					if ((actor->bOnLadder) && (bShoot == false))
						bShoot = true;

					D3DXVECTOR2 vShootDir;
					if (actor->m_AIcommands.vAimDir.x != 0.0f)
						vShootDir = actor->m_AIcommands.vAimDir;
					else
						vShootDir = D3DXVECTOR2(actor->lookDirXsign, 0.0f);

					if (bShoot)
					{
						if (ShootWeapon(actor->pCurrentWeapon, vShootDir))
							nWeaponShots++;
					}
					if (bShootSymmetric)
					{
						//swap direction and reset fire rate
						actor->lookDirXsign = -actor->lookDirXsign;
						//actor->pCurrentWeapon->fireRateTimer = 0.0f;
						//shoot
						vShootDir.x *= -1.0f;
						if (ShootWeapon(actor->pCurrentWeapon, vShootDir))
							nWeaponShots++;
						//look back to where we were
						actor->lookDirXsign = -actor->lookDirXsign;
					}
				}
			}
		}
		else  //shoot script
		{
			if (!actor->pCurrentWeapon->WeaponTemplate.bAnimSync)
			{
				UTGetScriptManager().StartScript(dwShootScriptUID, actor->UID);
				actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
			}
			else //daca e sincronizata cu animatia trage cand ajunge pe frame de action
			{
				bool bActionSignal = (aframeFlag & (K_LVL_ACTIVE_AFRAMEFLAG_ACTION | K_LVL_ACTIVE_AFRAMEFLAG_ACTION_SYMMETRIC));
				if ((bActionSignal) || (actor->bOnLadder))
				{
					UTGetScriptManager().StartScript(dwShootScriptUID, actor->UID);
					actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
				}
			}
		}

		//DK - some weapons share the same magazine
		if ((nWeaponShots > 0) && (actor->pCurrentWeapon->WeaponTemplate.bUsesMainWeaponAmmo))
		{
			if (actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->ammoLeft > 0)
			{
				dec_limit(actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->ammoLeft, nWeaponShots, 0);
				//update the weapon in case we run out of ammo so it sets the correct status
				UpdateWeapon(actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY], 0.0f);
			}
		}

		///--- check weapon script on empty ---
		CWeapon* pWpnToCheck = actor->pCurrentWeapon;
		if (actor->pCurrentWeapon->WeaponTemplate.bUsesMainWeaponAmmo)
			pWpnToCheck = actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY];

		EnumWeaponStatus gunstat = pWpnToCheck->status;
		//light weapons
		if (!actor->pCurrentWeapon->WeaponTemplate.bAnimSync)
		{
			if ((gunstat == K_LVL_WPN_STATUS_NO_AMMO) || (gunstat == K_LVL_WPN_STATUS_BURST_END) || (gunstat == K_LVL_WPN_STATUS_JAMMED))
			{
				actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
				bRunScriptOnEmpty = true;
			}
		}
		else //synchronized weapons
		{
			//#HACK #TODO: ca sa poata trage de pe scara fara animatie
			if ((actor->sprite.animStatus == ANIM_STATUS_FRAMELOCK) || (actor->bOnLadder))
			{
				actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
				//daca nu am ammo anulez starea de shoot
				if (gunstat == K_LVL_WPN_STATUS_NO_AMMO)
				{
					actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
				}
				//run script on empty weapon or after each shot if weapon has infinite ammo
				if ((gunstat == K_LVL_WPN_STATUS_NO_AMMO) || ((pWpnToCheck->WeaponTemplate.nClipSize < 0) && (pWpnToCheck->ammoLeft <= 0)))
				{
					bRunScriptOnEmpty = true;
				}
			}
		}

		///--- launch script when weapon runs out of ammo:
		if ((bRunScriptOnEmpty) && (pWpnToCheck->WeaponTemplate.shScript_OnEmpty.IsSet()) && (pWpnToCheck->ammoLeft <= 0))
		{
			UTGetScriptManager().StartScript(actor->pCurrentWeapon->WeaponTemplate.shScript_OnEmpty.getHash(), actor->GetUID());
		}

	}

	//opreste laser sight dupa ce a tras sau daca nu trage (a fost intrerupt, nu are ammo, etc)
	if (actor->pCurrentWeapon->WeaponTemplate.bHasLaserSight)
	{
		if((actor->nAttackStatus < K_LVL_ACT_ATTACK_SHOOTING) || (nWeaponShots > 0))
			actor->pCurrentWeapon->bPaintLaserSight = false;
	}

	//resetam stari reload la finalul incarcarii sau daca nu face arma reload
	if ((actor->nAttackStatus == K_LVL_ACT_ATTACK_RELOADING) && (actor->pCurrentWeapon->status != K_LVL_WPN_STATUS_RELOADING))
	{
		actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
	}

	///--- set icon ---
	if (actor->m_AIcommands.nIconType != K_LVL_ACT_ICON_NONE)
	{
		actor->SetIcon(actor->m_AIcommands.nIconType, actor->m_AIcommands.fIconDuration);
	}
	
	///--- speed ---
	//add equipped weapon speed penalty
	float fWpnSpeedPenaltyPercent = 0.0f;
	if (actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->status != K_LVL_WPN_STATUS_UNKNOWN)
		fWpnSpeedPenaltyPercent = actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->WeaponTemplate.fSpeedPenaltyPercent;

	if (actor->m_AIcommands.bThrust)
	{
		//add speed
		float fspeed = actor->templateActor.moveMinSpeed;
		//daca alearga schimb viteza
		if (actor->m_AIcommands.bRunning)
			fspeed = actor->templateActor.moveMaxSpeed;
		//speed penalty
		fspeed -= fspeed * fWpnSpeedPenaltyPercent;

		// set final speed
		actor->speed = actor->m_AIcommands.vMoveDir * fspeed;
	}
	else
	{
		actor->speed = D3DXVECTOR2(0.0f, 0.0f);
	}


	/*
	if (actor->m_AIcommands.bThrustX)
	{
		//add speed
 		float fspeed = actor->templateActor.moveMinSpeed;
		//daca alearga schimb viteza
		if (actor->m_AIcommands.bRunning)
			fspeed = actor->templateActor.moveMaxSpeed;
		//mers cu spatele
		if ((actor->m_AIcommands.nMoveDirX != 0) && (actor->lookDirXsign != actor->m_AIcommands.nMoveDirX))
			fspeed = actor->templateActor.moveBackSpeed;
		//roll speed
		if ((actor->bCrouched) && (actor->nRolling == K_STATE_EXECUTING))
		{
			fspeed = actor->templateActor.moveMinSpeed; 
		}

		//penalizare viteza 
		fspeed -= fspeed * fWpnSpeedPenaltyPercent;
		//walk slower when shooting
		if (actor->nAttackStatus > K_LVL_ACT_ATTACK_RELOADING)
		{
			//actioneaza doar cand nu esti in aer
			if (actor->collisionFlags & K_DIRFLAG_DOWN)
			{
				//se scade din viteza procentul setat de arma influentat de inversul dexteritatii
				float fSpeedDecrease = fspeed * actor->pCurrentWeapon->WeaponTemplate.fShooterSpeedSlowingPercent;
				fspeed -= LIMIT(fSpeedDecrease, 0.0f, fspeed);
			}
		}
		//CRIPPLED DoT
		if ((actor->cDamageOverTime.eType == CDamageOverTime::K_LVL_DoT_CRIPPLED) && (actor->collisionFlags & K_DIRFLAG_DOWN))
		{
			fspeed *= 0.3f;
		}
		//daca setez directia de move o foloseste pe cea comandata altfel se misca in directia in care se uita
		if (actor->m_AIcommands.nMoveDirX != 0)
			fspeed *= actor->m_AIcommands.nMoveDirX;
		else
			fspeed *= actor->lookDirXsign;
		//setam thrust
		actor->speed.x = fspeed;
	}
	else
	{
		//daca sunt pe pamant opresc viteza
		if ((actor->collisionFlags & K_DIRFLAG_DOWN) || (actor->bOnLadder))
			actor->speed.x = 0.0f;
		//#HACK:target overlap - pushes the enemy so it doesn't overlap the players (when no thrust or collisions)
		if ((actor->templateActor.actorClass >= K_LVL_ACT_CLASS_HUMAN) && (actor->m_AIsensorInfo.pTargetedActor != null) && 
			(actor->m_AIsensorInfo.fTargetOverlapX != 0.0f) && ((actor->collisionFlags & K_DIRFLAG_LEFT_RIGHT) == 0))
		{
			float fPushForce = actor->m_AIsensorInfo.fTargetOverlapX * 15.0f;
			//humans only get pushed back if they can walk backwards (so you can arrest them or at least get close to them)
			if ((actor->templateActor.actorClass == K_LVL_ACT_CLASS_HUMAN) && (actor->templateActor.moveBackSpeed == 0.0f))
				fPushForce = 0.0f;
			//push enemy only if not already pushed
			if(fabs(actor->vSpeedImpulse.x) < fabs(fPushForce))
				actor->vSpeedImpulse.x = fPushForce;
		}
	}
	*/

	//comanda culoare
	if (actor->m_AIcommands.nColor != 0)
	{
		actor->color = actor->m_AIcommands.nColor;
	}

	//death elements (intra doar daca e declarat mort in senzor sau daca i se forteaza starea de dead)
	if ((actor->m_AIsensorInfo.b_IsDead) || (actor->GetCurrentBehavior() == AI_BEHAVIOR_DEAD))
	{
		switch (actor->m_AIcommands.nDeathCommand)
		{
			case K_LVL_ACT_DEATHCMD_RESET_TO_ZERO:
			{
				actor->fLife = 0.0f;
			}
			break;
			case K_LVL_ACT_DEATHCMD_SPLAT:
			{
				//don't explode hidden actors
				if (actor->bSkipRender)
					break;

				if (!UTGetAppClass().m_Settings.bGoreEnabled)
				{
					g_particlesMgr.GenerateEnemySoftGib(actor->pos, 0xff32a7fa, K_PART_LAYER_RT_FRONT_NRM);
				}
				else
				{
					//blood splat (sortate crescator in animatie)
					AddDecal_BloodSplat(actor->GetPosHeart(), true, actor->templateActor.actorClass);

					//SND_PLAY_POSITIONAL_RAND2(SNDIDX_BULLET_BODY_GIBBED_01, SNDIDX_BULLET_BODY_GIBBED_02, actor->GetPosHeart());
					//meat lumps
					D3DXVECTOR2 bulletSpeed;
					D3DXVec2Normalize(&bulletSpeed, &actor->vSpeedImpulse);

					CAABB genbox = actor->bbox;
					genbox.Inflate(-2.0f, -2.0f);
					if (actor->templateActor.fLife > 10.0f)
					{
						DWORD dwCol = 0xff671010;
						int nSubType = 0;
						if (actor->templateActor.actorClass == K_LVL_ACT_CLASS_ZOMBIE)
						{
							dwCol = 0xff82b600;
							nSubType = 1;
						}
						for (int ll = 0; ll < 6; ll++)
						{
							AddProp(K_LVL_PROP_MEAT, AABB_GetRandomPointInBox(genbox), &D3DXVECTOR2(randfloatsgn(50.0f) + bulletSpeed.x * 50.0f, -130.0f - randfloat(100.0f)), &g_vecGravity, nSubType);
						}
						//goes straight down to stain the floor
						AddProp(K_LVL_PROP_MEAT, actor->GetPosHeart(), &D3DXVECTOR2(200.0f, 50.0f), &g_vecGravity, nSubType);
						AddProp(K_LVL_PROP_MEAT, actor->GetPosHeart(), &D3DXVECTOR2(-200.0f, 50.0f), &g_vecGravity, nSubType);
						//human blood gibs particle
						g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_HUMAN_SPLAT_MED, true, 0, &actor->pos, NULL, NULL, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, dwCol, K_PART_LAYER_RT_FRONT_NRM);
					}
					else //small animals and stuff
					{
						for (int ll = 0; ll < 2; ll++)
						{
							AddProp(K_LVL_PROP_MEAT, AABB_GetRandomPointInBox(genbox), &D3DXVECTOR2(randfloatsgn(50.0f) + bulletSpeed.x * 50.0f, -130.0f - randfloat(100.0f)), &g_vecGravity);
						}
						g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_HUMAN_SPLAT_SMALL, true, 0, &actor->pos, NULL, NULL, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xff671010, K_PART_LAYER_RT_FRONT_NRM);
					}
				}

				// only large enemies get scared
				if (actor->templateActor.fLife > 10.0f)
				{
					//#PERK: GORE DEALER - gibs scare enemies
					CActor* pPlayer = GetPlayerByUID(actor->m_AIsensorInfo.m_lastInteractingActorUID);
					if ((pPlayer != null) && (m_rand.RandFloat(100.0f) < 50.0f) &&
						(g_playerSelScr.IsPerkEnabled(pPlayer->nPlayerOrdinal, &shPerk_GORE_DEALER)))
					{
						AddProp_Explo(hash_EXPLO_FAKE_INTIMIDATE, actor->GetPosHeart(), pPlayer->GetUID(), K_LVL_ACT_CLASS_PLAYER);
					}
				}

				//#ZOMBIE: human cadavers become zombies so spawn one here
				if ( (actor->templateActor.eMaterial == K_LVL_MATERIAL_FLESH) && 
					 ((actor->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_TURN_TO_ZOMBIE) != 0) )
				{
					//only spawn if on ground and only if turn to zombie countdown finished
					if (((actor->collisionFlags & K_DIRFLAG_DOWN) != 0) && (actor->AItimer1 <= 0.0f))
					{
						CStringHash shTemplate;
						if (actor->templateActor.actorClass == K_LVL_ACT_CLASS_HOSTAGE)
						{
							shTemplate.Init(L"ACTOR_ZOMBIE_HOSTAGE");
						}
						else
						{
							int prob = m_rand.RandInt(100);
							if (prob < 30)
								shTemplate.Init(L"ACTOR_ZOMBIE_TORSO1");
							else if (prob < 50)
								shTemplate.Init(L"ACTOR_ZOMBIE_FAST1");
							else
								shTemplate.Init(L"ACTOR_ZOMBIE_SLOW1");
						}
						//spawn them already aware
						CStringHash shState(L"AWARE");
						SpawnActor(actor->pos, shTemplate.text, actor->lookDirXsign, &shState);
					}
				}

				//players don't deallocate. They only become invisible.
				if (actor->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER)
				{
					actor->fLife = 0.0f;
					actor->bHasGravity = false;
					actor->bHasCollision = false;
					actor->bSkipRender = true;
					//reset physics
					actor->speed = D3DXVECTOR2(0.0f, 0.0f);
					actor->vSpeedImpulse = D3DXVECTOR2(0.0f, 0.0f);
					actor->vecCamFollowPos = D3DXVECTOR2(0.0f, 0.0f);
					//move invisible body back to last safe pos
					D3DXVECTOR2 vSpawnPos = m_arrPlayerLastSafePos[actor->nPlayerOrdinal];
					actor->SetPos(vSpawnPos);
					break;
				}
				//deallocate
				actor->bSetHidden = true;
				actor->bReleaseIt = true;
			}
			break;
			case K_LVL_ACT_DEATHCMD_DEALLOCATE:
			{
				//dezalocare
				actor->bSetHidden = true;
				actor->bReleaseIt = true;
			}
			break;
		}
		//remove death command after execution
		actor->m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_EMPTY;
	}
	//------------------------------------------------------------------------------------------
	//	ELEMENTE GENERICE GAMEPLAY (arme, jump, ladder etc)
	//------------------------------------------------------------------------------------------

	///--- set animations ---
	//simple animations (not composed from torso and feet)
	if (!actor->templateActor.bComposedAnimation)
	{
		if (bSkipAI) //daca am skip AI nu mai setez alta animatie pentru ca nu mai primesc comenzi din AI si imi trece pe IDLE automat (apare tremurici)
		{
			if (actor->fStunTimer >= K_LVL_MIN_STUN_DIZZY_DURATION) 
			{
				//daca am stun mai de durata ii pun animatie speciala
				SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_STUNNED);
			}
		}
		else
		{
			if (actor->m_AIsensorInfo.b_IsDead)
			{
				SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_DIE);
			}
			else if (actor->m_AIcommands.eOverrideAnim != K_LVL_ACT_ANIM_EMPTY)
			{
				//daca am override la animatie o setez pe cea din comanda
				SetActorAnimationOnce(actor, actor->m_AIcommands.eOverrideAnim);
			}
			else if (actor->nInteractingState != 0)
			{
				if (actor->nInteractingState == 1)
				{
					SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_INTERACT);
				}
				else
				{
					SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_LOCKPICK);
				}
			}
			else if (actor->bCrouched)
			{
				if (actor->nAttackStatus != K_LVL_ACT_ATTACK_SHOOTING)
				{
					if (actor->nAttackStatus == K_LVL_ACT_ATTACK_RELOADING)
					{
						SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_CROUCH_RELOAD);
					}
					else
					{
						if(!actor->nRolling == K_STATE_EXECUTING)
							SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_CROUCH);
						else
							SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_ROLL);
					}
				}
				else
				{
					SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_CROUCH_SHOOT);
				}

			}
			else if (actor->nAttackStatus == K_LVL_ACT_ATTACK_SHOOTING_ALT)
			{
				SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_SHOOT_ALT);
			}
			else if (actor->nAttackStatus == K_LVL_ACT_ATTACK_USING_GEAR)
			{
				SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_USE_GEAR);
			}
			else if (actor->nAttackStatus == K_LVL_ACT_ATTACK_MELEE)
			{
				SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_MELEE);
			}
			else
			{
				if (actor->nAttackStatus != K_LVL_ACT_ATTACK_SHOOTING)
				{
					//daca are coliziune jos sau daca nu are flag de coliziuni sau de gravitatie intra pe aici si nu pe partea cu falling
					if ((actor->collisionFlags & K_DIRFLAG_DOWN) || (!actor->bHasCollision) || (!actor->bHasGravity))
					{
						if (actor->nAttackStatus == K_LVL_ACT_ATTACK_RELOADING)
						{
							SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_RELOAD);
						}
						else
						{
							if ((actor->speed.x == 0.0f) || (actor->collisionFlags & K_DIRFLAG_RIGHT) || (actor->collisionFlags & K_DIRFLAG_LEFT))
							{
								if ((actor->m_AIsensorInfo.pTargetedActor != null) && (ActorHasAnimation(actor, K_LVL_ACT_ANIM_IDLE_GUN)))
									SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_IDLE_GUN);
								else
									SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_IDLE);
							}
							else
							{
								//daca merge cu spatele si nu trage
								if (SIGN(actor->speed.x) != actor->lookDirXsign)
								{
									//daca are target tine pistolul scos
									if((actor->m_AIsensorInfo.pTargetedActor != null) && (ActorHasAnimation(actor, K_LVL_ACT_ANIM_MOVE_BK_GUN))) 
										SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_MOVE_BK_GUN, K_LVL_ACT_ANIM_EMPTY);
									else
										SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_MOVE_BK, K_LVL_ACT_ANIM_EMPTY);
								}
								else //daca merge cu fatza
								{
									if (actor->m_AIcommands.bRunning)
									{
										//daca are target tine pistolul scos
										if ((actor->m_AIsensorInfo.pTargetedActor != null) && (ActorHasAnimation(actor, K_LVL_ACT_ANIM_MOVE_FAST_GUN)))
											SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_MOVE_FAST_GUN, K_LVL_ACT_ANIM_EMPTY, true);
										else
											SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_MOVE_FAST, K_LVL_ACT_ANIM_EMPTY, true);
									}
									else 
									{
										if ((actor->m_AIsensorInfo.pTargetedActor != null) && (ActorHasAnimation(actor, K_LVL_ACT_ANIM_MOVE_GUN)))
											SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_MOVE_GUN, K_LVL_ACT_ANIM_EMPTY, true);
										else
											SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_MOVE, K_LVL_ACT_ANIM_EMPTY, true);
									}
								}
							}
						}
					}
					else //jumping or falling or on ladder
					{
						//daca sunt pe scara si am animatie de climb
						int climbAnimIdx = actor->templateActor.animIDs[K_LVL_ACT_ANIM_CLIMB_LADDER][0];
						if ((actor->bOnLadder) && (climbAnimIdx >= 0))
						{
							SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_CLIMB_LADDER);
							//set frame by Y pos
							int aframescnt = m_sprActors.GetAFramesCnt(climbAnimIdx);
							float frameAdvPerc = (float)aframescnt / (float)K_TILE_SIZE;
							actor->sprite.currentFrame = int(actor->pos.y * frameAdvPerc) % aframescnt;
						}
						else //in midair
						{
							if (actor->speed.y < -3.0f)
								SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_JUMP);
							else if (actor->speed.y > 3.0f)
								SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_FALL);
							else //midair
								SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_JUMP_STILL);
						}
					}
				}
				else
				{
					if (!actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->WeaponTemplate.bAnimSync)
					{
						//daca am arma usoara pot trage in timp ce merg
						if ((actor->speed.x == 0.0f) || (actor->collisionFlags & K_DIRFLAG_RIGHT) || (actor->collisionFlags & K_DIRFLAG_LEFT))
						{
							//daca sta pe loc face shoot normal from idle
							SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_SHOOT);
						}
						else
						{
							//daca se misca seteaza frame corespondent din animatia de shoot while moving
							if (SIGN(actor->speed.x) != actor->lookDirXsign)
								SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_MOVE_BK_SHOOTING);
							else
								SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_MOVE_SHOOTING, K_LVL_ACT_ANIM_EMPTY, true);
						}
					}
					else //armele sincronizate cu animatia
					{
						//cand arma e gata sa traga porneste animatia iar mai sus asteapta flag de shoot ca sa traga efectiv
						if (actor->pCurrentWeapon->status == K_LVL_WPN_STATUS_READY)
						{
							SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_SHOOT);
						}
						//#TODO:daca arma nu mai are gloante seteaza alta animatie eventual
						if (actor->pCurrentWeapon->status == K_LVL_WPN_STATUS_NO_AMMO)
							SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_SHOOT);
					}
				}
			}

			//#PERSONALIZARE: scale reload nimation. scalare animatie reload in fn de durata
			if (actor->eLastAnimSet == K_LVL_ACT_ANIM_RELOAD)
			{
				int reloadAnimIdx = actor->sprite.animationIdx;
				int aframescnt = m_sprActors.GetAFramesCnt(reloadAnimIdx);
				float frameAdvPerc = actor->pCurrentWeapon->reloadTimer / actor->pCurrentWeapon->WeaponTemplate.fReloadTimePerUnit;
				int nTopFrame = (int)(aframescnt * frameAdvPerc);
				CLAMP(nTopFrame, 0, (aframescnt - 1));
				actor->sprite.currentFrame = nTopFrame;
			}

		}
	}
	else  ///--- animatiile compuse ---
	{
		if (bSkipAI)
		{
			if (actor->fStunTimer >= K_LVL_MIN_STUN_DIZZY_DURATION) 
			{
				//daca am stun mai de durata ii pun animatie speciala
				SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_STUNNED, K_LVL_ACT_ANIM_FEET_IDLE);
			}
		}
		else
		{
			EActorAnims eTopAnim = K_LVL_ACT_ANIM_IDLE;
			EActorAnims eFeetAnim = K_LVL_ACT_ANIM_NOT_SET;
			int	nFeetFrame = -1, nTopFrame = -1; //not setting frame

			if (actor->m_AIsensorInfo.b_IsDead)
			{
				eTopAnim = K_LVL_ACT_ANIM_DIE;
				eFeetAnim = K_LVL_ACT_ANIM_EMPTY;
			}
			else if (actor->m_AIcommands.eOverrideAnim != K_LVL_ACT_ANIM_EMPTY)
			{
				//daca am override la animatie o setez pe cea din comanda
				eTopAnim = actor->m_AIcommands.eOverrideAnim;
				eFeetAnim = K_LVL_ACT_ANIM_EMPTY;
			}
			else if (actor->nInteractingState != 0) 
			{
				if (actor->nInteractingState == 1)
					eTopAnim = K_LVL_ACT_ANIM_INTERACT;
				else
					eTopAnim = K_LVL_ACT_ANIM_LOCKPICK;
				//empty feet anim
				eFeetAnim = K_LVL_ACT_ANIM_EMPTY;
			}
			else //daca nu sunt cazuri speciale setez separat animatiile top si bottom
			{
				//top anim
				eTopAnim = K_LVL_ACT_ANIM_IDLE;

				if ((actor->speed.x == 0.0f) || (actor->collisionFlags & K_DIRFLAG_RIGHT) || (actor->collisionFlags & K_DIRFLAG_LEFT))
				{
					eTopAnim = K_LVL_ACT_ANIM_IDLE;
				}
				else
				{
					if (actor->m_AIcommands.bRunning)
						eTopAnim = K_LVL_ACT_ANIM_MOVE_FAST;
					else
						eTopAnim = K_LVL_ACT_ANIM_MOVE;
				}

				if (actor->nAttackStatus == K_LVL_ACT_ATTACK_SHOOTING)
				{
					eTopAnim = K_LVL_ACT_ANIM_SHOOT;
				}
				else if (actor->nAttackStatus == K_LVL_ACT_ATTACK_SHOOTING_ALT)
					eTopAnim = K_LVL_ACT_ANIM_SHOOT_ALT;
				else if (actor->nAttackStatus == K_LVL_ACT_ATTACK_USING_GEAR)
					eTopAnim = K_LVL_ACT_ANIM_USE_GEAR;
				else if (actor->nAttackStatus == K_LVL_ACT_ATTACK_RELOADING)
				{
					eTopAnim = K_LVL_ACT_ANIM_RELOAD;
				}
				else if (actor->nAttackStatus == K_LVL_ACT_ATTACK_MELEE)
				{
					eTopAnim = K_LVL_ACT_ANIM_MELEE;
					eFeetAnim = K_LVL_ACT_ANIM_EMPTY;
					//make sure we randomly loop through all animations
					actor->SetAnimSet(-1);
				}
				else if (actor->nAttackStatus == K_LVL_ACT_ATTACK_BREACH)
				{
					eTopAnim = K_LVL_ACT_ANIM_BREACH_DOOR;
					eFeetAnim = K_LVL_ACT_ANIM_EMPTY;
				}
				//special case for crouch rolls
				if ((actor->bCrouched) && (actor->nRolling == K_STATE_EXECUTING))
				{
					eTopAnim = K_LVL_ACT_ANIM_ROLL;
					eFeetAnim = K_LVL_ACT_ANIM_EMPTY;
				}


				//set feet anim if not already set
				if (eFeetAnim == K_LVL_ACT_ANIM_NOT_SET)
				{
					//daca e pe pamant sau nu ia in seama gravitatia
					if ((actor->collisionFlags & K_DIRFLAG_DOWN) || (!actor->bHasCollision) || (!actor->bHasGravity))
					{
						if (actor->bCrouched)
						{
							eFeetAnim = K_LVL_ACT_ANIM_FEET_CROUCH;
						}
						else
						{
							if ((actor->speed.x == 0.0f) || (actor->collisionFlags & K_DIRFLAG_RIGHT) || (actor->collisionFlags & K_DIRFLAG_LEFT))
							{
								eFeetAnim = K_LVL_ACT_ANIM_FEET_IDLE;
							}
							else
							{
								if (actor->m_AIcommands.bRunning)
								{
									if (SIGN(actor->speed.x) != actor->lookDirXsign)
										eFeetAnim = K_LVL_ACT_ANIM_FEET_MOVE_BACK;
									else
										eFeetAnim = K_LVL_ACT_ANIM_FEET_MOVE_FAST;
								}
								else
								{
									if (SIGN(actor->speed.x) != actor->lookDirXsign)
										eFeetAnim = K_LVL_ACT_ANIM_FEET_MOVE_BACK;
									else
										eFeetAnim = K_LVL_ACT_ANIM_FEET_MOVE;
								}
							}
						}
						//daca trage si e idle pun animatia picioarelor pe still
						if ((actor->nAttackStatus != K_LVL_ACT_ATTACK_IDLE) && (eFeetAnim == K_LVL_ACT_ANIM_FEET_IDLE))
							eFeetAnim = K_LVL_ACT_ANIM_FEET_STILL;
					}
					else
					{
						//on ladder and we have climb anim
						int climbAnimIdx = actor->templateActor.animIDs[K_LVL_ACT_ANIM_CLIMB_LADDER][0];
						if ((actor->bOnLadder) && (climbAnimIdx >= 0))
						{
							eTopAnim = K_LVL_ACT_ANIM_CLIMB_LADDER;
							eFeetAnim = K_LVL_ACT_ANIM_EMPTY;

							//regleaza frame-ul in functie de pozitia pe y
							int aframescnt = m_sprActors.GetAFramesCnt(climbAnimIdx);
							float frameAdvPerc = (float)aframescnt / (float)K_TILE_SIZE;
							nTopFrame = int(actor->pos.y * frameAdvPerc) % aframescnt;

							//ladder sounds - only when changing frames
							if (nTopFrame != actor->sprite.currentFrame)
							{
								UINT32 fflag = m_sprActors.GetAFrameFlag(climbAnimIdx, nTopFrame);
								if ((fflag & K_LVL_ACTIVE_AFRAMEFLAG_SOUND) && (fabs(actor->speed.y) > 0.0f))
								{
									//SND_PLAY_POSITIONAL_RAND2(SNDIDX_CLIMB_LADDER1, SNDIDX_CLIMB_LADDER2, actor->pos);
								}
							}
						}
						else
						{
							eFeetAnim = K_LVL_ACT_ANIM_FEET_JUMP;
							//treat jumps
							if (actor->speed.y < -10.0f)
								nFeetFrame = 0;
							else if (actor->speed.y > 10.0f)
								nFeetFrame = 2;
							else //midair
								nFeetFrame = 1;
							//top anim
							if ((eTopAnim == K_LVL_ACT_ANIM_IDLE) || (eTopAnim == K_LVL_ACT_ANIM_MOVE) || (eTopAnim == K_LVL_ACT_ANIM_MOVE_FAST))
							{
								eTopAnim = K_LVL_ACT_ANIM_JUMP;
								nTopFrame = nFeetFrame;
							}
						}
					}
				}

				//#PERSONALIZARE: daca trag cu arma (!not sync!) resetez animatia de shoot ca sa apara flacara dupa fiecare glont
				if ( ((eTopAnim == K_LVL_ACT_ANIM_SHOOT) || (eTopAnim == K_LVL_ACT_ANIM_SHOOT_ALT)) && 
					(!actor->pCurrentWeapon->WeaponTemplate.bAnimSync) && (nWeaponShots > 0))
				{
					nTopFrame = 0; //no flame
				}
			}
			//#PERSONALIZARE: daca vreau sa trag dar arma nu e gata setez pe ultimul frame de tras. Valabil pentru armele fara anim sync
			//bool bSetLastFrame = ((eTopAnim == K_LVL_ACT_ANIM_SHOOT) && (eTopAnim != actor->eLastAnimSet) && (!bWeaponShot) && (!actor->pCurrentWeapon->WeaponTemplate.bAnimSync));

			//finally set animation
			SetActorAnimationOnce(actor, eTopAnim, eFeetAnim);

			//#PERSONALIZARE: scalare animatie reload. reload animation gets played for as long as the reload sequence goes
			if (eTopAnim == K_LVL_ACT_ANIM_RELOAD)
			{
				//#TODO: sa scaleze de fapt animatia de reload tinand cont de durata fiecarui frame. Sa gaseasca lungimea animatiei si sa vada in cat timp ar trebui playata si sa o scaleze
				int reloadAnimIdx = actor->sprite.animationIdx;
				int aframescnt = m_sprActors.GetAFramesCnt(reloadAnimIdx);
				float frameAdvPerc = actor->pCurrentWeapon->reloadTimer / actor->pCurrentWeapon->WeaponTemplate.fReloadTimePerUnit;
				nTopFrame = int(aframescnt * frameAdvPerc);
				CLAMP(nTopFrame, 0, (aframescnt - 1));
			}


			//if (bSetLastFrame)
			//{
			//	actor->sprite.SetFrame(m_sprActors.GetAFramesCnt(actor->sprite.animationIdx) - 1);
			//}

			if (nTopFrame >= 0)
			{
				actor->sprite.SetFrame(nTopFrame);
			}
			if (nFeetFrame >= 0)
			{
				actor->sprite_feet.SetFrame(nFeetFrame);
			}
		}
	}
	
	///--- anim sounds ---
	if (aframeFlag & K_LVL_ACTIVE_AFRAMEFLAG_SOUND)
	{
		switch (actor->eLastAnimSet)
		{
			case K_LVL_ACT_ANIM_MOVE:
			case K_LVL_ACT_ANIM_MOVE_FAST:
			case K_LVL_ACT_ANIM_MOVE_BK:
			case K_LVL_ACT_ANIM_MOVE_GUN:
			case K_LVL_ACT_ANIM_MOVE_FAST_GUN:
			case K_LVL_ACT_ANIM_MOVE_BK_GUN:
			{
				//SND_PLAY_POSITIONAL_RAND2(SNDIDX_FOOTSTEP_GENERIC_01, SNDIDX_FOOTSTEP_GENERIC_02, actor->pos);
			}
			break;
		}
	}	
	if (aframeFlag_feet & K_LVL_ACTIVE_AFRAMEFLAG_SOUND)
	{
		switch (actor->eLastAnimSet_feet)
		{
			case K_LVL_ACT_ANIM_FEET_MOVE:
			case K_LVL_ACT_ANIM_FEET_MOVE_FAST:
			{
				//SND_PLAY_POSITIONAL_RAND2(SNDIDX_FOOTSTEP_GENERIC_01, SNDIDX_FOOTSTEP_GENERIC_02, actor->pos);
			}
			break;
		}
	}
	///--- update-uri finale ---
	//set final position
	actor->SetPos(actor->pos);
	//set camera vector
	if (actor->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER)
	{
		actor->vecCamFollowPos = D3DXVECTOR2(K_LVL_CAM_LOOK_OFFSET * actor->lookDirXsign, 0.0f);
		if (actor->nAttackStatus == K_LVL_ACT_ATTACK_SHOOTING)
			actor->vecCamFollowPos.x += actor->lookDirXsign * actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->WeaponTemplate.fCameraRecoil;
	}
	//set sprite pos
	if (!actor->templateActor.bComposedAnimation)
	{
		actor->sprite.pos = D3DXVECTOR2((int)ROUND_FLOAT(actor->pos.x), (int)ROUND_FLOAT(actor->pos.y));
		actor->sprite_feet.pos = actor->sprite.pos;
	}
	else
	{
		actor->sprite_feet.pos = D3DXVECTOR2((int)ROUND_FLOAT(actor->pos.x), (int)ROUND_FLOAT(actor->pos.y));
		POINTXYZ_INT stitchpt(0, 0, 0);
		//find stitch point for torso
		if (actor->sprite_feet.animationIdx >= 0)
			m_sprActors.GetAFrameHitPoint(actor->sprite_feet.animationIdx, actor->sprite_feet.currentFrame, 0, &stitchpt);
		actor->sprite.pos = actor->sprite_feet.pos;
		actor->sprite.pos.x += stitchpt.x; actor->sprite.pos.y += stitchpt.y;
	}
	///--- shield and other overhead icons
	//shield/armor
	if ((actor->templateActor.actorClass != K_LVL_ACT_CLASS_PLAYER) && (actor->fArmor > 0.0f) && (actor->fArmor < actor->templateActor.fArmor) && (actor->templateActor.nArmorRating > 0))
	{
		bool bCanSet = true;
		if ((actor->m_sprOverheadIcon.animationIdx == ANM_IGM_INTERFACE_SPR_ICON_ARMOR_APPEAR) && (actor->m_sprOverheadIcon.animStatus != ANIM_STATUS_FRAMELOCK))
			bCanSet = false;

		if (bCanSet)
		{
			int nFrames = m_sprInterface.GetAFramesCnt(ANM_IGM_INTERFACE_SPR_ICON_ARMOR_PROGRESS);
			int nCurFrame = (int)floor(float(nFrames - 1) * (1.0f - (actor->fArmor / actor->templateActor.fArmor)));
			CLAMP(nCurFrame, 0, nFrames - 1);
			actor->m_sprOverheadIcon.animationIdx = ANM_IGM_INTERFACE_SPR_ICON_ARMOR_PROGRESS;
			actor->m_sprOverheadIcon.currentFrame = nCurFrame;
		}
		//clear animation after a time
		if (/*(actor->m_AIsensorInfo.fTimeSinceHit > 1.0f) || */(actor->fLife <= 0.0f))
			actor->m_sprOverheadIcon.animationIdx = -1;
	}
	//clear armor icon
	if ((actor->m_sprOverheadIcon.animationIdx == ANM_IGM_INTERFACE_SPR_ICON_ARMOR_DISAPPEAR) && (actor->m_sprOverheadIcon.animStatus == ANIM_STATUS_FRAMELOCK))
		actor->m_sprOverheadIcon.animationIdx = -1;

	//only update it if set
	if (actor->m_sprOverheadIcon.animationIdx >= 0)
	{
		actor->m_sprOverheadIcon.Update(&m_sprInterface, dTime);
	}

	///--- damage taken - paint red frame while taking damage
	DWORD dwCol = actor->color;
	//paint red when taking damage
	if (actor->nTookDamageFrames > 0)
	{
		actor->nTookDamageFrames--;
		float fAlpha = D3DCOLOR_GETFALPHA(dwCol);
		dwCol = D3DCOLOR_COLORALPHA(0xffff7777, fAlpha);
	}
	else if (actor->cDamageOverTime.eType == CDamageOverTime::K_LVL_DoT_INVINCIBLE) //invulnerability blink
	{
		float fAlpha = D3DCOLOR_GETFALPHA(dwCol);
		if ((actor->GetCurrentBehavior() != AI_BEHAVIOR_PLAY_ANIM) && (m_Timers.GetTimerValue(300.0f) < 0.2f))
			fAlpha *= 0.6f;
		dwCol = D3DCOLOR_COLORALPHA(0xffffffff, fAlpha);
	}

	actor->sprite.color = dwCol;
	actor->sprite_feet.color = dwCol;

}


CActor* CLevel::GetClosestTarget(CActor * sourceActor, EActorClass eTargetClassFilter1, EActorClass eTargetClassFilter2)
{
	if (sourceActor == null)
		return null;
	//nobody attacks if level finished
	if (m_levelState != K_LVL_STATE_PLAYING)
		return null;
	//save some data about current actor:
	bool bAlerted = (sourceActor->fFOVPercent >= 0.9f) ? true : false;
	float fDistSee = sourceActor->templateActor.distSee;
	float fDistHear = sourceActor->templateActor.distHear;
	float fDistDown = 1.0f * K_TILE_SIZE; //2
	float fDistUp = 4.0f * K_TILE_SIZE;	//6
	if (bAlerted)
	{
		fDistHear = fDistSee;
		fDistUp = 6.0f * K_TILE_SIZE; //9
		fDistDown = 2.0f * K_TILE_SIZE;	 //3
	}
	CAABB aabbvision;
	aabbvision.Set_Corrected(
		D3DXVECTOR2(sourceActor->pos.x + sourceActor->lookDirXsign * fDistSee, sourceActor->pos.y + fDistDown),
		D3DXVECTOR2(sourceActor->pos.x - sourceActor->lookDirXsign * fDistHear, sourceActor->pos.y - fDistUp)
	);

	//--- check all actors for enemy ---
	CActor* retvalenemy = null;

	float minDistSq = 1000000.0f;
	for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
	{
		CActor* enemy = m_arrActors[kk];
		
		if (enemy == null)
			continue;
		//can't attack himself
		if (enemy == sourceActor)
			continue;
		//never attack same class
		if (enemy->templateActor.actorClass == sourceActor->templateActor.actorClass)
			continue;

		if (sourceActor->templateActor.actorClass == K_LVL_ACT_CLASS_ZOMBIE)
		{
			//zombie classes attack everything that's made from meat
			if (enemy->templateActor.eMaterial != K_LVL_MATERIAL_FLESH)
				continue;
		}
		else
		{
			//don't attack same class enemies or traps and passive classes
			if (enemy->templateActor.actorClass < K_LVL_ACT_CLASS_PLAYER)
				continue;
		}

		//daca am filtru pe clasele de inamici verific clasa mai intai
		int nIgnore = 0, nIgnoreConditions = 0;
		if (eTargetClassFilter1 != K_LVL_ACT_CLASS_ANY)
		{
			nIgnoreConditions++;
			if (enemy->templateActor.actorClass != eTargetClassFilter1)
				nIgnore++;
		}
		if (eTargetClassFilter2 != K_LVL_ACT_CLASS_ANY)
		{
			nIgnoreConditions++;
			if (enemy->templateActor.actorClass != eTargetClassFilter2)
				nIgnore++;
		}
		if ((nIgnoreConditions > 0) && (nIgnore == nIgnoreConditions))
			continue;
		//nu ia in seama inamic cu energie sub 0 sau flag de not a target (setat de limbo)
		if ((enemy->fLife <= 0.0f) || ((enemy->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET) != 0))
			continue;

		D3DXVECTOR2 enemyDistV = enemy->posHeart - sourceActor->posHeart;
		float viewDstSq = sourceActor->templateActor.distSee * sourceActor->templateActor.distSee;
		float enemyDistSq = D3DXVec2LengthSq(&enemyDistV);

		bool bPreciseFOV = false; //approximate FOV with rectangle? (good for gameplay)
		if ((sourceActor->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_CAN_ROTATE_VIEW) != 0)
			bPreciseFOV = true;

		if (bPreciseFOV)
		{
			//daca inamicul este in spate modifica raza pe cea de auzit, doar daca nu e alertat la maxim. Daca are fov maxim ramane raza vizuala si in spate.
			if ((sourceActor->fFOVPercent < 1.0f) && (sourceActor->templateActor.distHear > 0.0f) && (SIGN(enemyDistV.x) != SIGN(sourceActor->lookDirXsign)))
			{
				viewDstSq = sourceActor->templateActor.distHear * sourceActor->templateActor.distHear;
				//daca il poate auzi si e in linie directa, il aude
				if (enemyDistSq <= viewDstSq)
				{
					if (!IsLineOfSight(sourceActor->posHeart, enemy->posHeart))
						continue;
					//aici il aude deci e foarte aproape, il returnez direct
					return enemy;
				}
			}
			//daca e prea departe sau daca avem unul mai aproape nu il ataca
			if ((enemyDistSq > viewDstSq) || (enemyDistSq >= minDistSq))
				continue;
			//daca e destul de aproape
			//vede daca inamicul este in FOV. Face testul doar daca FOV nu este maxim (adica vede si deasupra)
			if ((sourceActor->fFOVPercent < 1.0f) && (Math_GetAngleBetweenVectors(enemy->posHeart - sourceActor->posHeart, sourceActor->vAngleDir) > (HALF_PI * sourceActor->fFOVPercent)))
				continue;
			//verifica daca am linie directa de vedere
			if (!IsLineOfSight(sourceActor->posHeart, enemy->posHeart))
				continue;
		}
		else //Dreptunghi of Vision! such fast! Much optimal!
		{
			//not in view rectangle
			if (!aabbvision.PointIn(enemy->posHeart))
				continue;
			if (!IsLineOfSight(sourceActor->posHeart, enemy->posHeart))
				continue;
		}

		//perks
		//#PERK: CAUGHT IN THE ACT - if in cover hidden from perpetrators
		if ((enemy->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER) && (enemy->pCover != null) && (enemy->bCrouched) &&
			(SIGN(enemy->pCover->bbox.vCenter.x - enemy->posHeart.x) != SIGN(enemy->pCover->bbox.vCenter.x - sourceActor->posHeart.x)) && //cover between them
			(enemyDistSq > (K_TILE_SIZE * 4) * (K_TILE_SIZE * 4)) )
		{
			if (g_playerSelScr.IsPerkEnabled(enemy->nPlayerOrdinal, &shPerk_CAUGHT_IN_THE_ACT))
				continue;
		}

		//check smoke grenades
		bool bObscured = false;
		for (int ll = 0; ll < m_arrBulletsTemp.Count(); ll++)
		{
			CBullet* bul = m_arrBulletsTemp.m_pData[ll];
			if (bul->type != K_LVL_BULLET_SMOKE_GRENADE)
				continue;
			D3DXVECTOR2 vBulPos = bul->physPt->m_data.pos;
			
			CAABB smokeAABB;
			smokeAABB.Set(vBulPos.x - K_TILE_SIZE, vBulPos.y - 4 * K_TILE_SIZE, vBulPos.x + K_TILE_SIZE, vBulPos.y + K_TILE_SIZE);
			if (AABB_Segment_Intersection(sourceActor->posHeart, enemy->posHeart, smokeAABB))
			{
				bObscured = true;
				break;
			}
		}
		if (bObscured)
			continue;

		//passed all tests and is closer? set ptr on new one
		if ((retvalenemy == null) || (enemyDistSq < minDistSq))
		{
			retvalenemy = enemy;
			minDistSq = enemyDistSq;
		}
	}

	return retvalenemy;
}


CActor * CLevel::GetClosestActorByTemplateName(CActor * sourceActor, WCHAR * sTargetTemplateName, float fMaxDistance)
{
	CActor* retvalenemy = null;
	UINT32 nTargetNameHash = FastHash(sTargetTemplateName);

	for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
	{
		CActor* enemy = m_arrActors[kk];
		if ((enemy == null) || (enemy == sourceActor) || (enemy->templateActor.shName.textHash != nTargetNameHash) || (enemy->bHidden))
			continue;
		//nu ia in seama inamic cu energie sub 0 sau flag de not a target
		if ((enemy->fLife <= 0.0f) || ((enemy->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET) != 0))
			continue;

		D3DXVECTOR2 enemyDistV = enemy->posHeart - sourceActor->posHeart;
		float enemyDistSq = D3DXVec2LengthSq(&enemyDistV);
		//daca e prea departe trece mai departe
		float fSearchRadiusSq = (fMaxDistance <= 0.0f) ? (sourceActor->templateActor.distSee * sourceActor->templateActor.distSee) : (fMaxDistance * fMaxDistance);
		if (enemyDistSq > fSearchRadiusSq)
		{
			continue;
		}
		//daca e destul de aproape:
		//verifica daca am linie directa de vedere
		if (!IsLineOfSight(sourceActor->posHeart, enemy->posHeart))
			continue;

		//daca a trecut toate testele si inamicul curent este mai aproape decat cel selectat initial il setez pe cel nou
		if ((retvalenemy == null) || (D3DXVec2LengthSq(&(retvalenemy->posHeart - sourceActor->posHeart)) > enemyDistSq))
			retvalenemy = enemy;
	}

	return retvalenemy;
}


CCollisionShape* CLevel::GetClosestCover(D3DXVECTOR2 vPos, float fMaxDistance /*= 0.0f*/)
{
	float fMaxDstSq = fMaxDistance * fMaxDistance;
	float fCurrentDist = 0.0f;
	CCollisionShape* pRetShape = null;
	for (int kk = 0; kk < m_arrColShapes.GetSize(); kk++)
	{
		CCollisionShape* shape = m_arrColShapes[kk];
		if (shape->type != K_LVL_COLL_TYPE_COVER)
			continue;
		float fDstSq = D3DXVec2LengthSq(&(shape->bbox.vCenter - vPos));
		if((fMaxDistance > 0.0f) && (fDstSq > fMaxDstSq))
			continue;
		if (!IsLineOfSight(vPos, shape->bbox.vCenter))
			continue;
		if ((fDstSq < fCurrentDist) || (pRetShape == null))
		{
			pRetShape = shape;
			fCurrentDist = fDstSq;
		}
	}

	return pRetShape;
}

void CLevel::AddAIEvent(EAIEventType eventType, UINT32 ownerUID, int ownerClass, D3DXVECTOR2 vPos, float radius, float duration, UINT32 targetUID)
{
	//raza negativa inseamna infinita
	if ((radius == 0.0f) || (duration <= 0.0f))
		return; 
	//vad daca am deja un event cu acelasi owner si acelasi event il suprascriu pe cel vechi ca sa nu fie mai multe
	CAIEvent* nevt = null;
	//if owner is 0 means generic AI event (alert sounds)
	if (ownerUID != 0)
	{
		for (int kk = 0; kk < m_arrAIevents.GetSize(); kk++)
		{
			//daca are targetUID diferit nu il suprascrie pentru ca pot fi eventuri la grenade care sunt la fel in afara de targetUID
			if ((m_arrAIevents[kk]->ownerUID == ownerUID) && (m_arrAIevents[kk]->nType == eventType) && (m_arrAIevents[kk]->targetUID == targetUID))
			{
				nevt = m_arrAIevents[kk];
				break;
			}
		}
	}
	//daca nu am gasit atunci adaug unul nou
	if (nevt == null)
	{
		nevt = new CAIEvent();
		//adaug eventul doar daca este unul nou
		m_arrAIevents.Add(nevt);
	}
	//set event data
	nevt->ownerUID = ownerUID;
	nevt->nType = eventType;
	nevt->fRadius = radius;
	nevt->fDuration = duration;
	nevt->pos = vPos;
	nevt->ownerClass = ownerClass;
	nevt->targetUID = targetUID;
}

void CLevel::DeleteAITargetedEvent(EAIEventType eEvtType, UINT32 targetUID /*= 0*/)
{
	for (int kk = 0; kk < m_arrAIevents.GetSize(); kk++)
	{
		if ((m_arrAIevents[kk]->nType == eEvtType) && ((m_arrAIevents[kk]->targetUID == targetUID) || (targetUID == 0)))
		{
			m_arrAIevents[kk]->fDuration = 0.0f;
			m_arrAIevents[kk]->fRadius = 0.0f;
		}
	}

}

//Intoarce eventul cel mai apropiat de actorul callerActor
CAIEvent * CLevel::GetMostImportantAIEvent(CActor * callerActor, EAIEventType eTypeFilter)
{
	if (callerActor == null)
		return null;

	CAIEvent* returnEvent = null;
	float mindistSq = 100000.0f;

	for (int kk = 0; kk < m_arrAIevents.GetSize(); kk++)
	{
		CAIEvent* evt = m_arrAIevents[kk];
		UINT32 callerUID = callerActor->GetUID();
		//ignora mesajele initiate de el insusi
		if ((evt->ownerUID == callerUID) || (evt->fDuration <= 0.0f))
			continue;
		//type filter?
		if ((eTypeFilter > K_LVL_AI_EVENT_NONE) && (evt->nType != eTypeFilter))
			continue;
		//daca eventul este targetat pentru altcineva se ignora
		if ((evt->targetUID != 0) && (evt->targetUID != callerUID))
			continue;
		//ignore actor if different from class foe filters
		int nIgnore = 0, nIgnoreConditions = 0;
		if (callerActor->templateActor.foeClassFilter1 != K_LVL_ACT_CLASS_ANY)
		{
			nIgnoreConditions++;
			if (evt->ownerClass != callerActor->templateActor.foeClassFilter1)
				nIgnore++;
		}
		if (callerActor->templateActor.foeClassFilter2 != K_LVL_ACT_CLASS_ANY)
		{
			nIgnoreConditions++;
			if (evt->ownerClass != callerActor->templateActor.foeClassFilter2)
				nIgnore++;
		}
		if ((nIgnoreConditions > 0) && (nIgnore == nIgnoreConditions))
			continue;
		//!!! daca e event targetat il intoarce direct si il consuma, fara sa mai stea pe ganduri, cu exceptia IDLE_TICK
		if ((evt->targetUID == callerUID) && (evt->nType > K_LVL_AI_EVENT_IDLE_TICK))
			return evt;
		//verific distanta (daca raza event nu e infinita adica negativa)
		float evtdstsq = 0.0f;
		if (evt->fRadius > 0.0f)
		{
			evtdstsq = D3DXVec2LengthSq(&(callerActor->posHeart - evt->pos));
			if (evtdstsq > evt->fRadius * evt->fRadius)
				continue;
		}
		//verific sa am prioritate mai mare sau egala cu cea curenta si distanta mai mica
		if (returnEvent != null)
		{
			//daca are prioritate mai mica il sare
			if (evt->nType < returnEvent->nType)
			{
				continue;
			}
			else if ((evt->nType == returnEvent->nType) && (evtdstsq > mindistSq)) //daca are aceeasi prioritate dar este mai departe il sare
			{
				continue;
			}
		}
		//see if we have ignored events
		if (callerActor->templateActor.AItemplate->m_arrIgnoredEvents.Count() > 0)
		{
			if (callerActor->templateActor.AItemplate->m_arrIgnoredEvents.IndexOf(evt->nType) >= 0)
				continue;
		}
		//dupa ce am exclus eventurile ce se puteau exclude:
		//verific linie directa, cel mai costisitor test, sau daca e event cu raza infinita (fara pozitie)
		if ((evt->fRadius < 0.0f) || (IsLineOfSight(callerActor->posHeart, evt->pos)))
		{
			//daca eventul este mai aproape sau daca eventul e mai important decat cel initial
			if ((evtdstsq < mindistSq) || ((returnEvent != null) && (evt->nType > returnEvent->nType)) )
			{
				mindistSq = evtdstsq;
				returnEvent = evt;
			}
		}
	}

	return returnEvent;
}


void CLevel::SetAI(IActiveInterface * active, int AIstate, CVariantCollection * params, INT32 targetID)
{
	if (active == null)
		return;
	active->targetID_ini = targetID;
	active->pTarget = GetIActiveInterfacePtr(targetID);

	//daca am null la params nu seteaza params, doar le da clear
	if(params != null)
		active->varAIparams = *params; //aici sterge automat params vechi
	else  //daca este null sterg parametrii
		active->varAIparams.DeleteAll();

	active->AIstate = AIstate;
	//generice
	active->AItargetUID = 0;	//?? trebuie resetat?
	active->fTimelineAI = 0.0f; //?? trebuie resetat?
	active->AItimer1 = 0.0f; active->AItimer2 = 0.0f;
	active->AIfvar1 = 0.0f; active->AIfvar2 = 0.0f; active->AIfvar3 = 0.0f;
	active->AIsubState = 0;
	active->AIstrvar1.Reset(); active->AIstrvar2.Reset();
	//setari initiale particulare
	switch (AIstate)
	{
		case K_AI_STATE_ACTIVE_ZOMBIE_SPAWNER:
		{
			//spawn timer
			active->AItimer1 = 0.0f; 
			active->AItimer2 = active->varAIparams.GetVariantByName(L"f_spawnFreq")->m_asFloat;
			//spawns count
			active->AIvar1 = 0;
			active->AIvar2 = active->varAIparams.GetVariantByName(L"n_maxSpawns")->m_asINT32;
			//spawner state: 0-not enabled yet, 1-appearing, 2-active, 3-disabled
			active->AIsubState = 0;
		}
		break;
		case K_AI_STATE_ACTIVE_SWINGING_FRONTOBJ:
		{
			active->AIfvar1 = randsign() * (0.06f + randfloat(0.14f)); //viteza unghiulara
		}
		break;
		case K_AI_STATE_ACTIVE_HEALTH_BOX:
		case K_AI_STATE_ACTIVE_AMMO_BOX:
		{
			//wait 5 seconds before disappearing when empty
			active->AItimer1 = 5.0f;
		}
		break;
		case K_AI_STATE_ACTIVE_BOMB:
		{
			active->AItimer1 = active->varAIparams.GetVariantByName(L"f_explodeTimerSec")->m_asFloat;
			if (active->AItimer1 <= 0.0f)
			{
				ErrorBox(K_ERR_WARNING, L"Bomb without timer! ID:%d", active->ID);
				active->AItimer1 = 60.0f;
			}
		}
		break;
		case K_AI_STATE_FN_LIGHT_ANG_CONE_XZ_TIME:
		{
			active->AIfvar1 = active->varAIparams.GetVariantByName(L"f_coneHeight")->m_asFloat;
			active->AIfvar2 = active->varAIparams.GetVariantByName(L"f_coneRadius")->m_asFloat;
			active->AItimer1 = active->varAIparams.GetVariantByName(L"f_timeMul")->m_asFloat;
			active->AItimer2 = active->varAIparams.GetVariantByName(L"f_timeAdd")->m_asFloat;
		}
		break;
		case K_AI_STATE_ACTIVE_DOORFACE_AUTOCLOSE:
		{
			//door timer (cat timp sta usa deschisa) il tinem in AItimer1
			active->AItimer1 = 0.0f;
			//este deschisa sau inchisa acum?
			active->AIvarBool1 = false;
		}
		break;
		case K_AI_STATE_ACTIVE_TEAM_TELEPORTER_2FRAMES:
		{
			active->varAIparams.SetNamedVarUINT32(L"nToucherUID", 0);
			//door timer (cat timp sta usa deschisa) il tinem in AItimer1
			active->AItimer1 = 0.0f;
			//este deschisa sau inchisa acum?
			active->AIvarBool1 = false;
		}
		break;
		case K_AI_STATE_ACTIVE_CHECKPOINT:
		{
			bool bIsFirst = (active->varAIparams.GetVariantByName(L"n_isFirst")->m_asUINT32 != 0);
			//daca este primul ii dau touch automat
			if (bIsFirst)
			{
				active->Touch(active->GetUID(), 0.0f);
				//save checkpoint
				vLastSpawnPoint = active->pos;
			}
		}
		break;
		case K_AI_STATE_FN_TOUCH_WHEN_SEE_PLAYER:
		{
			//unghiul introdus
			active->AIfvar1 = active->varAIparams.GetVariantByName(L"f_angle")->m_asFloat;
			//aduc unghiul in -PI...PI
			//active->AIfvar1 -= PI; //aici ar trebui facuta o functie care sa trateze asta
			//FOV
			active->AIfvar2 = active->varAIparams.GetVariantByName(L"f_angleFOV")->m_asFloat;
			//range
			active->AIfvar3 = active->varAIparams.GetVariantByName(L"f_radius")->m_asFloat;
			//set angle
			active->fAngle = active->fAngle_ini = active->AIfvar1;
			active->AItimer1 = 0.0f; //timer cooldown
		}
		break;
		case K_AI_STATE_COLL_FOG_OF_WAR:
		{
			active->AIfvar1 = 1.0f; //transparenta (full opaque)
			active->color = active->color_ini = D3DCOLOR_COLORALPHA(K_LVL_COLL_FOW_COLOR, active->AIfvar1);
		}
		break;
		case K_AI_STATE_COLL_BREAKABLE_DOOR:
		{
			//was hit flag
			active->AIvarBool1 = false;
			//viata usii (poate fi sparta de unele gloante)
			active->AIfvar1 = 100000.0f; //by default nu poate fi distrusa de shotgun (sau foarte greu)
			active->AIfvar2 = active->AIfvar1; //viata initiala
			CVariantComplex *cvar = active->varAIparams.GetVariantByName(L"f_life");
			if (cvar->m_type == CVariantComplex::K_ARGTYPE_FLOAT)
			{
				active->AIfvar1 = cvar->m_asFloat;
				//salvam si energia initiala
				active->AIfvar2 = active->AIfvar1;
			}
			//flag for when it gets hit
			active->AIvarBool1 = false;
			//timer for when it shakes
			active->AItimer1 = 0.0f;
		}
		break;
		case K_AI_STATE_COLL_BREAKABLE_WINDOW:
		{
			//was hit flag
			active->AIvarBool1 = false;
			//viata 
			active->AIfvar1 = 2.0f; //by default se sparge usor
			active->AIfvar2 = active->AIfvar1; //viata initiala
			CVariantComplex *cvar = active->varAIparams.GetVariantByName(L"f_life");
			if (cvar->m_type == CVariantComplex::K_ARGTYPE_FLOAT)
			{
				active->AIfvar1 = cvar->m_asFloat;
				//salvam si energia initiala
				active->AIfvar2 = active->AIfvar1;
			}
		}
		break;
		case K_AI_STATE_PARTICLES_GENERATOR:
		{
			//tipul generatorului il ia din params
			int genType = g_particlesMgr.GetPartEmitterTypeByNameHash(active->varAIparams.GetVariantByName(L"s_Type")->m_strArg.textHash);
			int partLayer = g_particlesMgr.GetParticleLayerByName(active->varAIparams.GetVariantByName(L"s_Layer")->m_strArg.textHash);
			//ca sa nu intre de mai multe ori si sa aloce de mai multe ori. Daca se intampla trebuie dezalocat mai intai
			assert(active->varAIparams.GetVariantByName(L"emitterPtr")->m_type == CVariantComplex::K_ARGTYPE_NONE);

			CParticleEmitter * pe = g_particlesMgr.AddPartEmitter(genType, &active->bbox, partLayer);
			//salveaza aici pointer la ParticleEmitter-ul alocat si il controlez din update sa ii dau stop si play cand iese din ecran
			active->varAIparams.SetNamedVarVoidP(L"emitterPtr", pe);
		}
		break;
		case K_AI_STATE_TRIGGER_IN_OUT:
		{
			//b_triggerPlayer 
			active->AIfvar1 = (float)active->varAIparams.GetVariantByName(L"b_triggerPlayer")->m_asINT32;
			//b_triggerActor
			active->AIfvar2 = (float)active->varAIparams.GetVariantByName(L"b_triggerActor")->m_asINT32;
			if (active->AIfvar2 != 0.0f)
			{
				DebugPrintA("TRIGGER_IN_OUT - all actors flag enabled! don't use too much of these\n");
			}
			//s_onOutScript
			active->AIstrvar1.Init(active->varAIparams.GetVariantByName(L"s_onOutScript")->m_strArg.text);
			//last state:
			active->AIvar1 = 0; //deactivated
		}
		break;
		case K_AI_STATE_FN_FOLLOW_TARGET_RAIL:
		{
			CMiscObjectRail* rail = null;
			//find rail
			for (int kk = 0; kk < m_arrMiscObjects.Count(); kk++)
			{
				if (m_arrMiscObjects[kk]->ID == active->targetID_ini)
				{
					rail = dynamic_cast<CMiscObjectRail*>(m_arrMiscObjects[kk]);
				}
			}
			if (rail == NULL)
			{
				ErrorBox(K_ERR_WARNING, L"Rail id %d not found for object ID %d!", active->targetID_ini, active->ID);
				break;
			}
			//save rail ptr
			active->varAIparams.SetNamedVarVoidP(L"railPtr", rail);
			//this is first time initialization
			float fPos = active->varAIparams.GetVariantByName(L"f_positionPercent")->asFloat();
			CLAMP(fPos, 0.0f, 1.0f);
			//cursor pozitie rail
			active->AItimer1 = fPos * rail->fLength;
			//salvez si viteza
			active->AIfvar1 = 0.0f; //viteza
			active->AIfvar1 = active->varAIparams.GetVariantByName(L"f_speedPPS")->asFloat();
			//wait timerul de capat de rail
			active->AItimer2 = active->varAIparams.GetVariantByName(L"f_pointPauseSec")->asFloat();
			//looping rail?
			active->AIvarBool1 = (active->varAIparams.GetVariantByName(L"b_looping")->m_asINT32 != 0);
		}
		break;
		//unknown or no AI state
		default:
			break;
	}
}

void CLevel::SetAIparams(IActiveInterface * active, CVariantCollection * params, bool bClearParams)
{
	if (active == null)
		return;
	
	//daca am null la params nu seteaza params, doar le da clear
	if((params == null) || (bClearParams))
		active->varAIparams.DeleteAll();
	
	if (params != null)
	{
		for (int kk = 0; kk < params->GetVariantCount(); kk++)
		{
			active->varAIparams.AddVariant(*params->m_variants[kk]);
		}
	}
}


void CLevel::CleanupDeadObjects()
{
	//check active objects
	for (int kk = m_arrActives.GetSize() - 1; kk >= 0; kk--)
	{
		if (m_arrActives[kk]->bReleaseIt)
		{
			SAFE_DELETE(m_arrActives[kk]);
			m_arrActives.Remove(kk);
		}
	}

	//check actors
	for (int kk = m_arrActors.GetSize() - 1; kk >= 0; kk--)
	{
		///--- must kill actor! - last thing in update - dezalocari finale ---
		if (m_arrActors[kk]->bReleaseIt)
		{
			CActor* act = m_arrActors[kk];
			// make sure we don't keep pointer to actor
			for (int i = 0; i < m_arrActors.GetSize(); i++)
			{
				if (m_arrActors[i]->m_AIsensorInfo.pTargetedActor == act)
				{
					m_arrActors[i]->m_AIsensorInfo.pTargetedActor = NULL;
				}
			}
			// now release it
			SAFE_DELETE(m_arrActors[kk]);
			m_arrActors.Remove(kk);
		}
	}
}

void CLevel::UpdateAI(float dTime)
{
	//reset targets left (will be counted below)
	m_arrStats[K_LVL_STATS_TARGETS_LEFT] = 0;

	//update AI events
	for (int kk = m_arrAIevents.GetSize() - 1; kk >= 0; kk--)
	{
		CAIEvent* evt = m_arrAIevents[kk];
		evt->fDuration -= dTime;
		if (evt->fDuration <= 0.0f)
		{
			SAFE_DELETE(evt);
			m_arrAIevents.Remove(kk);
		}
	}

	//check active objects and save interactibles
	m_arrActivesPtrInteract.Clear();
	for (int kk = m_arrActives.GetSize() - 1; kk >= 0; kk--)
	{
		UpdateAI_active(m_arrActives[kk], dTime);
		//add interactible?
		if (m_arrActives[kk]->bCanInteract)
			m_arrActivesPtrInteract.Add(m_arrActives[kk]);
	}

	//check lights
	for (int kk = 0; kk < m_arrLights.GetSize(); kk++)
	{
		UpdateAI_light(m_arrLights[kk], dTime);
	}
	//check collision boxes
	for (int kk = m_arrColShapes.GetSize() - 1; kk >= 0; kk--)
	{
		UpdateAI_collshape(m_arrColShapes[kk], dTime);
	}

	//check actors
	double fHashKey = 0.0f;
	for (int kk = m_arrActors.GetSize() - 1; kk >= 0; kk--)
	{
		CActor* act = m_arrActors[kk];
		UpdateAI_actor(act, dTime);
		//add some floats to detect network inconsistencies
		fHashKey += act->pos.x + act->pos.y + act->AItimerDecision + act->fLife + act->fArmor + act->fStunTimer;

		//count targets left
		if (act->GetCurrentBehavior() != EAIBehaviorType::AI_BEHAVIOR_DEAD)
		{
			if ((act->templateActor.actorClass >= K_LVL_ACT_CLASS_HUMAN) || (act->templateActor.actorClass == K_LVL_ACT_CLASS_HOSTAGE))
			{
				m_arrStats[K_LVL_STATS_TARGETS_LEFT]++;
			}
		}
	}

	{
#if defined(K_NET_STRICT_SYNC_CHECK)
		//build hash
		WCHAR strKey[MAX_PATH];
		StringCchPrintf(strKey, MAX_PATH, L"%.9g", fHashKey);
		m_dwSyncCheckHash = FastHash(strKey);
#else
		m_dwSyncCheckHash = 0;
#endif
	}
}

int CLevel::GetNextRandomLevel()
{
	static int nNextIndex = UTGetChaptersList().GetTotalLevelsCnt();
	static int arrLevels[120] = { 0 };
	static int nValidLevels = 0;

	if (nNextIndex >= nValidLevels)
	{
		nNextIndex = 0;
		nValidLevels = 0;
		for (int kk = 0; kk < UTGetChaptersList().GetTotalLevelsCnt(); kk++)
		{
			if (g_levelStats[kk].nLevelType != K_GAME_LSTYPE_NOTSET)
				arrLevels[nValidLevels++] = kk;
		}
		Random_ShuffleArray(arrLevels, nValidLevels, 1000);
	}

	int nLevel = arrLevels[nNextIndex];
	nNextIndex++;
	return nLevel;
}


int CLevel::Local_ComputeMissionXP(int nStars)
{
	int nTotalXPPoints = 0;
	if (nStars > 0)
		nTotalXPPoints = 50;
	//Player 1
	int nXPpl1 = m_arrStats[K_LVL_STATS_PL1_KILLS] * 10 + m_arrStats[K_LVL_STATS_PL1_HOSTAGES_SAVED] * 20;
	if (nXPpl1 < 0) nXPpl1 = 0;
	//Player 2
	int nXPpl2 = m_arrStats[K_LVL_STATS_PL2_KILLS] * 10 + m_arrStats[K_LVL_STATS_PL2_HOSTAGES_SAVED] * 20;
	if (nXPpl2 < 0) nXPpl2 = 0;

	nTotalXPPoints += nXPpl1 + nXPpl2;
	//add common stuff
	nTotalXPPoints += m_arrStats[K_LVL_STATS_LEVEL_ARREST_TARGETS_ARRESTED] * 50;
	nTotalXPPoints += m_arrStats[K_LVL_STATS_BOMBS_DISARMED] * 50;

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	LOG(L"[Level] Mission total XP: %d", nTotalXPPoints);
#endif
	return nTotalXPPoints;
}

void CLevel::Update(float dTime_original)
{
	if (!m_bLoaded)
		return;

	///--- time control ---
	//variatie time multiplier
	if (m_fTimeMultiplierDuration > 0.0f)
	{
		m_fTimeMultiplierDuration -= dTime_original;
		if (m_fTimeMultiplierDuration <= 0.0f)
		{
			//cand durata scade la 0 revin la timeline original
			m_fTimeMultiplier = 1.0f;
			SND_STOP(SNDIDX_HEARTBEAT, true);
		}
	}
	REACH_VALUE_LINEAR(m_fTimeMultiplier_real, m_fTimeMultiplier, dTime_original);
	//set sounds freq global
	SND_SET_GROUP_FREQUENCY("ingame", m_fTimeMultiplier_real, false);

	//calcul dTime final
	float dTime = dTime_original * m_fTimeMultiplier_real;
	fLocalTimeline += dTime;

	//update local timers
	m_Timers.Update(dTime);

	//--- thunder timer ---
	if (m_fThunderTimer > 0.0f)
	{
		m_fThunderTimer -= dTime;
		//sunetul incepe mai devreme
		if ((m_fThunderTimer < 0.5f) && (m_fThunderTimer + dTime >= 0.5f))
		{
			//SND_PLAY(SNDIDX_THUNDER);
		}
		//resets counter
		if (m_fThunderTimer <= 0.0f)
		{
			m_fThunderTimer = 10.0f + randfloat(20.0f);
		}
	}

	//state machine logic
	switch (m_levelState)
	{
		case K_LVL_STATE_PLAYING:
		{
			//it plays the start game verse and join verse later for player 2 (when pl1 finishes talking, don't have a callback for that)
			//play level start sound from first player
			if ((pPlayerActor[0] != null) && (fLocalTimeline > 0.5f) && (fLocalTimeline - dTime <= 0.5f))
			{
				nIntroVerseState = 1;
				PlayActorSoundVerse(pPlayerActor[0], K_LVL_ACT_VERSE_START_GAME);
			}
			//check to see when he stopped talking
			if (nIntroVerseState == 1)
			{
				if (pPlayerActor[1] == null)
				{
					nIntroVerseState = 0;
				}
				else
				{
					if ((m_Timers.Tick(250)) && (pPlayerActor[0] != null) && (!SND_IS_PLAYING(pPlayerActor[0]->nLastPlayedVerseSndIdx)))
					{
						PlayActorSoundVerse(pPlayerActor[1], K_LVL_ACT_VERSE_JOIN_GAME);
						nIntroVerseState = 0;
					}
				}
			}

			//#TUTORIAL: showing tutorial windows if in single player and not on custom content
			/*
			if ((fLocalTimeline > 0.3f) && (fLocalTimeline - dTime <= 0.3f) && (m_nPlayers == 1))
			{
				if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
				{
					int nLevelIdx = g_userData[K_MEMID_SELECTED_LEVEL] + g_userData[K_MEMID_SELECTED_CHAPTER] * K_GAME_LEVELS_PER_CHAPTER;
					if (g_levelStats[nLevelIdx].nLevelType == K_GAME_LSTYPE_ARREST_WARRANT)
						App_TutorialWindowShow(K_MEMID_TUT_ARREST_MODE);
					else if (g_levelStats[nLevelIdx].nLevelType == K_GAME_LSTYPE_BOMB)
						App_TutorialWindowShow(K_MEMID_TUT_BOMB_MODE);
					else if (g_levelStats[nLevelIdx].nLevelType == K_GAME_LSTYPE_HOSTAGE)
						App_TutorialWindowShow(K_MEMID_TUT_HOSTAGE_MODE);
					else if ((g_levelStats[nLevelIdx].nPlayedTimes > 0) && (pPlayerActor[0]->pSelectedWeapon[K_LVL_ACT_WEAPON_SECONDARY]->status != K_LVL_WPN_STATUS_UNKNOWN))
						App_TutorialWindowShow(K_MEMID_TUT_INTERFACE_IGM);
				}
				//infinite mode
				if ((m_unLoadedLevelFlags & K_LVL_LEVEL_FLAG_VINFINITE_MODE) != 0)
				{
					App_TutorialWindowShow(K_MEMID_TUT_VINFINITE_MODE);
				}
			}
			*/

			//set to true to enable hot join
			static const bool bEnableHotJoin = false;
			///--- handle controllers dynamically and hot join ---
			for (int plidx = 0; plidx < K_MAX_PLAYERS_CNT; plidx++)
			{
				//hot join: enters here only once, for new controllers only
				if (m_arrPlayerControllersIIDs[plidx] == -1) //if empty check if fire was pressed on another ctrlr and set it to this player
				{
					//comment next line to enable first ingame hotjoin
					//if(!bEnableHotJoin)
						//continue;
					//HOT JOIN LOGIC
					for (int ll = 0; ll < UTGetCtrlrMgr().m_arrControllers.size(); ll++)
					{
						CController* ctrlr = UTGetCtrlrMgr().m_arrControllers[ll];
						//Shows controller mapping - only when not online
						if ((ctrlr->eType == K_CM_CT_JOYSTICK_SDL) && (!UTGetAppClass().IsGameNetworked()) && (false == UTGetControlsManager().bIsBlocking) && 
							(ctrlr->sCommands.keyState[K_CM_COMMAND_SELECT] == K_CM_BUTSTATE_JUSTPRESSED))
						{
							UTGetControlsManager().ShowLayerOnce("LAYER_ID_CONTROLLER_MAP");
						}
						//when player was left without controller give him the new controller when ctrlr touched
						bool bActivate = false;
						if (pPlayerActor[plidx] != null) //setting controller for player with disconnected controller
						{
							bActivate = ctrlr->WasControllerTouched(true);
						}
						else //joining now
						{
							bActivate = ((ctrlr->sCommands.keyState[K_CM_COMMAND_FIRE1] == K_CM_BUTSTATE_JUSTRELEASED) ||
										(ctrlr->sCommands.keyState[K_CM_COMMAND_JUMP] == K_CM_BUTSTATE_JUSTRELEASED));
						}

						if ((ctrlr != null) && (bActivate))
						{
							bool bAlreadyUsed = false;
							for (int jj = 0; jj < K_MAX_PLAYERS_CNT; jj++)
							{
								if (m_arrPlayerControllersIIDs[jj] == ctrlr->nSDLInstanceId)
								{
									bAlreadyUsed = true;
									break;
								}
							}
							//daca nu e folosit il seteaza playerului caruia ii lipseste
							if (!bAlreadyUsed)
							{
								//save ctrlr ID
								m_arrPlayerControllersIIDs[plidx] = ctrlr->nSDLInstanceId;
								//update selection screen too !!! used in respawn
								g_playerSelScr.m_arrPlayers[plidx].nInstanceID = ctrlr->nSDLInstanceId;
								//load saved type for panel
								EPSSPlayerClass eType = (EPSSPlayerClass)g_userData[K_MEMID_PANEL1_CLASS + plidx * (K_MEMID_PANEL2_CLASS - K_MEMID_PANEL1_CLASS)];
								//set hot join selection
								m_arrPlayerSelHotJoin[plidx] = (int)eType;
								//daca nu a fost facuta selectie in selScreen pun pe default first class
								if ((m_arrPlayerSelHotJoin[plidx] < 0) || (m_arrPlayerSelHotJoin[plidx] >= K_PSS_CLASSES_COUNT))
								{
									m_arrPlayerSelHotJoin[plidx] = K_PSS_CLASS_ASSAULTER;
								}
								break;
							}
						}
					}
				}
				else //daca nu e empty verific daca mai exista controllerul respectiv
				{
					CController* ctrlr = UTGetCtrlrMgr().GetControllerByInstanceID(m_arrPlayerControllersIIDs[plidx]);
					if (ctrlr == null)
					{
						m_arrPlayerControllersIIDs[plidx] = -1;
					}
					else //pentru hot join char selection
					{
						//he played before, must select again (HOT JOIN)
						if (m_arrPlayerSelHotJoin[plidx] == -1)
						{
							if (bEnableHotJoin)
							{		
								if ((ctrlr->sCommands.keyState[K_CM_COMMAND_FIRE1] == K_CM_BUTSTATE_JUSTRELEASED) ||
									(ctrlr->sCommands.keyState[K_CM_COMMAND_JUMP] == K_CM_BUTSTATE_JUSTRELEASED))
								{
									//set hot join selection
									m_arrPlayerSelHotJoin[plidx] = (int)g_playerSelScr.m_arrPlayers[plidx].eType;
									//daca nu a fost facuta selectie in selScreen pun pe default first class
									if (m_arrPlayerSelHotJoin[plidx] < 0)
									{
										m_arrPlayerSelHotJoin[plidx] = K_PSS_CLASS_ASSAULTER;
									}

									m_interfaceIGM.SetHotJoinSelection(plidx, m_arrPlayerSelHotJoin[plidx]);
								}
							}
						}
						else if ((m_arrPlayerSelHotJoin[plidx] != -1) && (pPlayerActor[plidx] == null))
						{
							bool bCheckSpawn = false;
							//played before: spawn it immediately
							if (m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED + plidx * K_LVL_STATS_PLAYER_STATS_COUNT] == 1)
							{
								bCheckSpawn = true;
							}
							else
							{
								if ((ctrlr->sCommands.keyState[K_CM_COMMAND_FIRE1] == K_CM_BUTSTATE_JUSTRELEASED) ||
									(ctrlr->sCommands.keyState[K_CM_COMMAND_JUMP] == K_CM_BUTSTATE_JUSTRELEASED))
									bCheckSpawn = true;
							}

							if (bCheckSpawn)
							{
								//spawn pos
								D3DXVECTOR2 vSpawnPos = m_arrPlayerLastSafePos[plidx];
								CAABB aabbSpawn;
								CAABB* p_aabbPeer = null;
								aabbSpawn.Set(vSpawnPos.x - 5.0f, vSpawnPos.y - 22.0f, vSpawnPos.x + 5.0f, vSpawnPos.y);

								int nOtherPlayerIdx = (plidx + 1) % K_MAX_PLAYERS_CNT;
								bool bSpawnIt = false;
								//always spawn near the other player when COOP
								if ((pPlayerActor[nOtherPlayerIdx] != null) && (pPlayerActor[nOtherPlayerIdx]->collisionFlags & K_DIRFLAG_DOWN) && (!pPlayerActor[nOtherPlayerIdx]->bOnLadder))
								{
									//only spawn if player is there
									EAIBehaviorType eOtherBehave = pPlayerActor[nOtherPlayerIdx]->GetCurrentBehavior();
									if ((eOtherBehave == AI_BEHAVIOR_PLAYER_CONTROL) || (eOtherBehave == AI_BEHAVIOR_DEAD))
									{
										vSpawnPos = pPlayerActor[nOtherPlayerIdx]->pos;
										aabbSpawn = pPlayerActor[nOtherPlayerIdx]->bbox;
										p_aabbPeer = &pPlayerActor[nOtherPlayerIdx]->bbox;
										bSpawnIt = true;
									}
								}
								else if (pPlayerActor[nOtherPlayerIdx] == null)
								{
									bSpawnIt = true;
								}

								if (bSpawnIt)
								{
									//set selScreen too for next spawn. If player is different from the selection it resets the selection
									if (g_playerSelScr.m_arrPlayers[plidx].eType != (EPSSPlayerClass)m_arrPlayerSelHotJoin[plidx])
									{
										g_playerSelScr.m_arrPlayers[plidx].Init((EPSSPlayerClass)m_arrPlayerSelHotJoin[plidx]);
										g_playerSelScr.m_arrPlayers[plidx].bSelected = true; //marcheaza ca si cum as fi selectat in ecranul anterior
									}
									g_playerSelScr.m_arrPlayers[plidx].nInstanceID = ctrlr->nSDLInstanceId;
									//save hotjoin selection?
									g_playerSelScr.SaveSelection();
									
									bool bNeverPlayed = false;
									if (m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED + plidx * K_LVL_STATS_PLAYER_STATS_COUNT] == 0)
										bNeverPlayed = true;
									
									//spawn it
									if (GetBestSpawningPos(&vSpawnPos, aabbSpawn, p_aabbPeer))
									{
										SpawnPlayer(vSpawnPos, plidx);
									}

									//achievements and level stats
									if(!bNeverPlayed)
										IncreaseLevelStatistics(K_LVL_STATS_PL1_USE_EXTRA_LIFE_CNT + pPlayerActor[plidx]->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT);

									//say spawn verse
									PlayActorSoundVerse(pPlayerActor[plidx], K_LVL_ACT_VERSE_JOIN_GAME);

									//scad numarul de vieti si anunt interfata
									if (m_arrStats[K_LVL_STATS_PL1_LIVES + plidx * K_LVL_STATS_PLAYER_STATS_COUNT] > 0)
										m_arrStats[K_LVL_STATS_PL1_LIVES + plidx * K_LVL_STATS_PLAYER_STATS_COUNT]--;

									m_interfaceIGM.SetLivesLeft(m_arrStats[K_LVL_STATS_PL1_LIVES], m_arrStats[K_LVL_STATS_PL2_LIVES]);
								}
								else
								{
									//SND_PLAY_ONCE(SNDIDX_DENIED);
								}
							}
							//here you can change character when hot joining (not having played before)
							/*
							#DMC: commented out 13 oct 2020
							if (m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED + plidx * K_LVL_STATS_PLAYER_STATS_COUNT] == 0)
							{
								if (ctrlr->sCommands.keyState[K_CM_COMMAND_LEFT] == K_CM_BUTSTATE_JUSTPRESSED)
								{
									m_arrPlayerSelHotJoin[plidx]--;
									if (m_arrPlayerSelHotJoin[plidx] < 0)
										m_arrPlayerSelHotJoin[plidx] = K_PSS_CLASSES_COUNT - 1;
								}
								else if (ctrlr->sCommands.keyState[K_CM_COMMAND_RIGHT] == K_CM_BUTSTATE_JUSTPRESSED)
								{
									m_arrPlayerSelHotJoin[plidx]++;
									if (m_arrPlayerSelHotJoin[plidx] >= K_PSS_CLASSES_COUNT)
										m_arrPlayerSelHotJoin[plidx] = 0;
								}
							}
							*/
						}
					}
				}

				//update player controller
				if (pPlayerActor[plidx] != null)
				{
					int nOldIID = pPlayerActor[plidx]->nControllerInstanceID;
					//update player ctrlr
					pPlayerActor[plidx]->nControllerInstanceID = m_arrPlayerControllersIIDs[plidx];
					//re-initialize igm interface when changing controller (update helper strings)
					if ((nOldIID < 0) && (m_arrPlayerControllersIIDs[plidx] >= 0))
					{
						//set interface pointers
						m_interfaceIGM.Init(&m_sprInterface, pPlayerActor[0], pPlayerActor[1]);
					}
				}
				else //hot join ingame selection and spawning
				{
					m_interfaceIGM.SetHotJoinSelection(plidx, m_arrPlayerSelHotJoin[plidx]);
				}

				///--- updates player selection for strategic points ---
				//#DMC: commented out 13 oct 2020
				/*
				if (m_arrPlayerSelStrategic[plidx] >= 0)
				{
					CController* ctrlr = UTGetControllersManager().GetControllerByInstanceID(m_arrPlayerControllersIIDs[plidx]);

					if ((ctrlr == null) || (ctrlr->sCommands.keyState[K_CM_COMMAND_STRATEGIC_MENU] != K_CM_BUTSTATE_PRESSING))
					{
						//exit 
						m_arrPlayerSelStrategic[plidx] = -1;
						m_interfaceIGM.SetStrategicSelection(plidx, -1);
						//#HACK:skip next just pressed check
						ctrlr->sCommands.keyState[K_CM_COMMAND_STRATEGIC_MENU] = K_CM_BUTSTATE_PRESSING;
						//play a sound on opening the interface
						SND_PLAY(SNDIDX_DENIED);
						//remove icon
						pPlayerActor[plidx]->SetIcon(K_LVL_ACT_ICON_NONE);
					}
					else if ((ctrlr->sCommands.keyState[K_CM_COMMAND_FIRE1] == K_CM_BUTSTATE_JUSTRELEASED) ||
						(ctrlr->sCommands.keyState[K_CM_COMMAND_JUMP] == K_CM_BUTSTATE_JUSTRELEASED))
					{
						int nAbility = m_arrPlayerSelStrategic[plidx];
						//select it
						if (ActivateSpecialAbility(nAbility, plidx))
						{
							//exit 
							m_arrPlayerSelStrategic[plidx] = -1;
							m_interfaceIGM.SetStrategicSelection(plidx, -1);
							//remove icon
							pPlayerActor[plidx]->SetIcon(K_LVL_ACT_ICON_NONE);
							//make sure we disable the tutorial
							g_userData[K_MEMID_TUT_INTERFACE_STRATEGIC] = 1;
						}
						else
						{
							SND_PLAY(SNDIDX_DENIED);
						}
					}
					else if (ctrlr->sCommands.keyState[K_CM_COMMAND_LEFT] == K_CM_BUTSTATE_JUSTPRESSED)
					{
						int nSelectedIdxNew = m_arrPlayerSelStrategic[plidx];
						do {
							nSelectedIdxNew--;
							//rollover
							if (nSelectedIdxNew < 0)
								nSelectedIdxNew = K_LVL_MAX_STRATEGIC_POINTS - 1;
						} while (m_arrStrategicAbilities[plidx][nSelectedIdxNew] < 0);
						CLAMP(nSelectedIdxNew, 0, K_LVL_MAX_STRATEGIC_POINTS - 1);

						SND_PLAY(SNDIDX_CLICK);
						m_arrPlayerSelStrategic[plidx] = nSelectedIdxNew;
						m_interfaceIGM.SetStrategicSelection(plidx, nSelectedIdxNew);
					}
					else if (ctrlr->sCommands.keyState[K_CM_COMMAND_RIGHT] == K_CM_BUTSTATE_JUSTPRESSED)
					{
						int nStatIdx = K_LVL_STATS_PL1_STRATEGIC_POINTS + plidx * K_LVL_STATS_PLAYER_STATS_COUNT;
						int nMaxPoint = (int)floor(m_arrStats[nStatIdx] / 1000.0f);


						int nSelectedIdxNew = m_arrPlayerSelStrategic[plidx];
						do {
							nSelectedIdxNew++;
							//rollover
							if (nSelectedIdxNew >= K_LVL_MAX_STRATEGIC_POINTS)
								nSelectedIdxNew = 0;
						} while (m_arrStrategicAbilities[plidx][nSelectedIdxNew] < 0);
						CLAMP(nSelectedIdxNew, 0, K_LVL_MAX_STRATEGIC_POINTS - 1);

						SND_PLAY(SNDIDX_CLICK);
						m_arrPlayerSelStrategic[plidx] = nSelectedIdxNew;
						m_interfaceIGM.SetStrategicSelection(plidx, nSelectedIdxNew);
					}
				}
				*/
			}

			///--- level targets - mission success accomplished ---
			bool bMissionFinished = true;
			if ((m_arrStats[K_LVL_STATS_LEVEL_HAS_BOMBS] != 0) && (m_arrStats[K_LVL_STATS_BOMBS_DISARMED] == 0))
				bMissionFinished = false;
			if (m_arrStats[K_LVL_STATS_TARGETS_LEFT] > 0)
				bMissionFinished = false;
			//#ZOMBIE: check all portals disabled to finish level
			if ((m_arrStats[K_LVL_STATS_ZOMBIE_PORTALS] > 0) && (m_arrStats[K_LVL_STATS_ZOMBIE_PORTALS_DESTROYED] < m_arrStats[K_LVL_STATS_ZOMBIE_PORTALS]))
				bMissionFinished = false;

			int nStrIdxMissionFailed = -1; //means win if -1 or lose if >=0
			///--- level failed if killed all hostages  ---
			//only fail because of hostages on hostage rescue missions
			if ((m_nLoadedLevelType == K_GAME_LSTYPE_HOSTAGE) &&
				(m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL] > 0) && 
				(m_arrStats[K_LVL_STATS_HOSTAGES_KILLED] >= m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]))
			{
				bMissionFinished = true;
				nStrIdxMissionFailed = STR_HOSTAGES_KILLED;
			}

			///--- LEVEL FAILED when not pressing continue ---
			bool bGaveUp = true;
			bool bPlayerMightContinue = false;
			for (int plidx = 0; plidx < K_MAX_PLAYERS_CNT; plidx++)
			{
				if (pPlayerActor[plidx] != null)
				{
					//there is still a dead player that could continue
					if (pPlayerActor[plidx]->fLife <= 0.0f)
						bPlayerMightContinue = true;
					if (pPlayerActor[plidx]->fLife > 0.0f)
						bGaveUp = false;
					if (m_arrPlayerSelHotJoin[plidx] != -1)
						bGaveUp = false;
				}
			}
			if (bGaveUp)
			{
				bMissionFinished = true;
				nStrIdxMissionFailed = STR_TEAM_KILLED;
			}

			//don't give verdict until all players are really dead
			if (bPlayerMightContinue)
				bMissionFinished = false;
			//mission win? wait for scripts
			if ((bMissionFinished) && (nStrIdxMissionFailed < 0) && (UTGetScriptManager().GetRunningScriptsCount() > 0))
				bMissionFinished = false;
			//on infinite mode you never finish!
			if ((g_gameMode == GAME_MODE_INFINITE_TOWER) && (bMissionFinished == true))
			{
				if (nStrIdxMissionFailed < 0) //success?
					bMissionFinished = false;
				else //fail?
				{
					//only fail if dead
					if (nStrIdxMissionFailed != STR_TEAM_KILLED)
						bMissionFinished = false;
				}
			}

			//is mission finished?
			if (bMissionFinished)
			{
				//make sure we stop all scripts (could generate enemies)
				UTGetScriptManager().StopAllScripts();
				//win or lose?
				if(nStrIdxMissionFailed < 0) //win
					SetLevelState(K_LVL_STATE_MISSION_ACCOMPLISHED);
				else //lose - show why
					SetLevelState(K_LVL_STATE_MISSION_FAILED, nStrIdxMissionFailed);
			}

		}
		break;

		case K_LVL_STATE_MISSION_ACCOMPLISHED:
		{
			//wait for network data
			if (UTGetAppClass().IsGameNetworked())
			{
				g_netlock.Net_UpdateLevelResults(dTime);
				//show net votes
				CCtrlLayer* layer = UTGetControlsManager().GetTopmostInputLayer();
				if (layer)
				{
					CControl* ctrl;
					if (ctrl = layer->GetControlByName("CTRL_NETVOTE_RESTART"))
					{
						ctrl->paramsDict.SetNamedVarINT32(L"leftVote", (g_netlock.m_arrLvlResPeerStates[0] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_RESTART) ? 1 : 0);
						ctrl->paramsDict.SetNamedVarINT32(L"rightVote", (g_netlock.m_arrLvlResPeerStates[1] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_RESTART) ? 1 : 0);
					}
					if (ctrl = layer->GetControlByName("CTRL_NETVOTE_CONTINUE"))
					{
						ctrl->paramsDict.SetNamedVarINT32(L"leftVote", (g_netlock.m_arrLvlResPeerStates[0] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CONTINUE) ? 1 : 0);
						ctrl->paramsDict.SetNamedVarINT32(L"rightVote", (g_netlock.m_arrLvlResPeerStates[1] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CONTINUE) ? 1 : 0);
					}
				}

				///check presses
				//if someone clicked cancel throw us to main menu without error
				if (g_netlock.Net_LevelResultsCountStates(CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CONTINUE) >= K_MAX_PLAYERS_CNT)
				{
					LOG(L"Game::Level results: Players voted to continue!");
					//see if we're hosting the game decide next level (advance)
					if (g_netlock.Net_GetIAmHosting())
					{
						//quick match
						if (UTGetAppClass().m_Settings.devnet_eNetGameType == CApplicationSettings::K_NETGAME_TYPE_QUICK_MATCH)
						{
							//random level on quick match
							int nLevel = GetNextRandomLevel();
							//saving in userData is optional as it gets overwritten anyway from the player selection screen
							g_userData[K_MEMID_SELECTED_CHAPTER] = nLevel / K_GAME_LEVELS_PER_CHAPTER;
							g_userData[K_MEMID_SELECTED_LEVEL] = nLevel % K_GAME_LEVELS_PER_CHAPTER;
							//save in netlock too
							g_netlock.m_ucSelChapter = g_userData[K_MEMID_SELECTED_CHAPTER];
							g_netlock.m_ucSelLevel = g_userData[K_MEMID_SELECTED_LEVEL];

							LOG(L"Game::Level: Decided random chapter(%d) and level(%d).", g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL]);
						}
						else //hosting game
						{
							//on normal coop gets to the next mission but on hosted downloaded content it just plays again
							int nLevel = g_userData[K_MEMID_SELECTED_CHAPTER] * K_GAME_LEVELS_PER_CHAPTER + g_userData[K_MEMID_SELECTED_LEVEL];
							if (g_netlock.m_ucModData == 0)	//not playing custom
							{
								nLevel++;
								if (nLevel >= UTGetChaptersList().GetTotalLevelsCnt())
									nLevel = 0;
							}

							g_userData[K_MEMID_SELECTED_CHAPTER] = nLevel / K_GAME_LEVELS_PER_CHAPTER;
							g_userData[K_MEMID_SELECTED_LEVEL] = nLevel % K_GAME_LEVELS_PER_CHAPTER;
							//save in netlock too
							g_netlock.m_ucSelChapter = g_userData[K_MEMID_SELECTED_CHAPTER];
							g_netlock.m_ucSelLevel = g_userData[K_MEMID_SELECTED_LEVEL];

							LOG(L"Game::Level: Decided next chapter(%d) and level(%d).", g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL]);
						}
					}

					if (!g_bDuringTransition)
					{
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_PLAYER_SELECTION);
						nevent->AddNamedArgINT32(L"arg1", 0); //reset player selection
						nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
						UTGetEventManager().QueueEvent(nevent);
					}
					//clear command
					g_netlock.Net_LevelResultsClearStates();
				}
				if (g_netlock.Net_LevelResultsCountStates(CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_RESTART) >= K_MAX_PLAYERS_CNT)
				{
					LOG(L"Game::Level Win: Players voted to restart the level!");
					//set loading levels
					g_userData[K_MEMID_SELECTED_CHAPTER] = g_netlock.m_ucSelChapter;
					g_userData[K_MEMID_SELECTED_LEVEL] = g_netlock.m_ucSelLevel;

					if (!g_bDuringTransition)
					{
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_PLAYER_SELECTION);
						nevent->AddNamedArgINT32(L"arg1", 0); //reset player selection
						nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
						UTGetEventManager().QueueEvent(nevent);
					}
					//clear command
					g_netlock.Net_LevelResultsClearStates();
				}
				//cancel button / command / window
				if (g_netlock.Net_LevelResultsCountStates(CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CANCEL) > 0)
				{
					LOG(L"Game::Level Win: Player chose to exit!");

					if (!g_bDuringTransition)
					{
						//change game state
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
						nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
						//check and see if other player requested exit and show message if so
						if (g_netlock.m_arrLvlResPeerStates[g_netlock.Net_GetOtherPlayerIndex()] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CANCEL)
							nevent->AddNamedArgINT32(L"stateErrorStrIdx", STR_NETWORK_ERROR_PLAYER_LEFT);

						UTGetEventManager().QueueEvent(nevent);
					}
					//clear command
					g_netlock.Net_LevelResultsClearStates();

					return;
				}
			}

			switch (m_levelSubState)
			{
				case 0: //wait for message to disappear
				{
					m_levelStateTimer += dTime;
					if (m_levelStateTimer > 2.0f)
					{
						m_levelStateTimer = 0.0f;
						m_levelSubState = 1;

						int nLevelIdx = -1;
						if(m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
							nLevelIdx = g_userData[K_MEMID_SELECTED_LEVEL] + g_userData[K_MEMID_SELECTED_CHAPTER] * K_GAME_LEVELS_PER_CHAPTER;
						
						//pregatim strings pentru interfata de level finished
						WCHAR tmpstr[MAX_PATH];
						int nPlayers = m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED] + m_arrStats[K_LVL_STATS_PL2_HAS_PLAYED];
						//--- PL1 data ---
						float fAccuracyP1 = 1.0f;
						if (m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT] > 0)
							fAccuracyP1 = (float)m_arrStats[K_LVL_STATS_PL1_BULLETS_HIT] / (float)m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT];
						CLAMP(fAccuracyP1, 0.0f, 1.0f);
						g_stringsMgr.SetString(STR_MISSION_P1_KILLS, L"%d", m_arrStats[K_LVL_STATS_PL1_KILLS]);
						if (m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT] > 0)
							g_stringsMgr.SetString(STR_MISSION_P1_ACCURACY, L"%.1f%%", 100.0f * fAccuracyP1);
						else
							g_stringsMgr.SetString(STR_MISSION_P1_ACCURACY, L"%s", g_stringsMgr.strings[STR_NOT_AVAILABLE]->sText);
						g_stringsMgr.SetString(STR_MISSION_P1_HOSTAGES, L"%d / %d", m_arrStats[K_LVL_STATS_PL1_HOSTAGES_SAVED], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);
						g_stringsMgr.SetString(STR_MISSION_P1_DEATHS, L"%d", m_arrStats[K_LVL_STATS_PL1_DEATHS]);
						//--- PL2 data ---
						float fAccuracyP2 = 1.0f;
						if (m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT] > 0)
							fAccuracyP2 = (float)m_arrStats[K_LVL_STATS_PL2_BULLETS_HIT] / (float)m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT];
						CLAMP(fAccuracyP2, 0.0f, 1.0f);
						g_stringsMgr.SetString(STR_MISSION_P2_KILLS, L"%d", m_arrStats[K_LVL_STATS_PL2_KILLS]);
						if (m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT] > 0)
							g_stringsMgr.SetString(STR_MISSION_P2_ACCURACY, L"%.1f%%", 100.0f * fAccuracyP2);
						else
							g_stringsMgr.SetString(STR_MISSION_P2_ACCURACY, L"%s", g_stringsMgr.strings[STR_NOT_AVAILABLE]->sText);
						g_stringsMgr.SetString(STR_MISSION_P2_HOSTAGES, L"%d / %d", m_arrStats[K_LVL_STATS_PL2_HOSTAGES_SAVED], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);
						g_stringsMgr.SetString(STR_MISSION_P2_DEATHS, L"%d", m_arrStats[K_LVL_STATS_PL2_DEATHS]);

						//level time
						int nTimeSpent = m_arrStats[K_LVL_STATS_LEVEL_END_SEC] - m_arrStats[K_LVL_STATS_LEVEL_START_SEC];
						//--- calculam stele si XP ---
						int nStars = 3;
						if (m_arrStats[K_LVL_STATS_HOSTAGES_KILLED] > 0)
							nStars--;
						if ((m_arrStats[K_LVL_STATS_PL1_DEATHS] + m_arrStats[K_LVL_STATS_PL2_DEATHS]) > 0)
							nStars--;
						//on arrest warrant missions remove a star per target kill
						if ((m_nLoadedLevelType == K_GAME_LSTYPE_ARREST_WARRANT) && (m_arrStats[K_LVL_STATS_LEVEL_ARREST_TARGETS_KILLED] > 0))
						{
							nStars -= m_arrStats[K_LVL_STATS_LEVEL_ARREST_TARGETS_KILLED];
						}

						CLAMP(nStars, 1, 3);

						//#ACHIEVEMENTS: 3 stars mission on any mission
						if (nStars == 3)
						{
							UTGetAchievementManager().UnlockAchievement(ACH_3STARS_MISSION);
						}

						///--- SCORE ---
						int nTotalLevelScore = nStars * 1500;
						nTotalLevelScore += (int)ceil((float)m_arrStats[K_LVL_STATS_PL1_KILLS] * fAccuracyP1 * 150.0f) + m_arrStats[K_LVL_STATS_PL1_HOSTAGES_SAVED] * 300 - m_arrStats[K_LVL_STATS_PL1_DEATHS] * 200;
						nTotalLevelScore += (int)ceil((float)m_arrStats[K_LVL_STATS_PL2_KILLS] * fAccuracyP2 * 150.0f) + m_arrStats[K_LVL_STATS_PL2_HOSTAGES_SAVED] * 300 - m_arrStats[K_LVL_STATS_PL2_DEATHS] * 200;
						//add civilians score
						nTotalLevelScore += m_arrStats[K_LVL_STATS_CIVILIANS_ARRESTED] * 100;
						nTotalLevelScore -= m_arrStats[K_LVL_STATS_CIVILIANS_KILLED] * 80;
						//lower limit on total XP
						if (nTotalLevelScore < 0)
							nTotalLevelScore = 0;
						//add time bonus
						int timeBonus = (60/*sec*/ * 15/*min*/ - nTimeSpent) * 20;
						if (timeBonus < 0) timeBonus = 0;
						//total XP points
						nTotalLevelScore += timeBonus;

						///--- XP Points ---
						int nXPpl1 = 0, nXPpl2 = 0;
						int nMaxXPPoints = App_GetMaxXP(K_GAME_MAX_UPGRADE_LEVELS);
						int nTotalXPPoints = Local_ComputeMissionXP(nStars);

						//--- STARS WINDOW ---
						OS_FormatTime(tmpstr, MAX_PATH, (float)(nTimeSpent));
						g_stringsMgr.SetString(STR_MISSION_TIME, tmpstr);
						g_stringsMgr.SetString(STR_MISSION_CASUALTIES, L"%d", m_arrStats[K_LVL_STATS_PL1_DEATHS] + m_arrStats[K_LVL_STATS_PL2_DEATHS]);
						g_stringsMgr.SetString(STR_MISSION_SCORE, L"%d", nTotalLevelScore);

						int nHostagesSaved = m_arrStats[K_LVL_STATS_PL1_HOSTAGES_SAVED] + m_arrStats[K_LVL_STATS_PL2_HOSTAGES_SAVED];
						g_stringsMgr.SetString(STR_MISSION_HOSTAGES, L"%d / %d", nHostagesSaved, m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);

						//--- SAVE LEVEL DATA ---
						if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
						{
							g_userData[K_MEMID_STARS_TOTAL] += LIMIT(nStars - g_levelStats[nLevelIdx].nStars, 0, 3);

							g_levelStats[nLevelIdx].nPlayedTimes++;
							if (g_levelStats[nLevelIdx].nStars < nStars)
								g_levelStats[nLevelIdx].nStars = nStars;
						}

						if (nPlayers == 1)
						{
							//only save best score on classic mode
							if ((g_gameMode == GAME_MODE_CLASSIC) && (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE))
							{
								if (g_levelStats[nLevelIdx].nScoreSolo < nTotalLevelScore)
									g_levelStats[nLevelIdx].nScoreSolo = nTotalLevelScore;
								if ((g_levelStats[nLevelIdx].nBestTimeSec_Solo == 0) || (g_levelStats[nLevelIdx].nBestTimeSec_Solo < nTimeSpent))
									g_levelStats[nLevelIdx].nBestTimeSec_Solo = nTimeSpent;
							}
							//XP points	save
							int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
							nXPpl1 = g_userData[nPlBaseIdx];
							inc_limit(g_userData[nPlBaseIdx], nTotalXPPoints, nMaxXPPoints);
						}
						else
						{
							//only save best score on classic mode
							if ((g_gameMode == GAME_MODE_CLASSIC) && (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE))
							{
								if (g_levelStats[nLevelIdx].nScoreCoop < nTotalLevelScore)
									g_levelStats[nLevelIdx].nScoreCoop = nTotalLevelScore;
								if ((g_levelStats[nLevelIdx].nBestTimeSec_Coop == 0) || (g_levelStats[nLevelIdx].nBestTimeSec_Coop < nTimeSpent))
									g_levelStats[nLevelIdx].nBestTimeSec_Coop = nTimeSpent;
							}

							//XP points	save
							if (!UTGetAppClass().IsGameNetworked())
							{
								//in local coop you only get half the XP for each player
								int nPl1BaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
								nXPpl1 = g_userData[nPl1BaseIdx]; //save old value
								inc_limit(g_userData[nPl1BaseIdx], nTotalXPPoints / 2, nMaxXPPoints);
								int nPl2BaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[1].eType;
								nXPpl2 = g_userData[nPl2BaseIdx]; //save old value
								inc_limit(g_userData[nPl2BaseIdx], nTotalXPPoints / 2, nMaxXPPoints);
							}
							else
							{
								//in network games each player gets it's own
								int nMyPlayerBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[g_netlock.Net_GetPlayerIndex()].eType;
								g_userData[nMyPlayerBaseIdx] += nTotalXPPoints;
								CLAMP(g_userData[nMyPlayerBaseIdx], 0, nMaxXPPoints);
							}
						}

						App_SaveUserData();

						//--- show windows and change portraits and title text ---
						UTGetControlsManager().RemoveAllLayers();
						//generic changes
						CCtrlLayer *layer = null;
						if (nPlayers == 1)
							layer = UTGetControlsManager().ShowLayerOnce("LAYER_ID_LEVELWIN_1P");
						else
						{
							if (!UTGetAppClass().IsGameNetworked())
								layer = UTGetControlsManager().ShowLayerOnce("LAYER_ID_LEVELWIN_2P");
							else
								layer = UTGetControlsManager().ShowLayerOnce("LAYER_ID_LEVELWIN_2P_COOP");
						}

						//report score to steam leaderboards
#ifdef ENABLE_LEADERBOARDS
						char pszBoardName[MAX_PATH];
						//only push scores to leaderboards if not playing a downloaded level and not using mods
						if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
						{
#ifdef ENABLE_STEAM
							char strFormat[] = "%s%d.%d";
#endif
#ifdef ENABLE_GALAXY
							char strFormat[] = "%s%d_%d";
#endif

							if (nPlayers == 1)
							{
								StringCchPrintfA(pszBoardName, MAX_PATH, strFormat, K_GAME_STR_LEADERBOARDS_PREFIX_SP, m_nLoadedChapter + 1, m_nLoadedLevel + 1);
							}
							else
							{
								StringCchPrintfA(pszBoardName, MAX_PATH, strFormat, K_GAME_STR_LEADERBOARDS_PREFIX_COOP, m_nLoadedChapter + 1, m_nLoadedLevel + 1);
							}
							//on zombie mode leaderboards have an appendix
							if (g_gameMode == GAME_MODE_ZOMBIE_INVASION)
								StringCchCatA(pszBoardName, MAX_PATH, "_zm");
							//reset old scores
							UTGetLeaderboards().ResetScoresList();
							//reset strings too
							g_stringsMgr.SetString(STR_LEADERBOARDS_NAMES_VAL, L"...");
							g_stringsMgr.SetString(STR_LEADERBOARDS_SCORES_VAL, L"...");
							g_stringsMgr.SetString(STR_LEADERBOARDS_PLAYERSCORE_VAL, L"...");
							//now upload score
							UTGetLeaderboards().QueueJob(K_JOB_UPLOAD_SCORE, pszBoardName, nTotalLevelScore);
							//request downloading of scores
							UTGetLeaderboards().QueueJob(K_JOB_GET_SCORES_AROUND_USER, pszBoardName);
							//request downloading of your own score - only if needed (when leaderboards don't update instantly)
							//UTGetLeaderboards().QueueJob(K_JOB_GET_SCORE_FOR_CURRENT_USER, pszBoardName, 0);
						}
#endif

						CControl* ctrl = null;
						if (layer != null)
						{
							if (ctrl = layer->GetControlByName("CTRL_STARS"))
							{
								ctrl->paramsDict.SetNamedVarINT32(L"nStars", nStars);
							}
							//red labels for conditions that aren't satisfied						   
							if (m_arrStats[K_LVL_STATS_HOSTAGES_KILLED] > 0)
							{
								if (ctrl = layer->GetControlByName("LABEL_HOSTAGES"))
								{
									ctrl->paramsDict.SetNamedVarString(L"FontColor", L"0xffff0000");
								}
							}
							if (m_arrStats[K_LVL_STATS_PL1_DEATHS] + m_arrStats[K_LVL_STATS_PL2_DEATHS] > 0)
							{
								if (ctrl = layer->GetControlByName("LABEL_CASUALTIES"))
								{
									ctrl->paramsDict.SetNamedVarString(L"FontColor", L"0xffff0000");
								}
							}

							//on custom downloaded levels hide the MELEE-leaderboards 
							if (m_unLoadedLevelFlags & K_LVL_LEVEL_FLAG_DOWNLOADED)
							{
								if (ctrl = layer->GetControlByName("LABEL_LEADERBOARDS"))
									ctrl->paramsDict.SetNamedVarString(L"FontColor", L"0x00000000");
							}

							if (nPlayers == 1)
							{
								CControl* ctrl = null;
								if (layer != null)
								{
									//portrete								
									if (ctrl = layer->GetControlByName("CTRL_ANIM_PORTRAIT_PL1"))
									{
										ctrl->paramsDict.SetNamedVarINT32(L"setFrame", (int)g_playerSelScr.m_arrPlayers[0].eType);
									}
									//XP bar
									if (ctrl = layer->GetControlByName("CTRL_XPBAR_PL1"))
									{
										int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
										ctrl->paramsDict.SetNamedVarINT32(L"nOldValue", nXPpl1);
										ctrl->paramsDict.SetNamedVarINT32(L"nNewValue", g_userData[nPlBaseIdx]);
									}
								}

								if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
								{
									CHAR ctxt[MAX_PATH];
									StringCchPrintfA(ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1);
									ANALYTICS_EVENT("level_win_1p", ctxt, "durationSec", nTimeSpent);
								}
							}
							else //2 players
							{
								CControl* ctrl = null;
								if (layer != null)
								{
									//portrete								
									if (ctrl = layer->GetControlByName("CTRL_ANIM_PORTRAIT_PL1"))
									{
										ctrl->paramsDict.SetNamedVarINT32(L"setFrame", (int)g_playerSelScr.m_arrPlayers[0].eType);
									}
									if (ctrl = layer->GetControlByName("CTRL_ANIM_PORTRAIT_PL2"))
									{
										ctrl->paramsDict.SetNamedVarINT32(L"setFrame", (int)g_playerSelScr.m_arrPlayers[1].eType);
									}
								}

								//network - replace player names with real ones
								if (UTGetAppClass().IsGameNetworked())
								{
									if (ctrl = layer->GetControlByName("CTRL_WND_PL1"))
									{
										ctrl->paramsDict.SetNamedVarINT32(L"stringID", STR_NETWORK_HOST_NAME);
									}
									if (ctrl = layer->GetControlByName("CTRL_WND_PL2"))
									{
										ctrl->paramsDict.SetNamedVarINT32(L"stringID", STR_NETWORK_PEER_NAME);
									}

									if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
									{
										CHAR ctxt[MAX_PATH];
										StringCchPrintfA(ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1);
										ANALYTICS_EVENT("level_win_2p_net", ctxt, "durationSec", nTimeSpent);
									}
									//XP bar - networked
									if (ctrl = layer->GetControlByName("CTRL_XPBAR_PL1"))
									{
										ctrl->paramsDict.SetNamedVarINT32(L"nOldValue", g_playerSelScr.m_arrPlayers[0].nPlayerXPPts);
										int nNew = LIMIT(g_playerSelScr.m_arrPlayers[0].nPlayerXPPts + nTotalXPPoints, 0, nMaxXPPoints);
										ctrl->paramsDict.SetNamedVarINT32(L"nNewValue", nNew);
									}
									if (ctrl = layer->GetControlByName("CTRL_XPBAR_PL2"))
									{
										ctrl->paramsDict.SetNamedVarINT32(L"nOldValue", g_playerSelScr.m_arrPlayers[1].nPlayerXPPts);
										int nNew = LIMIT(g_playerSelScr.m_arrPlayers[1].nPlayerXPPts + nTotalXPPoints, 0, nMaxXPPoints);
										ctrl->paramsDict.SetNamedVarINT32(L"nNewValue", nNew);
									}
								}
								else
								{
									if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
									{
										CHAR ctxt[MAX_PATH];
										StringCchPrintfA(ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1);
										ANALYTICS_EVENT("level_win_2p", ctxt, "durationSec", nTimeSpent);
									}
									//XP bar
									if (ctrl = layer->GetControlByName("CTRL_XPBAR_PL1"))
									{
										int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
										ctrl->paramsDict.SetNamedVarINT32(L"nOldValue", nXPpl1);
										ctrl->paramsDict.SetNamedVarINT32(L"nNewValue", g_userData[nPlBaseIdx]);
									}
									if (ctrl = layer->GetControlByName("CTRL_XPBAR_PL2"))
									{
										int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[1].eType;
										ctrl->paramsDict.SetNamedVarINT32(L"nOldValue", nXPpl2);
										ctrl->paramsDict.SetNamedVarINT32(L"nNewValue", g_userData[nPlBaseIdx]);
									}
								}
							}
						}

						//#WEEKLY: win weekly challenges
						if (g_userData[K_MEMID_SELECTED_CHAPTER] == K_GAME_WEEKLY_CHALLENGE_CHAPTER_NO)
						{
							int nLevel = g_userData[K_MEMID_SELECTED_LEVEL];
							bool bTask1OK = false;
							bool bTask2OK = false;
							switch (nLevel)
							{
								case 0:
								{
									if (nTimeSpent <= 180) //under 3 min
									{
										g_userData[K_MEMID_WEEKLY1_TASKS_FLAGS] |= 1;
										bTask1OK = true;
									}
									//show tasklist
									if (layer != null)
									{
										CControl* ctrl = null;
										if (ctrl = layer->GetControlByName("CTRL_TASKLIST_WEEKLY"))
										{
											ctrl->paramsDict.SetNamedVarINT32(L"nIconFrame1", ((bTask1OK) ? 1 : 0));
											ctrl->paramsDict.SetNamedVarString(L"stringID1", STRID_WEEK1_TASK);
										}
									}
								}
								break;
								case 1:
								{
									if (nTimeSpent <= 240) //under 4 min
									{
										g_userData[K_MEMID_WEEKLY2_TASKS_FLAGS] |= 1;
										bTask1OK = true;
									}
									if (m_arrStats[K_LVL_STATS_LEVEL_SNIPER_VICTIMS] == 0)
									{
										g_userData[K_MEMID_WEEKLY2_TASKS_FLAGS] |= 2;
										bTask2OK = true;
									}
									//show tasklist
									if (layer != null)
									{
										CControl* ctrl = null;
										if (ctrl = layer->GetControlByName("CTRL_TASKLIST_WEEKLY"))
										{
											ctrl->paramsDict.SetNamedVarINT32(L"nIconFrame1", ((bTask1OK) ? 1 : 0));
											ctrl->paramsDict.SetNamedVarString(L"stringID1", STRID_WEEK2_TASK1);
											ctrl->paramsDict.SetNamedVarINT32(L"nIconFrame2", ((bTask2OK) ? 1 : 0));
											ctrl->paramsDict.SetNamedVarString(L"stringID2", STRID_WEEK2_TASK2);
										}
									}
								}
								break;
								case 2: //week 3 - zombie - no snipers
								{
									if (m_arrStats[K_LVL_STATS_LEVEL_SNIPER_VICTIMS] == 0)
									{
										g_userData[K_MEMID_WEEKLY3_TASKS_FLAGS] |= 1;
										bTask1OK = true;
									}
									//show tasklist
									if (layer != null)
									{
										CControl* ctrl = null;
										if (ctrl = layer->GetControlByName("CTRL_TASKLIST_WEEKLY"))
										{
											ctrl->paramsDict.SetNamedVarINT32(L"nIconFrame1", ((bTask1OK) ? 1 : 0));
											ctrl->paramsDict.SetNamedVarString(L"stringID1", STRID_WEEK2_TASK2); //no snipers
										}
									}
								}
								break;
								case 3: //week 4 - no health, win with shield
								{
									if (m_arrStats[K_LVL_STATS_LEVEL_HEALTH_BOXES_USED] == 0)
									{
										g_userData[K_MEMID_WEEKLY4_TASKS_FLAGS] |= 1;
										bTask1OK = true;
									}

									if (((m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED] != 0) && (g_playerSelScr.m_arrPlayers[0].eType == K_PSS_CLASS_SHIELD)) ||
										((m_arrStats[K_LVL_STATS_PL2_HAS_PLAYED] != 0) && (g_playerSelScr.m_arrPlayers[1].eType == K_PSS_CLASS_SHIELD)) )
									{
										g_userData[K_MEMID_WEEKLY4_TASKS_FLAGS] |= 2;
										bTask2OK = true;
									}
									//show tasklist
									if (layer != null)
									{
										CControl* ctrl = null;
										if (ctrl = layer->GetControlByName("CTRL_TASKLIST_WEEKLY"))
										{
											ctrl->paramsDict.SetNamedVarINT32(L"nIconFrame1", ((bTask1OK) ? 1 : 0));
											ctrl->paramsDict.SetNamedVarString(L"stringID1", STRID_WEEK4_TASK1);
											ctrl->paramsDict.SetNamedVarINT32(L"nIconFrame2", ((bTask2OK) ? 1 : 0));
											ctrl->paramsDict.SetNamedVarString(L"stringID2", STRID_WEEK4_TASK2);
										}
									}
								}
								break;
								case 4: //week 5 - under 3 min, as breacher
								{
									if (nTimeSpent <= 180) //under 3 min
									{
										g_userData[K_MEMID_WEEKLY5_TASKS_FLAGS] |= 1;
										bTask1OK = true;
									}
									if (((m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED] != 0) && (g_playerSelScr.m_arrPlayers[0].eType == K_PSS_CLASS_BREACHER)) ||
										((m_arrStats[K_LVL_STATS_PL2_HAS_PLAYED] != 0) && (g_playerSelScr.m_arrPlayers[1].eType == K_PSS_CLASS_BREACHER)))
									{
										g_userData[K_MEMID_WEEKLY5_TASKS_FLAGS] |= 2;
										bTask2OK = true;
									}
									//show tasklist
									if (layer != null)
									{
										CControl* ctrl = null;
										if (ctrl = layer->GetControlByName("CTRL_TASKLIST_WEEKLY"))
										{
											ctrl->paramsDict.SetNamedVarINT32(L"nIconFrame1", ((bTask1OK) ? 1 : 0));
											ctrl->paramsDict.SetNamedVarString(L"stringID1", STRID_WEEK1_TASK);
											ctrl->paramsDict.SetNamedVarINT32(L"nIconFrame2", ((bTask2OK) ? 1 : 0));
											ctrl->paramsDict.SetNamedVarString(L"stringID2", STRID_WEEK5_TASK1);
										}
									}
								}
								break;
								case 5: //week 6 - win as fergie
								{
									if (((pPlayerActor[0] != null) && (g_playerSelScr.m_arrPlayers[pPlayerActor[0]->nPlayerOrdinal].eType == K_PSS_CLASS_FBI_AGENT)) ||
										((pPlayerActor[1] != null) && (g_playerSelScr.m_arrPlayers[pPlayerActor[1]->nPlayerOrdinal].eType == K_PSS_CLASS_FBI_AGENT)))
									{
										g_userData[K_MEMID_WEEKLY6_TASKS_FLAGS] |= 1;
										bTask1OK = true;
									}
									//show tasklist
									if (layer != null)
									{
										CControl* ctrl = null;
										if (ctrl = layer->GetControlByName("CTRL_TASKLIST_WEEKLY"))
										{
											ctrl->paramsDict.SetNamedVarINT32(L"nIconFrame1", ((bTask1OK) ? 1 : 0));
											ctrl->paramsDict.SetNamedVarString(L"stringID1", STRID_WEEK6_TASK1);
										}
									}
								}
								break;
								case 6: //week 7 - win as assaulter
								{
									if (((m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED] != 0) && (g_playerSelScr.m_arrPlayers[0].eType == K_PSS_CLASS_ASSAULTER)) ||
										((m_arrStats[K_LVL_STATS_PL2_HAS_PLAYED] != 0) && (g_playerSelScr.m_arrPlayers[1].eType == K_PSS_CLASS_ASSAULTER)))
									{
										g_userData[K_MEMID_WEEKLY7_TASKS_FLAGS] |= 1;
										bTask1OK = true;
									}
									//show tasklist
									if (layer != null)
									{
										CControl* ctrl = null;
										if (ctrl = layer->GetControlByName("CTRL_TASKLIST_WEEKLY"))
										{
											ctrl->paramsDict.SetNamedVarINT32(L"nIconFrame1", ((bTask1OK) ? 1 : 0));
											ctrl->paramsDict.SetNamedVarString(L"stringID1", STRID_WEEK7_TASK1);
										}
									}
								}
								break;
								case 7: //week 8 - no healing, no snipers
								{
									if (m_arrStats[K_LVL_STATS_LEVEL_HEALTH_BOXES_USED] == 0)
									{
										g_userData[K_MEMID_WEEKLY8_TASKS_FLAGS] |= 1;
										bTask1OK = true;
									}
									if (m_arrStats[K_LVL_STATS_LEVEL_SNIPER_VICTIMS] == 0)
									{
										g_userData[K_MEMID_WEEKLY8_TASKS_FLAGS] |= 2;
										bTask2OK = true;
									}
									//show tasklist
									if (layer != null)
									{
										CControl* ctrl = null;
										if (ctrl = layer->GetControlByName("CTRL_TASKLIST_WEEKLY"))
										{
											ctrl->paramsDict.SetNamedVarINT32(L"nIconFrame1", ((bTask1OK) ? 1 : 0));
											ctrl->paramsDict.SetNamedVarString(L"stringID1", STRID_WEEK4_TASK1); //no healing
											ctrl->paramsDict.SetNamedVarINT32(L"nIconFrame2", ((bTask2OK) ? 1 : 0));
											ctrl->paramsDict.SetNamedVarString(L"stringID2", STRID_WEEK2_TASK2); //no snipers
										}
									}
								}
								break;
								case 8: //week 9 - win as assaulter, no snipers
								{
									if (((m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED] != 0) && (g_playerSelScr.m_arrPlayers[0].eType == K_PSS_CLASS_ASSAULTER)) ||
										((m_arrStats[K_LVL_STATS_PL2_HAS_PLAYED] != 0) && (g_playerSelScr.m_arrPlayers[1].eType == K_PSS_CLASS_ASSAULTER)))
									{
										g_userData[K_MEMID_WEEKLY9_TASKS_FLAGS] |= 1;
										bTask1OK = true;
									}
									if (m_arrStats[K_LVL_STATS_LEVEL_SNIPER_VICTIMS] == 0)
									{
										g_userData[K_MEMID_WEEKLY9_TASKS_FLAGS] |= 2;
										bTask2OK = true;
									}
									//show tasklist
									if (layer != null)
									{
										CControl* ctrl = null;
										if (ctrl = layer->GetControlByName("CTRL_TASKLIST_WEEKLY"))
										{
											ctrl->paramsDict.SetNamedVarINT32(L"nIconFrame1", ((bTask1OK) ? 1 : 0));
											ctrl->paramsDict.SetNamedVarString(L"stringID1", STRID_WEEK7_TASK1);
											ctrl->paramsDict.SetNamedVarINT32(L"nIconFrame2", ((bTask2OK) ? 1 : 0));
											ctrl->paramsDict.SetNamedVarString(L"stringID2", STRID_WEEK2_TASK2); //no snipers
										}
									}
								}
								break;
								case 9: //week 10 - win as recon
								{
									if (((m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED] != 0) && (g_playerSelScr.m_arrPlayers[0].eType == K_PSS_CLASS_RECON)) ||
										((m_arrStats[K_LVL_STATS_PL2_HAS_PLAYED] != 0) && (g_playerSelScr.m_arrPlayers[1].eType == K_PSS_CLASS_RECON)))
									{
										g_userData[K_MEMID_WEEKLY10_TASKS_FLAGS] |= 1;
										bTask1OK = true;
									}
									//show tasklist
									if (layer != null)
									{
										CControl* ctrl = null;
										if (ctrl = layer->GetControlByName("CTRL_TASKLIST_WEEKLY"))
										{
											ctrl->paramsDict.SetNamedVarINT32(L"nIconFrame1", ((bTask1OK) ? 1 : 0));
											ctrl->paramsDict.SetNamedVarString(L"stringID1", STRID_WEEK10_TASK1);
										}
									}
								}
								break;
								case 10: //week 11 - win as fergie
								{
									if (((m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED] != 0) && (g_playerSelScr.m_arrPlayers[0].eType == K_PSS_CLASS_FBI_AGENT)) ||
										((m_arrStats[K_LVL_STATS_PL2_HAS_PLAYED] != 0) && (g_playerSelScr.m_arrPlayers[1].eType == K_PSS_CLASS_FBI_AGENT)))
									{
										g_userData[K_MEMID_WEEKLY11_TASKS_FLAGS] |= 1;
										bTask1OK = true;
									}
									//show tasklist
									if (layer != null)
									{
										CControl* ctrl = null;
										if (ctrl = layer->GetControlByName("CTRL_TASKLIST_WEEKLY"))
										{
											ctrl->paramsDict.SetNamedVarINT32(L"nIconFrame1", ((bTask1OK) ? 1 : 0));
											ctrl->paramsDict.SetNamedVarString(L"stringID1", STRID_WEEK6_TASK1);
										}
									}
								}
								break;
								case 11:
								{
									if (nTimeSpent <= 180) //under 3 min
									{
										g_userData[K_MEMID_WEEKLY12_TASKS_FLAGS] |= 1;
										bTask1OK = true;
									}
									//show tasklist
									if (layer != null)
									{
										CControl* ctrl = null;
										if (ctrl = layer->GetControlByName("CTRL_TASKLIST_WEEKLY"))
										{
											ctrl->paramsDict.SetNamedVarINT32(L"nIconFrame1", ((bTask1OK) ? 1 : 0));
											ctrl->paramsDict.SetNamedVarString(L"stringID1", STRID_WEEK1_TASK);
										}
									}
								}
								break;
							}
							//save user data again after setting level data
							App_SaveUserData();
						}

						// notify level finished for achievements
						if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)						
							UTGetAppClass().App_OnLevelFinished(g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL]);
					}
				}
				break;
				default:
				{
#ifdef ENABLE_LEADERBOARDS
					//show leaderboard when pressing melee key (any controller)
					if ((UTGetCtrlrMgr().KeyPressed(K_CM_COMMAND_MELEE)) && (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE))
					{
						CCtrlLayer* lay = UTGetControlsManager().GetLayerByName("LAYER_ID_LEADERBOARDS_IGM");
						if (lay == null)
						{
							//show layer
							lay = UTGetControlsManager().ShowLayerOnce("LAYER_ID_LEADERBOARDS_IGM");
							if (lay)
							{
								CControl* ctrl = null;
								//change label that tells type of leaderboard that is shown
								if (ctrl = lay->GetControlByName("LABEL_LBTYPE"))
								{
									int nPlayers = m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED] + m_arrStats[K_LVL_STATS_PL2_HAS_PLAYED];
									if(nPlayers == 1)
										ctrl->paramsDict.SetNamedVarINT32(L"stringID", STR_SINGLE_PLAYER);
									else
										ctrl->paramsDict.SetNamedVarINT32(L"stringID", STR_COOP_ONLINE);
									//level name in STR_TEMP10
									int nChapter = g_userData[K_MEMID_SELECTED_CHAPTER];
									int nLevel = g_userData[K_MEMID_SELECTED_LEVEL];
									int nStrIdxLevelName = UTGetChaptersList().m_arrChapters[nChapter]->arrLevelNameStrIdx[nLevel];
									if(nStrIdxLevelName >= 0)
										g_stringsMgr.SetString(STR_TEMP10, L"%d.%d %s", nChapter + 1, nLevel + 1, g_stringsMgr.strings[nStrIdxLevelName]->sText);
									else
										g_stringsMgr.SetString(STR_TEMP10, L"%d.%d", nChapter + 1, nLevel + 1);
								}
								//set player selection
								ctrl = lay->GetControlByName("CTRL_SCORESLIST_TT");
								if (ctrl != null)
								{
									int nPlIdx = UTGetLeaderboards().GetDownloadedScores_PlayerIndex();
									ctrl->paramsDict.SetNamedVarINT32(L"nSelectedIdx", nPlIdx);
									ctrl->paramsDict.SetNamedVarINT32(L"nOptionsCnt", UTGetLeaderboards().GetDownloadedScoresCount());
#ifndef ENABLE_LEADERBOARDS_NAMES_SELECTION
									ctrl->bCanHaveFocus = false;
									ctrl->paramsDict.SetNamedVarBool(L"bUserCanSelect", false);
#endif
								}
							}
						}
					}
#endif
				}
				break;
			}
		}
		break;

		case K_LVL_STATE_MISSION_FAILED:
		{
			//wait for network data
			if (UTGetAppClass().IsGameNetworked())
			{
				g_netlock.Net_UpdateLevelResults(dTime);
				//show net votes
				CCtrlLayer* layer = UTGetControlsManager().GetTopmostInputLayer();
				if (layer)
				{
					CControl* ctrl;
					//vote restart level
					if (ctrl = layer->GetControlByName("CTRL_NETVOTE_RESTART"))
					{
						ctrl->paramsDict.SetNamedVarINT32(L"leftVote", (g_netlock.m_arrLvlResPeerStates[0] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_RESTART) ? 1 : 0);
						ctrl->paramsDict.SetNamedVarINT32(L"rightVote", (g_netlock.m_arrLvlResPeerStates[1] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_RESTART) ? 1 : 0);
					}
					//vote continue to next level
					if (ctrl = layer->GetControlByName("CTRL_NETVOTE_CONTINUE"))
					{
						ctrl->paramsDict.SetNamedVarINT32(L"leftVote", (g_netlock.m_arrLvlResPeerStates[0] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CONTINUE) ? 1 : 0);
						ctrl->paramsDict.SetNamedVarINT32(L"rightVote", (g_netlock.m_arrLvlResPeerStates[1] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CONTINUE) ? 1 : 0);
					}
				}
				///check presses
				//if someone clicked cancel throw us to main menu without error
				if (g_netlock.Net_LevelResultsCountStates(CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CONTINUE) >= K_MAX_PLAYERS_CNT)
				{
					LOG(L"Game::Level results: Players voted to continue!");
					//see if we're hosting the game decide next level (advance)
					if (g_netlock.Net_GetIAmHosting())
					{
						//quick match
						if (UTGetAppClass().m_Settings.devnet_eNetGameType == CApplicationSettings::K_NETGAME_TYPE_QUICK_MATCH)
						{
							//random level on quick match
							int nLevel = GetNextRandomLevel();
							//saving in userData is optional as it gets overwritten anyway from the player selection screen
							g_userData[K_MEMID_SELECTED_CHAPTER] = nLevel / K_GAME_LEVELS_PER_CHAPTER;
							g_userData[K_MEMID_SELECTED_LEVEL] = nLevel % K_GAME_LEVELS_PER_CHAPTER;
							//save in netlock too
							g_netlock.m_ucSelChapter = g_userData[K_MEMID_SELECTED_CHAPTER];
							g_netlock.m_ucSelLevel = g_userData[K_MEMID_SELECTED_LEVEL];

							LOG(L"Game::Level: Decided random chapter(%d) and level(%d).", g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL]);
						}
						else //hosting game
						{
							int nLevel = g_userData[K_MEMID_SELECTED_CHAPTER] * K_GAME_LEVELS_PER_CHAPTER + g_userData[K_MEMID_SELECTED_LEVEL];
							nLevel++;
							if (nLevel >= UTGetChaptersList().GetTotalLevelsCnt())
								nLevel = 0;

							g_userData[K_MEMID_SELECTED_CHAPTER] = nLevel / K_GAME_LEVELS_PER_CHAPTER;
							g_userData[K_MEMID_SELECTED_LEVEL] = nLevel % K_GAME_LEVELS_PER_CHAPTER;
							//save in netlock too
							g_netlock.m_ucSelChapter = g_userData[K_MEMID_SELECTED_CHAPTER];
							g_netlock.m_ucSelLevel = g_userData[K_MEMID_SELECTED_LEVEL];

							LOG(L"Game::Level: Decided next chapter(%d) and level(%d).", g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL]);
						}
					}

					if (!g_bDuringTransition)
					{
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_PLAYER_SELECTION);
						nevent->AddNamedArgINT32(L"arg1", 0); //reset player selection
						nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
						UTGetEventManager().QueueEvent(nevent);
					}
					//clear command
					g_netlock.Net_LevelResultsClearStates();
				}
				if (g_netlock.Net_LevelResultsCountStates(CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_RESTART) >= K_MAX_PLAYERS_CNT)
				{
					LOG(L"Game::Level results: Players voted to restart the level!");
					//set loading levels
					g_userData[K_MEMID_SELECTED_CHAPTER] = g_netlock.m_ucSelChapter;
					g_userData[K_MEMID_SELECTED_LEVEL] = g_netlock.m_ucSelLevel;

					if (!g_bDuringTransition)
					{
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_PLAYER_SELECTION);
						nevent->AddNamedArgINT32(L"arg1", 0); //reset player selection
						nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
						UTGetEventManager().QueueEvent(nevent);
					}
					//clear command
					g_netlock.Net_LevelResultsClearStates();
				}
			
				//if someone clicked cancel throw us to main menu without error
				if (g_netlock.Net_LevelResultsCountStates(CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CANCEL) > 0)
				{
					LOG(L"Game::Level results: Peer left the game! Quit lobby!");

					if (!g_bDuringTransition)
					{
						//change game state
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
						nevent->AddNamedArgINT32(L"transitionType", K_TRANSITION_TYPE_SIMPLE);
						//check and see if other player requested exit and show message if so
						if (g_netlock.m_arrLvlResPeerStates[g_netlock.Net_GetOtherPlayerIndex()] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CANCEL)
							nevent->AddNamedArgINT32(L"stateErrorStrIdx", STR_NETWORK_ERROR_PLAYER_LEFT);

						UTGetEventManager().QueueEvent(nevent);
					}
					//clear command
					g_netlock.Net_LevelResultsClearStates();

					return;
				}
			}

			switch (m_levelSubState)
			{
				case 0: //wait for the level failed message to go away
				{
					m_levelStateTimer += dTime;
					if (m_levelStateTimer > 2.0f)
					{
						m_levelStateTimer = 0.0f;
						m_levelSubState = 1;

						//pregatim strings pentru interfata de level finished
						WCHAR tmpstr[MAX_PATH];
						int nPlayers = m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED] + m_arrStats[K_LVL_STATS_PL2_HAS_PLAYED];
						//--- PL1 data ---
						float fAccuracyP1 = 1.0f;
						if (m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT] > 0)
							fAccuracyP1 = (float)m_arrStats[K_LVL_STATS_PL1_BULLETS_HIT] / (float)m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT];
						CLAMP(fAccuracyP1, 0.0f, 1.0f);
						g_stringsMgr.SetString(STR_MISSION_P1_KILLS, L"%d", m_arrStats[K_LVL_STATS_PL1_KILLS]);
						if (m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT] > 0)
							g_stringsMgr.SetString(STR_MISSION_P1_ACCURACY, L"%.1f%%", 100.0f * fAccuracyP1);
						else
							g_stringsMgr.SetString(STR_MISSION_P1_ACCURACY, L"%s", g_stringsMgr.strings[STR_NOT_AVAILABLE]->sText);
						g_stringsMgr.SetString(STR_MISSION_P1_HOSTAGES, L"%d / %d", m_arrStats[K_LVL_STATS_PL1_HOSTAGES_SAVED], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);
						g_stringsMgr.SetString(STR_MISSION_P1_DEATHS, L"%d", m_arrStats[K_LVL_STATS_PL1_DEATHS]);
						//--- PL2 data ---
						float fAccuracyP2 = 1.0f;
						if (m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT] > 0)
							fAccuracyP2 = (float)m_arrStats[K_LVL_STATS_PL2_BULLETS_HIT] / (float)m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT];
						CLAMP(fAccuracyP2, 0.0f, 1.0f);
						g_stringsMgr.SetString(STR_MISSION_P2_KILLS, L"%d", m_arrStats[K_LVL_STATS_PL2_KILLS]);
						if (m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT] > 0)
							g_stringsMgr.SetString(STR_MISSION_P2_ACCURACY, L"%.1f%%", 100.0f * fAccuracyP2);
						else
							g_stringsMgr.SetString(STR_MISSION_P2_ACCURACY, L"%s", g_stringsMgr.strings[STR_NOT_AVAILABLE]->sText);
						g_stringsMgr.SetString(STR_MISSION_P2_HOSTAGES, L"%d / %d", m_arrStats[K_LVL_STATS_PL2_HOSTAGES_SAVED], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);
						g_stringsMgr.SetString(STR_MISSION_P2_DEATHS, L"%d", m_arrStats[K_LVL_STATS_PL2_DEATHS]);

						///--- XP Points ---
						int nXPpl1 = 0, nXPpl2 = 0;
						int nMaxXPPoints = App_GetMaxXP(K_GAME_MAX_UPGRADE_LEVELS);
						int nTotalXPPoints = Local_ComputeMissionXP(0);

						//--- STARS WINDOW ---
						int nTimeSpent = m_arrStats[K_LVL_STATS_LEVEL_END_SEC] - m_arrStats[K_LVL_STATS_LEVEL_START_SEC];
						OS_FormatTime(tmpstr, MAX_PATH, (float)(nTimeSpent));
						g_stringsMgr.SetString(STR_MISSION_TIME, tmpstr);

						//--- SAVE LEVEL DATA ---
						// not playing downloaded levels so save played times counter
						if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
						{
							int nLevelIdx = g_userData[K_MEMID_SELECTED_LEVEL] + g_userData[K_MEMID_SELECTED_CHAPTER] * K_GAME_LEVELS_PER_CHAPTER;
							g_levelStats[nLevelIdx].nPlayedTimes++;
						}

						if (nPlayers == 1)
						{
							//XP points	save
							int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
							nXPpl1 = g_userData[nPlBaseIdx];
							inc_limit(g_userData[nPlBaseIdx], nTotalXPPoints, nMaxXPPoints);
						}
						else
						{
							//XP points	save
							if (!UTGetAppClass().IsGameNetworked())
							{
								//in local coop you only get half the XP for each player
								int nPl1BaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
								nXPpl1 = g_userData[nPl1BaseIdx]; //save old value
								inc_limit(g_userData[nPl1BaseIdx], nTotalXPPoints / 2, nMaxXPPoints);
								int nPl2BaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[1].eType;
								nXPpl2 = g_userData[nPl2BaseIdx]; //save old value
								inc_limit(g_userData[nPl2BaseIdx], nTotalXPPoints / 2, nMaxXPPoints);
							}
							else
							{
								//in network games each player gets it's own
								int nMyPlayerBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[g_netlock.Net_GetPlayerIndex()].eType;
								g_userData[nMyPlayerBaseIdx] += nTotalXPPoints;
								CLAMP(g_userData[nMyPlayerBaseIdx], 0, nMaxXPPoints);
							}
						}

						App_SaveUserData();

						//report score to steam leaderboards for infinite mode
#ifdef ENABLE_LEADERBOARDS
						if (m_unLoadedLevelFlags & K_LVL_LEVEL_FLAG_VINFINITE_MODE)
						{
							char pszBoardName[MAX_PATH];
							if (nPlayers == 1)
							{
								StringCchPrintfA(pszBoardName, MAX_PATH, "%s", K_GAME_STR_LEADERBOARDS_VINFINITE_PREFIX_SP);
							}
							else
							{
								StringCchPrintfA(pszBoardName, MAX_PATH, "%s", K_GAME_STR_LEADERBOARDS_VINFINITE_PREFIX_COOP);
							}
							//on zombie mode leaderboards have an appendix
							if (g_gameMode == GAME_MODE_ZOMBIE_INVASION)
								StringCchCatA(pszBoardName, MAX_PATH, "_zm");
							//reset old scores
							UTGetLeaderboards().ResetScoresList();
							//reset strings too
							g_stringsMgr.SetString(STR_LEADERBOARDS_NAMES_VAL, L"...");
							g_stringsMgr.SetString(STR_LEADERBOARDS_SCORES_VAL, L"...");
							g_stringsMgr.SetString(STR_LEADERBOARDS_PLAYERSCORE_VAL, L"...");
							//now upload value
							UTGetLeaderboards().QueueJob(K_JOB_UPLOAD_SCORE, pszBoardName, m_arrStats[K_LVL_STATS_LEVEL_VINFINITE_FLOOR]);
							//request downloading of scores
							UTGetLeaderboards().QueueJob(K_JOB_GET_SCORES_GLOBAL, pszBoardName, 1);
							//request downloading of your own score
							UTGetLeaderboards().QueueJob(K_JOB_GET_SCORE_FOR_CURRENT_USER, pszBoardName, 0);
						}
#endif

						//--- show windows and change portraits and title text ---
						if (nPlayers == 1)
						{
							UTGetControlsManager().RemoveAllLayers();
							CCtrlLayer* layer = UTGetControlsManager().ShowLayerOnce("LAYER_ID_LEVELFAIL_1P");
							if (layer != null)
							{
								CControl* ctrl = layer->GetControlByName("CTRL_STARS");
								if (ctrl)
								{
									ctrl->paramsDict.SetNamedVarINT32(L"nStars", 0);
								}
								//reason why
								if (m_levelStateParam > 0) //if set
								{
									ctrl = layer->GetControlByName("BLINKER_REASON");
									if (ctrl)
									{
										ctrl->paramsDict.SetNamedVarINT32(L"stringID", m_levelStateParam);
									}
								}
								//portrete								
								ctrl = layer->GetControlByName("CTRL_ANIM_PORTRAIT_PL1");
								if (ctrl)
								{
									ctrl->paramsDict.SetNamedVarINT32(L"setFrame", (int)g_playerSelScr.m_arrPlayers[0].eType);
								}
								//XP bar
								if (ctrl = layer->GetControlByName("CTRL_XPBAR_PL1"))
								{
									int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
									ctrl->paramsDict.SetNamedVarINT32(L"nOldValue", nXPpl1);
									ctrl->paramsDict.SetNamedVarINT32(L"nNewValue", g_userData[nPlBaseIdx]);
								}
							}

							//show "melee = leaderboards" on tower mode and reached level
							if (m_unLoadedLevelFlags & K_LVL_LEVEL_FLAG_VINFINITE_MODE)
							{
								CControl* ctrl = null;
								if (ctrl = layer->GetControlByName("LABEL_LEADERBOARDS"))
									ctrl->paramsDict.SetNamedVarString(L"FontColor", L"0xffffffff");
								if (ctrl = layer->GetControlByName("LABEL_TITLE"))
									ctrl->paramsDict.SetNamedVarINT32(L"StringID", STR_FLOOR_X_VALUE);
							}

							if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
							{
								CHAR ctxt[MAX_PATH];
								StringCchPrintfA(ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1);
								ANALYTICS_EVENT("level_lose_1p", ctxt, "durationSec", nTimeSpent);
							}
						}
						else //2 players
						{
							UTGetControlsManager().RemoveAllLayers();

							CCtrlLayer* layer = null;
							if(!UTGetAppClass().IsGameNetworked())
								layer = UTGetControlsManager().ShowLayerOnce("LAYER_ID_LEVELFAIL_2P");
							else
								layer = UTGetControlsManager().ShowLayerOnce("LAYER_ID_LEVELFAIL_2P_COOP");

							if (layer != null)
							{
								CControl* ctrl = null;

								if (ctrl = layer->GetControlByName("CTRL_STARS"))
								{
									ctrl->paramsDict.SetNamedVarINT32(L"nStars", 0);
								}
								//reason why
								if (m_levelStateParam > 0) //if set
								{
									if (ctrl = layer->GetControlByName("BLINKER_REASON"))
									{
										ctrl->paramsDict.SetNamedVarINT32(L"stringID", m_levelStateParam);
									}
								}
								//portrete								
								if (ctrl = layer->GetControlByName("CTRL_ANIM_PORTRAIT_PL1"))
								{
									ctrl->paramsDict.SetNamedVarINT32(L"setFrame", (int)g_playerSelScr.m_arrPlayers[0].eType);
								}
								if (ctrl = layer->GetControlByName("CTRL_ANIM_PORTRAIT_PL2"))
								{
									ctrl->paramsDict.SetNamedVarINT32(L"setFrame", (int)g_playerSelScr.m_arrPlayers[1].eType);
								}

								//network - replace player names with real ones
								if (UTGetAppClass().IsGameNetworked())
								{
									if (ctrl = layer->GetControlByName("CTRL_WND_PL1"))
									{
										ctrl->paramsDict.SetNamedVarINT32(L"stringID", STR_NETWORK_HOST_NAME);
									}
									if (ctrl = layer->GetControlByName("CTRL_WND_PL2"))
									{
										ctrl->paramsDict.SetNamedVarINT32(L"stringID", STR_NETWORK_PEER_NAME);
									}

									if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
									{
										CHAR ctxt[MAX_PATH];
										StringCchPrintfA(ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1);
										ANALYTICS_EVENT("level_lose_2p_net", ctxt, "durationSec", nTimeSpent);
									}
									//XP bar - networked
									if (ctrl = layer->GetControlByName("CTRL_XPBAR_PL1"))
									{
										ctrl->paramsDict.SetNamedVarINT32(L"nOldValue", g_playerSelScr.m_arrPlayers[0].nPlayerXPPts);
										int nNew = LIMIT(g_playerSelScr.m_arrPlayers[0].nPlayerXPPts + nTotalXPPoints, 0, nMaxXPPoints);
										ctrl->paramsDict.SetNamedVarINT32(L"nNewValue", nNew);
									}
									if (ctrl = layer->GetControlByName("CTRL_XPBAR_PL2"))
									{
										ctrl->paramsDict.SetNamedVarINT32(L"nOldValue", g_playerSelScr.m_arrPlayers[1].nPlayerXPPts);
										int nNew = LIMIT(g_playerSelScr.m_arrPlayers[1].nPlayerXPPts + nTotalXPPoints, 0, nMaxXPPoints);
										ctrl->paramsDict.SetNamedVarINT32(L"nNewValue", nNew);
									}
								}
								else
								{
									if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
									{
										CHAR ctxt[MAX_PATH];
										StringCchPrintfA(ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1);
										ANALYTICS_EVENT("level_lose_2p", ctxt, "durationSec", nTimeSpent);
									}

									//XP bar
									if (ctrl = layer->GetControlByName("CTRL_XPBAR_PL1"))
									{
										int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
										ctrl->paramsDict.SetNamedVarINT32(L"nOldValue", nXPpl1);
										ctrl->paramsDict.SetNamedVarINT32(L"nNewValue", g_userData[nPlBaseIdx]);
									}
									if (ctrl = layer->GetControlByName("CTRL_XPBAR_PL2"))
									{
										int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[1].eType;
										ctrl->paramsDict.SetNamedVarINT32(L"nOldValue", nXPpl2);
										ctrl->paramsDict.SetNamedVarINT32(L"nNewValue", g_userData[nPlBaseIdx]);
									}
								}

								//show "melee = leaderboards" on tower mode	and reached level
								if (m_unLoadedLevelFlags & K_LVL_LEVEL_FLAG_VINFINITE_MODE)
								{
									CControl* ctrl = null;
									if (ctrl = layer->GetControlByName("LABEL_LEADERBOARDS"))
										ctrl->paramsDict.SetNamedVarString(L"FontColor", L"0xffffffff");
									if (ctrl = layer->GetControlByName("LABEL_TITLE"))
										ctrl->paramsDict.SetNamedVarINT32(L"StringID", STR_FLOOR_X_VALUE);
									//disable "next mission" on infinity towers mode
									if (ctrl = layer->GetControlByName("BUT_COOPFAIL_CONTINUE"))
										ctrl->bDisabled = true;
								}

							}
						}
						// notify level finished for achievements
						if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
							UTGetAppClass().App_OnLevelFinished(g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL]);
					}
				}
				break;
				default:
#ifdef ENABLE_LEADERBOARDS
					//show leaderboard when pressing melee key (any controller)	for VERTICAL VINFINITE mode
					if ((UTGetCtrlrMgr().KeyPressed(K_CM_COMMAND_MELEE)) && (m_unLoadedLevelFlags & K_LVL_LEVEL_FLAG_VINFINITE_MODE))
					{
						CCtrlLayer* lay = UTGetControlsManager().GetLayerByName("LAYER_ID_LEADERBOARDS_IGM");
						if (lay == null)
						{
							//show layer
							lay = UTGetControlsManager().ShowLayerOnce("LAYER_ID_LEADERBOARDS_IGM");
							if (lay)
							{
								CControl* ctrl = null;
								//change label that tells type of leaderboard that is shown
								if (ctrl = lay->GetControlByName("LABEL_LBTYPE"))
								{
									int nPlayers = m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED] + m_arrStats[K_LVL_STATS_PL2_HAS_PLAYED];
									if (nPlayers == 1)
										ctrl->paramsDict.SetNamedVarINT32(L"stringID", STR_SINGLE_PLAYER);
									else
										ctrl->paramsDict.SetNamedVarINT32(L"stringID", STR_COOP_ONLINE);
									//level name in STR_TEMP10
									g_stringsMgr.SetString(STR_TEMP10, L"%s", g_stringsMgr.strings[STR_VINFINITE_MODE]->sText);
								}
								//set player selection
								ctrl = lay->GetControlByName("CTRL_SCORESLIST_TT");
								if (ctrl != null)
								{
									ctrl->paramsDict.SetNamedVarINT32(L"nSelectedIdx", UTGetLeaderboards().GetDownloadedScores_PlayerIndex());
									ctrl->paramsDict.SetNamedVarINT32(L"nOptionsCnt", UTGetLeaderboards().GetDownloadedScoresCount());
#ifndef ENABLE_LEADERBOARDS_NAMES_SELECTION
									ctrl->bCanHaveFocus = false;
									ctrl->paramsDict.SetNamedVarBool(L"bUserCanSelect", false);
#endif
								}
							}
						}
					}
#endif
					break;
			}
		}
		break;

	}

	//clear poly buffers first
	m_bufferedPainter.ClearBuffers();

	///--- PHYSICS POINTS ---
	UpdatePhysicsPoints(dTime);
	///--- BULLETS ---
	UpdateBullets(dTime);
	///--- PROPS ---
	UpdateProps(dTime);
	///--- DECALS ---
	UpdateDecals(dTime);
	///--- ACTIVES ---
	UpdateAI(dTime);

	///--- release dead objects all at once here ---
	//(called before BuildVisibilityLists but after bullets,physics updates because it deallocates stuff from visibility lists)
	CleanupDeadObjects();

	///--- screen vignette ---
	m_screenVignetteDamage.Update(dTime);
	m_screenVignette.Update(dTime);

	///--- STATISTICS ---
	//active players
	m_nPlayersActive = m_nPlayers;
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if (pPlayerActor[kk] != null)
		{
			EAIBehaviorType curbeh = pPlayerActor[kk]->GetCurrentBehavior();
			if (curbeh == AI_BEHAVIOR_IN_LIMBO)
				m_nPlayersActive--;
		}
	}

	///--- update camera ---
	//default camera position following the players
	D3DXVECTOR2 avg_live(0.0f, 0.0f), avg_all(0.0f, 0.0f);
	int plcnt_live = 0, plcnt_all = 0;
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		//on networked games ignore the peer and stay locked onto the player
		if (UTGetAppClass().IsGameNetworked())
		{
			int nIndexToFollow = g_netlock.Net_GetPlayerIndex();
			// move camera on peer after you die
			if (pPlayerActor[g_netlock.Net_GetPlayerIndex()] == NULL)
			{
				nIndexToFollow = g_netlock.Net_GetOtherPlayerIndex();
				//if other player is dead too, just skip them and look at last spawn pos
				if (pPlayerActor[nIndexToFollow] == NULL)
					continue;
			}
			//in networked games just ignore the other player
			if ((UTGetAppClass().IsGameNetworked()) && (kk != nIndexToFollow))
				continue;
		}

		if ((pPlayerActor[kk] != NULL) && (pPlayerActor[kk]->GetCurrentBehavior() != AI_BEHAVIOR_IN_LIMBO) && (pPlayerActor[kk]->nSuspendedFlags == K_LVL_SUSPENDFLAG_NONE))
		{
			if (pPlayerActor[kk]->GetCurrentBehavior() != AI_BEHAVIOR_DEAD)
			{
				avg_live += pPlayerActor[kk]->posHeart + pPlayerActor[kk]->vecCamFollowPos;
				plcnt_live++;
			}

			avg_all += pPlayerActor[kk]->posHeart + pPlayerActor[kk]->vecCamFollowPos;
			plcnt_all++;
		}
	}

	//average player positions
	bool bAvgSet = false;
	D3DXVECTOR2 vPlayersAvg(0.0f, 0.0f);
	if (plcnt_all > 0)
	{
		bAvgSet = true;

		avg_all /= plcnt_all;
		vPlayersAvg = avg_all;

		if (plcnt_live > 0)
		{
			avg_live /= plcnt_live;
			//are they too far apart? 
			if (D3DXVec2Length(&(avg_all - avg_live)) > UTGetAppClass().g_rectGameScreen.h * 0.5f)
			{
				vPlayersAvg = avg_live;
			}
		}
	}

	//handles render size changes
	m_camLevel.SetViewport(UTGetAppClass().g_rectRender); 
	//daca nu are target se uita dupa players (media pozitiilor lor)
	if (m_camTargetActive == null)
	{
		if ((bAvgSet) && (!m_bInsideHiddenRoom))
			m_vCamPosDefault = vPlayersAvg;

#ifdef ENABLE_LEVEL_SHOWCASE
		//show the level with the mouse move
		m_vCamPosDefault = D3DXVECTOR2(	m_levelAABB.x + m_levelAABB.w * g_mouse.pos.x / UTGetAppClass().g_rectRender.w, 
										m_levelAABB.y + m_levelAABB.h * g_mouse.pos.y / UTGetAppClass().g_rectRender.h	);
#endif // ENABLE_LEVEL_SHOWCASE

		m_camLevel.SetCamPos(&m_vCamPosDefault);
	}
	else
	{
		m_camLevel.SetCamPos(&(m_camTargetActive->pos));
	}

	m_camLevel.Update(dTime);

	//find visible area
	RECTXYWH_F camrect = m_camLevel.GetCamWorldAABB();
	CAABB camAABB(D3DXVECTOR2(camrect.x, camrect.y), D3DXVECTOR2(camrect.Right(), camrect.Bottom()));

	//set sounds listener position
	SND_SET_LISTENER_POS(camrect.Center());
	//--- update particles and emitters ---
	g_particlesMgr.UpdatePartEmitters(dTime, camrect);
	g_particlesMgr.Update(dTime);
	g_particlesMgr.UpdateStringDummies(dTime);

	//set visible area rectangle
	m_visibleAreaTL.Set((int)camrect.x / tileW, (int)camrect.y / tileH, 2 + (int)camrect.w / tileW, 2 + (int)camrect.h / tileH);
	//limitare temp ca sa nu mai crasheze. Trebuie facut ceva serios in paint
	CLAMP(m_visibleAreaTL.x, m_levelAABB_TL.x, (m_levelAABB_TL.Right() - m_visibleAreaTL.w));
	CLAMP(m_visibleAreaTL.y, m_levelAABB_TL.y, (m_levelAABB_TL.Bottom() - m_visibleAreaTL.h));
	m_visibleArea.Set(m_visibleAreaTL.x * tileW, m_visibleAreaTL.y * tileH, m_visibleAreaTL.w * tileW, m_visibleAreaTL.h * tileH);
	///--- update visibility lists (after update) ---
	BuildVisibilityLists();

	///--- create vert buffers for lights ---
	for (int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++)
	{
		CLight *nl = m_visibleList.visible_lights.m_pData[kk];
		switch (nl->type)
		{
			case K_LVL_LIGHT_REALISTIC_IES_OBSOLETE:
			{
				ErrorBox(K_ERR_WARNING, L"CLevel::Update: Illegal light type!");
			}
			break;
			case K_LVL_LIGHT_POINT:
			{
				//Creez forma luminii (mesh-ul) - fac rotatia aici si nu in shader ca sa pot scoate bb-ul final al luminii
				D3DXVECTOR3 lcorners[4]; //ul, ur, dl, dr
				memcpy(lcorners, nl->lCorners, 4 * sizeof(D3DXVECTOR3));
				if (nl->fAngle != 0.0f)
				{
					D3DXMATRIXA16 matrot;
					D3DXMatrixRotationZ(&matrot, nl->fAngle);
					D3DXVec3TransformCoordArray(lcorners, sizeof(D3DXVECTOR3), lcorners, sizeof(D3DXVECTOR3), &matrot, 4);
				}
				//mut mesh pe pozitia finala
				lcorners[0].x += nl->pos.x; lcorners[0].y += nl->pos.y;
				lcorners[1].x += nl->pos.x; lcorners[1].y += nl->pos.y;
				lcorners[2].x += nl->pos.x; lcorners[2].y += nl->pos.y;
				lcorners[3].x += nl->pos.x; lcorners[3].y += nl->pos.y;
				//iau bbox-ul final dupa AABB-ul dat de cele 4 puncte rotite
				CAABB rotAABB = AABB_FromPoints(lcorners, 4);
				//scriu VS-ul final
				_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
				vul.pos = lcorners[0];
				vur.pos = lcorners[1];
				vdl.pos = lcorners[2];
				vdr.pos = lcorners[3];
				//setez culoarea
				vul.color = vur.color = vdl.color = vdr.color = nl->color;
				//setez coordonate textura spot
				//Coord de mapare pe RTT (tex2) se seteaza din shader
				vul.tex1 = D3DXVECTOR4(nl->lTexRect.left, nl->lTexRect.top, 0.0f, 0.0f);
				vur.tex1 = D3DXVECTOR4(nl->lTexRect.right, nl->lTexRect.top, 0.0f, 0.0f);
				vdl.tex1 = D3DXVECTOR4(nl->lTexRect.left, nl->lTexRect.bottom, 0.0f, 0.0f);
				vdr.tex1 = D3DXVECTOR4(nl->lTexRect.right, nl->lTexRect.bottom, 0.0f, 0.0f);
				//setez normalele finale
				vul.n = nl->pos3D - vul.pos;
				vur.n = nl->pos3D - vur.pos;
				vdl.n = nl->pos3D - vdl.pos;
				vdr.n = nl->pos3D - vdr.pos;
				//construiesc VB-ul exact
				_VERTEX_PNCT4T4 lightRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
				lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
				lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;

				//adauga mesh dinamic pentru volumul luminii
				nl->m_nLightMeshIdx = -1; //resetez idx mesh

				UINT32 meshidx;
				m_bufferedPainter.BeginMesh(meshidx);
				nl->m_nLightMeshIdx = (int)meshidx;
				m_bufferedPainter.AddTriangles(lightRectV, 2);
				m_bufferedPainter.EndMesh();

				//--- pentru luminile cu umbre creez shadow volumes	---
				if (nl->castShadows)
				{
					int occludersCnt = 0;
					nl->m_nShadowMeshIdx = -1;
					//foloseste pt verificare aabb-ul rotit al luminii
					COccluder* occludersArr = GetVisibleAABBs_toOccluders(D3DXVECTOR2(nl->pos.x, nl->pos.y), &rotAABB, occludersCnt); //returneaza pointer la array pe stack deci nu trebuie 
					//trimitem occluderele pt extinderea volumelor de umbre
					if (occludersCnt > 0)
					{
						//in cel mai rau caz avem toate occluderele splituite deci occCnt * 2 * 6 verts per occluder + sentinel (sunt cazuri cand suntem fix pe fix)
						_VERTEX_PNCT4T4 *arrVerts = new _VERTEX_PNCT4T4[occludersCnt * 12 + 12];
						int retVerts = BuildShadowVolume(nl, &rotAABB, occludersArr, occludersCnt, arrVerts, occludersCnt * 12 + 12);

						// adaugam triunghiurile ca si mesh
						if (retVerts > 0)
						{
							//adauga mesh dinamic pentru volumul umbrei
							UINT32 meshidx;
							m_bufferedPainter.BeginMesh(meshidx);
							nl->m_nShadowMeshIdx = (int)meshidx;
							m_bufferedPainter.AddTriangles(arrVerts, retVerts / 3);
							m_bufferedPainter.EndMesh();
						}

						SAFE_DELETE_ARRAY(arrVerts);
					}
				}
			}
			break;
			case K_LVL_LIGHT_AREA:
			{
				//create light mesh - rotating the actual mesh isn't necessary
				D3DXVECTOR3 lcorners[4]; //ul, ur, dl, dr
				memcpy(lcorners, nl->lCorners, 4 * sizeof(D3DXVECTOR3));
				//mut mesh pe pozitia finala
				lcorners[0].x += nl->pos.x; lcorners[0].y += nl->pos.y;
				lcorners[1].x += nl->pos.x; lcorners[1].y += nl->pos.y;
				lcorners[2].x += nl->pos.x; lcorners[2].y += nl->pos.y;
				lcorners[3].x += nl->pos.x; lcorners[3].y += nl->pos.y;
				//iau bbox-ul final dupa AABB-ul dat de cele 4 puncte rotite
				CAABB rotAABB = AABB_FromPoints(lcorners, 4);
				//scriu VS-ul final
				_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
				vul.pos = lcorners[0];
				vur.pos = lcorners[1];
				vdl.pos = lcorners[2];
				vdr.pos = lcorners[3];
				//setez culoarea
				vul.color = vur.color = vdl.color = vdr.color = nl->color;
				//setez coordonate textura spot
				vul.tex1 = D3DXVECTOR4(nl->lTexRect.left, nl->lTexRect.top, 0.0f, 0.0f);
				vur.tex1 = D3DXVECTOR4(nl->lTexRect.right, nl->lTexRect.top, 0.0f, 0.0f);
				vdl.tex1 = D3DXVECTOR4(nl->lTexRect.left, nl->lTexRect.bottom, 0.0f, 0.0f);
				vdr.tex1 = D3DXVECTOR4(nl->lTexRect.right, nl->lTexRect.bottom, 0.0f, 0.0f);
				//Coord de mapare pe RTT (tex2) se seteaza din shader
				//setez normalele finale - directia catre lumina
				vul.n = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
				vur.n = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
				vdl.n = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
				vdr.n = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
				//construiesc VB-ul exact
				_VERTEX_PNCT4T4 lightRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
				lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
				lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;

				//adauga mesh dinamic pentru volumul luminii
				nl->m_nLightMeshIdx = -1; //resetez idx mesh

				UINT32 meshidx;
				m_bufferedPainter.BeginMesh(meshidx);
				nl->m_nLightMeshIdx = (int)meshidx;
				m_bufferedPainter.AddTriangles(lightRectV, 2);
				m_bufferedPainter.EndMesh();
			}
			break;
			case K_LVL_LIGHT_AMBIENTAL:
			{
				//se face un dreptunghi cat ecranul, mapat din textura. Se va lua in considerare self illumination
				RECTXYWH_F camrect = m_camLevel.GetCamWorldAABB(); 

				_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
				vul.pos = D3DXVECTOR3(camrect.x, camrect.y, 0.0f);
				vur.pos = D3DXVECTOR3(camrect.Right(), camrect.y, 0.0f);
				vdl.pos = D3DXVECTOR3(camrect.x, camrect.Bottom(), 0.0f);
				vdr.pos = D3DXVECTOR3(camrect.Right(), camrect.Bottom(), 0.0f);
				//setez culoarea
				vul.color = vur.color = vdl.color = vdr.color = nl->color;
				//setez coordonate textura spot
				vul.tex1 = D3DXVECTOR4(nl->lTexRect.left, nl->lTexRect.top, 0.0f, 0.0f);
				vur.tex1 = D3DXVECTOR4(nl->lTexRect.right, nl->lTexRect.top, 0.0f, 0.0f);
				vdl.tex1 = D3DXVECTOR4(nl->lTexRect.left, nl->lTexRect.bottom, 0.0f, 0.0f);
				vdr.tex1 = D3DXVECTOR4(nl->lTexRect.right, nl->lTexRect.bottom, 0.0f, 0.0f);
				//Coord de mapare pe RTT (tex2) se seteaza din shader
				//setez normalele finale - directia catre lumina
				D3DXVECTOR3 lightdir(0.0f, 0.0f, 1.0f);
				vul.n = lightdir;
				vur.n = lightdir;
				vdl.n = lightdir;
				vdr.n = lightdir;

				//construiesc VB-ul exact
				_VERTEX_PNCT4T4 lightRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
				lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
				lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;

				//adauga mesh dinamic pentru volumul luminii
				nl->m_nLightMeshIdx = -1; //resetez idx mesh

				UINT32 meshidx;
				m_bufferedPainter.BeginMesh(meshidx);
				nl->m_nLightMeshIdx = (int)meshidx;
				m_bufferedPainter.AddTriangles(lightRectV, 2);
				m_bufferedPainter.EndMesh();
			}
			break;
			case K_LVL_LIGHT_DIRECTIONAL:
			{
				//Creez forma luminii (mesh-ul) - nu fac rotatie pentru ca nu foloseste la nimic
				D3DXVECTOR3 lcorners[4]; //ul, ur, dl, dr
				memcpy(lcorners, nl->lCorners, 4 * sizeof(D3DXVECTOR3));
				//mut mesh pe pozitia finala
				lcorners[0].x += nl->pos.x; lcorners[0].y += nl->pos.y;
				lcorners[1].x += nl->pos.x; lcorners[1].y += nl->pos.y;
				lcorners[2].x += nl->pos.x; lcorners[2].y += nl->pos.y;
				lcorners[3].x += nl->pos.x; lcorners[3].y += nl->pos.y;
				//iau bbox-ul final dupa AABB-ul dat de cele 4 puncte rotite
				CAABB rotAABB = AABB_FromPoints(lcorners, 4);
				//scriu VS-ul final
				_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
				vul.pos = lcorners[0];
				vur.pos = lcorners[1];
				vdl.pos = lcorners[2];
				vdr.pos = lcorners[3];
				//setez culoarea
				vul.color = vur.color = vdl.color = vdr.color = nl->color;
				//setez coordonate textura spot
				vul.tex1 = D3DXVECTOR4(nl->lTexRect.left, nl->lTexRect.top, 0.0f, 0.0f);
				vur.tex1 = D3DXVECTOR4(nl->lTexRect.right, nl->lTexRect.top, 0.0f, 0.0f);
				vdl.tex1 = D3DXVECTOR4(nl->lTexRect.left, nl->lTexRect.bottom, 0.0f, 0.0f);
				vdr.tex1 = D3DXVECTOR4(nl->lTexRect.right, nl->lTexRect.bottom, 0.0f, 0.0f);
				//Coord de mapare pe RTT (tex2) se seteaza din shader
				//setez normalele finale - directia catre lumina
				D3DXVECTOR3 lightdir(-100.0f * cos(nl->fAngle), -100.0f * sin(nl->fAngle), nl->pos3D.z);
				vul.n = lightdir;
				vur.n = lightdir;
				vdl.n = lightdir;
				vdr.n = lightdir;

				//construiesc VB-ul exact
				_VERTEX_PNCT4T4 lightRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
				lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
				lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;

				//adauga mesh dinamic pentru volumul luminii
				nl->m_nLightMeshIdx = -1; //resetez idx mesh

				UINT32 meshidx;
				m_bufferedPainter.BeginMesh(meshidx);
				nl->m_nLightMeshIdx = (int)meshidx;
				m_bufferedPainter.AddTriangles(lightRectV, 2);
				m_bufferedPainter.EndMesh();
			}
			break;
		}
	}
	//2. poligoane alte lumini: gloante, particule, etc
	//PROPS lights - temp lights - gunshot lights, explo lights
	m_propsLightsMeshIdx = -1;
	UINT32 meshidx;
	m_bufferedPainter.BeginMesh(meshidx);
	m_propsLightsMeshIdx = (int)meshidx;

	CLinkedPool<CLevelProp>::CLinkedPoolNode *node = m_poolProps.pListUsed.m_pNext;
	while (node != &m_poolProps.pListUsed)
	{
		//salvez locatia urmatoare ca s apot avansa pe ea
		CLinkedPool<CLevelProp>::CLinkedPoolNode *nextnode = node->m_pNext;
		CLevelProp* prop = &node->m_data;

		if (prop->bMakesLight)
		{
			if (prop->sprLight.animationIdx >= 0)
			{
				//Creez forma luminii (mesh-ul)
				RECTLTRB_F realrect = m_sprLights.GetAFrameBBox_real_LTRB(prop->sprLight.animationIdx, 0);
				//Scalez dreptunghi lumina
				if (prop->fLightScaling != 1.0f)
				{
					CAABB realaabb;
					realaabb.Set(realrect);
					realaabb.Scale(prop->fLightScaling);
					realrect.left = realaabb.vMin.x; realrect.top = realaabb.vMin.y;
					realrect.right = realaabb.vMax.x; realrect.bottom = realaabb.vMax.y;
				}
				//scriu VS-ul final
				_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
				D3DXVECTOR2 bpos2D = node->m_data.physPt->m_data.pos;
				D3DXVECTOR3 bpos(node->m_data.physPt->m_data.pos.x, node->m_data.physPt->m_data.pos.y, 50.0f);
				vul.pos = D3DXVECTOR3(bpos.x + realrect.left, bpos.y + realrect.top, 0.0f);
				vur.pos = D3DXVECTOR3(bpos.x + realrect.right, bpos.y + realrect.top, 0.0f);
				vdl.pos = D3DXVECTOR3(bpos.x + realrect.left, bpos.y + realrect.bottom, 0.0f);
				vdr.pos = D3DXVECTOR3(bpos.x + realrect.right, bpos.y + realrect.bottom, 0.0f);
				//setez culoarea
				float fLife = prop->fLightDuration;
				float fFadeTime = prop->fLightFadeOut;

				float alpha = 1.0f;
				//la unele nu setez fLife deci ma intereseaza sa se vada
				if (fLife > 0.0f)
				{
					if (prop->fLightTimer < fFadeTime)
						alpha = prop->fLightTimer / fFadeTime;
					else if (prop->fLightTimer > fLife)
						alpha = 0.0f;
					else if (prop->fLightTimer > fLife - fFadeTime)
						alpha = ((fLife - prop->fLightTimer) / fFadeTime);
				}

				float fOrigAlpha = D3DCOLOR_GETFALPHA(prop->sprLight.color);
				vul.color = vur.color = vdl.color = vdr.color = D3DCOLOR_COLORALPHA(prop->sprLight.color, fOrigAlpha * alpha);
				//setez coordonate textura spot
				RECTLTRB_F lTexRect = m_sprLights.GetModuleRect_TexCoords(prop->sprLight.animationIdx, 0, 0);
				//Coord de mapare pe RTT (tex2) se seteaza din shader
				vul.tex1 = D3DXVECTOR4(lTexRect.left, lTexRect.top, 0.0f, 0.0f);
				vur.tex1 = D3DXVECTOR4(lTexRect.right, lTexRect.top, 0.0f, 0.0f);
				vdl.tex1 = D3DXVECTOR4(lTexRect.left, lTexRect.bottom, 0.0f, 0.0f);
				vdr.tex1 = D3DXVECTOR4(lTexRect.right, lTexRect.bottom, 0.0f, 0.0f);
				//setez normalele finale
				vul.n = bpos - vul.pos;
				vur.n = bpos - vur.pos;
				vdl.n = bpos - vdl.pos;
				vdr.n = bpos - vdr.pos;
				//construiesc VB-ul exact
				_VERTEX_PNCT4T4 lightRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
				lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
				lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;

				m_bufferedPainter.AddTriangles(lightRectV, 2);
			}
		}

		//get to next node
		node = nextnode;
	}
	//inchid meshul
	m_bufferedPainter.EndMesh();
	
	//#TODO: should try not clamping water and FOW rects to screen maybe it fixes the texture/pshader issue on some cards

	//3. poligoane apa
	m_bufferedPainter.BeginMesh(meshidx);
	m_waterMeshIdx = (int)meshidx;
	//salvez date textura apa	
	float waterTexScale = 2.0f;
	float waterTexSize = m_texManager.m_Texs[m_waterTexIdx]->info.Width;

	for (int kk = 0; kk < m_visibleList.logic_colShapesSpecial.Count(); kk++)
	{
		if (m_visibleList.logic_colShapesSpecial.m_pData[kk]->type == K_LVL_COLL_TYPE_WATER)
		{
			CCollisionShape * col = m_visibleList.logic_colShapesSpecial.m_pData[kk];
			CAABB wbb; //water bbox
			if (AABB_Intersection(col->bbox, camAABB, wbb))
			{
				D3DXVECTOR2 texoff = col->bbox.vMin - wbb.vMin;
				//save water plys in a sigle mesh, clipped to screen rect
				
				//scriu VS-ul final
				_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
				vul.pos = D3DXVECTOR3(wbb.vMin.x, wbb.vMin.y, 0.0f);
				vur.pos = D3DXVECTOR3(wbb.vMax.x, wbb.vMin.y, 0.0f);
				vdl.pos = D3DXVECTOR3(wbb.vMin.x, wbb.vMax.y, 0.0f);
				vdr.pos = D3DXVECTOR3(wbb.vMax.x, wbb.vMax.y, 0.0f);
				//setez culoarea
				//#TODO: culoarea sa fie setata undeva in editor. Poate as putea sa pun control de culoare la collision boxuri...
				vul.color = vur.color = vdl.color = vdr.color = 0xaa30AFFF;// col->color;
				//setez coordonate textura apa
				D3DXVECTOR2 texul = (wbb.vMin * waterTexScale) / waterTexSize;
				D3DXVECTOR2 texdr = (wbb.vMax * waterTexScale) / waterTexSize;

				RECTLTRB_F lTexRect(texul.x, texul.y, texdr.x, texdr.y);
				//Coord de mapare pe RTT (tex2) se seteaza din shader
				vul.tex1 = D3DXVECTOR4(lTexRect.left, lTexRect.top, 0.0f, 0.0f);
				vur.tex1 = D3DXVECTOR4(lTexRect.right, lTexRect.top, 0.0f, 0.0f);
				vdl.tex1 = D3DXVECTOR4(lTexRect.left, lTexRect.bottom, 0.0f, 0.0f);
				vdr.tex1 = D3DXVECTOR4(lTexRect.right, lTexRect.bottom, 0.0f, 0.0f);
				//setez normalele finale
				vul.n = vur.n = vdl.n = vdr.n = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
				//construiesc VB-ul exact
				_VERTEX_PNCT4T4 waterRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
				waterRectV[0] = vul; waterRectV[1] = vur; waterRectV[2] = vdl;
				waterRectV[3] = vur; waterRectV[4] = vdl; waterRectV[5] = vdr;

				m_bufferedPainter.AddTriangles(waterRectV, 2);
			}
		}
	}
	//inchid meshul apelor
	m_bufferedPainter.EndMesh();


	//4. poligoane fow
	m_bufferedPainter.BeginMesh(meshidx);
	m_fogofwarMeshIdx = (int)meshidx;

	for (int kk = 0; kk < m_visibleList.logic_colShapesSpecial.Count(); kk++)
	{
		if (m_visibleList.logic_colShapesSpecial.m_pData[kk]->type == K_LVL_COLL_TYPE_FOG_OF_WAR)
		{
			CCollisionShape * col = m_visibleList.logic_colShapesSpecial.m_pData[kk];
			CAABB wbb; //bbox
			if (AABB_Intersection(col->bbox, camAABB, wbb))
			{
				//scriu VS-ul final
				_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
				vul.pos = D3DXVECTOR3(wbb.vMin.x, wbb.vMin.y, 0.0f);
				vur.pos = D3DXVECTOR3(wbb.vMax.x, wbb.vMin.y, 0.0f);
				vdl.pos = D3DXVECTOR3(wbb.vMin.x, wbb.vMax.y, 0.0f);
				vdr.pos = D3DXVECTOR3(wbb.vMax.x, wbb.vMax.y, 0.0f);
				//setez culoarea (setata pe onload)
				vul.color = vur.color = vdl.color = vdr.color = col->color;
				//setez coordonate tex2 (nu se folosesc)
				D3DXVECTOR2 texul = wbb.vMin;
				D3DXVECTOR2 texdr = wbb.vMax;

				RECTLTRB_F lTexRect(texul.x, texul.y, texdr.x, texdr.y);
				//Coord de mapare pe RTT (tex2) se seteaza din shader
				vul.tex1 = D3DXVECTOR4(lTexRect.left, lTexRect.top, 0.0f, 0.0f);
				vur.tex1 = D3DXVECTOR4(lTexRect.right, lTexRect.top, 0.0f, 0.0f);
				vdl.tex1 = D3DXVECTOR4(lTexRect.left, lTexRect.bottom, 0.0f, 0.0f);
				vdr.tex1 = D3DXVECTOR4(lTexRect.right, lTexRect.bottom, 0.0f, 0.0f);
				//setez normalele finale
				vul.n = vur.n = vdl.n = vdr.n = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
				//construiesc VB-ul exact
				_VERTEX_PNCT4T4 fowRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
				fowRectV[0] = vul; fowRectV[1] = vur; fowRectV[2] = vdl;
				fowRectV[3] = vur; fowRectV[4] = vdl; fowRectV[5] = vdr;

				m_bufferedPainter.AddTriangles(fowRectV, 2);
			}
		}
	}
	//inchid meshul apelor
	m_bufferedPainter.EndMesh();

	///--- build buffered painter buffers ---
	m_bufferedPainter.BuildBuffers();

	///--- update interface ---
	m_interfaceIGM.Update(dTime);
	m_interfaceTextBubble.Update(dTime);

	//set update done flag
	m_bOneUpdateDone = true;
}

HRESULT CLevel::PaintOffscreen()
{
	if ((!m_bLoaded) || (!m_bOneUpdateDone))
		return E_FAIL;

	RECTXYWH_F camrect = m_camLevel.GetCamWorldAABB();
	//CAABB al camerei
	CAABB		camAABB;  
	camAABB.Set(D3DXVECTOR2(camrect.x, camrect.y), D3DXVECTOR2(camrect.Right(), camrect.Bottom()));
	//matrice folosita local
	D3DXMATRIXA16 matlocal;

	HRESULT hr = S_OK;
	//daca nu am capabilitatea de offscreen ies cu eroare
	if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_RTT) == 0)
		return E_FAIL;

	hr = m_pRenderToSurface->BeginScene(m_pRTSurface, NULL);
	if (SUCCEEDED(hr))
	{
		// Clear the render target and the zbuffer 
		V(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, K_GAME_CLEAR_COLOR, 1.0f, 0));

		m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
		m_pDevice->SetTransform(D3DTS_VIEW, &g_matIdentity);
		//matrice de proiectie offsetata ca sa incapa un pixel intreg (pixel center e in centru)
		D3DXMATRIXA16 matProj;
		D3DXMatrixOrthoOffCenterLH(&matProj, 0.5f, K_RTT_WIDTH + 0.5f, K_RTT_HEIGHT + 0.5f, 0.5f, 0.0f, 1.0f);
		m_pDevice->SetTransform(D3DTS_PROJECTION, &matProj);

		m_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
		//use sprite
		m_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_OBJECTSPACE | D3DXSPRITE_DONOTSAVESTATE);

		m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

		m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		m_pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		//#HACK:cand am alpha pe jumatate rezultatul blendingului pe alpha ar iesi si el pe 0 deci in schimba alpha pe mai mica
		//asta inseamna ca daca am chestii semitransparente imi modifica alpha finala a render targetului
		//#TODO: aici ar trebui ca umbrele obiectelor sa fie facute din normal map cumva ca sa nu mai am nevoie de separate alpha blending
		//#TODO: totusi daca am tiles semitransparente pe layer din fatza imi apare aiurea pe cel din spate daca are semitransparenta sau nu e activat alphatest.
		if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0)
		{
			m_pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);  //????? - este necesara dar nu e foarte bine suportata de multe placi
			m_pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA);
			m_pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_DESTALPHA);
			m_pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
		}
		//set scroll matrix
		D3DXMATRIXA16 mattrans;
		D3DXMatrixAffineTransformation2D(&mattrans, 1.0f, NULL, 0.0f, &D3DXVECTOR2(-m_visibleArea.x, -m_visibleArea.y));
		m_pSprite->SetTransform(&mattrans);

		///.////////////////////////////////////////////////////////
		///	COLOR MAP
		///.////////////////////////////////////////////////////////

		CAABB aabbScissor;
		aabbScissor.Set(0.0f, 0.0f, (float)K_RTT_H_WIDTH, (float)K_RTT_H_HEIGHT);
		//set clip on colormap
		if ((m_bInsideHiddenRoom) && (m_HiddenRoomAABB.vSize.x > 0.0f) && (m_HiddenRoomAABB.vSize.y > 0.0f))
		{
			CAABB aabbVisible;
			aabbVisible.Set(m_visibleArea);
			AABB_Intersection(aabbVisible, m_HiddenRoomAABB, aabbScissor);
			aabbScissor.Move(D3DXVECTOR2(-m_visibleArea.x, -m_visibleArea.y));
		}

		SetScissorClip(m_pDevice, aabbScissor.vMin.x, aabbScissor.vMin.y, aabbScissor.vSize.x, aabbScissor.vSize.y);

		///--- tiles back layer
		m_pSprite->SetTransform(&g_matIdentity);
		//tiles - background
		for (int yy = 0; yy < m_visibleAreaTL.h; yy++)
		{
			for (int xx = 0; xx < m_visibleAreaTL.w; xx++)
			{
				int tlX = xx + m_visibleAreaTL.x - m_levelAABB_TL.x;
				int tlY = yy + m_visibleAreaTL.y - m_levelAABB_TL.y;
				CTile *tl = &tiles[tlX][tlY];
				if (tl->tileIDs[0] >= 0)
					m_pSprite->Draw(m_texManager.m_Texs[m_tilesTexBaseIdx]->pTexture, &tl->srcRects[0], NULL, &D3DXVECTOR3(xx * tileW, yy * tileH, 0.0f), 0xffffffff);
			}
		}
		m_pSprite->Flush();

		///--- objects back layer ---
		m_pSprite->SetTransform(&mattrans);

		for (int kk = 0; kk < m_visibleList.visible_actives[K_LVL_LAYER_BACK].Count(); kk++)
		{
			CActive *active = m_visibleList.visible_actives[K_LVL_LAYER_BACK].m_pData[kk];
			if (active->flipX /*|| active->flipY*/)
			{
				matlocal = mattrans;
				//pozitie sprite
				if (active->flipX)
				{
					matlocal._11 = -1.0f; //scalare X
					matlocal._41 += 2.0f * active->pos.x;// +2.0f * active_bbox.x + active_bbox.w; 
				}
				m_pSprite->SetTransform(&matlocal);
				active->sprite.paint_firstModule(&m_sprActives);
				m_pSprite->SetTransform(&mattrans);
			}
			else
			{
				active->sprite.paint_firstModule(&m_sprActives);
			}
		}
		m_pSprite->Flush();

		///--- tiles MIDDLE layer
		m_pSprite->SetTransform(&g_matIdentity);
		//tiles - background
		for (int yy = 0; yy < m_visibleAreaTL.h; yy++)
		{
			for (int xx = 0; xx < m_visibleAreaTL.w; xx++)
			{
				int tlX = xx + m_visibleAreaTL.x - m_levelAABB_TL.x;
				int tlY = yy + m_visibleAreaTL.y - m_levelAABB_TL.y;
				CTile *tl = &tiles[tlX][tlY];
				if (tl->tileIDs[1] >= 0)
					m_pSprite->Draw(m_texManager.m_Texs[m_tilesTexBaseIdx]->pTexture, &tl->srcRects[1], NULL, &D3DXVECTOR3(xx * tileW, yy * tileH, 0.0f), 0xffffffff);
			}
		}
		m_pSprite->Flush();

		///--- decals (blood stains, explosion marks, bullet holes etc) ---
		if (m_visibleList.visible_decals[K_LVL_DECAL_LAYER_BACKWALLS].Count() > 0)
		{
			m_pSprite->SetTransform(&mattrans);
			//set special state (keep alpha of destination)
			if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0)
			{
				m_pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ZERO);
				m_pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
				m_pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			}
			else
			{
				m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true); //neaparat nevoie
				m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
				m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			}
			//--- paint them ---
			for (int kk = 0; kk < m_visibleList.visible_decals[K_LVL_DECAL_LAYER_BACKWALLS].Count(); kk++)
			{
				m_visibleList.visible_decals[K_LVL_DECAL_LAYER_BACKWALLS].m_pData[kk]->sprite.paint_firstModule(&m_sprActives);
			}
			m_pSprite->Flush();

			//restore state
			if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0)
			{
				m_pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA);
				m_pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_DESTALPHA);
				m_pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
			}
			else
			{
				m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
				m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			}
		}

		///--- objects MIDDLE layer ---
		m_pSprite->SetTransform(&mattrans);

		for (int kk = 0; kk < m_visibleList.visible_actives[K_LVL_LAYER_MIDDLE].Count(); kk++)
		{
			CActive *active = m_visibleList.visible_actives[K_LVL_LAYER_MIDDLE].m_pData[kk];
			if (active->flipX)
			{
				matlocal = mattrans;
				//pozitie sprite
				if (active->flipX)
				{
					matlocal._11 = -1.0f; //scalare X
					matlocal._41 += 2.0f * active->pos.x;// +2.0f * active_bbox.x + active_bbox.w; 
				}
				m_pSprite->SetTransform(&matlocal);
				active->sprite.paint_firstModule(&m_sprActives);
				m_pSprite->SetTransform(&mattrans);
			}
			else
			{
				active->sprite.paint_firstModule(&m_sprActives);
			}
		}

		//--- middle objects that can be interacted with are blinking ---
		AdditiveBlendingON(m_pDevice, m_pSprite);
		float fAlp = 0.4f * LIMIT(float((2.0f * sin(fLocalTimeline * 2.5f)) - 1.0f), 0.0f, 1.0f);
		float fAlp2 = 0.4f * ((sin(fLocalTimeline * 10.0f) + 1.0f) / 2.0f);
		for (int kk = 0; kk < m_visibleList.visible_actives[K_LVL_LAYER_MIDDLE].Count(); kk++)
		{
			CActive *active = m_visibleList.visible_actives[K_LVL_LAYER_MIDDLE].m_pData[kk];
			
			if (!active->bStandsOut)
				continue;

			DWORD colAlpha = D3DCOLOR_FFFA(fAlp);
			//object is being touched so show it
			if( ((pPlayerActor[0] != null) && (active == pPlayerActor[0]->pClosestTouchable)) ||
				((pPlayerActor[1] != null) && (active == pPlayerActor[1]->pClosestTouchable)) )
				colAlpha = D3DCOLOR_FFFA(fAlp2);

			if (active->flipX)
			{
				matlocal = mattrans;
				//pozitie sprite
				if (active->flipX)
				{
					matlocal._11 = -1.0f; //scalare X
					matlocal._41 += 2.0f * active->pos.x;// +2.0f * active_bbox.x + active_bbox.w; 
				}

				m_pSprite->SetTransform(&matlocal);
				CSprite spr = active->sprite;
				spr.color = colAlpha;
				spr.paint_firstModule(&m_sprActives);
				m_pSprite->SetTransform(&mattrans);
			}
			else
			{
				CSprite spr = active->sprite;
				spr.color = colAlpha;
				spr.paint_firstModule(&m_sprActives);
			}
		}
		AdditiveBlendingOFF(m_pDevice, m_pSprite);


		///--- actor shadows --- only on super high level of detail ---
		m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true); //neaparat nevoie
		m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
		m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		m_pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);  //disable

		for (int kk = 0; kk < m_visibleList.visible_actors.Count(); kk++)
		{
			CActor* actor = m_visibleList.visible_actors.m_pData[kk];
		
			matlocal = mattrans;
			//aplic matrice flipX daca este cazul
			if (actor->lookDirXsign == -1)
			{
				matlocal._11 = -1.0f;
				matlocal._41 += 2.0f * actor->sprite_feet.pos.x;
			}

			//--- versiune cu o umbra fixa ---
			switch (UTGetAppClass().m_Settings.nLOD_shadows)
			{
				case K_UT_LOD_LOW:
				{
					matlocal._41 += 6.0f; //distanta umbrei
					m_pSprite->SetTransform(&matlocal);
					
					float spriteAlpha = D3DCOLOR_GETFALPHA(actor->color);
					DWORD dwShadCol = D3DCOLOR_XXXA(spriteAlpha * 0.4f);

					if ((actor->templateActor.bComposedAnimation) && (actor->sprite_feet.animationIdx >= 0))
					{
						actor->sprite_feet.paint_firstModuleColorized(&m_sprActors, dwShadCol);
					}
					actor->sprite.paint_firstModuleColorized(&m_sprActors, dwShadCol);
				}
				break;
				case K_UT_LOD_MED:
				{
					//versiune cu o singura umbra dinamica, media iluminarii
					//max shadow offset (16.0f)
					float fMaxOffset = 16.0f;

					float fIllumination = 0.0f; //cantitatea de lumina care cade pe actor
					D3DXVECTOR2 vLightResultant(0.0f, 0.0f); //media vectorilor de iluminare

					int influences = 0;
					for (int ll = 0; ll < m_visibleList.visible_lights.Count(); ll++)
					{
						CLight* light = m_visibleList.visible_lights[ll];
						if (!light->castShadows)
							continue;
						if (light->type != K_LVL_LIGHT_POINT)
							continue;
						D3DXVECTOR2 lightdir = light->pos - actor->posHeart;
						float lightdist = D3DXVec2Length(&lightdir);
						if (lightdist > light->fMaxRadius)
							continue;
						if (!IsLineOfSight(light->pos, actor->GetPosHeart()))
							continue;

						//normalize vector
						lightdir /= lightdist;
						float opacity = 1.0f - lightdist / m_visibleList.visible_lights.m_pData[ll]->fMaxRadius;

						fIllumination += opacity;
						//medie ponderata a vectorilor
						vLightResultant += (lightdir * (1.0f - opacity) * fMaxOffset) * opacity;
						influences++;
					}

					if (influences > 0)
					{
						vLightResultant /= (float)influences;
						D3DXMATRIXA16 matshad = matlocal;
						//scalare offset
						float offlen = D3DXVec2Length(&vLightResultant);
						if (offlen > fMaxOffset)
							vLightResultant *= fMaxOffset / offlen;

						matshad._41 -= vLightResultant.x;
						matshad._42 -= vLightResultant.y;

						m_pSprite->SetTransform(&matshad);

						float spriteAlpha = D3DCOLOR_GETFALPHA(actor->color);
						DWORD dwShadCol = D3DCOLOR_COLORALPHA(0x00000000, spriteAlpha * (fIllumination * 0.6f)); //max opacity = 0.7f

						if ((actor->templateActor.bComposedAnimation) && (actor->sprite_feet.animationIdx >= 0))
						{
							actor->sprite_feet.paint_firstModuleColorized(&m_sprActors, dwShadCol);
						}
						actor->sprite.paint_firstModuleColorized(&m_sprActors, dwShadCol);
					}
				}
				break;
				case K_UT_LOD_HIGH:
				{
					//pentru fiecare lumina vad unde pica proiectia umbrei - versiune cu umbre dinamice
					//are mici probleme cand nu te vede lumina si cand e lumina de sub tine si apare doar printr-o raza (umbra apare intreaga si iese din volumul luminii)
					D3DXMATRIXA16 matshad;
					float spriteAlpha = D3DCOLOR_GETFALPHA(actor->color);
					for (int ll = 0; ll < m_visibleList.visible_lights.Count(); ll++)
					{
						CLight* light = m_visibleList.visible_lights[ll];
						if (!light->castShadows)
							continue;
						if (light->type != K_LVL_LIGHT_POINT)
							continue;
						D3DXVECTOR2 lightdir = light->pos - actor->GetPosHeart();
						float lightdist = D3DXVec2Length(&lightdir);
						if (lightdist > light->fMaxRadius)
							continue;
						if (!IsLineOfSight(light->pos, actor->posHeart))
							continue;
						//normalize vector
						lightdir /= lightdist;
						float opacity = 1.0f - lightdist / light->fMaxRadius;

						matshad = matlocal;

						//max shadow offset (16.0f)
						matshad._41 -= (1.0f - opacity) * lightdir.x * 16.0f;
						matshad._42 -= (1.0f - opacity) * lightdir.y * 16.0f;

						m_pSprite->SetTransform(&matshad);
						DWORD dwShadCol = D3DCOLOR_COLORALPHA(0x00000000, spriteAlpha * opacity * 0.7f); //max opacity = 0.7f
						if ((actor->templateActor.bComposedAnimation) && (actor->sprite_feet.animationIdx >= 0))
						{
							actor->sprite_feet.paint_firstModuleColorized(&m_sprActors, dwShadCol);
						}
						actor->sprite.paint_firstModuleColorized(&m_sprActors, dwShadCol);
					}
				}
				break;
			}
		}
		m_pSprite->Flush();

		//restore state for normal rendering
		m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0)
		{
			m_pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
		}

		///--- WALLS - front layer/sections ---
		m_pSprite->SetTransform(&g_matIdentity);
		//tiles - foreground/sections
		for (int yy = 0; yy < m_visibleAreaTL.h; yy++)
		{
			for (int xx = 0; xx < m_visibleAreaTL.w; xx++)
			{
				int tlX = xx + m_visibleAreaTL.x - m_levelAABB_TL.x;
				int tlY = yy + m_visibleAreaTL.y - m_levelAABB_TL.y;
				CTile *tl = &tiles[tlX][tlY];
				if (tl->tileIDs[2] >= 0)
					m_pSprite->Draw(m_texManager.m_Texs[m_tilesTexBaseIdx]->pTexture, &tl->srcRects[2], NULL, &D3DXVECTOR3(xx * tileW, yy * tileH, 0.0f), 0xffffffff);
			}
		}
		m_pSprite->Flush();

		///--- decals objects FRONT layer 
		m_pSprite->SetTransform(&mattrans);
		for (int kk = 0; kk < m_visibleList.visible_decals[K_LVL_DECAL_LAYER_BACKOBJECTS].Count(); kk++)
		{
			m_visibleList.visible_decals[K_LVL_DECAL_LAYER_BACKOBJECTS].m_pData[kk]->sprite.paint_firstModule(&m_sprActives);
		}
		m_pSprite->Flush();

		///--- paint RT particles - BACK ---
		m_pSprite->SetTransform(&g_matIdentity);
		g_particlesMgr.PaintLayerOffset(K_PART_LAYER_RT_BACK_NRM, D3DXVECTOR2(-m_visibleArea.x, -m_visibleArea.y), false);
		g_particlesMgr.PaintLayerOffset(K_PART_LAYER_RT_BACK_NRM_LIGHT, D3DXVECTOR2(-m_visibleArea.x, -m_visibleArea.y), true);


		///--- paint actors (player included) ---
		m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
		for (int kk = 0; kk < m_visibleList.visible_actors.Count(); kk++)
		{
			CActor* actor = m_visibleList.visible_actors.m_pData[kk];

			matlocal = mattrans;
			//apply flipX matrix
			if (actor->lookDirXsign == -1)
			{
				matlocal._11 = -1.0f;
				matlocal._41 += 2.0f * actor->sprite_feet.pos.x; //takes the coord from the sprite position as that's already rounded (eliminates jitter)
			}
			m_pSprite->SetTransform(&matlocal);

			if ((actor->templateActor.bComposedAnimation) && (actor->sprite_feet.animationIdx >= 0))
			{
				actor->sprite_feet.paint_firstModule_texOverride(&m_sprActors, actor->nSkinIdx * 2);
			}
			actor->sprite.paint_firstModule_texOverride(&m_sprActors, actor->nSkinIdx * 2);

			//--- muzzle flash ---
			if ((actor->pCurrentWeapon != null) && (actor->pCurrentWeapon->m_sprMuzzleFlash.animationIdx >= 0) && 
				(actor->pCurrentWeapon->m_sprMuzzleFlash.animStatus != ANIM_STATUS_FRAMELOCK) )
			{
				actor->pCurrentWeapon->m_sprMuzzleFlash.pos = actor->pos + actor->vecWeapon_abs[((actor->bCrouched) ? 1 : 0)];
				actor->pCurrentWeapon->m_sprMuzzleFlash.paint_firstModule(&m_sprActors);
			}
		}

		m_pSprite->Flush();

		m_pDevice->SetTransform(D3DTS_WORLD, &mattrans);
		///--- BULLETS ---
		PaintBullets();
		//--- paint level Props ---
		PaintProps(); //se deseneaza din active
		
		m_pSprite->Flush();
		m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);

		//--- paint RT particles - FRONT ---
		m_pSprite->SetTransform(&g_matIdentity);
		g_particlesMgr.PaintLayerOffset(K_PART_LAYER_RT_FRONT_NRM, D3DXVECTOR2(-m_visibleArea.x, -m_visibleArea.y), false);
		g_particlesMgr.PaintLayerOffset(K_PART_LAYER_RT_FRONT_NRM_LIGHT, D3DXVECTOR2(-m_visibleArea.x, -m_visibleArea.y), true);

		///--- paint water details ---
		/*
		//#TODO: de vazut daca se mai poate optimiza aici...si daca merita optimizat
		//daca voi avea mai multe chestii de desenat din acest array atunci nu mai merita optimizat
		m_pSprite->SetTransform(&mattrans);
		for (int kk = 0; kk < m_visibleList.logic_colShapesSpecial.Count(); kk++)
		{
			CCollisionShape * col = m_visibleList.logic_colShapesSpecial.m_pData[kk];
			//water
			if (col->type == K_LVL_COLL_TYPE_WATER)
			{
				if (m_waterAnimIdx < 0)
				{
#if defined(_DEBUG) || defined(DEBUG)
					ErrorBox(K_ERR_WARNING, L"Water animation not set in background object (editor)!");
#endif
					continue;
				}
				CAABB wbb; //water bbox
				if (AABB_Intersection(col->bbox, camAABB, wbb))
				{
					D3DXVECTOR2 texoff = col->bbox.vMin - wbb.vMin;
					//!! animatia de apa trebuie sa aiba frame 1 pt suprafata apei si 2 pentru luminile din apa
					CSprite spr(m_waterAnimIdx, wbb.vMin.x, wbb.vMin.y);
					spr.color = 0xffffffff;

					spr.currentFrame = 1; //linie apa
					spr.paintTiledOffset(&m_sprBack, wbb.vSize.x, -1, texoff.x, texoff.y);

					spr.currentFrame = 2; //lumini prin apa
					spr.pos.y = col->bbox.vMin.y;
					//alterneaza luminile intre ele	(animatie)
					float alpha = (sin(fLocalTimeline) + 1.0f) * 0.5f;
					spr.color = D3DCOLOR_FFFA(alpha);
					spr.paintTiledOffset(&m_sprBack, wbb.vSize.x, -1, texoff.x, 0.0f);
					spr.color = D3DCOLOR_FFFA(1.0f - alpha);
					spr.paintTiledOffset(&m_sprBack, wbb.vSize.x, -1, texoff.x + 64.0f, 0.0f);
				}
			}
		}
		m_pSprite->SetTransform(&g_matIdentity);
		*/
		///--- paint actives front layer ---
		D3DXMatrixAffineTransformation2D(&mattrans, 1.0f, NULL, 0.0f, &D3DXVECTOR2(-m_visibleArea.x, -m_visibleArea.y));
		m_pSprite->SetTransform(&mattrans);

		for (int kk = 0; kk < m_visibleList.visible_actives[K_LVL_LAYER_FRONT].Count(); kk++)
		{
			CActive *active = m_visibleList.visible_actives[K_LVL_LAYER_FRONT].m_pData[kk];
			if (active->flipX)
			{
				matlocal = mattrans;
				if (active->flipX)
				{
					matlocal._11 = -1.0f; //scalare X
					matlocal._41 += 2.0f * active->pos.x;// +2.0f * active_bbox.x + active_bbox.w; //e un calcul logic ca sa ramana incadrat in acelasi bbox real
				}
				m_pSprite->SetTransform(&matlocal);
				active->sprite.paint_firstModule(&m_sprActives);
				m_pSprite->SetTransform(&mattrans);
			}
			else
			{
				if (active->fAngle == 0.0f)
				{
					active->sprite.paint_firstModule(&m_sprActives);
				}
				else
				{
					//for now only front objects can be rotated... much optimization, such speed
					D3DXMatrixAffineTransformation2D(&matlocal, 1.0f, NULL, active->fAngle, &D3DXVECTOR2(active->pos.x - m_visibleArea.x, active->pos.y - m_visibleArea.y));

					m_pSprite->SetTransform(&matlocal);
					active->sprite.pos = D3DXVECTOR2(0.0f, 0.0f);
					active->sprite.paint_firstModule(&m_sprActives);
					m_pSprite->SetTransform(&mattrans);
				}
			}
		}
		///------ paint interactible front objects ------
		AdditiveBlendingON(m_pDevice, m_pSprite);
		for (int kk = 0; kk < m_visibleList.visible_actives[K_LVL_LAYER_FRONT].Count(); kk++)
		{
			CActive *active = m_visibleList.visible_actives[K_LVL_LAYER_FRONT].m_pData[kk];
			if (!active->bStandsOut)
				continue;

			DWORD colAlpha = D3DCOLOR_FFFA(fAlp);
			//object is being touched so show it
			if (((pPlayerActor[0] != null) && (active == pPlayerActor[0]->pClosestTouchable)) ||
				((pPlayerActor[1] != null) && (active == pPlayerActor[1]->pClosestTouchable)))
				colAlpha = D3DCOLOR_FFFA(fAlp2);


			if (active->flipX)
			{
				RECTXYWH active_bbox = m_sprActives.GetAFrameBBox_real(active->sprite.animationIdx, active->sprite.currentFrame);

				matlocal = mattrans;
				//pozitie sprite
				if (active->flipX)
				{
					matlocal._11 = -1.0f; //scalare X
					matlocal._41 += 2.0f * active->pos.x;// +2.0f * active_bbox.x + active_bbox.w; //e un calcul logic ca sa ramana incadrat in acelasi bbox real
				}
				m_pSprite->SetTransform(&matlocal);

				CSprite spr = active->sprite;
				spr.color = colAlpha;
				spr.paint_firstModule(&m_sprActives);

				m_pSprite->SetTransform(&mattrans);
			}
			else
			{
				if (active->fAngle == 0.0f)
				{
					CSprite spr = active->sprite;
					spr.color = colAlpha;
					spr.paint_firstModule(&m_sprActives);
				}
				else
				{
					D3DXMatrixAffineTransformation2D(&matlocal, 1.0f, NULL, active->fAngle, &D3DXVECTOR2(active->pos.x - m_visibleArea.x, active->pos.y - m_visibleArea.y));

					m_pSprite->SetTransform(&matlocal);
					CSprite spr = active->sprite;
					spr.color = colAlpha;
					spr.pos = D3DXVECTOR2(0.0f, 0.0f);
					spr.paint_firstModule(&m_sprActives);
					m_pSprite->SetTransform(&mattrans);
				}
			}
		}
		AdditiveBlendingOFF(m_pDevice, m_pSprite);

		
		m_pSprite->SetTransform(&g_matIdentity);
		m_pSprite->Flush(); //acest flush trebuie chemat neaparat

		///.////////////////////////////////////////////////////////
		///	NORMAL MAP
		///.////////////////////////////////////////////////////////
		//set clip on colormap
		aabbScissor.Move(D3DXVECTOR2(K_RTT_H_WIDTH, 0.0f));
		SetScissorClip(m_pDevice, aabbScissor.vMin.x, aabbScissor.vMin.y, aabbScissor.vSize.x, aabbScissor.vSize.y);

		///--- back tiles normal map
		m_pSprite->SetTransform(&g_matIdentity);
		for (int yy = 0; yy < m_visibleAreaTL.h; yy++)
		{
			for (int xx = 0; xx < m_visibleAreaTL.w; xx++)
			{
				int tlX = xx + m_visibleAreaTL.x - m_levelAABB_TL.x;
				int tlY = yy + m_visibleAreaTL.y - m_levelAABB_TL.y;
				CTile *tl = &tiles[tlX][tlY];
				if (tl->tileIDs[0] >= 0)
					m_pSprite->Draw(m_texManager.m_Texs[m_tilesTexNormIdx]->pTexture, &tl->srcRects[0], NULL, &D3DXVECTOR3(K_RTT_H_WIDTH + xx * tileW, yy * tileH, 0.0f), 0xffffffff);
			}
		}
		m_pSprite->Flush();

		///--- actives back layer ---
		m_pSprite->SetTransform(&mattrans);

		for (int kk = 0; kk < m_visibleList.visible_actives[K_LVL_LAYER_BACK].Count(); kk++)
		{
			CActive *active = m_visibleList.visible_actives[K_LVL_LAYER_BACK].m_pData[kk];

			if (active->flipX)
			{
				RECTXYWH active_bbox = m_sprActives.GetAFrameBBox_real(active->sprite.animationIdx, active->sprite.currentFrame);

				matlocal = mattrans;
				//pozitie sprite
				if (active->flipX)
				{
					matlocal._11 = -1.0f; //scalare X
					matlocal._41 += 2.0f * active->pos.x;// +2.0f * active_bbox.x + active_bbox.w; 
				}
				//move obj to normals part of the RT
				matlocal._41 += K_RTT_H_WIDTH;

				m_pSprite->SetTransform(&matlocal);
				active->sprite.paint_firstModule_texOverride(&m_sprActives, 1);
				m_pSprite->SetTransform(&mattrans);
			}
			else
			{
				active->sprite.paint_firstModule_texOverride(&m_sprActives, 1, K_RTT_H_WIDTH);
			}
		}
		m_pSprite->Flush();

		///--- middle tiles normal map
		m_pSprite->SetTransform(&g_matIdentity);
		for (int yy = 0; yy < m_visibleAreaTL.h; yy++)
		{
			for (int xx = 0; xx < m_visibleAreaTL.w; xx++)
			{
				int tlX = xx + m_visibleAreaTL.x - m_levelAABB_TL.x;
				int tlY = yy + m_visibleAreaTL.y - m_levelAABB_TL.y;
				CTile *tl = &tiles[tlX][tlY];

				if (tl->tileIDs[1] >= 0)
					m_pSprite->Draw(m_texManager.m_Texs[m_tilesTexNormIdx]->pTexture, &tl->srcRects[1], NULL, &D3DXVECTOR3(K_RTT_H_WIDTH + xx * tileW, yy * tileH, 0.0f), 0xffffffff);
			}
		}
		m_pSprite->Flush();

		///--- MIDDLE LAYER objects
		m_pSprite->SetTransform(&mattrans);
		for (int kk = 0; kk < m_visibleList.visible_actives[K_LVL_LAYER_MIDDLE].Count(); kk++)
		{
			CActive *active = m_visibleList.visible_actives[K_LVL_LAYER_MIDDLE].m_pData[kk];

			if (active->flipX)
			{
				matlocal = mattrans;
				//pozitie sprite
				if (active->flipX)
				{
					matlocal._11 = -1.0f; //scalare X
					matlocal._41 += 2.0f * active->pos.x;
				}
				//mut sprite pe zona de normale
				matlocal._41 += K_RTT_H_WIDTH;

				m_pSprite->SetTransform(&matlocal);
				active->sprite.paint_firstModule_texOverride(&m_sprActives, 1);
				m_pSprite->SetTransform(&mattrans);
			}
			else
			{
				active->sprite.paint_firstModule_texOverride(&m_sprActives, 1, K_RTT_H_WIDTH);
			}
		}
		m_pSprite->Flush();

		///--- front tiles normal map ---
		m_pSprite->SetTransform(&g_matIdentity);
		for (int yy = 0; yy < m_visibleAreaTL.h; yy++)
		{
			for (int xx = 0; xx < m_visibleAreaTL.w; xx++)
			{
				int tlX = xx + m_visibleAreaTL.x - m_levelAABB_TL.x;
				int tlY = yy + m_visibleAreaTL.y - m_levelAABB_TL.y;
				CTile *tl = &tiles[tlX][tlY];
				if (tl->tileIDs[2] >= 0)
					m_pSprite->Draw(m_texManager.m_Texs[m_tilesTexNormIdx]->pTexture, &tl->srcRects[2], NULL, &D3DXVECTOR3(K_RTT_H_WIDTH + xx * tileW, yy * tileH, 0.0f), 0xffffffff);
			}
		}
		m_pSprite->Flush();

		///--- paint RT particles - BACK ---
		m_pSprite->SetTransform(&g_matIdentity);
		g_particlesMgr.PaintLayerOffset_texOverride(K_PART_LAYER_RT_BACK_NRM, D3DXVECTOR2(-m_visibleArea.x + K_RTT_H_WIDTH, -m_visibleArea.y), false, 1);
		g_particlesMgr.PaintLayerOffset_texOverride(K_PART_LAYER_RT_BACK_NRM_LIGHT, D3DXVECTOR2(-m_visibleArea.x + K_RTT_H_WIDTH, -m_visibleArea.y), false, 1);

		///--- ACTORS NORMALS
		for (int kk = 0; kk < m_visibleList.visible_actors.Count(); kk++)
		{
			CActor* actor = m_visibleList.visible_actors.m_pData[kk];
			//aplic matricea de aliniere cu m_visibleArea
			matlocal = mattrans;
			//aplic matrice flipX daca este cazul
			if (actor->lookDirXsign == -1)
			{
				matlocal._11 = -1.0f;
				matlocal._41 += 2.0f * actor->sprite_feet.pos.x;
			}
			matlocal._41 += K_RTT_H_WIDTH;

			m_pSprite->SetTransform(&matlocal);
			//#HACK: when taking damage paint the color frame instead of the normals frame so it looks loghter
			int nTexOverride = 1;
			if (actor->nTookDamageFrames > 0)
				nTexOverride = 0;

			if ((actor->templateActor.bComposedAnimation) && (actor->sprite_feet.animationIdx >= 0))
			{
				actor->sprite_feet.color = actor->color;
				actor->sprite_feet.paint_firstModule_texOverride(&m_sprActors, nTexOverride);
			}
			actor->sprite.color = actor->color;
			actor->sprite.paint_firstModule_texOverride(&m_sprActors, nTexOverride);

			//--- muzzle flash ---
			if ((actor->pCurrentWeapon != null) && (actor->pCurrentWeapon->m_sprMuzzleFlash.animationIdx >= 0) && (actor->pCurrentWeapon->m_sprMuzzleFlash.animStatus != ANIM_STATUS_FRAMELOCK))
			{
				actor->pCurrentWeapon->m_sprMuzzleFlash.pos = actor->pos + actor->vecWeapon_abs[((actor->bCrouched) ? 1 : 0)];
				actor->pCurrentWeapon->m_sprMuzzleFlash.paint_firstModule_texOverride(&m_sprActors, 1);
			}
		}
		m_pSprite->Flush();

		///--- BULLETS NORMALS/self illumi ---
		matlocal = mattrans;
		matlocal._41 += K_RTT_H_WIDTH;
		m_pDevice->SetTransform(D3DTS_WORLD, &matlocal);
		PaintBullets(true);
		m_pSprite->Flush();
		m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);

		///--- paint RT particles - FRONT ---
		m_pSprite->SetTransform(&g_matIdentity);
		g_particlesMgr.PaintLayerOffset_texOverride(K_PART_LAYER_RT_FRONT_NRM, D3DXVECTOR2(-m_visibleArea.x + K_RTT_H_WIDTH, -m_visibleArea.y), false, 1);
		g_particlesMgr.PaintLayerOffset_texOverride(K_PART_LAYER_RT_FRONT_NRM_LIGHT, D3DXVECTOR2(-m_visibleArea.x + K_RTT_H_WIDTH, -m_visibleArea.y), false, 1);

		///--- paint actives front layer NORMALS ---
		m_pSprite->SetTransform(&mattrans);
		for (int kk = 0; kk < m_visibleList.visible_actives[K_LVL_LAYER_FRONT].Count(); kk++)
		{
			CActive *active = m_visibleList.visible_actives[K_LVL_LAYER_FRONT].m_pData[kk];

			if (active->flipX)
			{
				RECTXYWH active_bbox = m_sprActives.GetAFrameBBox_real(active->sprite.animationIdx, active->sprite.currentFrame);

				matlocal = mattrans;
				//pozitie sprite
				if (active->flipX)
				{
					matlocal._11 = -1.0f; //scalare X
					matlocal._41 += 2.0f * active->pos.x;// +2.0f * active_bbox.x + active_bbox.w; //e un calcul logic ca sa ramana incadrat in acelasi bbox real
				}
				//move sprite to right side of RT (normals)
				matlocal._41 += K_RTT_H_WIDTH;

				m_pSprite->SetTransform(&matlocal);
				active->sprite.paint_firstModule_texOverride(&m_sprActives, 1);
				m_pSprite->SetTransform(&mattrans);
			}
			else
			{
				if (active->fAngle == 0.0f)
				{
					//mut sprite pe zona de normale
					active->sprite.paint_firstModule_texOverride(&m_sprActives, 1, K_RTT_H_WIDTH);
				}
				else
				{
					//#TODO: daca ma hotarasc sa nu pun rotatii la obiecte scot partea asta. Momentan am rotatii doar pe front layer la active
					D3DXMatrixAffineTransformation2D(&matlocal, 1.0f, NULL, active->fAngle, &D3DXVECTOR2(active->pos.x + K_RTT_H_WIDTH - m_visibleArea.x, active->pos.y - m_visibleArea.y));

					m_pSprite->SetTransform(&matlocal);
					active->sprite.pos = D3DXVECTOR2(0.0f, 0.0f);
					active->sprite.paint_firstModule_texOverride(&m_sprActives, 1);
					m_pSprite->SetTransform(&mattrans);
				}
			}
		}
		m_pSprite->Flush();


		//close separate alpha blending
		if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0)
		{
			m_pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, false);
		}

		//end sprite
		m_pSprite->SetTransform(&g_matIdentity);
		m_pSprite->End();
		//end scene paint/pass
		V(m_pRenderToSurface->EndScene(0));
	}

	return hr;
}

HRESULT CLevel::PaintOffscreen_nothing()
{
	HRESULT hr = S_OK;
	//daca nu am capabilitatea de offscreen ies cu eroare
	if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_RTT) == 0)
		return E_FAIL;

	hr = m_pRenderToSurface->BeginScene(m_pRTSurface, NULL);
	if (SUCCEEDED(hr))
	{
		// Clear the render target and the zbuffer 
		V(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, K_GAME_CLEAR_COLOR, 1.0f, 0));

	}
	//end scene paint/pass
	V(m_pRenderToSurface->EndScene(0));
	return S_OK;
}


//local usable variables
static CSprite	sprInteract;

HRESULT CLevel::PaintComposition()
{
	HRESULT hr = S_OK;
	//daca nu e incarcat ies
	if ((!m_bLoaded) || (!m_bOneUpdateDone))
		return E_FAIL;
	//daca nu am capabilitatea de offscreen ies cu eroare
	if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_RTT) == 0)
		return E_FAIL;
	//ps/vs generice
	LPDIRECT3DVERTEXSHADER9 pVShader = null;
	LPDIRECT3DPIXELSHADER9 pPShader = null;
	//--- RENDER LEVEL in compositing RT - COMPOZITIE ---
	hr = m_pRT_final->BeginScene(m_pRTSurface_final, NULL);
	if (SUCCEEDED(hr))
	{
		// Clear the render target and the zbuffer 
		V(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, K_GAME_CLEAR_COLOR, 1.0f, 0));

		//D3DXMATRIXA16 matProj;
		//D3DXMatrixOrthoOffCenterLH(&matProj, 0.5f, K_RTFINAL_WIDTH + 0.5f, K_RTFINAL_HEIGHT + 0.5f, 0.5f, 0.0f, 1.0f);
		//pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);

		//set 2d states
		m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
		m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

		m_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
		//DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"Game");

		//OBJECT_SPACE se foloseste ca sa nu modifice matricea de proiectie
		m_pSprite->Begin(D3DXSPRITE_ALPHABLEND | /*D3DXSPRITE_OBJECTSPACE |*/ D3DXSPRITE_DONOTSAVESTATE);

		m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

		///--- PAINT LEVEL ----
		Paint();

		///--- end everything ---
		m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
		m_pSprite->SetTransform(&g_matIdentity); //don't remove
		//-- signal END for main gamesprite ---
		m_pSprite->End();

		//!OPTIMIZARE driver: unbind all resource channels
		m_pDevice->SetTexture(0, NULL);
		m_pDevice->SetTexture(1, NULL);
		m_pDevice->SetStreamSource(0, NULL, 0, 0);
		m_pDevice->SetVertexShader(null);
		m_pDevice->SetPixelShader(null);

		V(m_pRT_final->EndScene(0));
	}
	else
	{
		ErrorBox(K_ERR_WARNING, L"PaintComposition failed rendering to RT!");
	}

	return hr;
}

HRESULT CLevel::PaintComposition_nothing()
{
	HRESULT hr = S_OK;
	//no good caps
	if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_RTT) == 0)
		return E_FAIL;

	hr = m_pRT_final->BeginScene(m_pRTSurface_final, NULL);
	if (SUCCEEDED(hr))
	{
		// Clear the render target and the zbuffer 
		V(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, K_GAME_CLEAR_COLOR, 1.0f, 0));
	}

	V(m_pRT_final->EndScene(0));
	return S_OK;
}

void CLevel::Paint()
{
	//daca nu e incarcat ies
	if ((!m_bLoaded) || (!m_bOneUpdateDone))
		return;

	LPDIRECT3DVERTEXSHADER9 pVShaderTEXspot = null;
	LPDIRECT3DPIXELSHADER9 pPShader = null;

	//1. set active camera
	m_pSprite->Flush(); //chem un flush ca sa fiu sigur ca nu intru peste ce s-a desenat inainte
	CCameraTransform::SetActiveCamera(m_pDevice, &m_camLevel);
	D3DXMATRIXA16 matCam = m_camLevel.GetViewTransform(); //matricea camerei

	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	//1. paint level background
	//PaintBackground();
	//--- paint thunder ---
	if ((m_fThunderTimer > 0.0f) && (m_fThunderTimer < 0.4f) && (randint(1000) < 500) && (!UTGetControlsManager().bIsBlocking) && (!DXUTIsTimePaused()) && (!m_bInsideHiddenRoom))
	{
		AdditiveBlendingON(m_pDevice, m_pSprite);

		D3DXMATRIXA16 mattranslg;
		RECTXYWH_F bbox = m_sprInterface.GetAFrameBBox_real(ANM_IGM_INTERFACE_SPR_VIGNETTES, 0);
		D3DXMatrixAffineTransformation2D(&mattranslg, UTGetAppClass().g_rectRender.w / bbox.w, NULL, 0.0f, &D3DXVECTOR2(0.0f, 0.0f));
		m_pSprite->SetTransform(&mattranslg);
		float alpha = 0.4f + randfloat(0.6f);
		CSprite::paintFrame(&m_sprInterface, 0.0f, 0.0f, ANM_IGM_INTERFACE_SPR_VIGNETTES, 0, D3DCOLOR_FFFA(alpha));
		m_pSprite->SetTransform(&g_matIdentity);

		AdditiveBlendingOFF(m_pDevice, m_pSprite);
	}


	//set textures, states and shaders
	m_pDevice->SetTexture(0, m_pRTTexture);
	//#HACK: daca am mai multe texturi de lumina trebuie schimbat settexture sa ia pentru fiecare lumina textura ei. Daca am o singura textura merge foarte bine asa
	m_pDevice->SetTexture(1, m_sprLights.Textures[0]->pTex);

	D3DXMATRIXA16 matWVP = matCam * UTGetAppClass().g_matProj;
	//vertex shaderulis the same for everything
	pVShaderTEXspot = UTGetShaderManager().GetVShaderByName(L"VS_TEXspot");
	m_pDevice->SetVertexShader(pVShaderTEXspot);

	m_pDevice->SetVertexDeclaration(UTGetShaderManager()._VERTEX_PNCT4T4_decl);
	float fConstDataVS[][4] = {
		{ m_visibleArea.x, m_visibleArea.y, m_visibleArea.w, m_visibleArea.h },//RTT rect_xywh in world coords
		{ 0.0f, 0.0, m_visibleArea.w / K_RTT_WIDTH, m_visibleArea.h / K_RTT_HEIGHT } //RTT rect_xywh in tex coords
	};
	m_pDevice->SetVertexShaderConstantF(0, (float*)&matWVP, 4);
	m_pDevice->SetVertexShaderConstantF(4, (float*)fConstDataVS, ARRAY_SIZE(fConstDataVS));


	///--- pixel shader ambient light with self illumination ---
	pPShader = UTGetShaderManager().GetPShaderByName(L"PS_AMBIENT_SI");
	m_pDevice->SetPixelShader(pPShader);

	float fConstDataAmbientPS[][4] = { { 0.0f, 0.0f, 0.0, 0.0 } };//no data
	m_pDevice->SetPixelShaderConstantF(0, (float*)fConstDataAmbientPS, ARRAY_SIZE(fConstDataAmbientPS));
	//2. paint level ambient lights - no additive so it only supports one (the last one)
	for (int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++)
	{
		CLight *nl = m_visibleList.visible_lights.m_pData[kk];
		if (nl->type == K_LVL_LIGHT_AMBIENTAL)
		{
			//paint and exit
			m_bufferedPainter.DrawMesh(nl->m_nLightMeshIdx, false);
			break;
		}
	}
	//set textures, states and shaders
	m_pDevice->SetTexture(0, m_pRTTexture);
	//#HACK: daca am mai multe texturi de lumina trebuie schimbat settexture sa ia pentru fiecare lumina textura ei. Daca am o singura textura merge foarte bine asa
	m_pDevice->SetTexture(1, m_sprLights.Textures[0]->pTex);

	///--- paint back without self illumination ---
	/*
	//RECT srcrect;
	//SetRect(&srcrect, 0, 0, m_visibleAreaTL.w * tileW, m_visibleAreaTL.h * tileH);
	//D3DXVECTOR3 bgpos(m_visibleArea.x, m_visibleArea.y, 0.0f);
	//m_pSprite->Draw(m_pRTTexture, &srcrect, NULL, &bgpos, m_colAmbientGlobal);
	//m_pSprite->Flush();
	*/
	///--- paint lights ---
	AdditiveBlendingON(m_pDevice, NULL);
	//pixel shader
	pPShader = UTGetShaderManager().GetPShaderByName(L"PS_TEXspot");
	m_pDevice->SetPixelShader(pPShader);

	///--- 3.paint lights ---
	CFixedArray<int, 64> arrLightsShadIdx; //shadowing lights
	arrLightsShadIdx.Clear();
	//draw non shadowing lights
	for (int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++)
	{
		CLight *nl = m_visibleList.visible_lights.m_pData[kk];
		//ambiental already painted
		if (nl->type == K_LVL_LIGHT_AMBIENTAL)
			continue; 

		if (nl->castShadows)
		{
			int idx = arrLightsShadIdx.Add(kk);
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
			if (idx < 0)
				ErrorBox(K_ERR_WARNING, L"[Warning]Too many shadowing lights onscreen!");
#endif			
			continue; //shadowing lights get painted below
		}
		//set shader constants
		float fConstData[][4] = {	{ K_LVL_LIGHTRENDER_SPECULAR_POWER, K_LVL_LIGHTRENDER_SPECULAR_INTENSITY, nl->fIntensity, 0.0 },//fSpecularPower (30.0f), fSpecularIntensity(0.5f), fLightIntensity
									{ K_LVL_LIGHTRENDER_SPOT_DIFFUSE_MUL, K_LVL_LIGHTRENDER_SPOT_COLOR_DODGE_ALPHA * nl->fVolumeAlpha, K_LVL_LIGHTRENDER_SPOT_LINEAR_DODGE_ALPHA * nl->fVolumeAlpha, 0.0 } };  //x=diffuse multiplier, y=spot color dodge layer opacity, z=spot linear dodge opacity
		m_pDevice->SetPixelShaderConstantF(0, (float*)fConstData, ARRAY_SIZE(fConstData));

		m_bufferedPainter.DrawMesh(nl->m_nLightMeshIdx, false);
	}

	///--- paint non shadowing bullets lights ---
	if (m_bufferedPainter.GetTrisCount(m_propsLightsMeshIdx) > 0)
	{
		float fConstData[][4] = {	{ K_LVL_LIGHTRENDER_SPECULAR_POWER, K_LVL_LIGHTRENDER_SPECULAR_INTENSITY, 1.0f, 0.0 },//fSpecularPower (30.0f), fSpecularIntensity(0.5f), fLightIntensity
									{ 1.0f, 0.2f, 0.05f, 0.0 } }; //x=diffuse multiplier, y=color dodge layer opacity, z=spot linear dodge opacity
		m_pDevice->SetPixelShaderConstantF(0, (float*)fConstData, ARRAY_SIZE(fConstData));

		m_bufferedPainter.DrawMesh(m_propsLightsMeshIdx, false);
	}

	///--- stencil shadows lights:
	m_pDevice->SetVertexShader(NULL);
	m_pDevice->SetPixelShader(NULL);

	///--- paint stencil masks ---
	int nLightsPasses = arrLightsShadIdx.Count() / UTGetAppClass().g_stencilBits;
	if ((arrLightsShadIdx.Count() % UTGetAppClass().g_stencilBits) > 0)
		nLightsPasses++;

	for (int nPass = 0; nPass < nLightsPasses; nPass++)
	{
		//prepare stencil - poligoane netexturate
		m_pDevice->SetFVF(_VERTEX_PNCT4T4::FVF);
		m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, false);
		m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		m_pDevice->SetTexture(0, NULL);
		m_pDevice->SetTexture(1, NULL);

		//desenez poly masca
		m_pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);

		// Enable stencil testing
		m_pDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);

		//reset stencil intre pass-urile succesive
		if (nPass > 0)
		{
			m_pDevice->Clear(0, NULL, D3DCLEAR_STENCIL, K_GAME_CLEAR_COLOR, 1.0f, 0);
			/*
			//clearing without calling Clear - IT WORKS
			m_pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
			m_pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_ZERO);

			//---TODO: need fast way to draw a fullscreen poly to clear the stencil to 0 (should be faster than calling Clear on the device)
			RECT rct;
			SetRect(&rct, UTGetAppClass().g_renderRect.x, UTGetAppClass().g_renderRect.y, UTGetAppClass().g_renderRect.Right(), UTGetAppClass().g_renderRect.Bottom());
			DrawRectUP_TL1T(m_pDevice, rct, D3DXVECTOR2(0.0f, 0.0f), D3DXVECTOR2(1.0f, 1.0f), 0xffffffff);
			m_pDevice->SetFVF(_VERTEX_PNCT4T4::FVF);
			*/
		}

		// Specify the stencil comparison function
		m_pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
		m_pDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
		m_pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_REPLACE);
		m_pDevice->SetRenderState(D3DRS_STENCILREF, 0xff);

		//part of the array that we process in this pass
		int nfrom = nPass * UTGetAppClass().g_stencilBits;
		int nto = nfrom + UTGetAppClass().g_stencilBits;
		if (nto > arrLightsShadIdx.nCount)
			nto = arrLightsShadIdx.nCount;

		int lightsCnt = 0;
		for (int kk = nfrom; kk < nto; kk++)
		{
			int nLgIdx = arrLightsShadIdx[kk];
			CLight *nl = m_visibleList.visible_lights.m_pData[nLgIdx];
			//if (nl->castShadows) //all lights in this array cast shadows
			{
				DWORD lightMask = (1 << lightsCnt);
				// scrie cate un bit pt fiecare umbra
				m_pDevice->SetRenderState(D3DRS_STENCILWRITEMASK, lightMask);
				//paint shadow volumes
				m_bufferedPainter.DrawMesh(nl->m_nShadowMeshIdx, false);

				lightsCnt++;
			}
		}

		///---paint lights through stencil
		m_pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0xf);
		m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
		m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true);
		//setez stencil sa refuze pixelii in afara zonei
		m_pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
		m_pDevice->SetRenderState(D3DRS_STENCILREF, 0x0);
		m_pDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_KEEP);
		m_pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

		//set textures and shaders
		//vshader
		m_pDevice->SetVertexShader(pVShaderTEXspot); //deja salvat la inceputul functiei
		m_pDevice->SetVertexDeclaration(UTGetShaderManager()._VERTEX_PNCT4T4_decl);
		m_pDevice->SetVertexShaderConstantF(0, (float*)&matWVP, 4);
		m_pDevice->SetVertexShaderConstantF(4, (float*)fConstDataVS, ARRAY_SIZE(fConstDataVS));
		//render target texture on level 0
		m_pDevice->SetTexture(0, m_pRTTexture);
		//light spot texture on level 1
		//#TODO: if we have more textures for the lights we'll need to set the right texture for each light
		m_pDevice->SetTexture(1, m_sprLights.Textures[0]->pTex);
		//pixel shader
		m_pDevice->SetPixelShader(pPShader); //should be already set anyway

		lightsCnt = 0;
		for (int kk = nfrom; kk < nto; kk++)
		{
			int nLgIdx = arrLightsShadIdx[kk];
			CLight *nl = m_visibleList.visible_lights.m_pData[nLgIdx];

			DWORD lightMask = (1 << lightsCnt);
			m_pDevice->SetRenderState(D3DRS_STENCILMASK, lightMask);
			//set shader constants
			float fConstData[][4] = {	{ K_LVL_LIGHTRENDER_SPECULAR_POWER, K_LVL_LIGHTRENDER_SPECULAR_INTENSITY, nl->fIntensity, 0.0 },//fSpecularPower (30.0f), fSpecularIntensity(0.5f), fLightIntensity
										{ K_LVL_LIGHTRENDER_SPOT_DIFFUSE_MUL, K_LVL_LIGHTRENDER_SPOT_COLOR_DODGE_ALPHA * nl->fVolumeAlpha, K_LVL_LIGHTRENDER_SPOT_LINEAR_DODGE_ALPHA * nl->fVolumeAlpha, 0.0 } };  //x=diffuse multiplier, y=spot color dodge layer opacity, z=spot linear dodge opacity
			m_pDevice->SetPixelShaderConstantF(0, (float*)fConstData, ARRAY_SIZE(fConstData));
			//paint light
			m_bufferedPainter.DrawMesh(nl->m_nLightMeshIdx, false);

			lightsCnt++;
		}

		m_pDevice->SetVertexShader(NULL);
		m_pDevice->SetPixelShader(NULL);
	}

	//end stencil ops
	m_pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
	
	//--- paint front particles layer ---
	AdditiveBlendingOFF(m_pDevice, m_pSprite);
	g_particlesMgr.PaintLayer(K_PART_LAYER_FRONT, false);

	m_pSprite->SetTransform(&g_matIdentity);
	m_pSprite->Flush();

	///--- paint additive particles layer ---
	g_particlesMgr.PaintLayer(K_PART_LAYER_FRONT_LIGHT, true);

	m_pSprite->SetTransform(&g_matIdentity);

	//am terminat cu luminile si self-illumination scot additive blending si fac flush (flush se face in AdditiveBlendingOff)
	AdditiveBlendingOFF(m_pDevice, m_pSprite);

	//final flush. do not remove!
	m_pSprite->Flush();
	m_pSprite->SetTransform(&g_matIdentity);
}

HRESULT CLevel::PaintUsingFinalRTT()
{
	HRESULT hr = S_OK;
	//daca nu e incarcat ies
	if ((!m_bLoaded) || (!m_bOneUpdateDone))
		return E_FAIL;
	//daca nu am capabilitatea de offscreen ies cu eroare
	if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_RTT) == 0)
		return E_FAIL;
	//ps/vs generice
	LPDIRECT3DVERTEXSHADER9 pVShader = null;
	LPDIRECT3DPIXELSHADER9 pPShader = null;

	CCameraTransform::SetActiveCamera(m_pDevice, &m_camLevel);
	//get camera data
	RECTXYWH_F camrect = m_camLevel.GetCamWorldAABB();
	D3DXMATRIXA16 matCam = m_camLevel.GetViewTransform();
	//CAABB al camerei
	CAABB		camAABB;
	camAABB.Set(D3DXVECTOR2(camrect.x, camrect.y), D3DXVECTOR2(camrect.Right(), camrect.Bottom()));

	///--- paint water ---
	if (m_bufferedPainter.GetTrisCount(m_waterMeshIdx) > 0)
	{
		//set textures, states and shaders
		assert(m_waterTexIdx >= 0);

		m_pDevice->SetTexture(0, m_pRTTexture_final);
		m_pDevice->SetTexture(1, m_texManager.m_Texs[m_waterTexIdx]->pTexture); //textura apa

		D3DXMATRIXA16 matWVP = matCam * UTGetAppClass().g_matProj;
		//vertex shaderul e acelasi pt toate
		pVShader = UTGetShaderManager().GetVShaderByName(L"VS_WATER");
		m_pDevice->SetVertexShader(pVShader);
		m_pDevice->SetVertexDeclaration(UTGetShaderManager()._VERTEX_PNCT4T4_decl);

		float fang = fLocalTimeline;
		if (fang >= 1000.0f * PI)
			fang -= 1000.0f * PI;
		D3DXVECTOR2 woff1(0.05f * sin(fang * 1.0f), 0.04f * cos(fang * 1.0f));
		D3DXVECTOR2 woff2(0.5f - 0.06f * sin(-fang * 0.63f), 0.5f - 0.05f * cos(-fang * 0.67f));
		float fConstDataVS[][4] = {
			{ camrect.x, camrect.y, camrect.w, camrect.h },//RTT rect_xywh
			{ 0.0f, 0.0, 1.0f, 1.0f }, //RTT rect_xywh in tex coords
			{ woff1.x, woff1.y, woff2.x, woff2.y } //fWaterOffsets (xy, wh sunt 2 vectori care misca textura de apa, textura ce vine suprapusa in shader)
		};
		m_pDevice->SetVertexShaderConstantF(0, (float*)&matWVP, 4);
		m_pDevice->SetVertexShaderConstantF(4, (float*)fConstDataVS, ARRAY_SIZE(fConstDataVS));

		///--- pixel shader ambient light cu self illumination ---
		pPShader = UTGetShaderManager().GetPShaderByName(L"PS_WATER");
		m_pDevice->SetPixelShader(pPShader);
		float fConstDataPS[][4] = { { 0.015f, 0.6f, 0.0, 0.0 } };//x=distort(0.1f), y=caustics alpha(0-2)
		m_pDevice->SetPixelShaderConstantF(0, (float*)fConstDataPS, ARRAY_SIZE(fConstDataPS));

		m_bufferedPainter.DrawMesh(m_waterMeshIdx, false);

		m_pDevice->SetVertexShader(null);
		m_pDevice->SetPixelShader(null);
	}

	//#HARDCODE: laser sight drawing for players 
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if (pPlayerActor[kk] == null)
			continue;
		if ((pPlayerActor[kk] != NULL) && (pPlayerActor[kk]->pCurrentWeapon->bPaintLaserSight))
		{
			float fRayLen = 0.0f;

			D3DXVECTOR2 vfrom = pPlayerActor[kk]->GetPosWeapon();
			D3DXVECTOR2 vto = vfrom;
			vto.x += pPlayerActor[kk]->lookDirXsign * pPlayerActor[kk]->templateActor.distSee;
			//coliziunea cu nivelul
			D3DXVECTOR2 collisionPoint, collisionNormal;
			CCollisionShape* colShape = ColShape_Segment_Intersection_Arr(vfrom, vto, m_visibleList.logic_colShapes.m_pData, m_visibleList.logic_colShapes.Count(), &collisionPoint, &collisionNormal);
			if (colShape != null)
			{
				vto = collisionPoint;
				fRayLen = fabs(vto.x - vfrom.x);
			}
			//coliziunea cu inamicii
			for (int ll = 0; ll < m_visibleList.visible_actors.Count(); ll++)
			{
				D3DXVECTOR2 retpt;
				CActor* enemy = m_visibleList.visible_actors.m_pData[ll];
				if (enemy->templateActor.actorClass < K_LVL_ACT_CLASS_HUMAN)
					continue;
				if ((enemy->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET) != 0)
					continue;

				CAABB actaabb = enemy->bbox;
				if (AABB_Segment_Intersection(vfrom, vto, actaabb, &retpt))
				{
					//vto se scurteaza pana cand nu mai colizioneaza cu nimic
					vto = retpt;
				}
			}

			//too short? don't draw
			if (fabs(vto.x - vfrom.x) < 1.0f)
				continue;

			if (vto.x < vfrom.x)
				SWAP(vfrom, vto);

			CSprite laserspr(ANM_ACTIVES_SPR_BULLETS_FIRE, vfrom);
			laserspr.currentFrame = 3;
			laserspr.color = 0xffff0000;

			//m_pSprite->SetTransform(&mattrans);
			laserspr.paintTiled(&m_sprActives, vto.x - vfrom.x);
			//capete raza laser
			CSprite::paintFrame(&m_sprActives, vfrom.x, vfrom.y, ANM_ACTIVES_SPR_BULLETS_FIRE, 0, 0xffff0000);
			CSprite::paintFrame(&m_sprActives, vto.x, vto.y, ANM_ACTIVES_SPR_BULLETS_FIRE, 0, 0xffff0000);
		}
	}


	//--- actors icons and stun stars ---
	for (int kk = 0; kk < m_visibleList.visible_actors.Count(); kk++)
	{
		CActor* act = m_visibleList.visible_actors.m_pData[kk];
		//shield/overhead icons for non players
		if ((act->templateActor.actorClass != K_LVL_ACT_CLASS_PLAYER) && (act->m_sprOverheadIcon.animationIdx >= 0))
		{
			act->m_sprOverheadIcon.pos = act->posHeart;
			act->m_sprOverheadIcon.paint(&m_sprInterface);
		}
		//overhead icon !!! only if no overhead icon set (hence the else)
		else if (act->nIconType != K_LVL_ACT_ICON_NONE)
		{
			CSprite::paintFrame(&m_sprInterface, act->posHeart.x, act->posHeart.y, ANM_IGM_INTERFACE_SPR_ACTOR_ICONS, act->nIconType);
		}

		//STUN STARS
		if (act->fStunTimer >= K_LVL_MIN_STUN_DIZZY_DURATION)
		{
			int curframe = int(fLocalTimeline * 25.0f) % g_particlesMgr.m_sprCol.GetAFramesCnt(ANM_PARTICLES_SPR_STUN_STARS);
			D3DXVECTOR2 vStarsPos = act->GetPosHeart();
			CSprite::paintFrameModule(&g_particlesMgr.m_sprCol, vStarsPos.x, vStarsPos.y - 10.0f, ANM_PARTICLES_SPR_STUN_STARS, curframe, 0, act->color);
		}

		//LASER SIGHT - for non players
		if ((act->pCurrentWeapon->bPaintLaserSight) && (act->m_AIsensorInfo.pTargetedActor != null) && (act->templateActor.actorClass != K_LVL_ACT_CLASS_PLAYER))
		{
			float fRayLen = 0.0f;

			D3DXVECTOR2 vfrom = act->GetPosWeapon();
			D3DXVECTOR2 vto = vfrom;
			vto.x += act->lookDirXsign * act->templateActor.distSee;
			//coliziunea cu nivelul
			D3DXVECTOR2 collisionPoint, collisionNormal;
			CCollisionShape* colShape = ColShape_Segment_Intersection_Arr(vfrom, vto, m_visibleList.logic_colShapes.m_pData, m_visibleList.logic_colShapes.Count(), &collisionPoint, &collisionNormal);
			if (colShape != null)
			{
				vto = collisionPoint;
				fRayLen = fabs(vto.x - vfrom.x);
			}
			//coliziunea cu inamicul targetat
			D3DXVECTOR2 retpt;
			CActor* enemy = act->m_AIsensorInfo.pTargetedActor;

			CAABB actaabb = enemy->bbox;
			if (AABB_Segment_Intersection(vfrom, vto, actaabb, &retpt))
			{
				//vto se scurteaza pana cand nu mai colizioneaza cu nimic
				vto = retpt;
			}

			//daca e prea scurt nu mai desenez raza
			if (fabs(vto.x - vfrom.x) > 1.0f)
			{
				if (vto.x < vfrom.x)
					SWAP(vfrom, vto);

				CSprite laserspr(ANM_ACTIVES_SPR_BULLETS_FIRE, vfrom);
				laserspr.currentFrame = 3;
				laserspr.color = 0xff00ff00;

				//m_pSprite->SetTransform(&mattrans);
				laserspr.paintTiled(&m_sprActives, vto.x - vfrom.x);
				//capete raza laser
				CSprite::paintFrame(&m_sprActives, vfrom.x, vfrom.y, ANM_ACTIVES_SPR_BULLETS_FIRE, 0, 0xff00ff00);
				CSprite::paintFrame(&m_sprActives, vto.x, vto.y, ANM_ACTIVES_SPR_BULLETS_FIRE, 0, 0xff00ff00);
			}
		}


		//--- paint DoT for static effects ---
		switch (act->cDamageOverTime.eType)
		{
			case CDamageOverTime::K_LVL_DoT_TARGETED_ALLY:
			case CDamageOverTime::K_LVL_DoT_TARGETED:
			{
				float fp = cos(act->cDamageOverTime.fDuration * 4.0f);
				float fAlphaHeads = MATH_GetAlphaOnDomainEnds(act->cDamageOverTime.fDuration, act->cDamageOverTime.fDuration_ini, 0.2f);
				//different colors for both effects
				DWORD dwCol = D3DCOLOR_COLORALPHA(0xffff0000, fAlphaHeads);
				if (act->cDamageOverTime.eType == CDamageOverTime::K_LVL_DoT_TARGETED_ALLY)
					dwCol = D3DCOLOR_COLORALPHA(0xff00ff00, fAlphaHeads);


				//find closest CAM BALL
				float fMinDist = 200.0f;
				CBullet *pCamball = null;
				for (int ll = 0; ll < m_arrBulletsTemp.Count(); ll++)
				{
					CBullet* bul = m_arrBulletsTemp.m_pData[ll];
					if (bul->type != K_LVL_BULLET_CAM_BALL)
						continue;
					if (!bul->physPt->m_data.bIsStatic)
						continue;
					D3DXVECTOR2 vBulPos = bul->physPt->m_data.pos;
					float fDist = D3DXVec2Length(&(vBulPos - act->posHeart));
					if (fDist <= fMinDist)
					{
						fMinDist = fDist;
						pCamball = bul;
					}
				}
				if (pCamball != null)
				{
					D3DXVECTOR2 vBulPos = pCamball->physPt->m_data.pos + pCamball->physPt->m_data.contactNormal * 2.0f;
					D3DXVECTOR2 vDir = act->posHeart - vBulPos;
					D3DXVec2Normalize(&vDir, &vDir);
					CSprite spr(ANM_IGM_INTERFACE_SPR_LINES_H, 0.0f, 0.0f);
					spr.currentFrame = 1;
					spr.color = D3DCOLOR_COLORALPHA(dwCol, 0.4f + 0.15f * sin(fLocalTimeline * 3.0f));
					spr.paintTiledHOriented(&m_sprInterface, vBulPos + vDir * 2.0f, act->posHeart);
				}

				//paint normal markings
				CSprite::paintFrame(&m_sprInterface, act->bbox.vMin.x - fabs(2.0f * fp), act->bbox.vMax.y, ANM_IGM_INTERFACE_SPR_IGM_STRATEGIC_EFFECTS, 2, dwCol);
				CSprite::paintFrame(&m_sprInterface, act->bbox.vMax.x + fabs(2.0f * fp), act->bbox.vMax.y, ANM_IGM_INTERFACE_SPR_IGM_STRATEGIC_EFFECTS, 3, dwCol);
			}
			break;
			case CDamageOverTime::K_LVL_DoT_SNIPER_TARGET:
			{
				D3DXVECTOR2 vTarget = act->GetPosHeart();
				D3DXMATRIXA16 matt;
				float fAlpha = 0.0f;
				float fScale = 0.8f;
				float fRotation = 0.0f;
				if (act->cDamageOverTime.fDuration <= 1.0f)
				{
					fAlpha = 1.0f - act->cDamageOverTime.fDuration;
				}
				if ((act->cDamageOverTime.fDuration > 0.4f) && (act->cDamageOverTime.fDuration < 1.0f))
				{
					fRotation = (act->cDamageOverTime.fDuration - 0.4f) * PI;
					fScale = 0.8f * (1.0f + act->cDamageOverTime.fDuration - 0.4f);
				}
				D3DXMatrixAffineTransformation2D(&matt, fScale, NULL, fRotation, &vTarget);
				m_pSprite->SetTransform(&matt);
				CSprite::paintFrame(&m_sprInterface, 0.0f, 0.0f, ANM_IGM_INTERFACE_SPR_IGM_STRATEGIC_EFFECTS, 0, D3DCOLOR_FFFA(fAlpha));
				m_pSprite->SetTransform(&g_matIdentity);
			}
			break;
		}

		//energy bars
		if ((act->templateActor.actorClass == K_LVL_ACT_CLASS_HUMAN) && (act->fLife > 0.0f) &&
			(act->templateActor.fLife > 100.0f) && (act->m_AIsensorInfo.fTimeSinceHit < 5.0f))
		{
			float fLife = act->fLife / act->templateActor.fLife;
			float fBarLen = act->templateActor.fLife / 5.0f;
			CLAMP(fBarLen, 40.0f, 60.0f);

			RECTXYWH barrect(act->bbox.vCenter.x - fBarLen / 2.0f, act->bbox.vMax.y + 3.0f, fBarLen, 8.0f);
			CtrlMgrDrawProgress_HeadsOutside(&m_sprInterface, ANM_IGM_INTERFACE_SPR_PROGRESS_HEALTH, barrect, fLife, 0xffffffff);
		}
	}

	//-- final flush for level space ---
	m_pSprite->Flush();

	///--- paint Fog Of War ---
	if (m_bufferedPainter.GetTrisCount(m_fogofwarMeshIdx) > 0)
	{
		//set textures, states and shaders
		//arata mai bine cu point filtering
		m_pDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		m_pDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		m_pDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

		m_pDevice->SetTexture(0, m_pRTTexture_final);
		m_pDevice->SetTexture(1, null);// m_texManager.m_Texs[m_fogofwarTexIdx]->pTexture); //textura FOW

		D3DXMATRIXA16 matWVP = matCam * UTGetAppClass().g_matProj;
		//vertex shaderul e acelasi pt toate
		pVShader = UTGetShaderManager().GetVShaderByName(L"VS_FOW");
		m_pDevice->SetVertexShader(pVShader);
		m_pDevice->SetVertexDeclaration(UTGetShaderManager()._VERTEX_PNCT4T4_decl);

		float fConstDataVS[][4] = {
			{ camrect.x, camrect.y, camrect.w, camrect.h },//RTT rect_xywh
			{ 0.0f, 0.0, 1.0f, 1.0f } //RTT rect_xywh in tex coords
		};
		m_pDevice->SetVertexShaderConstantF(0, (float*)&matWVP, 4);
		m_pDevice->SetVertexShaderConstantF(4, (float*)fConstDataVS, ARRAY_SIZE(fConstDataVS));

		///--- pixel shader ambient light cu self illumination ---
		pPShader = UTGetShaderManager().GetPShaderByName(L"PS_FOW");
		m_pDevice->SetPixelShader(pPShader);
		float fConstDataPS[][4] = { { 10.0f, 0.0f, 0.0f, 0.0 } };//x=distort(0.1f)
		m_pDevice->SetPixelShaderConstantF(0, (float*)fConstDataPS, ARRAY_SIZE(fConstDataPS));

		m_bufferedPainter.DrawMesh(m_fogofwarMeshIdx, false);

		m_pDevice->SetVertexShader(null);
		m_pDevice->SetPixelShader(null);
	}

	///--- paint front layer parallax objects with linear blending ---
	/*
	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	D3DXMATRIXA16 matfront;
	CCameraTransform::SetActiveCamera(m_pDevice, &m_camLevel);

	for (int kk = 0; kk < m_arrMiscObjects.Count(); kk++)
	{
		if (m_arrMiscObjects[kk]->type != K_LVL_MISC_FRONTLAYEROBJ)
			continue;
		//desenez cu scalare
		CMiscObject_FrontLayerObj* obj = dynamic_cast<CMiscObject_FrontLayerObj*>(m_arrMiscObjects[kk]);
		if (obj == null)
		{
			continue;
		}

		D3DXVECTOR3 campos = m_camLevel.GetCamPos();
		D3DXVECTOR2 off(obj->pos.x - campos.x, obj->pos.y - campos.y);
		off *= K_LVL_FRONTLAYER_PARALLAX; //front layer moves faster
										  //compute final aabb - visibility test
		CAABB finalaabb = obj->aabb_ini;
		finalaabb.vMin *= K_LVL_FRONTLAYER_SCALING; finalaabb.vMax *= K_LVL_FRONTLAYER_SCALING;
		finalaabb.Move(obj->pos + off);
		if (!finalaabb.Intersects(&camAABB))
			continue;

		D3DXMatrixAffineTransformation2D(&matfront, K_LVL_FRONTLAYER_SCALING, NULL, 0.0f, &(obj->pos + off));
		m_pSprite->SetTransform(&matfront);
		obj->sprite.paint(&m_sprBack);
	}
	m_pSprite->Flush();

	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	*/

	///--- paint string particles in level coords ---
	UTGetFontsManager().SetPauseOnTTFontsReplacement(true);
	g_particlesMgr.PaintStringParticles(K_PART_LAYER_NORMAL);
	UTGetFontsManager().SetPauseOnTTFontsReplacement(false);
	m_pSprite->Flush();

	//--- closest touchable and cover icons ---
	IActiveInterface * pLastPaintedTarget = null; //pointer la ultimul activ caruia i-am desenat interfata ca sa nu o desenez de 2 ori
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{

#if defined(_DEBUG) || defined(DEBUG)
		//respawn point painting
		if ((pPlayerActor[kk] != null) && (pPlayerActor[kk]->fLife <= 0.0f))
		{
			if (m_arrStats[K_LVL_STATS_PL1_LIVES + pPlayerActor[kk]->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT] > 0)
				CSprite::paintFrame(&m_sprInterface, m_arrPlayerLastSafePos[kk].x, m_arrPlayerLastSafePos[kk].y - 10.0f - 5.0f * sin(fLocalTimeline * 4.0f), ANM_IGM_INTERFACE_SPR_PLAYER_NR_ICONS, kk);
		}
#endif

		if ((pPlayerActor[kk] == null) || (pPlayerActor[kk]->GetCurrentBehavior() != AI_BEHAVIOR_PLAYER_CONTROL))
			continue;

		CActor* player = pPlayerActor[kk];
		//touchables
		if ((player->pClosestTouchable != null) && (player->pClosestTouchable->bHideInteractIcon == false) &&
			(player->pClosestTouchable != pLastPaintedTarget) && (player->collisionFlags & K_DIRFLAG_DOWN))
		{
			//save last painted target
			IActiveInterface * activ = player->pClosestTouchable;
			pLastPaintedTarget = player->pClosestTouchable;

			D3DXVECTOR2 vpos(activ->bbox_exported.vCenter.x, activ->bbox_exported.vMin.y);
			//too low? don't cover the player
			if (vpos.y > player->bbox_exported.vMin.y)
				vpos.y = player->bbox_exported.vMin.y;

			//#HARDCODE: recon lockpicking door - show progress
			if ((player->templateActor.shName.IsEqual(L"ACTOR_PLAYER_RECON")) && (activ->AIstate == K_AI_STATE_ACTIVE_DOOR_SECTION))
			{
				float fDuration = activ->varAIparams.GetVariantByName(L"f_lockpickTime")->m_asFloat;
				float perc = 1.0f - (activ->fTouchTimer / fabs(fDuration));
				float barlen = 16.0f;

				if ((perc > 0.0f) && (perc < 1.0f))
				{
					RECTXYWH recttemp(vpos.x - barlen / 2.0f, vpos.y - 20.0f, barlen, 6);
					CtrlMgrDrawProgress_HeadsOutside(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_PROGRESS_RED_GLOW_HO, recttemp, perc, 0xffffffff);
				}
			}

			//change this constants for analog sticks
			const int ANIM_IDX_INTERACT_ONCE = ANM_IGM_INTERFACE_SPR_INTERACT_ONCE;
			const int ANIM_IDX_INTERACT_KEEP_PRESSED = ANM_IGM_INTERFACE_SPR_INTERACT_KEEP_PRESSED;
			/*
			#ifdef (CONSOLE)
			const int ANIM_IDX_INTERACT_ONCE = ANM_IGM_INTERFACE_SPR_INTERACT_ONCE_ANALOG;
			const int ANIM_IDX_INTERACT_KEEP_PRESSED = ANM_IGM_INTERFACE_SPR_INTERACT_KEEP_PRESSED_ANALOG;
			#endif
			*/
			if (activ->fTouchDuration == 0.0f) //daca nu trebuie sa tina apasat afisez animatie de neapasare
			{
				player->m_sprOverheadIcon.setAnimationOnce(ANIM_IDX_INTERACT_ONCE);
				player->m_sprOverheadIcon.pos = vpos;

				/*
				DWORD dwcol = 0xff00c0ff;
				if (g_timers.GetTimerValue(600) < 0.3f)
				dwcol = 0xff0384af;
				g_font5ns2->DrawString(STR_TAP, vpos.x, vpos.y - 14, FONTFLAG_ANCHOR_BOTTOMCENTER, dwcol);
				*/
				//--- paint progress damage bar ---
				float perc = 1.0f;
				int barlen = 12;
				DWORD dwProgressColor = 0xffffffff;
				//daca am o usa ca closest touchable si daca se poate sparge afisez progress pe ea
				if ((activ->pTarget != null) && (activ->pTarget->AIstate == K_AI_STATE_COLL_BREAKABLE_DOOR))
				{
					perc = activ->pTarget->AIfvar1 / activ->pTarget->AIfvar2;
					barlen = (int)ceil(activ->pTarget->AIfvar2 * 0.2f); //lungime bara damage la usile care se sparg
					dwProgressColor = 0xff00c0ff;
					barlen = (barlen / 2) * 2; //odd length
											   //limit progress bar size
					CLAMP(barlen, 6, 20);

					if ((perc > 0.0f) && (perc < 1.0f))
					{
						//RECTXYWH recttemp(vpos.x - barlen / 2.0f, activ->bbox_exported.vMax.y + 4, barlen, 8);
						RECTXYWH recttemp(activ->pTarget->pos.x - barlen / 2.0f, activ->pTarget->bbox_exported.vMax.y + 4, barlen, 8);
						CtrlMgrDrawProgress_HeadsOutside(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_PROGRESS_RED_GLOW_HO, recttemp, perc, 0xffffffff);
					}
					//#HACK: set interact icon position too so it doesn't vibrate when kicking the door
					player->m_sprOverheadIcon.pos.x = activ->pTarget->bbox_exported.vCenter.x - 1.0f;
				}
				//paint interact icon at the end
				player->m_sprOverheadIcon.paint(&m_sprInterface);
			}
			else
			{
				player->m_sprOverheadIcon.setAnimationOnce(ANIM_IDX_INTERACT_KEEP_PRESSED);
				player->m_sprOverheadIcon.pos = vpos;
				player->m_sprOverheadIcon.paint(&m_sprInterface);

				DWORD dwcol = 0xff00c0ff;
				if (g_timers.GetTimerValue(600) < 0.3f)
					dwcol = 0xff0384af;
				//g_font5ns2->DrawString(STR_HOLD, vpos.x, vpos.y - 14, FONTFLAG_ANCHOR_BOTTOMCENTER, dwcol);
				//#HACK: pentru obiectivele unde e necesara toata echipa
				if ((activ->fTouchDuration < 0.0f) && (m_nPlayersActive > 1)) //daca e negativ inseamna ca e necesara toata echipa deci afisez si controlul cu numarul de players
				{
					//se deseneaza o singura data deci testez aici toti actorii sa fie pe interacting
					int frame = 0;
					if ((pPlayerActor[0]->nInteractingState != 0) && (pPlayerActor[1]->nInteractingState != 0) && (pPlayerActor[1]->pClosestTouchable == pPlayerActor[0]->pClosestTouchable))
						frame = 2;
					else if ((pPlayerActor[0]->nInteractingState != 0) || (pPlayerActor[1]->nInteractingState != 0))
						frame = 1;

					CSprite::paintFrame(&m_sprInterface, vpos.x, activ->bbox.vMax.y, ANM_IGM_INTERFACE_SPR_TEAM_TELEPORT_ICONS, frame, 0xffffffff);
				}

				//--- paint progress bar ---
				float perc = 1.0f;
				int barlen = 12;
				DWORD dwProgressColor = 0xffffffff;

				//touch duration
				perc = 1.0f - (activ->fTouchTimer / fabs(activ->fTouchDuration));
				barlen = (int)ceil(fabs(activ->fTouchDuration) * 4.0f); //lungime bara la touch normal
				barlen = (barlen / 2) * 2;
				//limit progress bar size
				CLAMP(barlen, 10, 20);

				if ((perc > 0.0f) && (perc < 1.0f))
				{
					RECTXYWH recttemp(vpos.x - barlen / 2.0f, vpos.y - 20.0f, barlen, 6);
					CtrlMgrDrawProgress_HeadsOutside(&UTGetControlsManager().m_sprCol, ANM_CONTROLS_SPR_PROGRESS_RED_GLOW_HO, recttemp, perc, 0xffffffff);
				}
			}
		}

		//cover shield
		if (player->pCover != null)
		{
			D3DXVECTOR2 vpos = D3DXVECTOR2(pPlayerActor[kk]->bbox.vCenter.x, pPlayerActor[kk]->bbox.vMin.y);
			CSprite::paintFrame(&UTGetControlsManager().m_sprCol, vpos.x, vpos.y, ANM_CONTROLS_SPR_PLAYER_ICONS, 0, pPlayerActor[kk]->color);
		}
		else if (UTGetAppClass().m_Settings.bShowInterfaceHelp) //player numeric icon (only if shield not visible)
		{
			D3DXVECTOR2 vpos = D3DXVECTOR2(pPlayerActor[kk]->bbox.vCenter.x, pPlayerActor[kk]->bbox.vMin.y + fabs(3.0f * sin(fLocalTimeline * 4.0f)));
			CSprite::paintFrame(&m_sprInterface, vpos.x, vpos.y, ANM_IGM_INTERFACE_SPR_PLAYER_NR_ICONS, pPlayerActor[kk]->nPlayerOrdinal);
		}

		//paint player numeric icon on multiplayer when peer outside the screen
		if (UTGetAppClass().IsGameNetworked())
		{
			if ((pPlayerActor[kk]->nPlayerOrdinal == g_netlock.Net_GetOtherPlayerIndex()) && (!camAABB.Intersects(&pPlayerActor[kk]->bbox)))
			{
				D3DXVECTOR2 vpos = D3DXVECTOR2(pPlayerActor[kk]->bbox.vCenter.x, pPlayerActor[kk]->bbox.vMin.y);
				CAABB localAABB = camAABB;
				localAABB.Inflate(-K_TILE_SIZE + fabs(3.0f * sin(fLocalTimeline * 4.0f)), -K_TILE_SIZE + fabs(3.0f * sin(fLocalTimeline * 4.0f)));
				if (AABB_Segment_Intersection(vpos, camAABB.vCenter, localAABB, &vpos))
				{
					float fAng = HALF_PI + Math_GetVectorAngle(camAABB.vCenter - vpos);
					D3DXMATRIXA16 matrt;
					D3DXMatrixAffineTransformation2D(&matrt, 1.0f, NULL, fAng, &vpos);
					m_pSprite->SetTransform(&matrt);
					CSprite::paintFrame(&m_sprInterface, 0.0f, 0.0f, ANM_IGM_INTERFACE_SPR_PLAYER_NR_ICONS, 2 + pPlayerActor[kk]->nPlayerOrdinal);
					m_pSprite->SetTransform(&g_matIdentity);
					CSprite::paintFrame(&m_sprInterface, vpos.x, vpos.y, ANM_IGM_INTERFACE_SPR_PLAYER_NR_ICONS, 4 + pPlayerActor[kk]->nPlayerOrdinal);
				}
			}
		}
	}

	//--- team icon team teleporters ---
	if ((m_pTeleportSource != null) && (m_nPlayersActive > 1) && (m_fTeleportTimer <= 0.0f))
	{
		//vedem cati players sunt in starea de DOOR_TELEPORT ca sa  afisam corect iconurile
		int icons = 0;
		for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
		{
			//varianta comentata este cea care apare verde playerul doar dupa ce a intrat
			//if ((pPlayerActor[kk] != null) && (pPlayerActor[kk]->GetCurrentBehavior() == AI_BEHAVIOR_PLAYER_TEAM_TELEPORT))
			if ((pPlayerActor[kk] != null) && (pPlayerActor[kk]->m_pAIcurrentState->name.IsEqual(L"TEAM_TELEPORT")))
				icons++;
		}

		CSprite::paintFrame(&m_sprInterface, m_pTeleportSource->bbox.vCenter.x, m_pTeleportSource->bbox.vMax.y, ANM_IGM_INTERFACE_SPR_TEAM_TELEPORT_ICONS, icons, 0xffffffff);
	}

	m_pSprite->Flush();

	//paint text bubble
	m_interfaceTextBubble.Paint(m_pDevice, m_pSprite);

	//set screen space
	CCameraTransform::SetActiveCamera(m_pDevice, &UTGetAppClass().g_camScreen);
	///--- paint vignettes ---
	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	m_screenVignetteDamage.Paint(m_pSprite, &UTGetControlsManager().m_sprCol);
	m_screenVignette.Paint(m_pSprite, &UTGetControlsManager().m_sprCol);

	///--- paint time slowdown screen effect ---
	if (m_fTimeMultiplier_real < 1.0f)
	{
		DWORD colEffect = D3DCOLOR_COLORALPHA(0xff000088, 1.0f - m_fTimeMultiplier_real);
		D3DXMATRIXA16 mattrans;
		RECTXYWH_F bbox = UTGetControlsManager().m_sprCol.GetAFrameBBox_real(ANM_CONTROLS_SPR_VIGNETTES, 1);
		D3DXMatrixAffineTransformation2D(&mattrans, UTGetAppClass().g_rectRender.h / bbox.h, NULL, 0.0f, &UTGetAppClass().g_rectRender.Center());
		m_pSprite->SetTransform(&mattrans);
		CSprite::paintFrame(&UTGetControlsManager().m_sprCol, 0.0f, 0.0f, ANM_CONTROLS_SPR_VIGNETTES, 1, colEffect);
		m_pSprite->Flush();
	}

	//return to point filtering
	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

	return hr;
}

void CLevel::Release()
{
	ClearVisibilityLists();

	if (tiles != NULL)
	{
		for (int kk = 0; kk < levelSizeTL.w; kk++)
		{
			SAFE_DELETE_ARRAY(tiles[kk]);
		}
		SAFE_DELETE_ARRAY(tiles);
	}

	SAFE_DELETE_GROWABLE_ARRAY(m_arrColShapes);
	SAFE_DELETE_GROWABLE_ARRAY(m_arrLights);
	m_arrActivesPtrInteract.Clear();
	SAFE_DELETE_GROWABLE_ARRAY(m_arrActives);
	SAFE_DELETE_GROWABLE_ARRAY(m_arrDecals);
	SAFE_DELETE_GROWABLE_ARRAY(m_arrActors);
	SAFE_DELETE_GROWABLE_ARRAY(m_arrMiscObjects);

	SAFE_DELETE_GROWABLE_ARRAY(m_arrTemplatesActor);
	SAFE_DELETE_GROWABLE_ARRAY(m_arrAItemplates);
	SAFE_DELETE_GROWABLE_ARRAY(m_arrTemplatesWeapon);
	SAFE_DELETE_GROWABLE_ARRAY(m_arrTemplatesExplosion);

	SAFE_DELETE_GROWABLE_ARRAY(m_arrAIevents);
	//release bullets
	m_poolBullets.Release();
	m_poolProps.Release();
	m_poolPhysPts.Release();

	m_sprLights.Release();
	m_sprActives.Release();
	m_sprActors.Release();
	m_sprInterface.Release();

	m_texManager.Release();
	//reset player list
	m_nPlayers = 0;
	m_nPlayersActive = 0;
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		pPlayerActor[kk] = null;
	}

	m_interfaceIGM.Release();
	m_interfaceTextBubble.Release();

	g_particlesMgr.RemoveAll();

	if (m_bLoaded)
	{
		LOG(L"Level Released.");
	}

	m_bLoaded = false;
	m_bOneUpdateDone = false;
}

///--- framework implementations ---
#pragma region FRAMEWORK_IMPL
HRESULT CLevel::OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr = S_OK;
	m_pDevice = pd3dDevice;

	V_RETURN(m_sprLights.OnCreateDevice(pd3dDevice));
	V_RETURN(m_sprActives.OnCreateDevice(pd3dDevice));
	V_RETURN(m_sprActors.OnCreateDevice(pd3dDevice));
	V_RETURN(m_sprInterface.OnCreateDevice(pd3dDevice));
	V_RETURN(m_texManager.OnCreateDevice(pd3dDevice));

	V_RETURN(m_bufferedPainter.OnCreateDevice(pd3dDevice));

	return S_OK;
}

HRESULT CLevel::OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr = S_OK;
	m_pDevice = pd3dDevice;

	//--- face textura pentru backbuffer RTT ---
	if (FAILED(D3DXCreateTexture(pd3dDevice,
								K_RTT_WIDTH,
								K_RTT_HEIGHT,
								1,
								D3DUSAGE_RENDERTARGET,
								D3DFMT_A8R8G8B8,   //am nevoie de alpha
								D3DPOOL_DEFAULT,
								&m_pRTTexture)))
	{
		UTGetAppClass().g_gfxFlags &= ~K_UT_GFXFLAG_RTT;
		ErrorBox(K_ERR_CRITICAL, L"Failed creating Level RTT texture. Setting CARD_FLAG_RTT to false. Application will now quit!");
	}
	else //if NOT failed
	{
		UTGetAppClass().g_gfxFlags |= K_UT_GFXFLAG_RTT;
		// Create off-screen "render to" surfaces...
		D3DSURFACE_DESC desc;
		m_pRTTexture->GetSurfaceLevel(0, &m_pRTSurface);
		m_pRTSurface->GetDesc(&desc);

		if (FAILED(D3DXCreateRenderToSurface(pd3dDevice,
			desc.Width,
			desc.Height,
			desc.Format,
			TRUE,
			D3DFMT_D24X8, //aici nu am nevoie de stencil
			&m_pRenderToSurface)))
		{
			UTGetAppClass().g_gfxFlags &= ~K_UT_GFXFLAG_RTT;
			ErrorBox(K_ERR_CRITICAL, L"Failed creating Level RTT surface. Setting CARD_FLAG_RTT to false. Application will now quit!");

			SAFE_RELEASE(m_pRTTexture);
		}
		else
		{
			//RTT ok
			UTGetAppClass().g_gfxFlags |= K_UT_GFXFLAG_RTT;
		}
	}


	//--- face textura pentru RT de compozitie ---
	if (FAILED(D3DXCreateTexture(pd3dDevice,
		pBackBufferSurfaceDesc->Width,
		pBackBufferSurfaceDesc->Height,
		1,
		D3DUSAGE_RENDERTARGET,
		D3DFMT_X8R8G8B8,   //nu am nevoie de alpha
		D3DPOOL_DEFAULT,
		&m_pRTTexture_final)))
	{
		UTGetAppClass().g_gfxFlags &= ~K_UT_GFXFLAG_RTT;
		ErrorBox(K_ERR_CRITICAL, L"Failed creating Compositing RTT texture. Setting CARD_FLAG_RTT to false. Application will now quit!");
	}
	else //if NOT failed
	{
		UTGetAppClass().g_gfxFlags |= K_UT_GFXFLAG_RTT;
		// Create off-screen "render to" surfaces...
		D3DSURFACE_DESC desc;
		m_pRTTexture_final->GetSurfaceLevel(0, &m_pRTSurface_final);
		m_pRTSurface_final->GetDesc(&desc);

		if (FAILED(D3DXCreateRenderToSurface(pd3dDevice,
			desc.Width,
			desc.Height,
			desc.Format,
			TRUE,
			D3DFMT_D24S8, //aici am nevoie de stencil
			&m_pRT_final)))
		{
			UTGetAppClass().g_gfxFlags &= ~K_UT_GFXFLAG_RTT;
			ErrorBox(K_ERR_CRITICAL, L"Failed creating Compositing RTT surface. Setting CARD_FLAG_RTT to false. Application will now quit!");

			SAFE_RELEASE(m_pRTTexture_final);
		}
		else
		{
			//RTT ok
			UTGetAppClass().g_gfxFlags |= K_UT_GFXFLAG_RTT;
		}
	}


	V_RETURN(m_sprLights.OnResetDevice(pd3dDevice));
	V_RETURN(m_sprActives.OnResetDevice(pd3dDevice));
	V_RETURN(m_sprActors.OnResetDevice(pd3dDevice));
	V_RETURN(m_sprInterface.OnResetDevice(pd3dDevice));
	V_RETURN(m_texManager.OnResetDevice(pd3dDevice));

	V_RETURN(m_bufferedPainter.OnResetDevice(pd3dDevice));

	return S_OK;
}

HRESULT CLevel::OnLostDevice( void* pUserContext )
{
	m_pDevice = NULL;

	//RTT
	SAFE_RELEASE(m_pRenderToSurface);
	SAFE_RELEASE(m_pRTTexture);
	SAFE_RELEASE(m_pRTSurface);

	SAFE_RELEASE(m_pRT_final);
	SAFE_RELEASE(m_pRTTexture_final);
	SAFE_RELEASE(m_pRTSurface_final);

	m_sprLights.OnLostDevice();
	m_sprActives.OnLostDevice();
	m_sprActors.OnLostDevice();
	m_sprInterface.OnLostDevice();
	m_texManager.OnLostDevice();

	m_bufferedPainter.OnLostDevice();

	return S_OK;
}

HRESULT CLevel::OnDestroyDevice( void* pUserContext )
{
	m_pDevice = NULL;

	m_sprLights.OnDestroyDevice();
	m_sprActives.OnDestroyDevice();
	m_sprActors.OnDestroyDevice();
	m_sprInterface.OnDestroyDevice();
	m_texManager.OnDestroyDevice();

	m_bufferedPainter.OnDestroyDevice();

	return S_OK;
}

#pragma endregion FRAMEWORK_IMPL

//-------------------------------------------------------------
// Functii pentru gasirea occluderelor pt iluminare
//-------------------------------------------------------------
void CLevel::AddOccludersFromAABB_stencil(D3DXVECTOR2 viewerPos, CAABB * aabb)
{
	assert(m_occludersCnt < K_LVL_MAX_OCCLUDERS_CNT - 4);
	//adaug marginile AABB-ului ce trebuies extrudate
	if (viewerPos.y > aabb->vMax.y)
	{
		m_occluders[m_occludersCnt].start = aabb->vMax;
		m_occluders[m_occludersCnt].end = D3DXVECTOR2(aabb->vMin.x, aabb->vMax.y);
		m_occludersCnt++;
	}
	else if (viewerPos.y < aabb->vMin.y)
	{
		m_occluders[m_occludersCnt].start = aabb->vMin;
		m_occluders[m_occludersCnt].end = D3DXVECTOR2(aabb->vMax.x, aabb->vMin.y);
		m_occludersCnt++;
	}
	else //daca e in interior le adauga pe ambele
	{
		m_occluders[m_occludersCnt].start = aabb->vMax;
		m_occluders[m_occludersCnt].end = D3DXVECTOR2(aabb->vMin.x, aabb->vMax.y);
		m_occludersCnt++;

		m_occluders[m_occludersCnt].start = aabb->vMin;
		m_occluders[m_occludersCnt].end = D3DXVECTOR2(aabb->vMax.x, aabb->vMin.y);
		m_occludersCnt++;
	}

	if (viewerPos.x > aabb->vMax.x)
	{
		m_occluders[m_occludersCnt].start = D3DXVECTOR2(aabb->vMax.x, aabb->vMin.y);
		m_occluders[m_occludersCnt].end = aabb->vMax;
		m_occludersCnt++;
	}
	else if (viewerPos.x < aabb->vMin.x)
	{
		m_occluders[m_occludersCnt].start = D3DXVECTOR2(aabb->vMin.x, aabb->vMax.y);
		m_occluders[m_occludersCnt].end = aabb->vMin;
		m_occludersCnt++;
	}
	else
	{
		m_occluders[m_occludersCnt].start = D3DXVECTOR2(aabb->vMax.x, aabb->vMin.y);
		m_occluders[m_occludersCnt].end = aabb->vMax;
		m_occludersCnt++;

		m_occluders[m_occludersCnt].start = D3DXVECTOR2(aabb->vMin.x, aabb->vMax.y);
		m_occluders[m_occludersCnt].end = aabb->vMin;
		m_occludersCnt++;
	}
}

//scrie intr-un array mare declarat in clasa clevel ca sa nu aloc si sa dezaloc mereu
COccluder* CLevel::GetVisibleAABBs_toOccluders(D3DXVECTOR2 viewPos, CAABB * viewRect, int & retOccludersCnt)
{
	m_occludersCnt = 0;
	//clip AABB
	CAABB clip = *viewRect;
	clip.Inflate(D3DXVECTOR2(-1.0f, -1.0f));

	//gasesc toate occluderele care se intersecteaza cu viewRect si le fac clip
	for (int kk = 0; kk < m_visibleList.visible_colShapesLights.Count(); kk++)
	{
		//sunt filtrate deja la constuirea listei (doar cele care fac shadow casting apar in lista)
		//if (!m_visibleList.colShapes[kk]->castShadows)
			//continue;
		//#TODO: aici ar trebui luate in calcul si celelalte tipuri de collision shapes cand ma hotarasc sa adaug segmente si alte forme
		CAABB retaabb;
		if (AABB_Intersection(clip, m_visibleList.visible_colShapesLights.m_pData[kk]->bbox, retaabb))
		{
			AddOccludersFromAABB_stencil(viewPos, &retaabb);
		}
	}
	//return 
	retOccludersCnt = m_occludersCnt;
	return m_occluders;
}

//outVerts e array-ul in care primesti vertecsii finali
//outVertsMaxCnt e marimea array-ului
//returns - numarul efectiv de verts scrisi
int CLevel::BuildShadowVolume(CLight * light, CAABB * visibleAABB, COccluder * p_arrOccluders, int nOccludersCount, _VERTEX_PNCT4T4 *outVerts, int outVertsMaxCnt)
{
	int vertsCur = 0;
	//1. daca punctul de distanta intr viewerPos si occluder care pe occluder ii facem split ca sa ne asiguram ca acopera tot volumul necesar
	//#TODO: sa permita shadow volumes si pt lumini directionale (extinde altfel poligoanele)
	//#TODO: ar fi tare daca as putea face clip la volume in aabb-ul luminii

	//alegem raza pana la care extindem poligoanele. Trebuie sa fie 2 * dim max bbox ca daca avem occluder f aproape de lumina sa fim siguri ca acopera cercul circumscris bbox-ului
	float outerR = 100.0f;
	if (visibleAABB == NULL)
	{
		outerR = 4.0f * max(light->bbox.vHalfSize.x, light->bbox.vHalfSize.y);
	}
	else
	{
		//daca lumina este descentrata total trebuie luata distanta maxima de la lumina la laturile aabb-ului si facuta o raza de 2X distanta asta sau nu va desena corect volumele de umbre
		float radx = max(fabs(light->pos.x - visibleAABB->vMin.x), fabs(visibleAABB->vMax.x - light->pos.x));
		float rady = max(fabs(light->pos.y - visibleAABB->vMin.y), fabs(visibleAABB->vMax.y - light->pos.y));
		outerR = 2.0f * max(radx, rady);
	}

	for (int kk = 0; kk < nOccludersCount; kk++)
	{
		COccluder* occ = &p_arrOccluders[kk];
		//gasim proiectia
		D3DXVECTOR2 dir = occ->end - occ->start;
		float dirL = D3DXVec2Length(&dir);
		D3DXVECTOR2 dirN = dir / dirL;
		D3DXVECTOR2 lDir(light->pos.x - occ->start.x, light->pos.y - occ->start.y);

		float dotN = D3DXVec2Dot(&lDir, &dirN);

		//daca proiectia pica pe segment ii fac split
		if ((dotN >= 0.0f) && (dotN <= dirL))
		{
			assert(vertsCur < outVertsMaxCnt - 12);

			D3DXVECTOR2 projPt = occ->start + dirN * dotN;
			COccluder oc1, oc2, exoc1, exoc2;
			oc1.start = occ->start; oc1.end = projPt;
			oc2.start = projPt; oc2.end = occ->end;
			//extrudam oc1
			D3DXVECTOR2 lDirN;
			float lDirL = D3DXVec2Length(&lDir);
			lDirN = lDir / lDirL;
			exoc1.start = oc1.start - lDirN * (outerR - lDirL);

			lDir = D3DXVECTOR2(light->pos.x - oc1.end.x, light->pos.y - oc1.end.y);
			lDirL = D3DXVec2Length(&lDir);
			lDirN = lDir / lDirL;
			exoc1.end = oc1.end - lDirN * (outerR - lDirL);
			//extrudam oc2
			lDir = D3DXVECTOR2(light->pos.x - oc2.start.x, light->pos.y - oc2.start.y);
			lDirL = D3DXVec2Length(&lDir);
			lDirN = lDir / lDirL;
			exoc2.start = oc2.start - lDirN * (outerR - lDirL);

			lDir = D3DXVECTOR2(light->pos.x - oc2.end.x, light->pos.y - oc2.end.y);
			lDirL = D3DXVec2Length(&lDir);
			lDirN = lDir / lDirL;
			exoc2.end = oc2.end - lDirN * (outerR - lDirL);
			//adaugam poligoanele
			outVerts[vertsCur++].pos = D3DXVECTOR3(oc1.start.x, oc1.start.y, 0.0f);
			outVerts[vertsCur++].pos = D3DXVECTOR3(exoc1.start.x, exoc1.start.y, 0.0f);
			outVerts[vertsCur++].pos = D3DXVECTOR3(exoc1.end.x, exoc1.end.y, 0.0f);

			outVerts[vertsCur++].pos = D3DXVECTOR3(oc1.start.x, oc1.start.y, 0.0f);
			outVerts[vertsCur++].pos = D3DXVECTOR3(exoc1.end.x, exoc1.end.y, 0.0f);
			outVerts[vertsCur++].pos = D3DXVECTOR3(oc1.end.x, oc1.end.y, 0.0f);
			////si al doilea
			outVerts[vertsCur++].pos = D3DXVECTOR3(oc2.start.x, oc2.start.y, 0.0f);
			outVerts[vertsCur++].pos = D3DXVECTOR3(exoc2.start.x, exoc2.start.y, 0.0f);
			outVerts[vertsCur++].pos = D3DXVECTOR3(exoc2.end.x, exoc2.end.y, 0.0f);

			outVerts[vertsCur++].pos = D3DXVECTOR3(oc2.start.x, oc2.start.y, 0.0f);
			outVerts[vertsCur++].pos = D3DXVECTOR3(exoc2.end.x, exoc2.end.y, 0.0f);
			outVerts[vertsCur++].pos = D3DXVECTOR3(oc2.end.x, oc2.end.y, 0.0f);

		}
		else //daca nu pica pe segment doar fac extrude
		{
			assert(vertsCur < outVertsMaxCnt - 6);

			COccluder oc, exoc;
			oc = *occ;

			D3DXVECTOR2 lDirN;
			float lDirL = D3DXVec2Length(&lDir);
			lDirN = lDir / lDirL;
			exoc.start = oc.start - lDirN * (outerR - lDirL);

			lDir = D3DXVECTOR2(light->pos.x - occ->end.x, light->pos.y - occ->end.y);
			lDirL = D3DXVec2Length(&lDir);
			lDirN = lDir / lDirL;
			exoc.end = oc.end - lDirN * (outerR - lDirL);
			//adaug verts
			outVerts[vertsCur++].pos = D3DXVECTOR3(oc.start.x, oc.start.y, 0.0f);
			outVerts[vertsCur++].pos = D3DXVECTOR3(exoc.start.x, exoc.start.y, 0.0f);
			outVerts[vertsCur++].pos = D3DXVECTOR3(exoc.end.x, exoc.end.y, 0.0f);

			outVerts[vertsCur++].pos = D3DXVECTOR3(oc.start.x, oc.start.y, 0.0f);
			outVerts[vertsCur++].pos = D3DXVECTOR3(exoc.end.x, exoc.end.y, 0.0f);
			outVerts[vertsCur++].pos = D3DXVECTOR3(oc.end.x, oc.end.y, 0.0f);
		}
	}

	return vertsCur;
}

void CLevel::InitializeStrategicAbilities(int nPlayerOrdinal)
{
	if ((nPlayerOrdinal < 0) || (nPlayerOrdinal >= K_MAX_PLAYERS_CNT))
	{
		ErrorBox(K_ERR_WARNING, L"InitializeStrategicAbilities: invalid playerOrdinal!");
		return;
	}
	//reset selction on NONE
	m_arrPlayerSelStrategic[nPlayerOrdinal] = -1;
	
	m_arrStrategicAbilities[nPlayerOrdinal][0] = -1;
	m_arrStrategicAbilitiesNames[nPlayerOrdinal][0] = -1;

	m_arrStrategicAbilities[nPlayerOrdinal][1] = K_CI_STRATEGIC_BODY_ARMOR;
	m_arrStrategicAbilitiesNames[nPlayerOrdinal][1] = STR_STRATEGIC1;

	m_arrStrategicAbilities[nPlayerOrdinal][2] = K_CI_STRATEGIC_GEAR_REFILL;
	m_arrStrategicAbilitiesNames[nPlayerOrdinal][2] = STR_STRATEGIC2;

	m_arrStrategicAbilities[nPlayerOrdinal][3] = -1;
	m_arrStrategicAbilitiesNames[nPlayerOrdinal][3] = -1;

	m_arrStrategicAbilities[nPlayerOrdinal][4] = K_CI_STRATEGIC_MEDIKIT;
	m_arrStrategicAbilitiesNames[nPlayerOrdinal][4] = STR_STRATEGIC3;

	m_arrStrategicAbilities[nPlayerOrdinal][5] = K_CI_STRATEGIC_REINFORCEMENT;
	m_arrStrategicAbilitiesNames[nPlayerOrdinal][5] = STR_STRATEGIC4;

	m_arrStrategicAbilities[nPlayerOrdinal][6] = K_CI_STRATEGIC_EXTRA_LIFE;
	m_arrStrategicAbilitiesNames[nPlayerOrdinal][6] = STR_STRATEGIC5;
	//ultima abilitate se ia din ecranul de player select screen
	eStrategicAbility eRetAbility = K_CI_STRATEGIC_NONE;
	int nAbilityStringIdx = -1;
	g_playerSelScr.GetUltimateAbility(&g_playerSelScr.m_arrPlayers[nPlayerOrdinal], eRetAbility, nAbilityStringIdx);
	//si se salveaza in array-ul corespunzator
	m_arrStrategicAbilities[nPlayerOrdinal][7] = (int)eRetAbility;
	m_arrStrategicAbilitiesNames[nPlayerOrdinal][7] = nAbilityStringIdx;
}

void CLevel::ResetLevelStatistics()
{
	for (int kk = 0; kk < K_LVL_STATS_CNT; kk++)
	{
		m_arrStats[kk] = 0;
	}

	m_interfaceIGM.SetStrategicPoints(0.0f, 0.0f);
	m_interfaceIGM.SetLivesLeft(m_arrStats[K_LVL_STATS_PL1_LIVES], m_arrStats[K_LVL_STATS_PL2_LIVES]);
}

void CLevel::IncreaseLevelStatistics(int K_LVL_STATS_n, int nValueToAdd /*= 1*/)
{
	if ((K_LVL_STATS_n < 0) || (K_LVL_STATS_n >= K_LVL_STATS_CNT))
	{
		ErrorBox(K_ERR_WARNING, L"Illegal Level Stat IDX!");
		return;
	}

	m_arrStats[K_LVL_STATS_n] += nValueToAdd;

	//#ACHIEVEMENTS: check level achievements
	switch (K_LVL_STATS_n)
	{
		case K_LVL_STATS_PL1_USE_EXTRA_LIFE_CNT:
		{
			if ((m_arrStats[K_LVL_STATS_n] >= 5) &&
				(pPlayerActor[0] != null) &&
				(!IsNetworkPlayer(pPlayerActor[0])))
			{
				UTGetAchievementManager().UnlockAchievement(ACH_TERMINATOR);
			}
		}
		break;
		case K_LVL_STATS_PL2_USE_EXTRA_LIFE_CNT:
		{
			if ((m_arrStats[K_LVL_STATS_n] >= 5) &&
				(pPlayerActor[1] != null) &&
				(!IsNetworkPlayer(pPlayerActor[1])))
			{
				UTGetAchievementManager().UnlockAchievement(ACH_TERMINATOR);
			}
		}
		break;
		case K_LVL_STATS_PL1_RESURRECT_PEER_CNT:
		{
			if ((m_arrStats[K_LVL_STATS_n] >= 5) &&
				(pPlayerActor[0] != null) &&
				(!IsNetworkPlayer(pPlayerActor[0])))
			{
				UTGetAchievementManager().UnlockAchievement(ACH_STAY_WITH_ME);
			}
		}
		break;
		case K_LVL_STATS_PL2_RESURRECT_PEER_CNT:
		{
			if ((m_arrStats[K_LVL_STATS_n] >= 5) &&
				(pPlayerActor[1] != null) &&
				(!IsNetworkPlayer(pPlayerActor[1])))
			{
				UTGetAchievementManager().UnlockAchievement(ACH_STAY_WITH_ME);
			}
		}
		break;
	}
}

bool CLevel::ActivateSpecialAbility(int nAbilityIdx, int nTargetPlayerOrdinal)
{
	return false;
}

void CLevel::GiveStrategicPoints(float fPoints, D3DXVECTOR2 * vPos)
{
	if (fPoints <= 0.0f)
		return;
	//on single player multiply the points
	float fMultiplier = 1.0f;
	//change multiplier based on XP bars
	//take the first player (always present)
	float fFilled = g_playerSelScr.GetUpgradeBarPercent(&g_playerSelScr.m_arrPlayers[0], L"TEAM_LOGISTICS");
	fMultiplier += fFilled * 0.5f;
	//double XP points on single player
	if (m_nPlayers == 1)
		fMultiplier *= 2.0f;
	//just making sure...
	assert((fPoints >= 0.0f) && (fPoints <= (float)K_LVL_MAX_STRATEGIC_POINTS));
	float fPointsGiven = LIMIT(fPoints, 0.0f, (float)K_LVL_MAX_STRATEGIC_POINTS);

	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if (pPlayerActor[kk] == null)
			continue;
		
		int fMaxPoints = K_LVL_MAX_STRATEGIC_POINTS;
		//#PERK: EXTRA SP SLOTS - gives you 2 additionsl SP slots
		if (g_playerSelScr.IsPerkEnabled(pPlayerActor[kk]->nPlayerOrdinal, &shPerk_EXTRA_SP_SLOTS))
			fMaxPoints += K_LVL_STRATEGIC_POINTS_ADDED_BY_PERK;

		int nStatIdx = K_LVL_STATS_PL1_STRATEGIC_POINTS + pPlayerActor[kk]->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT;
		int nAdder = (int)floor(fMultiplier * fPointsGiven * 1000.0f);
		m_arrStats[nStatIdx] += nAdder;
		CLAMP(m_arrStats[nStatIdx], 0, fMaxPoints * 1000);
	}

	m_interfaceIGM.SetStrategicPoints(m_arrStats[K_LVL_STATS_PL1_STRATEGIC_POINTS] / 1000.0f, m_arrStats[K_LVL_STATS_PL2_STRATEGIC_POINTS] / 1000.0f);

	//add text particle (visuals)
	if ((vPos != null) && (fPointsGiven > 0.0f))
	{
		WCHAR strPart[MAX_PATH];
		float fVal = fMultiplier * fPointsGiven;
		if (FLOAT_FRAC(fVal) > 0.1f)
			StringCchPrintf(strPart, MAX_PATH, L"+%.1f SP", fVal);
		else
			StringCchPrintf(strPart, MAX_PATH, L"+%d SP", (int)fVal);

		g_particlesMgr.AddStringParticle(g_font5ns2, strPart, vPos, NULL, &D3DXVECTOR2(0.0f, -20.0f), 1.2f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.4f, 0xcc21aec2, K_PART_LAYER_NORMAL);
	}

}


//-------------------------------------------------------------
// Functii utilitare
//-------------------------------------------------------------
bool CLevel::IsLineOfSight(D3DXVECTOR2 pt1, D3DXVECTOR2 pt2, D3DXVECTOR2 * retVecCollisionPt, D3DXVECTOR2 * retVecCollisionNormal)
{
	D3DXVECTOR2 collisionPoint, collisionNormal;
	//before enemies attacked each other too, here was checking with closeby collisions
	CCollisionShape* colShape = ColShape_Segment_Intersection_Arr(pt1, pt2, m_visibleList.logic_colShapesExtended.m_pData, m_visibleList.logic_colShapesExtended.Count(), retVecCollisionPt, retVecCollisionNormal);
	if (colShape != NULL)
	{
		return false;
	}
	return true;
}

///--- BULLETS MANAGER ---
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
	if((weapon->bTriggerDownOld == false) && (weapon->bTriggerDown == true))
	{
		if((weapon->ammoLeft == 0) && (weapon->WeaponTemplate.sndidxEmpty >= 0))
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
		//reloading faster when standing still
		float fSlowingReload = 0.0f;
		if (fabs(weapon->pOwner->speed.x) != 0.0f)
			fSlowingReload = (dTime * weapon->WeaponTemplate.fShooterSpeedSlowingPercent) * (1.0f / weapon->pOwner->templateActor.fDexterity);

		weapon->reloadTimer += dTime * weapon->pOwner->templateActor.fDexterity - fSlowingReload;

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

	if(weapon->fJammedTimer < weapon->WeaponTemplate.fJammedDuration)
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

bool CLevel::ShootWeapon(CWeapon * weapon, D3DXVECTOR2 vDir)
{
	if ((weapon == null) || (weapon->pOwner == null) || (weapon->status == K_LVL_WPN_STATUS_UNKNOWN))
		return false;
	//make sure we don't shoot a jammed weapon
	if (weapon->status == K_LVL_WPN_STATUS_JAMMED)
		return false;

	CActor* shooter = weapon->pOwner;
	D3DXVECTOR2 vFinalDir = vDir;
	D3DXVECTOR2 vShootPos = shooter->GetPosWeapon();

	int nFinalClass = shooter->templateActor.actorClass;
	//bullet has template class, set it to final class
	if (weapon->WeaponTemplate.bulletTemplate.eClass != K_LVL_ACT_CLASS_ANY)
		nFinalClass = weapon->WeaponTemplate.bulletTemplate.eClass;

	//#HACK: de pe scara trage cu grenada direct in jos daca mergi in jos doar
	if (weapon->pOwner->bOnLadder)
	{
		if (weapon->WeaponTemplate.bCanShootFromLadders)
		{
			vShootPos = shooter->GetPosHeart();

			if (weapon->pOwner->m_AIcommands.nMoveDirY > 0)
			{
				vFinalDir.y = 1.0f;
				vFinalDir.x = 0.0f;
			}
			else if (weapon->pOwner->m_AIcommands.nMoveDirY < 0)
			{
				//throw a little oblicque when going up the ladder
				vFinalDir.y = -2.0f;
				//#HACK: shoot from above the head
				vShootPos.y -= 5.0f;
			}
			else
			{
				vFinalDir.y = -1.0f;
			}
		}
	}
	//melee shoots from heart pos (it checks objects behind shooting point)
	if (weapon->WeaponTemplate.bulletTemplate.nGroup == K_LVL_BULLGROUP_MELEE)
	{
		vShootPos = shooter->GetPosHeart();
	}

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
		float fAimAng = Math_GetVectorAngle(vFinalDir);
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
		//#PERK: BARRICADE
		if ((shooter->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER) && (shooter->pCover != null))
			if (g_playerSelScr.IsPerkEnabled(shooter->nPlayerOrdinal, &shPerk_BARRICADE))
				fMul = K_LVL_COVER_ERROR_MULTIPLIER;
		//#PERK: STEADY HAND - precizion when crouched
		if ((shooter->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER) && (shooter->bCrouched))
			if (g_playerSelScr.IsPerkEnabled(shooter->nPlayerOrdinal, &shPerk_STEADY_HAND))
				fMul = K_LVL_COVER_ERROR_MULTIPLIER;
		
		fAimAngError *= fMul;
		//apply template aiming multiplier
		fAimAngError *= shooter->templateActor.fRecoilModifier;
		//add aiming error
		fAimAng += fAimAngError;

		//some weapons force the actor to play a verse when shooting
		PlayActorSoundVerse(shooter, weapon->WeaponTemplate.sndActorVerse);

		//#HACK: player's last bullet does double damage (for fun)
		if ((weapon->pOwner->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER) && (weapon->ammoLeft == 0))
			tmplBullet.fDamage *= 2.0f;
		//#PERK: INITMIDATING - intimidated enemies bullets do lower damage
		if (weapon->pOwner->cDamageOverTime.eType == CDamageOverTime::K_LVL_DoT_INTIMIDATED)
			tmplBullet.fDamage *= 0.8f;
		//#PERK: WEAK SPOTTER - shotgun pellets skip armor
		float fIgnoreArmorPerc = 0.0f;
		if ((weapon->pOwner->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER) && (weapon->WeaponTemplate.nBulletsPerShot > 1))
		{
			if (g_playerSelScr.IsPerkEnabled(shooter->nPlayerOrdinal, &shPerk_WEAK_SPOTTER))
			{
				fIgnoreArmorPerc = 30.0f;
			}
		}
		//also shoot bullets
		for (int kk = 0; kk < weapon->WeaponTemplate.nBulletsPerShot; kk++)
		{
			//add weapon spread
			float fSpreadAng = m_rand.RandFloatSgn(weapon->WeaponTemplate.fSpreadFOV);

			vFinalDir.x = cos(fAimAng + fSpreadAng); 
			vFinalDir.y = sin(fAimAng + fSpreadAng);
			D3DXVec2Normalize(&vFinalDir, &vFinalDir);

			if ((fIgnoreArmorPerc > 0.0f) && (m_rand.RandFloat(100.0f) < fIgnoreArmorPerc))
			{
				tmplBullet.nFlags |= K_LVL_BULLET_FLAG_IGNORE_ARMOR;
			}
			//apply weapon perk
			if (weapon->m_activePerk.bEnabled)
			{
				tmplBullet.fDamage += tmplBullet.fDamage * weapon->m_activePerk.fDamage_percAdd;
			}

			LOG_DBG_BUFF(L"= Shot:%s ID:%d =", weapon->WeaponTemplate.name.text, weapon->pOwner->ID);
			CBullet* bullet = ShootBullet(&tmplBullet, nFinalClass, shooter->GetUID(), vShootPos, vFinalDir);
			//--- statistics ---
			if ((bullet != NULL) && ((bullet->nFlags & K_LVL_BULLET_FLAG_NOT_BALLISTIC) == 0) && (weapon->pOwner->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER))
			{
				//aici numara si grenadele dar nu prea conteaza pt ca tragi multe gloante in joc
				m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT + weapon->pOwner->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT]++;
			}
		}

		//adaug shell
		if (weapon->WeaponTemplate.nDropShellFrame >= 0)
		{
			AddProp(K_LVL_PROP_SHELL, weapon->pOwner->GetPosHeart(), &D3DXVECTOR2(-weapon->pOwner->lookDirXsign * (40.0f + randfloat(30.0f)), -50.0f - randfloat(20.0f)), &g_vecGravity, weapon->WeaponTemplate.nDropShellFrame);
		}

		float fAimErrorMul = 1.0f;
		//#PERK: STRONG_STANCE - second shot has 70% lower aim error
		if ((shooter->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER) && (weapon->m_nBulletsShotSinceCool == 2))
			if (g_playerSelScr.IsPerkEnabled(shooter->nPlayerOrdinal, &shPerk_STRONG_STANCE))
				fAimErrorMul = 0.3f;			

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
			AddProp_Light(vShootPos, ANM_LIGHTS_SPR_POINT1, 0.05f, 0.0f, D3DCOLOR_COLORALPHA(0xffFDB727, fPropAlpha), weapon->WeaponTemplate.fMuzzleLightSize);
		}
		//adaug eventAI de sunet
		AddAIEvent(K_LVL_AI_EVENT_SOUND_THREAT, shooter->GetUID(), shooter->templateActor.actorClass, shooter->posHeart, weapon->WeaponTemplate.fSoundRadius);
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

CBullet* CLevel::ShootBullet(CBulletTemplate * bulletTemplate, int actorClass, UINT32 nOwnerUID, D3DXVECTOR2 pos, D3DXVECTOR2 shootDir)
{
	//dull bullets don't actually get spawned (sometimes we need them)
	if (bulletTemplate->nType == K_LVL_BULLET_DULL)
	{
		return NULL;
	}

	//melee bullets
	if (bulletTemplate->nGroup == K_LVL_BULLGROUP_MELEE)
	{
		//make melee range slightly larger than bullet normal range.
		float fRangeObjects = (bulletTemplate->fSpeed_ini * bulletTemplate->fLife);
		float fRange = fRangeObjects * 1.5f; //larger range for humans
		int nHitActors = MeleeBlow(bulletTemplate->nType, pos, shootDir, nOwnerUID, actorClass, fRange, bulletTemplate->fDamage, bulletTemplate->fMomentum, bulletTemplate->fStunDuration, (EActorClass)actorClass, fRangeObjects, bulletTemplate->fDamageObjects);

		return NULL;
	}

	CLinkedPool<CBullet>::CLinkedPoolNode *node = m_poolBullets.HireNode();
	//set 
	if (node == null)
	{
		ErrorBox(K_ERR_WARNING, L"ShootBullet:We need more bullets!");
		return NULL;
	}

	//add simulation container
	node->m_data.physPt = m_poolPhysPts.HireNode();
	if (node->m_data.physPt == NULL)
	{
		ErrorBox(K_ERR_WARNING, L"ShootBullet:We need more physics points!");
		m_poolBullets.DismissNode(node);
		return NULL;
	}
	//reset physics data
	node->m_data.physPt->m_data.Init();
	//set bullet generic data
	node->m_data.actorClass = actorClass;
	node->m_data.ownerUID = nOwnerUID;
	node->m_data.dwLastTargetUID = 0;
	node->m_data.nSubstate = 0;

	node->m_data.type = bulletTemplate->nType;
	node->m_data.nFlags = bulletTemplate->nFlags;
	node->m_data.nExploTemplateHash = bulletTemplate->nExploTemplateHash;

	node->m_data.fStunDuration = bulletTemplate->fStunDuration;
	node->m_data.fDamage = bulletTemplate->fDamage;
	node->m_data.fDamage_ini = node->m_data.fDamage;
	node->m_data.fDamageLossPPx = bulletTemplate->fDamageLossPPx;
	node->m_data.fMomentum = bulletTemplate->fMomentum;
	node->m_data.fLife = bulletTemplate->fLife;
	node->m_data.fLife_ini = node->m_data.fLife;
	node->m_data.nArmorPiercingRating = bulletTemplate->nArmorPiercingRating;
	node->m_data.fSelfDamageMultiplier = bulletTemplate->fSelfDamageMultiplier;
	node->m_data.fCriticalHitChance = bulletTemplate->fCriticalHitChance;

	node->m_data.vSpawnPos = pos;
	//physics
	node->m_data.physPt->m_data.pos = pos;
	node->m_data.physPt->m_data.pos_last = pos;
	//randomizam viteza glontului cu un procent anume
	node->m_data.physPt->m_data.speed = shootDir * (bulletTemplate->fSpeed_ini + m_rand.RandFloatSgn(bulletTemplate->fSpeed_ini * 0.075f));
	//default states
	node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_FAST;
	node->m_data.physPt->m_data.bFlagPhysicsEnabled = false;
	//tail
	node->m_data.szTailSize.w = 0.0f;
	node->m_data.szTailSize.h = 0.0f;

	//particularizari gloante
	switch (bulletTemplate->nType)
	{
		case K_LVL_BULLET_INVISIBLE:
		{
			node->m_data.nFlags |= K_LVL_BULLET_FLAG_NOT_BALLISTIC; //not counted when computing accuracy
		}
		break;
		case K_LVL_BULLET_FIRE_JET:
		{
			node->m_data.nFlags |= K_LVL_BULLET_FLAG_NOT_BALLISTIC | K_LVL_BULLET_FLAG_NO_IMPACT_PARTICLES;
			node->m_data.sprBullet.Init(ANM_ACTIVES_SPR_BULLETS_FIRE, 0.0f, 0.0f, 3);
			node->m_data.physPt->m_data.accel = g_vecGravity;
			//bullet tail																			 
			node->m_data.szTailSize.w = 32.0f;
			node->m_data.szTailSize.h = 4.0f;
			//texture rectangle
			node->m_data.rectTailTex = m_sprActives.GetModuleRect_TexCoords(ANM_ACTIVES_SPR_BULLETS_FIRE, 3, 0);
			//shoot with lava blobs too
			if (m_rand.RandFloat(100.0f) < 40.0f)
			{
				AddProp(K_LVL_PROP_FIRE_SOURCE, pos, &D3DXVECTOR2(node->m_data.physPt->m_data.speed.x * (0.7f + m_rand.RandFloatSgn(0.1f)), node->m_data.physPt->m_data.speed.y), &g_vecGravity);
			}
		}
		break;
		case K_LVL_BULLET_SHOTGUN_INCENDIARY:
		case K_LVL_BULLET_SHOTGUN:
		{
			node->m_data.sprBullet.Init(ANM_ACTIVES_SPR_BULLETS_FIRE, 0.0f, 0.0f, 3);
			//Gloantele de shotgun o iau putin in jos
			//node->m_data.physPt->m_data.accel = g_vecGravity;
			//bullet tail																			 
			node->m_data.szTailSize.w = 16.0f;
			node->m_data.szTailSize.h = 2.0f;
			//texture rectangle
			node->m_data.rectTailTex = m_sprActives.GetModuleRect_TexCoords(ANM_ACTIVES_SPR_BULLETS_FIRE, 3, 0);
		}
		break;
		case K_LVL_BULLET_SHOTGUN_SLUG:
		{
			node->m_data.sprBullet.Init(ANM_ACTIVES_SPR_BULLETS_FIRE, 0.0f, 0.0f, 6);
		}
		break;
		case K_LVL_BULLET_TRACER_AIMED_SHOT:
		{
			node->m_data.sprBullet.Init(ANM_ACTIVES_SPR_BULLETS_FIRE, 0.0f, 0.0f, 5);
		}
		break;
		case K_LVL_BULLET_TRACER_RECON:
		{
			node->m_data.sprBullet.Init(ANM_ACTIVES_SPR_BULLETS_FIRE, 0.0f, 0.0f, 7);
			//bullet tail																			 
			node->m_data.szTailSize.w = 48.0f;
			node->m_data.szTailSize.h = 3.6f;
			//texture rectangle
			node->m_data.rectTailTex = m_sprActives.GetModuleRect_TexCoords(ANM_ACTIVES_SPR_BULLETS_FIRE, 9, 0);
		}
		break;
		case K_LVL_BULLET_TRACER1:
		{
			node->m_data.sprBullet.Init(ANM_ACTIVES_SPR_BULLETS_FIRE, 0.0f, 0.0f, 1);
			//bullet tail																			 
			node->m_data.szTailSize.w = 32.0f;
			node->m_data.szTailSize.h = 2.6f;
			//texture rectangle
			node->m_data.rectTailTex = m_sprActives.GetModuleRect_TexCoords(ANM_ACTIVES_SPR_BULLETS_FIRE, 3, 0);
		}
		break;
		case K_LVL_BULLET_FLASHBANG:
		{
			node->m_data.sprBullet.Init(ANM_ACTIVES_SPR_FLASHBANG, 0.0f, 0.0f, 0);
			//physics
			node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_PRECISE;
			node->m_data.physPt->m_data.bFlagPhysicsEnabled = true;
			//gravity
			node->m_data.physPt->m_data.accel = g_vecGravity;
			node->m_data.nFlags |= K_LVL_BULLET_FLAG_NOT_BALLISTIC;
		}
		break;
		case K_LVL_BULLET_CAM_BALL:
		{
			node->m_data.sprBullet.Init(ANM_ACTIVES_SPR_GRENADE_CAM_BALL, 0.0f, 0.0f, 0);
			//physics
			node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_PRECISE;
			node->m_data.physPt->m_data.bFlagPhysicsEnabled = true;
			//gravity
			node->m_data.physPt->m_data.accel = g_vecGravity;
			node->m_data.nFlags |= K_LVL_BULLET_FLAG_NOT_BALLISTIC;
			node->m_data.nSubstate = 0;
		}
		break;
		case K_LVL_BULLET_SMOKE_GRENADE:
		{
			node->m_data.sprBullet.Init(ANM_ACTIVES_SPR_GRENADE, 0.0f, 0.0f, 0);
			//physics
			node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_PRECISE;
			node->m_data.physPt->m_data.bFlagPhysicsEnabled = true;
			//gravity
			node->m_data.physPt->m_data.accel = g_vecGravity;
			node->m_data.nFlags |= K_LVL_BULLET_FLAG_NOT_BALLISTIC;
			//play smoker sound
			//SND_PLAY_POSITIONAL(SNDIDX_SMOKE_GRENADE, pos);
		}
		break;
		case K_LVL_BULLET_GOO:
		{
			node->m_data.sprBullet.Init(ANM_ACTIVES_SPR_GOO_FLYING, 0.0f, 0.0f, 0);
			//physics
			node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_PRECISE;
			node->m_data.physPt->m_data.bFlagPhysicsEnabled = true;
			//gravity
			node->m_data.physPt->m_data.accel = g_vecGravity;
			node->m_data.nFlags |= K_LVL_BULLET_FLAG_NOT_BALLISTIC;
		}
		break;
		case K_LVL_BULLET_MOLOTOV:
		{
			node->m_data.sprBullet.Init(ANM_ACTIVES_SPR_MOLOTOV_BOTTLE1, 0.0f, 0.0f, 0);
			//physics
			node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_PRECISE;
			node->m_data.physPt->m_data.bFlagPhysicsEnabled = true;
			//gravity
			node->m_data.physPt->m_data.accel = g_vecGravity;
			node->m_data.nFlags |= K_LVL_BULLET_FLAG_NOT_BALLISTIC;
		}
		break;
		case K_LVL_BULLET_GRENADE_ROUND:
		{
			node->m_data.sprBullet.Init(ANM_ACTIVES_SPR_GRENADE_ROUND, 0.0f, 0.0f, 0);
			//physics
			node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_PRECISE;
			node->m_data.physPt->m_data.bFlagPhysicsEnabled = true;
			//gravity
			node->m_data.physPt->m_data.accel = g_vecGravity;
			node->m_data.nFlags |= K_LVL_BULLET_FLAG_NOT_BALLISTIC;
		}
		break;
		case K_LVL_BULLET_GRENADE:
		{
			node->m_data.sprBullet.Init(ANM_ACTIVES_SPR_GRENADE, 0.0f, 0.0f, 0);
			//physics
			node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_PRECISE;
			node->m_data.physPt->m_data.bFlagPhysicsEnabled = true;
			//gravity
			node->m_data.physPt->m_data.accel = g_vecGravity;
			node->m_data.nFlags |= K_LVL_BULLET_FLAG_NOT_BALLISTIC;
		}
		break;
		case K_LVL_BULLET_BREACHING_CHARGE:
		{
			node->m_data.sprBullet.Init(ANM_ACTIVES_SPR_BREACHING_CHARGE, 0.0f, 0.0f, 0);
			//physics
			node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_PRECISE;
			node->m_data.physPt->m_data.bFlagPhysicsEnabled = true;
			//gravity
			node->m_data.physPt->m_data.accel = g_vecGravity;
			node->m_data.nFlags |= K_LVL_BULLET_FLAG_NOT_BALLISTIC;
		}
		break;
		
		default:
		case K_LVL_BULLET_MELEE_SAW:
		case K_LVL_BULLET_MELEE:
		{
			ErrorBox(K_ERR_WARNING, L"Shouldn't get here!");
		}
		break;
	}

	return &node->m_data;
}

CBullet* CLevel::GetClosestBullet(D3DXVECTOR2 vCheckPos, EBulletType nBulletType, float fMaxDistance, int dwOwnerUID /*= 0*/)
{
	float fMinDist = 100000.0f;
	CBullet* pRetBullet = null;

	CLinkedPool<CBullet>::CLinkedPoolNode *node = m_poolBullets.pListUsed.m_pNext;
	while (node != &m_poolBullets.pListUsed)
	{
		//salvez locatia urmatoare ca sa pot avansa pe ea
		CLinkedPool<CBullet>::CLinkedPoolNode *nextnode = node->m_pNext;
		CBullet* bullet = &node->m_data;

		bool bPassed = true;
		if (bullet->type != nBulletType)
			bPassed = false;
		if ((dwOwnerUID != 0) && (bullet->ownerUID != dwOwnerUID))
			bPassed = false;
		if (bPassed)
		{
			float fDist = D3DXVec2Length(&(vCheckPos - bullet->physPt->m_data.pos));
			if ((fMaxDistance <= 0.0f) || ((fMaxDistance > 0.0f) && (fDist <= fMaxDistance)))
			{
				if (fDist < fMinDist)
				{
					fMinDist = fDist;
					pRetBullet = bullet;
				}
			}
		}

		//avansez pointer
		node = nextnode;
	}

	return pRetBullet;
}

void CLevel::ReleaseBullet(int nBulletType, UINT32 nOwnerUID)
{
	CLinkedPool<CBullet>::CLinkedPoolNode *node = m_poolBullets.pListUsed.m_pNext;
	while (node != &m_poolBullets.pListUsed)
	{
		//salvez locatia urmatoare ca sa pot avansa pe ea
		CLinkedPool<CBullet>::CLinkedPoolNode *nextnode = node->m_pNext;
		CBullet* bullet = &node->m_data;

		bool killbullet = false;
		if ((bullet->type == nBulletType) && (bullet->ownerUID == nOwnerUID))
			killbullet = true;

		//ii dam release
		if (killbullet)
		{
			//release la nodul de fizica !!!
			m_poolPhysPts.DismissNode(bullet->physPt);
			//si eliberez glontul
			m_poolBullets.DismissNode(node);
		}

		//avansez pointer
		node = nextnode;
	}
}

void CLevel::UpdateBullets(float dTime)
{
	//we'll store some important bullets in m_arrBulletsTemp so we can quickly check them later on when updating AIs
	m_arrBulletsTemp.Clear();

	static _VERTEX_PNCT4T4 arrBulletsTris[K_LVL_BULLETS_MAX_CNT * 6];
	//mesh dinamic pentru gloante
	m_bulletsMeshIdx = -1;
	int nBulletsTrisCnt = 0;

	bool bGoreEnabled = UTGetAppClass().m_Settings.bGoreEnabled;

	CLinkedPool<CBullet>::CLinkedPoolNode *node = m_poolBullets.pListUsed.m_pNext;
	while (node != &m_poolBullets.pListUsed)
	{
		//salvez locatia urmatoare ca sa pot avansa pe ea
		CLinkedPool<CBullet>::CLinkedPoolNode *nextnode = node->m_pNext;
		CBullet* bullet = &node->m_data;
		
		float fBulletOldLife = bullet->fLife;
		dec_limit(bullet->fLife, dTime, 0.0f);

		bool killbullet = false;
		
		//daca iese din zona de joc
		if (bullet->physPt->m_data.bIsDead)
			killbullet = true;

		switch (bullet->type)
		{
			case K_LVL_BULLET_INVISIBLE:
			case K_LVL_BULLET_SHOTGUN_SLUG:
			case K_LVL_BULLET_TRACER_AIMED_SHOT:
			case K_LVL_BULLET_TRACER_RECON:
			case K_LVL_BULLET_TRACER1:
			case K_LVL_BULLET_MELEE:
			case K_LVL_BULLET_MELEE_SAW:
			case K_LVL_BULLET_SHOTGUN_INCENDIARY:
			case K_LVL_BULLET_SHOTGUN:
			case K_LVL_BULLET_FIRE_JET:
			{
				///--- pentru cele care pierd din damage cu distanta
				if (bullet->fDamageLossPPx > 0.0f)
				{
					float fDist = D3DXVec2Length(&(bullet->physPt->m_data.pos - bullet->physPt->m_data.pos_last));
					bullet->fDamage -= fDist * bullet->fDamageLossPPx;
					//clamp
					if (bullet->fDamage < 0.0f)
						bullet->fDamage = 0.0f;
				}

				///--- out of range/life so kill it
				if ((bullet->fLife <= 0.0f) || (bullet->fDamage <= 0.0f))
				{
					killbullet = true;
				}
				///--- check enemy hit
				bool foundCollision = false;
				//check only closeby actors
				CActor* pHitActor = null;
				float fHitActorDist = 100000.0f;
				bool bSavedHitCover = false;
				bool bHitActor = false;
				D3DXVECTOR2 vecHitPoint(0.0f, 0.0f), vecHitPointCover(0.0f, 0.0f);
				
				//get closest hit actor
				for (int kk = 0; (kk < m_visibleList.logic_actors_closeby.Count()); kk++)
				{
					CActor* act = m_visibleList.logic_actors_closeby.m_pData[kk];
					//sar actorii ascunsi si cei care nu sunt targets
					if ((act->bHidden) || (act->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET))
						continue;
					//evit friendly fire
					if (act->templateActor.actorClass == bullet->actorClass)
						continue;
					//if it hits an enemy
					D3DXVECTOR2 retPt;
					CAABB box = act->bbox;
					//skip if bbox not set
					if ((box.vSize.x <= 0.0f) || (box.vSize.y <= 0.0f))
						continue;

					//hits cover and bullet doesn't have ignore cover flag, and actor is crouched
					bool bHitCover = false;
					D3DXVECTOR2 retPtCover;
					if ((act->pCover != NULL) && (act->bCrouched) && ((bullet->nFlags & K_LVL_BULLET_FLAG_IGNORE_COVER) == 0))
					{
						bHitCover = AABB_Segment_Intersection(bullet->physPt->m_data.pos_last, bullet->physPt->m_data.pos, act->pCover->bbox, &retPtCover);
					}
					//daca loveste actorul
					bool bHitActor = AABB_Segment_Intersection(bullet->physPt->m_data.pos_last, bullet->physPt->m_data.pos, box, &retPt);

					//save hit actor
					float fDist = D3DXVec2Length(&(bullet->vSpawnPos - act->posHeart));
					if ((bHitActor) || (bHitCover))
					{
						if ((pHitActor == null) || ((pHitActor != null) && (fDist < fHitActorDist)))
						{
							pHitActor = act;
							fHitActorDist = fDist;
							bSavedHitCover = bHitCover;
							//did we hit the actor or his cover?
							if (bHitActor)
								vecHitPoint = retPt;
							else
								vecHitPoint = act->posHeart; //we did hit the cover but bullet never reached the actor.
							//did we hit cover?
							if (bHitCover)
								vecHitPointCover = retPtCover;
						}
					}
				}

				//did we hit actor?
				if (pHitActor != null)
				{
					//vede daca a lovit mai intai coverul sau omul
					if (bSavedHitCover)
					{
						//#HACK: partea asta nu va merge daca trag inamicii si altfel decat orizontal. Va trebui facut cu distanta fata de glont
						if (SIGN(bullet->physPt->m_data.speed.x) == SIGN(vecHitPoint.x - vecHitPointCover.x))
						{
							if ((bullet->nFlags & K_LVL_BULLET_FLAG_NO_IMPACT_PARTICLES) == 0)
							{
								g_particlesMgr.GenerateBulletHitWall(vecHitPointCover, -bullet->physPt->m_data.speed, K_PART_LAYER_RT_FRONT_NRM);
								//#TODO: de pus zgomot de hit wall
								//SND_PLAY_POSITIONAL_RAND2(SNDIDX_BULLET_HIT_SHIELD_01, SNDIDX_BULLET_HIT_SHIELD_02, vecHitPointCover);
							}

							bullet->physPt->m_data.SetPosForced(vecHitPointCover);

							killbullet = true;
							break;
						}
					}

					//hit actor only if for the first time
					if (bullet->dwLastTargetUID != pHitActor->UID)
					{
						//place bullet on first collision
						bullet->physPt->m_data.SetPosForced(vecHitPoint);
						//save uid
						bullet->dwLastTargetUID = pHitActor->UID;

						bool bIsCritical = false;
						if ((bullet->fCriticalHitChance > 0.0f) && (pHitActor->fLife > 0.0f) && (pHitActor->templateActor.eMaterial == K_LVL_MATERIAL_FLESH))
						{
							if (m_rand.RandFloat(1.0f) <= bullet->fCriticalHitChance)
								bIsCritical = true;
							//don't get critical hits while rolling
							if ((bIsCritical) && (pHitActor->nRolling == K_STATE_EXECUTING))
								bIsCritical = false;
						}

						//PERKS
						//special critical bullet flags
						if ((bullet->nFlags & K_LVL_BULLET_FLAG_CRITICAL_IF_SCARED) && (pHitActor->nIconType == K_LVL_ACT_ICON_SURPRISE))
							bIsCritical = true;
						//special can cripple (only human enemies)
						if ((bullet->nFlags & K_LVL_BULLET_FLAG_CAN_CRIPPLE) && (pHitActor->templateActor.actorClass == K_LVL_ACT_CLASS_HUMAN) && (pHitActor->cDamageOverTime.eType != CDamageOverTime::K_LVL_DoT_CRIPPLED))
						{
							if (m_rand.RandInt(1000) < 50)
							{
								g_particlesMgr.AddStringParticle(g_font5ns2, g_stringsMgr.strings[STR_CRIPPLED]->sText, &vecHitPoint, NULL, &D3DXVECTOR2(0.0f, -10.0f), 0.8f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.4f, 0xccaeffc2, K_PART_LAYER_NORMAL);
								SetActorDoT(pHitActor, CDamageOverTime::K_LVL_DoT_CRIPPLED, 2.0f, 0.0f, K_LVL_ACT_CLASS_PLAYER, K_LVL_ACT_CLASS_HUMAN, bullet->ownerUID);
							}
						}
						//special ignore armor or critical shot on highlighted targets
						if ((bullet->nFlags & K_LVL_BULLET_FLAG_SURGEON_IF_TARGETED) && (pHitActor->cDamageOverTime.eType == CDamageOverTime::K_LVL_DoT_TARGETED))
						{
							if (pHitActor->fArmor > 0.0f)
								bullet->nFlags |= K_LVL_BULLET_FLAG_IGNORE_ARMOR;
							else
								bIsCritical = true;
						}
						//vulture mode
						if ((bullet->nFlags & K_LVL_BULLET_FLAG_CRITICAL_FROM_BEHIND) && (pHitActor->templateActor.actorClass == K_LVL_ACT_CLASS_HUMAN) && 
							(pHitActor->lookDirXsign == SIGN(bullet->physPt->m_data.speed.x)))
						{
							if (m_rand.RandInt(1000) < 100)
								bIsCritical = true;
						}

						if(bIsCritical)
						{
							//decided headshot
							bullet->nFlags |= K_LVL_BULLET_FLAG_DIE_ON_IMPACT;
							//do more damage too
							bullet->fDamage *= 1.3f + m_rand.RandFloat(0.2f);
							//sunet headshot
							//SND_PLAY_POSITIONAL_RAND2(SNDIDX_BULLET_HIT_BODY_CRITICAL_01, SNDIDX_BULLET_HIT_BODY_CRITICAL_02, vecHitPoint);
							
							g_particlesMgr.AddStringParticle(g_font5ns2, g_stringsMgr.strings[STR_CRITICAL_HIT]->sText, &vecHitPoint, NULL, &D3DXVECTOR2(0.0f, -10.0f), 0.8f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.4f, 0xccffaec2, K_PART_LAYER_NORMAL);
							//headshot effect
							if (bGoreEnabled)
							{
								DWORD dwEffectColor = 0xffff0000;
								if (pHitActor->templateActor.actorClass == K_LVL_ACT_CLASS_ZOMBIE)
									dwEffectColor = 0xff82b600;
								g_particlesMgr.GenerateHeadshot(vecHitPoint, -bullet->physPt->m_data.speed, dwEffectColor, K_PART_LAYER_RT_FRONT_NRM);
							}
						}

						//--- hit actor ---
						//momentum must not be 0.0 
						float fMoment = max(0.01f, bullet->fMomentum);
						D3DXVECTOR2 vProjMomentum = bullet->physPt->m_data.speed * fMoment;

						CBulletHitReturnData retData = HitActor(pHitActor, bullet, &vProjMomentum);

						//--- particles ---
						if ((bullet->nFlags & K_LVL_BULLET_FLAG_NO_IMPACT_PARTICLES) == 0)
						{
							if (retData.eMaterial == K_LVL_MATERIAL_FLESH)
							{
								if (bGoreEnabled)
								{
									g_particlesMgr.GenerateBulletHitEnemy(vecHitPoint, -bullet->physPt->m_data.speed, pHitActor->templateActor.actorClass, K_PART_LAYER_RT_FRONT_NRM);
									//--- generate blood splats ---
									if ((randfloat(100.0f) <= 20.0f) && (pHitActor->fLife > 0.0f))
									{
										//splaturile sunt sortate in fn de marime
										if ((bullet->nFlags & K_LVL_BULLET_FLAG_NO_DECALS) == 0)
											AddDecal_BloodSplat(bullet->physPt->m_data.pos, false, pHitActor->templateActor.actorClass);
									}
									//efecte blood
									if ((retData.bKilledTarget) && (bullet->actorClass == K_LVL_ACT_CLASS_PLAYER))
									{
										DWORD dwEffectColor = 0xffff0000;
										if (pHitActor->templateActor.actorClass == K_LVL_ACT_CLASS_ZOMBIE)
											dwEffectColor = 0xff82b600;
										g_particlesMgr.GenerateHeadshot(vecHitPoint, -bullet->physPt->m_data.speed, dwEffectColor, K_PART_LAYER_RT_FRONT_NRM);
									}
								}
								else
								{
									g_particlesMgr.GenerateBulletHitWall(vecHitPoint, -bullet->physPt->m_data.speed, K_PART_LAYER_RT_FRONT_NRM);
								}

								//SND_PLAY_POSITIONAL_RAND2(SNDIDX_BULLET_HIT_BODY_GENERIC_01, SNDIDX_BULLET_HIT_BODY_GENERIC_02, pHitActor->GetPosHeart());
							}
							else //metal shield
							{
								g_particlesMgr.GenerateBulletHitMetal(vecHitPoint, -bullet->physPt->m_data.speed, K_PART_LAYER_RT_FRONT_NRM);
								//SND_PLAY_POSITIONAL_RAND2(SNDIDX_BULLET_HIT_SHIELD_01, SNDIDX_BULLET_HIT_SHIELD_02, pHitActor->GetPosHeart());
							}

						}
						//--- stats ---
						if (((bullet->nFlags & K_LVL_BULLET_FLAG_NOT_BALLISTIC) == 0) && (bullet->actorClass == K_LVL_ACT_CLASS_PLAYER))
						{
							//bullet hits
							//#TODO: ar trebui scapat de GetPlayerByUID de aici
							CActor* pPlayer = GetPlayerByUID(bullet->ownerUID);
							if (pPlayer != NULL)
							{
								m_arrStats[K_LVL_STATS_PL1_BULLETS_HIT + pPlayer->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT]++;
							}
						}
						//iese din for
						foundCollision = true;

						//vede daca mai are putere glontul sa treaca prin target
						float fSelfDmg = bullet->fSelfDamageMultiplier;
						if (retData.bArmorHit) //did we hit higher armor? Bullet loses energy
							fSelfDmg = 1.0f;

						bullet->fDamage -= retData.fPointsTaken * fSelfDmg;
						//very low damage so just kill it
						if ((bullet->fDamage <= 0.1f) || (bullet->nFlags & K_LVL_BULLET_FLAG_DIE_ON_IMPACT))
						{
							bullet->fDamage = 0.0f;
							killbullet = true;
							//dead on collision with enemy
							bullet->physPt->m_data.SetPosForced(vecHitPoint);
						}
					}
				}
				else //check cover hit anyway
				{
					//no target, reset UID
					bullet->dwLastTargetUID = 0;

					if (bSavedHitCover)
					{
						if ((bullet->nFlags & K_LVL_BULLET_FLAG_NO_IMPACT_PARTICLES) == 0)
							g_particlesMgr.GenerateBulletHitWall(vecHitPointCover, -bullet->physPt->m_data.speed, K_PART_LAYER_RT_FRONT_NRM);

						//SND_PLAY_POSITIONAL_RAND2(SNDIDX_BULLET_HIT_SHIELD_01, SNDIDX_BULLET_HIT_SHIELD_02, vecHitPointCover);

						bullet->physPt->m_data.SetPosForced(vecHitPointCover);

						killbullet = true;
						break;
					}
				}
				
				///--- hits collision boxes? ---
				//aici ar putea fi mici probleme la gloantele rapide, adica sa sara verificarea cu peretii si sa traga prin ei
				if ((!killbullet) && (bullet->physPt->m_data.bContacting == true))
				{
					killbullet = true;

					if ((bullet->nFlags & K_LVL_BULLET_FLAG_NO_IMPACT_PARTICLES) == 0)
						g_particlesMgr.GenerateBulletHitWall(bullet->physPt->m_data.pos, bullet->physPt->m_data.contactNormal, K_PART_LAYER_RT_FRONT_NRM);

					//HARDCODE: gloantele care sparg usile si ferestrele
					if (bullet->physPt->m_data.pContactShape != NULL)
					{
						//daca e usa si glontul poate sparge usi si glontul este tras recent atunci sparge usa
						if (bullet->physPt->m_data.pContactShape->AIstate == K_AI_STATE_COLL_BREAKABLE_DOOR)
						{
							CCollisionShape* shape = bullet->physPt->m_data.pContactShape;
							//bullets breaking doors
							if (((bullet->nFlags & K_LVL_BULLET_FLAG_BREAKS_DOORS) != 0) && (D3DXVec2LengthSq(&(bullet->physPt->m_data.pos - bullet->vSpawnPos)) < K_TILE_SIZE * K_TILE_SIZE))
							{
								bool bImmune = false;
								//is it reinforced? only the SAW can breach it
								if (bullet->type != K_LVL_BULLET_MELEE_SAW)
								{
									if (shape->varAIparams.GetVariantByName(L"b_reinforced")->m_asINT32 != 0)
										bImmune = true;
								}

								//scadem viata
								if (!bImmune)
									shape->AIfvar1 -= bullet->fDamage;
								//mark hit
								shape->AIvarBool1 = true;
								//daca a murit usa salvam directia fortei aplicata de glont
								shape->varAIparams.SetNamedVarFloat(L"fForceDirX", bullet->physPt->m_data.speed.x);
								shape->varAIparams.SetNamedVarINT32(L"bExploded", 0);
							}
							//player bullets sound on the other side of the door
							if (bullet->actorClass == K_LVL_ACT_CLASS_PLAYER)
							{
								D3DXVECTOR2 sndpos1 = D3DXVECTOR2(shape->bbox.vCenter.x + SIGN(bullet->physPt->m_data.speed.x) * (shape->bbox.vHalfSize.x + 2.0f), shape->bbox.vCenter.y);
								AddAIEvent(K_LVL_AI_EVENT_SOUND_BEHIND_DOOR, 0, K_LVL_ACT_CLASS_PLAYER, sndpos1, 160.0f, 0.5f);
							}
						}
						//daca e fereastra
						else if (bullet->physPt->m_data.pContactShape->AIstate == K_AI_STATE_COLL_BREAKABLE_WINDOW)
						{
							CCollisionShape* shape = bullet->physPt->m_data.pContactShape;
							//scadem viata
							shape->AIfvar1 -= bullet->fDamage;
							//mark hit
							shape->AIvarBool1 = true;
							//daca a murit usa salvam directia fortei aplicata de glont
							shape->varAIparams.SetNamedVarFloat(L"fForceDirX", bullet->physPt->m_data.speed.x);
							//don't kill the bullet when passing through the glass!!!
							killbullet = false;
						}
					}
				}

				//unele gloante lasa in spate particule
				bool bLeaveTrail = false;
				float fTrailStep = 3.0f;
				DWORD dwTrailColor = 0xffffffff;
				int nAnmIdx = ANM_PARTICLES_SPR_FIRESPARK2;
				if (bullet->type == K_LVL_BULLET_TRACER_AIMED_SHOT)
				{
					bLeaveTrail = true;
					dwTrailColor = 0xaaff5555;
					fTrailStep = 3.0f;
				}
				else if (bullet->type == K_LVL_BULLET_SHOTGUN_SLUG)
				{
					bLeaveTrail = true;
					dwTrailColor = 0x995555ff;
					fTrailStep = 4.0f;
				}
				else if (bullet->type == K_LVL_BULLET_SHOTGUN_INCENDIARY)
				{
					bLeaveTrail = true;
					dwTrailColor = 0x99fdb727;
					fTrailStep = 6.0f;
				}
				else if (bullet->type == K_LVL_BULLET_FIRE_JET)
				{
					bLeaveTrail = true;
					dwTrailColor = 0xffffffff;
					fTrailStep = 2.0f;
					nAnmIdx = ANM_PARTICLES_SPR_FLAME_S1 + randint(4);
				}

				if(bLeaveTrail)
				{
					D3DXVECTOR2 start = bullet->physPt->m_data.pos_last;
					D3DXVECTOR2 end = bullet->physPt->m_data.pos;
					float fLen = D3DXVec2Length(&(end - start));
					D3DXVECTOR2 dir = (end - start) / fLen;
					dir *= fTrailStep;

					for (int kk = 0; kk < (int)(fLen / fTrailStep); kk++)
					{
						g_particlesMgr.AddParticle(nAnmIdx, true, randint(3), &(start + kk * dir), NULL, &randD3DXVECTOR2sgn(5.0f, 5.0f), 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, dwTrailColor, K_PART_LAYER_FRONT_LIGHT);
					}
				}
			}
			break;
			case K_LVL_BULLET_FLASHBANG:				
			{
				bullet->sprBullet.Update(&m_sprActives, dTime);
				//#PERK: DOUBLE BANGERS - flashbang bangs twice
				if ((fBulletOldLife > bullet->fLife_ini * 0.5f) && (bullet->fLife <= bullet->fLife_ini * 0.5f))
				{
					CActor* pPlayer = GetPlayerByUID(bullet->ownerUID);
					if (pPlayer != null)
					{
						if (g_playerSelScr.IsPerkEnabled(pPlayer->nPlayerOrdinal, &shPerk_DOUBLE_BANGERS))
						{
							D3DXVECTOR2 explopos = bullet->physPt->m_data.pos;
							if (bullet->physPt->m_data.bContacting)
								explopos += bullet->physPt->m_data.contactNormal * 2.0f;
							AddProp_Explo(hash_EXPLO_FLASHBANG, explopos, bullet->ownerUID, bullet->actorClass);
							AddDecal(K_LVL_DECAL_LAYER_BACKWALLS, explopos, ANM_ACTIVES_SPR_DECAL_EXPLOMARKS, randint(3), 0xffffffff);
						}
					}
				}

				//daca depaseste range-ul
				if (bullet->fLife <= 0.0f)
				{
					killbullet = true;
					//add explosion
					//!!! because punctul fizic ia direct coordonata marginii de bbox mut explozia mai sus ca sa nu am probleme la testul de coliziuni
					D3DXVECTOR2 explopos = bullet->physPt->m_data.pos;
					if (bullet->physPt->m_data.bContacting)
						explopos += bullet->physPt->m_data.contactNormal * 2.0f;
					AddProp_Explo(hash_EXPLO_FLASHBANG, explopos, bullet->ownerUID, bullet->actorClass);
					//decal explo mark
					AddDecal(K_LVL_DECAL_LAYER_BACKWALLS, explopos, ANM_ACTIVES_SPR_DECAL_EXPLOMARKS, randint(3), 0xffffffff);
				}
				if (bullet->physPt->m_data.bContacting)
				{
					bullet->sprBullet.SetFrame(0);
				}
			}
			break;
			case K_LVL_BULLET_SMOKE_GRENADE:
			{
				//daca depaseste range-ul de viata
				if (bullet->fLife <= 0.0f)
				{
					killbullet = true;
				}
				if (bullet->physPt->m_data.bContacting)
				{
					bullet->sprBullet.SetFrame(0);
				}
				//generate particles
				if (m_Timers.Tick(40))
				{
					g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_SMOKE, false, randint(2), &D3DXVECTOR2(bullet->physPt->m_data.pos.x + randfloatsgn(2.0f), bullet->physPt->m_data.pos.y + randfloatsgn(2.0f)),
						&D3DXVECTOR2(0.0f, -30.0f), &D3DXVECTOR2(randfloatsgn(50.0f), -10.0f - randfloat(30.0f)), 3.0f + randfloat(1.0f), 0.5f, 0.25f, randfloat(PI), randfloatsgn(0.3f), 0.5f, 2.0f, 0x33aa3333, K_PART_LAYER_FRONT, 2.0f);
					g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_SMOKE, false, randint(2), &D3DXVECTOR2(bullet->physPt->m_data.pos.x + randfloatsgn(2.0f), bullet->physPt->m_data.pos.y + randfloatsgn(2.0f)),
						&D3DXVECTOR2(0.0f, -30.0f), &D3DXVECTOR2(randfloatsgn(50.0f), -10.0f - randfloat(30.0f)), 3.0f + randfloat(1.0f), 0.5f, 0.25f, randfloat(PI), randfloatsgn(0.3f), 0.5f, 2.0f, 0x33ff6666, K_PART_LAYER_FRONT, 2.0f);
				}
				//save bullet for later check
				if (!killbullet)
					m_arrBulletsTemp.Add(bullet);
			}
			break;
			case K_LVL_BULLET_CAM_BALL:
			{
				if (!bullet->physPt->m_data.bContacting)
					bullet->sprBullet.Update(&m_sprActives, dTime);
				//die after a while
				if (bullet->fLife <= 0.0f)
					killbullet = true;
				if ((node->m_data.nSubstate == 0) && (bullet->physPt->m_data.bIsStatic))
				{
					g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_CAMBALL_DIGITAL, true, 0, &bullet->physPt->m_data.pos, NULL, NULL, 3.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_FRONT);
					AddProp_Light(bullet->physPt->m_data.pos, ANM_LIGHTS_SPR_POINT1, 0.6f, 0.2f, 0xff00d0ff, 2.0f);
					m_screenVignette.Init(0.2f, 0xff00d0ff, 0.0f, 0.2f, 0.6f);

					node->m_data.nSubstate = 1;
				}

				//mark them always (1 sec duration)
				if (m_Timers.Tick(1000))
				{
					D3DXVECTOR2 pos = bullet->physPt->m_data.pos;
					if (bullet->physPt->m_data.bContacting)
						pos += bullet->physPt->m_data.contactNormal * 2.0f;
					AddProp_Explo(hash_EXPLO_FAKE_CAM_BALL_1SEC, pos, bullet->ownerUID, bullet->actorClass);
				}

				//remove fog of war
				if ((killbullet) || (m_Timers.Tick(100)))
				{
					CCollisionShape* colshape = GetCollisionShapeAt(bullet->physPt->m_data.pos, K_LVL_COLL_TYPE_FOG_OF_WAR);
					if (colshape != null)
						colshape->AIfvar1 = 1.0f - EPS;
				}
				//save bullet for later check
				if (killbullet)
				{
					//smoke puff when dead
					int nAnmId = ANM_PARTICLES_SPR_PUFF_S_XS;
					if (randompercent(50.0f))
						nAnmId = ANM_PARTICLES_SPR_PUFF_S_XXS;

					g_particlesMgr.AddParticle(nAnmId, true, 0, &bullet->physPt->m_data.pos,
						NULL, NULL, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xaaffffff, K_PART_LAYER_RT_FRONT_NRM);
				}
				else
				{
					m_arrBulletsTemp.Add(bullet);
				}
			}
			break;
			case K_LVL_BULLET_GRENADE_ROUND:
			case K_LVL_BULLET_GRENADE:
			{
				bullet->sprBullet.Update(&m_sprActives, dTime);
				//daca depaseste range-ul
				if (bullet->fLife <= 0.0f)
				{
					killbullet = true;
					//add explosion
					//!!! because punctul fizic ia direct coordonata marginii de bbox mut explozia mai sus ca sa nu am probleme la testul de coliziuni
					D3DXVECTOR2 explopos = bullet->physPt->m_data.pos;
					if (bullet->physPt->m_data.bContacting)
					{
						explopos += bullet->physPt->m_data.contactNormal * 2.0f;
						AddProp_Explo(hash_EXPLO_GRENADE_GROUND, explopos, bullet->ownerUID, bullet->actorClass);
					}
					else
						AddProp_Explo(hash_EXPLO_GRENADE, explopos, bullet->ownerUID, bullet->actorClass);
					//decal explo mark
					AddDecal(K_LVL_DECAL_LAYER_BACKWALLS, explopos, ANM_ACTIVES_SPR_DECAL_EXPLOMARKS, randint(3), 0xffffffff);
				}
				if (bullet->physPt->m_data.bContacting)
				{
					bullet->sprBullet.SetFrame(0);
				}
				//-- red coloring ---
				if (m_Timers.GetTimerValue(300) < 0.15f)
					bullet->sprBullet.color = 0xffff0000;
				else
					bullet->sprBullet.color = 0xffffffff;
			}
			break;
			case K_LVL_BULLET_GOO:
			{
				bullet->sprBullet.Update(&m_sprActives, dTime);

				//break on contact
				bool bHitEnemy = false;
				if (bullet->physPt->m_data.bContactStarted)
				{
					killbullet = true;
					//!!! move collision a little up (not inside collision rect)
					D3DXVECTOR2 explopos = bullet->physPt->m_data.pos;
					if (bullet->physPt->m_data.bContacting)
						explopos += bullet->physPt->m_data.contactNormal * 2.0f;
					//set final position
					bullet->physPt->m_data.pos = explopos;
				}
				//players intersection
				CActor* pHitActor = null;
				float fHitActorDist = 100000.0f;
				bool bSavedHitCover = false;
				bool bHitActor = false;
				D3DXVECTOR2 vecHitPoint(0.0f, 0.0f), vecHitPointCover(0.0f, 0.0f);

				if (!killbullet)
				{
					//get closest hit actor
					for (int kk = 0; (kk < m_visibleList.logic_actors_closeby.Count())/* && (foundCollision == false)*/; kk++)
					{
						CActor* act = m_visibleList.logic_actors_closeby.m_pData[kk];
						//sar actorii ascunsi si cei care nu sunt targets
						if ((act->bHidden) || (act->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET))
							continue;
						//evit friendly fire
						if (act->templateActor.actorClass == bullet->actorClass)
							continue;
						//if it hits an enemy
						D3DXVECTOR2 retPt;
						CAABB box = act->bbox;
						//skip if bbox not set
						if ((box.vSize.x <= 0.0f) || (box.vSize.y <= 0.0f))
							continue;

						//hits cover and bullet doesn't have ignore cover flag, and actor is crouched
						bool bHitCover = false;
						D3DXVECTOR2 retPtCover;
						if ((act->pCover != NULL) && (act->bCrouched) && ((bullet->nFlags & K_LVL_BULLET_FLAG_IGNORE_COVER) == 0))
						{
							bHitCover = AABB_Segment_Intersection(bullet->physPt->m_data.pos_last, bullet->physPt->m_data.pos, act->pCover->bbox, &retPtCover);
						}
						//daca loveste actorul
						bool bHitActor = AABB_Segment_Intersection(bullet->physPt->m_data.pos_last, bullet->physPt->m_data.pos, box, &retPt);

						//save hit actor
						float fDist = D3DXVec2Length(&(bullet->vSpawnPos - act->posHeart));
						if ((bHitActor) || (bHitCover))
						{
							if ((pHitActor == null) || ((pHitActor != null) && (fDist < fHitActorDist)))
							{
								pHitActor = act;
								killbullet = true;
								fHitActorDist = fDist;
								bSavedHitCover = bHitCover;
								//did we hit the actor or his cover?
								if (bHitActor)
									vecHitPoint = retPt;
								else
									vecHitPoint = act->posHeart; //we did hit the cover but bullet never reached the actor.
																 //did we hit cover?
								if (bHitCover)
									vecHitPointCover = retPtCover;
							}
						}
					}
				}

				if (killbullet)
				{
					//SND_PLAY_POSITIONAL(SNDIDX_FRIED, bullet->physPt->m_data.pos);

					if (pHitActor != null)
					{
						AddProp_Explo(hash_EXPLO_GREEN_GOO, bullet->physPt->m_data.pos, bullet->ownerUID, bullet->actorClass);
					}
					else //hit geometry
					{
						AddProp_Explo(hash_EXPLO_GREEN_GOO_GROUND, bullet->physPt->m_data.pos, bullet->ownerUID, bullet->actorClass);
					}
				}
			}
			break;
			case K_LVL_BULLET_MOLOTOV:
			{
				bullet->sprBullet.Update(&m_sprActives, dTime);
				//break on contact
				if (bullet->physPt->m_data.bContactStarted)
				{
					killbullet = true;
					//!!! because punctul fizic ia direct coordonata marginii de bbox mut explozia mai sus ca sa nu am probleme la testul de coliziuni
					D3DXVECTOR2 explopos = bullet->physPt->m_data.pos;
					if (bullet->physPt->m_data.bContacting)
						explopos += bullet->physPt->m_data.contactNormal * 2.0f;
					//set final position
					bullet->physPt->m_data.pos = explopos;
				}
				//players intersection
				for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
				{
					CActor* act = pPlayerActor[kk];
					//sar actorii ascunsi si cei care nu sunt targets
					if ((act == null) || (act->bHidden) || (act->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET))
						continue;
					//evit friendly fire
					if (act->templateActor.actorClass == bullet->actorClass)
						continue;
					//daca lovesc inamic
					D3DXVECTOR2 retPt;
					CAABB box = act->bbox;
					//daca am bbox 0 nu verific
					if ((box.vSize.x <= 0.0f) || (box.vSize.y <= 0.0f))
						continue;

					//daca loveste cover - in cazul in care nu are flag de ignore cover
					bool bHitCover = false;
					D3DXVECTOR2 retPtCover;
					if ((act->pCover != NULL) && ((bullet->nFlags & K_LVL_BULLET_FLAG_IGNORE_COVER) == 0))
					{
						bHitCover = AABB_Segment_Intersection(bullet->physPt->m_data.pos_last, bullet->physPt->m_data.pos, act->pCover->bbox, &retPtCover);
					}
					//daca loveste actorul
					bool bHitActor = AABB_Segment_Intersection(bullet->physPt->m_data.pos_last, bullet->physPt->m_data.pos, box, &retPt);

					if ((bHitActor) || (bHitCover))
						killbullet = true;
				}
			}
			break;
			case K_LVL_BULLET_BREACHING_CHARGE:
			{
				bullet->sprBullet.Update(&m_sprActives, dTime);
				//sticks to it							//aici era sa se lipeasca doar de usi
				if (bullet->physPt->m_data.bContactStarted)// && (bullet->physPt->m_data.pContactShape->AIstate == K_AI_STATE_COLL_EXPLOCHARGE_TOUCH))
				{
					bullet->physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_NONE;
					bullet->physPt->m_data.bFlagPhysicsEnabled = false;
					bullet->physPt->m_data.bIsStatic = true;
					bullet->physPt->m_data.speed = D3DXVECTOR2(0.0f, 0.0f);
					bullet->physPt->m_data.accel = D3DXVECTOR2(0.0f, 0.0f);
					bullet->physPt->m_data.pos = bullet->physPt->m_data.contactPos;

					//SND_PLAY_POSITIONAL(SNDIDX_BREACHING_CHARGE_IN_PLACE, bullet->physPt->m_data.pos);
				}
				//vede daca nu cumva a disparut collisionul pe care era lipit
				if (bullet->physPt->m_data.bIsStatic)
				{
					if ((bullet->physPt->m_data.pContactShape == null) || (bullet->physPt->m_data.pContactShape->bHidden))
					{
						bullet->physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_PRECISE;
						bullet->physPt->m_data.bFlagPhysicsEnabled = true;
						bullet->physPt->m_data.bIsStatic = false;
						bullet->physPt->m_data.speed = D3DXVECTOR2(0.0f, 0.0f);
						node->m_data.physPt->m_data.accel = g_vecGravity;
					}
				}

				//detonate - KILL IT NOW se comanda din KillBulletsOfType - acolo verifica daca e deja charge in scena
				if (/*(bullet->fLife <= 0.0f) && */(bullet->nFlags & K_LVL_BULLET_FLAG_KILLITNOW))
				{
					killbullet = true;
					//!!! because punctul fizic ia direct coordonata marginii de bbox mut explozia mai sus ca sa nu am probleme la testul de coliziuni
					D3DXVECTOR2 explopos = bullet->physPt->m_data.pos;
					if (bullet->physPt->m_data.bContacting)
						explopos += bullet->physPt->m_data.contactNormal * 2.0f;

					bool bSecondExploPut = false;
					D3DXVECTOR2 chargedir(0.0f, -1.0f); //default charge dir
					//daca e lipita de o usa scade energia usii si genereaza inca o explozie de partea cealalta ca sa omoare oameni
					CCollisionShape* shape = bullet->physPt->m_data.pContactShape;
					if (shape != null)
					{
						chargedir = bullet->physPt->m_data.contactNormal;
						if ((shape->AIstate == K_AI_STATE_COLL_BREAKABLE_DOOR) || (shape->AIstate == K_AI_STATE_COLL_BREAKABLE_WINDOW))
						{
							bool bImmune = false;
							//is it reinforced? only the SAW can breach it
							if (shape->varAIparams.GetVariantByName(L"b_reinforced")->m_asINT32 != 0)
								bImmune = true;

							//daca e lipita de usa o distruge
							if (!bImmune)
							{
								shape->AIfvar1 = 0.0f;
								//was hit
								shape->AIvarBool1 = true;
								//salvez forta aplicata usii
								shape->varAIparams.SetNamedVarFloat(L"fForceDirX", -bullet->physPt->m_data.contactNormal.x);
								shape->varAIparams.SetNamedVarINT32(L"bExploded", 1);
								//generam explozie in partea cealalta a usii (adaug 4.0f ca sa compensez cei 2.0f de mai sus si sa o dau cu inca 2.0f mai departe
								D3DXVECTOR2 vContactNormal = bullet->physPt->m_data.contactNormal;
								float fNrmOffset = shape->bbox.vSize.x + 4.0f;
								if (fabs(vContactNormal.y) > fabs(vContactNormal.x))
									fNrmOffset = shape->bbox.vSize.y + 4.0f;
								D3DXVECTOR2 secondExploPos = explopos - bullet->physPt->m_data.contactNormal * fNrmOffset;

								AddProp_Explo(hash_EXPLO_CHARGE_INVISIBLE, secondExploPos, bullet->ownerUID, bullet->actorClass);
								bSecondExploPut = true;
								//set charge dir
								chargedir = -bullet->physPt->m_data.contactNormal;
							}
						}
					}
					//adaug explozie directionala
					AddProp_Explo(hash_EXPLO_CHARGE, explopos, bullet->ownerUID, bullet->actorClass, chargedir);
					if (!bSecondExploPut)
					{
						//put force explosion on the same side if not on door/window
						AddProp_Explo(hash_EXPLO_CHARGE_INVISIBLE, explopos, bullet->ownerUID, bullet->actorClass);
					}
					//decal explo mark
					AddDecal(K_LVL_DECAL_LAYER_BACKWALLS, explopos, ANM_ACTIVES_SPR_DECAL_EXPLOMARKS, randint(3), 0xffffffff);
				}
			}
			break;
		}

		//grenades break glass
		//-- sunet bounce si mecanica trecere prin geamuri ---
		//#TODO: enemies should run from grenades (should hear glass breaking)
		//#TODO: some bullets should have a flag saying that they can pass through glass
		if ((bullet->type == K_LVL_BULLET_GRENADE) || (bullet->type == K_LVL_BULLET_GRENADE_ROUND) || 
			(bullet->type == K_LVL_BULLET_SMOKE_GRENADE) || 
			(bullet->type == K_LVL_BULLET_CAM_BALL) || (bullet->type == K_LVL_BULLET_FLASHBANG))
		{
			if (bullet->physPt->m_data.bContactStarted)
			{
				//SND_PLAY_POSITIONAL(SNDIDX_GRENADE_HIT_WALL, bullet->physPt->m_data.pos);
				//break glass
				if ((bullet->physPt->m_data.pContactShape != null) && (bullet->physPt->m_data.pContactShape->AIstate == K_AI_STATE_COLL_BREAKABLE_WINDOW))
				{
					CCollisionShape* shape = bullet->physPt->m_data.pContactShape;
					//scadem viata
					shape->AIfvar1 = 0.0f;
					//#HACK: vedem daca e geam vertical sau orizontal ca sa anulam bounce grenada
					bool bVertical = true;
					if (shape->bbox.vSize.x > shape->bbox.vSize.y)
						bVertical = false;

					if (bVertical)
					{
						bullet->physPt->m_data.speed.x *= -1.0f;
						shape->varAIparams.SetNamedVarFloat(L"fForceDirX", bullet->physPt->m_data.speed.x);
					}
					else
					{
						shape->varAIparams.SetNamedVarFloat(L"fForceDirX", 0.0f);
						bullet->physPt->m_data.speed.y *= -1.0f;
					}
				}
			}
		}
		//bullet tails
		D3DXVECTOR2 vHead, vTail, vSpawnPos;
		SIZEWH_F szTail(bullet->szTailSize);
		if (szTail.w >= 0.0f)
		{
			vHead = bullet->physPt->m_data.pos;
			vTail = bullet->physPt->m_data.pos_last;
			vSpawnPos = bullet->vSpawnPos;
		}

		//add actual tail
		if (szTail.w > 0.0f)
		{
			D3DXVECTOR2 vDir = vHead - vTail;
			float fLen = D3DXVec2Length(&vDir);
			vDir /= fLen;
			float fSpawnLen = D3DXVec2Length(&(vHead - vSpawnPos));
			if (szTail.w > fSpawnLen)
				szTail.w = fSpawnLen;
			D3DXVECTOR2 vTan(-vDir.y, vDir.x);
			vTan *= szTail.h * 0.5f;
			vTail = vHead - vDir * szTail.w;

			_VERTEX_PNCT4T4 vCorners[4]; //frontL, frontR, backL, backR
			vCorners[0].pos = D3DXVECTOR3(vHead.x - vTan.x, vHead.y - vTan.y, 0.0f);
			vCorners[0].tex1 = D3DXVECTOR4(bullet->rectTailTex.right, bullet->rectTailTex.top, 0.0f, 0.0f);
			vCorners[0].color = 0xffffffff;
			vCorners[1].pos = D3DXVECTOR3(vHead.x + vTan.x, vHead.y + vTan.y, 0.0f);
			vCorners[1].tex1 = D3DXVECTOR4(bullet->rectTailTex.right, bullet->rectTailTex.bottom, 0.0f, 0.0f);
			vCorners[1].color = 0xffffffff;
			vCorners[2].pos = D3DXVECTOR3(vTail.x - vTan.x, vTail.y - vTan.y, 0.0f);
			vCorners[2].tex1 = D3DXVECTOR4(bullet->rectTailTex.left, bullet->rectTailTex.top, 0.0f, 0.0f);
			vCorners[2].color = 0x22ffffff;
			vCorners[3].pos = D3DXVECTOR3(vTail.x + vTan.x, vTail.y + vTan.y, 0.0f);
			vCorners[3].tex1 = D3DXVECTOR4(bullet->rectTailTex.left, bullet->rectTailTex.bottom, 0.0f, 0.0f);
			vCorners[3].color = 0x22ffffff;

			arrBulletsTris[nBulletsTrisCnt * 3 + 0] = vCorners[0];
			arrBulletsTris[nBulletsTrisCnt * 3 + 1] = vCorners[1];
			arrBulletsTris[nBulletsTrisCnt * 3 + 2] = vCorners[2];

			arrBulletsTris[nBulletsTrisCnt * 3 + 3] = vCorners[1];
			arrBulletsTris[nBulletsTrisCnt * 3 + 4] = vCorners[2];
			arrBulletsTris[nBulletsTrisCnt * 3 + 5] = vCorners[3];

			//increase painted tails count
			nBulletsTrisCnt += 2;
		}

		//release the bullet
		if (killbullet)
		{
			//some bullets explode at the end
			if (bullet->nExploTemplateHash != 0)
			{
				D3DXVECTOR2 vExploDir(0.0f, 0.0f);
				if (bullet->nFlags & K_LVL_BULLET_FLAG_DIRECTIONAL)
				{
					vExploDir = bullet->physPt->m_data.speed;
				}
				D3DXVECTOR2 vExploPos = bullet->physPt->m_data.pos;
				if (bullet->physPt->m_data.bContacting)
					vExploPos += bullet->physPt->m_data.contactNormal * 2.0f;
				//now add explo
				AddProp_Explo(bullet->nExploTemplateHash, vExploPos, bullet->ownerUID, bullet->actorClass, vExploDir);
			}

			//release la nodul de fizica !!!
			m_poolPhysPts.DismissNode(bullet->physPt);
			//si eliberez glontul
			m_poolBullets.DismissNode(node);
		}


		//avansez pointer
		node = nextnode;
	}

	if (nBulletsTrisCnt > 0)
	{
		UINT32 meshidx;
		m_bufferedPainter.BeginMesh(meshidx);
		m_bulletsMeshIdx = (int)meshidx;
		m_bufferedPainter.AddTriangles(arrBulletsTris, nBulletsTrisCnt);
		m_bufferedPainter.EndMesh();
	}
}

void CLevel::PaintBullets(bool paintNormals)
{
	D3DXMATRIXA16 matbullet;
	CLinkedPool<CBullet>::CLinkedPoolNode *node = m_poolBullets.pListUsed.m_pNext;

	if (!paintNormals)
	{
		////bullet tails
		if (m_bulletsMeshIdx >= 0)
		{
			//#HARDCODE: set first texture which contains color info
			m_pDevice->SetTexture(0, m_sprActives.Textures[0]->pTex);
			//draw textured bullets (actives texture, just like the bullets)
			m_bufferedPainter.DrawMesh(m_bulletsMeshIdx, true);
		}

		while (node != &m_poolBullets.pListUsed)
		{
			//salvez locatia urmatoare ca sa pot avansa pe ea
			CLinkedPool<CBullet>::CLinkedPoolNode *nextnode = node->m_pNext;

			switch (node->m_data.type)
			{
				case K_LVL_BULLET_TRACER_AIMED_SHOT:
				case K_LVL_BULLET_SHOTGUN_SLUG:
				case K_LVL_BULLET_GOO:
				{
					D3DXVECTOR2 vdir = node->m_data.physPt->m_data.pos - node->m_data.physPt->m_data.pos_last;
					float ang = Math_GetVectorAngle(vdir);
					D3DXMatrixAffineTransformation2D(&matbullet, 1.0f, NULL, ang, &node->m_data.physPt->m_data.pos);
					m_pSprite->SetTransform(&matbullet);
					node->m_data.sprBullet.paint_firstModule(&m_sprActives);
				}
				break;

				case K_LVL_BULLET_FIRE_JET:
				case K_LVL_BULLET_TRACER_RECON:
				case K_LVL_BULLET_SHOTGUN_INCENDIARY:
				case K_LVL_BULLET_SHOTGUN:
				case K_LVL_BULLET_TRACER1:
				{
					//not painting because not necessary
					/*
					D3DXMatrixTranslation(&matbullet, node->m_data.physPt->m_data.pos.x, node->m_data.physPt->m_data.pos.y, 0.0f);
					m_pSprite->SetTransform(&matbullet);
					node->m_data.sprBullet.paint_firstModule(&m_sprActives);
					*/
				}
				break;
				case K_LVL_BULLET_MELEE:
				case K_LVL_BULLET_MELEE_SAW:
				{
#if defined(_DEBUG) || defined(DEBUG)
					float ang = Math_GetVectorAngle(node->m_data.physPt->m_data.speed);
					D3DXMatrixAffineTransformation2D(&matbullet, 1.0f, NULL, ang, &node->m_data.physPt->m_data.pos);
					m_pSprite->SetTransform(&matbullet);
					node->m_data.sprBullet.paint_firstModule(&m_sprActives);
#endif
				}
				break;
				case K_LVL_BULLET_CAM_BALL:
				case K_LVL_BULLET_SMOKE_GRENADE:
				{
					D3DXMatrixAffineTransformation2D(&matbullet, 1.0f, NULL, 0.0f, &node->m_data.physPt->m_data.pos);
					m_pSprite->SetTransform(&matbullet);
					node->m_data.sprBullet.paint_firstModule(&m_sprActives);
				}
				break;
				case K_LVL_BULLET_FLASHBANG:
				case K_LVL_BULLET_MOLOTOV:
				case K_LVL_BULLET_GRENADE:
				case K_LVL_BULLET_GRENADE_ROUND:
				{
					D3DXMatrixAffineTransformation2D(&matbullet, 1.0f, NULL, 0.0f, &node->m_data.physPt->m_data.pos);
					m_pSprite->SetTransform(&matbullet);
					node->m_data.sprBullet.paint_firstModule(&m_sprActives);
					//exclamation sign
					if (sin(fLocalTimeline * ((node->m_data.fLife > 0.5f) ? 10.0f : 30.0f)) > 0.0f)
					{
						CSprite tmpspr(ANM_ACTIVES_SPR_ICONS, 0.0f, -10.0f);
						tmpspr.paint_firstModule(&m_sprActives);
					}
				}
				break;
				case K_LVL_BULLET_BREACHING_CHARGE:
				{
					float fang = node->m_data.physPt->m_data.speed.x * fLocalTimeline;
					if (node->m_data.physPt->m_data.bContacting)
					{
						if (node->m_data.physPt->m_data.contactNormal.x < 0.0f)
							fang = -HALF_PI;
						else if(node->m_data.physPt->m_data.contactNormal.x > 0.0f)
							fang = HALF_PI;
					}
					D3DXMatrixAffineTransformation2D(&matbullet, 1.0f, NULL, fang, &node->m_data.physPt->m_data.pos);
					m_pSprite->SetTransform(&matbullet);
					node->m_data.sprBullet.paint_firstModule(&m_sprActives);
				}
				break;
			}

			//avansez pointer
			node = nextnode;
		}
		//reset transform
		m_pSprite->SetTransform(&g_matIdentity);
	}
	else  //paint bullets normals and self illumination
	{
		//bullet tails
		if (m_bulletsMeshIdx >= 0)
		{
			//#HARDCODE: set first texture which contains color info
			m_pDevice->SetTexture(0, m_sprActives.Textures[1]->pTex);

			m_bufferedPainter.DrawMesh(m_bulletsMeshIdx, true);
		}

		while (node != &m_poolBullets.pListUsed)
		{
			//salvez locatia urmatoare ca sa pot avansa pe ea
			CLinkedPool<CBullet>::CLinkedPoolNode *nextnode = node->m_pNext;

			switch (node->m_data.type)
			{
				case K_LVL_BULLET_TRACER_AIMED_SHOT:
				case K_LVL_BULLET_SHOTGUN_SLUG:
				case K_LVL_BULLET_GOO:
				{
					D3DXVECTOR2 vdir = node->m_data.physPt->m_data.pos - node->m_data.physPt->m_data.pos_last;
					float ang = Math_GetVectorAngle(vdir);
					D3DXMatrixAffineTransformation2D(&matbullet, 1.0f, NULL, ang, &node->m_data.physPt->m_data.pos);
					m_pSprite->SetTransform(&matbullet);
					node->m_data.sprBullet.paint_firstModule_texOverride(&m_sprActives, 1);
				}
				break;

				case K_LVL_BULLET_FIRE_JET:
				case K_LVL_BULLET_TRACER_RECON:
				case K_LVL_BULLET_SHOTGUN_INCENDIARY:
				case K_LVL_BULLET_SHOTGUN:
				case K_LVL_BULLET_TRACER1:
				{
					/*
					D3DXMatrixTranslation(&matbullet, node->m_data.physPt->m_data.pos.x, node->m_data.physPt->m_data.pos.y, 0.0f);
					m_pSprite->SetTransform(&matbullet);
					node->m_data.sprBullet.paint_firstModule_texOverride(&m_sprActives, 1);
					*/
				}
				break;
				case K_LVL_BULLET_MELEE:
				case K_LVL_BULLET_MELEE_SAW:
				{
#if defined(_DEBUG) || defined(DEBUG)

					float ang = Math_GetVectorAngle(node->m_data.physPt->m_data.speed);
					D3DXMatrixAffineTransformation2D(&matbullet, 1.0f, NULL, ang, &node->m_data.physPt->m_data.pos);
					m_pSprite->SetTransform(&matbullet);
					node->m_data.sprBullet.paint_firstModule_texOverride(&m_sprActives, 1);
#endif
				}
				break;
				case K_LVL_BULLET_CAM_BALL:
 				case K_LVL_BULLET_SMOKE_GRENADE:
				{
					D3DXMatrixAffineTransformation2D(&matbullet, 1.0f, NULL, 0.0f, &node->m_data.physPt->m_data.pos);
					m_pSprite->SetTransform(&matbullet);
					node->m_data.sprBullet.paint_firstModule_texOverride(&m_sprActives, 1);
				}
				break;
				case K_LVL_BULLET_FLASHBANG:
				case K_LVL_BULLET_MOLOTOV:
				case K_LVL_BULLET_GRENADE:
				case K_LVL_BULLET_GRENADE_ROUND:
				{
					D3DXMatrixAffineTransformation2D(&matbullet, 1.0f, NULL, 0.0f, &node->m_data.physPt->m_data.pos);
					m_pSprite->SetTransform(&matbullet);
					node->m_data.sprBullet.paint_firstModule_texOverride(&m_sprActives, 1);
					//exclamation sign
					if (sin(fLocalTimeline * ((node->m_data.fLife > 0.5f) ? 10.0f : 30.0f)) > 0.0f)
					{
						CSprite tmpspr(ANM_ACTIVES_SPR_ICONS, 0.0f, -10.0f);
						tmpspr.paint_firstModule_texOverride(&m_sprActives, 1);
					}
				}
				break;
				case K_LVL_BULLET_BREACHING_CHARGE:
				{
					float fang = node->m_data.physPt->m_data.speed.x * fLocalTimeline;
					if (node->m_data.physPt->m_data.bContacting)
					{
						if (node->m_data.physPt->m_data.contactNormal.x < 0.0f)
							fang = -HALF_PI;
						else if (node->m_data.physPt->m_data.contactNormal.x > 0.0f)
							fang = HALF_PI;
					}
					D3DXMatrixAffineTransformation2D(&matbullet, 1.0f, NULL, fang, &node->m_data.physPt->m_data.pos);
					m_pSprite->SetTransform(&matbullet);
					node->m_data.sprBullet.paint_firstModule_texOverride(&m_sprActives, 1);
				}
				break;
			}

			//avansez pointer
			node = nextnode;
		}
		//reset transform
		m_pSprite->SetTransform(&g_matIdentity);
	}
}

int CLevel::KillBulletsOfType(int nBulletType, UINT32 dwOwnerUID)
{
	int nRetCnt = 0;
	//verifica daca ai aruncat deja un charge
	CLinkedPool<CBullet>::CLinkedPoolNode *node = m_poolBullets.pListUsed.m_pNext;
	while (node != &m_poolBullets.pListUsed)
	{
		//salvez locatia urmatoare ca sa pot avansa pe ea
		CLinkedPool<CBullet>::CLinkedPoolNode *nextnode = node->m_pNext;
		CBullet* bullet = &node->m_data;

		if ( (bullet->type == nBulletType) && ((dwOwnerUID == 0) || (bullet->ownerUID == dwOwnerUID)) )
		{
			//destroy charge
			bullet->nFlags |= K_LVL_BULLET_FLAG_KILLITNOW;
			nRetCnt++;
		}
		node = nextnode;
	}
	
	return nRetCnt;
}

///--- DECALS ---
void CLevel::AddDecal(EDecalLayer nLayer, D3DXVECTOR2 pos, int animIdx, int frameIdx /*= 0*/, DWORD color /*= 0xffffffff*/, bool bIsAnimated /*= false*/)
{
	CDecal *ndec = new CDecal();

	ndec->layer = nLayer;
	ndec->sprite.Init(animIdx, (int)pos.x, (int)pos.y, frameIdx, color);
	RECTLTRB_F framerect = m_sprActives.GetAFrameBBox_real_LTRB(animIdx, frameIdx);
	ndec->aabb.Set(D3DXVECTOR2(framerect.left + pos.x, framerect.top + pos.y), D3DXVECTOR2(framerect.right + pos.x, framerect.bottom + pos.y));
	ndec->bAnimated = bIsAnimated;

	m_arrDecals.Add(ndec);
}


void CLevel::UpdateDecals(float dTime)
{
	//Update less often
	for (int kk = 0; kk < m_arrDecals.GetSize(); kk++)
	{
		if (m_arrDecals[kk]->bAnimated)
		{
			m_arrDecals[kk]->sprite.Update(&m_sprActives, dTime);
			//remove animation flag when anim ends
			if (m_arrDecals[kk]->sprite.animStatus == ANIM_STATUS_FRAMELOCK)
				m_arrDecals[kk]->bAnimated = false;
		}
	}
}

void CLevel::AddDecal_BloodSplat(D3DXVECTOR2 pos, bool bLarge, EActorClass eVictimClass)
{
	//blood splats are sorted by size (ascending)
	switch (eVictimClass)
	{
		case K_LVL_ACT_CLASS_ZOMBIE:
		{
			if (bLarge) //when dying
			{
				AddDecal(K_LVL_DECAL_LAYER_BACKWALLS, pos, ANM_ACTIVES_SPR_BLOOD_SPLAT_GREEN, 3 + randint(4), 0xffffffff);
			}
			else
			{
				AddDecal(K_LVL_DECAL_LAYER_BACKWALLS, pos, ANM_ACTIVES_SPR_BLOOD_SPLAT_GREEN, randint(3), 0xffffffff);
			}
		}
		break;

		default:
		{
			if (bLarge) //when dying
			{
				if (randompercent(70.0f))
					AddDecal(K_LVL_DECAL_LAYER_BACKWALLS, pos, ANM_ACTIVES_SPR_BLOOD_SPLAT, 3 + randint(4), 0xffffffff);
				else //add animated blood splats
					AddDecal(K_LVL_DECAL_LAYER_BACKWALLS, pos, ANM_ACTIVES_SPR_BLOODSPLAT1_ANIM + randint(3), 0, 0xffffffff, true);
			}
			else
			{
				AddDecal(K_LVL_DECAL_LAYER_BACKWALLS, pos, ANM_ACTIVES_SPR_BLOOD_SPLAT, randint(3), 0xffffffff);
			}
		}
		break;
	}
}

void CLevel::UpdatePhysicsPoints(float dTime)
{
	CLinkedPool<CPhysicsPoint2D>::CLinkedPoolNode *node = m_poolPhysPts.pListUsed.m_pNext;
	while (node != &m_poolPhysPts.pListUsed)
	{
		//salvez locatia urmatoare ca s apot avansa pe ea
		CLinkedPool<CPhysicsPoint2D>::CLinkedPoolNode *nextnode = node->m_pNext;
		//update
		CPhysicsPoint2D *point = &node->m_data;
		D3DXVECTOR2 nextpos = point->pos + point->speed * dTime + point->accel * dTime * dTime;
		//salvez in "vecForces" fortele care actioneaza asupra punctului (acceleratia gravitationala in cazul asta)
		D3DXVECTOR2 vecForces = point->accel;
		//check bounce
		bool bWasContacting = point->bContacting;
		//resetam contact data doar pe collision enabled ca sa nu "uite" datele de contact cand 
		if (point->eCollType != CPhysicsPoint2D::K_COLLTYPE_NONE)
		{
			point->bContacting = false;
			point->pContactShape = NULL;
		}
		point->bContactStarted = false;
		//save last pos
		point->pos_last = point->pos;
		//checkcollision (if not static)
		if (point->eCollType != CPhysicsPoint2D::K_COLLTYPE_NONE)
		{
			//#TODO: de luat in seama coliziunea dupa flagurile setate
			D3DXVECTOR2 collisionPoint, collisionNormal;

			CCollisionShape* colShape = NULL;
			if (point->eCollType == CPhysicsPoint2D::K_COLLTYPE_FAST)
			{
				//fast collision computes collisions with visible colshapes
				colShape = ColShape_Segment_Intersection_Arr(point->pos, nextpos, m_visibleList.logic_colShapes.m_pData, m_visibleList.logic_colShapes.Count(), &collisionPoint, &collisionNormal);
			}
			else if (point->eCollType == CPhysicsPoint2D::K_COLLTYPE_PRECISE)
			{
				//precise collision checks all collision shapes
				colShape = ColShape_Segment_Intersection_Arr(point->pos, nextpos, m_visibleList.logic_colShapesExtended.m_pData, m_visibleList.logic_colShapesExtended.Count(), &collisionPoint, &collisionNormal);
			}

			if (colShape != NULL)
			{
				point->pos = collisionPoint;

				point->bContacting = true;
				point->pContactShape = colShape;
				point->contactNormal = collisionNormal;
				point->contactPos = collisionPoint;
				//check bounce or first contact - mainly for sounds and particles
				if (bWasContacting == false)
				{
					//send bounce message
					point->bContactStarted = true;
				}
				else //daca si frame-ul trecut a fost in contact verific sa fie static
				{
					if ((fabs(point->speed.x) < 0.1f) && (fabs(point->speed.y) < 0.1f))
					{
						point->bIsStatic = true;
					}
					else
					{
						point->bIsStatic = false;
					}
				}

				if (point->bFlagPhysicsEnabled)
				{
					D3DXVECTOR2 Vn = collisionNormal * D3DXVec2Dot(&point->speed, &point->contactNormal);
					D3DXVECTOR2 Vt = point->speed - Vn;
					//apply bounce and friction
					Vn *= point->fBounceF;
					Vt -= Vt * point->fFrictionF * dTime;
					//calculez viteza finala
					point->speed = -Vn + Vt; //inversam Vn dupa coliziune
					//forces - reduc fortele care actioneaza pe punct doar la componenta tangentiala (anulez componenta normala)
					D3DXVec2Normalize(&Vt, &Vt);
					vecForces = Vt * D3DXVec2Dot(&vecForces, &Vt);
				}
			}
		}

		//integrator
		point->speed += vecForces * dTime;
		//pe versiunea cu fizica modific pozitia la sfarsit, dupa rezolvarea sistemului
		if (point->bFlagPhysicsEnabled)
		{
			point->pos += point->speed * dTime;
			//reglaj pozitie dupa coliziune ca sa fie in punctul de contact (putin deasupra lui ca sa faca urmatoarele coliziuni bine)
			if (point->bContactStarted)
			{
				//deplasez cu normala * dTime ca sa nu vibreze pe framerate foarte mare
				point->pos = point->contactPos + (point->contactNormal * dTime);
			}
		}
		else //pe versiunea fara fizica nu suprascriu pos
		{
			if (!point->bContacting)
			{
				point->pos = nextpos;
			}
			//nu am else pentru ca daca e contacting a fost setata corect pozitia pe punctul de contact
		}

		//daca iese din zona de joc	il seteaza ca static
		if (!PointInRect(point->pos, m_levelAABB))
		{
			point->bIsDead = true;
		}

		//avansez pointer
		node = nextnode;
	}
}


///--- LEVEL PROPS MANAGER ---
void CLevel::AddProp(EPropType type, D3DXVECTOR2 pos, D3DXVECTOR2 * speed, D3DXVECTOR2 * accel, int nSubType /*= 0*/)
{
	bool bGoreEnabled = UTGetAppClass().m_Settings.bGoreEnabled;
	if ((!bGoreEnabled) && (type == K_LVL_PROP_MEAT))
		return;
	//pre-checks
	if (type == K_LVL_PROP_MEAT)
	{
		//don't spawn meat if inside collisions
		if (GetCollisionShapeAt(pos, K_LVL_COLL_TYPE_SOLID) != null)
			return;
	}

	CLinkedPool<CLevelProp>::CLinkedPoolNode *node = m_poolProps.HireNode();
	//set 
	if (node != null)
	{
		node->m_data.Reset();
		//add simulation container
		node->m_data.physPt = m_poolPhysPts.HireNode();
		if (node->m_data.physPt == NULL)
		{
			ErrorBox(K_ERR_WARNING, L"AddProp:We need more physics points!");
			m_poolProps.DismissNode(node);
			return;
		}
		//reset
		node->m_data.physPt->m_data.Init();

		node->m_data.type = type;
		node->m_data.nSubType = nSubType;

		switch (type)
		{
			default:
			{
				ErrorBox(K_ERR_WARNING, L"AddProp::Unknown prop type!");
			}
			break;
			
			case K_LVL_PROP_SHELL:
			{
				//check subtype validity
				if (nSubType * 4 + 3 >= m_sprActives.GetAFramesCnt(ANM_ACTIVES_SPR_SHELLS))
				{
					ErrorBox(K_ERR_WARNING, L"Invalid weapon nDropShellFrame param! resetting to 0");
					node->m_data.nSubType = 0;
				}
				//all shells are in the same animation, 4 frames each
				node->m_data.spr.Init(ANM_ACTIVES_SPR_SHELLS, 0.0f, 0.0f, node->m_data.nSubType * 4 + randint(4));
				node->m_data.bVar1 = false; //face sunet o singura data la coliziune apoi seteaza bVar1 pe true
				//physics
				node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_FAST;
				node->m_data.physPt->m_data.bFlagPhysicsEnabled = true;

				node->m_data.physPt->m_data.pos = pos;
				if (speed != null)
					node->m_data.physPt->m_data.speed = *speed;
				if (accel != null)
					node->m_data.physPt->m_data.accel = *accel;
			}
			break;
			
			case K_LVL_PROP_FIRE_SOURCE:
			{
				//punem aici tipul de lumina pe care il face
				node->m_data.spr.Init(ANM_ACTIVES_SPR_MELTING_LAVA, 0.0f, 0.0f, 0);
				node->m_data.fTimer = 2.0f + m_rand.RandFloat(1.0f);

				node->m_data.bMakesLight = true;
				node->m_data.fLightDuration = node->m_data.fTimer;
				node->m_data.fLightFadeOut = 0.3f;
				node->m_data.sprLight.Init(ANM_LIGHTS_SPR_POINT_SM1, 0.0f, 0.0f, 0, 0xfffdb727);
				//physics
				node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_PRECISE;
				node->m_data.physPt->m_data.bFlagPhysicsEnabled = true;

				node->m_data.physPt->m_data.pos = pos;
				if (speed != null)
					node->m_data.physPt->m_data.speed = *speed;
				if (accel != null)
					node->m_data.physPt->m_data.accel = *accel;
				//no bounce
				node->m_data.physPt->m_data.fBounceF = 0.0f;
				node->m_data.physPt->m_data.fFrictionF = 30.0f;
			}
			break;

			case K_LVL_PROP_MEAT:
			{
				//0-red meat, 1-green meat
				node->m_data.nSubType = nSubType;
				if(nSubType == 0)
					node->m_data.spr.Init(ANM_ACTIVES_SPR_MEAT, 0.0f, 0.0f, randint(6));
				else
					node->m_data.spr.Init(ANM_ACTIVES_SPR_MEAT_GREEN, 0.0f, 0.0f, randint(6));
				//physics
				node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_PRECISE;
				node->m_data.physPt->m_data.bFlagPhysicsEnabled = true;

				node->m_data.physPt->m_data.pos = pos;
				if (speed != null)
					node->m_data.physPt->m_data.speed = *speed;
				if (accel != null)
					node->m_data.physPt->m_data.accel = *accel;
				//no bounce
				node->m_data.physPt->m_data.fBounceF = 0.1f;
				node->m_data.physPt->m_data.fFrictionF = 30.0f;
			}
			break;

			case K_LVL_PROP_GOO:
			{
				node->m_data.spr.Init(ANM_ACTIVES_SPR_GOO_SM, 0.0f, 0.0f, randint(4));
				//physics
				node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_PRECISE;
				node->m_data.physPt->m_data.bFlagPhysicsEnabled = true;

				node->m_data.physPt->m_data.pos = pos;
				if (speed != null)
					node->m_data.physPt->m_data.speed = *speed;
				if (accel != null)
					node->m_data.physPt->m_data.accel = *accel;
				//no bounce
				node->m_data.physPt->m_data.fBounceF = 0.1f;
				node->m_data.physPt->m_data.fFrictionF = 30.0f;
			}
			break;


			case K_LVL_PROP_SHRAPNEL_SMOKING:
			{
				//grafica shrapnel
				node->m_data.spr.Init(ANM_ACTIVES_SPR_SHRAPNEL, 0.0f, 0.0f, 0);
				//grafica shrapnel luminos
				node->m_data.spr2.Init(ANM_ACTIVES_SPR_BULLETS_FIRE, 0.0f, 0.0f, 0);
				node->m_data.fTimer = 0.3f + randfloat(0.3f);
				//light
				node->m_data.bMakesLight = true;
				node->m_data.fLightDuration = node->m_data.fTimer;
				node->m_data.fLightFadeOut = 0.3f;
				node->m_data.sprLight.Init(ANM_LIGHTS_SPR_POINT_SM1, 0.0f, 0.0f, 0, 0xfffdb727);

				//physics
				node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_PRECISE;
				node->m_data.physPt->m_data.bFlagPhysicsEnabled = true;

				node->m_data.physPt->m_data.pos = pos;
				if (speed != null)
					node->m_data.physPt->m_data.speed = *speed;
				if (accel != null)
					node->m_data.physPt->m_data.accel = *accel;
				//no bounce
				node->m_data.physPt->m_data.fBounceF = 0.4f;
				node->m_data.physPt->m_data.fFrictionF = 10.0f;
			}
			break;
			case K_LVL_PROP_LIGHT:
				ErrorBox(K_ERR_WARNING, L"Use AddProp_Light() for this type!");
				break;
			case K_LVL_PROP_EXPLOSION:
				ErrorBox(K_ERR_WARNING, L"Use AddProp_Explo() for this type!");
				break;
		}
	}
}

void CLevel::AddProp_Light(D3DXVECTOR2 pos, int nLightAnimIdx, float fDuration, float fFadeTime, DWORD color, float fScale)
{
	CLinkedPool<CLevelProp>::CLinkedPoolNode *node = m_poolProps.HireNode();
	//set 
	if (node != null)
	{
		node->m_data.Reset();
		//add simulation container
		node->m_data.physPt = m_poolPhysPts.HireNode();
		if (node->m_data.physPt == NULL)
		{
			ErrorBox(K_ERR_WARNING, L"AddProp_Light:We need more physics points!");
			m_poolProps.DismissNode(node);
			return;
		}
		//reset
		node->m_data.physPt->m_data.Init();

		node->m_data.type = K_LVL_PROP_LIGHT;

		node->m_data.sprLight.Init(nLightAnimIdx, 0.0f, 0.0f, 0, color);
		node->m_data.fLightScaling = fScale * K_LVL_LIGHTRENDER_BSX_SCALING;
		node->m_data.bMakesLight = true;
		node->m_data.fLightDuration = fDuration;
		node->m_data.fLightFadeOut = fFadeTime;

		node->m_data.fTimer = 0.0f;
		//physics
		node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_NONE;
		node->m_data.physPt->m_data.bFlagPhysicsEnabled = false;
		node->m_data.physPt->m_data.bFlagRotationEnabled = false;

		node->m_data.physPt->m_data.pos = pos;
		node->m_data.physPt->m_data.speed = D3DXVECTOR2(0.0f, 0.0f);
		node->m_data.physPt->m_data.accel = D3DXVECTOR2(0.0f, 0.0f);
	}
}

//#TODO: de generalizat total exploziile la final cand stiu cum vor arata si cate tipuri vor fi. Sa am in xml si animatie si scalare si ce fel de particule arunca etc
void CLevel::AddProp_Explo(UINT32 exploNameHash, D3DXVECTOR2 pos, UINT32 dwOwnerUID, int exploOwnerClass, D3DXVECTOR2 vExploDir, CAABB* exploAABB)
{
	CExplosionTemplate* explotemplate = GetTemplateExplosion(exploNameHash);
	if (explotemplate == NULL)
		return;

	CLinkedPool<CLevelProp>::CLinkedPoolNode *node = m_poolProps.HireNode();
	//set 
	if (node != null)
	{
		node->m_data.Reset();
		//add simulation container
		node->m_data.physPt = m_poolPhysPts.HireNode();
		if (node->m_data.physPt == NULL)
		{
			ErrorBox(K_ERR_WARNING, L"AddProp_Explo:Need more physics points!");
			m_poolProps.DismissNode(node);
			return;
		}
		//reset
		node->m_data.physPt->m_data.Init();

		node->m_data.type = K_LVL_PROP_EXPLOSION;
		//physics
		node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_NONE;
		node->m_data.physPt->m_data.bFlagPhysicsEnabled = false;
		node->m_data.physPt->m_data.bFlagRotationEnabled = false;

		node->m_data.physPt->m_data.pos = pos;
		node->m_data.physPt->m_data.speed = D3DXVECTOR2(0.0f, 0.0f);
		node->m_data.physPt->m_data.accel = D3DXVECTOR2(0.0f, 0.0f);

		//default
		float fMaxDamage = explotemplate->fDamage;
		float fMaxStun = explotemplate->fStunDuration;
		float fDamageRadius = explotemplate->fDamageRadius;
		float fStunRadius = explotemplate->fStunRadius;
		float fMaxImpulse = explotemplate->fMaxImpulse;

		//add sound event
		if(explotemplate->fSoundRadius > 0.0f)
			AddAIEvent(K_LVL_AI_EVENT_SOUND_THREAT, 0, exploOwnerClass, pos, explotemplate->fSoundRadius, 1.0f);

		if (exploAABB == null)
		{
			//shrapnel
			for (int ll = 0; ll < explotemplate->nShrapnelCnt; ll++)
			{
				float fdx = m_rand.RandFloatSgn(150.0f);
				float fdy = -100.0f - m_rand.RandFloat(150.0f);
				AddProp(K_LVL_PROP_SHRAPNEL_SMOKING, pos, &D3DXVECTOR2(fdx, fdy), &g_vecGravity);
			}
			//napalm
			for (int ll = 0; ll < explotemplate->nNapalmCnt; ll++)
			{
				float fdx = m_rand.RandFloatSgn(60.0f);
				float fdy = -100.0f - m_rand.RandFloat(120.0f);
				AddProp(K_LVL_PROP_FIRE_SOURCE, pos, &D3DXVECTOR2(fdx, fdy), &g_vecGravity);
			}
		}
		else
		{
			//shrapnel
			for (int ll = 0; ll < explotemplate->nShrapnelCnt; ll++)
			{
				float fdx = m_rand.RandFloatSgn(150.0f);
				float fdy = -100.0f - m_rand.RandFloat(150.0f);
				AddProp(K_LVL_PROP_SHRAPNEL_SMOKING, pos + m_rand.RandD3DXVECTOR2sgn(exploAABB->vHalfSize.x, exploAABB->vHalfSize.y), 
					&D3DXVECTOR2(fdx, fdy), &g_vecGravity);
			}
			//napalm
			for (int ll = 0; ll < explotemplate->nNapalmCnt; ll++)
			{
				float fdx = m_rand.RandFloatSgn(60.0f);
				float fdy = -100.0f - m_rand.RandFloat(120.0f);
				AddProp(K_LVL_PROP_FIRE_SOURCE, pos + m_rand.RandD3DXVECTOR2sgn(exploAABB->vHalfSize.x, exploAABB->vHalfSize.y), 
					&D3DXVECTOR2(fdx, fdy), &g_vecGravity);
			}
		}

		//explo direction
		float fExploAng = Math_GetVectorAngle(vExploDir);
		//#TODO: explo-interactAI e o proprietate ce va fi exportata (interactioneaza cu AI-uri care se activeaza la explozii?)
		bool bInteractAI = false;

		if (explotemplate->name.textHash == hash_EXPLO_INVISIBLE_EXPLODING_ZOMBIE)
		{
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_GREEN_FATZOMBIE, true, 0, &D3DXVECTOR2(pos.x, pos.y + 6.0f), NULL, NULL, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_BARREL)
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_ROUND_XL, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;

			node->m_data.fTimer = 0.4f;

			//prop - light
			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 1.0f, 0.2f, 0xffFDB727, 2.0f);
			//add visually stunning stuff
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_ROUND_XL, true, 0, &pos, NULL, &D3DXVECTOR2(0.0f, -20.0f), 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			//add ring
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLOWS, false, 1, &pos, NULL, NULL, 0.15f, 1.0f, 16.0f, 0.0f, 0.0f, 0.05f, 0.1f, 0x55fdb727, K_PART_LAYER_FRONT_LIGHT);
			g_particlesMgr.GenerateSmokePuff(pos, 20.0f, K_PART_LAYER_RT_FRONT_NRM);

			m_camLevel.ShakeScreen(4.0f, 8.0f, &pos);
			//SND_PLAY_POSITIONAL_RAND2(SNDIDX_EXPLOSION_BARREL_01, SNDIDX_EXPLOSION_BARREL_02, pos);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_LARGE)
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_ROUND_LG1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;

			node->m_data.fTimer = 0.4f;

			//prop - light
			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.8f, 0.2f, 0xffFDB727, 1.6f);
			//add visually stunning stuff
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_ROUND_LG1, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			//add ring
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLOWS, false, 1, &pos, NULL, NULL, 0.15f, 1.0f, 16.0f, 0.0f, 0.0f, 0.05f, 0.1f, 0x55fdb727, K_PART_LAYER_FRONT_LIGHT);
			g_particlesMgr.GenerateSmokePuff(pos, 20.0f, K_PART_LAYER_RT_FRONT_NRM);

			m_camLevel.ShakeScreen(6.0f, 10.0f, &pos);
			//SND_PLAY_POSITIONAL_RAND2(SNDIDX_EXPLOSION_FRAG_GRENADE_01, SNDIDX_EXPLOSION_FRAG_GRENADE_02, pos);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_LARGE_XL)
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_ROUND_LG2, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;

			node->m_data.fTimer = 0.4f;

			//prop - light
			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.8f, 0.2f, 0xffFDB727, 1.6f);
			//add visually stunning stuff
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_ROUND_LG2, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			//add ring
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLOWS, false, 1, &pos, NULL, NULL, 0.15f, 1.0f, 16.0f, 0.0f, 0.0f, 0.05f, 0.1f, 0x55fdb727, K_PART_LAYER_FRONT_LIGHT);
			g_particlesMgr.GenerateSmokePuff(pos, 20.0f, K_PART_LAYER_RT_FRONT_NRM);

			m_camLevel.ShakeScreen(4.0f, 8.0f, &pos);
			//SND_PLAY_POSITIONAL_RAND2(SNDIDX_EXPLOSION_FRAG_GRENADE_01, SNDIDX_EXPLOSION_FRAG_GRENADE_02, pos);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_MOLOTOV)
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_ROUND_SM1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;
			node->m_data.fTimer = 0.4f;

			//prop - light
			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.8f, 0.2f, 0xffFDB727, 1.6f);
			//add visually stunning stuff
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_ATOMIC1, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			//add ring
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLOWS, false, 1, &pos, NULL, NULL, 0.15f, 1.0f, 16.0f, 0.0f, 0.0f, 0.05f, 0.1f, 0x55fdb727, K_PART_LAYER_FRONT_LIGHT);
			g_particlesMgr.GenerateSmokePuff(pos, 20.0f, K_PART_LAYER_RT_FRONT_NRM);

			m_camLevel.ShakeScreen(4.0f, 8.0f, &pos);
			//SND_PLAY_POSITIONAL_RAND2(SNDIDX_MOLOTOV_EXPLOSION_01, SNDIDX_MOLOTOV_EXPLOSION_02, pos);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_GREEN_GOO)
		{
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_GREEN_ROUND, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			for (int ll = 0; ll < 4; ll++)															  
			{
				AddProp(K_LVL_PROP_GOO, pos, &D3DXVECTOR2(m_rand.RandFloatSgn(50.0f), -100.0f - m_rand.RandFloat(60.0f)), &g_vecGravity);
			}
		}
		else if (explotemplate->name.textHash == hash_EXPLO_GREEN_GOO_GROUND)
		{
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_GREEN_GROUND, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			for (int ll = 0; ll < 4; ll++)
			{
				AddProp(K_LVL_PROP_GOO, pos, &D3DXVECTOR2(m_rand.RandFloatSgn(50.0f), -130.0f - m_rand.RandFloat(60.0f)), &g_vecGravity);
			}
		}
		else if (explotemplate->name.textHash == hash_EXPLO_BURN_DOT)
		{
			bInteractAI = FALSE;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_FIRECRACKER1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;
			node->m_data.fTimer = 0.4f;

			//add visually stunning stuff
			if(randompercent(50.0f))
				g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRECRACKER1, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			else
				g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRECRACKER2, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_FLAME_JET)
		{
			bInteractAI = FALSE;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_FIRECRACKER1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;
			node->m_data.fTimer = 0.4f;

			//add visually stunning stuff
			if (randompercent(50.0f))
				g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRECRACKER1, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			else
				g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRECRACKER2, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_CHARGE)
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_CHARGE1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;

			node->m_data.fTimer = 0.2f;

			//prop - light
			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.6f, 0.2f, 0xffFDB727, 1.6f);
			//add visually stunning stuff
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_CHARGE1, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			//add ring
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLOWS, false, 1, &pos, NULL, NULL, 0.15f, 1.0f, 16.0f, 0.0f, 0.0f, 0.05f, 0.1f, 0x55fdb727, K_PART_LAYER_FRONT_LIGHT);
			g_particlesMgr.GenerateSmokePuff(pos, 20.0f, K_PART_LAYER_RT_FRONT_NRM);

			m_camLevel.ShakeScreen(4.0f, 8.0f, &pos);
			//SND_PLAY_POSITIONAL_RAND2(SNDIDX_EXPLOSION_BREACHING_CHARGE_01, SNDIDX_EXPLOSION_BREACHING_CHARGE_02, pos);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_CHARGE_INVISIBLE)
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_CHARGE1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;

			node->m_data.fTimer = 0.2f;
		}
		else if (explotemplate->name.textHash == hash_EXPLO_STUN_INVISIBLE)
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_CHARGE1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;

			node->m_data.fTimer = 0.2f;
		}
		else if ((explotemplate->name.textHash == hash_EXPLO_GRENADE) || (explotemplate->name.textHash == hash_EXPLO_BLOWUP_VEST))
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_ROUND_SM1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.5f;

			node->m_data.fTimer = 0.2f;

			//prop - light
			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.6f, 0.2f, 0xffFDB727, 1.6f);
			//add visually stunning stuff
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_ROUND_SM1, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			//add ring
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLOWS, false, 1, &pos, NULL, NULL, 0.15f, 1.0f, 16.0f, 0.0f, 0.0f, 0.05f, 0.1f, 0x55fdb727, K_PART_LAYER_FRONT_LIGHT);
			g_particlesMgr.GenerateSmokePuff(pos, 20.0f, K_PART_LAYER_RT_FRONT_NRM);

			m_camLevel.ShakeScreen(4.0f, 8.0f, &pos);
//			SND_PLAY_POSITIONAL_RAND2(SNDIDX_EXPLOSION_FRAG_GRENADE_01, SNDIDX_EXPLOSION_FRAG_GRENADE_02, pos);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_GRENADE_GROUND)
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_GROUND1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.5f;

			node->m_data.fTimer = 0.2f;

			//prop - light
			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.6f, 0.2f, 0xffFDB727, 1.6f);
			//add visually stunning stuff
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_GROUND1, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			//add ring
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLOWS, false, 1, &pos, NULL, NULL, 0.15f, 1.0f, 16.0f, 0.0f, 0.0f, 0.05f, 0.1f, 0x55fdb727, K_PART_LAYER_FRONT_LIGHT);
			g_particlesMgr.GenerateSmokePuff(pos, 20.0f, K_PART_LAYER_RT_FRONT_NRM);

			m_camLevel.ShakeScreen(4.0f, 8.0f, &pos);
//			SND_PLAY_POSITIONAL_RAND2(SNDIDX_EXPLOSION_FRAG_GRENADE_01, SNDIDX_EXPLOSION_FRAG_GRENADE_02, pos);
		}
		else if ((explotemplate->name.textHash == hash_EXPLO_FLASHBANG) || (explotemplate->name.textHash == hash_EXPLO_SHIELD_FLASH))
		{
			node->m_data.spr.Init(ANM_PARTICLES_SPR_FLASH_AIR, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;
			node->m_data.fTimer = 0.2f;
			//prop - light
			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 1.0f, 0.2f, 0xffffffff, 1.5f);
			//add visually stunning stuff
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FLASH_AIR, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);

			m_camLevel.ShakeScreen(3.0f, 4.0f, &pos);
			m_screenVignette.Init(0.2f, 0xffffffff, 0.0f, 0.2f, 0.8f);

//			SND_PLAY_POSITIONAL_RAND2(SNDIDX_EXPLOSION_FLASHBANG_01, SNDIDX_EXPLOSION_FLASHBANG_02, pos);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_FAKE_SPY_CAMERA)
		{
			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.6f, 0.2f, 0xff00d0ff, 2.0f);
			m_screenVignette.Init(0.2f, 0xff00d0ff, 0.0f, 0.2f, 0.6f);
		}

		//pointer to player that spawned the explosion, or null if it wasn't a player
		CActor* pPlayer = GetPlayerByUID(dwOwnerUID);
		//perks below
		if ((pPlayer != null) && (pPlayer->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER))
		{
			//#PERK: RESCUE PLAN - highlight hostages too
			if (g_playerSelScr.IsPerkEnabled(pPlayer->nPlayerOrdinal, &shPerk_RESCUE_PLAN))
			{
				if (explotemplate->name.textHash == hash_EXPLO_FAKE_CAM_BALL)
					AddProp_Explo(hash_EXPLO_FAKE_CAM_BALL_HOSTAGE, pos, pPlayer->GetUID(), K_LVL_ACT_CLASS_PLAYER);
				else if (explotemplate->name.textHash == hash_EXPLO_FAKE_SPY_CAMERA)
					AddProp_Explo(hash_EXPLO_FAKE_SPY_CAMERA_HOSATAGE, pos, pPlayer->GetUID(), K_LVL_ACT_CLASS_PLAYER);
				else if (explotemplate->name.textHash == hash_EXPLO_FAKE_CAM_BALL_1SEC)
					AddProp_Explo(hash_EXPLO_FAKE_CAM_BALL_HOSTAGE_1SEC, pos, pPlayer->GetUID(), K_LVL_ACT_CLASS_PLAYER);
			}

			//#PERK: BIG BANGER - 50% more range for explo
			if (g_playerSelScr.IsPerkEnabled(pPlayer->nPlayerOrdinal, &shPerk_BIG_BANGER))
			{
				if ((explotemplate->name.textHash == hash_EXPLO_GRENADE) || (explotemplate->name.textHash == hash_EXPLO_GRENADE_GROUND))
				{
					fStunRadius *= 1.5f;
					fDamageRadius *= 1.5f;
				}
				if ((explotemplate->name.textHash == hash_EXPLO_CHARGE) || (explotemplate->name.textHash == hash_EXPLO_CHARGE_INVISIBLE))
				{
					fStunRadius *= 2.0f;
					fDamageRadius *= 2.0f;
				}
			}
		}

		//#IMPORTANT #TODO: should optimize in order to minimize the usage of UnobstructedLineOfSight
		//stun enemy and damage over time
		if ((fMaxStun > 0.0f) || (explotemplate->cDoT.eType != CDamageOverTime::K_LVL_DoT_NONE))
		{
			//find all actors and damage them (linearly)
			for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
			{
				CActor* act = m_arrActors[kk];
				//sar actorii ascunsi
				if ((act->bHidden) || (act->fLife < 0.0f) || (act->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET))
					continue;
				//never stun the hostages
				if ((act->templateActor.actorClass == K_LVL_ACT_CLASS_HOSTAGE) && (fMaxStun > 0.0f))
					continue;
				//distanta la inamic
				D3DXVECTOR2 vDir = act->posHeart - pos;
				float fDist = D3DXVec2Length(&vDir);

				bool bDirectLine = IsLineOfSight(act->posHeart, pos);
				//daca am damage over time il setez pe actor
				if ((bDirectLine) && (explotemplate->cDoT.eType != CDamageOverTime::K_LVL_DoT_NONE) && (fDist < explotemplate->fDoTRadius))
				{
					//momentan nu pune DoT in functie de distanta ci pune uniform la toti din raza
					SetActorDoT(act, explotemplate->cDoT.eType, explotemplate->cDoT.fDuration, explotemplate->cDoT.fDamagePerSec, explotemplate->cDoT.eExcludedActClass, explotemplate->cDoT.eFilteredActClass, dwOwnerUID);
				}

				//evit friendly stun
				if (act->templateActor.actorClass != K_LVL_ACT_CLASS_HUMAN)
					continue;
				//daca e prea departe nu il ia in seama
				if (fDist > fStunRadius)
					continue;
				//daca stun este directional si nu se potriveste directia
				if ((vExploDir.x != 0.0f) && (SIGN(vExploDir.x) != SIGN(vDir.x)))
					continue;
				//daca nu e linie directa nu loveste
				if (!bDirectLine)
					continue;
				//daca il vede ii da stun
				if ((act->fStunTimer < fMaxStun) && (fMaxStun > 0.0f))
				{
					SetActorStun(act, fMaxStun);
					//let him know he got stunned
					AddAIEvent(K_LVL_AI_EVENT_GOT_HIT, 0, exploOwnerClass, pos, fStunRadius, fMaxStun + 0.5f, act->GetUID());
				}
			}
		}

		//do some damage
		if ((fMaxDamage > 0.0f) && (fDamageRadius > 0.0f))
		{
			int nBombFrags = 0;
			//find all actors and damage them (linearly)
			for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
			{
				CActor* act = m_arrActors[kk];
				//sar actorii ascunsi
				if ((act->bHidden) || (act->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET))
					continue;
				//ignores specified classes
				if (act->templateActor.actorClass == explotemplate->eIgnoreActorClass)
					continue;
				//distanta la inamic
				D3DXVECTOR2 vDir = act->posHeart - pos;
				float fDist = D3DXVec2Length(&vDir);
				//daca e prea departe nu il ia in seama
				if (fDist > fDamageRadius)
					continue;
				//daca nu e linie directa nu loveste
				if (!IsLineOfSight(act->posHeart, pos))
					continue;
				//loveste liniar
				float fPercent = 1.0f - (fDist / fDamageRadius);
				CLAMP(fPercent, 0.0f, 1.0f);
				//under cover damage
				if (act->pCover != NULL)
				{
					D3DXVECTOR2 vIntPos;
					bool bHitCover = AABB_Segment_Intersection(pos, act->posHeart, act->pCover->bbox, &vIntPos);
					//atenueaza doar daca cover e intre om si explo
					if((bHitCover) && (D3DXVec2Length(&(vIntPos - act->posHeart)) > 2.0f))
						fPercent = fPercent * (1.0f - K_LVL_COVER_DAMAGE_ABSORBTION);
				}
				//add momentum
				D3DXVec2Normalize(&vDir, &vDir);
				vDir *= fPercent * fMaxImpulse;
				//#HACK: ca sa nu mai arunce cadavrele in sus
				if (vDir.y < 0.0f)
					vDir.y = 0.0f;
				
				CBulletHitReturnData retdata;
				retdata = HitActor(act, fPercent * fMaxDamage, dwOwnerUID, K_LVL_ACT_CLASS_EXPLOSION, &vDir, K_LVL_BULLET_FLAG_CAN_SPLAT, explotemplate->nArmorPiercingRating);
				//count only enemies
				if ((retdata.bKilledTarget) && (act->templateActor.actorClass >= K_LVL_ACT_CLASS_HUMAN))
					nBombFrags++;

				//#ACHIEVEMENTS: darwin award - player died from his own explosive
				if ((act->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER) && (!IsNetworkPlayer(act)) && (act->fLife <= 0.0f) && (dwOwnerUID == act->UID) && 
					((explotemplate->name.textHash == hash_EXPLO_GRENADE_GROUND) || (explotemplate->name.textHash == hash_EXPLO_GRENADE) || 
					 (explotemplate->name.textHash == hash_EXPLO_CHARGE) || (explotemplate->name.textHash == hash_EXPLO_CHARGE_INVISIBLE)) )
				{
					UTGetAchievementManager().UnlockAchievement(ACH_DARWIN_AWARD);
				}

			}

			//#ACHIEVEMENTS: explosion achievements
			if ((nBombFrags >= 3) && (pPlayer != null) && (!IsNetworkPlayer(pPlayer)))
			{
				//breaching charge behind the door
				if (explotemplate->name.textHash == hash_EXPLO_CHARGE_INVISIBLE)
				{
					UTGetAchievementManager().UnlockAchievement(ACH_GOOD_BREACH);
				}
				if (explotemplate->name.textHash == hash_EXPLO_BARREL)
				{
					UTGetAchievementManager().UnlockAchievement(ACH_HEAT_UP_THE_NIGHT);
				}
			}

			///--- check doors and windows breaking ---
			if ((fMaxDamage > 0.0f) && (fDamageRadius > 0.0f))
			{
				for (int kk = 0; kk < m_visibleList.logic_colShapesSpecial.Count(); kk++)
				{
					CCollisionShape* shape = m_visibleList.logic_colShapesSpecial.m_pData[kk];
					//breaks doors?
					if ((explotemplate->fDamageObjectsMultiplier > 0.0f) && (shape->AIstate == K_AI_STATE_COLL_BREAKABLE_DOOR))
					{
						if (shape->varAIparams.GetVariantByName(L"b_reinforced")->m_asINT32 != 0)
							continue;

						//loveste liniar
						D3DXVECTOR2 vDist = (shape->bbox.vCenter - pos);
						float fDist = D3DXVec2Length(&vDist);
						if (explotemplate->fDamageObjectsMultiplier <= 0.0f)
							continue;
						float fPercent = 1.0f - (fDist / (fDamageRadius * explotemplate->fDamageObjectsMultiplier));
						//too soft
						if (fPercent <= 0.0f)
							continue;
						//not straight line (can't check with center or it will fail because of the actual bbox)
						D3DXVECTOR2 vCheckPt = pos;
						vCheckPt.x -= (shape->bbox.vHalfSize.x + 2.0f) * SIGN(vDist.x);
						if (!IsLineOfSight(pos, vCheckPt))
							continue;
						//damage door
						shape->AIfvar1 -= (fPercent * fMaxDamage) * explotemplate->fDamageObjectsMultiplier;
						//was hit
						shape->AIvarBool1 = true;
						//save door explo direction
						shape->varAIparams.SetNamedVarFloat(L"fForceDirX", SIGN(vDist.x));
						shape->varAIparams.SetNamedVarINT32(L"bExploded", 1);
					}

					//windows?
					if (shape->AIstate == K_AI_STATE_COLL_BREAKABLE_WINDOW)
					{
						//linear distance hit
						float fDist = D3DXVec2Length(&(shape->bbox.vCenter - pos));
						float fPercent = 1.0f - (fDist / fDamageRadius);
						//subtract life
						if (fPercent > 0.0f)
						{
							shape->AIfvar1 -= fPercent * fMaxDamage;
							//door destroyed - save direction applied by explo
							if (shape->AIfvar1 <= 0.0f)
							{
								shape->varAIparams.SetNamedVarFloat(L"fForceDirX", 1000.0f * SIGN(shape->bbox.vCenter.x - pos.x));
							}
						}
					}
				}
			}
			//check grenade interaction AIs
			if ((fMaxDamage > 0.0f) && (bInteractAI) && (fDamageRadius > 0.0f))
			{
				for (int kk = 0; kk < m_visibleList.logic_actives_closeby[K_LVL_LAYER_FRONT].Count(); kk++)
				{
					CActive * activ = m_visibleList.logic_actives_closeby[K_LVL_LAYER_FRONT].m_pData[kk];
					if (activ->AIstate == K_AI_STATE_ACTIVE_SWINGING_FRONTOBJ)
					{
						//daca am activ swinging si e in raza grenadei
						D3DXVECTOR2 vDir = activ->pos - pos;
						float fDist = D3DXVec2Length(&vDir);
						//daca e prea departe nu il ia in seama
						if (fDist > fDamageRadius * 2.0f)
							continue;
						//direct line of sight
						if (!IsLineOfSight(activ->pos, pos))
							continue;

						//setam balans
						float maxperc = 1.0f - (fDist / (fDamageRadius * 2.0f));
						//viteza unghiulara
						activ->AIfvar1 = -SIGN(vDir.x) * 8.0f * maxperc;
					}
				}
			}
		}

		//damage over time
	}
}



void CLevel::UpdateProps(float dTime)
{
	RECTXYWH_F camrect = m_camLevel.GetCamWorldAABB();
	RECTXYWH_F camrect_larger = camrect;
	camrect_larger.Inflate(2.0f * K_TILE_SIZE);

	CLinkedPool<CLevelProp>::CLinkedPoolNode *node = m_poolProps.pListUsed.m_pNext;
	while (node != &m_poolProps.pListUsed)
	{
		//salvez locatia urmatoare ca s apot avansa pe ea
		CLinkedPool<CLevelProp>::CLinkedPoolNode *nextnode = node->m_pNext;
		CLevelProp* prop = &node->m_data;

		bool killprop = false;
		//daca iese din zona de joc
		if (prop->physPt->m_data.bIsDead)
			killprop = true;

		//generic updates
		prop->fLightTimer += dTime;

		switch (prop->type)
		{
			case K_LVL_PROP_SHELL:
			{
				if ((prop->bVar1 == false) && (prop->physPt->m_data.bContactStarted))
				{
					prop->bVar1 = true;
					/*
					if(prop->nSubType == 0) //shotgun shell
						SND_PLAY_POSITIONAL(SNDIDX_RIFLE_SHELL_DROP, prop->physPt->m_data.pos);
					else
						SND_PLAY_POSITIONAL(SNDIDX_SHOTGUN_SHELL_DROP, prop->physPt->m_data.pos);
						*/
				}

				if (prop->physPt->m_data.bIsStatic)
				{
					//put shell as decal
					if (randint(1000) < 200)
					{
						//daca collisionul are AI inseamna ca e lift sau ceva deci nu lasam sange
						if ((prop->physPt->m_data.pContactShape != null) && (prop->physPt->m_data.pContactShape->AIstate == K_AI_STATE_UNDEFINED))
							AddDecal(K_LVL_DECAL_LAYER_BACKOBJECTS, prop->physPt->m_data.pos, prop->spr.animationIdx, prop->spr.currentFrame, prop->spr.color);
					}
					killprop = true;
				}
				else if (!PointInRect(prop->physPt->m_data.pos, camrect))
				{
					killprop = true;
				}
			}
			break;
			case K_LVL_PROP_FIRE_SOURCE:
			{	
				if (!prop->physPt->m_data.bContacting)
				{
					if (m_Timers.Tick(50))
					{
						g_particlesMgr.GenerateFireRing(prop->physPt->m_data.pos, 5, 10.0f, 15.0f, K_PART_LAYER_RT_FRONT_NRM);
					}
				}
				//genereaza particula de lava
				if ((prop->physPt->m_data.bContactStarted) && (prop->physPt->m_data.contactNormal.y < -0.5f))
				{
					int anmidx = ANM_PARTICLES_SPR_MELTING_LAVA1;
					if (randint(100) < 60)
						anmidx = ANM_PARTICLES_SPR_MELTING_LAVA2;
					g_particlesMgr.AddParticle(anmidx, true, 0, &prop->physPt->m_data.contactPos, NULL, NULL, 4.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_BACK_NRM);
				}
				//genereaza damage pana inainte de ultima secunda
				if ((m_Timers.Tick(500.0f)) && (prop->physPt->m_data.bIsStatic) && (prop->fTimer > 1.0f))
				{
					CWeaponTemplate* wtempl = GetTemplateWeapon(L"WPN_LAVA_MELEE");
					D3DXVECTOR2 normal(0.0f, -1.0f);
					ShootBullet(&wtempl->bulletTemplate, K_LVL_ACT_CLASS_TRAP, 0, prop->physPt->m_data.pos + normal, normal);
				}

				prop->fTimer -= dTime;
				if (prop->fTimer <= 0.0f)
				{
					killprop = true;
				}
			}
			break;

			case K_LVL_PROP_MEAT:
			{
				DWORD dwCol = 0xff671010;
				if (prop->nSubType != 0) //zombies green blood
					dwCol = 0xff82b600;

				if (m_Timers.Tick(50))
				{
					g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_BLOOD, false, randint(5), &D3DXVECTOR2(prop->physPt->m_data.pos.x + randfloatsgn(5.0f), prop->physPt->m_data.pos.y + randfloatsgn(5.0f)), 
						&D3DXVECTOR2(0.0f, 20.0f), &(prop->physPt->m_data.speed / (5.0f + randfloat(4.0f))), 0.6f, 1.0f, 0.0f, randfloat(PI), randfloatsgn(2.0f), 0.1f, 0.1f, dwCol, K_PART_LAYER_RT_BACK_NRM);
				}
				//only stain at high velocities
				if ((prop->physPt->m_data.bContactStarted) && (prop->physPt->m_data.contactNormal.y < 0.0f))
				{
					//don't stain moving platforms
					if ((prop->physPt->m_data.pContactShape != null) && (prop->physPt->m_data.pContactShape->AIstate == K_AI_STATE_UNDEFINED))
					{
						if(prop->nSubType == 0)
							AddDecal(K_LVL_DECAL_LAYER_BACKOBJECTS, prop->physPt->m_data.pos, ANM_ACTIVES_SPR_DECAL_BLOOD_FRONTLAYER, randint(3), 0xffffffff);
						else
							AddDecal(K_LVL_DECAL_LAYER_BACKOBJECTS, prop->physPt->m_data.pos, ANM_ACTIVES_SPR_DECAL_BLOOD_FRONTLAYER_GREEN, randint(3), 0xffffffff);

//						SND_PLAY_POSITIONAL_RAND2(SNDIDX_GIBLET1, SNDIDX_GIBLET2, prop->physPt->m_data.pos);
					}
				}

				if ((prop->physPt->m_data.bIsStatic) || (!PointInRect(prop->physPt->m_data.pos, camrect_larger)))
				{
					killprop = true;
				}
			}
			break;

			case K_LVL_PROP_GOO:
			{
				if (m_Timers.Tick(50))
				{
					g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_BLOOD, false, randint(5), &D3DXVECTOR2(prop->physPt->m_data.pos.x + randfloatsgn(5.0f), prop->physPt->m_data.pos.y + randfloatsgn(5.0f)),
						&D3DXVECTOR2(0.0f, 20.0f), &(prop->physPt->m_data.speed / (5.0f + randfloat(4.0f))), 0.6f, 1.0f, 0.0f, randfloat(PI), randfloatsgn(2.0f), 0.1f, 0.1f, 0xff00ff00, K_PART_LAYER_RT_BACK_NRM);
				}

				if ((prop->physPt->m_data.bIsStatic) || (!PointInRect(prop->physPt->m_data.pos, camrect_larger)))
				{
					killprop = true;
//					SND_PLAY_POSITIONAL_RAND2(SNDIDX_GIBLET1, SNDIDX_GIBLET2, prop->physPt->m_data.pos);
				}
			}
			break;


			case K_LVL_PROP_SHRAPNEL_SMOKING:
			{
				//update sprite
				node->m_data.spr.Update(&m_sprActives, dTime);
				//add smoke
				if ((m_Timers.Tick(60)) && (!prop->physPt->m_data.bContacting))
				{
					float fAng = randfloat(DOUBLE_PI);
					D3DXVECTOR2 vDir(sin(fAng), cos(fAng));
					g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_PUFF_XS1 + randint(3), true, 0, &D3DXVECTOR2(prop->physPt->m_data.pos.x + randfloatsgn(2.0f), prop->physPt->m_data.pos.y + randfloatsgn(2.0f)),
						NULL, &(vDir * (5.0f + randfloat(5.0f))), 1.0f, 1.0f, 0.0f, fAng, 0.0f, 0.0f, 0.0f, 0xaaffffff, K_PART_LAYER_RT_FRONT_NRM);
				}
				//genereaza particule de foc doar cat e roshu
				if (node->m_data.fTimer > 0.0f)
				{
					node->m_data.fTimer -= dTime;
					if ((m_Timers.Tick(80)) && (!prop->physPt->m_data.bContacting))
					{
						g_particlesMgr.GenerateFireRing(prop->physPt->m_data.pos, 2, 8.0f, 10.0f, K_PART_LAYER_RT_FRONT_NRM);
					}
					//some secondary explosions too
					if ((m_Timers.Tick(120)) && (randompercent(50.0f)))
					{
						g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRECRACKER1 + randint(2), true, 0, &prop->physPt->m_data.pos,
							NULL, NULL, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
					}
				}

				if ((prop->physPt->m_data.bIsStatic) || (!PointInRect(prop->physPt->m_data.pos, camrect_larger)))
				{
					killprop = true;
					//smoke puff when dead
					int nAnmId = ANM_PARTICLES_SPR_PUFF_S_XS;
					if (randompercent(50.0f))
						nAnmId = ANM_PARTICLES_SPR_PUFF_S_XXS;

					g_particlesMgr.AddParticle(nAnmId, true, 0, &prop->physPt->m_data.pos,
						NULL, NULL, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xaaffffff, K_PART_LAYER_RT_FRONT_NRM);
				}
			}
			break;
			case K_LVL_PROP_LIGHT:
			{
				prop->fTimer += dTime;
				//kill on timing out
				if (prop->fTimer >= prop->fLightDuration)
					killprop = true;
			}
			break;
			case K_LVL_PROP_EXPLOSION:
			{
				prop->fTimer -= dTime;
				if (prop->fTimer <= 0.0f)
					killprop = true;
			}
			break;
		}
		//ii dam release
		if (killprop)
		{
			//release la nodul de fizica !!!
 			m_poolPhysPts.DismissNode(prop->physPt);
			//si eliberez glontul
			m_poolProps.DismissNode(node);
		}
		//avansez pointer
		node = nextnode;
	}
}

void CLevel::PaintProps()
{
	m_pSprite->SetTransform(&g_matIdentity);
	D3DXMATRIXA16 mattrans;

	CLinkedPool<CLevelProp>::CLinkedPoolNode *node = m_poolProps.pListUsed.m_pNext;
	while (node != &m_poolProps.pListUsed)
	{
		//salvez locatia urmatoare ca sa pot avansa pe ea
		CLinkedPool<CLevelProp>::CLinkedPoolNode *nextnode = node->m_pNext;

		switch (node->m_data.type)
		{
			case K_LVL_PROP_FIRE_SOURCE:
			{
				if (!node->m_data.physPt->m_data.bContacting)
				{
					D3DXVECTOR2 ppos = node->m_data.physPt->m_data.pos;
					float falpha = LIMIT(node->m_data.fTimer, 0.0f, 1.0f);
					CSprite::paintFrameModule(&m_sprActives, ppos.x, ppos.y, ANM_ACTIVES_SPR_BULLETS_FIRE, 0, 0, D3DCOLOR_FFFA(falpha));
				}
			}
			break;
			case K_LVL_PROP_SHELL:
			{
				node->m_data.spr.pos = node->m_data.physPt->m_data.pos;
				node->m_data.spr.currentFrame = node->m_data.nSubType * 4 + (int(node->m_data.spr.pos.x * 3.0f) % 4);
				node->m_data.spr.paint_firstModule(&m_sprActives);
			}
			break;
			case K_LVL_PROP_SHRAPNEL_SMOKING:
			{
				node->m_data.spr.pos = node->m_data.physPt->m_data.pos;
				
				node->m_data.spr2.pos = node->m_data.physPt->m_data.pos;
				node->m_data.spr2.color = D3DCOLOR_FFFA(LIMIT(node->m_data.fTimer, 0.0f, 1.0f));

				node->m_data.spr.paint_firstModule(&m_sprActives);
				node->m_data.spr2.paint_firstModule(&m_sprActives);
			}
			break;
			case K_LVL_PROP_GOO:
			case K_LVL_PROP_MEAT:
			{
				node->m_data.spr.pos = node->m_data.physPt->m_data.pos;
				node->m_data.spr.paint_firstModule(&m_sprActives);
			}
			break;
		}

		//avansez pointer
		node = nextnode;
	}
}


void CLevel::GenerateEffect(ELVLEffectType nEffectType, D3DXVECTOR2 pos, float fSize, DWORD color)
{
	switch (nEffectType)
	{
		case K_LVL_EFFECT_STONE_BREAK:
		{
			g_particlesMgr.GenerateSmokePuff(D3DXVECTOR2(pos.x, pos.y - 10.0f), 20.0f, K_PART_LAYER_RT_FRONT_NRM);
			m_camLevel.ShakeScreen(2.0f, 8.0f, &pos);

//			SND_PLAY_POSITIONAL(SNDIDX_STONE_BREAK1, pos);
		}
		break;
		case K_LVL_EFFECT_EXPLODING_ZOMBIE_DIE:
		{
			AddProp_Explo(hash_EXPLO_INVISIBLE_EXPLODING_ZOMBIE, pos, 0, K_LVL_ACT_CLASS_ZOMBIE);
			//throw slimes
			CWeaponTemplate* wpnTemplate = GetTemplateWeapon(FastHash(L"WPN_GREEN_GOO_EXPLODING_ZOMBIE"));
			if (wpnTemplate != null)
			{
				for (int kk = 0; kk < 6; kk++)
				{
					float ang = kk * (PI / 6.0f);
					D3DXVECTOR2 vdir(cos(ang), -sin(ang));

					ShootBullet(&wpnTemplate->bulletTemplate, K_LVL_ACT_CLASS_ZOMBIE, 0, D3DXVECTOR2(pos.x, pos.y), vdir);
				}
			}
			//gibs
			//blood splat (sortate crescator in animatie)
			AddDecal_BloodSplat(pos, true, K_LVL_ACT_CLASS_ZOMBIE);
//			SND_PLAY_POSITIONAL_RAND2(SNDIDX_BULLET_BODY_GIBBED_01, SNDIDX_BULLET_BODY_GIBBED_02, pos);
			CAABB genbox(pos.x - 10.0f, pos.y - 15.0f, pos.x + 10.0f, pos.y);
			for (int ll = 0; ll < 6; ll++)
			{
				AddProp(K_LVL_PROP_MEAT, AABB_GetRandomPointInBox(genbox), &D3DXVECTOR2(randfloatsgn(50.0f), -130.0f - randfloat(100.0f)), &g_vecGravity, 1);
			}
			//goes straight down to stain the floor
			AddProp(K_LVL_PROP_MEAT, D3DXVECTOR2(pos.x, pos.y - 10.0f), &D3DXVECTOR2(200.0f, 50.0f), &g_vecGravity, 1);
			AddProp(K_LVL_PROP_MEAT, D3DXVECTOR2(pos.x, pos.y - 10.0f), &D3DXVECTOR2(-200.0f, 50.0f), &g_vecGravity, 1);
			//human blood gibs
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_HUMAN_SPLAT_MED, true, 0, &pos, NULL, NULL, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xff1a3423, K_PART_LAYER_RT_FRONT_NRM);
		}
		break;
		case K_LVL_EFFECT_EXPLO_LARGE:
		{
			AddProp_Explo(hash_EXPLO_LARGE_XL, pos, 0, K_LVL_ACT_CLASS_EXPLOSION);
		}
		break;
		case K_LVL_EFFECT_ELECTRIC_BREAK_SPARKS:
		{
			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.4f, 0.1f, 0x88FDB727, 1.0f);
			//particule sparkle
			for (int kk = 0; kk < 20; kk++)
			{
				g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRESPARK2, true, randint(2), &D3DXVECTOR2(pos.x + randfloatsgn(fSize), pos.y + randfloatsgn(fSize)), &g_vecGravity, &D3DXVECTOR2(randfloatsgn(60.0f), -10.0f - randfloat(40.0f)), 0.2f + randfloat(0.4f), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM_LIGHT, 1.5f, kk * 0.025f);
			}
		}
		break;
		case K_LVL_EFFECT_STARS_CONFETTI:
		{
			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.6f, 0.2f, 0x88FDB727, 3.0f * fSize);
			//fire ring
			for (int kk = 0; kk < 30; kk++)
			{
				float ang = randfloat(DOUBLE_PI);
				D3DXVECTOR2 vdir(cos(ang), sin(ang));
				if (randompercent(50.0f))
					g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRESPARK1, true, randint(2), &(pos + vdir * 10.0f), NULL, &(vdir * (40.0f + randfloat(20.0f))), 1.0f + randfloat(0.5f), 1.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.5f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM_LIGHT, 2.0f);
				else
					g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRESPARK2, true, 0, &(pos + vdir * 10.0f), NULL, &(vdir * (40.0f + randfloat(20.0f))), 1.0f + randfloat(0.5f), 1.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.5f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM_LIGHT, 2.0f);
			}

			//linii verticale
			for (int kk = 0; kk < 6; kk++)
			{
				g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_TELEPORT, false, 5 + randint(2), &D3DXVECTOR2(pos.x + randfloatsgn(8.0f), pos.y - 3), NULL, &D3DXVECTOR2(0.0f, -60.0f - randfloat(20.0f)), 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.2f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM_LIGHT, 0.0f, kk * 0.1f);
			}
			//add ring
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLOWS, false, 1, &pos, NULL, NULL, 0.2f, 0.2f, 10.0f, 0.0f, 0.0f, 0.1f, 0.3f, 0x55ffffff, K_PART_LAYER_RT_FRONT_NRM_LIGHT);
		}
		break;
		default:
			ErrorBox(K_ERR_WARNING, L"CLevel::GenerateEffect - Unknown effect!");
			break;
	}
}

void CLevel::GenerateEffect(CStringHash sEffectName, D3DXVECTOR2 pos, float fSize, DWORD color)
{
	ELVLEffectType effectidx = (ELVLEffectType)GetListIndexByNameHash(sEffectName.textHash, ELVLEffectTypeNames, K_LVL_EFFECTS_CNT);
	GenerateEffect(effectidx, pos, fSize, color);
}

void CLevel::TouchClosestActive(CActor * pToucherAct, float dTime)
{
	assert(pToucherAct != null);

	if(pToucherAct->pClosestTouchable != null)
	{
		pToucherAct->pClosestTouchable->Touch(pToucherAct->GetUID(), dTime);
	}
}

CCollisionShape * CLevel::GetCollisionShapeAt(D3DXVECTOR2 point, int collisionType)
{
	for (int kk = 0; kk < m_arrColShapes.GetSize(); kk++)
	{
		if ((collisionType != -1) && (m_arrColShapes[kk]->type != collisionType))
			continue;
		if (m_arrColShapes[kk]->bbox.PointIn(point))
			return m_arrColShapes[kk];
	}
	return null;
}

CCollisionShape* CLevel::GetCollisionShapeByUID(UINT32 nUID)
{
	if (nUID == 0)
		return null;

	for (int kk = 0; kk < m_arrColShapes.GetSize(); kk++)
	{
		if (m_arrColShapes[kk]->UID == nUID)
			return m_arrColShapes[kk];
	}
	return null;
}

CCollisionShape* CLevel::SpawnCollisionShape(int nType, D3DXVECTOR2 vMin, D3DXVECTOR2 vMax)
{
	CCollisionShape* pCol = new CCollisionShape();
	pCol->ID = GenerateNextID();
	pCol->type = nType;
	pCol->bbox_ini.Set_Corrected(vMin, vMax);
	pCol->bbox = pCol->bbox_ini;
	pCol->pos = pCol->bbox_ini.vCenter;
	pCol->collFlags = K_DIRFLAG_NONE;

	pCol->castShadows = false;

	switch (nType)
	{
		case K_LVL_COLL_TYPE_SOLID:
			pCol->collFlags = K_DIRFLAG_ALL;
			break;
		default:
			pCol->collFlags = K_DIRFLAG_NONE;
			break;
	}

	m_arrColShapes.Add(pCol);
	return pCol;
}

CCollisionShape* CLevel::ColShape_Segment_Intersection_Arr(D3DXVECTOR2 & start, D3DXVECTOR2 & end, CCollisionShape * arrBoxes[], int nBoxesCnt, D3DXVECTOR2 * retCollisionPoint, D3DXVECTOR2 * retNormal)
{
	//verificari initiale
	assert(arrBoxes != NULL);

	if (nBoxesCnt <= 0)
		return null;
	//calculeaza termeni segment
	D3DXVECTOR2 dir = end - start;
	float seglen = D3DXVec2Length(&dir);
	if (seglen <= 0.0f)
		return null;
	dir /= seglen;

	D3DXVECTOR2 dirfrac;
	// r.dir is unit direction vector of ray
	dirfrac.x = 1.0f / dir.x;
	dirfrac.y = 1.0f / dir.y;
	//tine intersectia minima
	float minTfinal = FLT_MAX;
	//fast check box - checks box box intersection before checking segment intersection
	CAABB boxCheck;
	boxCheck.Set_Corrected(start, end);
	//valoarea de return 
	CAABB* retBox = null;
	CCollisionShape* retShape = null;
	for (int kk = 0; kk < nBoxesCnt; kk++)
	{
		//cursorul prin arrBoxes
		CAABB* box = &arrBoxes[kk]->bbox;
		//vector bounding box noit intersecting target box means no collision
		if ((box->vMin.x > boxCheck.vMax.x) || (box->vMax.x < boxCheck.vMin.x) || (box->vMin.y > boxCheck.vMax.y) || (box->vMax.y < boxCheck.vMin.y))
			continue;

		float t1 = (box->vMin.x - start.x) * dirfrac.x;
		float t2 = (box->vMax.x - start.x) * dirfrac.x;
		float t3 = (box->vMin.y - start.y) * dirfrac.y;
		float t4 = (box->vMax.y - start.y) * dirfrac.y;

		float tmin = max(min(t1, t2), min(t3, t4));
		float tmax = min(max(t1, t2), max(t3, t4));

		// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behind us
		// if tmin > tmax, ray doesn't intersect AABB
		//daca tmin e mai mare decat lungimea segmentului inseamna ca se intersecteaza dupa al doilea punct
		//daca tmin e mai mare decat minTfinal inseamna ca am coliziune mai departata decat ultima verificata
		//AM INVERSAT if-ul ca sa fie mai scurt codul
		if ((tmax >= 0.0f) && (tmin <= tmax) && (tmin <= seglen) && (tmin < minTfinal))
		{
			retBox = box;
			retShape = arrBoxes[kk];
			minTfinal = tmin;
		}
	}
	//minTfinal contine procentul intersectiei
	if (retCollisionPoint != NULL)
	{
		if (retBox != null)
		{
			*retCollisionPoint = start + minTfinal * dir;
			//pentru normala: daca e intre ymin si ymax e coliziune cu latura verticala
			if (retNormal != NULL)
			{
				retNormal->x = retNormal->y = 0.0f;
				if ((retCollisionPoint->y > retBox->vMin.y) && (retCollisionPoint->y < retBox->vMax.y))
				{
					if (retCollisionPoint->x < retBox->vCenter.x)
						retNormal->x = -1.0f;
					else
						retNormal->x = 1.0f;
				}
				else
				{
					if (retCollisionPoint->y < retBox->vCenter.y)
						retNormal->y = -1.0f;
					else
						retNormal->y = 1.0f;
				}
			}
		}
		else
		{
			*retCollisionPoint = end;
			if (retNormal != null)
			{
				*retNormal = D3DXVECTOR2(0.0f, 0.0f);
			}
		}
	}
	//intorc boxul colizionat daca este cazul sau null daca nu a avut coliziune
	return retShape;
}


CCollisionShape* CLevel::ColShape_CAABB_Intersect_Arr(CAABB * aabbSrc, CCollisionShape * arrBoxes[], int nBoxesCnt)
{
	if (aabbSrc == null)
		return null;
	for (int kk = 0; kk < nBoxesCnt; kk++)
	{
		//cursorul prin arrBoxes
		CAABB* box = &arrBoxes[kk]->bbox;
		if (box->Intersects(aabbSrc))
			return arrBoxes[kk];
	}
	return null;
}




#pragma warning(pop)
