#pragma once

//#TODO: make a class with empty methods and include that when imgui is off

class CimguiWrapper
{
public:
	void				Init(PDEVICE pDevice, HWND hwnd);
	void				Paint(PDEVICE pDevice, bool show_demo_window, bool show_another_window, ImVec4 clear_color);

public:
	//--- system framework ---
	HRESULT				OnCreateDevice(PDEVICE pDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT				OnResetDevice(PDEVICE pDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT				OnLostDevice(void);
	HRESULT				OnDestroyDevice(void);
};

//declar singletonul
CimguiWrapper& UTimgui();