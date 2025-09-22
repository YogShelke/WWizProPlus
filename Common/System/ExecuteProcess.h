/**********************************************************************************************************              	  Program Name          : ExecuteProcess.cpp
	  Description           : Wrapper class which executes the application úsing Explorer.
	  Author Name			: Ramkrushna Shelke                                                                          	  Date Of Creation      : 31st Aug 2014
	  Version No            : 1.0.0.8
	  Special Logic Used    : Need to execute the process under user context using service, we need to launch the process							  in current logged in user.

***********************************************************************************************************/

#pragma once

#ifndef EMPTY_STRING
#define EMPTY_STRING L""
#endif

class CExecuteProcess
{

public:
	CExecuteProcess(void);
	~CExecuteProcess(void);
	void RestoreEXE(CString csEXEName);
	BOOL StartProcessWithToken(CString csProcessPath, CString csCommandLineParam, 
		   CString csAccessProcessName, bool bWait = false);
	BOOL StartProcessWithTokenExplorerWait(CString csProcessPath, CString csCommandLineParam,
		CString csAccessProcessName, bool bWait = false); 
	BOOL StartProcessWithTokenExplorer(CString csProcessPath, DWORD dwSessionID, CString csCommandLineParam,
		CString csAccessProcessName, bool bWait = false);
	CString GetUserNamefromProcessID(unsigned long csProcessName);
	HANDLE GetExplorerProcessHandle(CString csAccessProcessName = _T("explorer.exe"));
	HANDLE GetExplorerProcessHandle(CString csAccessProcessName, DWORD dwSessionID);
};
