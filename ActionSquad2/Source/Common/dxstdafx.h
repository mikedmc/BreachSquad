#pragma once

#ifndef DXSDK_STDAFX_H
#define DXSDK_STDAFX_H

///----------------------
///--- CONTROL CENTER ---
///----------------------
//does the app have a pause screen?
#define K_GAME_HAS_PAUSE_SCREEN 1
//do we have very large pauses between frames? advance at 24fps minimum
#define K_MAX_TIMESTEP 0.0415f

//game CRC for current release
#define K_GAME_CRC 0xed27695c

//enables integration of STEAM libraries
#define ENABLE_STEAM
//enables integration of GOG.com libraries
//#define ENABLE_GALAXY

//enables chat window
#define ENABLE_CHAT_WINDOW
//enables fixed timestep on single player (necessary when vsync is disabled from the drivers to avoid FP precision issues)
#define ENABLE_FIXED_TIMESTEP_SINGLEPLAYER

//enables developer release mode (should be off in final version)
//#define ENABLE_DEVMODE_RELEASE										   

// if strict net check is defined it sends a checksum of all actors through the network
//#define K_NET_STRICT_SYNC_CHECK
// if set then the lobby gets closed when detected a coop sync
#define K_NET_DISCONNECT_ON_DESYNC
// set it when bandwidth is very limited, like on XBOX stress test (induces a bit of input lag but lowers the throughput at 4KBps)
//#define K_NET_FORCE_30FPS_SENDING
// set it when you want buffered output from the coop gameplay engine
//#define K_NET_ENGINE_DBG_VERBOSE
// enables debug output to check net coop desyncs
//#define K_SYNC_ENGINE_DBG_VERBOSE
// Enables IMGUI api
#define K_ENABLE_IMGUI

//important only in DEBUG mode:
#if defined(DEBUG) | defined(_DEBUG)						  
//reset all steam achievements on startup? 
//#define ENABLE_ACHIEVEMENTS_RESET_ON_STARTUP
//playerus invinctus:
//#define ENABLE_PLAYER_INVINCIBILITY
#endif

#if defined(ENABLE_DEVMODE_RELEASE)
//level camera scrolls with the mouse so we can check the level rendering
//#define ENABLE_LEVEL_SHOWCASE
#endif

//when enabled it checks if the game freezed during online gameplay
#define K_NET_CHECK_IF_NETSYNC_LOCKED
#define K_NET_CHECK_IF_NETSYNC_LOCKED_DURATION  10.0f

///----------------------
///----------------------

//remove some warnings
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define STRICT

// Works with Windows 2000 and later and Windows 98 or later
#undef _WIN32_IE
#undef WINVER
#undef _WIN32_WINDOWS
#undef _WIN32_WINNT
#define WINVER         0x0500 
#define _WIN32_WINDOWS 0x0410 
#define _WIN32_WINNT   0x0500 

//disable la warningul "possible loss of data"
#pragma warning (disable:4244)
//disable la warningul de nonstandard extension used
#pragma warning (disable:4238)

#include <windows.h>
#include <windowsx.h>
#include <tchar.h>
#include <assert.h>
#include <wchar.h>
#include <mmsystem.h>
#include <commctrl.h> // for InitCommonControls() 7
#include <shellapi.h> // for ExtractIcon()
#include <new.h>      // for placement new
#include <math.h>
#include <memory>
#include <limits.h>      
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <locale.h>

#include <lmcons.h>

#pragma warning(disable: 4995)
#include <stdio.h>
#include <shlobj.h>
//#include "ConfigDatabase.h"
//#include "ConfigManager.h"
#pragma warning(default: 4995)

//#DMC: set float rounding mode for framelock online play
#ifdef WIN32
#include <float.h>
#include <math.h>

// win32: we set /fp:strict, disable /Oi (Generate Intrinsic Functions) and set /arch:IA32 from the project settings

// these should already be on/off due to fp-strict compiler setting
//#pragma fenv_access (on)
//#pragma fp_contract (off)

#elif defined(__linux__)
#include <fpu_control.h>
#elif (defined(__APPLE__) && !TARGET_OS_IPHONE)
// osx (llvm)
#include <fenv.h>
#pragma STDC FENV_ACCESS ON
#endif

// Enable extra D3D debugging in debug builds if using the debug DirectX runtime.  
// This makes D3D objects work well in the debugger watch window, but slows down 
// performance slightly.
#if defined(DEBUG) | defined(_DEBUG)
#define D3D_DEBUG_INFO
#endif

#if defined(DEBUG) | defined(_DEBUG)
// include this line to enable the controls editor
#define K_CONTROLS_EDITOR
#endif

#define MAX_PATH_STD	MAX_PATH
//-- macrouri de stringuit alte macrouri - vezi versioning.h ---
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT __FILE__ ":" TOSTRING(__LINE__)
//opens a browser
#define WEBSITE_OPEN(wstrAddress) ShellExecuteW( NULL, L"open", wstrAddress, NULL, NULL, SW_SHOWNORMAL ); 
// Direct3D includes
#include <d3d9.h>
#include <d3dx9.h>
//#include <dxerr9.h>

/// --- IMGUI ---
#if defined(K_ENABLE_IMGUI)

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

#include "imgui/api/imgui.h"
#include "imgui/backends/imgui_impl_dx9.h"
#include "imgui/backends/imgui_impl_win32.h"
// callback for proc handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#else 
#define IMGUI_DISABLE
#endif

/// --- Get DX version ---
#pragma warning(disable: 4995)
#define INITGUID
#include <guiddef.h> //aici sunt puse GUID-urile
#include <dxdiag.h>
#pragma warning(default: 4995)

// strsafe.h deprecates old unsecure string functions.  If you 
// really do not want to it to (not recommended), then uncomment the next line and 
// comment out the following lines:
#define STRSAFE_NO_DEPRECATE
//#pragma deprecated(_tcsncpy)
//#pragma deprecated(_tcsncat)
#include <strsafe.h>

#include "DXUT.h"
#include "DXUTmisc.h"
#include "DXUTenum.h"

#ifndef V
	#define V(x)           { hr = x; if (FAILED(hr) ) { ErrorBoxFnW(K_ERR_CRITICAL,  __FILE__, __LINE__, L"FAILED(hr) HRESULT=%x", hr); } }
#endif
#ifndef V_RETURN
    #define V_RETURN(x)    { hr = x; if( FAILED(hr) ) { ErrorBoxFnW(K_ERR_CRITICAL,  __FILE__, __LINE__, L"FAILED(hr) HRESULT=%x", hr); return hr; } }
#endif

#ifndef SAFE_DELETE
    #define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
    #define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
    #define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

#ifndef SAFE_DELETE_GROWABLE_ARRAY
#define SAFE_DELETE_GROWABLE_ARRAY(p) { for(int xkx = 0; xkx < p.GetSize(); xkx++) { SAFE_DELETE(p[xkx]); } p.RemoveAll(); }
#endif

#ifndef SAFE_DELETE_STDVEC
#define SAFE_DELETE_STDVEC(p) { for(int xkx = 0; xkx < p.size(); xkx++) { SAFE_DELETE(p[xkx]); } p.clear(); }
#endif

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#define null NULL

#define DISALLOW_COPY(TypeName) \
  TypeName(const TypeName&)

#define DISALLOW_ASSIGN(TypeName) \
  void operator=(const TypeName&)

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);   \
  void operator=(const TypeName&)

// remove "insecure" warnings
#define _CRT_SECURE_NO_DEPRECATE 
//#define _CRT_SECURE_NO_WARNINGS

//#define _CRT_NONSTDC_NO_DEPRECATE

#include "../versioning/versioning.h"
#include "../Engine/dbgutil.h"
#include "OpResult.h"

#include "utils/UTMath.h"
#include "utils/PlatformTypes.h"

#define K_GAME_USERDATA_COMPANY_SUFFIX		L"\\PixelShard\\"
#define K_GAME_USERDATA_FOLDER_SUFFIX		L"ActionSquad2\\"
#define K_GAME_USERDATA_MODS_SUFFIX			L"mods\\"
#define K_GAME_USERDATA_MODS_SUFFIX_TEMP	L"AS2_mods_temp\\"
#define K_GAME_WINDOW_CLASSNAME				L"ActionSquad2WindowClass"
#define K_GAME_EMAIL						L"devteam@pixelshard.com"
#define K_GAME_CONTACT_URL					L"https://pixelshard.com/contact"

// DXUT DIALOGS are not used atm
#ifdef K_INCLUDE_DXUT_DIALOGS
#include "DXUTgui.h"
#include "DXUTsettingsDlg.h"
#endif


///--- generic states used for state machines ---
enum eGenericState {
	K_STATE_NOTSET = -1,
	K_STATE_READY = 0,
	K_STATE_EXECUTING = 1,
	K_STATE_FINISHED = 2,
};

///--- STARTUP COMMANDS ---
enum eStartupCommand {
	GAME_STARTUP_NONE = 0,
	GAME_STARTUP_UPLOAD_MOD = 1,
	GAME_STARTUP_JOIN_LOBBY,
	GAME_STARTUP_LOAD_MAP
};

///--- GAME STATES ---
enum eGameState {
	GAME_STATE_EMPTY = 0,
	GAME_STATE_PUBLISHER = 1,
	GAME_STATE_DEVELOPER,
	GAME_STATE_LOADING,
	GAME_STATE_SPLASH,

	GAME_STATE_MAINMENU,
	//networking
	GAME_STATE_NET_LOBBY, 
	GAME_STATE_JOIN_COOP_LIST,			//screen that shows and updates the available lobbies

	GAME_STATE_GAME_MODE_SELECTION,
	GAME_STATE_CHAPTER_SELECTION,
	GAME_STATE_LEVEL_SELECTION,
	GAME_STATE_PLAYER_SELECTION,
	GAME_STATE_GAME,
	//workshop - mod selection
	GAME_STATE_WORKSHOP,
	//controls editor
	GAME_STATE_CONTROLSED,
	//mod uploading to steam
	GAME_STATE_UPLOAD_MOD
};

///--- GAME MODES ---
enum eGameMode {
	GAME_MODE_CLASSIC = 0,
	GAME_MODE_ZOMBIE_INVASION,

	GAME_MODES_CNT
};

///--- RENDER TARGET IDs ---
enum ERTIDChannel {
	K_RTID_NONE = 0,

	K_RTID_COLORDEPTHSTENCIL = 1,	// has depth and stencil
	K_RTID_TEMP1 = 2,				// just colors
	K_RTID_FINAL = 3,				// just colors

	K_RTIDS_COUNT
};
///----------------------------------------------------
/// Texture channels for painting diffuse, normals, etc 
///----------------------------------------------------
enum ETexChannel {
	K_TEXCHAN_COLORMAP,
	K_TEXCHAN_NORMALMAP,
	K_TEXCHAN_SPECULARMAP
};

///--- CONSTANTE JOC ---
#define K_TILE_SIZE			16
#define K_TILE_HSIZE		8

// number of vertical visible tiles in a screen
#define K_GAME_HEIGHT_TL			15
// game scaling to final RT (pixel size)
#define K_GAME_PIXEL_SIZE			3
#define K_GAME_PIXEL_SIZE_F			3.0f

#define K_GAME_HEIGHT				(K_TILE_SIZE * K_GAME_HEIGHT_TL)
#define K_GAME_HALF_HEIGHT			(K_GAME_HEIGHT / 2)
#define K_GAME_WIDTH_MIN			(K_GAME_HEIGHT * K_WINDOW_ASPECT_RATIO_MIN)
#define K_GAME_WIDTH_MAX			(K_GAME_HEIGHT * K_WINDOW_ASPECT_RATIO_MAX)

///--- CLEAR COLOR ---
#define K_GAME_CLEAR_COLOR	0x00000066
// splashscreen show time
#define K_GAME_SPLASH_SHOW_TIMER 2.5f

// default resolution for when failing to get supported res list (1360x768) (1920x1080)
#define	K_WINDOW_WIDTH_SAFE		800
#define	K_WINDOW_HEIGHT_SAFE	600
// minimum resolution for the game (res list gets filtered by this)
#define	K_WINDOW_WIDTH_MIN		640
#define	K_WINDOW_HEIGHT_MIN		480
// aspect ratio limits
#define K_WINDOW_ASPECT_RATIO_MIN	(4.0f / 3.0f)
#define K_WINDOW_ASPECT_RATIO_MAX	(16.0f / 9.0f)

// max no of human players
#define K_MAX_PLAYERS_CNT	2

#ifndef uint64_t
typedef unsigned long long uint64_t;
#endif

#ifndef int64_t
typedef long long int64_t;
#endif

#ifdef ENABLE_GALAXY
#pragma comment(lib, "Galaxy.lib")

#include "galaxy/GalaxyID.h"

#define ENABLE_AUTO_JOIN_LOBBY
typedef galaxy::api::GalaxyID LobbyID;
typedef galaxy::api::GalaxyID NetID;

#include "galaxy/GalaxyApi.h"
// The following data is taken from GOG SDK Credentials in the devportal
static const char* GOG_CLIENT_ID = "52068914807629680";
static const char* GOG_CLIENT_SECRET = "416a364b92edd3ac24d9d8830e670d03de80e2770ac68fd44f8da2f5f7d6c9e0";

#endif //ENABLE_GALAXY

//--- STEAM is first to be included ---
#ifdef ENABLE_STEAM
	#define STEAM_APP_ID 1106210
	#define ENABLE_STEAM_WORKSHOP

	#include "steam.h"

	typedef uint64_t LobbyID;
	typedef CSteamID NetID;
#endif


//--- enable achievements and networking and generic stuff
#define ENABLE_ACHIEVEMENTS
#define ENABLE_NETWORKING
#define ENABLE_LEADERBOARDS

///--- constantele exportate de obicei din editoare ---
#include "constants\\strings.h"
#include "constants\\font_ids.h"
#include "constants\\sounds.h"
//exportate din editor
#include "constants\\lights_SPR.h"
#include "constants\\actives_SPR.h"
#include "constants\\particles_SPR.h"
#include "constants\\igm_interface_SPR.h"
#include "constants\\bk_city_SPR.h"
#include "constants\\controls_SPR.h"
#include "constants\\menus_SPR.h"
#include "constants\\title_screen_SPR.h"

//enable SDL support
#define K_GLOBAL_ENABLE_SDL
// comment out to use mouse events
//#define K_SDL_IGNORE_MOUSE_EVENTS

#if defined(K_GLOBAL_ENABLE_SDL)
	#include <SDL.h>
#endif

#include "MiniDump.h"
#include "List.h"
///--- enumerari ---
#include "BitPacker.h"
#include "DataTypes.h"
#include "enginecommon.h"
#include "CMathUtil.h"
#include "Randoms.h"
#include "GetDXVer.h"
#include "pugixml.hpp"
#include "FileManager.h"
#include "Chapters.h"

#include "log.h"
///--- analytics class ---
#include "analytics.h"
///--- events-constants (include before main classes)
#include "ConstEvents.h"
#include "ScriptManager.h"
#include "EventManager.h"

#include "RTManager.h"
#include "CameraTransform.h"

#include "ShaderManager.h"
#include "LibraryManager.h"
#include "StringsManager.h"

#include "ControllersManager.h"

#include "TextureManager.h"
#include "SpriteCollection.h"
#include "TexturedFont.h"

#include "TTFont.h"
#include "ChatWnd.h"

#include "Sprite.h"
#include "BufferedPainter.h"
#include "BufferedPainterQuad.h"
#include "PolyFOV.h"  //se ocupa de gasirea poligonului vizibil al unei lumini
#include "ParticlesManager.h"
#include "SoundManager.h"

#include "CustomInterfaces.h"
#include "ControlsManager.h" 
//main app class
#include "UTAppClass.h"
#include "imgui/imguiWrapper.h"
///--- Spine EsotericSoftware ---
//undefine min and max macros from windef.h because it conflicts with spine.mathutil
#undef min
#undef max

#include "spine/spine.h"
using namespace spine;
// Initialize default stuff so it can allocate and deallocate (uses malloc, free, FILE)
// Otherwise you can derrive from either SpineExtension or DefaultSpineExtension and override the _malloc, _calloc, _realloc, _free and _readFile methods.
#include "spine/Extension.h"

#include "spine/SpineManager.h"
#if defined(_DEBUG) || defined(DEBUG)
#include "spine/Debug.h"
#endif
//define min and max macros again
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
///--- game specific classes ---
#include "Shop.h"
#include "PlayerSelScr.h"
#include "Level.h"
#include "MainMenu.h"

#ifdef ENABLE_GALAXY
#include "galaxy/GalaxyApi.h"
#endif

///--- networking framelock classes ---
#include "Network.h"
#ifdef ENABLE_NETWORKING
	#ifdef ENABLE_STEAM
	#include "Steam/SteamMultiplayerSteamClientP2P.h"
	#endif // ENABLE_STEAM

	#ifdef ENABLE_GALAXY
	#include "GalaxyLocal/GalaxyMultiplayerClientP2P.h"
	#endif // ENABLE_GALAXY
#else 
//replaces actual network functionality with dummy (no functionality)
#include "NetworkDummy.h"
#endif

///--- actual networking class ---
#include "Netlock.h"

///--- achievements ---
#ifdef ENABLE_ACHIEVEMENTS
	#ifdef ENABLE_STEAM
	#include "Steam/SteamAchievements.h"
	// Do stats trigger achievements form the Steam/GOG server? If not they get enabled from the game
	#define K_AUTO_ACHIEVE_FROM_STATS
	#endif // ENABLE_STEAM

	#ifdef ENABLE_GALAXY
	#include "GalaxyLocal/GalaxyStatsAndAchievements.h"
	#endif // ENABLE_GALAXY

#include "AchievementManager.h"
#endif

///--- leaderboards ---

#ifdef ENABLE_LEADERBOARDS
	//#PORTING: enable next line to allow names selection in leaderboards
	//#define ENABLE_LEADERBOARDS_NAMES_SELECTION

	#ifdef ENABLE_STEAM
		//prefixes are used to find the leaderboard names for each level ([prefix][chapter].[level])
		#define K_GAME_STR_LEADERBOARDS_PREFIX_SP "solo.mission"
		#define K_GAME_STR_LEADERBOARDS_PREFIX_COOP "coop.mission"
		#define K_GAME_STR_LEADERBOARDS_VINFINITE_PREFIX_SP "solo.vinfinite"
		#define K_GAME_STR_LEADERBOARDS_VINFINITE_PREFIX_COOP "coop.vinfinite"
		//global leaderboard names
		#define K_GAME_STR_LEADERBOARDS_GLOBAL_SP "0.solo.global"
		#define K_GAME_STR_LEADERBOARDS_GLOBAL_COOP "0.coop.global"

		#include "Steam/SteamLeaderboards.h"
	#endif

	#ifdef ENABLE_GALAXY
		//prefixes are used to find the leaderboard names for each level ([prefix][chapter].[level])
		#define K_GAME_STR_LEADERBOARDS_PREFIX_SP "solo_mission"
		#define K_GAME_STR_LEADERBOARDS_PREFIX_COOP "coop_mission"
		#define K_GAME_STR_LEADERBOARDS_VINFINITE_PREFIX_SP "solo_vinfinite"
		#define K_GAME_STR_LEADERBOARDS_VINFINITE_PREFIX_COOP "coop_vinfinite"
		//global leaderboard names
		#define K_GAME_STR_LEADERBOARDS_GLOBAL_SP "0_solo_global"
		#define K_GAME_STR_LEADERBOARDS_GLOBAL_COOP "0_coop_global"

		#include "GalaxyLocal/GalaxyLeaderboards.h"
	#endif
#endif

///--- WORKSHOP ---
#include "Mods.h"
#ifdef ENABLE_STEAM_WORKSHOP
#include "Steam/SteamPublish.h"
#include "Steam/SteamUpdatePublished.h"
#include "Steam/SteamSubscriptions.h"
#endif

#include "AppGlobals.h"

#ifdef K_CONTROLS_EDITOR
	#include "..\\ControlsEd\\ControlsEditor.h"
#endif

///--------------------------------------------------------------------------------------
/// EXTERNALS - GLOBALS
///--------------------------------------------------------------------------------------
//text color multiplier
#define K_COLOR_DEFAULT_TEXT 0xff5a8dba
#define K_COLOR_DEFAULT_TEXT_HALFALPHA 0xaa5a8dba
#define K_COLOR_DEFAULT_TEXT_LIGHTER 0xff8fc7f8
#define K_COLOR_SELECTED_TEXT 0xffffffff
#define K_COLOR_WINDOW_TITLE 0xff212c35

extern CLog*				g_pLog;		//log class

extern bool					g_bCanPause;  //poate pune pauza?
extern D3DXMATRIXA16		g_matIdentity;
extern D3DXMATRIXA16		g_matWorld;
extern D3DXVECTOR2			g_vecGravity;
//mouse
extern CMouseData			g_mouse;
//gamestate
extern UINT32	g_gameState;	//state machine's current state. defined in dxstdafx.h 
extern UINT32	g_gameSubstate; //current state's substate - if needed
extern eGameMode g_gameMode;   //current selected game mode

extern eStartupCommand	g_startupCommand;
extern CStringHash		g_startupParam;

extern int		g_gameStateErrorStringIdx;
extern float	g_gameStateTimer; //timer folosit uneori
/*
* --- transitions ---
*/
#define K_TRANSITION_TYPE_NONE 0
#define K_TRANSITION_TYPE_SIMPLE 1
#define K_TRANSITION_TYPE_PIXELATE 2

extern bool g_bDuringTransition;
extern void ChangeGameState(int newState, int param1, int param2);
extern void ChangeGameStateTransition(int newState, int param1, int param2, int transitionType);
//particles
extern CTimersArray			g_timers;
extern CParticlesManager	g_particlesMgr;
extern CStringsManager		g_stringsMgr;
extern CPlayerSelScr		g_playerSelScr;

extern bool     g_bShowHelp;
extern bool		g_bLevelNeedsUpdate;

//fonts
extern CTexturedFont	*g_font12wow;
extern CTexturedFont	*g_font10b1, *g_font10bs1;
extern CTexturedFont	*g_font8b1, *g_font8bs1;
extern CTexturedFont	*g_font9b1;
extern CTexturedFont	*g_font6n1, *g_font6ns1, *g_font6nc1;
extern CTexturedFont	*g_font5n1, *g_font5n2, *g_font5ns2;

#ifdef K_CONTROLS_EDITOR
extern CControlsEditor				g_ControlsEditor;
#endif
//--- redefine keys ---
extern EControllerCommand g_keydef_command;
extern int g_keydef_scancode;

extern CNetLock						g_netlock;

extern CLevel						g_level;		
extern CMainMenu					g_mainMenu;	

#ifdef ENABLE_CHAT_WINDOW
extern CChatWnd						g_ChatWnd;
#endif

#endif