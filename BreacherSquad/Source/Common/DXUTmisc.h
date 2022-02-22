//**************************************************************************************
// File: DXUTMisc.h
//
// Helper functions for Direct3D programming.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//**************************************************************************************
#pragma once
#ifndef DXUT_MISC_H
#define DXUT_MISC_H


//**************************************************************************************
// A growable array
//**************************************************************************************
template<typename TYPE> class CGrowableArray
{
public:
	CGrowableArray()
	{
		m_pData = NULL; m_nSize = 0; m_nMaxSize = 0;
	}
	CGrowableArray(const CGrowableArray <TYPE>& a)
	{
		for (int i = 0; i < a.m_nSize; i++) Add(a.m_pData[i]);
	}
	~CGrowableArray()
	{
		RemoveAll();
	}

	const TYPE& operator[](int nIndex) const
	{
		return GetAt(nIndex);
	}
	TYPE& operator[](int nIndex)
	{
		return GetAt(nIndex);
	}

	CGrowableArray& operator=(const CGrowableArray <TYPE>& a)
	{
		if (this == &a)
			return *this;
		RemoveAll();
		for (int i = 0; i < a.m_nSize; i++)
			Add(a.m_pData[i]);
		return *this;
	}

	HRESULT SetSize(int nNewMaxSize);
	HRESULT Add(const TYPE& value);
	HRESULT Insert(int nIndex, const TYPE& value);
	HRESULT SetAt(int nIndex, const TYPE& value);
	TYPE& GetAt(int nIndex) const
	{
		assert(nIndex >= 0 && nIndex < m_nSize); return m_pData[nIndex];
	}
	int     GetSize() const
	{
		return m_nSize;
	}
	int     Count() const
	{
		return m_nSize;
	}
	TYPE* GetData()
	{
		return m_pData;
	}
	bool    Contains(const TYPE& value)
	{
		return (-1 != IndexOf(value));
	}

	int     IndexOf(const TYPE& value)
	{
		return (m_nSize > 0) ? IndexOf(value, 0, m_nSize) : -1;
	}
	int     IndexOf(const TYPE& value, int iStart)
	{
		return IndexOf(value, iStart, m_nSize - iStart);
	}
	int     IndexOf(const TYPE& value, int nIndex, int nNumElements);

	int     LastIndexOf(const TYPE& value)
	{
		return (m_nSize > 0) ? LastIndexOf(value, m_nSize - 1, m_nSize) : -1;
	}
	int     LastIndexOf(const TYPE& value, int nIndex)
	{
		return LastIndexOf(value, nIndex, nIndex + 1);
	}
	int     LastIndexOf(const TYPE& value, int nIndex, int nNumElements);

	HRESULT Remove(int nIndex);
	void    RemoveAll()
	{
		SetSize(0);
	}
	void    Reset()
	{
		m_nSize = 0;
	}

protected:
	TYPE* m_pData;      // the actual array of data
	int m_nSize;        // # of elements (upperBound - 1)
	int m_nMaxSize;     // max allocated

	HRESULT SetSizeInternal(int nNewMaxSize);  // This version doesn't call ctor or dtor.
};


//**************************************************************************************
// Performs timer operations
// Use DXUTGetGlobalTimer() to get the global instance
//**************************************************************************************
class CDXUTTimer
{
public:
	CDXUTTimer();

	void Reset(); // resets the timer
	void Start(); // starts the timer
	void Stop();  // stop (or pause) the timer
	void Advance(); // advance the timer by 0.1 seconds
	double GetAbsoluteTime(); // get the absolute system time
	double GetTime(); // get the current time
	double GetElapsedTime(); // get the time that elapsed between GetElapsedTime() calls
	bool IsStopped(); // returns true if timer stopped

protected:
	bool m_bUsingQPF;
	bool m_bTimerStopped;
	LONGLONG m_llQPFTicksPerSec;

	LONGLONG m_llStopTime;
	LONGLONG m_llLastElapsedTime;
	LONGLONG m_llBaseTime;
};

CDXUTTimer* DXUTGetGlobalTimer();


//-----------------------------------------------------------------------------
// Resource cache for textures, fonts, meshs, and effects.  
// Use DXUTGetGlobalResourceCache() to access the global cache
//-----------------------------------------------------------------------------

enum DXUTCACHE_SOURCELOCATION { DXUTCACHE_LOCATION_FILE, DXUTCACHE_LOCATION_RESOURCE };

struct DXUTCache_Texture
{
	DXUTCACHE_SOURCELOCATION Location;
	WCHAR wszSource[MAX_PATH];
	HMODULE hSrcModule;
	UINT Width;
	UINT Height;
	UINT Depth;
	UINT MipLevels;
	DWORD Usage;
	D3DFORMAT Format;
	D3DPOOL Pool;
	D3DRESOURCETYPE Type;
	IDirect3DBaseTexture9 *pTexture;
};

struct DXUTCache_Font : public D3DXFONT_DESC
{
	ID3DXFont *pFont;
};

struct DXUTCache_Effect
{
	DXUTCACHE_SOURCELOCATION Location;
	WCHAR wszSource[MAX_PATH];
	HMODULE hSrcModule;
	DWORD dwFlags;
	ID3DXEffect *pEffect;
};


class CDXUTResourceCache
{
public:
	~CDXUTResourceCache();

	HRESULT CreateTextureFromFile(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, LPDIRECT3DTEXTURE9 *ppTexture);
	HRESULT CreateTextureFromFileEx(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DTEXTURE9 *ppTexture);
	HRESULT CreateTextureFromResource(LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, LPDIRECT3DTEXTURE9 *ppTexture);
	HRESULT CreateTextureFromResourceEx(LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DTEXTURE9 *ppTexture);
	HRESULT CreateCubeTextureFromFile(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, LPDIRECT3DCUBETEXTURE9 *ppCubeTexture);
	HRESULT CreateCubeTextureFromFileEx(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, UINT Size, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DCUBETEXTURE9 *ppCubeTexture);
	HRESULT CreateCubeTextureFromResource(LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, LPDIRECT3DCUBETEXTURE9 *ppCubeTexture);
	HRESULT CreateCubeTextureFromResourceEx(LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, UINT Size, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DCUBETEXTURE9 *ppCubeTexture);
	HRESULT CreateVolumeTextureFromFile(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, LPDIRECT3DVOLUMETEXTURE9 *ppVolumeTexture);
	HRESULT CreateVolumeTextureFromFileEx(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, UINT Width, UINT Height, UINT Depth, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DVOLUMETEXTURE9 *ppTexture);
	HRESULT CreateVolumeTextureFromResource(LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, LPDIRECT3DVOLUMETEXTURE9 *ppVolumeTexture);
	HRESULT CreateVolumeTextureFromResourceEx(LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, UINT Width, UINT Height, UINT Depth, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DVOLUMETEXTURE9 *ppVolumeTexture);
	HRESULT CreateFont(LPDIRECT3DDEVICE9 pDevice, UINT Height, UINT Width, UINT Weight, UINT MipLevels, BOOL Italic, DWORD CharSet, DWORD OutputPrecision, DWORD Quality, DWORD PitchAndFamily, LPCTSTR pFacename, LPD3DXFONT *ppFont);
	HRESULT CreateFontIndirect(LPDIRECT3DDEVICE9 pDevice, CONST D3DXFONT_DESC *pDesc, LPD3DXFONT *ppFont);
	HRESULT CreateEffectFromFile(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, const D3DXMACRO *pDefines, LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXEFFECTPOOL pPool, LPD3DXEFFECT *ppEffect, LPD3DXBUFFER *ppCompilationErrors);
	HRESULT CreateEffectFromResource(LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, const D3DXMACRO *pDefines, LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXEFFECTPOOL pPool, LPD3DXEFFECT *ppEffect, LPD3DXBUFFER *ppCompilationErrors);

public:
	HRESULT OnCreateDevice(IDirect3DDevice9 *pd3dDevice);
	HRESULT OnResetDevice(IDirect3DDevice9 *pd3dDevice);
	HRESULT OnLostDevice();
	HRESULT OnDestroyDevice();

protected:
	friend CDXUTResourceCache& DXUTGetGlobalResourceCache();
	friend HRESULT DXUTInitialize3DEnvironment();
	friend HRESULT DXUTReset3DEnvironment();
	friend void DXUTCleanup3DEnvironment(bool bReleaseSettings);

	CDXUTResourceCache() { }

	CGrowableArray< DXUTCache_Texture > m_TextureCache;
	CGrowableArray< DXUTCache_Effect > m_EffectCache;
	CGrowableArray< DXUTCache_Font > m_FontCache;
};

CDXUTResourceCache& DXUTGetGlobalResourceCache();



#define KEY_WAS_DOWN_MASK 0x80
#define KEY_IS_DOWN_MASK  0x01

#define MOUSE_LEFT_BUTTON   0x01
#define MOUSE_MIDDLE_BUTTON 0x02
#define MOUSE_RIGHT_BUTTON  0x04
#define MOUSE_WHEEL         0x08


//**************************************************************************************
// Manages the intertion point when drawing text
//**************************************************************************************
class CDXUTTextHelper
{
public:
	CDXUTTextHelper(ID3DXFont* pFont, ID3DXSprite* pSprite, int nLineHeight);

	void SetInsertionPos(int x, int y) { m_pt.x = x; m_pt.y = y; }
	void SetForegroundColor(D3DXCOLOR clr) { m_clr = clr; }

	void Begin();
	HRESULT DrawFormattedTextLine(const WCHAR* strMsg, ...);
	HRESULT DrawTextLine(const WCHAR* strMsg);
	HRESULT DrawFormattedTextLine(RECT &rc, DWORD dwFlags, const WCHAR* strMsg, ...);
	HRESULT DrawTextLine(RECT &rc, DWORD dwFlags, const WCHAR* strMsg);
	void End();

protected:
	ID3DXFont*   m_pFont;
	ID3DXSprite* m_pSprite;
	D3DXCOLOR    m_clr;
	POINT        m_pt;
	int          m_nLineHeight;
};


//**************************************************************************************
// Manages a persistent list of lines and draws them using ID3DXLine
//**************************************************************************************
class CDXUTLineManager
{
public:
	CDXUTLineManager();
	~CDXUTLineManager();

	HRESULT OnCreatedDevice(IDirect3DDevice9* pd3dDevice);
	HRESULT OnResetDevice();
	HRESULT OnRender();
	HRESULT OnLostDevice();
	HRESULT OnDeletedDevice();

	HRESULT AddLine(int* pnLineID, D3DXVECTOR2* pVertexList, DWORD dwVertexListCount, D3DCOLOR Color, float fWidth, float fScaleRatio, bool bAntiAlias);
	HRESULT AddRect(int* pnLineID, RECT rc, D3DCOLOR Color, float fWidth, float fScaleRatio, bool bAntiAlias);
	HRESULT RemoveLine(int nLineID);
	HRESULT RemoveAllLines();

protected:
	struct LINE_NODE
	{
		int      nLineID;
		D3DCOLOR Color;
		float    fWidth;
		bool     bAntiAlias;
		float    fScaleRatio;
		D3DXVECTOR2* pVertexList;
		DWORD    dwVertexListCount;
	};

	CGrowableArray<LINE_NODE*> m_LinesList;
	IDirect3DDevice9* m_pd3dDevice;
	ID3DXLine* m_pD3DXLine;
};



//**************************************************************************************
// Returns the string for the given D3DFORMAT.
//       bWithPrefix determines whether the string should include the "D3DFMT_"
//**************************************************************************************
LPCWSTR DXUTD3DFormatToString(D3DFORMAT format, bool bWithPrefix);


//**************************************************************************************
// Builds and sets a cursor for the D3D device based on hCursor.
//**************************************************************************************
HRESULT DXUTSetDeviceCursor(IDirect3DDevice9* pd3dDevice, HCURSOR hCursor, bool bAddWatermark);


//**************************************************************************************
// Returns a view matrix for rendering to a face of a cubemap.
//**************************************************************************************
D3DXMATRIX DXUTGetCubeMapViewMatrix(DWORD dwFace);


//**************************************************************************************
// Debug printing support
// See dxerr9.h for more debug printing support
//**************************************************************************************
void DXUTOutputDebugStringW(LPCWSTR strMsg, ...);
void DXUTOutputDebugStringA(LPCSTR strMsg, ...);

#ifdef UNICODE
#define DXUTOutputDebugString DXUTOutputDebugStringW
#else
#define DXUTOutputDebugString DXUTOutputDebugStringA
#endif

HRESULT DXUTErrMsg(WCHAR * str, HRESULT hr);

#if defined(DEBUG) | defined(_DEBUG)
#define DXUT_ERR(str,hr)           DXUTErrMsg(str, hr);
#define DXUT_ERR_MSGBOX(str,hr)    DXUTErrMsg(str, hr);
#define DXUTTRACE                  DXUTOutputDebugString
#else
#define DXUT_ERR(str,hr)           (hr)
#define DXUT_ERR_MSGBOX(str,hr)    (hr)
#define DXUTTRACE                  (__noop)
#endif


//**************************************************************************************
// Direct3D9 dynamic linking support -- calls top-level D3D9 APIs with graceful
// failure if APIs are not present.
//**************************************************************************************

IDirect3D9 * WINAPI DXUT_Dynamic_Direct3DCreate9(UINT SDKVersion);
int WINAPI DXUT_Dynamic_D3DPERF_BeginEvent(D3DCOLOR col, LPCWSTR wszName);
int WINAPI DXUT_Dynamic_D3DPERF_EndEvent(void);
void WINAPI DXUT_Dynamic_D3DPERF_SetMarker(D3DCOLOR col, LPCWSTR wszName);
void WINAPI DXUT_Dynamic_D3DPERF_SetRegion(D3DCOLOR col, LPCWSTR wszName);
BOOL WINAPI DXUT_Dynamic_D3DPERF_QueryRepeatFrame(void);
void WINAPI DXUT_Dynamic_D3DPERF_SetOptions(DWORD dwOptions);
DWORD WINAPI DXUT_Dynamic_D3DPERF_GetStatus(void);


//**************************************************************************************
// Profiling/instrumentation support
//**************************************************************************************

//**************************************************************************************
// Some D3DPERF APIs take a color that can be used when displaying user events in 
// performance analysis tools.  The following constants are provided for your 
// convenience, but you can use any colors you like.
//**************************************************************************************
const D3DCOLOR DXUT_PERFEVENTCOLOR  = D3DCOLOR_XRGB(200,100,100);
const D3DCOLOR DXUT_PERFEVENTCOLOR2 = D3DCOLOR_XRGB(100,200,100);
const D3DCOLOR DXUT_PERFEVENTCOLOR3 = D3DCOLOR_XRGB(100,100,200);

//**************************************************************************************
// The following macros provide a convenient way for your code to call the D3DPERF 
// functions only when PROFILE is defined.  If PROFILE is not defined (as for the final 
// release version of a program), these macros evaluate to nothing, so no detailed event
// information is embedded in your shipping program.  It is recommended that you create
// and use three build configurations for your projects:
//     Debug (nonoptimized code, asserts active, PROFILE defined to assist debugging)
//     Profile (optimized code, asserts disabled, PROFILE defined to assist optimization)
//     Release (optimized code, asserts disabled, PROFILE not defined)
//**************************************************************************************
#ifdef PROFILE
// PROFILE is defined, so these macros call the D3DPERF functions
#define DXUT_BeginPerfEvent(color, pstrMessage)   DXUT_Dynamic_D3DPERF_BeginEvent(color, pstrMessage)
#define DXUT_EndPerfEvent()                         DXUT_Dynamic_D3DPERF_EndEvent()
#define DXUT_SetPerfMarker(color, pstrMessage)    DXUT_Dynamic_D3DPERF_SetMarker(color, pstrMessage)
#else
// PROFILE is not defined, so these macros do nothing
#define DXUT_BeginPerfEvent(color, pstrMessage)   (__noop)
#define DXUT_EndPerfEvent()                         (__noop)
#define DXUT_SetPerfMarker(color, pstrMessage)    (__noop)
#endif

//**************************************************************************************
// CDXUTPerfEventGenerator is a helper class that makes it easy to attach begin and end
// events to a block of code.  Simply define a CDXUTPerfEventGenerator variable anywhere 
// in a block of code, and the class's constructor will call DXUT_BeginPerfEvent when 
// the block of code begins, and the class's destructor will call DXUT_EndPerfEvent when 
// the block ends.
//**************************************************************************************
class CDXUTPerfEventGenerator
{
public:
	CDXUTPerfEventGenerator(D3DCOLOR color, WCHAR* pstrMessage) { DXUT_BeginPerfEvent(color, pstrMessage); }
	~CDXUTPerfEventGenerator(void) { DXUT_EndPerfEvent(); }
};

//**************************************************************************************
// Implementation of CGrowableArray
//**************************************************************************************

// This version doesn't call ctor or dtor.
template< typename TYPE >
HRESULT CGrowableArray<TYPE>::SetSizeInternal(int nNewMaxSize)
{
	if (nNewMaxSize < 0)
	{
		assert(false);
		return E_INVALIDARG;
	}

	if (nNewMaxSize == 0)
	{
		// Shrink to 0 size & cleanup
		if (m_pData)
		{
			free(m_pData);
			m_pData = NULL;
		}

		m_nMaxSize = 0;
		m_nSize = 0;
	}
	else if (m_pData == NULL || nNewMaxSize > m_nMaxSize)
	{
		// Grow array
		int nGrowBy = (m_nMaxSize == 0) ? 16 : m_nMaxSize;
		nNewMaxSize = max(nNewMaxSize, m_nMaxSize + nGrowBy);

		TYPE* pDataNew = (TYPE*) realloc(m_pData, nNewMaxSize * sizeof(TYPE));
		if (pDataNew == NULL)
			return E_OUTOFMEMORY;

		m_pData = pDataNew;
		m_nMaxSize = nNewMaxSize;
	}

	return S_OK;
}


//**************************************************************************************
template< typename TYPE >
HRESULT CGrowableArray<TYPE>::SetSize(int nNewMaxSize)
{
	int nOldSize = m_nSize;

	if (nOldSize > nNewMaxSize)
	{
		// Removing elements. Call dtor.

		for(int i = nNewMaxSize; i < nOldSize; ++i)
			m_pData[i].~TYPE();
	}

	// Adjust buffer.  Note that there's no need to check for error
	// since if it happens, nOldSize == nNewMaxSize will be true.)
	HRESULT hr = SetSizeInternal(nNewMaxSize);

	if (nOldSize < nNewMaxSize)
	{
		// Adding elements. Call ctor.

		for(int i = nOldSize; i < nNewMaxSize; ++i)
			::new (&m_pData[i]) TYPE;
	}

	return hr;
}


//**************************************************************************************
template< typename TYPE >
HRESULT CGrowableArray<TYPE>::Add(const TYPE& value)
{
	HRESULT hr;
	if (FAILED(hr = SetSizeInternal(m_nSize + 1)))
		return hr;

	// Construct the new element
	::new (&m_pData[m_nSize]) TYPE;

	// Assign
	m_pData[m_nSize] = value;
	++m_nSize;

	return S_OK;
}


//**************************************************************************************
template< typename TYPE >
HRESULT CGrowableArray<TYPE>::Insert(int nIndex, const TYPE& value)
{
	HRESULT hr;

	// Validate index
	if (nIndex < 0 || 
		nIndex > m_nSize)
	{
		assert(false);
		return E_INVALIDARG;
	}

	// Prepare the buffer
	if (FAILED(hr = SetSizeInternal(m_nSize + 1)))
		return hr;

	// Shift the array
	MoveMemory(&m_pData[nIndex+1], &m_pData[nIndex], sizeof(TYPE) * (m_nSize - nIndex));

	// Construct the new element
	::new (&m_pData[nIndex]) TYPE;

	// Set the value and increase the size
	m_pData[nIndex] = value;
	++m_nSize;

	return S_OK;
}


//**************************************************************************************
template< typename TYPE >
HRESULT CGrowableArray<TYPE>::SetAt(int nIndex, const TYPE& value)
{
	// Validate arguments
	if (nIndex < 0 ||
		nIndex >= m_nSize)
	{
		assert(false);
		return E_INVALIDARG;
	}

	m_pData[nIndex] = value;
	return S_OK;
}


//**************************************************************************************
// Searches for the specified value and returns the index of the first occurrence
// within the section of the data array that extends from iStart and contains the 
// specified number of elements. Returns -1 if value is not found within the given 
// section.
//**************************************************************************************
template< typename TYPE >
int CGrowableArray<TYPE>::IndexOf(const TYPE& value, int iStart, int nNumElements)
{
	// Validate arguments
	if (iStart < 0 || 
		iStart >= m_nSize ||
		nNumElements < 0 ||
		iStart + nNumElements > m_nSize)
	{
		assert(false);
		return -1;
	}

	// Search
	for(int i = iStart; i < (iStart + nNumElements); i++)
	{
		if (value == m_pData[i])
			return i;
	}

	// Not found
	return -1;
}


//**************************************************************************************
// Searches for the specified value and returns the index of the last occurrence
// within the section of the data array that contains the specified number of elements
// and ends at iEnd. Returns -1 if value is not found within the given section.
//**************************************************************************************
template< typename TYPE >
int CGrowableArray<TYPE>::LastIndexOf(const TYPE& value, int iEnd, int nNumElements)
{
	// Validate arguments
	if (iEnd < 0 || 
		iEnd >= m_nSize ||
		nNumElements < 0 ||
		iEnd - nNumElements < 0)
	{
		assert(false);
		return -1;
	}

	// Search
	for(int i = iEnd; i > (iEnd - nNumElements); i--)
	{
		if (value == m_pData[i])
			return i;
	}

	// Not found
	return -1;
}



//**************************************************************************************
template< typename TYPE >
HRESULT CGrowableArray<TYPE>::Remove(int nIndex)
{
	if (nIndex < 0 || 
		nIndex >= m_nSize)
	{
		assert(false);
		return E_INVALIDARG;
	}

	// Destruct the element to be removed
	m_pData[nIndex].~TYPE();

	// Compact the array and decrease the size
	MoveMemory(&m_pData[nIndex], &m_pData[nIndex+1], sizeof(TYPE) * (m_nSize - (nIndex+1)));
	--m_nSize;

	return S_OK;
}


#endif
