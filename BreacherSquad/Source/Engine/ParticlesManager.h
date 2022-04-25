#pragma once
#include "sprite/Spr.h"
#include "interfaces/DeviceRes.h"

class CParticle {
public:
	float			m_fLife;					
	float			m_fAlpha;
	//pointers to neighbours in list
	CParticle*		pNext;
	CParticle*		pPrev;   
public:
	Vec2			m_vPos;
	Vec2			m_vSpeed;		  
	float			m_fRotAngle;	  
	float			m_fRotSpeed;	  
	DWORD			m_Color;	
	float			m_fWaitTimer;		// will be spawned later if we want to spawn them in a row
	float			m_fLifetime;		// original life
	float			m_fFadeOut_Duration; // time to fade in
	float			m_fFadeIn_Duration;	// time to fade out (ends when life ends)
	Vec2			m_vGravity;			
	float			m_fSize;			
	float			m_fScaleSpeed;		// scaling speed
	bool			bAnimated;			// updates sprite? If so then it dies when animation ends
	float			m_fAirFriction;		// Air friction coefficient
	
	CSpr			sprite;

	CParticle();
};

///--- emitoare de particule ---
const CStringHash ParticleEmitter_names[] = {
	L"FOG",
	L"FIRE",
	L"FLARE",
	L"RAINDROPS",
};

enum ParticleEmitters {
	K_PART_PE_TYPE_UNKNOWN = -1,

	K_PART_PE_TYPE_FOG = 0,
	K_PART_PE_TYPE_FIRE = 1,
	K_PART_PE_TYPE_FLARE,
	K_PART_PE_TYPE_RAINDROPS,

	K_PART_PE_TYPES_CNT
};

//unitatea de suprafata pt emitoare. In cazul nostru este suprafata unui tile.
#define K_PART_PE_SURFACE_UNIT	(16.0f * 16.0f)


/*
//forma in care emite
#define K_PART_PEB_SHAPE_RECT 0
#define K_PART_PEB_SHAPE_ELLIPSE 1
#define K_PART_PEB_SHAPE_POINT 2
//clasa CPEBrush defineste tipul si proprietatile particulelor generate. Un Emitter poate avea mai multe brushes
class CParticleEmitterBrush {
public:
	//zona in care emite (forma si dimensiunile ei)
	int				m_spawnShape;	  //de tipul K_PART_PEB_SHAPE_
	RECTXYWH		m_spawnSize;	  //aabb-ul zonei in care emite
	//durata emisiei
	float			m_spawnFreq;	  //la ce perioada emite (rezolutia array-ului de timere); 0.0f - emite doar o data
	float			m_spawnStartTime, m_spawnEndTime; //-1 -> emite mereu

	//TODO: aici se vor specifica domeniile pe care pot varia toti parametrii particulelor. Daca vrei sa ii corelezi e mai trist. De exemplu marimea cu viteza
	Vec2		m_vPos;
	Vec2		m_vSpeed;		  //directia particulei (modificabila de update)
	float			m_fRotAngle;	  //unghiul de rotatie
	float			m_fRotSpeed;	  // viteza de rotatie
	DWORD			m_Color;
	float			m_waitTimer;		// cat asteapta intainte sa faca update
	float			m_fLifetime;	  //cat traieste particula in timp
	float			m_fFadeOut_Duration; //0.0f nu face fade, 
	float			m_fFadeIn_Duration;	//daca e 0 nu face fade-in; daca e diferit, face fade in pana cand viata ajunge la LIFE_END
	Vec2		m_vGravity;			//gravitatia particulei
	float			m_fSize;		   //marime
	float			m_fScaleSpeed;	   //viteza de variatie a marimii
	bool			bAnimated;			//daca face update la sprite sau nu
										//pentru desenare
	CSprite			sprite;
};

//Emitter-ul
class CParticleEmitter {
public:
CArray<CParticleEmitterBrush*> m_arrBrushes;
Vec2 pos, speed, dir;
};
//managerul de emitori
class CParticleEmitterManager {
public:
CParticleEmitter* HireEmitter(UINT32 templateID);
void DismissEmitter(CParticleEmitter* pEmitter);
};



//TODO: momentan facem ceva simplu si anume hardcodez emitterii dupa nume. Stiu ca cel cu foc genereaza cumva, cel de ceatza altcumva, etc.
// Fac functionarea de baza acum pentru jocul asta asa cum am zis mai sus iar pentru urmatorul poate fac un editor ingame.
//1. Emitorii ar trebui sa aiba lista de brushuri citite din XML. Practic sa fie niste template-uri salvate ca si emitori iar cand instantiezi unul ii copiaza toate proprietatile
//2. ar trebui ca emitorii sa aiba o lista de particule "furate" din pool-ul general de particule ca sa poti da pauza la update si la paint (cand iese din ecran)

*/

//Emitter-ul
class CParticleEmitter 
{
public:
	UINT32 UID;			//UID generic
	float fTimer;
	float fGenerateTime; //la cat timp genereaza particule
public:
	bool	bPauseUpdate, bPausePaint; //pauza pe update si paint
	bool	bGenerateOutsideScreen; //daca sa genereze particule in afara ecranului

	int		type;   //tipul generatorului K_PART_PE_
	CAABB	bbox;	//boxul in care genereaza
	float	bbox_surface; //suprafata bboxului
	float	densityPerSurfaceUnitPerSec; //numarul de particule generate pe secunda pe unitatea de suprafata

	int particleLayer;	//layerul ar trebui setat in fiecare brush in parte ca sa poti genera pe layere diferite

	CParticleEmitter() : type(K_PART_PE_TYPE_UNKNOWN), fTimer(0.0f), fGenerateTime(0.0f), bPauseUpdate(false), bPausePaint(false), particleLayer(0), bGenerateOutsideScreen(true)
	{
		UID = GenerateUID();
	}
};

const CStringHash EParticleLayer_names[] = {
	L"NORMAL",
	L"NORMAL_LIGHT",
	L"FRONT",
	L"FRONT_LIGHT",
	L"INTERFACE",
	L"INTERFACE_LIGHT",
};

enum EParticleLayer {
	K_PART_LAYER_NORMAL,
	K_PART_LAYER_NORMAL_LIGHT,
	K_PART_LAYER_FRONT,
	K_PART_LAYER_FRONT_LIGHT,
	K_PART_LAYER_INTERFACE,
	K_PART_LAYER_INTERFACE_LIGHT,							   
	// layers count
	K_PART_LAYERS_CNT
};

class CParticlesManager : public IDeviceRes {
private:
	bool				bInitialized;
	int					nParticlesCnt;					// total number of particles
	CParticle*			pParticles;						// particles pool of nParticlesCnt size
	CParticle			pListFree;						// available particles circular list list (only using pNext and pPrev)
	CParticle			pList[K_PART_LAYERS_CNT];		// used particles are kept in rings, one for each layer
	float				fLocalTimeline;

public:
	CSpriteLib			m_sprCol;						//sprites collection for particles

public:
	CParticlesManager();
	~CParticlesManager();

	OPRESULT			Init(WCHAR* XMLpath, int nMaxParticlesCnt = 1000);
	void				Release();			
	void				Update(float dtime);
	void				UpdateLayer(EParticleLayer eLayer, float dtime);
	void				PaintLayer(EParticleLayer eLayer, bool additiveBlending = false);
	//moves all particles to the "FREE" list
	void				ClearParticles();	
	//removes all particles from a layer
	void				RemoveAllFromLayer(EParticleLayer ePartLayer);
	//returns particle layer idx or -1 if not found
	int					GetParticleLayerByName(WCHAR * layerName);
	int					GetParticleLayerByName(UINT32 layerNameHash);

	//--- particles functions ---
	void AddParticle(int animID, bool animated, int currentFrame, Vec2* pos, 
						Vec2* gravity, Vec2* speed, 
						float lifetime, 
						float size, float scalespeed, 
						float rotangle, float rotspeed, 
						float fadeInDuration,
						float fadeOutDuration,
						DWORD nColor = 0xffffffff,
						int nLayer = K_PART_LAYER_NORMAL,
						float airFriction = 0.0f,
						float waitTimer = 0.0f);

///#TODO: remake this part:
	CArray<CParticleEmitter*> m_arrPartEmitters;
	CParticleEmitter* AddPartEmitter(int nType, CAABB * pe_aabb, int nParticleLayer);
	int GetPartEmitterTypeByNameHash(UINT32 generatorNameHash);
	void ReleasePartEmitter(CParticleEmitter* pEmit);
	void ReleaseAllPartEmitters();
	void UpdatePartEmitters(float dTime, RectXYWH screenRect);

///-=-= platform stuff =-=-
	OPRESULT OnCreateDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = nullptr) override;
	OPRESULT OnResetDevice(PDEVICE pDevice, const SURFACE_DESC* pBBDesc = nullptr) override;
	OPRESULT OnLostDevice() override;
	OPRESULT OnDestroyDevice() override;
};


///**************************************************************************************
/// SINGLETON
///**************************************************************************************
CParticlesManager& __Particles();
