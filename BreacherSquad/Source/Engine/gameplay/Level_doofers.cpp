#include "dxstdafx.h"
#include "Level_doofers.h"

void CLevel::AddDoofer(EDooferType type, Vec2 pos, Vec2 * speed, Vec2 * accel, int nSubType /*= 0*/)
{
	/*
	bool bGoreEnabled = UTGetAppClass().m_Settings.bGoreEnabled;
	if ((!bGoreEnabled) && (type == K_SPROP_MEAT))
		return;
	//pre-checks
	if (type == K_SPROP_MEAT)
	{
		//don't spawn meat if inside collisions
		if (GetCollisionShapeAt(pos, K_LVL_COLL_TYPE_SOLID) != null)
			return;
	}

	CLinkedPool<CSpecialProp>::CLinkedPoolNode *node = m_poolProps.HireNode();
	//set
	if (node != null)
	{
		node->m_data.Reset();
		//add simulation container
		node->m_data.physPt = m_poolPhysPts.HireNode();
		if (node->m_data.physPt == NULL)
		{
			ErrorBox(K_ERR_WARNING, L"AddProp:We need more physics points!");
			m_poolProps.DismissNode(node);
			return;
		}
		//reset
		node->m_data.physPt->m_data.Init();

		node->m_data.type = type;
		node->m_data.nSubType = nSubType;

		switch (type)
		{
			case K_SPROP_SHELL:
			{
				//check subtype validity
				if (nSubType * 4 + 3 >= m_sprProps.GetAFramesCnt(ANM_ACTIVES_SPR_SHELLS))
				{
					ErrorBox(K_ERR_WARNING, L"Invalid weapon nDropShellFrame param! resetting to 0");
					node->m_data.nSubType = 0;
				}
				//all shells are in the same animation, 4 frames each
				node->m_data.spr.Init(ANM_ACTIVES_SPR_SHELLS, 0.0f, 0.0f, node->m_data.nSubType * 4 + randint(4));
				node->m_data.bVar1 = false; //face sunet o singura data la coliziune apoi seteaza bVar1 pe true
				//physics
				node->m_data.physPt->m_data.eCollType = CPhysicsPoint2D::K_COLLTYPE_FAST;
				node->m_data.physPt->m_data.bFlagPhysicsEnabled = true;

				node->m_data.physPt->m_data.pos = pos;
				if (speed != null)
					node->m_data.physPt->m_data.speed = *speed;
				if (accel != null)
					node->m_data.physPt->m_data.accel = *accel;
			}
			break;

		}
	}
	*/
}

void CLevel::AddDoofer_Light(Vec2 pos, int nLightAnimIdx, float fDuration, float fFadeTime, DWORD color, float fScale)
{
	Vec3 vPos = Vec2ToVec3XY0(pos);
	CLinkedPool<CDoofer>::CLinkedPoolNode *node = m_poolDoofers.HireNode();
	//set 
	if (node != nullptr)
	{
		node->m_data.Reset();
		//add simulation container
		node->m_data.physPt = m_poolPhysPts.HireNode();
		if (node->m_data.physPt == nullptr)
		{
			ErrorBox(K_ERR_WARNING, L"AddProp_Light:We need more physics points!");
			m_poolDoofers.DismissNode(node);
			return;
		}
		//reset
		node->m_data.physPt->m_data.Reset();

		node->m_data.type = K_DOOFER_LIGHT;

		node->m_data.sprLight.Init(nLightAnimIdx, 0.0f, 0.0f, 0, color);
		node->m_data.fLightScaling = fScale * K_LVL_LIGHTRENDER_BSX_SCALING;
		node->m_data.bMakesLight = true;
		node->m_data.fLightDuration = fDuration;
		node->m_data.fLightFadeOut = fFadeTime;

		node->m_data.fTimer = 0.0f;
		//physics
		node->m_data.physPt->m_data.bFlagPhysicsEnabled = false;
		node->m_data.physPt->m_data.bFlagRotationEnabled = false;

		node->m_data.physPt->m_data.pos = vPos;
		node->m_data.physPt->m_data.speed = g_Vec3Zero;
		node->m_data.physPt->m_data.accel = g_Vec3Zero;
	}
}

//#TODO: de generalizat total exploziile la final cand stiu cum vor arata si cate tipuri vor fi. Sa am in xml si animatie si scalare si ce fel de particule arunca etc
void CLevel::AddDoofer_Explo(UINT32 exploNameHash, Vec2 pos, UINT32 dwOwnerUID, int exploOwnerClass, Vec2 vExploDir, CAABB* exploAABB)
{
	CExplosionTemplate* explotemplate = GetTemplateExplosion(exploNameHash);
	if (explotemplate == nullptr)
		return;

	CLinkedPool<CDoofer>::CLinkedPoolNode *node = m_poolDoofers.HireNode();
	//set 
	if (node != nullptr)
	{
		node->m_data.Reset();
		//add simulation container
		node->m_data.physPt = m_poolPhysPts.HireNode();
		if (node->m_data.physPt == nullptr)
		{
			ErrorBox(K_ERR_WARNING, L"AddProp_Explo:Need more physics points!");
			m_poolDoofers.DismissNode(node);
			return;
		}
		//reset
		node->m_data.physPt->m_data.Reset();

		node->m_data.type = K_DOOFER_EXPLOSION;
		//physics
		node->m_data.physPt->m_data.bFlagPhysicsEnabled = false;
		node->m_data.physPt->m_data.bFlagRotationEnabled = false;

		node->m_data.physPt->m_data.pos = Vec2ToVec3XY0(pos);
		node->m_data.physPt->m_data.speed = g_Vec3Zero;
		node->m_data.physPt->m_data.accel = g_Vec3Zero;

		//default
		float fMaxDamage = explotemplate->fDamage;
		float fMaxStun = explotemplate->fStunDuration;
		float fDamageRadius = explotemplate->fDamageRadius;
		float fStunRadius = explotemplate->fStunRadius;
		float fMaxImpulse = explotemplate->fMaxImpulse;

		//add sound event
		if (explotemplate->fSoundRadius > 0.0f)
			AddAIEvent(K_LVL_AI_EVENT_SOUND_THREAT, 0, exploOwnerClass, pos, explotemplate->fSoundRadius, 1.0f);

		if (exploAABB == null)
		{
			//shrapnel
			for (int ll = 0; ll < explotemplate->nShrapnelCnt; ll++)
			{
				float fdx = m_rand.RandFloatSgn(150.0f);
				float fdy = -100.0f - m_rand.RandFloat(150.0f);
				AddDoofer(K_DOOFER_SHRAPNEL_SMOKING, pos, &Vec2(fdx, fdy), &g_vecGravityOld);
			}
			//napalm
			for (int ll = 0; ll < explotemplate->nNapalmCnt; ll++)
			{
				float fdx = m_rand.RandFloatSgn(60.0f);
				float fdy = -100.0f - m_rand.RandFloat(120.0f);
				AddDoofer(K_DOOFER_FIRE_SOURCE, pos, &Vec2(fdx, fdy), &g_vecGravityOld);
			}
		}
		else
		{
			//shrapnel
			for (int ll = 0; ll < explotemplate->nShrapnelCnt; ll++)
			{
				float fdx = m_rand.RandFloatSgn(150.0f);
				float fdy = -100.0f - m_rand.RandFloat(150.0f);
				AddDoofer(K_DOOFER_SHRAPNEL_SMOKING, pos + m_rand.RandVec2Sgn(exploAABB->vHalfSize.x, exploAABB->vHalfSize.y),
					&Vec2(fdx, fdy), &g_vecGravityOld);
			}
			//napalm
			for (int ll = 0; ll < explotemplate->nNapalmCnt; ll++)
			{
				float fdx = m_rand.RandFloatSgn(60.0f);
				float fdy = -100.0f - m_rand.RandFloat(120.0f);
				AddDoofer(K_DOOFER_FIRE_SOURCE, pos + m_rand.RandVec2Sgn(exploAABB->vHalfSize.x, exploAABB->vHalfSize.y),
					&Vec2(fdx, fdy), &g_vecGravityOld);
			}
		}

		//explo direction
		float fExploAng = UTMath::GetVectorAngle(vExploDir);
		//#TODO: explo-interactAI e o proprietate ce va fi exportata (interactioneaza cu AI-uri care se activeaza la explozii?)
		bool bInteractAI = false;

		if (explotemplate->name.textHash == hash_EXPLO_INVISIBLE_EXPLODING_ZOMBIE)
		{
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_GREEN_FATZOMBIE, true, 0, &Vec2(pos.x, pos.y + 6.0f), NULL, NULL, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_BARREL)
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_ROUND_XL, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;

			node->m_data.fTimer = 0.4f;

			//prop - light
//			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 1.0f, 0.2f, 0xffFDB727, 2.0f);
			//add visually stunning stuff
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_ROUND_XL, true, 0, &pos, NULL, &Vec2(0.0f, -20.0f), 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			//add ring
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLOWS, false, 1, &pos, NULL, NULL, 0.15f, 1.0f, 16.0f, 0.0f, 0.0f, 0.05f, 0.1f, 0x55fdb727, K_PART_LAYER_FRONT_LIGHT);
			g_particlesMgr.GenerateSmokePuff(pos, 20.0f, K_PART_LAYER_RT_FRONT_NRM);

			m_camLevel.ShakeScreen(4.0f, 8.0f, &pos);
			//SND_PLAY_POSITIONAL_RAND2(SNDIDX_EXPLOSION_BARREL_01, SNDIDX_EXPLOSION_BARREL_02, pos);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_LARGE)
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_ROUND_LG1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;

			node->m_data.fTimer = 0.4f;

			//prop - light
//			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.8f, 0.2f, 0xffFDB727, 1.6f);
			//add visually stunning stuff
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_ROUND_LG1, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			//add ring
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLOWS, false, 1, &pos, NULL, NULL, 0.15f, 1.0f, 16.0f, 0.0f, 0.0f, 0.05f, 0.1f, 0x55fdb727, K_PART_LAYER_FRONT_LIGHT);
			g_particlesMgr.GenerateSmokePuff(pos, 20.0f, K_PART_LAYER_RT_FRONT_NRM);

			m_camLevel.ShakeScreen(6.0f, 10.0f, &pos);
			//SND_PLAY_POSITIONAL_RAND2(SNDIDX_EXPLOSION_FRAG_GRENADE_01, SNDIDX_EXPLOSION_FRAG_GRENADE_02, pos);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_LARGE_XL)
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_ROUND_LG2, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;

			node->m_data.fTimer = 0.4f;

			//prop - light
//			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.8f, 0.2f, 0xffFDB727, 1.6f);
			//add visually stunning stuff
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_ROUND_LG2, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			//add ring
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLOWS, false, 1, &pos, NULL, NULL, 0.15f, 1.0f, 16.0f, 0.0f, 0.0f, 0.05f, 0.1f, 0x55fdb727, K_PART_LAYER_FRONT_LIGHT);
			g_particlesMgr.GenerateSmokePuff(pos, 20.0f, K_PART_LAYER_RT_FRONT_NRM);

			m_camLevel.ShakeScreen(4.0f, 8.0f, &pos);
			//SND_PLAY_POSITIONAL_RAND2(SNDIDX_EXPLOSION_FRAG_GRENADE_01, SNDIDX_EXPLOSION_FRAG_GRENADE_02, pos);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_MOLOTOV)
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_ROUND_SM1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;
			node->m_data.fTimer = 0.4f;

			//prop - light
//			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.8f, 0.2f, 0xffFDB727, 1.6f);
			//add visually stunning stuff
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_ATOMIC1, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			//add ring
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLOWS, false, 1, &pos, NULL, NULL, 0.15f, 1.0f, 16.0f, 0.0f, 0.0f, 0.05f, 0.1f, 0x55fdb727, K_PART_LAYER_FRONT_LIGHT);
			g_particlesMgr.GenerateSmokePuff(pos, 20.0f, K_PART_LAYER_RT_FRONT_NRM);

			m_camLevel.ShakeScreen(4.0f, 8.0f, &pos);
			//SND_PLAY_POSITIONAL_RAND2(SNDIDX_MOLOTOV_EXPLOSION_01, SNDIDX_MOLOTOV_EXPLOSION_02, pos);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_GREEN_GOO)
		{
		}
		else if (explotemplate->name.textHash == hash_EXPLO_GREEN_GOO_GROUND)
		{
		}
		else if (explotemplate->name.textHash == hash_EXPLO_BURN_DOT)
		{
			bInteractAI = FALSE;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_FIRECRACKER1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;
			node->m_data.fTimer = 0.4f;

			//add visually stunning stuff
			if (randompercent(50.0f))
				g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRECRACKER1, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			else
				g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRECRACKER2, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_FLAME_JET)
		{
			bInteractAI = FALSE;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_FIRECRACKER1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;
			node->m_data.fTimer = 0.4f;

			//add visually stunning stuff
			if (randompercent(50.0f))
				g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRECRACKER1, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			else
				g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRECRACKER2, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_CHARGE)
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_CHARGE1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;

			node->m_data.fTimer = 0.2f;

			//prop - light
//			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.6f, 0.2f, 0xffFDB727, 1.6f);
			//add visually stunning stuff
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_CHARGE1, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			//add ring
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLOWS, false, 1, &pos, NULL, NULL, 0.15f, 1.0f, 16.0f, 0.0f, 0.0f, 0.05f, 0.1f, 0x55fdb727, K_PART_LAYER_FRONT_LIGHT);
			g_particlesMgr.GenerateSmokePuff(pos, 20.0f, K_PART_LAYER_RT_FRONT_NRM);

			m_camLevel.ShakeScreen(4.0f, 8.0f, &pos);
			//SND_PLAY_POSITIONAL_RAND2(SNDIDX_EXPLOSION_BREACHING_CHARGE_01, SNDIDX_EXPLOSION_BREACHING_CHARGE_02, pos);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_CHARGE_INVISIBLE)
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_CHARGE1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;

			node->m_data.fTimer = 0.2f;
		}
		else if (explotemplate->name.textHash == hash_EXPLO_STUN_INVISIBLE)
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_CHARGE1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;

			node->m_data.fTimer = 0.2f;
		}
		else if ((explotemplate->name.textHash == hash_EXPLO_GRENADE) || (explotemplate->name.textHash == hash_EXPLO_BLOWUP_VEST))
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_ROUND_SM1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.5f;

			node->m_data.fTimer = 0.2f;

			//prop - light
//			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.6f, 0.2f, 0xffFDB727, 1.6f);
			//add visually stunning stuff
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_ROUND_SM1, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			//add ring
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLOWS, false, 1, &pos, NULL, NULL, 0.15f, 1.0f, 16.0f, 0.0f, 0.0f, 0.05f, 0.1f, 0x55fdb727, K_PART_LAYER_FRONT_LIGHT);
			g_particlesMgr.GenerateSmokePuff(pos, 20.0f, K_PART_LAYER_RT_FRONT_NRM);

			m_camLevel.ShakeScreen(4.0f, 8.0f, &pos);
			//			SND_PLAY_POSITIONAL_RAND2(SNDIDX_EXPLOSION_FRAG_GRENADE_01, SNDIDX_EXPLOSION_FRAG_GRENADE_02, pos);
		}
		else if (explotemplate->name.textHash == hash_EXPLO_GRENADE_GROUND)
		{
			bInteractAI = true;

			node->m_data.spr.Init(ANM_PARTICLES_SPR_EXPLO_GROUND1, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.5f;

			node->m_data.fTimer = 0.2f;

			//prop - light
//			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 0.6f, 0.2f, 0xffFDB727, 1.6f);
			//add visually stunning stuff
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_EXPLO_GROUND1, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
			//add ring
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_GLOWS, false, 1, &pos, NULL, NULL, 0.15f, 1.0f, 16.0f, 0.0f, 0.0f, 0.05f, 0.1f, 0x55fdb727, K_PART_LAYER_FRONT_LIGHT);
			g_particlesMgr.GenerateSmokePuff(pos, 20.0f, K_PART_LAYER_RT_FRONT_NRM);

			m_camLevel.ShakeScreen(4.0f, 8.0f, &pos);
			//			SND_PLAY_POSITIONAL_RAND2(SNDIDX_EXPLOSION_FRAG_GRENADE_01, SNDIDX_EXPLOSION_FRAG_GRENADE_02, pos);
		}
		else if ((explotemplate->name.textHash == hash_EXPLO_FLASHBANG) || (explotemplate->name.textHash == hash_EXPLO_SHIELD_FLASH))
		{
			node->m_data.spr.Init(ANM_PARTICLES_SPR_FLASH_AIR, 0.0f, 0.0f, 0);
			node->m_data.fSize = 1.0f;
			node->m_data.fTimer = 0.2f;
			//prop - light
//			AddProp_Light(pos, ANM_LIGHTS_SPR_POINT1, 1.0f, 0.2f, 0xffffffff, 1.5f);
			//add visually stunning stuff
			g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FLASH_AIR, true, 0, &pos, NULL, NULL, 1.0f, node->m_data.fSize, 0.0f, fExploAng, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);

			m_camLevel.ShakeScreen(3.0f, 4.0f, &pos);

			//			SND_PLAY_POSITIONAL_RAND2(SNDIDX_EXPLOSION_FLASHBANG_01, SNDIDX_EXPLOSION_FLASHBANG_02, pos);
		}

		//pointer to player that spawned the explosion, or null if it wasn't a player
		CActor* pPlayer = GetPlayerByUID(dwOwnerUID);

		//#IMPORTANT #TODO: should optimize in order to minimize the usage of UnobstructedLineOfSight
		//stun enemy and damage over time
		if ((fMaxStun > 0.0f) || (explotemplate->cDoT.eType != CDamageOverTime::K_LVL_DoT_NONE))
		{
			//find all actors and damage them (linearly)
			for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
			{
				CActor* act = m_arrActors[kk];
				//sar actorii ascunsi
				if ((act->bHidden) || (act->fLife < 0.0f) || (act->actTemplate.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET))
					continue;
				//never stun the hostages
				if ((act->actTemplate.actorClass == K_LVL_ACT_CLASS_HOSTAGE) && (fMaxStun > 0.0f))
					continue;
				//distanta la inamic
				Vec2 vDir = act->GetPosHeart() - pos;
				float fDist = D3DXVec2Length(&vDir);

				bool bDirectLine = IsLineOfSight(act->GetPosHeart(), pos);
				//daca am damage over time il setez pe actor
				if ((bDirectLine) && (explotemplate->cDoT.eType != CDamageOverTime::K_LVL_DoT_NONE) && (fDist < explotemplate->fDoTRadius))
				{
					//momentan nu pune DoT in functie de distanta ci pune uniform la toti din raza
					SetActorDoT(act, explotemplate->cDoT.eType, explotemplate->cDoT.fDuration, explotemplate->cDoT.fDamagePerSec, explotemplate->cDoT.eExcludedActClass, explotemplate->cDoT.eFilteredActClass, dwOwnerUID);
				}

				//evit friendly stun
				if (act->actTemplate.actorClass != K_LVL_ACT_CLASS_HUMAN)
					continue;
				//daca e prea departe nu il ia in seama
				if (fDist > fStunRadius)
					continue;
				//daca stun este directional si nu se potriveste directia
				if ((vExploDir.x != 0.0f) && (SIGN(vExploDir.x) != SIGN(vDir.x)))
					continue;
				//daca nu e linie directa nu loveste
				if (!bDirectLine)
					continue;
				//daca il vede ii da stun
				if ((act->fStunTimer < fMaxStun) && (fMaxStun > 0.0f))
				{
					SetActorStun(act, fMaxStun);
					//let him know he got stunned
					AddAIEvent(K_LVL_AI_EVENT_GOT_HIT, 0, exploOwnerClass, pos, fStunRadius, fMaxStun + 0.5f, act->GetUID());
				}
			}
		}

		//do some damage
		if ((fMaxDamage > 0.0f) && (fDamageRadius > 0.0f))
		{
			int nBombFrags = 0;
			//find all actors and damage them (linearly)
			for (int kk = 0; kk < m_arrActors.GetSize(); kk++)
			{
				CActor* act = m_arrActors[kk];
				//sar actorii ascunsi
				if ((act->bHidden) || (act->actTemplate.eCaps & CActorTemplate::K_ACT_CAPS_NOT_A_TARGET))
					continue;
				//ignores specified classes
				if (act->actTemplate.actorClass == explotemplate->eIgnoreActorClass)
					continue;
				//distanta la inamic
				Vec2 vDir = act->GetPosHeart() - pos;
				float fDist = D3DXVec2Length(&vDir);
				//daca e prea departe nu il ia in seama
				if (fDist > fDamageRadius)
					continue;
				//daca nu e linie directa nu loveste
				if (!IsLineOfSight(act->GetPosHeart(), pos))
					continue;
				//loveste liniar
				float fPercent = 1.0f - (fDist / fDamageRadius);
				CLAMP(fPercent, 0.0f, 1.0f);
				//add momentum
				D3DXVec2Normalize(&vDir, &vDir);
				vDir *= fPercent * fMaxImpulse;
				//#HACK: ca sa nu mai arunce cadavrele in sus
				if (vDir.y < 0.0f)
					vDir.y = 0.0f;

				CBulletHitReturnData retdata;
				retdata = HitActor(act, fPercent * fMaxDamage, dwOwnerUID, K_LVL_ACT_CLASS_EXPLOSION, &vDir, K_LVL_BULLET_FLAG_CAN_SPLAT, explotemplate->nArmorPiercingRating);
				//count only enemies
				if ((retdata.bKilledTarget) && (act->actTemplate.actorClass >= K_LVL_ACT_CLASS_HUMAN))
					nBombFrags++;

				//#ACHIEVEMENTS: darwin award - player died from his own explosive
				if ((act->actTemplate.actorClass == K_LVL_ACT_CLASS_PLAYER) && (!IsNetworkPlayer(act)) && (act->fLife <= 0.0f) && (dwOwnerUID == act->UID) &&
					((explotemplate->name.textHash == hash_EXPLO_GRENADE_GROUND) || (explotemplate->name.textHash == hash_EXPLO_GRENADE) ||
					(explotemplate->name.textHash == hash_EXPLO_CHARGE) || (explotemplate->name.textHash == hash_EXPLO_CHARGE_INVISIBLE)))
				{
					UTGetAchievementManager().UnlockAchievement(ACH_DARWIN_AWARD);
				}

			}

			//#ACHIEVEMENTS: explosion achievements
			if ((nBombFrags >= 3) && (pPlayer != null) && (!IsNetworkPlayer(pPlayer)))
			{
				//breaching charge behind the door
				if (explotemplate->name.textHash == hash_EXPLO_CHARGE_INVISIBLE)
				{
					UTGetAchievementManager().UnlockAchievement(ACH_GOOD_BREACH);
				}
				if (explotemplate->name.textHash == hash_EXPLO_BARREL)
				{
					UTGetAchievementManager().UnlockAchievement(ACH_HEAT_UP_THE_NIGHT);
				}
			}

			///--- check doors and windows breaking ---
			if ((fMaxDamage > 0.0f) && (fDamageRadius > 0.0f))
			{
				for (int kk = 0; kk < m_visibleList.logic_colShapesSpecial.Count(); kk++)
				{
					CCollisionShape* shape = m_visibleList.logic_colShapesSpecial.m_pData[kk];
					//breaks doors?
					if ((explotemplate->fDamageObjectsMultiplier > 0.0f) && (shape->AIstate == K_AI_STATE_COLL_BREAKABLE_DOOR))
					{
						if (shape->varAIparams.GetVariantByName(L"b_reinforced")->m_asINT32 != 0)
							continue;

						//loveste liniar
						Vec2 vDist = (shape->bbox.vCenter - pos);
						float fDist = D3DXVec2Length(&vDist);
						if (explotemplate->fDamageObjectsMultiplier <= 0.0f)
							continue;
						float fPercent = 1.0f - (fDist / (fDamageRadius * explotemplate->fDamageObjectsMultiplier));
						//too soft
						if (fPercent <= 0.0f)
							continue;
						//not straight line (can't check with center or it will fail because of the actual bbox)
						Vec2 vCheckPt = pos;
						vCheckPt.x -= (shape->bbox.vHalfSize.x + 2.0f) * SIGN(vDist.x);
						if (!IsLineOfSight(pos, vCheckPt))
							continue;
						//damage door
						shape->AIfvar1 -= (fPercent * fMaxDamage) * explotemplate->fDamageObjectsMultiplier;
						//was hit
						shape->AIvarBool1 = true;
						//save door explo direction
						shape->varAIparams.SetNamedVarFloat(L"fForceDirX", SIGN(vDist.x));
						shape->varAIparams.SetNamedVarINT32(L"bExploded", 1);
					}

					//windows?
					if (shape->AIstate == K_AI_STATE_COLL_BREAKABLE_WINDOW)
					{
						//linear distance hit
						float fDist = D3DXVec2Length(&(shape->bbox.vCenter - pos));
						float fPercent = 1.0f - (fDist / fDamageRadius);
						//subtract life
						if (fPercent > 0.0f)
						{
							shape->AIfvar1 -= fPercent * fMaxDamage;
							//door destroyed - save direction applied by explo
							if (shape->AIfvar1 <= 0.0f)
							{
								shape->varAIparams.SetNamedVarFloat(L"fForceDirX", 1000.0f * SIGN(shape->bbox.vCenter.x - pos.x));
							}
						}
					}
				}
			}
			//check grenade interaction AIs
			/*
			if ((fMaxDamage > 0.0f) && (bInteractAI) && (fDamageRadius > 0.0f))
			{
				for (int kk = 0; kk < m_visibleList.logic_props_closeby.Count(); kk++)
				{
					CProp * activ = m_visibleList.logic_props_closeby.m_pData[kk];
					if (activ->AIstate == K_AI_STATE_ACTIVE_SWINGING_FRONTOBJ)
					{
						//daca am activ swinging si e in raza grenadei
						Vec2 vDir = activ->pos - pos;
						float fDist = D3DXVec2Length(&vDir);
						//daca e prea departe nu il ia in seama
						if (fDist > fDamageRadius * 2.0f)
							continue;
						//direct line of sight
						if (!IsLineOfSight(activ->pos, pos))
							continue;

						//setam balans
						float maxperc = 1.0f - (fDist / (fDamageRadius * 2.0f));
						//viteza unghiulara
						activ->AIfvar1 = -SIGN(vDir.x) * 8.0f * maxperc;
					}
				}
			}
			*/
		}

		//damage over time
	}
}



void CLevel::UpdateDoofers(float dTime)
{
	RECTXYWH_F camrect = m_camLevel.GetCamWorldAABB();
	RECTXYWH_F camrect_larger = camrect;
	camrect_larger.Inflate(2.0f * K_TILE_SIZE);

	CLinkedPool<CDoofer>::CLinkedPoolNode *node = m_poolDoofers.pListUsed.m_pNext;
	while (node != &m_poolDoofers.pListUsed)
	{
		//salvez locatia urmatoare ca s apot avansa pe ea
		CLinkedPool<CDoofer>::CLinkedPoolNode *nextnode = node->m_pNext;
		CDoofer* prop = &node->m_data;

		bool killprop = false;
		//daca iese din zona de joc
		if (prop->physPt->m_data.bIsDead)
			killprop = true;

		//generic updates
		prop->fLightTimer += dTime;

		switch (prop->type)
		{
			case K_DOOFER_SHELL:
			{
				if ((prop->bVar1 == false) && (prop->physPt->m_data.bContactStarted))
				{
					prop->bVar1 = true;
					/*
					if(prop->nSubType == 0) //shotgun shell
						SND_PLAY_POSITIONAL(SNDIDX_RIFLE_SHELL_DROP, prop->physPt->m_data.pos);
					else
						SND_PLAY_POSITIONAL(SNDIDX_SHOTGUN_SHELL_DROP, prop->physPt->m_data.pos);
						*/
				}

				if (prop->physPt->m_data.bIsStatic)
				{
					//put shell as decal
					/*
					if (randint(1000) < 200)
					{
						//daca collisionul are AI inseamna ca e lift sau ceva deci nu lasam sange
						if ((prop->physPt->m_data.pContactShape != null) && (prop->physPt->m_data.pContactShape->AIstate == K_AI_STATE_UNDEFINED))
							AddDecal(K_LVL_DECAL_LAYER_BACKOBJECTS, prop->physPt->m_data.pos, prop->spr.animationIdx, prop->spr.currentFrame, prop->spr.color);
					}
					*/
					killprop = true;
				}
				else if (!PointInRect(Vec3ProjVec2(prop->physPt->m_data.pos), camrect))
				{
					killprop = true;
				}
			}
			break;
			case K_DOOFER_FIRE_SOURCE:
			{
				/*
				if (!prop->physPt->m_data.bContacting)
				{
					if (m_Timers.Tick(50))
					{
						g_particlesMgr.GenerateFireRing(prop->physPt->m_data.pos, 5, 10.0f, 15.0f, K_PART_LAYER_RT_FRONT_NRM);
					}
				}
				//genereaza particula de lava
				if ((prop->physPt->m_data.bContactStarted) && (prop->physPt->m_data.contactNormal.y < -0.5f))
				{
					int anmidx = ANM_PARTICLES_SPR_MELTING_LAVA1;
					if (randint(100) < 60)
						anmidx = ANM_PARTICLES_SPR_MELTING_LAVA2;
					g_particlesMgr.AddParticle(anmidx, true, 0, &prop->physPt->m_data.contactPos, NULL, NULL, 4.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_BACK_NRM);
				}
				//genereaza damage pana inainte de ultima secunda
				if ((m_Timers.Tick(500.0f)) && (prop->physPt->m_data.bIsStatic) && (prop->fTimer > 1.0f))
				{
					CWeaponTemplate* wtempl = GetTemplateWeapon(L"WPN_LAVA_MELEE");
					Vec2 normal(0.0f, -1.0f);
					ShootBullet(&wtempl->bulletTemplate, K_LVL_ACT_CLASS_TRAP, 0, prop->physPt->m_data.pos + normal, normal);
				}
				*/

				prop->fTimer -= dTime;
				if (prop->fTimer <= 0.0f)
				{
					killprop = true;
				}
			}
			break;

			case K_DOOFER_MEAT:
			{
				DWORD dwCol = 0xff671010;
				if (prop->nSubType != 0) //zombies green blood
					dwCol = 0xff82b600;
				/*
				if (m_Timers.Tick(50))
				{
					g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_BLOOD, false, randint(5), &Vec2(prop->physPt->m_data.pos.x + randfloatsgn(5.0f), prop->physPt->m_data.pos.y + randfloatsgn(5.0f)),
						&Vec2(0.0f, 20.0f), &(prop->physPt->m_data.speed / (5.0f + randfloat(4.0f))), 0.6f, 1.0f, 0.0f, randfloat(PI), randfloatsgn(2.0f), 0.1f, 0.1f, dwCol, K_PART_LAYER_RT_BACK_NRM);
				}
				*/
				//only stain at high velocities
				if ((prop->physPt->m_data.bContactStarted) && (prop->physPt->m_data.contactNormal.y < 0.0f))
				{
					//don't stain moving platforms
					/*
					if ((prop->physPt->m_data.pContactShape != null) && (prop->physPt->m_data.pContactShape->AIstate == K_AI_STATE_UNDEFINED))
					{
						if(prop->nSubType == 0)
							AddDecal(K_LVL_DECAL_LAYER_BACKOBJECTS, prop->physPt->m_data.pos, ANM_ACTIVES_SPR_DECAL_BLOOD_FRONTLAYER, randint(3), 0xffffffff);
						else
							AddDecal(K_LVL_DECAL_LAYER_BACKOBJECTS, prop->physPt->m_data.pos, ANM_ACTIVES_SPR_DECAL_BLOOD_FRONTLAYER_GREEN, randint(3), 0xffffffff);

//						SND_PLAY_POSITIONAL_RAND2(SNDIDX_GIBLET1, SNDIDX_GIBLET2, prop->physPt->m_data.pos);
					}
					*/
				}

				if ((prop->physPt->m_data.bIsStatic) || (!PointInRect(Vec3XY(prop->physPt->m_data.pos), camrect_larger)))
				{
					killprop = true;
				}
			}
			break;

			case K_DOOFER_SHRAPNEL_SMOKING:
			{
				//update sprite
				node->m_data.spr.Update(&m_sprProps, dTime);
				//add smoke
				if ((m_Timers.Tick(60)) && (!prop->physPt->m_data.bContacting))
				{
					float fAng = randfloat(DOUBLE_PI);
					Vec2 vDir(sin(fAng), cos(fAng));
					g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_PUFF_XS1 + randint(3), true, 0, &Vec2(prop->physPt->m_data.pos.x + randfloatsgn(2.0f), prop->physPt->m_data.pos.y + randfloatsgn(2.0f)),
						NULL, &(vDir * (5.0f + randfloat(5.0f))), 1.0f, 1.0f, 0.0f, fAng, 0.0f, 0.0f, 0.0f, 0xaaffffff, K_PART_LAYER_RT_FRONT_NRM);
				}
				//genereaza particule de foc doar cat e roshu
				/*
				if (node->m_data.fTimer > 0.0f)
				{
					node->m_data.fTimer -= dTime;
					if ((m_Timers.Tick(80)) && (!prop->physPt->m_data.bContacting))
					{
						g_particlesMgr.GenerateFireRing(prop->physPt->m_data.pos, 2, 8.0f, 10.0f, K_PART_LAYER_RT_FRONT_NRM);
					}
					//some secondary explosions too
					if ((m_Timers.Tick(120)) && (randompercent(50.0f)))
					{
						g_particlesMgr.AddParticle(ANM_PARTICLES_SPR_FIRECRACKER1 + randint(2), true, 0, &prop->physPt->m_data.pos,
							NULL, NULL, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, K_PART_LAYER_RT_FRONT_NRM);
					}
				}

				if ((prop->physPt->m_data.bIsStatic) || (!PointInRect(prop->physPt->m_data.pos, camrect_larger)))
				{
					killprop = true;
					//smoke puff when dead
					int nAnmId = ANM_PARTICLES_SPR_PUFF_S_XS;
					if (randompercent(50.0f))
						nAnmId = ANM_PARTICLES_SPR_PUFF_S_XXS;

					g_particlesMgr.AddParticle(nAnmId, true, 0, &prop->physPt->m_data.pos,
						NULL, NULL, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xaaffffff, K_PART_LAYER_RT_FRONT_NRM);
				}
				*/
			}
			break;
			case K_DOOFER_LIGHT:
			{
				prop->fTimer += dTime;
				//kill on timing out
				if (prop->fTimer >= prop->fLightDuration)
					killprop = true;
			}
			break;
			case K_DOOFER_EXPLOSION:
			{
				prop->fTimer -= dTime;
				if (prop->fTimer <= 0.0f)
					killprop = true;
			}
			break;
		}
		//ii dam release
		if (killprop)
		{
			//release la nodul de fizica !!!
			m_poolPhysPts.DismissNode(prop->physPt);
			//si eliberez glontul
			m_poolDoofers.DismissNode(node);
		}
		//avansez pointer
		node = nextnode;
	}
}

void CLevel::PaintDoofers()
{
	m_pSprite->SetTransform(&g_matIdentity);
	D3DXMATRIXA16 mattrans;

	CLinkedPool<CDoofer>::CLinkedPoolNode *node = m_poolDoofers.pListUsed.m_pNext;
	while (node != &m_poolDoofers.pListUsed)
	{
		//salvez locatia urmatoare ca sa pot avansa pe ea
		CLinkedPool<CDoofer>::CLinkedPoolNode *nextnode = node->m_pNext;

		switch (node->m_data.type)
		{
			case K_DOOFER_FIRE_SOURCE:
			{
				if (!node->m_data.physPt->m_data.bContacting)
				{
					/*
					Vec2 ppos = node->m_data.physPt->m_data.pos;
					float falpha = LIMIT(node->m_data.fTimer, 0.0f, 1.0f);
					CSprite::paintFrameModule(&m_sprProps, ppos.x, ppos.y, ANM_ACTIVES_SPR_BULLETS_FIRE, 0, 0, D3DCOLOR_FFFA(falpha));
					*/
				}
			}
			break;
			case K_DOOFER_SHELL:
			{
				/*
				node->m_data.spr.pos = node->m_data.physPt->m_data.pos;
				node->m_data.spr.currentFrame = node->m_data.nSubType * 4 + (int(node->m_data.spr.pos.x * 3.0f) % 4);
				node->m_data.spr.paint_firstModule(&m_sprProps);
				*/
			}
			break;
			case K_DOOFER_SHRAPNEL_SMOKING:
			{
				/*
				node->m_data.spr.pos = node->m_data.physPt->m_data.pos;

				node->m_data.spr2.pos = node->m_data.physPt->m_data.pos;
				node->m_data.spr2.color = D3DCOLOR_FFFA(LIMIT(node->m_data.fTimer, 0.0f, 1.0f));

				node->m_data.spr.paint_firstModule(&m_sprProps);
				node->m_data.spr2.paint_firstModule(&m_sprProps);
				*/
			}
			break;
			case K_DOOFER_MEAT:
			{
				/*
				node->m_data.spr.pos = node->m_data.physPt->m_data.pos;
				node->m_data.spr.paint_firstModule(&m_sprProps);
				*/
			}
			break;
		}

		//avansez pointer
		node = nextnode;
	}
}

