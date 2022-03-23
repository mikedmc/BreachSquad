#include "dxstdafx.h"
#include "MultiSpriteLib.h"

CMultiSpriteLib::CMultiSpriteLib()
{
	m_pDevice = nullptr;
}

CMultiSpriteLib::~CMultiSpriteLib()
{
	Release();
}

OPRESULT CMultiSpriteLib::AddSprites( WCHAR* wcsFullPath, int & retLibIdx, WCHAR* nickname )
{
	retLibIdx = GetLibIndex( wcsFullPath );
	//already loaded?
	if ( retLibIdx >= 0 )
	{
		return K_OP_OK;
	}

	SpriteLibInstance *sli = new SpriteLibInstance();
	// initialize sprite lib (call reset device to set device pointer)
	sli->spriteLib.OnResetDevice( m_pDevice );
	OPRESULT err = sli->spriteLib.LoadSprites( wcsFullPath );
	if ( OP_FAILED( err ) ) 
	{
		SAFE_DELETE( sli );
		return err;
	}
	sli->shFullPath.Init( wcsFullPath );
	// init nickname
	if ( nickname != nullptr )
		sli->shNickname.Init( nickname );
	else
		sli->shNickname = sli->shFullPath;

	arrLibs.Add( sli );
	// return lib index
	retLibIdx = arrLibs.GetSize() - 1;

	return K_OP_OK;
}

void CMultiSpriteLib::Release()
{
	SAFE_DELETE_CArray( arrLibs );
}

int CMultiSpriteLib::GetLibIndex( WCHAR* wcsFullPath )
{
	UINT32 shID = HASHW( wcsFullPath );
	for ( int kk = 0; kk < arrLibs.Count(); kk++ )
	{
		if ( arrLibs[ kk ]->shFullPath.textHash == shID )
			return kk;
	}
	return -1;
}

int CMultiSpriteLib::GetLibIndexByNick( WCHAR* nickname )
{
	UINT32 shID = HASHW( nickname );
	for ( int kk = 0; kk < arrLibs.Count(); kk++ )
	{
		if ( arrLibs[ kk ]->shNickname.textHash == shID )
			return kk;
	}
	return -1;
}

CSpriteLib* CMultiSpriteLib::GetLib( int nLibIdx )
{
	if ((nLibIdx < 0) || (nLibIdx >= arrLibs.Count()))
		return nullptr;
	return &arrLibs[ nLibIdx ]->spriteLib;
}

CSpriteLib* CMultiSpriteLib::GetLibByNick( WCHAR* nickname )
{
	UINT32 shID = HASHW( nickname );
	for ( int kk = 0; kk < arrLibs.Count(); kk++ )
	{
		if ( arrLibs[ kk ]->shNickname.textHash == shID )
			return &arrLibs[kk]->spriteLib;
	}
	return nullptr;
}

OPRESULT CMultiSpriteLib::OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc /*= nullptr */ )
{
	m_pDevice = pDevice;
	for ( int kk = 0; kk < arrLibs.Count(); kk++ )
	{
		V_OP_RET( arrLibs[ kk ]->spriteLib.OnCreateDevice( pDevice, pBBDesc ) );
	}
	return K_OP_OK;
}

OPRESULT CMultiSpriteLib::OnResetDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc /*= nullptr */ )
{
	m_pDevice = pDevice;
	for ( int kk = 0; kk < arrLibs.Count(); kk++ )
	{
		V_OP_RET( arrLibs[ kk ]->spriteLib.OnResetDevice( pDevice, pBBDesc ) );
	}
	return K_OP_OK;

}

OPRESULT CMultiSpriteLib::OnLostDevice()
{
	for ( int kk = 0; kk < arrLibs.Count(); kk++ )
	{
		arrLibs[ kk ]->spriteLib.OnLostDevice();
	}
	return K_OP_OK;
}

OPRESULT CMultiSpriteLib::OnDestroyDevice()
{
	for ( int kk = 0; kk < arrLibs.Count(); kk++ )
	{
		arrLibs[ kk ]->spriteLib.OnDestroyDevice();
	}
	return K_OP_OK;
}
