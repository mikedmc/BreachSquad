#pragma once

#include "imguiWndInterface.h"

class CWndTest : public CimguiWndInterface
{
public:
	CWndTest(const char * sName, bool bShow, ImGuiWindowFlags wndFlags);
	~CWndTest();

	virtual void Paint() override;
};

