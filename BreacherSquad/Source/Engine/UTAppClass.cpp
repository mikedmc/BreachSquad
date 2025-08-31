#include "dxstdafx.h"

///**************************************************************************************
/// Game options class
///**************************************************************************************

CApplicationSettings::CApplicationSettings()
{
	fMusicVolume = 0.7f;
	fSoundsVolume = 0.9f;

	nWindowW = GetSystemMetrics(SM_CXSCREEN);
	nWindowH = GetSystemMetrics(SM_CYSCREEN);

	bFullscreen = true;
	bBorderlessFullscreen = true;
	bPixelPerfect = true;
	bScreenShakes = true;
	bGoreEnabled = true;
	bShowInterfaceHelp = false;

	nLOD_water = K_UT_LOD_HIGH;
	nLOD_shadows = K_UT_LOD_MED;
	nLOD_lights = K_UT_LOD_HIGH;

	//network flags
	devnet_eNetGameType = K_NETGAME_TYPE_NO_NETWORK;
	devnet_eSyncStatus = K_NETGAME_SYNC_STOPPED;

	//CRC
	dev_unCurrentCRC = 0;
	dev_unCurrentModsCRC = 0;

	//LANG/LOCA	- default on english
	shLanguageAlias.Init("notset");

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


///**************************************************************************************
/// Main Game Class
/// - listens to message dispatch
/// - holds some global game vars
///**************************************************************************************

CApplication::CApplication()
{
	//zeroing in
	ZeroMemory(g_wszExePath, sizeof(WCHAR) * MAX_PATH);
	ZeroMemory(g_wszModsDir, sizeof(WCHAR) * MAX_PATH);
	ZeroMemory(g_wszModsDirTemp, sizeof(WCHAR) * MAX_PATH);
	ZeroMemory(g_wszAppResDir, sizeof(WCHAR) * MAX_PATH);
	ZeroMemory(g_wszUserDataDir, sizeof(WCHAR) * MAX_PATH);
	ZeroMemory(g_wszTempFilePath, sizeof(WCHAR) * MAX_PATH);
	ZeroMemory(g_wszTempFolderPath, sizeof(WCHAR) * MAX_PATH);
	//get desktop resolution
	g_szDesktopSize.w = GetSystemMetrics(SM_CXSCREEN);
	g_szDesktopSize.h = GetSystemMetrics(SM_CYSCREEN);

	g_gfxFlags = 0;
	g_stencilBits = 0;

#if defined(K_GLOBAL_ENABLE_SDL)
	gWindow = nullptr;
#endif

	// keep real screen and virtual screen sizes
	g_rectRender = RectXYWH(0.0f, 0.0f, g_szDesktopSize.w, g_szDesktopSize.h);
	g_nPixelSizePP = 2;
	g_rectRenderPP = g_rectRender;
	g_rectScreen = RectXYWH(0.0f, 0.0f, g_szDesktopSize.w, g_szDesktopSize.h);
	g_rectRT = RectXYWH(0.0f, 0.0f, K_GAME_WIDTH * K_RT_PIXEL_SIZE_F, K_GAME_HEIGHT * K_RT_PIXEL_SIZE_F);
	g_rect360hWorld = RectXYWH(0.0f, 0.0f, ((g_rectRender.w / g_rectRender.h) * K_GAME_HEIGHT), K_GAME_HEIGHT);
	MUMatOrthoOffCenterLH(&g_matProj, g_rectRender.x + 0.5f, g_rectRender.w + 0.5f, g_rectRender.h + 0.5f, g_rectRender.y + 0.5f, 0.0f, 1.0f);
	//clear all resolutions
	g_arrResolutions.RemoveAll();
}

CApplication::~CApplication()
{
	g_arrResolutions.RemoveAll();
}

HRESULT CApplication::SaveScreenshot()
{
	HRESULT hr = S_OK;
	LPDIRECT3DDEVICE9 pd3dDevice = DXUTGetD3DDevice();
	if (pd3dDevice == nullptr)
	{
		ErrorBox(K_ERR_DEBUGOUT, L"Failed getting d3dDevice in SaveScreenshot()\n");
		return S_FALSE;
	}

	LPDIRECT3DSURFACE9 pBackBuffer = nullptr;
	//D3DSURFACE_DESC d3dsd;
	V_RETURN(pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer));

	SYSTEMTIME systime;
	GetLocalTime(&systime);
	WCHAR szFileName[MAX_PATH];
	StringCchPrintf(szFileName, MAX_PATH, L"BreacherSquad_%u_%u_%u_%u_%u_%u", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

	WCHAR szFullFileName[MAX_PATH];
	StringCchPrintf(szFullFileName, MAX_PATH, L"%s%s.png", UTApp().g_wszUserDataDir, szFileName);

	V_RETURN(D3DXSaveSurfaceToFile(szFullFileName, D3DXIFF_PNG, pBackBuffer, nullptr, nullptr));

	pBackBuffer->Release();
	return S_OK;
}


void CApplication::Init()
{
	HRESULT hr = S_OK;

	///--- path to exe ---
	GetExePathW(g_wszExePath, ARRAY_SIZE(g_wszExePath));
	///--- temp path ---
	hr = ::GetTempPath(MAX_PATH, g_wszTempFolderPath);
	StringCchPrintf(g_wszTempFilePath, MAX_PATH, L"%sfile.tmp", g_wszTempFolderPath);

	///---path to app data ---
	//CSIDL_PERSONAL - my documents, locked sometimes
	//CSIDL_LOCAL_APPDATA - appData/Local/etc
	//SHGFP_TYPE_CURRENT vs SHGFP_TYPE_DEFAULT - user/admin set value vs system default value
	hr = ::SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA, nullptr, SHGFP_TYPE_DEFAULT, g_wszUserDataDir);
	bool bFolderError = false;
	if (SUCCEEDED(hr))
	{
		//create company folder
		StringCchCat(g_wszUserDataDir, MAX_PATH, K_GAME_USERDATA_COMPANY_SUFFIX);
		int errval = ::CreateDirectory(g_wszUserDataDir, nullptr);
		if (errval == 0)//error
		{
			DWORD errcode = GetLastError();
			if (errcode != 183) //already exists error (no error)
				bFolderError = true;
		}
		//create game subfolder
		if (!bFolderError)
		{
			StringCchCat(g_wszUserDataDir, MAX_PATH, K_GAME_USERDATA_FOLDER_SUFFIX);
			errval = ::CreateDirectory(g_wszUserDataDir, nullptr);
			if (errval == 0)//error
			{
				DWORD errcode = GetLastError();
				if (errcode != 183) //already exists error (no error)
					bFolderError = true;
			}
		}
#ifdef ENABLE_STEAM_WORKSHOP
		//create mods subfolder
		if (!bFolderError)
		{
			StringCchPrintf(g_wszModsDir, MAX_PATH, L"%s%s", g_wszUserDataDir, K_GAME_USERDATA_MODS_SUFFIX);
			errval = ::CreateDirectory(g_wszModsDir, nullptr);
			if (errval == 0)//error
			{
				DWORD errcode = GetLastError();
				if (errcode != 183) //already exists error (no error)
					bFolderError = true;
			}
		}
		//create mods temp folder
		if (!bFolderError)
		{
			StringCchPrintf(g_wszModsDirTemp, MAX_PATH, L"%s%s", g_wszTempFolderPath, K_GAME_USERDATA_MODS_SUFFIX_TEMP);
			errval = ::CreateDirectory(g_wszModsDirTemp, nullptr);
			if (errval == 0)//error
			{
				DWORD errcode = GetLastError();
				if (errcode != 183) //already exists error (no error)
					bFolderError = true;
			}
		}
#endif
		if(bFolderError)
		{
			ErrorBox(K_ERR_CRITICAL, L"[ERROR] CApplication::Init(): Couldn't open/create user data directory!\nControlled Folder Access is enabled or your antivirus is blocking the access!");
		}
	}
	else
	{
		ErrorBox(K_ERR_CRITICAL, L"[ERROR] CApplication::Init(): Couldn't get user data directory!");
	}

	//set temp file and data folder
	StringCchPrintf(g_wszAppResDir, MAX_PATH, L"%smedia", g_wszExePath);


	//-- set cam animation ---
	//camera
	g_camScreen.SetCamAnimationNone();
	g_camRTScreen.SetCamAnimationNone();
	g_camRTScreen.SetPixelPerfect(true);

	g_cam360hScreen.SetCamAnimationNone();
	g_cam360hScreen.SetPixelPerfect(true);

	//-- resolutions --
	g_arrResolutions.RemoveAll();
}

void CApplication::OnRenderSizeChanged(int newSizeX, int newSizeY)
{
	g_rectScreen = RectXYWH(0.0f, 0.0f, (float)newSizeX, (float)newSizeY);
	// limit aspect ratio between min and max (4/3 si 16/9)
	float fAspectReal = (g_rectScreen.w / g_rectScreen.h);
	float fAspect = LIMIT(fAspectReal, K_WINDOW_ASPECT_RATIO_MIN, K_WINDOW_ASPECT_RATIO_MAX);
	float fAspectInv = 1.0f / fAspect;
	SizeWH szRender( (float)newSizeX, (float)newSizeY );
	///--- pixel perfect rendering ---
	SizeWH szRenderPP;
	const float ReferenceResolutionY = ( float ) K_GAME_TARGET_RESOLUTION_H;
	// Calculate the new art scale factor
	float minDiff = ReferenceResolutionY;
	// decide best pixel size for pixel perfect results
	g_nPixelSizePP = 1;
	for ( int kk = 1; kk < 10; kk++ )
	{
		float deltaH = fabs( newSizeY - kk * ReferenceResolutionY );
		if ( deltaH < minDiff )
		{
			minDiff = deltaH;
			g_nPixelSizePP = kk;
		}
	}
	//#TEMP: force pixel size to 3x to test clipping when source is larger
	//6g_nPixelSizePP = 3;

	szRenderPP.w = K_GAME_WIDTH * K_RT_PIXEL_SIZE * g_nPixelSizePP;
	szRenderPP.h = K_GAME_HEIGHT * K_RT_PIXEL_SIZE * g_nPixelSizePP;
	if ( szRenderPP.w > newSizeX )
		szRenderPP.w = newSizeX;
	if ( szRenderPP.h > newSizeY )
		szRenderPP.h = newSizeY;
	// compute final render rectangle
	if ( fAspectReal < K_WINDOW_ASPECT_RATIO_MIN )
	{
		szRender.w = ( float ) newSizeX;
		szRender.h = fAspectInv * newSizeX;
	}
	else if ( fAspectReal > K_WINDOW_ASPECT_RATIO_MAX )
	{
		szRender.h = ( float ) newSizeY;
		szRender.w = fAspect * newSizeY;
	}
	else
	{
		szRender.w = ( float ) newSizeX;
		szRender.h = ( float ) newSizeY;
	}

	SizeWH letterbox( (float)(newSizeX - szRender.w) / 2.0f, (float)(newSizeY - szRender.h) / 2.0f );
	//#TODO: must set rectRender to whatever we need (pixel perfect or stretched)
	g_rectRender = RectXYWH(letterbox.w, letterbox.h, szRender.w, szRender.h);
	g_rectRenderPP = RectXYWH( floor( ( newSizeX - szRenderPP.w ) / 2.0f ), floor( ( newSizeY - szRenderPP.h ) / 2.0f ), szRenderPP.w, szRenderPP.h );

	g_rect360hWorld = RectXYWH(0.0f, 0.0f, (fAspect * K_GAME_HEIGHT), K_GAME_HEIGHT);
	g_rectRT = RectXYWH( 0.0f, 0.0f, K_GAME_WIDTH * K_RT_PIXEL_SIZE_F, K_GAME_HEIGHT * K_RT_PIXEL_SIZE_F );
	MUMatOrthoOffCenterLH(&g_matProj, g_rectScreen.x + 0.5f, g_rectScreen.w + 0.5f, g_rectScreen.h + 0.5f, g_rectScreen.y + 0.5f, 0.0f, 1.0f);

	g_camScreen.SetWorldBounds(g_rectScreen, true, K_CAMTRANS_AXIS_V, g_rectScreen.h, g_rectScreen.h);
	g_camScreen.InitCamera(g_rectScreen, g_rectScreen.h, K_CAMTRANS_AXIS_V, g_rectScreen.Center());

	g_camRTScreen.SetWorldBounds(g_rectRT, true, K_CAMTRANS_AXIS_V, K_GAME_HEIGHT * K_RT_PIXEL_SIZE_F, K_GAME_HEIGHT * K_RT_PIXEL_SIZE_F);
	g_camRTScreen.InitCamera(g_rectRender, K_GAME_HEIGHT * K_RT_PIXEL_SIZE_F, K_CAMTRANS_AXIS_V, g_rectRT.Center());

	g_cam360hScreen.SetWorldBounds(g_rect360hWorld, true, K_CAMTRANS_AXIS_V, g_rect360hWorld.h, g_rect360hWorld.h);
	g_cam360hScreen.InitCamera(g_rectRender, g_rect360hWorld.h, K_CAMTRANS_AXIS_V, g_rect360hWorld.Center());

	//#HACK: set main flag for resolution change 
	g_bLevelNeedsUpdate = true;
}

void CApplication::Update(float dTime)
{
	g_camScreen.Update(dTime);
	g_camRTScreen.Update(dTime);
	g_cam360hScreen.Update(dTime);
}

RectXYWH CApplication::getRenderRect()
{
	if ( m_Settings.bPixelPerfect )
		return g_rectRenderPP;
	return g_rectRender;
}

bool CApplication::IsOnlyInstance(LPCTSTR className)
{
	// Find the window.  If active, set and return false
	// Only one game instance may have this mutex at a time...
	/*HANDLE handle = */CreateMutex(nullptr, TRUE, className);

	// Does anyone else think 'ERROR_SUCCESS' is a bit of a dichotomy?
	if (GetLastError() != ERROR_SUCCESS)
	{
		HWND hWnd = FindWindow(className, nullptr);
		if (hWnd)
		{
			// An instance of your game is already running.
			ShowWindow(hWnd, SW_SHOWNORMAL);
			SetFocus(hWnd);
			SetForegroundWindow(hWnd);
			SetActiveWindow(hWnd);
			return false;
		}
	}
	return true;
}

HRESULT CApplication::SaveSettings()
{
	pugi::xml_document doc;

	pugi::xml_node optionsNode = doc.append_child(L"OPTIONS");
	//children of OPTIONS
	pugi::xml_node soundsNode = optionsNode.append_child(L"SOUND");
	pugi::xml_node graphicsNode = optionsNode.append_child(L"GRAPHICS");
	pugi::xml_node miscNode = optionsNode.append_child(L"MISC");

	///misc
	miscNode.append_attribute(L"Language");
	miscNode.attribute(L"Language").set_value(m_Settings.shLanguageAlias.text);
	///sfx
	soundsNode.append_attribute(L"SoundsVolume");
	soundsNode.attribute(L"SoundsVolume").set_value(m_Settings.fSoundsVolume);
	soundsNode.append_attribute(L"MusicVolume");
	soundsNode.attribute(L"MusicVolume").set_value(m_Settings.fMusicVolume);
	///--- graphics options ---
	graphicsNode.append_attribute(L"nWindowW");
	graphicsNode.attribute(L"nWindowW").set_value(m_Settings.nWindowW);
	graphicsNode.append_attribute(L"nWindowH");
	graphicsNode.attribute(L"nWindowH").set_value(m_Settings.nWindowH);

	graphicsNode.append_attribute(L"bFullscreen");
	graphicsNode.attribute(L"bFullscreen").set_value(m_Settings.bFullscreen);
	graphicsNode.append_attribute( L"bPixelPerfect" );
	graphicsNode.attribute( L"bPixelPerfect" ).set_value( m_Settings.bPixelPerfect);
	graphicsNode.append_attribute(L"bBorderless");
	graphicsNode.attribute(L"bBorderless").set_value(m_Settings.bBorderlessFullscreen);
	graphicsNode.append_attribute(L"bScreenShakes");
	graphicsNode.attribute(L"bScreenShakes").set_value(m_Settings.bScreenShakes);
	graphicsNode.append_attribute(L"bGoreEnabled");
	graphicsNode.attribute(L"bGoreEnabled").set_value(m_Settings.bGoreEnabled);
	graphicsNode.append_attribute(L"bShowInterfaceHelp");
	graphicsNode.attribute(L"bShowInterfaceHelp").set_value(m_Settings.bShowInterfaceHelp);

	graphicsNode.append_attribute(L"LOD_water");
	graphicsNode.attribute(L"LOD_water").set_value(m_Settings.nLOD_water);
	graphicsNode.append_attribute(L"LOD_shadows");
	graphicsNode.attribute(L"LOD_shadows").set_value(m_Settings.nLOD_shadows);
	graphicsNode.append_attribute(L"LOD_lights");
	graphicsNode.attribute(L"LOD_lights").set_value(m_Settings.nLOD_lights);
	//saving bDevMode only if initially set from options.xml
	if (m_Settings.dev_bDevMode_forced)
	{
		graphicsNode.append_attribute(L"bDevMode");
		graphicsNode.attribute(L"bDevMode").set_value(m_Settings.dev_bDevMode_forced);
	}

	WCHAR sPath[MAX_PATH];
	StringCchPrintf(sPath, MAX_PATH, L"%soptions.xml", g_wszUserDataDir);

	FILE* file = OS_wfopen(sPath, L"w");
	if (file)
	{
		pugi::xml_writer_file writer(file);
		doc.save(writer);
	}
	else
	{
		ErrorBox(K_ERR_ONSCREEN, L"[WARNING] Failed to save settings to file: %s\nMake sure the game has access to the file and that your antivirus solution isn't blocking the game's access.", g_wszUserDataDir);
	}

	OS_fclose(file);

	return S_OK;
}

HRESULT CApplication::LoadSettings()
{
	WCHAR sPath[MAX_PATH];
	StringCchPrintf(sPath, MAX_PATH, L"%soptions.xml", g_wszUserDataDir);

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(sPath);
	if(result.status != pugi::status_ok)
	{
		//file doesn't exist so try and create it now
		if (result.status == pugi::status_file_not_found)
		{
			SaveSettings();
		}
		else 
		{
 			ErrorBox(K_ERR_ONSCREEN, L"[WARNING] Failed to load settings from file: %s\nMake sure the game has access to the file and that your antivirus solution isn't blocking the game's access.", g_wszUserDataDir);
		}

		return E_FAIL;
	}

	pugi::xml_node optionsNode = doc.child(L"OPTIONS");
	//children of OPTIONS:
	pugi::xml_node soundsNode = optionsNode.child(L"SOUND");
	pugi::xml_node graphicsNode = optionsNode.child(L"GRAPHICS");
	pugi::xml_node miscNode = optionsNode.child(L"MISC");

	///--- options ---
	m_Settings.shLanguageAlias.Init(miscNode.attribute(L"Language").value());
	///--- sound ---
	m_Settings.fSoundsVolume = soundsNode.attribute(L"SoundsVolume").as_float();
	m_Settings.fMusicVolume = soundsNode.attribute(L"MusicVolume").as_float();
	///--- graphics ---
	//resolution
	m_Settings.nWindowW = graphicsNode.attribute(L"nWindowW").as_uint();
	m_Settings.nWindowH = graphicsNode.attribute(L"nWindowH").as_uint();
	//resolution soft-check
	bool bSetDefaultRes = false;
	if ((m_Settings.nWindowW < K_WINDOW_WIDTH_MIN) || (m_Settings.nWindowH < K_WINDOW_HEIGHT_MIN))
		bSetDefaultRes = true;
	//resolution limits
	if ((m_Settings.nWindowW > 7680) || (m_Settings.nWindowH > 4320)) //not larger than 8K please
		bSetDefaultRes = true;
	if (bSetDefaultRes)
	{
		m_Settings.nWindowW = K_WINDOW_WIDTH_SAFE;
		m_Settings.nWindowH = K_WINDOW_HEIGHT_SAFE;

		ErrorBox(K_ERR_WARNING, L"LoadSettings: Wrong window sizes! Resetting to defaults.");
	}

	//misc
	m_Settings.bFullscreen = graphicsNode.attribute(L"bFullscreen").as_bool();
	m_Settings.bPixelPerfect = graphicsNode.attribute( L"bPixelPerfect" ).as_bool();
	m_Settings.bBorderlessFullscreen = graphicsNode.attribute(L"bBorderless").as_bool();
	m_Settings.bScreenShakes = graphicsNode.attribute(L"bScreenShakes").as_bool();
	m_Settings.bGoreEnabled = graphicsNode.attribute(L"bGoreEnabled").as_bool();
	m_Settings.bShowInterfaceHelp = graphicsNode.attribute(L"bShowInterfaceHelp").as_bool();
	//LODs
	m_Settings.nLOD_lights = graphicsNode.attribute(L"LOD_lights").as_int();
	m_Settings.nLOD_shadows = graphicsNode.attribute(L"LOD_shadows").as_int();
	m_Settings.nLOD_water = graphicsNode.attribute(L"LOD_water").as_int();

	//Developer mode enable
	if (graphicsNode.attribute(L"bDevMode").as_bool())
	{
		m_Settings.dev_bDevMode = true;
		m_Settings.dev_bDevMode_forced = true; //dev mode forced from options file

		m_Settings.dev_bLogWindowShow = true;
		m_Settings.dev_bLogWriteToFile = true;
		m_Settings.dev_bLogShowInDebugOutput = true;
	}

	return S_OK;
}

//-----------------------------------------------------------------------------
// EVENTS LISTENER for main app class
//-----------------------------------------------------------------------------
bool CApplication::HandleEvent(CEvent &nEvent)
{
	//system messages always get processed
	if (nEvent.m_eventType == CEventTypes::evtT_SYSTEM)
	{
		if (nEvent.m_eventCommand == CEventCommands::evtC_SYSTEM_RESOLUTION_CHANGE)
		{
			UINT32 nWidth = nEvent.GetArgumentByName(L"width")->m_asUINT32;
			UINT32 nHeight = nEvent.GetArgumentByName(L"height")->m_asUINT32;

			OnRenderSizeChanged(nWidth, nHeight);

			return false; //don't consume event
		}
		else if (nEvent.m_eventCommand == CEventCommands::evtC_SYSTEM_CONTROLLER_ADDED)
		{
			__Texts().SetString(STR_TEMP10, L"%s", nEvent.GetArgumentByName(L"strName")->m_strArg.text);
			__GUI().ShowLayer("LAYER_ID_CTRLR_CONNECTED");
		}
		else if (nEvent.m_eventCommand == CEventCommands::evtC_SYSTEM_CONTROLLER_REMOVED)
		{
			__Texts().SetString(STR_TEMP10, L"%s", nEvent.GetArgumentByName(L"strName")->m_strArg.text);
			__GUI().ShowLayer("LAYER_ID_CTRLR_DISCONNECTED");
		}
	}

	//handling EVTT_TRANSITION
	if (nEvent.m_eventType == CEventTypes::evtT_GAMESTATE)
	{
		if (nEvent.m_eventCommand == CEventCommands::evtC_GAMESTATE_CHANGE)
		{
			EGameState gameState = (EGameState)nEvent.GetArgumentByName(L"newGameState")->m_asUINT32;

			GameState::ChangeTo(gameState);
			return true; //consume event
		}
		if (nEvent.m_eventCommand == CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION)
		{
			EGameState gameState = (EGameState)nEvent.GetArgumentByName(L"newGameState")->m_asUINT32;
			ETransitionType transType = (ETransitionType)nEvent.GetArgumentByName(L"transitionType")->m_asINT32;

			GameState::ChangeTo_Transition(gameState, transType);
			return true; //consume event
		}
	}

	//don't process controls messages during transitions
	if (GameState::isTransitioning())
		return false;
	//CONTROLS messages handling
	if (nEvent.m_eventType == CEventTypes::evtT_CONTROLS)
	{
		if (nEvent.m_eventCommand == CEventCommands::evtC_CONTROLS_CLICK)
		{
			UINT32 layerID = nEvent.GetArgumentByName(L"layerID")->m_asUINT32;
			UINT32 ctrlID = nEvent.GetArgumentByName(L"ctrlID")->m_asUINT32;
			bool locked = nEvent.GetArgumentByName(L"locked")->m_asBool;

			//ID-uri generice butoane (remove layer, etc)
			if (ctrlID == HASH("BUT_CLOSE")) //close normal la orice fereastra
			{
				__GUI().RemoveTopmostLayer();
				return true;
			}
			else if (ctrlID == HASH("BUT_CLOSE_SETTINGS")) //close settings, save settings
			{
				__GUI().RemoveTopmostLayer();
				UTApp().SaveSettings();
				return true;
			}
			else if (ctrlID == HASH("BUT_CLOSE_KEYDEF")) //close key redefining
			{
				__GUI().RemoveTopmostLayer();
				//save user data (including keys)
				App_SaveUserData();

				CCtrlLayer* layer = __GUI().ShowLayerOnce("LAYER_ID_OPTIONS_MM");
				if (layer != null)
				{
					CControl* ctrl;
 					if (ctrl = layer->GetControlByName("CTRL_SLIDER_SOUNDVOL"))
					{
						ctrl->paramsDict.SetVarFloat(L"fSlidePercent", m_Settings.fSoundsVolume);
					}
					if (ctrl = layer->GetControlByName("CTRL_SLIDER_MUSICVOL"))
					{
						ctrl->paramsDict.SetVarFloat(L"fSlidePercent", m_Settings.fMusicVolume);
					}
				}

				return true;
			}
			else if (ctrlID == HASH("BUT_CLOSE_FORCED")) //face close la fereastra fara sa mai faca fade-out
			{
				__GUI().RemoveTopmostLayer(true);
				return true;
			}
			else if (ctrlID == HASH("BUT_EXIT_GAME"))
			{
				PostQuitMessage(0);
				//save states/options on exit?
				return true;
			}
			else if (ctrlID == HASH("BUT_SEND_FEEDBACK")) 
			{
				WEBSITE_OPEN(K_GAME_CONTACT_URL);
			}
			else if (ctrlID == HASH("BUT_GFX_OPTIONS"))
			{
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
				__Texts().SetString(STR_RESOLUTIONS_LIST, wsResStr);
				//setup controls
				CCtrlLayer* layer = __GUI().ShowLayerOnce("LAYER_ID_GFX_OPTIONS");
				if (layer != null)
				{
					CControl* ctrl = null;
					if (ctrl = layer->GetControlByName("CTRL_CHECK_FULLSCREEN"))
					{
						ctrl->paramsDict.SetVarBool(L"bChecked", m_Settings.bFullscreen);
					}
					if ( ctrl = layer->GetControlByName( "CTRL_CHECK_PIXELPERFECT" ) )
					{
						ctrl->paramsDict.SetVarBool( L"bChecked", m_Settings.bPixelPerfect);
					}
					if (ctrl = layer->GetControlByName("CTRL_CHECK_BORDERLESS"))
					{
						ctrl->paramsDict.SetVarBool(L"bChecked", m_Settings.bBorderlessFullscreen);
					}
					//set selected resolution
					int nSelIdx = g_arrResolutions.GetSize() - 1; //by default largest res possible
					for (int kk = g_arrResolutions.GetSize() - 1; kk >= 0; kk--)
					{
						if ((g_arrResolutions[kk].w <= m_Settings.nWindowW) && (g_arrResolutions[kk].h <= m_Settings.nWindowH))
						{
							nSelIdx = kk;
							break;
						}
					}
					if (ctrl = layer->GetControlByName("CTRL_DROP_RES"))
					{
						ctrl->paramsDict.SetVarINT32(L"nSelectedIdx", nSelIdx);
						//disable if fullscreen and borderless
						if (m_Settings.bFullscreen && m_Settings.bBorderlessFullscreen)
							ctrl->bDisabled = true;
					}

				}
			}
			else if (ctrlID == HASH("BUT_GFX_APPLY"))
			{
				//save old settings so we can see what's changed
				CApplicationSettings m_SettingsOld = m_Settings;

				CCtrlLayer* layer = __GUI().GetLayerByName("LAYER_ID_GFX_OPTIONS");
				if (layer != null)
				{
					CControl* ctrl;
					if (ctrl = layer->GetControlByName("CTRL_CHECK_FULLSCREEN"))
					{
						m_Settings.bFullscreen = ctrl->paramsDict[L"bChecked"].m_asBool;
					}
					if ( ctrl = layer->GetControlByName( "CTRL_CHECK_PIXELPERFECT" ) )
					{
						m_Settings.bPixelPerfect = ctrl->paramsDict[ L"bChecked" ].m_asBool;
					}
					if (ctrl = layer->GetControlByName("CTRL_CHECK_BORDERLESS"))
					{
						m_Settings.bBorderlessFullscreen = ctrl->paramsDict[L"bChecked"].m_asBool;
					}
					//set selected resolution
					if (ctrl = layer->GetControlByName("CTRL_DROP_RES"))
					{
						int nSelectedIdx = ctrl->paramsDict[L"nSelectedIdx"].m_asINT32;

						if ((nSelectedIdx >= 0) && (nSelectedIdx < g_arrResolutions.GetSize()))
						{
							SizeWHi szRes = g_arrResolutions[nSelectedIdx];

							m_Settings.nWindowW = szRes.w;
							m_Settings.nWindowH = szRes.h;
							LOG(L"Set res: %dx%d", m_Settings.nWindowW, m_Settings.nWindowH);
						}
					}

					//--- apply settings ---
					bool bIsWindowed = DXUTIsWindowed() && (App_IsBorderlessFullscreen() == false);
					//special case for when on fullscreen but changing only the bBorderless setting (just exit from fullscreen)
					if ((!bIsWindowed) && (!bIsWindowed == m_Settings.bFullscreen))
					{
						if (m_SettingsOld.bBorderlessFullscreen != m_Settings.bBorderlessFullscreen)
						{
							if (m_SettingsOld.bBorderlessFullscreen)
							{
								App_ToggleBorderlessFullscreen(DXUTGetHWNDDeviceWindowed());
							}
							else
							{
								DXUTToggleFullScreen();
							}
						}
					}
					
					bIsWindowed = DXUTIsWindowed() && (App_IsBorderlessFullscreen() == false);
					//going from windowd to FS or the other way around
					if (!bIsWindowed != m_Settings.bFullscreen)
					{
						//windowed FROM fullscreen
						if ((bIsWindowed == false) && (m_Settings.bFullscreen == false))
						{
							if (m_SettingsOld.bBorderlessFullscreen)
							{
								App_ToggleBorderlessFullscreen(DXUTGetHWNDDeviceWindowed());
							}
							else
							{
								DXUTToggleFullScreen();
							}
						}
						//going TO fullscreen
						else if ((bIsWindowed == true) && (m_Settings.bFullscreen == true))
						{
							if (m_Settings.bBorderlessFullscreen)
							{
								App_ToggleBorderlessFullscreen(DXUTGetHWNDDeviceWindowed());
							}
							else
							{
								DXUTToggleFullScreen();
							}
						}

						//sometimes window is outside screen area:
						HWND hwndWindowed = DXUTGetHWNDDeviceWindowed();
						if(DXUTIsWindowed() && (App_IsBorderlessFullscreen() == false))
						{
							App_CenterWindowOnMainDisplay(hwndWindowed);
						}
					}
					else //stays on same mode
					{
						DXUTDeviceSettings devsettings = DXUTGetDeviceSettings();
						DXUTDeviceSettings devsettings_old = devsettings;

						devsettings.pp.Windowed = !m_Settings.bFullscreen;
						//set resolution
						devsettings.pp.BackBufferWidth = m_Settings.nWindowW;
						devsettings.pp.BackBufferHeight = m_Settings.nWindowH;

						if (devsettings.pp.MultiSampleType != D3DMULTISAMPLE_NONE)
						{
							devsettings.pp.Flags &= ~D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
						}
						if (devsettings.pp.Windowed)
						{
							RECT rcWindowClient = DXUTGetWindowClientRect();
							if (((rcWindowClient.right - rcWindowClient.left) != m_Settings.nWindowW) ||
								((rcWindowClient.bottom - rcWindowClient.top) != m_Settings.nWindowH))
							{
								HWND hwndWindowed = DXUTGetHWNDDeviceWindowed();
								//gasesc monitorul pe care e fereastra si rezolutia de lucru a acestuia
								HMONITOR monitor = MonitorFromWindow(hwndWindowed, MONITOR_DEFAULTTOPRIMARY);
								MONITORINFO mi;
								mi.cbSize = sizeof(mi);
								GetMonitorInfo(monitor, &mi);
								RECT rectMonitor = mi.rcWork; //working area

								RECT wndrect;
								SetRect(&wndrect, 0, 0, m_Settings.nWindowW, m_Settings.nWindowH);
								DWORD style = GetWindowStyle(hwndWindowed);
								AdjustWindowRect(&wndrect, style, FALSE);

								App_CenterRectInRect(&wndrect, &rectMonitor);
								//center rect
								//int offx = (g_szDesktopSize.w - (wndrect.right - wndrect.left)) / 2 - wndrect.left;
								//int offy = (g_szDesktopSize.h - (wndrect.bottom - wndrect.top)) / 2 - wndrect.top;
								//wndrect.left += offx; wndrect.right += offx;
								//wndrect.top += offy; wndrect.bottom += offy;
								//if (wndrect.top < -10)
								//{
								//	wndrect.bottom += -(wndrect.top + 10);
								//	wndrect.top += -(wndrect.top + 10);
								//}

								SetWindowPos(hwndWindowed, 0, wndrect.left, wndrect.top, wndrect.right - wndrect.left, wndrect.bottom - wndrect.top, SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_SHOWWINDOW);
							}
						}
						else
						{
							RECT rcWindowClient = DXUTGetWindowClientRect();
							if ((devsettings_old.pp.BackBufferWidth != m_Settings.nWindowW) ||
								(devsettings_old.pp.BackBufferHeight != m_Settings.nWindowH))
							{
								LOG(L"[GFX]Changed Fullscreen resolution: [%dx%d]", devsettings.pp.BackBufferWidth, devsettings.pp.BackBufferHeight);
								DXUTCreateDeviceFromSettings(&devsettings);
							}
						}
					}
					//close layer
					__GUI().RemoveLayer("LAYER_ID_GFX_OPTIONS");
					//save settings
					SaveSettings();
				}
			}
			else if (ctrlID == HASH("BUT_QUIT_PLAYERSEL")) //fereastra de exit from player selection
			{
				if (UTApp().IsGameNetworked())
				{
					g_netlock.Net_QuitLobby();

					CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
					nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
					nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
					__Events().QueueEvent(nevent);
				}
				else
				{
					CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
					nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_LEVEL_SELECTION);
					nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
					__Events().QueueEvent(nevent);
				}

				return true;
			}
			else if (ctrlID == HASH("BUT_LEVEL_BACK")) //level finished - level failed but back
			{
				if (!UTApp().IsGameNetworked())
				{
					CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
					nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_LEVEL_SELECTION);
					nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
					__Events().QueueEvent(nevent);

					CHAR ctxt[MAX_PATH];
					StringCchPrintfA(ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL]);
					ANALYTICS_EVENT("level_fail_quit", ctxt, "", 0);
				}
				else //on networked games only send state to peer
				{
					CNetLock::sPacketLevelResults sPack(CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CANCEL);
					g_netlock.Net_SendLevelResultsCommand(sPack);

					CHAR ctxt[MAX_PATH];
					StringCchPrintfA(ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL]);
					ANALYTICS_EVENT("level_fail_quit_net", ctxt, "", 0);
				}

				return true;
			}
			else if (ctrlID == HASH("BUT_LEVEL_BACK_IGM")) //ingame menu - quit level
			{
				if (!UTApp().IsGameNetworked())
				{
					CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
					nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_LEVEL_SELECTION);
					nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
					__Events().QueueEvent(nevent);

					CHAR ctxt[MAX_PATH];
					StringCchPrintfA(ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL]);
					ANALYTICS_EVENT("igm_level_quit", ctxt, "", 0);
				}
				else //on networked games go to main menu
				{
					CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
					nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
					nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
					__Events().QueueEvent(nevent);

					CHAR ctxt[MAX_PATH];
					StringCchPrintfA(ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL]);
					ANALYTICS_EVENT("igm_level_quit_net", ctxt, "", 0);
				}

				return true;
			}
			else if (ctrlID == HASH("BUT_QUICK_MATCH"))
			{
				//mark game type
				UTApp().m_Settings.devnet_eNetGameType = CApplicationSettings::K_NETGAME_TYPE_QUICK_MATCH;
				//change state
				CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
				nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_GAME_MODE_SELECTION);
				nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
				__Events().QueueEvent(nevent);
			}
			else if (ctrlID == HASH("BUT_HOST_PUBLIC"))
			{
				//mark game type
				UTApp().m_Settings.devnet_eNetGameType = CApplicationSettings::K_NETGAME_TYPE_HOST_PUBLIC;
				//change state
				CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
				nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_GAME_MODE_SELECTION);
				nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
				__Events().QueueEvent(nevent);
			}
			else if (ctrlID == HASH("BUT_HOST_PRIVATE"))
			{
				//mark game type
				UTApp().m_Settings.devnet_eNetGameType = CApplicationSettings::K_NETGAME_TYPE_HOST_PRIVATE;
				//change state
				CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
				nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_GAME_MODE_SELECTION);
				nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
				__Events().QueueEvent(nevent);
			}
			else if (ctrlID == HASH("BUT_CLOSE_LOBBIES_LIST"))
			{
				CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
				nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
				nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
				__Events().QueueEvent(nevent);
			}
			else if (ctrlID == HASH("BUT_CANCEL_LOBBY"))
			{
				g_netlock.Net_QuitLobby();

				CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
				nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_MAINMENU);
				nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
				__Events().QueueEvent(nevent);
			}
			else if (ctrlID == HASH("BUT_JOIN_LOBBY"))
			{
				int nLobbyIdx = -1;
				CCtrlLayer* lay = __GUI().GetTopmostInputLayer();
				if (lay != null)
				{
					CControl *ctrl = lay->GetControlByName("CTRL_LOBBIES_SELECTOR");
					if (ctrl != null)
						nLobbyIdx = ctrl->paramsDict[L"nSelectedIdx"].m_asINT32;
				}

				uint64_t iLobbyID = 0;
				char strLobbyName[250];
				if (g_pNetwork->GetLobbyListEntry(nLobbyIdx, iLobbyID, strLobbyName))
				{
					//save lobby ID for connection
					g_netlock.m_ullCurLobbyID = iLobbyID;

					LOG(L"Game::COOP Joining Lobby LobbyID: %llu", g_netlock.m_ullCurLobbyID);
					//change state
					CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
					nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_NET_LOBBY);
					nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
					//set joining state
					nevent->AddNamedArgINT32(L"arg1", (int)CApplicationSettings::K_NETGAME_TYPE_QUICK_MATCH);
					__Events().QueueEvent(nevent);
				}
			}
			else if (ctrlID == HASH("BUT_REFRESH_LOBBIES"))
			{
				//as soon as we enter we ask for the lobbies list and the state will read the lobbies a little later (on a timer job)
				g_netlock.Net_RequestLobbyList(10);
				__Texts().SetString(STR_LOBBIES_LIST_VAL, L"%s", __Texts().strings[STR_PLEASE_HANG]->sText);

				CCtrlLayer* lay = __GUI().GetTopmostInputLayer();
				//disable the refresh button if still working
				if (lay != null)
				{
					CControl* ctrl = lay->GetControlByName("BUT_REFRESH_LOBBIES");
					if (ctrl)
						ctrl->bDisabled = true;
					ctrl = lay->GetControlByName("BUT_JOIN_LOBBY");
					if (ctrl)
						ctrl->bDisabled = true;
					ctrl = lay->GetControlByName("CTRL_LOBBIES_SELECTOR");
					if (ctrl)
					{
						ctrl->bDisabled = true;
						ctrl->paramsDict.SetVarINT32(L"nOptionsCnt", 1);
					}
				}
			}
			else if (ctrlID == HASH("BUT_JOIN_GAME"))
			{
				//mark game type as online coop
				UTApp().m_Settings.devnet_eNetGameType = CApplicationSettings::K_NETGAME_TYPE_QUICK_MATCH;
				//change state
				CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
				nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_GAME_MODE_SELECTION);
				nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
				//transmit that we want a special case as the next state (game state mode selection takes arg1 and transmits it to g_mainMenu)
				nevent->AddNamedArgINT32(L"arg1", GAME_STATE_JOIN_COOP_LIST);
				__Events().QueueEvent(nevent);
			}
			else if (ctrlID == HASH("BUT_INVITE_TO_LOBBY"))
			{
				g_pNetwork->ShowInviteToLobbyUI();
			}
			else if (ctrlID == HASH("BUT_LEVEL_WIN")) //fereastra de level finished - level win (continue)
			{
				if (!UTApp().IsGameNetworked()) //not networked
				{
					//avansez pe urmatorul nivel
					g_userData[K_MEMID_SELECTED_LEVEL]++;
					CLAMP(g_userData[K_MEMID_SELECTED_LEVEL], 0, K_GAME_LEVELS_PER_CHAPTER - 1);
					//comand schimbarea starii
					CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
					nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_LEVEL_SELECTION);
					nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
					__Events().QueueEvent(nevent);
				}
				else //on networked games only send state to peer
				{
					CNetLock::sPacketLevelResults sPack(CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CONTINUE);
					g_netlock.Net_SendLevelResultsCommand(sPack);
				}

				return true;
			}
			else if (ctrlID == HASH("BUT_COOPFAIL_CONTINUE")) //fereastra de level failed coop vote next level
			{
				if (UTApp().IsGameNetworked()) //only networked
				{
					CNetLock::sPacketLevelResults sPack(CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CONTINUE);
					g_netlock.Net_SendLevelResultsCommand(sPack);
				}
			}
			else if (ctrlID == HASH("BUT_LEVEL_RESTART")) //fereastra de level failed - main menu
			{
				if (!UTApp().IsGameNetworked()) //not networked
				{
					CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
					nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_PLAYER_SELECTION);
					nevent->AddNamedArgINT32(L"arg1", 0); //reset player selection
					nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
					__Events().QueueEvent(nevent);

					CHAR ctxt[MAX_PATH];
					StringCchPrintfA(ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL]);
					ANALYTICS_EVENT("level_restart", ctxt, "", 0);
				}
				else //on networked games only send state to peer
				{
					CNetLock::sPacketLevelResults sPack(CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_RESTART);
					g_netlock.Net_SendLevelResultsCommand(sPack);
				}

				return true;
			}
			else if (ctrlID == HASH("BUT_UNLOCK_WEAPON"))
			{
				UINT32 dwWeaponHash = nEvent.GetArgumentByName(L"nMsgParamUINT32")->m_asUINT32;
				//buy weapon and mark spent money
				int nPrice = UTGetShop().GetItemPrice(dwWeaponHash);
				if (SUCCEEDED(UTGetShop().UnlockItem(dwWeaponHash, true)))
				{
					if (nPrice > 0)
					{
						g_userData[K_MEMID_STARS_SPENT] += nPrice;
					}
				}

				App_SaveUserData();
				//update selection locked flags
				for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
				{
					if (g_playerSelScr.m_arrPlayers[kk].bIsNetworkPlayer)
						continue;
					if (g_playerSelScr.m_arrPlayers[kk].eType != K_PSS_CLASS_NOT_SELECTED)
					{
						g_playerSelScr.SetSelectionPrices(&g_playerSelScr.m_arrPlayers[kk]);
						//close selection:
						//SND_PLAY(SNDIDX_RELOAD_TACTICAL);
						g_playerSelScr.m_arrPlayers[kk].nCursorMoreReal = -1;

						if (UTApp().IsGameNetworked())
							g_playerSelScr.SendSelectionByNetwork(kk);
					}
				}

				__GUI().RemoveTopmostLayer();

				return true;
			}
			else if (ctrlID == HASH("BUT_RESET_KEYS_SURE"))
			{
				int nKeybdIdx = nEvent.GetArgumentByName(L"nMsgParamINT32")->m_asINT32;
				//trimitem mai departe indexul tastaturii selectate
				CCtrlLayer* layer = __GUI().ShowLayerOnce("LAYER_ID_RESET_KEYS");
				CControl* ctrl = null;
				if (ctrl = layer->GetControlByName("BUT_RESET_KEYS"))
				{
					ctrl->paramsDict.SetVarINT32(L"nMsgParamINT32", nKeybdIdx);
				}
			}
			else if (ctrlID == HASH("BUT_REDEFINE_KEYS"))
			{
				__GUI().ShowLayerOnce("LAYER_ID_REDEFINE_KEYS");
			}
			else if (ctrlID == HASH("BUT_CREDITS"))
			{
				__GUI().ShowLayerOnce("LAYER_ID_CREDITS");
			}
			else if (ctrlID == HASH("BUT_KEYS_LAYOUT"))
			{
				__GUI().ShowLayerOnce("LAYER_ID_KEYMAP");
			}
			else if (ctrlID == HASH("BUT_MORE_OPTIONS"))
			{
				CCtrlLayer* layer = __GUI().ShowLayerOnce("LAYER_ID_MORE_OPTIONS");
				if ( layer != null )
				{
					layer->SetControlParam( "CTRL_CHECK_SHAKES", L"bChecked", m_Settings.bScreenShakes );
					layer->SetControlParam( "CTRL_CHECK_GORE", L"bChecked", m_Settings.bGoreEnabled);
				}
			}
			else if (ctrlID == HASH("BUT_CREDITS_MORE"))
			{
				//remove credits layer
				__GUI().RemoveLayer("LAYER_ID_CREDITS");
				//add additional credits
				__GUI().ShowLayerOnce("LAYER_ID_CREDITS_MORE");
			}
			else if (ctrlID == HASH("BUT_RESET_PROGRESS"))
			{
				__GUI().RemoveLayer("LAYER_ID_MORE_OPTIONS");
				__GUI().ShowLayerOnce("LAYER_ID_RESET_PROGRESS");
			}
			//reset XP upgrades
			else if (ctrlID == HASH("BUT_RESET_UPGRADES"))
			{
				CCtrlLayer* lay = __GUI().GetLayerByName("LAYER_ID_PLAYER_UPGRADE");
				if (lay != null)
				{
					CControl* ctrl = lay->GetControlByName("CTRLID_UPGRADE_PLAYER");
					if (ctrl != null)
					{
						int nPlayerOrdinal = ctrl->paramsDict[L"nPlayerOrdinal"].m_asINT32;
						int nPlayerClass = (int)g_playerSelScr.m_arrPlayers[nPlayerOrdinal].eType;
						for (int ll = 0; ll < K_PSS_UPGRADE_BARS_CNT; ll++)
						{
							int nFilledReal = g_playerSelScr.m_arrPlayers[nPlayerOrdinal].arrUpgradeBarsPts[ll];

							WCHAR strParamName[MAX_PATH];
							StringCchPrintf(strParamName, MAX_PATH, L"spent_bar%d", ll);
							int nValue = -nFilledReal;
							//reset team bars to initial values
							if (ll <= 1)
								nValue = 0;
							ctrl->paramsDict.SetVarINT32(strParamName, nValue);
						}

					}
				}
			}
			//apply XP upgrades
			else if (ctrlID == HASH("BUT_ACCEPT_UPGRADE"))
			{
				CCtrlLayer* lay = __GUI().GetLayerByName("LAYER_ID_PLAYER_UPGRADE");
				if (lay != null)
				{
					CControl* ctrl = lay->GetControlByName("CTRLID_UPGRADE_PLAYER");
					if (ctrl != null)
					{
						int nPlayerOrdinal = ctrl->paramsDict[L"nPlayerOrdinal"].m_asINT32;
						int nPlayerClass = (int)g_playerSelScr.m_arrPlayers[nPlayerOrdinal].eType;
						//sum all spending here
						int nTotalSpentPoints = 0;
						for (int ll = 0; ll < K_PSS_UPGRADE_BARS_CNT; ll++)
						{
							WCHAR strParamName[MAX_PATH];
							StringCchPrintf(strParamName, MAX_PATH, L"spent_bar%d", ll);
							int nSpentPoints = ctrl->paramsDict[strParamName].m_asINT32;
							//save points back to memory
							int nBarIdx = g_playerSelScr.arrItemsByClass[nPlayerClass].arrUpgradeBarsIdx[ll];
							int nDataIdx = K_MEMID_UPGRADE_BAR_POINTS_START + g_playerSelScr.m_arrUpgradeBars[nBarIdx]->nMemSlot;
							g_userData[nDataIdx] += nSpentPoints;
							//make sure we don't get below 0 due to some bug
							if (g_userData[nDataIdx] < 0)
							{
								nSpentPoints -= g_userData[nDataIdx];
								g_userData[nDataIdx] = 0;
							}
							//update total spent points
							nTotalSpentPoints += nSpentPoints;
							//reset control spent points so it doesn't show more (while closing)
							ctrl->paramsDict.SetVarINT32(strParamName, 0);
						}
						//update spent points
						g_userData[K_MEMID_POINTS_SPENT_PER_CLASS_START + nPlayerClass] += nTotalSpentPoints;
						//apply data to bars (all local players)
						g_playerSelScr.InitUpgradeBars(&g_playerSelScr.m_arrPlayers[nPlayerOrdinal]);
						//other player too (might be the same class)
						int nOtherPlayerIdx = (nPlayerOrdinal + 1) % 2;
						if((g_playerSelScr.m_arrPlayers[nOtherPlayerIdx].eType != K_PSS_CLASS_NOT_SELECTED) && (false == g_playerSelScr.m_arrPlayers[nOtherPlayerIdx].bIsNetworkPlayer))
							g_playerSelScr.InitUpgradeBars(&g_playerSelScr.m_arrPlayers[nOtherPlayerIdx]);
						//save data
						App_SaveUserData();
						//hide window
						__GUI().RemoveLayer(lay->ID.textHash);
					}
				}
			}
			else if (ctrlID == HASH("BUT_RESET_PROGRESS_SURE"))
			{
				LOG(L"Progress Reset asked!");
				
				App_ResetUserData();
				App_SaveUserData();

				__GUI().RemoveLayer("LAYER_ID_RESET_PROGRESS");
			}
			else if (ctrlID == HASH("BUT_RESET_PROGRESS_SURE_EA")) //early access version
			{
				LOG(L"Progress Reset EA asked!");

				App_ResetUserData();
				App_SaveUserData();

				__GUI().RemoveLayer("LAYER_ID_RESET_PROGRESS_EA");
			}
			else if (ctrlID == HASH("BUT_LANGUAGE"))
			{
				//build strings list before adding the control so it can get the lines count
				WCHAR txt[1024] = { 0 };
				for (int kk = 0; kk < g_arrLangList.GetSize(); kk++)
				{
					StringCchCat(txt, 1024, g_arrLangList[kk].shLangName.text);
					if (kk < g_arrLangList.GetSize() - 1)
						StringCchCat(txt, 1024, L"\n");
				}
				//save lang list
				__Texts().SetString(STR_TEMP15, txt);

				__GUI().RemoveTopmostLayer();
				CCtrlLayer *pLay = __GUI().ShowLayerOnce("LAYER_ID_LANGUAGE");
				//set selection on current language
				if (pLay != null)
				{
					CControl *ctrl = pLay->GetControlByName("CTRL_LANGLIST_TT");
					if (ctrl != null)
					{
						for (int kk = 0; kk < g_arrLangList.Count(); kk++)
						{
							if (g_arrLangList[kk].shLangAlias == g_Language.shLangAlias)
							{
								ctrl->paramsDict.SetVarINT32(L"nSelectedIdx", kk);
								break;
							}
						}
					}
				}
			}
			else if (ctrlID == HASH("BUT_SELECT_LANGUAGE"))
			{
				int nLangIdx = -1;
				CCtrlLayer *pLay = __GUI().GetLayerByName("LAYER_ID_LANGUAGE");
				if (pLay != null)
				{
					CControl *ctrl = pLay->GetControlByName("CTRL_LANGLIST_TT");
					if (ctrl != null)
					{
						nLangIdx = ctrl->paramsDict[L"nSelectedIdx"].m_asINT32;
					}
				}
				//remove layer
				__GUI().RemoveTopmostLayer();
				//change language
				if (nLangIdx >= 0)
				{
					CLAMP(nLangIdx, 0, g_arrLangList.GetSize() - 1);

					App_LocaChangeLanguage(g_arrLangList[nLangIdx].shLangAlias);
					App_UpdateLevelStats();

					UTApp().SaveSettings();
					//set version number
					__Texts().SetString(STR_VERSION_NUMBER, L"v%d.%d.%d", _VERSION_MAJOR_, _VERSION_MINOR_, _VERSION_PATCH_);
				}
			}
			else if (ctrlID == HASH("BUT_CTRLR_LAYOUT"))
			{
				__GUI().ShowLayerOnce("LAYER_ID_CONTROLLER_MAP");
			}
#ifdef ENABLE_LEADERBOARDS
			//leaderboards from main menu, global ones
			else if (ctrlID == HASH("BUT_BOARDS_SINGLE"))
			{
				__Texts().SetString(STR_LEADERBOARDS_NAMES_VAL, L"...");
				__Texts().SetString(STR_LEADERBOARDS_SCORES_VAL, L"...");
				//request single player scores
				__Leaderboards().QueueJob(K_JOB_GET_SCORES_GLOBAL, K_GAME_STR_LEADERBOARDS_GLOBAL_SP, 1);
				//write score for YOUR SCORE label
				__Texts().SetString(STR_TEMP15, L"%d", g_userData[K_MEMID_TOTAL_SCORE_SOLO]);

				//reset scroll page and save leaderboard index as a payload in this control
				CCtrlLayer *pLay = __GUI().GetLayerByName("LAYER_ID_LEADERBOARDS_MM");
				if (pLay != null)
				{
					// set scores list on empty
					CControl *ctrl = pLay->GetControlByName("CTRL_SCORESLIST_TT");
					if (ctrl != null)
					{
						ctrl->paramsDict.SetVarINT32(L"nOptionsCnt", 0);
						ctrl->paramsDict.SetVarINT32(L"nPage", 0);
						ctrl->paramsDict.SetVarINT32(L"nLeaderboardID", 0); //Single Player
						ctrl->paramsDict.SetVarINT32(L"nSelectedIdx", -1);
					}
				}
			}
			else if (ctrlID == HASH("BUT_BOARDS_COOP"))
			{
				__Texts().SetString(STR_LEADERBOARDS_NAMES_VAL, L"...");
				__Texts().SetString(STR_LEADERBOARDS_SCORES_VAL, L"...");
				//request single player scores
				__Leaderboards().QueueJob(K_JOB_GET_SCORES_GLOBAL, K_GAME_STR_LEADERBOARDS_GLOBAL_COOP, 1);
				//write score for YOUR SCORE label
				__Texts().SetString(STR_TEMP15, L"%d", g_userData[K_MEMID_TOTAL_SCORE_COOP]);
				//reset scroll page and save leaderboard index as a payload in this control
				CCtrlLayer *pLay = __GUI().GetLayerByName("LAYER_ID_LEADERBOARDS_MM");
				if (pLay != null)
				{
					// set scores list on empty
					CControl *ctrl = pLay->GetControlByName("CTRL_SCORESLIST_TT");
					if (ctrl != null)
					{
						ctrl->paramsDict.SetVarINT32(L"nOptionsCnt", 0);
						ctrl->paramsDict.SetVarINT32(L"nPage", 0);
						ctrl->paramsDict.SetVarINT32(L"nLeaderboardID", 1); //Multiplayer
						ctrl->paramsDict.SetVarINT32(L"nSelectedIdx", -1);
					}
				}
			}
			//leaderboards in level selection screen
			else if (ctrlID == HASH("BUT_BOARDS_SINGLE_LVL"))
			{
				//reset scroll page and save leaderboard index as a payload in this control
				CCtrlLayer *pLay = __GUI().GetTopmostLayer();
				if (pLay != null)
				{
					// set scores list on empty
					CControl *ctrl = pLay->GetControlByName("CTRL_SCORESLIST_TT");
					if (ctrl != null)
					{
						ctrl->paramsDict.SetVarINT32(L"nOptionsCnt", 0);
						ctrl->paramsDict.SetVarINT32(L"nPage", 0);
						ctrl->paramsDict.SetVarINT32(L"nLeaderboardID", 1); //Multiplayer
						ctrl->paramsDict.SetVarINT32(L"nSelectedIdx", -1);
					}
				}

				g_mainMenu.RequestLeaderboardsUpdate(false);
			}
			else if (ctrlID == HASH("BUT_BOARDS_COOP_LVL"))
			{
				//reset scroll page and save leaderboard index as a payload in this control
				CCtrlLayer *pLay = __GUI().GetTopmostLayer();
				if (pLay != null)
				{
					// set scores list on empty
					CControl *ctrl = pLay->GetControlByName("CTRL_SCORESLIST_TT");
					if (ctrl != null)
					{
						ctrl->paramsDict.SetVarINT32(L"nOptionsCnt", 0);
						ctrl->paramsDict.SetVarINT32(L"nPage", 0);
						ctrl->paramsDict.SetVarINT32(L"nLeaderboardID", 1); //Multiplayer
						ctrl->paramsDict.SetVarINT32(L"nSelectedIdx", -1);
					}
				}

				g_mainMenu.RequestLeaderboardsUpdate(true);
			}

#endif

			else if (ctrlID == HASH("BUT_RESET_KEYS")) //resets player keys
			{
				int nKeybdIdx = nEvent.GetArgumentByName(L"nMsgParamINT32")->m_asINT32;
				if (nKeybdIdx == 0) //first keyboard
				{
					App_ResetKeybindings(0);
					//change actual triggers
					CController* keybd1 = __Controllers().GetControllerByInstanceID(K_CM_IID_KBM1);
					CController* keybd2 = nullptr;// UTGetCtrlrMgr().GetControllerByInstanceID(K_CM_DEFAULT_KEYBOARD2_INSTANCE_ID);
					App_SetSDLTriggersFromUserData(keybd1, keybd2);

					__GUI().RemoveTopmostLayer();
					return true;
				}
				else //second keyboard
				{
					App_ResetKeybindings(1);
					//change actual triggers
					CController* keybd1 = __Controllers().GetControllerByInstanceID(K_CM_IID_KBM1);
					CController* keybd2 = nullptr;// UTGetCtrlrMgr().GetControllerByInstanceID(K_CM_DEFAULT_KEYBOARD2_INSTANCE_ID);
					App_SetSDLTriggersFromUserData(keybd1, keybd2);

					__GUI().RemoveTopmostLayer();
					return true;
				}
			}
			else if (ctrlID == HASH("BUT_REDEFINE_KEY1"))
			{
				__GUI().RemoveLayer("LAYER_ID_REDEFINE_KEYS");
				__GUI().RemoveLayer("LAYER_ID_OPTIONS_MM");

				CCtrlLayer* layer = __GUI().ShowLayerOnce("LAYER_ID_KEYDEFINE");
				if (layer != null)
				{
					CControl* ctrl = null;
					if (ctrl = layer->GetControlByName("CTRL_KEYS_SELECTOR"))
					{
						ctrl->paramsDict.SetVarINT32(L"nBaseIndex", 0);
						ctrl->paramsDict.SetVarString(L"StringID_right", STRID_KEYS1_VAL);
						ctrl->paramsDict.SetVarString(L"nSelectedIdx", 0);
					}
					if (ctrl = layer->GetControlByName("WINDOW_REDEFINE"))
					{
						ctrl->paramsDict.SetVarINT32(L"stringID", STR_KEYBOARD1);
					}
					if (ctrl = layer->GetControlByName("BUT_RESET_KEYS_SURE"))
					{
						ctrl->paramsDict.SetVarINT32(L"nMsgParamINT32", 0); //keyboard index 
					}
				}
			}
			else if (ctrlID == HASH("BUT_REDEFINE_KEY2"))
			{
				__GUI().RemoveLayer("LAYER_ID_REDEFINE_KEYS");
				__GUI().RemoveLayer("LAYER_ID_OPTIONS_MM");

				CCtrlLayer* layer = __GUI().ShowLayerOnce("LAYER_ID_KEYDEFINE");
				if (layer != null)
				{
					CControl* ctrl = null;
					if (ctrl = layer->GetControlByName("CTRL_KEYS_SELECTOR"))
					{
						//base indexul este folosit ca sa detectez in mesajul de selectie daca sunt primele N comenzi sau urmatoarele N (keyboard 2)
						int nBaseIndex = K_MEMID_KEYS2_FIRSTITEM - K_MEMID_KEYS1_FIRSTITEM;
						ctrl->paramsDict.SetVarINT32(L"nBaseIndex", nBaseIndex);
						ctrl->paramsDict.SetVarString(L"StringID_right", STRID_KEYS2_VAL);
						ctrl->paramsDict.SetVarString(L"nSelectedIdx", 0);
					}
					if (ctrl = layer->GetControlByName("WINDOW_REDEFINE"))
					{
						ctrl->paramsDict.SetVarINT32(L"stringID", STR_KEYBOARD2);
					}
					if (ctrl = layer->GetControlByName("BUT_RESET_KEYS_SURE"))
					{
						ctrl->paramsDict.SetVarINT32(L"nMsgParamINT32", 1); //keyboard index
					}
				}
			}
		}
		else if (nEvent.m_eventCommand == CEventCommands::evtC_CONTROLS_SLIDER_CHANGED)
		{
			UINT32 layerID = nEvent.GetArgumentByName(L"layerID")->m_asUINT32;
			UINT32 ctrlID = nEvent.GetArgumentByName(L"ctrlID")->m_asUINT32;
			float slidePercent = nEvent.GetArgumentByName(L"fSlidePercent")->m_asFloat;

			if (ctrlID == HASH("CTRL_SLIDER_SOUNDVOL"))
			{
				m_Settings.fSoundsVolume = slidePercent;
				SND_SET_GROUP_VOLUME("sounds", slidePercent, false);
				SND_SET_GROUP_VOLUME("ingame", slidePercent, false);
				return true;
			}
			else if (ctrlID == HASH("CTRL_SLIDER_MUSICVOL"))
			{
				m_Settings.fMusicVolume = slidePercent;
				SND_SET_GROUP_VOLUME("music", slidePercent, false);
				return true;
			}
		}
		else if (nEvent.m_eventCommand == CEventCommands::evtC_CONTROLS_CHECK_CHANGED)
		{
			UINT32 layerID = nEvent.GetArgumentByName(L"layerID")->m_asUINT32;
			UINT32 ctrlID = nEvent.GetArgumentByName(L"ctrlID")->m_asUINT32;
			bool bCheck = nEvent.GetArgumentByName(L"bChecked")->m_asBool;

			if ((ctrlID == HASH("CTRL_CHECK_BORDERLESS")) ||
					 (ctrlID == HASH("CTRL_CHECK_FULLSCREEN"))	)
			{
				CCtrlLayer* layer = __GUI().GetTopmostInputLayer();
				if (layer != nullptr)
				{
					bool bFS = false, bBorderless = false;
					CControl* ctrl;
					if (ctrl = layer->GetControlByName("CTRL_CHECK_BORDERLESS"))
						bBorderless = ctrl->paramsDict[L"bChecked"].m_asBool;
					if (ctrl = layer->GetControlByName("CTRL_CHECK_FULLSCREEN"))
						bFS = ctrl->paramsDict[L"bChecked"].m_asBool;

					if (ctrl = layer->GetControlByName("CTRL_DROP_RES"))
					{
						if (bBorderless && bFS)
							ctrl->bDisabled = true;
						else
							ctrl->bDisabled = false;
					}
				}
				return true;
			}
			else if (ctrlID == HASH( "CTRL_CHECK_SHAKES" )) {
				CCtrlLayer* layer = __GUI().GetTopmostInputLayer();
				if ( layer != nullptr )
				{
					CControl* ctrl;
					if ( ctrl = layer->GetControlByName( "CTRL_CHECK_SHAKES" ) )
						m_Settings.bScreenShakes = ctrl->paramsDict[ L"bChecked" ].m_asBool;
				}
			}
			else if (ctrlID == HASH( "CTRL_CHECK_GORE" )) {
				CCtrlLayer* layer = __GUI().GetTopmostInputLayer();
				if ( layer != nullptr )
				{
					CControl* ctrl;
					if ( ctrl = layer->GetControlByName( "CTRL_CHECK_GORE" ) )
						m_Settings.bGoreEnabled = ctrl->paramsDict[ L"bChecked" ].m_asBool;
				}
			}
		}
		else if (nEvent.m_eventCommand == CEventCommands::evtC_CONTROLS_PAGE_CHANGED)
		{
			UINT32 layerID = nEvent.GetArgumentByName(L"layerID")->m_asUINT32;
			UINT32 ctrlID = nEvent.GetArgumentByName(L"ctrlID")->m_asUINT32;
			int nPageIdx = nEvent.GetArgumentByName(L"nPageIdx")->m_asINT32;
			int nOldIdx = nEvent.GetArgumentByName(L"nOldIdx")->m_asINT32;

			// highscores paging control
			if (ctrlID == HASH("CTRL_SCORESLIST_TT"))
			{
				int nLeaderboardID = 0;
				CCtrlLayer* pLay = __GUI().GetLayerByNameHash(layerID);
				//#HACK: get leaderboards type from the controls's payload
				if (pLay != null)
				{
					CControl *ctrl = pLay->GetControlByName("CTRL_SCORESLIST_TT");
					if (ctrl != null)
					{
						nLeaderboardID = ctrl->paramsDict[L"nLeaderboardID"].m_asINT32;
						bool bUserCanSelect = ctrl->paramsDict[L"bUserCanSelect"].m_asBool;

						ctrl->bDisabled = true;
						
						ctrl->paramsDict.SetVarINT32(L"nSelectedIdx", -1);
						ctrl->paramsDict.SetVarINT32(L"nOptionsCnt", 0);
					}
				}
				if (!__Leaderboards().IsBusy())
				{
					__Texts().SetString(STR_LEADERBOARDS_NAMES_VAL, L"...");
					__Texts().SetString(STR_LEADERBOARDS_SCORES_VAL, L"...");

					if (nLeaderboardID == 0)
						__Leaderboards().QueueJob(K_JOB_GET_SCORES_GLOBAL, K_GAME_STR_LEADERBOARDS_GLOBAL_SP, 1 + nPageIdx * K_LB_SCORES_LIST_SIZE);
					else
						__Leaderboards().QueueJob(K_JOB_GET_SCORES_GLOBAL, K_GAME_STR_LEADERBOARDS_GLOBAL_COOP, 1 + nPageIdx * K_LB_SCORES_LIST_SIZE);
				}

				return true;
			}

		}
		else if (nEvent.m_eventCommand == CEventCommands::evtC_CONTROLS_SELECTION_CHANGED)
		{
			UINT32 layerID = nEvent.GetArgumentByName(L"layerID")->m_asUINT32;
			UINT32 ctrlID = nEvent.GetArgumentByName(L"ctrlID")->m_asUINT32;
			int selection = nEvent.GetArgumentByName(L"nSelectedIdx")->m_asINT32;

			//redefine keys control for key capture
			if (ctrlID == HASH("CTRL_KEYGRABBER"))
			{
				//save controller instance ID and command that need redefining
				int nKeyboardOrdinal = nEvent.GetArgumentByName(L"nKeyboardOrdinal")->m_asINT32;
				int nSDLcommand = nEvent.GetArgumentByName(L"nSDLcommand")->m_asINT32;
				int nKeycode = nEvent.GetArgumentByName(L"nSDLscancode")->m_asINT32;
				//check validity
				if (nKeycode == SDL_SCANCODE_ESCAPE)
				{
					//hide layer
					__GUI().RemoveTopmostLayer();
				}
				else if ((nKeycode >= SDL_SCANCODE_F1) && (nKeycode <= SDL_SCANCODE_F12))
				{
					__GUI().MessageBoxOK(STR_WARNING, STR_KEY_INVALID);
				}
				else
				{
					//hide layer
					__GUI().RemoveTopmostLayer();
					//overwrite user command
					for (int kk = K_MEMID_KEYSALL_START; kk <= K_MEMID_KEYSALL_END; kk++)
					{
						if (g_userData[kk] == nKeycode)
							g_userData[kk] = -1; //unset used key
					}
					//set new key
					CLAMP(nKeyboardOrdinal, 0, 1);
					int nKeysOff = K_MEMID_KEYS2_FIRSTITEM - K_MEMID_KEYS1_FIRSTITEM;
					g_userData[K_MEMID_KEY1_LEFT + (nKeyboardOrdinal * nKeysOff) + nSDLcommand] = nKeycode;
					//change actual triggers
					CController* keybd1 = __Controllers().GetControllerByInstanceID(K_CM_IID_KBM1);
					CController* keybd2 = nullptr;// UTGetCtrlrMgr().GetControllerByInstanceID(K_CM_DEFAULT_KEYBOARD2_INSTANCE_ID);
					App_SetSDLTriggersFromUserData(keybd1, keybd2);
				}

				return true;
			}
			else if (ctrlID == HASH("CTRL_SCORESLIST_TT"))
			{
				int nSelIdx = nEvent.GetArgumentByName(L"nSelectedIdx")->m_asUINT32;
				int nSelPage = nEvent.GetArgumentByName(L"nPageIdx")->m_asUINT32;
#if defined(_DEBUG) || defined(DEBUG)
				//#PORTING: xbox code here to open selected player's page
				LOG(L"Scores list selected index:%d page:%d", nSelIdx, nSelPage);
#endif
			}
			//control custom paint selectie ability
			else if (ctrlID == HASH("CTRL_STRATEGIC_BAR"))
			{
				int nSDLinstanceID = nEvent.GetArgumentByName(L"nSDLinstanceID")->m_asINT32;
				int nSelectedIdx = nEvent.GetArgumentByName(L"nSelectedIdx")->m_asINT32;

				if (__Sim().ActivateSpecialAbility(nSelectedIdx, nSDLinstanceID))
				{
					__GUI().RemoveTopmostLayer();
				}

				return true;
			}
			else if (ctrlID == HASH("CTRL_LANGLIST_TT"))
			{
				int nLangIdx = nEvent.GetArgumentByName(L"nSelectedIdx")->m_asINT32;
				//remove layer
				__GUI().RemoveTopmostLayer();
				//change language
				if (nLangIdx >= 0)
				{
					CLAMP(nLangIdx, 0, g_arrLangList.GetSize() - 1);

					App_LocaChangeLanguage(g_arrLangList[nLangIdx].shLangAlias);
					App_UpdateLevelStats();

					UTApp().SaveSettings();
					//set version number
					__Texts().SetString(STR_VERSION_NUMBER, L"v%d.%d.%d", _VERSION_MAJOR_, _VERSION_MINOR_, _VERSION_PATCH_);
				}
				return true;
			}
			else if (ctrlID == HASH("CTRL_LOBBIES_SELECTOR"))
			{
				int nLobbyIdx = nEvent.GetArgumentByName(L"nSelectedIdx")->m_asINT32;
				//change language
				if (nLobbyIdx >= 0)
				{
					uint64_t iLobbyID = 0;
					char strLobbyName[250];
					if (g_pNetwork->GetLobbyListEntry(nLobbyIdx, iLobbyID, strLobbyName))
					{
						//save lobby ID for connection
						g_netlock.m_ullCurLobbyID = iLobbyID;

						LOG(L"Game::COOP Joining Lobby LobbyID: %llu", g_netlock.m_ullCurLobbyID);
						//change state
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_NET_LOBBY);
						nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
						//set joining state
						nevent->AddNamedArgINT32(L"arg1", (int)CApplicationSettings::K_NETGAME_TYPE_QUICK_MATCH);
						__Events().QueueEvent(nevent);
					}
				}
				return true;
			}
			//redefine keys
			else if (ctrlID == HASH("CTRL_KEYS_SELECTOR"))
			{
				int nKeysOff = K_MEMID_KEYS2_FIRSTITEM - K_MEMID_KEYS1_FIRSTITEM;
				//selectia se trimite ca si comanda + nKeysCnt * keyboardOrdinal
				int command = selection % nKeysOff;
				int keyboardOrdinal = selection / nKeysOff;

				CCtrlLayer* layer = __GUI().ShowLayerOnce("LAYER_ID_KEYGRAB");
				CControl* ctrl = null;
				if (layer != null)
				{
					if (ctrl = layer->GetControlByName("LABEL_COMMAND"))
					{
						ctrl->paramsDict.SetVarINT32(L"stringID", STR_KEY_LEFT + command);
					}
					if (ctrl = layer->GetControlByName("CTRL_KEYGRABBER"))
					{
						//save keyboard ordinal (0 or 1 for Keyboard1 and Keyboard2) and the command that need redefining
						ctrl->paramsDict.SetVarINT32(L"nKeyboardOrdinal", keyboardOrdinal);
						ctrl->paramsDict.SetVarINT32(L"nSDLcommand", command);
					}
				}

				return true;
			}
			else if (ctrlID == HASH("CTRL_SCROLL_MAINMENU"))
			{
				switch (selection)
				{
					case STR_COOP:
					{
#ifdef ENABLE_NETWORKING
						__GUI().ShowLayerOnce("LAYER_ID_COOP_WND");
#endif
					}
					break;

					case STR_WORKSHOP:
					{
#ifdef ENABLE_STEAM_WORKSHOP
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_WORKSHOP);
						nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
						__Events().QueueEvent(nevent);
#endif					
					}
					break;

					case STR_PLAY:
					{
						g_userData[K_MEMID_SELECTED_CHAPTER] = 0;
						g_userData[K_MEMID_SELECTED_LEVEL] = 0;

						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_PLAYER_SELECTION);
						nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
						__Events().QueueEvent(nevent);
					}
					break;
					case STR_LEADERBOARDS:
					{
#ifdef ENABLE_LEADERBOARDS
						//reset strings
						__Texts().SetString(STR_LEADERBOARDS_NAMES_VAL, L"...");
						__Texts().SetString(STR_LEADERBOARDS_SCORES_VAL, L"...");
						//request single player scores
						__Leaderboards().QueueJob(K_JOB_GET_SCORES_GLOBAL, K_GAME_STR_LEADERBOARDS_GLOBAL_SP, 1);
						//write score for YOUR SCORE label
						__Texts().SetString(STR_TEMP15, L"%d", g_userData[K_MEMID_TOTAL_SCORE_SOLO]);

						CCtrlLayer* pLay = __GUI().ShowLayerOnce("LAYER_ID_LEADERBOARDS_MM");
						//reset scroll page and save leaderboard index as a payload in this control
						if (pLay != null)
						{
							// set scores list on empty
							CControl *ctrl = pLay->GetControlByName("CTRL_SCORESLIST_TT");
							if (ctrl != null)
							{
								ctrl->paramsDict.SetVarINT32(L"nOptionsCnt", 0);
								ctrl->paramsDict.SetVarINT32(L"nPage", 0);
								ctrl->paramsDict.SetVarINT32(L"nLeaderboardID", 0); //Single Player
#ifndef ENABLE_LEADERBOARDS_NAMES_SELECTION
								ctrl->paramsDict.SetVarBool(L"bUserCanSelect", false);
#endif
							}
						}
#endif
					}
					break;
					case STR_CREDITS:
					{
						__GUI().ShowLayerOnce("LAYER_ID_CREDITS");
					}
					break;
					case STR_OPTIONS:
					{
						CCtrlLayer* layer = __GUI().ShowLayerOnce("LAYER_ID_OPTIONS_MM");
						if (layer != null)
						{
							CControl* ctrl;
							if (ctrl = layer->GetControlByName("CTRL_SLIDER_SOUNDVOL"))
							{
								ctrl->paramsDict.SetVarFloat(L"fSlidePercent", m_Settings.fSoundsVolume);
							}
							if (ctrl = layer->GetControlByName("CTRL_SLIDER_MUSICVOL"))
							{
								ctrl->paramsDict.SetVarFloat(L"fSlidePercent", m_Settings.fMusicVolume);
							}
						}
					}
					break;
					case STR_EXIT:
						__GUI().ShowLayerOnce("LAYER_ID_QUITGAME");
						break;
				}
				return true;
			}
			else if (ctrlID == HASH("CTRL_SCROLL_IGMMENU"))
			{
				switch (selection)
				{
					case STR_RESTART_LEVEL:
					{
						CEvent *nevent = new CEvent(CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION);
						nevent->AddNamedArgUINT32(L"newGameState", GAME_STATE_PLAYER_SELECTION);
						nevent->AddNamedArgINT32(L"arg1", 0); //reset player selection
						nevent->AddNamedArgINT32(L"transitionType", TRANSITION_SIMPLE);
						__Events().QueueEvent(nevent);
					}
					break;
					case STR_QUIT:
					{
						__GUI().ShowLayerOnce("LAYER_ID_QUITLEVEL");
					}
					break;
					case STR_OPTIONS:
					{
						CCtrlLayer* layer = __GUI().ShowLayerOnce("LAYER_ID_OPTIONS_IGM");
						if (layer != null)
						{
							CControl* ctrl;
							if (ctrl = layer->GetControlByName("CTRL_SLIDER_SOUNDVOL"))
							{
								ctrl->paramsDict.SetVarFloat(L"fSlidePercent", m_Settings.fSoundsVolume);
							}
							if (ctrl = layer->GetControlByName("CTRL_SLIDER_MUSICVOL"))
							{
								ctrl->paramsDict.SetVarFloat(L"fSlidePercent", m_Settings.fMusicVolume);
							}
						}
					}
					break;
					case STR_RESUME:
					default:
					{
						SND_PLAY(SNDIDX_DENIED);
						__GUI().RemoveTopmostLayer();
					}
					break;
				}

				return true;
			}
			else if (ctrlID == HASH("CTRL_SCROLL_IGMMENU_NET"))
			{
				switch (selection)
				{
					case STR_QUIT:
					{
						__GUI().ShowLayerOnce("LAYER_ID_QUITLEVEL_NET");
					}
					break;

					case STR_OPTIONS:
					{
						CCtrlLayer* layer = __GUI().ShowLayerOnce("LAYER_ID_OPTIONS_IGM");
						if (layer != null)
						{
							CControl* ctrl;
							if (ctrl = layer->GetControlByName("CTRL_SLIDER_SOUNDVOL"))
							{
								ctrl->paramsDict.SetVarFloat(L"fSlidePercent", m_Settings.fSoundsVolume);
							}
							if (ctrl = layer->GetControlByName("CTRL_SLIDER_MUSICVOL"))
							{
								ctrl->paramsDict.SetVarFloat(L"fSlidePercent", m_Settings.fMusicVolume);
							}
						}
					}
					break;

					case STR_ABORT_MISSION:
					{
						if (__Sim().m_levelState == K_LVL_STATE_PLAYING)
						{
							//can ask for restart if alive
							CActor *pPlayer = __Sim().pPlayerActor[g_netlock.Net_GetPlayerIndex()];
							if ((UTApp().IsGameNetworked()) && (pPlayer != null) && (pPlayer->fLife > 0.0f))
							{
								//send silent restart command by chat
								g_ChatWnd.AddLine(K_CW_STR_CMD_NET_ABORT_MISSION, L"SYSTEM", 0xffff0000);
								//send network line
								g_netlock.Net_SendChatLine(K_CW_STR_CMD_NET_ABORT_MISSION);
							}
							else
							{
								g_ChatWnd.AddLine(K_CW_STR_CMD_NET_ASK_ABORT_MISSION, g_netlock.m_sNames[g_netlock.Net_GetPlayerIndex()].text, K_CW_SYSTEM_COLOR_GREEN);
								//send network line
								g_netlock.Net_SendChatLine(K_CW_STR_CMD_NET_ASK_ABORT_MISSION);
							}

							//close menu
							__GUI().RemoveTopmostLayer();
						}
					}
					break;

					case STR_RESUME:
					default:
					{
						SND_PLAY(SNDIDX_DENIED);
						__GUI().RemoveTopmostLayer();
					}
					break;
				}

				return true;
			}

		}
	}
	
	return false;
}


#if defined(ENABLE_STEAM)
//-----------------------------------------------------------------------------
// Purpose: Handles notification that the Steam overlay is shown/hidden, note, this
// doesn't mean the overlay will or will not draw, it may still draw when not active.
// This does mean the time when the overlay takes over input focus from the game.
//-----------------------------------------------------------------------------
void CApplication::OnGameOverlayActivated(GameOverlayActivated_t *callback)
{
	if (callback->m_bActive)
	{
	}
	else
	{
	}
}
#endif

///**************************************************************************************
/// SDL related
///**************************************************************************************
#if defined(K_GLOBAL_ENABLE_SDL)

#if defined(K_SDL_IGNORE_MOUSE_EVENTS)
int SDLFilter_IsMouseEvent(void * userdata, SDL_Event* event)
{
	if (event->type == SDL_MOUSEMOTION)
		return 0;
	return 1;
}
#endif


bool CApplication::InitSDL(HWND hWnd)
{
	WCHAR txt[MAX_PATH];
	//Initialization flag

	//print out version used
	SDL_version compiled;
	SDL_version linked;

	SDL_VERSION(&compiled);
	SDL_GetVersion(&linked);
	LOG(L"SDL Init. compiled v%d.%d.%d linked v%d.%d.%d", compiled.major, compiled.minor, compiled.patch, linked.major, linked.minor, linked.patch);

	//Initialize SDL (video needed for window messages keys and mouse)
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
	{
		mbstowcs(txt, SDL_GetError(), MAX_PATH);
		ErrorBox(K_ERR_CRITICAL, L"SDL could not initialize! SDL Error: %s\n", txt);
		return false;
	}
	//Create window - needed for keyboard input
	gWindow = SDL_CreateWindowFrom((void*)hWnd);
	if (gWindow == nullptr)
	{
		mbstowcs(txt, SDL_GetError(), MAX_PATH);
		ErrorBox(K_ERR_CRITICAL, L"SDL Window couldn't initialize! SDL Error: %s\n", txt);
		return false;
	}

	// load custom mappings (might not work on chinese paths)
	/*
	WCHAR dbpath[MAX_PATH];
	swprintf_s(dbpath, MAX_PATH, L"%sgamecontrollerdb.txt", g_wszExePath);
	CHAR txtpath[1024] = { 0 };
	WCHARtoUTF8(txtpath, dbpath, 1024);
	*/
	//#HINT: most recent definitions are here: https://github.com/gabomdq/SDL_GameControllerDB
	// Make sure you have the WorkingDir set to $TargetDir or it won't load. If you can't do that, use the commented path built above.
	int nLoaded = SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
	if (nLoaded < 0)
	{
		mbstowcs(txt, SDL_GetError(), MAX_PATH);
		ErrorBox(K_ERR_LOG, L"SDL Couldn't load controller mappings from file! SDL Error: %s\n", txt);
	}
	else
	{
		LOG(L"SDL loaded %d controller mappings from file.", nLoaded);
	}

#if defined(K_SDL_IGNORE_MOUSE_EVENTS)
	SDL_SetEventFilter(SDLFilter_IsMouseEvent, null);
#endif

	return true;
}


void CApplication::CloseSDL()
{
	//Destroy window
	SDL_DestroyWindow(gWindow);
	gWindow = nullptr;

	//Quit SDL subsystems
	SDL_Quit();
}

//gets input data from SDL controllers
void CApplication::PollSDLControllers()
{
	// when using imGUI check if it wants exclusive control
	bool bIgnoreMouse = false;
#if defined(K_ENABLE_IMGUI)
	if (__ImGui().bEnabled && __ImGui().GetWantCaptureMouse())
		bIgnoreMouse = true;
#endif

	//---------------------------------------
	// SDL Event handler
	//---------------------------------------
	SDL_Event e;
	//Handle events on queue
	while (SDL_PollEvent(&e) != 0)
	{
		switch (e.type)
		{
#if !defined(K_SDL_IGNORE_MOUSE_EVENTS)
			//-- pointer ---
			case SDL_MOUSEMOTION:
			{
				if (!bIgnoreMouse)
				{
					__Controllers().OnSDLMouseMove(e.motion);
				}
			}
			break;
			case SDL_MOUSEBUTTONDOWN:
			{
				if (!bIgnoreMouse)
				{
					__Controllers().OnSDLMouseButton(e.button);
				}
			}
			break;
			case SDL_MOUSEBUTTONUP:
			{
				if (!bIgnoreMouse)
				{
					__Controllers().OnSDLMouseButton(e.button);
				}
			}
			break;
#endif
			//-- keyboard ---
			case SDL_KEYDOWN:
			{
				__Controllers().OnSDLKeypress(e.key, true);
				//send key up event to controls manager (for key redefining mostly)
				__GUI().ReceiveInput(K_CCTRLMGR_INPUT_SDL_KEY, 1, (int)e.key.keysym.scancode);
			}
			break;
			case SDL_KEYUP:
			{
				__Controllers().OnSDLKeypress(e.key, false);
				//send key up event to controls manager
				__GUI().ReceiveInput(K_CCTRLMGR_INPUT_SDL_KEY, 0, (int)e.key.keysym.scancode);
			}
			break;
			//--- controllers ---
			case SDL_CONTROLLERDEVICEADDED:
			{
				__Controllers().AddSDLController(e.cdevice.which);
			}
			break;

			case SDL_CONTROLLERDEVICEREMOVED:
			{
				__Controllers().RemoveSDLController(e.cdevice.which);
			}
			break;

			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
			{
				__Controllers().OnSDLControllerButton(e.cbutton);
			}
			break;

			case SDL_CONTROLLERAXISMOTION:
			{
				__Controllers().OnSDLControllerAxis(e.caxis);
			}
			break;
		}
	}
}

#endif


void CApplication::App_OnLevelFinished(int nEpisodeIdx, int nLevelIdx)
{
	if (!UTGetChaptersList().IsValidLevel(nEpisodeIdx, nLevelIdx))
	{
		LOG(L"[WARNING] App_OnLevelFinished: Invalid episode and level index! episode:%d level:%d", nEpisodeIdx, nLevelIdx);
		return;
	}

	int nStarsPerChapter = 0;
	int nLevelsFinishedPerChapter = 0;
	for (int ii = 0; ii < UTGetChaptersList().m_arrChapters[nEpisodeIdx]->nLevelsCnt; ii++)
	{
		int nRealIdx = ii + nEpisodeIdx * K_GAME_LEVELS_PER_CHAPTER;
		nStarsPerChapter += g_levelStats[nRealIdx].nStars;
		if (g_levelStats[nRealIdx].nStars > 0)
			nLevelsFinishedPerChapter++;
	}
	//#ACHIEVEMENTS: chapter achievements on level data (stars and all levels finished)
	int arr_ach_3_stars[] = {ACH_CLEANUP_THE_HOOD, ACH_NO_QUARTER, ACH_WE_STAND_ON_GUARD, ACH_NOT_IN_MY_CITY, ACH_CARTEL_ANNIHILATOR, -1 /*WEEKLY*/, ACH_KING_OF_CASTLE, -1, -1, -1};
	int arr_ach_all_played[] = {ACH_GANGLAND_PACIFIER, ACH_RADICALIZED, ACH_ALERT_EXTINGUISHED, ACH_BITE_THE_APPLE, ACH_NOT_ON_MY_WATCH, -1 /*WEEKLY*/, ACH_COURT_IS_CLEAR, -1, -1, -1};

	//3 stars all on chapter
	if ((arr_ach_3_stars[nEpisodeIdx] >= 0) && (nStarsPerChapter == 3 * K_GAME_LEVELS_PER_CHAPTER))
		__Achievements().UnlockAchievement((EGameAchievements)arr_ach_3_stars[nEpisodeIdx]);
	//all levels finished in chapter
	if ((arr_ach_all_played[nEpisodeIdx] >= 0) && (nLevelsFinishedPerChapter == K_GAME_LEVELS_PER_CHAPTER))
		__Achievements().UnlockAchievement((EGameAchievements)arr_ach_all_played[nEpisodeIdx]);
}



HRESULT CApplication::OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc , void* pUserContext )
{
	g_texManager.OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc);
	g_sprMgrGlobal.OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc);

	return S_OK;
}

HRESULT CApplication::OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc , void* pUserContext )
{
	HRESULT  hr = S_OK;

	g_texManager.OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc);
	g_sprMgrGlobal.OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc);

	//-- find all available resolutions --
	g_arrResolutions.RemoveAll();

	DXUTDeviceSettings eDeviceSettings = DXUTGetDeviceSettings();
	CD3DEnumeration* pD3DEnum = DXUTGetEnumeration();
	CD3DEnumAdapterInfo* pAdapterInfo = pD3DEnum->GetAdapterInfo(eDeviceSettings.AdapterOrdinal);
	if (pAdapterInfo == nullptr)
	{
		ErrorBox(K_ERR_WARNING, L"CApplication::OnResetDevice - Can't load resolution list!");
	}
	else
	{
		for (int idm = 0; idm < pAdapterInfo->displayModeList.GetSize(); idm++)
		{
			D3DDISPLAYMODE DisplayMode = pAdapterInfo->displayModeList.GetAt(idm);

			if (DisplayMode.Format == eDeviceSettings.AdapterFormat)
			{
				SizeWHi szres;
				szres.w = (int)DisplayMode.Width;
				szres.h = (int)DisplayMode.Height;
				//rezolutiile prea mici sunt sarite
				if ((szres.w < K_WINDOW_WIDTH_MIN) || (szres.h < K_WINDOW_HEIGHT_MIN))
					continue;
				//Rezolutiile apar dublate pe monitoarele capabile de mai multe refresh rates. Aici sunt filtrate.
				if (!g_arrResolutions.Contains(szres))
				{
					g_arrResolutions.Add(szres);
				}
			}
		}
	}

	return hr;
}

HRESULT CApplication::OnLostDevice(void* pUserContext)
{
	g_texManager.OnLostDevice();
	g_sprMgrGlobal.OnLostDevice();

	g_arrResolutions.RemoveAll();
	return S_OK;
}

HRESULT CApplication::OnDestroyDevice(void* pUserContext)
{
	g_texManager.OnDestroyDevice();
	g_sprMgrGlobal.OnDestroyDevice();

	return S_OK;
}


///**************************************************************************************
/// Sigleton 
///**************************************************************************************

CApplication& UTApp()
{
	static CApplication g_Application;
	return g_Application;
}

