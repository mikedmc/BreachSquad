#include "dxstdafx.h"

CControllersManager::CControllersManager()
{
	m_arrControllers.RemoveAll();
	for (int kk = 0; kk < K_CM_CONTROLLERTYPES_CNT; kk++)
	{
		arrControllerTypesCnt[kk] = 0;
	}
}

CControllersManager::~CControllersManager()
{
	SAFE_DELETE_GROWABLE_ARRAY(m_arrControllers);
}

CController* CControllersManager::AddController(EControllerType neType, WCHAR * strName)
{
	CController* ctrl = new CController();

	ctrl->strName.Init(strName);
	ctrl->eType = neType;
	ResetKeypresses(ctrl);

	arrControllerTypesCnt[neType]++; //cresc numarul de controllere de un anume tip

	//default mappings
	switch(neType)
	{
		case K_CM_CONTROLLERTYPE_KEYBOARD_SDL:
		{
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_LEFT, SDL_SCANCODE_LEFT);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_RIGHT, SDL_SCANCODE_RIGHT);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_UP, SDL_SCANCODE_UP);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_DOWN, SDL_SCANCODE_DOWN);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_JUMP, SDL_SCANCODE_SPACE);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_FIRE1, SDL_SCANCODE_LCTRL);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_FIRE2, SDL_SCANCODE_LSHIFT);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_USE_GEAR, SDL_SCANCODE_E);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_RELOAD, SDL_SCANCODE_R);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_MELEE, SDL_SCANCODE_C);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_SELECT, SDL_SCANCODE_RETURN);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_SELECT, SDL_SCANCODE_SPACE);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_BACK, SDL_SCANCODE_ESCAPE);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_STRATEGIC_MENU, SDL_SCANCODE_V);
		}
		break;
		case K_CM_CONTROLLERTYPE_KEYBOARD_WIN:
		{
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_LEFT, VK_LEFT);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_RIGHT, VK_RIGHT);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_UP, VK_UP);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_DOWN, VK_DOWN);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_JUMP, VK_SPACE);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_FIRE1, VK_LCONTROL);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_FIRE2, VK_LSHIFT);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_USE_GEAR, 'R');
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_RELOAD, 'E');
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_MELEE, 'C');
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_SELECT, VK_RETURN);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_SELECT, VK_SPACE);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_BACK, VK_ESCAPE);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_STRATEGIC_MENU, 'V');
		}
		break;
		case K_CM_CONTROLLERTYPE_JOYSTICK_SDL:
		{
			ctrl->AddTrigger(K_CM_BUTTYPE_AXIS, K_CM_COMMAND_LEFT, SDL_CONTROLLER_AXIS_LEFTX, -1.1f, -0.6f);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_LEFT, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
			ctrl->AddTrigger(K_CM_BUTTYPE_AXIS, K_CM_COMMAND_RIGHT, SDL_CONTROLLER_AXIS_LEFTX, 0.6f, 1.1f);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_RIGHT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
			ctrl->AddTrigger(K_CM_BUTTYPE_AXIS, K_CM_COMMAND_UP, SDL_CONTROLLER_AXIS_LEFTY, -1.1f, -0.7f);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_UP, SDL_CONTROLLER_BUTTON_DPAD_UP);
			ctrl->AddTrigger(K_CM_BUTTYPE_AXIS, K_CM_COMMAND_DOWN, SDL_CONTROLLER_AXIS_LEFTY, 0.7f, 1.1f);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_DOWN, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_JUMP, SDL_CONTROLLER_BUTTON_A);
			
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_FIRE1, SDL_CONTROLLER_BUTTON_X);
			ctrl->AddTrigger(K_CM_BUTTYPE_AXIS, K_CM_COMMAND_FIRE1, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 0.5f, 1.1f);
			ctrl->AddTrigger(K_CM_BUTTYPE_AXIS, K_CM_COMMAND_FIRE2, SDL_CONTROLLER_AXIS_TRIGGERLEFT, 0.5f, 1.1f);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_USE_GEAR, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_RELOAD, SDL_CONTROLLER_BUTTON_B);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_MELEE, SDL_CONTROLLER_BUTTON_Y);
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_SELECT, SDL_CONTROLLER_BUTTON_BACK);	//SELECT during gameplay shows mapping
			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_BACK, SDL_CONTROLLER_BUTTON_START);	//start is usually MENU

			ctrl->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_STRATEGIC_MENU, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
		}
		break;
	}

	m_arrControllers.Add(ctrl);

	return ctrl;
}

int CControllersManager::RegisterAllSDLControllers()
{
	//add SDL joysticks
	int nJoysticks = SDL_NumJoysticks();
	int nGameControllers = 0;

	for (int i = 0; i < nJoysticks; i++)
	{
		if (SDL_IsGameController(i))
		{
			nGameControllers++;

			SDL_GameController *pad = SDL_GameControllerOpen(i);
			if (pad)
			{
				SDL_Joystick *joy = SDL_GameControllerGetJoystick(pad);
				int instanceID = SDL_JoystickInstanceID(joy);

				//get controller name
				char ctrlrname[MAX_PATH];
				WCHAR wctrlrname[MAX_PATH];
				StringCchPrintfA(ctrlrname, MAX_PATH, SDL_GameControllerName(pad));
				mbstowcs(wctrlrname, ctrlrname, MAX_PATH);

				CController* ctrlr = AddController(K_CM_CONTROLLERTYPE_JOYSTICK_SDL, wctrlrname);
				//save SDL data too
				ctrlr->nSDLInstanceId = instanceID;
				ctrlr->nSDLidx = i;
				ctrlr->SDLpgc = pad;

				LOG(L"Found SDL joystick[%d]: %s", i, wctrlrname);
			}
		}
	}

	return nGameControllers;
}

void CControllersManager::ReleaseAllControllers(bool bOnlySDL)
{
	for (int kk = m_arrControllers.GetSize() - 1; kk >= 0; kk--)
	{
		if ((bOnlySDL) && (m_arrControllers[kk]->eType != K_CM_CONTROLLERTYPE_JOYSTICK_SDL))
			continue;
		if (m_arrControllers[kk]->SDLpgc != null)
		{
			SDL_GameControllerClose(m_arrControllers[kk]->SDLpgc);
			m_arrControllers[kk]->SDLpgc = null;
		}
		SAFE_DELETE(m_arrControllers[kk]);
		arrControllerTypesCnt[K_CM_CONTROLLERTYPE_JOYSTICK_SDL]--;
		m_arrControllers.Remove(kk);
	}
}

const char* CControllersManager::GetSDLScancodeName(SDL_Scancode scancode)
{
	switch (scancode)
	{
		case SDL_SCANCODE_LCTRL:
			return "LCtrl";
		case SDL_SCANCODE_RCTRL:
			return "RCtrl";
		case SDL_SCANCODE_LSHIFT:
			return "LShift";
		case SDL_SCANCODE_RSHIFT:
			return "RShift";
		case SDL_SCANCODE_LALT:
			return "LAlt";
		case SDL_SCANCODE_RALT:
			return "RAlt";
		case SDL_SCANCODE_BACKSPACE:
			return "BkSp";
		case SDL_SCANCODE_SPACE:
			return "Spc";
		case SDL_SCANCODE_PAGEDOWN:
			return "PgDn";
		case SDL_SCANCODE_PAGEUP:
			return "PgUp";
	}
	
	//default return SDL defaults
	return SDL_GetScancodeName(scancode);
}

void CControllersManager::ResetKeypresses(CController* ctrlr)
{
	if (ctrlr == null)
		return;

	ctrlr->ResetKeypresses();
}

void CControllersManager::ResetAllControllersKeypresses()
{
	for (int kk = 0; kk < m_arrControllers.GetSize(); kk++)
	{
		ResetKeypresses(m_arrControllers[kk]);
	}
}

void CControllersManager::ReceiveKeypress(UINT nChar, bool bIsKeyDown, bool bAltDown)
{
	if (arrControllerTypesCnt[K_CM_CONTROLLERTYPE_KEYBOARD_WIN] <= 0)
		return;

	for (int kk = 0; kk < m_arrControllers.GetSize(); kk++)
	{
		CController* ctrlr = m_arrControllers[kk];
		if (ctrlr->eType != K_CM_CONTROLLERTYPE_KEYBOARD_WIN)
			continue;

		for (int ll = 0; ll < ctrlr->arrTriggersCnt; ll++)
		{
			if (ctrlr->arrTriggers[ll].eType != K_CM_BUTTYPE_BUTTON)
				continue;
			if (ctrlr->arrTriggers[ll].keyMapping == nChar)
			{
				ctrlr->arrTriggers[ll].fTriggerActivatedPercent = ((bIsKeyDown == true) ? 1.0f : 0.0f);
			}
		}
	}
}


bool CControllersManager::KeyPressed(EControllerCommand eCommandFilter /*= K_CM_COMMAND_NONE*/)
{
	for (int ll = 0; ll < m_arrControllers.GetSize(); ll++)
	{
		CController* ctrlr = m_arrControllers[ll];
		if (eCommandFilter == K_CM_COMMAND_NONE)
		{
			for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
			{
				if (ctrlr->sCommands.bKeyDown[kk])
					return true;
			}
		}
		else
		{
			if (ctrlr->sCommands.bKeyDown[eCommandFilter])
				return true;
		}
	}
	return false;
}

void CControllersManager::AddSDLController(int SDL_ctrlr_idx)
{
	if (SDL_IsGameController(SDL_ctrlr_idx)) 
	{
		SDL_GameController *pad = SDL_GameControllerOpen(SDL_ctrlr_idx);
		if (pad) 
		{
			SDL_Joystick *joy = SDL_GameControllerGetJoystick(pad);
			int instanceID = SDL_JoystickInstanceID(joy);

			//vede daca e deja adaugat ca sa nu il adauge de mai multe ori
			if (GetControllerByInstanceID(instanceID) != null)
			{
				return;
			}
			//get controller name
			char ctrlrname[MAX_PATH];
			WCHAR wctrlrname[MAX_PATH];
			StringCchPrintfA(ctrlrname, MAX_PATH, SDL_GameControllerName(pad));
			mbstowcs(wctrlrname, ctrlrname, MAX_PATH);

			CController* ctrlr = AddController(K_CM_CONTROLLERTYPE_JOYSTICK_SDL, wctrlrname);

			//save SDL data too
			ctrlr->nSDLInstanceId = instanceID;
			ctrlr->nSDLidx = SDL_ctrlr_idx;
			ctrlr->SDLpgc = pad;

			//ErrorBox(K_ERR_WARNING, L"added instanceID:%d", instanceID);

			//signal with event
			CEvent *nevent = new CEvent(CEventTypes::evtT_SYSTEM, CEventCommands::evtC_SYSTEM_CONTROLLER_ADDED);
			nevent->AddNamedArgINT32(L"SDLinstanceID", instanceID);
			nevent->AddNamedArgString(L"strName", wctrlrname);
			UTGetEventManager().QueueEvent(nevent);
		}
	}
}

bool CControllersManager::RemoveSDLController(int nnInstanceID)
{
	for (int kk = 0; kk < m_arrControllers.GetSize(); kk++)
	{
		if (m_arrControllers[kk]->nSDLInstanceId == nnInstanceID)
		{
			//signal with event
			CEvent *nevent = new CEvent(CEventTypes::evtT_SYSTEM, CEventCommands::evtC_SYSTEM_CONTROLLER_REMOVED);
			nevent->AddNamedArgINT32(L"SDLinstanceID", nnInstanceID);
			nevent->AddNamedArgString(L"strName", m_arrControllers[kk]->strName.text);
			UTGetEventManager().QueueEvent(nevent);

			//remove selected characters on local game
			//#TODO: instead of calling it directly playerSelScr should register for SYSTEM events
			g_playerSelScr.OnControllerRemoved(m_arrControllers[kk]->nSDLInstanceId);

			SDL_GameControllerClose(m_arrControllers[kk]->SDLpgc);
			arrControllerTypesCnt[m_arrControllers[kk]->eType]--;
			SAFE_DELETE(m_arrControllers[kk]);
			m_arrControllers.Remove(kk);

			return true;
		}
	}

	ErrorBox(K_ERR_WARNING, L"RemoveSDLController: SDL Controller instanceID=%d not found!", nnInstanceID);
	return false;
}

CController* CControllersManager::GetControllerByInstanceID(int nnInstanceID)
{
	for (int kk = 0; kk < m_arrControllers.GetSize(); kk++)
	{
		if (m_arrControllers[kk]->nSDLInstanceId == nnInstanceID)
			return m_arrControllers[kk];
	}
	return null;
}

CController* CControllersManager::GetControllerByName(WCHAR* strControllerName)
{
	for (int kk = 0; kk < m_arrControllers.GetSize(); kk++)
	{
		if (m_arrControllers[kk]->strName.IsEqual(strControllerName))
			return m_arrControllers[kk];
	}
	ErrorBox(K_ERR_WARNING, L"* CControllersManager::GetControllerByName - Controller [%s] not found! Returning null.", strControllerName);
	return null;
}

void CControllersManager::OnSDLControllerButton(const SDL_ControllerButtonEvent sdlEvent)
{
	if (arrControllerTypesCnt[K_CM_CONTROLLERTYPE_JOYSTICK_SDL] <= 0)
		return;
	CController* ctrlr = GetControllerByInstanceID(sdlEvent.which);
	if (ctrlr == null)
	{
		ErrorBox(K_ERR_WARNING, L"OnSDLControllerButton:instanceID=%d not found!", sdlEvent.which);
		return;
	}
	bool bButDown = false;
	float fButPress = 0.0f;
	if (sdlEvent.state == SDL_PRESSED)
	{
		bButDown = true;
		fButPress = 1.0f;
	}
	//find button
	for (int ll = 0; ll < ctrlr->arrTriggersCnt; ll++)
	{
		if (ctrlr->arrTriggers[ll].eType != K_CM_BUTTYPE_BUTTON)
			continue;
		if (ctrlr->arrTriggers[ll].keyMapping == sdlEvent.button)
		{
			ctrlr->arrTriggers[ll].fTriggerActivatedPercent = fButPress;
		}
	}
}

void CControllersManager::OnSDLControllerAxis(const SDL_ControllerAxisEvent sdlEvent)
{
	if (arrControllerTypesCnt[K_CM_CONTROLLERTYPE_JOYSTICK_SDL] <= 0)
		return;
	CController* ctrlr = GetControllerByInstanceID(sdlEvent.which);
	if (ctrlr == null)
	{
		ErrorBox(K_ERR_WARNING, L"OnSDLControllerAxis:instanceID=%d not found!", sdlEvent.which);
		return;
	}
	float perc = (float)sdlEvent.value / 32767.0f;
	//find button
	for (int ll = 0; ll < ctrlr->arrTriggersCnt; ll++)
	{
		if (ctrlr->arrTriggers[ll].eType != K_CM_BUTTYPE_AXIS)
			continue;
		//trec prin toate controalele pentru ca pe o axa sunt 2 comenzi
		if (ctrlr->arrTriggers[ll].keyMapping == sdlEvent.axis)
		{
			bool bButDown = false;
			float fMin = ctrlr->arrTriggers[ll].fTriggerMin;
			float fMax = ctrlr->arrTriggers[ll].fTriggerMax;
			//already activated? move the INACTIVE domain a little to avoid analog jitter
			if (ctrlr->arrTriggers[ll].fTriggerActivatedPercent > 0.0f)
			{
				//  |----fmin----fmax---|zero|---fmin----fmax----|
				if (fMax < 0.05f)
					fMax += 0.05f;
				if (fMin > 0.05f)
					fMin -= 0.05f;
			}
			if ((perc >= fMin) && (perc <= fMax))
			{
				bButDown = true;
			}

			ctrlr->arrTriggers[ll].fTriggerActivatedPercent = ((bButDown == true) ? fabs(perc) : 0.0f);
		}
	}
}


void CControllersManager::OnSDLKeypress(const SDL_KeyboardEvent sdlEvent, bool bKeyDown)
{
	if (arrControllerTypesCnt[K_CM_CONTROLLERTYPE_KEYBOARD_SDL] <= 0)
		return;

	for (int kk = 0; kk < m_arrControllers.GetSize(); kk++)
	{
		CController* ctrlr = m_arrControllers[kk];
		if (ctrlr->eType != K_CM_CONTROLLERTYPE_KEYBOARD_SDL)
			continue;

		for (int ll = 0; ll < ctrlr->arrTriggersCnt; ll++)
		{
			if (ctrlr->arrTriggers[ll].eType != K_CM_BUTTYPE_BUTTON)
				continue;
			if (ctrlr->arrTriggers[ll].keyMapping == sdlEvent.keysym.scancode)
			{
				ctrlr->arrTriggers[ll].fTriggerActivatedPercent = ((bKeyDown == true) ? 1.0f : 0.0f);
			}
		}
	}

}


///----- CController -----

CController::CController()
{
	arrTriggersCnt = 0;
	nFlags = 0;

	//SDL
	nSDLidx = -1;
	nSDLInstanceId = -1;
	SDLpgc = null;

	fTimeSinceKeypress = 0.0f;
	for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
	{
		bKeyDown[kk] = false;
		fKeyDownPercent[kk] = 0.0f;
	}
}

void CController::UpdateCommands(float dTime)
{
	sCommands.UpdateCommands(dTime, fKeyDownPercent);
	//increase last keypress time
	fTimeSinceKeypress += dTime;

#if defined(ENABLE_CHEATS)
	//you have 2 seconds for next keypress
	if ((fTimeSinceKeypress >= 0.25f) && (fTimeSinceKeypress - dTime < 0.25f))
	{
		arrStackHistory.Clear();
	}
#endif
}

void CController::AddTrigger(EControllerTriggerType neType, EControllerCommand neCommand, int nKeyMapping, float nfTriggerMin /*= 0.1f*/, float nfTriggerMax /*= 1.1f*/)
{
	assert(arrTriggersCnt < K_CM_MAX_TRIGGERS);

	arrTriggers[arrTriggersCnt].eType = neType;
	arrTriggers[arrTriggersCnt].eTargetCommand = neCommand;
	arrTriggers[arrTriggersCnt].keyMapping = nKeyMapping;
	arrTriggers[arrTriggersCnt].fTriggerMin = nfTriggerMin;
	arrTriggers[arrTriggersCnt].fTriggerMax = nfTriggerMax;
	arrTriggers[arrTriggersCnt].fTriggerActivatedPercent = 0.0f;

	arrTriggersCnt++;
}

int CController::GetKeyMappingForCommand(EControllerCommand neCommand)
{
	for (int kk = 0; kk < arrTriggersCnt; kk++)
	{
		if (arrTriggers[kk].eTargetCommand == neCommand)
			return arrTriggers[kk].keyMapping;
	}
	return -1;
}

CControllerTrigger* CController::GetTriggerForCommand(EControllerCommand neCommand)
{
	for (int kk = 0; kk < arrTriggersCnt; kk++)
	{
		if (arrTriggers[kk].eTargetCommand == neCommand)
			return &arrTriggers[kk];
	}
	return null;
}

void CController::RemoveTriggers(EControllerCommand neCommand)
{
	for (int kk = 0; kk < arrTriggersCnt; kk++)
	{
		if (arrTriggers[kk].eTargetCommand == neCommand)
		{
			for (int ll = kk; ll < arrTriggersCnt - 1; ll++)
			{
				arrTriggers[ll] = arrTriggers[ll + 1];
			}

			arrTriggersCnt--;
		}
	}
}

void CController::RemoveTriggerByKeyMapping(int nKeyMapping)
{
	for (int kk = 0; kk < arrTriggersCnt; kk++)
	{
		if (arrTriggers[kk].keyMapping == nKeyMapping)
		{
			for (int ll = kk; ll < arrTriggersCnt - 1; ll++)
			{
				arrTriggers[ll] = arrTriggers[ll + 1];
			}

			arrTriggersCnt--;
		}
	}
}

void CController::ClearTriggers()
{
	arrTriggersCnt = 0;
}

void CController::TranslateTriggersToCommands()
{
#if defined(ENABLE_CHEATS)
	const int K_CHEATS_CNT = 2;
	const int K_CHEATS_COMBINATION_LENGTH = 8;
	EControllerCommand matCombinations[K_CHEATS_CNT][K_CHEATS_COMBINATION_LENGTH] = {
		{K_CM_COMMAND_LEFT, K_CM_COMMAND_LEFT, K_CM_COMMAND_RIGHT, K_CM_COMMAND_RIGHT, K_CM_COMMAND_DOWN, K_CM_COMMAND_UP, K_CM_COMMAND_DOWN, K_CM_COMMAND_MELEE},  //give stars
		{K_CM_COMMAND_LEFT, K_CM_COMMAND_LEFT, K_CM_COMMAND_LEFT, K_CM_COMMAND_DOWN, K_CM_COMMAND_RIGHT, K_CM_COMMAND_RIGHT, K_CM_COMMAND_RIGHT, K_CM_COMMAND_MELEE}, //give strategic points
	};
#endif

	if ((eType == K_CM_CONTROLLERTYPE_INVALID) || (eType == K_CM_CONTROLLERTYPE_NETWORK_FRAMELOCK))
		return;

	float fPressedPerc[K_CM_COMMANDS_COUNT] = { 0.0f };
	for (int kk = 0; kk < arrTriggersCnt; kk++)
	{
		CControllerTrigger* trig = &arrTriggers[kk];
		int command = trig->eTargetCommand;
		if (trig->fTriggerActivatedPercent > fPressedPerc[command])
			fPressedPerc[command] = trig->fTriggerActivatedPercent;
	}

	for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
	{
		bool bOldKeydownState = bKeyDown[kk];
		//translate data
		fKeyDownPercent[kk] = fPressedPerc[kk];
		bKeyDown[kk] = (fKeyDownPercent[kk] != 0.0f);
		//update keypress timer
		if(bKeyDown[kk] == true)
			fTimeSinceKeypress = 0.0f;

#if defined(ENABLE_CHEATS)
		if ((!UTGetAppClass().IsGameNetworked()) && (g_gameState == GAME_STATE_GAME) && (bKeyDown[kk] == true) && (bOldKeydownState == false))
		{
			arrStackHistory.Add((EControllerCommand)kk);

			if (arrStackHistory.Count() >= 8) //normal cheats len
			{
				for (int nCheatIdx = 0; nCheatIdx < K_CHEATS_CNT; nCheatIdx++)
				{
					bool bFound = true;
					for (int mm = 0; mm < K_CHEATS_COMBINATION_LENGTH; mm++)
					{
						if (matCombinations[nCheatIdx][mm] != arrStackHistory[arrStackHistory.nTail + mm])
						{
							bFound = false;
							break;
						}
					}
					if (bFound)
					{
						switch (nCheatIdx)
						{
							case 0:	//give stars
							{
								//give stars
								int nMaxStars = UTGetChaptersList().GetChaptersCnt() * K_GAME_LEVELS_PER_CHAPTER * 3;
								if (g_userData[K_MEMID_STARS_TOTAL] < nMaxStars)
									g_userData[K_MEMID_STARS_TOTAL] += 5;
								else 
									g_userData[K_MEMID_STARS_SPENT] -= 5;
								//limit
								if (g_userData[K_MEMID_STARS_SPENT] < 0)
									g_userData[K_MEMID_STARS_SPENT] = 0;
								if (g_userData[K_MEMID_STARS_TOTAL] > nMaxStars)
									g_userData[K_MEMID_STARS_TOTAL] = nMaxStars;

								App_UpdateLevelStats();
								LOG(L"CHEATER! More Stars to Spend!");

								D3DXVECTOR2 vpos = g_level.m_camLevel.GetCamWorldAABB().Center();
								g_particlesMgr.AddStringParticle(g_font12wow, L"CHEATER!", &vpos, NULL, NULL, 3.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0xffffffff, K_PART_LAYER_NORMAL);
								g_particlesMgr.AddStringParticle(g_font8b1, L"More Stars to Spend", &D3DXVECTOR2(vpos.x, vpos.y + 10.0f), NULL, NULL, 3.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0xffffffff, K_PART_LAYER_NORMAL);

								SND_PLAY(SNDIDX_STARHIT);
							}
							break;
							case 1: //give Strategic Points
							{
								g_level.GiveStrategicPoints(4.0f);
								LOG(L"CHEATER! Strategic Points");

								D3DXVECTOR2 vpos = g_level.m_camLevel.GetCamWorldAABB().Center();
								g_particlesMgr.AddStringParticle(g_font12wow, L"CHEATER!", &vpos, NULL, NULL, 3.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0xffffffff, K_PART_LAYER_NORMAL);
								g_particlesMgr.AddStringParticle(g_font8b1, L"Strategic Points", &D3DXVECTOR2(vpos.x, vpos.y + 10.0f), NULL, NULL, 3.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0xffffffff, K_PART_LAYER_NORMAL);

								SND_PLAY(SNDIDX_STARHIT);
							}
							break;
						}
						//clear history
						arrStackHistory.Clear();
						break; //!!!
					}
				}
			}
		}
#endif
	}

}

void CController::ResetKeypresses()
{
	//reset flags too
	nFlags = 0;

	for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
	{
		bKeyDown[kk] = false;
		fKeyDownPercent[kk] = 0.0f;
		//reset commands too
		sCommands.Reset();
	}
}

void CController::GetKeysDownPercents(float arrDest[K_CM_COMMANDS_COUNT])
{
	memcpy(arrDest, fKeyDownPercent, sizeof(float) * K_CM_COMMANDS_COUNT);
}

bool CController::WasControllerTouched(bool bSticksToo /*= false*/)
{
	for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
	{
		//lower than "down" we have only directionals
		if ((!bSticksToo) && (kk <= K_CM_COMMAND_DOWN))
			continue;

		if (sCommands.bKeyDown[kk])
			return true;
	}
	return false;
}

///----- sControllerCommands -----

CController::sControllerCommands::sControllerCommands()
{
	for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
	{
		bKeyDown[kk] = false;
		keyState[kk] = K_CM_BUTSTATE_NOTPRESSED;
		fKeyDownPercent[kk] = 0.0f;
		fKeyPressedTime[kk] = 0.0f;
	}
}

void CController::sControllerCommands::UpdateCommands(float dTime, float fKeysPressedPercents[K_CM_COMMANDS_COUNT])
{
	for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
	{
		//load data from param
		fKeyDownPercent[kk] = fKeysPressedPercents[kk];
		bKeyDown[kk] = (fKeyDownPercent[kk] > 0.0f);

		//update pressed time
		fKeyPressedTime[kk] += dTime;

		if (bKeyDown[kk])
		{
			if (keyState[kk] == K_CM_BUTSTATE_NOTPRESSED)
			{
				keyState[kk] = K_CM_BUTSTATE_JUSTPRESSED; //just pressed
			}
			else if (keyState[kk] == K_CM_BUTSTATE_JUSTPRESSED)
			{
				keyState[kk] = K_CM_BUTSTATE_PRESSING; //drag
			}
		}
		else
		{
			if (keyState[kk] == K_CM_BUTSTATE_PRESSING)
			{
				keyState[kk] = K_CM_BUTSTATE_JUSTRELEASED;
			}
			else
			{
				keyState[kk] = K_CM_BUTSTATE_NOTPRESSED;
			}
			//reset pressed timer when not pressed
			fKeyPressedTime[kk] = 0.0f;
		}
	}
}


void CController::sControllerCommands::Reset()
{
	for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
	{
		bKeyDown[kk] = false;
		keyState[kk] = K_CM_BUTSTATE_NOTPRESSED;
		fKeyDownPercent[kk] = 0.0f;
		fKeyPressedTime[kk] = 0.0f;
	}
}

///**************************************************************************************
/// Sigleton de acces
///**************************************************************************************

CControllersManager& UTGetControllersManager()
{
	static CControllersManager g_ControllersManager;
	return g_ControllersManager;
}
