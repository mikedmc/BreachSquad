#pragma once

class CLevel;

// interface for generic animation component
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
// interface for generic point physics component
class IBasePointPhysComponent
{
public:
	Vec3			speed;					//velocity
public:
	virtual			~IBasePointPhysComponent() {}
	// updates position based on speed
	virtual void	Update( VecProj& vPos, float dTime, CLevel & level ) = 0;
};