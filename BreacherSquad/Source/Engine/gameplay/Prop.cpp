#include "dxstdafx.h"
#include "Prop.h"

CProp::CProp( CPropAIComponent* pAIcomp ) :
	flags( 0 ), c_AI( pAIcomp )
{

}

CProp::~CProp()
{
	SAFE_DELETE( c_AI );
}

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
	heightZ = H_TO_Z((float)(AFrameFlags & K_FLAG_EDITOR_PROP_HEIGHTMASK));
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

void CProp::Update( float dTime )
{
	//check visibility change
	bEnabled = bSetEnabled;
	if ( !IsAlive() )
		return;

	//update sprite if animated
	if ( bAnimated )
	{
		sprite.Update( dTime );
		//cand ajunge la capatul animatiei scoate flagul de animated
		if ( sprite.animStatus == ANIM_STATUS_FRAMELOCK )
			bAnimated = false;
		// refresh bbox on each frame change
		if ( (sprite.animStatus == ANIM_STATUS_PLAYING_FRAME_ADVANCED) || (sprite.animStatus == ANIM_STATUS_FRAMELOCK) )
		{
			RectXYWHi frrect = sprite.pSprCol->GetAFrameBBox( sprite.animIdx, sprite.frameIdx );
			bbox_ini.Set( frrect );
		}
	}

	// Update AI 
	c_AI->Update( *this, dTime );
	//final updates
	sprite.pos = pos.xy_proj;
	sprite.color = color;
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
