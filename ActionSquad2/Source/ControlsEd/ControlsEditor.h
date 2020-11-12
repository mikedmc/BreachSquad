#pragma once

enum eCtrlEdTool
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
	void Paint();
	void PaintBBoxes();
	
	//--- IMGUI paint all
	void IMGUI_ShowInterfaces();
	//--- IMGUI adds controls specific to selected control
	void IMGUI_AddCurControlProps();
	//--- IMGUI adds controls for current layer
	void IMGUI_AddLayerProps();

	void DrawBBox(RECTXYWH rect, bool selected);
	void DrawLine(int x1, int y1, int x2, int y2, D3DCOLOR col = 0xff0000ff);

	void SetCameraTransform(CCameraTransform* pCamera);

	HRESULT LoadCtrlTemplatesXML(WCHAR* XMLpath);

	void AddControl(CVariantCollection* vcol);
	void CloneControl(int offx, int offy);
	void DeleteControl();
	void DeleteLayer();
	// Updates control's visual data from internal parameters
	void UpdateControlDisplayProps(CControl* ctrl);
	// changes paint order of selected control
	void ChangeControlPaintOrder(int dir);
	// centers selected elements or selected layer if no elements selected
	void CenterElements(bool H, bool V);

	void ReceiveKeys(UINT key);

	void SaveXML(WCHAR* XMLpath);

	CControlsEditor();
	~CControlsEditor();

	void SetSpritePtr(ID3DXSprite* pSprite);

	HRESULT OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnLostDevice();
	HRESULT OnDestroyDevice();
};

