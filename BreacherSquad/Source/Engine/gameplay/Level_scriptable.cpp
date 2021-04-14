#pragma once
#include "dxstdafx.h"

//porneste scriptul, salveaza UID-urile, face tot
void CLevel::StartScript(WCHAR* scriptName, IActiveInterface* active)
{
	//daca ruleaza deja un script nu mai lanseaza altul
	if (active->nRunningScriptUID > 0)
		return;
	//start script now
	active->nRunningScriptUID = UTGetScriptManager().StartScript(scriptName, active->GetUID(), &active->varAIparams);
}

void CLevel::StartScript(UINT32 scriptNameHash, IActiveInterface* active)
{
	//daca ruleaza deja un script nu mai lanseaza altul
	if (active->nRunningScriptUID > 0)
		return;
	//start script now
	active->nRunningScriptUID = UTGetScriptManager().StartScript(scriptNameHash, active->GetUID(), &active->varAIparams);
}

IActiveInterface* CLevel::ScriptGetActiveInterfaceByTargetParam(CVariantComplex* vcTarget, UINT32 executorUID)
{
	//AI TARGET
	IActiveInterface* target = null;
	if (vcTarget)
	{
		if (vcTarget->m_type == CVariantComplex::K_ARGTYPE_STRING)
		{
			if (vcTarget->m_strArg.getHash() == FastHash("self"))
			{
				target = GetIActiveInterfacePtr_byUID(executorUID);
			}
			else if (vcTarget->m_strArg.getHash() == FastHash("target"))
			{
				target = GetIActiveInterfacePtr_byUID(executorUID);
				//se duce pe targetul personal
				if ((target != null) && (target->pTarget != null))
				{
					target = target->pTarget;
				}
				else
				{
					LOG(L"ScriptGetActiveInterfaceByTargetParam - target is null");
				}
			}
			else if (vcTarget->m_strArg.getHash() == FastHash("targets_target"))
			{
				target = GetIActiveInterfacePtr_byUID(executorUID);
				//se duce pe targetul targetului
				if (target->pTarget != null)
				{
					target = target->pTarget->pTarget;
				}
				else
				{
					LOG(L"ScriptGetActiveInterfaceByTargetParam - target's target couldn't be found!\n");
					target = null;
				}
			}
			else if (vcTarget->m_strArg.getHash() == FastHash("toucher"))
			{
				//get toucher
				IActiveInterface* executor = GetIActiveInterfacePtr_byUID(executorUID);
				UINT32 toucherUID = executor->GetToucherUID();
				target = GetIActiveInterfacePtr_byUID(toucherUID);
			}
		}
		else if (vcTarget->m_type == CVariantComplex::K_ARGTYPE_INT32)
		{
			target = GetIActiveInterfacePtr(vcTarget->m_asINT32);
		}
		else //daca nu e setat inseamna ca e SELF
		{
			if (vcTarget->m_type == CVariantComplex::K_ARGTYPE_NONE)
			{
				target = GetIActiveInterfacePtr_byUID(executorUID);
			}
			else
			{
				LOG(L"ScriptGetActiveInterfaceByTargetParam - invalid target param value!\n");
				return null;
			}
		}
	}
	else
	{
		target = GetIActiveInterfacePtr_byUID(executorUID);
	}

	return target;
}


///--- SCRIPT CALLBACKS ---
bool CLevel::OnScriptFinished(UINT32 executorUID, UINT32 scriptUID, CVariantCollection * pArrScriptVars)
{
	IActiveInterface* active = GetIActiveInterfacePtr_byUID(executorUID);
	if (active != null)
	{
		//2. daca are target si script vars nRunTargetScript este diferit de 0 face touch la target
		if ((active->pTarget != null) && (pArrScriptVars->GetVariantByName(L"nRunTargetScript")->m_asINT32 != 0))
		{
			active->pTarget->Touch(active->GetUID(), 0.0f);
		}
		//reset touching flag
		active->bTouching = false;
		active->nTouchingUID = 0;
		//reset running script UID
		active->nRunningScriptUID = 0;
		return true;
	}

	return false;
}

///--- SCRIPT PROCESSOR ---
bool CLevel::ProcessScriptInstruction(CScriptInstruction *instr, UINT32 executorUID, UINT32 scriptUID)
{
	//get script instruction
	eLVLScriptInstruction eInstruction = (eLVLScriptInstruction)GetListIndexByNameHash(instr->m_instruction.textHash, eLVLScriptInstructionNames, instr_COUNT);
	if (eInstruction == -1)
	{
		return false;
	}

	//execute it if it belongs to us
	switch (eInstruction)
	{
		case instr_LEVEL_ENABLE_LIGHTNING:
		{
			m_fThunderTimer = 5.0f;
			return true;
		}
		break;
		case instr_LEVEL_GIVE_STRATEGIC_POINTS:
		{
			float fPts = instr->m_arrArgs.GetVariantByName(L"fPoints")->m_asFloat;
			if (fPts < 0.0f)
				fPts = 0.0f;

			CVariantComplex* vcTarget = instr->GetArgument(L"target");
			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcTarget, executorUID);
			if (vcTarget == null)
			{
				GiveStrategicPoints(fPts);
			}
			else
			{
				GiveStrategicPoints(fPts, &Vec2(target->bbox.vCenter.x, target->bbox.vMin.y));
			}

			return true;
		}
		break;
		case instr_LEVEL_NOTIFY_ENGINE:
		{
			//trece prin toti params si face verificarile de notificari
			for (int ii = 0; ii < instr->m_arrArgs.GetVariantCount(); ii++)
			{
				if (instr->m_arrArgs[ii]->m_name.getHash() == GET_FAST_HASH("nBombDefused"))
				{
					App_IncreaseGamestat(K_MEMID_GAMESTATS_BOMBS_DISARMED);
					//ACHIEVEMENTS: In the nick of time - bomb defusal
					if (m_interfaceIGM.GetBombTimer() <= 3.0f)
					{
						UTGetAchievementManager().UnlockAchievement(ACH_NICK_OF_TIME);
					}

					//clear bomb timer
					m_interfaceIGM.SetBombTimer(-1.0f);
					//erase level bombs flag
					m_arrStats[K_LVL_STATS_LEVEL_HAS_BOMBS] = 0;
					m_arrStats[K_LVL_STATS_BOMBS_DISARMED]++;
					//message bomb defused
					g_particlesMgr.AddStringDummy(K_PDUMMY_STRING_WIDEBAR, Vec2(0.0f, -50.0f), STR_BOMB_DEFUSED, FONTIDX_12_WOW, 1.0f, 3.0f, K_COLOR_SELECTED_TEXT);
					//sound verse - bomb defused
					IActiveInterface* target = GetIActiveInterfacePtr_byUID(executorUID);
					if (target)
					{
						CActor* act = GetClosestPlayer(target->pos);
						if (act)
						{
							//PlayActorSoundVerse(act, K_LVL_ACT_VERSE_BOMB_DEFUSED);
							//give points
							GiveStrategicPoints(2.0f, &Vec2(act->bbox.vCenter.x, act->bbox.vMin.y));
						}
					}
					else
					{
						//give points
						GiveStrategicPoints(2.0f);
					}

				}
				else if (instr->m_arrArgs[ii]->m_name.getHash() == GET_FAST_HASH("nSecretItem"))
				{
					int idx = instr->m_arrArgs[ii]->m_asINT32;
					if ((idx < 1) || (idx >= 32))
					{
						ErrorBox(K_ERR_WARNING, L"SCRIPT::LEVEL_NOTIFY_ENGINE nSecretItem index out of range!");
						break;
					}

					g_userData[K_MEMID_SECRET_ITEMS_FLAGS] |= (1 << (idx - 1));
					g_userData[K_MEMID_SECRET_ITEMS_FLAGS] &= 0xfffff; //masking 20 bits for 20 doughnuts
					UINT32 unBits = g_userData[K_MEMID_SECRET_ITEMS_FLAGS] & 0xfffff;
					int nCollectedCnt = UTMath::CountBits(unBits);
					//save collected
					UTGetAchievementManager().SetStat(EGameStats::N_STAT_SECRET_ITEMS, (float)nCollectedCnt);
#ifndef K_AUTO_ACHIEVE_FROM_STATS
					if (nCollectedCnt >= 20)
						UTGetAchievementManager().UnlockAchievement(ACH_COMPLETIONIST);
#endif
					UTGetAchievementManager().UnlockAchievement(ACH_LOOKS_INTERESTING);
					//set string for hint
					UTLang().ReplaceTokenInt(STR_SECRETS_COLLECTED_VAL, STR_SECRETS_COLLECTED_NN, 1, nCollectedCnt);
					UTLang().ReplaceTokenInt(STR_SECRETS_COLLECTED_VAL, STR_SECRETS_COLLECTED_VAL, 2, 20);
				}
				else if (instr->m_arrArgs[ii]->m_name.getHash() == GET_FAST_HASH("nCopSaved"))
				{
					App_IncreaseGamestat(K_MEMID_GAMESTATS_POLICE_SAVED);
				}
				else if (instr->m_arrArgs[ii]->m_name.getHash() == GET_FAST_HASH("nZombieSpawnerDisabled"))
				{
					m_arrStats[K_LVL_STATS_ZOMBIE_PORTALS_DESTROYED]++;
				}
				else if (instr->m_arrArgs[ii]->m_name.getHash() == GET_FAST_HASH("nArrestedTargets"))
				{
					//let level know we arrested a target
					m_arrStats[K_LVL_STATS_LEVEL_ARREST_TARGETS_ARRESTED]++;

					App_IncreaseGamestat(K_MEMID_GAMESTATS_ARREST_TARGETS_ARRESTED);
					//success message
					g_particlesMgr.AddStringDummy(K_PDUMMY_STRING_WIDEBAR, Vec2(0.0f, -50.0f), STR_TARGET_ARRESTED, FONTIDX_12_WOW, 1.0f, 3.0f, K_COLOR_SELECTED_TEXT);
				}
				else if (instr->m_arrArgs[ii]->m_name.getHash() == GET_FAST_HASH("nArrestedCivilians"))
				{
					m_arrStats[K_LVL_STATS_CIVILIANS_ARRESTED]++;
				}
				//door closed behind player in vertical infinite mode
				else if (instr->m_arrArgs[ii]->m_name.getHash() == GET_FAST_HASH("nVInfiniteDoorUsed"))
				{
					m_arrStats[K_LVL_STATS_LEVEL_VINFINITE_FLOOR]++;
					//show level number
					UTLang().ReplaceTokenInt(STR_FLOOR_X_VALUE, STR_FLOOR_X, 1, m_arrStats[K_LVL_STATS_LEVEL_VINFINITE_FLOOR]);

					g_particlesMgr.AddStringDummy(K_PDUMMY_STRING_LETTERWAVER, Vec2(0.0f, -50.0f), STR_FLOOR_X_VALUE, FONTIDX_12_WOW, 1.0f, 2.0f, K_COLOR_SELECTED_TEXT);
				}

			}

			return true;
		}
		break;
		case instr_IACTIVE_SET_AI:
		{
			CVariantComplex* vcAIname = instr->GetArgument(L"aiName");
			CVariantComplex* vcTarget = instr->GetArgument(L"target");

			EAIstate aistate = K_AI_STATE_UNDEFINED;
			if (vcAIname != null)
			{
				//AI STATE
				EAIstate aistate = (EAIstate)GetListIndexByNameHash(vcAIname->m_strArg.getHash(), EAIstate_names, K_AI_STATES_CNT);
				//it is ok if aistate becomes UNDEFINED because we use this instruction to erase the AI too
			}
			//AI TARGET
			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcTarget, executorUID);
			if (target == null)
			{
				LOG(L"SCRIPT::IACTIVE_SET_AI - invalid target param!\n");
				return true;
			}
			//AI PARAMS
			CVariantCollection varcol;
			//toti parametrii instructiunii, in afara de target si aiName se duc direct in varAIparams
			for (int ii = 0; ii < instr->m_arrArgs.GetVariantCount(); ii++)
			{
				if ((instr->m_arrArgs[ii]->m_name.getHash() != FastHash("target")) && (instr->m_arrArgs[ii]->m_name.getHash() != FastHash("aiName")))
				{
					varcol.AddVariant(instr->m_arrArgs[ii]);
				}
			}
			//set state too
			SetAI(target, aistate, &varcol);

			return true;
		}
		break;
		case instr_IACTIVE_SET_AI_PARAMS:
		{
			CVariantComplex* vcTarget = instr->GetArgument(L"target");
			//AI TARGET
			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcTarget, executorUID);
			if (vcTarget == null)
			{
				LOG(L"SCRIPT::IACTIVE_SET_AI_PARAMS - target is null! Probably wrong ID\n");
				return true;
			}
			//AI PARAMS
			CVariantCollection varcol;
			//toti parametrii instructiunii, in afara de target si aiName se duc direct in varAIparams
			for (int ii = 0; ii < instr->m_arrArgs.GetVariantCount(); ii++)
			{
				if (instr->m_arrArgs[ii]->m_name.getHash() != FastHash("target"))
				{
					varcol.AddVariant(instr->m_arrArgs[ii]);
				}
			}
			//set state too
			SetAIparams(target, &varcol);

			return true;
		}
		break;
		case instr_IACTIVE_RAIL_CHANGEDIR:
		{
			CVariantComplex* vcTarget = instr->GetArgument(L"target");
			//AI TARGET
			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcTarget, executorUID);
			if (target == null)
			{
				LOG(L"SCRIPT::IACTIVE_RAIL_CHANGEDIR - target not found!\n");
				return true;
			}

			CVariantComplex* stepdir = target->varAIparams.GetVariantByName(L"n_dir");
			float dir = stepdir->m_asINT32;

			if (dir != 0) //daca e in mers schimba direct
				dir *= -1;
			else //daca e la capat, vad la care capat este
			{
				if (target->AItimer1 <= 0.0f) //e la inceput
					dir = 1;
				else
					dir = -1;
			}

			//setam la loc
			target->varAIparams.SetNamedVarINT32(L"n_dir", dir);
			// aici suprascria parametrul default, nu era bine, asa ca am pus linia de mai sus
			//stepdir->Set_INT32(stepdir->m_name.text, dir);

			return true;
		}
		break;
		case instr_LEVEL_CAMERA_RESTORE_LAST_TARGET:
		{
			CVariantComplex* vcTeleport = instr->GetArgument(L"teleport");
			bool teleport = false;
			if (vcTeleport)
			{
				if (vcTeleport->asInt32() != 0)
					teleport = true;
			}

			m_camTargetActive = m_camTargetOld;
			if (teleport)
			{
				if (m_camTargetActive == null)
					m_camLevel.SetCamPos(&m_vCamPosDefault, 1.0f, true);
				else
					m_camLevel.SetCamPos(&m_camTargetActive->pos, 1.0f, true);
			}

			return true;
		}
		break;
		case instr_LEVEL_SET_TIME_MULTIPLIER:
		{
			CVariantComplex* vcValue = instr->GetArgument(L"fValue");
			CVariantComplex* vcDuration = instr->GetArgument(L"fDuration");
			float fDuration = 5.0f, fMultiplier = 1.0f;
			if ((vcValue) && (vcValue->m_type == CVariantComplex::K_ARGTYPE_FLOAT))
			{
				fMultiplier = vcValue->m_asFloat;
			}
			if ((vcDuration) && (vcDuration->m_type == CVariantComplex::K_ARGTYPE_FLOAT))
			{
				fDuration = vcDuration->m_asFloat;
			}

			SetTimeMultiplier(fMultiplier, fDuration);
			return true;
		}
		break;
		case instr_IACTIVE_CAMERA_SET_TARGET:
		{
			CVariantComplex* vcTarget = instr->GetArgument(L"target");
			CVariantComplex* vcTeleport = instr->GetArgument(L"teleport");

			bool teleport = false;
			if (vcTeleport)
			{
				if (vcTeleport->asInt32() != 0)
					teleport = true;
			}

			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcTarget, executorUID);
			if (target == null)
			{
				LOG(L"SCRIPT::IACTIVE_SET_CAMERA_TARGET - target is null! Probably wrong ID\n");
				return true;
			}

			m_camTargetOld = m_camTargetActive;
			m_camTargetActive = target;

			if (teleport)
			{
				if (m_camTargetActive == null)
					m_camLevel.SetCamPos(&m_vCamPosDefault, 1.0f, true);
				else
					m_camLevel.SetCamPos(&m_camTargetActive->pos, 1.0f, true);
			}

			return true;
		}
		break;
		case instr_ACTIVE_AMMOBOX_GIVE_AMMO:
		{
			/*
			CProp* active = GetActiveByUID(executorUID);
			if ((active == null) || (active->AIstate != K_AI_STATE_ACTIVE_AMMO_BOX))
			{
				LOG(L"SCRIPT::ACTIVE_AMMOBOX_GIVE_AMMO - executor doesn't have AMMO_BOX AI\n");
				return true;
			}
			//check ammo left
			int nAmmoLeft = active->varAIparams.GetVariantByName(L"n_ammoLeft")->m_asINT32;

			CActor* toucheractor = GetActorByUID(active->GetToucherUID());
			if ((toucheractor == null) || (toucheractor->actTemplate.actorClass != K_LVL_ACT_CLASS_PLAYER))
			{
				LOG(L"SCRIPT::ACTIVE_AMMOBOX_GIVE_AMMO - Could not find Actor Toucher UID or toucher not a player!\n");
				return true;
			}

			if (nAmmoLeft > 0)
			{
				if ((toucheractor->weapons[K_LVL_ACT_WEAPON_GEAR].status != K_LVL_WPN_STATUS_UNKNOWN) && //we have to have GEAR
					(toucheractor->weapons[K_LVL_ACT_WEAPON_GEAR].ammoLeft < toucheractor->weapons[K_LVL_ACT_WEAPON_GEAR].WeaponTemplate.nClipSize) &&
					(toucheractor->weapons[K_LVL_ACT_WEAPON_GEAR].WeaponTemplate.nClipSize > 0) &&
					(toucheractor->weapons[K_LVL_ACT_WEAPON_GEAR].WeaponTemplate.bulletTemplate.nType != K_LVL_BULLET_DULL))
				{
					//					SND_PLAY_POSITIONAL(SNDIDX_RELOAD_EMERGENCY, active->pos);
					g_particlesMgr.GenerateHealEffect(toucheractor->pos, 0xff5555ff, K_PART_LAYER_RT_FRONT_NRM);

					g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_INTERFACE_ELEMENTS, false, 0, &toucheractor->GetPosHeart(), NULL, &Vec2(0.0f, -10.0f), 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.4f, 0xffffffff, K_PART_LAYER_FRONT);

					toucheractor->weapons[K_LVL_ACT_WEAPON_GEAR].ammoLeft++;
					nAmmoLeft--;
					active->varAIparams.SetNamedVarINT32(L"n_ammoLeft", nAmmoLeft);

					//					SND_PLAY_POSITIONAL(SNDIDX_PLAYER_REPLENISH_AMMO, toucheractor->posHeart);

					if (nAmmoLeft <= 0)
					{
						active->bCanInteract = false;
						active->bStandsOut = false;
					}
				}
				else
				{
					//add notification if we actually have gear
					if (toucheractor->weapons[K_LVL_ACT_WEAPON_GEAR].status != K_LVL_WPN_STATUS_UNKNOWN)
					{
						m_interfaceTextBubble.ShowLevelHint(&m_camLevel, STR_FULL_AMMO, FONTIDX_6_NS1, Vec2(active->bbox_exported.vCenter.x, active->bbox_exported.vMin.y), 4.0f);
					}
					else
					{
						SND_PLAY_POSITIONAL(SNDIDX_DENIED, active->pos);
					}
				}
			}
			*/
			return true;
		}
		break;
		case instr_ACTIVE_HEALTHBOX_GIVE_HEALTH:
		{
			/*
			CProp* active = GetActiveByUID(executorUID);
			if ((active == null) || (active->AIstate != K_AI_STATE_ACTIVE_HEALTH_BOX))
			{
				LOG(L"SCRIPT::ACTIVE_HEALTHBOX_GIVE_HEALTH- executor doesn't have HEALTH_BOX AI\n");
				return true;
			}
			//check ammo left
			int nHealthLeft = active->varAIparams.GetVariantByName(L"n_healthLeft")->m_asINT32;

			CActor* toucheractor = GetActorByUID(active->GetToucherUID());
			if ((toucheractor == null) || (toucheractor->actTemplate.actorClass != K_LVL_ACT_CLASS_PLAYER))
			{
				LOG(L"SCRIPT::ACTIVE_HEALTHBOX_GIVE_HEALTH - Could not find Actor Toucher UID or toucher not a player!\n");
				return true;
			}

			if (nHealthLeft > 0)
			{
				if (toucheractor->fLife < toucheractor->actTemplate.fLife)
				{
					g_particlesMgr.GenerateHealEffect(toucheractor->pos, 0xff55ff55, K_PART_LAYER_RT_FRONT_NRM);

					toucheractor->fLife = toucheractor->actTemplate.fLife;
					nHealthLeft--;
					active->varAIparams.SetNamedVarINT32(L"n_healthLeft", nHealthLeft);

					if (nHealthLeft <= 0)
					{
						active->bCanInteract = false;
						active->bStandsOut = false;
					}
					//increase health boxes
					m_arrStats[K_LVL_STATS_LEVEL_HEALTH_BOXES_USED]++;
					//					SND_PLAY_POSITIONAL(SNDIDX_PLAYER_REPLENISH_HP, toucheractor->posHeart);
				}
				else
				{
					//add notification
					m_interfaceTextBubble.ShowLevelHint(&m_camLevel, STR_FULL_HEALTH, FONTIDX_6_NS1, Vec2(active->bbox_exported.vCenter.x, active->bbox_exported.vMin.y), 4.0f);
				}
			}
			*/
			return true;
		}
		break;
		case instr_IACTIVE_SET_CAN_INTERACT:
		{
			CVariantComplex* vcTarget = instr->GetArgument(L"target");
			CVariantComplex* vcCanInteract = instr->GetArgument(L"bCanInteract");

			bool bInteract = false;
			if (vcCanInteract)
			{
				if (vcCanInteract->asInt32() != 0)
					bInteract = true;
			}

			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcTarget, executorUID);
			if (target == null)
			{
				LOG(L"SCRIPT::IACTIVE_SET_CAN_INTERACT - target is null! Probably wrong ID\n");
				return true;
			}

			target->bCanInteract = bInteract;

			return true;
		}
		break;
		case instr_IACTIVE_SET_TARGETPTR:
		{
			CVariantComplex* vcTarget = instr->GetArgument(L"target");
			CVariantComplex* vcTargetID = instr->GetArgument(L"targetID");

			int targetID = -1;
			if (vcTargetID)
			{
				targetID = vcTargetID->asInt32();
			}
			else
			{
				LOG(L"SCRIPT::IACTIVE_SET_TARGETPTR targetID set on -1\n");
			}

			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcTarget, executorUID);
			if (target == null)
			{
				LOG(L"SCRIPT::IACTIVE_SET_TARGETPTR - target is null! Probably wrong ID\n");
				return true;
			}
			///set new target now
			target->targetID_ini = targetID;
			target->pTarget = GetIActiveInterfacePtr(targetID);

			return true;
		}
		break;
		case instr_COLL_ENTER_HIDDEN_ROOM:
		{
			return true;
		}
		break;
		case instr_COLL_CHECK_HIDDEN_ROOM_CLEARED:
		{

			return true;
		}
		break;
		case instr_IACTIVE_GET_AI_PARAM:
		{
			CVariantComplex* vcTarget = instr->GetArgument(L"target");
			CVariantComplex* vcParamName = instr->GetArgument(L"paramName");
			CVariantComplex* vcLocalVar = instr->GetArgument(L"destLocalVarName");

			if ((vcParamName == null) || (vcLocalVar == null) || (vcParamName->m_type != CVariantComplex::K_ARGTYPE_STRING) || (vcLocalVar->m_type != CVariantComplex::K_ARGTYPE_STRING))
			{
				LOG(L"SCRIPT::IACTIVE_GET_AIPARAM - paramName or destLocalVarName not specified or not string!\n");
				return true;
			}

			//AI TARGET
			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcTarget, executorUID);
			//AI PARAM
			if (target != null)
			{
				CVariantComplex* AIparam = target->varAIparams.GetVariantByNameHash(vcParamName->m_strArg.getHash());
				if (AIparam == null)
				{
					//daca nu gaseste param AI seteaza 0
					CVariantComplex* narg = new CVariantComplex();
					narg->Set_INT32(vcLocalVar->m_strArg.text, 0);
					UTGetScriptManager().SetLocalVar(scriptUID, narg);
					LOG(L"SCRIPT::IACTIVE_GET_AI_PARAM - AIparam not found. Setting destVar on 0\n");
					return true;
				}
				//daca am gasit setam varabila locala
				CVariantComplex* narg = new CVariantComplex();
				WCHAR AIparamValue[MAX_PATH];
				AIparam->asString(AIparamValue, MAX_PATH);

				narg->Set_AUTO(vcLocalVar->m_strArg.text, AIparamValue);
				UTGetScriptManager().SetLocalVar(scriptUID, narg);
			}
			else
			{
				LOG(L"SCRIPT::IACTIVE_GET_AI_PARAM - target is null! Probably wrong ID\n");
			}

			return true;
		}
		break;
		case instr_IACTIVE_REMOVE_NOTIFICATION:
		{
			m_interfaceTextBubble.Hide();
			return true;
		}
		break;
		case instr_IACTIVE_ADD_NOTIFICATION_LOCKED_DOOR:
		{
			return true;
		}
		break;
		case instr_IACTIVE_ADD_NOTIFICATION:
		{
			CVariantComplex* vcTarget = instr->GetArgument(L"target");
			CVariantComplex* vcStringID = instr->GetArgument(L"sStringID");
			CVariantComplex* vcDuration = instr->GetArgument(L"fDuration");

			float fDuration = vcDuration->m_asFloat;
			//TARGET																	 
			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcTarget, executorUID);
			if (target == null)
			{
				LOG(L"SCRIPT::IACTIVE_ADD_NOTIFICATION - target is null! Probably wrong ID\n");
				return true;
			}
			int stridx = UTLang().GetStrIdx(vcStringID->m_strArg.textHash);
			if ((vcStringID->m_type == CVariantComplex::K_ARGTYPE_NONE) || (stridx < 0))
			{
				LOG(L"SCRIPT::IACTIVE_ADD_NOTIFICATION - couldn't find string:[%s]\n", vcStringID->m_strArg.text);
				return true;
			}
			//add notification
			m_interfaceTextBubble.ShowLevelHint(&m_camLevel, stridx, FONTIDX_6_NS1, Vec2(target->bbox_exported.vCenter.x, target->bbox_exported.vMin.y), fDuration);
			return true;
		}
		break;
		case instr_IACTIVE_GENERATE_EFFECT:
		{
			CVariantComplex* vcTarget = instr->GetArgument(L"target");
			CVariantComplex* vcTypeS = instr->GetArgument(L"sEffectType");

			//TARGET																	 
			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcTarget, executorUID);
			if (target == null)
			{
				LOG(L"SCRIPT::IACTIVE_GENERATE_EFFECT - target is null! Probably wrong ID\n");
				return true;
			}
			//add effect
			GenerateEffect(vcTypeS->m_strArg, target->pos, 1.0f, 0xffffffff);
			return true;
		}
		break;
		case instr_IACTIVE_ADD_AI_EVENT:
		{
			CVariantComplex* vcTarget = instr->GetArgument(L"target");
			CVariantComplex* vcEvtType = instr->GetArgument(L"sEventType");
			CVariantComplex* vcEvtRange = instr->GetArgument(L"fRange");
			CVariantComplex* vcEvtDuration = instr->GetArgument(L"fDuration");
			CVariantComplex* vcEvtClass = instr->GetArgument(L"sEventClass");

			//TARGET
			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcTarget, executorUID);
			if (target == null)
			{
				LOG(L"SCRIPT::IACTIVE_ADD_AI_EVENT - target is null! Probably wrong ID\n");
				return true;
			}
			//evt type
			if (vcEvtType->m_type != CVariantComplex::K_ARGTYPE_STRING)
			{
				LOG(L"SCRIPT::IACTIVE_ADD_AI_EVENT - event type param missing!\n");
				return true;
			}
			EAIEventType evttype = (EAIEventType)GetListIndexByNameHash(vcEvtType->m_strArg.textHash, EAIEventTypeNames, K_LVL_AI_EVENTS_CNT);
			if (evttype < 0)
			{
				LOG(L"SCRIPT::IACTIVE_ADD_AI_EVENT - event type [%s] not found!\n", vcEvtType->m_strArg.text);
				return true;
			}
			float fRange = 128.0f;
			float fDuration = 1.0f;
			if (vcEvtRange->m_type == CVariantComplex::K_ARGTYPE_FLOAT)
				fRange = vcEvtRange->m_asFloat;
			if (vcEvtDuration->m_type == CVariantComplex::K_ARGTYPE_FLOAT)
				fDuration = vcEvtDuration->m_asFloat;
			int evtClass = K_LVL_ACT_CLASS_PASSIVE;
			if (vcEvtClass->m_type == CVariantComplex::K_ARGTYPE_STRING)
			{
				int retEvtClass = GetListIndexByNameHash(vcEvtClass->m_strArg.textHash, EActorClassNames, K_LVL_ACT_CLASSES_COUNT);
				if (retEvtClass >= 0)
					evtClass = retEvtClass;
			}
			//add event
			AddAIEvent(evttype, target->GetUID(), evtClass, target->pos, fRange, fDuration);

			return true;
		}
		break;
		case instr_IACTIVE_SET_SCRIPT:
		{
			CVariantComplex* vcScriptname = instr->GetArgument(L"scriptName");
			CVariantComplex* vcTarget = instr->GetArgument(L"target");

			//TARGET
			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcTarget, executorUID);
			if (target == null)
			{
				LOG(L"SCRIPT::IACTIVE_SET_SCRIPT - target is null! Probably wrong ID\n");
				return true;
			}
			//stop script
			if (target->nRunningScriptUID != 0)
			{
				UTGetScriptManager().StopScript(target->nRunningScriptUID);
				LOG(L"SCRIPT::IACTIVE_SET_SCRIPT - stopped running script!\n");
			}
			//daca nu pun param script sterge scriptul
			if (vcScriptname == null)
			{
				target->script_hash.Reset();
			}
			else //set new script
			{
				target->script_hash = vcScriptname->m_strArg;
			}

			return true;
		}
		break;
		case instr_ACTOR_TOUCHER_TELEPORT:
		{
			CVariantComplex* vcWhere = instr->GetArgument(L"where");
			CVariantComplex* vcOffX = instr->GetArgument(L"offX");
			CVariantComplex* vcOffY = instr->GetArgument(L"offY");
			//destination offsets
			float fOffX = 0.0f;
			float fOffY = 0.0f;
			if (vcOffX)
			{
				fOffX = vcOffX->asFloat();
			}
			if (vcOffY)
			{
				fOffY = vcOffY->asFloat();
			}

			//destination
			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcWhere, executorUID);
			if (target == null)
			{
				LOG(L"SCRIPT::ACTOR_TOUCHER_TELEPORT - where: missing param!\n");
				return true;
			}

			//get toucher
			IActiveInterface* executor = GetIActiveInterfacePtr_byUID(executorUID);
			CActor* toucheractor = GetActorByUID(executor->GetToucherUID());
			if (toucheractor == null)
			{
				LOG(L"SCRIPT::ACTOR_TOUCHER_TELEPORT - Could not find Actor Toucher UID! Probabil este o activare auto intre teleportoare prin targetID\n");
				return true;
			}
			//Set final pos
			toucheractor->SetPos(Vec2(target->pos.x + fOffX, target->pos.y + fOffY));

			return true;
		}
		break;
		case instr_ACTOR_SWITCH_WEAPONS:
		{
			/*
			CVariantComplex* vcWho = instr->GetArgument(L"who");
			int nIdx_src = instr->GetArgument(L"nWpnIdx_src")->m_asINT32;
			int nIdx_dest = instr->GetArgument(L"nWpnIdx_dest")->m_asINT32;
			int nKeepAmmoFromSrc = instr->GetArgument(L"nKeepAmmoFromSrc")->m_asINT32;
			int nRefillAmmo = instr->GetArgument(L"nRefillAmmo")->m_asINT32;

			CLAMP(nIdx_src, 0, (int)K_LVL_ACT_WEAPONS_CNT);
			CLAMP(nIdx_dest, 0, (int)K_LVL_ACT_WEAPONS_CNT);

			if (nIdx_src == nIdx_dest)
			{
				LOG(L"SCRIPT::ACTOR_SWITCH_WEAPONS - missing nWpnIdx1 or nWpnIdx2 param or they are equal!\n");
				return true;
			}
			CActor* targetAct = static_cast<CActor*>(ScriptGetActiveInterfaceByTargetParam(vcWho, executorUID));
			if (targetAct == null)
			{
				LOG(L"SCRIPT::ACTOR_SWITCH_WEAPONS - who: missing param!\n");
				return true;
			}

			SWAP(targetAct->pSelectedWeapon[nIdx_src], targetAct->pSelectedWeapon[nIdx_dest]);
			//keep ammo if requested (after weapon swap so we're writing into src, copying from dest)
			if (nKeepAmmoFromSrc != 0)
			{
				targetAct->pSelectedWeapon[nIdx_src]->ammoLeft = targetAct->pSelectedWeapon[nIdx_dest]->ammoLeft;
			}
			//refill ammo?
			if (nRefillAmmo != 0)
			{
				targetAct->pSelectedWeapon[nIdx_src]->ammoLeft = targetAct->pSelectedWeapon[nIdx_src]->WeaponTemplate.nClipSize;
			}
			//set weapon  perks only if changed active weapon
			if ((nIdx_src == K_LVL_ACT_WEAPON_PRIMARY) || (nIdx_dest == K_LVL_ACT_WEAPON_PRIMARY))
			{
				SetActorWeaponPerks(targetAct, targetAct->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]);
			}
			*/
			return true;
		}
		break;
		case instr_ACTOR_SET_WEAPON:
		{
			/*
			CVariantComplex* vcWho = instr->GetArgument(L"who");
			int nIdx1 = instr->GetArgument(L"nWpnIdx")->m_asINT32;
			CStringHash shWpnTemplate = instr->GetArgument(L"sWpnTemplate")->m_strArg;

			CLAMP(nIdx1, 0, (int)K_LVL_ACT_WEAPONS_CNT);

			CActor* targetAct = static_cast<CActor*>(ScriptGetActiveInterfaceByTargetParam(vcWho, executorUID));
			if (targetAct == null)
			{
				LOG(L"SCRIPT::ACTOR_SET_WEAPON - who: missing param!\n");
				return true;
			}
			//do a simple "clear" if template not specified
			if (shWpnTemplate.IsEmpty())
			{
				targetAct->weapons[nIdx1].Init();
				return true;
			}

			//init weapon if valid
			Weapon_Init(&targetAct->weapons[nIdx1], shWpnTemplate.text, targetAct);
			//set weapon  perks only if changed active weapon
			if (targetAct->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY] == &targetAct->weapons[nIdx1])
			{
				SetActorWeaponPerks(targetAct, targetAct->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]);
			}
			*/
			return true;
		}
		break;
		case instr_ACTOR_EQUIP_WEAPONS:
		{
			/*
			CVariantComplex* vcWho = instr->GetArgument(L"who");

			CActor* targetAct = static_cast<CActor*>(ScriptGetActiveInterfaceByTargetParam(vcWho, executorUID));
			if (targetAct == null)
			{
				LOG(L"SCRIPT::ACTOR_EQUIP_WEAPONS - who: missing param!\n");
				return true;
			}

			CVariantComplex* nPrimaryIdx = instr->GetArgument(L"nPrimaryIdx");
			if (nPrimaryIdx->m_type == CVariantComplex::K_ARGTYPE_INT32)
			{
				CWeapon* weapon_old = targetAct->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY];

				targetAct->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY] = &targetAct->weapons[nPrimaryIdx->m_asINT32];
				SetActorWeaponPerks(targetAct, targetAct->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]);

				CWeapon* weapon_new = targetAct->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY];
			}

			CVariantComplex* nSecondaryIdx = instr->GetArgument(L"nSecondaryIdx");
			if (nSecondaryIdx->m_type == CVariantComplex::K_ARGTYPE_INT32)
			{
				targetAct->pSelectedWeapon[K_LVL_ACT_WEAPON_SECONDARY] = &targetAct->weapons[nSecondaryIdx->m_asINT32];
			}

			CVariantComplex* nGearIdx = instr->GetArgument(L"nGearIdx");
			if (nGearIdx->m_type == CVariantComplex::K_ARGTYPE_INT32)
			{
				targetAct->pSelectedWeapon[K_LVL_ACT_WEAPON_GEAR] = &targetAct->weapons[nGearIdx->m_asINT32];
			}
			*/
			return true;
		}
		break;
		case instr_ACTOR_SHOOT_WEAPON:
		{
			/*
			CVariantComplex* vcWho = instr->GetArgument(L"who");

			CActor* targetAct = static_cast<CActor*>(ScriptGetActiveInterfaceByTargetParam(vcWho, executorUID));
			if (targetAct == null)
			{
				LOG(L"SCRIPT::ACTOR_SHOOT_WEAPON - who: missing param!\n");
				return true;
			}

			CVariantComplex* nWeaponIdx = instr->GetArgument(L"nWeaponIdx");
			Vec3 vShootDir = Vec2ToVec3XY0(targetAct->m_AIcommands.vAimVec);
			if (nWeaponIdx->m_type == CVariantComplex::K_ARGTYPE_INT32)
			{
				ShootWeapon(targetAct->pSelectedWeapon[nWeaponIdx->m_asINT32], vShootDir);
			}
			else
			{
				ShootWeapon(targetAct->pCurrentWeapon, vShootDir);
			}
			*/
			return true;
		}
		break;
		case instr_ACTOR_JAM_WEAPON:
		{
			/*
			CVariantComplex* vcWho = instr->GetArgument(L"who");
			float fJamTim = instr->GetArgument(L"fJamTimer")->m_asFloat;
			int nCanReset = instr->GetArgument(L"nCanResetJam")->m_asINT32;

			CActor* targetAct = static_cast<CActor*>(ScriptGetActiveInterfaceByTargetParam(vcWho, executorUID));
			if (targetAct == null)
			{
				LOG(L"SCRIPT::ACTOR_JAM_WEAPON - who: missing param!\n");
				return true;
			}

			CVariantComplex* nWeaponIdx = instr->GetArgument(L"nWeaponIdx");
			if (nWeaponIdx->m_type == CVariantComplex::K_ARGTYPE_INT32)
			{
				targetAct->pSelectedWeapon[nWeaponIdx->m_asINT32]->fJammedTimer = fJamTim;
				targetAct->pSelectedWeapon[nWeaponIdx->m_asINT32]->nCanResetJamCount = nCanReset;
			}
			else
			{
				targetAct->pCurrentWeapon->fJammedTimer = fJamTim;
				targetAct->pCurrentWeapon->nCanResetJamCount = nCanReset;
			}
			*/
			return true;
		}
		break;

		case instr_ACTOR_SET_DOT:
		{
			CVariantComplex* vcWho = instr->GetArgument(L"who");
			CVariantComplex* vcDoT = instr->GetArgument(L"sDoT");
			float fDuration = instr->GetArgument(L"fDuration")->m_asFloat;

			CActor* targetAct = static_cast<CActor*>(ScriptGetActiveInterfaceByTargetParam(vcWho, executorUID));
			if (targetAct == null)
			{
				LOG(L"SCRIPT::ACTOR_SET_DOT - who: missing param or target not an Actor!\n");
				return true;
			}
			if (fDuration <= 0.0f)
				fDuration = 1.0f;
			if (vcDoT->m_type != CVariantComplex::K_ARGTYPE_STRING)
			{
				LOG(L"SCRIPT::ACTOR_SET_DOT - sDoT: missing param or not string!\n");
				return true;
			}

			CDamageOverTime::EDoTType lDoTType = (CDamageOverTime::EDoTType)GetListIndexByNameHash(vcDoT->m_strArg.getHash(), EDoTTypeNames, CDamageOverTime::K_LVL_DoT_COUNT);

			SetActorDoT(targetAct, lDoTType, fDuration, 0.0f, K_LVL_ACT_CLASS_NOT_SET, K_LVL_ACT_CLASS_NOT_SET, 0);

			//customize effects
			switch (lDoTType)
			{
				case CDamageOverTime::K_LVL_DoT_INVINCIBLE:
				{
					g_particlesMgr.GenerateTeleportEffect(targetAct->pos, 0xfffdb727, K_PART_LAYER_RT_FRONT_NRM_LIGHT);
				}
				break;
			}

			return true;
		}
		break;

		case instr_ACTOR_HIT:
		{
			CVariantComplex* vcWho = instr->GetArgument(L"who");
			float fDamage = instr->GetArgument(L"fDamage")->m_asFloat;

			CActor* targetAct = static_cast<CActor*>(ScriptGetActiveInterfaceByTargetParam(vcWho, executorUID));
			if (targetAct == null)
			{
				LOG(L"SCRIPT::ACTOR_HIT - who: missing param or target not an Actor!\n");
				return true;
			}
			//#TODO: add more params to the script
			HitActor(targetAct, fDamage, 0, K_LVL_ACT_CLASS_TRAP, NULL, K_LVL_BULLET_FLAG_IGNORE_ARMOR, 5, 5.0f);

			return true;
		}
		break;
		case instr_ACTOR_PERK_MODIFIER:
		{
			CVariantComplex* vcWho = instr->GetArgument(L"who");
			CVariantComplex* vcPerk = instr->GetArgument(L"sPerkName");
			CVariantComplex* vcQty = instr->GetArgument(L"fQtyAdded");
			//who
			CActor* targetAct = static_cast<CActor*>(ScriptGetActiveInterfaceByTargetParam(vcWho, executorUID));
			if (targetAct == null)
			{
				LOG(L"SCRIPT::ACTOR_PERK_MODIFIER - who: missing param!\n");
				return true;
			}
			if (vcQty->m_type == CVariantComplex::K_ARGTYPE_NONE)
			{
				LOG(L"SCRIPT::ACTOR_PERK_MODIFIER - fQtyAdded param not specified!\n");
				return true;
			}
			if (vcPerk->m_type != CVariantComplex::K_ARGTYPE_STRING)
			{
				LOG(L"SCRIPT::ACTOR_PERK_MODIFIER - sPerkName missing or not a string!\n");
				return true;
			}
			/*
			if (vcPerk->m_strArg.IsEqual(L"GEAR_CAPACITY"))
			{
				int nQty = (int)floor(vcQty->m_asFloat);
				CLAMP(nQty, -5, 5);
				CWeapon* pGearWpn = targetAct->pSelectedWeapon[K_LVL_ACT_WEAPON_GEAR];
				if ((pGearWpn != null) && (pGearWpn->status != K_LVL_WPN_STATUS_UNKNOWN) && (pGearWpn->WeaponTemplate.nClipSize > 0) &&
					(pGearWpn->WeaponTemplate.bulletTemplate.nType != K_LVL_BULLET_DULL))
				{
					pGearWpn->WeaponTemplate.nClipSize += nQty;
					pGearWpn->ammoLeft += nQty;
				}
			}
			else if (vcPerk->m_strArg.IsEqual(L"SPEED_LOADER"))
			{
				int nQty = (int)vcQty->m_asFloat;
				CLAMP(nQty, 0, 100);
				if (targetAct->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->status != K_LVL_WPN_STATUS_UNKNOWN)
				{
					int nMaxAmmo = targetAct->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->WeaponTemplate.nClipSize + targetAct->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->WeaponTemplate.nBulletChamberSize;
					//do a quick reload
					inc_limit(targetAct->pSelectedWeapon[K_LVL_ACT_WEAPON_PRIMARY]->ammoLeft, nQty, nMaxAmmo);
				}
			}
			else
			{
				LOG(L"SCRIPT::ACTOR_PERK_MODIFIER - unknown perk [%s] !\n", vcPerk->m_strArg.text);
			}
			*/
			return true;
		}
		break;
		case instr_ACTOR_SET_COMMAND:
		{
			CVariantComplex* vcWho = instr->GetArgument(L"who");
			CVariantComplex* vcCommand = instr->GetArgument(L"sCommand");
			CVariantComplex* vcModifier = instr->GetArgument(L"nCommandModifier");
			//who
			CActor* targetAct = static_cast<CActor*>(ScriptGetActiveInterfaceByTargetParam(vcWho, executorUID));
			if (targetAct == null)
			{
				LOG(L"SCRIPT::ACTOR_SET_COMMAND - who: missing param!\n");
				return true;
			}

			if (vcCommand->m_type != CVariantComplex::K_ARGTYPE_STRING)
			{
				LOG(L"SCRIPT::ACTOR_SET_COMMAND - vcCommand missing or not a string!\n");
				return true;
			}

			//read command modifier
			int nModifier = 0;
			if (vcModifier->m_type == CVariantComplex::K_ARGTYPE_INT32)
			{
				nModifier = vcModifier->m_asINT32;
			}


			EControllerCommand cmd = (EControllerCommand)GetListIndexByName(vcCommand->m_strArg.text, EControllerCommandNames, EControllerCommand::K_CM_COMMANDS_COUNT);
			switch (cmd)
			{
				case K_CM_COMMAND_MELEE:
				{
					targetAct->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_MELEE;
					//breach door if ordered so
					if (nModifier != 0)
					{
						//#HACK: only used by doors when breaching them so we make sure the actor faces the door
						/*
						IActiveInterface* pExecutor = GetIActiveInterfacePtr_byUID(executorUID);
						if (pExecutor != null)
							targetAct->m_AIcommands.nLookDirX = SIGN(pExecutor->bbox.vCenter.x - targetAct->bbox.vCenter.x);
							*/
						targetAct->m_AIcommands.eAttackCommand = K_LVL_ACT_ATTACK_BREACH;
					}
				}
				break;
				default:
				{
					LOG(L"SCRIPT::ACTOR_SET_COMMAND - vcCommand invalid! Try COMMAND_LEFT, COMMAND_MELEE, etc\n");
					return true;
				}
				break;
			}

			return true;
		}
		break;
		case instr_IACTIVE_SAVE_TOUCHER_UID:
		{
			CVariantComplex* vcVarname = instr->GetArgument(L"sAIvarName");
			if (vcVarname == null)
			{
				LOG(L"SCRIPT::IACTIVE_SAVE_TOUCHER_UID - missing sAIvarName param!\n");
				return true;
			}

			//get toucher
			IActiveInterface* executor = GetIActiveInterfacePtr_byUID(executorUID);
			UINT32 toucherUID = executor->GetToucherUID();
			if (toucherUID == 0)
			{
				LOG(L"SCRIPT::IACTIVE_SAVE_TOUCHER_UID - toucher UID is 0\n");
			}
			//Set final pos
			executor->varAIparams.SetNamedVarUINT32(vcVarname->m_strArg.text, toucherUID);

			return true;
		}
		break;
		case instr_ACTOR_SET_TEMPLATE:
		{
			CVariantComplex* vcWhere = instr->GetArgument(L"who");
			CVariantComplex* vcTemplate = instr->GetArgument(L"sTemplateName");
			//destination
			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcWhere, executorUID);
			CActor* actor = dynamic_cast<CActor*>(target);
			if (actor == null)
			{
				LOG(L"SCRIPT::ACTOR_SET_TEMPLATE - where: missing param or target not an actor!\n");
				return true;
			}

			if (vcTemplate->m_type != CVariantComplex::K_ARGTYPE_STRING)
			{
				LOG(L"SCRIPT::ACTOR_SET_TEMPLATE - sTemplateName: missing param or not a string!\n");
				return true;
			}

			CActorTemplate* pTemplate = Actor_GetTemplate(vcTemplate->m_strArg.textHash);
			if (pTemplate == null)
			{
				LOG(L"SCRIPT::ACTOR_SET_TEMPLATE - sTemplateName: template %s not found!\n", vcTemplate->m_strArg.text);
				return true;
			}

			//InitActor(actor, pTemplate, actor->pos);

			return true;
		}
		break;
		case instr_ACTOR_SET_TEMPLATE_RANDOM:
		{
			CVariantComplex* vcWhere = instr->GetArgument(L"who");
			//destination
			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcWhere, executorUID);
			CActor* actor = dynamic_cast<CActor*>(target);
			if (actor == null)
			{
				LOG(L"SCRIPT::ACTOR_SET_TEMPLATE_RANDOM - who param missing or target not an actor!\n");
				return true;
			}

			CVariantComplex* vcprob1 = instr->GetArgument(L"fProbability1");
			CVariantComplex* vcprob2 = instr->GetArgument(L"fProbability2");

			if ((vcprob1->m_type != CVariantComplex::K_ARGTYPE_FLOAT) || (vcprob2->m_type != CVariantComplex::K_ARGTYPE_FLOAT))
			{
				LOG(L"SCRIPT::ACTOR_SET_TEMPLATE_RANDOM - missing fProbability1/fProbability2 or not float on ID:%d\n", actor->ID);
				return true;
			}

			float f1 = vcprob1->m_asFloat;
			float f2 = vcprob2->m_asFloat;
			float fval = m_rand.RandFloat(f1 + f2);

			CVariantComplex* vcTemplate = null;
			if (fval <= f1)
				vcTemplate = instr->GetArgument(L"sTemplateName1");
			else
				vcTemplate = instr->GetArgument(L"sTemplateName2");

			CActorTemplate* pTemplate = Actor_GetTemplate(vcTemplate->m_strArg.textHash);
			if (pTemplate == null)
			{
				LOG(L"SCRIPT::ACTOR_SET_TEMPLATE_RANDOM - sTemplateName: template [%s] not found for ID:%d!\n", vcTemplate->m_strArg.text, actor->ID);
				return true;
			}

			//InitActor(actor, pTemplate, actor->pos);
			return true;
		}
		break;

		case instr_ACTOR_SET_AI_STATE:
		{
			CVariantComplex* vcWhere = instr->GetArgument(L"who");
			CVariantComplex* vcAIstate = instr->GetArgument(L"AIstate");
			//destination
			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcWhere, executorUID);
			CActor* actor = dynamic_cast<CActor*>(target);
			if (actor == null)
			{
				LOG(L"SCRIPT::ACTOR_SET_AI_STATE - where: missing param or target not an actor!\n");
				return true;
			}

			//asa era pe modelul vechi de AI
			//int AIstate = GetAIStateByNameHash(vcAIstate->m_strArg.getHash());
			if ((vcAIstate == null) || (vcAIstate->m_type != CVariantComplex::K_ARGTYPE_STRING))
			{
				Actor_SetAIState(actor, actor->actTemplate.AItemplate->GetAIStateByName(actor->actTemplate.shAIState_ini));
			}
			else
			{
				Actor_SetAIState(actor, actor->actTemplate.AItemplate->GetAIStateByName(vcAIstate->m_strArg));
			}

			return true;
		}
		break;
		case instr_ACTOR_SPAWN:
		{
			CVariantComplex* vcWhere = instr->GetArgument(L"where");
			CVariantComplex* vcTemplate = instr->GetArgument(L"template");
			CVariantComplex* vcAIstate = instr->GetArgument(L"AIstate");
			CVariantComplex* vcDirection = instr->GetArgument(L"direction");

			//destination
			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcWhere, executorUID);
			if (target == null)
			{
				LOG(L"SCRIPT::ACTOR_SPAWN - where: missing param!\n");
				return true;
			}

			//get template and state
			if ((vcTemplate == null) || (vcTemplate->m_type != CVariantComplex::K_ARGTYPE_STRING))
			{
				LOG(L"SCRIPT::ACTOR_SPAWN - template name missing or wrong\n");
				return true;
			}

			CActorTemplate* ntempl = Actor_GetTemplate(vcTemplate->m_strArg.text);
			if (ntempl == null)
			{
				LOG(L"SCRIPT::ACTOR_SPAWN - template not found [%s] !\n", vcTemplate->m_strArg.text);
				return true;
			}

			CVariantComplex* vcOffX = instr->GetArgument(L"offX");
			CVariantComplex* vcOffY = instr->GetArgument(L"offY");
			//destination offsets
			float fOffX = 0.0f;
			float fOffY = 0.0f;
			if (vcOffX)
			{
				fOffX = vcOffX->asFloat();
			}
			if (vcOffY)
			{
				fOffY = vcOffY->asFloat();
			}
			Vec2 vSpawnPos = target->pos;
			vSpawnPos.x += fOffX;
			vSpawnPos.y += fOffY;

			CActor* nact = null;

			if ((vcAIstate == null) || (vcAIstate->m_type != CVariantComplex::K_ARGTYPE_STRING))
			{
				nact = SpawnActor(vSpawnPos, vcTemplate->m_strArg.text);
			}
			else
			{
				nact = SpawnActor(vSpawnPos, vcTemplate->m_strArg.text, &vcAIstate->m_strArg);
			}

			return true;
		}
		break;
		case instr_ACTIVE_INC_FRAME:
		{
			CVariantComplex* param1 = instr->GetArgument(L"step");
			CVariantComplex* param2 = instr->GetArgument(L"loop");
			CVariantComplex* vcTarget = instr->GetArgument(L"target");
			//AI TARGET
			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcTarget, executorUID);
			if (target == null)
			{
				LOG(L"SCRIPT::IACTIVE_INC_FRAME - target not found!\n");
				return true;
			}

			if (target->GetClassType() != K_LVL_IAI_TYPE_PROP)
			{
				LOG(L"SCRIPT::IACTIVE_INC_FRAME - target not an ACTIVE!\n");
				return true;
			}
			//get params
			int step = 1;
			if (param1) step = param1->m_asINT32;
			bool loop = true;
			if (param2) loop = param2->m_asBool;

			CProp* active = dynamic_cast<CProp*>(target);
			if (active == null)
			{
				LOG(L"SCRIPT::IACTIVE_INC_FRAME - Bad cast to CActive* !\n");
				return true;
			}

			active->sprite.frameIdx += step;
			if (loop)
			{
				int frcnt = m_sprProps.GetAFramesCnt(active->sprite.animIdx);
				if (active->sprite.frameIdx < 0)
					active->sprite.frameIdx += frcnt;
				else
					active->sprite.frameIdx %= frcnt;
			}
			else
			{
				CLAMP(active->sprite.frameIdx, 0, m_sprProps.GetAFramesCnt(active->sprite.animIdx));
			}
			//set new bbox
			RECTXYWH objbox = m_sprProps.GetAFrameBBox_real(active->sprite.animIdx, active->sprite.frameIdx);
			active->bbox_ini.Set(Vec2(objbox.x, objbox.y), Vec2(objbox.Right(), objbox.Bottom()));
			if (active->flipX)
			{
				active->bbox_ini.Move(Vec2(-2.0f * active->bbox_ini.vCenter.x, 0.0f));
			}
			active->bbox = active->bbox_ini;
			active->bbox.Move(active->pos);

			return true;
		}
		break;
		case instr_ACTIVE_SET_ANIM:
		{
			CVariantComplex* parAnim = instr->GetArgument(L"anim");
			CVariantComplex* parFrame = instr->GetArgument(L"frame");
			CVariantComplex* parAnimated = instr->m_arrArgs.GetVariantByName(L"animated");
			CVariantComplex* vcTarget = instr->GetArgument(L"target");
			//AI TARGET
			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcTarget, executorUID);
			if (target == null)
			{
				LOG(L"SCRIPT::IACTIVE_SET_ANIM - target not found!\n");
				return true;
			}

			if (target->GetClassType() != K_LVL_IAI_TYPE_PROP)
			{
				LOG(L"SCRIPT::IACTIVE_SET_ANIM - target not an ACTIVE!\n");
				return true;
			}

			CProp* active = dynamic_cast<CProp*>(target);
			if (active == null)
			{
				LOG(L"SCRIPT::IACTIVE_SET_ANIM - Bad cast to CActive!\n");
				return true;
			}
			//get params
			int anim = active->sprite.animIdx;
			if (parAnim)
				anim = m_sprProps.GetAnimationIdxByNameHash(parAnim->m_strArg.getHash());
			if (anim == -1)
			{
				anim = active->sprite.animIdx;
			}

			int frame = 0;
			if (parFrame)
			{
				frame = (int)parFrame->m_asINT32;
				CLAMP(frame, 0, m_sprProps.GetAFramesCnt(anim) - 1);
			}
			bool animated = active->bAnimated;
			if (parAnimated)
				animated = parAnimated->m_asBool;

			active->sprite.Init(&m_sprProps, anim, active->pos, frame);
			active->bAnimated = animated;
			//set new bbox
			RECTXYWH objbox = m_sprProps.GetAFrameBBox_real(active->sprite.animIdx, active->sprite.frameIdx);
			active->bbox_ini.Set(Vec2(objbox.x, objbox.y), Vec2(objbox.Right(), objbox.Bottom()));
			if (active->flipX)
			{
				active->bbox_ini.Move(Vec2(-2.0f * active->bbox_ini.vCenter.x, 0.0f));
			}
			active->bbox = active->bbox_ini;
			active->bbox.Move(active->pos);

			return true;
		}
		break;
		case instr_IACTIVE_TOGGLE_HIDDEN:
		{
			CVariantComplex* vcTarget = instr->GetArgument(L"target");
			//AI TARGET
			//default target is self

			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcTarget, executorUID);
			if (target == null)
			{
				LOG(L"SCRIPT::IACTIVE_TOGGLE_HIDDEN - invalid target param!\n");
				return true;
			}
			//toggle hidden flag
			target->bSetHidden = !target->bSetHidden;
			return true;
		}
		break;
		case instr_ACTIVE_DOORFACE_SET_OPEN_TIMER:
		{
			CVariantComplex* vcTarget = instr->GetArgument(L"target");
			CVariantComplex* vcDuration = instr->GetArgument(L"fDuration");
			IActiveInterface* target = ScriptGetActiveInterfaceByTargetParam(vcTarget, executorUID);
			if (target == null)
			{
				LOG(L"SCRIPT::ACTIVE_DOORFACE_SET_OPEN_TIMER - invalid target param!\n");
				return true;
			}
			//toggle hidden flag
			target->AItimer1 = vcDuration->m_asFloat;
			return true;
		}
		break;
		case instr_LEVEL_HIDE_BACKGROUND:
		{
			return true;
		}
		break;
		case instr_LEVEL_SHOW_BACKGROUND:
		{
			return true;
		}
		break;
		case instr_LEVEL_TOGGLE_BACKGROUND:
		{
			//m_bPaintBackground = !m_bPaintBackground;
			return true;
		}

		default:
		{
			ErrorBox(K_ERR_WARNING, L"[WARNING]CLevel::Script: Instruction recognized but not implemented!");
			return true;
		}
		break;
	}
}
