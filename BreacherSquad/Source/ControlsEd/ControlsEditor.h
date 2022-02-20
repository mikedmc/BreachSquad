#pragma once
#include "interfaces/DeviceRes.h"

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

class CControlsEditor : public IDeviceRes
{
private:
	CCameraTransform	camera;

public:
	bool				hideBBoxes;

	CArray<CVariantCollection*>		ctrlTemplates;
	CVariantCollection				layerTemplate;

	CCtrlLayer*			currLayer;			// current layer
	int					currLayerIdx;		// current layer idx in list
	CCtrlLayer*			testLayer;
	CArray<int>			selectedCtrls;		// list of selected controls
	int					currCtrlIdx;		// last selected control

	int					tool;				// moving / resizing controls
	bool				clickedInterface;
	CArray<int>			clickedCtrls;
	Vec2				offset;				// camera movement for panning

	CControlsEditor();
	~CControlsEditor();
	// Loads everything it needs
	void				Launch();
	// Deallocates everything
	void				Close();
	void				Update(float dTime);
	void				Paint();
	void				PaintBBoxes();
	
	//--- IMGUI paint all
	void				IMGUI_ShowInterfaces();
	//--- IMGUI adds controls specific to selected control
	void				IMGUI_AddCurControlProps();
	//--- IMGUI adds controls for current layer
	void				IMGUI_AddLayerProps();
	// Loads control templates
	OPRESULT			LoadCtrlTemplatesXML( WCHAR* XMLpath );
	void				DrawBBox(RectXYWHi rect, bool selected);
	void				DrawLine(int x1, int y1, int x2, int y2, D3DCOLOR col = 0xff0000ff);
	void				AddControl(CVariantCollection* vcol);
	void				CloneControl(int offx, int offy);
	void				DeleteControl();
	void				DeleteLayer();
	// Updates control's visual data from internal parameters
	void				UpdateControlDisplayProps(CControl* ctrl);
	// changes paint order of selected control
	void				ChangeControlPaintOrder(int dir);
	// centers selected elements or selected layer if no elements selected
	void				CenterElements(bool H, bool V);
	// receives keyboard input
	void				ReceiveKeys(UINT key);
	// saves edited controls in xml format
	void				SaveXML(WCHAR* XMLpath);

	// Inherited via IDeviceRes
	virtual OPRESULT OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc = nullptr ) override;
	virtual OPRESULT OnResetDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc = nullptr ) override;
	virtual OPRESULT OnLostDevice() override;
	virtual OPRESULT OnDestroyDevice() override;
};

