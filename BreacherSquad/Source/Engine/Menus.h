#pragma once

// Menu class that listens to state changes
class CMenus : public IEventListener {
private:
	PDEVICE				m_pDevice;
	eGameState			m_gameState;		// global gamestate 
	int					m_nSubstate;		// current sub state
	double				fLocalTimeline;

public: 
	CSpriteCollection	m_sprCol;			

public:
	CMenus();
	~CMenus();

	// Loads necessary sprites
	OPRESULT			Init();

	// Updates the scene
	void				Update( float dTime );

	// Renders final scene
	void				Paint();

	// Paints the animated background
	void				PaintBackground(RECTXYWH_F worldRect, DWORD dwColor, bool bPaintParticles = false, bool bPaintTitle = false);

	// Releases all resources
	void				Release();

public:
	//--- EVENTS ---
	char const* 		GetListenerName( void ) { return "Menus"; };
	// the game state event comes in through this and setState gets called
	bool				HandleEvent( CEvent &nEvent );

private:
	// Sets the current menus state to match the gamestate
	void				SetState( eGameState neState );

public:
	OPRESULT OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc = nullptr);
	OPRESULT OnResetDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc = nullptr);
	OPRESULT OnLostDevice();
	OPRESULT OnDestroyDevice();
};

