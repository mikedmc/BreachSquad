#include "dxstdafx.h"
#include "Caabb.h"


CAABB::CAABB(D3DXVECTOR2 min, D3DXVECTOR2 max)
{
	vMin = min; vMax = max;
	vSize = vMax - vMin;
	vHalfSize = vSize / 2.0f;
	vCenter = vMin + vHalfSize;
}

CAABB::CAABB(float vminx, float vminy, float vmaxx, float vmaxy)
{
	vMin = D3DXVECTOR2(vminx, vminy); vMax = D3DXVECTOR2(vmaxx, vmaxy);
	vSize = vMax - vMin;
	vHalfSize = vSize / 2.0f;
	vCenter = vMin + vHalfSize;
}

void CAABB::Set(D3DXVECTOR2 min, D3DXVECTOR2 max)
{
	vMin = min; vMax = max;
	vSize = vMax - vMin;
	vHalfSize = vSize / 2.0f;
	vCenter = vMin + vHalfSize;
}

void CAABB::Set(RECTXYWH_F rect)
{
	vMin = D3DXVECTOR2(rect.x, rect.y); vMax = D3DXVECTOR2(rect.x + rect.w, rect.y + rect.h);
	vSize = vMax - vMin;
	vHalfSize = vSize / 2.0f;
	vCenter = vMin + vHalfSize;
}

void CAABB::Set(RECTLTRB_F rect)
{
	vMin = D3DXVECTOR2(rect.left, rect.top); vMax = D3DXVECTOR2(rect.right, rect.bottom);
	vSize = vMax - vMin;
	vHalfSize = vSize / 2.0f;
	vCenter = vMin + vHalfSize;
}


void CAABB::Set(CAABB* sourceAABB, D3DXVECTOR2 vOffset)
{
	Set(sourceAABB->vMin + vOffset, sourceAABB->vMax + vOffset);
}

void CAABB::Set(float xmin, float ymin, float xmax, float ymax)
{
	vMin.x = xmin; vMin.y = ymin;
	vMax.x = xmax; vMax.y = ymax;
	Set(vMin, vMax);
}

// checks min and max points before setting the bbox
void CAABB::Set_Corrected(D3DXVECTOR2 pt1, D3DXVECTOR2 pt2)
{
	D3DXVECTOR2 min, max;
	min.x = min(pt1.x, pt2.x); min.y = min(pt1.y, pt2.y);
	max.x = max(pt1.x, pt2.x); max.y = max(pt1.y, pt2.y);
	Set(min, max);
}

void CAABB::Move(Vec2 delta)
{
	vMin += delta;
	vMax += delta;
	Set(vMin, vMax);
}

void CAABB::Flip(bool flipX, bool flipY)
{
	if (flipX)
	{
		SWAP(vMin.x, vMax.x);
		vMin.x = -vMin.x; vMax.x = -vMax.x;
	}
	if (flipY)
	{
		SWAP(vMin.y, vMax.y);
		vMin.y = -vMin.y; vMax.y = -vMax.y;
	}

	Set_Corrected(vMin, vMax);
}

RECTXYWH CAABB::as_RECTXYWH()
{
	RECTXYWH retval;
	retval.Set(vMin.x, vMin.y, vSize.x, vSize.y);
	return retval;
}

void CAABB::Inflate(D3DXVECTOR2 delta)
{
	vMin -= delta;
	vMax += delta;
	Set(vMin, vMax);
}

void CAABB::Inflate(float dX, float dY)
{
	vMin.x -= dX; vMin.y -= dY;
	vMax.x += dX; vMax.y += dY;
	Set(vMin, vMax);
}

void CAABB::Scale(float fScalePercent)
{
	vHalfSize *= fScalePercent;
	vMin = vCenter - vHalfSize;
	vMax = vCenter + vHalfSize;
	Set(vMin, vMax);
}


///--- teste pe AABB ---
bool CAABB::PointIn(D3DXVECTOR2 pt1)
{
	if ((pt1.x < vMin.x) || (pt1.x > vMax.x) || (pt1.y < vMin.y) || (pt1.y > vMax.y))
		return false;
	return true;
}

bool CAABB::PointIn(float x, float y)
{
	if ((x < vMin.x) || (x > vMax.x) || (y < vMin.y) || (y > vMax.y))
		return false;
	return true;
}

bool CAABB::Intersects(CAABB *dest)
{
	assert(dest != null);

	if ((vMin.x > dest->vMax.x) || (vMax.x < dest->vMin.x) || (vMin.y > dest->vMax.y) || (vMax.y < dest->vMin.y))
		return false;
	return true;
}

bool CAABB::IntersectsCircle(D3DXVECTOR2 center, float radius)
{
	if ((center.x < vMin.x - radius) || (center.x > vMax.x + radius) || (center.y < vMin.y - radius) || (center.y > vMax.y + radius))
		return false;
	return true;

}

CAABB AABB_Lerp(CAABB &a, CAABB &b, float fFactor)
{
	float fac = LIMIT(fFactor, 0.0f, 1.0f);
	float invfac = 1.0f - fac;
	D3DXVECTOR2 vmin = a.vMin * invfac + b.vMin * fac;
	D3DXVECTOR2 vmax = a.vMax * invfac + b.vMax * fac;

	return CAABB(vmin, vmax);
}

bool AABB_Intersection(CAABB & a, CAABB & b, CAABB & retVal)
{
	D3DXVECTOR2 vMax, vMin;
	vMin.x = max(a.vMin.x, b.vMin.x);
	vMin.y = max(a.vMin.y, b.vMin.y);
	vMax.x = min(a.vMax.x, b.vMax.x);
	vMax.y = min(a.vMax.y, b.vMax.y);
	retVal.Set(vMin, vMax);

	//negative means no intersection
	if ((vMax.x < vMin.x) || (vMax.y < vMin.y))
		return false;
	return true;
}

CAABB AABB_Union(CAABB &a, CAABB &b)
{
	D3DXVECTOR2 vMax, vMin;
	vMin.x = min(a.vMin.x, b.vMin.x);
	vMin.y = min(a.vMin.y, b.vMin.y);
	vMax.x = max(a.vMax.x, b.vMax.x);
	vMax.y = max(a.vMax.y, b.vMax.y);
	
	return CAABB(vMin, vMax);
}

CAABB AABB_FromPoints(D3DXVECTOR2 * vecArr, int vecCnt)
{
	CAABB retAABB;
	if ((vecArr == NULL) || (vecCnt == 0))
		return retAABB;
	D3DXVECTOR2 max = D3DXVECTOR2(-100000.0f, -100000.0f);
	D3DXVECTOR2 min = D3DXVECTOR2(100000.0f, 100000.0f);
	for (int kk = 0; kk < vecCnt; kk++)
	{
		D3DXVECTOR2 *v = &vecArr[kk];
		if (v->x < min.x) min.x = v->x;
		if (v->x > max.x) max.x = v->x;
		if (v->y < min.y) min.y = v->y;
		if (v->y > max.y) max.y = v->y;
	}

	retAABB.Set(min, max);
	return retAABB;
}

CAABB AABB_FromPoints(D3DXVECTOR3 * vecArr, int vecCnt)
{
	CAABB retAABB;
	if ((vecArr == NULL) || (vecCnt == 0))
		return retAABB;
	D3DXVECTOR2 max = D3DXVECTOR2(-100000.0f, -100000.0f);
	D3DXVECTOR2 min = D3DXVECTOR2(100000.0f, 100000.0f);
	for (int kk = 0; kk < vecCnt; kk++)
	{
		D3DXVECTOR3 *v = &vecArr[kk];
		if (v->x < min.x) min.x = v->x;
		if (v->x > max.x) max.x = v->x;
		if (v->y < min.y) min.y = v->y;
		if (v->y > max.y) max.y = v->y;
	}

	retAABB.Set(min, max);
	return retAABB;
}

bool AABB_KeepInside(CAABB & boxSource, CAABB & boxDest)
{
	//can't fit
	if ((boxSource.vSize.x > boxDest.vSize.x) || (boxSource.vSize.y > boxDest.vSize.y))
		return false;

	D3DXVECTOR2 delta = boxSource.vMin - boxDest.vMin;
	if (delta.x >= 0.0f) delta.x = 0.0f;
	if (delta.y >= 0.0f) delta.y = 0.0f;
	boxSource.Move(-delta);

	delta = boxDest.vMax - boxSource.vMax;
	if (delta.x >= 0.0f) delta.x = 0.0f;
	if (delta.y >= 0.0f) delta.y = 0.0f;
	boxSource.Move(delta);

	return true;
}

bool AABB_Segment_Intersection(D3DXVECTOR2 & start, D3DXVECTOR2 & end, CAABB & box, D3DXVECTOR2 * retCollisionPoint)
{
	//calculeaza termeni segment
	D3DXVECTOR2 dir = end - start;
	//make sure we don't have horizontal lines
	if (dir.x == 0.0f)
		dir.x = EPS;
	if (dir.y == 0.0f)
		dir.y = EPS;

	float seglen = D3DXVec2Length(&dir);
	//daca lungimea e 0 iese cu false
	if (seglen == 0.0f)
		return false;

	dir /= seglen;
	float dirfracx, dirfracy;
	// r.dir is unit direction vector of ray
	dirfracx = 1.0f / dir.x;
	dirfracy = 1.0f / dir.y;

	float t1 = (box.vMin.x - start.x) * dirfracx;
	float t2 = (box.vMax.x - start.x) * dirfracx;
	float t3 = (box.vMin.y - start.y) * dirfracy;
	float t4 = (box.vMax.y - start.y) * dirfracy;

	float tmin = max(min(t1, t2), min(t3, t4));
	float tmax = min(max(t1, t2), max(t3, t4));

	// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behind us
	if (tmax < 0.0f)
		return false;
	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax)
		return false;
	//daca tmin e mai mare decat lungimea segmentului inseamna ca se intersecteaza dupa al doilea punct
	if (tmin > seglen)
		return false;

	//tmin contine procentul intersectiei
	if (retCollisionPoint != NULL)
		*retCollisionPoint = start + tmin * dir;

	return true;
}

bool AABB_Segment_IntersectionEx(D3DXVECTOR2 & start, D3DXVECTOR2 & end, CAABB & box, D3DXVECTOR2 * retCollisionPoint, float &fRetT)
{
	//calculeaza termeni segment
	D3DXVECTOR2 dir = end - start;
	//make sure we don't have horizontal lines
	if (dir.x == 0.0f)
		dir.x = EPS;
	if (dir.y == 0.0f)
		dir.y = EPS;

	float seglen = D3DXVec2Length(&dir);
	//daca lungimea e 0 iese cu false
	if (seglen == 0.0f)
		return false;

	dir /= seglen;
	float dirfracx, dirfracy;
	// r.dir is unit direction vector of ray
	dirfracx = 1.0f / dir.x;
	dirfracy = 1.0f / dir.y;

	float t1 = (box.vMin.x - start.x) * dirfracx;
	float t2 = (box.vMax.x - start.x) * dirfracx;
	float t3 = (box.vMin.y - start.y) * dirfracy;
	float t4 = (box.vMax.y - start.y) * dirfracy;

	float tmin = max(min(t1, t2), min(t3, t4));
	float tmax = min(max(t1, t2), max(t3, t4));

	fRetT = -1.0f;
	// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behind us
	if (tmax < 0.0f)
		return false;
	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax)
		return false;
	//daca tmin e mai mare decat lungimea segmentului inseamna ca se intersecteaza dupa al doilea punct
	if (tmin > seglen)
		return false;

	//tmin contine procentul intersectiei
	if (retCollisionPoint != NULL)
		*retCollisionPoint = start + tmin * dir;

	//tmin contine procentul intersectiei
	fRetT = tmin / seglen;

	return true;
}

bool AABB_Segment_Intersection_NoHeads(D3DXVECTOR2 & start, D3DXVECTOR2 & end, CAABB & box, D3DXVECTOR2 * retCollisionPoint)
{
	//calculeaza termeni segment
	D3DXVECTOR2 dir = end - start;
	//make sure we don't have horizontal lines
	if (dir.x == 0.0f)
		dir.x = EPS;
	if (dir.y == 0.0f)
		dir.y = EPS;

	float seglen = D3DXVec2Length(&dir);
	//daca lungimea e 0 iese cu false
	if (seglen == 0.0f)
		return false;

	dir /= seglen;
	float dirfracx, dirfracy;
	// r.dir is unit direction vector of ray
	dirfracx = 1.0f / dir.x;
	dirfracy = 1.0f / dir.y;

	float t1 = (box.vMin.x - start.x) * dirfracx;
	float t2 = (box.vMax.x - start.x) * dirfracx;
	float t3 = (box.vMin.y - start.y) * dirfracy;
	float t4 = (box.vMax.y - start.y) * dirfracy;

	float tmin = max(min(t1, t2), min(t3, t4));
	float tmax = min(max(t1, t2), max(t3, t4));

	//sa nu intoarca coliziune in spate
	if (tmin <= 0.0f)
		return false;
	// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behind us
	if (tmax < 0.0f)
		return false;
	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax)
		return false;
	//daca tmin e mai mare decat lungimea segmentului inseamna ca se intersecteaza dupa al doilea punct
	if (tmin >= seglen)
		return false;

	//tmin contine procentul intersectiei
	if (retCollisionPoint != NULL)
		*retCollisionPoint = start + tmin * dir;

	return true;
}

CAABB* AABB_Segment_Intersection_Arr(D3DXVECTOR2 & start, D3DXVECTOR2 & end, CAABB * arrBoxes[], int nBoxesCnt, D3DXVECTOR2 * retCollisionPoint, D3DXVECTOR2 * retNormal)
{
	//verificari initiale
	assert(arrBoxes != NULL);

	if (nBoxesCnt <= 0)
		return null;
	//calculeaza termeni segment
	D3DXVECTOR2 dir = end - start;
	//make sure we don't have horizontal lines
	if (dir.x == 0.0f)
		dir.x = EPS;
	if (dir.y == 0.0f)
		dir.y = EPS;

	float seglen = D3DXVec2Length(&dir);
	dir /= seglen;
	float dirfracx, dirfracy;
	// r.dir is unit direction vector of ray
	dirfracx = 1.0f / dir.x;
	dirfracy = 1.0f / dir.y;
	//tine intersectia minima
	float minTfinal = FLT_MAX;
	//valoarea de return 
	CAABB* retBox = null;
	for (int kk = 0; kk < nBoxesCnt; kk++ )
	{
		//cursorul prin arrBoxes
		CAABB* box = arrBoxes[kk];

		float t1 = (box->vMin.x - start.x) * dirfracx;
		float t2 = (box->vMax.x - start.x) * dirfracx;
		float t3 = (box->vMin.y - start.y) * dirfracy;
		float t4 = (box->vMax.y - start.y) * dirfracy;

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
	return retBox;
}

CAABB AABB_GetMinkowskiDifference(CAABB &a, CAABB &b)
{
	CAABB retaabb;

	D3DXVECTOR2 topleft = a.vMin - b.vMax;
	// alternativ:
	//Vec2 sztotal = a.vSize + b.vSize;
	//Vec2 bottomright = topleft + sztotal;
	D3DXVECTOR2 bottomright = a.vMax - b.vMin;

	return CAABB(topleft, bottomright);
}

void AABB_MorphInto_linear(CAABB *source, CAABB *target, float fSpeed)
{
	if ((source == null) || (target == null))
		return;

	int foundpts = 0;
	D3DXVECTOR2 vdstm = target->vMin - source->vMin;
	float vlenm = D3DXVec2Length(&vdstm);
	if (vlenm <= fSpeed)
	{
		//point reached
		source->vMin = target->vMin;
	}
	else
	{
		vdstm /= vlenm;
		vdstm *= fSpeed;
		source->vMin += vdstm;
	}

	vdstm = target->vMax - source->vMax;
	vlenm = D3DXVec2Length(&vdstm);
	if (vlenm <= fSpeed)
	{
		//point reached
		source->vMax = target->vMax;
	}
	else
	{
		vdstm /= vlenm;
		vdstm *= fSpeed;
		source->vMax += vdstm;
	}
	//set all other data
	source->Set(source->vMin, source->vMax);
}

void AABB_MorphInto_quadratic(CAABB *source, CAABB *target, float fDistMultiplier, float fMinSpeed)
{
	if ((source == null) || (target == null))
		return;

	int foundpts = 0;
	D3DXVECTOR2 vdstm = target->vMin - source->vMin;
	float vlenm = D3DXVec2Length(&vdstm);
	if (vlenm <= fMinSpeed)
	{
		//point reached
		source->vMin = target->vMin;
	}
	else
	{
		vdstm /= vlenm; //normalizez
		float fspd = max(vlenm * fDistMultiplier, fMinSpeed);
		source->vMin += vdstm * fspd;
	}

	vdstm = target->vMax - source->vMax;
	vlenm = D3DXVec2Length(&vdstm);
	if (vlenm <= fMinSpeed)
	{
		//point reached
		source->vMax = target->vMax;
	}
	else
	{
		vdstm /= vlenm; //normalizez
		float fspd = max(vlenm * fDistMultiplier, fMinSpeed);
		source->vMax += vdstm * fspd;
	}
	//set all other data
	source->Set(source->vMin, source->vMax);

}

D3DXVECTOR2 AABB_GetRandomPointInBox(CAABB &a)
{
	return D3DXVECTOR2(a.vMin.x + randfloat(a.vSize.x), a.vMin.y + randfloat(a.vSize.y));
}

