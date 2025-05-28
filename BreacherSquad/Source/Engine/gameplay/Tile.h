#pragma once

///--- TILE LAYERS ---
// the ingame layers used for rendering
enum EEditorLayer {
	K_TILE_LAYER_UNDEFINED = -1,

	K_TILE_LAYER_UNDER_FLOOR = 0,
	// main walkable floor tiles
	K_TILE_LAYER_FLOOR = 1, 
	///----------------------------------------------------------------------------------
	/// Objects on layers until here aren't sorted and are painted as they come
	///----------------------------------------------------------------------------------
	// mainly used for transitions
	K_TILE_LAYER_FLOOR_DECO1,
	// secondary decorations, over transitions
	K_TILE_LAYER_FLOOR_DECO2,
	///----------------------------------------------------------------------------------
	/// Objects after this layer aren't sorted and are painted as they come
	///----------------------------------------------------------------------------------
	// vertical walls
	K_TILE_LAYER_WALLS, // objects on this layer don't get sorted
	// vertical walls decorations
	K_TILE_LAYER_WALLS_DECO,
	// ceiling objects like pipes and other stuff that can cast shadows
	K_TILE_LAYER_CEILING_DECO,
	// Wall sections and FOW
	K_TILE_LAYER_CEILING,

	//total number of layers
	K_TILE_LAYERS_CNT,
};

// value of empty tile
#define		K_TILEXY_EMPTY			0xffff

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
// flag for under the floor tiles (water usually)
#define		K_TILEFLAG_UNDER_FLOOR	256

class CTile {						
public:
	UINT32		flags;							// tile flags
	UINT16		tileXY[K_TILE_LAYERS_CNT]{K_TILEXY_EMPTY};	// tile position in tileset in tile coords  (empty = 0xffff) => [X << 8 | Y]
	
	Vec2		vUVmin[K_TILE_LAYERS_CNT];		// precomputed UV coords min 
	Vec2		vUVmax[K_TILE_LAYERS_CNT];		// precomputed UV coords max

	int			nShadowFrame;					// Frame of shadow from lights sprites (3 times the resolution). See sprite for shadow frames.
	CAABB		bbox;							// BBox of tile in world coords containing tile collision flags

	CTile() : flags(K_TILEFLAG_NONE), nShadowFrame(-1)
	{
		bbox.Set( 0, 0, 0, 0 );
		for (int kk = 0; kk < K_TILE_LAYERS_CNT; kk++)
		{
			tileXY[kk] = K_TILEXY_EMPTY;
			
			vUVmin[kk] = Vec2(0.0f, 0.0f);
			vUVmax[kk] = Vec2(0.0f, 0.0f);
		}
	}

	void PostConstructionInit()
	{
		// mark all floor tiles with walkable so we can make transitions
		if (( tileXY[K_TILE_LAYER_FLOOR] != K_TILEXY_EMPTY ) ||
			( tileXY[K_TILE_LAYER_FLOOR_DECO1] != K_TILEXY_EMPTY ) ||
			( tileXY[K_TILE_LAYER_FLOOR_DECO2] != K_TILEXY_EMPTY ) )
			flags |= K_TILEFLAG_WALKABLE;
		// mark all wall tiles with flags
		if (( tileXY[K_TILE_LAYER_WALLS] != K_TILEXY_EMPTY ) || 
			( tileXY[K_TILE_LAYER_WALLS_DECO] != K_TILEXY_EMPTY ))
			flags |= K_TILEFLAG_WALL;
		// mark under floor tiles
		if ( tileXY[K_TILE_LAYER_UNDER_FLOOR] != K_TILEXY_EMPTY )
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