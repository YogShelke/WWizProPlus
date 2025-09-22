/**********************************************************************************************************                     
	  Program Name          : WWizUninstSecondDlg.cpp
	  Description           : 
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 6th Feb 2015
	  Version No            : 1.9.0.0
	  Special Logic Used    : 
	  Modification Log      :           
***********************************************************************************************************/
#include "stdafx.h"
#include "WardWizUninstaller.h"
#include "WWizUninstSecondDlg.h"
#include "WardwizOSVersion.h"
#include "Enumprocess.h"
#include "iSpySrvMgmt.h"
#include "CSecure64.h"
#include "CScannerLoad.h"
#include <windows.h>
#include <stdio.h>
#include <strsafe.h>
#include "DriverConstants.h"

TCHAR	WARDWIZSERVICENAME[]				= L"WardwizComSrv" ;
TCHAR	WARDWIZDISPLAYNAME[]				= L"Wardwiz Communication Module" ;

TCHAR	WARDWIZUPDATESERVICENAME[]			= L"WardwizALUSrv" ;
TCHAR	WARDWIZUPDATEDISPLAYNAME[]			= L"Wardwiz Updater Module" ;
IMPLEMENT_DYNAMIC(CWWizUninstSecondDlg, CDialog)

#define TIMER_DELETE_STATUS			100

DWORD WINAPI UninstallThread(LPVOID lpvThreadParam);
/***************************************************************************
Function Name  : CWWizUninstSecondDlg
Description    : C'tor
Author Name    : Nitin K
SR_NO			 :
Date           : 06th Feb 2015
****************************************************************************/
CWWizUninstSecondDlg::CWWizUninstSecondDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CWWizUninstSecondDlg::IDD, pParent)
	,m_iTotalFileCount(0)
	,m_vFilePathLists(0)
	,m_vFolderPathLists(0)
	,m_bMoveFileExFlag(FALSE)
{
	m_csArrRegistryEntriesDel.RemoveAll();
	m_csArrRegistryEntriesDel.Add(L"AppFolder");
	m_csArrRegistryEntriesDel.Add(L"AppVersion");
	m_csArrRegistryEntriesDel.Add(L"DataBaseVersion");
	//if we uninstall the AV and keep all the settings and installed NON clam setup then clam scanner not launching.
	//Niranjan Deshak - 12/03/2015.
	m_csArrRegistryEntriesDel.Add(L"dwScanLevel");
	//Issue: During uninstallation if I select the checkboxes,and restart the machine,I am not able to install the setup the again.
	//REsolved By: Nitin K.
	m_csArrRegistryEntriesDel.Add(L"dwProductID");

}
/***************************************************************************
Function Name  : CWardWizUninstallerDlg
Description    : D'tor
Author Name    : Nitin K
SR_NO			 :
Date           : 06th Feb 2015
****************************************************************************/
CWWizUninstSecondDlg::~CWWizUninstSecondDlg()
{
}

/***************************************************************************
Function Name  : DoDataExchange
Description    : Called by the framework to exchange and validate dialog data.
Author Name    : 
SR_NO			 :
Date           : 06th Feb 2015
****************************************************************************/
void CWWizUninstSecondDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_BAR, m_pcDeleteProgress);
	//DDX_Control(pDX, IDC_STATIC_CONFIG_MSG, m_stConfigMessage);
	DDX_Control(pDX, IDC_BUTTON_MINIMIZE_TO_TASKBAR, m_btnMinimizeToTaskBar);
	DDX_Control(pDX, IDC_STATIC_UNINSTALLATION_INPROGRESS, m_stUninstallationInProgress);
	DDX_Control(pDX, IDC_STATIC_MINIMEZE_WINDOW_1, m_stMinimizeWindowText1);
	DDX_Control(pDX, IDC_STATIC_MINIMEZE_WINDOW_2, m_stMinimizeWindowText2);
	//DDX_Control(pDX, IDC_BUTTON_CLOSE, m_closeBtn);
	DDX_Control(pDX, IDC_STATIC_MINIMIZE, m_stImageMinimize);
}

/***************************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : 
*  SR_NO
*  Date           : 06th Feb 2015
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CWWizUninstSecondDlg, CDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_MINIMIZE_TO_TASKBAR, &CWWizUninstSecondDlg::OnBnClickedButtonMinimizeToTaskbar)
	ON_WM_QUERYDRAGICON()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CWWizUninstSecondDlg::OnBnClickedButtonClose)
END_MESSAGE_MAP()

/***************************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft Foundation Class Library dialog boxes
*  Author Name    : 
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
BOOL CWWizUninstSecondDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);

	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_SECOND_REGISTRATIONUI), _T("JPG")))
	{
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRATION_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
	}
	Draw();
	
	//m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));

	CRect rect;
	this->GetClientRect(rect);
	SetWindowPos(NULL, 0, 160, rect.Width(), rect.Height()-180 , SWP_NOREDRAW);
	
	//m_objWWizUninstThirdDlg.Create(IDD_UNINSTALL_THIRDPAGE, this);
	//m_objWWizUninstThirdDlg.ShowWindow(SW_HIDE);

	m_pcDeleteProgress.AlignText(DT_CENTER);
	//m_pcDeleteProgress.SetBarColor(RGB(0,0,255));
	m_pcDeleteProgress.SetBarColor(RGB(74, 167, 221));
	//m_prgScanProgress.SetBarColor(RGB(243,239,238));
	m_pcDeleteProgress.SetBkColor(RGB(243,239,238));
	m_pcDeleteProgress.SetShowPercent(true);

	m_pcDeleteProgress.SetRange(0, 100);
	m_pcDeleteProgress.SetPos(0);

	m_vFilePathLists.clear();
	m_vFolderPathLists.clear();
	

	/*m_closeBtn.SetSkin(theApp.m_hResDLL,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSEOVER,IDB_BITMAP_CLOSE,0,0,0,0,0);
	m_closeBtn.SetWindowPos(&wndTop,rect.Width()-40,0,26,17, SWP_NOREDRAW);
	m_closeBtn.ShowWindow(SW_SHOW);
	*/
	
	m_stUninstallationInProgress.SetWindowPos(&wndTop,rect.left +20,rect.top +25,300 ,25, SWP_NOREDRAW);
	//m_stUninstallationInProgress.SetWindowTextW(L"Un-Installation in progress"); 
	m_stUninstallationInProgress.SetWindowTextW(theApp.m_objwardwizLangManager.GetString( L"IDS_UNINSTALLER_TXT_UNINSTL_PROGRESS_TXT"));
	//m_stUninstallationInProgress.SetFont(&theApp.m_fontInnerDialogTitle);
	//Varada Ikhar, Date:4-3-2015, Issue: Uninstaller UI changed with Images. 
	m_stUninstallationInProgress.SetFont(&theApp.m_FontArialUninstall);


	m_pcDeleteProgress.SetWindowPos(&wndTop,rect.left +20,rect.top +65,rect.Width()-40  ,25, SWP_NOREDRAW);
	
	m_stMinimizeWindowText1.SetWindowPos(&wndTop,rect.left +100,rect.top +150,400 ,25, SWP_NOREDRAW);
	//m_stMinimizeWindowText1.SetWindowTextW(L"Feel free to minimize this window"); 
	m_stMinimizeWindowText1.SetWindowTextW(theApp.m_objwardwizLangManager.GetString( L"IDS_UNINSTALLER_TXT_UNINSTL_DLG_DES1"));
	//m_stMinimizeWindowText1.SetFont(&theApp.m_fontTextNormal);
	//Varada Ikhar, Date:4-3-2015, Issue: Uninstaller UI changed with Images. 
	m_stMinimizeWindowText1.SetFont(&theApp.m_FontArialFont);

	m_stMinimizeWindowText2.SetWindowPos(&wndTop,rect.left +100,rect.top +175,400 ,25, SWP_NOREDRAW);
	//m_stMinimizeWindowText2.SetWindowTextW(L"We'll let you know when uninstallation has finished"); 
	m_stMinimizeWindowText2.SetWindowTextW(theApp.m_objwardwizLangManager.GetString( L"IDS_UNINSTALLER_TXT_UNINSTL_DLG_DES2"));
	//m_stMinimizeWindowText2.SetFont(&theApp.m_fontTextNormal);
	//Varada Ikhar, Date:4-3-2015, Issue: Uninstaller UI changed with Images. 
	m_stMinimizeWindowText2.SetFont(&theApp.m_FontArialFont);

	//m_btnMinimizeToTaskBar.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	//Varada Ikhar, Date:4-3-2015, Issue: Uninstaller UI changed with Images. 
	m_btnMinimizeToTaskBar.SetSkin(theApp.m_hResDLL, IDB_BITMAP_MINIMIZE_TASKBAR_NORMAL, IDB_BITMAP_MINIMIZE_TASKBAR_NORMAL, IDB_BITMAP_MINIMIZE_TASKBAR_HOVER, IDB_BITMAP_MINIMIZE_TASKBAR_NORMAL, 0, 0, 0, 0, 0);
	m_btnMinimizeToTaskBar.SetWindowPos(&wndTop,rect.left+520,rect.top +155,167,36, SWP_NOREDRAW);
	//m_btnMinimizeToTaskBar.SetWindowTextW(L"Minimize to taskbar"); 
	m_btnMinimizeToTaskBar.SetWindowTextW(theApp.m_objwardwizLangManager.GetString( L"IDS_UNINSTALLER_TXT_UNINSTL_DLG_MINMIZ_TO_TSKBAR"));
	m_btnMinimizeToTaskBar.SetTextColorA(RGB(0,0,0),1,1);

	m_bmpImageMinimize = LoadBitmapW(AfxGetResourceHandle(), MAKEINTRESOURCE(IDB_BITMAP_UNINSTALL_MINIMIZE_BTN));
	m_stImageMinimize.SetBitmap(m_bmpImageMinimize);
	m_stImageMinimize.SetWindowPos(&wndTop, rect.left + 50, rect.top + 150, 33, 34, SWP_NOREDRAW);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/***************************************************************************************************
*  Function Name  : StartUninstallProgram
*  Description    : Starts the uninstallation process by creating UninstallThread
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CWWizUninstSecondDlg::StartUninstallProgram()
{
	theApp.m_bUninstallInProgress = TRUE;
	m_hThreadUninstall = ::CreateThread(NULL, 0, UninstallThread, (LPVOID) this, 0, NULL);
	Sleep(500);
	SetTimer(TIMER_DELETE_STATUS, 10, NULL);
}

/***************************************************************************************************
*  Function Name  : DeleteFiles
*  Description    : Deletes all the application files one by one
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
bool CWWizUninstSecondDlg :: DeleteFiles(std::vector<CString>	&vFilePathLists)
{
	try
	{
		AddLogEntry(L">>> CWWizUninstSecondDlg : DeleteFiles", 0, 0, true, ZEROLEVEL);
		CString csFileName = NULL;
		int size = static_cast<int>(vFilePathLists.size());
		while (!vFilePathLists.empty())
		{
			csFileName = vFilePathLists.back();
			if(PathFileExistsW(csFileName))
			{
				SetFileAttributes(csFileName,FILE_ATTRIBUTE_NORMAL);
				if(DeleteFile(csFileName))
				{
					
					m_iTotalDeletedFileCount++;
					Sleep(500);
				}
				else
				{
					DWORD err = GetLastError();
					m_iTotalDeletedFileCount++;
					m_bRestartReq = true;
					if(MoveFileEx(csFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
					{
						CString szMessage = NULL;
						szMessage.Format(L"### CWWizUninstSecondDlg::File to be deleted on Restart : %s", csFileName);
						AddLogEntry(szMessage, 0, 0, true, SECONDLEVEL);
						DWORD err1 = GetLastError();
					}
				}
			}
			vFilePathLists.pop_back();
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CuninstallDlg::DeleteFiles", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : DeleteFolder
*  Description    : Deletes all the application folder one by one
*  Author Name    : Nitin K. Kolapkar
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
bool CWWizUninstSecondDlg :: DeleteFolder(std::vector<CString>	&vFolderPathLists)
{
	try
	{
		AddLogEntry(L">>> CWWizUninstSecondDlg : DeleteFolder", 0, 0, true, ZEROLEVEL);
		CString csFolderName = NULL;
		int size = static_cast<int>(vFolderPathLists.size());
		while (!vFolderPathLists.empty())
		{
			csFolderName = vFolderPathLists.back();
			if(PathFileExistsW(csFolderName))
			{
				RemoveDirectory(csFolderName);
			}
			else
			{
				MoveFileEx(csFolderName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				CString szMessage = NULL;
				szMessage.Format(L"### CWWizUninstSecondDlg::Folder to be deleted on Restart : %s", csFolderName);
				AddLogEntry(szMessage, 0, 0, true, SECONDLEVEL);
			}
			vFolderPathLists.pop_back();
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CuninstallDlg::DeleteFiles", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************                    
*  Function Name  : EnumFolderToCountFiles                                                     
*  Description    : Enumrate all the files in folder to check whether folder is empty or not.
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 03 Feb 2015
*************************************************************************************************/
void CWWizUninstSecondDlg::EnumFolderToCountFiles( LPCTSTR pstr )
{
	try
	{
		CFileFind finder;
		// build a string with wildcards
		CString strWildcard(pstr);
		strWildcard += _T("\\*.*");

		// start working for files
		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			// if it's a directory, recursively search it 
			if (finder.IsDirectory())
			{
				CString csFilePath = finder.GetFilePath();
				SetFileAttributes(csFilePath,FILE_ATTRIBUTE_NORMAL);
				CString csFileName = finder.GetFileName();
				if( csFileName == "QUARANTINE") 
				{
					if( theApp.m_bQuarantine)
					{
						continue;
					}
					else
					{
						m_vFolderPathLists.push_back(csFilePath);
						m_bMoveFileExFlag = FALSE;
						EnumFolderToCountFiles(csFilePath );//, theApp.m_bQuarantine, theApp.m_bPassDbMngr);
					}
				}
				else if( csFileName == "WRDSETTINGS") 
				{
					SetFileAttributes(csFilePath,FILE_ATTRIBUTE_NORMAL);
					//MoveFileEx(csFileName, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
					m_bMoveFileExFlag = TRUE ;
					EnumFolderToCountFiles(csFilePath );
					MoveFileEx(csFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
					CString szMessage = NULL;
					szMessage.Format(L"### CWWizUninstSecondDlg::Folder to be deleted on Restart : %s", csFilePath);
					AddLogEntry(szMessage, 0, 0, true, SECONDLEVEL);
					m_bMoveFileExFlag = FALSE ;
					continue;
				}
				else
				{
					m_vFolderPathLists.push_back(csFilePath);
					m_bMoveFileExFlag = FALSE;
					EnumFolderToCountFiles(csFilePath );//, theApp.m_bQuarantine, theApp.m_bPassDbMngr);
				}
			}
			else
			{
				CString csFileName = finder.GetFileName();
				CString csFilePath = finder.GetFilePath();
				SetFileAttributes(csFilePath,FILE_ATTRIBUTE_NORMAL);
				if( csFileName == "WRDWIZEVALREG.DLL") 
				{
					if( theApp.m_bPassDbMngr )
					{
						continue;
					}
				}
				if( csFileName == "WRDWIZRESOURCE.DLL") 
				{
					MoveFileEx(csFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
					CString szMessage = NULL;
					szMessage.Format(L"### CWWizUninstSecondDlg::Folder to be deleted on Restart : %s", csFilePath);
					AddLogEntry(szMessage, 0, 0, true, SECONDLEVEL);
					continue;
				}
				if (csFileName == "WRDWIZSHELLEXT.DLL")
				{
					if (PathFileExists(csFilePath))
					{
						UnregisterComDll(csFilePath);
					}
				}
				if(m_bMoveFileExFlag)
				{
					MoveFileEx(csFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
					CString szMessage = NULL;
					szMessage.Format(L"### CWWizUninstSecondDlg::Folder to be deleted on Restart : %s", csFilePath);
					AddLogEntry(szMessage, 0, 0, true, SECONDLEVEL);
 					continue;
				}
				m_vFilePathLists.push_back(csFilePath);
				m_iTotalFileCount++;
			}
		}
		finder.Close();
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardWizScanner::EnumFolderToCountFiles", 0, 0, true, SECONDLEVEL);
	}
	
}


/***********************************************************************************************                    
*  Function Name  : DeleteRegistryKeys                                                     
*  Description    : Enumrate all the files in folder to check whether folder is empty or not.
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 03 Feb 2015
*************************************************************************************************/
bool CWWizUninstSecondDlg::DeleteRegistryKeys()
{
	LONG		lResult = 0x00 ;
	HKEY hKey = NULL;
	DWORD dwRegCount = 0x00;
	CString szSubkey = NULL;
	AddLogEntry(L">>> CWWizUninstSecondDlg : DeleteRegistryKeys", 0, 0, true, ZEROLEVEL);
	if(RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Wardwiz Antivirus"),&hKey)!= ERROR_SUCCESS)
	{
		AddLogEntry(L"### Unable to open registry key", 0, 0, true, SECONDLEVEL);
		return FALSE;
	}

	dwRegCount = static_cast<DWORD>( m_csArrRegistryEntriesDel.GetCount() );
	for (dwRegCount = 0; dwRegCount < static_cast<DWORD>(m_csArrRegistryEntriesDel.GetCount()); dwRegCount++)
	{
		szSubkey = L"";
		szSubkey = m_csArrRegistryEntriesDel.GetAt(dwRegCount);

		lResult = RegDeleteValue(hKey, szSubkey) ;
		if (lResult != ERROR_SUCCESS)
		{
			CString szMessage = NULL;
			szMessage.Format(L">>> CWWizUninstSecondDlg : DeleteRegistryKeys:: Unable to delete Registry key %s", szSubkey);
			AddLogEntry(szMessage, 0, 0, true, SECONDLEVEL);
			return FALSE;

		}
	}

	if(!theApp.m_bUserSettings)
	{
		if (theApp.m_dwProductID == 2)
		{
			lResult = RegDeleteKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Wardwiz Antivirus\\EmailScanSetting"));
			if (lResult == ERROR_SUCCESS)
			{
				AddLogEntry(L">>> successfully deleted key SOFTWARE\\Wardwiz Antivirus\\EmailScanSetting", 0, 0, true, FIRSTLEVEL);
			}
			else
			{
				AddLogEntry(L"### failed to delete key SOFTWARE\\Wardwiz Antivirus\\EmailScanSetting", 0, 0, true, SECONDLEVEL);
			}
		}

		lResult = RegDeleteKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Wardwiz Antivirus"));
		if (lResult == ERROR_SUCCESS)
		{
			AddLogEntry(L">>> successfully deleted key SOFTWARE\\Wardwiz Antivirus", 0, 0, true, FIRSTLEVEL);
		}
		else
		{
			AddLogEntry(L"### failed to delete key SOFTWARE\\Wardwiz Antivirus", 0, 0, true, SECONDLEVEL);
		}
	}
	

	lResult = RegDeleteKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{078ABE62-583D-43e6-96D6-5D092883DC82}_is1")) ;
	if (lResult == ERROR_SUCCESS)
	{
		AddLogEntry(L">>> successfully deleted key SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{078ABE62-583D-43e6-96D6-5D092883DC82}_is1", 0, 0, true, FIRSTLEVEL);
	}
	else
	{
		AddLogEntry(L"### failed to delet key SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\{078ABE62-583D-43e6-96D6-5D092883DC82}_is1 ", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***********************************************************************************************
*  Function Name  : UninstallThread
*  Description    : This thread stops the Driver protection and deletes the files
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 06 Feb 2015
*************************************************************************************************/
DWORD WINAPI UninstallThread(LPVOID lpvThreadParam)
{
	CWWizUninstSecondDlg *pThis = (CWWizUninstSecondDlg*)lpvThreadParam;
	if(!pThis)
	{
		return 0x00;
	}


	// Lalits function calls
	pThis->StopProtectionDrivers();
	//pThis->RemoveDriverfilesFromSystem32();
	if(!pThis->CloseAllAplication(true))	
	{
		theApp.m_PtWWizUninstallerDlg->CloseApp();
	}
	else
	{	
		theApp.m_PtWWizUninstallerDlg->ShowHideUninstallerDlg(false);
		theApp.m_objWWizUninstSecondDlg.ShowWindow(SW_SHOW);
		//Nitin's function calls
		pThis->m_iTotalDeletedFileCount = 0;

		theApp.m_csWardWizPath = theApp.GetWardWizPathFromRegistry();

		pThis->m_bMoveFileExFlag = FALSE;
		pThis->m_pcDeleteProgress.SetRange32( 0, pThis->m_iTotalFileCount ) ;
		pThis->DeleteFiles(pThis->m_vFilePathLists);
		pThis->DeleteFolder(pThis->m_vFolderPathLists);
		pThis->DeleteRegistryKeys();
		pThis->DeleteAllAppShortcut();
		pThis->DeleteFinished();
		pThis->UninstallationMessage();
		pThis->RemoveDriverRegistryKeyAndPauseDrivers();
	}
	return 0x00;
}

/***************************************************************************************************
*  Function Name  : OnTimer
*  Description    : The framework calls this member function after each interval specified in the SetTimer member function used to install a timer.
*  Author Name    :	Nitin K
*  SR_NO          : 
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CWWizUninstSecondDlg::OnTimer(UINT_PTR nIDEvent)
{
	TCHAR	szTemp[256] = {0} ;
	if(nIDEvent == TIMER_DELETE_STATUS)
	{
		int		iPercentage = int ( ((float) (m_iTotalDeletedFileCount) / m_iTotalFileCount) * 100 ) ;
		m_pcDeleteProgress.SetPos(m_iTotalDeletedFileCount ) ;
		
		m_pcDeleteProgress.ShowWindow(SW_SHOW);
		m_pcDeleteProgress.RedrawWindow();
		wsprintf(szTemp, TEXT("%d%%"), iPercentage ) ;
	}
	CJpegDialog::OnTimer(nIDEvent);
}

/***************************************************************************************************
*  Function Name  : DeleteFinished
*  Description    : called to kill the timer of status bar
*  Author Name    :	Nitin K
*  SR_NO          :
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CWWizUninstSecondDlg::DeleteFinished()
{
	AddLogEntry(L">>> CWWizUninstSecondDlg : DeleteFinished", 0, 0, true, ZEROLEVEL);
	KillTimer(TIMER_DELETE_STATUS);
}

/***************************************************************************************************
*  Function Name  : UninstallationMessage
*  Description    : it shows the third dialog of uninstallation finished
*  Author Name    :	Nitin K
*  SR_NO          :
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CWWizUninstSecondDlg::UninstallationMessage()
{
	if( !theApp.m_bPassDbMngr || !theApp.m_bQuarantine )
	{
		MoveFileEx(theApp.m_csWardWizPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
	}
	theApp.m_bUninstallInProgress = FALSE;
	theApp.m_PtWWizUninstallerDlg->ShowWindow(SW_RESTORE);
	theApp.m_objWWizUninstSecondDlg.ShowHideUninstSecondDlg(false);
	theApp.m_objWWizUninstThirdDlg.ShowWindow(SW_SHOW);
}

/***************************************************************************************************
*  Function Name  : ShowHideUninstSecondDlg
*  Description    : Show hide second dialog controls
*  Author Name    :	Nitin K
*  SR_NO          :
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CWWizUninstSecondDlg::ShowHideUninstSecondDlg(bool bEnable)
{
	//m_stConfigMessage.ShowWindow(bEnable);
	m_stUninstallationInProgress.ShowWindow(bEnable);
	m_pcDeleteProgress.ShowWindow(bEnable);
	m_btnMinimizeToTaskBar.ShowWindow(bEnable);
	m_stMinimizeWindowText1.ShowWindow(bEnable);
	m_stMinimizeWindowText2.ShowWindow(bEnable);
	//Varada Ikhar, Date: 11-03-2015, 
	//Issue: After uninstall is complete,when it ask us to restart the machine,the symbol below the tick mark should be removed.
	m_stImageMinimize.ShowWindow(bEnable); 
}

/***************************************************************************************************
*  Function Name  : OnBnClickedButtonMinimizeToTaskbar
*  Description    : Minimizes the dialog to task bar
*  Author Name    :	Nitin K
*  SR_NO          :
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CWWizUninstSecondDlg::OnBnClickedButtonMinimizeToTaskbar()
{
	theApp.m_PtWWizUninstallerDlg->ShowWindow(SW_MINIMIZE);
}

/***************************************************************************
Function Name  : CloseAllAplication
Description    : Function which closes Runing Application.
Author Name    : 
S.R. No        : WRDWIZSETUPDLL_0003
Date           : 
****************************************************************************/
bool CWWizUninstSecondDlg::CloseAllAplication(bool bIsReInstall)
{
	AddLogEntry(L">>> CWWizUninstSecondDlg : CloseAllAplication", 0, 0, true, ZEROLEVEL);
	//m_csAppPath =L"C:\\Program Files\\WardWiz Antivirus";
	m_csAppPath = theApp.GetModuleFilePath();
	bool bISFullPath = false;
	if(_tcslen(m_csAppPath) > 0)
	{
		bISFullPath = true;
	}

	CString csModulePath = m_csAppPath;

	if(bIsReInstall)
	{
		if(!GetCloseAction4OutlookIfRunning(true))
		{
			return false;
		}
	}

	//CString csAppPath = csModulePath + L"WRDWIZAVUI.EXE";
	CEnumProcess objEnumProcess;
	if(objEnumProcess.IsProcessRunning(L"WRDWIZAVUI.EXE", false, false, false))
	{
		int iRet = MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_IS_RUNNING"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO);
		if(iRet == IDYES)
		{
			objEnumProcess.IsProcessRunning(L"WRDWIZAVUI.EXE", true, false, false);

		}
		else
		{
			return false;
		}
	}
	//Issue Resolved : When we insert usb and uninstall the setup still usb scan is in process.
	//Resolved By : Nitin K. Date: 11th March 2015
	if (objEnumProcess.IsProcessRunning(L"WRDWIZUSBDETECTUI.EXE", false, false, false))
	{
		int iRet = MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_IS_RUNNING"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO);
		if (iRet == IDYES)
		{
			objEnumProcess.IsProcessRunning(L"WRDWIZUSBDETECTUI.EXE", true, false, false);
		}
		else
		{
			return false;
		}
	}
	CStringArray objcsaWardWizProcesses;
	objcsaWardWizProcesses.Add(L"WRDWIZSCANNER.EXE");
	objcsaWardWizProcesses.Add(L"WRDWIZTRAY.EXE");

	for(int iIndex = 0; iIndex < objcsaWardWizProcesses.GetCount(); iIndex++)
	{
		CString csProcessName = objcsaWardWizProcesses.GetAt(iIndex);
		if(objEnumProcess.IsProcessRunning(csProcessName, true, false, false))
		{
			AddLogEntry(L">>> %s was running, Terminated", csProcessName, 0, true, SECONDLEVEL);
		}

	}

	//close and remove service here
	CISpySrvMgmt		iSpySrvMgmtObj ;
	if(iSpySrvMgmtObj.StopServiceManually(WARDWIZSERVICENAME) != 0x00)
	{
		AddLogEntry(L"### Unable to Stop Service WardWizComsrv", 0, 0, true, SECONDLEVEL);
	}


	if(iSpySrvMgmtObj.UnInstallService(WARDWIZSERVICENAME) != 0x00)
	{
		AddLogEntry(L"### Unable to UnInstall Service WardWizComsrv", 0, 0, true, SECONDLEVEL);

	}

	//close and remove service here
	if(iSpySrvMgmtObj.StopServiceManually(WARDWIZUPDATESERVICENAME) != 0x00)
	{
		AddLogEntry(L"### Unable to Stop Update Service WardWizComsrv", 0, 0, true, SECONDLEVEL);
	}

	if(iSpySrvMgmtObj.UnInstallService(WARDWIZUPDATESERVICENAME) != 0x00)
	{
		AddLogEntry(L"### Unable to UnInstall Update Service WardWizComsrv", 0, 0, true, SECONDLEVEL);

	}

	return true;
}

/***************************************************************************
Function Name  : GetCloseAction4OutlookIfRunning
Description    : Closing outloog If Running.
Author Name    : Ramkrushna Shelke
S.R. No        : WRDWIZSETUPDLL_0004
Date           : 7/25/2014
****************************************************************************/
bool CWWizUninstSecondDlg::GetCloseAction4OutlookIfRunning(bool bIsReInstall)
{
	m_csProductName	= L"BASIC";
	//No need to show the prompt for outlook close if the product is BASIC/ESSENTIAL
	if(m_csProductName == L"BASIC" || m_csProductName == L"ESSENTIAL")
	{
		return true;
	}

	while(true)
	{
		CEnumProcess objEnumProcess;
		if(objEnumProcess.IsProcessRunning(L"OUTLOOK.EXE", false, false, false))
		{
			CString csMessage;
			if(bIsReInstall)
				csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_OUTLOOK_CLOSE_REINSTALL_MSG");
			else
				csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_OUTLOOK_CLOSE_UNINSTALL_MSG");

			int iRet = MessageBox(csMessage, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_RETRYCANCEL);
			if(iRet == IDCANCEL)
			{
				return false;
			}
		}
		else
		{
			break;
		}
	}
	return true;
}


HCURSOR CWWizUninstSecondDlg::OnQueryDragIcon()
{
	// TODO: Add your message handler code here and/or call default

	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CWWizUninstSecondDlg::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	/*CRect oRcDlgRect;
	GetClientRect(&oRcDlgRect);
	if( pDC != NULL )
	{
		pDC->FillSolidRect(oRcDlgRect, RGB(1, 1, 1));
	}*/
	return TRUE; 
	//return CJpegDialog::OnEraseBkgnd(pDC);
}

/***************************************************************************
Function Name  : OnCtlColor
Description    : The framework calls this member function when a child
control is about to be drawn.
Author Name    : 
Date           : 18th Nov 2013
****************************************************************************/
HBRUSH CWWizUninstSecondDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int	ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();

	if(	ctrlID == IDC_STATIC_UNINSTALLATION_INPROGRESS  ||
		ctrlID == IDC_STATIC_MINIMEZE_WINDOW_1 ||
		ctrlID == IDC_STATIC_MINIMEZE_WINDOW_2 
		//ctrlID == IDC_STATIC_ALL_RIGHTS_RESERVED||	
		//ctrlID == IDC_STATIC_DES_USER_SETTING ||
		//ctrlID == IDC_STATIC_DES_QURNTINE ||
		//ctrlID == IDC_STATIC_DES_PASSMGR 
		)
		{
			pDC->SetBkMode(TRANSPARENT);
			hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
		}	return hbr;

}

/***************************************************************************
Function Name  : OnPaint
Description    : The framework calls this member function when Windows or an
application makes a request to repaint a portion of an application's window.
Author Name    : Ramkrushna Shelke
SR_NO			 :
Date           : 18th Nov 2013
****************************************************************************/
void CWWizUninstSecondDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CJpegDialog::OnPaint() for painting messages
	
	OnBackground(&theApp.m_bmpMiddleScndDlg,&dc,0,true);
	OnBackgroundMiddleImage(&theApp.m_bmpSecondDlgLeft, &dc, 120, false);
	OnBackgroundMiddleImage(&theApp.m_bmpSecondDlgMiddle, &dc, 120, true);
	OnBackgroundMiddleImage(&theApp.m_bmpSecondDlgRight, &dc, 120, false);

	CDialog::OnPaint();	
}

void CWWizUninstSecondDlg::OnBackground(CBitmap* bmpImg, CDC* pDC,int yHight,bool isStretch)
{
	CBitmap bmpObj;
	CRect rect;
	CDC visibleDC;
	//int iOldStretchBltMode;
	GetClientRect(&rect);
	visibleDC.CreateCompatibleDC(pDC);
	bmpObj.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	visibleDC.SelectObject(bmpImg);
	
	if(isStretch)
	{
		for(int i=0; i <=rect.Width()/5 ;i++)
		{
			//pDC->StretchBlt(i*5, 0, rect.Width(), rect.Height(), &srcDC, 0, 0, 1000,500, SRCCOPY);
			pDC->BitBlt(i * 5, yHight, rect.Width(), rect.Height(), &visibleDC, 0, 0, SRCCOPY);
		}
	}
	else
	{
		pDC->BitBlt(0, yHight, rect.Width(), rect.Height(), &visibleDC, 0, 0, SRCCOPY);
	}

	bmpObj.DeleteObject();

	visibleDC.DeleteDC();

}

void CWWizUninstSecondDlg::OnBnClickedButtonClose()
{
	// TODO: Add your control notification handler code here
	OnCancel(); 
}

/***************************************************************************************************
*  Function Name  : DeleteAllAppShortcut
*  Description    : Deletes all shortcuts of application 
*  Author Name    : Lalit K
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CWWizUninstSecondDlg::DeleteAllAppShortcut()
{
	try
	{
		AddLogEntry(L">>> CWWizUninstSecondDlg : DeleteAllAppShortcut", 0, 0, true, ZEROLEVEL);
		TCHAR CommonAppPath[MAX_PATH];
		HRESULT result = SHGetFolderPath(NULL, CSIDL_COMMON_PROGRAMS, NULL, SHGFP_TYPE_CURRENT, CommonAppPath);
		CString obj= CommonAppPath;
		swprintf_s(	CommonAppPath, _countof(CommonAppPath), L"%s\\%s", CommonAppPath,L"WardWiz Antivirus");
		RemoveFilesUsingSHFileOperation(CommonAppPath);

		TCHAR DeskTopIconPath[MAX_PATH];
		HRESULT result1 = SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, DeskTopIconPath);
		swprintf_s(	DeskTopIconPath, _countof(DeskTopIconPath), L"%s\\%s", DeskTopIconPath,L"WardWiz.lnk");
		DeleteFile(DeskTopIconPath);
		//CString obj2= DeskTopIconPath;
		//AfxMessageBox(obj2);

		TCHAR CommDeskTopIconPath[MAX_PATH];
		HRESULT result2 = SHGetFolderPath(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, NULL, SHGFP_TYPE_CURRENT, CommDeskTopIconPath);
		swprintf_s(	CommDeskTopIconPath, _countof(CommDeskTopIconPath), L"%s\\%s", CommDeskTopIconPath,L"WardWiz.lnk");
		DeleteFile(CommDeskTopIconPath);
		
		TCHAR QuickLaunchIconPath[512] = { 0 };
		HRESULT result3 = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, QuickLaunchIconPath);
		swprintf_s(QuickLaunchIconPath, _countof(QuickLaunchIconPath), L"%s\\%s", QuickLaunchIconPath, L"Microsoft\\Internet Explorer\\Quick Launch\\WardWiz.lnk");
		DeleteFile(QuickLaunchIconPath);

		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	}
	catch(...)
	{
	}
}

/***************************************************************************
  Function Name  : RemoveFilesUsingSHFileOperation
  Description    : Function which removes file using SHFileOperation
				   This function can be used to copy, move, rename, or 
				   delete a file system object.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0037
  Date           : 07th Aug 2014
****************************************************************************/
DWORD CWWizUninstSecondDlg::RemoveFilesUsingSHFileOperation(TCHAR *pFolder )
{
	HRESULT			hr = 0x00 ;
	try
	{
		SHFILEOPSTRUCT	sfo = {0} ;

		TCHAR	szTemp[512] = {0} ;

		wsprintf(szTemp, L"%s\\*\0", pFolder ) ;

		sfo.hwnd = NULL ;
		sfo.wFunc = FO_DELETE ;
		sfo.pFrom = szTemp ;
		sfo.pTo = NULL ;
		sfo.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR ;

		hr = SHFileOperation( &sfo ) ;
		if( !hr )
		{
			_wrmdir( (wchar_t *) pFolder ) ;
			RemoveDirectory( pFolder ) ;
		}
	}
	catch(...)
	{

	}
	return hr ;
}

/***************************************************************************************************
*  Function Name  : StopProtectionDrivers
*  Description    : stops the driver protection
*  Author Name    : Lalit K. 
*  SR_NO
*  Date           : 06 FEB 2015
****************************************************************************************************/
void CWWizUninstSecondDlg::StopProtectionDrivers()
{
	// TODO: Add your control notification handler code here
	CScannerLoad scannerObj;
	CSecure64 cSecure64Obj;
	TCHAR szTemp[1024]  =  {0};	
	CWardWizOSversion objGetOSVersion;
	int OsType  = 0 ;
	bool returnStatus = false;

	try
	{

		OsType = objGetOSVersion.DetectClientOSVersion();
		if(OsType==5)
		{
			cSecure64Obj.RegisterProcessId(WLSRV_ID_NINE); // self protection
			scannerObj.RegisterProcessId(WLSRV_ID_NINE); // self protection
			//StopAndUninstallProcessProtection(OsType);
			//StopAndUninstallScanner();

	

		}
		else
			if(OsType==10)
			{

				//StopAndUninstallScanner();
				scannerObj.RegisterProcessId(WLSRV_ID_NINE); // self protection
			
			}
			else
			{

				cSecure64Obj.RegisterProcessId(WLSRV_ID_NINE); // self protection
				scannerObj.RegisterProcessId(WLSRV_ID_NINE); // self protection
				//StopAndUninstallProcessProtection(OsType);
				//StopAndUninstallScanner();
				
			}


	}
	catch(...)
	{
		AddLogEntry(L"### Exception in RemoveDriverService", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************
Function Name  : StopAndUninstallScanner
Description    : this function stop and uninstall the scanner driver which is used for file protection. 
Author Name    : lalit kumawat
S.R. No        : 
Date           :
****************************************************************************/
void CWWizUninstSecondDlg::StopAndUninstallScanner()
{
	//LPCWSTR SERVICE_NAME = (LPCWSTR)L"scanner";
	TCHAR	SERVICE_NAME[]  = L"wrdwizscanner";
	CISpySrvMgmt		iSpySrvMgmtObj ;

	if(iSpySrvMgmtObj.StopServiceManually(SERVICE_NAME) != 0x00)
	{
		AddLogEntry(L"### Unable to Stop Service Scanner", 0, 0, true, SECONDLEVEL);
		DWORD error = GetLastError();
	}

	if(iSpySrvMgmtObj.UnInstallService(SERVICE_NAME) != 0x00)
	{
		AddLogEntry(L"### Unable to UnInstall Service WardWizComsrv", 0, 0, true, SECONDLEVEL);
		DWORD error = GetLastError();
	}

}

/***************************************************************************
Function Name  : StopAndUninstallProcessProtection
Description    : this function stop and uninstall process protection drivers 
Author Name    : lalit kumawat
S.R. No        : 
Date           :
****************************************************************************/
void CWWizUninstSecondDlg::StopAndUninstallProcessProtection(int OsType)
{
	TCHAR	SERVICE_NAME[512] = {0};
	CString csServiceName = L"";

	if(OsType==5)
	{
		csServiceName = L"WrdWizXpProc";
	}
	else
	{
		csServiceName = L"WrdWizSecure64";
	}

	swprintf_s(SERVICE_NAME, _countof(SERVICE_NAME), L"%s",csServiceName);
	//LPCWSTR SERVICE_NAME = (LPCWSTR)csServiceName;
	CISpySrvMgmt		iSpySrvMgmtObj ;

	if(iSpySrvMgmtObj.StopServiceManually(SERVICE_NAME) != 0x00)
	{
		AddLogEntry(L"### Unable to Stop Service Scanner", 0, 0, true, SECONDLEVEL);
		DWORD error = GetLastError();
	}

	if(iSpySrvMgmtObj.UnInstallService(SERVICE_NAME) != 0x00)
	{
		AddLogEntry(L"### Unable to UnInstall Service WardWizComsrv", 0, 0, true, SECONDLEVEL);
		DWORD error = GetLastError();
	}

}

/***************************************************************************
Function Name  : RemoveDriverfilesFromSystem32
Description    : removes driver files from system32 folder
Author Name    : lalit kumawat
S.R. No        :
Date           :
****************************************************************************/
void CWWizUninstSecondDlg::RemoveDriverfilesFromSystem32()
{

	TCHAR System32Path[MAX_PATH];
	HRESULT result = SHGetFolderPath(NULL, CSIDL_SYSTEM, NULL, SHGFP_TYPE_CURRENT, System32Path);

	TCHAR szTemp[1024]  =  {0};	
	CWardWizOSversion objGetOSVersion;
	int OsType  = 0 ;
	bool returnStatus = false;

	try
	{

		OsType = objGetOSVersion.DetectClientOSVersion();
		if(OsType==5)
		{
			TCHAR System32FilePath[MAX_PATH];
			swprintf_s(	System32FilePath, _countof(System32FilePath), L"%s\\drivers\\%s", System32Path,L"wrdwizscanner.sys");
			DeleteFile(System32FilePath);
			ZeroMemory(System32FilePath, MAX_PATH * sizeof(wchar_t));
			swprintf_s(	System32FilePath, _countof(System32FilePath), L"%s\\drivers\\%s", System32Path,L"WrdWizXpProc.sys");
			DeleteFile(System32FilePath);

		}
		else
			if(OsType==10)
			{

				TCHAR System32FilePath[MAX_PATH];
				swprintf_s(	System32FilePath, _countof(System32FilePath), L"%s\\drivers\\%s", System32Path,L"wrdwizscanner.sys");
				DeleteFile(System32FilePath);

			}
			else
			{

				TCHAR System32FilePath[MAX_PATH];
				swprintf_s( System32FilePath, _countof(System32FilePath), L"%s\\drivers\\%s", System32Path,L"WRDWIZFILEPROT.SYS");
				DeleteFile(System32FilePath);
				ZeroMemory(System32FilePath, MAX_PATH * sizeof(wchar_t));
				swprintf_s(	System32FilePath, _countof(System32FilePath), L"%s\\drivers\\%s", System32Path,L"WRDWIZREGPROT.SYS");
				DeleteFile(System32FilePath);

			}


	}
	catch(...)
	{
		AddLogEntry(L"### Exception in RemoveDriverService", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : RemoveDriverRegistryKeyAndPauseDrivers
Description    : removes registry keys for drivers
Author Name    : lalit kumawat
S.R. No        :
Date           :
****************************************************************************/
void CWWizUninstSecondDlg::RemoveDriverRegistryKeyAndPauseDrivers()
{
	// TODO: Add your control notification handler code here
	CScannerLoad scannerObj;
	CSecure64 cSecure64Obj;
	TCHAR szTemp[1024] = { 0 };
	CWardWizOSversion objGetOSVersion;
	int OsType = 0;
	bool returnStatus = false;

	try
	{

		OsType = objGetOSVersion.DetectClientOSVersion();
		if (OsType == 5)
		{
			scannerObj.PauseDriverProtection(0); // self protection
			cSecure64Obj.PauseDriverProtection(0); // self protection
			RegDelnode(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\wrdwizscanner"));
			RegDelnode(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\WrdWizSecure64"));

		}
		else
			if (OsType == 10)
			{
				scannerObj.PauseDriverProtection(0); // self protection
				RegDelnode(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\wrdwizscanner"));
		     }
			else
			{

				scannerObj.PauseDriverProtection(0); // self protection
				cSecure64Obj.PauseDriverProtection(0); // self protection							\\scanner

				RegDelnode(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\wrdwizscanner"));
				RegDelnode(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\Services\\WrdWizSecure64"));
			}

			RegDelnode(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\services\\WardwizALUSrv"));
			RegDelnode(HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\services\\WardwizComSrv"));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in RemoveDriverService", 0, 0, true, SECONDLEVEL);
	}


}

//*************************************************************
//
//  RegDelnodeRecurse()
//
//  Purpose:    Deletes a registry key and all its subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************

BOOL CWWizUninstSecondDlg::RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey)
{
	LPTSTR lpEnd;
	LONG lResult;
	DWORD dwSize;
	TCHAR szName[MAX_PATH];
	HKEY hKey;
	FILETIME ftWrite;

	// First, see if we can delete the key without having
	// to recurse.

	lResult = RegDeleteKey(hKeyRoot, lpSubKey);

	if (lResult == ERROR_SUCCESS)
		return TRUE;

	lResult = RegOpenKeyEx(hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);

	if (lResult != ERROR_SUCCESS)
	{
		if (lResult == ERROR_FILE_NOT_FOUND) {
			printf("Key not found.\n");
			return TRUE;
		}
		else {
			printf("Error opening key.\n");
			return FALSE;
		}
	}

	// Check for an ending slash and add one if it is missing.

	lpEnd = lpSubKey + lstrlen(lpSubKey);

	if (*(lpEnd - 1) != TEXT('\\'))
	{
		*lpEnd = TEXT('\\');
		lpEnd++;
		*lpEnd = TEXT('\0');
	}

	// Enumerate the keys

	dwSize = MAX_PATH;
	lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
		NULL, NULL, &ftWrite);

	if (lResult == ERROR_SUCCESS)
	{
		do {

			StringCchCopy(lpEnd, MAX_PATH * 2, szName);

			if (!RegDelnodeRecurse(hKeyRoot, lpSubKey)) {
				break;
			}

			dwSize = MAX_PATH;

			lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
				NULL, NULL, &ftWrite);

		} while (lResult == ERROR_SUCCESS);
	}

	lpEnd--;
	*lpEnd = TEXT('\0');

	RegCloseKey(hKey);

	// Try again to delete the key.

	lResult = RegDeleteKey(hKeyRoot, lpSubKey);

	if (lResult == ERROR_SUCCESS)
		return TRUE;

	return FALSE;
}

//*************************************************************
//
//  RegDelnode()
//
//  Purpose:    Deletes a registry key and all its subkeys / values.
//
//  Parameters: hKeyRoot    -   Root key
//              lpSubKey    -   SubKey to delete
//
//  Return:     TRUE if successful.
//              FALSE if an error occurs.
//
//*************************************************************
BOOL CWWizUninstSecondDlg::RegDelnode(HKEY hKeyRoot, LPTSTR lpSubKey)
{
	TCHAR szDelKey[MAX_PATH * 2];

	StringCchCopy(szDelKey, MAX_PATH * 2, lpSubKey);
	return RegDelnodeRecurse(hKeyRoot, szDelKey);

}

/***************************************************************************
Function Name  : OnBackgroundMiddleImage
Description    : To draw the middle grey image.
Author Name    : Varada Ikhar
S.R. No        :
Date           : 11-03-2015
****************************************************************************/
void CWWizUninstSecondDlg::OnBackgroundMiddleImage(CBitmap* bmpImg, CDC* pDC, int yHight, bool isStretch)
{
	CBitmap bmpObj;
	CRect rect;
	CDC visibleDC;
	//int iOldStretchBltMode;
	GetClientRect(&rect);
	visibleDC.CreateCompatibleDC(pDC);
	bmpObj.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	visibleDC.SelectObject(bmpImg);

	if (isStretch)
	{
		for (int i = 0; i <= rect.Width() / 2; i++)
		{
			if (i <= 12 || i >= 362)
				continue;
			pDC->BitBlt(i * 2, yHight, rect.Width(), rect.Height(), &visibleDC, 0, 0, SRCCOPY);
		}
	}
	else
	{
		if (bmpImg == &theApp.m_bmpSecondDlgLeft)
		{
			pDC->BitBlt(20, yHight, rect.Width(), rect.Height(), &visibleDC, 0, 0, SRCCOPY);
		}
		else if (bmpImg == &theApp.m_bmpSecondDlgRight)
		{
			pDC->BitBlt((rect.Width() - 30), yHight, rect.Width(), rect.Height(), &visibleDC, 0, 0, SRCCOPY);
		}
	}

	bmpObj.DeleteObject();

	visibleDC.DeleteDC();

}

/***************************************************************************
Function Name  : UnregisterComDll
Description    : Unregister COM Dll
Author Name    : Neha Gharge
S.R. No        :
Date           : 3/28/2015
****************************************************************************/
void CWWizUninstSecondDlg::UnregisterComDll(CString csAppPath)
{
	try
	{
		CWardWizOSversion		objOSVersionWrap;
		CString csExePath, csCommandLine;
		TCHAR systemDirPath[MAX_PATH] = _T("");
		GetSystemDirectory(systemDirPath, sizeof(systemDirPath) / sizeof(_TCHAR));

		csExePath.Format(L"%s\\%s", systemDirPath, L"regsvr32.exe");

		//On xp runas parameter never work It will not unregister the wrdwizshellext.dll
		//So NUll parameter send.
		DWORD OSType = objOSVersionWrap.DetectClientOSVersion();
		//Neha Gharge Message box showing of register successful on reinstallation.
		switch (OSType)
		{
		case WINOS_XP:
		case WINOS_XP64:
			csCommandLine.Format(L"-u -s \"%s\"", csAppPath);
			ShellExecute(NULL, NULL, csExePath, csCommandLine, NULL, SWP_HIDEWINDOW);
			break;
		default:
			csCommandLine.Format(L"-u -s \"%s\"", csAppPath);
		ShellExecute(NULL, L"runas", csExePath, csCommandLine, NULL, SWP_HIDEWINDOW);
			break;
		}

		if (PathFileExists(csAppPath))
		{
			if (!DeleteFile(csAppPath))
			{
				AddLogEntry(L"### DeleteFile Failed for file: %s .", csAppPath, 0, true, SECONDLEVEL);
				if (!MoveFileEx(csAppPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT))
				{
					AddLogEntry(L"### MoveFileEx Failed for file: %s.", csAppPath, 0, true, SECONDLEVEL);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in UnRegisterComDLL", 0, 0, true, SECONDLEVEL);
	}

}