#include "dxstdafx.h"
#include "Level_bullets.h"


CBullet* CLevel::ShootBullet( CBulletTemplate * bulletTemplate, EActorClass actorClass, UINT32 nOwnerUID, Vec3 vPos, Vec3 vShootDir )
{
	CLevelArea* startArea = Areas_GetAt( Vec3XY( vPos ) );
	//dull bullets don't actually get spawned (sometimes we need them)
	if ( ( bulletTemplate->nType == K_LVL_BULLET_DULL ) || ( startArea == nullptr ) )
	{
		return nullptr;
	}
	//set bullet generic data
	CBullet* bullet = new CBullet( new CPointPhysComponent(true) );
	bullet->actorClass = actorClass;
	bullet->ownerUID = nOwnerUID;
	bullet->pArea = startArea;
	bullet->dwLastTargetUID = 0;
	bullet->nSubstate = 0;

	bullet->eType = bulletTemplate->nType;
	bullet->nFlags = bulletTemplate->nFlags;
	bullet->nExploTemplateHash = bulletTemplate->nExploTemplateHash;

	bullet->fStunDuration = bulletTemplate->fStunDuration;
	bullet->fDamage = bulletTemplate->fDamage;
	bullet->fDamage_ini = bullet->fDamage;
	bullet->fDamageLossPPx = bulletTemplate->fDamageLossPPx;
	bullet->fMomentum = bulletTemplate->fMomentum;
	bullet->fLife = bulletTemplate->fLife;
	bullet->fLife_ini = bullet->fLife;
	bullet->nArmorPiercingRating = bulletTemplate->nArmorPiercingRating;
	bullet->fSelfDamageMultiplier = bulletTemplate->fSelfDamageMultiplier;
	bullet->fCriticalHitChance = bulletTemplate->fCriticalHitChance;

	bullet->pos.Set( vPos );
	bullet->pos_last = bullet->pos;
	// randomize bullet speed
	Vec3 vdir = vShootDir * (bulletTemplate->fSpeed_ini + m_rand.RandFloatSgn(bulletTemplate->fSpeed_ini * 0.075f));
	bullet->c_pointPhys->SetSpeed(vdir);
	// hardcoded for now
	bullet->c_pointPhys->fBounceF = 0.0f;

	//bullet visuals
	bullet->sprBullet.Init(&m_sprProps, ANM_PROPS_SPR_BULLETS_NOANIM, bullet->pos.xy_proj, 0);
	bullet->fidLight.Init(ANM_PROPS_SPR_BULLETS_LIGHTS, 0);
	if (m_sprProps.GetAnimFlags(ANM_PROPS_SPR_BULLETS_NOANIM) & K_EDITOR_ANIMATION_FLAG_LOOPED)
		bullet->bAnimated = true;

	m_arrBullets.Add( bullet );

	return bullet;
}

CBullet* CLevel::GetClosestBullet(Vec2 vCheckPos, EBulletType nBulletType, float fMaxDistance, UINT32 dwOwnerUID /*= 0*/)
{
	float fMinDist = 100000.0f;
	float fMaxDistanceSq = fMaxDistance * fMaxDistance;
	CBullet* pRetBullet = null;

	for ( int kk = 0; kk < m_arrBullets.GetSize(); kk++ )
	{
		CBullet* bullet = m_arrBullets[ kk ];
		if ( bullet->bPendingKill )
			continue;

		bool bPassed = true;
		if (bullet->eType != nBulletType)
			bPassed = false;
		if ((dwOwnerUID != 0) && (bullet->ownerUID != dwOwnerUID))
			bPassed = false;
		if (bPassed)
		{
			float fDist = MUVec2LenSq(&(vCheckPos - bullet->pos.xy));
			if ((fMaxDistance <= 0.0f) || ((fMaxDistance > 0.0f) && (fDist <= fMaxDistanceSq)))
			{
				if (fDist < fMinDist)
				{
					fMinDist = fDist;
					pRetBullet = bullet;
				}
			}
		}
	}

	return pRetBullet;
}

void CLevel::ReleaseBulletType(int nBulletType, UINT32 nOwnerUID)
{
	for ( int kk = 0; kk < m_arrBullets.GetSize(); kk++ )
	{
		CBullet* bullet = m_arrBullets[ kk ];
		if ( bullet->bPendingKill )
			continue;

		if ((bullet->eType == nBulletType) && (bullet->ownerUID == nOwnerUID))
			bullet->bPendingKill = true;
	}
}

void CLevel::UpdateBullets(float dTime)
{
	static _VERTEX_PNCT4T4 arrBulletsTris[K_LVL_BULLETS_MAX_CNT * 6];
	// builds a dynamic mesh for the bullets
	m_bulletsMeshIdx = -1;
	int nBulletsTrisCnt = 0;

	for ( int kk = 0; kk < m_arrBullets.GetSize(); kk++ )
	{
		CBullet* bullet = m_arrBullets[ kk ];
		if ( bullet->bPendingKill )
			continue;
		// update bullet point physics component
		bullet->Update( dTime, ( *this ) );

		// animate sprite if necessary
		if ( bullet->bAnimated )
		{
			bullet->sprBullet.Update( dTime );
			// kill bullet if outside area. It should never get outside the area.
			if ( bullet->pArea == nullptr )
			{
				bullet->bPendingKill = true;
				continue;
			}
		}

		float fBulletOldLife = bullet->fLife;
		dec_limit( bullet->fLife, dTime, 0.0f );

		bool killbullet = false;

		// exited play area
		if ( bullet->c_pointPhys->bIsDead )
			killbullet = true;

		if ( bullet->c_pointPhys->bContacting )
		{
			killbullet = true;
		}

		//--- check collision with objects and actors ---
		//float fMinT = 100000.0f;
		Vec2 vColP, vColN;
		CVisibleSortable pRetObj;

		Vec2 vFrom = bullet->pos_last.xy_proj;
		Vec2 vTo = bullet->pos.xy_proj;
		CAABB aabbBullet;
		aabbBullet.Set_Corrected( vFrom, vTo );
		// collision return vars
		Vec2 vRetPt( 0.0f, 0.0f );
		float fRetT = 100000.0f;

		///----------------------------------------------------------------------------------
		/// Check collisions with objects and see which one is closer
		///----------------------------------------------------------------------------------
		if ( bullet->pArea != nullptr )
		{
			// props collision
			for ( auto prop : bullet->pArea->m_arrProps )
			{
				if ( ( !prop->IsAlive() ) || ( ( prop->flags & K_PROPFLAG_CAN_BE_SHOT ) == 0 ) )
					continue;
				if ( prop->bbox.Intersects(aabbBullet) )
				{
					if ( AABB::Segment_Intersection( vFrom, vTo, prop->bbox, &vRetPt ) )
					{
						//#TODO: return material too
						vColP = vRetPt;
						pRetObj.eType = K_VST_PROP;
						pRetObj.pPtr = prop;
						// shorten bullet vector so we only catch the closest ones
						vTo = vRetPt;
						aabbBullet.Set_Corrected( vFrom, vTo );
					}
				}
			}
			// collision with actors
			for ( auto actor : m_arrActors )
			{
				if ( ( !actor->IsAlive() ) || ( FLAG_ANY( actor->_template.eCaps, K_ACT_CAPS_NOT_A_TARGET ) ) )
					continue;
				// skip self class (no friendly fire)
				if ( bullet->actorClass == actor->_template.actorClass )
					continue;
				if ( actor->bbox.Intersects( aabbBullet ) )
				{
					if ( AABB::Segment_Intersection( vFrom, vTo, actor->bbox, &vRetPt ) )
					{
						//#TODO: return material too
						vColP = vRetPt;
						pRetObj.eType = K_VST_ACTOR;
						pRetObj.pPtr = actor;
						// shorten bullet vector so we only catch the closest ones
						vTo = vRetPt;
						aabbBullet.Set_Corrected( vFrom, vTo );
					}
				}
			}
			// Collision with props from neighbouring areas
			// we assume that if we had a collision with prop in current area then there will be no other closer prop in neighbouring areas
			if ( pRetObj.pPtr == nullptr )
			{
				for ( int oo = 0; oo < bullet->pArea->arrNeighbours.Count(); oo++ )
				{
					CLevelArea* area = bullet->pArea->arrNeighbours[ oo ];
					// eliminate areas that don't intersect bullet movement bbox
					if ( !aabbBullet.Intersects( area->AABBbounds ) )
						continue;

					for ( auto prop : area->m_arrProps )
					{
						if ( ( !prop->IsAlive() ) || ( ( prop->flags & K_PROPFLAG_CAN_BE_SHOT ) == 0 ) )
							continue;
						if ( prop->bbox.Intersects( aabbBullet ) )
						{
							if ( AABB::Segment_Intersection( vFrom, vTo, prop->bbox, &vRetPt ) )
							{
								//#TODO: return material too
								vColP = vRetPt;
								pRetObj.eType = K_VST_PROP;
								pRetObj.pPtr = prop;
								// shorten bullet vector so we only catch the closest ones
								vTo = vRetPt;
								aabbBullet.Set_Corrected( vFrom, vTo );
							}
						}
						//#TODO: check actors in neighboring areas too and make the check generalized somehow (don't clone code)
					}
				}
			}
		}

		///--- check hit object...
		if ( pRetObj.eType != K_VST_UNKNOWN )
		{
			_ASSERT( pRetObj.pPtr != nullptr );
			switch ( pRetObj.eType )
			{
				case K_VST_PROP:
				{
					auto hitprop = static_cast< CProp* >( pRetObj.pPtr );
					if ( nullptr != hitprop )
					{
						//#TODO: save targetedUID on bullet to avoid hitting same entity 2 times with penetrating bullets
						hitprop->Kill();
						killbullet = true;
						float fang = UTMath::GetVectorAngle( Vec3XY( -bullet->c_pointPhys->speed ) );
						__Particles().AddParticle( ANM_PARTICLES_SPR_IMPACT_FIRE1, true, 0, &vRetPt, nullptr,
							&Vec2( bullet->c_pointPhys->contactNormal * 20.0f ), 5.0f, 1.0f, 0.0f, -fang, 0.0f, 0.0f, 0.0f );
					}
				}
				break;
				case K_VST_ACTOR:
				{
					auto hitactor = static_cast< CActor* >( pRetObj.pPtr );
					if ( nullptr != hitactor )
					{
						//#TODO: save targetedUID on bullet to avoid hitting same entity 2 times with penetrating bullets
						CBulletHitReturnData bullhit = hitactor->HitActor( bullet );
						killbullet = true;
						float fang = UTMath::GetVectorAngle( Vec3XY( -bullet->c_pointPhys->speed ) );
						__Particles().AddParticle( ANM_PARTICLES_SPR_IMPACT_FIRE1, true, 0, &vRetPt, nullptr,
							&Vec2( bullet->c_pointPhys->contactNormal * 20.0f ), 5.0f, 1.0f, 0.0f, -fang, 0.0f, 0.0f, 0.0f );
					}
				}
				break;
				default:
					break;
			}
		}
		else
		{
			//#TEMP: assume wall hit for now but it might be the floor!
			if ( bullet->c_pointPhys->bContacting )
			{
				float fang = UTMath::GetVectorAngle(Vec3XY(bullet->c_pointPhys->contactNormal));
				__Particles().AddParticle(ANM_PARTICLES_SPR_IMPACT_SMOKE1, true, 0, &Vec3ProjVec2(bullet->c_pointPhys->contactPos), nullptr, 
					&Vec2(bullet->c_pointPhys->contactNormal * 20.0f), 5.0f, 1.0f, 0.0f, -fang, 0.0f, 0.0f, 0.0f);
			}
		}

		//release the bullet
		if ( killbullet )
		{
			//some bullets explode at the end
			if ( bullet->nExploTemplateHash != 0 )
			{
				/*
				Vec2 vExploDir(0.0f, 0.0f);
				if (bullet->nFlags & K_LVL_BULLET_FLAG_DIRECTIONAL)
				{
					vExploDir = bullet->physPt->m_data.speed;
				}
				Vec2 vExploPos = bullet->physPt->m_data.pos;
				if (bullet->physPt->m_data.bContacting)
					vExploPos += bullet->physPt->m_data.contactNormal * 2.0f;
				//now add explo
				AddProp_Explo(bullet->nExploTemplateHash, vExploPos, bullet->ownerUID, bullet->actorClass, vExploDir);
				*/
			}

			//and release the bullet
			bullet->bPendingKill = true;
		}
	}

	if (nBulletsTrisCnt > 0)
	{
		m_bufferedPainter.BeginMesh(m_bulletsMeshIdx);
		m_bufferedPainter.AddTriangles(arrBulletsTris, nBulletsTrisCnt);
		m_bufferedPainter.EndMesh();
	}
}

void CLevel::PaintBullets(eLVLRenderPass pass)
{
	D3DXMATRIXA16 matbullet;

	//#TODO: only paint visible bullets...
	switch(pass)
	{
		case K_LVL_RP_COLORS:
		{
			////bullet tails and other geometry
			/*
			if (m_bulletsMeshIdx >= 0)
			{
				//#HARDCODE: set first texture which contains color info
				m_pDevice->SetTexture(0, m_sprProps.Textures[0]->pTex);
				//draw textured bullets (actives texture, just like the bullets)
				m_bufferedPainter.DrawMesh(m_bulletsMeshIdx, true);
			}
			*/

			for ( int kk = 0; kk < m_arrBullets.Count(); kk++ )
			{
				CBullet* bullet = m_arrBullets[ kk ];
				if ( bullet->bPendingKill )
					continue;

				//Vec2 vdir = node->m_data.physPt->m_data.pos - node->m_data.physPt->m_data.pos_last;
				//float ang = UTMath::GetVectorAngle(vdir);
				bullet->sprBullet.pos = bullet->pos.xy_proj;
				bullet->sprBullet.PaintFModule(0);
				// paints shadow so we see where it falls:
				/*
				bullet->sprBullet.pos = bullet->pos.xy;
				bullet->sprBullet.PaintFModule(0);
				*/
			}
		}
		break;
		case K_LVL_RP_SHADOWS:
		{
			for ( int kk = 0; kk < m_arrBullets.Count(); kk++ )
			{
				CBullet* bullet = m_arrBullets[ kk ];
				if ( bullet->bPendingKill )
					continue;
				//Vec2 vdir = node->m_data.physPt->m_data.pos - node->m_data.physPt->m_data.pos_last;
				//float ang = UTMath::GetVectorAngle(vdir);
				UTSprite::PaintFModule(bullet->sprBullet.pSprCol, bullet->pos.xy, bullet->sprBullet.animIdx, bullet->sprBullet.frameIdx, 0, 0xaa000000);
			}
		}
		break;
		case K_LVL_RP_LIGHTS:
		{
			for ( int kk = 0; kk < m_arrBullets.Count(); kk++ )
			{
				CBullet* bullet = m_arrBullets[ kk ];
				if ( bullet->bPendingKill )
					continue;
				UTSprite::PaintFModule(bullet->sprBullet.pSprCol, bullet->pos.xy_proj, bullet->fidLight.animIdx, bullet->fidLight.frameIdx, 0);
			}
		}
		break;
	}
}

int CLevel::KillBulletsOfType(int nBulletType, UINT32 dwOwnerUID)
{
	int nRetCnt = 0;
	
	for ( int kk = 0; kk < m_arrBullets.Count(); kk++ )
	{
		CBullet* bullet = m_arrBullets[ kk ];
		if ((bullet->eType == nBulletType) && ((dwOwnerUID == 0) || (bullet->ownerUID == dwOwnerUID)))
		{
			bullet->bPendingKill = true;
			nRetCnt++;
		}
	}

	return nRetCnt;
}

CBullet::CBullet( CPointPhysComponent* pComPointPhys ) :
	eType( K_LVL_BULLET_SHOTGUN ), fDamage( 1.0f ), fDamage_ini( 1.0f ), fDamageLossPPx( 0.0f ),
	fLife( 1.0f ), fLife_ini( 1.0f ), actorClass( K_ACT_CLASS_PLAYER ), nSubstate( 0 ),
	fMomentum( 0.0f ), nFlags( 0 ), fStunDuration( 0.0f ), ownerUID( 0 ), dwLastTargetUID( 0 ),
	nArmorPiercingRating( 0 ), nExploTemplateHash( 0 ), fSelfDamageMultiplier( 1.0f ), fCriticalHitChance( 0.0f ),
	bAnimated( false ), pArea( nullptr ), bPendingKill( false )
{
	c_pointPhys = pComPointPhys;
}

CBullet::~CBullet()
{
	SAFE_DELETE( c_pointPhys );
}

void CBullet::Update( float dTime, CLevel & level )
{
	if ( bPendingKill )
		return;
	// only update physics component if we have one
	if ( c_pointPhys )
	{
		c_pointPhys->Update( pos, dTime, level );
		pArea = c_pointPhys->pArea;
	}
}
