#include "dxstdafx.h"
#include "TexFont.h"

CTexFont::CTexFont()
{
	loaded = false;

	pTexNode = nullptr;
	strLoadedTexture[ 0 ] = 0;
	strLoadedFile[ 0 ] = 0;

	moduleNo = 0;
	moduleUV = nullptr;
	moduleRect = nullptr;
	frameBBox = nullptr;

	ID = 0;
	letterSpacing = FONT_MIN_LETTER_SPACING;
	rowSpacing = FONT_MIN_ROW_SPACING;
	spaceSize = FONT_MIN_SPACE_SIZE;
	rowHeight = FONT_MIN_ROW_HEIGHT;
}

CTexFont::~CTexFont()
{
	Release();
}

OPRESULT CTexFont::LoadFontXML( WCHAR* XMLpath )
{
	if ( loaded )
	{
		Release();
	}

	StringCchCopy( strLoadedFile, MAX_PATH, XMLpath );

	pugi::xml_document doc;
	if ( !doc.load_file( XMLpath ) )
	{
		return OPRESULT( K_OP_FAILED, K_SEVERITY_WARNING, L"Unable to load Font XML:%s\n", XMLpath );
	}

	pugi::xml_attribute ver = doc.root().child( L"SpriteCollection" ).attribute( L"Version" );
	if ( ver.as_float() != BSX_VERSION )
	{
		return OPRESULT( K_OP_FAILED, K_SEVERITY_WARNING, L"SpriteCollection XML wrong version:%s\n", XMLpath );
	}

	pugi::xml_node spritenodes = doc.root().child( L"SpriteCollection" );

	///--- citeste date despre font, daca sunt gasite ---
	pugi::xml_node datanode = spritenodes.child( L"FontData" );
	ID = 0;
	letterSpacing = FONT_MIN_LETTER_SPACING;
	rowSpacing = FONT_MIN_ROW_SPACING;
	spaceSize = FONT_MIN_SPACE_SIZE;
	rowHeight = FONT_MIN_ROW_HEIGHT;
	if ( !datanode.attribute( L"ID" ).empty() )
	{
		WCHAR strID[ MAX_PATH ];
		StringCchCopy( strID, MAX_PATH, datanode.attribute( L"ID" ).value() );
		ID = FastHash( strID, wcslen( strID ) );
		shFontName.Init( strID );

		// already loaded ?
		if ( __TexFonts().GetFontIdx( strID ) >= 0 )
		{
			return OPRESULT( K_OP_OK, K_SEVERITY_WARNING, L"CTexFont::LoadFontXML -> Font already loaded! %s", strID );
		}
	}
	else
	{
		ErrorBox( K_ERR_WARNING, L"CTexFont::LoadFontXML -> Nameless font!\n%s", XMLpath );
	}
	if ( !datanode.attribute( L"LetterSpacing" ).empty() )
		letterSpacing = datanode.attribute( L"LetterSpacing" ).as_int();
	if ( !datanode.attribute( L"RowSpacing" ).empty() )
		rowSpacing = datanode.attribute( L"RowSpacing" ).as_int();
	if ( !datanode.attribute( L"RowHeight" ).empty() )
		rowHeight = datanode.attribute( L"RowHeight" ).as_int();
	if ( !datanode.attribute( L"SpaceSize" ).empty() )
		spaceSize = datanode.attribute( L"SpaceSize" ).as_int();

	///--- citeste modulele ---
	pugi::xml_node modulesnode = spritenodes.child( L"Modules" );
	CArray<CTFModule*> tempModules;
	for ( pugi::xml_node moduledata = modulesnode.first_child(); moduledata; moduledata = moduledata.next_sibling() )
	{
		CTFModule *nmod = new CTFModule();
		nmod->X = moduledata.attribute( L"X" ).as_int();
		nmod->Y = moduledata.attribute( L"Y" ).as_int();
		nmod->W = moduledata.attribute( L"W" ).as_int();
		nmod->H = moduledata.attribute( L"H" ).as_int();
		nmod->imgIdx = moduledata.attribute( L"ImageIdx" ).as_int();
		tempModules.Add( nmod );
	}

	///--- citeste frame modules ---
	CArray<CTFFModule*> FModules;

	pugi::xml_node framesnode = spritenodes.child( L"Frames" );
	pugi::xml_node fmodulesnode = spritenodes.child( L"FrameModules" );
	CArray<RectXYWHi*> tempFrameBBox;
	for ( pugi::xml_node fmoduledata = fmodulesnode.first_child(), framedata = framesnode.first_child();
		fmoduledata;
		fmoduledata = fmoduledata.next_sibling(), framedata = framedata.next_sibling() )
	{
		CTFFModule *nfmod = new CTFFModule();
		int midx = fmoduledata.attribute( L"ModuleIdx" ).as_int();
		nfmod->ox = fmoduledata.attribute( L"OX" ).as_int();
		nfmod->oy = fmoduledata.attribute( L"OY" ).as_int();
		nfmod->flags = fmoduledata.attribute( L"Flags" ).as_uint();

		nfmod->moduleX = tempModules[ midx ]->X; nfmod->moduleY = tempModules[ midx ]->Y;
		nfmod->moduleW = tempModules[ midx ]->W; nfmod->moduleH = tempModules[ midx ]->H;
		nfmod->imgIdx = tempModules[ midx ]->imgIdx;
		//seteaza si RECT-ul
		SetRect( &nfmod->moduleRect, tempModules[ midx ]->X, tempModules[ midx ]->Y, tempModules[ midx ]->X + tempModules[ midx ]->W, tempModules[ midx ]->Y + tempModules[ midx ]->H );
		FModules.Add( nfmod );

		RectXYWHi *frameR = new RectXYWHi();
		frameR->x = framedata.attribute( L"BBoxX" ).as_int();
		frameR->y = framedata.attribute( L"BBoxY" ).as_int();
		frameR->w = framedata.attribute( L"BBoxW" ).as_int();
		frameR->h = framedata.attribute( L"BBoxH" ).as_int();
		tempFrameBBox.Add( frameR );
		//daca nu am setat BBox din editor ia marimea din dimensiune modul
		if ( frameR->w == 0 )
			frameR->w = nfmod->moduleW;
		if ( frameR->h == 0 )
			frameR->h = nfmod->moduleH;
	}
	//salveaza nr de litere
	moduleNo = FModules.GetSize();
	//dezaloca tempmodules
	for ( int kk = 0; kk < tempModules.GetSize(); kk++ )
	{
		SAFE_DELETE( tempModules[ kk ] );
	}
	tempModules.RemoveAll();
	/// Loads font texture
	WCHAR szwPath[ MAX_PATH ];
	StringCchCopy( szwPath, MAX_PATH, XMLpath );
	int nIdx = ( int ) wcslen( szwPath );
	while ( --nIdx > 0 && szwPath[ nIdx ] != '\\' && szwPath[ nIdx ] != '/' );
	szwPath[ nIdx + 1 ] = '\0';

	// takes the first image node
	pugi::xml_node imagenode = spritenodes.child( L"Image" );

	const WCHAR* imgname = imagenode.child_value();
	StringCchPrintf( strLoadedTexture, MAX_PATH, L"%s%s", szwPath, imgname );

	pTexNode = __TexFonts().m_texManager.AddTexture( strLoadedTexture, D3DFMT_A8R8G8B8, D3DX_FILTER_NONE, D3DX_FILTER_NONE );
	if ( pTexNode == nullptr )
	{
		Release();
		return OPRESULT( K_OP_FAILED, K_SEVERITY_WARNING, L"CTexFont::LoadFontXML -> Could not load texture!\n %s", strLoadedTexture );
	}
	// save texture size
	Vec2 texsz = pTexNode->getSize();

	///--- acum aloca tot ce ii trebuie ca sa se miste rapid la desenare ---
	moduleRect = new RectLTRB[ moduleNo ];
	moduleUV = new RectLTRB[ moduleNo ];
	frameBBox = new RectXYWHi[ moduleNo ];
	// copy and compute module data
	for ( int kk = 0; kk < moduleNo; kk++ )
	{
		moduleRect[ kk ].Set( FModules[ kk ]->ox, FModules[ kk ]->oy, FModules[ kk ]->ox + FModules[ kk ]->moduleW, FModules[ kk ]->oy + FModules[ kk ]->moduleH );
		moduleUV[ kk ].Set( FModules[ kk ]->moduleX / texsz.x, FModules[ kk ]->moduleY / texsz.y,
			( FModules[ kk ]->moduleX + FModules[ kk ]->moduleW ) / texsz.x, ( FModules[ kk ]->moduleY + FModules[ kk ]->moduleH ) / texsz.y );
		frameBBox[ kk ] = *tempFrameBBox[ kk ];
	}

	SAFE_DELETE_GROWABLE_ARRAY( FModules );
	SAFE_DELETE_GROWABLE_ARRAY( tempFrameBBox );

	loaded = true;
	LOG( L"Fonts:: Loaded:[%s] from [%s]", shFontName.text, XMLpath );

	return K_OP_OK;
}


//face release la tot
void CTexFont::Release()
{
	if ( loaded )
	{
		LOG( L"Fonts:: Released:[%s]", shFontName.text );
	}

	SAFE_DELETE_ARRAY( moduleUV );
	SAFE_DELETE_ARRAY( moduleRect );
	SAFE_DELETE_ARRAY( frameBBox );

	strLoadedFile[ 0 ] = 0;
	strLoadedTexture[ 0 ] = 0;

	loaded = false;
}

int CTexFont::GetRowHeight( bool bIncludeSpacing )
{
	int rowh = rowHeight;
	if ( bIncludeSpacing )
		rowh += rowSpacing;

	return rowh;
}

int CTexFont::DrawStringClamped( CStringDesc *strDesc, int X, int Y, int maxW, UINT16 Flags /*= FONTFLAG_ANCHOR_BOTTOMLEFT*/, DWORD Color /*= 0xffffffff*/ )
{
	PTEXTURE pTexture = pTexNode->pTexture;

	UINT16 length = strDesc->len;
	UINT16* text = strDesc->codes;

	if ( length == 0 )
		return 0;

	int alignOffset = 0;
	int width = 0;
	int maxHeight = rowHeight;

	//measure 3 dots length
	int dotscode = __Texts().GetLetterIdx( '.' );
	int dotsw = 3 * ( frameBBox[ dotscode ].w + letterSpacing );

	int localLen = 0;
	// compute text len
	SizeWHi sz = MeasureString( strDesc );
	if ( sz.w > maxW )
	{
		for ( int ii = 0; ii < length; ii++ )
		{
			int cod = text[ ii ];
			if ( cod == K_STRMGR_RETURN )
			{
				continue;
			}
			if ( cod == K_STRMGR_SPACE )
			{
				width += spaceSize;
				continue;
			}
			//height
			//if (maxHeight < -fmodule_oy[cod])
			//	maxHeight = -fmodule_oy[cod];

			if ( width + frameBBox[ cod ].w + letterSpacing >= maxW - dotsw )
			{
				break;
			}

			width += ( frameBBox[ cod ].w + letterSpacing );
			//save last printed char
			localLen = ii;
		}
		// keep track of width
		width += dotsw;
	}
	else
	{
		width = sz.w;
		//maxHeight = sz.h;
		localLen = length - 1;
	}

	if ( ( Flags & FONTFLAG_ANCHOR_RIGHT ) != 0 )
		alignOffset = width;
	else if ( ( Flags & FONTFLAG_ANCHOR_CENTER ) != 0 )
		alignOffset = width / 2;

	int posx = X - alignOffset;

	int height = maxHeight;
	if ( ( Flags & FONTFLAG_ANCHOR_BOTTOM ) != 0 )
		height = 0;
	else if ( ( Flags & FONTFLAG_ANCHOR_VCENTER ) != 0 )
		height = height / 2;
	else if ( ( Flags & FONTFLAG_ANCHOR_TOP ) != 0 )
		height = height;

	int posy = Y + height;

	int startPosX = posx; //backup start pos

	for ( int ii = 0; ii <= localLen; ii++ )
	{
		int cod = text[ ii ];
		if ( cod == K_STRMGR_RETURN )
		{
			continue;
		}
		if ( cod == K_STRMGR_SPACE )
		{
			posx += spaceSize;
			continue;
		}

		// paint letter
		__Painter().Draw( pTexture, moduleUV[ cod ], moduleRect[ cod ], Vec2( posx, posy ), Color );
		posx += frameBBox[ cod ].w + letterSpacing;
	}
	// paint 3 dots if necessary
	if ( localLen < length - 1 )
	{
		for ( int ii = 0; ii < 3; ii++ )
		{
			__Painter().Draw( pTexture, moduleUV[ dotscode ], moduleRect[ dotscode ], Vec2( posx, posy ), Color );
			posx += frameBBox[ dotscode ].w + letterSpacing;
		}
	}
	return posx - X + alignOffset;
}

int CTexFont::DrawStringClamped( int strIdx, int X, int Y, int maxW, UINT16 Flags /*= FONTFLAG_ANCHOR_BOTTOMLEFT*/, DWORD Color /*= 0xffffffff*/ )
{
	CStringDesc *strDesc = __Texts().strings[ strIdx ];
	return DrawStringClamped( strDesc, X, Y, maxW, Flags, Color );
}

int CTexFont::DrawStringScaleW( int strIdx, int X, int Y, int maxW, UINT16 Flags, DWORD Color )
{
	//CStringDesc *strDesc = m_pStrManager->strings[strIdx];
	//return DrawStringScaleW(strDesc, X, Y, maxW, Flags, Color);
	return 0;
}

int CTexFont::DrawStringScaleW( CStringDesc *strDesc, int X, int Y, int maxW, UINT16 Flags, DWORD Color )
{
	/*
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
		*/
	return 0;
}

int CTexFont::DrawStringScaleW( int strIdx, RectXYWHi rect, UINT16 Flags, DWORD Color )
{
	CStringDesc *strDesc = __Texts().strings[ strIdx ];
	return DrawStringScaleW( strDesc, rect, Flags, Color );
}

int CTexFont::DrawStringScaleW( CStringDesc *strDesc, RectXYWHi rect, UINT16 Flags, DWORD Color )
{
	SizeWHi strW = MeasureString( strDesc );
	if ( strW.w > rect.w )
	{
		Vec2 vCenter( rect.CenterX(), rect.CenterY() );
		Flags &= ~FONTFLAG_ANCHOR_LEFT;
		Flags &= ~FONTFLAG_ANCHOR_RIGHT;
		Flags |= FONTFLAG_ANCHOR_CENTER;
		return DrawStringScaleW( strDesc, vCenter.x, vCenter.y, rect.w, Flags, Color );
	}
	else
	{
		DrawString( strDesc, rect, Flags, Color );
		return 0;
	}
}


void CTexFont::DrawString( CStringDesc *strDesc, RectXYWHi rect, UINT16 Flags, DWORD Color )
{
	PTEXTURE pTexture = pTexNode->pTexture;

	int textLen = strDesc->len;
	UINT16* textCodes = strDesc->codes;

	if ( textLen == 0 )
		return;

	if ( ( Flags & FONTFLAG_WRAPTEXT ) != 0 ) //WRAP text
	{
		int verticalOffset = 0;
		//daca am aliniere pe verticala masor stringul pe verticala
		if ( ( Flags & FONTFLAG_ANCHOR_VCENTER ) || ( Flags & FONTFLAG_ANCHOR_BOTTOM ) )
		{
			SizeWHi textsize = MeasureString( strDesc, rect.w );
			if ( Flags & FONTFLAG_ANCHOR_BOTTOM )
				verticalOffset = rect.h - textsize.h;
			else
				verticalOffset = ( rect.h - textsize.h ) / 2;
		}


		int posx = rect.x, posy = rect.y + rowHeight + verticalOffset;

		int countedSpaces = 0;

		int paintStart = 0;
		int lastSpace = 0;
		int lastSpaceWidth = 0;
		int tmpWidth = 0;
		int textCur = 0;

		while ( textCur < textLen )
		{
			UINT16 code = textCodes[ textCur ];

			if ( code == K_STRMGR_SPACE )
			{
				lastSpace = textCur;
				lastSpaceWidth = tmpWidth;

				tmpWidth += spaceSize;
				countedSpaces++;
			}
			else if ( code == K_STRMGR_RETURN )
			{
				lastSpaceWidth = tmpWidth;
				lastSpace = textCur;

				tmpWidth += rect.w + 1;
				countedSpaces = 0;
			}
			else
				tmpWidth += frameBBox[ code ].w + letterSpacing;
			//daca a ajuns la final forteaza desenare  ca sa goleasca ce a mai ramas de desenat
			if ( textCur >= textLen - 1 )
			{
				lastSpaceWidth = tmpWidth;
				tmpWidth += rect.w + 1;
				lastSpace = textLen;
				countedSpaces = 0;
			}

			if ( tmpWidth > rect.w ) //deseneaza de la paintStart pana la lastSpace
			{
				float spaceAdder = 0.0f;
				float spaceStep = 0.0f;
				countedSpaces--;
				if ( Flags & FONTFLAG_JUSTIFY )
				{
					if ( countedSpaces > 0 )
					{
						spaceStep = ( rect.w - lastSpaceWidth ) / countedSpaces;
						if ( spaceStep > 3.0f * spaceSize )
							spaceStep = 3.0f * spaceSize;
					}
					else //daca nu face justify, face aliniere
					{
						//aliniere centru
						if ( Flags & FONTFLAG_ANCHOR_CENTER )
							posx += ( ( rect.w - lastSpaceWidth ) >> 1 );
						else if ( Flags & FONTFLAG_ANCHOR_RIGHT )
							posx += ( rect.w - lastSpaceWidth );
					}
				}
				else//daca nu e pe justify
				{
					if ( Flags & FONTFLAG_ANCHOR_CENTER )
						posx += ( ( rect.w - lastSpaceWidth ) >> 1 );
					else if ( Flags & FONTFLAG_ANCHOR_RIGHT )
						posx += ( rect.w - lastSpaceWidth );
				}

				if ( ( Flags & FONTFLAG_CLIPTEXT ) == 0 )
				{
					//fara clip la rectangle
					for ( int kk = paintStart; kk < lastSpace; kk++ )
					{
						int cod2 = textCodes[ kk ];
						if ( ( cod2 == K_STRMGR_SPACE ) || ( cod2 == K_STRMGR_RETURN ) )
						{
							spaceAdder += spaceStep;
							posx += spaceSize + ( int ) floor( spaceAdder );
							spaceAdder -= floor( spaceAdder );
							continue;
						}
						//deseneaza litera
						__Painter().Draw( pTexture, moduleUV[ cod2 ], moduleRect[ cod2 ], Vec2( posx, posy ), Color );
						posx += frameBBox[ cod2 ].w + letterSpacing;
					}
				}
				else
				{
					//with clip
					for ( int kk = paintStart; kk < lastSpace; kk++ )
					{
						int cod2 = textCodes[ kk ];
						if ( ( cod2 == K_STRMGR_SPACE ) || ( cod2 == K_STRMGR_RETURN ) )
						{
							spaceAdder += spaceStep;
							posx += spaceSize + ( int ) floor( spaceAdder );
							spaceAdder -= floor( spaceAdder );
							continue;
						}
						//#TODO: needs a generic clip function in __Painter().Draw
						/*
						RECT letterRect = moduleRect[ cod2 ];
						if ( ( posy + fmodule_oy[ cod2 ] > rect.y + rect.h ) || ( posy + fmodule_oy[ cod2 ] + moduleH[ cod2 ] < rect.y ) )
						{
							posx += frameBBox[ cod2 ].w + letterSpacing;
							continue;
						}
						if ( posy + moduleH[ cod2 ] + fmodule_oy[ cod2 ] > rect.y + rect.h )
						{
							letterRect.bottom -= ( posy + moduleH[ cod2 ] + fmodule_oy[ cod2 ] - rect.y - rect.h );
						}
						s_pSprite->Draw( pTexture, &letterRect, NULL, &D3DXVECTOR3( posx + fmodule_ox[ cod2 ], posy + fmodule_oy[ cod2 ], 0.0f ), Color );
						*/
						posx += frameBBox[ cod2 ].w + letterSpacing;
					}
				}
				//reseteaza
				paintStart = lastSpace + 1;

				// daca spatiul orizontal e mai mic decat un cuvant, iese
				if ( lastSpace == 0 )
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
		if ( Flags & FONTFLAG_ANCHOR_BOTTOM )
			posy = rect.y + rect.h;
		else if ( Flags & FONTFLAG_ANCHOR_VCENTER )
			posy = rect.y + ( ( rect.h + rowHeight ) >> 1 );

		int width = 0;
		//daca e centrat calculeaza lungimea textului
		if ( ( Flags & ( FONTFLAG_ANCHOR_RIGHT | FONTFLAG_ANCHOR_CENTER ) ) != 0 )
		{
			for ( int ii = 0; ii < textLen; ii++ )
			{
				int cod = textCodes[ ii ];
				if ( ( cod == K_STRMGR_SPACE ) || ( cod == K_STRMGR_RETURN ) )
				{
					width += spaceSize;
					continue;
				}
				width += ( frameBBox[ cod ].w + letterSpacing );
			}
		}

		if ( ( Flags & FONTFLAG_ANCHOR_RIGHT ) != 0 )
			posx += rect.w - width;
		else if ( ( Flags & FONTFLAG_ANCHOR_CENTER ) != 0 )
			posx += ( rect.w - width ) >> 1;

		for ( int ii = 0; ii < textLen; ii++ )
		{
			int cod = textCodes[ ii ];
			if ( ( cod == K_STRMGR_SPACE ) || ( cod == K_STRMGR_RETURN ) )
			{
				posx += spaceSize;
				continue;
			}

			//deseneaza litera
			if ( ( Flags & FONTFLAG_CLIPTEXT ) == 0 )
			{
				__Painter().Draw( pTexture, moduleUV[ cod ], moduleRect[ cod ], Vec2( posx, posy ), Color );
				posx += frameBBox[ cod ].w + letterSpacing;
			}
			else
			{
				int offx = 0;
				/*
				RECT letterRect = moduleRect[ cod ];
				if ( ( posx > rect.x + rect.w ) || ( posx + frameBBox[ cod ].w < rect.x ) )
				{
					posx += frameBBox[ cod ].w + letterSpacing;
					continue;
				}
				if ( posx < rect.x )
				{
					letterRect.left += ( rect.x - posx );
					offx = rect.x - posx;
				}
				else if ( posx + frameBBox[ cod ].w > rect.x + rect.w )
				{
					letterRect.right -= ( posx + frameBBox[ cod ].w - rect.x - rect.w );
				}
				s_pSprite->Draw( pTexture, &letterRect, NULL, &D3DXVECTOR3( posx + fmodule_ox[ cod ], posy + fmodule_oy[ cod ], 0.0f ), Color );
				*/
				posx += frameBBox[ cod ].w + letterSpacing;
			}
		}
	}
}


int CTexFont::DrawString( CStringDesc *strDesc, float X, float Y, UINT16 Flags, DWORD Color )
{
	_ASSERT( strDesc != nullptr );

	PTEXTURE pTexture = pTexNode->pTexture;

	UINT16 length = strDesc->len;
	UINT16* text = strDesc->codes;

	if ( length == 0 )
		return 0;

	int alignOffset = 0;
	int width = 0;
	int maxHeight = 0;
	//daca e centrat calculeaza lungimea textului
	if ( ( Flags & ( FONTFLAG_ANCHOR_RIGHT | FONTFLAG_ANCHOR_CENTER | FONTFLAG_ANCHOR_BOTTOM | FONTFLAG_ANCHOR_VCENTER ) ) != 0 )
	{
		for ( int ii = 0; ii < length; ii++ )
		{
			int cod = text[ ii ];
			if ( cod == K_STRMGR_RETURN )
			{
				continue;
			}
			if ( cod == K_STRMGR_SPACE )
			{
				width += spaceSize;
				continue;
			}
			width += ( frameBBox[ cod ].w + letterSpacing );
			//calculeaza inaltimea
			maxHeight = rowHeight;
		}
	}
	else
	{
		maxHeight = rowHeight;
	}

	if ( ( Flags & FONTFLAG_ANCHOR_RIGHT ) != 0 )
		alignOffset = width;
	else if ( ( Flags & FONTFLAG_ANCHOR_CENTER ) != 0 )
		alignOffset = width / 2;

	float posx = X - alignOffset;

	int height = maxHeight;
	if ( ( Flags & FONTFLAG_ANCHOR_BOTTOM ) != 0 )
		height = 0;
	else if ( ( Flags & FONTFLAG_ANCHOR_VCENTER ) != 0 )
		height = height / 2;
	else if ( ( Flags & FONTFLAG_ANCHOR_TOP ) != 0 )
		height = height;

	int posy = Y + height;

	for ( int ii = 0; ii < length; ii++ )
	{
		int cod = text[ ii ];
		if ( cod == K_STRMGR_RETURN )
		{
			continue;
		}
		if ( cod == K_STRMGR_SPACE )
		{
			posx += spaceSize;
			continue;
		}

		//deseneaza litera
		__Painter().Draw( pTexture, moduleUV[ cod ], moduleRect[ cod ], Vec2( posx, posy ), Color );
		posx += frameBBox[ cod ].w + letterSpacing;
	}

	return posx - X + alignOffset;
}


int CTexFont::DrawString( int strIdx, float X, float Y, UINT16 Flags, DWORD Color )
{
	CStringDesc* strdesc = __Texts().GetStringDescByIdx( strIdx );
	return DrawString( strdesc, X, Y, Flags, Color );
}

void CTexFont::DrawString( int strIdx, RectXYWHi rect, UINT16 Flags, DWORD Color )
{
	CStringDesc* strdesc = __Texts().GetStringDescByIdx( strIdx );
	DrawString( strdesc, rect, Flags, Color );
}

/*
void CTexFont::DrawStringOffsetY(int strIdx, RECTXYWH rect, int offsetY, UINT16 Flags, DWORD Color)
{
#if defined(_DEBUG) || defined(DEBUG)
	if(m_pStrManager == NULL)
	{
		ErrorBox(K_ERR_WARNING, L"CTexFont::DrawString -> m_pStrManager is NULL! Returning.");
		return;
	}

	if((strIdx < 0) || (strIdx > m_pStrManager->strings.GetSize()))
	{
		ErrorBox(K_ERR_WARNING, L"CTexFont::DrawString -> Index out of range! idx=%d", strIdx);
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
*/

SizeWHi CTexFont::MeasureString( CStringDesc *strDesc )
{
	SizeWHi retsz( 0, rowHeight );
	for ( UINT ii = 0; ii < strDesc->len; ii++ )
	{
		int cod = strDesc->codes[ ii ];
		if ( cod == K_STRMGR_SPACE )
		{
			retsz.w += spaceSize;
			continue;
		}
		else if ( cod == K_STRMGR_RETURN )
		{
			retsz.h += rowHeight + rowSpacing;
			continue;
		}
		retsz.w += ( frameBBox[ cod ].w + letterSpacing );
	}

	return retsz;
}

SizeWHi CTexFont::MeasureString( int strIdx )
{
	SizeWHi retsz( 0, rowHeight );
	CStringDesc *strDesc = __Texts().GetStringDescByIdx(strIdx);
	return MeasureString( strDesc );
}

SizeWHi CTexFont::MeasureString( int strIdx, int nMaxWidth )
{
	int maxWidth = nMaxWidth;
	SizeWHi retsz( maxWidth, rowHeight );
	CStringDesc *strDesc = __Texts().GetStringDescByIdx( strIdx );
	return MeasureString( strDesc, nMaxWidth );
}

SizeWHi CTexFont::MeasureString( CStringDesc* strDesc, int nMaxWidth )
{
	int maxWidth = nMaxWidth;
	SizeWHi retsz( maxWidth, rowHeight );

	int textLen = strDesc->len;
	UINT16* textCodes = strDesc->codes;

	if ( textLen == 0 )
		return retsz;

	int posy = 0;//rowHeight;

	int paintStart = 0;
	int lastSpace = 0;
	int tmpWidth = 0;
	int textCur = 0;

	while ( textCur < textLen )
	{
		UINT16 code = textCodes[ textCur ];

		if ( code == K_STRMGR_SPACE )
		{
			lastSpace = textCur;
			tmpWidth += spaceSize;
		}
		else if ( code == K_STRMGR_RETURN )
		{
			lastSpace = textCur;
			tmpWidth = maxWidth + 1;
		}
		else
			tmpWidth += frameBBox[ code ].w + letterSpacing;
		//daca a ajuns la final forteaza ca sa goleasca ce a mai ramas
		if ( textCur >= textLen - 1 )
		{
			if ( posy == 0 ) //daca e pe un singur rand setez deja latimea
				retsz.w = tmpWidth;

			tmpWidth = maxWidth + 1;
			lastSpace = textLen;
		}

		if ( tmpWidth > maxWidth ) //calucleaza de la paintStart pana la lastSpace
		{
			// make sure we don't split if we had no space yet
			if ( lastSpace != 0 )
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

int CTexFont::DrawHString( UINT32 strHash, int X, int Y, UINT16 Flags, DWORD Color )
{
	int strIdx = __Texts().GetStrIdx( strHash );
	return DrawString( strHash, X, Y, Flags, Color );
}

void CTexFont::DrawHString( UINT32 strHash, RectXYWHi rect, UINT16 Flags, DWORD Color )
{
	int strIdx = __Texts().GetStrIdx( strHash );
	DrawString( strIdx, rect, Flags, Color );
}
/*
void CTexFont::DrawHStringOffsetY(UINT32 strHash, RECTXYWH rect, int offsetY, UINT16 Flags, DWORD Color)
{
	int strIdx = m_pStrManager->GetStrIdx(strHash);
	DrawStringOffsetY(strIdx, rect, offsetY, Flags, Color);
}
*/
SizeWHi CTexFont::MeasureHString( UINT32 strHash )
{
	int strIdx = __Texts().GetStrIdx( strHash );

	SizeWHi retsz( 0, rowHeight );

	CStringDesc *strDesc = __Texts().GetStringDescByIdx( strIdx );
	for ( UINT ii = 0; ii < strDesc->len; ii++ )
	{
		int cod = strDesc->codes[ ii ];
		if ( cod == K_STRMGR_RETURN )
			continue;
		if ( cod == K_STRMGR_SPACE )
		{
			retsz.w += spaceSize;
			continue;
		}
		retsz.w += ( frameBBox[ cod ].w + letterSpacing );
	}

	return retsz;
}

SizeWHi CTexFont::MeasureHString( UINT32 strHash, int maxWidth )
{
	int strIdx = __Texts().GetStrIdx( strHash );
	SizeWHi retsz( maxWidth, 0 );

	CStringDesc *strDesc = __Texts().GetStringDescByIdx( strIdx );
	int textLen = strDesc->len;
	UINT16* textCodes = strDesc->codes;

	if ( textLen == 0 )
		return retsz;

	int posy = 0;//rowHeight;

	int paintStart = 0;
	int lastSpace = 0;
	int tmpWidth = 0;
	int textCur = 0;

	while ( textCur < textLen )
	{
		UINT16 code = textCodes[ textCur ];

		if ( code == K_STRMGR_SPACE )
		{
			lastSpace = textCur;
			tmpWidth += spaceSize;
		}
		else if ( code == K_STRMGR_RETURN )
		{
			lastSpace = textCur;
			tmpWidth += maxWidth + 1;
		}
		else
			tmpWidth += frameBBox[ code ].w + letterSpacing;
		//daca a ajuns la final forteaza ca sa goleasca ce a mai ramas
		if ( textCur >= textLen - 1 )
		{
			tmpWidth += maxWidth + 1;
			lastSpace = textLen;
		}

		if ( tmpWidth > maxWidth ) //calucleaza de la paintStart pana la lastSpace
		{
			// make sure we don't split if we had no space yet
			if ( lastSpace != 0 )
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


//********************************************************************************
// Fonts Manager
//********************************************************************************
int CTexFontsManager::GetFontIdx( const CHAR* fontID )
{
	UINT32 fhash = FastHash( fontID, strlen( fontID ) );
	for ( int kk = 0; kk < fonts.GetSize(); kk++ )
	{
		if ( fonts[ kk ]->ID == fhash )
			return kk;
	}

	//ErrorBox(K_ERR_WARNING, L"GetFontIdx could not find fontID %s\n", fontID);
	return -1;
}

int CTexFontsManager::GetFontIdx( const WCHAR* fontID )
{
	UINT32 fhash = FastHash( fontID, wcslen( fontID ) );
	for ( int kk = 0; kk < fonts.GetSize(); kk++ )
	{
		if ( fonts[ kk ]->ID == fhash )
			return kk;
	}

	//ErrorBox(K_ERR_WARNING, L"GetFontIdx could not find fontID %s\n", fontID);
	return -1;
}

CTexFont* CTexFontsManager::operator[] ( const CHAR* fontID )
{
	int idx = GetFontIdx( fontID );
	assert( ( idx >= 0 ) && ( idx < fonts.GetSize() ) );
	return fonts[ idx ];
}

CTexFont* CTexFontsManager::operator[] ( const int fontIdx )
{
	assert( ( fontIdx >= 0 ) && ( fontIdx < fonts.GetSize() ) );
	return fonts[ fontIdx ];
}


///--- system framework ---
OPRESULT CTexFontsManager::OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc )
{
	m_pDevice = pDevice;
	m_texManager.OnCreateDevice( pDevice, pBBDesc);
	return K_OP_OK;
}

OPRESULT CTexFontsManager::OnResetDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc )
{
	m_pDevice = pDevice;
	m_texManager.OnResetDevice( pDevice, pBBDesc);
	return K_OP_OK;
}

OPRESULT CTexFontsManager::OnLostDevice()
{
	m_texManager.OnLostDevice();
	return K_OP_OK;
}

OPRESULT CTexFontsManager::OnDestroyDevice()
{
	m_texManager.OnDestroyDevice();
	return K_OP_OK;
}

void CTexFontsManager::Release()
{
	for ( int kk = 0; kk < fonts.GetSize(); kk++ )
	{
		fonts[ kk ]->Release();
		SAFE_DELETE( fonts[ kk ] );
	}
	fonts.RemoveAll();

	m_texManager.Release();
}

CTexFontsManager::CTexFontsManager()
{
	m_pDevice = nullptr;
}

CTexFontsManager::~CTexFontsManager()
{
	Release();
}


OPRESULT CTexFontsManager::AddFontXML( WCHAR *XMLpath, int *retIdx )
{
	CTexFont *nf = new CTexFont();
	if ( OP_FAILED( nf->LoadFontXML( XMLpath ) ) )
	{
		SAFE_DELETE( nf );
		if ( retIdx != NULL )
			*retIdx = -1;
		return K_OP_FAILED;
	}

	fonts.Add( nf );
	if ( retIdx != NULL )
		*retIdx = fonts.GetSize() - 1;

	return K_OP_OK;
}

///----------------------------------------------------------------------------------
/// Textured Fonts singleton
///----------------------------------------------------------------------------------
CTexFontsManager& __TexFonts()
{
	static CTexFontsManager g_TexFontsManager;
	return g_TexFontsManager;
}
