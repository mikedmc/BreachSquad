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

CActor::CActor(Vec2 vnPos, CActorTemplate* pActorTemplate, int nID,
	CSpriteActorComponent* pComGraphics, CWeaponsComponent* pComWpn, CActorAIComponent* pComAI ) :
	 nLastDamageTakenFromUID(0),
	pClosestTouchable(nullptr), nSuspendedFlags(0), fSuspendedTimer(0.0f), bSuspendInput(false), bHasGravity(true),
	eLastPlayedVerse(K_LVL_ACT_VERSE_EMPTY), fVerseCooldown(0.0f), nLastPlayedVerseSndIdx(-1),
	eInteractState(K_STATE_NOTSET), nInteractOptionsSelIdx(0), eAttackStatus(K_ACT_ATTACK_IDLE)
{
	_ASSERT(pComGraphics != nullptr && pComAI != nullptr && pComWpn != nullptr);
	// save pointer to component
	c_graphics = pComGraphics;
	c_weapons = pComWpn;
	c_AI = pComAI;

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
	SAFE_DELETE( c_AI );
}

bool CActor::IsAlive()
{
	return ((bPendingKill == false) && (bEnabled == true) && (fLife > 0.0f));
}

bool CActor::IsEnabled()
{
	return bEnabled;
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

void CActor::Update(float dTime, CLevel& level )
{
	//change visibility
	this->bEnabled = this->bSetEnabled;
	// actor is hidden or not active so ignore it
	if (!this->bEnabled)
		return;

	//verse timer
	dec_limit( fVerseCooldown, dTime, 0.0f );

	//#TODO: oare ar trebui sa isi ia singur datele din actor componenta si sa seteze singura animatiile??
	if(MUVec2AlmostZero(speed))
		c_graphics->SetAnimOnce(K_ACT_ANIM_IDLE);
	else
		c_graphics->SetAnimOnce(K_ACT_ANIM_RUN);

	Vec2 vAim = c_AI->m_AIcommands.vAimVec;

	c_AI->Update( *this, dTime, __Sim() );
	//update all components after we have the final player position
	c_graphics->Update(*this, dTime);
	// compute stuff linked to the weapons before updating the weapons
	ComputeAttackStatus();
	// update weapon after updating the body because it depends on mount points
	c_weapons->Update( *this, dTime );
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
	Vec2 v_mount = c_graphics->GetMountPoint( bTwoHanded, mountIndex );
	// add weapon mount offset
	Vec2 v_wpn_off = c_weapons->GetCurWeapon()->_template.vMountOffset;
	v_wpn_off.x *= (float)c_graphics->GetFlipDirX();
	v_mount += v_wpn_off;

	VecProj vpRet = pos;
	vpRet.Set( pos.xyz.x + v_mount.x, pos.xyz.y, pos.xyz.z - H_TO_Z( v_mount.y ));
	return vpRet;
}

VecProj CActor::GetWeaponMuzzleWorld( bool bTwoHanded, int mountIndex /*= 0 */ )
{
	// get mount position in screen space (from editor)
	Vec2 v_muzzle_vec = c_weapons->GetWeaponMuzzlePoint();
	VecProj vp_mount = GetWeaponMountWorld( bTwoHanded, mountIndex );
	// if animations are flipped we need to also flip the weapon vectors
	v_muzzle_vec.y *= (float)c_graphics->GetFlipDirX();
	// rotate weapon muzzle vector and add it to the projected position of the mount
	Mat mrot;
	float aim_angle = UTMath::GetVectorAngle( c_AI->m_AIcommands.vAimVec );
	MUMatRotZ( &mrot, aim_angle );
	MUVec2TransformCoord( &v_muzzle_vec, &v_muzzle_vec, &mrot );
	Vec2 muzzle_proj = vp_mount.xy_proj + v_muzzle_vec;
	// transform mount position from projected to 3d, knowing that it shoots at the heart height
	return VecProj( muzzle_proj.x, muzzle_proj.y + Z_TO_H(_template.heartZ), _template.heartZ );
}

void CActor::EquipWeapon( EWpnSlot wpnSlot )
{
	LOG( "Equipped slot: %d", wpnSlot );
	const CWeapon* wpn = c_weapons->Equip( wpnSlot );
	// hide hands corresponding to current weapon mode
	// it always does the full thing even if already on the same weapon
	//#TODO: ar trebui facuta o functie separata care sa ia in considerare si behaviour curent daca ascunde arme sau nu?
	if ( wpn == nullptr )
		c_graphics->SetSkinFlags( *this, true, true );
	else if ( wpn->_template.bTwoHanded == true || wpn->_template.bDualWielding == true )
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

void CActor::ComputeAttackStatus()
{
	switch ( eAttackStatus )
	{
		case K_ACT_ATTACK_IDLE:
		{
			eAttackStatus = c_AI->m_AIcommands.eAttackCommand;
			// only switch to alt weapon if we can shoot
			if ( c_AI->m_AIcommands.eAttackCommand == K_ACT_ATTACK_SHOOTING_ALT )
			{
				CWeapon* wpn = c_weapons->GetWeapon( K_WPNSLOT_ALTFIRE );
				if ( !wpn->IsReadyToShoot() )
					eAttackStatus = K_ACT_ATTACK_IDLE;
			}
		}
		break;
		case K_ACT_ATTACK_SHOOTING:
		{
			CWeapon* wpn = c_weapons->GetWeapon( K_WPNSLOT_PRIMARY );
			if ( wpn->IsShootingBullet() == false )
				eAttackStatus = K_ACT_ATTACK_IDLE;
		}
		break;
		case K_ACT_ATTACK_SHOOTING_ALT:
		{
			CWeapon* wpn = c_weapons->GetWeapon( K_WPNSLOT_ALTFIRE );
			if ( wpn->IsShootingBullet() == false )
				eAttackStatus = K_ACT_ATTACK_IDLE;
		}
		break;
		case K_ACT_ATTACK_RELOADING:
		{
			CWeapon* wpn = c_weapons->GetCurWeapon();
			/// loading can only be interrupted by nome actions
			if ( wpn->status == K_WPN_STATUS_RELOADING )
			{
				//can't shoot until you reload on weapons with bullets clip
				if ( wpn->_template.nReloadUnitSize >= wpn->_template.nClipSize )
				{
					// reloading can be interrupted by the following commands
					if ( c_AI->m_AIcommands.eAttackCommand == K_ACT_ATTACK_MELEE )
					{
						wpn->StopReloading();
						eAttackStatus = K_ACT_ATTACK_IDLE;
					}
				}
			}
			else
			{
				eAttackStatus = K_ACT_ATTACK_IDLE;
			}
		}
		break;
		default:
			eAttackStatus = K_ACT_ATTACK_IDLE;
			break;
	}


	//daca sunt cu arma care incarca glont cu glont pot schimba si in timp ce incarca
	/*
	if ((pNewWeapon != null) && (pNewWeapon != actor->pWeaponMain) &&
		(actor->pWeaponMain->_template.nReloadUnitSize < actor->pWeaponMain->_template.nClipSize) &&
		(actor->nAttackStatus == K_LVL_ACT_ATTACK_RELOADING))
	{
		Weapon_StopReloading(actor->pWeaponMain);
		actor->nAttackStatus = K_LVL_ACT_ATTACK_IDLE;
	}
	*/

	//change weapon
	/*
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
	*/
	//verificari diverse ex. daca esti in aer si tragi cu o arma ce nu poate fi trasa din aer se intrerupe
	/*
	if ((actor->m_AIcommands.eAttackCommand >= K_LVL_ACT_ATTACK_SHOOTING) || (actor->eAttackStatus >= K_LVL_ACT_ATTACK_SHOOTING))
	{
		if ( !actor->Weapons()->CanShoot( eCurSlot ) )
		{
			actor->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_IDLE;
			actor->eAttackStatus = K_LVL_ACT_ATTACK_IDLE;
		}
	}
	*/


	/// decide necessary weapon based on command
	EWpnSlot eCurSlot = K_WPNSLOT_PRIMARY;
	// switches weapon based on commands
	if ( (eAttackStatus == K_ACT_ATTACK_SHOOTING) || (eAttackStatus == K_ACT_ATTACK_RELOADING) )
		eCurSlot = K_WPNSLOT_PRIMARY;
	else if ( eAttackStatus == K_ACT_ATTACK_SHOOTING_ALT )
		eCurSlot = K_WPNSLOT_ALTFIRE;

	//see if weapon needs to be changed
	if ( eCurSlot != c_weapons->GetCurWeaponSlot() )
	{
		EquipWeapon( eCurSlot );
	}
	CWeapon* pWeapon = c_weapons->GetCurWeapon();

	if ( eAttackStatus >= K_ACT_ATTACK_SHOOTING )
	{
		pWeapon->SetTriggerStates( true, false );
	}
	else if ( eAttackStatus == K_ACT_ATTACK_RELOADING )
	{
		if ( pWeapon->_template.nReloadUnitSize != 0 )
			pWeapon->SetTriggerStates( false, true );
	}
	else
	{
		pWeapon->SetTriggerStates( false, false );
	}



	//daca are laser sight o activeaza acum, o singura data cand se da comanda de shoot
	/*
	if ((pWeaponMain->_template.bHasLaserSight) && (actor->nAttackStatus != actor->m_AIcommands.eAttackCommand) && (actor->m_AIcommands.eAttackCommand >= K_LVL_ACT_ATTACK_SHOOTING))
	{
		pWeaponMain->bPaintLaserSight = true;
	}
	*/


	//daca arma curenta nu poate trage din crouch scot crouch
	/*
	if ((actor->bCrouched == true) && (!pWeaponMain->_template.bCanShootFromCrouch))
	{
		if (actor->nAttackStatus >= K_LVL_ACT_ATTACK_SHOOTING)
			actor->bCrouched = false;
	}
	*/

	// weapons that stop you while shooting:
	/*
	if (actor->pWeaponMain->_template.fShooterSpeedSlowingPercent >= 1.0f)
	{
		if ((actor->m_AIcommands.eAttackCommand != K_LVL_ACT_ATTACK_IDLE) || (actor->nAttackStatus != K_LVL_ACT_ATTACK_IDLE))
		{
			actor->m_AIcommands.bThrust = false;
			//actor->m_AIcommands.nMoveDirX = 0;
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
