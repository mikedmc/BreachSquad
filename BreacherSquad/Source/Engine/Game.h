#pragma once

// max interval for GC calls
#define K_GAME_GC_TIMER_S	1.0f
// fixed timestep
#define	K_GAME_FIXED_TIMESTEP_DTIME			(1.0f / 60.0f)


class CGame {
private:
	double				fTimeline;
	float				fGCtimer;			// garbage collect timer
	float				fFixedStepTimer;	// time accumulator for fixed timestep

public:
	CGame();
	~CGame();

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
