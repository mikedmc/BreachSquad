#include "dxstdafx.h"

CInfiniteVerticalMode::CInfiniteVerticalMode()
{
	pLevel = null;
	m_nFloorWidth_TL = 0;
	m_ptGeneratePos_TL.x = 0;
	m_ptGeneratePos_TL.y = 0;
	m_ptGenCursor = m_ptGeneratePos_TL;

	m_dwLastDoorUp_UID = 0;
	//starting floor should be externalized into xml
	m_bFirstDoorUpOnRight = false;
	m_bDoorOnRightCursor = m_bFirstDoorUpOnRight;
	//first floor is already part of the starting level
	m_nLastGenFloor = 1;
}

CInfiniteVerticalMode::~CInfiniteVerticalMode()
{
	Release();
}

bool CInfiniteVerticalMode::Init(CLevel * pLevelPtr, WCHAR * wcsDescriptorPath)
{
	//release first
	Release();
	//set pointer
	pLevel = pLevelPtr;

	WCHAR strPath[MAX_PATH];
	FileManager::GetMediaPath(wcsDescriptorPath, strPath);
	//load descriptors
	pugi::xml_document doc;
	if (!doc.load_file(strPath))
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING] CInfiniteVerticalMode::Init:: Unable to load descriptors list XML:%s", strPath);
		return false;
	}

	//load explosion templates
	pugi::xml_node rootnode = doc.root().child(L"VerticalMode");
	//generic mode data
	shBaseLevelPath.Init(rootnode.attribute(L"sBaseLevelPath").value());
	m_nFloorWidth_TL = rootnode.attribute(L"nFloorWidthTL").as_int();
	m_ptGeneratePos_TL.x = rootnode.attribute(L"nGenOrigX").as_int(); 
	m_ptGeneratePos_TL.y = rootnode.attribute(L"nGenOrigY").as_int();
	m_ptGenCursor = m_ptGeneratePos_TL;
	m_bFirstDoorUpOnRight = false;
	if (rootnode.attribute(L"nLastFloorDoorAlign").as_int() > 0)
		m_bFirstDoorUpOnRight = true;

	//prefabs
	pugi::xml_node prefabsnode = rootnode.child(L"Prefabs");
	for (pugi::xml_node pnode = prefabsnode.first_child(); pnode; pnode = pnode.next_sibling())
	{
		CPrefabDesc* cdesc = new CPrefabDesc();
		//get chapter data
		cdesc->strPath.Init(pnode.attribute(L"sFilename").value());
		cdesc->nWidth_TL = pnode.attribute(L"nWidthTL").as_int();
		cdesc->nFlags = K_IVM_FLOORFLAG_TYPE_MIDDLE;
		int nAlign = pnode.attribute(L"nAlign").as_int();
		if (nAlign < 0)
			cdesc->nFlags = K_IVM_FLOORFLAG_TYPE_LEFT_ENDING;
		else if (nAlign > 0)
			cdesc->nFlags = K_IVM_FLOORFLAG_TYPE_RIGHT_ENDING;

		m_arrPrefabsList.Add(cdesc);
	}

	//actors
	pugi::xml_node actorsnode = rootnode.child(L"Actors");
	for (pugi::xml_node pnode = actorsnode.first_child(); pnode; pnode = pnode.next_sibling())
	{
		CActGenDesc* cdesc = new CActGenDesc();
		//get chapter data
		cdesc->strTemplateName.Init(pnode.attribute(L"sTemplate").value());
		cdesc->nMinFloor = pnode.attribute(L"nStartFloor").as_int();
		cdesc->nMaxFloor = pnode.attribute(L"nEndFloor").as_int();
		cdesc->fProbability = pnode.attribute(L"fProbability").as_float();

		m_arrActorsList.Add(cdesc);
	}

	//first floor is already part of the starting level
	m_nLastGenFloor = 1;

	shLoadedDescriptorPath.Init(wcsDescriptorPath);
	LOG(L"Loaded VInfiniteMode descriptors from [%s]", wcsDescriptorPath);

	return true;
}

void CInfiniteVerticalMode::RestartLevel()
{
	m_ptGenCursor = m_ptGeneratePos_TL;
	m_dwLastDoorUp_UID = 0;
	//starting floor should be externalized into xml
	m_bDoorOnRightCursor = m_bFirstDoorUpOnRight;
	//first floor is already part of the starting level
	m_nLastGenFloor = 1;
}

DWORD CInfiniteVerticalMode::GetFilesCRC(bool bIgnoreMods)
{
	DWORD l_dwCRC = 0;
	if ((pLevel == null) || (shLoadedDescriptorPath.IsEmpty()))
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING] CInfiniteVerticalMode::GetFilesCRC - call Init first!");
		return 1; //just a small amount to break the game's CRC
	}

	WCHAR strPath[MAX_PATH];
	FileManager::GetMediaPath(shLoadedDescriptorPath.text, strPath, bIgnoreMods);
	l_dwCRC += GetFileHash(strPath);

	//compute rest of hash
	FileManager::GetMediaPath(shBaseLevelPath.text, strPath, bIgnoreMods);
	l_dwCRC += GetFileHash(strPath);

	for (int kk = 0; kk < m_arrPrefabsList.GetSize(); kk++)
	{
		FileManager::GetMediaPath(m_arrPrefabsList[kk]->strPath.text, strPath, bIgnoreMods);
		l_dwCRC += GetFileHash(strPath);
	}

	LOG(L"--> Vertical Tower CRC[%08x] bIgnoreMods:%d <--", l_dwCRC, bIgnoreMods);
	return l_dwCRC;
}

const WCHAR* CInfiniteVerticalMode::GetBaseLevelMediaID()
{
	return shBaseLevelPath.text;
}

void CInfiniteVerticalMode::Release()
{
	SAFE_DELETE_GROWABLE_ARRAY(m_arrActorsList);
	SAFE_DELETE_GROWABLE_ARRAY(m_arrPrefabsList);

	pLevel = null;
}

void CInfiniteVerticalMode::Update(float dTime)
{
	if (pLevel == null)
		return;
	
	//can we generate floor without scrolling?
	float	l_fGenerateAbsPosY = pLevel->m_vLevelOrigin.y + m_ptGenCursor.y * K_TILE_SIZE;
	//if we have room for at least 3 floors don't scroll/shred the tileset
	bool    l_bScrollTiles = true;
	if (fabs(l_fGenerateAbsPosY - pLevel->m_levelAABB.y) >= 3 * K_IVM_FLOOR_HEIGHT_TL * K_TILE_SIZE)
		l_bScrollTiles = false;
	//keep a floor or two ahead of our current floor
	bool l_bGenerate = false;
	if (m_nLastGenFloor <= 1 + pLevel->m_arrStats[K_LVL_STATS_LEVEL_VINFINITE_FLOOR])
		l_bGenerate = true;

	if (l_bGenerate)
	{
		GenerateNextHorizontalFloor(l_bScrollTiles);
	}
}

void CInfiniteVerticalMode::GenerateNextHorizontalFloor(bool bScrollLevel)
{
	//Find UID of last door going up 
	if (m_dwLastDoorUp_UID == 0)
	{
		//start from end so we find it faster
		for (int kk = pLevel->m_arrActives.GetSize() - 1; kk >= 0; kk--)
		{
			CActive * pActive = pLevel->m_arrActives[kk];
			if ((pActive->AIstate == K_AI_STATE_ACTIVE_TEAM_TELEPORTER_2FRAMES) && (pActive->pTarget == null))
			{
				m_dwLastDoorUp_UID = pActive->UID;
				break;
			}
		}
	}
	if (m_dwLastDoorUp_UID == 0)
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING] GenerateNextHorizontalFloor: Couldn't find door going up!");
		return;
	}

	int nDoorsAnimIdx = pLevel->m_sprActives.getAnimationIdxByName("DOORS_NO_RETURN");
	//get side flag for next door
	int nDoorSideFlag = K_IVM_FLOORFLAG_TYPE_LEFT_ENDING;
	if (m_bDoorOnRightCursor)
		nDoorSideFlag = K_IVM_FLOORFLAG_TYPE_RIGHT_ENDING;

	//generate
	POINTXY_INT ptGenCursor_TL = m_ptGenCursor;
	ptGenCursor_TL.x += pLevel->m_vLevelOrigin.x / K_TILE_SIZE;
	ptGenCursor_TL.y += pLevel->m_vLevelOrigin.y / K_TILE_SIZE;

	int arrRoomsWidths[32];	//will hold actual room widths
	int nRoomsCnt = GetRoomSizes(m_nFloorWidth_TL, arrRoomsWidths);
	if (nRoomsCnt <= 0)
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING] GenerateNextHorizontalFloor: Couldn't generate rooms sizes!");
		return;
	}
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	LOG(L"-- Floor generated (rooms:%d scroll:%d) --", nRoomsCnt, bScrollLevel);
#endif

	//generate right to left
	for (int kk = 0; kk < nRoomsCnt; kk++)
	{
		int nRoomFlag = K_IVM_FLOORFLAG_TYPE_MIDDLE;
		if (kk == 0)
			nRoomFlag = K_IVM_FLOORFLAG_TYPE_RIGHT_ENDING;
		if (kk == nRoomsCnt - 1)
			nRoomFlag = K_IVM_FLOORFLAG_TYPE_LEFT_ENDING;

		int nPrefabIdx = GetPrefabIdx(nRoomFlag, arrRoomsWidths[kk]);
		if (nPrefabIdx < 0)
		{
			ErrorBox(K_ERR_WARNING, L"[WARNING] GenerateNextHorizontalFloor: Couldn't find prefab for width: %d and flag %d", arrRoomsWidths[kk], nRoomFlag);
			return;
		}

#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
		LOG(L"---- Room[%d] path[%s] --", kk, m_arrPrefabsList[nPrefabIdx]->strPath.text);
#endif
		//save data that helps us find the new door (actives index before next prefab)
		int nActiveCnt_old = pLevel->m_arrActives.GetSize();
		//generate prefab now (with a row lower because we scrolled the tileset)
		WCHAR wcsPath[MAX_PATH];
		FileManager::GetMediaPath(m_arrPrefabsList[nPrefabIdx]->strPath.text, wcsPath);
		g_level.LoadPrefabAtPosition(wcsPath, ptGenCursor_TL.x - m_arrPrefabsList[nPrefabIdx]->nWidth_TL, ptGenCursor_TL.y, false);
		//move cursor
		ptGenCursor_TL.x -= arrRoomsWidths[kk];
		//look for door going up on previous floor so we can link the floors
		if (nRoomFlag == nDoorSideFlag)
		{
			bool bFoundDoor = false;
			for (int ll = nActiveCnt_old; ll < pLevel->m_arrActives.GetSize(); ll++)
			{
				CActive * pDoor = pLevel->m_arrActives[ll];
				if ((pDoor->AIstate == K_AI_STATE_ACTIVE_TEAM_TELEPORTER_2FRAMES) && (pDoor->pTarget == null))
				{
					bFoundDoor = true;
					//found door, link it with the old one below
					CActive * pDoorBelow = pLevel->GetActiveByUID(m_dwLastDoorUp_UID);
					pDoorBelow->bCanInteract = true;
					pDoorBelow->pTarget = pDoor;
					pDoorBelow->targetID_ini = pDoor->UID;
					pDoorBelow->script_hash.Init(L"ACTIVE_TEAM_TELEPORTER_2FRAMES");
					//set anim
					pDoorBelow->nAnim_ini = nDoorsAnimIdx; pDoor->nFrame_ini = 0;
					pDoorBelow->sprite.animationIdx = nDoorsAnimIdx; pDoorBelow->sprite.currentFrame = 0;
					
					//disable top door but set target pointer so it doesn't seem unused
					pDoor->bCanInteract = false;
					pDoor->pTarget = pDoorBelow;
					pDoor->targetID_ini = pDoorBelow->UID;
					pDoor->varAIparams.SetNamedVarString(L"s_ScriptOnOpen", L"VINFINITE_DOOR_OPEN");
					pDoor->varAIparams.SetNamedVarString(L"s_ScriptOnClose", L"VINFINITE_DOOR_SET_NO_RETURN");
					pDoor->nAnim_ini = nDoorsAnimIdx; pDoor->nFrame_ini = 2;
					pDoor->sprite.animationIdx = nDoorsAnimIdx; pDoor->sprite.currentFrame = 2;
					//reset last door UID
					m_dwLastDoorUp_UID = 0;
					//change door going up location
					m_bDoorOnRightCursor = !m_bDoorOnRightCursor;
					
					//increase generation positions
					m_ptGenCursor.y -= K_IVM_FLOOR_HEIGHT_TL;
					m_nLastGenFloor++;

					break;
				}
			}
			if (!bFoundDoor)
			{
				ErrorBox(K_ERR_WARNING, L"[WARNING] GenerateNextHorizontalFloor: Couldn't find spawned door!");
			}
		}
	}

	//look for random enemies and process them
	CFixedArray<CActor*, 128> arrActors;
	//count possible spawns
	DWORD dwRandEnemyName = FastHash(L"ACTOR_RANDOM_ENEMY");
	for (int kk = 0; kk < g_level.m_arrActors.GetSize(); kk++)
	{
		CActor* act = g_level.m_arrActors[kk];
		if (act->templateActor.shName.textHash != dwRandEnemyName)
			continue;
		arrActors.Add(act);
	}
	//scale difficulty
	float fSpawnPercent = 0.6f + (m_nLastGenFloor * 0.025f);
	CLAMP(fSpawnPercent, 0.0f, 1.0f); 
	float fAwareProbability = 0.2f + (m_nLastGenFloor * 0.025);
	CLAMP(fAwareProbability, 0.0f, 0.9f); //keep a small margin so we still have unaware actors
	int nSpawnActors = (int)((float)arrActors.nCount * fSpawnPercent);
	if (nSpawnActors < 1)
		nSpawnActors = 1;

	//shuffle actors list
	for (int kk = 0; kk < 32; kk++)
	{
		int nFrom = pLevel->m_rand.RandInt(arrActors.nCount);
		int nTo = pLevel->m_rand.RandInt(arrActors.nCount);
		if (nFrom != nTo)
			SWAP(arrActors.m_pData[nFrom], arrActors.m_pData[nTo]);
	}

	CStringHash shAwareState(L"AWARE");
	for (int kk = 0; kk < arrActors.nCount; kk++)
	{
		CActor* act = arrActors[kk];
		if (kk < nSpawnActors)
		{
			int nEnemyIdx = GetRandomEnemy(m_nLastGenFloor);
			if (nEnemyIdx >= 0)
			{
				//remove loaded actor
				act->bReleaseIt = true;
				act->bSetHidden = true;
				act->bHidden = true;

				//spawn a new one instead
				CActor* pNewAct = null;
				CActorTemplate* pTemplate = pLevel->GetTemplateActor(m_arrActorsList[nEnemyIdx]->strTemplateName.textHash);
				if (pLevel->m_rand.RandFloat(1.0f) <= fAwareProbability)
					pNewAct = pLevel->SpawnActor(act->pos, pTemplate->shName.text, act->lookDirXsign, &shAwareState);
				else
					pNewAct = pLevel->SpawnActor(act->pos, pTemplate->shName.text, act->lookDirXsign);
				//set ID from deallocated actor
				if (pNewAct != null)
				{
					pNewAct->UID = act->UID;
				}
			}
			else
			{
				//shouldn't get here
				act->bReleaseIt = true;
			}
		}
		else
		{
			//just deallocate the rest
			act->bReleaseIt = true;
		}
	}

	//first scroll level down
	if(bScrollLevel)
		ScrollDownTilesetActiveArea(K_IVM_FLOOR_HEIGHT_TL);
}

bool CInfiniteVerticalMode::ScrollDownTilesetActiveArea(int nRows)
{
	if (pLevel == null)
		return false;

	float fdY = (float)(nRows * K_TILE_SIZE);
	D3DXVECTOR2 vdY(0.0f, fdY);
	
	///--- move tiles ---
	CTile** pNewTiles = null;
	pNewTiles = new CTile*[pLevel->levelSizeTL.w];
	for (int kk = 0; kk < pLevel->levelSizeTL.w; kk++)
	{
		pNewTiles[kk] = new CTile[pLevel->levelSizeTL.h];
	}

	//scroll tiles
	for (int yy = 0; yy < pLevel->levelSizeTL.h; yy++)
	{
		int fromy = yy - nRows;
		if ((fromy < 0) || (fromy >= pLevel->levelSizeTL.h))
			continue;
		//copy row
		for (int xx = 0; xx < pLevel->levelSizeTL.w; xx++)
		{
			pNewTiles[xx][yy] = pLevel->tiles[xx][fromy];
		}
	}

	//delete level tiles
	for (int kk = 0; kk < pLevel->levelSizeTL.w; kk++)
	{
		SAFE_DELETE_ARRAY(pLevel->tiles[kk]);
	}
	SAFE_DELETE_ARRAY(pLevel->tiles);
	//replace pointer with new one
	pLevel->tiles = pNewTiles;

	///--- move level generated collision boxes for level bounds ---
	//first 4 collisions are added by the engine
	for (int kk = 0; kk < 4; kk++)
	{
		CCollisionShape* pCol = pLevel->m_arrColShapes[kk];
		if (pCol->ubFlags & K_LVL_COLLFLAG_LEVEL_BOUNDS)
		{
			//move box, don't use Move() method or it will move touching actors too
			pCol->pos -= vdY;
			pCol->pos_ini -= vdY;
			pCol->bbox.Move(-vdY);
			pCol->bbox_exported = pCol->bbox;
		}
	}

	///--- move level area rectrangle and set camera world limit
	pLevel->m_levelAABB_TL.y -= nRows;
	pLevel->m_levelAABB.Set(pLevel->m_levelAABB_TL.x * K_TILE_SIZE, pLevel->m_levelAABB_TL.y * K_TILE_SIZE, pLevel->m_levelAABB_TL.w * K_TILE_SIZE, pLevel->m_levelAABB_TL.h * K_TILE_SIZE);
	pLevel->m_camLevel.SetWorldBounds(pLevel->m_levelAABB, true, K_CAMTRANS_AXIS_NONE);

	CAABB aabbLevel(pLevel->m_levelAABB);
	int nBottomY = pLevel->m_levelAABB.Bottom();
	///--- deallocate stuff outside the screen ---
	for (int kk = 0; kk < pLevel->m_arrColShapes.GetSize(); kk++)
	{
		CCollisionShape* pCol = pLevel->m_arrColShapes[kk];
		//skip level limits
		if (pCol->ubFlags & K_LVL_COLLFLAG_LEVEL_BOUNDS)
			continue;
		//deallocate if out
		if (pCol->pos.y > nBottomY)
			pCol->bReleaseIt = true;
	}
	///--- objects ---
	for (int kk = 0; kk < pLevel->m_arrActives.GetSize(); kk++)
	{
		CActive* pActiv = pLevel->m_arrActives[kk];
		//deallocate if out
		if (pActiv->bbox.vMin.y > nBottomY)
			pActiv->bReleaseIt = true;
	}
	///--- lights ---
	for (int kk = 0; kk < pLevel->m_arrLights.GetSize(); kk++)
	{
		CLight* pLight = pLevel->m_arrLights[kk];
		//deallocate if out
		if (pLight->bbox.vMin.y > nBottomY)
			pLight->bReleaseIt = true;
	}
	///--- actors ---
	for (int kk = 0; kk < pLevel->m_arrActors.GetSize(); kk++)
	{
		CActor* pAct = pLevel->m_arrActors[kk];
		//deallocate if out
		if (pAct->bbox.vMin.y > nBottomY)
			pAct->bReleaseIt = true;
	}
	///--- decals ---
	for (int kk = pLevel->m_arrDecals.GetSize() - 1; kk >= 0; kk--)
	{
		if (pLevel->m_arrDecals[kk]->aabb.vMin.y > nBottomY)
		{
			SAFE_DELETE(pLevel->m_arrDecals[kk]);
			pLevel->m_arrDecals.Remove(kk);
		}
	}

	return true;
}

int CInfiniteVerticalMode::GetPrefabIdx(int nPrefabFlagFilter /*= K_IVM_FLOORFLAG_NONE*/, int nFixedWidth /*= 0*/)
{
	int nPrefabsCnt = 0;
	int arrFilteredIndices[100];

	for (int kk = 0; kk < m_arrPrefabsList.GetSize(); kk++)
	{
		//filter out prefabs that don't correspond
		if ((nPrefabFlagFilter != K_IVM_FLOORFLAG_NONE) && ((m_arrPrefabsList[kk]->nFlags & nPrefabFlagFilter) == 0))
			continue;
		//requested fixed width?
		if ((nFixedWidth > 0) && (m_arrPrefabsList[kk]->nWidth_TL != nFixedWidth))
			continue;
		//save possible prefabs
		arrFilteredIndices[nPrefabsCnt] = kk;
		nPrefabsCnt++;
	}

	if (nPrefabsCnt == 0)
		return -1;

	int nRetIdx = arrFilteredIndices[pLevel->m_rand.RandInt(nPrefabsCnt)];
	return nRetIdx;
}

int CInfiniteVerticalMode::GetRoomSizes(int nFloorWidth_TL, int ret_arrSizes[32])
{
	//! GENERATES RIGHT TO LEFT ! this is why start is Right

	//save sizes of diffrent types
	CFixedArray<int, 256> arrSizesLeft;
	CFixedArray<int, 256> arrSizesMid;
	CFixedArray<int, 256> arrSizesRight;
	int nStartWLimit = 0;
	int nEndWLimit = 0;
	int nEndMinRoomW = 100;

	for (int kk = 0; kk < m_arrPrefabsList.GetSize(); kk++)
	{
		if (m_arrPrefabsList[kk]->nFlags & K_IVM_FLOORFLAG_TYPE_LEFT_ENDING)
		{
			arrSizesLeft.Add(m_arrPrefabsList[kk]->nWidth_TL);
			if (m_arrPrefabsList[kk]->nWidth_TL < nEndMinRoomW)
				nEndMinRoomW = m_arrPrefabsList[kk]->nWidth_TL;
			if (m_arrPrefabsList[kk]->nWidth_TL > nEndWLimit)
				nEndWLimit= m_arrPrefabsList[kk]->nWidth_TL;
		}
		if (m_arrPrefabsList[kk]->nFlags & K_IVM_FLOORFLAG_TYPE_MIDDLE)
			arrSizesMid.Add(m_arrPrefabsList[kk]->nWidth_TL);
		if (m_arrPrefabsList[kk]->nFlags & K_IVM_FLOORFLAG_TYPE_RIGHT_ENDING)
		{
			arrSizesRight.Add(m_arrPrefabsList[kk]->nWidth_TL);
			if (m_arrPrefabsList[kk]->nWidth_TL > nStartWLimit)
				nStartWLimit = m_arrPrefabsList[kk]->nWidth_TL;
		}
	}
	//--- find random combination ---
	bool bFound = false;
	int	nWidthLeft = nFloorWidth_TL;
	int nRoomsCnt = 0;

	while (!bFound)
	{
		if(nRoomsCnt == 0)
			ret_arrSizes[nRoomsCnt] = arrSizesRight.m_pData[pLevel->m_rand.RandInt(arrSizesRight.nCount)];
		else
			ret_arrSizes[nRoomsCnt] = arrSizesMid.m_pData[pLevel->m_rand.RandInt(arrSizesMid.nCount)];

		nWidthLeft -= ret_arrSizes[nRoomsCnt];
		nRoomsCnt++;
		//finished?
		if (nWidthLeft == 0)
		{
			//final room too large?
			if (ret_arrSizes[nRoomsCnt - 1] > nEndWLimit)
			{
				nRoomsCnt = 0;
				nWidthLeft = nFloorWidth_TL;
			}
			else
			{
				bFound = true;
			}
		}
		else if (nWidthLeft < nEndMinRoomW)
		{
			nRoomsCnt = 0;
			nWidthLeft = nFloorWidth_TL;
		}
		//first room too large?
		if ((nRoomsCnt - 1 == 0) && (ret_arrSizes[nRoomsCnt - 1] > nStartWLimit))
		{
			nRoomsCnt = 0;
			nWidthLeft = nFloorWidth_TL;
		}

		//failsafe
		if (nRoomsCnt >= 32)
			return 0;
	}

	return nRoomsCnt;
}

int CInfiniteVerticalMode::GetRandomEnemy(int nFloor)
{
	int arrIdx[512];
	float arrProb[512];
	int arrIdxCnt = 0;

	for (int kk = 0; kk < m_arrActorsList.GetSize(); kk++)
	{
		if (nFloor < m_arrActorsList[kk]->nMinFloor)
			continue;
		if ((m_arrActorsList[kk]->nMaxFloor > 0) && (nFloor > m_arrActorsList[kk]->nMaxFloor))
			continue;

		arrIdx[arrIdxCnt] = kk;
		arrProb[arrIdxCnt] = m_arrActorsList[kk]->fProbability;
		arrIdxCnt++;
	}

	if (arrIdxCnt == 0)
		return -1;

	int nRetArrIdx = pLevel->m_rand.GetProbabilityFromDomain(arrProb, arrIdxCnt);
	if (nRetArrIdx < 0)
		return -1;
	return arrIdx[nRetArrIdx];
}

