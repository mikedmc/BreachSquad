#include "dxstdafx.h"
#include "Actor.h"

void CActor::PostConstructionInit()
{
	// compute bboxes on init
	bbox.Set(&bbox_ini, pos.xy_proj);
	bbox_floor.Set(&bbox_floor_ini, pos.xy);
}

void CActor::BeginPlay()
{
	c_graphics->SetAnimOnce(K_ACT_ANIM_IDLE);
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

CActor::CActor(Vec2 vnPos, CActorTemplate* pActorTemplate, int nID, CSpriteActorComponent* pComGraphics, CWeaponsComponent* pComWpn ) :
	m_pAIcurrentState(nullptr), m_nAIcurrentBehaviorIdx(-1), m_fAIbehaviorTimer(0.0f), nLastDamageTakenFromUID(0),
	pClosestTouchable(nullptr), nSuspendedFlags(0), fSuspendedTimer(0.0f), bSuspendInput(false), bHasGravity(true),
	eLastPlayedVerse(K_LVL_ACT_VERSE_EMPTY), fVerseCooldown(0.0f), nLastPlayedVerseSndIdx(-1),
	eInteractState(K_STATE_NOTSET), nInteractOptionsSelIdx(0)
{
	_ASSERT(pComGraphics != nullptr);
	// save pointer to component
	c_graphics = pComGraphics;
	c_weapons = pComWpn;

	ID = nID;
	bAnimated = true;
	nControllerInstanceID = -1;
	vSpeedImpulse = Vec2(0.0f, 0.0f);
	speed = Vec2(0.0f, 0.0f);

	// init actor template data (loads files and spine skeletons)
	InitFromTemplate(pActorTemplate);
	
	//update all relative data
	SetPos(Vec2ToVec3XY0(vnPos));
}

CActor::~CActor()
{
	// remove used components received as pointers 
	SAFE_DELETE( c_graphics );
	SAFE_DELETE( c_weapons );
}

bool CActor::IsAlive()
{
	return ((IsPendingKill() == false) && (bHidden == true) && (fLife > 0.0f));
}

void CActor::SetPos(Vec3 newPos)
{
	pos_last = pos.xyz;
	pos = newPos;
	vHeart.Set( pos.xyz.x, pos.xyz.y, pos.xyz.z + _template.heartZ );

	bbox.Set(&bbox_ini, pos.xy_proj);
	bbox_floor.Set(&bbox_floor_ini, pos.xy);
}

void CActor::Move(Vec3 delta)
{
	pos_last = pos.xyz;
	Vec3 npos = pos_last + delta;
	pos = npos;
	vHeart.Set( pos.xyz.x, pos.xyz.y, pos.xyz.z + _template.heartZ );

	bbox.Set(&bbox_ini, pos.xy_proj);
	bbox_floor.Set(&bbox_floor_ini, pos.xy);
}


bool CActor::InitFromTemplate(CActorTemplate * pActorTemplate)
{
	if (pActorTemplate == NULL)
	{
		ErrorBox(K_ERR_CRITICAL, L"Actor template is null for ID:%d!", this->ID);
		return false;
	}
	//copy template data
	_template = *pActorTemplate;
	_template.FillDefaultValuesIfNotSet();

	eLastPlayedVerse = K_LVL_ACT_VERSE_EMPTY;

	bCrouched = false;
	nLastDamageTakenFromUID = 0;

	bSkipRender = false;
	// compute bboxes
	bbox_floor_ini = _template.bbox;
	//#TODO: should be different
	bbox_ini = bbox_floor_ini;
	heightZ = _template.heightZ;
	
	//set hue
	byte collvl = 255;
	color_ini = D3DCOLOR_ARGB(255, collvl, collvl, collvl);
	color = this->color_ini;

	fLife = this->_template.fLife;

	/*
	if (!this->actTemplate.shWeaponDefault.IsEmpty())
	{
		//weapons[0].Init(this->actTemplate.shWeaponDefault.text, this);
		weapons[0].Init();
	}
	*/

	///--- finished setting up, now save backup template for initial state ---
	_template_ini = _template;

	// Load actor graphics
	WCHAR Path[MAX_PATH];
	WCHAR wcsPath[MAX_PATH];
	swprintf_s(wcsPath, MAX_PATH, L"media/levels/data/actors/%s", _template.shSourceXML.text);
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

	//#TODO: oare ar trebui sa isi ia singur datele din actor componenta si sa seteze singura animatiile??
	if(MUVec2AlmostZero(speed))
		c_graphics->SetAnimOnce(K_ACT_ANIM_IDLE);
	else
		c_graphics->SetAnimOnce(K_ACT_ANIM_RUN);

	Vec2 vAim = m_AIcommands.vAimVec;

	//c_graphics->SetAimVecLocal(Vec2(vAim.x, vAim.y));
	/*
	Vec2 vGunMount(0.0f, 0.0f);
	if (c_graphics->GetGunPosWorld(vGunMount))
	{
		// find the position in 3d so that the projected position always matches the default bullet height
		this->posWeapon.x = vGunMount.x;
		this->posWeapon.y = vGunMount.y + Z_TO_H(K_BULLET_DEFAULT_Z);
		this->posWeapon.z = K_BULLET_DEFAULT_Z;
	}
	*/

	//#TODO: update all components after we have the final player position
	c_graphics->Update(*this, dTime);
	// update weapon after updating the body because it depends on mount points
	c_weapons->Update( *this, dTime );

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

void CActor::Paint( ETexChannel eChannel /*= K_TEXCHAN_COLORMAP */ )
{
	// get angle from actual animation and not aim vector because they might differ
	EDir6 actang = c_graphics->GetEAngle();
	bool bFacingS = GetDir6VecN( actang ).y > 0 ? true : false;
	
	if ( bFacingS )
	{
		c_graphics->Paint( *this, eChannel );
		c_weapons->Paint( *this, eChannel );
	}
	else
	{
		c_weapons->Paint( *this, eChannel );
		c_graphics->Paint( *this, eChannel );
	}
}

VecProj CActor::GetWeaponMountWorld( bool bTwoHanded, int mountIndex /*= 0 */ )
{
	// get mount position in screen space (from editor)
	Vec2 vMount = c_graphics->GetMountPoint( bTwoHanded, mountIndex );
	// add weapon mount offset
	Vec2 vWpnOff = c_weapons->GetCurrentWeapon()->_template.vMountOffset;
	vWpnOff.x *= (float)c_graphics->GetFlipDirX();
	vMount += vWpnOff;

	VecProj vpRet = pos;
	vpRet.Set( pos.xyz.x + vMount.x, pos.xyz.y, pos.xyz.z - H_TO_Z( vMount.y ));
	return vpRet;
}

VecProj CActor::GetWeaponMuzzleWorld( bool bTwoHanded, int mountIndex /*= 0 */ )
{
	// get mount position in screen space (from editor)
	Vec2 vMuzzleVec = c_weapons->GetWeaponMuzzlePoint();
	VecProj vpMount = GetWeaponMountWorld( bTwoHanded, mountIndex );
	// if animations are flipped we need to also flip the weapon vectors
	vMuzzleVec.y *= (float)c_graphics->GetFlipDirX();
	// rotate weapon muzzle vector and add it to the projected position of the mount
	Mat mrot;
	float aim_angle = UTMath::GetVectorAngle( m_AIcommands.vAimVec );
	MUMatRotZ( &mrot, aim_angle );
	MUVec2TransformCoord( &vMuzzleVec, &vMuzzleVec, &mrot );
	Vec2 muzzle_proj = vpMount.xy_proj + vMuzzleVec;
	// transform mount position from projected to 3d, knowing that it shoots at the heart height
	return VecProj( muzzle_proj.x, muzzle_proj.y + Z_TO_H(_template.heartZ), _template.heartZ );
}

void CActor::EquipWeapon( int wpnIdx )
{
	const CWeapon* wpn = c_weapons->Equip( wpnIdx );
	// hide hands corresponding to current weapon mode
	//#TODO: ar trebui facuta o functie separata care sa ia in considerare si behaviour curent daca ascunde arme sau nu?
	if ( wpn == nullptr )
		c_graphics->SetSkinFlags( *this, true, true );
	else if ( wpn->_template.bSingleHanded == false || wpn->_template.bDualWielding == true )
		c_graphics->SetSkinFlags( *this, false, false );
	else
		c_graphics->SetSkinFlags( *this, false, true );
}

void CActor::PlaySoundVersePos(D3DXVECTOR2 vListenerPos, EActorSoundVerse sVerse, bool bPlayIfNotPlayingOnly /*= false*/)
{
	if (sVerse == K_LVL_ACT_VERSE_EMPTY)
		return;

	int nVariation = -1;
	if (_template.soundIDs[(int)sVerse][0] >= 0)
		nVariation = 0;
	if (_template.soundIDs[(int)sVerse][1] >= 0)
		nVariation = randint(2);

	if (nVariation < 0)
		return;

	//play only once
	if (bPlayIfNotPlayingOnly)
	{
		if (SND_IS_PLAYING(_template.soundIDs[(int)sVerse][nVariation]))
			return;
	}
	// save last played verse
	eLastPlayedVerse = sVerse;
	//actually play the sound
	//play only nearby sounds
	Vec2 vDist(pos.xy.x - vListenerPos.x, pos.xy.y - vListenerPos.y);
	if (MUVec2Len(&vDist) < K_GAME_WIDTH * 0.5f * 1.5f)
	{
		SND_PLAY_POSITIONAL(_template.soundIDs[(int)sVerse][nVariation], pos.xy);
	}

}

void CActor::ApplyWeaponTemplate(CWeapon * pWeapon)
{
	//reset actor template to initial one
	_template = _template_ini;
	/*
	if ((pWeapon != null) && (!pWeapon->_template.shTemplateOverwrite.IsEmpty()))
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
