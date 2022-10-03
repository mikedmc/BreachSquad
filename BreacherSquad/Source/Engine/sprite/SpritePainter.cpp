#include "dxstdafx.h"
#include "SpritePainter.h"


CSpritePainter::CSpritePainter(void)
{
	m_pVShader = nullptr;
	MUMatIdentity( &m_matProj );
	MUMatIdentity( &m_matView);
	MUMatIdentity( &m_matWorld );

	m_vb = nullptr;
	m_ib = nullptr;

	m_pDevice = nullptr;

	m_nVertexCursor = 0;
	m_nTexChangesCursor = 0;
	m_verts = nullptr;

	bStarted = false;

	ClearStatistics();
}

CSpritePainter::~CSpritePainter(void)
{
	SAFE_DELETE_ARRAY(m_verts);
	SAFE_RELEASE(m_ib);
	SAFE_RELEASE(m_vb);
}


OPRESULT CSpritePainter::Begin(PVERTEXSHADER pVShader, Mat & matView, Mat & matProj, UINT32 flags /*= K_BS_ALPHABLENDING */)
{
	_ASSERT(m_pDevice != nullptr);
	// if already started make sure we do a flush
	if (bStarted)
		End();

#if defined(_DEBUG) || defined(DEBUG)
	stats_sequences++;
#endif
	// set shader and projection matrix
	m_pVShader = pVShader;
	m_matView = matView;
	m_matProj = matProj;
	MUMatIdentity( &m_matWorld );
	m_matWVP = matView * matProj;

	m_nFlags = flags;

	//reset verts
	m_nVertexCursor = 0;
	//reset tex changes
	m_nTexChangesCursor = 0;

	for(int kk=0; kk < K_BS_MAX_MODECHANGES_CNT; kk++)
	{
		m_texPtrs[kk] = nullptr;
		m_nTrisPerTexture[kk] = 0;
		m_nTrisOffsets[kk] = 0;
	}

	//--- set render flags ---
	if (m_nFlags & K_BS_ALPHABLENDING)
	{
		m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);

		m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		m_pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	}
	//do we need alpha testing? it accelerates a little
	if (m_nFlags & K_BS_ALPHATEST)
	{
		m_pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, true);
		m_pDevice->SetRenderState(D3DRS_ALPHAREF, 0x0000000C); //0.05f
	}

	bStarted = true;
	return K_OP_OK;
}

OPRESULT CSpritePainter::End()
{
	if ( !bStarted )
		return K_OP_OK;

	V_OP_RET(Flush());
	m_pDevice->SetIndices(nullptr);

	m_pDevice->SetVertexShader(nullptr);

	bStarted = false;
	return K_OP_OK;
}

OPRESULT CSpritePainter::SetViewProjMatrix(Mat & matView, Mat & matProj)
{
	if (!bStarted)
		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CSpritePainter::Can't use SetViewProjMatrix without calling Begin first!");
	
	V_OP_RET(Flush());
	// set new matrix
	m_matView = matView;
	m_matProj = matProj;
	// compute final matrix
	m_matWVP = m_matWorld * m_matView;
	m_matWVP = m_matWVP * m_matProj;

	return K_OP_OK;
}

OPRESULT CSpritePainter::SetTransformIdentity()
{
	V_OP_RET( Flush() );
	// set new matrix
	m_matWorld = g_matIdentity;
	// compute final matrix
	m_matWVP = m_matWorld * m_matView;
	m_matWVP = m_matWVP * m_matProj;

	return K_OP_OK;
}

OPRESULT CSpritePainter::SetTransform( Mat & matWorld )
{
	V_OP_RET( Flush() );
	// set new matrix
	m_matWorld = matWorld;
	// compute final matrix
	m_matWVP = m_matWorld * m_matView;
	m_matWVP = m_matWVP * m_matProj;

	return K_OP_OK;
}

OPRESULT CSpritePainter::SetViewTransform( Mat & matView )
{
	V_OP_RET( Flush() );
	// set new matrix
	m_matView = matView;
	// compute final matrix
	m_matWVP = m_matWorld * m_matView;
	m_matWVP = m_matWVP * m_matProj;

	return K_OP_OK;
}

OPRESULT CSpritePainter::SetShader(PVERTEXSHADER pVShader)
{
	if (!bStarted)
		return OPRESULT(K_OP_FAILED, K_SEVERITY_WARNING, L"CSpritePainter::Can't use SetViewProjMatrix without calling Begin first!");

	V_OP_RET(Flush());
	// set new shader
	m_pVShader = pVShader;

	return K_OP_OK;
}

void CSpritePainter::GetTransform( Mat * retWorld, Mat * retView /*= nullptr */ )
{
	if(retWorld != nullptr)
		*retWorld = m_matWorld;
	if ( retView != nullptr )
		*retView = m_matView;
}

OPRESULT CSpritePainter::Flush()
{
#if defined(_DEBUG) || defined(DEBUG)
	stats_flushes++;
#endif
	if ( !bStarted )
		return K_OP_OK;

	if (m_nVertexCursor == 0)
		return K_OP_OK;

	//before last draw we must force texture change
	m_nTexChangesCursor++;
	m_nTrisOffsets[m_nTexChangesCursor] = m_nTrisOffsets[m_nTexChangesCursor - 1] + m_nTrisPerTexture[m_nTexChangesCursor - 1];

	//write verts to VB
    _VERTEX_PNCT4T4* pVerts;
	if (FAILED(m_vb->Lock(0, m_nVertexCursor * sizeof(_VERTEX_PNCT4T4), (void**)&pVerts, D3DLOCK_DISCARD)))
	{
		return OPRESULT(K_OP_FAILED, L"[ERROR] CBufferedPainter::Flush() VB Lock failed!", K_SEVERITY_WARNING);
	}

	memcpy(pVerts, m_verts, m_nVertexCursor * sizeof(_VERTEX_PNCT4T4));

    m_vb->Unlock();


	//set vertex shader
	if (m_pVShader)
	{
		m_pDevice->SetVertexShader(m_pVShader);
		m_pDevice->SetVertexDeclaration(__Shaders()._VERTEX_PNCT4T4_decl);
		m_pDevice->SetVertexShaderConstantF(0, (float*)&m_matWVP, 4);
	}
	else
	{
		// no VS version:
		m_pDevice->SetFVF(_VERTEX_PNCT4T4::FVF);
	}
	// set streams
	m_pDevice->SetStreamSource(0, m_vb, 0, sizeof(_VERTEX_PNCT4T4));
	m_pDevice->SetIndices(m_ib);

	for (UINT32 kk = 0; kk < m_nTexChangesCursor; kk++)
	{
		if(m_nTrisPerTexture[kk] == 0)
			continue;
	
		m_pDevice->SetTexture(0, m_texPtrs[kk]);

		m_pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, m_nTrisOffsets[kk] * 2, m_nTrisPerTexture[kk] * 2, m_nTrisOffsets[kk] * 3, m_nTrisPerTexture[kk]);

#if defined(_DEBUG) || defined(DEBUG)
		stats_calls++;
		stats_sprites += m_nTrisPerTexture[kk] / 2;
#endif
	}

	//reset verts
	m_nVertexCursor = 0;
	//reset tex changes
	m_nTexChangesCursor = 0;
	for (int kk = 0; kk < K_BS_MAX_MODECHANGES_CNT; kk++)
	{
		m_texPtrs[kk] = nullptr;
		m_nTrisPerTexture[kk] = 0;
		m_nTrisOffsets[kk] = 0;
	}

	return K_OP_OK;
}

void CSpritePainter::ClearStatistics()
{
#if defined(_DEBUG) || defined(DEBUG)
	stats_calls = 0;
	stats_flushes = 0;
	stats_sequences = 0;
	stats_sprites = 0;
#endif
}

OPRESULT CSpritePainter::Draw(PTEXTURE pTexture, RectLTRB &pSrcUV, RectLTRB &pDestRect, Vec2 vPos, DWORD color, float fRotationZ, Vec2 vScale)
{
	_ASSERT(pTexture != nullptr);
	_ASSERT(m_nVertexCursor < (K_BS_MAX_QUAD_CNT * 4) - 4);
	_ASSERT(bStarted == true);

	// Shader data:
	// Normal contains position
	Vec3 vecPos(vPos.x, vPos.y, 0.0f);
	// Tex2 contains: x: scale x, y: scale y, z: Z rotation
	Vec4 vecScaleRot(vScale.x, vScale.y, fRotationZ, 0.0f);

	//ul
	m_verts[m_nVertexCursor].pos	= Vec3(pDestRect.left, pDestRect.top, 0.0f);
	m_verts[m_nVertexCursor].n		= vecPos;
	m_verts[m_nVertexCursor].tex1	= Vec4(pSrcUV.left, pSrcUV.top, 0.0f, 0.0f);
	m_verts[m_nVertexCursor].tex2	= vecScaleRot;
	m_verts[m_nVertexCursor].color	= color;
	m_nVertexCursor++;
	//ur
	m_verts[m_nVertexCursor].pos = Vec3(pDestRect.right, pDestRect.top, 0.0f);
	m_verts[m_nVertexCursor].n = vecPos;
	m_verts[m_nVertexCursor].tex1 = Vec4(pSrcUV.right, pSrcUV.top, 0.0f, 0.0f);
	m_verts[m_nVertexCursor].tex2 = vecScaleRot;
	m_verts[m_nVertexCursor].color = color;
	m_nVertexCursor++;
	//dl
	m_verts[m_nVertexCursor].pos = Vec3(pDestRect.left, pDestRect.bottom, 0.0f);
	m_verts[m_nVertexCursor].n = vecPos;
	m_verts[m_nVertexCursor].tex1 = Vec4(pSrcUV.left, pSrcUV.bottom, 0.0f, 0.0f);
	m_verts[m_nVertexCursor].tex2 = vecScaleRot;
	m_verts[m_nVertexCursor].color = color;
	m_nVertexCursor++;
	//dr
	m_verts[m_nVertexCursor].pos = Vec3(pDestRect.right, pDestRect.bottom, 0.0f);
	m_verts[m_nVertexCursor].n = vecPos;
	m_verts[m_nVertexCursor].tex1 = Vec4(pSrcUV.right, pSrcUV.bottom, 0.0f, 0.0f);
	m_verts[m_nVertexCursor].tex2 = vecScaleRot;
	m_verts[m_nVertexCursor].color = color;
	m_nVertexCursor++;

	//see if texture changed
	if (pTexture != m_texPtrs[m_nTexChangesCursor])
	{
		if(m_texPtrs[m_nTexChangesCursor] == nullptr)
		{
			m_texPtrs[m_nTexChangesCursor] = pTexture;
			m_nTrisOffsets[m_nTexChangesCursor] = 0;
		}
		else
		{
			m_nTexChangesCursor++;

			m_texPtrs[m_nTexChangesCursor] = pTexture;
			m_nTrisOffsets[m_nTexChangesCursor] = m_nTrisOffsets[m_nTexChangesCursor - 1] + m_nTrisPerTexture[m_nTexChangesCursor - 1];
			_ASSERT(m_nTexChangesCursor < K_BS_MAX_MODECHANGES_CNT);
		}
	}
	m_nTrisPerTexture[m_nTexChangesCursor] += 2;

	return K_OP_OK;
}


OPRESULT CSpritePainter::DrawEx(PTEXTURE pTexture, RectLTRB &pSrcUV, RectLTRB &pDestRect, Vec2 vPos, DWORD color, float fRotationZ, Vec2 vScale, UINT paintFlags)
{
	_ASSERT(pTexture != nullptr);
	_ASSERT(m_nVertexCursor < (K_BS_MAX_QUAD_CNT * 4) - 4);
	_ASSERT(bStarted == true);

	// Shader data:
	// Normal contains position
	Vec3 vecPos(vPos.x, vPos.y, 0.0f);
	// Tex2 contains: x: scale x, y: scale y, z: Z rotation
	Vec4 vecScaleRot(vScale.x, vScale.y, fRotationZ, 0.0f);

	// paint flags
	RectLTRB pUV(pSrcUV);
	if (paintFlags & K_SPRFLAG_FLIP_X)
	{
		pUV.left = pSrcUV.right;
		pUV.right = pSrcUV.left;
	}
	if (paintFlags & K_SPRFLAG_FLIP_Y)
	{
		pUV.top = pSrcUV.bottom;
		pUV.bottom = pSrcUV.top;
	}

	//ul
	m_verts[m_nVertexCursor].pos = Vec3(pDestRect.left, pDestRect.top, 0.0f);
	m_verts[m_nVertexCursor].n = vecPos;
	m_verts[m_nVertexCursor].tex1 = Vec4(pUV.left, pUV.top, 0.0f, 0.0f);
	m_verts[m_nVertexCursor].tex2 = vecScaleRot;
	m_verts[m_nVertexCursor].color = color;
	m_nVertexCursor++;
	//ur
	m_verts[m_nVertexCursor].pos = Vec3(pDestRect.right, pDestRect.top, 0.0f);
	m_verts[m_nVertexCursor].n = vecPos;
	m_verts[m_nVertexCursor].tex1 = Vec4(pUV.right, pUV.top, 0.0f, 0.0f);
	m_verts[m_nVertexCursor].tex2 = vecScaleRot;
	m_verts[m_nVertexCursor].color = color;
	m_nVertexCursor++;
	//dl
	m_verts[m_nVertexCursor].pos = Vec3(pDestRect.left, pDestRect.bottom, 0.0f);
	m_verts[m_nVertexCursor].n = vecPos;
	m_verts[m_nVertexCursor].tex1 = Vec4(pUV.left, pUV.bottom, 0.0f, 0.0f);
	m_verts[m_nVertexCursor].tex2 = vecScaleRot;
	m_verts[m_nVertexCursor].color = color;
	m_nVertexCursor++;
	//dr
	m_verts[m_nVertexCursor].pos = Vec3(pDestRect.right, pDestRect.bottom, 0.0f);
	m_verts[m_nVertexCursor].n = vecPos;
	m_verts[m_nVertexCursor].tex1 = Vec4(pUV.right, pUV.bottom, 0.0f, 0.0f);
	m_verts[m_nVertexCursor].tex2 = vecScaleRot;
	m_verts[m_nVertexCursor].color = color;
	m_nVertexCursor++;

	//see if texture changed
	if (pTexture != m_texPtrs[m_nTexChangesCursor])
	{
		if (m_texPtrs[m_nTexChangesCursor] == nullptr)
		{
			m_texPtrs[m_nTexChangesCursor] = pTexture;
			m_nTrisOffsets[m_nTexChangesCursor] = 0;
		}
		else
		{
			m_nTexChangesCursor++;

			m_texPtrs[m_nTexChangesCursor] = pTexture;
			m_nTrisOffsets[m_nTexChangesCursor] = m_nTrisOffsets[m_nTexChangesCursor - 1] + m_nTrisPerTexture[m_nTexChangesCursor - 1];
			_ASSERT(m_nTexChangesCursor < K_BS_MAX_MODECHANGES_CNT);
		}
	}
	m_nTrisPerTexture[m_nTexChangesCursor] += 2;

	return K_OP_OK;
}

void CSpritePainter::AdditiveBlendingOn()
{
	Flush();
	m_pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	m_pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
}

void CSpritePainter::AdditiveBlendingOff()
{
	Flush();
	m_pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
}

void CSpritePainter::SetClipWorld( RectXYWH clipWorldCoords )
{
	Flush();
	Vec2 vul(clipWorldCoords.x, clipWorldCoords.y);
	Vec2 vdr( clipWorldCoords.Right(), clipWorldCoords.Bottom() );
	Vec2 rul, rdr;
	Mat mWV = m_matWorld * m_matView;
	MUVec2TransformCoord( &rul, &vul, &mWV);
	MUVec2TransformCoord( &rdr, &vdr, &mWV);
	SetScissorClip( m_pDevice, (int)rul.x, (int)rul.y, (int)( rdr.x - rul.x ), (int)( rdr.y - rul.y ) );
}

void CSpritePainter::SetClip( RectXYWH clipCoord )
{
	Flush();
	SetScissorClip( m_pDevice, (int)clipCoord.x, (int)clipCoord.y, (int)clipCoord.w, (int)clipCoord.h );
}

void CSpritePainter::RemoveClip()
{
	Flush();
	RemoveScissorClip( m_pDevice );
}

//--- framework ---
OPRESULT CSpritePainter::OnCreateDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc )
{
	m_verts = new _VERTEX_PNCT4T4[(K_BS_MAX_QUAD_CNT + 2) * 4];

	m_pDevice = pDevice;
	//create index buffer (fixed) - deci va desena numai dreptunghiuri
	if (m_pDevice->CreateIndexBuffer((K_BS_MAX_QUAD_CNT + 2) * 6 * sizeof(DWORD), 0, D3DFMT_INDEX32, D3DPOOL_MANAGED, &m_ib, 0) != S_OK)
	{
		return OPRESULT(K_OP_FAILED, L"CSpritePainter::Failed to create index buffer!", K_SEVERITY_CRITICAL);
	}

	DWORD * pIndices;
	if (m_ib->Lock(0, NULL, (void**)&pIndices, 0) != S_OK)
	{
		return OPRESULT(K_OP_FAILED, L"CSpritePainter::Failed to lock index buffer!", K_SEVERITY_CRITICAL);
	}

	for (int kk = 0; kk < K_BS_MAX_QUAD_CNT; kk++)
	{
		pIndices[kk * 6 + 0] = (DWORD)(kk * 4 + 0);
		pIndices[kk * 6 + 1] = (DWORD)(kk * 4 + 1);
		pIndices[kk * 6 + 2] = (DWORD)(kk * 4 + 2);
		pIndices[kk * 6 + 3] = (DWORD)(kk * 4 + 2);
		pIndices[kk * 6 + 4] = (DWORD)(kk * 4 + 1);
		pIndices[kk * 6 + 5] = (DWORD)(kk * 4 + 3);
	}
	m_ib->Unlock();

	return K_OP_OK;
}

OPRESULT CSpritePainter::OnResetDevice( PDEVICE pDevice, const SURFACE_DESC* pBBDesc )
{
	m_pDevice = pDevice;

	//create vb and ib
	if (m_pDevice->CreateVertexBuffer((K_BS_MAX_QUAD_CNT + 2) * 4 * sizeof(_VERTEX_PNCT4T4),
		D3DUSAGE_WRITEONLY | D3DUSAGE_DYNAMIC,
		_VERTEX_PNCT4T4::FVF, D3DPOOL_DEFAULT,
		&m_vb, NULL) != S_OK)
	{
		SAFE_RELEASE(m_ib);
		return OPRESULT(K_OP_FAILED, L"CSpritePainter::Failed to create vertex buffer!", K_SEVERITY_CRITICAL);
	}

	return K_OP_OK;
}

OPRESULT CSpritePainter::OnLostDevice()
{
	SAFE_RELEASE(m_vb);

	return K_OP_OK;
}

OPRESULT CSpritePainter::OnDestroyDevice()
{
	SAFE_DELETE_ARRAY(m_verts);
	SAFE_RELEASE(m_ib);

	return K_OP_OK;
}



///**************************************************************************************
/// Sigleton 
///**************************************************************************************
CSpritePainter& __Painter()
{
	static CSpritePainter g_SpritePainter;
	return g_SpritePainter;
}

