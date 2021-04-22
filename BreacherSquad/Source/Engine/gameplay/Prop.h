#pragma once

// prop flags (they initially come from the editor)
#define K_PROPFLAG_COLLIDES_ACTOR			1
#define K_PROPFLAG_CAN_BE_SHOT				2

///--------------------------------------------------------------------------
/// PROPS - all objects that are not players
///--------------------------------------------------------------------------
class CProp : public IActiveInterface
{
public:
	CSpr			sprite;
	scFrameID		fid_ini;				// Initial animation and frame id
	DWORD			flags;

	CProp() :
		flags(0)
	{}

	const eActiveInterfaceType GetClassType() const {
		return K_LVL_IAI_TYPE_PROP;
	}

	void SetPos(Vec3 newPos) override;
	void Move(Vec3 delta) override;

	// Initializes custom internal data (hardcodes usually)
	void PostConstructionInit() override;
	void BeginPlay() override;
	void EndPlay() override;
};
