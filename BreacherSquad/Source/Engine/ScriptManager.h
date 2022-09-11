#pragma once


//numar maxim de argumente pt o instructiune de script
#define K_MAX_SCRIPT_INSTRUCTION_ARGS 10

//fiecare instructiune are un nume (si hash) si un numar de args cu nume (sau nu) fiecare
class CScriptInstruction {
public:
	CStringHash		m_instruction;
	CVariantMap		m_arrArgs; //instruction arguments
	//create event
	CScriptInstruction(const WCHAR* strInstruction)
	{
		m_instruction.Init(strInstruction);
		m_arrArgs.Clear();
	}

	FORCEINLINE CVariant* GetArgument(WCHAR* strArgName)
	{
		return &m_arrArgs[strArgName];
	}
	/*
	FORCEINLINE CVariant* GetArgument(UINT32 dwArgNameHash)
	{
		for ( auto & element : m_arrArgs.m_variants )
		{
			if(element.second.
		}
		return &m_arrArgs.GetVariantByNameHash(dwArgNameHash);
	}
	*/
};

///--- Script Definition ---
//In aceasta structura se incarca scripturile din XML-uri
class CScriptDeclaration
{
public:
	CStringHash	m_scriptName; //numele scriptului incarcat aici
	CArray<CScriptInstruction*> m_instructions;	//instructiunile scriptului

	/*!
	 * \brief Finds the index of the LABEL instruction with the specified name
	 * \param shLabelName - name of label
	 * \returns instruction index or -1 if label not found
	 */
	int GetLabelInstrIndex(CStringHash shLabelName);
};

///--- Script Instance ---
//cate locatii de memorie are scriptul?
#define K_SCRIPT_MEMORY_SLOTS	5
//script status
#define K_SCRIPT_STATUS_NOTINITIALIZED -1
#define K_SCRIPT_STATUS_RUNNING 0
#define K_SCRIPT_STATUS_SUSPENDED 1
#define K_SCRIPT_STATUS_ENDED 2

//ExecuteScript return values
#define K_EXECUTE_ERROR -1
#define K_EXECUTE_FINISHED 0
#define K_EXECUTE_SUSPENDED 1

class CScript 
{
private:
	UINT32		UID; //id unic
public:
	double		m_fStartTime; //script start time

	INT32	m_scriptDeclIDX; //index catre script declaration din colectia de scripturi
	INT32	m_currentInstruction; //instructiunea curenta in script declaration
	INT32	m_status;	//status script
	UINT32	m_executorUID; //ID unic al obiectului care a chemat scriptul
	float	m_fSuspendedTimer; //timer care creste cat timp scriptul este suspended

	//in memoria locala se vor inregistra variabile cu nume si valoare. Vor functiona si ca flaguri pr instructiuni de genul WAIT_FOR_LOCALMEM("nume", valoare)
	//flagurile(variabilele) vor putea fi setate si din afara scriptului pentru a semnaliza scriptul sa continue
	CVariantMap m_localMemory;  //sloturi locale de memorie pentru fiecare script

	FORCEINLINE const UINT32 GetUID() {return UID;}
	
	CScript():
	m_scriptDeclIDX(-1),
	m_currentInstruction(0),
	m_status(K_SCRIPT_STATUS_NOTINITIALIZED),
	UID(GenerateUID()), m_fSuspendedTimer(0.0f),
	m_executorUID(0)
	{
	}
	//sterge elementele alocate dinamic (memoria)
	~CScript();


};

///--- Script Processor Interface ---
//clasele care vor procesa instructiuni de script se inregistreaza implementand interfata asta
class IScriptable {
public:
	explicit IScriptable() {}
	virtual ~IScriptable() {}
	//for debug purposes
	virtual char const * GetScriptProcessorName(void) = 0;

	//RETURNS: consumed instruction? true - consumed, false - can propagate
	virtual bool ProcessScriptInstruction(CScriptInstruction *instr, UINT32 executorUID, UINT32 scriptUID) = 0;
	//RETURNS: consumed event - if consumed don't pass it on to the rest of the listeners chain
	virtual bool OnScriptFinished(UINT32 executorUID, UINT32 scriptUID, CVariantMap * pCollScriptVars) = 0;
};

///--- Script Manager ---
class CScriptManager {
private:
	double		m_fTimeline; //game's timeline

	CArray<CScriptDeclaration*> m_scriptDeclarations;  //list of loaded scripts
	CArray<CScript*> m_activeScripts; //scripturile care ruleaza
	//executa imediat scriptul dat ca pointer
	//returns: 0 - script finished/error, UID-script suspended
	int ExecuteScript(CScript* ns);
	//array de procesoare de instructiuni (celelalte clase care primesc instructiuni din script)
	CArray<IScriptable*>	m_arrProcessors;
	/*
	* Replaces instruction args with local and global memory var values where needed
	* - use "*" before arg value to replace it with local variable value (eg: target="*strLocalVarName")
	*/
	void PreprocessScriptInstruction(CScript* pOwnerScript, CScriptInstruction* instr);
public:
	//Adauga clase de listenere care proceseaza instructiuni de script
	bool AddProcessor(IScriptable* pScriptable);
	//#TODO: ar trebui sa fie si un fisier de constante pe care sa il incarce automat la inceput? pentru unele chestii specifice scriptului
	CVariantMap m_globalMemory; //memorie globala folosita de toate scripturile pentru a se sincroniza intre ele
	
	CScriptManager():
	m_fTimeline(0.0f)
	{ 
	};

	~CScriptManager() 
	{ 
		Release(); 
	}
	
	//adauga scripturile din XML la colectia de declaratii existente
	HRESULT AddScripts(WCHAR* XMLpath); 
	
	//unload all scripts
	//void UnloadScripts();

	/*
	* Porneste un script si transmite UID catre cel care l-a pornit
	*@cvcInitialLocalMemory - colectie de variants pentru initializarea memoriei locale
	*/
	UINT32 StartScript(WCHAR* scriptName, const UINT32 executorUID = 0, CVariantMap* cvcInitialLocalMemory = null);
	UINT32 StartScript(UINT32 scriptNameHash, const UINT32 executorUID = 0, CVariantMap* cvcInitialLocalMemory = null);
	HRESULT StopScript(UINT32 scriptUID);

	HRESULT StopAllScripts();

	//reincarca un singur script
	//HRESULT ReloadScript(WCHAR* scriptName, WCHAR* xmlFilePath);
	
	/*
	* Gets script by UID from active running scripts
	*/
	CScript* GetActiveScript(UINT32 scriptUID);
	//memory functions
	void ClearGlobalMemory();
	void SetGlobalVar(CVariant* varValue);
	void SetGlobalVar_INT32(WCHAR * varName, INT32 varValue);

	void SetGlobalVars( CVariantMap & inputVariants );
	void SetLocalVars( UINT32 scriptUID, CVariantMap & inputVariants );
	
	void SetLocalVar(UINT32 scriptUID, CVariant* varValue);

	CVariant* GetGlobalVar(WCHAR* varName);
	CVariant* GetLocalVar(UINT32 scriptUID, WCHAR* varName);

	/* Gets the total number of running scripts */
	int GetRunningScriptsCount();

	void Update(float dTime);
	void Release();
};

// Scripts manager singleton
CScriptManager& __Scripts();
