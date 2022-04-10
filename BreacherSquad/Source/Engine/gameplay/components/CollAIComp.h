#pragma once
#include "ComponentInterfaces.h"

class CCollisionShape;

class CCollAIComponent : public CActiveAIComponent
{
public:
	CCollAIComponent();
	~CCollAIComponent();

	bool					Update( CCollisionShape& active, float dTime );
};
