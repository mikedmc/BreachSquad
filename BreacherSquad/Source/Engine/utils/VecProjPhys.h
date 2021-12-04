#pragma once
#include "VecProj.h"
///----------------------------------------------------------------------------------
/// Vector class that encapsulates properties for physical points in 2.5D space
///----------------------------------------------------------------------------------
class VecProjPhys
{
public:
	VecProj		pos;		//pos
	Vec3		v;			//speed

	VecProjPhys();

	void		Set( Vec3 & vPos, Vec3 & vSpeed );
};
