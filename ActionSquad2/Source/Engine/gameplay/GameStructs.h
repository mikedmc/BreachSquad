#pragma once

///--- TILE CLASS ---
#define K_LVL_LAYER_BACK	0
#define K_LVL_LAYER_MIDDLE	1
#define K_LVL_LAYER_FRONT	2
//total number of layers
#define K_LVL_LAYERS_CNT 3

class CTile {
public:
	int tileIDs[K_LVL_LAYERS_CNT];
	RECT srcRects[K_LVL_LAYERS_CNT];

	CTile()
	{
		for (int kk = 0; kk < K_LVL_LAYERS_CNT; kk++)
		{
			tileIDs[kk] = -1;
			SetRect(&srcRects[kk], 0, 0, 0, 0);
		}
	}
};
