#include "dxstdafx.h"
#include "ActiveInterface.h"



IActiveInterface::IActiveInterface() : 
	ID(-1), targetID_ini(-1), bHidden(false), bSetHidden(false), bSkipRender(false), bAnimated(false),
	color(0xffffffff), color_ini(0xffffffff),
	bTouching(false), nTouchingUID(0), fTouchTimer(0.0f), fTouchDuration(0.0f), fTouchTimerReset(0.0f), 
	pTarget(null), bCanInteract(false), bHideInteractIcon(false), AIstate(K_AI_STATE_UNDEFINED), AItimerDecision(K_LVL_AI_DECISION_INTERVAL),
	nRunningScriptUID(0), AItargetUID(0), fTimelineAI(0.0f), 
	AItimer1(0.0f), AItimer2(0.0f), AIfvar1(0.0f), AIfvar2(0.0f), AIfvar3(0.0f), AIvar1(0), AIvar2(0), AIvarBool1(true), AIvarBool2(true),
	bPendingKill(false), pArea(nullptr), fHeight(K_WALL_HEIGHT_WORLD)
{
	UID = GenerateUID();

	pos = Vec3(0.0f, 0.0f, 0.0f);
	pos_ini = Vec3(0.0f, 0.0f, 0.0f);

	varAIparams.DeleteAll();

	AIvec1 = Vec2(0.0f, 0.0f);
}

IActiveInterface::~IActiveInterface()
{
	varAIparams.DeleteAll();
}

void IActiveInterface::LoadLogic(FILE* fl)
{
	byte n1b = OS_freadByte(fl);
	bCanInteract = (n1b & 0x1);
	bHideInteractIcon = (n1b & 0x2);
	//interact timer
	fTouchDuration = (float)OS_freadInt32(fl);

	//start hidden
	if (OS_freadByte(fl) != 0)
		bHidden = bSetHidden = true;

	CHAR strout[MAX_PATH];
	int targetid = OS_freadInt32(fl);
	targetID_ini = targetid; //save for later
	// read actions list for now, init later
	OS_freadString(fl, strout);
	shScriptActions.Init(strout);

	OS_freadString(fl, strout);
	if (strout[0] == 0) //empty
		AIstate = K_AI_STATE_UNDEFINED;
	else
		AIstate = (EAIstate)GetListIndexByNameHash(FastHash(strout), EAIstate_names, K_AI_STATES_CNT);

	int nAIparamsCnt = OS_freadByte(fl); //nr params
	if (nAIparamsCnt > 0)
	{
		for (int i = 0; i < nAIparamsCnt; i++)
		{
			CHAR varname[MAX_PATH] = { 0 };
			WCHAR wvarname[MAX_PATH] = { 0 };
			CHAR varval[MAX_PATH];
			WCHAR wvarval[MAX_PATH];

			OS_freadString(fl, varname);
			OS_freadString(fl, varval);
			size_t convnr;
			mbstowcs_s(&convnr, wvarname, varname, MAX_PATH);
			mbstowcs_s(&convnr, wvarval, varval, MAX_PATH);

			varAIparams.SetNamedVarAUTO(wvarname, wvarval);
		}
	}
}

void IActiveInterface::Touch(UINT32 touchingIActiveUID, float dTime, UINT32 overrideScriptHash /*= 0*/, bool bTouchTarget /*= true*/)
{
	if (nRunningScriptUID != 0)
	{
		//TODO: sa trimita mesaj de sunet gen "denied"
		return;
	}

	fTouchTimerReset = 0.0f;
	if (fabs(fTouchDuration) > 0.0f)
	{
		inc_limit(fTouchTimer, dTime, fabs(fTouchDuration));
		if (fTouchTimer < fabs(fTouchDuration))
			return;
	}

	if (bTouching)
	{
		return;
	}
	//override script?
	/*
	UINT32 scriptHash = script_hash.textHash;
	if (overrideScriptHash != 0)
		scriptHash = overrideScriptHash;

	bTouching = true;
	fTouchTimer = 0.0f; //reset touch timer
	nTouchingUID = touchingIActiveUID; //salvez uid al celui care face touch
	//1. are script si nu ruleaza deja alt script? run it
	if ((scriptHash != 0) && (nRunningScriptUID == 0))
	{
		//start script now
		nRunningScriptUID = UTGetScriptManager().StartScript(scriptHash, UID, &varAIparams);
	}
	//touch-ul si bTouching=false le face pe OnScriptFinished
	*/
}

void IActiveInterface::UpdateTouchTimerReset(float dTime)
{
	if (bCanInteract)
	{
		fTouchTimerReset += dTime;
		if (fTouchTimerReset >= K_LVL_TOUCH_TIMER_RESET_TIME)
		{
			fTouchTimer = 0.0f;
			fTouchTimerReset = K_LVL_TOUCH_TIMER_RESET_TIME;
		}
	}
}

void IActiveInterface::Kill()
{
	bPendingKill = true;
	// let it know he's out!
	EndPlay();
}
