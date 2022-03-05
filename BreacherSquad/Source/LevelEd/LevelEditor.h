#pragma once

#define K_LED_CAMSPEED				256.0f

enum eLvlEdTool {
	K_LED_TILE = 0,		
	// selectable objects from here
	K_LED_LIGHT = 1,		// lights, go figure
	K_LED_PROP,				// all other objects, static or active
	K_LED_ACTOR,			// players and enemies
	K_LED_COLBOX,			// collision boxes

	K_LED_TOOLS_CNT,
};

// index of first tool that allows selection
#define K_LED_TOOL_SELECTABLES_START	K_LED_LIGHT

class CLevelEditor
{
private:
	PDEVICE				m_pDevice;
	CLevel*				m_pLevel;

	CSpriteLib	m_sprCol;				// Sprite collection to hold editor only graphics

	eLvlEdTool			eTool;					// Current tool
	IActiveInterface*	pSelected;				// Selected item

	Vec2				m_vCamPos_ini;			// saved initial camera position (HOME)
	double				fTimeline;				// used for some animations
public: 
	Vec2				m_vCamPos;				// Camera position
	CCameraTransform*	m_pCam;					// pointer to level camera to screen

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
	void				Paint(ID3DXSprite* pSpr);
	
	inline bool			IsLaunched() {
		return (m_pLevel != nullptr);
	}

	void				ReceiveKeys(UINT key);
	OPRESULT			SaveLevel(WCHAR* strPath);

	//--- IMGUI paint all interfaces
	void				IMGUI_ShowInterfaces();

	// selects closest active depending on selected tool
	IActiveInterface*	SelectClosest(Vec2 vPoint, float fMaxRadius = 32.0f);
private:
	//--- IMGUI adds controls specific to selected light
	void				IMGUI_AddLightProps(CLight* light);
	
	// draws a ruler to show you the height of an object
	void				DrawHRuler(Vec2 vBase, float fHeight, DWORD col);
	void				DrawBBox(RectXYWH bbox, DWORD dwCol);


public: //--- framework methods ---
	OPRESULT OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL);
	OPRESULT OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = NULL);
	OPRESULT OnLostDevice();
	OPRESULT OnDestroyDevice();
};

