#include "dxstdafx.h"
#include "Spr.h"

//--- EDITOR duration per frame (100 = 1 sec) -----
#define SPR_ED_TIMELINE 100.0f

///--- STATIC MEMBERS ---
CSpritePainter* CSpr::s_pSP = nullptr;
void CSpr::SetGlobalSpritePtr(CSpritePainter* pSP)
{
	s_pSP = pSP;
}



CSpr::CSpr() 
{
	animID = 0;
	pos.x = 0.0f;
	pos.y = 0.0f;
	fTime = 0.0f;
	frameID = 0;
	animStatus = ANIM_JUST_STARTED;
	color = 0xffffffff;
}

CSpr::CSpr(int animIdx, float pX, float pY) 
{
	animID = animIdx;
	pos.x = pX;
	pos.y = pY;
	fTime = 0.0f;
	frameID = 0;
	animStatus = ANIM_JUST_STARTED;
	color=0xffffffff;
}

CSpr::CSpr(int animIdx, Vec2 vPos)
{
	animID = animIdx;
	pos = vPos;
	fTime = 0.0f;
	frameID = 0;
	animStatus = ANIM_JUST_STARTED;
	color = 0xffffffff;
}

CSpr::CSpr(const CSpr& sprite)
{
	animID = sprite.animID;
	pos = sprite.pos;
	fTime = sprite.fTime;
	frameID = sprite.frameID;
	animStatus = sprite.animStatus;
	color = sprite.color;
}

void CSpr::Init(int animIdx, Vec2 vPos, int nframeIdx, DWORD nColor)
{
	animID = animIdx;
	pos = vPos;
	fTime = 0.0f;
	frameID = nframeIdx;
	animStatus = ANIM_JUST_STARTED;
	color = nColor;
}

void CSpr::Init(CHAR* strAnimID, CSpriteCollection *sprCollection, Vec2 vPos, int nframeIdx, DWORD nColor)
{
	animID = sprCollection->getAnimationIdxByName(strAnimID);
	if (animID < 0)
	{
		LOG("Sprite::Init: Animation [%s] not found!", strAnimID);
		return;
	}

	pos = vPos;
	fTime = 0.0f;
	frameID = nframeIdx;
	animStatus = ANIM_JUST_STARTED;
	color = nColor;
}

void CSpr::SetAnim(int animIdx, int frameIdx)
{
	animID = animIdx;

	fTime = 0.0f;
	frameID = frameIdx;
	animStatus = ANIM_JUST_STARTED;
}

void CSpr::SetFrame(int nFrameIdx)
{
	frameID = nFrameIdx;
	fTime = 0.0f;
	animStatus = ANIM_PLAYING;
}

//seteaza animatia doar daca e alta
void CSpr::SetAnimOnce(int animIdx, int frameIdx) 
{
	if((animIdx == animID) || (animIdx < 0))
		return;

	animID = animIdx;

	fTime = 0.0f;
	frameID = frameIdx;
	animStatus = ANIM_JUST_STARTED;
}


void CSpr::SetAnim(CSpriteCollection *sprCollection, CHAR* strAnimID)
{
	animID = sprCollection->getAnimationIdxByName(strAnimID);
	if (animID < 0)
	{
		LOG("Sprite::SetAnim: Animation [%s] not found!", strAnimID);
		return;
	}

	fTime = 0.0f;
	frameID = 0;
	animStatus = ANIM_JUST_STARTED;
}

//face update la pozitie numai daca e true updatePos
UINT32 CSpr::Update(CSpriteCollection *sprCollection, float dTime, bool updatePos)
{
	UINT32 retAFrameFlag = 0;

	assert(animID < sprCollection->Animations.Count());
	assert(frameID < sprCollection->Animations[animID]->aframesNo);

	if (animStatus == ANIM_FRAMELOCK)
		return retAFrameFlag;

	int aframeID = sprCollection->Animations[animID]->aframesIdx[frameID];
	//pentru animatiile abia setate imi intoarce aframe flag-ul frame-ului curent
	if (animStatus == ANIM_JUST_STARTED)
	{
		//TODO: daca primul frame e foarte scurt pierde primul mesaj retAFrameFlag
		retAFrameFlag = sprCollection->AFrames[aframeID]->flags;
	}

	animStatus = ANIM_PLAYING;
	fTime += dTime;

	if( fTime >= float(sprCollection->AFrames[aframeID]->duration) / SPR_ED_TIMELINE )
	{
		//a avansat frame. Modifica si pozitia. Rezulta de aici ca modificarea pozitiei o face la sfarsitul frame-ului.
		//oricum ar trebui interpolat spline intre pozitii asa ca momentan e bine asa.
		animStatus = ANIM_PLAYING_FRAME_ADVANCED;
		if( updatePos )
		{
			pos.x += sprCollection->AFrames[aframeID]->dx;
			pos.y += sprCollection->AFrames[aframeID]->dy;
		}

		fTime -= float(sprCollection->AFrames[aframeID]->duration) / SPR_ED_TIMELINE;
		frameID++;

		if (frameID >= sprCollection->Animations[animID]->aframesNo) 
		{
			if ( (sprCollection->Animations[animID]->flags & ANIMATION_FLAG_LOOPED) == 0 )
			{ //daca e play once
				frameID--; //pozitioneaza pe ultimul frame
				animStatus = ANIM_FRAMELOCK; //face lock pe ultimul frame
				//get new flag
				aframeID = sprCollection->Animations[animID]->aframesIdx[frameID];
			}
			else 
			{ //daca e looping
				frameID = 0;
				animStatus = ANIM_LOOPRESET;
				//get new flag
				aframeID = sprCollection->Animations[animID]->aframesIdx[frameID];
			}
		}
		else
		{
			aframeID = sprCollection->Animations[animID]->aframesIdx[frameID];
		}
		//set aframe flag for return
		retAFrameFlag = sprCollection->AFrames[aframeID]->flags;
	}
	return retAFrameFlag;
}

void CSpr::Paint(CSpriteCollection* sprCol)
{
	assert(animID < sprCol->Animations.Count());
	assert(frameID < sprCol->Animations[animID]->aframesNo);
	//nu am luat in considerare inca flagsurile
	int aframeIdx = sprCol->Animations[animID]->aframesIdx[frameID];
	for( int ii = 0; ii < sprCol->AFrames[aframeIdx]->fmodulesNo; ii++ )
	{
		int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[ii];
		
		s_pSP->Draw(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx]->pTex, 
			&sprCol->FModules[fmoduleIdx]->moduleRect, 
			NULL, 
			&D3DXVECTOR3(pos.x + sprCol->FModules[fmoduleIdx]->ox, pos.y + sprCol->FModules[fmoduleIdx]->oy, 0.0f),
			color);
	}
}

void CSpr::PaintModule(CSpriteCollection* sprCol, int moduleIdx)
{
	_ASSERT(animID < sprCol->Animations.Count());
	_ASSERT(frameID < sprCol->Animations[animID]->aframesNo);
	int aframeIdx = sprCol->Animations[animID]->aframesIdx[frameID];
	_ASSERT(moduleIdx < sprCol->AFrames[aframeIdx]->fmodulesNo); 

	int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[moduleIdx];
	
	s_pSprite->Draw(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx]->pTex,
		&sprCol->FModules[fmoduleIdx]->moduleRect,
		NULL,
		&D3DXVECTOR3(pos.x + sprCol->FModules[fmoduleIdx]->ox, pos.y + sprCol->FModules[fmoduleIdx]->oy, 0.0f),
		color);
}

void UTSprite::PaintFrameEx(CSpriteCollection *sprCol, Vec2 vPos, int animID, int frameID, DWORD ncolor, float fRotZ, Vec2 vScale)
{
	_ASSERT(animID < sprCol->Animations.Count());
	_ASSERT(frameID < sprCol->Animations[animID]->aframesNo);

	int aframeIdx = sprCol->Animations[animID]->aframesIdx[frameID];
	for (int ii = 0; ii < sprCol->AFrames[aframeIdx]->fmodulesNo; ii++)
	{
		int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[ii];

		UTPainter().DrawEx(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx]->pTex,
			sprCol->FModules[fmoduleIdx]->texRect,
			sprCol->FModules[fmoduleIdx]->moduleRectOff,
			vPos,
			ncolor, fRotZ, vScale);
	}
}

void UTSprite::PaintFrameModule(CSpriteCollection *sprCol, Vec2 vPos, int animID, int frameID, int moduleID, DWORD ncolor)
{
	_ASSERT(animID < sprCol->Animations.Count());
	_ASSERT(frameID < sprCol->Animations[animID]->aframesNo);
	_ASSERT(moduleID < sprCol->AFrames[sprCol->Animations[animID]->aframesIdx[frameID]]->fmodulesNo);

	int aframeIdx = sprCol->Animations[animID]->aframesIdx[frameID];
	int fmoduleIdx = sprCol->AFrames[aframeIdx]->fmodulesIdx[moduleID];

	UTPainter().Draw(sprCol->Textures[sprCol->FModules[fmoduleIdx]->imgIdx]->pTex,
		sprCol->FModules[fmoduleIdx]->texRect,
		sprCol->FModules[fmoduleIdx]->moduleRectOff,
		vPos, ncolor);
}



DWORD UTSprite::GetFrameFlags(CSpriteCollection* sprCol, int animID, int frameID)
{
	if (animID < 0 || frameID < 0) return (DWORD)0;
	int frameIdx = sprCol->Animations[animID]->aframesIdx[frameID];
	return sprCol->AFrames[frameIdx]->flags;
}
