#pragma once
#include "ScannerContants.h"
#include "AVRegInfo.h"
#include "WardwizLangManager.h"
#include "WardwizOSVersion.h"
#include "WardWizCrashHandler.h"
#include "iTINRegWrapper.h"

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

typedef DWORD (*GETDAYSLEFT)		(DWORD) ;
typedef bool (*PERFORMREGISTRATION)	() ;
typedef DWORD(*UNZIPFILE)			(TCHAR *pZipFile, TCHAR *pUnzipPath, DWORD &dwUnzipCount);

class CISpyGUIApp : public CWinApp
{
public:
	CISpyGUIApp();
	~CISpyGUIApp(void);

	public:
	virtual BOOL InitInstance();

public:
	void CreateFonts();
public:
	CFont				m_fontInnerDialogTitle;
	CFont				m_fontText;
	CFont				m_FontNonProtectMsg;
	CFont				m_fontTextNormal;
	CFont				m_fontHyperlink;

	CFont               m_fontWWTextMediumSize;             // For bigger font than normal text         
	CFont				m_fontWWTextNormal;					//For normal Text in the GUI
	CFont				m_fontWWTextTitle;					//For Registry Optimizer and Data Encryption
	CFont				m_fontWWTextSmallTitle;				//For all other titles
	CFont				m_fontWWTextSubTitleDescription;	//For the Description given below the titles
	CFont				m_fontWWTextSubTitle;				//For the Sub titles.
	CFont				m_fontWWLogoHeader ;//For the logo header

	CFont				m_FontWWStartUpFontTitle;			//For Did you know...?
	CFont				m_FontWWStartUpFontSubTitle;		//For Tip of the Day
	CFont				m_FontWWStartUpTips;				//For Tips in Rich edit Control and other buttons
	CFont				m_FontWWDaysLeftFont;				//For No of Days left (Digits)
	bool				m_bCheckScan;
	bool				m_bStartUpScan;
	bool				m_bRunQuickScan;
	bool				m_bRunFullScan;
	bool                m_bAllowStartUpTip;
	bool				m_bAllowStartUpScan;
	bool				m_bAllowDemoEdition;
	DWORD				m_dwDaysLeft;
	DWORD				m_dwProductID;
	bool				m_bEnableSound;
	DWORD				m_ExistingBytes[5];
	bool				m_bSettingFlag;
//	bool			    m_bRegistrationInProcessForboth; //issue no : 816 neha gharge 30/6/2014
	bool				m_bRunLiveUpdate;/* ISSUE: LiveUpdate Tray Notification NAME - NITIN K. TIME - 25th July 2014 */

	HMODULE				m_hRegistrationDLL ;
	HMODULE				m_hRegisteredDataDLL ;
	LPTSTR				szEmailId;
	TCHAR				m_szRegKey[16];
	AVACTIVATIONINFO	m_ActInfo ;

	GETDAYSLEFT			m_GetDaysLeft;
	PERFORMREGISTRATION	m_PerformRegistration;
	PERFORMREGISTRATION	m_CloseRegistrationWindow;
	EXPIRYMSGBOX		m_lpLoadDoregistrationProc;
	DOREGISTRATION		m_lpLoadProductInformation;
	DOREGISTRATION		m_lpCloseProductInformationDlg; 
	GETREGISTEREFOLDERDDATA m_lpLoadEmail;
	CWardwizLangManager	m_objwardwizLangManager;
	HMODULE					m_hResDLL;
	DWORD					m_dwSelectedLangID;
	DWORD					m_dwOSType;
	CWardWizOSversion		m_objOSVersionWrap;
	unsigned char			*m_pbyEncDecKey;
	unsigned char			*m_pbyEncDecSig;
	CITinRegWrapper			m_objReg;
	SCANLEVEL				m_eScanLevel;
	SETUPLOCID				m_eSetupLocId;
	bool					m_bOnCloseFromMainUI; 
	HANDLE					m_hMutexHandle;
	HANDLE					m_hMutexHandleDriverInstallation;
	CDialog					*m_SettingsReport;
	CDialog					*m_pSettingsScanTypeDlg;
	CDialog					*m_SettingsPassChange;
	DWORD					m_dwDataEncDec;
	bool					m_bIsFileInegrityInprogrs;
	bool					m_bDialogsOpenInDataEnc;
	bool			        m_bRegistrationInProcess;

	virtual int ExitInstance();
	CString GetModuleFilePath();
	bool SingleInstanceCheck();

	DWORD CheckiSPYAVRegistered( ) ;
	bool ShowEvaluationExpiredMsg( bool bShowAtStartUp = false);
	DWORD GetDaysLeft();
	bool ShowProductInformation();
	void DoRegistration();
	bool CloseRegistrationWindow();
	DWORD DecryptData( LPBYTE lpBuffer, DWORD dwSize );
	DWORD GetRegistrationDataFromRegistry( );
	LPTSTR GetRegisteredUserInfo( );
	LPTSTR GetRegisteredEmailID();
	void LoadResourceDLL();
	void CreateFontsFor(DWORD OSType,DWORD LanguageType);
	bool ReadRegistryEntryofStartUp();
	bool SendEmailPluginChange2Service(int iMessageInfo, DWORD dwType = 0,bool bEmailPluginWait = false); 
	DWORD GetRegistrationDatafromFile( );
	DWORD GetRegistrationDatafromFile( CString strUserRegFile ) ;
	/*	ISSUE NO - 653 NAME - NITIN K. TIME - 12th June 2014 */
	void OnBnClickedButtonUpdate();

	/* ISSUE NO - 697 NAME - NITIN K. TIME - 15th June 2014 */
	bool ReadSoundSettingFromRegistry();
	bool CreateRandomKeyFromFile(HANDLE hFile, DWORD dwFileSize);
	bool ReadKeyFromEncryptedFile(HANDLE hFile);
	DWORD DecryptRegistryData( LPBYTE lpBuffer, DWORD dwSize );
	bool IsFileAlreadyEncrypted(HANDLE hFile);
	DWORD GetTotalExistingBytes();
	static void CrashCallback(LPVOID lpvState);
	void CheckScanLevel();
	bool ShowRestartMsgOnProductUpdate();
	bool isScanning;
	bool m_bIsEnDecFrmShellcmd;
	//Added by Nitin K
	bool				m_bDataCryptOpr;
	int					m_iDataOpr;
	CString				m_csDataCryptFilePath;
	DWORD				m_dwCryptFileCount;
	void StartDataCryptOpr(int iDataOpr, CString csDataCryptFilePath);
	bool SendDataCryptOpr2Gui(int iMessageInfo, DWORD dwDataOpr, CString csDataCryptFilePath);
	//Adding User, computer name and OS details
	//Added by Vilas on 04 May 2015
	void AddUserAndSystemInfoToLog();
	bool CheckMutexOfDriverInstallation();
	bool m_bCrKeepOrg;

	bool m_bIsPopUpDisplayed;
	HANDLE					m_hCryptOprEvent;
	HANDLE					m_hUpdateOprEvent;
	//Function added to check and set load behaviour of Add-In
	bool CheckAndSetLoadBehaviourOfAddin();
	bool CheckForShowProdExpMsgBox();
	void ShowProdExpiredUnRegisteredMsgBox();
	bool				m_bShowProdExpMsgBox;
	bool LoadExtractDll();
	HMODULE					m_hZip;
	UNZIPFILE			UnzipFile;

	void CheckSetupLocId();

	DECLARE_MESSAGE_MAP()
};

extern CISpyGUIApp theApp;