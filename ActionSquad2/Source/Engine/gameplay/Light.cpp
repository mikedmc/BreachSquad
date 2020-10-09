#include "dxstdafx.h"
#include "Light.h"


///--- CLIGHT ---
void CLight::SetAngle(float fnAngle)
{
	fAngle = fnAngle;
	//#TODO: IES light-ul ar trebui sa aiba altfel setarea aici ca sa permita miscari si pe alte axe 
	vnDirection = D3DXVECTOR3(-sin(fAngle), cos(fAngle), 0.0f);
}

void CLight::InitGeometry(CSpriteCollection* pLightsSprCol)
{
	switch (type)
	{
	case K_LVL_LIGHT_POINT:
	{
		//coord spotului in planul 0, relativ la lumina
		lCorners[0] = D3DXVECTOR3(bbox.vMin.x - pos.x, bbox.vMin.y - pos.y, 0.0f);
		lCorners[1] = D3DXVECTOR3(bbox.vMax.x - pos.x, bbox.vMin.y - pos.y, 0.0f);
		lCorners[2] = D3DXVECTOR3(bbox.vMin.x - pos.x, bbox.vMax.y - pos.y, 0.0f);
		lCorners[3] = D3DXVECTOR3(bbox.vMax.x - pos.x, bbox.vMax.y - pos.y, 0.0f);
		//coord in textura
		if (animID >= 0)
			lTexRect = pLightsSprCol->GetModuleRect_TexCoords(animID, 0, 0);
		//daca lumina este descentrata luam distanta maxima de la lumina la colturi si facem bbox-ul maxim in fn de ea
		float d1 = D3DXVec2Length(&D3DXVECTOR2(pos.x - bbox.vMin.x, pos.y - bbox.vMin.y));
		float d2 = max(d1, D3DXVec2Length(&D3DXVECTOR2(pos.x - bbox.vMax.x, pos.y - bbox.vMin.y)));
		float d3 = max(d2, D3DXVec2Length(&D3DXVECTOR2(pos.x - bbox.vMin.x, pos.y - bbox.vMax.y)));
		float dmax = max(d3, D3DXVec2Length(&D3DXVECTOR2(pos.x - bbox.vMax.x, pos.y - bbox.vMax.y)));
		fMaxRadius = dmax;
	}
	break;
	case K_LVL_LIGHT_REALISTIC_IES_OBSOLETE:
	{
		ErrorBox(K_ERR_WARNING, L"LoadLevel:: Found IES light! Shouldn't get here!");
	}
	break;
	case K_LVL_LIGHT_AMBIENTAL:
	{
		castShadows = false;
		fVolumeAlpha = 0.0f;
		bbox_ini.Set(D3DXVECTOR2(0.0f, 0.0f), D3DXVECTOR2(0.0f, 0.0f));
		bbox = bbox_ini;
	}
	break;
	case K_LVL_LIGHT_AREA:
	{
		castShadows = false;
		fVolumeAlpha = 0.0f;
		lCorners[0] = D3DXVECTOR3(-bbox.vHalfSize.x, -bbox.vHalfSize.y, 0.0f);
		lCorners[1] = D3DXVECTOR3(bbox.vHalfSize.x, -bbox.vHalfSize.y, 0.0f);
		lCorners[2] = D3DXVECTOR3(-bbox.vHalfSize.x, bbox.vHalfSize.y, 0.0f);
		lCorners[3] = D3DXVECTOR3(bbox.vHalfSize.x, bbox.vHalfSize.y, 0.0f);
		if (animID >= 0)
			lTexRect = pLightsSprCol->GetModuleRect_TexCoords(animID, 0, 0);
	}
	break;
	case K_LVL_LIGHT_DIRECTIONAL:
	{
		castShadows = false;
		fVolumeAlpha = 0.0f;
		lCorners[0] = D3DXVECTOR3(-bbox.vHalfSize.x, -bbox.vHalfSize.y, 0.0f);
		lCorners[1] = D3DXVECTOR3(bbox.vHalfSize.x, -bbox.vHalfSize.y, 0.0f);
		lCorners[2] = D3DXVECTOR3(-bbox.vHalfSize.x, bbox.vHalfSize.y, 0.0f);
		lCorners[3] = D3DXVECTOR3(bbox.vHalfSize.x, bbox.vHalfSize.y, 0.0f);
		if (animID >= 0)
			lTexRect = pLightsSprCol->GetModuleRect_TexCoords(animID, 0, 0);
	}
	break;
	}

}

void CLight::SetPos(D3DXVECTOR2 newPos)
{
	pos = newPos;
	//set relative data
	pos3D.x = pos.x; pos3D.y = pos.y;
	bbox = bbox_ini;
	bbox.Move(pos);
}

void CLight::Move(D3DXVECTOR2 delta)
{
	pos += delta;
	//set relative data
	pos3D.x = pos.x; pos3D.y = pos.y;
	bbox = bbox_ini;
	bbox.Move(pos);
}

