#pragma once

///--------------------------------------------------------------------------
/// PROPS - all objects that are not players
///--------------------------------------------------------------------------
class CProp : public IActiveInterface
{
public:
	CSpr			sprite;
	int				nAnim_ini, nFrame_ini;		// Initial animation and frame

	bool			flipX;

	CProp() :
		flipX(false),
		nAnim_ini(-1), nFrame_ini(-1)
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
