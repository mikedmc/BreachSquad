#include "dxstdafx.h"
#include "CMathUtil.h"

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


int Log2i(float val)
{
	return ((*(int *)(&val) >> 23) & 0xFF) - 127;
}

int Log2i(int val)
{

	return Log2i((float)val);
}

int MATH_GetBitsNeededForValue(int val)
{

	return Log2i(val) + 1;
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
