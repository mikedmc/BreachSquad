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
};
