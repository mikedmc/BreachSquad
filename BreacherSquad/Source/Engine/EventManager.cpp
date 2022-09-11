#include "dxstdafx.h"

///--- EVENT ---
static CVariant m_ArgZero;

CEvent::CEvent(const WCHAR *strEventType, const WCHAR *strEventCommand, double fEventTime)
{
	m_eventType.Init(strEventType);
	m_eventCommand.Init(strEventCommand);
	m_fEventTime = fEventTime;

	m_argsCnt = 0;
}

CEvent::CEvent(const CStringHash hashEventType, const CStringHash hashEventCommand, double fEventTime)
{
	m_eventType = hashEventType;
	m_eventCommand = hashEventCommand;
	m_fEventTime = fEventTime;

	m_argsCnt = 0;
}

///--- HELPER FUNCTIONS ---
int CEvent::AddArgUINT32(UINT32 val)
{
	assert(m_argsCnt < K_MAX_EVENT_ARGS);

	m_args[m_argsCnt].eType = CVariant::K_ARGTYPE_UINT32;
	m_args[m_argsCnt].m_asUINT32 = val;
	m_argsCnt++;
	return m_argsCnt;
}

int CEvent::AddArgINT32(INT32 val)
{
	assert(m_argsCnt < K_MAX_EVENT_ARGS);

	m_args[m_argsCnt].eType = CVariant::K_ARGTYPE_INT32;
	m_args[m_argsCnt].m_asINT32 = val;
	m_argsCnt++;
	return m_argsCnt;
}

int CEvent::AddArgFloat(float val)
{
	assert(m_argsCnt < K_MAX_EVENT_ARGS);

	m_args[m_argsCnt].eType = CVariant::K_ARGTYPE_FLOAT;
	m_args[m_argsCnt].m_asFloat = val;
	m_argsCnt++;
	return m_argsCnt;
}

int CEvent::AddArgBool(bool val)
{
	assert(m_argsCnt < K_MAX_EVENT_ARGS);

	m_args[m_argsCnt].eType = CVariant::K_ARGTYPE_BOOL;
	m_args[m_argsCnt].m_asBool = val;
	m_argsCnt++;
	return m_argsCnt;
}

int CEvent::AddArgVoidP(void* val)
{
	assert(m_argsCnt < K_MAX_EVENT_ARGS);

	m_args[m_argsCnt].eType = CVariant::K_ARGTYPE_VOIDP;
	m_args[m_argsCnt].m_asVoid = val;
	m_argsCnt++;
	return m_argsCnt;
}

int CEvent::AddArgString(const WCHAR* strVal)
{
	assert(m_argsCnt < K_MAX_EVENT_ARGS);

	m_args[m_argsCnt].eType = CVariant::K_ARGTYPE_STRING;
	m_args[m_argsCnt].m_asUINT32 = 0;
	m_args[m_argsCnt].m_strArg.Init(strVal);
	m_argsCnt++;
	return m_argsCnt;
}

//
// named functions
//
int CEvent::AddNamedArgUINT32(const WCHAR* argName, UINT32 val)
{
	m_args[m_argsCnt].shName.Init(argName);
	m_args[m_argsCnt].eType = CVariant::K_ARGTYPE_UINT32;
	m_args[m_argsCnt].m_asUINT32 = val;
	m_argsCnt++;
	return m_argsCnt;
}

int CEvent::AddNamedArgINT32(const WCHAR* argName, INT32 val)
{
	assert(m_argsCnt < K_MAX_EVENT_ARGS);

	m_args[m_argsCnt].shName.Init(argName);
	m_args[m_argsCnt].eType = CVariant::K_ARGTYPE_INT32;
	m_args[m_argsCnt].m_asINT32 = val;
	m_argsCnt++;
	return m_argsCnt;
}

int CEvent::AddNamedArgFloat(const WCHAR* argName, float val)
{
	assert(m_argsCnt < K_MAX_EVENT_ARGS);

	m_args[m_argsCnt].shName.Init(argName);
	m_args[m_argsCnt].eType = CVariant::K_ARGTYPE_FLOAT;
	m_args[m_argsCnt].m_asFloat = val;
	m_argsCnt++;
	return m_argsCnt;
}

int CEvent::AddNamedArgBool(const WCHAR* argName, UINT32 val)
{
	assert(m_argsCnt < K_MAX_EVENT_ARGS);

	m_args[m_argsCnt].shName.Init(argName);
	m_args[m_argsCnt].eType = CVariant::K_ARGTYPE_BOOL;
	m_args[m_argsCnt].m_asBool = (val != 0);
	m_argsCnt++;
	return m_argsCnt;
}

int CEvent::AddNamedArgVoidP(const WCHAR* argName, void* val)
{
	assert(m_argsCnt < K_MAX_EVENT_ARGS);

	m_args[m_argsCnt].shName.Init(argName);
	m_args[m_argsCnt].eType = CVariant::K_ARGTYPE_VOIDP;
	m_args[m_argsCnt].m_asVoid = val;
	m_argsCnt++;
	return m_argsCnt;
}

int CEvent::AddNamedArgString(const WCHAR* argName, const WCHAR* strVal)
{
	assert(m_argsCnt < K_MAX_EVENT_ARGS);

	m_args[m_argsCnt].shName.Init(argName);
	m_args[m_argsCnt].eType = CVariant::K_ARGTYPE_STRING;
	m_args[m_argsCnt].m_asUINT32 = 0;
	m_args[m_argsCnt].m_strArg.Init(strVal);
	m_argsCnt++;
	return m_argsCnt;
}

//
// get argument
// daca nu gaseste argumentul intoarce unul default setat pe 0
//
CVariant* CEvent::GetArgumentByName(const WCHAR* argName)
{
	UINT32 argNameHash = FastHash(argName, wcslen(argName));
	register int kk = 0;
	//optimizare de viteza
	CVariant *pArg = &m_args[0];
	while(kk < m_argsCnt)
	{
		if(pArg->shName.textHash == argNameHash)
			return pArg;

		pArg++;
		kk++;
	}

	//ErrorBox(K_ERR_DEBUGOUT, L"Event Argument not found! ArgName=%s", argName);
	return &m_ArgZero;
}

///--- EVENTS MANAGER ---
CEventManager::CEventManager()
{
	m_activeQueue = 0;
	m_eventListeners.RemoveAll();
	m_eventListenersTypes.RemoveAll();

	for(int kk=0; kk < K_EVENTMGR_QUEUES; kk++)
		m_eventQueues[kk].RemoveAll();
}

CEventManager::~CEventManager()
{
	for(int kk=0; kk < m_eventListenersTypes.GetSize(); kk++)
	{
		SAFE_DELETE(m_eventListenersTypes[kk]);
	}
	m_eventListenersTypes.RemoveAll();

	m_activeQueue = 0;
	m_eventListeners.RemoveAll();

	for(int kk=0; kk < K_EVENTMGR_QUEUES; kk++)
	{
		for(int ll=0; ll<m_eventQueues[kk].GetSize(); ll++)
		{
			SAFE_DELETE(m_eventQueues[kk][ll]);
		}
		m_eventQueues[kk].RemoveAll();
	}
}

bool CEventManager::AddListener(IEventListener* pEventListener, const CStringHash hashEventType)
{
	//verifica daca e deja adaugat
	UINT32 eventTypeHash = hashEventType.textHash;
	for (int kk = 0; kk < m_eventListeners.Count(); kk++)
	{
		if((pEventListener == m_eventListeners[kk]) && (m_eventListenersTypes[kk]->textHash == eventTypeHash))
			return false;
	}
	//daca nu este, il adaug acum
	m_eventListeners.Add(pEventListener);
	m_eventListenersTypes.Add(new CStringHash(hashEventType.text));

	return true;
}

bool CEventManager::AddListener(IEventListener* pEventListener, const WCHAR* strEventType)
{
	//verifica daca e deja adaugat
	UINT32 eventTypeHash = FastHash(strEventType, wcslen(strEventType));
	for(int kk=0; kk<m_eventListeners.Count(); kk++)
	{
		if((pEventListener == m_eventListeners[kk]) && (m_eventListenersTypes[kk]->textHash == eventTypeHash))
			return false;
	}
	//daca nu este, il adaug acum
	m_eventListeners.Add(pEventListener);
	m_eventListenersTypes.Add(new CStringHash(strEventType));

	return true;
}

bool CEventManager::QueueEvent(CEvent* inEvent)
{
	assert((m_activeQueue >= 0) && (m_activeQueue < K_EVENTMGR_QUEUES));

	m_eventQueues[m_activeQueue].Add(inEvent);
	return true;
}

bool CEventManager::TriggerEvent(CEvent *inEvent)
{
	assert(inEvent != NULL);

	CEvent* pEvent = inEvent;

	bool eventHandled = false;
	bool eventConsumed = false;
	for(int ll=0; ll < m_eventListenersTypes.GetSize(); ll++)
	{
		if(pEvent->m_eventType == *m_eventListenersTypes[ll])
		{
			if(m_eventListeners[ll]->HandleEvent(*pEvent)) //if consumed event
			{
				eventHandled = eventConsumed = true;
			}
			else
			{
				eventHandled = true;
			}
		}
		//if event was consumed skip the rest of the listeners
		if(eventConsumed)
			break;
	}

	//don't need the event anymore so delete event data
	SAFE_DELETE(inEvent);

	return eventConsumed;
}

void CEventManager::Update(float dTime, float fTimeLine)
{
//TODO: de pus un timp de procesare maxim a evnturilor dupa care sa le mute in coada urmatoare si sa iasa din functie
//TODO: de implementat cu stl::priorityqueue coada de mesaje?
	assert((m_activeQueue >= 0) && (m_activeQueue < K_EVENTMGR_QUEUES));
	//daca nu sunt mesaje iese imediat
	if(m_eventQueues[m_activeQueue].GetSize() <= 0)
		return;

	CEvent *pEvent = NULL;
	for(int kk=0; kk < m_eventQueues[m_activeQueue].GetSize(); kk++)
	{
		pEvent = m_eventQueues[m_activeQueue].GetAt(kk);

		//if event is in the future, add it to the next queue and continue to next event
		if(pEvent->m_fEventTime > fTimeLine)
		{
			//aici se foloseste timeLine-ul general (nu local)
			m_eventQueues[(m_activeQueue + 1) % K_EVENTMGR_QUEUES].Add(pEvent);
			continue;
		}
		//if event must be executed search for the listeners
		bool eventHandled = false;
		bool eventConsumed = false;
		for(int ll = 0; ll < m_eventListenersTypes.GetSize(); ll++)
		{
			if(pEvent->m_eventType == *m_eventListenersTypes[ll])
			{
				if (m_eventListeners[ll]->HandleEvent(*pEvent)) //if consumed event
				{
					eventHandled = eventConsumed = true;
				}
				else
				{
					eventHandled = true;
				}
			}
			//if event was consumed skip the rest of the listeners
			if(eventConsumed)
				break;
		}
		//if event wasn't handled nor consumed log it
#if defined(_DEBUG) || defined(DEBUG)
		if(!eventHandled)
		{
			LOG(L"EventManager::Event not handled!\nType: %s Command: %s\n", pEvent->m_eventType.text, pEvent->m_eventCommand.text);
		}
#endif
		//don't need the event anymore so delete event data
		SAFE_DELETE(m_eventQueues[m_activeQueue][kk]);
	}
	//checked all events so empty queue
	m_eventQueues[m_activeQueue].RemoveAll();
	//swap queues
	m_activeQueue = (m_activeQueue + 1) % K_EVENTMGR_QUEUES;
}


///**************************************************************************************
/// Sigleton 
///**************************************************************************************

CEventManager& __Events()
{
	static CEventManager g_EventMgr;
	return g_EventMgr;
}

