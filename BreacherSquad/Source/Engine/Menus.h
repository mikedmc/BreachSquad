#pragma once
#include "interfaces/DeviceRes.h"
#include "Queue.h"

// Menu class that listens to state changes
class CMenus : public IEventListener, IDeviceRes {
private:
	EGameState			m_state;		// global gamestate 
	int					m_nSubstate;		// current sub state
	double				fLocalTimeline;

public: 
	CSpriteLib			m_sprCol;			

public:
	CMenus();
	~CMenus();

	// Loads necessary sprites and initializes necessary data (on load)
	OPRESULT			Init();

	// Updates the scene
	void				Update( float dTime );

	// Renders final scene
	void				Paint();

	// Paints the animated background
	void				PaintBackground(RectXYWH worldRect, DWORD dwColor, bool bPaintParticles = false, bool bPaintTitle = false);

	// Releases all resources
	void				Release();

public:
	// Inherited via IEventListener
	char const* 		GetListenerName() override { return "Menus"; };
	// The game state event comes in through this and setState gets called
	bool				HandleEvent( CEvent &nEvent ) override;

private:
	// Sets the current menus state to match the gamestate
	void				SetState( EGameState neState );

public:
	// Inherited via IDeviceRes
	OPRESULT OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc = nullptr ) override;
	OPRESULT OnResetDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc = nullptr ) override;
	OPRESULT OnLostDevice() override;
	OPRESULT OnDestroyDevice() override;
};

