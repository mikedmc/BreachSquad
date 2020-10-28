#include "dxstdafx.h"
//#include "..\\dbgutil.h"
#include ".\TexturedFont.h"
#include ".\enginecommon.h"

///--- STATICS ---
//pointer comun catre strings manager ca sa nu mai folosesc extern
CStringsManager* CTexturedFont::m_pStrManager = NULL;
CTexturedFontsManager* CTexturedFont::m_pFontsManager = NULL;
LPDIRECT3DDEVICE9	CTexturedFont::pDevice = NULL;
ID3DXSprite*		CTexturedFont::s_pSprite = NULL;

void CTexturedFont::SetGlobalSpritePtr(ID3DXSprite* pSprite)
{
	s_pSprite = pSprite;
}

///--- end statics ---

CTexturedFont::CTexturedFont()
{
	loaded = false;

	pDevice	= NULL;
	nFontsMgrTexManagerIDX = -1;
	strLoadedTexture[0] = 0;
	strLoadedFile[0] = 0;

	moduleNo = 0;
	fmodule_oy = NULL;
	moduleW = moduleH = NULL;
	moduleRect = NULL;
	
	m_pStrManager = NULL;
	
	pFontReplacementTTF = NULL;
	pFontReplacementCam = NULL;
	fFontReplacementCamScaling = 1.0f;
	bFontReplacementOn = false;

	ID = 0;
	letterSpacing = FONT_MIN_LETTER_SPACING;
	rowSpacing = FONT_MIN_ROW_SPACING;
	spaceSize = FONT_MIN_SPACE_SIZE;
	rowHeight = FONT_MIN_ROW_HEIGHT;
}

CTexturedFont::~CTexturedFont()
{
	Release();
}

void CTexturedFont::SetManagersPtr(CStringsManager *strManager, CTexturedFontsManager *fontsManager)
{
	CTexturedFont::m_pStrManager = strManager;
	CTexturedFont::m_pFontsManager = fontsManager;
}


HRESULT CTexturedFont::LoadFontXML(WCHAR* XMLpath)
{
	if(loaded)
	{
		Release();
	}

	StringCchCopy(strLoadedFile, MAX_PATH, XMLpath);

	HRESULT hr = S_OK;

    pugi::xml_document doc;
	if (!doc.load_file(XMLpath))
	{
		ErrorBox(K_ERR_WARNING, L"Unable to load Font XML:%s\n", XMLpath);
		return(E_FAIL);
	}

	pugi::xml_attribute ver = doc.root().child(L"SpriteCollection").attribute(L"Version");
	if(ver.as_float() != BSX_VERSION)
	{
		ErrorBox(K_ERR_WARNING, L"SpriteCollection XML wrong version:%s\n", XMLpath);
		return E_FAIL;
	}
	
	pugi::xml_node spritenodes = doc.root().child(L"SpriteCollection");

	///--- citeste date despre font, daca sunt gasite ---
	pugi::xml_node datanode = spritenodes.child(L"FontData");
	ID = 0;
	letterSpacing = FONT_MIN_LETTER_SPACING;
	rowSpacing = FONT_MIN_ROW_SPACING;
	spaceSize = FONT_MIN_SPACE_SIZE;
	rowHeight = FONT_MIN_ROW_HEIGHT;
	if(!datanode.attribute(L"ID").empty())
	{				   
		WCHAR strID[MAX_PATH];
		StringCchCopy(strID, MAX_PATH, datanode.attribute(L"ID").value());
		ID = FastHash(strID, wcslen(strID));
		shFontName.Init(strID);

		if (m_pFontsManager == NULL)
		{
			ErrorBox(K_ERR_CRITICAL, L"CTexturedFont::LoadFontXML -> Fonts manager pointer not set! %s", strID);
			return E_FAIL;
		}
		//daca e deja incarcat, iese cu fail
		if((m_pFontsManager != NULL) && (m_pFontsManager->GetFontIdx(strID) >= 0))
		{
			ErrorBox(K_ERR_WARNING, L"CTexturedFont::LoadFontXML -> Font already loaded! %s", strID);
			return E_FAIL;
		}
	}
	else
	{
		ErrorBox(K_ERR_WARNING, L"CTexturedFont::LoadFontXML -> Nameless font!\n%s", XMLpath);
	}
	if(!datanode.attribute(L"LetterSpacing").empty())
		letterSpacing = datanode.attribute(L"LetterSpacing").as_int();
	if(!datanode.attribute(L"RowSpacing").empty())
		rowSpacing = datanode.attribute(L"RowSpacing").as_int();
	if(!datanode.attribute(L"RowHeight").empty())
		rowHeight = datanode.attribute(L"RowHeight").as_int();
	if(!datanode.attribute(L"SpaceSize").empty())
		spaceSize = datanode.attribute(L"SpaceSize").as_int();

	///--- citeste modulele ---
	pugi::xml_node modulesnode = spritenodes.child(L"Modules");
	CGrowableArray<tfModule*> tempModules;
	for (pugi::xml_node moduledata = modulesnode.first_child(); moduledata; moduledata = moduledata.next_sibling())
    {
		tfModule *nmod = new tfModule();
		nmod->X = moduledata.attribute(L"X").as_int();
		nmod->Y = moduledata.attribute(L"Y").as_int();
		nmod->W = moduledata.attribute(L"W").as_int();
		nmod->H = moduledata.attribute(L"H").as_int();
		nmod->imgIdx = moduledata.attribute(L"ImageIdx").as_int();
		tempModules.Add(nmod);
	}

	///--- citeste frame modules ---
	CGrowableArray<tfFModule*> FModules;

	pugi::xml_node framesnode = spritenodes.child(L"Frames");
	pugi::xml_node fmodulesnode = spritenodes.child(L"FrameModules");
	CGrowableArray<RECTXYWH*> tempFrameBBox;
	for (pugi::xml_node fmoduledata = fmodulesnode.first_child(), framedata = framesnode.first_child();
		fmoduledata; 
		fmoduledata = fmoduledata.next_sibling(), framedata = framedata.next_sibling())
    {
		tfFModule *nfmod = new tfFModule();
		int midx = fmoduledata.attribute(L"ModuleIdx").as_int();
		nfmod->ox = fmoduledata.attribute(L"OX").as_int();
		nfmod->oy = fmoduledata.attribute(L"OY").as_int();
		nfmod->flags = fmoduledata.attribute(L"Flags").as_uint();

		nfmod->moduleX = tempModules[midx]->X; nfmod->moduleY = tempModules[midx]->Y;
		nfmod->moduleW = tempModules[midx]->W; nfmod->moduleH = tempModules[midx]->H;
		nfmod->imgIdx = tempModules[midx]->imgIdx;
		//seteaza si RECT-ul
		SetRect(&nfmod->moduleRect, tempModules[midx]->X, tempModules[midx]->Y, tempModules[midx]->X + tempModules[midx]->W, tempModules[midx]->Y + tempModules[midx]->H);
		FModules.Add(nfmod);

		RECTXYWH *frameR = new RECTXYWH();
		frameR->x = framedata.attribute(L"BBoxX").as_int();
		frameR->y = framedata.attribute(L"BBoxY").as_int();
		frameR->w = framedata.attribute(L"BBoxW").as_int();
		frameR->h = framedata.attribute(L"BBoxH").as_int();
		tempFrameBBox.Add(frameR);
		//daca nu am setat BBox din editor ia marimea din dimensiune modul
		if (frameR->w == 0)
			frameR->w = nfmod->moduleW;
		if (frameR->h == 0)
			frameR->h = nfmod->moduleH;
	}
	//salveaza nr de litere
	moduleNo = FModules.GetSize();
	//dezaloca tempmodules
	for(int kk=0; kk<tempModules.GetSize(); kk++)
	{
		SAFE_DELETE(tempModules[kk]);
	}
	tempModules.RemoveAll();

	///--- acum aloca tot ce ii trebuie ca sa se miste rapid la desenare ---
	fmodule_ox = new int[moduleNo];
	fmodule_oy = new int[moduleNo];
	moduleW = new int[moduleNo];
	moduleH = new int[moduleNo];
	moduleRect = new RECT[moduleNo];
	frameBBox = new RECTXYWH[moduleNo]; // acelasi numar de frame-uri
	//si copiaza si datele in el
	for(int kk=0; kk < moduleNo; kk++)
	{
		fmodule_ox[kk] = FModules[kk]->ox;
		fmodule_oy[kk] = FModules[kk]->oy;
		moduleW[kk] = FModules[kk]->moduleW;
		moduleH[kk] = FModules[kk]->moduleH;
		moduleRect[kk] = FModules[kk]->moduleRect;
		frameBBox[kk] = *tempFrameBBox[kk];
	}
	//dezaloca FModules
	for (int kk = 0; kk < FModules.GetSize(); kk++)
	{
		SAFE_DELETE(FModules[kk]);
	}
	FModules.RemoveAll();

	for (int kk = 0; kk < tempFrameBBox.GetSize(); kk++)
	{
		SAFE_DELETE(tempFrameBBox[kk]);
	}
	tempFrameBBox.RemoveAll();

	///--- incarca textura ---
	//gaseste calea fisierului XML
	WCHAR szwPath[MAX_PATH];
	StringCchCopy(szwPath, MAX_PATH, XMLpath);
	int nIdx = (int)wcslen(szwPath);
	while (--nIdx > 0 && szwPath[nIdx] != '\\' && szwPath[nIdx] != '/');
	szwPath[nIdx + 1] = '\0';

	//ia doar primul nod Image gasit
	pugi::xml_node imagenode = spritenodes.child(L"Image");

	const WCHAR* imgname = imagenode.child_value();
	StringCchPrintf(strLoadedTexture, MAX_PATH, L"%s%s", szwPath, imgname);

	hr = m_pFontsManager->m_texManager.AddTexture(strLoadedTexture, &nFontsMgrTexManagerIDX, D3DFMT_A8R8G8B8, D3DX_FILTER_NONE, D3DX_FILTER_NONE);
	if(FAILED(hr))
	{
		ErrorBox(K_ERR_CRITICAL, L"CTexturedFont::LoadFontXML -> Could not load texture!\n %s", strLoadedTexture);
		Release();
	}

	loaded = true;
	LOG(L"Fonts:: Loaded:[%s] from [%s]", shFontName.text, XMLpath);

	return S_OK;
}


//face release la tot
void CTexturedFont::Release()
{
	if (loaded)
	{
		LOG(L"Fonts:: Released:[%s]", shFontName.text);
	}

	SAFE_DELETE_ARRAY(fmodule_ox);
	SAFE_DELETE_ARRAY(fmodule_oy);
	SAFE_DELETE_ARRAY(moduleW);
	SAFE_DELETE_ARRAY(moduleH);
	SAFE_DELETE_ARRAY(moduleRect);
	SAFE_DELETE_ARRAY(frameBBox);

	strLoadedFile[0] = 0;
	strLoadedTexture[0] = 0;

	loaded = false;
}

void CTexturedFont::SetFontReplacementTTF(CTTFont* pReplacementTTF, CCameraTransform* pCamTransformTTF)
{
	pFontReplacementTTF = pReplacementTTF;
	pFontReplacementCam = pCamTransformTTF;
	fFontReplacementCamScaling = 1.0f;
	if (pFontReplacementCam != null)
	{
		fFontReplacementCamScaling = pFontReplacementCam->GetCamWorldAABB().h / UTGetAppClass().g_cam240hScreen.GetCamWorldAABB().h;
	}

	bFontReplacementOn = true;
	if (pFontReplacementTTF == null)
		bFontReplacementOn = false;
}

int CTexturedFont::GetRowHeight(bool bIncludeSpacing)
{
	if ((pFontReplacementTTF != null) && (bFontReplacementOn))
	{
		return pFontReplacementTTF->nFontSize / fFontReplacementCamScaling;
	}

	int rowh = rowHeight;
	if (bIncludeSpacing)
		rowh += rowSpacing;

	return rowh;
}



int CTexturedFont::DrawStringClamped(CStringDesc *strDesc, int X, int Y, int maxW, UINT16 Flags /*= FONTFLAG_ANCHOR_BOTTOMLEFT*/, DWORD Color /*= 0xffffffff*/)
{
	//do we have a TTF replacement? paint with it instead of the normal font
	if ((pFontReplacementTTF != null) && (bFontReplacementOn))
	{
		UINT16 TTFlags = DT_SINGLELINE;
		if (Flags & FONTFLAG_ANCHOR_LEFT) TTFlags |= DT_LEFT;
		if (Flags & FONTFLAG_ANCHOR_RIGHT) TTFlags |= DT_RIGHT;
		if (Flags & FONTFLAG_ANCHOR_CENTER) TTFlags |= DT_CENTER;
		if (Flags & FONTFLAG_ANCHOR_TOP) TTFlags |= DT_TOP;
		if (Flags & FONTFLAG_ANCHOR_BOTTOM) TTFlags |= DT_BOTTOM;
		if (Flags & FONTFLAG_ANCHOR_VCENTER) TTFlags |= DT_VCENTER;

		//set new transform, bigger resolution
		if (fFontReplacementCamScaling != 1.0f)
		{
			s_pSprite->Flush();
			CCameraTransform* pOldCam = CCameraTransform::GetActiveCamera();
			CCameraTransform::SetActiveCamera(pDevice, pFontReplacementCam);
			fFontReplacementCamScaling = pFontReplacementCam->GetCamWorldAABB().h / pOldCam->GetCamWorldAABB().h;

			D3DXMATRIXA16 matLocal = g_matWorld;
			matLocal._41 *= fFontReplacementCamScaling; matLocal._42 *= fFontReplacementCamScaling;
			pDevice->SetTransform(D3DTS_WORLD, &matLocal);

			pFontReplacementTTF->DrawTextLine(strDesc->sText, X * fFontReplacementCamScaling, Y * fFontReplacementCamScaling, TTFlags, Color, rowHeight * fFontReplacementCamScaling);

			s_pSprite->Flush();
			pDevice->SetTransform(D3DTS_WORLD, &g_matWorld);
			CCameraTransform::SetActiveCamera(pDevice, pOldCam);
		}
		else
		{
			pFontReplacementTTF->DrawTextLine(strDesc->sText, X, Y, TTFlags, Color, rowHeight * fFontReplacementCamScaling);
		}

		return maxW;
	}


	LPDIRECT3DTEXTURE9 pTexture = m_pFontsManager->m_texManager.GetTexture(nFontsMgrTexManagerIDX);

	UINT16 length = strDesc->len;
	UINT16* text = strDesc->codes;

	if (length == 0)
		return 0;

	int alignOffset = 0;
	int width = 0;
	int maxHeight = rowHeight;

	//measure 3 dots length
	int dotscode = g_stringsMgr.getLetterIdx('.');
	int dotsw = 3 * (frameBBox[dotscode].w + letterSpacing);

	int localLen = 0;
	//calculeaza lungimea textului
	SIZEWH sz = MeasureString(strDesc);
	if (sz.w > maxW)
	{
		for (int ii = 0; ii < length; ii++)
		{
			int cod = text[ii];
			if (cod == K_STRMGR_RETURN)
			{
				continue;
			}
			if (cod == K_STRMGR_SPACE)
			{
				width += spaceSize;
				continue;
			}
			//height
			//if (maxHeight < -fmodule_oy[cod])
			//	maxHeight = -fmodule_oy[cod];

			if (width + frameBBox[cod].w + letterSpacing >= maxW - dotsw)
			{
				break;
			}

			width += (frameBBox[cod].w + letterSpacing);
			//save last printed char
			localLen = ii;
		}
		//adaugat latime puncte pentru centrare
		width += dotsw;
	}
	else
	{
		width = sz.w;
		//maxHeight = sz.h;
		localLen = length - 1;
	}

	if ((Flags & FONTFLAG_ANCHOR_RIGHT) != 0)
		alignOffset = width;
	else if ((Flags & FONTFLAG_ANCHOR_CENTER) != 0)
		alignOffset = width / 2;

	int posx = X - alignOffset;

	int height = maxHeight;
	if ((Flags & FONTFLAG_ANCHOR_BOTTOM) != 0)
		height = 0;
	else if ((Flags & FONTFLAG_ANCHOR_VCENTER) != 0)
		height = height / 2;
	else if ((Flags & FONTFLAG_ANCHOR_TOP) != 0)
		height = height;

	int posy = Y + height;

	int startPosX = posx; //face backup la poz de start

	for (int ii = 0; ii <= localLen; ii++)
	{
		int cod = text[ii];
		if (cod == K_STRMGR_RETURN)
		{
			continue;
		}
		if (cod == K_STRMGR_SPACE)
		{
			posx += spaceSize;
			continue;
		}

		//deseneaza litera
		s_pSprite->Draw(pTexture, &moduleRect[cod], NULL, &D3DXVECTOR3(posx + fmodule_ox[cod], posy + fmodule_oy[cod], 0.0f), Color);
		posx += frameBBox[cod].w + letterSpacing;
	}
	//deseneaza 3 puncte daca e cazul
	if (localLen < length - 1)
	{
		for (int ii = 0; ii < 3; ii++)
		{
			s_pSprite->Draw(pTexture, &moduleRect[dotscode], NULL, &D3DXVECTOR3(posx + fmodule_ox[dotscode], posy + fmodule_oy[dotscode], 0.0f), Color);
			posx += frameBBox[dotscode].w + letterSpacing;
		}
	}
	return posx - X + alignOffset;
}

int CTexturedFont::DrawStringClamped(int strIdx, int X, int Y, int maxW, UINT16 Flags /*= FONTFLAG_ANCHOR_BOTTOMLEFT*/, DWORD Color /*= 0xffffffff*/)
{
	CStringDesc *strDesc = m_pStrManager->strings[strIdx];
	return DrawStringClamped(strDesc, X, Y, maxW, Flags, Color);
}

int CTexturedFont::DrawStringScaleW(int strIdx, int X, int Y, int maxW, UINT16 Flags, DWORD Color)
{
	CStringDesc *strDesc = m_pStrManager->strings[strIdx];
	return DrawStringScaleW(strDesc, X, Y, maxW, Flags, Color);
}

int CTexturedFont::DrawStringScaleW(CStringDesc *strDesc, int X, int Y, int maxW, UINT16 Flags, DWORD Color)
{
	SIZEWH strW = MeasureString(strDesc);
	if(strW.w > maxW)
	{
		float scaleperc = (float)maxW / (float)strW.w;
		D3DXMATRIXA16 mattr;
		D3DXMatrixAffineTransformation2D(&mattr, scaleperc, NULL, 0.0f, &D3DXVECTOR2(fFontReplacementCamScaling * (X - X * scaleperc), fFontReplacementCamScaling * (Y - Y * scaleperc)));
		s_pSprite->SetTransform(&mattr);
		DrawString(strDesc, X, Y, Flags, Color);
		s_pSprite->SetTransform(&g_matIdentity);

		return maxW;
	}
	else
		return DrawString(strDesc, X, Y, Flags, Color);
}

int CTexturedFont::DrawStringScaleW(int strIdx, RECTXYWH rect, UINT16 Flags, DWORD Color)
{
	CStringDesc *strDesc = m_pStrManager->strings[strIdx];
	return DrawStringScaleW(strDesc, rect, Flags, Color);
}

int CTexturedFont::DrawStringScaleW(CStringDesc *strDesc, RECTXYWH rect, UINT16 Flags, DWORD Color)
{
	SIZEWH strW = MeasureString(strDesc);
	if (strW.w > rect.w)
	{
		D3DXVECTOR2 vCenter(rect.CenterX(), rect.CenterY());
		Flags &= ~FONTFLAG_ANCHOR_LEFT;
		Flags &= ~FONTFLAG_ANCHOR_RIGHT;
		Flags |= FONTFLAG_ANCHOR_CENTER;
		return DrawStringScaleW(strDesc, vCenter.x, vCenter.y, rect.w, Flags, Color);
	}
	else
	{
		DrawString(strDesc, rect, Flags, Color);
		return 0; 
	}
}


int CTexturedFont::DrawStringLightened(int strIdx, int X, int Y, float fLightPos, float fLightRange, UINT16 Flags, DWORD Color)
{
	return DrawStringLightened(m_pStrManager->strings[strIdx], X, Y, fLightPos, fLightRange, Flags, Color);
}

int CTexturedFont::DrawStringLightened(CStringDesc *strDesc, int X, int Y, float fLightPos, float fLightRange, UINT16 Flags, DWORD Color)
{
#if defined(_DEBUG) || defined(DEBUG)
	if(m_pStrManager == NULL)
	{
		ErrorBox(K_ERR_WARNING, L"CTexturedFont::DrawString -> m_pStrManager is NULL! Returning.");
		return 0;
	}

	if(strDesc == NULL)
	{
		ErrorBox(K_ERR_WARNING, L"CTexturedFont::DrawString -> StringDesc is NULL");
		return 0;
	}
#endif

	//do we have a TTF replacement? paint with it instead of the normal font
	if ((pFontReplacementTTF != null) && (bFontReplacementOn))
	{
		DrawString(strDesc, X, Y, Flags, Color);
		return 0;
	}


	LPDIRECT3DTEXTURE9 pTexture = m_pFontsManager->m_texManager.GetTexture(nFontsMgrTexManagerIDX);

	UINT16 length = strDesc->len;
	UINT16* text = strDesc->codes;

	if(length == 0) 
		return 0;
	
	int alignOffset = 0;
	int width = 0;
	int maxHeight = 0;
	//calculeaza lungimea textului
	for (int ii = 0; ii < length; ii++)
	{
		int cod = text[ii];
		if (cod == K_STRMGR_RETURN)
		{
			continue;
		}
		if (cod == K_STRMGR_SPACE)
		{
			width += spaceSize;
			continue;
		}
		width += (frameBBox[cod].w + letterSpacing);
		//height
		//if (maxHeight < -fmodule_oy[cod])
			//maxHeight = -fmodule_oy[cod];
		maxHeight = rowHeight;
	}

	if((Flags & FONTFLAG_ANCHOR_RIGHT) != 0) 
		alignOffset = width;
	else if((Flags & FONTFLAG_ANCHOR_CENTER) != 0) 
		alignOffset = width/2;

	int posx = X - alignOffset;
	
	int height = maxHeight;
	if((Flags & FONTFLAG_ANCHOR_BOTTOM) != 0)
		height = 0;
	else if((Flags & FONTFLAG_ANCHOR_VCENTER) != 0)
		height = height/2;
	else if((Flags & FONTFLAG_ANCHOR_TOP) != 0)
		height = height;

	int posy = Y + height;

	int startPosX = posx; //face backup la poz de start

	for(int ii=0; ii<length; ii++)	
	{
		int cod = text[ii];
		if (cod == K_STRMGR_RETURN)
		{
			continue;
		}
		if (cod == K_STRMGR_SPACE)
		{
			posx += spaceSize;
			continue;
		}

		//deseneaza litera
		s_pSprite->Draw(pTexture, &moduleRect[cod], NULL, &D3DXVECTOR3(posx + fmodule_ox[cod], posy + fmodule_oy[cod], 0.0f), Color);
		posx += frameBBox[cod].w + letterSpacing;
	}

	//lumina deasupra
	posx = startPosX;
	float lightPos = startPosX + fLightPos * width;
	if((lightPos > startPosX - fLightRange) && (lightPos < startPosX + width + fLightRange))
	{
		AdditiveBlendingON(pDevice, s_pSprite);

		for(int ii=0; ii<length; ii++)	
		{
			int cod = text[ii];
			if (cod == K_STRMGR_RETURN)
			{
				continue;
			}
			if (cod == K_STRMGR_SPACE)
			{
				posx += spaceSize;
				continue;
			}

			float alpha = max(0.0f, (1.0f - (fabs(lightPos - posx) / fLightRange)) );
			if(alpha > 0.0f)
			{
				//deseneaza litera
				s_pSprite->Draw(pTexture, &moduleRect[cod], NULL, &D3DXVECTOR3(posx + fmodule_ox[cod], posy + fmodule_oy[cod], 0.0f), D3DCOLOR_FFFA(alpha));
			}
			posx += frameBBox[cod].w + letterSpacing;
		}

		AdditiveBlendingOFF(pDevice, s_pSprite);
	}

	return posx - X + alignOffset;
}


int CTexturedFont::DrawStringTransformed(CStringDesc *strDesc, int X, int Y, float scale, float rotation, UINT16 Flags, DWORD Color)
{
	//do we have a TTF replacement? paint with it instead of the normal font
	if ((pFontReplacementTTF != null) && (bFontReplacementOn))
	{
		UINT16 TTFlags = DT_SINGLELINE | DT_NOCLIP;
		if (Flags & FONTFLAG_ANCHOR_LEFT) TTFlags |= DT_LEFT;
		if (Flags & FONTFLAG_ANCHOR_RIGHT) TTFlags |= DT_RIGHT;
		if (Flags & FONTFLAG_ANCHOR_CENTER) TTFlags |= DT_CENTER;
		if (Flags & FONTFLAG_ANCHOR_TOP) TTFlags |= DT_TOP;
		if (Flags & FONTFLAG_ANCHOR_BOTTOM) TTFlags |= DT_BOTTOM;
		if (Flags & FONTFLAG_ANCHOR_VCENTER) TTFlags |= DT_VCENTER;

		//set new transform, bigger resolution
		if (fFontReplacementCamScaling != 1.0f)
		{
			s_pSprite->Flush();
			CCameraTransform* pOldCam = CCameraTransform::GetActiveCamera();
			CCameraTransform::SetActiveCamera(pDevice, pFontReplacementCam);
			fFontReplacementCamScaling = pFontReplacementCam->GetCamWorldAABB().h / pOldCam->GetCamWorldAABB().h;

			D3DXMATRIXA16 mats;
			D3DXMatrixAffineTransformation2D(&mats, scale, NULL, rotation, &D3DXVECTOR2(X, Y));
			D3DXMATRIXA16 matLocal = mats * g_matWorld;
			matLocal._41 *= fFontReplacementCamScaling; matLocal._42 *= fFontReplacementCamScaling;

			pDevice->SetTransform(D3DTS_WORLD, &matLocal);

			pFontReplacementTTF->DrawTextLine(strDesc->sText, 0.0f, 0.0f, TTFlags, Color, rowHeight * fFontReplacementCamScaling);

			s_pSprite->Flush();
			pDevice->SetTransform(D3DTS_WORLD, &g_matWorld);
			CCameraTransform::SetActiveCamera(pDevice, pOldCam);
		}
		else
		{
			pFontReplacementTTF->DrawTextLine(strDesc->sText, X, Y, TTFlags, Color, rowHeight * fFontReplacementCamScaling);
		}

		return 0;
	}



	D3DXMATRIXA16 mats;
	D3DXMatrixAffineTransformation2D(&mats, scale, NULL, rotation, &D3DXVECTOR2(X, Y));
	s_pSprite->SetTransform(&mats);
	int retlen = DrawString(strDesc, 0, 0, Flags, Color);
	s_pSprite->SetTransform(&g_matIdentity);
	return retlen;
}

int CTexturedFont::DrawString(CStringDesc *strDesc, float X, float Y, UINT16 Flags, DWORD Color)
{
#if defined(_DEBUG) || defined(DEBUG)
	if(m_pStrManager == NULL)
	{
		ErrorBox(K_ERR_WARNING, L"CTexturedFont::DrawString -> m_pStrManager is NULL! Returning.");
		return 0;
	}

	if(strDesc == NULL)
	{
		ErrorBox(K_ERR_WARNING, L"CTexturedFont::DrawString -> StringDesc is NULL");
		return 0;
	}
#endif
	//do we have a TTF replacement? paint with it instead of the normal font
	if ((pFontReplacementTTF != null) && (bFontReplacementOn))
	{
		UINT16 TTFlags = DT_SINGLELINE | DT_NOCLIP;
		if (Flags & FONTFLAG_ANCHOR_LEFT) TTFlags |= DT_LEFT;
		if (Flags & FONTFLAG_ANCHOR_RIGHT) TTFlags |= DT_RIGHT;
		if (Flags & FONTFLAG_ANCHOR_CENTER) TTFlags |= DT_CENTER;
		if (Flags & FONTFLAG_ANCHOR_TOP) TTFlags |= DT_TOP;
		if (Flags & FONTFLAG_ANCHOR_BOTTOM) TTFlags |= DT_BOTTOM;
		if (Flags & FONTFLAG_ANCHOR_VCENTER) TTFlags |= DT_VCENTER;

		//set new transform, bigger resolution
		if (fFontReplacementCamScaling != 1.0f)
		{
			s_pSprite->Flush();
			CCameraTransform* pOldCam = CCameraTransform::GetActiveCamera();
			CCameraTransform::SetActiveCamera(pDevice, pFontReplacementCam);
			fFontReplacementCamScaling = pFontReplacementCam->GetCamWorldAABB().h / pOldCam->GetCamWorldAABB().h;

			D3DXMATRIXA16 matLocal = g_matWorld;
			matLocal._41 *= fFontReplacementCamScaling; matLocal._42 *= fFontReplacementCamScaling;
			pDevice->SetTransform(D3DTS_WORLD, &matLocal);

			pFontReplacementTTF->DrawTextLine(strDesc->sText, X * fFontReplacementCamScaling, Y * fFontReplacementCamScaling, TTFlags, Color, rowHeight * fFontReplacementCamScaling);

			s_pSprite->Flush();
			pDevice->SetTransform(D3DTS_WORLD, &g_matWorld);
			CCameraTransform::SetActiveCamera(pDevice, pOldCam);
		}
		else
		{
			pFontReplacementTTF->DrawTextLine(strDesc->sText, X, Y, TTFlags, Color, rowHeight * fFontReplacementCamScaling);
		}

		return 0;
	}


	LPDIRECT3DTEXTURE9 pTexture = m_pFontsManager->m_texManager.GetTexture(nFontsMgrTexManagerIDX);

	UINT16 length = strDesc->len;
	UINT16* text = strDesc->codes;

	if(length == 0) 
		return 0;
	
	int alignOffset = 0;
	int width = 0;
	int maxHeight = 0;
	//daca e centrat calculeaza lungimea textului
	if( (Flags & (FONTFLAG_ANCHOR_RIGHT | FONTFLAG_ANCHOR_CENTER | FONTFLAG_ANCHOR_BOTTOM | FONTFLAG_ANCHOR_VCENTER) ) != 0 )
	{
		for (int ii = 0; ii < length; ii++)
		{
			int cod = text[ii];
			if (cod == K_STRMGR_RETURN)
			{
				continue;
			}
			if (cod == K_STRMGR_SPACE)
			{
				width += spaceSize;
				continue;
			}
			width += (frameBBox[cod].w + letterSpacing);
			//calculeaza inaltimea
			maxHeight = rowHeight;
		}
	}
	else
	{
		maxHeight = rowHeight;
	}

	if((Flags & FONTFLAG_ANCHOR_RIGHT) != 0) 
		alignOffset = width;
	else if((Flags & FONTFLAG_ANCHOR_CENTER) != 0) 
		alignOffset = width/2;

	float posx = X - alignOffset;
	
	int height = maxHeight;
	if((Flags & FONTFLAG_ANCHOR_BOTTOM) != 0)
		height = 0;
	else if((Flags & FONTFLAG_ANCHOR_VCENTER) != 0)
		height = height/2;
	else if((Flags & FONTFLAG_ANCHOR_TOP) != 0)
		height = height;

	int posy = Y + height;

	for(int ii=0; ii<length; ii++)	
	{
		int cod = text[ii];
		if (cod == K_STRMGR_RETURN)
		{
			continue;
		}
		if (cod == K_STRMGR_SPACE)
		{
			posx += spaceSize;
			continue;
		}

		//deseneaza litera
		s_pSprite->Draw(pTexture, &moduleRect[cod], NULL, &D3DXVECTOR3(posx + fmodule_ox[cod], posy + fmodule_oy[cod], 0.0f), Color);
		posx += frameBBox[cod].w + letterSpacing;
	}

	return posx - X + alignOffset;
}


int CTexturedFont::DrawString(int strIdx, float X, float Y, UINT16 Flags, DWORD Color)
{
#if defined(_DEBUG) || defined(DEBUG)
	if (m_pStrManager == NULL)
	{
		ErrorBox(K_ERR_WARNING, L"CTexturedFont::DrawString -> m_pStrManager is NULL! Returning.");
		return 0;
	}

	if ((strIdx < 0) || (strIdx > m_pStrManager->strings.GetSize()))
	{
		ErrorBox(K_ERR_WARNING, L"CTexturedFont::DrawString -> Index out of range! idx=%d", strIdx);
		return 0;
	}
#endif

	LPDIRECT3DTEXTURE9 pTexture = m_pFontsManager->m_texManager.GetTexture(nFontsMgrTexManagerIDX);

	CStringDesc *strDesc = m_pStrManager->strings[strIdx];

	return DrawString(strDesc, X, Y, Flags, Color);
}

void CTexturedFont::DrawString(int strIdx, RECTXYWH rect, UINT16 Flags, DWORD Color)
{
#if defined(_DEBUG) || defined(DEBUG)
	if ((strIdx < 0) || (strIdx > m_pStrManager->strings.GetSize()))
	{
		ErrorBox(K_ERR_WARNING, L"CTexturedFont::DrawString -> Index out of range! idx=%d", strIdx);
		return;
	}
#endif

	DrawString(m_pStrManager->strings[strIdx], rect, Flags, Color);
}

int CTexturedFont::DrawStringClipped(CStringDesc *strDesc, float X, float Y, RECTXYWH clipRect, UINT16 Flags, DWORD Color)
{
#if defined(_DEBUG) || defined(DEBUG)
	if (m_pStrManager == NULL)
	{
		ErrorBox(K_ERR_WARNING, L"CTexturedFont::DrawString -> m_pStrManager is NULL! Returning.");
		return 0;
	}

	if (strDesc == NULL)
	{
		ErrorBox(K_ERR_WARNING, L"CTexturedFont::DrawString -> NULL string desc!");
		return 0;
	}
#endif

	//do we have a TTF replacement? paint with it instead of the normal font
	if ((pFontReplacementTTF != null) && (bFontReplacementOn))
	{
		UINT16 TTFlags = 0;
		if (Flags & FONTFLAG_ANCHOR_LEFT) TTFlags |= DT_LEFT;
		if (Flags & FONTFLAG_ANCHOR_RIGHT) TTFlags |= DT_RIGHT;
		if (Flags & FONTFLAG_ANCHOR_CENTER) TTFlags |= DT_CENTER;
		if (Flags & FONTFLAG_ANCHOR_TOP) TTFlags |= DT_TOP;
		if (Flags & FONTFLAG_ANCHOR_BOTTOM) TTFlags |= DT_BOTTOM;
		if (Flags & FONTFLAG_ANCHOR_VCENTER) TTFlags |= DT_VCENTER;
		
		if (Flags & FONTFLAG_WRAPTEXT) TTFlags &= ~DT_SINGLELINE;
		if (Flags & FONTFLAG_CLIPTEXT) TTFlags &= ~DT_NOCLIP;
		if ((Flags & FONTFLAG_JUSTIFY) || (Flags & FONTFLAG_WRAPTEXT)) TTFlags |= DT_WORDBREAK;

		//set new transform, bigger resolution
		if (fFontReplacementCamScaling != 1.0f)
		{
			s_pSprite->Flush();
			CCameraTransform* pOldCam = CCameraTransform::GetActiveCamera();
			CCameraTransform::SetActiveCamera(pDevice, pFontReplacementCam);
			fFontReplacementCamScaling = pFontReplacementCam->GetCamWorldAABB().h / pOldCam->GetCamWorldAABB().h;

			D3DXMATRIXA16 matLocal = g_matWorld;
			matLocal._41 *= fFontReplacementCamScaling; matLocal._42 *= fFontReplacementCamScaling;
			pDevice->SetTransform(D3DTS_WORLD, &matLocal);

			pFontReplacementTTF->DrawTextLine(strDesc->sText, X * fFontReplacementCamScaling, Y * fFontReplacementCamScaling, TTFlags, Color, rowHeight * fFontReplacementCamScaling);

			s_pSprite->Flush();
			pDevice->SetTransform(D3DTS_WORLD, &g_matWorld);
			CCameraTransform::SetActiveCamera(pDevice, pOldCam);
		}
		else
		{
			pFontReplacementTTF->DrawTextLine(strDesc->sText, X, Y, TTFlags, Color, rowHeight * fFontReplacementCamScaling);
		}

		return clipRect.w;
	}



	LPDIRECT3DTEXTURE9 pTexture = m_pFontsManager->m_texManager.GetTexture(nFontsMgrTexManagerIDX);

	UINT16 length = strDesc->len;
	UINT16* text = strDesc->codes;

	if (length == 0)
		return 0;

	int alignOffset = 0;
	int width = 0;
	int maxHeight = 0;
	//daca e centrat calculeaza lungimea textului
	if ((Flags & (FONTFLAG_ANCHOR_RIGHT | FONTFLAG_ANCHOR_CENTER | FONTFLAG_ANCHOR_TOP | FONTFLAG_ANCHOR_VCENTER)) != 0)
	{
		for (int ii = 0; ii < length; ii++)
		{
			int cod = text[ii];
			if (cod == K_STRMGR_RETURN)
			{
				continue;
			}
			if (cod == K_STRMGR_SPACE)
			{
				width += spaceSize;
				continue;
			}
			width += (frameBBox[cod].w + letterSpacing);

			if (maxHeight < -fmodule_oy[cod])
				maxHeight = -fmodule_oy[cod];
		}
	}
	else
	{
		maxHeight = rowHeight;
	}

	if ((Flags & FONTFLAG_ANCHOR_RIGHT) != 0)
		alignOffset = width;
	else if ((Flags & FONTFLAG_ANCHOR_CENTER) != 0)
		alignOffset = width / 2;

	float posx = X - alignOffset;

	int height = maxHeight;
	if ((Flags & FONTFLAG_ANCHOR_BOTTOM) != 0)
		height = 0;
	else if ((Flags & FONTFLAG_ANCHOR_VCENTER) != 0)
		height = height / 2;
	else if ((Flags & FONTFLAG_ANCHOR_TOP) != 0)
		height = maxHeight;

	int posy = Y + height;

	for (int ii = 0; ii < length; ii++)
	{
		int cod = text[ii];
		if (cod == K_STRMGR_RETURN)
		{
			continue;
		}
		if (cod == K_STRMGR_SPACE)
		{
			posx += spaceSize;
			continue;
		}

		RECT destrect;
		destrect.left = posx + fmodule_ox[cod];
		destrect.top = posy + fmodule_oy[cod];
		destrect.right = destrect.left + moduleRect[cod].right - moduleRect[cod].left;
		destrect.bottom = destrect.top + moduleRect[cod].bottom - moduleRect[cod].top;
		//eliminare daca e in afara clip
		if ((destrect.left > clipRect.x + clipRect.w) || (destrect.top > clipRect.y + clipRect.h) || (destrect.right < clipRect.x) || (destrect.bottom < clipRect.y))
		{
			posx += frameBBox[cod].w + letterSpacing;
			continue;
		}
		//decupare modul
		//stanga
		RECT modul = moduleRect[cod];
		int offx = 0, offy = 0;
		if (posx + fmodule_ox[cod] < clipRect.x)
		{
			offx = clipRect.x - (posx + fmodule_ox[cod]);
			modul.left += offx;
		}
		int rlim = posx + fmodule_ox[cod] + modul.right - modul.left;
		if (rlim > clipRect.x + clipRect.w)
			modul.right -= rlim - (clipRect.x + clipRect.w);
		if (posy + fmodule_oy[cod] < clipRect.y)
		{
			offy = clipRect.y - (posy + fmodule_oy[cod]);
			modul.top += offy;
		}
		int blim = posy + fmodule_oy[cod] + modul.bottom - modul.top;
		if (blim > clipRect.y + clipRect.h)
			modul.bottom -= blim - (clipRect.y + clipRect.h);
		//deseneaza litera
		s_pSprite->Draw(pTexture, &modul, NULL, &D3DXVECTOR3(posx + fmodule_ox[cod] + offx, posy + fmodule_oy[cod] + offy, 0.0f), Color);
		posx += frameBBox[cod].w + letterSpacing;
	}

	return posx - X + alignOffset;
}

int CTexturedFont::DrawStringClipped(int strIdx, float X, float Y, RECTXYWH clipRect, UINT16 Flags, DWORD Color)
{
#if defined(_DEBUG) || defined(DEBUG)
	if ((strIdx < 0) || (strIdx > m_pStrManager->strings.GetSize()))
	{
		ErrorBox(K_ERR_WARNING, L"CTexturedFont::DrawString -> Index out of range! idx=%d", strIdx);
		return 0;
	}
#endif

	LPDIRECT3DTEXTURE9 pTexture = m_pFontsManager->m_texManager.GetTexture(nFontsMgrTexManagerIDX);

	CStringDesc *strDesc = m_pStrManager->strings[strIdx];
	return DrawStringClipped(strDesc, X, Y, clipRect, Flags, Color);
}

void CTexturedFont::DrawString(CStringDesc *strDesc, RECTXYWH rect, UINT16 Flags, DWORD Color)
{

#if defined(_DEBUG) || defined(DEBUG)
	if(m_pStrManager == NULL)
	{
		ErrorBox(K_ERR_WARNING, L"CTexturedFont::DrawString -> m_pStrManager is NULL! Returning.");
		return;
	}
#endif

	//do we have a TTF replacement? paint with it instead of the normal font
	if ((pFontReplacementTTF != null) && (bFontReplacementOn))
	{
		UINT16 TTFlags = DT_NOCLIP;
		if (Flags & FONTFLAG_ANCHOR_LEFT) TTFlags |= DT_LEFT;
		if (Flags & FONTFLAG_ANCHOR_RIGHT) TTFlags |= DT_RIGHT;
		if (Flags & FONTFLAG_ANCHOR_CENTER) TTFlags |= DT_CENTER;
		if (Flags & FONTFLAG_ANCHOR_TOP) TTFlags |= DT_TOP;
		if (Flags & FONTFLAG_ANCHOR_BOTTOM) TTFlags |= DT_BOTTOM;
		if (Flags & FONTFLAG_ANCHOR_VCENTER) TTFlags |= DT_VCENTER;
		
		if (Flags & FONTFLAG_WRAPTEXT) TTFlags &= ~DT_SINGLELINE;
		if (Flags & FONTFLAG_CLIPTEXT) TTFlags &= ~DT_NOCLIP;
		if ((Flags & FONTFLAG_JUSTIFY) || (Flags & FONTFLAG_WRAPTEXT)) TTFlags |= DT_WORDBREAK;
		//set new transform, bigger resolution
		if (fFontReplacementCamScaling != 1.0f)
		{
			s_pSprite->Flush();
			CCameraTransform* pOldCam = CCameraTransform::GetActiveCamera();
			CCameraTransform::SetActiveCamera(pDevice, pFontReplacementCam);
			fFontReplacementCamScaling = pFontReplacementCam->GetCamWorldAABB().h / pOldCam->GetCamWorldAABB().h;

			D3DXMATRIXA16 matLocal = g_matWorld;
			matLocal._41 *= fFontReplacementCamScaling; matLocal._42 *= fFontReplacementCamScaling;
			pDevice->SetTransform(D3DTS_WORLD, &matLocal);

			RECT rc;
			//top aligned by default:
			int offy = (pFontReplacementTTF->nFontSize - rowHeight) / 2;
			SetRect(&rc, rect.x * fFontReplacementCamScaling, rect.y * fFontReplacementCamScaling - offy,
				rect.Right() * fFontReplacementCamScaling, rect.Bottom() * fFontReplacementCamScaling + offy);
			pFontReplacementTTF->pFont->DrawTextW(s_pSprite, strDesc->sText, -1, &rc, TTFlags, Color);

			s_pSprite->Flush();
			pDevice->SetTransform(D3DTS_WORLD, &g_matWorld);
			CCameraTransform::SetActiveCamera(pDevice, pOldCam);
		}
		else
		{
			RECT rc;
			SetRect(&rc, rect.x, rect.y, rect.Right(), rect.Bottom());
			pFontReplacementTTF->pFont->DrawTextW(s_pSprite, strDesc->sText, -1, &rc, TTFlags, Color);
		}

		return;
	}

	LPDIRECT3DTEXTURE9 pTexture = m_pFontsManager->m_texManager.GetTexture(nFontsMgrTexManagerIDX);

	int textLen = strDesc->len;
	UINT16* textCodes = strDesc->codes;

	if(textLen == 0) 
		return;

	if((Flags & FONTFLAG_WRAPTEXT) != 0) //WRAP text
	{
		int verticalOffset = 0;
		//daca am aliniere pe verticala masor stringul pe verticala
		if ((Flags & FONTFLAG_ANCHOR_VCENTER) || (Flags & FONTFLAG_ANCHOR_BOTTOM))
		{
			SIZEWH textsize = MeasureString(strDesc, rect.w);
			if (Flags & FONTFLAG_ANCHOR_BOTTOM)
				verticalOffset = rect.h - textsize.h;
			else
				verticalOffset = (rect.h - textsize.h) / 2;
		}


		int posx = rect.x, posy = rect.y + rowHeight + verticalOffset;

		int countedSpaces = 0;

		int paintStart = 0;
		int lastSpace = 0;
		int lastSpaceWidth = 0;
		int tmpWidth = 0;
		int textCur = 0;

		while(textCur < textLen)
		{
			UINT16 code = textCodes[textCur];

			if(code == K_STRMGR_SPACE)
			{
				lastSpace = textCur;
				lastSpaceWidth = tmpWidth;

				tmpWidth += spaceSize;
				countedSpaces++;
			}
			else if(code == K_STRMGR_RETURN)
			{
				lastSpaceWidth = tmpWidth;
				lastSpace = textCur;

				tmpWidth += rect.w + 1;
				countedSpaces = 0;
			}
			else
				tmpWidth += frameBBox[code].w + letterSpacing;
			//daca a ajuns la final forteaza desenare  ca sa goleasca ce a mai ramas de desenat
			if(textCur >= textLen-1)
			{
				lastSpaceWidth = tmpWidth;
				tmpWidth += rect.w + 1;
				lastSpace = textLen;
				countedSpaces = 0;
			}

			if(tmpWidth > rect.w) //deseneaza de la paintStart pana la lastSpace
			{
				float spaceAdder = 0.0f;
				float spaceStep = 0.0f;
				countedSpaces--;
				if(Flags & FONTFLAG_JUSTIFY)
				{
					if(countedSpaces > 0)
					{
						spaceStep = (rect.w - lastSpaceWidth) / countedSpaces;
						if(spaceStep > 3.0f * spaceSize)
							spaceStep = 3.0f * spaceSize;
					}
					else //daca nu face justify, face aliniere
					{
						//aliniere centru
						if(Flags & FONTFLAG_ANCHOR_CENTER)
							posx += ((rect.w - lastSpaceWidth) >> 1);
						else if(Flags & FONTFLAG_ANCHOR_RIGHT)
							posx += (rect.w - lastSpaceWidth);
					}
				}
				else//daca nu e pe justify
				{
					if(Flags & FONTFLAG_ANCHOR_CENTER)
						posx += ((rect.w - lastSpaceWidth) >> 1);
					else if(Flags & FONTFLAG_ANCHOR_RIGHT)
						posx += (rect.w - lastSpaceWidth);
				}

				if((Flags & FONTFLAG_CLIPTEXT) == 0)
				{
					//fara clip la rectangle
					for(int kk = paintStart; kk < lastSpace; kk++)
					{
						int cod2 = textCodes[kk];
						if((cod2 == K_STRMGR_SPACE)||(cod2 == K_STRMGR_RETURN)) 
						{
							spaceAdder += spaceStep;
							posx += spaceSize + (int)floor(spaceAdder);
							spaceAdder -= floor(spaceAdder);
							continue;
						}
						//deseneaza litera
						s_pSprite->Draw(pTexture, &moduleRect[cod2], NULL, &D3DXVECTOR3(posx + fmodule_ox[cod2], posy + fmodule_oy[cod2], 0.0f), Color);
						posx += frameBBox[cod2].w + letterSpacing;
					}
				}
				else
				{
					//cu clip
					for(int kk = paintStart; kk < lastSpace; kk++)
					{
						int cod2 = textCodes[kk];
						if((cod2 == K_STRMGR_SPACE)||(cod2 == K_STRMGR_RETURN)) 
						{
							spaceAdder += spaceStep;
							posx += spaceSize + (int)floor(spaceAdder);
							spaceAdder -= floor(spaceAdder);
							continue;
						}
						//deseneaza litera
						RECT letterRect = moduleRect[cod2];
						if((posy + fmodule_oy[cod2] > rect.y + rect.h) || (posy + fmodule_oy[cod2] + moduleH[cod2] < rect.y)) 
						{
							posx += frameBBox[cod2].w + letterSpacing;
							continue;
						}
						/* //aici facea clip in partea de sus a literei, dar cum nu am offset nu este cazul
						if(posy + fmodule_oy[cod2] < rect.y)
						{
							letterRect.top += (rect.y - posy - fmodule_oy[cod2]);
						}
						else */
						if(posy + moduleH[cod2] + fmodule_oy[cod2] > rect.y + rect.h)
						{
							letterRect.bottom -= (posy + moduleH[cod2] + fmodule_oy[cod2] - rect.y - rect.h);
						}
						s_pSprite->Draw(pTexture, &letterRect, NULL, &D3DXVECTOR3(posx + fmodule_ox[cod2], posy + fmodule_oy[cod2], 0.0f), Color);
						posx += frameBBox[cod2].w + letterSpacing;
					}
				}
				//reseteaza
				paintStart = lastSpace + 1;

				// daca spatiul orizontal e mai mic decat un cuvant, iese
				if (lastSpace == 0)
				{
					return;
				}

				textCur = lastSpace;
				tmpWidth = 0;

				posx = rect.x;
				posy += rowHeight + rowSpacing;
				countedSpaces = 0;
			}

			textCur++;
		}
	}
	else //nu face WRAP, deci ia alinierea in fn de dreptunghiul respectiv
	{
		int posx = rect.x, posy = rect.y + rowHeight;
		if(Flags & FONTFLAG_ANCHOR_BOTTOM)
			posy = rect.y + rect.h;
		else if(Flags & FONTFLAG_ANCHOR_VCENTER)
			posy = rect.y + ((rect.h + rowHeight) >> 1);

		int width = 0;
		//daca e centrat calculeaza lungimea textului
		if( (Flags & (FONTFLAG_ANCHOR_RIGHT | FONTFLAG_ANCHOR_CENTER) ) != 0 )
		{
			for(int ii = 0; ii < textLen; ii++)
			{			
				int cod = textCodes[ii];			
				if((cod == K_STRMGR_SPACE) || (cod == K_STRMGR_RETURN))
				{
					width += spaceSize;
					continue;
				}
				width += (frameBBox[cod].w + letterSpacing);
			}		
		}

		if((Flags & FONTFLAG_ANCHOR_RIGHT) != 0) 
			posx += rect.w - width;
		else if((Flags & FONTFLAG_ANCHOR_CENTER) != 0) 
			posx += (rect.w - width) >> 1;
		
		for(int ii=0; ii<textLen; ii++)	
		{
			int cod = textCodes[ii];
			if((cod == K_STRMGR_SPACE)||(cod == K_STRMGR_RETURN)) 
			{
				posx += spaceSize;
				continue;
			}

			//deseneaza litera
			if((Flags & FONTFLAG_CLIPTEXT) == 0)
			{
				s_pSprite->Draw(pTexture, &moduleRect[cod], NULL, &D3DXVECTOR3(posx + fmodule_ox[cod], posy + fmodule_oy[cod], 0.0f), Color);
				posx += frameBBox[cod].w + letterSpacing;
			}
			else
			{
				int offx = 0;
				RECT letterRect = moduleRect[cod];
				if((posx > rect.x + rect.w) || (posx + frameBBox[cod].w < rect.x)) 
				{
					posx += frameBBox[cod].w + letterSpacing;
					continue;
				}
				if(posx < rect.x)
				{
					letterRect.left += (rect.x - posx);
					offx = rect.x - posx;
				}
				else if (posx + frameBBox[cod].w > rect.x + rect.w)
				{
					letterRect.right -= (posx + frameBBox[cod].w - rect.x - rect.w);
				}
				s_pSprite->Draw(pTexture, &letterRect, NULL, &D3DXVECTOR3(posx + fmodule_ox[cod], posy + fmodule_oy[cod], 0.0f), Color);
				posx += frameBBox[cod].w + letterSpacing;
			}
		}
	}
}

void CTexturedFont::DrawStringOffsetY(int strIdx, RECTXYWH rect, int offsetY, UINT16 Flags, DWORD Color)
{
#if defined(_DEBUG) || defined(DEBUG)
	if(m_pStrManager == NULL)
	{
		ErrorBox(K_ERR_WARNING, L"CTexturedFont::DrawString -> m_pStrManager is NULL! Returning.");
		return;
	}

	if((strIdx < 0) || (strIdx > m_pStrManager->strings.GetSize()))
	{
		ErrorBox(K_ERR_WARNING, L"CTexturedFont::DrawString -> Index out of range! idx=%d", strIdx);
		return;
	}
#endif

	if ((pFontReplacementTTF != null) && (bFontReplacementOn))
	{
		DrawString(strIdx, rect, Flags, Color);
		return;
	}


	LPDIRECT3DTEXTURE9 pTexture = m_pFontsManager->m_texManager.GetTexture(nFontsMgrTexManagerIDX);

	int textLen = m_pStrManager->strings[strIdx]->len;
	UINT16* textCodes = m_pStrManager->strings[strIdx]->codes;

	if(textLen == 0) 
		return;

	int posx = rect.x;
	int posy = rect.y + rowHeight + offsetY;

	int countedSpaces = 0;

	int paintStart = 0;
	int lastSpace = 0;
	int lastSpaceWidth = 0;
	int tmpWidth = 0;
	int textCur = 0;

	while(textCur < textLen)
	{
		UINT16 code = textCodes[textCur];

		if(code == K_STRMGR_SPACE)
		{
			lastSpace = textCur;
			lastSpaceWidth = tmpWidth;

			tmpWidth += spaceSize;
			countedSpaces++;
		}
		else if(code == K_STRMGR_RETURN)
		{
			lastSpaceWidth = tmpWidth;
			lastSpace = textCur;

			tmpWidth += rect.w + 1;
			countedSpaces = 0;
		}
		else
			tmpWidth += frameBBox[code].w + letterSpacing;
		//daca a ajuns la final forteaza desenare  ca sa goleasca ce a mai ramas de desenat
		if(textCur >= textLen-1)
		{
			lastSpaceWidth = tmpWidth;
			tmpWidth += rect.w + 1;
			lastSpace = textLen;
			countedSpaces = 0;
		}

		if(tmpWidth > rect.w) //deseneaza de la paintStart pana la lastSpace
		{
			float spaceAdder = 0.0f;
			float spaceStep = 0.0f;
			countedSpaces--;
			if(Flags & FONTFLAG_JUSTIFY)
			{
				if(countedSpaces > 0)
				{
					spaceStep = (rect.w - lastSpaceWidth) / countedSpaces;
					if(spaceStep > 3.0f * spaceSize)
						spaceStep = 3.0f * spaceSize;
				}
				else //daca nu face justify, face aliniere
				{
					//aliniere centru
					if(Flags & FONTFLAG_ANCHOR_CENTER)
						posx += ((rect.w - lastSpaceWidth) >> 1);
					else if(Flags & FONTFLAG_ANCHOR_RIGHT)
						posx += (rect.w - lastSpaceWidth);
				}
			}
			else//daca nu e pe justify
			{
				if(Flags & FONTFLAG_ANCHOR_CENTER)
					posx += ((rect.w - lastSpaceWidth) >> 1);
				else if(Flags & FONTFLAG_ANCHOR_RIGHT)
					posx += (rect.w - lastSpaceWidth);
			}

			if((Flags & FONTFLAG_CLIPTEXT) == 0)
			{
				//fara clip la rectangle
				for(int kk = paintStart; kk < lastSpace; kk++)
				{
					int cod2 = textCodes[kk];
					if ((cod2 == K_STRMGR_SPACE) || (cod2 == K_STRMGR_RETURN))
					{
						spaceAdder += spaceStep;
						posx += spaceSize + (int)floor(spaceAdder);
						spaceAdder -= floor(spaceAdder);
						continue;
					}
					//deseneaza litera
					s_pSprite->Draw(pTexture, &moduleRect[cod2], NULL, &D3DXVECTOR3(posx + fmodule_ox[cod2], posy + fmodule_oy[cod2], 0.0f), Color);
					posx += frameBBox[cod2].w + letterSpacing;
				}
			}
			else
			{
				//cu clip
				for(int kk = paintStart; kk < lastSpace; kk++)
				{
					int cod2 = textCodes[kk];
					if ((cod2 == K_STRMGR_SPACE) || (cod2 == K_STRMGR_RETURN))
					{
						spaceAdder += spaceStep;
						posx += spaceSize + (int)floor(spaceAdder);
						spaceAdder -= floor(spaceAdder);
						continue;
					}
					int loffy = 0;
					//deseneaza litera
					RECT letterRect = moduleRect[cod2];
					if((posy + fmodule_oy[cod2] > rect.y + rect.h) || (posy + fmodule_oy[cod2] + moduleH[cod2] < rect.y)) 
					{
						posx += frameBBox[cod2].w + letterSpacing;
						continue;
					}
					if(posy + fmodule_oy[cod2] < rect.y)
					{
						loffy = (rect.y - posy - fmodule_oy[cod2]);
						letterRect.top += loffy;
					}
					else if(posy + moduleH[cod2] + fmodule_oy[cod2] > rect.y + rect.h) 
					{
						letterRect.bottom -= (posy + moduleH[cod2] + fmodule_oy[cod2] - rect.y - rect.h);
					}
					s_pSprite->Draw(pTexture, &letterRect, NULL, &D3DXVECTOR3(posx + fmodule_ox[cod2], posy + fmodule_oy[cod2] + loffy, 0.0f), Color);
					posx += frameBBox[cod2].w + letterSpacing;
				}
			}
			//reseteaza
			paintStart = lastSpace + 1;
			textCur = lastSpace;
			tmpWidth = 0;

			posx = rect.x;
			posy += rowHeight + rowSpacing;
			countedSpaces = 0;
		}

		textCur++;
	}
}


SIZEWH CTexturedFont::MeasureString(CStringDesc *strDesc)
{
	SIZEWH retsz(0, rowHeight);
	if ((pFontReplacementTTF != null) && (bFontReplacementOn))
	{
		UINT16 TTFlags = DT_CALCRECT | DT_NOCLIP | DT_SINGLELINE;
		RECT rc;
		SetRect(&rc, 0, 0, 0, 0);
		pFontReplacementTTF->pFont->DrawTextW(s_pSprite, strDesc->sText, -1, &rc, TTFlags, 0xffffffff);
		retsz.w = (rc.right - rc.left) / fFontReplacementCamScaling;
		retsz.h = (rc.bottom - rc.top) / fFontReplacementCamScaling;
		return retsz;
	}

	for (UINT ii = 0; ii < strDesc->len; ii++)
	{
		int cod = strDesc->codes[ii];
		if (cod == K_STRMGR_SPACE)
		{
			retsz.w += spaceSize;
			continue;
		}
		else if (cod == K_STRMGR_RETURN)
		{
			retsz.h += rowHeight + rowSpacing;
			continue;
		}
		retsz.w += (frameBBox[cod].w + letterSpacing);
	}

	return retsz;
}

SIZEWH CTexturedFont::MeasureString(int strIdx)
{
	SIZEWH retsz(0, rowHeight);
	CStringDesc *strDesc = m_pStrManager->strings[strIdx];

	return MeasureString(strDesc);
}

SIZEWH CTexturedFont::MeasureString(int strIdx, int nMaxWidth)
{
	int maxWidth = nMaxWidth;
	SIZEWH retsz(maxWidth, rowHeight);

	return MeasureString(m_pStrManager->strings[strIdx], nMaxWidth);
}

SIZEWH CTexturedFont::MeasureString(CStringDesc* strDesc, int nMaxWidth)
{
	int maxWidth = nMaxWidth;
	SIZEWH retsz(maxWidth, rowHeight);

	if ((pFontReplacementTTF != null) && (bFontReplacementOn))
	{
		UINT16 TTFlags = DT_CALCRECT | DT_NOCLIP | DT_WORDBREAK;
		RECT rc;
		SetRect(&rc, 0, 0, nMaxWidth, 10);
		pFontReplacementTTF->pFont->DrawTextW(s_pSprite, strDesc->sText, -1, &rc, TTFlags, 0xffffffff);
		retsz.w = (rc.right - rc.left) / fFontReplacementCamScaling;
		retsz.h = (rc.bottom - rc.top) / fFontReplacementCamScaling;
		return retsz;
	}


	int textLen = strDesc->len;
	UINT16* textCodes = strDesc->codes;

	if (textLen == 0)
		return retsz;

	int posy = 0;//rowHeight;

	int paintStart = 0;
	int lastSpace = 0;
	int tmpWidth = 0;
	int textCur = 0;

	while (textCur < textLen)
	{
		UINT16 code = textCodes[textCur];

		if (code == K_STRMGR_SPACE)
		{
			lastSpace = textCur;
			tmpWidth += spaceSize;
		}
		else if (code == K_STRMGR_RETURN)
		{
			lastSpace = textCur;
			tmpWidth = maxWidth + 1;
		}
		else
			tmpWidth += frameBBox[code].w + letterSpacing;
		//daca a ajuns la final forteaza ca sa goleasca ce a mai ramas
		if (textCur >= textLen - 1)
		{
			if (posy == 0) //daca e pe un singur rand setez deja latimea
				retsz.w = tmpWidth;

			tmpWidth = maxWidth + 1;
			lastSpace = textLen;
		}

		if (tmpWidth > maxWidth) //calucleaza de la paintStart pana la lastSpace
		{
			// make sure we don't split if we had no space yet
			if (lastSpace != 0)
			{
				//reset x, new row
				paintStart = lastSpace + 1;
				textCur = lastSpace;
				tmpWidth = 0;

				posy += rowHeight + rowSpacing;
			}
		}

		textCur++;
	}

	retsz.h = posy;
	return retsz;
}

///--- functiile care adreseaza stringul prin hash ---
int CTexturedFont::DrawHString(UINT32 strHash, int X, int Y, UINT16 Flags, DWORD Color)
{
	int strIdx = m_pStrManager->getStrIdx(strHash);
	return DrawString(strIdx, X, Y, Flags, Color);
}

void CTexturedFont::DrawHString(UINT32 strHash, RECTXYWH rect, UINT16 Flags, DWORD Color)
{
	int strIdx = m_pStrManager->getStrIdx(strHash);
	DrawString(strIdx, rect, Flags, Color);
}

void CTexturedFont::DrawHStringOffsetY(UINT32 strHash, RECTXYWH rect, int offsetY, UINT16 Flags, DWORD Color)
{
	int strIdx = m_pStrManager->getStrIdx(strHash);
	DrawStringOffsetY(strIdx, rect, offsetY, Flags, Color);
}

SIZEWH CTexturedFont::MeasureHString(UINT32 strHash)
{
	int strIdx = m_pStrManager->getStrIdx(strHash);

	SIZEWH retsz(0, rowHeight);

	CStringDesc *strDesc = m_pStrManager->strings[strIdx];
	for(UINT ii = 0; ii < strDesc->len; ii++)
	{			
		int cod = strDesc->codes[ii];			
		if (cod == K_STRMGR_RETURN)
			continue;
		if(cod == K_STRMGR_SPACE)
		{
			retsz.w += spaceSize;
			continue;
		}
		retsz.w += (frameBBox[cod].w + letterSpacing);
	}		

	return retsz;
}

SIZEWH CTexturedFont::MeasureHString(UINT32 strHash, int maxWidth)
{
	int strIdx = m_pStrManager->getStrIdx(strHash);
	SIZEWH retsz(maxWidth, 0);

	int textLen = m_pStrManager->strings[strIdx]->len;
	UINT16* textCodes = m_pStrManager->strings[strIdx]->codes;

	if(textLen == 0) 
		return retsz;

	int posy = 0;//rowHeight;

	int paintStart = 0;
	int lastSpace = 0;
	int tmpWidth = 0;
	int textCur = 0;

	while(textCur < textLen)
	{
		UINT16 code = textCodes[textCur];

		if(code == K_STRMGR_SPACE)
		{
			lastSpace = textCur;
			tmpWidth += spaceSize;
		}
		else if(code == K_STRMGR_RETURN)
		{
			lastSpace = textCur;
			tmpWidth += maxWidth + 1;
		}
		else
			tmpWidth += frameBBox[code].w + letterSpacing;
		//daca a ajuns la final forteaza ca sa goleasca ce a mai ramas
		if(textCur >= textLen-1)
		{
			tmpWidth += maxWidth + 1;
			lastSpace = textLen;
		}

		if(tmpWidth > maxWidth) //calucleaza de la paintStart pana la lastSpace
		{
			// make sure we don't split if we had no space yet
			if (lastSpace != 0)
			{
				//reset x, new row
				paintStart = lastSpace + 1;
				textCur = lastSpace;
				tmpWidth = 0;

				posy += rowHeight + rowSpacing;
			}
		}

		textCur++;
	}

	retsz.h = posy;
	return retsz;
}


///--- system framework ---
HRESULT CTexturedFont::OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr = S_OK;

	pDevice = pd3dDevice;

	return hr;
}

HRESULT CTexturedFont::OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	pDevice = pd3dDevice;

	return S_OK;
}

HRESULT CTexturedFont::OnLostDevice(void)
{
	pDevice = NULL;
	return S_OK;
}

HRESULT CTexturedFont::OnDestroyDevice(void)
{
	HRESULT hr = S_OK;

	pDevice = NULL;
	return hr;
}


  //********************************************************************************
 // Fonts Manager
//********************************************************************************
int CTexturedFontsManager::GetFontIdx(const CHAR* fontID)
{
	UINT32 fhash = FastHash(fontID, strlen(fontID));
	for(int kk=0; kk<fonts.GetSize(); kk++)
	{
		if(fonts[kk]->ID == fhash)
			return kk;
	}

	//ErrorBox(K_ERR_WARNING, L"GetFontIdx could not find fontID %s\n", fontID);
	return -1;
}

int CTexturedFontsManager::GetFontIdx(const WCHAR* fontID)
{
	UINT32 fhash = FastHash(fontID, wcslen(fontID));
	for(int kk=0; kk<fonts.GetSize(); kk++)
	{
		if(fonts[kk]->ID == fhash)
			return kk;
	}
	
	//ErrorBox(K_ERR_WARNING, L"GetFontIdx could not find fontID %s\n", fontID);
	return -1;
}

CTexturedFont* CTexturedFontsManager::operator[] (const CHAR* fontID)
{
	int idx = GetFontIdx(fontID);
	assert((idx >= 0) && (idx < fonts.GetSize()));
	return fonts[idx];
}

CTexturedFont* CTexturedFontsManager::operator[] (const int fontIdx)
{
	assert((fontIdx >= 0) && (fontIdx < fonts.GetSize()));
	return fonts[fontIdx];
}


///--- system framework ---
HRESULT CTexturedFontsManager::OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr = S_OK;
	pDevice = pd3dDevice;
	m_texManager.OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc);
	for(int kk=0; kk<fonts.GetSize(); kk++)
	{
		V_RETURN(fonts[kk]->OnCreateDevice(pd3dDevice, pBackBufferSurfaceDesc));
	}
	return hr;
}

HRESULT CTexturedFontsManager::OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr = S_OK;
	pDevice = pd3dDevice;
	m_texManager.OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc);
	for(int kk=0; kk<fonts.GetSize(); kk++)
	{
		V_RETURN(fonts[kk]->OnResetDevice(pd3dDevice, pBackBufferSurfaceDesc));
	}
	return hr;
}

HRESULT CTexturedFontsManager::OnLostDevice(void)
{
	HRESULT hr = S_OK;
	m_texManager.OnLostDevice();
	for(int kk=0; kk<fonts.GetSize(); kk++)
	{
		V_RETURN(fonts[kk]->OnLostDevice());
	}
	return hr;
}

HRESULT CTexturedFontsManager::OnDestroyDevice(void)
{
	HRESULT hr = S_OK;
	m_texManager.OnDestroyDevice();
	for(int kk=0; kk<fonts.GetSize(); kk++)
	{
		V_RETURN(fonts[kk]->OnDestroyDevice());
	}

	return hr;
}

void CTexturedFontsManager::Release()
{
	for (int kk = 0; kk < fonts.GetSize(); kk++)
	{
		fonts[kk]->Release();
		SAFE_DELETE(fonts[kk]);
	}
	fonts.RemoveAll();

	m_texManager.Release();
}

CTexturedFontsManager::CTexturedFontsManager()
{
	globalStrManager = NULL;
}

CTexturedFontsManager::~CTexturedFontsManager()
{
	Release();
}


void CTexturedFontsManager::SetPauseOnTTFontsReplacement(bool bPaused)
{
	for (int kk = 0; kk < fonts.GetSize(); kk++)
	{
		fonts[kk]->bFontReplacementOn = !bPaused;
		//can't activate TTF if no TTF available
		if ((fonts[kk]->bFontReplacementOn == true) && (fonts[kk]->pFontReplacementTTF == null))
			fonts[kk]->bFontReplacementOn = false;
	}
}

void CTexturedFontsManager::SetManagersPtr(CStringsManager *pStrManager)
{
	globalStrManager = pStrManager;
	CTexturedFont::SetManagersPtr(globalStrManager, this);
}

HRESULT CTexturedFontsManager::AddFontXML(WCHAR *XMLpath, int *retIdx)
{
	HRESULT hr = S_OK;
	if(globalStrManager == NULL)
	{
		ErrorBox(K_ERR_WARNING, L"CTexturedFontsManager::AddFontXML -> global strings manager pointer not set!\nCall SetManagersPtr(strMgr) first!");
		return E_FAIL;
	}

	CTexturedFont *nf = new CTexturedFont();
	CTexturedFont::pDevice = pDevice;
	if(FAILED(hr = nf->LoadFontXML(XMLpath)))
	{
		SAFE_DELETE(nf);
		if(retIdx != NULL)
			*retIdx = -1;
		return hr;
	}

	nf->SetManagersPtr(globalStrManager, this);

	fonts.Add(nf);
	if(retIdx != NULL)
		*retIdx = fonts.GetSize() - 1;

	return hr;
}


///**************************************************************************************
/// Sigleton de acces
///**************************************************************************************

CTexturedFontsManager& UTGetFontsManager()
{
	static CTexturedFontsManager g_FontsManager;
	return g_FontsManager;
}
