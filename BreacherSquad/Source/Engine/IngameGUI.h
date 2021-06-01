#pragma once

// we need refs
class CLevel;

// ingame interface
class CIngameGUI
{
protected:
	double					fLocalTimeline;				// local timeline for animations
	CLevel*					m_pLevel;					// pointer to level to get data from
	CCameraTransform*		m_pCamera;					// camera used for transformations and display size
	CSpriteCollection*		m_pSprite;					// sprite collection to paint from
public:
	// Initialize internal pointers
	void					Init(CLevel* pLevel, CCameraTransform* pCamera);
	// Update with each frame delta
	void					Update(float dTime);
	// Paints the actual interface
	void					Paint(PDEVICE pDevice);
	// Release all allocated stuff
	void					Release();
};
