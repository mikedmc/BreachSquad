#include "dxstdafx.h"
#include "TileBlockMesh.h"

CTileBlockMesh::CTileBlockMesh()
{
	m_Painter.Init(512);
	memset(m_arrMeshIdx, -1, sizeof(int));
}

CTileBlockMesh::~CTileBlockMesh()
{
}

OPRESULT CTileBlockMesh::BuildBuffers(POINTXY_INT vBlockPos_TL, CTile** map, SIZEWH mapSizeTL, Vec2 vLevelOrigin)
{
	// allocate maximum possible number
	//_VERTEX_PNCT4T4 arrVerts[K_TBM_BLOCK_W * K_TBM_BLOCK_H * 6];

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
			for (int yy = 0; yy < m_bboxTL.h; yy++)
			{
				for (int xx = 0; xx < m_bboxTL.w; xx++)
				{
					CTile* tl = &map[m_bboxTL.x + xx][m_bboxTL.y + yy];
					// add geometry
				}
			}

			int nTris = m_Painter.EndMesh();
			if (nTris > 0)
				bIsEmpty = false;				// we have at least some tris so block is not empty
			else
				m_arrMeshIdx[lay] = -1;			// make sure we don't even call draw if layer is empty
		}
	}

	if (bIsEmpty)
	{
		return OPRESULT(K_OP_FAILED, L"Empty block detected.", K_SEVERITY_WARNING);
	}

	return K_OP_OK;
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
	int blocksX = (mapSizeTL.w / K_TBM_BLOCK_W) + ((mapSizeTL.w % K_TBM_BLOCK_W) > 0) ? 1 : 0;
	int blocksY = (mapSizeTL.h / K_TBM_BLOCK_H) + ((mapSizeTL.h % K_TBM_BLOCK_H) > 0) ? 1 : 0;

	for (int blY = 0; blY < blocksY; blY++)
	{
		for (int blX = 0; blX < blocksX; blX++)
		{
			CTileBlockMesh* tbm = new CTileBlockMesh();
			if (OP_FAILED(tbm->BuildBuffers(POINTXY_INT(blX * K_TBM_BLOCK_W, blY * K_TBM_BLOCK_H), map, mapSizeTL, vLevelOrigin)))
			{
				delete tbm;
				continue;
			}
			// add block if it could be created
			arrBlocks.Add(tbm);
		}
	}

	return K_OP_OK;
}

OPRESULT CTileBlockMeshManager::OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc /*= NULL*/, void* pUserContext /*= NULL*/)
{
	return K_OP_OK;
}

OPRESULT CTileBlockMeshManager::OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc /*= NULL*/, void* pUserContext /*= NULL*/)
{
	return K_OP_OK;
}

OPRESULT CTileBlockMeshManager::OnLostDevice(void* pUserContext /*= NULL*/)
{
	return K_OP_OK;
}

OPRESULT CTileBlockMeshManager::OnDestroyDevice(void* pUserContext /*= NULL*/)
{
	return K_OP_OK;
}
