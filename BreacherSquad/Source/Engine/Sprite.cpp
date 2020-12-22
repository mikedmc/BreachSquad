#include "dxstdafx.h"
#include "Sprite.h"

//--- durata din INK EDITOR se da in sutimi de secunde (100 = 1 sec) deocamdata -----
#define INKED_TIMELINE 100.0f

///--- STATIC MEMBERS ---
ID3DXSprite* CSprite::s_pSprite = NULL;
CSpritePainter* CSprite::s_pSP = nullptr;

void CSprite::SetGlobalSpritePtr(ID3DXSprite* pSprite, CSpritePainter* pSP)
{
	s_pSprite = pSprite;
	s_pSP = pSP;
}
///--- end STATIC ---


DWORD GetFlags(CSpriteCollection* sprCol, int anim, int frame)
{
	if (anim < 0 || frame < 0) return (DWORD)0;
	int frameIdx = sprCol->Animations[anim]->aframesIdx[frame];
	return sprCol->AFrames[frameIdx]->flags;
}


CSprite::CSprite() 
{
	animationIdx = 0;
	pos.x = 0.0f;
	pos.y = 0.0f;
	timePassed = 0.0f;
	currentFrame = 0;
	animStatus = ANIM_STATUS_JUST_STARTED;
	color=0xffffffff;
}

CSprite::CSprite(int animIdx, float pX, float pY) 
{
	animationIdx = animIdx;
	pos.x = pX;
	pos.y = pY;
	timePassed = 0.0f;
	currentFrame = 0;
	animStatus = ANIM_STATUS_JUST_STARTED;
	color=0xffffffff;
}

CSprite::CSprite(int animIdx, D3DXVECTOR2 vPos)
{
	animationIdx = animIdx;
	pos = vPos;
	timePassed = 0.0f;
	currentFrame = 0;
	animStatus = ANIM_STATUS_JUST_STARTED;
	color = 0xffffffff;
}

CSprite::CSprite(const CSprite& sprite)
{
	animationIdx = sprite.animationIdx;
	pos.x = sprite.pos.x; pos.y = sprite.pos.y;
	timePassed = sprite.timePassed;
	currentFrame = sprite.currentFrame;
	animStatus = sprite.animStatus;
	color = sprite.color;
}

void CSprite::Init(int animIdx, float pX, float pY, int nframeIdx, DWORD nColor)
{
	animationIdx = animIdx;
	pos.x = pX;
	pos.y = pY;
	timePassed = 0.0f;
	currentFrame = nframeIdx;
	animStatus = ANIM_STATUS_JUST_STARTED;
	color = nColor;
}

void CSprite::Init(CHAR* animID, CSpriteCollection *sprCollection, float pX, float pY, int nframeIdx, DWORD nColor)
{
	animationIdx = sprCollection->getAnimationIdxByName(animID);
	assert((animationIdx >= 0) && (animationIdx < sprCollection->animationNo));

	pos.x = pX;
	pos.y = pY;
	timePassed = 0.0f;
	currentFrame = nframeIdx;
	animStatus = ANIM_STATUS_JUST_STARTED;
	color = nColor;
}

void CSprite::setAnimation(int animIdx, int frameIdx)
{
	animationIdx = animIdx;

	timePassed = 0.0f;
	currentFrame = frameIdx;
	animStatus = ANIM_STATUS_JUST_STARTED;
}

void CSprite::SetFrame(int nFrameIdx)
{
	currentFrame = nFrameIdx;
	timePassed = 0.0f;
	animStatus = ANIM_STATUS_PLAYING;
}

//seteaza animatia doar daca e alta
void CSprite::setAnimationOnce(int animIdx, int frameIdx) 
{
	if((animIdx == animationIdx) || (animIdx < 0))
		return;

	animationIdx = animIdx;

	timePassed = 0.0f;
	currentFrame = frameIdx;
	animStatus = ANIM_STATUS_JUST_STARTED;
}

void CSprite::setAnimationOnce_keepFrame(CSpriteCollection *sprCol, int animIdx)
{
	if (animIdx == animationIdx)
		return;

	animationIdx = animIdx;

	CLAMP(currentFrame, 0, sprCol->Animations[animIdx]->aframesNo - 1);
	timePassed = 0.0f;

	animStatus = ANIM_STATUS_JUST_STARTED;
}


void CSprite::setAnimation(CHAR* animID, CSpriteCollection *sprCollection)
{
	animationIdx = sprCollection->getAnimationIdxByName(animID);
	assert((animationIdx >= 0) && (animationIdx < sprCollection->animationNo));

	timePassed = 0.0f;
	currentFrame = 0;
	animStatus = ANIM_STATUS_JUST_STARTED;
}

//face update la pozitie numai daca e true updatePos
UINT32 CSprite::Update(CSpriteCollection *sprCollection, float dTime, bool updatePos)
{
	UINT32 retAFrameFlag = 0;

	assert(animationIdx < sprCollection->Animations.Count());
	assert(currentFrame < sprCollection->Animations[animationIdx]->aframesNo);

	if (animStatus == ANIM_STATUS_FRAMELOCK)
		return retAFrameFlag;

	int aframeID = sprCollection->Animations[animationIdx]->aframesIdx[currentFrame];
	//pentru animatiile abia setate imi intoarce aframe flag-ul frame-ului curent
	if (animStatus == ANIM_STATUS_JUST_STARTED)
	{
		//TODO: daca primul frame e foarte scurt pierde primul mesaj retAFrameFlag
		retAFrameFlag = sprCollection->AFrames[aframeID]->flags;
	}

	animStatus = ANIM_STATUS_PLAYING;
	timePassed += dTime;

	if( timePassed >= float(sprCollection->AFrames[aframeID]->duration) / INKED_TIMELINE )
	{
		//a avansat frame. Modifica si pozitia. Rezulta de aici ca modificarea pozitiei o face la sfarsitul frame-ului.
		//oricum ar trebui interpolat spline intre pozitii asa ca momentan e bine asa.
		animStatus = ANIM_STATUS_PLAYING_FRAME_ADVANCED;
		if( updatePos )
		{
			pos.x += sprCollection->AFrames[aframeID]->dx;
			pos.y += sprCollection->AFrames[aframeID]->dy;
		}

		timePassed -= float(sprCollection->AFrames[aframeID]->duration) / INKED_TIMELINE;
		currentFrame++;

		if (currentFrame >= sprCollection->Animations[animationIdx]->aframesNo) 
		{
			if ( (sprCollection->Animations[animationIdx]->flags & ANIMATION_FLAG_LOOPED) == 0 )
			{ //daca e play once
				currentFrame--; //pozitioneaza pe ultimul frame
				animStatus = ANIM_STATUS_FRAMELOCK; //face lock pe ultimul frame
				//get new flag
				aframeID = sprCollection->Animations[animationIdx]->aframesIdx[currentFrame];
			}
			else 
			{ //daca e looping
				currentFrame = 0;
				animStatus = ANIM_STATUS_LOOPRESET;
				//get new flag
				aframeID = sprCollection->Animations[animationIdx]->aframesIdx[currentFrame];
			}
		}
		else
		{
			aframeID = sprCollection->Animations[animationIdx]->aframesIdx[currentFrame];
		}
		//set aframe flag for return
		retAFrameFlag = sprCollection->AFrames[aframeID]->flags;
	}
	return retAFrameFlag;
}

void CSprite::paint(CSpriteCollection* sprCol)
{
	assert(animationIdx < sprCol->Animations.Count());
	assert(currentFrame < sprCol->Animations[animationIdx]->aframesNo);
	//nu am luat in considerare inca flagsurile
	int aframeIdx = sprCol->Animations[animationIdx]->aframesIdx[currentFrame];
	for( int ii = 0; ii < sprCol->AFrames[aframeIdx]->fmodulesNo; ii++ )
	{
		int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[ii];
		//deseneaza
		s_pSprite->Draw(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx]->pTex, 
			&sprCol->FModules[fmoduleIdx]->moduleRect, 
			NULL, 
			&D3DXVECTOR3(pos.x + sprCol->FModules[fmoduleIdx]->ox, pos.y + sprCol->FModules[fmoduleIdx]->oy, 0.0f),
			color);
	}
}

void CSprite::paint_firstModule(CSpriteCollection* sprCol)
{
	assert(animationIdx < sprCol->Animations.Count());
	assert(currentFrame < sprCol->Animations[animationIdx]->aframesNo);
	int aframeIdx = sprCol->Animations[animationIdx]->aframesIdx[currentFrame];
	assert(sprCol->AFrames[aframeIdx]->fmodulesNo == 1); 

	int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[0]; //!!! only first module
	//deseneaza
	s_pSprite->Draw(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx]->pTex,
		&sprCol->FModules[fmoduleIdx]->moduleRect,
		NULL,
		&D3DXVECTOR3(pos.x + sprCol->FModules[fmoduleIdx]->ox, pos.y + sprCol->FModules[fmoduleIdx]->oy, 0.0f),
		color);
}

void CSprite::paint_firstModuleColorized(CSpriteCollection *sprCol, DWORD dwCol)
{
	assert(animationIdx < sprCol->Animations.Count());
	assert(currentFrame < sprCol->Animations[animationIdx]->aframesNo);
	int aframeIdx = sprCol->Animations[animationIdx]->aframesIdx[currentFrame];
	assert(sprCol->AFrames[aframeIdx]->fmodulesNo == 1);

	int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[0]; //!!! only first module
	s_pSprite->Draw(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx]->pTex,
		&sprCol->FModules[fmoduleIdx]->moduleRect,
		NULL,
		&D3DXVECTOR3(pos.x + sprCol->FModules[fmoduleIdx]->ox, pos.y + sprCol->FModules[fmoduleIdx]->oy, 0.0f),
		dwCol);
}

void CSprite::paint_texOverride(CSpriteCollection *sprCol, int texIdxOffset, float fOffX /*= 0.0f*/, float fOffy /*= 0.0f*/)
{
	assert(animationIdx < sprCol->Animations.Count());
	assert(currentFrame < sprCol->Animations[animationIdx]->aframesNo);
	//nu am luat in considerare inca flagsurile
	int aframeIdx = sprCol->Animations[animationIdx]->aframesIdx[currentFrame];
	for (int ii = 0; ii < sprCol->AFrames[aframeIdx]->fmodulesNo; ii++)
	{
		int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[ii];
		//deseneaza
		s_pSprite->Draw(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx + texIdxOffset]->pTex,
			&sprCol->FModules[fmoduleIdx]->moduleRect,
			NULL,
			&D3DXVECTOR3(fOffX + pos.x + sprCol->FModules[fmoduleIdx]->ox, fOffy + pos.y + sprCol->FModules[fmoduleIdx]->oy, 0.0f),
			color);
	}
}

void CSprite::paint_firstModule_texOverride(CSpriteCollection *sprCol, int texIdxOffset, float fOffX /*= 0.0f*/, float fOffy /*= 0.0f*/)
{
	assert(animationIdx < sprCol->Animations.Count());
	assert(currentFrame < sprCol->Animations[animationIdx]->aframesNo);
	//nu am luat in considerare inca flagsurile
	int aframeIdx = sprCol->Animations[animationIdx]->aframesIdx[currentFrame];
	assert(sprCol->AFrames[aframeIdx]->fmodulesNo == 1);

	int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[0]; //!!! doar primul modul - optimizare viteza
	//deseneaza
	s_pSprite->Draw(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx + texIdxOffset]->pTex,
		&sprCol->FModules[fmoduleIdx]->moduleRect,
		NULL,
		&D3DXVECTOR3(fOffX + pos.x + sprCol->FModules[fmoduleIdx]->ox, fOffy + pos.y + sprCol->FModules[fmoduleIdx]->oy, 0.0f),
		color);
}



void CSprite::paintFlippedInPlace(CSpriteCollection *sprCol, bool flipX, bool flipY)
{
	assert(animationIdx < sprCol->Animations.Count());
	assert(currentFrame < sprCol->Animations[animationIdx]->aframesNo);
	//nu am luat in considerare inca flagsurile
	int aframeIdx = sprCol->Animations[animationIdx]->aframesIdx[currentFrame];
	for (int ii = 0; ii < sprCol->AFrames[aframeIdx]->fmodulesNo; ii++)
	{
		int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[ii];
		RECT modrect = sprCol->FModules[fmoduleIdx]->moduleRect;
		if (flipX)
			SWAP(modrect.left, modrect.right);
		if (flipY)
			SWAP(modrect.top, modrect.bottom);
		//deseneaza
		s_pSprite->Draw(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx]->pTex,
			&modrect,
			NULL,
			&D3DXVECTOR3(pos.x + sprCol->FModules[fmoduleIdx]->ox, pos.y + sprCol->FModules[fmoduleIdx]->oy, 0.0f),
			color);
	}
}


void CSprite::paintFrame(CSpriteCollection *sprCol, float nX, float nY, int animID, int frameID, DWORD ncolor)
{
	assert(animID < sprCol->Animations.Count());
	assert(frameID < sprCol->Animations[animID]->aframesNo);
	//nu am luat in considerare inca flagsurile
	int aframeIdx = sprCol->Animations[animID]->aframesIdx[frameID];
	for( int ii = 0; ii < sprCol->AFrames[aframeIdx]->fmodulesNo; ii++ )
	{
		int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[ii];
		//deseneaza
		s_pSprite->Draw(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx]->pTex, 
			&sprCol->FModules[fmoduleIdx]->moduleRect, 
			NULL, 
			&D3DXVECTOR3(nX + sprCol->FModules[fmoduleIdx]->ox, nY + sprCol->FModules[fmoduleIdx]->oy, 0.0f),
			ncolor);
	}
}

void CSprite::paintFrameModule(CSpriteCollection *sprCol, float nX, float nY, int animID, int frameID, int moduleID, DWORD ncolor)
{
	assert(animID < sprCol->Animations.Count());
	assert(frameID < sprCol->Animations[animID]->aframesNo);
	assert(moduleID < sprCol->AFrames[sprCol->Animations[animID]->aframesIdx[frameID]]->fmodulesNo);

	//nu am luat in considerare inca flagsurile
	int aframeIdx = sprCol->Animations[animID]->aframesIdx[frameID];
	int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[moduleID];
	//deseneaza
	s_pSprite->Draw(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx]->pTex, 
		&sprCol->FModules[fmoduleIdx]->moduleRect, 
		NULL, 
		&D3DXVECTOR3(nX + sprCol->FModules[fmoduleIdx]->ox, nY + sprCol->FModules[fmoduleIdx]->oy, 0.0f),
		ncolor);
}


void CSprite::paintFrameClipped(CSpriteCollection *sprCol, float nX, float nY, int animID, int frameID, RECTXYWH& clipRct, DWORD ncolor)
{
	assert(animID < sprCol->Animations.Count());
	assert(frameID < sprCol->Animations[animID]->aframesNo);

	RECT cliprect;
	if ((clipRct.x + clipRct.w <= clipRct.x) || (clipRct.y + clipRct.h <= clipRct.y))
		return;

	SetRect(&cliprect, clipRct.x, clipRct.y, clipRct.x + clipRct.w, clipRct.y + clipRct.h);

	//nu am luat in considerare inca flagsurile
	int aframeIdx = sprCol->Animations[animID]->aframesIdx[frameID];
	for (int ii = 0; ii < sprCol->AFrames[aframeIdx]->fmodulesNo; ii++)
	{
		int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[ii];
		RECT srcrect = sprCol->FModules[fmoduleIdx]->moduleRect;
		D3DXVECTOR3 drawpos(nX + sprCol->FModules[fmoduleIdx]->ox, nY + sprCol->FModules[fmoduleIdx]->oy, 0.0f);

		RECT destrect;
		destrect.left = (int)(nX + sprCol->FModules[fmoduleIdx]->ox);
		destrect.top = (int)(nY + sprCol->FModules[fmoduleIdx]->oy);
		destrect.right = destrect.left + srcrect.right - srcrect.left;
		destrect.bottom = destrect.top + srcrect.bottom - srcrect.top;
		drawpos = D3DXVECTOR3(destrect.left, destrect.top, 0.0f);

		//vede daca iese detot din dreptunghiul de clip
		if ((destrect.left > cliprect.right) || (destrect.top > cliprect.bottom) || (destrect.bottom < cliprect.top) || (destrect.right < cliprect.left))
			continue;
		//face clip
		if (destrect.right > cliprect.right)
		{
			srcrect.right -= destrect.right - cliprect.right;
		}
		if (destrect.bottom > cliprect.bottom)
		{
			srcrect.bottom -= destrect.bottom - cliprect.bottom;
		}
		if (destrect.left < cliprect.left)
		{
			srcrect.left += cliprect.left - destrect.left;
			drawpos.x += cliprect.left - destrect.left;
		}
		if (destrect.top < cliprect.top)
		{
			srcrect.top += cliprect.top - destrect.top;
			drawpos.y += cliprect.top - destrect.top;
		}

		//deseneaza
		s_pSprite->Draw(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx]->pTex,
			&srcrect,
			NULL,
			&drawpos,
			ncolor);
	}
}

void CSprite::paintFrameModuleClipped(CSpriteCollection *sprCol, float nX, float nY, int animID, int frameID, int moduleID, RECTXYWH_F * clipRct, DWORD ncolor)
{
	assert(animID < sprCol->Animations.Count());
	assert(frameID < sprCol->Animations[animID]->aframesNo);
	assert(moduleID < sprCol->AFrames[sprCol->Animations[animID]->aframesIdx[frameID]]->fmodulesNo);

	//nu am luat in considerare inca flagsurile
	int aframeIdx = sprCol->Animations[animID]->aframesIdx[frameID];
	int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[moduleID];

	RECT srcrect = sprCol->FModules[fmoduleIdx]->moduleRect;
	D3DXVECTOR3 drawpos(nX + sprCol->FModules[fmoduleIdx]->ox, nY + sprCol->FModules[fmoduleIdx]->oy, 0.0f);

	if (clipRct != NULL)
	{
		RECTLTRB_F cliprect(clipRct->x, clipRct->y, clipRct->x + clipRct->w, clipRct->y + clipRct->h);
		if ((cliprect.right <= cliprect.left) || (cliprect.bottom <= cliprect.top))
			return;

		RECTLTRB_F destrect;

		destrect.left = (nX + sprCol->FModules[fmoduleIdx]->ox);
		destrect.top = (nY + sprCol->FModules[fmoduleIdx]->oy);
		destrect.right = destrect.left + srcrect.right - srcrect.left;
		destrect.bottom = destrect.top + srcrect.bottom - srcrect.top;
		drawpos = D3DXVECTOR3(destrect.left, destrect.top, 0.0f);

		//vede daca iese detot din dreptunghiul de clip
		if ((destrect.left > cliprect.right) || (destrect.top > cliprect.bottom) || (destrect.bottom < cliprect.top) || (destrect.right < cliprect.left))
			return;

		//face clip
		if (destrect.right > cliprect.right)
		{
			srcrect.right -= ROUND_FLOAT(destrect.right - cliprect.right);
		}
		if (destrect.bottom > cliprect.bottom)
		{
			srcrect.bottom -= ROUND_FLOAT(destrect.bottom - cliprect.bottom);
		}
		if (destrect.left < cliprect.left)
		{
			srcrect.left += cliprect.left - destrect.left;
			drawpos.x += cliprect.left - destrect.left;
		}
		if (destrect.top < cliprect.top)
		{
			srcrect.top += cliprect.top - destrect.top;
			drawpos.y += cliprect.top - destrect.top;
		}
	}

	//deseneaza
	s_pSprite->Draw(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx]->pTex,
		&srcrect,
		NULL,
		&drawpos,
		ncolor);
}

void CSprite::paintFrameModuleTiled(CSpriteCollection *sprCol, float X, float Y, int animID, int frameID, int moduleID, DWORD ncolor, int W, int H)
{
	RECTXYWH sprrect = sprCol->GetAFrameBBox_real(animID, frameID);
	RECTXYWH destrect = sprrect;
	if (W >= 0)
		destrect.w = W;
	if (H >= 0)
		destrect.h = H;

	int intsX = destrect.w / sprrect.w;
	int intsY = destrect.h / sprrect.h;
	int restW = destrect.w - (intsX * sprrect.w);
	int restH = destrect.h - (intsY * sprrect.h);

	RECTXYWH_F destrectRC(destrect.x + X, destrect.y + Y, destrect.w, destrect.h);

	if ((intsX <= 0) && (intsY <= 0))
	{
		paintFrameModuleClipped(sprCol, X, Y, animID, frameID, moduleID, &destrectRC, ncolor);
	}
	else
	{
		float origX = X, origY = Y;
		//deseneaza partile intregi
		for (int xx = 0; xx < intsX; xx++)
		{
			for (int yy = 0; yy < intsY; yy++)
			{
				X = origX + xx * sprrect.w;
				Y = origY + yy * sprrect.h;
				paintFrameModule(sprCol, X, Y, animID, frameID, moduleID, ncolor);
			}
		}
		//daca avem si clip
		if (restW > 0)
		{
			X = origX + intsX * sprrect.w;
			Y = origY;
			for (int yy = 0; yy < intsY; yy++)
			{
				paintFrameModuleClipped(sprCol, X, Y, animID, frameID, moduleID, &destrectRC, ncolor);
				Y += sprrect.h;
			}
		}
		if (restH > 0)
		{
			X = origX;
			Y = origY + intsY * sprrect.h;
			for (int xx = 0; xx <= intsX; xx++)
			{
				paintFrameModuleClipped(sprCol, X, Y, animID, frameID, moduleID, &destrectRC, ncolor);
				X += sprrect.w;
			}
		}

		X = origX; Y = origY;
	}
}

void CSprite::paint(CSpriteCollection *sprCol, RECT *cliprect)
{
	if((cliprect->right <= cliprect->left) || (cliprect->bottom <= cliprect->top))
		return;
	int aframeIdx = sprCol->Animations[animationIdx]->aframesIdx[currentFrame];
	for( int ii = 0; ii < sprCol->AFrames[aframeIdx]->fmodulesNo; ii++ )
	{
		int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[ii];
		//face clip
		RECT srcrect = sprCol->FModules[fmoduleIdx]->moduleRect;
		RECT destrect;

		destrect.left = (int)(pos.x + sprCol->FModules[fmoduleIdx]->ox);
		destrect.top = (int)(pos.y + sprCol->FModules[fmoduleIdx]->oy);
		destrect.right = destrect.left + srcrect.right - srcrect.left;
		destrect.bottom = destrect.top + srcrect.bottom - srcrect.top;
		D3DXVECTOR3 drawpos(destrect.left, destrect.top, 0.0f);

		//vede daca iese detot din dreptunghiul de clip
		if((destrect.left > cliprect->right) || (destrect.top > cliprect->bottom) || (destrect.bottom < cliprect->top) || (destrect.right < cliprect->left))
			continue;
		//face clip
		if(destrect.right > cliprect->right)
		{
			srcrect.right -= destrect.right - cliprect->right;
		}
		if(destrect.bottom > cliprect->bottom)
		{
			srcrect.bottom -= destrect.bottom - cliprect->bottom;
		}
		if(destrect.left < cliprect->left)
		{
			srcrect.left += cliprect->left - destrect.left;
			drawpos.x += cliprect->left - destrect.left;
		}
		if(destrect.top < cliprect->top)
		{
			srcrect.top += cliprect->top - destrect.top;
			drawpos.y += cliprect->top - destrect.top;
		}

		//deseneaza
		s_pSprite->Draw(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx]->pTex, 
			&srcrect, 
			NULL, 
			&drawpos,
			color);
	}
}

void CSprite::paint(CSpriteCollection *sprCol, RECTXYWH *clipRct)
{
	RECT cliprect;
	SetRect(&cliprect, clipRct->x, clipRct->y, clipRct->x + clipRct->w, clipRct->y + clipRct->h);
	if((cliprect.right <= cliprect.left) || (cliprect.bottom <= cliprect.top))
		return;
	int aframeIdx = sprCol->Animations[animationIdx]->aframesIdx[currentFrame];
	for( int ii = 0; ii < sprCol->AFrames[aframeIdx]->fmodulesNo; ii++ )
	{
		int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[ii];
		//face clip
		RECT srcrect = sprCol->FModules[fmoduleIdx]->moduleRect;
		RECTLTRB_F destrect;

		destrect.left = pos.x + sprCol->FModules[fmoduleIdx]->ox;
		destrect.top = pos.y + sprCol->FModules[fmoduleIdx]->oy;
		destrect.right = destrect.left + srcrect.right - srcrect.left;
		destrect.bottom = destrect.top + srcrect.bottom - srcrect.top;
		D3DXVECTOR3 drawpos(destrect.left, destrect.top, 0.0f);

		//vede daca iese detot din dreptunghiul de clip
		if((destrect.left > cliprect.right) || (destrect.top > cliprect.bottom) || (destrect.bottom < cliprect.top) || (destrect.right < cliprect.left))
			continue;
		//face clip
		if(destrect.right > cliprect.right)
		{
			srcrect.right -= destrect.right - cliprect.right;
		}
		if(destrect.bottom > cliprect.bottom)
		{
			srcrect.bottom -= destrect.bottom - cliprect.bottom;
		}
		if(destrect.left < cliprect.left)
		{
			srcrect.left += cliprect.left - destrect.left;
			drawpos.x += (float)(cliprect.left - destrect.left);
		}
		if(destrect.top < cliprect.top)
		{
			srcrect.top += cliprect.top - destrect.top;
			drawpos.y += (float)(cliprect.top - destrect.top);
		}

		//deseneaza
		s_pSprite->Draw(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx]->pTex, 
			&srcrect, 
			NULL, 
			&drawpos,
			color);
	}
}

void CSprite::paintTiledOffset(CSpriteCollection *sprCol, int W, int H, int offsX, int offsY)
{
	if ((W == 0) || (H == 0))
		return;

	RECTXYWH sprrect = sprCol->GetAFrameBBox_real(animationIdx, currentFrame);
	RECTXYWH destrect = sprrect;

	if (W > 0)
		destrect.w = W;

	if (H > 0)
		destrect.h = H;


	int locOffsX = offsX % sprrect.w;
	if (locOffsX > 0)
		locOffsX -= sprrect.w;
	int locOffsY = offsY % sprrect.h;
	if (locOffsY > 0)
		locOffsY -= sprrect.h;
	//regleaza si offsetul
	int intsX = 2 + destrect.w / sprrect.w;
	int intsY = 2 + destrect.h / sprrect.h;

	RECT destrectRC;
	SetRect(&destrectRC, destrect.x + pos.x, destrect.y + pos.y, destrect.x + pos.x + destrect.w, destrect.y + pos.y + destrect.h);

	if ((intsX <= 0) && (intsY <= 0))
	{
		paint(sprCol, &destrectRC);
	}
	else
	{
		float oldX = pos.x, oldY = pos.y;
		float origX = pos.x + locOffsX , origY = pos.y + locOffsY;
		//deseneaza partile intregi
		for (int xx = 0; xx < intsX; xx++)
		{
			for (int yy = 0; yy < intsY; yy++)
			{
				pos.x = origX + xx * sprrect.w;
				pos.y = origY + yy * sprrect.h;
				paint(sprCol, &destrectRC);
			}
		}

		pos.x = oldX; pos.y = oldY;
	}
}


void CSprite::paintTiled(CSpriteCollection *sprCol, int W, int H)
{
	RECTXYWH sprrect = sprCol->GetAFrameBBox_real(animationIdx, currentFrame);
	RECTXYWH destrect = sprrect;
	if(W > 0)
		destrect.w = W;
	if(H > 0)
		destrect.h = H;

	int intsX = destrect.w / sprrect.w;
	int intsY = destrect.h / sprrect.h;
	int restW = destrect.w - (intsX * sprrect.w);
	int restH = destrect.h - (intsY * sprrect.h);

	RECT destrectRC;
	SetRect(&destrectRC, destrect.x + pos.x, destrect.y + pos.y, destrect.x + pos.x + destrect.w, destrect.y + pos.y + destrect.h);

	if((intsX <= 0) && (intsY <= 0))
		paint(sprCol, &destrectRC);
	else
	{
		float origX = pos.x, origY = pos.y;
		//deseneaza partile intregi
		for(int xx = 0; xx < intsX; xx++)
		{
			for(int yy = 0; yy < intsY; yy++)
			{
				pos.x = origX + xx * sprrect.w;
				pos.y = origY + yy * sprrect.h;
				paint(sprCol);
			}
		}
		//daca avem si clip
		if(restW > 0)
		{
			pos.x = origX + intsX * sprrect.w;
			pos.y = origY;
			for(int yy = 0; yy < intsY; yy++)
			{
				paint(sprCol, &destrectRC);
				pos.y += sprrect.h;
			}
		}
		if(restH > 0)
		{
			pos.x = origX;
			pos.y = origY + intsY * sprrect.h;
			for(int xx = 0; xx <= intsX; xx++)
			{
				paint(sprCol, &destrectRC);
				pos.x += sprrect.w;
			}
		}

		pos.x = origX; pos.y = origY;
	}
}


/*
void CSprite::paintTransformed(CSpriteMgr *sprManager, float scale, float rot)
{
	D3DXMATRIXA16 mat;

	//nu am luat in considerare inca flagsurile
	int frameID = sprManager->AFrames[sprManager->Animations[animationIdx]->animation_aframeIdx[currentFrame]]->aframe_frameIdx;
	int modulesCount = sprManager->Frames[frameID]->frame_fmodulesNo;

	for(int ii=0;ii<modulesCount;ii++)
	{
		int fmoduleIdx = sprManager->Frames[frameID]->frame_fmodulesIdx[ii];

		D3DXMatrixAffineTransformation2D(&mat, scale, 
			&D3DXVECTOR2(sprManager->FModules[fmoduleIdx]->fmodule_ox, sprManager->FModules[fmoduleIdx]->fmodule_oy),
			rot, &D3DXVECTOR2(posX, posY));
		s_pSprite->SetTransform(&mat);
										
		
		//trebuiesc tratate si flagurile de flip and so on...
		//int drawflag=0;
		//if ( (sprManager->flagsA & 128) != 0)
		//{
		//	int flag = sprManager->FModules[fmoduleIdx]->fmodule_flags;
		//	if ( (flag & 1) !=0) drawflag=Sprite.TRANS_MIRROR_ROT180;
		//	if ( (flag & 2) !=0) drawflag|=Sprite.TRANS_MIRROR;
		//}
		

		int moduleIdx = sprManager->FModules[fmoduleIdx]->fmodule_moduleIdx;
		//deseneaza
		s_pSprite->Draw(sprManager->m_texManager.GetTexture(sprManager->Modules[moduleIdx]->module_imgIdx), 
						&sprManager->Modules[moduleIdx]->moduleRect, 
						&v_center, 
						NULL,
						color);
	}
	s_pSprite->SetTransform(&g_matIdentity);
}
*/
void CSprite::paintTiledHOriented(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2)
{
	D3DXMATRIXA16 matr;
	D3DXVECTOR2 vecdir = pt2 - pt1;
	float len = D3DXVec2Length(&vecdir);
	if(len > 0.0f)
	{
		float ang = -atan2(vecdir.x, vecdir.y);
		D3DXMatrixAffineTransformation2D(&matr, 1.0f, NULL, ang + HALF_PI, &pt1);
		s_pSprite->SetTransform(&matr);
		paintTiled(sprManager, (int)len, -1);
		s_pSprite->SetTransform(&g_matIdentity);
	}
}

void CSprite::paintTiledHOrientedOffset(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2, int offsetX)
{
	D3DXMATRIXA16 matr;
	D3DXVECTOR2 vecdir = pt2 - pt1;
	float len = D3DXVec2Length(&vecdir);
	if (len > 0.0f)
	{
		float ang = -atan2(vecdir.x, vecdir.y);
		D3DXMatrixAffineTransformation2D(&matr, 1.0f, NULL, ang + HALF_PI, &pt1);
		s_pSprite->SetTransform(&matr);
		paintTiledOffset(sprManager, (int)len, -1, offsetX, 0);
		s_pSprite->SetTransform(&g_matIdentity);
	}
}

void CSprite::PaintStretchedXOriented(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2)
{
	D3DXMATRIXA16 matr, matr2;
	D3DXVECTOR2 vecdir = pt2 - pt1;
	float len = D3DXVec2Length(&vecdir);
	if (len > 0.0f)
	{
		RECTXYWH bbox = sprManager->GetAFrameBBox_real(animationIdx, currentFrame);
		float fstretch = len / bbox.w;
		float ang = -atan2(vecdir.x, vecdir.y);
		D3DXMatrixScaling(&matr, fstretch, 1.0f, 1.0f);
		D3DXMatrixAffineTransformation2D(&matr2, 1.0f, NULL, ang + HALF_PI, &pt1);
		matr *= matr2;
		s_pSprite->SetTransform(&matr);

		paint(sprManager);

		s_pSprite->SetTransform(&g_matIdentity);
	}
}

void CSprite::PaintStretchedXOriented_texOverride(CSpriteCollection *sprManager, D3DXVECTOR2 pt1, D3DXVECTOR2 pt2, int texIdxOffset)
{
	D3DXMATRIXA16 matr, matr2;
	D3DXVECTOR2 vecdir = pt2 - pt1;
	float len = D3DXVec2Length(&vecdir);
	if (len > 0.0f)
	{
		RECTXYWH bbox = sprManager->GetAFrameBBox_real(animationIdx, currentFrame);
		float fstretch = len / bbox.w;
		float ang = -atan2(vecdir.x, vecdir.y);
		D3DXMatrixScaling(&matr, fstretch, 1.0f, 1.0f);
		D3DXMatrixAffineTransformation2D(&matr2, 1.0f, NULL, ang + HALF_PI, &pt1);
		matr *= matr2;
		s_pSprite->SetTransform(&matr);

		paint_texOverride(sprManager, texIdxOffset);

		s_pSprite->SetTransform(&g_matIdentity);
	}
}

void CSprite::paintModule_texOverride(CSpriteCollection *sprCol, int nModuleIdx, int texIdxOffset)
{
	_ASSERT(animationIdx < sprCol->Animations.Count());
	_ASSERT(currentFrame < sprCol->Animations[animationIdx]->aframesNo);
	
	int aframeIdx = sprCol->Animations[animationIdx]->aframesIdx[currentFrame];
	_ASSERT(nModuleIdx < sprCol->AFrames[aframeIdx]->fmodulesNo);

	scFModule* mod = sprCol->FModules[sprCol->AFrames[aframeIdx]->fmodulesIdx[nModuleIdx]];
	
	s_pSP->Draw(sprCol->Textures[mod->imgIdx + texIdxOffset]->pTex,
		mod->texRect,
		mod->moduleRectOff,
		pos,
		color);
}

void CSprite::paintFrameNEW(CSpriteCollection *sprCol, Vec2 vPos, int animID, int frameID, DWORD ncolor, float fRotZ, Vec2 vScale)
{
	_ASSERT(animID < sprCol->Animations.Count());
	_ASSERT(frameID < sprCol->Animations[animID]->aframesNo);

	int aframeIdx = sprCol->Animations[animID]->aframesIdx[frameID];
	for (int ii = 0; ii < sprCol->AFrames[aframeIdx]->fmodulesNo; ii++)
	{
		int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[ii];
		
		s_pSP->DrawEx(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx]->pTex,
			sprCol->FModules[fmoduleIdx]->texRect,
			sprCol->FModules[fmoduleIdx]->moduleRectOff,
			vPos,
			ncolor, fRotZ, vScale);
	}
}

//?? this is bad...
void CSprite::paintFrameModuleNEW(CSpriteCollection *sprCol, float nX, float nY, int animID, int frameID, int moduleID, DWORD ncolor)
{
	_ASSERT(animID < sprCol->Animations.Count());
	_ASSERT(frameID < sprCol->Animations[animID]->aframesNo);
	_ASSERT(moduleID < sprCol->AFrames[sprCol->Animations[animID]->aframesIdx[frameID]]->fmodulesNo);

	int aframeIdx = sprCol->Animations[animID]->aframesIdx[frameID];
	int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[moduleID];
	
	s_pSP->Draw(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx]->pTex,
		sprCol->FModules[fmoduleIdx]->texRect,
		sprCol->FModules[fmoduleIdx]->moduleRectOff,
		Vec2(nX, nY), ncolor);
}
