#include "dxstdafx.h"
#include "Level_doofers.h"

void CLevel::AddDoofer(EDooferType type, VecProj pos, Vec3 * speed, Vec3 * accel, int nSubType /*= 0*/)
{
	// skip gore stuff when turned off from settings
	bool bGoreEnabled = UTApp().m_Settings.bGoreEnabled;
	if ((!bGoreEnabled) && (type == K_DOOFER_MEAT))
		return;

	auto node = m_poolDoofers.Hire();
	if ( node == nullptr )
		return;
	
	CDoofer* doof = &node->m_data;
	doof->Reset();

	doof->pos = pos;
	doof->type = type;
	doof->nSubType = nSubType;

	switch ( type )
	{
		case K_DOOFER_SHRAPNEL_SMOKING:
		{
			// initialize physics 
			doof->c_pointPhys->SetActive( true );
			doof->c_pointPhys->SetSpeed( *speed );
			doof->c_pointPhys->SetAccel( *accel );
			doof->c_pointPhys->SetBounceEnabled( true );

			doof->spr.Init( &__Particles().m_sprCol, ANM_PARTICLES_SPR_SHRAPNEL, pos.xy_proj, randint(3) );
		}
		break;
		default:
		{
			ErrorBox( K_ERR_WARNING, L"Not implemented!" );
		}
		break;
	}
}

void CLevel::AddDoofer_Light(Vec2 pos, int nLightAnimIdx, float fDuration, float fFadeTime, DWORD color, float fScale)
{
	Vec3 vPos = Vec2ToVec3XY0(pos);
	auto node = m_poolDoofers.Hire();
	if ( node == nullptr )
		return;
	//set 
	if (node != nullptr)
	{
		node->m_data.Reset();
		//add simulation container
		/*
		node->m_data.physPt = m_poolPhysPts.Hire();
		if (node->m_data.physPt == nullptr)
		{
			ErrorBox(K_ERR_WARNING, L"AddProp_Light:We need more physics points!");
			m_poolDoofers.Dismiss(node);
			return;
		}
		
		//reset
		node->m_data.physPt->m_data.Reset();
		 */
		/*
		node->m_data.type = K_DOOFER_LIGHT;

		node->m_data.sprLight.Init(nLightAnimIdx, 0.0f, 0.0f, 0, color);
		node->m_data.fLightScaling = fScale * K_LVL_LIGHTRENDER_BSX_SCALING;
		node->m_data.bMakesLight = true;
		node->m_data.fLightDuration = fDuration;
		node->m_data.fLightFadeOut = fFadeTime;

		node->m_data.fTimer = 0.0f;
		*/
		//physics
		/*
		node->m_data.physPt->m_data.bFlagPhysicsEnabled = false;
		node->m_data.physPt->m_data.bFlagRotationEnabled = false;

		node->m_data.physPt->m_data.pos = vPos;
		node->m_data.physPt->m_data.speed = g_Vec3Zero;
		node->m_data.physPt->m_data.accel = g_Vec3Zero;
		*/
	}
}

void CLevel::AddDoofer_Explo( UINT32 exploNameHash, VecProj pos, UINT32 dwOwnerUID, int exploOwnerClass, Vec3 vExploDir, CAABB* exploAABB )
{
	CExplosionTemplate* explotemplate = GetTemplateExplosion( exploNameHash );
	if ( explotemplate == nullptr )
	{
		ErrorBox( K_ERR_WARNING, L"Explo template not found!" );
		return;
	}

	auto node = m_poolDoofers.Hire();
	if ( node == nullptr )
		return;

	//set 
	node->m_data.Reset();
	node->m_data.type = K_DOOFER_EXPLOSION;

	//default
	float fMaxDamage = explotemplate->fDamage;
	float fMaxStun = explotemplate->fStunDuration;
	float fDamageRadius = explotemplate->fDamageRadius;
	float fStunRadius = explotemplate->fStunRadius;
	float fMaxImpulse = explotemplate->fMaxImpulse;

	//add sound event
	if ( explotemplate->fSoundRadius > 0.0f )
		AddAIEvent( K_AIEVT_SOUND_THREAT, 0, (EActorClass)exploOwnerClass, pos.xy, explotemplate->fSoundRadius, 1.0f );

	if ( exploAABB == nullptr )
	{
		for ( int ll = 0; ll < explotemplate->nShrapnelCnt; ll++ )
		{
			Vec2 vPlane = m_rand.RandDirV2() * 60.0f;
			AddDoofer( K_DOOFER_SHRAPNEL_SMOKING, pos, &Vec3( vPlane.x, vPlane.y, 60.0f + randfloat( 100.0f ) ), &g_vecGravity );
		}
		//napalm
		/*
		for ( int ll = 0; ll < explotemplate->nNapalmCnt; ll++ )
		{
			float fdx = m_rand.RandFloatSgn( 60.0f );
			float fdy = -100.0f - m_rand.RandFloat( 120.0f );
			AddDoofer( K_DOOFER_FIRE_SOURCE, pos, &Vec2( fdx, fdy ), &g_vecGravityOld );
		}
		*/
	}
	else
	{
		//shrapnel
		for ( int ll = 0; ll < explotemplate->nShrapnelCnt; ll++ )
		{
			Vec2 vPlane = m_rand.RandDirV2() * 60.0f;
			Vec2 vRandOff = m_rand.RandVec2Sgn( exploAABB->vHalfSize.x, exploAABB->vHalfSize.y );
			AddDoofer( K_DOOFER_SHRAPNEL_SMOKING, pos.xyz + Vec3(vRandOff.x, vRandOff.y, 0.0f), &Vec3( vPlane.x, vPlane.y, 60.0f + randfloat( 100.0f ) ), &g_vecGravity);
		}
		//napalm
		/*
		for ( int ll = 0; ll < explotemplate->nNapalmCnt; ll++ )
		{
			float fdx = m_rand.RandFloatSgn( 60.0f );
			float fdy = -100.0f - m_rand.RandFloat( 120.0f );
			AddDoofer( K_DOOFER_FIRE_SOURCE, pos + m_rand.RandVec2Sgn( exploAABB->vHalfSize.x, exploAABB->vHalfSize.y ),
				&Vec2( fdx, fdy ), &g_vecGravityOld );
		}
		*/
	}

	//explo direction
	float fExploAng = UTMath::GetVectorAngle( Vec3XY( vExploDir ) );
	// generate effect if we have one
	if ( explotemplate->shFX.IsSet() )
	{
		GenerateEffect( explotemplate->shFX, pos.xy_proj, 1.0f );
	}

	//pointer to player that spawned the explosion, or null if it wasn't a player
	CActor* pPlayer = GetPlayerByUID( dwOwnerUID );

	//#IMPORTANT #TODO: should optimize in order to minimize the usage of UnobstructedLineOfSight
	//stun enemy and damage over time
	if ( ( fMaxStun > 0.0f ) || ( explotemplate->cDoT.eType != CDamageOverTime::K_LVL_DoT_NONE ) )
	{
		//find all actors and damage them (linearly)
		for ( int kk = 0; kk < m_arrActors.GetSize(); kk++ )
		{
			CActor* act = m_arrActors[kk];
			if ( ( !act->IsAlive() ) || ( act->_template.eCaps & K_ACT_CAPS_NOT_A_TARGET ) )
				continue;
			//never stun the hostages
			if ( ( act->_template.actorClass == K_ACT_CLASS_HOSTAGE ) && ( fMaxStun > 0.0f ) )
				continue;

			Vec2 vDir = act->GetPosHeart() - pos.xy;
			float fDist = MUVec2Len( &vDir );

			bool bDirectLine = IsLineOfSight( act->GetPosHeart(), pos.xy, act->pArea );

			if ( ( bDirectLine ) && ( explotemplate->cDoT.eType != CDamageOverTime::K_LVL_DoT_NONE ) && ( fDist < explotemplate->fDoTRadius ) )
			{
				//momentan nu pune DoT in functie de distanta ci pune uniform la toti din raza
				SetActorDoT( act, explotemplate->cDoT.eType, explotemplate->cDoT.fDuration, explotemplate->cDoT.fDamagePerSec, explotemplate->cDoT.eExcludedActClass, explotemplate->cDoT.eFilteredActClass, dwOwnerUID );
			}

			//no friendly stun
			if ( act->_template.actorClass != K_ACT_CLASS_ENEMY )
				continue;
			if ( fDist > fStunRadius )
				continue;
			// if stun is directional but we have wrong direction
			//if ((vExploDir.x != 0.0f) && (SIGN(vExploDir.x) != SIGN(vDir.x)))
//					continue;
			if ( !bDirectLine )
				continue;

			if ( ( act->fStunTimer < fMaxStun ) && ( fMaxStun > 0.0f ) )
			{
				act->SetStun( fMaxStun );
			}
		}
	}

	//do some damage
	if ( ( fMaxDamage > 0.0f ) && ( fDamageRadius > 0.0f ) )
	{
		int nBombFrags = 0;
		//find all actors and damage them (linearly)
		for ( int kk = 0; kk < m_arrActors.GetSize(); kk++ )
		{
			CActor* act = m_arrActors[kk];

			if ( ( !act->IsEnabled() ) || ( act->_template.eCaps & K_ACT_CAPS_NOT_A_TARGET ) )
				continue;
			//ignores specified classes
			if ( act->_template.actorClass == explotemplate->eIgnoreActorClass )
				continue;

			Vec2 vDir = act->GetPosHeart() - pos.xy;
			float fDist = MUVec2Len( &vDir );
			if ( fDist > fDamageRadius )
				continue;
			if ( !IsLineOfSight( act->GetPosHeart(), pos.xy, act->pArea ) )
				continue;
			// linear damage atten
			float fPercent = 1.0f - ( fDist / fDamageRadius );
			CLAMP( fPercent, 0.0f, 1.0f );
			//add momentum
			MUVec2Norm( &vDir, &vDir );
			vDir *= fPercent * fMaxImpulse;

			CBulletHitReturnData retdata{};
			retdata = act->HitActor( fPercent * fMaxDamage, dwOwnerUID, K_ACT_CLASS_EXPLOSION, &vDir, K_LVL_BULLET_FLAG_CAN_SPLAT, explotemplate->nArmorPiercingRating );
			//count only enemies
			if ( ( retdata.bKilledTarget ) && ( act->_template.actorClass >= K_ACT_CLASS_ENEMY ) )
				nBombFrags++;

			//#ACHIEVEMENTS: darwin award - player died from his own explosive
			if ( ( act->_template.actorClass == K_ACT_CLASS_PLAYER ) && ( !IsNetworkPlayer( act ) ) && ( act->fLife <= 0.0f ) && ( dwOwnerUID == act->UID ) &&
				( ( explotemplate->name.textHash == hash_EXPLO_GRENADE_GROUND ) || ( explotemplate->name.textHash == hash_EXPLO_GRENADE ) ||
				( explotemplate->name.textHash == hash_EXPLO_CHARGE ) || ( explotemplate->name.textHash == hash_EXPLO_CHARGE_INVISIBLE ) ) )
			{
				__Achievements().UnlockAchievement( ACH_DARWIN_AWARD );
			}

		}

		///--- check doors and windows breaking ---
		/*
		if ((fMaxDamage > 0.0f) && (fDamageRadius > 0.0f))
		{
			for (int kk = 0; kk < m_visibleList.logic_colShapesSpecial.Count(); kk++)
			{
				CCollisionShape* shape = m_visibleList.logic_colShapesSpecial.m_pData[kk];
				//breaks doors?
				if ((explotemplate->fDamageObjectsMultiplier > 0.0f) && (shape->AIstate == K_AI_STATE_COLL_BREAKABLE_DOOR))
				{
					if (shape->varAIparams[L"b_reinforced")->m_asINT32 != 0)
						continue;

					//loveste liniar
					Vec2 vDist = (shape->bbox.vCenter - pos);
					float fDist = MUVec2Len(&vDist);
					if (explotemplate->fDamageObjectsMultiplier <= 0.0f)
						continue;
					float fPercent = 1.0f - (fDist / (fDamageRadius * explotemplate->fDamageObjectsMultiplier));
					//too soft
					if (fPercent <= 0.0f)
						continue;
					//not straight line (can't check with center or it will fail because of the actual bbox)
					Vec2 vCheckPt = pos;
					vCheckPt.x -= (shape->bbox.vHalfSize.x + 2.0f) * SIGN(vDist.x);
					if (!IsLineOfSight(pos, vCheckPt))
						continue;
					//damage door
					shape->AIfvar1 -= (fPercent * fMaxDamage) * explotemplate->fDamageObjectsMultiplier;
					//was hit
					shape->AIvarBool1 = true;
					//save door explo direction
					shape->varAIparams.SetVarFloat(L"fForceDirX", SIGN(vDist.x));
					shape->varAIparams.SetVarINT32(L"bExploded", 1);
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
							shape->varAIparams.SetVarFloat(L"fForceDirX", 1000.0f * SIGN(shape->bbox.vCenter.x - pos.x));
						}
					}
				}
			}
		}
		*/

		//check grenade interaction AIs
		/*
		if ((fMaxDamage > 0.0f) && (bInteractAI) && (fDamageRadius > 0.0f))
		{
			for (int kk = 0; kk < m_visibleList.logic_props_closeby.Count(); kk++)
			{
				CProp * activ = m_visibleList.logic_props_closeby.m_pData[kk];
				if (activ->AIstate == K_AI_STATE_ACTIVE_SWINGING_FRONTOBJ)
				{
					//daca am activ swinging si e in raza grenadei
					Vec2 vDir = activ->pos - pos;
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
		*/
	}

	//damage over time

}



void CLevel::UpdateDoofers(float dTime)
{
	RectXYWH camrect = m_camLevelToRT.GetCamWorldAABB();
	RectXYWH camrect_larger = camrect;
	camrect_larger.Inflate(2.0f * K_TILE_SIZE);

	for(auto node : m_poolDoofers)
	{
		//salvez locatia urmatoare ca s apot avansa pe ea
		CDoofer* prop = &node->m_data;
		prop->Update( dTime, ( *this ) );

		bool killprop = false;
		//generic updates
		prop->fLightTimer += dTime;

		switch (prop->type)
		{
			case K_DOOFER_SHELL:
			{
			}
			break;

			case K_DOOFER_MEAT:
			{
				DWORD dwCol = 0xff671010;
				if (prop->nSubType != 0) //zombies green blood
					dwCol = 0xff82b600;
				/*
				if (m_Timers.Tick(50))
				{
					__Particles().AddParticle(ANM_PARTICLES_SPR_BLOOD, false, randint(5), &Vec2(prop->physPt->m_data.pos.x + randfloatsgn(5.0f), prop->physPt->m_data.pos.y + randfloatsgn(5.0f)),
						&Vec2(0.0f, 20.0f), &(prop->physPt->m_data.speed / (5.0f + randfloat(4.0f))), 0.6f, 1.0f, 0.0f, randfloat(PI), randfloatsgn(2.0f), 0.1f, 0.1f, dwCol, K_PART_LAYER_RT_BACK_NRM);
				}
				*/
			}
			break;

			case K_DOOFER_SHRAPNEL_SMOKING:
			{
				//update sprite
				prop->spr.Update(dTime);
				//add smoke
				if (m_Timers.Tick(100) && (!prop->c_pointPhys->bContacting))
				{
					float fAng = randfloat(DOUBLE_PI);
					__Particles().AddParticle( ANM_PARTICLES_SPR_SMOKESWIRL1, true, randint( 2 ), &( prop->pos.xy_proj + randVec2sgn( 3.0f, 3.0f ) ), nullptr, nullptr, 3.0f, 0.4f + randfloat( 0.2f ), 0.0f,
						randfloat( DOUBLE_PI ), 0.0f, 0.32f, 0.0f, 0xaa2a2626, K_PART_LAYER_NORMAL );
						
				}
				/*
				if (node->m_data.fTimer > 0.0f)
				{
					node->m_data.fTimer -= dTime;
					if ((m_Timers.Tick(80)) && (!prop->physPt->m_data.bContacting))
					{
						__Particles().GenerateFireRing(prop->physPt->m_data.pos, 2, 8.0f, 10.0f, K_PART_LAYER_NORMAL);
					}
					//some secondary explosions too
					if ((m_Timers.Tick(120)) && (randompercent(50.0f)))
					{
						__Particles().AddParticle(ANM_PARTICLES_SPR_FIRECRACKER1 + randint(2), true, 0, &prop->physPt->m_data.pos,
							NULL, NULL, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_NORMAL);
					}
				}
								  */
				// kill it if it touches the floor or gets outside the level area
				//if ( !prop->c_pointPhys->bIsActive )
//					killprop = true;
				if ( ( prop->c_pointPhys->bContacting ) && ( prop->c_pointPhys->contactNormal.z != 0.0f ) )
					killprop = true;

				if(killprop)
				{
					killprop = true;
					//smoke puff when dead
					float anm = randint( 3 );
					int nAnmId = ( anm == 0 ) ? ANM_PARTICLES_SPR_SMOKEPART1 : ( anm == 1 ) ? ANM_PARTICLES_SPR_SMOKEPART2 : ANM_PARTICLES_SPR_SMOKEPART3;
					__Particles().AddParticle(nAnmId, true, 0, &prop->pos.xy_proj, nullptr, nullptr, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0x882a2626, K_PART_LAYER_NORMAL);
				}


				// update sprite pos				
				prop->spr.pos = prop->pos.xy_proj;
			}
			break;
			case K_DOOFER_LIGHT:
			{
				prop->fTimer += dTime;
				//kill on timing out
				if (prop->fTimer >= prop->fLightDuration)
					killprop = true;
			}
			break;
			case K_DOOFER_EXPLOSION:
			{
				prop->fTimer -= dTime;
				if (prop->fTimer <= 0.0f)
					killprop = true;
			}
			break;
		}
		// release it if dead
		if (killprop)
		{
			//release la nodul de fizica !!!
			//m_poolPhysPts.Dismiss(prop->physPt);
			//si eliberez glontul
			m_poolDoofers.Dismiss(node);
		}
	}
}

void CLevel::PaintDoofers( eLVLRenderPass pass )
{
	//#HACK: only render on normal pass
	if ( pass != K_LVL_RP_COLORS )
		return;

	Mat mattrans;

	for(auto node : m_poolDoofers)
	{
		auto doof = &node->m_data;
		switch (doof->type)
		{
			case K_DOOFER_FIRE_SOURCE:
			{
			}
			break;
			case K_DOOFER_SHELL:
			{
				/*
				node->m_data.spr.pos = node->m_data.physPt->m_data.pos;
				node->m_data.spr.currentFrame = node->m_data.nSubType * 4 + (int(node->m_data.spr.pos.x * 3.0f) % 4);
				node->m_data.spr.paint_firstModule(&m_sprProps);
				*/
			}
			break;
			case K_DOOFER_SHRAPNEL_SMOKING:
			{
				doof->spr.PaintFModule(0);
			}
			break;
			case K_DOOFER_MEAT:
			{
				/*
				node->m_data.spr.pos = node->m_data.physPt->m_data.pos;
				node->m_data.spr.paint_firstModule(&m_sprProps);
				*/
			}
			break;
		}
	}
}

CDoofer::CDoofer() : type( K_DOOFER_NOT_SET ), nSubType( 0 ), fTimer( 0.0f ), fSize( 1.0f ),
bMakesLight( false ), fLightDuration( 0.0f ), fLightFadeOut( 0.0f ), fLightScaling( 1.0f ), fLightTimer( 0.0f ),
bVar1( false ), nIntVar1( 0 )
{
	// by default physics is off
	c_pointPhys = new CPointPhysComponent( false );
	c_pointPhys->SetActive( false );
}

CDoofer::~CDoofer()
{
	SAFE_DELETE( c_pointPhys );
}

void CDoofer::Reset()
{
	type = K_DOOFER_NOT_SET; nSubType = 0; fTimer = 0.0f; fSize = 1.0f;
	bMakesLight = false; fLightDuration = 0.0f; fLightFadeOut = 0.0f; fLightScaling = 1.0f; fLightTimer = 0.0f;

	// by default doofers have no physics
	c_pointPhys->Reset( false );
	spr.SetAnim( -1 );
}

void CDoofer::Update( float dTime, CLevel & level )
{
	if ( c_pointPhys != nullptr )
	{
		c_pointPhys->Update( pos, dTime, level );
	}
}
