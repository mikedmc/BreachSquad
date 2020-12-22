#pragma once
#include "dxstdafx.h"

#define INTERFACES_VERSION 1.0f

//tratare input
enum ECtrlMgrInputType {
	K_CCTRLMGR_INPUT_KEY = 0,		//comanda key din handler tastatura WIN
	K_CCTRLMGR_INPUT_CHAR = 1,		//comanda char din windows messages
	K_CCTRLMGR_INPUT_SDL_KEY,		//tasta SDL
	K_CCTRLMGR_INPUT_COMMAND,		//comanda generica din ECtrlMgrCommandType, de la orice controller (include SDL)
};

//tipurile de comenzi ce pot fi primite de catre manager
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

//aliniere ancore layer
enum ECtrlAnchor {
	K_CCTRL_LAYER_ANCHOR_MIN = -1,
	K_CCTRL_LAYER_ANCHOR_CENTER = 0,
	K_CCTRL_LAYER_ANCHOR_MAX = 1
};

//tipuri mesaje trimise pe pipeline
enum CCTRL_MESSAGES {
	CCTRL_MESSAGE_CLICK = 1,
	CCTRL_MESSAGE_SLIDERCHANGED = 2, //param1 - volum intre 0 si 1000, param2 - 1000
	CCTRL_MESSAGE_CHECKCHANGED,		 
	CCTRL_MESSAGE_SELECTIONCHANGED,  //param1 - new selection, param2 - old selection
};


//flaguri
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

//is it focused?
#define CCTRL_STATUS_FLAG_HAS_FOCUS		1024
//click alternativ din taste - folosit la unele controale
#define CCTRL_STATUS_FLAG_CLICKED_ALT	2048
//in special pt layere
#define CCTRL_STATUS_FLAG_REMOVED	4096
#define CCTRL_STATUS_FLAG_FORCED	8192
#define CCTRL_STATUS_FLAG_REMOVEDFORCED	12288

//declare clasa layer si manager controale
class CControlsManager;
class CCtrlLayer;
enum EControlType;

///--- functii particulare de desenare ---
//deseneaza un frame dintr-o animatie cu 9 elemente (colturi, laterale, centru)
void CtrlMgrDrawFrameF(CSpriteCollection *sprCol, int animIdx, RECTXYWH_F BBox, DWORD color = 0xffffffff, float fInflate = 0.0f);
void CtrlMgrDrawFrame(CSpriteCollection *sprCol, int animIdx, RECTXYWH BBox, DWORD color = 0xffffffff, int nInflate = 0);
void CtrlMgrDrawWindow(CSpriteCollection *sprCol, int animIdx, RECTXYWH BBox, DWORD color, int nFontIdx, CStringDesc* strTitle, DWORD dwTitleColor = 0xffffffff);
void CtrlMgrDrawWindow(CSpriteCollection *sprCol, int animIdx, RECTXYWH BBox, DWORD color, int nFontIdx, int nStrIdxTitle, DWORD dwTitleColor = 0xffffffff);
void CtrlMgrDrawWidebar(CSpriteCollection *sprCol, int animIdx, RECTXYWH BBox, DWORD color);

/*
* \brief Deseneaza un buton (sau input box, slider, etc) dintr-o animatie cu 3 frames (capat, centru tiling, capat)
* capetele butonului se deseneaza in interiorul bboxului. Capatul stanga trebuie aliniat in dreapta axei verticale in editor.
* \param nStartFrame - frame-ul de la care incep cele 3 frames utile (daca butonul are mai multe stari in aceeasi anim, cum e si normal)
* \param strDesc - stringul pe care il masoara pentru a lua dimensiunea butonului
* \param nAlignHsign - 0 center, -1 left, 1 right
*/
void CtrlMgrDrawButtonFromText(CSpriteCollection *sprCol, int animIdx, bool bPressed, CStringDesc *strDesc, CTexturedFont* pFont, D3DXVECTOR2 vButCenter, DWORD color = 0xffffffff, int nAlignHsign = 0);
/*
* \brief Deseneaza un buton (sau input box, slider, etc) dintr-o animatie cu 3 frames (capat, centru tiling, capat)
* capetele butonului se deseneaza in interiorul bboxului. Capatul stanga trebuie aliniat in dreapta axei verticale in editor.
* \param nStartFrame - frame-ul de la care incep cele 3 frames utile (daca butonul are mai multe stari in aceeasi anim, cum e si normal)
*/
void CtrlMgrDrawHTilingAnim(CSpriteCollection *sprCol, int animIdx, int nStartFrame, RECTXYWH BBox, DWORD color = 0xffffffff);
/*
* \brief Deseneaza un buton (sau input box, slider, etc) dintr-o animatie cu 3 frames (capat, centru tiling, capat)
* capetele butonului se deseneaza in EXTERIORUL bboxului. Capatul stanga trebuie aliniat in stanga axei verticale in editor.
* \param nStartFrame - frame-ul de la care incep cele 3 frames utile (daca butonul are mai multe stari in aceeasi anim, cum e si normal)
*/
void CtrlMgrDrawHTilingAnim_HeadsOutside(CSpriteCollection *sprCol, int animIdx, int nStartFrame, RECTXYWH BBox, DWORD color = 0xffffffff);
/*
* Draws a progress bar from animation with 4 frames (left, tiling center, right, filler)
*/
void CtrlMgrDrawProgress(CSpriteCollection *sprCol, int animIdx, RECTXYWH BBox, float fPercentFull, DWORD color = 0xffffffff, int nTicks = 0);
/*
* Draws a progress bar from animation with 4 frames (left, tiling center, right, filler) - heads outside the BBox
*/
void CtrlMgrDrawProgress_HeadsOutside(CSpriteCollection *sprCol, int animIdx, RECTXYWH BBox, float fPercentFull, DWORD color = 0xffffffff, int nTicks = 0);
/*
* Draws a page selector control (colored dots)
* \param nAlign - 0 centered, -1 left, 1 right
*/
void CtrlMgrDrawPageSelector(CSpriteCollection *sprCol, int animIdx, RECTXYWH BBox, int nPagesCnt, int nSelectedPage, DWORD color = 0xffffffff, int nAlign = 0);

class CControl
{
protected:
	static		CSpriteCollection *m_pSprMgr;
	static		CTexturedFontsManager *m_pFontsMgr;
public:
	//se cheama doar odata pt ca seteaza var statice
	static void	SetManagersPtr(CTexturedFontsManager* fontsManager, CSpriteCollection* sprCol);

	CVariantCollection paramsDict; //lista de params. Don't set directly! Use SetParamValue
	EControlType	type; //tipul controlului din CCTRL_TYPE enum
	CCtrlLayer		*layer;  //pointer catre parinte

	//generic data
	RECTXYWH	bbox;	//don't set directly! Use SetParamValue(x,y,z,w)
	UINT32 statusFlags;
	float fDisabledPercent;
	bool bDisabled, bVisible;
	bool bCanHaveFocus, bShowFocusCursor;	//focus for keyboard input and paiting of focus

	// Called by manager when giving focus to current control
	// \param: nFocusDirection - specifies direction from last focused control (1-coming from above, -1-coming from below, 0-unknown)
	void OnFocused(int nFocusDirection = 0);

	void Reset(); // se initializeaza variabilele care nu fac parte din controls (alea care ajuta la animatii)
	void Update(float dTime, float fTimeline);
	void Paint(CCameraTransform	* pCamera, D3DXMATRIXA16 * matWorld);
	//Initializes the control clone after parameters get set
	void Initialize();	
	/*!
	 *	Handles keyboard commands
	 */
	bool HandleCommand(ECtrlMgrCommandType cmd, int nSDLinstanceID = -1);

	RECTXYWH GetBBox();

	//obsolete (only for debug)
	void drawDebugText(int x, int y, const wchar_t* text, DWORD color = 0xffff8888);

	CControl(const WCHAR* typeName);
	CControl(CControl* ctrl);

	~CControl();
};

//**********************************************************
// Controls LAYER
//**********************************************************
class CCtrlLayer 
{
public:
	ECtrlAnchor anchorX, anchorY; //ancore layer
	int X, Y; //pozitia layerului - readonly

	int nFocusFirstFocusableIdx; //primul idx de control focusabil ca sa stiu unde desenez cursorul de focus
	int	nFocusedControlIdx; //controlul pe care este setat inputul din taste
	CStringHash shFocusedControlID; //hashul controlului default
	CAABB m_focusRect;		//dreptunghiul de focus
public:
	float alpha;
	POINT mouseRelPos; //pozitia relativa a mouse-ului pe layer

	CStringHash ID; 
	bool bAnimate;  //se animeaza cand apare si dispare
	bool bBlocking; //daca blocheaza jocul in spate
	bool bGetsInput; //daca asteapta input de la user
	float fDestroyTimer; //daca e diferit de 0 scade iar cand ajunge la 0 il dezaloca
	UINT32 statusFlags; //flaguri de removed; setat diferit de 0 ca sa il dezaloce dupa ce a terminat update-urile controalelor; 1-removed normal, 2-removed forced

	CCtrlLayer();
	~CCtrlLayer();

	CControlsManager*	pControlsManager; //pointer catre parent

	CGrowableArray<CControl*> controls;
	void FocusInitialize();
	//focus next control down the list
	void FocusNextControl();
	//focus next control up the list
	void FocusPreviousControl();
	bool FocusControl(CControl* pCtrl);

	void SetAnchor(ECtrlAnchor nAnchorX, ECtrlAnchor nAnchorY);
	void SetPos(int nX, int nY) {X = nX; Y = nY; }
	void MoveLayer(int dX, int dY) {X += dX; Y += dY; }
	POINTXY_INT GetPos();

	CControl* GetControlByIdx(int nIdx);
	CControl* GetControlByName(char* ctrlName);
	bool ControlSetDisableByName(bool bDisabledValue, char* ctrlName);

	CCtrlLayer* Clone();
};


//**********************************************************
// CONTROLS MANAGER
//**********************************************************
#define	CCTRL_TIP_WAITTIMER 1.0f

class CControlsManager
{
private:
	CStringsManager *m_pStrMgr;
	CTexturedFontsManager *m_pFontsMgr;
	bool loaded;

public:
	float fLocalTimeline;	//timeline local

	CCameraTransform	*m_pCamera; //transformul folosit pe toate controalele active
	RECTXYWH_F			m_cameraScreenRect; //dreptunghiul ecran al camerei curente

	WCHAR loadedFile[MAX_PATH];
	//contine definitiile layerelor
	CGrowableArray<CCtrlLayer*> layersDefinitions;
	//pointere catre definitii
	CGrowableArray<CCtrlLayer*> Layers; 
	LPDIRECT3DDEVICE9	m_pDevice;
	ID3DXSprite*		m_pSprite;

	CSpriteCollection m_sprCol; //aici se incarca controalele (ferestre/butoane)
	bool bIsBlocking; //daca blocheaza jocul in spate

	void SetManagersPtr(CStringsManager* strManager, CTexturedFontsManager *fontsManager);
	void SetSpritePtr(ID3DXSprite *pSprite);
	void SetCameraTransform(CCameraTransform* pCamera);

	CCtrlLayer* GetTopmostLayer();

	CControlsManager();
	~CControlsManager();

	//seteaza un parametru al unui control si face conversiile automat la fonturi, animatii, alinieri etc
	void SetParamValue(CControl * pCtrl, const WCHAR * sParamName, WCHAR * sParamValue, bool bIgnoreWarnings = false);

	HRESULT LoadControlsXML(WCHAR* XMLpath); //incarca si numele sprite-ului din XML
	void Release();

	//CLayer* addCinematicLayer(int dialogIdx);
	//CLayer* addLayer(const int spec[], int specSize, bool blocking=true, bool getsInput=true, bool fadeIn=true, bool fadeOut=true, bool hasTimer=false, float timerValue=5.0f, bool slideIn = false, bool slideOut = false, float slideOffsetX = 0.0f, float slideOffsetY = -300.0f);
	CCtrlLayer* GetLayerByNameHash(UINT32 layerNameHash);
	CCtrlLayer* GetLayerByName(CHAR* layerName);
	CCtrlLayer* GetLayerByIdx(int layerIdx);

	/*!
	 * \brief Gets the topmost layer that receives input
	 */
	CCtrlLayer* GetTopmostInputLayer();

	//CControl* getControlByID(int ID);
	void RemoveLayer(UINT32 layerID, bool forced = false);
	void RemoveLayer(CHAR* layerName, bool forced = false);
	void RemoveTopmostLayer(bool forced = false);
	CCtrlLayer* ShowLayer(CHAR* layerName, float fAlpha = 0.0f, int posX = 0, int posY = 0);
	//return layer idx
	CCtrlLayer* ShowLayerOnce(CHAR* layerName, float fAlpha = 0.0f, int posX = 0, int posY = 0);
	//cand forced == true le sterge direct, fara animatii
	void RemoveAllLayers(bool forced = false);

	void Update(float dTime);
	void Paint();	
	//bool FocusControl(CControl* pCtrl);

	//managerul primeste comenzi de la diverse forme de input
	//nCommandParam este orice, de exemplu SDLinstanceID la comenzile de controller
	void ReceiveInput(ECtrlMgrInputType eCommandType, UINT32 nCommand, int nCommandParam = -1);

	// afiseaza un mesaj
	void MessageBoxOK(int titleStringId, int textStringId);

	//paint helpers
	//deseneaza progress bar liniar
	void DrawProgressBar(D3DXVECTOR2 vPos, int nWidth, float fPercent, DWORD nColor = 0xffffffff);

	HRESULT OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnLostDevice(void);
	HRESULT OnDestroyDevice(void);
};

//declare singleton
CControlsManager& UTGetControlsManager();