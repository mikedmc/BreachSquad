#pragma once

// Interface that all IMGUI windows must be derived from
class CimguiWndInterface
{
protected:
	ImGuiWindowFlags	nFlags;						 // -> enum ImGuiWindowFlags_     // Flags: for Begin(), BeginChild()

public:
	bool				bIsOpen;					// is this window rendering?
	char				strName[MAX_PATH]{};		// name of the window

	// Overwrites the old flags with the newely specified ones
	FORCEINLINE void	SetFlags(ImGuiWindowFlags wndFlags)
	{
		nFlags = wndFlags;
	}

	// XOR new flags over old flags
	FORCEINLINE void	ChangeFlagsXOR(ImGuiWindowFlags wndFlags)
	{
		nFlags ^= wndFlags;
	}

	// Set OPEN window flag
	FORCEINLINE void	SetOpen(bool bOpen) 
	{
		bIsOpen = bOpen;
	}

	// CTOR
	CimguiWndInterface(const char * sName, bool bShow, ImGuiWindowFlags wndFlags = 0)
	{
		memset(strName, 0, sizeof(char) * MAX_PATH);
		nFlags = wndFlags;
		bIsOpen = bShow;
		strcpy_s(strName, MAX_PATH, sName);
	}

	// Defaulted virtual destructor
	virtual ~CimguiWndInterface() = default;		
	
	// Implement Paint with IMGUI instructions
	virtual void		Paint() = 0;				
};


