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
void CProp::SetPos( Vec3 newPos )
{
	pos = newPos;
	bbox.RestoreSnapshot( pos.xy_proj );
	bbox_floor.RestoreSnapshot( pos.xy );
}

void CProp::Move( Vec3 delta )
{
	Vec3 npos = pos.xyz + delta;
	pos = npos;
	bbox.RestoreSnapshot( pos.xy_proj );
	bbox_floor.RestoreSnapshot( pos.xy );
}

void CProp::InitializeFromAFrameFlags( UINT32 AFrameFlags )
{
	// read height and convert from screen to world (usually double the height)
	heightZ = H_TO_Z( ( float ) ( AFrameFlags & K_FLAG_EDITOR_PROP_HEIGHTMASK ) );
	// read class as int and convert to StrHash
	int nClass = ( AFrameFlags & K_FLAG_EDITOR_PROP_CLASSMASK ) >> 8;
	if ( ( nClass > 0 ) && ( nClass < ARRAY_SIZE( EPropClassNames ) ) )
	{
		shClass = EPropClassNames[nClass - 1];
	}
	// reset flags
	flags = 0;
	if ( AFrameFlags & K_FLAG_EDITOR_PROP_COLLIDES_ACTORS ) flags |= K_PROPFLAG_COLLIDES_ACTOR;
	if ( AFrameFlags & K_FLAG_EDITOR_PROP_CAN_BE_SHOT ) flags |= K_PROPFLAG_CAN_BE_SHOT;
}

void CProp::Update( float dTime, CLevel& level )
{
	// clean target pointer (should be done by AI?)
	if ( ( pTarget != nullptr ) && pTarget->IsPendingKill() )
	{
		pTarget->FreeRef();
		pTarget = nullptr;
	}
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
		if ( ( sprite.animStatus == ANIM_STATUS_PLAYING_FRAME_ADVANCED ) || ( sprite.animStatus == ANIM_STATUS_FRAMELOCK ) )
		{
			RectXYWHi frrect = sprite.pSprCol->GetAFrameBBox( sprite.animIdx, sprite.frameIdx );
			bbox.SetSnapshot( frrect );
		}
	}

	// Update AI 
	c_AI->Update( *this, dTime, level );
	//final updates
	sprite.pos = pos.xy_proj;
	sprite.color = color;
}

void CProp::PostConstructionInit()
{
	UINT32 frameflag = sprite.GetAframeFlags();
	InitializeFromAFrameFlags( frameflag );

	// initialize secondary data
	RectXYWHi bb_floor = sprite.GetAFrameBBox();
	RectXYWHi bb_proj = bb_floor;
	// move box up
	bb_proj.Move( 0.0f, -Z_TO_H( heightZ ) );
	bb_proj.h = bb_floor.Bottom() - bb_proj.y;

	bbox_floor.Set( bb_floor );
	bbox_floor.SaveSnapshot();
	bbox.Set( bb_proj );
	bbox.SaveSnapshot();
	// when we flip it on X we flip bboxes too
	/*
	if (IS_FLAG_ALL(obj->flags, K_PROPFLAG_FLIP_X)
	{
		obj->bbox_ini.Move(Vec2(-2.0f * obj->bbox_ini.vCenter.x, 0.0f));
		obj->bbox_floor_ini.Move(Vec2(-2.0f * obj->bbox_floor.vCenter.x, 0.0f));
	}
	*/

	// place snapshot into box, at object position
	bbox.RestoreSnapshot( pos.xy_proj );
	bbox_floor.RestoreSnapshot( pos.xy );
}

void CProp::BeginPlay()
{

}

void CProp::EndPlay()
{

}

void CProp::SetAI( EAIstate newstate )
{
	c_AI->SetAI( *this, newstate );
}
