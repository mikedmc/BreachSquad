#pragma once

class CApplicationSettings {
public:
	enum eNetGameTypes {
		K_NETGAME_TYPE_NO_NETWORK = 0,
		K_NETGAME_TYPE_QUICK_MATCH = 1,
		K_NETGAME_TYPE_HOST_PUBLIC,
		K_NETGAME_TYPE_HOST_PRIVATE,

		K_NETGAME_TYPES_CNT
	};
	//types of sync states
	enum eNetSyncStatus {
		K_NETGAME_SYNC_STOPPED,	//stopped, usually after syncing is no longer necessary or before syncing
		K_NETGAME_SYNC_GET_READY,   //if GET_READY goes to SYNCING with first occasion, after initializing
		K_NETGAME_SYNC_SYNCING,	//syncing game state

		K_NETGAME_SYNC_CNT
	};

public:
	UINT32	nWindowW, nWindowH;
	float	fMusicVolume;
	float	fSoundsVolume;
	bool	bFullscreen;
	bool	bPixelPerfect;
	bool    bBorderlessFullscreen;
	bool	bScreenShakes;		//shake screen on explosions
	bool	bGoreEnabled;
	bool	bShowInterfaceHelp; // show command keys next to interface buttons
	bool	bEnableGI;			// should global illumination be enabled
	//--- language/loca ----
	CStringHash shLanguageAlias;			//current options language alias

	//--- developer settings (code only, not saved) ---
	bool	dev_bDebugEnabled;				//only set by engine. Never set it yourself!
	bool	dev_bDevMode;					//are we on dev mode
	bool	dev_bDevMode_forced;			//devMode was set from the options file (when true it saves the devModeOn setting in the options xml)

	bool	dev_bLogWindowShow;		//use/show log window
	bool	dev_bLogWriteToFile;	//write all info from log window to the log file (even with the window closed)
	bool	dev_bLogShowInDebugOutput;	//writes all log info to the debug line too
	//CRC - modding - not saved in files
	UINT32	dev_unCurrentCRC;		//current game CRC
	UINT32	dev_unCurrentModsCRC;	//current modded CRC

	//--- network flags ---
	eNetGameTypes	devnet_eNetGameType;		//type of network game (none for sigle player)
	eNetSyncStatus	devnet_eSyncStatus;			//commands the starting of syncing

#ifdef ENABLE_GALAXY
	bool			galaxyFullyLoaded;
#endif

public:
	CApplicationSettings();
};

