#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <stddef.h>

//#define null NULL

#define DW_COLOR_FFFA(a) (DWORD)(((int((a) * 255.0f)&0xff)<<24) | 0xffffff)
#define DW_COLOR_XXXA(a) (DWORD)(((int((a) * 255.0f)&0xff)<<24) | 0x000000)
#define DW_COLORALPHA(hexColor, fAlpha) (DWORD)(((int((fAlpha) * 255.0f)&0xff)<<24) | (hexColor & 0xffffff))
#define DW_GETFALPHA(hexColor) ((float)((hexColor & 0xff000000) >> 24) / 255.0f)
// channels are float between 0 and 1
#define DW_COLORVALUE(r,g,b,a) \
    D3DCOLOR_RGBA((DWORD)((r)*255.f),(DWORD)((g)*255.f),(DWORD)((b)*255.f),(DWORD)((a)*255.f))
// returns colors on channels between 0 and 1
void DW_COLOR_GETRBGA(DWORD hexColor, float & r, float & g, float & b, float & a);
// interpolates between 2 colors returning DWORD value
DWORD DW_COLOR_LERP( DWORD dwFrom, DWORD dwTo, float s );
// Unpacks DWORD color to float channels
void DW_COLOR_GETARGB( DWORD color, float & a, float & r, float & g, float & b );
// Unpacks DWORD color to byte channels
void DW_COLOR_GETBYTES( DWORD color, unsigned char & a, unsigned char & r, unsigned char & g, unsigned char & b );

// generic sides (corresponds to generic directions)
#define K_SIDE_NONE -1
#define K_SIDE_LEFT 0
#define K_SIDE_TOP 1
#define K_SIDE_RIGHT 2
#define K_SIDE_BOTTOM 3

// generic directions (clockwise starting left)
#define K_DIR_NONE -1
#define K_DIR_LEFT 0
#define K_DIR_UP 1
#define K_DIR_RIGHT 2
#define K_DIR_DOWN 3

enum EDir {
	EDIR_NONE = -1,
	EDIR_LEFT = 0,
	EDIR_UP = 1,
	EDIR_RIGHT = 2,
	EDIR_DOWN = 3,

	EDIRS_COUNT = 4,
};

// Actor animation angles (6 possible directions)
// the order of the enum is important as it helps extract the direction from atan2 results (see GetEAnimAngle)
enum EDir6 {
	EDIR6_NONE = -1,

	EDIR6_N = 0,
	EDIR6_NE,
	EDIR6_SE,
	EDIR6_S,
	EDIR6_SW,
	EDIR6_NW,

	EDIR6S_CNT,
};
// names of the animation directions
const CStringHash EDir6Names[ EDIR6S_CNT ] = { L"N", L"NE", L"SE", L"S", L"SW", L"NW" };

// Characters are animated on 6 directions: see EAnimAngle
// Returns animation direction as int, starting with top(0)
EDir6 GetDir6FromVec( Vec2 vDir );
// returns a direction vector for specified direction
Vec2 GetDir6VecN( EDir6 eDir );

//direction flags used when setting more directions on one int
#define K_DIRFLAG_NONE 0
#define K_DIRFLAG_LEFT 1
#define K_DIRFLAG_UP 2
#define K_DIRFLAG_RIGHT 4
#define K_DIRFLAG_DOWN 8
#define K_DIRFLAG_UP_DOWN 10
#define K_DIRFLAG_LEFT_RIGHT 5
#define K_DIRFLAG_ALL 15

//value not set/touched
#define K_DEAD_BEEF		0xDEADBEEF
#define K_NOT_SET		0xDEADBEEF

#define DEG_TO_RAD(a) ((a / 360.0f) * DOUBLE_PI)
#define RAD_TO_DEG(a) ((a / DOUBLE_PI) * 360.0f)

template <class anyType>
__inline void CLAMP(anyType &var, anyType min, anyType max)
{
	var = ((var < min) ? min : ((var > max) ? max : var));
}
/*
Limits "var" between "min" and "max" without changing the actual value of "var".
RETURNS: clamped value
*/
template <class anyType>
__inline anyType LIMIT(anyType var, anyType min, anyType max)
{
	return ((var < min) ? min : ((var > max) ? max : var));
}

template <class T>
void SWAP(T& x, T& y)
{
	T temp;
	temp = x;
	x = y;
	y = temp;
}



struct VERT_TL1TC
{
	Vec4 pos;
	DWORD color;

	static const DWORD FVF;
};

struct VERT_TL1T
{
	Vec4 pos;
	DWORD color;
	float tu, tv;

	static const DWORD FVF;
};

struct VERT_TL1TS
{
	Vec3 pos;
	DWORD color;
	float tu, tv;

	static const DWORD FVF;
};

struct VERT_TL2T
{
	Vec3 pos;
	float tu, tv;
	float lu, lv;

	static const DWORD FVF;
};

struct Vec2i {
	int x, y;
	Vec2i() :x(0), y(0) {}
	Vec2i(int nx, int ny) :x(nx), y(ny) {}
	Vec2i(const Vec2i& point) { x = point.x; y = point.y; }
	bool operator==(const Vec2i &other) const { return ((other.x == x) && (other.y == y)); }
	bool operator!=(const Vec2i &other) const { return ((other.x != x) || (other.y != y)); }
	operator Vec2() { return Vec2((float)x, (float)y); }
};

struct Vec3i {
	int x, y, z;
	Vec3i() :x(0), y(0), z(0) {}
	Vec3i(int nx, int ny, int nz) { x = nx; y = ny; z = nz; }
	Vec3i(const Vec3i& point) { x = point.x; y = point.y; z = point.z; }
	bool operator==(const Vec3i &other) const { return ((other.x == x) && (other.y == y) && (other.z == z)); }
	bool operator!=(const Vec3i &other) const { return ((other.x != x) || (other.y != y) || (other.z != z)); }
	operator Vec2() { return Vec2((float)x, (float)y); }
	operator Vec3() { return Vec3((float)x, (float)y, (float)z); }
};

// Gets direction vector (integer) from EDir
Vec2i GetDirVec2i(EDir dir);
// Gets inverse EDir from EDir
EDir GetDirInverse(EDir dir);

class SizeWHi {
public:
	int w, h;
	SizeWHi();
	SizeWHi(int nw, int nh);
	SizeWHi(const SizeWHi& szsrc);
	bool operator==(const SizeWHi &other) const { return ((other.w == w) && (other.h == h)); }
	void Init(int nw, int nh) {
		w = nw; h = nh;
	}
};

class SizeWH {
public:
	float w, h;
	SizeWH();
	SizeWH(float nw, float nh);
	SizeWH(const SizeWH& szsrc);
	bool operator==(const SizeWH &other) const { return (FLOATS_EQUAL(other.w, w, EPS) && FLOATS_EQUAL(other.h, h, EPS)); }
};


//--------------------------------------------------------------------------------------
// Mouse handling class
//--------------------------------------------------------------------------------------
//mouse buttons status
enum EMouseButtonState {
	K_MOUSE_BUTT_NOTPRESSED,
	K_MOUSE_BUTT_JUSTPRESSED,
	K_MOUSE_BUTT_DRAG,
	K_MOUSE_BUTT_JUSTRELEASED,
};
//mouse delta smooth
//#define K_MOUSE_SMOOTH_DELTA  true
#define K_MOUSE_FRAMES_TO_SMOOTH_DELTA 5.0f

class CMouseData {
public:
	bool	bLbut; //daca e apasat sau nu
	bool	bRbut; //daca e apasat sau nu
public:
	Vec2 pos; //in coordonate transformate prin camera transform
	Vec2	lastPos; //last mouse position in coord reale de viewport
	Vec2 delta; //delta movement
	EMouseButtonState Lbut; 
	EMouseButtonState Rbut; 
	int     wheelDelta; //delta rotita scroll

	bool	bCursorOutsideWindow;	//setat cand iese cursorul din fereastra
	float	fTimeSinceInput;		//time since last input received

	CMouseData() : pos(0.0f, 0.0f), lastPos(0.0f, 0.0f), delta(0.0f, 0.0f),
		bLbut(false), bRbut(false), Lbut(K_MOUSE_BUTT_NOTPRESSED), Rbut(K_MOUSE_BUTT_NOTPRESSED), bCursorOutsideWindow(false),
		fTimeSinceInput(0.0f)
	{};
	void	Update(float dTime);
};


//--------------------------------------------------------------------------------------
// Controller handling class
//--------------------------------------------------------------------------------------
//buttons status
enum EKeyState {
	K_KEYSTATE_NOTPRESSED,
	K_KEYSTATE_JUSTPRESSED,
	K_KEYSTATE_PRESSING,
	K_KEYSTATE_JUSTRELEASED,
};
//buttons
enum EControllerKeys {
	//player 1
	K_KEY_LEFT = 0,
	K_KEY_RIGHT = 1,
	K_KEY_UP,
	K_KEY_DOWN,
	K_KEY_JUMP,
	K_KEY_FIRE1,
	K_KEY_FIRE2,
	K_KEY_RELOAD,
	K_KEY_USE_GEAR,
	//count
	K_KEYS_COUNT
};


// changes saturation (rgb->hsl->h(s*S)l->rgb)
DWORD SetSaturation(DWORD c, float S);

__inline DWORD FtoDW(float f)
{
	return *((DWORD*)(&f));
}


// var changes into targetVar with specified fSpeed (must be called in loop)
void REACH_VALUE_LINEAR(float &var, float targetVar, float fSpeed);

///--- structura care poate contine mai multe tipuri de date ---
struct CVariant 
{
	enum Type 
	{
		K_VTYPE_INT32,
		K_VTYPE_FLOAT,
		K_VTYPE_BOOL,
		K_VTYPE_UINT32,
		K_VTYPE_VOIDP,

		K_VTYPE_COUNT
	};
	Type m_type;

	union 
	{
		INT32	m_asINT32;
		FLOAT	m_asFloat;
		bool	m_asBool;
		UINT32  m_asUINT32;
		VOID*	m_asVoid;
	};

	//--- conversion fn ---
	int asString(WCHAR *destStr, int maxLen)
	{
		switch (m_type)
		{
			case K_VTYPE_INT32:
				StringCchPrintf(destStr, maxLen, L"%d", m_asINT32);
				break;
			case K_VTYPE_FLOAT:
				StringCchPrintf(destStr, maxLen, L"%.2f", m_asFloat);
				break;
			default:
				StringCchPrintf(destStr, maxLen, L"%d", m_asUINT32);
				break;
		}
	}

	INT32 asInt32()
	{
		switch (m_type)
		{
			case K_VTYPE_FLOAT:
				return (int)m_asFloat;
			default:
				return m_asINT32;
		}
	};

	float asFloat() 
	{
		switch (m_type)
		{
			case K_VTYPE_FLOAT:
				return m_asFloat;
			case K_VTYPE_UINT32:
				return (float)m_asUINT32;
			default:
				return (float)m_asINT32;
		}
	};
};


//--- helper functions to read from files
INT8				OS_freadByte(FILE *fl);
UINT8				OS_freadUByte(FILE *fl);
INT16				OS_freadInt16(FILE* fl);
UINT16				OS_freadUInt16(FILE* fl);
UINT32				OS_freadUInt32(FILE* fl);
INT32				OS_freadInt32(FILE *fl);
bool				OS_freadBool(FILE *fl);
float				OS_freadFloat32(FILE *fl);
void				OS_freadString(FILE* fl, CHAR* outBuffer);
void				OS_freadWString(FILE* fl, WCHAR* outBuffer);
void				OS_fwriteWString(FILE* fl, WCHAR* inBuffer);

//read from buffer
//returns cursor position
long				buff_gets(CHAR* _out, int _maxcount, long &_cursor, void* buff);
char				buff_readByte(void* buff, long &_cursor);
unsigned char		buff_readUByte(void* buff, long &_cursor);
short				buff_readShort(void* buff, long &_cursor);
unsigned short		buff_readUShort(void* buff, long &_cursor);
unsigned int		buff_readUInt(void* buff, long &_cursor);

// Formats time in human readable form
void				OS_FormatTime(WCHAR* dest, int destSize, float timeInSecs);

void DrawRectUP_TL1T(LPDIRECT3DDEVICE9 pDevice, RECT scrRect, Vec2 texUL, Vec2 texDR, DWORD color = 0xffffffff);
void DrawLineUP_TL1T(LPDIRECT3DDEVICE9 pDevice, Vec2 start, Vec2 end, DWORD color = 0xffffffff);
void DrawFullscreenVignette(LPDIRECT3DDEVICE9 pDevice, float alpha);

//gets time by spline (0..1)
float TimeEasing(float t);
float EasingOutBackCubic(float t);
// Sets clip area on renderer (so you can't paint outside)
HRESULT SetScissorClip(LPDIRECT3DDEVICE9 pDevice, int clipX, int clipY, int clipW, int clipH);
// Removes clip from renderer
HRESULT RemoveScissorClip(LPDIRECT3DDEVICE9 pDevice);
// Splits string into tokens
std::vector<std::wstring> TokenizeString(const std::wstring& str, const std::wstring& delim);
// Returns true if str contains ANY token from strTokens (separated by tokensSeparator)
bool StringContainsAnyToken(const std::wstring& str, const std::wstring& strTokens, const std::wstring& tokensSeparator);
// Returns true if str contains ALL tokens from strTokens (separated by tokensSeparator)
bool StringContainsAllTokens(const std::wstring& str, const std::wstring& strTokens, const std::wstring& tokensSeparator);
// Splits version string into major, minor, patch. Expects "1.6.12"
bool GetVersionFromString(WCHAR * inStr, int & outMajor, int & outMinor, int & outPatch);

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
void str_replace(char * o_string, char * s_string, char * r_string);
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
void wcs_replace(WCHAR* o_string, WCHAR* s_string, WCHAR* r_string);

UINT32 GenerateUID(void); //generates a UID based on timestamp and 3 random floats

enum EVarTypes {
	K_RETTYPE_EMPTY = -1,
	K_RETTYPE_INT = 0,
	K_RETTYPE_FLOAT = 1,
	K_RETTYPE_STRING = 2,
	K_RETTYPE_HEXCOLOR = 3,
};
//RETURNS: type specified by *str: int, float or string
EVarTypes GetTypeFromString(const WCHAR *str);

// Class that holds multiple types of values
class CVariantComplex 
{
public:
	enum VariantType 
	{
		K_ARGTYPE_NONE = 0,	// default, not set

		K_ARGTYPE_INT32,
		K_ARGTYPE_FLOAT,
		K_ARGTYPE_BOOL,
		K_ARGTYPE_UINT32,
		K_ARGTYPE_HEXCOLOR,	//DWORD
		K_ARGTYPE_VOIDP,
		// special
		K_ARGTYPE_STRING,
	};
	VariantType		eType; // arg type
	CStringHash		shName; // arg name 

	union 
	{
		INT32	m_asINT32;
		FLOAT	m_asFloat;
		bool	m_asBool;
		UINT32  m_asUINT32;
		VOID*	m_asVoid;
	};
	CStringHash		m_strArg; //argument string
	// special types if needed:
	//Vec3	m_argVec3; //argument vector, daca va fi nevoie de el

	inline bool IsSet() {
		return eType != K_ARGTYPE_NONE;
	}

	CVariantComplex( const CVariantComplex &o):
		eType(o.eType),
		m_asUINT32(o.m_asUINT32)
	{
		shName.Init(o.shName.text);
		m_strArg.Init(o.m_strArg.text);
	}
	// serializes to file and returns true for success
	bool Serialize(FILE *fl);
	// Deserializes variant from file and returns true for success
	bool Deserialize(FILE* fl);
	// Deserializes from file and returns allocated variant or nullptr
	static CVariantComplex* DeserializeAlloc(FILE *f);

	//constructors
	CVariantComplex():
	eType(K_ARGTYPE_NONE),
	m_asUINT32(0)
	{
		m_strArg.Reset();
	}

	bool operator== (CVariantComplex const & o) const
	{
		if(eType == K_ARGTYPE_STRING)
			return (m_strArg.textHash == o.m_strArg.textHash);
		if(eType == K_ARGTYPE_FLOAT)
			return (m_asFloat == o.m_asFloat);
		//defaults on UINT32 valabil pentru toate celelalte
		return (m_asUINT32 == o.m_asUINT32);
	}

	void CopyValueFrom(CVariantComplex *cv)
	{
		eType = cv->eType;
		if (cv->eType == K_ARGTYPE_STRING)
		{
			m_asUINT32 = 0;
			m_strArg = cv->m_strArg;
		}
		else if (eType == K_ARGTYPE_FLOAT)
		{
			m_asFloat = cv->m_asFloat;
			m_strArg.Reset();
		}
		//defaults on UINT32 which contains all the other ones
		else
		{
			m_asUINT32 = cv->m_asUINT32;
			m_strArg.Reset();
		}
	}

	void Set_INT32(const WCHAR* argName, INT32 int32Val) { shName.Init(argName); m_asINT32 = int32Val; eType = K_ARGTYPE_INT32;}
	void Set_UINT32(const WCHAR* argName, UINT32 uint32Val) { shName.Init(argName); m_asUINT32 = uint32Val; eType = K_ARGTYPE_UINT32;}
	void Set_HEXCOLOR(const WCHAR* argName, UINT32 uint32Val) { shName.Init(argName); m_asUINT32 = uint32Val; eType = K_ARGTYPE_HEXCOLOR; }
	void Set_BOOL(const WCHAR* argName, bool boolVal) { shName.Init(argName); m_asBool = boolVal; eType = K_ARGTYPE_BOOL;}
	void Set_FLOAT(const WCHAR* argName, float floatVal) { shName.Init(argName); m_asFloat = floatVal; eType = K_ARGTYPE_FLOAT;}
	void Set_STRING(const WCHAR* argName, WCHAR* strVal) { shName.Init(argName); m_strArg.Init(strVal); m_asUINT32 = 0.0f; eType = K_ARGTYPE_STRING;}
	void Set_STRING(const WCHAR* argName, CHAR* strVal) { shName.Init(argName); m_strArg.Init(strVal); m_asUINT32 = 0.0f; eType = K_ARGTYPE_STRING; }
	void Set_VOIDP(const WCHAR* argName, void* voidP) { shName.Init(argName); m_asVoid = voidP; eType = K_ARGTYPE_VOIDP; }

	void Set_AUTO(const WCHAR* argName, WCHAR* strVal)
	{
		int rettype = GetTypeFromString(strVal);
		switch (rettype)
		{
			case K_RETTYPE_INT:
			{
				WCHAR *stopstr;
				INT32 val = (INT32)wcstol(strVal, &stopstr, 10);
				Set_INT32(argName, val);
			}
			break;
			case K_RETTYPE_HEXCOLOR:
			{
				UINT32 val = 0x0;
				WCHAR* p = strVal;
				while (*p == '#' || *p == ' ' || *p == '\t')
					p++;
				swscanf_s(p, L"%08X", &val);

				Set_HEXCOLOR(argName, val);
			}
			break;
			case K_RETTYPE_FLOAT:
			{
				WCHAR *stopstr;
				float val = (float)wcstod(strVal, &stopstr);
				Set_FLOAT(argName, val);
			}
			break;
			default:
			case K_RETTYPE_EMPTY:
			case K_RETTYPE_STRING:
			{
				Set_STRING(argName, strVal);
			}
			break;
		}
	}
	//--- functii conversie ---

	int asString(WCHAR *destStr, int maxLen)
	{
		switch (eType)
		{
			case K_ARGTYPE_STRING:
				StringCchCopy(destStr, maxLen, m_strArg.text);
				break;
			case K_ARGTYPE_INT32:
				StringCchPrintf(destStr, maxLen, L"%d", m_asINT32);
				break;
			case K_ARGTYPE_FLOAT:
				StringCchPrintf(destStr, maxLen, L"%.2f", m_asFloat);
				break;
			default:
			case K_ARGTYPE_UINT32:
				StringCchPrintf(destStr, maxLen, L"%d", m_asUINT32);
				break;
			case K_ARGTYPE_HEXCOLOR:
				StringCchPrintf(destStr, maxLen, L"#%08X", m_asUINT32);
				break;
			case K_ARGTYPE_BOOL:
				if(m_asBool)
					StringCchPrintf(destStr, maxLen, L"true");
				else
					StringCchPrintf(destStr, maxLen, L"false");
				break;
		}
		return 0;
	}

	int asString(CHAR *destStr, int maxLen)
	{
		switch (eType)
		{
			case K_ARGTYPE_STRING:
				wcstombs(destStr, m_strArg.text, maxLen);
				break;
			case K_ARGTYPE_INT32:
				sprintf(destStr, "%d", m_asINT32);
				break;
			case K_ARGTYPE_FLOAT:
				sprintf(destStr, "%.2f", m_asFloat);
				break;
			default:
			case K_ARGTYPE_UINT32:
				sprintf(destStr, "%d", m_asUINT32);
				break;
			case K_ARGTYPE_HEXCOLOR:
				sprintf(destStr, "#%08X", m_asUINT32);
				break;
			case K_ARGTYPE_BOOL:
				if (m_asBool)
					sprintf(destStr, "true");
				else
					sprintf(destStr, "false");
				break;
		}
		return 0;
	}


	INT32 asInt32() 
	{
		switch (eType)
		{
			case K_ARGTYPE_STRING:
				return _wtoi(m_strArg.text);
			case K_ARGTYPE_FLOAT:
				return (int)m_asFloat;
			default:
				return m_asINT32;
		}
	};

	float asFloat() {
		switch (eType)
		{
			case K_ARGTYPE_STRING:
				return _wtof(m_strArg.text);
			case K_ARGTYPE_FLOAT:
				return (float)m_asFloat;
			case K_ARGTYPE_HEXCOLOR:
			case K_ARGTYPE_UINT32:
				return (float)m_asUINT32;
			default:
				return (float)m_asINT32;
		}
	};
};

///--- TIMERS CLASS ---
class CTimersArray {
private:
	float fMinPeriod;
	int nMinPeriod_ms;

	int timersCnt;
	float* timers;
	bool* timerTicks;
public:
	CTimersArray(int maxPeriod_ms = 2000, int minPeriod_ms = 10);
	~CTimersArray();

	void Update(float dTime);
	
	//intoarce true pe un singur frame, atunci cand a trecut timpul respectiv
	inline bool Tick(int period_ms) { return (true == timerTicks[period_ms / nMinPeriod_ms]); }
	//intoarce cat timp s-a scurs in timer, pana la limita lui de period_ms
	inline float GetTimerValue(int period_ms) {return timers[period_ms / nMinPeriod_ms];};
	//resets all timers
	void ResetTimers();
};


//enum/name list index finder
int GetListIndexByName(const WCHAR* strName, const CStringHash *arrNamesList, int arrNamesListSize);
int GetListIndexByNameHash(const UINT32 nameHash, const CStringHash *arrNamesList, int arrNamesListSize);

///--- ADDITIVE BLENDING ---
inline void DeviceAdditiveON(PDEVICE pDevice)
{
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
}
inline void DeviceAdditiveOFF(PDEVICE pDevice)
{
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}

unsigned int			OS_GetTimeMS();

FORCEINLINE	FILE*		OS_fopen(const CHAR* szPath, const CHAR* szMode) { return fopen(szPath, szMode); }
FORCEINLINE FILE*		OS_wfopen(const WCHAR* wszPath, const WCHAR* wszMode) { return _wfopen(wszPath, wszMode); }
FORCEINLINE int			OS_wfopen_s(FILE** fl, const WCHAR* wszPath, const WCHAR* wszMode) { return _wfopen_s(fl, wszPath, wszMode); }
FORCEINLINE int			OS_fclose(FILE * fl) { return fclose(fl); }
FORCEINLINE size_t		OS_fread(void * _DstBuf, size_t _ElementSize, size_t _Count, FILE * _File) {
	return fread(_DstBuf, _ElementSize, _Count, _File);
}
FORCEINLINE size_t		OS_fwrite(const void * _Str, size_t _Size, size_t _Count, FILE * _File) {
	return fwrite(_Str, _Size, _Count, _File);
}

///--- FILE FUNCTIONS ---
// Reads a file and returns number of bytes read in nRetSize
char*					OS_readFileToBuffer(const WCHAR* wcsPath, int &nRetSize);
unsigned char*			OS_readFileToBufferUC(const WCHAR* wcsPath, int &nRetSize);
// Writes the path without extension in destPath and returns the length of the string 
int						OS_GetFileNameWithoutExtension(WCHAR * destBuffer, int destSize, WCHAR * srcPath);
// Writes the extension in destStr (without the dot) and returns the length of the string or 0 on error or no dot found
int						OS_GetFileNameExtension(WCHAR * destStr, int destSize, WCHAR * srcPath);

long					OS_GetFileSize(WCHAR *path);
bool					OS_CreateFolder(const char* szPath);
bool					OS_DeleteFolder(const char* szPath); // folder must be empty and you must have permission to delete it. For recursively deleting a folder hierarchy, use FileManager::DeleteFolder()
void					OS_GetFolderFiles(const char *directory, const char *extension, List<char*>& list, bool bFullPath); // Use "/" as the extension to get folders instead of files
// Copies a folder recursively - only windows (not needed elsewere)
bool					OS_CopyRecursive(WCHAR r_szSrcPath[1024], WCHAR r_szDesPath[1024]);
// Deletes a folder and contents
bool					OS_DeleteRecursive(WCHAR r_szPath[1024]);
// Copies already opened files
void					OS_FileCopy(FILE *dest, FILE *src);


///--- String utils ---

// Converts WCHAR text to UTF8 (platform specific, used for paths)
int						WCHARtoUTF8(char* dest, WCHAR* src, int maxSize);
// Converts UTF8 text to WCHAR (platform specific, used for paths)
int						UTF8toWCHAR(WCHAR* dest, char* src, int maxSize);


std::wstring RemoveQuotationMarks(const std::wstring& initialString);
std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems);

/*
// Use it to get charcater by character from utf8 encoded string:
// usage example:
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
int utf8_next(const char**  pcursor, const char*   end);
