#include "dxstdafx.h"
#include "FreeTypeFont.h"
// FreeType includes
#include "ft2build.h"
#include FT_FREETYPE_H
// PNG loader includes - used when texturing fonts
#include "lodepng/lodepng.h"
#include <iostream>

///--- STATICS ---
//CStringsManager*				CFreeTypeFont::m_pStrManager = NULL;

CFreeTypeFont::CFreeTypeFont()
{
	rowHeight = 0;
	letterSpacing = 0;
	rowSpacing = 1;
	spaceSize = 8;
	
	m_pSP = &UTPainter();
}

CFreeTypeFont::~CFreeTypeFont()
{
	Release();
}


OPRESULT CFreeTypeFont::CreateAtlas(PDEVICE pDevice, char* utf8Path, int nFontSize, WCHAR* wstrUniqueChars, sFreeTypeFontStyle *pStyle)
{
	//the raw pixels of the texturing png image	(R,G,B,A,...)
	unsigned png_width, png_height;
	unsigned char *png_bytes = nullptr;

	if (pStyle != nullptr)
	{
		if (!pStyle->strTexturePath.empty())
		{
			std::vector<unsigned char> png_image;
			unsigned error = lodepng::decode(png_image, png_width, png_height, pStyle->strTexturePath);
			if (error != 0)
			{
				LOG("CFreeTypeFont::CreateAtlas: Could not load overlay texture! %s\n%s", pStyle->strTexturePath.c_str(), lodepng_error_text(error));
			}
			else
			{
				// copy data to dinamically allocated array so it goes faster (yes it does!)
				png_bytes = new unsigned char[png_image.size()];
				memcpy(png_bytes, png_image.data(), png_image.size());
				png_image.clear();
				png_image.shrink_to_fit();
			}
		}
	}

	FT_Library ft;
	FT_Face    face;

	if (bLoaded)
		Release();

	int err = 0;
	err = FT_Init_FreeType(&ft);
	if(err != 0)
		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CFreeTypeFont::CreateAtlas: FT_Init_FreeType failed! code:%d", err);
	err = FT_New_Face(ft, utf8Path, 0, &face);
	if (err != 0)
		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CFreeTypeFont::CreateAtlas: FT_New_Face failed! code:%d font:%s", err, utf8Path);
	err = FT_Set_Char_Size(face, 0, nFontSize << 6, 96, 96);
	if (err != 0)
		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CFreeTypeFont::CreateAtlas: FT_Set_Char_size failed! code:%d", err);

	// quick and dirty max texture size estimate
	int nCharsCnt = wcslen(wstrUniqueChars);
	_ASSERT(nCharsCnt > 0);

	//#TODO: should parse all characters and compute the total texture area necessary for the letters and then find the closest texture size
	int max_dim = (1 + (face->size->metrics.height >> 6)) * ceilf(sqrtf(nCharsCnt));
	int tex_width = 1;
	while (tex_width < max_dim) tex_width <<= 1;
	int tex_height = tex_width;

	// create actual texture
	m_atlas.Release();
	HRESULT hr = UT3DCreateTexture(pDevice, tex_width, tex_height,
		1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &m_atlas.pTex);
	if (FAILED(hr))
	{
		FT_Done_Face(face);
		FT_Done_FreeType(ft);

		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CFreeTypeFont::CreateAtlas: Could not create atlas texture! hr=%x", hr);
	}

	m_atlas.atlasSize.w = tex_width;
	m_atlas.atlasSize.h = tex_height;

	// render glyphs to atlas in RGBA format (4 bytes per pixel)
	unsigned char* pixels = new unsigned char[tex_width * tex_height * 4];
	// clear pixels
	memset(pixels, 0, tex_width * tex_height * 4);

	m_atlas.nMaxBearingY = 0;
	int maxW = 0;

	int pen_x = 0, pen_y = 0;

	/*
	// alternative mode: convert string to utf8 (or get it as utf 8 and parse it char by char)

	char strutf8[2048];
	WCHARtoUTF8(strutf8, wstrUniqueChars, 2048);

	const char*    p = strutf8;
	const char*    end = p + strlen(strutf8); 
	for (;;)
	{
		int ch = utf8_next(&p, end);
		if (ch < 0)
			break;

		unsigned long codepoint = (unsigned long)ch;
	 }
	 */

	for (int i = 0; i < nCharsCnt; ++i) {
		FT_Int32 nLoadFlags = FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT;
		FT_ULong codepoint = (FT_ULong)wstrUniqueChars[i];
		
		FT_Load_Char(face, codepoint, nLoadFlags);

		FT_Bitmap* bmp = &face->glyph->bitmap;
		// compute max char height from all characters (ignoring letters that go under or over)
		if (m_atlas.nMaxBearingY < (face->glyph->metrics.horiBearingY >> 6))
			m_atlas.nMaxBearingY = (face->glyph->metrics.horiBearingY >> 6);
		// space size will be the widest character divided by 2
		if (maxW < bmp->width)
			maxW = bmp->width;

		if (pen_x + bmp->width >= tex_width) {
			pen_x = 0;
			pen_y += ((face->size->metrics.height >> 6) + 1);
		}

		int glyphBearing = (face->glyph->metrics.horiBearingY >> 6);
		for (int row = 0; row < bmp->rows; ++row) {
			for (int col = 0; col < bmp->width; ++col) {
				int x = pen_x + col;
				int y = pen_y + row;
				BYTE glyphcol = bmp->buffer[row * bmp->pitch + col];
				
				int pidx = y * tex_width + x;
				unsigned char pcol = glyphcol;
				unsigned char cola = pcol, colr = pcol, colg = pcol, colb = pcol;
				// do we have a loaded image?
				if (png_bytes != nullptr)
				{
					float fcol = ((float)pcol) / 255.0f;
					// texture center gets aligned to font baseline
					int png_idx = ((png_height + png_height / 2 + row - glyphBearing) % png_height) * png_width + (x % png_width);
					colr = (unsigned char)(fcol * png_bytes[png_idx * 4 + 0]);
					colg = (unsigned char)(fcol * png_bytes[png_idx * 4 + 1]);
					colb = (unsigned char)(fcol * png_bytes[png_idx * 4 + 2]);
					cola = (unsigned char)(fcol * png_bytes[png_idx * 4 + 3]);
				}
				// save final color into pixels in RGBA format
				pixels[pidx * 4 + 0] = colr;
				pixels[pidx * 4 + 1] = colg;
				pixels[pidx * 4 + 2] = colb;
				pixels[pidx * 4 + 3] = cola;
			}
		}

		// glyph data
		sGlyphInfo ginfo;

		/*
		// removed as unnecessary
		ginfo.x0 = pen_x;
		ginfo.y0 = pen_y;
		ginfo.x1 = pen_x + bmp->width;
		ginfo.y1 = pen_y + bmp->rows;

		ginfo.x_off = face->glyph->bitmap_left;
		ginfo.y_off = face->glyph->bitmap_top;
		*/
		ginfo.advanceX = face->glyph->advance.x >> 6;

		ginfo.texRect.Set((float)pen_x / (float)tex_width, (float)pen_y / (float)tex_height, 
			(float)(pen_x + bmp->width) / (float)tex_width, (float)(pen_y + bmp->rows) / (float)tex_height);

		int bx = face->glyph->metrics.horiBearingX >> 6;
		int by = face->glyph->metrics.horiBearingY >> 6;
		ginfo.moduleRectOff.Set(bx, -by,
			bx + bmp->width, -by + bmp->rows);

		m_atlas.arrGlyphs.Add(ginfo);

		pen_x += bmp->width + 1;
	}

	// save font data
	spaceSize = maxW / 2;
	// row height could be used from font metrics: face->size->metrics.height but this is usually bigger.
	rowHeight = m_atlas.nMaxBearingY;

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	// write to texture
	TEXTURE_LOCKRECT tex_locked_rect;
	if (m_atlas.pTex->LockRect(0, &tex_locked_rect, NULL, D3DLOCK_DISCARD) != D3D_OK)
		return OPRESULT(K_OP_FAILED, L"CFreeTypeFont::CreateAtlas: Could not lock atlas texture!", K_SEVERITY_WARNING);

	unsigned char* img = (unsigned char*)tex_locked_rect.pBits;
	//int imgpitchinbits = tex_locked_rect.Pitch;

	for (int yy = 0; yy < tex_height; yy++)
	{
		for (int xx = 0; xx < tex_width; xx++)
		{
			int idx = yy * tex_width + xx;
			// TARGET texture: BGRA, source pixels: RGBA
			img[idx * 4 + 0] = pixels[idx * 4 + 2];
			img[idx * 4 + 1] = pixels[idx * 4 + 1];
			img[idx * 4 + 2] = pixels[idx * 4 + 0];
			img[idx * 4 + 3] = pixels[idx * 4 + 3];
		}
	}

	m_atlas.pTex->UnlockRect(0);

	SAFE_DELETE_ARRAY(pixels);
	// clear texture image
	SAFE_DELETE_ARRAY(png_bytes);

	bLoaded = true;
	shFontName.Init(utf8Path);
	LOG("CFreeTypeFont::CreateAtlas [%dx%d] font:%s", tex_width, tex_height, utf8Path);

	return K_OP_OK;
}

void CFreeTypeFont::SetStyle(int nLetterSpacing, int nRowSpacing, int nSpaceSize)
{
	letterSpacing = nLetterSpacing;
	rowSpacing = nRowSpacing;
	spaceSize = nSpaceSize;
}

void CFreeTypeFont::Release()
{
	LOG(L"CFreeTypeFont::Release font:%s", shFontName.text);
	bLoaded = false;
	shFontName.Reset();
	m_atlas.Release();
}

RECTXYWH CFreeTypeFont::DrawStringLine(CStringDesc *strDesc, float X, float Y, UINT16 Flags, DWORD Color)
{
	RECTXYWH retBB;
	if (strDesc == nullptr)
		return retBB;

	Vec2 vpos(X, Y);

	UINT16 length = strDesc->len;
	UINT16* text = strDesc->codes;

	if (length == 0)
		return retBB;

	// we have to compute text line width
	int nLineW = 0;
	if (IS_FLAG_ANY(Flags, FTFF_CENTER | FTFF_RIGHT))
	{
		for (int ii = 0; ii < length; ii++)
		{
			int cod = text[ii];
			if (cod == K_STRMGR_RETURN)
				continue;
			else if (cod == K_STRMGR_SPACE)
			{
				nLineW += spaceSize;
				continue;
			}
			nLineW += m_atlas.arrGlyphs[cod].advanceX + letterSpacing;
		}

		if (Flags & FTFF_RIGHT)
		{
			vpos.x = X - nLineW;
		}
		else if (Flags & FTFF_CENTER)
		{
			vpos.x = X - nLineW / 2;
		}
	}

	if (Flags & FTFF_TOP)
	{
		vpos.y += rowHeight;
	}
	else if (Flags & FTFF_VCENTER)
	{
		// round to int so it looks crispy
		vpos.y += (int)(rowHeight / 2);
	}

	retBB.x = vpos.x; 
	retBB.y = vpos.y - rowHeight;

	// paint text
	nLineW = 0;
	for (int ii = 0; ii < length; ii++)
	{
		int cod = text[ii];
		if (cod == K_STRMGR_RETURN)
		{
			continue;
		}
		else if (cod == K_STRMGR_SPACE)
		{
			vpos.x += spaceSize;
			continue;
		}

		// draws the letter
		sGlyphInfo* glyph = &m_atlas.arrGlyphs[cod];
		UTPainter().Draw(m_atlas.pTex, glyph->texRect, glyph->moduleRectOff, vpos, Color);
		vpos.x += (float)(glyph->advanceX + letterSpacing);
		nLineW += glyph->advanceX + letterSpacing;
	}

	retBB.w = nLineW;
	retBB.h = rowHeight;

	return retBB;
}

RECTXYWH CFreeTypeFont::DrawStringLine(int strID, float X, float Y, UINT16 Flags /*= FTFF_BOTTOMLEFT*/, DWORD Color /*= 0xffffffff*/)
{
	int id = UTLang().getStrIdx(strID);
	return DrawStringLine(UTLang().strings[id], X, Y, Flags, Color);
}

