#pragma once
#include "interfaces/DeviceRes.h"

// Menu class that listens to state changes
class CMenus : public IEventListener, IDeviceRes {
private:
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
	// Inherited via IEventListener
	char const* 		GetListenerName( void ) { return "Menus"; };
	// The game state event comes in through this and setState gets called
	bool				HandleEvent( CEvent &nEvent );

private:
	// Sets the current menus state to match the gamestate
	void				SetState( eGameState neState );

public:
	// Inherited via IDeviceRes
	virtual OPRESULT OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc = nullptr ) override;
	virtual OPRESULT OnResetDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc = nullptr ) override;
	virtual OPRESULT OnLostDevice() override;
	virtual OPRESULT OnDestroyDevice() override;
};

