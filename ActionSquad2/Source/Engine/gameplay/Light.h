#pragma once

//  Light Type
enum eLightType {
	K_LVL_LT_UNKNOWN = -1,
	K_LVL_LT_AMBIENTAL = 0,
	K_LVL_LT_AREA = 1,
	K_LVL_LT_POINT,
	K_LVL_LT_DIRECTIONAL,
	K_LVL_LT_IES,

	K_LVL_LTS_CNT,
};

///--- LIGHTS EDITOR FLAGS ---
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
	const eActiveInterfaceType GetClassType() const {
		return K_LVL_IAI_TYPE_LIGHT;
	}

public:
	eLightType			type;

	Vec3				lCorners[4];				// ul, ur, dl, dr - mesh-ul spotului relativ la pozitia luminii; coord Z este 0 
	RECTLTRB_F			lTexRect;					// dreptunghiul in textura al spotului
	float				fMaxRadius;					// raza maxima a luminii
													   
	int					m_nLightMeshIdx;			// buffer-ul dinamic pt spotul luminii
	int					m_nShadowMeshIdx;			// buffer-ul dinamic pt shadow volume
													   
	int					animID;						// -1 - not set
	bool				castShadows;				   
	float				fIntensity;					// light intensity
	float				fVolumeAlpha;				// light's atmospheric volume alpha
	Vec3				vnDir;						// directia normalizata a luminii (folosita doar la unele lumini, cum ar fi spoturile IES)

public:
	CLight();

	void				SetPos(Vec2 newPos) override;
	void				Move(Vec2 delta) override;
	void				SetAngle(float fnAngle) override;

	// engine callbacks
	void				PostConstructionInit() override;
	void				BeginPlay() override;
	void				EndPlay() override;

	// Initializes internal data for rendering
	// Make sure all basic light data is set before calling it (or call inside PostConstructionInit)
	void				InitGeometry(CSpriteCollection* pLightsSprCol);
};
