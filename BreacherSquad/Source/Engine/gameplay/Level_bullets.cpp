#include "dxstdafx.h"
#include "Level_bullets.h"


CBullet* CLevel::ShootBullet(CBulletTemplate * bulletTemplate, int actorClass, UINT32 nOwnerUID, Vec3 vPos, Vec3 vShootDir, CLevelArea* pStartArea)
{
	//dull bullets don't actually get spawned (sometimes we need them)
	if (bulletTemplate->nType == K_LVL_BULLET_DULL)
	{
		return nullptr;
	}

	CLinkedPool<CBullet>::CLinkedPoolNode *node = m_poolBullets.HireNode();
	//set 
	if (node == null)
	{
		ErrorBox(K_ERR_WARNING, L"ShootBullet:We need more bullets!");
		return nullptr;
	}

	//add simulation container
	node->m_data.physPt = m_poolPhysPts.HireNode();
	if (node->m_data.physPt == nullptr)
	{
		ErrorBox(K_ERR_WARNING, L"ShootBullet:We need more physics points!");
		m_poolBullets.DismissNode(node);
		return nullptr;
	}
	//reset physics data
	node->m_data.physPt->m_data.Reset();
	//set bullet generic data
	CBullet* bullet = &node->m_data;
	bullet->actorClass = actorClass;
	bullet->ownerUID = nOwnerUID;
	bullet->pArea = pStartArea;
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

	bullet->pos_ini = vPos;
	bullet->posProj = Vec3ProjVec2(vPos);
	bullet->posShadow = Vec3ToVec2XY(vPos);
	//physics
	bullet->physPt->m_data.pArea = bullet->pArea;
	bullet->physPt->m_data.pos = vPos;
	bullet->physPt->m_data.pos_last = vPos;
	// randomize bullet speed
	Vec3 vdir = vShootDir * (bulletTemplate->fSpeed_ini + m_rand.RandFloatSgn(bulletTemplate->fSpeed_ini * 0.075f));
	bullet->physPt->m_data.speed = vdir;
	// hardcoded for now
	bullet->physPt->m_data.accel = g_Vec3Zero;
	bullet->physPt->m_data.fBounceF = 0.0f;
	//default states
	bullet->physPt->m_data.bFlagPhysicsEnabled = false;
	//tail
	bullet->szTailSize.w = 0.0f;
	bullet->szTailSize.h = 0.0f;

	//bullet visuals
	bullet->sprBullet.Init(&m_sprProps, ANM_PROPS_SPR_BULLETS_NOANIM, bullet->posProj, 0);
	bullet->fidLight.Init(ANM_PROPS_SPR_BULLETS_LIGHTS, 0);
	if (m_sprProps.GetAnimFlags(ANM_PROPS_SPR_BULLETS_NOANIM) & K_EDITOR_ANIMATION_FLAG_LOOPED)
		bullet->bAnimated = true;

	return &node->m_data;
}

CBullet* CLevel::GetClosestBullet(Vec2 vCheckPos, EBulletType nBulletType, float fMaxDistance, int dwOwnerUID /*= 0*/)
{
	float fMinDist = 100000.0f;
	float fMaxDistanceSq = fMaxDistance * fMaxDistance;
	CBullet* pRetBullet = null;

	CLinkedPool<CBullet>::CLinkedPoolNode *node = m_poolBullets.pListUsed.m_pNext;
	while (node != &m_poolBullets.pListUsed)
	{
		//salvez locatia urmatoare ca sa pot avansa pe ea
		CLinkedPool<CBullet>::CLinkedPoolNode *nextnode = node->m_pNext;
		CBullet* bullet = &node->m_data;

		bool bPassed = true;
		if (bullet->eType != nBulletType)
			bPassed = false;
		if ((dwOwnerUID != 0) && (bullet->ownerUID != dwOwnerUID))
			bPassed = false;
		if (bPassed)
		{
			float fDist = MUVec2LenSq(&(vCheckPos - Vec3ToVec2XY(bullet->physPt->m_data.pos)));
			if ((fMaxDistance <= 0.0f) || ((fMaxDistance > 0.0f) && (fDist <= fMaxDistanceSq)))
			{
				if (fDist < fMinDist)
				{
					fMinDist = fDist;
					pRetBullet = bullet;
				}
			}
		}

		//avansez pointer
		node = nextnode;
	}

	return pRetBullet;
}

void CLevel::ReleaseBulletType(int nBulletType, UINT32 nOwnerUID)
{
	CLinkedPool<CBullet>::CLinkedPoolNode *node = m_poolBullets.pListUsed.m_pNext;
	while (node != &m_poolBullets.pListUsed)
	{
		CLinkedPool<CBullet>::CLinkedPoolNode *nextnode = node->m_pNext;
		CBullet* bullet = &node->m_data;

		bool killbullet = false;
		if ((bullet->eType == nBulletType) && (bullet->ownerUID == nOwnerUID))
			killbullet = true;

		if (killbullet)
		{
			//release phys point
			m_poolPhysPts.DismissNode(bullet->physPt);
			//and bullet
			m_poolBullets.DismissNode(node);
		}

		node = nextnode;
	}
}

void CLevel::UpdateBullets(float dTime)
{
	//we'll store some important bullets in m_arrBulletsTemp so we can quickly check them later on when updating AIs
	m_arrBulletsTemp.Clear();

	static _VERTEX_PNCT4T4 arrBulletsTris[K_LVL_BULLETS_MAX_CNT * 6];
	//mesh dinamic pentru gloante
	m_bulletsMeshIdx = -1;
	int nBulletsTrisCnt = 0;

	bool bGoreEnabled = UTGetAppClass().m_Settings.bGoreEnabled;

	CLinkedPool<CBullet>::CLinkedPoolNode *node = m_poolBullets.pListUsed.m_pNext;
	while (node != &m_poolBullets.pListUsed)
	{
		// save link to next node as we might deallocate current node
		CLinkedPool<CBullet>::CLinkedPoolNode *nextnode = node->m_pNext;

		CBullet* bullet = &node->m_data;  
		// copy area pointer from phys pt
		bullet->pArea = bullet->physPt->m_data.pArea;
		// animate sprite if necessary
		if (bullet->bAnimated)
		{
			bullet->sprBullet.Update(dTime);
		}

		float fBulletOldLife = bullet->fLife;
		dec_limit(bullet->fLife, dTime, 0.0f);

		bool killbullet = false;

		// exited play area
		if (bullet->physPt->m_data.bIsDead)
			killbullet = true;

		// update position triplets
		bullet->posProj = Vec3ProjVec2(bullet->physPt->m_data.pos);
		bullet->posShadow = Vec3ToVec2XY(bullet->physPt->m_data.pos);

		if (bullet->physPt->m_data.bContacting)
		{
			killbullet = true;
		}

		//release the bullet
		if (killbullet)
		{
			//some bullets explode at the end
			if (bullet->nExploTemplateHash != 0)
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

			//release phys point
			m_poolPhysPts.DismissNode(bullet->physPt);
			//and release the bullet
			m_poolBullets.DismissNode(node);
		}


		//advance pointer
		node = nextnode;
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
	CLinkedPool<CBullet>::CLinkedPoolNode *node = m_poolBullets.pListUsed.m_pNext;

	switch(pass)
	{
		case K_LVL_RP_COLORS:
		{
			////bullet tails and other geometry
			if (m_bulletsMeshIdx >= 0)
			{
				//#HARDCODE: set first texture which contains color info
				m_pDevice->SetTexture(0, m_sprProps.Textures[0]->pTex);
				//draw textured bullets (actives texture, just like the bullets)
				m_bufferedPainter.DrawMesh(m_bulletsMeshIdx, true);
			}

			while (node != &m_poolBullets.pListUsed)
			{
				CBullet* bullet = &node->m_data;
				//Vec2 vdir = node->m_data.physPt->m_data.pos - node->m_data.physPt->m_data.pos_last;
				//float ang = UTMath::GetVectorAngle(vdir);
				bullet->sprBullet.pos = bullet->posProj;
				bullet->sprBullet.PaintModule(0);

				// advance to next bullet
				node = node->m_pNext;
			}
		}
		break;
		case K_LVL_RP_SHADOWS:
		{
			while (node != &m_poolBullets.pListUsed)
			{
				CBullet* bullet = &node->m_data;
				//Vec2 vdir = node->m_data.physPt->m_data.pos - node->m_data.physPt->m_data.pos_last;
				//float ang = UTMath::GetVectorAngle(vdir);
				UTSprite::PaintFrameModule(bullet->sprBullet.pSprCol, bullet->posShadow, bullet->sprBullet.animIdx, bullet->sprBullet.frameIdx, 0, 0xaa000000);

				// advance to next bullet
				node = node->m_pNext;
			}
		}
		break;
		case K_LVL_RP_LIGHTS:
		{
			while (node != &m_poolBullets.pListUsed)
			{
				CBullet* bullet = &node->m_data;
				UTSprite::PaintFrameModule(bullet->sprBullet.pSprCol, bullet->posProj, bullet->fidLight.animIdx, bullet->fidLight.frameIdx, 0);
				// advance to next bullet
				node = node->m_pNext;
			}
		}
		break;
	}
}

int CLevel::KillBulletsOfType(int nBulletType, UINT32 dwOwnerUID)
{
	int nRetCnt = 0;
	//verifica daca ai aruncat deja un charge
	CLinkedPool<CBullet>::CLinkedPoolNode *node = m_poolBullets.pListUsed.m_pNext;
	while (node != &m_poolBullets.pListUsed)
	{
		//salvez locatia urmatoare ca sa pot avansa pe ea
		CLinkedPool<CBullet>::CLinkedPoolNode *nextnode = node->m_pNext;
		CBullet* bullet = &node->m_data;

		if ((bullet->eType == nBulletType) && ((dwOwnerUID == 0) || (bullet->ownerUID == dwOwnerUID)))
		{
			//destroy charge
			bullet->nFlags |= K_LVL_BULLET_FLAG_KILLITNOW;
			nRetCnt++;
		}
		node = nextnode;
	}

	return nRetCnt;
}

