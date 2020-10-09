#include "dxstdafx.h"
#include "mathutil.h"

///------------------------------------------------------------------------------------------------------------------------------------------
///  Generic math equations
///------------------------------------------------------------------------------------------------------------------------------------------

float MATH_LineDist(D3DXVECTOR3 p, D3DXVECTOR3 v1, D3DXVECTOR3 v2, D3DXVECTOR3 *n)
{
	D3DXVECTOR3 v1p, v2p, v1v2, nr;
	float a, b, c, prj;

	v1p = v1 - p;
	v2p = v2 - p;
	v1v2 = v1 - v2;
	a = D3DXVec3Length(&v1v2);
	b = D3DXVec3Length(&v1p);
	c = D3DXVec3Length(&v2p);
	D3DXVec3Normalize(&v1v2, &v1v2);
	prj = D3DXVec3Dot(&v1p, &v1v2);

	if (prj<0)
	{
		D3DXVec3Normalize(n, &(-v1p));
		return b;
	}
	else
		if (prj>a)
		{
			D3DXVec3Normalize(n, &(-v2p));
			return c;
		}
		else
		{
			v1v2 *= prj;
			nr = v1v2 - v1p;
			D3DXVec3Normalize(n, &nr);

			return D3DXVec3Length(&nr);
		}
}


bool MATH_LineLineIntersection(D3DXVECTOR2 p1, D3DXVECTOR2 p2, D3DXVECTOR2 p3, D3DXVECTOR2 p4, D3DXVECTOR2 *outPt)
{
	// Store the values for fast access and easy
	// equations-to-code conversion
	float x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
	float y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;

	float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	// If d is zero, there is no intersection
	if (d == 0.0f)
		return false;

	// Get the x and y
	float pre = (x1 * y2 - y1 * x2), post = (x3 * y4 - y3 * x4);
	float x = (pre * (x3 - x4) - (x1 - x2) * post) / d;
	float y = (pre * (y3 - y4) - (y1 - y2) * post) / d;

	// Check if the x and y coordinates are within both lines
	//aici verifica daca se intersecteaza segmentele...cred
	//if (x < min(x1, x2) || x > max(x1, x2) ||
	//x < min(x3, x4) || x > max(x3, x4)) return NULL;
	//if (y < min(y1, y2) || y > max(y1, y2) ||
	//y < min(y3, y4) || y > max(y3, y4)) return NULL;

	// Return the point of intersection
	if (outPt)
	{
		outPt->x = x;
		outPt->y = y;
	}
	return true;
}

void MATH_EaseTo_quadratic(float * current, float target, float fDistMultiplier, float fMinSpeed)
{
	if (current == null)
		return;

	float dst = target - *current;
	if (fabs(dst) <= fMinSpeed)
	{
		//point reached
		*current = target;
	}
	else
	{
		float fspd = max(fabs(dst * fDistMultiplier), fMinSpeed);
		*current += SIGN(dst) * fspd;
	}
}

void MATH_EaseTo_linear(float * current, float target, float fSpeed)
{
	if (current == null)
		return;

	float dst = target - *current;
	if (fabs(dst) <= fSpeed)
		*current = target;
	else
		*current += SIGN(dst) * fSpeed;
}


float MATH_GetAlphaOnDomainEnds(float fCursor, float fDomainLength, float fAlphaRegionSize)
{
	CLAMP(fCursor, 0.0f, fDomainLength);
	if (fCursor < fAlphaRegionSize)
		return (fCursor / fAlphaRegionSize);
	if (fCursor > fDomainLength - fAlphaRegionSize)
		return ((fDomainLength - fCursor) / fAlphaRegionSize);
	return 1.0f;
}

bool MATH_IsPowerOfTwo(unsigned int nVal)
{
	return ((nVal != 0) && !(nVal & (nVal - 1)));
}

float Math_GetVectorAngle(D3DXVECTOR2 start, D3DXVECTOR2 end)
{
	D3DXVECTOR2 vecdir = end - start;
	float len = D3DXVec2LengthSq(&vecdir);
	if (len > 0.0f)
	{
		return((float)(HALF_PI - atan2(vecdir.x, vecdir.y)));
	}
	return 0.0f;
}

float Math_GetVectorAngle(D3DXVECTOR2 const &dir)
{
	float len = D3DXVec2LengthSq(&dir);
	if (len > 0.0f)
	{
		return((float)(HALF_PI - atan2(dir.x, dir.y)));
	}
	return 0.0f;
}

float Math_GetAngleBetweenVectors(D3DXVECTOR2 vec1, D3DXVECTOR2 vec2)
{
	D3DXVECTOR2 v1, v2;
	D3DXVec2Normalize(&v1, &vec1);
	D3DXVec2Normalize(&v2, &vec2);
	float ang = acos(D3DXVec2Dot(&v1, &v2));
	return ang;
}


//--- AABB ---
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

//seteaza corect bbox indiferent de pozitia punctelor
void CAABB::Set_Corrected(D3DXVECTOR2 pt1, D3DXVECTOR2 pt2)
{
	D3DXVECTOR2 min, max;
	min.x = min(pt1.x, pt2.x); min.y = min(pt1.y, pt2.y);
	max.x = max(pt1.x, pt2.x); max.y = max(pt1.y, pt2.y);
	Set(min, max);
}

void CAABB::Move(D3DXVECTOR2 delta)
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

///--- operatii pe aabb ---
bool AABB_Intersection(CAABB & a, CAABB & b, CAABB & retVal)
{
	D3DXVECTOR2 vMax, vMin;
	vMin.x = max(a.vMin.x, b.vMin.x);
	vMin.y = max(a.vMin.y, b.vMin.y);
	vMax.x = min(a.vMax.x, b.vMax.x);
	vMax.y = min(a.vMax.y, b.vMax.y);
	retVal.Set(vMin, vMax);

	//daca e negativ inseamna ca nu am intersectie
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

int MATH_CountBits(UINT32 dwValue)
{
	int nBits = 0;
	while (dwValue > 0)
	{
		if(dwValue & 1)
			nBits++;
		dwValue = dwValue >> 1;
	}

	return nBits;
}
