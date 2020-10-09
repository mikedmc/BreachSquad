#pragma once

// background elements will be exported with a special flag (flag builder in editor)

///--- BACKGROUNDS ---
//pe cati biti e salvata valoarea flagurilor
#define K_LVL_BK_AFRAMEBITMASK_REPEAT_X 1
#define K_LVL_BK_AFRAMEBITMASK_REPEAT_Y 1
#define K_LVL_BK_AFRAMEBITMASK_XMOVE_MUL 255
#define K_LVL_BK_AFRAMEBITMASK_YMOVE_MUL 255
#define K_LVL_BK_AFRAMEBITMASK_XBYTIME   63
#define K_LVL_BK_AFRAMEBITMASK_XBYTIMENEG 1
#define K_LVL_BK_AFRAMEBITMASK_YBYTIME 63
#define K_LVL_BK_AFRAMEBITMASK_YBYTIMENEG 1
//si shifingul dinainte de mascare
#define K_LVL_BK_AFRAMESHIFT_REPEAT_X 0
#define K_LVL_BK_AFRAMESHIFT_REPEAT_Y 1
#define K_LVL_BK_AFRAMESHIFT_XMOVE_MUL 2
#define K_LVL_BK_AFRAMESHIFT_YMOVE_MUL 10
#define K_LVL_BK_AFRAMESHIFT_XBYTIME   18
#define K_LVL_BK_AFRAMESHIFT_XBYTIMENEG 24
#define K_LVL_BK_AFRAMESHIFT_YBYTIME	25
#define K_LVL_BK_AFRAMESHIFT_YBYTIMENEG 31

void PaintParallaxBackground(CSpriteCollection* m_sprBack, int m_BackAnimIdx, RECTXYWH_F camRect, D3DXVECTOR2 vLevelOrigin, double fLocalTimeline);