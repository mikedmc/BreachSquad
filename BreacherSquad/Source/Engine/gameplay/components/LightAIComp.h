#pragma once
#include "ComponentInterfaces.h"

class CLight;

class CLightAIComponent : public CActiveAIComponent
{
public:
	CLightAIComponent();
	~CLightAIComponent();

	bool					Update( CLight& active, float dTime );
};
