#pragma once

enum EMM_State {
	K_MM_STATE_INVALID = -1, //invalid state

	K_MM_STATE_MAINMENU = 0,
	K_MM_STATE_GAME_MODE_SELECT = 1,
	K_MM_STATE_CHAPTER_SELECT,
	K_MM_STATE_LEVEL_SELECT,
	//online lobby (no logic, just paint the background)
	K_MM_STATE_NET_LOBBY,
	//mods selector screen
	K_MM_STATE_WORKSHOP,
	//selection of workshop downloaded levels
	K_MM_STATE_DOWNLOADED_LEVEL_SELECT,

	//count - last one
	K_MM_STATES_COUNT,
};

//enum necessary for mode selection (corresponds to anim)
enum EMM_GameModes {
	K_MM_MODE_WEEKLY_CHALLENGE = 0,
	K_MM_MODE_CLASSIC = 1,
	K_MM_MODE_ZOMBIE_INVASION,
	K_MM_MODE_VINFINITE,

	K_MM_MODES_CNT
};

/*!
 * \class CMainMenu
 *
 * \brief Handles hardcoded menu screens like main menu, mission and level select, credits etc
 *
 */
class CMainMenu {
private:
	EMM_State			m_eState;			//current state
	int					m_nSubstate;		//current sub state
//variabile folosite intern de catre stari
	int					m_nSelection;		//selectia curenta 
	int					m_nSelectionOld;	//vechea selectie folosita la nivele
	RECTXYWH_F			m_rectSel;			//dreptunghi selectie desenata
	RECTXYWH_F			m_rectSelTarget;	//dreptunghi spre care tinde selectia (real)
	int					m_nSelRows, m_nSelColumns, m_nSelElements; //cate randuri, coloane si elemente totale avem (pentru paginare)
	int					m_arrSelItems[256];	//used sometimes when we need special id-s in special order
	int					m_nSelPage, m_nSelPagesCnt;	//pagina curenta si numar total de pagini
	float				m_fSelPageCursor;	//float care se duce pe m_nSelPage pentru scroll pagini
	float				m_fSelTimer;		//timer used for selection
	bool				m_bSelectionMade;	//daca a selectat ce era de selectat
public:
	double				fLocalTimeline;
	LPDIRECT3DDEVICE9	m_pDevice;
	ID3DXSprite*		m_pSprite; //sprite painter class

	eGameState			m_eTargetGameState; //if set it changes to this state after selecting (used in game mode selection)

public: 
	CMainMenu();
	~CMainMenu();

	CSpriteCollection	m_sprCol;
	CSpriteCollection	m_sprColNew;
	HRESULT LoadSprites(WCHAR * strSpritePath, WCHAR * strSprPath);

	void SetSpritePtr(ID3DXSprite* pSprite);
	void SetState(EMM_State neState, int nArg1 = 0);

	bool RequestLeaderboardsUpdate(bool bCoop);

	void Update(float dTime);
	void Paint();

	// Paints the animated background
	void PaintBackground(RECTXYWH_F worldRect, DWORD dwColor, bool bPaintParticles = false);

	void PaintMainBackground(RECTXYWH_F worldRect, DWORD dwColor, bool bPaintParticles = false);
	void PaintChapterWindow(D3DXVECTOR2 vCenter, int nChapterIdx, DWORD dwColor);
	void PaintChapterWindowLarge(D3DXVECTOR2 vCenter, int nChapterIdx, float fAlpha);
	void PaintGameModeWindow(D3DXVECTOR2 vCenter, int nGameModeIdx, float fAlpha);

	void Release();

	HRESULT OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnLostDevice(void);
	HRESULT OnDestroyDevice(void);
};

