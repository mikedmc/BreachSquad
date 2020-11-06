#include "dxstdafx.h"
#include "BufferedPainterQuad.h"
///-----------------------------------------------------------------------------------------------
///	 BUFFERED PAINTER QUADS
///  - adaugi triunghiuri unul cate unul si la final face un VB si IB din care poti desena mesh-ul
///-----------------------------------------------------------------------------------------------

CBufferedPainterQuads::CBufferedPainterQuads() :
	m_vb(null), m_ib(null), m_pDevice(null),
	m_nMeshesCnt(0), m_nVertexCursor(0), m_bMeshStarted(false),
	m_verts(null), m_nMaxQuadsCnt(0)
{
}

CBufferedPainterQuads::~CBufferedPainterQuads(void)
{
	ClearBuffers();
	SAFE_DELETE_ARRAY(m_verts);

	SAFE_RELEASE(m_vb);
	SAFE_RELEASE(m_ib);

	m_nMaxQuadsCnt = 0;
}

void CBufferedPainterQuads::Init(int nMaxQuadsCnt)
{
	m_nMaxQuadsCnt = nMaxQuadsCnt;
	m_verts = new _VERTEX_PNCT4T4[(m_nMaxQuadsCnt + K_BP_SENTINEL_QUADS) * 4];
}

OPRESULT CBufferedPainterQuads::BeginMesh(int &retMeshIdx)
{
	if (m_bMeshStarted)
	{
		EndMesh();
		ErrorBox(K_ERR_LOG, L"CBufferedPainterQuads:: A mesh is already started. Closing mesh and starting another.");
		// closes the mesh automatically, no return value
	}

	if (m_nMeshesCnt >= K_BP_MAX_MESHES_CNT)
	{
		retMeshIdx = -1;
		return OPRESULT(K_OP_FAILED, L"CBufferedPainterQuads:: Too many meshes!", K_SEVERITY_WARNING);
	}
	//assert(m_nMeshesCnt < K_BP_MAX_MESHES_CNT);
	m_bMeshStarted = true;

	//reset mesh data
	m_nQuadsPerMesh[m_nMeshesCnt] = 0;
	m_nQuadsOffsets[m_nMeshesCnt] = 0;
	//return mesh idx
	retMeshIdx = m_nMeshesCnt;

	return K_OP_OK;
}

OPRESULT CBufferedPainterQuads::AddQuads(_VERTEX_PNCT4T4 *points, int quadsCount)
{
	assert(m_nMaxQuadsCnt > 0);

	if (!m_bMeshStarted)
		return OPRESULT(K_OP_FAILED, L"You have to call BeginMesh() first!", K_SEVERITY_WARNING);

#if defined(_DEBUG) || defined(DEBUG)
	assert(m_nVertexCursor + quadsCount * 4 < m_nMaxQuadsCnt * 4);
#endif

	memcpy(&m_verts[m_nVertexCursor], points, sizeof(_VERTEX_PNCT4T4) * 4 * quadsCount);

	m_nVertexCursor += 4 * quadsCount;
	m_nQuadsPerMesh[m_nMeshesCnt] += quadsCount;

	return K_OP_OK;
}

int CBufferedPainterQuads::EndMesh()
{
	//make sure we close any pending meshes
	if (m_bMeshStarted == false)
		return 0;

	m_bMeshStarted = false;

	int nQuads = 0;
	if (m_nMeshesCnt == 0) //is it the first mesh?
	{
		m_nQuadsOffsets[m_nMeshesCnt] = 0;
	}
	else
	{
		m_nQuadsOffsets[m_nMeshesCnt] = m_nQuadsOffsets[m_nMeshesCnt - 1] + m_nQuadsPerMesh[m_nMeshesCnt - 1];
	}
	nQuads = m_nQuadsPerMesh[m_nMeshesCnt];
	//get to next mesh
	m_nMeshesCnt++;

	return nQuads;
}

OPRESULT CBufferedPainterQuads::ClearBuffers()
{
	//reset all counters
	m_nMeshesCnt = 0;
	m_bMeshStarted = false;

	m_nVertexCursor = 0;
			  
	return K_OP_OK;
}

OPRESULT CBufferedPainterQuads::BuildBuffers()
{
	EndMesh();

	if (m_vb == nullptr)
		return OPRESULT(K_OP_FAILED, L"CBufferedPainterQuads::BuildBuffers(): VB is null!", K_SEVERITY_WARNING);

	if (m_nVertexCursor == 0)
		return K_OP_OK;

	HRESULT hr = S_OK;
	//write verts to VB
	_VERTEX_PNCT4T4* pVerts;
	if (FAILED(m_vb->Lock(0, m_nVertexCursor * sizeof(_VERTEX_PNCT4T4), (void**)&pVerts, D3DLOCK_DISCARD)))
	{
		return OPRESULT(K_OP_FAILED, L"[ERROR] CBufferedPainterQuads: Build buffers failed!", K_SEVERITY_WARNING);
	}

	memcpy(pVerts, m_verts, m_nVertexCursor * sizeof(_VERTEX_PNCT4T4));

	m_vb->Unlock();

	return K_OP_OK;
}

OPRESULT CBufferedPainterQuads::DrawMesh(int meshIdx, bool setFVF)
{
	HRESULT hr = S_OK;

	//empty mesh: exit
	if (meshIdx < 0)
	{
		// this happens often if light touches no shadow casters. Logging not necessary.
		//LOG(L"!!! WARNING: CBufferedPainterQuads::DrawMesh called with mesh idx = -1\n");
		return K_OP_INVALIDARGS;
	}

	if (meshIdx >= m_nMeshesCnt)
	{
		LOG(L"Trying to draw a mesh that doesn't exist! Input idx=%d meshesCnt=%d", meshIdx, m_nMeshesCnt);
		return K_OP_FAILED;
	}
	if (m_nQuadsPerMesh[meshIdx] == 0)
		return K_OP_OK;
	
	if(setFVF)
		m_pDevice->SetFVF(_VERTEX_PNCT4T4::FVF);

	m_pDevice->SetStreamSource(0, m_vb, 0, sizeof(_VERTEX_PNCT4T4));
	m_pDevice->SetIndices(m_ib);

	if (FAILED(m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, m_nQuadsOffsets[meshIdx] * 4, 0, m_nQuadsPerMesh[meshIdx] * 4, 0, m_nQuadsPerMesh[meshIdx] * 2)))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CBufferedPainterQuads::DrawMesh failed(%d, %d, %d)!", m_nQuadsOffsets[meshIdx], m_nQuadsPerMesh[meshIdx], m_nQuadsPerMesh[meshIdx]);
	}
	
	return K_OP_OK;
}

const int CBufferedPainterQuads::GetTrisCount(int meshIdx) const
{
	if ((meshIdx < 0) || (meshIdx >= m_nMeshesCnt))
	{
		return 0;
	}

	return m_nQuadsPerMesh[meshIdx] * 2;
}

//--- framework ---
OPRESULT CBufferedPainterQuads::OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	assert(m_nMaxQuadsCnt > 0);

	HRESULT hr = S_OK;

	m_pDevice = pDevice;
	//create index buffer (fixed) - deci va desena numai triunghiuri independente
	if (FAILED(m_pDevice->CreateIndexBuffer((m_nMaxQuadsCnt + K_BP_SENTINEL_QUADS) * 6 * sizeof(DWORD), 0, D3DFMT_INDEX32, D3DPOOL_MANAGED, &m_ib, 0)))
	{
		return OPRESULT(K_OP_FAILED, L"[ERROR] CBufferedPainterQuads: Create Index Buffer failed!", K_SEVERITY_WARNING);
	}
	//lock and fill
	DWORD * pIndices;
	if (FAILED(m_ib->Lock(0, NULL, (void**)&pIndices, 0)))
	{
		return OPRESULT(K_OP_FAILED, L"[ERROR] CBufferedPainterQuads: Lock Index Buffer failed!", K_SEVERITY_WARNING);
	}

	for (int kk = 0; kk < m_nMaxQuadsCnt; kk++)
	{
		pIndices[kk * 6 + 0] = (DWORD)(kk * 6 + 0);
		pIndices[kk * 6 + 1] = (DWORD)(kk * 6 + 1);
		pIndices[kk * 6 + 2] = (DWORD)(kk * 6 + 2);
											
		pIndices[kk * 6 + 0] = (DWORD)(kk * 6 + 0);
		pIndices[kk * 6 + 2] = (DWORD)(kk * 6 + 2);
		pIndices[kk * 6 + 3] = (DWORD)(kk * 6 + 3);
	}
	m_ib->Unlock();

	return K_OP_OK;
}

OPRESULT CBufferedPainterQuads::OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	assert(m_nMaxQuadsCnt > 0);

	HRESULT hr = S_OK;
	m_pDevice = pDevice;

	//create vb and ib
	if (FAILED(m_pDevice->CreateVertexBuffer((m_nMaxQuadsCnt + K_BP_SENTINEL_QUADS) * 4 * sizeof(_VERTEX_PNCT4T4),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
		_VERTEX_PNCT4T4::FVF, D3DPOOL_DEFAULT,
		&m_vb, NULL)))
	{
		return OPRESULT(K_OP_FAILED, L"[ERROR] CBufferedPainterQuads::OnResetDevice: Create Vertex Buffer failed!", K_SEVERITY_WARNING);
	}
	//builds buffers too
	if (OP_FAILED(BuildBuffers()))
	{
		return OPRESULT(K_OP_FAILED, L"[ERROR] CBufferedPainterQuads::OnResetDevice: BuildBuffers failed!", K_SEVERITY_WARNING);
	}

	return K_OP_OK;
}

OPRESULT CBufferedPainterQuads::OnLostDevice(void* pUserContext)
{
	SAFE_RELEASE(m_vb);

	return K_OP_OK;
}

OPRESULT CBufferedPainterQuads::OnDestroyDevice(void* pUserContext)
{
	SAFE_RELEASE(m_ib);

	return K_OP_OK;
}
