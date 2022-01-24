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
Vec2						g_vecGravityOld;					//gravity
Vec3						g_vecGravity;						//gravity

ID3DXSprite*				g_pGameSprite = NULL;				//Main Sprite class 
Mat							g_matIdentity;						//identity matrix
Mat							g_matWorld;							//world matrix

CLog*						g_pLog;								//log class

bool						g_bCanPause = false;				// Global flag: can we pause the game while in background?
bool						g_bLevelNeedsUpdate = false;		//#HACK: pentru un singur frame ramane true dupa resolution change ca sa faca update chiar daca jocul e pe pauza

///--- Redefine Keys ---
EControllerCommand			g_keydef_command = K_CM_COMMAND_NONE;	//command to redefine (NONE means sequence wasn't initialized)
int							g_keydef_scancode = -1;					//scancode for command (SDL scancodes for now)

CMouseData					g_mouse;								// Mouse data, global

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

CGame						g_game;								// main game wrapper


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


// Called before window and 3d device get created
OPRESULT	BeforeMount(void);
// Called after window and 3d device get created and are ready to be used
OPRESULT	AfterMount(void);
// Called after shutting down the device
void		ShutdownApp(void);
// Initializes the sound system
OPRESULT	InitSound(void);

///-----------------------------------------------------
/// MISC UTILITY FUNCTIONS
///-----------------------------------------------------

// Spine extension used for allocation and deallocations (singleton)
spine::SpineExtension *spine::getDefaultExtension() {
	static spine::DefaultSpineExtension g_spineExtension;
	return &g_spineExtension;
}

// Callback used by ControllersMgr to normalize mouse input from global to ingame player relative
// Hard to make it a class method and use as a callback so make it global
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

	if (!UTApp().IsOnlyInstance(K_GAME_WINDOW_CLASSNAME))
		return 0;

	HRESULT hr = S_OK;
	// Init crash dumper
	///#TODO: needed?
	//InitMiniDumper();
	// init game constants like paths to executable
	UTApp().Init();
	// clear debug log file 
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
		UTApp().m_Settings.galaxyFullyLoaded = true;
	}
	else
	{
		UTApp().m_Settings.galaxyFullyLoaded = false;
		ErrorBox(K_ERR_WARNING, L"Galaxy API is not fully loaded. Error: %s", err->GetMsg());
	}

	galaxy::api::User()->SignInGalaxy();
#endif // ENABLE_GALAXY

	// declare that we're DPI aware (even with imgui off)
	ImGui_ImplWin32_EnableDpiAwareness();

	//load game settings FIRST AND FOREMOST (includes selected language and so on)
	UTApp().LoadSettings();
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
	if (OP_FAILED(BeforeMount()))
	{
		ErrorBox(K_ERR_CRITICAL, L"Ooops, couldn't initialize game (BeforeMount) !\r\nTo fix it, check our support forum or contact us at %s\r\n", K_GAME_EMAIL );
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
	StringCchPrintf(windowTitle, MAX_PATH, L"%s", UTLang().strings[STR_TITLE]->sText);

	if (FAILED(DXUTCreateWindow(windowTitle, hInst, NULL, NULL /*, 0, 0*/)))
	{
		ErrorBox(K_ERR_CRITICAL, L"[ERROR] Couldn't create window!\r\nTo fix it, check our support forum or contact us at %s\r\n", K_GAME_EMAIL);
	}

	///--- LOG WINDOW ---
	if (UTApp().m_Settings.dev_bLogWindowShow)
	{
		OS_CreateLogWindow();
	}

	//Log current time too
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	LOG(L"Log system started. (%d-%d-%d %d:%d:%d)", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
#ifdef ENABLE_STEAM
	LOG(L"Steam Version %s, Savefile Version %d", UTLang().strings[STR_VERSION_NUMBER]->sText, _VERSION_DATAFILE_);
#endif // ENABLE_STEAM
#ifdef ENABLE_GALAXY
	LOG(L"GoG Version %s, Savefile Version %d", UTLang().strings[STR_VERSION_NUMBER]->sText, _VERSION_DATAFILE_);
#endif // ENABLE_GALAXY

	///--- startup commands (exe params) ---
#if defined(ENABLE_STEAM_WORKSHOP)
	if (g_startupCommand == GAME_STARTUP_UPLOAD_MOD)
	{
		GameState::ChangeTo(GAME_STATE_UPLOAD_MOD);
	}
#endif

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
		StringCchPrintf(wcsPath, MAX_PATH, L"%s/levels/missions/missions.xml", UTApp().g_wszAppResDir);
		UTGetChaptersList().LoadChapters(wcsPath);
		//load infinite tower mode desc
		//g_verticalMode.Init(&g_level, L"media/levels/mod_prefabs/infinite_tower.xml");
		//check CRC after loading chapters (levels needed)
		UINT32 unGameCRC = App_GetGameFilesCRC();
		//we loaded the descriptors
		//unGameCRC += g_verticalMode.GetFilesCRC(true);

		UTApp().m_Settings.dev_unCurrentCRC = unGameCRC;
		UTApp().m_Settings.dev_unCurrentModsCRC = unGameCRC;
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
	V_OP_RET(InitSound());

	//trigger resolution change event immediately
	CEvent *nevent = new CEvent(CEventTypes::evtT_SYSTEM, CEventCommands::evtC_SYSTEM_RESOLUTION_CHANGE);
	nevent->AddNamedArgUINT32(L"width", UTApp().m_Settings.nWindowW);
	nevent->AddNamedArgUINT32(L"height", UTApp().m_Settings.nWindowH);
	UTGetEventManager().TriggerEvent(nevent);
	///--- INITIALIZE 3D Device ---
	if (FAILED(DXUTCreateDevice(D3DADAPTER_DEFAULT, true, UTApp().m_Settings.nWindowW, UTApp().m_Settings.nWindowH, IsDeviceAcceptable, ModifyDeviceSettings)))
	{
		ErrorBox(K_ERR_CRITICAL, L"[ERROR] Couldn't create device (%dx%d)!\r\nTo fix it, check our support forum or contact us at %s\r\n", UTApp().m_Settings.nWindowW, UTApp().m_Settings.nWindowH, K_GAME_EMAIL);
	}
	
	if (OP_FAILED(AfterMount()))
	{
		ErrorBox(K_ERR_CRITICAL, L"Couldn't initialize game (AfterMount) !\r\nTo fix it, check our support forum or contact us at %s\r\n", K_GAME_EMAIL );
		return -1;
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
	UTApp().InitSDL(DXUTGetHWND());
	//add keyboard controllers and map keys
	CController* ctrlrkeys1 = UTGetCtrlrMgr().AddController(K_CM_CT_KBM_SDL, UTLang().strings[STR_KEYBOARD1]->sText);
	ctrlrkeys1->nSDLInstanceId = K_CM_IID_KBM1; //set keyboard instance ID so it isn't empty
	//ctrlrkeys1->ClearTriggers(); //clear default mapping

	//CController* ctrlrkeys2 = UTGetControllersManager().AddController(K_CM_CONTROLLERTYPE_KEYBOARD_SDL, UTLang().strings[STR_KEYBOARD2]->sText);
	//ctrlrkeys2->nSDLInstanceId = K_CM_DEFAULT_KEYBOARD2_INSTANCE_ID; //set keyboard instance ID so it isn't empty
	//ctrlrkeys2->ClearTriggers(); //clear default mapping

	//App_SetSDLTriggersFromUserData(ctrlrkeys1, ctrlrkeys2);
	
	//add network controller for coop play (used for peer controller simulation)
	CController* ctrlrnet1 = UTGetCtrlrMgr().AddController(K_CM_CT_NET_FRAMELOCK, UTLang().strings[STR_NETWORK1]->sText);
	ctrlrnet1->nSDLInstanceId = K_CM_IID_NET1;


	//find/add controllers if any
	UTGetCtrlrMgr().RegisterAllSDLControllers();

	//send analytics about gfx caps
	CHAR ctxt[MAX_PATH];
	StringCchPrintfA(ctxt, MAX_PATH, "GFXflags:%d", UTApp().g_gfxFlags);
	
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
	SIZEWH szwh(UTApp().m_Settings.nWindowW, UTApp().m_Settings.nWindowH);
	LOG(L"GFX:: Settings Resolution:%dx%d fullscreen:%d", szwh.w, szwh.h, UTApp().m_Settings.bFullscreen);
	if (UTApp().g_arrResolutions.IndexOf(szwh) < 0)
	{
		ErrorBox(K_ERR_WARNING, L"Unsupported window size found in settings (%d x %d)! Resetting to SAFE DEFAULTS!", szwh.w, szwh.h);
		//reset resolution
		UTApp().m_Settings.nWindowW = K_WINDOW_WIDTH_SAFE;
		UTApp().m_Settings.nWindowH = K_WINDOW_HEIGHT_SAFE;
		UTApp().m_Settings.bFullscreen = false;

		SetWindowPos(DXUTGetHWND(), 0, 0, 0, K_WINDOW_WIDTH_SAFE, K_WINDOW_HEIGHT_SAFE, SWP_NOOWNERZORDER | SWP_NOZORDER);
		UTApp().SaveSettings();
	}
	//--- start fullscreen? ---
	if (UTApp().m_Settings.bFullscreen)
	{
		if (UTApp().m_Settings.bBorderlessFullscreen)
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
	LOG(L"GFX [%s] GFXflags [%d]", DXUTGetDeviceStats(), UTApp().g_gfxFlags);

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
	UTApp().CloseSDL();

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
OPRESULT BeforeMount(void)
{
 ///--- Load strings here so we can set the window name ---
	if (OP_FAILED(App_LocaLoadLangList(UTApp().m_Settings.shLanguageAlias)))
	{
		ErrorBox(K_ERR_WARNING, L"[ERROR] Error loading strings list [texts/lang.xml]!");
	}
	//load strings for current language
	V_OP_RET(App_LocaLoadStrings());

	//--------------------------------------------------------------------------------------
	// setari initiale
	//--------------------------------------------------------------------------------------
	MUMatIdentity(&g_matIdentity);
	MUMatIdentity(&g_matWorld);
	g_vecGravityOld = Vec2(0.0f, K_GRAVITY);
	g_vecGravity = Vec3(0.0f, 0.0f, -K_GRAVITY);

	//set version number
	UTLang().SetString(STR_VERSION_NUMBER, L"v%d.%d.%d", _VERSION_MAJOR_, _VERSION_MINOR_, _VERSION_PATCH_);
	//--------------------------------------------------------------------------------------
	// add listeners
	//--------------------------------------------------------------------------------------
	//first listener must be UTAppClass
	UTGetEventManager().AddListener(&UTApp(), CEventTypes::evtT_SYSTEM);
	UTGetEventManager().AddListener(&UTApp(), CEventTypes::evtT_CONTROLS);
	UTGetEventManager().AddListener(&UTApp(), CEventTypes::evtT_GAMESTATE);
	//managerul de sunet
	UTGetEventManager().AddListener(&UTGetSoundManager(), CEventTypes::evtT_SOUND);

	//--------------------------------------------------------------------------------------
	// Script processors
	//--------------------------------------------------------------------------------------
	UTGetScriptManager().AddProcessor(&g_level);

	return K_OP_OK;
}


OPRESULT AfterMount(void)
{
	// load the minimum necessary to paint something (the sprites shader)
	WCHAR shpath[MAX_PATH];
	StringCchPrintf(shpath, MAX_PATH, L"%s/shaders/vs_sprites2d.vso", UTApp().g_wszAppResDir);
	if (OP_FAILED(UTGetShaderManager().AddVShader(shpath, L"VS_SPRITES2D")))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"Could not load SpritesVS!\n%s", shpath);
	}

	GameState::ChangeTo(GAME_STATE_PRELOAD);
	return K_OP_OK;
}


void ShutdownApp(void)
{
	g_editor.Release();
	UTLang().Release();
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

OPRESULT InitSound(void)
{
	// Initialize sound after we have the window
	//--- init sound system ---
	if (FAILED(UTGetSoundManager().Init(DXUTGetHWND(), 2, 44100, 16)))
	{
		return OPRESULT( K_OP_OK_WARNING, L"Failed INITSOUND->g_pSoundManager->Init()\nSOUNDS WILL BE DISABLED!\n", K_SEVERITY_WARNING );
	}

	UTGetSoundManager().EnablePositionalSounds(Vec2(0.0f, 0.0f), Vec2(UTApp().g_rectRT.w * 0.7f, UTApp().g_rectRT.h * 0.7f));
	UTGetSoundManager().SetListenerVolumeFadeStart(0.7f);

	return K_OP_OK;
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

		UTApp().g_gfxFlags &= ~K_UT_GFXFLAG_SEPARATEALPHABLEND;
	}
	else
	{
		UTApp().g_gfxFlags |= K_UT_GFXFLAG_SEPARATEALPHABLEND;
	}

	//--- does it support 2d clipping? ----
	if ((pCaps->RasterCaps & D3DPRASTERCAPS_SCISSORTEST) == 0)
	{
		ErrorBox(K_ERR_WARNING, L"Scissor Test not supported!");
		UTApp().g_gfxFlags &= ~K_UT_GFXFLAG_SCISSORTEST;
	}
	else
	{
		UTApp().g_gfxFlags |= K_UT_GFXFLAG_SCISSORTEST;
	}

	//request stencil buffer
	if (FAILED(pD3D->CheckDeviceFormat(pDeviceSettings->AdapterOrdinal, pDeviceSettings->DeviceType,
		pDeviceSettings->AdapterFormat, D3DUSAGE_DEPTHSTENCIL,
		D3DRTYPE_SURFACE, D3DFMT_D24S8)))
	{
		//TODO: showld fall back on 1bit stencil 
		ErrorBox(K_ERR_CRITICAL, L"8bit Stencil not supported!");

		UTApp().g_gfxFlags &= ~K_UT_GFXFLAG_8BITSTENCIL;
		UTApp().g_stencilBits = 0;
	}
	else
	{
		pDeviceSettings->pp.EnableAutoDepthStencil = TRUE;
		pDeviceSettings->pp.AutoDepthStencilFormat = D3DFMT_D24S8;

		UTApp().g_gfxFlags |= K_UT_GFXFLAG_8BITSTENCIL;
		UTApp().g_stencilBits = 8;
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
	if (OP_FAILED(UTApp().VerifyRequirements()))
	{
		DXUTShutdown();
		return S_OK;
	}

	UTGetTTFManager().OnCreateDevice(pDevice, pBBDesc);

	V_RETURN(UTApp().OnCreateDevice(pDevice, pBBDesc));
	V_OP_RETHR(UTGetRTManager().OnCreateDevice(pDevice, pBBDesc));
	UTimgui().OnCreateDevice(pDevice, pBBDesc);
	V_OP_RETHR(UTGetShaderManager().OnCreateDevice(pDevice, pBBDesc));
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
	UTGetSoundManager().EnablePositionalSounds(Vec2(0.0f, 0.0f), Vec2(UTApp().g_rectRT.w * 0.7f, UTApp().g_rectRT.h * 0.7f));

	HRESULT hr;

	// Create main game sprite
	V_RETURN(D3DXCreateSprite(pDevice, &g_pGameSprite));
	//should be first to be called here
	V_RETURN(UTApp().OnResetDevice(pDevice, pBBDesc));
	// Because the render targets and handled globally and are changing in size depending on screen resolution we just release them in OnLostDevice and re-create them in OnResetDevice
	V_OP_RETHR(UTGetRTManager().OnResetDevice(pDevice, pBBDesc));

	// Create necessary render targets when device gets reset (created or reset)
	UINT fGameHpx = K_GAME_HEIGHT * K_RT_PIXEL_SIZE;
	UINT fGameWpx = K_GAME_WIDTH * K_RT_PIXEL_SIZE;
	// Create RTs
	UTGetRTManager().AddRT(K_RTID_COLORDEPTHSTENCIL, fGameWpx, fGameHpx, 1, D3DFMT_A8R8G8B8, false);
	UTGetRTManager().AddRT(K_RTID_TEMP1, fGameWpx, fGameHpx, 1, D3DFMT_A8R8G8B8, false);
	UTGetRTManager().AddRT(K_RTID_FINAL, fGameWpx, fGameHpx, 1, D3DFMT_A8R8G8B8, false);
	//if (UTGetAppClass().m_Settings.nLOD_lights >= K_UT_LOD_MED)
		//UTGetRenderTargetsManager().AddRT(K_RTID_SPECULARMAP, fGameWpx, fGameHpx, 1, D3DFMT_A8R8G8B8, false);

	UTimgui().OnResetDevice(pDevice, pBBDesc);
	V_OP_RETHR(UTGetShaderManager().OnResetDevice(pDevice, pBBDesc));
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
	for (int kk = 0; kk < UTApp().g_arrResolutions.GetSize(); kk++)
	{
		WCHAR wsRes[MAX_PATH];
		if (kk < UTApp().g_arrResolutions.GetSize() - 1)
			StringCchPrintf(wsRes, MAX_PATH, L"%dx%d\n", UTApp().g_arrResolutions[kk].w, UTApp().g_arrResolutions[kk].h);
		else
			StringCchPrintf(wsRes, MAX_PATH, L"%dx%d", UTApp().g_arrResolutions[kk].w, UTApp().g_arrResolutions[kk].h);

		StringCchCat(wsResStr, 1024, wsRes);
	}
	UTLang().SetString(STR_RESOLUTIONS_LIST, wsResStr);


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

	UTApp().OnLostDevice();
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

	UTApp().OnDestroyDevice();
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
	UTApp().Update(fElapsedTime);
	//--- update clasa sunete pentru fade-uri ---
	UTGetSoundManager().Update(fElapsedTime);
	//-=-=-= controllers update =-=-=-
	//--must be called before updates
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

	// update main game engine
	g_game.Update( fElapsedTime, bSyncUpdate, g_nUpdateFrame );

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
	if (UTApp().IsGameNetworked())
		bAllowFullspeedUpdate = false;
#endif

	///--- networked game requested? reset sync data ---
	if ((UTApp().IsGameNetworked()) &&
		(UTApp().m_Settings.devnet_eSyncStatus == CApplicationSettings::K_NETGAME_SYNC_GET_READY))
	{
		UTApp().m_Settings.devnet_eSyncStatus = CApplicationSettings::K_NETGAME_SYNC_SYNCING;

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
	bool bSyncUpdate = (UTApp().m_Settings.devnet_eSyncStatus == CApplicationSettings::K_NETGAME_SYNC_SYNCING);

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
				UTApp().m_Settings.devnet_eSyncStatus = CApplicationSettings::K_NETGAME_SYNC_STOPPED;
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
			UTApp().PollSDLControllers();

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
			UTApp().PollSDLControllers();
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
						LOG(L"%s ID %d pos(%.4f, %.4f) decision(%.4f)", act->actTemplate.shID.text, act->ID, act->pos.xyz.x, act->pos.xyz.y, act->AItimerDecision);
					}

					//send analytics
					CHAR ctxt[MAX_PATH], ctxt2[MAX_PATH];
					StringCchPrintfA(ctxt, MAX_PATH, "mission_%d_%d_ver_%s", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1, _VERSION_CHARSTR_);
					StringCchPrintfA(ctxt2, MAX_PATH, "v%d_CRC[%08x]", _VERSION_INT_, UTApp().m_Settings.dev_unCurrentCRC);
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
		bSyncUpdate = (UTApp().m_Settings.devnet_eSyncStatus == CApplicationSettings::K_NETGAME_SYNC_SYNCING);

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
		GameState::UpdateTransition(fElapsedTime);

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
				UTLang().SetStringDescUTF8(&sdName, scoresList.m_arrNames[kk]);
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
			UTLang().SetString(STR_LEADERBOARDS_NAMES_VAL, strNames);
			UTLang().SetString(STR_LEADERBOARDS_SCORES_VAL, strScores);

			//save user score
			int nUserScore = UTGetLeaderboards().GetUserScore();
			if(nUserScore == 0)
				UTLang().SetString(STR_LEADERBOARDS_PLAYERSCORE_VAL, UTLang().strings[STR_NOT_AVAILABLE]->sText);
			else
				UTLang().SetString(STR_LEADERBOARDS_PLAYERSCORE_VAL, L"%d", nUserScore);
		}
		else
		{
			//no scores
			UTLang().SetString(STR_LEADERBOARDS_NAMES_VAL, UTLang().strings[STR_NOT_AVAILABLE]->sText);
			UTLang().SetString(STR_LEADERBOARDS_SCORES_VAL, UTLang().strings[STR_NOT_AVAILABLE]->sText);
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

	///--- check Float rounding mode wasn't changed ---
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	#ifdef WIN32
		_ASSERT((_controlfp(0, 0) & _MCW_PC) == _PC_24);
		_ASSERT((_controlfp(0, 0) & _MCW_RC) == _RC_NEAR);
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
	if ( !pDevice )
		return;

	///----------------------------------------------------------------------------------
	///	PART1. Paint the offscreen surfaces before the main render begin/end 
	///----------------------------------------------------------------------------------
	g_game.BeforePaint();

	///----------------------------------------------------------------------------------
	/// PART2. --- Render onscreen - FINAL PASS ---
	///----------------------------------------------------------------------------------
	if (OP_SUCCESS(UT3DBeginScene(pDevice)))
	{
		UT3DClear(pDevice, 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, K_GAME_CLEAR_COLOR, 1.0f, 0);

		g_pGameSprite->Begin(D3DXSPRITE_ALPHABLEND | /*D3DXSPRITE_OBJECTSPACE |*/ D3DXSPRITE_DONOTSAVESTATE);

		pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);

		// only start and end UTPainter after we preloaded the minimum painter shaders
		if (GameState::state != GAME_STATE_PRELOAD)
		{
			PVERTEXSHADER pSprVS = UTGetShaderManager().GetVShaderByName(L"VS_SPRITES2D");
			if (pSprVS)
				UTPainter().Begin(pSprVS, g_matIdentity);
		}

		///----------------------------------------------------------------------------------
		/// MAIN GAME PAINT
		///----------------------------------------------------------------------------------
		g_game.Paint( pDevice, g_pGameSprite, fElapsedTime );


#ifdef K_CONTROLS_EDITOR
		///--- controls editor paint ---
		if ( GameState::state == GAME_STATE_CONTROLSED )
		{
			g_ControlsEditor.Paint();
		}
#endif


		///--- chat window ---
#ifdef ENABLE_CHAT_WINDOW
		if ((UTApp().IsGameNetworked()) && ( GameState::state == GAME_STATE_GAME) && (g_level.m_levelState == K_LVL_STATE_PLAYING))
		{
			g_pGameSprite->Flush();
			CCameraTransform::SetActiveCamera(pDevice, &UTApp().g_camScreen);
			RECTXYWH_F camrectchat = UTApp().g_camScreen.GetCamWorldAABB();

			Vec2 vIgmIntSz = UTApp().g_cam360hScreen.WorldToScreen(Vec2(0.0f, 56.0f));
			g_ChatWnd.Paint(Vec2(camrectchat.x + 5.0f, camrectchat.Bottom() - vIgmIntSz.y));
			g_pGameSprite->Flush();
		}
#endif

		///--- GUI controls paint ---
#ifdef K_CONTROLS_EDITOR
		if ( GameState::state != GAME_STATE_CONTROLSED)
		{
			UTGetGUI().Paint();
			g_pGameSprite->Flush();
		}
#else
		UTGetGUI().Paint();
		g_pGameSprite->Flush();
#endif

		//real screen space
		CCameraTransform::SetActiveCamera(pDevice, &UTApp().g_camScreen);
		RECTXYWH_F camrect = UTApp().g_camScreen.GetCamWorldAABB();

		//--- TRANSITIONS ---		
		g_pGameSprite->Flush();
		GameState::PaintTransition(fElapsedTime, fTime, pDevice);
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
			SetRect(&rct, UTApp().g_rectRender.x, UTApp().g_rectRender.y, UTApp().g_rectRender.Right(), UTApp().g_rectRender.Bottom());
			DrawRectUP_TL1T(pDevice, rct, Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), color);

			g_font10b1->DrawString(STR_PAUSED, UTApp().g_rectRender.CenterX(), UTApp().g_rectRender.CenterY(), FONTFLAG_ANCHOR_BOTTOMCENTER, 0xffffffff);
		}
#endif

		///--- letterbox ---
		if ((UTApp().g_letterbox.w != 0.0f) || (UTApp().g_letterbox.h != 0.0f))
		{
			g_pGameSprite->Flush();
			//black poly over
			DWORD color = D3DCOLOR_COLORVALUE(0.0f, 0.0f, 0.0f, 1.0f);
			pDevice->SetTexture(0, NULL); //textura aiurea
			pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
			pDevice->SetTransform(D3DTS_VIEW, &g_matIdentity);

			RECT rct;
			if (UTApp().g_letterbox.w != 0.0f)
			{
				SetRect(&rct, UTApp().g_rectScreen.x, UTApp().g_rectScreen.y, UTApp().g_rectRender.x, UTApp().g_rectScreen.Bottom());
				DrawRectUP_TL1T(pDevice, rct, Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), color);
				SetRect(&rct, UTApp().g_rectRender.Right(), UTApp().g_rectScreen.y, UTApp().g_rectScreen.Right(), UTApp().g_rectScreen.Bottom());
				DrawRectUP_TL1T(pDevice, rct, Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), color);
			}
			else if (UTApp().g_letterbox.h != 0.0f)
			{
				SetRect(&rct, UTApp().g_rectScreen.x, UTApp().g_rectScreen.y, UTApp().g_rectScreen.Right(), UTApp().g_rectRender.y);
				DrawRectUP_TL1T(pDevice, rct, Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), color);
				SetRect(&rct, UTApp().g_rectScreen.x, UTApp().g_rectRender.Bottom(), UTApp().g_rectScreen.Right(), UTApp().g_rectScreen.Bottom());
				DrawRectUP_TL1T(pDevice, rct, Vec2(0.0f, 0.0f), Vec2(1.0f, 1.0f), color);
			}
		}

		///----- debug info -----
		if (g_bShowDebugStats)
		{
			g_pGameSprite->Flush();
			////reset transform
			pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
			CCameraTransform::SetActiveCamera(pDevice, &UTApp().g_camScreen);
			//find a pos so doesn't overlap with the igm interface
			Vec2 vStartPos = UTApp().g_cam360hScreen.WorldToScreen(Vec2(0.0f, 25.0f));
			int posY = vStartPos.y;
			WCHAR todraw[MAX_PATH] = { 0 };
			CStringDesc strdesc;

			if(UTApp().m_Settings.dev_bDevMode)
			{
				StringCchPrintf(todraw, MAX_PATH, DXUTGetFrameStats());
				UTLang().SetStringDesc(&strdesc, todraw);
				g_font10bs1->DrawString(&strdesc, 10, posY, FONTFLAG_ANCHOR_TOPLEFT, 0xffffffff);
				posY += 15;
				StringCchPrintf(todraw, MAX_PATH, L"pointer %.2f:%.2f", g_mouse.pos.x, g_mouse.pos.y);
				UTLang().SetStringDesc(&strdesc, todraw);
				g_font10bs1->DrawString(&strdesc, 10, posY, FONTFLAG_ANCHOR_TOPLEFT, 0xffffffff);
			}
			else  //no dev mode show only ping
			{
				//FPS and gfx data
				StringCchPrintf(todraw, MAX_PATH, DXUTGetFrameStats());
				UTLang().SetStringDesc(&strdesc, todraw);
				g_font10bs1->DrawString(&strdesc, 10, posY, FONTFLAG_ANCHOR_TOPLEFT, 0xffffffff);
				posY += 15;
			}
		}																

		//end game sprite
		g_pGameSprite->End();

		// end main painter
		if ( GameState::state != GAME_STATE_PRELOAD)
		{
			UTPainter().End();
		}

		//--- CONTROLS EDITOR PAINT ---
#ifdef K_CONTROLS_EDITOR
		if ( GameState::state == GAME_STATE_CONTROLSED)
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

		UT3DEndScene(pDevice);
	}

	///--- IMGUI UPDATE ---
	// must be last as it will enable and disable on user input
	if (UTimgui().BeginPaint())
	{
		// DEBUG IMGUI WINDOW
		if ( GameState::state == GAME_STATE_GAME)
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
				if ( GameState::state == GAME_STATE_GAME)
				{
					//ImGui::Text("Sortables: %d", g_level.m_visibleList.arrSortedItems.nCount);
					RECTXYWH_F		camrect = g_level.m_camLevelToRT.GetCamWorldAABB();
					ImGui::Text("Cam: X%.4f Y%.4f", FLOAT_FRAC(camrect.x), FLOAT_FRAC(camrect.y));
				}
				//ImGui::Text("Sprites: %d", UTPainter().stats_sprites);
				//ImGui::Text("Calls: %d", UTPainter().stats_calls);
				//ImGui::Text("Begin/End: %d", UTPainter().stats_sequences);
				//ImGui::Text("Flush Calls: %d", UTPainter().stats_flushes);
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
		if ( GameState::state == GAME_STATE_CONTROLSED)
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
			UTApp().SaveSettings();

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
			if (!UTApp().IsGameNetworked())
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
				if (UTApp().m_Settings.dev_bDevMode)
				{
					//SCRIPTS - hot reload
					if (bAltDown)
					{
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
						//if (GameState::state != GAME_STATE_GAME)
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
				if (( GameState::state == GAME_STATE_GAME) && (g_level.m_levelState == K_LVL_STATE_PLAYING))
				{
					if (bAltDown)
					{
						if (!UTApp().IsGameNetworked())
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
						UTApp().m_Settings.bShowInterfaceHelp = !UTApp().m_Settings.bShowInterfaceHelp;
					}
				}
			}
			break;

			///--- TAKES SCREENSHOTS ---
			case VK_F10:
			{
				if (( GameState::state != GAME_STATE_LOADING) && (!GameState::isTransitioning()))
				{
					if (FAILED(UTApp().SaveScreenshot()))
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
				if ((UTApp().IsGameNetworked()) && ( GameState::state == GAME_STATE_GAME) &&
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
				if ( GameState::state == GAME_STATE_CONTROLSED)
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



