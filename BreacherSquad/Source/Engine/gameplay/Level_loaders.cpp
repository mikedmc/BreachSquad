#include "dxstdafx.h"
using namespace std;

OPRESULT CLevel::LoadLevel_Static( WCHAR * strPathAbs )
{
	//set last ID on a number that will never get reached from the editor or by adding areas
	m_unLastID = 10000000; // #TODO: read max from level
	int nChapterNumber = g_userData[K_MEMID_SELECTED_CHAPTER];
	int nLevelNumber = g_userData[K_MEMID_SELECTED_LEVEL];
	//--- set loaded level flags
	m_unLoadedLevelFlags = K_LVL_LEVEL_FLAG_NONE;

	WCHAR Path[MAX_PATH] = { 0 };

	if ( UTApp().IsGameNetworked() )
	{
		m_rand.SetRandSeed( g_netlock.m_unRandomSeed );
	}
	else
	{
		//randomize seed
		m_rand.SetRandSeed( GetTickCount() );
	}
	//reset local timeline
	fLocalTimeline = 0.0f;
	vLastSpawnPoint = Vec2( 0.0f, 0.0f );

	//realease level if loaded
	Release();

	//reset shakes
	m_camLevelToRT.ShakeScreen( 0.0f, 0.0f );
	m_levelAABB.Set( 0, 0, 0, 0 );
	//reset all timers
	m_Timers.ResetTimers();

	m_bLoaded = false;
	m_bOneUpdateDone = false;

	m_dwSyncCheckHash = 0;
	//get rid of all particles
	__Particles().ClearParticles();
	m_arrActors.Init( K_LVL_ACTORS_POOL_SIZE );
	//--- setari initiale ---
	ResetLevelStatistics();

	m_colAmbientGlobal = 0xffffffff;

	//load interface sprites
	FileManager::GetMediaPath( L"media/interfaces/igm_interface.bsx", Path );
	V_OP_RET( m_sprInterface.LoadSprites( Path ) );

	//tileset name
	CHAR charArr[MAX_PATH]{ 0 };
	WCHAR wcharArr[MAX_PATH]{ 0 };
	WCHAR wcsMediaAddr[MAX_PATH]{ 0 };

	// Loads level defines (generic data like actions, inventory, etc)
	FileManager::GetMediaPath( L"media/gameplaydef.xml", Path );
	V_OP_RET( LoadLevelDefines( Path ) );
	// Load tileset (includes water and other dependencies)
	FileManager::GetMediaPath( L"media/levels/data/tileset1.xml", Path );
	V_OP_RET( LoadTileset( Path, m_tilesetDesc ) );

	//LIGHTS
	int libidxtmp = -1;
	FileManager::GetMediaPath( L"media/levels/data/lights.bsx", Path );
	V_OP_RET( m_sprLib.AddSprites( Path, libidxtmp, K_LIBNICK_LIGHTS ) );

	//load bsx
	FileManager::GetMediaPath( L"media/levels/data/objects.bsx", Path );
	V_OP_RET( m_sprLib.AddSprites( Path, libidxtmp, K_LIBNICK_PROPS ) );
	//bullets
	FileManager::GetMediaPath( L"media/levels/data/bullets.bsx", Path );
	V_OP_RET( m_sprLib.AddSprites( Path, libidxtmp, K_LIBNICK_BULLETS ) );

	//--- load actors templates and weaponry right after props sprite ---
	FileManager::GetMediaPath( L"media/levels/data/weapons/weapons_data.xml", Path );
	V_OP_RET( LoadWeaponTemplates( Path ) );

	UINT32 level_rand_seed = 1000000 + randint( 9999999 );
	LOG( L"StaticLevel RndSeed: %lu", level_rand_seed );

	///--- LOAD single area from a level (should have 0 connectors) ---
	V_OP_RET( DeployAreaInstance( m_pDevice, strPathAbs, 0, Vec2i(0, 0) ) );
	// set areas neighbour pointers
	for ( int ii = 0; ii < m_arrAreas.GetSize(); ii++ )
	{
		// order in m_arrAreas SHOULD correspond to the order in m_arrPlaced if area loading didn't fail
		CLevelArea* plarea = m_arrAreas[ii];
		plarea->arrNeighbours.Clear();
		// enlarge level area and other level data
		m_levelAABB.Union( plarea->AABBbounds.to_RECTXYWH_F() );
	}
	// set level aabb in tiles too
	m_levelAABB_TL.Set( (int)floor( m_levelAABB.x / K_TILE_SIZE ), (int)floor( m_levelAABB.y / K_TILE_SIZE ), (int)( m_levelAABB.w / K_TILE_SIZE ), (int)( m_levelAABB.h / K_TILE_SIZE ) );
	// allocate passability map
	_ASSERT( m_levelAABB_TL.w < 5000 && m_levelAABB_TL.h < 5000 );

	// initialize AStar search engine
	m_astar.Init( m_levelAABB_TL.w, m_levelAABB_TL.h, COL_MOVEMENT_BLOCK );

	///--- everything loaded, SetAI here again so it sets all necessary pointers ---
	// set AI at the end after we load everything or we won't have final targets for pointers
	for ( int kk = 0; kk < m_arrLights.GetSize(); kk++ )
	{
		CLight * light = m_arrLights[kk];
		IActiveInterface* pt = GetIActiveInterfacePtr( light->targetID_ini );
		if ( pt )
		{
			CSmartLink::SetLink( &light->pTarget, pt );
		}
		light->SetAI( light->AIstate );
	}
	for ( int kk = 0; kk < m_arrColShapes.GetSize(); kk++ )
	{
		CCollisionShape * shape = m_arrColShapes[kk];
		IActiveInterface* pt = GetIActiveInterfacePtr( shape->targetID_ini );
		if ( pt )
		{
			CSmartLink::SetLink( &shape->pTarget, pt );
		}
		shape->SetAI( shape->AIstate );
	}

	for ( int ar = 0; ar < m_arrAreas.Count(); ar++ )
	{
		CLevelArea* area = m_arrAreas[ar];
		for ( int kk = 0; kk < area->m_arrProps.GetSize(); kk++ )
		{
			CProp * activ = area->m_arrProps[kk];
			IActiveInterface* pt = GetIActiveInterfacePtr( activ->targetID_ini );
			if ( pt )
			{
				CSmartLink::SetLink( &activ->pTarget, pt );
			}
			activ->SetAI( activ->AIstate );
		}
	}
	for ( auto node : m_arrActors )
	{
		CActor* actor = &node->m_data;
		IActiveInterface* pt = GetIActiveInterfacePtr( actor->targetID_ini );
		if ( pt )
			CSmartLink::SetLink( &actor->pTarget, pt );
	}

	//-- find spawn point
	for ( auto miscobj : m_arrMiscObjects )
	{
		if ( miscobj->type == K_LVL_MISC_SPAWNPOINT )
		{
			CMiscObject_Spawnpoint* spawnobj = static_cast<CMiscObject_Spawnpoint*>( miscobj );
			vLastSpawnPoint = spawnobj->pos;
			break;
		}
	}
	///--- camera ---
	//target
	m_camTargetActive = null; //cand nu am target se uita dupa players
	m_camTargetOld = null;
	m_vCamPosDefault = vLastSpawnPoint;
	//level to RT cam settings
	m_camLevelToRT.SetWorldBounds( m_levelAABB, false, K_CAMTRANS_AXIS_NONE );
	m_camLevelToRT.InitCamera( UTApp().g_rectRT, K_GAME_HEIGHT, K_CAMTRANS_AXIS_V, m_vCamPosDefault );
	m_camLevelToRT.SetCamAnimationSpring( K_LVL_CAM_FOLLOW_SPRING_KS, K_LVL_CAM_FOLLOW_DAMPING_KD );
	// level to screen cam settings (copies position of level to RT)
	m_camLevelToScr.SetWorldBounds( m_levelAABB, false, K_CAMTRANS_AXIS_NONE );
	m_camLevelToScr.InitCamera( UTApp().g_rectRenderPP, K_GAME_HEIGHT, K_CAMTRANS_AXIS_V, m_vCamPosDefault );
	m_camLevelToScr.SetCamAnimationNone();
	// call one update so we're sure everything is initialized
	m_camLevelToRT.Update( 0.0f );
	m_camLevelToScr.Update( 0.0f );
	//pools
	//m_poolPhysPts.Init( K_LVL_PHYSP_MAX_CNT );
	m_poolDoofers.Init( K_LVL_DOOFERS_MAX_CNT );
	m_poolBullets.Init( K_LVL_BULLETS_MAX_CNT );

	//spawn selected players
	m_nPlayers = 0;
	for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
	{
		//trecem controller instanceIDs in arr local din level
		m_arrPlayerControllersIIDs[kk] = g_playerSelScr.m_arrPlayers[kk].nInstanceID;
		//set selected 
		m_arrPlayerSelHotJoin[kk] = -1;
		m_arrPlayerSelStrategic[kk] = -1;
		// we have selected player
		if ( g_playerSelScr.m_arrPlayers[kk].bSelected )
		{
			//spawn Player aloca si controllerul potrivit
			int offx = ( ( kk * 2 ) - 1 ) * K_TILE_HSIZE;
			SpawnPlayer( vLastSpawnPoint + Vec2( (float)offx, 0.0f ), kk, -1 );
			//resolve selection
			m_arrPlayerSelHotJoin[kk] = (int)g_playerSelScr.m_arrPlayers[kk].eType;
		}
		else
		{
			m_arrPlayerControllersIIDs[kk] = -1; //allow hot join
		}
	}

	// release mission generator data
	__MissionGen().Release();
	//clear global script memory (per level instance)
	__Scripts().ClearGlobalMemory();
	//reset time multiplier
	SetTimeMultiplier( 1.0f, 0.0f );
	// compute dirty rects (collisions and walls, wall shadows and other data)
	UpdateDirtyRects();
	// create Area meshes after shadows have been computed in UpdateDirtyRects
	for ( auto area : m_arrAreas ) {
		V_OP_RET( area->BuildBuffers() );
	}

	BuildVisibilityLists();
	//save type of loaded mission
	m_nLoadedLevelType = 0;

	// initialize IGM interface after everything has loaded
	// the interface will use the RT resolution, scaling to real screen
	m_interfaceIGM.Init( this, &UTApp().g_camRTScreen );

	///--- LAST THINGS ---
	//called after characters spawning
	SetLevelState( K_LVL_STATE_PLAYING );

	m_bLoaded = true;

	/*
	LOG(L"Game:: Level loaded:[%s] net.randcheck[%d]", strPathAbs, m_rand.RandInt(60000));

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	LOG(L"Game:: Total Targets:[%d] Hostages:[%d]", m_arrStats[K_LVL_STATS_TARGETS_TOTAL], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);
#endif
*/

	return K_OP_OK;
}


OPRESULT CLevel::LoadLevel_GenerateFromStory()
{
	//set last ID on a number that will never get reached from the editor or by adding areas
	m_unLastID = 10000000; // #TODO: read max from level
	int nChapterNumber = g_userData[K_MEMID_SELECTED_CHAPTER];
	int nLevelNumber = g_userData[K_MEMID_SELECTED_LEVEL];
	//--- set loaded level flags
	//are we loading a downloaded level?
	int nModIdx_SelectedContent = g_userData[K_MEMID_MOD_DWNLVL_SELECTED];
	m_unLoadedLevelFlags = K_LVL_LEVEL_FLAG_NONE;
	if ( nModIdx_SelectedContent >= 0 )
		m_unLoadedLevelFlags |= K_LVL_LEVEL_FLAG_DOWNLOADED;
	if ( UTApp().IsGameModded() )
		m_unLoadedLevelFlags |= K_LVL_LEVEL_FLAG_MODS_ON;

	WCHAR Path[MAX_PATH] = { 0 };

	if ( UTApp().IsGameNetworked() )
	{
		m_rand.SetRandSeed( g_netlock.m_unRandomSeed );
	}
	else
	{
		//randomize seed
		m_rand.SetRandSeed( GetTickCount() );
	}
	//reset local timeline
	fLocalTimeline = 0.0f;
	vLastSpawnPoint = Vec2( 0.0f, 0.0f );

	//realease level if loaded
	Release();

	//reset shakes
	m_camLevelToRT.ShakeScreen( 0.0f, 0.0f );
	m_levelAABB.Set( 0, 0, 0, 0 );
	//reset all timers
	m_Timers.ResetTimers();

	m_bLoaded = false;
	m_bOneUpdateDone = false;

	m_dwSyncCheckHash = 0;
	//get rid of all particles
	__Particles().ClearParticles();
	m_arrActors.Init( K_LVL_ACTORS_POOL_SIZE );
	//--- setari initiale ---
	ResetLevelStatistics();

	m_colAmbientGlobal = 0xffffffff;

	//load interface sprites
	FileManager::GetMediaPath( L"media/interfaces/igm_interface.bsx", Path );
	V_OP_RET( m_sprInterface.LoadSprites( Path ) );

	//tileset name
	CHAR charArr[MAX_PATH]{ 0 };
	WCHAR wcharArr[MAX_PATH]{ 0 };
	WCHAR wcsMediaAddr[MAX_PATH]{ 0 };

	// Loads level defines (generic data like actions, inventory, etc)
	FileManager::GetMediaPath( L"media/gameplaydef.xml", Path );
	V_OP_RET( LoadLevelDefines( Path ) );
	// Load tileset (includes water and other dependencies)
	FileManager::GetMediaPath( L"media/levels/data/tileset1.xml", Path );
	V_OP_RET( LoadTileset(Path, m_tilesetDesc) );

	//LIGHTS
	int libidxtmp = -1;
	FileManager::GetMediaPath( L"media/levels/data/lights.bsx", Path );
	V_OP_RET( m_sprLib.AddSprites( Path, libidxtmp, K_LIBNICK_LIGHTS ) );

	//load bsx
	FileManager::GetMediaPath( L"media/levels/data/objects.bsx", Path );
	V_OP_RET( m_sprLib.AddSprites( Path, libidxtmp, K_LIBNICK_PROPS ) );

	//--- load actors templates and weaponry right after props sprite ---
	FileManager::GetMediaPath( L"media/levels/data/weapons/weapons_data.xml", Path );
	V_OP_RET( LoadWeaponTemplates( Path ) );

	///--- level areas inventory ---
	//loading story
	FileManager::GetMediaPath( L"media/levels/stories/story_small.story", Path );
	V_OP_RET( m_story.LoadStory( Path ) );

	UINT32 level_rand_seed = 1000000 + randint(9999999);
	LOG( L"Level RndSeed: %lu", level_rand_seed );
	// build inventory and generate level
	__MissionGen().BuildAreasInventory();
	//if (!__MissionGen().GenerateLevelFromStory(&m_story))
		//return K_OP_FAILED;
	if ( !__MissionGen().GenerateLevelRandomly( 3, level_rand_seed ) )
		return K_OP_FAILED;

	///--- LOAD AREAS:
	for ( auto area : __MissionGen().m_arrPlaced )
	{
		WCHAR tmppath[MAX_PATH];
		swprintf_s( tmppath, MAX_PATH, L"media/levels/areas/%s", area->strAreaFile.c_str() );
		FileManager::GetMediaPath( tmppath, Path );
		V_OP_RET( DeployAreaInstance(m_pDevice, Path, area->nID , Vec2i( area->AABB.x * K_LGEN_BLOCK_W, area->AABB.y * K_LGEN_BLOCK_H ) ) );
	}
	// set areas neighbour pointers
	for ( int ii = 0; ii < m_arrAreas.GetSize(); ii++ )
	{
		// order in m_arrAreas SHOULD correspond to the order in m_arrPlaced if area loading didn't fail
		CLevelArea* plarea = m_arrAreas[ii];
		// get the same area description from missions generator and find neighbours
		CPlacedArea* srcarea = __MissionGen().m_arrPlaced[ii];
		for ( const auto& conn : srcarea->arrConnections )
		{
			CLevelArea* neigh = Areas_GetByID( conn.pConnectedArea->nID );
			_ASSERT( neigh != nullptr );
			plarea->arrNeighbours.Add( neigh );
		}
		// enlarge level area and other level data
		m_levelAABB.Union( plarea->AABBbounds.to_RECTXYWH_F() );
	}
	// set level aabb in tiles too
	m_levelAABB_TL.Set( (int)floor( m_levelAABB.x / K_TILE_SIZE ), (int)floor( m_levelAABB.y / K_TILE_SIZE ), (int)( m_levelAABB.w / K_TILE_SIZE ), (int)( m_levelAABB.h / K_TILE_SIZE ) );
	// allocate passability map
	_ASSERT( m_levelAABB_TL.w < 5000 && m_levelAABB_TL.h < 5000 );

	// initialize AStar search engine
	m_astar.Init( m_levelAABB_TL.w, m_levelAABB_TL.h, COL_MOVEMENT_BLOCK );

	///--- everything loaded, SetAI here again so it sets all necessary pointers ---
	// set AI at the end after we load everything or we won't have final targets for pointers
	for ( int kk = 0; kk < m_arrLights.GetSize(); kk++ )
	{
		CLight * light = m_arrLights[kk];
		IActiveInterface* pt = GetIActiveInterfacePtr( light->targetID_ini );
		if ( pt )
		{
			CSmartLink::SetLink( &light->pTarget, pt );
		}
		light->SetAI( light->AIstate );
	}
	for ( int kk = 0; kk < m_arrColShapes.GetSize(); kk++ )
	{
		CCollisionShape * shape = m_arrColShapes[kk];
		IActiveInterface* pt = GetIActiveInterfacePtr( shape->targetID_ini );
		if ( pt )
		{
			CSmartLink::SetLink( &shape->pTarget, pt );
		}
		shape->SetAI( shape->AIstate );
	}

	for ( int ar = 0; ar < m_arrAreas.Count(); ar++ )
	{
		CLevelArea* area = m_arrAreas[ar];
		for ( int kk = 0; kk < area->m_arrProps.GetSize(); kk++ )
		{
			CProp * activ = area->m_arrProps[kk];
			IActiveInterface* pt = GetIActiveInterfacePtr( activ->targetID_ini );
			if ( pt )
			{
				CSmartLink::SetLink( &activ->pTarget, pt );
			}
			activ->SetAI( activ->AIstate );
		}
	}
	for ( auto node : m_arrActors )
	{
		CActor* actor = &node->m_data;
		IActiveInterface* pt = GetIActiveInterfacePtr( actor->targetID_ini );
		if ( pt )
			CSmartLink::SetLink( &actor->pTarget, pt );
	}

	///--- camera ---
	//target
	m_camTargetActive = null; //cand nu am target se uita dupa players
	m_camTargetOld = null;
	m_vCamPosDefault = vLastSpawnPoint;
	//level to RT cam settings
	m_camLevelToRT.SetWorldBounds( m_levelAABB, false, K_CAMTRANS_AXIS_NONE );
	m_camLevelToRT.InitCamera( UTApp().g_rectRT, K_GAME_HEIGHT, K_CAMTRANS_AXIS_V, m_vCamPosDefault );
	m_camLevelToRT.SetCamAnimationSpring( K_LVL_CAM_FOLLOW_SPRING_KS, K_LVL_CAM_FOLLOW_DAMPING_KD );
	// level to screen cam settings (copies position of level to RT
	m_camLevelToScr.SetWorldBounds( m_levelAABB, false, K_CAMTRANS_AXIS_NONE );
	m_camLevelToScr.InitCamera( UTApp().g_rectRenderPP, K_GAME_HEIGHT, K_CAMTRANS_AXIS_V, m_vCamPosDefault );
	m_camLevelToScr.SetCamAnimationNone();
	// call one update so we're sure everything is initialized
	m_camLevelToRT.Update( 0.0f );
	m_camLevelToScr.Update( 0.0f );
	//pools
	//m_poolPhysPts.Init( K_LVL_PHYSP_MAX_CNT );
	m_poolDoofers.Init( K_LVL_DOOFERS_MAX_CNT );
	m_poolBullets.Init( K_LVL_BULLETS_MAX_CNT );

	//spawn selected players
	m_nPlayers = 0;
	for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
	{
		//trecem controller instanceIDs in arr local din level
		m_arrPlayerControllersIIDs[kk] = g_playerSelScr.m_arrPlayers[kk].nInstanceID;
		//set selected 
		m_arrPlayerSelHotJoin[kk] = -1;
		m_arrPlayerSelStrategic[kk] = -1;
		// we have selected player
		if ( g_playerSelScr.m_arrPlayers[kk].bSelected )
		{
			//spawn Player aloca si controllerul potrivit
			int offx = ( ( kk * 2 ) - 1 ) * K_TILE_HSIZE;
			SpawnPlayer( vLastSpawnPoint + Vec2( (float)offx, 0.0f ), kk, -1 );
			//resolve selection
			m_arrPlayerSelHotJoin[kk] = (int)g_playerSelScr.m_arrPlayers[kk].eType;
		}
		else
		{
			m_arrPlayerControllersIIDs[kk] = -1; //allow hot join
		}
	}

	// release mission generator data
	__MissionGen().Release();
	//clear global script memory (per level instance)
	__Scripts().ClearGlobalMemory();
	//reset time multiplier
	SetTimeMultiplier( 1.0f, 0.0f );
	// compute dirty rects (collisions and walls, wall shadows and other data)
	UpdateDirtyRects();
	// create Area meshes after shadows have been computed in UpdateDirtyRects
	for( auto area : m_arrAreas ) {
		V_OP_RET( area->BuildBuffers( ) );
	}

	BuildVisibilityLists();
	//save type of loaded mission
	m_nLoadedLevelType = 0;

	// initialize IGM interface after everything has loaded
	// the interface will use the RT resolution, scaling to real screen
	m_interfaceIGM.Init( this, &UTApp().g_camRTScreen );

	///--- LAST THINGS ---
	//called after characters spawning
	SetLevelState( K_LVL_STATE_PLAYING );

	m_bLoaded = true;

	/*
	LOG(L"Game:: Level loaded:[%s] net.randcheck[%d]", strPathAbs, m_rand.RandInt(60000));

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	LOG(L"Game:: Total Targets:[%d] Hostages:[%d]", m_arrStats[K_LVL_STATS_TARGETS_TOTAL], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);
#endif
*/

	return K_OP_OK;
}


OPRESULT CLevel::LoadTileset( WCHAR* strPath, CTilesetDesc& retTileDesc )
{
	int idx = 0;
	WCHAR tmppath[MAX_PATH_STD]{ 0 };
	WCHAR finalpath[MAX_PATH_STD]{ 0 };
	
	retTileDesc.Clear();

	LOG( L"LoadTileset:: %d", strPath );
	pugi::xml_document doc;
	if ( !doc.load_file( strPath ) )
	{
		return OP_ERR( K_OP_FAILED, K_SEVERITY_CRITICAL, L"LoadTileset:: Unable to load Tileset XML:%s\n", strPath );
	}

	CTexNode* arrTex_colors[K_TILE_LAYERS_CNT]{ nullptr };
	CTexNode* arrTex_normals[K_TILE_LAYERS_CNT]{ nullptr };

	pugi::xml_node rntileset = doc.root().child( L"TILESET" );
	// load water texture
	const WCHAR* waterN = rntileset.attribute( L"water_n" ).as_string();
	swprintf_s( tmppath, MAX_PATH_STD, L"media/levels/data/%s", waterN );
	FileManager::GetMediaPath( tmppath, finalpath );
	retTileDesc.pWaterTex = m_texManager.AddTexture( finalpath, D3DFMT_A8R8G8B8, D3DX_FILTER_NONE, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DX_DEFAULT );
	if ( nullptr == retTileDesc.pWaterTex )
	{
		m_texManager.Release();
		return OP_ERR( K_OP_FAILED, K_SEVERITY_CRITICAL, L"LoadTileset:: Unable to load:%s\n", finalpath );
	}

	idx = 0;
	pugi::xml_node rnimages = rntileset.child( L"IMAGES" );
	for ( pugi::xml_node bnode = rnimages.first_child(); bnode; bnode = bnode.next_sibling() )
	{

		const WCHAR* colormap = bnode.attribute( L"colormap" ).as_string();
		swprintf_s( tmppath, MAX_PATH_STD, L"media/levels/data/%s", colormap );
		FileManager::GetMediaPath( tmppath, finalpath );
		arrTex_colors[idx] = m_texManager.AddTexture( &finalpath[0], D3DFMT_A8R8G8B8, D3DX_FILTER_NONE, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DX_DEFAULT );
		if ( nullptr == arrTex_colors[idx] )
		{
			m_texManager.Release();
			return OP_ERR( K_OP_FAILED, K_SEVERITY_CRITICAL, L"LoadTileset:: Unable to load:%s\n", finalpath );
		}

		const WCHAR* normalmap = bnode.attribute( L"normalmap" ).as_string();
		swprintf_s( tmppath, MAX_PATH_STD, L"media/levels/data/%s", normalmap );
		FileManager::GetMediaPath( tmppath, finalpath );
		arrTex_normals[idx] = m_texManager.AddTexture( finalpath, D3DFMT_A8R8G8B8, D3DX_FILTER_NONE, D3DX_FILTER_NONE, D3DX_DEFAULT, D3DX_DEFAULT );
		if ( nullptr == arrTex_normals[idx] )
		{
			m_texManager.Release();
			return OP_ERR( K_OP_FAILED, K_SEVERITY_CRITICAL, L"LoadTileset:: Unable to load:%s\n", finalpath );
		}

		idx++;
	}

	// assign images to layers
	idx = 0;
	pugi::xml_node rnlayers = rntileset.child( L"TILE_LAYERS" );
	for ( pugi::xml_node bnode = rnlayers.first_child(); bnode; bnode = bnode.next_sibling() )
	{
		const int imgidx = bnode.attribute( L"imgidx" ).as_int();
		if ( ( imgidx < 0 ) || ( imgidx >= K_TILE_LAYERS_CNT ) || ( idx >= K_TILE_LAYERS_CNT ) )
		{
			ErrorBox( K_ERR_WARNING, L"Illegal image idx=%d in tileset descriptor xml! Skipping...", imgidx );
			continue;
		}
		retTileDesc.arrColorTex[idx] = arrTex_colors[imgidx];
		retTileDesc.arrNormalTex[idx] = arrTex_normals[imgidx];
		idx++;
	}

	LOG( L"LoadTileset:: OK" );
	return K_OP_OK;
}

OPRESULT CLevel::DeployAreaInstance( PDEVICE pDevice, WCHAR * strPathAbs, UINT32 nAreaID, Vec2i posTL )
{
	_ASSERT( pDevice != nullptr );
	LOG( L"Area ID:%d", nAreaID );
	// increment area ID for the next area
	CLevelArea* area = new CLevelArea( nAreaID );
	// base ID for level elements so we don't overwrite existing IDs
	UINT32 unBaseID = area->ID * 10000;

	int nLayersCnt = K_TILE_LAYERS_CNT;

	WCHAR Path[MAX_PATH] = { 0 };

	//load level
	FILE *fl = nullptr;
	int err = OS_wfopen_s( &fl, strPathAbs, L"rb" );

	if ( fl == nullptr || err != 0 )
	{
		return OP_ERR( K_OP_FAILED, K_SEVERITY_CRITICAL, L"Could not open area file:%s", strPathAbs );
	}

	//read int array (will disappear probably)
	UINT32 arrInts[10];	//VERSION, area W in blocks, area H in blocks, 0, 0, 0, 0...
	OS_fread( arrInts, sizeof( UINT32 ), 10, fl );
	if ( arrInts[0] != K_EDITOR_LEVEL_FILE_FORMAT_VERSION )
	{
		return OPRESULT( K_OP_FAILED, K_SEVERITY_CRITICAL, L"[Error] LoadLevel(%s)::Wrong file version found: %d !", strPathAbs, arrInts[0] );
	}
	int areaWblocks = arrInts[1];
	int areaHblocks = arrInts[2];

	CHAR charArr[MAX_PATH];
	WCHAR wcharArr[MAX_PATH];
	WCHAR wcsMediaAddr[MAX_PATH];

	// read connectors setup
	OS_freadString( fl, charArr );
	// read tags
	OS_freadString( fl, charArr );

	BYTE missionType = OS_freadByte( fl );
	//tileset name - not used for areas, the story tells you what tileset to load or there's only one
	OS_freadString( fl, charArr );
	//load tile size
	tileW = OS_freadByte( fl );
	tileH = OS_freadByte( fl );
	//level size
	int areaW = OS_freadUInt16( fl );
	int areaH = OS_freadUInt16( fl );
	area->sizeTL.Init( areaW, areaH );

	//level origin - in pixels
	int originY = OS_freadInt16( fl );
	int originX = OS_freadInt16( fl );

	Vec2 vOffset( posTL.x * K_TILE_SIZE_F, posTL.y * K_TILE_SIZE_F );
	//area size
	area->AABBbounds_TL.Set( posTL.x, posTL.y, areaW, areaH );
	area->AABBbounds.Set( area->AABBbounds_TL.x * tileW, area->AABBbounds_TL.y * tileH, area->AABBbounds_TL.Right() * tileW, area->AABBbounds_TL.Bottom() * tileH );
	//m_vLevelOrigin.x = (float)originX + m_levelAABB.x;
	//m_vLevelOrigin.y = (float)originY + m_levelAABB.y;

	// add dirty rect on area so it computes everything
	AddDirtyRect( posTL.x, posTL.y, areaW, areaH );


	area->tiles = new CTile*[areaW];
	for ( int kk = 0; kk < areaW; kk++ )
	{
		area->tiles[kk] = new CTile[areaH];
	}
	//read tiles matrix
	for ( int yy = 0; yy < areaH; yy++ )
	{
		for ( int xx = 0; xx < areaW; xx++ )
		{
			CTile* tl = &area->tiles[xx][yy];
			tl->bbox.Set( ( posTL.x + xx ) * K_TILE_SIZE_F, ( posTL.y + yy ) * K_TILE_SIZE_F, ( posTL.x + xx + 1 ) * K_TILE_SIZE_F, ( posTL.y + yy + 1 ) * K_TILE_SIZE_F );

			for ( int layer_index = 0; layer_index < nLayersCnt; layer_index++ )
			{
				// need to know the tileset size
				Vec2 vTilesetSize = m_tilesetDesc.arrColorTex[layer_index]->getSize();

				UINT16 tlXY = OS_freadUInt16( fl );
				tl->tileXY[layer_index] = tlXY;
				if ( tlXY != K_TILEXY_EMPTY )
				{
					RECT srcrect;
					int tlX = ( tlXY & 0xff00 ) >> 8, tlY = tlXY & 0xff;
					SetRect( &srcrect, tlX * tileW, tlY * tileH, tlX * tileW + tileW, tlY * tileH + tileH );
					
					//#HACK: we make the UV rect a little smaller so we don't get UV seams because of the point filtering
					tl->vUVmin[layer_index] = Vec2( ( srcrect.left + 0.001f ) / vTilesetSize.x, ( srcrect.top + 0.001f ) / vTilesetSize.y );
					tl->vUVmax[layer_index] = Vec2( ( srcrect.right - 0.001f ) / vTilesetSize.x, ( srcrect.bottom - 0.001f ) / vTilesetSize.y );
				}
			}
			// computes some basic data about tiles
			tl->PostConstructionInit();
		}
	}

	size_t converted;
	///--- lights ---
	//read path
	OS_freadString( fl, charArr );

	int lightsCnt = (int)OS_freadUInt32( fl );
	// Data for each light 
	for ( int kk = 0; kk < lightsCnt; kk++ )
	{
		CLight *nl = new CLight( new CPropAIComponent() );
		nl->m_nLightMeshIdx = -1;
		nl->ID = unBaseID + OS_freadUInt32( fl );
		nl->editor_paintOrderIdx = kk;

		nl->type = (eLightType)OS_freadByte( fl ); //tip lumina
		int nVolumeAttenuationPerc = (int)OS_freadUInt32( fl );
		nl->fVolumeAlpha = 1.0f - (float)nVolumeAttenuationPerc / 100.0f;
		nl->fIntensity = OS_freadFloat32( fl );
		CLAMP( nl->fIntensity, 0.0f, 1.0f );
		Vec3 vpos( 0.0f, 0.0f, 0.0f );
		vpos.x = (float)OS_freadInt32( fl );
		vpos.y = (float)OS_freadInt32( fl );
		vpos.z = (float)OS_freadInt32( fl );
		//#TODO: should load from level file
		vpos.z = 32.0f;
		vpos.x += vOffset.x; vpos.y += vOffset.y;
		nl->pos = vpos;
		nl->pos_ini = nl->pos;
		//animID
		CHAR charAnmName[MAX_PATH];
		OS_freadString( fl, charAnmName );

		CSpriteLib* spr_lights = m_sprLib.GetLibByNick( K_LIBNICK_LIGHTS );
		nl->fidTexture.animIdx = spr_lights->GetAnimationIdxByName( charAnmName );
		nl->fidTexture.frameIdx = 0;
		/*
		if ((nl->animID < 0) && (nl->type != K_LVL_LT_AMBIENTAL))
			ErrorBox(K_ERR_WARNING, L"Light ID:%d doesn't have animID!!", nl->ID);
			*/
			//color
		BYTE ca, cr, cg, cb;
		ca = OS_freadUByte( fl ); cr = OS_freadUByte( fl ); cg = OS_freadUByte( fl ); cb = OS_freadUByte( fl );
		nl->color = D3DCOLOR_ARGB( ca, cr, cg, cb );
		nl->color_ini = nl->color;

		Vec2 bbmin, bbmax;
		bbmin.x = (float)OS_freadInt32( fl );
		bbmin.y = (float)OS_freadInt32( fl );
		bbmax.x = bbmin.x + (float)OS_freadInt32( fl );
		bbmax.y = bbmin.y + (float)OS_freadInt32( fl );
		//set loaded size (default)
		nl->bbox.Set_Corrected( bbmin, bbmax );
		nl->bbox.SaveSnapshotOff( -nl->pos.xy );
		nl->fRadius = max( nl->bbox.vSize.x, nl->bbox.vSize.y );
		//re-arrange spots (maybe lights image changed)
		nl->SetLightTexture( spr_lights, nl->fidTexture.animIdx, nl->fidTexture.frameIdx );

		//#HACK: hardcodes the radius
		nl->fRadius = 128.0f;

		//read angle and convert to radians
		float fAngle = (float)OS_freadInt16( fl );
		fAngle = DEG_TO_RAD( fAngle );
		//casts shadows
		UINT16 u2b = OS_freadUInt16( fl );
		bool casts_shadows = ( ( u2b & K_EDITOR_LIGHT_FLAG_CAST_SHADOWS ) != 0 );
		nl->SetCastShadows( casts_shadows );

		//save global ambient light color
		if ( nl->type == K_LVL_LT_AMBIENTAL )
		{
			m_colAmbientGlobal = nl->color;
			// set ambiental bbox the size of the area
			nl->bbox = area->AABBbounds;
			nl->bbox.SaveSnapshot();
		}

		//load logic
		nl->LoadLogic( fl );
		if ( nl->targetID_ini >= 0 )
			nl->targetID_ini += unBaseID;

		//set all internal light data needed for rendering
		nl->UpdateInternalData( spr_lights );
		// called when adding the light to the lights array
		nl->PostConstructionInit();
		m_arrLights.Add( nl );
	}

	///--- collision elements ---

	//read level collision boxes
	int colCnt = (int)OS_freadUInt32( fl );

	for ( int kk = 0; kk < colCnt; kk++ )
	{
		CCollisionShape* colobj = new CCollisionShape( new CPropAIComponent() );
		colobj->ID = unBaseID + OS_freadUInt32( fl );
		colobj->editor_paintOrderIdx = kk;

		Vec2 cmin, cmax;
		cmin.x = (float)OS_freadInt32( fl ); cmin.y = (float)OS_freadInt32( fl ); //XY
		cmax.x = (float)OS_freadUInt32( fl ); cmax.y = (float)OS_freadUInt32( fl ); //WH
		cmax += cmin;
		colobj->bbox.Set( cmin, cmax );
		//bbox safeguarding
		if ( ( colobj->bbox.vSize.x <= 0.0f ) || ( colobj->bbox.vSize.y <= 0.0f ) )
			colobj->bbox.Set( Vec2( 0.0f, 0.0f ), Vec2( 16.0f, 16.0f ) );
		colobj->bbox.Move( vOffset );
		colobj->bbox.SaveSnapshot();
		//set pos on center
		colobj->pos = colobj->bbox.vCenter;
		//type (ub)
		colobj->eType = (ECollType)OS_freadUByte( fl );
		//cast shadows
		colobj->castShadows = ( OS_freadByte( fl ) != 0 ) ? true : false;

		//load logic and init custom data
		colobj->LoadLogic( fl );
		if ( colobj->targetID_ini >= 0 )
			colobj->targetID_ini += unBaseID;

		colobj->PostConstructionInit();
		m_arrColShapes.Add( colobj );
	}


	///--- objects - props ---
	//read path
	OS_freadString( fl, charArr );

	//--- props ---
	CSpriteLib* spr_props = m_sprLib.GetLibByNick( K_LIBNICK_PROPS );

	//#TODO: de folosit spawnProp peste tot
	int decocnt = (int)OS_freadUInt32( fl );
	for ( int kk = 0; kk < decocnt; kk++ )
	{
		CProp* obj = new CProp( *this, new CPropAIComponent() );

		obj->ID = unBaseID + OS_freadUInt32( fl );
		obj->editor_paintOrderIdx = kk;
		//load layer from editor
		obj->editor_layer = (EEditorLayer)OS_freadByte( fl );
		//position (used to load UINT32)
		Vec3 vpos( 0.0f, 0.0f, 0.0f );
		vpos.x = (float)OS_freadInt32( fl );
		vpos.y = (float)OS_freadInt32( fl );
		vpos.x += vOffset.x; vpos.y += vOffset.y;
		obj->pos = vpos;
		obj->pos_ini = obj->pos;
		//animation
		CHAR charAnmName[MAX_PATH];
		OS_freadString( fl, charAnmName );
		int animIdx = spr_props->GetAnimationIdxByName( charAnmName );
		if ( animIdx < 0 )
			ErrorBox( K_ERR_WARNING, L"Active ID:%d without animation!", obj->ID );
		//frame
		int frameIdx = OS_freadUInt16( fl );
		obj->color = 0xffffffff;
		obj->fid_ini.Init( animIdx, frameIdx );
		obj->sprite.Init( spr_props, animIdx, obj->pos.xy_proj, frameIdx, obj->color );
		//angle
		//obj->fAngle = 0.0f;
		//obj->fAngle_ini = 0.0f;
		//load flags that were set by the level editor
		UINT32 activ_flags = OS_freadUInt32( fl );
		//#TODO: flip xy
		//obj->flipX = ((activFlags & K_EDITOR_ACTIVE_FLAG_FLIPX) != 0);
		//animated
		//animated? select different start frame
		obj->bAnimated = ( ( activ_flags & K_EDITOR_ACTIVE_FLAG_ANIMATED ) != 0 );
		if ( obj->bAnimated )
		{
			// randomize starting frame if object is animated (loping usually)
			obj->sprite.frameIdx = m_rand.RandInt( spr_props->GetAFramesCnt( obj->sprite.animIdx ) );
		}

		//load logic and init data
		obj->LoadLogic( fl );
		if ( obj->targetID_ini >= 0 )
			obj->targetID_ini += unBaseID;
		// init actions
		if ( obj->shScriptActions.IsSet() )
		{
			vector<wstring> retarr = TokenizeString( obj->shScriptActions.text, L"," );
			for ( auto & token : retarr )
			{
				CScriptAction scra;
				if ( OP_SUCCESS( GetScriptAction( token.c_str(), scra ) ) )
				{
					obj->arrActions.Add( scra );
				}
			}
		}

		obj->PostConstructionInit();

		area->m_arrProps.Add( obj );
	}

	///--- load actors ---
	//read path
	OS_freadString( fl, charArr );

	int actorscnt = (int)OS_freadUInt32( fl );
	for ( int kk = 0; kk < actorscnt; kk++ )
	{
		UINT32 actID = unBaseID + OS_freadUInt32( fl );
		//pozitia
		Vec2 actPos;
		actPos.x = (float)OS_freadInt32( fl );
		actPos.y = (float)OS_freadInt32( fl );
		actPos += vOffset;
		//boolean SetAngle and angle
		bool bSetActorAngle = ( OS_freadByte( fl ) != 0 ) ? true : false;
		float fActorAngle = DEG_TO_RAD( OS_freadInt16( fl ) );
		//read template name
		CHAR readstr[MAX_PATH];
		WCHAR templateNameW[MAX_PATH];
		OS_freadString( fl, readstr );
		mbstowcs( templateNameW, readstr, MAX_PATH );
		//read selected AI state from editor
		WCHAR stateNameW[MAX_PATH];
		OS_freadString( fl, readstr );
		mbstowcs( stateNameW, readstr, MAX_PATH );
		//direction
		bool bactLookleft = ( OS_freadByte( fl ) != 0 ) ? true : false;
		bool bactCollision = ( OS_freadByte( fl ) != 0 ) ? true : false;
		bool bactGravity = ( OS_freadByte( fl ) != 0 ) ? true : false;
		//logic
		BYTE n1b = OS_freadByte( fl );
		bool bactCanInteract = ( n1b & 0x1 ) != 0;
		bool bactHideInteract = ( n1b & 0x2 ) != 0;
		float factTouchDuration = (float)OS_freadInt32( fl );
		bool bactStartHidden = ( OS_freadByte( fl ) != 0 ) ? true : false;
		INT32 nactTargetID = OS_freadInt32( fl );
		if ( nactTargetID >= 0 )
			nactTargetID += unBaseID;
		//read script AI name
		CHAR strScriptName[MAX_PATH];
		CHAR strAIname[MAX_PATH];
		OS_freadString( fl, strScriptName );
		OS_freadString( fl, strAIname );
		//read AI params
		CVariantMap arrParams;
		int nAIparamsCnt = OS_freadByte( fl ); //nr params
		if ( nAIparamsCnt > 0 )
		{
			for ( int i = 0; i < nAIparamsCnt; i++ )
			{
				CHAR varname[MAX_PATH] = { 0 };
				WCHAR wvarname[MAX_PATH] = { 0 };
				CHAR varval[MAX_PATH];
				WCHAR wvarval[MAX_PATH];

				OS_freadString( fl, varname );
				OS_freadString( fl, varval );

				size_t convnr;
				mbstowcs_s( &convnr, wvarname, varname, MAX_PATH );
				mbstowcs_s( &convnr, wvarval, varval, MAX_PATH );

				arrParams.SetVarAUTO( wvarname, wvarval );
			}
		}
		///--- FINISHED READING DATA ---
		CStringHash shTemplateNameHash( templateNameW );
		//add actor
		//#TODO: #IMPORTANT: use SpawnActor to spawn all new actors
		/*
		CActor* nact = new CActor();

		nact->ID = actID; //save actor ID
		nact->bAnimated = true;  //animated by default

		CActorTemplate* acttempl = Actor_GetTemplate(shTemplateNameHash.textHash);
		if (acttempl == null)
		{
			ErrorBox(K_ERR_WARNING, L"LoadLevel::GetTemplateActor - invalid template name: %s", shTemplateNameHash.text);
		}


		//#TODO: use SpawnActor
		//InitActor(nact, acttempl, actPos);

		nact->fAngle = nact->fAngle_ini = fActorAngle;
		//daca unghiul e setat din editor il las asa cum e, altfel il sincronizez cu lookdirXsign
		//Unghiul trebuie setat corect pentru ca e folosit la gasirea inamicilor
		if (bSetActorAngle)
		{
			nact->SetAngle(fActorAngle);
		}

		nact->bHasCollision = bactCollision;
		nact->bHasGravity = bactGravity;

		//logic
		nact->bCanInteract = bactCanInteract;
		nact->bHideInteractIcon = bactHideInteract;
		//interact timer
		nact->fTouchDuration = factTouchDuration;
		//start hidden
		nact->bHidden = nact->bSetHidden = bactStartHidden;

		nact->targetID_ini = nactTargetID; //save for later when we have loaded all the objects
		//script name
		nact->script_hash.Init(strScriptName);

		int nNewAIstate = nact->AIstate; //default state is old state
		//if (strout[0] != 0) //if not empty override template AI state
		//{
		//	nNewAIstate = GetAIStateByNameHash(FastHash(strout));
		//}

		//append the editor ai params as some of them are set from the AI function
		nact->varAIparams.AppendCollection(arrParams);

		//set AI
		//#TODO: next line is useless
		SetAI(nact, nNewAIstate, &nact->varAIparams, nact->targetID_ini);
		//set state that was set from the editor
		if (wcslen(stateNameW) > 0)
		{
			CAIState* nState = nact->actTemplate.AItemplate->GetAIStateByName(stateNameW);
			if (nState == null)
			{
				ErrorBox(K_ERR_WARNING, L"[WARNING] LoadLevel: State %s not found on ID:%d", stateNameW, nact->ID);
			}
			Actor_SetAIState(nact, nState);
		}

		m_arrActors.Add(nact);
		*/
	}

	///--- incarca elementele speciale ---
	UINT32 miscCnt = OS_freadUInt32( fl );
	for ( UINT32 kk = 0; kk < miscCnt; kk++ )
	{
		BYTE type = OS_freadByte( fl );

		switch ( type )
		{
			case K_LVL_MISC_SPAWNPOINT:
			{
				CMiscObject_Spawnpoint* spawnptobj = new CMiscObject_Spawnpoint();
				//generic data
				spawnptobj->ID = unBaseID + OS_freadUInt32( fl );
				//read params
				int nparamsCnt = OS_freadByte( fl ); //nr params
				if ( nparamsCnt > 0 )
				{
					for ( int i = 0; i < nparamsCnt; i++ )
					{
						CHAR varname[MAX_PATH] = { 0 };
						CHAR varval[MAX_PATH];
						WCHAR wvarval[MAX_PATH], wvarname[MAX_PATH];

						OS_freadString( fl, varname );
						OS_freadString( fl, varval );

						size_t convnr;
						mbstowcs_s( &convnr, wvarval, varval, MAX_PATH );
						mbstowcs_s( &convnr, wvarname, varname, MAX_PATH );

						spawnptobj->varParams.SetVarAUTO( wvarname, wvarval );
					}
				}
				//specific data 
				spawnptobj->pos.x = (float)OS_freadInt32( fl );
				spawnptobj->pos.y = (float)OS_freadInt32( fl );
				spawnptobj->pos += vOffset;

				m_arrMiscObjects.Add( spawnptobj );
			}
			break;
			case K_LVL_MISC_SCRIPT:
			{
				//generic data
				UINT32 ID = unBaseID + OS_freadUInt32( fl );
				//read params
				int nparamsCnt = OS_freadByte( fl ); //nr params
				if ( nparamsCnt > 0 )
				{
					for ( int i = 0; i < nparamsCnt; i++ )
					{
						CHAR varname[MAX_PATH] = { 0 };
						CHAR varval[MAX_PATH];
						WCHAR wvarname[MAX_PATH];
						WCHAR wvarval[MAX_PATH];

						OS_freadString( fl, varname );
						OS_freadString( fl, varval );

						size_t convnr;
						mbstowcs_s( &convnr, wvarval, varval, MAX_PATH );
						mbstowcs_s( &convnr, wvarname, varname, MAX_PATH );

						if ( wcscmp( wvarname, L"str_script" ) == 0 )
						{
							//am citit primul parametru iar valoarea lui este bsx-ul fundalului deci incarc fundalul
							__Scripts().StartScript( wvarval );
						}
					}
				}
				//pozitia o citesc si nu o folosesc
				OS_freadUInt32( fl ); OS_freadUInt32( fl );
			}
			break;
			case K_LVL_MISC_RAILS:
			{
				CMiscObjectRail* rail = new CMiscObjectRail();
				//generic data
				rail->ID = unBaseID + OS_freadUInt32( fl );
				//read params
				int nparamsCnt = OS_freadByte( fl ); //nr params
				if ( nparamsCnt > 0 )
				{
					for ( int i = 0; i < nparamsCnt; i++ )
					{
						CHAR varname[MAX_PATH] = { 0 };
						CHAR varval[MAX_PATH] = { 0 };
						WCHAR wvarname[MAX_PATH];
						WCHAR wvarval[MAX_PATH];

						OS_freadString( fl, varname );
						OS_freadString( fl, varval );

						size_t convnr;
						mbstowcs_s( &convnr, wvarname, varval, MAX_PATH );
						mbstowcs_s( &convnr, wvarval, varval, MAX_PATH );

						rail->varParams.SetVarAUTO( wvarname, wvarval );
					}
				}
				//specific data 
				//pozitia punctelor
				UINT16 ptscnt = OS_freadUInt16( fl );
				float totalLength = 0.0f;
				//coordonate puncte
				for ( int i = 0; i < ptscnt; i++ )
				{
					Vec2 pt;
					pt.x = OS_freadInt32( fl );
					pt.y = OS_freadInt32( fl );
					pt += vOffset;
					rail->arrPoints.Add( pt );
					//lungimile
					if ( i == 0 )
					{
						totalLength = 0.0f;
						rail->arrLenghts.Add( totalLength );
					}
					else
					{
						Vec2 dist = rail->arrPoints.m_pData[i] - rail->arrPoints.m_pData[i - 1];
						float ldist = D3DXVec2Length( &dist );
						totalLength += ldist;
						rail->arrLenghts.Add( totalLength );
					}
				}
				rail->fLength = totalLength;
				//check total len
				if ( totalLength <= 0.0f )
				{
					ErrorBox( K_ERR_WARNING, L"Zero length rail! ID:%d", rail->ID );
					SAFE_DELETE( rail );
					break;
				}
				//add rail to list if everything ok
				m_arrMiscObjects.Add( rail );
			}
			break;
		}
	}

	OS_fclose( fl );


	LOG( L"Game:: Area loaded:[%s] net.randcheck[%d]", strPathAbs, m_rand.RandInt( 60000 ) );

	// call device creation so it initializes everything device related
	area->OnCreateDevice( pDevice );
	// add to list of areas
	m_arrAreas.Add( area );

	return K_OP_OK;
}


OPRESULT CLevel::LoadLevelDefines( WCHAR* strPath )
{
	pugi::xml_document doc;
	if ( !doc.load_file( strPath ) )
	{
		return OP_ERR( K_OP_FAILED, K_SEVERITY_CRITICAL, L"LoadLevelDefines:: Unable to load Level Defines XML:%s\n", strPath );
	}

	///--- load ACTIONS templates
	m_arrActionTemplates.clear();
	m_arrActionTemplates.shrink_to_fit();

	pugi::xml_node rnactions = doc.root().child( L"ACTIONS" );
	for ( pugi::xml_node bnode = rnactions.first_child(); bnode; bnode = bnode.next_sibling() )
	{
		CScriptAction sa;
		const WCHAR* bType = bnode.name();
		sa.shID.Init( bType );

		sa.strTargetClasses = bnode.attribute( L"targetClasses" ).as_string();
		sa.shScriptName.Init( bnode.attribute( L"scriptName" ).as_string() );
		sa.strID_name = __Texts().GetStrIdx( bnode.attribute( L"strIDname" ).as_string() );

		m_arrActionTemplates.push_back( sa );
	}

	LOG( L"LoadLevelDefines:: OK" );
	return K_OP_OK;
}

