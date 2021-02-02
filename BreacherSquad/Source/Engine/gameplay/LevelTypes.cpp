#include "dxstdafx.h"


///--------------------------------------------------------------------------
///--- ACTORS AI ---
///--------------------------------------------------------------------------

CAIState::CAIState()
{
	nPriority = 0;
	fProbability = 100.0f;
}

CAIState::~CAIState()
{
}

///--- AI TEMPLATE ---
CAITemplate::~CAITemplate()
{
	SAFE_DELETE_GROWABLE_ARRAY(m_arrStates);
}

CAIState * CAITemplate::GetHighestPriorityState(EAIEventType evtType, CRandom* pRandomGen)
{
	if (pRandomGen == null)
		return null;

	CFixedArray<CAIState*, 16> arrSelStates;

	int nRetPriority = -1;
	for (int kk = 0; kk < m_arrStates.GetSize(); kk++)
	{
		CAIState* pState = m_arrStates[kk];
		//daca am event de tipul curent sau event any (nu se refera si la IDLE_TICK)
		if( (pState->m_arrTriggeringEventTypes.Contains(evtType)) || 
			((evtType > K_LVL_AI_EVENT_IDLE_TICK) && (pState->m_arrTriggeringEventTypes.Contains(K_LVL_AI_EVENT_ANY))) )
		{
			if (pState->nPriority > nRetPriority)
			{
				arrSelStates.Clear();
				arrSelStates.Add(pState);
				nRetPriority = pState->nPriority;
			}
			else if(pState->nPriority == nRetPriority)
			{
				arrSelStates.Add(pState);
			}
		}
	}

	//return selected state
	if (arrSelStates.Count() == 0)
		return null;
	if (arrSelStates.Count() == 1)
		return arrSelStates.m_pData[0];
	
	//get state based on probability when we have more probabilities with same priority
	float arrProbs[16] = { 0.0f };
	for (int kk = 0; kk < arrSelStates.Count(); kk++)
	{
		arrProbs[kk] = arrSelStates.m_pData[kk]->fProbability;
	}
	int nRetIdx = pRandomGen->GetProbabilityFromDomain(arrProbs, arrSelStates.Count());
	if (nRetIdx < 0)
	{
		ErrorBox(K_ERR_WARNING, L"[WARNING] GetHighestPriorityState returned -1! count:%d", arrSelStates.Count());
		return null;
	}

	return arrSelStates.m_pData[nRetIdx];
}

CAIState * CAITemplate::GetAIStateByName(CStringHash strName)
{
	for (int kk = 0; kk < m_arrStates.GetSize(); kk++)
	{
		CAIState* pState = m_arrStates[kk];
		if (pState->name.textHash == strName.textHash)
			return pState;
	}

#if defined(_DEBUG) || defined(DEBUG)
	ErrorBox(K_ERR_WARNING, L"CAITemplate::GetAIStateByName - state not found [%s]", strName.text);
#endif

	return null;
}



///--------------------------------------------------------------------------
/// MISC OBJECTS - diverse obiecte speciale exportate din editor (RAILS, etc)
///--------------------------------------------------------------------------

Vec2 CMiscObjectRail::GetPosNormalized(float fCursorNormalized, Vec2 * retDir)
{
	return GetPos(fLength * fCursorNormalized, retDir);
}

Vec2 CMiscObjectRail::GetPos(float fDistFromStart, Vec2 * retDir)
{
	if (fDistFromStart < 0.0f)
		return arrPoints[0];
	if (fDistFromStart > fLength)
		return arrPoints[arrPoints.nCount - 1];

	int selidx = 0;
	for (int kk = 1; kk < arrLenghts.nCount; kk++)
	{
		if (arrLenghts.m_pData[kk] >= fDistFromStart)
		{
			selidx = kk;
			break;
		}
	}
	//nu ar trebui sa nu gaseasca nod	
	assert(selidx > 0);
	//daca nu am gasit nod mai mare inseamna ca e in afara
	float percent = (fDistFromStart - arrLenghts.m_pData[selidx - 1]) / (arrLenghts.m_pData[selidx] - arrLenghts.m_pData[selidx - 1]);
	//directia
	if (retDir != NULL)
	{
		D3DXVec2Normalize(retDir, &(arrPoints.m_pData[selidx] - arrPoints.m_pData[selidx - 1]));
	}
	//interpolare liniara
	return arrPoints.m_pData[selidx] * percent + arrPoints.m_pData[selidx - 1] * (1.0f - percent);
}


