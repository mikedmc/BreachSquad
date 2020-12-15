#include "dxstdafx.h"
#include "Actor.h"

D3DXVECTOR2 CActor::GetPosHeart()
{
	//heart pos nu tine cont de flip (ar trebui sa aiba mereu X=0)
	if (fLife <= 0.0f)
		return (pos + vecHeart_abs[2]);
	if (bCrouched)
		return (pos + vecHeart_abs[1]);
	return pos + vecHeart_abs[0];
}

D3DXVECTOR2 CActor::GetPosWeapon()
{
	if (fLife <= 0.0f)
		return D3DXVECTOR2(pos.x + vecWeapon_abs[2].x * lookDirXsign, pos.y + vecWeapon_abs[2].y);
	else if (bCrouched)
		return D3DXVECTOR2(pos.x + vecWeapon_abs[1].x * lookDirXsign, pos.y + vecWeapon_abs[1].y);

	return D3DXVECTOR2(pos.x + vecWeapon_abs[0].x * lookDirXsign, pos.y + vecWeapon_abs[0].y);
}

void CActor::UpdateBBoxAndPoints()
{
	posWeapon = GetPosWeapon();
	posHeart = GetPosHeart();

	int bboxidx = 0;
	//trece pe bbox de dead doar daca a terminat animatia de dead
	if (fLife <= 0.0f)
	{
		//comenteaza linia de mai jos ca sa ia bbox doar cand a terminat animatia de moarte
		//if(((eLastAnimSet == K_LVL_ACT_ANIM_DIE) || (eLastAnimSet == K_LVL_ACT_ANIM_DIE_ALT)) && (sprite.animStatus == ANIM_STATUS_FRAMELOCK))
		bboxidx = 2;
	}
	else if (bCrouched)
		bboxidx = 1;

	bbox_ini.Set(stateBBoxes[bboxidx]);
	bbox = bbox_ini;
	bbox.Move(pos);

	bbox_exported_ini.Set(stateBBoxes[0]); //standing
	bbox_exported = bbox_exported_ini;
	bbox_exported.Move(pos);
}

void CActor::SetIcon(EActorIconTypes iconType, float fDuration)
{
	if ((iconType == K_LVL_ACT_ICON_REMOVE_ICON) || (templateActor.actorClass == K_LVL_ACT_CLASS_ZOMBIE))
	{
		fIconTimer = 0.0f;
		nIconType = K_LVL_ACT_ICON_NONE;
		return;
	}

	nIconType = iconType;
	fIconTimer = fDuration;

	if (fDuration <= 0.0f)
		nIconType = K_LVL_ACT_ICON_NONE;
}

void CActor::SetAnimSet(int newAnimSet)
{
	if (newAnimSet != nAnimSet)
	{
		nAnimSet = newAnimSet;
		//force reset animations
		eLastAnimSet = K_LVL_ACT_ANIM_EMPTY;
		eLastAnimSet_feet = K_LVL_ACT_ANIM_EMPTY;
	}
}

void CActor::SetAngle(float fNewAngle)
{
	fAngle = fNewAngle;
	vAngleDir = D3DXVECTOR2(cos(fAngle), sin(fAngle));
}

void CActor::PostConstructionInit()
{

}

void CActor::BeginPlay()
{

}

void CActor::EndPlay()
{

}

EAIBehaviorType CActor::GetCurrentBehavior()
{
	if ((m_nAIcurrentBehaviorIdx < 0) || (m_pAIcurrentState == null))
		return AI_BEHAVIOR_EMPTY;
	return m_pAIcurrentState->m_arrBehaviors[m_nAIcurrentBehaviorIdx].nType;
}

void CActor::SetPos(D3DXVECTOR2 newPos)
{
	pos_last = pos;
	pos = newPos;

	UpdateBBoxAndPoints();
}

void CActor::Move(D3DXVECTOR2 delta)
{
	pos += delta;

	UpdateBBoxAndPoints();
}
