#include "dxstdafx.h"
#include "Spr.h"

//--- EDITOR duration per frame (100 = 1 sec) -----
#define SPR_ED_TIMELINE 100.0f

///--- STATIC MEMBERS ---
CSpritePainter* CSpr::s_pSP = &UTPainter();

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

CSpr::CSpr(CSpriteCollection* pSpriteColl, int animIdx, float pX, float pY)
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

CSpr::CSpr(CSpriteCollection* pSpriteColl, int animIdx, Vec2 vPos)
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

void CSpr::Init(CSpriteCollection *sprCollection, int nAnimIdx, Vec2 vPos, int nframeIdx, DWORD nColor, float fRotation, Vec2 vScale)
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

void CSpr::Init(CSpriteCollection *sprCollection, CHAR* strAnimID, Vec2 vPos, int nframeIdx, DWORD nColor, float fRotation, Vec2 vScale)
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

//seteaza animatia doar daca e alta
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

//face update la pozitie numai daca e true updatePos
UINT32 CSpr::Update(float dTime, bool updatePos)
{
	UINT32 retAFrameFlag = 0;

	_ASSERT(animIdx < pSprCol->Animations.Count());
	_ASSERT(frameIdx < pSprCol->Animations[animIdx]->aframesNo);

	if (animStatus == ANIM_FRAMELOCK)
		return retAFrameFlag;

	int aframeID = pSprCol->Animations[animIdx]->aframesIdx[frameIdx];
	//pentru animatiile abia setate imi intoarce aframe flag-ul frame-ului curent
	if (animStatus == ANIM_JUST_STARTED)
	{
		//TODO: daca primul frame e foarte scurt pierde primul mesaj retAFrameFlag
		retAFrameFlag = pSprCol->AFrames[aframeID]->flags;
	}

	animStatus = ANIM_PLAYING;
	fTime += dTime;

	if( fTime >= float(pSprCol->AFrames[aframeID]->duration) / SPR_ED_TIMELINE )
	{
		//a avansat frame. Modifica si pozitia. Rezulta de aici ca modificarea pozitiei o face la sfarsitul frame-ului.
		//oricum ar trebui interpolat spline intre pozitii asa ca momentan e bine asa.
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
			{ //daca e play once
				frameIdx--; //pozitioneaza pe ultimul frame
				animStatus = ANIM_FRAMELOCK; //face lock pe ultimul frame
				//get new flag
				aframeID = pSprCol->Animations[animIdx]->aframesIdx[frameIdx];
			}
			else 
			{ //daca e looping
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
		//set aframe flag for return
		retAFrameFlag = pSprCol->AFrames[aframeID]->flags;
	}
	return retAFrameFlag;
}

void CSpr::Paint()
{
	_ASSERT(animIdx < pSprCol->Animations.Count());
	_ASSERT(frameIdx < pSprCol->Animations[animIdx]->aframesNo);
	//nu am luat in considerare inca flagsurile
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

void CSpr::PaintModule(int moduleIdx)
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
 

void UTSprite::PaintFrameEx(CSpriteCollection *sprCol, Vec2 vPos, int animID, int frameID, DWORD ncolor, float fRotZ, Vec2 vScale, UINT unFlags)
{
	_ASSERT(animID < sprCol->Animations.Count());
	_ASSERT(frameID < sprCol->Animations[animID]->aframesNo);

	int aframeIdx = sprCol->Animations[animID]->aframesIdx[frameID];
	for (int ii = 0; ii < sprCol->AFrames[aframeIdx]->fmodulesNo; ii++)
	{
		scFModule* mod = sprCol->FModules[sprCol->AFrames[aframeIdx]->fmodulesIdx[ii]];

		UTPainter().DrawEx(mod->pImg->pTex,
			mod->texRect,
			mod->moduleRectOff,
			vPos,
			ncolor, fRotZ, vScale, unFlags);
	}
}

void UTSprite::PaintFrameModule(CSpriteCollection *sprCol, Vec2 vPos, int animID, int frameID, int moduleID, DWORD ncolor)
{
	_ASSERT(animID < sprCol->Animations.Count());
	_ASSERT(frameID < sprCol->Animations[animID]->aframesNo);
	_ASSERT(moduleID < sprCol->AFrames[sprCol->Animations[animID]->aframesIdx[frameID]]->fmodulesNo);

	int aframeIdx = sprCol->Animations[animID]->aframesIdx[frameID];
	scFModule* mod = sprCol->FModules[sprCol->AFrames[aframeIdx]->fmodulesIdx[moduleID]];

	UTPainter().Draw(mod->pImg->pTex,
		mod->texRect,
		mod->moduleRectOff,
		vPos, ncolor);
}



DWORD UTSprite::GetFrameFlags(CSpriteCollection* sprCol, int animID, int frameID)
{
	if (animID < 0 || frameID < 0) return (DWORD)0;
	int frameIdx = sprCol->Animations[animID]->aframesIdx[frameID];
	return sprCol->AFrames[frameIdx]->flags;
}
