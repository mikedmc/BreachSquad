#pragma once
#include "interfaces/DeviceRes.h"

class CTexNode
{
friend class CTextureManager; 

private:
	bool				bLoaded;		// is it actually loaded?
public:
	UINT				widthToLoad;	// forces texture loading to this width
	UINT				heightToLoad;

	WCHAR				fileName[ MAX_PATH ];
	PTEXTURE			pTexture;
	FORMAT3D			format;
	IMAGE_INFO			info;
	DWORD				filter, mipFilter;
	//CTOR
	CTexNode() : widthToLoad( D3DX_DEFAULT ), heightToLoad( D3DX_DEFAULT ),
		filter( D3DX_DEFAULT ),
		mipFilter( D3DX_DEFAULT )
	{
		bLoaded = false;
		fileName[ 0 ] = 0;
	}

	inline bool isLoaded() {
		return bLoaded;
	}

	Vec2 getSize() {
		if ( bLoaded == false )
			return g_Vec2Zero;
		return Vec2( info.Width, info.Height );
	}
};

// Keeps collection of active textures.
class CTextureManager : public IDeviceRes
{
private:
	// Internal function that actually does the loading from the file
	OPRESULT				LoadTexture( const int nTexIdx );

public:
	CArray<CTexNode*>		arrTextures;

public:
	CTextureManager( void );
	~CTextureManager( void );

	// Releases a texture but doesn't delete array entry so we don't get dangling pointers
	void					ReleaseTexture( CTexNode* pTN );

	// Adds a new texture and returns a pointer to the texture structure or null if we have errors
	CTexNode*				AddTexture( const WCHAR* fileName, D3DFORMAT format, DWORD filter, DWORD mipFilter, UINT nSetWidth = D3DX_DEFAULT, UINT nSetHeight = D3DX_DEFAULT );

	// Use it only when you know what you're doing
	CTexNode*				GetTextureByIndex( int nIndex );

	// Releases all textures
	void					Release();

	// Inherited via IDeviceRes
	virtual OPRESULT OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc = nullptr ) override;
	virtual OPRESULT OnResetDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc = nullptr ) override;
	virtual OPRESULT OnLostDevice() override;
	virtual OPRESULT OnDestroyDevice() override;
};
