#include "dxstdafx.h"
#include "CMathUtil.h"

float UTMath::LineDist(Vec3 p, Vec3 v1, Vec3 v2, Vec3 *n)
{
	Vec3 v1p, v2p, v1v2, nr;
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


bool UTMath::LineLineIntersection(Vec2 p1, Vec2 p2, Vec2 p3, Vec2 p4, Vec2 *outPt)
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

bool UTMath::LineLineIntersects_denom(Vec2 a, Vec2 b, Vec2 c, Vec2 d, float & r, float & s)
{
	float denominator = ((b.x - a.x) * (d.y - c.y)) - ((b.y - a.y) * (d.x - c.x));

	// returns false for ALL parallel lines
	if (denominator == 0.0f)
		return false;

	float numerator1 = ((a.y - c.y) * (d.x - c.x)) - ((a.x - c.x) * (d.y - c.y));
	float numerator2 = ((a.y - c.y) * (b.x - a.x)) - ((a.x - c.x) * (b.y - a.y));
	// If you need intersections between parallel overlapping lines use this:
	//if (denominator == 0.0f) return numerator1 == 0 && numerator2 == 0;

	r = numerator2 / denominator; // segment a-b 
	s = numerator1 / denominator; // segment c-d

	return (r >= 0.0f && r <= 1.0f) && (s >= 0.0f && s <= 1.0f);
}

bool UTMath::RaySegmentIntersection(Vec2 a, Vec2 b, Vec2 c, Vec2 d, Vec2 *outPt)
{
	float denominator = ((b.x - a.x) * (d.y - c.y)) - ((b.y - a.y) * (d.x - c.x));

	// returns false for ALL parallel lines
	if (denominator == 0.0f)
		return false;

	float numerator1 = ((a.y - c.y) * (d.x - c.x)) - ((a.x - c.x) * (d.y - c.y));
	float numerator2 = ((a.y - c.y) * (b.x - a.x)) - ((a.x - c.x) * (b.y - a.y));
	// If you need intersections between parallel overlapping lines use this:
	//if (denominator == 0.0f) return numerator1 == 0 && numerator2 == 0;

	float s = numerator2 / denominator;		// segment (c-d)
	float r = numerator1 / denominator;		// ray (a->b)
	// intersection behind ray start
	if (r < 0.0f) return false;
	// intersection outside segment (includes heads in collisions)
	if (s < 0 || s > 1.0f) return false;

	if (outPt)
	{
		// interpolate between them
		*outPt = Vec2((1.0f - s) * c.x + s * d.x, (1.0f - s) * c.y + s * d.y);
	}
	return true;
}

bool UTMath::RaySegmentIntersection_denom(Vec2 a, Vec2 b, Vec2 c, Vec2 d, float &retR, float &retS, Vec2 *outPt /*= nullptr*/)
{
	float denominator = ((b.x - a.x) * (d.y - c.y)) - ((b.y - a.y) * (d.x - c.x));

	// returns false fa ALL parallel lines
	if (denominator == 0.0f)
		return false;

	float numerator1 = ((a.y - c.y) * (d.x - c.x)) - ((a.x - c.x) * (d.y - c.y));
	float numerator2 = ((a.y - c.y) * (b.x - a.x)) - ((a.x - c.x) * (b.y - a.y));
	// If you need intersections between parallel overlapping lines use this:
	//if (denominator == 0.0f) return numerator1 == 0 && numerator2 == 0;

	retS = numerator2 / denominator;		// segment (c-d)
	retR = numerator1 / denominator;		// ray (a->b)
	// intersection behind ray start
	if (retR < 0.0f) return false;
	// intersection outside segment (includes heads in collisions)
	if (retS < 0 || retS > 1.0f) return false;

	if (outPt)
	{
		// interpolate between them
		*outPt = Vec2((1.0f - retS) * c.x + retS * d.x, (1.0f - retS) * c.y + retS * d.y);
	}
	return true;
}

void UTMath::EaseTo_quadratic(float * current, float target, float fDistMultiplier, float fMinSpeed)
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

void UTMath::EaseTo_linear(float * current, float target, float fSpeed)
{
	if (current == null)
		return;

	float dst = target - *current;
	if (fabs(dst) <= fSpeed)
		*current = target;
	else
		*current += SIGN(dst) * fSpeed;
}


float UTMath::GetAlphaOnDomainEnds(float fCursor, float fDomainLength, float fAlphaRegionSize)
{
	CLAMP(fCursor, 0.0f, fDomainLength);
	if (fCursor < fAlphaRegionSize)
		return (fCursor / fAlphaRegionSize);
	if (fCursor > fDomainLength - fAlphaRegionSize)
		return ((fDomainLength - fCursor) / fAlphaRegionSize);
	return 1.0f;
}

bool UTMath::IsPowerOfTwo(unsigned int nVal)
{
	return ((nVal != 0) && !(nVal & (nVal - 1)));
}

float UTMath::GetVectorAngle(Vec2 start, Vec2 end)
{
	Vec2 vecdir = end - start;
	if ((vecdir.x == 0.0f) && (vecdir.y == 0))
		return 0.0f;

	return((float)(HALF_PI - atan2(vecdir.x, vecdir.y)));
}

float UTMath::GetVectorAngle(Vec2 const &dir)
{
	if ((dir.x == 0.0f) && (dir.y == 0.0f))
		return 0.0f;

	return((float)(HALF_PI - atan2(dir.x, dir.y)));
}

float UTMath::GetAngleBetweenVectors(Vec2 vec1, Vec2 vec2)
{
	Vec2 v1, v2;
	D3DXVec2Normalize(&v1, &vec1);
	D3DXVec2Normalize(&v2, &vec2);
	float ang = acos(D3DXVec2Dot(&v1, &v2));
	return ang;
}


int UTMath::Log2i(float val)
{
	return ((*(int *)(&val) >> 23) & 0xFF) - 127;
}

int UTMath::Log2i(int val)
{

	return UTMath::Log2i((float)val);
}

int UTMath::GetBitsNeededForValue(int val)
{

	return UTMath::Log2i(val) + 1;
}

int UTMath::CountBits(UINT32 dwValue)
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

/*
ATAN approximation
In my (somewhat limited but concise) testing on both a very fast Gen 7 Xeon and an STM32F4 (w/FPU) ARM micro show atan2_approximation1 to be faster (and much more accurate) than the 2nd version.
On STM32F4 a1 is ~2.3 times faster than stdlib (with gcc-arm 4.7.4)., while a2 is ~2.1 times faster than stdlib.
On the Xeon under Windows with MSVC a1 shows maybe a very slight improvement in speed over std (within margin of error, really), while with MinGW-w64 on same system the std version is a whopping 12 times slower!
Thanks for this snip, very handy!
*/

float UTMath::atan2_approximation1(float y, float x)
{
	//http://pubs.opengroup.org/onlinepubs/009695399/functions/atan2.html
	//Volkan SALMA

	float r, angle;
	float abs_y = fabs(y) + 1e-10f;      // kludge to prevent 0/0 condition
	if (x < 0.0f)
	{
		r = (x + abs_y) / (abs_y - x);
		angle = THRQTR_PI;
	}
	else
	{
		r = (x - abs_y) / (x + abs_y);
		angle = ONEQTR_PI;
	}
	angle += (0.1963f * r * r - 0.9817f) * r;
	if (y < 0.0f)
		return(-angle);     // negate if in quad III or IV
	else
		return(angle);


}

// |error| < 0.005
float UTMath::atan2_approximation2(float y, float x)
{
	if (x == 0.0f)
	{
		if (y > 0.0f) return ATAN2_PIBY2_FLOAT;
		if (y == 0.0f) return 0.0f;
		return -ATAN2_PIBY2_FLOAT;
	}
	float atan;
	float z = y / x;
	if (fabs(z) < 1.0f)
	{
		atan = z / (1.0f + 0.28f*z*z);
		if (x < 0.0f)
		{
			if (y < 0.0f) return atan - ATAN2_PI_FLOAT;
			return atan + ATAN2_PI_FLOAT;
		}
	}
	else
	{
		atan = ATAN2_PIBY2_FLOAT - z / (z*z + 0.28f);
		if (y < 0.0f) return atan - ATAN2_PI_FLOAT;
	}
	return atan;
}
