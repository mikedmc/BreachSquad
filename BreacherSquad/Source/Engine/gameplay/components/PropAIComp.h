#pragma once
#include "ComponentInterfaces.h"

class CPropAIComponent : public IBaseAIComponent
{
public:
	CPropAIComponent();
	~CPropAIComponent();

	bool					Update( IActiveInterface& active, float dTime, CLevel& level ) override;
	void					SetAI( IActiveInterface& active, EAIstate newstate ) override;
};
