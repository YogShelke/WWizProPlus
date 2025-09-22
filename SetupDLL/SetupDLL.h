// SetupDLL.h : main header file for the SetupDLL DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "XMessageBox.h"
#include "SetupDllConstants.h"
#include "WardwizLangManager.h"
#include "ISpyCommunicator.h"
#include "PipeConstants.h"
#include "WardWizDatabaseInterface.h"
#include "sqlite3.h"
#include "AVRegInfo.h"

#define SENDMSGINSTALLATIONDRIVER (WM_USER + 150)
#define SENDMSGREGISTEREDDRIVER (WM_USER + 250)
#define SENDMSGSTARTDRIVER (WM_USER + 350)
#define SENDMSGINSTALLCOMPLETED (WM_USER + 450)
#define SENDMSGCLOSETLB (WM_USER + 450)
#define HOSTNAME_SIZE   MAX_PATH

// CSetupDLLApp
// See SetupDLL.cpp for the implementation of this class
//

/*typedef struct _tagOtherAVList
{
	char szPublisherName[0x64];
	char szUninstallString[0xc8];
}STRUCTOTHERAVLIST;

typedef std::vector<STRUCTOTHERAVLIST> OTHERAVMAP;*/

class CSetupDLLApp : public CWinApp
{
public:
	CSetupDLLApp();

// Overrides
public:
	typedef  int(*SQLITE3_OPEN)(const char *filename, sqlite3 **ppDb);
	typedef  int(*SQLITE3_PREPARE)(sqlite3 *db, const char *zSql, int nByte, sqlite3_stmt **ppStmt, const char **pzTail);
	typedef  int(*SQLITE3_COLUMN_COUNT)(sqlite3_stmt*);
	typedef  int(*SQLITE3_STEP)(sqlite3_stmt*);
	typedef  int(*SQLITE3_COLUMN_TEXT)(sqlite3_stmt*, int iCol);
	typedef  int(*SQLITE3_CLOSE)(sqlite3 *);

	virtual BOOL InitInstance();
	CString GetModuleFilePath();
	bool CloseAllAplication(bool bIsReInstall = false);
	bool CloseAllAplicationForElite(bool bIsReInstall);
	bool GetCloseAction4OutlookIfRunning(bool bIsReInstall = false);
	bool StartStartUpApplications(CString csAppFolder);
	bool SetSelectedLanguage(LPTSTR szLanguageName);
	bool SetAttributesToFolder(CString path);
	bool SingleInstanceCheck();
	//Resolved Issue No. 24 While installing the same setup again it should be "Latest version of Wardwiz is already installed".
	//Niranjan Deshak. : 21/12/2014.
	bool ParseVersionString(int iDigits[4], CString& csVersion);
	int CompareVersions(int iVersion1[4], int iVersion2[4]);
	bool CheckNeedOfRestart();
	bool RestartNow();
	void UnregisterComDll(CString csAppPath);
	bool MoveFileEXtoRestartDlt();
	//Added function to show yesall message Box
	BOOL GetOptionsYesNOYESALLNOALLMessageBox(CString& strText, UINT *pnButton, UINT *pnDefButton, UINT *pnIconType,
		UINT *pnDontAsk, UINT *pnHelpId, CString& strCustomButtons, int  *pnTimeout, int  *pnDisabled, UINT * pnIdIcon);
	DWORD ShowYesAllNoAllMessagebox(CString cMessageContent);
	bool SendMsgToTroubleShooter(DWORD dwMsg);
	bool SendData2TLB(int iMessageInfo, DWORD dwMsg, bool bWait = false);
	bool StopDriverService(LPTSTR lpszDriverName);
	bool StartDriverService(LPTSTR lpszDriverName);
	BOOL InitializeCOMSecurity();
	BOOL CreateWardWizRestorePoint();
	bool StartEPSStartUpApplications(CString csAppFolder);
	void CreateDatabaseforElite();
	INT64 InsertDataToTable(const char* szQuery);
	bool CSetupDLLApp::WriteXMLValIntoINI(LPCTSTR NodeName, LPCTSTR NodeVal);
	static bool GetLocalName(LPTSTR strName, UINT nSize);   // GetLocalName
	/*DWORD GetOtherAVDetails(LPCTSTR lpszSubKey);
	DWORD GetOtherAVDetails4Wow64(LPCTSTR lpszSubKey);
	void GetRegOtherAVValues();*/
	bool LaunchProcessThrCommandLine(LPTSTR pszSqlServerTempPath, LPTSTR pszConfigInstallSqlServerTempPath);
	bool LaunchInstallFilesProgressBar(LPTSTR pszWWIZInstalltionFilePath, int iPercentage);
	bool SendData2WWizInstaller(DWORD dwMessage, DWORD dwStatusState, LPTSTR pszWWIZInstalltionFilePath, int iPercentage, bool bWait = false);
	bool GetUninstallAvInfo(LPTSTR pszUninstallAvDetails);
	void XMLListScanningFinished();
	bool SendUninstallAvData2WWizInstaller(DWORD dwMessage, DWORD dwStatusState, LPTSTR pszUninstallAvDetails, bool bWait = false);
	bool PostData2WWizInstaller(DWORD dwMessage, DWORD dwStatusState, LPTSTR pszWWIZInstalltionFilePath, int iPercentage, bool bWait = false);
	bool SendUninstallAvData2WWizInstaller(DWORD dwMessage, DWORD dwStatusState, DWORD dwFinishFlag, bool bWait = false);
	void SendTempPathNExtractFinishedFlag(LPTSTR pszExtracttempfolderDetails);
	void SendInstallerLocationPath(LPTSTR pszExtracttempfolderDetails);
	bool StopInstallerDriversService(LPTSTR lpszDriverName);
	bool SendData2WWizInstaller4CloseMsg(DWORD dwMessage, DWORD dwStatusState, CString pszCloseAppMsg, bool bWait = false);
	bool InstallerthroughUnregisterComDll(LPTSTR GetShellExtdllPath);
	//bool CloseAllwwizAplication(LPTSTR GetReinstallMsgs, bool bIsReInstall = false);
	bool CloseAllwwizAplication(bool bIsReInstall = false);
	bool						LoadRequiredLibrary();
	bool InstallerCheckForPreviousVersion(LPTSTR GetFirstAppMsg, LPTSTR GetSecondAppMsg, LPCTSTR szAppPath, LPCTSTR iCurrentVersion, bool IsPatch);
	bool SendData2WWizInstallerCheckForPreviousVersionMsg(DWORD dwMessage, DWORD dwStatusState, LPTSTR pszFirstAppMsg, LPTSTR pszSecondAppMsg, LPTSTR pszAppPath, bool bWait=false);
	void InstallerThroughUnregisterComDll(CString csAppPath);
	void SetupInstallingFinished(bool bInstallSuccess);
	BOOL InstallWWizSetupThrCommandLine(LPTSTR pszWWIZSetupPath, LPTSTR pszCmdLine, bool bWait = false);
	bool GetDomainName(LPTSTR pszDomainName, DWORD dwSize);
	CString GetMachineID();
	DWORD GetCPUID(TCHAR *pszCPUID);
	bool GetVGAAdapterID(TCHAR *pszDispAdapterID);
	DWORD GetMACAddress(TCHAR *pMacAddress, LPTSTR lpszOldMacID = NULL);
	DWORD GetMotherBoardSerialNumber(TCHAR *psMotherBoardSerialNumber);
	DWORD GetMotherBoardSerialNumberSEH(TCHAR *psMotherBoardSerialNumber);
	DWORD RemoveCharsIfExists(TCHAR *pszBIOSSerialNumber, int iLen, int iSize, TCHAR chRemove);
	DWORD GetBIOSSerialNumber(TCHAR *pszBIOSSerialNumber);
	DWORD GetBIOSSerialNumberSEH(TCHAR *psMotherBoardSerialNumber);
	void ErrorDescription(HRESULT hr);
	CString GenerateGUIID();
	DWORD ExtractDate(TCHAR *pTime, TCHAR *newKey, LPSYSTEMTIME lpServerTime, LPDWORD lpdwServerDays);
	DWORD AddRegistrationDataInFile(LPBYTE lpData, DWORD dwSize);
	DWORD DecryptData(LPBYTE lpBuffer, DWORD dwSize);
	DWORD SpreadRegistrationFilesInSystem();
public:
	CString m_csAppPath;
	CString m_csProductName;
	bool m_bUninstallation;
	bool m_bIsEliteVersion;
	bool SendAllStrings(CString csStrings);
	std::vector<CString>		m_csVectInputStrings;
	CWardwizLangManager			m_objwardwizLangManager;
	CString						g_csRegKeyPath;
	CISpyCommunicator			m_objCom;
private:
	sqlite3							*m_pdbfile;
	HMODULE							m_hSQLiteDLL;
	SQLITE3_OPEN					m_pSQliteOpen;
	SQLITE3_PREPARE					m_pSQLitePrepare;
	SQLITE3_COLUMN_COUNT			m_pSQLiteColumnCount;
	SQLITE3_STEP					m_pSQLiteStep;
	SQLITE3_COLUMN_TEXT				m_pSQLiteColumnText;
	SQLITE3_CLOSE					m_pSQLiteClose;
	bool							m_bISDBLoaded;
	//OTHERAVMAP					m_vOtherAVList;
	DECLARE_MESSAGE_MAP()
};
