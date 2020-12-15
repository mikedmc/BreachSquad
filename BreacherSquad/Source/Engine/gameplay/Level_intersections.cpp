#include "dxstdafx.h"

CCollisionShape* CLevel::ColShape_Segment_Intersection_Arr(D3DXVECTOR2 & start, D3DXVECTOR2 & end, CCollisionShape * arrBoxes[], int nBoxesCnt, D3DXVECTOR2 * retCollisionPoint, D3DXVECTOR2 * retNormal)
{
	//verificari initiale
	_ASSERT(arrBoxes != NULL);

	if (nBoxesCnt <= 0)
		return null;
	//calculeaza termeni segment
	D3DXVECTOR2 dir = end - start;
	float seglen = D3DXVec2Length(&dir);
	if (seglen <= 0.0f)
		return null;
	dir /= seglen;

	D3DXVECTOR2 dirfrac;
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
				*retNormal = D3DXVECTOR2(0.0f, 0.0f);
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


bool CLevel::SegmentTilesIntersection(Vec2 vStart, Vec2 vEnd, Vec2 & retPoint, Vec2 & retNormal, Vec2i *hitTilePosTL)
{
	// Works by walking from tile to tile on slopes, on X axis and Y axis then finding the closest point
	//#INFO: when going from right to left and bottom to top, if the end point is on the tile border it doesn't detect the intersection. Might happen to slow moving bullets but it should be fine.

	//#TODO: de pus tileflags options la coliziuni

	Vec2i startTL((int)floor(vStart.x / K_TILE_SIZE), (int)floor(vStart.y / K_TILE_SIZE));
	Vec2i endTL((int)floor(vEnd.x / K_TILE_SIZE), (int)floor(vEnd.y / K_TILE_SIZE));

	// check if current start is non walkable
	if (NIS_FLAG_ANY(tiles[startTL.x][startTL.y].flags, K_TILEFLAG_WALKABLE))
		return false;

	Vec2 vDir = vEnd - vStart;
	float fDirLen = MUVec2Len(&vDir);
	Vec2 vDirN = vDir / fDirLen;

	bool bFoundV = false;
	Vec2 vRetPtV(0.0f, 0.0f);
	Vec2 vRetNrmV(0.0f, 0.0f);
	Vec2i chktlV(0, 0);

	if (startTL.x != endTL.x)
	{
		// find first vertical grid collisions
		if (vDir.x < 0.0f)
		{
			float vLimit = max(0.0f, vEnd.x);
			// distance to margin
			float dstX = vStart.x - startTL.x * K_TILE_SIZE;
			// find first intersection with vertical axes
			float vecmul = dstX / fabs(vDirN.x);
			Vec2 vFrom(vStart.x + vDirN.x * vecmul, vStart.y + vDirN.y * vecmul);
			Vec2 vStep(SIGN(vDirN.x) * K_TILE_SIZE, vDirN.y * (K_TILE_SIZE / fabs(vDirN.x)));
			// walk from tile to tile horizontally until destination
			Vec2 vCur = vFrom;
			// test collisions on left side
			while (vCur.x >= vLimit)
			{
				chktlV = Vec2i((int)((vCur.x - K_TILE_SIZE / 2.0f) / K_TILE_SIZE), (int)(vCur.y / K_TILE_SIZE));
				//outside map?
				if ((chktlV.y < 0) || (chktlV.y >= levelSizeTL.w))
					break;

				if (NIS_FLAG_ANY(tiles[chktlV.x][chktlV.y].flags, K_TILEFLAG_WALKABLE))
				{
					bFoundV = true;
					vRetPtV = vCur;
					vRetNrmV = Vec2(1.0f, 0.0f);
					break;
				}
				vCur.x += vStep.x;
				vCur.y += vStep.y;
			}
		}
		else if (vDir.x > 0.0f)
		{
			float vLimit = min(vEnd.x, m_levelAABB.Right());
			// distance to margin
			float dstX = (startTL.x + 1) * K_TILE_SIZE - vStart.x;
			// find first intersection with vertical axes
			float vecmul = dstX / fabs(vDirN.x);
			Vec2 vFrom(vStart.x + vDirN.x * vecmul, vStart.y + vDirN.y * vecmul);
			Vec2 vStep(SIGN(vDirN.x) * K_TILE_SIZE, vDirN.y * (K_TILE_SIZE / fabs(vDirN.x)));
			// walk from tile to tile horizontally until destination
			Vec2 vCur = vFrom;
			// test collisions on right side
			while (vCur.x <= vLimit)
			{
				chktlV = Vec2i((int)((vCur.x + K_TILE_SIZE / 2.0f) / K_TILE_SIZE), (int)(vCur.y / K_TILE_SIZE));
				//outside map?
				if ((chktlV.y < 0) || (chktlV.y >= levelSizeTL.w))
					break;
				if (NIS_FLAG_ANY(tiles[chktlV.x][chktlV.y].flags, K_TILEFLAG_WALKABLE))
				{
					bFoundV = true;
					vRetPtV = vCur;
					vRetNrmV = Vec2(-1.0f, 0.0f);
					break;
				}
				vCur.x += vStep.x;
				vCur.y += vStep.y;
			}
		}
	}

	// Horizontal axes
	bool	bFoundH = false;
	Vec2	vRetPtH(0.0f, 0.0f);
	Vec2	vRetNrmH(0.0f, 0.0f);
	Vec2i	chktlH(0, 0);			// return hit tile pos
	if (startTL.y != endTL.y)
	{
		// find first vertical grid collisions
		if (vDir.y < 0.0f)
		{
			float vLimit = max(0.0f, vEnd.y);
			// distance to margin
			float dstY = vStart.y - startTL.y * K_TILE_SIZE;
			// find first intersection with vertical axes
			float vecmul = dstY / fabs(vDirN.y);
			Vec2 vFrom(vStart.x + vDirN.x * vecmul, vStart.y + vDirN.y * vecmul);
			Vec2 vStep(vDirN.x * (K_TILE_SIZE / fabs(vDirN.y)), SIGN(vDirN.y) * K_TILE_SIZE);
			// walk from tile to tile horizontally until destination
			Vec2 vCur = vFrom;
			// test collisions on left side
			while (vCur.y >= vLimit)
			{	
				chktlH = Vec2i((int)((vCur.x) / K_TILE_SIZE), (int)((vCur.y - K_TILE_SIZE / 2.0f) / K_TILE_SIZE));
				//outside map?
				if ((chktlH.x < 0) || (chktlH.x >= levelSizeTL.h))
					break;
				if (NIS_FLAG_ANY(tiles[chktlH.x][chktlH.y].flags, K_TILEFLAG_WALKABLE))
				{
					bFoundH = true;
					vRetPtH = vCur;
					vRetNrmH = Vec2(0.0f, 1.0f);
					break;
				}
				vCur.x += vStep.x;
				vCur.y += vStep.y;
			}
		}
		else if (vDir.y > 0.0f)
		{
			float vLimit = min(vEnd.y, m_levelAABB.Bottom());
			// distance to margin
			float dstY = (startTL.y + 1) * K_TILE_SIZE - vStart.y;
			// find first intersection with vertical axes
			float vecmul = dstY / fabs(vDirN.y);
			Vec2 vFrom(vStart.x + vDirN.x * vecmul, vStart.y + vDirN.y * vecmul);
			Vec2 vStep(vDirN.x * (K_TILE_SIZE / fabs(vDirN.y)), SIGN(vDirN.y) * K_TILE_SIZE);
			// walk from tile to tile horizontally until destination
			Vec2 vCur = vFrom;
			// test collisions on right side
			while (vCur.y <= vLimit)
			{
				chktlH = Vec2i((int)((vCur.x) / K_TILE_SIZE), (int)((vCur.y + K_TILE_SIZE / 2) / K_TILE_SIZE));
				//outside map?
				if ((chktlH.x < 0) || (chktlH.x >= levelSizeTL.h))
					break;
				if (NIS_FLAG_ANY(tiles[chktlH.x][chktlH.y].flags, K_TILEFLAG_WALKABLE))
				{
					bFoundH = true;
					vRetPtH = vCur;
					vRetNrmH = Vec2(0.0f, -1.0f);
					break;
				}
				vCur.x += vStep.x;
				vCur.y += vStep.y;
			}
		}
	}


	// return closest value
	if (bFoundH && bFoundV)
	{
		float minH = MUVec2LenSq(&(vRetPtH - vStart));
		float minV = MUVec2LenSq(&(vRetPtV - vStart));
		if (minV < minH)
		{
			retPoint	= vRetPtV;
			retNormal	= vRetNrmV;
			if (hitTilePosTL) *hitTilePosTL = chktlV;
		}
		else
		{
			retPoint	= vRetPtH;
			retNormal	= vRetNrmH;
			if (hitTilePosTL) *hitTilePosTL = chktlH;
		}
		return true;
	}
	else if (bFoundH)
	{
		retPoint	= vRetPtH;
		retNormal	= vRetNrmH;
		if (hitTilePosTL) *hitTilePosTL = chktlH;
		return true;
	}
	else if (bFoundV)
	{
		retPoint	= vRetPtV;
		retNormal	= vRetNrmV;
		if (hitTilePosTL) *hitTilePosTL = chktlV;
		return true;
	}

	// no collisions
	return false;
}

