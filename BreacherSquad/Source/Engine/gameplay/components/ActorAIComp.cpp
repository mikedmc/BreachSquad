#include "dxstdafx.h"
#include "ActorAIComp.h"

CActorAIComponent::CActorAIComponent(CLevel& levelref) :
	level(levelref),
	m_pAIcurrentState( nullptr ), m_nAIcurrentBehaviorIdx( -1 ), m_fAIbehaviorTimer( 0.0f )
{

}

CActorAIComponent::~CActorAIComponent()
{

}

void CActorAIComponent::Update( CActor& act, float dTime )
{
	//update timeline
	fTimelineAI += dTime;

	//reset previous commands
	AIcommands.Reset();
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
		// only enters once
		if ( AIsensor.evt.nType != K_LVL_AI_EVENT_DEAD )
		{
			AIsensor.b_IsDead = true;
			AIsensor.evt.Set( K_LVL_AI_EVENT_DEAD, act.GetUID(), act._template.actorClass, act.GetPosHeart(), -1.0f, 1.0f );
			//reset targeted actor
			if ( AIsensor.pTargetedActor != nullptr )
			{
				AIsensor.pTargetedActor->FreeRef();
				AIsensor.pTargetedActor = nullptr;
			}
			///THINK: force state decision
			CAIState* newState = act._template.AItemplate->GetHighestPriorityState( K_LVL_AI_EVENT_DEAD, &level.m_rand );
			SetAIState( act, newState );
		}
	}
	else //process low freq sensors only if no message from realtime sensors (more important)
	{
		bool bIgnoreAIEvents = false;
		if ( (m_pAIcurrentState != nullptr) && (m_nAIcurrentBehaviorIdx >= 0) )
			bIgnoreAIEvents = m_pAIcurrentState->m_arrBehaviors[ m_nAIcurrentBehaviorIdx ].bIgnoreEvents;

		///HIGH FREQUENCY SENSORS
		//hit timer (used in some behaviors)
		AIsensor.fTimeSinceHit += dTime;
		//did he get hit? reset time since hit 
		//if (act.nTookDamageFrames > 0)
			//m_AIsensorInfo.fTimeSinceHit = 0.0f;

		///LOW FREQUENCY SENSORS
		AItimerDecision -= dTime;
		if ( (AItimerDecision <= 0.0f) && (!bIgnoreAIEvents) && (AIsensor.m_bEnabled) )
		{
			// reset internal event
			AIsensor.evtInternal.Reset();
			//check for targets or other AI events
			CActor* targetActor = __Sim().GetClosestTarget(&act /*, act._template.foeClassFilter1, act.actTemplate.foeClassFilter2*/);
			if ( targetActor != nullptr )
			{
				//float enemyDst = MUVec2Len( &(targetActor->GetPosHeart() - act.GetPosHeart()) );
				AIsensor.evtInternal.Set( K_LVL_AI_EVENT_SEE_ENEMY, targetActor->GetUID(), targetActor->_template.actorClass, targetActor->GetPosHeart(), 0.0f, 1.0f );
				//#HACK: alerts the other enemies only if enemy class
				if ( act._template.actorClass >= K_ACT_CLASS_ENEMY )
					level.AddAIEvent( K_LVL_AI_EVENT_SOUND_THREAT, targetActor->GetUID(), targetActor->_template.actorClass, targetActor->GetPosHeart(), 200.0f, 0.6f );
				// set target pointer and increase ref
				targetActor->GetRef();
				AIsensor.pTargetedActor = targetActor;
				// enemies overlapping
				/*
				if (act.bbox.Intersects(targetActor->bbox))
				{
					m_AIsensorInfo.fTargetOverlapX = SIGN(act.pos.xy.x - targetActor->pos.xy.x) * ((act.bbox.vHalfSize.x + targetActor->bbox.vHalfSize.x) - fabs(act.pos.xy.x - targetActor->pos.x));
				}
				*/
				// send touching event
				/*
				// useless I think:
				if ( act.bbox.Intersects( targetActor->bbox ) )
				{
					level.AddAIEvent( K_LVL_AI_EVENT_TOUCH_ENEMY, targetActor->GetUID(), targetActor->_template.actorClass, targetActor->GetPosHeart(), enemyDst, 1.0f, act.GetUID() );
				}
				*/
				// write last interacting actor just for the sake of it. It will be rewritten later.
				AIsensor.m_lastInteractingActorUID = targetActor->GetUID();
			}
			else
			{
				//reset targeted actor
				if ( AIsensor.pTargetedActor != nullptr )
				{
					AIsensor.evtInternal.Set( K_LVL_AI_EVENT_LOST_ENEMY, 0, K_ACT_CLASS_ANY, AIsensor.pTargetedActor->pos.xy, 16.0f, 1.0f );
					//reset targeting actor
					AIsensor.pTargetedActor->FreeRef();
					AIsensor.pTargetedActor = nullptr;
				}
				// sometimes it doesn't see the enemy when it gets hit so we force the lost enemy onto him
				else if ( AIsensor.evt.nType == K_LVL_AI_EVENT_GOT_HIT )
				{
					AIsensor.evtInternal.Set( K_LVL_AI_EVENT_LOST_ENEMY, 0, K_ACT_CLASS_ANY, AIsensor.evt.pos, 16.0f, 0.5f );
				}
			}

			///--- select best event ---
			CAIEvent* evt = GetMostImportantAIEvent( act );
			CAIEvent evtFinal = AIsensor.evtInternal;
			if ( ( evt != nullptr ) && ( evt->nType > evtFinal.nType ) )
				evtFinal = *evt;

			if ( evtFinal.nType == K_LVL_AI_EVENT_NONE )
			{
				//nothing important, set idle tick
				evtFinal.Set( K_LVL_AI_EVENT_IDLE_TICK, 0, 0, Vec2( 0.0f, 0.0f ), -1.0f, 1.0f );
			}
			// see if event changed to check for new state
			if ( AIsensor.evt != evtFinal )
			{
				AIsensor.evt = evtFinal;
				if(AIsensor.evt.nType >= 0)
					LOG( L"%s checks event: %s \n",act._template.shID.text , EAIEventTypeNames[AIsensor.evt.nType].text );
				else 
					LOG( L"%s checks event: NONE \n", act._template.shID.text );

				//----------------------------------------
				//	THINK - decide best behavior
				//----------------------------------------
				//daca nu am behavior sau daca behaviorul imi permite sa il intrerup.
				//am comentat verificarea pe behaviorDurationFinished pentru ca mesajul de IDLE_TICK ma scotea dintre behaviors care nu pot fi intrerupte. Ca sa pot intrerupe cand vreau bag un behavior IDLE
				if ( ( m_nAIcurrentBehaviorIdx < 0 ) || ( m_pAIcurrentState->m_arrBehaviors[m_nAIcurrentBehaviorIdx].bCanInterrupt ) /*|| (bBehaviorDurationFinished)*/ )
				{
					CAIState* newState = act._template.AItemplate->GetHighestPriorityState( AIsensor.evt.nType, &level.m_rand );

					//daca vechea stare a fost setata de acelasi mesaj ca si acum si nu are prioritate mai mica nu ar mai trebui setata alta stare ci cel mult dat restart la starea curenta
					if ( ( newState != nullptr ) && ( AIsensor.evt.nType == AIsensor.evt.nType ) && ( newState->nPriority == m_pAIcurrentState->nPriority ) )
					{
						//#MAYBE: reset current behavior if it's the same state?
					}
					else
					{
						if (newState != nullptr)
							LOG(L"evttype:%d set_state: %s\n", AIsensor.evt.nType, newState->name.text);

						//state may also be null when no state is associated with an event
						SetAIState( act, newState );
					}
				}
			}
		}
	}


	//------------------------------------------------------------------------------------------
	//	THINK - run behavior - realtime	- proceseaza AIsensors si scrie doar in AIcommands
	//------------------------------------------------------------------------------------------
	bool bBehaviorFinished = false;

	bool bSkipAI = false;
	if ( (m_pAIcurrentState == nullptr) || (m_nAIcurrentBehaviorIdx < 0) || (act.fStunTimer > 0.0f) )
		bSkipAI = true;

	if ( bBehaviorDurationFinished )
	{
		bBehaviorFinished = true;
		bSkipAI = true;
	}
	//simple way to check if it's time to decide
	bool bTimeToDecide = (AItimerDecision <= 0.0f);

	if ( !bSkipAI ) 
	{
		const EAIBehaviorType eBehaviour = m_pAIcurrentState->m_arrBehaviors[m_nAIcurrentBehaviorIdx].nType;
		switch ( eBehaviour )
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

			case AI_BEHAVIOR_IDLE:
			{
			}
			break;

			case AI_BEHAVIOR_ATTACK:
			{

				if ( ( AIsensor.pTargetedActor == nullptr ) || ( !AIsensor.pTargetedActor->IsAlive() ) )
				{
					bBehaviorFinished = true;
					break;
				}
				
				act.vAim = AIsensor.pTargetedActor->pos.xy - act.pos.xy;
			}
			break;

			case AI_BEHAVIOR_PATROL:
			{
			}
			break;

			case AI_BEHAVIOR_SHOW_ENEMY:
			{
			}
			break;

			case AI_BEHAVIOR_PLAYER_CONTROL:
			{
				CController* pController = UTGetCtrlrMgr().GetControllerByInstanceID( act.nControllerInstanceID );
				//controller not set or removed, skipping AI
				if ( (pController == nullptr) || (pController->nFlags & K_CM_CTRLR_FLAG_PAUSED) || (act.bSuspendInput) )
				{
					//HitActor(actor, -1.0f, 0, 100, K_ACT_CLASS_TRAP);
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
					AIcommands.bThrust = true;
					AIcommands.vMoveDir = vMoveDir;
					AIcommands.bRunning = true;
				}
				Vec2 vAimVec = pController->GetDoubleAxisVector( K_CM_COMMAND_AIM_X, K_CM_COMMAND_AIM_Y, false );
				//DebugPrintA("aim: %.2f, %.2f\n", vAimVec.x, vAimVec.y);
				AIcommands.vAimVec = vAimVec;

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
					AIcommands.bInteract = true;
				}
				//FIRE SHOOT
				if ( pController->sCommands.bKeyDown[ K_CM_COMMAND_FIRE1 ] )
				{
					AIcommands.eAttackCommand = K_ACT_ATTACK_SHOOTING;
				}
				else if ( pController->sCommands.keyState[ K_CM_COMMAND_RELOAD ] == K_CM_BUTSTATE_JUSTPRESSED )
				{
					AIcommands.eAttackCommand = K_ACT_ATTACK_RELOADING;
				}
				else if ( pController->sCommands.bKeyDown[ K_CM_COMMAND_FIRE2 ] )
				{
					AIcommands.eAttackCommand = K_ACT_ATTACK_SHOOTING_ALT;
				}
				//lets you use MELEE while holding fire or reloading
				if ( pController->sCommands.keyState[ K_CM_COMMAND_MELEE ] == K_CM_BUTSTATE_JUSTPRESSED )
				{
					AIcommands.eAttackCommand = K_ACT_ATTACK_MELEE;
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
					AIcommands.bCrouched = true;
				//can he follow targets? does it only once
				if ( AIvarBool1 )
				{
					if ( AIsensor.pTargetedActor != nullptr )
					{
						//play the verse only once
						if ( AIsubState == 0 )
						{
							//PlayActorSoundVerse(actor, K_LVL_ACT_VERSE_TAUNT);
						}

						Vec2 vDelta = AIsensor.pTargetedActor->GetPosHeart() - act.GetPosHeart();
						float fDist = fabs( vDelta.x );
						float fDistMin = 32.0f;// max(K_TILE_SIZE, act.actTemplate.distAttackMin);
						//see if target is already too close
						if ( fDist <= fDistMin )
						{
							AIvarBool1 = false;
							break;
						}

						AIsubState = 1; //followed target
						AIcommands.bCrouched = false;
						//run to target
						AIcommands.bThrust = true;
						AIcommands.bRunning = true;
						//gets too close
						bool bHasLateralCollisions = ((act.collisionFlags & (K_DIRFLAG_RIGHT | K_DIRFLAG_LEFT)) != 0);
						if ( (fDist <= fDistMin) || (bHasLateralCollisions) )
						{
							AIvarBool1 = false;
						}
					}
					else
					{
						if ( AIsubState == 1 ) //already followed but lost him
						{
							AIvarBool1 = false;
						}
					}
				}
			}
			break;
			case AI_BEHAVIOR_IDLE_CROUCHED:
			{
				AIcommands.bCrouched = true;
				//handle fade out duration
				if ( (m_pAIcurrentState->m_arrBehaviors[ m_nAIcurrentBehaviorIdx ].fBehaviorDuration > 0.0f) && (AIfvar1 > 0.0f) )
				{
					float fLeftTime = m_pAIcurrentState->m_arrBehaviors[ m_nAIcurrentBehaviorIdx ].fBehaviorDuration - m_fAIbehaviorTimer;
					if ( fLeftTime <= AIfvar1 )
					{
						//setam comanda de culoare
						AIcommands.nColor = DW_COLORALPHA( act.color_ini, fLeftTime / AIfvar1 );
					}
				}
			}
			break;
			case AI_BEHAVIOR_PLAY_ANIM:
			{
				//playerii pot schimba directia si pe play anim
				if ( act._template.actorClass == K_ACT_CLASS_PLAYER )
				{
					CController* pController = UTGetCtrlrMgr().GetControllerByInstanceID( act.nControllerInstanceID );
					if ( pController != nullptr )
					{
						//daca apesi st/dr se intoarce cu fatza in directia respectiva
						bool bPressedRight = (pController->GetAxisVal( K_CM_COMMAND_MOVE_X ) > 0.0f);
						bool bPressedLeft = (pController->GetAxisVal( K_CM_COMMAND_MOVE_X ) < 0.0f);
						//daca apasa ambele butoane nu se misca
						if ( bPressedLeft && bPressedRight )
							bPressedLeft = bPressedRight = false;

						if ( !bPressedLeft && bPressedRight )
						{
							AIcommands.bThrust = false;
							//m_AIcommands.nMoveDirX = 1;
						}
						if ( bPressedLeft && !bPressedRight )
						{
							AIcommands.bThrust = false;
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
					float fPerc = m_fAIbehaviorTimer / m_pAIcurrentState->m_arrBehaviors[ m_nAIcurrentBehaviorIdx ].fBehaviorDuration;
					//setam comanda de culoare
					AIcommands.nColor = DW_COLORALPHA( act.color_ini, (1.0f - fPerc) * AIfvar2 + fPerc * AIfvar1 );
				}
			}
			break;


			case AI_BEHAVIOR_SURPRISED:
			{
				if ( AIsubState == 0 )
				{
					AIsubState = 1;
				}

				AItimer1 -= dTime;
				if ( AItimer1 <= 0.0f )
				{
					bBehaviorFinished = true;
				}
			}
			break;

			case AI_BEHAVIOR_WAIT:
			{
				//keep old crouch state
				AIcommands.bCrouched = act.bCrouched;
			}
			break;
			case AI_BEHAVIOR_RUN_SCRIPT:
			{
				if ( AIvar1 == 1 ) //wait script end
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
				CVariant cvdeath = act.varAIparams[ L"nDeathCommand" ];

				if ( cvdeath.IsSet() )
				{
					AIcommands.nDeathCommand = (EActorDeathCommand)cvdeath.m_asINT32;
					//delete the death value after saving it to local var
					act.varAIparams.DeleteVar( L"nDeathCommand" );
				}

				///--- enforce death commands ---
				if ( AIcommands.nDeathCommand == K_LVL_ACT_DEATHCMD_NONE )
				{
					//daca nu am animatie de dead face direct splat daca poate (sau daca am primit param de bSplat din Hit Actor)
					/*if ((act.actTemplate.animIDs[K_LVL_ACT_ANIM_DIE][0] == -1) || (act.varAIparams[L"bSplat")->m_asBool))
					{
						m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_SPLAT;
					} */
				}
				//cauta params particulari de AI setati din Hit Actor
				CVariant cvc = act.varAIparams[ L"nExplode" ];
				if ( cvc.IsSet() )
				{
					//comanda splat on explode daca e clasa care trebuie
					if ( (act._template.actorClass == K_ACT_CLASS_ENEMY) || (act._template.actorClass == K_ACT_CLASS_HOSTAGE) )
						AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_SPLAT;
					//get explo class
					UINT32 unExploUID = act.GetUID();
					if ( act.varAIparams[ L"bUseDamagerUID" ].m_asBool )
						unExploUID = act.nLastDamageTakenFromUID;
					//generate explo
					level.AddDoofer_Explo( cvc.m_asUINT32, act.GetPosHeart(), unExploUID, K_ACT_CLASS_EXPLOSION, Vec2( 0.0f, 0.0f ), &act.bbox );

					//decal explo mark
					//AddDecal(K_LVL_DECAL_LAYER_BACKWALLS, act.GetPosHeart(), ANM_ACTIVES_SPR_DECAL_EXPLOMARKS, randint(3), 0xffffffff);
				}

				///- when the player dies -
				if ( act._template.actorClass == K_ACT_CLASS_PLAYER )
				{
					//m_AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_RESET_TO_ZERO;
					//act.varAIparams.SetVarINT32(L"nDeathCommand", K_LVL_ACT_DEATHCMD_RESET_TO_ZERO);
					//#HACK: death timer - waits for the timer before executing the state, only for players
					//press fire to reset timer
					CController* pController = UTGetCtrlrMgr().GetControllerByInstanceID( act.nControllerInstanceID );
					//daca apesi fire dupa o secunda scursa nu mai asteapta timerul
					bool bContinue = false;
					if ( (pController != null) && (AItimer1 < K_LVL_PLAYER_DEATH_TIMER - 1.0f) &&
						(level.m_arrStats[ K_LVL_STATS_PL1_LIVES + act.nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT ] > 0) )
					{
						if ( (pController->sCommands.keyState[ K_CM_COMMAND_FIRE1 ] == K_CM_BUTSTATE_JUSTRELEASED) ||
							(pController->sCommands.keyState[ K_CM_COMMAND_JUMP ] == K_CM_BUTSTATE_JUSTRELEASED) )
						{
							AItimer1 = 0.0f;
							//continue only on keypress
							//bContinue = true;
						}
					}

					if ( (AItimer1 > 0.0f) && (level.m_levelState == K_LVL_STATE_PLAYING) )
					{
						AItimer1 -= dTime;
						break;
					}

					//continue if we have enough lives
					if ( level.m_arrStats[ K_LVL_STATS_PL1_LIVES + act.nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT ] > 0 )
						bContinue = true;
					//setam clasa pasiva ca sa putem sa distrugem cadavrul
					act._template.actorClass = K_ACT_CLASS_ENEMY;

					//daca avem breaching charges aruncate in nivel le dezalocam
					level.ReleaseBulletType( K_LVL_BULLET_BREACHING_CHARGE, act.GetUID() );

					//release camera
					level.pPlayerActor[ act.nPlayerOrdinal ] = null;
					if ( bContinue )
					{
						level.m_arrPlayerSelHotJoin[ act.nPlayerOrdinal ] = (int)g_playerSelScr.m_arrPlayers[ act.nPlayerOrdinal ].eType;
					}
					else
					{
						level.m_arrPlayerSelHotJoin[ act.nPlayerOrdinal ] = -1; //remove old selection so it doesn't show the hot join icon
					}

					level.m_nPlayers--;
					level.m_nPlayersActive--;

					//m_interfaceIGM.Init(&m_sprInterface, pPlayerActor[0], pPlayerActor[1]);
					//m_interfaceIGM.SetHotJoinSelection(act.nPlayerOrdinal, -1);

					//on local play check if other player is suspended and move camera on him
					if ( (!UTApp().IsGameNetworked()) && (!bContinue) )
					{
						for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
						{
							if ( (level.pPlayerActor[ kk ] != null) && (level.pPlayerActor[ kk ]->nSuspendedFlags & K_LVL_SUSPENDFLAG_OUTSIDE_SCREEN) )
							{
								level.m_vCamPosDefault = level.pPlayerActor[ kk ]->GetPosHeart();
								break;
							}
						}
					}
				}
				else //splat timer - splat corpse if timer is set
				{
					if ( (act._template.eMaterial == K_LVL_MATERIAL_FLESH) && (AItimer1 > 0.0f) )
					{
						AItimer1 -= dTime;
						if ( AItimer1 <= 0.0f )
						{
							AItimer1 = 0.0f;
							AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_SPLAT;
						}
					}
				}
				//only flesh can splat
				if ( (AIcommands.nDeathCommand == K_LVL_ACT_DEATHCMD_SPLAT) && (act._template.eMaterial != K_LVL_MATERIAL_FLESH) )
				{
					AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_DEALLOCATE;
				}
				//execute script on death if no other important command issued
				if ( AIcommands.nDeathCommand == K_LVL_ACT_DEATHCMD_RUNSCRIPT )
				{
					CVariant cvc2 = act.varAIparams[ L"sDeathScript" ];
					if ( cvc2.eType == CVariant::K_ARGTYPE_STRING )
					{
						act.StartScript( cvc2.m_strArg.text );
						//clear script and death command
						act.varAIparams.DeleteVar( L"sDeathScript" );
						AIcommands.nDeathCommand = K_LVL_ACT_DEATHCMD_NONE;
					}
				}
			}
			break;
			default:
				ErrorBox( K_ERR_WARNING, L"ActorAIComp::Update: Illegal behaviour!" );
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
			AIcommands.bCrouched = act.bCrouched;
		}
	}

	///reset decision timer - dupa THINK ca sa pot controla timer-ul direct din AI
	if ( AItimerDecision <= 0.0f )
	{
		AItimerDecision = K_LVL_AI_DECISION_INTERVAL + level.m_rand.RandFloatSgn( K_LVL_AI_DECISION_INTERVAL_VARIATION );
	}
	///if behavior ended get on to next one
	if ( bBehaviorFinished )
	{
		bool bShortBehavior = false;
		do
		{
			OnActorBehaviorFinished( act, act.GetCurrentBehavior() );
			SetActorAIBehaviorIdx( act, m_nAIcurrentBehaviorIdx + 1, bShortBehavior );
		} while ( bShortBehavior );
	}



}

EAIBehaviorType CActorAIComponent::GetCurrentBehavior()
{
	if ( (m_nAIcurrentBehaviorIdx < 0) || (m_pAIcurrentState == null) )
		return AI_BEHAVIOR_EMPTY;
	return m_pAIcurrentState->m_arrBehaviors[ m_nAIcurrentBehaviorIdx ].nType;
}

void CActorAIComponent::OnActorBehaviorFinished( CActor& actor, EAIBehaviorType eOldBehavior )
{
	switch ( eOldBehavior )
	{
		case AI_BEHAVIOR_HUMAN_SHIELD_ATTACK:
		{
		}
		break;
	}

}

CAIEvent* CActorAIComponent::GetMostImportantAIEvent( CActor& act, EAIEventType eTypeFilter /*= K_LVL_AI_EVENT_ANY */ )
{
	CAIEvent* returnEvent = nullptr;
	float mindistSq = 100000.0f;

	for ( int kk = 0; kk < level.m_arrAIevents.GetSize(); kk++ )
	{
		CAIEvent* evt = level.m_arrAIevents[ kk ];
		UINT32 callerUID = act.GetUID();
		//ignora mesajele initiate de el insusi
		if ( (evt->ownerUID == callerUID) || (evt->fDuration <= 0.0f) )
			continue;
		//type filter?
		if ( (eTypeFilter > K_LVL_AI_EVENT_NONE) && (evt->nType != eTypeFilter) )
			continue;
		//ignore actor if different from class foe filters
		/*
		int nIgnore = 0, nIgnoreConditions = 0;
		if (callerActor->actTemplate.foeClassFilter1 != K_ACT_CLASS_ANY)
		{
			nIgnoreConditions++;
			if (evt->ownerClass != callerActor->actTemplate.foeClassFilter1)
				nIgnore++;
		}
		if (callerActor->actTemplate.foeClassFilter2 != K_ACT_CLASS_ANY)
		{
			nIgnoreConditions++;
			if (evt->ownerClass != callerActor->actTemplate.foeClassFilter2)
				nIgnore++;
		}
		if ((nIgnoreConditions > 0) && (nIgnore == nIgnoreConditions))
			continue;
			*/
		//verific distanta (daca raza event nu e infinita adica negativa)
		float evtdstsq = 0.0f;
		if ( evt->fRadius > 0.0f )
		{
			evtdstsq = MUVec2LenSq( &(act.GetPosHeart() - evt->pos) );
			if ( evtdstsq > evt->fRadius * evt->fRadius )
				continue;
		}
		//verific sa am prioritate mai mare sau egala cu cea curenta si distanta mai mica
		if ( returnEvent != null )
		{
			//daca are prioritate mai mica il sare
			if ( evt->nType < returnEvent->nType )
			{
				continue;
			}
			else if ( (evt->nType == returnEvent->nType) && (evtdstsq > mindistSq) ) //daca are aceeasi prioritate dar este mai departe il sare
			{
				continue;
			}
		}
		//see if we have ignored events
		if ( act._template.AItemplate->m_arrIgnoredEvents.Count() > 0 )
		{
			if ( act._template.AItemplate->m_arrIgnoredEvents.IndexOf( evt->nType ) >= 0 )
				continue;
		}
		//dupa ce am exclus eventurile ce se puteau exclude:
		//verific linie directa, cel mai costisitor test, sau daca e event cu raza infinita (fara pozitie)
		if ( (evt->fRadius <= 0.0f) || (level.IsLineOfSight( act.GetPosHeart(), evt->pos )) )
		{
			//daca eventul este mai aproape sau daca eventul e mai important decat cel initial
			if ( (evtdstsq < mindistSq) || ((returnEvent != null) && (evt->nType > returnEvent->nType)) )
			{
				mindistSq = evtdstsq;
				returnEvent = evt;
			}
		}
	}

	return returnEvent;
}

void CActorAIComponent::SetAIState( CActor& actor, CAIState* pNewState )
{
	if ( (m_pAIcurrentState == pNewState) || (pNewState == nullptr) )
		return;

	LOG( L"->SetState: %s - %s", actor._template.shID.text, pNewState->name.text );

	///1. clean exit old state:
	OnActorBehaviorFinished( actor, actor.GetCurrentBehavior() );
	///2. sets the new behavior
	AIsensor.m_bEnabled = true; //enable sensors on new state
	m_pAIcurrentState = pNewState;
	int newBehaviorIdx = -1; //defaults on no behavior
							 //daca am stare not null si are behaviors il setez pe primul
	if ( (m_pAIcurrentState != nullptr) && (m_pAIcurrentState->m_arrBehaviors.nCount > 0) )
		newBehaviorIdx = 0;

	bool bShortBehavior = false;
	do
	{
		SetActorAIBehaviorIdx( actor, newBehaviorIdx, bShortBehavior );
		if ( bShortBehavior )
		{
			OnActorBehaviorFinished( actor, actor.GetCurrentBehavior() );
			newBehaviorIdx++;
		}
	} while ( bShortBehavior );

}

bool CActorAIComponent::SetAIState( CActor& actor, WCHAR * strStateName )
{
	CAIState* newstate = actor._template.AItemplate->GetAIStateByName( strStateName );
	if ( newstate == null )
	{
		LOG( L"CLevel::SetActorAIState - state not found! %s\n", strStateName );
		return false;
	}
	//everything ok, set state
	SetAIState( actor, newstate );
	return true;
}


bool CActorAIComponent::SetActorAIBehaviorIdx( CActor& actor, int nBehaviorIdx, bool &ret_bFinished )
{
	//by default all states need update
	ret_bFinished = false;
	if ( (nBehaviorIdx < 0) || (m_pAIcurrentState == nullptr) || (m_pAIcurrentState->m_arrBehaviors.nCount <= 0) )
	{
		m_nAIcurrentBehaviorIdx = -1;
		ret_bFinished = true;
		return false;
	}

	m_nAIcurrentBehaviorIdx = nBehaviorIdx % m_pAIcurrentState->m_arrBehaviors.nCount;
	CAIBehavior* pNewBehavior = &m_pAIcurrentState->m_arrBehaviors[ m_nAIcurrentBehaviorIdx ];
	// reset behavior timer
	m_fAIbehaviorTimer = 0.0f;

	AIcommands.Reset();
	actor.fFOVPercent = 1.0f;

	switch ( pNewBehavior->nType )
	{
		case AI_BEHAVIOR_EMPTY:
		{
			//state doesn't need update
			ret_bFinished = true;
		}
		break;
		case AI_BEHAVIOR_SET_STATE:
		{
			CVariant* vc = &pNewBehavior->m_vcolParams[ L"sState" ];
			if ( vc->eType != CVariant::K_ARGTYPE_STRING )
			{
				ErrorBox( K_ERR_WARNING, L"AI_BEHAVIOR_SET_STATE: sState arg not set or wrong type!" );
				break;
			}

			CAIState* newstate = actor._template.AItemplate->GetAIStateByName( vc->m_strArg );
			if ( newstate == null )
			{
				LOG( L"AI_BEHAVIOR_SET_STATE - state not found! %s\n", vc->m_strArg.text );
				break;
			}
			//everything ok, set state
			SetAIState( actor, newstate );
			//!!! make sure we stay:
			ret_bFinished = false;
		}
		break;
		case AI_BEHAVIOR_TELEPORT_TO_TOUCHABLE_TARGET:
		{
			/*
			if ( (actor->pClosestTouchable == null) || (actor->pClosestTouchable->pTarget == null) )
			{
				LOG( L"AI_BEHAVIOR_TELEPORT_TO_TOUCHABLE_TARGET::touchable is null or touchable target is null!" );
				break;
			}

			//teleportam playerul pe targetul lui closest touchable. Se presupune ca scriptul de pe touchable ii activeaza starea de SOLO_TELEPORT
			actor->SetPos( actor->pClosestTouchable->pTarget->pos.xyz );
			//set open frame (if necessary) on target
			IActiveInterface* active = actor->pClosestTouchable->pTarget;
			if ( (active->AIstate == K_AI_STATE_ACTIVE_DOORFACE_AUTOCLOSE) || (active->AIstate == K_AI_STATE_ACTIVE_TEAM_TELEPORTER_2FRAMES) )
			{
				bool bDontChangeFrames = (bool)(active->act.varAIparams[ L"b_DontChangeFrames" )->m_asBool);
				if ( !bDontChangeFrames )
					active->AItimer1 = 1.0f;
			}
			//state doesn't need update
			ret_bFinished = true;
			*/
		}
		break;
		case AI_BEHAVIOR_IDLE_CROUCHED:
		{
			//save fadeout duration
			AIfvar1 = pNewBehavior->m_vcolParams[ L"fFadeOutDuration" ].asFloat();
		}
		break;
		case AI_BEHAVIOR_HOSTAGE:
		{
			AIsubState = 0;
			//actor doesn't try to escape:
			AIvarBool1 = false;
			//can hostage escape?
			float fProbability = pNewBehavior->m_vcolParams[ L"fRunProbability" ].asFloat();
			if ( level.m_rand.RandFloat( 100.0f ) < fProbability * 100.0f )
			{
				//we have a runner!
				AIvarBool1 = true;
			}
		}
		break;
		case AI_BEHAVIOR_FLY_AWAY:
		{
			actor.bHasGravity = false;
		}
		break;
		case AI_BEHAVIOR_IDLE:
		{
			AIcommands.vAimVec = { -100.0f, -100.0f };
		}
		break;
		case AI_BEHAVIOR_SET_ANIMSET:
		{
			actor.SetAnimSet( pNewBehavior->m_vcolParams[ L"nSet" ].m_asINT32 );
			//state doesn't need update
			ret_bFinished = true;
		}
		break;
		case AI_BEHAVIOR_SET_CAPS:
		{
			CVariant* cvNotATarget = &pNewBehavior->m_vcolParams[ L"nNotATarget" ];
			if ( cvNotATarget->eType != CVariant::K_ARGTYPE_NONE )
			{
				bool bVal = (cvNotATarget->m_asINT32 != 0);
				if ( bVal )
					actor._template.eCaps |= K_ACT_CAPS_NOT_A_TARGET;
				else
					actor._template.eCaps &= ~K_ACT_CAPS_NOT_A_TARGET;
			}
			//state doesn't need update
			ret_bFinished = true;
		}
		break;
		case AI_BEHAVIOR_BARREL_EXPLODING:
		{
			//burns with flame?
			CVariant* cve = &pNewBehavior->m_vcolParams[ L"nCanBurn" ];
			AIvarBool1 = true;
			if ( (cve->eType != CVariant::K_ARGTYPE_NONE) && (cve->asInt32() == 0) )
				AIvarBool1 = false;

			AIsubState = 0;
			//setez din start comanda de explode ca atunci cand trece in dead sa explodeze
			actor.varAIparams.SetVarUINT32( L"nExplode", hash_EXPLO_BARREL );
			//special value that tells the engine that the explosion will have the last damager's UID so we can transmit barrel kills to players
			actor.varAIparams.SetVarBool( L"bUseDamagerUID", true );
		}
		break;
		case AI_BEHAVIOR_FLEE:
		{
		}
		break;
		case AI_BEHAVIOR_BLIND_RUN:
		{
		}
		break;
		case AI_BEHAVIOR_HOLD_POSITION:
		{
		}
		break;
		case AI_BEHAVIOR_PATROL:
		{
			//save wait timer
			AIfvar1 = pNewBehavior->m_vcolParams[ L"fWaitTimer" ].asFloat();
			AItimer1 = 0.0f;
			//patrol faster?
			AIvarBool1 = (pNewBehavior->m_vcolParams[ L"nRun" ].asInt32() != 0);
			//can he open doors?
			AIvarBool2 = (pNewBehavior->m_vcolParams[ L"nOpenUnlockedDoors" ].asInt32() != 0);
		}
		break;
		case AI_BEHAVIOR_PATROL_BREAK_DOORS:
		{
			//save wait timer
			AIfvar1 = pNewBehavior->m_vcolParams[ L"fWaitTimer" ].asFloat();
			AItimer1 = 0.0f;
			//patrol faster?
			AIvarBool1 = (pNewBehavior->m_vcolParams[ L"nRun" ].asInt32() != 0);
			//set on patroling
			AIsubState = 0;
			//break door probability
			AIfvar2 = pNewBehavior->m_vcolParams[ L"fBreakProb" ].asFloat();
		}
		break;
		case AI_BEHAVIOR_RUN_AWAY:
		{
			//running direction - to be set later on
			AIvar1 = 0;
			//can he open doors?
			AIvarBool2 = (pNewBehavior->m_vcolParams[ L"nOpenUnlockedDoors" ].asInt32() != 0);
		}
		break;
		case AI_BEHAVIOR_WAIT_FOR_ACTION:
		{
			//var that tells the enemy when he can shoot
			AIfvar1 = pNewBehavior->m_vcolParams[ L"fShootPeriod" ].asFloat();
			//timer that keeps actual time
			AItimer1 = AIfvar1;
		}
		break;
		case AI_BEHAVIOR_DETONATE_NEARBY:
		{
		}
		break;
		case AI_BEHAVIOR_CHANGE_COLOR:
		{
			float fDuration = pNewBehavior->m_vcolParams[ L"fTotalDuration" ].m_asFloat;
			CLAMP( fDuration, 0.0f, 60.0f );

			float fAlpha = 0.0f;
			CVariant* cvc = &pNewBehavior->m_vcolParams[ L"fAlpha" ];
			if ( cvc->eType == CVariant::K_ARGTYPE_FLOAT )
				fAlpha = cvc->m_asFloat;
			CLAMP( fAlpha, 0.0f, 1.0f );

			AItimer1 = 0.0f;
			AItimer2 = fDuration;
			AIfvar1 = fAlpha;
		}
		break;
		case AI_BEHAVIOR_PLAY_ANIM:
		{
			/*
			//salvez identificatorul animatiei
			CVariant* cvc = pNewBehavior->m_vcolParams[L"sAnimIdentifier");
			if (cvc->m_type == CVariant::K_ARGTYPE_STRING)
			{
				actor->AIvar1 = GetListIndexByName(cvc->m_strArg.text, EActorAnimNames, K_LVL_ACT_ANIMS_CNT);
			}
			else
			{
				ErrorBox(K_ERR_WARNING, L"Behavior PLAY ANIM sAnimIdentifier not set!");
			}

			//save dest alpha param (defaults on 1.0)
			actor->AIfvar1 = 1.0f;
			if (pNewBehavior->m_vcolParams[L"fDestAlpha")->m_type != CVariant::K_ARGTYPE_NONE)
			{
				actor->AIfvar1 = pNewBehavior->m_vcolParams[L"fDestAlpha")->asFloat();
			}
			//save actual alpha
			actor->AIfvar2 = D3DCOLOR_GETFALPHA(actor->color);
			*/
		}
		break;
		case AI_BEHAVIOR_RUN_SCRIPT:
		{
			bool bWaitScriptEnd = false;
			CVariant* cve = &pNewBehavior->m_vcolParams[ L"bWaitScriptEnd" ];
			if ( cve->IsSet() )
				bWaitScriptEnd = cve->m_asBool;
			AIvar1 = 0;
			if ( bWaitScriptEnd )
				AIvar1 = 1;

			bool bTouchTarget = false;
			CVariant* cvb = &pNewBehavior->m_vcolParams[ L"bTouchTarget" ];
			if ( cvb->IsSet() )
				bTouchTarget = cvb->m_asBool;

			UINT32 nScriptOverride = 0;
			CVariant* cvc = &pNewBehavior->m_vcolParams[ L"sScriptOverride" ];
			if ( cvc->IsSet() )
			{
				nScriptOverride = cvc->m_strArg.textHash;
			}
			//Run script
			actor.Touch( actor.GetUID(), 0.0f, nScriptOverride, bTouchTarget );
		}
		break;
		case AI_BEHAVIOR_PLAY_VERSE:
		{
			CVariant* cvc = &pNewBehavior->m_vcolParams[ L"sVerseName" ];
			if ( cvc->eType == CVariant::K_ARGTYPE_STRING )
			{
				EActorSoundVerse eVerse = (EActorSoundVerse)GetListIndexByNameHash( cvc->m_strArg.getHash(), EActorSoundVerseNames, K_LVL_ACT_VERSES_COUNT );
				if ( eVerse != K_LVL_ACT_VERSE_EMPTY )
				{
					//					PlayActorSoundVerse(actor, eVerse);
				}
				else
				{
					ErrorBox( K_ERR_WARNING, L"BEHAVIOR_PLAY_VERSE- Verse name not found!" );
				}
			}
			else
			{
				ErrorBox( K_ERR_WARNING, L"BEHAVIOR_PLAY_VERSE- sVerseName param not found!" );
			}
			//state doesn't need update
			ret_bFinished = true;
		}
		break;
		case AI_BEHAVIOR_GENERATE_EFFECT:
		{
			float fSize = 1.0f;
			CVariant* cvb = &pNewBehavior->m_vcolParams[ L"fSize" ];
			if ( cvb->IsSet() )
				fSize = cvb->m_asFloat;

			CVariant* cvc = &pNewBehavior->m_vcolParams[ L"sEffectType" ];
			if ( cvc->eType == CVariant::K_ARGTYPE_STRING )
			{
				level.GenerateEffect( cvc->m_strArg, actor.GetPosHeart(), fSize );
			}
			else
			{
				ErrorBox( K_ERR_WARNING, L"BEHAVIOR_GENERATE_EFFECT - sEffectType parameter not found!" );
			}
			//state doesn't need update
			ret_bFinished = true;
		}
		break;

		case AI_BEHAVIOR_SURPRISED:
		{
			/*
			//se intoarce catre event
			if (actor->m_AIsensorInfo.m_AIevent.nType > K_LVL_AI_EVENT_IDLE_TICK)
			{
				actor->m_AIcommands.nLookDirX = SIGN(actor->m_AIsensorInfo.m_AIevent.pos.x - actor->pos.x);
			}
			//set wait timer
			actor->AItimer1 = pNewBehavior->m_vcolParams[L"fWaitTimer")->asFloat();
			actor->AIsubState = 0;

			*/
		}
		break;

		case AI_BEHAVIOR_HUMAN_SHIELD_ATTACK:
		{
			AItargetUID = 0; //unset target ID (this will be the hostage UID)
			AIsubState = 0; //0-looking for hostage, 1-normal attack
			AIvarBool1 = true;	//decide movement helper var
			AItimer1 = 0.0f;
		}
		break;
		case AI_BEHAVIOR_GET_IN_COVER:
		{
			AIsubState = 0;
		}
		break;
		case AI_BEHAVIOR_GUNPOINT_HOSTAGE:
		{
			AIsubState = 0;
			//save execute delay
			AIfvar1 = pNewBehavior->m_vcolParams[ L"fExecuteDelay" ].asFloat();
			if ( AIfvar1 <= 0.0f )
				AIfvar1 = 2.0f; //defaults on 0
		}
		break;
		case AI_BEHAVIOR_ATTACK_COVER:
		{
			AIvarBool1 = true; //decide movement helper var
			AIsubState = 0;
			AItimer1 = 0.0f;	//generic timer for decision making
		}
		break;
		case AI_BEHAVIOR_ATTACK_BACKSTAB:
		case AI_BEHAVIOR_ATTACK_HITNRUN:
		case AI_BEHAVIOR_ATTACK:
		{
			AIvarBool1 = true; //decide movement helper var
			AIsubState = 0;
			AItimer1 = 0.0f;	//generic timer for decision making
		}
		break;
		case AI_BEHAVIOR_SUICIDE:
		{
			actor.bCrouched = false;
			actor.fStunTimer = 0.0f;

			actor.fLife = 0.0f; //kill it
			//trateaza death commands din script
			EActorDeathCommand dcmd = K_LVL_ACT_DEATHCMD_NONE;
			CVariant* cvc = &pNewBehavior->m_vcolParams[ L"sDeathCommand" ];
			if ( cvc->eType == CVariant::K_ARGTYPE_STRING )
			{
				int ndcmd = GetListIndexByName( cvc->m_strArg.text, EActorDeathCommandNames, K_LVL_ACT_DEATHCMD_CNT );
				//daca avem comanda de death o trimitem mai departe
				if ( ndcmd >= 0 )
					actor.varAIparams.SetVarINT32( L"nDeathCommand", ndcmd );
			}
			//state doesn't need update
			ret_bFinished = true;
		}
		break;
		case AI_BEHAVIOR_DEAD:
		{
			//#TODO: shouldn't make changes to the actor maybe?... actor should take decisions based on the states
			//#TODO: toata partea de die ar trebui rescrisa

			//make sure he's dead!
			bool bSpawnedDead = false;
			if ( actor.fLife > 0.0f )
			{
				actor.fLife = 0.0f;
				bSpawnedDead = true;
			}
			actor.fArmor = 0.0f;

			actor.bCrouched = false;
			actor.fStunTimer = 0.0f;
			//death timer for players or splat timer for others
			AItimer1 = 0.0f;
			if ( actor._template.actorClass != K_ACT_CLASS_PLAYER )
			{
				CVariant* cvt = &pNewBehavior->m_vcolParams[ L"fSplatTimer" ];
				if ( cvt->eType == CVariant::K_ARGTYPE_FLOAT )
					AItimer1 = cvt->asFloat();
			}

			//remove icons
			AIcommands.ResetMoveCommands();
			//reset color
			AIcommands.nColor = actor.color_ini;
			//stop weapons
			actor.Weapons()->StopReloading();
			//trateaza death commands din script
			EActorDeathCommand dcmd = K_LVL_ACT_DEATHCMD_NONE;
			CVariant* cvc = &pNewBehavior->m_vcolParams[ L"sDeathCommand" ];
			if ( cvc->eType == CVariant::K_ARGTYPE_STRING )
			{
				dcmd = (EActorDeathCommand)GetListIndexByName( cvc->m_strArg.text, EActorDeathCommandNames, K_LVL_ACT_DEATHCMD_CNT );
				//daca avem comanda de death o trimitem mai departe
				if ( dcmd >= K_LVL_ACT_DEATHCMD_NONE )
					actor.varAIparams.SetVarINT32( L"nDeathCommand", (int)dcmd );
			}
			//trateaza death script
			CVariant* cvs = &pNewBehavior->m_vcolParams[ L"sDeathScript" ];
			if ( cvc->eType == CVariant::K_ARGTYPE_STRING )
			{
				actor.varAIparams.AddVariant( cvs );
			}

			if ( actor._template.actorClass == K_ACT_CLASS_PLAYER )
			{
				//timerul este folosit ca sa nu sara camera de pe cadavru prea repede
				AItimer1 = K_LVL_PLAYER_DEATH_TIMER;
				//daca nu mai are vieti pun un timer mai mic dar il pun totusi ca sa nu sara camera prea repede
				if ( level.m_arrStats[ K_LVL_STATS_PL1_LIVES + actor.nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT ] <= 0 )
					AItimer1 = K_LVL_PLAYER_DEATH_TIMER * 0.25f;

				//actor->act.varAIparams.SetVarINT32(L"nDeathCommand", K_LVL_ACT_DEATHCMD_RESET_TO_ZERO);

				level.m_arrPlayerSelStrategic[ actor.nPlayerOrdinal ] = -1;
				//m_interfaceIGM.SetStrategicSelection(actor->nPlayerOrdinal, -1);
				//dam remove la particles de pe interfata cand moare un player
				__Particles().RemoveAllFromLayer( K_PART_LAYER_INTERFACE_LIGHT );
			}
			else if ( actor._template.actorClass >= K_ACT_CLASS_ENEMY )
			{
				if ( !bSpawnedDead )
				{
					//counts online coop victims too but keeps achievements separated (steam counter)
					App_IncreaseGamestat( K_MEMID_GAMESTATS_ENEMIES_KILLED, 1 );
					//statistics for each class
					CActor* pPlayer = level.GetPlayerByUID( AIsensor.m_lastInteractingActorUID );
					if ( (pPlayer != null) && (!level.IsNetworkPlayer( pPlayer )) )
					{
						switch ( g_playerSelScr.m_arrPlayers[ pPlayer->nPlayerOrdinal ].eType )
						{
							case K_PSS_CLASS_ASSAULTER:
								App_IncreaseGamestat( K_MEMID_GAMESTATS_KILLS_ASSAULTER );
								break;
							case K_PSS_CLASS_BREACHER:
								App_IncreaseGamestat( K_MEMID_GAMESTATS_KILLS_BREACHER );
								break;
							case K_PSS_CLASS_SHIELD:
								App_IncreaseGamestat( K_MEMID_GAMESTATS_KILLS_SHIELD );
								break;
							case K_PSS_CLASS_FBI_AGENT:
								App_IncreaseGamestat( K_MEMID_GAMESTATS_KILLS_FBI );
								break;
							case K_PSS_CLASS_RECON:
								App_IncreaseGamestat( K_MEMID_GAMESTATS_KILLS_RECON );
								break;
							case K_PSS_CLASS_OFFDUTYGUY:
								App_IncreaseGamestat( K_MEMID_GAMESTATS_KILLS_OFFDUTY );
								break;
						}
					}

				}

			}

			//play death verses
			if ( (dcmd == K_LVL_ACT_DEATHCMD_NONE) && (!bSpawnedDead) )
			{
				//PlayActorSoundVerse(actor, K_LVL_ACT_VERSE_DIE);
			}

			//hostages specials
			/*
			if ( actor->_template.actorClass == K_ACT_CLASS_HOSTAGE )
			{
				//save stats for saviour only if deallocating by itself (not killed)
				if ( dcmd == K_LVL_ACT_DEATHCMD_DEALLOCATE )
				{
					UINT32 nToucherUID = actor->act.varAIparams[ L"nToucherUID" )->m_asUINT32;
					CActor* pact = GetPlayerByUID( nToucherUID );
					if ( pact )
					{
						m_arrStats[ K_LVL_STATS_PL1_HOSTAGES_SAVED + pact->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT ]++;
					}
					else
					{
						ErrorBox( K_ERR_WARNING, L"Hostage save went unnoticed! level %d chapter %d", m_nLoadedLevel + 1, m_nLoadedChapter + 1 );
					}

					App_IncreaseGamestat( K_MEMID_GAMESTATS_HOSTAGES_SAVED );
					//GiveStrategicPoints(actor->actTemplate.fStrategicPoints, &Vec2(actor->bbox.vCenter.x, actor->bbox.vMin.y));
					//make sure we release it on the next frame
					actor->Kill();
				}
				else //hostage killed
				{
					CActor* pPlayer = GetPlayerByUID( actor->nLastDamageTakenFromUID );
					if ( pPlayer != null )
					{
						HitActor( pPlayer, pPlayer->fLife * 0.25f, 0, K_ACT_CLASS_TRAP, null,
							K_LVL_BULLET_FLAG_NO_IMPACT_PARTICLES | K_LVL_BULLET_FLAG_NOT_BALLISTIC | K_LVL_BULLET_FLAG_IGNORE_ARMOR | K_LVL_BULLET_FLAG_IGNORE_COVER | K_LVL_BULLET_FLAG_NO_DECALS, 10, 0.0f );
						if ( !IsNetworkPlayer( pPlayer ) )
						{
							//count only hostages killed by local players
							App_IncreaseGamestat( K_MEMID_GAMESTATS_HOSTAGES_KILLED );
						}
					}
					//--- level stats ---
					if ( !bSpawnedDead )
					{
						m_arrStats[ K_LVL_STATS_HOSTAGES_KILLED ]++;
					}
				}
			}
			*/
			//make sure we release it on the next frame
			if ( dcmd == K_LVL_ACT_DEATHCMD_DEALLOCATE )
			{
				actor.Kill();
			}
		}
		break;
	}

	return true;
}
