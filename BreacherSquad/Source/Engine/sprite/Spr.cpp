#include "dxstdafx.h"
#include "Spr.h"

///--- STATIC MEMBERS ---
CSpritePainter* CSpr::s_pSP = &__Painter();

void CSpr::Reset()
{
	curFrameEvent = 0;
	animIdx = 0;
	pos.x = 0.0f;
	pos.y = 0.0f;
	fTime = 0.0f;
	frameIdx = 0;
	animStatus = ANIM_JUST_STARTED;
	color = 0xffffffff;
	scale = Vec2( 1.0f, 1.0f );
	rotation = 0.0f;
	fTimeScale = 1.0f;
	animDirection = 1;
}

CSpr::CSpr() 
{
	Reset();
	pSprCol = nullptr;
}

CSpr::CSpr(CSpriteLib* pSpriteColl, int animIdx, float pX, float pY)
{
	Reset();
	pSprCol = pSpriteColl;
	animIdx = animIdx;
	pos.x = pX;
	pos.y = pY;
}

CSpr::CSpr(CSpriteLib* pSpriteColl, int animIdx, Vec2 vPos)
{
	Reset();
	pSprCol = pSpriteColl;
	animIdx = animIdx;
	pos = vPos;
}

CSpr::CSpr(const CSpr& sprite)
{
	curFrameEvent = sprite.curFrameEvent;
	pSprCol = sprite.pSprCol;
	animIdx = sprite.animIdx;
	pos = sprite.pos;
	fTime = sprite.fTime;
	frameIdx = sprite.frameIdx;
	animStatus = sprite.animStatus;
	color = sprite.color;
	scale = sprite.scale;
	rotation = sprite.rotation;
	fTimeScale = sprite.fTimeScale;
	animDirection = sprite.animDirection;
}

void CSpr::Init(CSpriteLib *sprCollection, int nAnimIdx, Vec2 vPos, int nframeIdx, DWORD nColor, float fRotation, Vec2 vScale)
{
	Reset();
	pSprCol = sprCollection;
	animIdx = nAnimIdx;
	frameIdx = nframeIdx;
	pos = vPos;
	color = nColor;
	scale = vScale;
	rotation = fRotation;
}

void CSpr::Init(CSpriteLib *sprCollection, CHAR* strAnimID, Vec2 vPos, int nframeIdx, DWORD nColor, float fRotation, Vec2 vScale)
{
	int newAnim = sprCollection->GetAnimationIdxByName( strAnimID );
	if (newAnim < 0)
	{
		LOG("Sprite::Init: Animation [%s] not found!", strAnimID);
		return;
	}

	Reset();
	pSprCol = sprCollection;
	animIdx = newAnim;
	frameIdx = nframeIdx;
	pos = vPos;
	color = nColor;
	rotation = fRotation;
	scale = vScale;
}

void CSpr::SetAnim(int nAnimIdx, int nFrameIdx)
{
	animIdx = nAnimIdx;
	curFrameEvent = 0;
	fTime = 0.0f;
	fTimeScale = 1.0f;
	animDirection = 1;
	frameIdx = nFrameIdx;
	animStatus = ANIM_JUST_STARTED;
}

void CSpr::SetFrame(int nFrameIdx)
{
	frameIdx = nFrameIdx;
	curFrameEvent = 0;
	fTime = 0.0f;
}

bool CSpr::SetAnimOnce(int nAnimIdx, int nFrameIdx) 
{
	if((animIdx == nAnimIdx) || (nAnimIdx < 0))
		return false;

	animIdx = nAnimIdx;

	curFrameEvent = 0;
	fTime = 0.0f;
	fTimeScale = 1.0f;
	animDirection = 1;
	frameIdx = nFrameIdx;
	animStatus = ANIM_JUST_STARTED;

	return true;
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

	curFrameEvent = 0;
	fTime = 0.0f;
	fTimeScale = 1.0f;
	animDirection = 1;
	frameIdx = nFrameIdx;
	animStatus = ANIM_JUST_STARTED;
}

void CSpr::Update(float dTime, bool updatePos)
{
	curFrameEvent = 0;

	_ASSERT(animIdx < pSprCol->Animations.Count());
	_ASSERT(frameIdx < pSprCol->Animations[animIdx]->aframesNo);

	if (animStatus == ANIM_FRAMELOCK)
		return;

	int aframeID = pSprCol->Animations[animIdx]->aframesIdx[frameIdx];
	// return frame flag for animations that were just set now
	if (animStatus == ANIM_JUST_STARTED)
	{
		//#TODO: if first frame is too short the message gets lost retAFrameFlag
		curFrameEvent = pSprCol->AFrames[aframeID]->flags;
	}

	animStatus = ANIM_PLAYING;
	fTime += dTime * fTimeScale;

	if( fTime >= float(pSprCol->AFrames[aframeID]->duration) / SPR_ED_TIMELINE )
	{
		animStatus = ANIM_PLAYING_FRAME_ADVANCED;
		if( updatePos )
		{
			pos.x += pSprCol->AFrames[aframeID]->dx;
			pos.y += pSprCol->AFrames[aframeID]->dy;
		}

		fTime -= float(pSprCol->AFrames[aframeID]->duration) / SPR_ED_TIMELINE;
		// animDir must always be -1 or 1
		frameIdx += animDirection;
		bool bReachedEnd = false;
		if ( ((animDirection > 0) && (frameIdx >= pSprCol->Animations[ animIdx ]->aframesNo)) ||
			((animDirection < 0) && (frameIdx < 0)) ) {
			bReachedEnd = true;
		}

		if (bReachedEnd) 
		{
			if ( (pSprCol->Animations[animIdx]->flags & ANIMATION_FLAG_LOOPED) == 0 ) 
			{ //play once?
				// return on last frame and stop
				frameIdx -= animDirection; 
				animStatus = ANIM_FRAMELOCK; 
				//get new flag
				aframeID = pSprCol->Animations[animIdx]->aframesIdx[frameIdx];
			}
			else 
			{ //looping (depending on direction of playback)
				frameIdx = (animDirection > 0) ? 0 : pSprCol->Animations[ animIdx ]->aframesNo - 1;
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
		curFrameEvent = pSprCol->AFrames[aframeID]->flags;
	}
}

void CSpr::Paint()
{
	_ASSERT(animIdx < pSprCol->Animations.Count());
	_ASSERT(frameIdx < pSprCol->Animations[animIdx]->aframesNo);
	// editor flip flags not considered yet, probably not needed
	int aframeIdx = pSprCol->Animations[animIdx]->aframesIdx[frameIdx];
	for( int ii = 0; ii < pSprCol->AFrames[aframeIdx]->fmodulesNo; ii++ )
	{
		scFModule* mod = pSprCol->FModules[pSprCol->AFrames[aframeIdx]->fmodulesIdx[ii]];
		
		s_pSP->Draw(mod->pImg->pTex,
			mod->texRect,
			mod->moduleRectOff,
			pos,
			color,
			rotation, scale);
	}
}

void CSpr::Paint( RectLTRB& clip )
{
	// clipped paint doesn't handle rotation and scaling
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

void CSpr::PaintEx( UINT texFlipFlags )
{
	_ASSERT( animIdx < pSprCol->Animations.Count() );
	_ASSERT( frameIdx < pSprCol->Animations[ animIdx ]->aframesNo );
	// editor flip flags not considered yet, probably not needed
	int aframeIdx = pSprCol->Animations[ animIdx ]->aframesIdx[ frameIdx ];
	for ( int ii = 0; ii < pSprCol->AFrames[ aframeIdx ]->fmodulesNo; ii++ )
	{
		scFModule* mod = pSprCol->FModules[ pSprCol->AFrames[ aframeIdx ]->fmodulesIdx[ ii ] ];

		s_pSP->DrawEx( mod->pImg->pTex,
			mod->texRect,
			mod->moduleRectOff,
			pos,
			color,
			rotation, scale, 
			texFlipFlags );
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

void CSpr::PaintFModule_texOverride(int moduleIdx, int texIdxOffset)
{
	_ASSERT(animIdx < pSprCol->Animations.Count());
	_ASSERT(frameIdx < pSprCol->Animations[animIdx]->aframesNo);
	int aframeIdx = pSprCol->Animations[animIdx]->aframesIdx[frameIdx];
	_ASSERT(moduleIdx < pSprCol->AFrames[aframeIdx]->fmodulesNo);

	scFModule* mod = pSprCol->FModules[pSprCol->AFrames[aframeIdx]->fmodulesIdx[moduleIdx]];

	s_pSP->Draw(pSprCol->Textures[mod->imgIdx + texIdxOffset]->pTex, mod->texRect, mod->moduleRectOff, pos, color, rotation, scale);
}

void CSpr::Stop()
{
	animStatus = ANIM_FRAMELOCK;
}

void CSpr::ScaleAnimTime( float target_duration_sec )
{
	_ASSERT( animIdx < pSprCol->Animations.Count() );
	float fAnimDuration = 0.0f;

	for ( int kk = 0; kk < pSprCol->Animations[ animIdx ]->aframesNo; kk++ )
	{
		int aframeID = pSprCol->Animations[ animIdx ]->aframesIdx[ frameIdx ];
		fAnimDuration += float( pSprCol->AFrames[ aframeID ]->duration ) / SPR_ED_TIMELINE;
	}
	// scale total duration by changing the time multiplier
	fTimeScale = fAnimDuration / target_duration_sec;
}

void CSpr::SetAnimSpeed( float time_multiplier )
{
	fTimeScale = time_multiplier;
}

void CSpr::SetAnimDirection( bool bReverseAnimation, bool bRewind /*= false */ )
{
	animDirection = (bReverseAnimation == true) ? -1 : 1;
	if ( bRewind )
	{
		if ( bReverseAnimation )
			frameIdx = pSprCol->Animations[ animIdx ]->aframesNo - 1;
		else
			frameIdx = 0;
	}
}

void CSpr::Play( bool bReset /*= false */ )
{
	animStatus = ANIM_JUST_STARTED;
	if ( bReset )
		frameIdx = 0;
}

RectXYWHi CSpr::GetAFrameBBox()
{
	_ASSERT( pSprCol != nullptr );
	return pSprCol->GetAFrameBBox( animIdx, frameIdx );
}

UINT32 CSpr::GetAframeFlags()
{
	_ASSERT( pSprCol != nullptr );
	return pSprCol->GetAFrameFlags( animIdx, frameIdx );
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

void UTSprite::PaintFrameClipped( CSpriteLib *sprCol, Vec2 vPos, int animID, int frameIdx, RectXYWH& clipr, DWORD ncolor /*= 0xffffffff */ )
{
	RectLTRB clip(clipr);
	PaintFrameClipped( sprCol, vPos, animID, frameIdx, clip, ncolor );
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
