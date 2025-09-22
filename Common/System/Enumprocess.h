#pragma once

#pragma warning (disable : 4996)

#include <tlhelp32.h>
#include <winsvc.h>
typedef BOOL (CALLBACK *PROCESSHANDLER)(LPCTSTR szExeName, LPCTSTR szExePath, DWORD dwProcessID, LPVOID pThis, bool &bStopEnum);

typedef BOOL (CALLBACK *PROCESSMODULEHANDLER)(DWORD dwProcessID, HANDLE hProcess, HMODULE hModule, LPCTSTR szModulePath, LPVOID pThis, bool &bStopEnum);

// Functions loaded from PSAPI
typedef BOOL (WINAPI *PFEnumProcesses)(DWORD * lpidProcess, DWORD cb, DWORD * cbNeeded);
typedef BOOL (WINAPI *PFEnumProcessModules)(HANDLE hProcess, HMODULE * lphModule, DWORD cb, LPDWORD lpcbNeeded);
typedef DWORD (WINAPI *PFGetModuleFileNameEx)(HANDLE hProcess, HMODULE hModule, LPTSTR lpFilename, DWORD nSize);
typedef DWORD (WINAPI *PFGetModuleBaseName)(HANDLE hProcess, HMODULE hModule, LPTSTR lpFilename, DWORD nSize);

//Functions loaded from Kernel32
typedef HANDLE (WINAPI *PFCreateToolhelp32Snapshot)(DWORD dwFlags, DWORD th32ProcessID);
typedef BOOL (WINAPI *PFProcess32First)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef BOOL (WINAPI *PFProcess32Next)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef BOOL (WINAPI *PFModule32First)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);
typedef BOOL (WINAPI *PFModule32Next)(HANDLE hSnapshot, LPMODULEENTRY32 lpme);

class CEnumProcess
{
public:
	CEnumProcess();
	virtual ~CEnumProcess();

	bool EnumRunningProcesses(PROCESSHANDLER lpProc, LPVOID pThis);
	bool IsProcessRunning(CString sProcName, bool bTerminateProcess, bool bIsFullPath = true, bool bTerminateTree = false);
	bool IsProcessRunning(DWORD dwSesionID, CString sProcName, bool bTerminateProcess, bool bIsFullPath = true, bool bTerminateTree = false);
	bool KillProcess(DWORD ProcessID);
	void RebootSystem(DWORD dwType = 0);
	bool KillExplorer();
	void RestoreExplorer();
	BOOL EnablePrivilege(LPCTSTR szPrivilege);
	DWORD GetProcessIDByName(CString csProcName);

private:
	HMODULE						PSAPI;
	PFEnumProcesses				FEnumProcesses;				// Pointer to EnumProcess
	PFEnumProcessModules		FEnumProcessModules;		// Pointer to EnumProcessModules
	PFGetModuleFileNameEx		FGetModuleFileNameEx;		// Pointer to GetModuleFileNameEx
	PFGetModuleBaseName         FGetModuleBaseName;		// Pointer to GetModuleFileName

	HMODULE						TOOLHELP;					//Handle to the module (Kernel32)
	PFCreateToolhelp32Snapshot	FCreateToolhelp32Snapshot;
	PFProcess32First			FProcess32First;
	PFProcess32Next				FProcess32Next;
	PFModule32First				FModule32First;
	PFModule32Next				FModule32Next;

	OSVERSIONINFO  osver;
	DWORD m_idExplorer;
	DWORD m_idSms;

	bool InitProcessDll();
	bool FreeProcessDll();
	bool GetProcessModule(DWORD dwPID, LPCTSTR pstrModule, LPMODULEENTRY32 lpMe32, DWORD cbMe32);
	BOOL EnableTokenPrivilege(HANDLE htok, LPCTSTR szPrivilege, TOKEN_PRIVILEGES *tpOld);
	bool HandleRequest(CString sProcName = CString(_T("")), bool bTerminateProcess = false, PROCESSHANDLER lpProc = NULL, LPVOID pThis = NULL, LPDWORD pdwProdID = NULL, bool bIsFullPath = true, bool bTerminateTree = false);
	bool HandleRequest(DWORD dwSessionID, CString sProcName = _T(""), bool bTerminateProcess = false, PROCESSHANDLER lpProc = NULL, LPVOID pThis = NULL, LPDWORD pdwProdID = NULL, bool bIsFullPath = true, bool bTerminateTree = false);
	HANDLE _GetProcID(CString csProcName, PROCESSENTRY32 pe32, HANDLE hSnapshot);

	bool SuspendProcess(HANDLE hProcess);
};
