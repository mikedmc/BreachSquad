#pragma once
#include "components/PhysPtComp.h"

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
	CPointPhysComponent*	c_pointPhys;		// point physics component
	VecProj					pos;

public:
	EDooferType			type;
	int					nSubType;

	float				fTimer;			
	//bool				bAnimated;	
	CSpr				spr;
	float				fSize;

	bool				bMakesLight;
	//CSpr				sprLight;
	float				fLightDuration, fLightFadeOut;	
	float				fLightScaling;
	float				fLightTimer;		

	bool				bVar1;
	int					nIntVar1;

	CDoofer();
	~CDoofer();
	// resets a doofer for reusage
	void				Reset();
	// updates a doofer (mostly for physics but sprites can be updated here too)
	void				Update( float dTime, CLevel & level );
};



