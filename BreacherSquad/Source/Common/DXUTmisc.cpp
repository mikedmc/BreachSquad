//--------------------------------------------------------------------------------------
// File: DXUTMisc.cpp
//
// Shortcut macros and functions for using DX objects
//
// Copyright (c) Microsoft Corporation. All rights reserved
//--------------------------------------------------------------------------------------
#include "dxstdafx.h"

//--------------------------------------------------------------------------------------
// Global/Static Members
//--------------------------------------------------------------------------------------
CDXUTResourceCache& DXUTGetGlobalResourceCache()
{
	// Using an accessor function gives control of the construction order
	static CDXUTResourceCache cache;
	return cache;
}
CDXUTTimer* DXUTGetGlobalTimer()
{
	// Using an accessor function gives control of the construction order
	static CDXUTTimer timer;
	return &timer;
}

//--------------------------------------------------------------------------------------
CDXUTTimer::CDXUTTimer()
{
	m_bUsingQPF         = false;
	m_bTimerStopped     = true;
	m_llQPFTicksPerSec  = 0;

	m_llStopTime        = 0;
	m_llLastElapsedTime = 0;
	m_llBaseTime        = 0;

	// Use QueryPerformanceFrequency() to get frequency of timer.  
	LARGE_INTEGER qwTicksPerSec;
	m_bUsingQPF = (bool) (QueryPerformanceFrequency(&qwTicksPerSec) != 0);
	m_llQPFTicksPerSec = qwTicksPerSec.QuadPart;
}


//--------------------------------------------------------------------------------------
void CDXUTTimer::Reset()
{
	if (!m_bUsingQPF)
		return;

	// Get either the current time or the stop time
	LARGE_INTEGER qwTime;
	if (m_llStopTime != 0)
		qwTime.QuadPart = m_llStopTime;
	else
		QueryPerformanceCounter(&qwTime);

	m_llBaseTime        = qwTime.QuadPart;
	m_llLastElapsedTime = qwTime.QuadPart;
	m_llStopTime        = 0;
	m_bTimerStopped     = FALSE;
}


//--------------------------------------------------------------------------------------
void CDXUTTimer::Start()
{
	if (!m_bUsingQPF)
		return;

	// Get the current time
	LARGE_INTEGER qwTime;
	QueryPerformanceCounter(&qwTime);

	if (m_bTimerStopped)
		m_llBaseTime += qwTime.QuadPart - m_llStopTime;
	m_llStopTime = 0;
	m_llLastElapsedTime = qwTime.QuadPart;
	m_bTimerStopped = FALSE;
}


//--------------------------------------------------------------------------------------
void CDXUTTimer::Stop()
{
	if (!m_bUsingQPF)
		return;

	if (!m_bTimerStopped)
	{
		// Get either the current time or the stop time
		LARGE_INTEGER qwTime;
		if (m_llStopTime != 0)
			qwTime.QuadPart = m_llStopTime;
		else
			QueryPerformanceCounter(&qwTime);

		m_llStopTime = qwTime.QuadPart;
		m_llLastElapsedTime = qwTime.QuadPart;
		m_bTimerStopped = TRUE;
	}
}


//--------------------------------------------------------------------------------------
void CDXUTTimer::Advance()
{
	if (!m_bUsingQPF)
		return;

	m_llStopTime += m_llQPFTicksPerSec/10;
}


//--------------------------------------------------------------------------------------
double CDXUTTimer::GetAbsoluteTime()
{
	if (!m_bUsingQPF)
		return -1.0;

	// Get either the current time or the stop time
	LARGE_INTEGER qwTime;
	if (m_llStopTime != 0)
		qwTime.QuadPart = m_llStopTime;
	else
		QueryPerformanceCounter(&qwTime);

	double fTime = qwTime.QuadPart / (double) m_llQPFTicksPerSec;

	return fTime;
}


//--------------------------------------------------------------------------------------
double CDXUTTimer::GetTime()
{
	if (!m_bUsingQPF)
		return -1.0;

	// Get either the current time or the stop time
	LARGE_INTEGER qwTime;
	if (m_llStopTime != 0)
		qwTime.QuadPart = m_llStopTime;
	else
		QueryPerformanceCounter(&qwTime);

	double fAppTime = (double) (qwTime.QuadPart - m_llBaseTime) / (double) m_llQPFTicksPerSec;

	return fAppTime;
}


//--------------------------------------------------------------------------------------
double CDXUTTimer::GetElapsedTime()
{
	if (!m_bUsingQPF)
		return -1.0;

	// Get either the current time or the stop time
	LARGE_INTEGER qwTime;
	if (m_llStopTime != 0)
		qwTime.QuadPart = m_llStopTime;
	else
		QueryPerformanceCounter(&qwTime);

	double fElapsedTime = (double) (qwTime.QuadPart - m_llLastElapsedTime) / (double) m_llQPFTicksPerSec;
	m_llLastElapsedTime = qwTime.QuadPart;

	return fElapsedTime;
}


//--------------------------------------------------------------------------------------
bool CDXUTTimer::IsStopped()
{
	return m_bTimerStopped;
}


//--------------------------------------------------------------------------------------
// CDXUTResourceCache
//--------------------------------------------------------------------------------------


CDXUTResourceCache::~CDXUTResourceCache()
{
	OnDestroyDevice();

	m_TextureCache.RemoveAll();
	m_EffectCache.RemoveAll();
	m_FontCache.RemoveAll();
}


HRESULT CDXUTResourceCache::CreateTextureFromFile(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, LPDIRECT3DTEXTURE9 *ppTexture)
{
	return CreateTextureFromFileEx(pDevice, pSrcFile, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
		0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
		0, NULL, NULL, ppTexture);
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::CreateTextureFromFileEx(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DTEXTURE9 *ppTexture)
{
	// Search the cache for a matching entry.
	for(int i = 0; i < m_TextureCache.GetSize(); ++i)
	{
		DXUTCache_Texture &Entry = m_TextureCache[i];
		if (Entry.Location == DXUTCACHE_LOCATION_FILE &&
			!lstrcmpW(Entry.wszSource, pSrcFile) &&
			Entry.Width == Width &&
			Entry.Height == Height &&
			Entry.MipLevels == MipLevels &&
			Entry.Usage == Usage &&
			Entry.Format == Format &&
			Entry.Pool == Pool &&
			Entry.Type == D3DRTYPE_TEXTURE)
		{
			// A match is found. Obtain the IDirect3DTexture9 interface and return that.
			return Entry.pTexture->QueryInterface(IID_IDirect3DTexture9, (LPVOID*)ppTexture);
		}
	}

	HRESULT hr;

	// No matching entry.  Load the resource and create a new entry.
	hr = D3DXCreateTextureFromFileEx(pDevice, pSrcFile, Width, Height, MipLevels, Usage, Format,
		Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);
	if (FAILED(hr))
		return hr;

	DXUTCache_Texture NewEntry;
	NewEntry.Location = DXUTCACHE_LOCATION_FILE;
	StringCchCopy(NewEntry.wszSource, MAX_PATH, pSrcFile);
	NewEntry.Width = Width;
	NewEntry.Height = Height;
	NewEntry.MipLevels = MipLevels;
	NewEntry.Usage = Usage;
	NewEntry.Format = Format;
	NewEntry.Pool = Pool;
	NewEntry.Type = D3DRTYPE_TEXTURE;
	(*ppTexture)->QueryInterface(IID_IDirect3DBaseTexture9, (LPVOID*)&NewEntry.pTexture);

	m_TextureCache.Add(NewEntry);
	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::CreateTextureFromResource(LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, LPDIRECT3DTEXTURE9 *ppTexture)
{
	return CreateTextureFromResourceEx(pDevice, hSrcModule, pSrcResource, D3DX_DEFAULT, D3DX_DEFAULT,
		D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT,
		D3DX_DEFAULT, 0, NULL, NULL, ppTexture);
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::CreateTextureFromResourceEx(LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, UINT Width, UINT Height, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DTEXTURE9 *ppTexture)
{
	// Search the cache for a matching entry.
	for(int i = 0; i < m_TextureCache.GetSize(); ++i)
	{
		DXUTCache_Texture &Entry = m_TextureCache[i];
		if (Entry.Location == DXUTCACHE_LOCATION_RESOURCE &&
			Entry.hSrcModule == hSrcModule &&
			!lstrcmpW(Entry.wszSource, pSrcResource) &&
			Entry.Width == Width &&
			Entry.Height == Height &&
			Entry.MipLevels == MipLevels &&
			Entry.Usage == Usage &&
			Entry.Format == Format &&
			Entry.Pool == Pool &&
			Entry.Type == D3DRTYPE_TEXTURE)
		{
			// A match is found. Obtain the IDirect3DTexture9 interface and return that.
			return Entry.pTexture->QueryInterface(IID_IDirect3DTexture9, (LPVOID*)ppTexture);
		}
	}

	HRESULT hr;

	// No matching entry.  Load the resource and create a new entry.
	hr = D3DXCreateTextureFromResourceEx(pDevice, hSrcModule, pSrcResource, Width, Height, MipLevels, Usage,
		Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);
	if (FAILED(hr))
		return hr;

	DXUTCache_Texture NewEntry;
	NewEntry.Location = DXUTCACHE_LOCATION_RESOURCE;
	NewEntry.hSrcModule = hSrcModule;
	StringCchCopy(NewEntry.wszSource, MAX_PATH, pSrcResource);
	NewEntry.Width = Width;
	NewEntry.Height = Height;
	NewEntry.MipLevels = MipLevels;
	NewEntry.Usage = Usage;
	NewEntry.Format = Format;
	NewEntry.Pool = Pool;
	NewEntry.Type = D3DRTYPE_TEXTURE;
	(*ppTexture)->QueryInterface(IID_IDirect3DBaseTexture9, (LPVOID*)&NewEntry.pTexture);

	m_TextureCache.Add(NewEntry);
	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::CreateCubeTextureFromFile(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, LPDIRECT3DCUBETEXTURE9 *ppCubeTexture)
{
	return CreateCubeTextureFromFileEx(pDevice, pSrcFile, D3DX_DEFAULT, D3DX_DEFAULT, 0,
		D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
		0, NULL, NULL, ppCubeTexture);
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::CreateCubeTextureFromFileEx(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, UINT Size, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DCUBETEXTURE9 *ppCubeTexture)
{
	// Search the cache for a matching entry.
	for(int i = 0; i < m_TextureCache.GetSize(); ++i)
	{
		DXUTCache_Texture &Entry = m_TextureCache[i];
		if (Entry.Location == DXUTCACHE_LOCATION_FILE &&
			!lstrcmpW(Entry.wszSource, pSrcFile) &&
			Entry.Width == Size &&
			Entry.MipLevels == MipLevels &&
			Entry.Usage == Usage &&
			Entry.Format == Format &&
			Entry.Pool == Pool &&
			Entry.Type == D3DRTYPE_CUBETEXTURE)
		{
			// A match is found. Obtain the IDirect3DCubeTexture9 interface and return that.
			return Entry.pTexture->QueryInterface(IID_IDirect3DCubeTexture9, (LPVOID*)ppCubeTexture);
		}
	}

	HRESULT hr;

	// No matching entry.  Load the resource and create a new entry.
	hr = D3DXCreateCubeTextureFromFileEx(pDevice, pSrcFile, Size, MipLevels, Usage, Format, Pool, Filter,
		MipFilter, ColorKey, pSrcInfo, pPalette, ppCubeTexture);
	if (FAILED(hr))
		return hr;

	DXUTCache_Texture NewEntry;
	NewEntry.Location = DXUTCACHE_LOCATION_FILE;
	StringCchCopy(NewEntry.wszSource, MAX_PATH, pSrcFile);
	NewEntry.Width = Size;
	NewEntry.MipLevels = MipLevels;
	NewEntry.Usage = Usage;
	NewEntry.Format = Format;
	NewEntry.Pool = Pool;
	NewEntry.Type = D3DRTYPE_CUBETEXTURE;
	(*ppCubeTexture)->QueryInterface(IID_IDirect3DBaseTexture9, (LPVOID*)&NewEntry.pTexture);

	m_TextureCache.Add(NewEntry);
	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::CreateCubeTextureFromResource(LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, LPDIRECT3DCUBETEXTURE9 *ppCubeTexture)
{
	return CreateCubeTextureFromResourceEx(pDevice, hSrcModule, pSrcResource, D3DX_DEFAULT, D3DX_DEFAULT,
		0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
		0, NULL, NULL, ppCubeTexture);
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::CreateCubeTextureFromResourceEx(LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, UINT Size, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DCUBETEXTURE9 *ppCubeTexture)
{
	// Search the cache for a matching entry.
	for(int i = 0; i < m_TextureCache.GetSize(); ++i)
	{
		DXUTCache_Texture &Entry = m_TextureCache[i];
		if (Entry.Location == DXUTCACHE_LOCATION_RESOURCE &&
			Entry.hSrcModule == hSrcModule &&
			!lstrcmpW(Entry.wszSource, pSrcResource) &&
			Entry.Width == Size &&
			Entry.MipLevels == MipLevels &&
			Entry.Usage == Usage &&
			Entry.Format == Format &&
			Entry.Pool == Pool &&
			Entry.Type == D3DRTYPE_CUBETEXTURE)
		{
			// A match is found. Obtain the IDirect3DCubeTexture9 interface and return that.
			return Entry.pTexture->QueryInterface(IID_IDirect3DCubeTexture9, (LPVOID*)ppCubeTexture);
		}
	}

	HRESULT hr;

	// No matching entry.  Load the resource and create a new entry.
	hr = D3DXCreateCubeTextureFromResourceEx(pDevice, hSrcModule, pSrcResource, Size, MipLevels, Usage, Format,
		Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppCubeTexture);
	if (FAILED(hr))
		return hr;

	DXUTCache_Texture NewEntry;
	NewEntry.Location = DXUTCACHE_LOCATION_RESOURCE;
	NewEntry.hSrcModule = hSrcModule;
	StringCchCopy(NewEntry.wszSource, MAX_PATH, pSrcResource);
	NewEntry.Width = Size;
	NewEntry.MipLevels = MipLevels;
	NewEntry.Usage = Usage;
	NewEntry.Format = Format;
	NewEntry.Pool = Pool;
	NewEntry.Type = D3DRTYPE_CUBETEXTURE;
	(*ppCubeTexture)->QueryInterface(IID_IDirect3DBaseTexture9, (LPVOID*)&NewEntry.pTexture);

	m_TextureCache.Add(NewEntry);
	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::CreateVolumeTextureFromFile(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, LPDIRECT3DVOLUMETEXTURE9 *ppVolumeTexture)
{
	return CreateVolumeTextureFromFileEx(pDevice, pSrcFile, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT,
		0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
		0, NULL, NULL, ppVolumeTexture);
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::CreateVolumeTextureFromFileEx(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, UINT Width, UINT Height, UINT Depth, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DVOLUMETEXTURE9 *ppTexture)
{
	// Search the cache for a matching entry.
	for(int i = 0; i < m_TextureCache.GetSize(); ++i)
	{
		DXUTCache_Texture &Entry = m_TextureCache[i];
		if (Entry.Location == DXUTCACHE_LOCATION_FILE &&
			!lstrcmpW(Entry.wszSource, pSrcFile) &&
			Entry.Width == Width &&
			Entry.Height == Height &&
			Entry.Depth == Depth &&
			Entry.MipLevels == MipLevels &&
			Entry.Usage == Usage &&
			Entry.Format == Format &&
			Entry.Pool == Pool &&
			Entry.Type == D3DRTYPE_VOLUMETEXTURE)
		{
			// A match is found. Obtain the IDirect3DVolumeTexture9 interface and return that.
			return Entry.pTexture->QueryInterface(IID_IDirect3DVolumeTexture9, (LPVOID*)ppTexture);
		}
	}

	HRESULT hr;

	// No matching entry.  Load the resource and create a new entry.
	hr = D3DXCreateVolumeTextureFromFileEx(pDevice, pSrcFile, Width, Height, Depth, MipLevels, Usage, Format,
		Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppTexture);
	if (FAILED(hr))
		return hr;

	DXUTCache_Texture NewEntry;
	NewEntry.Location = DXUTCACHE_LOCATION_FILE;
	StringCchCopy(NewEntry.wszSource, MAX_PATH, pSrcFile);
	NewEntry.Width = Width;
	NewEntry.Height = Height;
	NewEntry.Depth = Depth;
	NewEntry.MipLevels = MipLevels;
	NewEntry.Usage = Usage;
	NewEntry.Format = Format;
	NewEntry.Pool = Pool;
	NewEntry.Type = D3DRTYPE_VOLUMETEXTURE;
	(*ppTexture)->QueryInterface(IID_IDirect3DBaseTexture9, (LPVOID*)&NewEntry.pTexture);

	m_TextureCache.Add(NewEntry);
	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::CreateVolumeTextureFromResource(LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, LPDIRECT3DVOLUMETEXTURE9 *ppVolumeTexture)
{
	return CreateVolumeTextureFromResourceEx(pDevice, hSrcModule, pSrcResource, D3DX_DEFAULT, D3DX_DEFAULT,
		D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
		D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, ppVolumeTexture);
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::CreateVolumeTextureFromResourceEx(LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, UINT Width, UINT Height, UINT Depth, UINT MipLevels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, DWORD Filter, DWORD MipFilter, D3DCOLOR ColorKey, D3DXIMAGE_INFO *pSrcInfo, PALETTEENTRY *pPalette, LPDIRECT3DVOLUMETEXTURE9 *ppVolumeTexture)
{
	// Search the cache for a matching entry.
	for(int i = 0; i < m_TextureCache.GetSize(); ++i)
	{
		DXUTCache_Texture &Entry = m_TextureCache[i];
		if (Entry.Location == DXUTCACHE_LOCATION_RESOURCE &&
			Entry.hSrcModule == hSrcModule &&
			!lstrcmpW(Entry.wszSource, pSrcResource) &&
			Entry.Width == Width &&
			Entry.Height == Height &&
			Entry.Depth == Depth &&
			Entry.MipLevels == MipLevels &&
			Entry.Usage == Usage &&
			Entry.Format == Format &&
			Entry.Pool == Pool &&
			Entry.Type == D3DRTYPE_VOLUMETEXTURE)
		{
			// A match is found. Obtain the IDirect3DVolumeTexture9 interface and return that.
			return Entry.pTexture->QueryInterface(IID_IDirect3DVolumeTexture9, (LPVOID*)ppVolumeTexture);
		}
	}

	HRESULT hr;

	// No matching entry.  Load the resource and create a new entry.
	hr = D3DXCreateVolumeTextureFromResourceEx(pDevice, hSrcModule, pSrcResource, Width, Height, Depth, MipLevels, Usage,
		Format, Pool, Filter, MipFilter, ColorKey, pSrcInfo, pPalette, ppVolumeTexture);
	if (FAILED(hr))
		return hr;

	DXUTCache_Texture NewEntry;
	NewEntry.Location = DXUTCACHE_LOCATION_RESOURCE;
	NewEntry.hSrcModule = hSrcModule;
	StringCchCopy(NewEntry.wszSource, MAX_PATH, pSrcResource);
	NewEntry.Width = Width;
	NewEntry.Height = Height;
	NewEntry.Depth = Depth;
	NewEntry.MipLevels = MipLevels;
	NewEntry.Usage = Usage;
	NewEntry.Format = Format;
	NewEntry.Pool = Pool;
	NewEntry.Type = D3DRTYPE_VOLUMETEXTURE;
	(*ppVolumeTexture)->QueryInterface(IID_IDirect3DBaseTexture9, (LPVOID*)&NewEntry.pTexture);

	m_TextureCache.Add(NewEntry);
	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::CreateFont(LPDIRECT3DDEVICE9 pDevice, UINT Height, UINT Width, UINT Weight, UINT MipLevels, BOOL Italic, DWORD CharSet, DWORD OutputPrecision, DWORD Quality, DWORD PitchAndFamily, LPCTSTR pFacename, LPD3DXFONT *ppFont)
{
	D3DXFONT_DESCW Desc;

	Desc.Height = Height;
	Desc.Width = Width;
	Desc.Weight = Weight;
	Desc.MipLevels = MipLevels;
	Desc.Italic = Italic;
	Desc.CharSet = (BYTE)CharSet;
	Desc.OutputPrecision = (BYTE)OutputPrecision;
	Desc.Quality = (BYTE)Quality;
	Desc.PitchAndFamily = (BYTE)PitchAndFamily;
	StringCchCopy(Desc.FaceName, LF_FACESIZE, pFacename);

	return CreateFontIndirect(pDevice, &Desc, ppFont);
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::CreateFontIndirect(LPDIRECT3DDEVICE9 pDevice, CONST D3DXFONT_DESC *pDesc, LPD3DXFONT *ppFont)
{
	// Search the cache for a matching entry.
	for(int i = 0; i < m_FontCache.GetSize(); ++i)
	{
		DXUTCache_Font &Entry = m_FontCache[i];

		if (Entry.Width == pDesc->Width &&
			Entry.Height == pDesc->Height &&
			Entry.Weight == pDesc->Weight &&
			Entry.MipLevels == pDesc->MipLevels &&
			Entry.Italic == pDesc->Italic &&
			Entry.CharSet == pDesc->CharSet &&
			Entry.OutputPrecision == pDesc->OutputPrecision &&
			Entry.Quality == pDesc->Quality &&
			Entry.PitchAndFamily == pDesc->PitchAndFamily &&
			CompareString(LOCALE_USER_DEFAULT, NORM_IGNORECASE,
			Entry.FaceName, -1,
			pDesc->FaceName, -1) == CSTR_EQUAL)
		{
			// A match is found.  Increment the reference and return the ID3DXFont object.
			Entry.pFont->AddRef();
			*ppFont = Entry.pFont;
			return S_OK;
		}
	}

	HRESULT hr;

	// No matching entry.  Load the resource and create a new entry.
	hr = D3DXCreateFontIndirect(pDevice, pDesc, ppFont);
	if (FAILED(hr))
		return hr;

	DXUTCache_Font NewEntry;
	(D3DXFONT_DESC &)NewEntry = *pDesc;
	NewEntry.pFont = *ppFont;
	NewEntry.pFont->AddRef();

	m_FontCache.Add(NewEntry);
	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::CreateEffectFromFile(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, const D3DXMACRO *pDefines, LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXEFFECTPOOL pPool, LPD3DXEFFECT *ppEffect, LPD3DXBUFFER *ppCompilationErrors)
{
	// Search the cache for a matching entry.
	for(int i = 0; i < m_EffectCache.GetSize(); ++i)
	{
		DXUTCache_Effect &Entry = m_EffectCache[i];

		if (Entry.Location == DXUTCACHE_LOCATION_FILE &&
			!lstrcmpW(Entry.wszSource, pSrcFile) &&
			Entry.dwFlags == Flags)
		{
			// A match is found.  Increment the ref coutn and return the ID3DXEffect object.
			*ppEffect = Entry.pEffect;
			(*ppEffect)->AddRef();
			return S_OK;
		}
	}

	HRESULT hr;

	// No matching entry.  Load the resource and create a new entry.
	hr = D3DXCreateEffectFromFile(pDevice, pSrcFile, pDefines, pInclude, Flags, pPool, ppEffect, ppCompilationErrors);
	if (FAILED(hr))
		return hr;

	DXUTCache_Effect NewEntry;
	NewEntry.Location = DXUTCACHE_LOCATION_FILE;
	StringCchCopy(NewEntry.wszSource, MAX_PATH, pSrcFile);
	NewEntry.dwFlags = Flags;
	NewEntry.pEffect = *ppEffect;
	NewEntry.pEffect->AddRef();

	m_EffectCache.Add(NewEntry);
	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::CreateEffectFromResource(LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, const D3DXMACRO *pDefines, LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXEFFECTPOOL pPool, LPD3DXEFFECT *ppEffect, LPD3DXBUFFER *ppCompilationErrors)
{
	// Search the cache for a matching entry.
	for(int i = 0; i < m_EffectCache.GetSize(); ++i)
	{
		DXUTCache_Effect &Entry = m_EffectCache[i];

		if (Entry.Location == DXUTCACHE_LOCATION_RESOURCE &&
			Entry.hSrcModule == hSrcModule &&
			!lstrcmpW(Entry.wszSource, pSrcResource) &&
			Entry.dwFlags == Flags)
		{
			// A match is found.  Increment the ref coutn and return the ID3DXEffect object.
			*ppEffect = Entry.pEffect;
			(*ppEffect)->AddRef();
			return S_OK;
		}
	}

	HRESULT hr;

	// No matching entry.  Load the resource and create a new entry.
	hr = D3DXCreateEffectFromResource(pDevice, hSrcModule, pSrcResource, pDefines, pInclude, Flags,
		pPool, ppEffect, ppCompilationErrors);
	if (FAILED(hr))
		return hr;

	DXUTCache_Effect NewEntry;
	NewEntry.Location = DXUTCACHE_LOCATION_RESOURCE;
	NewEntry.hSrcModule = hSrcModule;
	StringCchCopy(NewEntry.wszSource, MAX_PATH, pSrcResource);
	NewEntry.dwFlags = Flags;
	NewEntry.pEffect = *ppEffect;
	NewEntry.pEffect->AddRef();

	m_EffectCache.Add(NewEntry);
	return S_OK;
}


//--------------------------------------------------------------------------------------
// Device event callbacks
//--------------------------------------------------------------------------------------


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::OnCreateDevice(IDirect3DDevice9* /*pd3dDevice*/)
{
	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::OnResetDevice(IDirect3DDevice9* /*pd3dDevice*/)
{
	// Call OnResetDevice on all effect and font objects
	for(int i = 0; i < m_EffectCache.GetSize(); ++i)
		m_EffectCache[i].pEffect->OnResetDevice();
	for(int i = 0; i < m_FontCache.GetSize(); ++i)
		m_FontCache[i].pFont->OnResetDevice();


	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::OnLostDevice()
{
	// Call OnLostDevice on all effect and font objects
	for(int i = 0; i < m_EffectCache.GetSize(); ++i)
		m_EffectCache[i].pEffect->OnLostDevice();
	for(int i = 0; i < m_FontCache.GetSize(); ++i)
		m_FontCache[i].pFont->OnLostDevice();

	// Release all the default pool textures
	for(int i = m_TextureCache.GetSize() - 1; i >= 0; --i)
		if (m_TextureCache[i].Pool == D3DPOOL_DEFAULT)
		{
			SAFE_RELEASE(m_TextureCache[i].pTexture);
			m_TextureCache.Remove(i);  // Remove the entry
		}

		return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTResourceCache::OnDestroyDevice()
{
	// Release all resources
	for(int i = m_EffectCache.GetSize() - 1; i >= 0; --i)
	{
		SAFE_RELEASE(m_EffectCache[i].pEffect);
		m_EffectCache.Remove(i);
	}
	for(int i = m_FontCache.GetSize() - 1; i >= 0; --i)
	{
		SAFE_RELEASE(m_FontCache[i].pFont);
		m_FontCache.Remove(i);
	}
	for(int i = m_TextureCache.GetSize() - 1; i >= 0; --i)
	{
		SAFE_RELEASE(m_TextureCache[i].pTexture);
		m_TextureCache.Remove(i);
	}

	return S_OK;
}




//--------------------------------------------------------------------------------------
// Gives the D3D device a cursor with image and hotspot from hCursor.
//--------------------------------------------------------------------------------------
HRESULT DXUTSetDeviceCursor(IDirect3DDevice9* pd3dDevice, HCURSOR hCursor, bool bAddWatermark)
{
	HRESULT hr = E_FAIL;
	ICONINFO iconinfo;
	bool bBWCursor;
	LPDIRECT3DSURFACE9 pCursorSurface = NULL;
	HDC hdcColor = NULL;
	HDC hdcMask = NULL;
	HDC hdcScreen = NULL;
	BITMAP bm;
	DWORD dwWidth;
	DWORD dwHeightSrc;
	DWORD dwHeightDest;
	COLORREF crColor;
	COLORREF crMask;
	UINT x;
	UINT y;
	BITMAPINFO bmi;
	COLORREF* pcrArrayColor = NULL;
	COLORREF* pcrArrayMask = NULL;
	DWORD* pBitmap;
	HGDIOBJ hgdiobjOld;

	ZeroMemory(&iconinfo, sizeof(iconinfo));
	if (!GetIconInfo(hCursor, &iconinfo))
		goto End;

	if (0 == GetObject((HGDIOBJ)iconinfo.hbmMask, sizeof(BITMAP), (LPVOID)&bm))
		goto End;
	dwWidth = bm.bmWidth;
	dwHeightSrc = bm.bmHeight;

	if (iconinfo.hbmColor == NULL)
	{
		bBWCursor = TRUE;
		dwHeightDest = dwHeightSrc / 2;
	}
	else 
	{
		bBWCursor = FALSE;
		dwHeightDest = dwHeightSrc;
	}

	// Create a surface for the fullscreen cursor
	if (FAILED(hr = pd3dDevice->CreateOffscreenPlainSurface(dwWidth, dwHeightDest, 
		D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, &pCursorSurface, NULL)))
	{
		goto End;
	}

	pcrArrayMask = new DWORD[dwWidth * dwHeightSrc];

	ZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
	bmi.bmiHeader.biWidth = dwWidth;
	bmi.bmiHeader.biHeight = dwHeightSrc;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	hdcScreen = GetDC(NULL);
	hdcMask = CreateCompatibleDC(hdcScreen);
	if (hdcMask == NULL)
	{
		hr = E_FAIL;
		goto End;
	}
	hgdiobjOld = SelectObject(hdcMask, iconinfo.hbmMask);
	GetDIBits(hdcMask, iconinfo.hbmMask, 0, dwHeightSrc, 
		pcrArrayMask, &bmi, DIB_RGB_COLORS);
	SelectObject(hdcMask, hgdiobjOld);

	if (!bBWCursor)
	{
		pcrArrayColor = new DWORD[dwWidth * dwHeightDest];
		hdcColor = CreateCompatibleDC(hdcScreen);
		if (hdcColor == NULL)
		{
			hr = E_FAIL;
			goto End;
		}
		SelectObject(hdcColor, iconinfo.hbmColor);
		GetDIBits(hdcColor, iconinfo.hbmColor, 0, dwHeightDest, 
			pcrArrayColor, &bmi, DIB_RGB_COLORS);
	}

	// Transfer cursor image into the surface
	D3DLOCKED_RECT lr;
	pCursorSurface->LockRect(&lr, NULL, 0);
	pBitmap = (DWORD*)lr.pBits;
	for(y = 0; y < dwHeightDest; y++)
	{
		for(x = 0; x < dwWidth; x++)
		{
			if (bBWCursor)
			{
				crColor = pcrArrayMask[dwWidth*(dwHeightDest-1-y) + x];
				crMask = pcrArrayMask[dwWidth*(dwHeightSrc-1-y) + x];
			}
			else
			{
				crColor = pcrArrayColor[dwWidth*(dwHeightDest-1-y) + x];
				crMask = pcrArrayMask[dwWidth*(dwHeightDest-1-y) + x];
			}
			if (crMask == 0)
				pBitmap[dwWidth*y + x] = 0xff000000 | crColor;
			else
				pBitmap[dwWidth*y + x] = 0x00000000;

			// It may be helpful to make the D3D cursor look slightly 
			// different from the Windows cursor so you can distinguish 
			// between the two when developing/testing code.  When
			// bAddWatermark is TRUE, the following code adds some
			// small grey "D3D" characters to the upper-left corner of
			// the D3D cursor image.
			if (bAddWatermark && x < 12 && y < 5)
			{
				// 11.. 11.. 11.. .... CCC0
				// 1.1. ..1. 1.1. .... A2A0
				// 1.1. .1.. 1.1. .... A4A0
				// 1.1. ..1. 1.1. .... A2A0
				// 11.. 11.. 11.. .... CCC0

				const WORD wMask[5] = { 0xccc0, 0xa2a0, 0xa4a0, 0xa2a0, 0xccc0 };
				if (wMask[y] & (1 << (15 - x)))
				{
					pBitmap[dwWidth*y + x] |= 0xff808080;
				}
			}
		}
	}
	pCursorSurface->UnlockRect();

	// Set the device cursor
	if (FAILED(hr = pd3dDevice->SetCursorProperties(iconinfo.xHotspot, 
		iconinfo.yHotspot, pCursorSurface)))
	{
		goto End;
	}

	hr = S_OK;

End:
	if (iconinfo.hbmMask != NULL)
		DeleteObject(iconinfo.hbmMask);
	if (iconinfo.hbmColor != NULL)
		DeleteObject(iconinfo.hbmColor);
	if (hdcScreen != NULL)
		ReleaseDC(NULL, hdcScreen);
	if (hdcColor != NULL)
		DeleteDC(hdcColor);
	if (hdcMask != NULL)
		DeleteDC(hdcMask);
	SAFE_DELETE_ARRAY(pcrArrayColor);
	SAFE_DELETE_ARRAY(pcrArrayMask);
	SAFE_RELEASE(pCursorSurface);
	return hr;
}


//--------------------------------------------------------------------------------------
// Desc: Returns a view matrix for rendering to a face of a cubemap.
//--------------------------------------------------------------------------------------
D3DXMATRIX DXUTGetCubeMapViewMatrix(DWORD dwFace)
{
	D3DXVECTOR3 vEyePt   = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vLookDir;
	D3DXVECTOR3 vUpDir;

	switch(dwFace)
	{
	case D3DCUBEMAP_FACE_POSITIVE_X:
		vLookDir = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
		vUpDir   = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		break;
	case D3DCUBEMAP_FACE_NEGATIVE_X:
		vLookDir = D3DXVECTOR3(-1.0f, 0.0f, 0.0f);
		vUpDir   = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		break;
	case D3DCUBEMAP_FACE_POSITIVE_Y:
		vLookDir = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		vUpDir   = D3DXVECTOR3(0.0f, 0.0f,-1.0f);
		break;
	case D3DCUBEMAP_FACE_NEGATIVE_Y:
		vLookDir = D3DXVECTOR3(0.0f,-1.0f, 0.0f);
		vUpDir   = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		break;
	case D3DCUBEMAP_FACE_POSITIVE_Z:
		vLookDir = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		vUpDir   = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		break;
	case D3DCUBEMAP_FACE_NEGATIVE_Z:
		vLookDir = D3DXVECTOR3(0.0f, 0.0f,-1.0f);
		vUpDir   = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
		break;
	}

	// Set the view transform for this cubemap surface
	D3DXMATRIXA16 mView;
	D3DXMatrixLookAtLH(&mView, &vEyePt, &vLookDir, &vUpDir);
	return mView;
}


//--------------------------------------------------------------------------------------
// Returns the string for the given D3DFORMAT.
//--------------------------------------------------------------------------------------
LPCWSTR DXUTD3DFormatToString(D3DFORMAT format, bool bWithPrefix)
{
	WCHAR* pstr = NULL;
	switch(format)
	{
	case D3DFMT_UNKNOWN:         pstr = L"D3DFMT_UNKNOWN"; break;
	case D3DFMT_R8G8B8:          pstr = L"D3DFMT_R8G8B8"; break;
	case D3DFMT_A8R8G8B8:        pstr = L"D3DFMT_A8R8G8B8"; break;
	case D3DFMT_X8R8G8B8:        pstr = L"D3DFMT_X8R8G8B8"; break;
	case D3DFMT_R5G6B5:          pstr = L"D3DFMT_R5G6B5"; break;
	case D3DFMT_X1R5G5B5:        pstr = L"D3DFMT_X1R5G5B5"; break;
	case D3DFMT_A1R5G5B5:        pstr = L"D3DFMT_A1R5G5B5"; break;
	case D3DFMT_A4R4G4B4:        pstr = L"D3DFMT_A4R4G4B4"; break;
	case D3DFMT_R3G3B2:          pstr = L"D3DFMT_R3G3B2"; break;
	case D3DFMT_A8:              pstr = L"D3DFMT_A8"; break;
	case D3DFMT_A8R3G3B2:        pstr = L"D3DFMT_A8R3G3B2"; break;
	case D3DFMT_X4R4G4B4:        pstr = L"D3DFMT_X4R4G4B4"; break;
	case D3DFMT_A2B10G10R10:     pstr = L"D3DFMT_A2B10G10R10"; break;
	case D3DFMT_A8B8G8R8:        pstr = L"D3DFMT_A8B8G8R8"; break;
	case D3DFMT_X8B8G8R8:        pstr = L"D3DFMT_X8B8G8R8"; break;
	case D3DFMT_G16R16:          pstr = L"D3DFMT_G16R16"; break;
	case D3DFMT_A2R10G10B10:     pstr = L"D3DFMT_A2R10G10B10"; break;
	case D3DFMT_A16B16G16R16:    pstr = L"D3DFMT_A16B16G16R16"; break;
	case D3DFMT_A8P8:            pstr = L"D3DFMT_A8P8"; break;
	case D3DFMT_P8:              pstr = L"D3DFMT_P8"; break;
	case D3DFMT_L8:              pstr = L"D3DFMT_L8"; break;
	case D3DFMT_A8L8:            pstr = L"D3DFMT_A8L8"; break;
	case D3DFMT_A4L4:            pstr = L"D3DFMT_A4L4"; break;
	case D3DFMT_V8U8:            pstr = L"D3DFMT_V8U8"; break;
	case D3DFMT_L6V5U5:          pstr = L"D3DFMT_L6V5U5"; break;
	case D3DFMT_X8L8V8U8:        pstr = L"D3DFMT_X8L8V8U8"; break;
	case D3DFMT_Q8W8V8U8:        pstr = L"D3DFMT_Q8W8V8U8"; break;
	case D3DFMT_V16U16:          pstr = L"D3DFMT_V16U16"; break;
	case D3DFMT_A2W10V10U10:     pstr = L"D3DFMT_A2W10V10U10"; break;
	case D3DFMT_UYVY:            pstr = L"D3DFMT_UYVY"; break;
	case D3DFMT_YUY2:            pstr = L"D3DFMT_YUY2"; break;
	case D3DFMT_DXT1:            pstr = L"D3DFMT_DXT1"; break;
	case D3DFMT_DXT2:            pstr = L"D3DFMT_DXT2"; break;
	case D3DFMT_DXT3:            pstr = L"D3DFMT_DXT3"; break;
	case D3DFMT_DXT4:            pstr = L"D3DFMT_DXT4"; break;
	case D3DFMT_DXT5:            pstr = L"D3DFMT_DXT5"; break;
	case D3DFMT_D16_LOCKABLE:    pstr = L"D3DFMT_D16_LOCKABLE"; break;
	case D3DFMT_D32:             pstr = L"D3DFMT_D32"; break;
	case D3DFMT_D15S1:           pstr = L"D3DFMT_D15S1"; break;
	case D3DFMT_D24S8:           pstr = L"D3DFMT_D24S8"; break;
	case D3DFMT_D24X8:           pstr = L"D3DFMT_D24X8"; break;
	case D3DFMT_D24X4S4:         pstr = L"D3DFMT_D24X4S4"; break;
	case D3DFMT_D16:             pstr = L"D3DFMT_D16"; break;
	case D3DFMT_L16:             pstr = L"D3DFMT_L16"; break;
	case D3DFMT_VERTEXDATA:      pstr = L"D3DFMT_VERTEXDATA"; break;
	case D3DFMT_INDEX16:         pstr = L"D3DFMT_INDEX16"; break;
	case D3DFMT_INDEX32:         pstr = L"D3DFMT_INDEX32"; break;
	case D3DFMT_Q16W16V16U16:    pstr = L"D3DFMT_Q16W16V16U16"; break;
	case D3DFMT_MULTI2_ARGB8:    pstr = L"D3DFMT_MULTI2_ARGB8"; break;
	case D3DFMT_R16F:            pstr = L"D3DFMT_R16F"; break;
	case D3DFMT_G16R16F:         pstr = L"D3DFMT_G16R16F"; break;
	case D3DFMT_A16B16G16R16F:   pstr = L"D3DFMT_A16B16G16R16F"; break;
	case D3DFMT_R32F:            pstr = L"D3DFMT_R32F"; break;
	case D3DFMT_G32R32F:         pstr = L"D3DFMT_G32R32F"; break;
	case D3DFMT_A32B32G32R32F:   pstr = L"D3DFMT_A32B32G32R32F"; break;
	case D3DFMT_CxV8U8:          pstr = L"D3DFMT_CxV8U8"; break;
	default:                     pstr = L"Unknown format"; break;
	}
	if (bWithPrefix || wcsstr(pstr, L"D3DFMT_")== NULL)
		return pstr;
	else
		return pstr + lstrlen(L"D3DFMT_");
}



//--------------------------------------------------------------------------------------
// Outputs to the debug stream a formatted Unicode string with a variable-argument list.
//--------------------------------------------------------------------------------------
VOID DXUTOutputDebugStringW(LPCWSTR strMsg, ...)
{
#if defined(DEBUG) | defined(_DEBUG)
	WCHAR strBuffer[512];

	va_list args;
	va_start(args, strMsg);
	StringCchVPrintfW(strBuffer, 512, strMsg, args);
	strBuffer[511] = L'\0';
	va_end(args);

	OutputDebugString(strBuffer);
#else
	UNREFERENCED_PARAMETER(strMsg);
#endif
}


//--------------------------------------------------------------------------------------
// Outputs to the debug stream a formatted MBCS string with a variable-argument list.
//--------------------------------------------------------------------------------------
VOID DXUTOutputDebugStringA(LPCSTR strMsg, ...)
{
#if defined(DEBUG) | defined(_DEBUG)
	CHAR strBuffer[512];

	va_list args;
	va_start(args, strMsg);
	StringCchVPrintfA(strBuffer, 512, strMsg, args);
	strBuffer[511] = '\0';
	va_end(args);

	OutputDebugStringA(strBuffer);
#else
	UNREFERENCED_PARAMETER(strMsg);
#endif
}


HRESULT DXUTErrMsg(WCHAR * str, HRESULT hr)
{
	MessageBox(nullptr, str, L"Warning!", MB_OK);
	return hr;
}

//--------------------------------------------------------------------------------------
CDXUTLineManager::CDXUTLineManager()
{
	m_pd3dDevice = NULL;
	m_pD3DXLine = NULL;
}


//--------------------------------------------------------------------------------------
CDXUTLineManager::~CDXUTLineManager()
{
	OnDeletedDevice();
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::OnCreatedDevice(IDirect3DDevice9* pd3dDevice)
{
	m_pd3dDevice = pd3dDevice;

	HRESULT hr;
	hr = D3DXCreateLine(m_pd3dDevice, &m_pD3DXLine);
	if (FAILED(hr))
		return hr;

	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::OnResetDevice()
{
	if (m_pD3DXLine)
		m_pD3DXLine->OnResetDevice();

	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::OnRender()
{
	HRESULT hr;
	if (NULL == m_pD3DXLine)
		return E_INVALIDARG;

	bool bDrawingHasBegun = false;
	float fLastWidth = 0.0f;
	bool bLastAntiAlias = false;

	for(int i=0; i<m_LinesList.GetSize(); i++)
	{
		LINE_NODE* pLineNode = m_LinesList.GetAt(i);
		if (pLineNode)
		{
			if (!bDrawingHasBegun || 
				fLastWidth != pLineNode->fWidth || 
				bLastAntiAlias != pLineNode->bAntiAlias)
			{
				if (bDrawingHasBegun)
				{
					hr = m_pD3DXLine->End();
					if (FAILED(hr))
						return hr;
				}

				m_pD3DXLine->SetWidth(pLineNode->fWidth);
				m_pD3DXLine->SetAntialias(pLineNode->bAntiAlias);

				fLastWidth = pLineNode->fWidth;
				bLastAntiAlias = pLineNode->bAntiAlias;

				hr = m_pD3DXLine->Begin();
				if (FAILED(hr))
					return hr;
				bDrawingHasBegun = true;
			}

			hr = m_pD3DXLine->Draw(pLineNode->pVertexList, pLineNode->dwVertexListCount, pLineNode->Color);
			if (FAILED(hr))
				return hr;
		}
	}

	if (bDrawingHasBegun)
	{
		hr = m_pD3DXLine->End();
		if (FAILED(hr))
			return hr;
	}

	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::OnLostDevice()
{
	if (m_pD3DXLine)
		m_pD3DXLine->OnLostDevice();

	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::OnDeletedDevice()
{
	RemoveAllLines();
	SAFE_RELEASE(m_pD3DXLine);

	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::AddLine(int* pnLineID, D3DXVECTOR2* pVertexList, DWORD dwVertexListCount, D3DCOLOR Color, float fWidth, float fScaleRatio, bool bAntiAlias)
{
	if (pVertexList == NULL || dwVertexListCount == 0)
		return E_INVALIDARG;

	LINE_NODE* pLineNode = new LINE_NODE;
	if (pLineNode == NULL)
		return E_OUTOFMEMORY;
	ZeroMemory(pLineNode, sizeof(LINE_NODE));

	pLineNode->nLineID = m_LinesList.GetSize();
	pLineNode->Color = Color;
	pLineNode->fWidth = fWidth;
	pLineNode->bAntiAlias = bAntiAlias;
	pLineNode->dwVertexListCount = dwVertexListCount;

	if (pnLineID)
		*pnLineID = pLineNode->nLineID;

	pLineNode->pVertexList = new D3DXVECTOR2[dwVertexListCount];
	if (pLineNode->pVertexList == NULL)
	{
		delete pLineNode;
		return E_OUTOFMEMORY;
	}
	for(DWORD i=0; i<dwVertexListCount; i++)
	{
		pLineNode->pVertexList[i] = pVertexList[i] * fScaleRatio;
	}

	m_LinesList.Add(pLineNode);

	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::AddRect(int* pnLineID, RECT rc, D3DCOLOR Color, float fWidth, float fScaleRatio, bool bAntiAlias)
{
	if (fWidth > 2.0f)
	{
		D3DXVECTOR2 vertexList[8];

		vertexList[0].x = (float)rc.left;
		vertexList[0].y = (float)rc.top - (fWidth/2.0f);

		vertexList[1].x = (float)rc.left;
		vertexList[1].y = (float)rc.bottom + (fWidth/2.0f);

		vertexList[2].x = (float)rc.left;
		vertexList[2].y = (float)rc.bottom - 0.5f;

		vertexList[3].x = (float)rc.right;
		vertexList[3].y = (float)rc.bottom - 0.5f;

		vertexList[4].x = (float)rc.right;
		vertexList[4].y = (float)rc.bottom + (fWidth/2.0f);

		vertexList[5].x = (float)rc.right;
		vertexList[5].y = (float)rc.top - (fWidth/2.0f);

		vertexList[6].x = (float)rc.right;
		vertexList[6].y = (float)rc.top;

		vertexList[7].x = (float)rc.left;
		vertexList[7].y = (float)rc.top;

		return AddLine(pnLineID, vertexList, 8, Color, fWidth, fScaleRatio, bAntiAlias);
	}
	else
	{
		D3DXVECTOR2 vertexList[5];
		vertexList[0].x = (float)rc.left;
		vertexList[0].y = (float)rc.top;

		vertexList[1].x = (float)rc.left;
		vertexList[1].y = (float)rc.bottom;

		vertexList[2].x = (float)rc.right;
		vertexList[2].y = (float)rc.bottom;

		vertexList[3].x = (float)rc.right;
		vertexList[3].y = (float)rc.top;

		vertexList[4].x = (float)rc.left;
		vertexList[4].y = (float)rc.top;

		return AddLine(pnLineID, vertexList, 5, Color, fWidth, fScaleRatio, bAntiAlias);
	}
}



//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::RemoveLine(int nLineID)
{
	for(int i=0; i<m_LinesList.GetSize(); i++)
	{
		LINE_NODE* pLineNode = m_LinesList.GetAt(i);
		if (pLineNode && pLineNode->nLineID == nLineID)
		{
			SAFE_DELETE_ARRAY(pLineNode->pVertexList);
			delete pLineNode;
			m_LinesList.SetAt(i, NULL);
		}
	}

	return S_OK;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTLineManager::RemoveAllLines()
{
	for(int i=0; i<m_LinesList.GetSize(); i++)
	{
		LINE_NODE* pLineNode = m_LinesList.GetAt(i);
		if (pLineNode)
		{
			SAFE_DELETE_ARRAY(pLineNode->pVertexList);
			delete pLineNode;
		}
	}
	m_LinesList.RemoveAll();

	return S_OK;
}


//--------------------------------------------------------------------------------------
CDXUTTextHelper::CDXUTTextHelper(ID3DXFont* pFont, ID3DXSprite* pSprite, int nLineHeight)
{
	m_pFont = pFont;
	m_pSprite = pSprite;
	m_clr = D3DXCOLOR(1,1,1,1);
	m_pt.x = 0; 
	m_pt.y = 0; 
	m_nLineHeight = nLineHeight;
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTTextHelper::DrawFormattedTextLine(const WCHAR* strMsg, ...)
{
	WCHAR strBuffer[512];

	va_list args;
	va_start(args, strMsg);
	StringCchVPrintf(strBuffer, 512, strMsg, args);
	strBuffer[511] = L'\0';
	va_end(args);

	return DrawTextLine(strBuffer);
}


//--------------------------------------------------------------------------------------
HRESULT CDXUTTextHelper::DrawTextLine(const WCHAR* strMsg)
{
	if (NULL == m_pFont)
	{
		DXUT_ERR_MSGBOX(L"DrawTextLine", E_INVALIDARG);
		return E_INVALIDARG;
	}

	HRESULT hr;
	RECT rc;
	SetRect(&rc, m_pt.x, m_pt.y, 0, 0); 
	hr = m_pFont->DrawText(m_pSprite, strMsg, -1, &rc, DT_NOCLIP, m_clr);
	if (FAILED(hr))
	{
		ErrorBox(K_ERR_WARNING, L"DrawTextLine error! hresult:%x", hr);
		return E_FAIL;// DXTRACE_ERR_MSGBOX(L"DrawText", hr);
	}

	m_pt.y += m_nLineHeight;

	return S_OK;
}


HRESULT CDXUTTextHelper::DrawFormattedTextLine(RECT &rc, DWORD dwFlags, const WCHAR* strMsg, ...)
{
	WCHAR strBuffer[512];

	va_list args;
	va_start(args, strMsg);
	StringCchVPrintf(strBuffer, 512, strMsg, args);
	strBuffer[511] = L'\0';
	va_end(args);

	return DrawTextLine(rc, dwFlags, strBuffer);
}


HRESULT CDXUTTextHelper::DrawTextLine(RECT &rc, DWORD dwFlags, const WCHAR* strMsg)
{
	if (NULL == m_pFont)
	{
		DXUT_ERR_MSGBOX(L"DrawTextLine", E_INVALIDARG);
		return E_INVALIDARG;
	}

	HRESULT hr;
	hr = m_pFont->DrawText(m_pSprite, strMsg, -1, &rc, dwFlags, m_clr);
	if (FAILED(hr))
	{
		ErrorBox(K_ERR_WARNING, L"DrawTextLine error! hresult:%x", hr);
		return E_FAIL;// DXTRACE_ERR_MSGBOX(L"DrawText", hr);
	}

	m_pt.y += m_nLineHeight;

	return S_OK;
}


//--------------------------------------------------------------------------------------
void CDXUTTextHelper::Begin()
{
	if (m_pSprite)
		m_pSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);
}
void CDXUTTextHelper::End()
{
	if (m_pSprite)
		m_pSprite->End();
}


//--------------------------------------------------------------------------------------
// Direct3D9 dynamic linking support -- calls top-level D3D9 APIs with graceful
// failure if APIs are not present.
//--------------------------------------------------------------------------------------

// Function prototypes
typedef IDirect3D9* (WINAPI * LPDIRECT3DCREATE9) (UINT);
typedef INT         (WINAPI * LPD3DPERF_BEGINEVENT)(D3DCOLOR, LPCWSTR);
typedef INT         (WINAPI * LPD3DPERF_ENDEVENT)(void);
typedef VOID        (WINAPI * LPD3DPERF_SETMARKER)(D3DCOLOR, LPCWSTR);
typedef VOID        (WINAPI * LPD3DPERF_SETREGION)(D3DCOLOR, LPCWSTR);
typedef BOOL        (WINAPI * LPD3DPERF_QUERYREPEATFRAME)(void);
typedef VOID        (WINAPI * LPD3DPERF_SETOPTIONS)(DWORD dwOptions);
typedef DWORD       (WINAPI * LPD3DPERF_GETSTATUS)(void);

// Module and function pointers
static HMODULE s_hModD3D9 = NULL;
static LPDIRECT3DCREATE9 s_DynamicDirect3DCreate9 = NULL;
static LPD3DPERF_BEGINEVENT s_DynamicD3DPERF_BeginEvent = NULL;
static LPD3DPERF_ENDEVENT s_DynamicD3DPERF_EndEvent = NULL;
static LPD3DPERF_SETMARKER s_DynamicD3DPERF_SetMarker = NULL;
static LPD3DPERF_SETREGION s_DynamicD3DPERF_SetRegion = NULL;
static LPD3DPERF_QUERYREPEATFRAME s_DynamicD3DPERF_QueryRepeatFrame = NULL;
static LPD3DPERF_SETOPTIONS s_DynamicD3DPERF_SetOptions = NULL;
static LPD3DPERF_GETSTATUS s_DynamicD3DPERF_GetStatus = NULL;

// Ensure function pointers are initialized
static bool DXUT_EnsureD3DAPIs(void)
{
	// If module is non-NULL, this function has already been called.  Note
	// that this doesn't guarantee that all D3D9 procaddresses were found.
	if (s_hModD3D9 != NULL)
		return true;

	// This may fail if DirectX 9 isn't installed
	WCHAR wszPath[MAX_PATH+1];
	if (!::GetSystemDirectory(wszPath, MAX_PATH+1))
		return false;
	StringCchCat(wszPath, MAX_PATH, L"\\d3d9.dll");
	s_hModD3D9 = LoadLibrary(wszPath);
	if (s_hModD3D9 == NULL) 
		return false;
	s_DynamicDirect3DCreate9 = (LPDIRECT3DCREATE9)GetProcAddress(s_hModD3D9, "Direct3DCreate9");
	s_DynamicD3DPERF_BeginEvent = (LPD3DPERF_BEGINEVENT)GetProcAddress(s_hModD3D9, "D3DPERF_BeginEvent");
	s_DynamicD3DPERF_EndEvent = (LPD3DPERF_ENDEVENT)GetProcAddress(s_hModD3D9, "D3DPERF_EndEvent");
	s_DynamicD3DPERF_SetMarker = (LPD3DPERF_SETMARKER)GetProcAddress(s_hModD3D9, "D3DPERF_SetMarker");
	s_DynamicD3DPERF_SetRegion = (LPD3DPERF_SETREGION)GetProcAddress(s_hModD3D9, "D3DPERF_SetRegion");
	s_DynamicD3DPERF_QueryRepeatFrame = (LPD3DPERF_QUERYREPEATFRAME)GetProcAddress(s_hModD3D9, "D3DPERF_QueryRepeatFrame");
	s_DynamicD3DPERF_SetOptions = (LPD3DPERF_SETOPTIONS)GetProcAddress(s_hModD3D9, "D3DPERF_SetOptions");
	s_DynamicD3DPERF_GetStatus = (LPD3DPERF_GETSTATUS)GetProcAddress(s_hModD3D9, "D3DPERF_GetStatus");
	return true;
}

IDirect3D9 * WINAPI DXUT_Dynamic_Direct3DCreate9(UINT SDKVersion) 
{
	if (DXUT_EnsureD3DAPIs() && s_DynamicDirect3DCreate9 != NULL)
		return s_DynamicDirect3DCreate9(SDKVersion);
	else
		return NULL;
}

int WINAPI DXUT_Dynamic_D3DPERF_BeginEvent(D3DCOLOR col, LPCWSTR wszName)
{
	if (DXUT_EnsureD3DAPIs() && s_DynamicD3DPERF_BeginEvent != NULL)
		return s_DynamicD3DPERF_BeginEvent(col, wszName);
	else
		return -1;
}

int WINAPI DXUT_Dynamic_D3DPERF_EndEvent(void)
{
	if (DXUT_EnsureD3DAPIs() && s_DynamicD3DPERF_EndEvent != NULL)
		return s_DynamicD3DPERF_EndEvent();
	else
		return -1;
}

void WINAPI DXUT_Dynamic_D3DPERF_SetMarker(D3DCOLOR col, LPCWSTR wszName)
{
	if (DXUT_EnsureD3DAPIs() && s_DynamicD3DPERF_SetMarker != NULL)
		s_DynamicD3DPERF_SetMarker(col, wszName);
}

void WINAPI DXUT_Dynamic_D3DPERF_SetRegion(D3DCOLOR col, LPCWSTR wszName)
{
	if (DXUT_EnsureD3DAPIs() && s_DynamicD3DPERF_SetRegion != NULL)
		s_DynamicD3DPERF_SetRegion(col, wszName);
}

BOOL WINAPI DXUT_Dynamic_D3DPERF_QueryRepeatFrame(void)
{
	if (DXUT_EnsureD3DAPIs() && s_DynamicD3DPERF_QueryRepeatFrame != NULL)
		return s_DynamicD3DPERF_QueryRepeatFrame();
	else
		return FALSE;
}

void WINAPI DXUT_Dynamic_D3DPERF_SetOptions(DWORD dwOptions)
{
	if (DXUT_EnsureD3DAPIs() && s_DynamicD3DPERF_SetOptions != NULL)
		s_DynamicD3DPERF_SetOptions(dwOptions);
}

DWORD WINAPI DXUT_Dynamic_D3DPERF_GetStatus(void)
{
	if (DXUT_EnsureD3DAPIs() && s_DynamicD3DPERF_GetStatus != NULL)
		return s_DynamicD3DPERF_GetStatus();
	else
		return 0;
}
