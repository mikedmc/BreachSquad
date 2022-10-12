#include "dxstdafx.h"
#include "particlesmanager.h"

CParticle::CParticle()
{
	pNext = nullptr;
	pPrev = nullptr;   
	//default values
	m_fSize		=	1.0f;
	m_fScaleSpeed = 0.0f;
	m_vPos = Vec2(0.0f, 0.0f);
	m_vSpeed = Vec2(0.0f, 0.0f);
	m_fRotAngle = 0.0f;
	m_fRotSpeed = 0.0f;
	m_Color = 0xffffffff;
	m_fLifetime = 2.0f;
	m_fFadeOut_Duration = 0.0f;
	m_fFadeIn_Duration = 0.0f;
	m_vGravity = Vec2(0.0f, 0.0f);
	m_fSize = 1.0f;
	m_fScaleSpeed = 0.0f;
	bAnimated = false;
	m_fLife = 0.0f;
	m_fAlpha = 1.0f;
	m_fAirFriction = 0.0f;
	m_fWaitTimer = 0.0f;
}


CParticlesManager::CParticlesManager()
{
	bInitialized = false;
	nParticlesCnt = 0;
	pParticles = nullptr;
	//empty all lists
	pListFree.pNext = pListFree.pPrev = &pListFree;	//indica spre ele insele
	for (auto & layer : pList)
	{
		layer.pNext = layer.pPrev = &layer;
	}

	fLocalTimeline = 0.0f;
	m_pDevice = nullptr;
}

CParticlesManager::~CParticlesManager()
{
	Release();
}

OPRESULT CParticlesManager::Init(WCHAR* XMLpath, int nMaxParticlesCnt)
{
	//load sprites
	V_OP_RET(m_sprCol.LoadSprites(XMLpath));
	//allocate particles
	nParticlesCnt = nMaxParticlesCnt;
	if(nMaxParticlesCnt < 100)  
		nParticlesCnt = 100;
	pParticles = new CParticle[nParticlesCnt];
	if(pParticles == nullptr)
	{
		ErrorBox(K_ERR_CRITICAL, L"CParticlesManager::Init failed! Out of memory!");
		m_sprCol.Release();
		return K_OP_FAILED;
	}
	// place all particles in "free" list
	pListFree.pNext = &pParticles[0];
	pListFree.pPrev = &pParticles[nParticlesCnt - 1];
	// close particles ring list
	pParticles[0].pNext = &pParticles[1];
	pParticles[0].pPrev = &pListFree;
	pParticles[nParticlesCnt - 1].pPrev = &pParticles[nParticlesCnt - 2];
	pParticles[nParticlesCnt - 1].pNext = &pListFree;
	// set all others
	for (int kk = 1; kk < nParticlesCnt - 1; kk++)
	{
		pParticles[kk].pNext = &pParticles[kk + 1];
		pParticles[kk].pPrev = &pParticles[kk - 1];
	}
	// empty all other layers
	for (auto & kk : pList)
	{
		kk.pNext = kk.pPrev = &kk;
	}


	LOG(L"CParticlesManager::Initialized [%s] maxParticlesCnt:%d", XMLpath, nMaxParticlesCnt);
	bInitialized = true;

	return K_OP_OK;
}

void CParticlesManager::Release()
{
	if (bInitialized)
	{
		LOG(L"CParticlesManager::Released");
	}

	//deallocate particles
	pListFree.pNext = pListFree.pPrev = &pListFree;	
	for (auto & kk : pList)
	{
		kk.pNext = kk.pPrev = &kk;
	}

	SAFE_DELETE_ARRAY(pParticles);
	m_sprCol.Release();

	//release particle emitters
	ReleaseAllPartEmitters();

	bInitialized = false;
}

void CParticlesManager::Update(float dTime)
{
	fLocalTimeline += dTime;
	// follows all rings in all layers and updates the particles
	for (int oo = 0; oo < K_PART_LAYERS_CNT; oo++)
	{
		UpdateLayer((EParticleLayer)oo, dTime);
	}
}

void CParticlesManager::UpdateLayer(EParticleLayer eLayer, float dtime)
{
	CParticle *part = pList[eLayer].pNext;
	while (part != &pList[eLayer])
	{
		if (part->m_fWaitTimer > 0.0f)
		{
			part->m_fWaitTimer -= dtime;
		}
		else
		{
			if (part->bAnimated)
				part->sprite.Update(dtime);
			//apply acceleration
			part->m_vSpeed += part->m_vGravity * dtime;
			//apply air friction
			part->m_vSpeed -= part->m_fAirFriction * part->m_vSpeed * dtime;
			//integrate position
			part->m_vPos += part->m_vSpeed * dtime;
			part->m_fRotAngle += part->m_fRotSpeed * dtime;
			part->m_fSize += part->m_fScaleSpeed * dtime;
			//fade in 
			float newalpha = part->m_fAlpha;
			if (part->m_fLife < part->m_fFadeIn_Duration)
			{
				newalpha *= part->m_fLife / part->m_fFadeIn_Duration;
			}
			//fade out
			if (part->m_fFadeOut_Duration > 0.0f)
			{
				float lfdiff = part->m_fLifetime - part->m_fLife;
				if (lfdiff < part->m_fFadeOut_Duration)
				{
					newalpha *= lfdiff / part->m_fFadeOut_Duration;
				}
			}
			// write data into sprite for rendering
			part->sprite.color = (part->m_Color & 0x00ffffff) | ((BYTE(newalpha * 255)) << 24);
			part->sprite.rotation = part->m_fRotAngle;
			part->sprite.scale = { part->m_fSize, part->m_fSize };
			part->sprite.pos = part->m_vPos;

			part->m_fLife += dtime;
		}
		if ((part->m_fLife >= part->m_fLifetime) || (part->sprite.animStatus == ANIM_STATUS_FRAMELOCK))
		{
			CParticle* nextp = part->pNext;
			// link neighbours between them
			part->pNext->pPrev = part->pPrev;
			part->pPrev->pNext = part->pNext;
			// move particle to free particles list
			pListFree.pNext->pPrev = part;
			part->pNext = pListFree.pNext;
			part->pPrev = &pListFree;
			pListFree.pNext = part;
			// move pointer
			part = nextp;
		}
		else
		{
			// move ponter
			part = part->pNext;
		}
	}
}

void CParticlesManager::PaintLayer(EParticleLayer eLayer, bool additiveBlending)
{
	_ASSERT((eLayer >= 0) && (eLayer < K_PART_LAYERS_CNT));
	Mat mattrans;

	// empty layer, early exit
	if(pList[eLayer].pNext == &pList[eLayer])
		return;

	if(additiveBlending)
		UT3D::DeviceAdditiveON(m_pDevice);

	CParticle *part = pList[eLayer].pNext;
	while(part != &pList[eLayer])
	{
		if (part->m_fWaitTimer <= 0.0f)
		{
			part->sprite.PaintFModule(0);
		}
		part = part->pNext;
	}

	if(additiveBlending)
		UT3D::DeviceAdditiveOFF(m_pDevice);
}

void CParticlesManager::RemoveAllFromLayer(EParticleLayer ePartLayer)
{
	if ((ePartLayer < 0) || (ePartLayer >= EParticleLayer::K_PART_LAYERS_CNT))
		return;

	CParticle *part = pList[ePartLayer].pNext;
	while (part != &pList[ePartLayer])
	{
		CParticle* nextp = part->pNext;
		// link neighbours between them
		part->pNext->pPrev = part->pPrev;
		part->pPrev->pNext = part->pNext;
		// put particle in free list
		pListFree.pNext->pPrev = part;
		part->pNext = pListFree.pNext;
		part->pPrev = &pListFree;
		pListFree.pNext = part;
		// move pointer to the next
		part = nextp;
	}
}

void CParticlesManager::ClearParticles()
{
	// empty all lists
	for (int kk = 0; kk < K_PART_LAYERS_CNT; kk++)
	{
		pList[kk].pNext = pList[kk].pPrev = &pList[kk];
	}
	// place all particles in free list
	pListFree.pNext = &pParticles[0];
	pListFree.pPrev = &pParticles[nParticlesCnt - 1];
	if(pParticles)
	{
		//set first and last particles
		pParticles[0].pNext = &pParticles[1];
		pParticles[0].pPrev = &pListFree;
		pParticles[nParticlesCnt - 1].pPrev = &pParticles[nParticlesCnt - 2];
		pParticles[nParticlesCnt - 1].pNext = &pListFree;
		//set all others
		for (int kk = 1; kk < nParticlesCnt - 1; kk++)
		{
			pParticles[kk].pNext = &pParticles[kk + 1];
			pParticles[kk].pPrev = &pParticles[kk - 1];
		}
	}
	//release particle emitters
	ReleaseAllPartEmitters();
}

int CParticlesManager::GetParticleLayerByName(WCHAR * layerName)
{
	UINT32 layerNameHash = FastHash(layerName);
	for (int kk = 0; kk < K_PART_LAYERS_CNT; kk++)
	{
		if (layerNameHash == EParticleLayer_names[kk].textHash)
			return kk;
	}

	ErrorBox(K_ERR_WARNING, L"GetParticleLayerByName::Unknown layer name!");
	return -1;
}
int CParticlesManager::GetParticleLayerByName(UINT32 layerNameHash)
{
	for (int kk = 0; kk < K_PART_LAYERS_CNT; kk++)
	{
		if (layerNameHash == EParticleLayer_names[kk].textHash)
			return kk;
	}

	ErrorBox(K_ERR_WARNING, L"GetParticleLayerByName::Unknown layer hash!");
	return -1;
}


void CParticlesManager::AddParticle(int animID, bool animated, int currentFrame, Vec2* pos, 
					Vec2* gravity, Vec2* speed, 
					float lifetime, 
					float size, float scalespeed, 
					float rotangle, float rotspeed, 
					float fadeInDuration,
					float fadeOutDuration,
					DWORD nColor,
					int nLayer,
					float airFriction,
					float waitTimer)
{
	_ASSERT((nLayer >= 0) && (nLayer < K_PART_LAYERS_CNT));
	CParticle* newp = pListFree.pNext;
	if(newp == &pListFree)
	{
#if defined(_DEBUG) || defined(DEBUG)
		ErrorBox(K_ERR_WARNING, L"CParticlesMgr::AddParticle could not add particle. No more free particles!");
#endif
		return;
	}

	pListFree.pNext = pListFree.pNext->pNext;
	//add particle to the end of requested layer
	pList[nLayer].pPrev->pNext = newp;
	newp->pPrev = pList[nLayer].pPrev;
	pList[nLayer].pPrev = newp;
	newp->pNext = &pList[nLayer]; 
	// init particle sprite
	newp->sprite.Init(&m_sprCol, animID);
	newp->sprite.frameIdx = currentFrame;
	//set particle properties
	newp->bAnimated = animated;
	newp->m_fAirFriction = airFriction;
	newp->m_vPos = ( pos != nullptr ) ? *pos : g_Vec2Zero;
	newp->m_vGravity = ( gravity != nullptr ) ? *gravity : g_Vec2Zero;
	newp->m_vSpeed = (speed != nullptr)? *speed : g_Vec2Zero;
	newp->m_fLifetime = lifetime;
	newp->m_fLife = 0.0f;
	newp->m_fScaleSpeed = scalespeed;
	newp->m_fSize = size;
	newp->m_fRotSpeed = rotspeed;
	newp->m_fRotAngle = rotangle;
	newp->m_fFadeOut_Duration = fadeOutDuration;
	newp->m_fFadeIn_Duration= fadeInDuration;

	newp->m_Color = nColor;
	newp->m_fAlpha = ((float)((newp->m_Color & 0xff000000)>>24))/255.0f;
	if(newp->m_fLife < newp->m_fFadeIn_Duration)
	{
		float newAlpha = newp->m_fAlpha * (newp->m_fLife / newp->m_fFadeIn_Duration);
		newp->sprite.color = (newp->m_Color & 0x00ffffff) | ((BYTE(newAlpha * 255))<<24);
	}

	newp->m_fWaitTimer = waitTimer;
}

//-=-=-= PARTICLE EMITTERS =-=-=-
int CParticlesManager::GetPartEmitterTypeByNameHash(UINT32 generatorNameHash)
{
	for (int kk = 0; kk < K_PART_PE_TYPES_CNT; kk++)
	{
		if (generatorNameHash == ParticleEmitter_names[kk].textHash)
			return kk;
	}

	ErrorBox(K_ERR_WARNING, L"GetPartEmitterByNameHash::Unknown generator name!");
	return K_PART_PE_TYPE_UNKNOWN;
}

CParticleEmitter* CParticlesManager::AddPartEmitter(int nType, CAABB * pe_aabb, int nParticleLayer)
{
	if (nType == K_PART_PE_TYPE_UNKNOWN)
	{
		ErrorBox(K_ERR_WARNING, L"Particle Emitter type unknown!");
		return nullptr;
	}

	CParticleEmitter* npe = new CParticleEmitter();

	npe->type = nType;
	npe->particleLayer = nParticleLayer;
	npe->bbox = *pe_aabb;
	npe->bbox_surface = npe->bbox.vSize.x * npe->bbox.vSize.y;
	npe->bGenerateOutsideScreen = true;
	switch (nType)
	{
		case K_PART_PE_TYPE_FOG:
		{
			npe->densityPerSurfaceUnitPerSec = 1.0f;
		}
		break;
		case K_PART_PE_TYPE_FIRE:
		{
			npe->densityPerSurfaceUnitPerSec = 50.0f;
		}
		break;
		case K_PART_PE_TYPE_FLARE:
		{
			npe->densityPerSurfaceUnitPerSec = 20.0f;
		}
		break;
		case K_PART_PE_TYPE_RAINDROPS:
		{
			npe->densityPerSurfaceUnitPerSec = 30.0f;
			npe->bGenerateOutsideScreen = false;
		}
		break;
		default:
		{
			ErrorBox(K_ERR_WARNING, L"Particle Emitter type unknown! Shouldn't get here!");
		}
		break;
	}
	//finish setting up
	npe->fGenerateTime = 1.0f / (npe->densityPerSurfaceUnitPerSec * (npe->bbox_surface / K_PART_PE_SURFACE_UNIT));
	npe->fTimer = 0.0f;

	m_arrPartEmitters.Add(npe);
	return npe;
}

void CParticlesManager::ReleasePartEmitter(CParticleEmitter* pEmit)
{
	int idx = m_arrPartEmitters.IndexOf(pEmit);
	if (idx >= 0)
		m_arrPartEmitters.Remove(idx);
}

void CParticlesManager::ReleaseAllPartEmitters()
{
	SAFE_DELETE_GROWABLE_ARRAY(m_arrPartEmitters);
}

void CParticlesManager::UpdatePartEmitters(float dTime, RectXYWH screenRect)
{
	for (int kk = 0; kk < m_arrPartEmitters.GetSize(); kk++)
	{
		CParticleEmitter * npe = m_arrPartEmitters[kk];
		//este pe pauza
		if (npe->bPauseUpdate)
			continue;
		//generic updates
		bool generatePart = false;

		npe->fTimer += dTime;
		if (npe->fTimer >= npe->fGenerateTime)
		{
			generatePart = true;
			npe->fTimer -= npe->fGenerateTime;
		}
		//nu generez particule in afara ecranului
		Vec2 gpos(0.0f, 0.0f);
		if (generatePart)
		{
			gpos = AABB::GetRandomPointInBox(npe->bbox);
			if ((!npe->bGenerateOutsideScreen) && (!Rects::PointInRect(gpos, screenRect)))
				generatePart = false;
		}

		if (generatePart)
		{
			switch (npe->type)
			{
				case K_PART_PE_TYPE_FOG:
				{
					//AddParticle(ANM_PARTICLES_SPR_SMOKE, false, randint(3), &gpos, nullptr, &Vec2(randfloatsgn(8.0f), 0.0f), 6.0f, 0.7f, 0.2f, randfloat(PI), 0.0f, 1.0f, 2.0f, 0x22ffffff, npe->particleLayer);
				}
				break;
				case K_PART_PE_TYPE_FIRE:
				{
					//AddParticle(ANM_PARTICLES_SPR_FLAME_SM, false, randint(5), &gpos, &Vec2(0.0f, -100.0f), &Vec2(randfloatsgn(1.0f), randfloatsgn(1.0f)), 1.0f, 1.0f, -0.4f, 0.0f, 0.0f, 0.1f, 0.5f, 0xffffffff, npe->particleLayer);
				}
				break;
				case K_PART_PE_TYPE_FLARE:
				{
					//AddParticle(ANM_PARTICLES_SPR_SMOKE, false, randint(3), &gpos, &Vec2(0.0f, -100.0f), &Vec2(randfloatsgn(50.0f), -randfloat(40.0f)), 2.0f + randfloat(1.0f), 0.5f, 0.2f, randfloat(DOUBLE_PI), randfloatsgn(1.0f), 0.1f, 0.5f, 0x66ff8888, npe->particleLayer, 5.0f);
				}
				break;
				case K_PART_PE_TYPE_RAINDROPS:
				{
					//AddParticle(ANM_PARTICLES_SPR_RAINDROP1, true, 0, &gpos, nullptr, nullptr, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0xffffffff, npe->particleLayer);
				}
				break;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
OPRESULT CParticlesManager::OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc)
{
	m_pDevice = pDevice;
	return m_sprCol.OnCreateDevice(pDevice);
}

OPRESULT CParticlesManager::OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc) 
{
	m_pDevice = pDevice;
	return m_sprCol.OnResetDevice(pDevice);
}

OPRESULT CParticlesManager::OnLostDevice()
{
	m_pDevice = nullptr;
	return m_sprCol.OnLostDevice();
}

OPRESULT CParticlesManager::OnDestroyDevice()
{
	m_pDevice = nullptr;
	return m_sprCol.OnDestroyDevice();
}


///**************************************************************************************
/// SINGLETON
///**************************************************************************************
CParticlesManager& __Particles()
{
	static CParticlesManager g_particlesMgr;
	return g_particlesMgr;
}
