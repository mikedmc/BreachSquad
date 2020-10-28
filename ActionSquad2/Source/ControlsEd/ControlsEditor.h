#pragma once

#define K_GL_BUTTON_HIDE_BBOX 0
#define K_GL_BUTTON_VCENTER 1
#define K_GL_BUTTON_HCENTER 2
#define K_GL_BUTTON_MOVEUP 3
#define K_GL_BUTTON_MOVEDOWN 4
#define K_GL_BUTTON_SAVE 5

// controalele din controls panel
#define K_CP_CONTROLS_LISTBOX 0

// controalele din layers panel
#define K_LP_LAYERS_LISTBOX 0
#define K_LP_BUTTON_NEW_LAYER 1
#define K_LP_BUTTON_CLONE_LAYER 2

// controalele din properties panel
#define K_PP_BUTTON_CLONE_CONTROL 0
#define K_PP_CONTROLS_LIST 1 // lista de controale ale layer-ului selectat
#define K_PP_CONTROLS_PROPS_START 10
#define K_PP_CONTROLS_PROPS_END 100


enum ToolType
{
	TOOL_TYPE_NO_TOOL = 0,
	TOOL_TYPE_RESIZE_TOP_LEFT = 1,
	TOOL_TYPE_RESIZE_TOP_MID,
	TOOL_TYPE_RESIZE_TOP_RIGHT,
	TOOL_TYPE_RESIZE_BOTT_RIGHT,
	TOOL_TYPE_RESIZE_BOTT_MID,
	TOOL_TYPE_RESIZE_BOTT_LEFT,
	TOOL_TYPE_RESIZE_LEFT_MID,
	TOOL_TYPE_RESIZE_RIGHT_MID,
	TOOL_TYPE_MOVE
};

class CControlsEditor
{
private:
	CCameraTransform*	m_pCamera;
	IDirect3DDevice9*	m_pd3dDevice;
	ID3DXSprite*		m_pSprite;
public:
	bool				hideBBoxes;
	CDXUTDialog			propertiesPanel;

	CGrowableArray<CVariantCollection*>		ctrlTemplates;
	CVariantCollection						layerTemplate;

	CCtrlLayer*			currLayer;		// current layer
	int					currLayerIdx;	// current layer idx in list
	CCtrlLayer*			testLayer;
	CGrowableArray<int>	selectedCtrls;	// lista de controale selectate (pt selectie multipla)
	int					currCtrlIdx;	// tine ultimul control pe care am apasat

	int tool; // moving / resizing controls
	bool clickedInterface;
	CGrowableArray<int>	clickedCtrls;

	// offset-ul layer-ului fata de centrul ecranului
	D3DXVECTOR2 offset;

	// Loads everything it needs
	void Launch();
	// Deallocates everything
	void Close();
	void Update(float dTime);
	void UpdateCtrlParamsList(); // asta e update-ul scrisului din panel pt atunci cand misti un control cu mouse-u sau din sageti
	void Paint();
	void PaintBBoxes();
	void PaintInterface(float fElapsedTime);
	
	//--- IMGUI paint all
	void PaintImguiInterfaces();
	//--- IMGUI adds controls specific to selected control
	void IMGUI_AddCurControlProps();

	void DrawBBox(RECTXYWH rect, bool selected);
	void DrawLine(int x1, int y1, int x2, int y2, D3DCOLOR col = 0xff0000ff);

	void SetCameraTransform(CCameraTransform* pCamera);

	HRESULT LoadCtrlTemplatesXML(WCHAR* XMLpath);
	// incarca lista de controale a unui layer in panel
	void FillLayerControlsList(int	layIdx);
	// incarca lista de proprietati a unui control in panel
	void FillControlProperties();
	// incarca lista de proprietati a layer-ului selectat
	void FillLayerProperties();

	void AddControl(CVariantCollection* vcol);
	void CloneControl();
	void DeleteControl();
	void DeleteLayer();
	// changes paint order of selected control
	void ChangeControlPaintOrder(int dir);
	// centers selected elements or selected layer if no elements selected
	void CenterElements(bool H, bool V);

	void ReceiveKeys(UINT key);

	void SaveXML(WCHAR* XMLpath);

	CControlsEditor();
	~CControlsEditor();

	void PropertiesPanelCallBack(UINT nEvent, int nControlID, CDXUTControl* pControl);

	void SetSpritePtr(ID3DXSprite* pSprite);

	HRESULT OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnLostDevice();
	HRESULT OnDestroyDevice();
};

