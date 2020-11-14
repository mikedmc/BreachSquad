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
		//coord spotului in planul 0, relativ la lumina
		lCorners[0] = Vec3(bbox.vMin.x - pos.x, bbox.vMin.y - pos.y, 0.0f);
		lCorners[1] = Vec3(bbox.vMax.x - pos.x, bbox.vMin.y - pos.y, 0.0f);
		lCorners[2] = Vec3(bbox.vMin.x - pos.x, bbox.vMax.y - pos.y, 0.0f);
		lCorners[3] = Vec3(bbox.vMax.x - pos.x, bbox.vMax.y - pos.y, 0.0f);
		//coord in textura
		if (animID >= 0)
			lTexRect = pLightsSprCol->GetModuleRect_TexCoords(animID, 0, 0);
		//daca lumina este descentrata luam distanta maxima de la lumina la colturi si facem bbox-ul maxim in fn de ea
		float d1 = MUVec2Len(&Vec2(pos.x - bbox.vMin.x, pos.y - bbox.vMin.y));
		float d2 = max(d1, MUVec2Len(&Vec2(pos.x - bbox.vMax.x, pos.y - bbox.vMin.y)));
		float d3 = max(d2, MUVec2Len(&Vec2(pos.x - bbox.vMin.x, pos.y - bbox.vMax.y)));
		float dmax = max(d3, MUVec2Len(&Vec2(pos.x - bbox.vMax.x, pos.y - bbox.vMax.y)));
		fMaxRadius = dmax;
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
	m_nLightMeshIdx(-1), m_nShadowMeshIdx(-1), type(K_LVL_LT_UNKNOWN), animID(-1), fMaxRadius(0.0f), fVolumeAlpha(1.0f), castShadows(false)
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

