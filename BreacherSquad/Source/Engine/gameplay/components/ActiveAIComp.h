#pragma once
#include "ComponentInterfaces.h"

class CActiveAIComponent : public IBaseAIComponent
{
public:
	CActiveAIComponent();
	~CActiveAIComponent();

	bool					Update( IActiveInterface& active, float dTime, CLevel& level ) override;
	void					SetAI( IActiveInterface& active, EAIstate newstate ) override;
};
