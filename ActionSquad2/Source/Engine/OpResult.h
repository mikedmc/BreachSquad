#pragma once

//#TODO: constructor with printf format for message

#define OP_FAILED(er) ((((int)er.code)) < 0)
#define OP_SUCCESS(er) ((((int)er.code)) >= 0)

///--- generic return values
enum eOpResult {
	K_OP_INVALIDARGS = -2,
	K_OP_FAILED = -1,
	K_OP_OK = 0,
};

enum eOpSeverity {
	K_OP_SEVERITY_FORGET = 0,
	K_OP_SEVERITY_LOG,
	K_OP_SEVERITY_WARNING,
	K_OP_SEVERITY_ERROR,
};

///-----------------------------------------------------------
/// generic error class for return values (to replace HRESULT)
///-----------------------------------------------------------
class OPRESULT {
public:
	eOpResult			code;
	eOpSeverity			severity;
	WCHAR				message[256];

	// converts OPRESULT to HRESULT
	operator HRESULT() const
	{
		return (code >= 0) ? S_OK : E_FAIL;
	}

	// converts HRESULT to OPRESULT
	OPRESULT(HRESULT hr, eOpSeverity eSeverity = K_OP_SEVERITY_FORGET)
	{
		code = (hr >= 0) ? K_OP_OK : K_OP_FAILED;
		severity = eSeverity;
		wsprintf(message, L"HRESULT: %d", hr);

		LogResult();
	}

	OPRESULT(eOpResult eCode, eOpSeverity eSeverity = K_OP_SEVERITY_FORGET) 
	{
		code = eCode;
		severity = eSeverity;
		wcscpy_s(message, TEXT("No message"));

		LogResult();
	}

	OPRESULT(eOpResult eCode, const WCHAR * strMessage, eOpSeverity eSeverity = K_OP_SEVERITY_FORGET) 
	{
		code = eCode;
		severity = eSeverity;
		wcscpy_s(message, strMessage);

		LogResult();
	}

private:
	// after setting all vars call this to show the return op onscreen
	void LogResult()
	{
		if (severity == K_OP_SEVERITY_LOG)
		{
			LOG(TEXT("OPRESULT[%d] %s"), code, message);
		}
		else if (severity == K_OP_SEVERITY_WARNING)
		{
			ErrorBox(K_ERR_WARNING, TEXT("OPRESULT[%d] %s"), code, message);
		}
		else if (severity == K_OP_SEVERITY_ERROR)
		{
			ErrorBox(K_ERR_CRITICAL, TEXT("OPRESULT[%d] %s"), code, message);
		}
	}
};

