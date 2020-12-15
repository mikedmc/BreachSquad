#include "dxstdafx.h"
#include "TileBlockMesh.h"

CTileBlockMesh::CTileBlockMesh()
{
	m_Painter.Init(512);
	memset(m_arrMeshIdx, -1, sizeof(int) * ARRAY_SIZE(m_arrMeshIdx));
	m_ShadowMeshIdx = -1;
}

CTileBlockMesh::CTileBlockMesh(PDEVICE pDevice)
{
	m_Painter.Init(512, pDevice);
	memset(m_arrMeshIdx, -1, sizeof(int) * ARRAY_SIZE(m_arrMeshIdx));
	m_ShadowMeshIdx = -1;
}

CTileBlockMesh::~CTileBlockMesh()
{
	Clear();
}

OPRESULT CTileBlockMesh::BuildBuffers(POINTXY_INT vBlockPos_TL, CTile** map, SIZEWH mapSizeTL, Vec2 vOffset, CSpriteCollection* pLightsSpr)
{
	// allocate maximum possible number per layer plus sentinel
	_VERTEX_PNCT4T4 arrVerts[K_TBM_BLOCK_W * K_TBM_BLOCK_H * 4 + 16];
	int nCur = 0;

	m_mapAreaTL.Set(vBlockPos_TL.x, vBlockPos_TL.y, K_TBM_BLOCK_W, K_TBM_BLOCK_H);
	// limits bbox to valid map area
	if (vBlockPos_TL.x < 0) m_mapAreaTL.x = 0;
	if (vBlockPos_TL.y < 0) m_mapAreaTL.y = 0;
	if (m_mapAreaTL.x + m_mapAreaTL.w >= mapSizeTL.w)
		m_mapAreaTL.w -= m_mapAreaTL.x + m_mapAreaTL.w - mapSizeTL.w;
	if (m_mapAreaTL.y + m_mapAreaTL.h >= mapSizeTL.h)
		m_mapAreaTL.h -= m_mapAreaTL.y + m_mapAreaTL.h - mapSizeTL.h;
	// set bbox in world coords
	m_bbox.Set(vOffset.x + (float)(m_mapAreaTL.x * K_TILE_SIZE), vOffset.y + (float)(m_mapAreaTL.y * K_TILE_SIZE),
		vOffset.x + (float)(m_mapAreaTL.x + m_mapAreaTL.w) * K_TILE_SIZE, vOffset.y + (float)(m_mapAreaTL.y + m_mapAreaTL.h) * K_TILE_SIZE);

	bool bIsEmpty = true;

	Vec2 vOrig(vOffset.x + m_bbox.vMin.x, vOffset.y + m_bbox.vMin.y);
	//#TODO: create all meshes in a single pass with many open meshes
	for (int lay = 0; lay < K_TILE_LAYERS_CNT; lay++)
	{
		if (OP_SUCCESS(m_Painter.BeginMesh(m_arrMeshIdx[lay])))
		{
			nCur = 0;
			for (int yy = 0; yy < m_mapAreaTL.h; yy++)
			{
				for (int xx = 0; xx < m_mapAreaTL.w; xx++)
				{
					CTile* tl = &map[m_mapAreaTL.x + xx][m_mapAreaTL.y + yy];
					// skip empty tiles
					if (tl->tileIDs[lay] < 0)
						continue;
					// add geometry
					SET_PNCT4T4(&arrVerts[nCur++], Vec3(vOrig.x + xx * K_TILE_SIZE, vOrig.y + yy * K_TILE_SIZE, 0.0f),
						Vec3(0.0f, 0.0f, 1.0f), 0xffffffff,
						Vec4(tl->vUVmin[lay].x, tl->vUVmin[lay].y, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 0.0f));
					SET_PNCT4T4(&arrVerts[nCur++], Vec3(vOrig.x + (xx + 1) * K_TILE_SIZE, vOrig.y + yy * K_TILE_SIZE, 0.0f),
						Vec3(0.0f, 0.0f, 1.0f), 0xffffffff,
						Vec4(tl->vUVmax[lay].x, tl->vUVmin[lay].y, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 0.0f));
					SET_PNCT4T4(&arrVerts[nCur++], Vec3(vOrig.x + (xx + 1) * K_TILE_SIZE, vOrig.y + (yy + 1) * K_TILE_SIZE, 0.0f),
						Vec3(0.0f, 0.0f, 1.0f), 0xffffffff,
						Vec4(tl->vUVmax[lay].x, tl->vUVmax[lay].y, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 0.0f));
					SET_PNCT4T4(&arrVerts[nCur++], Vec3(vOrig.x + xx * K_TILE_SIZE, vOrig.y + (yy + 1) * K_TILE_SIZE, 0.0f),
						Vec3(0.0f, 0.0f, 1.0f), 0xffffffff,
						Vec4(tl->vUVmin[lay].x, tl->vUVmax[lay].y, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 0.0f));
				}
			}

			m_Painter.AddQuads(arrVerts, nCur / 4);
			int nQuads = m_Painter.EndMesh();

			LOG("map lay:%d pos:[%d,%d] WH:[%d,%d] quads:%d", lay, m_mapAreaTL.x, m_mapAreaTL.y, m_mapAreaTL.w, m_mapAreaTL.h, nQuads);
			if (nQuads > 0)
				bIsEmpty = false;				// we have at least some tris so block is not empty
			else
				m_arrMeshIdx[lay] = -1;			// make sure we don't even call draw if layer is empty
		}
	}

	///--- build shadow buffer ---
	if (OP_SUCCESS(m_Painter.BeginMesh(m_ShadowMeshIdx)))
	{
		nCur = 0;
		for (int yy = 0; yy < m_mapAreaTL.h; yy++)
		{
			for (int xx = 0; xx < m_mapAreaTL.w; xx++)
			{
				CTile* tl = &map[m_mapAreaTL.x + xx][m_mapAreaTL.y + yy];
				// skip non shadowed tiles
				if (tl->nShadowFrame < 0)
					continue;
				// get shadow tex coords
				RECTLTRB_F texrect = pLightsSpr->GetModuleRect_TexCoords(ANM_LIGHTS_SPR_SHADOWS, tl->nShadowFrame, 0);
				// add geometry (Clockwise)
				SET_PNCT4T4(&arrVerts[nCur++], Vec3(vOrig.x + xx * K_TILE_SIZE, vOrig.y + yy * K_TILE_SIZE, 0.0f),
					Vec3(0.0f, 0.0f, 1.0f), 0xffffffff,
					Vec4(texrect.left, texrect.top, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 0.0f));
				SET_PNCT4T4(&arrVerts[nCur++], Vec3(vOrig.x + (xx + 1) * K_TILE_SIZE, vOrig.y + yy * K_TILE_SIZE, 0.0f),
					Vec3(0.0f, 0.0f, 1.0f), 0xffffffff,
					Vec4(texrect.right, texrect.top, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 0.0f));
				SET_PNCT4T4(&arrVerts[nCur++], Vec3(vOrig.x + (xx + 1) * K_TILE_SIZE, vOrig.y + (yy + 1) * K_TILE_SIZE, 0.0f),
					Vec3(0.0f, 0.0f, 1.0f), 0xffffffff,
					Vec4(texrect.right, texrect.bottom, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 0.0f));
				SET_PNCT4T4(&arrVerts[nCur++], Vec3(vOrig.x + xx * K_TILE_SIZE, vOrig.y + (yy + 1) * K_TILE_SIZE, 0.0f),
					Vec3(0.0f, 0.0f, 1.0f), 0xffffffff,
					Vec4(texrect.left, texrect.bottom, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 0.0f));
			}
		}

		m_Painter.AddQuads(arrVerts, nCur / 4);
		int nQuads = m_Painter.EndMesh();

		LOG("shadow quads:%d", nQuads);
		if (nQuads > 0)
			bIsEmpty = false;				// we have at least some tris so block is not empty
		else
			m_ShadowMeshIdx = -1;			// make sure we don't even call draw if layer is empty
	}


	if (bIsEmpty)
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_LOG, L"Empty block detected! pos:%d,%d ", m_mapAreaTL.x, m_mapAreaTL.y);
	}
	else
	{
		m_Painter.BuildBuffers();
	}

	return K_OP_OK;
}

void CTileBlockMesh::Clear()
{
	memset(m_arrMeshIdx, -1, sizeof(int) * ARRAY_SIZE(m_arrMeshIdx));
	m_Painter.ClearBuffers();
	m_ShadowMeshIdx = -1;
}

void CTileBlockMesh::PaintLayer(int nLayer, bool bSetFVF /*= false*/)
{
	_ASSERT((nLayer >= 0) && (nLayer < K_TBM_MAX_LAYERS));
	m_Painter.DrawMesh(m_arrMeshIdx[nLayer], bSetFVF);
}

void CTileBlockMesh::PaintShadowLayer(bool bSetFVF /*= false*/)
{
	m_Painter.DrawMesh(m_ShadowMeshIdx, bSetFVF);
}

///----------------------------------------------
/// TILE BLOCK MANAGER class
///----------------------------------------------

CTileBlockMeshManager::CTileBlockMeshManager() :
	m_pDevice(null)
{

}

CTileBlockMeshManager::~CTileBlockMeshManager()
{
	Release();
}

void CTileBlockMeshManager::Release()
{
	arrVisible.Clear();
	SAFE_DELETE_GROWABLE_ARRAY(arrBlocks);
}

OPRESULT CTileBlockMeshManager::BuildBuffers(CTile** map, SIZEWH mapSizeTL, Vec2 vOffset, CSpriteCollection* pLightsSpr)
{
	if (map == nullptr)
		return OPRESULT(K_OP_INVALIDARGS, L"BuildBuffers:: Map param is null!", K_SEVERITY_WARNING);

	// parse map block by block
	int blocksX = (mapSizeTL.w / K_TBM_BLOCK_W) + (((mapSizeTL.w % K_TBM_BLOCK_W) > 0) ? 1 : 0);
	int blocksY = (mapSizeTL.h / K_TBM_BLOCK_H) + (((mapSizeTL.h % K_TBM_BLOCK_H) > 0) ? 1 : 0);

	for (int blY = 0; blY < blocksY; blY++)
	{
		for (int blX = 0; blX < blocksX; blX++)
		{
			// block is allocated now so it missed device creation. Set device pointer and create needed buffers now
			CTileBlockMesh* tbm = new CTileBlockMesh(m_pDevice);

			if (OP_FAILED(tbm->BuildBuffers(POINTXY_INT(blX * K_TBM_BLOCK_W, blY * K_TBM_BLOCK_H), map, mapSizeTL, vOffset, pLightsSpr)))
			{
				LOG("Block NOT added!");
				delete tbm;
				continue;
			}
			// add block if it could be created
			LOG("Block added!");
			arrBlocks.Add(tbm);
		}
	}

	return K_OP_OK;
}

int CTileBlockMeshManager::UpdateVisibility(RECTXYWH_F camRect)
{
	arrVisible.Clear();
	for (int kk = 0; kk < arrBlocks.GetSize(); kk++)
	{
		CAABB camAABB(camRect);
		if (camAABB.Intersects(&arrBlocks[kk]->m_bbox))
			arrVisible.Add(arrBlocks[kk]);
	}
	return arrVisible.Count();
}

OPRESULT CTileBlockMeshManager::PaintLayer(int layerIdx)
{
	if (arrVisible.Count() <= 0)
		return OPRESULT(K_OP_FAILED, L"No visible blocks to paint!", K_SEVERITY_NONE);

	for (int kk = 0; kk < arrVisible.Count() ; kk++)
	{
		CTileBlockMesh* tbm = arrVisible[kk];
		tbm->PaintLayer(layerIdx, true);
	}

	return K_OP_OK;
}

OPRESULT CTileBlockMeshManager::PaintShadowLayer()
{
	if (arrVisible.Count() <= 0)
		return OPRESULT(K_OP_FAILED, L"No visible blocks to paint!", K_SEVERITY_NONE);

	for (int kk = 0; kk < arrVisible.Count(); kk++)
	{
		arrVisible[kk]->PaintShadowLayer(true);
	}

	return K_OP_OK;
}

OPRESULT CTileBlockMeshManager::OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc /*= NULL*/, void* pUserContext /*= NULL*/)
{
	m_pDevice = pDevice;
	for (int kk = 0; kk < arrBlocks.GetSize(); kk++)
	{
		V_OP_RET(arrBlocks[kk]->m_Painter.OnCreateDevice(pDevice));
	}
	return K_OP_OK;
}

OPRESULT CTileBlockMeshManager::OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc /*= NULL*/, void* pUserContext /*= NULL*/)
{
	m_pDevice = pDevice;
	for (int kk = 0; kk < arrBlocks.GetSize(); kk++)
	{
		V_OP_RET(arrBlocks[kk]->m_Painter.OnResetDevice(pDevice));
	}
	return K_OP_OK;
}

OPRESULT CTileBlockMeshManager::OnLostDevice(void* pUserContext /*= NULL*/)
{
	for (int kk = 0; kk < arrBlocks.GetSize(); kk++)
	{
		arrBlocks[kk]->m_Painter.OnLostDevice();
	}
	return K_OP_OK;
}

OPRESULT CTileBlockMeshManager::OnDestroyDevice(void* pUserContext /*= NULL*/)
{
	for (int kk = 0; kk < arrBlocks.GetSize(); kk++)
	{
		arrBlocks[kk]->m_Painter.OnDestroyDevice();
	}
	return K_OP_OK;
}
