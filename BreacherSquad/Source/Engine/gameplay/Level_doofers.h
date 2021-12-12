#pragma once


#define K_LVL_DOOFERS_MAX_CNT 256
//types
enum EDooferType {
	K_DOOFER_NOT_SET = -1,

	K_DOOFER_SHELL = 0,	//cartusele jucatorului
	K_DOOFER_MEAT = 1,	//carnea care sare din oameni
	K_DOOFER_SHRAPNEL_SMOKING, //bucati de bomba care lasa fum in urma
	K_DOOFER_LIGHT,		//lumina temporara pentru arme, explozii, etc. Deseneaza din m_sprLights.
	K_DOOFER_EXPLOSION,	//explozie care deformeaza ecranul (si deseneaza si explozia (cu particule))
	K_DOOFER_FIRE_SOURCE, //o bucata de foc care moare dupa un timp dar loveste toti oamenii
};

// Class of objects that are "active" and may have special behaviors, and are not props. 
// Includes bullet shells, gibs, etc. Behavior usually hardcoded.
// They are usually short-lived so they don't need visibility lists
class CDoofer {
public:
	EDooferType	type;
	CDoubleLinkedPool<CPhysicsPoint>::CLinkedPoolNode *physPt; //punctul fizic (coliziune, pozitie, etc)

	int			nSubType;	//folosit de fiecare tip in mod diferit
	float		fTimer;		//timer care porneste de la 0
	bool		bAnimated;	//daca e animat sprite-ul
	CSprite		spr, spr2;	//grafica din Props (unele au nevoie de 2 sprites)
	float		fSize;
	//--- variabile lumini ---
	bool		bMakesLight;
	CSprite		sprLight;
	float		fLightDuration, fLightFadeOut;	//durata totala a luminii si durata de fade out
	float		fLightScaling;
	float		fLightTimer;		//timer-ul de viata al luminii
	//--- diverse variabile ---
	bool		bVar1;
	int			nIntVar1;

	CDoofer() : physPt(null), type(K_DOOFER_NOT_SET), nSubType(0), fTimer(0.0f), bAnimated(false), fSize(1.0f),
		bMakesLight(false), fLightDuration(0.0f), fLightFadeOut(0.0f), fLightScaling(1.0f), fLightTimer(0.0f), bVar1(false), nIntVar1(0)
	{
	}

	void Reset()
	{
		physPt = null;
		type = K_DOOFER_NOT_SET; nSubType = 0; fTimer = 0.0f; bAnimated = false; fSize = 1.0f;
		bMakesLight = false; fLightDuration = 0.0f; fLightFadeOut = 0.0f; fLightScaling = 1.0f; fLightTimer = 0.0f;
	}
};



