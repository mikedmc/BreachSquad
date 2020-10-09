#pragma once

///--------------------------------------------------------------------------
///--- LIGHTS ---
///--------------------------------------------------------------------------
#define K_LVL_LIGHT_AMBIENTAL 0
#define K_LVL_LIGHT_AREA 1
#define K_LVL_LIGHT_POINT 2
#define K_LVL_LIGHT_DIRECTIONAL 3
#define K_LVL_LIGHT_REALISTIC_IES_OBSOLETE 4

///--- LIGHTS FLAGS ---
#define K_EDITOR_LIGHT_FLAG_CAST_SHADOWS 1
#define K_EDITOR_LIGHT_FLAG_HAS_LENS_FLARE_OBSOLETE 2

///--- LIGHTS RENDER DATA ---
//scaling lights when loading sizes from BSX
#define K_LVL_LIGHTRENDER_BSX_SCALING				0.75f
//constants used inside the pixel shader when painting spotlights
#define K_LVL_LIGHTRENDER_SPECULAR_POWER			20.0f
#define K_LVL_LIGHTRENDER_SPECULAR_INTENSITY		0.8f
//x=diffuse multiplier, y=spot color dodge layer opacity, z=spot linear dodge opacity
#define K_LVL_LIGHTRENDER_SPOT_DIFFUSE_MUL			1.0f
#define K_LVL_LIGHTRENDER_SPOT_COLOR_DODGE_ALPHA	0.9f
#define K_LVL_LIGHTRENDER_SPOT_LINEAR_DODGE_ALPHA	0.55f

class CLight : public IActiveInterface
{
public:
	D3DXVECTOR3		lCorners[4];		//ul, ur, dl, dr - mesh-ul spotului relativ la pozitia luminii; coord Z este 0 
	RECTLTRB_F		lTexRect;			//dreptunghiul in textura al spotului
	float			fMaxRadius;			//raza maxima a luminii
public:
	int				m_nLightMeshIdx;	//buffer-ul dinamic pt spotul luminii
	int				m_nShadowMeshIdx;	//buffer-ul dinamic pt shadow volume

public:
	D3DXVECTOR3		pos3D;				//avem nevoie de pozitie 3D. Se modifica in Update in fn de pos.
	int				animID;				//-1 - not set
	int				type;
	bool			castShadows;
	float			fIntensity;			//light intensity
	float			fVolumeAlpha;		//light's atmospheric volume alpha
	D3DXVECTOR3		vnDirection;		//directia normalizata a luminii (folosita doar la unele lumini, cum ar fi spoturile IES)

	CLight() :
		m_nLightMeshIdx(-1), m_nShadowMeshIdx(-1), type(0), animID(-1), fMaxRadius(0.0f), fVolumeAlpha(1.0f), castShadows(false)
	{
		vnDirection = D3DXVECTOR3(0.0f, 1.0f, 0.0f); //default direction
		pos3D = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	}

	const int GetClassType() const {
		return K_LVL_IAI_TYPE_LIGHT;
	}

	void SetPos(D3DXVECTOR2 newPos) override;
	void Move(D3DXVECTOR2 delta) override;
	void SetAngle(float fnAngle) override;

	// Initializes internal data for rendering
	// Make sure all basic light data is set before calling 
	void InitGeometry(CSpriteCollection* pLightsSprCol);
};
