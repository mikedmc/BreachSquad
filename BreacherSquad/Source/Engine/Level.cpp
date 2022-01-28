#include "dxstdafx.h"

//saves warnings stack
#pragma warning(push)
//disable warning
//#pragma warning(disable : 4706)  //assignment within conditional expression


CBulletHitReturnData CLevel::HitActor(CActor* actor, CBullet *pBullet, Vec2* pvProjectileMomentum)
{
	CBulletHitReturnData retData;
 	retData.eMaterial = actor->actTemplate.eMaterial;
	retData.bPenetratedShield = false;
	retData.bKilledTarget = false;
	retData.bArmorHit = false;

	if ((actor == null) || (pBullet == null))
	{
		ErrorBox(K_ERR_WARNING, L"CLevel::HitActor invalid params!");
		return retData;
	}

	bool bGoreEnabled = UTApp().m_Settings.bGoreEnabled;

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
		//actor->nTookDamageFrames = 4;
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
		if (((pBullet->nFlags & K_LVL_BULLET_FLAG_IGNORE_ARMOR) == 0) && (pvProjectileMomentum != null) && (actor->fArmor > 0.0f) ) 
		{
			int nActorAR = 1;
			//melee damage is treated differently
			if (pBullet->nFlags & K_LVL_BULLET_FLAG_MELEE)
			{
				//melee ignores armor usually but if armor hase melee resistance then it takes first from the armor and then from life
				float fDmgToArmor = fHitPointsTaken * 1;
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

			//scade shield points din armor
			actor->fArmor -= fShieldPointsTaken;
			//took too much armor? get extra armor taken from life
			if (actor->fArmor <= 0.0f)
			{
				fLifeTaken += -actor->fArmor;
				actor->fArmor = 0.0f;
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
			fBulletLostEnergy = actor->actTemplate.fLife * 0.1f;

		//transmit bullet momentum daca nu sunt under cover
		if ((actor->actTemplate.fMass > 0.0f) && (pvProjectileMomentum))
		{
			actor->vSpeedImpulse += *pvProjectileMomentum / actor->actTemplate.fMass;
		}
		
		//subtract life	if no invincibility
		if (actor->cDamageOverTime.eType == CDamageOverTime::K_LVL_DoT_INVINCIBLE)
			fLifeTaken = 0.0f;

		if (actor->actTemplate.actorClass == K_LVL_ACT_CLASS_PLAYER)
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
		if ((actor->fLife > 0.0f) && (fLifeTaken >= actor->actTemplate.fLife * 0.1f))
//			PlayActorSoundVerse(actor, K_LVL_ACT_VERSE_TAKING_DAMAGE, true);

		//life left in it?
		if (actor->fLife > 0.0f)
		{
			//mesaj LOW_HEALTH - la 10% din viata originala
			float fLifeLowLimit = actor->actTemplate.fLife * 0.1f;
			if ((actor->fLife < fLifeLowLimit) && (actor->fLife + fLifeTaken >= fLifeLowLimit))
			{
				AddAIEvent(K_LVL_AI_EVENT_LOW_HEALTH, 0, pBullet->actorClass, actor->GetPosHeart(), 10000.0f, 0.6f, actor->GetUID());
			}

			//adaugam si stun
			if (actor->fStunTimer < pBullet->fStunDuration)
			{
				SetActorStun(actor, pBullet->fStunDuration);
			}
		}
	}

	//event got_hit
	if ((actor->fLife > 0.0f) && (actor->actTemplate.actorClass > K_LVL_ACT_CLASS_PLAYER))
	{
		//adaug eventuri de GOT_HIT doar pe clasele HUMAN, cand sunt lovite de catre player
		//find shooter pos. defaults on pos based on bullet speed
		/*
		Vec3 evtpos = actor->GetPosHeart();
		if (pvProjectileMomentum != null)
			evtpos -= *pvProjectileMomentum;

		CActor* pPlayer = GetPlayerByUID(pBullet->ownerUID);
		if (pPlayer)
			evtpos = pPlayer->GetPosHeart();
		
		//only add "got hit" events for enemy classes
		if (pBullet->actorClass >= K_LVL_ACT_CLASS_EXPLOSION)
		{
			AddAIEvent(K_LVL_AI_EVENT_GOT_HIT, pBullet->ownerUID, pBullet->actorClass, evtpos, -1.0f, 1.2f, actor->GetUID());
		}
		*/
	}

	//set dead AI on humans
	if ((actor->fLife <= 0.0f) && (actor->actTemplate.eMaterial == K_LVL_MATERIAL_FLESH))
	{
		//give strategic points on death
		if ((fOldLife > 0.0f) && (actor->actTemplate.actorClass >= K_LVL_ACT_CLASS_HUMAN))
		{
			//you get points if enemy killed by player or explo
			if (((pBullet->actorClass == K_LVL_ACT_CLASS_PLAYER) || (pBullet->actorClass == K_LVL_ACT_CLASS_EXPLOSION)) && (actor->actTemplate.actorClass != K_LVL_ACT_CLASS_PLAYER))
			{
				if (actor->UID != pBullet->ownerUID)
				{
					GiveStrategicPoints(1.0f, &Vec2(actor->bbox.vCenter.x, actor->bbox.vMin.y));
				}
			}
		}

		//cadavers get pushed more by kicking them
		if ((fOldLife > 0.0f) && (actor->actTemplate.fMass > 0.0f) && (pvProjectileMomentum != null))
			actor->vSpeedImpulse += K_LVL_DEAD_BODY_BULLET_MOMENTUM_MULTIPLIER * (*pvProjectileMomentum / actor->actTemplate.fMass);

		//erase shooting flags
		actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;

		bool bSplatActor = false;
		
		//very low life from the first hit? splat!
		if ((pBullet->nFlags & K_LVL_BULLET_FLAG_CAN_SPLAT) && (actor->GetCurrentBehavior() != AI_BEHAVIOR_DEAD) && (pBullet->actorClass == K_LVL_ACT_CLASS_PLAYER) && (actor->fLife < -actor->actTemplate.fLife * 0.5f))
		{
			bSplatActor = true;
			//if bullets lose power then only splat from close quarters
			if ((pBullet->fDamageLossPPx > 0.0f) && ((pBullet->fLife / pBullet->fLife_ini) < 0.9f))
				bSplatActor = false;
		}
		//grenades splat dead bodies
		if ((actor->GetCurrentBehavior() == AI_BEHAVIOR_DEAD) && (pBullet->actorClass == K_LVL_ACT_CLASS_EXPLOSION) && (fLifeTaken >= actor->actTemplate.fLife))
				bSplatActor = true;
		//if dead but you keep kicking him it explodes
		if ((actor->GetCurrentBehavior() == AI_BEHAVIOR_DEAD) && (pBullet->nFlags & K_LVL_BULLET_FLAG_CAN_SPLAT) && (actor->fLife < -actor->actTemplate.fLife))
			bSplatActor = true;
		//if lucky cancel splat
		if (m_rand.RandInt(100) <= 10)
		{
			bSplatActor = false;
			actor->fLife = 0.0f;
		}

		//--- generate blood splats on death ---
		if ((fOldLife > 0.0f) && (actor->actTemplate.eMaterial == K_LVL_MATERIAL_FLESH))
		{
			//splaturile sunt sortate in fn de marime (folosesc posHeart in log de GetPosHeart() pentru ca altfel imi da deja pozitia de dupa moarte, adica prea jos)
			//splaturile sunt sortate in functie de dimensiune (crescator)
			if (bGoreEnabled)
			{
				if ((pBullet->nFlags & K_LVL_BULLET_FLAG_NO_DECALS) == 0)
				{
					AddDecal_BloodSplat(actor->GetPosHeart(), true, actor->actTemplate.actorClass);
				}
			}

			retData.bKilledTarget = true;
			//say shooter verse
			CActor* pShooter = GetActorByUID(pBullet->ownerUID);
			if (pShooter != null)
			{
//				PlayActorSoundVerse(pShooter, K_LVL_ACT_VERSE_KILL_MADE);
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
	if ((actor->actTemplate.actorClass != K_LVL_ACT_CLASS_HUMAN) && (actor->actTemplate.actorClass != K_LVL_ACT_CLASS_FRIENDLY))
		return;

	if ((actor->actTemplate.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET) != 0)
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
		Weapon_StopReloading(actor->pWeaponMain);
		bInterrupting = true;

		//only count stunned enemies
		if (actor->actTemplate.actorClass == K_LVL_ACT_CLASS_HUMAN)
			App_IncreaseGamestat(K_MEMID_GAMESTATS_ENEMIES_STUNNED);
	}
	//reset actions
	actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
	//blochez arma principala la orice fel de stun (ca sa nu traga nici cand are stun mic)
	Weapon_Jam(actor->pWeaponMain);
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

	CActor* nact = SpawnActor(spawnPos, L"act_assaulter1.xml");

	if (nact)
	{
		pPlayerActor[nPlayerOrdinal] = nact;

		//set controller
		pPlayerActor[nPlayerOrdinal]->nPlayerOrdinal = nPlayerOrdinal;
		pPlayerActor[nPlayerOrdinal]->nControllerInstanceID = m_arrPlayerControllersIIDs[nPlayerOrdinal];
	}

	//save last safe position as spawn position
	m_arrPlayerLastSafePos[nPlayerOrdinal] = spawnPos;

	//update backup template
	nact->actTemplate_ini = nact->actTemplate;

	//run ON_SPAWN script
	/*
	if (!templateLocal.shScript_OnSpawn.IsEmpty())
	{
		StartScript(templateLocal.shScript_OnSpawn.getHash(), nact);
	}
	*/

	//animate player on spawn (only if told otherwise by nAnimset=-1)
	/*
	if (nAnimset >= 0)
	{
		nact->SetAnimSet(nAnimset);
		SetActorAIState(nact, L"JOIN_GAME");
		//AddProp_Light(nact->GetPosHeart(), ANM_LIGHTS_SPR_POINT1, 0.5f, 0.1f, 0x8888ff00, 1.0f);

		//set invulnerability
		SetActorDoT(nact, CDamageOverTime::K_LVL_DoT_INVINCIBLE, 2.0f, 0.0f, K_LVL_ACT_CLASS_ANY, K_LVL_ACT_CLASS_ANY, 0);

		//SND_PLAY(SNDIDX_UI_PLAYER_JOIN);
	}
	*/

	//set skin
	//nact->nSkinIdx = nPlayerOrdinal;

	//resetam numarul de puncte strategice si scoatem selectia
	m_arrStats[K_LVL_STATS_PL1_STRATEGIC_POINTS + nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT] = 0;
	//m_interfaceIGM.SetStrategicPoints(m_arrStats[K_LVL_STATS_PL1_STRATEGIC_POINTS] / 1000.0f, m_arrStats[K_LVL_STATS_PL2_STRATEGIC_POINTS] / 1000.0f);
	//m_interfaceIGM.SetStrategicSelection(nPlayerOrdinal, -1);
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

CActor* CLevel::SpawnActor(Vec2 spawnPos, WCHAR* strTemplateFileName, CStringHash* shStateOverride)
{
	CActorTemplate* acttemplate = Actor_LoadTemplate(strTemplateFileName);
	if (acttemplate == null)
	{
		ErrorBox(K_ERR_WARNING, L"LoadLevel::Actor_GetTemplate - invalid template name: %s", strTemplateFileName);
		return null;
	}

	//copy template locally and customize it based on gear selection
	CActorTemplate templateLocal = *acttemplate;
	templateLocal.FillDefaultValuesIfNotSet();
	///--- set weapons and gear modifiers ---
	UINT32 namehash = 0;
	//equipment	- add equipment template
	/*
	namehash = 0; //equipment hash name
	if (namehash != 0)
	{
	///ADD EQUIPMENT TEMPLATE
	CActorTemplate* acttempl = Actor_GetTemplate(namehash);
	if (acttempl != null)
	{
	templateLocal.AddGenericDataFromTemplate(acttempl);
	templateLocal.OverwriteAnimsFromTemplate(acttempl);
	}
	}
	//gear
	namehash = 0; //gear template hash
	CWeaponTemplate* wGear = Weapon_GetTemplate(namehash);
	if ((wGear != nullptr) && (!wGear->bPassive))
	{
	templateLocal.weaponTypeGear = wGear->name;
	}
	if ((wGear != nullptr) && (!wGear->shTemplateOverwrite.IsEmpty()))
	{
	///ADD GEAR TEMPLATE
	CActorTemplate* acttempl = Actor_GetTemplate(wGear->shTemplateOverwrite.getHash());
	if (acttempl != null)
	{
	templateLocal.AddGenericDataFromTemplate(acttempl);
	templateLocal.OverwriteAnimsFromTemplate(acttempl);
	}
	}
	*/
	///LAST! PRIMARY WEAPON TEMPLATE GETS ADDED WHEN CHANGING WEAPONS (equiping main weapon)
	/// We don't add it here so it saves the actor->template_ini without the equipped weapons
	//alt fire from primary weapon
	/*
	namehash = 0;
	CWeaponTemplate* wPrimaryALT = Weapon_GetTemplate(namehash);
	if ((wPrimaryALT != nullptr) && (!wPrimaryALT->bPassive))
	{
	templateLocal.weaponTypeAlt = wPrimaryALT->name;
	}
	//get selected primary weapon
	namehash = 0;
	CWeaponTemplate* wPrimary = Weapon_GetTemplate(namehash);
	if ((wPrimary != nullptr) && (!wPrimary->bPassive))
	{
	templateLocal.weaponType = wPrimary->name;
	}
	*/

	CActor* nact = new CActor(spawnPos, &templateLocal, GenerateNextID(), new CSpineAnimComponent());
	// create a weapon and add it to the player's arsenal
	CWeapon* wpn = Weapon_Create(L"WPN_SMG_MP5A3", nact);
	nact->AddWeapon(wpn, true);
	// initialize AI
	Actor_SetAIState(nact, nact->actTemplate.AItemplate->GetAIStateByName(nact->actTemplate.shAIState_ini));

	//finish up adding the actor
	nact->PostConstructionInit();
	m_arrActors.Add(nact);

	nact->BeginPlay();

	return nact;
}

CProp* CLevel::SpawnProp(CLevelArea* pArea, Vec2 spawnPos, int nAnimIdx, int nFrameIdx)
{
	_ASSERT(pArea != nullptr);

	CProp* obj = new CProp();

	obj->ID = GenerateNextID();
	//pozitia
	obj->pos = spawnPos;
	obj->pos_ini = obj->pos;
	//anim
	int animIdx = nAnimIdx;
	int frameIdx = nFrameIdx;
	obj->sprite.Init(&m_sprProps, animIdx, obj->pos.xy_proj, frameIdx,  0xffffffff);
	obj->fid_ini.Init(animIdx, frameIdx);
	obj->sprite.color = obj->color;
	//angle
	//obj->fAngle = obj->fAngle_ini = 0.0f;
	//load flags and split
	UINT32 activFlags = 0;
	//flip xy
	//obj->flipX = ((activFlags & K_EDITOR_ACTIVE_FLAG_FLIPX) != 0);
	//obj->flipY = ((activFlags & K_EDITOR_ACTIVE_FLAG_FLIPY) != 0);
	//animated
	//obj->bAnimated = ((activFlags & K_EDITOR_ACTIVE_FLAG_ANIMATED) != 0);
	//cand e animat selecteaza random frame-ul de pornire
	if (obj->bAnimated)
	{
		obj->sprite.frameIdx = m_rand.RandInt(m_sprProps.GetAFramesCnt(obj->sprite.animIdx));
	}
	//bbox
	RECTXYWH bbox_set = m_sprProps.GetAFrameBBox(animIdx, frameIdx);
	RECTXYWH objbox = m_sprProps.GetAFrameBBox_real(animIdx, frameIdx);
	obj->bbox_ini.Set(objbox);
	obj->bbox_floor_ini.Set(bbox_set);
	//daca e flipat pe X flipez si bbox. Pe Y nu e cazul pt ca se pastreaza in acelasi bbox in paint
	/*
	if (obj->flipX)
	{
		obj->bbox_ini.Move(Vec2(-2.0f * obj->bbox_ini.vCenter.x, 0.0f));
		obj->bbox_floor_ini.Move(Vec2(-2.0f * obj->bbox_floor_ini.vCenter.x, 0.0f));
	}
	*/

	//load logic
	obj->bCanInteract = false;
	obj->bHideInteractIcon = false;
	//start hidden
	obj->bHidden = obj->bSetHidden = false;

	obj->targetID_ini = -1;
	obj->AIstate = K_AI_STATE_UNDEFINED;
	// add to specified area
	obj->PostConstructionInit();
	pArea->m_arrProps.Add(obj);

	return obj;
}

CLight*	CLevel::SpawnLight(Vec3 spawnPos, eLightType eType, DWORD dwColor, float fRadius, int profileID, bool bCastShadows)
{
	CLight *nl = new CLight();
	nl->ID = GenerateNextID();
	nl->type = eType;
	nl->fVolumeAlpha = 1.0f;
	nl->fIntensity = 1.0f;
	nl->pos = spawnPos;
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
	CCollisionShape* pCol = GetCollisionShapeAt(active->pos.xy);
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

		if (pActiv->bbox.Intersects(active->bbox))
			nScore--;
	}
	//interactible actors
	for (int kk = 0; kk < m_visibleList.logic_actors_closeby.nCount; kk++)
	{
		CActor* pAct = m_visibleList.logic_actors_closeby.m_pData[kk];
		if (!pAct->bCanInteract)
			continue;

		if (pAct->bbox.Intersects(active->bbox))
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
		if ((rectToAvoid != null) && (rectToAvoid->Intersects(rectCheck)))
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
		if (ColShape_CAABB_Intersect_Arr(rectCheck, m_visibleList.logic_colShapes.m_pData, m_visibleList.logic_colShapes.Count()) != null)
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


UINT32 CLevel::GenerateNextID()
{
	m_unLastID++; //last ID always stays on a new ID
	return (m_unLastID - 1);
}

CLevel::CLevel()
{
	m_unLastID = 10000000;

	m_bufferedPainter.Init(4000);
		
	m_bLoaded = false;
	m_bOneUpdateDone = false;

	m_dwSyncCheckHash = 0;

	fLocalTimeline = 0.0f;

	m_pDevice = NULL;
	tileW = tileH = 0;

	m_levelAABB.Set(0.0f, 0.0f, 0.0f, 0.0f);
	m_levelAABB_TL.Set(0, 0, 0, 0);
	//bullets
	m_propsLightsMeshIdx = -1;

	//indexuri texturi
	m_pTexTilesNorm = nullptr;
	m_pTexTilesColor = nullptr;
	//fog of war
	m_fogofwarMeshIdx = -1;
	m_bulletsMeshIdx = -1;
	//level states
	m_levelState = K_LVL_STATE_PLAYING;
	m_levelSubState = 0;
	m_levelStateTimer = 0.0f;

	m_camLevelToRT.SetViewport(UTApp().g_rectRT);
	m_camLevelToScr.SetViewport(UTApp().g_rectRender);

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


void CLevel::UpdateDirtyRects()
{
	//#TODO: doesn't change WALKABLE floor flags, that should be done during loading or level editing for speed
	//#TODO: should make sure the level always has a 1 tile border!
	///--- compute tile flags ---
	for (auto rect : m_arrDirtyRectsTL)
	{
		for (int ii = 0; ii < m_arrAreas.GetSize(); ii++)
		{
			CLevelArea* area = m_arrAreas[ii];
			if (!rect.Intersects(area->AABBbounds_TL))
				continue;
			// take border tiles into account:
			// clamp to smaller size because we check neighbours
			RECTXYWH lrect = rect;
//			area->AABBbounds_TL.Intersects(
			// clamp and bring rectangle to local space
			lrect.IntersectWith(area->AABBbounds_TL);
			if ((lrect.w == 0) || (lrect.h == 0))
				continue;

			for (int yy = lrect.y; yy < lrect.y + lrect.h; yy++)
			{
				for (int xx = lrect.x; xx < lrect.x + lrect.w; xx++)
				{
					CTile* tl = area->GetTile(xx, yy);
					// neighbours
					CTile* tlL = area->GetTile(xx - 1, yy);
					CTile* tlR = area->GetTile(xx + 1, yy);
					CTile* tlU = area->GetTile(xx, yy - 1);
					CTile* tlD = area->GetTile(xx, yy + 1);
					///--- set wall flags on non walkable tiles
					if ((tl->flags & K_TILEFLAG_WALKABLE) == 0)
					{
						// clear flags
						FLAGOP_CLEAR(tl->flags, K_TILEFLAG_HASWALL_MASK);

						if ((tlL) && (IS_FLAG_ANY(tlL->flags, K_TILEFLAG_WALKABLE)))
						{
							tl->flags |= K_TILEFLAG_HASWALL_L;
						}
						if ((tlR) && (IS_FLAG_ANY(tlR->flags, K_TILEFLAG_WALKABLE)))
						{
							tl->flags |= K_TILEFLAG_HASWALL_R;
						}
						if ((tlU) && (IS_FLAG_ANY(tlU->flags, K_TILEFLAG_WALKABLE)))
						{
							tl->flags |= K_TILEFLAG_HASWALL_U;
						}
						if ((tlD) && (IS_FLAG_ANY(tlD->flags, K_TILEFLAG_WALKABLE)))
						{
							tl->flags |= K_TILEFLAG_HASWALL_D;
						}
					}


					///--- compute wall shadows
					// it can only receive if it's a floor or a wall but not a ceiling on that tile
					tl->nShadowFrame = -1;

					// only walls and floor get shadowed, when having a non walkable tile on the left (hole in the floor usually, but not water hole)
					bool bCanReceive = ((tl->tileIDs[K_TILE_LAYER_FLOOR] >= 0) || (tl->tileIDs[K_TILE_LAYER_WALLS] >= 0)) &&
						(tlL) && (tlL->tileIDs[K_TILE_LAYER_FLOOR] < 0) && 
						(tl->tileIDs[K_TILE_LAYER_CEILING] < 0);
					if (bCanReceive)
					{
						CTile* tlDL = area->GetTile(xx - 1, yy + 1);
						int nCasterH = 0;
						if (tlL->tileIDs[K_TILE_LAYER_CEILING] >= 0) nCasterH = 3;
						else if (tlL->tileIDs[K_TILE_LAYER_WALLS] >= 0)
						{
							if ((tlDL) && (tlDL->tileIDs[K_TILE_LAYER_WALLS] >= 0))
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
	}

	// finished with dirty rects, clear the array
	m_arrDirtyRectsTL.clear();
}

int CLevel::Areas_UpdateVisibility(RECTXYWH_F camRect)
{
	int nVisible = 0;
	for (int ii = 0; ii < m_arrAreas.GetSize(); ii++)
	{
		CLevelArea* area = m_arrAreas[ii];
		if (area->UpdateVisibility(camRect))
			nVisible++;
	}
	return nVisible;
}

OPRESULT CLevel::Areas_PaintLayer(eAreaLayer layerIdx)
{
	for (int ii = 0; ii < m_arrAreas.GetSize(); ii++)
	{
		CLevelArea* area = m_arrAreas[ii];
		V_OP_RET(area->areaMesh.PaintLayer(layerIdx));
	}
	return K_OP_OK;
}

std::vector<CLevelArea*> CLevel::Areas_GetAreasInRect(CAABB aabb)
{
	vector<CLevelArea*> retarr;
	for (int ii = 0; ii < m_arrAreas.GetSize(); ii++)
	{
		CLevelArea* area = m_arrAreas[ii];
		if (area->AABBbounds.Intersects(aabb))
			retarr.push_back(area);
	}
	return retarr;
}

CLevelArea* CLevel::Areas_GetAt(Vec2 vPos)
{
	for (int ii = 0; ii < m_arrAreas.GetSize(); ii++)
	{
		CLevelArea* area = m_arrAreas[ii];
		if (area->AABBbounds.PointIn(vPos))
			return area;
	}
	return nullptr;
}

CLevelArea* CLevel::Areas_GetByID(UINT32 nID)
{
	for (int ii = 0; ii < m_arrAreas.GetSize(); ii++)
	{
		CLevelArea* area = m_arrAreas[ii];
		if (area->ID == nID)
			return area;
	}
	return nullptr;
}

void CLevel::Areas_GetTilesSnapshot(RECTXYWH srcRectTL, CTile** arrTiles, int arrCapacity)
{
	_ASSERT(arrTiles != nullptr);
	if ((srcRectTL.w * srcRectTL.h) > arrCapacity)
	{
		ErrorBox(K_ERR_WARNING, L"Areas_GetTilesSnapshot:: array too small!");
		return;
	}
	// clear array
	memset(arrTiles, 0, sizeof(CTile*) * arrCapacity);
	int nCur = 0;
	// scan all areas one by one
	for (int ii = 0; ii < m_arrAreas.Count(); ii++)
	{
		CLevelArea* area = m_arrAreas[ii];
		RECTXYWH rectloc = area->AABBbounds_TL;
		rectloc.IntersectWith(srcRectTL);
		if ((rectloc.w <= 0) || (rectloc.h <= 0))
			continue;
		// bring to area space
		rectloc.Move(-area->AABBbounds_TL.x, -area->AABBbounds_TL.y);
		for (int yy = rectloc.y; yy < rectloc.y + rectloc.h; yy++)
		{
			for (int xx = rectloc.x; xx < rectloc.x + rectloc.w; xx++)
			{
				// brings tile from world pos to srcRectTL relative pos
				Vec2i vPosTL(xx + area->AABBbounds_TL.x - srcRectTL.x, yy + area->AABBbounds_TL.y - srcRectTL.y);
				int nidx = vPosTL.x + vPosTL.y * srcRectTL.w;
				_ASSERT(nidx < arrCapacity);
				// debug checkup:
				//_ASSERT(xx >= 0 && yy >= 0 && xx < area->AABBbounds_TL.w && yy < area->AABBbounds_TL.h);
				arrTiles[nidx] = &area->tiles[xx][yy];
			}
		}
	}
}

OPRESULT CLevel::GetScriptAction(const WCHAR* strID, CScriptAction& retAction)
{
	CStringHash shID(strID);
	for (auto scra : m_arrActionTemplates)
	{
		if (scra.shID == shID)
		{
			retAction = scra;
			return K_OP_OK;
		}
	}

	return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"GetScriptAction:: Could not find action: %s", strID);
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

CActorTemplate* CLevel::Actor_LoadTemplate(WCHAR * strTemplateFileName)
{
	// LOAD ACTOR TEMPLATE LoadActorTemplate
	char strbuff[MAX_PATH] = { 0 };

	//does it exist?
	CActorTemplate* templ = Actor_GetTemplate(strTemplateFileName);
	if (templ != null)
	{
		LOG_DBG(L"ActTemplates_Add - reusing template: %s", strTemplateFileName);
		return templ;
	}

	// build file path
	WCHAR Path[MAX_PATH];
	WCHAR wcsPath[MAX_PATH];
	StringCchPrintf(wcsPath, MAX_PATH, L"media/levels/data/actors/%s", strTemplateFileName);
	FileManager::GetMediaPath(wcsPath, Path);

	//does not exist, open xml
	pugi::xml_document doc;
	if (!doc.load_file(Path))
	{
		ErrorBox(K_ERR_WARNING, L"Unable to load actor template XML:%s\n", strTemplateFileName);
		return null;
	}

	//load actor templates
	pugi::xml_node rootnode = doc.root().child(L"ACTOR");

	templ = new CActorTemplate();
	templ->shID.Init(strTemplateFileName);
	templ->shSkeletonXML.Init(rootnode.attribute(L"sSkeletonTemplateXML").value());
	if (templ->shSkeletonXML.IsEmpty())
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING] Template skeleton template XML not set!\n%s", templ->shSkeletonXML.text);
		SAFE_DELETE(templ);
		return null;
	}
	//read skin name (if any)
	if (!rootnode.attribute(L"sSkin").empty())
		templ->shSkinName.Init(rootnode.attribute(L"sSkin").value());

	//ACTOR_DATA node
	pugi::xml_node actnode = rootnode.child(L"ACTOR_DATA");
	// bbox and heights
	float xmin = actnode.attribute(L"bboxMinX").as_float();
	float ymin = actnode.attribute(L"bboxMinY").as_float();
	float xmax = actnode.attribute(L"bboxMaxX").as_float();
	float ymax = actnode.attribute(L"bboxMaxY").as_float();
	templ->bbox.Set(xmin, ymin, xmax, ymax);
	templ->fHeight = actnode.attribute(L"nHeight").as_float();

	if (!actnode.attribute(L"fSpeedMove").empty()) { templ->fSpeedMove = actnode.attribute(L"fSpeedMove").as_float(); }
	//life
	if (!actnode.attribute(L"fLife").empty())
		templ->fLife = actnode.attribute(L"fLife").as_float();
	if (!actnode.attribute(L"fArmor").empty())
		templ->fArmor = actnode.attribute(L"fArmor").as_float();
	//caps
	templ->eCaps = 0;
	if (actnode.attribute(L"bCanCover").as_bool())
		templ->eCaps |= CActorTemplate::K_ACT_CAPS_CAN_COVER;
	if (actnode.attribute(L"bCanInteract").as_bool())
		templ->eCaps |= CActorTemplate::K_ACT_CAPS_CAN_INTERACT;
	//other
	if (!actnode.attribute(L"sWeapon").empty())
	{
		templ->shWeaponDefault.Init(actnode.attribute(L"sWeapon").value());
	}
	if (!actnode.attribute(L"sAIstate").empty())
	{
		templ->shAIState_ini.Init(actnode.attribute(L"sAIstate").value());
	}

	//anims
	pugi::xml_node anmnode = rootnode.child(L"ANIMS");
	if (anmnode != NULL)
	{
		for (int kk = 0; kk < K_SD_ANIMS_CNT; kk++)
		{
			pugi::xml_node nmnode = anmnode.child(ESpineAnimNames[kk].text);
			if (nmnode != NULL)
			{
				// read anim names
				for (int nset = 0; nset < K_ACT_ANIM_MAX_SETS; nset++)
				{
					WCHAR strSetName[MAX_PATH];
					StringCchPrintf(strSetName, MAX_PATH, L"set%d", nset);

					if (!nmnode.attribute(strSetName).empty())
					{
						templ->arrAnims[kk].animNamesA[nset].Init(nmnode.attribute(strSetName).value());
					}

					// first set is mandatory:
					if (nset == 0)
					{
						if (templ->arrAnims[kk].animNamesA[nset].IsEmpty())
						{
							ErrorBox(K_ERR_WARNING, L"[WARNING] Template set0 animation not found!\n %s: %s", templ->shSkeletonXML.text, ESpineAnimNames[kk].text);
						}
					}
				}
				// read looping flag
				templ->arrAnims[kk].bLooping = true;
				if (!nmnode.attribute(L"bLoop").empty())
				{
					templ->arrAnims[kk].bLooping = nmnode.attribute(L"bLoop").as_bool();
				}
			}
		}
	}

	//sound verses
	pugi::xml_node versenode = rootnode.child(L"VERSES");
	if (versenode != NULL)
	{
		for (int kk = 0; kk < K_LVL_ACT_VERSES_COUNT; kk++)
		{
			pugi::xml_node nmnode = versenode.child(EActorSoundVerseNames[kk].text);
			if (nmnode != NULL)
			{
				if (!nmnode.attribute(L"set0").empty())
				{
					templ->soundIDs[kk][0] = UTGetSoundManager().getSndIdxW(nmnode.attribute(L"set0").value());
					if ((!nmnode.attribute(L"set0").empty()) && (templ->soundIDs[kk][0] == -1))
					{
						//#TEMP: until I change the templates
						//ErrorBox(K_ERR_WARNING, L"Template set0 sound not found!\n%s", nmnode.attribute(L"set0").value());
					}
				}
				//variation
				if (!nmnode.attribute(L"set1").empty())
				{
					templ->soundIDs[kk][1] = UTGetSoundManager().getSndIdxW(nmnode.attribute(L"set1").value());
					if ((!nmnode.attribute(L"set1").empty()) && (templ->soundIDs[kk][1] == -1))
					{
						//#TEMP: until I change the templates
						//ErrorBox(K_ERR_WARNING, L"Template set1 sound not found!\n%s", nmnode.attribute(L"set1").value());
					}
				}
			}
		}
	}

	//create local AI template copy
	CAITemplate* aitemplate = new CAITemplate();

	//AI ignored events
	pugi::xml_node aiignorenode = rootnode.child(L"AI_IGNORE_EVENTS");
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
	pugi::xml_node ainode = rootnode.child(L"AI");
	if (ainode != NULL)
	{
		// parse all states
		for (pugi::xml_node statenode = ainode.first_child(); statenode; statenode = statenode.next_sibling())
		{
			CAIState * nstate = new CAIState();
			nstate->name.Init(statenode.attribute(L"name").value());
			nstate->nPriority = statenode.attribute(L"nPriority").as_int();
			// read state probability and set to 100.0 if missing
			nstate->fProbability = statenode.attribute(L"fProbability").as_float();
			if (nstate->fProbability == 0.0f)
				nstate->fProbability = 100.0f;
			// find triggers
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
					if (!behnode.attribute(L"bIgnoreEvents").empty())
						nbeh.bIgnoreEvents = behnode.attribute(L"bIgnoreEvents").as_bool();
					if (!behnode.attribute(L"fBehaviorDuration").empty())
						nbeh.fBehaviorDuration = behnode.attribute(L"fBehaviorDuration").as_float();
					//read all behavior specific attributes
					for (pugi::xml_attribute_iterator ait = behnode.attributes_begin(); ait != behnode.attributes_end(); ++ait)
					{
						// jump over generic params and add all others 
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

			aitemplate->m_arrStates.Add(nstate);
		}
	}
	// add the AItemplate to the list and save pointer to it in the actor template
	m_arrAItemplates.Add(aitemplate);
	templ->AItemplate = aitemplate;

	LOG_DBG(L"ActTemplates_Add - added template: %s", templ->shSkeletonXML.text);

	//finished loading template
	m_arrTemplatesActor.Add(templ);

	return templ;
}


void CLevel::KillActor(CActor * actor, bool bSplatTarget)
{
	CBullet bullet;
	bullet.fDamage = actor->actTemplate.fLife;
	bullet.nFlags |= K_LVL_BULLET_FLAG_IGNORE_ARMOR | K_LVL_BULLET_FLAG_IGNORE_COVER | K_LVL_BULLET_FLAG_NO_IMPACT_PARTICLES | K_LVL_BULLET_FLAG_NOT_BALLISTIC | K_LVL_BULLET_FLAG_NO_DECALS;
	bullet.actorClass = K_LVL_ACT_CLASS_TRAP;

	if (bSplatTarget)
	{
		bullet.nFlags |= K_LVL_BULLET_FLAG_CAN_SPLAT;
		bullet.nFlags &= ~K_LVL_BULLET_FLAG_NO_DECALS;
		//very large damage
		bullet.fDamage = -actor->actTemplate.fLife;
	}

	HitActor(actor, &bullet);
}

CActorTemplate* CLevel::Actor_GetTemplate(const WCHAR * templateName)
{
	UINT32 nameHash = FastHash(templateName);
	//get template now
	for (int kk = 0; kk < m_arrTemplatesActor.GetSize(); kk++)
	{
		if (m_arrTemplatesActor[kk]->shID.getHash() == nameHash)
			return m_arrTemplatesActor[kk];
	}
	return NULL;
}

CActorTemplate* CLevel::Actor_GetTemplate(const DWORD templateNameHash)
{
	if (templateNameHash == 0)
		return NULL;

	for (int kk = 0; kk < m_arrTemplatesActor.GetSize(); kk++)
	{
		if (m_arrTemplatesActor[kk]->shID.getHash() == templateNameHash)
			return m_arrTemplatesActor[kk];
	}

	return NULL;
}

void CLevel::RandomizeTemplateActor(CActorTemplate * actTemplate)
{			   
	// don't randomize player
	if (actTemplate->actorClass == K_LVL_ACT_CLASS_PLAYER)
		return;
	////randomizeaza vitezele cu 10%
	//if(actTemplate->moveMaxSpeed > 0.0f)
	//	actTemplate->moveMaxSpeed += m_rand.RandFloatSgn(actTemplate->moveMaxSpeed * 0.1f);
	//if (actTemplate->moveMinSpeed > 0.0f)
	//	actTemplate->moveMinSpeed += m_rand.RandFloatSgn(actTemplate->moveMinSpeed * 0.1f);
}

///--- IACTIVE ---
IActiveInterface* CLevel::GetIActiveInterfacePtr(int ID)
{
	if (ID < 0)
		return null;
	//check actives
	for (int ar = 0; ar < m_arrAreas.Count(); ar++)
	{
		CLevelArea* area = m_arrAreas[ar];
		for (int kk = 0; kk < area->m_arrProps.GetSize(); kk++)
		{
			if (area->m_arrProps[kk]->ID == ID)
				return area->m_arrProps[kk];
		}
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
	for (int ar = 0; ar < m_arrAreas.Count(); ar++)
	{
		CLevelArea* area = m_arrAreas[ar];
		for (int kk = 0; kk < area->m_arrProps.GetSize(); kk++)
		{
			if (area->m_arrProps[kk]->GetUID() == UID)
				return area->m_arrProps[kk];
		}
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
		float fDist = MUVec2Len(&(pPlayerActor[kk]->pos.xy - sourceActor->pos.xy));
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
		float fDist = MUVec2Len(&(pPlayerActor[kk]->pos.xy - vSrcPos));
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
	if (pPlayer == nullptr)
		return false;

	return (pPlayer->nControllerInstanceID == K_CM_IID_NET1);
}

CProp* CLevel::GetActiveByUID(UINT32 UID)
{
	if (UID == 0)
		return nullptr;
	for (int ar = 0; ar < m_arrAreas.Count(); ar++)
	{
		CLevelArea* area = m_arrAreas[ar];
		for (int kk = 0; kk < area->m_arrProps.Count(); kk++)
		{
			if (area->m_arrProps[kk]->GetUID() == UID)
				return area->m_arrProps[kk];
		}
	}
	return nullptr;
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

			CHAR ctxt[MAX_PATH];
			int nLevelIdx = g_userData[K_MEMID_SELECTED_LEVEL] + g_userData[K_MEMID_SELECTED_CHAPTER] * K_GAME_LEVELS_PER_CHAPTER;
			StringCchPrintfA(ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1);

			if (UTApp().IsGameNetworked())
			{
				//mark sync start here, after loading the game
				UTApp().m_Settings.devnet_eSyncStatus = CApplicationSettings::K_NETGAME_SYNC_GET_READY;
				//send loaded level confirmation
				g_netlock.Net_SendGameplayCommand(g_netlock.K_GAMPLAYCMD_LEVEL_LOADED);

#ifdef ENABLE_CHAT_WINDOW
				//say: "press ENTER to chat"
				g_ChatWnd.AddLine(__Texts().strings[STR_ENTER_TO_CHAT]->sText, L"SYSTEM", K_CW_SYSTEM_COLOR);
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
			if (UTApp().IsGameNetworked())
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
					//m_interfaceIGM.SetStrategicSelection(kk, -1);
					pPlayerActor[kk]->SetIcon(K_LVL_ACT_ICON_NONE);

				}

				if ((m_arrPlayerSelHotJoin[kk] != -1) && (pPlayerActor[kk] == null))
				{
					m_arrPlayerControllersIIDs[kk] = -1;
					m_arrPlayerSelHotJoin[kk] = -1;
					g_playerSelScr.m_arrPlayers[kk].nInstanceID = -1;
					//m_interfaceIGM.SetHotJoinSelection(kk, m_arrPlayerSelHotJoin[kk]);
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
			if (UTApp().IsGameNetworked())
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
					//m_interfaceIGM.SetStrategicSelection(kk, -1);
					pPlayerActor[kk]->SetIcon(K_LVL_ACT_ICON_NONE);

				}
				if ((m_arrPlayerSelHotJoin[kk] != -1) && (pPlayerActor[kk] == null))
				{
					m_arrPlayerControllersIIDs[kk] = -1;
					m_arrPlayerSelHotJoin[kk] = -1;
					g_playerSelScr.m_arrPlayers[kk].nInstanceID = -1;
					//m_interfaceIGM.SetHotJoinSelection(kk, m_arrPlayerSelHotJoin[kk]);
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
				// bring real screen to RT screen space
				Vec2 retpt = m_camLevelToScr.ScreenToWorld(Vec2(fAxisValue, 0.0f));
				// make coords relative to player
				retpt.x -= pPlayer->pos.xyz.x;
				// set final coords
				ret_fAxisValue = retpt.x;
				return true;
			}
			else
			{
				Vec2 retpt = m_camLevelToScr.ScreenToWorld(Vec2(0.0f, fAxisValue));
				// make coords relative to player
				retpt.y -= pPlayer->pos.xyz.y;
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
					int nOccluders = GetOccluderSegments(nl->pos.xy, nl->bbox, arrOccluders, arrOccludersSize);


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
						int retVerts = FOVUtil::BuildOccludedVolume(nl->pos.xy, nl->color, arrOccluders, nOccluders, temp_arrVerts, temp_arrVertsSize);

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
					lcorners[0].x += nl->pos.xy_proj.x; lcorners[0].y += nl->pos.xy_proj.y;
					lcorners[1].x += nl->pos.xy_proj.x; lcorners[1].y += nl->pos.xy_proj.y;
					lcorners[2].x += nl->pos.xy_proj.x; lcorners[2].y += nl->pos.xy_proj.y;
					lcorners[3].x += nl->pos.xy_proj.x; lcorners[3].y += nl->pos.xy_proj.y;
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
				lcorners[0].x += nl->pos.xy.x; lcorners[0].y += nl->pos.xy.y;
				lcorners[1].x += nl->pos.xy.x; lcorners[1].y += nl->pos.xy.y;
				lcorners[2].x += nl->pos.xy.x; lcorners[2].y += nl->pos.xy.y;
				lcorners[3].x += nl->pos.xy.x; lcorners[3].y += nl->pos.xy.y;
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

			case K_LVL_LT_AMBIENTAL:
			{
				// ambiental light only influence the area where they reside, have the bbox the size of the area so we clip to camera rect
				// use BBOX_INI because bbox gets moved to light position
				CAABB realbb; 
				AABB::Intersection(camAABB, nl->bbox_ini, realbb);
				if (realbb.GetArea() <= 0.0f)
				{
					nl->m_nLightMeshIdx = -1;
					break;
				}

				_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
				vul.pos = Vec3(realbb.vMin.x, realbb.vMin.y, 0.0f);
				vur.pos = Vec3(realbb.vMax.x, realbb.vMin.y, 0.0f);
				vdl.pos = Vec3(realbb.vMin.x, realbb.vMax.y, 0.0f);
				vdr.pos = Vec3(realbb.vMax.x, realbb.vMax.y, 0.0f);
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

	CDoubleLinkedPool<CDoofer>::CLinkedPoolNode *node = m_poolDoofers.pListUsed.m_pNext;
	while (node != &m_poolDoofers.pListUsed)
	{
		//salvez locatia urmatoare ca s apot avansa pe ea
		CDoubleLinkedPool<CDoofer>::CLinkedPoolNode *nextnode = node->m_pNext;
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

				float fOrigAlpha = DW_GETFALPHA(prop->sprLight.color);
				vul.color = vur.color = vdl.color = vdr.color = DW_COLORALPHA(prop->sprLight.color, fOrigAlpha * alpha);
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


	///--- water ---
	/*
	m_bufferedPainter.BeginMesh(m_waterMeshIdx);
	//salvez date textura apa	
	float waterTexScale = 2.0f;
	float waterTexSize = m_texManager.arrTextures[m_waterTexIdx]->info.Width;

	for (int kk = 0; kk < m_visibleList.logic_colShapesSpecial.Count(); kk++)
	{
		if (m_visibleList.logic_colShapesSpecial.m_pData[kk]->type == K_LVL_COLL_TYPE_WATER)
		{
			CCollisionShape * col = m_visibleList.logic_colShapesSpecial.m_pData[kk];
			CAABB wbb; //water bbox
			if (AABB::Intersection(col->bbox, camAABB, wbb))
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
	*/

	//4. poligoane fow
	m_bufferedPainter.BeginMesh(m_fogofwarMeshIdx);

	for (int kk = 0; kk < m_visibleList.logic_colShapesSpecial.Count(); kk++)
	{
		if (m_visibleList.logic_colShapesSpecial.m_pData[kk]->type == K_LVL_COLL_TYPE_FOG_OF_WAR)
		{
			CCollisionShape * col = m_visibleList.logic_colShapesSpecial.m_pData[kk];
			CAABB wbb; //bbox
			if (AABB::Intersection(col->bbox, camAABB, wbb))
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
					if ((pPlayerActor[kk] != NULL) && (!pPlayerActor[kk]->bHidden) && (pPlayerActor[kk]->bbox.Intersects(active->bbox)))
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
					if ((m_arrActors[kk]->bHidden) || (m_arrActors[kk]->fLife <= 0.0f) || (m_arrActors[kk]->actTemplate.actorClass == K_LVL_ACT_CLASS_PLAYER) ||
						((m_arrActors[kk]->actTemplate.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET) != 0))
							continue;
					if (m_arrActors[kk]->bbox.Intersects(active->bbox))
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
			/*
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
			*/
		}
		break;
		case K_AI_STATE_FN_ANG_SIN_TIME:
		{
			/*
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
			*/
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
			active->color = DW_COLORALPHA(active->color_ini, falpha);
		}
		break;
		case K_AI_STATE_FN_GET_TARGET_POS:
		{
			/*
			if (active->pTarget != NULL)
			{
				Vec2 targetDelta = active->pTarget->pos.xy - active->pTarget->pos_ini.xy;
				//mut obiectul cu delta totala a targetului
				active->SetPos(active->pos_ini + targetDelta);
			}
			*/
		}
		break;
		case K_AI_STATE_FN_GET_TARGET_ANG:
		{
			/*
			if (active->pTarget != NULL)
			{
				Vec2 targetVec = active->pos_ini.xy - active->pTarget->pos_ini.xy;
				Mat matrot;
				MuMatRotZ(&matrot, active->pTarget->fAngle - active->pTarget->fAngle_ini);
				MUVec2TransformCoord(&targetVec, &targetVec, &matrot);
				//mut obiectul cu delta totala a targetului
				active->SetPos(active->pTarget->pos_ini.xy + targetVec);
				active->SetAngle(active->fAngle_ini + (active->pTarget->fAngle - active->pTarget->fAngle_ini));
			}
			*/
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
			active->SetPos(Vec3(newpos.x, newpos.y, active->pos.xyz.z));
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
					((pPlayerActor[kk]->actTemplate.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET) != 0) )
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
				if ((m_arrActors[kk]->bHidden) || (m_arrActors[kk]->fLife <= 0.0f) || (m_arrActors[kk]->actTemplate.actorClass == K_LVL_ACT_CLASS_PLAYER) ||
					(m_arrActors[kk]->fLife <= 0.0f) ||
					((m_arrActors[kk]->actTemplate.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET) != 0))
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
					bool bKillPlayer = (bool)colshape->varAIparams.GetVariantByName(L"b_killPlayer")->m_asBool;
					bool bKillOthers = (bool)colshape->varAIparams.GetVariantByName(L"b_killOthers")->m_asBool;
					bool bSplat = (bool)colshape->varAIparams.GetVariantByName(L"b_splat")->m_asBool;

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
							if ((m_arrActors[kk]->bHidden) || (m_arrActors[kk]->fLife <= 0.0f) || (m_arrActors[kk]->actTemplate.actorClass == K_LVL_ACT_CLASS_PLAYER) ||
								(m_arrActors[kk]->fLife <= 0.0f) ||
								((m_arrActors[kk]->actTemplate.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET) != 0))
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

						winact->sprite.frameIdx++;
						//reset object script and interact
						winact->arrActions.Clear();

						//generate particles
						CVariantComplex* cvar = colshape->varAIparams.GetVariantByName(L"fForceDirX");
						float dirx = SIGN(cvar->m_asFloat);
						for (int ll = 0; ll < 20; ll++)
						{
							Vec2 ppos = AABB::GetRandomPointInBox(colshape->bbox);
							g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLASS_SHARDS, false, randint(5), &ppos, &g_vecGravityOld, &Vec2(dirx * (60.0f + randfloat(60.0f)), -40.0f + randfloatsgn(50.0f)), 0.3f + randfloat(0.2f), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
						}
						//sound
						//SND_PLAY_POSITIONAL_RAND2(SNDIDX_WINDOWBREAK1, SNDIDX_WINDOWBREAK2, colshape->bbox.vCenter);
					}
					//destroy collision box
					colshape->Kill();
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
						Vec2 ppos = AABB::GetRandomPointInBox(colshape->bbox);
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
				float falpha = UTPerlin::PerlinNoise1D(light->fTimelineAI * timeMul, 2.0f, 3.0f, 0.8f, 0.25f, 2);
				if (falpha > fThreshold)
					falpha = 1.0f;
				else
					falpha = falpha / fThreshold;
				//falpha = (falpha < fThreshold) ? 0.0f : 1.0f;
				light->color = DW_COLORALPHA(light->color_ini, falpha);
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
	light->SetPos(light->pos.xyz);
}

void CLevel::UpdateAI_prop(CProp* prop, float dTime)
{
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
		UINT32 aframeFlag = prop->sprite.Update(dTime);
		//cand ajunge la capatul animatiei scoate flagul de animated
		if (prop->sprite.animStatus == ANIM_STATUS_FRAMELOCK)
			prop->bAnimated = false;
		//la obiectele animate luam bbox-ul la fiecare frame
		if ((prop->sprite.animStatus == ANIM_STATUS_PLAYING_FRAME_ADVANCED) || (prop->sprite.animStatus == ANIM_STATUS_FRAMELOCK))
		{
			RECTXYWH frrect = m_sprProps.GetAFrameBBox(prop->sprite.animIdx, prop->sprite.frameIdx);
			prop->bbox_ini.Set(frrect);
			//nu pastreaza acelasi bbox la flip deci flipam bboxul
			/*
			if (prop->flipX)
			{
				prop->bbox_ini.Flip(true, false);
			}
			*/
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
				//m_interfaceIGM.SetBombTimer(prop->AItimer1);

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
					//m_interfaceIGM.SetBombTimer(0.0f);
					//add some explosions so everybody will die
					AddDoofer_Explo(hash_EXPLO_LARGE_XL, prop->pos.xy, prop->UID, K_LVL_ACT_CLASS_EXPLOSION);
					AddDoofer_Explo(hash_EXPLO_LARGE_XL, prop->pos.xy + Vec2(32.0f, 0.0f), prop->UID, K_LVL_ACT_CLASS_EXPLOSION);
					AddDoofer_Explo(hash_EXPLO_LARGE_XL, prop->pos.xy - Vec2(32.0f, 0.0f), prop->UID, K_LVL_ACT_CLASS_EXPLOSION);

					g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_ROUND_XL, true, 0, &Vec2(prop->pos.xy.x, prop->pos.xy.y - 15.0f), NULL, NULL, 1.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);

					prop->sprite.SetAnim("BOMB_EXPLODED");

					SetLevelState(K_LVL_STATE_MISSION_FAILED, STR_BOMB_EXPLODED);
				}
			}
			break;
			case K_AI_STATE_ACTIVE_AMMO_BOX:
			{
				int nAmmoLeft = prop->varAIparams.GetVariantByName(L"n_ammoLeft")->m_asINT32;
				prop->sprite.frameIdx = nAmmoLeft;

				//fade out
				if (nAmmoLeft <= 0)
				{
					prop->AItimer1 -= dTime;
					if (prop->AItimer1 <= 0.0f)
					{
						prop->Kill();
					}
					//color
					float fAlpha = LIMIT(prop->AItimer1, 0.0f, 1.0f);
					prop->color = DW_COLORALPHA(prop->color_ini, fAlpha);
				}
			}
			break;
			case K_AI_STATE_ACTIVE_HEALTH_BOX:
			{
				int nHealthLeft = prop->varAIparams.GetVariantByName(L"n_healthLeft")->m_asINT32;
				prop->sprite.frameIdx = nHealthLeft;

				//fade out
				if (nHealthLeft <= 0)
				{
					prop->AItimer1 -= dTime;
					if (prop->AItimer1 <= 0.0f)
					{
						prop->Kill();
					}
					//color
					float fAlpha = LIMIT(prop->AItimer1, 0.0f, 1.0f);
					prop->color = DW_COLORALPHA(prop->color_ini, fAlpha);
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
				prop->sprite.frameIdx = prop->fid_ini.frameIdx;
				if (prop->AItimer1 > 0.0f)
				{
					prop->AItimer1 -= dTime;
					
					bool bDontChangeFrames = (bool)(prop->varAIparams.GetVariantByName(L"b_DontChangeFrames")->m_asBool);
					if (!bDontChangeFrames)
					{
						prop->sprite.frameIdx++;
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
						SND_PLAY_POSITIONAL(sndidx, prop->pos.xy);
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
						SND_PLAY_POSITIONAL(sndidx, prop->pos.xy);
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
				/*
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
				*/
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
					if (pPlayerActor[kk]->bbox.Intersects(prop->bbox))
					{
						prop->Touch(pPlayerActor[kk]->GetUID(), dTime);
						//save checkpoint
						vLastSpawnPoint = prop->pos.xy;
						break;
					}
				}
			}
			break;
			default:
			{
				if (!UpdateAI_base(prop, dTime, prop->fTimelineAI))
				{
					ErrorBox(K_ERR_WARNING, L"CLevel::UpdateAI_prop - AIstate not handled: %d", prop->AIstate);
				}
			}
			break;
		}
	}

	//final updates
	prop->sprite.pos = prop->pos.xy_proj;
	prop->sprite.color = prop->color;
}


void CLevel::Actor_SetAIState(CActor * actor, CAIState* pNewState)
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

bool CLevel::Actor_SetAIState(CActor * actor, WCHAR * strStateName)
{
	CAIState* newstate = actor->actTemplate.AItemplate->GetAIStateByName(strStateName);
	if (newstate == null)
	{
		LOG(L"CLevel::SetActorAIState - state not found! %s\n", strStateName);
		return false;
	}
	//everything ok, set state
	Actor_SetAIState(actor, newstate);
	return true;
}

void CLevel::SetActorWeaponPerks(CActor * pActor, CWeapon * pWeapon)
{
	_ASSERT((pWeapon != null) && (pActor != null));

	//reset actor template to initial one
	pActor->actTemplate = pActor->actTemplate_ini;

	if (!pWeapon->WeaponTemplate.shTemplateOverwrite.IsEmpty())
	{
		CActorTemplate* updateTemplate = Actor_GetTemplate(pWeapon->WeaponTemplate.shTemplateOverwrite.textHash);
		if (updateTemplate == null)
		{
			ErrorBox(K_ERR_WARNING, L"SetActorCurrentWeapon failed! Template %s not found for weapon %s!", pWeapon->WeaponTemplate.shTemplateOverwrite.text, pWeapon->WeaponTemplate.name.text);
		}
		//set animations from new template
		pActor->actTemplate.AddGenericDataFromTemplate(updateTemplate);
		pActor->actTemplate.OverwriteAnimsFromTemplate(updateTemplate);
	}
	//set the heart and gun vectors again
	//LoadActorBBoxAndPoints(pActor, K_LVL_ACT_ANIM_REF_POSE, 0);

	///--- PERKS ---
	//apply perks that change current weapon
	if ((pActor->actTemplate.actorClass == K_LVL_ACT_CLASS_PLAYER) && (pActor->nPlayerOrdinal >= 0))
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
	actor->fFOVPercent = 1.0f;

	switch (pNewBehavior->nType)
	{
		case AI_BEHAVIOR_EMPTY:
		{
			//state doesn't need update
			ret_bFinished = true;
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

			CAIState* newstate = actor->actTemplate.AItemplate->GetAIStateByName(vc->m_strArg);
			if (newstate == null)
			{
				LOG(L"AI_BEHAVIOR_SET_STATE - state not found! %s\n", vc->m_strArg.text);
				break;
			}
			//everything ok, set state
			Actor_SetAIState(actor, newstate);
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
			actor->SetPos(actor->pClosestTouchable->pTarget->pos.xyz);
			//set open frame (if necessary) on target
			IActiveInterface* active = actor->pClosestTouchable->pTarget;
			if ((active->AIstate == K_AI_STATE_ACTIVE_DOORFACE_AUTOCLOSE) || (active->AIstate == K_AI_STATE_ACTIVE_TEAM_TELEPORTER_2FRAMES))
			{
				bool bDontChangeFrames = (bool)(active->varAIparams.GetVariantByName(L"b_DontChangeFrames")->m_asBool);
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
					actor->actTemplate.eCaps |= CActorTemplate::K_ACT_CAPS_NOT_A_TARGET;
				else
					actor->actTemplate.eCaps &= ~CActorTemplate::K_ACT_CAPS_NOT_A_TARGET;
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
		}
		break;
		case AI_BEHAVIOR_FLEE:
		{
		}
		break;
		case AI_BEHAVIOR_BLIND_RUN:
		{
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
			/*
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
			*/
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
//					PlayActorSoundVerse(actor, eVerse);
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
				GenerateEffect(cvc->m_strArg, actor->GetPosHeart(), fSize);
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
			/*
			CActor* plAct = GetClosestPlayer(actor);
			if (plAct != null)
			{
				actor->m_AIcommands.nLookDirX = SIGN(plAct->GetPosHeart().x - actor->GetPosHeart().x);
			}

			actor->fFOVPercent = 1.0f;  //disable FOV check while attacking
			actor->AIfvar1 = 0.0f;		//teleporter timer
			actor->AItimer1 = 0.0f;		//forget about target timer (usually 10 sec)
			*/
		}
		break;

		case AI_BEHAVIOR_SURPRISED:
		{
			/*
			//se intoarce catre event
			if (actor->m_AIsensorInfo.m_AIcurrentEvent.nType > K_LVL_AI_EVENT_IDLE_TICK)
			{
				actor->m_AIcommands.nLookDirX = SIGN(actor->m_AIsensorInfo.m_AIcurrentEvent.pos.x - actor->pos.x);
			}
			//set wait timer
			actor->AItimer1 = pNewBehavior->m_vcolParams.GetVariantByName(L"fWaitTimer")->asFloat();
			actor->AIsubState = 0;

			actor->fFOVPercent = 1.0f; //disable FOV check while attacking
			*/
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
			actor->bCrouched = false;
			actor->fStunTimer = 0.0f;

			actor->m_AIcommands.nIconType = K_LVL_ACT_ICON_REMOVE_ICON;
			actor->m_AIcommands.fIconDuration = -1.0f;

			actor->fLife = 0.0f; //kill it
			//trateaza death commands din script
			EActorDeathCommand dcmd = K_LVL_ACT_DEATHCMD_NONE;
			CVariantComplex* cvc = pNewBehavior->m_vcolParams.GetVariantByName(L"sDeathCommand");
			if (cvc->m_type == CVariantComplex::K_ARGTYPE_STRING)
			{
				int ndcmd = GetListIndexByName(cvc->m_strArg.text, EActorDeathCommandNames, K_LVL_ACT_DEATHCMD_CNT);
				//daca avem comanda de death o trimitem mai departe
				if(ndcmd >= 0)
					actor->varAIparams.SetNamedVarINT32(L"nDeathCommand", ndcmd);
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

			actor->bCrouched = false;
			actor->fStunTimer = 0.0f;
			//death timer for players or splat timer for others
			actor->AItimer1 = 0.0f;
			if (actor->actTemplate.actorClass != K_LVL_ACT_CLASS_PLAYER) 
			{
				CVariantComplex* cvt = pNewBehavior->m_vcolParams.GetVariantByName(L"fSplatTimer");
				if (cvt->m_type == CVariantComplex::K_ARGTYPE_FLOAT)
					actor->AItimer1 = cvt->asFloat();
			}

			//remove icons
			actor->m_AIcommands.nIconType = K_LVL_ACT_ICON_REMOVE_ICON;
			actor->m_AIcommands.fIconDuration = -1.0f;
			actor->m_AIcommands.ResetMoveCommands();
			//reset color
			actor->m_AIcommands.nColor = actor->color_ini;
			//stop weapons
			Weapon_StopReloading(actor->pWeaponMain);
			Weapon_Jam(actor->pWeaponMain);
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

			if (actor->actTemplate.actorClass == K_LVL_ACT_CLASS_PLAYER)
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
				//m_interfaceIGM.SetStrategicSelection(actor->nPlayerOrdinal, -1);
				//remove icon
				actor->SetIcon(K_LVL_ACT_ICON_NONE);
				//dam remove la particles de pe interfata cand moare un player
				g_particlesMgr.RemoveAllFromLayer(K_PART_LAYER_INTERFACE_LIGHT);
			}
			else if (actor->actTemplate.actorClass >= K_LVL_ACT_CLASS_HUMAN)
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
			{
				//PlayActorSoundVerse(actor, K_LVL_ACT_VERSE_DIE);
			}

			//hostages specials
			if (actor->actTemplate.actorClass == K_LVL_ACT_CLASS_HOSTAGE)
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
					//GiveStrategicPoints(actor->actTemplate.fStrategicPoints, &Vec2(actor->bbox.vCenter.x, actor->bbox.vMin.y));
					//make sure we release it on the next frame
					actor->Kill();
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
				actor->Kill();
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
	if ((eFilterClass > K_LVL_ACT_CLASS_ANY) && (act->actTemplate.actorClass != eFilterClass))
		return;
	if ((eExcludedClass > K_LVL_ACT_CLASS_ANY) && (act->actTemplate.actorClass == eExcludedClass))
		return;

	if ((eType == CDamageOverTime::K_LVL_DoT_INTIMIDATED) && (act->fLife <= 0.0f))
		return;

	//#HARDCODE: DoT_TARGETED only works on enemies
	if ((eType == CDamageOverTime::K_LVL_DoT_TARGETED) && (act->actTemplate.actorClass < K_LVL_ACT_CLASS_HUMAN))
		return;

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	//LOG(L"- SetDoT %s for %.4f", act->templateActor.shName.text, eType);
#endif

	if (act->cDamageOverTime.Set(eType, fDuration, fDamagePerSec, eExcludedClass, eFilterClass, dwOwnerUID))
	{
		//pointer to player owner or null if not a player
		CActor* pPlayerOwner = GetPlayerByUID(dwOwnerUID);
		//special statistics
		if ((eType == CDamageOverTime::K_LVL_DoT_FIRE) && (act->actTemplate.actorClass >= K_LVL_ACT_CLASS_HUMAN))
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
	UINT32 aframeFlag = 0; //flagul aframe-ului resetat
	UINT32 aframeFlag_feet = 0; //flagul aframe-ului resetat

	///--- ACTOR CAPS ---
	/*

	if (actor->actTemplate.eCaps & CActorTemplate::K_ACT_CAPS_CAN_INTERACT)
	{
		// LOOK ONLY IN ACTOR AREA
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
			if (AABB::Intersection(actor->bbox, activ->bbox_exported, retAABB))
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
	*/

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

	if (actor->fLife <= 0.0f)
	{
		//ca sa intre doar o singura data:
		if (actor->m_AIsensorInfo.m_AIcurrentEvent.nType != K_LVL_AI_EVENT_DEAD)
		{
			actor->m_AIsensorInfo.b_IsDead = true;
			actor->m_AIsensorInfo.m_AIcurrentEvent.Set(K_LVL_AI_EVENT_DEAD, actor->GetUID(), actor->actTemplate.actorClass, actor->GetPosHeart(), -1.0f, 1.0f, actor->GetUID());
			//save in memory
			actor->m_AIsensorInfo.m_AIlastEvent = actor->m_AIsensorInfo.m_AIcurrentEvent;
			//reset targeted actor
			actor->m_AIsensorInfo.pTargetedActor = null;
			///THINK: force state decision
			CAIState* newState = actor->actTemplate.AItemplate->GetHighestPriorityState(K_LVL_AI_EVENT_DEAD, &m_rand);
			Actor_SetAIState(actor, newState);
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
		//did he get hit? reset time since hit 
		//if (actor->nTookDamageFrames > 0)
			//actor->m_AIsensorInfo.fTimeSinceHit = 0.0f;

		///LOW FREQUENCY SENSORS
		actor->AItimerDecision -= dTime;
		if ((actor->AItimerDecision <= 0.0f) && (!bIgnoreAIEvents) && (actor->m_AIsensorInfo.m_bEnabled))
		{
			//save previous event in memory only if not IDLE_TICK
			if(actor->m_AIsensorInfo.m_AIcurrentEvent.nType > K_LVL_AI_EVENT_IDLE_TICK)
				actor->m_AIsensorInfo.m_AIlastEvent = actor->m_AIsensorInfo.m_AIcurrentEvent;

			//check for targets or other AI events
			CActor* targetActor = nullptr;// GetClosestTarget(actor, actor->actTemplate.foeClassFilter1, actor->actTemplate.foeClassFilter2);
			if (targetActor != nullptr)
			{
				float enemyDst = MUVec2Len(&(targetActor->GetPosHeart() - actor->GetPosHeart()));
				AddAIEvent(K_LVL_AI_EVENT_SEE_ENEMY, targetActor->GetUID(), targetActor->actTemplate.actorClass, targetActor->GetPosHeart(), enemyDst, 1.0f, actor->GetUID());
				//#HACK: alerts the other enemies only if enemy class
				if(actor->actTemplate.actorClass >= K_LVL_ACT_CLASS_HUMAN)
					AddAIEvent(K_LVL_AI_EVENT_SOUND_THREAT, targetActor->GetUID(), targetActor->actTemplate.actorClass, targetActor->GetPosHeart(), 200.0f, 0.6f);
				//set target pointer
				actor->m_AIsensorInfo.pTargetedActor = targetActor;
				//vede daca face overlap
				/*
				if (actor->bbox.Intersects(targetActor->bbox))
				{
					actor->m_AIsensorInfo.fTargetOverlapX = SIGN(actor->pos.xy.x - targetActor->pos.xy.x) * ((actor->bbox.vHalfSize.x + targetActor->bbox.vHalfSize.x) - fabs(actor->pos.xy.x - targetActor->pos.x));
				}
				*/
				//daca se ating trimit si event de touch enemy, doar daca vede inamicul
				if (actor->bbox.Intersects(targetActor->bbox))
				{
					AddAIEvent(K_LVL_AI_EVENT_TOUCH_ENEMY, targetActor->GetUID(), targetActor->actTemplate.actorClass, targetActor->GetPosHeart(), enemyDst, 1.0f, actor->GetUID());
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
					AddAIEvent(K_LVL_AI_EVENT_LOST_ENEMY, 0, K_LVL_ACT_CLASS_ANY, actor->GetPosHeart() + Vec2(16.0f, 0.0f), 16.0f, 1.0f, actor->GetUID());
					//reset targeting actor
					actor->m_AIsensorInfo.pTargetedActor = null;
				}

				//#HACK: uneori e lovit dar nu apuca sa vada inamicul si ramane blocat ca nu primeste LOST_ENEMY asa ca il trimitem acum
				if ((actor->m_AIsensorInfo.pTargetedActor == null) && (actor->m_AIsensorInfo.m_AIlastEvent.nType == K_LVL_AI_EVENT_GOT_HIT))
				{
					//put event behind him
					AddAIEvent(K_LVL_AI_EVENT_LOST_ENEMY, 0, K_LVL_ACT_CLASS_ANY, actor->GetPosHeart() - Vec2(16.0f, 0.0f), 16.0f, 0.5f, actor->GetUID());
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
				CAIState* newState = actor->actTemplate.AItemplate->GetHighestPriorityState(actor->m_AIsensorInfo.m_AIcurrentEvent.nType, &m_rand);
				
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
					Actor_SetAIState(actor, newState);
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
					//m_interfaceIGM.SetStrategicSelection(actor->nPlayerOrdinal, nSel);
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

				//interact
				if (pController->sCommands.keyState[K_CM_COMMAND_JUMP] == K_CM_BUTSTATE_JUSTPRESSED)
				{
					actor->m_AIcommands.bInteract = true;
				}
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
				//RELOAD ON SHOOT - overwrites previous commands
				if ((pController->sCommands.keyState[K_CM_COMMAND_FIRE1] == K_CM_BUTSTATE_JUSTPRESSED) && (actor->pWeaponMain->ammoLeft == 0))
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
					AddAIEvent(K_LVL_AI_EVENT_SOUND_THREAT, actor->GetUID(), actor->actTemplate.actorClass, actor->GetPosHeart(), 32.0f, 0.5f);
				}

				if (actor->AIvarBool1) //can it burn?
				{
					//when it catches fire it burns 'till the end
					if (actor->cDamageOverTime.eType == CDamageOverTime::K_LVL_DoT_FIRE)
					{
						actor->cDamageOverTime.fDuration = actor->cDamageOverTime.fDuration_ini;
					}
					//daca are viata mai mica de 90% din viata initiala in mai putin de o secunda explodeaza
					if ((actor->fLife <= actor->actTemplate.fLife * 0.9f) && (actor->AIsubState == 0))
					{
						//starts to shake
						actor->AIsubState = 1; 
						actor->AItimer1 = 2.0f;
					}

					//explodes after a while
					if (actor->AIsubState == 1)
					{
						//when flaming it takes 10% of life
						actor->fLife -= dTime * actor->actTemplate.fLife * 0.5f;
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

							//actor->m_AIcommands.bThrustX = true;
							//actor->m_AIcommands.nMoveDirX = m_rand.RandSign();
						}
					}
				}
			}
			break;
			case AI_BEHAVIOR_HOSTAGE:
			{
				//always set crouched command if actor can crouch
				if (actor->actTemplate.eCaps & CActorTemplate::K_ACT_CAPS_CAN_CROUCH)
					actor->m_AIcommands.bCrouched = true;
				//can he follow targets? does it only once
				if (actor->AIvarBool1)
				{
					if (actor->m_AIsensorInfo.pTargetedActor != null)
					{
						//play the verse only once
						if (actor->AIsubState == 0)
						{
//							PlayActorSoundVerse(actor, K_LVL_ACT_VERSE_TAUNT);
						}

						Vec2 vDelta = actor->m_AIsensorInfo.pTargetedActor->GetPosHeart() - actor->GetPosHeart();
						float fDist = fabs(vDelta.x);
						float fDistMin = 32.0f;// max(K_TILE_SIZE, actor->actTemplate.distAttackMin);
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
						actor->m_AIcommands.bThrust = true;
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
						actor->m_AIcommands.nColor = DW_COLORALPHA(actor->color_ini, fLeftTime / actor->AIfvar1);
					}
				}
			}
			break;
			case AI_BEHAVIOR_PLAY_ANIM:
			{
				//playerii pot schimba directia si pe play anim
				if (actor->actTemplate.actorClass == K_LVL_ACT_CLASS_PLAYER)
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
							actor->m_AIcommands.bThrust = false;
							//actor->m_AIcommands.nMoveDirX = 1;
						}
						if (bPressedLeft && !bPressedRight)
						{
							actor->m_AIcommands.bThrust = false;
							//actor->m_AIcommands.nMoveDirX = -1;
						}
					}
				}

				//seteaza comanda de override anim cu valoarea salvata in SetActorAIBehavior din params behavior
				//actor->m_AIcommands.eOverrideAnim = (EActorAnims)actor->AIvar1;

				//conditii final (sfarsit animatie)
				//if (actor->sprite.animStatus == ANIM_STATUS_FRAMELOCK)
					//bBehaviorFinished = true;
				//modificare alpha daca e setat duration
				if (actor->m_pAIcurrentState->m_arrBehaviors[actor->m_nAIcurrentBehaviorIdx].fBehaviorDuration > 0.0f)
				{
					float fPerc = actor->m_fAIbehaviorTimer / actor->m_pAIcurrentState->m_arrBehaviors[actor->m_nAIcurrentBehaviorIdx].fBehaviorDuration;
					//setam comanda de culoare
					actor->m_AIcommands.nColor = DW_COLORALPHA(actor->color_ini, (1.0f - fPerc) * actor->AIfvar2 + fPerc * actor->AIfvar1);
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
					/*if ((actor->actTemplate.animIDs[K_LVL_ACT_ANIM_DIE][0] == -1) || (actor->varAIparams.GetVariantByName(L"bSplat")->m_asBool))
					{
						actor->m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_SPLAT;
					} */
				}
				//cauta params particulari de AI setati din Hit Actor
				CVariantComplex* cvc = actor->varAIparams.GetVariantByName(L"nExplode");
				if (cvc->m_type != CVariantComplex::K_ARGTYPE_NONE)
				{
					//comanda splat on explode daca e clasa care trebuie
					if ((actor->actTemplate.actorClass == K_LVL_ACT_CLASS_HUMAN) || (actor->actTemplate.actorClass == K_LVL_ACT_CLASS_HOSTAGE))
						actor->m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_SPLAT;
					//get explo class
					UINT32 unExploUID = actor->GetUID();
					if (actor->varAIparams.GetVariantByName(L"bUseDamagerUID")->m_asBool)
						unExploUID = actor->nLastDamageTakenFromUID;
					//generate explo
					AddDoofer_Explo(cvc->m_asUINT32, actor->GetPosHeart(), unExploUID, K_LVL_ACT_CLASS_EXPLOSION, Vec2(0.0f, 0.0f), &actor->bbox);

					//decal explo mark
					//AddDecal(K_LVL_DECAL_LAYER_BACKWALLS, actor->GetPosHeart(), ANM_ACTIVES_SPR_DECAL_EXPLOMARKS, randint(3), 0xffffffff);
				}

				///- when the player dies -
				if (actor->actTemplate.actorClass == K_LVL_ACT_CLASS_PLAYER)
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
					actor->actTemplate.actorClass = K_LVL_ACT_CLASS_HUMAN;

					//daca avem breaching charges aruncate in nivel le dezalocam
					ReleaseBulletType(K_LVL_BULLET_BREACHING_CHARGE, actor->GetUID());

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

					//m_interfaceIGM.Init(&m_sprInterface, pPlayerActor[0], pPlayerActor[1]);
					//m_interfaceIGM.SetHotJoinSelection(actor->nPlayerOrdinal, -1);

					//on local play check if other player is suspended and move camera on him
					if ((!UTApp().IsGameNetworked()) && (!bContinue))
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
					if ((actor->actTemplate.eMaterial == K_LVL_MATERIAL_FLESH) && (actor->AItimer1 > 0.0f))
					{
						actor->AItimer1 -= dTime;
						if (actor->AItimer1 <= 0.0f)
						{
							actor->AItimer1 = 0.0f;
							actor->m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_SPLAT;
						}
					}
				}
				//only flesh can splat
				if ((actor->m_AIcommands.nDeathCommand == K_LVL_ACT_DEATHCMD_SPLAT) && (actor->actTemplate.eMaterial != K_LVL_MATERIAL_FLESH))
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

	//save old crouch state
	bool bCrouchedOldState = actor->bCrouched;

	//set crouch (not on ladder)
	actor->bCrouched = actor->m_AIcommands.bCrouched;

	//check crouch conditions when falling or jumping
	/*
	if (fabs(actor->speed.y) > 1.0f)
		actor->bCrouched = false;

	if (actor->bCrouched)
	{
		//can he roll?
		if ((actor->actTemplate.eCaps & CActorTemplate::K_ACT_CAPS_CAN_ROLL) && 
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
		if ((bCanceledByKeys) ||
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
	*/

	//look for cover when entering crouched state
	/*
	if (actor->actTemplate.eCaps & CActorTemplate::K_ACT_CAPS_CAN_COVER)
	{
		//entering crouch state
		if ((actor->bCrouched) && (!bCrouchedOldState) && (actor->pCover == null))
		{
			int defaultCheckAttackStatus = K_LVL_ACT_ATTACK_SHOOTING;
			//can crouch?
			if ((actor->nAttackStatus <= defaultCheckAttackStatus) && (actor->collisionFlags & K_DIRFLAG_DOWN))
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
						}
						else
						{
							actor->pos.x += coverbox->vMax.x - actor->bbox.vCenter.x + 2.0f;
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
		if ((actor->pCover != null) && (!actor->bbox.Intersects(actor->pCover->bbox)))
		{
			actor->pCover = null;
		}
	}
	*/
	
	///--- Look direction ---
	//trebuie sa avem pointerul mereu setat
	_ASSERT(actor->pWeaponMain != null);
	///--- Shooting and reloading ---
	if (actor->pWeaponMain->status == K_LVL_WPN_STATUS_RELOADING)
	{
		//can't shoot until you reload on weapons with bullets clip
		if (actor->pWeaponMain->WeaponTemplate.nReloadUnitSize >= actor->pWeaponMain->WeaponTemplate.nClipSize)
		{
			//poti intrerupe reload cu urmatoarele comenzi:
			if (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_MELEE)
			{
				Weapon_StopReloading(actor->pWeaponMain);
				actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
			}
			else
			{
				actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_RELOADING;
			}
		}
	}

	///--- keep players together, limits movement on couch multiplayer but not on net multiplayer
	if ((actor->GetCurrentBehavior() == AI_BEHAVIOR_PLAYER_CONTROL) && (!UTApp().IsGameNetworked()))
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
					if (GetBestSpawningPos(&pOther->pos.xy, pOther->bbox, &pOther->bbox))
					{
						actor->SetPos(pOther->pos.xyz);
						//animate player on spawn (only if told otherwise by nAnimset=-1)
						actor->SetAnimSet(0);
						Actor_SetAIState(actor, L"JOIN_GAME");
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
			RECTXYWH_F camrect = m_camLevelToRT.GetCamWorldAABB();
			CAABB camAABB(Vec2(camrect.x, camrect.y), Vec2(camrect.Right(), camrect.Bottom()));

			camrect.Inflate(-16.0f);
			//players midpoint
			Vec2 avg(0.0f, 0.0f);
			int plcnt = 0;
			for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
			{
				if ((pPlayerActor[kk] != NULL) && (pPlayerActor[kk]->GetCurrentBehavior() != AI_BEHAVIOR_IN_LIMBO) && (pPlayerActor[kk]->nSuspendedFlags == K_LVL_SUSPENDFLAG_NONE))
				{
					avg += pPlayerActor[kk]->GetPosHeart();
					plcnt++;
				}
			}
			if (plcnt > 0)
			{
				avg /= plcnt;
			}
			//--- gaseste zona in care ai voie sa te misti, limitare miscare multiplayer ---
			/*
			RECTXYWH_F lockrect(avg.x - camrect.w / 2.0f, avg.y - camrect.h / 2.0f, camrect.w, camrect.h);

			if (((actor->bbox.vMin.x < lockrect.x) && (actor->m_AIcommands.nMoveDirX < 0)) ||
				((actor->bbox.vMax.x > lockrect.Right()) && (actor->m_AIcommands.nMoveDirX > 0)))
			{
				actor->m_AIcommands.bThrustX = false;
			}
			if (((actor->bbox.vMin.y < lockrect.y) && (actor->m_AIcommands.nMoveDirY < 0)) ||
				((actor->bbox.vMax.y > lockrect.Bottom()) && (actor->m_AIcommands.nMoveDirY > 0)))
			{
				actor->m_AIcommands.bThrustY = false;
			}
			*/
			//daca actorul a iesit din ecran ii da suspend
			if (camAABB.Intersects(actor->bbox))
			{
				actor->nSuspendedFlags &= ~K_LVL_SUSPENDFLAG_OUTSIDE_SCREEN;
			}
			else
			{
				actor->nSuspendedFlags |= K_LVL_SUSPENDFLAG_OUTSIDE_SCREEN;
			}
		}
	}

	CWeapon* pNewWeapon = actor->pWeaponMain; //defaults on primary default weapon
	if ((actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_SHOOTING) || (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_RELOADING))
		pNewWeapon = actor->pWeaponMain;
	/*
	else if (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_SHOOTING_ALT)
		pNewWeapon = actor->pSelectedWeapon[K_LVL_ACT_WEAPON_SECONDARY];
	else if (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_USING_GEAR)
		pNewWeapon = actor->pSelectedWeapon[K_LVL_ACT_WEAPON_GEAR];
	else if (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_MELEE) 
		pNewWeapon = actor->pSelectedWeapon[K_LVL_ACT_WEAPON_MELEE];
	else if (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_BREACH)
		pNewWeapon = actor->pSelectedWeapon[K_LVL_ACT_WEAPON_BREACH];
	  */
	//daca sunt cu arma care incarca glont cu glont pot schimba si in timp ce incarca
	if ((pNewWeapon != null) && (pNewWeapon != actor->pWeaponMain) &&
		(actor->pWeaponMain->WeaponTemplate.nReloadUnitSize < actor->pWeaponMain->WeaponTemplate.nClipSize) && 
		(actor->nAttackStatus == K_LVL_ACT_ATTACK_RELOADING))
	{
		Weapon_StopReloading(actor->pWeaponMain);
		actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
	}

	//change weapon
	if ((pNewWeapon != null) && (actor->nAttackStatus == K_LVL_ACT_ATTACK_IDLE) && (pNewWeapon != actor->pWeaponMain) &&
		((actor->pWeaponMain->status <= K_LVL_WPN_STATUS_COOLING) || (actor->pWeaponMain->status == K_LVL_WPN_STATUS_NO_AMMO)) )
	{
		//raise triggers
		actor->pWeaponMain->SetTriggerStates(false, false);
		//stop reloading if was reloading
		Weapon_StopReloading(actor->pWeaponMain);
		//switch to new weapon
		actor->pWeaponMain = pNewWeapon;
		SetActorWeaponPerks(actor, pNewWeapon);
	}
	else
	{
		// non valid weapon change
		if (pNewWeapon != actor->pWeaponMain)
			actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_IDLE;
	}

	//verificari diverse ex. daca esti in aer si tragi cu o arma ce nu poate fi trasa din aer se intrerupe
	if ((actor->m_AIcommands.eAttackCommand >= K_LVL_ACT_ATTACK_SHOOTING) || (actor->nAttackStatus >= K_LVL_ACT_ATTACK_SHOOTING))
	{
		if (!Weapon_CanShoot(actor->pWeaponMain))
		{
			actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_IDLE;
			actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
		}
	}

	//command weapon
	if (actor->m_AIcommands.eAttackCommand >= K_LVL_ACT_ATTACK_SHOOTING)
	{
		actor->pWeaponMain->SetTriggerStates(true, false);
	}
	else if (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_RELOADING)
	{
		if(actor->pWeaponMain->WeaponTemplate.nReloadUnitSize != 0)
			actor->pWeaponMain->SetTriggerStates(false, true);
	}
	else
	{
		actor->pWeaponMain->SetTriggerStates(false, false);
	}

	///--- update weapons ---
	for (int kk = 0; kk < actor->arrWeapons.Count(); kk++)
	{
		Weapon_Update(actor->arrWeapons[kk], dTime);
	}

	//check before ShootWeapon
	if (actor->m_AIcommands.eAttackCommand == K_LVL_ACT_ATTACK_IDLE)
	{
		if(actor->pWeaponMain->status != K_LVL_WPN_STATUS_COOLING)
			actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
	}


	//daca are laser sight o activeaza acum, o singura data cand se da comanda de shoot
	if ((actor->pWeaponMain->WeaponTemplate.bHasLaserSight) && (actor->nAttackStatus != actor->m_AIcommands.eAttackCommand) && (actor->m_AIcommands.eAttackCommand >= K_LVL_ACT_ATTACK_SHOOTING))
	{
		actor->pWeaponMain->bPaintLaserSight = true;
	}

	///--- set actor attack status from command
	if(actor->m_AIcommands.eAttackCommand != K_LVL_ACT_ATTACK_IDLE)
		actor->nAttackStatus = actor->m_AIcommands.eAttackCommand;

	//daca arma curenta nu poate trage din crouch scot crouch
	if ((actor->bCrouched == true) && (!actor->pWeaponMain->WeaponTemplate.bCanShootFromCrouch))
	{
		if (actor->nAttackStatus >= K_LVL_ACT_ATTACK_SHOOTING)
			actor->bCrouched = false;
	}

	// weapons that stop you while shooting:
	if (actor->pWeaponMain->WeaponTemplate.fShooterSpeedSlowingPercent >= 1.0f)
	{
		if ((actor->m_AIcommands.eAttackCommand != K_LVL_ACT_ATTACK_IDLE) || (actor->nAttackStatus != K_LVL_ACT_ATTACK_IDLE))
		{
			actor->m_AIcommands.bThrust = false;
			//actor->m_AIcommands.nMoveDirX = 0;
		}
	}

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
		if (actor->pWeaponMain->WeaponTemplate.shScript_OnFire.IsSet())
		{
			dwShootScriptUID = actor->pWeaponMain->WeaponTemplate.shScript_OnFire.getHash();
		}
		//Shooting ALT fire and primary weapon ALTfire script is set? then run it.
		/*
		else if ((actor->nAttackStatus == K_LVL_ACT_ATTACK_SHOOTING_ALT) && (actor->pCurrentWeapon->WeaponTemplate.shScript_OnFireALT.IsSet()))
		{
			dwShootScriptUID = actor->pCurrentWeapon->WeaponTemplate.shScript_OnFireALT.getHash();
		}
		*/
		
		//no shoot script
		if (dwShootScriptUID == 0)
		{
			//#PERSONALIZARE: check animation firerate reset and other weapon info
			if (aframeFlag & K_LVL_ACTIVE_AFRAMEFLAG_RESET_FIRE_RATE)
				actor->pWeaponMain->fireRateTimer = 0.0f;
			
			//#PERSONALIZARE: ca sa traga si in spate uneori (AKIMBO DUAL YELD)
			bool bShoot = ((aframeFlag & K_LVL_ACTIVE_AFRAMEFLAG_ACTION) != 0);
			bool bShootSymmetric = ((aframeFlag & K_LVL_ACTIVE_AFRAMEFLAG_ACTION_SYMMETRIC) != 0);

			//can we shoot?
			Vec3 vShootDir;
			vShootDir = Vec2ToVec3XY0(actor->m_AIcommands.vAimVec);

			//shoot without waiting for a flag (forward or back)
			if (Weapon_Shoot(actor->pWeaponMain, vShootDir))
				nWeaponShots++;
		}
		else  //shoot script
		{
			UTGetScriptManager().StartScript(dwShootScriptUID, actor->UID);
			actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
		}

		//DK - some weapons share the same magazine
		/*
		if ((nWeaponShots > 0) && (actor->pCurrentWeapon->WeaponTemplate.bUsesMainWeaponAmmo))
		{
			if (actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->ammoLeft > 0)
			{
				dec_limit(actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->ammoLeft, nWeaponShots, 0);
				//update the weapon in case we run out of ammo so it sets the correct status
				UpdateWeapon(actor->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY], 0.0f);
			}
		}
		*/
		///--- check weapon script on empty ---
		CWeapon* pWpnToCheck = actor->pWeaponMain;

		EnumWeaponStatus gunstat = pWpnToCheck->status;

		if ((gunstat == K_LVL_WPN_STATUS_NO_AMMO) || (gunstat == K_LVL_WPN_STATUS_BURST_END) || (gunstat == K_LVL_WPN_STATUS_JAMMED))
		{
			actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
			bRunScriptOnEmpty = true;
		}

		///--- launch script when weapon runs out of ammo:
		if ((bRunScriptOnEmpty) && (pWpnToCheck->WeaponTemplate.shScript_OnEmpty.IsSet()) && (pWpnToCheck->ammoLeft <= 0))
		{
			UTGetScriptManager().StartScript(actor->pWeaponMain->WeaponTemplate.shScript_OnEmpty.getHash(), actor->GetUID());
		}

	}


	//resetam stari reload la finalul incarcarii sau daca nu face arma reload
	if ((actor->nAttackStatus == K_LVL_ACT_ATTACK_RELOADING) && (actor->pWeaponMain->status != K_LVL_WPN_STATUS_RELOADING))
	{
		actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
	}

	///--- set icon ---
	if (actor->m_AIcommands.nIconType != K_LVL_ACT_ICON_NONE)
	{
		actor->SetIcon(actor->m_AIcommands.nIconType, actor->m_AIcommands.fIconDuration);
	}
	
	///--- speed ---

	if (actor->m_AIcommands.bThrust)
	{
		//add speed
		float fspeed = actor->actTemplate.fSpeedMove;

		// set final speed
		actor->speed = actor->m_AIcommands.vMoveDir * fspeed;
	}
	else
	{
		actor->speed = Vec2(0.0f, 0.0f);
	}

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

				if (!UTApp().m_Settings.bGoreEnabled)
				{
					g_particlesMgr.GenerateEnemySoftGib(actor->pos.xy_proj, 0xff32a7fa, K_PART_LAYER_RT_FRONT_NRM);
				}
				else
				{
					//blood splat (sortate crescator in animatie)
					AddDecal_BloodSplat(actor->GetPosHeart(), true, actor->actTemplate.actorClass);

					//SND_PLAY_POSITIONAL_RAND2(SNDIDX_BULLET_BODY_GIBBED_01, SNDIDX_BULLET_BODY_GIBBED_02, actor->GetPosHeart());
					//meat lumps
					Vec2 bulletSpeed;
					MUVec2Norm(&bulletSpeed, &actor->vSpeedImpulse);

					CAABB genbox = actor->bbox;
					genbox.Inflate(-2.0f, -2.0f);
					if (actor->actTemplate.fLife > 10.0f)
					{
						DWORD dwCol = 0xff671010;
						int nSubType = 0;
						if (actor->actTemplate.actorClass == K_LVL_ACT_CLASS_ZOMBIE)
						{
							dwCol = 0xff82b600;
							nSubType = 1;
						}
						for (int ll = 0; ll < 6; ll++)
						{
							AddDoofer(K_DOOFER_MEAT, AABB::GetRandomPointInBox(genbox), &Vec2(randfloatsgn(50.0f) + bulletSpeed.x * 50.0f, -130.0f - randfloat(100.0f)), &g_vecGravityOld, nSubType);
						}
						//goes straight down to stain the floor
						AddDoofer(K_DOOFER_MEAT, actor->GetPosHeart(), &Vec2(200.0f, 50.0f), &g_vecGravityOld, nSubType);
						AddDoofer(K_DOOFER_MEAT, actor->GetPosHeart(), &Vec2(-200.0f, 50.0f), &g_vecGravityOld, nSubType);
						//human blood gibs particle
						g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_HUMAN_SPLAT_MED, true, 0, &actor->pos.xy_proj, NULL, NULL, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, dwCol, K_PART_LAYER_RT_FRONT_NRM);
					}
					else //small animals and stuff
					{
						for (int ll = 0; ll < 2; ll++)
						{
							AddDoofer(K_DOOFER_MEAT, AABB::GetRandomPointInBox(genbox), &Vec2(randfloatsgn(50.0f) + bulletSpeed.x * 50.0f, -130.0f - randfloat(100.0f)), &g_vecGravityOld);
						}
						g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_HUMAN_SPLAT_SMALL, true, 0, &actor->pos.xy_proj, NULL, NULL, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xff671010, K_PART_LAYER_RT_FRONT_NRM);
					}
				}

				//players don't deallocate. They only become invisible.
				if (actor->actTemplate.actorClass == K_LVL_ACT_CLASS_PLAYER)
				{
					actor->fLife = 0.0f;
					actor->bHasGravity = false;
					actor->bHasCollision = false;
					actor->bSkipRender = true;
					//reset physics
					actor->speed = Vec2(0.0f, 0.0f);
					actor->vSpeedImpulse = Vec2(0.0f, 0.0f);
					//move invisible body back to last safe pos
					Vec2 vSpawnPos = m_arrPlayerLastSafePos[actor->nPlayerOrdinal];
					actor->SetPos(Vec3(vSpawnPos.x, vSpawnPos.y, 0.0f));
					break;
				}
				//deallocate
				actor->bSetHidden = true;
				actor->Kill();
			}
			break;
			case K_LVL_ACT_DEATHCMD_DEALLOCATE:
			{
				//dezalocare
				actor->bSetHidden = true;
				actor->Kill();
			}
			break;
		}
		//remove death command after execution
		actor->m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_EMPTY;
	}

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
	
	Vec3 vPosIni = actor->pos.xyz;
	UINT16 unCollFlags = 0;

	///--- collision detection ---
	{
		///a.calculezi vectorul de miscare al actorului(viteza * dt + miscare paltforma daca e necesar)
		Vec2 vNextMove = (actor->speed + actor->vSpeedImpulse) * dTime; // Add connected platform movement if needed
		///b.detectezi coliziuni posibile(bbox old + new pos)
		//1. find bbox start and end union that includes all collisions when moving at high speeds
		CAABB destbox, srcbox;
		srcbox = actor->bbox_ini; srcbox.Move(actor->pos.xy);
		destbox = actor->bbox_ini; destbox.Move(actor->pos.xy + vNextMove);
		// box unions to check all possible collisions
		CAABB boxUnion = AABB::Union(destbox, srcbox);
		// bbox union in tile coords, including every touched tile
		RECTXYXY boxUnionTiles(floor(boxUnion.vMin.x / K_TILE_SIZE_F), floor(boxUnion.vMin.y / K_TILE_SIZE_F),
			ceil(boxUnion.vMax.x / K_TILE_SIZE_F), ceil(boxUnion.vMax.y / K_TILE_SIZE_F));
		RECTXYWH boxUnionTilesWH(boxUnionTiles.x1, boxUnionTiles.y1, boxUnionTiles.x2 - boxUnionTiles.x1 + 1, boxUnionTiles.y2 - boxUnionTiles.y1 + 1);
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
			if (!boxUnion.Intersects(m_arrColShapes[kk]->bbox))
				continue;

			//adauga bbox in lista de probabile pt intersectie
			if (m_arrColShapes[kk]->collFlags != K_DIRFLAG_NONE)
			{
				tempCollBoxList.Add(m_arrColShapes[kk]->bbox);
			}
		}
		//add boxes from tiles
		//#MAYBE: if it catches some corners sometimes try enlarging the tiles collision area (boxUnionTiles) by 1 tile in all directions
		static CAABB retAABBs[64];
		if (actor->pArea != nullptr)
		{
			// get collision tiles for current area
			// tiles collboxes
			int nadded = actor->pArea->GetTilesCollisionBoxes(boxUnionTiles, retAABBs, 64);
			if (nadded > 0)
			{
				for (int oo = 0; oo < nadded; oo++)
				{
					tempCollBoxList.Add(retAABBs[oo]);
				}
			}
			// props collboxes
			nadded = actor->pArea->GetPropsCollisionBoxes(boxUnion, retAABBs, 64);
			if (nadded > 0)
			{
				for (int oo = 0; oo < nadded; oo++)
				{
					tempCollBoxList.Add(retAABBs[oo]);
				}
			}
			// if movement bbox is not completely contained in the current area BBox try with the neighbours too
			if (!actor->pArea->AABBbounds.Contains(boxUnion))
			{
				for (int kk = 0; kk < actor->pArea->arrNeighbours.Count(); kk++)
				{
					CLevelArea* area = actor->pArea->arrNeighbours.m_pData[kk];
					if (!area->AABBbounds_TL.Intersects(boxUnionTilesWH))
						continue;
					// tiles collboxes
					nadded = area->GetTilesCollisionBoxes(boxUnionTiles, retAABBs, 64);
					if (nadded > 0)
					{
						for (int oo = 0; oo < nadded; oo++)
						{
							tempCollBoxList.Add(retAABBs[oo]);
						}
					}
					// props collboxes
					nadded = area->GetPropsCollisionBoxes(boxUnion, retAABBs, 64);
					if (nadded > 0)
					{
						for (int oo = 0; oo < nadded; oo++)
						{
							tempCollBoxList.Add(retAABBs[oo]);
						}
					}
				}
			}
		}

		/// COLLISION HANDLING

		float fRemainingTime = 1.0f;
		while (fRemainingTime > 0.0f)
		{
			// compute source box
			srcbox = actor->bbox_ini; srcbox.Move(actor->pos.xy);
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

				actor->pos.xy += vNextMove * hit.fCollisionTime;

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
					newboxsrc.Move(actor->pos.xy);
					CAABB newboxdest = newboxsrc;
					newboxdest.Move(vNextMove);
					CAABB newBoundary = AABB::Union(newboxsrc, newboxdest);

					// call and implement this if you need tile sized boxes to enter tile wide holes
					//this.fixEqualSizedHoleCollision(hit, potential, time, collisionStack);

					//DMC: deactivate those boxes that don't fit the new boundary
					for (int kk = 0; kk < tempCollBoxList.Count(); kk++)
					{
						SweepAABB* it = &tempCollBoxList.m_pData[kk];
						if (it->bDisabled)
							continue;
						// disable non intersecting ones
						if (!newBoundary.Intersects(tempCollBoxList.m_pData[kk]))
							it->bDisabled = true;
					}
					// update remaining time and do again
					fRemainingTime = ftime;
				}

			}
			else
			{
				actor->pos.xy += vNextMove;
				fRemainingTime = 0.0f;
			}
		}

		//#TODO: could use a penetration resolution round. Maybe after solving each collision so we make sure boxes don't actually touch? TBD
	}

	actor->collisionFlags = unCollFlags;

	//check world bounds for each actor - kill if out
	if (!PointInRect(actor->pos.xy, m_levelAABB))
	{
		KillActor(actor);
	}

	//set final position
	actor->SetPos(Vec3(actor->pos.xy.x, actor->pos.xy.y, 0.0f));
	// save last position in pos_last (SetPos does but we already altered actor->pos)
	actor->pos_last = vPosIni;

	//#TODO: Speeds and accelerations should be treated here, after the collision detection
		
	///--- set current area if null or changed after updating the position
	if ((actor->pArea == nullptr) || (!actor->pArea->AABBbounds.PointIn(actor->pos.xy.x, actor->pos.xy.y)))
	{
		actor->pArea = Areas_GetAt(actor->pos.xy);
	}

	///--- find closest interactible object in range, aka touchable
	if (actor->actTemplate.eCaps & CActorTemplate::K_ACT_CAPS_CAN_INTERACT)
	{
		//#TODO: put interact area in special constant
		CAABB aabbInteract(-K_TILE_SIZE_F, -K_TILE_SIZE_F, K_TILE_SIZE_F, K_TILE_SIZE_F);
		aabbInteract.Move(actor->pos.xy);

		CArray<CProp*> arrTouchProps;
		arrTouchProps.SetSize(16);
		if (actor->pArea != nullptr)
		{
			actor->pArea->GetPropsTouchingBox(aabbInteract, arrTouchProps, true);
			// if bbox is not completely contained in the current area BBox try with the neighbours too
			if (!actor->pArea->AABBbounds.Contains(aabbInteract))
			{
				for (int oo = 0; oo < actor->pArea->arrNeighbours.Count(); oo++)
				{
					CLevelArea* area = actor->pArea->arrNeighbours.m_pData[oo];
					if (!area->AABBbounds.Intersects(aabbInteract))
						continue;
					area->GetPropsTouchingBox(aabbInteract, arrTouchProps, true);
				}
			}
		}
		//#TODO: see which one is closer to the aim dir
		if (arrTouchProps.GetSize() > 0)
		{
			IActiveInterface* pNewTouchable = arrTouchProps[0];
			if (actor->pClosestTouchable != pNewTouchable)
			{
				actor->ClearActionsList();
			}
			actor->pClosestTouchable = pNewTouchable;
		}
		else
		{
			if (actor->pClosestTouchable != nullptr) 
			{
				actor->ClearActionsList();
			}
			actor->pClosestTouchable = nullptr;
		}
	}

	// check touch/interact
	if ((actor->m_AIcommands.bInteract) && (actor->pClosestTouchable != nullptr))
	{
		actor->BuildActionsList();
		if (actor->arrInteractOptions.Count() > 0)
		{
			actor->eInteractState = K_STATE_READY;
			actor->nInteractOptionsSelIdx = 0;
		}
	}

	///--- call internal actor update at the end
	actor->Update(dTime);
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
	float fDistSee = 100.0f;// sourceActor->actTemplate.distSee;
	float fDistHear = 100.0f;// sourceActor->actTemplate.distHear;
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
		Vec2(sourceActor->pos.xy.x/* + sourceActor->lookDirXsign * fDistSee*/, sourceActor->pos.xy.y + fDistDown),
		Vec2(sourceActor->pos.xy.x/* - sourceActor->lookDirXsign * fDistHear*/, sourceActor->pos.xy.y - fDistUp)
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
		if (enemy->actTemplate.actorClass == sourceActor->actTemplate.actorClass)
			continue;

		if (sourceActor->actTemplate.actorClass == K_LVL_ACT_CLASS_ZOMBIE)
		{
			//zombie classes attack everything that's made from meat
			if (enemy->actTemplate.eMaterial != K_LVL_MATERIAL_FLESH)
				continue;
		}
		else
		{
			//don't attack same class enemies or traps and passive classes
			if (enemy->actTemplate.actorClass < K_LVL_ACT_CLASS_PLAYER)
				continue;
		}

		//daca am filtru pe clasele de inamici verific clasa mai intai
		int nIgnore = 0, nIgnoreConditions = 0;
		if (eTargetClassFilter1 != K_LVL_ACT_CLASS_ANY)
		{
			nIgnoreConditions++;
			if (enemy->actTemplate.actorClass != eTargetClassFilter1)
				nIgnore++;
		}
		if (eTargetClassFilter2 != K_LVL_ACT_CLASS_ANY)
		{
			nIgnoreConditions++;
			if (enemy->actTemplate.actorClass != eTargetClassFilter2)
				nIgnore++;
		}
		if ((nIgnoreConditions > 0) && (nIgnore == nIgnoreConditions))
			continue;
		//nu ia in seama inamic cu energie sub 0 sau flag de not a target (setat de limbo)
		if ((enemy->fLife <= 0.0f) || ((enemy->actTemplate.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET) != 0))
			continue;

		Vec2 enemyDistV = enemy->GetPosHeart() - sourceActor->GetPosHeart();
		float viewDstSq = 100.0f * 100.0f;//sourceActor->actTemplate.distSee * sourceActor->actTemplate.distSee;
		float enemyDistSq = MUVec2LenSq(&enemyDistV);

		bool bPreciseFOV = false; //approximate FOV with rectangle? (good for gameplay)
		//if ((sourceActor->actTemplate.eCaps & CActorTemplate::K_ACT_CAPS_CAN_ROTATE_VIEW) != 0)
			//bPreciseFOV = true;

		if (bPreciseFOV)
		{
			/*
			//daca inamicul este in spate modifica raza pe cea de auzit, doar daca nu e alertat la maxim. Daca are fov maxim ramane raza vizuala si in spate.
			if ((sourceActor->fFOVPercent < 1.0f) && (sourceActor->actTemplate.distHear > 0.0f) && (SIGN(enemyDistV.x) != SIGN(sourceActor->lookDirXsign)))
			{
				viewDstSq = sourceActor->actTemplate.distHear * sourceActor->actTemplate.distHear;
				//daca il poate auzi si e in linie directa, il aude
				if (enemyDistSq <= viewDstSq)
				{
					if (!IsLineOfSight(sourceActor->GetPosHeart(), enemy->GetPosHeart()))
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
			if ((sourceActor->fFOVPercent < 1.0f) && (UTMath::GetAngleBetweenVectors(enemy->GetPosHeart() - sourceActor->GetPosHeart(), sourceActor->vAngleDir) > (HALF_PI * sourceActor->fFOVPercent)))
				continue;
			//verifica daca am linie directa de vedere
			if (!IsLineOfSight(sourceActor->GetPosHeart(), enemy->GetPosHeart()))
				continue;
				*/
		}
		else //Dreptunghi of Vision! such fast! Much optimal!
		{
			//not in view rectangle
			if (!aabbvision.PointIn(enemy->GetPosHeart()))
				continue;
			if (!IsLineOfSight(sourceActor->GetPosHeart(), enemy->GetPosHeart()))
				continue;
		}

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
		if ((enemy == null) || (enemy == sourceActor) || (enemy->actTemplate.shID.textHash != nTargetNameHash) || (enemy->bHidden))
			continue;
		//nu ia in seama inamic cu energie sub 0 sau flag de not a target
		if ((enemy->fLife <= 0.0f) || ((enemy->actTemplate.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET) != 0))
			continue;

		Vec2 enemyDistV = enemy->GetPosHeart() - sourceActor->GetPosHeart();
		float enemyDistSq = MUVec2LenSq(&enemyDistV);
		//daca e prea departe trece mai departe
		float fSearchRadiusSq = (fMaxDistance * fMaxDistance);
		if (enemyDistSq > fSearchRadiusSq)
		{
			continue;
		}
		//daca e destul de aproape:
		//verifica daca am linie directa de vedere
		if (!IsLineOfSight(sourceActor->GetPosHeart(), enemy->GetPosHeart()))
			continue;

		//daca a trecut toate testele si inamicul curent este mai aproape decat cel selectat initial il setez pe cel nou
		if ((retvalenemy == null) || (MUVec2LenSq(&(retvalenemy->GetPosHeart() - sourceActor->GetPosHeart())) > enemyDistSq))
			retvalenemy = enemy;
	}

	return retvalenemy;
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
		/*
		int nIgnore = 0, nIgnoreConditions = 0;
		if (callerActor->actTemplate.foeClassFilter1 != K_LVL_ACT_CLASS_ANY)
		{
			nIgnoreConditions++;
			if (evt->ownerClass != callerActor->actTemplate.foeClassFilter1)
				nIgnore++;
		}
		if (callerActor->actTemplate.foeClassFilter2 != K_LVL_ACT_CLASS_ANY)
		{
			nIgnoreConditions++;
			if (evt->ownerClass != callerActor->actTemplate.foeClassFilter2)
				nIgnore++;
		}
		if ((nIgnoreConditions > 0) && (nIgnore == nIgnoreConditions))
			continue;
			*/
		//!!! daca e event targetat il intoarce direct si il consuma, fara sa mai stea pe ganduri, cu exceptia IDLE_TICK
		if ((evt->targetUID == callerUID) && (evt->nType > K_LVL_AI_EVENT_IDLE_TICK))
			return evt;
		//verific distanta (daca raza event nu e infinita adica negativa)
		float evtdstsq = 0.0f;
		if (evt->fRadius > 0.0f)
		{
			evtdstsq = MUVec2LenSq(&(callerActor->GetPosHeart() - evt->pos));
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
		if (callerActor->actTemplate.AItemplate->m_arrIgnoredEvents.Count() > 0)
		{
			if (callerActor->actTemplate.AItemplate->m_arrIgnoredEvents.IndexOf(evt->nType) >= 0)
				continue;
		}
		//dupa ce am exclus eventurile ce se puteau exclude:
		//verific linie directa, cel mai costisitor test, sau daca e event cu raza infinita (fara pozitie)
		if ((evt->fRadius < 0.0f) || (IsLineOfSight(callerActor->GetPosHeart(), evt->pos)))
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


void CLevel::SetAI(IActiveInterface * active, EAIstate AIstate, CVariantCollection * params, INT32 targetID)
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
				vLastSpawnPoint = active->pos.xy;
			}
		}
		break;
		case K_AI_STATE_FN_TOUCH_WHEN_SEE_PLAYER:
		{
			/*
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
			*/
		}
		break;
		case K_AI_STATE_COLL_FOG_OF_WAR:
		{
			active->AIfvar1 = 1.0f; //transparenta (full opaque)
			active->color = active->color_ini = DW_COLORALPHA(K_LVL_COLL_FOW_COLOR, active->AIfvar1);
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
	for (int ar = 0; ar < m_arrAreas.Count(); ar++)
	{
		CLevelArea* area = m_arrAreas[ar];
		if(!area->bActive)
			continue;
		// check props lifetime
		for (int kk = area->m_arrProps.GetSize() - 1; kk >= 0; kk--)
		{
			if (area->m_arrProps[kk]->IsPendingKill())
			{
				// call framework end play
				area->m_arrProps[kk]->EndPlay();
				// remove from array, call dtor
				SAFE_DELETE(area->m_arrProps[kk]);
				area->m_arrProps.Remove(kk);
			}
		}
	}

	//check actors
	for (int kk = m_arrActors.GetSize() - 1; kk >= 0; kk--)
	{
		///--- must kill actor! - last thing in update - dezalocari finale ---
		if (m_arrActors[kk]->IsPendingKill())
		{
			CActor* act = m_arrActors[kk];
			// call framework end play
			act->EndPlay();
			//#HACK: make sure we don't keep pointer to actor - should be replaced by weak_ptr
			for (int i = 0; i < m_arrActors.GetSize(); i++)
			{
				if (m_arrActors[i]->m_AIsensorInfo.pTargetedActor == act)
				{
					m_arrActors[i]->m_AIsensorInfo.pTargetedActor = NULL;
				}
			}
			// now release it (destructor)
			SAFE_DELETE(m_arrActors[kk]);
			m_arrActors.Remove(kk);
		}
	}

	//check lights
	for ( int kk = m_arrLights.GetSize() - 1; kk >= 0; kk-- )
	{
		if ( m_arrLights[ kk ]->IsPendingKill() )
		{
			m_arrLights[ kk ]->EndPlay();
			SAFE_DELETE( m_arrLights[ kk ] );
			m_arrLights.Remove( kk );
		}
	}

	//check bullets
	for ( int kk = m_arrBullets.Count() - 1; kk >= 0; kk-- )
	{
		if ( m_arrBullets[ kk ]->bPendingKill )
		{
			SAFE_DELETE( m_arrBullets[ kk ] );
			m_arrBullets.Remove( kk );
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

	//check active objects
	for (int ar = 0; ar < m_arrAreas.Count(); ar++)
	{
		CLevelArea* area = m_arrAreas[ar];
		if (!area->bActive)
			continue;
		for (int kk = area->m_arrProps.GetSize() - 1; kk >= 0; kk--)
		{
			UpdateAI_prop(area->m_arrProps[kk], dTime);
		}
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
		fHashKey += act->pos.xyz.x + act->pos.xyz.y + act->AItimerDecision + act->fLife + act->fArmor + act->fStunTimer;

		//count targets left
		if (act->GetCurrentBehavior() != EAIBehaviorType::AI_BEHAVIOR_DEAD)
		{
			if ((act->actTemplate.actorClass >= K_LVL_ACT_CLASS_HUMAN) || (act->actTemplate.actorClass == K_LVL_ACT_CLASS_HOSTAGE))
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
		//Random_ShuffleArray(arrLevels, nValidLevels, 1000);
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

void CLevel::UpdateFixedTimestep(float dTime_original)
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
					for (size_t ll = 0; ll < UTGetCtrlrMgr().m_arrControllers.size(); ll++)
					{
						CController* ctrlr = UTGetCtrlrMgr().m_arrControllers[ll];
						//Shows controller mapping - only when not online
						if ((ctrlr->eType == K_CM_CT_JOYSTICK_SDL) && (!UTApp().IsGameNetworked()) && (false == UTGetGUI().bIsBlocking) && 
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

									//m_interfaceIGM.SetHotJoinSelection(plidx, m_arrPlayerSelHotJoin[plidx]);
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
								if ((pPlayerActor[nOtherPlayerIdx] != null) && (pPlayerActor[nOtherPlayerIdx]->collisionFlags & K_DIRFLAG_DOWN))
								{
									//only spawn if player is there
									EAIBehaviorType eOtherBehave = pPlayerActor[nOtherPlayerIdx]->GetCurrentBehavior();
									if ((eOtherBehave == AI_BEHAVIOR_PLAYER_CONTROL) || (eOtherBehave == AI_BEHAVIOR_DEAD))
									{
										vSpawnPos = pPlayerActor[nOtherPlayerIdx]->pos.xy;
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
//									PlayActorSoundVerse(pPlayerActor[plidx], K_LVL_ACT_VERSE_JOIN_GAME);

									//scad numarul de vieti si anunt interfata
									if (m_arrStats[K_LVL_STATS_PL1_LIVES + plidx * K_LVL_STATS_PLAYER_STATS_COUNT] > 0)
										m_arrStats[K_LVL_STATS_PL1_LIVES + plidx * K_LVL_STATS_PLAYER_STATS_COUNT]--;

									//m_interfaceIGM.SetLivesLeft(m_arrStats[K_LVL_STATS_PL1_LIVES], m_arrStats[K_LVL_STATS_PL2_LIVES]);
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
						//m_interfaceIGM.Init(&m_sprInterface, pPlayerActor[0], pPlayerActor[1]);
					}
				}
				else //hot join ingame selection and spawning
				{
					//m_interfaceIGM.SetHotJoinSelection(plidx, m_arrPlayerSelHotJoin[plidx]);
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
			bool bMissionFinished = false;
			int nStrIdxMissionFailed = -1; //means win if -1 or lose if >=0
			/*
			bool bMissionFinished = true;
			if ((m_arrStats[K_LVL_STATS_LEVEL_HAS_BOMBS] != 0) && (m_arrStats[K_LVL_STATS_BOMBS_DISARMED] == 0))
				bMissionFinished = false;
			if (m_arrStats[K_LVL_STATS_TARGETS_LEFT] > 0)
				bMissionFinished = false;

			///--- level failed if killed all hostages  ---
			//only fail because of hostages on hostage rescue missions
			if ((m_nLoadedLevelType == K_GAME_LSTYPE_HOSTAGE) &&
				(m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL] > 0) && 
				(m_arrStats[K_LVL_STATS_HOSTAGES_KILLED] >= m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]))
			{
				bMissionFinished = true;
				nStrIdxMissionFailed = STR_HOSTAGES_KILLED;
			}
			*/
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
			if (UTApp().IsGameNetworked())
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
						if (UTApp().m_Settings.devnet_eNetGameType == CApplicationSettings::K_NETGAME_TYPE_QUICK_MATCH)
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

					if (!GameState::isTransitioning())
					{
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_PLAYER_SELECTION);
						nevent->AddNamedArgINT32(L"arg1", 0); //reset player selection
						nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
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

					if (!GameState::isTransitioning())
					{
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_PLAYER_SELECTION);
						nevent->AddNamedArgINT32(L"arg1", 0); //reset player selection
						nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
						UTGetEventManager().QueueEvent(nevent);
					}
					//clear command
					g_netlock.Net_LevelResultsClearStates();
				}
				//cancel button / command / window
				if (g_netlock.Net_LevelResultsCountStates(CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CANCEL) > 0)
				{
					LOG(L"Game::Level Win: Player chose to exit!");

					if (!GameState::isTransitioning())
					{
						//change game state
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
						nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
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
						__Texts().SetString(STR_MISSION_P1_KILLS, L"%d", m_arrStats[K_LVL_STATS_PL1_KILLS]);
						if (m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT] > 0)
							__Texts().SetString(STR_MISSION_P1_ACCURACY, L"%.1f%%", 100.0f * fAccuracyP1);
						else
							__Texts().SetString(STR_MISSION_P1_ACCURACY, L"%s", __Texts().strings[STR_NOT_AVAILABLE]->sText);
						__Texts().SetString(STR_MISSION_P1_HOSTAGES, L"%d / %d", m_arrStats[K_LVL_STATS_PL1_HOSTAGES_SAVED], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);
						__Texts().SetString(STR_MISSION_P1_DEATHS, L"%d", m_arrStats[K_LVL_STATS_PL1_DEATHS]);
						//--- PL2 data ---
						float fAccuracyP2 = 1.0f;
						if (m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT] > 0)
							fAccuracyP2 = (float)m_arrStats[K_LVL_STATS_PL2_BULLETS_HIT] / (float)m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT];
						CLAMP(fAccuracyP2, 0.0f, 1.0f);
						__Texts().SetString(STR_MISSION_P2_KILLS, L"%d", m_arrStats[K_LVL_STATS_PL2_KILLS]);
						if (m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT] > 0)
							__Texts().SetString(STR_MISSION_P2_ACCURACY, L"%.1f%%", 100.0f * fAccuracyP2);
						else
							__Texts().SetString(STR_MISSION_P2_ACCURACY, L"%s", __Texts().strings[STR_NOT_AVAILABLE]->sText);
						__Texts().SetString(STR_MISSION_P2_HOSTAGES, L"%d / %d", m_arrStats[K_LVL_STATS_PL2_HOSTAGES_SAVED], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);
						__Texts().SetString(STR_MISSION_P2_DEATHS, L"%d", m_arrStats[K_LVL_STATS_PL2_DEATHS]);

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
						__Texts().SetString(STR_MISSION_TIME, tmpstr);
						__Texts().SetString(STR_MISSION_CASUALTIES, L"%d", m_arrStats[K_LVL_STATS_PL1_DEATHS] + m_arrStats[K_LVL_STATS_PL2_DEATHS]);
						__Texts().SetString(STR_MISSION_SCORE, L"%d", nTotalLevelScore);

						int nHostagesSaved = m_arrStats[K_LVL_STATS_PL1_HOSTAGES_SAVED] + m_arrStats[K_LVL_STATS_PL2_HOSTAGES_SAVED];
						__Texts().SetString(STR_MISSION_HOSTAGES, L"%d / %d", nHostagesSaved, m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);

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
							if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
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
							if (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE)
							{
								if (g_levelStats[nLevelIdx].nScoreCoop < nTotalLevelScore)
									g_levelStats[nLevelIdx].nScoreCoop = nTotalLevelScore;
								if ((g_levelStats[nLevelIdx].nBestTimeSec_Coop == 0) || (g_levelStats[nLevelIdx].nBestTimeSec_Coop < nTimeSpent))
									g_levelStats[nLevelIdx].nBestTimeSec_Coop = nTimeSpent;
							}

							//XP points	save
							if (!UTApp().IsGameNetworked())
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
							if (!UTApp().IsGameNetworked())
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
							//reset old scores
							UTGetLeaderboards().ResetScoresList();
							//reset strings too
							__Texts().SetString(STR_LEADERBOARDS_NAMES_VAL, L"...");
							__Texts().SetString(STR_LEADERBOARDS_SCORES_VAL, L"...");
							__Texts().SetString(STR_LEADERBOARDS_PLAYERSCORE_VAL, L"...");
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
									if ((ctrl = layer->GetControlByName("CTRL_ANIM_PORTRAIT_PL1")) != nullptr)
									{
										ctrl->paramsDict.SetNamedVarINT32(L"setFrame", (int)g_playerSelScr.m_arrPlayers[0].eType);
									}
									//XP bar
									if ((ctrl = layer->GetControlByName("CTRL_XPBAR_PL1")) != nullptr)
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
									if ((ctrl = layer->GetControlByName("CTRL_ANIM_PORTRAIT_PL1")) != nullptr)
									{
										ctrl->paramsDict.SetNamedVarINT32(L"setFrame", (int)g_playerSelScr.m_arrPlayers[0].eType);
									}
									if ((ctrl = layer->GetControlByName("CTRL_ANIM_PORTRAIT_PL2")) != nullptr)
									{
										ctrl->paramsDict.SetNamedVarINT32(L"setFrame", (int)g_playerSelScr.m_arrPlayers[1].eType);
									}
								}

								//network - replace player names with real ones
								if (UTApp().IsGameNetworked())
								{
									if ((ctrl = layer->GetControlByName("CTRL_WND_PL1")) != nullptr)
									{
										ctrl->paramsDict.SetNamedVarINT32(L"stringID", STR_NETWORK_HOST_NAME);
									}
									if ((ctrl = layer->GetControlByName("CTRL_WND_PL2")) != nullptr)
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
									if ((ctrl = layer->GetControlByName("CTRL_XPBAR_PL1")) != nullptr)
									{
										ctrl->paramsDict.SetNamedVarINT32(L"nOldValue", g_playerSelScr.m_arrPlayers[0].nPlayerXPPts);
										int nNew = LIMIT(g_playerSelScr.m_arrPlayers[0].nPlayerXPPts + nTotalXPPoints, 0, nMaxXPPoints);
										ctrl->paramsDict.SetNamedVarINT32(L"nNewValue", nNew);
									}
									if ((ctrl = layer->GetControlByName("CTRL_XPBAR_PL2")) != nullptr)
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
									if ((ctrl = layer->GetControlByName("CTRL_XPBAR_PL1")) != nullptr)
									{
										int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
										ctrl->paramsDict.SetNamedVarINT32(L"nOldValue", nXPpl1);
										ctrl->paramsDict.SetNamedVarINT32(L"nNewValue", g_userData[nPlBaseIdx]);
									}
									if ((ctrl = layer->GetControlByName("CTRL_XPBAR_PL2")) != nullptr)
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
							UTApp().App_OnLevelFinished(g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL]);
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
								if ((ctrl = lay->GetControlByName("LABEL_LBTYPE")) != nullptr)
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
										__Texts().SetString(STR_TEMP10, L"%d.%d %s", nChapter + 1, nLevel + 1, __Texts().strings[nStrIdxLevelName]->sText);
									else
										__Texts().SetString(STR_TEMP10, L"%d.%d", nChapter + 1, nLevel + 1);
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
			if (UTApp().IsGameNetworked())
			{
				g_netlock.Net_UpdateLevelResults(dTime);
				//show net votes
				CCtrlLayer* layer = UTGetGUI().GetTopmostInputLayer();
				if (layer)
				{
					CControl* ctrl;
					//vote restart level
					if ((ctrl = layer->GetControlByName("CTRL_NETVOTE_RESTART")) != nullptr)
					{
						ctrl->paramsDict.SetNamedVarINT32(L"leftVote", (g_netlock.m_arrLvlResPeerStates[0] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_RESTART) ? 1 : 0);
						ctrl->paramsDict.SetNamedVarINT32(L"rightVote", (g_netlock.m_arrLvlResPeerStates[1] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_RESTART) ? 1 : 0);
					}
					//vote continue to next level
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
						if (UTApp().m_Settings.devnet_eNetGameType == CApplicationSettings::K_NETGAME_TYPE_QUICK_MATCH)
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

					if (!GameState::isTransitioning())
					{
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_PLAYER_SELECTION);
						nevent->AddNamedArgINT32(L"arg1", 0); //reset player selection
						nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
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

					if (!GameState::isTransitioning())
					{
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_PLAYER_SELECTION);
						nevent->AddNamedArgINT32(L"arg1", 0); //reset player selection
						nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
						UTGetEventManager().QueueEvent(nevent);
					}
					//clear command
					g_netlock.Net_LevelResultsClearStates();
				}
			
				//if someone clicked cancel throw us to main menu without error
				if (g_netlock.Net_LevelResultsCountStates(CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CANCEL) > 0)
				{
					LOG(L"Game::Level results: Peer left the game! Quit lobby!");

					if (!GameState::isTransitioning())
					{
						//change game state
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
						nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
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
						__Texts().SetString(STR_MISSION_P1_KILLS, L"%d", m_arrStats[K_LVL_STATS_PL1_KILLS]);
						if (m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT] > 0)
							__Texts().SetString(STR_MISSION_P1_ACCURACY, L"%.1f%%", 100.0f * fAccuracyP1);
						else
							__Texts().SetString(STR_MISSION_P1_ACCURACY, L"%s", __Texts().strings[STR_NOT_AVAILABLE]->sText);
						__Texts().SetString(STR_MISSION_P1_HOSTAGES, L"%d / %d", m_arrStats[K_LVL_STATS_PL1_HOSTAGES_SAVED], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);
						__Texts().SetString(STR_MISSION_P1_DEATHS, L"%d", m_arrStats[K_LVL_STATS_PL1_DEATHS]);
						//--- PL2 data ---
						float fAccuracyP2 = 1.0f;
						if (m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT] > 0)
							fAccuracyP2 = (float)m_arrStats[K_LVL_STATS_PL2_BULLETS_HIT] / (float)m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT];
						CLAMP(fAccuracyP2, 0.0f, 1.0f);
						__Texts().SetString(STR_MISSION_P2_KILLS, L"%d", m_arrStats[K_LVL_STATS_PL2_KILLS]);
						if (m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT] > 0)
							__Texts().SetString(STR_MISSION_P2_ACCURACY, L"%.1f%%", 100.0f * fAccuracyP2);
						else
							__Texts().SetString(STR_MISSION_P2_ACCURACY, L"%s", __Texts().strings[STR_NOT_AVAILABLE]->sText);
						__Texts().SetString(STR_MISSION_P2_HOSTAGES, L"%d / %d", m_arrStats[K_LVL_STATS_PL2_HOSTAGES_SAVED], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);
						__Texts().SetString(STR_MISSION_P2_DEATHS, L"%d", m_arrStats[K_LVL_STATS_PL2_DEATHS]);

						///--- XP Points ---
						int nXPpl1 = 0, nXPpl2 = 0;
						int nMaxXPPoints = App_GetMaxXP(K_GAME_MAX_UPGRADE_LEVELS);
						int nTotalXPPoints = Local_ComputeMissionXP(0);

						//--- STARS WINDOW ---
						int nTimeSpent = m_arrStats[K_LVL_STATS_LEVEL_END_SEC] - m_arrStats[K_LVL_STATS_LEVEL_START_SEC];
						OS_FormatTime(tmpstr, MAX_PATH, (float)(nTimeSpent));
						__Texts().SetString(STR_MISSION_TIME, tmpstr);

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
							if (!UTApp().IsGameNetworked())
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
								if ((ctrl = layer->GetControlByName("CTRL_XPBAR_PL1")) != nullptr)
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
							if(!UTApp().IsGameNetworked())
								layer = UTGetGUI().ShowLayerOnce("LAYER_ID_LEVELFAIL_2P");
							else
								layer = UTGetGUI().ShowLayerOnce("LAYER_ID_LEVELFAIL_2P_COOP");

							if (layer != null)
							{
								CControl* ctrl = null;

								if ((ctrl = layer->GetControlByName("CTRL_STARS")) != nullptr)
								{
									ctrl->paramsDict.SetNamedVarINT32(L"nStars", 0);
								}
								//reason why
								if (m_levelStateParam > 0) //if set
								{
									if ((ctrl = layer->GetControlByName("BLINKER_REASON")) != nullptr)
									{
										ctrl->paramsDict.SetNamedVarINT32(L"stringID", m_levelStateParam);
									}
								}
								//portrete								
								if ((ctrl = layer->GetControlByName("CTRL_ANIM_PORTRAIT_PL1")) != nullptr)
								{
									ctrl->paramsDict.SetNamedVarINT32(L"setFrame", (int)g_playerSelScr.m_arrPlayers[0].eType);
								}
								if ((ctrl = layer->GetControlByName("CTRL_ANIM_PORTRAIT_PL2")) != nullptr)
								{
									ctrl->paramsDict.SetNamedVarINT32(L"setFrame", (int)g_playerSelScr.m_arrPlayers[1].eType);
								}

								//network - replace player names with real ones
								if (UTApp().IsGameNetworked())
								{
									if ((ctrl = layer->GetControlByName("CTRL_WND_PL1")) != nullptr)
									{
										ctrl->paramsDict.SetNamedVarINT32(L"stringID", STR_NETWORK_HOST_NAME);
									}
									if ((ctrl = layer->GetControlByName("CTRL_WND_PL2")) != nullptr)
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
									if ((ctrl = layer->GetControlByName("CTRL_XPBAR_PL1")) != nullptr)
									{
										ctrl->paramsDict.SetNamedVarINT32(L"nOldValue", g_playerSelScr.m_arrPlayers[0].nPlayerXPPts);
										int nNew = LIMIT(g_playerSelScr.m_arrPlayers[0].nPlayerXPPts + nTotalXPPoints, 0, nMaxXPPoints);
										ctrl->paramsDict.SetNamedVarINT32(L"nNewValue", nNew);
									}
									if ((ctrl = layer->GetControlByName("CTRL_XPBAR_PL2")) != nullptr)
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
									if ((ctrl = layer->GetControlByName("CTRL_XPBAR_PL1")) != nullptr)
									{
										int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
										ctrl->paramsDict.SetNamedVarINT32(L"nOldValue", nXPpl1);
										ctrl->paramsDict.SetNamedVarINT32(L"nNewValue", g_userData[nPlBaseIdx]);
									}
									if ((ctrl = layer->GetControlByName("CTRL_XPBAR_PL2")) != nullptr)
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
							UTApp().App_OnLevelFinished(g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL]);
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


	//set update done flag
	m_bOneUpdateDone = true;
}


void CLevel::Update( float dTime )
{
	///--- update camera ---
	//default camera position following the players
	Vec2 avg_live( 0.0f, 0.0f ), avg_all( 0.0f, 0.0f );
	int plcnt_live = 0, plcnt_all = 0;
	for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
	{
		//on networked games ignore the peer and stay locked onto the player
		if ( UTApp().IsGameNetworked() )
		{
			int nIndexToFollow = g_netlock.Net_GetPlayerIndex();
			// move camera on peer after you die
			if ( pPlayerActor[ g_netlock.Net_GetPlayerIndex() ] == NULL )
			{
				nIndexToFollow = g_netlock.Net_GetOtherPlayerIndex();
				//if other player is dead too, just skip them and look at last spawn pos
				if ( pPlayerActor[ nIndexToFollow ] == NULL )
					continue;
			}
			//in networked games just ignore the other player
			if ( ( UTApp().IsGameNetworked() ) && ( kk != nIndexToFollow ) )
				continue;
		}

		if ( ( pPlayerActor[ kk ] != NULL ) && ( pPlayerActor[ kk ]->GetCurrentBehavior() != AI_BEHAVIOR_IN_LIMBO ) && ( pPlayerActor[ kk ]->nSuspendedFlags == K_LVL_SUSPENDFLAG_NONE ) )
		{
			//#TODO: add constants or special camera class for this wicked camera movement
			const float fMaxCameraMovement = K_TILE_SIZE_F * 4.0f;
			const float fMaxAimVecRadius = K_TILE_SIZE_F * 8.0f;
			const float fCameraDeadRadius = K_TILE_SIZE_F * 1.0f;
			// find look direction normalized
			float fLookDist = MUVec2Len( &pPlayerActor[ kk ]->m_AIcommands.vAimVec );
			Vec2 fLookOff = pPlayerActor[ kk ]->m_AIcommands.vAimVec / fLookDist;
			// normalize distance and square it so if varies less when cursor is close to character
			fLookDist -= fCameraDeadRadius;
			fLookDist /= fMaxAimVecRadius;
			CLAMP( fLookDist, 0.0f, 1.0f );
			//fLookDist *= fLookDist;
			// compute final camera vector
			fLookOff *= fLookDist * fMaxCameraMovement;

			if ( pPlayerActor[ kk ]->GetCurrentBehavior() != AI_BEHAVIOR_DEAD )
			{
				avg_live += Vec3XY( pPlayerActor[ kk ]->pos_last ) + fLookOff;
				plcnt_live++;
			}

			avg_all += Vec3XY( pPlayerActor[ kk ]->pos_last ) + fLookOff;
			plcnt_all++;
		}
	}

	//average player positions
	bool bAvgSet = false;
	Vec2 vPlayersAvg( 0.0f, 0.0f );
	if ( plcnt_all > 0 )
	{
		bAvgSet = true;

		avg_all /= plcnt_all;
		vPlayersAvg = avg_all;

		if ( plcnt_live > 0 )
		{
			avg_live /= plcnt_live;
			//are they too far apart? 
			if ( MUVec2Len( &( avg_all - avg_live ) ) > UTApp().g_rectRT.h * 0.5f )
			{
				vPlayersAvg = avg_live;
			}
		}
	}

	//handles render size changes
	m_camLevelToRT.SetViewport( UTApp().g_rectRT );
	m_camLevelToScr.SetViewport( UTApp().g_rectRender );
	if ( g_editor.IsLaunched() )
	{
		m_camLevelToRT.SetCamPos( &g_editor.m_vCamPos );
	}
	else
	{
		//no target camera object? look at the player pos average
		if ( m_camTargetActive == null )
		{
			if ( bAvgSet )
				m_vCamPosDefault = vPlayersAvg;

			m_camLevelToRT.SetCamPos( &m_vCamPosDefault );
		}
		else
		{
			m_camLevelToRT.SetCamPos( &( m_camTargetActive->pos.xy_proj ) );
		}
	}

	m_camLevelToRT.Update( dTime );
	Vec3 vCamPos = m_camLevelToRT.GetCamPos();
	m_camLevelToScr.SetCamPos( &Vec2( vCamPos.x, vCamPos.y ), vCamPos.z );
	m_camLevelToScr.Update( dTime );

	//find visible area
	RECTXYWH_F camrect = m_camLevelToRT.GetCamWorldAABB();
	CAABB camAABB( Vec2( camrect.x, camrect.y ), Vec2( camrect.Right(), camrect.Bottom() ) );

	//set sounds listener position
	SND_SET_LISTENER_POS( camrect.Center() );
	//--- update particles and emitters ---
	g_particlesMgr.UpdatePartEmitters( dTime, camrect );
	g_particlesMgr.Update( dTime );
	g_particlesMgr.UpdateStringDummies( dTime );

	Areas_UpdateVisibility( camrect );
	///--- update visibility lists (after update) ---
	BuildVisibilityLists();

	// builds all dynamic meshes necessary for drawing the next frame
	BuildDynamicGeometry( camAABB );

	///--- update interface ---
	m_interfaceIGM.Update( dTime );
	//m_interfaceTextBubble.Update(dTime);
}

OPRESULT CLevel::PaintDeferredBuffers( float fBetweenFramesPercent )
{
	CRTManager::CEngineRenderTarget* pRT = nullptr;
	///----------------------------------------------------
	/// 1. NORMAL MAP AND HEIGHT MAP
	///----------------------------------------------------
	pRT = UTGetRTManager().GetRTbyUID(K_RTID_TEMP1);
	if (pRT != null)
	{
		if(OP_SUCCESS(UTGetRTManager().BeginSceneRT(pRT)))
		{
			// Clear the render target and the zbuffer 
			if (FAILED(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, K_GAME_CLEAR_COLOR, 1.0f, 0)))
			{
				return K_OP_FAILED;
			}
			//use sprite
			//m_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_OBJECTSPACE | D3DXSPRITE_DONOTSAVESTATE);

			//Mat matProj;
			//D3DXMatrixOrthoOffCenterLH(&matProj, 0.5f, pRT->nWidth + 0.5f, pRT->nHeight + 0.5f, 0.5f, 0.0f, 1.0f);
			m_pDevice->SetTransform(D3DTS_PROJECTION, &pRT->matProj);

			m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
			m_pDevice->SetTransform(D3DTS_VIEW, &g_matIdentity);

			RenderPass(K_LVL_RP_NORMALS_HEIGHT, &pRT->matProj, fBetweenFramesPercent);

			// end sprite
			//m_pSprite->End();

			V_OP_RET(UTGetRTManager().EndSceneRT(pRT));
		}
	}

	///----------------------------------------------------
	/// 2. LIGHT MAP
	///----------------------------------------------------
	pRT = UTGetRTManager().GetRTbyUID(K_RTID_COLORDEPTHSTENCIL);
	if (pRT != null)
	{
		if(OP_SUCCESS(UTGetRTManager().BeginSceneRT(pRT)))
		{
			// Clear the render target and the zbuffer 
			if (FAILED(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0)))
			{
				return K_OP_FAILED;
			}
			//use sprite
			//m_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_OBJECTSPACE | D3DXSPRITE_DONOTSAVESTATE);

			//Mat matProj;
			//D3DXMatrixOrthoOffCenterLH(&matProj, 0.5f, pRT->nWidth + 0.5f, pRT->nHeight + 0.5f, 0.5f, 0.0f, 1.0f);
			m_pDevice->SetTransform(D3DTS_PROJECTION, &pRT->matProj);

			m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
			m_pDevice->SetTransform(D3DTS_VIEW, &g_matIdentity);

			// special method for rendering lights pass
			// uses the height/normals render target
			RenderPass_Lights(&pRT->matProj, fBetweenFramesPercent);

			// end sprite
			//m_pSprite->End();

			V_OP_RET(UTGetRTManager().EndSceneRT(pRT));

		}
	}

	///----------------------------------------------------
	/// 3. COLOR MAP - overwrites the normal map as we don't need it anymore
	///----------------------------------------------------
	pRT = UTGetRTManager().GetRTbyUID(K_RTID_TEMP1);
	if (pRT != null)
	{
		if(OP_SUCCESS(UTGetRTManager().BeginSceneRT(pRT)))
		{
			// Clear the render target and the zbuffer 
			if (FAILED(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, K_GAME_CLEAR_COLOR, 1.0f, 0)))
			{
				return K_OP_FAILED;
			}
			//use sprite
			//m_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_OBJECTSPACE | D3DXSPRITE_DONOTSAVESTATE);

			//#TODO: este corect ?? offset the projection matrix by 0.5f because in DX the pixel's 0.0 is the center of the pixel
			//Mat matProj;
			//D3DXMatrixOrthoOffCenterLH(&matProj, 0.5f, pRT->nWidth + 0.5f, pRT->nHeight + 0.5f, 0.5f, 0.0f, 1.0f);
			m_pDevice->SetTransform(D3DTS_PROJECTION, &pRT->matProj);

			m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
			m_pDevice->SetTransform(D3DTS_VIEW, &g_matIdentity);

			RenderPass(K_LVL_RP_COLORS, &pRT->matProj, fBetweenFramesPercent);

			// end sprite
			//m_pSprite->End();

			V_OP_RET(UTGetRTManager().EndSceneRT(pRT));
		}
	}

	///----------------------------------------------------
	/// 4. COMPOSITION - composes buffers into one
	///----------------------------------------------------
	pRT = UTGetRTManager().GetRTbyUID(K_RTID_FINAL);
	if (pRT != null)
	{
		if(OP_SUCCESS(UTGetRTManager().BeginSceneRT(pRT)))
		{
			// Clear the render target and the zbuffer 
			if (FAILED(m_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, 0xffff0000, 1.0f, 0)))
			{
				return K_OP_FAILED;
			}
			//use sprite
			m_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_OBJECTSPACE | D3DXSPRITE_DONOTSAVESTATE);

			//#TODO: este corect ?? offset the projection matrix by 0.5f because in DX the pixel's 0.0 is the center of the pixel
			//Mat matProj;
			//D3DXMatrixOrthoOffCenterLH(&matProj, 0.5f, pRT->nWidth + 0.5f, pRT->nHeight + 0.5f, 0.5f, 0.0f, 1.0f);
			m_pDevice->SetTransform(D3DTS_PROJECTION, &pRT->matProj);

			m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
			m_pDevice->SetTransform(D3DTS_VIEW, &g_matIdentity);

			// RT sized quad with tex1 color, tex2 lightmap
			RenderPass_Composition(&pRT->matProj, fBetweenFramesPercent);

			// end sprite
			m_pSprite->End();

			V_OP_RET(UTGetRTManager().EndSceneRT(pRT));

		}
	}


	return K_OP_OK;
}

OPRESULT CLevel::RenderPass(eLVLRenderPass ePass, Mat* matProj, float fBetweenFramesPercent )
{
	_ASSERT((ePass > K_LVL_RP_NONE) && (ePass < K_LVL_RP_COUNT));

	Mat	matView;

	RECTXYWH_F		camrect = m_camLevelToRT.GetCamWorldAABB();
	CAABB			camAABB(camrect);

	//locally used temp matrix
	Mat	matlocal;

	///----------------------------------------------------
	/// INITIAL SETUP
	///----------------------------------------------------
	m_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	if ((UTApp().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0)
	{
		//#IMPORTANT: we need separate alpha blending or it will look bad when blending alpha values between them.
		//eg: if we blend semitransparent things on top of fully opaque walls, the walls become transparent
		//necessary but not really well supported. Better with a shader and custom sprite painter
		m_pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
		m_pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA);
		m_pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_DESTALPHA);
		m_pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
	}

	//#HACK: we floor the camera pos if we get UV seams in DX9. See LoadArea for another hack regarding UV coords and UV seams (UV shrinking)
	// moves from tex pixel to pixel, no half pixels
	MUMatAffine2D(&matView, K_RT_PIXEL_SIZE_F, NULL, 0.0f, &Vec2(-floor(camrect.x) * K_RT_PIXEL_SIZE_F, -floor(camrect.y) * K_RT_PIXEL_SIZE_F));
	m_pDevice->SetTransform(D3DTS_VIEW, &matView);
	m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
	UTGetShaderManager().SetVS(nullptr);

	CTexNode* pTexToUse = m_pTexTilesColor;
	// Offset in texture index so we paint from the normals texture when we render the normals pass
	int nTexIdxOffset = 0;		
	ETexChannel	eTexChannel = K_TEXCHAN_NONE;
	switch (ePass)
	{
		case K_LVL_RP_COLORS:
		{
			pTexToUse = m_pTexTilesColor;
			nTexIdxOffset = 0;
			eTexChannel = K_TEXCHAN_COLORMAP;
		}
		break;
		case K_LVL_RP_NORMALS_HEIGHT:
		{
			pTexToUse = m_pTexTilesNorm;
			nTexIdxOffset = 1;
			eTexChannel = K_TEXCHAN_NORMALMAP;
		}
		break;
		case K_LVL_RP_LIGHTS:
		{
			ErrorBox(K_ERR_WARNING, L"Render lights using RenderPass_Lights() instead!");
		}
		break;
	}

	m_pDevice->SetTexture(0, pTexToUse->pTexture);
	// paint floors and vertical walls

	Areas_PaintLayer(K_AL_FLOOR);
	Areas_PaintLayer(K_AL_WALLS);

	m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);

	/// BEGIN SPRITES PAINTER
	PVERTEXSHADER pSprVS = UTGetShaderManager().GetVShaderByName(L"VS_SPRITES2D");
	if (pSprVS)
		__Painter().Begin(pSprVS, matView * *matProj);

	eVisibleSortableType eLastVis = K_VST_UNKNOWN;
	for (int kk = 0; kk < m_visibleList.arrSortedItems.nCount; kk++)
	{
		CVisibleSortable* vis = &m_visibleList.arrSortedItems.m_pData[kk];

		switch (vis->eType)
		{
			case K_VST_ACTOR:
			{
				if (eLastVis != K_VST_ACTOR)
				{
					// if last painted element was not an actor then do a flush on UTpainter
					__Painter().Flush();
					// remove shaders that were set
					UTGetShaderManager().SetVS(nullptr);
				}
			
				CActor* act = static_cast<CActor*>(vis->pPtr);
				act->Paint(eTexChannel);
			}
			break;
			case K_VST_PROP:
			{
				CProp *prop = static_cast<CProp*>(vis->pPtr);
				prop->sprite.PaintModule_texOverride(0, nTexIdxOffset);
			}
			break;
			default:
				break;
		}

		// save last type of painted item
		eLastVis = vis->eType;
	}

	__Painter().Flush();
	// now paint the bullets
	PaintBullets(ePass);

	/// END SPRITES PAINTER
	__Painter().End();

	// top layer of tiles
	UTGetShaderManager().SetVS(nullptr);
	m_pDevice->SetTexture( 0, m_pTexTilesColor->pTexture );
	Areas_PaintLayer(K_AL_CEILINGS);

	return K_OP_OK;
}

OPRESULT CLevel::RenderPass_Lights(Mat* matProj, float fBetweenFramesPercent )
{
	Mat				matView;

	RECTXYWH_F		camrect = m_camLevelToRT.GetCamWorldAABB();
	CAABB			camAABB(camrect);

	///----------------------------------------------------
	/// INITIAL SETUP
	///----------------------------------------------------
	m_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);

	m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	if ((UTApp().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0)
	{
		//#IMPORTANT: we need separate alpha blending or it will look bad when blending alpha values between them.
		//eg: if we blend semitransparent things on top of fully opaque walls, the walls become transparent
		//necessary but not really well supported. Better with a shader and custom sprite painter
		m_pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
		m_pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA);
		m_pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_DESTALPHA);
		m_pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
	}

	//#HACK: we floor the camera pos if we get UV seams in DX9. See LoadArea for another hack regarding UV coords and UV seams
	MUMatAffine2D(&matView, K_RT_PIXEL_SIZE_F, NULL, 0.0f, &Vec2(-floor(camrect.x) * K_RT_PIXEL_SIZE_F, -floor(camrect.y) * K_RT_PIXEL_SIZE_F));

	m_pDevice->SetTransform(D3DTS_VIEW, &matView);
	m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);


	///--- paint lights ---
	//PVERTEXSHADER pVShader = null;
	//PPIXELSHADER pPShader = null;

	Mat matWVP = matView * (*matProj);
	// begin the painter
	PVERTEXSHADER pSprVS = UTGetShaderManager().GetVShaderByName(L"VS_SPRITES2D");
	if (pSprVS)
		__Painter().Begin(pSprVS, matWVP);


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
		{ floor(camrect.x), floor(camrect.y), camrect.w, camrect.h } //RTT rect_xywh in world coords
		//#HACK: if flooring the campos then floor this camrect too that gets sent to the shader, but floor it to submultiples of pixel size (shader view is real space not screen space)
		//{ floor(camrect.x * K_GAME_PIXEL_SIZE_F) / K_GAME_PIXEL_SIZE_F, floor(camrect.y * K_GAME_PIXEL_SIZE_F) / K_GAME_PIXEL_SIZE_F, camrect.w, camrect.h } //RTT rect_xywh in world coords
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
	//#TODO: should be completely removed....
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


	AdditiveBlendingOFF(m_pDevice, NULL);
	///--- ambient light(s)
	// paint all general ambient lights and area lights here
	//#TODO: paint one ambiental per area!
	UTGetShaderManager().SetVS(nullptr);
	UTGetShaderManager().SetPS(nullptr);

	m_pDevice->SetTexture(0, nullptr);
	m_pDevice->SetTexture(1, nullptr);
	for (int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++)
	{
		CLight *nl = m_visibleList.visible_lights.m_pData[kk];
		if (nl->type != K_LVL_LT_AMBIENTAL)
			continue;
		//paint and exit
		m_bufferedPainter.DrawMesh(nl->m_nLightMeshIdx, true);
	}


	///----------------------------------------------------------------------------------
	/// SHADOWS
	///----------------------------------------------------------------------------------
	scTexture* pShadowsTex = m_sprLights.GetTextureByAnim(ANM_LIGHTS_SPR_SHADOWS, 0, 0);
	if (pShadowsTex)
		m_pDevice->SetTexture(0, pShadowsTex->pTex);
	//#HINT: UpdateVisibility is optional as it was done in the previous colors render pass
	//#TODO: should be called only once on update as it will control the activation of areas
	Areas_PaintLayer(K_AL_WALLSHADOWS);
	
	///----------------------------------------------------------------------------------
	/// LIGHTS
	///----------------------------------------------------------------------------------
	AdditiveBlendingON(m_pDevice, NULL);

	///--- bullet lights
	// bullet shadows
	PaintBullets(K_LVL_RP_LIGHTS);
	__Painter().Flush();
	

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
			{ nl->pos.xyz.x, nl->pos.xyz.y, nl->pos.xyz.z, 0.0f }
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
			{ nl->pos.xyz.x, nl->pos.xyz.y, nl->pos.xyz.z, 0.0f },
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
			{ nl->pos.xyz.x, nl->pos.xyz.y, nl->pos.xyz.z, 0.0f },
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
	__Painter().End();


	return K_OP_OK;
}

OPRESULT CLevel::RenderPass_Composition( Mat* matProj, float fBetweenFramesPercent )
{
	Mat				matView;
	///----------------------------------------------------
	/// INITIAL SETUP
	///----------------------------------------------------
	m_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);

	m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
	m_pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

	m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	if ((UTApp().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0)
	{
		//#IMPORTANT: we need separate alpha blending or it will look bad when blending alpha values between them.
		//eg: if we blend semitransparent things on top of fully opaque walls, the walls become transparent
		//necessary but not really well supported. Better with a shader and custom sprite painter
		m_pDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, true);
		m_pDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA);
		m_pDevice->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_DESTALPHA);
		m_pDevice->SetRenderState(D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD);
	}

	MUMatIdentity(&matView);
	m_pDevice->SetTransform(D3DTS_VIEW, &matView);
	m_pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);

	///--- compose scene from normals and color ---
	PVERTEXSHADER pVShader = null;
	PPIXELSHADER pPShader = null;
	Mat matWVP = matView * (*matProj);

	CRTManager::CEngineRenderTarget* pRTcolor = UTGetRTManager().GetRTbyUID(K_RTID_TEMP1);
	if (pRTcolor != null)
		m_pDevice->SetTexture(0, pRTcolor->m_pRTTexture);
	CRTManager::CEngineRenderTarget* pRTlights = UTGetRTManager().GetRTbyUID(K_RTID_COLORDEPTHSTENCIL);
	if (pRTlights != null)
		m_pDevice->SetTexture(1, pRTlights->m_pRTTexture);

	//--- build RT rect ---
	_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
	vul.pos = Vec3(0.0f, 0.0f, 0.0f);
	vur.pos = Vec3((float)pRTcolor->nWidth, 0.0f, 0.0f);
	vdl.pos = Vec3(0.0f, (float)pRTcolor->nHeight, 0.0f);
	vdr.pos = Vec3((float)pRTcolor->nWidth, (float)pRTcolor->nHeight, 0.0f);

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

	PaintUsingFinalRTT();

	//ingame interface
	//m_interfaceIGM.Paint(m_pDevice, g_pGameSprite);
	//interface particles
	//g_particlesMgr.PaintLayer(K_PART_LAYER_INTERFACE_LIGHT, true);
}

HRESULT CLevel::PaintUsingFinalRTT()
{
	HRESULT hr = S_OK;
	//daca nu e incarcat ies
	if ((!m_bLoaded) || (!m_bOneUpdateDone))
		return E_FAIL;
	//daca nu am capabilitatea de offscreen ies cu eroare
	if ((UTApp().g_gfxFlags & K_UT_GFXFLAG_RTT) == 0)
		return E_FAIL;

	RECTXYWH_F rectRender = UTApp().g_rectRenderPP;
	int nPixelScaling = UTApp().g_nPixelSizePP;
	///--- PAINT LEVEL ---
	//real screen space
	CCameraTransform::SetActiveCamera(m_pDevice, &UTApp().g_camScreen);
	//paint game 
	CRTManager::CEngineRenderTarget* pRTfinal = UTGetRTManager().GetRTbyUID(K_RTID_FINAL);
	if (pRTfinal != null)
	{
		CCameraTransform::SetActiveCameraIdentity(m_pDevice);
		RECT src;
		SIZEWH_F szSrc( rectRender.w / ( float ) nPixelScaling, rectRender.h / ( float ) nPixelScaling );
		// display the center part of the source RT that fits the screen
		Vec2i vUL( ( int ) floor( pRTfinal->nWidth / 2.0f - szSrc.w / 2.0f ), ( int ) floor( pRTfinal->nHeight / 2.0f - szSrc.h / 2.0f ) );
		Vec2i vDR( vUL.x + ( int ) ceil(szSrc.w), vUL.y + ( int ) ceil(szSrc.h) );
		SetRect( &src, vUL.x, vUL.y, vDR.x, vDR.y );
		//use SRC rect for scaling and not the nPixelScaling.
		float fRTscale = nPixelScaling;
		//#TODO: when using NON PIXEL PERFECT scaling just scale the whole RT (keeping the aspect ratio)
		//float fRTscale = (float)rectRender.h / (float)pRTfinal->nHeight;
		Mat matpaint;
		// computes sub pixel offsets for smooth scrolling. the RT renders only on tileset pixels, no subpixels, for precision.
		// we remove the clunky camera movement by moving the final RT onscreen with subpixel coordinates
		RECTXYWH_F camrect = g_level.m_camLevelToRT.GetCamWorldAABB();
		Vec2 vSubPxOff(-FLOAT_FRAC(camrect.x) * (fRTscale * K_RT_PIXEL_SIZE_F), -FLOAT_FRAC(camrect.y) * (fRTscale * K_RT_PIXEL_SIZE_F));
		MUMatAffine2D(&matpaint, fRTscale, nullptr, 0.0f, &Vec2(rectRender.x + vSubPxOff.x, rectRender.y + vSubPxOff.y));
		m_pSprite->SetTransform(&matpaint);
		m_pSprite->Draw(pRTfinal->m_pRTTexture, &src, NULL, &g_Vec3Zero, 0xffffffff);
		m_pSprite->Flush();
		m_pSprite->SetTransform(&g_matIdentity);
	}


	m_pSprite->SetTransform(&g_matIdentity);
	CCameraTransform::SetActiveCamera(m_pDevice, &m_camLevelToScr);
	//get camera data
	RECTXYWH_F	camrect = m_camLevelToScr.GetCamWorldAABB();
	Mat			matCam = m_camLevelToScr.GetViewTransform();
	CAABB		camAABB(camrect);

	///--- paint water ---
	/*
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
	*/

	///#TEMP: paint interactible
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if (pPlayerActor[kk] == null)
			continue;
		CActor* pPlayer = pPlayerActor[kk];
		if (pPlayer->pClosestTouchable != nullptr)
		{
			Vec2 vpos = pPlayer->pClosestTouchable->pos.xy_proj;
			CSprite::paintFrame(&m_sprInterface, vpos.x, vpos.y, ANM_IGM_INTERFACE_SPR_INTERACT_ONCE, 0, 0xffffffff);
		}
	}


	///--- paint crosshairs 
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		if (pPlayerActor[kk] == null)
			continue;

		// paint aiming cursor
		// vAimVec was normalized using last frame data so paint it at last frame actor position
		Vec2 vto = Vec3XY(pPlayerActor[kk]->pos_last) + pPlayerActor[kk]->m_AIcommands.vAimVec;
		CSprite::paintFrame(&m_sprInterface, vto.x, vto.y, ANM_IGM_INTERFACE_SPR_IGM_STRATEGIC_EFFECTS, 4, 0xffffffff);
	}
								  
	///--- actors icons and stun stars ---
	/*
	for (int kk = 0; kk < m_visibleList.visible_actors.Count(); kk++)
	{
		CActor* act = m_visibleList.visible_actors.m_pData[kk];
		//shield/overhead icons for non players
		if ((act->actTemplate.actorClass != K_LVL_ACT_CLASS_PLAYER) && (act->m_sprOverheadIcon.animationIdx >= 0))
		{
			act->m_sprOverheadIcon.pos = act->GetPosHeart();
			act->m_sprOverheadIcon.paint(&m_sprInterface);
		}
		//overhead icon !!! only if no overhead icon set (hence the else)
		else if (act->nIconType != K_LVL_ACT_ICON_NONE)
		{
			CSprite::paintFrame(&m_sprInterface, act->GetPosHeart().x, act->GetPosHeart().y, ANM_IGM_INTERFACE_SPR_ACTOR_ICONS, act->nIconType);
		}

		//STUN STARS
		if (act->fStunTimer >= K_LVL_MIN_STUN_DIZZY_DURATION)
		{
			int curframe = int(fLocalTimeline * 25.0f) % g_particlesMgr.m_sprCol.GetAFramesCnt(ANM_PARTICLES_SPR_STUN_STARS);
			Vec2 vStarsPos = act->GetPosHeart();
			CSprite::paintFrameModule(&g_particlesMgr.m_sprCol, vStarsPos.x, vStarsPos.y - 10.0f, ANM_PARTICLES_SPR_STUN_STARS, curframe, 0, act->color);
		}

		//energy bars
		if ((act->actTemplate.actorClass == K_LVL_ACT_CLASS_HUMAN) && (act->fLife > 0.0f) &&
			(act->actTemplate.fLife > 100.0f) && (act->m_AIsensorInfo.fTimeSinceHit < 5.0f))
		{
			float fLife = act->fLife / act->actTemplate.fLife;
			float fBarLen = act->actTemplate.fLife / 5.0f;
			CLAMP(fBarLen, 40.0f, 60.0f);

			RECTXYWH barrect(act->bbox.vCenter.x - fBarLen / 2.0f, act->bbox.vMax.y + 3.0f, fBarLen, 8.0f);
			CtrlMgrDrawProgress_HeadsOutside(&m_sprInterface, ANM_IGM_INTERFACE_SPR_PROGRESS_HEALTH, barrect, fLife, 0xffffffff);
		}
	}
	*/

	//-- final flush for level space ---
	m_pSprite->Flush();

	///--- paint Fog Of War ---
	/*
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
	*/


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

			Vec2 vpos(activ->bbox.vCenter.x, activ->bbox.vMin.y);
			//too low? don't cover the player
			if (vpos.y > player->bbox.vMin.y)
				vpos.y = player->bbox.vMin.y;

			//change this constants for analog sticks
			const int ANIM_IDX_INTERACT_ONCE = ANM_IGM_INTERFACE_SPR_INTERACT_ONCE;
			const int ANIM_IDX_INTERACT_KEEP_PRESSED = ANM_IGM_INTERFACE_SPR_INTERACT_KEEP_PRESSED;
			/*
			#ifdef (CONSOLE)
			const int ANIM_IDX_INTERACT_ONCE = ANM_IGM_INTERFACE_SPR_INTERACT_ONCE_ANALOG;
			const int ANIM_IDX_INTERACT_KEEP_PRESSED = ANM_IGM_INTERFACE_SPR_INTERACT_KEEP_PRESSED_ANALOG;
			#endif
			*/


			//player->m_sprOverheadIcon.setAnimationOnce(ANIM_IDX_INTERACT_ONCE);
			//player->m_sprOverheadIcon.pos = vpos;

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
					RECTXYWH recttemp(activ->pTarget->pos.xy_proj.x - barlen / 2.0f, activ->pTarget->bbox.vMax.y + 4, barlen, 8);
					CtrlMgrDrawProgress_HeadsOutside(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_PROGRESS_RED_GLOW_HO, recttemp, perc, 0xffffffff);
				}
			}
			//paint interact icon at the end
			//player->m_sprOverheadIcon.paint(&m_sprInterface);
		}

		//cover shield
		if (UTApp().m_Settings.bShowInterfaceHelp) //player numeric icon (only if shield not visible)
		{
			Vec2 vpos = Vec2(pPlayerActor[kk]->bbox.vCenter.x, pPlayerActor[kk]->bbox.vMin.y + fabs(3.0f * sin(fLocalTimeline * 4.0f)));
			CSprite::paintFrame(&m_sprInterface, vpos.x, vpos.y, ANM_IGM_INTERFACE_SPR_PLAYER_NR_ICONS, pPlayerActor[kk]->nPlayerOrdinal);
		}

		//paint player numeric icon on multiplayer when peer outside the screen
		if (UTApp().IsGameNetworked())
		{

			if ((pPlayerActor[kk]->nPlayerOrdinal == g_netlock.Net_GetOtherPlayerIndex()) && (!camAABB.Intersects(pPlayerActor[kk]->bbox)))
			{
				Vec2 vpos = Vec2(pPlayerActor[kk]->bbox.vCenter.x, pPlayerActor[kk]->bbox.vMin.y);
				CAABB localAABB = camAABB;
				localAABB.Inflate(-K_TILE_SIZE + fabs(3.0f * sin(fLocalTimeline * 4.0f)), -K_TILE_SIZE + fabs(3.0f * sin(fLocalTimeline * 4.0f)));
				if (AABB::Segment_Intersection(vpos, camAABB.vCenter, localAABB, &vpos))
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
	//m_interfaceTextBubble.Paint(m_pDevice, m_pSprite);

	//set screen space
	CCameraTransform::SetActiveCamera(m_pDevice, &UTApp().g_camScreen);

	///--- paint time slowdown screen effect ---
	m_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	if (m_fTimeMultiplier_real < 1.0f)
	{
		DWORD colEffect = DW_COLORALPHA(0xff000088, 1.0f - m_fTimeMultiplier_real);
		Mat mattrans;
		RECTXYWH_F bbox = UTGetGUI().m_sprCol.GetAFrameBBox_real(ANM_CONTROLS_SPR_VIGNETTES, 1);
		MUMatAffine2D(&mattrans, rectRender.h / bbox.h, NULL, 0.0f, &rectRender.Center());
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

	SAFE_DELETE_GROWABLE_ARRAY(m_arrAreas);

	SAFE_DELETE_GROWABLE_ARRAY(m_arrColShapes);
	SAFE_DELETE_GROWABLE_ARRAY(m_arrLights);
	SAFE_DELETE_GROWABLE_ARRAY(m_arrDecals);
	SAFE_DELETE_GROWABLE_ARRAY(m_arrActors);
	SAFE_DELETE_GROWABLE_ARRAY(m_arrMiscObjects);

	SAFE_DELETE_GROWABLE_ARRAY(m_arrTemplatesActor);
	SAFE_DELETE_GROWABLE_ARRAY(m_arrAItemplates);
	SAFE_DELETE_GROWABLE_ARRAY(m_arrTemplatesWeapon);
	SAFE_DELETE_GROWABLE_ARRAY(m_arrTemplatesExplosion);

	SAFE_DELETE_GROWABLE_ARRAY(m_arrAIevents);
	m_arrActionTemplates.clear();
	//release bullets
	SAFE_DELETE_GROWABLE_ARRAY(m_arrBullets);
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
	//m_interfaceTextBubble.Release();

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
	Vec2 vFrom = light->pos.xy;
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
		CTile* pColTile = SegmentTilesIntersection(vFrom, vTo, vRetPt, vRetNrm, &tilePosTL);
		if (pColTile)
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

	//m_interfaceIGM.SetStrategicPoints(0.0f, 0.0f);
	//m_interfaceIGM.SetLivesLeft(m_arrStats[K_LVL_STATS_PL1_LIVES], m_arrStats[K_LVL_STATS_PL2_LIVES]);
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

	//m_interfaceIGM.SetStrategicPoints(m_arrStats[K_LVL_STATS_PL1_STRATEGIC_POINTS] / 1000.0f, m_arrStats[K_LVL_STATS_PL2_STRATEGIC_POINTS] / 1000.0f);

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
	CDoubleLinkedPool<CPhysicsPoint>::CLinkedPoolNode *node = m_poolPhysPts.pListUsed.m_pNext;
	while (node != &m_poolPhysPts.pListUsed)
	{
		CDoubleLinkedPool<CPhysicsPoint>::CLinkedPoolNode *nextnode = node->m_pNext;
		//update
		CPhysicsPoint*	point = &node->m_data;
		// kill it when it gets outside the play area
		if (!PointInRect(Vec3XY(point->pos), m_levelAABB))
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
			Vec2 vFrom = Vec3XY(point->pos_last);
			Vec2 vTo = Vec3XY(point->pos);
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
					Vec2i ptHitTilePos(0, 0);
					CTile* pColTile = SegmentTilesIntersectionEx(vFrom, vTo, collisionPoint, collisionNormal, &ptHitTilePos, point->pArea);
					if (pColTile)
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

			//update current area (change only if not static)
			Vec2 vpos2d = Vec3XY(point->pos);
			if ((point->pArea == nullptr) || (!point->pArea->AABBbounds.PointIn(vpos2d)))
			{
				point->pArea = Areas_GetAt(vpos2d);
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
			m_camLevelToRT.ShakeScreen(2.0f, 8.0f, &pos);

//			SND_PLAY_POSITIONAL(SNDIDX_STONE_BREAK1, pos);
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

void CLevel::GC()
{

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

	// array to hold the tiles snapshots (linear matrix). Permits a max size of 32x32 tiles.
	CTile* arrTilesSnapshot[32 * 32];
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
		if (AABB::Intersection(bbox, m_visibleList.visible_colShapesLights.m_pData[kk]->bbox, retbb))
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
	// limit to current level aabb in tiles
	RECTXYWH lightAABB_TL(tlmin.x, tlmin.y, tlmax.x - tlmin.x + 1, tlmax.y - tlmin.y + 1);
	lightAABB_TL.IntersectWith(m_levelAABB_TL);
	// tiles are returned in the arrTilesetSnapshot as a matrix in linear form, 0 base index (vector[xx + yy * lightAABB_TL.w])
	Areas_GetTilesSnapshot(lightAABB_TL, arrTilesSnapshot, ARRAY_SIZE(arrTilesSnapshot));

	for (int yy = tlmin.y; yy <= tlmax.y; yy++)
	{
		for (int xx = tlmin.x; xx <= tlmax.x; xx++)
		{
			_ASSERT(nCur < maxRetArrSize - 2);

			CTile* tl = arrTilesSnapshot[xx - tlmin.x + (yy - tlmin.y) * lightAABB_TL.w];
			if(tl == nullptr)
				continue;

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
					if ((nCur > 0) && ((int)pRetArr[nCur - 1].dwWallID == yy) && (pRetArr[nCur - 1].vEnd.x == chkbb.vMin.x))
						pRetArr[nCur - 1].MoveEnd(chkbb.vMax, vPos);
					else
						/*ID is wall Y in tileset plus a value to not collide with the collbox ids */
						pRetArr[nCur++].Set(Vec2(chkbb.vMin.x, chkbb.vMax.y), chkbb.vMax, vNYp, vPos, yy, K_WALL_HEIGHT_SCREEN);
				}
				else if ((tl->flags & K_TILEFLAG_HASWALL_U) && (vEye.y < chkbb.vMin.y))
				{
					//optimize same wall: check last wall and if it's the same just make the occluder longer
					if ((nCur > 0) && ((int)pRetArr[nCur - 1].dwWallID == yy) && (pRetArr[nCur - 1].vStart.x == chkbb.vMin.x))
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



///--- framework implementations ---
#pragma region FRAMEWORK_IMPL
OPRESULT CLevel::OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc, void* pUserContext)
{
	m_pDevice = pDevice;

	V_OP_RET(m_sprLights.OnCreateDevice(pDevice));
	V_OP_RET(m_sprProps.OnCreateDevice(pDevice));
	V_OP_RET(m_sprActors.OnCreateDevice(pDevice));
	V_OP_RET(m_sprInterface.OnCreateDevice(pDevice));
	V_OP_HRTOOP(m_texManager.OnCreateDevice(pDevice));
	V_OP_RET(m_bufferedPainter.OnCreateDevice(pDevice));

	for (int ii = 0; ii < m_arrAreas.GetSize(); ii++)
	{
		CLevelArea* area = m_arrAreas[ii];
		V_OP_RET(area->OnCreateDevice(pDevice));
	}

	return K_OP_OK;
}

OPRESULT CLevel::OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc, void* pUserContext)
{
	m_pDevice = pDevice;

	V_OP_RET(m_sprLights.OnResetDevice(pDevice));
	V_OP_RET(m_sprProps.OnResetDevice(pDevice));
	V_OP_RET(m_sprActors.OnResetDevice(pDevice));
	V_OP_RET(m_sprInterface.OnResetDevice(pDevice));
	V_OP_HRTOOP(m_texManager.OnResetDevice(pDevice));
	V_OP_RET(m_bufferedPainter.OnResetDevice(pDevice));

	for (int ii = 0; ii < m_arrAreas.GetSize(); ii++)
	{
		CLevelArea* area = m_arrAreas[ii];
		V_OP_RET(area->OnResetDevice(pDevice));
	}

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

	for (int ii = 0; ii < m_arrAreas.GetSize(); ii++)
	{
		CLevelArea* area = m_arrAreas[ii];
		V_OP_RET(area->OnLostDevice());
	}

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

	for (int ii = 0; ii < m_arrAreas.GetSize(); ii++)
	{
		CLevelArea* area = m_arrAreas[ii];
		V_OP_RET(area->OnDestroyDevice());
	}

	return K_OP_OK;
}

#pragma endregion FRAMEWORK_IMPL


#pragma warning(pop)
