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

void CBufferedSprites::SetTransformWorld(D3DXMATRIXA16 *matWrld)
{
	Flush();
	m_matWorld = *matWrld;
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
	//use the last world transformation
	pDevice->SetTransform(D3DTS_WORLD, &m_matWorld);

	for (UINT32 kk = 0; kk < m_nTexChangesCursor; kk++)
	{
		if(m_nTrisPerTexture[kk] == 0)
			continue;
	
		pDevice->SetTexture(0, m_texPtrs[kk]);

		pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, m_nTrisOffsets[kk] * 2, m_nTrisPerTexture[kk] * 2, m_nTrisOffsets[kk] * 3, m_nTrisPerTexture[kk]);
	}
/*	
	pDevice->SetTexture(0, m_texPtrs[0]);
	pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 4);
	pDevice->SetTexture(0, m_texPtrs[1]);
	pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 8, 8, 12, 4);
	*/

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


HRESULT CBufferedSprites::Draw4VertsSpriteBuffered(LPDIRECT3DTEXTURE9 pTexture, _VERTEX_PNCT4T4 *verts, D3DXVECTOR3 *pos, DWORD color)
{
	m_verts[m_nVertexCursor] = verts[0];
	m_verts[m_nVertexCursor].color = color;
	m_verts[m_nVertexCursor++].pos += *pos;

	m_verts[m_nVertexCursor] = verts[1];
	m_verts[m_nVertexCursor].color = color;
	m_verts[m_nVertexCursor++].pos += *pos;

	m_verts[m_nVertexCursor] = verts[2];
	m_verts[m_nVertexCursor].color = color;
	m_verts[m_nVertexCursor++].pos += *pos;

	m_verts[m_nVertexCursor] = verts[3];
	m_verts[m_nVertexCursor].color = color;
	m_verts[m_nVertexCursor++].pos += *pos;

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
			m_texPtrs[m_nTexChangesCursor] = pTexture;
			m_nTrisOffsets[m_nTexChangesCursor] = m_nTrisOffsets[m_nTexChangesCursor - 1] + m_nTrisPerTexture[m_nTexChangesCursor - 1];
		}
	}

	//verificare sa nu faca prea multe schimbari de textura
	assert(m_nTexChangesCursor < K_BS_MAX_TEXCHANGES_CNT);
	m_nTrisPerTexture[m_nTexChangesCursor] += 2;

	return S_OK;
}

//--- acelasi format ca DRAW=ul din Sprite ---
// !!! trebuie optimizat ca coord in textura sa fie calculate inca de la import !!!
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

//aplica o matrice de transformare pe coordonate inainte sa adauge pozitia primita
//adauga transformat in buffer
HRESULT CBufferedSprites::DrawBufferedTransformed(LPDIRECT3DTEXTURE9 pTexture, RECTXYXY_F *pSrcRectUV, RECTXYWH_F *pSrcCoord, D3DXVECTOR3 *pCenter, D3DXMATRIXA16* coordtrans, D3DXVECTOR3 *pPosition, DWORD color)
{
	vecPos.x = vecPos.y = vecPos.z = 0.0f;
	if(pPosition != NULL)
		vecPos += *pPosition;
	if(pCenter != NULL)
		vecPos -= *pCenter;

	
	vecArr[0] = vecArr[1] = vecArr[2] = vecArr[3] = -(*pCenter);
	vecArr[1].x += pSrcCoord->w;
	vecArr[2].y += pSrcCoord->h;
	vecArr[3].x += pSrcCoord->w;
	vecArr[3].y += pSrcCoord->h;
	D3DXVec3TransformCoordArray(&vecArr[0], sizeof(D3DXVECTOR3), &vecArr[0], sizeof(D3DXVECTOR3), coordtrans, 4);
	//ul
	m_verts[m_nVertexCursor].pos = vecPos + vecArr[0];
	m_verts[m_nVertexCursor].tex1.x = pSrcRectUV->x1;
	m_verts[m_nVertexCursor].tex1.y = pSrcRectUV->y1;
	m_verts[m_nVertexCursor++].color = color;
	//ur
	m_verts[m_nVertexCursor].pos = vecPos + vecArr[1];
	m_verts[m_nVertexCursor].tex1.x = pSrcRectUV->x2;
	m_verts[m_nVertexCursor].tex1.y = pSrcRectUV->y1;
	m_verts[m_nVertexCursor++].color = color;
	//dl
	m_verts[m_nVertexCursor].pos = vecPos + vecArr[2];
	m_verts[m_nVertexCursor].tex1.x = pSrcRectUV->x1;
	m_verts[m_nVertexCursor].tex1.y = pSrcRectUV->y2;
	m_verts[m_nVertexCursor++].color = color;
	//dr
	m_verts[m_nVertexCursor].pos = vecPos + vecArr[3];
	m_verts[m_nVertexCursor].tex1.x = pSrcRectUV->x2;
	m_verts[m_nVertexCursor].tex1.y = pSrcRectUV->y2;
	m_verts[m_nVertexCursor++].color = color;

	//D3DXVec3TransformCoordArray(&m_verts[m_nVertexCursor - 4].pos, sizeof(_VERTEX_PNCT4T4), &m_verts[m_nVertexCursor - 4].pos, sizeof(_VERTEX_PNCT4T4), coordtrans, 4);

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

HRESULT CBufferedPainter::BeginMesh(UINT32 & retMeshIdx)
{
	if (m_bMeshStarted)
	{
		EndMesh();
		ErrorBox(K_ERR_DEBUGOUT, L"A mesh is already started. Closing mesh and starting another.");
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

	memcpy(&m_verts[m_nVertexCursor], points, sizeof(_VERTEX_PNCT4T4) * 3 * trisCount);

	m_nVertexCursor += 3 * trisCount;
	m_nTrisPerMesh[m_nMeshesCnt] += trisCount;

	return S_OK;
}

HRESULT CBufferedPainter::EndMesh()
{
	m_bMeshStarted = false;

	if (m_nMeshesCnt == 0) //daca e primul mesh
	{
		m_nTrisOffsets[m_nMeshesCnt] = 0;
	}
	else
	{
		m_nTrisOffsets[m_nMeshesCnt] = m_nTrisOffsets[m_nMeshesCnt - 1] + m_nTrisPerMesh[m_nMeshesCnt - 1];
	}
	//trec la urmatorul mesh
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
		//DebugPrintA("!!! WARNING: CBufferedPainter::DrawMesh called with mesh idx = -1\n");
		return E_INVALIDARG;
	}

	if (meshIdx >= m_nMeshesCnt)
	{
		//ErrorBox(K_ERR_WARNING, L"Trying to draw a mesh that doesn't exist! Input idx=%d meshesCnt=%d", meshIdx, m_nMeshesCnt);
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
