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
	// clean target pointer (should be done by AI?)
	if ( (pTarget != nullptr) && pTarget->IsPendingKill() )
	{
		pTarget->FreeRef();
		pTarget = nullptr;
	}

	bEnabled = bSetEnabled;
	// hidden? skip update
	if ( !IsAlive() )
		return;

	c_AI->Update( *this, dTime, level );
	// enforce position updating after UI pass
	SetPos( pos.xyz );
}

void CLight::SetDir(Vec3 nDir)
{
	if (MUVec3AlmostZero(nDir))
	{
		vnDir = Vec3(0.0f, 0.0f, -1.0f); //looking down
		return;
	}

	MUVec3Norm(&vnDir, &nDir);
}

void CLight::UpdateInternalData(CSpriteLib* pLightsSprCol)
{
	switch (type)
	{
		case K_LVL_LT_IES:
		case K_LVL_LT_POINT:
		{
			// Gaussian attenuated radius. 
			// The attenuation with a 0.55 coefficient dops off to 0 at about 2.0f * fRadius. (2.0 is a little too big)
			// Beacuse light is farther away from the lit surfaces radius can be smaller so adjust this based on usage.
			float fRad = fRadius * 1.0f;

			// screen space light rectangle (clockwise) relative to light
			lCorners[0] = Vec3(-fRad, -fRad, 0.0f);
			lCorners[1] = Vec3(fRad, -fRad, 0.0f);
			lCorners[2] = Vec3(fRad, fRad, 0.0f);
			lCorners[3] = Vec3(-fRad, fRad, 0.0f);

			bbox_ini.Set(lCorners[0].x, lCorners[0].y, lCorners[2].x, lCorners[2].y);
		}
		break;
		case K_LVL_LT_DIRECTIONAL:
		{
			castShadows = false;
			fVolumeAlpha = 0.0f;
			bbox_ini.Set(Vec2(0.0f, 0.0f), Vec2(0.0f, 0.0f));
			bbox = bbox_ini;
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
			pos = Vec3(pos.xyz.x, pos.xyz.y, 0.0f);
			castShadows = false;


			// intersects light direction with level top and bottom planes, projects back to 2D and make a union between them.
			// can still be optimized
			Vec3 vmove = -vnDir * K_WALL_HEIGHT_WORLD;
			CAABB lowRect(bbox_ini);
			CAABB highRect(bbox_ini);
			Vec2 vmoveproj = Vec3ProjVec2(vmove);
			highRect.Move(vmoveproj);
			lowRect.Move(-vmoveproj);
			CAABB unionAABB = AABB::Union(lowRect, highRect);
			//clockwise
			lCorners[0] = Vec3(unionAABB.vMin.x, unionAABB.vMin.y, 0.0f);
			lCorners[1] = Vec3(unionAABB.vMax.x, unionAABB.vMin.y, 0.0f);
			lCorners[2] = Vec3(unionAABB.vMax.x, unionAABB.vMax.y, 0.0f);
			lCorners[3] = Vec3(unionAABB.vMin.x, unionAABB.vMax.y, 0.0f);

			if ((animID >= 0) && (pLightsSprCol != null))
				lTexRect = pLightsSprCol->GetModuleRect_TexCoords(animID, frameID, 0);
		}
		break;
	}

}

void CLight::SetLightTexture(CSpriteLib* sprCol, int nAnimID, int nFrameID)
{
	animID = nAnimID;
	frameID = nFrameID;
	if (animID >= 0)
	{
		//lTexRect = sprCol->GetModuleRect_TexCoords(animID, frameID, 0);
		RectXYWHi lrect = sprCol->GetAFrameBBox_real(animID, frameID);
		bbox_ini.Set(lrect);
		bbox = bbox_ini;
		bbox.Move(pos.xy_proj);
	}

	UpdateInternalData(sprCol);
}

CLight::CLight( CLightAIComponent* pLightAIComp ) :
	m_nLightMeshIdx( -1 ), m_nShadowMeshIdx( -1 ),
	type( K_LVL_LT_UNKNOWN ), animID( -1 ), frameID( 0 ),
	fRadius( 0.0f ), fVolumeAlpha( 1.0f ), castShadows( false ),
	nProfileID( 0 ),
	c_AI( pLightAIComp )
{
	vnDir = Vec3(0.0f, 0.0f, -1.0f); //default direction (looking down)
}

CLight::~CLight()
{
	SAFE_DELETE( c_AI );
}

void CLight::SetPos(Vec3 newPos)
{
	pos = newPos;
	bbox = bbox_ini;
	bbox.Move(pos.xy_proj);
}

void CLight::Move(Vec3 delta)
{
	Vec3 npos = pos.xyz + delta;
	pos = npos;
	bbox = bbox_ini;
	bbox.Move(pos.xy_proj);
}

