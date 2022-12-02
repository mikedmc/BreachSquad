#pragma once

#define K_LVL_DOOFERS_MAX_CNT 256
// doofer types
enum EDooferType {
	K_DOOFER_NOT_SET = -1,

	K_DOOFER_SHELL = 0,
	K_DOOFER_MEAT = 1,	
	K_DOOFER_SHRAPNEL_SMOKING, 
	K_DOOFER_LIGHT,		
	K_DOOFER_EXPLOSION,	
	K_DOOFER_FIRE_SOURCE,
};

// Class of objects that are "active" and may have special behaviors, and are not props. 
// Includes bullet shells, gibs, etc. Behavior usually hardcoded.
// They are usually short-lived so they don't need visibility lists
class CDoofer {
public:
	EDooferType	type;

	int			nSubType;	
	float		fTimer;			
	bool		bAnimated;	
	CSpr		spr, spr2;	
	float		fSize;

	bool		bMakesLight;
	CSpr		sprLight;
	float		fLightDuration, fLightFadeOut;	
	float		fLightScaling;
	float		fLightTimer;		

	bool		bVar1;
	int			nIntVar1;

	CDoofer() : type(K_DOOFER_NOT_SET), nSubType(0), fTimer(0.0f), bAnimated(false), fSize(1.0f),
		bMakesLight(false), fLightDuration(0.0f), fLightFadeOut(0.0f), fLightScaling(1.0f), fLightTimer(0.0f), bVar1(false), nIntVar1(0)
	{
	}

	void Reset()
	{
		type = K_DOOFER_NOT_SET; nSubType = 0; fTimer = 0.0f; bAnimated = false; fSize = 1.0f;
		bMakesLight = false; fLightDuration = 0.0f; fLightFadeOut = 0.0f; fLightScaling = 1.0f; fLightTimer = 0.0f;
	}
};



