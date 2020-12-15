#pragma once

#define K_MAX_EVENT_ARGS 10

class CEvent {
public:
	//static CVariantComplex	m_ArgZero; //argument intors cand nu gaseste argumentul cautat dupa nume

	CStringHash		m_eventType;
	CStringHash		m_eventCommand;
	float			m_fEventTime;  //time of execution (mai mic decat fTimeline pt activare imediata sau mai mare pt activare in viitor)
	//event arguments
	int				m_argsCnt; //nr parametri event
	CVariantComplex m_args[K_MAX_EVENT_ARGS]; //daca merge incet cu VariantComplex revin la CVariant simplu
	//create event
	CEvent(const WCHAR* strEventType, const WCHAR* strEventCommand, double fEventTime = 0.0f);
	CEvent(const CStringHash hashEventType, const CStringHash hashEventCommand, double fEventTime = 0.0f);
	//set event params
	int AddArgUINT32(UINT32 val);
	int AddArgINT32(INT32 val);
	int AddArgFloat(float val);
	int AddArgBool(bool val);
	int AddArgVoidP(void* val);
	int AddArgString(const WCHAR* strVal);

	int AddNamedArgUINT32(const WCHAR* argName, UINT32 val);
	int AddNamedArgINT32(const WCHAR* argName, INT32 val);
	int AddNamedArgFloat(const WCHAR* argName, float val);
	int AddNamedArgBool(const WCHAR* argName, UINT32 val);
	int AddNamedArgVoidP(const WCHAR* argName, void* val);
	int AddNamedArgString(const WCHAR* argName, const WCHAR* strVal);

	CVariantComplex* GetArgumentByName(const WCHAR* argName);
};


///--- Event Listener Interface ---
//clasele care vor avea acces la events vor fi derivate din IEventListener
class IEventListener {
public:
	explicit IEventListener() {}
	virtual ~IEventListener() {}
	//for debug purposes
	virtual char const * GetListenerName(void) = 0;

	//RETURNS: consumed event? true - event consumed, false - event can propagate
	virtual bool HandleEvent( CEvent &nEvent ) = 0
	{
		//must be implemented
	}
};

///--- Event Manager ---
//minim 2 cozi
#define K_EVENTMGR_QUEUES 2

class CEventManager {
private:
	//TODO: Sa sorteze eventurile dupa timp si prioritate (daca o fi cazul intr-un engine mai avansat)
	//lista de listeners si tipul de event ascultat de fiecare
	CGrowableArray<IEventListener*> m_eventListeners;  //list of event listeners
	CGrowableArray<CStringHash*> m_eventListenersTypes; //event type pt fiecare listener de deasupra

	CGrowableArray<CEvent*> m_eventQueues[K_EVENTMGR_QUEUES]; //cozile de events
	int m_activeQueue;	//coada activa
public:
	CEventManager();
	~CEventManager();

	bool AddListener(IEventListener* pEventListener, const CStringHash hashEventType);
	bool AddListener(IEventListener* pEventListener, const WCHAR* strEventType);
	//Pune un event in coada. 
	//Returns: true daca a fost adaugat si false daca nu
	bool QueueEvent(CEvent* inEvent);
	//Activeaza un Event imediat. 
	//Returns: true daca a fost consumat si false daca nu
	bool TriggerEvent(CEvent* inEvent);

	void Update(float dTime, float fTimeLine);
};

//declar singletonul - vom avea mereu o singura clasa de mesaje
CEventManager& UTGetEventManager();
