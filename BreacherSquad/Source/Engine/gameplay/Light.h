#pragma once

#include "gameplay/components/ActiveAIComp.h"

//  Light Type
enum eLightType {
	K_LVL_LT_UNKNOWN = -1,
	K_LVL_LT_AMBIENTAL = 0,
	K_LVL_LT_PROJECTED_DIR = 1,			// projected directional (lights from windows and other parallel textured sources)
	K_LVL_LT_POINT,						// classic pointlight
	K_LVL_LT_DIRECTIONAL,				// full level directional light (like ambient but with shadows)
	K_LVL_LT_IES,
	//K_LVL_LT_TEX_SPOT					// textured spotlight (texture in polar coordinates)
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

// distance attenuation formula: 1.0/(1.0 + c1*dist + c2*dist*dist)
#define K_LVL_LIGHTRENDER_ATTEN_C1					0.2f
#define K_LVL_LIGHTRENDER_ATTEN_C2					1.0f

class CLight : public IActiveInterface
{
public:
	const EActiveInterfaceType GetClassType() const override {
		return K_LVL_IAI_TYPE_LIGHT;
	}

public:
	CActiveAIComponent*	c_AI;						// AI component for lights
public:
	eLightType			type;

	Vec3				lCorners[4];				// screen space light rectangle (clockwise) relative to light (Z must be 0). Min rect that fits 2d projection of light. Used to accelerate creation of light mesh.
	Vec3				vnDir;						// normalized direction of light (necessary for some lights)
	RectLTRB			lTexRect;					// light spot source texture when necessary
	float				fRadius;					// radius of light where necessary
	bool				castShadows;
	float				fIntensity;					// light intensity
	int					nProfileID;					// keeps frameID for textured lights and IES profile for IES lights

	int					m_nLightMeshIdx;			// buffer-ul dinamic pt spotul luminii
	int					m_nShadowMeshIdx;			// buffer-ul dinamic pt shadow volume
													   
	int					animID;						// -1 - not set
	int					frameID;					// must be set when animID is set
	float				fVolumeAlpha;				// light's atmospheric volume alpha

public:
	CLight(CActiveAIComponent* pLightAIComp);
	~CLight();

	void				SetPos(Vec3 newPos) override;
	void				Move(Vec3 delta) override;
	void				PostConstructionInit() override;
	void				BeginPlay() override;
	void				EndPlay() override;
	void				SetAI( EAIstate newstate ) override;

	void				Update( float dTime, CLevel& level );

	// sets light direction with fallback for empty vectors
	void				SetDir(Vec3 nDir);
	// Initializes internal data for rendering
	// Make sure all basic light data is set before calling it (or call inside PostConstructionInit)
	void				UpdateInternalData(CSpriteLib* pLightsSprCol = nullptr);
	// Sets the light's texture, if necessary
	void				SetLightTexture(CSpriteLib* sprCol, int nAnimID, int nFrameID);
};
