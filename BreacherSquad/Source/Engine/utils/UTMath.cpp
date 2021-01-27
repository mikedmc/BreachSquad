#include "dxstdafx.h"
#include "UTMath.h"

bool MUVec2AlmostZero(Vec2 vec, float fThreshold)
{
	if (MUVec2LenSq(&vec) <= fThreshold * fThreshold)
		return true;
	return false;
}

bool MUVec3AlmostZero(Vec3 vec, float fThreshold)
{
	if (MUVec3LenSq(&vec) <= fThreshold * fThreshold)
		return true;
	return false;
}
