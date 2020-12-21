#include "dxstdafx.h"
#include "BufferedPainter.h"


///-----------------------------------------------------------------------------------------------
///	 BUFFERED PAINTER
///  - adaugi triunghiuri unul cate unul si la final face un VB si IB din care poti desena mesh-ul
///-----------------------------------------------------------------------------------------------

CBufferedPainter::CBufferedPainter() :
	m_vb(null), m_ib(null), m_pDevice(null),
	m_nMeshesCnt(0), m_nVertexCursor(0), m_bMeshStarted(false),
	m_verts(null), m_nMaxTrisCnt(0)
{
}

CBufferedPainter::~CBufferedPainter(void)
{
	ClearBuffers();
	SAFE_DELETE_ARRAY(m_verts);

	SAFE_RELEASE(m_vb);
	SAFE_RELEASE(m_ib);

	m_nMaxTrisCnt = 0;
}

void CBufferedPainter::Init(int nMaxTrisCnt)
{
	m_nMaxTrisCnt = nMaxTrisCnt;
	m_verts = new _VERTEX_PNCT4T4[(m_nMaxTrisCnt + K_BP_SENTINEL_TRIS) * 3];
}

OPRESULT CBufferedPainter::BeginMesh(int &retMeshIdx)
{
	if (m_bMeshStarted)
	{
		EndMesh();
		ErrorBox(K_ERR_LOG, L"CBufferedPainter:: A mesh is already started. Closing mesh and starting another.");
		// closes the mesh automatically, no return value
	}

	if (m_nMeshesCnt >= K_BP_MAX_MESHES_CNT)
	{
		retMeshIdx = -1;
		return OPRESULT(K_OP_FAILED, L"CBufferedPainter:: Too many meshes!", K_SEVERITY_WARNING);
	}
	//assert(m_nMeshesCnt < K_BP_MAX_MESHES_CNT);
	m_bMeshStarted = true;

	//reset mesh data
	m_nTrisPerMesh[m_nMeshesCnt] = 0;
	m_nTrisOffsets[m_nMeshesCnt] = 0;
	//return mesh idx
	retMeshIdx = m_nMeshesCnt;

	return K_OP_OK;
}

OPRESULT CBufferedPainter::AddTriangles(_VERTEX_PNCT4T4 *points, int trisCount)
{
	assert(m_nMaxTrisCnt > 0);

	if (!m_bMeshStarted)
		return OPRESULT(K_OP_FAILED, L"You have to call BeginMesh() first!", K_SEVERITY_WARNING);

#if defined(_DEBUG) || defined(DEBUG)
	assert(m_nVertexCursor + trisCount * 3 < m_nMaxTrisCnt * 3);
#endif

	memcpy(&m_verts[m_nVertexCursor], points, sizeof(_VERTEX_PNCT4T4) * 3 * trisCount);

	m_nVertexCursor += 3 * trisCount;
	m_nTrisPerMesh[m_nMeshesCnt] += trisCount;

	return K_OP_OK;
}

int CBufferedPainter::EndMesh()
{
	//make sure we close any pending meshes
	if (m_bMeshStarted == false)
		return 0;

	m_bMeshStarted = false;

	int nTris = 0;
	if (m_nMeshesCnt == 0) //is it the first mesh?
	{
		m_nTrisOffsets[m_nMeshesCnt] = 0;
	}
	else
	{
		m_nTrisOffsets[m_nMeshesCnt] = m_nTrisOffsets[m_nMeshesCnt - 1] + m_nTrisPerMesh[m_nMeshesCnt - 1];
	}
	nTris = m_nTrisPerMesh[m_nMeshesCnt];
	//get to next mesh
	m_nMeshesCnt++;

	return nTris;
}

OPRESULT CBufferedPainter::ClearBuffers()
{
	//reset all counters
	m_nMeshesCnt = 0;
	m_bMeshStarted = false;

	m_nVertexCursor = 0;
			  
	return K_OP_OK;
}

OPRESULT CBufferedPainter::BuildBuffers()
{
	EndMesh();

	if (m_nVertexCursor == 0)
		return K_OP_OK;

	HRESULT hr = S_OK;
	//write verts to VB
	_VERTEX_PNCT4T4* pVerts;
	if (FAILED(m_vb->Lock(0, m_nVertexCursor * sizeof(_VERTEX_PNCT4T4), (void**)&pVerts, D3DLOCK_DISCARD)))
	{
		return OPRESULT(K_OP_FAILED, L"[ERROR] CBufferedPainter: Build buffers failed!", K_SEVERITY_WARNING);
	}

	memcpy(pVerts, m_verts, m_nVertexCursor * sizeof(_VERTEX_PNCT4T4));

	m_vb->Unlock();

	return K_OP_OK;
}

OPRESULT CBufferedPainter::DrawMesh(int meshIdx, bool setFVF)
{
	HRESULT hr = S_OK;

	//empty mesh: exit
	if (meshIdx < 0)
	{
		// this happens often if light touches no shadow casters. Logging not necessary.
		//LOG(L"!!! WARNING: CBufferedPainter::DrawMesh called with mesh idx = -1\n");
		return K_OP_INVALIDARGS;
	}

	if (meshIdx >= m_nMeshesCnt)
	{
		LOG(L"Trying to draw a mesh that doesn't exist! Input idx=%d meshesCnt=%d", meshIdx, m_nMeshesCnt);
		return K_OP_FAILED;
	}
	if (m_nTrisPerMesh[meshIdx] == 0)
		return K_OP_OK;
	
	if(setFVF)
		m_pDevice->SetFVF(_VERTEX_PNCT4T4::FVF);

	m_pDevice->SetStreamSource(0, m_vb, 0, sizeof(_VERTEX_PNCT4T4));
	m_pDevice->SetIndices(m_ib);

	if (FAILED(m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, m_nTrisOffsets[meshIdx] * 3, 0, m_nTrisPerMesh[meshIdx] * 3, 0, m_nTrisPerMesh[meshIdx])))
	{
		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CBufferedPainter::DrawMesh failed(%d, %d, %d)!", m_nTrisOffsets[meshIdx], m_nTrisPerMesh[meshIdx], m_nTrisPerMesh[meshIdx]);
	}
	
	return K_OP_OK;
}

const int CBufferedPainter::GetTrisCount(int meshIdx) const
{
	if ((meshIdx < 0) || (meshIdx >= m_nMeshesCnt))
	{
		return 0;
	}

	return m_nTrisPerMesh[meshIdx];
}

//--- framework ---
OPRESULT CBufferedPainter::OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	assert(m_nMaxTrisCnt > 0);

	HRESULT hr = S_OK;

	m_pDevice = pDevice;
	//create index buffer (fixed) - deci va desena numai triunghiuri independente
	if (FAILED(m_pDevice->CreateIndexBuffer((m_nMaxTrisCnt + K_BP_SENTINEL_TRIS) * 3 * sizeof(DWORD), 0, D3DFMT_INDEX32, D3DPOOL_MANAGED, &m_ib, 0)))
	{
		return OPRESULT(K_OP_FAILED, L"[ERROR] CBufferedPainter: Create Index Buffer failed!", K_SEVERITY_WARNING);
	}
	//lock and fill
	DWORD * pIndices;
	if (FAILED(m_ib->Lock(0, NULL, (void**)&pIndices, 0)))
	{
		return OPRESULT(K_OP_FAILED, L"[ERROR] CBufferedPainter: Lock Index Buffer failed!", K_SEVERITY_WARNING);
	}

	for (int kk = 0; kk < m_nMaxTrisCnt; kk++)
	{
		pIndices[kk * 3 + 0] = (DWORD)(kk * 3 + 0);
		pIndices[kk * 3 + 1] = (DWORD)(kk * 3 + 1);
		pIndices[kk * 3 + 2] = (DWORD)(kk * 3 + 2);
	}
	m_ib->Unlock();

	return K_OP_OK;
}

OPRESULT CBufferedPainter::OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	assert(m_nMaxTrisCnt > 0);

	HRESULT hr = S_OK;
	m_pDevice = pDevice;

	//create vb and ib
	if (FAILED(m_pDevice->CreateVertexBuffer((m_nMaxTrisCnt + K_BP_SENTINEL_TRIS) * 3 * sizeof(_VERTEX_PNCT4T4),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
		_VERTEX_PNCT4T4::FVF, D3DPOOL_DEFAULT,
		&m_vb, NULL)))
	{
		return OPRESULT(K_OP_FAILED, L"[ERROR] CBufferedPainter::OnResetDevice: Create Vertex Buffer failed!", K_SEVERITY_WARNING);
	}
	//builds buffers too
	if (OP_FAILED(BuildBuffers()))
	{
		return OPRESULT(K_OP_FAILED, L"[ERROR] CBufferedPainter::OnResetDevice: BuildBuffers failed!", K_SEVERITY_WARNING);
	}

	return K_OP_OK;
}

OPRESULT CBufferedPainter::OnLostDevice(void* pUserContext)
{
	SAFE_RELEASE(m_vb);

	return K_OP_OK;
}

OPRESULT CBufferedPainter::OnDestroyDevice(void* pUserContext)
{
	SAFE_RELEASE(m_ib);

	return K_OP_OK;
}

///----------------------------------------------------
/// CBufferedTexPainter
///----------------------------------------------------

CBufferedSpinePainter::CBufferedSpinePainter()
{
	passesCnt = 0;
}

CBufferedSpinePainter::~CBufferedSpinePainter()
{
	Clear();
}

void CBufferedSpinePainter::BufferMesh(_VERTEX_PNCT4T4 *points, int trisCount, CSpineTex* pTex, EBlendMode eMode)
{
	//if we have no mesh or if last mesh has another mode or texture then we initialize another mesh
	if ((passesCnt == 0) || (arrPasses[passesCnt - 1].eMode != eMode) || (arrPasses[passesCnt - 1].pTex != pTex))
	{
		if (!OP_FAILED(BeginMesh(arrPasses[passesCnt].nMeshIdx)))
		{
			arrPasses[passesCnt].pTex = pTex;
			arrPasses[passesCnt].eMode = eMode;
			passesCnt++;
		}
	}

	AddTriangles(points, trisCount);
}

void CBufferedSpinePainter::Clear()
{
	passesCnt = 0;
	ClearBuffers();
}

void CBufferedSpinePainter::Paint(bool setFVF /*= true*/, ETexChannel eChannel)
{
	//set FVF if necessary
	if (setFVF)
		m_pDevice->SetFVF(_VERTEX_PNCT4T4::FVF);

	for (int kk = 0; kk < passesCnt; kk++)
	{
		//#TODO: set blending modes
		switch (arrPasses[kk].eMode)
		{
		case BLEND_NORMAL:
		{
		}
		break;
		case BLEND_ADDITIVE:
		{
		}
		break;
		case BLEND_MULTIPLY:
		{
		}
		break;
		case BLEND_SCREEN:
		{
		}
		break;

		default:
			break;
		}

		//set texture
		if(eChannel == K_TEXCHAN_COLORMAP)
			m_pDevice->SetTexture(0, arrPasses[kk].pTex->pTexture);
		else if (eChannel == K_TEXCHAN_NORMALMAP)
			m_pDevice->SetTexture(0, arrPasses[kk].pTex->pTexture_N);
		else if (eChannel == K_TEXCHAN_SPECULARMAP)
			m_pDevice->SetTexture(0, arrPasses[kk].pTex->pTexture_S);

		DrawMesh(arrPasses[kk].nMeshIdx, false);
	}
}


