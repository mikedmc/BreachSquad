#pragma once

class CLog
{
public:
	void Write(const char *str, ...);
};

void OS_CreateLogWindow();
void OS_PrintLog(const WCHAR* szLogLine, int logLineLength = -1);
