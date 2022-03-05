#pragma once

// declare 
enum eStrategicAbility;

//fixed coords
#define K_PSS_PLAYER_WINDOW_WIDTH 132
#define K_PSS_PLAYER_WINDOW_HEIGHT 216
//how much time it waits before saying the class selection verse
#define K_PSS_WAIT_BEFORE_VERSE_SEC 0.5f
//number of total upgrade bars per class
#define K_PSS_UPGRADE_BARS_CNT 5
//time to wait before showing hints
#define K_PSS_HINT_WAIT_TIMER 0.5f

enum EPSSInputCommand {
	K_PSS_COMMAND_NONE = -1,

	K_PSS_COMMAND_LEFT = 0,
	K_PSS_COMMAND_RIGHT,
	K_PSS_COMMAND_DOWN,
	K_PSS_COMMAND_UP,
	K_PSS_COMMAND_SELECT,
	K_PSS_COMMAND_BACK,
};
//type of player
enum EPSSPlayerClass {
	K_PSS_CLASS_NOT_SELECTED = -1,

	K_PSS_CLASS_ASSAULTER = 0,
	K_PSS_CLASS_BREACHER = 1,
	K_PSS_CLASS_SHIELD,
	K_PSS_CLASS_FBI_AGENT,
	K_PSS_CLASS_RECON,
	K_PSS_CLASS_OFFDUTYGUY,
	//numarul total de tipuri
	K_PSS_CLASSES_COUNT
};

//vertical cursor position (hardoced here for the menus)
enum EPSSCursorPosition {
	K_PSS_CURPOS_CLASS = 0,
	K_PSS_CURPOS_XP_POINTS = 1,
	K_PSS_CURPOS_WEAPON,
	K_PSS_CURPOS_EQUIPMENT,
	K_PSS_CURPOS_GEAR,
	K_PSS_CURPOS_ULTIMATE,
	K_PSS_CURPOS_READY,			//ready to go button
	//always alst
	K_PSS_CURPOS_CNT			//number of positions
};

//--- default templates for player classes ---
const CStringHash EPSSPlayerTypeTemplate[] = {
	L"ACTOR_PLAYER_ASSAULTER",
	L"ACTOR_PLAYER_BREACHER",
	L"ACTOR_PLAYER_SHIELD",
	L"ACTOR_PLAYER_FBI_AGENT",
	L"ACTOR_PLAYER_RECON",
	L"ACTOR_PLAYER_OFFDUTYGUY"
};
//names of types
const CStringHash EPSSPlayerClassNames[K_PSS_CLASSES_COUNT] =
{
	L"ASSAULTER",
	L"BREACHER",
	L"SHIELD",
	L"FBI_AGENT",
	L"RECON",
	L"OFFDUTYGUY"
};


//data structure that holds the items that can be selected with all necessary data for each one of them
struct sPSSItemData {
	CStringHash			shName;				//actual weapon/item selection name/ID. For weapons it corresponds to the weapon names from the weapons_data.xml. For others the name is not so important
	int					iconIdx;			//main icon (from different animations depending on type)
	int					strIdxScreenName;	//string idx for item screen name

	CStringHash			shALTweaponTemplate;	//alt weapon template name (if set)
	int					iconALTidx;				//alt weapon main icon
	int					strIdxALTscreenName;	//alt weapon string idx for item screen name

	int					nStatsData[4 * 2];			//max 4 stats (strIdx, percent0..100, strIdx etc). Painted when strIdx >= 0
	int					strIdxLongDescription;		//string idx for long description. If set it replaces the displayed stats.

	CStringHash			shModifierTemplate;		//if set it specifies the template that gets applied over the base actor template (upgrades)

	sPSSItemData() : iconIdx(0), strIdxScreenName(-1),
		iconALTidx(0), strIdxALTscreenName(-1),
		strIdxLongDescription(-1)
	{
		for (int kk = 0; kk < 4; kk++)
		{
			nStatsData[kk * 2] = -1;	//strIdx
			nStatsData[kk * 2 + 1] = 0;	//percent
		}
	}
};

//types of items that the player can select from
enum ePSSItemCategory {
	PSS_ITEMCAT_NOT_SET = -1,

	PSS_ITEMCAT_ULTIMATE = 0,
	PSS_ITEMCAT_WEAPON = 1,
	PSS_ITEMCAT_EQUIPMENT,
	PSS_ITEMCAT_GEAR,

	PSS_ITEMCATS_COUNT
};
//holds all items that a player can select from
struct ePSSPlayerOptions {
	EPSSPlayerClass					ePlayerType;
	CFixedArray<sPSSItemData, 16>	matOptionsByItemType[PSS_ITEMCATS_COUNT];
	int								arrUpgradeBarsIdx[K_PSS_UPGRADE_BARS_CNT]; //indices of upgrade bars in m_ArrUpgradeBars collection for this class
	//informational class items
	int								nStrIdx_desc, nStrIdx_difficulty;

	void Reset()
	{
		ePlayerType = K_PSS_CLASS_NOT_SELECTED;
		for (int kk = 0; kk < PSS_ITEMCATS_COUNT; kk++)
		{
			matOptionsByItemType[kk].Clear();
		}
		memset(arrUpgradeBarsIdx, 0, K_PSS_UPGRADE_BARS_CNT * sizeof(int));

		nStrIdx_desc = -1;
		nStrIdx_difficulty = -1;
	}

	ePSSPlayerOptions()
	{
		Reset();
	}
};

///--- PLAYER UPGRADES ---

struct CUpgradePerk {
	CStringHash		shUID;
	int				nPointPrice;	//when upgrading to this value it gets activated
	int				nIconIdx;		//icon index in animation
	int				nStrIdx_desc;	//description string index

	CUpgradePerk()
	{
		nPointPrice = -1;			//no price
		nIconIdx = -1;				//no icon
		nStrIdx_desc = -1;			//no desc
	}
};

#define K_PSS_UPGRADE_BAR_MAX_POINTS 40

//unique upgrade bars
struct CUpgradeBar {
	CStringHash		shUID;
	int				nTotalPoints;	//total bar length
	int				nMemSlot;		//memory slot where points get saved
	int				nStrIdx_name;		//bar name
	int				nStrIdx_desc;		//description shown when no perk is selected
	CFixedArray<CUpgradePerk, K_PSS_UPGRADE_BAR_MAX_POINTS> m_arrPerks;	//list of perks (limited to 50 points)

	CUpgradeBar()
	{
		nMemSlot = 0;
		shUID.Reset();
		nTotalPoints = 0;
		nStrIdx_name = -1;
		nStrIdx_desc = -1;
		m_arrPerks.Clear();
	}
};


class CPlayerSelScr {
public:
	double				fLocalTimeline;
	LPDIRECT3DDEVICE9	m_pDevice;
	//structura cu toate optiunile selectabile pentru fiecare player 
	struct CPlayerCharSelection 
	{
		int				nInstanceID;		//instance ID controller (-1 - not set)
		EPSSPlayerClass	eType;				//player type
		bool			bSelected;			//is selection final?
		bool			bIsNetworkPlayer;	//is player from network?

		int				nSelection[PSS_ITEMCATS_COUNT];	//current selection for each options wheel
		int				nPrice[PSS_ITEMCATS_COUNT];		//current price for each selection (wrote by network code sometimes)
		//cursor selectie optiuni
		int				nCursorPosReal;		//cursor vertical position (last one is READY BUTTON)
		float			fTimeSinceCursorMoved; //counts time since cursor moved
		CAABB			aabbCursor;			//selection cursor
		
		float			fVerseReadyTimer;	//timer READY voice on class selection

		int				nCursorMoreReal;	//(<0 means disabled) cursor for details window (Use together with nCursorPosReal)
		float			fAnimCursor;		//animation timer

		int				nPlayerXPPts;		//total player XP points synced
		int				arrUpgradeBarsPts[K_PSS_UPGRADE_BARS_CNT]; //points placed in each upgrade bar synced via network

		void Init(EPSSPlayerClass neType = K_PSS_CLASS_NOT_SELECTED)
		{
			fVerseReadyTimer = 0.0f;
			nCursorPosReal =  K_PSS_CURPOS_READY;
			aabbCursor.Set(0, 0, 0, 0); //unset it
			fTimeSinceCursorMoved = 0.0f;

			nInstanceID = -1;
			eType = neType;  
			bSelected = false;
			bIsNetworkPlayer = false;

			for (int kk = 0; kk < PSS_ITEMCATS_COUNT; kk++)
			{
				nSelection[kk] = 0;
				nPrice[kk] = 0;
			}
			//reset upgrades
			nPlayerXPPts = 0;
			memset(arrUpgradeBarsPts, 0, K_PSS_UPGRADE_BARS_CNT * sizeof(int));

			nCursorMoreReal = -1;
			fAnimCursor = 0.0f;
		}

		CPlayerCharSelection()
		{
			Init();
		}
	};

public: 
	CPlayerCharSelection	m_arrPlayers[K_MAX_PLAYERS_CNT];	//contains player selection data	
	int						m_nPlayersCnt;						//number of actual players

	CArray<CUpgradeBar*>	m_arrUpgradeBars;				//Upgrades library (holds all unique bars)
	//gets the index of an upgrade bar by its name string hash
	int						GetUpgradeBarIdx(const WCHAR* sBarName);

	CPlayerSelScr();
	~CPlayerSelScr();

	ePSSPlayerOptions		arrItemsByClass[K_PSS_CLASSES_COUNT];	//all player selections by player class
	FORCEINLINE int			GetItemsCount(EPSSPlayerClass ePlayerClass, ePSSItemCategory eItemsCategory);
	sPSSItemData*			GetItem(EPSSPlayerClass ePlayerClass, ePSSItemCategory eItemsCategory, int nIndex);

	// Resets previous player selection
	void					ResetSelection(bool bResetInstanceIDs);	
	// handles controller removed events
	void					OnControllerRemoved(int ctrlrInstanceID);

	// Saves the current panel/class selections 
	void					SaveSelection();
	// Loads player selection from local repo for specified ordinal (or panel)
	void					LoadSelectionForPanel(int nPlayerOrdinal);
	
	CSpriteLib		m_sprCol;
	HRESULT					InitSprites(WCHAR * strPath);
	void					ReleaseSprites();
	
	// sends current selection to network peer 
	bool					SendSelectionByNetwork(int nPlayerOrdinal);
	
	// Paints the player selection window for a single player
	// @nPlayerOrdinal - the number of the player to be painted
	void					PaintPlayerSelectionWindow(int nPlayerOrdinal, D3DXVECTOR2 pos, float fAlpha = 1.0f, CStringHash * sPlayerName = null);

	//paints the details window for all selections and upgrade options
	void					PaintDetailsWindow(int nPlayerOrdinal, D3DXVECTOR2 pos, float fAlpha = 1.0f);

	void					Update(float dTime);
	void					Paint(ID3DXSprite* pSprite);

	// Loads all necessary items from gear_screen.xml
	HRESULT					LoadItems();
	// releases items from gear screen
	void					ReleaseItems();


	// Gets selected primary weapon name hash 
	UINT32					GetPrimaryWeaponNameHash(CPlayerCharSelection* pSel);
	// Gets selected primary weapon ALT FIRE name hash or secondary weapon ALT FIRE (if primary is empty) 
	UINT32					GetALTWeaponNameHash(CPlayerCharSelection* pSel);
	// Gets the template (name hash) that the current equipment overwrites over the actor base template 
	UINT32					GetEquipmentTemplateModifierHash(CPlayerCharSelection* pSel);
	// Gets selected gear name hash 
	UINT32					GetGearNameHash(CPlayerCharSelection* pSel);
	// Gets selected ultimate ability. eTerAbility is the ability and nRetNameStrIdx is the string index of the ability's name 
	bool					GetUltimateAbility(CPlayerCharSelection* pSel, eStrategicAbility & eRetAbility, int & nRetNameStrIdx);
	// Updates weapons selection options from current type
	void					SetWeaponsOptions(CPlayerCharSelection* pSel, bool bResetWeaponsSelection = true);
	// Sets the current selected items prices in the selection price field 
	void					SetSelectionPrices(CPlayerCharSelection* pSel);
	// Initializes the current upgrade bars
	void					InitUpgradeBars(CPlayerCharSelection* pSel);
	//Fills arrRetValues with perks UIDs
	int						GetAllActivePerks(CPlayerCharSelection* pSel, UINT32 arrRetValues[], int nRetValuesArrSize);
	//Returns perk activation status by perk name (activated or not)
	bool					IsPerkEnabled(int nPlayerOrdinal, const CStringHash * strPerkName);
	//Gets the fill percent of an upgrade bar specified by name
	float					GetUpgradeBarPercent(CPlayerCharSelection* pSel, WCHAR* strBarName);


	HRESULT OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnLostDevice(void);
	HRESULT OnDestroyDevice(void);
};

