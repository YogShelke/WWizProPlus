/****************************************************
*  Program Name: WardWizUI.h
*  Author Name: Nitin Kolapkar
*  Date Of Creation: 27th May 2016
*  Version No: 2.0.0.1
****************************************************/
// WardWizUI.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "WardwizLangManager.h"
#include "EnumProcess.h"
#include "WardWizCrashHandler.h"
#include "iTINRegWrapper.h"
#include "ScannerContants.h"
#include "AVRegInfo.h"
// CWardWizUIApp:
// See WardWizUI.cpp for the implementation of this class
//
typedef DWORD(*UNZIPFILE)			(TCHAR *pZipFile, TCHAR *pUnzipPath, DWORD &dwUnzipCount);
typedef DWORD(*GETDAYSLEFT)		(DWORD);
typedef bool(*PERFORMREGISTRATION)	();
typedef DWORD(*GetByteStringHashFunc)(TCHAR *, int, TCHAR *);

class CWardWizUIApp : public CWinApp
{
public:
	virtual BOOL InitInstance();
	CWardWizUIApp();
	~CWardWizUIApp(void);

public:
	HMODULE					m_hResDLL;
	HMODULE					m_hZip;
	HANDLE					m_hMutexHandle;
	SCANLEVEL				m_eScanLevel;
	HMODULE					m_hRegistrationDLL;
	HMODULE					m_hRegisteredDataDLL;
	UNZIPFILE				m_UnzipFile;

	DWORD				m_dwCryptFileCount;
	CString				m_csDataCryptFilePath;
	CString				m_csTaskID;
	int					m_iDataOpr;
	bool				m_bCheckScan;
	bool				m_bStartUpScan;
	bool				m_bScanPage;
	bool				m_bReportPage;
	bool				m_bSchedScan;
	bool				m_bRunQuickScan;
	bool				m_bRunFullScan;
	bool				m_bRunCustomScan;
	bool				m_bRunRegOpt;
	bool				m_bAllowStartUpScan;
	bool				m_bRunLiveUpdate;
	bool				m_bDataCryptOpr;
	bool				m_bDaysLeft;
	bool				m_bIsEnDecFrmShellcmd;
	bool				m_bDialogsOpenInDataEnc;
	bool				m_bQuickScan;
	bool				m_bFullScan;
	bool				m_bCustomScan;
	bool				m_bAntirootscan;
	bool				m_bIsScanning;
	bool				m_bIsAntirootkitScanning;
	bool				m_bUpdates;
	bool				m_bOpenRegDlg;
	bool                m_bAllowStartUpTip;
	bool				m_bAllowDemoEdition;
	bool				m_bSchedScanShutDown;
	bool				m_bEPSQuickScanNoUI;
	bool				m_bEPSFullScanNoUI;
	bool				m_bEPSNoUIScan;
	bool				m_bIsQScanPageSwitched;
	bool				m_bIsCScanPageSwitched;
	bool				m_bIsFScanPageSwitched;
	bool				m_bIsARootScanPageSwitched;
	bool				m_bIsRegOptPageSwitched;
	bool				m_bIsDataEncDecPageSwitched;
	bool				m_bIsCustomScanUIReceptive;
	bool				m_bIsFullScanUIReceptive;
	bool				m_bIsQuickScanUIReceptive;
	bool				m_bIsARootScanUIReceptive;
	bool				m_bIsRegOptUIReceptive;
	bool				m_bIsDataEncDecUIReceptive;
	bool				m_bIsRecoverFileUIReceptive;
	bool				m_bIsRecoverFilePageSwitched;
	CWardwizLangManager			m_objwardwizLangManager;
	CITinRegWrapper				m_objReg;
	GETDAYSLEFT					m_GetDaysLeft;
	PERFORMREGISTRATION			m_PerformRegistration;
	PERFORMREGISTRATION			m_CloseRegistrationWindow;

	EXPIRYMSGBOX				m_lpLoadDoregistrationProc;
	DOREGISTRATION				m_lpLoadProductInformation;
	DOREGISTRATION				m_lpCloseProductInformationDlg;
	GETREGISTEREFOLDERDDATA		m_lpLoadEmail;
	AVACTIVATIONINFO			m_ActInfo;
	GETREGISTRATIONINFO			m_lpFnLoadProductInformation;

	DWORD						m_dwDaysLeft;
	DWORD						m_dwProductID;
	CString						m_csRegKeyPath;
	CString						m_AppPath;
	TCHAR						m_szRegKey[0x32];
	CString						m_csPageName;
	DWORD						m_dwScanType;
public:
	CEvent						m_objCompleteEvent;
	SCITER_VALUE				m_bRetval;
	SCITER_VALUE				m_iRetval;
	HINSTANCE					m_hInstLibrary;
	GetByteStringHashFunc		m_pGetbyteStrHash;
public:
	CString GetModuleFilePath();
	
	bool GetAppPath();
	bool SingleInstanceCheck();
	bool GetRegisteredUserInfo();
	bool ShowRestartMsgOnProductUpdate();
	bool SendDataCryptOpr2Gui(int iMessageInfo, DWORD dwDataOpr, CString csDataCryptFilePath);
	bool SendEmailPluginChange2Service(int iMessageInfo, DWORD dwType = 0, bool bEmailPluginWait = false);
	bool ReadRegistryEntryofStartUp();

	void DoRegistration();
	void LoadResourceDLL();
	void CheckScanLevel();
	void AddUserAndSystemInfoToLog();
	void StartDataCryptOpr(int iDataOpr, CString csDataCryptFilePath);
	void MessageBoxUI(CString csMessage, CString csMessageHeader, CString csMessageType = L"information");
	void StartScheduledScan();
	void StartScheduledScan4RegOpt(CString csOptionList);
	void LoadReqdLibrary();
	
	DWORD GetDaysLeft();
	DWORD CheckisWardWizRegistered();
	DWORD GetRegistrationDatafromFile();
	DWORD GetRegistrationDataFromRegistry();
	DWORD GetRegistrationDatafromFile(CString strUserRegFile);
	DWORD DecryptRegistryData(LPBYTE lpBuffer, DWORD dwSize);
	
	LPTSTR GetRegisteredEmailID();
	LPTSTR GetRegisteredUserInformation();
	LPCTSTR GetCurrentUserName();
	bool SendData2ComService(int iMessageInfo, DWORD dwType = 0, bool bWait = false);
	bool CheckForMachineID(const AVACTIVATIONINFO	&actInfo);
	bool SendData2StartScheduledScan(int iMessageInfo, DWORD dwScanType, bool IsShutDownSchedScan);
	bool SendData2StartScheduledScan4RegOpt(int iMessageInfo, DWORD dwScanType, bool IsShutDownSchedScan, CString csRegOptList);
	DWORD CalculateMD5(TCHAR *pString, int iStringlen, TCHAR *pFileHash);
	DECLARE_MESSAGE_MAP()
};

extern CWardWizUIApp theApp;