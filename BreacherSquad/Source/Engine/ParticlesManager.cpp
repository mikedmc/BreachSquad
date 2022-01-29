#include "dxstdafx.h"
#include "particlesmanager.h"

//---- constructor particula -----
CParticle::CParticle()
{
	pNext = NULL;
	pPrev = NULL;   
	//default values
	m_fSize		=	1.0f;
	m_fScaleSpeed = 0.0f;
	m_vPos = D3DXVECTOR2(0.0f, 0.0f);
	m_vSpeed = D3DXVECTOR2(0.0f, 0.0f);
	m_fRotAngle = 0.0f;
	m_fRotSpeed = 0.0f;
	m_Color = 0xffffffff;
	m_fLifetime = 2.0f;
	m_fFadeOut_Duration = 0.0f; //incepand de la lifetime<=m_fFade_LifeStart face fade
	m_fFadeIn_Duration = 0.0f;
	m_vGravity = D3DXVECTOR2(0.0f, 0.0f);
	m_fSize = 1.0f;
	m_fScaleSpeed = 0.0f;
	bAnimated = false;
	m_fLife = 0.0f;
	m_fAlpha = 1.0f;
	m_fAirFriction = 0.0f;
	m_fWaitTimer = 0.0f;
	//pentru desenare
	sprite.Init(0, 0.0f, 0.0f);
}

CStringParticle::CStringParticle()
{
	//default values
	m_fSize		=	1.0f;
	m_fScaleSpeed = 0.0f;
	m_vPos = D3DXVECTOR2(0.0f, 0.0f);
	m_vSpeed = D3DXVECTOR2(0.0f, 0.0f);
	m_fRotAngle = 0.0f;
	m_fRotSpeed = 0.0f;
	m_fLifetime = 2.0f;
	m_fFadeOut_Duration = 0.0f; //incepand de la lifetime<=m_fFade_LifeStart face fade
	m_fFadeIn_Duration = 0.0f;
	m_vGravity = D3DXVECTOR2(0.0f, 0.0f);
	m_fSize = 1.0f;
	m_fScaleSpeed = 0.0f;
	m_fLife = 0.0f;
	//string
	m_pFont = NULL;
	strColor = 0xffffffff;
	mLayer = K_PART_LAYER_NORMAL;
	m_fAlpha = 1.0f;
}

void CParticlesManager::AddStringParticle(CTexFont *pFont, WCHAR* text,
						D3DXVECTOR2* pos, 
						D3DXVECTOR2* gravity, D3DXVECTOR2* speed, 
						float lifetime, 
						float size, float scalespeed, 
						float rotangle, float rotspeed, 
						float fadeInDuration,
						float fadeOutDuration,
						DWORD nColor,
						int nLayer)
{
	CStringParticle *newp = new CStringParticle();
	newp->m_pFont = pFont;
	newp->m_stringDesc.len = wcslen(text);
	newp->m_stringDesc.sText = new WCHAR[newp->m_stringDesc.len + 1];
	newp->strColor = nColor;
	memcpy(newp->m_stringDesc.sText, text, sizeof(WCHAR) * (newp->m_stringDesc.len + 1));
	__Texts().BuildStringCodes(&newp->m_stringDesc);

	if(pos)
		newp->m_vPos = *pos;
	else
		newp->m_vPos = D3DXVECTOR2(0.0f, 0.0f);
	if(gravity)
		newp->m_vGravity = *gravity;
	else
		newp->m_vGravity = D3DXVECTOR2(0.0f, 0.0f);
	if(speed)
		newp->m_vSpeed = *speed;
	else
		newp->m_vSpeed = D3DXVECTOR2(0.0f, 0.0f);
	newp->m_fLifetime = lifetime;
	newp->m_fScaleSpeed = scalespeed;
	newp->m_fSize = size;
	newp->m_fRotSpeed = rotspeed;
	newp->m_fRotAngle = rotangle;
	newp->m_fFadeOut_Duration = fadeOutDuration;
	newp->m_fFadeIn_Duration= fadeInDuration;

	newp->m_fAlpha = ((float)((newp->strColor & 0xff000000)>>24))/255.0f;
	
	float newcol = newp->m_fAlpha;
	//fade in   (nu intra cand fadeIn = 0.0f
	if(newp->m_fLife < newp->m_fFadeIn_Duration)
	{
		newcol *= newp->m_fLife / newp->m_fFadeIn_Duration;
	}
	newp->strColor = (newp->strColor & 0x00ffffff) | ((BYTE(newcol * 255))<<24);

	newp->mLayer = nLayer;

	m_vStringParticles.Add(newp);
}

void CParticlesManager::UpdateStringParticles(float dtime)
{
	//particule text
	for (int ii = m_vStringParticles.GetSize() - 1; ii >= 0; ii--)
	{
		CStringParticle* parts = m_vStringParticles[ii];

		//update pos
		parts->m_vSpeed += parts->m_vGravity * dtime;
		parts->m_vPos += parts->m_vSpeed * dtime;
		parts->m_fRotAngle += parts->m_fRotSpeed * dtime;
		parts->m_fSize += parts->m_fScaleSpeed * dtime;
		//--- transparenta ---
		float newcol = parts->m_fAlpha;
		//fade in   (nu intra cand fadeIn = 0.0f
		if(parts->m_fLife < parts->m_fFadeIn_Duration)
		{
			newcol *= parts->m_fLife / parts->m_fFadeIn_Duration;
		}
		//fade out
		if(parts->m_fFadeOut_Duration > 0.0f)
		{
			float lfdiff = parts->m_fLifetime - parts->m_fLife;
			if(lfdiff < parts->m_fFadeOut_Duration)
			{
				newcol *= lfdiff / parts->m_fFadeOut_Duration;
			}
		}
		
		parts->strColor = (parts->strColor & 0x00ffffff) | ((BYTE(newcol * 255))<<24);

		//-- verificari dezalocare ----
		parts->m_fLife += dtime;
		if(parts->m_fLife >= parts->m_fLifetime)
		{
			SAFE_DELETE(parts);
			m_vStringParticles.Remove(ii);
		}
	}

}

void CParticlesManager::PaintStringParticles(int nLayer, bool paintUsingMultiply)
{
	CStringParticle *parts;
	D3DXMATRIXA16 mattrans;

	if(paintUsingMultiply)
	{
		AdditiveBlendingON(m_pDevice, m_pSprite);
	}

	for(int ii=0; ii<m_vStringParticles.GetSize(); ii++)
	{
		parts = m_vStringParticles[ii];
		if(parts->mLayer != nLayer)
			continue;

		D3DXMatrixAffineTransformation2D(&mattrans, parts->m_fSize, &D3DXVECTOR2(0,0), parts->m_fRotAngle, &parts->m_vPos);
		m_pSprite->SetTransform(&mattrans);
		parts->m_pFont->DrawString(&parts->m_stringDesc, 0.0f, 0.0f, FONTFLAG_ANCHOR_VCENTERHCENTER, parts->strColor);
	}

	if(paintUsingMultiply)
	{
		AdditiveBlendingOFF(m_pDevice, m_pSprite);
	}

	m_pSprite->SetTransform(&g_matIdentity);
}

void CParticlesManager::PaintStringParticles(int nLayer, D3DXVECTOR2 offset, bool paintUsingMultiply)
{
	CStringParticle *parts;
	D3DXMATRIXA16 mattrans;

	if (paintUsingMultiply)
	{
		AdditiveBlendingON(m_pDevice, m_pSprite);
	}

	for (int ii = 0; ii<m_vStringParticles.GetSize(); ii++)
	{
		parts = m_vStringParticles[ii];
		if (parts->mLayer != nLayer)
			continue;

		D3DXMatrixAffineTransformation2D(&mattrans, parts->m_fSize, &D3DXVECTOR2(0, 0), parts->m_fRotAngle, &(parts->m_vPos + offset));
		m_pSprite->SetTransform(&mattrans);
		parts->m_pFont->DrawString(&parts->m_stringDesc, 0.0f, 0.0f, FONTFLAG_ANCHOR_VCENTERHCENTER, parts->strColor);
	}

	if (paintUsingMultiply)
	{
		AdditiveBlendingOFF(m_pDevice, m_pSprite);
	}

	m_pSprite->SetTransform(&g_matIdentity);
}


///------ DUMMIES -----------
CStringDummy* CParticlesManager::GetStringDummy(int nType)
{
	for (int kk = 0; kk < m_vDummies.GetSize(); kk++)
	{
		if (m_vDummies[kk]->type == nType)
			return m_vDummies[kk];
	}
	return NULL;
}

int CParticlesManager::AddStringDummy(int nType, D3DXVECTOR2 np1, int stringID, int fontID, float size, float showTime, DWORD color, float fDelay)
{
	CStringDummy *ndum = new CStringDummy();
	ndum->type = nType;
	ndum->fShowDelay = fDelay;

	ndum->flag = 1;

	switch(nType)
	{
	case K_PDUMMY_STRING_WIDEBAR:
		{
			ndum->pos = np1;
			ndum->status = 0;
			ndum->intParam = stringID;
			ndum->intParam2 = fontID;
			ndum->intParam3 = ndum->intParam4 = -1; //second string id and font id
			ndum->timer = showTime;
			ndum->sprite.color = color;
			//text offset
			ndum->fparam2 = showTime;

			ndum->fPtr = NULL;
		}
		break;

	case K_PDUMMY_STRING_WOBBLER:
		{
			ndum->pt1 = np1;
			ndum->status = 0;
			ndum->intParam = stringID;
			ndum->intParam2 = fontID;
			ndum->timer = 0.0f;
			ndum->intPt.x = fontID;
			ndum->intPt.y = size * __TexFonts()[fontID]->MeasureHString(__Texts().strings[stringID]->shStringName.textHash).w;
			ndum->fparam = size;
			ndum->fparam2 = showTime;

			ndum->sprite.color = color;

			ndum->fPtr = NULL;
		}
		break;

	case K_PDUMMY_STRING_BLINKER:
	{
		ndum->pt1 = np1;
		ndum->status = 0;
		ndum->intParam = stringID;
		ndum->intParam2 = fontID;
		ndum->timer = 0.0f;
		ndum->fparam = size;
		ndum->fparam2 = showTime;

		ndum->sprite.color = color;
	}
	break;
	case K_PDUMMY_STRING_LETTERWAVER:
		{
			ndum->pt1 = np1;
			ndum->status = 0;
			ndum->intParam = stringID;
			ndum->intParam2 = __Texts().strings[stringID]->len;
			ndum->timer = 0.0f;
			ndum->intPt.x = fontID;
			ndum->intPt.y = size * __TexFonts()[fontID]->MeasureHString(__Texts().strings[stringID]->shStringName.textHash).w;
			ndum->fparam = size;
			ndum->fparam2 = showTime;

			ndum->sprite.color = color;

			ndum->fPtr = new float[ndum->intParam2];
			for(int kk=0; kk<ndum->intParam2; kk++)
				ndum->fPtr[kk] = 0.0f;

		}
		break;
	}

	m_vDummies.Add(ndum);
	//return index
	return m_vDummies.GetSize() - 1;
}


void CParticlesManager::UpdateStringDummies(float dTime)
{
	for(int kk = m_vDummies.GetSize() - 1; kk >= 0; kk--)
	{
		CStringDummy* ndum = m_vDummies[kk];

		if (ndum->fShowDelay > 0.0f)
		{
			ndum->fShowDelay -= dTime;
			continue;
		}

		switch (ndum->type)
		{
			case K_PDUMMY_STRING_WIDEBAR:
			{
				dec_limit(ndum->timer, dTime, 0.0f);

				if (ndum->timer <= 0.0f)
				{
					SAFE_DELETE(m_vDummies[kk]);
					m_vDummies.Remove(kk);
				}
			}
			break;

			//afiseaza o linie de text marind fiecare litera pe rand, pentru "level finished" si alte mesaje
			case K_PDUMMY_STRING_WOBBLER:
			{
				if (ndum->status == 0) //apare
				{
					ndum->timer += dTime * 2.0f;
					if (ndum->timer >= 1.0f)
					{
						ndum->timer = ndum->fparam2;
						if (ndum->timer >= 0.0f)
						{
							ndum->status = 1;
						}
					}
				}
				else if (ndum->status == 1) //afiseaza text
				{
					ndum->timer -= dTime;

					if (ndum->timer <= 0.0f)
					{
						ndum->timer = 1.0f;
						ndum->status = 2;
					}
				}
				else if (ndum->status == 2) //dispare
				{
					ndum->timer -= dTime * 2.0f;
					if (ndum->timer <= 0.0f)
					{
						//aici moare
						SAFE_DELETE(m_vDummies[kk]);
						m_vDummies.Remove(kk);
					}
				}

			}
			break;

		case K_PDUMMY_STRING_BLINKER:
		{
			ndum->timer += dTime;
			if (ndum->timer >= ndum->fparam2)
			{
				//aici moare
				SAFE_DELETE(m_vDummies[kk]);
				m_vDummies.Remove(kk);
			}
		}
		break;

		case K_PDUMMY_STRING_LETTERWAVER:
			{
				if(ndum->status == 0) //creste literele
				{
					ndum->timer += dTime * 1.5f;
					//scalare
					for(int ll=0; ll<ndum->intParam2; ll++)
					{
						ndum->fPtr[ll] += (dTime * 4.0f * 1.5f) * (1.0f - 0.8f * (float)ll/(float)ndum->intParam2 );
						if(ndum->fPtr[ll] > 1.0f)
							ndum->fPtr[ll] = 1.0f;
					}

					if(ndum->timer >= 1.0f)
					{
						ndum->timer = ndum->fparam2;
						if(ndum->timer >= 0.0f)
							ndum->status = 1; //dispare cu timer
						else
							ndum->status = 3; //dispare la click

						for(int ll=0; ll<ndum->intParam2; ll++)
							ndum->fPtr[ll] = 1.0f;
					}
				}
				else if(ndum->status == 1) //afiseaza text
				{
					ndum->timer -= dTime;
					if(ndum->timer <= 0.0f)
					{
						ndum->timer = 1.0f;
						ndum->status = 2;
					}
				}
				else if(ndum->status == 2) //dispare textul
				{
					ndum->timer -= dTime * 2.0f;
					//scalare
					for(int ll=0; ll < ndum->intParam2; ll++)
					{
						ndum->fPtr[ll] -= (dTime * 4.0f * 1.5f) * (1.0f - 0.8f * (float)ll/(float)ndum->intParam2 );
						if(ndum->fPtr[ll] < 0.0f)
							ndum->fPtr[ll] = 0.0f;
					}

					if(ndum->timer <= 0.0f)
					{
						SAFE_DELETE_ARRAY(ndum->fPtr);

						//aici moare
						SAFE_DELETE(m_vDummies[kk]);
						m_vDummies.Remove(kk);
					}
				}
				else if(ndum->status == 3) //dispare la click
				{
					//momentan e folosit numai in joc la results, deci nu dispare decat pe changegamestate
					/*
					if((g_mouseLbutt != MOUSE_BUTT_NOTPRESSED) || (g_mouseLbutt != MOUSE_BUTT_NOTPRESSED))
					{
						ndum->timer = 1.0f;
						ndum->status = 2;
					}
					*/
				}
			}
			break;
		}
	}
}

void CParticlesManager::PaintStringDummies(UINT8 pflags)
{
	for(int kk = 0; kk < m_vDummies.GetSize(); kk++)
	{
		CStringDummy* ndum = m_vDummies[kk];

		//daca se cere un flag anume (si parametrul pflags este setat, adica e diferit de 0)
		if(pflags)
		{
			if((ndum->flag & pflags) == 0)
			{
				continue;
			}
		}
		//skip delayed ones
		if (ndum->fShowDelay > 0.0f)
		{
			continue;
		}

		RECTXYWH_F camrect = CCameraTransform::GetActiveCamera()->GetCamWorldAABB();
		D3DXMATRIXA16 mat1, mat2;

		switch(ndum->type)
		{
			case K_PDUMMY_STRING_WIDEBAR:
			{
				float fAlpha = 1.0f;
				if (ndum->timer > ndum->fparam2 - 0.25f)
					fAlpha = (ndum->fparam2 - ndum->timer) * 4.0f;
				else if (ndum->timer < 0.25f)
					fAlpha = ndum->timer * 4.0f;

				int fonth = __TexFonts()[ndum->intParam2]->rowHeight;
				int fonth2 = 0;
				if ((ndum->intParam3 >= 0) && (ndum->intParam4 >= 0))
				{
					fonth2 = __TexFonts()[ndum->intParam4]->rowHeight;
				}

				DWORD dwcol = DW_COLORALPHA(ndum->sprite.color, fAlpha);
				RECTXYWH ptrect(0, (int)floor(camrect.CenterY() + ndum->pos.y - fonth * 0.75f), 200, fonth * 1.5f + fonth2);
				GUIUtils::DrawWidebar(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_WIDEBAR2, ptrect, dwcol);

				//paint string
				float offx = 0.0f;
				if (ndum->timer > ndum->fparam2 - 0.25f)
					offx = TimeEasing(1.0f - ((ndum->fparam2 - ndum->timer) * 4.0f)) * camrect.w;
				else if (ndum->timer < 0.25f)
					offx = -(1.0f - TimeEasing(ndum->timer * 4.0f)) * camrect.w;


				__TexFonts()[ndum->intParam2]->DrawString(ndum->intParam, camrect.CenterX() + ndum->pos.x + offx, camrect.CenterY() + ndum->pos.y, FONTFLAG_ANCHOR_VCENTERHCENTER, dwcol);
				//second string
				if ((ndum->intParam3 >= 0) && (ndum->intParam4 >= 0))
				{
					__TexFonts()[ndum->intParam4]->DrawString(ndum->intParam3, camrect.CenterX() + ndum->pos.x + offx, camrect.CenterY() + ndum->pos.y + fonth, FONTFLAG_ANCHOR_VCENTERHCENTER, dwcol);
				}
			}
			break;
			case K_PDUMMY_STRING_WOBBLER:
			{
				D3DXVECTOR2 realPos(camrect.CenterX() + ndum->pt1.x, camrect.CenterY() + ndum->pt1.y);
				if((ndum->status == 0) || (ndum->status == 2))
				{
					float wobbleAmpX = 1.0f + 5.0f * (1.0f - ndum->timer) * sin(ndum->timer * 4.0f);
					float wobbleAmpY = 1.0f + 3.0f * (1.0f - ndum->timer) * sin(ndum->timer * 4.0f);
					D3DXMatrixScaling(&mat1, wobbleAmpX * ndum->fparam * ndum->timer, wobbleAmpY * ndum->fparam * ndum->timer, 1.0f);
					D3DXMatrixAffineTransformation2D(&mat2, 1.0f, NULL, 0.0f /*(wobbleAmpY - 1.0f) * 2.0f*/, &realPos);
					mat1 *= mat2;

					m_pSprite->SetTransform(&mat1);
					__TexFonts()[ndum->intPt.x]->DrawString(ndum->intParam, 0, 0, FONTFLAG_ANCHOR_VCENTERHCENTER, DW_COLOR_FFFA(ndum->timer));
					m_pSprite->SetTransform(&g_matIdentity);
				}
				else if(ndum->status == 1)
				{
					/*
					D3DXMatrixAffineTransformation2D(&mat1, ndum->fparam, NULL, 0.0f, &realPos);
	
					m_pSprite->SetTransform(&mat1);
					__TexFonts()[ndum->intPt.x]->DrawStringLightened(ndum->intParam, 0, 0, ndum->fparam2 - ndum->timer, 60.0f, FONTFLAG_ANCHOR_VCENTERHCENTER, 0xffffffff);
					m_pSprite->SetTransform(&g_matIdentity);
					*/
				}
			}
			break;
		//afisare text level finished si alte mesaje
		case K_PDUMMY_STRING_BLINKER:
			{
				D3DXVECTOR2 realPos(camrect.CenterX() + ndum->pt1.x, camrect.CenterY() + ndum->pt1.y);
				DWORD color = ndum->sprite.color;
				if (FLOAT_FRAC(ndum->timer) > 0.8f)
					color = 0x00000000;
				__TexFonts()[ndum->intParam2]->DrawString(ndum->intParam, realPos.x, realPos.y, FONTFLAG_ANCHOR_VCENTERHCENTER, color);
			}
			break;
		case K_PDUMMY_STRING_LETTERWAVER:
			{
				float posx = camrect.CenterX() + ndum->pt1.x - ndum->intPt.y * 0.5f;
				float glowalpha = 1.0f;
				if((ndum->status == 0) || (ndum->status == 2))
				{
					glowalpha = ndum->timer;
				}
				//deseneaza umbra neagra sub text
				//CSprite shadowact(ANM_PARTICLES_SPR_TEXT_DECO, 0, 0);
				//D3DXMatrixAffineTransformation2D(&mat1, ndum->fparam * 2.5f * glowalpha, NULL, 0.0f, &(ndum->pt1 - D3DXVECTOR2(0.0f, __TexFonts()[ndum->intPt.x]->rowSpacing * ndum->fparam * 0.2f)) );
				//m_pSprite->SetTransform(&mat1);
				//shadowact.color = D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, glowalpha * 0.6f);
				//shadowact.paint(&m_sprCol);
				//m_pSprite->SetTransform(&g_matIdentity);
				//deseneaza litere
				for( int ii = 0; ii < ndum->intParam2; ii++ )
				{
					RECT letterRect;
					int cod = __Texts().strings[ndum->intParam]->codes[ii];
					if((cod == K_STRMGR_SPACE)||(cod == K_STRMGR_RETURN)) 
					{
						posx += __TexFonts()[ndum->intPt.x]->spaceSize * ndum->fPtr[ii] * ndum->fparam;
						continue;
					}
					/*
					letterRect = __TexFonts()[ndum->intPt.x]->moduleRect[cod];

					int posy = camrect.CenterY() + ndum->pt1.y + __TexFonts()[ndum->intPt.x]->fmodule_oy[cod] * ndum->fparam;

					//deseneaza litera
					D3DXVECTOR3 center((letterRect.right - letterRect.left)/2.0f, (letterRect.bottom - letterRect.top)/2.0f, 0.0f);
					D3DXMatrixAffineTransformation2D(&mat1, ndum->fparam, NULL, 0.0f, &D3DXVECTOR2(posx + center.x * ndum->fparam, posy + center.y * ndum->fparam - 100.0f * ndum->fparam * (1.0f - ndum->fPtr[ii]) ));
					m_pSprite->SetTransform(&mat1);
					DWORD col = (ndum->sprite.color & 0xffffff) | ((BYTE)(ndum->fPtr[ii] * 255.0f) << 24);
					//D3DCOLOR_COLORVALUE(1.0f, 1.0f, 1.0f, ndum->fPtr[ii]);
					//m_pSprite->Draw(__TexFonts().m_texManager.GetTexture(__TexFonts()[ndum->intPt.x]->nFontsMgrTexManagerIDX), &letterRect, &center, NULL, col);
					posx += (__TexFonts()[ndum->intPt.x]->frameBBox[cod].w + __TexFonts()[ndum->intPt.x]->letterSpacing) * ndum->fparam;
					*/
				}
				m_pSprite->SetTransform(&g_matIdentity);
			}
			break;
		}
	}
}

void CParticlesManager::RemoveStringDummies()
{
	for(int kk = m_vDummies.GetSize() - 1; kk >= 0; kk--)
	{
		if(m_vDummies[kk]->fPtr != NULL)
			SAFE_DELETE_ARRAY(m_vDummies[kk]->fPtr);

		SAFE_DELETE(m_vDummies[kk]);
	}
	m_vDummies.RemoveAll();
}

CStringDummy* CParticlesManager::GetDummy(int nType)
{
	for(int kk = 0; kk < m_vDummies.GetSize(); kk++)
	{
		if(m_vDummies[kk]->type == nType)
			return m_vDummies[kk];
	}
	return NULL;
}


void CParticlesManager::RemoveStringParticles()
{
	for(int kk = m_vStringParticles.GetSize() - 1; kk >= 0; kk--)
	{
		SAFE_DELETE(m_vStringParticles[kk]);
	}
	m_vStringParticles.RemoveAll();
}


///------- PARTICLES MANAGER ---------

CParticlesManager::CParticlesManager()
{
	bInitialized = false;
	nParticlesCnt = 0;
	pParticles = NULL;
	//empty all lists
	pListFree.pNext = pListFree.pPrev = &pListFree;	//indica spre ele insele
	for (int kk = 0; kk < K_PART_LAYERS_CNT; kk++)
	{
		pList[kk].pNext = pList[kk].pPrev = &pList[kk];
	}

	fLocalTimeline = 0.0f;
	m_pDevice = NULL;
	m_pSprite = NULL;
}

CParticlesManager::~CParticlesManager()
{
	Release();
}


//Initialize
HRESULT CParticlesManager::Init(WCHAR* XMLpath, int nMaxParticlesCnt)
{
	HRESULT hr = S_OK;
	//load sprites
	V_OP_RETHR(m_sprCol.LoadSprites(XMLpath));
	//allocate particles
	nParticlesCnt = nMaxParticlesCnt;
	//asigura un numar minim de particule
	if(nMaxParticlesCnt < 100)  
		nParticlesCnt = 100;
	pParticles = new CParticle[nParticlesCnt];
	if(pParticles == NULL)
	{
		ErrorBox(K_ERR_CRITICAL, L"CParticlesManager::Init failed! Out of memory!");
		m_sprCol.Release();
		return E_OUTOFMEMORY;
	}
	//pun toate particulele in lista free
	pListFree.pNext = &pParticles[0];
	pListFree.pPrev = &pParticles[nParticlesCnt - 1];
	//set first and last particles
	pParticles[0].pNext = &pParticles[1];
	pParticles[0].pPrev = &pListFree;
	pParticles[nParticlesCnt - 1].pPrev = &pParticles[nParticlesCnt - 2];
	pParticles[nParticlesCnt - 1].pNext = &pListFree;
	//set all others
	for (int kk = 1; kk < nParticlesCnt - 1; kk++)
	{
		pParticles[kk].pNext = &pParticles[kk + 1];
		pParticles[kk].pPrev = &pParticles[kk - 1];
	}
	//golesc restul listelor
	for (int kk = 0; kk < K_PART_LAYERS_CNT; kk++)
	{
		pList[kk].pNext = pList[kk].pPrev = &pList[kk];
	}


	LOG(L"CParticlesManager::Initialized [%s] maxParticlesCnt:%d", XMLpath, nMaxParticlesCnt);
	bInitialized = true;

	return hr;
}

void CParticlesManager::Release()
{
	if (bInitialized)
	{
		bInitialized = false;
		LOG(L"CParticlesManager::Released");
	}

	RemoveStringDummies();
	RemoveStringParticles();
	//deallocate particles
	pListFree.pNext = pListFree.pPrev = &pListFree;	//indica spre ele insele
	for (int kk = 0; kk < K_PART_LAYERS_CNT; kk++)
	{
		pList[kk].pNext = pList[kk].pPrev = &pList[kk];
	}
	//sterge toate particulele
	SAFE_DELETE_ARRAY(pParticles);
	//release sprite manager
	m_sprCol.Release();

	//release particle emitters
	ReleaseAllPartEmitters();
}

void CParticlesManager::Update(float dTime)
{
	fLocalTimeline += dTime;
	//particule normale
	for (int oo = 0; oo < K_PART_LAYERS_CNT; oo++)
	{
		CParticle *part = pList[oo].pNext;
		while (part != &pList[oo])
		{
			if (part->m_fWaitTimer > 0.0f)
			{
				part->m_fWaitTimer -= dTime;
			}
			else 
			{
				if (part->bAnimated) 
					part->sprite.Update(&m_sprCol, dTime);
				//apply acceleration
				part->m_vSpeed += part->m_vGravity * dTime;
				//apply air friction
				part->m_vSpeed -= part->m_fAirFriction * part->m_vSpeed * dTime;
				//integrate position
				part->m_vPos += part->m_vSpeed * dTime;
				part->m_fRotAngle += part->m_fRotSpeed * dTime;
				part->m_fSize += part->m_fScaleSpeed * dTime;
				//--- transparenta ---
				//fade in  (nu intra cand fadeIn = 0.0f
				float newalpha = part->m_fAlpha;
				if (part->m_fLife < part->m_fFadeIn_Duration)
				{
					newalpha *= part->m_fLife / part->m_fFadeIn_Duration;
				}
				//fade out
				if (part->m_fFadeOut_Duration > 0.0f)
				{
					float lfdiff = part->m_fLifetime - part->m_fLife;
					if (lfdiff < part->m_fFadeOut_Duration)
					{
						newalpha *= lfdiff / part->m_fFadeOut_Duration;
					}
				}

				part->sprite.color = (part->m_Color & 0x00ffffff) | ((BYTE(newalpha * 255)) << 24);

				//-- verificari eliberare particula --
				part->m_fLife += dTime;
			}
			if((part->m_fLife >= part->m_fLifetime) || (part->sprite.animStatus == ANIM_STATUS_FRAMELOCK))
			{
				CParticle* nextp = part->pNext;
				//leaga vecinii ei intre ei
				part->pNext->pPrev = part->pPrev;
				part->pPrev->pNext = part->pNext;
				//adauga particula in lista celor libere, la inceput ca sa o refoloseasca
				pListFree.pNext->pPrev = part;
				part->pNext = pListFree.pNext;
				part->pPrev = &pListFree;
				pListFree.pNext = part;
				//move pointer
				part = nextp;
			}
			else
			{
				//avansez
				part = part->pNext;
			}
		}

	}
	
	//update the rest
	UpdateStringParticles(dTime);
}

void CParticlesManager::UpdateLayer(int nLayer, float dtime)
{
	CParticle *part = pList[nLayer].pNext;
	while (part != &pList[nLayer])
	{
		if (part->m_fWaitTimer > 0.0f)
		{
			part->m_fWaitTimer -= dtime;
		}
		else
		{
			if (part->bAnimated)
				part->sprite.Update(&m_sprCol, dtime);
			//apply acceleration
			part->m_vSpeed += part->m_vGravity * dtime;
			//apply air friction
			part->m_vSpeed -= part->m_fAirFriction * part->m_vSpeed * dtime;
			//integrate position
			part->m_vPos += part->m_vSpeed * dtime;
			part->m_fRotAngle += part->m_fRotSpeed * dtime;
			part->m_fSize += part->m_fScaleSpeed * dtime;
			//--- transparenta ---
			//fade in  (nu intra cand fadeIn = 0.0f
			float newalpha = part->m_fAlpha;
			if (part->m_fLife < part->m_fFadeIn_Duration)
			{
				newalpha *= part->m_fLife / part->m_fFadeIn_Duration;
			}
			//fade out
			if (part->m_fFadeOut_Duration > 0.0f)
			{
				float lfdiff = part->m_fLifetime - part->m_fLife;
				if (lfdiff < part->m_fFadeOut_Duration)
				{
					newalpha *= lfdiff / part->m_fFadeOut_Duration;
				}
			}

			part->sprite.color = (part->m_Color & 0x00ffffff) | ((BYTE(newalpha * 255)) << 24);

			//-- verificari eliberare particula --
			part->m_fLife += dtime;
		}
		if ((part->m_fLife >= part->m_fLifetime) || (part->sprite.animStatus == ANIM_STATUS_FRAMELOCK))
		{
			CParticle* nextp = part->pNext;
			//leaga vecinii ei intre ei
			part->pNext->pPrev = part->pPrev;
			part->pPrev->pNext = part->pNext;
			//adauga particula in lista celor libere, la inceput ca sa o refoloseasca
			pListFree.pNext->pPrev = part;
			part->pNext = pListFree.pNext;
			part->pPrev = &pListFree;
			pListFree.pNext = part;
			//move pointer
			part = nextp;
		}
		else
		{
			//avansez
			part = part->pNext;
		}
	}
}

void CParticlesManager::PaintLayerOffset(int nLayer, D3DXVECTOR2 offset, bool additiveBlending)
{
	assert((nLayer >= 0) && (nLayer < K_PART_LAYERS_CNT));
	D3DXMATRIXA16 mattrans;

	//layer gol deci iese
	if (pList[nLayer].pNext == &pList[nLayer])
		return;

	if (additiveBlending)
		AdditiveBlendingON(m_pDevice, m_pSprite);

	CParticle *part = pList[nLayer].pNext;
	while (part != &pList[nLayer])
	{
		if (part->m_fWaitTimer <= 0.0f)
		{
			D3DXMatrixAffineTransformation2D(&mattrans, part->m_fSize, &D3DXVECTOR2(0, 0), part->m_fRotAngle, &(part->m_vPos + offset));
			m_pSprite->SetTransform(&mattrans);
			part->sprite.paint_firstModule(&m_sprCol);
		}

		//avansez
		part = part->pNext;
	}

	m_pSprite->SetTransform(&g_matIdentity);

	if (additiveBlending)
		AdditiveBlendingOFF(m_pDevice, m_pSprite);
}

void CParticlesManager::PaintLayerOffset_texOverride(int nLayer, D3DXVECTOR2 offset, bool additiveBlending, int texIdxOffset)
{
	assert((nLayer >= 0) && (nLayer < K_PART_LAYERS_CNT));
	D3DXMATRIXA16 mattrans;

	//layer gol deci iese
	if (pList[nLayer].pNext == &pList[nLayer])
		return;

	if (additiveBlending)
		AdditiveBlendingON(m_pDevice, m_pSprite);

	CParticle *part = pList[nLayer].pNext;
	while (part != &pList[nLayer])
	{
		if (part->m_fWaitTimer <= 0.0f)
		{
			D3DXMatrixAffineTransformation2D(&mattrans, part->m_fSize, &D3DXVECTOR2(0, 0), part->m_fRotAngle, &(part->m_vPos + offset));
			m_pSprite->SetTransform(&mattrans);
			part->sprite.paint_firstModule_texOverride(&m_sprCol, texIdxOffset);
		}

		//avansez
		part = part->pNext;
	}

	m_pSprite->SetTransform(&g_matIdentity);

	if (additiveBlending)
		AdditiveBlendingOFF(m_pDevice, m_pSprite);
}


void CParticlesManager::PaintLayer(int nLayer, bool additiveBlending)
{
	assert((nLayer >= 0) && (nLayer < K_PART_LAYERS_CNT));
	D3DXMATRIXA16 mattrans;

	//layer gol deci iese
	if(pList[nLayer].pNext == &pList[nLayer])
		return;

	if(additiveBlending)
		AdditiveBlendingON(m_pDevice, m_pSprite);

	CParticle *part = pList[nLayer].pNext;
	while(part != &pList[nLayer])
	{
		if (part->m_fWaitTimer <= 0.0f)
		{
			D3DXMatrixAffineTransformation2D(&mattrans, part->m_fSize, &D3DXVECTOR2(0, 0), part->m_fRotAngle, &part->m_vPos);
			m_pSprite->SetTransform(&mattrans);
			part->sprite.paint_firstModule(&m_sprCol);
		}

		//avansez
		part = part->pNext;
	}

	m_pSprite->SetTransform(&g_matIdentity);

	if(additiveBlending)
		AdditiveBlendingOFF(m_pDevice, m_pSprite);
}

void CParticlesManager::RemoveAllFromLayer(ParticleLayers ePartLayer)
{
	if ((ePartLayer < 0) || (ePartLayer >= ParticleLayers::K_PART_LAYERS_CNT))
		return;

	CParticle *part = pList[ePartLayer].pNext;
	while (part != &pList[ePartLayer])
	{
		CParticle* nextp = part->pNext;
		//leaga vecinii ei intre ei
		part->pNext->pPrev = part->pPrev;
		part->pPrev->pNext = part->pNext;
		//adauga particula in lista celor libere, la inceput ca sa o refoloseasca
		pListFree.pNext->pPrev = part;
		part->pNext = pListFree.pNext;
		part->pPrev = &pListFree;
		pListFree.pNext = part;
		//move pointer
		part = nextp;
	}
}

void CParticlesManager::RemoveAll()
{
	//remove string particles
	RemoveStringDummies();
	RemoveStringParticles();
	//golesc toate listele
	for (int kk = 0; kk < K_PART_LAYERS_CNT; kk++)
	{
		pList[kk].pNext = pList[kk].pPrev = &pList[kk];
	}
	//pun toate particulele in lista free
	pListFree.pNext = &pParticles[0];
	pListFree.pPrev = &pParticles[nParticlesCnt - 1];
	if(pParticles)
	{
		//set first and last particles
		pParticles[0].pNext = &pParticles[1];
		pParticles[0].pPrev = &pListFree;
		pParticles[nParticlesCnt - 1].pPrev = &pParticles[nParticlesCnt - 2];
		pParticles[nParticlesCnt - 1].pNext = &pListFree;
		//set all others
		for (int kk = 1; kk < nParticlesCnt - 1; kk++)
		{
			pParticles[kk].pNext = &pParticles[kk + 1];
			pParticles[kk].pPrev = &pParticles[kk - 1];
		}
	}
	//release particle emitters
	ReleaseAllPartEmitters();
}

int CParticlesManager::GetParticleLayerByName(WCHAR * layerName)
{
	UINT32 layerNameHash = FastHash(layerName);
	for (int kk = 0; kk < K_PART_LAYERS_CNT; kk++)
	{
		if (layerNameHash == ParticleLayers_names[kk].textHash)
			return kk;
	}

	ErrorBox(K_ERR_WARNING, L"GetParticleLayerByName::Unknown layer name!");
	return -1;
}
int CParticlesManager::GetParticleLayerByName(UINT32 layerNameHash)
{
	for (int kk = 0; kk < K_PART_LAYERS_CNT; kk++)
	{
		if (layerNameHash == ParticleLayers_names[kk].textHash)
			return kk;
	}

	ErrorBox(K_ERR_WARNING, L"GetParticleLayerByName::Unknown layer hash!");
	return -1;
}


void CParticlesManager::AddParticle(int animID, bool animated, int currentFrame, D3DXVECTOR2* pos, 
					D3DXVECTOR2* gravity, D3DXVECTOR2* speed, 
					float lifetime, 
					float size, float scalespeed, 
					float rotangle, float rotspeed, 
					float fadeInDuration,
					float fadeOutDuration,
					DWORD nColor,
					int nLayer,
					float airFriction,
					float waitTimer)
{
	assert((nLayer >= 0) && (nLayer < K_PART_LAYERS_CNT));
	CParticle* newp = pListFree.pNext;
	if(newp == &pListFree)
	{
#if defined(_DEBUG) || defined(DEBUG)
		ErrorBox(K_ERR_WARNING, L"CParticlesMgr::AddParticle could not add particle. No more free particles!");
#endif
		return;
	}

	pListFree.pNext = pListFree.pNext->pNext;
	//add particle to the end of requested layer
	pList[nLayer].pPrev->pNext = newp;
	newp->pPrev = pList[nLayer].pPrev;
	pList[nLayer].pPrev = newp;
	newp->pNext = &pList[nLayer]; 
	//set particle properties
	newp->bAnimated = animated;
	newp->m_fAirFriction = airFriction;
	newp->sprite.Init(animID, 0, 0);
	newp->sprite.currentFrame = currentFrame;
	if(pos)
		newp->m_vPos = *pos;
	else
		newp->m_vPos = D3DXVECTOR2(0.0f, 0.0f);
	if(gravity)
		newp->m_vGravity = *gravity;
	else
		newp->m_vGravity = D3DXVECTOR2(0.0f, 0.0f);
	if(speed)
		newp->m_vSpeed = *speed;
	else
		newp->m_vSpeed = D3DXVECTOR2(0.0f, 0.0f);
	newp->m_fLifetime = lifetime;
	newp->m_fLife = 0.0f;
	newp->m_fScaleSpeed = scalespeed;
	newp->m_fSize = size;
	newp->m_fRotSpeed = rotspeed;
	newp->m_fRotAngle = rotangle;
	newp->m_fFadeOut_Duration = fadeOutDuration;
	newp->m_fFadeIn_Duration= fadeInDuration;

	newp->m_Color = nColor;
	newp->m_fAlpha = ((float)((newp->m_Color & 0xff000000)>>24))/255.0f;
	if(newp->m_fLife < newp->m_fFadeIn_Duration)
	{
		float newAlpha = newp->m_fAlpha * (newp->m_fLife / newp->m_fFadeIn_Duration);
		newp->sprite.color = (newp->m_Color & 0x00ffffff) | ((BYTE(newAlpha * 255))<<24);
	}

	newp->m_fWaitTimer = waitTimer;
}

///------ TAILS -------------

CTail::CTail()
{
	status = 0;
	timer = 0.0f;
	texPt1 = D3DXVECTOR2(0.0f, 0.0f);
	texPt2 = D3DXVECTOR2(1.0f, 1.0f);
	color = 0xffffffff;
	for (int kk = 0; kk < K_PART_TAIL_MAX_SIZE; kk++)
	{
		tailLife[kk] = 0.0f;
	}
}

int CParticlesManager::GetFreeTail(int AnimIDx, int frameIDx, float timerDropPerSec, float tailDropPerSec, DWORD nColor)
{
	if (AnimIDx >= m_sprCol.animationNo)
	{
		ErrorBox(K_ERR_WARNING, L"CParticlesManager::Wrong AnimIDx in GetFreeTail!");
		return 0;
	}
	if (frameIDx >= m_sprCol.Animations[AnimIDx]->aframesNo)
	{
		ErrorBox(K_ERR_WARNING, L"CParticlesManager::Wrong frameIDx in GetFreeTail!");
		return 0;
	}

	for (int kk = 0; kk < K_PART_TAIL_MAX_COUNT; kk++)
	{
		if (tails[kk].status == 0)
		{
			//reseteaza coada
			tails[kk].status = 1;
			tails[kk].timer = 0.0f;
			tails[kk].timerDropSpeed = timerDropPerSec;
			tails[kk].tailDropSpeed = tailDropPerSec;
			tails[kk].color = nColor;
			for (int ll = 0; ll < K_PART_TAIL_MAX_SIZE; ll++)
			{
				tails[kk].tailLife[ll] = -0.01f;
			}
			//set texture rect
			int aframeIdx = m_sprCol.Animations[AnimIDx]->aframesIdx[frameIDx];
			int fmoduleIDx = m_sprCol.AFrames[aframeIdx]->fmodulesIdx[0];

			RECT modrect = m_sprCol.FModules[fmoduleIDx]->moduleRect;
			float texW = (float)m_sprCol.Textures[m_sprCol.FModules[fmoduleIDx]->imgIdx]->info.Width;
			float texH = (float)m_sprCol.Textures[m_sprCol.FModules[fmoduleIDx]->imgIdx]->info.Height;

			tails[kk].texPt1.x = (float)modrect.left / texW;
			tails[kk].texPt1.y = (float)modrect.top / texH;
			tails[kk].texPt2.x = (float)modrect.right / texW;
			tails[kk].texPt2.y = (float)modrect.bottom / texH;

			return kk;
		}
	}

	ErrorBox(K_ERR_WARNING, L"No free particle tails! Defaulting to first in queue.");
	return 0;
}

void CParticlesManager::UpdateTailData(int tailIdx, D3DXVECTOR2 newPos, float newWidth)
{
	if ((tailIdx < 0) || (tailIdx >= K_PART_TAIL_MAX_COUNT))
		return;

	tails[tailIdx].width = newWidth;
	//cat timp se cheama functia asta, adauga elemente la coada
	CTail *ntail = &tails[tailIdx];

	//scrie mereu pe pozitia 0
	ntail->tailLife[0] = 1.0f;
	ntail->tailPos[0] = newPos;
	//daca primul segment nu e initializat il initializeaza aici
	if (ntail->tailLife[1] < 0.0f)
	{
		ntail->tailLife[1] = 0.0f;
		ntail->tailPos[1] = newPos;
	}

	if (ntail->timer <= 0.0f)
	{
		//muta coada si face loc in fatza pt inca o valoare
		for (int ll = K_PART_TAIL_MAX_SIZE - 1; ll > 0; ll--)
		{
			ntail->tailLife[ll] = ntail->tailLife[ll - 1];
			ntail->tailPos[ll] = ntail->tailPos[ll - 1];
		}

		ntail->timer = 1.0f;
		//scrie mereu pe pozitia 0
		ntail->tailLife[0] = 1.0f;
		ntail->tailPos[0] = newPos;
	}
}

void CParticlesManager::UpdateTails(float dTime)
{
	for (int kk = 0; kk < K_PART_TAIL_MAX_COUNT; kk++)
	{
		if (tails[kk].status == 0)
			continue;
		CTail *ntail = &tails[kk];

		ntail->timer -= dTime * ntail->timerDropSpeed;

		//face update-ul la coada
		bool allzeros = true;
		for (int ll = 0; ll < K_PART_TAIL_MAX_SIZE; ll++)
		{
			if (ntail->tailLife[ll] > 0.0f)
			{
				ntail->tailLife[ll] -= dTime * ntail->tailDropSpeed;
				if (ntail->tailLife[ll] < 0.0f)
					ntail->tailLife[ll] = 0.0f;
				allzeros = false;
			}
		}
		//daca coada s-a stins inseamna ca nu mai e folosita
		if (allzeros)
		{
			ntail->status = 0;
		}
	}
}

void CParticlesManager::PaintTails(D3DXVECTOR2* offset)
{
	AdditiveBlendingON(m_pDevice, m_pSprite);

	m_pDevice->SetFVF(VERT_TL1TS::FVF);
	//TODO: aici ar trebui sa ia textura efectiva salvata in prealabil in tail
	m_pDevice->SetTexture(0, m_sprCol.Textures[0]->pTex);

	VERT_TL1TS verts[K_PART_TAIL_MAX_SIZE * 6];
	for (int kk = 0; kk < K_PART_TAIL_MAX_COUNT; kk++)
	{
		if (tails[kk].status == 0)
			continue;
		CTail *ntail = &tails[kk];

		//deseneaza 
		int polyc = 0;
		for (int ll = 0; ll < K_PART_TAIL_MAX_SIZE; ll++)
		{
			//daca gaseste element de coada cu viatza 0 intrerupe desenarea
			if (ntail->tailLife[ll] > 0.0f)
				polyc++;
			else
				break;
		}
		polyc = min(K_PART_TAIL_MAX_SIZE - 2, polyc);

		if (polyc <= 0)
			continue;

		D3DXVECTOR3 tableoffset(0.0f, 0.0f, 0.0f);
		if (offset != NULL)
		{
			tableoffset.x = offset->x;
			tableoffset.y = offset->y;
		}

		D3DXVECTOR2 tailPos[K_PART_TAIL_MAX_SIZE];
		for (int nn = 0; nn < K_PART_TAIL_MAX_SIZE; nn++)
		{
			tailPos[nn] = ntail->tailPos[nn];
		}

		for (int ll = 0; ll <= polyc; ll++)
		{
			//directia vectorului
			D3DXVECTOR2 dir1 = ntail->tailPos[ll + 1] - ntail->tailPos[ll];
			D3DXVECTOR2 dir2;
			if (ll < polyc - 2)
				dir2 = ntail->tailPos[ll + 2] - ntail->tailPos[ll + 1];
			else
				dir2 = dir1;

			D3DXVec2Normalize(&dir2, &dir2);
			D3DXVECTOR2 tangentafar(dir2.y, -dir2.x);
			D3DXVec2Normalize(&dir1, &dir1);
			D3DXVECTOR2 tangentanear(dir1.y, -dir1.x);

			float newy = ntail->texPt1.y;
			if (ll == 0)
				newy = ntail->texPt2.y - (ntail->texPt2.y - ntail->texPt1.y) * LIMIT((1.0f - ntail->timer), 0.0f, 1.0f);
														  
			verts[ll * 6 + 0].pos = D3DXVECTOR3(tailPos[ll].x - tangentanear.x*ntail->width, tailPos[ll].y - tangentanear.y*ntail->width, 0.0f) + tableoffset;
			verts[ll * 6 + 1].pos = D3DXVECTOR3(tailPos[ll].x + tangentanear.x*ntail->width, tailPos[ll].y + tangentanear.y*ntail->width, 0.0f) + tableoffset;
			verts[ll * 6 + 2].pos = D3DXVECTOR3(tailPos[ll + 1].x - tangentafar.x*ntail->width, tailPos[ll + 1].y - tangentafar.y*ntail->width, 0.0f) + tableoffset;
			//la UV-uri se adauga textureOffset * 0.0625f pe orizontala ca sa schimbe textura particulei
			verts[ll * 6 + 0].tu = ntail->texPt1.x; verts[ll * 6 + 0].tv = newy;
			verts[ll * 6 + 1].tu = ntail->texPt2.x; verts[ll * 6 + 1].tv = newy;
			verts[ll * 6 + 2].tu = ntail->texPt1.x; verts[ll * 6 + 2].tv = ntail->texPt2.y;

			
			verts[ll * 6 + 0].color = verts[ll * 6 + 1].color = verts[ll * 6 + 3].color = DW_COLORALPHA(ntail->color, ntail->tailLife[ll]);
			verts[ll * 6 + 2].color = verts[ll * 6 + 4].color = verts[ll * 6 + 5].color = DW_COLORALPHA(ntail->color, ntail->tailLife[ll + 1]);

			verts[ll * 6 + 3].pos = D3DXVECTOR3(tailPos[ll].x + tangentanear.x*ntail->width, tailPos[ll].y + tangentanear.y*ntail->width, 0.0f) + tableoffset;
			verts[ll * 6 + 4].pos = D3DXVECTOR3(tailPos[ll + 1].x - tangentafar.x*ntail->width, tailPos[ll + 1].y - tangentafar.y*ntail->width, 0.0f) + tableoffset;
			verts[ll * 6 + 5].pos = D3DXVECTOR3(tailPos[ll + 1].x + tangentafar.x*ntail->width, tailPos[ll + 1].y + tangentafar.y*ntail->width, 0.0f) + tableoffset;
			verts[ll * 6 + 3].tu = ntail->texPt2.x; verts[ll * 6 + 3].tv = newy;
			verts[ll * 6 + 4].tu = ntail->texPt1.x; verts[ll * 6 + 4].tv = ntail->texPt2.y;
			verts[ll * 6 + 5].tu = ntail->texPt2.x; verts[ll * 6 + 5].tv = ntail->texPt2.y;
		}

		m_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, polyc * 2, verts, sizeof(VERT_TL1TS));
	}

	AdditiveBlendingOFF(m_pDevice, m_pSprite);
}

//-=-=-= PARTICLE EMITTERS =-=-=-
int CParticlesManager::GetPartEmitterTypeByNameHash(UINT32 generatorNameHash)
{
	for (int kk = 0; kk < K_PART_PE_TYPES_CNT; kk++)
	{
		if (generatorNameHash == ParticleEmitter_names[kk].textHash)
			return kk;
	}

	ErrorBox(K_ERR_WARNING, L"GetPartEmitterByNameHash::Unknown generator name!");
	return K_PART_PE_TYPE_UNKNOWN;
}

CParticleEmitter* CParticlesManager::AddPartEmitter(int nType, CAABB * pe_aabb, int nParticleLayer)
{
	if (nType == K_PART_PE_TYPE_UNKNOWN)
	{
		ErrorBox(K_ERR_WARNING, L"Particle Emitter type unknown!");
		return null;
	}

	CParticleEmitter* npe = new CParticleEmitter();

	npe->type = nType;
	npe->particleLayer = nParticleLayer;
	npe->bbox = *pe_aabb;
	npe->bbox_surface = npe->bbox.vSize.x * npe->bbox.vSize.y;
	npe->bGenerateOutsideScreen = true;
	switch (nType)
	{
		case K_PART_PE_TYPE_FOG:
		{
			npe->densityPerSurfaceUnitPerSec = 1.0f;
		}
		break;
		case K_PART_PE_TYPE_FIRE:
		{
			npe->densityPerSurfaceUnitPerSec = 50.0f;
		}
		break;
		case K_PART_PE_TYPE_FLARE:
		{
			npe->densityPerSurfaceUnitPerSec = 20.0f;
		}
		break;
		case K_PART_PE_TYPE_RAINDROPS:
		{
			npe->densityPerSurfaceUnitPerSec = 30.0f;
			npe->bGenerateOutsideScreen = false;
		}
		break;
		default:
		{
			ErrorBox(K_ERR_WARNING, L"Particle Emitter type unknown! Shouldn't get here!");
		}
		break;
	}
	//finish setting up
	npe->fGenerateTime = 1.0f / (npe->densityPerSurfaceUnitPerSec * (npe->bbox_surface / K_PART_PE_SURFACE_UNIT));
	npe->fTimer = 0.0f;

	m_arrPartEmitters.Add(npe);
	return npe;
}

void CParticlesManager::ReleasePartEmitter(CParticleEmitter* pEmit)
{
	int idx = m_arrPartEmitters.IndexOf(pEmit);
	if (idx >= 0)
		m_arrPartEmitters.Remove(idx);
}

void CParticlesManager::ReleaseAllPartEmitters()
{
	SAFE_DELETE_GROWABLE_ARRAY(m_arrPartEmitters);
}

void CParticlesManager::UpdatePartEmitters(float dTime, RECTXYWH_F screenRect)
{
	for (int kk = 0; kk < m_arrPartEmitters.GetSize(); kk++)
	{
		CParticleEmitter * npe = m_arrPartEmitters[kk];
		//este pe pauza
		if (npe->bPauseUpdate)
			continue;
		//generic updates
		bool generatePart = false;

		npe->fTimer += dTime;
		if (npe->fTimer >= npe->fGenerateTime)
		{
			generatePart = true;
			npe->fTimer -= npe->fGenerateTime;
		}
		//nu generez particule in afara ecranului
		D3DXVECTOR2 gpos(0.0f, 0.0f);
		if (generatePart)
		{
			gpos = AABB::GetRandomPointInBox(npe->bbox);
			if ((!npe->bGenerateOutsideScreen) && (!PointInRect(gpos, screenRect)))
				generatePart = false;
		}

		if (generatePart)
		{
			switch (npe->type)
			{
				case K_PART_PE_TYPE_FOG:
				{
					AddParticle(ANM_PARTICLES_SPR_SMOKE, false, randint(3), &gpos, NULL, &D3DXVECTOR2(randfloatsgn(8.0f), 0.0f), 6.0f, 0.7f, 0.2f, randfloat(PI), 0.0f, 1.0f, 2.0f, 0x22ffffff, npe->particleLayer);
				}
				break;
				case K_PART_PE_TYPE_FIRE:
				{
					AddParticle(ANM_PARTICLES_SPR_FLAME_SM, false, randint(5), &gpos, &D3DXVECTOR2(0.0f, -100.0f), &D3DXVECTOR2(randfloatsgn(1.0f), randfloatsgn(1.0f)), 1.0f, 1.0f, -0.4f, 0.0f, 0.0f, 0.1f, 0.5f, 0xffffffff, npe->particleLayer);
				}
				break;
				case K_PART_PE_TYPE_FLARE:
				{
					AddParticle(ANM_PARTICLES_SPR_SMOKE, false, randint(3), &gpos, &D3DXVECTOR2(0.0f, -100.0f), &D3DXVECTOR2(randfloatsgn(50.0f), -randfloat(40.0f)), 2.0f + randfloat(1.0f), 0.5f, 0.2f, randfloat(DOUBLE_PI), randfloatsgn(1.0f), 0.1f, 0.5f, 0x66ff8888, npe->particleLayer, 5.0f);
				}
				break;
				case K_PART_PE_TYPE_RAINDROPS:
				{
					AddParticle(ANM_PARTICLES_SPR_RAINDROP1, true, 0, &gpos, NULL, NULL, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, npe->particleLayer);
				}
				break;
			}
		}
	}
}


///- Helper Functions -////////////////////////////////////////////////////////////////////////////////////////////
void CParticlesManager::GenerateBulletHitWall(D3DXVECTOR2 npos, D3DXVECTOR2 ndir, int nLayer)
{

	D3DXVECTOR2 dir;
	D3DXVec2Normalize(&dir, &ndir);
	for (int kk = 0; kk < 5; kk++)
	{
		AddParticle(ANM_PARTICLES_SPR_DEBRIS_WALLS, false, randint(5), &npos, NULL, &(D3DXVECTOR2(dir.x + randfloatsgn(0.4f), dir.y + randfloatsgn(0.4f)) * (40.0f + randfloat(20.0f))), 0.2f, 1.0f, 0.0f, randint(4) * HALF_PI, 0.0f, 0.0f, 0.1f, 0xffffffff, nLayer);
	}
	AddParticle(ANM_PARTICLES_SPR_SPARKS, false, 0, &npos, NULL, null, 0.1f, 1.0f, -2.0f, randfloat(PI), 5.0f, 0.0f, 0.05f, 0xffffffff, nLayer);
}

void CParticlesManager::GenerateBulletHitEnemy(D3DXVECTOR2 npos, D3DXVECTOR2 ndir, int eVictimClass, int nLayer)
{
	D3DXVECTOR2 dir;
	D3DXVec2Normalize(&dir, &ndir);

	DWORD dwCol = 0xff671010; //red blood
	if (eVictimClass == K_LVL_ACT_CLASS_ZOMBIE)
		dwCol = 0xff1a3423;

	if (dir.x > 0)
	{
		AddParticle(ANM_PARTICLES_SPR_BLOODSHOT1_R + randint(3), true, 0, &npos, NULL, NULL, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, dwCol, nLayer);
	}
	else
	{
		AddParticle(ANM_PARTICLES_SPR_BLOODSHOT1_L + randint(3), true, 0, &npos, NULL, NULL, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, dwCol, nLayer);
	}
}

void CParticlesManager::GenerateBulletHitMetal(D3DXVECTOR2 npos, D3DXVECTOR2 ndir, int nLayer /*= K_PART_LAYER_NORMAL*/)
{
	D3DXVECTOR2 dir;
	D3DXVec2Normalize(&dir, &ndir);
	for (int kk = 0; kk < 8; kk++)
	{
		AddParticle(ANM_PARTICLES_SPR_FIRESPARK2, true, 2 + randint(2), &npos, NULL, &(D3DXVECTOR2(dir.x + randfloatsgn(0.2f), dir.y + randfloatsgn(0.2f)) * (40.0f + randfloat(20.0f))), 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, nLayer, 2.0f);
	}
}

void CParticlesManager::GenerateFireRing(D3DXVECTOR2 npos, int nPartCnt, float fSpeedMin, float fSpeedMax, int nLayer /*= K_PART_LAYER_NORMAL*/)
{
	for (int kk = 0; kk < nPartCnt; kk++)
	{
		float ang = randfloat(DOUBLE_PI);
		if (randompercent(50.0f))
		{
			AddParticle(ANM_PARTICLES_SPR_FIRESPARK1, true, randint(2), &npos, NULL, &(D3DXVECTOR2(cos(ang), sin(ang)) * (fSpeedMin + randfloat(fSpeedMax - fSpeedMin))), 1.0f + randfloat(0.5f), 1.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.5f, 0xffffffff, nLayer);
		}
		else
		{
			AddParticle(ANM_PARTICLES_SPR_FIRESPARK2, true, 0, &npos, NULL, &(D3DXVECTOR2(cos(ang), sin(ang)) * (fSpeedMin + randfloat(fSpeedMax - fSpeedMin))), 1.0f + randfloat(0.5f), 1.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.5f, 0xffffffff, nLayer);
		}
	}
}

void CParticlesManager::GenerateTeleportEffect(D3DXVECTOR2 npos, DWORD dwColor, int nLayer /*= K_PART_LAYER_NORMAL*/)
{
	//baza rotunda de jos
	AddParticle(ANM_PARTICLES_SPR_TELEPORT, false, 0, &npos, NULL, NULL, 0.5f, 1.0f, 0.2f, 0.0f, 0.0f, 0.1f, 0.3f, dwColor, nLayer);
	//linii verticale stelute samd
	for (int kk = 0; kk < 5; kk++)
	{
		//linii verticale
		AddParticle(ANM_PARTICLES_SPR_TELEPORT, false, 5 + randint(2), &D3DXVECTOR2(npos.x + randfloatsgn(8.0f), npos.y - 3), NULL, &D3DXVECTOR2(0.0f, -60.0f - randfloat(20.0f)), 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.2f, dwColor, nLayer, 0.0f, kk * 0.1f);
		//stele
		AddParticle(ANM_PARTICLES_SPR_TELEPORT, false, 3 + randint(2), &D3DXVECTOR2(npos.x + randfloatsgn(8.0f), npos.y - randfloat(20.0f)), NULL, &D3DXVECTOR2(0.0f, -30.0f-randfloat(10.0f)), 0.6f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.2f, dwColor, nLayer, 0.0f, kk * 0.1f);
	}
	//paranteze
	for (int kk = 0; kk < 2; kk++)
	{
		AddParticle(ANM_PARTICLES_SPR_TELEPORT, false, 1 + randint(2), &D3DXVECTOR2(npos.x + randfloatsgn(6.0f), npos.y - 1), NULL, &D3DXVECTOR2(0.0f, -40.0f - randfloat(10.0f)), 0.4f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.2f, dwColor, nLayer, 0.0f, kk * 0.1f);
	}
}

void CParticlesManager::GenerateHealEffect(D3DXVECTOR2 npos, DWORD dwColor, int nLayer /*= K_PART_LAYER_NORMAL*/)
{
	//baza rotunda de jos
	AddParticle(ANM_PARTICLES_SPR_TELEPORT, false, 0, &npos, NULL, NULL, 0.5f, 1.0f, 0.2f, 0.0f, 0.0f, 0.1f, 0.3f, dwColor, nLayer);
	for (int kk = 0; kk < 20; kk++)
	{
		//crosses
		AddParticle(ANM_PARTICLES_SPR_CROSS_SM, true, 0, &D3DXVECTOR2(npos.x + randfloatsgn(8.0f), npos.y - randfloat(20.0f)), NULL, &D3DXVECTOR2(0.0f, -30.0f - randfloat(10.0f)), 0.6f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.2f, dwColor, nLayer, 0.0f, kk * 0.02f);
	}
}

void CParticlesManager::GenerateZombieSpawn(D3DXVECTOR2 npos, DWORD dwColor, int nLayer /*= K_PART_LAYER_NORMAL*/)
{
	//round light
	AddParticle(ANM_PARTICLES_SPR_TELEPORT, false, 0, &npos, NULL, NULL, 0.5f, 1.0f, 0.2f, 0.0f, 0.0f, 0.1f, 0.3f, dwColor, nLayer);
	for (int kk = 0; kk < 10; kk++)
	{
		AddParticle(ANM_PARTICLES_SPR_SMOKE, false, randint(3), &D3DXVECTOR2(npos.x + randfloatsgn(10.0f), npos.y - randfloat(30.0f)), NULL, &D3DXVECTOR2(0.0f, -20.0f - randfloat(10.0f)), 0.8f + randfloat(0.4f), 1.0f, 0.2f, 0.0f, randfloatsgn(2.0f), 0.2f, 0.4f, dwColor, nLayer);
	}

}

void CParticlesManager::GenerateRaysHemi(D3DXVECTOR2 npos, DWORD dwColor, int nLayer /*= K_PART_LAYER_NORMAL*/)
{
	//baza rotunda de jos
	AddParticle(ANM_PARTICLES_SPR_TELEPORT, false, 0, &npos, NULL, NULL, 0.5f, 1.0f, 0.2f, 0.0f, 0.0f, 0.1f, 0.3f, dwColor, nLayer);
	//linii verticale stelute samd
	for (int kk = 0; kk < 10; kk++)
	{
		float fang = -randfloat(M_PI);
		D3DXVECTOR2 vdir = D3DXVECTOR2(cos(fang), sin(fang)) * (20.0f + randfloat(10.0f));
		//lines
		AddParticle(ANM_PARTICLES_SPR_TELEPORT, false, 5 + randint(2), &D3DXVECTOR2(npos.x + randfloatsgn(8.0f), npos.y - 3), NULL, &vdir, 0.5f, 1.0f, 1.0f, fang + HALF_PI, 0.0f, 0.2f, 0.2f, dwColor, nLayer, 0.0f, kk * 0.05f);
		//stars
		AddParticle(ANM_PARTICLES_SPR_TELEPORT, false, 3 + randint(2), &D3DXVECTOR2(npos.x + randfloatsgn(8.0f), npos.y - randfloat(20.0f)), NULL, &D3DXVECTOR2(0.0f, -30.0f - randfloat(10.0f)), 0.6f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.2f, dwColor, nLayer, 0.0f, kk * 0.05f);
	}
}

void CParticlesManager::GenerateEnemySoftGib(D3DXVECTOR2 npos, DWORD dwColor, int nLayer /*= K_PART_LAYER_NORMAL*/)
{
	float fAlpha = DW_GETFALPHA(dwColor);
	DWORD dwSmokeColor = DW_COLORALPHA(dwColor, fAlpha * 0.6f);
	//smoke
	for (int kk = 0; kk < 6; kk++)
	{
		AddParticle(ANM_PARTICLES_SPR_SMOKE_DARK, false, randint(2), &D3DXVECTOR2(npos.x + randfloatsgn(8.0f), npos.y - randfloat(20.0f)), NULL, &D3DXVECTOR2(0.0f, -10.0f-randfloat(10.0f)), 
			1.5f + randfloat(0.5f), 1.0f, 0.2f, randfloat(PI), randfloatsgn(0.4f), 0.3f, 1.0f, dwSmokeColor, nLayer, 1.3f);
	}
	//crosses
	for (int kk = 0; kk < 20; kk++)
	{
		AddParticle(ANM_PARTICLES_SPR_CROSS_SM, true, 0, &D3DXVECTOR2(npos.x + randfloatsgn(8.0f), npos.y - randfloat(20.0f)), NULL, 
			&D3DXVECTOR2(0.0f, -30.0f - randfloat(10.0f)), 0.6f, 0.7f, 0.0f, 0.0f, 0.0f, 0.2f, 0.2f, dwColor, nLayer, 0.0f, kk * 0.02f);
	}
}

void CParticlesManager::GenerateStarEffect(D3DXVECTOR2 npos, int nLayer /*= K_PART_LAYER_NORMAL*/)
{
	//fire ring
	for (int kk = 0; kk < 40; kk++)
	{
		float ang = randfloat(DOUBLE_PI);
		D3DXVECTOR2 vdir(cos(ang), sin(ang));
		if (randompercent(50.0f))
		{
			AddParticle(ANM_PARTICLES_SPR_FIRESPARK1, true, randint(2), &(npos + vdir * 10.0f), NULL, &(vdir * (20.0f + randfloat(30.0f))), 1.0f + randfloat(0.5f), 1.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.5f, 0xffffffff, nLayer);
		}
		else
		{
			AddParticle(ANM_PARTICLES_SPR_FIRESPARK2, true, 0, &(npos + vdir * 10.0f), NULL, &(vdir * (20.0f + randfloat(30.0f))), 1.0f + randfloat(0.5f), 1.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.5f, 0xffffffff, nLayer);
		}
	}

	//linii verticale
	for (int kk = 0; kk < 10; kk++)
	{
		AddParticle(ANM_PARTICLES_SPR_TELEPORT, false, 5 + randint(2), &D3DXVECTOR2(npos.x + randfloatsgn(15.0f), npos.y - 3), NULL, &D3DXVECTOR2(0.0f, -60.0f - randfloat(20.0f)), 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.2f, 0xffffffff, nLayer, 0.0f, kk * 0.1f);
	}
	//glow
	AddParticle(ANM_PARTICLES_SPR_GLOWS, false, 0, &npos, NULL, NULL, 0.3f, 0.7f, 0.3f, 0.0f, 10.0f, 0.1f, 0.1f, 0xaaffffff, nLayer);
	//add ring
	AddParticle(ANM_PARTICLES_SPR_GLOWS, false, 1, &npos, NULL, NULL, 0.4f, 0.6f, 10.0f, 0.0f, 0.0f, 0.1f, 0.3f, 0x55ffffff, nLayer);
}

void CParticlesManager::GenerateDoorBreak(D3DXVECTOR2 npos, D3DXVECTOR2 dir, int nLayer /*= K_PART_LAYER_NORMAL*/)
{
	float fAngBase = UTMath::GetVectorAngle(dir);
	D3DXVec2Normalize(&dir, &dir);
	D3DXVECTOR2 ldir;
	//praf
	for (int kk = 0; kk < 5; kk++)
	{
		ldir = dir;
		AddParticle(ANM_PARTICLES_SPR_SMOKE_DARK, false, randint(2), &(npos + ldir * (5.0f + kk * 5.0f + randfloatsgn(10.0f))), NULL, &(ldir * (30.0f + randfloat(20.0f))), 1.0f + randfloat(1.0f), 1.0f, 0.2f, randfloat(PI), ldir.x * randfloat(0.4f), 0.1f, 1.0f, 0x66ffffff, nLayer, 2.0f);
	}
	//semilune
	for (int kk = 0; kk < 5; kk++)
	{
		float fang = fAngBase + randfloatsgn(QUARTER_PI);
		ldir = D3DXVECTOR2(cos(fang), sin(fang));

		AddParticle(ANM_PARTICLES_SPR_LIGHT_HIT, false, randint(2), &(npos + ldir * randfloat(20.0f)), NULL, &(ldir * (200.0f + randfloat(100.0f))), 0.5f, 1.0f, 0.0f, fang, 0.0f, 0.1f, 0.2f, 0xffffffff, nLayer, 8.0f);
	}
	//lumini
	for (int kk = 0; kk < 10; kk++)
	{
		float fang = fAngBase + randfloatsgn(QUARTER_PI);
		ldir = D3DXVECTOR2(cos(fang), sin(fang));

		AddParticle(ANM_PARTICLES_SPR_LIGHT_HIT, false, 2, &(npos + ldir * (3.0f + randfloat(15.0f))), NULL, &(ldir * (500.0f + randfloat(100.0f))), 0.5f + randfloat(0.2f), 1.0f, 0.0f, fang, 0.0f, 0.1f, 0.3f, 0xffffffff, nLayer, 12.0f);
	}
}

void CParticlesManager::GenerateSmokePuff(D3DXVECTOR2 npos, float fRadius, int nLayer /*= K_PART_LAYER_NORMAL*/)
{
	D3DXVECTOR2 ldir;
	//praf
	for (int kk = 0; kk < 10; kk++)
	{
		float fAng = randfloat(DOUBLE_PI);
		ldir = D3DXVECTOR2(cos(fAng), sin(fAng));
		AddParticle(ANM_PARTICLES_SPR_SMOKE_DARK, false, randint(2), &(npos + ldir * randfloatsgn(fRadius)), NULL, &(ldir * (30.0f + randfloat(20.0f))), 2.5f + randfloat(1.0f), 1.0f, 0.2f, randfloat(PI), ldir.x * randfloat(0.4f), 0.1f, 1.0f, 0x66ffffff, nLayer, 2.0f);
	}
}

void CParticlesManager::GenerateHeadshot(D3DXVECTOR2 npos, D3DXVECTOR2 dir, DWORD dwColor, int nLayer /*= K_PART_LAYER_NORMAL*/)
{
	float fAngBase = UTMath::GetVectorAngle(dir);
	D3DXVec2Normalize(&dir, &dir);
	D3DXVECTOR2 ldir;
	//praf rosu
	for (int kk = 0; kk < 5; kk++)
	{
		ldir = dir;
		AddParticle(ANM_PARTICLES_SPR_SMOKE, false, randint(2), &(npos + ldir * (kk * 5.0f + randfloatsgn(5.0f))), NULL, &(ldir * (10.0f + randfloat(20.0f))), 0.6f + randfloat(0.4f), 0.5f + randfloat(0.2f), 0.2f, randfloat(PI), ldir.x * randfloat(0.4f), 0.1f, 0.4f, DW_COLORALPHA(dwColor, 0.45f), nLayer, 2.0f);
	}
	//raze
	for (int kk = 0; kk < 8; kk++)
	{
		float fang = fAngBase + randfloatsgn(QUARTER_PI);;
		ldir = D3DXVECTOR2(cos(fang), sin(fang));

		AddParticle(ANM_PARTICLES_SPR_LIGHT_HIT, false, 2, &(npos + ldir * (3.0f + randfloat(15.0f))), NULL, &(ldir * (200.0f + randfloat(50.0f))), 0.4f + randfloat(0.2f), 0.8f, 0.0f, fang, 0.0f, 0.1f, 0.3f, dwColor, nLayer, 12.0f);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CParticlesManager::OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	m_pDevice = pd3dDevice;
	return m_sprCol.OnCreateDevice(pd3dDevice);
}

HRESULT CParticlesManager::OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	m_pDevice = pd3dDevice;
	return m_sprCol.OnResetDevice(pd3dDevice);
}

HRESULT CParticlesManager::OnLostDevice(void)
{
	m_pDevice = NULL;
	return m_sprCol.OnLostDevice();
}

HRESULT CParticlesManager::OnDestroyDevice(void)
{
	m_pDevice = NULL;
	return m_sprCol.OnDestroyDevice();
}
