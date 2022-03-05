#include "dxstdafx.h"
#include "Spr.h"
//--- EDITOR duration per frame (100 = 1 sec) -----
#define SPR_ED_TIMELINE 100.0f

///--- STATIC MEMBERS ---
CSpritePainter* CSpr::s_pSP = &__Painter();

CSpr::CSpr() 
{
	pSprCol = nullptr;
	animIdx = 0;
	pos.x = 0.0f;
	pos.y = 0.0f;
	fTime = 0.0f;
	frameIdx = 0;
	animStatus = ANIM_JUST_STARTED;
	color = 0xffffffff;
	scale = Vec2(1.0f, 1.0f);
	rotation = 0.0f;
}

CSpr::CSpr(CSpriteLib* pSpriteColl, int animIdx, float pX, float pY)
{
	pSprCol = pSpriteColl;
	animIdx = animIdx;
	pos.x = pX;
	pos.y = pY;
	fTime = 0.0f;
	frameIdx = 0;
	animStatus = ANIM_JUST_STARTED;
	color=0xffffffff;
	rotation = 0.0f;
	scale = Vec2(1.0f, 1.0f);
}

CSpr::CSpr(CSpriteLib* pSpriteColl, int animIdx, Vec2 vPos)
{
	pSprCol = pSpriteColl;
	animIdx = animIdx;
	pos = vPos;
	fTime = 0.0f;
	frameIdx = 0;
	animStatus = ANIM_JUST_STARTED;
	color = 0xffffffff;
	rotation = 0.0f;
	scale = Vec2(1.0f, 1.0f);
}

CSpr::CSpr(const CSpr& sprite)
{
	pSprCol = sprite.pSprCol;
	animIdx = sprite.animIdx;
	pos = sprite.pos;
	fTime = sprite.fTime;
	frameIdx = sprite.frameIdx;
	animStatus = sprite.animStatus;
	color = sprite.color;
	scale = sprite.scale;
	rotation = sprite.rotation;
}

void CSpr::Init(CSpriteLib *sprCollection, int nAnimIdx, Vec2 vPos, int nframeIdx, DWORD nColor, float fRotation, Vec2 vScale)
{
	pSprCol = sprCollection;
	animIdx = nAnimIdx;
	pos = vPos;
	fTime = 0.0f;
	frameIdx = nframeIdx;
	animStatus = ANIM_JUST_STARTED;
	color = nColor;
	scale = vScale;
	rotation = fRotation;
}

void CSpr::Init(CSpriteLib *sprCollection, CHAR* strAnimID, Vec2 vPos, int nframeIdx, DWORD nColor, float fRotation, Vec2 vScale)
{
	animIdx = sprCollection->GetAnimationIdxByName(strAnimID);
	if (animIdx < 0)
	{
		LOG("Sprite::Init: Animation [%s] not found!", strAnimID);
		return;
	}

	pSprCol = sprCollection;
	pos = vPos;
	fTime = 0.0f;
	frameIdx = nframeIdx;
	animStatus = ANIM_JUST_STARTED;
	color = nColor;
	scale = vScale;
	rotation = fRotation;
}

void CSpr::SetAnim(int nAnimIdx, int nFrameIdx)
{
	animIdx = nAnimIdx;

	fTime = 0.0f;
	frameIdx = nFrameIdx;
	animStatus = ANIM_JUST_STARTED;
}

void CSpr::SetFrame(int nFrameIdx)
{
	frameIdx = nFrameIdx;
	fTime = 0.0f;
	animStatus = ANIM_PLAYING;
}

void CSpr::SetAnimOnce(int nAnimIdx, int nFrameIdx) 
{
	if((animIdx == nAnimIdx) || (nAnimIdx < 0))
		return;

	animIdx = nAnimIdx;

	fTime = 0.0f;
	frameIdx = nFrameIdx;
	animStatus = ANIM_JUST_STARTED;
}

void CSpr::SetAnim(CHAR* strAnimID, int nFrameIdx)
{
	_ASSERT(pSprCol);
	animIdx = pSprCol->GetAnimationIdxByName(strAnimID);
	if (animIdx < 0)
	{
		LOG("Sprite::SetAnim: Animation [%s] not found!", strAnimID);
		return;
	}

	fTime = 0.0f;
	frameIdx = nFrameIdx;
	animStatus = ANIM_JUST_STARTED;
}

UINT32 CSpr::Update(float dTime, bool updatePos)
{
	UINT32 retAFrameFlag = 0;

	_ASSERT(animIdx < pSprCol->Animations.Count());
	_ASSERT(frameIdx < pSprCol->Animations[animIdx]->aframesNo);

	if (animStatus == ANIM_FRAMELOCK)
		return retAFrameFlag;

	int aframeID = pSprCol->Animations[animIdx]->aframesIdx[frameIdx];
	// return frame flag for animations that were just set now
	if (animStatus == ANIM_JUST_STARTED)
	{
		//#TODO: if first frame is too short the message gets lost retAFrameFlag
		retAFrameFlag = pSprCol->AFrames[aframeID]->flags;
	}

	animStatus = ANIM_PLAYING;
	fTime += dTime;

	if( fTime >= float(pSprCol->AFrames[aframeID]->duration) / SPR_ED_TIMELINE )
	{
		animStatus = ANIM_PLAYING_FRAME_ADVANCED;
		if( updatePos )
		{
			pos.x += pSprCol->AFrames[aframeID]->dx;
			pos.y += pSprCol->AFrames[aframeID]->dy;
		}

		fTime -= float(pSprCol->AFrames[aframeID]->duration) / SPR_ED_TIMELINE;
		frameIdx++;

		if (frameIdx >= pSprCol->Animations[animIdx]->aframesNo) 
		{
			if ( (pSprCol->Animations[animIdx]->flags & ANIMATION_FLAG_LOOPED) == 0 )
			{ //play once?
				frameIdx--; // sets on last frame
				animStatus = ANIM_FRAMELOCK; 
				//get new flag
				aframeID = pSprCol->Animations[animIdx]->aframesIdx[frameIdx];
			}
			else 
			{ //looping
				frameIdx = 0;
				animStatus = ANIM_LOOPRESET;
				//get new flag
				aframeID = pSprCol->Animations[animIdx]->aframesIdx[frameIdx];
			}
		}
		else
		{
			aframeID = pSprCol->Animations[animIdx]->aframesIdx[frameIdx];
		}
		// set aframe flag for return
		retAFrameFlag = pSprCol->AFrames[aframeID]->flags;
	}
	return retAFrameFlag;
}

void CSpr::Paint()
{
	_ASSERT(animIdx < pSprCol->Animations.Count());
	_ASSERT(frameIdx < pSprCol->Animations[animIdx]->aframesNo);
	//#TODO: flip flags not considered yet
	int aframeIdx = pSprCol->Animations[animIdx]->aframesIdx[frameIdx];
	for( int ii = 0; ii < pSprCol->AFrames[aframeIdx]->fmodulesNo; ii++ )
	{
		scFModule* mod = pSprCol->FModules[pSprCol->AFrames[aframeIdx]->fmodulesIdx[ii]];
		
		s_pSP->Draw(mod->pImg->pTex,
			mod->texRect,
			mod->moduleRectOff,
			pos,
			color);
	}
}

void CSpr::Paint( RectLTRB& clip )
{
	int aframeIdx = pSprCol->Animations[ animIdx ]->aframesIdx[ frameIdx ];
	for ( int ii = 0; ii < pSprCol->AFrames[ aframeIdx ]->fmodulesNo; ii++ )
	{
		int fmoduleIdx = pSprCol->AFrames[ aframeIdx ]->fmodulesIdx[ ii ];
		scFModule* mod = pSprCol->FModules[ fmoduleIdx ];

		RectLTRB destrect = mod->moduleRectOff;
		destrect.Move( pos.x, pos.y );
		// optimization: if contained, paint in full
		if ( clip.Contains( destrect ) )
		{
			__Painter().Draw( mod->pImg->pTex,
				mod->texRect,
				mod->moduleRectOff,
				pos, color );
			continue;
		}
		// compute intersection
		RectLTRB intersection;
		bool bIntersecting = RectLTRB::Intersection( destrect, clip, intersection );
		if ( !bIntersecting )
			continue;
		// compute texture coords by percenting coords (barycentric)
		Vec2 posCornerULPerc( ( intersection.left - destrect.left ) / destrect.Width(), ( intersection.top - destrect.top ) / destrect.Height() );
		Vec2 posCornerDRPerc( ( intersection.right - destrect.left ) / destrect.Width(), ( intersection.bottom - destrect.top ) / destrect.Height() );
		RectLTRB texrect = mod->texRect;
		Vec2 vTexSz( texrect.Width(), texrect.Height() );
		RectLTRB finaltex( texrect.left + posCornerULPerc.x * vTexSz.x, texrect.top + posCornerULPerc.y * vTexSz.y,
			texrect.left + posCornerDRPerc.x * vTexSz.x, texrect.top + posCornerDRPerc.y * vTexSz.y );
		// paints without offset because the offset is already in the clipped rectangle
		__Painter().Draw( mod->pImg->pTex,
			finaltex,
			intersection,
			Vec2( 0.0f, 0.0f ), color );
	}

}

void CSpr::PaintFModule(int moduleIdx)
{
	_ASSERT(animIdx < pSprCol->Animations.Count());
	_ASSERT(frameIdx < pSprCol->Animations[animIdx]->aframesNo);
	int aframeIdx = pSprCol->Animations[animIdx]->aframesIdx[frameIdx];
	_ASSERT(moduleIdx < pSprCol->AFrames[aframeIdx]->fmodulesNo); 

	scFModule* mod = pSprCol->FModules[pSprCol->AFrames[aframeIdx]->fmodulesIdx[moduleIdx]];

	s_pSP->Draw(mod->pImg->pTex, mod->texRect, mod->moduleRectOff, pos, color, rotation, scale);
}

void CSpr::PaintModule_texOverride(int moduleIdx, int texIdxOffset)
{
	_ASSERT(animIdx < pSprCol->Animations.Count());
	_ASSERT(frameIdx < pSprCol->Animations[animIdx]->aframesNo);
	int aframeIdx = pSprCol->Animations[animIdx]->aframesIdx[frameIdx];
	_ASSERT(moduleIdx < pSprCol->AFrames[aframeIdx]->fmodulesNo);

	scFModule* mod = pSprCol->FModules[pSprCol->AFrames[aframeIdx]->fmodulesIdx[moduleIdx]];

	s_pSP->Draw(pSprCol->Textures[mod->imgIdx + texIdxOffset]->pTex, mod->texRect, mod->moduleRectOff, pos, color, rotation, scale);
}

///----------------------------------------------------------------------------------
/// GENERIC STATIC FUNCTIONS
///----------------------------------------------------------------------------------
 

void UTSprite::PaintFrameEx(CSpriteLib *sprCol, Vec2 vPos, int animID, int frameIdx, DWORD ncolor, float fRotZ, Vec2 vScale, UINT unFlags)
{
	_ASSERT(animID < sprCol->Animations.Count());
	_ASSERT(frameIdx < sprCol->Animations[animID]->aframesNo);

	int aframeIdx = sprCol->Animations[animID]->aframesIdx[frameIdx];
	for (int ii = 0; ii < sprCol->AFrames[aframeIdx]->fmodulesNo; ii++)
	{
		scFModule* mod = sprCol->FModules[sprCol->AFrames[aframeIdx]->fmodulesIdx[ii]];

		__Painter().DrawEx(mod->pImg->pTex,
			mod->texRect,
			mod->moduleRectOff,
			vPos,
			ncolor, fRotZ, vScale, unFlags);
	}
}

void UTSprite::PaintFModule(CSpriteLib *sprCol, Vec2 vPos, int animID, int frameIdx, int moduleIdx, DWORD ncolor)
{
	_ASSERT(animID < sprCol->Animations.Count());
	_ASSERT(frameIdx < sprCol->Animations[animID]->aframesNo);
	_ASSERT(moduleIdx < sprCol->AFrames[sprCol->Animations[animID]->aframesIdx[frameIdx]]->fmodulesNo);

	int aframeIdx = sprCol->Animations[animID]->aframesIdx[frameIdx];
	scFModule* mod = sprCol->FModules[sprCol->AFrames[aframeIdx]->fmodulesIdx[moduleIdx]];

	__Painter().Draw(mod->pImg->pTex,
		mod->texRect,
		mod->moduleRectOff,
		vPos, ncolor);
}


void UTSprite::PaintFrameClipped( CSpriteLib *sprCol, Vec2 vPos, int animID, int frameIdx, RectLTRB& clip, DWORD ncolor )
{
	int aframeIdx = sprCol->Animations[ animID ]->aframesIdx[ frameIdx ];
	for ( int ii = 0; ii < sprCol->AFrames[ aframeIdx ]->fmodulesNo; ii++ )
	{
		int fmoduleIdx = sprCol->AFrames[ aframeIdx ]->fmodulesIdx[ ii ];
		scFModule* mod = sprCol->FModules[ fmoduleIdx ];

		RectLTRB destrect = mod->moduleRectOff;
		destrect.Move( vPos.x, vPos.y );
		// optimization: if contained, paint in full
		if ( clip.Contains( destrect ) )
		{
			__Painter().Draw( mod->pImg->pTex,
				mod->texRect,
				mod->moduleRectOff,
				vPos, ncolor );
			continue;
		}
		// compute intersection
		RectLTRB intersection;
		bool bIntersecting = RectLTRB::Intersection( destrect, clip, intersection );
		if ( !bIntersecting )
			continue;
		// compute texture coords by percenting coords (barycentric)
		SizeWH destsz = destrect.Size();
		Vec2 posCornerULPerc( ( intersection.left - destrect.left ) / destsz.w, ( intersection.top - destrect.top ) / destsz.h );
		Vec2 posCornerDRPerc( ( intersection.right - destrect.left ) / destsz.w, ( intersection.bottom - destrect.top ) / destsz.h );
		RectLTRB texrect = mod->texRect;
		SizeWH vTexSz = texrect.Size();
		RectLTRB finaltex( texrect.left + posCornerULPerc.x * vTexSz.w, texrect.top + posCornerULPerc.y * vTexSz.h,
			texrect.left + posCornerDRPerc.x * vTexSz.w, texrect.top + posCornerDRPerc.y * vTexSz.h );
		// paints without offset because the offset is already in the clipped rectangle
		__Painter().Draw( mod->pImg->pTex,
			finaltex,
			intersection,
			Vec2( 0.0f, 0.0f ), ncolor );
	}
}

void UTSprite::PaintFModuleClipped( CSpriteLib *sprCol, Vec2 vPos, int animID, int frameIdx, int moduleIdx, RectLTRB& clip, DWORD ncolor /*= 0xffffffff*/ )
{
	int aframeIdx = sprCol->Animations[ animID ]->aframesIdx[ frameIdx ];

	int fmoduleIdx = sprCol->AFrames[ aframeIdx ]->fmodulesIdx[ moduleIdx ];
	scFModule* mod = sprCol->FModules[ fmoduleIdx ];

	RectLTRB destrect = mod->moduleRectOff;
	destrect.Move( vPos.x, vPos.y );
	// optimization: if contained, paint in full
	if ( clip.Contains( destrect ) )
	{
		__Painter().Draw( mod->pImg->pTex,
			mod->texRect,
			mod->moduleRectOff,
			vPos, ncolor );
		return;
	}
	// compute intersection
	RectLTRB intersection;
	bool bIntersecting = RectLTRB::Intersection( destrect, clip, intersection );
	if ( !bIntersecting )
		return;
	// compute texture coords by percenting coords (barycentric)
	SizeWH destsz = destrect.Size();
	Vec2 posCornerULPerc( ( intersection.left - destrect.left ) / destsz.w, ( intersection.top - destrect.top ) / destsz.h );
	Vec2 posCornerDRPerc( ( intersection.right - destrect.left ) / destsz.w, ( intersection.bottom - destrect.top ) / destsz.h );
	RectLTRB texrect = mod->texRect;
	SizeWH vTexSz = texrect.Size();
	RectLTRB finaltex( texrect.left + posCornerULPerc.x * vTexSz.w, texrect.top + posCornerULPerc.y * vTexSz.h,
		texrect.left + posCornerDRPerc.x * vTexSz.w, texrect.top + posCornerDRPerc.y * vTexSz.h );
	// paints without offset because the offset is already in the clipped rectangle
	__Painter().Draw( mod->pImg->pTex,
		finaltex,
		intersection,
		Vec2( 0.0f, 0.0f ), ncolor );
}

void UTSprite::PaintFModuleStretched( CSpriteLib *sprCol, Vec2 vPos, int animID, int frameIdx, int moduleIdx, DWORD ncolor, float W, float H )
{
	if ( W == 0.0f || H == 0.0f )
		return;

	int aframeIdx = sprCol->Animations[ animID ]->aframesIdx[ frameIdx ];
	scFModule* mod = sprCol->FModules[ sprCol->AFrames[ aframeIdx ]->fmodulesIdx[ moduleIdx ] ];
	RectLTRB destrect = mod->moduleRectOff;

	if ( W > 0.0f )
		destrect.right = destrect.left + W;
	if ( H > 0.0f )
		destrect.bottom = destrect.top + H;

	__Painter().Draw( mod->pImg->pTex,
		mod->texRect,
		destrect,
		vPos, ncolor );
}

void UTSprite::PaintFModuleTiled( CSpriteLib *sprCol, Vec2 vPos, int animID, int frameIdx, int moduleIdx, DWORD ncolor /*= 0xffffffff*/, float W /*= -1.0f*/, float H /*= -1.0f */ )
{
	if ( ( W == 0.0f ) || ( H == 0.0f ) )
		return;
		RectXYWH sprrect = sprCol->GetAFrameBBox_real( animID, frameIdx );
		RectXYWH destrect = sprrect;
		if ( W > 0.0f )
			destrect.w = W;
		if ( H > 0.0f )
			destrect.h = H;

		int intsX = destrect.w / sprrect.w;
		int intsY = destrect.h / sprrect.h;
		int restW = destrect.w - ( intsX * sprrect.w );
		int restH = destrect.h - ( intsY * sprrect.h );

		RectLTRB destrectRC( destrect.x + vPos.x, destrect.y + vPos.y, destrect.x + vPos.x + destrect.w, destrect.y + vPos.y + destrect.h );
		Vec2 vCur = vPos;
		if ( ( intsX <= 0 ) && ( intsY <= 0 ) )
		{
			PaintFModuleClipped( sprCol, vPos, animID, frameIdx, moduleIdx, destrectRC, ncolor );
		}
		else
		{
			Vec2 vOrig = vPos;
			//deseneaza partile intregi
			for ( int xx = 0; xx < intsX; xx++ )
			{
				for ( int yy = 0; yy < intsY; yy++ )
				{
					vCur.x = vOrig.x + xx * sprrect.w;
					vCur.y = vOrig.y + yy * sprrect.h;
					PaintFModule( sprCol, vCur, animID, frameIdx, moduleIdx, ncolor );
				}
			}
			//daca avem si clip
			if ( restW > 0 )
			{
				vCur.x = vOrig.x + intsX * sprrect.w;
				vCur.y = vOrig.y;
				for ( int yy = 0; yy < intsY; yy++ )
				{
					PaintFModuleClipped( sprCol, vCur, animID, frameIdx, moduleIdx, destrectRC, ncolor );
					vCur.y += sprrect.h;
				}
			}
			if ( restH > 0 )
			{
				vCur.x = vOrig.x;
				vCur.y = vOrig.y + intsY * sprrect.h;
				for ( int xx = 0; xx <= intsX; xx++ )
				{
					PaintFModuleClipped( sprCol, vCur, animID, frameIdx, moduleIdx, destrectRC, ncolor );
					vCur.x += sprrect.w;
				}
			}

			vCur = vOrig;
		}
}

void UTSprite::PaintFrame( CSpriteLib *sprCol, Vec2 vPos, int animID, int frameIdx, DWORD ncolor /*= 0xffffffff*/ )
{
	_ASSERT( animID < sprCol->Animations.Count() );
	_ASSERT( frameIdx < sprCol->Animations[ animID ]->aframesNo );
	//#TODO: no flip flags were taken into account
	int aframeIdx = sprCol->Animations[ animID ]->aframesIdx[ frameIdx ];
	for ( int ii = 0; ii < sprCol->AFrames[ aframeIdx ]->fmodulesNo; ii++ )
	{
		int fmoduleIdx = sprCol->AFrames[ aframeIdx ]->fmodulesIdx[ ii ];
		scFModule* mod = sprCol->FModules[ fmoduleIdx ];

		__Painter().Draw( mod->pImg->pTex,
			mod->texRect,
			mod->moduleRectOff,
			vPos, ncolor );
	}
}

void UTSprite::PaintFrame( CSpriteLib *sprCol, float posX, float posY, int animID, int frameIdx, DWORD ncolor /*= 0xffffffff*/ )
{
	_ASSERT( animID < sprCol->Animations.Count() );
	_ASSERT( frameIdx < sprCol->Animations[ animID ]->aframesNo );
	//#TODO: no flip flags were taken into account
	int aframeIdx = sprCol->Animations[ animID ]->aframesIdx[ frameIdx ];
	for ( int ii = 0; ii < sprCol->AFrames[ aframeIdx ]->fmodulesNo; ii++ )
	{
		int fmoduleIdx = sprCol->AFrames[ aframeIdx ]->fmodulesIdx[ ii ];
		scFModule* mod = sprCol->FModules[ fmoduleIdx ];

		__Painter().Draw( mod->pImg->pTex,
			mod->texRect,
			mod->moduleRectOff,
			Vec2(posX, posY), ncolor );
	}
}
