#include "dxstdafx.h"
#include "spriteLib.h"

CSpriteLib::CSpriteLib( void )
{
	m_pDevice = nullptr;
	bIsLoaded = false;
}

CSpriteLib::~CSpriteLib( void )
{
	Release();
}

OPRESULT CSpriteLib::LoadSprites( WCHAR* wcsFullPath )
{
	if ( bIsLoaded )
	{
		if ( (0 == _wcsnicmp( wcsLoadedFile, wcsFullPath, MAX_PATH )) ) //already loaded
			return K_OP_OK;
		else //not loaded, release current one
			Release();
	}
	//save loaded file (relative)
	StringCchCopy( wcsLoadedFile, MAX_PATH, wcsFullPath );

	pugi::xml_document doc;
	if ( !doc.load_file( wcsFullPath ) )
	{
		return OPRESULT( K_OP_FAILED, K_SEVERITY_CRITICAL, L"Unable to load Sprites XML:%s\n", wcsFullPath );
	}

	pugi::xml_attribute ver = doc.root().child( L"SpriteCollection" ).attribute( L"Version" );
	if ( ver.as_float() != BSX_VERSION )
	{
		return OPRESULT( K_OP_FAILED, K_SEVERITY_CRITICAL, L"SpriteCollection XML wrong version:%s\n", wcsFullPath );
	}


	pugi::xml_node spritenodes = doc.root().child( L"SpriteCollection" );
	///--- load images
	WCHAR wcsFullFolder[ MAX_PATH ];
	StringCchCopy( wcsFullFolder, MAX_PATH, wcsFullPath );
	//get sprite's folder
	int nIdx = (int)wcslen( wcsFullFolder );
	while ( --nIdx > 0 && wcsFullFolder[ nIdx ] != '\\' && wcsFullFolder[ nIdx ] != '/' );
	wcsFullFolder[ nIdx + 1 ] = '\0';

	for ( pugi::xml_node imagenode = spritenodes.child( L"Image" ); imagenode; imagenode = imagenode.next_sibling( L"Image" ) )
	{
		scTexture* ntex = new scTexture();
		const WCHAR* imgname = imagenode.child_value();

#ifdef ENABLE_STEAM_WORKSHOP
		//find path relative to "media" folder so we can search for the images in the mods:
		std::wstring str_fullpath( wcsFullFolder );
		std::size_t pos = str_fullpath.find( L"media" );
		if ( pos != std::string::npos )
		{
			str_fullpath = str_fullpath.substr( pos );
		}

		WCHAR wcsImgPath[ MAX_PATH ];
		StringCchPrintf( wcsImgPath, MAX_PATH, L"%s%s", str_fullpath.c_str(), imgname );
		for ( UINT ll = 0; ll < wcslen( wcsImgPath ); ll++ )
		{
			if ( wcsImgPath[ ll ] == '\\' )
				wcsImgPath[ ll ] = '/';
		}

		FileManager::GetMediaPath( wcsImgPath, ntex->imagePath );

#else
		StringCchPrintf( ntex->imagePath, MAX_PATH, L"%s%s", wcsFullFolder, imgname );
#endif

		_ASSERT( (m_pDevice != nullptr ) && "Device shouldn't be null" );
		if ( m_pDevice != nullptr )
		{
			//tries to create textures here
			HRESULT hr = D3DXCreateTextureFromFileEx( m_pDevice, ntex->imagePath, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2,
				1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
				D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0,
				NULL, NULL, &ntex->pTex );

			if ( FAILED( hr ) )
			{
				Release();
				return OPRESULT( K_OP_FAILED, K_SEVERITY_CRITICAL, L"SpriteCollection::loadSpriteXML->createTextures\n%s", ntex->imagePath );
			}
			// get real texture size from first level of the texture
			ntex->pTex->GetLevelDesc( 0, &ntex->info );
		}
		Textures.Add( ntex );
	}
	imageNo = Textures.GetSize();
	///--- read modules
	pugi::xml_node modulesnode = spritenodes.child( L"Modules" );
	CArray<scModule*> tempModules;
	for ( pugi::xml_node moduledata = modulesnode.first_child(); moduledata; moduledata = moduledata.next_sibling() )
	{
		scModule *nmod = new scModule();
		nmod->X = moduledata.attribute( L"X" ).as_int();
		nmod->Y = moduledata.attribute( L"Y" ).as_int();
		nmod->W = moduledata.attribute( L"W" ).as_int();
		nmod->H = moduledata.attribute( L"H" ).as_int();
		nmod->imgIdx = moduledata.attribute( L"ImageIdx" ).as_int();
		tempModules.Add( nmod );
	}
	///--- reads frame modules
	pugi::xml_node fmodulesnode = spritenodes.child( L"FrameModules" );
	for ( pugi::xml_node fmoduledata = fmodulesnode.first_child(); fmoduledata; fmoduledata = fmoduledata.next_sibling() )
	{
		scFModule *nfmod = new scFModule();
		int midx = fmoduledata.attribute( L"ModuleIdx" ).as_int();
		nfmod->ox = fmoduledata.attribute( L"OX" ).as_int();
		nfmod->oy = fmoduledata.attribute( L"OY" ).as_int();
		nfmod->flags = fmoduledata.attribute( L"Flags" ).as_uint();

		nfmod->moduleXYWH.Set( tempModules[ midx ]->X, tempModules[ midx ]->Y, tempModules[ midx ]->W, tempModules[ midx ]->H );
		nfmod->imgIdx = tempModules[ midx ]->imgIdx;
		nfmod->pImg = Textures[ nfmod->imgIdx ];
		//compute and save tex coords
		int texW = Textures[ nfmod->imgIdx ]->info.Width;
		int texH = Textures[ nfmod->imgIdx ]->info.Height;

		nfmod->texRect.left = nfmod->moduleXYWH.x / (float)texW;
		nfmod->texRect.top = nfmod->moduleXYWH.y / (float)texH;
		nfmod->texRect.right = (nfmod->moduleXYWH.x + nfmod->moduleXYWH.w) / (float)texW;
		nfmod->texRect.bottom = (nfmod->moduleXYWH.y + nfmod->moduleXYWH.h) / (float)texH;

		// set module rect moved by ox/oy (by default painted in 0,0 top left corner)
		nfmod->moduleRectOff.Set( nfmod->ox, nfmod->oy, nfmod->moduleXYWH.w + nfmod->ox, nfmod->moduleXYWH.h + nfmod->oy );

		//set RECT for painting
		SetRect( &nfmod->moduleRect, nfmod->moduleXYWH.x, nfmod->moduleXYWH.y, nfmod->moduleXYWH.x + nfmod->moduleXYWH.w, nfmod->moduleXYWH.y + nfmod->moduleXYWH.h );
		FModules.Add( nfmod );
	}
	fmoduleNo = FModules.GetSize();
	//release tempmodules
	for ( int kk = 0; kk < tempModules.GetSize(); kk++ )
	{
		SAFE_DELETE( tempModules[ kk ] );
	}
	tempModules.RemoveAll();
	///--- load temp frames
	pugi::xml_node framesnode = spritenodes.child( L"Frames" );
	CArray<scFrame*> tempFrames;
	for ( pugi::xml_node framedata = framesnode.first_child(); framedata; framedata = framedata.next_sibling() )
	{
		scFrame *nfrm = new scFrame();
		nfrm->BBox.x = framedata.attribute( L"BBoxX" ).as_int();
		nfrm->BBox.y = framedata.attribute( L"BBoxY" ).as_int();
		nfrm->BBox.w = framedata.attribute( L"BBoxW" ).as_int();
		nfrm->BBox.h = framedata.attribute( L"BBoxH" ).as_int();
		//lista de FModules
		nfrm->frame_fmodulesNo = 0;
		nfrm->frame_fmodulesIdx = nullptr;
		pugi::xml_node fmodulesn = framedata.child( L"FModules" );
		int tempicnt = 0;
		for ( pugi::xml_node fmodnode = fmodulesn.first_child(); fmodnode; fmodnode = fmodnode.next_sibling() )
		{
			tempicnt++;
		}
		if ( tempicnt > 0 )
		{
			nfrm->frame_fmodulesNo = tempicnt;
			nfrm->frame_fmodulesIdx = new int[ tempicnt ];
			int cur = 0;
			for ( pugi::xml_node fmodnode = fmodulesn.first_child(); fmodnode; fmodnode = fmodnode.next_sibling() )
			{
				_ASSERT( cur < tempicnt );
				nfrm->frame_fmodulesIdx[ cur++ ] = fmodnode.attribute( L"Idx" ).as_int();
			}
		}
		//lista de hitpoints
		nfrm->frame_hitPtsNo = 0;
		nfrm->frame_hitPtsPosXYF = nullptr;
		pugi::xml_node fptsn = framedata.child( L"Points" );
		tempicnt = 0;
		for ( pugi::xml_node fptsnode = fptsn.first_child(); fptsnode; fptsnode = fptsnode.next_sibling() )
		{
			tempicnt++;
		}
		if ( tempicnt > 0 )
		{
			nfrm->frame_hitPtsNo = tempicnt;
			nfrm->frame_hitPtsPosXYF = new int[ tempicnt * 3 ];
			int cur = 0;
			for ( pugi::xml_node fptsnode = fptsn.first_child(); fptsnode; fptsnode = fptsnode.next_sibling() )
			{
				_ASSERT( cur < tempicnt * 3 );
				nfrm->frame_hitPtsPosXYF[ cur++ ] = fptsnode.attribute( L"X" ).as_int();
				nfrm->frame_hitPtsPosXYF[ cur++ ] = fptsnode.attribute( L"Y" ).as_int();
				nfrm->frame_hitPtsPosXYF[ cur++ ] = fptsnode.attribute( L"Flags" ).as_int();
			}
		}

		tempFrames.Add( nfrm );
	}

	///--- load AFrames
	pugi::xml_node faframesnode = spritenodes.child( L"AnimationFrames" );
	for ( pugi::xml_node faframedata = faframesnode.first_child(); faframedata; faframedata = faframedata.next_sibling() )
	{
		scAFrame *naframe = new scAFrame();
		int idx = faframedata.attribute( L"FrameIdx" ).as_int();
		naframe->duration = faframedata.attribute( L"Duration" ).as_int();
		naframe->dx = faframedata.attribute( L"DX" ).as_int();
		naframe->dy = faframedata.attribute( L"DY" ).as_int();
		naframe->flags = faframedata.attribute( L"Flags" ).as_uint();
		//scrie datele din frame
		naframe->fmodulesNo = tempFrames[ idx ]->frame_fmodulesNo;
		if ( naframe->fmodulesNo > 0 )
		{
			naframe->fmodulesIdx = new int[ naframe->fmodulesNo ];
			memcpy( naframe->fmodulesIdx, tempFrames[ idx ]->frame_fmodulesIdx, sizeof( int ) * naframe->fmodulesNo );
		}
		else
			naframe->fmodulesIdx = nullptr;

		naframe->PointsNo = tempFrames[ idx ]->frame_hitPtsNo;
		if ( naframe->PointsNo > 0 )
		{
			naframe->PointsXYFlag = new int[ naframe->PointsNo * 3 ];
			memcpy( naframe->PointsXYFlag, tempFrames[ idx ]->frame_hitPtsPosXYF, sizeof( int ) * naframe->PointsNo * 3 );
		}
		else
			naframe->PointsXYFlag = nullptr;

		naframe->BBox.Set( tempFrames[ idx ]->BBox.x, tempFrames[ idx ]->BBox.y, tempFrames[ idx ]->BBox.w, tempFrames[ idx ]->BBox.h );
		//aici calculeaza si BBox-ul real, de obicei folosit la desenare sa stiu cand e in afara ecranului
		int minx = 100000, miny = 100000, maxx = -100000, maxy = -100000;
		for ( int kk = 0; kk < naframe->fmodulesNo; kk++ )
		{
			minx = min( minx, FModules[ naframe->fmodulesIdx[ kk ] ]->ox );
			miny = min( miny, FModules[ naframe->fmodulesIdx[ kk ] ]->oy );
			maxx = max( maxx, (FModules[ naframe->fmodulesIdx[ kk ] ]->ox + FModules[ naframe->fmodulesIdx[ kk ] ]->moduleXYWH.w) );
			maxy = max( maxy, (FModules[ naframe->fmodulesIdx[ kk ] ]->oy + FModules[ naframe->fmodulesIdx[ kk ] ]->moduleXYWH.h) );
		}
		naframe->BBox_real.Set( minx, miny, maxx - minx, maxy - miny );

		AFrames.Add( naframe );
	}
	aframesNo = AFrames.GetSize();

	//delete temp frames
	for ( int kk = 0; kk < tempFrames.GetSize(); kk++ )
	{
		SAFE_DELETE_ARRAY( tempFrames[ kk ]->frame_fmodulesIdx );
		SAFE_DELETE_ARRAY( tempFrames[ kk ]->frame_hitPtsPosXYF );
		SAFE_DELETE( tempFrames[ kk ] );
	}
	tempFrames.RemoveAll();

	///--- load anims
	pugi::xml_node animsnode = spritenodes.child( L"Animations" );
	for ( pugi::xml_node animdata = animsnode.first_child(); animdata; animdata = animdata.next_sibling() )
	{
		scAnimation *nanim = new scAnimation();

		UINT stringLen;
		StringCchLength( animdata.attribute( L"ID" ).value(), MAX_PATH, &stringLen );
		if ( stringLen > 0 )
		{
			nanim->animName.Init( animdata.attribute( L"ID" ).value() );

			// on debug check anim name hash collisions
#if defined(_DEBUG) || defined(DEBUG)
			for ( int jj = 0; jj < Animations.GetSize(); jj++ )
				if ( Animations[ jj ]->animName == nanim->animName )
					ErrorBox( K_ERR_WARNING, L"HashCollision found in SpriteCollection!\nname:%s", Animations[ jj ]->animName.text );
#endif			
		}
		else
		{
			ErrorBox( K_ERR_WARNING, L"Animation ID empty in file: %s", wcsFullPath );
		}

		nanim->flags = animdata.attribute( L"Flags" ).as_uint();
		//lista de FModules
		nanim->aframesNo = 0;
		nanim->aframesIdx = nullptr;
		int tempicnt = 0;
		for ( pugi::xml_node animnode = animdata.first_child(); animnode; animnode = animnode.next_sibling() )
		{
			tempicnt++;
		}
		if ( tempicnt > 0 )
		{
			nanim->aframesNo = tempicnt;
			nanim->aframesIdx = new int[ tempicnt ];
			int cur = 0;
			for ( pugi::xml_node animnode = animdata.first_child(); animnode; animnode = animnode.next_sibling() )
			{
				_ASSERT( cur < tempicnt );
				nanim->aframesIdx[ cur++ ] = int( animnode.attribute( L"Idx" ).as_int() );
			}
		}

		Animations.Add( nanim );
	}
	animationNo = Animations.GetSize();

	bIsLoaded = true;

	LOG_DBG( L"SpriteCollection:: Loaded: %s", wcsFullPath );

	return K_OP_OK;
}


void CSpriteLib::Release()
{
	for ( int kk = 0; kk < Textures.GetSize(); kk++ )
	{
		SAFE_RELEASE( Textures[ kk ]->pTex );
		SAFE_DELETE( Textures[ kk ] );
	}
	Textures.RemoveAll();

	for ( int kk = 0; kk < Animations.GetSize(); kk++ )
	{
		SAFE_DELETE_ARRAY( Animations[ kk ]->aframesIdx );
		SAFE_DELETE( Animations[ kk ] );
	}
	Animations.RemoveAll();

	for ( int kk = 0; kk < AFrames.GetSize(); kk++ )
	{
		SAFE_DELETE_ARRAY( AFrames[ kk ]->fmodulesIdx );
		SAFE_DELETE_ARRAY( AFrames[ kk ]->PointsXYFlag );
		SAFE_DELETE( AFrames[ kk ] );
	}
	AFrames.RemoveAll();

	for ( int kk = 0; kk < FModules.GetSize(); kk++ )
	{
		SAFE_DELETE( FModules[ kk ] );
	}
	FModules.RemoveAll();

	bIsLoaded = false;
	wcsLoadedFile[ 0 ] = 0;
}

//--- utilities ---

//RETURNS: animIdx or -1 if animation wasn't found
int CSpriteLib::GetAnimationIdxByName( const CHAR* animName )
{
	UINT32 strHash = FastHash( animName, strlen( animName ) );

	for ( int kk = 0; kk < Animations.GetSize(); kk++ )
	{
		if ( Animations[ kk ]->animName.textHash == strHash )
			return kk;
	}

	return -1;
}

//RETURNS: animIdx or -1 if animation wasn't found
int CSpriteLib::GetAnimationIdxByName( const WCHAR* animName )
{
	UINT32 strHash = FastHash( animName, wcslen( animName ) );

	for ( int kk = 0; kk < Animations.GetSize(); kk++ )
	{
		if ( Animations[ kk ]->animName.textHash == strHash )
			return kk;
	}

	return -1;
}

/**
 * Gets an animation index from animation name hash
 *
 * @return animation index or -1 if not found
 */
int CSpriteLib::GetAnimationIdxByNameHash( const UINT32 animNameHash )
{
	for ( int kk = 0; kk < Animations.GetSize(); kk++ )
	{
		if ( Animations[ kk ]->animName.textHash == animNameHash )
			return kk;
	}

	return -1;
}

int CSpriteLib::GetAFrameHitPointsCntFlag( int animIdx, int frameIdx, DWORD flagFilter )
{
	if ( animIdx >= Animations.GetSize() )
		return 0;
	if ( frameIdx >= Animations[ animIdx ]->aframesNo )
		return 0;
	int flcnt = 0;
	for ( int kk = 0; kk < AFrames[ Animations[ animIdx ]->aframesIdx[ frameIdx ] ]->PointsNo; kk++ )
	{
		if ( (AFrames[ Animations[ animIdx ]->aframesIdx[ frameIdx ] ]->PointsXYFlag[ kk * 3 + 2 ] & flagFilter) != 0 )
			flcnt++;
	}
	return flcnt;
}

int CSpriteLib::GetAFrameHitPointsCnt( int animIdx, int frameIdx )
{
	if ( animIdx >= Animations.GetSize() )
		return 0;
	if ( frameIdx >= Animations[ animIdx ]->aframesNo )
		return 0;
	return AFrames[ Animations[ animIdx ]->aframesIdx[ frameIdx ] ]->PointsNo;
}

bool CSpriteLib::GetAFrameHitPoint( int animIdx, int frameIdx, int pointIdx, Vec3i *outvar )
{
	if ( pointIdx >= AFrames[ Animations[ animIdx ]->aframesIdx[ frameIdx ] ]->PointsNo )
	{
		//ErrorBox(K_ERR_WARNING, L"Frame %d of anim %d doesn't have enough HitPoints!\n", frameIdx, animIdx);
		if ( outvar )
		{
			outvar->x = outvar->y = outvar->z = 0;
		}
		return false;
	}
	if ( outvar )
	{
		outvar->x = AFrames[ Animations[ animIdx ]->aframesIdx[ frameIdx ] ]->PointsXYFlag[ pointIdx * 3 ];
		outvar->y = AFrames[ Animations[ animIdx ]->aframesIdx[ frameIdx ] ]->PointsXYFlag[ pointIdx * 3 + 1 ];
		outvar->z = AFrames[ Animations[ animIdx ]->aframesIdx[ frameIdx ] ]->PointsXYFlag[ pointIdx * 3 + 2 ];
	}
	return true;
}

bool CSpriteLib::GetAFrameHitPointFlag( int animIdx, int frameIdx, int pointIdx, DWORD flagFilter, Vec3i *outvar )
{
	if ( outvar == nullptr )
	{
		ErrorBox( K_ERR_DEBUGOUT, L"GetAframeHitPointFlag outvar param is NULL!" );
		return false;
	}
	//init on 0
	outvar->x = outvar->y = outvar->z = 0;

	if ( pointIdx >= AFrames[ Animations[ animIdx ]->aframesIdx[ frameIdx ] ]->PointsNo )
	{
		return true;
	}

	scAFrame* aframe = AFrames[ Animations[ animIdx ]->aframesIdx[ frameIdx ] ];
	int skip = pointIdx;
	for ( int kk = 0; kk < aframe->PointsNo; kk++ )
	{
		if ( (aframe->PointsXYFlag[ kk * 3 + 2 ] & flagFilter) != 0 )
		{
			if ( skip == 0 )
			{
				outvar->x = aframe->PointsXYFlag[ kk * 3 ];
				outvar->y = aframe->PointsXYFlag[ kk * 3 + 1 ];
				outvar->z = aframe->PointsXYFlag[ kk * 3 + 2 ];
				return true;
			}
			else
			{
				//aici salveaza datele ultimului punct gasit ca atunci cand vrei punctul 2 si nu ai atatea sa ti-l dea pe ultimul (util la DOOR ca e doar una)
				outvar->x = aframe->PointsXYFlag[ kk * 3 ];
				outvar->y = aframe->PointsXYFlag[ kk * 3 + 1 ];
				outvar->z = aframe->PointsXYFlag[ kk * 3 + 2 ];
				skip--;
			}
		}
	}
	return false;
}

RectXYWHi CSpriteLib::GetModuleRect( int animIdx, int frameIdx, int moduleIdx )
{
#if defined(_DEBUG) || defined(DEBUG)
	if ( (animIdx >= Animations.GetSize()) || (frameIdx >= Animations[ animIdx ]->aframesNo) || (moduleIdx >= AFrames[ Animations[ animIdx ]->aframesIdx[ frameIdx ] ]->fmodulesNo) )
	{
		ErrorBox( K_ERR_WARNING, L"GetModuleRect -> out of bounds!" );
		return RectXYWHi( 0, 0, 0, 0 );
	}
#endif;
	return FModules[ AFrames[ Animations[ animIdx ]->aframesIdx[ frameIdx ] ]->fmodulesIdx[ moduleIdx ] ]->moduleXYWH;
}

RectLTRB CSpriteLib::GetModuleRect_TexCoords( int animIdx, int frameIdx, int moduleIdx )
{
#if defined(_DEBUG) || defined(DEBUG)
	if ( (animIdx >= Animations.GetSize()) || (frameIdx >= Animations[ animIdx ]->aframesNo) || (moduleIdx >= AFrames[ Animations[ animIdx ]->aframesIdx[ frameIdx ] ]->fmodulesNo) )
	{
		ErrorBox( K_ERR_WARNING, L"GetModuleRect -> out of bounds!" );
		return RectLTRB( 0.0f, 0.0f, 0.0f, 0.0f );
	}
#endif;
	scFModule *module = FModules[ AFrames[ Animations[ animIdx ]->aframesIdx[ frameIdx ] ]->fmodulesIdx[ moduleIdx ] ];
	return module->texRect;
}


SizeWHi CSpriteLib::GetTextureSizeByAnim( int animIdx )
{
#if defined(_DEBUG) || defined(DEBUG)
	if ( animIdx >= Animations.GetSize() )
	{
		ErrorBox( K_ERR_WARNING, L"GetTextureSizeByAnim -> out of bounds!" );
		return SizeWHi( -1, -1 );
	}
#endif;
	scFModule *module = FModules[ AFrames[ Animations[ animIdx ]->aframesIdx[ 0 ] ]->fmodulesIdx[ 0 ] ];
	return SizeWHi( module->pImg->info.Width, module->pImg->info.Height );
}

scTexture* CSpriteLib::GetTextureByAnim( int animIdx, int frameIdx, int moduleIdx )
{
#if defined(_DEBUG) || defined(DEBUG)
	if ( (animIdx >= Animations.GetSize()) || (frameIdx >= Animations[ animIdx ]->aframesNo) || (moduleIdx >= AFrames[ Animations[ animIdx ]->aframesIdx[ frameIdx ] ]->fmodulesNo) )
	{
		ErrorBox( K_ERR_WARNING, L"GetTextureByAnim -> out of bounds!" );
		return nullptr;
	}
#endif;
	scFModule *module = FModules[ AFrames[ Animations[ animIdx ]->aframesIdx[ 0 ] ]->fmodulesIdx[ 0 ] ];
	return Textures[ module->imgIdx ];
}

inline bool CSpriteLib::IsLooping( int animIdx )
{
	return ((Animations[ animIdx ]->flags & K_EDITOR_ANIMATION_FLAG_LOOPED) != 0);
}

//-=-=-= SYSTEM / FRAMEWORK =-=-=-
OPRESULT CSpriteLib::OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc )
{
	m_pDevice = pDevice;
	// Reload textures when device is lost
	for ( int kk = 0; kk < Textures.GetSize(); kk++ )
	{
		scTexture *ntex = Textures[ kk ];

		HRESULT hr = D3DXCreateTextureFromFileEx( m_pDevice, ntex->imagePath, D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2,
			1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED,
			D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0,
			NULL, NULL, &ntex->pTex );

		if ( FAILED( hr ) )
		{
			return OPRESULT( K_OP_FAILED, K_SEVERITY_CRITICAL, L"SpriteCollection::OnCreateDevice->createTextures\n%s", ntex->imagePath );
		}
		// get real texture size from first level of the texture
		ntex->pTex->GetLevelDesc( 0, &ntex->info );
	}

	return K_OP_OK;
}

OPRESULT CSpriteLib::OnResetDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc )
{
	m_pDevice = pDevice;
	return K_OP_OK;
}

OPRESULT CSpriteLib::OnLostDevice( void )
{
	m_pDevice = nullptr;
	return K_OP_OK;
}

OPRESULT CSpriteLib::OnDestroyDevice( void )
{
	m_pDevice = nullptr;
	// on losing the device textures are deallocated
	for ( int kk = 0; kk < Textures.GetSize(); kk++ )
	{
		SAFE_RELEASE( Textures[ kk ]->pTex );
	}

	return K_OP_OK;
}

