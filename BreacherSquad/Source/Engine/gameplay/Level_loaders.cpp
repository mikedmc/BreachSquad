#include "dxstdafx.h"


HRESULT CLevel::LoadLevel(WCHAR * strPathAbs)
{
	HRESULT hr = S_OK;

	//set last ID on a number that will never get reached from the editor
	m_unLastID = 100000;
	int nLayersCnt = K_LVL_LAYERS_CNT;
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
		m_rand.SetRandomSeed(g_netlock.m_unRandomSeed);
	}
	else
	{
		//randomize seed
		m_rand.SetRandomSeed(GetTickCount());
	}
	//reset local timeline
	fLocalTimeline = 0.0f;
	vLastSpawnPoint = D3DXVECTOR2(0.0f, 0.0f);

	//realease level if loaded
	Release();

	//reset shakes
	m_camLevel.ShakeScreen(0.0f, 0.0f);
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
	//team doors
	m_nTeleportSlots = 0;
	m_bTeleportActivated = false;
	//	m_bTeleportRequested = false;
	m_bInsideHiddenRoom = false;
	m_bPlayerInHiddenRoom[0] = m_bPlayerInHiddenRoom[1] = false;
	m_pTeleportSource = null;

	m_interfaceIGM.Reset();
	m_interfaceIGM.SetBombTimer(-1.0f);
	//load interface sprites
	FileManager::GetMediaPath(L"media/interfaces/igm_interface.bsx", Path);
	if (OP_FAILED(m_sprInterface.LoadSprites(Path)))
	{
		return E_FAIL;
	}

	//load level
	FILE *fl = NULL;
	int err = OS_wfopen_s(&fl, strPathAbs, L"rb");

	if (fl == NULL || err != 0)
	{
		return E_FAIL;
	}

	//read int array
	//#IMPORTANT: !!!If you change this part make sure you update it in App_ResetUserData() too!!!
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
			ErrorBox(K_ERR_WARNING, L"[Error] LoadLevel(%s)::Wrong file version found: %d !", strPathAbs, arrInts[0]);
			return E_FAIL;
		}
	}

	//tip misiune
	byte missionType = OS_freadByte(fl);
	//save the level type again if we're playing a standard level
	if (m_unLoadedLevelFlags != K_LVL_LEVEL_FLAG_NONE)
	{
		g_levelStats[nLevelNumber + nChapterNumber * K_GAME_LEVELS_PER_CHAPTER].nLevelType = missionType;
	}
	//tileset name
	CHAR charArr[MAX_PATH];
	WCHAR wcharArr[MAX_PATH];
	WCHAR wcsMediaAddr[MAX_PATH];

	OS_freadString(fl, charArr);
	size_t converted;
	mbstowcs_s(&converted, wcharArr, charArr, MAX_PATH);

	StringCchPrintf(wcsMediaAddr, MAX_PATH, L"media/levels/data/%s", wcharArr);
	FileManager::GetMediaPath(wcsMediaAddr, Path);
	if (FAILED(m_texManager.AddTexture(Path, &m_tilesTexBaseIdx, D3DFMT_A8R8G8B8, D3DX_FILTER_NONE, D3DX_FILTER_NONE)))
	{
		ErrorBox(K_ERR_CRITICAL, L"Couldn't load tileset texture: %s", Path);
		return E_FAIL;
	}
	// SAVE TEXTURE SIZE
	Vec2 vTilesetSize = m_texManager.GetTextureSize(m_tilesTexBaseIdx);
	assert(vTilesetSize.x > 0.0f && vTilesetSize.y > 0.0f);

	//#TODO: deletes the last 4 characters (.png) and adds another ending... should be handled differently (from the editor)
	wcsMediaAddr[wcslen(wcsMediaAddr) - 4] = 0;
	StringCchCat(wcsMediaAddr, MAX_PATH, L"_nh.png");
	FileManager::GetMediaPath(wcsMediaAddr, Path);
	if (FAILED(m_texManager.AddTexture(Path, &m_tilesTexNormIdx, D3DFMT_A8R8G8B8, D3DX_FILTER_NONE, D3DX_FILTER_NONE)))
	{
		ErrorBox(K_ERR_CRITICAL, L"Couldn't load tileset normals texture: %s", Path);
		return E_FAIL;
	}
	//#TODO: water texture should be loaded from the editor also (not hardcoded)
	FileManager::GetMediaPath(L"media/levels/data/water1_n.png", Path);
	if (FAILED(m_texManager.AddTexture(Path, &m_waterTexIdx, D3DFMT_A8R8G8B8, D3DX_FILTER_NONE, D3DX_FILTER_NONE)))
	{
		ErrorBox(K_ERR_CRITICAL, L"Couldn't load water texture: %s", Path);
		return E_FAIL;
	}

	int tilesetColumns;
	//load tile size
	tileW = OS_freadByte(fl);
	tileH = OS_freadByte(fl);
	tilesetColumns = OS_freadUInt16(fl);
	//level size
	int levelW = OS_freadUInt16(fl);
	int levelH = OS_freadUInt16(fl);

	//level origin - in pixels
	int originY = OS_freadInt16(fl);
	int originX = OS_freadInt16(fl);

	levelSizeTL.w = levelW;
	levelSizeTL.h = levelH;

	//set level size
	m_levelAABB_TL.Set(0, 0, levelSizeTL.w, levelSizeTL.h);
	m_levelAABB.Set(m_levelAABB_TL.x * tileW, m_levelAABB_TL.y * tileH, levelSizeTL.w * tileW, levelSizeTL.h * tileH);
	m_vLevelOrigin.x = (float)originX + m_levelAABB.x;
	m_vLevelOrigin.y = (float)originY + m_levelAABB.y;

	tiles = new CTile*[levelSizeTL.w];
	for (int kk = 0; kk < levelSizeTL.w; kk++)
	{
		tiles[kk] = new CTile[levelSizeTL.h];
	}
	//read tiles, not a rare matrix
	eTileLayer layerIDs[] = { K_TILE_LAYER_FLOOR, K_TILE_LAYER_WALLS, K_TILE_LAYER_CEILING };
	for (int yy = 0; yy < levelSizeTL.h; yy++)
	{
		for (int xx = 0; xx < levelSizeTL.w; xx++)
		{
			CTile* tl = &tiles[xx][yy];
			for (int kk = 0; kk < nLayersCnt; kk++)
			{
				//nivel
				int layID = layerIDs[kk];

				int tileID = OS_freadInt32(fl);
				tl->tileIDs[layID] = tileID;
				if (tileID >= 0)
				{
					RECT srcrect;
					SetRect(&srcrect, (tileID % tilesetColumns) * tileW, (tileID / tilesetColumns) * tileH, 
						(tileID % tilesetColumns) * tileW + tileW, (tileID / tilesetColumns) * tileH + tileH);
					tiles[xx][yy].srcRects[layID] = srcrect;
					//#TODO: aici trebuie sa le scada jumatate de texel daca e DX, si sa verifici ca afiseaza si ultimul pixel din textura
					tl->vUVmin[layID] = Vec2(srcrect.left / vTilesetSize.x, srcrect.top / vTilesetSize.y);
					tl->vUVmax[layID] = Vec2(srcrect.right / vTilesetSize.x, srcrect.bottom / vTilesetSize.y);
				}
			}
			// computes some basic data about tiles
			tl->PostConstructionInit();
		}
	}
	// add dirty rect on level so it computes everything
	m_arrDirtyRectsTL.clear();
	m_arrDirtyRectsTL.reserve(5);
	m_arrDirtyRectsTL.push_back(RECTXYXY(0, 0, levelSizeTL.w, levelSizeTL.h));

	///--- lights ---
	SAFE_DELETE_GROWABLE_ARRAY(m_arrLights);
	//read path
	OS_freadString(fl, charArr);
	mbstowcs_s(&converted, wcharArr, charArr, MAX_PATH);
	//load bsx
	StringCchPrintf(wcsMediaAddr, MAX_PATH, L"media/levels/data/%s", wcharArr);
	FileManager::GetMediaPath(wcsMediaAddr, Path);
	if (OP_FAILED(m_sprLights.LoadSprites(Path)))
	{
		return E_FAIL;
	}

	int lightsCnt = (int)OS_freadUInt32(fl);
	// Data for each light 
	for (int kk = 0; kk < lightsCnt; kk++)
	{
		CLight *nl = new CLight();
		nl->m_nLightMeshIdx = -1;
		nl->m_nShadowMeshIdx = -1;

		nl->ID = OS_freadUInt32(fl);
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

		nl->vPos_ini = nl->vPos;
		nl->pos_ini = nl->pos = D3DXVECTOR2(nl->vPos.x, nl->vPos.y);
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

		D3DXVECTOR2 bbmin, bbmax;
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
			m_colAmbientGlobal = nl->color;

		//load logic
		nl->LoadLogic(fl);

		//set all internal light data needed for rendering
		nl->UpdateInternalData(&m_sprLights);
		// called when adding the light to the lights array
		nl->PostConstructionInit();
		m_arrLights.Add(nl);
	}

	///--- collision elements ---
	SAFE_DELETE_GROWABLE_ARRAY(m_arrColShapes);
	//add level limits
	CCollisionShape* cc = null;
	//left
	cc = new CCollisionShape();
	cc->bbox.Set(D3DXVECTOR2(m_levelAABB.x - 32.0f, m_levelAABB.y), D3DXVECTOR2(m_levelAABB.x + 2.0f, m_levelAABB.Bottom()));
	cc->ubFlags = K_LVL_COLLFLAG_LEVEL_BOUNDS;
	m_arrColShapes.Add(cc);
	//right
	cc = new CCollisionShape();
	cc->bbox.Set(D3DXVECTOR2(m_levelAABB.Right() - 2.0f, m_levelAABB.y), D3DXVECTOR2(m_levelAABB.Right() + 32.0f, m_levelAABB.Bottom()));
	cc->ubFlags = K_LVL_COLLFLAG_LEVEL_BOUNDS;
	m_arrColShapes.Add(cc);
	//bottom
	cc = new CCollisionShape();
	cc->bbox.Set(D3DXVECTOR2(m_levelAABB.x, m_levelAABB.Bottom() - 2.0f), D3DXVECTOR2(m_levelAABB.Right(), m_levelAABB.Bottom() + 32.0f));
	cc->ubFlags = K_LVL_COLLFLAG_LEVEL_BOUNDS;
	m_arrColShapes.Add(cc);
	//top
	cc = new CCollisionShape();
	cc->bbox.Set(D3DXVECTOR2(m_levelAABB.x, m_levelAABB.y - 32.0f), D3DXVECTOR2(m_levelAABB.Right(), m_levelAABB.y + 2.0f));
	cc->ubFlags = K_LVL_COLLFLAG_LEVEL_BOUNDS;
	m_arrColShapes.Add(cc);

	//read level collision boxes
	int colCnt = (int)OS_freadUInt32(fl);
	//date fiecare element
	for (int kk = 0; kk < colCnt; kk++)
	{
		CCollisionShape* colobj = new CCollisionShape();
		colobj->ID = OS_freadUInt32(fl);

		D3DXVECTOR2 cmin, cmax;
		cmin.x = (float)OS_freadInt32(fl); cmin.y = (float)OS_freadInt32(fl); //XY
		cmax.x = (float)OS_freadUInt32(fl); cmax.y = (float)OS_freadUInt32(fl); //WH
		cmax += cmin;
		colobj->bbox.Set(cmin, cmax);
		//bbox safeguarding
		if ((colobj->bbox.vSize.x <= 0.0f) || (colobj->bbox.vSize.y <= 0.0f))
			colobj->bbox.Set(D3DXVECTOR2(0.0f, 0.0f), D3DXVECTOR2(16.0f, 16.0f));
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
		colobj->PostConstructionInit();
		m_arrColShapes.Add(colobj);
	}


	///--- objects - decorations ---
	SAFE_DELETE_GROWABLE_ARRAY(m_arrProps);
	m_arrPropsPtrInteract.Clear();
	//read path
	OS_freadString(fl, charArr);
	mbstowcs_s(&converted, wcharArr, charArr, MAX_PATH);
	//load bsx
	StringCchPrintf(wcsMediaAddr, MAX_PATH, L"media/levels/data/%s", wcharArr);
	FileManager::GetMediaPath(wcsMediaAddr, Path);
	if (OP_FAILED(m_sprProps.LoadSprites(Path)))
	{
		return E_FAIL;
	}

	//--- props ---
	CFixedArray<int, 20> arrLocalBombIDs;

	int decocnt = (int)OS_freadUInt32(fl);
	for (int kk = 0; kk < decocnt; kk++)
	{
		CProp* obj = new CProp();

		obj->ID = OS_freadUInt32(fl);
		//convert layer from editor values to game values (editor misses MID layer):
		byte nLayer = OS_freadByte(fl);
		obj->nLayer = nLayer;
		//position (used to load UINT32)
		obj->pos.x = (float)OS_freadInt32(fl);
		obj->pos.y = (float)OS_freadInt32(fl);
		obj->pos_ini = obj->pos;
		//animation
		CHAR charAnmName[MAX_PATH];
		OS_freadString(fl, charAnmName);
		int animIdx = m_sprProps.GetAnimationIdxByName(charAnmName);
		if (animIdx < 0)
			ErrorBox(K_ERR_WARNING, L"Active ID:%d without animation!", obj->ID);
		//frame
		int frameIdx = OS_freadUInt16(fl);
		obj->sprite.Init(animIdx, obj->pos.x, obj->pos.y, frameIdx);
		obj->nAnim_ini = animIdx;
		obj->nFrame_ini = frameIdx;
		obj->color = 0xffffffff;
		obj->sprite.color = obj->color;
		obj->bStandsOut = false;
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
		obj->bReleaseIt = false;
		//animated? select different start frame
		if (obj->bAnimated)
		{
			obj->sprite.currentFrame = m_rand.RandInt(m_sprProps.GetAFramesCnt(obj->sprite.animationIdx));
		}
		//bbox
		RECTXYWH bbox_set = m_sprProps.GetAFrameBBox(animIdx, frameIdx);
		RECTXYWH objbox = m_sprProps.GetAFrameBBox_real(animIdx, frameIdx);
		obj->bbox_ini.Set(objbox);
		obj->bbox_exported_ini.Set(bbox_set);
		//daca e flipat pe X flipez si bbox. Pe Y nu e cazul pt ca se pastreaza in acelasi bbox in paint
		if (obj->flipX)
		{
			obj->bbox_ini.Move(D3DXVECTOR2(-2.0f * obj->bbox_ini.vCenter.x, 0.0f));
			obj->bbox_exported_ini.Move(D3DXVECTOR2(-2.0f * obj->bbox_exported_ini.vCenter.x, 0.0f));
		}
		obj->bbox = obj->bbox_ini;
		obj->bbox.Move(obj->pos);

		obj->bbox_exported = obj->bbox_exported_ini;
		obj->bbox_exported.Move(obj->pos);

		//load logic and init data
		obj->LoadLogic(fl);
		obj->PostConstructionInit();

		m_arrProps.Add(obj);

		//mark and save interactibles
		if (obj->bCanInteract)
			m_arrPropsPtrInteract.Add(obj);

		///--- setari speciale ---
		//#HARDCODE: is cover? - add bbox as cover box
		if (activFlags & K_EDITOR_ACTIVE_FLAG_IS_COVER)
		{
			//make the object stand out
			obj->bStandsOut = true;

			CCollisionShape* colobj = new CCollisionShape();
			colobj->ID = GenerateNextID();

			RECTXYWH objbox = m_sprProps.GetAFrameBBox(obj->sprite.animationIdx, obj->sprite.currentFrame);
			//objbox.Move(obj->pos.x, obj->pos.y);

			colobj->bbox.Set(objbox);
			colobj->bbox.Flip(obj->flipX, false);
			colobj->Move(obj->pos);

			colobj->bbox_ini = colobj->bbox;
			//setam si pos pe centrul ei
			colobj->pos = colobj->bbox_ini.vCenter;
			colobj->type = K_LVL_COLL_TYPE_COVER;
			//cast shadows - off by default
			colobj->castShadows = false;
			//load logic
			colobj->bCanInteract = false;
			colobj->bHideInteractIcon = false;
			colobj->bHidden = false;

			colobj->targetID_ini = -1; //save for later
			colobj->AIstate = K_AI_STATE_UNDEFINED;
			colobj->collFlags = K_DIRFLAG_NONE;
			colobj->stairSize = 0;

			colobj->bReleaseIt = false;

			m_arrColShapes.Add(colobj);
		}
		//#HACK: is it a door? set special AI
		CStringHash shAnimName(charAnmName);
		const CStringHash shAnimDoors(L"DOORS_SECTION");
		if (shAnimName == shAnimDoors)
		{
			obj->AIstate = K_AI_STATE_ACTIVE_DOOR_SECTION;
			if (obj->script_hash.IsEqual(L"ACTIVE_LOCKED_BREAKABLE"))
			{
				obj->varAIparams.SetNamedVarINT32(L"n_locked", 1);
				obj->varAIparams.SetNamedVarFloat(L"f_lockpickTime", 2.5f);
			}
		}
		//--- bomb defusal mode ---
		if (obj->AIstate == K_AI_STATE_ACTIVE_BOMB)
		{
			arrLocalBombIDs.Add(obj->ID);
		}
		//--- zombie spawners ---
		if (obj->AIstate == K_AI_STATE_ACTIVE_ZOMBIE_SPAWNER)
		{
			m_arrStats[K_LVL_STATS_ZOMBIE_PORTALS]++;
		}
	}
	//--- init bomb mode ---
	if (arrLocalBombIDs.Count() > 0)
	{
		m_arrStats[K_LVL_STATS_LEVEL_HAS_BOMBS] = 1;
		m_arrStats[K_LVL_STATS_LEVEL_BOMB_SEEN] = 0;
		//select random bomb
		int nFinalBombIdx = m_rand.RandInt(arrLocalBombIDs.Count());
		m_arrStats[K_LVL_STATS_LEVEL_BOMB_ID] = arrLocalBombIDs[nFinalBombIdx];
		//hide-deallocate other bombs
		for (int kk = 0; kk < arrLocalBombIDs.Count(); kk++)
		{
			if (kk != nFinalBombIdx)
			{
				CProp* bombact = dynamic_cast<CProp*>(GetIActiveInterfacePtr(arrLocalBombIDs[kk]));
				if (bombact != null)
				{
					bombact->bSetHidden = true;
					bombact->bHidden = true;
					bombact->bReleaseIt = true;
				}
			}
		}
	}

	///--- load actors ---
	SAFE_DELETE_GROWABLE_ARRAY(m_arrActors);
	//read path
	OS_freadString(fl, charArr);
	mbstowcs_s(&converted, wcharArr, charArr, MAX_PATH);
	//load bsx
	StringCchPrintf(wcsMediaAddr, MAX_PATH, L"media/levels/data/%s", wcharArr);
	FileManager::GetMediaPath(wcsMediaAddr, Path);
	if (OP_FAILED(m_sprActors.LoadSprites(Path)))
	{
		return E_FAIL;
	}

	//--- load actors templates and weaponry right after props sprite ---
	FileManager::GetMediaPath(L"media/levels/data/weapons_data.xml", Path);
	if (FAILED(LoadWeaponTemplates(Path)))
	{
		return E_FAIL;
	}

	FileManager::GetMediaPath(L"media/levels/data/actors_data.xml", Path);
	if (FAILED(LoadActorTemplates(Path)))
	{
		return E_FAIL;
	}


	int actorscnt = (int)OS_freadUInt32(fl);
	for (int kk = 0; kk < actorscnt; kk++)
	{
		UINT32 actID = OS_freadUInt32(fl);
		//pozitia
		D3DXVECTOR2 actPos;
		actPos.x = (float)OS_freadInt32(fl);
		actPos.y = (float)OS_freadInt32(fl);
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
		INT32 nactTargetID = OS_freadInt32(fl);
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

		///--- RANDOM ENEMIES HERE ---
		CStringHash shTemplateNameHash(templateNameW);
		if (shTemplateNameHash.textHash == FastHash(L"ACTOR_RANDOM_ENEMY"))
		{
			//random enemy should have multiple AI params lines with different templates on them.
			//will select a random character from the AI specified lines
			int nTemplatesCnt = arrParams.GetVariantCount();
			if (nTemplatesCnt == 0)
			{
				ErrorBox(K_ERR_CRITICAL, L"Random Enemy (ID:%d) should have more templates! Please specify templates in AI params as strings!", actID);
			}
			else
			{
				int nRandTemplate = m_rand.RandInt(nTemplatesCnt);
				shTemplateNameHash = arrParams[nRandTemplate]->m_strArg;
				if (shTemplateNameHash.textHash == 0)
				{
					shTemplateNameHash.Init(L"ACTOR_RANDOM_ENEMY");
					ErrorBox(K_ERR_CRITICAL, L"Random Enemy (ID:%d) illegal template name: [%s]", actID, arrParams[nRandTemplate]->m_strArg.text);
				}
			}
			//erase AI params
			arrParams.DeleteAll();
		}

		//add actor
		CActor* nact = new CActor();

		nact->ID = actID; //save actor ID
		nact->bAnimated = true;  //animated by default

		CActorTemplate* acttempl = GetTemplateActor(shTemplateNameHash.textHash);
		if (acttempl == null)
		{
			ErrorBox(K_ERR_WARNING, L"LoadLevel::GetTemplateActor - invalid template name: %s", shTemplateNameHash.text);
		}
		InitActor(nact, acttempl, actPos);

		nact->lookDirXsign = (bactLookleft) ? -1 : 1;
		nact->fAngle = nact->fAngle_ini = fActorAngle;
		//daca unghiul e setat din editor il las asa cum e, altfel il sincronizez cu lookdirXsign
		//Unghiul trebuie setat corect pentru ca e folosit la gasirea inamicilor
		if (bSetActorAngle)
		{
			nact->SetAngle(fActorAngle);
		}
		else
		{
			if (nact->lookDirXsign == -1)
				nact->SetAngle(PI);
			else
				nact->SetAngle(0.0f);
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
			CAIState* nState = nact->templateActor.AItemplate->GetAIStateByName(stateNameW);
			if (nState == null)
			{
				ErrorBox(K_ERR_WARNING, L"[WARNING] LoadLevel: State %s not found on ID:%d", stateNameW, nact->ID);
			}
			SetActorAIState(nact, nState);
		}

		m_arrActors.Add(nact);

		//--- update statistics ---
		bool bCountEnemy = false;
		if (nact->GetCurrentBehavior() != AI_BEHAVIOR_DEAD)
		{
			//#HACK: set script on hostage and count them as targets
			if (nact->templateActor.actorClass == K_LVL_ACT_CLASS_HOSTAGE)
			{
				bCountEnemy = false;
				//add actor as target only if not spawned already dead (DEAD behavior)
				m_arrStats[K_LVL_STATS_TARGETS_TOTAL]++;
				m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]++;
				//forteaza scriptul de save hostage chiar daca nu l-ai setat din editor
				nact->bCanInteract = true;
				if (nact->script_hash.IsEmpty())
					nact->script_hash.Init(L"SAVE_HOSTAGE");
			}
			else if (nact->templateActor.actorClass == K_LVL_ACT_CLASS_HUMAN)
			{
				//add actor as target only if not spawned already dead (DEAD behavior)
				bCountEnemy = true;
			}
			else if (nact->templateActor.actorClass == K_LVL_ACT_CLASS_ZOMBIE)
			{
				m_arrStats[K_LVL_STATS_ZOMBIES_TOTAL]++;
				bCountEnemy = true;
			}
		}
		//now count enemy targets
		if (bCountEnemy)
		{
			m_arrStats[K_LVL_STATS_TARGETS_TOTAL]++;
		}
	}

	//initialize targets left
	m_arrStats[K_LVL_STATS_TARGETS_LEFT] = m_arrStats[K_LVL_STATS_TARGETS_TOTAL];

	///--- incarca elementele speciale ---
	SAFE_DELETE_GROWABLE_ARRAY(m_arrMiscObjects);

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
				frontobj->ID = OS_freadUInt32(fl);
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
				//pozitia o citesc si nu o folosesc
				frontobj->pos.x = (float)OS_freadInt32(fl);
				frontobj->pos.y = (float)OS_freadInt32(fl);
				//set color
				frontobj->sprite.color = m_colAmbientGlobal;

				m_arrMiscObjects.Add(frontobj);
			}
			break;
			case K_LVL_MISC_SCRIPT:
			{
				//generic data
				UINT32 ID = OS_freadUInt32(fl);
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
			case K_LVL_MISC_BACKGROUND:
			{
				//generic data
				UINT32 ID = OS_freadUInt32(fl);
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
						//HARDCODE: de scos hardcodarea dupa i
						if (wcscmp(wvarname, L"str_bsx") == 0)
						{
							//first param is the bsx-ul for the background so load it
							StringCchPrintf(wcsMediaAddr, MAX_PATH, L"media/back/%s", wvarval);
							FileManager::GetMediaPath(wcsMediaAddr, Path);
							//m_sprBack.LoadSprites(Path);
							//defaults on first anim
							//m_BackAnimIdx = 0;
						}
						else if (wcscmp(wvarname, L"str_anim") == 0)
						{
							//m_BackAnimIdx = m_sprBack.getAnimationIdxByName(wvarval);
						}
						else if (wcscmp(wvarname, L"str_water_anim") == 0)
						{
							//m_waterAnimIdx = m_sprBack.getAnimationIdxByName(wvarval);
						}
					}
				}
				//specific data 
				//pozitia o citesc si nu o folosesc
				OS_freadUInt32(fl); OS_freadUInt32(fl);

				//m_bPaintBackground = true;
			}
			break;
			case K_LVL_MISC_RAILS:
			{
				CMiscObjectRail* rail = new CMiscObjectRail();
				//generic data
				rail->ID = OS_freadUInt32(fl);
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
					D3DXVECTOR2 pt;
					pt.x = OS_freadInt32(fl);
					pt.y = OS_freadInt32(fl);
					rail->arrPoints.Add(pt);
					//lungimile
					if (i == 0)
					{
						totalLength = 0.0f;
						rail->arrLenghts.Add(totalLength);
					}
					else
					{
						D3DXVECTOR2 dist = rail->arrPoints.m_pData[i] - rail->arrPoints.m_pData[i - 1];
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

	///--- everything loaded, SetAI here ---
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
	for (int kk = 0; kk < m_arrProps.GetSize(); kk++)
	{
		CProp * activ = m_arrProps[kk];
		SetAI(activ, activ->AIstate, &activ->varAIparams, activ->targetID_ini);
	}
	//ma asigur ca toti actorii au pointerii setati bine chemand inca odata setAI
	for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
	{
		CActor* actor = m_arrActors[kk];
		SetAI(actor, actor->AIstate, &actor->varAIparams, actor->targetID_ini);
	}

	//other settings:
	SAFE_DELETE_GROWABLE_ARRAY(m_arrDecals);

	//set interfaces ptrs
	m_interfaceIGM.Init(&m_sprInterface, pPlayerActor[0], pPlayerActor[1]);
	m_interfaceTextBubble.Init(&UTGetGUI().m_sprCol);
	///--- camera ---
	//target
	m_camTargetActive = null; //cand nu am target se uita dupa players
	m_camTargetOld = null;
	//cam settings
	m_camLevel.SetWorldBounds(m_levelAABB, true, K_CAMTRANS_AXIS_NONE);
	m_HiddenRoomAABB.Set(0.0f, 0.0f, 0.0f, 0.0f);
	//for the render targets we render 1:1
	m_vCamPosDefault = vLastSpawnPoint; //spawn pointul este initializat in setAI cand gaseste checkpoint cu bIsFirst
	m_camLevel.InitCamera(UTGetAppClass().g_rectRT, K_GAME_HEIGHT, K_CAMTRANS_AXIS_V, m_vCamPosDefault); //initializam pe primul spawn point
	m_camLevel.SetCamAnimationSpring(K_LVL_CAM_FOLLOW_SPRING_KS, K_LVL_CAM_FOLLOW_DAMPING_KD);
	//chemam un update ca sa ne asiguram ca am initializat toate variabilele camerei
	m_camLevel.Update(0.0f);
	//pools
	m_poolPhysPts.Init(K_LVL_PHYSP_MAX_CNT);
	m_poolBullets.Init(K_LVL_BULLETS_MAX_CNT);
	m_poolProps.Init(K_LVL_PROPS_MAX_CNT);

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
			SpawnPlayer(vLastSpawnPoint + D3DXVECTOR2((float)offx, 0.0f), kk, -1);
			//resolve selection
			m_arrPlayerSelHotJoin[kk] = (int)g_playerSelScr.m_arrPlayers[kk].eType;
		}
		else
		{
			m_arrPlayerControllersIIDs[kk] = -1; //allow hot join
		}
	}

	//clear global script memory (per level instance)
	UTGetScriptManager().ClearGlobalMemory();

	//reset time multiplier
	SetTimeMultiplier(1.0f, 0.0f);
	// compute dirty rects (collisions and walls and other data)
	UpdateDirtyRects();
	//facem un build visibility lists
	BuildVisibilityLists();

	//save type of loaded mission
	m_nLoadedLevelType = missionType;
	///--- LAST THINGS ---
	//called after characters spawning
	SetLevelState(K_LVL_STATE_PLAYING);

	m_bLoaded = true;

	LOG(L"Game:: Level loaded:[%s] net.randcheck[%d]", strPathAbs, m_rand.RandInt(60000));
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	LOG(L"Game:: Total Targets:[%d] Hostages:[%d]", m_arrStats[K_LVL_STATS_TARGETS_TOTAL], m_arrStats[K_LVL_STATS_HOSTAGES_TOTAL]);
#endif

	// create meshes
	V_OP_RETHR(mapMesh.BuildBuffers(tiles, levelSizeTL, Vec2(0.0f, 0.0f), &m_sprLights));

	return hr;
}


