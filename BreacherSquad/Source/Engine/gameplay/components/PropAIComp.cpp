#include "dxstdafx.h"
#include "LightAIComp.h"

CPropAIComponent::CPropAIComponent()
{

}

CPropAIComponent::~CPropAIComponent()
{

}

bool CPropAIComponent::Update( CProp& active, float dTime, CLevel& level )
{
	if ( active.AIstate == K_AI_STATE_UNDEFINED )
		return true;
	//update timeline
	fTimelineAI += dTime;

	switch ( active.AIstate )
	{
		case K_AI_STATE_ACTIVE_BOMB:
		{
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
				level.AddDoofer_Explo( hash_EXPLO_LARGE_XL, active.pos.xy, active.UID, K_LVL_ACT_CLASS_EXPLOSION );
				level.AddDoofer_Explo( hash_EXPLO_LARGE_XL, active.pos.xy + Vec2( 32.0f, 0.0f ), active.UID, K_LVL_ACT_CLASS_EXPLOSION );
				level.AddDoofer_Explo( hash_EXPLO_LARGE_XL, active.pos.xy - Vec2( 32.0f, 0.0f ), active.UID, K_LVL_ACT_CLASS_EXPLOSION );

				__Particles().AddParticle( ANM_PARTICLES_SPR_EXPLO_ROUND_XL, true, 0, &active.pos.xy, NULL, NULL, 1.0f, 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM );

				active.sprite.SetAnim( "BOMB_EXPLODED" );

				level.SetLevelState( K_LVL_STATE_MISSION_FAILED, STR_BOMB_EXPLODED );
			}
		}
		break;
		case K_AI_STATE_ACTIVE_AMMO_BOX:
		{
			int nAmmoLeft = active.varAIparams.GetVariantByName( L"n_ammoLeft" )->m_asINT32;
			active.sprite.frameIdx = nAmmoLeft;

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
			int nHealthLeft = active.varAIparams.GetVariantByName( L"n_healthLeft" )->m_asINT32;
			active.sprite.frameIdx = nHealthLeft;

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
			//keep door open (AIvar1 contine frame-ul default) - set frame
			active.sprite.frameIdx = active.fid_ini.frameIdx;
			if ( mem.AItimer1 > 0.0f )
			{
				mem.AItimer1 -= dTime;

				bool bDontChangeFrames = (bool)(active.varAIparams.GetVariantByName( L"b_DontChangeFrames" )->m_asBool);
				if ( !bDontChangeFrames )
				{
					active.sprite.frameIdx++;
				}

				if ( mem.AItimer1 < 0.0f )
					mem.AItimer1 = 0.0f;
			}

			//open/close sounds
			if ( (mem.AIvarBool1 == false) && (mem.AItimer1 > 0.0f) )
			{
				//just opened
				CVariantComplex* cvc = active.varAIparams.GetVariantByName( L"s_openSnd" );
				if ( cvc->m_type == CVariantComplex::K_ARGTYPE_STRING )
				{
					int sndidx = UTGetSoundManager().getSndIdx( cvc->m_strArg.textHash );
					SND_PLAY_POSITIONAL( sndidx, active.pos.xy );
				}
				//on open script
				cvc = active.varAIparams.GetVariantByName( L"s_ScriptOnOpen" );
				if ( cvc->m_type == CVariantComplex::K_ARGTYPE_STRING )
				{
					UTGetScriptManager().StartScript( cvc->m_strArg.textHash, active.UID );
				}

				mem.AIvarBool1 = true;
			}
			else if ( (mem.AIvarBool1 == true) && (mem.AItimer1 <= 0.0f) )
			{
				//just closed
				CVariantComplex* cvc = active.varAIparams.GetVariantByName( L"s_closeSnd" );
				if ( cvc->m_type == CVariantComplex::K_ARGTYPE_STRING )
				{
					int sndidx = UTGetSoundManager().getSndIdx( cvc->m_strArg.textHash );
					SND_PLAY_POSITIONAL( sndidx, active.pos.xy );
				}
				//on close script
				cvc = active.varAIparams.GetVariantByName( L"s_ScriptOnClose" );
				if ( cvc->m_type == CVariantComplex::K_ARGTYPE_STRING )
				{
					UTGetScriptManager().StartScript( cvc->m_strArg.textHash, active.UID );
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
				if ( level.pPlayerActor[ kk ] == null )
					continue;
				if ( level.pPlayerActor[ kk ]->bbox.Intersects( active.bbox ) )
				{
					active.Touch( level.pPlayerActor[ kk ]->GetUID(), dTime );
					//save checkpoint
					level.vLastSpawnPoint = active.pos.xy;
					break;
				}
			}
		}
		break;
		default:
		{
			// call base update if not handled
			if ( !CActiveAIComponent::Update( active, dTime, level ) )
			{
				ErrorBox( K_ERR_WARNING, L"PropAIComp::Update - AIstate not handled: %d", EAIstate_names[ active.AIstate ] );
			}
		}
		break;
	}
}
