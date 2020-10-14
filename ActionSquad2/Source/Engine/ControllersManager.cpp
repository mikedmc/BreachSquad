#include "dxstdafx.h"

CControllersManager::CControllersManager()
{
	// reset normalize function
	pNormalizeFn = nullptr;

	m_arrControllers.RemoveAll();
	for (int kk = 0; kk < K_CM_CTS_CNT; kk++)
	{
		arrControllerTypesCnt[kk] = 0;
	}
}

CControllersManager::~CControllersManager()
{
	SAFE_DELETE_GROWABLE_ARRAY(m_arrControllers);
	// reset normalize function
	pNormalizeFn = nullptr;
}

void CControllersManager::SetNormalizeCoordsFunctionPtr(NormalizeCoordsFn pFnPtr)
{
	pNormalizeFn = pFnPtr;
	DebugPrintA("--CControllersManager:: Normalize ptr set to %d \n", pFnPtr);
}

CController* CControllersManager::AddController(EControllerType neType, WCHAR * strName)
{
	CController* ctrl = new CController();

	ctrl->strName.Init(strName);
	ctrl->eType = neType;
	ResetKeypresses(ctrl);

	arrControllerTypesCnt[neType]++; //cresc numarul de controllere de un anume tip

	//default mappings
	switch (neType)
	{
		case K_CM_CT_KBM_SDL:
		{
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_MOVE_X, SDL_SCANCODE_LEFT, 0.0f, -1.0f);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_MOVE_X, SDL_SCANCODE_RIGHT, 0.0f, 1.0f);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_MOVE_Y, SDL_SCANCODE_UP, 0.0f, -1.0f);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_MOVE_Y, SDL_SCANCODE_DOWN, 0.0f, 1.0f);

			//ctrl->AddTrigger(K_CM_POINTER_BUTTON, K_CM_COMMAND_FIRE1, SDL_BUTTON_LEFT, 0.0f, 1.0f);
			ctrl->AddTrigger(K_CM_POINTER_X, K_CM_COMMAND_AIM_X, 0);
			ctrl->AddTrigger(K_CM_POINTER_Y, K_CM_COMMAND_AIM_Y, 0);

			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_JUMP, SDL_SCANCODE_SPACE, 0.0f, 1.0f);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_FIRE1, SDL_SCANCODE_LCTRL, 0.0f, 1.0f);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_FIRE2, SDL_SCANCODE_LSHIFT, 0.0f, 1.0f);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_USE_GEAR, SDL_SCANCODE_E, 0.0f, 1.0f);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_RELOAD, SDL_SCANCODE_R, 0.0f, 1.0f);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_MELEE, SDL_SCANCODE_C, 0.0f, 1.0f);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_SELECT, SDL_SCANCODE_RETURN, 0.0f, 1.0f);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_SELECT, SDL_SCANCODE_SPACE, 0.0f, 1.0f);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_BACK, SDL_SCANCODE_ESCAPE, 0.0f, 1.0f);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_STRATEGIC_MENU, SDL_SCANCODE_V, 0.0f, 1.0f);
		}
		break;
		/*
		case K_CM_CONTROLLERTYPE_KEYBOARD_WIN:
		{
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_LEFT, VK_LEFT);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_RIGHT, VK_RIGHT);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_UP, VK_UP);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_DOWN, VK_DOWN);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_JUMP, VK_SPACE);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_FIRE1, VK_LCONTROL);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_FIRE2, VK_LSHIFT);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_USE_GEAR, 'R');
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_RELOAD, 'E');
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_MELEE, 'C');
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_SELECT, VK_RETURN);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_SELECT, VK_SPACE);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_BACK, VK_ESCAPE);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_STRATEGIC_MENU, 'V');
		}
		break;
		*/
		case K_CM_CT_JOYSTICK_SDL:
		{
			ctrl->AddTrigger(K_CM_AXIS, K_CM_COMMAND_MOVE_X, SDL_CONTROLLER_AXIS_LEFTX, 0.2f, 1.0f);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_MOVE_X, SDL_CONTROLLER_BUTTON_DPAD_LEFT, 0.0f, -1.0f);
			ctrl->AddTrigger(K_CM_AXIS, K_CM_COMMAND_MOVE_Y, SDL_CONTROLLER_AXIS_LEFTY, 0.2f, 1.0f);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_MOVE_Y, SDL_CONTROLLER_BUTTON_DPAD_UP, 0.0f, -1.0f);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_MOVE_Y, SDL_CONTROLLER_BUTTON_DPAD_DOWN, 0.0f, 1.0f);

			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_JUMP, SDL_CONTROLLER_BUTTON_A);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_FIRE1, SDL_CONTROLLER_BUTTON_X);
			ctrl->AddTrigger(K_CM_HALF_AXIS, K_CM_COMMAND_FIRE1, SDL_CONTROLLER_AXIS_TRIGGERRIGHT, 0.5f, 1.1f);
			ctrl->AddTrigger(K_CM_HALF_AXIS, K_CM_COMMAND_FIRE2, SDL_CONTROLLER_AXIS_TRIGGERLEFT, 0.5f, 1.1f);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_USE_GEAR, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_RELOAD, SDL_CONTROLLER_BUTTON_B);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_MELEE, SDL_CONTROLLER_BUTTON_Y);
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_SELECT, SDL_CONTROLLER_BUTTON_BACK);	//SELECT during gameplay shows mapping
			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_BACK, SDL_CONTROLLER_BUTTON_START);	//start is usually MENU

			ctrl->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_STRATEGIC_MENU, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
		}
		break;
	}

	m_arrControllers.Add(ctrl);

	return ctrl;
}

int CControllersManager::RegisterAllSDLControllers()
{
	//add SDL joysticks
	const int nJoysticks = SDL_NumJoysticks();
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

				CController* ctrlr = AddController(K_CM_CT_JOYSTICK_SDL, wctrlrname);
				//save SDL data too
				ctrlr->nSDLInstanceId = instanceID;
				ctrlr->nSDLidx = i;
				ctrlr->SDLpgc = pad;

				LOG(TEXT("Found SDL joystick[%d]: %s"), i, wctrlrname);
			}
		}
	}

	return nGameControllers;
}

void CControllersManager::ReleaseAllControllers(bool bOnlySDL)
{
	for (int kk = m_arrControllers.GetSize() - 1; kk >= 0; kk--)
	{
		if ((bOnlySDL) && (m_arrControllers[kk]->eType != K_CM_CT_JOYSTICK_SDL))
			continue;
		if (m_arrControllers[kk]->SDLpgc != null)
		{
			SDL_GameControllerClose(m_arrControllers[kk]->SDLpgc);
			m_arrControllers[kk]->SDLpgc = null;
		}
		SAFE_DELETE(m_arrControllers[kk]);
		arrControllerTypesCnt[K_CM_CT_JOYSTICK_SDL]--;
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

/*
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
			if (ctrlr->arrTriggers[ll].eType != K_CM_BUTTON)
				continue;
			if (ctrlr->arrTriggers[ll].keyMapping == nChar)
			{
				ctrlr->arrTriggers[ll].fTriggerActivatedPercent = ((bIsKeyDown == true) ? 1.0f : 0.0f);
			}
		}
	}
}
*/


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

void CControllersManager::UpdateController(CController* ctrlr, float dTime, float * arrOverrideDownPercents)
{
	if ((ctrlr == nullptr) || (ctrlr->eType == K_CM_CT_INVALID) || (ctrlr->eType == K_CM_CT_NET_FRAMELOCK))
		return;

	///--- 1. Translate triggers to commands or handle override

	// if we don't override pressed precents then data is computed from triggers
	if (arrOverrideDownPercents == null)
	{
		float fPressedPerc[K_CM_COMMANDS_COUNT] = { 0.0f };
		for (int kk = 0; kk < ctrlr->arrTriggersCnt; kk++)
		{
			CControllerTrigger* trig = &ctrlr->arrTriggers[kk];
			float fAxisVal = trig->fTriggerActivatedPercent;
			// normalize axis if necessary without touching the internal data
			if (pNormalizeFn)
			{
				if (trig->eType == K_CM_POINTER_X)
				{
					(*pNormalizeFn)(ctrlr->nSDLInstanceId, trig->fTriggerActivatedPercent, true, fAxisVal);
				}
				if (trig->eType == K_CM_POINTER_Y)
				{
					(*pNormalizeFn)(ctrlr->nSDLInstanceId, trig->fTriggerActivatedPercent, false, fAxisVal);
				}
			}
			// sum all triggers here
			fPressedPerc[trig->eTargetCommand] += fAxisVal;
		}
		// copy final data to normalized data
		memcpy(ctrlr->sCommands.arrAxisVal_N, fPressedPerc, sizeof(float) * K_CM_COMMANDS_COUNT);
	}
	else
	{
		// copy override data (could come from the network step)
		memcpy(ctrlr->sCommands.arrAxisVal_N, arrOverrideDownPercents, sizeof(float) * K_CM_COMMANDS_COUNT);
	}

	///--- 2. Update internal commands structure
	CController::sControllerCommands* scom = &ctrlr->sCommands;
	for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
	{
		scom->bKeyDown[kk] = (fabs(scom->arrAxisVal_N[kk]) > 0.0f);

		//update pressed time
		scom->fKeyPressedTime[kk] += dTime;

		if (scom->bKeyDown[kk])
		{
			if (scom->keyState[kk] == K_CM_BUTSTATE_NOTPRESSED || scom->keyState[kk] == K_CM_BUTSTATE_JUSTRELEASED)
			{
				scom->keyState[kk] = K_CM_BUTSTATE_JUSTPRESSED; //just pressed
			}
			else if (scom->keyState[kk] == K_CM_BUTSTATE_JUSTPRESSED)
			{
				scom->keyState[kk] = K_CM_BUTSTATE_PRESSING; //drag
			}
		}
		else
		{
			if (scom->keyState[kk] == K_CM_BUTSTATE_PRESSING || scom->keyState[kk] == K_CM_BUTSTATE_JUSTPRESSED)
			{
				scom->keyState[kk] = K_CM_BUTSTATE_JUSTRELEASED;
			}
			else
			{
				scom->keyState[kk] = K_CM_BUTSTATE_NOTPRESSED;
			}
			//reset pressed timer when not pressed
			scom->fKeyPressedTime[kk] = 0.0f;
		}
	}
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

			CController* ctrlr = AddController(K_CM_CT_JOYSTICK_SDL, wctrlrname);

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

	ErrorBox(K_ERR_WARNING, TEXT("RemoveSDLController: SDL Controller instanceID=%d not found!"), nnInstanceID);
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
	ErrorBox(K_ERR_WARNING, TEXT("* CControllersManager::GetControllerByName - Controller [%s] not found! Returning null."), strControllerName);
	return null;
}

void CControllersManager::OnSDLControllerButton(const SDL_ControllerButtonEvent sdlEvent)
{
	if (arrControllerTypesCnt[K_CM_CT_JOYSTICK_SDL] <= 0)
		return;
	CController* ctrlr = GetControllerByInstanceID(sdlEvent.which);
	if (ctrlr == null)
	{
		ErrorBox(K_ERR_WARNING, TEXT("OnSDLControllerButton:instanceID=%d not found!"), sdlEvent.which);
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
		if (ctrlr->arrTriggers[ll].eType != K_CM_BUTTON)
			continue;
		if (ctrlr->arrTriggers[ll].keyMapping == sdlEvent.button)
		{
			ctrlr->arrTriggers[ll].fTriggerActivatedPercent = fButPress;
		}
	}
}

void CControllersManager::OnSDLControllerAxis(const SDL_ControllerAxisEvent sdlEvent)
{
	if (arrControllerTypesCnt[K_CM_CT_JOYSTICK_SDL] <= 0)
		return;
	CController* ctrlr = GetControllerByInstanceID(sdlEvent.which);
	if (ctrlr == null)
	{
		ErrorBox(K_ERR_WARNING, TEXT("OnSDLControllerAxis:instanceID=%d not found!"), sdlEvent.which);
		return;
	}
	float perc = (float)sdlEvent.value / 32767.0f;
	//find trigger
	for (int ll = 0; ll < ctrlr->arrTriggersCnt; ll++)
	{
		// only axis triggers selected
		if (ctrlr->arrTriggers[ll].eType != K_CM_HALF_AXIS && ctrlr->arrTriggers[ll].eType != K_CM_AXIS)
			continue;
		//trec prin toate controalele pentru ca pe o axa sunt 2 comenzi
		CControllerTrigger* trigger = &ctrlr->arrTriggers[ll];
		if (trigger->keyMapping != sdlEvent.axis)
			continue;

		if (trigger->eType == K_CM_HALF_AXIS)
		{
			bool bButDown = false;
			float fMin = trigger->fTriggerMin;
			float fMax = trigger->fTriggerMax;
			//already activated? move the INACTIVE domain a little to avoid analog jitter
			if (trigger->fTriggerActivatedPercent > 0.0f)
			{
				//  |----fmin----fmax---|zero|---fmin----fmax----|
				if (fMax < 0.05f)
					fMax += 0.05f;
				if (fMin > 0.05f)
					fMin -= 0.05f;
			}
			if ((perc >= fMin) && (perc <= fMax))
				bButDown = true;

			trigger->fTriggerActivatedPercent = ((bButDown == true) ? perc : 0.0f);
		}
		// AXIS - returns actual +/- percent if over the minimum threshold
		else if (trigger->eType == K_CM_AXIS)
		{
			bool bButDown = false;
			float fMinAbs = fabs(trigger->fTriggerMin);
			//already activated? move the INACTIVE domain a little to avoid analog jitter
			if (fabs(trigger->fTriggerActivatedPercent) > 0.0f)
			{
				//  |--------fminabs---|zero|---fminabs--------|
				if (fMinAbs > 0.05f)
					fMinAbs -= 0.05f;
			}
			if (fabs(perc) >= fMinAbs)
				bButDown = true;

			trigger->fTriggerActivatedPercent = ((bButDown == true) ? perc : 0.0f);
		}
	}
}


void CControllersManager::OnSDLKeypress(const SDL_KeyboardEvent sdlEvent, bool bKeyDown)
{
	if (arrControllerTypesCnt[K_CM_CT_KBM_SDL] <= 0)
		return;

	for (int kk = 0; kk < m_arrControllers.GetSize(); kk++)
	{
		CController* ctrlr = m_arrControllers[kk];
		if (ctrlr->eType != K_CM_CT_KBM_SDL)
			continue;

		for (int ll = 0; ll < ctrlr->arrTriggersCnt; ll++)
		{
			CControllerTrigger* trigger = &ctrlr->arrTriggers[ll];
			if ((trigger->eType != K_CM_BUTTON) || (trigger->keyMapping != sdlEvent.keysym.scancode))
				continue;
			// set absolute values (+/- values) set when defining triggers
			trigger->fTriggerActivatedPercent = ((bKeyDown == true) ? trigger->fTriggerMax : trigger->fTriggerMin);
		}
	}

}


void CControllersManager::OnSDLMouseButton(const SDL_MouseButtonEvent sdlEvent)
{
	if (arrControllerTypesCnt[K_CM_CT_KBM_SDL] <= 0)
		return;

	bool bButDown = false;
	float fButPress = 0.0f;
	if (sdlEvent.state == SDL_PRESSED)
	{
		bButDown = true;
		fButPress = 1.0f;
	}
	//find button
	for (int kk = 0; kk < m_arrControllers.GetSize(); kk++)
	{
		CController* ctrlr = m_arrControllers[kk];
		if (ctrlr->eType != K_CM_CT_KBM_SDL)
			continue;

		for (int ll = 0; ll < ctrlr->arrTriggersCnt; ll++)
		{
			if (ctrlr->arrTriggers[ll].eType != K_CM_POINTER_BUTTON)
				continue;
			if (ctrlr->arrTriggers[ll].keyMapping == sdlEvent.button)
			{
				ctrlr->arrTriggers[ll].fTriggerActivatedPercent = fButPress;
			}
		}
	}
}

void CControllersManager::OnSDLMouseMove(const SDL_MouseMotionEvent sdlEvent)
{
	if (arrControllerTypesCnt[K_CM_CT_KBM_SDL] <= 0)
		return;
	//find button
	for (int kk = 0; kk < m_arrControllers.GetSize(); kk++)
	{
		CController* ctrlr = m_arrControllers[kk];
		if (ctrlr->eType != K_CM_CT_KBM_SDL)
			continue;
		// if we have a normalization fn pointer then call it on the data
		float retX = (float)sdlEvent.x;
		float retY = (float)sdlEvent.y;

		for (int ll = 0; ll < ctrlr->arrTriggersCnt; ll++)
		{
			if (ctrlr->arrTriggers[ll].eType == K_CM_POINTER_X)
			{
				ctrlr->arrTriggers[ll].fTriggerActivatedPercent = retX;
			}
			else if (ctrlr->arrTriggers[ll].eType == K_CM_POINTER_Y)
			{
				ctrlr->arrTriggers[ll].fTriggerActivatedPercent = retY;
			}
		}
	}
}

///----- CController -----

CController::CController() : 
	eType(K_CM_CT_INVALID), arrTriggersCnt(0), nFlags(0), 
	nSDLidx(-1), nSDLInstanceId(-1), SDLpgc(nullptr)
{
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
	return nullptr;
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

void CController::ResetKeypresses()
{
	//reset flags too
	nFlags = 0;
	sCommands.Reset();
}

void CController::GetKeysDownPercents(float arrDest[K_CM_COMMANDS_COUNT])
{
	memcpy(arrDest, sCommands.arrAxisVal_N, sizeof(float) * K_CM_COMMANDS_COUNT);
}

bool CController::WasControllerTouched(bool bSticksToo /*= false*/)
{
	for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
	{
		//lower than "down" we have only directionals
		/*
		if ((!bSticksToo) && (kk <= K_CM_COMMAND_DOWN))
			continue;
		*/
		if (sCommands.bKeyDown[kk])
			return true;
	}
	return false;
}


D3DXVECTOR2 CController::GetDoubleAxisVectorN(const EControllerCommand commXaxis, const EControllerCommand commYaxis)
{
	D3DXVECTOR2 retvec(0.0f, 0.0f);
	D3DXVec2Normalize(&retvec, &D3DXVECTOR2(sCommands.arrAxisVal_N[commXaxis], sCommands.arrAxisVal_N[commYaxis]));
	return retvec;
}

///----- sControllerCommands -----

CController::sControllerCommands::sControllerCommands()
{
	Reset();
}

void CController::sControllerCommands::Reset()
{
	for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
	{
		bKeyDown[kk] = false;
		keyState[kk] = K_CM_BUTSTATE_NOTPRESSED;
		fKeyPressedTime[kk] = 0.0f;
		arrAxisVal_N[kk] = 0.0f;
	}
}


///**************************************************************************************
/// Sigleton de acces
///**************************************************************************************

CControllersManager& UTGetCtrlrMgr()
{
	static CControllersManager g_ControllersManager;
	return g_ControllersManager;
}
