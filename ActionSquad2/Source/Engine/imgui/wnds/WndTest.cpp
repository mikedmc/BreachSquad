#include "dxstdafx.h"
#include "WndTest.h"

CWndTest::CWndTest(const char * sName, bool bShow, ImGuiWindowFlags wndFlags) :
	// call parent class constructor (call it in initializer list)
	CimguiWndInterface(sName, bShow, wndFlags)
{
	//LOG("CWndTest CTOR CALLED! [%s] show:%d flags:%d", sName, bShow, wndFlags);
}

CWndTest::~CWndTest()
{
	//LOG("CWndTest DTOR CALLED! name:%s", strName);
}

void CWndTest::Paint()
{
	///--- START WINDOW
	// Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
	ImGui::Begin(strName, &bIsOpen, nFlags);   

	///--- write CONTENT here
	ImGui::Text("Hello from %s!", strName);
	if (ImGui::Button("Close it"))
		bIsOpen = false;

	///--- END WINDOW
	ImGui::End();
}
