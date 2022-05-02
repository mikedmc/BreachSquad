#include "dxstdafx.h"
#include "imguiWrapper.h"


void CimguiWrapper::SetGlobalEnabled(bool bEnable)
{
	bEnabled = bEnable;
}

bool CimguiWrapper::GetWantCaptureMouse()
{
	return ImGui::GetIO().WantCaptureMouse;
}

bool CimguiWrapper::GetWantCaptureKeyboard()
{
	return ImGui::GetIO().WantCaptureKeyboard;
}

void CimguiWrapper::Init(PDEVICE pDevice, HWND hwnd)
{
	// create ini file path
	char sTempPath[MAX_PATH];
	wcstombs(sTempPath, UTApp().g_wszTempFolderPath, MAX_PATH);
	sprintf_s(sIniPath, "%simgui.ini", sTempPath);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = sIniPath;									// save ini file location
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX9_Init(pDevice);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	bInitialized = true;
	LOG("[IMGUI] v%s Initialized! ini file: %s", ImGui::GetVersion(), sIniPath);
}

bool CimguiWrapper::BeginPaint()
{
	// not enabled? skip everything
	if (bEnabled == false || bInitialized == false)
		return false;

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	return true;
}

void CimguiWrapper::EndPaint(PDEVICE pDevice)
{
	if (bEnabled == false || bInitialized == false)
		return;
	// Rendering
	ImGui::EndFrame();
	pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
	D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(255, 0, 0, 255);
	pDevice->Clear(0, NULL, D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
	if (pDevice->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		pDevice->EndScene();
	}

	// Update and Render additional Platform Windows
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

}


void CimguiWrapper::AddWatch_Int( WCHAR* varname, int value )
{
	arrDebugVars.SetVarINT32( varname, value );
}

void CimguiWrapper::AddWatch_Float( WCHAR* varname, float value )
{
	arrDebugVars.SetVarFloat( varname, value );
}

void CimguiWrapper::PaintDebugVars()
{
	if ( __ImGui().arrDebugVars.GetSize() == 0 )
	{
		ImGui::Text( "No Debug Watch Vars!" );
		return;
	}

	CStringHashA cname;
	for(auto & elem : __ImGui().arrDebugVars.m_variants)
	{
		CVariant* cvc = &elem.second;
		switch ( cvc->eType )
		{
			case CVariant::K_ARGTYPE_INT32:
				cname = cvc->shName;
				ImGui::Text( "%s: %d", cname.text, cvc->m_asINT32 );
				break;
			case CVariant::K_ARGTYPE_FLOAT:
				cname = cvc->shName;
				ImGui::Text( "%s: %.2f", cname.text, cvc->m_asFloat );
				break;
			default:
				break;
		}
	}
}

CimguiWrapper::CimguiWrapper() : bEnabled(false), bInitialized(false)
{
	
}


HRESULT CimguiWrapper::OnCreateDevice(PDEVICE pDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc /*= NULL*/)
{
	Init(pDevice, DXUTGetHWND());

	return S_OK;
}

HRESULT CimguiWrapper::OnResetDevice(PDEVICE pDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc /*= NULL*/)
{
	bInitialized = true;
	ImGui_ImplDX9_CreateDeviceObjects();
	return S_OK;
}

HRESULT CimguiWrapper::OnLostDevice(void)
{
	bInitialized = false;
	ImGui_ImplDX9_InvalidateDeviceObjects();
	return S_OK;
}

HRESULT CimguiWrapper::OnDestroyDevice(void)
{
	bInitialized = false;
	bEnabled = false;
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	LOG("[IMGUI] Released.");

	return S_OK;
}


///**************************************************************************************
/// Sigleton
///**************************************************************************************

CimguiWrapper& __ImGui()
{
	static CimguiWrapper g_imguiWrapper;
	return g_imguiWrapper;
}

