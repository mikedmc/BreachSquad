#include "dxstdafx.h"

void CLog::Write(const char *str, ...)
{
	CHAR szBuffer[2048];
	va_list marker;
	va_start(marker, str);
	StringCchVPrintfA(szBuffer, ARRAY_SIZE(szBuffer), str, marker);
	va_end(marker);
	
	OutputDebugStringA(szBuffer);

	WCHAR wBuffer[2048];
	mbstowcs(wBuffer, szBuffer, 2048);
	OS_PrintLog(wBuffer);
}

//--- HWNDs ---
HWND			g_logWindowParent = NULL;
HWND			g_logWindow = NULL;

LRESULT CALLBACK LogWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_CREATE:
		{
			RECT rc;
			GetClientRect(hwnd, &rc);

			g_logWindow = CreateWindowEx(0, L"EDIT", L"", // "LISTBOX"
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | /*LBS_HASSTRINGS*/ ES_MULTILINE | ES_READONLY,
				0, 0, rc.right, rc.bottom, hwnd, NULL, GetModuleHandle(NULL), NULL);

			HDWP hdwp = BeginDeferWindowPos(1);
			DeferWindowPos(hdwp, g_logWindow, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, 0);
			EndDeferWindowPos(hdwp);
		}
		return 0;

		case WM_COMMAND:
			return 0;

		case WM_CLOSE:
			DestroyWindow(g_logWindow);
			DestroyWindow(g_logWindowParent);
			UnregisterClass(L"LogWindow", GetModuleHandle(NULL));
			g_logWindow = g_logWindowParent = NULL;
			return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void OS_CreateLogWindow()
{
	WNDCLASSEX wc;
	memset(&wc, 0, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = 0;
	wc.lpfnWndProc = LogWindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_BTNSHADOW);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"LogWindow";
	wc.hIconSm = NULL;
	RegisterClassEx(&wc);

	g_logWindowParent = CreateWindowEx(WS_EX_CLIENTEDGE, L"LogWindow", L"LogWindow",
		WS_VISIBLE | WS_BORDER | WS_POPUPWINDOW | WS_OVERLAPPEDWINDOW,
		SW_SHOWDEFAULT, SW_SHOWDEFAULT, 1000, 700, NULL, NULL, GetModuleHandle(NULL), NULL);

	if (!g_logWindowParent)
	{
		ErrorBox(K_ERR_WARNING, L"Log window failed initializing with error: %d", GetLastError());
	}

	SendMessage(g_logWindow, EM_LIMITTEXT, (WPARAM)1048576, 0); // 1 mb of text should be enough

	ShowWindow(g_logWindowParent, SW_SHOWNOACTIVATE);

	SetForegroundWindow(DXUTGetHWND()); // don't let it come into foreground
}

void OS_PrintLog(const WCHAR* szLogLine, int logLineLength)
{
	//flush buffered logs 
	LOG_DBG_BUFF_FLUSH();

	if (UTApp().m_Settings.dev_bLogWriteToFile)
	{
		DebugLogW(L"%s", szLogLine);
	}
	
	if (UTApp().m_Settings.dev_bLogShowInDebugOutput)
	{
		DebugPrintW(L"LOG:%s", szLogLine);
		//OutputDebugString(szLogLine);
	}

	if (!g_logWindow || !szLogLine)
		return;

	static int textLength = 0;

	if (logLineLength < 0)
		logLineLength = wcslen(szLogLine);
	logLineLength += 1; // include NULL character

	//
	// minimum buffered edit box (only buffering non-EOL log lines)
	const int logBufferSize = 256;
	static WCHAR bufferedLog[logBufferSize] = { '\0' };
	static int bufferedLogLength = 0;

	bool bHasEOL = false;
	if (logLineLength >= 2 && szLogLine[logLineLength - 2] == '\n')
		bHasEOL = true;

	bool bFlush = false;
	if (logLineLength < logBufferSize)
	{
		memcpy(&bufferedLog[bufferedLogLength], szLogLine, sizeof(WCHAR) * (logLineLength - 1));
		bufferedLogLength += logLineLength - 1;

		if (bufferedLog[bufferedLogLength - 1] == '\n')
		{
			bufferedLog[bufferedLogLength - 1] = '\r';
			bufferedLog[bufferedLogLength] = '\n';
			bufferedLogLength++;
		}
	}
	else
	{
		bFlush = true;
	}

	if ((bufferedLogLength + logLineLength) >= logBufferSize && bufferedLogLength
		|| bHasEOL
		|| bFlush)
	{
		// buffer is full, print it before writing to it again
		bufferedLog[bufferedLogLength] = 0;
		SendMessage(g_logWindow, EM_SETSEL, (WPARAM)textLength, (LPARAM)textLength);
		SendMessage(g_logWindow, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)bufferedLog);
		textLength += bufferedLogLength + 1;
		bufferedLogLength = 0;
		bufferedLog[0] = 0;
	}

	if (bFlush)
	{
		// didn't fit in buffer, flush immediately
		SendMessage(g_logWindow, EM_SETSEL, (WPARAM)textLength, (LPARAM)textLength);
		SendMessage(g_logWindow, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)szLogLine);
		textLength += logLineLength;
	}
}
