#pragma once

///--------------------------------------------------------------------------
///--- ACTIVES : obiectele din nivel, de decor sau scriptabile
///--------------------------------------------------------------------------
class CActive : public IActiveInterface
{
public:
	CSprite			sprite;
	int				nAnim_ini, nFrame_ini; //animatie si frame initial
	int				nLayer; //pe ce layer este obiectul de decor

	bool			flipX, flipY; //flip flags

	CActive() :
		flipX(false), flipY(false),
		nAnim_ini(-1), nFrame_ini(-1), nLayer(0)
	{}

	const int GetClassType() const {
		return K_LVL_IAI_TYPE_ACTIVE;
	}

	void SetPos(D3DXVECTOR2 newPos) override;
	void Move(D3DXVECTOR2 delta) override;
	void SetAngle(float fnAngle) override;

	// Initializes custom internal data (hardcodes usually)
	void InitInternalData();
	void BeginPlay() override;
};
