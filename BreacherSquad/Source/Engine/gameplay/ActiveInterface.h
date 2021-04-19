#pragma once

// declare useful classes
class CLevelArea;

//perioada standard de decizie pt AI (in secunde) si variatia random a acestuia
#define	K_LVL_AI_DECISION_INTERVAL				0.25f
#define	K_LVL_AI_DECISION_INTERVAL_VARIATION	0.05f

enum eActiveInterfaceType {
	K_LVL_IAI_TYPE_UNKNOWN,
	K_LVL_IAI_TYPE_BASE,		// basic IActiveInterface
	K_LVL_IAI_TYPE_LIGHT,
	K_LVL_IAI_TYPE_PROP,
	K_LVL_IAI_TYPE_ACTOR,
	K_LVL_IAI_TYPE_COLSHAPE,
};

///----------------------------------------------------------------------------------
/// Base class for all active elements (actors, objects, etc) 
///----------------------------------------------------------------------------------
class IActiveInterface
{
protected:
	bool					bPendingKill;			// exited gameplay, waits for garbage collection

public:
	UINT32					UID;					// ingame UID
	CLevelArea*				pArea;					// pointer to current area
	int						ID;						// ID exported from editor (not the same as UID).
	Vec3					vPos;					// Z coord gets added to Y to simulate 3D when rendering (see Z_TO_H macros)
	Vec3					vPos_ini;				// initial position needed for relative calculations. Usually only set when spawned.

	bool					bTouching;				// folosit ca sa elimine eventuale cicluri infinite.
	UINT32 					nTouchingUID;   		// Reprezinta UID-ul celui care a facut touch sau 0 pt niciunul
	float					fTouchTimerReset;		// folosit la resetarea touch timerului
	float					fTouchTimer;			// pentru cat timp s-a facut touch? sunt obiecte la care trebuie sa faci touch pentru o durata anume
	float					fTouchDuration;			// durata ceruta pentru touch
													   
	Vec2					pos;					
	float					fAngle;
	DWORD					color;
	float					fHeight;				// height in world coords
	CAABB					bbox;
	CAABB					bbox_proj;				// projected bbox (combines floor bbox and height)
	CAABB					bbox_exported;			// bbox exported from BSX (moved at position)
// initial values necessary for relative movements
public: 
	CAABB					bbox_ini;				// non relative to object position AABB
	CAABB					bbox_proj_ini;			// non relative to pos projected bbox
	CAABB					bbox_exported_ini;		// initial bbox exported from BSX
	Vec2					pos_ini;
	float					fAngle_ini;
	DWORD					color_ini;
	INT32					targetID_ini;			// target ID citit din editor

public: //logic
	IActiveInterface		*pTarget;				// target-ul din editor //TODO:poate trebuie inlocuita cu un UID ca sa nu am probleme cand dezaloc obiecte...? depinde de viteza cu care se cheama la rails
	bool					bCanInteract;			// can interact with it?  #TODO: replace with interact-type or actions list
	bool					bHideInteractIcon;		// hide the icon

	EAIstate				AIstate;				// state AI (AI_STATE ENUM)
	CVariantCollection		varAIparams;			// AIstate params
	float					AItimerDecision;		// takes decisions when it reaches 0
	//diverse pt logica
	UINT32					AItargetUID;			// enemy UID (not the one set from the editor!!!)
	double					fTimelineAI;			// local timeline for AI 
	
	//#TODO: variabile locale rapide AI - ar trebui incluse intr-o structura cu serialize/deserialize eventual
	float					AItimer1, AItimer2;		
	float					AIfvar1, AIfvar2, AIfvar3; 
	int						AIvar1, AIvar2;
	Vec2					AIvec1;
	bool					AIvarBool1, AIvarBool2;
	CStringHash				AIstrvar1, AIstrvar2; 
	int						AIsubState;				// AI substate used here and there, everywhere

	CStringHash				script_hash;			// script hash
	UINT32					nRunningScriptUID;		// UID of script that is running now on this element

	bool					bHidden;				// DO NOT SET DIRECTLY! (use bSetHidden) flag de hidden. vizibil si din editor
	bool					bSetHidden;				// #TODO: ar trebui inlocuit cu SetHidden(T/F, FORCED)
	bool					bAnimated;				// este animat? daca da face play la animatie
	bool					bSkipRender;			// skips render...

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
