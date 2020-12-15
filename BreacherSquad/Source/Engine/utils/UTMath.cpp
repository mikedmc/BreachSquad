#include "dxstdafx.h"
#include "UTMath.h"

bool MUVec2AlmostZero(Vec2 vec)
{
	if (MUVec2LenSq(&vec) <= 0.00001f)
		return true;
	return false;
}

bool MUVec3AlmostZero(Vec3 vec)
{
	if (MUVec3LenSq(&vec) <= 0.00001f)
		return true;
	return false;
}
