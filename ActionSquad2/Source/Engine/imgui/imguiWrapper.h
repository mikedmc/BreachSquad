#pragma once

#include <vector>

#include "wnds/imguiWndInterface.h"
#include "wnds/WndTest.h"

using namespace std;
//#TODO: make a class with empty methods and include that when imgui is off

class CimguiWrapper
{
private:
	vector<shared_ptr<CimguiWndInterface>> arrWnds;
public:
	bool				bEnabled;		// Enables all updating and rendering

public:
	// Enables or disables imgui
	void				SetGlobalEnabled(bool bEnable);
	// IMGUI wants mouse exclusively
	bool				GetWantCaptureMouse();
	// IMGUI wants keyboard exclusively
	bool				GetWantCaptureKeyboard();
	
	// Initializes all systems
	void				Init(PDEVICE pDevice, HWND hwnd);
	// Paint and update in one step
	void				Paint(PDEVICE pDevice, bool show_demo_window, bool show_another_window, ImVec4 clear_color);

	// Adds a new window to the rendering collection
	// std::vector<std::shared_ptr<AbstractBase>> v = { std::make_shared<Derived>() };
	// use like AddWindow(std::make_shared<Derived>());
	shared_ptr<CimguiWndInterface>	AddWindow(shared_ptr<CimguiWndInterface> wndptr);

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