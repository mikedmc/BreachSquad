#pragma once

// Interface that all IMGUI windows must be derived from
class CimguiWndInterface
{
protected:
	ImGuiWindowFlags	nFlags;				// can this window be closed from the X button?

public:
	bool				bIsOpen;					// is this window rendering?
	char				strName[MAX_PATH]{};		// name of the window

	CimguiWndInterface(const char * sName, bool bShow, ImGuiWindowFlags wndFlags = 0)
	{
		memset(strName, 0, sizeof(char) * MAX_PATH);
		nFlags = wndFlags;
		bIsOpen = bShow;
		strcpy_s(strName, MAX_PATH, sName);
	}

	virtual ~CimguiWndInterface() = default;		// defaulted virtual destructor

	virtual void		Paint() = 0;				// Implement Paint with IMGUI instructions
};


