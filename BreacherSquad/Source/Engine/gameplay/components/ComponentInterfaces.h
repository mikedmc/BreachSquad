#pragma once

// interface for generic base component
class IBaseComponent
{
public:
	virtual ~IBaseComponent() {}
	virtual void Update(CActor& act, float dTime) = 0;
	virtual void Paint(CActor& act, ETexChannel eChannel = K_TEXCHAN_COLORMAP) = 0;
};