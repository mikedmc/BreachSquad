#pragma once

///--- TILE LAYERS ---
enum eTileLayer {
	// main walkable floor tiles
	K_TILE_LAYER_FLOOR = 0,
	// mainly used for transitions
	K_TILE_LAYER_FLOOR_DECO1 = 1,
	// secondary decorations, over transitions
	K_TILE_LAYER_FLOOR_DECO2,
	// vertical walls
	K_TILE_LAYER_WALLS,
	// wall shadows (generated automatically)
	K_TILE_LAYER_SHADOWS,
	// ceiling objects like pipes and other stuff that can cast shadows
	K_TILE_LAYER_CEILING_DECO,
	// Wall sections and FOW
	K_TILE_LAYER_CEILING,

	//total number of layers
	K_TILE_LAYERS_CNT,
};

//#TEMP: back compatibility, to be removed
#define		K_LVL_LAYER_BACK	K_TILE_LAYER_FLOOR
#define		K_LVL_LAYER_MIDDLE	K_TILE_LAYER_WALLS
#define		K_LVL_LAYER_FRONT	K_TILE_LAYER_CEILING
#define		K_LVL_LAYERS_CNT	3



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
	int tileIDs[K_TILE_LAYERS_CNT];
	RECT srcRects[K_TILE_LAYERS_CNT];
	// precomputed UV coords (min and max)
	Vec2 vUVmin[K_TILE_LAYERS_CNT];
	Vec2 vUVmax[K_TILE_LAYERS_CNT];

	CTile()
	{
		for (int kk = 0; kk < K_TILE_LAYERS_CNT; kk++)
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
		if (tileIDs[K_TILE_LAYER_FLOOR] >= 0)
			flags |= K_TILEFLAG_WALKABLE;
	}
};


