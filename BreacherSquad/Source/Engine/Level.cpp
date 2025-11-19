#include "dxstdafx.h"

//saves warnings stack
#pragma warning(push)
//disable warning
//#pragma warning(disable : 4706)  //assignment within conditional expression

using namespace std;


/* \brief Spawns a new player at spawnPos
* Takes all the necessary spawn data from the "Gear selection screen" object.
* \param nPlayerOrdinal - 0-player1 or 1-player2
* \param nAnimset: -1 to skip spawn animation, 0 first animation, 1 second animation
*/
void CLevel::SpawnPlayer( Vec2 spawnPos, int nPlayerOrdinal, int nAnimset )
{
	if ( (nPlayerOrdinal < 0) || (nPlayerOrdinal >= K_MAX_PLAYERS_CNT) )
	{
		ErrorBox( K_ERR_WARNING, L"SpawnPlayer::Wrong Player Ordinal!" );
		return;
	}

	if ( m_arrPlayerControllersIIDs[nPlayerOrdinal] < 0 )
	{
		ErrorBox( K_ERR_WARNING, L"SpawnPlayer::Invalid controller UID! Probably controller was removed." );
		return;
	}

	//find selected template
	CPlayerSelScr::CPlayerCharSelection* playersel = &g_playerSelScr.m_arrPlayers[nPlayerOrdinal];

	if ( playersel->eType == K_PSS_CLASS_NOT_SELECTED )
	{
		ErrorBox( K_ERR_WARNING, L"SpawnPlayer::Player type not selected!" );
		return;
	}

	GenKey actidx = SpawnActor( spawnPos, L"act_breacher.xml" );
	if ( actidx.index < 0 )
	{
		ErrorBox( K_ERR_WARNING, L"Could not spawn actor!" );
		return;
	}

	CActor* nact = m_arrActors.GetByKey( actidx );
	pPlayerActor[nPlayerOrdinal] = nact;

	//set controller
	pPlayerActor[nPlayerOrdinal]->nPlayerOrdinal = nPlayerOrdinal;
	pPlayerActor[nPlayerOrdinal]->nControllerInstanceID = m_arrPlayerControllersIIDs[nPlayerOrdinal];

	//update backup template
	nact->_template_ini = nact->_template;

	//run ON_SPAWN script
	/*
	if (!templateLocal.shScript_OnSpawn.IsEmpty())
	{
		StartScript(templateLocal.shScript_OnSpawn.getHash(), nact);
	}
	*/

	//animate player on spawn (only if told otherwise by nAnimset=-1)
	/*
	if (nAnimset >= 0)
	{
		nact->SetAnimSet(nAnimset);
		SetActorAIState(nact, L"JOIN_GAME");
		//AddProp_Light(nact->GetPosHeart(), ANM_LIGHTS_SPR_POINT1, 0.5f, 0.1f, 0x8888ff00, 1.0f);

		//set invulnerability
		SetActorDoT(nact, CDamageOverTime::K_LVL_DoT_INVINCIBLE, 2.0f, 0.0f, K_ACT_CLASS_ANY, K_ACT_CLASS_ANY, 0);

		//SND_PLAY(SNDIDX_UI_PLAYER_JOIN);
	}
	*/

	//set skin
	//nact->nSkinIdx = nPlayerOrdinal;

	//resetam numarul de puncte strategice si scoatem selectia
	m_arrStats[K_LVL_STATS_PL1_STRATEGIC_POINTS + nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT] = 0;
	//m_interfaceIGM.SetStrategicPoints(m_arrStats[K_LVL_STATS_PL1_STRATEGIC_POINTS] / 1000.0f, m_arrStats[K_LVL_STATS_PL2_STRATEGIC_POINTS] / 1000.0f);
	//m_interfaceIGM.SetStrategicSelection(nPlayerOrdinal, -1);
	//initialize arrays
	InitializeStrategicAbilities( nPlayerOrdinal );

	//count players again
	m_nPlayers = 0;
	m_nPlayersActive = 0;
	for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
	{
		if ( pPlayerActor[kk] != nullptr )
		{
			m_nPlayers++;
			//HAS_PLAYED needs to be 0 or 1
			m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED + kk * K_LVL_STATS_PLAYER_STATS_COUNT] = 1;

			EAIBehaviorType curbeh = pPlayerActor[kk]->GetCurrentBehavior();
			if ( curbeh != AI_BEHAVIOR_IN_LIMBO )
				m_nPlayersActive++;
		}
	}
}

GenKey CLevel::SpawnActor( Vec2 spawnPos, WCHAR* strTemplateFileName, CStringHash* shStateOverride )
{
	CActorTemplate* acttemplate = Actor_LoadTemplate( strTemplateFileName );
	if ( acttemplate == nullptr )
	{
		ErrorBox( K_ERR_WARNING, L"LoadLevel::Actor_GetTemplate - invalid template name: %s", strTemplateFileName );
		return { -1,-1 };
	}

	//copy template locally and customize it based on gear selection
	CActorTemplate* templateLocal = new CActorTemplate();
	*templateLocal = *acttemplate;
	templateLocal->FillDefaultValuesIfNotSet();
	///--- set weapons and gear modifiers ---
	UINT32 namehash = 0;
	//equipment	- add equipment template
	/*
	namehash = 0; //equipment hash name
	if (namehash != 0)
	{
	///ADD EQUIPMENT TEMPLATE
	CActorTemplate* acttempl = Actor_GetTemplate(namehash);
	if (acttempl != null)
	{
	templateLocal.AddGenericDataFromTemplate(acttempl);
	templateLocal.OverwriteAnimsFromTemplate(acttempl);
	}
	}
	//gear
	namehash = 0; //gear template hash
	CWeaponTemplate* wGear = Weapon_GetTemplate(namehash);
	if ((wGear != nullptr) && (!wGear->bPassive))
	{
	templateLocal.weaponTypeGear = wGear->name;
	}
	if ((wGear != nullptr) && (!wGear->shTemplateOverwrite.IsEmpty()))
	{
	///ADD GEAR TEMPLATE
	CActorTemplate* acttempl = Actor_GetTemplate(wGear->shTemplateOverwrite.getHash());
	if (acttempl != null)
	{
	templateLocal.AddGenericDataFromTemplate(acttempl);
	templateLocal.OverwriteAnimsFromTemplate(acttempl);
	}
	}
	*/
	///LAST! PRIMARY WEAPON TEMPLATE GETS ADDED WHEN CHANGING WEAPONS (equiping main weapon)
	/// We don't add it here so it saves the actor->template_ini without the equipped weapons
	//alt fire from primary weapon
	/*
	namehash = 0;
	CWeaponTemplate* wPrimaryALT = Weapon_GetTemplate(namehash);
	if ((wPrimaryALT != nullptr) && (!wPrimaryALT->bPassive))
	{
	templateLocal.weaponTypeAlt = wPrimaryALT->name;
	}
	//get selected primary weapon
	namehash = 0;
	CWeaponTemplate* wPrimary = Weapon_GetTemplate(namehash);
	if ((wPrimary != nullptr) && (!wPrimary->bPassive))
	{
	templateLocal.weaponType = wPrimary->name;
	}
	*/
	// get weapons sprite lib and send it to the weapons component
	CSpriteLib* pSprWpn = m_sprLib.GetLibByNick( K_LIBNICK_WEAPONS );
	auto nactnode = m_arrActors.Hire();
	if ( nactnode == nullptr )
	{
		return { -1,-1 };
	}

	CActor* nact = &nactnode->m_data;
	nact->Init( spawnPos, templateLocal, GenerateNextID(),
		this,
		new CSpriteActorComponent( &m_sprLib ),
		new CWeaponsComponent( pSprWpn ),
		new CActorAIComponent( *this )
	);

	// create a weapon and add it to the player's arsenal
	//#TODO: create actor::AddWeapon and EquipWeapon that handles this plus AddWeaponTemplate
	CWeaponTemplate* wpntMain = GetTemplateWeapon( nact->_template.shWeaponDefault.text );
	CWeaponTemplate* wpntAlt = nullptr;
	if ( wpntMain != nullptr && wpntMain->shAltFireTemplate.IsSet() )
		wpntAlt = GetTemplateWeapon( wpntMain->shAltFireTemplate.text );
	//#TODO: weapons should have a type like primary, alt, melee, gear?
	nact->Weapons()->AddWeapon( *nact, wpntMain, K_WPNSLOT_PRIMARY );
	if ( wpntAlt ) {
		nact->Weapons()->AddWeapon( *nact, wpntAlt, K_WPNSLOT_ALTFIRE );
	}

	nact->EquipWeapon( K_WPNSLOT_PRIMARY );
	// initialize AI
	nact->SetAIState( nact->_template.AItemplate->GetAIStateByName( nact->_template.shAIState_ini ) );
	// prepare actor for play after everything is loaded and set up
	nact->PostConstructionInit();

	nact->BeginPlay();


	SAFE_DELETE( templateLocal );

	return nactnode->GetGenKey();
}

CProp* CLevel::SpawnProp( CLevelArea* pArea, Vec2 spawnPos, int nAnimIdx, int nFrameIdx )
{
	_ASSERT( pArea != nullptr );
	ErrorBox( K_ERR_WARNING, L"Not implemented! See level_loaders when loading props!" );
	//#TODO: shuntat pentru moment, de rescris
	return nullptr;

	CProp* obj = new CProp( *this, new CPropAIComponent() );

	obj->ID = GenerateNextID();
	//pozitia
	obj->pos = spawnPos;
	obj->pos_ini = obj->pos;
	//anim
	int animIdx = nAnimIdx;
	int frameIdx = nFrameIdx;
	CSpriteLib* spr_props = m_sprLib.GetLibByNick( K_LIBNICK_PROPS );
	obj->sprite.Init( spr_props, animIdx, obj->pos.xy_proj, frameIdx, 0xffffffff );
	obj->fid_ini.Init( animIdx, frameIdx );
	obj->sprite.color = obj->color;
	//#TODO: de mutat initializari de height si flags in PostConstructionInit si de cache spr_props sau luat ca param
	//angle
	//obj->fAngle = obj->fAngle_ini = 0.0f;
	//load flags and split
	UINT32 activFlags = 0;
	//flip xy
	//obj->flipX = ((activFlags & K_EDITOR_ACTIVE_FLAG_FLIPX) != 0);
	//obj->flipY = ((activFlags & K_EDITOR_ACTIVE_FLAG_FLIPY) != 0);
	//animated
	//obj->bAnimated = ((activFlags & K_EDITOR_ACTIVE_FLAG_ANIMATED) != 0);
	//cand e animat selecteaza random frame-ul de pornire
	if ( obj->bAnimated )
	{
		obj->sprite.frameIdx = m_rand.RandInt( spr_props->GetAFramesCnt( obj->sprite.animIdx ) );
	}
	//bbox
	RectXYWHi bbox_set = spr_props->GetAFrameBBox( animIdx, frameIdx );
	RectXYWHi objbox = spr_props->GetAFrameBBox_real( animIdx, frameIdx );
	obj->bbox.Set( objbox );
	obj->bbox.SaveSnapshot();
	obj->bbox_floor.Set( bbox_set );
	obj->bbox_floor.SaveSnapshot();
	//daca e flipat pe X flipez si bbox. Pe Y nu e cazul pt ca se pastreaza in acelasi bbox in paint
	/*
	if (obj->flipX)
	{
		obj->bbox_ini.Move(Vec2(-2.0f * obj->bbox_ini.vCenter.x, 0.0f));
		obj->bbox_floor_ini.Move(Vec2(-2.0f * obj->bbox_floor_ini.vCenter.x, 0.0f));
	}
	*/

	//load logic
	obj->bCanInteract = false;
	obj->bHideInteractIcon = false;

	obj->SetEnabled( true, true );

	obj->targetID_ini = -1;
	obj->AIstate = K_AI_STATE_UNDEFINED;
	// add to specified area
	obj->PostConstructionInit();
	pArea->m_arrProps.Add( obj );
	obj->BeginPlay();
	return obj;
}

CLight* CLevel::SpawnLight( Vec3 spawnPos, eLightType eType, DWORD dwColor, float fRadius, int profileID, bool bCastShadows )
{
	CLight* nl = new CLight( new CPropAIComponent() );
	nl->ID = GenerateNextID();
	nl->type = eType;
	nl->fVolumeAlpha = 1.0f;
	nl->fIntensity = 1.0f;
	nl->pos = spawnPos;
	nl->pos_ini = nl->pos;
	//animID
	nl->fidTexture.Init( 0, 0 );
	nl->nProfileID = profileID;
	nl->fRadius = fRadius;
	nl->color = dwColor;
	nl->color_ini = nl->color;
	nl->SetCastShadows( bCastShadows );

	//set all internal light data needed for rendering
	CSpriteLib* spr_lights = m_sprLib.GetLibByNick( K_LIBNICK_LIGHTS );
	nl->UpdateInternalData( spr_lights );
	// called when adding the light to the lights array
	nl->PostConstructionInit();
	//add light and return it
	m_arrLights.Add( nl );
	nl->BeginPlay();
	return nl;
}


UINT32 CLevel::GenerateNextID()
{
	m_unLastID++; //last ID always stays on a new ID
	return (m_unLastID - 1);
}

CLevel::CLevel()
{
	m_unLastID = 10000000;

	m_bufferedPainter.Init( 4000 );

	m_bLoaded = false;
	m_bOneUpdateDone = false;

	m_dwSyncCheckHash = 0;

	fLocalTimeline = 0.0f;

	m_pDevice = nullptr;
	tileW = tileH = 0;

	m_levelAABB.Set( 0.0f, 0.0f, 0.0f, 0.0f );
	m_levelAABB_TL.Set( 0, 0, 0, 0 );
	//bullets
	m_propsLightsMeshIdx = -1;

	//fog of war
	m_bulletsMeshIdx = -1;
	//level states
	m_levelState = K_LVL_STATE_PLAYING;
	m_levelSubState = 0;
	m_levelStateTimer = 0.0f;

	m_camLevelToRT.SetViewport( UTApp().g_rectRT );
	m_camLevelToScr.SetViewport( UTApp().g_rectRenderPP );

	m_nPlayers = 0;
	m_nPlayersActive = 0;
	for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
	{
		pPlayerActor[kk] = null;
		m_arrPlayerControllersIIDs[kk] = -1; //init player controllers array on no controller
		m_arrPlayerSelHotJoin[kk] = -1;
		m_arrPlayerSelStrategic[kk] = -1;
	}

	vLastSpawnPoint = Vec2( 0.0f, 0.0f );
	m_vCamPosDefault = Vec2( 0.0f, 0.0f );
	//time control
	m_fTimeMultiplier = m_fTimeMultiplier_real = 1.0f;
	m_fTimeMultiplierDuration = 0.0f;
}

CLevel::~CLevel()
{
	Release();
}


void CLevel::UpdateDirtyRects()
{
	//#TODO: doesn't change WALKABLE floor flags, that should be done during loading or level editing for speed
	//#TODO: should make sure the level always has a 1 tile border!
	//#TODO: should set and update the passability values
	//#TODO: should set a flag after updating a tile so it doesn't update again if 2 rectangles overlap
	///--- compute tile flags ---
	for ( auto rect : m_arrDirtyRectsTL )
	{
		for ( int ii = 0; ii < m_arrAreas.GetSize(); ii++ )
		{
			CLevelArea* area = m_arrAreas[ii];
			if ( !rect.Intersects( area->AABBbounds_TL ) )
				continue;
			// take border tiles into account:
			// clamp to smaller size because we check neighbours
			RectXYWHi lrect = rect;
			// clamp and bring rectangle to local space
			lrect.IntersectWith( area->AABBbounds_TL );
			if ( (lrect.w == 0) || (lrect.h == 0) )
				continue;

			for ( int yy = lrect.y; yy < lrect.y + lrect.h; yy++ )
			{
				for ( int xx = lrect.x; xx < lrect.x + lrect.w; xx++ )
				{
					CTile* tl = area->GetTile( xx, yy );
					if ( tl == nullptr )
						continue;
					// set passability flags in pathfinding map 
					if ( tl->flags & K_TILEFLAG_WALKABLE )
						m_astar.SetNodeFlags( xx, yy, COL_ACCESSIBLE );
					else
						m_astar.SetNodeFlags( xx, yy, COL_MOVEMENT_BLOCK );

					// neighbours
					CTile* tlL = area->GetTile( xx - 1, yy );
					CTile* tlR = area->GetTile( xx + 1, yy );
					CTile* tlU = area->GetTile( xx, yy - 1 );
					CTile* tlD = area->GetTile( xx, yy + 1 );
					///--- set wall flags on non walkable tiles for shadows and other 
					if ( FLAG_NONE( tl->flags, K_TILEFLAG_WALKABLE ) && FLAG_NONE( tl->flags, K_TILEFLAG_UNDER_FLOOR ) )
					{
						// clear flags
						FLAGOP_CLEAR( tl->flags, K_TILEFLAG_HASWALL_MASK | K_TILEFLAG_WALLENDING_MASK );

						if ( (tlL) && (FLAG_ANY( tlL->flags, K_TILEFLAG_WALKABLE )) )
						{
							tl->flags |= K_TILEFLAG_HASWALL_L;
						}
						if ( (tlR) && (FLAG_ANY( tlR->flags, K_TILEFLAG_WALKABLE )) )
						{
							tl->flags |= K_TILEFLAG_HASWALL_R;
						}
						if ( (tlU) && (FLAG_ANY( tlU->flags, K_TILEFLAG_WALKABLE )) )
						{
							tl->flags |= K_TILEFLAG_HASWALL_U;
						}
						if ( (tlD) && (FLAG_ANY( tlD->flags, K_TILEFLAG_WALKABLE )) )
						{
							tl->flags |= K_TILEFLAG_HASWALL_D;
						}
					}
					else
					{
						///--- process walkable flags ---
						// corners don't matter for now, we just check immediate neighbours URDL
						//#TODO: should add corners too
						bool bFloorBorder = false;
						if ( (tlL) && ((tlL->flags & K_TILEFLAG_WALKABLE) == 0) )
							bFloorBorder = true;
						else if ( (tlR) && ((tlR->flags & K_TILEFLAG_WALKABLE) == 0) )
							bFloorBorder = true;
						else if ( (tlU) && ((tlU->flags & K_TILEFLAG_WALKABLE) == 0) )
							bFloorBorder = true;
						else if ( (tlD) && ((tlD->flags & K_TILEFLAG_WALKABLE) == 0) )
							bFloorBorder = true;

						if ( bFloorBorder )
						{
							m_astar.SetNodeFlags( xx, yy, COL_CLEARANCE0 );
						}
					}
					// find wall endings
					if ( FLAG_ANY( tl->flags, K_TILEFLAG_WALL ) )
					{
						// check neighbours so we set the wall ending flags
						if ( (tlL) && (FLAG_NONE( tlL->flags, K_TILEFLAG_WALL )) )
							tl->flags |= K_TILEFLAG_WALLENDING_L;
						if ( (tlR) && (FLAG_NONE( tlR->flags, K_TILEFLAG_WALL )) )
							tl->flags |= K_TILEFLAG_WALLENDING_R;
					}

					///--- compute wall shadows
					// it can only receive if it's a floor or a wall but not a ceiling on that tile
					tl->nShadowFrame = -1;

					// only walls and floor get shadowed, when having a non walkable tile on the left (hole in the floor usually, but not water hole)
					bool bCanReceive = ((tl->tileXY[K_TILE_LAYER_FLOOR] != K_TILEXY_EMPTY) || (tl->tileXY[K_TILE_LAYER_WALLS] != K_TILEXY_EMPTY)) &&
						(tlL) && (tlL->tileXY[K_TILE_LAYER_FLOOR] == K_TILEXY_EMPTY) && ((tlL->flags & K_TILEFLAG_UNDER_FLOOR) == 0) &&
						(tl->tileXY[K_TILE_LAYER_CEILING] == K_TILEXY_EMPTY);

					if ( bCanReceive )
					{
						CTile* tlDL = area->GetTile( xx - 1, yy + 1 );
						int nCasterH = 0;
						if ( tlL->tileXY[K_TILE_LAYER_CEILING] != K_TILEXY_EMPTY ) nCasterH = 3;
						else if ( tlL->tileXY[K_TILE_LAYER_WALLS] != K_TILEXY_EMPTY )
						{
							if ( (tlDL) && (tlDL->tileXY[K_TILE_LAYER_WALLS] != K_TILEXY_EMPTY) )
								nCasterH = 2;	// top of the wall
							else
								nCasterH = 1;   // base of the wall
						}
						int nReceiverH = 0;
						if ( tl->tileXY[K_TILE_LAYER_WALLS] != K_TILEXY_EMPTY )
						{
							if ( tlD->tileXY[K_TILE_LAYER_WALLS] != K_TILEXY_EMPTY )
								nReceiverH = 2;
							else
								nReceiverH = 1;
						}

						if ( nReceiverH == 0 ) //floor
						{
							if ( nCasterH == 1 )
								tl->nShadowFrame = 0; //floor shadow start
							else
							{
								tl->nShadowFrame = 1; //continuous shadow
							}
						}
						else if ( (nReceiverH == 1) && (nCasterH > 1) )
						{
							tl->nShadowFrame = 2; //base of wall shadowed
						}
						else if ( (nReceiverH == 2) && (nCasterH > 2) )
						{
							tl->nShadowFrame = 3; //top of wall shadowed
						}
					}

					///--- compute wall ending shadows ---
					//#TODO: doesn't support right wall endings and both wall endings on already shadowed walls
					if ( tl->nShadowFrame < 0 )
					{
						UINT32 wallends = tl->flags & K_TILEFLAG_WALLENDING_MASK;
						if ( FLAG_ALL( wallends, K_TILEFLAG_WALLENDING_L | K_TILEFLAG_WALLENDING_R ) )
						{
							//both sides thin wall
							tl->nShadowFrame = 6;
						}
						else if ( wallends == K_TILEFLAG_WALLENDING_L )
						{
							tl->nShadowFrame = 5;
						}
						else if ( wallends == K_TILEFLAG_WALLENDING_R )
						{
							tl->nShadowFrame = 4;
						}
					}
				}
			}
		}
	}

	// finished with dirty rects, clear the array
	m_arrDirtyRectsTL.clear();
}

int CLevel::Areas_UpdateVisibility( RectXYWH camRect )
{
	int nVisible = 0;
	for ( int ii = 0; ii < m_arrAreas.GetSize(); ii++ )
	{
		CLevelArea* area = m_arrAreas[ii];
		if ( area->UpdateVisibility( camRect ) )
			nVisible++;
	}
	return nVisible;
}

OPRESULT CLevel::Areas_PaintLayer( eAreaLayer layerIdx )
{
	for ( int ii = 0; ii < m_arrAreas.GetSize(); ii++ )
	{
		CLevelArea* area = m_arrAreas[ii];
		V_OP_RET( area->areaMesh.PaintLayer( layerIdx ) );
	}
	return K_OP_OK;
}

bool CLevel::Areas_IsLayerVisible( eAreaLayer layerIdx )
{
	for ( auto area : m_arrAreas )
	{
		if ( area->IsLayerMeshVisible( layerIdx ) )
			return true;
	}
	return false;
}

std::vector<CLevelArea*> CLevel::Areas_GetAreasInRect( CAABB aabb )
{
	vector<CLevelArea*> retarr;
	for ( int ii = 0; ii < m_arrAreas.GetSize(); ii++ )
	{
		CLevelArea* area = m_arrAreas[ii];
		if ( area->AABBbounds.Intersects( aabb ) )
			retarr.push_back( area );
	}
	return retarr;
}

CLevelArea* CLevel::Areas_GetAt( Vec2 vPos )
{
	for ( int ii = 0; ii < m_arrAreas.GetSize(); ii++ )
	{
		CLevelArea* area = m_arrAreas[ii];
		if ( area->AABBbounds.PointIn( vPos ) )
			return area;
	}
	return nullptr;
}

CLevelArea* CLevel::Areas_GetByID( UINT32 nID )
{
	for ( int ii = 0; ii < m_arrAreas.GetSize(); ii++ )
	{
		CLevelArea* area = m_arrAreas[ii];
		if ( area->ID == nID )
			return area;
	}
	return nullptr;
}

CTile* CLevel::Areas_GetTileAt( Vec2 vPos )
{
	CLevelArea* area = Areas_GetAt( vPos );
	if ( area == nullptr )
		return nullptr;
	return area->GetTile( (int)floor( vPos.x / K_TILE_SIZE_F ), (int)floor( vPos.y / K_TILE_SIZE_F ) );
}

void CLevel::Areas_GetTilesSnapshot( RectXYWHi srcRectTL, CTile** arrTiles, int arrCapacity )
{
	_ASSERT( arrTiles != nullptr );
	if ( (srcRectTL.w * srcRectTL.h) > arrCapacity )
	{
		ErrorBox( K_ERR_WARNING, L"Areas_GetTilesSnapshot:: array too small!" );
		return;
	}
	// clear array
	memset( arrTiles, 0, sizeof( CTile* ) * arrCapacity );
	// scan all areas one by one
	for ( int ii = 0; ii < m_arrAreas.Count(); ii++ )
	{
		CLevelArea* area = m_arrAreas[ii];
		RectXYWHi rectloc = area->AABBbounds_TL;
		rectloc.IntersectWith( srcRectTL );
		if ( (rectloc.w <= 0) || (rectloc.h <= 0) )
			continue;
		// bring to area space
		rectloc.Move( -area->AABBbounds_TL.x, -area->AABBbounds_TL.y );
		for ( int yy = rectloc.y; yy < rectloc.y + rectloc.h; yy++ )
		{
			for ( int xx = rectloc.x; xx < rectloc.x + rectloc.w; xx++ )
			{
				// brings tile from world pos to srcRectTL relative pos
				Vec2i vPosTL( xx + area->AABBbounds_TL.x - srcRectTL.x, yy + area->AABBbounds_TL.y - srcRectTL.y );
				int nidx = vPosTL.x + vPosTL.y * srcRectTL.w;
				_ASSERT( nidx < arrCapacity );
				// debug checkup:
				//_ASSERT(xx >= 0 && yy >= 0 && xx < area->AABBbounds_TL.w && yy < area->AABBbounds_TL.h);
				arrTiles[nidx] = &area->tiles[xx][yy];
			}
		}
	}
}

bool CLevel::Areas_IsBoxColliding( CAABB srcBox, bool bCheckProps )
{
	std::vector<CLevelArea*> arrAreas = Areas_GetAreasInRect( srcBox );
	for ( auto ar : arrAreas )
	{
		if ( ar->IsBoxColliding( srcBox, bCheckProps ) )
			return true;
	}
	return false;
}

bool CLevel::Areas_IsBoxColliding( CAABB srcBox, Vec2 vecMove, bool bCheckProps )
{
	// temp list for collisions
	CFixedArray<SweepAABB, 128> tempList;
	// find starting area
	CLevelArea* pArea = Areas_GetAt( srcBox.vCenter );
	if ( pArea == nullptr )
	{
		ErrorBox( K_ERR_WARNING, L"Areas_IsBoxColliding: Starting area not found!" );
		return true;
	}
	/// filter possible collisions
	//1. find bbox start and end union that includes all collisions when moving at high speeds
	CAABB destbox, srcbox;
	srcbox = srcBox;
	destbox = srcbox;
	destbox.Move( vecMove );
	// box unions to check all possible collisions
	CAABB boxUnion = AABB::Union( destbox, srcbox );
	boxUnion.Inflate( K_TILE_HSIZE_F, K_TILE_HSIZE_F );
	// bbox union in tile coords, including every touched tile
	RectXYXYi boxUnionTiles( floor( boxUnion.vMin.x / K_TILE_SIZE_F ), floor( boxUnion.vMin.y / K_TILE_SIZE_F ),
		ceil( boxUnion.vMax.x / K_TILE_SIZE_F ), ceil( boxUnion.vMax.y / K_TILE_SIZE_F ) );
	RectXYWHi boxUnionTilesWH( boxUnionTiles.x1, boxUnionTiles.y1, boxUnionTiles.x2 - boxUnionTiles.x1 + 1, boxUnionTiles.y2 - boxUnionTiles.y1 + 1 );

	// keeps a list of all boxes that might be colliding
	tempList.Clear();

	/// BROAD PHASE SWEEP (find all POSSIBLE collision objects)

	//add boxes from collision shapes
	/*
	for ( int kk = 0; kk < m_arrColShapes.GetSize(); kk++ )
	{
		if ( !m_arrColShapes[kk]->IsAlive() )
			continue;

		if ( !boxUnion.Intersects( m_arrColShapes[kk]->bbox ) )
			continue;

		// save box for later collision check
		if ( m_arrColShapes[kk]->collFlags != K_DIRFLAG_NONE )
		{
			tempList.Add( level.m_arrColShapes[kk]->bbox );
		}
	}
	*/
	//add boxes from tiles and props
	CAABB retAABBs[128];
	if ( pArea != nullptr )
	{
		// get collision tiles for current area
		// tiles collboxes
		int nadded = pArea->GetTilesCollisionBoxes( boxUnionTiles, retAABBs, 128 );
		if ( nadded > 0 )
		{
			for ( int oo = 0; oo < nadded; oo++ )
			{
				tempList.Add( retAABBs[oo] );
			}
		}
		// props collboxes
		if ( bCheckProps )
		{
			nadded = pArea->GetPropsCollisionBoxes( boxUnion, retAABBs, 64 );
			if ( nadded > 0 )
			{
				for ( int oo = 0; oo < nadded; oo++ )
				{
					tempList.Add( retAABBs[oo] );
				}
			}
		}
		// if movement bbox is not completely contained in the current area BBox try with the neighbours too
		if ( !pArea->AABBbounds.Contains( boxUnion ) )
		{
			for ( int kk = 0; kk < pArea->arrNeighbours.Count(); kk++ )
			{
				CLevelArea* area = pArea->arrNeighbours.m_pData[kk];
				if ( !area->AABBbounds_TL.Intersects( boxUnionTilesWH ) )
					continue;
				// tiles collboxes
				nadded = area->GetTilesCollisionBoxes( boxUnionTiles, retAABBs, 64 );
				if ( nadded > 0 )
				{
					for ( int oo = 0; oo < nadded; oo++ )
					{
						tempList.Add( retAABBs[oo] );
					}
				}
				// props collboxes
				if ( bCheckProps )
				{
					nadded = area->GetPropsCollisionBoxes( boxUnion, retAABBs, 64 );
					if ( nadded > 0 )
					{
						for ( int oo = 0; oo < nadded; oo++ )
						{
							tempList.Add( retAABBs[oo] );
						}
					}
				}
			}
		}
	}

	/// COLLISION HANDLING

	float fRemainingTime = 1.0f;
	// compute source box
	srcbox = srcBox;
	// find closest collider
	float minDistSq = 100000.0f;
	float fClosestTime = 100000.0f;
	SweepAABB* pClosestBox = nullptr;
	for ( int kk = 0; kk < tempList.Count(); kk++ )
	{
		SweepAABB* tmpbox = &tempList[kk];
		// skip boxes that have been handled this step
		if ( tmpbox->bDisabled )
			continue;

		SweepData sdata = AABBSweep::CalculateSweepData( srcbox, vecMove, *tmpbox );
		// computes even if no valid collision. needs flag to eliminate them
		if ( sdata.bIsValid == false )
			continue;

		if ( sdata.fCollisionTime < fClosestTime )
		{
			fClosestTime = sdata.fCollisionTime;
			minDistSq = sdata.fDistance;
			pClosestBox = tmpbox;
		}
		else if ( sdata.fCollisionTime == fClosestTime )
		{
			if ( sdata.fDistance < minDistSq )
			{
				fClosestTime = sdata.fCollisionTime;
				minDistSq = sdata.fDistance;
				pClosestBox = tmpbox;
			}
		}
		// exit on first collision
		if ( pClosestBox != nullptr )
		{
			return true;
		}
	}

	// no collisions detected until now
	return false;
}

int CLevel::SmoothPath( Vec2* arrInPoints, int arrInItems, Vec2* arrOutPoints, int arrOutSize )
{
	// no input items
	if ( arrInItems == 0 || arrInPoints == nullptr )
		return 0;
	// 1-2 input points
	_ASSERT( arrOutSize > 2 );
	if ( arrInItems <= 2 )
	{
		for ( int kk = 0; kk < arrInItems; kk++ )
			arrOutPoints[kk] = arrInPoints[kk];
		// returns 1 or 2
		return arrInItems;
	}
	// do the actual smoothing
	int outcur = 0;		// out vector cursor
	int incur = 0;		// in vector current point
	// add first point as checkpoint
	//#TODO if solution doesn't give the character position ar trebui sa il puna fortat
	Vec2 vFrom = arrInPoints[incur];
	arrOutPoints[outcur++] = vFrom;
	while ( incur < arrInItems - 1 )
	{
		// walk on next points while they are still visible
		for ( int tocur = incur + 1; tocur < arrInItems; tocur++ )
		{
			int nFoundCur = -1;
			// we reached last element, save it as waypoint
			if ( tocur >= arrInItems - 1 )
			{
				nFoundCur = arrInItems - 1;
			}
			// we can't see this point so we save last point as checkpoint and start again
			else if ( !IsLineOfSight( vFrom, arrInPoints[tocur] ) )
			{
				// if next point isn't visible (some engine element blocking the way or something)
				// then just add it as a checkpoint instead of returning invalid smoothing
				if ( tocur - 1 == incur )
					nFoundCur = tocur;
				else
					nFoundCur = tocur - 1; // save last visible point otherwise
			}

			if ( nFoundCur >= 0 )
			{
				// save last visible point
				arrOutPoints[outcur++] = arrInPoints[nFoundCur];
				_ASSERT( outcur < arrOutSize );
				incur = nFoundCur;
				vFrom = arrInPoints[incur];
				break;
			}
		}
	}

	return outcur;
}

int CLevel::SmoothPathEx( CActor* act, Vec2* arrInPoints, int arrInItems, Vec2* arrOutPoints, int arrOutSize )
{
	// no input items
	if ( arrInItems == 0 || arrInPoints == nullptr )
		return 0;
	// 1-2 input points
	_ASSERT( arrOutSize > 2 );
	if ( arrInItems <= 2 )
	{
		for ( int kk = 0; kk < arrInItems; kk++ )
			arrOutPoints[kk] = arrInPoints[kk];
		// returns 1 or 2
		return arrInItems;
	}
	// do the actual smoothing
	int outcur = 0;		// out vector cursor
	int incur = 0;		// in vector current point
	// add first point as checkpoint
	//#TODO if solution doesn't give the character position ar trebui sa il puna fortat
	Vec2 vFrom = arrInPoints[incur];
	CAABB fromaabb = act->bbox_floor.GetSnapshot();
	fromaabb.Move( vFrom );

	arrOutPoints[outcur++] = vFrom;
	while ( incur < arrInItems - 1 )
	{
		// walk on next points while they are still visible
		for ( int tocur = incur + 1; tocur < arrInItems; tocur++ )
		{
			int nFoundCur = -1;
			// we reached last element, save it as waypoint
			if ( tocur >= arrInItems - 1 )
			{
				nFoundCur = arrInItems - 1;
			}
			// we can't see this point so we save last point as checkpoint and start again
			//else if ( !IsLineOfSight( vFrom, arrInPoints[tocur] ) )
			else if ( Areas_IsBoxColliding( fromaabb, arrInPoints[tocur] - vFrom, true ) )
			{
				// if next point isn't visible (some engine element blocking the way or something)
				// then just add it as a checkpoint instead of returning invalid smoothing
				if ( tocur - 1 == incur )
					nFoundCur = tocur;
				else
					nFoundCur = tocur - 1; // save last visible point otherwise
			}

			if ( nFoundCur >= 0 )
			{
				// save last visible point
				arrOutPoints[outcur++] = arrInPoints[nFoundCur];
				_ASSERT( outcur < arrOutSize );
				incur = nFoundCur;
				vFrom = arrInPoints[incur];
				fromaabb = act->bbox_floor.GetSnapshot();
				fromaabb.Move( vFrom );
				break;
			}
		}
	}

	return outcur;
}


OPRESULT CLevel::GetScriptAction( const WCHAR* strID, CScriptAction& retAction )
{
	CStringHash shID( strID );
	for ( const auto& scra : m_arrActionTemplates )
	{
		if ( scra.shID == shID )
		{
			retAction = scra;
			return K_OP_OK;
		}
	}

	return OPRESULT( K_OP_FAILED, K_SEVERITY_WARNING, L"GetScriptAction:: Could not find action: %s", strID );
}

CWeaponTemplate* CLevel::GetTemplateWeapon( WCHAR* templateName )
{
	UINT32 nameHash = FastHash( templateName );
	for ( int kk = 0; kk < m_arrTemplatesWeapon.GetSize(); kk++ )
	{
		if ( m_arrTemplatesWeapon[kk]->name.getHash() == nameHash )
			return m_arrTemplatesWeapon[kk];
	}

#if defined(_DEBUG) || defined(DEBUG)
	if ( wcslen( templateName ) > 0 )
		ErrorBox( K_ERR_WARNING, L"Weapon template not found! %s", templateName );
#endif

	return nullptr;
}

CWeaponTemplate* CLevel::GetTemplateWeapon( DWORD templateNameHash )
{
	for ( int kk = 0; kk < m_arrTemplatesWeapon.GetSize(); kk++ )
	{
		if ( m_arrTemplatesWeapon[kk]->name.getHash() == templateNameHash )
			return m_arrTemplatesWeapon[kk];
	}

#if defined(_DEBUG) || defined(DEBUG)
	if ( templateNameHash != 0 )
		ErrorBox( K_ERR_WARNING, L"Weapon template (hash) not found!" );
#endif

	return nullptr;
}

CExplosionTemplate* CLevel::GetTemplateExplosion( UINT32 templateNameHash )
{
	for ( int kk = 0; kk < m_arrTemplatesExplosion.GetSize(); kk++ )
	{
		if ( m_arrTemplatesExplosion[kk]->name.getHash() == templateNameHash )
			return m_arrTemplatesExplosion[kk];
	}

#if defined(_DEBUG) || defined(DEBUG)
	ErrorBox( K_ERR_WARNING, L"Explosion template not found!" );
#endif

	return nullptr;
}

CActorTemplate* CLevel::Actor_LoadTemplate( WCHAR* strTemplateFileName )
{
	// LOAD ACTOR TEMPLATE LoadActorTemplate
	char strbuff[MAX_PATH] = { 0 };

	//does it exist?
	CActorTemplate* templ = Actor_GetTemplate( strTemplateFileName );
	if ( templ != nullptr )
	{
		LOG_DBG( L"ActTemplates_Add - reusing template: %s", strTemplateFileName );
		return templ;
	}

	// build file path
	WCHAR Path[MAX_PATH];
	WCHAR wcsPath[MAX_PATH];
	StringCchPrintf( wcsPath, MAX_PATH, L"media/levels/data/actors/%s", strTemplateFileName );
	FileManager::GetMediaPath( wcsPath, Path );

	//does not exist, open xml
	pugi::xml_document doc;
	if ( !doc.load_file( Path ) )
	{
		ErrorBox( K_ERR_WARNING, L"Unable to load actor template XML:%s\n", strTemplateFileName );
		return nullptr;
	}

	//load actor templates
	pugi::xml_node rootnode = doc.root().child( L"ACTOR" );

	templ = new CActorTemplate();
	templ->shID.Init( strTemplateFileName );
	templ->shSourceXML.Init( rootnode.attribute( L"file" ).value() );
	if ( templ->shSourceXML.IsEmpty() )
	{
		ErrorBox( K_ERR_WARNING, L"[WARNING] ActorTemplate source XML not set!\n%s", templ->shSourceXML.text );
		SAFE_DELETE( templ );
		return nullptr;
	}

	//ACTOR_DATA node
	pugi::xml_node actnode = rootnode.child( L"ACTOR_DATA" );
	// bbox and heights
	float xmin = actnode.attribute( L"bboxMinX" ).as_float();
	float ymin = actnode.attribute( L"bboxMinY" ).as_float();
	float xmax = actnode.attribute( L"bboxMaxX" ).as_float();
	float ymax = actnode.attribute( L"bboxMaxY" ).as_float();
	templ->bbox.Set( xmin, ymin, xmax, ymax );
	templ->heightZ = actnode.attribute( L"heightZ" ).as_float();
	templ->heartZ = actnode.attribute( L"heartZ" ).as_float();

	if ( !actnode.attribute( L"fSpeedMove" ).empty() ) { templ->fSpeedMove = actnode.attribute( L"fSpeedMove" ).as_float(); }
	if ( !actnode.attribute( L"fSpeedRun" ).empty() ) { templ->fSpeedRun = actnode.attribute( L"fSpeedRun" ).as_float(); }
	if ( !actnode.attribute( L"fSeeDist" ).empty() ) { templ->fDistSee = actnode.attribute( L"fSeeDist" ).as_float(); }
	if ( !actnode.attribute( L"fAttackMin" ).empty() ) { templ->fAttackMin = actnode.attribute( L"fAttackMin" ).as_float(); }
	if ( !actnode.attribute( L"fAttackMax" ).empty() ) { templ->fAttackMax = actnode.attribute( L"fAttackMax" ).as_float(); }
	//life
	if ( !actnode.attribute( L"fLife" ).empty() ) templ->fLife = actnode.attribute( L"fLife" ).as_float();
	if ( !actnode.attribute( L"fArmor" ).empty() ) templ->fArmor = actnode.attribute( L"fArmor" ).as_float();
	//caps
	templ->eCaps = 0;
	if ( actnode.attribute( L"bCanCover" ).as_bool() )
		templ->eCaps |= K_ACT_CAPS_CAN_COVER;
	if ( actnode.attribute( L"bCanInteract" ).as_bool() )
		templ->eCaps |= K_ACT_CAPS_CAN_INTERACT;
	//other
	if ( !actnode.attribute( L"class" ).empty() )
		templ->actorClass = (EActorClass)GetListIndexByName( actnode.attribute( L"class" ).value(), EActorClassNames, ARRAY_SIZE( EActorClassNames ) );
	if ( !actnode.attribute( L"sWeapon" ).empty() )
		templ->shWeaponDefault.Init( actnode.attribute( L"sWeapon" ).value() );
	if ( !actnode.attribute( L"sAIstate" ).empty() )
		templ->shAIState_ini.Init( actnode.attribute( L"sAIstate" ).value() );

	// skins
	pugi::xml_node skinsnode = rootnode.child( L"SKINS" );
	templ->arrSkinsCnt = 0;
	if ( skinsnode != nullptr )
	{
		for ( auto& nodeskin : skinsnode.children() )
		{
			templ->arrSkins[templ->arrSkinsCnt].name.Init( nodeskin.attribute( L"name" ).value() );
			templ->arrSkins[templ->arrSkinsCnt].layersVisMask = nodeskin.attribute( L"layersVisibilityMask" ).as_uint();
			templ->arrSkins[templ->arrSkinsCnt].hand2Mask = nodeskin.attribute( L"handL_layerMask" ).as_uint();
			templ->arrSkins[templ->arrSkinsCnt].hand1Mask = nodeskin.attribute( L"handR_layerMask" ).as_uint();
			templ->arrSkinsCnt++;
			_ASSERT( templ->arrSkinsCnt < K_ACT_SKINS_MAX_SETS );
		}
	}
	// create default skin if none present (all layers visible)
	if ( templ->arrSkinsCnt == 0 )
	{
		templ->arrSkins[templ->arrSkinsCnt].name.Init( "default" );
		templ->arrSkinsCnt++;
		ErrorBox( K_ERR_WARNING, L"No skin found in template:%s", strTemplateFileName );
	}

	// anims
	pugi::xml_node anmnode = rootnode.child( L"ANIMS" );
	if ( anmnode != nullptr )
	{
		for ( int kk = 0; kk < K_ACT_ANIMS_CNT; kk++ )
		{
			pugi::xml_node nmnode = anmnode.child( EActorAnimNames[kk].text );
			if ( nmnode != nullptr )
			{
				// read anim names
				for ( int nset = 0; nset < K_ACT_ANIM_MAX_SETS; nset++ )
				{
					WCHAR strSetName[MAX_PATH];
					swprintf_s( strSetName, MAX_PATH, L"set%d", nset );
					// read set0 or set1 and set for all angles
					if ( !nmnode.attribute( strSetName ).empty() )
					{
						for ( int ang = 0; ang < EDIR6S_CNT; ang++ )
							templ->arrAnims[kk].animNamesA[nset][ang].Init( nmnode.attribute( strSetName ).value() );
					}
					// read all angles for every set and overwrite
					for ( int ang = 0; ang < EDIR6S_CNT; ang++ )
					{
						WCHAR strAnim[MAX_PATH];
						swprintf_s( strAnim, MAX_PATH, L"%s_%s", strSetName, EDir6Names[ang].text );
						if ( !nmnode.attribute( strAnim ).empty() )
						{
							templ->arrAnims[kk].animNamesA[nset][ang].Init( nmnode.attribute( strAnim ).value() );
						}
					}
				}
			}
		}
	}

	//sound verses
	pugi::xml_node versenode = rootnode.child( L"VERSES" );
	if ( versenode != nullptr )
	{
		for ( int kk = 0; kk < K_LVL_ACT_VERSES_COUNT; kk++ )
		{
			pugi::xml_node nmnode = versenode.child( EActorSoundVerseNames[kk].text );
			if ( nmnode != nullptr )
			{
				if ( !nmnode.attribute( L"set0" ).empty() )
				{
					templ->soundIDs[kk][0] = __Audio().getSndIdxW( nmnode.attribute( L"set0" ).value() );
					if ( (!nmnode.attribute( L"set0" ).empty()) && (templ->soundIDs[kk][0] == -1) )
					{
						//#TEMP: until I change the templates
						//ErrorBox(K_ERR_WARNING, L"Template set0 sound not found!\n%s", nmnode.attribute(L"set0").value());
					}
				}
				//variation
				if ( !nmnode.attribute( L"set1" ).empty() )
				{
					templ->soundIDs[kk][1] = __Audio().getSndIdxW( nmnode.attribute( L"set1" ).value() );
					if ( (!nmnode.attribute( L"set1" ).empty()) && (templ->soundIDs[kk][1] == -1) )
					{
						//#TEMP: until I change the templates
						//ErrorBox(K_ERR_WARNING, L"Template set1 sound not found!\n%s", nmnode.attribute(L"set1").value());
					}
				}
			}
		}
	}

	//create local AI template copy
	CAITemplate* aitemplate = new CAITemplate();

	//AI ignored events
	pugi::xml_node aiignorenode = rootnode.child( L"AI_IGNORE_EVENTS" );
	if ( aiignorenode != nullptr )
	{
		//parcurg nodurile de stari
		for ( pugi::xml_node statenode = aiignorenode.first_child(); statenode; statenode = statenode.next_sibling() )
		{
			EAIEventType nevttype = (EAIEventType)GetListIndexByName( statenode.attribute( L"type" ).value(), EAIEventTypeNames, ARRAY_SIZE( EAIEventTypeNames ) );
			if ( nevttype >= 0 )
			{
				aitemplate->m_arrIgnoredEvents.Add( nevttype );
			}
		}
	}

	//AI template
	pugi::xml_node ainode = rootnode.child( L"AI" );
	if ( ainode != nullptr )
	{
		// parse all states
		for each( auto& statenode in ainode.children() )
		{
			CAIState* nstate = new CAIState();
			nstate->name.Init( statenode.attribute( L"name" ).value() );
			nstate->nPriority = statenode.attribute( L"nPriority" ).as_int();
			// read state probability and set to 100.0 if missing
			nstate->fProbability = statenode.attribute( L"fProbability" ).as_float();
			if ( nstate->fProbability == 0.0f )
				nstate->fProbability = 100.0f;
			// find triggers
			pugi::xml_node triggersparent = statenode.child( L"TRIGGERING_EVENTS" );
			if ( triggersparent != null )
			{
				for ( pugi::xml_node eventnode = triggersparent.first_child(); eventnode; eventnode = eventnode.next_sibling() )
				{
					CStringHash evtTypeStr( eventnode.attribute( L"type" ).value() );
					EAIEventType nevt = K_AIEVT_NONE;
					// handle "ANY" keyword
					if ( evtTypeStr.textHash == FastHash( L"any" ) )
						nevt = K_AIEVT_ANY;
					else
						nevt = (EAIEventType)GetListIndexByName( eventnode.attribute( L"type" ).value(), EAIEventTypeNames, ARRAY_SIZE( EAIEventTypeNames ) );

					nstate->m_arrTriggeringEventTypes.Add( nevt );
				}
			}
			//find behaviors
			pugi::xml_node behaviorsparent = statenode.child( L"BEHAVIORS" );
			if ( behaviorsparent != null )
			{
				for ( pugi::xml_node behnode = behaviorsparent.first_child(); behnode; behnode = behnode.next_sibling() )
				{
					CAIBehavior nbeh;
					nbeh.nType = (EAIBehaviorType)GetListIndexByName( behnode.attribute( L"name" ).value(), EAIBehaviorTypeNames, ARRAY_SIZE( EAIBehaviorTypeNames ) );
					//salvam cativa params generici
					if ( !behnode.attribute( L"bCanInterrupt" ).empty() )
						nbeh.bCanInterrupt = behnode.attribute( L"bCanInterrupt" ).as_bool();
					if ( !behnode.attribute( L"bIgnoreEvents" ).empty() )
						nbeh.bIgnoreEvents = behnode.attribute( L"bIgnoreEvents" ).as_bool();
					if ( !behnode.attribute( L"fBehaviorDuration" ).empty() )
						nbeh.fBehaviorDuration = behnode.attribute( L"fBehaviorDuration" ).as_float();
					//read all behavior specific attributes
					for ( pugi::xml_attribute_iterator ait = behnode.attributes_begin(); ait != behnode.attributes_end(); ++ait )
					{
						// jump over generic params and add all others 
						if ( ait->internal_object() == behnode.attribute( L"name" ).internal_object() )
							continue;
						if ( ait->internal_object() == behnode.attribute( L"bCanInterrupt" ).internal_object() )
							continue;
						if ( ait->internal_object() == behnode.attribute( L"bIgnoreEvents" ).internal_object() )
							continue;
						if ( ait->internal_object() == behnode.attribute( L"fBehaviorDuration" ).internal_object() )
							continue;

						WCHAR wval[MAX_PATH];
						StringCchCopy( wval, MAX_PATH, ait->value() );
						nbeh.m_vcolParams.SetVarAUTO( ait->name(), wval );
					}

					nstate->m_arrBehaviors.Add( nbeh );
				}
			}

			aitemplate->m_arrStates.Add( nstate );
		}
	}
	// add the AItemplate to the list and save pointer to it in the actor template
	m_arrAItemplates.Add( aitemplate );
	templ->AItemplate = aitemplate;

	LOG_DBG( L"ActTemplates_Add - added template: %s", templ->shSourceXML.text );

	//finished loading template
	m_arrTemplatesActor.Add( templ );

	return templ;
}


void CLevel::KillActor( CActor* actor, bool bSplatTarget )
{
	CBullet bullet;
	bullet.fDamage = actor->_template.fLife;
	bullet.nFlags |= K_LVL_BULLET_FLAG_IGNORE_ARMOR | K_LVL_BULLET_FLAG_IGNORE_COVER | K_LVL_BULLET_FLAG_NO_IMPACT_PARTICLES | K_LVL_BULLET_FLAG_NOT_BALLISTIC | K_LVL_BULLET_FLAG_NO_DECALS;
	bullet.actorClass = K_ACT_CLASS_TRAP;

	if ( bSplatTarget )
	{
		bullet.nFlags |= K_LVL_BULLET_FLAG_CAN_SPLAT;
		bullet.nFlags &= ~K_LVL_BULLET_FLAG_NO_DECALS;
		//very large damage
		bullet.fDamage = -actor->_template.fLife;
	}

	actor->HitActor( &bullet );
}

CActorTemplate* CLevel::Actor_GetTemplate( const WCHAR* templateName )
{
	UINT32 nameHash = FastHash( templateName );
	//get template now
	for ( int kk = 0; kk < m_arrTemplatesActor.GetSize(); kk++ )
	{
		if ( m_arrTemplatesActor[kk]->shID.getHash() == nameHash )
			return m_arrTemplatesActor[kk];
	}
	return nullptr;
}

CActorTemplate* CLevel::Actor_GetTemplate( const DWORD templateNameHash )
{
	if ( templateNameHash == 0 )
		return nullptr;

	for ( int kk = 0; kk < m_arrTemplatesActor.GetSize(); kk++ )
	{
		if ( m_arrTemplatesActor[kk]->shID.getHash() == templateNameHash )
			return m_arrTemplatesActor[kk];
	}

	return nullptr;
}

void CLevel::RandomizeTemplateActor( CActorTemplate* actTemplate )
{
	// don't randomize player
	if ( actTemplate->actorClass == K_ACT_CLASS_PLAYER )
		return;
	////randomizeaza vitezele cu 10%
	//if(actTemplate->moveMaxSpeed > 0.0f)
	//	actTemplate->moveMaxSpeed += m_rand.RandFloatSgn(actTemplate->moveMaxSpeed * 0.1f);
	//if (actTemplate->moveMinSpeed > 0.0f)
	//	actTemplate->moveMinSpeed += m_rand.RandFloatSgn(actTemplate->moveMinSpeed * 0.1f);
}

///--- IACTIVE ---
IActiveInterface* CLevel::GetIActiveInterfacePtr( int editorID )
{
	if ( editorID < 0 )
		return nullptr;
	//check actives
	for ( int ar = 0; ar < m_arrAreas.Count(); ar++ )
	{
		CLevelArea* area = m_arrAreas[ar];
		for ( int kk = 0; kk < area->m_arrProps.GetSize(); kk++ )
		{
			if ( area->m_arrProps[kk]->ID == editorID )
				return area->m_arrProps[kk];
		}
	}
	//check lights
	for ( int kk = 0; kk < m_arrLights.GetSize(); kk++ )
	{
		if ( m_arrLights[kk]->ID == editorID )
			return m_arrLights[kk];
	}
	//check collision boxes
	for ( int kk = 0; kk < m_arrColShapes.GetSize(); kk++ )
	{
		if ( m_arrColShapes[kk]->ID == editorID )
			return m_arrColShapes[kk];
	}
	// check actors
	for ( auto node : m_arrActors )
	{
		if ( node->m_data.ID == editorID )
			return &node->m_data;
	}

	return nullptr;
}

IActiveInterface* CLevel::GetIActiveInterfacePtr_byUID( UINT32 UID )
{
	if ( UID == 0 )
		return null;
	//check actives
	for ( int ar = 0; ar < m_arrAreas.Count(); ar++ )
	{
		CLevelArea* area = m_arrAreas[ar];
		for ( int kk = 0; kk < area->m_arrProps.GetSize(); kk++ )
		{
			if ( area->m_arrProps[kk]->GetUID() == UID )
				return area->m_arrProps[kk];
		}
	}
	// check actors
	for ( auto node : m_arrActors )
	{
		if ( node->m_data.GetUID() == UID )
			return &node->m_data;
	}
	//check lights - less probable
	for ( int kk = 0; kk < m_arrLights.GetSize(); kk++ )
	{
		if ( m_arrLights[kk]->GetUID() == UID )
			return m_arrLights[kk];
	}
	//check collision boxes - even less probable
	for ( int kk = 0; kk < m_arrColShapes.GetSize(); kk++ )
	{
		if ( m_arrColShapes[kk]->GetUID() == UID )
			return m_arrColShapes[kk];
	}

	return nullptr;
}

CActor* CLevel::GetActorByUID( UINT32 UID )
{
	if ( UID == 0 )
		return nullptr;
	for ( auto node : m_arrActors )
	{
		if ( node->m_data.GetUID() == UID )
			return &node->m_data;
	}
	return nullptr;
}

CLight* CLevel::GetLightByUID( UINT32 UID )
{
	if ( UID == 0 )
		return nullptr;
	for ( int kk = 0; kk < m_arrLights.Count(); kk++ )
	{
		if ( m_arrLights[kk]->GetUID() == UID )
			return m_arrLights[kk];
	}
	return nullptr;
}

CActor* CLevel::GetPlayerByUID( UINT32 UID )
{
	if ( UID == 0 )
		return nullptr;
	for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
	{
		if ( pPlayerActor[kk] == nullptr )
			continue;
		if ( pPlayerActor[kk]->UID == UID )
		{
			return pPlayerActor[kk];
		}
	}
	return nullptr;
}

CActor* CLevel::GetClosestPlayer( CActor* sourceActor, bool bIgnoreDead )
{
	float fMinDist = 100000.0f;
	CActor* plact = nullptr;
	for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
	{
		if ( pPlayerActor[kk] == nullptr )
			continue;
		EAIBehaviorType beh = pPlayerActor[kk]->GetCurrentBehavior();
		if ( (bIgnoreDead) && (beh == AI_BEHAVIOR_DEAD) )
			continue;
		float fDist = MUVec2Len( &(pPlayerActor[kk]->pos.xy - sourceActor->pos.xy) );
		if ( fDist < fMinDist )
		{
			plact = pPlayerActor[kk];
			fMinDist = fDist;
		}
	}
	return plact;
}

CActor* CLevel::GetClosestPlayer( Vec2 vSrcPos, bool bIgnoreDead )
{
	float fMinDist = 100000.0f;
	CActor* plact = null;
	for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
	{
		if ( pPlayerActor[kk] == null )
			continue;
		if ( (bIgnoreDead) && (pPlayerActor[kk]->GetCurrentBehavior() == AI_BEHAVIOR_DEAD) )
			continue;
		float fDist = MUVec2Len( &(pPlayerActor[kk]->pos.xy - vSrcPos) );
		if ( fDist < fMinDist )
		{
			plact = pPlayerActor[kk];
			fMinDist = fDist;
		}
	}
	return plact;
}

bool CLevel::IsNetworkPlayer( CActor* pPlayer )
{
	if ( pPlayer == nullptr )
		return false;

	return (pPlayer->nControllerInstanceID == K_CM_IID_NET1);
}

CProp* CLevel::GetActiveByUID( UINT32 UID )
{
	if ( UID == 0 )
		return nullptr;
	for ( int ar = 0; ar < m_arrAreas.Count(); ar++ )
	{
		CLevelArea* area = m_arrAreas[ar];
		for ( int kk = 0; kk < area->m_arrProps.Count(); kk++ )
		{
			if ( area->m_arrProps[kk]->GetUID() == UID )
				return area->m_arrProps[kk];
		}
	}
	return nullptr;
}

void CLevel::SetLevelState( ELevelState eNewState, int nLevelStateParam )
{
	//set actual state
	m_levelState = eNewState;
	switch ( m_levelState )
	{
		case K_LVL_STATE_PLAYING:
		{
			// save level start time
			m_arrStats[K_LVL_STATS_LEVEL_START_SEC] = (int)floor( fLocalTimeline );

			m_levelSubState = 0;
			m_levelStateTimer = 0.0f;

			CHAR ctxt[MAX_PATH];
			int nLevelIdx = g_userData[K_MEMID_SELECTED_LEVEL] + g_userData[K_MEMID_SELECTED_CHAPTER] * K_GAME_LEVELS_PER_CHAPTER;
			StringCchPrintfA( ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1 );

			if ( UTApp().IsGameNetworked() )
			{
				//mark sync start here, after loading the game
				UTApp().m_Settings.devnet_eSyncStatus = CApplicationSettings::K_NETGAME_SYNC_GET_READY;
				//send loaded level confirmation
				g_netlock.Net_SendGameplayCommand( g_netlock.K_GAMPLAYCMD_LEVEL_LOADED );

#ifdef ENABLE_CHAT_WINDOW
				//say: "press ENTER to chat"
				g_ChatWnd.AddLine( __Texts().strings[STR_ENTER_TO_CHAT]->sText, L"SYSTEM", K_CW_SYSTEM_COLOR );
#endif
				LOG( L"Level::SetLevelState - Started networked game!" );
				if ( m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE )
					ANALYTICS_EVENT( "level_start_net", ctxt, "playedTimes", g_levelStats[nLevelIdx].nPlayedTimes );
				else //custom level
				{
					StringCchPrintfA( ctxt, MAX_PATH, "lvlflag_%d", m_unLoadedLevelFlags );
					ANALYTICS_EVENT( "level_start_net_custom", ctxt, "val", 0 );
				}
			}
			else
			{
				LOG( L"Level::SetLevelState - Started game!" );
				if ( m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE )
					ANALYTICS_EVENT( "level_start", ctxt, "playedTimes", g_levelStats[nLevelIdx].nPlayedTimes );
				else //custom level
				{
					StringCchPrintfA( ctxt, MAX_PATH, "lvlflag_%d", m_unLoadedLevelFlags );
					ANALYTICS_EVENT( "level_start_custom", ctxt, "val", 0 );
				}
			}

		}
		break;
		case K_LVL_STATE_MISSION_ACCOMPLISHED:
		{
#ifdef ENABLE_CHAT_WINDOW
			g_ChatWnd.CancelInput();
#endif
			//save level finished time
			m_arrStats[K_LVL_STATS_LEVEL_END_SEC] = (int)floor( fLocalTimeline );
			//remove any interfaces that might be shown
			__GUI().RemoveAllLayers();

			SND_STOP_GROUP( "music", false, true );

			//SND_PLAY_ONCE(SNDIDX_ANNOUNCER_WIN, 0);
			//SND_PLAY_ONCE(SNDIDX_STINGER_WIN, 0);

			m_levelSubState = 0;
			m_levelStateTimer = 0.0f;

			//__Particles().AddStringDummy(K_PDUMMY_STRING_WIDEBAR, Vec2(0.0f, -50.0f), STR_MISSION_ACCOMPLISHED, FONTIDX_12_WOW, 1.0f, 1.8f, K_COLOR_SELECTED_TEXT);
			//enter level results sync
			if ( UTApp().IsGameNetworked() )
			{
				LOG( L"Net::Level: Signal mission accomplished." );
				g_netlock.Net_EnterLevelResults();
			}

			// activate coop achievement on local matches too
			int nPlayers = m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED] + m_arrStats[K_LVL_STATS_PL2_HAS_PLAYED];
			if ( nPlayers > 1 )
			{
				App_IncreaseGamestat( K_MEMID_GAMESTATS_COOP_GAMES_WON );
			}

			//remove hot join
			for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
			{
				if ( pPlayerActor[kk] != null )
				{
					m_arrPlayerSelStrategic[kk] = -1;
					//m_interfaceIGM.SetStrategicSelection(kk, -1);

				}

				if ( (m_arrPlayerSelHotJoin[kk] != -1) && (pPlayerActor[kk] == null) )
				{
					m_arrPlayerControllersIIDs[kk] = -1;
					m_arrPlayerSelHotJoin[kk] = -1;
					g_playerSelScr.m_arrPlayers[kk].nInstanceID = -1;
					//m_interfaceIGM.SetHotJoinSelection(kk, m_arrPlayerSelHotJoin[kk]);
				}
			}

		}
		break;
		case K_LVL_STATE_MISSION_FAILED:
		{
#ifdef ENABLE_CHAT_WINDOW
			g_ChatWnd.CancelInput();
#endif
			//save level finished time
			m_arrStats[K_LVL_STATS_LEVEL_END_SEC] = (int)floor( fLocalTimeline );
			//remove any interfaces that might be shown
			__GUI().RemoveAllLayers();

			SND_STOP_GROUP( "music", false, true );

			//SND_PLAY_ONCE(SNDIDX_ANNOUNCER_FAIL, 0);
			//SND_PLAY_ONCE(SNDIDX_STINGER_LOSE, 0);

			m_levelStateParam = nLevelStateParam; //reason why failed - stringIDX
			m_levelSubState = 0;
			m_levelStateTimer = 0.0f;

			//__Particles().AddStringDummy(K_PDUMMY_STRING_WIDEBAR, Vec2(0.0f, -50.0f), STR_MISSION_FAILED, FONTIDX_12_WOW, 1.0f, 1.8f, K_COLOR_SELECTED_TEXT);
			//enter level results sync
			if ( UTApp().IsGameNetworked() )
			{
				LOG( L"Net::Level: Signal mission failed." );
				g_netlock.Net_EnterLevelResults();
			}

			//remove hot join and strategic menu
			for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
			{
				if ( pPlayerActor[kk] != null )
				{
					m_arrPlayerSelStrategic[kk] = -1;
					//m_interfaceIGM.SetStrategicSelection(kk, -1);

				}
				if ( (m_arrPlayerSelHotJoin[kk] != -1) && (pPlayerActor[kk] == null) )
				{
					m_arrPlayerControllersIIDs[kk] = -1;
					m_arrPlayerSelHotJoin[kk] = -1;
					g_playerSelScr.m_arrPlayers[kk].nInstanceID = -1;
					//m_interfaceIGM.SetHotJoinSelection(kk, m_arrPlayerSelHotJoin[kk]);
				}
			}
		}
		break;

		default:
			LOG( L"[WARNING] Net::Level - SetLevelState state %d not handled!", eNewState );
			break;
	}
}

void CLevel::SetTimeMultiplier( float fMultiplier, float fDuration )
{
	m_fTimeMultiplier = fMultiplier;
	m_fTimeMultiplierDuration = fDuration;
	//la reset nu am durata
	if ( fMultiplier == 1.0f )
		m_fTimeMultiplierDuration = 0.0f;

	//play sound
	/*
	if ( fMultiplier < 1.0f )
	{
		SND_PLAY_ONCE( SNDIDX_TIME_SLOW, 0 );
		SND_PLAY_ONCE( SNDIDX_HEARTBEAT, DSBPLAY_LOOPING );
	}
	else if ( fMultiplier >= 1.0f )
	{
		SND_STOP( SNDIDX_HEARTBEAT, true );
	}
	*/
}

bool CLevel::NormalizeMouseCoords( int ControllerIID, float fAxisValue, bool bIsHorizontalAxis, float& ret_fAxisValue )
{
	ret_fAxisValue = fAxisValue;
	// level not loaded? return same coordinates
	if ( !m_bLoaded )
		return false;

	for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
	{
		if ( m_arrPlayerControllersIIDs[kk] == ControllerIID ) {
			CActor* pPlayer = pPlayerActor[kk];
			if ( pPlayerActor == nullptr )
			{
				ErrorBox( K_ERR_WARNING, L"NormalizeMouseCoords player pointer is missing! idx:", kk );
				return false;
			}

			if ( bIsHorizontalAxis )
			{
				// bring real screen to RT screen space
				Vec2 retpt = m_camLevelToScr.ScreenToWorld( Vec2( fAxisValue, 0.0f ) );
				// make coords relative to player
				retpt.x -= pPlayer->vHeart.xy_proj.x;
				// set final coords
				ret_fAxisValue = retpt.x;
				return true;
			}
			else
			{
				Vec2 retpt = m_camLevelToScr.ScreenToWorld( Vec2( 0.0f, fAxisValue ) );
				// make coords relative to player
				retpt.y -= pPlayer->vHeart.xy_proj.y;
				// set final coords
				ret_fAxisValue = retpt.y;
				return true;
			}
		}
	}

	ErrorBox( K_ERR_WARNING, L"NormalizeMouseCoords couldn't find player with ControllerIID:%d", ControllerIID );
	return false;
}


void CLevel::BuildDynamicGeometry( CAABB _camAABB )
{
	// rounding camera bbox so we fall on pixel edges
	CAABB camAABB( _camAABB );
	camAABB.vMin.x = floor( camAABB.vMin.x );
	camAABB.vMin.y = floor( camAABB.vMin.y );

	const int arrOccludersSize = 200;
	COccluderSegment arrOccluders[arrOccludersSize];
	///--- create vert buffers for lights ---
	for ( int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++ )
	{
		CLight* nl = m_visibleList.visible_lights.m_pData[kk];

		switch ( nl->type )
		{
			case K_LVL_LT_IES:
			case K_LVL_LT_POINT:
			{
				//--- create light volumes for shadow casting lights	---
				nl->m_nLightMeshIdx = -1;
				if ( nl->GetCastShadows() )
				{
#if defined(DEBUG_LIGHTS_SHOW_OCCLUDERS)
					// returns a list of segments that will form shadows (from both tiles and collision boxes)
					int nOccluders = GetOccluderSegments( nl->pos.xy, nl->bbox, arrOccluders, arrOccludersSize );

					const int temp_arrVertsSize = 1200 * 3;
					_VERTEX_PNCT4T4 temp_arrVerts[temp_arrVertsSize];

					//do not delete! shows occluders instead of mesh. Checked for consistency.
					int nVertCnt = 0;
					for ( int kk = 0; kk < nOccluders; kk++ )
					{
						Vec3 vnrm = arrOccluders[kk].vStart + arrOccluders[kk].vN * 10.0f;
						//temp_arrVerts[nVertCnt].pos = Vec3(nl->pos.xy.x, nl->pos.xy.y, 0.0f);
						temp_arrVerts[nVertCnt].pos = Vec3( vnrm.x, vnrm.y, 0.0f );
						temp_arrVerts[nVertCnt].color = 0x00ffffff; nVertCnt++;
						temp_arrVerts[nVertCnt].pos = Vec2ToVec3XY0( arrOccluders[kk].vStart );
						temp_arrVerts[nVertCnt].color = 0xff00ff00; nVertCnt++;
						temp_arrVerts[nVertCnt].pos = Vec2ToVec3XY0( arrOccluders[kk].vEnd );
						temp_arrVerts[nVertCnt].color = 0xff0000ff; nVertCnt++;
					}

					if ( nVertCnt > 3 )
					{
						m_bufferedPainter.BeginMesh( nl->m_nLightMeshIdx );
						m_bufferedPainter.AddTriangles( temp_arrVerts, nVertCnt / 3 );
						m_bufferedPainter.EndMesh();
					}

#else				
					if ( nl->IsDirty() )
					{
						// returns a list of segments that will form shadows (from both tiles and collision boxes)
						int nOccluders = GetOccluderSegments( nl->pos.xy, nl->bbox, arrOccluders, arrOccludersSize );

						// default branch for showing lights:
						if ( nOccluders > 0 )
						{
							// builds the light FOV as a triangle list mesh (sending rays or skyscraper method)
							nl->m_arrVertsCnt = FOVUtil::BuildOccludedVolume( nl->pos.xy, nl->color, arrOccluders, nOccluders, nl->m_arrVerts, K_LVL_LIGHT_MAX_VERTS );
						}

						nl->SetDirty( false );
					}

					// add tris from light cache buffer as light volume
					if ( nl->m_arrVertsCnt > 0 )
					{
						// adds dynamic mesh for light volume
						m_bufferedPainter.BeginMesh( nl->m_nLightMeshIdx );
						m_bufferedPainter.AddTriangles( nl->m_arrVerts, nl->m_arrVertsCnt / 3 );
						m_bufferedPainter.EndMesh();
					}
#endif					

				}
				else
				{
					Vec3 lcorners[4]; //ul, ur, dl, dr
					memcpy( lcorners, nl->lCorners, 4 * sizeof( Vec3 ) );
					// move mesh to light position (!z must remain 0!)
					lcorners[0].x += nl->pos.xy_proj.x; lcorners[0].y += nl->pos.xy_proj.y;
					lcorners[1].x += nl->pos.xy_proj.x; lcorners[1].y += nl->pos.xy_proj.y;
					lcorners[2].x += nl->pos.xy_proj.x; lcorners[2].y += nl->pos.xy_proj.y;
					lcorners[3].x += nl->pos.xy_proj.x; lcorners[3].y += nl->pos.xy_proj.y;
					//write final VS verts
					_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
					vul.pos = lcorners[0];
					vur.pos = lcorners[1];
					vdr.pos = lcorners[2];
					vdl.pos = lcorners[3];
					//set color
					vul.color = vur.color = vdl.color = vdr.color = nl->color;
					// triangles vb
					_VERTEX_PNCT4T4 lightRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
					lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
					lightRectV[3] = vur; lightRectV[4] = vdr; lightRectV[5] = vdl;

					m_bufferedPainter.BeginMesh( nl->m_nLightMeshIdx );
					m_bufferedPainter.AddTriangles( lightRectV, 2 );
					m_bufferedPainter.EndMesh();

					nl->SetDirty( false );
				}
			}
			break;
			case K_LVL_LT_PROJECTED_DIR:
			{
				//create light mesh - rotating the actual mesh isn't necessary
				Vec3 lcorners[4]; //ul, ur, dr, dl
				memcpy( lcorners, nl->lCorners, 4 * sizeof( Vec3 ) );
				//move mesh to final pos
				lcorners[0].x += nl->pos.xy.x; lcorners[0].y += nl->pos.xy.y;
				lcorners[1].x += nl->pos.xy.x; lcorners[1].y += nl->pos.xy.y;
				lcorners[2].x += nl->pos.xy.x; lcorners[2].y += nl->pos.xy.y;
				lcorners[3].x += nl->pos.xy.x; lcorners[3].y += nl->pos.xy.y;
				//scriu VS-ul final
				_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
				vul.pos = lcorners[0];
				vur.pos = lcorners[1];
				vdr.pos = lcorners[2];
				vdl.pos = lcorners[3];
				//setez culoarea
				vul.color = vur.color = vdl.color = vdr.color = nl->color;
				//light direction as normals but not really used
				vul.n = vur.n = vdl.n = vdr.n = nl->vnDir;

				_VERTEX_PNCT4T4 lightRectV[6];
				lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
				lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;

				//dynamic mesh index for light geometry
				nl->m_nLightMeshIdx = -1; //resetez idx mesh
				m_bufferedPainter.BeginMesh( nl->m_nLightMeshIdx );
				m_bufferedPainter.AddTriangles( lightRectV, 2 );
				m_bufferedPainter.EndMesh();
			}
			break;

			case K_LVL_LT_DIRECTIONAL:
			{
				//#TODO: should go away
				_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
				vul.pos = Vec3( camAABB.vMin.x, camAABB.vMin.y, 0.0f );
				vur.pos = Vec3( camAABB.vMax.x, camAABB.vMin.y, 0.0f );
				vdl.pos = Vec3( camAABB.vMin.x, camAABB.vMax.y, 0.0f );
				vdr.pos = Vec3( camAABB.vMax.x, camAABB.vMax.y, 0.0f );
				//set color
				vul.color = vur.color = vdl.color = vdr.color = nl->color;
				//build verts
				_VERTEX_PNCT4T4 lightRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
				lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
				lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;

				m_bufferedPainter.BeginMesh( nl->m_nLightMeshIdx );
				m_bufferedPainter.AddTriangles( lightRectV, 2 );
				m_bufferedPainter.EndMesh();
			}
			break;

			case K_LVL_LT_AMBIENTAL:
			{
				// ambiental light only influence the area where they reside, have the bbox the size of the area so we clip to camera rect
				// use BBOX_INI because bbox gets moved to light position
				CAABB realbb;
				CAABB lightbb = nl->bbox.GetSnapshot();
				AABB::Intersection( camAABB, lightbb, realbb );
				if ( realbb.GetArea() <= 0.0f )
				{
					nl->m_nLightMeshIdx = -1;
					break;
				}

				_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
				vul.pos = Vec3( realbb.vMin.x, realbb.vMin.y, 0.0f );
				vur.pos = Vec3( realbb.vMax.x, realbb.vMin.y, 0.0f );
				vdl.pos = Vec3( realbb.vMin.x, realbb.vMax.y, 0.0f );
				vdr.pos = Vec3( realbb.vMax.x, realbb.vMax.y, 0.0f );
				//set color
				vul.color = vur.color = vdl.color = vdr.color = nl->color;
				//build verts
				_VERTEX_PNCT4T4 lightRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
				lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
				lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;

				m_bufferedPainter.BeginMesh( nl->m_nLightMeshIdx );
				m_bufferedPainter.AddTriangles( lightRectV, 2 );
				m_bufferedPainter.EndMesh();
			}
			break;

		}
	}


	/// 2. other lights: bullets, particles, etc
	//PROPS lights - temp lights - gunshot lights, explo lights
	m_propsLightsMeshIdx = -1;

	/*
	m_bufferedPainter.BeginMesh( m_propsLightsMeshIdx );

	for(auto node : m_poolDoofers)
	{
		CDoofer* prop = &node->m_data;

		if ( prop->bMakesLight )
		{
			if ( prop->sprLight.animationIdx >= 0 )
			{
				// create mesh shape of light
				RectLTRB realrect = m_sprLights.GetAFrameBBox_real( prop->sprLight.animationIdx, 0 );

				if ( prop->fLightScaling != 1.0f )
				{
					CAABB realaabb;
					realaabb.Set( realrect );
					realaabb.Scale( prop->fLightScaling );
					realrect.left = realaabb.vMin.x; realrect.top = realaabb.vMin.y;
					realrect.right = realaabb.vMax.x; realrect.bottom = realaabb.vMax.y;
				}

				_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
				Vec2 bpos2D = node->m_data.physPt->m_data.pos;
				Vec3 bpos( node->m_data.physPt->m_data.pos.x, node->m_data.physPt->m_data.pos.y, 50.0f );
				vul.pos = Vec3( bpos.x + realrect.left, bpos.y + realrect.top, 0.0f );
				vur.pos = Vec3( bpos.x + realrect.right, bpos.y + realrect.top, 0.0f );
				vdl.pos = Vec3( bpos.x + realrect.left, bpos.y + realrect.bottom, 0.0f );
				vdr.pos = Vec3( bpos.x + realrect.right, bpos.y + realrect.bottom, 0.0f );

				float fLife = prop->fLightDuration;
				float fFadeTime = prop->fLightFadeOut;

				float alpha = 1.0f;
				if ( fLife > 0.0f )
				{
					if ( prop->fLightTimer < fFadeTime )
						alpha = prop->fLightTimer / fFadeTime;
					else if ( prop->fLightTimer > fLife )
						alpha = 0.0f;
					else if ( prop->fLightTimer > fLife - fFadeTime )
						alpha = ( ( fLife - prop->fLightTimer ) / fFadeTime );
				}

				float fOrigAlpha = DW_GETFALPHA( prop->sprLight.color );
				vul.color = vur.color = vdl.color = vdr.color = DW_COLORALPHA( prop->sprLight.color, fOrigAlpha * alpha );
				//setez coordonate textura spot
				RectLTRB lTexRect = m_sprLights.GetModuleRect_TexCoords( prop->sprLight.animationIdx, 0, 0 );
				//Coord de mapare pe RTT (tex2) se seteaza din shader
				vul.tex1 = Vec4( lTexRect.left, lTexRect.top, 0.0f, 0.0f );
				vur.tex1 = Vec4( lTexRect.right, lTexRect.top, 0.0f, 0.0f );
				vdl.tex1 = Vec4( lTexRect.left, lTexRect.bottom, 0.0f, 0.0f );
				vdr.tex1 = Vec4( lTexRect.right, lTexRect.bottom, 0.0f, 0.0f );

				vul.n = bpos - vul.pos;
				vur.n = bpos - vur.pos;
				vdl.n = bpos - vdl.pos;
				vdr.n = bpos - vdr.pos;

				_VERTEX_PNCT4T4 lightRectV[6]; //tex2 - back buffer mapping, tex1-light spot
				lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
				lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;

				m_bufferedPainter.AddTriangles( lightRectV, 2 );
			}
		}
	}
	//inchid meshul
	m_bufferedPainter.EndMesh();
	*/

	///--- water ---
	/*
	m_bufferedPainter.BeginMesh(m_waterMeshIdx);
	//salvez date textura apa
	float waterTexScale = 2.0f;
	float waterTexSize = m_texManager.arrTextures[m_waterTexIdx]->info.Width;

	for (int kk = 0; kk < m_visibleList.logic_colShapesSpecial.Count(); kk++)
	{
		if (m_visibleList.logic_colShapesSpecial.m_pData[kk]->type == K_LVL_COLL_TYPE_WATER)
		{
			CCollisionShape * col = m_visibleList.logic_colShapesSpecial.m_pData[kk];
			CAABB wbb; //water bbox
			if (AABB::Intersection(col->bbox, camAABB, wbb))
			{
				Vec2 texoff = col->bbox.vMin - wbb.vMin;
				//save water plys in a sigle mesh, clipped to screen rect

				//scriu VS-ul final
				_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
				vul.pos = Vec3(wbb.vMin.x, wbb.vMin.y, 0.0f);
				vur.pos = Vec3(wbb.vMax.x, wbb.vMin.y, 0.0f);
				vdl.pos = Vec3(wbb.vMin.x, wbb.vMax.y, 0.0f);
				vdr.pos = Vec3(wbb.vMax.x, wbb.vMax.y, 0.0f);
				//setez culoarea
				//#TODO: culoarea sa fie setata undeva in editor. Poate as putea sa pun control de culoare la collision boxuri...
				vul.color = vur.color = vdl.color = vdr.color = 0xaa30AFFF;// col->color;
				//setez coordonate textura apa
				Vec2 texul = (wbb.vMin * waterTexScale) / waterTexSize;
				Vec2 texdr = (wbb.vMax * waterTexScale) / waterTexSize;

				RECTLTRB_F lTexRect(texul.x, texul.y, texdr.x, texdr.y);
				//Coord de mapare pe RTT (tex2) se seteaza din shader
				vul.tex1 = Vec4(lTexRect.left, lTexRect.top, 0.0f, 0.0f);
				vur.tex1 = Vec4(lTexRect.right, lTexRect.top, 0.0f, 0.0f);
				vdl.tex1 = Vec4(lTexRect.left, lTexRect.bottom, 0.0f, 0.0f);
				vdr.tex1 = Vec4(lTexRect.right, lTexRect.bottom, 0.0f, 0.0f);
				//setez normalele finale
				vul.n = vur.n = vdl.n = vdr.n = Vec3(0.0f, 0.0f, 0.0f);
				//construiesc VB-ul exact
				_VERTEX_PNCT4T4 waterRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
				waterRectV[0] = vul; waterRectV[1] = vur; waterRectV[2] = vdl;
				waterRectV[3] = vur; waterRectV[4] = vdl; waterRectV[5] = vdr;

				m_bufferedPainter.AddTriangles(waterRectV, 2);
			}
		}
	}
	//inchid meshul apelor
	m_bufferedPainter.EndMesh();
	*/

	//4. FOW Fog of War
	/*
	m_bufferedPainter.BeginMesh(m_fogofwarMeshIdx);

	for (int kk = 0; kk < m_visibleList.logic_colShapesSpecial.Count(); kk++)
	{
		if (m_visibleList.logic_colShapesSpecial.m_pData[kk]->nType == K_LVL_COLL_TYPE_FOG_OF_WAR)
		{
			CCollisionShape * col = m_visibleList.logic_colShapesSpecial.m_pData[kk];
			CAABB wbb; //bbox
			if (AABB::Intersection(col->bbox, camAABB, wbb))
			{
				//scriu VS-ul final
				_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
				vul.pos = Vec3(wbb.vMin.x, wbb.vMin.y, 0.0f);
				vur.pos = Vec3(wbb.vMax.x, wbb.vMin.y, 0.0f);
				vdl.pos = Vec3(wbb.vMin.x, wbb.vMax.y, 0.0f);
				vdr.pos = Vec3(wbb.vMax.x, wbb.vMax.y, 0.0f);
				//setez culoarea (setata pe onload)
				vul.color = vur.color = vdl.color = vdr.color = col->color;
				//setez coordonate tex2 (nu se folosesc)
				Vec2 texul = wbb.vMin;
				Vec2 texdr = wbb.vMax;

				RectLTRB lTexRect(texul.x, texul.y, texdr.x, texdr.y);
				//Coord de mapare pe RTT (tex2) se seteaza din shader
				vul.tex1 = Vec4(lTexRect.left, lTexRect.top, 0.0f, 0.0f);
				vur.tex1 = Vec4(lTexRect.right, lTexRect.top, 0.0f, 0.0f);
				vdl.tex1 = Vec4(lTexRect.left, lTexRect.bottom, 0.0f, 0.0f);
				vdr.tex1 = Vec4(lTexRect.right, lTexRect.bottom, 0.0f, 0.0f);
				//setez normalele finale
				vul.n = vur.n = vdl.n = vdr.n = Vec3(0.0f, 0.0f, 0.0f);
				//construiesc VB-ul exact
				_VERTEX_PNCT4T4 fowRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
				fowRectV[0] = vul; fowRectV[1] = vur; fowRectV[2] = vdl;
				fowRectV[3] = vur; fowRectV[4] = vdl; fowRectV[5] = vdr;

				m_bufferedPainter.AddTriangles(fowRectV, 2);
			}
		}
	}
	//inchid meshul apelor
	m_bufferedPainter.EndMesh();
	*/

	///--- build buffered painter buffers ---
	m_bufferedPainter.BuildBuffers();

}


void CLevel::AddDirtyRect( int x, int y, int w, int h )
{
	//#TODO: should check existing dirty rects and only add if not contained. 
	m_arrDirtyRectsTL.emplace_back( RectXYWHi( x, y, w, h ) );
}

void CLevel::SetActorWeaponPerks( CActor* pActor, CWeapon* pWeapon )
{
	_ASSERT( (pWeapon != null) && (pActor != null) );

	// reset actor template to initial one
	pActor->_template = pActor->_template_ini;

	if ( !pWeapon->_template.shTemplateOverwrite.IsEmpty() )
	{
		CActorTemplate* updateTemplate = Actor_GetTemplate( pWeapon->_template.shTemplateOverwrite.textHash );
		if ( updateTemplate == null )
		{
			ErrorBox( K_ERR_WARNING, L"SetActorCurrentWeapon failed! Template %s not found for weapon %s!", pWeapon->_template.shTemplateOverwrite.text, pWeapon->_template.name.text );
		}
		//set animations from new template
		pActor->_template.AddGenericDataFromTemplate( updateTemplate );
		pActor->_template.OverwriteAnimsFromTemplate( updateTemplate );
	}
	//set the heart and gun vectors again
	//LoadActorBBoxAndPoints(pActor, K_LVL_ACT_ANIM_REF_POSE, 0);

	///--- PERKS ---
	//apply perks that change current weapon
	if ( (pActor->_template.actorClass == K_ACT_CLASS_PLAYER) && (pActor->nPlayerOrdinal >= 0) )
	{
		switch ( g_playerSelScr.m_arrPlayers[pActor->nPlayerOrdinal].eType )
		{
			case K_PSS_CLASS_ASSAULTER:
			{
				if ( pWeapon->_template.bulletTemplate.nGroup == K_LVL_BULLGROUP_BULLETS )
				{
					float fAccuracy = g_playerSelScr.GetUpgradeBarPercent( &g_playerSelScr.m_arrPlayers[pActor->nPlayerOrdinal], L"A1_ACCURACY" );
					pWeapon->_template.fSpreadFOV *= 1.0f - fAccuracy * 0.3f;
				}
			}
			break;
			case K_PSS_CLASS_RECON:
			{
				if ( pWeapon->_template.bulletTemplate.nGroup == K_LVL_BULLGROUP_BULLETS )
				{
					float fAccuracy = g_playerSelScr.GetUpgradeBarPercent( &g_playerSelScr.m_arrPlayers[pActor->nPlayerOrdinal], L"R1_GUNPLAY" );
					pWeapon->_template.fSpreadFOV *= 1.0f - fAccuracy * 0.3f;
				}
			}
			break;
			case K_PSS_CLASS_FBI_AGENT:
			{
				if ( pWeapon->_template.bulletTemplate.nGroup == K_LVL_BULLGROUP_BULLETS )
				{
					float fAccuracy = g_playerSelScr.GetUpgradeBarPercent( &g_playerSelScr.m_arrPlayers[pActor->nPlayerOrdinal], L"F1_HANDGUN" );
					pWeapon->_template.fSpreadFOV *= 1.0f - fAccuracy * 0.3f;
				}
			}
			break;
			case K_PSS_CLASS_SHIELD:
			{
				if ( pWeapon->_template.bulletTemplate.nGroup == K_LVL_BULLGROUP_BULLETS )
				{
					float fAccuracy = g_playerSelScr.GetUpgradeBarPercent( &g_playerSelScr.m_arrPlayers[pActor->nPlayerOrdinal], L"S1_HANDGUN" );
					pWeapon->_template.fSpreadFOV *= 1.0f - fAccuracy * 0.3f;
				}
			}
			break;
			case K_PSS_CLASS_BREACHER:
			{
			}
			break;
			case K_PSS_CLASS_OFFDUTYGUY:
			{
				if ( pWeapon->_template.bulletTemplate.nGroup == K_LVL_BULLGROUP_BULLETS )
				{
					float fAccuracy = g_playerSelScr.GetUpgradeBarPercent( &g_playerSelScr.m_arrPlayers[pActor->nPlayerOrdinal], L"O1_SHOOTING" );
					pWeapon->_template.fSpreadFOV *= 1.0f - fAccuracy * 0.3f;
				}
			}
			break;
		}
	}
}



void CLevel::SetActorDoT( CActor* act, CDamageOverTime::EDoTType eType, float fDuration, float fDamagePerSec, EActorClass eExcludedClass, EActorClass eFilterClass, DWORD dwOwnerUID )
{
	if ( act == nullptr )
		return;
	if ( (eFilterClass > K_ACT_CLASS_ANY) && (act->_template.actorClass != eFilterClass) )
		return;
	if ( (eExcludedClass > K_ACT_CLASS_ANY) && (act->_template.actorClass == eExcludedClass) )
		return;

	if ( (eType == CDamageOverTime::K_LVL_DoT_INTIMIDATED) && (act->fLife <= 0.0f) )
		return;

	//#HARDCODE: DoT_TARGETED only works on enemies
	if ( (eType == CDamageOverTime::K_LVL_DoT_TARGETED) && (act->_template.actorClass < K_ACT_CLASS_ENEMY) )
		return;

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	//LOG(L"- SetDoT %s for %.4f", act->templateActor.shName.text, eType);
#endif

	if ( act->cDamageOverTime.Set( eType, fDuration, fDamagePerSec, eExcludedClass, eFilterClass, dwOwnerUID ) )
	{
		//pointer to player owner or null if not a player
		CActor* pPlayerOwner = GetPlayerByUID( dwOwnerUID );
		//special statistics
		if ( (eType == CDamageOverTime::K_LVL_DoT_FIRE) && (act->_template.actorClass >= K_ACT_CLASS_ENEMY) )
		{
			if ( (pPlayerOwner != nullptr) && (!IsNetworkPlayer( pPlayerOwner )) )
				App_IncreaseGamestat( K_MEMID_GAMESTATS_ENEMIES_SET_ON_FIRE );
		}
	}
}



CActor* CLevel::GetClosestTarget( CActor* sourceActor, EActorClass eTargetClassFilter1, EActorClass eTargetClassFilter2 )
{
	if ( sourceActor == nullptr )
		return nullptr;
	//nobody attacks if level finished
	if ( m_levelState != K_LVL_STATE_PLAYING )
		return nullptr;
	//save some data about current actor:
	bool bAlerted = (sourceActor->fFOVPercent >= 0.9f) ? true : false;
	float fDistSee = sourceActor->_template.fDistSee;
	float fDistHear = 100.0f;// sourceActor->actTemplate.distHear;
	if ( bAlerted )
	{
		fDistHear = fDistSee;
	}
	CAABB aabbvision;

	aabbvision.Set_Corrected(
		Vec2( sourceActor->pos.xy.x - fDistSee, sourceActor->pos.xy.y - fDistSee ),
		Vec2( sourceActor->pos.xy.x + fDistSee, sourceActor->pos.xy.y + fDistSee )
	);

	//--- check all actors for enemy ---
	CActor* retvalenemy = nullptr;

	float minDistSq = 1000000.0f;
	for ( auto node : m_arrActors )
	{
		CActor* enemy = &node->m_data;
		if ( enemy == nullptr || enemy == sourceActor )
			continue;
		//never attack same class
		if ( enemy->_template.actorClass == sourceActor->_template.actorClass )
			continue;
		//don't attack same class enemies or traps and passive classes
		if ( enemy->_template.actorClass <= K_ACT_CLASSCHECKPOINT_NEUTRALS )
			continue;

		//filter enemy classes
		int nIgnore = 0, nIgnoreConditions = 0;
		if ( eTargetClassFilter1 != K_ACT_CLASS_ANY )
		{
			nIgnoreConditions++;
			if ( enemy->_template.actorClass != eTargetClassFilter1 )
				nIgnore++;
		}
		if ( eTargetClassFilter2 != K_ACT_CLASS_ANY )
		{
			nIgnoreConditions++;
			if ( enemy->_template.actorClass != eTargetClassFilter2 )
				nIgnore++;
		}
		if ( (nIgnoreConditions > 0) && (nIgnore == nIgnoreConditions) )
			continue;
		// ignore dead enemies
		if ( (enemy->fLife <= 0.0f) || ((enemy->_template.eCaps & K_ACT_CAPS_NOT_A_TARGET) != 0) )
			continue;

		Vec2 enemyDistV = enemy->GetPosHeart() - sourceActor->GetPosHeart();
		float enemyDistSq = MUVec2LenSq( &enemyDistV );
		// new enemy is too far?
		if ( enemyDistSq >= minDistSq )
			continue;
		//not in view rectangle
		if ( !aabbvision.PointIn( enemy->GetPosHeart() ) )
			continue;
		// line of sight is decided between weapon muzzle and enemy heart without projecting the coords on the floor because bullets do the same, they check screen coords
		if ( !IsLineOfSight( sourceActor->GetCurWeaponMuzzleWorld().xy, enemy->GetPosHeart3D().xy, sourceActor->pArea ) )
			continue;

		//passed all tests and is closer? set ptr on new one
		if ( (retvalenemy == nullptr) || (enemyDistSq < minDistSq) )
		{
			retvalenemy = enemy;
			minDistSq = enemyDistSq;
		}
	}

	return retvalenemy;
}


CActor* CLevel::GetClosestActorByTemplateName( CActor* sourceActor, WCHAR* sTargetTemplateName, float fMaxDistance )
{
	_ASSERT( sourceActor != nullptr );

	CActor* retvalenemy = nullptr;
	UINT32 nTargetNameHash = FastHash( sTargetTemplateName );

	for ( auto node : m_arrActors )
	{
		CActor* enemy = &node->m_data;
		if ( (enemy == nullptr) || (enemy == sourceActor) || (enemy->_template.shID.textHash != nTargetNameHash) || (!enemy->IsAlive()) )
			continue;
		if ( (enemy->_template.eCaps & K_ACT_CAPS_NOT_A_TARGET) != 0 )
			continue;

		Vec2 enemyDistV = enemy->GetPosHeart() - sourceActor->GetPosHeart();
		float enemyDistSq = MUVec2LenSq( &enemyDistV );
		//daca e prea departe trece mai departe
		float fSearchRadiusSq = (fMaxDistance * fMaxDistance);
		if ( enemyDistSq > fSearchRadiusSq )
		{
			continue;
		}
		//daca e destul de aproape:
		//verifica daca am linie directa de vedere
		if ( !IsLineOfSight( sourceActor->GetPosHeart(), enemy->GetPosHeart(), sourceActor->pArea ) )
			continue;

		//daca a trecut toate testele si inamicul curent este mai aproape decat cel selectat initial il setez pe cel nou
		if ( (retvalenemy == null) || (MUVec2LenSq( &(retvalenemy->GetPosHeart() - sourceActor->GetPosHeart()) ) > enemyDistSq) )
			retvalenemy = enemy;
	}

	return retvalenemy;
}

void CLevel::AddAIEvent( EAIEventType eventType, UINT32 ownerUID, EActorClass ownerClass, Vec2 vPos, float radius, float duration )
{
	// negative radius = infinite radius
	if ( (radius == 0.0f) || (duration <= 0.0f) )
		return;
	//vad daca am deja un event cu acelasi owner si acelasi event il suprascriu pe cel vechi ca sa nu fie mai multe
	CAIEvent* nevt = nullptr;
	//if owner is 0 means generic AI event (alert sounds)
	if ( ownerUID != 0 )
	{
		for ( int kk = 0; kk < m_arrAIevents.GetSize(); kk++ )
		{
			if ( (m_arrAIevents[kk]->ownerUID == ownerUID) && (m_arrAIevents[kk]->nType == eventType) )
			{
				nevt = m_arrAIevents[kk];
				break;
			}
		}
	}
	// only add event if new
	if ( nevt == nullptr )
	{
		nevt = new CAIEvent();
		m_arrAIevents.Add( nevt );
	}
	//set event data
	nevt->ownerUID = ownerUID;
	nevt->nType = eventType;
	nevt->fRadius = radius;
	nevt->fDuration = duration;
	nevt->pos = vPos;
	nevt->ownerClass = ownerClass;
}


void CLevel::CleanupDeadObjects()
{
	//check active objects
	for ( int ar = 0; ar < m_arrAreas.Count(); ar++ )
	{
		CLevelArea* area = m_arrAreas[ar];
		if ( !area->bActive )
			continue;
		// check props lifetime
		for ( int kk = area->m_arrProps.GetSize() - 1; kk >= 0; kk-- )
		{
			if ( area->m_arrProps[kk]->CanBeReleased() )
			{
				// remove from array, call dtor
				LOG( L"Released Prop: %d", area->m_arrProps[kk]->ID );
				SAFE_DELETE( area->m_arrProps[kk] );
				area->m_arrProps.Remove( kk );
			}
		}
	}

	//check actors
	for ( auto node : m_arrActors )
	{
		CActor* act = &node->m_data;
		if ( act->CanBeReleased() )
		{
			LOG( L"Released actor: %s", act->_template.shID.text );
			// now release it
			act->Dispose();
			m_arrActors.Dismiss( node );
		}
	}

	//check lights
	for ( int kk = m_arrLights.GetSize() - 1; kk >= 0; kk-- )
	{
		if ( m_arrLights[kk]->CanBeReleased() )
		{
			SAFE_DELETE( m_arrLights[kk] );
			m_arrLights.Remove( kk );
		}
	}

	//check bullets
	for ( auto node : m_poolBullets )
	{
		CBullet* bullet = &node->m_data;
		if ( bullet->bPendingKill )
		{
			m_poolBullets.Dismiss( node );
		}
	}
}

void CLevel::UpdateAI( float dTime, bool bInEditor )
{
	//reset targets left (will be counted below)
	m_arrStats[K_LVL_STATS_TARGETS_LEFT] = 0;

	//update AI events
	for ( int kk = m_arrAIevents.GetSize() - 1; kk >= 0; kk-- )
	{
		CAIEvent* evt = m_arrAIevents[kk];
		evt->fDuration -= dTime;
		if ( evt->fDuration <= 0.0f )
		{
			SAFE_DELETE( evt );
			m_arrAIevents.Remove( kk );
		}
	}

	//check active objects
	for ( int ar = 0; ar < m_arrAreas.Count(); ar++ )
	{
		CLevelArea* area = m_arrAreas[ar];
		if ( !area->bActive )
			continue;
		for ( int kk = area->m_arrProps.GetSize() - 1; kk >= 0; kk-- )
		{
			area->m_arrProps[kk]->Update( dTime );
		}
	}
	//#TODO: only update lights and col shapes in activated areas
	//check lights
	for ( int kk = 0; kk < m_arrLights.GetSize(); kk++ )
	{
		m_arrLights[kk]->Update( dTime, *this );
	}
	//check collision boxes
	for ( int kk = m_arrColShapes.GetSize() - 1; kk >= 0; kk-- )
	{
		m_arrColShapes[kk]->Update( dTime, *this );
	}

	//check actors - must be done after moving platforms (usually last is best)
	double fHashKey = 0.0f;
	if ( !bInEditor )
	{
		for ( auto node : m_arrActors )
		{
			CActor* act = &node->m_data;
			act->Update( dTime );
			//add some floats to detect network inconsistencies
			fHashKey += act->pos.xyz.x + act->pos.xyz.y + act->fLife + act->fArmor + act->fStunTimer;

			//count targets left
			if ( act->GetCurrentBehavior() != EAIBehaviorType::AI_BEHAVIOR_DEAD )
			{
				if ( (act->_template.actorClass >= K_ACT_CLASS_ENEMY) || (act->_template.actorClass == K_ACT_CLASS_HOSTAGE) )
				{
					m_arrStats[K_LVL_STATS_TARGETS_LEFT]++;
				}
			}
		}
	}
	{
#if defined(K_NET_STRICT_SYNC_CHECK)
		//build hash
		WCHAR strKey[MAX_PATH];
		StringCchPrintf( strKey, MAX_PATH, L"%.9g", fHashKey );
		m_dwSyncCheckHash = FastHash( strKey );
#else
		m_dwSyncCheckHash = 0;
#endif
	}
}

int CLevel::GetNextRandomLevel()
{
	static int nNextIndex = UTGetChaptersList().GetTotalLevelsCnt();
	static int arrLevels[120] = { 0 };
	static int nValidLevels = 0;

	if ( nNextIndex >= nValidLevels )
	{
		nNextIndex = 0;
		nValidLevels = 0;
		for ( int kk = 0; kk < UTGetChaptersList().GetTotalLevelsCnt(); kk++ )
		{
			if ( g_levelStats[kk].nLevelType != K_GAME_LSTYPE_NOTSET )
				arrLevels[nValidLevels++] = kk;
		}
		//Random_ShuffleArray(arrLevels, nValidLevels, 1000);
	}

	int nLevel = arrLevels[nNextIndex];
	nNextIndex++;
	return nLevel;
}


int CLevel::Local_ComputeMissionXP( int nStars )
{
	int nTotalXPPoints = 0;
	if ( nStars > 0 )
		nTotalXPPoints = 50;
	//Player 1
	int nXPpl1 = m_arrStats[K_LVL_STATS_PL1_KILLS] * 10 + m_arrStats[K_LVL_STATS_PL1_HOSTAGES_SAVED] * 20;
	if ( nXPpl1 < 0 ) nXPpl1 = 0;
	//Player 2
	int nXPpl2 = m_arrStats[K_LVL_STATS_PL2_KILLS] * 10 + m_arrStats[K_LVL_STATS_PL2_HOSTAGES_SAVED] * 20;
	if ( nXPpl2 < 0 ) nXPpl2 = 0;

	nTotalXPPoints += nXPpl1 + nXPpl2;
	//add common stuff
	nTotalXPPoints += m_arrStats[K_LVL_STATS_LEVEL_ARREST_TARGETS_ARRESTED] * 50;
	nTotalXPPoints += m_arrStats[K_LVL_STATS_BOMBS_DISARMED] * 50;

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	LOG( L"[Level] Mission total XP: %d", nTotalXPPoints );
#endif
	return nTotalXPPoints;
}

void CLevel::UpdateFixedTimestep( float dTime_original )
{
	if ( !m_bLoaded )
		return;

	///--- time control ---
	//variatie time multiplier
	if ( m_fTimeMultiplierDuration > 0.0f )
	{
		m_fTimeMultiplierDuration -= dTime_original;
		if ( m_fTimeMultiplierDuration <= 0.0f )
		{
			//cand durata scade la 0 revin la timeline original
			m_fTimeMultiplier = 1.0f;
			SND_STOP( SNDIDX_HEARTBEAT, true );
		}
	}
	REACH_VALUE_LINEAR( m_fTimeMultiplier_real, m_fTimeMultiplier, dTime_original );
	//set sounds freq global
	SND_SET_GROUP_FREQUENCY( "ingame", m_fTimeMultiplier_real, false );

	//calcul dTime final
	float dTime = dTime_original * m_fTimeMultiplier_real;
	fLocalTimeline += dTime;

	//update local timers
	m_Timers.Update( dTime );


	//state machine logic
	switch ( m_levelState )
	{
		case K_LVL_STATE_PLAYING:
		{
			//set to true to enable hot join
			static const bool bEnableHotJoin = false;
			///--- handle controllers dynamically and hot join ---
			for ( int plidx = 0; plidx < K_MAX_PLAYERS_CNT; plidx++ )
			{
				//hot join: enters here only once, for new controllers only
				if ( m_arrPlayerControllersIIDs[plidx] == -1 ) //if empty check if fire was pressed on another ctrlr and set it to this player
				{
					//comment next line to enable first ingame hotjoin
					//if(!bEnableHotJoin)
						//continue;
					//HOT JOIN LOGIC
					for ( auto ctrlr : __Controllers().m_arrControllers )
					{
						//Shows controller mapping - only when not online
						if ( (ctrlr->eType == K_CM_CT_JOYSTICK_SDL) && (!UTApp().IsGameNetworked()) && (false == __GUI().bIsBlocking) &&
							(ctrlr->sCommands.keyState[K_CM_COMMAND_SELECT] == K_CM_BUTSTATE_JUSTPRESSED) )
						{
							__GUI().ShowLayerOnce( "LAYER_ID_CONTROLLER_MAP" );
						}
						//when player was left without controller give him the new controller when ctrlr touched
						bool bActivate = false;
						if ( pPlayerActor[plidx] != null ) //setting controller for player with disconnected controller
						{
							bActivate = ctrlr->WasControllerTouched( true );
						}
						else //joining now
						{
							bActivate = ((ctrlr->sCommands.keyState[K_CM_COMMAND_FIRE1] == K_CM_BUTSTATE_JUSTRELEASED) ||
								(ctrlr->sCommands.keyState[K_CM_COMMAND_JUMP] == K_CM_BUTSTATE_JUSTRELEASED));
						}

						if ( (ctrlr != null) && (bActivate) )
						{
							bool bAlreadyUsed = false;
							for ( int jj = 0; jj < K_MAX_PLAYERS_CNT; jj++ )
							{
								if ( m_arrPlayerControllersIIDs[jj] == ctrlr->nSDLInstanceId )
								{
									bAlreadyUsed = true;
									break;
								}
							}
							//daca nu e folosit il seteaza playerului caruia ii lipseste
							if ( !bAlreadyUsed )
							{
								//save ctrlr ID
								m_arrPlayerControllersIIDs[plidx] = ctrlr->nSDLInstanceId;
								//update selection screen too !!! used in respawn
								g_playerSelScr.m_arrPlayers[plidx].nInstanceID = ctrlr->nSDLInstanceId;
								//load saved type for panel
								EPSSPlayerClass eType = (EPSSPlayerClass)g_userData[K_MEMID_PANEL1_CLASS + plidx * (K_MEMID_PANEL2_CLASS - K_MEMID_PANEL1_CLASS)];
								//set hot join selection
								m_arrPlayerSelHotJoin[plidx] = (int)eType;
								//daca nu a fost facuta selectie in selScreen pun pe default first class
								if ( (m_arrPlayerSelHotJoin[plidx] < 0) || (m_arrPlayerSelHotJoin[plidx] >= K_PSS_CLASSES_COUNT) )
								{
									m_arrPlayerSelHotJoin[plidx] = K_PSS_CLASS_ASSAULTER;
								}
								break;
							}
						}
					}
				}
				else // controller not empty, check it
				{
					CController* ctrlr = __Controllers().GetControllerByInstanceID( m_arrPlayerControllersIIDs[plidx] );
					if ( ctrlr == null )
					{
						m_arrPlayerControllersIIDs[plidx] = -1;
					}
					else //pentru hot join char selection
					{
						//he played before, must select again (HOT JOIN)
						if ( m_arrPlayerSelHotJoin[plidx] == -1 )
						{
							if ( bEnableHotJoin )
							{
								if ( (ctrlr->sCommands.keyState[K_CM_COMMAND_FIRE1] == K_CM_BUTSTATE_JUSTRELEASED) ||
									(ctrlr->sCommands.keyState[K_CM_COMMAND_JUMP] == K_CM_BUTSTATE_JUSTRELEASED) )
								{
									//set hot join selection
									m_arrPlayerSelHotJoin[plidx] = (int)g_playerSelScr.m_arrPlayers[plidx].eType;
									//daca nu a fost facuta selectie in selScreen pun pe default first class
									if ( m_arrPlayerSelHotJoin[plidx] < 0 )
									{
										m_arrPlayerSelHotJoin[plidx] = K_PSS_CLASS_ASSAULTER;
									}

									//m_interfaceIGM.SetHotJoinSelection(plidx, m_arrPlayerSelHotJoin[plidx]);
								}
							}
						}
						else if ( (m_arrPlayerSelHotJoin[plidx] != -1) && (pPlayerActor[plidx] == nullptr) )
						{
							bool bCheckSpawn = false;
							//played before: spawn it immediately
							if ( m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED + plidx * K_LVL_STATS_PLAYER_STATS_COUNT] == 1 )
							{
								bCheckSpawn = true;
							}
							else
							{
								if ( (ctrlr->sCommands.keyState[K_CM_COMMAND_FIRE1] == K_CM_BUTSTATE_JUSTRELEASED) ||
									(ctrlr->sCommands.keyState[K_CM_COMMAND_JUMP] == K_CM_BUTSTATE_JUSTRELEASED) )
									bCheckSpawn = true;
							}

							if ( bCheckSpawn )
							{
								//spawn pos
								Vec2 vSpawnPos = vLastSpawnPoint;
								CAABB aabbSpawn;
								CAABB* p_aabbPeer = nullptr;
								aabbSpawn.Set( vSpawnPos.x - 5.0f, vSpawnPos.y - 22.0f, vSpawnPos.x + 5.0f, vSpawnPos.y );

								int nOtherPlayerIdx = (plidx + 1) % K_MAX_PLAYERS_CNT;
								bool bSpawnIt = false;
								//always spawn near the other player when COOP
								if ( (pPlayerActor[nOtherPlayerIdx] != nullptr) && (pPlayerActor[nOtherPlayerIdx]->collisionFlags & K_DIRFLAG_DOWN) )
								{
									//only spawn if player is there
									EAIBehaviorType eOtherBehave = pPlayerActor[nOtherPlayerIdx]->GetCurrentBehavior();
									if ( (eOtherBehave == AI_BEHAVIOR_PLAYER_CONTROL) || (eOtherBehave == AI_BEHAVIOR_DEAD) )
									{
										vSpawnPos = pPlayerActor[nOtherPlayerIdx]->pos.xy;
										aabbSpawn = pPlayerActor[nOtherPlayerIdx]->bbox;
										p_aabbPeer = &pPlayerActor[nOtherPlayerIdx]->bbox;
										bSpawnIt = true;
									}
								}
								else if ( pPlayerActor[nOtherPlayerIdx] == nullptr )
								{
									bSpawnIt = true;
								}

								if ( bSpawnIt )
								{
									//set selScreen too for next spawn. If player is different from the selection it resets the selection
									if ( g_playerSelScr.m_arrPlayers[plidx].eType != (EPSSPlayerClass)m_arrPlayerSelHotJoin[plidx] )
									{
										g_playerSelScr.m_arrPlayers[plidx].Init( (EPSSPlayerClass)m_arrPlayerSelHotJoin[plidx] );
										g_playerSelScr.m_arrPlayers[plidx].bSelected = true; //marcheaza ca si cum as fi selectat in ecranul anterior
									}
									g_playerSelScr.m_arrPlayers[plidx].nInstanceID = ctrlr->nSDLInstanceId;
									//save hotjoin selection?
									g_playerSelScr.SaveSelection();

									bool bNeverPlayed = false;
									if ( m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED + plidx * K_LVL_STATS_PLAYER_STATS_COUNT] == 0 )
										bNeverPlayed = true;

									//spawn it
									//if (GetBestSpawningPos(&vSpawnPos, aabbSpawn, p_aabbPeer))
									{
										SpawnPlayer( vSpawnPos, plidx );
									}

									//achievements and level stats
									if ( !bNeverPlayed )
										IncreaseLevelStatistics( K_LVL_STATS_PL1_USE_EXTRA_LIFE_CNT + pPlayerActor[plidx]->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT );

									//say spawn verse
	//									PlayActorSoundVerse(pPlayerActor[plidx], K_LVL_ACT_VERSE_JOIN_GAME);

										//scad numarul de vieti si anunt interfata
									if ( m_arrStats[K_LVL_STATS_PL1_LIVES + plidx * K_LVL_STATS_PLAYER_STATS_COUNT] > 0 )
										m_arrStats[K_LVL_STATS_PL1_LIVES + plidx * K_LVL_STATS_PLAYER_STATS_COUNT]--;

									//m_interfaceIGM.SetLivesLeft(m_arrStats[K_LVL_STATS_PL1_LIVES], m_arrStats[K_LVL_STATS_PL2_LIVES]);
								}
								else
								{
									//SND_PLAY_ONCE(SNDIDX_DENIED);
								}
							}
							//here you can change character when hot joining (not having played before)
							/*
							#DMC: commented out 13 oct 2020
							if (m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED + plidx * K_LVL_STATS_PLAYER_STATS_COUNT] == 0)
							{
								if (ctrlr->sCommands.keyState[K_CM_COMMAND_LEFT] == K_CM_BUTSTATE_JUSTPRESSED)
								{
									m_arrPlayerSelHotJoin[plidx]--;
									if (m_arrPlayerSelHotJoin[plidx] < 0)
										m_arrPlayerSelHotJoin[plidx] = K_PSS_CLASSES_COUNT - 1;
								}
								else if (ctrlr->sCommands.keyState[K_CM_COMMAND_RIGHT] == K_CM_BUTSTATE_JUSTPRESSED)
								{
									m_arrPlayerSelHotJoin[plidx]++;
									if (m_arrPlayerSelHotJoin[plidx] >= K_PSS_CLASSES_COUNT)
										m_arrPlayerSelHotJoin[plidx] = 0;
								}
							}
							*/
						}
					}
				}

				//update player controller
				if ( pPlayerActor[plidx] != null )
				{
					int nOldIID = pPlayerActor[plidx]->nControllerInstanceID;
					//update player ctrlr
					pPlayerActor[plidx]->nControllerInstanceID = m_arrPlayerControllersIIDs[plidx];
					//re-initialize igm interface when changing controller (update helper strings)
					if ( (nOldIID < 0) && (m_arrPlayerControllersIIDs[plidx] >= 0) )
					{
						//set interface pointers
						//m_interfaceIGM.Init(&m_sprInterface, pPlayerActor[0], pPlayerActor[1]);
					}
				}
				else //hot join ingame selection and spawning
				{
					//m_interfaceIGM.SetHotJoinSelection(plidx, m_arrPlayerSelHotJoin[plidx]);
				}

				///--- updates player selection for strategic points ---
				//#DMC: commented out 13 oct 2020
				/*
				if (m_arrPlayerSelStrategic[plidx] >= 0)
				{
					CController* ctrlr = UTGetControllersManager().GetControllerByInstanceID(m_arrPlayerControllersIIDs[plidx]);

					if ((ctrlr == null) || (ctrlr->sCommands.keyState[K_CM_COMMAND_STRATEGIC_MENU] != K_CM_BUTSTATE_PRESSING))
					{
						//exit
						m_arrPlayerSelStrategic[plidx] = -1;
						m_interfaceIGM.SetStrategicSelection(plidx, -1);
						//#HACK:skip next just pressed check
						ctrlr->sCommands.keyState[K_CM_COMMAND_STRATEGIC_MENU] = K_CM_BUTSTATE_PRESSING;
						//play a sound on opening the interface
						SND_PLAY(SNDIDX_DENIED);
						//remove icon
						pPlayerActor[plidx]->SetIcon(K_LVL_ACT_ICON_NONE);
					}
					else if ((ctrlr->sCommands.keyState[K_CM_COMMAND_FIRE1] == K_CM_BUTSTATE_JUSTRELEASED) ||
						(ctrlr->sCommands.keyState[K_CM_COMMAND_JUMP] == K_CM_BUTSTATE_JUSTRELEASED))
					{
						int nAbility = m_arrPlayerSelStrategic[plidx];
						//select it
						if (ActivateSpecialAbility(nAbility, plidx))
						{
							//exit
							m_arrPlayerSelStrategic[plidx] = -1;
							m_interfaceIGM.SetStrategicSelection(plidx, -1);
							//remove icon
							pPlayerActor[plidx]->SetIcon(K_LVL_ACT_ICON_NONE);
							//make sure we disable the tutorial
							g_userData[K_MEMID_TUT_INTERFACE_STRATEGIC] = 1;
						}
						else
						{
							SND_PLAY(SNDIDX_DENIED);
						}
					}
					else if (ctrlr->sCommands.keyState[K_CM_COMMAND_LEFT] == K_CM_BUTSTATE_JUSTPRESSED)
					{
						int nSelectedIdxNew = m_arrPlayerSelStrategic[plidx];
						do {
							nSelectedIdxNew--;
							//rollover
							if (nSelectedIdxNew < 0)
								nSelectedIdxNew = K_LVL_MAX_STRATEGIC_POINTS - 1;
						} while (m_arrStrategicAbilities[plidx][nSelectedIdxNew] < 0);
						CLAMP(nSelectedIdxNew, 0, K_LVL_MAX_STRATEGIC_POINTS - 1);

						SND_PLAY(SNDIDX_CLICK);
						m_arrPlayerSelStrategic[plidx] = nSelectedIdxNew;
						m_interfaceIGM.SetStrategicSelection(plidx, nSelectedIdxNew);
					}
					else if (ctrlr->sCommands.keyState[K_CM_COMMAND_RIGHT] == K_CM_BUTSTATE_JUSTPRESSED)
					{
						int nStatIdx = K_LVL_STATS_PL1_STRATEGIC_POINTS + plidx * K_LVL_STATS_PLAYER_STATS_COUNT;
						int nMaxPoint = (int)floor(m_arrStats[nStatIdx] / 1000.0f);


						int nSelectedIdxNew = m_arrPlayerSelStrategic[plidx];
						do {
							nSelectedIdxNew++;
							//rollover
							if (nSelectedIdxNew >= K_LVL_MAX_STRATEGIC_POINTS)
								nSelectedIdxNew = 0;
						} while (m_arrStrategicAbilities[plidx][nSelectedIdxNew] < 0);
						CLAMP(nSelectedIdxNew, 0, K_LVL_MAX_STRATEGIC_POINTS - 1);

						SND_PLAY(SNDIDX_CLICK);
						m_arrPlayerSelStrategic[plidx] = nSelectedIdxNew;
						m_interfaceIGM.SetStrategicSelection(plidx, nSelectedIdxNew);
					}
				}
				*/
			}

			///--- level targets - mission success accomplished ---
			bool bMissionFinished = false;
			int nStrIdxMissionFailed = -1; //means win if -1 or lose if >=0
			/*
			bool bMissionFinished = true;
			if ((m_arrStats[K_LVL_STATS_LEVEL_HAS_BOMBS] != 0) && (m_arrStats[K_LVL_STATS_BOMBS_DISARMED] == 0))
				bMissionFinished = false;
			if (m_arrStats[K_LVL_STATS_TARGETS_LEFT] > 0)
				bMissionFinished = false;

			///--- level failed if killed all hostages  ---
			//only fail because of hostages on hostage rescue missions
			if ((m_nLoadedLevelType == K_GAME_LSTYPE_HOSTAGE) &&
				(m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL] > 0) &&
				(m_arrStats[K_LVL_STATS_HOSTAGES_KILLED] >= m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]))
			{
				bMissionFinished = true;
				nStrIdxMissionFailed = STR_HOSTAGES_KILLED;
			}
			*/
			///--- LEVEL FAILED when not pressing continue ---
			bool bGaveUp = true;
			bool bPlayerMightContinue = false;
			for ( int plidx = 0; plidx < K_MAX_PLAYERS_CNT; plidx++ )
			{
				if ( pPlayerActor[plidx] != null )
				{
					//there is still a dead player that could continue
					if ( pPlayerActor[plidx]->fLife <= 0.0f )
						bPlayerMightContinue = true;
					if ( pPlayerActor[plidx]->fLife > 0.0f )
						bGaveUp = false;
					if ( m_arrPlayerSelHotJoin[plidx] != -1 )
						bGaveUp = false;
				}
			}
			if ( bGaveUp )
			{
				bMissionFinished = true;
				nStrIdxMissionFailed = STR_TEAM_KILLED;
			}

			//don't give verdict until all players are really dead
			if ( bPlayerMightContinue )
				bMissionFinished = false;
			//mission win? wait for scripts
			if ( (bMissionFinished) && (nStrIdxMissionFailed < 0) && (__Scripts().GetRunningScriptsCount() > 0) )
				bMissionFinished = false;

			//is mission finished?
			if ( bMissionFinished )
			{
				//make sure we stop all scripts (could generate enemies)
				__Scripts().StopAllScripts();
				//win or lose?
				if ( nStrIdxMissionFailed < 0 ) //win
					SetLevelState( K_LVL_STATE_MISSION_ACCOMPLISHED );
				else //lose - show why
					SetLevelState( K_LVL_STATE_MISSION_FAILED, nStrIdxMissionFailed );
			}

		}
		break;

		case K_LVL_STATE_MISSION_ACCOMPLISHED:
		{
			//wait for network data
			if ( UTApp().IsGameNetworked() )
			{
				g_netlock.Net_UpdateLevelResults( dTime );
				//show net votes
				CCtrlLayer* layer = __GUI().GetTopmostInputLayer();
				if ( layer )
				{
					CControl* ctrl;
					if ( (ctrl = layer->GetControlByName( "CTRL_NETVOTE_RESTART" )) != nullptr )
					{
						ctrl->paramsDict.SetVarINT32( L"leftVote", (g_netlock.m_arrLvlResPeerStates[0] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_RESTART) ? 1 : 0 );
						ctrl->paramsDict.SetVarINT32( L"rightVote", (g_netlock.m_arrLvlResPeerStates[1] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_RESTART) ? 1 : 0 );
					}
					if ( (ctrl = layer->GetControlByName( "CTRL_NETVOTE_CONTINUE" )) != nullptr )
					{
						ctrl->paramsDict.SetVarINT32( L"leftVote", (g_netlock.m_arrLvlResPeerStates[0] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CONTINUE) ? 1 : 0 );
						ctrl->paramsDict.SetVarINT32( L"rightVote", (g_netlock.m_arrLvlResPeerStates[1] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CONTINUE) ? 1 : 0 );
					}
				}

				///check presses
				//if someone clicked cancel throw us to main menu without error
				if ( g_netlock.Net_LevelResultsCountStates( CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CONTINUE ) >= K_MAX_PLAYERS_CNT )
				{
					LOG( L"Game::Level results: Players voted to continue!" );
					//see if we're hosting the game decide next level (advance)
					if ( g_netlock.Net_GetIAmHosting() )
					{
						//quick match
						if ( UTApp().m_Settings.devnet_eNetGameType == CApplicationSettings::K_NETGAME_TYPE_QUICK_MATCH )
						{
							//random level on quick match
							int nLevel = GetNextRandomLevel();
							//saving in userData is optional as it gets overwritten anyway from the player selection screen
							g_userData[K_MEMID_SELECTED_CHAPTER] = nLevel / K_GAME_LEVELS_PER_CHAPTER;
							g_userData[K_MEMID_SELECTED_LEVEL] = nLevel % K_GAME_LEVELS_PER_CHAPTER;
							//save in netlock too
							g_netlock.m_ucSelChapter = g_userData[K_MEMID_SELECTED_CHAPTER];
							g_netlock.m_ucSelLevel = g_userData[K_MEMID_SELECTED_LEVEL];

							LOG( L"Game::Level: Decided random chapter(%d) and level(%d).", g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL] );
						}
						else //hosting game
						{
							//on normal coop gets to the next mission but on hosted downloaded content it just plays again
							int nLevel = g_userData[K_MEMID_SELECTED_CHAPTER] * K_GAME_LEVELS_PER_CHAPTER + g_userData[K_MEMID_SELECTED_LEVEL];
							if ( g_netlock.m_ucModData == 0 )	//not playing custom
							{
								nLevel++;
								if ( nLevel >= UTGetChaptersList().GetTotalLevelsCnt() )
									nLevel = 0;
							}

							g_userData[K_MEMID_SELECTED_CHAPTER] = nLevel / K_GAME_LEVELS_PER_CHAPTER;
							g_userData[K_MEMID_SELECTED_LEVEL] = nLevel % K_GAME_LEVELS_PER_CHAPTER;
							//save in netlock too
							g_netlock.m_ucSelChapter = g_userData[K_MEMID_SELECTED_CHAPTER];
							g_netlock.m_ucSelLevel = g_userData[K_MEMID_SELECTED_LEVEL];

							LOG( L"Game::Level: Decided next chapter(%d) and level(%d).", g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL] );
						}
					}

					if ( !GameState::isTransitioning() )
					{
						CEvent* nevent = new CEvent( CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION );
						nevent->AddNamedArgUINT32( L"newGameState", GAME_STATE_PLAYER_SELECTION );
						nevent->AddNamedArgINT32( L"arg1", 0 ); //reset player selection
						nevent->AddNamedArgINT32( L"transitionType", TRANSITION_SIMPLE );
						__Events().QueueEvent( nevent );
					}
					//clear command
					g_netlock.Net_LevelResultsClearStates();
				}
				if ( g_netlock.Net_LevelResultsCountStates( CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_RESTART ) >= K_MAX_PLAYERS_CNT )
				{
					LOG( L"Game::Level Win: Players voted to restart the level!" );
					//set loading levels
					g_userData[K_MEMID_SELECTED_CHAPTER] = g_netlock.m_ucSelChapter;
					g_userData[K_MEMID_SELECTED_LEVEL] = g_netlock.m_ucSelLevel;

					if ( !GameState::isTransitioning() )
					{
						CEvent* nevent = new CEvent( CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION );
						nevent->AddNamedArgUINT32( L"newGameState", GAME_STATE_PLAYER_SELECTION );
						nevent->AddNamedArgINT32( L"arg1", 0 ); //reset player selection
						nevent->AddNamedArgINT32( L"transitionType", TRANSITION_SIMPLE );
						__Events().QueueEvent( nevent );
					}
					//clear command
					g_netlock.Net_LevelResultsClearStates();
				}
				//cancel button / command / window
				if ( g_netlock.Net_LevelResultsCountStates( CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CANCEL ) > 0 )
				{
					LOG( L"Game::Level Win: Player chose to exit!" );

					if ( !GameState::isTransitioning() )
					{
						//change game state
						CEvent* nevent = new CEvent( CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION );
						nevent->AddNamedArgUINT32( L"newGameState", GAME_STATE_MAINMENU );
						nevent->AddNamedArgINT32( L"transitionType", TRANSITION_SIMPLE );
						//check and see if other player requested exit and show message if so
						if ( g_netlock.m_arrLvlResPeerStates[g_netlock.Net_GetOtherPlayerIndex()] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CANCEL )
							nevent->AddNamedArgINT32( L"stateErrorStrIdx", STR_NETWORK_ERROR_PLAYER_LEFT );

						__Events().QueueEvent( nevent );
					}
					//clear command
					g_netlock.Net_LevelResultsClearStates();

					return;
				}
			}

			switch ( m_levelSubState )
			{
				case 0: //wait for message to disappear
				{
					m_levelStateTimer += dTime;
					if ( m_levelStateTimer > 2.0f )
					{
						m_levelStateTimer = 0.0f;
						m_levelSubState = 1;

						int nLevelIdx = -1;
						if ( m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE )
							nLevelIdx = g_userData[K_MEMID_SELECTED_LEVEL] + g_userData[K_MEMID_SELECTED_CHAPTER] * K_GAME_LEVELS_PER_CHAPTER;

						//pregatim strings pentru interfata de level finished
						WCHAR tmpstr[MAX_PATH];
						int nPlayers = m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED] + m_arrStats[K_LVL_STATS_PL2_HAS_PLAYED];
						//--- PL1 data ---
						float fAccuracyP1 = 1.0f;
						if ( m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT] > 0 )
							fAccuracyP1 = (float)m_arrStats[K_LVL_STATS_PL1_BULLETS_HIT] / (float)m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT];
						CLAMP( fAccuracyP1, 0.0f, 1.0f );
						__Texts().SetString( STR_MISSION_P1_KILLS, L"%d", m_arrStats[K_LVL_STATS_PL1_KILLS] );
						if ( m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT] > 0 )
							__Texts().SetString( STR_MISSION_P1_ACCURACY, L"%.1f%%", 100.0f * fAccuracyP1 );
						else
							__Texts().SetString( STR_MISSION_P1_ACCURACY, L"%s", __Texts().strings[STR_NOT_AVAILABLE]->sText );
						__Texts().SetString( STR_MISSION_P1_HOSTAGES, L"%d / %d", m_arrStats[K_LVL_STATS_PL1_HOSTAGES_SAVED], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL] );
						__Texts().SetString( STR_MISSION_P1_DEATHS, L"%d", m_arrStats[K_LVL_STATS_PL1_DEATHS] );
						//--- PL2 data ---
						float fAccuracyP2 = 1.0f;
						if ( m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT] > 0 )
							fAccuracyP2 = (float)m_arrStats[K_LVL_STATS_PL2_BULLETS_HIT] / (float)m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT];
						CLAMP( fAccuracyP2, 0.0f, 1.0f );
						__Texts().SetString( STR_MISSION_P2_KILLS, L"%d", m_arrStats[K_LVL_STATS_PL2_KILLS] );
						if ( m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT] > 0 )
							__Texts().SetString( STR_MISSION_P2_ACCURACY, L"%.1f%%", 100.0f * fAccuracyP2 );
						else
							__Texts().SetString( STR_MISSION_P2_ACCURACY, L"%s", __Texts().strings[STR_NOT_AVAILABLE]->sText );
						__Texts().SetString( STR_MISSION_P2_HOSTAGES, L"%d / %d", m_arrStats[K_LVL_STATS_PL2_HOSTAGES_SAVED], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL] );
						__Texts().SetString( STR_MISSION_P2_DEATHS, L"%d", m_arrStats[K_LVL_STATS_PL2_DEATHS] );

						//level time
						int nTimeSpent = m_arrStats[K_LVL_STATS_LEVEL_END_SEC] - m_arrStats[K_LVL_STATS_LEVEL_START_SEC];
						//--- calculam stele si XP ---
						int nStars = 3;
						if ( m_arrStats[K_LVL_STATS_HOSTAGES_KILLED] > 0 )
							nStars--;
						if ( (m_arrStats[K_LVL_STATS_PL1_DEATHS] + m_arrStats[K_LVL_STATS_PL2_DEATHS]) > 0 )
							nStars--;
						//on arrest warrant missions remove a star per target kill
						if ( (m_nLoadedLevelType == K_GAME_LSTYPE_ARREST_WARRANT) && (m_arrStats[K_LVL_STATS_LEVEL_ARREST_TARGETS_KILLED] > 0) )
						{
							nStars -= m_arrStats[K_LVL_STATS_LEVEL_ARREST_TARGETS_KILLED];
						}

						CLAMP( nStars, 1, 3 );

						//#ACHIEVEMENTS: 3 stars mission on any mission
						if ( nStars == 3 )
						{
							__Achievements().UnlockAchievement( ACH_3STARS_MISSION );
						}

						///--- SCORE ---
						int nTotalLevelScore = nStars * 1500;
						nTotalLevelScore += (int)ceil( (float)m_arrStats[K_LVL_STATS_PL1_KILLS] * fAccuracyP1 * 150.0f ) + m_arrStats[K_LVL_STATS_PL1_HOSTAGES_SAVED] * 300 - m_arrStats[K_LVL_STATS_PL1_DEATHS] * 200;
						nTotalLevelScore += (int)ceil( (float)m_arrStats[K_LVL_STATS_PL2_KILLS] * fAccuracyP2 * 150.0f ) + m_arrStats[K_LVL_STATS_PL2_HOSTAGES_SAVED] * 300 - m_arrStats[K_LVL_STATS_PL2_DEATHS] * 200;
						//add civilians score
						nTotalLevelScore += m_arrStats[K_LVL_STATS_CIVILIANS_ARRESTED] * 100;
						nTotalLevelScore -= m_arrStats[K_LVL_STATS_CIVILIANS_KILLED] * 80;
						//lower limit on total XP
						if ( nTotalLevelScore < 0 )
							nTotalLevelScore = 0;
						//add time bonus
						int timeBonus = (60/*sec*/ * 15/*min*/ - nTimeSpent) * 20;
						if ( timeBonus < 0 ) timeBonus = 0;
						//total XP points
						nTotalLevelScore += timeBonus;

						///--- XP Points ---
						int nXPpl1 = 0, nXPpl2 = 0;
						int nMaxXPPoints = App_GetMaxXP( K_GAME_MAX_UPGRADE_LEVELS );
						int nTotalXPPoints = Local_ComputeMissionXP( nStars );

						//--- STARS WINDOW ---
						OS_FormatTime( tmpstr, MAX_PATH, (float)(nTimeSpent) );
						__Texts().SetString( STR_MISSION_TIME, tmpstr );
						__Texts().SetString( STR_MISSION_CASUALTIES, L"%d", m_arrStats[K_LVL_STATS_PL1_DEATHS] + m_arrStats[K_LVL_STATS_PL2_DEATHS] );
						__Texts().SetString( STR_MISSION_SCORE, L"%d", nTotalLevelScore );

						int nHostagesSaved = m_arrStats[K_LVL_STATS_PL1_HOSTAGES_SAVED] + m_arrStats[K_LVL_STATS_PL2_HOSTAGES_SAVED];
						__Texts().SetString( STR_MISSION_HOSTAGES, L"%d / %d", nHostagesSaved, m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL] );

						//--- SAVE LEVEL DATA ---
						if ( m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE )
						{
							g_userData[K_MEMID_STARS_TOTAL] += LIMIT( nStars - g_levelStats[nLevelIdx].nStars, 0, 3 );

							g_levelStats[nLevelIdx].nPlayedTimes++;
							if ( g_levelStats[nLevelIdx].nStars < nStars )
								g_levelStats[nLevelIdx].nStars = nStars;
						}

						if ( nPlayers == 1 )
						{
							//only save best score on classic mode
							if ( m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE )
							{
								if ( g_levelStats[nLevelIdx].nScoreSolo < nTotalLevelScore )
									g_levelStats[nLevelIdx].nScoreSolo = nTotalLevelScore;
								if ( (g_levelStats[nLevelIdx].nBestTimeSec_Solo == 0) || (g_levelStats[nLevelIdx].nBestTimeSec_Solo < nTimeSpent) )
									g_levelStats[nLevelIdx].nBestTimeSec_Solo = nTimeSpent;
							}
							//XP points	save
							int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
							nXPpl1 = g_userData[nPlBaseIdx];
							inc_limit( g_userData[nPlBaseIdx], nTotalXPPoints, nMaxXPPoints );
						}
						else
						{
							//only save best score on classic mode
							if ( m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE )
							{
								if ( g_levelStats[nLevelIdx].nScoreCoop < nTotalLevelScore )
									g_levelStats[nLevelIdx].nScoreCoop = nTotalLevelScore;
								if ( (g_levelStats[nLevelIdx].nBestTimeSec_Coop == 0) || (g_levelStats[nLevelIdx].nBestTimeSec_Coop < nTimeSpent) )
									g_levelStats[nLevelIdx].nBestTimeSec_Coop = nTimeSpent;
							}

							//XP points	save
							if ( !UTApp().IsGameNetworked() )
							{
								//in local coop you only get half the XP for each player
								int nPl1BaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
								nXPpl1 = g_userData[nPl1BaseIdx]; //save old value
								inc_limit( g_userData[nPl1BaseIdx], nTotalXPPoints / 2, nMaxXPPoints );
								int nPl2BaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[1].eType;
								nXPpl2 = g_userData[nPl2BaseIdx]; //save old value
								inc_limit( g_userData[nPl2BaseIdx], nTotalXPPoints / 2, nMaxXPPoints );
							}
							else
							{
								//in network games each player gets it's own
								int nMyPlayerBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[g_netlock.Net_GetPlayerIndex()].eType;
								g_userData[nMyPlayerBaseIdx] += nTotalXPPoints;
								CLAMP( g_userData[nMyPlayerBaseIdx], 0, nMaxXPPoints );
							}
						}

						App_SaveUserData();

						//--- show windows and change portraits and title text ---
						__GUI().RemoveAllLayers();
						//generic changes
						CCtrlLayer* layer = null;
						if ( nPlayers == 1 )
							layer = __GUI().ShowLayerOnce( "LAYER_ID_LEVELWIN_1P" );
						else
						{
							if ( !UTApp().IsGameNetworked() )
								layer = __GUI().ShowLayerOnce( "LAYER_ID_LEVELWIN_2P" );
							else
								layer = __GUI().ShowLayerOnce( "LAYER_ID_LEVELWIN_2P_COOP" );
						}

						//report score to steam leaderboards
#ifdef ENABLE_LEADERBOARDS
						char pszBoardName[MAX_PATH];
						//only push scores to leaderboards if not playing a downloaded level and not using mods
						if ( m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE )
						{
#ifdef ENABLE_STEAM
							char strFormat[] = "%s%d.%d";
#endif
#ifdef ENABLE_GALAXY
							char strFormat[] = "%s%d_%d";
#endif

							if ( nPlayers == 1 )
							{
								StringCchPrintfA( pszBoardName, MAX_PATH, strFormat, K_GAME_STR_LEADERBOARDS_PREFIX_SP, m_nLoadedChapter + 1, m_nLoadedLevel + 1 );
							}
							else
							{
								StringCchPrintfA( pszBoardName, MAX_PATH, strFormat, K_GAME_STR_LEADERBOARDS_PREFIX_COOP, m_nLoadedChapter + 1, m_nLoadedLevel + 1 );
							}
							//reset old scores
							__Leaderboards().ResetScoresList();
							//reset strings too
							__Texts().SetString( STR_LEADERBOARDS_NAMES_VAL, L"..." );
							__Texts().SetString( STR_LEADERBOARDS_SCORES_VAL, L"..." );
							__Texts().SetString( STR_LEADERBOARDS_PLAYERSCORE_VAL, L"..." );
							//now upload score
							__Leaderboards().QueueJob( K_JOB_UPLOAD_SCORE, pszBoardName, nTotalLevelScore );
							//request downloading of scores
							__Leaderboards().QueueJob( K_JOB_GET_SCORES_AROUND_USER, pszBoardName );
							//request downloading of your own score - only if needed (when leaderboards don't update instantly)
							//__Leaderboards().QueueJob(K_JOB_GET_SCORE_FOR_CURRENT_USER, pszBoardName, 0);
						}
#endif

						if ( layer != null )
						{
							CControl* ctrltop = null;
							if ( (ctrltop = layer->GetControlByName( "CTRL_STARS" )) != nullptr )
							{
								ctrltop->paramsDict.SetVarINT32( L"nStars", nStars );
							}
							//red labels for conditions that aren't satisfied						   
							if ( m_arrStats[K_LVL_STATS_HOSTAGES_KILLED] > 0 )
							{
								if ( (ctrltop = layer->GetControlByName( "LABEL_HOSTAGES" )) != nullptr )
								{
									ctrltop->paramsDict.SetVarString( L"fontColor", L"0xffff0000" );
								}
							}
							if ( m_arrStats[K_LVL_STATS_PL1_DEATHS] + m_arrStats[K_LVL_STATS_PL2_DEATHS] > 0 )
							{
								if ( (ctrltop = layer->GetControlByName( "LABEL_CASUALTIES" )) != nullptr )
								{
									ctrltop->paramsDict.SetVarString( L"fontColor", L"0xffff0000" );
								}
							}

							//on custom downloaded levels hide the MELEE-leaderboards 
							if ( m_unLoadedLevelFlags & K_LVL_LEVEL_FLAG_DOWNLOADED )
							{
								if ( (ctrltop = layer->GetControlByName( "LABEL_LEADERBOARDS" )) != nullptr )
									ctrltop->paramsDict.SetVarString( L"fontColor", L"0x00000000" );
							}

							if ( nPlayers == 1 )
							{
								CControl* ctrl = null;
								if ( layer != null )
								{
									//portrete								
									if ( (ctrl = layer->GetControlByName( "CTRL_ANIM_PORTRAIT_PL1" )) != nullptr )
									{
										ctrl->paramsDict.SetVarINT32( L"setFrame", (int)g_playerSelScr.m_arrPlayers[0].eType );
									}
									//XP bar
									if ( (ctrl = layer->GetControlByName( "CTRL_XPBAR_PL1" )) != nullptr )
									{
										int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
										ctrl->paramsDict.SetVarINT32( L"nOldValue", nXPpl1 );
										ctrl->paramsDict.SetVarINT32( L"nNewValue", g_userData[nPlBaseIdx] );
									}
								}

								if ( m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE )
								{
									CHAR ctxt[MAX_PATH];
									StringCchPrintfA( ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1 );
									ANALYTICS_EVENT( "level_win_1p", ctxt, "durationSec", nTimeSpent );
								}
							}
							else //2 players
							{
								CControl* ctrl = null;
								if ( layer != null )
								{
									//portrete								
									if ( (ctrl = layer->GetControlByName( "CTRL_ANIM_PORTRAIT_PL1" )) != nullptr )
									{
										ctrl->paramsDict.SetVarINT32( L"setFrame", (int)g_playerSelScr.m_arrPlayers[0].eType );
									}
									if ( (ctrl = layer->GetControlByName( "CTRL_ANIM_PORTRAIT_PL2" )) != nullptr )
									{
										ctrl->paramsDict.SetVarINT32( L"setFrame", (int)g_playerSelScr.m_arrPlayers[1].eType );
									}
								}

								//network - replace player names with real ones
								if ( UTApp().IsGameNetworked() )
								{
									if ( (ctrl = layer->GetControlByName( "CTRL_WND_PL1" )) != nullptr )
									{
										ctrl->paramsDict.SetVarINT32( L"stringID", STR_NETWORK_HOST_NAME );
									}
									if ( (ctrl = layer->GetControlByName( "CTRL_WND_PL2" )) != nullptr )
									{
										ctrl->paramsDict.SetVarINT32( L"stringID", STR_NETWORK_PEER_NAME );
									}

									if ( m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE )
									{
										CHAR ctxt[MAX_PATH];
										StringCchPrintfA( ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1 );
										ANALYTICS_EVENT( "level_win_2p_net", ctxt, "durationSec", nTimeSpent );
									}
									//XP bar - networked
									if ( (ctrl = layer->GetControlByName( "CTRL_XPBAR_PL1" )) != nullptr )
									{
										ctrl->paramsDict.SetVarINT32( L"nOldValue", g_playerSelScr.m_arrPlayers[0].nPlayerXPPts );
										int nNew = LIMIT( g_playerSelScr.m_arrPlayers[0].nPlayerXPPts + nTotalXPPoints, 0, nMaxXPPoints );
										ctrl->paramsDict.SetVarINT32( L"nNewValue", nNew );
									}
									if ( (ctrl = layer->GetControlByName( "CTRL_XPBAR_PL2" )) != nullptr )
									{
										ctrl->paramsDict.SetVarINT32( L"nOldValue", g_playerSelScr.m_arrPlayers[1].nPlayerXPPts );
										int nNew = LIMIT( g_playerSelScr.m_arrPlayers[1].nPlayerXPPts + nTotalXPPoints, 0, nMaxXPPoints );
										ctrl->paramsDict.SetVarINT32( L"nNewValue", nNew );
									}
								}
								else
								{
									if ( m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE )
									{
										CHAR ctxt[MAX_PATH];
										StringCchPrintfA( ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1 );
										ANALYTICS_EVENT( "level_win_2p", ctxt, "durationSec", nTimeSpent );
									}
									//XP bar
									if ( (ctrl = layer->GetControlByName( "CTRL_XPBAR_PL1" )) != nullptr )
									{
										int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
										ctrl->paramsDict.SetVarINT32( L"nOldValue", nXPpl1 );
										ctrl->paramsDict.SetVarINT32( L"nNewValue", g_userData[nPlBaseIdx] );
									}
									if ( (ctrl = layer->GetControlByName( "CTRL_XPBAR_PL2" )) != nullptr )
									{
										int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[1].eType;
										ctrl->paramsDict.SetVarINT32( L"nOldValue", nXPpl2 );
										ctrl->paramsDict.SetVarINT32( L"nNewValue", g_userData[nPlBaseIdx] );
									}
								}
							}
						}

						// notify level finished for achievements
						if ( m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE )
							UTApp().App_OnLevelFinished( g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL] );
					}
				}
				break;
				default:
				{
#ifdef ENABLE_LEADERBOARDS
					//show leaderboard when pressing melee key (any controller)
					if ( (__Controllers().KeyPressed( K_CM_COMMAND_MELEE )) && (m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE) )
					{
						CCtrlLayer* lay = __GUI().GetLayerByName( "LAYER_ID_LEADERBOARDS_IGM" );
						if ( lay == null )
						{
							//show layer
							lay = __GUI().ShowLayerOnce( "LAYER_ID_LEADERBOARDS_IGM" );
							if ( lay )
							{
								CControl* ctrl = null;
								//change label that tells type of leaderboard that is shown
								if ( (ctrl = lay->GetControlByName( "LABEL_LBTYPE" )) != nullptr )
								{
									int nPlayers = m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED] + m_arrStats[K_LVL_STATS_PL2_HAS_PLAYED];
									if ( nPlayers == 1 )
										ctrl->paramsDict.SetVarINT32( L"stringID", STR_SINGLE_PLAYER );
									else
										ctrl->paramsDict.SetVarINT32( L"stringID", STR_COOP_ONLINE );
									//level name in STR_TEMP10
									int nChapter = g_userData[K_MEMID_SELECTED_CHAPTER];
									int nLevel = g_userData[K_MEMID_SELECTED_LEVEL];
									int nStrIdxLevelName = UTGetChaptersList().m_arrChapters[nChapter]->arrLevelNameStrIdx[nLevel];
									if ( nStrIdxLevelName >= 0 )
										__Texts().SetString( STR_TEMP10, L"%d.%d %s", nChapter + 1, nLevel + 1, __Texts().strings[nStrIdxLevelName]->sText );
									else
										__Texts().SetString( STR_TEMP10, L"%d.%d", nChapter + 1, nLevel + 1 );
								}
								//set player selection
								ctrl = lay->GetControlByName( "CTRL_SCORESLIST_TT" );
								if ( ctrl != null )
								{
									int nPlIdx = __Leaderboards().GetDownloadedScores_PlayerIndex();
									ctrl->paramsDict.SetVarINT32( L"nSelectedIdx", nPlIdx );
									ctrl->paramsDict.SetVarINT32( L"nOptionsCnt", __Leaderboards().GetDownloadedScoresCount() );
#ifndef ENABLE_LEADERBOARDS_NAMES_SELECTION
									ctrl->bCanHaveFocus = false;
									ctrl->paramsDict.SetVarBool( L"bUserCanSelect", false );
#endif
								}
							}
						}
					}
#endif
				}
				break;
			}
		}
		break;

		case K_LVL_STATE_MISSION_FAILED:
		{
			//wait for network data
			if ( UTApp().IsGameNetworked() )
			{
				g_netlock.Net_UpdateLevelResults( dTime );
				//show net votes
				CCtrlLayer* layer = __GUI().GetTopmostInputLayer();
				if ( layer )
				{
					CControl* ctrl;
					//vote restart level
					if ( (ctrl = layer->GetControlByName( "CTRL_NETVOTE_RESTART" )) != nullptr )
					{
						ctrl->paramsDict.SetVarINT32( L"leftVote", (g_netlock.m_arrLvlResPeerStates[0] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_RESTART) ? 1 : 0 );
						ctrl->paramsDict.SetVarINT32( L"rightVote", (g_netlock.m_arrLvlResPeerStates[1] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_RESTART) ? 1 : 0 );
					}
					//vote continue to next level
					if ( (ctrl = layer->GetControlByName( "CTRL_NETVOTE_CONTINUE" )) != nullptr )
					{
						ctrl->paramsDict.SetVarINT32( L"leftVote", (g_netlock.m_arrLvlResPeerStates[0] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CONTINUE) ? 1 : 0 );
						ctrl->paramsDict.SetVarINT32( L"rightVote", (g_netlock.m_arrLvlResPeerStates[1] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CONTINUE) ? 1 : 0 );
					}
				}
				///check presses
				//if someone clicked cancel throw us to main menu without error
				if ( g_netlock.Net_LevelResultsCountStates( CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CONTINUE ) >= K_MAX_PLAYERS_CNT )
				{
					LOG( L"Game::Level results: Players voted to continue!" );
					//see if we're hosting the game decide next level (advance)
					if ( g_netlock.Net_GetIAmHosting() )
					{
						//quick match
						if ( UTApp().m_Settings.devnet_eNetGameType == CApplicationSettings::K_NETGAME_TYPE_QUICK_MATCH )
						{
							//random level on quick match
							int nLevel = GetNextRandomLevel();
							//saving in userData is optional as it gets overwritten anyway from the player selection screen
							g_userData[K_MEMID_SELECTED_CHAPTER] = nLevel / K_GAME_LEVELS_PER_CHAPTER;
							g_userData[K_MEMID_SELECTED_LEVEL] = nLevel % K_GAME_LEVELS_PER_CHAPTER;
							//save in netlock too
							g_netlock.m_ucSelChapter = g_userData[K_MEMID_SELECTED_CHAPTER];
							g_netlock.m_ucSelLevel = g_userData[K_MEMID_SELECTED_LEVEL];

							LOG( L"Game::Level: Decided random chapter(%d) and level(%d).", g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL] );
						}
						else //hosting game
						{
							int nLevel = g_userData[K_MEMID_SELECTED_CHAPTER] * K_GAME_LEVELS_PER_CHAPTER + g_userData[K_MEMID_SELECTED_LEVEL];
							nLevel++;
							if ( nLevel >= UTGetChaptersList().GetTotalLevelsCnt() )
								nLevel = 0;

							g_userData[K_MEMID_SELECTED_CHAPTER] = nLevel / K_GAME_LEVELS_PER_CHAPTER;
							g_userData[K_MEMID_SELECTED_LEVEL] = nLevel % K_GAME_LEVELS_PER_CHAPTER;
							//save in netlock too
							g_netlock.m_ucSelChapter = g_userData[K_MEMID_SELECTED_CHAPTER];
							g_netlock.m_ucSelLevel = g_userData[K_MEMID_SELECTED_LEVEL];

							LOG( L"Game::Level: Decided next chapter(%d) and level(%d).", g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL] );
						}
					}

					if ( !GameState::isTransitioning() )
					{
						CEvent* nevent = new CEvent( CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION );
						nevent->AddNamedArgUINT32( L"newGameState", GAME_STATE_PLAYER_SELECTION );
						nevent->AddNamedArgINT32( L"arg1", 0 ); //reset player selection
						nevent->AddNamedArgINT32( L"transitionType", TRANSITION_SIMPLE );
						__Events().QueueEvent( nevent );
					}
					//clear command
					g_netlock.Net_LevelResultsClearStates();
				}
				if ( g_netlock.Net_LevelResultsCountStates( CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_RESTART ) >= K_MAX_PLAYERS_CNT )
				{
					LOG( L"Game::Level results: Players voted to restart the level!" );
					//set loading levels
					g_userData[K_MEMID_SELECTED_CHAPTER] = g_netlock.m_ucSelChapter;
					g_userData[K_MEMID_SELECTED_LEVEL] = g_netlock.m_ucSelLevel;

					if ( !GameState::isTransitioning() )
					{
						CEvent* nevent = new CEvent( CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION );
						nevent->AddNamedArgUINT32( L"newGameState", GAME_STATE_PLAYER_SELECTION );
						nevent->AddNamedArgINT32( L"arg1", 0 ); //reset player selection
						nevent->AddNamedArgINT32( L"transitionType", TRANSITION_SIMPLE );
						__Events().QueueEvent( nevent );
					}
					//clear command
					g_netlock.Net_LevelResultsClearStates();
				}

				//if someone clicked cancel throw us to main menu without error
				if ( g_netlock.Net_LevelResultsCountStates( CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CANCEL ) > 0 )
				{
					LOG( L"Game::Level results: Peer left the game! Quit lobby!" );

					if ( !GameState::isTransitioning() )
					{
						//change game state
						CEvent* nevent = new CEvent( CEventTypes::evtT_GAMESTATE, CEventCommands::evtC_GAMESTATE_CHANGE_TRANSITION );
						nevent->AddNamedArgUINT32( L"newGameState", GAME_STATE_MAINMENU );
						nevent->AddNamedArgINT32( L"transitionType", TRANSITION_SIMPLE );
						//check and see if other player requested exit and show message if so
						if ( g_netlock.m_arrLvlResPeerStates[g_netlock.Net_GetOtherPlayerIndex()] == CNetLock::sPacketLevelResults::K_LEVRES_STATE_CLICKED_CANCEL )
							nevent->AddNamedArgINT32( L"stateErrorStrIdx", STR_NETWORK_ERROR_PLAYER_LEFT );

						__Events().QueueEvent( nevent );
					}
					//clear command
					g_netlock.Net_LevelResultsClearStates();

					return;
				}
			}

			switch ( m_levelSubState )
			{
				case 0: //wait for the level failed message to go away
				{
					m_levelStateTimer += dTime;
					if ( m_levelStateTimer > 2.0f )
					{
						m_levelStateTimer = 0.0f;
						m_levelSubState = 1;

						//pregatim strings pentru interfata de level finished
						WCHAR tmpstr[MAX_PATH];
						int nPlayers = m_arrStats[K_LVL_STATS_PL1_HAS_PLAYED] + m_arrStats[K_LVL_STATS_PL2_HAS_PLAYED];
						//--- PL1 data ---
						float fAccuracyP1 = 1.0f;
						if ( m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT] > 0 )
							fAccuracyP1 = (float)m_arrStats[K_LVL_STATS_PL1_BULLETS_HIT] / (float)m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT];
						CLAMP( fAccuracyP1, 0.0f, 1.0f );
						__Texts().SetString( STR_MISSION_P1_KILLS, L"%d", m_arrStats[K_LVL_STATS_PL1_KILLS] );
						if ( m_arrStats[K_LVL_STATS_PL1_BULLETS_SHOT] > 0 )
							__Texts().SetString( STR_MISSION_P1_ACCURACY, L"%.1f%%", 100.0f * fAccuracyP1 );
						else
							__Texts().SetString( STR_MISSION_P1_ACCURACY, L"%s", __Texts().strings[STR_NOT_AVAILABLE]->sText );
						__Texts().SetString( STR_MISSION_P1_HOSTAGES, L"%d / %d", m_arrStats[K_LVL_STATS_PL1_HOSTAGES_SAVED], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL] );
						__Texts().SetString( STR_MISSION_P1_DEATHS, L"%d", m_arrStats[K_LVL_STATS_PL1_DEATHS] );
						//--- PL2 data ---
						float fAccuracyP2 = 1.0f;
						if ( m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT] > 0 )
							fAccuracyP2 = (float)m_arrStats[K_LVL_STATS_PL2_BULLETS_HIT] / (float)m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT];
						CLAMP( fAccuracyP2, 0.0f, 1.0f );
						__Texts().SetString( STR_MISSION_P2_KILLS, L"%d", m_arrStats[K_LVL_STATS_PL2_KILLS] );
						if ( m_arrStats[K_LVL_STATS_PL2_BULLETS_SHOT] > 0 )
							__Texts().SetString( STR_MISSION_P2_ACCURACY, L"%.1f%%", 100.0f * fAccuracyP2 );
						else
							__Texts().SetString( STR_MISSION_P2_ACCURACY, L"%s", __Texts().strings[STR_NOT_AVAILABLE]->sText );
						__Texts().SetString( STR_MISSION_P2_HOSTAGES, L"%d / %d", m_arrStats[K_LVL_STATS_PL2_HOSTAGES_SAVED], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL] );
						__Texts().SetString( STR_MISSION_P2_DEATHS, L"%d", m_arrStats[K_LVL_STATS_PL2_DEATHS] );

						///--- XP Points ---
						int nXPpl1 = 0, nXPpl2 = 0;
						int nMaxXPPoints = App_GetMaxXP( K_GAME_MAX_UPGRADE_LEVELS );
						int nTotalXPPoints = Local_ComputeMissionXP( 0 );

						//--- STARS WINDOW ---
						int nTimeSpent = m_arrStats[K_LVL_STATS_LEVEL_END_SEC] - m_arrStats[K_LVL_STATS_LEVEL_START_SEC];
						OS_FormatTime( tmpstr, MAX_PATH, (float)(nTimeSpent) );
						__Texts().SetString( STR_MISSION_TIME, tmpstr );

						//--- SAVE LEVEL DATA ---
						// not playing downloaded levels so save played times counter
						if ( m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE )
						{
							int nLevelIdx = g_userData[K_MEMID_SELECTED_LEVEL] + g_userData[K_MEMID_SELECTED_CHAPTER] * K_GAME_LEVELS_PER_CHAPTER;
							g_levelStats[nLevelIdx].nPlayedTimes++;
						}

						if ( nPlayers == 1 )
						{
							//XP points	save
							int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
							nXPpl1 = g_userData[nPlBaseIdx];
							inc_limit( g_userData[nPlBaseIdx], nTotalXPPoints, nMaxXPPoints );
						}
						else
						{
							//XP points	save
							if ( !UTApp().IsGameNetworked() )
							{
								//in local coop you only get half the XP for each player
								int nPl1BaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
								nXPpl1 = g_userData[nPl1BaseIdx]; //save old value
								inc_limit( g_userData[nPl1BaseIdx], nTotalXPPoints / 2, nMaxXPPoints );
								int nPl2BaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[1].eType;
								nXPpl2 = g_userData[nPl2BaseIdx]; //save old value
								inc_limit( g_userData[nPl2BaseIdx], nTotalXPPoints / 2, nMaxXPPoints );
							}
							else
							{
								//in network games each player gets it's own
								int nMyPlayerBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[g_netlock.Net_GetPlayerIndex()].eType;
								g_userData[nMyPlayerBaseIdx] += nTotalXPPoints;
								CLAMP( g_userData[nMyPlayerBaseIdx], 0, nMaxXPPoints );
							}
						}

						App_SaveUserData();


						//--- show windows and change portraits and title text ---
						if ( nPlayers == 1 )
						{
							__GUI().RemoveAllLayers();
							CCtrlLayer* layer = __GUI().ShowLayerOnce( "LAYER_ID_LEVELFAIL_1P" );
							if ( layer != null )
							{
								CControl* ctrl = layer->GetControlByName( "CTRL_STARS" );
								if ( ctrl )
								{
									ctrl->paramsDict.SetVarINT32( L"nStars", 0 );
								}
								//reason why
								if ( m_levelStateParam > 0 ) //if set
								{
									ctrl = layer->GetControlByName( "BLINKER_REASON" );
									if ( ctrl )
									{
										ctrl->paramsDict.SetVarINT32( L"stringID", m_levelStateParam );
									}
								}
								//portrete								
								ctrl = layer->GetControlByName( "CTRL_ANIM_PORTRAIT_PL1" );
								if ( ctrl )
								{
									ctrl->paramsDict.SetVarINT32( L"setFrame", (int)g_playerSelScr.m_arrPlayers[0].eType );
								}
								//XP bar
								if ( (ctrl = layer->GetControlByName( "CTRL_XPBAR_PL1" )) != nullptr )
								{
									int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
									ctrl->paramsDict.SetVarINT32( L"nOldValue", nXPpl1 );
									ctrl->paramsDict.SetVarINT32( L"nNewValue", g_userData[nPlBaseIdx] );
								}
							}

							if ( m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE )
							{
								CHAR ctxt[MAX_PATH];
								StringCchPrintfA( ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1 );
								ANALYTICS_EVENT( "level_lose_1p", ctxt, "durationSec", nTimeSpent );
							}
						}
						else //2 players
						{
							__GUI().RemoveAllLayers();

							CCtrlLayer* layer = null;
							if ( !UTApp().IsGameNetworked() )
								layer = __GUI().ShowLayerOnce( "LAYER_ID_LEVELFAIL_2P" );
							else
								layer = __GUI().ShowLayerOnce( "LAYER_ID_LEVELFAIL_2P_COOP" );

							if ( layer != null )
							{
								CControl* ctrl = null;

								if ( (ctrl = layer->GetControlByName( "CTRL_STARS" )) != nullptr )
								{
									ctrl->paramsDict.SetVarINT32( L"nStars", 0 );
								}
								//reason why
								if ( m_levelStateParam > 0 ) //if set
								{
									if ( (ctrl = layer->GetControlByName( "BLINKER_REASON" )) != nullptr )
									{
										ctrl->paramsDict.SetVarINT32( L"stringID", m_levelStateParam );
									}
								}
								//portrete								
								if ( (ctrl = layer->GetControlByName( "CTRL_ANIM_PORTRAIT_PL1" )) != nullptr )
								{
									ctrl->paramsDict.SetVarINT32( L"setFrame", (int)g_playerSelScr.m_arrPlayers[0].eType );
								}
								if ( (ctrl = layer->GetControlByName( "CTRL_ANIM_PORTRAIT_PL2" )) != nullptr )
								{
									ctrl->paramsDict.SetVarINT32( L"setFrame", (int)g_playerSelScr.m_arrPlayers[1].eType );
								}

								//network - replace player names with real ones
								if ( UTApp().IsGameNetworked() )
								{
									if ( (ctrl = layer->GetControlByName( "CTRL_WND_PL1" )) != nullptr )
									{
										ctrl->paramsDict.SetVarINT32( L"stringID", STR_NETWORK_HOST_NAME );
									}
									if ( (ctrl = layer->GetControlByName( "CTRL_WND_PL2" )) != nullptr )
									{
										ctrl->paramsDict.SetVarINT32( L"stringID", STR_NETWORK_PEER_NAME );
									}

									if ( m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE )
									{
										CHAR ctxt[MAX_PATH];
										StringCchPrintfA( ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1 );
										ANALYTICS_EVENT( "level_lose_2p_net", ctxt, "durationSec", nTimeSpent );
									}
									//XP bar - networked
									if ( (ctrl = layer->GetControlByName( "CTRL_XPBAR_PL1" )) != nullptr )
									{
										ctrl->paramsDict.SetVarINT32( L"nOldValue", g_playerSelScr.m_arrPlayers[0].nPlayerXPPts );
										int nNew = LIMIT( g_playerSelScr.m_arrPlayers[0].nPlayerXPPts + nTotalXPPoints, 0, nMaxXPPoints );
										ctrl->paramsDict.SetVarINT32( L"nNewValue", nNew );
									}
									if ( (ctrl = layer->GetControlByName( "CTRL_XPBAR_PL2" )) != nullptr )
									{
										ctrl->paramsDict.SetVarINT32( L"nOldValue", g_playerSelScr.m_arrPlayers[1].nPlayerXPPts );
										int nNew = LIMIT( g_playerSelScr.m_arrPlayers[1].nPlayerXPPts + nTotalXPPoints, 0, nMaxXPPoints );
										ctrl->paramsDict.SetVarINT32( L"nNewValue", nNew );
									}
								}
								else
								{
									if ( m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE )
									{
										CHAR ctxt[MAX_PATH];
										StringCchPrintfA( ctxt, MAX_PATH, "mission_%d_%d", g_userData[K_MEMID_SELECTED_CHAPTER] + 1, g_userData[K_MEMID_SELECTED_LEVEL] + 1 );
										ANALYTICS_EVENT( "level_lose_2p", ctxt, "durationSec", nTimeSpent );
									}

									//XP bar
									if ( (ctrl = layer->GetControlByName( "CTRL_XPBAR_PL1" )) != nullptr )
									{
										int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[0].eType;
										ctrl->paramsDict.SetVarINT32( L"nOldValue", nXPpl1 );
										ctrl->paramsDict.SetVarINT32( L"nNewValue", g_userData[nPlBaseIdx] );
									}
									if ( (ctrl = layer->GetControlByName( "CTRL_XPBAR_PL2" )) != nullptr )
									{
										int nPlBaseIdx = K_MEMID_TOTALXP_PER_CLASS_START + (int)g_playerSelScr.m_arrPlayers[1].eType;
										ctrl->paramsDict.SetVarINT32( L"nOldValue", nXPpl2 );
										ctrl->paramsDict.SetVarINT32( L"nNewValue", g_userData[nPlBaseIdx] );
									}
								}

							}
						}
						// notify level finished for achievements
						if ( m_unLoadedLevelFlags == K_LVL_LEVEL_FLAG_NONE )
							UTApp().App_OnLevelFinished( g_userData[K_MEMID_SELECTED_CHAPTER], g_userData[K_MEMID_SELECTED_LEVEL] );
					}
				}
				break;
				default:
					break;
			}
		}
		break;

	}

	// updates dirty rects (tileset and more)
	UpdateDirtyRects();

	//clear poly buffers first
	m_bufferedPainter.ClearBuffers();

	///--- PHYSICS POINTS ---
	//UpdatePhysicsPoints( dTime );
	///--- BULLETS (after phys pts) ---
	UpdateBullets( dTime );
	///--- PROPS ---
	UpdateDoofers( dTime );
	///--- DECALS ---
	UpdateDecals( dTime );
	///--- ACTIVES ---
	UpdateAI( dTime, g_editor.IsLaunched() );

	///--- STATISTICS ---
	//active players
	m_nPlayersActive = m_nPlayers;
	for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
	{
		if ( pPlayerActor[kk] != nullptr )
		{
			EAIBehaviorType curbeh = pPlayerActor[kk]->GetCurrentBehavior();
			if ( curbeh == AI_BEHAVIOR_IN_LIMBO )
				m_nPlayersActive--;
		}
	}


	//set update done flag
	m_bOneUpdateDone = true;
}


void CLevel::Update( float dTime )
{
	///--- update camera ---
	//default camera position following the players
	Vec2 avg_live( 0.0f, 0.0f ), avg_all( 0.0f, 0.0f );
	int plcnt_live = 0, plcnt_all = 0;
	for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
	{
		//on networked games ignore the peer and stay locked onto the player
		if ( UTApp().IsGameNetworked() )
		{
			int nIndexToFollow = g_netlock.Net_GetPlayerIndex();
			// move camera on peer after you die
			if ( pPlayerActor[g_netlock.Net_GetPlayerIndex()] == nullptr )
			{
				nIndexToFollow = g_netlock.Net_GetOtherPlayerIndex();
				//if other player is dead too, just skip them and look at last spawn pos
				if ( pPlayerActor[nIndexToFollow] == nullptr )
					continue;
			}
			//in networked games just ignore the other player
			if ( (UTApp().IsGameNetworked()) && (kk != nIndexToFollow) )
				continue;
		}

		if ( (pPlayerActor[kk] != nullptr) && (pPlayerActor[kk]->GetCurrentBehavior() != AI_BEHAVIOR_IN_LIMBO) && (pPlayerActor[kk]->nSuspendedFlags == K_LVL_SUSPENDFLAG_NONE) )
		{
			//#TODO: add constants or special camera class for this wicked camera movement
			const float fMaxCameraMovement = K_TILE_SIZE_F * 4.0f;
			const float fMaxAimVecRadius = K_TILE_SIZE_F * 8.0f;
			const float fCameraDeadRadius = K_TILE_SIZE_F * 1.0f;
			// find look direction normalized
			float fLookDist = MUVec2Len( &pPlayerActor[kk]->GetAimVec() );
			Vec2 fLookOff = pPlayerActor[kk]->GetAimVec() / fLookDist;
			// normalize distance and square it so if varies less when cursor is close to character
			fLookDist -= fCameraDeadRadius;
			fLookDist /= fMaxAimVecRadius;
			CLAMP( fLookDist, 0.0f, 1.0f );
			//fLookDist *= fLookDist;
			// compute final camera vector
			fLookOff *= fLookDist * fMaxCameraMovement;

			if ( pPlayerActor[kk]->GetCurrentBehavior() != AI_BEHAVIOR_DEAD )
			{
				avg_live += Vec3XY( pPlayerActor[kk]->pos_last ) + fLookOff;
				plcnt_live++;
			}

			avg_all += Vec3XY( pPlayerActor[kk]->pos_last ) + fLookOff;
			plcnt_all++;
		}
	}

	//average player positions
	bool bAvgSet = false;
	Vec2 vPlayersAvg( 0.0f, 0.0f );
	if ( plcnt_all > 0 )
	{
		bAvgSet = true;

		avg_all /= plcnt_all;
		vPlayersAvg = avg_all;

		if ( plcnt_live > 0 )
		{
			avg_live /= plcnt_live;
			//are they too far apart? 
			if ( MUVec2Len( &(avg_all - avg_live) ) > UTApp().g_rectRT.h * 0.5f )
			{
				vPlayersAvg = avg_live;
			}
		}
	}

	//handles render size changes
	m_camLevelToRT.SetViewport( UTApp().g_rectRT );
	m_camLevelToScr.SetViewport( UTApp().g_rectRender );
	if ( g_editor.IsLaunched() )
	{
		m_camLevelToRT.SetCamPos( &g_editor.m_vCamPos );
	}
	else
	{
		//no target camera object? look at the player pos average
		if ( m_camTargetActive == null )
		{
			if ( bAvgSet )
				m_vCamPosDefault = vPlayersAvg;

			m_camLevelToRT.SetCamPos( &m_vCamPosDefault );
		}
		else
		{
			m_camLevelToRT.SetCamPos( &(m_camTargetActive->pos.xy_proj) );
		}
	}

	m_camLevelToRT.Update( dTime );
	Vec3 vCamPos = m_camLevelToRT.GetCamPos();
	m_camLevelToScr.SetCamPos( &Vec2( vCamPos.x, vCamPos.y ), vCamPos.z );
	m_camLevelToScr.Update( dTime );

	//find visible area
	RectXYWH camrect = m_camLevelToRT.GetCamWorldAABB();
	CAABB camAABB( Vec2( camrect.x, camrect.y ), Vec2( camrect.Right(), camrect.Bottom() ) );

	//set sounds listener position
	SND_SET_LISTENER_POS( camrect.Center() );
	//--- update particles and emitters ---
	__Particles().UpdatePartEmitters( dTime, camrect );
	__Particles().Update( dTime );
	//__Particles().UpdateStringDummies( dTime );

	Areas_UpdateVisibility( camrect );
	///--- update visibility lists (after update) ---
	BuildVisibilityLists();

	// builds all dynamic meshes necessary for drawing the next frame
	BuildDynamicGeometry( camAABB );

	///--- update interface ---
	m_interfaceIGM.Update( dTime );
	//m_interfaceTextBubble.Update(dTime);
}

OPRESULT CLevel::PaintDeferredBuffers( float fBetweenFramesPercent )
{
	CRTManager::CEngineRenderTarget* pRT = nullptr;
	///----------------------------------------------------
	/// 1. NORMAL MAP AND HEIGHT MAP
	///----------------------------------------------------
	pRT = __RTManager().GetRTbyUID( K_RTID_TEMP1 );
	if ( pRT != nullptr )
	{
		if ( OP_SUCCESS( __RTManager().BeginSceneRT( pRT ) ) )
		{
			// Clear the render target and the zbuffer 
			if ( FAILED( m_pDevice->Clear( 0, nullptr, D3DCLEAR_TARGET, K_GAME_CLEAR_COLOR, 1.0f, 0 ) ) )
			{
				return K_OP_FAILED;
			}
			//Mat matProj;
			//D3DXMatrixOrthoOffCenterLH(&matProj, 0.5f, pRT->nWidth + 0.5f, pRT->nHeight + 0.5f, 0.5f, 0.0f, 1.0f);
			m_pDevice->SetTransform( D3DTS_PROJECTION, &pRT->matProj );

			m_pDevice->SetTransform( D3DTS_WORLD, &g_matIdentity );
			m_pDevice->SetTransform( D3DTS_VIEW, &g_matIdentity );

			RenderPass( K_LVL_RP_NORMALS_HEIGHT, &pRT->matProj, fBetweenFramesPercent );

			V_OP_RET( __RTManager().EndSceneRT( pRT ) );
		}
	}

	///----------------------------------------------------
	/// 2. LIGHT MAP
	///----------------------------------------------------
	pRT = __RTManager().GetRTbyUID( K_RTID_COLORDEPTHSTENCIL );
	if ( pRT != nullptr )
	{
		if ( OP_SUCCESS( __RTManager().BeginSceneRT( pRT ) ) )
		{
			// Clear the render target and the zbuffer 
			if ( FAILED( m_pDevice->Clear( 0, nullptr, D3DCLEAR_TARGET, 0xff000000, 1.0f, 0 ) ) )
			{
				return K_OP_FAILED;
			}
			//Mat matProj;
			//D3DXMatrixOrthoOffCenterLH(&matProj, 0.5f, pRT->nWidth + 0.5f, pRT->nHeight + 0.5f, 0.5f, 0.0f, 1.0f);
			m_pDevice->SetTransform( D3DTS_PROJECTION, &pRT->matProj );

			m_pDevice->SetTransform( D3DTS_WORLD, &g_matIdentity );
			m_pDevice->SetTransform( D3DTS_VIEW, &g_matIdentity );

			// special method for rendering lights pass
			// uses the height/normals render target
			RenderPass_Lights( &pRT->matProj, fBetweenFramesPercent );

			V_OP_RET( __RTManager().EndSceneRT( pRT ) );

		}
	}

	///----------------------------------------------------
	/// 3. COLOR MAP - overwrites the normal map as we don't need it anymore
	///----------------------------------------------------
	pRT = __RTManager().GetRTbyUID( K_RTID_TEMP1 );
	if ( pRT != nullptr )
	{
		if ( OP_SUCCESS( __RTManager().BeginSceneRT( pRT ) ) )
		{
			// Clear the render target and the zbuffer 
			if ( FAILED( m_pDevice->Clear( 0, nullptr, D3DCLEAR_TARGET, K_GAME_CLEAR_COLOR, 1.0f, 0 ) ) )
			{
				return K_OP_FAILED;
			}
			//#TODO: este corect ?? offset the projection matrix by 0.5f because in DX the pixel's 0.0 is the center of the pixel
			//Mat matProj;
			//D3DXMatrixOrthoOffCenterLH(&matProj, 0.5f, pRT->nWidth + 0.5f, pRT->nHeight + 0.5f, 0.5f, 0.0f, 1.0f);
			m_pDevice->SetTransform( D3DTS_PROJECTION, &pRT->matProj );
			m_pDevice->SetTransform( D3DTS_WORLD, &g_matIdentity );
			m_pDevice->SetTransform( D3DTS_VIEW, &g_matIdentity );

			RenderPass( K_LVL_RP_COLORS, &pRT->matProj, fBetweenFramesPercent );

			V_OP_RET( __RTManager().EndSceneRT( pRT ) );
		}
	}


	///----------------------------------------------------
	/// START GI
	/// 1. build occluders/emitters map
	/// 2. apply voronoi seed PS on 1
	/// 3. apply multipass voronoi on (starting with) 2
	/// 4. convert voronoi from 3 to SDF
	/// 5. raymarch
	///----------------------------------------------------


	///----------------------------------------------------
	/// 1. build occluders/emitters map (emissive map)
	///----------------------------------------------------
	// render on transparent background, colored lights, black walls
	pRT = __RTManager().GetRTbyUID( K_RTID_WORLDSCENE );
	if ( pRT != nullptr )
	{
		if ( OP_SUCCESS( __RTManager().BeginSceneRT( pRT ) ) )
		{
			Matrix matView;
			// Clear the render target and the zbuffer 
			if ( FAILED( m_pDevice->Clear( 0, nullptr, D3DCLEAR_TARGET, 0x00000000, 1.0f, 0 ) ) )
			{
				return K_OP_FAILED;
			}

			RectXYWH		camrect = m_camLevelToRT.GetCamWorldAABB();
			CAABB			camAABB( camrect );

			RenderPass_GIEmissive( &pRT->matProj, fBetweenFramesPercent );
			V_OP_RET( __RTManager().EndSceneRT( pRT ) );
		}
	}

	///----------------------------------------------------
	/// 2. apply voronoi seed PS on 1
	///----------------------------------------------------
	pRT = __RTManager().GetRTbyUID( K_RTID_JUMPFLOOD );
	if ( pRT != nullptr )
	{
		if ( OP_SUCCESS( __RTManager().BeginSceneRT( pRT ) ) )
		{
			if ( FAILED( m_pDevice->Clear( 0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB( 0, 0, 0, 0 ), 1.0f, 0 ) ) )
				return K_OP_FAILED;

			m_pDevice->SetTransform( D3DTS_PROJECTION, &pRT->matProj );

			m_pDevice->SetTransform( D3DTS_WORLD, &g_matIdentity );
			m_pDevice->SetTransform( D3DTS_VIEW, &g_matIdentity );

			// RT sized quad with tex1 color, tex2 lightmap

			Matrix matView;
			m_pDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );

			m_pDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
			m_pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
			m_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

			m_pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
			m_pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
			m_pDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );

			MUMatIdentity( &matView );
			m_pDevice->SetTransform( D3DTS_VIEW, &matView );
			m_pDevice->SetTransform( D3DTS_WORLD, &g_matIdentity );

			///--- compose scene from normals and color ---
			Matrix matWVP = matView * pRT->matProj;

			CRTManager::CEngineRenderTarget* pRTcolor = __RTManager().GetRTbyUID( K_RTID_WORLDSCENE );
			_ASSERT( pRTcolor != nullptr );
			m_pDevice->SetTexture( 0, pRTcolor->m_pRTTexture );

			//--- build RT rect ---
			_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
			vul.pos = Vec3( 0.0f, 0.0f, 0.0f );
			vur.pos = Vec3( (float)pRTcolor->nWidth, 0.0f, 0.0f );
			vdl.pos = Vec3( 0.0f, (float)pRTcolor->nHeight, 0.0f );
			vdr.pos = Vec3( (float)pRTcolor->nWidth, (float)pRTcolor->nHeight, 0.0f );

			vul.tex1 = vul.tex2 = Vec4( 0.0f, 0.0f, 0.0f, 0.0f );
			vur.tex1 = vur.tex2 = Vec4( 1.0f, 0.0f, 0.0f, 0.0f );
			vdl.tex1 = vdl.tex2 = Vec4( 0.0f, 1.0f, 0.0f, 0.0f );
			vdr.tex1 = vdr.tex2 = Vec4( 1.0f, 1.0f, 0.0f, 0.0f );
			//set color
			vul.color = vur.color = vdl.color = vdr.color = 0xffffffff;
			//build verts
			_VERTEX_PNCT4T4 lightRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
			lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
			lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;

			__Shaders().SetVSByName( L"VS_COMPOSITION" );
			__Shaders().SetVertexDeclaration( K_SHM_PNCT4T4 );
			__Shaders().SetVSConstantF( 0, (float*)&matWVP, 4 );

			__Shaders().SetPSByName( L"PS_VORONOI_SEED" );

			m_pDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 2, &lightRectV, sizeof( _VERTEX_PNCT4T4 ) );
			// remove VS PS
			__Shaders().SetPS( nullptr );
			__Shaders().SetVS( nullptr );

			V_OP_RET( __RTManager().EndSceneRT( pRT ) );

		}
	}

	///----------------------------------------------------
	/// 3. apply multipass voronoi on (starting with) 2
	///----------------------------------------------------
	pRT = __RTManager().GetRTbyUID( K_RTID_TEMPORARY );
	int passes = ceil( log( max( pRT->nWidth, pRT->nHeight ) ) / log( 2.0 ) );
	Matrix matView;
	MUMatIdentity( &matView );
	m_pDevice->SetTransform( D3DTS_VIEW, &matView );
	m_pDevice->SetTransform( D3DTS_WORLD, &g_matIdentity );

	///--- compose scene from normals and color ---
	Matrix matWVP = matView * pRT->matProj;

	//--- build RT rect ---
	_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
	vul.pos = Vec3( 0.0f, 0.0f, 0.0f );
	vur.pos = Vec3( (float)pRT->nWidth, 0.0f, 0.0f );
	vdl.pos = Vec3( 0.0f, (float)pRT->nHeight, 0.0f );
	vdr.pos = Vec3( (float)pRT->nWidth, (float)pRT->nHeight, 0.0f );

	vul.tex1 = vul.tex2 = Vec4( 0.0f, 0.0f, 0.0f, 0.0f );
	vur.tex1 = vur.tex2 = Vec4( 1.0f, 0.0f, 0.0f, 0.0f );
	vdl.tex1 = vdl.tex2 = Vec4( 0.0f, 1.0f, 0.0f, 0.0f );
	vdr.tex1 = vdr.tex2 = Vec4( 1.0f, 1.0f, 0.0f, 0.0f );
	//set color
	vul.color = vur.color = vdl.color = vdr.color = 0xffffffff;
	//build verts
	_VERTEX_PNCT4T4 lightRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
	lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
	lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;

	Vec2 vScreenPixelSize( 1.0f / (float)pRT->nWidth, 1.0f / (float)pRT->nHeight );
	int last_pass_idx = 0;
	// we start with JUMPFLOOD as src (voronoi seed in it) and paint to TEMP1
	ERTIDChannel arr_swap_rt[] = { K_RTID_JUMPFLOOD , K_RTID_TEMPORARY };
	for ( int i = 0; i < passes; i++ )
	{
		// save last pass so we know what the last RT was in next step
		last_pass_idx = i;
		// offset for each pass is half the previous one, starting at half the square resolution rounded up to nearest power 2.
		// i.e. for 768x512 we round up to 1024x1024 and the offset for the first pass is 512x512, then 256x256, etc.
		float offset = pow( 2, passes - i - 1 );
		///--- set source texture
		CRTManager::CEngineRenderTarget* pRTcolor = __RTManager().GetRTbyUID( arr_swap_rt[i % 2] );
		m_pDevice->SetTexture( 1, pRTcolor->m_pRTTexture );
		m_pDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_POINT );
		m_pDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
		m_pDevice->SetSamplerState( 1, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP );
		m_pDevice->SetSamplerState( 1, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP );

		pRT = __RTManager().GetRTbyUID( arr_swap_rt[(i + 1) % 2] );
		if ( pRT != nullptr )
		{
			if ( OP_SUCCESS( __RTManager().BeginSceneRT( pRT ) ) )
			{
				if ( FAILED( m_pDevice->Clear( 0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB( 0, 0, 0, 0 ), 1.0f, 0 ) ) )
					return K_OP_FAILED;

				__Shaders().SetVSByName( L"VS_COMPOSITION" );
				__Shaders().SetVertexDeclaration( K_SHM_PNCT4T4 );
				__Shaders().SetVSConstantF( 0, (float*)&matWVP, 4 );

				__Shaders().SetPSByName( L"PS_VORONOI_MULTIPASS" );
				//set Pshader constants
				float fConstData[][4] = {
					// x: texture offset
					{ offset, offset, 0.0f, 0.0f},
					// xy: inverse of RT resolution
					{ vScreenPixelSize.x, vScreenPixelSize.y, .0f, .0f },
				};
				__Shaders().SetPSConstantF( 0, (float*)fConstData, ARRAY_SIZE( fConstData ) );

				m_pDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 2, &lightRectV, sizeof( _VERTEX_PNCT4T4 ) );


				// remove VS PS
				__Shaders().SetPS( nullptr );
				__Shaders().SetVS( nullptr );

				V_OP_RET( __RTManager().EndSceneRT( pRT ) );
			}
		}
	}

	///----------------------------------------------------
	/// 4. convert voronoi diagram to distance field
	///----------------------------------------------------
	ERTIDChannel eRTlastVoronoi = arr_swap_rt[(last_pass_idx + 1) % 2];


	CRTManager::CEngineRenderTarget* pRTcolor = __RTManager().GetRTbyUID( eRTlastVoronoi );
	m_pDevice->SetTexture( 0, pRTcolor->m_pRTTexture );
	pRT = __RTManager().GetRTbyUID( K_RTID_DISTANCEFIELD );
	if ( pRT != nullptr )
	{
		if ( OP_SUCCESS( __RTManager().BeginSceneRT( pRT ) ) )
		{
			if ( FAILED( m_pDevice->Clear( 0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB( 0, 0, 0, 0 ), 1.0f, 0 ) ) )
				return K_OP_FAILED;


			__Shaders().SetVSByName( L"VS_COMPOSITION" );
			__Shaders().SetVertexDeclaration( K_SHM_PNCT4T4 );
			__Shaders().SetVSConstantF( 0, (float*)&matWVP, 4 );

			__Shaders().SetPSByName( L"PS_VORONOI_DISTANCE" );
			//set Pshader constants
			float fConstData[][4] = {
				// x: distance modifier (default 8.0, must match x:u_dist_mod from ray tracing shader)
				{ 1.0, 0.0f, 0.0f, 0.0f },
				// xy: inverse of RT resolution
				//{ vScreenPixelSize.x, vScreenPixelSize.y, .0f, .0f },
			};
			__Shaders().SetPSConstantF( 0, (float*)fConstData, ARRAY_SIZE( fConstData ) );

			m_pDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 2, &lightRectV, sizeof( _VERTEX_PNCT4T4 ) );


			// remove VS PS
			__Shaders().SetPS( nullptr );
			__Shaders().SetVS( nullptr );

			V_OP_RET( __RTManager().EndSceneRT( pRT ) );
		}
	}



	///----------------------------------------------------
	/// 5. radiance cascades
	///----------------------------------------------------
	//	for(var n = 0; n < global.radiance_cascade_count; n++) {
	//		shader_set(global.radiance_intervals);
	//		uniform_f1(global.radiance_intervals_uRenderExtent, global.radiance_render_extent);
	//		uniform_f1(global.radiance_intervals_uRenderDecayRate, global.radiance_render_decay);
	//		uniform_tx(global.radiance_intervals_uDistanceField, distfield);
	//		uniform_tx(global.radiance_intervals_uWorldScene, worldscene);
	//
	//		uniform_f1(global.radiance_intervals_uCascadeExtent, global.radiance_cascade_extent);
	//		uniform_f1(global.radiance_intervals_uCascadeSpacing, global.radiance_cascade_spacing);
	//		uniform_f1(global.radiance_intervals_uCascadeInterval, global.radiance_cascade_interval);
	//		uniform_f1(global.radiance_intervals_uCascadeAngular, global.radiance_cascade_angular);
	//		uniform_f1(global.radiance_intervals_uCascadeIndex, n);
	//
	//			surface_set_target(cascade_surfarray[n]);
	//			draw_clear_alpha(c_black, 0);
	//			// It doesn't matter what we render here, we just need a render source to set the render area size.
	//			draw_surface(storage, 0, 0);
	//			surface_reset_target();
	//
	//		shader_reset();
	//	}


	// clamp textures so we don't bleed light
	for ( int kk = 0; kk < 5; kk++ ) {
		m_pDevice->SetSamplerState( kk, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
		m_pDevice->SetSamplerState( kk, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

		m_pDevice->SetSamplerState( kk, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState( kk, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		m_pDevice->SetSamplerState( kk, D3DSAMP_MIPFILTER, 0 );
	}

	int screenWidth = K_GAME_WIDTH * K_RT_PIXEL_SIZE;
	int screenHeight = K_GAME_HEIGHT * K_RT_PIXEL_SIZE;
	Vec2 vaspect( screenWidth / max( screenWidth, screenHeight ), screenHeight / max( screenWidth, screenHeight ) );
	Vec2 vscreen( screenWidth, screenHeight );

	//auto ptex = UTApp().g_texManager.GetTextureByID( FastHash( L"SCENE1024" ) );
	for ( int n = 0; n < UTApp().gi_global.radiance_cascade_count; n++ )
	{
		///--- set textures
		CRTManager::CEngineRenderTarget* pRTlastGI = __RTManager().GetRTbyUID( K_RTID_DISTANCEFIELD );
		m_pDevice->SetTexture( 1, pRTlastGI->m_pRTTexture );
		CRTManager::CEngineRenderTarget* pRTemissive = __RTManager().GetRTbyUID( K_RTID_WORLDSCENE );
		m_pDevice->SetTexture( 2, pRTemissive->m_pRTTexture );

		pRT = __RTManager().GetRTbyUID( K_RTID_CASCADE0 + n );
		if ( (pRT != nullptr) && (OP_SUCCESS( __RTManager().BeginSceneRT( pRT ) )) )
		{
			matWVP = pRT->matProj;

			if ( FAILED( m_pDevice->Clear( 0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB( 0, 0, 0, 0 ), 1.0f, 0 ) ) )
				return K_OP_FAILED;

			__Shaders().SetVSByName( L"VS_COMPOSITION" );
			__Shaders().SetVertexDeclaration( K_SHM_PNCT4T4 );
			__Shaders().SetVSConstantF( 0, (float*)&matWVP, 4 );

			__Shaders().SetPSByName( L"PS_GI_RADIANCE_INTERVALS" );
			///   Name                Reg   Size
			///   ------------------- ----- ----
			///   in_RenderExtent        c0       1
			///   in_CascadeExtent       c1       1
			///   in_CascadeSpacing      c2       1
			///   in_CascadeInterval     c3       1
			///   in_CascadeAngular      c4       1
			///   in_CascadeIndex        c5       1
			///   samp0+in_DistanceField s1       1
			///   samp0+in_WorldScene    s2       1

			///--- set Pshader constants
			float fConstData[][4] = {
				//   in_RenderExtent        c0       1
				{ UTApp().gi_global.radiance_render_extent, 0,0,0},
				//   in_CascadeExtent       c1       1
				{ UTApp().gi_global.radiance_cascade_extent, 0,0,0},
				//   in_CascadeSpacing      c2       1
				{ UTApp().gi_global.radiance_cascade_spacing, 0,0,0},
				//   in_CascadeInterval     c3       1
				{ UTApp().gi_global.radiance_cascade_interval, 0,0,0},
				//   in_CascadeAngular      c4       1
				{ UTApp().gi_global.radiance_cascade_angular, 0,0,0},
				//   in_CascadeIndex        c5       1
				{ n, 0, 0, 0 },
			};
			__Shaders().SetPSConstantF( 0, (float*)fConstData, ARRAY_SIZE( fConstData ) );

			//--- build RT rect ---
			_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
			vul.pos = Vec3( 0.0f, 0.0f, 0.0f );
			vur.pos = Vec3( (float)pRT->nWidth, 0.0f, 0.0f );
			vdl.pos = Vec3( 0.0f, (float)pRT->nHeight, 0.0f );
			vdr.pos = Vec3( (float)pRT->nWidth, (float)pRT->nHeight, 0.0f );

			vul.tex1 = vul.tex2 = Vec4( 0.0f, 0.0f, 0.0f, 0.0f );
			vur.tex1 = vur.tex2 = Vec4( 1.0f, 0.0f, 0.0f, 0.0f );
			vdl.tex1 = vdl.tex2 = Vec4( 0.0f, 1.0f, 0.0f, 0.0f );
			vdr.tex1 = vdr.tex2 = Vec4( 1.0f, 1.0f, 0.0f, 0.0f );

			lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
			lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;
			m_pDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 2, &lightRectV, sizeof( _VERTEX_PNCT4T4 ) );

			// remove VS PS
			__Shaders().SetPS( nullptr );
			__Shaders().SetVS( nullptr );

			V_OP_RET( __RTManager().EndSceneRT( pRT ) );
		}
	}

	
	///----------------------------------------------------
	/// 6. radiance merging
	///----------------------------------------------------

		//for(var n = global.radiance_cascade_count - 1; n >= 0; n--) {
		//	shader_set(global.radiance_merging);
		//	uniform_f1(global.radiance_merging_uCascadeExtent, global.radiance_cascade_extent);
		//	uniform_f1(global.radiance_merging_uCascadeAngular, global.radiance_cascade_angular);
		//	uniform_f1(global.radiance_merging_uCascadeCount, global.radiance_cascade_count);
		//	uniform_f1(global.radiance_merging_uCascadeIndex, n);

		//	var cascaden1 = (n + 1) % global.radiance_cascade_count;
		//	uniform_tx(global.radiance_merging_uCascadeUpper, cascade_surfarray[cascaden1]);

		//	surface_set_target(cascade_temporary);
		//	draw_clear_alpha(c_black, 0);

		//	// In this pass we're reading from cascade N+1 to merge cascade N with cascade N+1.
		//	draw_surface(cascade_surfarray[n], 0, 0);
		//	surface_reset_target();
		//	shader_reset();

		//	// Copy from the tmeporary cascade surface to cascade N.
		//	surface_set_target(cascade_surfarray[n]);
		//	draw_clear_alpha(c_black, 0);
		//	draw_surface(cascade_temporary, 0, 0);
		//	surface_reset_target();
		//}


	for ( int kk = 0; kk < 5; kk++ ) {
		m_pDevice->SetSamplerState( kk, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP );
		m_pDevice->SetSamplerState( kk, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP );

		m_pDevice->SetSamplerState( kk, D3DSAMP_MINFILTER, D3DTEXF_POINT );
		m_pDevice->SetSamplerState( kk, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
		m_pDevice->SetSamplerState( kk, D3DSAMP_MIPFILTER, 0 );
	}

	for ( int n = UTApp().gi_global.radiance_cascade_count - 1; n >= 0; n-- )
	{
		_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
		vul.color = 0xffffffff; vur.color = 0xffffffff; vdl.color = 0xffffffff; vdr.color = 0xffffffff;
		vul.tex1 = vul.tex2 = Vec4( 0.0f, 0.0f, 0.0f, 0.0f );
		vur.tex1 = vur.tex2 = Vec4( 1.0f, 0.0f, 0.0f, 0.0f );
		vdl.tex1 = vdl.tex2 = Vec4( 0.0f, 1.0f, 0.0f, 0.0f );
		vdr.tex1 = vdr.tex2 = Vec4( 1.0f, 1.0f, 0.0f, 0.0f );

		int cascaden1 = (n + 1) % UTApp().gi_global.radiance_cascade_count;
		///--- set textures
		CRTManager::CEngineRenderTarget* pRTlastGI = __RTManager().GetRTbyUID( K_RTID_CASCADE0 + n );
		m_pDevice->SetTexture( 1, pRTlastGI->m_pRTTexture );
		// cascade upper:
		CRTManager::CEngineRenderTarget* pRTemissive = __RTManager().GetRTbyUID( K_RTID_CASCADE0 + cascaden1 );
		m_pDevice->SetTexture( 2, pRTemissive->m_pRTTexture );

		/// we paint into storage then save back into CASCADE texture
		pRT = __RTManager().GetRTbyUID( K_RTID_STORAGE );
		if ( pRT != nullptr && (OP_SUCCESS( __RTManager().BeginSceneRT( pRT ) )) )
		{
			vul.pos = Vec3( 0.0f, 0.0f, 0.0f );
			vur.pos = Vec3( (float)pRT->nWidth, 0.0f, 0.0f );
			vdl.pos = Vec3( 0.0f, (float)pRT->nHeight, 0.0f );
			vdr.pos = Vec3( (float)pRT->nWidth, (float)pRT->nHeight, 0.0f );

			lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
			lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;

			matWVP = pRT->matProj;

			if ( FAILED( m_pDevice->Clear( 0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB( 0, 0, 0, 0 ), 1.0f, 0 ) ) )
				return K_OP_FAILED;

			__Shaders().SetVSByName( L"VS_COMPOSITION" );
			__Shaders().SetVertexDeclaration( K_SHM_PNCT4T4 );
			__Shaders().SetVSConstantF( 0, (float*)&matWVP, 4 );

			__Shaders().SetPSByName( L"PS_GI_RADIANCE_MERGING" );
			///   Name                Reg   Size
			///   ------------------- ----- ----
			///   in_CascadeExtent      c0       1
			///   in_CascadeAngular     c1       1
			///   in_CascadeCount       c2       1
			///   in_CascadeIndex       c3       1
			///   samp0+gm_BaseTexture  s1       1
			///   samp0+in_CascadeAtlas s2       1

			///--- set Pshader constants
			float fConstData[][4] = {
				{ UTApp().gi_global.radiance_cascade_extent, 0, 0, 0 },
				{ UTApp().gi_global.radiance_cascade_angular, 0, 0, 0 },
				{ (float)UTApp().gi_global.radiance_cascade_count, 0, 0, 0 },
				{ (float)n, 0, 0, 0 },
			};
			__Shaders().SetPSConstantF( 0, (float*)fConstData, ARRAY_SIZE( fConstData ) );

			m_pDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 2, &lightRectV, sizeof( _VERTEX_PNCT4T4 ) );

			// remove VS PS
			__Shaders().SetPS( nullptr );
			__Shaders().SetVS( nullptr );

			V_OP_RET( __RTManager().EndSceneRT( pRT ) );
		}
		///--- moving result from STORAGE back to cascade (must have same size)
		pRT = __RTManager().GetRTbyUID( K_RTID_CASCADE0 + n );
		if ( pRT != nullptr && (OP_SUCCESS( __RTManager().BeginSceneRT( pRT ) )) )
		{
			CRTManager::CEngineRenderTarget* pRTlastGI = __RTManager().GetRTbyUID( K_RTID_STORAGE );
			m_pDevice->SetTexture( 0, pRTlastGI->m_pRTTexture );

			matWVP = pRT->matProj;
			if ( FAILED( m_pDevice->Clear( 0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB( 0, 0, 0, 0 ), 1.0f, 0 ) ) )
				return K_OP_FAILED;

			__Shaders().SetVSByName( L"VS_COMPOSITION" );
			__Shaders().SetVertexDeclaration( K_SHM_PNCT4T4 );
			__Shaders().SetVSConstantF( 0, (float*)&matWVP, 4 );
			__Shaders().SetPS( nullptr );
			// no PS - just copy
			__Shaders().SetPSByName( L"PS_COPY1TEX" );

			m_pDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 2, &lightRectV, sizeof( _VERTEX_PNCT4T4 ) );

			// remove VS PS
			__Shaders().SetPS( nullptr );
			__Shaders().SetVS( nullptr );

			V_OP_RET( __RTManager().EndSceneRT( pRT ) );
		}

	}


	///----------------------------------------------------
	/// 7. radiance mipmap
	///----------------------------------------------------
	
	//function radiancecascades_mipmap( cascade_surfarray, mipmap_surf ) {
	//	var mipmap_width = surface_get_width( mipmap_surf );
	//	var mipmap_height = surface_get_width( mipmap_surf );
	//	var mipmap0_width = surface_get_width( mipmap_surf );
	//	var mipmap0_height = surface_get_width( mipmap_surf );

	//	shader_set( global.radiance_mipmap );
	//	uniform_f1( global.radiance_mipmap_uMipMapExtent, max( mipmap_width, mipmap_height ) );
	//	uniform_f1( global.radiance_mipmap_uCascadeExtent, global.radiance_cascade_extent );
	//	uniform_f1( global.radiance_mipmap_uCascadeAngular, global.radiance_cascade_angular );
	//	uniform_f1( global.radiance_mipmap_uCascadeIndex, 0);
	//	uniform_tx( global.radiance_mipmap_uCascadeAtlas, cascade_surfarray[0] );

	//	surface_set_target( mipmap_surf );
	//	draw_clear_alpha( c_black, 0 );
	//	draw_surface_ext( mipmap_surf, 0, 0, mipmap_width / mipmap0_width, mipmap_height / mipmap0_height, 0, c_black, 1 );
	//	surface_reset_target();

	//	shader_reset();
		



	///--- set cascade 0 texture
	CRTManager::CEngineRenderTarget* pRTlastGI = __RTManager().GetRTbyUID( K_RTID_CASCADE0 );
	m_pDevice->SetTexture( 1, pRTlastGI->m_pRTTexture );
	/// we paint into storage then save back into CASCADE texture
	pRT = __RTManager().GetRTbyUID( K_RTID_MIPMAP );
	if ( pRT != nullptr && (OP_SUCCESS( __RTManager().BeginSceneRT( pRT ) )) )
	{
		float mipmap_extent = (float)max(pRT->nWidth, pRT->nHeight);

		vul.pos = Vec3( 0.0f, 0.0f, 0.0f );
		vur.pos = Vec3( (float)pRT->nWidth, 0.0f, 0.0f );
		vdl.pos = Vec3( 0.0f, (float)pRT->nHeight, 0.0f );
		vdr.pos = Vec3( (float)pRT->nWidth, (float)pRT->nHeight, 0.0f );

		lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
		lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;

		matWVP = pRT->matProj;

		if ( FAILED( m_pDevice->Clear( 0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_ARGB( 1, 0, 0, 0 ), 1.0f, 0 ) ) )
			return K_OP_FAILED;

		__Shaders().SetVSByName( L"VS_COMPOSITION" );
		__Shaders().SetVertexDeclaration( K_SHM_PNCT4T4 );
		__Shaders().SetVSConstantF( 0, (float*)&matWVP, 4 );

		__Shaders().SetPSByName( L"PS_GI_RADIANCE_MIPMAP" );
		///   Name             Reg   Size
		///   ---------------- ----- ----
		///   in_MipMapExtent  c0       1
		///   in_CascadeExtent c1       1
		///   in_CascadeAtlas  s0       1

		///--- set Pshader constants
		float fConstData[][4] = {
			{ mipmap_extent, 0, 0, 0 },
			{ (float)UTApp().gi_global.radiance_cascade_extent, 0, 0, 0 },
		};
		__Shaders().SetPSConstantF( 0, (float*)fConstData, ARRAY_SIZE( fConstData ) );

		m_pDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 2, &lightRectV, sizeof( _VERTEX_PNCT4T4 ) );

		// remove VS PS
		__Shaders().SetPS( nullptr );
		__Shaders().SetVS( nullptr );

		V_OP_RET( __RTManager().EndSceneRT( pRT ) );
	}
	





	///----------------------------------------------------
	/// FINAL COMPOSITION - composes buffers into one
	///----------------------------------------------------
	pRT = __RTManager().GetRTbyUID( K_RTID_FINAL );
	if ( pRT != nullptr )
	{
		if ( OP_SUCCESS( __RTManager().BeginSceneRT( pRT ) ) )
		{
			// Clear the render target and the zbuffer
			if ( FAILED( m_pDevice->Clear( 0, nullptr, D3DCLEAR_TARGET, 0xffff0000, 1.0f, 0 ) ) )
			{
				return K_OP_FAILED;
			}

			//#TODO: este corect ?? offset the projection matrix by 0.5f because in DX the pixel's 0.0 is the center of the pixel
			//Mat matProj;
			//D3DXMatrixOrthoOffCenterLH(&matProj, 0.5f, pRT->nWidth + 0.5f, pRT->nHeight + 0.5f, 0.5f, 0.0f, 1.0f);
			m_pDevice->SetTransform( D3DTS_PROJECTION, &pRT->matProj );

			m_pDevice->SetTransform( D3DTS_WORLD, &g_matIdentity );
			m_pDevice->SetTransform( D3DTS_VIEW, &g_matIdentity );

			// RT sized quad with tex1 color, tex2 lightmap
			RenderPass_Composition( &pRT->matProj, fBetweenFramesPercent );

			V_OP_RET( __RTManager().EndSceneRT( pRT ) );

		}
	}

	return K_OP_OK;
}

OPRESULT CLevel::RenderPass( eLVLRenderPass ePass, Matrix* matProj, float fBetweenFramesPercent )
{
	_ASSERT( (ePass > K_LVL_RP_NONE) && (ePass < K_LVL_RP_COUNT) );

	Matrix	matView;

	RectXYWH		camrect = m_camLevelToRT.GetCamWorldAABB();
	CAABB			camAABB( camrect );

	//locally used temp matrix
	Matrix	matlocal;

	///----------------------------------------------------
	/// INITIAL SETUP
	///----------------------------------------------------
	m_pDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );
	m_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	m_pDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	m_pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

	m_pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	m_pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	m_pDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );

	m_pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	m_pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	m_pDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
	if ( (UTApp().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0 )
	{
		//#IMPORTANT: we need separate alpha blending or it will look bad when blending alpha values between them.
		//eg: if we blend semitransparent things on top of fully opaque walls, the walls become transparent
		//necessary but not really well supported. Better with a shader and custom sprite painter
		m_pDevice->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, true );
		m_pDevice->SetRenderState( D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA );
		m_pDevice->SetRenderState( D3DRS_DESTBLENDALPHA, D3DBLEND_DESTALPHA );
		m_pDevice->SetRenderState( D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD );
	}

	//#HACK: we floor the camera pos if we get UV seams in DX9. See LoadArea for another hack regarding UV coords and UV seams (UV shrinking)
	// moves from tex pixel to pixel, no half pixels but we add the subpixel movement when painting the final scene so if moves smoothly
	MUMatAffine2D( &matView, K_RT_PIXEL_SIZE_F, nullptr, 0.0f, &Vec2( -floor( camrect.x ) * K_RT_PIXEL_SIZE_F, -floor( camrect.y ) * K_RT_PIXEL_SIZE_F ) );
	m_pDevice->SetTransform( D3DTS_VIEW, &matView );
	m_pDevice->SetTransform( D3DTS_WORLD, &g_matIdentity );

	Matrix matWVP = matView * (*matProj);

	__Shaders().SetVS( nullptr );
	__Shaders().SetPS( nullptr );

	int nTexIdxOffset = 0; // for objects - texture index offset so we paint from the normals texture when we render the normals pass
	bool bPaintsNormals = false;
	ETexChannel	eTexChannel = K_TEXCHAN_NONE;
	switch ( ePass )
	{
		case K_LVL_RP_COLORS:
		{
			nTexIdxOffset = 0;
			bPaintsNormals = false;
			eTexChannel = K_TEXCHAN_COLORMAP;
		}
		break;
		case K_LVL_RP_NORMALS_HEIGHT:
		{
			nTexIdxOffset = 2; //#HACK: normal textures are added at the end after 2 color textures
			bPaintsNormals = true;
			eTexChannel = K_TEXCHAN_NORMALMAP;
		}
		break;
		case K_LVL_RP_LIGHTS:
		{
			ErrorBox( K_ERR_WARNING, L"Render lights using RenderPass_Lights() instead!" );
			return K_OP_OK;
		}
		break;
		default:
			// unknown pass -> exit fn
			return K_OP_OK;
	}

	/// paint floors and vertical walls
	CTexNode* pTex = m_tilesetDesc.GetTexture( K_AL_UNDER_FLOOR, bPaintsNormals );
	m_pDevice->SetTexture( 0, pTex->pTexture );
	// paint water with special shader on color pass
	if ( Areas_IsLayerVisible( K_AL_UNDER_FLOOR ) )
	{
		if ( ePass == K_LVL_RP_COLORS )
		{
			CTexNode* pTexWater = m_tilesetDesc.pWaterTex;
			CTexNode* pTexNrm = m_tilesetDesc.GetTexture( K_AL_UNDER_FLOOR, true );
			m_pDevice->SetTexture( 1, pTexNrm->pTexture );
			m_pDevice->SetTexture( 2, pTexWater->pTexture );
			__Shaders().SetVSByName( L"VS_WATER" );
			__Shaders().SetVertexDeclaration( K_SHM_PNCT4T4 );
			__Shaders().SetVSConstantF( 0, (float*)&matWVP, 4 );

			__Shaders().SetPSByName( L"PS_WATER" );
			//set Pshader constants
			float fConstData[][4] = {
				// x: murkyness multiplier,
				{ ct_waterFog, ct_waterScale, ct_waterDiffract, ct_waterHeight },
				// water color (f4)
				{ 0.13, 0.16, 0.56f, 1.0f },
				// x: surface color adder, y: specular alpha
				{ ct_waterColorAdd, ct_waterSpecular, 1.0f, 1.0f },
				// UV animation	for 2 layers of water
				{ sin( (float)fLocalTimeline * 0.9 ) * 0.05f, cos( (float)fLocalTimeline * 0.45 ) * 0.07f, sin( (float)fLocalTimeline * 1.0 ) * 0.06f, cos( (float)fLocalTimeline * 0.5 ) * 0.06},
			};
			__Shaders().SetPSConstantF( 0, (float*)fConstData, ARRAY_SIZE( fConstData ) );


			Areas_PaintLayer( K_AL_UNDER_FLOOR );

			m_pDevice->SetTexture( 1, nullptr );
			__Shaders().SetVS( nullptr );
			__Shaders().SetPS( nullptr );
		}
		else
		{
			Areas_PaintLayer( K_AL_UNDER_FLOOR );
		}
	}

	/// normal floors
	pTex = m_tilesetDesc.GetTexture( K_AL_FLOOR, bPaintsNormals );
	m_pDevice->SetTexture( 0, pTex->pTexture );
	Areas_PaintLayer( K_AL_FLOOR );
	/// vertical walls
	pTex = m_tilesetDesc.GetTexture( K_AL_WALLS, bPaintsNormals );
	m_pDevice->SetTexture( 0, pTex->pTexture );
	Areas_PaintLayer( K_AL_WALLS );

	/// SHADOWS - blends wall shadows into the color map so it won't come over the players heads
	if ( ePass == K_LVL_RP_COLORS )
	{
		CSpriteLib* spr_lights = m_sprLib.GetLibByNick( K_LIBNICK_LIGHTS );
		scTexture* pShadowsTex = spr_lights->GetTextureByAnim( ANM_LIGHTS_SPR_SHADOWS, 0, 0 );
		if ( pShadowsTex )
			m_pDevice->SetTexture( 0, pShadowsTex->pTex );

		Areas_PaintLayer( K_AL_WALLSHADOWS );
	}

	m_pDevice->SetTransform( D3DTS_WORLD, &g_matIdentity );
	///--- BEGIN SPRITES PAINTER ---
	PVERTEXSHADER pSprVS = __Shaders().GetVShaderByName( L"VS_SPRITES2D" );
	if ( pSprVS )
		__Painter().Begin( pSprVS, matView, *matProj );

	// paint non sorted floor and wall elements
	if ( ePass != K_LVL_RP_NORMALS_HEIGHT ) {
		for ( int kk = 0; kk < m_visibleList.visible_props.nCount; kk++ )
		{
			CProp* prop = m_visibleList.visible_props[kk];
			if ( (prop->editor_layer == K_TILE_LAYER_UNDER_FLOOR) ||
				(prop->editor_layer == K_TILE_LAYER_FLOOR) ||
				(prop->editor_layer == K_TILE_LAYER_WALLS) ||
				(prop->editor_layer == K_TILE_LAYER_WALLS_DECO) )
			{
				prop->sprite.PaintFModule_texOverride( 0, nTexIdxOffset );
			}
		}
	}
	// paint sortable elements
	eVisibleSortableType eLastVis = K_VST_UNKNOWN;
	for ( int kk = 0; kk < m_visibleList.arrSortedItems.nCount; kk++ )
	{
		CVisibleSortable* vis = &m_visibleList.arrSortedItems.m_pData[kk];

		switch ( vis->eType )
		{
			case K_VST_ACTOR:
			{
				CActor* act = static_cast<CActor*>( vis->pPtr );
				act->Paint( eTexChannel );

				//#TEMP: paint target position
				UTSprite::PaintFModule( &m_sprInterface, act->GetAI()->AIsensor.vGoTo + Vec2( 2.0f, 2.0f ), ANM_IGM_INTERFACE_SPR_IGM_STRATEGIC_EFFECTS, 4, 0, 0x88ff0000 );
				//#TEMP: paint waypoints
				for ( int ll = 0; ll < act->GetAI()->AIsensor.arrGoToPoints.Count(); ll++ )
				{
					Vec2 pathpt = act->GetAI()->AIsensor.arrGoToPoints[ll];
					UTSprite::PaintFModule( &m_sprInterface, pathpt, ANM_IGM_INTERFACE_SPR_IGM_STRATEGIC_EFFECTS, 4, 0, 0x880000ff );
				}

				/*
				VecProj vpMuzz = act->GetCurWeaponMuzzleWorld();
				UTSprite::PaintFrame( &m_sprInterface, vpMuzz.xy.x, vpMuzz.xy.y, ANM_IGM_INTERFACE_SPR_IGM_STRATEGIC_EFFECTS, 4, 0x88ff0000 );
				UTSprite::PaintFrame( &m_sprInterface, vpMuzz.xy_proj.x, vpMuzz.xy_proj.y, ANM_IGM_INTERFACE_SPR_IGM_STRATEGIC_EFFECTS, 4, 0x8800ff00 );
				*/

			}
			break;
			case K_VST_PROP:
			{
				CProp* prop = static_cast<CProp*>(vis->pPtr);
				prop->sprite.PaintFModule_texOverride( 0, nTexIdxOffset );
			}
			break;
			default:
				break;
		}

		// save last type of painted item
		eLastVis = vis->eType;
	}

	__Painter().Flush();
	// paint the doofers
	PaintDoofers( ePass );
	// now paint the bullets
	PaintBullets( ePass );

	if ( ePass == K_LVL_RP_COLORS )
	{
		__Particles().PaintLayer( K_PART_LAYER_NORMAL );
	}

	/// END SPRITES PAINTER
	__Painter().End();

	// top layer of tiles
	__Shaders().SetVS( nullptr );
	pTex = m_tilesetDesc.GetTexture( K_AL_CEILINGS, bPaintsNormals );
	m_pDevice->SetTexture( 0, pTex->pTexture );
	Areas_PaintLayer( K_AL_CEILINGS );

	return K_OP_OK;
}

OPRESULT CLevel::RenderPass_Lights( Matrix* matProj, float fBetweenFramesPercent )
{
	Matrix				matView;

	RectXYWH		camrect = m_camLevelToRT.GetCamWorldAABB();
	CAABB			camAABB( camrect );

	CSpriteLib* spr_lights = m_sprLib.GetLibByNick( K_LIBNICK_LIGHTS );
	///----------------------------------------------------
	/// INITIAL SETUP
	///----------------------------------------------------
	m_pDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );

	m_pDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	m_pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	m_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	m_pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	m_pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	m_pDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );

	m_pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	m_pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	m_pDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
	if ( (UTApp().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0 )
	{
		//#IMPORTANT: we need separate alpha blending or it will look bad when blending alpha values between them.
		//eg: if we blend semitransparent things on top of fully opaque walls, the walls become transparent
		//necessary but not really well supported. Better with a shader and custom sprite painter
		m_pDevice->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, true );
		m_pDevice->SetRenderState( D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA );
		m_pDevice->SetRenderState( D3DRS_DESTBLENDALPHA, D3DBLEND_DESTALPHA );
		m_pDevice->SetRenderState( D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD );
	}

	//#HACK: we floor the camera pos if we get UV seams in DX9. See LoadArea for another hack regarding UV coords and UV seams
	MUMatAffine2D( &matView, K_RT_PIXEL_SIZE_F, nullptr, 0.0f, &Vec2( -floor( camrect.x ) * K_RT_PIXEL_SIZE_F, -floor( camrect.y ) * K_RT_PIXEL_SIZE_F ) );

	m_pDevice->SetTransform( D3DTS_VIEW, &matView );
	m_pDevice->SetTransform( D3DTS_WORLD, &g_matIdentity );


	///--- paint lights ---
	//PVERTEXSHADER pVShader = null;
	//PPIXELSHADER pPShader = null;
	// matWVP is used by level
	Matrix matWVP = matView * (*matProj);
	// begin the painter
	PVERTEXSHADER pSprVS = __Shaders().GetVShaderByName( L"VS_SPRITES2D" );
	if ( pSprVS )
		__Painter().Begin( pSprVS, matView, *matProj );



	// generic VS data so we can automatically find positions
	float fConstDataVS[][4] = {
		{ floor( camrect.x ), floor( camrect.y ), camrect.w, camrect.h } //RTT rect_xywh in world coords
		//#HACK: if flooring the campos then floor this camrect too that gets sent to the shader, but floor it to submultiples of pixel size (shader view is real space not screen space)
		//{ floor(camrect.x * K_RT_PIXEL_SIZE_F ) / K_RT_PIXEL_SIZE_F, floor(camrect.y * K_RT_PIXEL_SIZE_F ) / K_RT_PIXEL_SIZE_F, camrect.w, camrect.h } //RTT rect_xywh in world coords
	};

	UT3D::DeviceAdditiveON( m_pDevice );

	/*
	// directional light with shader, more expensive, harder to control
	CRTManager::CEngineRenderTarget* pRT = UTGetRTManager().GetRTbyUID(K_RTID_TEMP1);
	if (pRT != null)
	{
		m_pDevice->SetTexture(0, pRT->m_pRTTexture);
	}


	///--- directional light(s)
	// VS
	UTGetShaderManager().SetVSByName(L"VS_POINTLIGHT");
	UTGetShaderManager().SetVertexDeclaration(K_SHM_PNCT4T4);
	UTGetShaderManager().SetVSConstantF(0, (float*)&matWVP, 4);
	UTGetShaderManager().SetVSConstantF(4, (float*)fConstDataVS, ARRAY_SIZE(fConstDataVS));
	// PS
	UTGetShaderManager().SetPSByName(L"PS_DIRECTIONAL");
	for (int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++)
	{
		CLight *nl = m_visibleList.visible_lights.m_pData[kk];
		if (nl->type != K_LVL_LT_DIRECTIONAL)
			continue;

		//set Pshader constants
		float fConstData[][4] = {
			//x: intensity
			{ nl->fIntensity, 0.0f, 0.0f, 0.0f },
			// xyz: inversed normalized directon
			{ -nl->vnDir.x, -nl->vnDir.y, -nl->vnDir.z, 0.0f }
		};
		UTGetShaderManager().SetPSConstantF(0, (float*)fConstData, ARRAY_SIZE(fConstData));
		m_bufferedPainter.DrawMesh(nl->m_nLightMeshIdx, true);
	}
	*/

	__Shaders().SetVS( nullptr );
	__Shaders().SetPS( nullptr );

	///--- directional lights (under shadow)
	//#TODO: should be completely removed....
	// directional light without shader, doesn't take into account the object normals
	m_pDevice->SetTexture( 0, nullptr );
	m_pDevice->SetTexture( 1, nullptr );
	for ( int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++ )
	{
		CLight* nl = m_visibleList.visible_lights.m_pData[kk];
		if ( nl->type == K_LVL_LT_DIRECTIONAL )
		{
			//paint and exit
			m_bufferedPainter.DrawMesh( nl->m_nLightMeshIdx, true );
			break;
		}
	}


	UT3D::DeviceAdditiveOFF( m_pDevice );
	///--- ambient light(s)
	// paint all general ambient lights and area lights here
	//#TODO: paint one ambiental per area!
	__Shaders().SetVS( nullptr );
	__Shaders().SetPS( nullptr );

	m_pDevice->SetTexture( 0, nullptr );
	m_pDevice->SetTexture( 1, nullptr );

	for ( int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++ )
	{
		CLight* nl = m_visibleList.visible_lights.m_pData[kk];
		if ( nl->type != K_LVL_LT_AMBIENTAL )
			continue;
		//paint and exit
		m_bufferedPainter.DrawMesh( nl->m_nLightMeshIdx, true );
		break;
	}


#if defined(DEBUG_LIGHTS)
	// Paints the shadowed lights volume in wireframe, the one selected in the editor
	if ( g_editor.IsLaunched() )
	{
		IActiveInterface* pLight = g_editor.GetSelected();
		if ( pLight != nullptr && pLight->GetClassType() == K_LVL_IAI_TYPE_LIGHT )
		{
			m_pDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
			m_pDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD ); //needed for color interpolation
			for ( int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++ )
			{
				CLight* nl = m_visibleList.visible_lights.m_pData[kk];
				if ( nl != pLight )
					continue;

				if ( (nl->type != K_LVL_LT_POINT) || (!nl->GetCastShadows()) )
					continue;

				m_bufferedPainter.DrawMesh( nl->m_nLightMeshIdx, true );
			}
			m_pDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );
			m_pDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
		}
	}
#endif


	///----------------------------------------------------------------------------------
	/// SHADOWS
	///----------------------------------------------------------------------------------
	//scTexture* pShadowsTex = m_sprLights.GetTextureByAnim( ANM_LIGHTS_SPR_SHADOWS, 0, 0 );
	//if ( pShadowsTex )
		//m_pDevice->SetTexture( 0, pShadowsTex->pTex );
	/// actor shadows
	for ( int kk = 0; kk < m_visibleList.arrSortedItems.nCount; kk++ )
	{
		CVisibleSortable* vis = &m_visibleList.arrSortedItems.m_pData[kk];
		if ( vis->eType != K_VST_ACTOR )
			continue;
		CActor* act = static_cast<CActor*>( vis->pPtr );
		UTSprite::PaintFModule( spr_lights, act->pos.xy, ANM_LIGHTS_SPR_CHAR_SHADOWS, 0, 0 );
	}
	__Painter().Flush();

	///----------------------------------------------------------------------------------
	/// LIGHTS
	///----------------------------------------------------------------------------------
	UT3D::DeviceAdditiveON( m_pDevice );

	///--- bullet lights
	// bullet shadows
	PaintBullets( K_LVL_RP_LIGHTS );
	__Painter().Flush();

	///--- point lights
	CRTManager::CEngineRenderTarget* pRT = __RTManager().GetRTbyUID( K_RTID_TEMP1 );
	if ( pRT != null )
	{
		m_pDevice->SetTexture( 0, pRT->m_pRTTexture );
	}
	// VS
	__Shaders().SetVSByName( L"VS_POINTLIGHT" );
	__Shaders().SetVertexDeclaration( K_SHM_PNCT4T4 );
	__Shaders().SetVSConstantF( 0, (float*)&matWVP, 4 );
	__Shaders().SetVSConstantF( 4, (float*)fConstDataVS, ARRAY_SIZE( fConstDataVS ) );
	// PS
	__Shaders().SetPSByName( L"PS_POINTLIGHT" );

	for ( int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++ )
	{
		CLight* nl = m_visibleList.visible_lights.m_pData[kk];

		if ( nl->type != K_LVL_LT_POINT )
			continue;

		//set Pshader constants
		float fConstData[][4] = {
			// x:atten c1, y:atten c2, z:light radius, w:
			{ nl->fIntensity, nl->fRadius, 0.0f, 0.0f },
			// x: game height projection, y: height projection inverse (projected -> real), z: gauss dist atten factor
			{ ZHSCALE, INV_ZHSCALE, ct_fGaussLen, 0.0f },
			// xyz: light world position
			{ nl->pos.xyz.x, nl->pos.xyz.y, nl->pos.xyz.z, 0.0f }
		};
		__Shaders().SetPSConstantF( 0, (float*)fConstData, ARRAY_SIZE( fConstData ) );
		m_bufferedPainter.DrawMesh( nl->m_nLightMeshIdx, false );
	}


	///--- directional projected lights
	//all directional projected light must be in the same animation
	scTexture* pLightTex = spr_lights->GetTextureByAnim( ANM_LIGHTS_SPR_PROJECTED_DIR, 0, 0 );
	if ( pLightTex )
		m_pDevice->SetTexture( 1, pLightTex->pTex );
	m_pDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	m_pDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_POINT );

	__Shaders().SetVSByName( L"VS_PROJECTEDDIR" );
	__Shaders().SetVertexDeclaration( K_SHM_PNCT4T4 );
	__Shaders().SetVSConstantF( 0, (float*)&matWVP, 4 );
	__Shaders().SetVSConstantF( 4, (float*)fConstDataVS, ARRAY_SIZE( fConstDataVS ) );

	__Shaders().SetPSByName( L"PS_PROJECTEDDIR" );
	for ( int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++ )
	{
		CLight* nl = m_visibleList.visible_lights.m_pData[kk];
		CAABB bbox_ini = nl->bbox.GetSnapshot();

		if ( nl->type != K_LVL_LT_PROJECTED_DIR )
			continue;

		//set Pshader constants
		float fConstData[][4] = {
			//x: light intensity, y: geometry half size W, z: geometry half size H
			{ nl->fIntensity, bbox_ini.vHalfSize.x, bbox_ini.vHalfSize.y, 0.0f },
			// x: game height projection, y: height projection inverse (projected -> real), z: gauss dist atten factor
			{ ZHSCALE, INV_ZHSCALE, ct_fGaussLen, 0.0f },
			// xyz: light world position
			{ nl->pos.xyz.x, nl->pos.xyz.y, nl->pos.xyz.z, 0.0f },
			// xyz: direction of light, normalized
			{ nl->vnDir.x, nl->vnDir.y, nl->vnDir.z, 0.0f },
			// xy: UL tex spot coords; zw: WH spot width height
			{ nl->lTexRect.left, nl->lTexRect.top, nl->lTexRect.right - nl->lTexRect.left, nl->lTexRect.bottom - nl->lTexRect.top }
		};
		__Shaders().SetPSConstantF( 0, (float*)fConstData, ARRAY_SIZE( fConstData ) );
		m_bufferedPainter.DrawMesh( nl->m_nLightMeshIdx, false );
	}


	///--- IES lights without shadow
	scTexture* pIESTex = spr_lights->GetTextureByAnim( ANM_LIGHTS_SPR_IES, 0, 0 );
	_ASSERT( pIESTex != nullptr );
	m_pDevice->SetTexture( 1, pIESTex->pTex );
	m_pDevice->SetSamplerState( 1, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	m_pDevice->SetSamplerState( 1, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	// VS
	__Shaders().SetVSByName( L"VS_POINTLIGHT" );
	__Shaders().SetVertexDeclaration( K_SHM_PNCT4T4 );
	__Shaders().SetVSConstantF( 0, (float*)&matWVP, 4 );
	__Shaders().SetVSConstantF( 4, (float*)fConstDataVS, ARRAY_SIZE( fConstDataVS ) );
	// PS
	__Shaders().SetPSByName( L"PS_IESLIGHT" );

	for ( int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++ )
	{
		CLight* nl = m_visibleList.visible_lights.m_pData[kk];

		if ( (nl->type != K_LVL_LT_IES) || (nl->GetCastShadows()) )
			continue;

		//the IES dot texture has 3 pixel lines per IES profile so we don't get interpolation problems
		float IES_texV = (float)(nl->nProfileID * 3 + 1) / (float)pIESTex->info.Height;
		//set Pshader constants
		float fConstData[][4] = {
			//x:light intensity, y:light radius, z: IES profile (V in texture coordinates)
			{ nl->fIntensity, nl->fRadius, IES_texV, 0.0f },
			// x: game height projection, y: height projection inverse (projected -> real), z: gauss dist atten factor
			{ ZHSCALE, INV_ZHSCALE, ct_fGaussLen, 0.0f },
			// xyz: light world position
			{ nl->pos.xyz.x, nl->pos.xyz.y, nl->pos.xyz.z, 0.0f },
			// light direction normalized
			{ nl->vnDir.x, nl->vnDir.y, nl->vnDir.z, 0.0f }
		};
		__Shaders().SetPSConstantF( 0, (float*)fConstData, ARRAY_SIZE( fConstData ) );
		m_bufferedPainter.DrawMesh( nl->m_nLightMeshIdx, false );
	}

	__Shaders().SetVS( nullptr );
	__Shaders().SetPS( nullptr );


	UT3D::DeviceAdditiveOFF( m_pDevice );
	// end sprite painter
	__Painter().End();


	return K_OP_OK;
}

OPRESULT CLevel::RenderPass_GIEmissive( Matrix* matProj, float /*fBetweenFramesPercent*/ )
{
	CSpriteLib* sprlib_lights = m_sprLib.GetLibByNick( K_LIBNICK_LIGHTS );

	Matrix	matView;

	RectXYWH	camrect = m_camLevelToRT.GetCamWorldAABB();
	Vec2		campos = m_camLevelToRT.GetCamPos();
	float		renderw = UTApp().gi_global.radiance_render_extent;
	CAABB		camAABB( campos.x - renderw, campos.y - renderw, campos.x + renderw, campos.y + renderw);

	//locally used temp matrix
	Matrix	matlocal;

	///----------------------------------------------------
	/// INITIAL SETUP
	///----------------------------------------------------
	m_pDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );
	m_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	m_pDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	m_pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );

	m_pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	m_pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	m_pDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );

	m_pDevice->SetTransform( D3DTS_WORLD, &g_matIdentity );

	//#HACK: we floor the camera pos if we get UV seams in DX9. See LoadArea for another hack regarding UV coords and UV seams (UV shrinking)
	// moves from tex pixel to pixel, no half pixels but we add the subpixel movement when painting the final scene so if moves smoothly
	MUMatAffine2D( &matView, K_RT_PIXEL_SIZE_F, nullptr, 0.0f, &Vec2( -floor( camrect.x ) * K_RT_PIXEL_SIZE_F, -floor( camrect.y ) * K_RT_PIXEL_SIZE_F ) );
	m_pDevice->SetTransform( D3DTS_VIEW, &matView );
	m_pDevice->SetTransform( D3DTS_PROJECTION, matProj);

	//Matrix matWVP = matView *(*matProj);

	__Shaders().SetVS( nullptr );
	__Shaders().SetPS( nullptr );

	/// paint occluders in black
	auto ptexnoise = UTApp().g_texManager.GetTextureByID( FastHash( L"BLACK32" ) );	
	m_pDevice->SetTexture( 0, ptexnoise->pTexture );

	Areas_PaintLayer( K_AL_OCCLUDERS );

	m_pDevice->SetTransform( D3DTS_WORLD, &g_matIdentity );
	///--- BEGIN SPRITES PAINTER ---
	PVERTEXSHADER pSprVS = __Shaders().GetVShaderByName( L"VS_SPRITES2D" );
	if ( pSprVS )
		__Painter().Begin( pSprVS, matView, *matProj );

	for ( int kk = 0; kk < m_visibleList.visible_lights.Count(); kk++ )
	{
		CLight* nl = m_visibleList.visible_lights.m_pData[kk];
		CSpr spr( sprlib_lights, ANM_LIGHTS_SPR_GI_LIGHTS, nl->pos.xy );
		spr.frameIdx = 1;
		spr.color = nl->color;
		spr.Paint();
	}

	__Painter().Flush();

	/// END SPRITES PAINTER
	__Painter().End();


	__Shaders().SetVS( nullptr );
	__Shaders().SetPS( nullptr );
	return K_OP_OK;
}


OPRESULT CLevel::RenderPass_Composition( Matrix* matProj, float fBetweenFramesPercent )
{
	Matrix				matView;
	///----------------------------------------------------
	/// INITIAL SETUP
	///----------------------------------------------------
	m_pDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT );

	m_pDevice->SetRenderState( D3DRS_ZENABLE, FALSE );
	m_pDevice->SetRenderState( D3DRS_ZWRITEENABLE, FALSE );
	m_pDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	m_pDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_POINT );
	m_pDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT );
	m_pDevice->SetSamplerState( 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE );

	m_pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	m_pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	m_pDevice->SetRenderState( D3DRS_BLENDOP, D3DBLENDOP_ADD );
	if ( (UTApp().g_gfxFlags & K_UT_GFXFLAG_SEPARATEALPHABLEND) != 0 )
	{
		//#IMPORTANT: we need separate alpha blending or it will look bad when blending alpha values between them.
		//eg: if we blend semitransparent things on top of fully opaque walls, the walls become transparent
		//necessary but not really well supported. Better with a shader and custom sprite painter
		m_pDevice->SetRenderState( D3DRS_SEPARATEALPHABLENDENABLE, true );
		m_pDevice->SetRenderState( D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA );
		m_pDevice->SetRenderState( D3DRS_DESTBLENDALPHA, D3DBLEND_DESTALPHA );
		m_pDevice->SetRenderState( D3DRS_BLENDOPALPHA, D3DBLENDOP_ADD );
	}

	MUMatIdentity( &matView );
	m_pDevice->SetTransform( D3DTS_VIEW, &matView );
	m_pDevice->SetTransform( D3DTS_WORLD, &g_matIdentity );

	///--- compose scene from normals and color ---
	PVERTEXSHADER pVShader = null;
	PPIXELSHADER pPShader = null;
	Matrix matWVP = matView * (*matProj);

	CRTManager::CEngineRenderTarget* pRTcolor = __RTManager().GetRTbyUID( K_RTID_TEMP1 );
	CRTManager::CEngineRenderTarget* pRTlights = __RTManager().GetRTbyUID( K_RTID_COLORDEPTHSTENCIL );
	_ASSERT( pRTcolor != nullptr && pRTlights != nullptr );
	m_pDevice->SetTexture( 0, pRTcolor->m_pRTTexture );
	m_pDevice->SetTexture( 1, pRTlights->m_pRTTexture );

	//--- build RT rect ---
	_VERTEX_PNCT4T4 vul, vur, vdl, vdr;
	vul.pos = Vec3( 0.0f, 0.0f, 0.0f );
	vur.pos = Vec3( (float)pRTcolor->nWidth, 0.0f, 0.0f );
	vdl.pos = Vec3( 0.0f, (float)pRTcolor->nHeight, 0.0f );
	vdr.pos = Vec3( (float)pRTcolor->nWidth, (float)pRTcolor->nHeight, 0.0f );

	vul.tex1 = vul.tex2 = Vec4( 0.0f, 0.0f, 0.0f, 0.0f );
	vur.tex1 = vur.tex2 = Vec4( 1.0f, 0.0f, 0.0f, 0.0f );
	vdl.tex1 = vdl.tex2 = Vec4( 0.0f, 1.0f, 0.0f, 0.0f );
	vdr.tex1 = vdr.tex2 = Vec4( 1.0f, 1.0f, 0.0f, 0.0f );
	//set color
	vul.color = vur.color = vdl.color = vdr.color = 0xffffffff;
	//build verts
	_VERTEX_PNCT4T4 lightRectV[6]; //tex2-mapare back buffer, tex1-spot lumina
	lightRectV[0] = vul; lightRectV[1] = vur; lightRectV[2] = vdl;
	lightRectV[3] = vur; lightRectV[4] = vdl; lightRectV[5] = vdr;

	__Shaders().SetVSByName( L"VS_COMPOSITION" );
	__Shaders().SetVertexDeclaration( K_SHM_PNCT4T4 );
	__Shaders().SetVSConstantF( 0, (float*)&matWVP, 4 );

	__Shaders().SetPSByName( L"PS_COMPOSITION" );
	// set Pshader constants
	float fGamma = 2.2f;
	float fConstData[][4] = {
		// x:gamma, y:1.0f/gamma
		{ fGamma, 1.0f / fGamma, ct_fLightMul, ct_fColorDodge}
	};
	__Shaders().SetPSConstantF( 0, (float*)fConstData, ARRAY_SIZE( fConstData ) );
	m_pDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST, 2, &lightRectV, sizeof( _VERTEX_PNCT4T4 ) );
	// remove VS PS
	__Shaders().SetVS( nullptr );
	__Shaders().SetPS( nullptr );

	return K_OP_OK;
}

void CLevel::Paint()
{
	if ( (!m_bLoaded) || (!m_bOneUpdateDone) )
		return;

	m_pDevice->SetTransform( D3DTS_WORLD, &g_matIdentity );

	PaintGameFinalRT();
	PaintOverGameLayer();
	PaintGUILayer();
}

OPRESULT CLevel::PaintGameFinalRT()
{
	RectXYWH rectRender = UTApp().g_rectRenderPP;
	int nPixelScaling = UTApp().g_nPixelSizePP;
	///--- PAINT LEVEL ---
	//paint game 
	CRTManager::CEngineRenderTarget* pRTfinal = __RTManager().GetRTbyUID( K_RTID_FINAL );
	if ( pRTfinal == nullptr )
		return OP_ERR( K_OP_FAILED, K_SEVERITY_WARNING, L"Final RT not available!" );

	RECT src;
	SizeWH szSrc( rectRender.w / (float)nPixelScaling, rectRender.h / (float)nPixelScaling );
	// display the center part of the source RT that fits the screen
	Vec2i vUL( (int)floor( pRTfinal->nWidth / 2.0f - szSrc.w / 2.0f ), (int)floor( pRTfinal->nHeight / 2.0f - szSrc.h / 2.0f ) );
	Vec2i vDR( vUL.x + (int)ceil( szSrc.w ), vUL.y + (int)ceil( szSrc.h ) );
	SetRect( &src, vUL.x, vUL.y, vDR.x, vDR.y );
	//use SRC rect for scaling and not the nPixelScaling.
	float fRTscale = nPixelScaling;
	//#TODO: when using NON PIXEL PERFECT scaling just scale the whole RT (keeping the aspect ratio)
	//float fRTscale = (float)rectRender.h / (float)pRTfinal->nHeight;
	Matrix matpaint;
	// computes sub pixel offsets for smooth scrolling. the RT renders only on tileset pixels, no subpixels, for precision.
	// we remove the clunky camera movement by moving the final RT onscreen with subpixel coordinates
	RectXYWH camrect = m_camLevelToRT.GetCamWorldAABB();
	Vec2 vSubPxOff( -FLOAT_FRAC( camrect.x ) * (fRTscale * K_RT_PIXEL_SIZE_F), -FLOAT_FRAC( camrect.y ) * (fRTscale * K_RT_PIXEL_SIZE_F) );
	///-- Moves the onscreen rectangle with sub-pixecl precision to smooth out the fixed pixel corner rendering
	RectLTRB destRect( rectRender );
	//destRect.Move( vSubPxOff.x, vSubPxOff.y );
	RectLTRB srcUV( vUL.x / pRTfinal->nWidth, vUL.y / pRTfinal->nHeight, vDR.x / pRTfinal->nWidth, vDR.y / pRTfinal->nHeight );

	m_pDevice->SetTexture( 0, pRTfinal->m_pRTTexture );
	UT3D::DrawRectUP_TL1T( m_pDevice, destRect, srcUV, 0xffffffff );

	//#TODO: explosion deforming effects and other stuff

	return K_OP_OK;
}

OPRESULT CLevel::PaintOverGameLayer()
{
	//get camera data
	RectXYWH	camrect = m_camLevelToScr.GetCamWorldAABB();
	Matrix			matCam = m_camLevelToScr.GetViewTransform();
	CAABB		camAABB( camrect );

	__Painter().SetViewTransform( matCam );

	///#TEMP: paint interactible
	for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
	{
		if ( pPlayerActor[kk] == null )
			continue;
		CActor* pPlayer = pPlayerActor[kk];
		if ( pPlayer->pClosestTouchable.IsSet() )
		{
			Vec2 vpos = pPlayer->pClosestTouchable.GetTo()->pos.xy_proj;
			UTSprite::PaintFrame( &m_sprInterface, vpos.x, vpos.y, ANM_IGM_INTERFACE_SPR_INTERACT_ONCE, 0, 0xffffffff );
		}
	}

	///#TEMP: --- paint crosshairs 
	for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
	{
		if ( pPlayerActor[kk] == nullptr )
			continue;

		// paint aiming cursor
		// vAimVec was normalized using last frame data so paint it at last frame actor position
		Vec2 vto = Vec3XY( pPlayerActor[kk]->pos_last ) - Vec2( 0.0f, pPlayerActor[kk]->vHeart.proj_h ) + pPlayerActor[kk]->GetAimVec();
		UTSprite::PaintFrame( &m_sprInterface, vto.x, vto.y, ANM_IGM_INTERFACE_SPR_IGM_STRATEGIC_EFFECTS, 4, 0xffffffff );

	}


	///--- paint string particles in level coords ---
	//__Particles().PaintStringParticles(K_PART_LAYER_NORMAL);

	//--- closest touchable and cover icons ---
	 //pointer to last painted active interface so we don't draw it twice
	IActiveInterface* pLastPaintedTarget = nullptr;
	for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
	{
		if ( (pPlayerActor[kk] == nullptr) || (pPlayerActor[kk]->GetCurrentBehavior() != AI_BEHAVIOR_PLAYER_CONTROL) )
			continue;

		CActor* player = pPlayerActor[kk];
		//touchables
		IActiveInterface* activ = nullptr;
		if ( player->pClosestTouchable.IsSet() )
			activ = player->pClosestTouchable.GetTo();

		if ( (activ) && (activ->bHideInteractIcon == false) &&
			(activ != pLastPaintedTarget) && (player->collisionFlags & K_DIRFLAG_DOWN) )
		{
			//save last painted target
			pLastPaintedTarget = activ;

			Vec2 vpos( activ->bbox.vCenter.x, activ->bbox.vMin.y );
			//too low? don't cover the player
			if ( vpos.y > player->bbox.vMin.y )
				vpos.y = player->bbox.vMin.y;

			//change this constants for analog sticks
			const int ANIM_IDX_INTERACT_ONCE = ANM_IGM_INTERFACE_SPR_INTERACT_ONCE;
			const int ANIM_IDX_INTERACT_KEEP_PRESSED = ANM_IGM_INTERFACE_SPR_INTERACT_KEEP_PRESSED;
			/*
			#ifdef (CONSOLE)
			const int ANIM_IDX_INTERACT_ONCE = ANM_IGM_INTERFACE_SPR_INTERACT_ONCE_ANALOG;
			const int ANIM_IDX_INTERACT_KEEP_PRESSED = ANM_IGM_INTERFACE_SPR_INTERACT_KEEP_PRESSED_ANALOG;
			#endif
			*/


			//player->m_sprOverheadIcon.setAnimationOnce(ANIM_IDX_INTERACT_ONCE);
			//player->m_sprOverheadIcon.pos = vpos;

			/*
			DWORD dwcol = 0xff00c0ff;
			if (g_timers.GetTimerValue(600) < 0.3f)
			dwcol = 0xff0384af;
			g_font5ns2->DrawString(STR_TAP, vpos.x, vpos.y - 14, FONTFLAG_ANCHOR_BOTTOMCENTER, dwcol);
			*/
			//--- paint progress damage bar ---
			/*
			float perc = 1.0f;
			int barlen = 12;
			DWORD dwProgressColor = 0xffffffff;
			//daca am o usa ca closest touchable si daca se poate sparge afisez progress pe ea
			if ((activ->pTarget != null) && (activ->pTarget->AIstate == K_AI_STATE_COLL_BREAKABLE_DOOR))
			{
				perc = activ->pTarget->AIfvar1 / activ->pTarget->AIfvar2;
				barlen = (int)ceil(activ->pTarget->AIfvar2 * 0.2f); //lungime bara damage la usile care se sparg
				dwProgressColor = 0xff00c0ff;
				barlen = (barlen / 2) * 2; //odd length
											//limit progress bar size
				CLAMP(barlen, 6, 20);

				if ((perc > 0.0f) && (perc < 1.0f))
				{
					//RECTXYWH recttemp(vpos.x - barlen / 2.0f, activ->bbox_exported.vMax.y + 4, barlen, 8);
					RectXYWHi recttemp(activ->pTarget->pos.xy_proj.x - barlen / 2.0f, activ->pTarget->bbox.vMax.y + 4, barlen, 8);
					GUIUtils::DrawProgress_HeadsOutside(&UTGetGUI().m_sprCol, ANM_CONTROLS_SPR_PROGRESS_RED_GLOW_HO, recttemp, perc, 0xffffffff);
				}
			}
			*/
			//paint interact icon at the end
			//player->m_sprOverheadIcon.paint(&m_sprInterface);
		}

	}

	__Painter().Flush();
	return K_OP_OK;
}

OPRESULT CLevel::PaintGUILayer()
{
	//get camera data
	RectXYWH	camrect = UTApp().g_camRTScreen.GetCamWorldAABB();
	Matrix			matCam = UTApp().g_camRTScreen.GetViewTransform();
	CAABB		camAABB( camrect );

	__Painter().SetViewTransform( matCam );

	//ingame interface
	//m_interfaceIGM.Paint(m_pDevice, g_pGameSprite);
	//interface particles
	//__Particles().PaintLayer(K_PART_LAYER_INTERFACE_LIGHT, true);

	__Painter().Flush();
	return K_OP_OK;
}

void CLevel::Release()
{
	ClearVisibilityLists();
	// release passability map 
	m_astar.Release();
	m_tilesetDesc.Clear();

	SAFE_DELETE_GROWABLE_ARRAY( m_arrAreas );

	CSmartLink::RemoveAllLinks();
	SAFE_DELETE_GROWABLE_ARRAY( m_arrColShapes );
	SAFE_DELETE_GROWABLE_ARRAY( m_arrLights );
	SAFE_DELETE_GROWABLE_ARRAY( m_arrDecals );
	m_arrActors.Release();
	SAFE_DELETE_GROWABLE_ARRAY( m_arrMiscObjects );

	SAFE_DELETE_GROWABLE_ARRAY( m_arrTemplatesActor );
	SAFE_DELETE_GROWABLE_ARRAY( m_arrAItemplates );
	SAFE_DELETE_GROWABLE_ARRAY( m_arrTemplatesWeapon );
	SAFE_DELETE_GROWABLE_ARRAY( m_arrTemplatesExplosion );

	SAFE_DELETE_GROWABLE_ARRAY( m_arrAIevents );
	m_arrActionTemplates.clear();
	//release bullets
	m_poolBullets.Release();
	m_poolDoofers.Release();
	//m_poolPhysPts.Release();

	m_sprLib.Release();
	m_sprInterface.Release();

	m_texManager.Release();
	//reset player list
	m_nPlayers = 0;
	m_nPlayersActive = 0;
	for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
	{
		pPlayerActor[kk] = nullptr;
	}

	m_interfaceIGM.Release();
	//m_interfaceTextBubble.Release();

	__Particles().ClearParticles();

	if ( m_bLoaded )
	{
		LOG( L"Level Released." );
	}

	m_bLoaded = false;
	m_bOneUpdateDone = false;
}



int CLevel::BuildLightVolume360( CLight* light, _VERTEX_PNCT4T4* outVerts, int outVertsMaxCnt )
{
	_ASSERT( outVerts != null );
	// sends 360 rays and finds collisions with tileset base of walls. When we collide with a wall facing the camera we also add polys for the wall.
	// could be optimized: if we collide with same tile then we just move last point instead of adding another.

	struct sCollPoint {
		Vec2 vPos;
		Vec2 vNorm;
		Vec2i tlPos;
		bool bCollided{};
	};

	const int	nSteps = 360;
	int			nVertCnt = 0;
	// collisions array
	sCollPoint	arrColl[nSteps];
	int			arrCollCur = 0;

	float		fAngStep = DOUBLE_PI / (float)nSteps;
	float		fAng = 0.0f;

	const CAABB bbox_ini = light->bbox.GetSnapshot();
	float fMaxRad = max( bbox_ini.vHalfSize.x, bbox_ini.vHalfSize.y );
	Vec2 vFrom = light->pos.xy;
	Vec3 vFrom3 = Vec2ToVec3XY0( vFrom );
	// collision results
	Vec2 vRetPt( 0.0f, 0.0f ), vRetNrm( 0.0f, 0.0f );
	Vec2i tilePosTL;
	// counts how many collisions of the same type (no collision or same tile) were made in order
	int nSameSince = 0;

	for ( int kk = 0; kk < nSteps; kk++ )
	{
		Vec2 vdir( cos( fAng ), sin( fAng ) );
		Vec2 vTo = vFrom + vdir * fMaxRad;
		CTile* pColTile = SegmentTilesIntersection( vFrom, vTo, vRetPt, vRetNrm, &tilePosTL );
		if ( pColTile )
		{
			// are we still on the same tile, same kind of collision? take a step back and overwrite last value

			if ( (arrCollCur == 0) || ((tilePosTL == arrColl[arrCollCur - 1].tlPos) && (vRetNrm == arrColl[arrCollCur - 1].vNorm)) )
				nSameSince++;
			else
				nSameSince = 0;
			// make sure we use the first different collision (from nothing to wall for example) so it doesn't cut corners
			if ( nSameSince > 1 )
				arrCollCur--;

			// fix wiggling corners (snap to tile corners when colliding visible wall)
			float chkX1 = tilePosTL.x * K_TILE_SIZE_F, chkX2 = chkX1 + K_TILE_SIZE_F;
			if ( vRetNrm.y > 0.0f )
			{
				if ( fabs( vRetPt.x - chkX1 ) <= 1.5f )
					vRetPt.x = chkX1;
				else if ( fabs( vRetPt.x - chkX2 ) <= 1.5f )
					vRetPt.x = chkX2;
			}

			arrColl[arrCollCur].bCollided = true;
			arrColl[arrCollCur].vPos = vRetPt;
			arrColl[arrCollCur].vNorm = vRetNrm;
			arrColl[arrCollCur].tlPos = tilePosTL;
			arrCollCur++;
		}
		else
		{
			// optimizes so it just adds one triangle every N collisions
			/*
			nSameSince++;
			if (nSameSince > 5)
				nSameSince = 0;
			if (nSameSince > 1)
				arrCollCur--;
			  */
			  // add end of ray
			arrColl[arrCollCur].bCollided = false;
			arrColl[arrCollCur].vPos = vTo;
			arrColl[arrCollCur].vNorm = Vec2( 0.0f, 0.0f );
			arrColl[arrCollCur].tlPos = Vec2i( -1, -1 );
			arrCollCur++;
		}
		// increase angle
		fAng += fAngStep;
	}

	// create triangles (skip first point, will be handled last)
	for ( int kk = 1; kk <= arrCollCur; kk++ )
	{
		_ASSERT( nVertCnt < outVertsMaxCnt );

		sCollPoint* pt = &arrColl[kk % arrCollCur];
		sCollPoint* ptold = &arrColl[(kk - 1) % arrCollCur];

		Vec3 ptpos( pt->vPos.x, pt->vPos.y, 0.0f );
		Vec3 ptoldpos( ptold->vPos.x, ptold->vPos.y, 0.0f );

		outVerts[nVertCnt].pos = vFrom3; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;
		outVerts[nVertCnt].pos = ptpos; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;
		outVerts[nVertCnt].pos = ptoldpos; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;

		// extend on wall
		if ( (pt->bCollided) && (ptold->bCollided) && (pt->tlPos.y == ptold->tlPos.y) &&
			(pt->vNorm.y >= 1.0f) && (ptold->vNorm.y >= 1.0f) )
		{
			// add 2 tris per wall segment
			Vec3 vWallH( 0.0f, -K_WALL_HEIGHT_SCREEN, 0.0f );

			outVerts[nVertCnt].pos = ptpos; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;
			outVerts[nVertCnt].pos = ptpos + vWallH; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;
			outVerts[nVertCnt].pos = ptoldpos; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;

			outVerts[nVertCnt].pos = ptpos + vWallH; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;
			outVerts[nVertCnt].pos = ptoldpos; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;
			outVerts[nVertCnt].pos = ptoldpos + vWallH; outVerts[nVertCnt].color = 0xffff00ff; nVertCnt++;
		}
	}

	return nVertCnt;
}



void CLevel::InitializeStrategicAbilities( int nPlayerOrdinal )
{
	if ( (nPlayerOrdinal < 0) || (nPlayerOrdinal >= K_MAX_PLAYERS_CNT) )
	{
		ErrorBox( K_ERR_WARNING, L"InitializeStrategicAbilities: invalid playerOrdinal!" );
		return;
	}
	//reset selction on NONE
	m_arrPlayerSelStrategic[nPlayerOrdinal] = -1;

	m_arrStrategicAbilities[nPlayerOrdinal][0] = -1;
	m_arrStrategicAbilitiesNames[nPlayerOrdinal][0] = -1;

	m_arrStrategicAbilities[nPlayerOrdinal][1] = K_CI_STRATEGIC_BODY_ARMOR;
	m_arrStrategicAbilitiesNames[nPlayerOrdinal][1] = STR_STRATEGIC1;

	m_arrStrategicAbilities[nPlayerOrdinal][2] = K_CI_STRATEGIC_GEAR_REFILL;
	m_arrStrategicAbilitiesNames[nPlayerOrdinal][2] = STR_STRATEGIC2;

	m_arrStrategicAbilities[nPlayerOrdinal][3] = -1;
	m_arrStrategicAbilitiesNames[nPlayerOrdinal][3] = -1;

	m_arrStrategicAbilities[nPlayerOrdinal][4] = K_CI_STRATEGIC_MEDIKIT;
	m_arrStrategicAbilitiesNames[nPlayerOrdinal][4] = STR_STRATEGIC3;

	m_arrStrategicAbilities[nPlayerOrdinal][5] = K_CI_STRATEGIC_REINFORCEMENT;
	m_arrStrategicAbilitiesNames[nPlayerOrdinal][5] = STR_STRATEGIC4;

	m_arrStrategicAbilities[nPlayerOrdinal][6] = K_CI_STRATEGIC_EXTRA_LIFE;
	m_arrStrategicAbilitiesNames[nPlayerOrdinal][6] = STR_STRATEGIC5;
	//ultima abilitate se ia din ecranul de player select screen
	eStrategicAbility eRetAbility = K_CI_STRATEGIC_NONE;
	int nAbilityStringIdx = -1;
	g_playerSelScr.GetUltimateAbility( &g_playerSelScr.m_arrPlayers[nPlayerOrdinal], eRetAbility, nAbilityStringIdx );
	//si se salveaza in array-ul corespunzator
	m_arrStrategicAbilities[nPlayerOrdinal][7] = (int)eRetAbility;
	m_arrStrategicAbilitiesNames[nPlayerOrdinal][7] = nAbilityStringIdx;
}

void CLevel::ResetLevelStatistics()
{
	for ( int kk = 0; kk < K_LVL_STATS_CNT; kk++ )
	{
		m_arrStats[kk] = 0;
	}

	//m_interfaceIGM.SetStrategicPoints(0.0f, 0.0f);
	//m_interfaceIGM.SetLivesLeft(m_arrStats[K_LVL_STATS_PL1_LIVES], m_arrStats[K_LVL_STATS_PL2_LIVES]);
}

void CLevel::IncreaseLevelStatistics( int K_LVL_STATS_n, int nValueToAdd /*= 1*/ )
{
	if ( (K_LVL_STATS_n < 0) || (K_LVL_STATS_n >= K_LVL_STATS_CNT) )
	{
		ErrorBox( K_ERR_WARNING, L"Illegal Level Stat IDX!" );
		return;
	}

	m_arrStats[K_LVL_STATS_n] += nValueToAdd;

	//#ACHIEVEMENTS: check level achievements
	switch ( K_LVL_STATS_n )
	{
		case K_LVL_STATS_PL1_USE_EXTRA_LIFE_CNT:
		{
			if ( (m_arrStats[K_LVL_STATS_n] >= 5) &&
				(pPlayerActor[0] != null) &&
				(!IsNetworkPlayer( pPlayerActor[0] )) )
			{
				__Achievements().UnlockAchievement( ACH_TERMINATOR );
			}
		}
		break;
		case K_LVL_STATS_PL2_USE_EXTRA_LIFE_CNT:
		{
			if ( (m_arrStats[K_LVL_STATS_n] >= 5) &&
				(pPlayerActor[1] != null) &&
				(!IsNetworkPlayer( pPlayerActor[1] )) )
			{
				__Achievements().UnlockAchievement( ACH_TERMINATOR );
			}
		}
		break;
		case K_LVL_STATS_PL1_RESURRECT_PEER_CNT:
		{
			if ( (m_arrStats[K_LVL_STATS_n] >= 5) &&
				(pPlayerActor[0] != null) &&
				(!IsNetworkPlayer( pPlayerActor[0] )) )
			{
				__Achievements().UnlockAchievement( ACH_STAY_WITH_ME );
			}
		}
		break;
		case K_LVL_STATS_PL2_RESURRECT_PEER_CNT:
		{
			if ( (m_arrStats[K_LVL_STATS_n] >= 5) &&
				(pPlayerActor[1] != null) &&
				(!IsNetworkPlayer( pPlayerActor[1] )) )
			{
				__Achievements().UnlockAchievement( ACH_STAY_WITH_ME );
			}
		}
		break;
	}
}

bool CLevel::ActivateSpecialAbility( int nAbilityIdx, int nTargetPlayerOrdinal )
{
	return false;
}

void CLevel::GiveStrategicPoints( float fPoints, Vec2* vPos )
{
	if ( fPoints <= 0.0f )
		return;
	//on single player multiply the points
	float fMultiplier = 1.0f;
	//change multiplier based on XP bars
	//take the first player (always present)
	float fFilled = g_playerSelScr.GetUpgradeBarPercent( &g_playerSelScr.m_arrPlayers[0], L"TEAM_LOGISTICS" );
	fMultiplier += fFilled * 0.5f;
	//double XP points on single player
	if ( m_nPlayers == 1 )
		fMultiplier *= 2.0f;
	//just making sure...
	_ASSERT( (fPoints >= 0.0f) && (fPoints <= (float)K_LVL_MAX_STRATEGIC_POINTS) );
	float fPointsGiven = LIMIT( fPoints, 0.0f, (float)K_LVL_MAX_STRATEGIC_POINTS );

	for ( int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++ )
	{
		if ( pPlayerActor[kk] == null )
			continue;

		int fMaxPoints = K_LVL_MAX_STRATEGIC_POINTS;

		int nStatIdx = K_LVL_STATS_PL1_STRATEGIC_POINTS + pPlayerActor[kk]->nPlayerOrdinal * K_LVL_STATS_PLAYER_STATS_COUNT;
		int nAdder = (int)floor( fMultiplier * fPointsGiven * 1000.0f );
		m_arrStats[nStatIdx] += nAdder;
		CLAMP( m_arrStats[nStatIdx], 0, fMaxPoints * 1000 );
	}

	//m_interfaceIGM.SetStrategicPoints(m_arrStats[K_LVL_STATS_PL1_STRATEGIC_POINTS] / 1000.0f, m_arrStats[K_LVL_STATS_PL2_STRATEGIC_POINTS] / 1000.0f);

	//add text particle (visuals)
	/*
	if ((vPos != null) && (fPointsGiven > 0.0f))
	{
		WCHAR strPart[MAX_PATH];
		float fVal = fMultiplier * fPointsGiven;
		if (FLOAT_FRAC(fVal) > 0.1f)
			StringCchPrintf(strPart, MAX_PATH, L"+%.1f SP", fVal);
		else
			StringCchPrintf(strPart, MAX_PATH, L"+%d SP", (int)fVal);

		__Particles().AddStringParticle(g_font5ns2, strPart, vPos, NULL, &Vec2(0.0f, -20.0f), 1.2f, 1.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.4f, 0xcc21aec2, K_PART_LAYER_NORMAL);
	}
	  */
}


bool CLevel::IsLineOfSight( Vec2 pt_from, Vec2 pt_to, CLevelArea* pStartArea )
{
	//#TODO: de verificat daca pleaca direct din interiorul unei coliziuni

	Vec2 collisionPoint, collisionNormal;
	CTile* tl = SegmentTilesIntersectionEx( pt_from, pt_to, collisionPoint, collisionNormal, nullptr, pStartArea );

	return (tl == nullptr) ? true : false;
	//#TODO: add intersection with shapes contained in pt1 pt2 bbox
	/*
	CCollisionShape* colShape = ColShape_Segment_Intersection_Arr( pt1, pt2, m_visibleList.logic_colShapesExtended.m_pData, m_visibleList.logic_colShapesExtended.Count(), retVecCollisionPt, retVecCollisionNormal );
	if ( colShape != nullptr )
	{
		return false;
	}
	return true;
	*/
}



///--- DECALS ---
void CLevel::AddDecal( EDecalLayer nLayer, Vec2 pos, int animIdx, int frameIdx /*= 0*/, DWORD color /*= 0xffffffff*/, bool bIsAnimated /*= false*/ )
{
	/*
	CDecal *ndec = new CDecal();

	ndec->layer = nLayer;
	ndec->sprite.Init( animIdx, ( int ) pos.x, ( int ) pos.y, frameIdx, color );
	RectLTRB framerect = m_sprProps.GetAFrameBBox_real( animIdx, frameIdx );
	ndec->aabb.Set( Vec2( framerect.left + pos.x, framerect.top + pos.y ), Vec2( framerect.right + pos.x, framerect.bottom + pos.y ) );
	ndec->bAnimated = bIsAnimated;

	m_arrDecals.Add( ndec );
	*/
}


void CLevel::UpdateDecals( float dTime )
{
	//#TODO: maybe remove animated decals and move decals to each area
	/*
	//Update less often
	for ( int kk = 0; kk < m_arrDecals.GetSize(); kk++ )
	{
		if ( m_arrDecals[kk]->bAnimated )
		{
			m_arrDecals[kk]->sprite.Update( &m_sprProps, dTime );
			//remove animation flag when anim ends
			if ( m_arrDecals[kk]->sprite.animStatus == ANIM_STATUS_FRAMELOCK )
				m_arrDecals[kk]->bAnimated = false;
		}
	}
	*/
}

/*
void CLevel::UpdatePhysicsPoints( float dTime )
{
	for(auto node : m_poolPhysPts)
	{
		//update
		CPhysicsPoint*	point = &node->m_data;
		// kill it when it gets outside the play area
		if ( !Rects::PointInRect( Vec3XY( point->pos ), m_levelAABB ) )
		{
			point->bIsDead = true;
			point->bIsStatic = true;
		}

		// what forces act on the point
		Vec3			vecForces = point->accel;
		bool			bWasContacting = point->bContacting;

		point->contactType = K_COLLTYPE_NONE;
		point->bContacting = false;
		point->pContactShape = nullptr;
		point->bContactStarted = false;
		//save last pos
		point->pos_last = point->pos;

		///--- integrator
		//integrator
		if ( point->bIsStatic )
			vecForces = g_Vec3Zero;
		if ( point->bIsStaticZ )
			vecForces.z = 0.0f;

		point->speed += vecForces * dTime;
		point->pos += point->speed * dTime;


		///--- check collisions
		{
			Vec2 collisionPoint, collisionNormal;
			Vec2 vFrom = Vec3XY( point->pos_last );
			Vec2 vTo = Vec3XY( point->pos );
			Vec2 vMove = vTo - vFrom;

			eRetContactType contactT = K_COLLTYPE_NONE;
			CCollisionShape* colShape = nullptr;

			// XY plane collision
			bool bCollided = false;
			float fMinContactDistance = 100000.0f;
			if ( ( vMove.x != 0.0f ) && ( vMove.y != 0.0f ) )
			{
				// tiles collision
				if ( point->nFlagsCollision & K_LVL_PHYSP_COLLFLAG_TILES )
				{
					Vec2i ptHitTilePos( 0, 0 );
					CTile* pColTile = SegmentTilesIntersectionEx( vFrom, vTo, collisionPoint, collisionNormal, &ptHitTilePos, point->pArea );
					if ( pColTile )
					{
						bCollided = true;
						contactT = K_COLLTYPE_TILE;
						fMinContactDistance = MUVec2Len( &( vFrom - collisionPoint ) );
					}
				}

				// collision with shapes
				if ( point->nFlagsCollision & K_LVL_PHYSP_COLLFLAG_BOXES )
				{
					// (overwrite collpoint ONLY if closer and make other optimizations to see if we CAN collide with anything)
				// get only the boxes in vMove box
				//colShape = ColShape_Segment_Intersection_Arr(point->pos_last, point->pos, m_visibleList.logic_colShapes.m_pData, m_visibleList.logic_colShapes.Count(), &collisionPoint, &collisionNormal);
				}
			}

			if ( bCollided )
			{
				point->pos.x = collisionPoint.x + collisionNormal.x;
				point->pos.y = collisionPoint.y + collisionNormal.y;

				point->bContacting = true;
				point->pContactShape = colShape;
				point->contactNormal = Vec2ToVec3XY0( collisionNormal );
				point->contactPos = Vec3( collisionPoint.x, collisionPoint.y, point->pos.z );
				point->contactType = contactT;

				if ( point->bFlagPhysicsEnabled )
				{
					float fDot = MUVec3Dot( &point->speed, &point->contactNormal );
					Vec3 Vn = point->contactNormal * fDot;
					Vec3 Vt = point->speed - Vn;
					// compute final speed
					point->speed = -Vn + Vt;
				}
				else
				{
					point->speed = g_Vec3Zero;
				}
			}

			// Minimum speed on Z when we consider the point stopped
			const float fMinSpeedZ = 0.1f;
			// Current floor height. #MAYBE: should get it from each tile
			float fFloorH = 0.0f;

			// Z floor collision at the end to bring it back up
			// Only compute this part if we have vertical acceleration and speed
			//#MAYBE: should check collision with ceiling too
			if ( ( point->accel.z != 0.0f ) && ( point->speed.z != 0.0f ) && ( point->pos.z <= fFloorH ) )
			{
				point->bContacting = true;
				// walls collisions have priority so only set normals if no other collision happened
				if ( !bCollided )
				{
					point->contactNormal = Vec3( 0.0f, 0.0f, -1.0f );
					point->contactPos = point->pos;
					point->pContactShape = nullptr;
					point->contactType = K_COLLTYPE_FLOOR;
				}
				// get the point back above the floor
				point->pos.z = fFloorH - point->pos.z;

				if ( point->bFlagPhysicsEnabled )
				{
					// make sure it always ricochets upwards
					point->speed.z = fabs( point->speed.z * point->fBounceF );

					if ( fabs( point->speed.z * dTime ) < fMinSpeedZ )
					{
						point->speed.z = 0.0f;
						point->pos.z = fFloorH;
						point->bIsStaticZ = true;
					}
					// apply friction
					point->speed.x -= point->speed.x * point->fFrictionF * dTime;
					point->speed.y -= point->speed.y * point->fFrictionF * dTime;
				}
				else
				{
					point->speed = g_Vec3Zero;
				}
			}

			//check bounce or first contact - mainly for sounds and particles
			if ( bWasContacting == false )
			{
				point->bContactStarted = true;
			}

			//update current area (change only if not static)
			Vec2 vpos2d = Vec3XY( point->pos );
			if ( ( point->pArea == nullptr ) || ( !point->pArea->AABBbounds.PointIn( vpos2d ) ) )
			{
				point->pArea = Areas_GetAt( vpos2d );
			}

			// is it almost stopped?
			if ( UTMath::Vec3AlmostZero( point->speed * dTime, 0.5f ) )
			{
				point->bIsStatic = true;
				point->speed = g_Vec3Zero;
			}
			else
			{
				point->bIsStatic = false;
			}
		}
	}
}
*/


void CLevel::GenerateEffect( ELVLEffectType nEffectType, Vec2 pos, float fSize, DWORD color )
{
	switch ( nEffectType )
	{
		case K_FX_EXPLONICE_SM1:
		{
			//not nice ring: __Particles().AddParticle( ANM_PARTICLES_SPR_DUST_RINGS, false, 0, &pos, nullptr, nullptr, 0.4f, 1.0f, 6.0f, 0.0f, 0.0f, 0.02f, 0.2f, 0x66ffffff, K_PART_LAYER_NORMAL );
			for ( int kk = 0; kk < 6; kk++ )
			{
				__Particles().AddParticle( ANM_PARTICLES_SPR_SMOKESWIRL1, true, randint( 2 ), &(pos + randVec2sgn( 12.0f, 12.0f )), nullptr, nullptr, 5.0f, 1.0f + randfloat( 0.2f ), 0.0f,
					randfloat( DOUBLE_PI ), 0.0f, 0.0f, 0.0f, 0xaa2a2626, K_PART_LAYER_NORMAL );
			}
			__Particles().AddParticle( ANM_PARTICLES_SPR_EXPLONICE_SM1, true, 0, &pos, nullptr, nullptr, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_NORMAL );
		}
		break;
		case K_FX_EXPLONICE_BIG1:
		{
			for ( int kk = 0; kk < 6; kk++ )
			{
				__Particles().AddParticle( ANM_PARTICLES_SPR_SMOKESWIRL1, true, randint( 2 ), &(pos + randVec2sgn( 16.0f, 16.0f )), nullptr, nullptr, 5.0f, 1.2f + randfloat( 0.4f ), 0.0f,
					randfloat( DOUBLE_PI ), 0.0f, 0.0f, 0.0f, 0xaa2a2626, K_PART_LAYER_NORMAL );
			}
			__Particles().AddParticle( ANM_PARTICLES_SPR_EXPLONICE_BIG1, true, 0, &pos, nullptr, nullptr, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_NORMAL );
		}
		break;
		case K_FX_STONE_BREAK:
		{
			//__Particles().GenerateSmokePuff(Vec2(pos.x, pos.y - 10.0f), 20.0f, K_PART_LAYER_RT_FRONT_NRM);
			m_camLevelToRT.ShakeScreen( 2.0f, 8.0f, &pos );

			//			SND_PLAY_POSITIONAL(SNDIDX_STONE_BREAK1, pos);
		}
		break;
		case K_FX_EXPLO_LARGE:
		{
			AddDoofer_Explo( hash_EXPLO_LARGE_XL, pos, 0, K_ACT_CLASS_EXPLOSION );
		}
		break;
		case K_FX_ELECTRIC_BREAK_SPARKS:
		{
			//			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.4f, 0.1f, 0x88FDB727, 1.0f);
						//particule sparkle
						/*
						for (int kk = 0; kk < 20; kk++)
						{
							__Particles().AddParticle(ANM_PARTICLES_SPR_FIRESPARK2, true, randint(2), &Vec2(pos.x + randfloatsgn(fSize), pos.y + randfloatsgn(fSize)), &g_vecGravityOld, &Vec2(randfloatsgn(60.0f), -10.0f - randfloat(40.0f)), 0.2f + randfloat(0.4f), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM_LIGHT, 1.5f, kk * 0.025f);
						}
						*/
		}
		break;
		default:
			ErrorBox( K_ERR_WARNING, L"CLevel::GenerateEffect - Unknown effect!" );
			break;
	}
}

void CLevel::GenerateEffect( CStringHash& sEffectName, Vec2 pos, float fSize, DWORD color )
{
	ELVLEffectType effectidx = (ELVLEffectType)GetListIndexByNameHash( sEffectName.textHash, ELVLEffectTypeNames, ARRAY_SIZE( ELVLEffectTypeNames ) );
	GenerateEffect( effectidx, pos, fSize, color );
}

void CLevel::GC()
{
	// called just once in a while, maybe every 2 seconds
	///--- release dead objects all at once here ---
	CleanupDeadObjects();
}

void CLevel::TouchClosestActive( CActor* pToucherAct, float dTime )
{
	_ASSERT( pToucherAct != nullptr );

	if ( pToucherAct->pClosestTouchable.IsSet() )
	{
		pToucherAct->pClosestTouchable.GetTo()->Touch( pToucherAct->GetUID(), dTime );
	}
}

#define K_CLIP_OCCLUDERS_TO_LIGHT
int CLevel::GetOccluderSegments( Vec2 vEye, CAABB bbox, COccluderSegment* pRetArr, int maxRetArrSize )
{
	_ASSERT( pRetArr != nullptr && maxRetArrSize > 0 );

	// array to hold the tiles snapshots (linear matrix).
	CTile* arrTilesSnapshot[64 * 64];
	// clear it on every run
	memset( arrTilesSnapshot, null, sizeof( CTile* ) * ARRAY_SIZE( arrTilesSnapshot ) );

	int nCur = 0;

	Vec2 vPos = vEye;
	Vec2 vNYp( 0.0f, 1.0f ), vNYn( 0.0f, -1.0f ), vNXp( 1.0f, 0.0f ), vNXn( -1.0f, 0.0f );
	///#TODO: --- add segments from bboxes (doors mainly)
	//check only the object occluders in the visible area as we don't process lights outside the screen
	/*
	for ( int kk = 0; kk < m_visibleList.visible_colShapesLights.Count(); kk++ )
	{
		_ASSERT( nCur < maxRetArrSize - 2 );
		// if we want to clip occluders to light bbox:
#ifdef K_CLIP_OCCLUDERS_TO_LIGHT
		CAABB retbb;
		CAABB* chkbb = &retbb;
		// clipped check (looks better with longer occluders):
		if ( AABB::Intersection( bbox, m_visibleList.visible_colShapesLights.m_pData[kk]->bbox, retbb ) )
#else
		// if we want the whole bbox:
		CAABB* chkbb = &m_visibleList.visible_colShapesLights.m_pData[kk]->bbox;
		// non clipped check (faster):
		if ( bbox.Intersects( *chkbb ) )
#endif
		{
			if ( vEye.y > chkbb->vMax.y )
			{
				pRetArr[nCur++].Set( Vec2( chkbb->vMin.x, chkbb->vMax.y ), chkbb->vMax, vNYp, vPos, m_visibleList.visible_colShapesLights[kk]->ID, K_WALL_HEIGHT_SCREEN );
			}
			else if ( vEye.y < chkbb->vMin.y )
			{
				pRetArr[nCur++].Set( Vec2( chkbb->vMax.x, chkbb->vMin.y ), chkbb->vMin, vNYn, vPos );
			}

			if ( vEye.x > chkbb->vMax.x )
			{
				pRetArr[nCur++].Set( chkbb->vMax, Vec2( chkbb->vMax.x, chkbb->vMin.y ), vNXp, vPos );
			}
			else if ( vEye.x < chkbb->vMin.x )
			{
				pRetArr[nCur++].Set( chkbb->vMin, Vec2( chkbb->vMin.x, chkbb->vMax.y ), vNXn, vPos );
			}
		}
	}
	*/

	///--- add occluders from tiles, optimizing for same wall lines
	Vec2i tlmin( floor( bbox.vMin.x / K_TILE_SIZE_F ), floor( bbox.vMin.y / K_TILE_SIZE_F ) );
	Vec2i tlmax( floor( bbox.vMax.x / K_TILE_SIZE_F ), floor( bbox.vMax.y / K_TILE_SIZE_F ) );
	// limit to current level aabb in tiles
	RectXYWHi lightAABB_TL( tlmin.x, tlmin.y, tlmax.x - tlmin.x + 1, tlmax.y - tlmin.y + 1 );

	// I didn't optimize the lightAABB by intersecting it with the level bbox because when outside the level it would clip and write to arrTilesSnapshot[] 
	// but then when reading them back it would appear that they are higher because the null tiles don't get written
	//leave commented: lightAABB_TL.IntersectWith( m_levelAABB_TL );
	// tiles are returned in the arrTilesetSnapshot as a matrix in linear form, 0 base index (vector[xx + yy * lightAABB_TL.w])
	Areas_GetTilesSnapshot( lightAABB_TL, arrTilesSnapshot, ARRAY_SIZE( arrTilesSnapshot ) );

	// in order to optimize and transform many segments into a single one, we do the horizontal walls first, then the vertical ones in another for
	for ( int yy = tlmin.y; yy <= tlmax.y; yy++ )
	{
		for ( int xx = tlmin.x; xx <= tlmax.x; xx++ )
		{
			_ASSERT( nCur < maxRetArrSize - 2 );

			CTile* tl = arrTilesSnapshot[xx - tlmin.x + (yy - tlmin.y) * lightAABB_TL.w];
			if ( tl == nullptr )
				continue;
			// can the tile cast shadows?
			if ( FLAG_NONE( tl->flags, K_TILEFLAG_HASWALL_MASK ) )
				continue;

			CAABB chkbb{ xx * K_TILE_SIZE_F, yy * K_TILE_SIZE_F, (xx + 1) * K_TILE_SIZE_F, (yy + 1) * K_TILE_SIZE_F };

			// clip occluders horizontally, do it in a fast way just so we don't miss wall intersections when colliders go outside the light bbox
			if ( chkbb.vMax.x > bbox.vMax.x ) chkbb.vMax.x = bbox.vMax.x;
			if ( chkbb.vMin.x < bbox.vMin.x ) chkbb.vMin.x = bbox.vMin.x;

			DWORD wall_id = yy + 100;
			if ( (tl->flags & K_TILEFLAG_HASWALL_D) && (vPos.y > chkbb.vMax.y) )
			{
				//optimize same wall: check last wall and if it's the same just make the occluder longer
				if ( (nCur > 0) && ((int)pRetArr[nCur - 1].dwWallID == wall_id) && (pRetArr[nCur - 1].vEnd.x == chkbb.vMin.x) )
					pRetArr[nCur - 1].MoveEnd( chkbb.vMax, vPos );
				else
					/*ID is wall Y in tileset plus a value to not collide with the collbox ids */
					pRetArr[nCur++].Set( Vec2( chkbb.vMin.x, chkbb.vMax.y ), chkbb.vMax, vNYp, vPos, wall_id, K_WALL_HEIGHT_SCREEN );
			}
			else if ( (tl->flags & K_TILEFLAG_HASWALL_U) && (vPos.y < chkbb.vMin.y) )
			{
				//optimize same wall: check last wall and if it's the same just make the occluder longer
				if ( (nCur > 0) && ((int)pRetArr[nCur - 1].dwWallID == wall_id) && (pRetArr[nCur - 1].vStart.x == chkbb.vMin.x) )
					pRetArr[nCur - 1].MoveStart( Vec2( chkbb.vMax.x, chkbb.vMin.y ), vPos );
				else
					pRetArr[nCur++].Set( Vec2( chkbb.vMax.x, chkbb.vMin.y ), chkbb.vMin, vNYn, vPos, wall_id, 0.0f );
			}
			/*
			// uncomment this area to add back the many vertical occluders (but comment out the next big for)
			if ( ( tl->flags & K_TILEFLAG_HASWALL_R ) && ( vPos.x > chkbb.vMax.x ) )
			{
				pRetArr[nCur++].Set( chkbb.vMax, Vec2( chkbb.vMax.x, chkbb.vMin.y ), vNXp, vPos );
			}
			else if ( ( tl->flags & K_TILEFLAG_HASWALL_L ) && ( vPos.x < chkbb.vMin.x ) )
			{
				pRetArr[nCur++].Set( chkbb.vMin, Vec2( chkbb.vMin.x, chkbb.vMax.y ), vNXn, vPos );
			}
			*/
		}
	}

	for ( int xx = tlmin.x; xx <= tlmax.x; xx++ )
	{
		for ( int yy = tlmin.y; yy <= tlmax.y; yy++ )
		{
			_ASSERT( nCur < maxRetArrSize - 2 );

			CTile* tl = arrTilesSnapshot[xx - tlmin.x + (yy - tlmin.y) * lightAABB_TL.w];
			if ( tl == nullptr )
				continue;
			// can the tile cast shadows
			if ( FLAG_NONE( tl->flags, K_TILEFLAG_HASWALL_MASK ) )
				continue;

			CAABB chkbb{ xx * K_TILE_SIZE_F, yy * K_TILE_SIZE_F, (xx + 1) * K_TILE_SIZE_F, (yy + 1) * K_TILE_SIZE_F };

			// clip occluders to light rect
			if ( chkbb.vMax.y > bbox.vMax.y ) chkbb.vMax.y = bbox.vMax.y;
			if ( chkbb.vMin.y < bbox.vMin.y ) chkbb.vMin.y = bbox.vMin.y;

			DWORD wall_id = xx + 10000;
			if ( (tl->flags & K_TILEFLAG_HASWALL_R) && (vPos.x > chkbb.vMax.x) )
			{
				if ( (nCur > 0) && ((int)pRetArr[nCur - 1].dwWallID == wall_id) && (pRetArr[nCur - 1].vStart.y == chkbb.vMin.y) )
					pRetArr[nCur - 1].MoveStart( chkbb.vMax, vPos );
				else
					pRetArr[nCur++].Set( chkbb.vMax, Vec2( chkbb.vMax.x, chkbb.vMin.y ), vNXp, vPos, wall_id, 0.0f );
			}
			else if ( (tl->flags & K_TILEFLAG_HASWALL_L) && (vPos.x < chkbb.vMin.x) )
			{
				if ( (nCur > 0) && ((int)pRetArr[nCur - 1].dwWallID == wall_id) && (pRetArr[nCur - 1].vEnd.y == chkbb.vMin.y) )
					pRetArr[nCur - 1].MoveEnd( Vec2( chkbb.vMin.x, chkbb.vMax.y ), vPos );
				else
					pRetArr[nCur++].Set( chkbb.vMin, Vec2( chkbb.vMin.x, chkbb.vMax.y ), vNXn, vPos, wall_id, 0.0f );
			}
		}
	}

	///--- add light range segments (don't set normals so we don't extend the walls on it)
	// add them last so we prioritize intersecting with the others first
	_ASSERT( nCur < maxRetArrSize - 4 );
	pRetArr[nCur++].Set( bbox.vMin, Vec2( bbox.vMax.x, bbox.vMin.y ), vNYp, vPos, 1 );
	pRetArr[nCur++].Set( Vec2( bbox.vMax.x, bbox.vMin.y ), bbox.vMax, vNXn, vPos, 2 );
	pRetArr[nCur++].Set( bbox.vMax, Vec2( bbox.vMin.x, bbox.vMax.y ), vNYn, vPos, 3 );
	pRetArr[nCur++].Set( Vec2( bbox.vMin.x, bbox.vMax.y ), bbox.vMin, vNXp, vPos, 4 );

	return nCur;
}



///--- framework implementations ---
#pragma region FRAMEWORK_IMPL
OPRESULT CLevel::OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc, void* pUserContext )
{
	m_pDevice = pDevice;

	V_OP_RET( m_sprLib.OnCreateDevice( pDevice ) );
	V_OP_RET( m_sprInterface.OnCreateDevice( pDevice ) );
	V_OP_RET( m_texManager.OnCreateDevice( pDevice ) );
	V_OP_RET( m_bufferedPainter.OnCreateDevice( pDevice ) );

	for ( int ii = 0; ii < m_arrAreas.GetSize(); ii++ )
	{
		CLevelArea* area = m_arrAreas[ii];
		V_OP_RET( area->OnCreateDevice( pDevice ) );
	}

	return K_OP_OK;
}

OPRESULT CLevel::OnResetDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc, void* pUserContext )
{
	m_pDevice = pDevice;

	V_OP_RET( m_sprLib.OnResetDevice( pDevice ) );
	V_OP_RET( m_sprInterface.OnResetDevice( pDevice ) );
	V_OP_RET( m_texManager.OnResetDevice( pDevice ) );
	V_OP_RET( m_bufferedPainter.OnResetDevice( pDevice ) );

	for ( int ii = 0; ii < m_arrAreas.GetSize(); ii++ )
	{
		CLevelArea* area = m_arrAreas[ii];
		V_OP_RET( area->OnResetDevice( pDevice ) );
	}

	return K_OP_OK;
}

OPRESULT CLevel::OnLostDevice( void* pUserContext )
{
	m_pDevice = nullptr;

	m_sprLib.OnLostDevice();
	m_sprInterface.OnLostDevice();
	m_texManager.OnLostDevice();

	m_bufferedPainter.OnLostDevice();

	for ( int ii = 0; ii < m_arrAreas.GetSize(); ii++ )
	{
		CLevelArea* area = m_arrAreas[ii];
		V_OP_RET( area->OnLostDevice() );
	}

	return K_OP_OK;
}

OPRESULT CLevel::OnDestroyDevice( void* pUserContext )
{
	m_pDevice = nullptr;

	m_sprLib.OnDestroyDevice();
	m_sprInterface.OnDestroyDevice();
	m_texManager.OnDestroyDevice();

	m_bufferedPainter.OnDestroyDevice();

	for ( int ii = 0; ii < m_arrAreas.GetSize(); ii++ )
	{
		CLevelArea* area = m_arrAreas[ii];
		V_OP_RET( area->OnDestroyDevice() );
	}

	return K_OP_OK;
}

#pragma endregion FRAMEWORK_IMPL


#pragma warning(pop)
