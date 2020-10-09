#pragma once

///---------------------------------------------------
/// Level data and user data, settings
///---------------------------------------------------

///--- USER DATA ---
//--- PERSISTENT MEMORY INDICES ---
#define K_MEMID_SELECTED_CHAPTER 0
#define K_MEMID_SELECTED_LEVEL 1
#define K_MEMID_STARS_TOTAL 2
#define K_MEMID_STARS_SPENT 3
#define K_MEMID_MISSIONS_COMPLETED 4
//used to change between different tracks without random
#define K_MEMID_MUSIC_TRACK_COUNTER		5
//scores updated by updateLevelStats
#define K_MEMID_TOTAL_SCORE_SOLO		6
#define K_MEMID_TOTAL_SCORE_COOP		7
//custom downloaded level mod selected idx (or -1 if none selected)
#define K_MEMID_MOD_DWNLVL_SELECTED		10
//each secret item has a flag.
#define K_MEMID_SECRET_ITEMS_FLAGS		20
#define K_MEMID_OFFER_RESET_USER_DATA	21

//--- codurile SDL pentru taste (sincronizate ca ordine cu strings si cu enum K_CM_COMMAND_ ---
#define K_MEMID_KEYSALL_START			70
//show slots start
#define K_MEMID_KEYS1_FIRSTITEM			70
//keys keyboard1
#define K_MEMID_KEY1_LEFT				70
#define K_MEMID_KEY1_RIGHT				71
#define K_MEMID_KEY1_UP					72
#define K_MEMID_KEY1_DOWN				73
#define K_MEMID_KEY1_JUMP				74
#define K_MEMID_KEY1_FIRE1				75
#define K_MEMID_KEY1_FIRE2				76
#define K_MEMID_KEY1_RELOAD				77
#define K_MEMID_KEY1_USE_GEAR			78
#define K_MEMID_KEY1_MELEE				79
#define K_MEMID_KEY1_STRATEGIC_MENU		80

//shows slots start
#define K_MEMID_KEYS2_FIRSTITEM			90
//keys keyboard1
#define K_MEMID_KEY2_LEFT			90
#define K_MEMID_KEY2_RIGHT			91
#define K_MEMID_KEY2_UP				92
#define K_MEMID_KEY2_DOWN			93
#define K_MEMID_KEY2_JUMP			94
#define K_MEMID_KEY2_FIRE1			95
#define K_MEMID_KEY2_FIRE2			96
#define K_MEMID_KEY2_RELOAD			97
#define K_MEMID_KEY2_USE_GEAR		98
#define K_MEMID_KEY2_MELEE			99 
#define K_MEMID_KEY2_STRATEGIC_MENU	100 
//where all keys end
#define K_MEMID_KEYSALL_END			100

//#WEEKLY: temp data for weekly
#define K_MEMID_WEEKLY1_TASKS_FLAGS		101
#define K_MEMID_WEEKLY2_TASKS_FLAGS		102
#define K_MEMID_WEEKLY3_TASKS_FLAGS		103
#define K_MEMID_WEEKLY4_TASKS_FLAGS		104
#define K_MEMID_WEEKLY5_TASKS_FLAGS		105
#define K_MEMID_WEEKLY6_TASKS_FLAGS		106
#define K_MEMID_WEEKLY7_TASKS_FLAGS		107
#define K_MEMID_WEEKLY8_TASKS_FLAGS		108
#define K_MEMID_WEEKLY9_TASKS_FLAGS		109
#define K_MEMID_WEEKLY10_TASKS_FLAGS	110
#define K_MEMID_WEEKLY11_TASKS_FLAGS	111
#define K_MEMID_WEEKLY12_TASKS_FLAGS	112

//folosite pentru a numara cate usi s-au spart si pentru a da badges
#define K_MEMID_GAMESTATS_START					150

#define K_MEMID_GAMESTATS_DOORS_BREACHED		150
#define K_MEMID_GAMESTATS_DOORS_EXPLODED		151
#define K_MEMID_GAMESTATS_ENEMIES_KILLED		152
#define K_MEMID_GAMESTATS_ENEMIES_STUNNED		153
#define K_MEMID_GAMESTATS_ENEMIES_SET_ON_FIRE	154
#define K_MEMID_GAMESTATS_HOSTAGES_SAVED		155
#define K_MEMID_GAMESTATS_HOSTAGES_KILLED		156
#define K_MEMID_GAMESTATS_POLICE_SAVED			157
//sniper ability frags
#define K_MEMID_GAMESTATS_SA_SNIPER_FRAGS		158
#define K_MEMID_GAMESTATS_BOMBS_DISARMED		159
#define K_MEMID_GAMESTATS_SECRETS_COLLECTED		160
#define K_MEMID_GAMESTATS_COOP_GAMES_WON		161
#define K_MEMID_GAMESTATS_RATS_KILLED			162
//other used mostly for GOG Galaxy
#define K_MEMID_GAMESTATS_KILLS_ASSAULTER		163
#define K_MEMID_GAMESTATS_KILLS_BREACHER		164
#define K_MEMID_GAMESTATS_KILLS_SHIELD			165
#define K_MEMID_GAMESTATS_KILLS_RECON			166
#define K_MEMID_GAMESTATS_KILLS_FBI				167
#define K_MEMID_GAMESTATS_KILLS_OFFDUTY			168

#define K_MEMID_GAMESTATS_ARREST_TARGETS_ARRESTED		170

#define K_MEMID_GAMESTATS_END				199

///panel selections saved by class (200 - 299):
//panel 1 selected class:
#define K_MEMID_PANEL1_CLASS				200
//panel 1 saves: 5 selections per class, max 8 classes = 40 slots
#define K_MEMID_PANEL1_CLASSDATA_START		201
#define K_MEMID_PANEL1_CLASSDATA_END		249
//panel 2 selected class:
#define K_MEMID_PANEL2_CLASS				250
//panel 1 saves: 5 selections per class, max 8 classes = 40 slots
#define K_MEMID_PANEL2_CLASSDATA_START		251
#define K_MEMID_PANEL2_CLASSDATA_END		299

///classes upgrades data:
#define K_MEMIDALT_XP_STUFF_START				300
//all-time XP points per class (10 classes):
#define K_MEMID_TOTALXP_PER_CLASS_START			300
#define K_MEMID_TOTALXP_PER_CLASS_END			309

//saved upgrade points spent per bar (bar's nMemSlot tells you where to look: nMemSlot + POINTS_START)
#define K_MEMID_UPGRADE_BAR_POINTS_START		310
#define K_MEMID_UPGRADE_BAR_POINTS_END			342
//spent upgrade points per class (10 classes)
#define K_MEMID_POINTS_SPENT_PER_CLASS_START	343
#define K_MEMID_POINTS_SPENT_PER_CLASS_END		352
//where all upgrades stuff ends:
#define K_MEMIDALT_XP_STUFF_END					360

///tutorials
#define K_MEMID_TUTORIALS_START					380

#define K_MEMID_TUT_XP_TEAM_ADD					380
#define K_MEMID_TUT_ARREST_MODE					381
#define K_MEMID_TUT_BOMB_MODE					382
#define K_MEMID_TUT_HOSTAGE_MODE				383
#define K_MEMID_TUT_INTERFACE_STRATEGIC			384
#define K_MEMID_TUT_INTERFACE_IGM				385
#define K_MEMID_TUT_VINFINITE_MODE				386

#define K_MEMID_TUTORIALS_END					390

//uid user - set only once
#define K_MEMID_USER_UID 399
//--- slots total count ---
#define K_MEMID_SLOTS_COUNT 400

//--- persistent memory ---
extern int			g_userData[K_MEMID_SLOTS_COUNT];


//frame from animation that gets painted for each button (same size as SDL_GameControllerButton)
const int K_CI_ARR_BUTICONS_FRAMES[SDL_CONTROLLER_BUTTON_MAX] = {
	2,	/*SDL_CONTROLLER_BUTTON_A*/
	3,	/*SDL_CONTROLLER_BUTTON_B*/
	4,	/*SDL_CONTROLLER_BUTTON_X*/
	5,	/*SDL_CONTROLLER_BUTTON_Y*/
	0,	/*SDL_CONTROLLER_BUTTON_BACK*/
	-1,	/*SDL_CONTROLLER_BUTTON_GUIDE*/
	1,	/*SDL_CONTROLLER_BUTTON_START*/
	-1,	/*SDL_CONTROLLER_BUTTON_LEFTSTICK*/
	-1,	/*SDL_CONTROLLER_BUTTON_RIGHTSTICK*/
	6,	/*SDL_CONTROLLER_BUTTON_LEFTSHOULDER*/
	7,	/*SDL_CONTROLLER_BUTTON_RIGHTSHOULDER*/
	-1,	/*SDL_CONTROLLER_BUTTON_DPAD_UP*/
	-1,	/*SDL_CONTROLLER_BUTTON_DPAD_DOWN*/
	-1,	/*SDL_CONTROLLER_BUTTON_DPAD_LEFT*/
	-1,	/*SDL_CONTROLLER_BUTTON_DPAD_RIGHT*/
};
//frame from animation that gets painted for each axis (same size as SDL_GameControllerAxis)
const int K_CI_ARR_AXISICONS_FRAMES[SDL_CONTROLLER_AXIS_MAX] = {
	-1,	/*SDL_CONTROLLER_AXIS_LEFTX*/
	-1,	/*SDL_CONTROLLER_AXIS_LEFTY*/
	-1,	/*SDL_CONTROLLER_AXIS_RIGHTX*/
	-1,	/*SDL_CONTROLLER_AXIS_RIGHTY*/
	8,	/*SDL_CONTROLLER_AXIS_TRIGGERLEFT*/
	9,	/*SDL_CONTROLLER_AXIS_TRIGGERRIGHT*/
};

//PlayStation 
//const int K_CI_ARR_BUTICONS_FRAMES[SDL_CONTROLLER_BUTTON_MAX] = {3, 2, 1, 0, -1, -1, -1, -1, -1, 4, 6, -1, -1, -1, -1};
//const int K_CI_ARR_AXISICONS_FRAMES[SDL_CONTROLLER_AXIS_MAX] = {-1, -1, -1, -1, 5, 7};
//Nintendo connected controllers and nintendo separate controllers
//const int K_CI_ARR_BUTICONS_FRAMES[SDL_CONTROLLER_BUTTON_MAX] = {0, 1, 3, 2, -1, -1, -1, -1, -1, 8, 9, -1, -1, -1, -1 };
//const int K_CI_ARR_AXISICONS_FRAMES[SDL_CONTROLLER_AXIS_MAX] = {-1, -1, -1, -1, 6, 7};




/*!
* Types of missions
*/
enum eMissionType {
	K_GAME_LSTYPE_NOTSET = -1,
	K_GAME_LSTYPE_KILL_ALL = 0,
	K_GAME_LSTYPE_HOSTAGE = 1,
	K_GAME_LSTYPE_BOMB,
	K_GAME_LSTYPE_ARREST_WARRANT,
	//last one
	K_GAME_LSTYPES_COUNT
};

// Gets type of mission. Returns 
eMissionType App_GetMissionType(WCHAR * strLevelPath);
// Parses all levels and saves level types into g_levelStats
void App_ParseAllLevelsForData(bool bResetAllStats);
void App_ResetUserData();
void App_SaveUserData();
void App_LoadUserData();
// Reloads all files that can be modded
void App_ReloadContentChanges();
/*!
* Stores data about levels (stars, scores, etc)
* version: 515
*/
struct CLevelStats {
	int		nLevelType;  //mission type flags 
	int		nPlayedTimes;
	int		nStars;
	int		nScoreSolo;			//score solo
	int		nScoreCoop;			//score coop
	int		nBestTimeSec_Solo;		//best time in seconds or 0 for not set
	int		nBestTimeSec_Coop;		

	void Reset()
	{
		nLevelType = K_GAME_LSTYPE_NOTSET; //orice combinatie

		nBestTimeSec_Solo = 0;
		nBestTimeSec_Coop = 0;
		nPlayedTimes = 0;
		nStars = 0;
		nScoreSolo = 0;
		nScoreCoop = 0;
	}

	CLevelStats()
	{
		Reset();
	}
};
//back compatibility
//struct v514 from v0.5.18 RC6 game
struct CLevelStats_v514 {
	int		nLevelType;  //mission type flags 
	int		nPlayedTimes;
	int		nStars;
	int		nXPpoints;	
	int		nBestTimeSec;
};

//max 10 chapters
extern CLevelStats g_levelStats[K_GAME_LEVELS_PER_CHAPTER * 10];

/*!
*	Updates generic level statistics into g_levelStats (stars, scores, thumbs, etc)
*/
void			App_UpdateLevelStats();

/*!
 *	Increases a game statistic and gives badges, calls analytics events, etc
 * \param K_MEMID_GAMESTATS_var - index of GAMESTAT to be increased
 * \param nAddQuantity - value to be added (negative is ok too)
 */
void			App_IncreaseGamestat(int K_MEMID_GAMESTATS_var, int nAddQuantity = 1);

/*!
 *	Sets statistics to specified value
 */
void			App_SetGamestat(int K_MEMID_GAMESTATS_var, int nValue);

/*!
 * \brief Resets userData Key bindings
 * \param nKeyboardIdx - zero based keyboard index
 */
void			App_ResetKeybindings(int nKeyboardIdx);

/*!
 *	Sets the keyboard keys from the codes stored in g_userData
 *	It also saves the keys names into special strings to be displayed later
 */
void			App_SetSDLTriggersFromUserData(CController* ctrlrkeys1, CController* ctrlrkeys2);

/*
#define K_MONITOR_CENTER     0x0001        // center rect to monitor
#define K_MONITOR_CLIP     0x0000        // clip rect to monitor
#define K_MONITOR_WORKAREA 0x0002        // use monitor work area
#define K_MONITOR_AREA     0x0000        // use monitor entire area

extern void App_ClipOrCenterWindowToMonitor(HWND hwnd);
extern void App_ClipOrCenterRectToMonitor(LPRECT prc, UINT flags = K_MONITOR_WORKAREA | K_MONITOR_CENTER);
*/

void			App_CenterRectInRect(RECT *rectSrc, RECT *rectDest);


// Centers the window
void			App_CenterWindowOnMainDisplay(HWND wndHwnd);
// Sets borderless window fullscreen
void			App_ToggleBorderlessFullscreen(HWND wndHwnd);
// Tells us if current window is borderless fullscreen
bool			App_IsBorderlessFullscreen();
// Shows a tutorial whenre nTutID starts from K_MEMID_TUTORIALS_START
// \returns true if tutorial is activated
bool			App_TutorialWindowShow(int nTutID);
// \brief Computes a UINT32 CRC on all important game files to detect changes
// It doesn't look into the mods, just checks the base files
UINT32			App_GetGameFilesCRC();

// \brief Computes a UINT32 CRC on all important game files to detect changes
// Checks game changer mods 
UINT32			App_GetActiveModsCRC();

//maximum number of upgrades
#define K_GAME_MAX_UPGRADE_LEVELS 16
//number of XP points to spend per gained level
#define K_GAME_XPPOINTS_PER_XPLEVEL 2
// Gets number of XP points necessary for a level upgrade (XP bar)
// \param nUpgradeLevel - zero based character upgrade level
UINT32			App_GetMaxXP(int nUpgradeLevel);

// Gives the available XP upgrade points
UINT32			App_GetAvailableXPPoints(EPSSPlayerClass eClass);

//Gets the upgrade level where dwXPpoints belongs
int				App_GetXPLevel(UINT32 dwXPpoints);

//paints a controller key, depending on controller (keyboard or xbox ctrlr)
// \param nAlign -1 left, 1 right, 0 center
void			App_PaintControllerKey(CController* pCtrlr, EControllerCommand eCommand, D3DXVECTOR2 vPos, bool bPressed, int nAlign, DWORD dwColor = 0xffffffff);


///----------------------------- TRANSFORMS HELPERS ----------------------------

//sets the world matrix. Don't set it directly if you need to know the last set world matrix
void			App_SetWorldTransform(LPDIRECT3DDEVICE9 pDevice, D3DXMATRIXA16* matWorld);

///----------------------------- LOCALIZATIONS - LANGUAGES ----------------------------
struct CLocaLanguage {
	CStringHash shLangName;
	CStringHash	shLangAlias;
	CStringHash shFileName;
	bool		bUseTTFonts; //use true type fonts only

	CLocaLanguage() : bUseTTFonts(false)
	{
		shLangName.Reset();
		shLangAlias.Reset();
		shFileName.Reset();
	}
};

extern CLocaLanguage g_Language;						//current game language
extern CGrowableArray<CLocaLanguage> g_arrLangList;		//list of languages from lang.xml
//fonts list
extern CStringHash shTTFID_SZ40;
extern CStringHash shTTFID_SZ30;
extern CStringHash shTTFID_SZ20;

HRESULT App_LocaLoadLangList(CStringHash shSelectedLangAlias);
HRESULT App_LocaLoadStrings();

// Loads necessary fonts upon language change.
// \param bUseTTFonts - Asian languages usually need True Type Fonts so we replace bitmap fonts with TTF
HRESULT App_LocaLoadFonts(bool bUseTTFonts = false);

// Orders language change
HRESULT App_LocaChangeLanguage(CStringHash shSelectedLangAlias);

// Returns current set language
CLocaLanguage App_LocaGetCurrentLanguage();

//returns the language set for the system/game as a char*
//return values: see steam API language codes in supported languages list https://partner.steamgames.com/doc/store/localization#supported_languages
//defaults on first language in g_arrLangList if no compatible language is found
const char* App_LocaGetSystemLanguage();


