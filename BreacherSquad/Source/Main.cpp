#include "dxstdafx.h"
#include <userenv.h>

#if defined(_DEBUG) || defined(DEBUG)
//memory leaks check (Visual Leak Detector) - comment next line to remove
#include "vld.h"
//check leaks
#define _CHECK_HEAP_STACK_

//profile - CheatsRelease
//#define _CHEATS_ENABLED_
#endif

//**************************************************************************************
// Global variables
//**************************************************************************************
///--- FIXED TIMESTEP ---
//fixed timestep simulation (netsync)
#define	K_FIXED_TIMESTEP_DTIME (1.0f / 60.0f)
#define K_FIXED_TIMESTEP_DTIME_MS (1000.0f / 60.0f)

bool						g_bRequestedExit = false;			// Exit game was requested
double						g_fTimeAccumInput = 0.0f;			// Time accumulator that handles the fixed timestep input acquiring pipeline
double						g_fTimeAccumUpdate = 0.0f;			// Time accumulator for fixed timestep update (different from input acquiring)
double						g_fTimeAccumSend = 0.0f;			// Time accumulator for coop input sending

///--- startup commands ---
eStartupCommand				g_startupCommand = GAME_STARTUP_NONE;	// Startup command set usually by command line params
CStringHash					g_startupParam;							// Parameter used for startup commands

///--- network data ---
int							g_nUpdateFrame = 0;					//frame counter for updates
float						g_fLastUpdateTimer = 0.0f;			//timer that detects when network freezes
int							g_nInputFrame = 0;					//input taken frames counter
int							g_nLastSyncedFrame = -1;			//transmit last frame that was simulated locally
DWORD						g_nLastSyncHash = 0;				//last hash to check sync for g_nLastSyncedFrame

bool						g_bShowDebugStats = false;			//when enabled it paints debug information
bool						g_bJustStarted = true;				//game was just started now
bool						g_bForceOneUpdatePerFrame = false;	// flag used to force only one update per frame when necessary (like during loading)

#define						K_GRAVITY	500.0f
Vec2						g_vecGravity;						//gravity

ID3DXSprite*				g_pGameSprite = NULL;				//Main Sprite class 
MatA16						g_matIdentity;						//identity matrix
MatA16						g_matWorld;							//world matrix

CLog*						g_pLog;								//log class

UINT32						g_gameState = GAME_STATE_EMPTY;		//state machine's current state. defined in dxstdafx.h 
UINT32						g_gameSubstate = 0;					//current state's substate - if needed
eGameMode					g_gameMode = GAME_MODE_CLASSIC;		//current selected game mode

float						g_gameStateTimer;					
int							g_gameStateErrorStringIdx;			// not 0 => after changing the state shows error box with specified message

bool						g_bDuringTransition = false;		// Is it during transition?
bool						g_bCanPause = false;				// Global flag: can we pause the game while in background?
bool						g_bLevelNeedsUpdate = false;		//#HACK: pentru un singur frame ramane true dupa resolution change ca sa faca update chiar daca jocul e pe pauza

///--- Redefine Keys ---
EControllerCommand			g_keydef_command = K_CM_COMMAND_NONE;	//command to redefine (NONE means sequence wasn't initialized)
int							g_keydef_scancode = -1;					//scancode for command (SDL scancodes for now)

CMouseData					g_mouse;								// Mouse data, global

CStringsManager				g_stringsMgr;	
CParticlesManager			g_particlesMgr; 
///--- Fonts ---
//fonts pointers
CTexturedFont				*g_font12wow;
CTexturedFont				*g_font10b1, *g_font10bs1;
CTexturedFont				*g_font8b1, *g_font8bs1;
CTexturedFont				*g_font9b1;
CTexturedFont				*g_font6n1, *g_font6ns1, *g_font6nc1;
CTexturedFont				*g_font5n1, *g_font5n2, *g_font5ns2;

CTimersArray				g_timers(3000, 10);					//Timers array

///--- Game classes ---
CPlayerSelScr				g_playerSelScr;						// Player selection screen
CMainMenu					g_mainMenu;							// Main menu class
CLevel						g_level;							// Current Level
CLevelEditor				g_editor;							// Level editor - defined global, initialized on loading, destroyed on app shutdown

#ifdef K_CONTROLS_EDITOR
CControlsEditor				g_ControlsEditor;					// Controls editor for debug/develop mode (F2 to show)
#endif

#ifdef ENABLE_CHAT_WINDOW
CChatWnd					g_ChatWnd;							// Ingame chat window for networked matches
#endif

///--- Lockstep networking class ---
CNetLock					g_netlock;
///-- spine manager --
CSpineManager				g_spineMgr;

CFreeTypeFont				g_font1;

//#TODO: default value for gauss bell with attenuation almost 2 at fRadius * 2.0f
// convert these to constants
float ct_fGaussLen = 0.35f; 
float ct_fLightMul = 2.0f;
float ct_fColorDodge = 0.4f;


//**************************************************************************************
// Forward declarations 
//**************************************************************************************
bool    CALLBACK IsDeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool bWindowed);
void    CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps);
HRESULT CALLBACK OnCreateDevice(PDEVICE pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
HRESULT CALLBACK OnResetDevice(PDEVICE pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc);
void    CALLBACK OnFrameMove(PDEVICE pd3dDevice, double fTime, float fElapsedTime);
void    CALLBACK OnFrameRender(PDEVICE pd3dDevice, double fTime, float fElapsedTime);
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing);
void    CALLBACK KeyboardProc(UINT nChar, bool bKeyDown, bool bAltDown);
void    CALLBACK OnLostDevice(void);
void    CALLBACK OnDestroyDevice(void);
void	CALLBACK MouseProc(bool bLeftButton, bool bRightButton, bool bMiddleButton, bool bSideButton1, bool bSideButton2, int nMouseWheelDelta, int xPos, int yPos);


HRESULT InitApp(void);
void	ShutdownApp(void);
HRESULT	InitSound(void);

// Transition functions
void ChangeGameState(int newState, int param1 = 0, int param2 = 0); //are parametru default, in caz ca e necesar
void ChangeGameStateTransition(int newState, int param1 = 0, int param2 = 0, int transitionType = K_TRANSITION_TYPE_SIMPLE);
void UpdateTransition(float dTime);
void PaintTransition(float dTime, float fTimeline, PDEVICE pDevice); 

///-----------------------------------------------------
/// MISC UTILITY FUNCTIONS
///-----------------------------------------------------

// Spine extension used for allocation and deallocations (singleton)
spine::SpineExtension *spine::getDefaultExtension() {
	static spine::DefaultSpineExtension g_spineExtension;
	return &g_spineExtension;
}

// Callback used by ControllersMgr to normalize mouse input from global to ingame player relative
void NormalizeIngameMouseCoords(int ControllerIID, float fAxisValue, bool bIsHorizontalAxis, float & ret_fAxisValue)
{
	g_level.NormalizeMouseCoords(ControllerIID, fAxisValue, bIsHorizontalAxis, ret_fAxisValue);
	//DebugPrintA("coords: axis:%d %.2f -> %.2f\n", bIsHorizontalAxis, fAxisValue, ret_fAxisValue);
}

//#define DEBUG_VS
//#define DEBUG_PS


INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{

#if defined(_CHECK_HEAP_STACK_)
	_CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode ( _CRT_ERROR, _CRTDBG_MODE_DEBUG);
	/*
	//to catch new and new-delete with line number:
	//#define DEBUG_NEW new(_NORMAL_BLOCK, _FILE_, _LINE_)
	//#define new DEBUG_NEW
	*/
	_ASSERTE( _CrtCheckMemory( ) );
#endif

	//init log system
	g_pLog = new CLog();

	///--- verificari imediate ---
	//verifica sa nu poti lansa jocul de mai multe ori
	if (!UTGetAppClass().IsOnlyInstance(K_GAME_WINDOW_CLASSNAME))
		return 0;

	HRESULT hr = S_OK;
	//Init crash dumper
	InitMiniDumper();
	//initializeaza constante joc gen cai catre executabile samd
	UTGetAppClass().Init();
	//clear debug file 
	DebugLogClear();

#if defined(ENABLE_STEAM)

	//next line starts the Steam client (and the game from Steam library) when Steam isn't started (not good for debug)
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
#else
	//if we uncomment this line it will always call the exe from Steam
	//if (SteamAPI_RestartAppIfNecessary(STEAM_APP_ID))
	//{
	//	//game was started with steam closed, start steam
	//	return -1;
	//}
#endif

	if (!SteamAPI_Init())
	{
		MessageBox(NULL, L"Steam must be running in order to play this game! Please (re)start the Steam Client!", L"Error", MB_OK);
		LOG(L"Steam Client not started! Shutting down!");
		return -1;
	}
#endif //ENABLE_STEAM

#ifdef ENABLE_GALAXY
	galaxy::api::InitOptions initOptions(
		GOG_CLIENT_ID,
		GOG_CLIENT_SECRET
	);

	galaxy::api::Init(initOptions);
	const galaxy::api::IError * err = galaxy::api::GetError();
	if (!err)
	{
		UTGetAppClass().m_Settings.galaxyFullyLoaded = true;
	}
	else
	{
		UTGetAppClass().m_Settings.galaxyFullyLoaded = false;
		ErrorBox(K_ERR_WARNING, L"Galaxy API is not fully loaded. Error: %s", err->GetMsg());
	}

	galaxy::api::User()->SignInGalaxy();
#endif // ENABLE_GALAXY

	// declare that we're DPI aware (even with imgui off)
	ImGui_ImplWin32_EnableDpiAwareness();

	//load game settings FIRST AND FOREMOST (includes selected language and so on)
	UTGetAppClass().LoadSettings();
	//initialize randomness
	randseed(GetTickCount());

	// Set the callback functions. 
	DXUTSetCallbackDeviceCreated(OnCreateDevice);
	DXUTSetCallbackDeviceReset(OnResetDevice);
	DXUTSetCallbackDeviceLost(OnLostDevice);
	DXUTSetCallbackDeviceDestroyed(OnDestroyDevice);
	DXUTSetCallbackMsgProc(MsgProc);
	DXUTSetCallbackKeyboard(KeyboardProc);
	DXUTSetCallbackFrameRender(OnFrameRender);
	DXUTSetCallbackFrameMove(OnFrameMove);
	DXUTSetCallbackMouse(MouseProc, true);

	// Show the cursor and clip it when in full screen
	DXUTSetCursorSettings(true, true);

	//for now it can't pause on losing focus
	g_bCanPause = false;

	if (FAILED(InitApp()))
	{
		WCHAR szResult[MAX_PATH];
		StringCchPrintf(szResult, MAX_PATH, L"Ooops, couldn't initialize game!\r\nTo fix it, check our support forum or contact us at %s\r\n", K_GAME_EMAIL);
		MessageBoxW(NULL, szResult, NULL, MB_OK | MB_ICONERROR);

		return -1;
	}

#if defined(_DEBUG) || defined(DEBUG)
	DXUTInit(true, true, true); // Parse the command line, handle the default hotkeys, and show msgboxes
	DXUTSetShortcutKeySettings(true, true);
#else
	DXUTInit(true, true, false);
	DXUTSetShortcutKeySettings(false, false);
#endif
	DXUTSetMultimonSettings(true);

	WCHAR windowTitle[MAX_PATH];
	StringCchPrintf(windowTitle, MAX_PATH, L"%s", g_stringsMgr.strings[STR_TITLE]->sText);

	if (FAILED(DXUTCreateWindow(windowTitle, hInst, NULL, NULL /*, 0, 0*/)))
	{
		ErrorBox(K_ERR_CRITICAL, L"[ERROR] Couldn't create window!\r\nTo fix it, check our support forum or contact us at %s\r\n", K_GAME_EMAIL);
	}

	///--- startup commands (exe params) ---
#if defined(ENABLE_STEAM_WORKSHOP)
	if (g_startupCommand == GAME_STARTUP_UPLOAD_MOD)
	{
		ChangeGameState(GAME_STATE_UPLOAD_MOD);
	}
#endif

	///--- LOG WINDOW ---
	if (UTGetAppClass().m_Settings.dev_bLogWindowShow)
	{
		OS_CreateLogWindow();
	}

	//Log current time too
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	LOG(L"Log system started. (%d-%d-%d %d:%d:%d)", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
#ifdef ENABLE_STEAM
	LOG(L"Steam Version %s, Savefile Version %d", g_stringsMgr.strings[STR_VERSION_NUMBER]->sText, _VERSION_DATAFILE_);
#endif // ENABLE_STEAM
#ifdef ENABLE_GALAXY
	LOG(L"GoG Version %s, Savefile Version %d", g_stringsMgr.strings[STR_VERSION_NUMBER]->sText, _VERSION_DATAFILE_);
#endif // ENABLE_GALAXY


	//init subsystems
#ifdef ENABLE_ACHIEVEMENTS
	UTGetAchievementManager().Init();
#endif
#ifdef ENABLE_LEADERBOARDS
	UTGetLeaderboards().Init();
#endif

	if (g_startupCommand != GAME_STARTUP_UPLOAD_MOD)
	{
		///--- COMPUTE BASE GAME CRC ---
		//Chapters should be loaded before everything! Load original chapters list to compute game base CRC.
		WCHAR wcsPath[MAX_PATH];
		StringCchPrintf(wcsPath, MAX_PATH, L"%s/levels/missions/missions.xml", UTGetAppClass().g_wszAppResDir);
		UTGetChaptersList().LoadChapters(wcsPath);
		//load infinite tower mode desc
		//g_verticalMode.Init(&g_level, L"media/levels/mod_prefabs/infinite_tower.xml");
		//check CRC after loading chapters (levels needed)
		UINT32 unGameCRC = App_GetGameFilesCRC();
		//we loaded the descriptors
		//unGameCRC += g_verticalMode.GetFilesCRC(true);

		UTGetAppClass().m_Settings.dev_unCurrentCRC = unGameCRC;
		UTGetAppClass().m_Settings.dev_unCurrentModsCRC = unGameCRC;
		//check CRC 
		if (unGameCRC != K_GAME_CRC)
		{
			LOG(L"--> CRC check failed! CRC[%08x]. CHANGES TO CORE FILES DETECTED! <--", unGameCRC);
		}
		else
		{
			LOG(L"--> CRC check ok! CRC[%08x] <--", unGameCRC);
		}

		//loads shop items - must be loaded before loading user data (which tells us what items are unlocked)
		UTGetShop().LoadItemsAndPrices();
		///--- loads user data (saves) ---
		App_LoadUserData();
	}

#ifdef K_NET_STRICT_SYNC_CHECK
	LOG(L"!!!----> Strict NET Sync checking enabled <----!!!");
#endif

#ifdef K_NET_CHECK_IF_NETSYNC_LOCKED
	LOG(L"Net:: Network game netsync lock watchdog enabled!");
#endif

	///--- ANALYTICS (after loading user data - user UID) ---
	CHAR strUID[MAX_PATH];
	StringCchPrintfA(strUID, MAX_PATH, "%u", g_userData[K_MEMID_USER_UID]);

#if defined(_DEBUG) || defined(DEBUG)
	//debug test
	UTGetAnalytics().Init("UA-181007525-1", strUID);
#else

	#ifdef ENABLE_STEAM
		//final steam - RELEASE
	UTGetAnalytics().Init("UA-181007525-2", strUID);
	#endif
	#ifdef ENABLE_GALAXY
		//final GoG - RELEASE
	UTGetAnalytics().Init("UA-181007525-3", strUID);
	#endif

#endif //else

#ifdef ENABLE_NETWORKING
	//initialize network class
	g_pNetwork->Start();
#endif

	// Initialize sound
	V_RETURN(InitSound());

	//trigger resolution change event immediately
	CEvent *nevent = new CEvent(CEventTypes::evtT_SYSTEM, CEventCommands::evtC_SYSTEM_RESOLUTION_CHANGE);
	nevent->AddNamedArgUINT32(L"width", UTGetAppClass().m_Settings.nWindowW);
	nevent->AddNamedArgUINT32(L"height", UTGetAppClass().m_Settings.nWindowH);
	UTGetEventManager().TriggerEvent(nevent);

	if (FAILED(DXUTCreateDevice(D3DADAPTER_DEFAULT, true, UTGetAppClass().m_Settings.nWindowW, UTGetAppClass().m_Settings.nWindowH, IsDeviceAcceptable, ModifyDeviceSettings)))
	{
		ErrorBox(K_ERR_CRITICAL, L"[ERROR] Couldn't create device (%dx%d)!\r\nTo fix it, check our support forum or contact us at %s\r\n", UTGetAppClass().m_Settings.nWindowW, UTGetAppClass().m_Settings.nWindowH, K_GAME_EMAIL);
	}
	
#ifdef ENABLE_DEVMODE_RELEASE
#if defined(_DEBUG) || defined(DEBUG)
	LOG(L"!!!----> DevMode Release enabled! Should be off! <----!!!");
#else
	MessageBox(NULL, L"DevMode Release enabled! Should be off!", L"Please check!", MB_OK);
#endif
#endif

	//after device creation:
	///--- init SDL ---
	UTGetAppClass().InitSDL(DXUTGetHWND());
	//add keyboard controllers and map keys
	CController* ctrlrkeys1 = UTGetCtrlrMgr().AddController(K_CM_CT_KBM_SDL, g_stringsMgr.strings[STR_KEYBOARD1]->sText);
	ctrlrkeys1->nSDLInstanceId = K_CM_IID_KBM1; //set keyboard instance ID so it isn't empty
	//ctrlrkeys1->ClearTriggers(); //clear default mapping

	//CController* ctrlrkeys2 = UTGetControllersManager().AddController(K_CM_CONTROLLERTYPE_KEYBOARD_SDL, g_stringsMgr.strings[STR_KEYBOARD2]->sText);
	//ctrlrkeys2->nSDLInstanceId = K_CM_DEFAULT_KEYBOARD2_INSTANCE_ID; //set keyboard instance ID so it isn't empty
	//ctrlrkeys2->ClearTriggers(); //clear default mapping

	//App_SetSDLTriggersFromUserData(ctrlrkeys1, ctrlrkeys2);
	
	//add network controller for coop play (used for peer controller simulation)
	CController* ctrlrnet1 = UTGetCtrlrMgr().AddController(K_CM_CT_NET_FRAMELOCK, g_stringsMgr.strings[STR_NETWORK1]->sText);
	ctrlrnet1->nSDLInstanceId = K_CM_IID_NET1;


	//find/add controllers if any
	UTGetCtrlrMgr().RegisterAllSDLControllers();

	//send analytics about gfx caps
	CHAR ctxt[MAX_PATH];
	StringCchPrintfA(ctxt, MAX_PATH, "GFXflags:%d", UTGetAppClass().g_gfxFlags);
	
	//send data analytics about controllers
	/*
	int nHasControllers = 0;
	for (int kk = 0; kk < UTGetControllersManager().m_arrControllers.GetSize(); kk++)
	{
		if (UTGetControllersManager().m_arrControllers[kk]->eType == K_CM_CONTROLLERTYPE_JOYSTICK_SDL)
		{
			nHasControllers++;
		}
	}
	CHAR ctxt[MAX_PATH];
	StringCchPrintfA(ctxt, MAX_PATH, "joysticks:%d", nHasControllers);
	ANALYTICS_EVENT("controllers_count", ctxt, "", nHasControllers);
	*/

	//see if resolution is supported
	SIZEWH szwh(UTGetAppClass().m_Settings.nWindowW, UTGetAppClass().m_Settings.nWindowH);
	LOG(L"GFX:: Settings Resolution:%dx%d fullscreen:%d", szwh.w, szwh.h, UTGetAppClass().m_Settings.bFullscreen);
	if (UTGetAppClass().g_arrResolutions.IndexOf(szwh) < 0)
	{
		ErrorBox(K_ERR_WARNING, L"Unsupported window size found in settings (%d x %d)! Resetting to SAFE DEFAULTS!", szwh.w, szwh.h);
		//reset resolution
		UTGetAppClass().m_Settings.nWindowW = K_WINDOW_WIDTH_SAFE;
		UTGetAppClass().m_Settings.nWindowH = K_WINDOW_HEIGHT_SAFE;
		UTGetAppClass().m_Settings.bFullscreen = false;

		SetWindowPos(DXUTGetHWND(), 0, 0, 0, K_WINDOW_WIDTH_SAFE, K_WINDOW_HEIGHT_SAFE, SWP_NOOWNERZORDER | SWP_NOZORDER);
		UTGetAppClass().SaveSettings();
	}
	//--- start fullscreen? ---
	if (UTGetAppClass().m_Settings.bFullscreen)
	{
		if (UTGetAppClass().m_Settings.bBorderlessFullscreen)
		{
			LOG(L"GFX:: Switching to borderless fullscreen.");
			HWND hwndWindowed = DXUTGetHWNDDeviceWindowed();
			App_ToggleBorderlessFullscreen(hwndWindowed);
		}
		else
		{
			LOG(L"GFX:: Switching to exclusive fullscreen.");
			DXUTToggleFullScreen();
		}
	}
	else //center window
	{
		LOG(L"GFX:: Starting Windowed. Centered window.");
		HWND hwndWindowed = DXUTGetHWNDDeviceWindowed();
		App_CenterWindowOnMainDisplay(hwndWindowed);
	}

	//log GFX type
	LOG(L"GFX [%s] GFXflags [%d]", DXUTGetDeviceStats(), UTGetAppClass().g_gfxFlags);

#ifdef ENABLE_CHAT_WINDOW
	g_ChatWnd.Init();
#endif

	LOG(L"System:: All systems up and running!");

	if (SUCCEEDED(hr))
	{
		// Pass control to the framework for handling the message pump and 
		// dispatching render calls. The framework will call FrameMove 
		// and FrameRender callback when there is idle time between handling window messages.
		DXUTMainLoop();
	}
	LOG(L"System:: Main loop ended.");

	///--- release all controllers ---
	UTGetCtrlrMgr().ReleaseAllControllers(false);
	///--- shut down SDL ---
	UTGetAppClass().CloseSDL();

	// Perform any application-level cleanup here. Direct3D device resources are released within the
	// appropriate callback functions and therefore don't require any cleanup code here.
	ShutdownApp();

#ifdef ENABLE_STEAM
	SteamAPI_Shutdown();
#endif
#ifdef ENABLE_GALAXY
	galaxy::api::Shutdown();
#endif // ENABLE_GALAXY

	//shutdown Log
	SAFE_DELETE(g_pLog);

#if defined(_CHECK_HEAP_STACK_)
	_ASSERTE( _CrtCheckMemory( ) );
#endif

	return DXUTGetExitCode();
}

//**************************************************************************************
// Initialize the app - before creating the window
//**************************************************************************************
/*
struct CTest {
	int value;
	CTest() { value = 1; ErrorBox(K_ERR_WARNING, L"CTest constructed!"); };
	~CTest() { ErrorBox(K_ERR_WARNING, L"CTest destructor! val:%d", value); };
};
*/

HRESULT InitApp(void)
{
	/*
	std::vector<std::shared_ptr<CTest>> vecActors;
	auto nact = std::make_shared<CTest>();
	nact->value = 666;
	ErrorBox(K_ERR_WARNING, L"Value:%d", nact->value);
	vecActors.push_back(std::move(nact));

	// set weak_ptr to point
	std::weak_ptr<CTest> pointing;
	pointing = vecActors[0];
	{
		auto test = pointing.lock();
		if (test) {
			ErrorBox(K_ERR_WARNING, L"pointing Value:%d", test->value);
		}
	}


	ErrorBox(K_ERR_WARNING, L"deallocationg");
	vecActors.clear();

	{
		auto test = pointing.lock();
		if (test) {
			ErrorBox(K_ERR_WARNING, L"pointing Value:%d", test->value);
		}
		else 
		{
			// goes here!!
			ErrorBox(K_ERR_WARNING, L"pointing Value was reset!");
		}
	}
	*/

	HRESULT hr = S_OK;

	g_bDuringTransition = false; //nu este in timpul unei tranzitii

 ///--- Load strings here so we can set the window name ---
	if (FAILED(hr = App_LocaLoadLangList(UTGetAppClass().m_Settings.shLanguageAlias)))
	{
		ErrorBox(K_ERR_WARNING, L"[ERROR] Error loading strings list [texts/lang.xml]!");
	}
	//load strings for current language
	if (FAILED(hr = App_LocaLoadStrings()))
	{
		ErrorBox(K_ERR_CRITICAL, L"Critical Error! Failed loading strings XML.");
		return hr;
	}

	//--------------------------------------------------------------------------------------
	// setari initiale
	//--------------------------------------------------------------------------------------
	MUMatIdentity(&g_matIdentity);
	MUMatIdentity(&g_matWorld);
	g_vecGravity = Vec2(0.0f, K_GRAVITY);

	//set version number
	g_stringsMgr.SetString(STR_VERSION_NUMBER, L"v%d.%d.%d", _VERSION_MAJOR_, _VERSION_MINOR_, _VERSION_PATCH_);
	//--------------------------------------------------------------------------------------
	// add listeners
	//--------------------------------------------------------------------------------------
	//first listener must be UTAppClass
	UTGetEventManager().AddListener(&UTGetAppClass(), CEventTypes::evtT_SYSTEM);
	UTGetEventManager().AddListener(&UTGetAppClass(), CEventTypes::evtT_CONTROLS);
	UTGetEventManager().AddListener(&UTGetAppClass(), CEventTypes::evtT_GAMESTATE);
	//managerul de sunet
	UTGetEventManager().AddListener(&UTGetSoundManager(), CEventTypes::evtT_SOUND);

	//--------------------------------------------------------------------------------------
	// Script processors
	//--------------------------------------------------------------------------------------
	UTGetScriptManager().AddProcessor(&g_level);

	//initial game state
	g_gameStateErrorStringIdx = -1; //no error message
	//default to a good game mode
	g_gameMode = GAME_MODE_CLASSIC;

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	ChangeGameState(GAME_STATE_LOADING);
#else
	ChangeGameState(GAME_STATE_PUBLISHER);
#endif

	return S_OK;
}

void ShutdownApp(void)
{
	g_editor.Release();
	g_stringsMgr.Release();
	g_particlesMgr.Release();

	UTGetSoundManager().Release();
	UTGetScriptManager().Release();

	UTGetAnalytics().Shutdown();

#ifdef ENABLE_ACHIEVEMENTS
	UTGetAchievementManager().Release();
#endif
#ifdef ENABLE_LEADERBOARDS
	UTGetLeaderboards().Release();
#endif
	g_spineMgr.Release();

}


//*************************************************************************************************
// Sound initialization
//*************************************************************************************************

HRESULT InitSound(void)
{
	HRESULT hr = S_OK;
	// Initialize sound after we have the window
	//--- init sound system ---
	if (FAILED(UTGetSoundManager().Init(DXUTGetHWND(), 2, 44100, 16)))
	{
		ErrorBox(K_ERR_WARNING, L"Failed INITSOUND->g_pSoundManager->Init()\nSOUNDS WILL BE DISABLED!\n");
		return hr;
	}

	UTGetSoundManager().EnablePositionalSounds(Vec2(0.0f, 0.0f), Vec2(UTGetAppClass().g_rectGameScreen.w * 0.7f, UTGetAppClass().g_rectGameScreen.h * 0.7f));
	UTGetSoundManager().SetListenerVolumeFadeStart(0.7f);

	return hr;
}


//**************************************************************************************
// Called during device initialization, this code checks the device for some 
// minimum set of capabilities, and rejects those that don't pass by returning false.
//**************************************************************************************
bool CALLBACK IsDeviceAcceptable(D3DCAPS9* pCaps, D3DFORMAT AdapterFormat, D3DFORMAT BackBufferFormat, bool /*bWindowed*/)
{
	// Skip backbuffer formats that don't support alpha blending
	IDirect3D9* pD3D = DXUTGetD3DObject();
	if (FAILED(pD3D->CheckDeviceFormat(pCaps->AdapterOrdinal, pCaps->DeviceType,
		AdapterFormat, D3DUSAGE_QUERY_POSTPIXELSHADER_BLENDING,
		D3DRTYPE_TEXTURE, BackBufferFormat)))
		return false;

	//DMC:request 8bit stencil support
	if (FAILED(pD3D->CheckDeviceFormat(pCaps->AdapterOrdinal, pCaps->DeviceType,
		AdapterFormat, D3DUSAGE_DEPTHSTENCIL,
		D3DRTYPE_SURFACE, D3DFMT_D24S8)))
		return false;

	//DMC: reject if doesn't support at least ps2.0
	if (pCaps->PixelShaderVersion < D3DPS_VERSION(2, 0))
		return false;

	return true;
}

//**************************************************************************************
// This callback function is called immediately before a device is created to allow the 
// application to modify the device settings. The supplied pDeviceSettings parameter 
// contains the settings that the framework has selected for the new device, and the 
// application can make any desired changes directly to this structure.  Note however that 
// the sample framework will not correct invalid device settings so care must be taken 
// to return valid device settings, otherwise IDirect3D9::CreateDevice() will fail.  
//**************************************************************************************
void CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, const D3DCAPS9* pCaps)
{
	IDirect3D9* pD3D = DXUTGetD3DObject();
	// If device doesn't support HW T&L or doesn't support 1.1 vertex shaders or vertex tweening in HW 
	// then switch to SWVP.
	if ((pCaps->DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) == 0 ||
		(pCaps->VertexShaderVersion < D3DVS_VERSION(1,1)) )   //ultima linie e adaugata de mine
	{
		pDeviceSettings->BehaviorFlags = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
	else
	{
		pDeviceSettings->BehaviorFlags = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	}

	// This application is designed to work on a pure device by not using 
	// IDirect3D9::Get*() methods, so create a pure device if supported and using HWVP.
	if ((pCaps->DevCaps & D3DDEVCAPS_PUREDEVICE) != 0 && 
		(pDeviceSettings->BehaviorFlags & D3DCREATE_HARDWARE_VERTEXPROCESSING) != 0)
		pDeviceSettings->BehaviorFlags |= D3DCREATE_PUREDEVICE;

	//presentation interval:
	//DEFAULT = refresh rate
	//IMMEDIATE - ignore vsync
	pDeviceSettings->pp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT; 

	//vede daca stie separate alpha blending pt problema cu alpha pe render target
	if ((pCaps->PrimitiveMiscCaps & D3DPMISCCAPS_SEPARATEALPHABLEND) == 0)
	{
		ErrorBox(K_ERR_WARNING, L"Separate Alpha Blending not supported!");

		UTGetAppClass().g_gfxFlags &= ~K_UT_GFXFLAG_SEPARATEALPHABLEND;
	}
	else
	{
		UTGetAppClass().g_gfxFlags |= K_UT_GFXFLAG_SEPARATEALPHABLEND;
	}

	//--- does it support 2d clipping? ----
	if ((pCaps->RasterCaps & D3DPRASTERCAPS_SCISSORTEST) == 0)
	{
		ErrorBox(K_ERR_WARNING, L"Scissor Test not supported!");
		UTGetAppClass().g_gfxFlags &= ~K_UT_GFXFLAG_SCISSORTEST;
	}
	else
	{
		UTGetAppClass().g_gfxFlags |= K_UT_GFXFLAG_SCISSORTEST;
	}

	//request stencil buffer
	if (FAILED(pD3D->CheckDeviceFormat(pDeviceSettings->AdapterOrdinal, pDeviceSettings->DeviceType,
		pDeviceSettings->AdapterFormat, D3DUSAGE_DEPTHSTENCIL,
		D3DRTYPE_SURFACE, D3DFMT_D24S8)))
	{
		//TODO: showld fall back on 1bit stencil 
		ErrorBox(K_ERR_CRITICAL, L"8bit Stencil not supported!");

		UTGetAppClass().g_gfxFlags &= ~K_UT_GFXFLAG_8BITSTENCIL;
		UTGetAppClass().g_stencilBits = 0;
	}
	else
	{
		pDeviceSettings->pp.EnableAutoDepthStencil = TRUE;
		pDeviceSettings->pp.AutoDepthStencilFormat = D3DFMT_D24S8;

		UTGetAppClass().g_gfxFlags |= K_UT_GFXFLAG_8BITSTENCIL;
		UTGetAppClass().g_stencilBits = 8;
	}


	// Debugging vertex shaders requires either REF or software vertex processing 
	// and debugging pixel shaders requires REF.  
#ifdef DEBUG_VS
	if (pDeviceSettings->DeviceType != D3DDEVTYPE_REF)
	{
		pDeviceSettings->BehaviorFlags &= ~D3DCREATE_HARDWARE_VERTEXPROCESSING;
		pDeviceSettings->BehaviorFlags &= ~D3DCREATE_PUREDEVICE;
		pDeviceSettings->BehaviorFlags |= D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	}
#endif
#ifdef DEBUG_PS
	pDeviceSettings->DeviceType = D3DDEVTYPE_REF;
#endif

	// For the first device created if its a REF device, optionally display a warning dialog box
	static bool s_bFirstTime = true;
	if (s_bFirstTime)
	{
		s_bFirstTime = false;
		if (pDeviceSettings->DeviceType == D3DDEVTYPE_REF)
		{
			ErrorBox(K_ERR_WARNING, L"Created REF device! The game will run slowly.");
		}
	}

}

//**************************************************************************************
// This callback function will be called immediately after the Direct3D device has been 
// created, which will happen during application initialization and windowed/full screen 
// toggles. This is the best location to create D3DPOOL_MANAGED resources since these 
// resources need to be reloaded whenever the device is destroyed. Resources created  
// here should be released in the OnDestroyDevice callback. 
//**************************************************************************************

HRESULT CALLBACK OnCreateDevice(PDEVICE pDevice, const D3DSURFACE_DESC* pBBDesc)
{
	HRESULT hr = S_OK;

	//trigger resolution change immediately
	CEvent *nevent = new CEvent(CEventTypes::evtT_SYSTEM, CEventCommands::evtC_SYSTEM_RESOLUTION_CHANGE);
	nevent->AddNamedArgUINT32(L"width", pBBDesc->Width);
	nevent->AddNamedArgUINT32(L"height", pBBDesc->Height);
	UTGetEventManager().TriggerEvent(nevent);

	// check minimum requirements and exit if not met
	if (FAILED(UTGetAppClass().VerifyRequirements()))
	{
		DXUTShutdown();
		return S_OK;
	}

	UTGetTTFManager().OnCreateDevice(pDevice, pBBDesc);

	V_RETURN(UTGetAppClass().OnCreateDevice(pDevice, pBBDesc));
	V_OP_RETHR(UTGetRTManager().OnCreateDevice(pDevice, pBBDesc));
	UTimgui().OnCreateDevice(pDevice, pBBDesc);
	V_RETURN(UTGetShaderManager().OnCreateDevice(pDevice, pBBDesc));
	V_OP_RETHR(UTPainter().OnCreateDevice(pDevice, pBBDesc));
	V_RETURN(UTGetFontsManager().OnCreateDevice(pDevice, pBBDesc));
	V_RETURN(g_level.OnCreateDevice(pDevice, pBBDesc));
	V_OP_RETHR(g_editor.OnCreateDevice(pDevice, pBBDesc));
	V_RETURN(g_particlesMgr.OnCreateDevice(pDevice, pBBDesc));
	V_RETURN(UTGetGUI().OnCreateDevice(pDevice, pBBDesc));
	V_RETURN(g_playerSelScr.OnCreateDevice(pDevice, pBBDesc));
	V_RETURN(g_mainMenu.OnCreateDevice(pDevice, pBBDesc));
	V_RETURN(g_spineMgr.OnCreateDevice(pDevice, pBBDesc));

#ifdef K_CONTROLS_EDITOR
	V_RETURN(g_ControlsEditor.OnCreateDevice(pDevice, pBBDesc));
#endif

	//restore sampler settings
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	//texture mirrors
	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
	pDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
	pDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);

	return S_OK;
}

//**************************************************************************************
// This callback function will be called immediately after the Direct3D device has been 
// reset, which will happen after a lost device scenario. This is the best location to 
// create D3DPOOL_DEFAULT resources since these resources need to be reloaded whenever 
// the device is lost. Resources created here should be released in the OnLostDevice 
// callback. 
//**************************************************************************************
HRESULT CALLBACK OnResetDevice(PDEVICE pDevice, const D3DSURFACE_DESC* pBBDesc)
{
	LOG(L"---OnResetDevice w:%d h:%d ---", pBBDesc->Width, pBBDesc->Height);
	
	//trigger resolution change immediately
	CEvent *nevent = new CEvent(CEventTypes::evtT_SYSTEM, CEventCommands::evtC_SYSTEM_RESOLUTION_CHANGE);
	nevent->AddNamedArgUINT32(L"width", pBBDesc->Width);
	nevent->AddNamedArgUINT32(L"height", pBBDesc->Height);
	UTGetEventManager().TriggerEvent(nevent);

	//keep render rect always updated - se cheama si prin triggerEvent de mai sus
	//UTGetAppClass().OnRenderSizeChanged(pBackBufferSurfaceDesc->Width, pBackBufferSurfaceDesc->Height);
	//se va auzi inca jumatate de ecran in afara ecranului vizibil
	UTGetSoundManager().EnablePositionalSounds(Vec2(0.0f, 0.0f), Vec2(UTGetAppClass().g_rectGameScreen.w * 0.7f, UTGetAppClass().g_rectGameScreen.h * 0.7f));

	HRESULT hr;

	// Create main game sprite
	V_RETURN(D3DXCreateSprite(pDevice, &g_pGameSprite));
	//should be first to be called here
	V_RETURN(UTGetAppClass().OnResetDevice(pDevice, pBBDesc));
	// Because the render targets and handled globally and are changing in size depending on screen resolution we just release them in OnLostDevice and re-create them in OnResetDevice
	V_OP_RETHR(UTGetRTManager().OnResetDevice(pDevice, pBBDesc));

	// Create necessary render targets when device gets reset (created or reset)
	float fAspectReal = (float)pBBDesc->Width / (float)pBBDesc->Height;
	float fAspect = LIMIT(fAspectReal, K_WINDOW_ASPECT_RATIO_MIN, K_WINDOW_ASPECT_RATIO_MAX);
	UINT fGameHpx = K_GAME_HEIGHT * K_GAME_PIXEL_SIZE;
	UINT fGameWpx = (UINT)ceil(fGameHpx * fAspect);
	// Create RTs
	UTGetRTManager().AddRT(K_RTID_COLORDEPTHSTENCIL, fGameWpx, fGameHpx, 1, D3DFMT_A8R8G8B8, false);
	UTGetRTManager().AddRT(K_RTID_TEMP1, fGameWpx, fGameHpx, 1, D3DFMT_A8R8G8B8, false);
	UTGetRTManager().AddRT(K_RTID_FINAL, fGameWpx, fGameHpx, 1, D3DFMT_A8R8G8B8, false);
	//if (UTGetAppClass().m_Settings.nLOD_lights >= K_UT_LOD_MED)
		//UTGetRenderTargetsManager().AddRT(K_RTID_SPECULARMAP, fGameWpx, fGameHpx, 1, D3DFMT_A8R8G8B8, false);

	UTimgui().OnResetDevice(pDevice, pBBDesc);
	V_RETURN(UTGetShaderManager().OnResetDevice(pDevice, pBBDesc));
	V_OP_RETHR(UTPainter().OnResetDevice(pDevice, pBBDesc));

	UTGetTTFManager().OnResetDevice(pDevice, pBBDesc);

	V_RETURN(UTGetFontsManager().OnResetDevice(pDevice, pBBDesc));
	V_RETURN(g_level.OnResetDevice(pDevice, pBBDesc));
	V_OP_RETHR(g_editor.OnResetDevice(pDevice, pBBDesc));
	V_RETURN(g_particlesMgr.OnResetDevice(pDevice, pBBDesc));
	V_RETURN(UTGetGUI().OnResetDevice(pDevice, pBBDesc));
	V_RETURN(g_playerSelScr.OnResetDevice(pDevice, pBBDesc));
	V_RETURN(g_mainMenu.OnResetDevice(pDevice, pBBDesc));
	V_RETURN(g_spineMgr.OnResetDevice(pDevice, pBBDesc));

#ifdef K_CONTROLS_EDITOR
	V_RETURN(g_ControlsEditor.OnResetDevice(pDevice, pBBDesc));
	g_ControlsEditor.SetSpritePtr(g_pGameSprite);
#endif
	//--- set Sprite painter class pointer ---
	g_level.SetSpritePtr(g_pGameSprite);
	g_particlesMgr.SetSpritePtr(g_pGameSprite);
	CTexturedFont::SetGlobalSpritePtr(g_pGameSprite);
	CSprite::SetGlobalSpritePtr(g_pGameSprite, &UTPainter());
	CTTFontsManager::SetGlobalSpritePtr(g_pGameSprite);
	UTGetGUI().SetSpritePtr(g_pGameSprite);
	g_mainMenu.SetSpritePtr(g_pGameSprite);

	///--- write resolution string for use in options screen ---
	WCHAR wsResStr[1024] = { 0 };
	for (int kk = 0; kk < UTGetAppClass().g_arrResolutions.GetSize(); kk++)
	{
		WCHAR wsRes[MAX_PATH];
		if (kk < UTGetAppClass().g_arrResolutions.GetSize() - 1)
			StringCchPrintf(wsRes, MAX_PATH, L"%dx%d\n", UTGetAppClass().g_arrResolutions[kk].w, UTGetAppClass().g_arrResolutions[kk].h);
		else
			StringCchPrintf(wsRes, MAX_PATH, L"%dx%d", UTGetAppClass().g_arrResolutions[kk].w, UTGetAppClass().g_arrResolutions[kk].h);

		StringCchCat(wsResStr, 1024, wsRes);
	}
	g_stringsMgr.SetString(STR_RESOLUTIONS_LIST, wsResStr);


	//reface setarile initiale
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	pDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	pDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	//texture mirrors
	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);
	pDevice->SetSamplerState(1, D3DSAMP_ADDRESSU, D3DTADDRESS_MIRROR);
	pDevice->SetSamplerState(1, D3DSAMP_ADDRESSV, D3DTADDRESS_MIRROR);

	return S_OK;
}

//**************************************************************************************
// This callback function will be called immediately after the Direct3D device has 
// entered a lost state and before IDirect3DDevice9::Reset is called. Resources created
// in the OnResetDevice callback should be released here, which generally includes all 
// D3DPOOL_DEFAULT resources. See the "Lost Devices" section of the documentation for 
// information about lost devices.
//**************************************************************************************
void CALLBACK OnLostDevice(void)
{
	DebugPrintA("---On lost device---\n");

	UTGetAppClass().OnLostDevice();
	UTimgui().OnLostDevice();
	UTGetTTFManager().OnLostDevice();
	UTGetShaderManager().OnLostDevice();
	UTPainter().OnLostDevice();

	UTGetFontsManager().OnLostDevice();
	UTGetGUI().OnLostDevice();
	//because the render targets and handled globally and are changing in size depending on screen resolution we just release them in OnLostDevice and re-create them in OnResetDevice
	UTGetRTManager().Release();
	UTGetRTManager().OnLostDevice();

	g_level.OnLostDevice();
	g_editor.OnLostDevice();
	g_particlesMgr.OnLostDevice();
	g_playerSelScr.OnLostDevice();
	g_mainMenu.OnLostDevice();
	g_spineMgr.OnLostDevice();

	SAFE_RELEASE(g_pGameSprite);

#ifdef K_CONTROLS_EDITOR
	g_ControlsEditor.OnLostDevice();
#endif
}

//**************************************************************************************
// This callback function will be called immediately after the Direct3D device has 
// been destroyed, which generally happens as a result of application termination or 
// windowed/full screen toggles. Resources created in the OnCreateDevice callback 
// should be released here, which generally includes all D3DPOOL_MANAGED resources. 
//**************************************************************************************
void CALLBACK OnDestroyDevice(void)
{
	g_font1.Release();

	UTGetAppClass().OnDestroyDevice();
	UTGetRTManager().OnDestroyDevice();
	UTimgui().OnDestroyDevice();
	UTGetShaderManager().OnDestroyDevice();
	UTPainter().OnDestroyDevice();
	UTGetTTFManager().OnDestroyDevice();
	UTGetFontsManager().OnDestroyDevice();
	UTGetGUI().OnDestroyDevice();
	g_level.OnDestroyDevice();
	g_editor.OnDestroyDevice();
	g_particlesMgr.OnDestroyDevice();
	g_playerSelScr.OnDestroyDevice();
	g_mainMenu.OnDestroyDevice();
	g_spineMgr.OnDestroyDevice();

#ifdef K_CONTROLS_EDITOR
	g_ControlsEditor.OnDestroyDevice();
#endif

}

///----------------------------------------------------
/// Updates the game
///----------------------------------------------------
void UpdateGame(PDEVICE pDevice, float fElapsedTime, float fTime, bool bNetCoop)
{
	bool bSyncUpdate = bNetCoop;

	//--- update global timers ---
	g_timers.Update(fElapsedTime);
	//--- update application class ---
	UTGetAppClass().Update(fElapsedTime);
	//--- update clasa sunete pentru fade-uri ---
	UTGetSoundManager().Update(fElapsedTime);
	//-=-=-= controllers update =-=-=-
	//--must be called before updates: ca sa corespunda bPressed cu JUST_PRESSED
	g_mouse.Update(fElapsedTime);
	//hide mouse when not moving (not too often)
	if (g_mouse.fTimeSinceInput > 5.0f)
	{
		int times = ShowCursor(false);
		while (times > 0)
			times = ShowCursor(false);
	}

	//update controls manager
	UTGetGUI().Update(fElapsedTime);

	switch (g_gameState)
	{
		case GAME_STATE_PUBLISHER:
		{
			UTGetAppClass().App_UpdateState_Publisher(pDevice, fTime, fElapsedTime);
		}
		break;
		case GAME_STATE_DEVELOPER:
		{
			UTGetAppClass().App_UpdateState_Developer(pDevice, fTime, fElapsedTime);
		}
		break;
		case GAME_STATE_LOADING:
		{
			UTGetAppClass().App_UpdateState_Loading(pDevice, fTime, fElapsedTime);
		}
		break;
		case GAME_STATE_SPLASH:
		{
			UTGetAppClass().App_UpdateState_Splash(pDevice, fTime, fElapsedTime);
		}
		break;

		//utility mod uploading to Steam
		case GAME_STATE_UPLOAD_MOD:
		{
#ifdef ENABLE_STEAM_WORKSHOP
			switch (g_gameSubstate)
			{
				case 0: //a few settings and checks
				{
					UTGetAppClass().m_Settings.dev_bDevMode = true;

					LOG(L"\nMod Upload/Update Started...");
					g_gameSubstate++;
				}
				break;
				case 1:
				{
					LOG(L"[Workshop] Trying to update mod...");
					//try update
					if (true == Workshop_UpdatePublished(g_startupParam.text))
					{
						g_gameSubstate = 10; //exit
						LOG(L"Mod UPDATED successfully!");
						MessageBox(null, L"Mod UPDATED successfully!", L"Info", MB_OK);
					}
					else
					{
						LOG(L"Mod not found! Uploading as new mod.");
						g_gameSubstate = 2;
					}
				}
				break;
				case 2:
				{
					LOG(L"--- PUBLISHING NEW MOD ---");
					//try publish
					if (true == Workshop_Publish(g_startupParam.text))
					{
						LOG(L"Mod UPLOADED successfully as new mod!");
						MessageBox(null, L"Mod UPLOADED successfully as new mod!", L"Info", MB_OK);
					}
					else
					{
						LOG(L"Mod upload FAILED! See error.log for details!");
						MessageBox(null, L"Mod upload FAILED! See error.log for details!", L"Error", MB_OK | MB_ICONERROR);
					}

					g_gameSubstate++;
				}
				break;

				//exit game at the end
				default:
				{
					PostQuitMessage(0);
				}
				break;
			}
			break;
#endif
		}
		break;

		case GAME_STATE_PLAYER_SELECTION:
		{
			if ((!g_bDuringTransition) && (!UTGetGUI().bIsBlocking))
				g_playerSelScr.Update(fElapsedTime);
		}
		break;

		case GAME_STATE_JOIN_COOP_LIST:
		{
			//update list on timer
			if (g_timers.Tick(2000))
			{
				CCtrlLayer* lay = UTGetGUI().GetTopmostInputLayer();
				//disable the refresh button if still working
				if (lay != null)
				{
					CControl* ctrl = lay->GetControlByName("BUT_REFRESH_LOBBIES");
					if (ctrl)
					{
						ctrl->bDisabled = false;
						if (g_pNetwork->IsRequestingLobby())
							ctrl->bDisabled = true;
					}
				}

				int nLobbiesCnt = g_pNetwork->GetLobbyListEntriesCount();
				if (nLobbiesCnt == 0)
				{
					if (!g_pNetwork->IsRequestingLobby())
						g_stringsMgr.SetString(STR_LOBBIES_LIST_VAL, L"%s", g_stringsMgr.strings[STR_NO_LOBBIES]->sText);

					//disable controls (list, join)
					if (lay != null)
					{
						CControl* ctrl = lay->GetControlByName("BUT_JOIN_LOBBY");
						if (ctrl)
							ctrl->bDisabled = true;
						ctrl = lay->GetControlByName("CTRL_LOBBIES_SELECTOR");
						if (ctrl)
						{
							ctrl->bDisabled = true;
							ctrl->paramsDict.SetNamedVarINT32(L"nOptionsCnt", 1);
						}
					}
				}
				else
				{
					WCHAR	strLobbiesList[2048] = { 0 };
					for (int kk = 0; kk < nLobbiesCnt; kk++)
					{
						CStringDesc sdName;
						uint64_t iLobbyID = 0;
						char strLobbyName[250];

						g_pNetwork->GetLobbyListEntry(kk, iLobbyID, strLobbyName);
						g_stringsMgr.SetStringDescUTF8(&sdName, strLobbyName);

						StringCchCat(strLobbiesList, 2048, sdName.sText);
						if (kk < nLobbiesCnt - 1)
							StringCchCat(strLobbiesList, 2048, L"\n");
					}

					g_stringsMgr.SetString(STR_LOBBIES_LIST_VAL, strLobbiesList);

					//enable controls (list, join)
					if (lay != null)
					{
						CControl* ctrl = lay->GetControlByName("BUT_JOIN_LOBBY");
						if (ctrl)
							ctrl->bDisabled = false;
						ctrl = lay->GetControlByName("CTRL_LOBBIES_SELECTOR");
						if (ctrl)
						{
							ctrl->bDisabled = false;
							ctrl->paramsDict.SetNamedVarINT32(L"nOptionsCnt", nLobbiesCnt);
						}
					}
				}
			}
		}
		break;

		case GAME_STATE_WORKSHOP:
		case GAME_STATE_GAME_MODE_SELECTION:
		case GAME_STATE_LEVEL_SELECTION:
		case GAME_STATE_CHAPTER_SELECTION:
		{
			if ((!g_bDuringTransition) && (!UTGetGUI().bIsBlocking))
				g_mainMenu.Update(fElapsedTime);
		}
		break;

		case GAME_STATE_NET_LOBBY:
		{
			if (g_bDuringTransition)
				break;
			//update background
			if (!UTGetGUI().bIsBlocking)
				g_mainMenu.Update(fElapsedTime);

			//update lobby
			g_netlock.Net_UpdateLobby(fElapsedTime);
		}
		break;

		case GAME_STATE_MAINMENU:
		{
			//offer to reset the user data
			if (g_userData[K_MEMID_OFFER_RESET_USER_DATA] != 0)
			{
				UTGetGUI().ShowLayerOnce("LAYER_ID_RESET_PROGRESS_EA");
				g_userData[K_MEMID_OFFER_RESET_USER_DATA] = 0;
			}

			if ((!g_bDuringTransition) && (!UTGetGUI().bIsBlocking))
				g_mainMenu.Update(fElapsedTime);

			//always check to see if menu exists
#ifdef ENABLE_STEAM_WORKSHOP
			if (UTGetGUI().GetLayerByName("LAYER_ID_MAINMENU") == null)
			{
				UTGetGUI().ShowLayerOnce("LAYER_ID_MAINMENU");
			}
#else
			if (UTGetGUI().GetLayerByName("LAYER_ID_MAINMENU_NOWORKSHOP") == null)
			{
				UTGetGUI().ShowLayerOnce("LAYER_ID_MAINMENU_NOWORKSHOP");
			}
#endif

			for (UINT kk = 0; kk < UTGetCtrlrMgr().m_arrControllers.size(); kk++)
			{
				if (UTGetCtrlrMgr().m_arrControllers[kk]->sCommands.keyState[K_CM_COMMAND_BACK] == K_CM_BUTSTATE_JUSTPRESSED)
				{
					CCtrlLayer* layer = UTGetGUI().GetLayerByName("LAYER_ID_QUITGAME");
					if ((layer == null) && (!UTGetGUI().bIsBlocking))
					{
						SND_PLAY(SNDIDX_CLICK);
						UTGetGUI().ShowLayerOnce("LAYER_ID_QUITGAME");
					}
					/*
					//windows close when pressing back
					else if ((layer != null) && (layer == UTGetControlsManager().GetTopmostInputLayer()))
					{
					SND_PLAY(SNDIDX_DENIED);
					UTGetControlsManager().RemoveLayer("LAYER_ID_QUITGAME");
					}
					*/
					break;
				}
			}
		}
		break;

		case GAME_STATE_GAME:
		{
			if (!bSyncUpdate) //not networked or network sync finised even if still during gameplay
			{
				//ingame menu on ESC-back
				if (g_level.m_levelState == K_LVL_STATE_PLAYING)
				{
					CCtrlLayer* layer = UTGetGUI().GetLayerByName("LAYER_ID_IGM_MENU");
					if ((layer == null) && (!UTGetGUI().bIsBlocking))
					{
						for (UINT kk = 0; kk < UTGetCtrlrMgr().m_arrControllers.size(); kk++)
						{
							//show menu
							if (UTGetCtrlrMgr().m_arrControllers[kk]->sCommands.keyState[K_CM_COMMAND_BACK] == K_CM_BUTSTATE_JUSTPRESSED)
							{
								SND_PLAY(SNDIDX_CLICK);
								UTGetGUI().ShowLayerOnce("LAYER_ID_IGM_MENU");
								break;
							}
						}
					}
					else if ((layer != null) && (layer == UTGetGUI().GetTopmostInputLayer()))
					{
						for (UINT kk = 0; kk < UTGetCtrlrMgr().m_arrControllers.size(); kk++)
						{
							//remove onscreen menu
							if ((UTGetCtrlrMgr().m_arrControllers[kk]->sCommands.keyState[K_CM_COMMAND_BACK] == K_CM_BUTSTATE_JUSTPRESSED) ||
								(UTGetCtrlrMgr().m_arrControllers[kk]->sCommands.keyState[K_CM_COMMAND_RELOAD] == K_CM_BUTSTATE_JUSTPRESSED))
							{
								SND_PLAY(SNDIDX_DENIED);
								UTGetGUI().RemoveLayer("LAYER_ID_IGM_MENU");
								break;
							}
						}
					}
				}

				//update game if no blocking window is shown
				if (!UTGetGUI().bIsBlocking)
				{
					//SPINE update animation states
					g_spineMgr.UpdateAnimationStates(fElapsedTime, fTime);

					g_level.Update(fElapsedTime);
					//SPINE update final skeleton world positions (no bone changes allowed after this)
					g_spineMgr.Update(fElapsedTime, fTime);

					g_bLevelNeedsUpdate = false;
				}
				//#HACK: update once after resolution changed so we adjust cameras
				if (g_bLevelNeedsUpdate)
				{
					LOG_DBG(L"> Update called with dtime: 0.0");
					g_level.Update(0.0f);
					g_bLevelNeedsUpdate = false;
				}
			}
			else  //networked, syncing update
			{
				//ingame menu on ESC-back (if chat is closed)
				bool bCanOpenMenu = true;
#ifdef ENABLE_CHAT_WINDOW
				//because the chat window exits immediately we have to wait a little until we can bring the menu up
				if ((g_ChatWnd.IsReceivingInput()) || (g_ChatWnd.fTimeSinceLastInput < K_CW_MIN_TIME_BETWEEN_INPUTS))
					bCanOpenMenu = false;
#endif
				if (bCanOpenMenu)
				{
					//ingame menu on ESC-back
					if (g_level.m_levelState == K_LVL_STATE_PLAYING)
					{
						CCtrlLayer* layer = UTGetGUI().GetLayerByName("LAYER_ID_IGM_MENU_NET");
						if ((layer == null) && (!UTGetGUI().bIsBlocking))
						{
							for (UINT kk = 0; kk < UTGetCtrlrMgr().m_arrControllers.size(); kk++)
							{
								CController* ctrlr = UTGetCtrlrMgr().m_arrControllers[kk];
								//ignore network controllers
								if (ctrlr->eType == K_CM_CT_NET_FRAMELOCK)
									continue;
								//show menu
								if (ctrlr->sCommands.keyState[K_CM_COMMAND_BACK] == K_CM_BUTSTATE_JUSTPRESSED)
								{
									SND_PLAY(SNDIDX_CLICK);
									UTGetGUI().ShowLayerOnce("LAYER_ID_IGM_MENU_NET");
									break;
								}
							}
						}
						else if ((layer != null) && (layer == UTGetGUI().GetTopmostInputLayer()) && (layer->alpha >= 1.0f))
						{
							for (UINT kk = 0; kk < UTGetCtrlrMgr().m_arrControllers.size(); kk++)
							{
								CController* ctrlr = UTGetCtrlrMgr().m_arrControllers[kk];
								//ignore network controllers
								if (ctrlr->eType == K_CM_CT_NET_FRAMELOCK)
									continue;
								//remove onscreen menu
								if ((ctrlr->sCommands.keyState[K_CM_COMMAND_BACK] == K_CM_BUTSTATE_JUSTPRESSED) ||
									(ctrlr->sCommands.keyState[K_CM_COMMAND_RELOAD] == K_CM_BUTSTATE_JUSTPRESSED))
								{
									SND_PLAY(SNDIDX_DENIED);
									UTGetGUI().RemoveLayer("LAYER_ID_IGM_MENU_NET");
									break;
								}
							}
						}
					}
				}
				//sync random seed again here (makes sure we don't get desynced between debug and release versions)
				//resets the number of random numbers requested
				g_level.m_rand.SetRandomSeed(g_netlock.m_unRandomSeed + g_nUpdateFrame);
				//LOG(L"--update dT=%.6f T=%.6f rand:%d--", fElapsedTime, fTime, g_level.m_rand.GetRandomSeed());

				//SPINE update animation states
				g_spineMgr.UpdateAnimationStates(fElapsedTime, fTime);

				g_level.Update(fElapsedTime);
				//SPINE update animation states
				g_spineMgr.UpdateAnimationStates(fElapsedTime, fTime);
				g_bLevelNeedsUpdate = false;

				//check sync by log
				/*
				double faccum = 0.0f;
				for (int ll = 0; ll < g_level.m_arrActors.GetSize(); ll++)
				{
				faccum += g_level.m_arrActors[ll]->pos.x;
				faccum += g_level.m_arrActors[ll]->pos.y;
				}
				LOG(L"-- frame %d faccum %.9g", g_nUpdateFrame, faccum);
				*/
			}

			g_editor.Update(fElapsedTime);
		}
		break;

#ifdef K_CONTROLS_EDITOR
		case GAME_STATE_CONTROLSED:
			g_ControlsEditor.SetCameraTransform(&UTGetAppClass().g_cam240hScreen);
			g_ControlsEditor.Update(fElapsedTime);
			break;
#endif

	}

	///--- ANALYTICS ---
	UTGetAnalytics().Update();

	///--- SCRIPTS UPDATE ---
	UTGetScriptManager().Update(fElapsedTime);
	///--- EVENTS UPDATE ---
	UTGetEventManager().Update(fElapsedTime, fTime);
}


// Checks if all conditions are met for simulating a frame
bool AllowCoopUpdateCheck(bool bSyncUpdate, int nFrame)
{
	if (!bSyncUpdate)
		return true;

	if (nFrame < 0)
	{
		LOG(L"[WARNING] Update check called on nFrame < 0 !");
		return false;
	}

	bool bUpdateGo = true;
	// Make sure we have all necessary input data to simulate next frame
	if (bSyncUpdate)
	{
		//check to see if we have any data
		if (g_netlock.m_nReceived_Tail < 0)
		{
			bUpdateGo = false;
		}
		//check peer
		if (g_netlock.m_arrReceived[nFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].m_nFrame != nFrame)
		{
			bUpdateGo = false;
			//LOG_DBG(L"Peer data not found! updFrame:%d sendFrameNr:%d", nFrame, g_netlock.m_arrReceived[nFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].m_nFrame);
			if (g_netlock.m_nToSend_Head > nFrame)
			{
				LOG(L"[WARNING] Net::Error - Package drop/loss detected. Missing peer frame %d data but received more recent data!", nFrame);
#ifdef K_NET_ENGINE_DBG_VERBOSE
				LOG_DBG_BUFF(L"sndBuff[H:%d T:%d] lastSync:%d rcvBuff[H:%d T:%d] rcvSync:%d",
					g_netlock.m_nToSend_Head, g_netlock.m_nToSend_Tail, g_nLastSyncedFrame,
					g_netlock.m_nReceived_Head, g_netlock.m_nReceived_Tail, g_netlock.m_nReceived_SyncFrame);
#endif
			}
		}
		//check local
		if (g_netlock.m_arrToSend[nFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].m_nFrame != nFrame)
		{
			bUpdateGo = false;
			//LOG_DBG(L"Local data not found! updFrame:%d sendFrameNr:%d", nFrame, g_netlock.m_arrReceived[nFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].m_nFrame);
		}
	}
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	if (bUpdateGo)
	{
		UINT32 unButMasks_peer = 0;
		UINT32 unButMasks_local = 0;
		for (int kk = 0; kk < K_CM_COMMANDS_COUNT; kk++)
		{
			if (g_netlock.m_arrReceived[nFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].m_bButStates[kk])
				unButMasks_peer |= (1 << kk);
			if (g_netlock.m_arrToSend[nFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].m_bButStates[kk])
				unButMasks_local |= (1 << kk);
		}

		//UINT32 unLocalSync = g_netlock.m_arrToSend[nFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].m_dwSyncCheck;
		//UINT32 unPeerSync = g_netlock.m_arrReceived[nFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].m_dwSyncCheck;
		//LOG(L"Sim frame %d local[%d,%d] - peer[%d,%d]", nFrame, unButMasks_local, unLocalSync, unButMasks_peer, unPeerSync);
	}
#endif

	return bUpdateGo;
}


void CALLBACK OnFrameMove(PDEVICE pDevice, double fTime, float fElapsedTime_original)
{
	///--- Set float rounding mode for online play (framesync) ---
#ifdef WIN32
	_controlfp(_PC_24, _MCW_PC);
	_controlfp(_RC_NEAR, _MCW_RC);
	//_set_SSE2_enable(0); // must also disable /Oi (Generate Intrinsic Functions) and set /arch:IA32
#elif defined(__linux__)
	fpu_control_t _oldcw, _cw;
	_FPU_GETCW(_oldcw); // store old cw
	_cw = (_oldcw & ~_FPU_EXTENDED & ~_FPU_DOUBLE & ~_FPU_SINGLE) | _FPU_SINGLE;
	_FPU_SETCW(_cw);
#elif (defined(__APPLE__) && !TARGET_OS_IPHONE)
	unsigned int _cw = 127;
	asm("fnclex");
	asm("fldcw %0" : : "m" (*&_cw));
	fesetround(FE_TONEAREST);
#endif (edited)


#if defined(ENABLE_STEAM)
	SteamAPI_RunCallbacks();
#endif

#ifdef ENABLE_GALAXY
	galaxy::api::ProcessData();
#endif // ENABLE_GALAXY


	//--- limit timestep on 24fps when fElapsedTime is too large ---
	float fElapsedTime = fElapsedTime_original;
	if ((fElapsedTime > K_MAX_TIMESTEP) || (fElapsedTime < 0.0f))
		fElapsedTime = K_MAX_TIMESTEP;

	//update achievements and stats
	UTGetAchievementManager().Update(fElapsedTime);

	if (!pDevice)
	{
		return;
	}

#if defined(ENABLE_FIXED_TIMESTEP_SINGLEPLAYER)
	// Makes it update at a fixed 60FPS
	bool bAllowFullspeedUpdate = false;
#else
	// Lets it run at fullspeed FPS on local games
	bool bAllowFullspeedUpdate = true;
	if (UTGetAppClass().IsGameNetworked())
		bAllowFullspeedUpdate = false;
#endif

	///--- networked game requested? reset sync data ---
	if ((UTGetAppClass().IsGameNetworked()) &&
		(UTGetAppClass().m_Settings.devnet_eSyncStatus == CApplicationSettings::K_NETGAME_SYNC_GET_READY))
	{
		UTGetAppClass().m_Settings.devnet_eSyncStatus = CApplicationSettings::K_NETGAME_SYNC_SYNCING;

		g_nUpdateFrame = 0;
		g_nInputFrame = 0;
		g_nLastSyncedFrame = -1;
		
		g_nLastSyncHash = 0;

		g_fLastUpdateTimer = 0.0f;
		//time accumulators
		g_fTimeAccumInput = 0.0f; 
		g_fTimeAccumUpdate = 0.0f;
		g_fTimeAccumSend = 0.0f;
		//reset everything for netlock step
		g_netlock.Net_EnterNetlock();
		//reset level results stuff too
		g_netlock.Net_ResetLevelResults();
	}
	//easy access
	bool bSyncUpdate = (UTGetAppClass().m_Settings.devnet_eSyncStatus == CApplicationSettings::K_NETGAME_SYNC_SYNCING);

	if (DXUTIsTimePaused())
	{
		//game is networked => don't pause it
		if (!bSyncUpdate)
			return;
	}

	///--- always read events and update network ---
	g_netlock.Net_UpdateEventLoop();
	// some net checks
	if (bSyncUpdate)
	{
		//update and query current lobby status
		const INetwork::sLobby& lobby = g_pNetwork->GetCurrentLobby();
		if (lobby.eState != INetwork::LOBBY_IN_LOBBY || lobby.iNumPlayers != 2)
		{
			//lobby problems? don't continue
			LOG(L"Net::Error - FrameMove Sync(ON) lobby error. Back to menu!");

			//change game state
			CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE);
			nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
			nevent->AddNamedArgINT32(L"stateErrorStrIdx", STR_NETWORK_ERROR_PLAYER_LEFT);
			UTGetEventManager().TriggerEvent(nevent);

			return;
		}
		//check net lock (exit if detected)
#if defined(K_NET_CHECK_IF_NETSYNC_LOCKED)
		g_fLastUpdateTimer += fElapsedTime; // Update coop watchdog timer

		if (g_fLastUpdateTimer > K_NET_CHECK_IF_NETSYNC_LOCKED_DURATION)
		{
			LOG(L"Net Failsafe: Network locked up or one player paused the game! fLastUpdateTimer:%.4f", g_fLastUpdateTimer);
			//send analytics
			CHAR ctxt[MAX_PATH];
			StringCchPrintfA(ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1);
			ANALYTICS_EVENT("net_locked_up", ctxt, "net_lock_timer", 0);

			//change game state
			CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE);
			nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
			nevent->AddNamedArgINT32(L"stateErrorStrIdx", STR_NETWORK_ERROR_GENERIC);
			UTGetEventManager().TriggerEvent(nevent);

			return;
		}
#endif
		///--- LAST: make sure we stop syncing/sending frame data only after peer finished level too (both peers agree) ---
		if (g_level.m_levelState > K_LVL_STATE_PLAYING)
		{
			if ((g_netlock.m_arrLvlResPeerStates[0] != CNetLock::sPacketLevelResults::K_LEVRES_STATE_UNDEFINED) &&
				(g_netlock.m_arrLvlResPeerStates[1] != CNetLock::sPacketLevelResults::K_LEVRES_STATE_UNDEFINED))
			{
				UTGetAppClass().m_Settings.devnet_eSyncStatus = CApplicationSettings::K_NETGAME_SYNC_STOPPED;
				LOG(L"COOP:: Peers agree on level finished! Ended network Sync!");
				//don't exit, let him send pending messages
				bSyncUpdate = false;
			}
		}

	}

	///--- LISTEN FOR LATEST NETWORK DATA ---
	if (bSyncUpdate)
	{
		g_netlock.m_fTimeSinceLastRCV += fElapsedTime_original;
		while (g_netlock.Net_ReceiveFrameData(g_nLastSyncedFrame))
		{
		}
	}

	g_fTimeAccumInput += fElapsedTime;
	g_fTimeAccumUpdate += fElapsedTime;
	g_fTimeAccumSend += fElapsedTime;

	double l_fPeriodInput = K_FIXED_TIMESTEP_DTIME;
	double l_fPeriodUpdate = K_FIXED_TIMESTEP_DTIME;

	if (bAllowFullspeedUpdate)
	{
		l_fPeriodInput = fElapsedTime;
		l_fPeriodUpdate = fElapsedTime;
	}

	//frequency of data sending
	double l_fPeriodSend = K_FIXED_TIMESTEP_DTIME;
#ifdef K_NET_FORCE_30FPS_SENDING
	l_fPeriodSend = K_FIXED_TIMESTEP_DTIME * 2.0f;
#endif
	// Allow input acquiring
	bool bInputGo = true;
	// Allow input consuming (sim update)
	bool bUpdateGo = true;

	///--- 1. INPUT ACQ ---
	if (bSyncUpdate)
	{
		// Wait for other player to load the level
		if ((g_netlock.m_nPlayerFlags[g_netlock.Net_GetOtherPlayerIndex()] & K_NETLOCK_PLAYERFLAG_STARTED_LEVEL) == 0)
		{
			bInputGo = false;
			//reset accumulator so it doesn't do a ot of updates at once
			g_fTimeAccumInput = 0.0f;
			g_fTimeAccumUpdate = 0.0f;
			LOG_DBG(L"[INFO]Netlock:: Waiting for peer to load the level!");
		}
		// balance tests
		const int K_COOP_FRAMES_BEFORE_WAIT = 5; //default 5
		//ideal value for computing frames ahead: g_nInputFrame - g_netlock.m_nReceived_Tail
		int nFramesAhead = g_nInputFrame - g_netlock.m_nReceived_Tail;
		int nLagFrames = g_nLastSyncedFrame - g_netlock.m_nReceived_SyncFrame;
		// Take lag into consideration or they'll both start waiting for each other when lag is big
		//int nFramesOff = nFramesAhead - nLagFrames;
		int nFramesOff = SIGN(nFramesAhead) * max(0, (abs(nFramesAhead) - abs(nLagFrames)));  //<-- better
		//int nFramesOff = SIGN(nFramesAhead) * (abs(nFramesAhead) - abs(nLagFrames));  //<-- original, not good

		// WAIT net peer (we went ahead with the sim)
		if (nFramesOff > K_COOP_FRAMES_BEFORE_WAIT)
		{
			float fWaitPerc = nFramesOff - K_COOP_FRAMES_BEFORE_WAIT;
			//scale waiting timer over a few frames
			fWaitPerc *= 0.1f; //lower speed if N frames over the K_COOP_FRAMES_BEFORE_WAIT frames limit (default 0.25 but 0.2 works well too)
			CLAMP(fWaitPerc, 0.0f, 1.0f); 
			float fWaitTime = fWaitPerc * K_FIXED_TIMESTEP_DTIME;
			//slow down input acquiring
			l_fPeriodInput += fWaitTime;

#ifdef K_NET_ENGINE_DBG_VERBOSE
			LOG_DBG_BUFF(L"[INFO] NET WAIT (fWaitPerc:%.1f): fInp:%.4f fUpd:%.4f frames-off:%d. ", fWaitPerc, l_fPeriodInput, l_fPeriodUpdate, nFramesOff);
			LOG_DBG_BUFF(L"sndBuff[H:%d T:%d] lastSync:%d rcvBuff[H:%d T:%d] rcvSync:%d framesAhead:%d lagFrames:%d",
				g_netlock.m_nToSend_Head, g_netlock.m_nToSend_Tail, g_nLastSyncedFrame,
				g_netlock.m_nReceived_Head, g_netlock.m_nReceived_Tail, g_netlock.m_nReceived_SyncFrame,
				nFramesAhead, nLagFrames);
#endif
		}

		//catch up to consume buffer
		const int K_COOP_FRAMES_BEFORE_CATCHUP = 2; 
		int nBuffSzLocal = g_netlock.m_nToSend_Tail - g_nLastSyncedFrame;
		int nBuffSzRemote = g_netlock.m_nReceived_Tail - g_nLastSyncedFrame;
		int nBuffAvailable = min(nBuffSzLocal, nBuffSzRemote);
		if (nBuffAvailable > K_COOP_FRAMES_BEFORE_CATCHUP)
		{
			float fAccelPerc = (nBuffAvailable - K_COOP_FRAMES_BEFORE_CATCHUP) * 0.1f;
			CLAMP(fAccelPerc, 0.0f, 0.5f); //should never reach 1.0f so we always update a little (default 0.9 but seemed too large, 0.5 was ok)
			float fAccelTime = fAccelPerc * K_FIXED_TIMESTEP_DTIME;
			//slow down input acquiring (don't go negative)
			l_fPeriodUpdate -= fAccelTime;
			if (l_fPeriodUpdate < 0.4f * K_FIXED_TIMESTEP_DTIME)
				l_fPeriodUpdate = 0.4f * K_FIXED_TIMESTEP_DTIME;

#ifdef K_NET_ENGINE_DBG_VERBOSE
			LOG_DBG_BUFF(L"[INFO] NET CATCH UP: fInp:%.4f fUpd:%.4f frames-off:%d. ", l_fPeriodInput, l_fPeriodUpdate, nFramesOff);
			LOG_DBG_BUFF(L"sndBuff[H:%d T:%d] lastSync:%d rcvBuff[H:%d T:%d] rcvSync:%d lagFrames:%d",
				g_netlock.m_nToSend_Head, g_netlock.m_nToSend_Tail, g_nLastSyncedFrame,
				g_netlock.m_nReceived_Head, g_netlock.m_nReceived_Tail, g_netlock.m_nReceived_SyncFrame,
				nLagFrames);
#endif
		}

		// if we didn't hear from the other side don't fill the buffer yet (level start)
		if ((nFramesOff > K_COOP_FRAMES_BEFORE_WAIT * 2) && (g_netlock.m_nReceived_SyncFrame < 0))
		{
			bInputGo = false;
			//reset accumulator so it doesn't do a lot of updates at once
			g_fTimeAccumInput = 0.0f;
			g_fTimeAccumUpdate = 0.0f;
			LOG("[INFO] INPUT STOP: Too far ahead with no answer at all! frames off: %d", nFramesOff);
		}

		if (nFramesOff >= CNetLock::K_NETLOCK_MAX_INPUT_FRAMES_AHEAD)
		{
			bInputGo = false;
			//bUpdateGo = false;
			//reset accumulator so it doesn't do a ot of updates at once
			g_fTimeAccumInput = 0.0f;
			LOG("[INFO] INPUT STOP: Too far ahead! frames off: %d", nFramesOff);
		}

		///--- poll input at fixed intervals ---
		while ((g_fTimeAccumInput >= l_fPeriodInput) && (bInputGo))
		{
#if defined(K_NET_ENGINE_DBG_VERBOSE) || defined(K_SYNC_ENGINE_DBG_VERBOSE)
			LOG_DBG_BUFF(L"$--> upd: %d fTimeAccum:%.6f", g_nInputFrame, g_fTimeAccumInput);
#endif

			//read SDL ctrlrs only when saving controllers snapshot or it will read from the controllers at different freqs
			//later we force net values over actual controller
			UTGetAppClass().PollSDLControllers();

			//save actual controller states (not commands because those are secundary byproducts)
			float arrKeysDown[K_CM_COMMANDS_COUNT] = { 0.0f };

			int nInstanceLocal = g_level.m_arrPlayerControllersIIDs[g_netlock.Net_GetPlayerIndex()];
			CController* ctrlr = UTGetCtrlrMgr().GetControllerByInstanceID(nInstanceLocal);
			if (ctrlr != null)
				ctrlr->GetKeysDownPercents(arrKeysDown);
			///write controller data into net package
			g_netlock.m_arrToSend[g_nInputFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].SaveButtonsPressedPercents(g_nInputFrame, arrKeysDown);

			///save other data about the current frame
			WORD wFrameFlag = 0;
			//blocking interface shown so block controller input (includes ingame menu and level finished windows)
			if (UTGetGUI().GetTopmostInputLayer() != null)
				wFrameFlag |= K_NETLOCK_FRAMEFLAG_INPUT_PAUSED_INGAME;
			//chat window open, block local controller
#ifdef ENABLE_CHAT_WINDOW
			if (g_ChatWnd.IsReceivingInput())
				wFrameFlag |= K_NETLOCK_FRAMEFLAG_INPUT_PAUSED_INGAME;
#endif
			//save the flags
			g_netlock.m_arrToSend[g_nInputFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].m_wFrameFlags = wFrameFlag;

			// ALWAYS write new data even if too far ahead
			g_netlock.m_nToSend_Tail = g_nInputFrame;
			//input was read - mark next input frame
			g_nInputFrame++;
			//clear sync detection ahead
			g_netlock.m_arrToSend[g_nInputFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].m_dwSyncCheck = 0;

			//dump local ToSend packs if confirmed
			//ACK packages with frames lower than the largest received one but keeps at least one message in the queue (the most recent one)
			if (g_netlock.m_nReceived_SyncFrame >= 0)
				g_netlock.m_nToSend_Head = min(g_nUpdateFrame, g_netlock.m_nReceived_SyncFrame);

			if (g_netlock.m_nToSend_Head < 0)
				g_netlock.m_nToSend_Head = 0;
			//if lag is too big just discard oldest packages
			if ((g_netlock.m_nToSend_Tail - g_netlock.m_nToSend_Head > CNetLock::K_NETLOCK_MAX_STATE_PACKAGES - CNetLock::K_NETLOCK_PACKAGE_CURFRAME_SAFEGUARD))
			{
				g_netlock.m_nToSend_Head = g_netlock.m_nToSend_Tail - CNetLock::K_NETLOCK_MAX_STATE_PACKAGES + CNetLock::K_NETLOCK_PACKAGE_CURFRAME_SAFEGUARD;

#ifdef K_NET_ENGINE_DBG_VERBOSE
				LOG_DBG_BUFF(L"[WARNING]Netlock:: Lag too big! Dumping oldest unconfirmed packages!");
				LOG_DBG_BUFF(L"sndBuff[H:%d T:%d] lastSync:%d rcvBuff[H:%d T:%d] rcvSync:%d lagFrames:%d",
					g_netlock.m_nToSend_Head, g_netlock.m_nToSend_Tail, g_nLastSyncedFrame,
					g_netlock.m_nReceived_Head, g_netlock.m_nReceived_Tail, g_netlock.m_nReceived_SyncFrame,
					nLagFrames);
#endif
			}

			//move tail if head advanced too much
			if (g_netlock.m_nToSend_Tail < g_netlock.m_nToSend_Head)
				g_netlock.m_nToSend_Tail = g_netlock.m_nToSend_Head;

			///--- FINISH UP ---
			g_fTimeAccumInput -= l_fPeriodInput;
			if (bAllowFullspeedUpdate)
				g_fTimeAccumInput = 0.0f;
			// brute force it to one update per frame
			if (g_bForceOneUpdatePerFrame)
				g_fTimeAccumInput = 0.0f;
		}

		///--- SEND NETWORK CONTROLS DATA ---
		// SEND NETWORK PACKAGE at fixed intervals
		if (g_fTimeAccumSend >= l_fPeriodSend)
		{
			// only after recording at least one input
			if (g_nInputFrame > 0)
			{
				//write sync hash for synced frame after we sync a little to avoid sync=-1
				if(g_nLastSyncedFrame >= 0)
					g_netlock.m_arrToSend[g_nLastSyncedFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].m_dwSyncCheck = g_nLastSyncHash;
				//and send data
				g_netlock.Net_SendFrameData(g_nLastSyncedFrame, g_nLastSyncHash);
			}

			g_fTimeAccumSend -= l_fPeriodSend;
		}

		//force fixed timestep on net synced gameplay
		fElapsedTime = K_FIXED_TIMESTEP_DTIME;
	}
	else //if(bSyncUpdate)
	{
		while (g_fTimeAccumInput >= l_fPeriodInput)
		{
			//read SDL controllers data as often as possible
			UTGetAppClass().PollSDLControllers();
			//accumulator update
			g_fTimeAccumInput -= l_fPeriodInput;
			// brute force it to one update per frame
			if (g_bForceOneUpdatePerFrame)
				g_fTimeAccumInput = 0.0f;
		}

		//force fixed timestep on net synced gameplay
		if (!bAllowFullspeedUpdate)
			fElapsedTime = K_FIXED_TIMESTEP_DTIME;
	}

	///--- 2. UPDATE BY TIME ---
	// Make sure we have all necessary input data to simulate next frame
	bUpdateGo = AllowCoopUpdateCheck(bSyncUpdate, g_nUpdateFrame);

	while ((g_fTimeAccumUpdate >= l_fPeriodUpdate) && (bUpdateGo))
	{
		///--- Write data in controller structures and update the other local controllers ---
		if (bSyncUpdate)
		{
#if defined(K_NET_ENGINE_DBG_VERBOSE) || defined(K_SYNC_ENGINE_DBG_VERBOSE)
			LOG_DBG_BUFF(L"#--> sim: %d fTimeAccum:%.6f", g_nUpdateFrame, g_fTimeAccumUpdate);
#endif
			///--- Check to see if it gets desynced ---
			// Because random data is written after an update (not when writing input) we must make sure we received it first (and check back)
			DWORD rndpeer = 0;
			DWORD rndlocal = 0;
			if (g_nLastSyncedFrame >= 0)
			{
				rndpeer = g_netlock.m_arrReceived[g_nLastSyncedFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].m_dwSyncCheck;
				rndlocal = g_netlock.m_arrToSend[g_nLastSyncedFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].m_dwSyncCheck;
			}
			// don't check for desync if we're ahead (might get old value from buffer -> false positive)
			// with this logic only one of the peers (the one that's behind) will be able to test for desync but that's enough
			if ((rndlocal != 0) && (rndpeer != 0) && (g_nLastSyncedFrame <= g_netlock.m_nReceived_SyncFrame) && (g_nLastSyncedFrame >= 0))
			{
				//we happen to have data about the sync hash (not always)
				if (rndpeer != rndlocal)
				{
					LOG(L"[WARNING] Game:: Clients desynced on frame %d values peer(%d) local(%d)!", g_nLastSyncedFrame, rndpeer, rndlocal);

#if defined(K_NET_DISCONNECT_ON_DESYNC)
					g_netlock.Net_LogFrameData(10);

					LOG(L"-- scene actors %d --", g_level.m_arrActors.GetSize());
					for (int ll = 0; ll < g_level.m_arrActors.GetSize(); ll++)
					{
						CActor* act = g_level.m_arrActors[ll];
						LOG(L"%s ID %d pos(%.4f, %.4f) decision(%.4f)", act->templateActor.shName.text, act->ID, act->pos.x, act->pos.y, act->AItimerDecision);
					}

					//send analytics
					CHAR ctxt[MAX_PATH], ctxt2[MAX_PATH];
					StringCchPrintfA(ctxt, MAX_PATH, "mission_%d_%d_ver_%s", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1, _VERSION_CHARSTR_);
					StringCchPrintfA(ctxt2, MAX_PATH, "v%d_CRC[%08x]", _VERSION_INT_, UTGetAppClass().m_Settings.dev_unCurrentCRC);
					ANALYTICS_EVENT("net_desync", ctxt, ctxt2, 0);

					//quit lobby
					g_netlock.Net_QuitLobby();

					//change game state
					CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE);
					nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
					nevent->AddNamedArgINT32(L"stateErrorStrIdx", STR_NETWORK_ERROR_GENERIC);
					UTGetEventManager().QueueEvent(nevent);
#endif
				}
			}
			// Write controllers data from network
			int nInstanceLocal = g_level.m_arrPlayerControllersIIDs[g_netlock.Net_GetPlayerIndex()];
			int nInstancePeer = g_level.m_arrPlayerControllersIIDs[g_netlock.Net_GetOtherPlayerIndex()];

			CController* ctrlr_local = null;
			ctrlr_local = UTGetCtrlrMgr().GetControllerByInstanceID(nInstanceLocal);
			CController* ctrlr_peer = null;
			ctrlr_peer = UTGetCtrlrMgr().GetControllerByInstanceID(nInstancePeer);

			//save local buttons states
			float arrStateLocal[K_CM_COMMANDS_COUNT] = { 0.0f };
			g_netlock.m_arrToSend[g_nUpdateFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].GetButtonsPressedPercents(arrStateLocal);
			WORD wFrameFlagsLocal = g_netlock.m_arrToSend[g_nUpdateFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].m_wFrameFlags;
			//peer buttons states
			float arrStatePeer[K_CM_COMMANDS_COUNT] = { 0.0f };
			g_netlock.m_arrReceived[g_nUpdateFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].GetButtonsPressedPercents(arrStatePeer);
			WORD wFrameFlagsPeer = g_netlock.m_arrReceived[g_nUpdateFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].m_wFrameFlags;

			//update all controllers with internal data but used ones with network data
			for (UINT ll = 0; ll < UTGetCtrlrMgr().m_arrControllers.size(); ll++)
			{
				CController* ctrlr = UTGetCtrlrMgr().m_arrControllers[ll];
				//update local controller with net data only when not in menus
				if (ctrlr == ctrlr_local)
				{
					// update controller overriding keypresses with what we registered before
					UTGetCtrlrMgr().UpdateController(ctrlr, fElapsedTime, arrStateLocal);
					//set paused if needed
					if (wFrameFlagsLocal & K_NETLOCK_FRAMEFLAG_INPUT_PAUSED_INGAME)
						ctrlr->nFlags |= K_CM_CTRLR_FLAG_PAUSED;
					else
						ctrlr->nFlags &= ~K_CM_CTRLR_FLAG_PAUSED;
				}
				else if (ctrlr == ctrlr_peer)
				{
					// update controller overriding keypresses with what we received
					UTGetCtrlrMgr().UpdateController(ctrlr, fElapsedTime, arrStateLocal);
					//set paused if needed
					if (wFrameFlagsPeer & K_NETLOCK_FRAMEFLAG_INPUT_PAUSED_INGAME)
						ctrlr->nFlags |= K_CM_CTRLR_FLAG_PAUSED;
					else
						ctrlr->nFlags &= ~K_CM_CTRLR_FLAG_PAUSED;
				}
				else //all the other non synced controllers get updated the usual way
				{
					UTGetCtrlrMgr().UpdateController(ctrlr, fElapsedTime);
				}
			}
		}
		else  //if(bSync)
		{
			//update all controllers with internal data
			for (UINT ll = 0; ll < UTGetCtrlrMgr().m_arrControllers.size(); ll++)
			{
				UTGetCtrlrMgr().UpdateController(UTGetCtrlrMgr().m_arrControllers[ll], fElapsedTime);
			}
		}

		///--- UPDATE THE GAME ---
		UpdateGame(pDevice, fElapsedTime, fTime, bSyncUpdate);

		//!!! make sure we're still syncing the update(net state can change on level finished)
		bSyncUpdate = (UTGetAppClass().m_Settings.devnet_eSyncStatus == CApplicationSettings::K_NETGAME_SYNC_SYNCING);

		///--- INCREASE UPDATE FRAME COUNTER ---
		if (bSyncUpdate)
		{
			//mark frame as executed (clean entry)
			g_netlock.m_arrReceived[g_nUpdateFrame % CNetLock::K_NETLOCK_MAX_STATE_PACKAGES].m_nFrame = -1;
			//frame was simulated
			g_nLastSyncedFrame = g_nUpdateFrame;
			//compute sync check for this frame
			g_nLastSyncHash = g_level.m_dwSyncCheckHash + g_level.m_rand.GetRandomCallsCount();
		}

		g_nUpdateFrame++;
		//reset update timer (watchdog)
		g_fLastUpdateTimer = 0.0f;

		//--- LAST CALL ---
		UpdateTransition(fElapsedTime);

		///--- FINISH UP ---
		g_fTimeAccumUpdate -= l_fPeriodUpdate;
		if (bAllowFullspeedUpdate)
			g_fTimeAccumUpdate = 0.0f;

		// keep buffer to smooth out play
		// we don't have buffer but still have to simulate, reset the timer so we try again later
#ifdef K_NET_FORCE_30FPS_SENDING
		if (bSyncUpdate)
		{
			int nBuffSzLocal = g_netlock.m_nToSend_Tail - g_nLastSyncedFrame;
			int nBuffSzRemote = g_netlock.m_nReceived_Tail - g_nLastSyncedFrame;
			int nBuffAvailable = min(nBuffSzLocal, nBuffSzRemote);
			//make sure we leave a value in the buffer to handle the jitter
			if (nBuffAvailable - 1 <= 0)
				g_fTimeAccumUpdate = 0.0f;
		}
#endif
		///--- Check again if we can simulate next frame (this is inside a while) ---
		bUpdateGo = AllowCoopUpdateCheck(bSyncUpdate, g_nUpdateFrame);

		// brute force it to one update per frame
		if (g_bForceOneUpdatePerFrame)
			g_fTimeAccumUpdate = 0.0f;
	}

	///--- CHAT WINDOW ---
#ifdef ENABLE_CHAT_WINDOW
	g_ChatWnd.Update(fElapsedTime);
#endif

#ifdef ENABLE_LEADERBOARDS
	ELBJobStatus eJobStat = UTGetLeaderboards().Update(fElapsedTime);
	//after each finished job try and write the leaderboards strings
	if (eJobStat == K_JOBSTATUS_JUST_FINISHED)
	{
		// save scores to strings
		CScoresList scoresList;
		int nScores = UTGetLeaderboards().GetDownloadedScores(&scoresList);
		if (nScores > 0)
		{
			WCHAR strNames[ 2048 ] = { 0 };
			WCHAR strScores[ 2048 ] = { 0 };
			WCHAR strLine[MAX_PATH];
			for (int kk = 0; kk < nScores; kk++)
			{
				//name and rank
				CStringDesc sdName;
				g_stringsMgr.SetStringDescUTF8(&sdName, scoresList.m_arrNames[kk]);
				StringCchPrintf(strLine, MAX_PATH, L"%d.%s\n", scoresList.m_arrRank[kk], sdName.sText);
				//append
				StringCchCat(strNames, 1024, strLine);

				//score
				StringCchPrintf(strLine, MAX_PATH, L"%d\n", scoresList.m_arrScores[kk]);
				//append
				StringCchCat(strScores, 1024, strLine);
			}
			//hack: append some empty chars or alignment fails
			StringCchCat(strNames, 1024, L" ");
			StringCchCat(strScores, 1024, L" ");
			//set final strings
			g_stringsMgr.SetString(STR_LEADERBOARDS_NAMES_VAL, strNames);
			g_stringsMgr.SetString(STR_LEADERBOARDS_SCORES_VAL, strScores);

			//save user score
			int nUserScore = UTGetLeaderboards().GetUserScore();
			if(nUserScore == 0)
				g_stringsMgr.SetString(STR_LEADERBOARDS_PLAYERSCORE_VAL, g_stringsMgr.strings[STR_NOT_AVAILABLE]->sText);
			else
				g_stringsMgr.SetString(STR_LEADERBOARDS_PLAYERSCORE_VAL, L"%d", nUserScore);
		}
		else
		{
			//no scores
			g_stringsMgr.SetString(STR_LEADERBOARDS_NAMES_VAL, g_stringsMgr.strings[STR_NOT_AVAILABLE]->sText);
			g_stringsMgr.SetString(STR_LEADERBOARDS_SCORES_VAL, g_stringsMgr.strings[STR_NOT_AVAILABLE]->sText);
		}

		// update the number of selectable items in the leaderboards window
		CCtrlLayer* pLay = UTGetGUI().GetTopmostLayer();
		if (pLay != null)
		{
			CControl* ctrl = pLay->GetControlByName("CTRL_SCORESLIST_TT");
			if (ctrl != null)
			{
				int nPlIdx = UTGetLeaderboards().GetDownloadedScores_PlayerIndex();
				ctrl->paramsDict.SetNamedVarINT32(L"nOptionsCnt", nScores);
				//set selection on valid item if we are allowed to select
				bool bUserCanSelect = ctrl->paramsDict.GetVariantByName(L"bUserCanSelect")->m_asBool;
				int nSelIdx = ctrl->paramsDict.GetVariantByName(L"nSelectedIdx")->m_asINT32;
				if (bUserCanSelect)
				{
					if ((nScores > 0) && (nSelIdx < 0))
						ctrl->paramsDict.SetNamedVarINT32(L"nSelectedIdx", max(0, nPlIdx));
				}
				else
				{
					if(nPlIdx >= 0)
						ctrl->paramsDict.SetNamedVarINT32(L"nSelectedIdx", nPlIdx);
				}
				//re-enable the control (was disabled while asking for the scores so you can't scroll)
				ctrl->bDisabled = false;
			}
		}
	}
#endif

	//show error box if coming from other states
	if ((!g_bDuringTransition) && (g_gameStateErrorStringIdx >= 0))
	{
		//save string idx
		int stridx = g_gameStateErrorStringIdx;
		//reset error string
		g_gameStateErrorStringIdx = -1;
		//show window after reset
		UTGetGUI().MessageBoxOK(STR_OOPS, stridx);
	}



	///--- check Float rounding mode wasn't changed ---
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	#ifdef WIN32
		assert((_controlfp(0, 0) & _MCW_PC) == _PC_24);
		assert((_controlfp(0, 0) & _MCW_RC) == _RC_NEAR);
	#elif defined(__linux__)
		_FPU_GETCW(_oldcw);
		assert(_oldcw == _cw);
	#elif (defined(__APPLE__) && !TARGET_OS_IPHONE)
		// get flags
		unsigned int _oldcw = 0;
		asm("fnstcw %0" : "=m" (*&_oldcw));
		assert(_oldcw == _cw);
		assert(fegetround() == FE_TONEAREST);
	#endif
#endif
}


//**************************************************************************************
// This callback function will be called at the end of every frame to perform all the 
// rendering calls for the scene, and it will also be called if the window needs to be 
// repainted.
//**************************************************************************************
void CALLBACK OnFrameRender(PDEVICE pDevice, double fTime, float fElapsedTime)
{
	if(!pDevice)
	{
		return;
	}

	HRESULT hr;
	MatA16 mView;
	MatA16 mProj;
	MatA16 mWorldView;
	MatA16 mWorldViewProjection;


	///PART1. Here it paints the offscreen surfaces
	switch (g_gameState)
	{
		case GAME_STATE_GAME:
		{
			//game is networked? Don't paint until we sync one frame (fixes bug that showed a frame from last coop game)
			if ((UTGetAppClass().IsGameNetworked()) && (g_nLastSyncedFrame <= 1))
			{
				break;
			}

			//build normal maps and self illumi
			//g_level.PaintOffscreen();
			//compose maps into final RT
			/*
			//#DMC: asa se foloseste RT manager, vezi rebel strain
			RECT srcrct;
			SetRect(&srcrct, gamerect.x, gamerect.y, gamerect.w, gamerect.h);
			CRTManager::CEngineRenderTarget* pRT = UTGetRenderTargetsManager().GetRTbyUID(K_RTID_FINAL);
			if (pRT != null)
				g_pGameSprite->Draw(pRT->m_pRTTexture, &srcrct, NULL, &D3DXVECTOR3(0.0f, 0.0f, 0.0f), 0xffffffff);
			*/

			//g_level.PaintComposition();

			// this will be called here (the only one)
			g_level.PaintDeferredBuffers();
		}
		break;

		default:  //on all other states just clear the RTT for now
		{
			g_level.PaintOffscreen_nothing();
			g_level.PaintComposition_nothing();
		}
		break;
	}


	///PART2. --- Render onscreen - FINAL PASS ---
	if (SUCCEEDED(pDevice->BeginScene()))
	{
		V(pDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, K_GAME_CLEAR_COLOR, 1.0f, 0));


		g_pGameSprite->Begin(D3DXSPRITE_ALPHABLEND | /*D3DXSPRITE_OBJECTSPACE |*/ D3DXSPRITE_DONOTSAVESTATE);

		pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

		switch (g_gameState)
		{
			case GAME_STATE_PUBLISHER:
			{
				UTGetAppClass().App_PaintState_Publisher(pDevice, g_pGameSprite, fElapsedTime);
			}
			break;

			case GAME_STATE_DEVELOPER:
			{
				UTGetAppClass().App_PaintState_Developer(pDevice, g_pGameSprite, fElapsedTime);
			}
			break;

			case GAME_STATE_LOADING:
			{
				UTGetAppClass().App_PaintState_Loading(pDevice, g_pGameSprite, fElapsedTime);
			}
			break;

			case GAME_STATE_SPLASH:
			{
				UTGetAppClass().App_PaintState_Splash(pDevice, g_pGameSprite, fElapsedTime);
			}
			break;

			case GAME_STATE_NET_LOBBY:
			{
				g_mainMenu.Paint();
			}
			break;

			case GAME_STATE_JOIN_COOP_LIST:
			case GAME_STATE_WORKSHOP:
			case GAME_STATE_GAME_MODE_SELECTION:
			case GAME_STATE_CHAPTER_SELECTION:
			case GAME_STATE_LEVEL_SELECTION:
			case GAME_STATE_MAINMENU:
			{
				g_mainMenu.Paint();

				// show font image
				if (DXUTIsKeyDown('6'))
				{
					if (g_font1.m_atlas.pTex != nullptr)
					{
						g_pGameSprite->Flush();
						CCameraTransform::SetActiveCameraIdentity(pDevice);
						RECT src;
						SetRect(&src, 0, 0, g_font1.m_atlas.atlasSize.w, g_font1.m_atlas.atlasSize.h);
						g_pGameSprite->SetTransform(&g_matIdentity);
						g_pGameSprite->Draw(g_font1.m_atlas.pTex, &src, NULL, &D3DXVECTOR3(UTGetAppClass().g_rectRender.x, 0.0f, 0.0f), 0xffffffff);
						g_pGameSprite->Flush();
					}
				}

			}
			break;
			case GAME_STATE_PLAYER_SELECTION:
			{
				g_playerSelScr.Paint(g_pGameSprite);
			}
			break;
			case GAME_STATE_GAME:
			{
				//if level not loaded just skip paint
				if (!g_level.m_bLoaded)
					break;

				RECTXYWH_F gamerect(0.0f, 0.0f, UTGetAppClass().g_rectScreen.w, UTGetAppClass().g_rectScreen.h);

				

				//real screen space
				CCameraTransform::SetActiveCamera(pDevice, &UTGetAppClass().g_camScreen);

				//paint game 
				/*
				RECT srcrct;
				SetRect(&srcrct, gamerect.x, gamerect.y, gamerect.w, gamerect.h);
				g_pGameSprite->Draw(g_level.m_pRTTexture_final, &srcrct, NULL, &Vec3(0.0f, 0.0f, 0.0f), 0xffffffff);
				g_pGameSprite->Flush();
				*/

				CRTManager::CEngineRenderTarget* pRTfinal = UTGetRTManager().GetRTbyUID(K_RTID_FINAL);
				if (pRTfinal != null)
				{
					g_pGameSprite->Flush();
					CCameraTransform::SetActiveCameraIdentity(pDevice);
					RECT src;
					SetRect(&src, 0, 0, pRTfinal->nWidth, pRTfinal->nHeight);
					g_pGameSprite->SetTransform(&g_matIdentity);
					g_pGameSprite->Draw(pRTfinal->m_pRTTexture, &src, NULL, &D3DXVECTOR3(UTGetAppClass().g_rectRender.x, 0.0f, 0.0f), 0xffffffff);
					g_pGameSprite->Flush();
				}
				// paint game elements above RTT content
				g_level.PaintUsingFinalRTT();
				
				//final flush
				g_pGameSprite->Flush();
				//ingame interface
				//g_level.m_interfaceIGM.Paint(pDevice, g_pGameSprite);
				//interface particles
				//g_particlesMgr.PaintLayer(K_PART_LAYER_INTERFACE_LIGHT, true);

				/*
				g_pGameSprite->Flush();
				PVERTEXSHADER vsspr = UTGetShaderManager().GetVShaderByName(L"VS_SPRITES2D");
				if (vsspr)
				{
					g_SprPainter.Begin(vsspr, UTGetAppClass().g_matProj);

					for (int kk = 0; kk < 5; kk++)
						CSprite::paintFrameNEW(&g_level.m_sprInterface, Vec3(100.0f + 30.0f * kk, 100.0f + 30.0f * kk, 0.0f), ANM_IGM_INTERFACE_SPR_PORTRAITS, kk, 
							0xffffffff, fTime, Vec2(1.0f + 0.4f * sin(fTime), 1.0f - 0.4f * sin(fTime)));


					g_SprPainter.End();
				}
				*/


				///--- level editor ---
				g_editor.Paint(g_pGameSprite);

				///--- string dummies ---
				g_pGameSprite->SetTransform(&g_matIdentity);
				//paint string dummies
				/*
				CCameraTransform::SetActiveCamera(pDevice, &UTGetAppClass().g_cam240hScreen);
				g_particlesMgr.PaintStringDummies();
				g_pGameSprite->Flush();
				 */
				//debug stuff
#if defined(_DEBUG) || defined(DEBUG)
				//game screen space
				CCameraTransform::SetActiveCamera(pDevice, &UTGetAppClass().g_camGameScreen);

				if (DXUTIsKeyDown('9'))
				{
					CRTManager::CEngineRenderTarget* pRT = UTGetRTManager().GetRTbyUID(K_RTID_COLORDEPTHSTENCIL);
					if (pRT != null)
					{
						g_pGameSprite->Flush();
						CCameraTransform::SetActiveCameraIdentity(pDevice);
						RECT src;
						SetRect(&src, 0, 0, pRT->nWidth, pRT->nHeight);
						g_pGameSprite->SetTransform(&g_matIdentity);
						g_pGameSprite->Draw(pRT->m_pRTTexture, &src, NULL, &D3DXVECTOR3(UTGetAppClass().g_rectRender.x, 0.0f, 0.0f), 0xffffffff);
						g_pGameSprite->Flush();
					}
				}
				if (DXUTIsKeyDown('8'))
				{
					CRTManager::CEngineRenderTarget* pRT = UTGetRTManager().GetRTbyUID(K_RTID_TEMP1);
					if (pRT != null)
					{
						g_pGameSprite->Flush();
						CCameraTransform::SetActiveCameraIdentity(pDevice);
						RECT src;
						SetRect(&src, 0, 0, pRT->nWidth, pRT->nHeight);
						g_pGameSprite->SetTransform(&g_matIdentity);
						g_pGameSprite->Draw(pRT->m_pRTTexture, &src, NULL, &D3DXVECTOR3(UTGetAppClass().g_rectRender.x, 0.0f, 0.0f), 0xffffffff);
						g_pGameSprite->Flush();
					}
				}
				if (DXUTIsKeyDown('0'))
				{
					CRTManager::CEngineRenderTarget* pRT = UTGetRTManager().GetRTbyUID(K_RTID_FINAL);
					if (pRT != null)
					{
						g_pGameSprite->Flush();
						CCameraTransform::SetActiveCameraIdentity(pDevice);
						RECT src;
						SetRect(&src, 0, 0, pRT->nWidth, pRT->nHeight);
						g_pGameSprite->SetTransform(&g_matIdentity);
						g_pGameSprite->Draw(pRT->m_pRTTexture, &src, NULL, &D3DXVECTOR3(UTGetAppClass().g_rectRender.x, 0.0f, 0.0f), 0xffffffff);
						g_pGameSprite->Flush();
					}
				}
				//if (DXUTIsKeyDown('9'))
				//{
				//	g_pGameSprite->Flush();
				//	CCameraTransform::SetActiveCameraIdentity(pDevice);
				//	RECT src;
				//	SetRect(&src, 0, 0, 512, 512);
				//	g_pGameSprite->SetTransform(&g_matIdentity);
				//	g_pGameSprite->Draw(g_level.m_pRTTexture, &src, NULL, &Vec3(UTGetAppClass().g_rectRender.x, 0.0f, 0.0f), 0xffffffff);
				//	g_pGameSprite->Flush();
				//}
				//if (DXUTIsKeyDown('0'))
				//{
				//	g_pGameSprite->Flush();
				//	CCameraTransform::SetActiveCameraIdentity(pDevice);
				//	RECT src;
				//	SetRect(&src, 512, 0, 1024, 512);
				//	g_pGameSprite->SetTransform(&g_matIdentity);
				//	g_pGameSprite->Draw(g_level.m_pRTTexture, &src, NULL, &Vec3(UTGetAppClass().g_rectRender.x, 0.0f, 0.0f), 0xffffffff);
				//	g_pGameSprite->Flush();
				//}
#endif
			}
			break;

#ifdef K_CONTROLS_EDITOR
			case GAME_STATE_CONTROLSED:
			{
				g_ControlsEditor.Paint();
			}
			break;
#endif
		}

		///--- chat window ---
#ifdef ENABLE_CHAT_WINDOW
		if ((UTGetAppClass().IsGameNetworked()) && (g_gameState == GAME_STATE_GAME) && (g_level.m_levelState == K_LVL_STATE_PLAYING))
		{
			g_pGameSprite->Flush();
			CCameraTransform::SetActiveCamera(pDevice, &UTGetAppClass().g_camScreen);
			RECTXYWH_F camrectchat = UTGetAppClass().g_camScreen.GetCamWorldAABB();

			Vec2 vIgmIntSz = UTGetAppClass().g_cam240hScreen.WorldToScreen(Vec2(0.0f, 56.0f));
			g_ChatWnd.Paint(Vec2(camrectchat.x + 5.0f, camrectchat.Bottom() - vIgmIntSz.y));
			g_pGameSprite->Flush();
		}
#endif

		///--- controls paint ---
#ifdef K_CONTROLS_EDITOR
		if (g_gameState != GAME_STATE_CONTROLSED)
		{
			UTGetGUI().Paint();
			g_pGameSprite->Flush();
		}
#else
		UTGetGUI().Paint();
		g_pGameSprite->Flush();
#endif

		//real screen space
		CCameraTransform::SetActiveCamera(pDevice, &UTGetAppClass().g_camScreen);
		RECTXYWH_F camrect = UTGetAppClass().g_camScreen.GetCamWorldAABB();

		//--- TRANSITIONS ---		
		PaintTransition(fElapsedTime, fTime, pDevice);
		//--- if it is paused paints "PAUSE" ---
#ifdef K_GAME_HAS_PAUSE_SCREEN
		if ((g_bCanPause) && (DXUTIsTimePaused()) && (g_font10b1 != NULL))
		{
			g_pGameSprite->Flush();
			//draw black poly over
			DWORD color = D3DCOLOR_COLORVALUE(0.0f, 0.0f, 0.0f, 0.6f);
			pDevice->SetTexture(0, NULL); 
			pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
			pDevice->SetTransform(D3DTS_VIEW, &g_matIdentity);

			RECT rct;
			SetRect(&rct, UTGetAppClass().g_rectRender.x, UTGetAppClass().g_rectRender.y, UTGetAppClass().g_rectRender.Right(), UTGetAppClass().g_rectRender.Bottom());
			DrawRectUP_TL1T(pDevice, rct, Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), color);

			g_font10b1->DrawString(STR_PAUSED, UTGetAppClass().g_rectRender.CenterX(), UTGetAppClass().g_rectRender.CenterY(), FONTFLAG_ANCHOR_BOTTOMCENTER, 0xffffffff);
		}
#endif

		///--- letterbox ---
		if ((UTGetAppClass().g_letterbox.w != 0.0f) || (UTGetAppClass().g_letterbox.h != 0.0f))
		{
			g_pGameSprite->Flush();
			//black poly over
			DWORD color = D3DCOLOR_COLORVALUE(0.0f, 0.0f, 0.0f, 1.0f);
			pDevice->SetTexture(0, NULL); //textura aiurea
			pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
			pDevice->SetTransform(D3DTS_VIEW, &g_matIdentity);

			RECT rct;
			if (UTGetAppClass().g_letterbox.w != 0.0f)
			{
				SetRect(&rct, UTGetAppClass().g_rectScreen.x, UTGetAppClass().g_rectScreen.y, UTGetAppClass().g_rectRender.x, UTGetAppClass().g_rectScreen.Bottom());
				DrawRectUP_TL1T(pDevice, rct, Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), color);
				SetRect(&rct, UTGetAppClass().g_rectRender.Right(), UTGetAppClass().g_rectScreen.y, UTGetAppClass().g_rectScreen.Right(), UTGetAppClass().g_rectScreen.Bottom());
				DrawRectUP_TL1T(pDevice, rct, Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), color);
			}
			else if (UTGetAppClass().g_letterbox.h != 0.0f)
			{
				SetRect(&rct, UTGetAppClass().g_rectScreen.x, UTGetAppClass().g_rectScreen.y, UTGetAppClass().g_rectScreen.Right(), UTGetAppClass().g_rectRender.y);
				DrawRectUP_TL1T(pDevice, rct, Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), color);
				SetRect(&rct, UTGetAppClass().g_rectScreen.x, UTGetAppClass().g_rectRender.Bottom(), UTGetAppClass().g_rectScreen.Right(), UTGetAppClass().g_rectScreen.Bottom());
				DrawRectUP_TL1T(pDevice, rct, Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), color);
			}
		}

		///----- debug info -----
		if (g_bShowDebugStats)
		{
			g_pGameSprite->Flush();
			////reset transform
			pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
			CCameraTransform::SetActiveCamera(pDevice, &UTGetAppClass().g_camScreen);
			//find a pos so doesn't overlap with the igm interface
			Vec2 vStartPos = UTGetAppClass().g_cam240hScreen.WorldToScreen(Vec2(0.0f, 25.0f));
			int posY = vStartPos.y;
			WCHAR todraw[MAX_PATH] = { 0 };
			CStringDesc strdesc;

			if(UTGetAppClass().m_Settings.dev_bDevMode)
			{
				StringCchPrintf(todraw, MAX_PATH, DXUTGetFrameStats());
				g_stringsMgr.SetStringDesc(&strdesc, todraw);
				g_font10bs1->DrawString(&strdesc, 10, posY, FONTFLAG_ANCHOR_TOPLEFT, 0xffffffff);
				posY += 15;
				StringCchPrintf(todraw, MAX_PATH, L"pointer %.2f:%.2f", g_mouse.pos.x, g_mouse.pos.y);
				g_stringsMgr.SetStringDesc(&strdesc, todraw);
				g_font10bs1->DrawString(&strdesc, 10, posY, FONTFLAG_ANCHOR_TOPLEFT, 0xffffffff);
			}
			else  //no dev mode show only ping
			{
				//FPS and gfx data
				StringCchPrintf(todraw, MAX_PATH, DXUTGetFrameStats());
				g_stringsMgr.SetStringDesc(&strdesc, todraw);
				g_font10bs1->DrawString(&strdesc, 10, posY, FONTFLAG_ANCHOR_TOPLEFT, 0xffffffff);
				posY += 15;
			}
		}																

		//end game sprite
		g_pGameSprite->End();

		//--- CONTROLS EDITOR PAINT ---
#ifdef K_CONTROLS_EDITOR
		if (g_gameState == GAME_STATE_CONTROLSED)
		{
			//pd3dDevice->SetTransform(D3DTS_VIEW, &g_matIdentity);
			g_ControlsEditor.PaintBBoxes();
		}
#endif


		//!driver optimization: unbind all resource channels
		pDevice->SetTexture(0, NULL);
		pDevice->SetTexture(1, NULL);
		pDevice->SetStreamSource(0, NULL, 0, 0);
		pDevice->SetVertexShader(null);
		pDevice->SetPixelShader(null);

		V(pDevice->EndScene());
	}

	///--- IMGUI UPDATE ---
	// must be last as it will enable and disable on user input
	if (UTimgui().BeginPaint())
	{
		// DEBUG IMGUI WINDOW
		if (g_gameState == GAME_STATE_GAME)
		{
			// non editor windows
			{
				// debug controls
				ImGui::Begin("Debug Info", null, ImGuiWindowFlags_NoNavInputs);

				if (ImGui::Button("Reload Shaders", ImVec2(120, 0)))
				{
					UTGetShaderManager().ReloadAllShaders();
				}

				ImGui::SliderFloat("gauss", &ct_fGaussLen, 0.0, 5.0);
				ImGui::SliderFloat("final multiplier", &ct_fLightMul, 0.0, 10.0);
				ImGui::SliderFloat("dodge", &ct_fColorDodge, 0.0, 1.0);

				//ImGui::Text("Visible Blocks %d", g_level.mapMesh.arrVisible.Count());
#if defined(_DEBUG) || defined(DEBUG)
				ImGui::Separator();
				ImGui::Text("Sprites: %d", UTPainter().stats_sprites);
				ImGui::Text("Calls: %d", UTPainter().stats_calls);
				ImGui::Text("Begin/End: %d", UTPainter().stats_sequences);
				ImGui::Text("Flush Calls: %d", UTPainter().stats_flushes);
#endif

				ImGui::End();
			}

			// editor block
			{
				ImGui::Begin("Commands", null, ImGuiWindowFlags_NoNavInputs);
				if (!g_editor.IsLaunched())
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.8f, 0.2f, 1.0f));
					if (ImGui::Button("Start Editor", ImVec2(120, 0)))
						g_editor.Launch(&g_level);
					ImGui::PopStyleColor(1);
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
					if (ImGui::Button("Close Editor", ImVec2(120, 0)))
						g_editor.Close();
					ImGui::PopStyleColor(1);

					// show editor controls
					g_editor.IMGUI_ShowInterfaces();
				}
				ImGui::End();
			}
		}
	

		// IMGUI tutorial window
		static bool show_demo_window = true;
		ImGui::ShowDemoWindow(&show_demo_window);
		//--- CONTROLS EDITOR INTERFACES ---
#ifdef K_CONTROLS_EDITOR
		if (g_gameState == GAME_STATE_CONTROLSED)
		{
			g_ControlsEditor.IMGUI_ShowInterfaces();
		}
#endif
		// Last but not least, paint
		UTimgui().EndPaint(pDevice);
	}

#if defined(_DEBUG) || defined(DEBUG)
	UTPainter().ClearStatistics();
#endif
}


//**************************************************************************************
// Before handling window messages, the sample framework passes incoming windows 
// messages to the application through this callback function. If the application sets 
// *pbNoFurtherProcessing to TRUE, then the sample framework will not process this message.
//**************************************************************************************
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool *pbNoFurtherProcessing)
{

	///--- IMGUI message handler---
#if defined(K_ENABLE_IMGUI)
	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
#endif

	switch (uMsg)
	{

		case WM_CREATE:
		{
			//--- pune tracker pe mouse ca sa stii cand iese din ecran ---
			TRACKMOUSEEVENT EventTrack;
			EventTrack.dwFlags = TME_LEAVE;
			EventTrack.hwndTrack = hWnd;
			EventTrack.cbSize = sizeof(TRACKMOUSEEVENT);
			TrackMouseEvent(&EventTrack);
		}
		break;
	//daca apare vreun msg de mouse reactivez mouse-ul
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_XBUTTONDBLCLK:
		case WM_MOUSEWHEEL:
		case WM_MOUSEMOVE:
		{
			g_mouse.fTimeSinceInput = 0.0f;
			int times = ShowCursor(true);
			while (times < 0)
				times = ShowCursor(true);

			ShowCursor(true);

			//if (g_bCustomMouseCursor)
			//{
			//	//la fiecare mesaj de mouse cheama cursorul pe NULL (sper sa nu mearga mai greu)
			//	SetCursor(hCustomCursorHand);
			//}
			//else
			//{
			//	SetCursor(hCursorArrow);
			//}
			if (g_mouse.bCursorOutsideWindow)
			{
				g_mouse.bCursorOutsideWindow = false;
				SetCursor(NULL);
				g_mouse.lastPos = g_mouse.pos;
				g_mouse.Lbut = K_MOUSE_BUTT_NOTPRESSED;
				g_mouse.Rbut = K_MOUSE_BUTT_NOTPRESSED;

				//start tracking so we know when the cursor is outside clientrect
				TRACKMOUSEEVENT EventTrack;
				EventTrack.dwFlags = TME_LEAVE;
				EventTrack.hwndTrack = hWnd;
				EventTrack.cbSize = sizeof(TRACKMOUSEEVENT);
				TrackMouseEvent(&EventTrack);

				return 0;
			}
		}
		break;
	//--- daca iese cursorul din fereastra pune mouseout pe true ---
		case WM_MOUSELEAVE:
		{
			g_mouse.bCursorOutsideWindow = true;
			//SetCursor(hCursorArrow);
		}
		break;

	//--- window focus handling ---
		case WM_CLOSE:
		{
			//force quit lobby on exit
			g_netlock.Net_QuitLobby();

			g_bRequestedExit = true;
			DXUTSetShortcutKeySettings(true, true);
			//set old cursor
			//::SetCursor(hcurOriginal);
			//save settings on exit
			UTGetAppClass().SaveSettings();

			LOG(L"Shutting down!");
		}
		break;

		case WM_SIZE:
		{
		}
		break;
		case WM_KILLFOCUS:
		{
			//on lost focus reset keypresses (ONLY ON NOT NETWORKED GAMES OR IT WILL DESYNC)
			if (!UTGetAppClass().IsGameNetworked())
				UTGetCtrlrMgr().ResetAllControllersKeypresses();
			//cand e pe fullscreen si pierzi focus forteaza minimize ca sa vezi ce se intampla
			if (!DXUTIsWindowed())
				ShowWindow(hWnd, SW_MINIMIZE);
		}
		break;

		case WM_SETFOCUS:
		case WM_MOVE:
		{
		}
		break;
		case WM_EXITSIZEMOVE:
		{
			//window moved or finished resizing
		}
		break;
		
		case WM_DPICHANGED:
		{
			/*
			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
			{
				//const int dpi = HIWORD(wParam);
				//printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
				const RECT* suggested_rect = (RECT*)lParam;
				::SetWindowPos(hWnd, NULL, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			*/
		}
		break;
		
		case WM_CHAR:
		{
			UTGetGUI().ReceiveInput(K_CCTRLMGR_INPUT_CHAR, (UINT32)wParam);
#ifdef ENABLE_CHAT_WINDOW
			g_ChatWnd.ReceiveChar((UINT32)wParam);
#endif
		}
		break;
	}

	return 0;
}

//*********************************
// Mouse callback
//*********************************
void CALLBACK MouseProc(bool bLeftButton, bool bRightButton, bool /*bMiddleButton*/, bool /*bSideButton1*/, bool /*bSideButton2*/, int nMouseWheelDelta, int xPos, int yPos)
{
	//coordonate mouse in pixeli ecran/fereastra
	g_mouse.pos.x = xPos;
	g_mouse.pos.y = yPos;

	g_mouse.bLbut = bLeftButton;
	g_mouse.bRbut = bRightButton;
	//salvez si delta
	g_mouse.wheelDelta = nMouseWheelDelta;

}



//**************************************************************************************
// Keypressed - key processing
//**************************************************************************************
void CALLBACK KeyboardProc(UINT nChar, bool bKeyDown, bool bAltDown)
{
	//send keypress to controllers class
	//UTGetControllersManager().ReceiveKeypress(nChar, bKeyDown, bAltDown);

	if (bKeyDown)
	{
		if((bAltDown) && (nChar == VK_F4) && (!g_bRequestedExit))
		{
			g_bRequestedExit = true;

			PostQuitMessage(0);
			//ar trebui sa salveze starea instantaneu
			return;
		}

#ifdef K_CONTROLS_EDITOR
		g_ControlsEditor.ReceiveKeys(nChar);
#endif

		g_editor.ReceiveKeys(nChar);
		///--- send keys to controls manager ---
		UTGetGUI().ReceiveInput(K_CCTRLMGR_INPUT_KEY, (UINT32)nChar);
																				 
		switch (nChar)
		{
			//--- shows debug info ---
			case VK_F5:
			{
				if (UTGetAppClass().m_Settings.dev_bDevMode)
				{
					//SCRIPTS - hot reload
					if (bAltDown)
					{
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
						//if (g_gameState != GAME_STATE_GAME)
						//{
						//	UTGetScriptManager().Release();

						//	WCHAR xmlpath[MAX_PATH];
						//	StringCchPrintf(xmlpath, MAX_PATH, L"%s/scripts.xml", UTGetAppClass().g_wszAppResDir);
						//	UTGetScriptManager().AddScripts(xmlpath);

						//	///--- player selection screen items ---
						//	g_playerSelScr.LoadItems();

						//	MessageBox(NULL, L"Scripts Reloaded! Player selection screen Items reloaded!", L"Info", MB_OK);
						//}
#endif
					}
					else
					{
						g_bShowDebugStats = !g_bShowDebugStats;
					}
				}
				else  //no dev mode
				{
					g_bShowDebugStats = !g_bShowDebugStats;
				}
			}
			break;

			///--- Shows the mey mapping screen ---
			case VK_F1:
			{
				if ((g_gameState == GAME_STATE_GAME) && (g_level.m_levelState == K_LVL_STATE_PLAYING))
				{
					if (bAltDown)
					{
						if (!UTGetAppClass().IsGameNetworked())
						{
							CCtrlLayer* layer = UTGetGUI().GetLayerByName("LAYER_ID_KEYMAP");
							if (layer == null)
							{
								CCtrlLayer *lay =UTGetGUI().ShowLayerOnce("LAYER_ID_KEYMAP");
								if (lay)
								{
									CControl *ctrl = lay->GetControlByName("LS_KEYS1");
									if (ctrl)
									{
										ctrl->bDisabled = true;
										ctrl->bCanHaveFocus = false;
									}
									ctrl = lay->GetControlByName("LS_KEYS2");
									if (ctrl)
									{
										ctrl->bDisabled = true;
										ctrl->bCanHaveFocus = false;
									}
								}
							}
							else
							{
								UTGetGUI().RemoveLayer("LAYER_ID_KEYMAP");
							}
						}
					}
					else
					{
						UTGetAppClass().m_Settings.bShowInterfaceHelp = !UTGetAppClass().m_Settings.bShowInterfaceHelp;
					}
				}
			}
			break;

			///--- TAKES SCREENSHOTS ---
			case VK_F10:
			{
				if ((g_gameState != GAME_STATE_LOADING) && (!g_bDuringTransition))
				{
					if (FAILED(UTGetAppClass().SaveScreenshot()))
					{
						SND_PLAY(SNDIDX_DENIED);
					}								  
					else
					{
						SND_PLAY(SNDIDX_STARHIT);
					}
				}
			}
			break;

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
			case VK_F7:
			{
				g_font12wow->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ40.textHash), &UTGetAppClass().g_cam480hScreen);
				g_font10b1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ40.textHash), &UTGetAppClass().g_cam480hScreen);
				g_font10bs1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ40.textHash), &UTGetAppClass().g_cam480hScreen);
				g_font9b1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ30.textHash), &UTGetAppClass().g_cam480hScreen);
				g_font8b1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ30.textHash), &UTGetAppClass().g_cam480hScreen);
				g_font8bs1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ30.textHash), &UTGetAppClass().g_cam480hScreen);
				g_font6n1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ20.textHash), &UTGetAppClass().g_cam480hScreen);
				g_font6ns1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ20.textHash), &UTGetAppClass().g_cam480hScreen);
				g_font6nc1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ20.textHash), &UTGetAppClass().g_cam480hScreen);
				g_font5n1->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ20.textHash), &UTGetAppClass().g_cam480hScreen);
				g_font5n2->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ20.textHash), &UTGetAppClass().g_cam480hScreen);
				g_font5ns2->SetFontReplacementTTF(UTGetTTFManager().GetFont(shTTFID_SZ20.textHash), &UTGetAppClass().g_cam480hScreen);
			}
			break;
			case VK_F8:
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
			break;
#endif

#ifdef ENABLE_CHAT_WINDOW
			case VK_RETURN:
			{
				//chat available only when playing networked game and no other interface visible
				if ((UTGetAppClass().IsGameNetworked()) && (g_gameState == GAME_STATE_GAME) && 
					(g_level.m_levelState == K_LVL_STATE_PLAYING) && (UTGetGUI().Layers.GetSize() == 0))
				{
					//enable input if not already enabled
					if (!g_ChatWnd.IsReceivingInput())
						g_ChatWnd.StartInput();
				}
			}
			break;
#endif

#ifdef K_CONTROLS_EDITOR
			case VK_F2:
			{
				int nextState = GAME_STATE_CONTROLSED;
				if (g_gameState == GAME_STATE_CONTROLSED)
					nextState = GAME_STATE_MAINMENU;

				CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE);
				nevent->AddNamedArgUINT32(L"newGameState", nextState);
				UTGetEventManager().QueueEvent(nevent);
			}
			break;
#endif

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
			// IMGUI show/hide
			case VK_F3:
			{
				UTimgui().SetGlobalEnabled(!UTimgui().bEnabled);
			}
			break;
#endif
		}
	}
}


//--------------------------------------------------------------------------------------
// GAMESTATE CHANGER
//--------------------------------------------------------------------------------------
void ChangeGameState(int newState, int param1, int param2)
{
	LOG(L"System:: ChangeGameState(%d)", newState);
	int oldGameState = g_gameState;

	///--- from what state is it coming? ---
	switch (oldGameState)
	{
		case GAME_STATE_PUBLISHER:
		{
			UTGetAppClass().App_ExitState_Publisher();
		}
		break;
		case GAME_STATE_DEVELOPER:
		{
			UTGetAppClass().App_ExitState_Developer();
		}
		break;
		case GAME_STATE_LOADING:
		{
			UTGetAppClass().App_ExitState_Loading();
			g_bForceOneUpdatePerFrame = false;
		}
		break;
		case GAME_STATE_SPLASH:
		{
			UTGetAppClass().App_ExitState_Splash();
		}
		break;

		case GAME_STATE_PLAYER_SELECTION:
		{
			g_playerSelScr.ReleaseSprites();
			///--- load main menu ---
			WCHAR xmlpath[MAX_PATH];
			FileManager::GetMediaPath(L"media/interfaces/menus.bsx", xmlpath);
			if (FAILED(g_mainMenu.LoadSprites(xmlpath)))
			{
				ErrorBox(K_ERR_CRITICAL, L"Main Menu file not found:\n%s", xmlpath);
			}

			UTGetGUI().RemoveAllLayers(true);
		}
		break;
		case GAME_STATE_GAME:
		{

#ifdef ENABLE_CHAT_WINDOW
			//cancel current input if exited
			g_ChatWnd.CancelInput();
			g_ChatWnd.Clear();
#endif

			//push global scores to leaderboard when returning from the game
#ifdef ENABLE_LEADERBOARDS
			//upload multiplayer score
			if (g_userData[K_MEMID_TOTAL_SCORE_COOP] > 0)
				UTGetLeaderboards().QueueJob(K_JOB_UPLOAD_SCORE, K_GAME_STR_LEADERBOARDS_GLOBAL_COOP, g_userData[K_MEMID_TOTAL_SCORE_COOP]);
			//upload single player score so that current leaderboard remains the single player one
			if (g_userData[K_MEMID_TOTAL_SCORE_SOLO] > 0)
				UTGetLeaderboards().QueueJob(K_JOB_UPLOAD_SCORE, K_GAME_STR_LEADERBOARDS_GLOBAL_SP, g_userData[K_MEMID_TOTAL_SCORE_SOLO]);
#endif
			//must be called here to reset controller flags
			UTGetCtrlrMgr().ResetAllControllersKeypresses();
			//stop all sounds
			UTGetSoundManager().StopGroup("sounds", false, true);
			UTGetSoundManager().StopGroup("ingame", false, true);
			
			SND_SET_GROUP_FREQUENCY("ingame", 1.0f, false);

			g_level.Release();
			// level was unloaded, immediately set the controller pointer to null
			UTGetCtrlrMgr().SetNormalizeCoordsFunctionPtr(nullptr);

			UTGetGUI().RemoveAllLayers(true);

			UTGetSoundManager().StopGroup("music", false, true);
			if (newState != GAME_STATE_GAME)
			{
				if (g_gameMode == GAME_MODE_ZOMBIE_INVASION)
				{
					//SND_PLAY_ONCE(SNDIDX_THEME_HALLOWEEN, DSBPLAY_LOOPING);
				}
				else
				{
					SND_PLAY_ONCE(SNDIDX_THEME_MENU1, DSBPLAY_LOOPING);
				}
			}

			//set volumes
			SND_SET_GROUP_VOLUME("sounds", UTGetAppClass().m_Settings.fSoundsVolume, false);
			SND_SET_GROUP_VOLUME("ingame", UTGetAppClass().m_Settings.fSoundsVolume, false);
			SND_SET_GROUP_VOLUME("music", UTGetAppClass().m_Settings.fMusicVolume, false);

			///--- load main menu ---
			WCHAR xmlpath[MAX_PATH];
			FileManager::GetMediaPath(L"media/interfaces/menus.bsx", xmlpath);
			if (FAILED(g_mainMenu.LoadSprites(xmlpath)))
			{
				ErrorBox(K_ERR_CRITICAL, L"Main Menu file not found:\n%s", xmlpath);
			}
		}
		break;

		case GAME_STATE_WORKSHOP:
		{
			UTGetSoundManager().StopGroup("sounds", false, true);
			UTGetGUI().RemoveAllLayers(true);
			//release used textures here:
			UTGetAppClass().g_texManager.Release();
			//make sure we reload everything that can be modded
			App_ReloadContentChanges();
			///compute mods CRC
			UINT32 unModsCRC = App_GetActiveModsCRC();
			//initialize vertical mode after modding
			//g_verticalMode.Init(&g_level, L"media/levels/mod_prefabs/infinite_tower.xml");
			
			//unModsCRC += g_verticalMode.GetFilesCRC(false);
			UTGetAppClass().m_Settings.dev_unCurrentModsCRC = unModsCRC;
			LOG(L"--> CRC_BASE [%08x] CRC_MODS [%08x] <--", UTGetAppClass().m_Settings.dev_unCurrentCRC, UTGetAppClass().m_Settings.dev_unCurrentModsCRC);
			//when returning from the mods screen reload the main menu in case it changed
			g_mainMenu.Release();
			
			WCHAR xmlpath[MAX_PATH];
			FileManager::GetMediaPath(L"media/interfaces/menus.bsx", xmlpath);
			if (FAILED(g_mainMenu.LoadSprites(xmlpath)))
			{
				ErrorBox(K_ERR_CRITICAL, L"Main Menu file not found:\n%s", xmlpath);
			}
		}
		break;

		case GAME_STATE_JOIN_COOP_LIST:
		{
			UTGetSoundManager().StopGroup("sounds", false, true);
			UTGetGUI().RemoveAllLayers(true);
		}
		break;

		case GAME_STATE_NET_LOBBY:
		case GAME_STATE_GAME_MODE_SELECTION:
		case GAME_STATE_CHAPTER_SELECTION:
		case GAME_STATE_LEVEL_SELECTION:
		case GAME_STATE_MAINMENU:
		{
			UTGetSoundManager().StopGroup("sounds", false, true);
			UTGetGUI().RemoveAllLayers(true);
			//release used textures here:
			UTGetAppClass().g_texManager.Release();
		}
		break;
#ifdef K_CONTROLS_EDITOR
		case GAME_STATE_CONTROLSED:
		{
			UTimgui().SetGlobalEnabled(false);
			g_ControlsEditor.Close();
		}
		break;
#endif
	}

	///--- set new game state here ---
	g_gameState = newState;

	switch (newState)
	{
		case GAME_STATE_PUBLISHER:
		{
			UTGetAppClass().App_EnterState_Publisher();
		}
		break;

		case GAME_STATE_DEVELOPER:
		{
			UTGetAppClass().App_EnterState_Developer();
		}
		break;

		case GAME_STATE_LOADING:
		{
			g_bForceOneUpdatePerFrame = true;
			UTGetAppClass().App_EnterState_Loading();

			//on loading disable sync
			UTGetAppClass().m_Settings.devnet_eNetGameType = CApplicationSettings::K_NETGAME_TYPE_NO_NETWORK;
			UTGetAppClass().m_Settings.devnet_eSyncStatus = CApplicationSettings::K_NETGAME_SYNC_STOPPED;
		}
		break;

		case GAME_STATE_SPLASH:
		{
			UTGetAppClass().App_EnterState_Splash();
		}
		break;

		case GAME_STATE_UPLOAD_MOD:
		{
			//some initial settings
			g_gameSubstate = 0;
			//force creating log window?
			UTGetAppClass().m_Settings.dev_bLogWindowShow = true;
		}
		break;

		case GAME_STATE_MAINMENU:
		{
			g_level.GetNextRandomLevel();
#if defined(_DEBUG) || defined(DEBUG)
	#if defined(ENABLE_ACHIEVEMENTS_RESET_ON_STARTUP)
			ErrorBox(K_ERR_ONSCREEN, L"---> [Achievements] Resetting achievements on startup!");
			SteamUserStats()->ResetAllStats(true);
	#endif
#endif
			g_gameMode = GAME_MODE_CLASSIC;
			//set menu state
			g_mainMenu.SetState(K_MM_STATE_MAINMENU);

			//set just started
			g_netlock.Net_QuitLobby();
			if (g_bJustStarted)
			{
				g_bJustStarted = false;

				//analytics
				ANALYTICS_SCREENVIEW("main_menu");
#ifdef ENABLE_STEAM
				ANALYTICS_EVENT("game_started_STEAM", _VERSION_CHARSTR_, "", 1);
#endif
#ifdef ENABLE_GALAXY
				ANALYTICS_EVENT("game_started_GOG", _VERSION_CHARSTR_, "", 1);
#endif
			}

			//mark game as NON networked
			UTGetAppClass().m_Settings.devnet_eNetGameType = CApplicationSettings::K_NETGAME_TYPE_NO_NETWORK;
			UTGetAppClass().m_Settings.devnet_eSyncStatus = CApplicationSettings::K_NETGAME_SYNC_STOPPED;
		}
		break;

		case GAME_STATE_JOIN_COOP_LIST:
		{
			//set a state that has no update logic (just displays the background)
			g_mainMenu.SetState(K_MM_STATE_NET_LOBBY);

			//as soon as we enter we ask for the lobbies list and the state will read the lobbies a little later (on a timer job)
			g_netlock.Net_RequestLobbyList(10);

			g_stringsMgr.SetString(STR_LOBBIES_LIST_VAL, L"%s", g_stringsMgr.strings[STR_PLEASE_HANG]->sText);
			//add the window
			CCtrlLayer* lay = UTGetGUI().ShowLayerOnce("LAYER_ID_LOBBIES_LIST");
			if (lay != null)
			{
				CControl* ctrl = lay->GetControlByName("BUT_JOIN_LOBBY");
				if (ctrl)
					ctrl->bDisabled = true;

				ctrl = lay->GetControlByName("CTRL_LOBBIES_SELECTOR");
				if (ctrl)
				{
					ctrl->bDisabled = true;
					ctrl->paramsDict.SetNamedVarINT32(L"nOptionsCnt", 1);
				}

				ctrl = lay->GetControlByName("BUT_REFRESH_LOBBIES");
				if (ctrl)
					ctrl->bDisabled = true;
			}
		}
		break;

		case GAME_STATE_NET_LOBBY:
		{
			g_gameSubstate = 0;

			//exit lobby if was in lobby
			if (UTGetAppClass().m_Settings.devnet_eNetGameType != CApplicationSettings::K_NETGAME_TYPE_NO_NETWORK)
			{
				g_netlock.Net_QuitLobby();
			}

			//show window
#ifdef ENABLE_STEAM
			UTGetGUI().ShowLayerOnce("LAYER_ID_QUICK_MATCH_INVITE");
#endif
#ifdef ENABLE_GALAXY
			UTGetGUI().ShowLayerOnce("LAYER_ID_QUICK_MATCH");
#endif
			//change menu on net lobby background
			g_mainMenu.SetState(K_MM_STATE_NET_LOBBY);

			//set correct network game type
			UTGetAppClass().m_Settings.devnet_eNetGameType = (CApplicationSettings::eNetGameTypes)param1; //param1 contains net match type 
			UTGetAppClass().m_Settings.devnet_eSyncStatus = CApplicationSettings::K_NETGAME_SYNC_STOPPED;
			//enter lobby
			switch (UTGetAppClass().m_Settings.devnet_eNetGameType)
			{
				case CApplicationSettings::eNetGameTypes::K_NETGAME_TYPE_QUICK_MATCH:
				{
					g_netlock.Net_EnterLobby(false, false);
					LOG(L"[NET] Entered lobby Quick Match (game mode: %d)", g_gameMode);
				}
				break;
				case CApplicationSettings::eNetGameTypes::K_NETGAME_TYPE_HOST_PUBLIC:
				{
					g_netlock.Net_EnterLobby(true, false);
					LOG(L"[NET] Entered lobby Host Public (game mode: %d)", g_gameMode);
				}
				break;
				case CApplicationSettings::eNetGameTypes::K_NETGAME_TYPE_HOST_PRIVATE:
				{
					g_netlock.Net_EnterLobby(true, true);
					LOG(L"[NET] Entered lobby Host Private (game mode: %d)", g_gameMode);
				}
				break;
			}

			//analytics
			ANALYTICS_SCREENVIEW("net_match");
		}
		break;

		case GAME_STATE_WORKSHOP:
		{
			//save user data here, will load it when exiting the workshop
			App_SaveUserData();
			//set state
			g_mainMenu.SetState(K_MM_STATE_WORKSHOP);
		}
		break;

		case GAME_STATE_GAME_MODE_SELECTION:
		{
			//reset game mode
			g_gameMode = GAME_MODE_CLASSIC;
			//set state (and transmit arg for target game state)
			g_mainMenu.SetState(K_MM_STATE_GAME_MODE_SELECT, param1);
		}
		break;

		case GAME_STATE_CHAPTER_SELECTION:
		{
			if (g_gameMode == GAME_MODE_ZOMBIE_INVASION)
			{
				/*
				if (!SND_IS_PLAYING(SNDIDX_THEME_HALLOWEEN))
				{
					SND_STOP_GROUP("music", false, true);
					SND_PLAY_ONCE(SNDIDX_THEME_HALLOWEEN, DSBPLAY_LOOPING);
				}
				*/
			}

			g_mainMenu.SetState(K_MM_STATE_CHAPTER_SELECT);
		}
		break;

		case GAME_STATE_LEVEL_SELECTION:
		{
			g_mainMenu.SetState(K_MM_STATE_LEVEL_SELECT);
			//reset downloaded mod selection
			g_userData[K_MEMID_MOD_DWNLVL_SELECTED] = -1;
		}
		break;

		case GAME_STATE_PLAYER_SELECTION:
		{
			//release main menu sprites
			g_mainMenu.Release();
			//just to load them back in the player selection screen
			WCHAR xmlpath[MAX_PATH];
			FileManager::GetMediaPath(L"media/interfaces/menus.bsx", xmlpath);
			if (FAILED(g_playerSelScr.InitSprites(xmlpath)))
			{
				ErrorBox(K_ERR_CRITICAL, L"File not found:\n%s", xmlpath);
				break;
			}

			//param1: reset instanceIDs - On networked games don't reset instance ids so they appear already selected.
			if (param1 != 0)
			{
				g_playerSelScr.ResetSelection(true);
			}
			else
			{
				g_playerSelScr.ResetSelection(false);
			}
			//on networked games send local selection immediately so they sync levels
			if (UTGetAppClass().IsGameNetworked())
			{
				g_netlock.Net_EnterPlayerSelScreen();

				g_playerSelScr.SendSelectionByNetwork(g_netlock.Net_GetPlayerIndex());
			}
		}
		break;
		case GAME_STATE_GAME:
		{
			//increase music counter
			g_userData[K_MEMID_MUSIC_TRACK_COUNTER]++;
			//save user data again
			App_SaveUserData();
			//release main menu class
			g_mainMenu.Release();
			// set the controller pointer normalization function (gets set to nullptr when not in game)
			UTGetCtrlrMgr().SetNormalizeCoordsFunctionPtr(NormalizeIngameMouseCoords);

			//reset all scripts
			UTGetScriptManager().StopAllScripts();
			//clear global memory - nothing stays between levels
			UTGetScriptManager().ClearGlobalMemory();

			//loads the level
			if (g_userData[K_MEMID_MOD_DWNLVL_SELECTED] < 0)
			{
				//classic levels
				///--- find chapter and level in levels.xml ---	
				WCHAR strLevelPath[MAX_PATH] = { 0 };
				if (g_startupCommand == GAME_STARTUP_LOAD_MAP)
				{
					std::wstring sProcessedPath = RemoveQuotationMarks(g_startupParam.text);
					//starting game with forced map
					StringCchPrintf(strLevelPath, MAX_PATH, L"%s", sProcessedPath.c_str());
					//write current mission name and number
					g_stringsMgr.SetString(STR_CURRENT_MISSION_VAL, L"");
				}
				else
				{
					int nChapterNumber = g_userData[K_MEMID_SELECTED_CHAPTER];
					int nLevelNumber = g_userData[K_MEMID_SELECTED_LEVEL];
					bool bLevelFound = UTGetChaptersList().GetMissionFilename(nChapterNumber, nLevelNumber, strLevelPath, MAX_PATH);
					if (!bLevelFound)
					{
						ChangeGameStateTransition(GAME_STATE_LEVEL_SELECTION, 0, 0, K_TRANSITION_TYPE_SIMPLE);
						break;
					}
					//write current mission name and number
					int nStrIdxLevelName = UTGetChaptersList().m_arrChapters[nChapterNumber]->arrLevelNameStrIdx[nLevelNumber];
					g_stringsMgr.SetString(STR_CURRENT_MISSION_VAL, L"%d.%d %s", nChapterNumber + 1, nLevelNumber + 1, g_stringsMgr.strings[nStrIdxLevelName]->sText);
				}

				if (FAILED(g_level.LoadLevel(strLevelPath)))
				{
					ErrorBox(K_ERR_WARNING, L"Could not load level [%s]!", strLevelPath);
					ChangeGameStateTransition(GAME_STATE_LEVEL_SELECTION, 0, 0, K_TRANSITION_TYPE_SIMPLE);
					break;
				}
			}
			else
			{
				//modded custom levels
#ifdef ENABLE_STEAM_WORKSHOP
				CModsManager::CModDescriptor *mod = UTGetModsManager().GetModDescByIndex(g_userData[K_MEMID_MOD_DWNLVL_SELECTED]);
				if (mod == null)
				{
					ErrorBox(K_ERR_WARNING, L"Couldn't find custom level!");
					ChangeGameStateTransition(GAME_STATE_LEVEL_SELECTION, 0, 0, K_TRANSITION_TYPE_SIMPLE);
					break;
				}

				WCHAR wcsLevelPath[MAX_PATH] = { 0 };
				mod->GetFullPathToAffectedFile(0, wcsLevelPath, MAX_PATH);
				//write current mission name and number
				g_stringsMgr.SetString(STR_CURRENT_MISSION_VAL, L"%s", mod->shName.text);

				if (FAILED(g_level.LoadLevel(wcsLevelPath)))
				{
					ErrorBox(K_ERR_WARNING, L"Could not load downloaded level [%s]!", wcsLevelPath);
					ChangeGameStateTransition(GAME_STATE_LEVEL_SELECTION, 0, 0, K_TRANSITION_TYPE_SIMPLE);
					break;
				}
#endif // ENABLE_STEAM_WORKSHOP
			}

			//start menu music
			SND_STOP_GROUP("music", false, true);

			if ((g_userData[K_MEMID_SELECTED_CHAPTER] == K_GAME_WEEKLY_CHALLENGE_CHAPTER_NO) && (g_userData[K_MEMID_SELECTED_LEVEL] == 2))
			{
				SND_STOP_GROUP("music", false, true);
				//SND_PLAY_ONCE(SNDIDX_THEME_HALLOWEEN, DSBPLAY_LOOPING);
			}
			else
			{
				SND_PLAY_ONCE(SNDIDX_THEME_MENU1, DSBPLAY_LOOPING);
			}
			//analytics
			ANALYTICS_SCREENVIEW("INGAME");
		}
		break;

#ifdef K_CONTROLS_EDITOR
		case GAME_STATE_CONTROLSED:
		{
			UTGetGUI().RemoveAllLayers(true);
			g_ControlsEditor.Launch();
			UTimgui().SetGlobalEnabled(true);
		}
		break;
#endif
	}
}

//variabile tranzitie
int g_nTransitionStep = 0;
float g_fTransitionPercent = 0.0f;
int g_nNextState = 0;
int g_nSentParameter1 = 0;
int g_nSentParameter2 = 0;
int g_nTransitionType = 0;

void ChangeGameStateTransition(int newState, int param1, int param2, int transitionType)
{
	g_nTransitionStep = 0;
	g_fTransitionPercent = 0.0f;
	g_bDuringTransition = true;
	g_nNextState = newState;
	g_nSentParameter1 = param1;
	g_nSentParameter2 = param2;

	g_nTransitionType = transitionType;
}

void UpdateTransition(float dTime)
{
	if (!g_bDuringTransition) return;

	switch (g_nTransitionType)
	{
		case K_TRANSITION_TYPE_SIMPLE:
		{
			switch (g_nTransitionStep)
			{
				case 0: //show full screen black poly
				{
					if (g_fTransitionPercent >= 1.0f)
					{
						g_fTransitionPercent = 0.0f;
						g_nTransitionStep = 1;
						//full black, change state now
						ChangeGameState(g_nNextState, g_nSentParameter1, g_nSentParameter2);
					}

					g_fTransitionPercent += dTime * 6.0f;
					CLAMP(g_fTransitionPercent, 0.0f, 1.0f);
				}
				break;
				case 1: 
				{
					g_fTransitionPercent += dTime * 6.0f;
					if (g_fTransitionPercent >= 1.0f)
					{
						g_fTransitionPercent = 0.0f;
						g_bDuringTransition = false;
					}
				}
				break;
			}
		}
		break;
	}

}

void PaintTransition(float dTime, float fTimeline, PDEVICE pDevice)
{
	if (!g_bDuringTransition) return;

	switch (g_nTransitionType)
	{
		//tranzitia neagra simpla
		case K_TRANSITION_TYPE_SIMPLE:
		{
			//reset transforms
			pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
			pDevice->SetTransform(D3DTS_VIEW, &g_matIdentity);

			RECT rect;
			SetRect(&rect, UTGetAppClass().g_rectRender.x, UTGetAppClass().g_rectRender.y, UTGetAppClass().g_rectRender.Right(), UTGetAppClass().g_rectRender.Bottom());

			switch (g_nTransitionStep)
			{
				case 0: 
				{
					g_pGameSprite->Flush();
					//deseneaza poly negru peste
					pDevice->SetTexture(0, NULL); //textura aiurea
					//fac ca jumatate din timpul tranzitiei sa stea pe full opac ca sa nu se vada absolut nimic cand schimba starea
					float fAlpha = LIMIT(1.5f * g_fTransitionPercent, 0.0f, 1.0f);
					DrawRectUP_TL1T(pDevice, rect, Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), D3DCOLOR_XXXA(fAlpha));
					//write "loading"
					if ((UTGetGUI().m_sprCol.IsLoaded()) && (fAlpha >= 0.95f))
					{
						CCameraTransform::SetActiveCamera(pDevice, &UTGetAppClass().g_cam240hScreen);
						RECTXYWH_F scrrect = UTGetAppClass().g_cam240hScreen.GetCamWorldAABB();
						CSprite::paintFrame(&UTGetGUI().m_sprCol, scrrect.Right() - 3, scrrect.Bottom() - 3, ANM_CONTROLS_SPR_LOADING_ICONS, 0, 0x88ffffff);
					}
				}
				break;
				case 1:	
				{
					g_pGameSprite->Flush();
					//deseneaza poly negru peste
					pDevice->SetTexture(0, NULL); //textura aiurea
					float fAlpha = LIMIT((1.5f - 1.5f * g_fTransitionPercent), 0.0f, 1.0f);
					DrawRectUP_TL1T(pDevice, rect, Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), D3DCOLOR_XXXA(fAlpha));
				}
				break;
			}
		}
		break;

	}
}

