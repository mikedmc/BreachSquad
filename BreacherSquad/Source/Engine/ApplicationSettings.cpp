#include "dxstdafx.h"
#include "ApplicationSettings.h"

CApplicationSettings::CApplicationSettings()
{
	fMusicVolume = 0.7f;
	fSoundsVolume = 0.9f;

	nWindowW = GetSystemMetrics( SM_CXSCREEN );
	nWindowH = GetSystemMetrics( SM_CYSCREEN );

	bFullscreen = true;
	bBorderlessFullscreen = true;
	bPixelPerfect = true;
	bScreenShakes = true;
	bGoreEnabled = true;
	bShowInterfaceHelp = false;
	bEnableGI = false;

	//network flags
	devnet_eNetGameType = K_NETGAME_TYPE_NO_NETWORK;
	devnet_eSyncStatus = K_NETGAME_SYNC_STOPPED;

	//CRC
	dev_unCurrentCRC = 0;
	dev_unCurrentModsCRC = 0;

#if defined(_DEBUG) || defined(DEBUG)
	dev_bDebugEnabled = true;
#else
	dev_bDebugEnabled = false;
#endif

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	dev_bDevMode = true;
	dev_bDevMode_forced = false; //always false

	dev_bLogWindowShow = true;
	dev_bLogWriteToFile = true;
	dev_bLogShowInDebugOutput = true;
#else
	dev_bDevMode = false;
	dev_bDevMode_forced = false; //always false

	dev_bLogWindowShow = false;
	dev_bLogWriteToFile = true;
	dev_bLogShowInDebugOutput = false;
#endif

#ifdef ENABLE_GALAXY
	galaxyFullyLoaded = false;
#endif

}

