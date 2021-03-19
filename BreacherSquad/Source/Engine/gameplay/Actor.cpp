#include "dxstdafx.h"
#include "Actor.h"

Vec2 CActor::GetPosHeart()
{
	return pos;
}

Vec2 CActor::GetPosWeapon()
{
	return Vec2(pos.x, pos.y);
}

void CActor::UpdateBBoxAndPoints()
{
	posWeapon = GetPosWeapon();
	posHeart = GetPosHeart();

	int bboxidx = 0;
	//trece pe bbox de dead doar daca a terminat animatia de dead
	if (fLife <= 0.0f)
	{
		//comenteaza linia de mai jos ca sa ia bbox doar cand a terminat animatia de moarte
		//if(((eLastAnimSet == K_LVL_ACT_ANIM_DIE) || (eLastAnimSet == K_LVL_ACT_ANIM_DIE_ALT)) && (sprite.animStatus == ANIM_STATUS_FRAMELOCK))
		bboxidx = 2;
	}
	else if (bCrouched)
		bboxidx = 1;

	bbox_ini.Set(stateBBoxes[bboxidx]);
	bbox = bbox_ini;
	bbox.Move(pos);

	bbox_exported_ini.Set(stateBBoxes[0]); //standing
	bbox_exported = bbox_exported_ini;
	bbox_exported.Move(pos);
}

void CActor::SetIcon(EActorIconTypes iconType, float fDuration)
{
	if (iconType == K_LVL_ACT_ICON_REMOVE_ICON)
	{
		fIconTimer = 0.0f;
		nIconType = K_LVL_ACT_ICON_NONE;
		return;
	}

	nIconType = iconType;
	fIconTimer = fDuration;

	if (fDuration <= 0.0f)
		nIconType = K_LVL_ACT_ICON_NONE;
}

bool CActor::Spine_SetSkin(const char * strSkinName)
{
	if ((pSkeleton == null) || (pSkeleton->skel == null) || (pSkelTemplate == null) || (strSkinName[0] == 0))
	{
		ErrorBox(K_ERR_WARNING, L"CActor::Spine_SetSkin: Empty skin name or skeleton data not loaded!");
		return false;
	}

	spine::Skin* pSkin = pSkelTemplate->m_skeletonData->findSkin(strSkinName);
	if (pSkin == null)
	{
		ErrorBox(K_ERR_WARNING, L"CActor::Spine_SetSkin: Skin not found!");
		return false;
	}

	pSkeleton->skel->setSkin(pSkin);
	LOG_DBG("Spine_SetSkin: setting skin: %s", strSkinName);

	return true;
}

bool CActor::HasAnimation(ESpineAnim nAnimType, int nSet)
{
	if ((nSet < 0) || (nSet >= K_ACT_ANIM_MAX_SETS))
		return false;

	bool bHasIt = (this->arrAnimsPtr[nAnimType].pAnim[nSet] != null);
	return bHasIt;
}

void CActor::SetAnimSet(int newAnimSet)
{
	if (newAnimSet != nAnimSet)
	{
		nAnimSet = newAnimSet;
	}
}

void CActor::SetAngle(float fNewAngle)
{
	fAngle = fNewAngle;
	vAngleDir = Vec2(cos(fAngle), sin(fAngle));
}

void CActor::PostConstructionInit()
{

}

void CActor::BeginPlay()
{

}

void CActor::EndPlay()
{

}

EAIBehaviorType CActor::GetCurrentBehavior()
{
	if ((m_nAIcurrentBehaviorIdx < 0) || (m_pAIcurrentState == null))
		return AI_BEHAVIOR_EMPTY;
	return m_pAIcurrentState->m_arrBehaviors[m_nAIcurrentBehaviorIdx].nType;
}

void CActor::SetPos(Vec2 newPos)
{
	pos_last = pos;
	pos = newPos;

	UpdateBBoxAndPoints();
}

void CActor::Move(Vec2 delta)
{
	pos += delta;

	UpdateBBoxAndPoints();
}


CActorTemplate::CActorTemplate() :
	//generic params
	fLife(K_NOT_SET), fArmor(K_NOT_SET), fSpeedMove(K_NOT_SET),
	actorClass(K_LVL_ACT_CLASS_NOT_SET),
	//more important values
	eMaterial(K_LVL_MATERIAL_UNKNOWN), eCaps(K_ACT_CAPS_NONE),
	AItemplate(nullptr), fMass(100.0f)
{
	//reset anim IDs
	for (int kk = 0; kk < K_SD_ANIMS_CNT; kk++)
	{
		arrAnims[kk].Reset();
	}
	//reset verses
	for (int kk = 0; kk < K_LVL_ACT_VERSES_COUNT; kk++)
	{
		for (int jj = 0; jj < K_ACT_VERSES_MAX_SETS; jj++)
		{
			soundIDs[kk][jj] = -1; //default value for missing verse
		}
	}
}

void CActorTemplate::FillDefaultValuesIfNotSet()
{
	if (eMaterial == K_LVL_MATERIAL_UNKNOWN) { eMaterial = K_LVL_MATERIAL_FLESH; }

	if (fLife == K_NOT_SET) { fLife = 100.0f; }
	if (fArmor == K_NOT_SET) { fArmor = 0.0f; }
	if (fSpeedMove == K_NOT_SET) { fSpeedMove = 64.0f; }
}

bool CActorTemplate::OverwriteAnimsFromTemplate(CActorTemplate* pTemplate, bool bEraseOldAnimations /*= false*/)
{
	if (pTemplate == null)
	{
		return false;
	}

	bool bChanged = false;
	for (int kk = 0; kk < K_SD_ANIMS_CNT; kk++)
	{
		if (bEraseOldAnimations)
		{
			arrAnims[kk].Reset();
			bChanged = true;
		}

		for (int jj = 0; jj < K_ACT_ANIM_MAX_SETS; jj++)
		{
			//overwrite if existing
			if (pTemplate->arrAnims[kk].animNamesA[jj].IsSet())
			{
				arrAnims[kk].animNamesA[jj] = pTemplate->arrAnims[kk].animNamesA[jj];
				arrAnims[kk].bLooping = pTemplate->arrAnims[kk].bLooping;
				bChanged = true;
			}
		}
	}

	return bChanged;
}

void CActorTemplate::OverwriteGenericDataFromTemplate(CActorTemplate* pTemplate)
{
	if (pTemplate == null)
	{
		return;
	}
	//#TEMP: cand am templates aproape gata
	/*
	if (pTemplate->actorClass != K_LVL_ACT_CLASS_NOT_SET) { actorClass = pTemplate->actorClass; }
	if (pTemplate->foeClassFilter1 != K_LVL_ACT_CLASS_ANY) { foeClassFilter1 = pTemplate->foeClassFilter1; }
	if (pTemplate->foeClassFilter2 != K_LVL_ACT_CLASS_ANY) { foeClassFilter2 = pTemplate->foeClassFilter2; }
	if (pTemplate->eMaterial != K_LVL_MATERIAL_UNKNOWN) { eMaterial = pTemplate->eMaterial; }
	//add new caps (should XOR if need to disable them)
	if(pTemplate->eCaps != 0) eCaps = pTemplate->eCaps;

	if (pTemplate->nHUDportraitFrameIdx != K_NOT_SET) { nHUDportraitFrameIdx = pTemplate->nHUDportraitFrameIdx; }
	if (pTemplate->fLife != K_NOT_SET) { fLife = pTemplate->fLife; }
	if (pTemplate->fArmor != K_NOT_SET) { fArmor = pTemplate->fArmor; }
	if (pTemplate->nArmorDir != K_NOT_SET) { nArmorDir = pTemplate->nArmorDir; }
	if (pTemplate->nArmorRating != K_NOT_SET) { nArmorRating = pTemplate->nArmorRating; }
	if (pTemplate->jumpSpeed != K_NOT_SET) { jumpSpeed = pTemplate->jumpSpeed; }
	if (pTemplate->moveMaxSpeed != K_NOT_SET) { moveMaxSpeed = pTemplate->moveMaxSpeed; }
	if (pTemplate->moveMinSpeed != K_NOT_SET) { moveMinSpeed = pTemplate->moveMinSpeed; }
	if (pTemplate->moveBackSpeed != K_NOT_SET) { moveBackSpeed = pTemplate->moveBackSpeed; }
	if (pTemplate->fDexterity != K_NOT_SET) { fDexterity = pTemplate->fDexterity; }
	if (pTemplate->fRecoilModifier != K_NOT_SET) { fRecoilModifier = pTemplate->fRecoilModifier; }
	if (pTemplate->climbSpeed != K_NOT_SET) { climbSpeed = pTemplate->climbSpeed; }
	if (pTemplate->distSee != K_NOT_SET) { distSee = pTemplate->distSee; }
	if (pTemplate->distHear != K_NOT_SET) { distHear = pTemplate->distHear; }
	if (pTemplate->distAttackMax != K_NOT_SET) { distAttackMax = pTemplate->distAttackMax; }
	if (pTemplate->distAttackMin != K_NOT_SET) { distAttackMin = pTemplate->distAttackMin; }
	if (pTemplate->fMass != K_NOT_SET) { fMass = pTemplate->fMass; }
	if (pTemplate->fFOVpercent != K_NOT_SET) { fFOVpercent = pTemplate->fFOVpercent; }
	if (pTemplate->fArmorMPP != K_NOT_SET) { fArmorMPP = pTemplate->fArmorMPP; }
	//scripts
	if (!pTemplate->shScript_OnSpawn.IsEmpty()) { shScript_OnSpawn = pTemplate->shScript_OnSpawn; }
	*/
}

void CActorTemplate::AddGenericDataFromTemplate(CActorTemplate* pTemplate)
{
	if (pTemplate == null)
	{
		return;
	}

	//#TEMP: cand am templates aproape gata
	/*
	if (pTemplate->actorClass != K_LVL_ACT_CLASS_NOT_SET) { actorClass = pTemplate->actorClass; }
	if (pTemplate->foeClassFilter1 != K_LVL_ACT_CLASS_ANY) { foeClassFilter1 = pTemplate->foeClassFilter1; }
	if (pTemplate->foeClassFilter2 != K_LVL_ACT_CLASS_ANY) { foeClassFilter2 = pTemplate->foeClassFilter2; }
	if (pTemplate->eMaterial != K_LVL_MATERIAL_UNKNOWN) { eMaterial = pTemplate->eMaterial; }
	//XOR in new caps:
	eCaps ^= pTemplate->eCaps;

	if (pTemplate->nHUDportraitFrameIdx != K_NOT_SET) { nHUDportraitFrameIdx = pTemplate->nHUDportraitFrameIdx; }
	if (pTemplate->fLife != K_NOT_SET) { fLife += pTemplate->fLife; }
	if (pTemplate->fArmor != K_NOT_SET) { fArmor += pTemplate->fArmor; }
	if (pTemplate->nArmorDir != K_NOT_SET) { nArmorDir = pTemplate->nArmorDir; }  //exception: armor dir can't be added so it gets written over
	if (pTemplate->nArmorRating != K_NOT_SET) { nArmorRating += pTemplate->nArmorRating; }
	if (pTemplate->jumpSpeed != K_NOT_SET) { jumpSpeed += pTemplate->jumpSpeed; }
	if (pTemplate->moveMaxSpeed != K_NOT_SET) { moveMaxSpeed += pTemplate->moveMaxSpeed; }
	if (pTemplate->moveMinSpeed != K_NOT_SET) { moveMinSpeed += pTemplate->moveMinSpeed; }
	if (pTemplate->moveBackSpeed != K_NOT_SET) { moveBackSpeed += pTemplate->moveBackSpeed; }
	if (pTemplate->fDexterity != K_NOT_SET) { fDexterity += pTemplate->fDexterity; }
	if (pTemplate->fRecoilModifier != K_NOT_SET) { fRecoilModifier += pTemplate->fRecoilModifier; }
	if (pTemplate->climbSpeed != K_NOT_SET) { climbSpeed += pTemplate->climbSpeed; }
	if (pTemplate->distSee != K_NOT_SET) { distSee += pTemplate->distSee; }
	if (pTemplate->distHear != K_NOT_SET) { distHear += pTemplate->distHear; }
	if (pTemplate->distAttackMax != K_NOT_SET) { distAttackMax += pTemplate->distAttackMax; }
	if (pTemplate->distAttackMin != K_NOT_SET) { distAttackMin += pTemplate->distAttackMin; }
	if (pTemplate->fMass != K_NOT_SET) { fMass += pTemplate->fMass; }
	if (pTemplate->fFOVpercent != K_NOT_SET) { fFOVpercent += pTemplate->fFOVpercent; }
	if (pTemplate->fArmorMPP != K_NOT_SET) { fArmorMPP += pTemplate->fArmorMPP; }
	//weapon scripts
	if (!pTemplate->shScript_OnSpawn.IsEmpty()) { shScript_OnSpawn = pTemplate->shScript_OnSpawn; }

	//normalize some values
	if (fLife < 0.0f) fLife = 0.0f;
	if (fArmor < 0.0f) fArmor = 0.0f;
	CLAMP(fArmorMPP, 0.0f, 1.0f);
	if (nArmorRating < 0) nArmorRating = 0;
	if (fDexterity < 0.0f) fDexterity = 0.0f;
	if (fRecoilModifier < 0.0f) fRecoilModifier = 0.0f;
	if (fMass < 0.1f) fMass = 0.1f;
	if (moveMinSpeed < 0.0f) moveMinSpeed = 0.0f;
	if (moveMaxSpeed < 0.0f) moveMaxSpeed = 0.0f;
	*/
}


bool CActor::Init(CActorTemplate * pActorTemplate, Vec2 vSpawnPos)
{
	if (pActorTemplate == NULL)
	{
		ErrorBox(K_ERR_CRITICAL, L"Actor template is null for ID:%d!", this->ID);
		return false;
	}
	//copy template data
	this->actTemplate = *pActorTemplate;

	this->actTemplate.FillDefaultValuesIfNotSet();
	//save a copy
	this->actTemplate_ini = this->actTemplate;

	for (int kk = 0; kk < K_ACT_MAX_ANIM_TRACKS; kk++)
	{
		this->eLastAnim[kk] = K_SD_ANIM_EMPTY;
	}
	this->eLastPlayedVerse = K_LVL_ACT_VERSE_EMPTY;

	this->bOnLadder = false;
	this->bCrouched = false;
	this->nLastDamageTakenFromUID = 0;
	this->nAnimSet = 0;

	this->SetAngle(0.0f);

	this->bSkipRender = false;

	//set hue
	byte collvl = 255;
	this->color_ini = D3DCOLOR_ARGB(255, collvl, collvl, collvl);
	this->color = this->color_ini;

	this->fLife = this->actTemplate.fLife;

	if (!this->actTemplate.shWeaponDefault.IsEmpty())
	{
		//weapons[0].Init(this->actTemplate.shWeaponDefault.text, this);
		weapons[0].Init();
	}

	/*
	Weapon_Init(&this->weapons[K_LVL_ACT_WEAPON_SECONDARY], this->templateActor.weaponTypeAlt.text, actor);
	Weapon_Init(&this->weapons[K_LVL_ACT_WEAPON_GEAR], this->templateActor.weaponTypeGear.text, actor);
	Weapon_Init(&this->weapons[K_LVL_ACT_WEAPON_MELEE], this->templateActor.weaponTypeMelee.text, actor);
	//set breach weapon
	Weapon_Init(&this->weapons[K_LVL_ACT_WEAPON_BREACH], this->templateActor.weaponTypeBreach.text, actor);
	//clear temp weapons
	this->weapons[K_LVL_ACT_WEAPON_TEMPORARY].Init();
	this->weapons[K_LVL_ACT_WEAPON_TEMPORARY_ALT].Init();
	//clear no weapon weapon
	this->weapons[K_LVL_ACT_WEAPON_NO_WEAPON].Init();

	this->pCurrentWeapon = &this->weapons[K_LVL_ACT_WEAPON_PRIMARY];
	//set selected weapons
	this->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY] = &this->weapons[K_LVL_ACT_WEAPON_PRIMARY];
	this->pSelectedWeapon[K_LVL_ACT_WEAPON_SECONDARY] = &this->weapons[K_LVL_ACT_WEAPON_SECONDARY];
	this->pSelectedWeapon[K_LVL_ACT_WEAPON_GEAR] = &this->weapons[K_LVL_ACT_WEAPON_GEAR];
	this->pSelectedWeapon[K_LVL_ACT_WEAPON_MELEE] = &this->weapons[K_LVL_ACT_WEAPON_MELEE];
	this->pSelectedWeapon[K_LVL_ACT_WEAPON_BREACH] = &this->weapons[K_LVL_ACT_WEAPON_BREACH];
	//others are null:
	this->pSelectedWeapon[K_LVL_ACT_WEAPON_TEMPORARY] = null;
	this->pSelectedWeapon[K_LVL_ACT_WEAPON_TEMPORARY_ALT] = null;
	this->pSelectedWeapon[K_LVL_ACT_WEAPON_NO_WEAPON] = null;
	*/
	this->AddWpnTemplate(&this->weapons[0]);

	this->pCurrentWeapon = null;

	//set position
	this->pos = vSpawnPos;

	// selected animset
	this->SetAnimSet(0);

	//init bbox too
	this->bbox_ini.Set(Vec2(-6.0f, -6.0f), Vec2(6.0f, 6.0f));
	this->bbox = this->bbox_ini;
	this->bbox_exported_ini = this->bbox_ini;
	this->bbox_exported = this->bbox_exported_ini;

	//update all relative data
	this->SetPos(this->pos);

	return true;
}

void CActor::Update(float dTime)
{
	//change visibility
	this->bHidden = this->bSetHidden;
	// actor is hidden or not active so ignore it
	if (this->bHidden)
		return;

	//update timeline
	this->fTimelineAI += dTime;

	/*
	this->SetAnimOnce(1, K_SD_ANIM_EMPTY);
	if (this->bCrouched)
		this->SetAnimOnce(0, K_SD_ANIM_IDLE_CROUCH);
	else
		this->SetAnimOnce(0, K_SD_ANIM_IDLE);
	*/

	///--- update weapons ---
	/*
	//aiming hand error
	if (this->pCurrentWeapon != null)
	{
		if (this->pCurrentWeapon->eStatus != K_LVL_WPN_STATUS_SHOOTING)
		{
			this->fAimTimer += dTime;
		}

		D3DXVECTOR2 vAimFinal = this->GetAimDir();
		this->pCurrentWeapon->SetAimDir(vAimFinal);
	}
	//actual shooting
	for (int kk = 0; kk < K_LVL_ACT_WEAPONS_CNT; kk++)
	{
		this->weapons[kk].Update(dTime);
	}
	*/

	/*
	switch (this->AIstate)
	{
		case K_AI_STATE_ACTOR_DEAD:
		{
			this->SetAnimOnce(0, K_SD_ANIM_DIE);
			this->SetAnimOnce(1, K_SD_ANIM_EMPTY);
			//busy dying
			//#TODO: see when anim or sym ends
			//if (this->sprite.animStatus != ANIM_STATUS_FRAMELOCK)
			//this->bIsBusy = true;
		}
		break;
		case K_AI_STATE_ACTOR_ACTIVE:
		{
			if ((!this->bIsBusy) && (this->bCrouched) && (this->eLastAnim[0] != K_SD_ANIM_IDLE_CROUCH))
				this->SetAnimOnce(0, K_SD_ANIM_IDLE_CROUCH);

			// aiming IK node must be set each frame or they get reset by the animation
			if (bIsAiming)
			{
				//#TODO: ar trebui sa setez osul mereu ca sa fie bine setat si pe tranzitii intre animatii
				// vezi transformul asta ca sa muti din world space in skeleton space:
				//Vector2 ledgePointLocalSpace = skeletonAnimation.transform.InverseTransformPoint(ledgePoint); // your ledgePoint
				// find aim bone and move it
				if (pSkeleton->arrBones[K_SD_BONE_AIM_IK] != null)
				{
					spine::Bone* b_aim = pSkeleton->arrBones[K_SD_BONE_AIM_IK];

					D3DXVECTOR2 vAim = this->GetAimDir();
					b_aim->setX(SIGN(this->vLookDir.x) * vAim.x * -100.0f);
					b_aim->setY(-vAim.y * 100.0f);
				}
			}

		}
		break;
	}
	*/

	///--- final update step ---
	//set skeleton data
	if ((this->pSkeleton != null) && (this->pSkeleton->skel != null))
	{
		this->pSkeleton->skel->setPosition(this->pos.x, this->pos.y);
		//flip on X based on looking direction
		/*
		if (this->vLookDir.x < 0.0f)
			this->pSkeleton->skel->setScaleX(1.0f);
		else
			this->pSkeleton->skel->setScaleX(-1.0f);
			*/
	}
}

void CActor::Spine_SaveAnimPointers()
{
	int nAnimsChanged = 0;
	for (int anm = 0; anm < K_SD_ANIMS_CNT; anm++)
	{
		for (int kk = 0; kk < K_ACT_ANIM_MAX_SETS; kk++)
		{
			this->arrAnimsPtr[anm].bLooping = this->actTemplate.arrAnims[anm].bLooping;
			// check for changes in the template
			if ((this->actTemplate.arrAnims[anm].animNamesA[kk].IsEmpty()) ||
				(this->arrAnimsPtr[anm].animNamesA[kk] != this->actTemplate.arrAnims[anm].animNamesA[kk]))
			{
				this->arrAnimsPtr[anm].animNamesA[kk] = this->actTemplate.arrAnims[anm].animNamesA[kk];
				// erase outdated anim pointers
				this->arrAnimsPtr[anm].pAnim[kk] = null;
			}

			if ((this->arrAnimsPtr[anm].pAnim[kk] == null) && (this->arrAnimsPtr[anm].animNamesA[kk].IsSet()))
			{
				// set new anim pointer
				spine::Animation* anim = pSkelTemplate->GetAnimation(this->arrAnimsPtr[anm].animNamesA[kk].text);
				if (anim != null)
				{
					this->arrAnimsPtr[anm].pAnim[kk] = anim;
					nAnimsChanged++;
				}
				else
				{
					LOG_DBG("[WARNING] CActor::UpdateAnimationPointers: Couldn't find animation [%s]", this->arrAnimsPtr[anm].animNamesA[kk].text);
				}
			}
		}
	}

	LOG_DBG(L"CActor::UpdateAnimationPointers: Updates %d animations", nAnimsChanged);
}

spine::TrackEntry* CActor::SetAnimOnce(int nTrack, ESpineAnim eAnim)
{
	assert((nTrack >= 0) || (nTrack < K_ACT_MAX_ANIM_TRACKS));
	assert((eAnim >= K_SD_ANIM_EMPTY) && (eAnim < K_SD_ANIMS_CNT));

	spine::TrackEntry* trk = null;
	if (eAnim != eLastAnim[nTrack])
	{
		eLastAnim[nTrack] = eAnim;

		if (eAnim <= K_SD_ANIM_EMPTY)
		{
			LOG_DBG(L"ACTOR[%s].SetAnimOnce: Clearing track %d.", actTemplate.shID.text, nTrack);
			pSkeleton->anim->setEmptyAnimation(nTrack, K_SM_DEFAULT_MIX_DURATION);
		}
		else
		{
			if (HasAnimation(eAnim))
			{
				LOG_DBG(L"ACTOR[%s]:SetAnimOnce:", actTemplate.shID.text);
				LOG_DBG("track:%d anim:%s", nTrack, actTemplate.arrAnims[(int)eAnim].animNamesA[this->nAnimSet].text);
				trk = pSkeleton->anim->setAnimation(nTrack, this->arrAnimsPtr[(int)eAnim].pAnim[this->nAnimSet], this->arrAnimsPtr[(int)eAnim].bLooping);
			}
			else
			{
				LOG_DBG(L"ACTOR[%s]:SetAnimOnce:", actTemplate.shID.text);
				LOG_DBG("ANIM NOT FOUND! [%s]", actTemplate.arrAnims[eAnim].animNamesA[this->nAnimSet].text);
			}
		}
	}

	return trk;
}

spine::TrackEntry* CActor::AddAnimOnce(int nTrack, ESpineAnim eAnim, float fMixTime /*= K_SM_DEFAULT_MIX_DURATION*/, float fDelay /*= 0.0f*/)
{
	assert((nTrack >= 0) || (nTrack < K_ACT_MAX_ANIM_TRACKS));
	assert((eAnim >= K_SD_ANIM_EMPTY) && (eAnim < K_SD_ANIMS_CNT));

	spine::TrackEntry* trk = null;
	if (eAnim != eLastAnim[nTrack])
	{
		eLastAnim[nTrack] = eAnim;

		// set the anim on empty => clear the track
		if (eAnim <= K_SD_ANIM_EMPTY)
		{
			LOG_DBG(L"[WARNING] ACTOR[%s].AddAnimOnce: Empty anim on track %d.", actTemplate.shID.text, nTrack);
			pSkeleton->anim->addEmptyAnimation(nTrack, fMixTime, 0.0f);
		}
		else
		{
			if (HasAnimation(eAnim))
			{
				LOG_DBG(L"ACTOR[%s]:AddAnimOnce:", actTemplate.shID.text);
				LOG_DBG("track:%d anim:%s fMixtime:%.2f fDelay:%.2f", nTrack, actTemplate.arrAnims[(int)eAnim].animNamesA[this->nAnimSet].text, fMixTime, fDelay);
				trk = pSkeleton->anim->addAnimation(nTrack, this->arrAnimsPtr[(int)eAnim].pAnim[this->nAnimSet], this->arrAnimsPtr[(int)eAnim].bLooping, fDelay);
				//set custom mix time
				trk->setMixTime(fMixTime);
			}
			else
			{
				LOG_DBG(L"ACTOR[%s]:AddAnimOnce:", actTemplate.shID.text);
				LOG_DBG("ANIM NOT FOUND! [%s]", actTemplate.arrAnims[eAnim].animNamesA[this->nAnimSet].text);
			}
		}
	}

	return trk;
}

void CActor::PlaySoundVersePos(D3DXVECTOR2 vListenerPos, EActorSoundVerse sVerse, bool bPlayIfNotPlayingOnly /*= false*/)
{
	if (sVerse == K_LVL_ACT_VERSE_EMPTY)
		return;

	int nVariation = -1;
	if (actTemplate.soundIDs[(int)sVerse][0] >= 0)
		nVariation = 0;
	if (actTemplate.soundIDs[(int)sVerse][1] >= 0)
		nVariation = randint(2);

	if (nVariation < 0)
		return;

	//play only once
	if (bPlayIfNotPlayingOnly)
	{
		if (SND_IS_PLAYING(actTemplate.soundIDs[(int)sVerse][nVariation]))
			return;
	}
	// save last played verse
	eLastPlayedVerse = sVerse;
	//actually play the sound
	//play only nearby sounds
	D3DXVECTOR2 vDist(pos.x - vListenerPos.x, pos.y - vListenerPos.y);
	if (D3DXVec2Length(&vDist) < K_GAME_WIDTH_MAX * 0.5f * 1.5f)
	{
		SND_PLAY_POSITIONAL(actTemplate.soundIDs[(int)sVerse][nVariation], pos);
	}

}

void CActor::EquipWpn(CWeapon * pWeapon)
{
	pCurrentWeapon = pWeapon;
	if (pWeapon != null)
	{
		AddWpnTemplate(pWeapon);

		//vAimPos = posHeart + vLookDir * 100.0f;
	}
	else
	{
		AddWpnTemplate(null);
	}
}

void CActor::AddWpnTemplate(CWeapon * pWeapon)
{
	//reset actor template to initial one
	actTemplate = actTemplate_ini;
	/*
	if ((pWeapon != null) && (!pWeapon->m_template.shTemplateOverwrite.IsEmpty()))
	{
		CActorTemplate* updateTemplate = GetSim().Actor_GetTemplate(pWeapon->m_template.shTemplateOverwrite.textHash);
		if (updateTemplate == null)
		{
			ErrorBox(K_ERR_WARNING, L"CActor::AddWpnTemplate failed! Template %s not found for weapon %s!", pWeapon->m_template.shTemplateOverwrite.text, pWeapon->m_template.name.text);
		}
		actTemplate.AddGenericDataFromTemplate(updateTemplate);
		bool bChangedAnims = actTemplate.OverwriteAnimsFromTemplate(updateTemplate);
		if (bChangedAnims)
			this->Spine_SaveAnimPointers();

		//reset animations (make sure they get set)
		for (int kk = 0; kk < K_ACT_MAX_ANIM_TRACKS; kk++)
		{
			eLastAnim[kk] = K_SD_ANIM_EMPTY;
		}
	}
	*/
}
