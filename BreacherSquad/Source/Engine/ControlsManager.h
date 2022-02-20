#pragma once
#include "dxstdafx.h"
#include "interfaces/DeviceRes.h"

#define INTERFACES_VERSION 1.0f

enum ECtrlMgrInputType {
	K_CCTRLMGR_INPUT_KEY = 0,		//comanda key din handler tastatura WIN
	K_CCTRLMGR_INPUT_CHAR = 1,		//comanda char din windows messages
	K_CCTRLMGR_INPUT_SDL_KEY,		//tasta SDL
	K_CCTRLMGR_INPUT_COMMAND,		//comanda generica din ECtrlMgrCommandType, de la orice controller (include SDL)
};

// commands that the manager listens to
enum ECtrlMgrCommandType {
	K_CCTRLMGR_COMMAND_NONE = -1,

	K_CCTRLMGR_COMMAND_UP = 0,
	K_CCTRLMGR_COMMAND_DOWN,
	K_CCTRLMGR_COMMAND_LEFT,
	K_CCTRLMGR_COMMAND_RIGHT,
	K_CCTRLMGR_COMMAND_SELECT,
	K_CCTRLMGR_COMMAND_BACK,
	K_CCTRLMGR_COMMAND_NEXT,
};

// layers aligning
enum ECtrlAnchor {
	K_CCTRL_LAYER_ANCHOR_MIN = -1,
	K_CCTRL_LAYER_ANCHOR_CENTER = 0,
	K_CCTRL_LAYER_ANCHOR_MAX = 1
};

// controls messages
enum CCTRL_MESSAGES {
	CCTRL_MESSAGE_CLICK = 1,
	CCTRL_MESSAGE_SLIDERCHANGED = 2, //param1 - volum intre 0 si 1000, param2 - 1000
	CCTRL_MESSAGE_CHECKCHANGED,		 
	CCTRL_MESSAGE_SELECTIONCHANGED,  //param1 - new selection, param2 - old selection
};


#define CCTRL_STATUS_FLAG_HOVER			1
#define CCTRL_STATUS_FLAG_CLICKED		2
#define CCTRL_STATUS_FLAG_HOVERLEFT		4
#define CCTRL_STATUS_FLAG_HOVERUP		8
#define CCTRL_STATUS_FLAG_HOVERRIGHT	16
#define CCTRL_STATUS_FLAG_HOVERDOWN		32
#define CCTRL_STATUS_FLAG_CLICKEDLEFT	64
#define CCTRL_STATUS_FLAG_CLICKEDUP		128
#define CCTRL_STATUS_FLAG_CLICKEDRIGHT	256
#define CCTRL_STATUS_FLAG_CLICKEDDOWN	512

// is it focused?
#define CCTRL_STATUS_FLAG_HAS_FOCUS		1024
// alternative click command
#define CCTRL_STATUS_FLAG_CLICKED_ALT	2048
// layer statuses
#define CCTRL_STATUS_FLAG_REMOVED	4096
#define CCTRL_STATUS_FLAG_FORCED	8192
#define CCTRL_STATUS_FLAG_REMOVEDFORCED	12288

// declares helper classes and enums
class CControlsManager;
class CCtrlLayer;
enum EControlType;

namespace GUIUtils {
	// colors for different elements
	const DWORD colPanelIdle = 0xff151515;
	const DWORD colPanelFocused = 0xff323232;
	const DWORD colPanelIconIdle = 0xff606e1f;
	const DWORD colPanelIconFocused = 0xff85962e;

	void DrawWindowFrameF( CSpriteCollection *sprCol, int animIdx, RectXYWH BBox, DWORD color = 0xffffffff, float fInflate = 0.0f );
	void DrawWindowFrame( CSpriteCollection *sprCol, int animIdx, RectXYWHi BBox, DWORD color = 0xffffffff, int nInflate = 0 );
	void DrawWindow( CSpriteCollection *sprCol, int animIdx, RectXYWHi BBox, float alpha, int nFontIdx, CStringDesc* strTitle, DWORD dwTitleColor = 0xffffffff );
	void DrawWindow( CSpriteCollection *sprCol, int animIdx, RectXYWHi BBox, float alpha, int nFontIdx, int nStrIdxTitle, DWORD dwTitleColor = 0xffffffff );
	// draws a container panel used for all selectable controls
	void DrawPanel( CSpriteCollection *sprCol, RectXYWHi BBox, float fFocusPercent, float fAlpha, int nIconAnimIdx = -1, int nIconFrame = -1 );
	// Draws a progress bar from animation with specific frames, without buttons
	void DrawProgress( CSpriteCollection *sprCol, int animIdx, RectXYWHi BBox, float fPercentFull, float fFocus, float fAlpha = 1.0f, int nSteps = 0 );
	// Paints a smaller panel without icon, with just the focus line on the left
	void DrawPanelSM( CSpriteCollection *sprCol, RectXYWHi BBox, float fFocusPercent, float fAlpha = 1.0f );
	// Draws a button centered on the specified bbox
	void DrawButton(CSpriteCollection *sprCol, int animIdx, RectXYWHi BBox, bool bPressed, float fHoverPercent, float fFocusPercent, float fAlpha = 1.0f);
	/*
	* \brief Deseneaza un buton (sau input box, slider, etc) dintr-o animatie cu 3 frames (capat, centru tiling, capat)
	* capetele butonului se deseneaza in interiorul bboxului. Capatul stanga trebuie aliniat in dreapta axei verticale in editor.
	* \param nStartFrame - frame-ul de la care incep cele 3 frames utile (daca butonul are mai multe stari in aceeasi anim, cum e si normal)
	* \param strDesc - stringul pe care il masoara pentru a lua dimensiunea butonului
	* \param nAlignHsign - 0 center, -1 left, 1 right
	*/
	void DrawButtonFromText( CSpriteCollection *sprCol, int animIdx, bool bPressed, CStringDesc *strDesc, CTexFont* pFont, Vec2 vButCenter, DWORD color = 0xffffffff, int nAlignHsign = 0 );
	/*
	* \brief Deseneaza un buton (sau input box, slider, etc) dintr-o animatie cu 3 frames (capat, centru tiling, capat)
	* capetele butonului se deseneaza in interiorul bboxului. Capatul stanga trebuie aliniat in dreapta axei verticale in editor.
	* \param nStartFrame - frame-ul de la care incep cele 3 frames utile (daca butonul are mai multe stari in aceeasi anim, cum e si normal)
	*/
	void DrawHTilingAnim( CSpriteCollection *sprCol, int animIdx, int nStartFrame, RectXYWHi BBox, DWORD color = 0xffffffff );
	/*
	* \brief Deseneaza un buton (sau input box, slider, etc) dintr-o animatie cu 3 frames (capat, centru tiling, capat)
	* capetele butonului se deseneaza in EXTERIORUL bboxului. Capatul stanga trebuie aliniat in stanga axei verticale in editor.
	* \param nStartFrame - frame-ul de la care incep cele 3 frames utile (daca butonul are mai multe stari in aceeasi anim, cum e si normal)
	*/
	void DrawHTilingAnim_HeadsOutside( CSpriteCollection *sprCol, int animIdx, int nStartFrame, RectXYWHi BBox, DWORD color = 0xffffffff );
	/*
	* Draws a progress bar from animation with 4 frames (left, tiling center, right, filler) - heads outside the BBox
	*/
	void DrawProgress_HeadsOutside( CSpriteCollection *sprCol, int animIdx, RectXYWHi BBox, float fPercentFull, DWORD color = 0xffffffff, int nTicks = 0 );
	/*
	* Draws a page selector control (colored dots)
	* \param nAlign - 0 centered, -1 left, 1 right
	*/
	void DrawPageSelector( CSpriteCollection *sprCol, int animIdx, RectXYWHi BBox, int nPagesCnt, int nSelectedPage, DWORD color = 0xffffffff, int nAlign = 0 );
}

class CControl
{
//protected:
	static CSpriteCollection *m_pSprCol;		// Pointer to sprite collection from the manager
public:
	// Sets useful static data (used by all controls)
	static void			SetManagersPtr(CSpriteCollection* sprCol);

	EControlType		type;				// Type of control CCTRL_TYPE_...
	CVariantCollection	paramsDict;			// List of controls params. Don't set directly! Use SetParamValue
	CCtrlLayer*			layer;				// Pointer to parent layer
	RectXYWHi			bbox;				// Don't set directly! Use SetParamValue(x,y,z,w)
	UINT32				statusFlags;		// CCTRL_STATUS_FLAG_...
	float				fDisabledPercent;	// disabled controls can have a disabling animation
	bool				bDisabled;
	bool				bVisible;
	bool				bCanHaveFocus;		// can control hold focus?
	bool				bShowFocusCursor;	// focus for keyboard input and paiting of focus
	float				fFocusPercent;		// visual 0..1 for focus painting. Updated by the layer and not by the control itself

	CControl( const WCHAR* typeName );
	CControl( CControl* ctrl );
	~CControl();

	// Called by manager when giving focus to current control
	// \param: nFocusDirection - specifies direction from last focused control (1-coming from above, -1-coming from below, 0-unknown)
	void				OnFocused(int nFocusDirection = 0);
	// Initializes specific vars based on control type
	void				Reset(); 
	void				Update(float dTime, float fTimeline);
	void				Paint(CCameraTransform	* pCamera, Mat * matWorld);
	//Initializes the control clone after parameters get set
	void				Initialize();	
	// Handles input commands
	bool				HandleCommand(ECtrlMgrCommandType cmd, int nSDLinstanceID = -1);
	// returns control bbox
	RectXYWHi			GetBBox();
	// (only for debug to show errors)
	void				drawDebugText(int x, int y, const wchar_t* text, DWORD color = 0xffff8888);
};

//**********************************************************
// Controls LAYER
//**********************************************************
class CCtrlLayer
{
public:
	CStringHash			ID;
	float				alpha;
	int					X, Y;

	ECtrlAnchor			anchorX, anchorY;
	int					nFocusFirstFocusableIdx;	// First focusable control so we know where to put the focus when creating the layer
	int					nFocusedControlIdx;			// Controls index that receives the keyboard input
	CStringHash			shFocusedControlID;			// hash ID of focused control

	POINT				mouseRelPos;				// local pointer position
	bool				bAnimate;					// has show/hide animation
	bool				bBlocking;
	bool				bGetsInput;					// expects user input
	float				fDestroyTimer;				// gets destroyed after the timer expires (for popups usually)
	UINT32				statusFlags;				// remove flags; !=0 to deallocate it after controls updating finished; 1-removed normal, 2-removed forced

	CControlsManager*	pControlsManager;			// pointer to parent controls manager
	CArray<CControl*>	controls;

	CCtrlLayer();
	~CCtrlLayer();

	void				FocusInitialize();
	// Focus next control down the list
	void				FocusNextControl();
	// Focus next control up the list
	void				FocusPreviousControl();
	bool				FocusControl( CControl* pCtrl );
	void				SetAnchor( ECtrlAnchor nAnchorX, ECtrlAnchor nAnchorY );
	void				SetPos( int nX, int nY ) { X = nX; Y = nY; }
	void				MoveLayer( int dX, int dY ) { X += dX; Y += dY; }
	Vec2i				GetPos();
	CControl*			GetControlByIdx( int nIdx );
	CControl*			GetControlByName( char* ctrlName );
	bool				ControlSetDisableByName( bool bDisabledValue, char* ctrlName );
	// Clones a control returning a pointer to the cloned control
	CCtrlLayer*			Clone();
};


//**********************************************************
// CONTROLS MANAGER
//**********************************************************
class CControlsManager : public IDeviceRes
{
private:
	bool bLoaded;

public:
	float				fLocalTimeline;				// Local timeline
	CCameraTransform*	m_pCamera;					// Camera used for painting
	RectXYWH			m_cameraScreenRect;			// Current camera screen rectangle
	CArray<CCtrlLayer*> layersDefinitions;			// Contains layer definitions
	CArray<CCtrlLayer*> Layers;						// Contains actual cloned layers
	CSpriteCollection	m_sprCol;					// Sprite collection for controls sprites
	bool				bIsBlocking;				// Does it block user input?

	CControlsManager();
	~CControlsManager();

	// Loads controls templates (loads sprite file name too)
	OPRESULT			LoadControlsXML( WCHAR* XMLpath );
	// Sets currently used camera
	void				SetCameraTransform(CCameraTransform* pCamera);
	// Sets a control parameter value and automatically converts to necessary value
	void				SetParamValue(CControl * pCtrl, const WCHAR * sParamName, WCHAR * sParamValue, bool bIgnoreWarnings = false);
	// Releases everything
	void				Release();

	//CLayer* addLayer(const int spec[], int specSize, bool blocking=true, bool getsInput=true, bool fadeIn=true, bool fadeOut=true, bool hasTimer=false, float timerValue=5.0f, bool slideIn = false, bool slideOut = false, float slideOffsetX = 0.0f, float slideOffsetY = -300.0f);
	CCtrlLayer*			GetLayerByNameHash(UINT32 layerNameHash);
	CCtrlLayer*			GetLayerByName(CHAR* layerName);
	CCtrlLayer*			GetLayerByIdx(int layerIdx);
	// Returns pointer to layer that is closest to player
	CCtrlLayer*			GetTopmostLayer();
	// Gets the topmost layer that receives input
	CCtrlLayer*			GetTopmostInputLayer();
	void				RemoveLayer(UINT32 layerID, bool forced = false);
	void				RemoveLayer(CHAR* layerName, bool forced = false);
	void				RemoveTopmostLayer(bool forced = false);
	// Creates one layer and returns pointer to it
	CCtrlLayer*			ShowLayer(CHAR* layerName, float fAlpha = 0.0f, int posX = 0, int posY = 0);
	// Creates one layer and returns pointer to it (checking that it's not already showing)
	CCtrlLayer*			ShowLayerOnce(CHAR* layerName, float fAlpha = 0.0f, int posX = 0, int posY = 0);
	// forced == true avoids animations
	void				RemoveAllLayers(bool forced = false);

	void				Update(float dTime);
	void				Paint();	

	// Receives input commands from different forms of input
	// nCommandParam is anything (eg: SDLinstanceID when processing controller input)
	void				ReceiveInput(ECtrlMgrInputType eCommandType, UINT32 nCommand, int nCommandParam = -1);

	// Shows a message box with a OK button
	void				MessageBoxOK(int titleStringId, int textStringId);

	// Inherited via IDeviceRes
	virtual OPRESULT OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc = nullptr ) override;
	virtual OPRESULT OnResetDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc = nullptr ) override;
	virtual OPRESULT OnLostDevice() override;
	virtual OPRESULT OnDestroyDevice() override;
};

//declare singleton
CControlsManager& UTGetGUI();