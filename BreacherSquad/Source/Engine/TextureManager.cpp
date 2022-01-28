#include "dxstdafx.h"

CTextureManager::CTextureManager( void )
{
	m_pDevice = nullptr;
}

CTextureManager::~CTextureManager( void )
{
	Release();
}

void CTextureManager::Release( void )
{
	// Release textures and delete array of textures
	for ( int i = 0; i < arrTextures.GetSize(); i++ )
	{
		CTexNode *pTN = arrTextures.GetAt( i );
		LOG( L"CTextureManager::Released %s", pTN->fileName );
		SAFE_RELEASE( pTN->pTexture );
		SAFE_DELETE( pTN );
	}
	arrTextures.RemoveAll();
}

CTexNode* CTextureManager::AddTexture( const WCHAR* fileName, D3DFORMAT format, DWORD filter, DWORD mipFilter, UINT nSetWidth, UINT nSetHeight )
{
	int texIdx = -1;

	if ( wcslen( fileName ) <= 0 )
	{
		ErrorBox( K_ERR_WARNING, L"AddTexture:fileName is empty! No texture added." );
		return nullptr;
	}
	//--- check file exists ---
	if ( FILE *file = OS_wfopen( fileName, L"r" ) ) {
		OS_fclose( file );
	}
	else
	{
		ErrorBox( K_ERR_WARNING, L"AddTexture:file not found!\n%s", fileName );
		return nullptr;
	}

	for ( int i = 0; i < arrTextures.GetSize(); i++ )
	{
		CTexNode* pTN = arrTextures.GetAt( i );

		//TODO: maybe it should reload the texture?
		if ( wcscmp( pTN->fileName, fileName ) == 0 )
		{
			// The texture already exists
			texIdx = i;
			return arrTextures[ texIdx ];
		}
	}

	// Add the new texture
	CTexNode *pNewTex = new CTexNode();
	if ( pNewTex == NULL )
		return nullptr;
	_ASSERT( pNewTex != nullptr );

	ZeroMemory( pNewTex, sizeof( CTexNode ) );

	StringCchCopy( pNewTex->fileName, MAX_PATH, fileName );
	pNewTex->format = format;
	pNewTex->filter = filter;
	pNewTex->mipFilter = mipFilter;
	pNewTex->widthToLoad = nSetWidth;
	pNewTex->heightToLoad = nSetHeight;

	arrTextures.Add( pNewTex );
	texIdx = arrTextures.GetSize() - 1;

	// Try to create the new texture now
	if ( OP_FAILED( LoadTexture( texIdx ) ) )
	{
		SAFE_DELETE( pNewTex );
		arrTextures.Remove( texIdx );
		return nullptr;
	}

	return arrTextures[ texIdx ];
}

CTexNode* CTextureManager::GetTextureByIndex( int nIndex )
{
	if ( nIndex < 0 || nIndex >= arrTextures.Count() )
		return nullptr;
	return arrTextures[ nIndex ];
}

void CTextureManager::ReleaseTexture( CTexNode* pTN )
{
	_ASSERT( pTN != nullptr );

	LOG( L"CTextureManager::DeleteTexture released %s", pTN->fileName );
	SAFE_RELEASE( pTN->pTexture );
	pTN->fileName[ 0 ] = 0;
	pTN->bLoaded = false;
}

OPRESULT CTextureManager::LoadTexture( const int nTexIdx )
{
	if ( m_pDevice == nullptr || nTexIdx < 0 || nTexIdx >= arrTextures.GetSize() )
		return K_OP_INVALIDARGS;

	CTexNode *pTN = arrTextures.GetAt( nTexIdx );
	// Make sure there's a texture to create
	if ( wcslen( pTN->fileName ) == 0 )
		return K_OP_OK;
	// Deallocate if already allocated
	if ( pTN->pTexture != nullptr )
	{
		SAFE_RELEASE( pTN->pTexture );
		pTN->bLoaded = false;
		LOG( L"CTextureManager::LoadTexture released %s", pTN->fileName );
	}

	// Create texture (managed)
	if ( FAILED( D3DXCreateTextureFromFileEx( m_pDevice, pTN->fileName, pTN->widthToLoad, pTN->heightToLoad,
		1, 0, pTN->format, D3DPOOL_MANAGED,
		pTN->filter, pTN->mipFilter, 0,
		&pTN->info, NULL, &pTN->pTexture ) ) )
	{
		return OPRESULT( K_OP_FAILED, K_SEVERITY_WARNING, L"[CTextureManager::LoadTexture] D3DXCreateTextureFromFileEx\n -Could not load texture %s\n", pTN->fileName );
	}

	pTN->bLoaded = true;
	LOG( L"CTextureManager::Loaded %s", pTN->fileName );
	return K_OP_OK;
}


OPRESULT CTextureManager::OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc )
{
	// the textures are managed, only reallocate if device changed
	for ( int kk = 0; kk < arrTextures.GetSize(); kk++ )
	{
		V_OP_RET( LoadTexture( kk ) );
	}
	return K_OP_OK;
}

OPRESULT CTextureManager::OnResetDevice( PDEVICE pDevice, const SURFACE_DESC * pBBDesc )
{
	m_pDevice = pDevice;
	return K_OP_OK;
}

OPRESULT CTextureManager::OnLostDevice()
{
	m_pDevice = nullptr;
	return K_OP_OK;
}

OPRESULT CTextureManager::OnDestroyDevice()
{
	for ( int kk = 0; kk < arrTextures.GetSize(); kk++ )
	{
		LOG( L"CTextureManager::OnDestroyDevice released %s", arrTextures[ kk ]->fileName );
		SAFE_RELEASE( arrTextures[ kk ]->pTexture );
		arrTextures[ kk ]->bLoaded = false;
	}

	return K_OP_OK;
}
