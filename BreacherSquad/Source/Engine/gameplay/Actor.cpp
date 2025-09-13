#include "dxstdafx.h"
#include "Actor.h"

void CActor::PostConstructionInit()
{
	// compute bboxes from backup on init (might not be necessary but it doesn't hurt)
	//#TODO: see if they're already initialized when spawned
	bbox.RestoreSnapshot( pos.xy_proj );
	bbox_floor.RestoreSnapshot( pos.xy );
	bbox_cull.RestoreSnapshot( pos.xy_proj );
	vHeart.Set( pos.xyz.x, pos.xyz.y, pos.xyz.z + _template.heartZ );
}

void CActor::BeginPlay()
{
	pArea = level->Areas_GetAt( pos.xy );
	_ASSERT(pArea != nullptr);
	c_graphics->SetAnimOnce(K_ACT_ANIM_IDLE);
	SetEnabled( true, true );
}

void CActor::EndPlay()
{
}

CActor::CActor() :
	nLastDamageTakenFromUID( 0 ), bInitialized(false),
	nSuspendedFlags( 0 ), fSuspendedTimer( 0.0f ), bSuspendInput( false ), bHasGravity( true ),
	eLastPlayedVerse( K_LVL_ACT_VERSE_EMPTY ), fVerseCooldown( 0.0f ), nLastPlayedVerseSndIdx( -1 ),
	eInteractState( K_STATE_NOTSET ), nInteractOptionsSelIdx( 0 ), eAttackStatus( K_ACT_ATTACK_IDLE ),
	fStunTimer( 0.0f ),
	level( nullptr ), nControllerInstanceID(-1),
	c_graphics(nullptr), c_weapons(nullptr), c_AI(nullptr)
{
	ID = -1;
}

CActor::~CActor()
{
	Dispose();
}

void CActor::Init( Vec2 vnPos, CActorTemplate* pActorTemplate, int nID, CLevel* refLevel, CSpriteActorComponent* pComGraphics, CWeaponsComponent* pComWpn, CActorAIComponent* pComAI )
{
	_ASSERT( pComGraphics != nullptr && pComAI != nullptr && pComWpn != nullptr );
	// cleans previous instance for reuse
	if ( bInitialized )
	{
		Dispose();
	}
	// save pointer to component
	c_graphics = pComGraphics;
	c_weapons = pComWpn;
	c_AI = pComAI;
	// init defaults
	nLastDamageTakenFromUID = 0;
	nSuspendedFlags = 0; fSuspendedTimer = 0.0f; bSuspendInput = false; bHasGravity = true;
	eLastPlayedVerse = K_LVL_ACT_VERSE_EMPTY; fVerseCooldown = 0.0f; nLastPlayedVerseSndIdx = -1;
	eInteractState = K_STATE_NOTSET; nInteractOptionsSelIdx = 0; eAttackStatus = K_ACT_ATTACK_IDLE;
	fStunTimer = 0.0f;
	level = refLevel;

	ID = nID;
	bAnimated = true;
	nControllerInstanceID = -1;
	vSpeedImpulse = Vec2( 0.0f, 0.0f );
	speed = Vec2( 0.0f, 0.0f );
	vAim = Vec2( 0.0f, -10.0f );

	// init actor template data (loads files and spine skeletons)
	InitFromTemplate( pActorTemplate );

	//update all relative data
	SetPos( Vec2ToVec3XY0( vnPos ) );

	bInitialized = true;
	SetEnabled( true, true );
	bPendingKill = false;
}

void CActor::Dispose()
{
	if ( !bInitialized )
		return;
	CSmartLink::RemoveLink( &pClosestTouchable );
	// remove used components received as pointers 
	SAFE_DELETE( c_graphics );
	SAFE_DELETE( c_weapons );
	SAFE_DELETE( c_AI );

	bInitialized = false;
}

void CActor::SetAIState( CAIState* pNewState )
{
	return c_AI->SetAIState( *this, pNewState );
}

bool CActor::SetAIState( WCHAR * strStateName )
{
	return c_AI->SetAIState( *this, strStateName );
}

bool CActor::IsAlive()
{
	return ((bPendingKill == false) && (bEnabled == true) && (fLife > 0.0f));
}

void CActor::SetPos(Vec3 newPos)
{
	pos_last = pos.xyz;
	pos = newPos;
	vHeart.Set( pos.xyz.x, pos.xyz.y, pos.xyz.z + _template.heartZ );

	bbox.RestoreSnapshot(pos.xy_proj);
	bbox_floor.RestoreSnapshot(pos.xy);
	bbox_cull.RestoreSnapshot( pos.xy_proj );
}

void CActor::Move(Vec3 delta)
{
	pos_last = pos.xyz;
	Vec3 npos = pos_last + delta;
	pos = npos;
	vHeart.Set( pos.xyz.x, pos.xyz.y, pos.xyz.z + _template.heartZ );

	bbox.RestoreSnapshot(pos.xy_proj);
	bbox_floor.RestoreSnapshot(pos.xy);
	bbox_cull.RestoreSnapshot( pos.xy_proj );
}


void CActor::SetAI( EAIstate newstate )
{
	ErrorBox( K_ERR_WARNING, L"CActor::SetAI should not be used! Use SetAIState instead!" );
}

bool CActor::InitFromTemplate(CActorTemplate * pActorTemplate)
{
	if (pActorTemplate == nullptr)
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
	bbox_floor = _template.bbox;
	bbox_floor.SaveSnapshot();
	heightZ = _template.heightZ;
	bbox.Set( _template.bbox.vMin.x, _template.bbox.vMin.y - Z_TO_H(heightZ), _template.bbox.vMax.x, _template.bbox.vMax.y );
	bbox.SaveSnapshot();
	bbox_cull = bbox;
	bbox_cull.SaveSnapshot();

	//set hue
	BYTE collvl = 255;
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

void CActor::Update(float dTime )
{
	// clean target pointer when target dies (should be done by AI?)
	/*
	if ( (pTarget != nullptr) && pTarget->IsPendingKill() )
	{
		pTarget->FreeRef();
		pTarget = nullptr;
	}
	*/

//#TEMP: watchdog for hanging actors
#if defined(_DEBUG) || defined(DEBUG)
	if ( bPendingKill )
	{
		fPendingKillTimer += dTime;
		if ( fPendingKillTimer > 10.0f )
		{
			ErrorBox( K_ERR_WARNING, L"Actor hanged!" );
		}
	}

#endif
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

	// Update actor AI
	c_AI->Update( *this, dTime );
	// now process the AI commands
	ProcessAICommands();
	// Move based on speeds
	DoMove( dTime );
	// Processes extra stuff before painting
	ProcessExtras();
	// Set actor animations based on behaviour
	ProcessAnimations();
	// Update all components after we have the final player position
	c_graphics->Update(*this, dTime);
	// compute weapon control before updating the weapons
	ComputeAttackStatus();
	// update weapon after updating the graphics component because it depends on mount points
	c_weapons->Update( *this, dTime );
	// now we check if the weapon shot and generate the bullets
	bool bShot = CheckShoot();
}

void CActor::Paint( ETexChannel eChannel /*= K_TEXCHAN_COLORMAP */ )
{
	// get angle from actual animation and not aim vector (because they might differ)
	EDir6 actang = c_graphics->GetEAngle();
	bool bFacingS = GetDir6VecN( actang ).y > 0 ? true : false;
	
	// see if we need to clip and on which side. We clip to wall borders when we push left-right against vertical walls not hidden by ceiling.
	// clip coords are a little hardcoded to look good
	bool bClipped = false;
	CTile* tll = level->Areas_GetTileAt(Vec2(pos.xy.x - K_TILE_SIZE_F, pos.xy.y));
	if ( tll != nullptr && tll->flags & K_TILEFLAG_WALLENDING_R )
	{
		__Painter().SetClipWorld( RectXYWH( tll->bbox.vMax.x + 1.0f, this->pos.xy_proj.y - 4.0f * K_TILE_SIZE_F, 4.0f * K_TILE_SIZE_F, 5.0f * K_TILE_SIZE_F ) );
		bClipped = true;
	}
	else
	{
		CTile* tlr = level->Areas_GetTileAt( Vec2( pos.xy.x + K_TILE_SIZE_F, pos.xy.y ) );
		if ( tlr != nullptr && tlr->flags & K_TILEFLAG_WALLENDING_L )
		{
			__Painter().SetClipWorld( RectXYWH( tlr->bbox.vMin.x - 4.0f * K_TILE_SIZE_F + 2.0f, this->pos.xy_proj.y - 4.0f * K_TILE_SIZE_F, 4.0f * K_TILE_SIZE_F, 5.0f * K_TILE_SIZE_F ) );
			bClipped = true;
		}
	}
	
	///--- do the actual painting ---
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

	// remove clipping if set above
	if ( bClipped )
	{
		__Painter().RemoveClip();
	}
}

VecProj CActor::GetWeaponMountWorld( int nHands, int mountIndex /*= 0 */ )
{
	// get mount position in screen space (from editor)
	Vec2 v_mount = c_graphics->GetMountPoint( nHands, mountIndex );
	// add weapon mount offset
	Vec2 v_wpn_off = c_weapons->GetCurWeapon()->_template.vMountOffset;
	v_wpn_off.x *= (float)c_graphics->GetFlipDirX();
	v_mount += v_wpn_off;

	VecProj vpRet = pos;
	vpRet.Set( pos.xyz.x + v_mount.x, pos.xyz.y, pos.xyz.z - H_TO_Z( v_mount.y ));
	return vpRet;
}

VecProj CActor::GetWeaponMuzzleWorld( int nHands, int mountIndex /*= 0 */ )
{
	// get mount position in screen space (from editor)
	Vec2 v_muzzle_vec = c_weapons->GetWeaponMuzzlePoint();
	VecProj vp_mount = GetWeaponMountWorld( nHands, mountIndex );
	// if animations are flipped we need to also flip the weapon vectors
	v_muzzle_vec.y *= (float)c_graphics->GetFlipDirX();
	// rotate weapon muzzle vector and add it to the projected position of the mount
	Matrix mrot;
	float aim_angle = UTMath::GetVectorAngle( vAim );
	MUMatRotZ( &mrot, aim_angle );
	MUVec2TransformCoord( &v_muzzle_vec, &v_muzzle_vec, &mrot );
	Vec2 muzzle_proj = vp_mount.xy_proj + v_muzzle_vec;
	// transform mount position from projected to 3d, knowing that it shoots at the heart height
	return { muzzle_proj.x, muzzle_proj.y + Z_TO_H(_template.heartZ), _template.heartZ };
}

VecProj CActor::GetCurWeaponMuzzleWorld( int mountIndex /*= 0 */ )
{
	CWeapon* wpn = c_weapons->GetCurWeapon();
	if ( wpn == nullptr )
		return GetPosHeart3D();
	int nHands = c_weapons->GetCurWeapon()->_template.nHands;
	//bool bDualWielding = weapon->_template.bDualWielding;

	return GetWeaponMuzzleWorld( nHands, 0 );
}

void CActor::EquipWeapon( EWpnSlot wpnSlot )
{
	LOG( "Equipped slot: %d", wpnSlot );
	const CWeapon* wpn = c_weapons->Equip( wpnSlot );
	// hide hands corresponding to current weapon mode
	// it always does the full thing even if already on the same weapon
	//#TODO: ar trebui facuta o functie separata care sa ia in considerare si behaviour curent daca ascunde arme sau nu?
	if ( wpn == nullptr || wpn->status == K_WPN_STATUS_UNKNOWN || wpn->_template.nHands <= 0)
		c_graphics->SetSkinFlags( *this, true, true );
	else if ( wpn->_template.nHands == 2 || wpn->_template.bDualWielding == true )
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
			eAttackStatus = c_AI->AIcommands.eAttackCommand;
			// only switch to alt weapon if we can shoot
			if ( c_AI->AIcommands.eAttackCommand == K_ACT_ATTACK_SHOOTING_ALT )
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
					if ( c_AI->AIcommands.eAttackCommand == K_ACT_ATTACK_MELEE )
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
	if ( !IsAlive() )
		eCurSlot = K_WPNSLOT_EMPTYHANDS;

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

void CActor::ProcessAICommands()
{
	//----------------------------------------
	//	EXECUTE - process AI output  
	//----------------------------------------
	///--- AI commands ---

	//save old crouch state
	bool bCrouchedOldState = bCrouched;

	//set crouch
	bCrouched = c_AI->AIcommands.bCrouched;
	///--- aiming ---
	// get aim vector from AI commands, if set
	if ( !UTMath::Vec2IsZero( c_AI->AIcommands.vAimVec ) )
		vAim = c_AI->AIcommands.vAimVec;

	///--- speed and movement ---
	if ( ( c_AI->AIcommands.bThrust ) && ( !UTMath::Vec2IsZero( c_AI->AIcommands.vMoveDir ) ) )
	{
		//add speed
		float fspeed = _template.fSpeedMove;
		if ( c_AI->AIcommands.bRunning )
			fspeed = _template.fSpeedRun;

		// set final speed (normalize direction and multiply with linear speed)
		MUVec2Norm( &speed, &c_AI->AIcommands.vMoveDir );
		speed *= fspeed;
	}
	else
	{
		speed = Vec2( 0.0f, 0.0f );
	}

	//comanda culoare
	if ( c_AI->AIcommands.nColor != 0 )
	{
		color = c_AI->AIcommands.nColor;
	}

	//death elements 
	if ( GetCurrentBehavior() == AI_BEHAVIOR_DEAD )
	{
		switch ( c_AI->AIcommands.nDeathCommand )
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
					//__Particles().GenerateEnemySoftGib( pos.xy_proj, 0xff32a7fa, K_PART_LAYER_RT_FRONT_NRM );
				}
				else
				{
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
						/*
						for ( int ll = 0; ll < 6; ll++ )
						{
							level->AddDoofer( K_DOOFER_MEAT, AABB::GetRandomPointInBox( genbox ), &Vec2( randfloatsgn( 50.0f ) + bulletSpeed.x * 50.0f, -130.0f - randfloat( 100.0f ) ), &g_vecGravityOld, nSubType );
						}
						//goes straight down to stain the floor
						level->AddDoofer( K_DOOFER_MEAT, GetPosHeart(), &Vec2( 200.0f, 50.0f ), &g_vecGravityOld, nSubType );
						level->AddDoofer( K_DOOFER_MEAT, GetPosHeart(), &Vec2( -200.0f, 50.0f ), &g_vecGravityOld, nSubType );
						*/
						//human blood gibs particle
//						__Particles().AddParticle( ANM_PARTICLES_SPR_HUMAN_SPLAT_MED, true, 0, &pos.xy_proj, nullptr, nullptr, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, dwCol, K_PART_LAYER_RT_FRONT_NRM );
					}
					else //small animals and stuff
					{
						/*
						for ( int ll = 0; ll < 2; ll++ )
						{
							level->AddDoofer( K_DOOFER_MEAT, AABB::GetRandomPointInBox( genbox ), &Vec2( randfloatsgn( 50.0f ) + bulletSpeed.x * 50.0f, -130.0f - randfloat( 100.0f ) ), &g_vecGravityOld );
						}
						*/
//						__Particles().AddParticle( ANM_PARTICLES_SPR_HUMAN_SPLAT_SMALL, true, 0, &pos.xy_proj, nullptr, nullptr, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xff671010, K_PART_LAYER_RT_FRONT_NRM );
					}
				}

				//players don't deallocate. They only become invisible.
				if ( _template.actorClass == K_ACT_CLASS_PLAYER )
				{
					fLife = 0.0f;
					bSkipRender = true;
					//reset physics
					speed = Vec2( 0.0f, 0.0f );
					vSpeedImpulse = Vec2( 0.0f, 0.0f );
					//move invisible body back to last safe pos
					//Vec2 vSpawnPos = level->m_arrPlayerLastSafePos[ nPlayerOrdinal ];
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
		c_AI->AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_EMPTY;
	}


}

void CActor::DoMove( float dTime )
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
		///a. compute actor movement vector
		Vec2 vNextMove = (speed + vSpeedImpulse) * dTime;
		///b. filter possible collisions
		//1. find bbox start and end union that includes all collisions when moving at high speeds
		CAABB destbox, srcbox;
		srcbox = bbox_floor.GetSnapshot();
		destbox = srcbox; 
		srcbox.Move( pos.xy );
		destbox.Move( pos.xy + vNextMove );
		// box unions to check all possible collisions
		CAABB boxUnion = AABB::Union( destbox, srcbox );
		boxUnion.Inflate( K_TILE_HSIZE_F, K_TILE_HSIZE_F );
		// bbox union in tile coords, including every touched tile
		RectXYXYi boxUnionTiles( floor( boxUnion.vMin.x / K_TILE_SIZE_F ), floor( boxUnion.vMin.y / K_TILE_SIZE_F ),
			ceil( boxUnion.vMax.x / K_TILE_SIZE_F ), ceil( boxUnion.vMax.y / K_TILE_SIZE_F ) );
		RectXYWHi boxUnionTilesWH( boxUnionTiles.x1, boxUnionTiles.y1, boxUnionTiles.x2 - boxUnionTiles.x1 + 1, boxUnionTiles.y2 - boxUnionTiles.y1 + 1 );

		// keeps a list of all boxes that might be colliding
		tempCollBoxList.Clear();

		/// BROAD PHASE SWEEP (find all POSSIBLE collision objects)

		//add boxes from collision shapes
		for ( int kk = 0; kk < level->m_arrColShapes.GetSize(); kk++ )
		{
			if ( !level->m_arrColShapes[ kk ]->IsAlive() )
				continue;

			if ( !boxUnion.Intersects( level->m_arrColShapes[ kk ]->bbox ) )
				continue;

			// save box for later collision checl
			if ( level->m_arrColShapes[ kk ]->collFlags != K_DIRFLAG_NONE )
			{
				tempCollBoxList.Add( level->m_arrColShapes[ kk ]->bbox );
			}
		}
		//add boxes from tiles
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
			srcbox = bbox_floor.GetSnapshot();
			srcbox.Move( pos.xy );
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

				// Calculate the collision tangent (vector used to slide the object that collided)
				// normala is actually the tangent
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

					CAABB newboxsrc = bbox_floor.GetSnapshot();
					CAABB newboxdest = newboxsrc;
					newboxsrc.Move( pos.xy );
					newboxdest.Move( pos.xy + vNextMove );
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
	if ( !Rects::PointInRect( pos.xy, level->m_levelAABB ) )
	{
		level->KillActor( this );
	}

	//set final position
	SetPos( Vec3( pos.xy.x, pos.xy.y, 0.0f ) );
	// save last position in pos_last (SetPos does but we already altered pos)
	pos_last = vPosIni;

}

void CActor::ProcessExtras()
{
	///--- set current area if null or changed after updating the position
	if ( (pArea == nullptr) || (!pArea->AABBbounds.PointIn( pos.xy )) )
	{
		pArea = level->Areas_GetAt( pos.xy );
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
			if ( pClosestTouchable.GetTo() != pNewTouchable )
			{
				ClearActionsList();
				CSmartLink::SetLink( &pClosestTouchable, pNewTouchable );
			}
		}
		else
		{
			if ( pClosestTouchable.IsSet() )
			{
				ClearActionsList();
				CSmartLink::RemoveLink( &pClosestTouchable );
			}
		}

		// check touch/interact
		if ( ( c_AI->AIcommands.bInteract ) && ( pClosestTouchable.IsSet() ) )
		{
			BuildActionsList();
			if ( arrInteractOptions.Count() > 0 )
			{
				eInteractState = K_STATE_READY;
				nInteractOptionsSelIdx = 0;
			}
		}
	}


}

void CActor::ProcessAnimations()
{
	Vec2 vAimN( 0.0f, 0.0f );
	MUVec2Norm( &vAimN, &vAim );
	// see if he's walking backwards
	float fSpeedDot = MUVec2Dot( &vAim, &speed );

	if ( GetCurrentBehavior() == AI_BEHAVIOR_DEAD )
	{
		c_graphics->SetAnimOnce( K_ACT_ANIM_DIE );
		return;
	}
	
	if ( UTMath::Vec2AlmostZero( speed ) )
	{
		c_graphics->SetAnimOnce( K_ACT_ANIM_IDLE );
		c_graphics->SetAnimDirection( false );
	}
	else
	{
		if(c_AI->AIcommands.bRunning)
			c_graphics->SetAnimOnce( K_ACT_ANIM_RUN );
		else
			c_graphics->SetAnimOnce( K_ACT_ANIM_WALK );
		// change animation direction if walking back
		if ( fSpeedDot < 0.0f )
			c_graphics->SetAnimDirection( true );
		else
			c_graphics->SetAnimDirection( false );
	}
}

bool CActor::CheckShoot()
{
	//#TODO: must add support for weapon scripts on shoot and empty
	CWeapon* weapon = c_weapons->GetCurWeapon();
	if ( ( weapon == nullptr ) || ( weapon->pOwner == nullptr ) || ( weapon->status == K_WPN_STATUS_UNKNOWN ) )
		return false;

	// only shoot on JUST_SHOT
	if ( weapon->status != K_WPN_STATUS_JUST_SHOT )
		return false;

	bool nHands = weapon->_template.nHands;
	bool bDualWielding = weapon->_template.bDualWielding;

	CActor* shooter = weapon->pOwner;
	Vec3 vFinalDir;
	MUVec3Norm( &vFinalDir, &c_weapons->GetWeaponAimVec() );
	//#TODO: add support for dual wielding
	VecProj vShootPos = shooter->GetWeaponMuzzleWorld( nHands, 0 );

	// checks if muzzle is inside the level, outside of collisions and walls
	//#OPTIMIZE: poate poate sa verifice direct in pathfinding map
	Vec2 vRetP( 0.0f, 0.0f ), vRetN( 0.0f, 0.0f );
	CTile* tl = level->SegmentTilesIntersectionEx( GetPosHeart3D().xy, vShootPos.xy, vRetP, vRetN, nullptr, pArea );
	if ( tl != nullptr )
	{
		//#TODO: ar trebui sa verifice si cu inamicii si cu alte entitati gen cutii, mese etc. Ar trebui sa spawneze particule cand tragi etc
		// ideal ar trebui sa simuleze ca ai tras, sa faca damage si toate cele ca sa nu tragi de dincolo de inamic cand e foarte aproape
		// idee: poate sa traga din heart pos dar sa fie invizibil glontul (set flag invisible)
		//!! de vazut daca trebuie facut ceva special dar nu cred.... poate sa fac glontul invizibil
	}

	EActorClass nFinalClass = shooter->_template.actorClass;
	//bullet has template class, set it to final class
	if ( weapon->_template.bulletTemplate.eClass != K_ACT_CLASS_ANY )
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
			float fSpreadAng = level->m_rand.RandFloatSgn( weapon->_template.fSpreadFOV );

			//vFinalDir.x = cos(fAimAng + fSpreadAng);
			//vFinalDir.y = sin(fAimAng + fSpreadAng);
			//D3DXVec2Normalize(&vFinalDir, &vFinalDir);

			level->ShootBullet( &tmplBullet, nFinalClass, shooter->GetUID(), vShootPos.xyz, vFinalDir );
		}

		// add shell
		if ( weapon->_template.nDropShellFrame >= 0 )
		{
			//level->AddDoofer( K_DOOFER_SHELL, weapon->pOwner->GetPosHeart(), &Vec2( (40.0f + randfloat( 30.0f )), -50.0f - randfloat( 20.0f ) ), &g_vecGravityOld, weapon->_template.nDropShellFrame );
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
		level->AddAIEvent( K_AIEVT_SOUND_THREAT, shooter->GetUID(), shooter->_template.actorClass, shooter->GetPosHeart(), weapon->_template.fSoundRadius );
	}

	return true;
}

void CActor::BuildActionsList()
{
	// ABOUT: 
	// - all objects will come with a particular list of actions that can be executed on them (eg: box comes with "open" and "investigate")
	// - shScriptActions - all objects have some keywords regarding their type or what class of actions you can execute on them. For example doors can be "door,door_locked" (see gameplaydef.xml)
	// - based on shScriptActions and actions defined in gameplaydef.xml we add actions that can be done on object. eg: open and knock work on DOOR,DOOR_LOCKED, breach on DOOR_LOCKED
	// - inventory objects get parsed and actions like "lockpick" can be added to "door, door_locked" if we have a lockpick in the inventory
	arrInteractOptions.Clear();
	if ( !pClosestTouchable.IsSet() )
		return;
	//1. get object specific actions
	for (int kk = 0; kk < pClosestTouchable.GetTo()->arrActions.Count(); kk++)
	{
		arrInteractOptions.Add(pClosestTouchable.GetTo()->arrActions[kk]);
	}
	//#TODO: 2. get inventory specific actions for targeted object class
	//#TODO: 3. get player class specific actions for targeted object class
}

void CActor::ClearActionsList()
{
	arrInteractOptions.Clear();
}


CBulletHitReturnData CActor::HitActor( CBullet *pBullet, Vec2* pvProjectileMomentum )
{
	CBulletHitReturnData retData;
	retData.eMaterial = this->_template.eMaterial;
	retData.bPenetratedShield = false;
	retData.bKilledTarget = false;
	retData.bArmorHit = false;

	if ( pBullet == nullptr )
	{
		ErrorBox( K_ERR_WARNING, L"CActor::HitActor invalid params!" );
		return retData;
	}

	bool bGoreEnabled = UTApp().m_Settings.bGoreEnabled;

	float fHitPointsTaken = pBullet->fDamage;
	float fActorInitialLife = this->fLife;
	//recon targeted enemies die 30% faster
	if ( this->cDamageOverTime.eType == CDamageOverTime::K_LVL_DoT_TARGETED )
	{
		if ( ( pBullet->actorClass == K_ACT_CLASS_PLAYER ) || ( pBullet->actorClass == K_ACT_CLASS_EXPLOSION ) )
		{
			//fVar1 contains the actual damage multiplier
			fHitPointsTaken += fHitPointsTaken * this->cDamageOverTime.fVar1;
		}
	}
	//recon targeted allies take less damage
	if ( this->cDamageOverTime.eType == CDamageOverTime::K_LVL_DoT_TARGETED_ALLY )
	{
		if ( pBullet->actorClass == K_ACT_CLASS_PLAYER )
			fHitPointsTaken -= fHitPointsTaken * 0.5f;
	}

	float fOldLife = this->fLife;
	if ( fOldLife > 0.0f )
	{
		//signal damage made by coloring them in red
		//this->nTookDamageFrames = 4;
	}

	float fBulletLostEnergy = 0.0f;
	float fLifeTaken = 0.0f; //cata viata ia din actor. Se foloseste doar local.
	float fShieldPointsTaken = 0.0f; //shield taken
	if ( fHitPointsTaken < 0.0f )	//kill actor command
	{
		fBulletLostEnergy = this->fLife + this->fArmor;
		fLifeTaken = fBulletLostEnergy;
		//daca am valoare negativa la hitpoints setam direct viata la valoarea respectiva
		this->fLife = fHitPointsTaken;
		this->varAIparams.SetVarINT32( L"nDeathCommand", K_LVL_ACT_DEATHCMD_SPLAT );

		this->fArmor = 0.0f;
	}
	else
	{
		fLifeTaken = fHitPointsTaken;
		bool bBulletStopped = false;
		//decidere directie shield vs directie projectileMomentum daca avem directie pe shield (sau shield all around)		
		if ( ( ( pBullet->nFlags & K_LVL_BULLET_FLAG_IGNORE_ARMOR ) == 0 ) && ( pvProjectileMomentum != null ) && ( this->fArmor > 0.0f ) )
		{
			int nActorAR = 1;
			//melee damage is treated differently
			if ( pBullet->nFlags & K_LVL_BULLET_FLAG_MELEE )
			{
				//melee ignores armor usually but if armor hase melee resistance then it takes first from the armor and then from life
				float fDmgToArmor = fHitPointsTaken * 1;
				fShieldPointsTaken = min( fDmgToArmor, this->fArmor );
				fLifeTaken = fHitPointsTaken - fShieldPointsTaken;

				bBulletStopped = true;
				retData.bPenetratedShield = true;
				retData.eMaterial = K_LVL_MATERIAL_FLESH;
			}
			else
			{
				if ( nActorAR < 0 ) //special case for human shield (hostage)
				{
					fShieldPointsTaken = min( this->fArmor, fHitPointsTaken );
					fLifeTaken = max( 0.0f, fHitPointsTaken - fShieldPointsTaken );
					bBulletStopped = false;
					retData.eMaterial = K_LVL_MATERIAL_FLESH;
				}
				else if ( pBullet->nArmorPiercingRating < nActorAR )
				{
					fLifeTaken = 0.0f;

					float fShieldPerc = max( 0.25f, 1.0f - ( nActorAR - pBullet->nArmorPiercingRating ) * 0.25f );
					fShieldPointsTaken = fHitPointsTaken * fShieldPerc;
					bBulletStopped = true;
					retData.eMaterial = K_LVL_MATERIAL_METAL;
					retData.bArmorHit = true;
				}
				else if ( pBullet->nArmorPiercingRating == nActorAR )
				{
					fLifeTaken = 0.0f;
					fShieldPointsTaken = fHitPointsTaken;
					bBulletStopped = true;
					retData.bPenetratedShield = true;
					retData.eMaterial = K_LVL_MATERIAL_METAL;
				}
				else
				{
					float fLifePerc = max( 1.0f, ( ( pBullet->nArmorPiercingRating - nActorAR ) * 0.25f ) );
					fLifeTaken = fHitPointsTaken * fLifePerc;
					fShieldPointsTaken = fHitPointsTaken;
					bBulletStopped = false;
					retData.bPenetratedShield = true;
					retData.eMaterial = K_LVL_MATERIAL_FLESH;
				}
			}

			//scade shield points din armor
			this->fArmor -= fShieldPointsTaken;
			//took too much armor? get extra armor taken from life
			if ( this->fArmor <= 0.0f )
			{
				fLifeTaken += -this->fArmor;
				this->fArmor = 0.0f;
			}

			//--- calculam energia ramasa in glont ---
			if ( bBulletStopped )
			{
				//bullet loses all its energy so it dies
				fBulletLostEnergy = pBullet->fDamage;
			}
			else
			{
				fBulletLostEnergy = fShieldPointsTaken + min( fLifeTaken, max( this->fLife, 0.0f ) );
			}
		}
		else //no shield
		{
			//already dead bodies stop bullets
			if ( ( this->fLife <= 0.0f ) && ( this->GetCurrentBehavior() == AI_BEHAVIOR_DEAD ) )
				bBulletStopped = true;

			fBulletLostEnergy = fShieldPointsTaken + min( fLifeTaken, max( this->fLife, 0.0f ) );
		}
		//when shooting a dead body take a maximum of 10% energy from the bullet
		//daca nu luam energia asta in momentul in care glontul tras se duce in cadavru nu il strapunge si timp de mai multe frames sta pe loc si face zgomot de damage
		if ( ( this->fLife <= 0.0f ) && ( fBulletLostEnergy <= 0.0f ) )
			fBulletLostEnergy = this->_template.fLife * 0.1f;

		//transmit bullet momentum daca nu sunt under cover
		if ( ( this->_template.fMass > 0.0f ) && ( pvProjectileMomentum ) )
		{
			this->vSpeedImpulse += *pvProjectileMomentum / this->_template.fMass;
		}

		//subtract life	if no invincibility
		if ( this->cDamageOverTime.eType == CDamageOverTime::K_LVL_DoT_INVINCIBLE )
			fLifeTaken = 0.0f;

		if ( this->_template.actorClass == K_ACT_CLASS_PLAYER )
		{
			float fDecLife = fLifeTaken;

#if defined(ENABLE_PLAYER_INVINCIBILITY)
			fDecLife = 0.0f;
#endif

			this->fLife -= fDecLife;
			//analytics
			level->m_arrStats[K_LVL_STATS_PL1_DAMAGE_TAKEN + this->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT] += ( int ) ceil( fDecLife );
		}
		else
		{
			this->fLife -= fLifeTaken;


#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
			//LOG(L"--> Damaged %s: fLifeTaken:%.2f fArmorTaken:%.2f(AR:%d) bIgnoreArmor:%d <--", this->templateActor.shName.text, fLifeTaken, fShieldPointsTaken, this->templateActor.nArmorRating, (pBullet->nFlags & K_LVL_BULLET_FLAG_IGNORE_ARMOR));
#endif
		}
		//save last damager UID
		this->nLastDamageTakenFromUID = pBullet->ownerUID;
		//if he's still alive and you took enough of it's life say verse
		if ( ( this->fLife > 0.0f ) && ( fLifeTaken >= this->_template.fLife * 0.1f ) )
			//			PlayActorSoundVerse(actor, K_LVL_ACT_VERSE_TAKING_DAMAGE, true);

					//life left in it?
			if ( this->fLife > 0.0f )
			{
				//mesaj LOW_HEALTH - la 10% din viata originala
				/*
				float fLifeLowLimit = this->_template.fLife * 0.1f;
				if ( ( this->fLife < fLifeLowLimit ) && ( this->fLife + fLifeTaken >= fLifeLowLimit ) )
				{
					AddAIEvent( K_AIEVT_LOW_HEALTH, 0, pBullet->actorClass, this->GetPosHeart(), 10000.0f, 0.6f, this->GetUID() );
				}
				*/

				//adaugam si stun
				if ( this->fStunTimer < pBullet->fStunDuration )
				{
					SetStun( pBullet->fStunDuration );
				}
			}
	}

	//event got_hit
	if ( ( this->fLife > 0.0f ) && ( this->_template.actorClass > K_ACT_CLASS_PLAYER ) )
	{
		//adaug eventuri de GOT_HIT doar pe clasele HUMAN, cand sunt lovite de catre player
		//find shooter pos. defaults on pos based on bullet speed
		/*
		Vec3 evtpos = this->GetPosHeart();
		if (pvProjectileMomentum != null)
			evtpos -= *pvProjectileMomentum;

		CActor* pPlayer = GetPlayerByUID(pBullet->ownerUID);
		if (pPlayer)
			evtpos = pPlayer->GetPosHeart();

		//only add "got hit" events for enemy classes
		if (pBullet->actorClass >= K_ACT_CLASS_EXPLOSION)
		{
			AddAIEvent(K_AIEVT_GOT_HIT, pBullet->ownerUID, pBullet->actorClass, evtpos, -1.0f, 1.2f, this->GetUID());
		}
		*/
	}

	//set dead AI on humans
	if ( ( this->fLife <= 0.0f ) && ( this->_template.eMaterial == K_LVL_MATERIAL_FLESH ) )
	{
		//give strategic points on death
		if ( ( fOldLife > 0.0f ) && ( this->_template.actorClass >= K_ACT_CLASS_ENEMY ) )
		{
			//you get points if enemy killed by player or explo
			if ( ( ( pBullet->actorClass == K_ACT_CLASS_PLAYER ) || ( pBullet->actorClass == K_ACT_CLASS_EXPLOSION ) ) && ( this->_template.actorClass != K_ACT_CLASS_PLAYER ) )
			{
				if ( this->UID != pBullet->ownerUID )
				{
					level->GiveStrategicPoints( 1.0f, &Vec2( this->bbox.vCenter.x, this->bbox.vMin.y ) );
				}
			}
		}

		//cadavers get pushed more by kicking them
		if ( ( fOldLife > 0.0f ) && ( this->_template.fMass > 0.0f ) && ( pvProjectileMomentum != nullptr ) )
			this->vSpeedImpulse += K_LVL_DEAD_BODY_BULLET_MOMENTUM_MULTIPLIER * ( *pvProjectileMomentum / this->_template.fMass );

		//erase shooting flags
		this->eAttackStatus = K_ACT_ATTACK_IDLE;

		bool bSplatActor = false;

		//very low life from the first hit? splat!
		if ( ( pBullet->nFlags & K_LVL_BULLET_FLAG_CAN_SPLAT ) && ( this->GetCurrentBehavior() != AI_BEHAVIOR_DEAD ) && ( pBullet->actorClass == K_ACT_CLASS_PLAYER ) && ( this->fLife < -this->_template.fLife * 0.5f ) )
		{
			bSplatActor = true;
			//if bullets lose power then only splat from close quarters
			if ( ( pBullet->fDamageLossPPx > 0.0f ) && ( ( pBullet->fLife / pBullet->fLife_ini ) < 0.9f ) )
				bSplatActor = false;
		}
		//grenades splat dead bodies
		if ( ( this->GetCurrentBehavior() == AI_BEHAVIOR_DEAD ) && ( pBullet->actorClass == K_ACT_CLASS_EXPLOSION ) && ( fLifeTaken >= this->_template.fLife ) )
			bSplatActor = true;
		//if dead but you keep kicking him it explodes
		if ( ( this->GetCurrentBehavior() == AI_BEHAVIOR_DEAD ) && ( pBullet->nFlags & K_LVL_BULLET_FLAG_CAN_SPLAT ) && ( this->fLife < -this->_template.fLife ) )
			bSplatActor = true;
		//if lucky cancel splat
		if ( level->RNG().RandInt( 100 ) <= 10 )
		{
			bSplatActor = false;
			this->fLife = 0.0f;
		}

		//--- generate blood splats on death ---
		if ( ( fOldLife > 0.0f ) && ( this->_template.eMaterial == K_LVL_MATERIAL_FLESH ) )
		{
			if ( bGoreEnabled )
			{
				if ( ( pBullet->nFlags & K_LVL_BULLET_FLAG_NO_DECALS ) == 0 )
				{
					//level->AddDecal_BloodSplat( this->GetPosHeart(), true, this->_template.actorClass );
				}
			}

			retData.bKilledTarget = true;
			//say shooter verse
			CActor* pShooter = level->GetActorByUID( pBullet->ownerUID );
			if ( pShooter != null )
			{
				//				PlayActorSoundVerse(pShooter, K_LVL_ACT_VERSE_KILL_MADE);
			}
		}

		//set splat command
		if ( bSplatActor )
		{
			this->varAIparams.SetVarINT32( L"nDeathCommand", K_LVL_ACT_DEATHCMD_SPLAT );
		}
	}

	//daca are coliziuni laterale anulez impulsul ca sa nu intre prin geometrie
	/*
	if (((this->collisionFlags & K_DIRFLAG_RIGHT) && (this->vSpeedImpulse.x > 0.0f)) ||
		((this->collisionFlags & K_DIRFLAG_LEFT) && (this->vSpeedImpulse.x < 0.0f)))
	{
		this->vSpeedImpulse.x = 0.0f;
	}
	*/

	retData.fPointsTaken = fBulletLostEnergy;
	return retData;
}

CBulletHitReturnData CActor::HitActor( float fDamage, UINT32 dwOwnerUID, EActorClass eOwnerClass, Vec2 *vDir /*= null*/, UINT32 dwBulletFlags /*= 0*/, int nArmorPiercingRating /*= 100*/, float fStunDuration /*= 0.0f*/ )
{
	CBullet bullet;

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	//LOG(L"-Damaged %s with %.2f", this->templateActor.shName.text, fDamage);
#endif

	bullet.fDamage = fDamage;
	bullet.ownerUID = dwOwnerUID;
	bullet.actorClass = eOwnerClass;
	bullet.nFlags = dwBulletFlags;
	//always pierce armor (by default)
	bullet.nArmorPiercingRating = nArmorPiercingRating;
	bullet.fStunDuration = fStunDuration;

	return HitActor( &bullet, vDir );
}

/*
* Sets STUN timer
*/
void CActor::SetStun( float fStunDuration )
{
	if ( ( this->_template.actorClass != K_ACT_CLASS_ENEMY ) && ( this->_template.actorClass != K_ACT_CLASS_FRIENDLY ) )
		return;

	if ( ( this->_template.eCaps & K_ACT_CAPS_NOT_A_TARGET ) != 0 )
		return;

	if ( fStunDuration > this->fStunTimer )
	{
		this->fStunTimer = fStunDuration;
	}

	bool bInterrupting = false;
	//reset actions
	if ( this->fStunTimer >= K_LVL_MIN_STUN_DIZZY_DURATION )
	{
		this->Weapons()->StopReloading();
		bInterrupting = true;
	}
	//reset actions
	this->eAttackStatus = K_ACT_ATTACK_IDLE;
	//stop moving
	if ( this->collisionFlags & K_DIRFLAG_DOWN )
		this->speed.x = 0.0f;

	//#TODO: let him know he got stunned
	//AddAIEvent(K_AIEVT_GOT_HIT, 0, (EActorClass)exploOwnerClass, pos, fStunRadius, fMaxStun + 0.5f, act->GetUID());

	//custom stun responses
	/*
	switch (this->GetCurrentBehavior())
	{
		//don't stun cadavers
		case AI_BEHAVIOR_DEAD:
		{
			this->fStunTimer = 0.0f;
		}
		break;
	}
	*/
}


