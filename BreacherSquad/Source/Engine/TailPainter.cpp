#include "dxstdafx.h"
#include "TailPainter.h"


static _VERTEX_PNCT4T4 tpts[Tails::K_MAX_TAIL_POINTS * 6];
int Tails::BuildTail( CBufferedPainter* pPainter, Vec2 * arrPos, int nPoints, float fWidth )
{
	int nMeshIdx = -1;
	int vcur = 0;

	_ASSERT( nPoints > 1 && nPoints < K_MAX_TAIL_POINTS && arrPos != nullptr && pPainter != nullptr );
	pPainter->BeginMesh( nMeshIdx );

	for ( int kk = nPoints - 1; kk > 0; kk-- )
	{
		Vec2 vTan{};
		Vec3 vA(arrPos[kk].x, arrPos[kk].y, 0.0f);
		Vec3 vB( arrPos[kk - 1].x, arrPos[kk - 1].y, 0.0f );
		Vec2 vTo = vB - vA;
		Vec2 vnTo;
		MUVec2Norm( &vnTo, &vTo );
		// normal:
		vTan.x = -vnTo.y; vTan.y = vnTo.x;
		Vec3 v3Tan = Vec2ToVec3XY0( vTan ) * fWidth;
		// add triangles
		tpts[vcur].pos = vA; tpts[vcur].color = 0xffffffff; vcur++;
		tpts[vcur].pos = vB; tpts[vcur].color = 0xffffffff; vcur++;
		tpts[vcur].pos = vB + v3Tan; tpts[vcur].color = 0xffffffff; vcur++;

		tpts[vcur].pos = vA; tpts[vcur].color = 0xffffffff; vcur++;
		tpts[vcur].pos = vB + v3Tan; tpts[vcur].color = 0xffffffff; vcur++;
		tpts[vcur].pos = vA + v3Tan; tpts[vcur].color = 0xffffffff; vcur++;
	}

	pPainter->AddTriangles(tpts, vcur / 3);
	int nTris = pPainter->EndMesh();

	return nMeshIdx;
}
