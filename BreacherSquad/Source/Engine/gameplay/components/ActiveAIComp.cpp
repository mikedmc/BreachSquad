#include "dxstdafx.h"
#include "ActiveAIComp.h"

CActiveAIComponent::CActiveAIComponent()
{

}

CActiveAIComponent::~CActiveAIComponent()
{

}

bool CActiveAIComponent::Update( IActiveInterface& active, float dTime )
{
	bool bProcessedState = true;
	if ( active.AIstate == K_AI_STATE_UNDEFINED )
		return true;

	switch ( active.AIstate )
	{
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
					if ( (!m_arrActors[ kk ]->IsAlive()) || (m_arrActors[ kk ]->_template.actorClass == K_LVL_ACT_CLASS_PLAYER) ||
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
			if ( active.varAIparams.GetVariantCount() < 3 )
			{
				LOG( L"UpdateAI_base::ID:%d class:%d needs more AI params", active.ID, active.GetClassType() );
				break;
			}
			float fmin = active.varAIparams[ 0 ]->asFloat();
			float fmax = active.varAIparams[ 1 ]->asFloat();
			float timeMul = active.varAIparams[ 2 ]->asFloat();
			float timeAdd = active.varAIparams[ 3 ]->asFloat();

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
			static const UINT32 hash_v_railPtr = FastHash( L"railPtr" );
			static const UINT32 hash_n_dir = FastHash( L"n_dir" );
			static const UINT32 hash_f_pointPauseSec = FastHash( L"f_pointPauseSec" );
			static const UINT32 hash_b_autoChangeDirection = FastHash( L"b_autoChangeDirection" );
			
			CMiscObjectRail* rail = null;
			
			CVariantComplex* railvc = active.varAIparams.GetVariantByNameHash( hash_v_railPtr );
			if ( railvc->m_type == CVariantComplex::K_ARGTYPE_NONE )
			{
				ErrorBox( K_ERR_WARNING, L"Rail pointer not found!", active.targetID_ini );
				break;
			}
			//get rail pointer
			rail = static_cast<CMiscObjectRail*>(railvc->m_asVoid);
			// advances without pause at the ends (for now)
			if ( mem.AItimer2 > 0.0f )
			{
				mem.AItimer2 -= dTime;
			}
			else
			{
				//move 
				int movedir = active.varAIparams.GetVariantByNameHash( hash_n_dir )->m_asINT32;
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
							if ( active.varAIparams.GetVariantByNameHash( hash_b_autoChangeDirection )->m_asINT32 != 0 )
							{
								active.varAIparams.SetNamedVarINT32( L"n_dir", -movedir );
							}
						}

						CVariantComplex* waitTimer = active.varAIparams.GetVariantByNameHash( hash_f_pointPauseSec );
						mem.AItimer2 = waitTimer->asFloat();
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
							if ( active.varAIparams.GetVariantByNameHash( hash_b_autoChangeDirection )->m_asINT32 != 0 )
								active.varAIparams.SetNamedVarINT32( L"n_dir", -movedir ); //inversam directia
						}
						//reset wait timer
						CVariantComplex* waitTimer = active.varAIparams.GetVariantByNameHash( hash_f_pointPauseSec );
						mem.AItimer2 = waitTimer->asFloat();
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
			mem.AItimer1 = active.varAIparams.GetVariantByName( L"f_explodeTimerSec" )->m_asFloat;
			if ( mem.AItimer1 <= 0.0f )
			{
				ErrorBox( K_ERR_WARNING, L"Bomb without timer! ID:%d", active.ID );
				mem.AItimer1 = 60.0f;
			}
		}
		break;
		case K_AI_STATE_FN_LIGHT_ANG_CONE_XZ_TIME:
		{
			mem.AIfvar1 = active.varAIparams.GetVariantByName( L"f_coneHeight" )->m_asFloat;
			mem.AIfvar2 = active.varAIparams.GetVariantByName( L"f_coneRadius" )->m_asFloat;
			mem.AItimer1 = active.varAIparams.GetVariantByName( L"f_timeMul" )->m_asFloat;
			mem.AItimer2 = active.varAIparams.GetVariantByName( L"f_timeAdd" )->m_asFloat;
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
			active.varAIparams.SetNamedVarUINT32( L"nToucherUID", 0 );
			//door timer (cat timp sta usa deschisa) il tinem in AItimer1
			mem.AItimer1 = 0.0f;
			//este deschisa sau inchisa acum?
			mem.AIvarBool1 = false;
		}
		break;
		case K_AI_STATE_ACTIVE_CHECKPOINT:
		{
			bool bIsFirst = (active.varAIparams.GetVariantByName( L"n_isFirst" )->m_asUINT32 != 0);
			//daca este primul ii dau touch automat
			if ( bIsFirst )
			{
				active.Touch( active.GetUID(), 0.0f );
				//save checkpoint
				vLastSpawnPoint = mem.pos.xy;
			}
		}
		break;
		case K_AI_STATE_FN_TOUCH_WHEN_SEE_PLAYER:
		{
			/*
			//unghiul introdus
			mem.AIfvar1 = active.varAIparams.GetVariantByName(L"f_angle")->m_asFloat;
			//aduc unghiul in -PI...PI
			//mem.AIfvar1 -= PI; //aici ar trebui facuta o functie care sa trateze asta
			//FOV
			mem.AIfvar2 = active.varAIparams.GetVariantByName(L"f_angleFOV")->m_asFloat;
			//range
			mem.AIfvar3 = active.varAIparams.GetVariantByName(L"f_radius")->m_asFloat;
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
			CVariantComplex *cvar = active.varAIparams.GetVariantByName( L"f_life" );
			if ( cvar->m_type == CVariantComplex::K_ARGTYPE_FLOAT )
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
			CVariantComplex *cvar = active.varAIparams.GetVariantByName( L"f_life" );
			if ( cvar->m_type == CVariantComplex::K_ARGTYPE_FLOAT )
			{
				mem.AIfvar1 = cvar->m_asFloat;
				//salvam si energia initiala
				mem.AIfvar2 = mem.AIfvar1;
			}
		}
		break;
		case K_AI_STATE_PARTICLES_GENERATOR:
		{
			//tipul generatorului il ia din params
			int genType = g_particlesMgr.GetPartEmitterTypeByNameHash( active.varAIparams.GetVariantByName( L"s_Type" )->m_strArg.textHash );
			int partLayer = g_particlesMgr.GetParticleLayerByName( active.varAIparams.GetVariantByName( L"s_Layer" )->m_strArg.textHash );
			//ca sa nu intre de mai multe ori si sa aloce de mai multe ori. Daca se intampla trebuie dezalocat mai intai
			_ASSERT( active.varAIparams.GetVariantByName( L"emitterPtr" )->m_type == CVariantComplex::K_ARGTYPE_NONE );

			CParticleEmitter * pe = g_particlesMgr.AddPartEmitter( genType, &active.bbox, partLayer );
			//salveaza aici pointer la ParticleEmitter-ul alocat si il controlez din update sa ii dau stop si play cand iese din ecran
			active.varAIparams.SetNamedVarVoidP( L"emitterPtr", pe );
		}
		break;
		case K_AI_STATE_TRIGGER_IN_OUT:
		{
			//b_triggerPlayer 
			mem.AIfvar1 = (float)active.varAIparams.GetVariantByName( L"b_triggerPlayer" )->m_asINT32;
			//b_triggerActor
			mem.AIfvar2 = (float)active.varAIparams.GetVariantByName( L"b_triggerActor" )->m_asINT32;
			if ( mem.AIfvar2 != 0.0f )
			{
				DebugPrintA( "TRIGGER_IN_OUT - all actors flag enabled! don't use too much of these\n" );
			}
			//s_onOutScript
			mem.AIstrvar1.Init( active.varAIparams.GetVariantByName( L"s_onOutScript" )->m_strArg.text );
			//last state:
			mem.AIvar1 = 0; //deactivated
		}
		break;
		case K_AI_STATE_FN_FOLLOW_TARGET_RAIL:
		{
			CMiscObjectRail* rail = null;
			//find rail
			for ( int kk = 0; kk < m_arrMiscObjects.Count(); kk++ )
			{
				if ( m_arrMiscObjects[ kk ]->ID == mem.targetID_ini )
				{
					rail = dynamic_cast<CMiscObjectRail*>(m_arrMiscObjects[ kk ]);
				}
			}
			if ( rail == NULL )
			{
				ErrorBox( K_ERR_WARNING, L"Rail id %d not found for object ID %d!", active.targetID_ini, active.ID );
				break;
			}
			//save rail ptr
			active.varAIparams.SetNamedVarVoidP( L"railPtr", rail );
			//this is first time initialization
			float fPos = active.varAIparams.GetVariantByName( L"f_positionPercent" )->asFloat();
			CLAMP( fPos, 0.0f, 1.0f );
			//cursor pozitie rail
			mem.AItimer1 = fPos * rail->fLength;
			//salvez si viteza
			mem.AIfvar1 = 0.0f; //viteza
			mem.AIfvar1 = active.varAIparams.GetVariantByName( L"f_speedPPS" )->asFloat();
			//wait timerul de capat de rail
			mem.AItimer2 = active.varAIparams.GetVariantByName( L"f_pointPauseSec" )->asFloat();
			//looping rail?
			mem.AIvarBool1 = (active.varAIparams.GetVariantByName( L"b_looping" )->m_asINT32 != 0);
		}
		break;
		//unknown or no AI state
		default:
			break;
	}

}
