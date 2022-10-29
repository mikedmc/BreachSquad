#include "dxstdafx.h"
#include "Light.h"

///--- CLIGHT ---
void CLight::PostConstructionInit()
{
	bPendingKill = false;
}

void CLight::BeginPlay()
{

}

void CLight::EndPlay()
{

}

void CLight::SetAI( EAIstate newstate )
{
	c_AI->SetAI( *this, newstate );
}

void CLight::Update( float dTime, CLevel& level )
{
	bEnabled = bSetEnabled;
	// hidden? skip update
	if ( !IsAlive() )
		return;

	c_AI->Update( *this, dTime, level );
	// enforce position updating after UI pass
	SetPos( pos.xyz );
}

void CLight::SetCastShadows( bool bCast )
{
	if ( bCast == castShadows )
		return;
	// allocate and deallocate lights meshes and set dirty flag. We only need the mesh for shadow casting lights.
	m_bDirty = true;
	castShadows = bCast;
	m_arrVertsCnt = 0;
	if ( castShadows )
	{
		_ASSERT( m_arrVerts == nullptr );
		m_arrVerts = new _VERTEX_PNCT4T4[K_LVL_LIGHT_MAX_VERTS];
	}
	else
	{
		SAFE_DELETE_ARRAY( m_arrVerts );
	}
}

void CLight::SetDirty( bool bDirty )
{
	m_bDirty = bDirty;
}

void CLight::SetDir( Vec3 nDir )
{
	if ( UTMath::Vec3AlmostZero( nDir ) )
	{
		vnDir = Vec3( 0.0f, 0.0f, -1.0f ); //looking down
		return;
	}

	MUVec3Norm( &vnDir, &nDir );
}

void CLight::UpdateInternalData( CSpriteLib* pLightsSprCol )
{
	switch ( type )
	{
		case K_LVL_LT_IES:
		case K_LVL_LT_POINT:
		{
			// Gaussian attenuated radius. 
			// The attenuation with a 0.55 coefficient dops off to 0 at about 2.0f * fRadius. (2.0 is a little too big)
			// Beacuse light is farther away from the lit surfaces radius can be smaller so adjust this based on usage.
			float fRad = fRadius * 1.0f;

			// screen space light rectangle (clockwise) relative to light
			lCorners[0] = Vec3( -fRad, -fRad, 0.0f );
			lCorners[1] = Vec3( fRad, -fRad, 0.0f );
			lCorners[2] = Vec3( fRad, fRad, 0.0f );
			lCorners[3] = Vec3( -fRad, fRad, 0.0f );

			bbox.SetSnapshot( lCorners[0].x, lCorners[0].y, lCorners[2].x, lCorners[2].y );
			bbox.RestoreSnapshot( pos.xy_proj );

			m_bDirty = true;
		}
		break;
		case K_LVL_LT_DIRECTIONAL:
		{
			castShadows = false;
			fVolumeAlpha = 0.0f;
			bbox.Set( 0.0f, 0.0f, 0.0f, 0.0f );
			bbox.SaveSnapshot();
			m_bDirty = true;
		}
		break;
		case K_LVL_LT_AMBIENTAL:
		{
			castShadows = false;
			fVolumeAlpha = 0.0f;
		}
		break;
		case K_LVL_LT_PROJECTED_DIR:
		{
			pos = Vec3( pos.xyz.x, pos.xyz.y, 0.0f );
			castShadows = false;


			// intersects light direction with level top and bottom{} planes, projects back to 2D and make a union between them.
			// can still be optimized
			Vec3 vmove = -vnDir * K_WALL_HEIGHT_WORLD;
			CAABB lowRect = bbox.GetSnapshot();
			CAABB highRect = lowRect;
			Vec2 vmoveproj = Vec3ProjVec2( vmove );
			highRect.Move( vmoveproj );
			lowRect.Move( -vmoveproj );
			CAABB unionAABB = AABB::Union( lowRect, highRect );
			//clockwise
			lCorners[0] = Vec3( unionAABB.vMin.x, unionAABB.vMin.y, 0.0f );
			lCorners[1] = Vec3( unionAABB.vMax.x, unionAABB.vMin.y, 0.0f );
			lCorners[2] = Vec3( unionAABB.vMax.x, unionAABB.vMax.y, 0.0f );
			lCorners[3] = Vec3( unionAABB.vMin.x, unionAABB.vMax.y, 0.0f );

			if ( fidTexture.IsSet() && ( pLightsSprCol != null ) )
				lTexRect = pLightsSprCol->GetModuleRect_TexCoords( fidTexture.animIdx, fidTexture.frameIdx, 0 );

			m_bDirty = true;
		}
		break;
	}

}

void CLight::SetLightTexture( CSpriteLib* sprCol, int nAnimID, int nFrameID )
{
	fidTexture.animIdx = nAnimID;
	fidTexture.frameIdx = nFrameID;
	if ( fidTexture.animIdx >= 0 )
	{
		//lTexRect = sprCol->GetModuleRect_TexCoords(animID, frameID, 0);
		RectXYWHi lrect = sprCol->GetAFrameBBox_real( fidTexture.animIdx, fidTexture.frameIdx );
		bbox.Set( lrect );
		bbox.SaveSnapshot();
		bbox.Move( pos.xy_proj );
	}

	UpdateInternalData( sprCol );
}

CLight::CLight( CActiveAIComponent* pLightAIComp ) :
	c_AI( pLightAIComp ), type( K_LVL_LT_UNKNOWN ),
	m_arrVerts( nullptr ), m_bDirty( true ), m_nLightMeshIdx( -1 ), m_arrVertsCnt(0),
	fRadius( 0.0f ), fVolumeAlpha( 1.0f ), castShadows( false ), nProfileID( 0 )
{
	vnDir = { 0.0f, 0.0f, -1.0f }; //default direction (looking down)
	fidTexture.Reset();
}

CLight::~CLight()
{
	SAFE_DELETE( c_AI );
	SAFE_DELETE_ARRAY( m_arrVerts );
}

void CLight::SetPos( Vec3 newPos )
{
	if ( newPos != pos.xyz )
	{
		m_bDirty = true;
		pos = newPos;
		bbox.RestoreSnapshot( pos.xy_proj );
	}
}

void CLight::Move( Vec3 delta )
{
	if ( delta.x != 0.0f || delta.y != 0.0f || delta.z != 0.0f )
	{
		m_bDirty = true;
		Vec3 npos = pos.xyz + delta;
		pos = npos;
		bbox.RestoreSnapshot( pos.xy_proj );
	}
}

