#pragma once
//base class
class CCustomInterface
{
protected:
	float				fLocalTimeline;
	CSpriteCollection*	sprCol;
	float				fAnchorX;				// anchor between -1.0f and 1.0f
	float				fAnchorY;

public:
	CCustomInterface():
		fLocalTimeline(0.0f), sprCol(nullptr), fAnchorX(0.0f), fAnchorY(0.0f)
	{}

	// Sets the anchor for the current interface.
	// @anchorX, anchorY - float values (-1.0f min, 0.0f mid, 1.0f max)
	void			SetAnchor(float anchorX, float anchorY) { fAnchorX = anchorX; fAnchorY = anchorY; };

	// \returns true if clicked interface
	virtual bool	Update(float dTime) = 0; 
	virtual void	Paint(PDEVICE pDevice) = 0;
	virtual void	Release() = 0;
};

// we need refs
class CLevel;
class CActor;

#define K_CI_IGM_LIFE_BAR_MULTIPLIER 0.5f
#define K_CI_IGM_SHIELD_BAR_MULTIPLIER 0.2f
class CCustomInterfaceIGM: public CCustomInterface
{
private:
	int			nPortraitFrame[K_MAX_PLAYERS_CNT];			// Portrait frame for each player
	int			nPortraitFrameHotJoin[K_MAX_PLAYERS_CNT];	// Frames for hot join (-1 = none). Painted when player is null (not spawned yet)
	CActor*		playerAct[K_MAX_PLAYERS_CNT];				// Pointers to players
	bool		bPlayerPlayedBefore[K_MAX_PLAYERS_CNT];		// Played before?
	bool		bHasExtraSPSlots[K_MAX_PLAYERS_CNT];		// Has extra SP slots?
	float		m_fBombTimer;								// Shown only if >= 0.0f
public:
	//string names for keys
	CStringDesc	arrKeyNames[K_MAX_PLAYERS_CNT][10]; //0-change wpn, 1-alt fire, 2-use gear, 3-reload, 4 - kick, 5 - strategic menu, 6-activate special
	//icon frames for controllers. -1 to use string name instead
	int			arrKeyIcons[K_MAX_PLAYERS_CNT][10]; //0-change wpn, 1-alt fire, 2-use gear, 3-reload, 4 - kick, 5 - strategic menu, 6-activate special
private:
	// Strategic bar vars:
	float		fStrategicHighlightTimer;					// just received points, show them
	float		fStrategicPoints[K_MAX_PLAYERS_CNT];
	float		fStrategicPoints_old[K_MAX_PLAYERS_CNT];	// old value in order to detect changes
	int			nStrategicSelection[K_MAX_PLAYERS_CNT];		// selectia curenta pe strategic bar
	int			nLivesLeft[K_MAX_PLAYERS_CNT];				// lives for each player

	void		PaintInterfaceForPlayer(PDEVICE pDevice, int nPlayerOrdinal, RECTXYWH_F scrRect, bool bFlipped, int arrStrategic[], int arrStrategicNames[]);
	void		UpdateInterfaceForPlayer(int nPlayerOrdinal, Vec2 vPos, bool bFlipped);

public:
	CCustomInterfaceIGM();

	void Init(CSpriteCollection* sprCollection, CActor * player1, CActor * player2);
	//resets everything
	void Reset(); 
	void SetHotJoinSelection(int nPlayerOrdinal, int nSelectedType);
	//sets the strategic points new value and saves the old value too
	void SetStrategicPoints(float fPl1newVal, float fPl2newVal);
	 // Sets the selection for the interface strategic 
	 // nSelection = -1 for no selection
	void SetStrategicSelection(int nPlayerOrdinal, int nSelection);
	 //	Sets the interface countdown. Negative value means Don't show!
	void SetBombTimer(float fTimer);
	const float GetBombTimer() const {
		return m_fBombTimer;
	};
	//sets number of lives left
	void SetLivesLeft(int nPl1Lives, int nPl2Lives);


	bool Update(float dTime); //returns true if clicked interface
	void Paint(PDEVICE pDevice);
	void Release();
};


//MAXIMUM BUBBLE WIDTH
#define K_CI_TB_MAX_WIDTH 120
//bubble fade speed
#define K_CI_TB_FADE_SPEED_PERSEC 5.0f

// Used to draw text windows for tutorials and the dialogs between characters
class CCustomInterfaceTextBubble : public CCustomInterface
{
private:
	enum eCIType {
		K_TYPE_NONE,

		K_TYPE_LEVEL_HINT,
		K_TYPE_LOCKED_DOOR_HINT,
	};
private:
	eCIType			eType;
	D3DXVECTOR2		pos;
	RECTXYWH_F		bbox;
	float			fAlpha;		//current alpha
	bool			bActive;	//daca este activa sau nu
	int				nTextIDX;	//index string
	float			fShowTimer;	//for how long will we see it
	int				nFontID;	//font id

	int				nParam1;	//generic int param
	CStringHash		shString;	//local generic string

	CCameraTransform	*m_pCamera; //camera pointer

public:
	CCustomInterfaceTextBubble();

	 // \brief Shows a text hint in level coords and fits the text on screen
	 // \param fTimer - sets the show timer only if bigger than 0.0f 
	void ShowLevelHint(CCameraTransform* pCamera, int nnTextIndex, int nnFontID, D3DXVECTOR2 vPos, float fTimer = 0.0f);

	void Hide(bool bForced = false);

	void Init(CSpriteCollection* sprCollection);
	bool Update(float dTime);
	void Paint(PDEVICE pDevice);
	void Release();
};

