#pragma once

#include <windows.h>
#include <atlstr.h>
#include "Constants.h"
#include "EPSConstants.h"

bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
void AddLogEntry(const TCHAR *sFormatString, const TCHAR *sEntry1 = 0, const TCHAR *sEntry2 = 0, bool isDateTime = true, DWORD dwLogLevel = 0);
