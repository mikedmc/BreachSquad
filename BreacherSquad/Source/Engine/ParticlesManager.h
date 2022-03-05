#pragma once


///--- constante coada grafica ---
#define K_PART_TAIL_MAX_SIZE  15
#define	K_PART_TAIL_MAX_COUNT	200

class CTail {
public:
	int				status; //0 nefolosita, !=1 folosita -> arata si textura pe care o foloseste
	D3DXVECTOR2		tailPos[K_PART_TAIL_MAX_SIZE];
	float			tailLife[K_PART_TAIL_MAX_SIZE]; //cat traieste un nod de coada
	//variabile updatate per frame
	float			width; //latimea curenta 

	float			timer; //la cat timp pune un punct in lista
	float			timerDropSpeed; //viteza cu care scade timer-ul de la 1.0f in jos
	float			tailDropSpeed;	//viteza cu care scade transparenta la coada
	D3DXVECTOR2		texPt1;	//top left in textura
	D3DXVECTOR2		texPt2;	//bottom right in textura

	DWORD			color;

	CTail();
};

///--- new particles manager ----
class CParticle {
public:
	float m_fLife;					//viata actuala
	float m_fAlpha;
	//pointeri catre vecini
	CParticle*		pNext;
	CParticle*		pPrev;   
public:
	D3DXVECTOR2		m_vPos;
	D3DXVECTOR2		m_vSpeed;		  //directia particulei (modificabila de update)
	float			m_fRotAngle;	  //unghiul de rotatie
	float			m_fRotSpeed;	  // viteza de rotatie
	DWORD			m_Color;	
	float			m_fWaitTimer;		// cat asteapta intainte sa faca update	(spawning intarziat)
	float			m_fLifetime;	  //cat traieste particula in timp
	float			m_fFadeOut_Duration; //0.0f nu face fade, 
	float			m_fFadeIn_Duration;	//daca e 0 nu face fade-in; daca e diferit, face fade in pana cand viata ajunge la LIFE_END
	D3DXVECTOR2		m_vGravity;			//gravitatia particulei
	float			m_fSize;		   //marime
	float			m_fScaleSpeed;	   //viteza de variatie a marimii
	bool			bAnimated;			//daca face update la sprite sau nu
	float			m_fAirFriction;		//frecarea cu aerul
	//pentru desenare
	CSprite			sprite;

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
	D3DXVECTOR2		m_vPos;
	D3DXVECTOR2		m_vSpeed;		  //directia particulei (modificabila de update)
	float			m_fRotAngle;	  //unghiul de rotatie
	float			m_fRotSpeed;	  // viteza de rotatie
	DWORD			m_Color;
	float			m_waitTimer;		// cat asteapta intainte sa faca update
	float			m_fLifetime;	  //cat traieste particula in timp
	float			m_fFadeOut_Duration; //0.0f nu face fade, 
	float			m_fFadeIn_Duration;	//daca e 0 nu face fade-in; daca e diferit, face fade in pana cand viata ajunge la LIFE_END
	D3DXVECTOR2		m_vGravity;			//gravitatia particulei
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
D3DXVECTOR2 pos, speed, dir;
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


///--- string dummies ---
#define K_PDUMMY_STRING_BLINKER			0
#define K_PDUMMY_STRING_LETTERWAVER		1
#define K_PDUMMY_STRING_WOBBLER			2
//pozitia este offset fata de centru:
#define K_PDUMMY_STRING_WIDEBAR			3

class CStringDummy {
public:
	int				type;
	int 			status;
	D3DXVECTOR2		pos;			//pozitia
	D3DXVECTOR2		v;				//viteza
	D3DXVECTOR2		pt1, pt2, pt3;	//ajutatoare pentru animatie
	Vec2i		intPt;			//coord int
	float			timer;			//timer propriu pt animatie sau viatza
	float			fparam;			//parametru float
	float			fparam2;			//parametru float 2
	float			angle;			//generic angle
	int				intParam;		//parametru int
	int				intParam2;		//alt param int
	int				intParam3;		//parametru int
	int				intParam4;		//alt param int
	float			fShowDelay;		//hidden while delayed

	UINT8			flag;			//folosit cand vreau sa desenez pe layere separate
	CSprite			sprite;

	float*			fPtr;

	CStringDummy() : fShowDelay(0.0f), fPtr(NULL), intParam3(-1), intParam4(-1)
	{
	}
};

class CStringParticle {
public:
	D3DXVECTOR2		m_vPos;
	D3DXVECTOR2		m_vSpeed;		  //directia particulei (modificabila de update)
	float			m_fRotAngle;	  //unghiul de rotatie
	float			m_fRotSpeed;	  // viteza de rotatie
	float			m_fLifetime;	  //cat traieste particula in timp
	float			m_fFadeOut_Duration; //0.0f nu face fade, 
	float			m_fFadeIn_Duration;	//daca e 0 nu face fade-in; daca e diferit, face fade in pana cand viata ajunge la LIFE_END
	D3DXVECTOR2		m_vGravity;			//gravitatia particulei
	float			m_fSize;		   //marime
	float			m_fScaleSpeed;	   //viteza de variatie a marimii

	//pentru particule string
	CStringDesc		m_stringDesc;
	CTexFont		*m_pFont;	//id-ul fontului bitmap
	DWORD			strColor;

	int				mLayer; //layer particula

	CStringParticle();
//private:
	float m_fLife;					//viata actuala
	float m_fAlpha;
};


const CStringHash ParticleLayers_names[] = {
	L"RT_BACK_NRM",
	L"RT_BACK_NRM_LIGHT",
	L"RT_FRONT_NRM",
	L"RT_FRONT_NRM_LIGHT",
	L"NORMAL",
	L"NORMAL_LIGHT",
	L"FRONT",
	L"FRONT_LIGHT",
	L"INTERFACE",
	L"INTERFACE_LIGHT",
	L"CONTROLS",
	L"CONTROLS_LIGHT"
};

enum ParticleLayers{
	K_PART_LAYER_RT_BACK_NRM = 0, //in spatele poersonajului; deseneaza si normalele si self illumination
	K_PART_LAYER_RT_BACK_NRM_LIGHT = 1, //in spatele poersonajului; deseneaza si normalele si self illumination - additive blending
	K_PART_LAYER_RT_FRONT_NRM, //in fata personajului; deseneaza si normalele si self illumination
	K_PART_LAYER_RT_FRONT_NRM_LIGHT, //in fata personajului; deseneaza si normalele si self illumination - additive blending
	K_PART_LAYER_NORMAL,
	K_PART_LAYER_NORMAL_LIGHT,
	K_PART_LAYER_FRONT,	//deasupra intregii scene
	K_PART_LAYER_FRONT_LIGHT, //deasupra intregii scene, cu additive
	//paint-ul layerelor de mai jos este administrat de interfete_custom/controale specifice. Nu se cheama in main, generic
	K_PART_LAYER_INTERFACE,
	K_PART_LAYER_INTERFACE_LIGHT,							   
	K_PART_LAYER_CONTROLS,
	K_PART_LAYER_CONTROLS_LIGHT,

	K_PART_LAYERS_CNT
};

class CParticlesManager {
private:
	bool			bInitialized;
	int				nParticlesCnt;	//numarul total de particule folosite
	CParticle*		pParticles;		//array-ul alocat dintr-o bucata cu toate particulele
	//listele circulare de particule libere si folosite
	CParticle pListFree;	//free particles list (folosim doar pointerii pNext si pPrev)
	CParticle pList[K_PART_LAYERS_CNT]; //used particles (folosim doar pointerii pNext si pPrev)

	float fLocalTimeline;
	LPDIRECT3DDEVICE9	m_pDevice;
	ID3DXSprite*		m_pSprite;  //pointer la sprite

	//update chemate din update-ul particles manager
	void UpdateStringParticles(float dTime);
public:
	//sprites collection
	CSpriteLib m_sprCol;	
	//string particles - growable array pentru ca sunt foarte putine mereu
	CArray<CStringDummy*> m_vDummies;
	//string dummies
	CArray<CStringParticle*> m_vStringParticles; //colectie de particule string

	void SetSpritePtr(ID3DXSprite* pSprite) {
		m_pSprite = pSprite;
	}
	///-=-=-= TAILS =-=-=-
	CTail tails[K_PART_TAIL_MAX_COUNT];
	int GetFreeTail(int AnimIDx, int frameIDx, float timerDropPerSec, float tailDropPerSec, DWORD nColor = 0xffffffff);
	void UpdateTailData(int tailIdx, D3DXVECTOR2 newPos, float newWidth);
	void UpdateTails(float dTime);
	void PaintTails(D3DXVECTOR2* offset = NULL);
	///-=-= PARTICLES =-=-
	CParticlesManager();
	~CParticlesManager();
	//Initialize
	HRESULT Init(WCHAR* XMLpath, int nMaxParticlesCnt = 1000);
	void Release();			//releases everything

	void Update(float dtime);
	void UpdateLayer(int nLayer, float dtime);
	void PaintLayer(int nLayer, bool additiveBlending = false);
	void PaintLayerOffset(int nLayer, D3DXVECTOR2 offset, bool additiveBlending = false);
	void PaintLayerOffset_texOverride(int nLayer, D3DXVECTOR2 offset, bool additiveBlending = false, int texIdxOffset = 0);
	//moves all particles to the "FREE" list
	void RemoveAll();	
	//removes all particles from a layer
	void RemoveAllFromLayer(ParticleLayers ePartLayer);

	//returns particle layer idx or -1 if not found
	int GetParticleLayerByName(WCHAR * layerName);
	int GetParticleLayerByName(UINT32 layerNameHash);

	//--- particles functions ---
	void AddParticle(int animID, bool animated, int currentFrame, D3DXVECTOR2* pos, 
						D3DXVECTOR2* gravity, D3DXVECTOR2* speed, 
						float lifetime, 
						float size, float scalespeed, 
						float rotangle, float rotspeed, 
						float fadeInDuration,
						float fadeOutDuration,
						DWORD nColor = 0xffffffff,
						int nLayer = K_PART_LAYER_NORMAL,
						float airFriction = 0.0f,
						float waitTimer = 0.0f);

///-=-=-= STRING DUMMIES =-=-=-

	int AddStringDummy(int nType, D3DXVECTOR2 np1, int stringID, int fontID, float size = 1.0f, float showTime = 1.0f, DWORD color = 0xffffffff, float fDelay = 0.0f);
	CStringDummy* GetStringDummy(int nType);
	void UpdateStringDummies(float dTime);
	void PaintStringDummies(UINT8 pflags = 0);
	void RemoveStringDummies();	 
	CStringDummy* GetDummy(int nType);

///-=-= STRING PARTICLES =-=-

	void PaintStringParticles(int nLayer, bool paintUsingMultiply = false);
	void PaintStringParticles(int nLayer, D3DXVECTOR2 offset, bool paintUsingMultiply = false);

	void AddStringParticle(CTexFont *pFont, WCHAR* text,
						D3DXVECTOR2* pos,
						D3DXVECTOR2* gravity, D3DXVECTOR2* speed,
						float lifetime,
						float size, float scalespeed,
						float rotangle, float rotspeed,
						float fadeInDuration,
						float fadeOutDuration,
						DWORD nColor = 0xffffffff,
						int nLayer = K_PART_LAYER_NORMAL);
	void RemoveStringParticles();

///-=-=-= PARTICLE_EMITTERS =-=-=-
	int GetPartEmitterTypeByNameHash(UINT32 generatorNameHash);
	
	CArray<CParticleEmitter*> m_arrPartEmitters;

	CParticleEmitter* AddPartEmitter(int nType, CAABB * pe_aabb, int nParticleLayer);
	void ReleasePartEmitter(CParticleEmitter* pEmit);
	void ReleaseAllPartEmitters();
	void UpdatePartEmitters(float dTime, RectXYWH screenRect);

///-=-=-= HELPER FUNCTIONS =-=-=-
	void GenerateBulletHitWall(D3DXVECTOR2 npos, D3DXVECTOR2 ndir, int nLayer = K_PART_LAYER_NORMAL);
	void GenerateBulletHitEnemy(D3DXVECTOR2 npos, D3DXVECTOR2 ndir, int eVictimClass, int nLayer = K_PART_LAYER_NORMAL);
	void GenerateBulletHitMetal(D3DXVECTOR2 npos, D3DXVECTOR2 ndir, int nLayer = K_PART_LAYER_NORMAL);
	void GenerateFireRing(D3DXVECTOR2 npos, int nPartCnt, float fSpeedMin, float fSpeedMax, int nLayer = K_PART_LAYER_NORMAL);
	void GenerateTeleportEffect(D3DXVECTOR2 npos, DWORD dwColor, int nLayer = K_PART_LAYER_NORMAL);
	void GenerateHealEffect(D3DXVECTOR2 npos, DWORD dwColor, int nLayer = K_PART_LAYER_NORMAL);
	void GenerateZombieSpawn(D3DXVECTOR2 npos, DWORD dwColor, int nLayer = K_PART_LAYER_NORMAL);
	void GenerateRaysHemi(D3DXVECTOR2 npos, DWORD dwColor, int nLayer = K_PART_LAYER_NORMAL);
	void GenerateEnemySoftGib(D3DXVECTOR2 npos, DWORD dwColor, int nLayer = K_PART_LAYER_NORMAL);
	//folosit cand apar stelele pe fereastra
	void GenerateStarEffect(D3DXVECTOR2 npos, int nLayer = K_PART_LAYER_NORMAL);
	//folosit cand spargi o usa
	void GenerateDoorBreak(D3DXVECTOR2 npos, D3DXVECTOR2 dir, int nLayer = K_PART_LAYER_NORMAL);
	void GenerateSmokePuff(D3DXVECTOR2 npos, float fRadius, int nLayer = K_PART_LAYER_NORMAL);
	void GenerateHeadshot(D3DXVECTOR2 npos, D3DXVECTOR2 dir, DWORD dwColor, int nLayer = K_PART_LAYER_NORMAL);

///-=-=-= framework stuff =-=-=-
	HRESULT OnCreateDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnResetDevice(IDirect3DDevice9* pd3dDevice, const D3DSURFACE_DESC* pBackBufferSurfaceDesc = NULL);
	HRESULT OnLostDevice(void);
	HRESULT OnDestroyDevice(void);
};


