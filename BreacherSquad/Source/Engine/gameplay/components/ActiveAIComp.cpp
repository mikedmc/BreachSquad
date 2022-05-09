#include "dxstdafx.h"
#include "ActiveAIComp.h"

CActiveAIComponent::CActiveAIComponent()
{

}

CActiveAIComponent::~CActiveAIComponent()
{

}

bool CActiveAIComponent::Update( IActiveInterface& active, float dTime, CLevel& level )
{
	bool bProcessedState = true;
	if ( active.AIstate == K_AI_STATE_UNDEFINED )
		return true;

	switch ( active.AIstate )
	{
		case K_AI_STATE_ACTIVE_BOMB:
		{
			CProp* prop = dynamic_cast< CProp* >( &active );
			if ( prop == nullptr )
				break;
			// only decrease bomb timer if playing (not on level results)
			if ( level.m_levelState != K_LVL_STATE_PLAYING )
				break;

			float fOldTimer = mem.AItimer1;
			mem.AItimer1 -= dTime;
			//m_interfaceIGM.SetBombTimer(mem.AItimer1);

			//--- sounds ---
			if ( mem.AItimer1 > 15.0f )
			{
				if ( floor( fOldTimer ) > floor( mem.AItimer1 ) )
				{
					//SND_PLAY(SNDIDX_BOMBBEEP);
				}
			}
			else
			{
				if ( level.m_Timers.Tick( 250 ) )
				{
					//SND_PLAY(SNDIDX_BOMBBEEP);
				}
			}

			if ( mem.AItimer1 <= 0.0f )
			{
				//m_interfaceIGM.SetBombTimer(0.0f);
				//add some explosions so everybody will die
				level.AddDoofer_Explo( hash_EXPLO_LARGE_XL, active.pos.xy, active.UID, K_ACT_CLASS_EXPLOSION );
				level.AddDoofer_Explo( hash_EXPLO_LARGE_XL, active.pos.xy + Vec2( 32.0f, 0.0f ), active.UID, K_ACT_CLASS_EXPLOSION );
				level.AddDoofer_Explo( hash_EXPLO_LARGE_XL, active.pos.xy - Vec2( 32.0f, 0.0f ), active.UID, K_ACT_CLASS_EXPLOSION );

				//__Particles().AddParticle( ANM_PARTICLES_SPR_EXPLO_ROUND_XL, true, 0, &active.pos.xy, NULL, NULL, 1.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM );

				prop->sprite.SetAnim( "BOMB_EXPLODED" );

				level.SetLevelState( K_LVL_STATE_MISSION_FAILED, STR_BOMB_EXPLODED );
			}
		}
		break;
		case K_AI_STATE_ACTIVE_AMMO_BOX:
		{
			CProp* prop = dynamic_cast< CProp* >( &active );
			if ( prop == nullptr )
				break;

			int nAmmoLeft = active.varAIparams[L"n_ammoLeft"].m_asINT32;
			prop->sprite.frameIdx = nAmmoLeft;

			//fade out
			if ( nAmmoLeft <= 0 )
			{
				mem.AItimer1 -= dTime;
				if ( mem.AItimer1 <= 0.0f )
				{
					active.Kill();
				}
				//color
				float fAlpha = LIMIT( mem.AItimer1, 0.0f, 1.0f );
				active.color = DW_COLORALPHA( active.color_ini, fAlpha );
			}
		}
		break;
		case K_AI_STATE_ACTIVE_HEALTH_BOX:
		{
			CProp* prop = dynamic_cast< CProp* >( &active );
			if ( prop == nullptr )
				break;
			int nHealthLeft = active.varAIparams[L"n_healthLeft"].m_asINT32;
			prop->sprite.frameIdx = nHealthLeft;

			//fade out
			if ( nHealthLeft <= 0 )
			{
				mem.AItimer1 -= dTime;
				if ( mem.AItimer1 <= 0.0f )
				{
					active.Kill();
				}
				//color
				float fAlpha = LIMIT( mem.AItimer1, 0.0f, 1.0f );
				active.color = DW_COLORALPHA( active.color_ini, fAlpha );
			}
		}
		break;
		case K_AI_STATE_ACTIVE_TEAM_TELEPORTER_2FRAMES:
		{
		}
		break;

		case K_AI_STATE_ACTIVE_DOOR_SECTION:
		{
			mem.AItimer1 = 0.0f;
		}
		break;

		case K_AI_STATE_ACTIVE_DOORFACE_AUTOCLOSE:
		{
			CProp* prop = dynamic_cast< CProp* >( &active );
			if ( prop == nullptr )
				break;
			//keep door open (AIvar1 contine frame-ul default) - set frame
			prop->sprite.frameIdx = prop->fid_ini.frameIdx;
			if ( mem.AItimer1 > 0.0f )
			{
				mem.AItimer1 -= dTime;

				bool bDontChangeFrames = ( bool ) ( active.varAIparams[L"b_DontChangeFrames"].m_asBool );
				if ( !bDontChangeFrames )
				{
					prop->sprite.frameIdx++;
				}

				if ( mem.AItimer1 < 0.0f )
					mem.AItimer1 = 0.0f;
			}

			//open/close sounds
			if ( ( mem.AIvarBool1 == false ) && ( mem.AItimer1 > 0.0f ) )
			{
				//just opened
				CVariant cvc = active.varAIparams[L"s_openSnd"];
				if ( cvc.eType == CVariant::K_ARGTYPE_STRING )
				{
					int sndidx = UTGetSoundManager().getSndIdx( cvc.m_strArg.textHash );
					SND_PLAY_POSITIONAL( sndidx, active.pos.xy );
				}
				//on open script
				cvc = active.varAIparams[L"s_ScriptOnOpen"];
				if ( cvc.eType == CVariant::K_ARGTYPE_STRING )
				{
					UTGetScriptManager().StartScript( cvc.m_strArg.textHash, active.UID );
				}

				mem.AIvarBool1 = true;
			}
			else if ( ( mem.AIvarBool1 == true ) && ( mem.AItimer1 <= 0.0f ) )
			{
				//just closed
				CVariant cvc = active.varAIparams[L"s_closeSnd"];
				if ( cvc.eType == CVariant::K_ARGTYPE_STRING )
				{
					int sndidx = UTGetSoundManager().getSndIdx( cvc.m_strArg.textHash );
					SND_PLAY_POSITIONAL( sndidx, active.pos.xy );
				}
				//on close script
				cvc = active.varAIparams[L"s_ScriptOnClose"];
				if ( cvc.eType == CVariant::K_ARGTYPE_STRING )
				{
					UTGetScriptManager().StartScript( cvc.m_strArg.textHash, active.UID );
				}
				mem.AIvarBool1 = false;
			}

		}
		break;

		case K_AI_STATE_ACTIVE_EXPLO_TRAP:
		{
		}
		break;
		case K_AI_STATE_ACTIVE_CHECKPOINT:
		{
			for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
			{
				if ( level.pPlayerActor[kk] == null )
					continue;
				if ( level.pPlayerActor[kk]->bbox.Intersects( active.bbox ) )
				{
					active.Touch( level.pPlayerActor[kk]->GetUID(), dTime );
					//save checkpoint
					level.vLastSpawnPoint = active.pos.xy;
					break;
				}
			}
		}
		break;


		case K_AI_STATE_COLL_KILL_ACTORS:
		{
		}
		break;

		case K_AI_STATE_COLL_BREAKABLE_WINDOW:
		{
			if ( mem.AIfvar1 <= 0.0f )
			{
				// set broken door anim
				if ( active.pTarget != nullptr )
				{
					//trebuie sa pointeze spre un CActive neaparat
					CProp* winact = dynamic_cast< CProp* >( active.pTarget );
					if ( winact == null )
					{
						ErrorBox( K_ERR_WARNING, L"K_AI_STATE_COLL_BREAKABLE_WINDOW bad cast to CActive" );
						break;
					}

					winact->sprite.frameIdx++;
					//reset object script and interact
					winact->arrActions.Clear();

					//generate particles
					float dirx = SIGN( active.varAIparams[L"fForceDirX"].m_asFloat );
					for ( int ll = 0; ll < 20; ll++ )
					{
						Vec2 ppos = AABB::GetRandomPointInBox( active.bbox );
						//__Particles().AddParticle( ANM_PARTICLES_SPR_GLASS_SHARDS, false, randint( 5 ), &ppos, &g_vecGravityOld, &Vec2( dirx * (60.0f + randfloat( 60.0f )), -40.0f + randfloatsgn( 50.0f ) ), 0.3f + randfloat( 0.2f ), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM );
					}
					//sound
					//SND_PLAY_POSITIONAL_RAND2(SNDIDX_WINDOWBREAK1, SNDIDX_WINDOWBREAK2, colshape->bbox.vCenter);
				}
				//destroy collision box
				active.Kill();
				//force hidden here to avoid collisions after death
				active.SetEnabled( false );
			}
		}
		break;
		case K_AI_STATE_COLL_BREAKABLE_DOOR:
		{
			float fForceDirX = 0.0f;
			if ( mem.AIvarBool1 ) //was hit?
			{
				//erase hit flag  (speed optimization)
				mem.AIvarBool1 = false;
				//set shake timer
				mem.AItimer1 = 1.0f;
				//just set fForeceDirX to something in order to make it get hit
				CVariant cvar = active.varAIparams[L"fForceDirX"];
				if ( cvar.eType == CVariant::K_ARGTYPE_FLOAT )
				{
					fForceDirX = cvar.m_asFloat;
					active.varAIparams.DeleteVar( L"fForceDirX" );
				}
			}
			float dirx = SIGN( fForceDirX );
			//if hit
			if ( fForceDirX != 0.0f )
			{
				//generate particles
				for ( int ll = 0; ll < 30; ll++ )
				{
					Vec2 ppos = AABB::GetRandomPointInBox( active.bbox );
					//__Particles().AddParticle( ANM_PARTICLES_SPR_WOODEN_SPLINTERS, false, randint( 6 ), &ppos, &g_vecGravityOld, &Vec2( dirx * (100.0f + randfloat( 60.0f )), -40.0f + randfloatsgn( 50.0f ) ), 0.3f + randfloat( 0.2f ), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM );
				}
				// add noise event behind the door
				Vec2 sndpos1 = Vec2( active.bbox.vCenter.x + dirx * ( active.bbox.vHalfSize.x + 2.0f ), active.bbox.vCenter.y );
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
						AddProp_Explo(hash_EXPLO_STUN_INVISIBLE, secondExploPos, 0, K_ACT_CLASS_PLAYER);
					}
					else
					{
						//trebuie setat si pe else pentru ca poate veni deja flipat din editor
						dooract->flipX = false;

						Vec2 secondExploPos(colshape->bbox.vCenter.x + (colshape->bbox.vHalfSize.x + 1.0f), colshape->bbox.vCenter.y);
						AddProp_Explo(hash_EXPLO_STUN_INVISIBLE, secondExploPos, 0, K_ACT_CLASS_PLAYER);
					}

					__Particles().GenerateDoorBreak(colshape->bbox.vCenter, Vec2(dirx, 0.0f), K_PART_LAYER_RT_FRONT_NRM);
					//sound
					//SND_PLAY_POSITIONAL(SNDIDX_DOOR_BREAK, colshape->bbox.vCenter);
					//analytics locale
					CVariant* cvexploded = active.varAIparams[L"bExploded");
					if ((cvexploded->m_type == CVariant::K_ARGTYPE_INT32) && (cvexploded->m_asINT32 != 0))
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
			CVariant* emittervc = active.varAIparams.GetVariantByNameHash(hash_v_emitterPtr);
			if (emittervc->m_type == CVariant::K_ARGTYPE_NONE)
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


		case K_AI_STATE_FN_LIGHT_FLICKER1:
		{
			//params: f_timeMul, f_threshold
			float timeMul = active.varAIparams[L"f_timeMul"].m_asFloat;
			float fThreshold = active.varAIparams[L"f_threshold"].asFloat();
			float falpha = UTPerlin::PerlinNoise1D( fTimelineAI * timeMul, 2.0f, 3.0f, 0.8f, 0.25f, 2 );
			if ( falpha > fThreshold )
				falpha = 1.0f;
			else
				falpha = falpha / fThreshold;
			//falpha = (falpha < fThreshold) ? 0.0f : 1.0f;
			active.color = DW_COLORALPHA( active.color_ini, falpha );
		}
		break;
		case K_AI_STATE_FN_LIGHT_ANG_CONE_XZ_TIME:
		{
			if ( active.GetClassType() != K_LVL_IAI_TYPE_LIGHT )
				break;
			//fvar1 - height, fvar2 - radius, timer1 - timeMul, timer2 - timeAdd
			auto *light = dynamic_cast<CLight *>(&active);
			if (nullptr != light)
			{
				Vec3 conepoint( 0.0f, -mem.AIfvar1, 0.0f );
				Vec3 ppos = Vec3( mem.AIfvar2 * sin( ( fTimelineAI + mem.AItimer2 ) * mem.AItimer1 ), 0.0f, mem.AIfvar2 * cos( ( fTimelineAI + mem.AItimer2 ) * mem.AItimer1 ) );
				MUVec3Norm( &light->vnDir, &( ppos - conepoint ) );
			}
		}
		break;

		case K_AI_STATE_FN_TOUCH_WHEN_SEE_PLAYER:
		{
		}
		break;
		case K_AI_STATE_TRIGGER_IN_OUT:
		{
			/*
			bool bTrigger = false;
			//triggered by players
			if ( mem.AIfvar1 != 0.0f )
			{
				for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
				{
					if ( (pPlayerActor[ kk ] != nullptr) && (pPlayerActor[ kk ]->IsAlive()) && (pPlayerActor[ kk ]->bbox.Intersects( bbox )) )
					{
						bTrigger = true;
						break;
					}
				}
			}
			//triggered by enemies
			if ( mem.AIfvar2 != 0.0f )
			{
				for ( int kk = 0; kk < m_arrActors.GetSize(); kk++ )
				{
					//skip actors that are: hidden, dead, players or not a target
					if ( (!m_arrActors[ kk ]->IsAlive()) || (m_arrActors[ kk ]->_template.actorClass == K_ACT_CLASS_PLAYER) ||
						((m_arrActors[ kk ]->_template.eCaps & K_ACT_CAPS_NOT_A_TARGET) != 0) )
						continue;
					if ( m_arrActors[ kk ]->bbox.Intersects( bbox ) )
					{
						bTrigger = true;
						break;
					}
				}
			}
			//see if toggled and run scripts
			if ( (AIvar1 == 0) && (bTrigger) )
			{
				//On Enter
				Touch( GetUID(), 0.0f );
				AIvar1 = 1;
			}
			else if ( (AIvar1 != 0) && (!bTrigger) && (AIstrvar1.textHash != 0) )
			{
				//On Leave if script present
				Touch( GetUID(), 0.0f, AIstrvar1.textHash );
				AIvar1 = 0;
			}
			*/
		}
		break;

		case K_AI_STATE_FN_POS_ELLIPSE:	//f_radX, f_radY, f_timeMul
		{
			/*
			//check number of params
			if (varAIparams.GetVariantCount() < 3)
			{
				LOG(L"UpdateAI_base::ID:%d class:%d needs more AI params", ID, GetClassType());
				break;
			}
			float radX = varAIparams[0]->asFloat();
			float radY = varAIparams[1]->asFloat();
			float timeMul = varAIparams[2]->asFloat();

			Vec2 delta = Vec2(radX * cos(timeMul * fTimeline), radY * sin(timeMul * fTimeline));
			SetPos(pos_ini + delta);
			*/
		}
		break;
		case K_AI_STATE_FN_ANG_SIN_TIME:
		{
			/*
			//check number of params
			if (varAIparams.GetVariantCount() < 4)
			{
				LOG(L"UpdateAI_base::ID:%d class:%d needs more AI params", ID, GetClassType());
				break;
			}
			float fmin = varAIparams[0]->asFloat();
			float fmax = varAIparams[1]->asFloat();
			float timeMul = varAIparams[2]->asFloat();
			float timeAdd = varAIparams[3]->asFloat();

			float dangle = fmin + (fmax - fmin) * ((sin(fLocalTimeline * timeMul + timeAdd) + 1.0f) / 2.0f);
			fAngle = fAngle_ini + dangle;
			*/
		}
		break;
		case K_AI_STATE_FN_ALPHA_SIN_TIME:
		{
			//check number of params
			if ( active.varAIparams.GetSize() < 3 )
			{
				LOG( L"UpdateAI_base::ID:%d class:%d needs more AI params", active.ID, active.GetClassType() );
				break;
			}
			//f_min, f_max, f_timeMul, f_timeAdd
			float fmin = active.varAIparams[ L"f_min" ].asFloat();
			float fmax = active.varAIparams[ L"f_max" ].asFloat();
			float timeMul = active.varAIparams[ L"f_timeMul" ].asFloat();
			float timeAdd = active.varAIparams[ L"f_timeAdd" ].asFloat();

			float falpha = fmin + (fmax - fmin) * ((sin( fTimelineAI * timeMul + timeAdd ) + 1.0f) / 2.0f);
			active.color = DW_COLORALPHA( active.color_ini, falpha );
		}
		break;
		case K_AI_STATE_FN_GET_TARGET_POS:
		{
			/*
			if (pTarget != NULL)
			{
				Vec2 targetDelta = pTarget->pos.xy - pTarget->pos_ini.xy;
				//mut obiectul cu delta totala a targetului
				SetPos(pos_ini + targetDelta);
			}
			*/
		}
		break;
		case K_AI_STATE_FN_GET_TARGET_ANG:
		{
			/*
			if (pTarget != NULL)
			{
				Vec2 targetVec = pos_ini.xy - pTarget->pos_ini.xy;
				Mat matrot;
				MuMatRotZ(&matrot, pTarget->fAngle - pTarget->fAngle_ini);
				MUVec2TransformCoord(&targetVec, &targetVec, &matrot);
				//mut obiectul cu delta totala a targetului
				SetPos(pTarget->pos_ini.xy + targetVec);
				SetAngle(fAngle_ini + (pTarget->fAngle - pTarget->fAngle_ini));
			}
			*/
		}
		break;
		case K_AI_STATE_FN_FOLLOW_TARGET_RAIL:
		{
			static const UINT32 hash_f_pointPauseSec = FastHash( L"f_pointPauseSec" );
			static const UINT32 hash_b_autoChangeDirection = FastHash( L"b_autoChangeDirection" );
			
			CMiscObjectRail* rail = null;
			
			CVariant railvc = active.varAIparams[L"railPtr"];
			if ( railvc.eType != CVariant::K_ARGTYPE_VOIDP)
			{
				ErrorBox( K_ERR_WARNING, L"Rail pointer not found!", active.targetID_ini );
				break;
			}
			//get rail pointer
			rail = static_cast<CMiscObjectRail*>(railvc.m_asVoid);
			// advances without pause at the ends (for now)
			if ( mem.AItimer2 > 0.0f )
			{
				mem.AItimer2 -= dTime;
			}
			else
			{
				//move 
				int movedir = active.varAIparams[L"n_dir"].m_asINT32;
				if ( movedir != 0 ) //movedir == 0 inseamna ca sta pe loc
				{
					// rail pos perc	    //speed
					mem.AItimer1 += mem.AIfvar1 * dTime * movedir;

					if ( mem.AItimer1 >= rail->fLength )
					{
						mem.AItimer1 = rail->fLength;
						//is looping? go to the other side of the rail
						if ( mem.AIvarBool1 )
						{
							mem.AItimer1 = 0.0f;
						}
						else
						{
							//inverseaza directia daca e pe auto
							if ( active.varAIparams[L"b_autoChangeDirection"].m_asINT32 != 0 )
							{
								active.varAIparams.SetVarINT32( L"n_dir", -movedir );
							}
						}

						mem.AItimer2 = active.varAIparams[L"f_pointPauseSec"].asFloat();
					}
					else if ( mem.AItimer1 <= 0.0f )
					{
						mem.AItimer1 = 0.0f;
						if ( mem.AIvarBool1 )
						{
							mem.AItimer1 = rail->fLength;
						}
						else
						{
							//reverse direction only if not looping
							if ( active.varAIparams[L"b_autoChangeDirection"].m_asINT32 != 0 )
								active.varAIparams.SetVarINT32( L"n_dir", -movedir ); //reverse dir
						}
						//reset wait timer
						mem.AItimer2 = active.varAIparams[L"f_pointPauseSec"].asFloat();
					}
				}
			}
			Vec2 newpos = rail->GetPos( mem.AItimer1 );
			active.SetPos( Vec3( newpos.x, newpos.y, active.pos.xyz.z ) );
		}
		break;
		default:
			bProcessedState = false;
			break;
	}

	return bProcessedState;

}

void CActiveAIComponent::SetAI( IActiveInterface& active, EAIstate newstate )
{
	active.AIstate = newstate;

	// Some states need setting up:
	switch ( newstate )
	{
		case K_AI_STATE_ACTIVE_HEALTH_BOX:
		case K_AI_STATE_ACTIVE_AMMO_BOX:
		{
			//wait 5 seconds before disappearing when empty
			mem.AItimer1 = 5.0f;
		}
		break;
		case K_AI_STATE_ACTIVE_BOMB:
		{
			mem.AItimer1 = active.varAIparams[ L"f_explodeTimerSec" ].m_asFloat;
			if ( mem.AItimer1 <= 0.0f )
			{
				ErrorBox( K_ERR_WARNING, L"Bomb without timer! ID:%d", active.ID );
				mem.AItimer1 = 60.0f;
			}
		}
		break;
		case K_AI_STATE_FN_LIGHT_ANG_CONE_XZ_TIME:
		{
			mem.AIfvar1 = active.varAIparams[ L"f_coneHeight" ].m_asFloat;
			mem.AIfvar2 = active.varAIparams[ L"f_coneRadius" ].m_asFloat;
			mem.AItimer1 = active.varAIparams[ L"f_timeMul" ].m_asFloat;
			mem.AItimer2 = active.varAIparams[ L"f_timeAdd" ].m_asFloat;
		}
		break;
		case K_AI_STATE_ACTIVE_DOORFACE_AUTOCLOSE:
		{
			//door timer (how long it stais open)
			mem.AItimer1 = 0.0f;
			//este deschisa sau inchisa acum?
			mem.AIvarBool1 = false;
		}
		break;
		case K_AI_STATE_ACTIVE_TEAM_TELEPORTER_2FRAMES:
		{
			active.varAIparams.SetVarUINT32( L"nToucherUID", 0 );
			//door timer (cat timp sta usa deschisa) il tinem in AItimer1
			mem.AItimer1 = 0.0f;
			//este deschisa sau inchisa acum?
			mem.AIvarBool1 = false;
		}
		break;
		case K_AI_STATE_ACTIVE_CHECKPOINT:
		{
			bool bIsFirst = (active.varAIparams[ L"n_isFirst" ].m_asUINT32 != 0);
			//daca este primul ii dau touch automat
			if ( bIsFirst )
			{
				active.Touch( active.GetUID(), 0.0f );
				//save checkpoint
				__Sim().vLastSpawnPoint = active.pos.xy;
			}
		}
		break;
		case K_AI_STATE_FN_TOUCH_WHEN_SEE_PLAYER:
		{
			/*
			//unghiul introdus
			mem.AIfvar1 = active.varAIparams[L"f_angle")->m_asFloat;
			//aduc unghiul in -PI...PI
			//mem.AIfvar1 -= PI; //aici ar trebui facuta o functie care sa trateze asta
			//FOV
			mem.AIfvar2 = active.varAIparams[L"f_angleFOV")->m_asFloat;
			//range
			mem.AIfvar3 = active.varAIparams[L"f_radius")->m_asFloat;
			//set angle
			mem.fAngle = mem.fAngle_ini = mem.AIfvar1;
			mem.AItimer1 = 0.0f; //timer cooldown
			*/
		}
		break;
		case K_AI_STATE_COLL_BREAKABLE_DOOR:
		{
			//was hit flag
			mem.AIvarBool1 = false;
			//viata usii (poate fi sparta de unele gloante)
			mem.AIfvar1 = 100000.0f; //by default nu poate fi distrusa de shotgun (sau foarte greu)
			mem.AIfvar2 = mem.AIfvar1; //viata initiala
			CVariant *cvar = &active.varAIparams[ L"f_life" ];
			if ( cvar->eType == CVariant::K_ARGTYPE_FLOAT )
			{
				mem.AIfvar1 = cvar->m_asFloat;
				//salvam si energia initiala
				mem.AIfvar2 = mem.AIfvar1;
			}
			//flag for when it gets hit
			mem.AIvarBool1 = false;
			//timer for when it shakes
			mem.AItimer1 = 0.0f;
		}
		break;
		case K_AI_STATE_COLL_BREAKABLE_WINDOW:
		{
			//was hit flag
			mem.AIvarBool1 = false;
			//viata 
			mem.AIfvar1 = 2.0f; //by default se sparge usor
			mem.AIfvar2 = mem.AIfvar1; //viata initiala
			CVariant *cvar = &active.varAIparams[ L"f_life" ];
			if ( cvar->eType == CVariant::K_ARGTYPE_FLOAT )
			{
				mem.AIfvar1 = cvar->m_asFloat;
				//salvam si energia initiala
				mem.AIfvar2 = mem.AIfvar1;
			}
		}
		break;
		case K_AI_STATE_PARTICLES_GENERATOR:
		{
			// tipul generatorului il ia din params
			int genType = __Particles().GetPartEmitterTypeByNameHash( active.varAIparams[ L"s_Type" ].m_strArg.textHash );
			int partLayer = __Particles().GetParticleLayerByName( active.varAIparams[ L"s_Layer" ].m_strArg.textHash );
			//ca sa nu intre de mai multe ori si sa aloce de mai multe ori. Daca se intampla trebuie dezalocat mai intai
			//_ASSERT( active.varAIparams[ L"emitterPtr" ]->m_type == CVariant::K_ARGTYPE_NONE );

			CParticleEmitter * pe = __Particles().AddPartEmitter( genType, &active.bbox, partLayer );
			//salveaza aici pointer la ParticleEmitter-ul alocat si il controlez din update sa ii dau stop si play cand iese din ecran
			active.varAIparams.SetVarVoidP( L"emitterPtr", pe );
		}
		break;
		case K_AI_STATE_TRIGGER_IN_OUT:
		{
			//b_triggerPlayer 
			mem.AIfvar1 = (float)active.varAIparams[ L"b_triggerPlayer" ].m_asINT32;
			//b_triggerActor
			mem.AIfvar2 = (float)active.varAIparams[ L"b_triggerActor" ].m_asINT32;
			if ( mem.AIfvar2 != 0.0f )
			{
				DebugPrintA( "TRIGGER_IN_OUT - all actors flag enabled! don't use too much of these\n" );
			}
			//s_onOutScript
			mem.AIstrvar1.Init( active.varAIparams[ L"s_onOutScript" ].m_strArg.text );
			//last state:
			mem.AIvar1 = 0; //deactivated
		}
		break;
		case K_AI_STATE_FN_FOLLOW_TARGET_RAIL:
		{
			CMiscObjectRail* rail = null;
			//find rail
			for ( int kk = 0; kk < __Sim().m_arrMiscObjects.Count(); kk++ )
			{
				if ( __Sim().m_arrMiscObjects[ kk ]->ID == active.targetID_ini )
				{
					rail = dynamic_cast<CMiscObjectRail*>(__Sim().m_arrMiscObjects[ kk ]);
				}
			}
			if ( rail == nullptr )
			{
				ErrorBox( K_ERR_WARNING, L"Rail id %d not found for object ID %d!", active.targetID_ini, active.ID );
				break;
			}
			//save rail ptr
			active.varAIparams.SetVarVoidP( L"railPtr", rail );
			//this is first time initialization
			float fPos = active.varAIparams[ L"f_positionPercent" ].asFloat();
			CLAMP( fPos, 0.0f, 1.0f );
			//cursor pozitie rail
			mem.AItimer1 = fPos * rail->fLength;
			//salvez si viteza
			mem.AIfvar1 = 0.0f; //viteza
			mem.AIfvar1 = active.varAIparams[ L"f_speedPPS" ].asFloat();
			//wait timerul de capat de rail
			mem.AItimer2 = active.varAIparams[ L"f_pointPauseSec" ].asFloat();
			//looping rail?
			mem.AIvarBool1 = (active.varAIparams[ L"b_looping" ].m_asINT32 != 0);
		}
		break;
		//unknown or no AI state
		default:
			// no need for alerts
			break;
	}

}
