#include "dxstdafx.h"

//**************************************************************************************
// Level data and user data, settings
//**************************************************************************************
///--- LEVEL DATA ---

//IMPORTANT!!! When changing max number of chapters make sure you update the LoadUserData fn
#define K_GAME_MAX_CHAPTERS 10
CLevelStats g_levelStats[K_GAME_LEVELS_PER_CHAPTER * K_GAME_MAX_CHAPTERS];	//contains enough data for 10 chapters so we don't resize

void App_UpdateLevelStats()
{
	//make sure we have enough space (unlikely):
	if (UTGetChaptersList().GetTotalLevelsCnt() >= K_GAME_LEVELS_PER_CHAPTER * 10)
	{
		ErrorBox(K_ERR_CRITICAL, L"[ERROR] App_UpdateLevelStats::Array too small!!! Contact support!");
	}

	__Texts().SetString(STR_TOTAL_STARS_VAL, L"%d", g_userData[K_MEMID_STARS_TOTAL] - g_userData[K_MEMID_STARS_SPENT]);
	//count played missions, total scores, etc
	int nCompletedMissions = 0;
	g_userData[K_MEMID_TOTAL_SCORE_SOLO] = 0;
	g_userData[K_MEMID_TOTAL_SCORE_COOP] = 0;

	for (int ii = 0; ii < UTGetChaptersList().GetTotalLevelsCnt(); ii++)
	{
		if (g_levelStats[ii].nStars > 0)
			nCompletedMissions++;
		g_userData[K_MEMID_TOTAL_SCORE_SOLO] += g_levelStats[ii].nScoreSolo;
		g_userData[K_MEMID_TOTAL_SCORE_COOP] += g_levelStats[ii].nScoreCoop;
	}
	//save completed missions
	g_userData[K_MEMID_MISSIONS_COMPLETED] = nCompletedMissions;
}

///--- PERSISTENT MEMORY ---

//--- USER DATA ---
int	g_userData[K_MEMID_SLOTS_COUNT];

void App_IncreaseGamestat(int K_MEMID_GAMESTATS_var, int nAddQuantity)
{
	assert((K_MEMID_GAMESTATS_var >= K_MEMID_GAMESTATS_START) && (K_MEMID_GAMESTATS_var <= K_MEMID_GAMESTATS_END));

	int nOldVal = g_userData[K_MEMID_GAMESTATS_var];
	g_userData[K_MEMID_GAMESTATS_var] += nAddQuantity;
	int nNewVal = g_userData[K_MEMID_GAMESTATS_var];

	switch (K_MEMID_GAMESTATS_var)
	{
		case K_MEMID_GAMESTATS_ARREST_TARGETS_ARRESTED:
		{
			__Achievements().IncStat(EGameStats::N_STAT_ARRESTS_MADE, 1);
#ifndef K_AUTO_ACHIEVE_FROM_STATS
			if (nNewVal >= 5)
				__Achievements().UnlockAchievement(ACH_WARRANT_SERVER);
#endif
		}
		break;

		case K_MEMID_GAMESTATS_KILLS_ASSAULTER:
		{
			__Achievements().IncStat(EGameStats::N_STAT_ASSAULTER_KILLS, (float)nAddQuantity);
#ifndef K_AUTO_ACHIEVE_FROM_STATS
			if (nNewVal >= 100)
				__Achievements().UnlockAchievement(ACH_DEATH_DEALER);
#endif
		}
		break;
		case K_MEMID_GAMESTATS_KILLS_BREACHER:
		{
			__Achievements().IncStat(EGameStats::N_STAT_BREACHER_KILLS, (float)nAddQuantity);
#ifndef K_AUTO_ACHIEVE_FROM_STATS
			if (nNewVal >= 100)
				__Achievements().UnlockAchievement(ACH_BUCKSHOT_SEASON);
#endif
		}
		break;
		case K_MEMID_GAMESTATS_KILLS_SHIELD:
		{
			__Achievements().IncStat(EGameStats::N_STAT_SHIELD_KILLS, (float)nAddQuantity);
#ifndef K_AUTO_ACHIEVE_FROM_STATS
			if (nNewVal >= 100)
				__Achievements().UnlockAchievement(ACH_LADY_JUSTICE);
#endif
		}
		break;
		case K_MEMID_GAMESTATS_KILLS_RECON:
		{
			__Achievements().IncStat(EGameStats::N_STAT_RECON_KILLS, (float)nAddQuantity);
#ifndef K_AUTO_ACHIEVE_FROM_STATS
			if (nNewVal >= 100)
				__Achievements().UnlockAchievement(ACH_I_SEE_THINGS);
#endif
		}
		break;
		case K_MEMID_GAMESTATS_KILLS_FBI:
		{
			__Achievements().IncStat(EGameStats::N_STAT_FBI_KILLS, (float)nAddQuantity);
#ifndef K_AUTO_ACHIEVE_FROM_STATS
			if (nNewVal >= 100)
				__Achievements().UnlockAchievement(ACH_FEDERAL_AUTHORITY);
#endif
		}
		break;
		case K_MEMID_GAMESTATS_KILLS_OFFDUTY:
		{
			__Achievements().IncStat(EGameStats::N_STAT_OFFDUTY_KILLS, (float)nAddQuantity);
#ifndef K_AUTO_ACHIEVE_FROM_STATS
			if (nNewVal >= 100)
				__Achievements().UnlockAchievement(ACH_GRUMPY_MCANGRY);
#endif
		}
		break;

		case K_MEMID_GAMESTATS_ENEMIES_SET_ON_FIRE:
		{
			__Achievements().IncStat(EGameStats::N_STAT_FLAMED_ENEMIES, (float)nAddQuantity);
#ifndef K_AUTO_ACHIEVE_FROM_STATS
			if(nNewVal >= 20)
				__Achievements().UnlockAchievement(ACH_CLEANSE_BY_FIRE);
#endif
		}
		break;
		case K_MEMID_GAMESTATS_HOSTAGES_SAVED:
		{
#ifndef K_AUTO_ACHIEVE_FROM_STATS
			if (nNewVal >= 1000)
			{
				__Achievements().UnlockAchievement(ACH_BUILDING_COMMUNITY);
			}
			else if (nNewVal >= 50)
			{
				__Achievements().UnlockAchievement(ACH_CAVALRY_IS_HERE);
			}
#endif
			//save stat
			__Achievements().IncStat(EGameStats::N_STAT_SAVED_HOSTAGES, (float)nAddQuantity);
		}
		break;
		case K_MEMID_GAMESTATS_COOP_GAMES_WON:
		{
			if (nNewVal >= 1)
			{
				__Achievements().UnlockAchievement(ACH_TWO_IS_A_PARTY);
			}
		}
		break;
		case K_MEMID_GAMESTATS_SA_SNIPER_FRAGS:
		{
#ifndef K_AUTO_ACHIEVE_FROM_STATS
			if (nNewVal >= 1000)
			{
				__Achievements().UnlockAchievement(ACH_SLACKER);
			}
			else if (nNewVal >= 100)
			{
				__Achievements().UnlockAchievement(ACH_EFFICIENT);
			}
#endif
			__Achievements().IncStat(EGameStats::N_STAT_SNIPED_ENEMIES, (float)nAddQuantity);
		}
		break;
		case K_MEMID_GAMESTATS_ENEMIES_KILLED:
		{
#ifndef K_AUTO_ACHIEVE_FROM_STATS
			if (nNewVal >= 1000)
			{
				__Achievements().UnlockAchievement(ACH_NO_RESPITE);
			}
			else if (nNewVal >= 50)
			{
				__Achievements().UnlockAchievement(ACH_NO_MERCY);
			}
#endif
			//save stat
			__Achievements().IncStat(EGameStats::N_STAT_KILLED_ENEMIES, (float)nAddQuantity);
		}
		break;
		case K_MEMID_GAMESTATS_DOORS_BREACHED:
		{
#ifndef K_AUTO_ACHIEVE_FROM_STATS
			if (nNewVal >= 1000)
			{
				__Achievements().UnlockAchievement(ACH_DOOR_HATER);
			}
			else if (nNewVal >= 100)
			{
				__Achievements().UnlockAchievement(ACH_DOOR_KICKER);
			}
#endif
			__Achievements().IncStat(EGameStats::N_STAT_DOORS_KICKED, (float)nAddQuantity);
		}
		break;
		case K_MEMID_GAMESTATS_DOORS_EXPLODED:
		{
#ifndef K_AUTO_ACHIEVE_FROM_STATS
			if (nNewVal >= 100)
			{
				__Achievements().UnlockAchievement(ACH_ATTENTION_GETTER);
			}
#endif
			__Achievements().IncStat(EGameStats::N_STAT_DOORS_EXPLODED, (float)nAddQuantity);
		}
		break;
		case K_MEMID_GAMESTATS_BOMBS_DISARMED:
		{
#ifndef K_AUTO_ACHIEVE_FROM_STATS
			if (nNewVal >= 30)
			{
				__Achievements().UnlockAchievement(ACH_EOD);
			}
#endif
			__Achievements().IncStat(EGameStats::N_STAT_BOMBS_DISARMED, (float)nAddQuantity);
		}
		break;
		case K_MEMID_GAMESTATS_ENEMIES_STUNNED:
		{
#ifndef K_AUTO_ACHIEVE_FROM_STATS
			if (nNewVal >= 500)
			{
				__Achievements().UnlockAchievement(ACH_I_LIKE_THEM_NUMB);
			}
#endif
			__Achievements().IncStat(EGameStats::N_STAT_STUNNED_ENEMIES, (float)nAddQuantity);
		}
		break;
		case K_MEMID_GAMESTATS_POLICE_SAVED:
		{
#ifndef K_AUTO_ACHIEVE_FROM_STATS
			if (nNewVal >= 50)
			{
				__Achievements().UnlockAchievement(ACH_THE_THIN_BLUE_LINE);
			}
#endif
			__Achievements().IncStat(EGameStats::N_STAT_SAVED_POLICEMEN, (float)nAddQuantity);
		}
		break;
		case K_MEMID_GAMESTATS_HOSTAGES_KILLED:
		{
			if (nNewVal >= 50)
			{
				__Achievements().UnlockAchievement(ACH_BAD_EYESIGHT);
			}
		}
		break;
		case K_MEMID_GAMESTATS_RATS_KILLED:
		{
			if (nNewVal >= 50)
			{
				__Achievements().UnlockAchievement(ACH_PEST_CONTROL);
			}
		}
		break;
	}
}

void App_SetGamestat(int K_MEMID_GAMESTATS_var, int nValue)
{
	assert((K_MEMID_GAMESTATS_var >= K_MEMID_GAMESTATS_START) && (K_MEMID_GAMESTATS_var <= K_MEMID_GAMESTATS_END));
	g_userData[K_MEMID_GAMESTATS_var] = nValue;
}

void App_ResetKeybindings(int nKeyboardIdx)
{
	if (nKeyboardIdx == 0)
	{
		g_userData[K_MEMID_KEY1_LEFT] = SDL_SCANCODE_LEFT;
		g_userData[K_MEMID_KEY1_RIGHT] = SDL_SCANCODE_RIGHT;
		g_userData[K_MEMID_KEY1_UP] = SDL_SCANCODE_UP;
		g_userData[K_MEMID_KEY1_DOWN] = SDL_SCANCODE_DOWN;
		g_userData[K_MEMID_KEY1_JUMP] = SDL_SCANCODE_SPACE;
		g_userData[K_MEMID_KEY1_FIRE1] = SDL_SCANCODE_S;
		g_userData[K_MEMID_KEY1_FIRE2] = SDL_SCANCODE_LALT;
		g_userData[K_MEMID_KEY1_RELOAD] = SDL_SCANCODE_R;
		g_userData[K_MEMID_KEY1_USE_GEAR] = SDL_SCANCODE_E;
		g_userData[K_MEMID_KEY1_MELEE] = SDL_SCANCODE_F;
		g_userData[K_MEMID_KEY1_STRATEGIC_MENU] = SDL_SCANCODE_LSHIFT;
	}
	else
	{
		//g_userData[K_MEMID_KEY2_LEFT] = SDL_SCANCODE_A;
		//g_userData[K_MEMID_KEY2_RIGHT] = SDL_SCANCODE_D;
		//g_userData[K_MEMID_KEY2_UP] = SDL_SCANCODE_W;
		//g_userData[K_MEMID_KEY2_DOWN] = SDL_SCANCODE_S;
		//g_userData[K_MEMID_KEY2_JUMP] = SDL_SCANCODE_O;
		//g_userData[K_MEMID_KEY2_FIRE1] = SDL_SCANCODE_P;
		//g_userData[K_MEMID_KEY2_FIRE2] = SDL_SCANCODE_F;
		//g_userData[K_MEMID_KEY2_RELOAD] = SDL_SCANCODE_R;
		//g_userData[K_MEMID_KEY2_USE_GEAR] = SDL_SCANCODE_G;
		//g_userData[K_MEMID_KEY2_CHANGE_WEAPON] = SDL_SCANCODE_E;
		//g_userData[K_MEMID_KEY2_STRATEGIC_MENU] = SDL_SCANCODE_I;

		g_userData[K_MEMID_KEY2_LEFT] = -1;
		g_userData[K_MEMID_KEY2_RIGHT] = -1;
		g_userData[K_MEMID_KEY2_UP] = -1;
		g_userData[K_MEMID_KEY2_DOWN] = -1;
		g_userData[K_MEMID_KEY2_JUMP] = -1;
		g_userData[K_MEMID_KEY2_FIRE1] = -1;
		g_userData[K_MEMID_KEY2_FIRE2] = -1;
		g_userData[K_MEMID_KEY2_RELOAD] = -1;
		g_userData[K_MEMID_KEY2_USE_GEAR] = -1;
		g_userData[K_MEMID_KEY2_MELEE] = -1;
		g_userData[K_MEMID_KEY2_STRATEGIC_MENU] = -1;
	}

}


eMissionType App_GetMissionType(WCHAR * strLevelPath)
{
	eMissionType eRetType = K_GAME_LSTYPE_NOTSET;

	FILE *fl = NULL;
	int err = OS_wfopen_s(&fl, strLevelPath, L"rb");
	if (fl == NULL || err != 0)
	{
		return K_GAME_LSTYPE_NOTSET;
	}
	else
	{
		//read int array
		UINT32 arrInts[10];
		OS_fread(arrInts, sizeof(UINT32), 10, fl);
		if (arrInts[0] < 1014)
		{
			return K_GAME_LSTYPE_NOTSET;
		}
		//mission type, file version 1014 and up
		eRetType = (eMissionType)OS_freadByte(fl);

		//close file
		OS_fclose(fl);
	}
	return eRetType;
}

void App_ParseAllLevelsForData(bool bResetAllStats)
{
	/*
	LOG(L"--> Parsing all levels for type...");
	//parse all levels and see what kind they are
	WCHAR xmlPath[MAX_PATH];
	FileManager::GetMediaPath(L"media/levels/missions/missions.xml", xmlPath);
	//load level
	for (int kk = 0; kk < UTGetChaptersList().GetTotalLevelsCnt(); kk++)
	{
		//reset stats too if wanted
		if (bResetAllStats)
		{
			g_levelStats[kk].Reset();
		}
		//--- read type of level ---
		int nChapterNumber = kk / K_GAME_LEVELS_PER_CHAPTER;
		int nLevelNumber = kk % K_GAME_LEVELS_PER_CHAPTER;
		WCHAR strLevelPath[MAX_PATH] = { 0 };
		bool bLevelFound = UTGetChaptersList().GetMissionFilename(nChapterNumber, nLevelNumber, strLevelPath, MAX_PATH);
		if (!bLevelFound)
		{
			g_levelStats[kk].nLevelType = K_GAME_LSTYPE_NOTSET;
		}
		else
		{
			g_levelStats[kk].nLevelType = App_GetMissionType(strLevelPath);
		}
	}
	LOG(L"... Parsing finished on %d levels.", UTGetChaptersList().GetTotalLevelsCnt());
	*/
}

void App_ResetUserData()
{
	//--- reset data ---
	for (int kk = 0; kk < K_MEMID_SLOTS_COUNT; kk++)
	{
		g_userData[kk] = 0;
	}

	//UID user
	g_userData[K_MEMID_USER_UID] = GenerateUID();
	//--- keys 1 ---
	App_ResetKeybindings(0);
	//--- keys 2 ---
	App_ResetKeybindings(1);
	//change actual triggers
	CController* keybd1 = __Controllers().GetControllerByInstanceID(K_CM_IID_KBM1);
	CController* keybd2 = nullptr;// UTGetCtrlrMgr().GetControllerByInstanceID(K_CM_DEFAULT_KEYBOARD2_INSTANCE_ID);
	App_SetSDLTriggersFromUserData(keybd1, keybd2);

	//parse all levels and see what kind they are
	App_ParseAllLevelsForData(true);

	//--- weapons shop ---
	UTGetShop().Reset();

	//update statistics
	App_UpdateLevelStats();
	// expected to be -1 in the chapter selection screen
	g_userData[K_MEMID_MOD_DWNLVL_SELECTED] = -1;

	LOG(L"Game::User data was reset/created!");
}

void App_SaveUserData()
{
	//--- version data[file version, game version, ... empty space for future data] ---
	int g_versionData[10] = { _VERSION_DATAFILE_, _VERSION_INT_, 0, 0, 0, 0, 0, 0, 0, 0 }; //save some empty slots for the version

	WCHAR sPath[MAX_PATH];
	StringCchPrintf(sPath, MAX_PATH, L"%suserdata.bin", UTApp().g_wszUserDataDir);
	FILE *fl = NULL;
	int err = OS_wfopen_s(&fl, sPath, L"wb");
	if (fl == NULL || err != 0)
	{
		ErrorBox(K_ERR_ONSCREEN, L"[WARNING] Couldn't save player data! Your progress could be lost!\nMake sure you have the rights to save to this folder and that your Antivirus isn't blocking the access:\n%s", sPath);
		return;
	}
	//--- version ---
	OS_fwrite(g_versionData, sizeof(int), 10, fl);
	//--- persistent memory ---
	OS_fwrite(g_userData, sizeof(int), K_MEMID_SLOTS_COUNT, fl);
	//--- level stats ---
	OS_fwrite(g_levelStats, sizeof(CLevelStats), ARRAY_SIZE(g_levelStats), fl);
	//--- weapon shop ---
	UTGetShop().SaveOwnedItems(fl);

	OS_fclose(fl);
	//update statistics
	App_UpdateLevelStats();
}

void App_LoadUserData()
{
	bool bOfferResetData = false;
	int arrVersionData[10];

	WCHAR sPath[MAX_PATH];
	StringCchPrintf(sPath, MAX_PATH, L"%suserdata.bin", UTApp().g_wszUserDataDir);
	FILE *fl = NULL;
	int err = OS_wfopen_s(&fl, sPath, L"rb");
	if (fl == NULL || err != 0)
	{
		App_ResetUserData();
		App_SaveUserData();
		return;
	}
	//--- version ---
	OS_fread(arrVersionData, sizeof(int), 10, fl);
	
	///--- CURRENT VERSION ---
	//--- persistent memory ---
	OS_fread(g_userData, sizeof(int), K_MEMID_SLOTS_COUNT, fl);
	//--- level stats ---
	OS_fread(g_levelStats, sizeof(CLevelStats), ARRAY_SIZE(g_levelStats), fl);
	
	//--- weapon shop ---
	UTGetShop().LoadOwnedItems(fl);

	OS_fclose(fl);
	//offer to reset user data?
	if (bOfferResetData)
		g_userData[K_MEMID_OFFER_RESET_USER_DATA] = 1;

	//UID user - make sure it is set
	if(g_userData[K_MEMID_USER_UID] == 0)
		g_userData[K_MEMID_USER_UID] = GenerateUID();

	//reset mod selection
	g_userData[K_MEMID_MOD_DWNLVL_SELECTED] = -1;

	//make sure we didn't spend more points than we had
	bool bResetUpgrades = false;
	//check negative bars
	for (int kk = K_MEMID_UPGRADE_BAR_POINTS_START; kk <= K_MEMID_UPGRADE_BAR_POINTS_END; kk++)
	{
		if (g_userData[kk] < 0)
		{
			LOG(L"[WARNING] Negative XP points found! Reset all upgrade bars!");
			bResetUpgrades = true;
			break;
		}
	}
	//check for lost points
	if (!bResetUpgrades)
	{
		int nBarsPoints = 0;
		int nTotalPts = 0;
		int nTotalNormal = 0;

		for (int kk = K_MEMID_UPGRADE_BAR_POINTS_START; kk <= K_MEMID_UPGRADE_BAR_POINTS_END; kk++)
			nBarsPoints += g_userData[kk];
		for (int kk = 0; kk < K_PSS_CLASSES_COUNT; kk++)
		{
			nTotalPts += App_GetAvailableXPPoints((EPSSPlayerClass)kk);
			nTotalNormal += K_GAME_XPPOINTS_PER_XPLEVEL * App_GetXPLevel(g_userData[K_MEMID_TOTALXP_PER_CLASS_START + kk]);
		}
		if (nTotalPts + nBarsPoints != nTotalNormal)
		{
			LOG(L"[WARNING] XP spending issue found! Reset all upgrade bars!");
			bResetUpgrades = true;
		}
	}

	if (bResetUpgrades)
	{
		for (int kk = K_MEMID_UPGRADE_BAR_POINTS_START; kk <= K_MEMID_UPGRADE_BAR_POINTS_END; kk++)
			g_userData[kk] = 0;
		for (int kk = K_MEMID_POINTS_SPENT_PER_CLASS_START; kk <= K_MEMID_POINTS_SPENT_PER_CLASS_END; kk++)
			g_userData[kk] = 0;
	}


#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	//g_userData[K_MEMID_TOTALXP_PER_CLASS_START + 0] = 27000;
	//g_userData[K_MEMID_TOTALXP_PER_CLASS_START + 1] = 27000;
	//g_userData[K_MEMID_TOTALXP_PER_CLASS_START + 2] = 27000;
	//g_userData[K_MEMID_TOTALXP_PER_CLASS_START + 3] = 27000;
	//g_userData[K_MEMID_TOTALXP_PER_CLASS_START + 4] = 27000;
	//g_userData[K_MEMID_TOTALXP_PER_CLASS_START + 5] = 27000;

	App_SaveUserData();
#endif

	//update statistics
	App_UpdateLevelStats();
}

void App_ReloadContentChanges()
{
	//reload strings too
	App_LocaLoadStrings();

	//load modded chapters again after modding
	WCHAR wcsPath[MAX_PATH];
	FileManager::GetMediaPath(L"media/levels/missions/missions.xml", wcsPath);
	UTGetChaptersList().LoadChapters(wcsPath);
	//loads shop items - must be loaded before loading user data (which tells us what items are unlocked)
	UTGetShop().LoadItemsAndPrices();
	//load user data so it unlocks weapons
	App_LoadUserData();
	//load weapons and items
	g_playerSelScr.LoadItems();
}

void App_SetSDLTriggersFromUserData(CController* ctrlrkeys1, CController* ctrlrkeys2)
{
	CHAR strKeys[MAX_PATH];
	WCHAR wcsKeys[MAX_PATH] = { 0 };
	//--- KEYBOARD 1 ---
	/*
	if (ctrlrkeys1 != null)
	{
		ctrlrkeys1->ClearTriggers(); //clear default mapping
		for (int kk = K_CM_COMMAND_LEFT; kk <= K_CM_COMMAND_STRATEGIC_MENU; kk++)
		{
			if (g_userData[K_MEMID_KEY1_LEFT + kk] >= 0)
				ctrlrkeys1->AddTrigger(K_CM_BUTTON, (EControllerCommand)kk, g_userData[K_MEMID_KEY1_LEFT + kk]);
		}

		ctrlrkeys1->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_SELECT, SDL_SCANCODE_RETURN);
		//ctrlrkeys1->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_SELECT, SDL_SCANCODE_SPACE);
		ctrlrkeys1->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_BACK, SDL_SCANCODE_ESCAPE);

		//set keyboard keys string
		memset(strKeys, 0, MAX_PATH * sizeof(CHAR));
		for (int ll = K_CM_COMMAND_LEFT; ll <= K_CM_COMMAND_STRATEGIC_MENU; ll++)
		{
			SDL_Scancode commandscan = (SDL_Scancode)ctrlrkeys1->GetKeyMappingForCommand((EControllerCommand)ll);
			if (commandscan >= 0)
			{
				StringCchCatA(strKeys, MAX_PATH, SDL_GetScancodeName(commandscan));
				StringCchCatA(strKeys, MAX_PATH, "\n");
			}
			else
			{
				StringCchCatA(strKeys, MAX_PATH, " \n");
			}
		}
		//add a space at the end
		StringCchCatA(strKeys, MAX_PATH, " ");
		mbstowcs(wcsKeys, strKeys, MAX_PATH);
		UTLang().SetString(STR_KEYS1_VAL, wcsKeys);
	}
	//--- KEYBOARD 2 ---
	if (ctrlrkeys2 != null)
	{
		ctrlrkeys2->ClearTriggers(); //clear default mapping
		for (int kk = K_CM_COMMAND_LEFT; kk <= K_CM_COMMAND_STRATEGIC_MENU; kk++)
		{
			if (g_userData[K_MEMID_KEY2_LEFT + kk] >= 0)
				ctrlrkeys2->AddTrigger(K_CM_BUTTON, (EControllerCommand)kk, g_userData[K_MEMID_KEY2_LEFT + kk]);
		}

		ctrlrkeys2->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_SELECT, SDL_SCANCODE_RETURN);
		//ctrlrkeys2->AddTrigger(K_CM_BUTTYPE_BUTTON, K_CM_COMMAND_SELECT, SDL_SCANCODE_SPACE);
		ctrlrkeys2->AddTrigger(K_CM_BUTTON, K_CM_COMMAND_BACK, SDL_SCANCODE_ESCAPE);

		//set keyboard keys string
		memset(strKeys, 0, MAX_PATH * sizeof(CHAR));
		for (int ll = K_CM_COMMAND_LEFT; ll <= K_CM_COMMAND_STRATEGIC_MENU; ll++)
		{
			SDL_Scancode commandscan = (SDL_Scancode)ctrlrkeys2->GetKeyMappingForCommand((EControllerCommand)ll);
			if (commandscan >= 0)
			{
				StringCchCatA(strKeys, MAX_PATH, SDL_GetScancodeName(commandscan));
				StringCchCatA(strKeys, MAX_PATH, "\n");
			}
			else
			{
				StringCchCatA(strKeys, MAX_PATH, " \n");
			}
		}
		//add a space at the end
		StringCchCatA(strKeys, MAX_PATH, " ");
		mbstowcs(wcsKeys, strKeys, MAX_PATH);
		UTLang().SetString(STR_KEYS2_VAL, wcsKeys);
	}
	*/
}

///--- WINDOW CLIP/CENTER functions ---

void App_CenterRectInRect(RECT *rectSrc, RECT *rectDest)
{
	if ((rectSrc == null) || (rectDest == null))
		return;

	SizeWH szSrc(rectSrc->right - rectSrc->left, rectSrc->bottom - rectSrc->top);
	rectSrc->left = rectDest->left + ((rectDest->right - rectDest->left) - szSrc.w) / 2;
	rectSrc->top = rectDest->top + ((rectDest->bottom - rectDest->top) - szSrc.h) / 2;
	rectSrc->right = rectSrc->left + szSrc.w;
	rectSrc->bottom = rectSrc->top + szSrc.h;
}

void App_CenterWindowOnMainDisplay(HWND wndHwnd)
{
	RECT wndrect, wndrect_new;
	GetWindowRect(wndHwnd, &wndrect);
	SizeWHi szWnd(wndrect.right - wndrect.left, wndrect.bottom - wndrect.top);
	Vec2i ptWndPos((UTApp().g_szDesktopSize.w - szWnd.w) / 2, (UTApp().g_szDesktopSize.h - szWnd.h) / 2);
	SetRect(&wndrect_new, ptWndPos.x, ptWndPos.y, ptWndPos.x + szWnd.w, ptWndPos.y + szWnd.h);
	if ((wndrect_new.left != wndrect.left) || (wndrect_new.top != wndrect.top) || (wndrect_new.right != wndrect.right) || (wndrect_new.bottom != wndrect.bottom))
	{
		SetWindowPos(wndHwnd, 0, ptWndPos.x, ptWndPos.y, szWnd.w, szWnd.h, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOSIZE);
	}
}

static bool bBorderlessFullscreenOn = false;
void App_ToggleBorderlessFullscreen(HWND wndHwnd)
{
	if (GetWindowLongPtr(wndHwnd, GWL_STYLE) & WS_POPUP)
	{
		DWORD dwWindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX/* | WS_MAXIMIZEBOX*/;
		SetWindowLongPtr(wndHwnd, GWL_STYLE, WS_VISIBLE | dwWindowStyle | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_SHOWWINDOW | SWP_NOSIZE);
		
		RECT wndrect;
		SetRect(&wndrect, 0, 0, UTApp().m_Settings.nWindowW, UTApp().m_Settings.nWindowH);
		AdjustWindowRect(&wndrect, dwWindowStyle, FALSE);
		
		SetWindowPos(wndHwnd, NULL, 0, 0, wndrect.right - wndrect.left, wndrect.bottom - wndrect.top, SWP_FRAMECHANGED);
		App_CenterWindowOnMainDisplay(wndHwnd);
		
		bBorderlessFullscreenOn = false;
	}
	else
	{
		//show full-screen
		int w = GetSystemMetrics(SM_CXSCREEN);
		int h = GetSystemMetrics(SM_CYSCREEN);
		SetWindowLongPtr(wndHwnd, GWL_STYLE, WS_VISIBLE | WS_POPUP);
		SetWindowPos(wndHwnd, HWND_TOP, 0, 0, w, h, SWP_FRAMECHANGED);

		bBorderlessFullscreenOn = true;
	}

	//save final fullscreen status
	UTApp().m_Settings.bFullscreen = (bBorderlessFullscreenOn || !DXUTIsWindowed());
	//close GFX options window if open
	CCtrlLayer* layer = __GUI().GetLayerByName("LAYER_ID_GFX_OPTIONS");
	if (layer != null)
	{
		__GUI().RemoveLayer("LAYER_ID_GFX_OPTIONS");
	}
}

bool App_IsBorderlessFullscreen()
{
	return bBorderlessFullscreenOn;
}

bool App_TutorialWindowShow(int nTutID)
{
	if (g_userData[nTutID] != 0)
			return false;

	if (UTApp().IsGameNetworked())
		return false;
	if (__GUI().bIsBlocking)
		return false;

	SND_PLAY(SNDIDX_DENIED);

	switch (nTutID)
	{
		case K_MEMID_TUT_INTERFACE_IGM:
		{
			__GUI().ShowLayerOnce("LAYER_ID_TUT_IGM");
		}
		break;
		case K_MEMID_TUT_INTERFACE_STRATEGIC:
		{
		}
		break;
		case K_MEMID_TUT_VINFINITE_MODE:
		{
			CCtrlLayer* lay = __GUI().ShowLayerOnce("LAYER_ID_TUT_PIC");
			if (lay != null)
			{
				CControl* ctrl = lay->GetControlByName("WINDOW");
				if (ctrl)
					ctrl->paramsDict.SetVarINT32(L"stringID", STR_VINFINITE_MODE);
				ctrl = lay->GetControlByName("TEXT_LABEL");
				if (ctrl)
					ctrl->paramsDict.SetVarINT32(L"stringID", STR_TUTORIAL_VINFINITE_TEXT);
				ctrl = lay->GetControlByName("ANIMATION");
				if (ctrl)
					ctrl->paramsDict.SetVarINT32(L"setFrame", 3);
			}
		}
		break;
		case K_MEMID_TUT_ARREST_MODE:
		{
			CCtrlLayer* lay = __GUI().ShowLayerOnce("LAYER_ID_TUT_PIC");
			if (lay != null)
			{
				CControl* ctrl = lay->GetControlByName("WINDOW");
				if(ctrl)
					ctrl->paramsDict.SetVarINT32(L"stringID", STR_MISSION_TYPE4);
				ctrl = lay->GetControlByName("TEXT_LABEL");
				if (ctrl)
					ctrl->paramsDict.SetVarINT32(L"stringID", STR_TUTORIAL_ARREST_TEXT);
				ctrl = lay->GetControlByName("ANIMATION");
				if (ctrl)
					ctrl->paramsDict.SetVarINT32(L"setFrame", 0);
			}
		}
		break;
		case K_MEMID_TUT_BOMB_MODE:
		{
			CCtrlLayer* lay = __GUI().ShowLayerOnce("LAYER_ID_TUT_PIC");
			if (lay != null)
			{
				CControl* ctrl = lay->GetControlByName("WINDOW");
				if (ctrl)
					ctrl->paramsDict.SetVarINT32(L"stringID", STR_MISSION_TYPE3);
				ctrl = lay->GetControlByName("TEXT_LABEL");
				if (ctrl)
					ctrl->paramsDict.SetVarINT32(L"stringID", STR_TUTORIAL_BOMB_TEXT);
				ctrl = lay->GetControlByName("ANIMATION");
				if (ctrl)
					ctrl->paramsDict.SetVarINT32(L"setFrame", 2);
			}
		}
		break;
		case K_MEMID_TUT_HOSTAGE_MODE:
		{
			CCtrlLayer* lay = __GUI().ShowLayerOnce("LAYER_ID_TUT_PIC");
			if (lay != null)
			{
				CControl* ctrl = lay->GetControlByName("WINDOW");
				if (ctrl)
					ctrl->paramsDict.SetVarINT32(L"stringID", STR_MISSION_TYPE2);
				ctrl = lay->GetControlByName("TEXT_LABEL");
				if (ctrl)
					ctrl->paramsDict.SetVarINT32(L"stringID", STR_TUTORIAL_HOSTAGE_TEXT);
				ctrl = lay->GetControlByName("ANIMATION");
				if (ctrl)
					ctrl->paramsDict.SetVarINT32(L"setFrame", 1);
			}
		}
		break;
	}

	g_userData[nTutID]++;
	return true;
}

//important files to check
static const WCHAR* arr_wcsImportantFilesCRC[] = {
	L"media/interfaces/interfaces.xml",
	L"media/scripts.xml",
	L"media/levels/data/weapons/weapons_data.xml",
	L"media/levels/data/actors_data.xml",
	L"media/levels/data/gear_screen.xml",
	L"media/levels/data/props.bsx",
	L"media/levels/data/actors.bsx",
	L"media/levels/missions/missions.xml"
};

UINT32 App_GetGameFilesCRC()
{
	UINT32 unRetCRC = 0;
	
	WCHAR strPath[MAX_PATH];
	///--- hash of important files ---
	for (int kk = 0; kk < ARRAY_SIZE(arr_wcsImportantFilesCRC); kk++)
	{
		StringCchPrintf(strPath, MAX_PATH, L"%s/%s", UTApp().g_wszExePath, arr_wcsImportantFilesCRC[kk]);
		UINT32 flcrc = GetFileHash(strPath);
		//add with overflow
		unRetCRC += flcrc;
	}

	///--- all playable missions ---
	for (int nchap = 0; nchap < UTGetChaptersList().GetChaptersCnt(); nchap++)
	{
		for (int nlvl = 0; nlvl < UTGetChaptersList().m_arrChapters[nchap]->nLevelsCnt; nlvl++)
		{
			bool bLevelFound = UTGetChaptersList().GetMissionFilename(nchap, nlvl, strPath, MAX_PATH);
			if (!bLevelFound)
				continue;

			UINT32 flcrc = GetFileHash(strPath);
			//add with overflow
			unRetCRC += flcrc;
		}
	}
	//return total crc
	return unRetCRC;
}

UINT32 App_GetActiveModsCRC()
{
#ifdef ENABLE_STEAM_WORKSHOP
	UINT32 unRetCRC = 0;

	WCHAR strPath[MAX_PATH];
	///--- hash of important files ---
	for (int kk = 0; kk < ARRAY_SIZE(arr_wcsImportantFilesCRC); kk++)
	{
		FileManager::GetMediaPath(arr_wcsImportantFilesCRC[kk], strPath);
		UINT32 flcrc = GetFileHash(strPath);
		//add with overflow
		unRetCRC += flcrc;
	}

	///--- all playable missions ---
	for (int nchap = 0; nchap < UTGetChaptersList().GetChaptersCnt(); nchap++)
	{
		for (int nlvl = 0; nlvl < UTGetChaptersList().m_arrChapters[nchap]->nLevelsCnt; nlvl++)
		{
			bool bLevelFound = UTGetChaptersList().GetMissionFilename(nchap, nlvl, strPath, MAX_PATH);
			if (!bLevelFound)
				continue;

			UINT32 flcrc = GetFileHash(strPath);
			//add with overflow
			unRetCRC += flcrc;
		}
	}

	LOG(L"--> Active mods CRC[%08x] <--", unRetCRC);
	//return total crc
	return unRetCRC;
#else
	return App_GetGameFilesCRC();
#endif
}

//hardcoded upgrade levels
int g_arrLevelsXP[K_GAME_MAX_UPGRADE_LEVELS] = {
	200, 600, 1200, 2000,
	3000, 4200, 5600, 7200,
	9000, 11000, 13200, 15600,
	18200, 21000, 24000, 27200 
};

UINT32 App_GetMaxXP(int nUpgradeLevel)
{
	if (nUpgradeLevel < 0)
		return 0;

	int nLvl = LIMIT(nUpgradeLevel, 0, K_GAME_MAX_UPGRADE_LEVELS - 1);
	return g_arrLevelsXP[nLvl];
}

int App_GetXPLevel(UINT32 dwXPpoints)
{
	for (int kk = 0; kk < K_GAME_MAX_UPGRADE_LEVELS; kk++)
	{
		if (dwXPpoints < g_arrLevelsXP[kk])
			return kk;
	}

	return K_GAME_MAX_UPGRADE_LEVELS;
}

void App_PaintControllerKey(CController* pCtrlr, EControllerCommand eCommand, D3DXVECTOR2 vPos, bool bPressed, int nAlign, DWORD dwColor)
{
	if (pCtrlr == null)
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING] App_PaintControllerKey: Illegal controller!");
		return;
	}

	WCHAR strKey[MAX_PATH];
	int nKeyIcon = -1;
	CStringDesc sdKeyName;
	//save command names
	if (pCtrlr->eType == K_CM_CT_NET_FRAMELOCK)
	{
		nKeyIcon = -1;
		sdKeyName.Reset();
		return;
	}
	else if (pCtrlr->eType == K_CM_CT_KBM_SDL)
	{
		nKeyIcon = -1;//reset icons on DON'T SHOW

		SDL_Scancode commandscan = (SDL_Scancode)pCtrlr->GetKeyMappingForCommand(eCommand);
		mbstowcs_s(null, strKey, __Controllers().GetSDLScancodeName(commandscan), MAX_PATH);
		__Texts().SetStringDesc(&sdKeyName, strKey);
	}
	else if (pCtrlr->eType == K_CM_CT_JOYSTICK_SDL)
	{
		CControllerTrigger* trigger = pCtrlr->GetTriggerForCommand(eCommand);

		if (trigger->eType == K_CM_BUTTON)
		{
			//set icon from CTRLR_XBOX_UP/DOWN from controls.bsx
			nKeyIcon = K_CI_ARR_BUTICONS_FRAMES[trigger->keyMapping];
			//save string name as a fallback - not used 
			/*
			CHAR sName[MAX_PATH];
			sprintf(sName, SDL_GameControllerGetStringForButton((SDL_GameControllerButton)trigger->keyMapping));
			for (int ll = 0; ll < (int)strlen(sName); ll++)
				sName[ll] = toupper(sName[ll]);

			mbstowcs_s(null, strKey, sName, MAX_PATH);
			UTLang().SetStringDesc(&sdKeyName, strKey);
			*/
		}
		else if (trigger->eType == K_CM_HALF_AXIS)
		{
			nKeyIcon = K_CI_ARR_AXISICONS_FRAMES[trigger->keyMapping];
			//save string name as a fallback - not used
			/*
			mbstowcs_s(null, strKey, SDL_GameControllerGetStringForAxis((SDL_GameControllerAxis)trigger->keyMapping), MAX_PATH);
			UTLang().SetStringDesc(&sdKeyName, strKey);
			*/
		}
	}
	///--- now paint ---
	D3DXVECTOR2 vBP = vPos;
	//key icons not set, defaults on key names
	if (nKeyIcon < 0)
	{
		int anmIdx = ANM_CONTROLS_SPR_BUT_SM_GREY2;
		GUIUtils::DrawButtonFromText(&__GUI().m_sprCol, anmIdx, bPressed, &sdKeyName, g_font5n2, vBP, dwColor, nAlign);
	}
	else //key icons set
	{
		int anmIdx = ANM_CONTROLS_SPR_CTRLR_XBOX_UP;
		if (bPressed)
			anmIdx = ANM_CONTROLS_SPR_CTRLR_XBOX_DOWN;

		int butw = __GUI().m_sprCol.GetAFrameBBox(anmIdx, nKeyIcon).w;
		int algnoffx = (-nAlign * butw) / 2;
		CSprite::paintFrame(&__GUI().m_sprCol, vBP.x + algnoffx, vBP.y, anmIdx, nKeyIcon, dwColor);
	}
}

void App_SetWorldTransform(PDEVICE pDevice, Matrix* matWorld)
{
	g_matWorld = *matWorld;
	pDevice->SetTransform(D3DTS_WORLD, matWorld);
}

UINT32 App_GetAvailableXPPoints(EPSSPlayerClass eClass)
{
	//give 2 points per upgrade level
	int nPts = K_GAME_XPPOINTS_PER_XPLEVEL * App_GetXPLevel(g_userData[K_MEMID_TOTALXP_PER_CLASS_START + (int)eClass]);
	//subtract spent points
	nPts -= g_userData[K_MEMID_POINTS_SPENT_PER_CLASS_START + (int)eClass];

	return nPts;
}


///----- LANGUAGES - LOCALIZATIONS ---

CLocaLanguage g_Language;						//current game language
CArray<CLocaLanguage> g_arrLangList;	//list of languages from lang.xml

OPRESULT App_LocaLoadLangList(CStringHash shSelectedLangAlias)
{
	//if requested language is empty just take the system setting
	if (shSelectedLangAlias.IsEmpty())
	{
		const char* strSysLang = App_LocaGetSystemLanguage();
		shSelectedLangAlias.Init(strSysLang);
	}

	WCHAR xmlpath[MAX_PATH];
	FileManager::GetMediaPath(L"media/texts/lang.xml", xmlpath);
	//parse language list

	pugi::xml_document doc;
	if (!doc.load_file(xmlpath))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"Unable to load languages XML:%s\n", xmlpath);
	}

	bool bLangSet = false;

	pugi::xml_node listnode = doc.root().child(L"LocaList");
	for (pugi::xml_node langnode = listnode.first_child(); langnode; langnode = langnode.next_sibling())
	{
		CLocaLanguage nlang;
		nlang.shLangName.Init(langnode.attribute(L"LangName").value());
		nlang.shLangAlias.Init(langnode.attribute(L"LangAlias").value());
		nlang.shFileName.Init(langnode.attribute(L"TextsFileName").value());
		nlang.bUseTTFonts = langnode.attribute(L"bUseTTFonts").as_bool();
		nlang.strMinAlphabet = langnode.attribute(L"MinimumAlphabet").value();
		if ((nlang.shLangName.IsEmpty()) || (nlang.shLangAlias.IsEmpty()) || (nlang.shFileName.IsEmpty()))
		{
			ErrorBox(K_ERR_WARNING, L"Error loading language list! name[%s] alias[%s] file[%s]", nlang.shLangName.text, nlang.shLangAlias.text, nlang.shFileName.text);
		}
		else
		{
			g_arrLangList.Add(nlang);
			//set selected language now
			if (shSelectedLangAlias == nlang.shLangAlias)
			{
				g_Language = nlang;
				bLangSet = true;
			}
		}
	}

	if (!bLangSet)
	{
		g_Language.shLangName.Init(L"English");
		g_Language.shLangAlias.Init(L"english");
		g_Language.shFileName.Init(L"strings.xml");

		ErrorBox(K_ERR_WARNING, L"The language specified in the options.xml file was not found! Defaulting to English!");
	}

	UTApp().m_Settings.shLanguageAlias = g_Language.shLangAlias;

	return K_OP_OK;
}

OPRESULT App_LocaLoadStrings()
{
	WCHAR wcsMediaName[MAX_PATH];
	StringCchPrintf(wcsMediaName, MAX_PATH, L"media/texts/%s", g_Language.shFileName.text);
	WCHAR xmlpath[MAX_PATH];
	FileManager::GetMediaPath(wcsMediaName, xmlpath);
	LOG(L"[LANG] Loading strings: %s", xmlpath);
	
	//load strings and ignore missing characters when using TTF fonts
	int nLoaded = __Texts().LoadFromXML(xmlpath, g_Language.shLangName.text, &g_Language.strMinAlphabet, g_Language.bUseTTFonts);
	if (nLoaded <= 0)
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"[ERROR] Failed loading strings XML! file:%s", xmlpath);
	}
	//set version number
	__Texts().SetString(STR_VERSION_NUMBER, L"v%d.%d.%d", _VERSION_MAJOR_, _VERSION_MINOR_, _VERSION_PATCH_);
	//set keyboard strings
	CController* keybd1 = __Controllers().GetControllerByInstanceID(K_CM_IID_KBM1);
	CController* keybd2 = nullptr;// UTGetCtrlrMgr().GetControllerByInstanceID(K_CM_DEFAULT_KEYBOARD2_INSTANCE_ID);
	App_SetSDLTriggersFromUserData(keybd1, keybd2);

	return K_OP_OK;
}

//generic TTF names (precomputed hash)
CStringHash shTTFID_SZ40(L"TTFID_SZ40"); //large font replacement
CStringHash shTTFID_SZ30(L"TTFID_SZ30"); //medium fonts
CStringHash shTTFID_SZ20(L"TTFID_SZ20"); //always loaded (needed for chat)

OPRESULT App_LocaLoadFonts(bool bUseTTFonts)
{
	//release all
	__TexFonts().Release();

	HRESULT hr = S_OK;

	//load TTF
	if (bUseTTFonts)
	{
		//Load all necessary TTF fonts
		if (FAILED(hr = UTGetTTFManager().LoadFont(shTTFID_SZ20.text, L"Noto Sans Med", L"media/fonts/NotoSans-Medium.ttf", 20)))
		{
			ErrorBox(K_ERR_WARNING, L"[WARNING] Couldn't load TrueType font [%s]!", shTTFID_SZ20.text);
		}
		/*
		if (FAILED(hr = UTGetTTFManager().LoadFont(shTTFID_SZ30.text, L"Noto Sans Med", L"media/fonts/NotoSans-Medium.ttf", 30)))
		{
			ErrorBox(K_ERR_WARNING, L"[WARNING] Couldn't load TrueType font [%s]!", shTTFID_SZ30.text);
		}
		if (FAILED(hr = UTGetTTFManager().LoadFont(shTTFID_SZ40.text, L"Noto Sans Med", L"media/fonts/NotoSans-Medium.ttf", 40)))
		{
			ErrorBox(K_ERR_WARNING, L"[WARNING] Couldn't load TrueType font [%s]!", shTTFID_SZ40.text);
		}
		*/

		//needed bitmap fonts (maybe all of them, not a mem problem)
		WCHAR xmlpath[MAX_PATH];
		FileManager::GetMediaPath(L"media/fonts/font_12_WOW.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_10_B1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_10_BS1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_9_B1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_8_B1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_8_BS1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_6_N1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_6_NS1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_6_NC1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_5_N1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_5_N2.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_5_NS2.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
	}
	else
	{
		//only 1 TTF font needed for cross lang chat
		if (FAILED(hr = UTGetTTFManager().LoadFont(shTTFID_SZ20.text, L"Noto Sans Med", L"media/fonts/NotoSans-Medium.ttf", 20)))
		{
			ErrorBox(K_ERR_WARNING, L"[WARNING] Couldn't load TrueType font [%s]!", shTTFID_SZ20.text);
		}

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
		//don't load larger TTF fonts
		if (FAILED(hr = UTGetTTFManager().LoadFont(shTTFID_SZ30.text, L"Noto Sans Med", L"media/fonts/NotoSans-Medium.ttf", 30)))
		{
			ErrorBox(K_ERR_WARNING, L"[WARNING] Couldn't load TrueType font [%s]!", shTTFID_SZ30.text);
		}
		if (FAILED(hr = UTGetTTFManager().LoadFont(shTTFID_SZ40.text, L"Noto Sans Med", L"media/fonts/NotoSans-Medium.ttf", 40)))
		{
			ErrorBox(K_ERR_WARNING, L"[WARNING] Couldn't load TrueType font [%s]!", shTTFID_SZ40.text);
		}
#endif

		//bitmap fonts
		WCHAR xmlpath[MAX_PATH];
		FileManager::GetMediaPath(L"media/fonts/font_12_WOW.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_10_B1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_10_BS1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_9_B1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_8_B1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_8_BS1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_6_N1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_6_NS1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_6_NC1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_5_N1.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_5_N2.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
		FileManager::GetMediaPath(L"media/fonts/font_5_NS2.bsx", xmlpath);
		__TexFonts().AddFontXML(xmlpath);
	}

	//sets direct pointers to ingame fonts
	g_font12wow = __TexFonts()["FONT_12_WOW"];
	g_font10b1 = __TexFonts()["FONT_10_B1"];
	g_font10bs1 = __TexFonts()["FONT_10_BS1"];
	g_font9b1 = __TexFonts()["FONT_9_B1"];
	g_font8b1 = __TexFonts()["FONT_8_B1"];
	g_font8bs1 = __TexFonts()["FONT_8_BS1"];
	g_font6n1 = __TexFonts()["FONT_6_N1"];
	g_font6ns1 = __TexFonts()["FONT_6_NS1"];
	g_font6nc1 = __TexFonts()["FONT_6_NC1"];
	g_font5n1 = __TexFonts()["FONT_5_N1"];
	g_font5n2 = __TexFonts()["FONT_5_N2"];
	g_font5ns2 = __TexFonts()["FONT_5_NS2"];

	//set font replacements if using TTF
	/*
	if (bUseTTFonts)
	{
		g_font12wow->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ40.textHash), &UTApp().g_cam480hScreen);
		g_font10b1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ40.textHash), &UTApp().g_cam480hScreen);
		g_font10bs1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ40.textHash), &UTApp().g_cam480hScreen);
		g_font9b1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ30.textHash), &UTApp().g_cam480hScreen);
		g_font8b1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ30.textHash), &UTApp().g_cam480hScreen);
		g_font8bs1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ30.textHash), &UTApp().g_cam480hScreen);
		g_font6n1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ20.textHash), &UTApp().g_cam480hScreen);
		g_font6ns1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ20.textHash), &UTApp().g_cam480hScreen);
		g_font6nc1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ20.textHash), &UTApp().g_cam480hScreen);
		g_font5n1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ20.textHash), &UTApp().g_cam480hScreen);
		g_font5n2->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ20.textHash), &UTApp().g_cam480hScreen);
		g_font5ns2->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ20.textHash), &UTApp().g_cam480hScreen);
	}
	else
	{
		g_font12wow->SetFontReplacementTTF(null);
		g_font10b1->SetFontReplacementTTF(null);
		g_font10bs1->SetFontReplacementTTF(null);
		g_font9b1->SetFontReplacementTTF(null);
		g_font8b1->SetFontReplacementTTF(null);
		g_font8bs1->SetFontReplacementTTF(null);
		g_font6n1->SetFontReplacementTTF(null);
		g_font6ns1->SetFontReplacementTTF(null);
		g_font6nc1->SetFontReplacementTTF(null);
		g_font5n1->SetFontReplacementTTF(null);
		g_font5n2->SetFontReplacementTTF(null);
		g_font5ns2->SetFontReplacementTTF(null);
	}
	*/

	return K_OP_OK;
}

OPRESULT App_LocaChangeLanguage(CStringHash shSelectedLangAlias)
{
	CLocaLanguage cllLangOld = g_Language;
	//find alias name
	bool bLangFound = false;
	for (int kk = 0; kk < g_arrLangList.GetSize(); kk++)
	{
		if (g_arrLangList[kk].shLangAlias == shSelectedLangAlias)
		{
			g_Language = g_arrLangList[kk];
			bLangFound = true;
		}
	}

	if (!bLangFound)
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"App_LocaChangeLanguage: Language alias not found [%s] !", shSelectedLangAlias.text );
	}

	if (cllLangOld.shLangAlias.textHash != g_Language.shLangAlias.textHash)
	{
		V_OP_RET(App_LocaLoadStrings());
		//now load the fonts
		if(cllLangOld.bUseTTFonts != g_Language.bUseTTFonts)
			App_LocaLoadFonts(g_Language.bUseTTFonts);

		//save settings 
		UTApp().m_Settings.shLanguageAlias = g_Language.shLangAlias;
		UTApp().SaveSettings();
	}

	LOG(L"[LANG] Language changed to [%s]", shSelectedLangAlias.text);
	return K_OP_OK;
}

CLocaLanguage App_LocaGetCurrentLanguage()
{
	return g_Language;
}


#include "isteamutils.h"
const char* App_LocaGetSystemLanguage()
{
#if defined(ENABLE_STEAM)
	//return SteamApps()->GetCurrentGameLanguage();
	return SteamUtils()->GetSteamUILanguage();
#else
	//this branch should return the OS language in string form, compatible with the steam languages list (see header)
	ErrorBox(K_ERR_WARNING, L"App_LocaGetSystemLanguage should read language from OS! Now it just returns english.");
	return "english";
#endif
}

/*
// 
//  ClipOrCenterRectToMonitor 
// 
//  The most common problem apps have when running on a 
//  multimonitor system is that they "clip" or "pin" windows 
//  based on the SM_CXSCREEN and SM_CYSCREEN system metrics. 
//  Because of app compatibility reasons these system metrics 
//  return the size of the primary monitor. 
// 
//  This shows how you use the multi-monitor functions 
//  to do the same thing. 
// 
void App_ClipOrCenterRectToMonitor(LPRECT prc, UINT flags)
{
HMONITOR hMonitor;
MONITORINFO mi;
RECT        rc;
int         w = prc->right - prc->left;
int         h = prc->bottom - prc->top;

// 
// get the nearest monitor to the passed rect. 
// 
hMonitor = MonitorFromRect(prc, MONITOR_DEFAULTTONEAREST);

// 
// get the work area or entire monitor rect. 
// 
mi.cbSize = sizeof(mi);
GetMonitorInfo(hMonitor, &mi);

if (flags & K_MONITOR_WORKAREA)
rc = mi.rcWork;
else
rc = mi.rcMonitor;

// 
// center or clip the passed rect to the monitor rect 
// 
if (flags & K_MONITOR_CENTER)
{
prc->left = rc.left + (rc.right - rc.left - w) / 2;
prc->top = rc.top + (rc.bottom - rc.top - h) / 2;
prc->right = prc->left + w;
prc->bottom = prc->top + h;
}
else
{
prc->left = max(rc.left, min(rc.right - w, prc->left));
prc->top = max(rc.top, min(rc.bottom - h, prc->top));
prc->right = prc->left + w;
prc->bottom = prc->top + h;
}
}

void App_ClipOrCenterWindowToMonitor(HWND hwnd)
{
RECT rc;
GetWindowRect(hwnd, &rc);
App_ClipOrCenterRectToMonitor(&rc, K_MONITOR_WORKAREA);
SetWindowPos(hwnd, NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}
*/

