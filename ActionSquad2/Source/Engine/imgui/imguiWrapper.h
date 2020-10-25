#pragma once

//#TODO: make a class with empty methods and include that when imgui is off

class CimguiWrapper
{
public:
	bool				bEnabled;		// Enables all updating and rendering

public:
	// Enables or disables imgui
	void				SetEnabled(bool bEnable);
	// IMGUI wants mouse exclusively
	bool				GetWantCaptureMouse();
	// IMGUI wants keyboard exclusively
	bool				GetWantCaptureKeyboard();
	
	void				Init(PDEVICE pDevice, HWND hwnd);
	// Paint and update in one step
	void				Paint(PDEVICE pDevice, bool show_demo_window, bool show_another_window, ImVec4 clear_color);

public:
	CimguiWrapper();

	//--- system framework ---
	HRESULT				OnCreateDevice(PDEVICE pDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT				OnResetDevice(PDEVICE pDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT				OnLostDevice(void);
	HRESULT				OnDestroyDevice(void);
};

//declar singletonul
CimguiWrapper& UTimgui();