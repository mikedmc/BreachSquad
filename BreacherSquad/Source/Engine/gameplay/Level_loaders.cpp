#include "dxstdafx.h"


OPRESULT CLevel::LoadLevel(WCHAR * strPathAbs)
{
	//set last ID on a number that will never get reached from the editor or by adding areas
	m_unLastID = 10000000;
	int nChapterNumber = g_userData[K_MEMID_SELECTED_CHAPTER];
	int nLevelNumber = g_userData[K_MEMID_SELECTED_LEVEL];
	//--- set loaded level flags
	//are we loading a downloaded level?
	int nModIdx_SelectedContent = g_userData[K_MEMID_MOD_DWNLVL_SELECTED];
	m_unLoadedLevelFlags = K_LVL_LEVEL_FLAG_NONE;
	if (nModIdx_SelectedContent >= 0)
		m_unLoadedLevelFlags |= K_LVL_LEVEL_FLAG_DOWNLOADED;
	if (UTGetAppClass().IsGameModded())
		m_unLoadedLevelFlags |= K_LVL_LEVEL_FLAG_MODS_ON;

	WCHAR Path[MAX_PATH] = { 0 };

	if (UTGetAppClass().IsGameNetworked())
	{
		m_rand.SetRandSeed(g_netlock.m_unRandomSeed);
	}
	else
	{
		//randomize seed
		m_rand.SetRandSeed(GetTickCount());
	}
	//reset local timeline
	fLocalTimeline = 0.0f;
	vLastSpawnPoint = Vec2(0.0f, 0.0f);

	//realease level if loaded
	Release();

	//reset shakes
	m_camLevel.ShakeScreen(0.0f, 0.0f);
	m_levelAABB.Set(0, 0, 0, 0);
	//reset all timers
	m_Timers.ResetTimers();

	m_bLoaded = false;
	m_bOneUpdateDone = false;

	m_dwSyncCheckHash = 0;
	//get rid of all particles
	g_particlesMgr.RemoveAll();
	//--- setari initiale ---
	ResetLevelStatistics();

	m_colAmbientGlobal = 0xffffffff;
	m_fThunderTimer = 0.0f;

	m_waterAnimIdx = -1;

	m_interfaceIGM.Reset();
	m_interfaceIGM.SetBombTimer(-1.0f);
	//load interface sprites
	FileManager::GetMediaPath(L"media/interfaces/igm_interface.bsx", Path);
	V_OP_RET(m_sprInterface.LoadSprites(Path));

	//tileset name
	CHAR charArr[MAX_PATH];
	WCHAR wcharArr[MAX_PATH];
	WCHAR wcsMediaAddr[MAX_PATH];

	FileManager::GetMediaPath(L"media/levels/data/tileset1.png", Path);
	if (FAILED(m_texManager.AddTexture(Path, &m_tilesTexBaseIdx, D3DFMT_A8R8G8B8, D3DX_FILTER_NONE, D3DX_FILTER_NONE)))
	{
		ErrorBox(K_ERR_CRITICAL, L"Couldn't load tileset texture: %s", Path);
		return K_OP_FAILED;
	}

	FileManager::GetMediaPath(L"media/levels/data/tileset1_nh.png", Path);
	if (FAILED(m_texManager.AddTexture(Path, &m_tilesTexNormIdx, D3DFMT_A8R8G8B8, D3DX_FILTER_NONE, D3DX_FILTER_NONE)))
	{
		ErrorBox(K_ERR_CRITICAL, L"Couldn't load tileset normals texture: %s", Path);
		return K_OP_FAILED;
	}
	FileManager::GetMediaPath(L"media/levels/data/water1_n.png", Path);
	if (FAILED(m_texManager.AddTexture(Path, &m_waterTexIdx, D3DFMT_A8R8G8B8, D3DX_FILTER_NONE, D3DX_FILTER_NONE)))
	{
		ErrorBox(K_ERR_CRITICAL, L"Couldn't load water texture: %s", Path);
		return K_OP_FAILED;
	}

	//LIGHTS
	FileManager::GetMediaPath(L"media/levels/data/lights.bsx", Path);
	V_OP_RET(m_sprLights.LoadSprites(Path));

	//load bsx
	FileManager::GetMediaPath(L"media/levels/data/props.bsx", Path);
	V_OP_RET(m_sprProps.LoadSprites(Path));

	///--- load actors ---
	FileManager::GetMediaPath(L"media/levels/data/actors.bsx", Path);
	V_OP_RET(m_sprActors.LoadSprites(Path));

	//--- load actors templates and weaponry right after props sprite ---
	FileManager::GetMediaPath(L"media/levels/data/weapons_data.xml", Path);
	if (FAILED(LoadWeaponTemplates(Path)))
	{
		return K_OP_FAILED;
	}

	//#TODO: release resources on errors (goto ERROR)
											  

	///--- level areas inventory ---
	//loading story
	FileManager::GetMediaPath(L"media/levels/stories/story_small.story", Path);
	V_OP_RET(m_story.LoadStory(Path));

	// build inventory and generate level
	auto arrAreas = UTGetAreasInv().GetAreas();
	UTGetMissionGen().BuildInventory(arrAreas);
	if (!UTGetMissionGen().GenerateLevelFromStory(&m_story))
		return K_OP_FAILED;
	//UTGetMissionGen().GenerateLevelRandomly(3);

	///--- LOAD AREAS:
	for (auto area : UTGetMissionGen().m_arrPlaced)
	{
		WCHAR tmppath[MAX_PATH];
		wsprintf(tmppath, L"media/levels/areas/%s.area", area->strAreaFile.c_str());
		FileManager::GetMediaPath(tmppath, Path);
		V_OP_RET(LoadArea(Path, area->nID, Vec2i(area->AABB.x * K_LGEN_BLOCK_W, area->AABB.y * K_LGEN_BLOCK_H)));
	}

	// set areas neighbour pointers
	for (int ii = 0; ii < m_arrAreas.GetSize(); ii++)
	{
		// order in m_arrAreas SHOULD correspond to the order in m_arrPlaced if area loading didn't fail
		CLevelArea* plarea = m_arrAreas[ii];
		// get the same area description from missions generator and find neighbours
		CPlacedArea* srcarea = UTGetMissionGen().m_arrPlaced[ii];
		for (auto conn : srcarea->arrConnections)
		{
			CLevelArea* neigh = Areas_GetByID(conn.pConnectedArea->nID);
			_ASSERT(neigh != nullptr);
			plarea->arrNeighbours.Add(neigh);
		}
		// enlarge level area and other level data
		m_levelAABB.Union(plarea->AABBbounds.to_RECTXYWH_F());
	}
	// set level aabb in tiles too
	m_levelAABB_TL.Set(floor(m_levelAABB.x / K_TILE_SIZE), floor(m_levelAABB.y / K_TILE_SIZE), (int)(m_levelAABB.w / K_TILE_SIZE), (int)(m_levelAABB.h / K_TILE_SIZE));


	///--- everything loaded, SetAI here again so it sets all necessary pointers ---
	//setez ai-ul la final ca sa execute functiile de initializare cand avem toate array-urile incarcate (ca sa ma asigur ca gaseste target ID-urile)
	for (int kk = 0; kk < m_arrLights.GetSize(); kk++)
	{
		CLight * light = m_arrLights[kk];
		SetAI(light, light->AIstate, &light->varAIparams, light->targetID_ini);
	}
	for (int kk = 0; kk < m_arrColShapes.GetSize(); kk++)
	{
		CCollisionShape * shape = m_arrColShapes[kk];
		SetAI(shape, shape->AIstate, &shape->varAIparams, shape->targetID_ini);
	}
	
	for (int ar = 0; ar < m_arrAreas.Count(); ar++)
	{
		CLevelArea* area = m_arrAreas[ar];
		for (int kk = 0; kk < area->m_arrProps.GetSize(); kk++)
		{
			CProp * activ = area->m_arrProps[kk];
			SetAI(activ, activ->AIstate, &activ->varAIparams, activ->targetID_ini);
		}
	}

	//#TODO: should change this or remove completely!!
	//ma asigur ca toti actorii au pointerii setati bine chemand inca odata setAI
	for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
	{
		CActor* actor = m_arrActors[kk];
		SetAI(actor, actor->AIstate, &actor->varAIparams, actor->targetID_ini);
	}



	//set interfaces ptrs
	m_interfaceIGM.Init(&m_sprInterface, pPlayerActor[0], pPlayerActor[1]);
	m_interfaceTextBubble.Init(&UTGetGUI().m_sprCol);
	///--- camera ---
	//target
	m_camTargetActive = null; //cand nu am target se uita dupa players
	m_camTargetOld = null;
	//cam settings
	m_camLevel.SetWorldBounds(m_levelAABB, false, K_CAMTRANS_AXIS_NONE);
	//for the render targets we render 1:1
	m_vCamPosDefault = vLastSpawnPoint; //spawn pointul este initializat in setAI cand gaseste checkpoint cu bIsFirst
	m_camLevel.InitCamera(UTGetAppClass().g_rectRT, K_GAME_HEIGHT, K_CAMTRANS_AXIS_V, m_vCamPosDefault); //initializam pe primul spawn point
	m_camLevel.SetCamAnimationSpring(K_LVL_CAM_FOLLOW_SPRING_KS, K_LVL_CAM_FOLLOW_DAMPING_KD);
	//chemam un update ca sa ne asiguram ca am initializat toate variabilele camerei
	m_camLevel.Update(0.0f);
	//pools
	m_poolPhysPts.Init(K_LVL_PHYSP_MAX_CNT);
	m_poolBullets.Init(K_LVL_BULLETS_MAX_CNT);
	m_poolDoofers.Init(K_LVL_DOOFERS_MAX_CNT);

	//spawn selected players
	m_nPlayers = 0;
	for (int kk = 0; kk < K_MAX_PLAYERS_CNT; kk++)
	{
		//trecem controller instanceIDs in arr local din level
		m_arrPlayerControllersIIDs[kk] = g_playerSelScr.m_arrPlayers[kk].nInstanceID;
		//set selected 
		m_arrPlayerSelHotJoin[kk] = -1;
		m_arrPlayerSelStrategic[kk] = -1;
		//daca am selectat player
		if (g_playerSelScr.m_arrPlayers[kk].bSelected)
		{
			//spawn Player aloca si controllerul potrivit
			int offx = ((kk * 2) - 1) * K_TILE_HSIZE;
			SpawnPlayer(vLastSpawnPoint + Vec2((float)offx, 0.0f), kk, -1);
			//resolve selection
			m_arrPlayerSelHotJoin[kk] = (int)g_playerSelScr.m_arrPlayers[kk].eType;
		}
		else
		{
			m_arrPlayerControllersIIDs[kk] = -1; //allow hot join
		}
	}

	// release mission generator data
	UTGetMissionGen().Release();
	//clear global script memory (per level instance)
	UTGetScriptManager().ClearGlobalMemory();
	//reset time multiplier
	SetTimeMultiplier(1.0f, 0.0f);
	// compute dirty rects (collisions and walls, wall shadows and other data)
	UpdateDirtyRects();
	// create Area meshes after shadows have been computed in UpdateDirtyRects
	for (int ii = 0; ii < m_arrAreas.GetSize(); ii++)
	{
		CLevelArea* area = m_arrAreas[ii];
		V_OP_RET(area->BuildBuffers(m_pDevice, &m_sprLights));
	}
	
	BuildVisibilityLists();

	//save type of loaded mission
	m_nLoadedLevelType = 0;
	///--- LAST THINGS ---
	//called after characters spawning
	SetLevelState(K_LVL_STATE_PLAYING);

	m_bLoaded = true;

	/*
	LOG(L"Game:: Level loaded:[%s] net.randcheck[%d]", strPathAbs, m_rand.RandInt(60000));
	
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	LOG(L"Game:: Total Targets:[%d] Hostages:[%d]", m_arrStats[K_LVL_STATS_TARGETS_TOTAL], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);
#endif
*/

	return K_OP_OK;
}


OPRESULT CLevel::LoadArea(WCHAR * strPathAbs, UINT32 nAreaID, Vec2i posTL)
{
	// increment area ID for the next area
	CLevelArea* area = new CLevelArea(nAreaID);
	// base ID for level elements so we don't overwrite existing IDs
	UINT32 unBaseID = area->ID * 10000;
	
	int nLayersCnt = K_TILE_LAYERS_CNT;

	WCHAR Path[MAX_PATH] = { 0 };

	//load level
	FILE *fl = NULL;
	int err = OS_wfopen_s(&fl, strPathAbs, L"rb");

	if (fl == NULL || err != 0)
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"Could not open area file:%s", strPathAbs);
	}

	//read int array (will disappear probably)
	UINT32 arrInts[10];
	OS_fread(arrInts, sizeof(UINT32), 10, fl);
	if (arrInts[0] != K_EDITOR_LEVEL_FILE_FORMAT_VERSION)
	{
		if (arrInts[0] == 1014)
		{
			LOG(L"LoadLevel:: Old level format found [1014]! Loading and converting lights to new format.");
		}
		else if (arrInts[0] < 1014) //last version files didn't have light volumes alpha
		{
			return OPRESULT(K_OP_FAILED, K_SEVERITY_CRITICAL, L"[Error] LoadLevel(%s)::Wrong file version found: %d !", strPathAbs, arrInts[0]);
		}
	}

	//tip misiune
	CHAR charArr[MAX_PATH];
	WCHAR wcharArr[MAX_PATH];
	WCHAR wcsMediaAddr[MAX_PATH];

	byte missionType = OS_freadByte(fl);
	//tileset name
	OS_freadString(fl, charArr);
	int tilesetColumns;
	//load tile size
	tileW = OS_freadByte(fl);
	tileH = OS_freadByte(fl);
	tilesetColumns = OS_freadUInt16(fl);
	//level size
	int areaW = OS_freadUInt16(fl);
	int areaH = OS_freadUInt16(fl);
	area->sizeTL.Init(areaW, areaH);

	//level origin - in pixels
	int originY = OS_freadInt16(fl);
	int originX = OS_freadInt16(fl);

	Vec2 vOffset(posTL.x * K_TILE_SIZE_F, posTL.y * K_TILE_SIZE_F);
	//area size
	area->AABBbounds_TL.Set(posTL.x, posTL.y, areaW, areaH);
	area->AABBbounds.Set(area->AABBbounds_TL.x * tileW, area->AABBbounds_TL.y * tileH, area->AABBbounds_TL.Right() * tileW, area->AABBbounds_TL.Bottom() * tileH);
	//m_vLevelOrigin.x = (float)originX + m_levelAABB.x;
	//m_vLevelOrigin.y = (float)originY + m_levelAABB.y;

	// add dirty rect on area so it computes everything (dirty rect is inclusive so we subtract 1 from width and height)
	m_arrDirtyRectsTL.push_back(RECTXYWH(posTL.x, posTL.y, areaW, areaH));

	// need to know the tileset size
	Vec2 vTilesetSize = m_texManager.GetTextureSize(m_tilesTexBaseIdx);

	area->tiles = new CTile*[areaW];
	for (int kk = 0; kk < areaW; kk++)
	{
		area->tiles[kk] = new CTile[areaH];
	}
	//read tiles matrix
	for (int yy = 0; yy < areaH; yy++)
	{
		for (int xx = 0; xx < areaW; xx++)
		{
			CTile* tl = &area->tiles[xx][yy];
			tl->bbox.Set((posTL.x + xx) * K_TILE_SIZE_F, (posTL.y + yy) * K_TILE_SIZE_F, (posTL.x + xx + 1) * K_TILE_SIZE_F, (posTL.y + yy + 1) * K_TILE_SIZE_F);
			for (int kk = 0; kk < nLayersCnt; kk++)
			{
				//tile layer
				int layer_index = kk;

				int tileID = OS_freadInt32(fl);
				tl->tileIDs[layer_index] = tileID;
				if (tileID >= 0)
				{
					RECT srcrect;
					SetRect(&srcrect, (tileID % tilesetColumns) * tileW, (tileID / tilesetColumns) * tileH,
						(tileID % tilesetColumns) * tileW + tileW, (tileID / tilesetColumns) * tileH + tileH);
					area->tiles[xx][yy].srcRects[layer_index] = srcrect;
					//#HACK: we make the UV rect a little smaller so we don't get UV seams because of the point filtering
					tl->vUVmin[layer_index] = Vec2((srcrect.left + 0.001f) / vTilesetSize.x, (srcrect.top + 0.001f) / vTilesetSize.y);
					tl->vUVmax[layer_index] = Vec2((srcrect.right - 0.001f) / vTilesetSize.x, (srcrect.bottom - 0.001f) / vTilesetSize.y);
				}
			}
			// computes some basic data about tiles
			tl->PostConstructionInit();
		}
	}

	size_t converted;
	///--- lights ---
	//read path
	OS_freadString(fl, charArr);

	int lightsCnt = (int)OS_freadUInt32(fl);
	// Data for each light 
	for (int kk = 0; kk < lightsCnt; kk++)
	{
		CLight *nl = new CLight();
		nl->m_nLightMeshIdx = -1;
		nl->m_nShadowMeshIdx = -1;

		nl->ID = unBaseID + OS_freadUInt32(fl);
		nl->type = (eLightType)OS_freadByte(fl); //tip lumina
		int nVolumeAttenuationPerc = (int)OS_freadUInt32(fl);
		nl->fVolumeAlpha = 1.0f - (float)nVolumeAttenuationPerc / 100.0f;
		nl->fIntensity = OS_freadFloat32(fl);
		CLAMP(nl->fIntensity, 0.0f, 1.0f);
		nl->vPos.x = (float)OS_freadInt32(fl);
		nl->vPos.y = (float)OS_freadInt32(fl);
		nl->vPos.z = (float)OS_freadInt32(fl);
		//#TODO: should load from level file
		nl->vPos.z = 32.0f;
		nl->vPos.x += vOffset.x; nl->vPos.y += vOffset.y;

		nl->vPos_ini = nl->vPos;
		nl->pos_ini = nl->pos = Vec2(nl->vPos.x, nl->vPos.y);
		//animID
		CHAR charAnmName[MAX_PATH];
		OS_freadString(fl, charAnmName);

		nl->animID = m_sprLights.GetAnimationIdxByName(charAnmName);
		nl->frameID = 0;
		/*
		if ((nl->animID < 0) && (nl->type != K_LVL_LT_AMBIENTAL))
			ErrorBox(K_ERR_WARNING, L"Light ID:%d doesn't have animID!!", nl->ID);
			*/
			//color
		BYTE ca, cr, cg, cb;
		ca = OS_freadUByte(fl); cr = OS_freadUByte(fl); cg = OS_freadUByte(fl); cb = OS_freadUByte(fl);
		nl->color = D3DCOLOR_ARGB(ca, cr, cg, cb);
		nl->color_ini = nl->color;

		Vec2 bbmin, bbmax;
		bbmin.x = (float)OS_freadInt32(fl);
		bbmin.y = (float)OS_freadInt32(fl);
		bbmax.x = bbmin.x + (float)OS_freadInt32(fl);
		bbmax.y = bbmin.y + (float)OS_freadInt32(fl);
		//set loaded size (default)
		nl->bbox.Set_Corrected(bbmin, bbmax);
		nl->bbox_ini = nl->bbox;
		nl->bbox_ini.Move(-nl->pos);
		nl->fRadius = max(nl->bbox.vSize.x, nl->bbox.vSize.y);
		//re-arrange spots (maybe lights image changed)
		nl->SetLightTexture(&m_sprLights, nl->animID, nl->frameID);

		//#HACK: hardcodes the radius
		nl->fRadius = 128.0f;

		//read angle and convert to radians
		nl->fAngle = (float)OS_freadInt16(fl);
		nl->fAngle = DEG_TO_RAD(nl->fAngle);
		nl->fAngle_ini = nl->fAngle;
		//casts shadows
		UINT16 u2b = OS_freadUInt16(fl);
		nl->castShadows = ((u2b & K_EDITOR_LIGHT_FLAG_CAST_SHADOWS) != 0);

		//save global ambient light color
		if (nl->type == K_LVL_LT_AMBIENTAL)
		{
			m_colAmbientGlobal = nl->color;
			// set ambiental bbox the size of the area
			nl->bbox_ini = area->AABBbounds;
			nl->bbox = nl->bbox_ini;
			LOG(L"light: %.2f %.2f", nl->bbox.vMin.x, nl->bbox.vMin.y);
		}

		//load logic
		nl->LoadLogic(fl);
		if (nl->targetID_ini >= 0)
			nl->targetID_ini += unBaseID;

		//set all internal light data needed for rendering
		nl->UpdateInternalData(&m_sprLights);
		// called when adding the light to the lights array
		nl->PostConstructionInit();
		m_arrLights.Add(nl);
	}

	///--- collision elements ---

	//read level collision boxes
	int colCnt = (int)OS_freadUInt32(fl);
	//date fiecare element
	for (int kk = 0; kk < colCnt; kk++)
	{
		CCollisionShape* colobj = new CCollisionShape();
		colobj->ID = unBaseID + OS_freadUInt32(fl);

		Vec2 cmin, cmax;
		cmin.x = (float)OS_freadInt32(fl); cmin.y = (float)OS_freadInt32(fl); //XY
		cmax.x = (float)OS_freadUInt32(fl); cmax.y = (float)OS_freadUInt32(fl); //WH
		cmax += cmin;
		colobj->bbox.Set(cmin, cmax);
		//bbox safeguarding
		if ((colobj->bbox.vSize.x <= 0.0f) || (colobj->bbox.vSize.y <= 0.0f))
			colobj->bbox.Set(Vec2(0.0f, 0.0f), Vec2(16.0f, 16.0f));
		colobj->bbox.Move(vOffset);
		colobj->bbox_ini = colobj->bbox;
		//set exported bboxes too
		colobj->bbox_exported = colobj->bbox;
		colobj->bbox_exported_ini = colobj->bbox_ini;
		//setam pos on center
		colobj->pos = colobj->bbox_ini.vCenter;
		//type (ub)
		colobj->type = OS_freadUByte(fl);
		//cast shadows
		colobj->castShadows = (OS_freadByte(fl) != 0) ? true : false;

		//load logic and init custom data
		colobj->LoadLogic(fl);
		if (colobj->targetID_ini >= 0)
			colobj->targetID_ini += unBaseID;

		colobj->PostConstructionInit();
		m_arrColShapes.Add(colobj);
	}


	///--- objects - props ---
	//read path
	OS_freadString(fl, charArr);

	//--- props ---
	//#TODO: de folosit spawnProp
	int decocnt = (int)OS_freadUInt32(fl);
	for (int kk = 0; kk < decocnt; kk++)
	{
		CProp* obj = new CProp();

		obj->ID = unBaseID + OS_freadUInt32(fl);
		//load layer from editor (not used atm)
		byte nLayer = OS_freadByte(fl);
		//position (used to load UINT32)
		obj->pos.x = (float)OS_freadInt32(fl);
		obj->pos.y = (float)OS_freadInt32(fl);
		obj->pos.x += vOffset.x; obj->pos.y += vOffset.y;
		obj->pos_ini = obj->pos;
		//animation
		CHAR charAnmName[MAX_PATH];
		OS_freadString(fl, charAnmName);
		int animIdx = m_sprProps.GetAnimationIdxByName(charAnmName);
		if (animIdx < 0)
			ErrorBox(K_ERR_WARNING, L"Active ID:%d without animation!", obj->ID);
		//frame
		int frameIdx = OS_freadUInt16(fl);
		obj->color = 0xffffffff;
		obj->nAnim_ini = animIdx;
		obj->nFrame_ini = frameIdx;
		obj->sprite.Init(&m_sprProps, animIdx, obj->pos, frameIdx, obj->color);
		//angle
		obj->fAngle = 0.0f;
		obj->fAngle_ini = 0.0f;
		//load flags and split
		UINT32 activFlags = OS_freadUInt32(fl);
		//flip xy
		obj->flipX = ((activFlags & K_EDITOR_ACTIVE_FLAG_FLIPX) != 0);
		obj->flipY = ((activFlags & K_EDITOR_ACTIVE_FLAG_FLIPY) != 0);
		//animated
		obj->bAnimated = ((activFlags & K_EDITOR_ACTIVE_FLAG_ANIMATED) != 0);
		//animated? select different start frame
		if (obj->bAnimated)
		{
			obj->sprite.frameIdx = m_rand.RandInt(m_sprProps.GetAFramesCnt(obj->sprite.animIdx));
		}
		//bbox
		RECTXYWH bbox_set = m_sprProps.GetAFrameBBox(animIdx, frameIdx);
		RECTXYWH objbox = m_sprProps.GetAFrameBBox_real(animIdx, frameIdx);
		obj->bbox_ini.Set(objbox);
		obj->bbox_exported_ini.Set(bbox_set);
		//daca e flipat pe X flipez si bbox. Pe Y nu e cazul pt ca se pastreaza in acelasi bbox in paint
		if (obj->flipX)
		{
			obj->bbox_ini.Move(Vec2(-2.0f * obj->bbox_ini.vCenter.x, 0.0f));
			obj->bbox_exported_ini.Move(Vec2(-2.0f * obj->bbox_exported_ini.vCenter.x, 0.0f));
		}
		obj->bbox = obj->bbox_ini;
		obj->bbox.Move(obj->pos);

		obj->bbox_exported = obj->bbox_exported_ini;
		obj->bbox_exported.Move(obj->pos);

		//load logic and init data
		obj->LoadLogic(fl);
		if (obj->targetID_ini >= 0)
			obj->targetID_ini += unBaseID;

		obj->PostConstructionInit();

		area->m_arrProps.Add(obj);
	}

	///--- load actors ---
	//read path
	OS_freadString(fl, charArr);

	int actorscnt = (int)OS_freadUInt32(fl);
	for (int kk = 0; kk < actorscnt; kk++)
	{
		UINT32 actID = unBaseID + OS_freadUInt32(fl);
		//pozitia
		Vec2 actPos;
		actPos.x = (float)OS_freadInt32(fl);
		actPos.y = (float)OS_freadInt32(fl);
		actPos += vOffset;
		//boolean SetAngle si unghi
		bool bSetActorAngle = (OS_freadByte(fl) != 0) ? true : false;
		float fActorAngle = DEG_TO_RAD(OS_freadInt16(fl));
		//read template name
		CHAR readstr[MAX_PATH];
		WCHAR templateNameW[MAX_PATH];
		OS_freadString(fl, readstr);
		mbstowcs(templateNameW, readstr, MAX_PATH);
		//read selected AI state from editor
		WCHAR stateNameW[MAX_PATH];
		OS_freadString(fl, readstr);
		mbstowcs(stateNameW, readstr, MAX_PATH);
		//direction
		bool bactLookleft = (OS_freadByte(fl) != 0) ? true : false;
		bool bactCollision = (OS_freadByte(fl) != 0) ? true : false;
		bool bactGravity = (OS_freadByte(fl) != 0) ? true : false;
		//logic
		byte n1b = OS_freadByte(fl);
		bool bactCanInteract = (n1b & 0x1);
		bool bactHideInteract = (n1b & 0x2);
		float factTouchDuration = (float)OS_freadInt32(fl);
		bool bactStartHidden = (OS_freadByte(fl) != 0) ? true : false;
		INT32 nactTargetID = unBaseID + OS_freadInt32(fl);
		//read script AI name
		CHAR strScriptName[MAX_PATH];
		CHAR strAIname[MAX_PATH];
		OS_freadString(fl, strScriptName);
		OS_freadString(fl, strAIname);
		//read AI params
		CVariantCollection arrParams;
		int nAIparamsCnt = OS_freadByte(fl); //nr params
		if (nAIparamsCnt > 0)
		{
			for (int i = 0; i < nAIparamsCnt; i++)
			{
				CHAR varname[MAX_PATH] = { 0 };
				WCHAR wvarname[MAX_PATH] = { 0 };
				CHAR varval[MAX_PATH];
				WCHAR wvarval[MAX_PATH];

				OS_freadString(fl, varname);
				OS_freadString(fl, varval);

				size_t convnr;
				mbstowcs_s(&convnr, wvarname, varname, MAX_PATH);
				mbstowcs_s(&convnr, wvarval, varval, MAX_PATH);

				arrParams.SetNamedVarAUTO(wvarname, wvarval);
			}
		}
		///--- FINISHED READING DATA ---
		CStringHash shTemplateNameHash(templateNameW);
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
	UINT32 miscCnt = OS_freadUInt32(fl);
	for (UINT32 kk = 0; kk < miscCnt; kk++)
	{
		byte type = OS_freadByte(fl);

		switch (type)
		{
			case K_LVL_MISC_FRONTLAYEROBJ:
			{
				CMiscObject_FrontLayerObj * frontobj = new CMiscObject_FrontLayerObj();
				//generic data
				frontobj->ID = unBaseID + OS_freadUInt32(fl);
				//read params
				int nparamsCnt = OS_freadByte(fl); //nr params
				if (nparamsCnt > 0)
				{
					for (int i = 0; i < nparamsCnt; i++)
					{
						CHAR varname[MAX_PATH] = { 0 };
						CHAR varval[MAX_PATH];
						WCHAR wvarval[MAX_PATH], wvarname[MAX_PATH];

						OS_freadString(fl, varname);
						OS_freadString(fl, varval);

						size_t convnr;
						mbstowcs_s(&convnr, wvarval, varval, MAX_PATH);
						mbstowcs_s(&convnr, wvarname, varname, MAX_PATH);

						frontobj->varParams.SetNamedVarAUTO(wvarname, wvarval);
					}
				}
				//specific data 
				frontobj->pos.x = (float)OS_freadInt32(fl);
				frontobj->pos.y = (float)OS_freadInt32(fl);
				frontobj->pos += vOffset;
				//set color
				frontobj->sprite.color = m_colAmbientGlobal;

				m_arrMiscObjects.Add(frontobj);
			}
			break;
			case K_LVL_MISC_SCRIPT:
			{
				//generic data
				UINT32 ID = unBaseID + OS_freadUInt32(fl);
				//read params
				int nparamsCnt = OS_freadByte(fl); //nr params
				if (nparamsCnt > 0)
				{
					for (int i = 0; i < nparamsCnt; i++)
					{
						CHAR varname[MAX_PATH] = { 0 };
						CHAR varval[MAX_PATH];
						WCHAR wvarname[MAX_PATH];
						WCHAR wvarval[MAX_PATH];

						OS_freadString(fl, varname);
						OS_freadString(fl, varval);

						size_t convnr;
						mbstowcs_s(&convnr, wvarval, varval, MAX_PATH);
						mbstowcs_s(&convnr, wvarname, varname, MAX_PATH);

						if (wcscmp(wvarname, L"str_script") == 0)
						{
							//am citit primul parametru iar valoarea lui este bsx-ul fundalului deci incarc fundalul
							UTGetScriptManager().StartScript(wvarval);
						}
					}
				}
				//pozitia o citesc si nu o folosesc
				OS_freadUInt32(fl); OS_freadUInt32(fl);
			}
			break;
			case K_LVL_MISC_RAILS:
			{
				CMiscObjectRail* rail = new CMiscObjectRail();
				//generic data
				rail->ID = unBaseID + OS_freadUInt32(fl);
				//read params
				int nparamsCnt = OS_freadByte(fl); //nr params
				if (nparamsCnt > 0)
				{
					for (int i = 0; i < nparamsCnt; i++)
					{
						CHAR varname[MAX_PATH] = { 0 };
						CHAR varval[MAX_PATH] = { 0 };
						WCHAR wvarname[MAX_PATH];
						WCHAR wvarval[MAX_PATH];

						OS_freadString(fl, varname);
						OS_freadString(fl, varval);

						size_t convnr;
						mbstowcs_s(&convnr, wvarname, varval, MAX_PATH);
						mbstowcs_s(&convnr, wvarval, varval, MAX_PATH);

						rail->varParams.SetNamedVarAUTO(wvarname, wvarval);
					}
				}
				//specific data 
				//pozitia punctelor
				UINT16 ptscnt = OS_freadUInt16(fl);
				float totalLength = 0.0f;
				//coordonate puncte
				for (int i = 0; i < ptscnt; i++)
				{
					Vec2 pt;
					pt.x = OS_freadInt32(fl);
					pt.y = OS_freadInt32(fl);
					pt += vOffset;
					rail->arrPoints.Add(pt);
					//lungimile
					if (i == 0)
					{
						totalLength = 0.0f;
						rail->arrLenghts.Add(totalLength);
					}
					else
					{
						Vec2 dist = rail->arrPoints.m_pData[i] - rail->arrPoints.m_pData[i - 1];
						float ldist = D3DXVec2Length(&dist);
						totalLength += ldist;
						rail->arrLenghts.Add(totalLength);
					}
				}
				rail->fLength = totalLength;
				//check total len
				if (totalLength <= 0.0f)
				{
					ErrorBox(K_ERR_WARNING, L"Zero length rail! ID:%d", rail->ID);
					SAFE_DELETE(rail);
					break;
				}
				//add rail to list if everything ok
				m_arrMiscObjects.Add(rail);
			}
			break;
		}
	}
	//set animation data at the end (some front objs need bg to be loaded)
	for (UINT32 kk = 0; kk < m_arrMiscObjects.GetSize(); kk++)
	{
		CMiscObjectBase* mob = m_arrMiscObjects[kk];
		if (mob->type == K_LVL_MISC_FRONTLAYEROBJ)
		{
			CMiscObject_FrontLayerObj *frontobj = dynamic_cast<CMiscObject_FrontLayerObj*>(mob);
			if (frontobj != null)
			{
				//anim name
				UINT32 animHash = frontobj->varParams.GetVariantByName(L"strAnim")->m_strArg.getHash();
				frontobj->sprite.animationIdx = -1;// m_sprBack.getAnimationIdxByNameHash(animHash);
				frontobj->sprite.currentFrame = frontobj->varParams.GetVariantByName(L"nFrame")->m_asUINT32;
				//set bbox
				//frontobj->aabb_ini.Set(m_sprBack.GetAFrameBBox(frontobj->sprite.animationIdx, frontobj->sprite.currentFrame));
			}
		}
	}

	OS_fclose(fl);


	LOG(L"Game:: Area loaded:[%s] net.randcheck[%d]", strPathAbs, m_rand.RandInt(60000));

	m_arrAreas.Add(area);

	return K_OP_OK;
}


