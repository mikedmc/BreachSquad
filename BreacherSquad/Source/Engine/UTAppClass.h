#pragma once

///--- CARD FALGS ---
// render to target caps
#define K_UT_GFXFLAG_RTT 1
#define K_UT_GFXFLAG_8BITSTENCIL 2
#define K_UT_GFXFLAG_SEPARATEALPHABLEND 4
#define K_UT_GFXFLAG_SCISSORTEST 8

///--- OPTIONS ---
#define K_UT_LOD_LOW	0
#define K_UT_LOD_MED	1
#define K_UT_LOD_HIGH	2


struct CGIGlobal {
	float radiance_render_extent;              // extent resolution.. output resolution will be SQUARE.
	float radiance_render_boost;               // How much to boost light levels.
	float radiance_render_decay;
};


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
	bool	bScreenShakes; //shake screen on explosions
	bool	bGoreEnabled;
	bool	bShowInterfaceHelp; //daca sa afiseze literele langa interfata
	//--- graphics ---
	int		nLOD_water;
	int		nLOD_shadows;
	int		nLOD_lights;
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

/*!
 * \brief Main Game Class
 * #TODO: could/should be moved to Game.cpp
 */
class CApplication : public IEventListener
{
public:
	///--- available resolutions ---
	CArray<SizeWHi> g_arrResolutions;  //available resolutions
	SizeWHi g_szDesktopSize;	//desktop resolution
public:
	CApplicationSettings	m_Settings;
	HRESULT SaveSettings();
	HRESULT LoadSettings();

	/* Tells if the current game was started as a network game */
	FORCEINLINE const bool IsGameNetworked() const {
		return (m_Settings.devnet_eNetGameType != CApplicationSettings::K_NETGAME_TYPE_NO_NETWORK);
	}
	/* Returns true if you have mods that change the game and could generate online desyncs */
	FORCEINLINE const bool IsGameModded() const {
		return (m_Settings.dev_unCurrentCRC != m_Settings.dev_unCurrentModsCRC);
	}
	/* Returns true if there are changes detected to the original game files */
	FORCEINLINE const bool IsGameModified() const {
		return (m_Settings.dev_unCurrentCRC != K_GAME_CRC);
	}

	///--- EVENTS ---
public:
	CGIGlobal	gi_global; //gi settings

	char const * GetListenerName( void ) { return "CApplication"; };
	bool HandleEvent( CEvent &nEvent );

	///--- useful paths ---
	WCHAR	g_wszExePath[MAX_PATH];			//absolute exe path
	WCHAR	g_wszModsDir[MAX_PATH];			//user mods folder 
	WCHAR	g_wszModsDirTemp[MAX_PATH];		//user mods temporary folder (safer location for unpacking)
	WCHAR	g_wszUserDataDir[MAX_PATH];		//user data folder 
	WCHAR	g_wszAppResDir[MAX_PATH];		//path to game's res folder "media" (no trailing separator)
	WCHAR	g_wszTempFilePath[MAX_PATH];	//path to temp file
	WCHAR	g_wszTempFolderPath[MAX_PATH];	//temp folder path, finishing with folder separator

	///--- card flags si minime de sistem ---
	UINT32	g_gfxFlags;						//flaguri importante pt joc
	UINT32	g_stencilBits;					//nr de biti disponibili pe stencil
	//float	procFreq;

	//screenshot utility
	HRESULT SaveScreenshot();

	///--------------------------------------------------------------------------------------
	/// Variabile globale legate de dimensiunea ecranului
	///--------------------------------------------------------------------------------------
	RectXYWH		g_rectScreen;		// real screen size
	RectXYWH		g_rectRender;		// rectangle that we render to (in actual final screen resolution after letterboxing)
	RectXYWH		g_rectRenderPP;		// rectangle that the level should render to so it scales with integers (in actual final screen coordinates)
	float			g_nPixelSizePP;		//#TODO: change to float for when not using pixel perfect. Pixel size in real pixels for when rendering with perfect pixel
	RectXYWH		g_rectRT;			// render target render rectangle
	RectXYWH		g_rect360hWorld;	// world rect for menus and interfaces. W Computed depending on screen spect ratio.
	Matrix				g_matProj;			// projection matrix
	//--- screen camera ---
	CCameraTransform g_camScreen;		//real screen camera
	CCameraTransform g_camRTScreen;		//game screen camera with height of RT targets (RT to screen)
	CCameraTransform g_cam360hScreen;	//360px high camera (scales up to real resolution) - 360px h is default resolution of the game
public:
	static bool			IsOnlyInstance( LPCTSTR className );
	void				OnRenderSizeChanged( int newSizeX, int newSizeY );

	void Init();
	void Update( float dTime );

	// returns rectangle where rendering should be made to considering the pixel perfect setting
	// you get g_rectRender or g_rectRenderPP
	RectXYWH getRenderRect();

	//--- SDL data ---
#if defined(K_GLOBAL_ENABLE_SDL)
	SDL_Window* gWindow;

	bool InitSDL( HWND hWnd );
	void CloseSDL();
	void PollSDLControllers();
#endif
	CApplication();
	~CApplication();

	///--- STEAM CALLBACKS ---
#if defined(ENABLE_STEAM)
private:
	STEAM_CALLBACK( CApplication, OnGameOverlayActivated, GameOverlayActivated_t );
#endif

	///----- Application properties -----
public:
	CTextureManager				g_texManager;		// global textures manager
	CSpriteLib					g_sprMgrGlobal;		// global sprite manager 

///----- Application states (not all of them are treated here) -----
	//#TODO: de facut o interfata gen IGameState si fiecare stare sa fie o clasa derivata din interfata respectiva si instantiata aici dar setat pointer pe currentState prin changeGameState
public:
	void App_EnterState_Loading();
	void App_UpdateState_Loading( LPDIRECT3DDEVICE9 pDevice, double fTimeline, float dTime );
	void App_PaintState_Loading( LPDIRECT3DDEVICE9 pDevice, ID3DXSprite* pSprite, double fTimeline );
	void App_ExitState_Loading();

	void App_EnterState_Developer();
	void App_UpdateState_Developer( LPDIRECT3DDEVICE9 pDevice, double fTimeline, float dTime );
	void App_PaintState_Developer( LPDIRECT3DDEVICE9 pDevice, ID3DXSprite* pSprite, double fTimeline );
	void App_ExitState_Developer();
	// Called after each finished level (win or lose or cancelled)
	void App_OnLevelFinished( int nEpisodeIdx, int nLevelIdx );

//-- GI: should be moved
	void radiance_initialize( float extent, float boost = 1.0, float decayrate = 0.65 );

public: //--- framework methods ---
	HRESULT OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL, void* pUserContext = NULL );
	HRESULT OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL, void* pUserContext = NULL );
	HRESULT OnLostDevice( void* pUserContext = NULL );
	HRESULT OnDestroyDevice( void* pUserContext = NULL );
};


//SINGLETON
CApplication& UTApp();