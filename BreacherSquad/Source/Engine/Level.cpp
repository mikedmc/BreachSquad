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
	///--- PROPS ---
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


HRESULT CLevel::InitActor(CActor * actor, CActorTemplate * actTemplate, Vec2 spawnPos)
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

	actor->vMoveDirN = Vec2(0.0f, 0.0f);

	actor->bReleaseIt = false;
	actor->speed = Vec2(0.0f, 0.0f);
	actor->vSpeedImpulse = Vec2(0.0f, 0.0f);
	actor->vecCamFollowPos = Vec2(0.0f, 0.0f);

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


CBulletHitReturnData CLevel::HitActor(CActor* actor, CBullet *pBullet, Vec2* pvProjectileMomentum)
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
			}
		}
	}

	//event got_hit
	if ((actor->fLife > 0.0f) && (actor->templateActor.actorClass > K_LVL_ACT_CLASS_PLAYER))
	{
		//adaug eventuri de GOT_HIT doar pe clasele HUMAN, cand sunt lovite de catre player
		//find shooter pos. defaults on pos based on bullet speed
		Vec2 evtpos = actor->posHeart;
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
					GiveStrategicPoints(actor->templateActor.fStrategicPoints, &Vec2(actor->bbox.vCenter.x, actor->bbox.vMin.y));
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
	/*
	if (((actor->collisionFlags & K_DIRFLAG_RIGHT) && (actor->vSpeedImpulse.x > 0.0f)) ||
		((actor->collisionFlags & K_DIRFLAG_LEFT) && (actor->vSpeedImpulse.x < 0.0f)))
	{
		actor->vSpeedImpulse.x = 0.0f;
	}
	*/

	retData.fPointsTaken = fBulletLostEnergy;
	return retData;
}

CBulletHitReturnData CLevel::HitActor(CActor * actor, float fDamage, UINT32 dwOwnerUID, EActorClass eOwnerClass, Vec2 *vDir /*= null*/, UINT32 dwBulletFlags /*= 0*/, int nArmorPiercingRating /*= 100*/, float fStunDuration /*= 0.0f*/)
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
int CLevel::MeleeBlow(int nBulletType, Vec2 vPos, Vec2 vDirection, UINT32 nOwnerUID, int nOwnerClass, float fRange, float fDamageActors, float fImpulse, float fStunDurationMax, EActorClass eIgnoredClass, float fRangeObjects, float fDamageObjects)
{
	//MeleeHit
	CFixedArray<CActor*, 50> arrAffectedActors;
	CActor* pClosestActor = null;
	CActor* pClosestActorAlive = null;
	float fMinDistSq = 100000.0f;
	float fMinDistSqAlive = 100000.0f;

	Vec2 vDirN;
	MUVec2Norm(&vDirN, &vDirection);
	//get shooter bbox
	CActor* pShooter = GetActorByUID(nOwnerUID);
	///--- check doors and windows vs melee ---
	if (fDamageObjects > 0.0f)
	{
		//coliziunea cu nivelul
		Vec2 collisionPoint, collisionNormal;
		Vec2 vEnd = (vPos + vDirN * fRangeObjects);
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
						Vec2 dir = collisionNormal;
						dir.y -= 1.0f;
						MUVec2Norm(&dir, &dir);
						for (int kk = 0; kk < 12; kk++)
						{
							g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRESPARK2, true, 2 + randint(2), &collisionPoint, &g_vecGravityOld, 
								&(Vec2(dir.x + randfloatsgn(0.4f), dir.y + randfloatsgn(0.4f)) * (40.0f + randfloat(20.0f))), 
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
					Vec2 sndpos1 = Vec2(colShape->bbox.vCenter.x - vDirN.x * (colShape->bbox.vHalfSize.x + 2.0f), colShape->bbox.vCenter.y);
					AddAIEvent(K_LVL_AI_EVENT_SOUND_THREAT, nOwnerUID, nOwnerClass, sndpos1, 200.0f, 1.0f);

					//add events behind door
					sndpos1 = Vec2(colShape->bbox.vCenter.x + vDirN.x * (colShape->bbox.vHalfSize.x + 2.0f), colShape->bbox.vCenter.y);
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
		Vec2 vTo = act->posHeart - vPos;
		//ignored class
		if (act->templateActor.actorClass == eIgnoredClass)
			continue;
		if ((act->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET) != 0)
			continue;
		//not in front of player and point not in bbox, skip it
		if ((SIGN(vTo.x) != SIGN(vDirection.x)) && (!act->bbox.PointIn(vPos)))
			continue;
		//too far?
		float fDistSq = MUVec2LenSq(&vTo);
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
* Takes all the necessary spawn data from the "Gear selection screen" object.
* \param nPlayerOrdinal - 0-player1 or 1-player2
* \param nAnimset: -1 to skip spawn animation, 0 first animation, 1 second animation
*/
void CLevel::SpawnPlayer(Vec2 spawnPos, int nPlayerOrdinal, int nAnimset)
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
		//AddProp_Light(nact->GetPosHeart(), ANM_LIGHTS_SPR_POINT1, 0.5f, 0.1f, 0x8888ff00, 1.0f);

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

CActor* CLevel::SpawnActor(Vec2 spawnPos, WCHAR* strTemplateName, int nLookDirSign, CStringHash* shStateOverride)
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

CProp* CLevel::SpawnProp(Vec2 spawnPos, int nAnimIdx, int nFrameIdx, int nLayer)
{
	CProp* obj = new CProp();

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
		obj->sprite.currentFrame = m_rand.RandInt(m_sprProps.GetAFramesCnt(obj->sprite.animationIdx));
	}
	//bbox
	RECTXYWH bbox_set = m_sprProps.GetAFrameBBox(animIdx, frameIdx);
	RECTXYWH objbox = m_sprProps.GetAFrameBBox_real(animIdx, frameIdx);
	obj->bbox_ini.Set(objbox);
	obj->bbox_exported_ini.Set(bbox_set);
	//daca e flipat pe X flipez si bbox. Pe Y nu e cazul pt ca se pastreaza in acelasi bbox in paint
	if (obj->flipX)
	{
		obj->bbox_ini.Move(Vec2(-2.0f * obj->bbox_ini.vCenter.x, 0.0f));
		obj->bbox_exported_ini.Move(Vec2(-2.0f * obj->bbox_exported_ini.vCenter.x, 0.0f));
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

	m_arrProps.Add(obj);

	return obj;
}

CLight*	CLevel::SpawnLight(Vec3 spawnPos, eLightType eType, DWORD dwColor, float fRadius, int profileID, bool bCastShadows)
{
	CLight *nl = new CLight();
	nl->ID = GenerateNextID();
	nl->type = eType;
	nl->fVolumeAlpha = 1.0f;
	nl->fIntensity = 1.0f;
	nl->vPos = spawnPos;

	nl->pos = Vec3ToVec2XY(nl->vPos);
	nl->pos_ini = nl->pos;
	//animID
	nl->animID = 0;
	nl->nProfileID = profileID;
	nl->fRadius = fRadius;
	nl->color = dwColor;
	nl->color_ini = nl->color;
	nl->castShadows = bCastShadows;

	//set all internal light data needed for rendering
	nl->UpdateInternalData(&m_sprLights);
	// called when adding the light to the lights array
	nl->PostConstructionInit();
	//add light and return it
	m_arrLights.Add(nl);
	return nl;
}

int CLevel::GetPowerupPlacingScore(CProp* active, Vec2 vPlacerPos)
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
	for (int kk = 0; kk < m_visibleList.logic_props_closeby.nCount; kk++)
	{
		CProp* pActiv = m_visibleList.logic_props_closeby.m_pData[kk];
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

bool CLevel::GetBestSpawningPos(Vec2 * vSpawn_ret, CAABB rectStart, CAABB * rectToAvoid)
{
	if (vSpawn_ret == null)
		return false;

	//#TODO: make sure we don't spawn under an elevator and return false if all spawn positions return under the elevator

	///--- find best spawn position ---
	Vec2 vSpawnFinal(rectStart.vCenter.x, rectStart.vMax.y);
	Vec2 spawnPos = vSpawnFinal;
	//try a few times to the left and right and compute score
	int nPlaceScore = -100000;
	for (int kk = 0; kk < 8; kk++)
	{
		int nScore = 0;
		int offx = ((kk / 2) * (((kk % 2) * 2) - 1)) * K_TILE_HSIZE;
		Vec2 vCheck(spawnPos.x + (float)offx, spawnPos.y);
		CAABB rectCheck = rectStart;
		rectCheck.Move(Vec2((float)offx, 0.0f));
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
		col = GetCollisionShapeAt(Vec2(vCheck.x + 6.0f, vCheck.y + 1.0f));
		if (col == null)
			nScore -= 50;
		col = GetCollisionShapeAt(Vec2(vCheck.x - 6.0f, vCheck.y + 1.0f));
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
	CCollisionShape* col = GetCollisionShapeAt(Vec2(vSpawnFinal.x + 5.0f, vSpawnFinal.y + 1.0f));
	if (col == null)
		col = GetCollisionShapeAt(Vec2(vSpawnFinal.x - 5.0f, vSpawnFinal.y + 1.0f));
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
	for (int kk = 0; kk < m_arrProps.GetSize(); kk++)
	{
		CProp* pActiv = m_arrProps[kk];
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
	m_bufferedPainter.Init(4000);
		
	m_bLoaded = false;
	m_bOneUpdateDone = false;

	m_dwSyncCheckHash = 0;

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
		m_arrPlayerLastSafePos[kk] = Vec2(0.0f, 0.0f);
	}
	//init interfaces
	m_interfaceIGM.Init(&m_sprInterface, pPlayerActor[0], pPlayerActor[1]);
	m_interfaceTextBubble.Init(&UTGetGUI().m_sprCol);

	vLastSpawnPoint = Vec2(0.0f, 0.0f);
	m_vCamPosDefault = Vec2(0.0f, 0.0f);
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
		if (!bnode.attribute(L"sMuzzleFlashAnim").empty())
			templ->nMuzzleFlashAnim = m_sprActors.GetAnimationIdxByName(bnode.attribute(L"sMuzzleFlashAnim").value());
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

void CLevel::UpdateDirtyRects()
{
	//#TODO: doesn't change WALKABLE floor flags, that should be done during loading or level editing for speed
	//#TODO: should make sure the level always has a 1 tile border!
	///--- compute tile flags ---
	for (int kk = 0; kk < m_arrDirtyRectsTL.size(); kk++)
	{
		RECTXYXY rect = m_arrDirtyRectsTL[kk];
		// take border tiles into account:
		// clamp to smaller size because we check neighbours
		rect.Clamp(0, 0, levelSizeTL.w - 1, levelSizeTL.h - 1);
		for (int yy = rect.y1; yy <= rect.y2; yy++)
		{
			for (int xx = rect.x1; xx <= rect.x2; xx++)
			{
				CTile* tl = &tiles[xx][yy];

				// take border tiles into account. they can't check for neighbours so we suppose they are solid
				if ((xx <= 0) || (yy <= 0) || (xx == levelSizeTL.w - 1) || (yy == levelSizeTL.h - 1))
				{
					tl->flags |= K_TILEFLAG_HASWALL_MASK;
					continue;
				}

				// neighbours
				CTile* tlL = &tiles[xx - 1][yy];
				CTile* tlR = &tiles[xx + 1][yy];
				CTile* tlU = &tiles[xx][yy - 1];
				CTile* tlD = &tiles[xx][yy + 1];
				///--- set wall flags on non walkable tiles
				if ((tl->flags & K_TILEFLAG_WALKABLE) == 0)
				{
					// clear flags
					FLAGOP_CLEAR(tl->flags, K_TILEFLAG_HASWALL_MASK);

					if (IS_FLAG_ANY(tlL->flags, K_TILEFLAG_WALKABLE))
					{
						tl->flags |= K_TILEFLAG_HASWALL_L;
					}
					if (IS_FLAG_ANY(tlR->flags, K_TILEFLAG_WALKABLE))
					{
						tl->flags |= K_TILEFLAG_HASWALL_R;
					}
					if (IS_FLAG_ANY(tlU->flags, K_TILEFLAG_WALKABLE))
					{
						tl->flags |= K_TILEFLAG_HASWALL_U;
					}
					if (IS_FLAG_ANY(tlD->flags, K_TILEFLAG_WALKABLE))
					{
						tl->flags |= K_TILEFLAG_HASWALL_D;
					}
				}


				///--- compute wall shadows
				CTile* tlDL = &tiles[xx - 1][yy + 1];
				// it can only receive if it's a floor or a wall but not a ceiling on that tile
				tl->nShadowFrame = -1;

				// only walls and floor get shadowed, when having a non walkable tile on the left (hole in the floor usually, but not water hole)
				bool bCanReceive = ((tl->tileIDs[K_TILE_LAYER_FLOOR] >= 0) || (tl->tileIDs[K_TILE_LAYER_WALLS] >= 0)) && 
					(tlL->tileIDs[K_TILE_LAYER_FLOOR] < 0) && (tl->tileIDs[K_TILE_LAYER_CEILING] < 0);
				if (bCanReceive)
				{
					int nCasterH = 0; 
					if (tlL->tileIDs[K_TILE_LAYER_CEILING] >= 0) nCasterH = 3;
					else if (tlL->tileIDs[K_TILE_LAYER_WALLS] >= 0)
					{
						if (tlDL->tileIDs[K_TILE_LAYER_WALLS] >= 0)
							nCasterH = 2;	// top of the wall
						else
							nCasterH = 1;   // base of the wall
					}
					int nReceiverH = 0;
					if (tl->tileIDs[K_TILE_LAYER_WALLS] >= 0)
					{
						if (tlD->tileIDs[K_TILE_LAYER_WALLS] >= 0)
							nReceiverH = 2;
						else
							nReceiverH = 1;
					}

					if (nReceiverH == 0) //floor
					{
						if (nCasterH == 1)
							tl->nShadowFrame = 0; //floor shadow start
						else
						{
							tl->nShadowFrame = 1; //continuous shadow
						}
					}
					else if ((nReceiverH == 1) && (nCasterH > 1))
					{
						tl->nShadowFrame = 2; //base of wall shadowed
					}
					else if ((nReceiverH == 2) && (nCasterH > 2))
					{
						tl->nShadowFrame = 3; //top of wall shadowed
					}
				}
			}
		}
	}

	// finished with dirty rects, clear the array
	m_arrDirtyRectsTL.clear();
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
					templ->animIDs[kk][0] = m_sprActors.GetAnimationIdxByName(nmnode.attribute(L"set0").value());
					if (templ->animIDs[kk][0] == -1)
					{
						ErrorBox(K_ERR_WARNING, L"Template set0 animation not found!\n%s", nmnode.attribute(L"set0").value());
					}
					//next sets aren't mandatory
					if (!nmnode.attribute(L"set1").empty())
					{
						templ->animIDs[kk][1] = m_sprActors.GetAnimationIdxByName(nmnode.attribute(L"set1").value());
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
		destAct->vecWeapon_abs[ll] = Vec2(0.0f, -1.0f); //setez pe -1 ca sa iasa din podea
		destAct->vecHeart_abs[ll] = Vec2(0.0f, -1.0f);
		destAct->vecGroundCheck_abs[ll] = Vec2(0.0f, 0.0f);
		destAct->stateBBoxes[ll].Set(0.0f, 0.0f, 0.0f, 0.0f);

		if (ll >= refposeframes)
			continue;

		if (refposeAnim >= 0)
		{
			POINTXYZ_INT pt;
			//cautam punct arma
			if (SUCCEEDED(m_sprActors.GetAFrameHitPointFlag(refposeAnim, ll, 0, K_LVL_ACTOR_HITPOINTFLAG_GUNPOS, &pt)))
			{
				destAct->vecWeapon_abs[ll] = Vec2(pt.x, pt.y);
			}
			//cautam punct inima
			if (SUCCEEDED(m_sprActors.GetAFrameHitPointFlag(refposeAnim, ll, 0, K_LVL_ACTOR_HITPOINTFLAG_HEARTPOS, &pt)))
			{
				destAct->vecHeart_abs[ll] = Vec2(pt.x, pt.y);
			}
			//cautam punct ground check
			if (SUCCEEDED(m_sprActors.GetAFrameHitPointFlag(refposeAnim, ll, 0, K_LVL_ACTOR_HITPOINTFLAG_GROUND_SWEEP, &pt)))
			{
				destAct->vecGroundCheck_abs[ll] = Vec2(pt.x, pt.y);
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
	Vec2 vDist(actor->pos.x - m_camLevel.GetCamPos().x, actor->pos.y - m_camLevel.GetCamPos().y);
	if (MUVec2Len(&vDist) < K_GAME_HALF_HEIGHT * 1.5f)
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
	Vec2 vChkPos = actor->pos;
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
	for (int kk = 0; kk < m_arrProps.GetSize(); kk++)
	{
		if (m_arrProps[kk]->ID == ID)
			return m_arrProps[kk];
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
	for (int kk = 0; kk < m_arrProps.GetSize(); kk++)
	{
		if (m_arrProps[kk]->GetUID() == UID)
			return m_arrProps[kk];
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
		float fDist = MUVec2Len(&(pPlayerActor[kk]->pos - sourceActor->pos));
		if (fDist < fMinDist)
		{
			plact = pPlayerActor[kk];
			fMinDist = fDist;
		}
	}
	return plact;
}

CActor* CLevel::GetClosestPlayer(Vec2 vSrcPos, bool bIgnoreDead)
{
	float fMinDist = 100000.0f;
	CActor* plact = null;
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if (pPlayerActor[kk] == null)
			continue;
		if ((bIgnoreDead) && (pPlayerActor[kk]->GetCurrentBehavior() == AI_BEHAVIOR_DEAD))
			continue;
		float fDist = MUVec2Len(&(pPlayerActor[kk]->pos - vSrcPos));
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

CProp* CLevel::GetActiveByUID(UINT32 UID)
{
	if (UID == 0)
		return NULL;
	for (int kk = 0; kk < m_arrProps.Count(); kk++)
	{
		if (m_arrProps[kk]->GetUID() == UID)
			return m_arrProps[kk];
	}
	return NULL;
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
				g_ChatWnd.AddLine(UTLang().strings[STR_ENTER_TO_CHAT]->sText, L"SYSTEM", K_CW_SYSTEM_COLOR);
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
			UTGetGUI().RemoveAllLayers();

			SND_STOP_GROUP("music", false, true);

			//SND_PLAY_ONCE(SNDIDX_ANNOUNCER_WIN, 0);
			//SND_PLAY_ONCE(SNDIDX_STINGER_WIN, 0);

			m_levelSubState = 0;
			m_levelStateTimer = 0.0f;

			g_particlesMgr.AddStringDummy(K_PDUMMY_STRING_WIDEBAR, Vec2(0.0f, -50.0f), STR_MISSION_ACCOMPLISHED, FONTIDX_12_WOW, 1.0f, 1.8f, K_COLOR_SELECTED_TEXT);
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
			UTGetGUI().RemoveAllLayers();

			SND_STOP_GROUP("music", false, true);

			//SND_PLAY_ONCE(SNDIDX_ANNOUNCER_FAIL, 0);
			//SND_PLAY_ONCE(SNDIDX_STINGER_LOSE, 0);

			m_levelStateParam = nLevelStateParam; //reason why failed - stringIDX
			m_levelSubState = 0;
			m_levelStateTimer = 0.0f;

			g_particlesMgr.AddStringDummy(K_PDUMMY_STRING_WIDEBAR, Vec2(0.0f, -50.0f), STR_MISSION_FAILED, FONTIDX_12_WOW, 1.0f, 1.8f, K_COLOR_SELECTED_TEXT);
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
				Vec2 retpt = m_camLevel.ScreenToWorld(Vec2(fAxisValue, 0.0f));
				// make coords relative to player
				retpt.x -= pPlayer->pos.x;
				// set final coords
				ret_fAxisValue = retpt.x;
				return true;
			}
			else
			{
				Vec2 retpt = m_camLevel.ScreenToWorld(Vec2(0.0f, fAxisValue));
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


// allocate temp verts buffer on stack
const int temp_arrVertsSize = 1200 * 3;
_VERTEX_PNCT4T4 temp_arrVerts[temp_arrVertsSize];

void CLevel::BuildDynamicGeometry(CAABB camAABB)
{
	const int arrOccludersSize = 200;
	COccluderSegment arrOccluders[arrOccludersSize];
	///--- create vert buffers for lights ---
	for (int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++)
	{
		CLight *nl = m_visibleList.visible_lights.m_pData[kk];
		switch (nl->type)
		{
			case K_LVL_LT_IES:
			case K_LVL_LT_POINT:
			{
				//--- create light volumes for shadow casting lights	---
				nl->m_nLightMeshIdx = -1;
				if (nl->castShadows)
				{
					// returns a list of segments that will form shadows (from both tiles and collision boxes)
					int nOccluders = GetOccluderSegments(Vec3ToVec2XY(nl->vPos), nl->bbox, arrOccluders, arrOccludersSize);


					// shows occluders instead of mesh. Checked for consistency.
					/*
					int nVertCnt = 0;
					for (int kk = 0; kk < nOccluders; kk++)
					{
						temp_arrVerts[nVertCnt].pos = Vec3(nl->vPos.x, nl->vPos.y, 0.0f);
						temp_arrVerts[nVertCnt].color = 0x00ffffff; nVertCnt++;
						temp_arrVerts[nVertCnt].pos = Vec2ToVec3XY0(arrOccluders[kk].vStart);
						temp_arrVerts[nVertCnt].color = 0xff00ff00; nVertCnt++;
						temp_arrVerts[nVertCnt].pos = Vec2ToVec3XY0(arrOccluders[kk].vEnd);
						temp_arrVerts[nVertCnt].color = 0xff0000ff; nVertCnt++;
					}

					if (nVertCnt > 3)
					{
						m_bufferedPainter.BeginMesh(nl->m_nLightMeshIdx);
						m_bufferedPainter.AddTriangles(temp_arrVerts, nVertCnt / 3);
						m_bufferedPainter.EndMesh();
					}
					*/


					if (nOccluders > 0)
					{
						// sends rays and builds the light FOV as a triangle list mesh
						int retVerts = FOVUtil::BuildOccludedVolume(Vec3ToVec2XY(nl->vPos), nl->color, arrOccluders, nOccluders, temp_arrVerts, temp_arrVertsSize);

						// adaugam triunghiurile ca si mesh
						if (retVerts > 0)
						{
							//adauga mesh dinamic pentru volumul umbrei
							m_bufferedPainter.BeginMesh(nl->m_nLightMeshIdx);
							m_bufferedPainter.AddTriangles(temp_arrVerts, retVerts / 3);
							m_bufferedPainter.EndMesh();
						}
					}

				}
				else
				{
					Vec3 lcorners[4]; //ul, ur, dl, dr
					memcpy(lcorners, nl->lCorners, 4 * sizeof(Vec3));
					// move mesh to light position (!z must remain 0!)
					lcorners[0].x += nl->vPos.x; lcorners[0].y += nl->vPos.y;
					lcorners[1].x += nl->vPos.x; lcorners[1].y += nl->vPos.y;
					lcorners[2].x += nl->vPos.x; lcorners[2].y += nl->vPos.y;
					lcorners[3].x += nl->vPos.x; lcorners[3].y += nl->vPos.y;
					//write final VS verts
					_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
					vul.pos = lcorners[0];
					vur.pos = lcorners[1];
					vdr.pos = lcorners[2];
					vdl.pos = lcorners[3];
					//set color
					vul.color = vur.color = vdl.color = vdr.color = nl->color;
					// triangles vb
					_VERTEX_PNCT4T4 lightRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
					lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
					lightRectV[3] = vur; lightRectV[4] = vdr; lightRectV[5] = vdl;

					m_bufferedPainter.BeginMesh(nl->m_nLightMeshIdx);
					m_bufferedPainter.AddTriangles(lightRectV, 2);
					m_bufferedPainter.EndMesh();
				}
			}
			break;
			case K_LVL_LT_PROJECTED_DIR:
			{
				//create light mesh - rotating the actual mesh isn't necessary
				Vec3 lcorners[4]; //ul, ur, dr, dl
				memcpy(lcorners, nl->lCorners, 4 * sizeof(Vec3));
				//move mesh to final pos
				lcorners[0].x += nl->vPos.x; lcorners[0].y += nl->vPos.y;
				lcorners[1].x += nl->vPos.x; lcorners[1].y += nl->vPos.y;
				lcorners[2].x += nl->vPos.x; lcorners[2].y += nl->vPos.y;
				lcorners[3].x += nl->vPos.x; lcorners[3].y += nl->vPos.y;
				//scriu VS-ul final
				_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
				vul.pos = lcorners[0];
				vur.pos = lcorners[1];
				vdr.pos = lcorners[2];
				vdl.pos = lcorners[3];
				//setez culoarea
				vul.color = vur.color = vdl.color = vdr.color = nl->color;
				//light direction as normals but not really used
				vul.n = vur.n = vdl.n = vdr.n = nl->vnDir;

				_VERTEX_PNCT4T4 lightRectV[6];
				lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
				lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;

				//dynamic mesh index for light geometry
				nl->m_nLightMeshIdx = -1; //resetez idx mesh
				m_bufferedPainter.BeginMesh(nl->m_nLightMeshIdx);
				m_bufferedPainter.AddTriangles(lightRectV, 2);
				m_bufferedPainter.EndMesh();
			}
			break;

			case K_LVL_LT_DIRECTIONAL:
			case K_LVL_LT_AMBIENTAL:
			{
				_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
				vul.pos = Vec3(camAABB.vMin.x, camAABB.vMin.y, 0.0f);
				vur.pos = Vec3(camAABB.vMax.x, camAABB.vMin.y, 0.0f);
				vdl.pos = Vec3(camAABB.vMin.x, camAABB.vMax.y, 0.0f);
				vdr.pos = Vec3(camAABB.vMax.x, camAABB.vMax.y, 0.0f);
				//set color
				vul.color = vur.color = vdl.color = vdr.color = nl->color;
				//build verts
				_VERTEX_PNCT4T4 lightRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
				lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
				lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;

				m_bufferedPainter.BeginMesh(nl->m_nLightMeshIdx);
				m_bufferedPainter.AddTriangles(lightRectV, 2);
				m_bufferedPainter.EndMesh();
			}
			break;
		}
	}


	/// 2. other lights: bullets, particles, etc
	//PROPS lights - temp lights - gunshot lights, explo lights
	m_propsLightsMeshIdx = -1;
	m_bufferedPainter.BeginMesh(m_propsLightsMeshIdx);

	CLinkedPool<CDoofer>::CLinkedPoolNode *node = m_poolDoofers.pListUsed.m_pNext;
	while (node != &m_poolDoofers.pListUsed)
	{
		//salvez locatia urmatoare ca s apot avansa pe ea
		CLinkedPool<CDoofer>::CLinkedPoolNode *nextnode = node->m_pNext;
		CDoofer* prop = &node->m_data;

		if (prop->bMakesLight)
		{
			if (prop->sprLight.animationIdx >= 0)
			{
				//Creez forma luminii (mesh-ul)
				RECTLTRB_F realrect = m_sprLights.GetAFrameBBox_real(prop->sprLight.animationIdx, 0);
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
				Vec2 bpos2D = node->m_data.physPt->m_data.pos;
				Vec3 bpos(node->m_data.physPt->m_data.pos.x, node->m_data.physPt->m_data.pos.y, 50.0f);
				vul.pos = Vec3(bpos.x + realrect.left, bpos.y + realrect.top, 0.0f);
				vur.pos = Vec3(bpos.x + realrect.right, bpos.y + realrect.top, 0.0f);
				vdl.pos = Vec3(bpos.x + realrect.left, bpos.y + realrect.bottom, 0.0f);
				vdr.pos = Vec3(bpos.x + realrect.right, bpos.y + realrect.bottom, 0.0f);
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
				vul.tex1 = Vec4(lTexRect.left, lTexRect.top, 0.0f, 0.0f);
				vur.tex1 = Vec4(lTexRect.right, lTexRect.top, 0.0f, 0.0f);
				vdl.tex1 = Vec4(lTexRect.left, lTexRect.bottom, 0.0f, 0.0f);
				vdr.tex1 = Vec4(lTexRect.right, lTexRect.bottom, 0.0f, 0.0f);
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
	m_bufferedPainter.BeginMesh(m_waterMeshIdx);
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
				Vec2 texoff = col->bbox.vMin - wbb.vMin;
				//save water plys in a sigle mesh, clipped to screen rect

				//scriu VS-ul final
				_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
				vul.pos = Vec3(wbb.vMin.x, wbb.vMin.y, 0.0f);
				vur.pos = Vec3(wbb.vMax.x, wbb.vMin.y, 0.0f);
				vdl.pos = Vec3(wbb.vMin.x, wbb.vMax.y, 0.0f);
				vdr.pos = Vec3(wbb.vMax.x, wbb.vMax.y, 0.0f);
				//setez culoarea
				//#TODO: culoarea sa fie setata undeva in editor. Poate as putea sa pun control de culoare la collision boxuri...
				vul.color = vur.color = vdl.color = vdr.color = 0xaa30AFFF;// col->color;
				//setez coordonate textura apa
				Vec2 texul = (wbb.vMin * waterTexScale) / waterTexSize;
				Vec2 texdr = (wbb.vMax * waterTexScale) / waterTexSize;

				RECTLTRB_F lTexRect(texul.x, texul.y, texdr.x, texdr.y);
				//Coord de mapare pe RTT (tex2) se seteaza din shader
				vul.tex1 = Vec4(lTexRect.left, lTexRect.top, 0.0f, 0.0f);
				vur.tex1 = Vec4(lTexRect.right, lTexRect.top, 0.0f, 0.0f);
				vdl.tex1 = Vec4(lTexRect.left, lTexRect.bottom, 0.0f, 0.0f);
				vdr.tex1 = Vec4(lTexRect.right, lTexRect.bottom, 0.0f, 0.0f);
				//setez normalele finale
				vul.n = vur.n = vdl.n = vdr.n = Vec3(0.0f, 0.0f, 0.0f);
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
	m_bufferedPainter.BeginMesh(m_fogofwarMeshIdx);

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
				vul.pos = Vec3(wbb.vMin.x, wbb.vMin.y, 0.0f);
				vur.pos = Vec3(wbb.vMax.x, wbb.vMin.y, 0.0f);
				vdl.pos = Vec3(wbb.vMin.x, wbb.vMax.y, 0.0f);
				vdr.pos = Vec3(wbb.vMax.x, wbb.vMax.y, 0.0f);
				//setez culoarea (setata pe onload)
				vul.color = vur.color = vdl.color = vdr.color = col->color;
				//setez coordonate tex2 (nu se folosesc)
				Vec2 texul = wbb.vMin;
				Vec2 texdr = wbb.vMax;

				RECTLTRB_F lTexRect(texul.x, texul.y, texdr.x, texdr.y);
				//Coord de mapare pe RTT (tex2) se seteaza din shader
				vul.tex1 = Vec4(lTexRect.left, lTexRect.top, 0.0f, 0.0f);
				vur.tex1 = Vec4(lTexRect.right, lTexRect.top, 0.0f, 0.0f);
				vdl.tex1 = Vec4(lTexRect.left, lTexRect.bottom, 0.0f, 0.0f);
				vdr.tex1 = Vec4(lTexRect.right, lTexRect.bottom, 0.0f, 0.0f);
				//setez normalele finale
				vul.n = vur.n = vdl.n = vdr.n = Vec3(0.0f, 0.0f, 0.0f);
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
					Vec2 vActPl = pPlayerActor[kk]->posHeart - active->pos;
					float fActPlLen = MUVec2Len(&vActPl);
					if (fActPlLen > active->AIfvar3) //radius
						break;
					//normalize
					//vActPl /= fActPlLen;
					float fPlayerAng = UTMath::GetVectorAngle(vActPl);
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

			Vec2 delta = Vec2(radX * cos(timeMul * fTimeline), radY * sin(timeMul * fTimeline));
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
				Vec2 targetDelta = active->pTarget->pos - active->pTarget->pos_ini;
				//mut obiectul cu delta totala a targetului
				active->SetPos(active->pos_ini + targetDelta);
			}
		}
		break;
		case K_AI_STATE_FN_GET_TARGET_ANG:
		{
			if (active->pTarget != NULL)
			{
				Vec2 targetVec = active->pos_ini - active->pTarget->pos_ini;
				Mat matrot;
				MuMatRotZ(&matrot, active->pTarget->fAngle - active->pTarget->fAngle_ini);
				MUVec2TransformCoord(&targetVec, &targetVec, &matrot);
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
			Vec2 newpos = rail->GetPos(active->AItimer1);
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
				Vec2 vCheckPt(pPlayerActor[kk]->bbox.vCenter.x, pPlayerActor[kk]->bbox.vMin.y);
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
				Vec2 vCheckPt(m_arrActors[kk]->bbox.vCenter.x, m_arrActors[kk]->bbox.vMin.y);
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
						CProp* winact = dynamic_cast<CProp*>(colshape->pTarget);
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
							Vec2 ppos = AABB_GetRandomPointInBox(colshape->bbox);
							g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLASS_SHARDS, false, randint(5), &ppos, &g_vecGravityOld, &Vec2(dirx * (60.0f + randfloat(60.0f)), -40.0f + randfloatsgn(50.0f)), 0.3f + randfloat(0.2f), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
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
						Vec2 ppos = AABB_GetRandomPointInBox(colshape->bbox);
						g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_WOODEN_SPLINTERS, false, randint(6), &ppos, &g_vecGravityOld, &Vec2(dirx * (100.0f + randfloat(60.0f)), -40.0f + randfloatsgn(50.0f)), 0.3f + randfloat(0.2f), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
					}
					//Adauga events de zgomot dincolo de usa
					Vec2 sndpos1 = Vec2(colshape->bbox.vCenter.x + dirx * (colshape->bbox.vHalfSize.x + 2.0f), colshape->bbox.vCenter.y);
					//SND_PLAY_POSITIONAL_RAND2(SNDIDX_DOOR_HIT1, SNDIDX_DOOR_HIT2, sndpos1);
				}

				//if dead
				/*
				if (colshape->AIfvar1 <= 0.0f)
				{
					//seteaza animatia de usa sparta
					if (colshape->pTarget != NULL)
					{
						//trebuie sa pointeze spre un CActive neaparat
						CProp* dooract = dynamic_cast<CProp*>(colshape->pTarget);
						if (dooract == null)
						{
							ErrorBox(K_ERR_WARNING, L"K_AI_STATE_COLL_BREAKABLE_DOOR bad cast to CActive");
							break;
						}

						//centram pe bboxul initial
						Vec2 vcenter = dooract->bbox.vCenter;
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

							Vec2 secondExploPos(colshape->bbox.vCenter.x - (colshape->bbox.vHalfSize.x + 1.0f), colshape->bbox.vCenter.y);
							AddProp_Explo(hash_EXPLO_STUN_INVISIBLE, secondExploPos, 0, K_LVL_ACT_CLASS_PLAYER);
						}
						else
						{
							//trebuie setat si pe else pentru ca poate veni deja flipat din editor
							dooract->flipX = false;

							Vec2 secondExploPos(colshape->bbox.vCenter.x + (colshape->bbox.vHalfSize.x + 1.0f), colshape->bbox.vCenter.y);
							AddProp_Explo(hash_EXPLO_STUN_INVISIBLE, secondExploPos, 0, K_LVL_ACT_CLASS_PLAYER);
						}

						g_particlesMgr.GenerateDoorBreak(colshape->bbox.vCenter, Vec2(dirx, 0.0f), K_PART_LAYER_RT_FRONT_NRM);
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
							CProp* dooract = dynamic_cast<CProp*>(colshape->pTarget);
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
				*/
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
						CAABB aabbPlayerView(Vec2(pPlayer->posHeart.x - pPlayer->templateActor.distSee, pPlayer->posHeart.y - pPlayer->templateActor.distSee),
							Vec2(pPlayer->posHeart.x + pPlayer->templateActor.distSee, pPlayer->posHeart.y + pPlayer->templateActor.distSee));

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

							Vec2 vfrom = pPlayer->posHeart;
							Vec2 vto = vfrom;
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
							Vec2 fowColPt;
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
				Vec3 conepoint(0.0f, -light->AIfvar1, 0.0f);
				Vec3 ppos = Vec3(light->AIfvar2 * sin((light->fTimelineAI + light->AItimer2) * light->AItimer1), 0.0f, light->AIfvar2 * cos((light->fTimelineAI + light->AItimer2) * light->AItimer1));
				MUVec3Norm(&light->vnDir, &(ppos - conepoint));
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

void CLevel::UpdateAI_prop(CProp* prop, float dTime)
{
	//touch timer reset
	//daca trebuie actionat de toata echipa verific aici (doar pentru props pentru ca nu voi actiona pe actori sau collisions)
	bool bResetTouchTimer = false;
	if (prop->fTouchDuration < 0.0f)
	{
		for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
		{
			if (pPlayerActor[kk] != null)
			{
				//sa fiu sigur ca interactioneaza pe acelasi obiect
				if ((pPlayerActor[kk]->nInteractingState == 0) || (pPlayerActor[kk]->pClosestTouchable != prop))
				{
					prop->fTouchTimer = 0.0f;
					break;
				}
			}
		}
	}

	prop->UpdateTouchTimerReset(dTime);
	//daca am schimbat vizibilitatea
	prop->bHidden = prop->bSetHidden;
	//daca este hidden nu mai verifica AI
	if (prop->bHidden)
		return;

	//update timeline
	prop->fTimelineAI += dTime;

	//update sprite if animated
	if (prop->bAnimated)
	{
		UINT32 aframeFlag = prop->sprite.Update(&m_sprProps, dTime);
		//cand ajunge la capatul animatiei scoate flagul de animated
		if (prop->sprite.animStatus == ANIM_STATUS_FRAMELOCK)
			prop->bAnimated = false;
		//la obiectele animate luam bbox-ul la fiecare frame
		if ((prop->sprite.animStatus == ANIM_STATUS_PLAYING_FRAME_ADVANCED) || (prop->sprite.animStatus == ANIM_STATUS_FRAMELOCK))
		{
			RECTXYWH frrect = m_sprProps.GetAFrameBBox(prop->sprite.animationIdx, prop->sprite.currentFrame);
			prop->bbox_ini.Set(frrect);
			//nu pastreaza acelasi bbox la flip deci flipam bboxul
			if (prop->flipX)
			{
				prop->bbox_ini.Flip(true, false);
			}
		}
	}

	//daca nu a fost tratata starea curenta inseamna ca este particulara pt clasa asta
	if (prop->AIstate != K_AI_STATE_UNDEFINED)
	{
		//stari particulare obiectelor (se pot suprascrie cele default)
		switch (prop->AIstate)
		{
			case K_AI_STATE_ACTIVE_ZOMBIE_SPAWNER:
			{
			}
			break;

			case K_AI_STATE_ACTIVE_BOMB:
			{
				//daca nu esti pe playing nu mai scade counterul la bomba
				if (m_levelState != K_LVL_STATE_PLAYING)
					break;

				float fOldTimer = prop->AItimer1;
				prop->AItimer1 -= dTime;
				m_interfaceIGM.SetBombTimer(prop->AItimer1);

				//--- sounds ---
				if (prop->AItimer1 > 15.0f)
				{
					if (floor(fOldTimer) > floor(prop->AItimer1))
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

				if (prop->AItimer1 <= 0.0f)
				{
					m_interfaceIGM.SetBombTimer(0.0f);
					//add some explosions so everybody will die
					AddDoofer_Explo(hash_EXPLO_LARGE_XL, prop->pos, prop->UID, K_LVL_ACT_CLASS_EXPLOSION);
					AddDoofer_Explo(hash_EXPLO_LARGE_XL, prop->pos + Vec2(32.0f, 0.0f), prop->UID, K_LVL_ACT_CLASS_EXPLOSION);
					AddDoofer_Explo(hash_EXPLO_LARGE_XL, prop->pos - Vec2(32.0f, 0.0f), prop->UID, K_LVL_ACT_CLASS_EXPLOSION);

					g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_ROUND_XL, true, 0, &Vec2(prop->pos.x, prop->pos.y - 15.0f), NULL, NULL, 1.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);

					prop->sprite.setAnimation("BOMB_EXPLODED", &m_sprProps);

					SetLevelState(K_LVL_STATE_MISSION_FAILED, STR_BOMB_EXPLODED);
				}
			}
			break;
			case K_AI_STATE_ACTIVE_AMMO_BOX:
			{
				int nAmmoLeft = prop->varAIparams.GetVariantByName(L"n_ammoLeft")->m_asINT32;
				prop->sprite.currentFrame = nAmmoLeft;

				//fade out
				if (nAmmoLeft <= 0)
				{
					prop->AItimer1 -= dTime;
					if (prop->AItimer1 <= 0.0f)
					{
						prop->bReleaseIt = true;
					}
					//color
					float fAlpha = LIMIT(prop->AItimer1, 0.0f, 1.0f);
					prop->color = D3DCOLOR_COLORALPHA(prop->color_ini, fAlpha);
				}
			}
			break;
			case K_AI_STATE_ACTIVE_HEALTH_BOX:
			{
				int nHealthLeft = prop->varAIparams.GetVariantByName(L"n_healthLeft")->m_asINT32;
				prop->sprite.currentFrame = nHealthLeft;

				//fade out
				if (nHealthLeft <= 0)
				{
					prop->AItimer1 -= dTime;
					if (prop->AItimer1 <= 0.0f)
					{
						prop->bReleaseIt = true;
					}
					//color
					float fAlpha = LIMIT(prop->AItimer1, 0.0f, 1.0f);
					prop->color = D3DCOLOR_COLORALPHA(prop->color_ini, fAlpha);
				}
			}
			break;
			case K_AI_STATE_ACTIVE_TEAM_TELEPORTER_2FRAMES:
			{
			}
			break;

			case K_AI_STATE_ACTIVE_DOOR_SECTION:
			{
				prop->AItimer1 = 0.0f;
			}
			break;

			case K_AI_STATE_ACTIVE_DOORFACE_AUTOCLOSE:
			{
				//keep door open (AIvar1 contine frame-ul default) - set frame
				prop->sprite.currentFrame = prop->nFrame_ini;
				if (prop->AItimer1 > 0.0f)
				{
					prop->AItimer1 -= dTime;
					
					bool bDontChangeFrames = (bool)(prop->varAIparams.GetVariantByName(L"b_DontChangeFrames")->m_asINT32);
					if (!bDontChangeFrames)
					{
						prop->sprite.currentFrame++;
					}

					if (prop->AItimer1 < 0.0f)
						prop->AItimer1 = 0.0f;
				}

				//open/close sounds
				if ((prop->AIvarBool1 == false) && (prop->AItimer1 > 0.0f))
				{
					//just opened
					CVariantComplex* cvc = prop->varAIparams.GetVariantByName(L"s_openSnd");
					if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
					{
						int sndidx = UTGetSoundManager().getSndIdx(cvc->m_strArg.textHash);
						SND_PLAY_POSITIONAL(sndidx, prop->pos);
					}
					//on open script
					cvc = prop->varAIparams.GetVariantByName(L"s_ScriptOnOpen");
					if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
					{
						UTGetScriptManager().StartScript(cvc->m_strArg.textHash, prop->UID);
					}

					prop->AIvarBool1 = true;
				}
				else if ((prop->AIvarBool1 == true) && (prop->AItimer1 <= 0.0f))
				{
					//just closed
					CVariantComplex* cvc = prop->varAIparams.GetVariantByName(L"s_closeSnd");
					if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
					{
						int sndidx = UTGetSoundManager().getSndIdx(cvc->m_strArg.textHash);
						SND_PLAY_POSITIONAL(sndidx, prop->pos);
					}
					//on close script
					cvc = prop->varAIparams.GetVariantByName(L"s_ScriptOnClose");
					if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
					{
						UTGetScriptManager().StartScript(cvc->m_strArg.textHash, prop->UID);
					}
					prop->AIvarBool1 = false;
				}

			}
			break;

			case K_AI_STATE_ACTIVE_SWINGING_FRONTOBJ:
			{
				//implementare balans
				float fAng = prop->fAngle;
				float angDelta = prop->fAngle - prop->fAngle_ini;
				
				float fFriction = 0.4f;
				//ca sa se miste incet scot frecarea la viteze mici
				if (fabs(prop->AIfvar1) <= 0.04f)
					fFriction = 0.0f;
				prop->AIfvar1 -= angDelta * dTime * 20.0f + prop->AIfvar1 * dTime * fFriction;
				fAng += prop->AIfvar1 * dTime;
				CLAMP(fAng, prop->fAngle_ini - 1.4f, prop->fAngle_ini + 1.4f);
				
				prop->SetAngle(fAng);
			}
			break;

			case K_AI_STATE_ACTIVE_EXPLO_TRAP:
			{
			}
			break;
			case K_AI_STATE_ACTIVE_CHECKPOINT:
			{
				for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
				{
					if (pPlayerActor[kk] == null)
						continue;
					if (pPlayerActor[kk]->bbox.Intersects(&prop->bbox))
					{
						prop->Touch(pPlayerActor[kk]->GetUID(), dTime);
						//save checkpoint
						vLastSpawnPoint = prop->pos;
						break;
					}
				}
			}
			break;
			default:
			{
				if (!UpdateAI_base(prop, dTime, prop->fTimelineAI))
				{
					ErrorBox(K_ERR_WARNING, L"CLevel::UpdateAI_active - AIstate not handled: %d", prop->AIstate);
				}
			}
			break;
		}
	}

	prop->SetPos(prop->pos);
	//update-uri finale
	prop->sprite.pos = prop->pos;
	prop->sprite.color = prop->color;
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
	_ASSERT((pWeapon != null) && (pActor != null));

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
			Vec2 vPos = actor->GetPosHeart();
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
				Vec2 vDelta = tact->GetPosHeart() - actor->GetPosHeart();
				float fDist = MUVec2Len(&vDelta);
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
					GiveStrategicPoints(actor->templateActor.fStrategicPoints, &Vec2(actor->bbox.vCenter.x, actor->bbox.vMin.y));
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
CFixedArray<SweepAABB, 100> tempCollBoxList;

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
	Vec2 vAnimMove(0.0f, 0.0f);
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
	


	///--- ACTOR CAPS ---
	//--- find closest touchable ---
	if (actor->templateActor.eCaps & CActorTemplate::K_ACT_CAPS_CAN_INTERACT)
	{
		IActiveInterface* pLowPrioTouch = null;
		actor->pClosestTouchable = null;
		//find the active that has the biggest bbox intersection surface with our player
		float fSurface = 0.0f;
		for (int kk = 0; kk < m_arrPropsPtrInteract.Count(); kk++)
		{
			CProp* activ = m_arrPropsPtrInteract.m_pData[kk];
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
				float enemyDst = MUVec2Len(&(targetActor->posHeart - actor->posHeart));
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
					AddAIEvent(K_LVL_AI_EVENT_LOST_ENEMY, 0, K_LVL_ACT_CLASS_ANY, actor->posHeart + Vec2(16.0f * actor->lookDirXsign, 0.0f), 16.0f, 1.0f, actor->GetUID());
					//reset targeting actor
					actor->m_AIsensorInfo.pTargetedActor = null;
				}

				//#HACK: uneori e lovit dar nu apuca sa vada inamicul si ramane blocat ca nu primeste LOST_ENEMY asa ca il trimitem acum
				if ((actor->m_AIsensorInfo.pTargetedActor == null) && (actor->m_AIsensorInfo.m_AIlastEvent.nType == K_LVL_AI_EVENT_GOT_HIT))
				{
					//put event behind him
					AddAIEvent(K_LVL_AI_EVENT_LOST_ENEMY, 0, K_LVL_ACT_CLASS_ANY, actor->posHeart - Vec2(16.0f * actor->lookDirXsign, 0.0f), 16.0f, 0.5f, actor->GetUID());
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
				actor->m_AIsensorInfo.m_AIcurrentEvent.Set(K_LVL_AI_EVENT_IDLE_TICK, 0, 0, Vec2(0.0f, 0.0f), -1.0f, 1.0f);
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

				Vec2 vMoveDir = pController->GetDoubleAxisVector(K_CM_COMMAND_MOVE_X, K_CM_COMMAND_MOVE_Y, true);
				if (MUVec2LenSq(&vMoveDir) > 0.0f)
				{
					actor->m_AIcommands.bThrust = true;
					actor->m_AIcommands.vMoveDir = vMoveDir;
					actor->m_AIcommands.bRunning = true;
				}
				Vec2 vAimVec = pController->GetDoubleAxisVector(K_CM_COMMAND_AIM_X, K_CM_COMMAND_AIM_Y, false);
				//DebugPrintA("aim: %.2f, %.2f\n", vAimVec.x, vAimVec.y);
				actor->m_AIcommands.vAimVec = vAimVec;

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
								Vec2 ppos(actor->bbox.vMin.x + randfloat(actor->bbox.vSize.x), actor->bbox.vMin.y + randfloat(actor->bbox.vSize.y));
								g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRESPARK1, true, randint(2), &ppos, NULL, &Vec2(randfloatsgn(1.0f), -10.0f - randfloat(5.0f)), 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.5f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
								ppos = Vec2(actor->bbox.vMin.x + randfloat(actor->bbox.vSize.x), actor->bbox.vMin.y + randfloat(actor->bbox.vSize.y));
								g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRESPARK2, true, 0, &ppos, NULL, &Vec2(randfloatsgn(1.0f), -10.0f - randfloat(5.0f)), 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.5f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
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

						Vec2 vDelta = actor->m_AIsensorInfo.pTargetedActor->GetPosHeart() - actor->GetPosHeart();
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
					AddDoofer_Explo(cvc->m_asUINT32, actor->posHeart, unExploUID, K_LVL_ACT_CLASS_EXPLOSION, Vec2(0.0f, 0.0f), &actor->bbox);

					//decal explo mark
					//AddDecal(K_LVL_DECAL_LAYER_BACKWALLS, actor->posHeart, ANM_ACTIVES_SPR_DECAL_EXPLOMARKS, randint(3), 0xffffffff);
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
									g_particlesMgr.GenerateBulletHitEnemy(Vec2(actor->posHeart.x + randfloatsgn(5.0f), actor->posHeart.y),
										Vec2((float)randsign(), -1.0f), K_LVL_ACT_CLASS_HUMAN, K_PART_LAYER_RT_FRONT_NRM);
								}
								else
								{
									//on gore off generate some stars
									g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_CROSS_SM, true, 0, &Vec2(actor->posHeart.x + randfloatsgn(8.0f), actor->posHeart.y), NULL,
										&Vec2(0.0f, -30.0f - randfloat(10.0f)), 0.6f, 0.7f, 0.0f, 0.0f, 0.0f, 0.2f, 0.2f, 0xff32a7fa, K_PART_LAYER_RT_FRONT_NRM);

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
					CVariantComplex* cvc2 = actor->varAIparams.GetVariantByName(L"sDeathScript");
					if (cvc2->m_type == CVariantComplex::K_ARGTYPE_STRING)
					{
						StartScript(cvc2->m_strArg.text, actor);
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
	_ASSERT(actor->pCurrentWeapon != null);
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
					Vec2 vTeleportPos = pOther->pos;
					if (GetBestSpawningPos(&vTeleportPos, pOther->bbox, &pOther->bbox))
					{
						actor->SetPos(vTeleportPos);
						//animate player on spawn (only if told otherwise by nAnimset=-1)
						actor->SetAnimSet(0);
						SetActorAIState(actor, L"JOIN_GAME");
						//AddProp_Light(actor->GetPosHeart(), ANM_LIGHTS_SPR_POINT1, 0.5f, 0.1f, 0x8888ff00, 1.0f);
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
			CAABB camAABB(Vec2(camrect.x, camrect.y), Vec2(camrect.Right(), camrect.Bottom()));

			camrect.Inflate(-16.0f);
			//players midpoint
			Vec2 avg(0.0f, 0.0f);
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
				Vec2 vChkPos = actor->GetPosHeart();
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
				Vec2 vShootDir;
				if (actor->m_AIcommands.vAimVec.x != 0.0f)
					vShootDir = actor->m_AIcommands.vAimVec;
				else
					vShootDir = Vec2(actor->lookDirXsign, 0.0f);

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

					Vec2 vShootDir;
					if (actor->m_AIcommands.vAimVec.x != 0.0f)
						vShootDir = actor->m_AIcommands.vAimVec;
					else
						vShootDir = Vec2(actor->lookDirXsign, 0.0f);

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
		actor->speed = Vec2(0.0f, 0.0f);
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
					Vec2 bulletSpeed;
					MUVec2Norm(&bulletSpeed, &actor->vSpeedImpulse);

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
							AddDoofer(K_DOOFER_MEAT, AABB_GetRandomPointInBox(genbox), &Vec2(randfloatsgn(50.0f) + bulletSpeed.x * 50.0f, -130.0f - randfloat(100.0f)), &g_vecGravityOld, nSubType);
						}
						//goes straight down to stain the floor
						AddDoofer(K_DOOFER_MEAT, actor->GetPosHeart(), &Vec2(200.0f, 50.0f), &g_vecGravityOld, nSubType);
						AddDoofer(K_DOOFER_MEAT, actor->GetPosHeart(), &Vec2(-200.0f, 50.0f), &g_vecGravityOld, nSubType);
						//human blood gibs particle
						g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_HUMAN_SPLAT_MED, true, 0, &actor->pos, NULL, NULL, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, dwCol, K_PART_LAYER_RT_FRONT_NRM);
					}
					else //small animals and stuff
					{
						for (int ll = 0; ll < 2; ll++)
						{
							AddDoofer(K_DOOFER_MEAT, AABB_GetRandomPointInBox(genbox), &Vec2(randfloatsgn(50.0f) + bulletSpeed.x * 50.0f, -130.0f - randfloat(100.0f)), &g_vecGravityOld);
						}
						g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_HUMAN_SPLAT_SMALL, true, 0, &actor->pos, NULL, NULL, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xff671010, K_PART_LAYER_RT_FRONT_NRM);
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
					actor->speed = Vec2(0.0f, 0.0f);
					actor->vSpeedImpulse = Vec2(0.0f, 0.0f);
					actor->vecCamFollowPos = Vec2(0.0f, 0.0f);
					//move invisible body back to last safe pos
					Vec2 vSpawnPos = m_arrPlayerLastSafePos[actor->nPlayerOrdinal];
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
	SetActorAnimationOnce(actor, K_LVL_ACT_ANIM_IDLE, K_LVL_ACT_ANIM_FEET_IDLE);
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

	//------------------------------------------------------------------------------------------
	//	INTEGRATOR - physics
	//------------------------------------------------------------------------------------------
	//#TODO: check speed limits - should be done on the speed vector, normalized
	CLAMP(actor->speed.x, -K_LVL_ACTOR_MAX_SPEED, K_LVL_ACTOR_MAX_SPEED);
	CLAMP(actor->speed.y, -K_LVL_ACTOR_MAX_SPEED, K_LVL_ACTOR_MAX_SPEED);
	//update impulse
	Vec2 impFriction(K_LVL_GROUND_DEFAULT_FRICTION, K_LVL_GROUND_DEFAULT_FRICTION);
	//limit impulse
	CLAMP(actor->vSpeedImpulse.y, -K_LVL_ACTOR_MAX_IMPULSE, K_LVL_ACTOR_MAX_IMPULSE);
	CLAMP(actor->vSpeedImpulse.x, -K_LVL_ACTOR_MAX_IMPULSE, K_LVL_ACTOR_MAX_IMPULSE);

	actor->vSpeedImpulse.x -= actor->vSpeedImpulse.x * impFriction.x * dTime;
	actor->vSpeedImpulse.y -= actor->vSpeedImpulse.y * impFriction.y * dTime;
	
	Vec2 vPosIni = actor->pos;
	UINT16 unCollFlags = 0;

	{
		///a.calculezi vectorul de miscare al actorului(viteza * dt + miscare paltforma daca e necesar)
		Vec2 vNextMove = (actor->speed + actor->vSpeedImpulse) * dTime; // Add connected platform movement if needed
		///b.detectezi coliziuni posibile(bbox old + new pos)
		//1. find bbox start and end union that includes all collisions when moving at high speeds
		CAABB destbox, srcbox;
		srcbox = actor->bbox_ini; srcbox.Move(actor->pos);
		destbox = actor->bbox_ini; destbox.Move(actor->pos + vNextMove);
		// box unions to check all possible collisions
		CAABB boxUnion = AABB_Union(destbox, srcbox);
		// bbox union in tile coords, including every touched tile
		RECTXYXY boxUnionTiles(floor(boxUnion.vMin.x / K_TILE_SIZE_F), floor(boxUnion.vMin.y / K_TILE_SIZE_F),
			ceil(boxUnion.vMax.x / K_TILE_SIZE_F), ceil(boxUnion.vMax.y / K_TILE_SIZE_F));
		//optional - to include more of the boxes
		//boxUnion.Inflate(K_TILE_HSIZE, K_TILE_HSIZE);

		// keeps a list of all boxes that might be colliding
		tempCollBoxList.Clear();

		/// BROAD PHASE SWEEP (find all POSSIBLE collision objects)

		//add boxes from collision shapes
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
				tempCollBoxList.Add(m_arrColShapes[kk]->bbox);
			}
		}
		//add boxes from tiles
		for (int yy = boxUnionTiles.y1; yy <= boxUnionTiles.y2; yy++)
		{
			for (int xx = boxUnionTiles.x1; xx <= boxUnionTiles.x2; xx++)
			{
				CTile* tl = &tiles[xx][yy];
				if ((tl->flags & K_TILEFLAG_WALKABLE) == 0)
				{
					tempCollBoxList.Add(tl->bbox);
				}
			}
		}

		/// COLLISION HANDLING

		float fRemainingTime = 1.0f;
		while (fRemainingTime > 0.0f)
		{
			// compute source box
			srcbox = actor->bbox_ini; srcbox.Move(actor->pos);
			// find closest collider
			float minDistSq = 100000.0f;
			float fClosestTime = 100000.0f;
			SweepAABB* pClosestBox = nullptr;
			for (int kk = 0; kk < tempCollBoxList.Count(); kk++)
			{
				SweepAABB* tmpbox = &tempCollBoxList[kk];
				// skip boxes that have been handled this step
				if (tmpbox->bDisabled)
					continue;

				SweepData sdata = AABBSweep::CalculateSweepData(srcbox, vNextMove, *tmpbox);
				// computes even if no valid collision. needs flag to eliminate them
				if (sdata.bIsValid == false)
					continue;

				if (sdata.fCollisionTime < fClosestTime)
				{
					fClosestTime = sdata.fCollisionTime;
					minDistSq = sdata.fDistance;
					pClosestBox = tmpbox;
				}
				else if (sdata.fCollisionTime == fClosestTime)
				{
					if (sdata.fDistance < minDistSq)
					{
						fClosestTime = sdata.fCollisionTime;
						minDistSq = sdata.fDistance;
						pClosestBox = tmpbox;
					}
				}
			}

			// do we have a collider?
			if (pClosestBox != nullptr)
			{
				SweepData hit = AABBSweep::CalculateSweepData(srcbox, vNextMove, *pClosestBox);
				//handled already, disable it
				pClosestBox->bDisabled = true; 

				actor->pos += vNextMove * hit.fCollisionTime;

				// Calculate the correct time of impact for the remaining
				// collisions or to apply movement
				float ftime = fRemainingTime - hit.fCollisionTime;

				// Calculate the collision normal (vector used to slide the object that collided)
				// normala e tangenta de fapt...
				float dotProduct = MUVec2Dot(&vNextMove, &hit.vNormal) * ftime;
				hit.vNormal *= dotProduct;

				// Handle events after each respective side that collided
				//DMC: could implement actor->OnCollision(hit.eSide) if needed
				switch (hit.eSide)
				{
					case K_SIDE_BOTTOM:
					{
						actor->speed.y = 0.0f;
						actor->vSpeedImpulse.y = 0.0f;
						unCollFlags |= K_DIRFLAG_DOWN;
					}
					break;
					case K_SIDE_TOP:
					{
						actor->speed.y = 0.0f;
						actor->vSpeedImpulse.y = 0.0f;
						unCollFlags |= K_DIRFLAG_UP;
					}
					break;
					case K_SIDE_LEFT:
					{
						actor->speed.x = 0.0f;
						actor->vSpeedImpulse.x = 0.0f;
						unCollFlags |= K_DIRFLAG_LEFT;
					}
					break;
					case K_SIDE_RIGHT:
					{
						actor->speed.x = 0.0f;
						actor->vSpeedImpulse.x = 0.0f;
						unCollFlags |= K_DIRFLAG_RIGHT;
					}
					break;
				}

				if (ftime > 0.0f)
				{
					vNextMove = hit.vNormal;

					CAABB newboxsrc = actor->bbox_ini;
					newboxsrc.Move(actor->pos);
					CAABB newboxdest = newboxsrc;
					newboxdest.Move(vNextMove);
					CAABB newBoundary = AABB_Union(newboxsrc, newboxdest);

					// call and implement this if you need tile sized boxes to enter tile wide holes
					//this.fixEqualSizedHoleCollision(hit, potential, time, collisionStack);

					//DMC: deactivate those boxes that don't fit the new boundary
					for (int kk = 0; kk < tempCollBoxList.Count(); kk++)
					{
						SweepAABB* it = &tempCollBoxList.m_pData[kk];
						if (it->bDisabled)
							continue;
						// disable non intersecting ones
						if (!newBoundary.Intersects(it))
							it->bDisabled = true;
					}
					// update remaining time and do again
					fRemainingTime = ftime;
				}

			}
			else
			{
				actor->pos += vNextMove;
				fRemainingTime = 0.0f;
			}
		}

		//#TODO: could use a penetration resolution round. Maybe after solving each collision so we make sure boxes don't actually touch? TBD
	}

	actor->collisionFlags = unCollFlags;

	//check world bounds for each actor - kill if out
	if (!PointInRect(actor->pos, m_levelAABB))
	{
		KillActor(actor);
	}

	//set final position
	actor->SetPos(actor->pos);
	// save last position in pos_last (SetPos does but we already altered actor->pos)
	actor->pos_last = vPosIni;

	//#TODO: Speeds and accelerations should be treated here, after the collision detection
		

	//end phys

	//set camera vector
	if (actor->templateActor.actorClass == K_LVL_ACT_CLASS_PLAYER)
	{
		actor->vecCamFollowPos = Vec2(K_LVL_CAM_LOOK_OFFSET * actor->lookDirXsign, 0.0f);
		if (actor->nAttackStatus == K_LVL_ACT_ATTACK_SHOOTING)
			actor->vecCamFollowPos.x += actor->lookDirXsign * actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->WeaponTemplate.fCameraRecoil;
	}
	//set sprite pos
	if (!actor->templateActor.bComposedAnimation)
	{
		actor->sprite.pos = Vec2((int)ROUND_FLOAT(actor->pos.x), (int)ROUND_FLOAT(actor->pos.y));
		actor->sprite_feet.pos = actor->sprite.pos;
	}
	else
	{
		actor->sprite_feet.pos = Vec2((int)ROUND_FLOAT(actor->pos.x), (int)ROUND_FLOAT(actor->pos.y));
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
		Vec2(sourceActor->pos.x + sourceActor->lookDirXsign * fDistSee, sourceActor->pos.y + fDistDown),
		Vec2(sourceActor->pos.x - sourceActor->lookDirXsign * fDistHear, sourceActor->pos.y - fDistUp)
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

		Vec2 enemyDistV = enemy->posHeart - sourceActor->posHeart;
		float viewDstSq = sourceActor->templateActor.distSee * sourceActor->templateActor.distSee;
		float enemyDistSq = MUVec2LenSq(&enemyDistV);

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
			if ((sourceActor->fFOVPercent < 1.0f) && (UTMath::GetAngleBetweenVectors(enemy->posHeart - sourceActor->posHeart, sourceActor->vAngleDir) > (HALF_PI * sourceActor->fFOVPercent)))
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

		//check smoke grenades
		bool bObscured = false;
		for (int ll = 0; ll < m_arrBulletsTemp.Count(); ll++)
		{
			CBullet* bul = m_arrBulletsTemp.m_pData[ll];
			if (bul->eType != K_LVL_BULLET_SMOKE_GRENADE)
				continue;
			Vec2 vBulPos = bul->physPt->m_data.pos;
			
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

		Vec2 enemyDistV = enemy->posHeart - sourceActor->posHeart;
		float enemyDistSq = MUVec2LenSq(&enemyDistV);
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
		if ((retvalenemy == null) || (MUVec2LenSq(&(retvalenemy->posHeart - sourceActor->posHeart)) > enemyDistSq))
			retvalenemy = enemy;
	}

	return retvalenemy;
}


CCollisionShape* CLevel::GetClosestCover(Vec2 vPos, float fMaxDistance /*= 0.0f*/)
{
	float fMaxDstSq = fMaxDistance * fMaxDistance;
	float fCurrentDist = 0.0f;
	CCollisionShape* pRetShape = null;
	for (int kk = 0; kk < m_arrColShapes.GetSize(); kk++)
	{
		CCollisionShape* shape = m_arrColShapes[kk];
		if (shape->type != K_LVL_COLL_TYPE_COVER)
			continue;
		float fDstSq = MUVec2LenSq(&(shape->bbox.vCenter - vPos));
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

void CLevel::AddAIEvent(EAIEventType eventType, UINT32 ownerUID, int ownerClass, Vec2 vPos, float radius, float duration, UINT32 targetUID)
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
			evtdstsq = MUVec2LenSq(&(callerActor->posHeart - evt->pos));
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
			_ASSERT(active->varAIparams.GetVariantByName(L"emitterPtr")->m_type == CVariantComplex::K_ARGTYPE_NONE);

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
	for (int kk = m_arrProps.GetSize() - 1; kk >= 0; kk--)
	{
		if (m_arrProps[kk]->bReleaseIt)
		{
			SAFE_DELETE(m_arrProps[kk]);
			m_arrProps.Remove(kk);
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

	//check lights
	for (int kk = 0; kk < m_arrLights.GetSize(); kk++)
	{
		if (m_arrLights[kk]->IsPendingKill())
		{
			SAFE_DELETE(m_arrLights[kk]);
			m_arrLights.Remove(kk);
		}
	}
}

void CLevel::UpdateAI(float dTime, bool bInEditor)
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
	m_arrPropsPtrInteract.Clear();
	for (int kk = m_arrProps.GetSize() - 1; kk >= 0; kk--)
	{
		UpdateAI_prop(m_arrProps[kk], dTime);
		//add interactible?
		if (m_arrProps[kk]->bCanInteract)
			m_arrPropsPtrInteract.Add(m_arrProps[kk]);
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

	//check actors - must be done after moving platforms (usually last is best)
	double fHashKey = 0.0f;
	for (int kk = m_arrActors.GetSize() - 1; kk >= 0; kk--)
	{
		CActor* act = m_arrActors[kk];
		if (!bInEditor)
		{
			UpdateAI_actor(act, dTime);
		}
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
						if ((ctrlr->eType == K_CM_CT_JOYSTICK_SDL) && (!UTGetAppClass().IsGameNetworked()) && (false == UTGetGUI().bIsBlocking) && 
							(ctrlr->sCommands.keyState[K_CM_COMMAND_SELECT] == K_CM_BUTSTATE_JUSTPRESSED))
						{
							UTGetGUI().ShowLayerOnce("LAYER_ID_CONTROLLER_MAP");
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
								Vec2 vSpawnPos = m_arrPlayerLastSafePos[plidx];
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
				CCtrlLayer* layer = UTGetGUI().GetTopmostInputLayer();
				if (layer)
				{
					CControl* ctrl;
					if ((ctrl = layer->GetControlByName("CTRL_NETVOTE_RESTART")) != nullptr)
					{
						ctrl->paramsDict.SetNamedVarINT32(L"leftVote", (g_netlock.m_arrLvlResPeerStates[0] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_RESTART) ? 1 : 0);
						ctrl->paramsDict.SetNamedVarINT32(L"rightVote", (g_netlock.m_arrLvlResPeerStates[1] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_RESTART) ? 1 : 0);
					}
					if ((ctrl = layer->GetControlByName("CTRL_NETVOTE_CONTINUE")) != nullptr)
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
						UTLang().SetString(STR_MISSION_P1_KILLS, L"%d", m_arrStats[K_LVL_STATS_PL1_KILLS]);
						if (m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT] > 0)
							UTLang().SetString(STR_MISSION_P1_ACCURACY, L"%.1f%%", 100.0f * fAccuracyP1);
						else
							UTLang().SetString(STR_MISSION_P1_ACCURACY, L"%s", UTLang().strings[STR_NOT_AVAILABLE]->sText);
						UTLang().SetString(STR_MISSION_P1_HOSTAGES, L"%d / %d", m_arrStats[K_LVL_STATS_PL1_HOSTAGES_SAVED], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);
						UTLang().SetString(STR_MISSION_P1_DEATHS, L"%d", m_arrStats[K_LVL_STATS_PL1_DEATHS]);
						//--- PL2 data ---
						float fAccuracyP2 = 1.0f;
						if (m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT] > 0)
							fAccuracyP2 = (float)m_arrStats[K_LVL_STATS_PL2_BULLETS_HIT] / (float)m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT];
						CLAMP(fAccuracyP2, 0.0f, 1.0f);
						UTLang().SetString(STR_MISSION_P2_KILLS, L"%d", m_arrStats[K_LVL_STATS_PL2_KILLS]);
						if (m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT] > 0)
							UTLang().SetString(STR_MISSION_P2_ACCURACY, L"%.1f%%", 100.0f * fAccuracyP2);
						else
							UTLang().SetString(STR_MISSION_P2_ACCURACY, L"%s", UTLang().strings[STR_NOT_AVAILABLE]->sText);
						UTLang().SetString(STR_MISSION_P2_HOSTAGES, L"%d / %d", m_arrStats[K_LVL_STATS_PL2_HOSTAGES_SAVED], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);
						UTLang().SetString(STR_MISSION_P2_DEATHS, L"%d", m_arrStats[K_LVL_STATS_PL2_DEATHS]);

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
						UTLang().SetString(STR_MISSION_TIME, tmpstr);
						UTLang().SetString(STR_MISSION_CASUALTIES, L"%d", m_arrStats[K_LVL_STATS_PL1_DEATHS] + m_arrStats[K_LVL_STATS_PL2_DEATHS]);
						UTLang().SetString(STR_MISSION_SCORE, L"%d", nTotalLevelScore);

						int nHostagesSaved = m_arrStats[K_LVL_STATS_PL1_HOSTAGES_SAVED] + m_arrStats[K_LVL_STATS_PL2_HOSTAGES_SAVED];
						UTLang().SetString(STR_MISSION_HOSTAGES, L"%d / %d", nHostagesSaved, m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);

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
						UTGetGUI().RemoveAllLayers();
						//generic changes
						CCtrlLayer *layer = null;
						if (nPlayers == 1)
							layer = UTGetGUI().ShowLayerOnce("LAYER_ID_LEVELWIN_1P");
						else
						{
							if (!UTGetAppClass().IsGameNetworked())
								layer = UTGetGUI().ShowLayerOnce("LAYER_ID_LEVELWIN_2P");
							else
								layer = UTGetGUI().ShowLayerOnce("LAYER_ID_LEVELWIN_2P_COOP");
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
							UTLang().SetString(STR_LEADERBOARDS_NAMES_VAL, L"...");
							UTLang().SetString(STR_LEADERBOARDS_SCORES_VAL, L"...");
							UTLang().SetString(STR_LEADERBOARDS_PLAYERSCORE_VAL, L"...");
							//now upload score
							UTGetLeaderboards().QueueJob(K_JOB_UPLOAD_SCORE, pszBoardName, nTotalLevelScore);
							//request downloading of scores
							UTGetLeaderboards().QueueJob(K_JOB_GET_SCORES_AROUND_USER, pszBoardName);
							//request downloading of your own score - only if needed (when leaderboards don't update instantly)
							//UTGetLeaderboards().QueueJob(K_JOB_GET_SCORE_FOR_CURRENT_USER, pszBoardName, 0);
						}
#endif

						if (layer != null)
						{
							CControl* ctrltop = null;
							if ((ctrltop = layer->GetControlByName("CTRL_STARS")) != nullptr)
							{
								ctrltop->paramsDict.SetNamedVarINT32(L"nStars", nStars);
							}
							//red labels for conditions that aren't satisfied						   
							if (m_arrStats[K_LVL_STATS_HOSTAGES_KILLED] > 0)
							{
								if ((ctrltop = layer->GetControlByName("LABEL_HOSTAGES")) != nullptr)
								{
									ctrltop->paramsDict.SetNamedVarString(L"fontColor", L"0xffff0000");
								}
							}
							if (m_arrStats[K_LVL_STATS_PL1_DEATHS] + m_arrStats[K_LVL_STATS_PL2_DEATHS] > 0)
							{
								if ((ctrltop = layer->GetControlByName("LABEL_CASUALTIES")) != nullptr)
								{
									ctrltop->paramsDict.SetNamedVarString(L"fontColor", L"0xffff0000");
								}
							}

							//on custom downloaded levels hide the MELEE-leaderboards 
							if (m_unLoadedLevelFlags & K_LVL_LEVEL_FLAG_DOWNLOADED)
							{
								if ((ctrltop = layer->GetControlByName("LABEL_LEADERBOARDS")) != nullptr)
									ctrltop->paramsDict.SetNamedVarString(L"fontColor", L"0x00000000");
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
						CCtrlLayer* lay = UTGetGUI().GetLayerByName("LAYER_ID_LEADERBOARDS_IGM");
						if (lay == null)
						{
							//show layer
							lay = UTGetGUI().ShowLayerOnce("LAYER_ID_LEADERBOARDS_IGM");
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
										UTLang().SetString(STR_TEMP10, L"%d.%d %s", nChapter + 1, nLevel + 1, UTLang().strings[nStrIdxLevelName]->sText);
									else
										UTLang().SetString(STR_TEMP10, L"%d.%d", nChapter + 1, nLevel + 1);
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
				CCtrlLayer* layer = UTGetGUI().GetTopmostInputLayer();
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
						UTLang().SetString(STR_MISSION_P1_KILLS, L"%d", m_arrStats[K_LVL_STATS_PL1_KILLS]);
						if (m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT] > 0)
							UTLang().SetString(STR_MISSION_P1_ACCURACY, L"%.1f%%", 100.0f * fAccuracyP1);
						else
							UTLang().SetString(STR_MISSION_P1_ACCURACY, L"%s", UTLang().strings[STR_NOT_AVAILABLE]->sText);
						UTLang().SetString(STR_MISSION_P1_HOSTAGES, L"%d / %d", m_arrStats[K_LVL_STATS_PL1_HOSTAGES_SAVED], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);
						UTLang().SetString(STR_MISSION_P1_DEATHS, L"%d", m_arrStats[K_LVL_STATS_PL1_DEATHS]);
						//--- PL2 data ---
						float fAccuracyP2 = 1.0f;
						if (m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT] > 0)
							fAccuracyP2 = (float)m_arrStats[K_LVL_STATS_PL2_BULLETS_HIT] / (float)m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT];
						CLAMP(fAccuracyP2, 0.0f, 1.0f);
						UTLang().SetString(STR_MISSION_P2_KILLS, L"%d", m_arrStats[K_LVL_STATS_PL2_KILLS]);
						if (m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT] > 0)
							UTLang().SetString(STR_MISSION_P2_ACCURACY, L"%.1f%%", 100.0f * fAccuracyP2);
						else
							UTLang().SetString(STR_MISSION_P2_ACCURACY, L"%s", UTLang().strings[STR_NOT_AVAILABLE]->sText);
						UTLang().SetString(STR_MISSION_P2_HOSTAGES, L"%d / %d", m_arrStats[K_LVL_STATS_PL2_HOSTAGES_SAVED], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);
						UTLang().SetString(STR_MISSION_P2_DEATHS, L"%d", m_arrStats[K_LVL_STATS_PL2_DEATHS]);

						///--- XP Points ---
						int nXPpl1 = 0, nXPpl2 = 0;
						int nMaxXPPoints = App_GetMaxXP(K_GAME_MAX_UPGRADE_LEVELS);
						int nTotalXPPoints = Local_ComputeMissionXP(0);

						//--- STARS WINDOW ---
						int nTimeSpent = m_arrStats[K_LVL_STATS_LEVEL_END_SEC] - m_arrStats[K_LVL_STATS_LEVEL_START_SEC];
						OS_FormatTime(tmpstr, MAX_PATH, (float)(nTimeSpent));
						UTLang().SetString(STR_MISSION_TIME, tmpstr);

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


						//--- show windows and change portraits and title text ---
						if (nPlayers == 1)
						{
							UTGetGUI().RemoveAllLayers();
							CCtrlLayer* layer = UTGetGUI().ShowLayerOnce("LAYER_ID_LEVELFAIL_1P");
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

							if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
							{
								CHAR ctxt[MAX_PATH];
								StringCchPrintfA(ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1);
								ANALYTICS_EVENT("level_lose_1p", ctxt, "durationSec", nTimeSpent);
							}
						}
						else //2 players
						{
							UTGetGUI().RemoveAllLayers();

							CCtrlLayer* layer = null;
							if(!UTGetAppClass().IsGameNetworked())
								layer = UTGetGUI().ShowLayerOnce("LAYER_ID_LEVELFAIL_2P");
							else
								layer = UTGetGUI().ShowLayerOnce("LAYER_ID_LEVELFAIL_2P_COOP");

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

							}
						}
						// notify level finished for achievements
						if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
							UTGetAppClass().App_OnLevelFinished(g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL]);
					}
				}
				break;
				default:
					break;
			}
		}
		break;

	}

	// updates dirty rects (tileset and more)
	UpdateDirtyRects();

	//clear poly buffers first
	m_bufferedPainter.ClearBuffers();

	///--- PHYSICS POINTS ---
	UpdatePhysicsPoints(dTime);
	///--- BULLETS (after phys pts) ---
	UpdateBullets(dTime);
	///--- PROPS ---
	UpdateDoofers(dTime);
	///--- DECALS ---
	UpdateDecals(dTime);
	///--- ACTIVES ---
	UpdateAI(dTime, g_editor.IsLaunched());

	///--- release dead objects all at once here ---
	//(called before BuildVisibilityLists but after bullets,physics updates because it deallocates stuff from visibility lists)
	CleanupDeadObjects();

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
	Vec2 avg_live(0.0f, 0.0f), avg_all(0.0f, 0.0f);
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
	Vec2 vPlayersAvg(0.0f, 0.0f);
	if (plcnt_all > 0)
	{
		bAvgSet = true;

		avg_all /= plcnt_all;
		vPlayersAvg = avg_all;

		if (plcnt_live > 0)
		{
			avg_live /= plcnt_live;
			//are they too far apart? 
			if (MUVec2Len(&(avg_all - avg_live)) > UTGetAppClass().g_rectGameScreen.h * 0.5f)
			{
				vPlayersAvg = avg_live;
			}
		}
	}

	//handles render size changes
	m_camLevel.SetViewport(UTGetAppClass().g_rectRT); 
	if (g_editor.IsLaunched())
	{
		m_camLevel.SetCamPos(&g_editor.m_vCamPos);
	}
	else
	{
		//no target camera object? look at the player pos average
		if (m_camTargetActive == null)
		{
			if (bAvgSet)
				m_vCamPosDefault = vPlayersAvg;

			m_camLevel.SetCamPos(&m_vCamPosDefault);
		}
		else
		{
			m_camLevel.SetCamPos(&(m_camTargetActive->pos));
		}
	}

	m_camLevel.Update(dTime);

	//find visible area
	RECTXYWH_F camrect = m_camLevel.GetCamWorldAABB();
	CAABB camAABB(Vec2(camrect.x, camrect.y), Vec2(camrect.Right(), camrect.Bottom()));

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
	CAABB visibleAABB(m_visibleArea);
	///--- update visibility lists (after update) ---
	BuildVisibilityLists();

	// builds all dynamic meshes necessary for drawing the next frame
	BuildDynamicGeometry(camAABB);

	///--- update interface ---
	m_interfaceIGM.Update(dTime);
	m_interfaceTextBubble.Update(dTime);

	//set update done flag
	m_bOneUpdateDone = true;
}

HRESULT CLevel::PaintOffscreen()
{
//	if ((!m_bLoaded) || (!m_bOneUpdateDone))
//		return E_FAIL;
//
//	RECTXYWH_F camrect = m_camLevel.GetCamWorldAABB();
//	//CAABB al camerei
//	CAABB		camAABB;  
//	camAABB.Set(Vec2(camrect.x, camrect.y), Vec2(camrect.Right(), camrect.Bottom()));
//	//matrice folosita local
//	Mat matlocal;
//
//	HRESULT hr = S_OK;
//	//daca nu am capabilitatea de offscreen ies cu eroare
//	if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_RTT) == 0)
//		return E_FAIL;
//
//	hr = S_OK;
//	if (SUCCEEDED(hr))
//	{
//		// Clear the render target and the zbuffer 
//		V(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET , K_GAME_CLEAR_COLOR, 1.0f, 0));
//
//		m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
//		m_pDevice->SetTransform(D3DTS_VIEW, &g_matIdentity);
//		//matrice de proiectie offsetata ca sa incapa un pixel intreg (pixel center e in centru)
//		Mat matProj;
//		D3DXMatrixOrthoOffCenterLH(&matProj, 0.5f, K_RTT_WIDTH + 0.5f, K_RTT_HEIGHT + 0.5f, 0.5f, 0.0f, 1.0f);
//		m_pDevice->SetTransform(D3DTS_PROJECTION, &matProj);
//
//		m_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
//		//use sprite
//		m_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_OBJECTSPACE | D3DXSPRITE_DONOTSAVESTATE);
//
//		m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
//		m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
//		m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
//
//		m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
//		m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
//		m_pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
//		//#HACK:cand am alpha pe jumatate rezultatul blendingului pe alpha ar iesi si el pe 0 deci in schimba alpha pe mai mica
//		//asta inseamna ca daca am chestii semitransparente imi modifica alpha finala a render targetului
//		//#TODO: aici ar trebui ca umbrele obiectelor sa fie facute din normal map cumva ca sa nu mai am nevoie de separate alpha blending
//		//#TODO: totusi daca am tiles semitransparente pe layer din fatza imi apare aiurea pe cel din spate daca are semitransparenta sau nu e activat alphatest.
//		if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0)
//		{
//			m_pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);  //????? - este necesara dar nu e foarte bine suportata de multe placi
//			m_pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA);
//			m_pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_DESTALPHA);
//			m_pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
//		}
//		//set scroll matrix
//		Mat mattrans;
//		MUMatAffine2D(&mattrans, 1.0f, NULL, 0.0f, &Vec2(-m_visibleArea.x, -m_visibleArea.y));
//		m_pSprite->SetTransform(&mattrans);
//
//		///.////////////////////////////////////////////////////////
//		///	COLOR MAP
//		///.////////////////////////////////////////////////////////
//
//		CAABB aabbScissor;
//		aabbScissor.Set(0.0f, 0.0f, (float)K_RTT_H_WIDTH, (float)K_RTT_H_HEIGHT);
//		//set clip on colormap
//		if ((m_bInsideHiddenRoom) && (m_HiddenRoomAABB.vSize.x > 0.0f) && (m_HiddenRoomAABB.vSize.y > 0.0f))
//		{
//			CAABB aabbVisible;
//			aabbVisible.Set(m_visibleArea);
//			AABB_Intersection(aabbVisible, m_HiddenRoomAABB, aabbScissor);
//			aabbScissor.Move(Vec2(-m_visibleArea.x, -m_visibleArea.y));
//		}
//
//		SetScissorClip(m_pDevice, aabbScissor.vMin.x, aabbScissor.vMin.y, aabbScissor.vSize.x, aabbScissor.vSize.y);
//
//		///--- tiles back layer
//		m_pSprite->SetTransform(&g_matIdentity);
//		//tiles - background
//		for (int yy = 0; yy < m_visibleAreaTL.h; yy++)
//		{
//			for (int xx = 0; xx < m_visibleAreaTL.w; xx++)
//			{
//				int tlX = xx + m_visibleAreaTL.x - m_levelAABB_TL.x;
//				int tlY = yy + m_visibleAreaTL.y - m_levelAABB_TL.y;
//				CTile *tl = &tiles[tlX][tlY];
//				if (tl->tileIDs[0] >= 0)
//					m_pSprite->Draw(m_texManager.m_Texs[m_tilesTexBaseIdx]->pTexture, &tl->srcRects[0], NULL, &Vec3(xx * tileW, yy * tileH, 0.0f), 0xffffffff);
//			}
//		}
//		m_pSprite->Flush();
//
//		///--- objects back layer ---
//		m_pSprite->SetTransform(&mattrans);
//
//		for (int kk = 0; kk < m_visibleList.visible_props[K_LVL_LAYER_BACK].Count(); kk++)
//		{
//			CProp *active = m_visibleList.visible_props[K_LVL_LAYER_BACK].m_pData[kk];
//			if (active->flipX /*|| active->flipY*/)
//			{
//				matlocal = mattrans;
//				//pozitie sprite
//				if (active->flipX)
//				{
//					matlocal._11 = -1.0f; //scalare X
//					matlocal._41 += 2.0f * active->pos.x;// +2.0f * active_bbox.x + active_bbox.w; 
//				}
//				m_pSprite->SetTransform(&matlocal);
//				active->sprite.paint_firstModule(&m_sprProps);
//				m_pSprite->SetTransform(&mattrans);
//			}
//			else
//			{
//				active->sprite.paint_firstModule(&m_sprProps);
//			}
//		}
//		m_pSprite->Flush();
//
//		///--- tiles MIDDLE layer
//		m_pSprite->SetTransform(&g_matIdentity);
//		//tiles - background
//		for (int yy = 0; yy < m_visibleAreaTL.h; yy++)
//		{
//			for (int xx = 0; xx < m_visibleAreaTL.w; xx++)
//			{
//				int tlX = xx + m_visibleAreaTL.x - m_levelAABB_TL.x;
//				int tlY = yy + m_visibleAreaTL.y - m_levelAABB_TL.y;
//				CTile *tl = &tiles[tlX][tlY];
//				if (tl->tileIDs[1] >= 0)
//					m_pSprite->Draw(m_texManager.m_Texs[m_tilesTexBaseIdx]->pTexture, &tl->srcRects[1], NULL, &Vec3(xx * tileW, yy * tileH, 0.0f), 0xffffffff);
//			}
//		}
//		m_pSprite->Flush();
//
//		///--- decals (blood stains, explosion marks, bullet holes etc) ---
//		if (m_visibleList.visible_decals[K_LVL_DECAL_LAYER_BACKWALLS].Count() > 0)
//		{
//			m_pSprite->SetTransform(&mattrans);
//			//set special state (keep alpha of destination)
//			if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0)
//			{
//				m_pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ZERO);
//				m_pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_ONE);
//				m_pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
//			}
//			else
//			{
//				m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true); //neaparat nevoie
//				m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
//				m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
//			}
//			//--- paint them ---
//			for (int kk = 0; kk < m_visibleList.visible_decals[K_LVL_DECAL_LAYER_BACKWALLS].Count(); kk++)
//			{
//				m_visibleList.visible_decals[K_LVL_DECAL_LAYER_BACKWALLS].m_pData[kk]->sprite.paint_firstModule(&m_sprProps);
//			}
//			m_pSprite->Flush();
//
//			//restore state
//			if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0)
//			{
//				m_pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA);
//				m_pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_DESTALPHA);
//				m_pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
//			}
//			else
//			{
//				m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
//				m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
//			}
//		}
//
//		///--- objects MIDDLE layer ---
//		m_pSprite->SetTransform(&mattrans);
//
//		for (int kk = 0; kk < m_visibleList.visible_props[K_LVL_LAYER_MIDDLE].Count(); kk++)
//		{
//			CProp *active = m_visibleList.visible_props[K_LVL_LAYER_MIDDLE].m_pData[kk];
//			if (active->flipX)
//			{
//				matlocal = mattrans;
//				//pozitie sprite
//				if (active->flipX)
//				{
//					matlocal._11 = -1.0f; //scalare X
//					matlocal._41 += 2.0f * active->pos.x;// +2.0f * active_bbox.x + active_bbox.w; 
//				}
//				m_pSprite->SetTransform(&matlocal);
//				active->sprite.paint_firstModule(&m_sprProps);
//				m_pSprite->SetTransform(&mattrans);
//			}
//			else
//			{
//				active->sprite.paint_firstModule(&m_sprProps);
//			}
//		}
//
//		//--- middle objects that can be interacted with are blinking ---
//		AdditiveBlendingON(m_pDevice, m_pSprite);
//		float fAlp = 0.4f * LIMIT(float((2.0f * sin(fLocalTimeline * 2.5f)) - 1.0f), 0.0f, 1.0f);
//		float fAlp2 = 0.4f * ((sin(fLocalTimeline * 10.0f) + 1.0f) / 2.0f);
//		for (int kk = 0; kk < m_visibleList.visible_props[K_LVL_LAYER_MIDDLE].Count(); kk++)
//		{
//			CProp *active = m_visibleList.visible_props[K_LVL_LAYER_MIDDLE].m_pData[kk];
//			
//			if (!active->bStandsOut)
//				continue;
//
//			DWORD colAlpha = D3DCOLOR_FFFA(fAlp);
//			//object is being touched so show it
//			if( ((pPlayerActor[0] != null) && (active == pPlayerActor[0]->pClosestTouchable)) ||
//				((pPlayerActor[1] != null) && (active == pPlayerActor[1]->pClosestTouchable)) )
//				colAlpha = D3DCOLOR_FFFA(fAlp2);
//
//			if (active->flipX)
//			{
//				matlocal = mattrans;
//				//pozitie sprite
//				if (active->flipX)
//				{
//					matlocal._11 = -1.0f; //scalare X
//					matlocal._41 += 2.0f * active->pos.x;// +2.0f * active_bbox.x + active_bbox.w; 
//				}
//
//				m_pSprite->SetTransform(&matlocal);
//				CSprite spr = active->sprite;
//				spr.color = colAlpha;
//				spr.paint_firstModule(&m_sprProps);
//				m_pSprite->SetTransform(&mattrans);
//			}
//			else
//			{
//				CSprite spr = active->sprite;
//				spr.color = colAlpha;
//				spr.paint_firstModule(&m_sprProps);
//			}
//		}
//		AdditiveBlendingOFF(m_pDevice, m_pSprite);
//
//
//		///--- actor shadows --- only on super high level of detail ---
//		m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true); //neaparat nevoie
//		m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_DESTALPHA);
//		m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
//
//		m_pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);  //disable
//
//		for (int kk = 0; kk < m_visibleList.visible_actors.Count(); kk++)
//		{
//			CActor* actor = m_visibleList.visible_actors.m_pData[kk];
//		
//			matlocal = mattrans;
//			//aplic matrice flipX daca este cazul
//			if (actor->lookDirXsign == -1)
//			{
//				matlocal._11 = -1.0f;
//				matlocal._41 += 2.0f * actor->sprite_feet.pos.x;
//			}
//
//			//--- versiune cu o umbra fixa ---
//			switch (UTGetAppClass().m_Settings.nLOD_shadows)
//			{
//				case K_UT_LOD_LOW:
//				{
//					matlocal._41 += 6.0f; //distanta umbrei
//					m_pSprite->SetTransform(&matlocal);
//					
//					float spriteAlpha = D3DCOLOR_GETFALPHA(actor->color);
//					DWORD dwShadCol = D3DCOLOR_XXXA(spriteAlpha * 0.4f);
//
//					if ((actor->templateActor.bComposedAnimation) && (actor->sprite_feet.animationIdx >= 0))
//					{
//						actor->sprite_feet.paint_firstModuleColorized(&m_sprActors, dwShadCol);
//					}
//					actor->sprite.paint_firstModuleColorized(&m_sprActors, dwShadCol);
//				}
//				break;
//				case K_UT_LOD_MED:
//				{
//					//versiune cu o singura umbra dinamica, media iluminarii
//					//max shadow offset (16.0f)
//					float fMaxOffset = 16.0f;
//
//					float fIllumination = 0.0f; //cantitatea de lumina care cade pe actor
//					Vec2 vLightResultant(0.0f, 0.0f); //media vectorilor de iluminare
//
//					int influences = 0;
//					for (int ll = 0; ll < m_visibleList.visible_lights.Count(); ll++)
//					{
//						CLight* light = m_visibleList.visible_lights[ll];
//						if (!light->castShadows)
//							continue;
//						if (light->type != K_LVL_LT_POINT)
//							continue;
//						Vec2 lightdir = light->pos - actor->posHeart;
//						float lightdist = MUVec2Len(&lightdir);
//						if (lightdist > light->fRadius)
//							continue;
//						if (!IsLineOfSight(light->pos, actor->GetPosHeart()))
//							continue;
//
//						//normalize vector
//						lightdir /= lightdist;
//						float opacity = 1.0f - lightdist / m_visibleList.visible_lights.m_pData[ll]->fRadius;
//
//						fIllumination += opacity;
//						//medie ponderata a vectorilor
//						vLightResultant += (lightdir * (1.0f - opacity) * fMaxOffset) * opacity;
//						influences++;
//					}
//
//					if (influences > 0)
//					{
//						vLightResultant /= (float)influences;
//						Mat matshad = matlocal;
//						//scalare offset
//						float offlen = MUVec2Len(&vLightResultant);
//						if (offlen > fMaxOffset)
//							vLightResultant *= fMaxOffset / offlen;
//
//						matshad._41 -= vLightResultant.x;
//						matshad._42 -= vLightResultant.y;
//
//						m_pSprite->SetTransform(&matshad);
//
//						float spriteAlpha = D3DCOLOR_GETFALPHA(actor->color);
//						DWORD dwShadCol = D3DCOLOR_COLORALPHA(0x00000000, spriteAlpha * (fIllumination * 0.6f)); //max opacity = 0.7f
//
//						if ((actor->templateActor.bComposedAnimation) && (actor->sprite_feet.animationIdx >= 0))
//						{
//							actor->sprite_feet.paint_firstModuleColorized(&m_sprActors, dwShadCol);
//						}
//						actor->sprite.paint_firstModuleColorized(&m_sprActors, dwShadCol);
//					}
//				}
//				break;
//				case K_UT_LOD_HIGH:
//				{
//					//pentru fiecare lumina vad unde pica proiectia umbrei - versiune cu umbre dinamice
//					//are mici probleme cand nu te vede lumina si cand e lumina de sub tine si apare doar printr-o raza (umbra apare intreaga si iese din volumul luminii)
//					Mat matshad;
//					float spriteAlpha = D3DCOLOR_GETFALPHA(actor->color);
//					for (int ll = 0; ll < m_visibleList.visible_lights.Count(); ll++)
//					{
//						CLight* light = m_visibleList.visible_lights[ll];
//						if (!light->castShadows)
//							continue;
//						if (light->type != K_LVL_LT_POINT)
//							continue;
//						Vec2 lightdir = light->pos - actor->GetPosHeart();
//						float lightdist = MUVec2Len(&lightdir);
//						if (lightdist > light->fRadius)
//							continue;
//						if (!IsLineOfSight(light->pos, actor->posHeart))
//							continue;
//						//normalize vector
//						lightdir /= lightdist;
//						float opacity = 1.0f - lightdist / light->fRadius;
//
//						matshad = matlocal;
//
//						//max shadow offset (16.0f)
//						matshad._41 -= (1.0f - opacity) * lightdir.x * 16.0f;
//						matshad._42 -= (1.0f - opacity) * lightdir.y * 16.0f;
//
//						m_pSprite->SetTransform(&matshad);
//						DWORD dwShadCol = D3DCOLOR_COLORALPHA(0x00000000, spriteAlpha * opacity * 0.7f); //max opacity = 0.7f
//						if ((actor->templateActor.bComposedAnimation) && (actor->sprite_feet.animationIdx >= 0))
//						{
//							actor->sprite_feet.paint_firstModuleColorized(&m_sprActors, dwShadCol);
//						}
//						actor->sprite.paint_firstModuleColorized(&m_sprActors, dwShadCol);
//					}
//				}
//				break;
//			}
//		}
//		m_pSprite->Flush();
//
//		//restore state for normal rendering
//		m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
//		m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
//		if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0)
//		{
//			m_pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
//		}
//
//		///--- WALLS - front layer/sections ---
//		m_pSprite->SetTransform(&g_matIdentity);
//		//tiles - foreground/sections
//		for (int yy = 0; yy < m_visibleAreaTL.h; yy++)
//		{
//			for (int xx = 0; xx < m_visibleAreaTL.w; xx++)
//			{
//				int tlX = xx + m_visibleAreaTL.x - m_levelAABB_TL.x;
//				int tlY = yy + m_visibleAreaTL.y - m_levelAABB_TL.y;
//				CTile *tl = &tiles[tlX][tlY];
//				if (tl->tileIDs[2] >= 0)
//					m_pSprite->Draw(m_texManager.m_Texs[m_tilesTexBaseIdx]->pTexture, &tl->srcRects[2], NULL, &Vec3(xx * tileW, yy * tileH, 0.0f), 0xffffffff);
//			}
//		}
//		m_pSprite->Flush();
//
//		///--- decals objects FRONT layer 
//		m_pSprite->SetTransform(&mattrans);
//		for (int kk = 0; kk < m_visibleList.visible_decals[K_LVL_DECAL_LAYER_BACKOBJECTS].Count(); kk++)
//		{
//			m_visibleList.visible_decals[K_LVL_DECAL_LAYER_BACKOBJECTS].m_pData[kk]->sprite.paint_firstModule(&m_sprProps);
//		}
//		m_pSprite->Flush();
//
//		///--- paint RT particles - BACK ---
//		m_pSprite->SetTransform(&g_matIdentity);
//		g_particlesMgr.PaintLayerOffset(K_PART_LAYER_RT_BACK_NRM, Vec2(-m_visibleArea.x, -m_visibleArea.y), false);
//		g_particlesMgr.PaintLayerOffset(K_PART_LAYER_RT_BACK_NRM_LIGHT, Vec2(-m_visibleArea.x, -m_visibleArea.y), true);
//
//
//		///--- paint actors (player included) ---
//		m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
//		for (int kk = 0; kk < m_visibleList.visible_actors.Count(); kk++)
//		{
//			CActor* actor = m_visibleList.visible_actors.m_pData[kk];
//
//			matlocal = mattrans;
//			//apply flipX matrix
//			if (actor->lookDirXsign == -1)
//			{
//				matlocal._11 = -1.0f;
//				matlocal._41 += 2.0f * actor->sprite_feet.pos.x; //takes the coord from the sprite position as that's already rounded (eliminates jitter)
//			}
//			m_pSprite->SetTransform(&matlocal);
//
//			if ((actor->templateActor.bComposedAnimation) && (actor->sprite_feet.animationIdx >= 0))
//			{
//				actor->sprite_feet.paint_firstModule_texOverride(&m_sprActors, actor->nSkinIdx * 2);
//			}
//			actor->sprite.paint_firstModule_texOverride(&m_sprActors, actor->nSkinIdx * 2);
//
//			//--- muzzle flash ---
//			if ((actor->pCurrentWeapon != null) && (actor->pCurrentWeapon->m_sprMuzzleFlash.animationIdx >= 0) && 
//				(actor->pCurrentWeapon->m_sprMuzzleFlash.animStatus != ANIM_STATUS_FRAMELOCK) )
//			{
//				actor->pCurrentWeapon->m_sprMuzzleFlash.pos = actor->pos + actor->vecWeapon_abs[((actor->bCrouched) ? 1 : 0)];
//				actor->pCurrentWeapon->m_sprMuzzleFlash.paint_firstModule(&m_sprActors);
//			}
//		}
//
//		m_pSprite->Flush();
//
//		m_pDevice->SetTransform(D3DTS_WORLD, &mattrans);
//		///--- BULLETS ---
//		PaintBullets();
//		//--- paint level Props ---
//		PaintProps(); //se deseneaza din active
//		
//		m_pSprite->Flush();
//		m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
//
//		//--- paint RT particles - FRONT ---
//		m_pSprite->SetTransform(&g_matIdentity);
//		g_particlesMgr.PaintLayerOffset(K_PART_LAYER_RT_FRONT_NRM, Vec2(-m_visibleArea.x, -m_visibleArea.y), false);
//		g_particlesMgr.PaintLayerOffset(K_PART_LAYER_RT_FRONT_NRM_LIGHT, Vec2(-m_visibleArea.x, -m_visibleArea.y), true);
//
//		///--- paint water details ---
//		/*
//		//#TODO: de vazut daca se mai poate optimiza aici...si daca merita optimizat
//		//daca voi avea mai multe chestii de desenat din acest array atunci nu mai merita optimizat
//		m_pSprite->SetTransform(&mattrans);
//		for (int kk = 0; kk < m_visibleList.logic_colShapesSpecial.Count(); kk++)
//		{
//			CCollisionShape * col = m_visibleList.logic_colShapesSpecial.m_pData[kk];
//			//water
//			if (col->type == K_LVL_COLL_TYPE_WATER)
//			{
//				if (m_waterAnimIdx < 0)
//				{
//#if defined(_DEBUG) || defined(DEBUG)
//					ErrorBox(K_ERR_WARNING, L"Water animation not set in background object (editor)!");
//#endif
//					continue;
//				}
//				CAABB wbb; //water bbox
//				if (AABB_Intersection(col->bbox, camAABB, wbb))
//				{
//					Vec2 texoff = col->bbox.vMin - wbb.vMin;
//					//!! animatia de apa trebuie sa aiba frame 1 pt suprafata apei si 2 pentru luminile din apa
//					CSprite spr(m_waterAnimIdx, wbb.vMin.x, wbb.vMin.y);
//					spr.color = 0xffffffff;
//
//					spr.currentFrame = 1; //linie apa
//					spr.paintTiledOffset(&m_sprBack, wbb.vSize.x, -1, texoff.x, texoff.y);
//
//					spr.currentFrame = 2; //lumini prin apa
//					spr.pos.y = col->bbox.vMin.y;
//					//alterneaza luminile intre ele	(animatie)
//					float alpha = (sin(fLocalTimeline) + 1.0f) * 0.5f;
//					spr.color = D3DCOLOR_FFFA(alpha);
//					spr.paintTiledOffset(&m_sprBack, wbb.vSize.x, -1, texoff.x, 0.0f);
//					spr.color = D3DCOLOR_FFFA(1.0f - alpha);
//					spr.paintTiledOffset(&m_sprBack, wbb.vSize.x, -1, texoff.x + 64.0f, 0.0f);
//				}
//			}
//		}
//		m_pSprite->SetTransform(&g_matIdentity);
//		*/
//		///--- paint actives front layer ---
//		MUMatAffine2D(&mattrans, 1.0f, NULL, 0.0f, &Vec2(-m_visibleArea.x, -m_visibleArea.y));
//		m_pSprite->SetTransform(&mattrans);
//
//		for (int kk = 0; kk < m_visibleList.visible_props[K_LVL_LAYER_FRONT].Count(); kk++)
//		{
//			CProp *active = m_visibleList.visible_props[K_LVL_LAYER_FRONT].m_pData[kk];
//			if (active->flipX)
//			{
//				matlocal = mattrans;
//				if (active->flipX)
//				{
//					matlocal._11 = -1.0f; //scalare X
//					matlocal._41 += 2.0f * active->pos.x;// +2.0f * active_bbox.x + active_bbox.w; //e un calcul logic ca sa ramana incadrat in acelasi bbox real
//				}
//				m_pSprite->SetTransform(&matlocal);
//				active->sprite.paint_firstModule(&m_sprProps);
//				m_pSprite->SetTransform(&mattrans);
//			}
//			else
//			{
//				if (active->fAngle == 0.0f)
//				{
//					active->sprite.paint_firstModule(&m_sprProps);
//				}
//				else
//				{
//					//for now only front objects can be rotated... much optimization, such speed
//					MUMatAffine2D(&matlocal, 1.0f, NULL, active->fAngle, &Vec2(active->pos.x - m_visibleArea.x, active->pos.y - m_visibleArea.y));
//
//					m_pSprite->SetTransform(&matlocal);
//					active->sprite.pos = Vec2(0.0f, 0.0f);
//					active->sprite.paint_firstModule(&m_sprProps);
//					m_pSprite->SetTransform(&mattrans);
//				}
//			}
//		}
//		///------ paint interactible front objects ------
//		AdditiveBlendingON(m_pDevice, m_pSprite);
//		for (int kk = 0; kk < m_visibleList.visible_props[K_LVL_LAYER_FRONT].Count(); kk++)
//		{
//			CProp *active = m_visibleList.visible_props[K_LVL_LAYER_FRONT].m_pData[kk];
//			if (!active->bStandsOut)
//				continue;
//
//			DWORD colAlpha = D3DCOLOR_FFFA(fAlp);
//			//object is being touched so show it
//			if (((pPlayerActor[0] != null) && (active == pPlayerActor[0]->pClosestTouchable)) ||
//				((pPlayerActor[1] != null) && (active == pPlayerActor[1]->pClosestTouchable)))
//				colAlpha = D3DCOLOR_FFFA(fAlp2);
//
//
//			if (active->flipX)
//			{
//				RECTXYWH active_bbox = m_sprProps.GetAFrameBBox_real(active->sprite.animationIdx, active->sprite.currentFrame);
//
//				matlocal = mattrans;
//				//pozitie sprite
//				if (active->flipX)
//				{
//					matlocal._11 = -1.0f; //scalare X
//					matlocal._41 += 2.0f * active->pos.x;// +2.0f * active_bbox.x + active_bbox.w; //e un calcul logic ca sa ramana incadrat in acelasi bbox real
//				}
//				m_pSprite->SetTransform(&matlocal);
//
//				CSprite spr = active->sprite;
//				spr.color = colAlpha;
//				spr.paint_firstModule(&m_sprProps);
//
//				m_pSprite->SetTransform(&mattrans);
//			}
//			else
//			{
//				if (active->fAngle == 0.0f)
//				{
//					CSprite spr = active->sprite;
//					spr.color = colAlpha;
//					spr.paint_firstModule(&m_sprProps);
//				}
//				else
//				{
//					MUMatAffine2D(&matlocal, 1.0f, NULL, active->fAngle, &Vec2(active->pos.x - m_visibleArea.x, active->pos.y - m_visibleArea.y));
//
//					m_pSprite->SetTransform(&matlocal);
//					CSprite spr = active->sprite;
//					spr.color = colAlpha;
//					spr.pos = Vec2(0.0f, 0.0f);
//					spr.paint_firstModule(&m_sprProps);
//					m_pSprite->SetTransform(&mattrans);
//				}
//			}
//		}
//		AdditiveBlendingOFF(m_pDevice, m_pSprite);
//
//		
//		m_pSprite->SetTransform(&g_matIdentity);
//		m_pSprite->Flush(); //acest flush trebuie chemat neaparat
//
//		///.////////////////////////////////////////////////////////
//		///	NORMAL MAP
//		///.////////////////////////////////////////////////////////
//		//set clip on colormap
//		aabbScissor.Move(Vec2(K_RTT_H_WIDTH, 0.0f));
//		SetScissorClip(m_pDevice, aabbScissor.vMin.x, aabbScissor.vMin.y, aabbScissor.vSize.x, aabbScissor.vSize.y);
//
//		///--- back tiles normal map
//		m_pSprite->SetTransform(&g_matIdentity);
//		for (int yy = 0; yy < m_visibleAreaTL.h; yy++)
//		{
//			for (int xx = 0; xx < m_visibleAreaTL.w; xx++)
//			{
//				int tlX = xx + m_visibleAreaTL.x - m_levelAABB_TL.x;
//				int tlY = yy + m_visibleAreaTL.y - m_levelAABB_TL.y;
//				CTile *tl = &tiles[tlX][tlY];
//				if (tl->tileIDs[0] >= 0)
//					m_pSprite->Draw(m_texManager.m_Texs[m_tilesTexNormIdx]->pTexture, &tl->srcRects[0], NULL, &Vec3(K_RTT_H_WIDTH + xx * tileW, yy * tileH, 0.0f), 0xffffffff);
//			}
//		}
//		m_pSprite->Flush();
//
//		///--- actives back layer ---
//		m_pSprite->SetTransform(&mattrans);
//
//		for (int kk = 0; kk < m_visibleList.visible_props[K_LVL_LAYER_BACK].Count(); kk++)
//		{
//			CProp *active = m_visibleList.visible_props[K_LVL_LAYER_BACK].m_pData[kk];
//
//			if (active->flipX)
//			{
//				RECTXYWH active_bbox = m_sprProps.GetAFrameBBox_real(active->sprite.animationIdx, active->sprite.currentFrame);
//
//				matlocal = mattrans;
//				//pozitie sprite
//				if (active->flipX)
//				{
//					matlocal._11 = -1.0f; //scalare X
//					matlocal._41 += 2.0f * active->pos.x;// +2.0f * active_bbox.x + active_bbox.w; 
//				}
//				//move obj to normals part of the RT
//				matlocal._41 += K_RTT_H_WIDTH;
//
//				m_pSprite->SetTransform(&matlocal);
//				active->sprite.paint_firstModule_texOverride(&m_sprProps, 1);
//				m_pSprite->SetTransform(&mattrans);
//			}
//			else
//			{
//				active->sprite.paint_firstModule_texOverride(&m_sprProps, 1, K_RTT_H_WIDTH);
//			}
//		}
//		m_pSprite->Flush();
//
//		///--- middle tiles normal map
//		m_pSprite->SetTransform(&g_matIdentity);
//		for (int yy = 0; yy < m_visibleAreaTL.h; yy++)
//		{
//			for (int xx = 0; xx < m_visibleAreaTL.w; xx++)
//			{
//				int tlX = xx + m_visibleAreaTL.x - m_levelAABB_TL.x;
//				int tlY = yy + m_visibleAreaTL.y - m_levelAABB_TL.y;
//				CTile *tl = &tiles[tlX][tlY];
//
//				if (tl->tileIDs[1] >= 0)
//					m_pSprite->Draw(m_texManager.m_Texs[m_tilesTexNormIdx]->pTexture, &tl->srcRects[1], NULL, &Vec3(K_RTT_H_WIDTH + xx * tileW, yy * tileH, 0.0f), 0xffffffff);
//			}
//		}
//		m_pSprite->Flush();
//
//		///--- MIDDLE LAYER objects
//		m_pSprite->SetTransform(&mattrans);
//		for (int kk = 0; kk < m_visibleList.visible_props[K_LVL_LAYER_MIDDLE].Count(); kk++)
//		{
//			CProp *active = m_visibleList.visible_props[K_LVL_LAYER_MIDDLE].m_pData[kk];
//
//			if (active->flipX)
//			{
//				matlocal = mattrans;
//				//pozitie sprite
//				if (active->flipX)
//				{
//					matlocal._11 = -1.0f; //scalare X
//					matlocal._41 += 2.0f * active->pos.x;
//				}
//				//mut sprite pe zona de normale
//				matlocal._41 += K_RTT_H_WIDTH;
//
//				m_pSprite->SetTransform(&matlocal);
//				active->sprite.paint_firstModule_texOverride(&m_sprProps, 1);
//				m_pSprite->SetTransform(&mattrans);
//			}
//			else
//			{
//				active->sprite.paint_firstModule_texOverride(&m_sprProps, 1, K_RTT_H_WIDTH);
//			}
//		}
//		m_pSprite->Flush();
//
//		///--- front tiles normal map ---
//		m_pSprite->SetTransform(&g_matIdentity);
//		for (int yy = 0; yy < m_visibleAreaTL.h; yy++)
//		{
//			for (int xx = 0; xx < m_visibleAreaTL.w; xx++)
//			{
//				int tlX = xx + m_visibleAreaTL.x - m_levelAABB_TL.x;
//				int tlY = yy + m_visibleAreaTL.y - m_levelAABB_TL.y;
//				CTile *tl = &tiles[tlX][tlY];
//				if (tl->tileIDs[2] >= 0)
//					m_pSprite->Draw(m_texManager.m_Texs[m_tilesTexNormIdx]->pTexture, &tl->srcRects[2], NULL, &Vec3(K_RTT_H_WIDTH + xx * tileW, yy * tileH, 0.0f), 0xffffffff);
//			}
//		}
//		m_pSprite->Flush();
//
//		///--- paint RT particles - BACK ---
//		m_pSprite->SetTransform(&g_matIdentity);
//		g_particlesMgr.PaintLayerOffset_texOverride(K_PART_LAYER_RT_BACK_NRM, Vec2(-m_visibleArea.x + K_RTT_H_WIDTH, -m_visibleArea.y), false, 1);
//		g_particlesMgr.PaintLayerOffset_texOverride(K_PART_LAYER_RT_BACK_NRM_LIGHT, Vec2(-m_visibleArea.x + K_RTT_H_WIDTH, -m_visibleArea.y), false, 1);
//
//		///--- ACTORS NORMALS
//		for (int kk = 0; kk < m_visibleList.visible_actors.Count(); kk++)
//		{
//			CActor* actor = m_visibleList.visible_actors.m_pData[kk];
//			//aplic matricea de aliniere cu m_visibleArea
//			matlocal = mattrans;
//			//aplic matrice flipX daca este cazul
//			if (actor->lookDirXsign == -1)
//			{
//				matlocal._11 = -1.0f;
//				matlocal._41 += 2.0f * actor->sprite_feet.pos.x;
//			}
//			matlocal._41 += K_RTT_H_WIDTH;
//
//			m_pSprite->SetTransform(&matlocal);
//			//#HACK: when taking damage paint the color frame instead of the normals frame so it looks loghter
//			int nTexOverride = 1;
//			if (actor->nTookDamageFrames > 0)
//				nTexOverride = 0;
//
//			if ((actor->templateActor.bComposedAnimation) && (actor->sprite_feet.animationIdx >= 0))
//			{
//				actor->sprite_feet.color = actor->color;
//				actor->sprite_feet.paint_firstModule_texOverride(&m_sprActors, nTexOverride);
//			}
//			actor->sprite.color = actor->color;
//			actor->sprite.paint_firstModule_texOverride(&m_sprActors, nTexOverride);
//
//			//--- muzzle flash ---
//			if ((actor->pCurrentWeapon != null) && (actor->pCurrentWeapon->m_sprMuzzleFlash.animationIdx >= 0) && (actor->pCurrentWeapon->m_sprMuzzleFlash.animStatus != ANIM_STATUS_FRAMELOCK))
//			{
//				actor->pCurrentWeapon->m_sprMuzzleFlash.pos = actor->pos + actor->vecWeapon_abs[((actor->bCrouched) ? 1 : 0)];
//				actor->pCurrentWeapon->m_sprMuzzleFlash.paint_firstModule_texOverride(&m_sprActors, 1);
//			}
//		}
//		m_pSprite->Flush();
//
//		///--- BULLETS NORMALS/self illumi ---
//		matlocal = mattrans;
//		matlocal._41 += K_RTT_H_WIDTH;
//		m_pDevice->SetTransform(D3DTS_WORLD, &matlocal);
//		PaintBullets(true);
//		m_pSprite->Flush();
//		m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
//
//		///--- paint RT particles - FRONT ---
//		m_pSprite->SetTransform(&g_matIdentity);
//		g_particlesMgr.PaintLayerOffset_texOverride(K_PART_LAYER_RT_FRONT_NRM, Vec2(-m_visibleArea.x + K_RTT_H_WIDTH, -m_visibleArea.y), false, 1);
//		g_particlesMgr.PaintLayerOffset_texOverride(K_PART_LAYER_RT_FRONT_NRM_LIGHT, Vec2(-m_visibleArea.x + K_RTT_H_WIDTH, -m_visibleArea.y), false, 1);
//
//		///--- paint actives front layer NORMALS ---
//		m_pSprite->SetTransform(&mattrans);
//		for (int kk = 0; kk < m_visibleList.visible_props[K_LVL_LAYER_FRONT].Count(); kk++)
//		{
//			CProp *active = m_visibleList.visible_props[K_LVL_LAYER_FRONT].m_pData[kk];
//
//			if (active->flipX)
//			{
//				RECTXYWH active_bbox = m_sprProps.GetAFrameBBox_real(active->sprite.animationIdx, active->sprite.currentFrame);
//
//				matlocal = mattrans;
//				//pozitie sprite
//				if (active->flipX)
//				{
//					matlocal._11 = -1.0f; //scalare X
//					matlocal._41 += 2.0f * active->pos.x;// +2.0f * active_bbox.x + active_bbox.w; //e un calcul logic ca sa ramana incadrat in acelasi bbox real
//				}
//				//move sprite to right side of RT (normals)
//				matlocal._41 += K_RTT_H_WIDTH;
//
//				m_pSprite->SetTransform(&matlocal);
//				active->sprite.paint_firstModule_texOverride(&m_sprProps, 1);
//				m_pSprite->SetTransform(&mattrans);
//			}
//			else
//			{
//				if (active->fAngle == 0.0f)
//				{
//					//mut sprite pe zona de normale
//					active->sprite.paint_firstModule_texOverride(&m_sprProps, 1, K_RTT_H_WIDTH);
//				}
//				else
//				{
//					//#TODO: daca ma hotarasc sa nu pun rotatii la obiecte scot partea asta. Momentan am rotatii doar pe front layer la active
//					MUMatAffine2D(&matlocal, 1.0f, NULL, active->fAngle, &Vec2(active->pos.x + K_RTT_H_WIDTH - m_visibleArea.x, active->pos.y - m_visibleArea.y));
//
//					m_pSprite->SetTransform(&matlocal);
//					active->sprite.pos = Vec2(0.0f, 0.0f);
//					active->sprite.paint_firstModule_texOverride(&m_sprProps, 1);
//					m_pSprite->SetTransform(&mattrans);
//				}
//			}
//		}
//		m_pSprite->Flush();
//
//
//		//close separate alpha blending
//		if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0)
//		{
//			m_pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, false);
//		}
//
//		//end sprite
//		m_pSprite->SetTransform(&g_matIdentity);
//		m_pSprite->End();
//		//end scene paint/pass
//		//V(m_pRenderToSurface->EndScene(0));
//	}
//
//	return hr;
	return S_OK;
}

HRESULT CLevel::PaintOffscreen_nothing()
{
	HRESULT hr = S_OK;
	//daca nu am capabilitatea de offscreen ies cu eroare
	if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_RTT) == 0)
		return E_FAIL;

	hr = S_OK;// m_pRenderToSurface->BeginScene(m_pRTSurface, NULL);
	if (SUCCEEDED(hr))
	{
		// Clear the render target and the zbuffer 
		V(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, K_GAME_CLEAR_COLOR, 1.0f, 0));

	}
	//end scene paint/pass
	//V(m_pRenderToSurface->EndScene(0));
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
	hr = S_OK;// m_pRT_final->BeginScene(m_pRTSurface_final, NULL);
	if (SUCCEEDED(hr))
	{
		// Clear the render target and the zbuffer 
		V(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, K_GAME_CLEAR_COLOR, 1.0f, 0));

		//Mat matProj;
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

		//V(m_pRT_final->EndScene(0));
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

	//hr = m_pRT_final->BeginScene(m_pRTSurface_final, NULL);
	if (SUCCEEDED(hr))
	{
		// Clear the render target and the zbuffer 
		V(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, K_GAME_CLEAR_COLOR, 1.0f, 0));
	}

	//V(m_pRT_final->EndScene(0));
	return S_OK;
}

OPRESULT CLevel::PaintDeferredBuffers()
{
	HRESULT hr = S_OK;
	CRTManager::CEngineRenderTarget* pRT = nullptr;
	///----------------------------------------------------
	/// 1. NORMAL MAP AND HEIGHT MAP
	///----------------------------------------------------
	pRT = UTGetRTManager().GetRTbyUID(K_RTID_TEMP1);
	if (pRT != null)
	{
		hr = UTGetRTManager().BeginSceneRT(pRT);
		if (SUCCEEDED(hr))
		{
			// Clear the render target and the zbuffer 
			V(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, K_GAME_CLEAR_COLOR, 1.0f, 0));
			//use sprite
			//m_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_OBJECTSPACE | D3DXSPRITE_DONOTSAVESTATE);

			//Mat matProj;
			//D3DXMatrixOrthoOffCenterLH(&matProj, 0.5f, pRT->nWidth + 0.5f, pRT->nHeight + 0.5f, 0.5f, 0.0f, 1.0f);
			m_pDevice->SetTransform(D3DTS_PROJECTION, &pRT->matProj);

			m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
			m_pDevice->SetTransform(D3DTS_VIEW, &g_matIdentity);

			RenderPass(K_LVL_RP_NORMALS_HEIGHT, &pRT->matProj);

			// end sprite
			//m_pSprite->End();

			V(UTGetRTManager().EndSceneRT(pRT));

		}
	}

	///----------------------------------------------------
	/// 2. LIGHT MAP
	///----------------------------------------------------
	pRT = UTGetRTManager().GetRTbyUID(K_RTID_COLORDEPTHSTENCIL);
	if (pRT != null)
	{
		hr = UTGetRTManager().BeginSceneRT(pRT);
		if (SUCCEEDED(hr))
		{
			// Clear the render target and the zbuffer 
			V(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET , 0xff000000, 1.0f, 0));
			//use sprite
			//m_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_OBJECTSPACE | D3DXSPRITE_DONOTSAVESTATE);

			//Mat matProj;
			//D3DXMatrixOrthoOffCenterLH(&matProj, 0.5f, pRT->nWidth + 0.5f, pRT->nHeight + 0.5f, 0.5f, 0.0f, 1.0f);
			m_pDevice->SetTransform(D3DTS_PROJECTION, &pRT->matProj);

			m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
			m_pDevice->SetTransform(D3DTS_VIEW, &g_matIdentity);

			// special method for rendering lights pass
			// uses the height/normals render target
			RenderPass_Lights(&pRT->matProj);

			// end sprite
			//m_pSprite->End();

			V(UTGetRTManager().EndSceneRT(pRT));

		}
	}

	///----------------------------------------------------
	/// 3. COLOR MAP - overwrites the normal map as we don't need it anymore
	///----------------------------------------------------
	pRT = UTGetRTManager().GetRTbyUID(K_RTID_TEMP1);
	if (pRT != null)
	{
		hr = UTGetRTManager().BeginSceneRT(pRT);
		if (SUCCEEDED(hr))
		{
			// Clear the render target and the zbuffer 
			V(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, K_GAME_CLEAR_COLOR, 1.0f, 0));
			//use sprite
			//m_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_OBJECTSPACE | D3DXSPRITE_DONOTSAVESTATE);

			//#TODO: este corect ?? offset the projection matrix by 0.5f because in DX the pixel's 0.0 is the center of the pixel
			//Mat matProj;
			//D3DXMatrixOrthoOffCenterLH(&matProj, 0.5f, pRT->nWidth + 0.5f, pRT->nHeight + 0.5f, 0.5f, 0.0f, 1.0f);
			m_pDevice->SetTransform(D3DTS_PROJECTION, &pRT->matProj);

			m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
			m_pDevice->SetTransform(D3DTS_VIEW, &g_matIdentity);

			RenderPass(K_LVL_RP_COLORS, &pRT->matProj);

			// end sprite
			//m_pSprite->End();

			V(UTGetRTManager().EndSceneRT(pRT));

		}
	}

	///----------------------------------------------------
	/// 4. COMPOSITION - composes buffers into one
	///----------------------------------------------------
	pRT = UTGetRTManager().GetRTbyUID(K_RTID_FINAL);
	if (pRT != null)
	{
		hr = UTGetRTManager().BeginSceneRT(pRT);
		if (SUCCEEDED(hr))
		{
			// Clear the render target and the zbuffer 
			V(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0xffff0000, 1.0f, 0));
			//use sprite
			m_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_OBJECTSPACE | D3DXSPRITE_DONOTSAVESTATE);

			//#TODO: este corect ?? offset the projection matrix by 0.5f because in DX the pixel's 0.0 is the center of the pixel
			//Mat matProj;
			//D3DXMatrixOrthoOffCenterLH(&matProj, 0.5f, pRT->nWidth + 0.5f, pRT->nHeight + 0.5f, 0.5f, 0.0f, 1.0f);
			m_pDevice->SetTransform(D3DTS_PROJECTION, &pRT->matProj);

			m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
			m_pDevice->SetTransform(D3DTS_VIEW, &g_matIdentity);

			// RT sized quad with tex1 color, tex2 lightmap
			RenderPass_Composition(&pRT->matProj);

			// end sprite
			m_pSprite->End();

			V(UTGetRTManager().EndSceneRT(pRT));

		}
	}


	return K_OP_OK;
}

OPRESULT CLevel::RenderPass(eLVLRenderPass ePass, Mat* matProj)
{
	_ASSERT((ePass > K_LVL_RP_NONE) && (ePass < K_LVL_RP_COUNT));

	Mat	matView;

	RECTXYWH_F		camrect = m_camLevel.GetCamWorldAABB();
	CAABB			camAABB(camrect.x, camrect.y, camrect.Right(), camrect.Bottom());

	//locally used temp matrix
	Mat	matlocal;

	///----------------------------------------------------
	/// INITIAL SETUP
	///----------------------------------------------------
	m_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);

	m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0)
	{
		//#IMPORTANT: we need separate alpha blending or it will look bad when blending alpha values between them.
		//eg: if we blend semitransparent things on top of fully opaque walls, the walls become transparent
		//necessary but not really well supported. Better with a shader and custom sprite painter
		m_pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
		m_pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA);
		m_pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_DESTALPHA);
		m_pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
	}

	MUMatAffine2D(&matView, K_GAME_PIXEL_SIZE_F, NULL, 0.0f, &Vec2(-camrect.x * K_GAME_PIXEL_SIZE_F, -camrect.y * K_GAME_PIXEL_SIZE_F));
	m_pDevice->SetTransform(D3DTS_VIEW, &matView);
	m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);

	int nTilesTexIdx = g_level.m_tilesTexBaseIdx;
	// Offset in texture index so we paint from the normals texture when we render the normals pass
	int nTexIdxOffset = 0;				
	switch (ePass)
	{
		case K_LVL_RP_COLORS:
		{
			nTilesTexIdx = g_level.m_tilesTexBaseIdx;
			nTexIdxOffset = 0;
		}
		break;
		case K_LVL_RP_NORMALS_HEIGHT:
		{
			nTilesTexIdx = g_level.m_tilesTexNormIdx;
			nTexIdxOffset = 1;
		}
		break;
		case K_LVL_RP_LIGHTS:
		{
			ErrorBox(K_ERR_WARNING, L"Render lights using RenderPass_Lights() instead!");
		}
		break;
	}

	m_pDevice->SetTexture(0, g_level.m_texManager.GetTexture(nTilesTexIdx));
	// paint floors and vertical walls
	mapMesh.UpdateVisibility(camrect);
	mapMesh.PaintLayer(K_TILE_LAYER_FLOOR);
	mapMesh.PaintLayer(K_TILE_LAYER_WALLS);


	PVERTEXSHADER pSprVS = UTGetShaderManager().GetVShaderByName(L"VS_SPRITES2D");
	if (pSprVS)
		UTPainter().Begin(pSprVS, matView * *matProj);

	///--- paint actors
	m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);

	// old method below, convert to CSpr!!
	for (int kk = 0; kk < m_visibleList.visible_actors.Count(); kk++)
	{
		CActor* actor = m_visibleList.visible_actors.m_pData[kk];

		if ((actor->templateActor.bComposedAnimation) && (actor->sprite_feet.animationIdx >= 0))
		{
			actor->sprite_feet.paintModule_texOverride(&m_sprActors, 0, 0);
		}																 
		actor->sprite.paintModule_texOverride(&m_sprActors, 0, 0);
	}

	// now paint the bullets
	PaintBullets(ePass);

	UTPainter().Flush();

	///--- props = objects
	for (int kk = 0; kk < m_visibleList.visible_props.Count(); kk++)
	{
		CProp *prop = m_visibleList.visible_props.m_pData[kk];
		prop->sprite.paintModule_texOverride(&m_sprProps, 0, nTexIdxOffset);
	}

	UTPainter().Flush();
	UTPainter().End();

	// top layer of tiles
	UTGetShaderManager().SetVS(nullptr);
	m_pDevice->SetTexture(0, g_level.m_texManager.GetTexture(nTilesTexIdx));
	mapMesh.PaintLayer(K_TILE_LAYER_CEILING);

	return K_OP_OK;
}

OPRESULT CLevel::RenderPass_Lights(Mat* matProj)
{
	Mat				matView;

	RECTXYWH_F		camrect = m_camLevel.GetCamWorldAABB();
	CAABB			camAABB(camrect.x, camrect.y, camrect.Right(), camrect.Bottom());

	///----------------------------------------------------
	/// INITIAL SETUP
	///----------------------------------------------------
	m_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);

	m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0)
	{
		//#IMPORTANT: we need separate alpha blending or it will look bad when blending alpha values between them.
		//eg: if we blend semitransparent things on top of fully opaque walls, the walls become transparent
		//necessary but not really well supported. Better with a shader and custom sprite painter
		m_pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
		m_pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA);
		m_pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_DESTALPHA);
		m_pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
	}

	MUMatAffine2D(&matView, K_GAME_PIXEL_SIZE_F, NULL, 0.0f, &Vec2(-camrect.x * K_GAME_PIXEL_SIZE_F, -camrect.y * K_GAME_PIXEL_SIZE_F));

	m_pDevice->SetTransform(D3DTS_VIEW, &matView);
	m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);


	///--- paint lights ---
	PVERTEXSHADER pVShader = null;
	PPIXELSHADER pPShader = null;

	Mat matWVP = matView * (*matProj);
	// begin the painter
	PVERTEXSHADER pSprVS = UTGetShaderManager().GetVShaderByName(L"VS_SPRITES2D");
	if (pSprVS)
		UTPainter().Begin(pSprVS, matWVP);


#if defined(_DEBUG) || defined(DEBUG)
	/*
	// Paints the shadowed lights volume in wireframe
	m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	m_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD); //needed for color interpolation
	for (int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++)
	{
		CLight *nl = m_visibleList.visible_lights.m_pData[kk];

		if ((nl->type != K_LVL_LT_POINT) || (nl->castShadows == false))
			continue;

		m_bufferedPainter.DrawMesh(nl->m_nLightMeshIdx, true);
	}
	m_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
	m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	*/
#endif

	// generic VS data so we can automatically find positions
	float fConstDataVS[][4] = {
		{ camrect.x, camrect.y, camrect.w, camrect.h } //RTT rect_xywh in world coords
	};

	AdditiveBlendingON(m_pDevice, NULL);
	
	/*
	// directional light with shader, more expensive, harder to control
	CRTManager::CEngineRenderTarget* pRT = UTGetRTManager().GetRTbyUID(K_RTID_TEMP1);
	if (pRT != null)
	{
		m_pDevice->SetTexture(0, pRT->m_pRTTexture);
	}


	///--- directional light(s)
	// VS
	UTGetShaderManager().SetVSByName(L"VS_POINTLIGHT");
	UTGetShaderManager().SetVertexDeclaration(K_SHM_PNCT4T4);
	UTGetShaderManager().SetVSConstantF(0, (float*)&matWVP, 4);
	UTGetShaderManager().SetVSConstantF(4, (float*)fConstDataVS, ARRAY_SIZE(fConstDataVS));
	// PS
	UTGetShaderManager().SetPSByName(L"PS_DIRECTIONAL");
	for (int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++)
	{
		CLight *nl = m_visibleList.visible_lights.m_pData[kk];
		if (nl->type != K_LVL_LT_DIRECTIONAL)
			continue;

		//set Pshader constants
		float fConstData[][4] = {
			//x: intensity 
			{ nl->fIntensity, 0.0f, 0.0f, 0.0f },
			// xyz: inversed normalized directon
			{ -nl->vnDir.x, -nl->vnDir.y, -nl->vnDir.z, 0.0f }
		};
		UTGetShaderManager().SetPSConstantF(0, (float*)fConstData, ARRAY_SIZE(fConstData));
		m_bufferedPainter.DrawMesh(nl->m_nLightMeshIdx, true);
	}
	*/

	UTGetShaderManager().SetVS(nullptr);
	UTGetShaderManager().SetPS(nullptr);

	///--- directional lights (under shadow)
	// directional light without shader, doesn't take into account the object normals
	m_pDevice->SetTexture(0, nullptr);
	m_pDevice->SetTexture(1, nullptr);
	for (int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++)
	{
		CLight *nl = m_visibleList.visible_lights.m_pData[kk];
		if (nl->type == K_LVL_LT_DIRECTIONAL)
		{
			//paint and exit
			m_bufferedPainter.DrawMesh(nl->m_nLightMeshIdx, true);
			break;
		}
	}

	///----------------------------------------------------------------------------------
	/// SHADOWS
	///----------------------------------------------------------------------------------

	///--- precomputed wall shadows over directional lights
	AdditiveBlendingOFF(m_pDevice, NULL);
	scTexture* pShadowsTex = m_sprLights.GetTextureByAnim(ANM_LIGHTS_SPR_SHADOWS, 0, 0);
	if (pShadowsTex)
		m_pDevice->SetTexture(0, pShadowsTex->pTex);
	//#HINT: UpdateVisibility is optional as it was done in the previous colors render pass
	mapMesh.UpdateVisibility(camrect);
	mapMesh.PaintShadowLayer();
	
	///----------------------------------------------------------------------------------
	/// LIGHTS
	///----------------------------------------------------------------------------------
	AdditiveBlendingON(m_pDevice, NULL);

	///--- bullet lights
	// bullet shadows
	PaintBullets(K_LVL_RP_LIGHTS);
	UTPainter().Flush();
	

	///--- ambient light(s)
	// paint all general ambient lights and area lights here
	//#TODO: if we only have one ambiental per level then take color from g_wAmbientcolor
	UTGetShaderManager().SetVS(nullptr);
	UTGetShaderManager().SetPS(nullptr);
	m_pDevice->SetTexture(0, nullptr);
	m_pDevice->SetTexture(1, nullptr);
	for (int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++)
	{
		CLight *nl = m_visibleList.visible_lights.m_pData[kk];
		if (nl->type == K_LVL_LT_AMBIENTAL)
		{
			//paint and exit
			m_bufferedPainter.DrawMesh(nl->m_nLightMeshIdx, true);
			break;
		}
	}

	///--- point lights
	CRTManager::CEngineRenderTarget* pRT = UTGetRTManager().GetRTbyUID(K_RTID_TEMP1);
	if (pRT != null)
	{
		m_pDevice->SetTexture(0, pRT->m_pRTTexture);
	}
	// VS
	UTGetShaderManager().SetVSByName(L"VS_POINTLIGHT");
	UTGetShaderManager().SetVertexDeclaration(K_SHM_PNCT4T4);
	UTGetShaderManager().SetVSConstantF(0, (float*)&matWVP, 4);
	UTGetShaderManager().SetVSConstantF(4, (float*)fConstDataVS, ARRAY_SIZE(fConstDataVS));
	// PS
	UTGetShaderManager().SetPSByName(L"PS_POINTLIGHT");
	
	for (int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++)
	{
		CLight *nl = m_visibleList.visible_lights.m_pData[kk];
		
		if (nl->type != K_LVL_LT_POINT) 
			continue;

		//set Pshader constants
		float fConstData[][4] = { 
			// x:atten c1, y:atten c2, z:light radius, w:
			{ nl->fIntensity, nl->fRadius, 0.0f, 0.0f },
			// x: game height projection, y: height projection inverse (projected -> real), z: gauss dist atten factor
			{ ZHSCALE, INV_ZHSCALE, ct_fGaussLen, 0.0f },
			// xyz: light world position
			{ nl->vPos.x, nl->vPos.y, nl->vPos.z, 0.0f }
		};
		UTGetShaderManager().SetPSConstantF(0, (float*)fConstData, ARRAY_SIZE(fConstData));
		m_bufferedPainter.DrawMesh(nl->m_nLightMeshIdx, false);
	}


	///--- directional projected lights
	//all directional projected light must be in the same animation
	scTexture* pLightTex = m_sprLights.GetTextureByAnim(ANM_LIGHTS_SPR_PROJECTED_DIR, 0, 0);
	if(pLightTex)
		m_pDevice->SetTexture(1, pLightTex->pTex);
	m_pDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

	UTGetShaderManager().SetVSByName(L"VS_PROJECTEDDIR");
	UTGetShaderManager().SetVertexDeclaration(K_SHM_PNCT4T4);
	UTGetShaderManager().SetVSConstantF(0, (float*)&matWVP, 4);
	UTGetShaderManager().SetVSConstantF(4, (float*)fConstDataVS, ARRAY_SIZE(fConstDataVS));

	UTGetShaderManager().SetPSByName(L"PS_PROJECTEDDIR");
	for (int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++)
	{
		CLight *nl = m_visibleList.visible_lights.m_pData[kk];

		if (nl->type != K_LVL_LT_PROJECTED_DIR)
			continue;

		//set Pshader constants
		float fConstData[][4] = {
			//x: light intensity, y: geometry half size W, z: geometry half size H
			{ nl->fIntensity, nl->bbox_ini.vHalfSize.x, nl->bbox_ini.vHalfSize.y, 0.0f },
			// x: game height projection, y: height projection inverse (projected -> real), z: gauss dist atten factor
			{ ZHSCALE, INV_ZHSCALE, ct_fGaussLen, 0.0f },
			// xyz: light world position
			{ nl->vPos.x, nl->vPos.y, nl->vPos.z, 0.0f },
			// xyz: direction of light, normalized
			{ nl->vnDir.x, nl->vnDir.y, nl->vnDir.z, 0.0f },
			// xy: UL tex spot coords; zw: WH spot width height
			{ nl->lTexRect.left, nl->lTexRect.top, nl->lTexRect.right - nl->lTexRect.left, nl->lTexRect.bottom - nl->lTexRect.top }
		};
		UTGetShaderManager().SetPSConstantF(0, (float*)fConstData, ARRAY_SIZE(fConstData));
		m_bufferedPainter.DrawMesh(nl->m_nLightMeshIdx, false);
	}


	///--- IES lights without shadow
	scTexture* pIESTex = m_sprLights.GetTextureByAnim(ANM_LIGHTS_SPR_IES, 0, 0);
	if(pIESTex)
		m_pDevice->SetTexture(1, pIESTex->pTex);
	m_pDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	// VS
	UTGetShaderManager().SetVSByName(L"VS_POINTLIGHT");
	UTGetShaderManager().SetVertexDeclaration(K_SHM_PNCT4T4);
	UTGetShaderManager().SetVSConstantF(0, (float*)&matWVP, 4);
	UTGetShaderManager().SetVSConstantF(4, (float*)fConstDataVS, ARRAY_SIZE(fConstDataVS));
	// PS
	UTGetShaderManager().SetPSByName(L"PS_IESLIGHT");

	for (int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++)
	{
		CLight *nl = m_visibleList.visible_lights.m_pData[kk];

		if ((nl->type != K_LVL_LT_IES) || (nl->castShadows))
			continue;

		//the IES dot texture has 3 pixel lines per IES profile so we don't get interpolation problems
		float IES_texV = (float)(nl->nProfileID * 3 + 1) / (float)pIESTex->info.Height;
		//set Pshader constants
		float fConstData[][4] = {
			//x:light intensity, y:light radius, z: IES profile (V in texture coordinates)
			{ nl->fIntensity, nl->fRadius, IES_texV, 0.0f },
			// x: game height projection, y: height projection inverse (projected -> real), z: gauss dist atten factor
			{ ZHSCALE, INV_ZHSCALE, ct_fGaussLen, 0.0f },
			// xyz: light world position
			{ nl->vPos.x, nl->vPos.y, nl->vPos.z, 0.0f },
			// light direction normalized
			{ nl->vnDir.x, nl->vnDir.y, nl->vnDir.z, 0.0f }
		};
		UTGetShaderManager().SetPSConstantF(0, (float*)fConstData, ARRAY_SIZE(fConstData));
		m_bufferedPainter.DrawMesh(nl->m_nLightMeshIdx, false);
	}

	UTGetShaderManager().SetVS(nullptr);
	UTGetShaderManager().SetPS(nullptr);


	AdditiveBlendingOFF(m_pDevice, NULL);
	// end sprite painter
	UTPainter().End();


	return K_OP_OK;
}

OPRESULT CLevel::RenderPass_Composition(Mat* matProj)
{
	Mat				matView;

	RECTXYWH_F		camrect = m_camLevel.GetCamWorldAABB();
	CAABB			camAABB(camrect.x, camrect.y, camrect.Right(), camrect.Bottom());
	///----------------------------------------------------
	/// INITIAL SETUP
	///----------------------------------------------------
	m_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);

	m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	if ((UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0)
	{
		//#IMPORTANT: we need separate alpha blending or it will look bad when blending alpha values between them.
		//eg: if we blend semitransparent things on top of fully opaque walls, the walls become transparent
		//necessary but not really well supported. Better with a shader and custom sprite painter
		m_pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
		m_pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA);
		m_pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_DESTALPHA);
		m_pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
	}

	MUMatAffine2D(&matView, K_GAME_PIXEL_SIZE_F, NULL, 0.0f, &Vec2(-camrect.x * K_GAME_PIXEL_SIZE_F, -camrect.y * K_GAME_PIXEL_SIZE_F));
	m_pDevice->SetTransform(D3DTS_VIEW, &matView);
	m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);

	///--- compose scene from normals and color ---
	PVERTEXSHADER pVShader = null;
	PPIXELSHADER pPShader = null;

	Mat matWVP = matView * (*matProj);

	//--- build RT rect ---
	_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
	vul.pos = Vec3(camAABB.vMin.x, camAABB.vMin.y, 0.0f);
	vur.pos = Vec3(camAABB.vMax.x, camAABB.vMin.y, 0.0f);
	vdl.pos = Vec3(camAABB.vMin.x, camAABB.vMax.y, 0.0f);
	vdr.pos = Vec3(camAABB.vMax.x, camAABB.vMax.y, 0.0f);

	vul.tex1 = vul.tex2 = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
	vur.tex1 = vur.tex2 = Vec4(1.0f, 0.0f, 0.0f, 0.0f);
	vdl.tex1 = vdl.tex2 = Vec4(0.0f, 1.0f, 0.0f, 0.0f);
	vdr.tex1 = vdr.tex2 = Vec4(1.0f, 1.0f, 0.0f, 0.0f);
	//set color
	vul.color = vur.color = vdl.color = vdr.color = 0xffffffff;
	//build verts
	_VERTEX_PNCT4T4 lightRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
	lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
	lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;

	CRTManager::CEngineRenderTarget* pRTcolor = UTGetRTManager().GetRTbyUID(K_RTID_TEMP1);
	if (pRTcolor != null)
		m_pDevice->SetTexture(0, pRTcolor->m_pRTTexture);
	CRTManager::CEngineRenderTarget* pRTlights = UTGetRTManager().GetRTbyUID(K_RTID_COLORDEPTHSTENCIL);
	if (pRTlights != null)
		m_pDevice->SetTexture(1, pRTlights->m_pRTTexture);

	pVShader = UTGetShaderManager().GetVShaderByName(L"VS_COMPOSITION");
	m_pDevice->SetVertexShader(pVShader);
	m_pDevice->SetVertexDeclaration(UTGetShaderManager()._VERTEX_PNCT4T4_decl);
	m_pDevice->SetVertexShaderConstantF(0, (float*)&matWVP, 4);

	pPShader = UTGetShaderManager().GetPShaderByName(L"PS_COMPOSITION");
	m_pDevice->SetPixelShader(pPShader);
	// set Pshader constants
	float fGamma = 2.2f;
	float fConstData[][4] = {
		// x:gamma, y:1.0f/gamma
		{ fGamma, 1.0f / fGamma, ct_fLightMul, ct_fColorDodge}
	};
	m_pDevice->SetPixelShaderConstantF(0, (float*)fConstData, ARRAY_SIZE(fConstData));
	m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 2, &lightRectV, sizeof(_VERTEX_PNCT4T4));
	// remove VS PS
	m_pDevice->SetVertexShader(nullptr);
	m_pDevice->SetPixelShader(nullptr);

	return K_OP_OK;
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
	Mat matCam = m_camLevel.GetViewTransform(); //matricea camerei

	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	//1. paint level background
	//PaintBackground();
	//--- paint thunder ---
	if ((m_fThunderTimer > 0.0f) && (m_fThunderTimer < 0.4f) && (randint(1000) < 500) && (!UTGetGUI().bIsBlocking) && (!DXUTIsTimePaused()))
	{
	}


	//set textures, states and shaders
	//m_pDevice->SetTexture(0, m_pRTTexture);
	//#HACK: daca am mai multe texturi de lumina trebuie schimbat settexture sa ia pentru fiecare lumina textura ei. Daca am o singura textura merge foarte bine asa
	m_pDevice->SetTexture(1, m_sprLights.Textures[0]->pTex);

	Mat matWVP = matCam * UTGetAppClass().g_matProj;
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
		if (nl->type == K_LVL_LT_AMBIENTAL)
		{
			//paint and exit
			m_bufferedPainter.DrawMesh(nl->m_nLightMeshIdx, false);
			break;
		}
	}
	//set textures, states and shaders
	//m_pDevice->SetTexture(0, m_pRTTexture);
	//#HACK: daca am mai multe texturi de lumina trebuie schimbat settexture sa ia pentru fiecare lumina textura ei. Daca am o singura textura merge foarte bine asa
	m_pDevice->SetTexture(1, m_sprLights.Textures[0]->pTex);

	///--- paint back without self illumination ---
	/*
	//RECT srcrect;
	//SetRect(&srcrect, 0, 0, m_visibleAreaTL.w * tileW, m_visibleAreaTL.h * tileH);
	//Vec3 bgpos(m_visibleArea.x, m_visibleArea.y, 0.0f);
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
		if (nl->type == K_LVL_LT_AMBIENTAL)
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
			DrawRectUP_TL1T(m_pDevice, rct, Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), 0xffffffff);
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
//		m_pDevice->SetTexture(0, m_pRTTexture);
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
	Mat matCam = m_camLevel.GetViewTransform();
	//CAABB al camerei
	CAABB		camAABB;
	camAABB.Set(Vec2(camrect.x, camrect.y), Vec2(camrect.Right(), camrect.Bottom()));

	///--- paint water ---
	if (m_bufferedPainter.GetTrisCount(m_waterMeshIdx) > 0)
	{
		//set textures, states and shaders
		_ASSERT(m_waterTexIdx >= 0);

//		m_pDevice->SetTexture(0, m_pRTTexture_final);
		m_pDevice->SetTexture(1, m_texManager.m_Texs[m_waterTexIdx]->pTexture); //textura apa

		Mat matWVP = matCam * UTGetAppClass().g_matProj;
		//vertex shaderul e acelasi pt toate
		pVShader = UTGetShaderManager().GetVShaderByName(L"VS_WATER");
		m_pDevice->SetVertexShader(pVShader);
		m_pDevice->SetVertexDeclaration(UTGetShaderManager()._VERTEX_PNCT4T4_decl);

		float fang = fLocalTimeline;
		if (fang >= 1000.0f * PI)
			fang -= 1000.0f * PI;
		Vec2 woff1(0.05f * sin(fang * 1.0f), 0.04f * cos(fang * 1.0f));
		Vec2 woff2(0.5f - 0.06f * sin(-fang * 0.63f), 0.5f - 0.05f * cos(-fang * 0.67f));
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

	///--- paint crosshairs 
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if (pPlayerActor[kk] == null)
			continue;

		// paint aiming cursor
		// vAimVec was normalized using last frame data so paint it at last frame actor position
		Vec2 vto = pPlayerActor[kk]->pos_last + pPlayerActor[kk]->m_AIcommands.vAimVec;
		CSprite::paintFrame(&m_sprInterface, vto.x, vto.y, ANM_IGM_INTERFACE_SPR_IGM_STRATEGIC_EFFECTS, 4, 0xffffffff);
	}
								  
	///--- actors icons and stun stars ---
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
			Vec2 vStarsPos = act->GetPosHeart();
			CSprite::paintFrameModule(&g_particlesMgr.m_sprCol, vStarsPos.x, vStarsPos.y - 10.0f, ANM_PARTICLES_SPR_STUN_STARS, curframe, 0, act->color);
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

//		m_pDevice->SetTexture(0, m_pRTTexture_final);
		m_pDevice->SetTexture(1, null);// m_texManager.m_Texs[m_fogofwarTexIdx]->pTexture); //textura FOW

		Mat matWVP = matCam * UTGetAppClass().g_matProj;
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

	Mat matfront;
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

		Vec3 campos = m_camLevel.GetCamPos();
		Vec2 off(obj->pos.x - campos.x, obj->pos.y - campos.y);
		off *= K_LVL_FRONTLAYER_PARALLAX; //front layer moves faster
										  //compute final aabb - visibility test
		CAABB finalaabb = obj->aabb_ini;
		finalaabb.vMin *= K_LVL_FRONTLAYER_SCALING; finalaabb.vMax *= K_LVL_FRONTLAYER_SCALING;
		finalaabb.Move(obj->pos + off);
		if (!finalaabb.Intersects(&camAABB))
			continue;

		MUMatAffine2D(&matfront, K_LVL_FRONTLAYER_SCALING, NULL, 0.0f, &(obj->pos + off));
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

			Vec2 vpos(activ->bbox_exported.vCenter.x, activ->bbox_exported.vMin.y);
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
					CtrlMgrDrawProgress_HeadsOutside(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_PROGRESS_RED_GLOW_HO, recttemp, perc, 0xffffffff);
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
						CtrlMgrDrawProgress_HeadsOutside(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_PROGRESS_RED_GLOW_HO, recttemp, perc, 0xffffffff);
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
					CtrlMgrDrawProgress_HeadsOutside(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_PROGRESS_RED_GLOW_HO, recttemp, perc, 0xffffffff);
				}
			}
		}

		//cover shield
		if (player->pCover != null)
		{
			Vec2 vpos = Vec2(pPlayerActor[kk]->bbox.vCenter.x, pPlayerActor[kk]->bbox.vMin.y);
			CSprite::paintFrame(&UTGetGUI().m_sprCol, vpos.x, vpos.y, ANM_CONTROLS_SPR_PLAYER_ICONS, 0, pPlayerActor[kk]->color);
		}
		else if (UTGetAppClass().m_Settings.bShowInterfaceHelp) //player numeric icon (only if shield not visible)
		{
			Vec2 vpos = Vec2(pPlayerActor[kk]->bbox.vCenter.x, pPlayerActor[kk]->bbox.vMin.y + fabs(3.0f * sin(fLocalTimeline * 4.0f)));
			CSprite::paintFrame(&m_sprInterface, vpos.x, vpos.y, ANM_IGM_INTERFACE_SPR_PLAYER_NR_ICONS, pPlayerActor[kk]->nPlayerOrdinal);
		}

		//paint player numeric icon on multiplayer when peer outside the screen
		if (UTGetAppClass().IsGameNetworked())
		{
			if ((pPlayerActor[kk]->nPlayerOrdinal == g_netlock.Net_GetOtherPlayerIndex()) && (!camAABB.Intersects(&pPlayerActor[kk]->bbox)))
			{
				Vec2 vpos = Vec2(pPlayerActor[kk]->bbox.vCenter.x, pPlayerActor[kk]->bbox.vMin.y);
				CAABB localAABB = camAABB;
				localAABB.Inflate(-K_TILE_SIZE + fabs(3.0f * sin(fLocalTimeline * 4.0f)), -K_TILE_SIZE + fabs(3.0f * sin(fLocalTimeline * 4.0f)));
				if (AABB_Segment_Intersection(vpos, camAABB.vCenter, localAABB, &vpos))
				{
					float fAng = HALF_PI + UTMath::GetVectorAngle(camAABB.vCenter - vpos);
					Mat matrt;
					MUMatAffine2D(&matrt, 1.0f, NULL, fAng, &vpos);
					m_pSprite->SetTransform(&matrt);
					CSprite::paintFrame(&m_sprInterface, 0.0f, 0.0f, ANM_IGM_INTERFACE_SPR_PLAYER_NR_ICONS, 2 + pPlayerActor[kk]->nPlayerOrdinal);
					m_pSprite->SetTransform(&g_matIdentity);
					CSprite::paintFrame(&m_sprInterface, vpos.x, vpos.y, ANM_IGM_INTERFACE_SPR_PLAYER_NR_ICONS, 4 + pPlayerActor[kk]->nPlayerOrdinal);
				}
			}
		}
	}

	m_pSprite->Flush();

	//paint text bubble
	m_interfaceTextBubble.Paint(m_pDevice, m_pSprite);

	//set screen space
	CCameraTransform::SetActiveCamera(m_pDevice, &UTGetAppClass().g_camScreen);

	///--- paint time slowdown screen effect ---
	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	if (m_fTimeMultiplier_real < 1.0f)
	{
		DWORD colEffect = D3DCOLOR_COLORALPHA(0xff000088, 1.0f - m_fTimeMultiplier_real);
		Mat mattrans;
		RECTXYWH_F bbox = UTGetGUI().m_sprCol.GetAFrameBBox_real(ANM_CONTROLS_SPR_VIGNETTES, 1);
		MUMatAffine2D(&mattrans, UTGetAppClass().g_rectRender.h / bbox.h, NULL, 0.0f, &UTGetAppClass().g_rectRender.Center());
		m_pSprite->SetTransform(&mattrans);
		CSprite::paintFrame(&UTGetGUI().m_sprCol, 0.0f, 0.0f, ANM_CONTROLS_SPR_VIGNETTES, 1, colEffect);
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

	mapMesh.Release();

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
	m_arrPropsPtrInteract.Clear();
	SAFE_DELETE_GROWABLE_ARRAY(m_arrProps);
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
	m_poolDoofers.Release();
	m_poolPhysPts.Release();

	m_sprLights.Release();
	m_sprProps.Release();
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



int CLevel::BuildLightVolume360(CLight * light, _VERTEX_PNCT4T4 *outVerts, int outVertsMaxCnt)
{
	_ASSERT(outVerts != null);
	// sends 360 rays and finds collisions with tileset base of walls. When we collide with a wall facing the camera we also add polys for the wall.
	// could be optimized: if we collide with same tile then we just move last point instead of adding another.

	struct sCollPoint {
		Vec2 vPos;
		Vec2 vNorm;
		POINTXY_INT tlPos;
		bool bCollided;
	};

	const int	nSteps = 360;
	int			nVertCnt = 0;
	// collisions array
	sCollPoint	arrColl[nSteps];
	int			arrCollCur = 0;

	float		fAngStep = DOUBLE_PI / (float)nSteps;
	float		fAng = 0.0f;

	float fMaxRad = max(light->bbox_ini.vHalfSize.x, light->bbox_ini.vHalfSize.y);
	Vec2 vFrom = Vec3ToVec2XY(light->vPos);
	Vec3 vFrom3 = Vec2ToVec3XY0(vFrom);
	// collision results
	Vec2 vRetPt(0.0f, 0.0f), vRetNrm(0.0f, 0.0f);
	POINTXY_INT tilePosTL;
	// counts how many collisions of the same type (no collision or same tile) were made in order
	int nSameSince = 0;

	for (int kk = 0; kk < nSteps; kk++)
	{
		Vec2 vdir(cos(fAng), sin(fAng));
		Vec2 vTo = vFrom + vdir * fMaxRad;
		if (SegmentTilesIntersection(vFrom, vTo, vRetPt, vRetNrm, &tilePosTL))
		{
			// are we still on the same tile, same kind of collision? take a step back and overwrite last value
			
			if ((tilePosTL == arrColl[arrCollCur - 1].tlPos) && (vRetNrm == arrColl[arrCollCur - 1].vNorm))
				nSameSince++;
			else
				nSameSince = 0;
			// make sure we use the first different collision (from nothing to wall for example) so it doesn't cut corners
			if(nSameSince > 1)
				arrCollCur--;
			
			// fix wiggling corners (snap to tile corners when colliding visible wall)
			float chkX1 = tilePosTL.x * K_TILE_SIZE_F, chkX2 = chkX1 + K_TILE_SIZE_F;
			if (vRetNrm.y > 0.0f)
			{
				if (fabs(vRetPt.x - chkX1) <= 1.5f)
					vRetPt.x = chkX1;
				else if (fabs(vRetPt.x - chkX2) <= 1.5f)
					vRetPt.x = chkX2;
			}

			arrColl[arrCollCur].bCollided = true;
			arrColl[arrCollCur].vPos = vRetPt;
			arrColl[arrCollCur].vNorm = vRetNrm;
			arrColl[arrCollCur].tlPos = tilePosTL;
			arrCollCur++;
		}
		else
		{
			// optimizes so it just adds one triangle every N collisions
			/*
			nSameSince++;
			if (nSameSince > 5)
				nSameSince = 0;
			if (nSameSince > 1)
				arrCollCur--;
			  */
			// add end of ray
			arrColl[arrCollCur].bCollided = false;
			arrColl[arrCollCur].vPos = vTo;
			arrColl[arrCollCur].vNorm = Vec2(0.0f, 0.0f);
			arrColl[arrCollCur].tlPos = Vec2i(-1, -1);
			arrCollCur++;
		}
		// increase angle
		fAng += fAngStep;
	}

	// create triangles (skip first point, will be handled last)
	for (int kk = 1; kk <= arrCollCur; kk++)
	{
		_ASSERT(nVertCnt < outVertsMaxCnt);

		sCollPoint* pt = &arrColl[kk % arrCollCur];
		sCollPoint* ptold = &arrColl[(kk - 1) % arrCollCur];

		Vec3 ptpos(pt->vPos.x, pt->vPos.y, 0.0f);
		Vec3 ptoldpos(ptold->vPos.x, ptold->vPos.y, 0.0f);

		outVerts[nVertCnt].pos = vFrom3; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;
		outVerts[nVertCnt].pos = ptpos; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;
		outVerts[nVertCnt].pos = ptoldpos; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;

		// extend on wall
		if ((pt->bCollided) && (ptold->bCollided) && (pt->tlPos.y == ptold->tlPos.y) &&
			(pt->vNorm.y >= 1.0f) && (ptold->vNorm.y >= 1.0f))
		{
			// add 2 tris per wall segment
			Vec3 vWallH(0.0f, -K_WALL_HEIGHT_SCREEN, 0.0f);

			outVerts[nVertCnt].pos = ptpos; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;
			outVerts[nVertCnt].pos = ptpos + vWallH; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;
			outVerts[nVertCnt].pos = ptoldpos; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;

			outVerts[nVertCnt].pos = ptpos + vWallH; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;
			outVerts[nVertCnt].pos = ptoldpos; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;
			outVerts[nVertCnt].pos = ptoldpos + vWallH; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;
		}
	}

	return nVertCnt;
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

void CLevel::GiveStrategicPoints(float fPoints, Vec2 * vPos)
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
	_ASSERT((fPoints >= 0.0f) && (fPoints <= (float)K_LVL_MAX_STRATEGIC_POINTS));
	float fPointsGiven = LIMIT(fPoints, 0.0f, (float)K_LVL_MAX_STRATEGIC_POINTS);

	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if (pPlayerActor[kk] == null)
			continue;
		
		int fMaxPoints = K_LVL_MAX_STRATEGIC_POINTS;

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

		g_particlesMgr.AddStringParticle(g_font5ns2, strPart, vPos, NULL, &Vec2(0.0f, -20.0f), 1.2f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.4f, 0xcc21aec2, K_PART_LAYER_NORMAL);
	}

}


bool CLevel::IsLineOfSight(Vec2 pt1, Vec2 pt2, Vec2 * retVecCollisionPt /*= null*/, Vec2 * retVecCollisionNormal /*= null*/)
{
	Vec2 collisionPoint, collisionNormal;
	//before enemies attacked each other too, here was checking with closeby collisions
	CCollisionShape* colShape = ColShape_Segment_Intersection_Arr(pt1, pt2, m_visibleList.logic_colShapesExtended.m_pData, m_visibleList.logic_colShapesExtended.Count(), retVecCollisionPt, retVecCollisionNormal);
	if (colShape != NULL)
	{
		return false;
	}
	return true;
}



///--- DECALS ---
void CLevel::AddDecal(EDecalLayer nLayer, Vec2 pos, int animIdx, int frameIdx /*= 0*/, DWORD color /*= 0xffffffff*/, bool bIsAnimated /*= false*/)
{
	CDecal *ndec = new CDecal();

	ndec->layer = nLayer;
	ndec->sprite.Init(animIdx, (int)pos.x, (int)pos.y, frameIdx, color);
	RECTLTRB_F framerect = m_sprProps.GetAFrameBBox_real(animIdx, frameIdx);
	ndec->aabb.Set(Vec2(framerect.left + pos.x, framerect.top + pos.y), Vec2(framerect.right + pos.x, framerect.bottom + pos.y));
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
			m_arrDecals[kk]->sprite.Update(&m_sprProps, dTime);
			//remove animation flag when anim ends
			if (m_arrDecals[kk]->sprite.animStatus == ANIM_STATUS_FRAMELOCK)
				m_arrDecals[kk]->bAnimated = false;
		}
	}
}

void CLevel::AddDecal_BloodSplat(Vec2 pos, bool bLarge, EActorClass eVictimClass)
{
	/*
	//blood splats are sorted by size (ascending)
	switch (eVictimClass)
	{
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
	*/
}

void CLevel::UpdatePhysicsPoints(float dTime)
{
	CLinkedPool<CPhysicsPoint>::CLinkedPoolNode *node = m_poolPhysPts.pListUsed.m_pNext;
	while (node != &m_poolPhysPts.pListUsed)
	{
		CLinkedPool<CPhysicsPoint>::CLinkedPoolNode *nextnode = node->m_pNext;
		//update
		CPhysicsPoint*	point = &node->m_data;
		// kill it when it gets outside the play area
		if (!PointInRect(Vec3ToVec2XY(point->pos), m_levelAABB))
		{
			point->bIsDead = true;
			point->bIsStatic = true;
		}

		// what forces act on the point
		Vec3			vecForces = point->accel;
		bool			bWasContacting = point->bContacting;

		point->contactType = K_COLLTYPE_NONE;
		point->bContacting = false;
		point->pContactShape = NULL;
		point->bContactStarted = false;
		//save last pos
		point->pos_last = point->pos;

		///--- integrator
		//integrator
		if (point->bIsStatic)
			vecForces = g_Vec3Zero;
		if (point->bIsStaticZ)
			vecForces.z = 0.0f;

		point->speed += vecForces * dTime;
		point->pos += point->speed * dTime;


		///--- check collisions
		{
			Vec2 collisionPoint, collisionNormal;
			Vec2 vFrom = Vec3ToVec2XY(point->pos_last);
			Vec2 vTo = Vec3ToVec2XY(point->pos);
			Vec2 vMove = vTo - vFrom;

			eRetContactType contactT = K_COLLTYPE_NONE;
			CCollisionShape* colShape = nullptr;
			
			// XY plane collision
			bool bCollided = false;
			float fMinContactDistance = 100000.0f;
			if ((vMove.x != 0.0f) && (vMove.y != 0.0f))
			{
				// tiles collision
				if (point->nFlagsCollision & K_LVL_PHYSP_COLLFLAG_TILES)
				{
					if (SegmentTilesIntersection(vFrom, vTo, collisionPoint, collisionNormal))
					{
						bCollided = true;
						contactT = K_COLLTYPE_TILE;
						fMinContactDistance = MUVec2Len(&(vFrom - collisionPoint));
					}
				}

				// collision with shapes 
				if (point->nFlagsCollision & K_LVL_PHYSP_COLLFLAG_BOXES)
				{
					// (overwrite collpoint ONLY if closer and make other optimizations to see if we CAN collide with anything)
				// get only the boxes in vMove box
				//colShape = ColShape_Segment_Intersection_Arr(point->pos_last, point->pos, m_visibleList.logic_colShapes.m_pData, m_visibleList.logic_colShapes.Count(), &collisionPoint, &collisionNormal);
				}
			}

			if (bCollided)
			{
				point->pos.x = collisionPoint.x + collisionNormal.x;
				point->pos.y = collisionPoint.y + collisionNormal.y;

				point->bContacting = true;
				point->pContactShape = colShape;
				point->contactNormal = Vec2ToVec3XY0(collisionNormal);
				point->contactPos = Vec3(collisionPoint.x, collisionPoint.y, point->pos.z);
				point->contactType = contactT;

				if (point->bFlagPhysicsEnabled)
				{
					float fDot = MUVec3Dot(&point->speed, &point->contactNormal);
					Vec3 Vn = point->contactNormal * fDot;
					Vec3 Vt = point->speed - Vn;
					// compute final speed
					point->speed = -Vn + Vt; 
				}
				else
				{
					point->speed = g_Vec3Zero;
				}
			}

			// Minimum speed on Z when we consider the point stopped
			const float fMinSpeedZ = 0.1f;
			// Current floor height. #MAYBE: should get it from each tile
			float fFloorH = 0.0f;

			// Z floor collision at the end to bring it back up
			// Only compute this part if we have vertical acceleration and speed
			//#MAYBE: should check collision with ceiling too
			if ((point->accel.z != 0.0f) && (point->speed.z != 0.0f) && (point->pos.z <= fFloorH))
			{
				point->bContacting = true;
				// walls collisions have priority so only set normals if no other collision happened
				if (!bCollided)
				{
					point->contactNormal = Vec3(0.0f, 0.0f, -1.0f);
					point->contactPos = point->pos;
					point->pContactShape = nullptr;
					point->contactType = K_COLLTYPE_FLOOR;
				}
				// get the point back above the floor
				point->pos.z = fFloorH - point->pos.z;

				if (point->bFlagPhysicsEnabled)
				{
					// make sure it always ricochets upwards
					point->speed.z = fabs(point->speed.z * point->fBounceF);

					if (fabs(point->speed.z * dTime) < fMinSpeedZ)
					{
						point->speed.z = 0.0f;
						point->pos.z = fFloorH;
						point->bIsStaticZ = true;
					}
					// apply friction
					point->speed.x -= point->speed.x * point->fFrictionF * dTime;
					point->speed.y -= point->speed.y * point->fFrictionF * dTime;
				}
				else
				{
					point->speed = g_Vec3Zero;
				}
			}

			//check bounce or first contact - mainly for sounds and particles
			if (bWasContacting == false)
			{
				point->bContactStarted = true;
			}

			// is it almost stopped?
			if (MUVec3AlmostZero(point->speed * dTime, 0.5f))
			{
				point->bIsStatic = true;
				point->speed = g_Vec3Zero;
			}
			else
			{
				point->bIsStatic = false;
			}
		}


		//avansez pointer
		node = nextnode;
	}
}



void CLevel::GenerateEffect(ELVLEffectType nEffectType, Vec2 pos, float fSize, DWORD color)
{
	switch (nEffectType)
	{
		case K_LVL_EFFECT_STONE_BREAK:
		{
			g_particlesMgr.GenerateSmokePuff(Vec2(pos.x, pos.y - 10.0f), 20.0f, K_PART_LAYER_RT_FRONT_NRM);
			m_camLevel.ShakeScreen(2.0f, 8.0f, &pos);

//			SND_PLAY_POSITIONAL(SNDIDX_STONE_BREAK1, pos);
		}
		break;
		case K_LVL_EFFECT_EXPLODING_ZOMBIE_DIE:
		{
			AddDoofer_Explo(hash_EXPLO_INVISIBLE_EXPLODING_ZOMBIE, pos, 0, K_LVL_ACT_CLASS_ZOMBIE);
			//throw slimes
			CWeaponTemplate* wpnTemplate = GetTemplateWeapon(FastHash(L"WPN_GREEN_GOO_EXPLODING_ZOMBIE"));
			if (wpnTemplate != null)
			{
				for (int kk = 0; kk < 6; kk++)
				{
					float ang = kk * (PI / 6.0f);
					Vec2 vdir(cos(ang), -sin(ang));

					ShootBullet(&wpnTemplate->bulletTemplate, K_LVL_ACT_CLASS_ZOMBIE, 0, Vec2(pos.x, pos.y), vdir);
				}
			}
			//gibs
			//blood splat (sortate crescator in animatie)
			AddDecal_BloodSplat(pos, true, K_LVL_ACT_CLASS_ZOMBIE);
//			SND_PLAY_POSITIONAL_RAND2(SNDIDX_BULLET_BODY_GIBBED_01, SNDIDX_BULLET_BODY_GIBBED_02, pos);
			CAABB genbox(pos.x - 10.0f, pos.y - 15.0f, pos.x + 10.0f, pos.y);
			for (int ll = 0; ll < 6; ll++)
			{
				AddDoofer(K_DOOFER_MEAT, AABB_GetRandomPointInBox(genbox), &Vec2(randfloatsgn(50.0f), -130.0f - randfloat(100.0f)), &g_vecGravityOld, 1);
			}
			//goes straight down to stain the floor
			AddDoofer(K_DOOFER_MEAT, Vec2(pos.x, pos.y - 10.0f), &Vec2(200.0f, 50.0f), &g_vecGravityOld, 1);
			AddDoofer(K_DOOFER_MEAT, Vec2(pos.x, pos.y - 10.0f), &Vec2(-200.0f, 50.0f), &g_vecGravityOld, 1);
			//human blood gibs
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_HUMAN_SPLAT_MED, true, 0, &pos, NULL, NULL, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xff1a3423, K_PART_LAYER_RT_FRONT_NRM);
		}
		break;
		case K_LVL_EFFECT_EXPLO_LARGE:
		{
			AddDoofer_Explo(hash_EXPLO_LARGE_XL, pos, 0, K_LVL_ACT_CLASS_EXPLOSION);
		}
		break;
		case K_LVL_EFFECT_ELECTRIC_BREAK_SPARKS:
		{
//			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.4f, 0.1f, 0x88FDB727, 1.0f);
			//particule sparkle
			for (int kk = 0; kk < 20; kk++)
			{
				g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRESPARK2, true, randint(2), &Vec2(pos.x + randfloatsgn(fSize), pos.y + randfloatsgn(fSize)), &g_vecGravityOld, &Vec2(randfloatsgn(60.0f), -10.0f - randfloat(40.0f)), 0.2f + randfloat(0.4f), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM_LIGHT, 1.5f, kk * 0.025f);
			}
		}
		break;
		case K_LVL_EFFECT_STARS_CONFETTI:
		{
//			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.6f, 0.2f, 0x88FDB727, 3.0f * fSize);
			//fire ring
			for (int kk = 0; kk < 30; kk++)
			{
				float ang = randfloat(DOUBLE_PI);
				Vec2 vdir(cos(ang), sin(ang));
				if (randompercent(50.0f))
					g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRESPARK1, true, randint(2), &(pos + vdir * 10.0f), NULL, &(vdir * (40.0f + randfloat(20.0f))), 1.0f + randfloat(0.5f), 1.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.5f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM_LIGHT, 2.0f);
				else
					g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRESPARK2, true, 0, &(pos + vdir * 10.0f), NULL, &(vdir * (40.0f + randfloat(20.0f))), 1.0f + randfloat(0.5f), 1.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.5f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM_LIGHT, 2.0f);
			}

			//linii verticale
			for (int kk = 0; kk < 6; kk++)
			{
				g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_TELEPORT, false, 5 + randint(2), &Vec2(pos.x + randfloatsgn(8.0f), pos.y - 3), NULL, &Vec2(0.0f, -60.0f - randfloat(20.0f)), 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.2f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM_LIGHT, 0.0f, kk * 0.1f);
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

void CLevel::GenerateEffect(CStringHash sEffectName, Vec2 pos, float fSize, DWORD color)
{
	ELVLEffectType effectidx = (ELVLEffectType)GetListIndexByNameHash(sEffectName.textHash, ELVLEffectTypeNames, K_LVL_EFFECTS_CNT);
	GenerateEffect(effectidx, pos, fSize, color);
}

void CLevel::TouchClosestActive(CActor * pToucherAct, float dTime)
{
	_ASSERT(pToucherAct != null);

	if(pToucherAct->pClosestTouchable != null)
	{
		pToucherAct->pClosestTouchable->Touch(pToucherAct->GetUID(), dTime);
	}
}

#define K_CLIP_OCCLUDERS_TO_LIGHT
int CLevel::GetOccluderSegments(Vec2 vEye, CAABB bbox, COccluderSegment* pRetArr, int maxRetArrSize)
{
	_ASSERT(pRetArr != nullptr && maxRetArrSize > 0);

	int nCur = 0;

	Vec2 vPos = vEye;
	Vec2 vNYp(0.0f, 1.0f), vNYn(0.0f, -1.0f), vNXp(1.0f, 0.0f), vNXn(-1.0f, 0.0f);
	///--- add segments from bboxes
	//check only the occluders in the visible area as we don't process lights outside the screen
	for (int kk = 0; kk < m_visibleList.visible_colShapesLights.Count(); kk++)
	{
		_ASSERT(nCur < maxRetArrSize - 2);
		// if we want to clip occluders to light bbox:
#ifdef K_CLIP_OCCLUDERS_TO_LIGHT
		CAABB retbb;
		CAABB* chkbb = &retbb;
		// clipped check (looks better with longer occluders):
		if (AABB_Intersection(bbox, m_visibleList.visible_colShapesLights.m_pData[kk]->bbox, retbb))
#else
		// if we want the whole bbox:
		CAABB* chkbb = &m_visibleList.visible_colShapesLights.m_pData[kk]->bbox;
		// non clipped check (faster):
		if (lbox.Intersects(chkbb)) 
#endif
		{
			if (vEye.y > chkbb->vMax.y)
			{
				pRetArr[nCur++].Set(Vec2(chkbb->vMin.x, chkbb->vMax.y), chkbb->vMax, vNYp, vPos, m_visibleList.visible_colShapesLights[kk]->ID, K_WALL_HEIGHT_SCREEN);
			}
			else if (vEye.y < chkbb->vMin.y)
			{
				pRetArr[nCur++].Set(Vec2(chkbb->vMax.x, chkbb->vMin.y), chkbb->vMin, vNYn, vPos);
			}

			if (vEye.x > chkbb->vMax.x)
			{
				pRetArr[nCur++].Set(chkbb->vMax, Vec2(chkbb->vMax.x, chkbb->vMin.y), vNXp, vPos);
			}
			else if (vEye.x < chkbb->vMin.x)
			{
				pRetArr[nCur++].Set(chkbb->vMin, Vec2(chkbb->vMin.x, chkbb->vMax.y), vNXn, vPos);
			}
		}
	}
		
	///--- add occluders from tiles, optimizing for same wall lines
	Vec2i tlmin(floor(bbox.vMin.x / K_TILE_SIZE_F), floor(bbox.vMin.y / K_TILE_SIZE_F));
	Vec2i tlmax(floor(bbox.vMax.x / K_TILE_SIZE_F), floor(bbox.vMax.y / K_TILE_SIZE_F));
	if (tlmin.x < 0) tlmin.x = 0;
	if (tlmin.y < 0) tlmin.y = 0;
	if (tlmax.x > levelSizeTL.w - 1) tlmax.x = levelSizeTL.w - 1;
	if (tlmax.y > levelSizeTL.h - 1) tlmax.y = levelSizeTL.h - 1;

	for (int yy = tlmin.y; yy <= tlmax.y; yy++)
	{
		for (int xx = tlmin.x; xx <= tlmax.x; xx++)
		{
			_ASSERT(nCur < maxRetArrSize - 2);

			CTile* tl = &tiles[xx][yy];
			CAABB chkbb(xx * K_TILE_SIZE_F, yy * K_TILE_SIZE_F, (xx + 1) * K_TILE_SIZE_F, (yy + 1) * K_TILE_SIZE_F);
#ifdef K_CLIP_OCCLUDERS_TO_LIGHT
			// clip horizontally, do it in a fast way just so we don't miss wall intersections when colliders go outside the light bbox
			if (chkbb.vMax.x > bbox.vMax.x) chkbb.vMax.x = bbox.vMax.x;
			if (chkbb.vMin.x < bbox.vMin.x) chkbb.vMin.x = bbox.vMin.x;
			// ignore vertically for now, it errors but not so much as to be visible
			//if (chkbb.vMax.y > bbox.vMax.y) chkbb.vMax.y = bbox.vMax.y;
			//if (chkbb.vMin.y < bbox.vMin.y) chkbb.vMin.y = bbox.vMin.y;
#endif

			// can the tile cast shadows
			if (tl->flags & K_TILEFLAG_HASWALL_MASK)
			{
				if ((tl->flags & K_TILEFLAG_HASWALL_D) && (vEye.y > chkbb.vMax.y))
				{
					//optimize same wall: check last wall and if it's the same just make the occluder longer
					if ((nCur > 0) && (pRetArr[nCur - 1].dwWallID == yy) && (pRetArr[nCur - 1].vEnd.x == chkbb.vMin.x))
						pRetArr[nCur - 1].MoveEnd(chkbb.vMax, vPos);
					else
						/*ID is wall Y in tileset plus a value to not collide with the collbox ids */
						pRetArr[nCur++].Set(Vec2(chkbb.vMin.x, chkbb.vMax.y), chkbb.vMax, vNYp, vPos, yy, K_WALL_HEIGHT_SCREEN);
				}
				else if ((tl->flags & K_TILEFLAG_HASWALL_U) && (vEye.y < chkbb.vMin.y))
				{
					//optimize same wall: check last wall and if it's the same just make the occluder longer
					if ((nCur > 0) && (pRetArr[nCur - 1].dwWallID == yy) && (pRetArr[nCur - 1].vStart.x == chkbb.vMin.x))
						pRetArr[nCur - 1].MoveStart(Vec2(chkbb.vMax.x, chkbb.vMin.y), vPos);
					else
						pRetArr[nCur++].Set(Vec2(chkbb.vMax.x, chkbb.vMin.y), chkbb.vMin, vNYn, vPos, yy, 0.0f);
				}

				if ((tl->flags & K_TILEFLAG_HASWALL_R) && (vEye.x > chkbb.vMax.x))
				{
					pRetArr[nCur++].Set(chkbb.vMax, Vec2(chkbb.vMax.x, chkbb.vMin.y), vNXp, vPos);
				}
				else if ((tl->flags & K_TILEFLAG_HASWALL_L) && (vEye.x < chkbb.vMin.x))
				{
					pRetArr[nCur++].Set(chkbb.vMin, Vec2(chkbb.vMin.x, chkbb.vMax.y), vNXn, vPos);
				}
			}
		}
	}

	///--- add light range segments (don't set normals so we don't extend the walls on it)
	// add them last so we prioritize intersecting with the others first
	_ASSERT(nCur < maxRetArrSize - 4);
	pRetArr[nCur++].Set(bbox.vMin, Vec2(bbox.vMax.x, bbox.vMin.y), vNYp, vPos);
	pRetArr[nCur++].Set(Vec2(bbox.vMin.x, bbox.vMax.y), bbox.vMax, vNYn, vPos);
	pRetArr[nCur++].Set(bbox.vMin, Vec2(bbox.vMin.x, bbox.vMax.y), vNXp, vPos);
	pRetArr[nCur++].Set(Vec2(bbox.vMax.x, bbox.vMin.y), bbox.vMax, vNXn, vPos);

	return nCur;
}

CCollisionShape * CLevel::GetCollisionShapeAt(Vec2 point, int collisionType)
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

CCollisionShape* CLevel::SpawnCollisionShape(int nType, Vec2 vMin, Vec2 vMax)
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


///--- framework implementations ---
#pragma region FRAMEWORK_IMPL
OPRESULT CLevel::OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc, void* pUserContext)
{
	HRESULT hr = S_OK;
	m_pDevice = pDevice;

	V_OP_HRTOOP(m_sprLights.OnCreateDevice(pDevice));
	V_OP_HRTOOP(m_sprProps.OnCreateDevice(pDevice));
	V_OP_HRTOOP(m_sprActors.OnCreateDevice(pDevice));
	V_OP_HRTOOP(m_sprInterface.OnCreateDevice(pDevice));
	V_OP_HRTOOP(m_texManager.OnCreateDevice(pDevice));
	V_OP_HRTOOP(m_bufferedPainter.OnCreateDevice(pDevice));
	V_OP_RET(mapMesh.OnCreateDevice(pDevice));
	return K_OP_OK;
}

OPRESULT CLevel::OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc, void* pUserContext)
{
	HRESULT hr = S_OK;
	m_pDevice = pDevice;

	V_OP_HRTOOP(m_sprLights.OnResetDevice(pDevice));
	V_OP_HRTOOP(m_sprProps.OnResetDevice(pDevice));
	V_OP_HRTOOP(m_sprActors.OnResetDevice(pDevice));
	V_OP_HRTOOP(m_sprInterface.OnResetDevice(pDevice));
	V_OP_HRTOOP(m_texManager.OnResetDevice(pDevice));
	V_OP_HRTOOP(m_bufferedPainter.OnResetDevice(pDevice));
	V_OP_RET(mapMesh.OnResetDevice(pDevice));

	return K_OP_OK;
}

OPRESULT CLevel::OnLostDevice(void* pUserContext)
{
	m_pDevice = NULL;

	m_sprLights.OnLostDevice();
	m_sprProps.OnLostDevice();
	m_sprActors.OnLostDevice();
	m_sprInterface.OnLostDevice();
	m_texManager.OnLostDevice();

	m_bufferedPainter.OnLostDevice();
	mapMesh.OnLostDevice();

	return K_OP_OK;
}

OPRESULT CLevel::OnDestroyDevice(void* pUserContext)
{
	m_pDevice = NULL;

	m_sprLights.OnDestroyDevice();
	m_sprProps.OnDestroyDevice();
	m_sprActors.OnDestroyDevice();
	m_sprInterface.OnDestroyDevice();
	m_texManager.OnDestroyDevice();

	m_bufferedPainter.OnDestroyDevice();
	mapMesh.OnDestroyDevice();

	return K_OP_OK;
}

#pragma endregion FRAMEWORK_IMPL


#pragma warning(pop)
