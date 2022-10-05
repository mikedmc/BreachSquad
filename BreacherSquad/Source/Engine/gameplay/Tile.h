#pragma once

///--- TILE LAYERS ---
// the ingame layers used for rendering
enum eTileLayer {
	K_TILE_LAYER_UNDER_FLOOR = 0,
	// main walkable floor tiles
	K_TILE_LAYER_FLOOR = 1,
	// mainly used for transitions
	K_TILE_LAYER_FLOOR_DECO1,
	// secondary decorations, over transitions
	K_TILE_LAYER_FLOOR_DECO2,
	// vertical walls
	K_TILE_LAYER_WALLS,
	// ceiling objects like pipes and other stuff that can cast shadows
	K_TILE_LAYER_CEILING_DECO,
	// Wall sections and FOW
	K_TILE_LAYER_CEILING,

	//total number of layers
	K_TILE_LAYERS_CNT,
};

//#MAYBE: should be removed after transitioning the editor to the game engine?
enum eEditorLayer {
};

//#TEMP: back compatibility, to be removed
/*
#define		K_LVL_LAYER_BACK	K_TILE_LAYER_FLOOR
#define		K_LVL_LAYER_MIDDLE	K_TILE_LAYER_WALLS
#define		K_LVL_LAYER_FRONT	K_TILE_LAYER_CEILING
#define		K_LVL_LAYERS_CNT	3
*/

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
// flag for tile with wall on it
#define		K_TILEFLAG_WALL 32
// wall lateral endings for wall ending shadows (on wall ending marks)
#define		K_TILEFLAG_WALLENDING_L 64
#define		K_TILEFLAG_WALLENDING_R 128
// mask that deletes wall endings
#define		K_TILEFLAG_WALLENDING_MASK 0xC0
// flag for under the floor tiles
#define		K_TILEFLAG_UNDER_FLOOR	256

class CTile {						
public:
	UINT32		flags;							// tile flags
	int			tileIDs[K_TILE_LAYERS_CNT]{-1};	// actual tile
	//RECT		srcRects[K_TILE_LAYERS_CNT]{};	// #TEMP: will be removed (precomputed RECT for drawing as sprite)
	
	Vec2		vUVmin[K_TILE_LAYERS_CNT];		// precomputed UV coords min 
	Vec2		vUVmax[K_TILE_LAYERS_CNT];		// precomputed UV coords max

	int			nShadowFrame;					// Frame of shadow from lights sprites (3 times the resolution). See sprite for shadow frames.
	CAABB		bbox;							// BBox of tile in world coords containing tile collision flags

	CTile() : flags(K_TILEFLAG_NONE), nShadowFrame(-1)
	{
		bbox.Set( 0, 0, 0, 0 );
		for (int kk = 0; kk < K_TILE_LAYERS_CNT; kk++)
		{
			tileIDs[kk] = -1;
			//SetRect(&srcRects[kk], 0, 0, 0, 0);
			
			vUVmin[kk] = Vec2(0.0f, 0.0f);
			vUVmax[kk] = Vec2(0.0f, 0.0f);
		}
	}

	void PostConstructionInit()
	{
		// mark all floor tiles with flags
		if ((tileIDs[K_TILE_LAYER_FLOOR] >= 0) || ( tileIDs[K_TILE_LAYER_FLOOR_DECO1] >= 0 ) || ( tileIDs[K_TILE_LAYER_FLOOR_DECO2] >= 0 ))
			flags |= K_TILEFLAG_WALKABLE;
		// mark all wall tiles with flags
		if ( tileIDs[K_TILE_LAYER_WALLS] >= 0 )
			flags |= K_TILEFLAG_WALL;
		// mark under floor tiles
		if ( tileIDs[K_TILE_LAYER_UNDER_FLOOR] >= 0 )
			flags |= K_TILEFLAG_UNDER_FLOOR;
	}
};


namespace nsTiles
{
	// converts world coords into tile coords
	inline Vec2i ToTilePos( Vec2 vPosPixels ) 
	{
		return { ( int ) floor( vPosPixels.x / K_TILE_SIZE_F ), ( int ) floor( vPosPixels.y / K_TILE_SIZE_F ) };
	}

	// converts tile pos to tile center in pixels
	inline Vec2 GetTileCenter( Vec2i vTilePos )
	{
		return { vTilePos.x * K_TILE_SIZE_F + K_TILE_HSIZE_F, vTilePos.y * K_TILE_SIZE_F + K_TILE_HSIZE_F };
	}
}