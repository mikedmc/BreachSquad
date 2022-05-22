#pragma once
#include "ComponentInterfaces.h"

class CActorAIComponent : public IBaseActorAIComponent
{
private:
	CLevel&					level;						// reference to level for accessing global data
public: 
	float					AItimerDecision;			// takes decisions when it reaches 0

public:
	CAICommands				AIcommands;					// Commands issued by AI to be executed by the actor
	CAISensorInfo			AIsensor;					// AI sensory information
	CAIState*				m_pAIcurrentState;
	int						m_nAIcurrentBehaviorIdx;	// current behaviour index (in current state) or -1 when not set
	float					m_fAIbehaviorTimer;			// timer used for timed behaviors

public:
	CActorAIComponent( CLevel& levelref );
	~CActorAIComponent();

	void					Update( CActor& act, float dTime ) override;
	//#TODO: should show AI state visually (text) for easy debugging, only on debug builds
	//void Paint( CActor& act );
	
	// Returns current behaviour
	EAIBehaviorType			GetCurrentBehavior();
	// Sets a new AI state
	void					SetAIState( CActor& actor, CAIState* pNewState );
	// Sets a new AI state
	// \returns true:success false:state not found
	bool					SetAIState( CActor& actor, WCHAR * strStateName );

private:
	// Processes the sensor vGoTo request and returns the status (colliding, reached, etc)
	EGenericState			ProcessGoToRequest( CActor & act );
	// Sets behavior by idx, from current state behaviors array
	// \param: ret_bFinished - set to true if current behavior doesn't need an update (like set_animation or set_flag, etc)
	// \returns: true if set, false if error
	bool					SetActorAIBehaviorIdx( CActor& actor, int nBehaviorIdx, bool &ret_bFinished );
	// Called when changing behaviors (to exit them gracefully)
	void					OnActorBehaviorFinished( CActor& actor, EAIBehaviorType eOldBehavior );
	// Finds closest valid AI event of typeFilter (if specified)
	CAIEvent*				GetMostImportantAIEvent( CActor& act, EAIEventType eTypeFilter = K_AIEVT_ANY );
};
