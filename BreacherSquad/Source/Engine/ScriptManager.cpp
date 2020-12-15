#include "dxstdafx.h"
#include "ScriptManager.h"

///--- SCRIPT ---
CScript::~CScript()
{
	m_localMemory.DeleteAll();

	m_executorUID = 0;
	m_status = K_SCRIPT_STATUS_NOTINITIALIZED;
	m_currentInstruction = 0;
}

///--- SCRIPT MANAGER ---
HRESULT CScriptManager::AddScripts(WCHAR* XMLpath)
{
    pugi::xml_document doc;
	if (!doc.load_file(XMLpath))
	{
		ErrorBox(K_ERR_WARNING, L"Unable to load scripts XML:%s\n", XMLpath);
		return(E_FAIL);
	}

	pugi::xml_node nodeCollection = doc.root().child(L"COLLECTION");
	for (pugi::xml_node scriptsnode = nodeCollection.child(L"script"); scriptsnode; scriptsnode = scriptsnode.next_sibling())
	{
		if(scriptsnode.attribute(L"name").empty())
		{
			ErrorBox(K_ERR_WARNING, L"Script has no name! Skipping.");
			continue;
		}

		CScriptDeclaration* sdc = new CScriptDeclaration();
		sdc->m_scriptName.Init(scriptsnode.attribute(L"name").value());

		for (pugi::xml_node scriptdata = scriptsnode.first_child(); scriptdata; scriptdata = scriptdata.next_sibling())
		{
			CScriptInstruction *si = new CScriptInstruction(scriptdata.name());

			for (pugi::xml_attribute_iterator ait = scriptdata.attributes_begin(); ait != scriptdata.attributes_end(); ++ait)
			{
				int rettype = GetTypeFromString(ait->value());
				switch(rettype)
				{
				case K_RETTYPE_EMPTY:
					{
						ErrorBox(K_ERR_WARNING, L"Empty parameter in script instruction[%s]!\nparamName=%s value=%s", si->m_instruction.text, ait->name(), ait->value());
					}
					break;
				case K_RETTYPE_INT:
					{
						const WCHAR *str = ait->value();
						WCHAR *stopstr;

						INT32 val = (INT32)wcstol(str, &stopstr, 10);
						si->m_arrArgs.SetNamedVarINT32(ait->name(), val);
					}
					break;
				case K_RETTYPE_FLOAT:
					{
						const WCHAR *str = ait->value();
						WCHAR *stopstr;

						float val = (float)wcstod(str, &stopstr);
						si->m_arrArgs.SetNamedVarFloat(ait->name(), val);
					}
					break;
				default:
					{
						const WCHAR* argname = ait->name();
						const WCHAR* argval = ait->value();
						WCHAR wargval[MAX_PATH];
						StringCchPrintf(wargval, MAX_PATH, argval);
						si->m_arrArgs.SetNamedVarString(argname, wargval);
					}
					break;
				}
			} //end attributes

			//add instruction to script
			sdc->m_instructions.Add(si);
		} //end instructions

		//add script to pool
		m_scriptDeclarations.Add(sdc);

	} //end scripts

	LOG(L"CScriptManager:: loaded: %s", XMLpath);

	return S_OK;
}

//RETURNS: scriptUID - id unic script daca e inca valabil, 0 daca s-a terminat deja scriptul si -1 daca scriptul a dat eroare
UINT32 CScriptManager::StartScript(UINT32 scriptNameHash, const UINT32 executorUID, CVariantCollection* cvcInitialLocalMemory)
{
	//exista scriptul cu acest nume?
	int declIDX = -1;
	for(int kk=0; kk<m_scriptDeclarations.GetSize(); kk++)
	{
		if(m_scriptDeclarations[kk]->m_scriptName.textHash == scriptNameHash)
		{
			declIDX = kk;
			break;
		}
	}
	//a gasit scriptul?
	if(declIDX < 0)
	{
		ErrorBox(K_ERR_WARNING, L"StartScript::Script not found: %x", scriptNameHash);
		return 0;  //trebuia -1
	}
	//creez script nou gol si ii setez datele importante
	CScript* ns = new CScript();
	//set data
	ns->m_executorUID = executorUID;
	ns->m_scriptDeclIDX = declIDX;  //index declaratie script
	ns->m_status = K_SCRIPT_STATUS_RUNNING;
	ns->m_fStartTime = m_fTimeline; //save start time
	ns->m_fSuspendedTimer = 0.0f; //reset suspended timer
	//initialize local memory
	if (cvcInitialLocalMemory == null)
		ns->m_localMemory.DeleteAll(); //clear local memory
	else
		ns->m_localMemory = *cvcInitialLocalMemory; //copy local memory from parameter

	//il adauga oricum, chiar daca termina executia imediat dedesubt. Cand termina pune status pe ended si il dezaloca managerul mai tarziu.
	m_activeScripts.Add(ns);

	//run script now
	int ret = ExecuteScript(ns);

	if(ret == K_EXECUTE_SUSPENDED)
		return ns->GetUID();
	//daca e eroare sau s-a terminat intoarce 0
	return 0;
}


HRESULT CScriptManager::StopScript(UINT32 scriptUID)
{
	for (int kk = 0; kk < m_activeScripts.GetSize(); kk++)
	{
		if(m_activeScripts[kk]->GetUID() == scriptUID)
		{
			CScript* nscript = m_activeScripts[kk];
			//anuntam toate procesoarele de script ca s-a terminat un script
			for (int ll = 0; ll < m_arrProcessors.GetSize(); ll++)
			{
				if (m_arrProcessors[ll]->OnScriptFinished(nscript->m_executorUID, nscript->GetUID(), &nscript->m_localMemory))
					break;
			}

			SAFE_DELETE(m_activeScripts[kk]);
			m_activeScripts.Remove(kk);
			return S_OK;
		}
	}
	return S_FALSE;
}

HRESULT CScriptManager::StopAllScripts()
{
	for (int kk = m_activeScripts.GetSize() - 1; kk >= 0; kk--)
	{
		CScript* nscript = m_activeScripts[kk];
		//anuntam toate procesoarele de script ca s-a terminat un script
		for (int ll = 0; ll < m_arrProcessors.GetSize(); ll++)
		{
			if (m_arrProcessors[ll]->OnScriptFinished(nscript->m_executorUID, nscript->GetUID(), &nscript->m_localMemory))
				break;
		}

		SAFE_DELETE(m_activeScripts[kk]);
		m_activeScripts.Remove(kk);
	}

	return S_OK;
}

//RETURNS: scriptUID - id unic script daca e inca valabil, 0 daca s-a terminat deja scriptul si -1 daca scriptul a dat eroare
UINT32 CScriptManager::StartScript(WCHAR* scriptName, const UINT32 executorUID, CVariantCollection* cvcInitialLocalMemory)
{
	UINT32 scrNameHash = FastHash(scriptName, wcslen(scriptName));
	//exista scriptul cu acest nume?
	return StartScript(scrNameHash, executorUID, cvcInitialLocalMemory);
}

void CScriptManager::Update(float dTime)
{
	//set local timeline
	m_fTimeline += dTime;
	//incearca sa execute scripturile active
	for (int kk = 0; kk < m_activeScripts.GetSize(); ++kk)
	{
		ExecuteScript(m_activeScripts[kk]);
	}
	//sterge scripturile terminate
	for (int kk = m_activeScripts.GetSize() - 1; kk >= 0; --kk)
	{
		CScript* nscript = m_activeScripts[kk];
		//keep suspended timer
		if (nscript->m_status == K_SCRIPT_STATUS_SUSPENDED)
			nscript->m_fSuspendedTimer += dTime;
		else
			nscript->m_fSuspendedTimer = 0.0f;
		//end very old, blocked scripts (more than 10 min)
		if((nscript->m_fStartTime - m_fTimeline > 600.0f) || (nscript->m_fSuspendedTimer > 600.0f))
		{
			nscript->m_status = K_SCRIPT_STATUS_ENDED;
			ErrorBox(K_ERR_WARNING, L"Hanging script found! %s ran for more than 10 minutes. Stopping it!", m_scriptDeclarations[m_activeScripts[kk]->m_scriptDeclIDX]->m_scriptName.text);
		}

		if(nscript->m_status == K_SCRIPT_STATUS_ENDED)
		{
			//anuntam toate procesoarele de script ca s-a terminat un script
			for (int ll = 0; ll < m_arrProcessors.GetSize(); ll++)
			{
				if (m_arrProcessors[ll]->OnScriptFinished(nscript->m_executorUID, nscript->GetUID(), &nscript->m_localMemory))
					break;
			}

			SAFE_DELETE(m_activeScripts[kk]);
			m_activeScripts.Remove(kk);
		}
	}
}

void CScriptManager::Release()
{
	//stop active scripts
	for (int kk = 0; kk < m_activeScripts.GetSize(); ++kk)
	{
		SAFE_DELETE(m_activeScripts[kk]);
	}
	m_activeScripts.RemoveAll();
	//release script declarations
	for(int kk=0; kk<m_scriptDeclarations.GetSize(); ++kk)
	{
		for(int ll=0; ll<m_scriptDeclarations[kk]->m_instructions.GetSize(); ll++)
		{
			SAFE_DELETE(m_scriptDeclarations[kk]->m_instructions[ll]);
		}
		m_scriptDeclarations[kk]->m_instructions.RemoveAll();

		SAFE_DELETE(m_scriptDeclarations[kk]);
	}
	m_scriptDeclarations.RemoveAll();
	//release global memory
	m_globalMemory.DeleteAll();

	LOG(L"CScriptManager:: Script Engine stopped.");
}


//RETURNS: script pointer or NULL if not found
CScript* CScriptManager::GetActiveScript(UINT32 scriptUID)
{
	for (int kk = 0; kk < m_activeScripts.GetSize(); kk++)
	{
		if(m_activeScripts[kk]->GetUID() == scriptUID)
			return m_activeScripts[kk];
	}
	return NULL;
}

void CScriptManager::ClearGlobalMemory()
{
	m_globalMemory.DeleteAll();
}

void CScriptManager::SetGlobalVar(CVariantComplex* varValue)
{
	if (varValue == null)
		return;

	m_globalMemory.AddVariant(*varValue);
}

//memory functions
void CScriptManager::SetGlobalVar_INT32(WCHAR * varName, INT32 varValue)
{
	m_globalMemory.SetNamedVarINT32(varName, varValue);
}

HRESULT CScriptManager::SetLocalVar(UINT32 scriptUID, CVariantComplex* varValue)
{
	CScript* as = GetActiveScript(scriptUID);
	if(!as)
	{
		ErrorBox(K_ERR_WARNING, L"ScriptManager::SetLocalVar->Active script not found! (UID: %d)", scriptUID);
		return S_FALSE;
	}

	as->m_localMemory.AddVariant(*varValue);

	return S_OK;
}

CVariantComplex* CScriptManager::GetGlobalVar(WCHAR* varName)
{
	return m_globalMemory.GetVariantByName(varName);
}

CVariantComplex* CScriptManager::GetLocalVar(UINT32 scriptUID, WCHAR* varName)
{
	CScript* as = GetActiveScript(scriptUID);
	if(!as)
	{
		ErrorBox(K_ERR_WARNING, L"ScriptManager::GetLocalVar->Active script not fond! (UID: %d)", scriptUID);
		return NULL;
	}

	return as->m_localMemory.GetVariantByName(varName);
}


int CScriptManager::GetRunningScriptsCount()
{
	return m_activeScripts.GetSize();
}

///--- SCRIPT INSTRUCTIONS - prehashed ---
static UINT32 instr_DEBUG_PRINT = FastHash(L"DEBUG_PRINT");
static UINT32 instr_SET_GLOBAL_VAR = FastHash(L"SET_GLOBAL_VAR");
static UINT32 instr_SET_LOCAL_VAR = FastHash(L"SET_LOCAL_VAR");
static UINT32 instr_MATH_LOCAL_VAR_MUL = FastHash(L"MATH_LOCAL_VAR_MUL");;//params: var - name = "numeVarLocala" factor = "3.0"
static UINT32 instr_RAND_INT_GLOBAL = FastHash(L"RAND_INT_GLOBAL"); //params: var_name="varName" nMin="0" nMax="100"
static UINT32 instr_WAIT_LOCAL_VAR = FastHash(L"WAIT_LOCAL_VAR");
static UINT32 instr_WAIT_GLOBAL_VAR = FastHash(L"WAIT_GLOBAL_VAR");
static UINT32 instr_PLAY_SOUND = FastHash(L"PLAY_SOUND");
static UINT32 instr_STOP_SOUND = FastHash(L"STOP_SOUND");
static UINT32 instr_SLEEP_ASYNC = FastHash(L"SLEEP_ASYNC"); //params: duration="3.0"

static UINT32 instr_LABEL = FastHash(L"LABEL"); //params: name="LABEL_NAME"
static UINT32 instr_GOTO = FastHash(L"GOTO"); //params: label="LABEL_NAME"
static UINT32 instr_IF_EQUAL_GLOBAL = FastHash(L"IF_EQUAL_GLOBAL"); //params: var_name="varName" check_value="10" goto="LABEL_NAME" [else_goto="LABEL2_NAME"]
static UINT32 instr_IF_LOWER_GLOBAL = FastHash(L"IF_LOWER_GLOBAL"); //params: var_name="varName" check_value="10" goto="LABEL_NAME" [else_goto="LABEL2_NAME"]

///--- EXECUTE INSTRUCTIONS ---
//returns: 0 - script finished, <0 script error, >0 script suspended
//TODO: de revizuit partea de error handling. Ar trebui sa inchida scriptul sau sa treaca la linia urmatoare?
int CScriptManager::ExecuteScript(CScript* ns)
{
	if((ns == NULL) || (ns->m_status == K_SCRIPT_STATUS_NOTINITIALIZED) || (ns->m_status == K_SCRIPT_STATUS_ENDED))
		return 0;

	int retval = K_EXECUTE_FINISHED;
	bool executed = false;

	bool running = true;
	while (running)
	{
		CScriptDeclaration* scrd = m_scriptDeclarations[ns->m_scriptDeclIDX];
		//local variable for preprocessing the instruction
		CScriptInstruction instr = *scrd->m_instructions[ns->m_currentInstruction];
		//preprocess instruction (replaces local memory vars and global memory vars where needed)
		PreprocessScriptInstruction(ns, &instr);

		//execute current instruction
		executed = false;
		///--- GLOBAL INSTRUCTIONS ---
		if (instr.m_instruction.textHash == instr_DEBUG_PRINT)
		{
			for (int kk = 0; kk < instr.m_arrArgs.GetVariantCount(); kk++)
			{
				LOG(L"%s::%s\n", scrd->m_scriptName.text, instr.m_arrArgs[kk]->m_strArg.text);
			}
			executed = true;
		}
		else if (instr.m_instruction.textHash == instr_LABEL)
		{
			executed = true;
		}
		else if (instr.m_instruction.textHash == instr_GOTO)
		{
			CVariantComplex* label = instr.GetArgument(L"label");
			int labelIdx = -1;

			if ((label->m_type != CVariantComplex::K_ARGTYPE_NONE) && (!label->m_strArg.IsEmpty()))
			{
				labelIdx = scrd->GetLabelInstrIndex(label->m_strArg);
			}

			if (labelIdx >= 0)
			{
				ns->m_currentInstruction = labelIdx; //nu scad 1 pentru ca oricum se sare instructiunea label (mai jos se creste automat currentInstr)
				LOG(L"SCRIPT::GOTO - jumped to index %d label %s\n", ns->m_currentInstruction, label->m_strArg.text);
			}
			else
			{
				LOG(L"SCRIPT::GOTO - didn't find label %s\n", label->m_strArg.text);
			}
			executed = true;
		}
		else if (instr.m_instruction.textHash == instr_IF_EQUAL_GLOBAL)
		{
			CVariantComplex* var_name = instr.GetArgument(L"var_name");
			CVariantComplex* check_value = instr.GetArgument(L"check_value");
			CVariantComplex* label_goto = instr.GetArgument(L"goto");

			if ((var_name->m_type == CVariantComplex::K_ARGTYPE_NONE) || (check_value->m_type == CVariantComplex::K_ARGTYPE_NONE) || (label_goto->m_type == CVariantComplex::K_ARGTYPE_NONE))
			{
				LOG(L"SCRIPT::IF_EQUAL_GLOBAL - missing params!\n");
			}
			else
			{
				int labelIdx = scrd->GetLabelInstrIndex(label_goto->m_strArg);

				if (labelIdx >= 0)
				{
					//find global var
					CVariantComplex* global_var = GetGlobalVar(var_name->m_strArg.text);
					if (*global_var == *check_value)
					{
						ns->m_currentInstruction = labelIdx; //nu scad 1 pentru ca oricum se sare instructiunea label (mai jos se creste automat currentInstr)
						//LOG(L"SCRIPT::IF_EQUAL_GLOBAL - check passed->jumped to index %d label %s\n", ns->m_currentInstruction, label_goto->m_strArg.text);
					}
					//else
					//{
					//	LOG(L"SCRIPT::IF_EQUAL_GLOBAL - check failed->didn't jump to label %s\n", label_goto->m_strArg.text);
					//}
				}
				else
				{
					LOG(L"SCRIPT::IF_EQUAL_GLOBAL - didn't find label %s\n", label_goto->m_strArg.text);
				}
			}

			executed = true;
		}
		else if (instr.m_instruction.textHash == instr_RAND_INT_GLOBAL)
		{
			CVariantComplex* var_name = instr.GetArgument(L"var_name");
			CVariantComplex* vmin = instr.GetArgument(L"nMin");
			CVariantComplex* vmax = instr.GetArgument(L"nMax");

			if ((var_name->m_type == CVariantComplex::K_ARGTYPE_NONE) || (vmin->m_type != CVariantComplex::K_ARGTYPE_INT32) || (vmax->m_type != CVariantComplex::K_ARGTYPE_INT32))
			{
				LOG(L"SCRIPT::RAND_INT_GLOBAL - missing params or wrong types! [var_name, nMin, nMax]\n");
			}
			else
			{
				int randval = randint_range(vmin->m_asINT32, vmax->m_asINT32);
				SetGlobalVar_INT32(var_name->m_strArg.text, randval);
			}

			executed = true;
		}
		else if (instr.m_instruction.textHash == instr_IF_LOWER_GLOBAL)
		{
			CVariantComplex* var_name = instr.GetArgument(L"var_name");
			CVariantComplex* check_value = instr.GetArgument(L"check_value");
			CVariantComplex* label_goto = instr.GetArgument(L"goto");

			if ((var_name->m_type == CVariantComplex::K_ARGTYPE_NONE) || (check_value->m_type == CVariantComplex::K_ARGTYPE_NONE) || (label_goto->m_type == CVariantComplex::K_ARGTYPE_NONE))
			{
				LOG(L"SCRIPT::IF_LOWER_GLOBAL - missing params!\n");
			}
			else
			{
				int labelIdx = scrd->GetLabelInstrIndex(label_goto->m_strArg);

				if (labelIdx >= 0)
				{
					//find global var
					CVariantComplex* global_var = GetGlobalVar(var_name->m_strArg.text);
					float fVar1 = global_var->asFloat();
					float fVar2 = check_value->asFloat();
					if (fVar1 < fVar2)
					{
						ns->m_currentInstruction = labelIdx; //nu scad 1 pentru ca oricum se sare instructiunea label (mai jos se creste automat currentInstr)
						LOG(L"SCRIPT::IF_LOWER_GLOBAL - check passed->jumped to index %d label %s\n", ns->m_currentInstruction, label_goto->m_strArg.text);
					}
					else
					{
						LOG(L"SCRIPT::IF_LOWER_GLOBAL - check failed->didn't jump to label %s\n", label_goto->m_strArg.text);
					}
				}
				else
				{
					LOG(L"SCRIPT::IF_LOWER_GLOBAL - didn't find label %s\n", label_goto->m_strArg.text);
				}
			}

			executed = true;
		}
		//TODO: daca apar mai multe instructiuni de sunet ar trbeui procesate in procesor separat de sunet (cume cel din clevel)
		else if(instr.m_instruction.textHash == instr_PLAY_SOUND)
		{
			CVariantComplex* param1 = instr.GetArgument(L"ID");
			CVariantComplex* param2 = instr.GetArgument(L"looping");
			//check looping
			UINT32 sndflags = 0;
			if (param2 && (param2->m_asBool == true))
				sndflags = DSBPLAY_LOOPING;

			CEvent *nevent = new CEvent(CEventTypes::evtT_SOUND, CEventCommands::evtC_SOUND_PLAY_HASH);
			nevent->AddNamedArgUINT32(L"sndHash", param1->m_strArg.getHash());
			nevent->AddNamedArgUINT32(L"sndFlags", sndflags);
			UTGetEventManager().QueueEvent(nevent);

			executed = true;
		}
		else if (instr.m_instruction.textHash == instr_STOP_SOUND)
		{
			CVariantComplex* param1 = instr.GetArgument(L"ID");
			CVariantComplex* param2 = instr.GetArgument(L"fadeOut");
			bool bFadeOut = false;
			if (param2 && param2->m_asBool)
				bFadeOut = true;

			CEvent *nevent = new CEvent(CEventTypes::evtT_SOUND, CEventCommands::evtC_SOUND_STOP_HASH);
			nevent->AddNamedArgUINT32(L"sndHash", param1->m_strArg.getHash());
			nevent->AddNamedArgBool(L"fadeOut", bFadeOut);
			UTGetEventManager().QueueEvent(nevent);

			executed = true;
		}
		else if(instr.m_instruction.textHash == instr_SET_GLOBAL_VAR)
		{
			SetGlobalVar(instr.m_arrArgs.m_variants[0]);
			executed = true;
		}
		else if(instr.m_instruction.textHash == instr_SET_LOCAL_VAR)
		{
			SetLocalVar(ns->GetUID(), instr.m_arrArgs.m_variants[0]);
			executed = true;
		}
		else if (instr.m_instruction.textHash == instr_MATH_LOCAL_VAR_MUL)
		{
			CVariantComplex* param1 = instr.GetArgument(L"var-name");
			CVariantComplex* param2 = instr.GetArgument(L"factor");

			if ((param1 == null) || (param2 == null) || (param1->m_type != CVariantComplex::K_ARGTYPE_STRING))
			{
				LOG(L"SCRIPT::MATH_LOCAL_VAR_MUL - var-name or factor are empty or var-name isn't string");
			}
			else
			{
				float value = 0.0f;

				CVariantComplex* localVar = GetLocalVar(ns->GetUID(), param1->m_strArg.text);
				if (localVar != null)
				{
					value = localVar->asFloat();
				}
				
				float mulval = param2->asFloat();

				CVariantComplex* narg = new CVariantComplex();
				narg->Set_FLOAT(param1->m_strArg.text, value * mulval);
				SetLocalVar(ns->GetUID(), narg);
			}

			executed = true;
		}
		else if (instr.m_instruction.textHash == instr_SLEEP_ASYNC)
		{
			//cand intra prima data este running
			if (ns->m_status == K_SCRIPT_STATUS_RUNNING)
			{
				ns->m_status = K_SCRIPT_STATUS_SUSPENDED;
				ns->m_fSuspendedTimer = 0.0f;
				running = false;
				retval = K_EXECUTE_SUSPENDED;
			}
			else if (ns->m_status == K_SCRIPT_STATUS_SUSPENDED)
			{
				running = false;
				retval = K_EXECUTE_SUSPENDED;

				float fTimeWait = instr.m_arrArgs[0]->m_asFloat;

				if (ns->m_fSuspendedTimer >= fTimeWait)
				{
					ns->m_status = K_SCRIPT_STATUS_RUNNING;
					ns->m_fSuspendedTimer = 0.0f;
					running = true;
					retval = K_EXECUTE_FINISHED;
				}
			}

			executed = true;
		}
		else if (instr.m_instruction.textHash == instr_WAIT_LOCAL_VAR)
		{
			if(instr.m_arrArgs.GetVariantCount() > 0)
			{
				bool equal = true;
				CVariantComplex* lv = GetLocalVar(ns->GetUID(), instr.m_arrArgs[0]->m_name.text);
				if(lv == NULL) 
				{
					equal = false;
				}
				else
				{
					equal = false;
					if(*lv == *instr.m_arrArgs.m_variants[0])
						equal = true;
				}
				
				if(ns->m_status == K_SCRIPT_STATUS_RUNNING)
				{
					if(!equal)
					{
						ns->m_status = K_SCRIPT_STATUS_SUSPENDED;
						running = false;
						retval = K_EXECUTE_SUSPENDED;

						LOG(L"SCRIPT::WAIT_LOCAL_VAR %s", instr.m_arrArgs[0]->m_name.text);
					}
				}
				else if(ns->m_status == K_SCRIPT_STATUS_SUSPENDED)
				{
					running = false;
					retval = K_EXECUTE_SUSPENDED;
					if(equal)
					{
						ns->m_status = K_SCRIPT_STATUS_RUNNING;
						running = true;
						retval = K_EXECUTE_FINISHED;

						LOG(L"SCRIPT::WAIT_LOCAL_VAR %s FINISHED!", instr.m_arrArgs[0]->m_name.text);
					}
				}
			}
			else
			{
				ErrorBox(K_ERR_WARNING, L"WAIT_LOCAL_VAR has no arguments!");
				retval = K_EXECUTE_ERROR;
			}
			executed = true;
		}
		else if(instr.m_instruction.textHash == instr_WAIT_GLOBAL_VAR)
		{
			if(instr.m_arrArgs.GetVariantCount() > 0)
			{
				bool equal = true;
				CVariantComplex* lv = GetGlobalVar(instr.m_arrArgs[0]->m_name.text);
				if(lv == NULL) 
				{
					equal = false;
				}
				else
				{
					equal = false;
					if(*lv == *instr.m_arrArgs.m_variants[0])
						equal = true;
				}
				
				if(ns->m_status == K_SCRIPT_STATUS_RUNNING)
				{
					if(!equal)
					{
						ns->m_status = K_SCRIPT_STATUS_SUSPENDED;
						running = false;
						retval = K_EXECUTE_SUSPENDED;
					}
				}
				else if(ns->m_status == K_SCRIPT_STATUS_SUSPENDED)
				{
					running = false;
					retval = K_EXECUTE_SUSPENDED;
					if(equal)
					{
						ns->m_status = K_SCRIPT_STATUS_RUNNING;
						running = true;
						retval = K_EXECUTE_FINISHED;
					}
				}
			}
			else
			{
				ErrorBox(K_ERR_WARNING, L"WAIT_GLOBAL_VAR has no arguments!");
				retval = K_EXECUTE_ERROR;
			}
			executed = true;
		}
		
		///--- PROCESS INSTRUCTIONS IN OUTSIDE PROCESSORS ---
		if(!executed) //nu este o instructiune generica recunoscuta deci trebuie trimisa la celelalte procesoare
		{
			for (int kk = 0; kk < m_arrProcessors.GetSize(); kk++)
			{
				executed = m_arrProcessors[kk]->ProcessScriptInstruction(&instr, ns->m_executorUID, ns->GetUID());
				//daca am executat instructiunea iese din for
				if (executed)
				{
					//aici nu prea trebuie sa faca nimic in afara de break decat daca vrem sa afisam rezultatul
					break;
				}
			}
		}
		///--- UNRECOGNIZED INSTRUCTION ---
		if(!executed) //daca nu cunoaste o instructiune ii face log
		{
			ErrorBox(K_ERR_WARNING, L"Unknown script instruction! %s\n", instr.m_instruction.text);
			retval = K_EXECUTE_ERROR;
		}
		
		//trece la instructiunea urmatoare
		if(running)
		{
			ns->m_currentInstruction++;
			if(ns->m_currentInstruction >= m_scriptDeclarations[ns->m_scriptDeclIDX]->m_instructions.GetSize())
			{
				ns->m_status = K_SCRIPT_STATUS_ENDED;
				running = false;
				retval = K_EXECUTE_FINISHED; //script ended
			}
		}
	}

	//daca da eroare il opreste
	if(retval == K_EXECUTE_ERROR)
		ns->m_status = K_SCRIPT_STATUS_ENDED;

	return retval;
}

void CScriptManager::PreprocessScriptInstruction(CScript* pOwnerScript, CScriptInstruction* instr)
{
	if ((pOwnerScript == null) || (instr == null))
		return;
	for (int kk = 0; kk < instr->m_arrArgs.GetVariantCount(); kk++)
	{
		if (instr->m_arrArgs.m_variants[kk]->m_type != CVariantComplex::K_ARGTYPE_STRING)
			continue;

		CVariantComplex* cvar = instr->m_arrArgs.m_variants[kk];
		//daca primul caracter este * fac replace cu variabila locala cu numele respectiv
		if (cvar->m_strArg.text[0] == '*')
		{
			int varlen = wcslen(cvar->m_strArg.text);
			if (varlen > 1)
			{
				WCHAR sLocalVarName[MAX_PATH] = { 0 };
				memcpy(sLocalVarName, cvar->m_strArg.text + 1, varlen * sizeof(WCHAR));
				//get local memory variant
				CVariantComplex* cvLocal = pOwnerScript->m_localMemory.GetVariantByName(sLocalVarName);
				//copy value from that one
				cvar->CopyValueFrom(cvLocal);
			}
		}
		//TODO: simbolul # sa fie ca si * dar sa inlocuiasca cu variabila globala
	}
}

bool CScriptManager::AddProcessor(IScriptable * pScriptable)
{
	if (pScriptable == null)
		return false;

	m_arrProcessors.Add(pScriptable);
	return true;
}


///--- SCRIPT DECLARATIONS ---
int CScriptDeclaration::GetLabelInstrIndex(CStringHash shLabelName)
{
	if (!shLabelName.IsEmpty())
	{
		for (int kk = 0; kk < m_instructions.GetSize(); kk++)
		{
			//daca gasesc instructiune de tip LABEL verific numele
			if (m_instructions[kk]->m_instruction.textHash == instr_LABEL)
			{
				//am gasit labelul deci sar aici
				if (m_instructions[kk]->GetArgument(L"name")->m_strArg.textHash == shLabelName.textHash)
				{
					return kk;
				}
			}
		}
	}

	return -1;
}

///**************************************************************************************
/// Sigleton de acces
///**************************************************************************************

CScriptManager& UTGetScriptManager()
{
	static CScriptManager g_ScriptMgr;
	return g_ScriptMgr;
}

