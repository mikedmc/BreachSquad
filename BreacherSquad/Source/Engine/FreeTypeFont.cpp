#include "dxstdafx.h"
#include "FreeTypeFont.h"

#include "ft2build.h"
#include FT_FREETYPE_H


///--- STATICS ---
//CStringsManager*				CFreeTypeFont::m_pStrManager = NULL;

CFreeTypeFont::CFreeTypeFont()
{
	//m_pStrManager = nullptr;
}

CFreeTypeFont::~CFreeTypeFont()
{
	Release();
}

OPRESULT CFreeTypeFont::CreateAtlas(PDEVICE pDevice, char* utf8Path, int nFontSize, WCHAR* wstrUniqueChars)
{
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

	// render glyphs to atlas
	char* pixels = new char[tex_width * tex_height];
	// clear pixels
	memset(pixels, 0, tex_width * tex_height);

	int pen_x = 0, pen_y = 0;

	for (int i = 0; i < nCharsCnt; ++i) {
		FT_Load_Char(face, wstrUniqueChars[i], FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LIGHT);
		FT_Bitmap* bmp = &face->glyph->bitmap;

		if (pen_x + bmp->width >= tex_width) {
			pen_x = 0;
			pen_y += ((face->size->metrics.height >> 6) + 1);
		}

		for (int row = 0; row < bmp->rows; ++row) {
			for (int col = 0; col < bmp->width; ++col) {
				int x = pen_x + col;
				int y = pen_y + row;
				pixels[y * tex_width + x] = bmp->buffer[row * bmp->pitch + col];
			}
		}

		// glyph data
		sGlyphInfo ginfo;

		ginfo.x0 = pen_x;
		ginfo.y0 = pen_y;
		ginfo.x1 = pen_x + bmp->width;
		ginfo.y1 = pen_y + bmp->rows;

		ginfo.x_off = face->glyph->bitmap_left;
		ginfo.y_off = face->glyph->bitmap_top;
		ginfo.advanceX = face->glyph->advance.x >> 6;

		m_atlas.arrGlyphs.Add(ginfo);

		pen_x += bmp->width + 1;
	}

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
			char pcol = pixels[yy * tex_width + xx];
			img[idx * 4 + 0] = pcol;
			img[idx * 4 + 1] = pcol;
			img[idx * 4 + 2] = pcol;
			img[idx * 4 + 3] = pcol;
		}
	}

	m_atlas.pTex->UnlockRect(0);

	delete [] pixels;

	bLoaded = true;
	shFontName.Init(utf8Path);
	LOG("CFreeTypeFont::CreateAtlas [%dx%d] font:%s", tex_width, tex_height, utf8Path);

	return K_OP_OK;
}

void CFreeTypeFont::Release()
{
	LOG(L"CFreeTypeFont::Release font:%s", shFontName.text);
	bLoaded = false;
	shFontName.Reset();
	m_atlas.Release();
}

