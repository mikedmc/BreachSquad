#include "dxstdafx.h"
#include <windows.h>
#include <windef.h>
#include <WinBase.h>
#include <dbghelp.h>
#include <stdio.h>

#pragma comment ( lib, "dbghelp.lib" )

BOOL CALLBACK MyMiniDumpCallback(PVOID /*pParam*/, const PMINIDUMP_CALLBACK_INPUT pInput, PMINIDUMP_CALLBACK_OUTPUT pOutput) 
{
	BOOL bRet = FALSE;

	// Check parameters 
	if( pInput == 0 ) 
		return FALSE;
	if( pOutput == 0 ) 
		return FALSE;

	// Process the callbacks 
	switch( pInput->CallbackType ) 
	{
		case IncludeModuleCallback: 
		{
			// Include the module into the dump 
			bRet = TRUE; 
		}
		break; 

		case IncludeThreadCallback: 
		{
			// Include the thread into the dump 
			bRet = TRUE; 
		}
		break; 

		case ModuleCallback: 
		{
			// Does the module have ModuleReferencedByMemory flag set ? 
			if( !(pOutput->ModuleWriteFlags & ModuleReferencedByMemory) ) 
			{
				// No, it does not - exclude it 
				wprintf( L"Excluding module: %s \n", pInput->Module.FullPath ); 
				pOutput->ModuleWriteFlags &= (~ModuleWriteModule); 
			}

			bRet = TRUE; 
		}
		break; 

		case ThreadCallback: 
		{
			// Include all thread information into the minidump 
			bRet = TRUE;  
		}
		break; 

		case ThreadExCallback: 
		{
			// Include this information 
			bRet = TRUE;  
		}
		break; 

		case MemoryCallback: 
		{
			// We do not include any information here -> return FALSE 
			bRet = FALSE; 
		}
		break; 

		case CancelCallback: 
			break; 
	}

	return bRet;
}

LONG WINAPI CreateMiniDump( struct _EXCEPTION_POINTERS *pep )
{
	// send analytics
	ANALYTICS_EVENT("crashdump", _VERSION_CHARSTR_, "CRC", UTApp().m_Settings.dev_unCurrentCRC);
	//force an update
	UTGetAnalytics().Update();

	// create dump file name with time-stamp
	SYSTEMTIME systime;
	GetLocalTime(&systime);
	WCHAR szDumpFileName[MAX_PATH];
	StringCchPrintf(szDumpFileName, MAX_PATH, L"CrashDump_%u_%u_%u_%u_%u", systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute);

	WCHAR szDumpFullFileName[MAX_PATH];
	StringCchPrintf(szDumpFullFileName, MAX_PATH, L"%s%s.dmp", UTApp().g_wszUserDataDir, szDumpFileName);
	// save log with dump name
	WCHAR szLogFullFileName[MAX_PATH];
	StringCchPrintf(szLogFullFileName, MAX_PATH, L"%s%s.log", UTApp().g_wszUserDataDir, szDumpFileName);
	
	WCHAR logPath[MAX_PATH];
	DebugGetLogFilePath(logPath, MAX_PATH);

	FILE * infile = _wfopen(logPath, L"rb");
	FILE * outfile = _wfopen(szLogFullFileName, L"wb");
	if ((infile) && (outfile))
	{
		filecopy(outfile, infile);
	}
	fclose(infile);
	fclose(outfile);

	// create crash dump file
	HANDLE hFile = CreateFileW( szDumpFullFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ); 
	if( ( hFile != NULL ) && ( hFile != INVALID_HANDLE_VALUE ) ) 
	{
		MINIDUMP_EXCEPTION_INFORMATION mdei; 
		mdei.ThreadId = GetCurrentThreadId(); 
		mdei.ExceptionPointers = pep; 
		mdei.ClientPointers = FALSE; 

		MINIDUMP_CALLBACK_INFORMATION mci; 
		mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MyMiniDumpCallback; 
		mci.CallbackParam = 0; 

		MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(MiniDumpWithIndirectlyReferencedMemory | MiniDumpScanMemory | MiniDumpWithDataSegs); 

		BOOL rv = MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, mdt, (pep != 0) ? &mdei : 0, 0, &mci ); 

		if( !rv ) 
		{
			MessageBoxW(NULL, L"MiniDumpWriteDump failed!", NULL, MB_OK | MB_ICONERROR);
		}

		CloseHandle( hFile ); 
	}
	else 
	{
		MessageBoxW(NULL, L"MiniDumpWriteDump failed!", NULL, MB_OK | MB_ICONERROR);
	}

	WCHAR szResult[MAX_PATH];
	StringCchPrintf(szResult, MAX_PATH, L"Ooops, game crashed!\r\nTo fix it, check our support forum or the Readme.txt\r\n\r\nAlso, please send this file to %s\r\n%s\n", K_GAME_EMAIL, szDumpFullFileName);
	MessageBoxW(NULL, szResult, NULL, MB_OK | MB_ICONERROR);

	return EXCEPTION_EXECUTE_HANDLER;
}

#ifndef _M_IX86
#error "The following code only works for x86!"
#endif
LPTOP_LEVEL_EXCEPTION_FILTER WINAPI MyDummySetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER /*lpTopLevelExceptionFilter*/)
{
	return NULL;
}

BOOL PreventSetUnhandledExceptionFilter()
{
	HMODULE hKernel32 = LoadLibrary(L"kernel32.dll");
	if (hKernel32 == NULL) 
		return FALSE;

	void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
	if (pOrgEntry == NULL)
		return FALSE;

	unsigned char newJump[100];
	DWORD dwOrgEntryAddr = (DWORD)pOrgEntry;
	dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far
	void *pNewFunc = &MyDummySetUnhandledExceptionFilter;
	DWORD dwNewEntryAddr = (DWORD)pNewFunc;
	DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;

	newJump[0] = 0xE9;  // JMP absolute
	memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));
	SIZE_T bytesWritten;
	BOOL bRet = WriteProcessMemory(GetCurrentProcess(),	pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten);
	return bRet;
}

bool InitMiniDumper()
{
	SetUnhandledExceptionFilter(CreateMiniDump);
    PreventSetUnhandledExceptionFilter(); // http://blog.kalmbachnet.de/?postid=75

	return TRUE;
}
