#pragma once

//#TODO: make a class with empty methods and include that when imgui is off

class CimguiWrapper
{
private:
	char				sIniPath[MAX_PATH]{};	// Path of ini file
	bool				bInitialized;			// Is IMGUI initialized?
public:
	bool				bEnabled;			// Enables all updating and rendering

public:
	// Enables or disables imgui
	void				SetGlobalEnabled(bool bEnable);
	// IMGUI wants mouse exclusively
	bool				GetWantCaptureMouse();
	// IMGUI wants keyboard exclusively
	bool				GetWantCaptureKeyboard();
	// Is IMGUI enabled?
	inline bool			IsEnabled() {
		return bEnabled && bInitialized;
	};
	
	// Initializes all systems
	void				Init(PDEVICE pDevice, HWND hwnd);
	// Starts a new frame
	bool				BeginPaint();
	// Paints controls
	void				EndPaint(PDEVICE pDevice);

public:
	CimguiWrapper();

	//--- system framework ---
	HRESULT				OnCreateDevice(PDEVICE pDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT				OnResetDevice(PDEVICE pDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT				OnLostDevice(void);
	HRESULT				OnDestroyDevice(void);
};

//declare singleton
CimguiWrapper& UTimgui();