#pragma once
#include "ComponentInterfaces.h"

class CActorAIComponent : public IBaseAIComponent
{
public: 
	//#TODO: de mutat in ibaseAIcomponent variabilele comune
	IActiveInterface		*pTarget;				// target-ul din editor //TODO:poate trebuie inlocuita cu un UID ca sa nu am probleme cand dezaloc obiecte...? depinde de viteza cu care se cheama la rails
	bool					bCanInteract;			// can interact with it?  #TODO: replace with interact-type or actions list
	bool					bHideInteractIcon;		// hide the icon

	EAIstate				AIstate;				// state AI (AI_STATE ENUM)
	CVariantCollection		varAIparams;			// AIstate params
	float					AItimerDecision;		// takes decisions when it reaches 0

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

public:
	CAISensorInfo	m_AIsensorInfo;				// AI sensory information
	CAIState*		m_pAIcurrentState;
	int				m_nAIcurrentBehaviorIdx;	// current behaviour index (in current state) or -1 when not set
	float			m_fAIbehaviorTimer;			// timer used for timed behaviors

public:
	CActorAIComponent();
	~CActorAIComponent();

	void Update( CActor& act, float dTime, CLevel & level ) override;
	//#TODO: should show AI state visually (text) for easy debugging, only on debug builds
	//void Paint( CActor& act );
	
	// Returns current behaviour
	EAIBehaviorType GetCurrentBehavior();
};
