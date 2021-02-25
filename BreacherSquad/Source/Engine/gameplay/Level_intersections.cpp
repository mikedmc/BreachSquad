#include "dxstdafx.h"

CCollisionShape* CLevel::ColShape_Segment_Intersection_Arr(Vec2 & start, Vec2 & end, CCollisionShape * arrBoxes[], int nBoxesCnt, Vec2 * retCollisionPoint, Vec2 * retNormal)
{
	//verificari initiale
	_ASSERT(arrBoxes != NULL);

	if (nBoxesCnt <= 0)
		return null;
	//calculeaza termeni segment
	Vec2 dir = end - start;
	float seglen = D3DXVec2Length(&dir);
	if (seglen <= 0.0f)
		return null;
	dir /= seglen;

	Vec2 dirfrac;
	// r.dir is unit direction vector of ray
	dirfrac.x = 1.0f / dir.x;
	dirfrac.y = 1.0f / dir.y;
	//tine intersectia minima
	float minTfinal = FLT_MAX;
	//fast check box - checks box box intersection before checking segment intersection
	CAABB boxCheck;
	boxCheck.Set_Corrected(start, end);
	//valoarea de return 
	CAABB* retBox = null;
	CCollisionShape* retShape = null;
	for (int kk = 0; kk < nBoxesCnt; kk++)
	{
		//cursorul prin arrBoxes
		CAABB* box = &arrBoxes[kk]->bbox;
		//vector bounding box noit intersecting target box means no collision
		if ((box->vMin.x > boxCheck.vMax.x) || (box->vMax.x < boxCheck.vMin.x) || (box->vMin.y > boxCheck.vMax.y) || (box->vMax.y < boxCheck.vMin.y))
			continue;

		float t1 = (box->vMin.x - start.x) * dirfrac.x;
		float t2 = (box->vMax.x - start.x) * dirfrac.x;
		float t3 = (box->vMin.y - start.y) * dirfrac.y;
		float t4 = (box->vMax.y - start.y) * dirfrac.y;

		float tmin = max(min(t1, t2), min(t3, t4));
		float tmax = min(max(t1, t2), max(t3, t4));

		// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behind us
		// if tmin > tmax, ray doesn't intersect AABB
		//daca tmin e mai mare decat lungimea segmentului inseamna ca se intersecteaza dupa al doilea punct
		//daca tmin e mai mare decat minTfinal inseamna ca am coliziune mai departata decat ultima verificata
		//AM INVERSAT if-ul ca sa fie mai scurt codul
		if ((tmax >= 0.0f) && (tmin <= tmax) && (tmin <= seglen) && (tmin < minTfinal))
		{
			retBox = box;
			retShape = arrBoxes[kk];
			minTfinal = tmin;
		}
	}
	//minTfinal contine procentul intersectiei
	if (retCollisionPoint != NULL)
	{
		if (retBox != null)
		{
			*retCollisionPoint = start + minTfinal * dir;
			//pentru normala: daca e intre ymin si ymax e coliziune cu latura verticala
			if (retNormal != NULL)
			{
				retNormal->x = retNormal->y = 0.0f;
				if ((retCollisionPoint->y > retBox->vMin.y) && (retCollisionPoint->y < retBox->vMax.y))
				{
					if (retCollisionPoint->x < retBox->vCenter.x)
						retNormal->x = -1.0f;
					else
						retNormal->x = 1.0f;
				}
				else
				{
					if (retCollisionPoint->y < retBox->vCenter.y)
						retNormal->y = -1.0f;
					else
						retNormal->y = 1.0f;
				}
			}
		}
		else
		{
			*retCollisionPoint = end;
			if (retNormal != null)
			{
				*retNormal = Vec2(0.0f, 0.0f);
			}
		}
	}
	//intorc boxul colizionat daca este cazul sau null daca nu a avut coliziune
	return retShape;
}


CCollisionShape* CLevel::ColShape_CAABB_Intersect_Arr(CAABB * aabbSrc, CCollisionShape * arrBoxes[], int nBoxesCnt)
{
	if (aabbSrc == null)
		return null;
	for (int kk = 0; kk < nBoxesCnt; kk++)
	{
		//cursorul prin arrBoxes
		CAABB* box = &arrBoxes[kk]->bbox;
		if (box->Intersects(aabbSrc))
			return arrBoxes[kk];
	}
	return null;
}


CTile* CLevel::SegmentTilesIntersection(Vec2 vStart, Vec2 vEnd, Vec2 & retPoint, Vec2 & retNormal, Vec2i *hitTilePosTL)
{
	CAABB segAABB;
	Vec2 vFrom = vStart;
	Vec2 vTo = vEnd;
	// selects all areas that can have positive hits and shortens the vector on collision so we always have the minimal one
	CTile* rettile = nullptr;
	for (auto area : m_arrAreas)
	{
		segAABB.Set_Corrected(vFrom, vTo);
		if (area->AABBbounds.Intersects(&segAABB))
		{
			Vec2 hitPt, hitN;
			Vec2i hitTL;
			CTile* tl = area->SegmentTilesIntersection(vStart, vEnd, hitPt, hitN, &hitTL);
			if (tl != nullptr)
			{
				// on collision shorten the vector so we elimintate areas that are farther away
				rettile = tl;
				retPoint = hitPt;
				retNormal = hitN;
				if (hitTilePosTL != nullptr)
					*hitTilePosTL = hitTL;
				// shorten the vector 
				vTo = hitPt;
			}
		}
	}


	return rettile;
}

