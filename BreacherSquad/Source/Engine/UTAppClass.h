#pragma once
#include "ApplicationSettings.h"

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
	UINT32	g_gfxFlags;						// gfx flags for the game
	UINT32	g_stencilBits;					

	//screenshot utility
	HRESULT SaveScreenshot();

	///--------------------------------------------------------------------------------------
	/// Variabile globale legate de dimensiunea ecranului
	///--------------------------------------------------------------------------------------
	RectXYWH			g_rectScreen;		// real screen size
	RectXYWH			g_rectRender;		// rectangle that we render to (in actual final screen resolution after letterboxing)
	RectXYWH			g_rectRenderPP;		// rectangle that the level should render to so it scales with integers (in actual final screen coordinates)
	float				g_nPixelSizePP;		//#TODO: change to float for when not using pixel perfect. Pixel size in real pixels for when rendering with perfect pixel
	RectXYWH			g_rectRT;			// render target render rectangle
	RectXYWH			g_rect360hWorld;	// world rect for the 360h camera. W Computed depending on screen spect ratio.
	Matrix				g_matProj;			// projection matrix

	CCameraTransform	camScreen;			// real screen camera
	CCameraTransform	camScreen360h;		// camera for the screen with 360 pixels height and a width corresponding to the camScreen

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

public: //--- framework methods ---
	HRESULT OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL, void* pUserContext = NULL );
	HRESULT OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL, void* pUserContext = NULL );
	HRESULT OnLostDevice( void* pUserContext = NULL );
	HRESULT OnDestroyDevice( void* pUserContext = NULL );
};


//SINGLETON
CApplication& UTApp();