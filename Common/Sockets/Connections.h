#pragma once
#include <Winsvc.h>
#include "EPSConstants.h"

class CConnection
{
public:
	CConnection();
	CConnection(CString csRemoteMachineIP, CString csRemoteMachineName, CString csPipeName, HWND hMainFrm, bool bAuditFlag);
	virtual ~CConnection();
	DWORD EstablishAdminConnection(LPTSTR lpszTaskID);
	bool CopyReqFilesToRemoteMachine(LPTSTR lpszTaskID, CString csMachineID);
	bool CopyFileToRemoteMachine(TCHAR* szInstalledPath, CString FileName, bool  bCheckAlreadyPresent = true, bool bDataBaseFile = false);
	bool StartRemoteService(CString csServiceName, bool bShowMessage = true);
	DWORD StopRemoteService(CString csServiceName, bool	bDelete = false);
	BOOL WriteinPipe(void*  lpBuffer, DWORD  nNumberOfBytesToWrite, DWORD* lpNumberOfBytesWritten);
	DWORD InstallClient(LPTSTR lpszTaskID, CString csMachineId);
	void ShowErrorMessage(CString str);
public:
	CString m_csRemoteMachineIP;
	CString m_csRemoteMachineName;
	CString m_csUserName;
	CString m_csPassword;
	CString	m_csPipeName;
	HANDLE m_hPipe;
	HWND	m_hMainFrm;
	bool	m_bAskPwd;
};

