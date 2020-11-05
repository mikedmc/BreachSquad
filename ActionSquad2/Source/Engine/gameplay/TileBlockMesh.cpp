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
		return OPRESULT(K_OP_INVALIDARGS, L"BuildBuffers:: Map param is null!", K_OP_SEVERITY_WARNING);

	return K_OP_OK;
}
