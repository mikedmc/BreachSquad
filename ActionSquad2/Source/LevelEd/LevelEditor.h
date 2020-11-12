#pragma once


class CLevelEditor
{
private:
	PDEVICE				m_pDevice;
	ID3DXSprite*		m_pSprite;
	CLevel*				m_pLevel;

	CSpriteCollection	m_sprMgr;				// Sprite collection to hold editor only graphics

	IActiveInterface*	pSelected;				// Selected item

public:
	CCameraTransform	camMain;				// main camera of the level editor

public: 
	CLevelEditor();
	~CLevelEditor();

	// Loads everything it needs and sets pointer to sprites painter
	OPRESULT			Init();
	// Deallocates everything
	void				Release();

	// Launch it on a level to start editing	
	void				Launch(CLevel* level);
	// Closes the level editor
	void				Close();

	void				Update(float dTime);
	void				Paint();
	
	inline bool			IsLaunched() {
		return (m_pLevel != nullptr);
	}

	//--- IMGUI paint all
	void				IMGUI_ShowInterfaces();
	/*
	//--- IMGUI adds controls specific to selected control
	void IMGUI_AddCurControlProps();
	//--- IMGUI adds controls for current layer
	void IMGUI_AddLayerProps();
	*/

	void				DrawBBox(RECTXYWH rect, bool selected);
	void				DrawLine(int x1, int y1, int x2, int y2, D3DCOLOR col = 0xffffffff);

	void				CloneCamTransform(CCameraTransform* pSrcCamera);

	void				ReceiveKeys(UINT key);
	OPRESULT			SaveLevel(WCHAR* strPath);

	void				SetSpritePtr(ID3DXSprite* pSprite);

public: //--- framework methods ---
	OPRESULT OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL);
	OPRESULT OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL);
	OPRESULT OnLostDevice();
	OPRESULT OnDestroyDevice();
};

