#pragma once
#include "ComponentInterfaces.h"

class CProp;

class CPropAIComponent : public CActiveAIComponent
{
public:
	CPropAIComponent();
	~CPropAIComponent();

	bool					Update( CProp& active, float dTime, CLevel& level );
};
