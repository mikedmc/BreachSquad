#pragma once

///--- TILE CLASS ---
#define		K_LVL_LAYER_BACK		0
#define		K_LVL_LAYER_MIDDLE		1
#define		K_LVL_LAYER_FRONT		2
//total number of layers
#define		K_LVL_LAYERS_CNT		3

///--- TILE FLAGS ---
#define		K_TILEFLAG_NONE			0
// do we have a floor on FLOORS layer? then we call it walkable
#define		K_TILEFLAG_WALKABLE		1
// visible walls flags (bits 1-4)
#define		K_TILEFLAG_HASWALL_U	2
#define		K_TILEFLAG_HASWALL_R	4
#define		K_TILEFLAG_HASWALL_D	8
#define		K_TILEFLAG_HASWALL_L	16
#define		K_TILEFLAG_HASWALL_MASK 0x1E

class CTile {						
public:
	UINT32					flags;
	int tileIDs[K_LVL_LAYERS_CNT];
	RECT srcRects[K_LVL_LAYERS_CNT];
	// precomputed UV coords (min and max)
	Vec2 vUVmin[K_LVL_LAYERS_CNT];
	Vec2 vUVmax[K_LVL_LAYERS_CNT];

	CTile()
	{
		for (int kk = 0; kk < K_LVL_LAYERS_CNT; kk++)
		{
			flags = K_TILEFLAG_NONE;
			tileIDs[kk] = -1;
			SetRect(&srcRects[kk], 0, 0, 0, 0);
			
			vUVmin[kk] = Vec2(0.0f, 0.0f);
			vUVmax[kk] = Vec2(0.0f, 0.0f);
		}
	}

	void PostConstructionInit()
	{
		if (tileIDs[K_LVL_LAYER_BACK] >= 0)
			flags |= K_TILEFLAG_WALKABLE;
	}
};


