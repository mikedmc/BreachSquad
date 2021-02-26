///-----------------------------------------------------------
/// generic error class for return values (to replace HRESULT)
///-----------------------------------------------------------

#pragma once

#define OP_FAILED(er) ((((int)er.code)) < 0)
#define OP_SUCCESS(er) ((((int)er.code)) >= 0)
#define OPHRFAILED(hr) (hr < 0)

#ifndef V_OP_RETHR
	#define V_OP_RETHR(x)           { if (OP_FAILED(x)) { return E_FAIL; } }
#endif
#ifndef V_OP_RET
	#define V_OP_RET(x)           { OPRESULT opr = (x); if (OP_FAILED(opr)) { return opr; } }
#endif
#ifndef V_OP_HRTOOP
	#define V_OP_HRTOOP(hr)           { if (hr < 0) { return K_OP_FAILED; } }
#endif


///--- generic return values
enum eOpResult {
	K_OP_INVALIDARGS = -2,
	K_OP_FAILED = -1,
	K_OP_OK = 0,
	K_OP_OK_WARNING = 1,	// success but with log output (warnings)
};

enum eOpSeverity {
	K_SEVERITY_NONE = 0,
	K_SEVERITY_WARNING = 1,
	K_SEVERITY_CRITICAL = 2,
};

class OPRESULT {
public:
	eOpResult			code;
	eOpSeverity			severity;
	WCHAR				message[256]{};

	// converts OPRESULT to HRESULT
	operator HRESULT() const
	{
		return (code >= 0) ? S_OK : E_FAIL;
	}
	/*
	// converts HRESULT to OPRESULT
	OPRESULT(HRESULT hr, eOpSeverity eSeverity = K_SEVERITY_FORGET)
	{
		code = (hr >= 0) ? K_OP_OK : K_OP_FAILED;
		severity = eSeverity;
		wsprintf(message, L"HRESULT: %d", hr);

		LogResult();
	}
	*/

	OPRESULT(eOpResult eCode, eOpSeverity eSeverity = K_SEVERITY_NONE) 
	{
		code = eCode;
		severity = eSeverity;
		wcscpy_s(message, TEXT("No message"));

		LogResult();
	}

	OPRESULT(eOpResult eCode, const WCHAR * strMessage, eOpSeverity eSeverity = K_SEVERITY_NONE) 
	{
		code = eCode;
		severity = eSeverity;
		wcscpy_s(message, strMessage);

		LogResult();
	}

	OPRESULT(HRESULT hr, const WCHAR * strMessage, eOpSeverity eSeverity = K_SEVERITY_NONE)
	{
		code = (hr >= 0) ? K_OP_OK : K_OP_FAILED;
		severity = eSeverity;
		wsprintf(message, L"HRESULT[%d] %s", hr, strMessage);

		LogResult();
	}


	OPRESULT(eOpResult eCode, eOpSeverity eSeverity, WCHAR* szFormat, ...)
	{
		code = eCode;
		severity = eSeverity;

		va_list marker;
		va_start(marker, szFormat);
		wvsprintf(message, szFormat, marker);
		va_end(marker);

		LogResult();
	}


private:
	// after setting all vars call this to show the return op onscreen
	inline void LogResult()
	{
		// log nothing on OK codes
		if (code == K_OP_OK)
			return;

		int nErrSeverity = K_ERR_LOG;
		if (severity == K_SEVERITY_WARNING)
			nErrSeverity = K_ERR_WARNING;
		else if (severity == K_SEVERITY_CRITICAL)
			nErrSeverity = K_ERR_CRITICAL;

		ErrorBox(nErrSeverity, TEXT("OPRESULT[%d] %s"), code, message);
	}
};

