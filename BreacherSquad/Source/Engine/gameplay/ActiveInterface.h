#pragma once

//--- clasa din care se deriva clasele active ---
//perioada standard de decizie pt AI (in secunde) si variatia random a acestuia
#define	K_LVL_AI_DECISION_INTERVAL				0.25f
#define	K_LVL_AI_DECISION_INTERVAL_VARIATION	0.05f
//perioada standard de asteptare inainte sa schimbe starile (de ex daca nu te mai vede timp de N secunde trece pe idle)
#define K_LVL_AI_STATE_WAIT		3.0f

enum eActiveInterfaceType {
	K_LVL_IAI_TYPE_UNKNOWN,
	K_LVL_IAI_TYPE_BASE,		// basinc IActiveInterface
	K_LVL_IAI_TYPE_LIGHT,
	K_LVL_IAI_TYPE_PROP,
	K_LVL_IAI_TYPE_ACTOR,
	K_LVL_IAI_TYPE_COLSHAPE,
};

class IActiveInterface
{
protected:
	bool				bPendingKill;	// exited gameplay, waits for garbage collection

public:
	UINT32				UID;			// ingame UID
	int					ID;				// ID exported from editor (not the same as UID).
	Vec3				vPos;			// Z coord gets added to Y to simulate 3D when rendering (see Z_TO_H macros)
	Vec3				vPos_ini;		// initial position needed for relative calculations. Usually only set when spawned.

	bool	bTouching;			//folosit ca sa elimine eventuale cicluri infinite.
	UINT32 	nTouchingUID;   	//Reprezinta UID-ul celui care a facut touch sau 0 pt niciunul
	float	fTouchTimerReset;	//folosit la resetarea touch timerului
	float	fTouchTimer;		//pentru cat timp s-a facut touch? sunt obiecte la care trebuie sa faci touch pentru o durata anume
	float	fTouchDuration;		//durata ceruta pentru touch

	Vec2		pos;		   //pozitie activ
	float		fAngle;
	DWORD		color;
	CAABB		bbox;
	CAABB		bbox_exported; //bboxul exportat din BSX (mutat la pozitia activului)
public: //valori initiale ale coordonatelor folositoare la miscari nerelative (unele stari AI le folosesc)
	CAABB		bbox_ini;			//non relative to object position AABB
	CAABB		bbox_exported_ini; //bboxul exportat din BSX (inital)
	Vec2		pos_ini;
	float		fAngle_ini;
	DWORD		color_ini;
	INT32		targetID_ini;	//target ID citit din editor

public: //logic
	IActiveInterface	*pTarget;		//target-ul din editor //TODO:poate trebuie inlocuita cu un UID ca sa nu am probleme cand dezaloc obiecte...? depinde de viteza cu care se cheama la rails
	bool				bCanInteract;	//can interact with it?
	bool				bHideInteractIcon;	//hide the icon

	int					AIstate;		//state AI (AI_STATE ENUM)
	CVariantCollection	varAIparams;	//parametrii state-ului AI
	float				AItimerDecision;//cand ajunge la 0 ia decizii
	float				AItimerSurprise;//timer surprindere inamic
	//diverse pt logica
	UINT32				AItargetUID;	//enemy UID (not the one set from the editor!!!)
	double				fTimelineAI;	//timeline local pt AI (folosit mai ales la animatii in fn de timp)
	//variabile locale rapide AI
	float				AItimer1, AItimer2;	//timere folosite la diverse functii
	float				AIfvar1, AIfvar2, AIfvar3; //diverse variabile folosite in AI
	int					AIvar1, AIvar2;
	Vec2			AIvec1;
	bool				AIvarBool1, AIvarBool2;
	CStringHash			AIstrvar1, AIstrvar2; //variabile string
	int					AIsubState;			//sub-stare folosita la diferite AI-uri

	CStringHash			script_hash;		//hash-ul scriptului
	UINT32				nRunningScriptUID;	//scriptul care ruleaza acum

	bool				bHidden;			//DO NOT SET DIRECTLY! (use bSetHidden) flag de hidden. vizibil si din editor
	bool				bSetHidden;			//flag folosit sa setam hidden in update si nu imediat in script
	bool				bAnimated;			//este animat? daca da face play la animatie
	bool				bReleaseIt;			//needs to be released? (not always implemented)
	bool				bStandsOut;			//if it stands out it should attract attention (used for interactibles)
	bool				bSkipRender;		//skips render...

	//CTOR/DTOR
	IActiveInterface();
	virtual ~IActiveInterface();

	virtual const eActiveInterfaceType GetClassType() const {
		return K_LVL_IAI_TYPE_BASE;
	}

	inline UINT32 GetUID() const {
		return UID;
	}

	//Returns: UID of activ that interacted with it
	inline UINT32 GetToucherUID() const {
		return nTouchingUID;
	}

	// tells if object is still alive or if it is pending kill
	inline bool IsAlive() {
		return (!bPendingKill);
	}

	// Tells if object is waiting to be deallocated
	inline bool IsPendingKill() {
		return bPendingKill;
	}	

	// Loads logic from binary file (editor exported logic)
	void LoadLogic(FILE* fl);

	//functie care se cheama cand interactionezi cu obiectul sau cand este pTarget
	void Touch(UINT32 touchingIActiveUID, float dTime, UINT32 overrideScriptHash = 0, bool bTouchTarget = true);

	void UpdateTouchTimerReset(float dTime);

	//functie care seteaza pozitia complet (adica pos, bbox, puncte relative, etc)
	virtual void SetPos(Vec2 newPos) = 0;
	virtual void Move(Vec2 delta) = 0;
	virtual void SetAngle(float fnAngle) = 0;

	// Call this to mark it for destruction
	void Kill();

	// Gets called after active was added to the actives array, after being fully initialized (end of loading or spawn)
	virtual void PostConstructionInit() = 0;
	// Gets called by the engine as soon as the object gets initialized
	virtual void BeginPlay() = 0;
	// Gets called when gets killed
	virtual void EndPlay() = 0;
};
