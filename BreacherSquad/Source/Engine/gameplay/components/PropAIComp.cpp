#include "dxstdafx.h"
#include "LightAIComp.h"

CPropAIComponent::CPropAIComponent()
{

}

CPropAIComponent::~CPropAIComponent()
{

}

bool CPropAIComponent::Update( CProp& active, float dTime )
{
	if ( active.AIstate == K_AI_STATE_UNDEFINED )
		return true;
	//update timeline
	fTimelineAI += dTime;

	//stari particulare lumini (se pot suprascrie cele default)
	switch ( active.AIstate )
	{
		case K_AI_STATE_ACTIVE_BOMB:
		{
			//daca nu esti pe playing nu mai scade counterul la bomba
			if ( m_levelState != K_LVL_STATE_PLAYING )
				break;

			float fOldTimer = prop->AItimer1;
			prop->AItimer1 -= dTime;
			//m_interfaceIGM.SetBombTimer(prop->AItimer1);

			//--- sounds ---
			if ( prop->AItimer1 > 15.0f )
			{
				if ( floor( fOldTimer ) > floor( prop->AItimer1 ) )
				{
					//SND_PLAY(SNDIDX_BOMBBEEP);
				}
			}
			else
			{
				if ( m_Timers.Tick( 250 ) )
				{
					//SND_PLAY(SNDIDX_BOMBBEEP);
				}
			}

			if ( prop->AItimer1 <= 0.0f )
			{
				//m_interfaceIGM.SetBombTimer(0.0f);
				//add some explosions so everybody will die
				AddDoofer_Explo( hash_EXPLO_LARGE_XL, prop->pos.xy, prop->UID, K_LVL_ACT_CLASS_EXPLOSION );
				AddDoofer_Explo( hash_EXPLO_LARGE_XL, prop->pos.xy + Vec2( 32.0f, 0.0f ), prop->UID, K_LVL_ACT_CLASS_EXPLOSION );
				AddDoofer_Explo( hash_EXPLO_LARGE_XL, prop->pos.xy - Vec2( 32.0f, 0.0f ), prop->UID, K_LVL_ACT_CLASS_EXPLOSION );

				g_particlesMgr.AddParticle( ANM_PARTICLES_SPR_EXPLO_ROUND_XL, true, 0, &Vec2( prop->pos.xy.x, prop->pos.xy.y - 15.0f ), NULL, NULL, 1.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM );

				prop->sprite.SetAnim( "BOMB_EXPLODED" );

				SetLevelState( K_LVL_STATE_MISSION_FAILED, STR_BOMB_EXPLODED );
			}
		}
		break;
		case K_AI_STATE_ACTIVE_AMMO_BOX:
		{
			int nAmmoLeft = prop->varAIparams.GetVariantByName( L"n_ammoLeft" )->m_asINT32;
			prop->sprite.frameIdx = nAmmoLeft;

			//fade out
			if ( nAmmoLeft <= 0 )
			{
				prop->AItimer1 -= dTime;
				if ( prop->AItimer1 <= 0.0f )
				{
					prop->Kill();
				}
				//color
				float fAlpha = LIMIT( prop->AItimer1, 0.0f, 1.0f );
				prop->color = DW_COLORALPHA( prop->color_ini, fAlpha );
			}
		}
		break;
		case K_AI_STATE_ACTIVE_HEALTH_BOX:
		{
			int nHealthLeft = prop->varAIparams.GetVariantByName( L"n_healthLeft" )->m_asINT32;
			prop->sprite.frameIdx = nHealthLeft;

			//fade out
			if ( nHealthLeft <= 0 )
			{
				prop->AItimer1 -= dTime;
				if ( prop->AItimer1 <= 0.0f )
				{
					prop->Kill();
				}
				//color
				float fAlpha = LIMIT( prop->AItimer1, 0.0f, 1.0f );
				prop->color = DW_COLORALPHA( prop->color_ini, fAlpha );
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
			if ( prop->AItimer1 > 0.0f )
			{
				prop->AItimer1 -= dTime;

				bool bDontChangeFrames = (bool)(prop->varAIparams.GetVariantByName( L"b_DontChangeFrames" )->m_asBool);
				if ( !bDontChangeFrames )
				{
					prop->sprite.frameIdx++;
				}

				if ( prop->AItimer1 < 0.0f )
					prop->AItimer1 = 0.0f;
			}

			//open/close sounds
			if ( (prop->AIvarBool1 == false) && (prop->AItimer1 > 0.0f) )
			{
				//just opened
				CVariantComplex* cvc = prop->varAIparams.GetVariantByName( L"s_openSnd" );
				if ( cvc->m_type == CVariantComplex::K_ARGTYPE_STRING )
				{
					int sndidx = UTGetSoundManager().getSndIdx( cvc->m_strArg.textHash );
					SND_PLAY_POSITIONAL( sndidx, prop->pos.xy );
				}
				//on open script
				cvc = prop->varAIparams.GetVariantByName( L"s_ScriptOnOpen" );
				if ( cvc->m_type == CVariantComplex::K_ARGTYPE_STRING )
				{
					UTGetScriptManager().StartScript( cvc->m_strArg.textHash, prop->UID );
				}

				prop->AIvarBool1 = true;
			}
			else if ( (prop->AIvarBool1 == true) && (prop->AItimer1 <= 0.0f) )
			{
				//just closed
				CVariantComplex* cvc = prop->varAIparams.GetVariantByName( L"s_closeSnd" );
				if ( cvc->m_type == CVariantComplex::K_ARGTYPE_STRING )
				{
					int sndidx = UTGetSoundManager().getSndIdx( cvc->m_strArg.textHash );
					SND_PLAY_POSITIONAL( sndidx, prop->pos.xy );
				}
				//on close script
				cvc = prop->varAIparams.GetVariantByName( L"s_ScriptOnClose" );
				if ( cvc->m_type == CVariantComplex::K_ARGTYPE_STRING )
				{
					UTGetScriptManager().StartScript( cvc->m_strArg.textHash, prop->UID );
				}
				prop->AIvarBool1 = false;
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
				if ( pPlayerActor[ kk ] == null )
					continue;
				if ( pPlayerActor[ kk ]->bbox.Intersects( prop->bbox ) )
				{
					prop->Touch( pPlayerActor[ kk ]->GetUID(), dTime );
					//save checkpoint
					vLastSpawnPoint = prop->pos.xy;
					break;
				}
			}
		}
		break;
		default:
		{
			// call base update if not handled
			if ( !CActiveAIComponent::Update( active, dTime ) )
			{
				ErrorBox( K_ERR_WARNING, L"PropAIComp::Update - AIstate not handled: %d", EAIstate_names[ active.AIstate ] );
			}
		}
		break;
	}
}
