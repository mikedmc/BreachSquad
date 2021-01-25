#include "dxstdafx.h"
#include "Level_bullets.h"


CBullet* CLevel::ShootBullet(CBulletTemplate * bulletTemplate, int actorClass, UINT32 nOwnerUID, D3DXVECTOR2 pos, D3DXVECTOR2 shootDir)
{
	//dull bullets don't actually get spawned (sometimes we need them)
	if (bulletTemplate->nType == K_LVL_BULLET_DULL)
	{
		return NULL;
	}

	//melee bullets
	if (bulletTemplate->nGroup == K_LVL_BULLGROUP_MELEE)
	{
		//make melee range slightly larger than bullet normal range.
		float fRangeObjects = (bulletTemplate->fSpeed_ini * bulletTemplate->fLife);
		float fRange = fRangeObjects * 1.5f; //larger range for humans
		int nHitActors = MeleeBlow(bulletTemplate->nType, pos, shootDir, nOwnerUID, actorClass, fRange, bulletTemplate->fDamage, bulletTemplate->fMomentum, bulletTemplate->fStunDuration, (EActorClass)actorClass, fRangeObjects, bulletTemplate->fDamageObjects);

		return NULL;
	}

	CLinkedPool<CBullet>::CLinkedPoolNode *node = m_poolBullets.HireNode();
	//set 
	if (node == null)
	{
		ErrorBox(K_ERR_WARNING, L"ShootBullet:We need more bullets!");
		return NULL;
	}

	//add simulation container
	node->m_data.physPt = m_poolPhysPts.HireNode();
	if (node->m_data.physPt == NULL)
	{
		ErrorBox(K_ERR_WARNING, L"ShootBullet:We need more physics points!");
		m_poolBullets.DismissNode(node);
		return NULL;
	}
	//reset physics data
	node->m_data.physPt->m_data.Init();
	//set bullet generic data
	node->m_data.actorClass = actorClass;
	node->m_data.ownerUID = nOwnerUID;
	node->m_data.dwLastTargetUID = 0;
	node->m_data.nSubstate = 0;

	node->m_data.type = bulletTemplate->nType;
	node->m_data.nFlags = bulletTemplate->nFlags;
	node->m_data.nExploTemplateHash = bulletTemplate->nExploTemplateHash;

	node->m_data.fStunDuration = bulletTemplate->fStunDuration;
	node->m_data.fDamage = bulletTemplate->fDamage;
	node->m_data.fDamage_ini = node->m_data.fDamage;
	node->m_data.fDamageLossPPx = bulletTemplate->fDamageLossPPx;
	node->m_data.fMomentum = bulletTemplate->fMomentum;
	node->m_data.fLife = bulletTemplate->fLife;
	node->m_data.fLife_ini = node->m_data.fLife;
	node->m_data.nArmorPiercingRating = bulletTemplate->nArmorPiercingRating;
	node->m_data.fSelfDamageMultiplier = bulletTemplate->fSelfDamageMultiplier;
	node->m_data.fCriticalHitChance = bulletTemplate->fCriticalHitChance;

	node->m_data.vSpawnPos = pos;
	//physics
	node->m_data.physPt->m_data.pos = pos;
	node->m_data.physPt->m_data.pos_last = pos;
	//randomizam viteza glontului cu un procent anume
	node->m_data.physPt->m_data.speed = shootDir * (bulletTemplate->fSpeed_ini + m_rand.RandFloatSgn(bulletTemplate->fSpeed_ini * 0.075f));
	//default states
	node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_FAST;
	node->m_data.physPt->m_data.bFlagPhysicsEnabled = false;
	//tail
	node->m_data.szTailSize.w = 0.0f;
	node->m_data.szTailSize.h = 0.0f;

	//particularizari gloante
	node->m_data.sprBullet.Init(ANM_PROPS_SPR_BULLETS, Vec2(0.0f, 0.0f), 0);

	return &node->m_data;
}

CBullet* CLevel::GetClosestBullet(D3DXVECTOR2 vCheckPos, EBulletType nBulletType, float fMaxDistance, int dwOwnerUID /*= 0*/)
{
	float fMinDist = 100000.0f;
	CBullet* pRetBullet = null;

	CLinkedPool<CBullet>::CLinkedPoolNode *node = m_poolBullets.pListUsed.m_pNext;
	while (node != &m_poolBullets.pListUsed)
	{
		//salvez locatia urmatoare ca sa pot avansa pe ea
		CLinkedPool<CBullet>::CLinkedPoolNode *nextnode = node->m_pNext;
		CBullet* bullet = &node->m_data;

		bool bPassed = true;
		if (bullet->type != nBulletType)
			bPassed = false;
		if ((dwOwnerUID != 0) && (bullet->ownerUID != dwOwnerUID))
			bPassed = false;
		if (bPassed)
		{
			float fDist = D3DXVec2Length(&(vCheckPos - bullet->physPt->m_data.pos));
			if ((fMaxDistance <= 0.0f) || ((fMaxDistance > 0.0f) && (fDist <= fMaxDistance)))
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

void CLevel::ReleaseBullet(int nBulletType, UINT32 nOwnerUID)
{
	CLinkedPool<CBullet>::CLinkedPoolNode *node = m_poolBullets.pListUsed.m_pNext;
	while (node != &m_poolBullets.pListUsed)
	{
		//salvez locatia urmatoare ca sa pot avansa pe ea
		CLinkedPool<CBullet>::CLinkedPoolNode *nextnode = node->m_pNext;
		CBullet* bullet = &node->m_data;

		bool killbullet = false;
		if ((bullet->type == nBulletType) && (bullet->ownerUID == nOwnerUID))
			killbullet = true;

		//ii dam release
		if (killbullet)
		{
			//release la nodul de fizica !!!
			m_poolPhysPts.DismissNode(bullet->physPt);
			//si eliberez glontul
			m_poolBullets.DismissNode(node);
		}

		//avansez pointer
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
		CLinkedPool<CBullet>::CLinkedPoolNode *nextnode = node->m_pNext;
		CBullet* bullet = &node->m_data;

		float fBulletOldLife = bullet->fLife;
		dec_limit(bullet->fLife, dTime, 0.0f);

		bool killbullet = false;

		// exited play area
		if (bullet->physPt->m_data.bIsDead)
			killbullet = true;


		//release the bullet
		if (killbullet)
		{
			//some bullets explode at the end
			if (bullet->nExploTemplateHash != 0)
			{
				D3DXVECTOR2 vExploDir(0.0f, 0.0f);
				if (bullet->nFlags & K_LVL_BULLET_FLAG_DIRECTIONAL)
				{
					vExploDir = bullet->physPt->m_data.speed;
				}
				D3DXVECTOR2 vExploPos = bullet->physPt->m_data.pos;
				if (bullet->physPt->m_data.bContacting)
					vExploPos += bullet->physPt->m_data.contactNormal * 2.0f;
				//now add explo
				AddProp_Explo(bullet->nExploTemplateHash, vExploPos, bullet->ownerUID, bullet->actorClass, vExploDir);
			}

			//release la nodul de fizica !!!
			m_poolPhysPts.DismissNode(bullet->physPt);
			//si eliberez glontul
			m_poolBullets.DismissNode(node);
		}


		//avansez pointer
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

	if (pass == K_LVL_RP_COLORS)
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
			//salvez locatia urmatoare ca sa pot avansa pe ea
			CLinkedPool<CBullet>::CLinkedPoolNode *nextnode = node->m_pNext;

			//D3DXVECTOR2 vdir = node->m_data.physPt->m_data.pos - node->m_data.physPt->m_data.pos_last;
			//float ang = UTMath::GetVectorAngle(vdir);
			node->m_data.sprBullet.pos = node->m_data.physPt->m_data.pos;
			node->m_data.sprBullet.PaintModule(&m_sprProps, 0);

			// advance to next bullet
			node = nextnode;
		}
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

		if ((bullet->type == nBulletType) && ((dwOwnerUID == 0) || (bullet->ownerUID == dwOwnerUID)))
		{
			//destroy charge
			bullet->nFlags |= K_LVL_BULLET_FLAG_KILLITNOW;
			nRetCnt++;
		}
		node = nextnode;
	}

	return nRetCnt;
}

