#include "dxstdafx.h"
#include "ActorAICompTypes.h"

CAIState::CAIState()
{
	nPriority = 0;
	fProbability = 100.0f;
}

CAIState::~CAIState()
{
}

CAITemplate::~CAITemplate()
{
	SAFE_DELETE_CArray( m_arrStates );
}

CAIState * CAITemplate::GetHighestPriorityState( EAIEventType evtType, CRandom* pRandomGen )
{
	if ( pRandomGen == nullptr )
		return nullptr;

	CFixedArray<CAIState*, 16> arrSelStates;

	int nRetPriority = -1;
	for ( int kk = 0; kk < m_arrStates.GetSize(); kk++ )
	{
		CAIState* pState = m_arrStates[kk];
		//daca am event de tipul curent sau event any (nu se refera si la IDLE_TICK)
		if ( ( pState->m_arrTriggeringEventTypes.Contains( evtType ) ) ||
			( ( evtType > K_AIEVT_IDLE_TICK ) && ( pState->m_arrTriggeringEventTypes.Contains( K_AIEVT_ANY ) ) ) )
		{
			if ( pState->nPriority > nRetPriority )
			{
				arrSelStates.Clear();
				arrSelStates.Add( pState );
				nRetPriority = pState->nPriority;
			}
			else if ( pState->nPriority == nRetPriority )
			{
				arrSelStates.Add( pState );
			}
		}
	}

	//return selected state
	if ( arrSelStates.Count() == 0 )
		return nullptr;
	if ( arrSelStates.Count() == 1 )
		return arrSelStates.m_pData[0];

	//get state based on probability when we have more probabilities with same priority
	float arrProbs[16] = { 0.0f };
	for ( int kk = 0; kk < arrSelStates.Count(); kk++ )
	{
		arrProbs[kk] = arrSelStates.m_pData[kk]->fProbability;
	}
	int nRetIdx = pRandomGen->GetProbabilityFromDomain( arrProbs, arrSelStates.Count() );
	if ( nRetIdx < 0 )
	{
		ErrorBox( K_ERR_WARNING, L"[WARNING] GetHighestPriorityState returned -1! count:%d", arrSelStates.Count() );
		return nullptr;
	}

	return arrSelStates.m_pData[nRetIdx];
}

CAIState * CAITemplate::GetAIStateByName( CStringHash strName )
{
	for ( int kk = 0; kk < m_arrStates.GetSize(); kk++ )
	{
		CAIState* pState = m_arrStates[kk];
		if ( pState->name.textHash == strName.textHash )
			return pState;
	}

#if defined(_DEBUG) || defined(DEBUG)
	ErrorBox( K_ERR_WARNING, L"CAITemplate::GetAIStateByName - state not found [%s]", strName.text );
#endif

	return nullptr;
}



CAISensorInfo::CAISensorInfo() : pTargetedActor(nullptr)
{
	Reset();
}

void CAISensorInfo::Reset()
{
	// call freeref automatically when resetting the sensor info
	FREE_REF( pTargetedActor );
	pTargetedActor = nullptr;
	m_lastInteractingActorUID = 0;
	m_bEnabled = true;
	fTimeSinceHit = 1000.0f;
	vGoTo = { 0.0f, 0.0f };

	evt.Reset();
	evtInternal.Reset();

	WpnStatePrimary = UNAVAILABLE;
	WpnStateSecondary = UNAVAILABLE;
}
