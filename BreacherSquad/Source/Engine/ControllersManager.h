#pragma once
#include "dxstdafx.h"

// \brief The type of callback member function to use when normalizing XY coords, returning data in ret_fX/Y
//
// Some coordinates (like mouse position) depend on window size but we want them to correspond between
// network peers so they need to be normalized into game/level space, relative to controller owner character
typedef void (*NormalizeCoordsFn)(int ControllerIID, float fAxisValue, bool bIsHorizontalAxis, float & ret_fAxisValue);

// Each controller has a list of triggers that translate to commands. you can have more triggers per command, each adding its value to the total axis value.
enum EControllerTriggerType {
	// 0/1 button that adds to command: fTriggerMin when released and fTriggerMax when pressed
	K_CM_BUTTON = 0,
	// analog half axis (used to transform analog movement on same axis to 2 separate commands)
	// activated when axis >= fTriggerMin && axis <= fTriggerMax
	K_CM_HALF_AXIS,
	// analog full axis converted into a single command
	// activated when fabs(axis) > fabs(fTriggerMin)
	K_CM_AXIS,
	// mouse or touch pointer split into 2 axis
	// can/should/will be transformed to game space, relative to controller owner character
	K_CM_POINTER_X,
	K_CM_POINTER_Y,
	// mouse button  or touch event
	// 0/1 button that adds to command: fTriggerMin when released and fTriggerMax when pressed
	K_CM_POINTER_BUTTON,
};

// Controller types
enum EControllerType {
	K_CM_CT_INVALID = -1,
	K_CM_CT_KBM_SDL = 0,						// KeyBoard and Mouse type (one per computer)
	K_CM_CT_JOYSTICK_SDL,						// SDL controller
	//K_CM_CT_KEYBOARD_WIN ,					// takes input from windows messages
	K_CM_CT_NET_FRAMELOCK,						// framelock networked helper controller
	//count
	K_CM_CTS_CNT,		
};

// Buttons statuses
enum EControllerButtonState {
	K_CM_BUTSTATE_NOTPRESSED = 0,
	K_CM_BUTSTATE_JUSTPRESSED,
	K_CM_BUTSTATE_PRESSING,
	K_CM_BUTSTATE_JUSTRELEASED,
};

// Virtual controller commands: commands - game specific
// Don't change the order! Synced with strings and memid items
enum EControllerCommand {
	K_CM_COMMAND_NONE = -1, //not used, default

	K_CM_COMMAND_MOVE_X = 0,
	K_CM_COMMAND_MOVE_Y,
	K_CM_COMMAND_AIM_X,
	K_CM_COMMAND_AIM_Y,
	K_CM_COMMAND_JUMP,
	K_CM_COMMAND_FIRE1,
	K_CM_COMMAND_FIRE2,
	K_CM_COMMAND_RELOAD,
	K_CM_COMMAND_USE_GEAR,
	K_CM_COMMAND_MELEE,
	K_CM_COMMAND_STRATEGIC_MENU,

	K_CM_COMMAND_SELECT, //used mainly for menus
	K_CM_COMMAND_BACK, //used mainly for menus
	//count
	K_CM_COMMANDS_COUNT
};

#define K_CM_AXIS_START K_CM_COMMAND_MOVE_X
#define K_CM_AXIS_END K_CM_COMMAND_AIM_Y
#define K_CM_BUTT_START K_CM_COMMAND_JUMP
#define K_CM_BUTT_END K_CM_COMMAND_BACK


const CStringHash EControllerCommandNames[] = {
	L"COMMAND_MOVE_X",
	L"COMMAND_MOVE_Y",
	L"COMMAND_AIM_X",
	L"COMMAND_AIM_Y",
	L"COMMAND_JUMP",
	L"COMMAND_FIRE1",
	L"COMMAND_FIRE2",
	L"COMMAND_RELOAD",
	L"COMMAND_USE_GEAR",
	L"COMMAND_MELEE",
	L"COMMAND_STRATEGIC_MENU",
	L"COMMAND_SELECT", 
	L"COMMAND_BACK", 
};

// Input trigger for commands
class CControllerTrigger {
public:
	EControllerTriggerType	eType;						// button or axis or pointer
	EControllerCommand		eTargetCommand;				// action that the trigger does
	float					fTriggerMin, fTriggerMax;	// trigger min max values
	int						keyMapping;					// trigger value(specific to command): When using SDL_KEYBOARD map with SDL scancodes, WIN_KEYBOARD with VK_ codes and Joysticks with SDL_CONTROLLER_AXIS_ and SDL_CONTROLLER_BUTTON_

	float					fTriggerActivatedPercent;	//procentul de activare a triggerului
};

///--- controller class ---
//max number of triggers per controller
#define K_CM_MAX_TRIGGERS 30

// Default KBM IID, added from the start on PC builds
#define K_CM_IID_KBM1 0xffff0
// Default virtual network controller ID	(if we'll ever need more network controllers we should add a flag bPlayerIsNetworked)
#define K_CM_IID_NET1 0xffff5

//--- controller flags ---
//flag that tells us that the controller is paused
#define K_CM_CTRLR_FLAG_PAUSED	1
//other controller flags
#define K_CM_CTRLR_FLAG_ETC		2

class CController
{
public:
	// structure that keeps data about all commands in a controller
	struct sControllerCommands
	{
		bool					bKeyDown[K_CM_COMMANDS_COUNT];			//pressed or not
		EControllerButtonState	keyState[K_CM_COMMANDS_COUNT];			//0-not pressed, 1-just pressed, 2-drag, 3-just released
		float					fKeyPressedTime[K_CM_COMMANDS_COUNT];	//time it's been kept pressed
		float					arrAxisVal_N[K_CM_COMMANDS_COUNT];		//analog axis value (finale, after normalization and update)

		/* CTOR */
		sControllerCommands();

		/* resets keypresses */
		void Reset();
	};

public:
	//SDL data
	int						nSDLidx;						//controller index
	int						nSDLInstanceId;					//instance id used by SDL
	SDL_GameController		*SDLpgc;						//pointer to controller
public:
	EControllerType			eType;							//controller type
	sControllerCommands		sCommands;						//controller commands (to be used ingame)
	int						nFlags;							//flags needed sometimes
public:
	int						arrTriggersCnt;
	CControllerTrigger		arrTriggers[K_CM_MAX_TRIGGERS]; //array of triggers
	CStringHash				strName;						//controller name
																	
public:
	CController();

	// Adds a trigger for a specific command
	// param: fTriggerMin si fTriggerMax will be axis sorted (negative, min is -1.1  max is -0.1)
	void AddTrigger(EControllerTriggerType neType, EControllerCommand neCommand, int nKeyMapping, float nfTriggerMin = 0.1f, float nfTriggerMax = 1.1f);

	// Returns the first key mapping for a specified command or -1 if command isn't mapped
	// TODO: it should return all triggers
	int GetKeyMappingForCommand(EControllerCommand neCommand) const;

	// Returns the first trigger for a specified command or null if command isn't mapped
	CControllerTrigger* GetTriggerForCommand(EControllerCommand neCommand);

	// Removes all triggers for a specified command
	void RemoveTriggers(EControllerCommand neCommand);

	 //	Removes trigger by key mapping
	void RemoveTriggerByKeyMapping(int nKeyMapping);

	// Clears all triggers
	void ClearTriggers();

	// \brief Resets all keypresses
	void ResetKeypresses();

	// Gets all keys pressed percentages into the destination array
	void GetKeysDownPercents(float arrDest[K_CM_COMMANDS_COUNT]) const;

	// Tells if a button was pressed on the controller (or a stick too)
	bool WasControllerTouched(bool bSticksToo = false);

	// returns the pressed state of specified command
	inline EControllerButtonState GetButState(const EControllerCommand comm) {
		return sCommands.keyState[comm];
	}

	// returns the pressed percentage of specified command
	inline float GetAxisVal(const EControllerCommand comm) {
		return sCommands.arrAxisVal_N[comm];
	}

	// returns the normalized compound vector for 2 axis commands
	Vec2 GetDoubleAxisVector(const EControllerCommand commXaxis, const EControllerCommand commYaxis, bool bNormalize = false);
};

///--- controllers manager ---

class CControllersManager
{
protected:
	// Number of active controls per controller type
	int					arrControllerTypesCnt[K_CM_CTS_CNT];  
	// pointer to normalization function for absolute axis like mouse coords
	NormalizeCoordsFn	pNormalizeFn;
public:
	std::vector<CController*> m_arrControllers;
	//CTOR/DTOR
	CControllersManager();
	~CControllersManager();

	// Sets the normalize axis function pointer
	void				SetNormalizeCoordsFunctionPtr(NormalizeCoordsFn pFnPtr);

	CController*		AddController(EControllerType neType, WCHAR * strName);
	//finds all connected controllers (keyboard and joysticks) - se cheama la inceputul jocului
	int					RegisterAllSDLControllers(); 
	//deallocates all SDL controllers
	void				ReleaseAllControllers(bool bOnlySDL = false);

	// Get scancode name. Shorten default SDL names before.
	const char*			GetSDLScancodeName(SDL_Scancode scancode);
	// Adds SDL after SDL index
	void				AddSDLController(int SDL_ctrlr_idx);
	// Deletes controller with nnInstanceID from ctrlrs array
	bool				RemoveSDLController(int nnInstanceID);

	void				OnSDLControllerButton(const SDL_ControllerButtonEvent sdlEvent);
	void				OnSDLControllerAxis(const SDL_ControllerAxisEvent sdlEvent);
	void				OnSDLKeypress(const SDL_KeyboardEvent sdlEvent, bool bKeyDown);
	void				OnSDLMouseButton(const SDL_MouseButtonEvent sdlEvent);
	void				OnSDLMouseMove(const SDL_MouseMotionEvent sdlEvent);

	//get pointer to Controller by SDLInstanceID
	CController*		GetControllerByInstanceID(int nnInstanceID);
	//get pointer to Controller by name
	CController*		GetControllerByName(WCHAR* strControllerName);
	// Resets all keypresses to NOT PRESSED
	void				ResetKeypresses(CController* ctrlr);
	// Reset keypresses on all controllers
	void				ResetAllControllersKeypresses();
	// Used to get messages from Windows for K_CM_CONTROLLER_KEYBOARD_WIN - obsolete
	//void ReceiveKeypress(UINT nChar, bool bIsKeyDown, bool bAltDown);
	// Tells you if any key is pressed on any controller (excluding axes)
	bool				KeyPressed(EControllerCommand eCommandFilter = K_CM_COMMAND_NONE);
	// Updates internal controller data, must be called every frame, before using the controller data
	// \param arrOverrideDownPercent - must be an array of K_CM_COMMANDS_CNT length and it gets copied over the internal normalized array (updates are made after it gets copied)
	void				UpdateController(CController* ctrlr, float dTime, float * arrOverrideDownPercents = nullptr);
};


//singleton to access Controllers manager class
CControllersManager& UTGetCtrlrMgr();