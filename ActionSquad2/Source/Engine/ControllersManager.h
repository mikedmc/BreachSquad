#pragma once
//--------------------------------------------------------------------------------------
// Controller handling class
//--------------------------------------------------------------------------------------
//fiecare controller are o lista de "triggers" care pointeaza spre comenzi ca sa poti avea mai multe butoane pe aceeasi actiune
enum EControllerTriggerType {
	K_CM_BUTTYPE_BUTTON = 0,
	K_CM_BUTTYPE_AXIS
};

//controller types
enum EControllerType {
	K_CM_CONTROLLERTYPE_INVALID = -1,
	K_CM_CONTROLLERTYPE_KEYBOARD_WIN = 0,  //tastatura cu input din mesaje de windows
	K_CM_CONTROLLERTYPE_KEYBOARD_SDL,		//tastatura cu mesaje SDL
	K_CM_CONTROLLERTYPE_JOYSTICK_SDL,		//controllere SDL
	K_CM_CONTROLLERTYPE_NETWORK_FRAMELOCK,	//controller folosit pentru framelock

	K_CM_CONTROLLERTYPES_CNT,		//count
};
//buttons status
enum EControllerButtonState {
	K_CM_BUTSTATE_NOTPRESSED = 0,
	K_CM_BUTSTATE_JUSTPRESSED,
	K_CM_BUTSTATE_PRESSING,
	K_CM_BUTSTATE_JUSTRELEASED,
};
//virtual controller commands: commands - game specific
enum EControllerCommand {
	K_CM_COMMAND_NONE = -1, //not used, default
	//don't change the order! Synced with strings and memid items
	K_CM_COMMAND_LEFT = 0,
	K_CM_COMMAND_RIGHT,
	K_CM_COMMAND_UP,
	K_CM_COMMAND_DOWN,
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

const CStringHash EControllerCommandNames[] = {
	L"COMMAND_LEFT",
	L"COMMAND_RIGHT",
	L"COMMAND_UP",
	L"COMMAND_DOWN",
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

//input trigger on commands
class CControllerTrigger {
public:
	EControllerTriggerType	eType;						//button or axis
	EControllerCommand		eTargetCommand;				//comanda pe care o efectueaza triggerul
	float					fTriggerMin, fTriggerMax;	//valorile min si max intre care se face trigger
	int						keyMapping;					//valoare ce activeaza trigger //when using SDL_KEYBOARD map with SDL scancodes, WIN_KEYBOARD with VK_ codes and Joysticks with SDL_CONTROLLER_AXIS_ and SDL_CONTROLLER_BUTTON_

	float					fTriggerActivatedPercent;	//procentul de activare a triggerului
};

///--- controller class ---
//max number of triggers per controller
#define K_CM_MAX_TRIGGERS 30
//default keyboard ID
#define K_CM_DEFAULT_KEYBOARD1_INSTANCE_ID 0xffff0
#define K_CM_DEFAULT_KEYBOARD2_INSTANCE_ID 0xffff1
//default virtual network controller ID	(if we'll ever need more network controllers we should add a flag bPlayerIsNetworked)
#define K_CM_DEFAULT_NETWORK1_INSTANCE_ID 0xffff5

//--- controller flags ---
//flag that tells us that the controller is paused
#define K_CM_CTRLR_FLAG_PAUSED	1
//other controller flags
#define K_CM_CTRLR_FLAG_ETC		2

class CController
{
public:
	struct sControllerCommands
	{
		bool					bKeyDown[K_CM_COMMANDS_COUNT];			//daca e apasat sau nu
		EControllerButtonState	keyState[K_CM_COMMANDS_COUNT];			//0-not pressed, 1-just pressed, 2-drag, 3-just released
		float					fKeyPressedTime[K_CM_COMMANDS_COUNT];	//de cat timp e apasata o tasta anume
		float					fKeyDownPercent[K_CM_COMMANDS_COUNT];	//cat de apasat e (merge si pt axe)

		/* CTOR */
		sControllerCommands();

		/* updates all internal arrays based on pressed percents */
		void UpdateCommands(float dTime, float fKeysPressedPercents[K_CM_COMMANDS_COUNT]);

		/* resets keypresses */
		void Reset();
	};

public:
	int						arrTriggersCnt;
	CControllerTrigger		arrTriggers[K_CM_MAX_TRIGGERS]; //array de triggers
	CStringHash				strName;						//controller name
public:
	//SDL data
	int						nSDLidx;						//index controller 
	int						nSDLInstanceId;					//instance id folosit de SDL
	SDL_GameController		*SDLpgc;						//pointer la controller
public:
	EControllerType			eType;							//tip controller
	sControllerCommands		sCommands;						//controller commands (to be used ingame)
	int						nFlags;							//flags needed sometimes
																	
//internal data, don't use
private:
	bool					bKeyDown[K_CM_COMMANDS_COUNT];			//daca e apasat sau nu
	float					fKeyDownPercent[K_CM_COMMANDS_COUNT];	//cat de apasat e (merge si pt axe)
	float					fTimeSinceKeypress;						//cat timp de la ultima apasare

#if defined(ENABLE_CHEATS)		
	CCircularStack<EControllerCommand, 10>	arrStackHistory;		//istoric comenzi
#endif

public:
	CController();

	/* Updates internal commands property using private fKeydownPercent array */
	void UpdateCommands(float dTime);

	/*
	* Adds a trigger for a specific command
	* param: fTriggerMin si fTriggerMax se seteaza sortate pe axa (negative, min va fi -1.1f iar max va fi mai spre 0.0f)
	*/
	void AddTrigger(EControllerTriggerType neType, EControllerCommand neCommand, int nKeyMapping, float nfTriggerMin = 0.1f, float nfTriggerMax = 1.1f);

	/*!
	 *	Returns the first key mapping for a specified command or -1 if command isn't mapped
	 *	TODO: it should return all triggers
	 */
	int GetKeyMappingForCommand(EControllerCommand neCommand);

	/*!
	*	Returns the first trigger for a specified command or null if command isn't mapped
	*/
	CControllerTrigger* GetTriggerForCommand(EControllerCommand neCommand);

	/*!
	 *	Removes all triggers for a specified command
	 */
	void RemoveTriggers(EControllerCommand neCommand);

	/*!
	 *	Removes trigger by key mapping
	 */
	void RemoveTriggerByKeyMapping(int nKeyMapping);

	/*!
	 * Clears all triggers
	 */
	void ClearTriggers();

	/*!
	 * \brief Translates triggers to commands. Handles all triggers before setting command On or Off
	 * Must be called after reading the input. Very important when using analog and digital triggers on the same command so they do not cancel each other out
	 */
	void TranslateTriggersToCommands();

	/*!
	 * \brief Resets all keypresses
	 */
	void ResetKeypresses();

	/* gets all keys pressed percentages into the destination array */
	void GetKeysDownPercents(float arrDest[K_CM_COMMANDS_COUNT]);

	// Tells if a button was pressed on the controller (or a stick too)
	bool WasControllerTouched(bool bSticksToo = false);
};

///--- controllers manager ---

class CControllersManager
{
protected:
	int arrControllerTypesCnt[K_CM_CONTROLLERTYPES_CNT];  //aici se scrie cate controale din fiecare tip avem alocate
public:
	CGrowableArray<CController*> m_arrControllers;
	//CTOR/DTOR
	CControllersManager();
	~CControllersManager();

	CController* AddController(EControllerType neType, WCHAR * strName);
	//finds all connected controllers (keyboard and joysticks) - se cheama la inceputul jocului
	int RegisterAllSDLControllers(); 
	//deallocates all SDL controllers
	void ReleaseAllControllers(bool bOnlySDL = false);
	///--- SDL methods ---
	
	//get scancode name. Shorten default SDL names before.
	const char* GetSDLScancodeName(SDL_Scancode scancode);
	//adauga un controller SDL dupa idx-ul acestuia
	void AddSDLController(int SDL_ctrlr_idx);
	//sterge un controller SDL dupa instanceID
	bool RemoveSDLController(int nnInstanceID);
	//callback SDL buttons
	void OnSDLControllerButton(const SDL_ControllerButtonEvent sdlEvent);
	//callback SDL axis
	void OnSDLControllerAxis(const SDL_ControllerAxisEvent sdlEvent);
	//callback SDL keys
	void OnSDLKeypress(const SDL_KeyboardEvent sdlEvent, bool bKeyDown);
	//get pointer to Controller by SDLInstanceID
	CController* GetControllerByInstanceID(int nnInstanceID);
	//get pointer to Controller by name
	CController* GetControllerByName(WCHAR* strControllerName);
	//Reseteaza apasarile de taste pe NOT PRESSED
	void ResetKeypresses(CController* ctrlr);
	//Reset keypresses on all controllers
	void ResetAllControllersKeypresses();
	//metoda folosita pentru a trimite mesajele de Windows pentru K_CM_CONTROLLER_KEYBOARD_WIN
	void ReceiveKeypress(UINT nChar, bool bIsKeyDown, bool bAltDown);
	/*!
	 *	Tells you if any key is pressed on any controller
	 */
	bool KeyPressed(EControllerCommand eCommandFilter = K_CM_COMMAND_NONE);
};


//declar singletonul
CControllersManager& UTGetControllersManager();