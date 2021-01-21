#pragma once

///------------------------------------------------------------------------------------------------------------------------------------------
///  2D AABB with collision flags
/// eg: K_DIRFLAG_UP specifies that the collision aabb has a hard edge UP 
///------------------------------------------------------------------------------------------------------------------------------------------
class CAABBColl : public CAABB
{
public:
	UINT16			collFlags;		//flags from K_DIRFLAG_... collection
	
	CAABBColl() :
		collFlags(0)
		//#TODO: if we need moving objects to collide then add this:
		//Vec2 vMove;				// vector that shows how much it will move (speed * dTime usually)
	{}

	CAABBColl(CAABB & src)
	{
		vHalfSize = src.vHalfSize;
		vCenter = src.vCenter;
		vMin = src.vMin;
		vMax = src.vMax;
		vSize = src.vSize;		 

		collFlags = 0;
	}

	// Finds the closest intersection as a fraction of ^direction
	float GetRayIntersectionFraction(Vec2 origin, Vec2 direction)
	{
		Vec2 end = origin + direction;

		// for each of the AABB's four edges
		// calculate the minimum fraction of "direction"
		// in order to find where the ray FIRST intersects
		// the AABB (if it ever does)

		//DMC: only check the sides that have collision flags set
		float r = 0.0f, s = 0.0f, minT = 100000.0f;
		if ((collFlags & K_DIRFLAG_LEFT) && (UTMath::LineLineIntersects_denom(origin, end, vMin, Vec2(vMin.x, vMax.y), r, s)))
		{
			minT = r;
		}
		if ((collFlags & K_DIRFLAG_DOWN) && (UTMath::LineLineIntersects_denom(origin, end, Vec2(vMin.x, vMax.y), vMax, r, s)))
		{
			if (r < minT) minT = r;
		}
		if ((collFlags & K_DIRFLAG_RIGHT) && (UTMath::LineLineIntersects_denom(origin, end, vMax, Vec2(vMax.x, vMin.y), r, s)))
		{
			if (r < minT)
				minT = r;
		}
		if ((collFlags & K_DIRFLAG_UP) && (UTMath::LineLineIntersects_denom(origin, end, Vec2(vMax.x, vMin.y), vMin, r, s)))
		{
			if (r < minT)
				minT = r;
		}

		// return the fractional component along the ray where we collided
		return minT;
	}
};


// !!! VEZI: bool			LineLineIntersects_denom(Vec2 a, Vec2 b, Vec2 c, Vec2 d, float & r, float & s); e aceeasi functie ca mai  jos:

// returns true if they intersect. Sets &frac for the intersection point representing the fraction of originA->endA vector where they intersect
/*
float GetRayIntersectionFractionOfFirstRay(Vec2 originA, Vec2 endA, Vec2 originB, Vec2 endB, float &frac)
{
	Vec2 r = endA - originA;
	Vec2 s = endB - originB;

	float numerator = MUVec2Dot(&(originB - originA), &r);
	float denominator = MUVec2Dot(&r, &s);

	if (numerator == 0 && denominator == 0)
	{
		// the lines are co-linear
		// check if they overlap
		// todo: calculate intersection point
		return false;
	}
	if (denominator == 0)
	{
		// lines are parallel
		return false;
	}

	float u = numerator / denominator;
	float t = MUVec2Dot(&(originB - originA), &s) / denominator;
	if ((t >= 0) && (t <= 1) && (u >= 0) && (u <= 1))
	{
		frac = t;
		return true;
	}
	return false;
}
*/

