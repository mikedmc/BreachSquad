#include "dxstdafx.h"
#include "LightAIComp.h"

CCollAIComponent::CCollAIComponent()
{

}

CCollAIComponent::~CCollAIComponent()
{

}

bool CCollAIComponent::Update( CCollisionShape& active, float dTime, CLevel& level )
{
	if ( active.AIstate == K_AI_STATE_UNDEFINED )
		return true;
	//update timeline
	fTimelineAI += dTime;

	switch ( active.AIstate )
	{
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
					CProp* winact = dynamic_cast<CProp*>(active.pTarget);
					if ( winact == null )
					{
						ErrorBox( K_ERR_WARNING, L"K_AI_STATE_COLL_BREAKABLE_WINDOW bad cast to CActive" );
						break;
					}

					winact->sprite.frameIdx++;
					//reset object script and interact
					winact->arrActions.Clear();

					//generate particles
					CVariantComplex* cvar = active.varAIparams.GetVariantByName( L"fForceDirX" );
					float dirx = SIGN( cvar->m_asFloat );
					for ( int ll = 0; ll < 20; ll++ )
					{
						Vec2 ppos = AABB::GetRandomPointInBox( active.bbox );
						g_particlesMgr.AddParticle( ANM_PARTICLES_SPR_GLASS_SHARDS, false, randint( 5 ), &ppos, &g_vecGravityOld, &Vec2( dirx * (60.0f + randfloat( 60.0f )), -40.0f + randfloatsgn( 50.0f ) ), 0.3f + randfloat( 0.2f ), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM );
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
				CVariantComplex* cvar = active.varAIparams.GetVariantByName( L"fForceDirX" );
				if ( cvar->m_type == CVariantComplex::K_ARGTYPE_FLOAT )
				{
					fForceDirX = cvar->m_asFloat;
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
					g_particlesMgr.AddParticle( ANM_PARTICLES_SPR_WOODEN_SPLINTERS, false, randint( 6 ), &ppos, &g_vecGravityOld, &Vec2( dirx * (100.0f + randfloat( 60.0f )), -40.0f + randfloatsgn( 50.0f ) ), 0.3f + randfloat( 0.2f ), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM );
				}
				// add noise event behind the door
				Vec2 sndpos1 = Vec2( active.bbox.vCenter.x + dirx * (active.bbox.vHalfSize.x + 2.0f), active.bbox.vCenter.y );
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
					CVariantComplex* cvexploded = active.varAIparams.GetVariantByName(L"bExploded");
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
			CVariantComplex* emittervc = active.varAIparams.GetVariantByNameHash(hash_v_emitterPtr);
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
			// call base update if not handled
			if ( !CActiveAIComponent::Update( active, dTime, level ) )
			{
				ErrorBox( K_ERR_WARNING, L"CollAIComp::Update - AIstate not handled: %d", EAIstate_names[ active.AIstate ] );
			}
		}
		break;
	}
}
