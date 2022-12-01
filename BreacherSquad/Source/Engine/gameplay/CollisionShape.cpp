#include "dxstdafx.h"
#include "CollisionShape.h"

CCollisionShape::CCollisionShape( CPropAIComponent* AIcomponent ) :
	eType( K_SHAPE_SOLID ), castShadows( false ),
	collFlags( K_DIRFLAG_ALL ), ubFlags( K_LVL_COLLFLAG_SOLID ),
	c_AI( AIcomponent )
{
}

CCollisionShape::~CCollisionShape()
{
	SAFE_DELETE( c_AI );
}

///--- COLLISION SHAPES ---
void CCollisionShape::SetPos( Vec3 newPos )
{
	pos = newPos;
	//set relative data
	bbox.RestoreSnapshot( pos.xy );
}

void CCollisionShape::Move( Vec3 delta )
{
	pos.Move( delta );
	//set relative data
	bbox.Move( Vec3XY( delta ) );
}

void CCollisionShape::SetAI( EAIstate newstate )
{
	c_AI->SetAI( *this, newstate );
}

void CCollisionShape::BeginPlay()
{

}

void CCollisionShape::EndPlay()
{

}

void CCollisionShape::Update( float dTime, CLevel& level )
{
	// clean target pointer (should be done by AI?)
	/*
	if ( ( pTarget != nullptr ) && pTarget->IsPendingKill() )
	{
		pTarget->FreeRef();
		pTarget = nullptr;
	}
	*/

	bEnabled = bSetEnabled;
	if ( !IsAlive() )
		return;

	c_AI->Update( *this, dTime, level );
}

void CCollisionShape::PostConstructionInit()
{
	switch ( eType )
	{
	case K_SHAPE_SOLID:
		collFlags = K_DIRFLAG_ALL;
		break;
	case K_SHAPE_TRIGGER:
		collFlags = K_DIRFLAG_NONE;
		castShadows = false;
		break;
	case K_SHAPE_PARTICLEGEN:
		collFlags = K_DIRFLAG_NONE;
		castShadows = false;
		break;
	default:
		collFlags = K_DIRFLAG_ALL;
		castShadows = true;
		break;
	}
}

///----------------------------------------------------------------------------------
/// LEVEL CollShape related methods
///----------------------------------------------------------------------------------


CCollisionShape * CLevel::GetCollisionShapeAt( Vec2 point, int collisionType )
{
	//#TODO: to optimize! only search in area pointed by the point
	for ( int kk = 0; kk < m_arrColShapes.GetSize(); kk++ )
	{
		if ( ( collisionType != -1 ) && ( m_arrColShapes[kk]->eType != collisionType ) )
			continue;
		if ( m_arrColShapes[kk]->bbox.PointIn( point ) )
			return m_arrColShapes[kk];
	}
	return nullptr;
}

CCollisionShape* CLevel::GetCollisionShapeByUID( UINT32 nUID )
{
	if ( nUID == 0 )
		return nullptr;

	for ( int kk = 0; kk < m_arrColShapes.GetSize(); kk++ )
	{
		if ( m_arrColShapes[kk]->UID == nUID )
			return m_arrColShapes[kk];
	}
	return nullptr;
}

CCollisionShape* CLevel::SpawnCollisionShape( ECollType newType, Vec2 vMin, Vec2 vMax )
{
	CCollisionShape* pCol = new CCollisionShape( new CPropAIComponent() );
	pCol->ID = GenerateNextID();
	pCol->eType = newType;
	pCol->bbox.Set_Corrected( vMin, vMax );
	pCol->bbox.SaveSnapshot();
	pCol->pos = pCol->bbox.vCenter;

	pCol->collFlags = K_DIRFLAG_NONE;
	pCol->castShadows = false;

	pCol->PostConstructionInit();
	m_arrColShapes.Add( pCol );
	pCol->BeginPlay();

	return pCol;
}
