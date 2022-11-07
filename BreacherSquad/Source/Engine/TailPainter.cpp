#include "dxstdafx.h"
#include "TailPainter.h"


static _VERTEX_PNCT4T4 tpts[Tails::K_MAX_TAIL_POINTS * 6];

Vec2 Tails::GetJointMedian( Vec2 pJoint, Vec2 pAfter )
{
	Vec2 vToAfter = pAfter - pJoint;
	Vec2 vnAfter{};
	MUVec2Norm( &vnAfter, &vToAfter );
	// return normalized perpendicular vector to (toDest vector)
	return Vec2( -vnAfter.y, vnAfter.x );
}

int Tails::BuildTail( CBufferedPainter* pPainter, Vec2 * arrPos, int nPoints, float fWidth, RectLTRB & texRect )
{
	int nMeshIdx = -1;
	int vcur = 0;

	_ASSERT( nPoints > 1 && nPoints < K_MAX_TAIL_POINTS && arrPos != nullptr && pPainter != nullptr );
	pPainter->BeginMesh( nMeshIdx );

	float fTexAdv = texRect.Width() / (nPoints - 1);
	for ( int kk = 0; kk < nPoints - 1; kk++ )
	{
		Vec2 pPoint = arrPos[kk];
		Vec2 pNext = arrPos[kk + 1];
		Vec2 vToNext = pNext - pPoint;
		Vec2 pNextNext = ( kk == nPoints - 2 ) ? pNext + vToNext : arrPos[kk + 2];

		Vec2 vMedian = Tails::GetJointMedian( pPoint, pNext );
		Vec2 vMedianNext = Tails::GetJointMedian( pNext, pNextNext );

		Vec3 v3Median = Vec2ToVec3XY0( vMedian ) * fWidth;
		Vec3 v3MedianNext = Vec2ToVec3XY0( vMedianNext ) * fWidth;
		Vec3 v3Point = Vec2ToVec3XY0( pPoint );
		Vec3 v3Next = Vec2ToVec3XY0( pNext );

		RectLTRB UV( texRect.left + fTexAdv * kk, texRect.top, texRect.left + fTexAdv * (kk + 1), texRect.bottom );
		// add triangles
		tpts[vcur].pos = v3Point - v3Median; tpts[vcur].color = 0xffffffff; 
		tpts[vcur].tex1 = { UV.left, UV.top, 0.0f, 0.0f }; vcur++;
		tpts[vcur].pos = v3Next - v3MedianNext; tpts[vcur].color = 0xffffffff; 
		tpts[vcur].tex1 = { UV.right, UV.top, 0.0f, 0.0f }; vcur++;
		tpts[vcur].pos = v3Next + v3MedianNext; tpts[vcur].color = 0xffffffff;
		tpts[vcur].tex1 = { UV.right, UV.bottom, 0.0f, 0.0f }; vcur++;

		tpts[vcur].pos = v3Point - v3Median; tpts[vcur].color = 0xffffffff;
		tpts[vcur].tex1 = { UV.left, UV.top, 0.0f, 0.0f }; vcur++;
		tpts[vcur].pos = v3Next + v3MedianNext; tpts[vcur].color = 0xffffffff;
		tpts[vcur].tex1 = { UV.right, UV.bottom, 0.0f, 0.0f }; vcur++;
		tpts[vcur].pos = v3Point + v3Median; tpts[vcur].color = 0xffffffff;
		tpts[vcur].tex1 = { UV.left, UV.bottom, 0.0f, 0.0f }; vcur++;
	}

	pPainter->AddTriangles(tpts, vcur / 3);
	int nTris = pPainter->EndMesh();

	return nMeshIdx;
}
