#include "dxstdafx.h"
#include "Actor.h"

Vec2 CActor::GetPosHeart()
{
	return pos.xy_proj;
}

Vec3 CActor::GetPosWeapon()
{
	return posWeapon;
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

void CActor::AddWeapon(CWeapon* wpn, bool bEquip)
{
	arrWeapons.Add(wpn);
	if (bEquip)
	{
		pWeaponMain = wpn;
		AddWpnTemplate(pWeaponMain);
	}
}

void CActor::EquipWeapon(int nWeaponIdx)
{
	if ((nWeaponIdx < 0) || (nWeaponIdx >= arrWeapons.GetSize()))
		return;

	pWeaponMain = arrWeapons[nWeaponIdx];
	AddWpnTemplate(pWeaponMain);
}

void CActor::PostConstructionInit()
{
	// compute bboxes on init
	bbox.Set(&bbox_ini, pos.xy_proj);
	bbox_floor.Set(&bbox_floor_ini, pos.xy);
}

void CActor::BeginPlay()
{
	c_graphics->SetAnimOnce(K_ACT_ANIM_IDLE, eAngle);
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

CActor::CActor(Vec2 vnPos, CActorTemplate* pActorTemplate, int nID, CSpriteAnimComponent* pComGraphics) :
	m_pAIcurrentState(nullptr), m_nAIcurrentBehaviorIdx(-1), m_fAIbehaviorTimer(0.0f), nLastDamageTakenFromUID(0),
	pWeaponMain(nullptr), pClosestTouchable(nullptr), bAnimFlipX(false), eAngle(EDIR6_S),
	nAnimSet(0), nSuspendedFlags(0), fSuspendedTimer(0.0f), bSuspendInput(false),
	eLastPlayedVerse(K_LVL_ACT_VERSE_EMPTY), fVerseCooldown(0.0f), nLastPlayedVerseSndIdx(-1),
	eInteractState(K_STATE_NOTSET), nInteractOptionsSelIdx(0)
{
	_ASSERT(pComGraphics != nullptr);
	// save pointer to component
	c_graphics = pComGraphics;

	ID = nID;
	bAnimated = true;
	nControllerInstanceID = -1;
	vSpeedImpulse = Vec2(0.0f, 0.0f);
	speed = Vec2(0.0f, 0.0f);
	posWeapon = Vec3(0.0f, 0.0f, 0.0f);

	// init actor template data (loads files and spine skeletons)
	InitFromTemplate(pActorTemplate);
	
	//update all relative data
	SetPos(Vec2ToVec3XY0(vnPos));
}

CActor::~CActor()
{
	// remove used spine component
	SAFE_DELETE(c_graphics);

	// release allocated weapons arsenal
	SAFE_DELETE_GROWABLE_ARRAY(arrWeapons);
}

bool CActor::IsAlive()
{
	return ((IsPendingKill() == false) && (bHidden == true) && (fLife > 0.0f));
}

void CActor::SetPos(Vec3 newPos)
{
	pos_last = pos.xyz;
	pos = newPos;

	bbox.Set(&bbox_ini, pos.xy_proj);
	bbox_floor.Set(&bbox_floor_ini, pos.xy);
}

void CActor::Move(Vec3 delta)
{
	pos_last = pos.xyz;
	Vec3 npos = pos_last + delta;
	pos = npos;

	bbox.Set(&bbox_ini, pos.xy_proj);
	bbox_floor.Set(&bbox_floor_ini, pos.xy);
}

CActorTemplate::CActorTemplate() :
	arrSkinsCnt( 0 ),
	//generic params
	fLife( K_NOT_SET ), fArmor( K_NOT_SET ), fSpeedMove( K_NOT_SET ),
	actorClass( K_LVL_ACT_CLASS_NOT_SET ),
	//more important values
	eMaterial( K_LVL_MATERIAL_UNKNOWN ), eCaps( K_ACT_CAPS_NONE ),
	AItemplate( nullptr ), fMass( 100.0f ),
	fHeight( 32.0f )
{
	//reset anim IDs
	for (int kk = 0; kk < K_ACT_ANIMS_CNT; kk++)
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
	// set default aabb
	bbox.Set(-6.0f, -6.0f, 6.0f, 6.0f);
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
	if (pTemplate == NULL)
	{
		return false;
	}

	bool bChanged = false;
	for (int kk = 0; kk < K_ACT_ANIMS_CNT; kk++)
	{
		if (bEraseOldAnimations)
		{
			arrAnims[kk].Reset();
			bChanged = true;
		}

		for (int jj = 0; jj < K_ACT_ANIM_MAX_SETS; jj++)
		{
			for ( int ang = 0; ang < EDIR6S_CNT; ang++ )
			{
				//overwrite if existing
				if ( pTemplate->arrAnims[ kk ].animNamesA[ jj ][ ang ].IsSet() )
				{
					arrAnims[ kk ].animNamesA[ jj ][ ang ] = pTemplate->arrAnims[ kk ].animNamesA[ jj ][ ang ];
					bChanged = true;
				}
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


UINT32 CActorTemplate::GetSkinMaskValue( WCHAR* skinName )
{
	for ( int kk = 0; kk < K_ACT_SKINS_MAX_SETS; kk++ )
	{
		if ( arrSkins[ kk ].name.IsEqual( skinName ) )
			return arrSkins[kk].layersVisMask;
	}
	return 0xffffffff;
}

bool CActor::InitFromTemplate(CActorTemplate * pActorTemplate)
{
	if (pActorTemplate == NULL)
	{
		ErrorBox(K_ERR_CRITICAL, L"Actor template is null for ID:%d!", this->ID);
		return false;
	}
	//copy template data
	actTemplate = *pActorTemplate;
	actTemplate.FillDefaultValuesIfNotSet();

	eLastPlayedVerse = K_LVL_ACT_VERSE_EMPTY;

	bCrouched = false;
	nLastDamageTakenFromUID = 0;
	nAnimSet = 0;

	bSkipRender = false;
	// compute bboxes
	bbox_floor_ini = actTemplate.bbox;
	//#TODO: should be different
	bbox_ini = bbox_floor_ini;
	fHeight = actTemplate.fHeight;
	
	//set hue
	byte collvl = 255;
	color_ini = D3DCOLOR_ARGB(255, collvl, collvl, collvl);
	color = this->color_ini;

	fLife = this->actTemplate.fLife;

	/*
	if (!this->actTemplate.shWeaponDefault.IsEmpty())
	{
		//weapons[0].Init(this->actTemplate.shWeaponDefault.text, this);
		weapons[0].Init();
	}
	*/

	pWeaponMain = null;

	///--- finished setting up, now save backup template for initial state ---
	actTemplate_ini = actTemplate;

	// Load spine skeleton
	WCHAR Path[MAX_PATH];
	WCHAR wcsPath[MAX_PATH];
	swprintf_s(wcsPath, MAX_PATH, L"media/levels/data/actors/%s", actTemplate.shSourceXML.text);
	FileManager::GetMediaPath(wcsPath, Path);
	c_graphics->InitFromFile(*this, Path);

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

	if(MUVec2AlmostZero(speed))
		c_graphics->SetAnimOnce(K_ACT_ANIM_IDLE, eAngle);
	else
		c_graphics->SetAnimOnce(K_ACT_ANIM_RUN, eAngle);

	//this->SetAnimOnce(1, K_SD_ANIM_SHOOT);

	Vec2 vAim = m_AIcommands.vAimVec;
	// set generic stuff
	bAnimFlipX = (vAim.x < 0.0f) ? true : false;
	int nAnimFlipMul = (bAnimFlipX) ? -1 : 1;
	eAngle = GetDir6FromVec(vAim);

	// aiming IK node must be set each frame or they get reset by the animation
	c_graphics->SetAimVecLocal(Vec2(vAim.x * nAnimFlipMul, -vAim.y));
	Vec2 vGunMount(0.0f, 0.0f);
	if (c_graphics->GetGunPosWorld(vGunMount))
	{
		// find the position in 3d so that the projected position always matches the default bullet height
		this->posWeapon.x = vGunMount.x;
		this->posWeapon.y = vGunMount.y + Z_TO_H(K_BULLET_DEFAULT_Z);
		this->posWeapon.z = K_BULLET_DEFAULT_Z;
	}

	// set new position of the skeleton now before we compute the gun position?
	c_graphics->Update(*this, dTime);

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
	Vec2 vDist(pos.xy.x - vListenerPos.x, pos.xy.y - vListenerPos.y);
	if (MUVec2Len(&vDist) < K_GAME_WIDTH * 0.5f * 1.5f)
	{
		SND_PLAY_POSITIONAL(actTemplate.soundIDs[(int)sVerse][nVariation], pos.xy);
	}

}

void CActor::EquipWpn(CWeapon * pWeapon)
{
	pWeaponMain = pWeapon;
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

void CActor::BuildActionsList()
{
	arrInteractOptions.Clear();
	if (pClosestTouchable == nullptr)
		return;
	//1. get object specific actions
	for (int kk = 0; kk < pClosestTouchable->arrActions.Count(); kk++)
	{
		arrInteractOptions.Add(pClosestTouchable->arrActions[kk]);
	}
	//#TODO: 2. get inventory specific actions for targeted object class
	//#TODO: 3. get player class specific actions for targeted object class
}

void CActor::ClearActionsList()
{
	arrInteractOptions.Clear();
}
