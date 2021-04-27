#include "dxstdafx.h"
#include "Prop.h"

///--- CACTIVE ---
void CProp::SetPos(Vec3 newPos)
{
	pos = newPos;
	bbox.Set(&bbox_ini, pos.xy_proj);
	bbox_floor.Set(&bbox_floor_ini, pos.xy);
}

void CProp::Move(Vec3 delta)
{
	Vec3 npos = pos.xyz + delta;
	pos = npos;
	bbox.Set(&bbox_ini, pos.xy_proj);
	bbox_floor.Set(&bbox_floor_ini, pos.xy);
}

void CProp::InitializeFromAFrameFlags(UINT32 AFrameFlags)
{
	// read height and convert from screen to world (usually double the height)
	fHeight = H_TO_Z((float)(AFrameFlags & K_FLAG_EDITOR_PROP_HEIGHTMASK));
	// read class as int and convert to StrHash
	int nClass = (AFrameFlags & K_FLAG_EDITOR_PROP_CLASSMASK) >> 8;
	if ((nClass > 0) && (nClass < ARRAY_SIZE(EPropClassNames)))
	{
		shClass = EPropClassNames[nClass - 1];
	}
	// reset flags
	flags = 0;
	if (AFrameFlags & K_FLAG_EDITOR_PROP_COLLIDES_ACTORS) flags |= K_PROPFLAG_COLLIDES_ACTOR;
	if (AFrameFlags & K_FLAG_EDITOR_PROP_CAN_BE_SHOT) flags |= K_PROPFLAG_CAN_BE_SHOT;
}

void CProp::PostConstructionInit()
{
	// compute bboxes on init
	bbox.Set(&bbox_ini, pos.xy_proj);
	bbox_floor.Set(&bbox_floor_ini, pos.xy);
}

void CProp::BeginPlay()
{

}

void CProp::EndPlay()
{

}
