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

void CActor::SetAIState( CAIState* pNewState )
{
	return c_AI->Actor_SetAIState( *this, pNewState );
}

bool CActor::SetAIState( WCHAR * strStateName )
{
	return c_AI->Actor_SetAIState( *this, strStateName );
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

	//#TODO: Stun Timer ar trebui sa fie parte din componenta de AI ca si input
	if ( fLife > 0.0f )
	{
		if ( fStunTimer > 0.0f )
		{
			fStunTimer -= dTime;
			//#TODO: cand a terminat stun il anunt ca a fost lovit
		}
	}
	else
		fStunTimer = 0.0f;
	//verse timer
	dec_limit( fVerseCooldown, dTime, 0.0f );

	//#TODO: oare ar trebui sa isi ia singur datele din actor componenta si sa seteze singura animatiile??
	if(MUVec2AlmostZero(speed))
		c_graphics->SetAnimOnce(K_ACT_ANIM_IDLE);
	else
		c_graphics->SetAnimOnce(K_ACT_ANIM_RUN);

	// Update actor AI
	c_AI->Update( *this, dTime );
	Vec2 vAim = c_AI->m_AIcommands.vAimVec;
	// now process the AI commands
	ProcessAICommands( level );
	// Move based on speeds
	DoMove( dTime, level );
	// Processes extra stuff before painting
	ProcessExtras( level );
	// Update all components after we have the final player position
	c_graphics->Update(*this, dTime);
	// compute weapon control before updating the weapons
	ComputeAttackStatus();
	// update weapon after updating the graphics component because it depends on mount points
	c_weapons->Update( *this, dTime );
	// now we check if the weapon shot and generate the bullets
	CheckShoot( level );
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

void CActor::ProcessAICommands( CLevel& level )
{
	//----------------------------------------
	//	EXECUTE - process AI output  
	//----------------------------------------
	///--- AI commands ---

	//save old crouch state
	bool bCrouchedOldState = bCrouched;

	//set crouch
	bCrouched = c_AI->m_AIcommands.bCrouched;

	///--- speed and movement ---
	if ( c_AI->m_AIcommands.bThrust )
	{
		//add speed
		float fspeed = _template.fSpeedMove;

		// set final speed
		speed = c_AI->m_AIcommands.vMoveDir * fspeed;
	}
	else
	{
		speed = Vec2( 0.0f, 0.0f );
	}

	//comanda culoare
	if ( c_AI->m_AIcommands.nColor != 0 )
	{
		color = c_AI->m_AIcommands.nColor;
	}

	//death elements (intra doar daca e declarat mort in senzor sau daca i se forteaza starea de dead)
	if ( (c_AI->m_AIsensorInfo.b_IsDead) || (GetCurrentBehavior() == AI_BEHAVIOR_DEAD) )
	{
		switch ( c_AI->m_AIcommands.nDeathCommand )
		{
			case K_LVL_ACT_DEATHCMD_RESET_TO_ZERO:
			{
				fLife = 0.0f;
			}
			break;
			case K_LVL_ACT_DEATHCMD_SPLAT:
			{
				//don't explode hidden actors
				if ( bSkipRender )
					break;

				if ( !UTApp().m_Settings.bGoreEnabled )
				{
					g_particlesMgr.GenerateEnemySoftGib( pos.xy_proj, 0xff32a7fa, K_PART_LAYER_RT_FRONT_NRM );
				}
				else
				{
					//blood splat (sortate crescator in animatie)
					level.AddDecal_BloodSplat( GetPosHeart(), true, _template.actorClass );

					//SND_PLAY_POSITIONAL_RAND2(SNDIDX_BULLET_BODY_GIBBED_01, SNDIDX_BULLET_BODY_GIBBED_02, actor->GetPosHeart());
					//meat lumps
					Vec2 bulletSpeed;
					MUVec2Norm( &bulletSpeed, &vSpeedImpulse );

					CAABB genbox = bbox;
					genbox.Inflate( -2.0f, -2.0f );
					if ( _template.fLife > 10.0f )
					{
						DWORD dwCol = 0xff671010;
						int nSubType = 0;
						if ( _template.actorClass == K_LVL_ACT_CLASS_ZOMBIE )
						{
							dwCol = 0xff82b600;
							nSubType = 1;
						}
						for ( int ll = 0; ll < 6; ll++ )
						{
							level.AddDoofer( K_DOOFER_MEAT, AABB::GetRandomPointInBox( genbox ), &Vec2( randfloatsgn( 50.0f ) + bulletSpeed.x * 50.0f, -130.0f - randfloat( 100.0f ) ), &g_vecGravityOld, nSubType );
						}
						//goes straight down to stain the floor
						level.AddDoofer( K_DOOFER_MEAT, GetPosHeart(), &Vec2( 200.0f, 50.0f ), &g_vecGravityOld, nSubType );
						level.AddDoofer( K_DOOFER_MEAT, GetPosHeart(), &Vec2( -200.0f, 50.0f ), &g_vecGravityOld, nSubType );
						//human blood gibs particle
						g_particlesMgr.AddParticle( ANM_PARTICLES_SPR_HUMAN_SPLAT_MED, true, 0, &pos.xy_proj, NULL, NULL, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, dwCol, K_PART_LAYER_RT_FRONT_NRM );
					}
					else //small animals and stuff
					{
						for ( int ll = 0; ll < 2; ll++ )
						{
							level.AddDoofer( K_DOOFER_MEAT, AABB::GetRandomPointInBox( genbox ), &Vec2( randfloatsgn( 50.0f ) + bulletSpeed.x * 50.0f, -130.0f - randfloat( 100.0f ) ), &g_vecGravityOld );
						}
						g_particlesMgr.AddParticle( ANM_PARTICLES_SPR_HUMAN_SPLAT_SMALL, true, 0, &pos.xy_proj, NULL, NULL, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xff671010, K_PART_LAYER_RT_FRONT_NRM );
					}
				}

				//players don't deallocate. They only become invisible.
				if ( _template.actorClass == K_LVL_ACT_CLASS_PLAYER )
				{
					fLife = 0.0f;
					bSkipRender = true;
					//reset physics
					speed = Vec2( 0.0f, 0.0f );
					vSpeedImpulse = Vec2( 0.0f, 0.0f );
					//move invisible body back to last safe pos
					//Vec2 vSpawnPos = level.m_arrPlayerLastSafePos[ nPlayerOrdinal ];
					//SetPos( Vec3( vSpawnPos.x, vSpawnPos.y, 0.0f ) );
					break;
				}
				//deallocate
				SetEnabled( false );
				Kill();
			}
			break;
			case K_LVL_ACT_DEATHCMD_DEALLOCATE:
			{
				//dezalocare
				SetEnabled( false );
				Kill();
			}
			break;
		}
		//remove death command after execution
		c_AI->m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_EMPTY;
	}


}

void CActor::DoMove( float dTime, CLevel& level )
{
	// temp list for collisions
	static CFixedArray<SweepAABB, 100> tempCollBoxList;
	///------------------------------------------------------------------------------------------
	///	INTEGRATOR - physics
	///------------------------------------------------------------------------------------------
	//#TODO: check speed limits - should be done on the speed vector, normalized
	CLAMP( speed.x, -K_LVL_ACTOR_MAX_SPEED, K_LVL_ACTOR_MAX_SPEED );
	CLAMP( speed.y, -K_LVL_ACTOR_MAX_SPEED, K_LVL_ACTOR_MAX_SPEED );
	//update impulse
	Vec2 impFriction( K_LVL_GROUND_DEFAULT_FRICTION, K_LVL_GROUND_DEFAULT_FRICTION );
	//limit impulse
	CLAMP( vSpeedImpulse.y, -K_LVL_ACTOR_MAX_IMPULSE, K_LVL_ACTOR_MAX_IMPULSE );
	CLAMP( vSpeedImpulse.x, -K_LVL_ACTOR_MAX_IMPULSE, K_LVL_ACTOR_MAX_IMPULSE );

	vSpeedImpulse.x -= vSpeedImpulse.x * impFriction.x * dTime;
	vSpeedImpulse.y -= vSpeedImpulse.y * impFriction.y * dTime;

	Vec3 vPosIni = pos.xyz;
	UINT16 unCollFlags = 0;

	///--- collision detection ---
	{
		///a.calculezi vectorul de miscare al actorului(viteza * dt + miscare paltforma daca e necesar)
		Vec2 vNextMove = (speed + vSpeedImpulse) * dTime; // Add connected platform movement if needed
		///b.detectezi coliziuni posibile(bbox old + new pos)
		//1. find bbox start and end union that includes all collisions when moving at high speeds
		CAABB destbox, srcbox;
		srcbox = bbox_ini; srcbox.Move( pos.xy );
		destbox = bbox_ini; destbox.Move( pos.xy + vNextMove );
		// box unions to check all possible collisions
		CAABB boxUnion = AABB::Union( destbox, srcbox );
		// bbox union in tile coords, including every touched tile
		RectXYXYi boxUnionTiles( floor( boxUnion.vMin.x / K_TILE_SIZE_F ), floor( boxUnion.vMin.y / K_TILE_SIZE_F ),
			ceil( boxUnion.vMax.x / K_TILE_SIZE_F ), ceil( boxUnion.vMax.y / K_TILE_SIZE_F ) );
		RectXYWHi boxUnionTilesWH( boxUnionTiles.x1, boxUnionTiles.y1, boxUnionTiles.x2 - boxUnionTiles.x1 + 1, boxUnionTiles.y2 - boxUnionTiles.y1 + 1 );
		//optional - to include more of the boxes
		//boxUnion.Inflate(K_TILE_HSIZE, K_TILE_HSIZE);

		// keeps a list of all boxes that might be colliding
		tempCollBoxList.Clear();

		/// BROAD PHASE SWEEP (find all POSSIBLE collision objects)

		//add boxes from collision shapes
		for ( int kk = 0; kk < level.m_arrColShapes.GetSize(); kk++ )
		{
			if ( !level.m_arrColShapes[ kk ]->IsAlive() )
				continue;

			//nu am intersectie probabils - trec mai departe
			if ( !boxUnion.Intersects( level.m_arrColShapes[ kk ]->bbox ) )
				continue;

			//adauga bbox in lista de probabile pt intersectie
			if ( level.m_arrColShapes[ kk ]->collFlags != K_DIRFLAG_NONE )
			{
				tempCollBoxList.Add( level.m_arrColShapes[ kk ]->bbox );
			}
		}
		//add boxes from tiles
		//#MAYBE: if it catches some corners sometimes try enlarging the tiles collision area (boxUnionTiles) by 1 tile in all directions
		static CAABB retAABBs[ 64 ];
		if ( pArea != nullptr )
		{
			// get collision tiles for current area
			// tiles collboxes
			int nadded = pArea->GetTilesCollisionBoxes( boxUnionTiles, retAABBs, 64 );
			if ( nadded > 0 )
			{
				for ( int oo = 0; oo < nadded; oo++ )
				{
					tempCollBoxList.Add( retAABBs[ oo ] );
				}
			}
			// props collboxes
			nadded = pArea->GetPropsCollisionBoxes( boxUnion, retAABBs, 64 );
			if ( nadded > 0 )
			{
				for ( int oo = 0; oo < nadded; oo++ )
				{
					tempCollBoxList.Add( retAABBs[ oo ] );
				}
			}
			// if movement bbox is not completely contained in the current area BBox try with the neighbours too
			if ( !pArea->AABBbounds.Contains( boxUnion ) )
			{
				for ( int kk = 0; kk < pArea->arrNeighbours.Count(); kk++ )
				{
					CLevelArea* area = pArea->arrNeighbours.m_pData[ kk ];
					if ( !area->AABBbounds_TL.Intersects( boxUnionTilesWH ) )
						continue;
					// tiles collboxes
					nadded = area->GetTilesCollisionBoxes( boxUnionTiles, retAABBs, 64 );
					if ( nadded > 0 )
					{
						for ( int oo = 0; oo < nadded; oo++ )
						{
							tempCollBoxList.Add( retAABBs[ oo ] );
						}
					}
					// props collboxes
					nadded = area->GetPropsCollisionBoxes( boxUnion, retAABBs, 64 );
					if ( nadded > 0 )
					{
						for ( int oo = 0; oo < nadded; oo++ )
						{
							tempCollBoxList.Add( retAABBs[ oo ] );
						}
					}
				}
			}
		}

		/// COLLISION HANDLING

		float fRemainingTime = 1.0f;
		while ( fRemainingTime > 0.0f )
		{
			// compute source box
			srcbox = bbox_ini; srcbox.Move( pos.xy );
			// find closest collider
			float minDistSq = 100000.0f;
			float fClosestTime = 100000.0f;
			SweepAABB* pClosestBox = nullptr;
			for ( int kk = 0; kk < tempCollBoxList.Count(); kk++ )
			{
				SweepAABB* tmpbox = &tempCollBoxList[ kk ];
				// skip boxes that have been handled this step
				if ( tmpbox->bDisabled )
					continue;

				SweepData sdata = AABBSweep::CalculateSweepData( srcbox, vNextMove, *tmpbox );
				// computes even if no valid collision. needs flag to eliminate them
				if ( sdata.bIsValid == false )
					continue;

				if ( sdata.fCollisionTime < fClosestTime )
				{
					fClosestTime = sdata.fCollisionTime;
					minDistSq = sdata.fDistance;
					pClosestBox = tmpbox;
				}
				else if ( sdata.fCollisionTime == fClosestTime )
				{
					if ( sdata.fDistance < minDistSq )
					{
						fClosestTime = sdata.fCollisionTime;
						minDistSq = sdata.fDistance;
						pClosestBox = tmpbox;
					}
				}
			}

			// do we have a collider?
			if ( pClosestBox != nullptr )
			{
				SweepData hit = AABBSweep::CalculateSweepData( srcbox, vNextMove, *pClosestBox );
				//handled already, disable it
				pClosestBox->bDisabled = true;

				pos.xy += vNextMove * hit.fCollisionTime;

				// Calculate the correct time of impact for the remaining
				// collisions or to apply movement
				float ftime = fRemainingTime - hit.fCollisionTime;

				// Calculate the collision normal (vector used to slide the object that collided)
				// normala e tangenta de fapt...
				float dotProduct = MUVec2Dot( &vNextMove, &hit.vNormal ) * ftime;
				hit.vNormal *= dotProduct;

				// Handle events after each respective side that collided
				//DMC: could implement OnCollision(hit.eSide) if needed
				switch ( hit.eSide )
				{
					case K_SIDE_BOTTOM:
					{
						speed.y = 0.0f;
						vSpeedImpulse.y = 0.0f;
						unCollFlags |= K_DIRFLAG_DOWN;
					}
					break;
					case K_SIDE_TOP:
					{
						speed.y = 0.0f;
						vSpeedImpulse.y = 0.0f;
						unCollFlags |= K_DIRFLAG_UP;
					}
					break;
					case K_SIDE_LEFT:
					{
						speed.x = 0.0f;
						vSpeedImpulse.x = 0.0f;
						unCollFlags |= K_DIRFLAG_LEFT;
					}
					break;
					case K_SIDE_RIGHT:
					{
						speed.x = 0.0f;
						vSpeedImpulse.x = 0.0f;
						unCollFlags |= K_DIRFLAG_RIGHT;
					}
					break;
				}

				if ( ftime > 0.0f )
				{
					vNextMove = hit.vNormal;

					CAABB newboxsrc = bbox_ini;
					newboxsrc.Move( pos.xy );
					CAABB newboxdest = newboxsrc;
					newboxdest.Move( vNextMove );
					CAABB newBoundary = AABB::Union( newboxsrc, newboxdest );

					// call and implement this if you need tile sized boxes to enter tile wide holes
					//this.fixEqualSizedHoleCollision(hit, potential, time, collisionStack);

					//DMC: deactivate those boxes that don't fit the new boundary
					for ( int kk = 0; kk < tempCollBoxList.Count(); kk++ )
					{
						SweepAABB* it = &tempCollBoxList.m_pData[ kk ];
						if ( it->bDisabled )
							continue;
						// disable non intersecting ones
						if ( !newBoundary.Intersects( tempCollBoxList.m_pData[ kk ] ) )
							it->bDisabled = true;
					}
					// update remaining time and do again
					fRemainingTime = ftime;
				}

			}
			else
			{
				pos.xy += vNextMove;
				fRemainingTime = 0.0f;
			}
		}

		//#TODO: could use a penetration resolution round. Maybe after solving each collision so we make sure boxes don't actually touch? TBD
	}

	collisionFlags = unCollFlags;

	//check world bounds for each actor - kill if out
	if ( !Rects::PointInRect( pos.xy, level.m_levelAABB ) )
	{
		level.KillActor( this );
	}

	//set final position
	SetPos( Vec3( pos.xy.x, pos.xy.y, 0.0f ) );
	// save last position in pos_last (SetPos does but we already altered pos)
	pos_last = vPosIni;

}

void CActor::ProcessExtras( CLevel& level )
{
	///--- set current area if null or changed after updating the position
	if ( (pArea == nullptr) || (!pArea->AABBbounds.PointIn( pos.xy )) )
	{
		pArea = level.Areas_GetAt( pos.xy );
	}

	///--- find closest interactible object in range, aka touchable
	if ( _template.eCaps & K_ACT_CAPS_CAN_INTERACT )
	{
		//#TODO: put interact area in special constant
		CAABB aabbInteract( -K_TILE_SIZE_F, -K_TILE_SIZE_F, K_TILE_SIZE_F, K_TILE_SIZE_F );
		aabbInteract.Move( pos.xy );

		CArray<CProp*> arrTouchProps;
		arrTouchProps.SetSize( 16 );
		if ( pArea != nullptr )
		{
			pArea->GetPropsTouchingBox( aabbInteract, arrTouchProps, true );
			// if bbox is not completely contained in the current area BBox try with the neighbours too
			if ( !pArea->AABBbounds.Contains( aabbInteract ) )
			{
				for ( int oo = 0; oo < pArea->arrNeighbours.Count(); oo++ )
				{
					CLevelArea* area = pArea->arrNeighbours.m_pData[ oo ];
					if ( !area->AABBbounds.Intersects( aabbInteract ) )
						continue;
					area->GetPropsTouchingBox( aabbInteract, arrTouchProps, true );
				}
			}
		}
		//#TODO: see which one is closer to the aim dir
		if ( arrTouchProps.GetSize() > 0 )
		{
			IActiveInterface* pNewTouchable = arrTouchProps[ 0 ];
			if ( pClosestTouchable != pNewTouchable )
			{
				ClearActionsList();
			}
			pClosestTouchable = pNewTouchable;
		}
		else
		{
			if ( pClosestTouchable != nullptr )
			{
				ClearActionsList();
			}
			pClosestTouchable = nullptr;
		}
	}

	// check touch/interact
	if ( (c_AI->m_AIcommands.bInteract) && (pClosestTouchable != nullptr) )
	{
		BuildActionsList();
		if ( arrInteractOptions.Count() > 0 )
		{
			eInteractState = K_STATE_READY;
			nInteractOptionsSelIdx = 0;
		}
	}

}

bool CActor::CheckShoot( CLevel& level )
{
	//#TODO: must add support for weapon scripts on shoot and empty
	CWeapon* weapon = c_weapons->GetCurWeapon();
	if ( (weapon == nullptr) || (weapon->pOwner == nullptr) || (weapon->status == K_WPN_STATUS_UNKNOWN) )
		return false;

	// only shoot on JUST_SHOT
	if ( weapon->status != K_WPN_STATUS_JUST_SHOT )
		return false;


	bool bTwoHanded = weapon->_template.bTwoHanded;
	bool bDualWielding = weapon->_template.bDualWielding;

	CActor* shooter = weapon->pOwner;
	Vec3 vFinalDir;
	MUVec3Norm( &vFinalDir, &c_weapons->GetWeaponAimVec() );
	//#TODO: add support for dual wielding
	VecProj vShootPos = shooter->GetWeaponMuzzleWorld( bTwoHanded, 0 );

	int nFinalClass = shooter->_template.actorClass;
	//bullet has template class, set it to final class
	if ( weapon->_template.bulletTemplate.eClass != K_LVL_ACT_CLASS_ANY )
		nFinalClass = weapon->_template.bulletTemplate.eClass;

	//save local bullet template copy
	CBulletTemplate tmplBullet = weapon->_template.bulletTemplate;
	//ammo (-1 infinite)
	int nAmmoReal = weapon->ammoLeft;
	//if weapon uses main weapon ammo check that ammo
	/*
	if (weapon->WeaponTemplate.bUsesMainWeaponAmmo)
		nAmmoReal = weapon->pOwner->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->ammoLeft;
		*/

	if ( nAmmoReal != 0 )
	{


		// shoot bullets
		for ( int kk = 0; kk < weapon->_template.nBulletsPerShot; kk++ )
		{
			//add weapon spread
			float fSpreadAng = level.m_rand.RandFloatSgn( weapon->_template.fSpreadFOV );

			//vFinalDir.x = cos(fAimAng + fSpreadAng);
			//vFinalDir.y = sin(fAimAng + fSpreadAng);
			//D3DXVec2Normalize(&vFinalDir, &vFinalDir);

			CBullet* bullet = level.ShootBullet( &tmplBullet, nFinalClass, shooter->GetUID(), vShootPos.xyz, vFinalDir );
		}

		// add shell
		if ( weapon->_template.nDropShellFrame >= 0 )
		{
			level.AddDoofer( K_DOOFER_SHELL, weapon->pOwner->GetPosHeart(), &Vec2( (40.0f + randfloat( 30.0f )), -50.0f - randfloat( 20.0f ) ), &g_vecGravityOld, weapon->_template.nDropShellFrame );
		}


		// make light
		if ( weapon->_template.fMuzzleLightSize > 0.0f )
		{
			//prop - nozzle light
			float fPropAlpha = 0.8f * weapon->_template.fMuzzleLightSize;
			CLAMP( fPropAlpha, 0.0f, 1.0f );
			//			AddProp_Light(vShootPos, ANM_LIGHTS_SPR_POINT1, 0.05f, 0.0f, D3DCOLOR_COLORALPHA(0xffFDB727, fPropAlpha), weapon->WeaponTemplate.fMuzzleLightSize);
		}
		// add AI sound event
		level.AddAIEvent( K_LVL_AI_EVENT_SOUND_THREAT, shooter->GetUID(), shooter->_template.actorClass, shooter->GetPosHeart(), weapon->_template.fSoundRadius );
	}

	return true;
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
