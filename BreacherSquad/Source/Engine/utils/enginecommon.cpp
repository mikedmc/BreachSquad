#include "dxstdafx.h"

#include <io.h>
#include <direct.h>

#include "List.h"

#pragma warning(disable: 4995)
#pragma warning(default: 4995)

const DWORD VERT_TL2T::FVF = D3DFVF_XYZ | D3DFVF_TEX2;
const DWORD VERT_TL1T::FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
const DWORD VERT_TL1TS::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
const DWORD VERT_TL1TC::FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;


Vec2i GetDirVec2i(EDir dir)
{
	Vec2i dirs[4] = { Vec2i(-1, 0), Vec2i(0, -1), Vec2i(1, 0), Vec2i(0, 1) };

	int idx = (int)dir;
	if ((idx < 0) || (idx >= EDIRS_COUNT))
		return Vec2i(0, 0);
	return dirs[idx];
}

EDir GetDirInverse(EDir dir)
{
	if ((dir < 0) || (dir >= EDIRS_COUNT))
		return EDIR_NONE;
	return (EDir)((dir + 2) % EDIRS_COUNT);
}

void DW_COLOR_GETRBGA( DWORD hexColor, float & r, float & g, float & b, float & a )
{
	a = ((float)((hexColor & 0xff000000) >> 24) / 255.0f);
	r = ((float)((hexColor & 0x00ff0000) >> 16) / 255.0f);
	g = ((float)((hexColor & 0x0000ff00) >> 8) / 255.0f);
	b = ((float)(hexColor & 0x000000ff) / 255.0f);
}

DWORD DW_COLOR_LERP( DWORD dwFrom, DWORD dwTo, float s )
{
	float a;
	Vec4 cFrom, cTo;
	DW_COLOR_GETRBGA( dwFrom, cFrom.x, cFrom.y, cFrom.z, cFrom.w );
	DW_COLOR_GETRBGA( dwTo, cTo.x, cTo.y, cTo.z, cTo.w );
	Vec4 colOut;
	MUVec4Lerp( &colOut, &cFrom, &cTo, s );
	return DW_COLORVALUE( colOut.x, colOut.y, colOut.z, colOut.w );
}

EDir6 GetDir6FromVec(Vec2 vDir)
{
	if ((vDir.x == 0.0f) && (vDir.y == 0.0f))
		return EDIR6_S;
	// angle between -pi..pi
	float fang = atan2(vDir.y, vDir.x);
	int retang = 0;
	// divide angle in 3 equal parts and convert to EANG_
	if (fang < 0.0f)
		retang = ((int)EDIR6_NE - (int)fabs((fang / PI) * 3.0f));
	else
		retang = ((int)EDIR6_SE + (int)fabs((fang / PI) * 3.0f));
	// clamp to 6 possible directions
	CLAMP(retang, 0, 5);
	return (EDir6)retang;
}

void DW_COLOR_GETARGB(DWORD color, float & a, float & r, float & g, float & b)
{
	unsigned char cnl = color & 0x000000ff;
	b = (float)cnl / 255.0f;
	cnl = (color >> 8) & 0x000000ff;
	g = (float)cnl / 255.0f;
	cnl = (color >> 16) & 0x000000ff;
	r = (float)cnl / 255.0f;
	cnl = (color >> 24) & 0x000000ff;
	a = (float)cnl / 255.0f;
}

void DW_COLOR_GETBYTES(DWORD color, unsigned char & a, unsigned char & r, unsigned char & g, unsigned char & b)
{
	b = color & 0x000000ff;
	g = (color >> 8) & 0x000000ff;
	r = (color >> 16) & 0x000000ff;
	a = (color >> 24) & 0x000000ff;
}

DWORD SetSaturation(DWORD c, float S)
{
	float frac = 1.0f / 255.0f;
	float r, g, b, a;
	// facem culorile sa fie intre 0 si 1
	b = ((c >> 0) & 0xFF) * frac;
	g = ((c >> 8) & 0xFF) * frac;
	r = ((c >> 16) & 0xFF) * frac;
	a = ((c >> 24) & 0xFF) * frac;
	float h = 0.0f, s, l;

	// ===convertim in hsl===
	{ // sa nu poluam toata functia cu variabile cu uz intermediar
		float Cmax = max(r, max(g, b));
		float Cmin = min(r, min(g, b));

		l = Cmax;
		float delta = Cmax - Cmin;

		if (Cmax <= 0.0f) return c;
		s = delta / Cmax;

		// hue
		if (r >= Cmax) h = (g - b) / delta;
		else if (g >= Cmax) h = 2.0f + (b - r) / delta;
		else h = 4.0f + (r - g) / delta;

		h *= 60.0f;

		if (h < 0.0f)
			h += 360.0f;

		s *= S;
	}

	// === convertim inapoi in rgb ===
	{
		if (s <= 0.0f) { r = g = b = l; return D3DCOLOR_COLORVALUE(r, g, b, a); }
		float hh = h;
		if (hh >= 360.0f) hh = 0.0f;
		hh /= 60.0f;
		int i = (int)hh;
		float ff = hh - i;
		float p = l * (1.0f - s);
		float q = l * (1.0f - s * ff);
		float t = l * (1.0f - s * (1.0f - ff));

		float rgb[][3] = {
			{ l, t, p }, { q, l, p }, { p, l, t },
			{ p, q, l }, { t, p, l }, { l, p, q }
		};

		r = rgb[i][0];
		g = rgb[i][1];
		b = rgb[i][2];
	}

	return D3DCOLOR_COLORVALUE(r, g, b, a);
}


void REACH_VALUE_LINEAR(float &var, float targetVar, float fSpeed)
{
	if (var < targetVar)
	{
		var += fSpeed;
		if (var >= targetVar)
			var = targetVar;
	}
	else if (var > targetVar)
	{
		var -= fSpeed;
		if (var <= targetVar)
			var = targetVar;
	}
}

//functii de citit cate ceva din fisier
bool OS_freadBool(FILE *fl)
{
	bool ret;
	fread(&ret, sizeof(bool), 1, fl);
	return ret;
}

float OS_freadFloat32(FILE *fl)
{
	float ret;
	fread(&ret, sizeof(float), 1, fl);
	return ret;
}

INT8 OS_freadByte(FILE *fl)
{
	INT8 ret;
	fread(&ret, sizeof(INT8), 1, fl);
	return ret;
}
UINT8 OS_freadUByte(FILE *fl)
{
	UINT8 ret;
	fread(&ret, sizeof(UINT8), 1, fl);
	return ret;
}
INT16 OS_freadInt16(FILE* fl)
{
	INT16 ret;
	fread(&ret, sizeof(INT16), 1, fl);
	return ret;
}
UINT16 OS_freadUInt16(FILE* fl)
{
	UINT16 ret;
	fread(&ret, sizeof(UINT16), 1, fl);
	return ret;
}

UINT32 OS_freadUInt32(FILE* fl)
{
	UINT32 ret;
	fread(&ret, sizeof(UINT32), 1, fl);
	return ret;
}

INT32 OS_freadInt32(FILE* fl)
{
	INT32 ret;
	fread(&ret, sizeof(INT32), 1, fl);
	return ret;
}
//citeste un string CHAR. Primul numar citit va fi lungimea
//outBuffer trebuie sa fie suficient de mare pentru a citi tot (min 256)
void OS_freadString(FILE* fl, CHAR* outBuffer)
{
	outBuffer[0] = 0;
	int ctlen = OS_freadByte(fl);
	if(ctlen > 0)
	{
		for(int ll=0; ll<ctlen; ll++)
		{
			outBuffer[ll] = OS_freadByte(fl);
		}
		outBuffer[ctlen] = 0;
	}
}

void OS_freadWString(FILE* fl, WCHAR* outBuffer)
{
	if (fl == NULL) return;
	USHORT len = OS_freadUInt16(fl);
	outBuffer[len] = L'\0';
	fread(outBuffer, sizeof(WCHAR), len, fl);
}

void OS_fwriteWString(FILE* fl, WCHAR* inBuffer)
{
	if (fl == NULL) return;
	USHORT len = (USHORT)wcslen(inBuffer);
	fwrite(&len, sizeof(len), 1, fl);
	fwrite(inBuffer, sizeof(WCHAR), len, fl);
}

//--- functii de citit din buffer void valori ---
char buff_readByte(void* buff, long &_cursor)
{
	char *buf = (char*)buff;
	return(buf[_cursor++]);
}

unsigned char buff_readUByte(void* buff, long &_cursor)
{
	unsigned char *buf = (unsigned char*)buff;
	return(buf[_cursor++]);
}

short buff_readShort(void* buff, long &_cursor)
{
	unsigned char *buf = (unsigned char*)buff;
	short ret = (short)buf[_cursor] | (short)(buf[_cursor+1]<<8);
	_cursor+=2;
	return(ret);
}

unsigned short buff_readUShort(void* buff, long &_cursor)
{
	unsigned char *buf = (unsigned char*)buff;
	unsigned short ret = (unsigned short)buf[_cursor] | (unsigned short)(buf[_cursor+1]<<8);
	_cursor+=2;
	return(ret);
}

unsigned int buff_readUInt(void* buff, long &_cursor)
{
	unsigned char *buf = (unsigned char*)buff;
	unsigned int ret = (UINT)buf[_cursor] | (UINT)(buf[_cursor+1]<<8) | (UINT)(buf[_cursor+2]<<16) | (UINT)(buf[_cursor+3]<<24);
	_cursor+=4;
	return(ret);
}

SizeWHi::SizeWHi()
{
	w = h = 0;
}
SizeWHi::SizeWHi(int nw, int nh)
{
	w = nw; h = nh;
}
SizeWHi::SizeWHi(const SizeWHi& szsrc)
{
	w = szsrc.w;
	h = szsrc.h;
}


SizeWH::SizeWH()
{
	w = h = 0;
}
SizeWH::SizeWH(float nw, float nh)
{
	w = nw; h = nh;
}
SizeWH::SizeWH(const SizeWH& szsrc)
{
	w = szsrc.w;
	h = szsrc.h;
}


//--------------------------------------------------------------------------------------
// Mouse handling class
//--------------------------------------------------------------------------------------
void CMouseData::Update(float dTime)
{
	//increase time since last input
	if(!g_mouse.bCursorOutsideWindow)
		fTimeSinceInput += dTime;
	//--- mouse delta ---
#if defined(K_MOUSE_SMOOTH_DELTA)
	float fPercentOfNew = 1.0f / K_MOUSE_FRAMES_TO_SMOOTH_DELTA; //FramesToSmoothMouseData;
	float fPercentOfOld = 1.0f - fPercentOfNew;
	//cu smooth
	delta.x = delta.x * fPercentOfOld + (pos.x - lastPos.x) * fPercentOfNew;
	delta.y = delta.y * fPercentOfOld + (pos.y - lastPos.y) * fPercentOfNew;
#else
	//fara smooth
	delta.x = pos.x - lastPos.x;
	delta.y = pos.y - lastPos.y;
#endif
	//update last pos
	lastPos = pos;
	//left mouse buttons states
	if (bLbut)
	{
		if (Lbut == K_MOUSE_BUTT_NOTPRESSED)
		{
			Lbut = K_MOUSE_BUTT_JUSTPRESSED; //just pressed
		}
		else
			if (Lbut == K_MOUSE_BUTT_JUSTPRESSED)
				Lbut = K_MOUSE_BUTT_DRAG; //drag
	}
	else
	{
		if (Lbut == K_MOUSE_BUTT_DRAG)
		{
			Lbut = K_MOUSE_BUTT_JUSTRELEASED;
		}
		else
		{
			Lbut = K_MOUSE_BUTT_NOTPRESSED;
		}
	}
	//right mouse buttons states
	if (bRbut)
	{
		if (Rbut == K_MOUSE_BUTT_NOTPRESSED)
		{
			Rbut = K_MOUSE_BUTT_JUSTPRESSED; //just pressed
		}
		else
			if (Rbut == K_MOUSE_BUTT_JUSTPRESSED)
				Rbut = K_MOUSE_BUTT_DRAG; //drag
	}
	else
	{
		if (Rbut == K_MOUSE_BUTT_DRAG)
		{
			Rbut = K_MOUSE_BUTT_JUSTRELEASED;
		}
		else
		{
			Rbut = K_MOUSE_BUTT_NOTPRESSED;
		}
	}
	//clear delta
	//m_vMouseDelta.x = m_vMouseDelta.y = 0.0f;
}


long buff_gets(CHAR* _out, int _maxcount, long &_cursor, void* buff)
{
	long to = _cursor;
	char* buf = (char*)buff;

	do
	{
		to++;
	}
	while((buf[to]!=10)&&(buf[to]!=13));
	//sare si caracterele speciale	
	while((buf[to]==10)||(buf[to]==13)) to++;

	long count = to - _cursor;
	if(count>0) 
	{
		if(count>_maxcount)
		{
			memcpy(_out, buf+_cursor, _maxcount);
			_out[_maxcount] = 0;
		}	
		else
		{
			memcpy(_out, buf+_cursor, count);
			_out[count] = 0;
		}
	}
	else _out[0] = 0;
	_cursor = to;
	return count;
}



void OS_FormatTime(WCHAR* dest, int destSize, float timeInSecs)
{
	int tm = floor(timeInSecs);
	int secs = tm % 60;
	int mins = (tm / 60) % 60;
	int hours = tm / 3600;
	if (hours > 0)
	{
		StringCchPrintf(dest, destSize, L"%d:%02d:%02d", hours, mins, secs);
	}
	else
	{
		StringCchPrintf(dest, destSize, L"%d:%02d", mins, secs);
	}
}

//bool mouseIn(int x, int y, int w, int h){
//	return ((g_ptMouse.x >= x) && (g_ptMouse.y >= y)
//		&& (g_ptMouse.x <= x + w) && (g_ptMouse.y <= y + h));
//}
//
//bool mouseIn(RECT r){
//	return g_ptMouse.x >= r.left && g_ptMouse.y >= r.top
//		&& g_ptMouse.x <= r.right && g_ptMouse.y <= r.bottom;
//}
//
//bool mouseIn(RECTXYWH *r)
//{
//	if ((g_ptMouse.x < r->x) || (g_ptMouse.y < r->y) || (g_ptMouse.x >(r->x + r->w)) || (g_ptMouse.y >(r->y + r->h)))
//	{
//		return false;
//	}
//	return true;
//}


void DrawRectUP_TL1T(LPDIRECT3DDEVICE9 pDevice, RECT scrRect, Vec2 texUL, Vec2 texDR, DWORD color)
{
	VERT_TL1T verts[4];
	verts[0].pos = Vec4(scrRect.left, scrRect.top, 0.0f, 1.0f);
	verts[1].pos = Vec4(scrRect.right, scrRect.top, 0.0f, 1.0f);
	verts[2].pos = Vec4(scrRect.left, scrRect.bottom, 0.0f, 1.0f);
	verts[3].pos = Vec4(scrRect.right, scrRect.bottom, 0.0f, 1.0f);
	verts[0].tu = texUL.x;verts[1].tu = texDR.x;verts[2].tu = texUL.x;verts[3].tu = texDR.x;
	verts[0].tv = texUL.y;verts[1].tv = texUL.y;verts[2].tv = texDR.y;verts[3].tv = texDR.y;
	verts[0].color = verts[1].color = verts[2].color = verts[3].color = color;

	pDevice->SetFVF(VERT_TL1T::FVF);
	pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, &verts, sizeof(VERT_TL1T));
}

void DrawFullscreenVignette(LPDIRECT3DDEVICE9 pDevice, float alpha)
{
	if (alpha <= 0.0f)
		return;
/*
	// vignette
	RECT rect;
	SetRect(&rect, g_renderRect.x, g_renderRect.y, g_renderRect.Right(), g_renderRect.Bottom());
	pDevice->SetTexture(0, NULL); //textura aiurea
	pDevice->SetTransform(D3DTS_WORLD, &g_matIdentity);
	DrawRectUP_TL1T(pDevice, rect, Vec2(0, 0), Vec2(0, 0), D3DCOLOR_XXXA(alpha));
	*/
}


void DrawLineUP_TL1T(LPDIRECT3DDEVICE9 pDevice, Vec2 start, Vec2 end, DWORD color )
{
	VERT_TL1T verts[2];
	verts[0].pos = Vec4(start.x, start.y, 0.0f, 1.0f);
	verts[1].pos = Vec4(end.x, end.y, 0.0f, 1.0f);
	verts[0].tu = 0.0f; verts[1].tu = 0.0f;
	verts[0].tv = 0.0f; verts[1].tv = 0.0f;
	verts[0].color = verts[1].color = color;

	pDevice->SetFVF(VERT_TL1T::FVF);
	pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, &verts, sizeof(VERT_TL1T));
}


Vec2 easing_a(0.0f, 0.0f), easing_at(1.0f, 0.0f), easing_b(1.0f, 1.0f), easing_bt(1.0f, 0.0f);
float TimeEasing(float t)
{
	Vec2 ret;
	D3DXVec2Hermite(&ret, &easing_a, &easing_at, &easing_b, &easing_bt, t);
	return ret.y;
}

float EasingOutBackCubic(float t)
{
	float ts = t * t;
	float tc = ts * t;
	//return (4.0f * tc + -9.0 * ts + 6.0 * t);
	//return (-0.7025f*tc*ts + 1.355f*ts*ts + 4.395f*tc + -10.295f*ts + 6.2475f*t);
	return (-5.7*tc*ts + 11.4*ts*ts + -1.7*tc + -9*ts + 6*t);
}
//clipping functions
HRESULT SetScissorClip(LPDIRECT3DDEVICE9 pDevice, int clipX, int clipY, int clipW, int clipH)
{
	assert(pDevice != null);
	if (UTApp().g_gfxFlags & K_UT_GFXFLAG_SCISSORTEST)
	{
		RECT rect_colorClip;
		SetRect(&rect_colorClip, clipX, clipY, clipX + clipW, clipY + clipH);
		pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
		pDevice->SetScissorRect(&rect_colorClip);
		
		return S_OK;
	}
	else
		return E_FAIL;
}

HRESULT RemoveScissorClip(LPDIRECT3DDEVICE9 pDevice)
{
	assert(pDevice != null);
	pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	return S_OK;
}

std::vector<std::wstring> TokenizeString(const std::wstring& str, const std::wstring& delim)
{
	std::vector<std::wstring> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == std::wstring::npos) pos = str.length();
		std::wstring token = str.substr(prev, pos - prev);
		if (!token.empty()) tokens.push_back(token);
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());
	return tokens;
}

bool StringContainsAnyToken(const std::wstring& str, const std::wstring& strTokens, const std::wstring& tokensSeparator)
{
	std::vector<std::wstring> arrtokens = TokenizeString(strTokens, tokensSeparator);
	for (size_t kk = 0; kk < arrtokens.size(); kk++)
	{
		std::size_t found = str.find(arrtokens[kk]);
		if (found != std::string::npos)
			return true;
	}
	return false;
}

bool StringContainsAllTokens(const std::wstring& str, const std::wstring& strTokens, const std::wstring& tokensSeparator)
{
	std::vector<std::wstring> arrtokens = TokenizeString(strTokens, tokensSeparator);
	size_t nFound = 0;
	for (size_t kk = 0; kk < arrtokens.size(); kk++)
	{
		std::size_t found = str.find(arrtokens[kk]);
		if (found != std::string::npos)
			nFound++;
	}
	return (nFound == arrtokens.size());
}

bool GetVersionFromString(WCHAR * inStr, int & outMajor, int & outMinor, int & outPatch)
{
	if (inStr == null)
		return false;
	int nInLen = wcslen(inStr);
	if (nInLen < 5)
		return false;

	const std::wstring strIn(inStr);
	const std::wstring strDelim(L".");

	std::vector<std::wstring> arrTokens = TokenizeString(strIn, strDelim);

	if (arrTokens.size() != 3)
		return false;

	outMajor = _wtoi(arrTokens[0].c_str());
	outMinor = _wtoi(arrTokens[1].c_str());
	outPatch = _wtoi(arrTokens[2].c_str());

	return true;
}

///--- FILE FUNCTIONS ---
long OS_GetFileSize( WCHAR *path )
{
	FILE *pFile = NULL;
	errno_t err;
	if( (err  = _wfopen_s(&pFile, path, L"rb")) !=0 )
	{
		return 0;
	}

	fseek( pFile, 0, SEEK_END );
	long size = ftell( pFile );
	// close stream and release buffer
	fclose( pFile );
	return size;
}


//--- generates a unique UINT32 ID ---
UINT32 GenerateUID(void)
{
	WCHAR hashstr[MAX_PATH];
	WCHAR c1[11];
	for(int kk=0; kk<10; kk++)
	{
		int a = randint(900);
		if(a < 300)
			c1[kk] = 48 + randint(10);
		else if(a < 600)
			c1[kk] = 65 + randint(26);
		else
			c1[kk] = 97 + randint(26);
	}
	c1[10] = 0;
	StringCchPrintf(hashstr, MAX_PATH, L"UID[%s]%.6f-%.6f-%.6f-%.6f", c1, DXUTGetTime(), randfloat(10000.0f), randfloat(10000.0f), randfloat(10000.0f));
	return FastHashCS(hashstr, wcslen(hashstr));
}

///--- TIMERS ARRAY ---
CTimersArray::CTimersArray(int maxPeriod_ms, int minPeriod_ms)
{
	fMinPeriod = minPeriod_ms / (float)1000;
	nMinPeriod_ms = minPeriod_ms;

	timersCnt = maxPeriod_ms / minPeriod_ms;
	timers = new float[timersCnt];
	timerTicks = new bool[timersCnt];

	ResetTimers();
}

CTimersArray::~CTimersArray()
{
	SAFE_DELETE_ARRAY(timers);
	SAFE_DELETE_ARRAY(timerTicks);
}

void CTimersArray::Update(float dTime)
{
	for (int kk = 1; kk < timersCnt; kk++)
	{
		timers[kk] += dTime;
		timerTicks[kk] = false;
		float curTimerPeriod = (float)(kk * fMinPeriod);
		while (timers[kk] >= curTimerPeriod)
		{
			timers[kk] -= curTimerPeriod;
			timerTicks[kk] = true;
		}
	}
}
	
void CTimersArray::ResetTimers()
{
	for (int kk = 0; kk < timersCnt; kk++)
	{
		timers[kk] = 0.0f;
		timerTicks[kk] = false;
	}
}

//RETURNS: guesses the type of a string variable
eVarTypes GetTypeFromString(const WCHAR *str)
{
	int hasdot = 0;
	int len = wcslen(str);

	if(len <= 0)
		return K_RETTYPE_EMPTY;

	if ((str[0] == '#') && (len > 1))
		return K_RETTYPE_HEXCOLOR;

	for(int kk=0; kk<len; kk++)
	{
		//ignores +/- and space
		if((str[kk] == L'-') || (str[kk] == L'+') || (str[kk]==L' '))
			continue;
		if(str[kk] == L'.')
		{
			hasdot++;
			continue;
		}
		if(!isdigit(str[kk]))
		{
			return K_RETTYPE_STRING;
		}
	}

	if(hasdot)
		return K_RETTYPE_FLOAT;

	return K_RETTYPE_INT;
}

/**
 * CHAR CASE SENSITIVE replace function
 *
 * Searches all of the occurrences using recursion
 * and replaces with the given string
 * @param char * o_string The original string. Must be large enough!
 * @param char * s_string The string to search for
 * @param char * r_string The replace string
 * @return void The o_string passed is modified
 */
void str_replace(char * o_string, char * s_string, char * r_string) 
{
	//a buffer variable to do all replace things
	char buffer[MAX_PATH];
	//to store the pointer returned from strstr
	char * ch;

	//first exit condition
	if(!(ch = strstr(o_string, s_string)))
		return;

	//copy all the content to buffer before the first occurrence of the search string
	strncpy(buffer, o_string, ch-o_string);

	//prepare the buffer for appending by adding a null to the end of it
	buffer[ch-o_string] = 0;

	//append using sprintf function
	sprintf(buffer+(ch - o_string), "%s%s", r_string, ch + strlen(s_string));

	//empty o_string for copying
	o_string[0] = 0;
	strcpy(o_string, buffer);
	//pass recursively to replace other occurrences
	return str_replace(o_string, s_string, r_string);
}

/**
 * WCHAR CASE SENSITIVE replace function
 *
 * Searches all of the occurrences using recursion
 * and replaces with the given string
 * @param wchar * o_string The original string. Must be large enough!
 * @param wchar * s_string The string to search for
 * @param wchar * r_string The replace string
 * @return void The o_string passed is modified
 */
void wcs_replace(WCHAR* o_string, WCHAR* s_string, WCHAR* r_string) 
{
	//a buffer variable to do all replace things
	WCHAR buffer[4096];
	//to store the pointer returned from strstr
	WCHAR* ch;

	//first exit condition
	if(!(ch = wcsstr(o_string, s_string)))
		return;

	//copy all the content to buffer before the first occurrence of the search string
	wcsncpy(buffer, o_string, ch-o_string);

	//prepare the buffer for appending by adding a null to the end of it
	buffer[ch-o_string] = 0;

	//append using sprintf function
	StringCchPrintf(buffer+(ch - o_string), MAX_PATH, L"%s%s", r_string, ch + wcslen(s_string));

	//empty o_string for copying
	o_string[0] = 0;
	wcscpy(o_string, buffer);
	//pass recursively to replace other occurrences
	return wcs_replace(o_string, s_string, r_string);
 }

void CVariantComplex::Serialize(FILE *f)
{
	OS_fwrite(&m_type, sizeof(m_type), 1, f);
	OS_fwriteWString(f, m_name.text);

	switch (m_type)
	{
	case CVariantComplex::K_ARGTYPE_INT32:
		OS_fwrite(&m_asINT32, sizeof(INT32), 1, f);
		break;

	case CVariantComplex::K_ARGTYPE_FLOAT:
		OS_fwrite(&m_asFloat, sizeof(FLOAT), 1, f);
		break;

	case CVariantComplex::K_ARGTYPE_BOOL:
		OS_fwrite(&m_asBool, sizeof(bool), 1, f);
		break;

	case CVariantComplex::K_ARGTYPE_UINT32:
		OS_fwrite(&m_asUINT32, sizeof(UINT32), 1, f);
		break;

	case CVariantComplex::K_ARGTYPE_STRING:
		OS_fwriteWString(f, m_strArg.text);
		break;
	}
}

CVariantComplex* CVariantComplex::Deserialize(FILE *f)
{
	WCHAR name[K_MAX_STRINGHASH_LEN];
	WCHAR strVal[K_MAX_STRINGHASH_LEN];
	CVariantComplex::ArgumentType t;

	OS_fread(&t, sizeof(t), 1, f);
	OS_freadWString(f, name);

	switch (t)
	{
		case CVariantComplex::K_ARGTYPE_INT32:
		{
			CVariantComplex *nvc = new CVariantComplex();
			nvc->Set_INT32(name, OS_freadInt32(f));
			return nvc;
		}
		break;

		case CVariantComplex::K_ARGTYPE_FLOAT:
		{
			CVariantComplex *nvc = new CVariantComplex();
			nvc->Set_FLOAT(name, OS_freadFloat32(f));
			return nvc;
		}
		break;

		case CVariantComplex::K_ARGTYPE_BOOL:
		{
			CVariantComplex *nvc = new CVariantComplex();
			nvc->Set_BOOL(name, OS_freadBool(f));
			return nvc;
		}
		break;

		case CVariantComplex::K_ARGTYPE_UINT32:
		{
			CVariantComplex *nvc = new CVariantComplex();
			nvc->Set_UINT32(name, OS_freadUInt32(f));
			return nvc;
		}
		break;

		case CVariantComplex::K_ARGTYPE_STRING:
		{
			CVariantComplex *nvc = new CVariantComplex();
			OS_freadWString(f, strVal);
			nvc->Set_STRING(name, strVal);
			return nvc;
		}
		break;
	}

	return nullptr;
}

///--- CComplexVariant NAMED COLLECTION ---
//colectie cu nume pentru variants nume+valoare generala
CVariantCollection::CVariantCollection(CVariantCollection& collection)
{
	DeleteAll();
	m_collectionName.Init(collection.m_collectionName.text);
	
	for (int ii = 0; ii < collection.m_variants.Count(); ii++)
	{
		m_variants.Add(new CVariantComplex(*collection.m_variants[ii]));
	}
}
CVariantCollection::CVariantCollection(const WCHAR* strCollectionName)
{
	m_collectionName.Init(strCollectionName);
}
CVariantCollection::CVariantCollection()
{
	m_collectionName.Init(L"NO_NAME_COLLECTION");
}

CVariantCollection::~CVariantCollection()
{
	for(int kk=0; kk<m_variants.GetSize(); kk++)
	{
		SAFE_DELETE(m_variants[kk]);
	}
	m_variants.RemoveAll();
}

int CVariantCollection::AddVarUINT32(UINT32 val)
{
	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_UINT32(NULL, val);
	m_variants.Add(nvar);
	return m_variants.GetSize() - 1;
}

int CVariantCollection::AddVarINT32(INT32 val)
{
	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_INT32(NULL, val);
	m_variants.Add(nvar);
	return m_variants.GetSize() - 1;
}

int CVariantCollection::AddVarFloat(float val)
{
	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_FLOAT(NULL, val);
	m_variants.Add(nvar);
	return m_variants.GetSize() - 1;
}

int CVariantCollection::AddVarBool(bool val)
{
	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_BOOL(NULL, val);
	m_variants.Add(nvar);
	return m_variants.GetSize() - 1;
}

int CVariantCollection::AddVarVoidP(void* val)
{
	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_VOIDP(NULL, val);
	m_variants.Add(nvar);
	return m_variants.GetSize() - 1;
}

int CVariantCollection::AddVarString(WCHAR* strVal)
{
	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_STRING(NULL, strVal);
	m_variants.Add(nvar);
	return m_variants.GetSize() - 1;
}

//
// named functions
//
void CVariantCollection::Serialize(FILE *f)
{
	if (f == NULL) return;

	int nvars = m_variants.GetSize();

	OS_fwrite(&nvars, sizeof(nvars), 1, f);
	for (int ii = 0; ii < nvars; ii++)
	{
		m_variants[ii]->Serialize(f);
	}
}

void CVariantCollection::Deserialize(CVariantCollection* vc, FILE *f)
{
	if ((f == NULL) || (vc == NULL)) return;

	int nvars = 0;
	OS_fread(&nvars, sizeof(nvars), 1, f);

	for (int ii = 0; ii < nvars; ii++)
	{
		CVariantComplex* v = CVariantComplex::Deserialize(f);
		assert(v != NULL);
		vc->m_variants.Add(v);
	}
}

void CVariantCollection::DeleteVar(const WCHAR* varName)
{
	CStringHash argNameH(varName);
	DeleteVar(argNameH.getHash());
}

void CVariantCollection::DeleteVar(const UINT32 varHash)
{
	for (int kk = 0; kk<m_variants.GetSize(); kk++)
	{
		if (m_variants[kk]->m_name.getHash() == varHash)
		{
			SAFE_DELETE(m_variants[kk]);
			m_variants.Remove(kk);
			break;
		}
	}
}

void CVariantCollection::DeleteAll()
{
	for(int kk=0; kk<m_variants.GetSize(); kk++)
	{
		SAFE_DELETE(m_variants[kk]);
	}
	m_variants.RemoveAll();
}

int CVariantCollection::AddVariant(CVariantComplex variant)
{
	DeleteVar(variant.m_name.getHash());

	CVariantComplex* nvar = new CVariantComplex();
	*nvar = variant;
	m_variants.Add(nvar);
	return m_variants.GetSize() - 1;
}

int CVariantCollection::AddVariant(CVariantComplex * variant)
{
	DeleteVar(variant->m_name.getHash());

	CVariantComplex* nvar = new CVariantComplex();
	*nvar = *variant;
	m_variants.Add(nvar);
	return m_variants.GetSize() - 1;
}

int CVariantCollection::SetNamedVarUINT32(const WCHAR* argName, UINT32 val)
{
	DeleteVar(argName);

	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_UINT32(argName, val);
	m_variants.Add(nvar);
	return m_variants.GetSize() - 1;
}

int CVariantCollection::SetNamedVarHEXCOLOR(const WCHAR* argName, UINT32 val)
{
	DeleteVar(argName);

	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_HEXCOLOR(argName, val);
	m_variants.Add(nvar);
	return m_variants.GetSize() - 1;
}

int CVariantCollection::SetNamedVarINT32(const WCHAR* argName, INT32 val)
{
	DeleteVar(argName);

	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_INT32(argName, val);
	m_variants.Add(nvar);
	return m_variants.GetSize() - 1;
}

int CVariantCollection::SetNamedVarFloat(const WCHAR* argName, float val)
{
	DeleteVar(argName);

	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_FLOAT(argName, val);
	m_variants.Add(nvar);
	return m_variants.GetSize() - 1;
}

int CVariantCollection::SetNamedVarBool(const WCHAR* argName, bool val)
{
	DeleteVar(argName);

	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_BOOL(argName, val);
	m_variants.Add(nvar);
	return m_variants.GetSize() - 1;
}

int CVariantCollection::SetNamedVarVoidP(const WCHAR* argName, void* val)
{
	DeleteVar(argName);

	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_VOIDP(argName, val);
	m_variants.Add(nvar);
	return m_variants.GetSize() - 1;
}

int CVariantCollection::SetNamedVarString(const WCHAR* argName, WCHAR* strVal)
{
	DeleteVar(argName);

	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_STRING(argName, strVal);
	m_variants.Add(nvar);
	return m_variants.GetSize() - 1;
}

int CVariantCollection::SetNamedVarAUTO(const WCHAR* argName, WCHAR* strVal)
{
	DeleteVar(argName);

	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_AUTO(argName, strVal);
	m_variants.Add(nvar);
	return m_variants.GetSize() - 1;
}

#if defined(_DEBUG) || defined(DEBUG)
void CVariantCollection::DumpDataToOutputWindow()
{
	//#TODO: aici ar trebui sa ia in considerare tipul variantului pentru output
	for (int kk = 0; kk < m_variants.GetSize(); kk++)
	{
		DebugPrintFnW(L"%s=%d\n", m_variants[kk]->m_name.text, m_variants[kk]->m_asUINT32);
	}
}
#endif

//
// get argument
//
CVariantComplex* CVariantCollection::GetVariantByName(const WCHAR* argName)
{
	UINT32 argNameHash = FastHash(argName);

	register int kk = 0;
	//optimizare de viteza
	for(kk=0; kk<m_variants.GetSize(); kk++)
	{
		if(m_variants[kk]->m_name.textHash == argNameHash)
			return m_variants[kk];
	}
	
	return &defaultVariant;
}

CVariantComplex* CVariantCollection::GetVariantByNameHash(const UINT32 varNameHash)
{
	register int kk = 0;
	//exit if not initialized
	if(varNameHash == 0)
		return &defaultVariant;
	//optimizare de viteza
	for( kk = 0; kk < m_variants.GetSize(); kk++ )
	{
		if(m_variants[kk]->m_name.textHash == varNameHash)
			return m_variants[kk];
	}
	return &defaultVariant;
}

//functie generica de cautat un string intr-o lista de CStringHash si intors indexul lui. Folositor la parsarea xml-urilor pt conversie in valori
int GetListIndexByName(const WCHAR * strName, const CStringHash *arrNamesList, int arrNamesListSize)
{
	UINT32 listNameHash = FastHash(strName);
	for (int kk = 0; kk < arrNamesListSize; kk++)
	{
		if (arrNamesList[kk].textHash == listNameHash)
			return kk;
	}
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	ErrorBox(K_ERR_WARNING, L"Item not found in list: %s", strName);
#endif
	return -1;
}

int GetListIndexByNameHash(const UINT32 nameHash, const CStringHash *arrNamesList, int arrNamesListSize)
{
	for (int kk = 0; kk < arrNamesListSize; kk++)
	{
		if (arrNamesList[kk].textHash == nameHash)
			return kk;
	}
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	ErrorBox(K_ERR_WARNING, L"Item not found in list (by hash): %d", nameHash);
#endif
	return -1;
}

char* OS_readFileToBuffer(const WCHAR* wcsPath, int &nRetSize)
{
	FILE *pFile = NULL;
	errno_t err;
	if ((err = _wfopen_s(&pFile, wcsPath, L"rb")) != 0)
	{
		return null;
	}

	fseek(pFile, 0, SEEK_END);
	long size = ftell(pFile);
	//allocate buffer
	if (size <= 0)
		return null;

	fseek(pFile, 0, SEEK_SET);
	char* retPtr = new char[size];
	fread(retPtr, sizeof(char), size, pFile);
	// close stream
	fclose(pFile);

	nRetSize = size;
	return retPtr;
}

unsigned char* OS_readFileToBufferUC(const WCHAR* wcsPath, int &nRetSize)
{
	FILE *pFile = NULL;
	errno_t err;
	if ((err = _wfopen_s(&pFile, wcsPath, L"rb")) != 0)
	{
		return null;
	}

	fseek(pFile, 0, SEEK_END);
	long size = ftell(pFile);
	//allocate buffer
	if (size <= 0)
		return null;

	fseek(pFile, 0, SEEK_SET);
	unsigned char* retPtr = new unsigned char[size];
	fread(retPtr, sizeof(unsigned char), size, pFile);
	// close stream
	fclose(pFile);

	nRetSize = size;
	return retPtr;
}


int OS_GetFileNameWithoutExtension(WCHAR * destBuffer, int destSize, WCHAR * srcPath)
{
	if (srcPath == null)
		return -1;
	if ((int)wcslen(srcPath) == 0)
		return 0;

	StringCchCopy(destBuffer, destSize, srcPath);

	int nIdx = (int)wcslen(destBuffer);
	while (--nIdx > 0 && destBuffer[nIdx] != '.');
	if (nIdx <= 0)
		return 0;

	destBuffer[nIdx] = '\0';
	return nIdx;
}

int OS_GetFileNameExtension(WCHAR * destStr, int destSize, WCHAR * srcPath)
{
	if (srcPath == null)
		return -1;
	int srclen = (int)wcslen(srcPath);
	if (srclen == 0)
		return 0;

	int nIdx = (int)wcslen(srcPath);
	while (--nIdx > 0 && srcPath[nIdx] != '.');
	if (nIdx <= 0)
		return 0;
	// ending in "." ?
	int extlen = srclen - nIdx - 1;
	if (extlen <= 0)
		return 0;
	if (extlen > destSize - 1)
		extlen = destSize - 1;

	memcpy(destStr, srcPath + nIdx + 1, extlen * sizeof(WCHAR));
	destStr[extlen] = '\0';
	return extlen;
}


unsigned int OS_GetTimeMS()
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	LARGE_INTEGER timer;
	QueryPerformanceCounter(&timer);

	unsigned int time = (unsigned int)(timer.QuadPart * 1000 / frequency.QuadPart);
	return time;
}

bool OS_CreateFolder(const char* szPath)
{
	BOOL result = CreateDirectoryA(szPath, NULL);

	DWORD err = GetLastError();
	if (result == 0 && err == ERROR_ALREADY_EXISTS)
		return true;
	return (result != 0);
}

bool OS_DeleteFolder(const char* szPath)
{
	return (_rmdir(szPath) == 0);
}

void OS_GetFolderFiles(const char *directory, const char *extension, List<char*>& list, bool bFullPath)
{
	if (!extension)
		extension = "";

	// passing a slash as extension will find directories
	int	flag;
	if ( extension[0] == '/' && extension[1] == 0 )
	{
		extension = "";
		flag = 0;
	}
	else
	{
		flag = _A_SUBDIR;
	}

	char search[MAX_PATH_STD];
	sprintf(search, "%s/*%s", directory, extension);

	struct _finddata_t findinfo;
	int findhandle = _findfirst(search, &findinfo);
	if (findhandle == -1)
		return;

	do
	{
		if (!(flag ^ (findinfo.attrib & _A_SUBDIR)))
			continue;

		char* pName = NULL;
		if (bFullPath)
		{
			pName = new char[strlen(directory) + strlen(findinfo.name) + 2]; // a backslash and null terminator
			sprintf(pName, "%s/%s", directory, findinfo.name);
		}
		else
			pName = strdup(findinfo.name);
		list.Add(pName);
	} while (_findnext(findhandle, &findinfo) != -1);

	_findclose(findhandle);
}

/*
//from Gosa pe 26 nov 2018
bool OS_CreateFolder(const char* szPathUTF8)
{
	// convert to wchar
	wchar_t szPathWC[MAX_PATH_STD];
	int convResult = MultiByteToWideChar(CP_UTF8, 0, szPathUTF8, -1, szPathWC, sizeof(szPathWC));
	ASSERT(convResult != 0);

	BOOL result = CreateDirectoryW(szPathWC, NULL);
	DWORD err = GetLastError();
	if (result == 0 && err == ERROR_ALREADY_EXISTS)
		return true;

	// if !result, probably ERROR_PATH_NOT_FOUND
	ASSERT(result != 0);
	return (result != 0);
}

bool OS_DeleteFolder(const char* szPathUTF8)
{
	// convert to wchar
	wchar_t szPathWC[MAX_PATH_STD];
	int result = MultiByteToWideChar(CP_UTF8, 0, szPathUTF8, -1, szPathWC, sizeof(szPathWC));
	ASSERT(result != 0);

	return (_wrmdir(szPathWC) == 0);
}


int OS_fremove(const char* szPathUTF8)
{
	wchar_t szPathWC[MAX_PATH_STD];
	int result = MultiByteToWideChar(CP_UTF8, 0, szPathUTF8, -1, szPathWC, sizeof(szPathWC));
	ASSERT(result != 0);
	return _wremove(szPathWC);
}

int OS_fstat(const char* szPathUTF8, struct _stat32* filestat)
{
	wchar_t szPathWC[MAX_PATH_STD];
	int result = MultiByteToWideChar(CP_UTF8, 0, szPathUTF8, -1, szPathWC, sizeof(szPathWC));
	ASSERT(result != 0);

	return _wstat32(szPathWC, filestat);
}

int OS_frename(const char* szFileFrom, const char* szFileTo)
{
	wchar_t szFromW[MAX_PATH_STD];
	wchar_t szToW[MAX_PATH_STD];
	MultiByteToWideChar(CP_UTF8, 0, szFileFrom, -1, szFromW, sizeof(szFromW));
	MultiByteToWideChar(CP_UTF8, 0, szFileTo, -1, szToW, sizeof(szToW));
	return _wrename(szFromW, szToW);
}
*/

//****************************************************************************************

bool OS_DeleteRecursive(WCHAR r_szPath[1024])
{
#ifdef ENABLE_STEAM_WORKSHOP
	//WINDOWS ONLY - deletes folders recursively
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	WCHAR l_szPath[1025] = { 0 };
	memcpy(l_szPath, r_szPath, 1024 * sizeof(WCHAR));

	WCHAR l_szNewPath[1025] = { 0 };

	WCHAR l_szTmp[1025] = { 0 };
	memcpy(l_szTmp, r_szPath, 1024 * sizeof(WCHAR));
	wcscat(l_szTmp, L"/*");

	hFind = FindFirstFile(l_szTmp, &FindFileData);
	if ((hFind == NULL) || (hFind == INVALID_HANDLE_VALUE)) 
		return false;

	do
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wcscmp(FindFileData.cFileName, L"."))
			{
				if (wcscmp(FindFileData.cFileName, L".."))
				{
					swprintf_s(l_szNewPath, 1025, L"%s/%s", l_szPath, FindFileData.cFileName);
					OS_DeleteRecursive(l_szNewPath);
				}
			}
		}
		else
		{
			WCHAR l_szFile[1025] = { 0 };
			swprintf_s(l_szFile, 1025, L"%s/%s", l_szPath, FindFileData.cFileName);
			if (!DeleteFile(l_szFile))
			{
				LOG(L"OS_DeleteRecursive could not DeleteFile: %s", l_szFile);
			}
		}

	} while (FindNextFile(hFind, &FindFileData));
	FindClose(hFind);
	//delete actual folder
	/*BOOL l_bRet = */RemoveDirectory(r_szPath);

	return true;
#else 
	return false;
#endif
}

void OS_FileCopy(FILE *dest, FILE *src)
{
	const int size = 16384;
	char buffer[size];

	while (!feof(src))
	{
		int n = fread(buffer, 1, size, src);
		fwrite(buffer, 1, n, dest);
	}

	fflush(dest);
}

bool OS_CopyRecursive(WCHAR r_szSrcPath[1024], WCHAR r_szDesPath[1024])
{
#ifdef ENABLE_STEAM_WORKSHOP
	//WINDOWS ONLY - copies folders recursively
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	WCHAR l_szTmp[1025] = { 0 };
	memcpy(l_szTmp, r_szSrcPath, 1024 * sizeof(WCHAR));


	WCHAR l_szSrcPath[1025] = { 0 };
	WCHAR l_szDesPath[1025] = { 0 };
	memcpy(l_szSrcPath, r_szSrcPath, 1024 * sizeof(WCHAR));
	memcpy(l_szDesPath, r_szDesPath, 1024 * sizeof(WCHAR));

	WCHAR l_szNewSrcPath[1025] = { 0 };
	WCHAR l_szNewDesPath[1025] = { 0 };

	wcscat(l_szTmp, L"/*");

	hFind = FindFirstFile(l_szTmp, &FindFileData);
	if ((hFind == NULL) || (hFind == INVALID_HANDLE_VALUE))
		return false;

	do
	{

		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wcscmp(FindFileData.cFileName, L"."))
			{
				if (wcscmp(FindFileData.cFileName, L".."))
				{
					swprintf_s(l_szNewDesPath, 1025, L"%s/%s", l_szDesPath, FindFileData.cFileName);
					swprintf_s(l_szNewSrcPath, 1025, L"%s/%s", l_szSrcPath, FindFileData.cFileName);
					CreateDirectory(l_szNewDesPath, NULL);
					OS_CopyRecursive(l_szNewSrcPath, l_szNewDesPath);
				}
			}
		}
		else
		{
			WCHAR l_szSrcFile[1025] = { 0 };
			WCHAR l_szDesFile[1025] = { 0 };
			swprintf_s(l_szDesFile, 1025, L"%s/%s", l_szDesPath, FindFileData.cFileName);
			swprintf_s(l_szSrcFile, 1025, L"%s/%s", l_szSrcPath, FindFileData.cFileName);
			BOOL l_bRet = CopyFile(l_szSrcFile, l_szDesFile, FALSE); //overwrites existing files
			if (!l_bRet) 
			{
				LOG(L"OS_CopyRecursive failed to copy file: %s", l_szSrcFile);
			}
		}

	} while (FindNextFile(hFind, &FindFileData));
	FindClose(hFind);
	return TRUE;
#else 
	return FALSE;
#endif
}

std::wstring RemoveQuotationMarks(const std::wstring& initialString)
{
	std::wstring sOutString = initialString;
	if( sOutString.size() )
	{
		if( sOutString[0] == '\"' )
			sOutString.erase(sOutString.begin() +0);

		if( sOutString[sOutString.size() -1] == '\"' )
			sOutString.erase(sOutString.begin() +sOutString.size() -1);
	}
	return sOutString;
}

int WCHARtoUTF8(char* dest, WCHAR* src, int maxSize)
{
	return WideCharToMultiByte(CP_UTF8, 0, src, (int)wcslen(src), dest, maxSize - 1, null, null);
}

int UTF8toWCHAR(WCHAR* dest, char* src, int maxSize)
{
	return MultiByteToWideChar(CP_UTF8, 0, src, (int)strlen(src), dest, maxSize - 1);
}


//****************************************************************************************

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}


int utf8_next(const char**  pcursor, const char*   end)
{
	const unsigned char*  p = (const unsigned char*)*pcursor;
	int                   ch;


	if ((const char*)p >= end) /* end of stream */
		return -1;

	ch = *p++;
	if (ch >= 0x80)
	{
		int  len;


		if (ch < 0xc0)  /* malformed data */
			goto BAD_DATA;
		else if (ch < 0xe0)
		{
			len = 1;
			ch &= 0x1f;
		}
		else if (ch < 0xf0)
		{
			len = 2;
			ch &= 0x0f;
		}
		else
		{
			len = 3;
			ch &= 0x07;
		}

		while (len > 0)
		{
			if ((const char*)p >= end || (p[0] & 0xc0) != 0x80)
				goto BAD_DATA;

			ch = (ch << 6) | (p[0] & 0x3f);
			p += 1;
			len -= 1;
		}
	}

	*pcursor = (const char*)p;

	return ch;

BAD_DATA:
	return -1;
}
