#include "dxstdafx.h"
#include "TileBlockMesh.h"

CTileBlockMesh::CTileBlockMesh()
{
	m_Painter.Init(512);
	memset(m_arrMeshIdx, -1, sizeof(int) * ARRAY_SIZE(m_arrMeshIdx));
}

CTileBlockMesh::~CTileBlockMesh()
{
	Clear();
}

OPRESULT CTileBlockMesh::BuildBuffers(POINTXY_INT vBlockPos_TL, CTile** map, SIZEWH mapSizeTL, Vec2 vLevelOrigin)
{
	// allocate maximum possible number per layer plus sentinel
	_VERTEX_PNCT4T4 arrVerts[K_TBM_BLOCK_W * K_TBM_BLOCK_H * 4 + 16];
	int nCur = 0;

	m_bboxTL.Set(vBlockPos_TL.x, vBlockPos_TL.y, K_TBM_BLOCK_W, K_TBM_BLOCK_H);
	// limits bbox to valid map area
	if (vBlockPos_TL.x < 0) m_bboxTL.x = 0;
	if (vBlockPos_TL.y < 0) m_bboxTL.y = 0;
	if (m_bboxTL.x + m_bboxTL.w >= mapSizeTL.w)
		m_bboxTL.w -= m_bboxTL.x + m_bboxTL.w - mapSizeTL.w;
	if (m_bboxTL.y + m_bboxTL.h >= mapSizeTL.h)
		m_bboxTL.h -= m_bboxTL.y + m_bboxTL.h - mapSizeTL.h;
	// set bbox in world coords
	m_bbox.Set((float)(m_bboxTL.x * K_TILE_SIZE), (float)(m_bboxTL.y * K_TILE_SIZE), 
		(float)(m_bboxTL.x + m_bboxTL.w) * K_TILE_SIZE, (float)(m_bboxTL.y + m_bboxTL.h) * K_TILE_SIZE);

	bool bIsEmpty = true;

	for (int lay = 0; lay < K_LVL_LAYERS_CNT; lay++)
	{
		if (OP_SUCCESS(m_Painter.BeginMesh(m_arrMeshIdx[lay])))
		{
			nCur = 0;
			for (int yy = 0; yy < m_bboxTL.h; yy++)
			{
				for (int xx = 0; xx < m_bboxTL.w; xx++)
				{
					CTile* tl = &map[m_bboxTL.x + xx][m_bboxTL.y + yy];
					// skip empty tiles
					if (tl->tileIDs[lay] < 0)
						continue;
					// add geometry
					SET_PNCT4T4(&arrVerts[nCur++], Vec3(m_bbox.vMin.x + xx * K_TILE_SIZE, m_bbox.vMin.y + yy * K_TILE_SIZE, 0.0f), 
						Vec3(0.0f, 0.0f, 1.0f), 0xffffffff,
						Vec4(tl->vUVmin[lay].x, tl->vUVmin[lay].y, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 0.0f));
					SET_PNCT4T4(&arrVerts[nCur++], Vec3(m_bbox.vMin.x + (xx + 1) * K_TILE_SIZE, m_bbox.vMin.y + yy * K_TILE_SIZE, 0.0f),
						Vec3(0.0f, 0.0f, 1.0f), 0xffffffff,
						Vec4(tl->vUVmax[lay].x, tl->vUVmin[lay].y, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 0.0f));
					SET_PNCT4T4(&arrVerts[nCur++], Vec3(m_bbox.vMin.x + (xx + 1) * K_TILE_SIZE, m_bbox.vMin.y + (yy + 1) * K_TILE_SIZE, 0.0f),
						Vec3(0.0f, 0.0f, 1.0f), 0xffffffff,
						Vec4(tl->vUVmax[lay].x, tl->vUVmax[lay].y, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 0.0f));
					SET_PNCT4T4(&arrVerts[nCur++], Vec3(m_bbox.vMin.x + xx * K_TILE_SIZE, m_bbox.vMin.y + (yy + 1) * K_TILE_SIZE, 0.0f),
						Vec3(0.0f, 0.0f, 1.0f), 0xffffffff,
						Vec4(tl->vUVmin[lay].x, tl->vUVmax[lay].y, 0.0f, 0.0f), Vec4(0.0f, 0.0f, 0.0f, 0.0f));
				}
			}

			m_Painter.AddQuads(arrVerts, nCur / 4);
			int nQuads = m_Painter.EndMesh();

			LOG("map lay:%d pos:[%d,%d] WH:[%d,%d] quads:%d", lay, m_bboxTL.x, m_bboxTL.y, m_bboxTL.w, m_bboxTL.h, nQuads);
			if (nQuads > 0)
				bIsEmpty = false;				// we have at least some tris so block is not empty
			else
				m_arrMeshIdx[lay] = -1;			// make sure we don't even call draw if layer is empty
		}
	}

	if (bIsEmpty)
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_LOG, L"Empty block detected! pos:%d,%d ", m_bboxTL.x, m_bboxTL.y);
	}
	else
	{
		//m_Painter.BuildBuffers();
	}

	return K_OP_OK;
}

void CTileBlockMesh::Clear()
{
	memset(m_arrMeshIdx, -1, sizeof(int) * ARRAY_SIZE(m_arrMeshIdx));
	m_Painter.ClearBuffers();
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
	SAFE_DELETE_GROWABLE_ARRAY(arrBlocks);
}

OPRESULT CTileBlockMeshManager::BuildBuffers(CTile** map, SIZEWH mapSizeTL, Vec2 vLevelOrigin)
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
			CTileBlockMesh* tbm = new CTileBlockMesh();
			if (OP_FAILED(tbm->BuildBuffers(POINTXY_INT(blX * K_TBM_BLOCK_W, blY * K_TBM_BLOCK_H), map, mapSizeTL, vLevelOrigin)))
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

OPRESULT CTileBlockMeshManager::OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc /*= NULL*/, void* pUserContext /*= NULL*/)
{
	for (int kk = 0; kk < arrBlocks.GetSize(); kk++)
	{
		V_OP_RET(arrBlocks[kk]->m_Painter.OnCreateDevice(pDevice, pBBDesc));
	}
	return K_OP_OK;
}

OPRESULT CTileBlockMeshManager::OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc /*= NULL*/, void* pUserContext /*= NULL*/)
{
	for (int kk = 0; kk < arrBlocks.GetSize(); kk++)
	{
		V_OP_RET(arrBlocks[kk]->m_Painter.OnResetDevice(pDevice, pBBDesc));
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
