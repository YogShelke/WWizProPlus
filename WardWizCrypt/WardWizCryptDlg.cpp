/**********************************************************************************************************
	Program Name			: WardWizCryptDlg.cpp
	Description				: This class is derived from CDialogEx, which is main Window for this project
	Author Name				: Ramkrushna Shelke
	Date Of Creation		: 5th Jun 2015
	Version No				: 1.11.0.0
	Special Logic Used		:
	Modification Log		:
	***********************************************************************************************************/
#include "stdafx.h"
#include "WardWizCrypt.h"
#include "WardWizCryptDlg.h"
#include "afxdialogex.h"
#include<stdio.h>
#include "CSecure64.h"
#include "DriverConstants.h"
#include "CScannerLoad.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

PROCESS_INFORMATION		pi;
DWORD					exitcode = 0;
#define BUFSIZE			2024

const int TIMER_SHOW_COMPRS_STATUS = 1600;
//const int TIMER_CALL_BACK = 1500;
/***************************************************************************
						CAboutDlg class declaration
						****************************************************************************/
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

/***************************************************************************
	Function Name  : CAboutDlg
	Description    : C'tor
	Author Name    : Ramkrushna Shelke
	SR_NO		   :
	Date           : 5th Jun 2015
	****************************************************************************/
CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

/***************************************************************************
Function Name  : DoDataExchange
Description    :
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

DWORD WINAPI StartCryptoThread(LPVOID lpvThreadParam);
DWORD WINAPI CallOnTimerThread(LPVOID lpvThreadParam);
CWardWizCryptDlg * CWardWizCryptDlg::m_pThis = NULL;
CISpyCommunicatorServer  g_objCommServer(WWIZ_CRYPT_SERVER, CWardWizCryptDlg::OnDataReceiveCallBack, sizeof(ISPY_PIPE_DATA));
/***************************************************************************
Function Name  : CWardWizCryptDlg
Description    : C'tor
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
CWardWizCryptDlg::CWardWizCryptDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWardWizCryptDlg::IDD, pParent)
	, m_dwFilesProcessed(0)
	, m_dwTotalFiles(0)
	, m_objCom(UI_SERVER, true)
	, m_dwKeepOriginal(0)
	, m_objWardWizCrypter()
	, m_hEvtStop(NULL)
	, m_hEncrDecryptionThread(NULL)
	, m_csSelectedFilePath(L"")
	, m_IsOperSuccess(false)
	, m_csEmptyDrive(L"")
	, m_csOrgFileName(L"")
	, m_csOrgFileNamePath(L"")
	, m_csCurntArchTmpFilePath(L"")
	, m_csCurntEncFilePath(L"")
	, m_csFinalOutputFilePath(L"")
	, m_bIsAborted(false)
	, m_bIsManualAbort(false)
	, m_bIsEnDecStarted(false)
	, m_bIsComDomStarted(false)
	, m_csSaveAsPath(L"")
	, m_bIsFileFromOSPath(false)
	, m_dwRequiredSpace(0)
{
	m_pThis = this;
	g_objCommServer.Run();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	// bytes available to caller
	m_uliFreeBytesAvailable.QuadPart = 0L;
	// bytes on disk
	m_uliTotalNumberOfBytes.QuadPart = 0L;
	// free bytes on disk
	m_uliTotalNumberOfFreeBytes.QuadPart = 0L;
}

/***************************************************************************
Function Name  : CWardWizCryptDlg
Description    : Called by the framework to exchange and validate dialog data.
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
void CWardWizCryptDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_STATUS, m_editStatus);
	DDX_Control(pDX, IDC_EDIT_FILEFOLDER_PATH, m_editFolderPath);
	DDX_Control(pDX, IDC_BUTTON_BROWSE_FOLDER, m_btnBrowse);
	DDX_Control(pDX, ID_BUTTON_START, m_btnStart);
	DDX_Control(pDX, ID_BUTTON_CANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_STATIC_FILES_PROCESSED, m_stFilesProcessed);
	DDX_Control(pDX, IDC_STATIC_TOTAL_FILES, m_stTotalFiles);
	DDX_Control(pDX, IDC_LIST_REPORT, m_lstCryptReport);
	DDX_Control(pDX, IDC_RADIO_ENCRYPT, m_rdbEncrypt);
	DDX_Control(pDX, IDC_RADIO_SELECTFILE, m_rdbFile);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, m_editPassword);
	DDX_Control(pDX, IDC_STATIC_TOTALFILES, m_stTotalFilesCount);
}

BEGIN_MESSAGE_MAP(CWardWizCryptDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_FOLDER, &CWardWizCryptDlg::OnBnClickedButtonBrowseFolder)
	ON_BN_CLICKED(ID_BUTTON_START, &CWardWizCryptDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(ID_BUTTON_CANCEL, &CWardWizCryptDlg::OnBnClickedButtonCancel)
	ON_WM_TIMER()
END_MESSAGE_MAP()


/***************************************************************************
Function Name  : OnInitDialog
Description    : his method is called in response to the WM_INITDIALOG message.
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
BOOL CWardWizCryptDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	if (theApp.m_bIsNoGui)
	{
		ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	}
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	//Added By Nitin Kolapkar
	//added for performing Commandline Crypt request
	if (theApp.m_bIsNoGui)
	{
		ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	}
	m_rdbEncrypt.SetCheck(TRUE);

	CSecure64  objCSecure;
	objCSecure.RegisterProcessId(WLSRV_ID_TWO);  // to register service for process protection

	CScannerLoad	objCScanner;
	objCScanner.RegisterProcessId(WLSRV_ID_TWO);//WLSRV_ID_TWO to register service for process protection

	m_rdbFile.SetCheck(TRUE);

	m_lstCryptReport.InsertColumn(0, L"File Path", LVCFMT_LEFT, 500);
	m_lstCryptReport.InsertColumn(1, L"Status", LVCFMT_LEFT, 100);

	ListView_SetExtendedListViewStyle(m_lstCryptReport.m_hWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);

	UpdateStatus(L"Please select options to start operation.");

	// issue:- during encryption abot dump getting created.its happening due to unsynchronization.
	// resolve by lalit kumawat 7-292015
	m_hStopEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);

	//Added By Nitin Kolapkar
	//Parsing the command line for request from Main UI and starting the CryptOpr byDefault
	if (theApp.m_bIsNoGui)
	{
		OnBnClickedButtonStart();
	}

	m_bIsAborted = false;
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************
Function Name  : OnSysCommand
Description    : The framework calls this member function when the user selects
a command from the Control menu, or when the user selects the
Maximize or the Minimize button.
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
void CWardWizCryptDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}

}

/***************************************************************************
Function Name  : OnPaint
Description    : If you add a minimize button to your dialog, you will need the code below
to draw the icon.  For MFC applications using the document/view model,
this is automatically done for you by the framework.
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
void CWardWizCryptDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

/***************************************************************************
Function Name  : OnQueryDragIcon
Description    : The system calls this function to obtain the cursor to display while the user drags
the minimized window.
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
HCURSOR CWardWizCryptDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/***************************************************************************
Function Name  : OnBnClickedButtonBrowseFolder
Description    : Function handler to browse a folder/File
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
void CWardWizCryptDlg::OnBnClickedButtonBrowseFolder()
{
	BOOL bIsFile = m_rdbFile.GetCheck();

	if (bIsFile)
	{
		TCHAR szFilter[] = L"All Files (*.*)|*.*||";
		CFileDialog FileDlg(TRUE, L".xls", NULL, 0, szFilter);
		if (FileDlg.DoModal() == IDOK)
		{
			CString strFile = FileDlg.GetPathName();
			m_editFolderPath.SetWindowTextW(strFile);
		}
	}
	else
	{
		TCHAR *pszPath = new TCHAR[MAX_PATH];
		SecureZeroMemory(pszPath, MAX_PATH*sizeof(TCHAR));

		CString csMessage = L"Select Folder";
		BROWSEINFO        bi = { m_hWnd, NULL, pszPath, csMessage, BIF_RETURNONLYFSDIRS, NULL, 0L, 0 };

		LPITEMIDLIST      pIdl = NULL;

		LPITEMIDLIST  pRoot = NULL;
		//SHGetFolderLocation(m_hWnd, CSIDL_DRIVES, 0, 0, &pRoot);
		bi.pidlRoot = pRoot;
		pIdl = SHBrowseForFolder(&bi);
		if (NULL != pIdl)
		{
			SHGetPathFromIDList(pIdl, pszPath);
			size_t iLen = wcslen(pszPath);
			if (iLen > 0)
			{
				CString csTemp;
				csTemp = pszPath;
				if (csTemp.Right(1) == L"\\")
				{
					csTemp = csTemp.Left((int)iLen - 1);
				}
				m_editFolderPath.SetWindowTextW(csTemp);
			}
		}
		delete[] pszPath;
		pszPath = NULL;
	}

}

/***************************************************************************
Function Name  : OnBnClickedButtonStart
Description    : Function handler to start Encryption/Decryption operation.
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
void CWardWizCryptDlg::OnBnClickedButtonStart()
{
	try
	{
		InitializeVariables();

		if (!theApp.m_bIsNoGui)
		{
			m_bIsEncrypt = m_rdbEncrypt.GetCheck();
			m_bIsFile = m_rdbFile.GetCheck();

			m_btnStart.EnableWindow(FALSE);

			m_editFolderPath.GetWindowText(m_csFileFolderPath);
			m_editPassword.GetWindowText(m_csPassword);

			if (!PathFileExists(m_csFileFolderPath))
			{
				MessageBox(L"Please select valid file/Folder", L"Vibranium", MB_ICONINFORMATION);
				m_btnStart.EnableWindow(TRUE);
				return;
			}
		}

		GetEnviormentVariablesForAllMachine();

		m_tsStartTime = CTime::GetCurrentTime();

		AddLogEntry(L"==========================================================================", 0, 0, true, SECONDLEVEL);
		AddLogEntry(L"Operation started", 0, 0, true, SECONDLEVEL);

		m_hThreadCrypto = ::CreateThread(NULL, 0, StartCryptoThread, (LPVOID) this, 0, NULL);

		Sleep(10);
		m_hThreadTimer = ::CreateThread(NULL, 0, CallOnTimerThread, (LPVOID) this, 0, NULL);

		if (m_hThreadCrypto != NULL)
		{
			WaitForSingleObject(m_hThreadCrypto, INFINITE);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCryptDlg::OnBnClickedButtonStart", 0, 0, true, SECONDLEVEL);
	}
	exit(0);
}

/***************************************************************************
Function Name  : OnBnClickedButtonCancel
Description    : Function handler to cancel crypto operations.
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
void CWardWizCryptDlg::OnBnClickedButtonCancel()
{
	if (m_hThreadCrypto != NULL)
	{

		CString csMessage;
		if (!theApp.m_bIsNoGui)
		{
			if (m_bIsEncrypt)
			{
				csMessage = L"Do you want to Stop Encryption Process?";
			}
			else
			{
				csMessage = L"Do you want to Stop Decryption Process?";
			}
			int iRet = MessageBox(csMessage, L"Vibranium", MB_ICONINFORMATION | MB_YESNO);

			if (iRet != IDYES)
			{
				return;
			}
		}

		if (m_hThreadTimer != NULL)
		{
			//ResumeThread(m_hThreadCrypto);
			TerminateThread(m_hThreadTimer, 0);
			m_hThreadTimer = NULL;
		}

		if (m_hThreadCrypto != NULL)
		{
			//ResumeThread(m_hThreadCrypto);
			TerminateThread(m_hThreadCrypto, 0);
			m_hThreadCrypto = NULL;
		}
	}
	//OnCancel();
}

/***************************************************************************
Function Name  : StartCryptoThread
Description    : Thread function to handle Encryption/Decryption
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
DWORD WINAPI StartCryptoThread(LPVOID lpvThreadParam)
{
	CWardWizCryptDlg *pThis = (CWardWizCryptDlg*)lpvThreadParam;
	//pThis->m_hEvntSaveAs = CreateEvent(NULL, TRUE, FALSE, NULL);

	if (!pThis)
		return 0;

	bool bSuccess = true;
	//Implementation : Addingr Total file count and Processed file count
	//Added By : Nitin K Date: 2nd July 2015
	DWORD dwTotalFileCount = 0;

	if (theApp.m_bIsNoGui)
	{
		dwTotalFileCount = pThis->EnumerateAllFolderAndFiles(theApp.m_csaTokenEntryFilePath);
		CString szFileCount;
		szFileCount.Format(L"%s %d", theApp.m_objwardwizLangManager.GetString(L"IDS_DATA_CRYPT_TOTAL_FILES"), dwTotalFileCount);
		pThis->SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szFileCount, OPR_FILE_COUNT);
	}
	// no file to process handling
	// resolved by lalit kumawat 7-21-2015
	if (dwTotalFileCount == 0)
	{
		pThis->CryptOprFinished();
		return 1;
	}

	if (pThis->m_bIsEncrypt)
	{
		if (theApp.m_bIsNoGui)
		{
			for (int i = 0; theApp.m_csaTokenEntryFilePath[i] != "\0"; i++)
			{
				if (pThis->m_bIsAborted)
				{
					break;
				}

				if (PathFileExists(theApp.m_csaTokenEntryFilePath[i]))
				{
					pThis->m_csFileFolderPath = theApp.m_csaTokenEntryFilePath[i];
				}
				else
				{
					pThis->SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, theApp.m_csaTokenEntryFilePath[i], INSERT_NEW_ITEM);
					pThis->SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, theApp.m_csaTokenEntryFilePath[i], FILE_NOT_FOUND);
					continue;
				}
				if (!pThis->EncryptOperation())
				{
					bSuccess = false;
					//message to display about failed
				}

				if (pThis->m_bIsFileFromOSPath)
				{
					break;
				}

				//issue: in case of multiple folder after success encr/dec it showing low disk space message instead of success message.
				// resolve by lalit kumawat 8-14-2015
				break;
			}

			SetEvent(pThis->m_hStopEvent);
			pThis->CryptOprFinished();
			return 1;
		}
		else
		{
			if (!pThis->EncryptOperation())
			{
				bSuccess = false;
			}
		}

	}
	else
	{
		if (theApp.m_bIsNoGui)
		{
			for (int i = 0; theApp.m_csaTokenEntryFilePath[i] != "\0"; i++)
			{
				if (pThis->m_bIsAborted)
				{
					break;
				}

				if (PathFileExists(theApp.m_csaTokenEntryFilePath[i]))
				{
					pThis->m_csFileFolderPath = theApp.m_csaTokenEntryFilePath[i];
				}
				else
				{
					pThis->SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, theApp.m_csaTokenEntryFilePath[i], INSERT_NEW_ITEM);
					pThis->SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, theApp.m_csaTokenEntryFilePath[i], FILE_NOT_FOUND);
					continue;
				}
				if (!pThis->DecryptOperation())
				{
					bSuccess = false;
				}
				if (pThis->m_bIsFileFromOSPath)
				{
					break;
				}
				//issue: in case of multiple folder after success encr/dec it showing low disk space message instead of success message.
				// resolve by lalit kumawat 8-14-2015
				break;
			}


			pThis->CryptOprFinished();
			// issue:-during decr temp file remains exist if abort the operation
			// resolved by lalit kumawat 8-14-2015
			SetEvent(pThis->m_hStopEvent);
			return 1;
		}
		else
		{
			if (!pThis->DecryptOperation())
			{
				bSuccess = false;
			}
		}

	}

	pThis->m_btnStart.EnableWindow(TRUE);
	pThis->UpdateResults();
	pThis->m_hThreadCrypto = NULL;

	if (!theApp.m_bIsNoGui)
	{
		if (bSuccess)
		{
			pThis->UpdateStatus(L"Operation completed Successfully");
			pThis->MessageBox(L"Operation completed Successfully", L"Vibranium", MB_ICONINFORMATION);
		}
		else
		{
			pThis->UpdateStatus(L"Operation failed");
			pThis->MessageBox(L"Operation failed", L"Vibranium", MB_ICONEXCLAMATION);
		}
	}
	AddLogEntry(L"### Exiting StartCryptoThread", 0, 0, true, ZEROLEVEL);
	return 1;
}

/***************************************************************************
Function Name  : EncryptOperation
Description    : Function to handle Encryption operation.
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
bool CWardWizCryptDlg::EncryptOperation()
{
	bool bReturn = false;
	bool bIsAlreadyEncrypted = false;
	//issue:-if i selected file and folder then it again include file as enc/ decr
	// resolved by lalit kumawat 8-14-2015
	try
	{

		//	if (!PathIsDirectory(m_csFileFolderPath))
		//{
		//	if (PathFileExists(m_csFileFolderPath))
		//	{

		//		TCHAR szFileStatus[MAX_PATH] = { 0 };
		//		TCHAR szTmpFilePath[MAX_PATH] = { 0 };
		//		TCHAR szOldFilePath[MAX_PATH] = { 0 };
		//		TCHAR szNewFilePath[MAX_PATH] = { 0 };
		//		TCHAR  szZipFilePath[MAX_PATH] = { 0 };
		//		TCHAR szPercentageDone[MAX_PATH] = { 0 };
		//		bool  isSameSaveAsDirectory = true;
		//		CString csfileExtension = L"";
		//		bool isbSpaceAvailable = false;
		//		bool isbSpaceOnDefaultAvbl = false;
		//		CString csFileNameWithoutExtension = L"";
		//		CString csTempPath = L"";
		//		TCHAR szDrive = 'A';
		//		CString csEncryptOutputFilePath = L"";
		//		TCHAR szEncFilePath[MAX_PATH] = { 0 };
		//		bool bIsMiniMumSpace = false;
		//		// issue :- file size exceed to 3GB size handline ,now we are not allowing encryption/decryption if file size more then 3GB
		//		// resolved by lalit kumawat, 7-9-2015

		//		// resolved by lalit kumawat 7-27-2015
		//		// issue :if default disk does not have required space then creating temp file on another drive which have free space
		//		GetAllSystemDrive();
		//		szDrive = GetDefaultDrive(m_csFileFolderPath);
		//		m_vcsListOfDrive.push_back(szDrive);

		//		bIsMiniMumSpace = IsDriveHaveRequiredSpace(szDrive, m_csFileFolderPath, EQUAL_SIZE); // disk space require as same for file size
		//		
		//		if (!bIsMiniMumSpace)
		//		{
		//			SendLowDiskSpaceMessageToGUI();
		//			return false; // disk space low error.
		//		}
		//		
		//		isbSpaceOnDefaultAvbl = IsDriveHaveRequiredSpace(szDrive, m_csFileFolderPath, TRIPLE_SIZE); // 3 for maximum space for all operation on same disk

		//		if (!isbSpaceOnDefaultAvbl)
		//		{
		//			isbSpaceAvailable = GetSystemTempPath(m_csFileFolderPath, csTempPath);
		//		}
		//		else
		//		{
		//			isbSpaceAvailable = isbSpaceOnDefaultAvbl;
		//		}

		//		if (!isbSpaceAvailable)
		//		{
		//			SendLowDiskSpaceMessageToGUI();
		//			return false; // disk space low error.
		//		}

		//		if (IsPathBelongsToOSReservDirectory(m_csFileFolderPath))
		//		{
		//			m_bIsFileFromOSPath = true;

		//			if (theApp.m_bIsNoGui)
		//			{
		//				SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, SYSTEM_FILE_FOLDER_PATH);
		//				return false;
		//			}
		//			else
		//			{
		//				MessageBox(L"Access Denied: Encryption for Operating system file(s) or folder(s) not allowed.");
		//				return false;
		//			}
		//		}

		//		// issue :- file size is 0KB, then give proper message on UI
		//		// resolved by Nitin Kolapkar 21st July 2015
		//		DWORD dwFileSize = IsFileSizeMorethen3G(m_csFileFolderPath);
		//		if (dwFileSize == 0x01)
		//		{
		//			if (theApp.m_bIsNoGui)
		//			{
		//				SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, INSERT_NEW_ITEM);
		//				swprintf_s(szPercentageDone, _countof(szPercentageDone), L"Compressing %d%% ", 0);
		//				SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szPercentageDone, ENC_INPROGRESS, 0);

		//				SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, FILE_SIZE_MORETHEN_3GB);
		//				return false;
		//			}
		//			else
		//			{
		//				m_lstCryptReport.InsertItem(0, m_csFileFolderPath);

		//				m_lstCryptReport.SetItemText(0, 1, theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_FILE_SIZE_EXCEED"));
		//				return false;
		//			}
		//		}
		//		if (dwFileSize == 0x02)
		//		{
		//			if (theApp.m_bIsNoGui)
		//			{
		//				SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, INSERT_NEW_ITEM);
		//				//						swprintf_s(szPercentageDone, _countof(szPercentageDone), L"Compressing %d%% ", 0);
		//				//						SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szPercentageDone, ENC_INPROGRESS, 0);

		//				SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, ZERO_KB_FILE);
		//				return false;
		//			}
		//			else
		//			{
		//				m_lstCryptReport.InsertItem(0, m_csFileFolderPath);

		//				m_lstCryptReport.SetItemText(0, 1, theApp.m_objwardwizLangManager.GetString(L"IDS_DATA_CRYPT_ZERO_KB_FILE"));
		//				return false;
		//			}
		//		}

		//		// avoiding encryption of wardwiz registration dll located at all drive should not get encrypted.
		//		// handled by lalit kumawat 7-20-2015
		//		if (IsWardwizFile(m_csFileFolderPath))
		//		{
		//			if (theApp.m_bIsNoGui)
		//			{
		//				SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, INSERT_NEW_ITEM);

		//				SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, WARDWIZ_DB_FILE);
		//				return false;
		//			}
		//			else
		//			{
		//				m_lstCryptReport.InsertItem(0, m_csFileFolderPath);

		//				m_lstCryptReport.SetItemText(0, 1, theApp.m_objwardwizLangManager.GetString(L"IDS_ENCR_WARDWIZ_DB_FILE"));
		//				return false;
		//			}
		//		}

		//		bIsAlreadyEncrypted = IsFileAlreadyEncrypted(m_csFileFolderPath);

		//		if (!bIsAlreadyEncrypted)
		//		{
		//			swprintf_s(szZipFilePath, _countof(szZipFilePath), L"%s", m_csFileFolderPath);
		//			m_csOrgFileName = m_csFileFolderPath.Mid(m_csFileFolderPath.ReverseFind('\\') + 1);
		//// issue- if file does not having extension then getting failed.
		////resolved by lalit kumawat 7-6-2015
		//			int iPos = 0;
		//			iPos = m_csOrgFileName.ReverseFind('.');

		//			if (iPos != -1)
		//			{
		//				csfileExtension = m_csOrgFileName.Mid(m_csOrgFileName.ReverseFind('.') + 1); // to get file extension

		//				TCHAR	*pTemp = wcsrchr(szZipFilePath, '.');
		//				if (!pTemp)
		//				{
		//					AddLogEntry(L"### Failed getting in getting commandline paramater CWardWizCryptDlg::EncryptOperation");
		//					return false;
		//				}
		//				*pTemp = '\0';

		//			}

		//			csEncryptOutputFilePath = szZipFilePath;

		//			// resolved by lalit kumawat 7-27-2015
		//			// issue :if default disk does not have required space then creating temp file on another drive which have free space

		//			if (!isbSpaceOnDefaultAvbl)
		//			{
		//				csFileNameWithoutExtension = m_csOrgFileName.Left(m_csOrgFileName.ReverseFind('.') + 0); // to get file extension

		//				swprintf_s(szTmpFilePath, _countof(szTmpFilePath), L"%s\\%s", csTempPath, csFileNameWithoutExtension);
		//				swprintf_s(szZipFilePath, _countof(szZipFilePath), L"%s.tmp", szTmpFilePath);
		//				swprintf_s(szEncFilePath, _countof(szEncFilePath), L"%s.tmp", csEncryptOutputFilePath);

		//			}
		//			else
		//			{
		//				swprintf_s(szTmpFilePath, _countof(szTmpFilePath), L"%s", szZipFilePath);
		//				swprintf_s(szZipFilePath, _countof(szZipFilePath), L"%s.tmp", szZipFilePath);
		//				swprintf_s(szEncFilePath, _countof(szEncFilePath), L"%s.tmp", csEncryptOutputFilePath);
		//			}
		//			

		//			m_csSelectedFilePath = m_csFileFolderPath;

		//			swprintf_s(szFileStatus, _countof(szFileStatus), L"%s", m_csFileFolderPath);
		//			swprintf_s(szPercentageDone, _countof(szPercentageDone), L"Compressing %d%% ", 0);
		//			
		//			if(!theApp.m_bIsNoGui)
		//			{
		//				m_lstCryptReport.InsertItem(0, szFileStatus);
		//			}

		//			if (theApp.m_bIsNoGui)
		//			{
		//				SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szFileStatus, INSERT_NEW_ITEM);
		//				SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szPercentageDone, ENC_INPROGRESS, 0);

		//			}
		//			if (!theApp.m_bIsNoGui)
		//			{
		//				m_lstCryptReport.SetItemText(0, 1, szPercentageDone);  // to show on ui with 0 percente at starting.
		//			}

		//			swprintf_s(szOldFilePath, _countof(szOldFilePath), L"%s.tmp.WWIZ", csEncryptOutputFilePath);

		//			if (iPos != -1)
		//			{
		//				swprintf_s(szNewFilePath, _countof(szNewFilePath), L"%s_%s.WWIZ", csEncryptOutputFilePath, csfileExtension);
		//			}
		//			else
		//			{
		//				swprintf_s(szNewFilePath, _countof(szNewFilePath), L"%s.WWIZ", csEncryptOutputFilePath);
		//			}

		//			if (PathFileExists(szNewFilePath))
		//			{

		//				if (theApp.m_bIsNoGui)
		//				{
		//					SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szNewFilePath, SAVE_AS);

		//					// issue- during saveAs option user clicked on cancled option then file Encryption option need to cancled.
		//					// resolved by lalit kumawat 7-4-2015
		//					if (m_csSaveAsPath == L"")
		//					{
		//						SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, OPR_CANCELED);
		//						return true;
		//					}
		//					isSameSaveAsDirectory = IsPathBelongToSameCurntDirectory(szNewFilePath, m_csSaveAsPath);
		//					swprintf_s(szNewFilePath, _countof(szNewFilePath), L"%s", m_csSaveAsPath);
		//					// file CopyFile code..
		//				}
		//			}
		//			else
		//			{
		//				isSameSaveAsDirectory = true;
		//			}
		//			SetTimer(TIMER_SHOW_COMPRS_STATUS, 2000, NULL);

		//			StartUpLogEntry();

		//			m_csCurntArchTmpFilePath = szZipFilePath;
		//			m_bIsComDomStarted = true;

		//			if (m_pThis->m_hThreadTimer != NULL)
		//			{
		//				ResumeThread(m_pThis->m_hThreadTimer);
		//			}
		//			
		//			if (m_CWardWizCompressor.CreateArchive(szZipFilePath, m_csFileFolderPath))
		//			{
		//				KillTimer(TIMER_SHOW_COMPRS_STATUS);
		//				EndingLogEntry();
		//				//issue xyz.temp.wwiz file not getting deleted if user select save as option 
		//				// issue resolved by lalit kumawat 7-3-2015
		//				m_csCurntEncFilePath = szOldFilePath;
		//				m_csFinalOutputFilePath = szNewFilePath;

		//				if (m_pThis->m_hThreadTimer != NULL)
		//				{
		//					SuspendThread(m_pThis->m_hThreadTimer);
		//				}

		//				if (m_bIsAborted)
		//					return false;

		//				m_bIsEnDecStarted = true;

		//				

		//				bReturn = EncryptSingleFile(szZipFilePath, szEncFilePath, m_csOrgFileName);			// file encryption
		//				// issue - encrypted file integrity handling
		//				
		//				if (bReturn)
		//				{
		//					if (theApp.m_bIsNoGui)
		//					{
		//						SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szZipFilePath, FILE_LOCKING);

		//					}

		//					if (!m_objWardWizCrypter.AddCheckSum(szOldFilePath))
		//					{
		//						if (theApp.m_bIsNoGui)
		//						{
		//							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szZipFilePath, FILE_LOCKING_SUCCESS);

		//						}
		//					}
		//					else
		//					{
		//						if (theApp.m_bIsNoGui)
		//						{
		//							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szZipFilePath, OPR_FAILED);

		//						}

		//						// need to handle crc faild cleanup required.. 
		//					}
		//				}

		//				if (PathFileExists(szZipFilePath))
		//				{
		//					DeleteFile(szZipFilePath);

		//				}

		//				if (PathFileExists(szOldFilePath))
		//				{
		//					if (isSameSaveAsDirectory)
		//					{
		//						if (PathFileExists(szNewFilePath))
		//						{
		//							DeleteFile(szNewFilePath);
		//						}
		//						_wrename(szOldFilePath, szNewFilePath);
		//					}
		//					
		//					else
		//					{
		//						CopyFile(szOldFilePath, szNewFilePath, false);
		//						DeleteFile(szOldFilePath);
		//					}

		//					return true;
		//				}
		//				ZeroMemory(szNewFilePath, _countof(szNewFilePath));
		//				return bReturn;
		//			}
		//			else
		//			{

		//				EndingLogEntry();
		//				KillTimer(TIMER_SHOW_COMPRS_STATUS);

		//				m_csCurntArchTmpFilePath = L"";
		//				swprintf_s(szOldFilePath, _countof(szOldFilePath), L"%s.WWIZ", m_csFileFolderPath);
		//				swprintf_s(szNewFilePath, _countof(szNewFilePath), L"%s.WWIZ", szTmpFilePath);

		//				m_csCurntEncFilePath = szOldFilePath;
		//				m_csFinalOutputFilePath = szNewFilePath;

		//				if (m_pThis->m_hThreadTimer != NULL)
		//				{
		//					SuspendThread(m_pThis->m_hThreadTimer);
		//				}

		//				bReturn = EncryptSingleFile(m_csFileFolderPath, csEncryptOutputFilePath, m_csOrgFileName);

		//				if (PathFileExists(szOldFilePath))
		//				{
		//					if (isSameSaveAsDirectory)
		//					{
		//						if (PathFileExists(szNewFilePath))
		//						{
		//							DeleteFile(szNewFilePath);
		//						}
		//						_wrename(szOldFilePath, szNewFilePath);
		//					}
		//						
		//					else
		//					{
		//						CopyFile(szOldFilePath, szNewFilePath, false);
		//						DeleteFile(szOldFilePath);
		//					}

		//					return true;
		//				}
		//				//m_csCurntEncFilePath = L"";
		//				return bReturn;
		//			}
		//		}
		//		else
		//		{
		//			CString csStatus;
		//			csStatus.Format(L"%s", m_csFileFolderPath);
		//			AddLogEntry(csStatus, 0, 0, true, SECONDLEVEL);
		//			
		//			if (!theApp.m_bIsNoGui)
		//			{
		//				m_lstCryptReport.InsertItem(0, m_csFileFolderPath);
		//				UpdateStatus(csStatus);

		//				m_lstCryptReport.SetItemText(0, 1, L"Already Encrypted");
		//			}

		//			if (theApp.m_bIsNoGui)
		//			{
		//				SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, INSERT_NEW_ITEM);
		//				SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, ALREADY_ENCRYPTED);
		//			}
		//			AddLogEntry(L"### File Already Encrypted [File: %s]", m_csFileFolderPath);
		//			return bReturn;
		//		}
		//		bIsAlreadyEncrypted = false;

		//		if (!m_bIsAborted)
		//		{   // lalit kumawat
		//			// issue:- if both orginal file and encrypted file present in folder and i go for again file encryption to org file now abort then it will delete previous encrypted file that should not happen.
		//			m_bIsEnDecStarted = false;
		//			m_bIsComDomStarted = false;
		//		}
		//	}
		//}

		if (IsPathBelongsToWardWizDir(m_csFileFolderPath))
		{
			m_bIsFileFromOSPath = true;

			if (theApp.m_bIsNoGui)
			{
				SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, SYSTEM_FILE_FOLDER_PATH);
				return false;
			}
			else
			{
				MessageBox(L"Access Denied: Encryption for Operating system file(s) or folder(s) not allowed.");
				return false;
			}
		}
		else if (IsPathBelongsToOSReservDirectory(m_csFileFolderPath))
		{
			m_bIsFileFromOSPath = true;

			if (theApp.m_bIsNoGui)
			{
				SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, SYSTEM_FILE_FOLDER_PATH);
				return false;
			}
			else
			{
				MessageBox(L"Access Denied: Encryption for Operating system file(s) or folder(s) not allowed.");
				return false;
			}
		}
		else
		{
			// following  issue : resolved by lalit kumawat 7-22-2015 
			//1.Select file for encryption.(I selected file which are present in USB).
			//	2.Once encryption process starts it will again take same file and show.WWIZ extension.
			//CFileFind finder;

			//// build a string with wildcards
			//CString strWildcard(m_csFileFolderPath);
			//strWildcard += _T("\\*.*");

			//// start working for files
			//BOOL bWorking = finder.FindFile(strWildcard);
			/*while (bWorking)
			{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
			continue;*/

			// resolved by lalit kumawat 7-27-2015
			// issue :if default disk does not have required space then creating temp file on another drive which have free space

			bool isbSpaceAvailable = false;
			bool bIsMiniMumSpaceOnSame = false;
			TCHAR szDrive = 'A';
			TCHAR szDefaultTmpDrive = 'D';
			GetAllSystemDrive();
			szDrive = GetDefaultDrive(m_csFileFolderPath);
			m_vcsListOfDrive.push_back(szDrive);
			szDefaultTmpDrive = szDrive;

			bIsMiniMumSpaceOnSame = IsDriveHaveRequiredSpace(szDrive, m_csFileFolderPath, EQUAL_SIZE);

			if (!bIsMiniMumSpaceOnSame)
			{
				SendLowDiskSpaceMessageToGUI();
				return false; // disk space low error.
			}


			isbSpaceAvailable = IsSizeForTempAvble();

			if (!isbSpaceAvailable)
			{
				SendLowDiskSpaceMessageToGUI();
				return false;
			}

			for (int iListItm = 0; iListItm < static_cast<int>(m_vcsListOfFilePath.size()); iListItm++)
			{
				// if it's a directory, recursively search it 
				/*if (!finder.IsDirectory())
				{*/
				//CString csFilePath = finder.GetFilePath();
				if (m_bIsAborted)
					return false;

				CString csFilePath = m_vcsListOfFilePath.at(iListItm);


				TCHAR szOldFilePath[MAX_PATH] = { 0 };
				TCHAR szNewFilePath[MAX_PATH] = { 0 };

				if (PathFileExists(csFilePath))
				{
					TCHAR  szZipFilePath[MAX_PATH] = { 0 };
					TCHAR szFileStatus[MAX_PATH] = { 0 };
					bIsAlreadyEncrypted = false;
					TCHAR szTmpFilePath[MAX_PATH] = { 0 };
					//TCHAR  szZipFilePath[MAX_PATH] = { 0 };
					TCHAR szPercentageDone[MAX_PATH] = { 0 };
					bool  isSameSaveAsDirectory = false;
					CString csfileExtension = L"";
					CString csEncryptOutputFilePath = L"";
					TCHAR szEncFilePath[MAX_PATH] = { 0 };
					bool bIsMiniMumSpace = false;
					bool isbSpaceAvailable = false;
					bool isbSpaceOnDefaultAvbl = false;
					CString csFileNameWithoutExtension = L"";
					CString csTempPath = L"";
					TCHAR szDrive = 'A';

					if (!theApp.m_bIsNoGui)
					{
						m_lstCryptReport.InsertItem(0, szFileStatus);
					}


					swprintf_s(szPercentageDone, _countof(szPercentageDone), L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_CRYPT_COMPRESSING"));

					if (!theApp.m_bIsNoGui)
					{
						m_lstCryptReport.SetItemText(0, 1, szPercentageDone);
					}

					swprintf_s(szZipFilePath, _countof(szZipFilePath), L"%s", csFilePath);
					m_csOrgFileName = csFilePath.Mid(csFilePath.ReverseFind('\\') + 1);

					// resolved by lalit kumawat 7-27-2015
					// issue :if default disk does not have required space then creating temp file on another drive which have free space

					GetAllSystemDrive();
					szDrive = GetDefaultDrive(csFilePath);
					m_vcsListOfDrive.push_back(szDrive);

					isbSpaceOnDefaultAvbl = IsDriveHaveRequiredSpace(szDrive, csFilePath, TRIPLE_SIZE); // 3 for maximum space for all operation on same disk


					if (!isbSpaceOnDefaultAvbl)
					{
						GetSystemTempPath(csFilePath, csTempPath);
					}

					// issue :- file size exceed to 3GB size handline ,now we are not allowing encryption/decryption if file size more then 3GB
					// resolved by lalit kumawat, 7-9-2015
					// issue :- file size is 0KB, then give proper message on UI
					// resolved by Nitin Kolapkar 21st July 2015
					DWORD dwFileSize = IsFileSizeMorethen3G(csFilePath);
					if (dwFileSize == 0x01)
					{
						if (theApp.m_bIsNoGui)
						{
							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, INSERT_NEW_ITEM);
							swprintf_s(szPercentageDone, _countof(szPercentageDone), L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_CRYPT_COMPRESSING"));
							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szPercentageDone, ENC_INPROGRESS, 0);

							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, FILE_SIZE_MORETHEN_3GB);
							goto outlbl;
						}
						else
						{
							m_lstCryptReport.InsertItem(0, csFilePath);
							m_lstCryptReport.SetItemText(0, 1, theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_FILE_SIZE_EXCEED"));
							goto outlbl;
						}
					}
					if (dwFileSize == 0x02)
					{
						if (theApp.m_bIsNoGui)
						{
							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, INSERT_NEW_ITEM);
							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, ZERO_KB_FILE);
							goto outlbl;
						}
						else
						{
							m_lstCryptReport.InsertItem(0, csFilePath);

							m_lstCryptReport.SetItemText(0, 1, theApp.m_objwardwizLangManager.GetString(L"IDS_DATA_CRYPT_ZERO_KB_FILE"));
							goto outlbl;
						}
					}
					if (dwFileSize == 0x05)
					{
						if (theApp.m_bIsNoGui)
						{
							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, INSERT_NEW_ITEM);
							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, WARDWIZ_DB_FILE);
							goto outlbl;
						}
						else
						{
							m_lstCryptReport.InsertItem(0, m_csFileFolderPath);

							m_lstCryptReport.SetItemText(0, 1, theApp.m_objwardwizLangManager.GetString(L"WARDWIZ_DB_FILE"));
							return false;
						}
					}
					// avoiding encryption of wardwiz registration dll located at all drive should not get encrypted.
					// handled by lalit kumawat 7-20-2015

					if (IsWardwizFile(csFilePath))
					{
						if (theApp.m_bIsNoGui)
						{
							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, INSERT_NEW_ITEM);

							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, WARDWIZ_DB_FILE);
							goto outlbl;
						}
						else
						{
							m_lstCryptReport.InsertItem(0, csFilePath);

							m_lstCryptReport.SetItemText(0, 1, theApp.m_objwardwizLangManager.GetString(L"IDS_ENCR_WARDWIZ_DB_FILE"));
							goto outlbl;
						}
					}
					//Issue: Encryption Key Changed for New Implementation. SO need to check the specific Key at the time of Decryption
					//Return types : If not encrypted = 0x00;  If encrypted by new version : 0x01; If encrypted by old version : 0x02; If failed : 0x03;
					//Resolved By: Nitin K. Date: 14th August 2015
					DWORD dwRetValue = 0x00;
					dwRetValue = IsFileAlreadyEncrypted(csFilePath);

					int iPos = 0;
					iPos = m_csOrgFileName.ReverseFind('.');

					if (dwRetValue == 0x00)
					{
						if (iPos != -1)
						{

							TCHAR	*pTemp = wcsrchr(szZipFilePath, '.');
							if (!pTemp)
							{
								AddLogEntry(L"### Failed to get path of Orginal file name  CWardWizCryptDlg::EncryptOperation");
								goto outlbl;
							}
							*pTemp = '\0';

							csfileExtension = m_csOrgFileName.Mid(m_csOrgFileName.ReverseFind('.') + 1); // to get file extension
						}

						csEncryptOutputFilePath = szZipFilePath;
						if (!isbSpaceOnDefaultAvbl)
						{
							csFileNameWithoutExtension = m_csOrgFileName.Left(m_csOrgFileName.ReverseFind('.') + 0); // to get file extension

							swprintf_s(szTmpFilePath, _countof(szTmpFilePath), L"%s\\%s", csTempPath, csFileNameWithoutExtension);
							swprintf_s(szZipFilePath, _countof(szZipFilePath), L"%s.tmp", szTmpFilePath);
							swprintf_s(szEncFilePath, _countof(szEncFilePath), L"%s.tmp", csEncryptOutputFilePath);

						}
						else
						{
							swprintf_s(szTmpFilePath, _countof(szTmpFilePath), L"%s", szZipFilePath);
							swprintf_s(szZipFilePath, _countof(szZipFilePath), L"%s.tmp", szZipFilePath);
							swprintf_s(szEncFilePath, _countof(szEncFilePath), L"%s.tmp", csEncryptOutputFilePath);
						}

						//swprintf_s(szTmpFilePath, _countof(szTmpFilePath), L"%s", szZipFilePath);
						//swprintf_s(szZipFilePath, _countof(szZipFilePath), L"%s.tmp", szZipFilePath);
						swprintf_s(szFileStatus, _countof(szFileStatus), L"%s", csFilePath);
						swprintf_s(szOldFilePath, _countof(szOldFilePath), L"%s.tmp.AK", csEncryptOutputFilePath);

						if (iPos != -1)
						{
							swprintf_s(szNewFilePath, _countof(szNewFilePath), L"%s_%s.AK", csEncryptOutputFilePath, csfileExtension);
						}
						else
						{
							swprintf_s(szNewFilePath, _countof(szNewFilePath), L"%s.AK", csEncryptOutputFilePath);
						}


						m_csCurntEncFilePath = szOldFilePath;
						m_csFinalOutputFilePath = szNewFilePath;

						m_csSelectedFilePath = csFilePath;

						if (theApp.m_bIsNoGui)
						{
							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, INSERT_NEW_ITEM);
							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szPercentageDone, ENC_INPROGRESS, 0);

						}

						if (PathFileExists(szNewFilePath))
						{
							if (theApp.m_bIsNoGui)
							{
								SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szNewFilePath, SAVE_AS);

								if (m_csSaveAsPath == L"")
								{
									SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, OPR_CANCELED);
									goto outlbl;
								}

								isSameSaveAsDirectory = IsPathBelongToSameCurntDirectory(szNewFilePath, m_csSaveAsPath);
								swprintf_s(szNewFilePath, _countof(szNewFilePath), L"%s", m_csSaveAsPath);
								// file CopyFile code..
							}
						}
						else
						{
							isSameSaveAsDirectory = true;
						}

						StartUpLogEntry();
						SetTimer(TIMER_SHOW_COMPRS_STATUS, 2000, NULL);

						if (m_pThis->m_hThreadTimer != NULL)
						{
							ResumeThread(m_pThis->m_hThreadTimer);
						}
						m_csCurntArchTmpFilePath = szZipFilePath;
						m_bIsComDomStarted = true;
						if (m_CWardWizCompressor.CreateArchive(szZipFilePath, csFilePath))
						{
							EndingLogEntry();
							KillTimer(TIMER_SHOW_COMPRS_STATUS);

							if (m_pThis->m_hThreadTimer != NULL)
							{
								SuspendThread(m_pThis->m_hThreadTimer);
							}

							if (m_bIsAborted)
								return false;

							m_bIsEnDecStarted = true;


							bReturn = EncryptSingleFile(szZipFilePath, szEncFilePath, m_csOrgFileName);

							if (bReturn)
							{
								if (theApp.m_bIsNoGui)
								{
									SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szZipFilePath, FILE_LOCKING);

								}

								if (!m_objWardWizCrypter.AddCheckSum(szOldFilePath))
								{
									if (theApp.m_bIsNoGui)
									{
										SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szZipFilePath, FILE_LOCKING_SUCCESS);

									}
								}
								else
								{
									if (theApp.m_bIsNoGui)
									{
										SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szZipFilePath, OPR_FAILED);

									}

									// need to handle crc faild cleanup required.. 
								}
							}

							if (PathFileExists(szZipFilePath))
							{
								DeleteFile(szZipFilePath);
							}

							m_csCurntArchTmpFilePath = L"";

							if (PathFileExists(szOldFilePath))
							{
								if (isSameSaveAsDirectory)
								{
									if (PathFileExists(szNewFilePath))
									{
										DeleteFile(szNewFilePath);
									}
									_wrename(szOldFilePath, szNewFilePath);
								}

								else
								{
									CopyFile(szOldFilePath, szNewFilePath, false);
									DeleteFile(szOldFilePath);
								}

							}

							ZeroMemory(szNewFilePath, _countof(szNewFilePath));
						}
						else
						{
							EndingLogEntry();
							KillTimer(TIMER_SHOW_COMPRS_STATUS);
							m_bIsEnDecStarted = true;

							if (m_pThis->m_hThreadTimer != NULL)
							{
								SuspendThread(m_pThis->m_hThreadTimer);
							}

							bReturn = EncryptSingleFile(csFilePath, szEncFilePath, m_csOrgFileName);
						}

						if (!m_bIsAborted)
						{   // lalit kumawat
							// issue:- if both orginal file and encrypted file present in folder and i go for again file encryption to org file now abort then it will delete previous encrypted file that should not happen.
							m_bIsEnDecStarted = false;
							m_bIsComDomStarted = false;
						}
					}
					else
					{
						CString csStatus;
						csStatus.Format(L"%s", csFilePath);
						AddLogEntry(csStatus, 0, 0, true, SECONDLEVEL);
						//m_lstCryptReport.InsertItem(0, csFilePath);
						UpdateStatus(csStatus);

						if (theApp.m_bIsNoGui)
						{
							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, INSERT_NEW_ITEM);
							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, ALREADY_ENCRYPTED);

						}
						if (!theApp.m_bIsNoGui)
						{
							m_lstCryptReport.SetItemText(0, 1, L"Already Encrypted");
						}

						if (!m_bIsAborted)
						{   // lalit kumawat
							// issue:- if both orginal file and encrypted file present in folder and i go for again file encryption to org file now abort then it will delete previous encrypted file that should not happen.
							m_bIsEnDecStarted = false;
							m_bIsComDomStarted = false;
						}
						AddLogEntry(L"### File Already Encrypted [File: %s]", csFilePath);
						//return bReturn;
					}
				outlbl:
					m_dwTotalFiles++;
				}
				//}
			}
			//finder.Close();
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCryptDlg::EncryptOperation", 0, 0, true, SECONDLEVEL);
	}

	if (m_pThis->m_vcsListOfFilePath.size() > 0)
	{
		m_pThis->m_vcsListOfFilePath.clear();
	}

	return bReturn;
}

/***************************************************************************
Function Name  : EncryptSingleFile
Description    : Function to handle Encryption of selected file.
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
bool CWardWizCryptDlg::EncryptSingleFile(CString csFilePath, CString csOutputPath, CString csOrgFileName)
{
	bool bReturn = false;


	swprintf(m_szInputFilePath, 0, L"");
	swprintf(m_szOutputFilePath, 0, L"");
	TCHAR	szPassword[33] = { 0 };
	TCHAR	szOrgTmpFileName[MAX_PATH] = { 0 };

	swprintf(m_szInputFilePath, MAX_PATH, TEXT("%s"), csFilePath);
	swprintf(m_szOutputFilePath, MAX_PATH, TEXT("%s.AK"), csOutputPath);
	_tcscpy_s(szPassword, 33, m_csPassword);

	DWORD dwFileStatus = 0;
	CString csStatus;
	csStatus.Format(L"%s", m_szInputFilePath);
	AddLogEntry(csStatus, 0, 0, true, SECONDLEVEL);

	//m_lstCryptReport.InsertItem(0, csFilePath);
	if (!theApp.m_bIsNoGui)
	{
		m_lstCryptReport.SetItemText(0, 0, csStatus);
		UpdateStatus(csStatus);
	}

	/*if (theApp.m_bIsNoGui)
	{
	SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_szInputFilePath, INSERT_NEW_ITEM);
	}*/
	swprintf(szOrgTmpFileName, MAX_PATH, TEXT("%s.tmp"), csOrgFileName);
	csOrgFileName = szOrgTmpFileName;
	//CWardWizCrypter objWardWizCrypter;
	DWORD dwRet = m_objWardWizCrypter.EncryptFile(m_szInputFilePath, csOrgFileName, m_szOutputFilePath,
		szPassword, &CWardWizCryptDlg::UpdateOpProcess, this);

	if (dwRet != 0x00)
	{
		bReturn = false;
		if (!theApp.m_bIsNoGui)
		{
			m_lstCryptReport.SetItemText(0, 1, L"Failed");
		}
		AddLogEntry(L"### Encryption Failed [File: %s]", m_szInputFilePath);

		if (theApp.m_bIsNoGui)
		{
			SendStatustoUI(dwRet, m_szInputFilePath);
		}
		return bReturn;
	}

	m_dwFilesProcessed++;

	bReturn = true;
	AddLogEntry(L"Encryption Finished", 0, 0, true, SECONDLEVEL);

	csStatus = m_bIsEncrypt ? L"Encrypted" : L"Decrypted";
	if (!theApp.m_bIsNoGui)
	{
		m_lstCryptReport.SetItemText(0, 1, csStatus);
	}

	if (theApp.m_bIsNoGui)
	{
		if (m_dwKeepOriginal == DELETEORIGINAL)
		{
			SetFileAttributes(m_csSelectedFilePath, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(m_csSelectedFilePath);
		}
		dwFileStatus = m_bIsEncrypt ? ENCRYPT_SUCCESS : DECRYPT_SUCCESS;
		SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_szInputFilePath, dwFileStatus);
	}
	return bReturn;
}

bool CWardWizCryptDlg::DecryptSingleFile(CString csFilePath, CString csTmpPath, bool bTempRequired)
{
	bool bReturn = false;



	TCHAR	szPassword[33] = { 0 };

	swprintf(m_szInputFilePath, MAX_PATH, TEXT("%s"), csFilePath);
	swprintf(m_szOutputFilePath, MAX_PATH, TEXT("%s"), csFilePath.Left(csFilePath.GetLength() - 5));
	_tcscpy_s(szPassword, 33, m_csPassword);

	DWORD dwFileStatus = 0;
	CString csStatus;
	csStatus.Format(L"Decrypting File: %s", m_szInputFilePath);
	AddLogEntry(csStatus, 0, 0, true, SECONDLEVEL);

	if (!theApp.m_bIsNoGui)
	{
		m_lstCryptReport.InsertItem(0, csFilePath);

		UpdateStatus(csStatus);
	}
	/*if (theApp.m_bIsNoGui)
	{
	SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_szInputFilePath, INSERT_NEW_ITEM);
	}*/
	//CWardWizCrypter objWardWizCrypter;

	DWORD dwRet = m_objWardWizCrypter.DecryptFile(m_szInputFilePath, m_szOutputFilePath,
		csTmpPath, bTempRequired, szPassword, &CWardWizCryptDlg::UpdateOpProcess, &CWardWizCryptDlg::FileSaveAs, this);

	m_csOrgFileNamePath = m_objWardWizCrypter.m_csOutputFrmDecrpt;

	if (dwRet != 0x00)
	{
		bReturn = false;

		CString csStatus;
		switch (dwRet)
		{
		case 0x07:
			csStatus = L"Invalid File";
			break;
		case 0x08:
			csStatus = L"Password mismatch";
			break;
		case 0x04:
			csStatus = L"Cancelled";
			break;
		case 0x0A:
			//Neha Gharge 10 july,2015 If DataEncVersionno Mismatched
			csStatus = L"Version Mismatch";
			break;
		default:
			csStatus = L"Failed";
		}

		if (!FileIntegrityRollBack())
		{
			AddFileInfoToINI(m_csFileFolderPath, m_objWardWizCrypter.m_dwFileChecksum, 0);

			if (theApp.m_bIsNoGui)
			{
				SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, INTIGRITY_ROLLBACK_FAILED);
			}
			else
			{
				m_lstCryptReport.SetItemText(0, 1, L"System reboot required for integrity rollback");
			}
		}

		if (theApp.m_bIsNoGui)
		{
			SendStatustoUI(dwRet, m_szInputFilePath);
		}

		if (!theApp.m_bIsNoGui)
		{
			m_lstCryptReport.SetItemText(0, 1, csStatus);
		}
		AddLogEntry(L"### %s [File: %s]", csStatus, m_szInputFilePath, true, SECONDLEVEL);
		return bReturn;
	}

	bReturn = true;
	AddLogEntry(L"Decryption Finished", 0, 0, true, SECONDLEVEL);


	csStatus = m_bIsEncrypt ? L"Encrypted" : L"Decrypted";
	if (!theApp.m_bIsNoGui)
	{
		m_lstCryptReport.SetItemText(0, 1, csStatus);
	}
	// issue -resolved by lalit kumawat 7-15-2015
	// issue- orginal file getting deleted even if file decryption failed 
	//during file de-compression. so it below cleanup should be done after successfull de-compression.

	//if (theApp.m_bIsNoGui)
	//{
	//	if (m_dwKeepOriginal == DELETEORIGINAL)
	//	{
	//		SetFileAttributes(m_szInputFilePath, FILE_ATTRIBUTE_NORMAL);
	//		DeleteFile(m_szInputFilePath);
	//	}

	//	dwFileStatus = m_bIsEncrypt ? ENCRYPT_SUCCESS : DECRYPT_SUCCESS;
	//	//SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_szInputFilePath, dwFileStatus);
	//}
	m_dwFilesProcessed++;

	return bReturn;
}

/***************************************************************************
Function Name  : DecryptOperation
Description    : Function to handle Decryption operation.
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
bool CWardWizCryptDlg::DecryptOperation()
{
	bool bReturn = false;
	try
	{
		// resolved by lalit kumawat 7-27-2015
		// issue :if default disk does not have required space then creating temp file on another drive which have free space
		bool isbSpaceAvailable = false;
		bool bIsMiniMumSpaceOnSame = false;
		TCHAR szDrive = L'A';
		TCHAR szDefaultTmpDrive = L'A';

		GetAllSystemDrive();
		szDrive = GetDefaultDrive(m_csFileFolderPath);
		m_vcsListOfDrive.push_back(szDrive);

		szDefaultTmpDrive = szDrive;
		bIsMiniMumSpaceOnSame = IsDriveHaveRequiredSpace(szDrive, m_csFileFolderPath, EQUAL_SIZE);

		if (!bIsMiniMumSpaceOnSame)
		{
			SendLowDiskSpaceMessageToGUI();
			return false; // disk space low error.
		}

		isbSpaceAvailable = IsSizeForTempAvble();

		if (!isbSpaceAvailable)
		{
			SendLowDiskSpaceMessageToGUI();
			return false; // disk space low error.
		}
		// following  issue : resolved by lalit kumawat 7-22-2015 
		//1.Select file for encryption.(I selected file which are present in USB).
		//	2.Once encryption process starts it will again take same file and show.AK extension.
		//CFileFind finder;

		//// build a string with wildcards
		//CString strWildcard(m_csFileFolderPath);
		//strWildcard += _T("\\*.*");

		//// start working for files
		//BOOL bWorking = finder.FindFile(strWildcard);
		//while (bWorking)
		for (int iListItm = 0; iListItm < static_cast<int>(m_vcsListOfFilePath.size()); iListItm++)
		{
			/*bWorking = finder.FindNextFile();
			if (finder.IsDots())
			continue;*/

			// if it's a directory, recursively search it 
			/*	if (!finder.IsDirectory())
				{*/
			//CString csFilePath = finder.GetFilePath();
			if (m_bIsAborted)
				return false;

			CString csFilePath = m_vcsListOfFilePath.at(iListItm);

			if (PathFileExists(csFilePath))
			{
				// issue :- file size exceed to 3GB size handline ,now we are not allowing encryption/decryption if file size more then 3GB
				// resolved by lalit kumawat, 7-9-2015
				TCHAR szPercentageDone[MAX_PATH] = { 0 };
				bool isbSpaceAvailable = false;
				bool isbSpaceOnDefaultAvbl = false;
				CString csFileNameWithoutExtension = L"";
				CString csTempPath = L"";
				TCHAR szDrive = 'D';
				CString csDecryptOutputFilePath = L"";
				TCHAR szEncFilePath[MAX_PATH] = { 0 };
				bool bIsMiniMumSpace = false;

				szDrive = GetDefaultDrive(csFilePath);

				isbSpaceOnDefaultAvbl = IsDriveHaveRequiredSpace(szDrive, csFilePath, TRIPLE_SIZE); // 3 for maximum space for all operation on same disk


				if (!isbSpaceOnDefaultAvbl)
				{
					GetSystemTempPath(csFilePath, csTempPath);
				}
				// issue :- file size is 0KB, then give proper message on UI
				// resolved by Nitin Kolapkar 21st July 2015
				DWORD dwFileSize = IsFileSizeMorethen3G(csFilePath);
				if (dwFileSize == 0x01)
				{
					if (theApp.m_bIsNoGui)
					{
						SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, INSERT_NEW_ITEM);
						swprintf_s(szPercentageDone, _countof(szPercentageDone), L"%s %d%% ", theApp.m_objwardwizLangManager.GetString(L"IDS_CRYPT_DECRYPTING"), 0);
						SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szPercentageDone, DEC_INPROGRESS, 0);

						SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, FILE_SIZE_MORETHEN_3GB);
						goto outLbl;
					}
					else
					{
						m_lstCryptReport.InsertItem(0, csFilePath);
						m_lstCryptReport.SetItemText(0, 1, theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_FILE_SIZE_EXCEED"));
						goto outLbl;
					}
				}
				if (dwFileSize == 0x02)
				{
					if (theApp.m_bIsNoGui)
					{
						SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, INSERT_NEW_ITEM);
						SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, ZERO_KB_FILE);
						goto outLbl;
					}
					else
					{
						m_lstCryptReport.InsertItem(0, csFilePath);

						m_lstCryptReport.SetItemText(0, 1, theApp.m_objwardwizLangManager.GetString(L"IDS_DATA_CRYPT_ZERO_KB_FILE"));
						goto outLbl;
					}
				}
				if (dwFileSize == 0x05)
				{
					if (theApp.m_bIsNoGui)
					{
						SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, INSERT_NEW_ITEM);
						SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, WARDWIZ_DB_FILE);
						goto outLbl;
					}
					else
					{
						m_lstCryptReport.InsertItem(0, m_csFileFolderPath);

						m_lstCryptReport.SetItemText(0, 1, theApp.m_objwardwizLangManager.GetString(L"WARDWIZ_DB_FILE"));
						return false;
					}
				}

				if (theApp.m_bIsNoGui)
				{
					SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, INSERT_NEW_ITEM);
					SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, INTIGRITY_CHECKING);
				}
				else
				{
					m_lstCryptReport.InsertItem(0, csFilePath);

					m_lstCryptReport.SetItemText(0, 1, L"Initializing..");

					// need to handle crc faild cleanup required.. 
				}
				//Issue: Encryption Key Changed for New Implementation. SO need to check the specific Key at the time of Decryption
				//Return types : If not encrypted = 0x00;  If encrypted by new version : 0x01; If encrypted by old version : 0x02; If failed : 0x03;
				//Resolved By: Nitin K. Date: 14th August 2015
				DWORD dwRet = 0x00;
				dwRet = IsFileAlreadyEncrypted(csFilePath);
				if (dwRet != 0x01 && dwRet != 0x02)
				{
					if (theApp.m_bIsNoGui)
					{
						SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, INVALID_FILE);
					}
					else
					{
						m_lstCryptReport.SetItemText(0, 1, L"Invalid file");
					}
					goto outLbl;
				}
				if (dwRet == 0x02)
				{
					if (theApp.m_bIsNoGui)
					{
						SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, FILE_ENC_USING_OLD_VERSION);

					}
					else
					{
						m_lstCryptReport.SetItemText(0, 1, L"Encrypted using Older version of Encryption");
					}
					goto outLbl;
				}

				DWORD dwIntegrtyRet = 0;

				dwIntegrtyRet = m_objWardWizCrypter.CheckFileIntegrityBeforeDecryption(csFilePath);

				if (!dwIntegrtyRet)
				{
					if (theApp.m_bIsNoGui)
					{
						SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szPercentageDone, DEC_INPROGRESS, 0);

					}
				}
				else
				{
					if (theApp.m_bIsNoGui)
					{
						if (dwIntegrtyRet == 0x04) // integrity rollback failed
						{
							AddFileInfoToINI(m_csFileFolderPath, m_objWardWizCrypter.m_dwFileChecksum, 0);
							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, INTIGRITY_ROLLBACK_FAILED);
						}
						else
						{
							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csFilePath, INTIGRITY_FAILED);
						}
					}
					else
					{
						if (dwIntegrtyRet == 0x04) // integrity rollback failed
						{
							m_lstCryptReport.SetItemText(0, 1, L"System reboot required for integrity rollback");
						}
						else
						{
							m_lstCryptReport.SetItemText(0, 1, L"Integrity checking failed");
						}

					}
					goto outLbl;
					// need to handle crc faild cleanup required.. 
				}

				if (m_pThis->m_hThreadTimer != NULL)
				{
					SuspendThread(m_pThis->m_hThreadTimer);
				}
				m_csFileFolderPath = csFilePath;

				bool bTempRequired = true; // temp required 

				if (isbSpaceOnDefaultAvbl)
				{
					csTempPath = m_csFileFolderPath;
					bTempRequired = false;
				}


				bReturn = DecryptSingleFile(csFilePath, csTempPath, bTempRequired);

				bool bIsValidForDecom = false;

				if (bReturn)
				{
					bIsValidForDecom = IsFileValidForDecompress(m_csOrgFileNamePath); // to check if file ecrypted with old implementation.

					if (bIsValidForDecom)
					{
						TCHAR	szExtractedPath[1024] = { 0 };
						TCHAR	szNormalFilePath[1024] = { 0 };
						TCHAR szFileStatus[MAX_PATH] = { 0 };
						CString csFileName = L"";

						swprintf_s(szExtractedPath, _countof(szExtractedPath), L"%s", csFilePath);
						swprintf_s(szFileStatus, _countof(szFileStatus), L" %s", szExtractedPath);
						swprintf_s(szExtractedPath, _countof(szExtractedPath), L"%s", m_csOrgFileNamePath);

						// resolved by lalit kumawat 7-27-2015
						// issue :if default disk does not have required space then creating temp file on another drive which have free space
						if (!isbSpaceOnDefaultAvbl)
						{
							csTempPath = csFilePath;
							csFileName = m_csOrgFileNamePath.Mid(m_csOrgFileNamePath.ReverseFind('\\') + 1);
							csTempPath = csTempPath.Left(csTempPath.ReverseFind('\\') + 0);

							swprintf_s(szNormalFilePath, _countof(szNormalFilePath), L"%s\\%s", csTempPath, csFileName);

							TCHAR	*pTempPr = wcsrchr(szNormalFilePath, '.');
							if (!pTempPr)
							{
								AddLogEntry(L"### faild to create output file path and extracted file path CWardWizCryptDlg::DecryptOperation");
								return false;
							}
							*pTempPr = '\0';
						}
						else
						{
							swprintf_s(szNormalFilePath, _countof(szNormalFilePath), L"%s", m_csOrgFileNamePath);
							TCHAR	*pTempPr = wcsrchr(szNormalFilePath, '.');
							if (!pTempPr)
							{
								AddLogEntry(L"### faild to create output file path and extracted file path CWardWizCryptDlg::DecryptOperation");
								return false;
							}
							*pTempPr = '\0';
						}


						if (m_bIsAborted)
						{
							bReturn = false;
							break;
						}

						if (!theApp.m_bIsNoGui)
						{
							m_lstCryptReport.SetItemText(0, 0, szFileStatus);
						}

						if (m_pThis->m_hThreadTimer != NULL)
						{
							ResumeThread(m_pThis->m_hThreadTimer);
						}

						m_csSelectedFilePath = szExtractedPath;
						SetTimer(TIMER_SHOW_COMPRS_STATUS, 500, NULL);
						StartUpLogEntry();
						if (m_CWardWizCompressor.ExtractArchive(szExtractedPath, szNormalFilePath))
						{
							if (m_bIsAborted)
							{
								if (!FileIntegrityRollBack())
								{
									AddFileInfoToINI(m_csFileFolderPath, m_objWardWizCrypter.m_dwFileChecksum, 0);
									if (theApp.m_bIsNoGui)
									{
										SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, INTIGRITY_ROLLBACK_FAILED);
									}
									else
									{
										m_lstCryptReport.SetItemText(0, 1, L"System reboot required for integrity rollback");
									}
								}

								m_csFinalOutputFilePath = szNormalFilePath;
							}
							else
							{
								if (theApp.m_bIsNoGui)
								{
									if (m_dwKeepOriginal == DELETEORIGINAL)
									{
										SetFileAttributes(m_csFileFolderPath, FILE_ATTRIBUTE_NORMAL);
										DeleteFile(m_csFileFolderPath);
									}

									//dwFileStatus = m_bIsEncrypt ? ENCRYPT_SUCCESS : DECRYPT_SUCCESS;
									////SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_szInputFilePath, dwFileStatus);
								}
							}
							if (PathFileExists(szExtractedPath))
							{
								DeleteFile(szExtractedPath);
							}
						}
						else
						{
							//issue - rollback crc in case of de-compression aborted
							// lalit kumawat 7-15-2015
							if (m_bIsAborted)
							{
								if (!FileIntegrityRollBack())
								{
									AddFileInfoToINI(m_csFileFolderPath, m_objWardWizCrypter.m_dwFileChecksum, 0);

									if (theApp.m_bIsNoGui)
									{
										SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_csFileFolderPath, INTIGRITY_ROLLBACK_FAILED);
									}
									else
									{
										m_lstCryptReport.SetItemText(0, 1, L"System reboot required for integrity rollback");
									}
								}

								m_csFinalOutputFilePath = szNormalFilePath;
							}

							if (PathFileExists(szExtractedPath))
							{
								DeleteFile(szExtractedPath);
								_wrename(szExtractedPath, szNormalFilePath);
								m_csCurntEncFilePath = L"";
							}

						}

						m_csSelectedFilePath = szNormalFilePath;
						if (!theApp.m_bIsNoGui)
						{
							m_lstCryptReport.SetItemText(0, 1, L"Decrypted");
						}
						if (m_pThis->m_hThreadTimer != NULL)
						{
							SuspendThread(m_pThis->m_hThreadTimer);
						}
						if (!m_bIsAborted)
						{
							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_szInputFilePath, DECRYPT_SUCCESS);
						}
						EndingLogEntry();
						KillTimer(TIMER_SHOW_COMPRS_STATUS);
					}
					else
					{
						if (!theApp.m_bIsNoGui)
						{
							m_lstCryptReport.SetItemText(0, 1, L"Decrypted");
						}
						if (m_pThis->m_hThreadTimer != NULL)
						{
							SuspendThread(m_pThis->m_hThreadTimer);
						}
						SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, m_szInputFilePath, DECRYPT_SUCCESS);
					}
				}
				else
				{
					if (!theApp.m_bIsNoGui)
					{
						m_lstCryptReport.SetItemText(0, 1, L"Invalid File");
					}
				}

			outLbl:

				bIsValidForDecom = false;
				m_dwTotalFiles++;
			}
			//	}
		}
		//finder.Close();

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCryptDlg::DecryptOperation", 0, 0, true, SECONDLEVEL);
	}

	if (m_pThis->m_vcsListOfFilePath.size() > 0)
	{
		m_pThis->m_vcsListOfFilePath.clear();
	}

	return bReturn;
}

/***************************************************************************
Function Name  : UpdateResults
Description    : Function to Display results on UI.
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
bool CWardWizCryptDlg::UpdateResults()
{
	bool bReturn = false;

	CString csEntry;
	csEntry.Format(L"%d", m_dwFilesProcessed);
	if (!theApp.m_bIsNoGui)
	{
		m_stFilesProcessed.SetWindowText(csEntry);
	}

	csEntry.Format(L"%d", m_dwTotalFiles);
	if (!theApp.m_bIsNoGui)
	{
		m_stTotalFilesCount.SetWindowText(csEntry);
	}

	CTimeSpan m_tsScanElapsedTime = CTime::GetCurrentTime() - m_tsStartTime;
	//CString csTime =  tsScanElapsedTime.Format(_T("Elapsed Time:%H:%M:%S"));
	CString csTime = m_tsScanElapsedTime.Format(_T("%H:%M:%S"));
	AddLogEntry(L"==========================================================================", 0, 0, true, SECONDLEVEL);
	AddLogEntry(L"Total Time taken: %s", csTime, 0, true, SECONDLEVEL);
	AddLogEntry(L"==========================================================================", 0, 0, true, SECONDLEVEL);

	return bReturn;
}

/***************************************************************************
Function Name  : InitializeVariables
Description    : Function to initialize variables before star operation.
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
void CWardWizCryptDlg::InitializeVariables()
{
	m_dwFilesProcessed = 0;
	m_dwTotalFiles = 0;
	ResetEvent(m_hStopEvent);
	if (!theApp.m_bIsNoGui)
	{
		m_lstCryptReport.DeleteAllItems();
	}
}


/***************************************************************************
Function Name  : UpdateStatus
Description    : Function to update the status in edit control present on main UI.
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
void CWardWizCryptDlg::UpdateStatus(CString csStatus)
{
	if (!theApp.m_bIsNoGui)
	{
		m_editStatus.SetWindowText(csStatus);
	}
}

/***************************************************************************
Function Name  : UpdateOpProcess
Description    : call back function pointer given to Encrypt/Decrypt to set
percentage.
Author Name    : Ramkrushna Shelke
SR_NO		   :
Date           : 5th Jun 2015
****************************************************************************/
void CWardWizCryptDlg::UpdateOpProcess(DWORD dwpercent, void * param)
{
	if (param == NULL)
	{
		return;
	}

	static DWORD dwPervPerc = dwpercent;
	if (dwPervPerc == dwpercent)
	{
		return;
	}
	dwPervPerc = dwpercent;

	//input CDialog pointer
	CWardWizCryptDlg *pObjCryptDlg;
	pObjCryptDlg = (CWardWizCryptDlg *)param;

	if (pObjCryptDlg == NULL)
	{
		return;
	}

	TCHAR *_curStatus = new TCHAR[128];
	if (pObjCryptDlg->m_bIsEncrypt)
	{
		_stprintf_s(_curStatus, 128, _T("%s %d%%"), theApp.m_objwardwizLangManager.GetString(L"IDS_CRYPT_ENCRYPTING"), dwpercent);
		pObjCryptDlg->SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, _curStatus, ENC_INPROGRESS, dwpercent);
	}
	else
	{
		_stprintf_s(_curStatus, 128, _T("%s %d%%"), theApp.m_objwardwizLangManager.GetString(L"IDS_CRYPT_DECRYPTING"), dwpercent);
		pObjCryptDlg->SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, _curStatus, DEC_INPROGRESS, dwpercent);
	}

	//Set status

	if (!theApp.m_bIsNoGui)
	{
		pObjCryptDlg->m_lstCryptReport.SetItemText(0, 1, _curStatus);
	}

	free(_curStatus);
}

/***********************************************************************************************
Function Name  : OnDataReceiveCallBack
Description    : Callback function to receive notification from User interface.
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 15th June 2015
***********************************************************************************************/
void CWardWizCryptDlg::OnDataReceiveCallBack(LPVOID lpParam)
{
	__try
	{
		//OutputDebugString(L">>> In CWardWizCryptDlg::OnDataReceiveCallBack");
		LPISPY_PIPE_DATA lpSpyData = (LPISPY_PIPE_DATA)lpParam;

		if (!lpSpyData)
			return;

		switch (lpSpyData->iMessageInfo)
		{
		case STOP_CRYPT_OPR:
			//Issue: UI getting unresponsive andCrypt exe not getting exited
			//Resolved By : Nitin K Date: 10th July 2015
			if (lpSpyData->dwValue == 1)
			{
				m_pThis->m_bIsManualAbort = false;
			}
			else
			{
				m_pThis->m_bIsManualAbort = true;
			}

			m_pThis->m_bIsAborted = true;
			m_pThis->m_objWardWizCrypter.m_bStopOpr = true;
			m_pThis->m_CWardWizCompressor.StopOperation();

			//m_pThis->SendCompressionAbortMsgToGUI();

			ResumeThread(m_pThis->m_hThreadCrypto);

			Sleep(500);

			m_pThis->KillTimer(TIMER_SHOW_COMPRS_STATUS);

			// issue:- during encryption abot dump getting created.its happening due to unsynchronization.
			// resolve by lalit kumawat 7-292015
			//Sleep(1000);//m_bIsEncrypt
			WaitForSingleObject(m_pThis->m_hStopEvent, INFINITE);

			if (m_pThis->m_bIsEncrypt)
			{
				if (m_pThis->m_csCurntArchTmpFilePath != L"" && PathFileExists(m_pThis->m_csCurntArchTmpFilePath))
				{
					//m_pThis->m_CWardWizCompressor.m_zip.CloseNew();
					SetFileAttributes(m_pThis->m_csCurntArchTmpFilePath, FILE_ATTRIBUTE_NORMAL);
					DeleteFile(m_pThis->m_csCurntArchTmpFilePath);
					m_pThis->m_csCurntArchTmpFilePath = L"";
				}

				if (m_pThis->m_csCurntEncFilePath != L"" && PathFileExists(m_pThis->m_csCurntEncFilePath))
				{
					DWORD dwError = 0;
					SetFileAttributes(m_pThis->m_csCurntEncFilePath, FILE_ATTRIBUTE_NORMAL);
					DeleteFile(m_pThis->m_csCurntEncFilePath);
					dwError = GetLastError();
					//	m_pThis->m_csCurntEncFilePath = L"";

				}
				if (m_pThis->m_csFinalOutputFilePath != L"" && PathFileExists(m_pThis->m_csFinalOutputFilePath))
				{
					DWORD dwError = 0;
					SetFileAttributes(m_pThis->m_csFinalOutputFilePath, FILE_ATTRIBUTE_NORMAL);
					DeleteFile(m_pThis->m_csFinalOutputFilePath);
					dwError = GetLastError();
					m_pThis->m_csFinalOutputFilePath = L"";
				}
			}
			else
			{
				//SetFileAttributes(m_pThis->m_szOutputFilePath, FILE_ATTRIBUTE_NORMAL);
				//DeleteFile(m_pThis->m_szOutputFilePath);
				Sleep(200);
				if (m_pThis->m_csFinalOutputFilePath != L"" && PathFileExists(m_pThis->m_csFinalOutputFilePath))
				{
					//m_pThis->m_CWardWizCompressor.m_zip.CloseNew();
					SetFileAttributes(m_pThis->m_csFinalOutputFilePath, FILE_ATTRIBUTE_NORMAL);
					DeleteFile(m_pThis->m_csFinalOutputFilePath);
					m_pThis->m_csFinalOutputFilePath = L"";
				}

				if (m_pThis->m_csCurntArchTmpFilePath != L"" && PathFileExists(m_pThis->m_csCurntArchTmpFilePath))
				{
					DWORD dwError = 0;
					SetFileAttributes(m_pThis->m_csCurntArchTmpFilePath, FILE_ATTRIBUTE_NORMAL);
					DeleteFile(m_pThis->m_csCurntArchTmpFilePath);
					dwError = GetLastError();
					//	m_pThis->m_csCurntEncFilePath = L"";

				}

				if (m_pThis->m_csOrgFileNamePath != L"" && PathFileExists(m_pThis->m_csOrgFileNamePath))
				{
					DWORD dwError = 0;
					SetFileAttributes(m_pThis->m_csOrgFileNamePath, FILE_ATTRIBUTE_NORMAL);
					DeleteFile(m_pThis->m_csOrgFileNamePath);
					dwError = GetLastError();
					m_pThis->m_csOrgFileNamePath = L"";

				}
			}


			if (m_pThis->m_vcsListOfFilePath.size() > 0)
			{
				m_pThis->m_vcsListOfFilePath.clear();
			}

			// issue :- GUI getting hang when user abort encryption/decryption by clicking tab menu
			// resolved by lalit kumawat 8-27-2015
			//m_pThis->OnBnClickedButtonCancel();
			//lpSpyData->dwValue = 0;
			//g_objCommServer.SendResponse(lpSpyData);
			//exit(0);
			//	m_pThis->m_bIsAborted = false;
			break;

		case PAUSE_CRYPT_OPR:
			if (m_pThis->m_hThreadCrypto != NULL)
			{
				m_pThis->KillTimer(TIMER_SHOW_COMPRS_STATUS);
				SuspendThread(m_pThis->m_hThreadCrypto);
			}
			lpSpyData->dwValue = 0;
			g_objCommServer.SendResponse(lpSpyData);
			break;

		case RESUME_CRYPT_OPR:
			if (m_pThis->m_hThreadCrypto != NULL)
			{
				m_pThis->SetTimer(TIMER_SHOW_COMPRS_STATUS, 2000, NULL);
				ResumeThread(m_pThis->m_hThreadCrypto);
			}
			lpSpyData->dwValue = 0;
			g_objCommServer.SendResponse(lpSpyData);
			break;

		default:
			AddLogEntry(L"### Exception in CWardWizCryptDlg::OnDataReceiveCallBack:: No parameters passed default case got called", 0, 0, true, SECONDLEVEL);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardWizCryptDlg::OnDataReceiveCallBack", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : SendInfo2UI
Description    : Sends notification to User interface using pipe.
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 15th June 2015
***********************************************************************************************/
bool CWardWizCryptDlg::SendInfo2UI(int iRequest, CString csFilePath, DWORD dwFileStatus, DWORD dwNewInsertItem)
{
	try
	{
		//Issue: UI getting unresponsive andCrypt exe not getting exited
		//Resolved By : Nitin K Date: 10th July 2015
		if (!m_bIsManualAbort)
		{
			ISPY_PIPE_DATA szPipeData = { 0 };
			szPipeData.iMessageInfo = iRequest;
			szPipeData.dwValue = dwFileStatus;
			szPipeData.dwSecondValue = dwNewInsertItem;
			_tcscpy_s(szPipeData.szFirstParam, _countof(szPipeData.szFirstParam), csFilePath);

			if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CClamScanner::SendMessage2UI", 0, 0, true, SECONDLEVEL);
				return false;
			}

			if (dwFileStatus == SAVE_AS)
			{
				if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
				{
					AddLogEntry(L"### Failed to send data in CDataEncryptionDlg : SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
					return false;
				}

				m_csSaveAsPath = szPipeData.szFirstParam;
				//m_objWardWizCrypter.m_csSaveAsPathDecrpt = m_csSaveAsPath;
				AddLogEntry(L">>> Received file Path: %s", szPipeData.szFirstParam, 0, true, SECONDLEVEL);
				//m_csOrgFileNamePath = m_csSaveAsPath + L".tmp";
				//SetEvent(m_pThis->m_hEvntSaveAs);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception inCWardWizCryptDlg::SendInfo2UI", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************
Function Name  : SendInfo2UI
Description    : Sends notification to User interface using with predefined constants.
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 15th June 2015
***********************************************************************************************/
void CWardWizCryptDlg::SendStatustoUI(DWORD dwRet, CString szInputFilePath)
{
	DWORD dwFileStatus = 0;

	try
	{
		DWORD dwSendUIStatus = dwRet;
		if (dwRet == 0x06)
		{
			dwFileStatus = ALREADY_ENCRYPTED;
		}
		else if (dwRet == 0x07)
		{
			dwFileStatus = INVALID_FILE;
		}
		else if (dwRet == 0x08)
		{
			dwFileStatus = PASS_MISMATCH;
		}
		else if (dwRet == 0x09)
		{
			dwFileStatus = OPR_CANCELED;
		}
		else if (dwRet == 0x0A)
		{
			//Neha Gharge 10 july,2015 If DataEncVersionno Mismatched
			m_objWardWizCrypter.CovertDataVersionNoDWORDToString();
			szInputFilePath = m_objWardWizCrypter.m_szDataEncVerfromFile;
			dwFileStatus = OPR_VERSION_MISMATCH;
		}
		else
		{
			dwFileStatus = OPR_FAILED;
		}

		if (dwSendUIStatus != 0)
		{
			SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szInputFilePath, dwFileStatus);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCryptDlg::SendStatustoUI", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : CryptOprFinished
Description    : Sends notification from User interface using with predefined constants.
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 15th June 2015
***********************************************************************************************/
void CWardWizCryptDlg::CryptOprFinished()
{
	try
	{
		if (!m_bIsFileFromOSPath)
		{
			SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, L"", CRYPT_FINISHED, 0x01);
			Sleep(900);
			if (!m_bIsAborted)
				exit(0);
		}
		else
		{
			m_bIsFileFromOSPath = false;
			exit(0);
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCryptDlg::CryptOprFinished", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : EndingLogEntry
Description    : it create the required entries into log file as footer.
percentage.
Author Name    : Lalit Kumawat
SR_NO		   :
Date           : 27th Jun 2015
****************************************************************************/
void CWardWizCryptDlg::EndingLogEntry()
{
	TCHAR szFilePath[255] = { 0 };
	TCHAR szTotalTime[255] = { 0 };
	TCHAR szFileSize[255] = { 0 };
	DWORD64 dwfileSize = 0;
	swprintf_s(szFilePath, _countof(szFilePath), L"%s.tmp", m_csSelectedFilePath);
	swprintf_s(szTotalTime, _countof(szTotalTime), L"Total time taken :  %s", (CTime::GetCurrentTime() - m_tsCmpDmpStartTime).Format(_T("%H:%M:%S")));

	GetSelectedFileSize(szFilePath, dwfileSize);

	swprintf_s(szFileSize, _countof(szFileSize), L"File size in bytes:  %d ", dwfileSize);

	TCHAR szEndTime[255] = { 0 };
	swprintf_s(szEndTime, _countof(szEndTime), L"End time:-  %s", CTime::GetCurrentTime().Format(_T("%H:%M:%S")));
	AddLogEntry(szEndTime, 0, 0, true, SECONDLEVEL);

	m_tsCmpDmpStartTime = CTime::GetCurrentTime();
	//AddLogEntry(L"--------------------------------------------------------------------------", 0, 0, true, SECONDLEVEL);
	AddLogEntry(szTotalTime, 0, 0, true, SECONDLEVEL);
	AddLogEntry(szFileSize, 0, 0, true, SECONDLEVEL);
	AddLogEntry(L"==========================================================================", 0, 0, true, SECONDLEVEL);
}

/***************************************************************************
Function Name  : StartUpLogEntry
Description    : it create the required entries into log file as header.
percentage.
Author Name    : Lalit Kumawat
SR_NO		   :
Date           : 27th Jun 2015
****************************************************************************/
void CWardWizCryptDlg::StartUpLogEntry()
{
	TCHAR szFilePathLog[255] = { 0 };
	TCHAR szFilePath[255] = { 0 };
	TCHAR szStartTime[255] = { 0 };
	TCHAR szFileSize[255] = { 0 };
	DWORD64 dwfileSize = 0;
	swprintf_s(szFilePathLog, _countof(szFilePathLog), L"Selected file path:-  %s", m_csSelectedFilePath);
	swprintf_s(szFilePath, _countof(szFilePath), L"%s", m_csSelectedFilePath);
	m_tsCmpDmpStartTime = CTime::GetCurrentTime();
	swprintf_s(szStartTime, _countof(szStartTime), L"Start time:-  %s", m_tsCmpDmpStartTime.Format(_T("%H:%M:%S")));

	GetSelectedFileSize(szFilePath, dwfileSize);

	//issue if file size more then 2 gb percentage on GUI getting more then 100% 
	// issue resolved by lalit kumawat 7-3-2015
	m_CWardWizCompressor.m_dwFileSize = dwfileSize;
	swprintf_s(szFileSize, _countof(szFileSize), L"File size in KB:  %lu", dwfileSize);

	AddLogEntry(L"==========================================================================", 0, 0, true, SECONDLEVEL);
	if (m_bIsEncrypt)
	{
		AddLogEntry(L"Operation Type : Encryption", 0, 0, true, SECONDLEVEL);
	}
	else
	{
		AddLogEntry(L"Operation Type : Decryption", 0, 0, true, SECONDLEVEL);
	}
	AddLogEntry(szFilePathLog, 0, 0, true, SECONDLEVEL);
	AddLogEntry(szStartTime, 0, 0, true, SECONDLEVEL);
	AddLogEntry(szFileSize, 0, 0, true, SECONDLEVEL);

}

/***************************************************************************
Function Name  : GetSelectedFileSize
Description    : it provides the size of selected file.
percentage.
Author Name    : Lalit Kumawat
SR_NO		   :
Date           : 27th Jun 2015
****************************************************************************/
bool CWardWizCryptDlg::GetSelectedFileSize(TCHAR *pFilePath, DWORD64 &dwFileSize)
{
	DWORD64 dwfolderSize = 0;

	try
	{
		if (!PathIsDirectory(pFilePath))
		{

			HANDLE hFile = CreateFile(pFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile == INVALID_HANDLE_VALUE)
			{
				DWORD dwError = GetLastError(); 
				AddLogEntry(L"### GetSelectedFileSize : Error in opening file %s", pFilePath);
				return false;
			}

			//dwFileSize = GetFileSize(hFile, NULL);

			//issue if file size more then 2 gb percentage on GUI getting more then 100% 
			// issue resolved by lalit kumawat 7-3-2015

			//Function to handle file size more than 2 GB
			LARGE_INTEGER szLargeInt;
			if (GetFileSizeEx(hFile, &szLargeInt) == FALSE)
			{
				return false;
			}

			dwFileSize = szLargeInt.QuadPart;
			//m_CWardWizCompressor.m_dwFileSize = szLargeInt.QuadPart;
			//dwFileType = GetFileType(hFile);
			CloseHandle(hFile);
		}
		else
		{
			for (int iListItm = 0; iListItm < static_cast<int>(m_vcsListOfFilePath.size()); iListItm++)
			{

				HANDLE hFile = CreateFile(m_vcsListOfFilePath.at(iListItm), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile == INVALID_HANDLE_VALUE)
				{
					DWORD dwError = GetLastError();
					AddLogEntry(L"### GetSelectedFileSize : Error in opening file %s", pFilePath);
					return false;
				}

				//Function to handle file size more than 2 GB
				LARGE_INTEGER szLargeInt;
				if (GetFileSizeEx(hFile, &szLargeInt) == FALSE)
				{
					return false;
				}

				dwfolderSize = dwfolderSize + szLargeInt.QuadPart;
				CloseHandle(hFile);
			}

			dwFileSize = dwfolderSize;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCryptDlg::GetSelectedFileSize");
		return false;
	}
	return true;
}

/***************************************************************************
Function Name  : GetEmptyDrivePath
Description    : it provides the other empty drive if current drive full.
percentage.
Author Name    : Lalit Kumawat
SR_NO		   :
Date           : 27th Jun 2015
****************************************************************************/
bool CWardWizCryptDlg::IsDriveHaveRequiredSpace(TCHAR szDrive, CString csFilePath, int iSpaceRatio)
{
	bool bReturn = false;
	bool isbSpaceAvailable = false;
	try
	{
		CString csDrive;

		DWORD64 TotalNumberOfFreeBytes;
		csDrive.Format(L"%c:", szDrive);

		if (PathFileExists(csDrive))
		{
			if (!GetDiskFreeSpaceEx((LPCWSTR)csDrive, &m_uliFreeBytesAvailable, &m_uliTotalNumberOfBytes, &m_uliTotalNumberOfFreeBytes))
			{
				isbSpaceAvailable = false;
				bReturn = false;
				AddLogEntry(L"### Failed in  GetDiskFreeSpaceEx", 0, 0, true, SECONDLEVEL);
			}

			TotalNumberOfFreeBytes = GetTotalNumberOfFreeBytes();
			TCHAR szFilePath[255] = { 0 };
			DWORD64 dwfileSize = 0;
			swprintf_s(szFilePath, _countof(szFilePath), L"%s", csFilePath);
			GetSelectedFileSize(szFilePath, dwfileSize);
			dwfileSize = (dwfileSize * iSpaceRatio) / (1024 * 1024);
			TotalNumberOfFreeBytes = TotalNumberOfFreeBytes / (1024 * 1024);
			if (dwfileSize < TotalNumberOfFreeBytes)
			{
				m_csEmptyDrive = szDrive;
				bReturn = true;
			}
			else
			{
				m_dwRequiredSpace = dwfileSize - TotalNumberOfFreeBytes;
			}
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCryptDlg::IsDriveHaveRequiredSpace", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/***************************************************************************
Function Name  : GetTotalNumberOfFreeBytes
Description    : it return the size available on selected drive.
percentage.
Author Name    : Lalit Kumawat
SR_NO		   :
Date           : 27th Jun 2015
****************************************************************************/
DWORD64 CWardWizCryptDlg::GetTotalNumberOfFreeBytes(void)
{
	return m_uliTotalNumberOfFreeBytes.QuadPart;
}

/***************************************************************************
Function Name  : GetDefaultDrive
Description    : it provides functionality to get default working drive.
percentage.
Author Name    : Lalit Kumawat
SR_NO		   :
Date           : 27th Jun 2015
****************************************************************************/
TCHAR CWardWizCryptDlg::GetDefaultDrive(CString csFilePath)
{
	TCHAR driveName;
	try
	{

		driveName = csFilePath.GetAt(0);

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCryptDlg::GetDefaultDrive", 0, 0, true, SECONDLEVEL);
	}
	return driveName;
}

/***************************************************************************
Function Name  : OnTimer
Description    : it provides functionality to get current percentage compress or d-compression done.
percentage.
Author Name    : Lalit Kumawat
SR_NO		   :
Date           : 27th Jun 2015
****************************************************************************/
void CWardWizCryptDlg::OnTimer(UINT_PTR nIDEvent)
{
	try
	{
		if (nIDEvent == TIMER_SHOW_COMPRS_STATUS)
		{
			m_CWardWizCompressor.GetStatus();   // get the current status of percentage compression- dcompression done.
			int dwPer = 0;
			DWORD64 dwIteration = 0;
			TCHAR szFileSize[255] = { 0 };
			DWORD64 dwfileSize = 0;
			TCHAR szFilePath[MAX_PATH] = { 0 };
			TCHAR szPercentageDone[MAX_PATH] = { 0 };
			DWORD64 dwOddFileSize = 0;

			if (m_CWardWizCompressor.m_dwFileSize > 0 && m_CWardWizCompressor.m_dwBufferSize > 0 && m_CWardWizCompressor.m_dwDone > 0)
			{
				dwIteration = m_CWardWizCompressor.m_dwFileSize / m_CWardWizCompressor.m_dwBufferSize;

				dwOddFileSize = m_CWardWizCompressor.m_dwFileSize % m_CWardWizCompressor.m_dwBufferSize;

				if (!dwOddFileSize)
				{
					dwIteration = dwIteration + 1;
				}

				dwPer = (int)(((double)m_CWardWizCompressor.m_dwDone / (double)dwIteration) * 100);
				swprintf_s(szFilePath, _countof(szFilePath), L"%s", m_csSelectedFilePath);

				if (dwPer > 100)
				{
					dwPer = 100;
				}
				else if (dwPer <= 0)
				{
					dwPer = 0;
				}

				if (m_bIsEncrypt)
				{
					swprintf_s(szFileSize, _countof(szFileSize), L"File Compressing : %d%% ", dwPer);
					//swprintf_s(szPercentageDone, _countof(szPercentageDone), L"Compressing %d%% ", dwPer); 
					swprintf_s(szPercentageDone, _countof(szPercentageDone), L"%s %d%% ", theApp.m_objwardwizLangManager.GetString(L"IDS_CRYPT_COMPRESSING"), dwPer);
					if (theApp.m_bIsNoGui)
					{
						if (theApp.m_OSType != WINOS_XP && theApp.m_OSType != WINOS_XP64 && theApp.m_OSType != WINOS_VISTA)
						{
							SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szPercentageDone, ENC_INPROGRESS, dwPer);
						}
					}
					if (!theApp.m_bIsNoGui)
					{
						m_lstCryptReport.SetItemText(0, 1, szPercentageDone);
					}
				}
				else
				{
					swprintf_s(szFileSize, _countof(szFileSize), L"File de-Compressing : %d%% ", dwPer);
					//swprintf_s(szPercentageDone, _countof(szPercentageDone), L"De-compressing %d%% ", dwPer );
					swprintf_s(szPercentageDone, _countof(szPercentageDone), L"%s %d%% ", theApp.m_objwardwizLangManager.GetString(L"IDS_CRYPT_DECOMPRESSING"), dwPer);
					if (theApp.m_bIsNoGui)
					{
						SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szPercentageDone, DEC_INPROGRESS, dwPer);
					}
					if (!theApp.m_bIsNoGui)
					{
						m_lstCryptReport.SetItemText(0, 1, szPercentageDone);
					}
				}

			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCryptDlg::OnTimer");
	}

	CDialogEx::OnTimer(nIDEvent);
}

/***************************************************************************
Function Name  : IsFileAlreadyEncrypted
Description    : wrapper function to check is file already encrypted.
percentage.
Author Name    : Lalit Kumawat
SR_NO		   :
Date           : 27th Jun 2015
****************************************************************************/
DWORD CWardWizCryptDlg::IsFileAlreadyEncrypted(CString csFilePath)
{
	CWardWizCrypter objWardWizCrypter;
	DWORD dwRet = 0x00;

	//objWardWizCrypter.IsFileAlreadyEncrypted(HANDLE hFile);
	try
	{
		HANDLE hFile = CreateFile(csFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwError = GetLastError();
			AddLogEntry(L"### IsFileAlreadyEncrypted : Error in opening file %s", csFilePath);
			return false;
		}

		dwRet = objWardWizCrypter.IsFileAlreadyEncrypted(hFile);
		CloseHandle(hFile);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCryptDlg::IsFileAlreadyEncrypted");
		return false;
	}
	return dwRet;
}

/***************************************************************************
Function Name  : IsFileValidForDecompress
Description    : it provide functionality to check during decryption that encrypted file created with compression or not.
Author Name    : Lalit kumawat
SR_NO		   :
Date           : 27th Jun 2015
****************************************************************************/
bool  CWardWizCryptDlg::IsFileValidForDecompress(CString csOrgFileNamePath)
{
	bool bRet = false;
	try
	{

		CString csExtension = L"";
		csExtension = csOrgFileNamePath.Mid(csOrgFileNamePath.ReverseFind('.') + 1);
		if (csExtension == L"tmp")
			return true;
		else
			return false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CISpyGUIDlg::IsFileValidForDecompress", 0, 0, true, SECONDLEVEL);
	}
	return bRet;
}

/***************************************************************************
Function Name  : EnumerateAllFolderAndFiles
Description    : Enumerate all files/folder to get total count
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 2nd July 2015
****************************************************************************/
DWORD CWardWizCryptDlg::EnumerateAllFolderAndFiles(CString	m_csaTokenEntryFilePath[])
{
	CString csFileFolderPath = NULL;
	DWORD dwFileCount = 0;

	if (m_vcsListOfFilePath.size() > 0)
	{
		m_vcsListOfFilePath.clear();
	}

	for (int i = 0; m_csaTokenEntryFilePath[i] != "\0"; i++)
	{
		if (PathFileExists(m_csaTokenEntryFilePath[i]))
		{
			csFileFolderPath = m_csaTokenEntryFilePath[i];
		}
		else
		{
			continue;
		}
		if (!PathIsDirectory(csFileFolderPath))
		{
			dwFileCount++;
			m_vcsListOfFilePath.push_back(csFileFolderPath);
		}
		else
		{
			CFileFind finder;

			// build a string with wildcards
			CString strWildcard(csFileFolderPath);
			strWildcard += _T("\\*.*");

			// start working for files
			BOOL bWorking = finder.FindFile(strWildcard);
			while (bWorking)
			{
				bWorking = finder.FindNextFile();
				if (finder.IsDots())
					continue;

				// if it's a directory, recursively search it 
				if (!finder.IsDirectory())
				{
					CString csFilePath = finder.GetFilePath();
					if (PathFileExists(csFilePath))
					{
						dwFileCount++;
						m_vcsListOfFilePath.push_back(csFilePath);
					}
				}
			}
			finder.Close();
		}

	}
	return dwFileCount;
}

/***************************************************************************
Function Name  : SendCompressionAbortMsgToGUI
Description    : it provides functionality to to send abort message to GUI because CString not allowed in Structured exception haldline so it used as wrapper
Author Name    : Lalit kumawat
SR_NO		   :
Date           : 27th Jun 2015
****************************************************************************/
void  CWardWizCryptDlg::SendCompressionAbortMsgToGUI()
{
	//Issue: UI getting unresponsive andCrypt exe not getting exited
	//Resolved By : Nitin K Date: 10th July 2015
	SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, L"", OPR_CANCELED);
	//SendStatustoUI(9, L"");   //0x09 refer for canceled message
}

/***************************************************************************
Function Name  : IsPathBelongToSameCurntDirectory
Description    : it provides functionality to check whether save as path and previous path belongs to same directory or not
Author Name    : Lalit kumawat
SR_NO		   :
Date           : 27th Jun 2015
****************************************************************************/
bool  CWardWizCryptDlg::IsPathBelongToSameCurntDirectory(CString cs_oldPath, CString cs_newPath)
{
	bool bRet = false;
	try
	{
		cs_oldPath = cs_oldPath.Left(cs_oldPath.ReverseFind(_T('\\')) + 1);
		cs_newPath = cs_newPath.Left(cs_oldPath.ReverseFind(_T('\\')) + 1);

		if (cs_oldPath == cs_newPath)
			return true;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CISpyGUIDlg::IsPathBelongToSameCurntDirectory", 0, 0, true, SECONDLEVEL);
	}

	return bRet;
}

/***************************************************************************
Function Name  : FileSaveAs
Description    : it provides functionality for save As during decrypt option if file already exists. and send GUI to show SaveAs fileDialog popup
Author Name    : Lalit kumawat
SR_NO		   :
Date           : 7-4-2015
****************************************************************************/
void  CWardWizCryptDlg::FileSaveAs(LPTSTR lpFilePath, void * param)
{
	if (param == NULL)
	{
		return;
	}

	CWardWizCryptDlg *pObjCryptDlg;
	pObjCryptDlg = (CWardWizCryptDlg *)param;

	if (pObjCryptDlg == NULL)
	{
		return;
	}

	TCHAR szOutputFilePath[MAX_PATH] = { 0 };
	swprintf_s(szOutputFilePath, _countof(szOutputFilePath), L"%s", lpFilePath);

	TCHAR	*pTempPr = wcsrchr(szOutputFilePath, '.');
	if (!pTempPr)
	{
		AddLogEntry(L"### faild to create output file path and extracted file path CWardWizCryptDlg::DecryptOperation");
		return;
	}
	*pTempPr = '\0';

	if (PathFileExists(szOutputFilePath))
	{
		TCHAR szNewFilePath[MAX_PATH] = { 0 };

		if (theApp.m_bIsNoGui)
		{
			pObjCryptDlg->SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, szOutputFilePath, SAVE_AS);

			pObjCryptDlg->m_objWardWizCrypter.m_csSaveAsPathDecrpt = pObjCryptDlg->m_csSaveAsPath;
			pObjCryptDlg->m_csOrgFileNamePath = pObjCryptDlg->m_csSaveAsPath + L".tmp";
		}
	}

}

/***************************************************************************
Function Name  : IsFileSizeMorethen3G
Description    : it provides the functionality to check is file having more then 3GB size,because we are not allowing en/dec if file size greater then 3GB
Author Name    : Lalit kumawat
SR_NO		   :
Date           : 7-9-2015
Modified By	   : Nitin Kolapkar
Description    : To check same file for 0KB contents
****************************************************************************/

DWORD CWardWizCryptDlg::IsFileSizeMorethen3G(CString csFilePath)
{
	DWORD dwRet = 0x00;
	try
	{

		DWORD64 dw64FileSize = 0;
		TCHAR szFilePath[MAX_PATH] = { 0 };

		swprintf_s(szFilePath, _countof(szFilePath), L"%s", csFilePath);

		GetSelectedFileSize(szFilePath, dw64FileSize);

		if (dw64FileSize > 3221225472)
		{
			dwRet = 0x01;
		}
		if (dw64FileSize == 0)
		{
			DWORD dwError = 0;
			dwError = GetLastError();
			if (dwError == 5)
			{
				dwRet = 0x05;
			}
			else
			{
				dwRet = 0x02;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCryptDlg::IsFileSizeMorethen3G", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}

/***********************************************************************************************
Function Name  : CryptOprFailed
Description    : Sends failure notification to User interface using with predefined constants.
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 15th June 2015
***********************************************************************************************/
void CWardWizCryptDlg::CryptOprFailed()
{
	try
	{
		SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, L"", CRYPT_FINISHED, 0x00);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCryptDlg::CryptOprFailed", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : FileIntegrityRollBack
Description    : Sends failure notification to User interface using with predefined constants.
SR.NO		   :
Author Name    : Lalit kumawat
Date           : 16th July 2015
***********************************************************************************************/
bool CWardWizCryptDlg::FileIntegrityRollBack()
{
	try
	{
		if (m_bIsEncrypt)//No need to rollback integrity in case of encryption.
			return true;

		if (m_objWardWizCrypter.AddCheckSum(m_csFileFolderPath, m_objWardWizCrypter.m_dwFileChecksum))
		{
			AddLogEntry(L"### failed to rollback integrity operation CWardWizCryptDlg::AddCheckSum");
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCryptDlg::FileIntegrityRollBack", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************
Function Name  : CallOnTimerThread
Description    : Thread function to handle and send Compression/Decompression status on UI
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 17th July 2015
****************************************************************************/
DWORD WINAPI CallOnTimerThread(LPVOID lpvThreadParam)
{
	CWardWizCryptDlg *pThis = (CWardWizCryptDlg*)lpvThreadParam;

	if (!pThis)
		return 0;
	do
	{
		pThis->OnTimer(TIMER_SHOW_COMPRS_STATUS);
		Sleep(2000);  // issue : resolved by lalit, for small file de-compression directly showing 100%
	} while (true);

	return 1;
}

/***********************************************************************************************
Function Name  : AddFileInfoToINI
Description    : it provides functionality to add file integrity info to ini file so it can proceed at system reboot time.
SR.NO		   :
Author Name    : Lalit kumawat
Date           : 16th July 2015
***********************************************************************************************/
bool CWardWizCryptDlg::AddFileInfoToINI(CString csfilePath, DWORD dwCRC, DWORD dwAttemp)
{
	bool bRet = false;
	HANDLE hFile = NULL;
	FILE *pInteRollBackFile = NULL;
	CString szFileInfo = L"";
	CString csIniFilePath = L"";
	TCHAR szCount[255] = { 0 };
	DWORD dwCount = 0;

	try
	{
		csIniFilePath = theApp.m_csModulePath + L"\\FAILDINTIGRITYINFO.INI";

		if (!PathFileExists(csIniFilePath))
		{
			hFile = CreateFile(csIniFilePath, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			if (hFile == NULL)
			{
				AddLogEntry(L">>> Failed during FAILDINTIGRITYINFO.INI file creation in CWardWizCryptDlg::AddFileInfoToINI", 0, 0, true, SECONDLEVEL);
				return false;
			}

			CloseHandle(hFile);

			if (!pInteRollBackFile)
				pInteRollBackFile = _wfsopen(csIniFilePath, _T("a"), 0x40);

			if (pInteRollBackFile == NULL)
			{
				AddLogEntry(L">>> Failed during FAILDINTIGRITYINFO.INI file creation in CWardWizCryptDlg::AddFileInfoToINI", 0, 0, true, SECONDLEVEL);
				return false;
			}

			szFileInfo.Format(_T("[%s]\n"), static_cast<LPCTSTR>(L"FILEINFO"));

			fputws((LPCTSTR)szFileInfo, pInteRollBackFile);

			szFileInfo.Format(_T("%s = 0\n"), static_cast<LPCTSTR>(L"COUNT"));

			fputws((LPCTSTR)szFileInfo, pInteRollBackFile);

			fflush(pInteRollBackFile);
			fclose(pInteRollBackFile);
			pInteRollBackFile = NULL;
		}


		if (!pInteRollBackFile)
			pInteRollBackFile = _wfsopen(csIniFilePath, _T("a"), 0x40);

		if (pInteRollBackFile == NULL)
		{
			AddLogEntry(L">>> Failed during FAILDINTIGRITYINFO.INI file open for write CWardWizCryptDlg::AddFileInfoToINI", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (pInteRollBackFile != NULL)
		{

			GetPrivateProfileString(L"FILEINFO", L"COUNT", L"", szCount, 255, csIniFilePath);

			swscanf(szCount, L"%lu", &dwCount);

			dwCount++;
			swprintf_s(szCount, _countof(szCount), L"%d", dwCount);

			WritePrivateProfileString(L"FILEINFO", L"COUNT", szCount, csIniFilePath);

			szFileInfo.Format(_T("%lu=%s|%lu,%lu \r\n"), dwCount, static_cast<LPCTSTR>(csfilePath), dwCRC, dwAttemp);

			fputws((LPCTSTR)szFileInfo, pInteRollBackFile);

			fflush(pInteRollBackFile);
			fclose(pInteRollBackFile);
		}

	}
	catch (...)
	{
		AddLogEntry(L">>> Exception in GetModulePath in AddFileInfoToINI", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************
Function Name  : GetEnviormentVariablesForAllMachine
Description    : this function provides functionality to find the all system defined path
Author Name    : Lalit kumawat
Date           : 7-18-2015
****************************************************************************/
void CWardWizCryptDlg::GetEnviormentVariablesForAllMachine()
{

	TCHAR		 szWindow[512];
	TCHAR		 szProgramFile[512];
	TCHAR		 szProgramFilex86[512];
	//TCHAR	 szAppData[512] ;
	TCHAR		 szAppData4[512];
	TCHAR		 szDriveName[512];
	TCHAR		 szArchitecture[512];
	//TCHAR		 szBrowseFilePath[MAX_PATH];
	TCHAR		 szUserProfile[MAX_PATH];
	TCHAR		 szAppData[MAX_PATH];
	TCHAR		 szProgramData[MAX_PATH];
	TCHAR		 szModulePath[MAX_PATH];

	try
	{
		GetEnvironmentVariable(TEXT("ALLUSERSPROFILE"), szProgramData, 511);//program data
		m_csProgramData = szProgramData;

		GetEnvironmentVariable(TEXT("USERPROFILE"), szUserProfile, 511);
		m_csUserProfile = szUserProfile;

		GetEnvironmentVariable(TEXT("windir"), szWindow, 511);//windows
		m_csWindow = szWindow;

		GetEnvironmentVariable(TEXT("APPDATA"), szAppData, 511);
		m_csAppData = szAppData;

		GetEnvironmentVariable(TEXT("PROCESSOR_ARCHITECTURE"), szAppData4, 511);
		m_csAppData4 = szAppData4;

		GetEnvironmentVariable(TEXT("SystemDrive"), szDriveName, 511);
		m_csDriveName = szDriveName;

		GetModulePath(szModulePath, MAX_PATH);
		m_csModulePath = szModulePath;

		GetEnvironmentVariable(TEXT("PROCESSOR_ARCHITEW6432"), szArchitecture, 511);
		m_csArchitecture = szArchitecture;

		GetEnvironmentVariable(TEXT("ProgramFiles"), szProgramFile, 511);
		m_csProgramFile = szProgramFile;

		GetEnvironmentVariable(TEXT("ProgramFiles(x86)"), szProgramFilex86, 511);

		if (!_tcscmp(szProgramFilex86, L""))
		{
			_tcscpy(szProgramFilex86, szProgramFile);

		}
		m_csProgramFilex86 = szProgramFilex86;

	}
	catch (...)
	{
		AddLogEntry(L">>> Exception in GetEnviormentVariablesForAllMachine", 0, 0, true, SECONDLEVEL);
	}

}

/***************************************************************************
Function Name  : IsPathBelongsToOSReservDirectory
Description    : this function provides functionality to check is selected path belongs to system drive or folder
Author Name    : Lalit kumawat
Date           : 7-18-2015
****************************************************************************/
bool CWardWizCryptDlg::IsPathBelongsToOSReservDirectory(CString csFilefolderPath)
{
	bool bRet = false;
	TCHAR szOSSystemVolumInfo[MAX_PATH] = { 0 };
	CString csSystemVolumeFolder = L"";

	try
	{
		// s1 contain s2
		swprintf_s(szOSSystemVolumInfo, _countof(szOSSystemVolumInfo), L"%s\\System Volume Information", m_csDriveName);
		csSystemVolumeFolder = szOSSystemVolumInfo;

		if ((csFilefolderPath.Find(m_csProgramData) != -1) && m_csProgramData != L"")
		{
			return true;
		}
		else if ((csFilefolderPath.Find(m_csWindow) != -1) && m_csWindow != L"")
		{
			return true;
		}
		else if ((csFilefolderPath.Find(m_csModulePath) != -1) && m_csModulePath != L"")
		{
			return true;
		}
		else if ((csFilefolderPath.Find(m_csProgramFilex86) != -1) && m_csProgramFilex86 != L"")
		{
			return true;
		}
		else if ((csFilefolderPath.Find(m_csProgramFile) != -1) && m_csProgramFile != L"")
		{
			return true;
		}
		else if (csFilefolderPath == m_csDriveName || csFilefolderPath == m_csDriveName + L"\\")
		{
			return true;
		}
		else if ((csFilefolderPath.Find(csSystemVolumeFolder) != -1) && csSystemVolumeFolder != L"")
		{
			return true;
		}

		// issue: hiding encryption option on System volume information directory in all directories
		// resolved by lalit kumawat 7-29-2015

		TCHAR szDrive = csFilefolderPath.GetAt(0);
		TCHAR szSystemVolumInfoDir[MAX_PATH] = { 0 };

		swprintf_s(szSystemVolumInfoDir, _countof(szSystemVolumInfoDir), L"%c:\\System Volume Information", szDrive);

		if ((csFilefolderPath.Find(szSystemVolumInfoDir) != -1) || csFilefolderPath == szSystemVolumInfoDir)
		{
			return true;
		}

	}
	catch (...)
	{
		AddLogEntry(L">>> Exception in IsPathBelongsToOSReservDirectory", 0, 0, true, SECONDLEVEL);
	}

	return false;
}

/***************************************************************************
Function Name  : IsWardwizFile
Description    : this function provides functionality to is selected file is operation sys file or warwiz registration db file.
Author Name    : Lalit kumawat
Date           : 7-18-2015
****************************************************************************/
bool CWardWizCryptDlg::IsWardwizFile(CString csFilefolderPath)
{
	bool bRet = false;
	CString csWrdwizDbFileName = L"VBUSERREG.DB";
	TCHAR szOSPagefile[MAX_PATH] = { 0 };
	TCHAR szOShiberfile[MAX_PATH] = { 0 };
	TCHAR szOSSwapfile[MAX_PATH] = { 0 };
	TCHAR szOSconfigFile[MAX_PATH] = { 0 };
	TCHAR szOSFilebootmgr[MAX_PATH] = { 0 };
	TCHAR szOSAUTOEXEC[MAX_PATH] = { 0 };
	TCHAR szOSawayFile[MAX_PATH] = { 0 };
	TCHAR szOSBOOTNXT[MAX_PATH] = { 0 };
	TCHAR szOSgone[MAX_PATH] = { 0 };
	TCHAR szOSBOOTSECT[MAX_PATH] = { 0 };
	TCHAR szOSIO[MAX_PATH] = { 0 };
	TCHAR szOSMSDOS[MAX_PATH] = { 0 };
	TCHAR szOSNTDETECT[MAX_PATH] = { 0 };
	TCHAR szOSntldr[MAX_PATH] = { 0 };

	try
	{
		swprintf_s(szOSPagefile, _countof(szOSPagefile), L"%s\\pagefile.sys", m_csDriveName);
		swprintf_s(szOShiberfile, _countof(szOShiberfile), L"%s\\hiberfil.sys", m_csDriveName);
		swprintf_s(szOSSwapfile, _countof(szOSSwapfile), L"%s\\swapfile.sys", m_csDriveName);
		swprintf_s(szOSconfigFile, _countof(szOSconfigFile), L"%s\\config.sys", m_csDriveName);
		swprintf_s(szOSFilebootmgr, _countof(szOSFilebootmgr), L"%s\\bootmgr", m_csDriveName);
		swprintf_s(szOSAUTOEXEC, _countof(szOSAUTOEXEC), L"%s\\AUTOEXEC.BAT", m_csDriveName);
		swprintf_s(szOSawayFile, _countof(szOSawayFile), L"%s\\away.dat", m_csDriveName);
		swprintf_s(szOSBOOTNXT, _countof(szOSBOOTNXT), L"%s\\BOOTNXT", m_csDriveName);
		swprintf_s(szOSgone, _countof(szOSgone), L"%s\\gone.dat", m_csDriveName);
		swprintf_s(szOSBOOTSECT, _countof(szOSBOOTSECT), L"%s\\BOOTSECT.BAK", m_csDriveName);
		swprintf_s(szOSIO, _countof(szOSIO), L"%s\\IO.SYS", m_csDriveName);
		swprintf_s(szOSMSDOS, _countof(szOSMSDOS), L"%s\\MSDOS.SYS", m_csDriveName);
		swprintf_s(szOSNTDETECT, _countof(szOSNTDETECT), L"%s\\NTDETECT.COM", m_csDriveName);
		swprintf_s(szOSntldr, _countof(szOSntldr), L"%s\\ntldr", m_csDriveName);


		if ((csFilefolderPath.Find(csWrdwizDbFileName) != -1) || csFilefolderPath == csWrdwizDbFileName)
		{
			return true;
		}
		else if ((csFilefolderPath.Find(szOSPagefile) != -1) || csFilefolderPath == szOSPagefile)
		{
			return true;
		}
		else if ((csFilefolderPath.Find(szOShiberfile) != -1) || csFilefolderPath == szOShiberfile)
		{
			return true;
		}
		else if ((csFilefolderPath.Find(szOSSwapfile) != -1) || csFilefolderPath == szOSSwapfile)
		{
			return true;
		}
		else if ((csFilefolderPath.Find(szOSconfigFile) != -1) || csFilefolderPath == szOSconfigFile)
		{
			return true;
		}
		else if ((csFilefolderPath.Find(szOSFilebootmgr) != -1) || csFilefolderPath == szOSFilebootmgr)
		{
			return true;
		}
		else if ((csFilefolderPath.Find(szOSAUTOEXEC) != -1) || csFilefolderPath == szOSAUTOEXEC)
		{
			return true;
		}
		else if ((csFilefolderPath.Find(szOSawayFile) != -1) || csFilefolderPath == szOSawayFile)
		{
			return true;
		}
		else if ((csFilefolderPath.Find(szOSBOOTNXT) != -1) || csFilefolderPath == szOSBOOTNXT)
		{
			return true;
		}
		else if ((csFilefolderPath.Find(szOSgone) != -1) || csFilefolderPath == szOSgone)
		{
			return true;
		}
		else if ((csFilefolderPath.Find(szOSBOOTSECT) != -1) || csFilefolderPath == szOSBOOTSECT)
		{
			return true;
		}
		else if ((csFilefolderPath.Find(szOSIO) != -1) || csFilefolderPath == szOSIO)
		{
			return true;
		}
		else if ((csFilefolderPath.Find(szOSMSDOS) != -1) || csFilefolderPath == szOSMSDOS)
		{
			return true;
		}
		else if ((csFilefolderPath.Find(szOSNTDETECT) != -1) || csFilefolderPath == szOSNTDETECT)
		{
			return true;
		}
		else if ((csFilefolderPath.Find(szOSntldr) != -1) || csFilefolderPath == szOSntldr)
		{
			return true;
		}

	}
	catch (...)
	{
		AddLogEntry(L">>> Exception in IsWardwizFile", 0, 0, true, SECONDLEVEL);
	}

	return bRet;
}

/***************************************************************************
Function Name  : GetSystemTempPath
Description    : this function provides functionality to send low space message to gui
Author Name    : Lalit kumawat
Date           : 7-28-2015
****************************************************************************/
void CWardWizCryptDlg::SendLowDiskSpaceMessageToGUI()
{
	CString csMsgToLowSpace = L"";
	csMsgToLowSpace.Format(L"( %ld MB )", m_dwRequiredSpace);
	if (theApp.m_bIsNoGui)
	{
		m_bIsFileFromOSPath = true;
		SendInfo2UI(DATA_ENC_DEC_SHOW_STATUS, csMsgToLowSpace, DISK_SPACE_LOW);
	}
	else
	{
		MessageBox(csMsgToLowSpace);
	}
}

/***************************************************************************
Function Name  : GetSystemTempPath
Description    : this function provides functionality to get temp path
Author Name    : Lalit kumawat
Date           : 7-28-2015
****************************************************************************/
bool CWardWizCryptDlg::GetSystemTempPath(CString csFileFolderPath, CString &csOutTmpPath)
{
	TCHAR szDrive = 'D';
	bool bRet = false;
	szDrive = 'A';
	CString csTemp = L"";

	try
	{
		for (int iIndex = 0; iIndex < static_cast<int>(m_vcsListOfDrive.size()); iIndex++)
		{
			CString csDrive;
			csDrive.Format(L"%c:", m_vcsListOfDrive.at(iIndex));
			szDrive = m_vcsListOfDrive.at(iIndex);

			if (PathFileExists(csDrive))
			{
				bRet = IsDriveHaveRequiredSpace(m_vcsListOfDrive.at(iIndex), csFileFolderPath, EQUAL_SIZE); // temp operation for other drive
				if (bRet)
					break;
			}
		}

		TCHAR szDriveName[5] = { 0 };
		GetEnvironmentVariable(TEXT("SystemDrive"), szDriveName, 5);
		CString csDriveLatter = L"";
		csDriveLatter = szDriveName;
		csDriveLatter = csDriveLatter.Left(csDriveLatter.ReverseFind(':') + 0);

		if (csDriveLatter.Find(szDrive) != -1)
		{
			TCHAR szTmpPath[MAX_PATH] = { 0 };
			GetEnvironmentVariable(TEXT("TEMP"), szTmpPath, MAX_PATH);
			csTemp = szTmpPath;
		}
		else
		{
			csTemp.Format(L"%c:\\Temp", szDrive);

			if (!PathFileExists(csTemp))
			{
				CreateDirectory(csTemp, NULL);
			}
		}
		csOutTmpPath = csTemp;
	}
	catch (...)
	{
		AddLogEntry(L">>> Exception in GetSystemTempPath", 0, 0, true, SECONDLEVEL);
	}

	return bRet;
}

/***************************************************************************
Function Name  : IsSizeForTempAvble
Description    : this function provides functionality to check is size for temp operation available
Author Name    : Lalit kumawat
Date           : 7-28-2015
****************************************************************************/
bool CWardWizCryptDlg::IsSizeForTempAvble()
{
	bool bRet = false;

	try
	{
		for (int iListItm = 0; iListItm < static_cast<int>(m_vcsListOfFilePath.size()); iListItm++)
		{
			for (int iIndex = 0; iIndex < static_cast<int>(m_vcsListOfDrive.size()); iIndex++)
			{
				bRet = IsDriveHaveRequiredSpace(m_vcsListOfDrive.at(iIndex), m_vcsListOfFilePath.at(iListItm), EQUAL_SIZE); // disk space require as same file size for temp 

				if (bRet)
					break;
			}

			if (!bRet)
				break;
		}

	}
	catch (...)
	{
		AddLogEntry(L">>> Exception in IsSizeForTempAvble", 0, 0, true, SECONDLEVEL);
	}

	return bRet;
}
/***************************************************************************
Function Name  : GetAllSystemDrive
Description    : this function provides functionality to get all list of os drive in vector
Author Name    : Lalit kumawat
Date           : 7-28-2015
****************************************************************************/
void CWardWizCryptDlg::GetAllSystemDrive()
{
	TCHAR cDrive;
	UINT  uDriveType;
	TCHAR szDriveRoot[] = _T("x:\\");
	DWORD dwDrivesOnSystem = GetLogicalDrives();

	try
	{
		if (m_vcsListOfDrive.size() > 0)
		{
			m_vcsListOfDrive.clear();
		}

		for (cDrive = 'A'; cDrive <= 'Z'; cDrive++, dwDrivesOnSystem >>= 1)
		{
			if (!(dwDrivesOnSystem & 1))
				continue;

			// Get the type for the next drive, and check dwFlags to determine
			// if we should show it in the list.

			szDriveRoot[0] = cDrive;

			uDriveType = GetDriveType(szDriveRoot);

			if (uDriveType == DRIVE_FIXED)
			{
				m_vcsListOfDrive.push_back(cDrive);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L">>> Exception in GetAllSystemDrive", 0, 0, true, SECONDLEVEL);
	}

}


/***************************************************************************
Function Name  : IsPathBelongsToOSReservDirectory
Description    : Check if Path belongs to wardwiz directory
Author Name    : Ramkrushna Shelke
Date           : 9-11-2015
****************************************************************************/
bool CWardWizCryptDlg::IsPathBelongsToWardWizDir(CString csFilefolderPath)
{
	bool bReturn = false;

	try
	{
		//Check here if the path is lnk ( shortcut )
		if (csFilefolderPath.Right(4).CompareNoCase(L".lnk") == 0)
		{
			TCHAR szModulePath[MAX_PATH] = { 0 };
			if (GetModulePath(szModulePath, sizeof(szModulePath)))
			{
				CString csExpandedPath;
				csExpandedPath = ExpandShortcut(csFilefolderPath);
				if (csExpandedPath.Trim().GetLength() != 0)
				{
					int iPos = csExpandedPath.ReverseFind(L'\\');
					if (csExpandedPath.Left(iPos).CompareNoCase(szModulePath) == 0)
					{
						bReturn = true;
					}
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCryptDlg::IsPathBelongsToWardWizDir", 0, 0, true, SECONDLEVEL);;
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : ExpandShortcut
*  Description    : Uses IShellLink to expand a shortcut.
*  Return value	  : the expanded filename, or "" on error or if filename
wasn't a shortcut
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
CString CWardWizCryptDlg::ExpandShortcut(CString& csFilename)
{
	CString csExpandedFile = NULL;

	try
	{
		USES_CONVERSION;		// For T2COLE() below

		// Make sure we have a path
		if (csFilename.IsEmpty())
		{
			ASSERT(FALSE);
			return csExpandedFile;
		}

		// Get a pointer to the IShellLink interface
		HRESULT hr;
		IShellLink* pIShellLink;

		hr = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
			IID_IShellLink, (LPVOID*)&pIShellLink);

		if (SUCCEEDED(hr))
		{
			// Get a pointer to the persist file interface
			IPersistFile* pIPersistFile;
			hr = pIShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pIPersistFile);

			if (SUCCEEDED(hr))
			{
				// Load the shortcut and resolve the path
				// IPersistFile::Load() expects a UNICODE string
				// so we're using the T2COLE macro for the conversion
				// For more info, check out MFC Technical note TN059
				// (these macros are also supported in ATL and are
				// so much better than the ::MultiByteToWideChar() family)
				hr = pIPersistFile->Load(T2COLE(csFilename), STGM_READ);

				if (SUCCEEDED(hr))
				{
					WIN32_FIND_DATA wfd;
					hr = pIShellLink->GetPath(csExpandedFile.GetBuffer(MAX_PATH),
						MAX_PATH,
						&wfd,
						SLGP_UNCPRIORITY);

					csExpandedFile.ReleaseBuffer(-1);
				}
				pIPersistFile->Release();
			}
			pIShellLink->Release();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCryptDlg::ExpandShortcut()", 0, 0, true, SECONDLEVEL);;
	}
	return csExpandedFile;
}