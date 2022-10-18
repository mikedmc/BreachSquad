#include "dxstdafx.h"
#include "BufferedSpinePainter.h"


///----------------------------------------------------
/// CBufferedSpinePainter
///----------------------------------------------------

CBufferedSpinePainter::CBufferedSpinePainter()
{
	passesCnt = 0;
}

CBufferedSpinePainter::~CBufferedSpinePainter()
{
	Clear();
}

int CBufferedSpinePainter::BufferMesh(_VERTEX_PNCT4T4 *points, int trisCount, CSpineTex* pTex, EBlendMode eMode, UINT dwUID)
{
	int retPassIdx = -1;
	//if we have no mesh or if last mesh has another mode or texture then we initialize another mesh
	if ((passesCnt == 0) || (arrPasses[passesCnt - 1].dwSkelUID != dwUID) || (arrPasses[passesCnt - 1].eMode != eMode) || (arrPasses[passesCnt - 1].pTex != pTex))
	{
		if (!OP_FAILED(BeginMesh(arrPasses[passesCnt].nMeshIdx)))
		{
			arrPasses[passesCnt].pTex = pTex;
			arrPasses[passesCnt].eMode = eMode;
			arrPasses[passesCnt].dwSkelUID = dwUID;
			// saves current pass and returns it ONLY when starting a new mesh
			retPassIdx = passesCnt;
			passesCnt++;
		}
	}

	AddTriangles(points, trisCount);

	return retPassIdx;
}

void CBufferedSpinePainter::Clear()
{
	passesCnt = 0;
	ClearBuffers();
}

void CBufferedSpinePainter::Paint(bool setFVF /*= true*/, ETexChannel eChannel)
{
	if (eChannel == K_TEXCHAN_NONE)
		return;
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
		//else if (eChannel == K_TEXCHAN_SPECULARMAP)
			//m_pDevice->SetTexture(0, arrPasses[kk].pTex->pTexture_S);

		DrawMesh(arrPasses[kk].nMeshIdx, false);
	}
}

OPRESULT CBufferedSpinePainter::PaintPass(int nPassIdx, bool setFVF /*= true*/, ETexChannel eChannel /*= K_TEXCHAN_COLORMAP*/)
{
	if (eChannel == K_TEXCHAN_NONE)
		return K_OP_OK;
	if ((nPassIdx < 0) || (nPassIdx >= passesCnt))
		return OPRESULT(K_OP_OK_WARNING, L"CBufferedSpinePainter::PaintPass: Pass outside bounds!", K_SEVERITY_WARNING);

	//#TODO: set blending modes
	switch (arrPasses[nPassIdx].eMode)
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
	if (eChannel == K_TEXCHAN_COLORMAP)
		m_pDevice->SetTexture(0, arrPasses[nPassIdx].pTex->pTexture);
	else if (eChannel == K_TEXCHAN_NORMALMAP)
		m_pDevice->SetTexture(0, arrPasses[nPassIdx].pTex->pTexture_N);
	//else if (eChannel == K_TEXCHAN_SPECULARMAP)
		//m_pDevice->SetTexture(0, arrPasses[kk].pTex->pTexture_S);

	DrawMesh(arrPasses[nPassIdx].nMeshIdx, setFVF);

	return K_OP_OK;
}

