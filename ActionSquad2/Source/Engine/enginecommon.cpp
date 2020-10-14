#include "dxstdafx.h"

#include <io.h>
#include <direct.h>

#pragma warning(disable: 4995)
#pragma warning(default: 4995)

const DWORD VERT_TL2T::FVF = D3DFVF_XYZ | D3DFVF_TEX2;
const DWORD VERT_TL1T::FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
const DWORD VERT_TL1TS::FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
const DWORD VERT_TL1TC::FVF = D3DFVF_XYZRHW | D3DFVF_DIFFUSE;

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
	USHORT len = wcslen(inBuffer);
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

SIZEWH::SIZEWH()
{
	w = h = 0;
}
SIZEWH::SIZEWH(int nw, int nh)
{
	w = nw; h = nh;
}
SIZEWH::SIZEWH(const SIZEWH& szsrc)
{
	w = szsrc.w;
	h = szsrc.h;
}


SIZEWH_F::SIZEWH_F()
{
	w = h = 0;
}
SIZEWH_F::SIZEWH_F(float nw, float nh)
{
	w = nw; h = nh;
}
SIZEWH_F::SIZEWH_F(const SIZEWH_F& szsrc)
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


bool PointInRect(D3DXVECTOR2 pt, RECTXYWH_F rct)
{
	if ((pt.x < rct.x) || (pt.y < rct.y) || (pt.x > rct.x + rct.w) || (pt.y > rct.y + rct.h))
		return false;
	return true;
}

bool PointInRect(int x, int y, int rx, int ry, int rw, int rh)
{
	if ((x < rx) || (y < ry) || (x > rx + rw) || (y > ry + rh))
		return false;
	return true;
}

bool PointInRect(int x, int y, RECTXYWH *r)
{
	if((x<r->x) || (x>r->x + r->w) || (y < r->y) || (y > r->y + r->h))
		return false;
	return true;
}

bool PointInRect(float x, float y, RECTXYWH_F *r)
{
	if ((x<r->x) || (x>r->x + r->w) || (y < r->y) || (y > r->y + r->h))
		return false;
	return true;
}

bool PointInRect(POINT *pt, RECTXYWH *r)
{
	if((pt->x < r->x) || (pt->x > r->x + r->w) || (pt->y < r->y) || (pt->y > r->y + r->h))
		return false;
	return true;
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


void DrawRectUP_TL1T(LPDIRECT3DDEVICE9 pDevice, RECT scrRect, D3DXVECTOR2 texUL, D3DXVECTOR2 texDR, DWORD color)
{
	VERT_TL1T verts[4];
	verts[0].pos = D3DXVECTOR4(scrRect.left, scrRect.top, 0.0f, 1.0f);
	verts[1].pos = D3DXVECTOR4(scrRect.right, scrRect.top, 0.0f, 1.0f);
	verts[2].pos = D3DXVECTOR4(scrRect.left, scrRect.bottom, 0.0f, 1.0f);
	verts[3].pos = D3DXVECTOR4(scrRect.right, scrRect.bottom, 0.0f, 1.0f);
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
	DrawRectUP_TL1T(pDevice, rect, D3DXVECTOR2(0, 0), D3DXVECTOR2(0, 0), D3DCOLOR_XXXA(alpha));
	*/
}


void DrawLineUP_TL1T(LPDIRECT3DDEVICE9 pDevice, D3DXVECTOR2 start, D3DXVECTOR2 end, DWORD color )
{
	VERT_TL1T verts[2];
	verts[0].pos = D3DXVECTOR4(start.x, start.y, 0.0f, 1.0f);
	verts[1].pos = D3DXVECTOR4(end.x, end.y, 0.0f, 1.0f);
	verts[0].tu = 0.0f; verts[1].tu = 0.0f;
	verts[0].tv = 0.0f; verts[1].tv = 0.0f;
	verts[0].color = verts[1].color = color;

	pDevice->SetFVF(VERT_TL1T::FVF);
	pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, &verts, sizeof(VERT_TL1T));
}


D3DXVECTOR2 easing_a(0.0f, 0.0f), easing_at(1.0f, 0.0f), easing_b(1.0f, 1.0f), easing_bt(1.0f, 0.0f);
float TimeEasing(float t)
{
	D3DXVECTOR2 ret;
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
	if (UTGetAppClass().g_gfxFlags & K_UT_GFXFLAG_SCISSORTEST)
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


 ///--- STRING HASH FUNCTION ---
unsigned __int32 FastHash(const WCHAR *str)
{
	int len = wcslen(str);
    unsigned __int32 hash = 5381;
    int c;
	int flen = len;

	while(flen > 0)
	{
		flen--;
		c = tolower(*str++);
        hash = ((hash << 5) + hash) + c; // hash * 33 + c 
	}
    return hash;
}

unsigned __int32 FastHash(const WCHAR *str, int len)
{
    unsigned __int32 hash = 5381;
    int c;
	int flen = len;

	while(flen > 0)
	{
		flen--;
		c = tolower(*str++);
        hash = ((hash << 5) + hash) + c; // hash * 33 + c 
	}
    return hash;
}


unsigned __int32 FastHash(const char *str)
{
	int len = strlen(str);
    unsigned __int32 hash = 5381;
    int c;
	int flen = len;

	while(flen > 0)
	{
		flen--;
		c = tolower(*str++);
        hash = ((hash << 5) + hash) + c; // hash * 33 + c 
	}
    return hash;
}

unsigned __int32 FastHash(const char *str, int len)
{
    unsigned __int32 hash = 5381;
    int c;
	int flen = len;

	while(flen > 0)
	{
		flen--;
		c = tolower(*str++);
        hash = ((hash << 5) + hash) + c; // hash * 33 + c 
	}
    return hash;
}

//case sensitive functions (a bit faster)
unsigned __int32 FastHashCS(const WCHAR *str)
{
	int len = wcslen(str);
    unsigned __int32 hash = 5381;
    int c;
	int flen = len;

	while(flen > 0)
	{
		flen--;
		c = *str++;
        hash = ((hash << 5) + hash) + c; // hash * 33 + c 
	}
    return hash;
}
unsigned __int32 FastHashCS(const WCHAR *str, int len)
{
    unsigned __int32 hash = 5381;
    int c;
	int flen = len;

	while(flen > 0)
	{
		flen--;
		c = *str++;
        hash = ((hash << 5) + hash) + c; // hash * 33 + c 
	}
    return hash;
}

unsigned __int32 FastHashCS(const char *str)
{
	int len = strlen(str);
    unsigned __int32 hash = 5381;
    int c;
	int flen = len;

	while(flen > 0)
	{
		flen--;
		c = *str++;
        hash = ((hash << 5) + hash) + c; // hash * 33 + c 
	}
    return hash;
}

unsigned __int32 FastHashCS(const char *str, int len)
{
    unsigned __int32 hash = 5381;
    int c;
	int flen = len;

	while(flen > 0)
	{
		flen--;
		c = *str++;
        hash = ((hash << 5) + hash) + c; // hash * 33 + c 
	}
    return hash;
}


//calculeaza hash pt un fisier
unsigned __int32 GetFileHash(WCHAR *filename)
{
	long filebufSize = OS_GetFileSize(filename);
	if((filebufSize <= 0) || (filebufSize > 5000000))
	{
		ErrorBox(K_ERR_WARNING, L"GetFileHash::file size 0 or over 5MB!\n%s", filename);
		return 0;
	}

	errno_t err;
	FILE *fl = NULL;
	if( (err  = OS_wfopen_s(&fl, filename, L"rb")) !=0 )
	{
		ErrorBox(K_ERR_WARNING, L"GetFileHash::File does not exist!\n%s", filename);
		return 0;
	}

	char* filebuf = new char[filebufSize];

	OS_fread(filebuf, sizeof(char), filebufSize, fl);

	OS_fclose(fl);
	//make hash
	unsigned __int32 filehash = FastHash(filebuf, filebufSize);

	delete [] filebuf;

	return filehash;
}
//cauta un fisier cu acelasi nume (filename.ext.hash), citeste uint32 din el si verifica sa corespunda cu cel al fisierului
HRESULT CheckFileSignatureHash(WCHAR *filename)
{
	WCHAR signaturePath[MAX_PATH];
	StringCchPrintf(signaturePath, MAX_PATH, L"%s.hash", filename);
	unsigned __int32 fileHash = GetFileHash(filename);
	if(fileHash == 0)
	{
		return E_FAIL;
	}

	//deschide semnatura si citeste hash de acolo
	unsigned __int32 signatureHash = 0;
	errno_t err;
	FILE *fl = NULL;
	if( (err  = OS_wfopen_s(&fl, signaturePath, L"rb")) !=0 )
	{
		return E_FAIL;
	}

	OS_fread(&signatureHash, sizeof(unsigned __int32), 1, fl);
	OS_fclose(fl);

	if(signatureHash != fileHash)
	{
		return E_ACCESSDENIED;
	}

	return S_OK;
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

//RETURNS: tipul variabilei scrise in string: int, float sau string
eVarTypes GetTypeFromString(const WCHAR *str)
{
	int hasdot = 0;
	int len = wcslen(str);

	if(len <= 0)
		return K_RETTYPE_EMPTY;

	for(int kk=0; kk<len; kk++)
	{
		//ignora semnele + si - si spatiile
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

	return NULL;
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
	return m_variants.GetSize();
}

int CVariantCollection::AddVarINT32(INT32 val)
{
	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_INT32(NULL, val);
	m_variants.Add(nvar);
	return m_variants.GetSize();
}

int CVariantCollection::AddVarFloat(float val)
{
	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_FLOAT(NULL, val);
	m_variants.Add(nvar);
	return m_variants.GetSize();
}

int CVariantCollection::AddVarBool(bool val)
{
	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_BOOL(NULL, val);
	m_variants.Add(nvar);
	return m_variants.GetSize();
}

int CVariantCollection::AddVarVoidP(void* val)
{
	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_VOIDP(NULL, val);
	m_variants.Add(nvar);
	return m_variants.GetSize();
}

int CVariantCollection::AddVarString(WCHAR* strVal)
{
	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_STRING(NULL, strVal);
	m_variants.Add(nvar);
	return m_variants.GetSize();
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
	return m_variants.GetSize();
}

int CVariantCollection::AddVariant(CVariantComplex * variant)
{
	DeleteVar(variant->m_name.getHash());

	CVariantComplex* nvar = new CVariantComplex();
	*nvar = *variant;
	m_variants.Add(nvar);
	return m_variants.GetSize();
}

int CVariantCollection::SetNamedVarUINT32(const WCHAR* argName, UINT32 val)
{
	DeleteVar(argName);

	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_UINT32(argName, val);
	m_variants.Add(nvar);
	return m_variants.GetSize();
}

int CVariantCollection::SetNamedVarINT32(const WCHAR* argName, INT32 val)
{
	DeleteVar(argName);

	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_INT32(argName, val);
	m_variants.Add(nvar);
	return m_variants.GetSize();
}

int CVariantCollection::SetNamedVarFloat(const WCHAR* argName, float val)
{
	DeleteVar(argName);

	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_FLOAT(argName, val);
	m_variants.Add(nvar);
	return m_variants.GetSize();
}

int CVariantCollection::SetNamedVarBool(const WCHAR* argName, bool val)
{
	DeleteVar(argName);

	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_BOOL(argName, val);
	m_variants.Add(nvar);
	return m_variants.GetSize();
}

int CVariantCollection::SetNamedVarVoidP(const WCHAR* argName, void* val)
{
	DeleteVar(argName);

	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_VOIDP(argName, val);
	m_variants.Add(nvar);
	return m_variants.GetSize();
}

int CVariantCollection::SetNamedVarString(const WCHAR* argName, WCHAR* strVal)
{
	DeleteVar(argName);

	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_STRING(argName, strVal);
	m_variants.Add(nvar);
	return m_variants.GetSize();
}

int CVariantCollection::SetNamedVarAUTO(const WCHAR* argName, WCHAR* strVal)
{
	DeleteVar(argName);

	CVariantComplex* nvar = new CVariantComplex();
	nvar->Set_AUTO(argName, strVal);
	m_variants.Add(nvar);
	return m_variants.GetSize();
}

#if defined(_DEBUG) || defined(DEBUG)
void CVariantCollection::DumpDataToOutputWindow()
{
	//TODO: aici ar trebui sa ia in considerare tipul variantului pentru output
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
	wcscat(l_szTmp, L"\\*");

	hFind = FindFirstFile(l_szTmp, &FindFileData);
	if ((hFind == NULL) || (hFind == INVALID_HANDLE_VALUE)) 
		return FALSE;

	do
	{
		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wcscmp(FindFileData.cFileName, L"."))
			{
				if (wcscmp(FindFileData.cFileName, L".."))
				{
					wsprintf(l_szNewPath, L"%s\\%s", l_szPath, FindFileData.cFileName);
					OS_DeleteRecursive(l_szNewPath);
				}
			}
		}
		else
		{
			WCHAR l_szFile[1025] = { 0 };
			wsprintf(l_szFile, L"%s\\%s", l_szPath, FindFileData.cFileName);
			BOOL l_bRet = DeleteFile(l_szFile);
		}

	} while (FindNextFile(hFind, &FindFileData));
	FindClose(hFind);
	//delete actual folder
	BOOL l_bRet = RemoveDirectory(r_szPath);

	return TRUE;
#else 
	return FALSE;
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

	wcscat(l_szTmp, L"\\*");

	hFind = FindFirstFile(l_szTmp, &FindFileData);
	if ((hFind == NULL) || (hFind == INVALID_HANDLE_VALUE))
		return FALSE;

	do
	{

		if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (wcscmp(FindFileData.cFileName, L"."))
			{
				if (wcscmp(FindFileData.cFileName, L".."))
				{
					wsprintf(l_szNewDesPath, L"%s\\%s", l_szDesPath, FindFileData.cFileName);
					wsprintf(l_szNewSrcPath, L"%s\\%s", l_szSrcPath, FindFileData.cFileName);
					CreateDirectory(l_szNewDesPath, NULL);
					OS_CopyRecursive(l_szNewSrcPath, l_szNewDesPath);
				}
			}
		}
		else
		{
			WCHAR l_szSrcFile[1025] = { 0 };
			WCHAR l_szDesFile[1025] = { 0 };
			wsprintf(l_szDesFile, L"%s\\%s", l_szDesPath, FindFileData.cFileName);
			wsprintf(l_szSrcFile, L"%s\\%s", l_szSrcPath, FindFileData.cFileName);
			BOOL l_bRet = CopyFile(l_szSrcFile, l_szDesFile, FALSE); //overwrites existing files
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

//****************************************************************************************

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}
