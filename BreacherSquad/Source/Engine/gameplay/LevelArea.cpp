#include "dxstdafx.h"
#include "LevelArea.h"

CLevelArea::CLevelArea(UINT32 nID)
{
	ID = nID;
	tiles = nullptr;
	bVisible = false;
}

CLevelArea::~CLevelArea()
{
	Release();
}

OPRESULT CLevelArea::PostConstructionInit(PDEVICE pDevice, CSpriteCollection* pLightsSprCol)
{
	_ASSERT(pDevice != nullptr && pLightsSprCol != nullptr);

	areaMesh.Init(pDevice);
	V_OP_RET(areaMesh.BuildBuffers(tiles, sizeTL, Vec2(AABBbounds.vMin.x, AABBbounds.vMin.y), pLightsSprCol));

	return K_OP_OK;
}

void CLevelArea::Release()
{
	// release tiles
	if (tiles != nullptr)
	{
		for (int kk = 0; kk < sizeTL.w; kk++)
		{
			SAFE_DELETE_ARRAY(tiles[kk]);
		}
		SAFE_DELETE_ARRAY(tiles);
	}

	areaMesh.Release();
}

CTile* CLevelArea::GetTile(int x, int y)
{
	//#TODO: should return a generic empty tile??
	if ((x < AABBbounds_TL.x) || (y < AABBbounds_TL.y) || (x >= AABBbounds_TL.x + AABBbounds_TL.w) || (y >= AABBbounds_TL.y + AABBbounds_TL.h))
		return nullptr;

	return &tiles[x][y];
}

bool CLevelArea::UpdateVisibility(RECTXYWH_F camRect)
{
	CAABB camAABB(camRect);
	if (camAABB.Intersects(&camAABB))
		bVisible = true;
	else
		bVisible = false;
	// check mesh visibility (level blocks)
	areaMesh.UpdateVisibility(camRect);

	return bVisible;
}

OPRESULT CLevelArea::OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc /*= NULL*/, void* pUserContext /*= NULL*/)
{
	V_OP_RET(areaMesh.OnCreateDevice(pDevice, pBBDesc, pUserContext));
	return K_OP_OK;
}

OPRESULT CLevelArea::OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc /*= NULL*/, void* pUserContext /*= NULL*/)
{
	V_OP_RET(areaMesh.OnCreateDevice(pDevice, pBBDesc, pUserContext));
	return K_OP_OK;
}

OPRESULT CLevelArea::OnLostDevice(void* pUserContext /*= NULL*/)
{
	areaMesh.OnLostDevice();
	return K_OP_OK;
}

OPRESULT CLevelArea::OnDestroyDevice(void* pUserContext /*= NULL*/)
{
	areaMesh.OnDestroyDevice();
	return K_OP_OK;
}

