#pragma once

extern bool g_bDebugVersion;
extern HWND m_hLstMsgs;

#if defined(_DEBUG) || defined(DEBUG)
	#define DebugPrintA	DebugPrintFnA
	#define DebugPrintW	DebugPrintFnW
	#define DebugLogA	DebugLogFnA
	#define DebugLogW	DebugLogFnW
#else
	#define DebugPrintA	sizeof
	#define DebugPrintW	sizeof
	#define DebugLogA	DebugLogFnA
	#define DebugLogW	DebugLogFnW
#endif

///--- ERROR BOX ---
#define K_ERR_ONSCREEN  -1
#define K_ERR_DEBUGOUT  -2
#define K_ERR_LOG  -2
#define K_ERR_WARNING  -3
#define K_ERR_CRITICAL -4

#define ErrorBox(nSeverity, ...) ErrorBoxFnW(nSeverity, __FILE__, __LINE__, __VA_ARGS__)
VOID	ErrorBoxFnW(int nSeverity, const CHAR* strFile, DWORD dwLine, WCHAR* szFormat, ...);

/*!
 * \brief Writes a message line to the log window and file
 */
VOID	LOG(WCHAR* szFormat, ...);
VOID	LOG(CHAR* pchFormat, ...);
VOID	LOG_DBG(WCHAR* szFormat, ...);
VOID	LOG_DBG(CHAR* pchFormat, ...);
VOID	LOG_DBG_BUFF(WCHAR* szFormat, ...);
VOID	LOG_DBG_BUFF_FLUSH();
//daca da fail baga mesaj de critical
#define VQUIT(x) { hr = x; if( FAILED(hr) ) { ErrorBoxFnW(K_ERR_CRITICAL, __FILE__, (DWORD)__LINE__, L"CRITICAL - FAILED(hr) HR=%x", hr ); } }
#define VRETURN_QUIT(x) { hr = x; if( FAILED(hr) ) { ErrorBoxFnW(K_ERR_CRITICAL, __FILE__, (DWORD)__LINE__, L"CRITICAL - FAILED(hr) HR=%x", hr ); return hr; } }
#define VWARN(x) { hr = x; if( FAILED(hr) ) { ErrorBoxFnW(K_ERR_WARNING, __FILE__, (DWORD)__LINE__, L"WARN - FAILED(hr) HR=%x", hr ); } }

///--------------------
VOID	DebugPrintFnA(CHAR* szFormat, ...);
VOID	DebugPrintFnW(WCHAR* szFormat, ...);
VOID	DebugLogFnA(CHAR* szFormat, ...);
VOID	DebugLogFnW(WCHAR* szFormat, ...);
VOID	DebugLogClear();
VOID	DebugGetLogFilePath(WCHAR* destStr, int maxLen);

VOID	GetErrorMessageA(DWORD dwError, CHAR *lpszMsgBuf, DWORD dwMaxLen);
VOID	GetErrorMessageW(DWORD dwError, WCHAR *lpszMsgBuf, DWORD dwMaxLen);
void	GetExePathA(CHAR szPath[], int nLen);
void	GetExePathW(WCHAR szPath[], int nLen);

/* It just crashes the game */
VOID DebugCrash();
void filecopy(FILE *dest, FILE *src);