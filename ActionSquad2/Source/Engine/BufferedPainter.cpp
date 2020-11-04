#include "dxstdafx.h"



///-------------------------------------------------------------
///	 buffered sprites 
///-------------------------------------------------------------

CBufferedSprites::CBufferedSprites(void)
{
	m_vb = NULL;
	m_ib = NULL;

	pDevice = NULL;

	m_nVertexCursor = 0;
	m_verts = NULL;
}

CBufferedSprites::~CBufferedSprites(void)
{
}

HRESULT CBufferedSprites::Begin(UINT32 flags)
{
	//--- set render flags ---
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	if (flags & K_BS_ALPHABLENDING)
	{
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);

		pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}
	//pha test = true => poate accelereaza putin dat fiind ca o mare parte din sprite e transparent total de obicei
	if (flags & K_BS_ALPHATEST)
	{
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true);
		pDevice->SetRenderState(D3DRS_ALPHAREF, 0x0000000C); //0.05f
	}

	if (flags & K_BS_MODULATE_COLORS)
	{
		pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
		pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	}
	else
	{
		pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
		pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
		pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
		pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	}

	//reset verts
	m_nVertexCursor = 0;
	//reset tex changes
	m_nTexChangesCursor = 0;
	for(int kk=0; kk < K_BS_MAX_TEXCHANGES_CNT; kk++)
	{
		m_texPtrs[kk] = NULL;
		m_nTrisPerTexture[kk] = 0;
		m_nTrisOffsets[kk] = 0;
	}

	//vertex shader
 //   D3DXMATRIXA16 mWorld;
 //   D3DXMATRIXA16 mView;
 //   D3DXMATRIXA16 mProj;

	//D3DXMatrixIdentity(&mWorld);
	//D3DXMatrixIdentity(&mView);
	//D3DXMatrixOrthoOffCenterLH(&mProj, 0.0f, SCREEN_WIDTH_F, SCREEN_HEIGHT_F, 0.0f, 0.0f, 1000.0f);

	//D3DXMATRIXA16 matViewProj;
	//D3DXMatrixMultiply(&matViewProj, &mView, &mProj);

	//pDevice->SetVertexShader(g_shaderMan.VertexShaders[0]->pShader);
	//pDevice->SetVertexDeclaration(g_shaderMan._VERTEX_PNCT4T4_decl);
	//pDevice->SetVertexShaderConstantF(0, (float*)&matViewProj,			4);

	pDevice->SetFVF(_VERTEX_PNCT4T4::FVF);
	pDevice->SetStreamSource(0, m_vb, 0, sizeof(_VERTEX_PNCT4T4));
	pDevice->SetIndices(m_ib);

	return S_OK;
}

HRESULT CBufferedSprites::End()
{
	HRESULT hr = S_OK;

	V_RETURN(Flush());
	pDevice->SetIndices(NULL);

	//pDevice->SetVertexShader(NULL);

	return S_OK;
}

HRESULT CBufferedSprites::Flush()
{
	//inainte de ultima afisare trebe sa forteze schimbarea de textura
	m_nTexChangesCursor++;
	m_nTrisOffsets[m_nTexChangesCursor] = m_nTrisOffsets[m_nTexChangesCursor - 1] + m_nTrisPerTexture[m_nTexChangesCursor - 1];

	HRESULT hr = S_OK;
	if(m_nVertexCursor == 0)
		return S_OK;
	//write verts to VB
    _VERTEX_PNCT4T4* pVerts;
	if (FAILED(m_vb->Lock(0, m_nVertexCursor * sizeof(_VERTEX_PNCT4T4), (void**)&pVerts, D3DLOCK_DISCARD)))
	{
		ErrorBox(K_ERR_WARNING, L"[ERROR] CBufferedPainter::Flush() VB Lock failed!");
		return E_FAIL;
	}

	memcpy(pVerts, m_verts, m_nVertexCursor * sizeof(_VERTEX_PNCT4T4));

    m_vb->Unlock();
	//paint

	pDevice->SetFVF(_VERTEX_PNCT4T4::FVF);
	pDevice->SetStreamSource(0, m_vb, 0, sizeof(_VERTEX_PNCT4T4));
	pDevice->SetIndices(m_ib);

	for (UINT32 kk = 0; kk < m_nTexChangesCursor; kk++)
	{
		if(m_nTrisPerTexture[kk] == 0)
			continue;
	
		pDevice->SetTexture(0, m_texPtrs[kk]);

		pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, m_nTrisOffsets[kk] * 2, m_nTrisPerTexture[kk] * 2, m_nTrisOffsets[kk] * 3, m_nTrisPerTexture[kk]);
	}

	//reset verts
	m_nVertexCursor = 0;
	//reset tex changes
	m_nTexChangesCursor = 0;
	for (int kk = 0; kk < K_BS_MAX_TEXCHANGES_CNT; kk++)
	{
		m_texPtrs[kk] = NULL;
		m_nTrisPerTexture[kk] = 0;
		m_nTrisOffsets[kk] = 0;
	}

	return S_OK;
}

//--- acelasi format ca DRAW=ul din Sprite ---
//#TODO: !!! trebuie optimizat ca coord in textura sa fie calculate inca de la import !!!
D3DXVECTOR3 vecPos;
D3DXVECTOR3 vecArr[4];
HRESULT CBufferedSprites::DrawBuffered(LPDIRECT3DTEXTURE9 pTexture, RECTXYXY_F *pSrcRectUV, RECTXYWH_F *pSrcCoord, D3DXVECTOR3 *pCenter, D3DXVECTOR3 *pPosition, DWORD color)
{
	vecPos.x = vecPos.y = vecPos.z = 0.0f;
	if(pPosition != NULL)
		vecPos += *pPosition;
	if(pCenter != NULL)
		vecPos -= *pCenter;

	//ul
	m_verts[m_nVertexCursor].pos = vecPos;
	m_verts[m_nVertexCursor].tex1.x = pSrcRectUV->x1;
	m_verts[m_nVertexCursor].tex1.y = pSrcRectUV->y1;
	m_verts[m_nVertexCursor++].color = color;
	//ur
	m_verts[m_nVertexCursor].pos = vecPos;
	m_verts[m_nVertexCursor].pos.x += pSrcCoord->w;
	m_verts[m_nVertexCursor].tex1.x = pSrcRectUV->x2;
	m_verts[m_nVertexCursor].tex1.y = pSrcRectUV->y1;
	m_verts[m_nVertexCursor++].color = color;
	//dl
	m_verts[m_nVertexCursor].pos = vecPos;
	m_verts[m_nVertexCursor].pos.y += pSrcCoord->h;
	m_verts[m_nVertexCursor].tex1.x = pSrcRectUV->x1;
	m_verts[m_nVertexCursor].tex1.y = pSrcRectUV->y2;
	m_verts[m_nVertexCursor++].color = color;
	//dr
	m_verts[m_nVertexCursor].pos = vecPos;
	m_verts[m_nVertexCursor].pos.x += pSrcCoord->w;
	m_verts[m_nVertexCursor].pos.y += pSrcCoord->h;
	m_verts[m_nVertexCursor].tex1.x = pSrcRectUV->x2;
	m_verts[m_nVertexCursor].tex1.y = pSrcRectUV->y2;
	m_verts[m_nVertexCursor++].color = color;

	assert(m_nVertexCursor < (K_BS_MAX_QUAD_CNT * 4));
	//verifica daca a schimbat textura
	if (pTexture != m_texPtrs[m_nTexChangesCursor])
	{
		if(m_texPtrs[m_nTexChangesCursor] == NULL)
		{
			m_texPtrs[m_nTexChangesCursor] = pTexture;
			m_nTrisOffsets[m_nTexChangesCursor] = 0;
		}
		else
		{
			m_nTexChangesCursor++;
			//verifica pe debug cand depaseste array-ul
			assert(m_nTexChangesCursor < K_BS_MAX_TEXCHANGES_CNT);

			m_texPtrs[m_nTexChangesCursor] = pTexture;
			m_nTrisOffsets[m_nTexChangesCursor] = m_nTrisOffsets[m_nTexChangesCursor - 1] + m_nTrisPerTexture[m_nTexChangesCursor - 1];
		}
	}
	m_nTrisPerTexture[m_nTexChangesCursor] += 2;

	return S_OK;
}


//--- framework ---
HRESULT CBufferedSprites::OnCreateDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr = S_OK;
	m_verts = new _VERTEX_PNCT4T4[(K_BS_MAX_QUAD_CNT + 2) * 4];

	pDevice = pd3dDevice;
	//create index buffer (fixed) - deci va desena numai dreptunghiuri
	V_RETURN(pd3dDevice->CreateIndexBuffer((K_BS_MAX_QUAD_CNT + 2) * 6 * sizeof(DWORD), 0, D3DFMT_INDEX32, D3DPOOL_MANAGED, &m_ib, 0));

	return S_OK;
}

HRESULT CBufferedSprites::OnResetDevice( IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext )
{
	HRESULT hr = S_OK;
	pDevice = pd3dDevice;

    DWORD * pIndices;
    V_RETURN(m_ib->Lock( 0, NULL, (void**) &pIndices, 0 ));
	for(int kk=0; kk < K_BS_MAX_QUAD_CNT; kk++)
	{
		pIndices[kk * 6 + 0] = (DWORD)(kk * 4 + 0);
		pIndices[kk * 6 + 1] = (DWORD)(kk * 4 + 1);
		pIndices[kk * 6 + 2] = (DWORD)(kk * 4 + 2);
		pIndices[kk * 6 + 3] = (DWORD)(kk * 4 + 2);
		pIndices[kk * 6 + 4] = (DWORD)(kk * 4 + 1);
		pIndices[kk * 6 + 5] = (DWORD)(kk * 4 + 3);
	}
	m_ib->Unlock();
	//create vb and ib
    V_RETURN( pd3dDevice->CreateVertexBuffer( (K_BS_MAX_QUAD_CNT + 2) * 4 * sizeof(_VERTEX_PNCT4T4), 
                                                D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
                                                _VERTEX_PNCT4T4::FVF, D3DPOOL_DEFAULT,
                                                &m_vb, NULL ) );

    
	return S_OK;
}

HRESULT CBufferedSprites::OnLostDevice( void* pUserContext )
{
	SAFE_RELEASE(m_vb);

	return S_OK;
}

HRESULT CBufferedSprites::OnDestroyDevice( void* pUserContext )
{
	SAFE_DELETE_ARRAY(m_verts);
	SAFE_RELEASE(m_ib);

	return S_OK;
}


///-----------------------------------------------------------------------------------------------
///	 BUFFERED PAINTER
///  - adaugi triunghiuri unul cate unul si la final face un VB si IB din care poti desena mesh-ul
///-----------------------------------------------------------------------------------------------

CBufferedPainter::CBufferedPainter(void)
{
	m_verts = new _VERTEX_PNCT4T4[(K_BP_MAX_TRIS_CNT + K_BP_SENTINEL) * 3];

	m_nMeshesCnt = 0;
	m_nVertexCursor = 0;
	m_bMeshStarted = false;
}

CBufferedPainter::~CBufferedPainter(void)
{
	SAFE_DELETE_ARRAY(m_verts);
}

HRESULT CBufferedPainter::BeginMesh(int &retMeshIdx)
{
	if (m_bMeshStarted)
	{
		EndMesh();
		ErrorBox(K_ERR_LOG, L"A mesh is already started. Closing mesh and starting another.");
	}

	if (m_nMeshesCnt >= K_BP_MAX_MESHES_CNT)
	{
		ErrorBox(K_ERR_WARNING, L"Too many meshes!");

		retMeshIdx = -1;
		return E_FAIL;
	}
	//assert(m_nMeshesCnt < K_BP_MAX_MESHES_CNT);
	m_bMeshStarted = true;

	//reset mesh data
	m_nTrisPerMesh[m_nMeshesCnt] = 0;
	m_nTrisOffsets[m_nMeshesCnt] = 0;
	//return mesh idx
	retMeshIdx = m_nMeshesCnt;

	return S_OK;
}

HRESULT CBufferedPainter::AddTriangles(_VERTEX_PNCT4T4 *points, int trisCount)
{
	if (!m_bMeshStarted)
	{
		ErrorBox(K_ERR_WARNING, L"You have to call BeginMesh() first!");
		return E_FAIL;
	}

#if defined(_DEBUG) || defined(DEBUG)
	assert(m_nVertexCursor + trisCount * 3 < K_BP_MAX_TRIS_CNT * 3);
#endif

	memcpy(&m_verts[m_nVertexCursor], points, sizeof(_VERTEX_PNCT4T4) * 3 * trisCount);

	m_nVertexCursor += 3 * trisCount;
	m_nTrisPerMesh[m_nMeshesCnt] += trisCount;

	return S_OK;
}

HRESULT CBufferedPainter::EndMesh()
{
	//make sure we close any pending meshes
	if (m_bMeshStarted == false)
		return S_OK;

	m_bMeshStarted = false;

	if (m_nMeshesCnt == 0) //is it the first mesh?
	{
		m_nTrisOffsets[m_nMeshesCnt] = 0;
	}
	else
	{
		m_nTrisOffsets[m_nMeshesCnt] = m_nTrisOffsets[m_nMeshesCnt - 1] + m_nTrisPerMesh[m_nMeshesCnt - 1];
	}
	//get to next mesh
	m_nMeshesCnt++;

	return S_OK;
}

HRESULT CBufferedPainter::ClearBuffers()
{
	//reset all counters
	m_nMeshesCnt = 0;
	m_bMeshStarted = false;

	m_nVertexCursor = 0;
			  
	return S_OK;
}

HRESULT CBufferedPainter::BuildBuffers()
{
	EndMesh();

	if (m_nVertexCursor == 0)
		return S_OK;

	HRESULT hr = S_OK;
	//write verts to VB
	_VERTEX_PNCT4T4* pVerts;
	if (FAILED(m_vb->Lock(0, m_nVertexCursor * sizeof(_VERTEX_PNCT4T4), (void**)&pVerts, D3DLOCK_DISCARD)))
	{
		ErrorBox(K_ERR_WARNING, L"[ERROR] CBufferedPainter: Build buffers failed!");
		return E_FAIL;
	}

	memcpy(pVerts, m_verts, m_nVertexCursor * sizeof(_VERTEX_PNCT4T4));

	m_vb->Unlock();

	return S_OK;
}

HRESULT CBufferedPainter::DrawMesh(int meshIdx, bool setFVF)
{
	HRESULT hr = S_OK;

	//empty mesh: exit
	if (meshIdx < 0)
	{
		// this happens often if light touches no shadow casters. Logging not necessary.
		//LOG(L"!!! WARNING: CBufferedPainter::DrawMesh called with mesh idx = -1\n");
		return E_INVALIDARG;
	}

	if (meshIdx >= m_nMeshesCnt)
	{
		LOG(L"Trying to draw a mesh that doesn't exist! Input idx=%d meshesCnt=%d", meshIdx, m_nMeshesCnt);
		return E_FAIL;
	}
	if (m_nTrisPerMesh[meshIdx] == 0)
		return S_OK;
	
	if(setFVF)
		pDevice->SetFVF(_VERTEX_PNCT4T4::FVF);

	pDevice->SetStreamSource(0, m_vb, 0, sizeof(_VERTEX_PNCT4T4));
	pDevice->SetIndices(m_ib);

	if (FAILED(pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, m_nTrisOffsets[meshIdx] * 3, 0, m_nTrisPerMesh[meshIdx] * 3, 0, m_nTrisPerMesh[meshIdx])))
	{
		ErrorBox(K_ERR_WARNING, L"CBufferedPainter::DrawMesh failed(%d, %d, %d)!", m_nTrisOffsets[meshIdx], m_nTrisPerMesh[meshIdx], m_nTrisPerMesh[meshIdx]);
		return E_FAIL;
	}
	
	return S_OK;
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
HRESULT CBufferedPainter::OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	HRESULT hr = S_OK;

	pDevice = pd3dDevice;
	//create index buffer (fixed) - deci va desena numai triunghiuri independente
	if (FAILED(pd3dDevice->CreateIndexBuffer((K_BP_MAX_TRIS_CNT + K_BP_SENTINEL) * 3 * sizeof(DWORD), 0, D3DFMT_INDEX32, D3DPOOL_MANAGED, &m_ib, 0)))
	{
		ErrorBox(K_ERR_WARNING, L"[ERROR] CBufferedPainter: Create Index Buffer failed!");
		return E_FAIL;
	}
	//lock and fill
	DWORD * pIndices;
	if (FAILED(m_ib->Lock(0, NULL, (void**)&pIndices, 0)))
	{
		ErrorBox(K_ERR_WARNING, L"[ERROR] CBufferedPainter: Lock Index Buffer failed!");
		return E_FAIL;
	}

	for (int kk = 0; kk < K_BP_MAX_TRIS_CNT; kk++)
	{
		pIndices[kk * 3 + 0] = (DWORD)(kk * 3 + 0);
		pIndices[kk * 3 + 1] = (DWORD)(kk * 3 + 1);
		pIndices[kk * 3 + 2] = (DWORD)(kk * 3 + 2);
	}
	m_ib->Unlock();

	return S_OK;
}

HRESULT CBufferedPainter::OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	HRESULT hr = S_OK;
	pDevice = pd3dDevice;

	//create vb and ib
	if (FAILED(pd3dDevice->CreateVertexBuffer((K_BP_MAX_TRIS_CNT + K_BP_SENTINEL) * 3 * sizeof(_VERTEX_PNCT4T4),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
		_VERTEX_PNCT4T4::FVF, D3DPOOL_DEFAULT,
		&m_vb, NULL)))
	{
		ErrorBox(K_ERR_WARNING, L"[ERROR] CBufferedPainter::OnResetDevice: Create Vertex Buffer failed!");
		return E_FAIL;
	}
	//builds buffers too
	if (FAILED(BuildBuffers()))
	{
		ErrorBox(K_ERR_WARNING, L"[ERROR] CBufferedPainter::OnResetDevice: BuildBuffers failed!");
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CBufferedPainter::OnLostDevice(void* pUserContext)
{
	SAFE_RELEASE(m_vb);

	return S_OK;
}

HRESULT CBufferedPainter::OnDestroyDevice(void* pUserContext)
{
	SAFE_RELEASE(m_ib);

	return S_OK;
}

///----------------------------------------------------
/// CBufferedTexPainter
///----------------------------------------------------

CBufferedTexPainter::CBufferedTexPainter()
{
	passesCnt = 0;
}

CBufferedTexPainter::~CBufferedTexPainter()
{
	Clear();
}

void CBufferedTexPainter::BufferMesh(_VERTEX_PNCT4T4 *points, int trisCount, CSpineTex* pTex, EBlendMode eMode)
{
	//if we have no mesh or if last mesh has another mode or texture then we initialize another mesh
	if ((passesCnt == 0) || (arrPasses[passesCnt - 1].eMode != eMode) || (arrPasses[passesCnt - 1].pTex != pTex))
	{
		if (!FAILED(BeginMesh(arrPasses[passesCnt].nMeshIdx)))
		{
			arrPasses[passesCnt].pTex = pTex;
			arrPasses[passesCnt].eMode = eMode;
			passesCnt++;
		}
	}

	AddTriangles(points, trisCount);
}

void CBufferedTexPainter::Clear()
{
	passesCnt = 0;
	ClearBuffers();
}

void CBufferedTexPainter::Paint(bool setFVF /*= true*/, ETexChannel eChannel)
{
	//set FVF if necessary
	if (setFVF)
		pDevice->SetFVF(_VERTEX_PNCT4T4::FVF);

	for (int kk = 0; kk < passesCnt; kk++)
	{
		//#TODO: set mode
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
			pDevice->SetTexture(0, arrPasses[kk].pTex->pTexture);
		else if (eChannel == K_TEXCHAN_NORMALMAP)
			pDevice->SetTexture(0, arrPasses[kk].pTex->pTexture_N);
		else if (eChannel == K_TEXCHAN_SPECULARMAP)
			pDevice->SetTexture(0, arrPasses[kk].pTex->pTexture_S);

		DrawMesh(arrPasses[kk].nMeshIdx, false);
	}
}


