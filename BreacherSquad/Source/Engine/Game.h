#pragma once

// max interval for GC calls
#define K_GAME_GC_TIMER_S	1.0f

class CGame {
private:
	double				fTimeline;
	float				fGCtimer;			// garbage collect timer

public:
	CGame();
	~CGame();

	// updates at a fixed timestep
	void				UpdateFixed( float dTime );
	// updates at variable timestep
	void				Update( float dTime, bool bSyncUpdate, int nUpdateFrame );
	// called before the actual painting the final scene but it is still called inside the paint method. Used to prepare deferred buffers.
	void				BeforePaint();
	// paints the actual final game
	void				Paint( PDEVICE pDevice, ID3DXSprite* pSpr, float dTime );
	// releases all game resources
	void				Release();
	// force call garbage collect. It gets called periodically after Update();
	void				GC();
};
