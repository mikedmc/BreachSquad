#include "dxstdafx.h"
#include "Light.h"

///--- CLIGHT ---
void CLight::SetAngle(float fnAngle)
{
	fAngle = fnAngle;
	//#TODO: IES light-ul ar trebui sa aiba altfel setarea aici ca sa permita miscari si pe alte axe 
	vnDir = Vec3(-sin(fAngle), cos(fAngle), 0.0f);
}

void CLight::PostConstructionInit()
{

}

void CLight::BeginPlay()
{

}

void CLight::EndPlay()
{

}

void CLight::InitGeometry(CSpriteCollection* pLightsSprCol)
{
	switch (type)
	{
	case K_LVL_LT_POINT:
	{
		// Gaussian attenuated radius. 
		// The attenuation with a 0.55 coefficient dops off to 0 at about 2.0f * fRadius. (2.0 is a little too big)
		// Beacuse light is farther away from the lit surfaces radius can be smaller so adjust this based on usage.
		float fRad = fRadius * 1.0f;

		// screen space light rectangle (clockwise) relative to light
		lCorners[0] = Vec3(-fRad, -fRad, 0.0f);
		lCorners[1] = Vec3( fRad, -fRad, 0.0f);
		lCorners[2] = Vec3( fRad,  fRad, 0.0f);
		lCorners[3] = Vec3(-fRad,  fRad, 0.0f);
	}
	break;
	case K_LVL_LT_IES:
	{
		ErrorBox(K_ERR_WARNING, L"LoadLevel:: Found IES light! Shouldn't get here!");
	}
	break;
	case K_LVL_LT_AMBIENTAL:
	{
		castShadows = false;
		fVolumeAlpha = 0.0f;
		bbox_ini.Set(Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f));
		bbox = bbox_ini;
	}
	break;
	case K_LVL_LT_AREA:
	{
		castShadows = false;
		fVolumeAlpha = 0.0f;
		lCorners[0] = Vec3(-bbox.vHalfSize.x, -bbox.vHalfSize.y, 0.0f);
		lCorners[1] = Vec3(bbox.vHalfSize.x, -bbox.vHalfSize.y, 0.0f);
		lCorners[2] = Vec3(-bbox.vHalfSize.x, bbox.vHalfSize.y, 0.0f);
		lCorners[3] = Vec3(bbox.vHalfSize.x, bbox.vHalfSize.y, 0.0f);
		if (animID >= 0)
			lTexRect = pLightsSprCol->GetModuleRect_TexCoords(animID, 0, 0);
	}
	break;
	case K_LVL_LT_DIRECTIONAL:
	{
		castShadows = false;
		fVolumeAlpha = 0.0f;
		lCorners[0] = Vec3(-bbox.vHalfSize.x, -bbox.vHalfSize.y, 0.0f);
		lCorners[1] = Vec3(bbox.vHalfSize.x, -bbox.vHalfSize.y, 0.0f);
		lCorners[2] = Vec3(-bbox.vHalfSize.x, bbox.vHalfSize.y, 0.0f);
		lCorners[3] = Vec3(bbox.vHalfSize.x, bbox.vHalfSize.y, 0.0f);
		if (animID >= 0)
			lTexRect = pLightsSprCol->GetModuleRect_TexCoords(animID, 0, 0);
	}
	break;
	}

}

CLight::CLight() :
	m_nLightMeshIdx(-1), m_nShadowMeshIdx(-1), type(K_LVL_LT_UNKNOWN), animID(-1), fRadius(0.0f), fVolumeAlpha(1.0f), castShadows(false)
{
	vnDir = Vec3(0.0f, 1.0f, 0.0f); //default direction
}

void CLight::SetPos(Vec2 newPos)
{
	pos = newPos;
	//set relative data
	vPos.x = pos.x; vPos.y = pos.y;
	bbox = bbox_ini;
	bbox.Move(pos);
}

void CLight::Move(Vec2 delta)
{
	pos += delta;
	//set relative data
	vPos.x = pos.x; vPos.y = pos.y;
	bbox = bbox_ini;
	bbox.Move(pos);
}

