#pragma once
#include "ActorAICompTypes.h"					  

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

///--- interface for generic AI component
class IBaseAIComponent
{
public:
	CAICommands		m_AIcommands;	// Commands issued by AI to be executed by the actor
public:
	virtual			~IBaseAIComponent() {}
	// AI components need access to the level to get data about the enemies and what not
	virtual void	Update( CActor& act, float dTime, CLevel & level ) = 0;
};