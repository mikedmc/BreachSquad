#include "dxstdafx.h"
#include "ActorAIComp.h"

CActorAIComponent::CActorAIComponent() :
	m_pAIcurrentState( nullptr ), m_nAIcurrentBehaviorIdx( -1 ), m_fAIbehaviorTimer( 0.0f )
{

}

CActorAIComponent::~CActorAIComponent()
{

}

void CActorAIComponent::Update( CActor& act, float dTime, CLevel & level )
{
	//update timeline
	fTimelineAI += dTime;

	//reset previous commands
	m_AIcommands.Reset();
	//----------------------------------------
	//			PERCEIVE			
	//----------------------------------------

	//vedem daca a expirat durata behavior curent si daca da fortam un pas de decizie AI
	bool bBehaviorDurationFinished = false;
	if ( m_pAIcurrentState->m_arrBehaviors[ m_nAIcurrentBehaviorIdx ].fBehaviorDuration > 0.0f )
	{
		m_fAIbehaviorTimer += dTime;
		if ( m_fAIbehaviorTimer >= m_pAIcurrentState->m_arrBehaviors[ m_nAIcurrentBehaviorIdx ].fBehaviorDuration )
		{
			bBehaviorDurationFinished = true;
			AItimerDecision = 0.0f;
		}
	}

	if ( act.fLife <= 0.0f )
	{
		//ca sa intre doar o singura data:
		if ( m_AIsensorInfo.m_AIcurrentEvent.nType != K_LVL_AI_EVENT_DEAD )
		{
			m_AIsensorInfo.b_IsDead = true;
			m_AIsensorInfo.m_AIcurrentEvent.Set( K_LVL_AI_EVENT_DEAD, act.GetUID(), act._template.actorClass, act.GetPosHeart(), -1.0f, 1.0f, act.GetUID() );
			//save in memory
			m_AIsensorInfo.m_AIlastEvent = m_AIsensorInfo.m_AIcurrentEvent;
			//reset targeted actor
			m_AIsensorInfo.pTargetedActor = null;
			///THINK: force state decision
			CAIState* newState = act._template.AItemplate->GetHighestPriorityState( K_LVL_AI_EVENT_DEAD, &m_rand );
			Actor_SetAIState( actor, newState );
		}
	}
	else //process low freq sensors only if no message from realtime sensors (more important)
	{
		bool bIgnoreAIEvents = false;
		if ( (m_pAIcurrentState != null) && (m_nAIcurrentBehaviorIdx >= 0) )
			bIgnoreAIEvents = m_pAIcurrentState->m_arrBehaviors[ m_nAIcurrentBehaviorIdx ].bIgnoreEvents;

		///HIGH FREQUENCY SENSORS
		//hit timer (used in some behaviors)
		m_AIsensorInfo.fTimeSinceHit += dTime;
		//did he get hit? reset time since hit 
		//if (act.nTookDamageFrames > 0)
			//m_AIsensorInfo.fTimeSinceHit = 0.0f;

		///LOW FREQUENCY SENSORS
		AItimerDecision -= dTime;
		if ( (AItimerDecision <= 0.0f) && (!bIgnoreAIEvents) && (m_AIsensorInfo.m_bEnabled) )
		{
			//save previous event in memory only if not IDLE_TICK
			if ( m_AIsensorInfo.m_AIcurrentEvent.nType > K_LVL_AI_EVENT_IDLE_TICK )
				m_AIsensorInfo.m_AIlastEvent = m_AIsensorInfo.m_AIcurrentEvent;

			//check for targets or other AI events
			CActor* targetActor = nullptr;// GetClosestTarget(actor, act.actTemplate.foeClassFilter1, act.actTemplate.foeClassFilter2);
			if ( targetActor != nullptr )
			{
				float enemyDst = MUVec2Len( &(targetact.GetPosHeart() - act.GetPosHeart()) );
				AddAIEvent( K_LVL_AI_EVENT_SEE_ENEMY, targetact.GetUID(), targetact._template.actorClass, targetact.GetPosHeart(), enemyDst, 1.0f, act.GetUID() );
				//#HACK: alerts the other enemies only if enemy class
				if ( act._template.actorClass >= K_LVL_ACT_CLASS_HUMAN )
					AddAIEvent( K_LVL_AI_EVENT_SOUND_THREAT, targetact.GetUID(), targetact._template.actorClass, targetact.GetPosHeart(), 200.0f, 0.6f );
				//set target pointer
				m_AIsensorInfo.pTargetedActor = targetActor;
				//vede daca face overlap
				/*
				if (act.bbox.Intersects(targetact.bbox))
				{
					m_AIsensorInfo.fTargetOverlapX = SIGN(act.pos.xy.x - targetact.pos.xy.x) * ((act.bbox.vHalfSize.x + targetact.bbox.vHalfSize.x) - fabs(act.pos.xy.x - targetact.pos.x));
				}
				*/
				//daca se ating trimit si event de touch enemy, doar daca vede inamicul
				if ( act.bbox.Intersects( targetact.bbox ) )
				{
					AddAIEvent( K_LVL_AI_EVENT_TOUCH_ENEMY, targetact.GetUID(), targetact._template.actorClass, targetact.GetPosHeart(), enemyDst, 1.0f, act.GetUID() );
				}
				//scrie ultimul actor cu care a interactionat (nu este vital)
				m_AIsensorInfo.m_lastInteractingActorUID = targetact.GetUID();
			}
			else
			{
				//reset targeted actor
				if ( m_AIsensorInfo.pTargetedActor != null )
				{
					//sterg mesaj de see enemy pt actorul curent
					DeleteAITargetedEvent( K_LVL_AI_EVENT_SEE_ENEMY, act.GetUID() );
					//Trimit mesaj de LOST_ENEMY
					AddAIEvent( K_LVL_AI_EVENT_LOST_ENEMY, 0, K_LVL_ACT_CLASS_ANY, act.GetPosHeart() + Vec2( 16.0f, 0.0f ), 16.0f, 1.0f, act.GetUID() );
					//reset targeting actor
					m_AIsensorInfo.pTargetedActor = null;
				}

				//#HACK: uneori e lovit dar nu apuca sa vada inamicul si ramane blocat ca nu primeste LOST_ENEMY asa ca il trimitem acum
				if ( (m_AIsensorInfo.pTargetedActor == null) && (m_AIsensorInfo.m_AIlastEvent.nType == K_LVL_AI_EVENT_GOT_HIT) )
				{
					//put event behind him
					AddAIEvent( K_LVL_AI_EVENT_LOST_ENEMY, 0, K_LVL_ACT_CLASS_ANY, act.GetPosHeart() - Vec2( 16.0f, 0.0f ), 16.0f, 0.5f, act.GetUID() );
				}
			}

			///--- select best event ---
			CAIEvent* evt = GetMostImportantAIEvent( actor );

			if ( evt != null )
			{
				m_AIsensorInfo.m_AIcurrentEvent = *evt;
			}
			else
			{
				//nothing important, set idle tick
				m_AIsensorInfo.m_AIcurrentEvent.Set( K_LVL_AI_EVENT_IDLE_TICK, 0, 0, Vec2( 0.0f, 0.0f ), -1.0f, 1.0f );
			}
			//----------------------------------------
			//	THINK - decide best behavior
			//----------------------------------------
			//daca nu am behavior sau daca behaviorul imi permite sa il intrerup.
			//am comentat verificarea pe behaviorDurationFinished pentru ca mesajul de IDLE_TICK ma scotea dintre behaviors care nu pot fi intrerupte. Ca sa pot intrerupe cand vreau bag un behavior IDLE
			if ( (m_nAIcurrentBehaviorIdx < 0) || (m_pAIcurrentState->m_arrBehaviors[ m_nAIcurrentBehaviorIdx ].bCanInterrupt) /*|| (bBehaviorDurationFinished)*/ )
			{
				CAIState* newState = act._template.AItemplate->GetHighestPriorityState( m_AIsensorInfo.m_AIcurrentEvent.nType, &m_rand );

				//daca vechea stare a fost setata de acelasi mesaj ca si acum si nu are prioritate mai mica nu ar mai trebui setata alta stare ci cel mult dat restart la starea curenta
				if ( (newState != null) && (m_AIsensorInfo.m_AIlastEvent.nType == m_AIsensorInfo.m_AIcurrentEvent.nType) && (newState->nPriority == m_pAIcurrentState->nPriority) )
				{
					//#MAYBE: reset current behavior if it's the same state?
				}
				else
				{
					//if (newState != null)
					//	DebugPrintW(L"evttype:%d set_state: %s\n", m_AIsensorInfo.m_AIcurrentEvent.nType, newState->name.text);

					//state may also be null when no state is associated with an event
					Actor_SetAIState( actor, newState );
				}
			}
		}
	}


	//------------------------------------------------------------------------------------------
	//	THINK - run behavior - realtime	- proceseaza AIsensors si scrie doar in AIcommands
	//------------------------------------------------------------------------------------------
	bool bBehaviorFinished = false;

	bool bSkipAI = false;
	if ( (m_pAIcurrentState == null) || (m_nAIcurrentBehaviorIdx < 0) || (act.fStunTimer > 0.0f) )
		bSkipAI = true;

	if ( bBehaviorDurationFinished )
	{
		bBehaviorFinished = true;
		bSkipAI = true;
	}
	//simple way to check if it's time to decide
	bool bTimeToDecide = (AItimerDecision <= 0.0f);

	if ( !bSkipAI ) //daca nu am skip AI procesez switch-ul
	{
		switch ( m_pAIcurrentState->m_arrBehaviors[ m_nAIcurrentBehaviorIdx ].nType )
		{
			//------------------------------------------------------------------------------------------
			//	this switch only progesses AIcommands for movement
			//------------------------------------------------------------------------------------------

			//make sure we exit immediately from these
			case AI_BEHAVIOR_TELEPORT_TO_TOUCHABLE_TARGET:
			case AI_BEHAVIOR_EMPTY:
			case AI_BEHAVIOR_PLAY_VERSE:
			case AI_BEHAVIOR_GENERATE_EFFECT:
			case AI_BEHAVIOR_SET_ANIMSET:
			case AI_BEHAVIOR_SET_CAPS:
			case AI_BEHAVIOR_SUICIDE:
			{
				bBehaviorFinished = true;
			}
			break;

			//waits animation to play out before exiting
			case AI_BEHAVIOR_SHOW_ENEMY:
			{
			}
			break;

			case AI_BEHAVIOR_PLAYER_CONTROL:
			{
				CController* pController = UTGetCtrlrMgr().GetControllerByInstanceID( act.nControllerInstanceID );
				//controller not set or removed, skipping AI
				if ( (pController == null) || (pController->nFlags & K_CM_CTRLR_FLAG_PAUSED) || (act.bSuspendInput) )
				{
					//HitActor(actor, -1.0f, 0, 100, K_LVL_ACT_CLASS_TRAP);
					break;
				}
				//if suspended or other don't process input
				if ( (act.nSuspendedFlags != K_LVL_SUSPENDFLAG_NONE) || (level.m_levelState != K_LVL_STATE_PLAYING) )
					break;

				//switch to Strategic Ability selection
				/*
				if ( pController->sCommands.keyState[ K_CM_COMMAND_STRATEGIC_MENU ] == K_CM_BUTSTATE_JUSTPRESSED )
				{
					int nStatIdx = K_LVL_STATS_PL1_STRATEGIC_POINTS + act.nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT;

					//aici se seteaza pleayerselStrategic pe cea mai mare optiune
					//float fPoints = m_arrStats[nStatIdx] / 1000.0f;
					//set selection on first valid
					int nSel = 0;
					while ( (m_arrStrategicAbilities[ act.nPlayerOrdinal ][ nSel ] < 0) && (nSel < K_LVL_MAX_STRATEGIC_POINTS) )
						nSel++;

					m_arrPlayerSelStrategic[ act.nPlayerOrdinal ] = nSel;
					//m_interfaceIGM.SetStrategicSelection(act.nPlayerOrdinal, nSel);
					//play a sound on opening the interface
					//SND_PLAY(SNDIDX_CLICK_DENIED);
				}
				*/
				Vec2 vMoveDir = pController->GetDoubleAxisVector( K_CM_COMMAND_MOVE_X, K_CM_COMMAND_MOVE_Y, true );
				if ( MUVec2LenSq( &vMoveDir ) > 0.0f )
				{
					m_AIcommands.bThrust = true;
					m_AIcommands.vMoveDir = vMoveDir;
					m_AIcommands.bRunning = true;
				}
				Vec2 vAimVec = pController->GetDoubleAxisVector( K_CM_COMMAND_AIM_X, K_CM_COMMAND_AIM_Y, false );
				//DebugPrintA("aim: %.2f, %.2f\n", vAimVec.x, vAimVec.y);
				m_AIcommands.vAimVec = vAimVec;

				//reset roll status
				/*
				if (act.nRolling == K_STATE_FINISHED)
				{
					//reset only when thrustX off
					if ((!bPressedLeft) && (!bPressedRight))
						act.nRolling = K_STATE_READY;
				}
				*/

				//interact
				if ( pController->sCommands.keyState[ K_CM_COMMAND_JUMP ] == K_CM_BUTSTATE_JUSTPRESSED )
				{
					m_AIcommands.bInteract = true;
				}
				//FIRE SHOOT
				if ( pController->sCommands.bKeyDown[ K_CM_COMMAND_FIRE1 ] )
				{
					m_AIcommands.eAttackCommand = K_ACT_ATTACK_SHOOTING;
				}
				else if ( pController->sCommands.keyState[ K_CM_COMMAND_RELOAD ] == K_CM_BUTSTATE_JUSTPRESSED )
				{
					m_AIcommands.eAttackCommand = K_ACT_ATTACK_RELOADING;
				}
				else if ( pController->sCommands.bKeyDown[ K_CM_COMMAND_FIRE2 ] )
				{
					m_AIcommands.eAttackCommand = K_ACT_ATTACK_SHOOTING_ALT;
				}
				//lets you use MELEE while holding fire or reloading
				if ( pController->sCommands.keyState[ K_CM_COMMAND_MELEE ] == K_CM_BUTSTATE_JUSTPRESSED )
				{
					m_AIcommands.eAttackCommand = K_ACT_ATTACK_MELEE;
				}
				//RELOAD ON SHOOT - overwrites previous commands
				/*
				if ((pController->sCommands.keyState[K_CM_COMMAND_FIRE1] == K_CM_BUTSTATE_JUSTPRESSED) && (act.pWeaponMain->ammoLeft == 0))
				{
					m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_RELOADING;
				}
				*/

				//when selecting strategic ability only crouch
				/*
				if ( m_arrPlayerSelStrategic[ act.nPlayerOrdinal ] >= 0 )
				{
					m_AIcommands.ResetMoveCommands();
					if ( act.collisionFlags & K_DIRFLAG_DOWN )
					{
						m_AIcommands.bCrouched = true;
					}
					break;
				}
				*/
			}
			break;

			case AI_BEHAVIOR_BARREL_EXPLODING:
			{
			}
			break;
			case AI_BEHAVIOR_HOSTAGE:
			{
				//always set crouched command if actor can crouch
				if ( act._template.eCaps & K_ACT_CAPS_CAN_CROUCH )
					m_AIcommands.bCrouched = true;
				//can he follow targets? does it only once
				if ( act.AIvarBool1 )
				{
					if ( m_AIsensorInfo.pTargetedActor != null )
					{
						//play the verse only once
						if ( act.AIsubState == 0 )
						{
							//							PlayActorSoundVerse(actor, K_LVL_ACT_VERSE_TAUNT);
						}

						Vec2 vDelta = m_AIsensorInfo.pTargetedActor.GetPosHeart() - act.GetPosHeart();
						float fDist = fabs( vDelta.x );
						float fDistMin = 32.0f;// max(K_TILE_SIZE, act.actTemplate.distAttackMin);
						//see if target is already too close
						if ( fDist <= fDistMin )
						{
							act.AIvarBool1 = false;
							break;
						}

						act.AIsubState = 1; //followed target
						m_AIcommands.bCrouched = false;
						//run to target
						m_AIcommands.bThrust = true;
						m_AIcommands.bRunning = true;
						//gets too close
						bool bHasLateralCollisions = ((act.collisionFlags & (K_DIRFLAG_RIGHT | K_DIRFLAG_LEFT)) != 0);
						if ( (fDist <= fDistMin) || (bHasLateralCollisions) )
						{
							act.AIvarBool1 = false;
						}
					}
					else
					{
						if ( act.AIsubState == 1 ) //already followed but lost him
						{
							act.AIvarBool1 = false;
						}
					}
				}
			}
			break;
			case AI_BEHAVIOR_IDLE_CROUCHED:
			{
				m_AIcommands.bCrouched = true;
				//handle fade out duration
				if ( (m_pAIcurrentState->m_arrBehaviors[ m_nAIcurrentBehaviorIdx ].fBehaviorDuration > 0.0f) && (act.AIfvar1 > 0.0f) )
				{
					float fLeftTime = m_pAIcurrentState->m_arrBehaviors[ m_nAIcurrentBehaviorIdx ].fBehaviorDuration - act.m_fAIbehaviorTimer;
					if ( fLeftTime <= act.AIfvar1 )
					{
						//setam comanda de culoare
						m_AIcommands.nColor = DW_COLORALPHA( act.color_ini, fLeftTime / act.AIfvar1 );
					}
				}
			}
			break;
			case AI_BEHAVIOR_PLAY_ANIM:
			{
				//playerii pot schimba directia si pe play anim
				if ( act._template.actorClass == K_LVL_ACT_CLASS_PLAYER )
				{
					CController* pController = UTGetCtrlrMgr().GetControllerByInstanceID( act.nControllerInstanceID );
					if ( pController != null )
					{
						//daca apesi st/dr se intoarce cu fatza in directia respectiva
						bool bPressedRight = (pController->GetAxisVal( K_CM_COMMAND_MOVE_X ) > 0.0f);
						bool bPressedLeft = (pController->GetAxisVal( K_CM_COMMAND_MOVE_X ) < 0.0f);
						//daca apasa ambele butoane nu se misca
						if ( bPressedLeft && bPressedRight )
							bPressedLeft = bPressedRight = false;

						if ( !bPressedLeft && bPressedRight )
						{
							m_AIcommands.bThrust = false;
							//m_AIcommands.nMoveDirX = 1;
						}
						if ( bPressedLeft && !bPressedRight )
						{
							m_AIcommands.bThrust = false;
							//m_AIcommands.nMoveDirX = -1;
						}
					}
				}

				//seteaza comanda de override anim cu valoarea salvata in SetActorAIBehavior din params behavior
				//m_AIcommands.eOverrideAnim = (EActorAnims)act.AIvar1;

				//conditii final (sfarsit animatie)
				//if (act.sprite.animStatus == ANIM_STATUS_FRAMELOCK)
					//bBehaviorFinished = true;
				//modificare alpha daca e setat duration
				if ( m_pAIcurrentState->m_arrBehaviors[ m_nAIcurrentBehaviorIdx ].fBehaviorDuration > 0.0f )
				{
					float fPerc = act.m_fAIbehaviorTimer / m_pAIcurrentState->m_arrBehaviors[ m_nAIcurrentBehaviorIdx ].fBehaviorDuration;
					//setam comanda de culoare
					m_AIcommands.nColor = DW_COLORALPHA( act.color_ini, (1.0f - fPerc) * act.AIfvar2 + fPerc * act.AIfvar1 );
				}
			}
			break;


			case AI_BEHAVIOR_SURPRISED:
			{
				if ( act.AIsubState == 0 )
				{
					act.AIsubState = 1;
				}

				act.AItimer1 -= dTime;
				if ( act.AItimer1 <= 0.0f )
				{
					bBehaviorFinished = true;
				}
			}
			break;

			case AI_BEHAVIOR_WAIT:
			{
				//keep old crouch state
				m_AIcommands.bCrouched = act.bCrouched;
			}
			break;
			case AI_BEHAVIOR_RUN_SCRIPT:
			{
				//se termina behavior-ul cand s-a terminat de rulat scriptul
				if ( act.AIvar1 == 1 ) //wait script end
				{
					if ( act.nRunningScriptUID == 0 )
						bBehaviorFinished = true;
				}
				else
				{
					bBehaviorFinished = true;
				}
			}
			break;
			case AI_BEHAVIOR_DEAD:
			{
				//get inherited death command from other states
				CVariantComplex* cvdeath = act.varAIparams.GetVariantByName( L"nDeathCommand" );

				if ( cvdeath->m_type != CVariantComplex::K_ARGTYPE_NONE )
				{
					m_AIcommands.nDeathCommand = (EActorDeathCommand)cvdeath->m_asINT32;
					//delete the death value after saving it to local var
					act.varAIparams.DeleteVar( L"nDeathCommand" );
				}

				///--- enforce death commands ---
				if ( m_AIcommands.nDeathCommand == K_LVL_ACT_DEATHCMD_NONE )
				{
					//daca nu am animatie de dead face direct splat daca poate (sau daca am primit param de bSplat din Hit Actor)
					/*if ((act.actTemplate.animIDs[K_LVL_ACT_ANIM_DIE][0] == -1) || (act.varAIparams.GetVariantByName(L"bSplat")->m_asBool))
					{
						m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_SPLAT;
					} */
				}
				//cauta params particulari de AI setati din Hit Actor
				CVariantComplex* cvc = act.varAIparams.GetVariantByName( L"nExplode" );
				if ( cvc->m_type != CVariantComplex::K_ARGTYPE_NONE )
				{
					//comanda splat on explode daca e clasa care trebuie
					if ( (act._template.actorClass == K_LVL_ACT_CLASS_HUMAN) || (act._template.actorClass == K_LVL_ACT_CLASS_HOSTAGE) )
						m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_SPLAT;
					//get explo class
					UINT32 unExploUID = act.GetUID();
					if ( act.varAIparams.GetVariantByName( L"bUseDamagerUID" )->m_asBool )
						unExploUID = act.nLastDamageTakenFromUID;
					//generate explo
					AddDoofer_Explo( cvc->m_asUINT32, act.GetPosHeart(), unExploUID, K_LVL_ACT_CLASS_EXPLOSION, Vec2( 0.0f, 0.0f ), &act.bbox );

					//decal explo mark
					//AddDecal(K_LVL_DECAL_LAYER_BACKWALLS, act.GetPosHeart(), ANM_ACTIVES_SPR_DECAL_EXPLOMARKS, randint(3), 0xffffffff);
				}

				///- when the player dies -
				if ( act._template.actorClass == K_LVL_ACT_CLASS_PLAYER )
				{
					//m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_RESET_TO_ZERO;
					//act.varAIparams.SetNamedVarINT32(L"nDeathCommand", K_LVL_ACT_DEATHCMD_RESET_TO_ZERO);
					//#HACK: death timer - waits for the timer before executing the state, only for players
					//press fire to reset timer
					CController* pController = UTGetCtrlrMgr().GetControllerByInstanceID( act.nControllerInstanceID );
					//daca apesi fire dupa o secunda scursa nu mai asteapta timerul
					bool bContinue = false;
					if ( (pController != null) && (act.AItimer1 < K_LVL_PLAYER_DEATH_TIMER - 1.0f) &&
						(m_arrStats[ K_LVL_STATS_PL1_LIVES + act.nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT ] > 0) )
					{
						if ( (pController->sCommands.keyState[ K_CM_COMMAND_FIRE1 ] == K_CM_BUTSTATE_JUSTRELEASED) ||
							(pController->sCommands.keyState[ K_CM_COMMAND_JUMP ] == K_CM_BUTSTATE_JUSTRELEASED) )
						{
							act.AItimer1 = 0.0f;
							//continue only on keypress
							//bContinue = true;
						}
					}

					if ( (act.AItimer1 > 0.0f) && (m_levelState == K_LVL_STATE_PLAYING) )
					{
						act.AItimer1 -= dTime;
						break;
					}

					//continue if we have enough lives
					if ( m_arrStats[ K_LVL_STATS_PL1_LIVES + act.nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT ] > 0 )
						bContinue = true;
					//setam clasa pasiva ca sa putem sa distrugem cadavrul
					act._template.actorClass = K_LVL_ACT_CLASS_HUMAN;

					//daca avem breaching charges aruncate in nivel le dezalocam
					ReleaseBulletType( K_LVL_BULLET_BREACHING_CHARGE, act.GetUID() );

					//release camera
					pPlayerActor[ act.nPlayerOrdinal ] = null;
					if ( bContinue )
					{
						m_arrPlayerSelHotJoin[ act.nPlayerOrdinal ] = (int)g_playerSelScr.m_arrPlayers[ act.nPlayerOrdinal ].eType;
					}
					else
					{
						m_arrPlayerSelHotJoin[ act.nPlayerOrdinal ] = -1; //remove old selection so it doesn't show the hot join icon
					}

					m_nPlayers--;
					m_nPlayersActive--;

					//m_interfaceIGM.Init(&m_sprInterface, pPlayerActor[0], pPlayerActor[1]);
					//m_interfaceIGM.SetHotJoinSelection(act.nPlayerOrdinal, -1);

					//on local play check if other player is suspended and move camera on him
					if ( (!UTApp().IsGameNetworked()) && (!bContinue) )
					{
						for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
						{
							if ( (pPlayerActor[ kk ] != null) && (pPlayerActor[ kk ]->nSuspendedFlags & K_LVL_SUSPENDFLAG_OUTSIDE_SCREEN) )
							{
								m_vCamPosDefault = pPlayerActor[ kk ]->GetPosHeart();
								break;
							}
						}
					}
				}
				else //splat timer - splat corpse if timer is set
				{
					if ( (act._template.eMaterial == K_LVL_MATERIAL_FLESH) && (act.AItimer1 > 0.0f) )
					{
						act.AItimer1 -= dTime;
						if ( act.AItimer1 <= 0.0f )
						{
							act.AItimer1 = 0.0f;
							m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_SPLAT;
						}
					}
				}
				//only flesh can splat
				if ( (m_AIcommands.nDeathCommand == K_LVL_ACT_DEATHCMD_SPLAT) && (act._template.eMaterial != K_LVL_MATERIAL_FLESH) )
				{
					m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_DEALLOCATE;
				}
				//execute script on death if no other important command issued
				if ( m_AIcommands.nDeathCommand == K_LVL_ACT_DEATHCMD_RUNSCRIPT )
				{
					CVariantComplex* cvc2 = act.varAIparams.GetVariantByName( L"sDeathScript" );
					if ( cvc2->m_type == CVariantComplex::K_ARGTYPE_STRING )
					{
						StartScript( cvc2->m_strArg.text, actor );
						//clear script and death command
						act.varAIparams.DeleteVar( L"sDeathScript" );
						m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_NONE;
					}
				}
			}
			break;
			default:
				break;
		}

		///update behavior timer - signal behavior finished when timer expires
		if ( bBehaviorDurationFinished == true )
			bBehaviorFinished = bBehaviorDurationFinished;
	}
	else //skipping AI
	{
		//!!! keep old crouched state if short stun so it doesn't jitter when shot
		if ( act.fStunTimer < K_LVL_MIN_STUN_DIZZY_DURATION )
		{
			m_AIcommands.bCrouched = act.bCrouched;
		}
	}

	///reset decision timer - dupa THINK ca sa pot controla timer-ul direct din AI
	if ( AItimerDecision <= 0.0f )
	{
		AItimerDecision = K_LVL_AI_DECISION_INTERVAL + m_rand.RandFloatSgn( K_LVL_AI_DECISION_INTERVAL_VARIATION );
	}
	///if behavior ended get on to next one
	if ( bBehaviorFinished )
	{
		bool bShortBehavior = false;
		do
		{
			OnActorBehaviorFinished( actor, act.GetCurrentBehavior() );
			SetActorAIBehaviorIdx( actor, m_nAIcurrentBehaviorIdx + 1, bShortBehavior );
		} while ( bShortBehavior );
	}



}

EAIBehaviorType CActorAIComponent::GetCurrentBehavior()
{
	if ( (m_nAIcurrentBehaviorIdx < 0) || (m_pAIcurrentState == null) )
		return AI_BEHAVIOR_EMPTY;
	return m_pAIcurrentState->m_arrBehaviors[ m_nAIcurrentBehaviorIdx ].nType;
}
