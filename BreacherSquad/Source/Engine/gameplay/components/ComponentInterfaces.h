#pragma once
#include "ActorAICompTypes.h"		
#include "AICompCommon.h"

class CLevel;
class CActor;

///--- interface for generic animation component
class IBaseAnimComponent
{
public:
	virtual			~IBaseAnimComponent() {}
	virtual void	Update( CActor& act, float dTime ) = 0;
	virtual void	Paint( CActor& act, ETexChannel eChannel = K_TEXCHAN_COLORMAP ) = 0;
};


// Type of returned collision
enum ePPCContactType {
	PCT_NONE = 0,
	PCT_TILE,
	PCT_BOX,
	PCT_FLOOR,
};
///--- interface for generic point physics component
class IBasePointPhysComponent
{
public:
	Vec3			speed;					//velocity
public:
	virtual			~IBasePointPhysComponent() {}
	// updates position based on speed
	virtual void	Update( VecProj& vPos, float dTime, CLevel & level ) = 0;
	// sets point speed
	virtual void	SetSpeed(Vec3 vSpeed) = 0;
};

///--- interface for generic AI component (actors mainly)
class IBaseActorAIComponent
{
public:
	int				AIsubState;				// AI substate used here and there, everywhere
	double			fTimelineAI;			// local timeline for AI 
public:
	virtual			~IBaseActorAIComponent() {}

	virtual void	Update( CActor& act, float dTime ) = 0;
};

///--- base AI component for non actors
class IBaseAIComponent
{
protected:
	int				AIsubState;				// AI substate used here and there, everywhere
	double			fTimelineAI;			// local timeline for AI 
	AImem			mem;					// memory that holds AI vars
public:
	IBaseAIComponent() : AIsubState( 0 ), fTimelineAI( 0.0f ) {}
	virtual			~IBaseAIComponent() {}
	// updates and returns true if state was handled (so we can call another one in the chain if not)
	virtual bool	Update( IActiveInterface& active, float dTime ) = 0;
	// call this to set the AI state
	virtual void	SetAI( IActiveInterface& active, EAIstate newstate ) = 0;
};
