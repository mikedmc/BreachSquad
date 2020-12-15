#include "dxstdafx.h"
#include "ParallaxBg.h"

void PaintParallaxBackground(CSpriteCollection* m_sprBack, int m_BackAnimIdx, RECTXYWH_F camRect, D3DXVECTOR2 vLevelOrigin, double fLocalTimeline)
{
	if ((!m_sprBack->IsLoaded()) || (m_BackAnimIdx < 0))
		return;

	CSprite	spr;
	//2. merge prin toate frames din prima animatie din fisier (cea care contine toate layerele de desenat)
	for (int kk = 0; kk < m_sprBack->GetAFramesCnt(m_BackAnimIdx); kk++)
	{
		//Cred ca toate aceste decodari de flag ar trebui facute pe onload si puse in structuri de background layer
		UINT32 aframeflag = m_sprBack->GetAFrameFlag(m_BackAnimIdx, kk);
		bool repeatX = (aframeflag >> K_LVL_BK_AFRAMESHIFT_REPEAT_X) & K_LVL_BK_AFRAMEBITMASK_REPEAT_X;
		bool repeatY = (aframeflag >> K_LVL_BK_AFRAMESHIFT_REPEAT_Y) & K_LVL_BK_AFRAMEBITMASK_REPEAT_Y;
		float xmove_mul = float((aframeflag >> K_LVL_BK_AFRAMESHIFT_XMOVE_MUL) & K_LVL_BK_AFRAMEBITMASK_XMOVE_MUL) / 255.0f;
		float ymove_mul = float((aframeflag >> K_LVL_BK_AFRAMESHIFT_YMOVE_MUL) & K_LVL_BK_AFRAMEBITMASK_YMOVE_MUL) / 255.0f;
		float xmove_byTime = float((aframeflag >> K_LVL_BK_AFRAMESHIFT_XBYTIME) & K_LVL_BK_AFRAMEBITMASK_XBYTIME);
		if ((aframeflag >> K_LVL_BK_AFRAMESHIFT_XBYTIMENEG) & K_LVL_BK_AFRAMEBITMASK_XBYTIMENEG)
			xmove_byTime = -xmove_byTime;
		float ymove_byTime = float((aframeflag >> K_LVL_BK_AFRAMESHIFT_YBYTIME) & K_LVL_BK_AFRAMEBITMASK_YBYTIME);
		if ((aframeflag >> K_LVL_BK_AFRAMESHIFT_YBYTIMENEG) & K_LVL_BK_AFRAMEBITMASK_YBYTIMENEG)
			ymove_byTime = -ymove_byTime;

		const float fMoveByTimeMultiplier = 2.0f;
		ymove_byTime *= fMoveByTimeMultiplier;
		xmove_byTime *= fMoveByTimeMultiplier;

		RECTXYWH aframeBB = m_sprBack->GetAFrameBBox(m_BackAnimIdx, kk);
		///--- desenarea efectiva ---
		D3DXVECTOR2 camOffset(vLevelOrigin.x - camRect.x, vLevelOrigin.y - camRect.Bottom());
		D3DXVECTOR2 paintOffset(camOffset.x * xmove_mul - xmove_byTime * fLocalTimeline, camOffset.y * ymove_mul + ymove_byTime * fLocalTimeline);
		//#TODO: de facut sa se repete si pe Y
		if (repeatX && repeatY)
		{
			spr.Init(m_BackAnimIdx, 0.0f, 0.0f, kk);
			int timesx = 2 + (int)(ceil(camRect.w)) / aframeBB.w;
			int timesy = 2 + (int)(ceil(camRect.h)) / aframeBB.h;

			float localoffsetx = FLOAT_FRAC(paintOffset.x / (float)aframeBB.w) * aframeBB.w;
			float localoffsety = FLOAT_FRAC(paintOffset.y / (float)aframeBB.h) * aframeBB.h;
			for (int llx = -1; llx < timesx; llx++)
			{
				for (int lly = -1; lly <= timesy; lly++)
				{
					spr.pos = D3DXVECTOR2(camRect.x + localoffsetx + llx * aframeBB.w, camRect.Bottom() + localoffsety - lly * aframeBB.h);
					spr.paint(m_sprBack);
				}
			}
		}
		else if (repeatX)
		{
			spr.Init(m_BackAnimIdx, 0.0f, 0.0f, kk);
			//daca trebuie facut tiling pe X se calculeaza de cate ori intra in ecran ca sa-l acopere
			int times = 2 + (int)(ceil(camRect.w)) / aframeBB.w;
			//desenam unul in stanga originii locale in plus fata de cate ori intra
			float localoffsetx = FLOAT_FRAC(paintOffset.x / (float)aframeBB.w) * aframeBB.w;
			for (int ll = -1; ll < times; ll++)
			{
				spr.pos = D3DXVECTOR2(camRect.x + localoffsetx + ll * aframeBB.w, camRect.Bottom() + paintOffset.y);
				spr.paint(m_sprBack);
			}
		}
		else if (repeatY)
		{
			spr.Init(m_BackAnimIdx, 0.0f, 0.0f, kk);
			//daca trebuie facut tiling pe Y se calculeaza de cate ori intra in ecran ca sa-l acopere
			int times = 2 + (int)(ceil(camRect.h)) / aframeBB.h; //adaug 2 ca sa nu conteze cum sunt aliniate obiectele din editor fata de origine
			 //desenam unul in stanga originii locale in plus fata de cate ori intra
			float localoffsety = FLOAT_FRAC(paintOffset.y / (float)aframeBB.h) * aframeBB.h;
			for (int ll = -1; ll <= times; ll++)
			{
				spr.pos = D3DXVECTOR2(camRect.x + paintOffset.x, camRect.Bottom() + localoffsety - ll * aframeBB.h);
				spr.paint(m_sprBack);
			}
		}
		else //no repeat flags
		{
			spr.Init(m_BackAnimIdx, camRect.x + paintOffset.x, camRect.Bottom() + paintOffset.y, kk);
			spr.paint(m_sprBack);
		}
	}
}
