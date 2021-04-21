#include "dxstdafx.h"
#include "CollisionShape.h"

///--- COLLISION SHAPES ---
void CCollisionShape::SetPos(Vec3 newPos)
{
	//Vec2 delta = newPos - vPos;
	//vede daca am actori care stau pe mine si ii mut si pe ei
	/*
	for (int kk = 0; kk < touchingActors.GetSize(); kk++)
	{
		touchingActors[kk]->Move(delta);
	}
	*/

	pos = newPos;
	//set relative data
	bbox = bbox_ini;
	bbox.Move(pos.xy);

	bbox_exported = bbox;
	bbox_exported_ini = bbox_ini;
}

void CCollisionShape::Move(Vec3 delta)
{
	//vede daca am actori care stau pe mine si ii mut si pe ei
	/*
	for (int kk = 0; kk < touchingActors.GetSize(); kk++)
	{
		touchingActors[kk]->Move(delta);
	}
	*/
	pos.Move(delta);
	//set relative data
	bbox.Move(Vec3ToVec2XY(delta));
	bbox_exported = bbox;
}

void CCollisionShape::BeginPlay()
{

}

void CCollisionShape::EndPlay()
{

}

void CCollisionShape::PostConstructionInit()
{
	switch (type)
	{
	case K_LVL_COLL_TYPE_LEDGE:
	case K_LVL_COLL_TYPE_BOX:
		collFlags = K_DIRFLAG_DOWN;
		stairSize = bbox_ini.vHalfSize.y * 2.0f;
		break;
	case K_LVL_COLL_TYPE_LADDER:
		collFlags = K_DIRFLAG_DOWN;
		stairSize = bbox_ini.vHalfSize.y * 2.0f;
		//not too narrow
		if (bbox.vSize.x < 14.0f)
		{
			ErrorBox(K_ERR_WARNING, L"Game::LoadLevel:Ladder too narrow! RESIZING IT! Collision ID %d", ID);
			bbox.Set(bbox.vMin, bbox.vMin + Vec2(14.0f, bbox.vSize.y));
		}
		break;
	case K_LVL_COLL_TYPE_STAIRS:
	{
		collFlags = K_DIRFLAG_DOWN;
		stairSize = K_TILE_SIZE;
		//verificare suplimentara sa fie multiplu de tiles pe inaltime
		int h = int(bbox_ini.vSize.y);
		float adder = h % K_TILE_SIZE;
		if (adder > 0)
		{
			bbox_ini.vMax.y += (float)(K_TILE_SIZE - adder);
			bbox_ini.Set(bbox_ini.vMin, bbox_ini.vMax);
			bbox = bbox_ini;
		}
	}
	break;
	case K_LVL_COLL_TYPE_WATER:
		collFlags = K_DIRFLAG_NONE;
		stairSize = 0;
		castShadows = false;
		ubFlags = K_LVL_COLLFLAG_NONE;
		break;
	case K_LVL_COLL_TYPE_TRIGGER:
		collFlags = K_DIRFLAG_NONE;
		stairSize = 0;
		castShadows = false;
		break;
	case K_LVL_COLL_TYPE_PARTICLE_GENERATOR:
		collFlags = K_DIRFLAG_NONE;
		stairSize = 0;
		castShadows = false;
		break;
	case K_LVL_COLL_TYPE_FOG_OF_WAR:
		collFlags = K_DIRFLAG_NONE;
		stairSize = 0;
		castShadows = false;
		//setez by default AI de FOW unde nu am setat AI din editor
		if (AIstate == K_AI_STATE_UNDEFINED)
			AIstate = K_AI_STATE_COLL_FOG_OF_WAR;
		break;
	case K_LVL_COLL_TYPE_COVER:
		collFlags = K_DIRFLAG_NONE;
		stairSize = 0;
		break;
	case K_LVL_COLL_TYPE_MOVING_PLATFORM:
		collFlags = K_DIRFLAG_ALL;
		stairSize = 0;
		ubFlags = K_LVL_COLLFLAG_NONE;
		break;
	default:
		collFlags = K_DIRFLAG_ALL;
		stairSize = 0;
		break;
	}
}

bool CCollisionShape::RemoveTouchingActor(CActor* actor)
{
	return true;
}


///----------------------------------------------------------------------------------
/// LEVEL CollShape related methods
///----------------------------------------------------------------------------------


CCollisionShape * CLevel::GetCollisionShapeAt(Vec2 point, int collisionType)
{
	for (int kk = 0; kk < m_arrColShapes.GetSize(); kk++)
	{
		if ((collisionType != -1) && (m_arrColShapes[kk]->type != collisionType))
			continue;
		if (m_arrColShapes[kk]->bbox.PointIn(point))
			return m_arrColShapes[kk];
	}
	return null;
}

CCollisionShape* CLevel::GetCollisionShapeByUID(UINT32 nUID)
{
	if (nUID == 0)
		return null;

	for (int kk = 0; kk < m_arrColShapes.GetSize(); kk++)
	{
		if (m_arrColShapes[kk]->UID == nUID)
			return m_arrColShapes[kk];
	}
	return null;
}

CCollisionShape* CLevel::SpawnCollisionShape(int nType, Vec2 vMin, Vec2 vMax)
{
	CCollisionShape* pCol = new CCollisionShape();
	pCol->ID = GenerateNextID();
	pCol->type = nType;
	pCol->bbox_ini.Set_Corrected(vMin, vMax);
	pCol->bbox = pCol->bbox_ini;
	pCol->pos = pCol->bbox_ini.vCenter;
	pCol->collFlags = K_DIRFLAG_NONE;

	pCol->castShadows = false;

	switch (nType)
	{
		case K_LVL_COLL_TYPE_SOLID:
			pCol->collFlags = K_DIRFLAG_ALL;
			break;
		default:
			pCol->collFlags = K_DIRFLAG_NONE;
			break;
	}

	m_arrColShapes.Add(pCol);
	return pCol;
}
