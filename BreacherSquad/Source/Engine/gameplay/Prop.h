#pragma once

#include "components/ActiveAIComp.h"

///----------------------------------------------------------------------------------
/// List of possible classes for props (set from sprites editor but not only)
///----------------------------------------------------------------------------------
const CStringHash EPropClassNames[] =
{
	L"DOOR",
	L"DOOR_LOCKED",
	L"DOOR_KEYCARD_RED",
	L"WINDOW",
};

// aframe flags set from the sprites editor, for the props
#define K_FLAG_EDITOR_PROP_HEIGHTMASK				63
#define K_FLAG_EDITOR_PROP_COLLIDES_ACTORS			64
#define K_FLAG_EDITOR_PROP_CAN_BE_SHOT				128
#define K_FLAG_EDITOR_PROP_CLASSMASK				0xf00

// prop flags (they initially come from the editor)
#define K_PROPFLAG_COLLIDES_ACTOR			1
#define K_PROPFLAG_CAN_BE_SHOT				2

///--------------------------------------------------------------------------
/// PROPS - all objects that are not players
///--------------------------------------------------------------------------
class CProp : public IActiveInterface
{
public:
	CActiveAIComponent*	c_AI;					// AI component for prop
public:
	CSpr				sprite;
	SprFrameId			fid_ini;				// Initial animation and frame id
	DWORD				flags;
	CStringHash			shClass;				// class of prop kept as string for max flexibility

	CProp( CActiveAIComponent* pAIcomp );
	~CProp();

	const EActiveInterfaceType GetClassType() const override {
		return K_LVL_IAI_TYPE_PROP;
	}

	void				SetPos(Vec3 newPos) override;
	void				Move(Vec3 delta) override;
	void				PostConstructionInit() override;
	void				BeginPlay() override;
	void				EndPlay() override;
	void				SetAI( EAIstate newstate ) override;

	// sets internal flags reading from the AFrame flags (set in sprite editor)
	void				InitializeFromAFrameFlags( UINT32 AFrameFlags );
	// Updates everything
	void				Update( float dTime, CLevel& level );
};
