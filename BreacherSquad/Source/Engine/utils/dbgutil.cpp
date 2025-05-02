#include "dxstdafx.h"

#include <stdarg.h>
#include <time.h>
#include "dbgutil.h"

void DebugPrintFnA(CHAR* szFormat, ...)
{
	CHAR szBuffer[2048];
	va_list marker;
	va_start(marker, szFormat);
	StringCchVPrintfA(szBuffer, ARRAY_SIZE(szBuffer), szFormat, marker);
	va_end(marker);
	OutputDebugStringA(szBuffer);
}

void DebugPrintFnW(WCHAR* szFormat, ...)
{
	WCHAR szBuffer[2048];
	va_list marker;
	va_start(marker, szFormat);
	StringCchVPrintf(szBuffer, ARRAY_SIZE(szBuffer), szFormat, marker);
	va_end(marker);
	OutputDebugString(szBuffer);
}


//pe debug apare message box, pe release face log
VOID ErrorBoxFnW(int nSeverity, const CHAR* strFile, DWORD dwLine, WCHAR* szFormat, ...)
{
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	WCHAR szBuffer[2048];
	va_list marker;
	va_start(marker, szFormat);
	StringCchVPrintf(szBuffer, ARRAY_SIZE(szBuffer), szFormat, marker);
	va_end(marker);

	WCHAR szLine[MAX_PATH];
	WCHAR strFileW[MAX_PATH];
	size_t cnt;
	mbstowcs_s(&cnt, strFileW, strFile, MAX_PATH);

	if(nSeverity == K_ERR_CRITICAL)
	{
		StringCchPrintfW(szLine, MAX_PATH, L"\nCRITICAL ERROR in %s line %d\n", strFileW, dwLine);
		StringCchCat(szBuffer, 2048, szLine);
		//logs error
		OS_PrintLog(szBuffer);

		StringCchPrintfW(szLine, MAX_PATH, L"\nDo you wish to DEBUG?\n YES - Debug, NO - Ignore, CANCEL - Exit Application\n");
		StringCchCat(szBuffer, 2048, szLine);

		int retval = MessageBox(NULL, szBuffer, L"Critical Error!", MB_YESNOCANCEL | MB_ICONERROR);
		if(retval == IDYES)
		{
			__asm int 3;
		} else if(retval == IDCANCEL)
		{
			PostQuitMessage(nSeverity);
		}																													   
	}
	else if(nSeverity == K_ERR_WARNING)
	{
		StringCchPrintfW(szLine, MAX_PATH, L"\nWARNING in %s line %d\n", strFileW, dwLine);
		StringCchCat(szBuffer, 2048, szLine);
		//logs error
		OS_PrintLog(szBuffer);

		StringCchPrintfW(szLine, MAX_PATH, L"\nDo you wish to DEBUG?\n YES - Debug, NO - Ignore\n");
		StringCchCat(szBuffer, 2048, szLine);

		int retval = MessageBox(NULL, szBuffer, L"Warning!", MB_YESNO | MB_ICONWARNING);
		if(retval == IDYES)
		{
			__asm int 3;
		}
	}
	else if(nSeverity == K_ERR_DEBUGOUT)
	{
		StringCchPrintfW(szLine, MAX_PATH, L"\n%s line %d\n", strFileW, dwLine);
		StringCchCat(szBuffer, 2048, szLine);
		//logs error

		OS_PrintLog(szBuffer);
	}
	else if (nSeverity == K_ERR_ONSCREEN)
	{
		StringCchPrintfW(szLine, MAX_PATH, L"\n\n%s line %d\n", strFileW, dwLine);
		StringCchCat(szBuffer, 2048, szLine);
		MessageBox(NULL, szBuffer, L"Info", MB_OK);	
		//logs error
		OS_PrintLog(szBuffer);
	}
#else
	WCHAR szBuffer[2048];
	va_list marker;
	va_start(marker, szFormat);
	StringCchVPrintf(szBuffer, ARRAY_SIZE(szBuffer), szFormat, marker);
	va_end(marker);

	WCHAR szLine[MAX_PATH];
	WCHAR strFileW[MAX_PATH];
	mbstowcs(strFileW, strFile, MAX_PATH);

	if(nSeverity == K_ERR_CRITICAL)
	{
		StringCchPrintfW(szLine, MAX_PATH, L"\nCRITICAL ERROR in %s line %d\n", strFileW, dwLine);
		StringCchCat(szBuffer, 2048, szLine);
		//logs error
		OS_PrintLog(szBuffer);

		StringCchPrintfW(szLine, MAX_PATH, L"\nThe Application will now exit!\n");
		StringCchCat(szBuffer, 2048, szLine);
		int retval = MessageBox(NULL, szBuffer, L"Critical Error!", MB_OK | MB_ICONERROR);

		PostQuitMessage(nSeverity);
	}
	else if (nSeverity == K_ERR_ONSCREEN)
	{
		StringCchPrintfW(szLine, MAX_PATH, L"\n%s line %d\n", strFileW, dwLine);
		StringCchCat(szBuffer, 2048, szLine);
		MessageBox(NULL, szBuffer, L"Info", MB_OK);
		//logs error
		OS_PrintLog(szBuffer);
	}
	else
	{
		StringCchPrintfW(szLine, MAX_PATH, L"\nWARNING in %s line %d\n", strFileW, dwLine);
		StringCchCat(szBuffer, 2048, szLine);
		//logs error
		OS_PrintLog(szBuffer);
	}
#endif

}

VOID LOG(WCHAR* szFormat, ...)
{
	WCHAR szBuffer[2048] = { 0 };
	va_list marker;
	va_start(marker, szFormat);
	StringCchVPrintf(szBuffer, ARRAY_SIZE(szBuffer), szFormat, marker);
	va_end(marker);

	szBuffer[wcslen(szBuffer)] = '\n';

	//prints on window
	OS_PrintLog(szBuffer);
}

VOID DEBUG_BREAK()
{
	__asm int 3;
}

VOID LOG(CHAR* szFormat, ...)
{
	CHAR szBuffer[2048] = { 0 };
	va_list marker;
	va_start(marker, szFormat);
	StringCchVPrintfA(szBuffer, ARRAY_SIZE(szBuffer), szFormat, marker);
	va_end(marker);

	szBuffer[strlen(szBuffer)] = '\n';

	//prints on window
	WCHAR wcsBuffer[2048] = { 0 };
	mbstowcs(wcsBuffer, szBuffer, 2048);
	OS_PrintLog(wcsBuffer);
}


VOID LOG_DBG(WCHAR* szFormat, ...)
{
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	WCHAR szBuffer[2048] = { 0 };
	va_list marker;
	va_start(marker, szFormat);
	StringCchVPrintf(szBuffer, ARRAY_SIZE(szBuffer), szFormat, marker);
	va_end(marker);

	szBuffer[wcslen(szBuffer)] = '\n';

	//prints on window
	OS_PrintLog(szBuffer);
#endif
}

VOID LOG_DBG(CHAR* szFormat, ...)
{
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	CHAR szBuffer[2048] = { 0 };
	va_list marker;
	va_start(marker, szFormat);
	StringCchVPrintfA(szBuffer, ARRAY_SIZE(szBuffer), szFormat, marker);
	va_end(marker);

	szBuffer[strlen(szBuffer)] = '\n';

	//prints on window
	WCHAR wcsBuffer[2048] = { 0 };
	mbstowcs(wcsBuffer, szBuffer, 2048);
	OS_PrintLog(wcsBuffer);
#endif
}

#define K_LOG_BUFF_SIZE 128
static int nBuffIdx = 0;
static WCHAR buffTemp[K_LOG_BUFF_SIZE][2048];
VOID LOG_DBG_BUFF(WCHAR* szFormat, ...)
{
#if defined(_DEBUG) || defined(DEBUG) || defined(ENABLE_DEVMODE_RELEASE)
	WCHAR szBuffer[2048] = { 0 };
	va_list marker;
	va_start(marker, szFormat);
	StringCchVPrintf(szBuffer, ARRAY_SIZE(szBuffer), szFormat, marker);
	va_end(marker);

	szBuffer[wcslen(szBuffer)] = '\n';
	
	wcscpy(buffTemp[nBuffIdx], szBuffer);
	nBuffIdx++;
	if (nBuffIdx >= K_LOG_BUFF_SIZE - 1)
	{
		LOG_DBG_BUFF_FLUSH();
	}
#endif
}

VOID LOG_DBG_BUFF_FLUSH()
{
	if (nBuffIdx > 0)
	{
		WCHAR szPath[MAX_PATH];
		StringCchPrintf(szPath, MAX_PATH, L"%serror.log", UTApp().g_wszUserDataDir);
		FILE* log = NULL;
		if ((log = _wfopen(szPath, L"at")) == NULL)
		{
			return;
		}

		for (int kk = 0; kk < nBuffIdx; kk++)
		{
			fwprintf(log, buffTemp[kk]);
		}

		fclose(log);

		nBuffIdx = 0;
	}
}


void DebugLogFnA(CHAR* szFormat, ...)
{
	WCHAR szPath[MAX_PATH];
	StringCchPrintf(szPath, MAX_PATH, L"%serror.log", UTApp().g_wszUserDataDir);
	FILE* log = NULL;
	if ((log = _wfopen(szPath, L"at")) == NULL)
	{
		return;
	}

	char szTime[9];
	_strtime_s(szTime);
	fprintf(log, "%s\t", szTime);
	va_list args;
	va_start(args, szFormat);
	vfprintf(log, szFormat, args);
	va_end(args);

	fclose(log);
}

void DebugLogFnW(WCHAR* szFormat, ...)
{
	WCHAR szPath[MAX_PATH];
	StringCchPrintf(szPath, MAX_PATH, L"%serror.log", UTApp().g_wszUserDataDir);
	FILE* log = NULL;
	if ((log = _wfopen(szPath, L"at")) == NULL)
	{
		return;
	}

	va_list args;
	va_start(args, szFormat);
	vfwprintf(log, szFormat, args);
	va_end(args);

	fclose(log);
}

VOID DebugLogClear()
{
	WCHAR szPath[MAX_PATH];
	StringCchPrintf(szPath, MAX_PATH, L"%serror.log", UTApp().g_wszUserDataDir);
	FILE* log = NULL;
	if ((log = _wfopen(szPath, L"wt")) == NULL)
	{
		return;
	}

	fclose(log);
}

VOID DebugGetLogFilePath(WCHAR* destStr, int maxLen)
{
	StringCchPrintf(destStr, maxLen, L"%serror.log", UTApp().g_wszUserDataDir);
}

VOID GetErrorMessageA(DWORD dwError, CHAR *lpszMsgBuf, DWORD dwMaxLen)
{
	FormatMessageA(
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPSTR)lpszMsgBuf,
		dwMaxLen,
		NULL);
}

VOID GetErrorMessageW(DWORD dwError, WCHAR *lpszMsgBuf, DWORD dwMaxLen)
{
	FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPWSTR)lpszMsgBuf,
		dwMaxLen,
		NULL);
}

void GetExePathA(CHAR szPath[], int nLen)
{
#ifdef UNICODE
	WCHAR szwPath[MAX_PATH];
	GetModuleFileName(NULL, szwPath, nLen);
	int nIdx = (int)wcslen(szwPath);
	while (--nIdx > 0 && szwPath[nIdx] != '\\' && szwPath[nIdx] != '/');
	szwPath[nIdx + 1] = '\0';
	wcstombs(szPath, szwPath, nLen);
#else
	GetModuleFileName(NULL, szPath, nLen);
	int nIdx = (int)wcslen(szPath);
	while (--nIdx > 0 && szPath[nIdx] != '\\' && szwPath[nIdx] != '/');
	szPath[nIdx + 1] = '\0';
#endif
}

void GetExePathW(WCHAR szwPath[], int nLen)
{
#ifdef UNICODE
	GetModuleFileName(NULL, szwPath, nLen);
	int nIdx = (int)wcslen(szwPath);
	while (--nIdx > 0 && szwPath[nIdx] != '\\' && szwPath[nIdx] != '/');
	szwPath[nIdx + 1] = '\0';
#else
	CHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, nLen);
	int nIdx = (int)wcslen(szPath);
	while (--nIdx > 0 && szPath[nIdx] != '\\' && szwPath[nIdx] != '/');
	szPath[nIdx + 1] = '\0';
	mbstowcs(szwPath, szPath, nLen);
#endif
}

VOID DebugCrash()
{
	*((int *)NULL) = 123;
}

void filecopy(FILE *dest, FILE *src)
{
	const int size = 16384;
	char buffer[size];

	while (!feof(src))
	{
		int n = fread(buffer, 1, size, src);
		fwrite(buffer, 1, n, dest);
	}

	fflush(dest);
}

///----------------------------------------------------------------------------------
/// PerfTimestamp class and accessor function
///----------------------------------------------------------------------------------

CPerfTimestamp& __PerfCounter()
{
	// Using an accessor function gives control of the construction order
	static CPerfTimestamp timer;
	return timer;
}

CPerfTimestamp::CPerfTimestamp()
{
	LARGE_INTEGER qwTicksPerSec;
	QueryPerformanceFrequency( &qwTicksPerSec );
	m_llQPFTicksPerSec = qwTicksPerSec.QuadPart;
}

LONGLONG CPerfTimestamp::GetTime()
{
	LARGE_INTEGER ticks;
	if ( !QueryPerformanceCounter( &ticks ) )
	{
		ErrorBox( K_ERR_WARNING, L"CPerfTimestamp::GetTime failed!" );
	}
	return ticks.QuadPart;
}

double CPerfTimestamp::GetPeriodInMS( LONGLONG start_time, LONGLONG end_time )
{
	return ( end_time - start_time ) / ( double ) m_llQPFTicksPerSec;
}
