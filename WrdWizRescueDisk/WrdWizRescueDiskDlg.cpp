
/*********************************************************************
*  Program Name		: WrdWizRescueDiskDlg.cpp
*  Description		: CWrdWizRescueDiskDlg Implementation to make 
					  bootable USB drive.
*  Author Name		: NITIN SHELAR
*  Date Of Creation	: 13 feb 2019
**********************************************************************/

#include "stdafx.h"
#include "WrdWizRescueDisk.h"
#include "WrdWizRescueDiskDlg.h"
#include "afxdialogex.h"
#include "WrdWizSystemInfo.h"
#include <dbt.h>
#include <comutil.h> 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/***************************************************************************************************
*  Function Name  : INTERFACE_MAP
*  Description    : MFC/OLE includes an implementation of "Interface Maps for creating aggregatable 
					COM objects
*  Author Name    : NITIN SHELAR
*  Date           : 25/06/2019
****************************************************************************************************/
BEGIN_INTERFACE_MAP(CWrdWizRescueDiskDlg, CCmdTarget)
	INTERFACE_PART(CWrdWizRescueDiskDlg, IID_IDispatch, FormatDataEvents)
	INTERFACE_PART(CWrdWizRescueDiskDlg, IID_DDiscFormat2DataEvents, FormatDataEvents)
END_INTERFACE_MAP()

DWORD WINAPI FormatThread(LPVOID lpParam);
DWORD WINAPI FormatCDThread(LPVOID lpParam);
#define SET_EVENT_CODE (FIRST_APPLICATION_EVENT_CODE + 9)

/***************************************************************************************************
*  Function Name  : CAboutDlg
*  Description    : C'tor
*  Author Name    : NITIN SHELAR
*  SR_NO		  : 
*  Date           : 27/02/2019
****************************************************************************************************/
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

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

/***************************************************************************
Function Name  : DoDataExchange()
Description    : Called by the framework to exchange and validate dialog data.
Author Name    : NITIN SHELAR
Date           : 11/02/2019
SR_NO		   :
****************************************************************************/
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

/***************************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : NITIN SHELAR
*  Date           : 11/02/2019
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CWrdWizRescueDiskDlg * CWrdWizRescueDiskDlg::m_pThis = NULL;

/***************************************************************************
Function Name  : CWrdWizRescueDiskDlg()
Description    : constructor
Author Name    : NITIN SHELAR
Date           : 11/02/2019
SR_NO		   :
****************************************************************************/
CWrdWizRescueDiskDlg::CWrdWizRescueDiskDlg(CWnd* pParent)
: CDialogEx(CWrdWizRescueDiskDlg::IDD, pParent), behavior_factory("WrdWizRescueDiskDlg")
{
	m_pThis = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hFormatThread = NULL;
	m_hFormatCDThread = NULL;
}

/***************************************************************************
Function Name  : ~CWrdWizRescueDiskDlg()
Description    : Dest'r
Author Name    : NITIN SHELAR
Date           : 11/02/2019
SR_NO		   :
****************************************************************************/
CWrdWizRescueDiskDlg::~CWrdWizRescueDiskDlg()
{
	if (m_hFormatThread != NULL)
	{
		SuspendThread(m_hFormatThread);
		TerminateThread(m_hFormatThread, 0);
		m_hFormatThread = NULL;
	}
	
	if (m_hFormatCDThread != NULL)
	{
		SuspendThread(m_hFormatCDThread);
		TerminateThread(m_hFormatCDThread, 0);
		m_hFormatCDThread = NULL;
	}
}

/***************************************************************************
Function Name  : DoDataExchange()
Description    : Called by the framework to exchange and validate dialog data.
Author Name    : NITIN SHELAR
Date           : 11/02/2019
SR_NO		   :
****************************************************************************/
void CWrdWizRescueDiskDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

/***************************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : NITIN SHELAR
*  Date           : 11/02/2019
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CWrdWizRescueDiskDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CWrdWizRescueDiskDlg::OnBnClickedCancel)
	ON_MESSAGE(WM_IMAPI_UPDATE, OnImapiUpdate)
END_MESSAGE_MAP()

HWINDOW   CWrdWizRescueDiskDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWrdWizRescueDiskDlg::get_resource_instance() { return theApp.m_hInstance; }
// CWrdWizRescueDiskDlg message handlers

/***************************************************************************
Function Name  : DoDataExchange()
Description    : Initializes the dialog window.
Author Name    : NITIN SHELAR
Date           : 11/02/2019
SR_NO		   :
****************************************************************************/
BOOL CWrdWizRescueDiskDlg::OnInitDialog()
{ 
	CDialogEx::OnInitDialog();
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

	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0; UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_WARDWIZ_RESCUEDISK.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_WARDWIZ_RESCUEDISK.htm");

	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;

	m_root_el = sciter::dom::element::root_element(this->get_hwnd());
	//sciter::dom::element root = sciter::dom::element::root_element(this->get_hwnd());
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);

	int cxIcon = GetSystemMetrics(SM_CXFULLSCREEN);
	int cyIcon = GetSystemMetrics(SM_CYFULLSCREEN);

	int ixRect = cxIcon - pIntMaxWidth;
	int iyRect = cyIcon - pIntHeight;

	::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);
	GetCheckZipReg();
	m_hNotifyWnd = this->get_hwnd();
	// TODO: Add extra initialization here
	this->SetWindowText(L"VibraniumRESCUDISK ");
	return TRUE;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************
Function Name  : DoDataExchange()
Description    : The framework calls this member function when the user selects a command
				 from the Control menu, or when the user selects the Maximize or the Minimize button.
Author Name    : NITIN SHELAR
Date           : 11/02/2019
SR_NO		   :
****************************************************************************/
void CWrdWizRescueDiskDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

/***************************************************************************
Function Name  : OnPaint()
Description    : The framework calls this member function when Windows or an 
				 application makes a request to repaint a portion of an application's window.
Author Name    : NITIN SHELAR
Date           : 11/02/2019
SR_NO		   :
****************************************************************************/
void CWrdWizRescueDiskDlg::OnPaint()
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
Function Name  : OnQueryDragIcon()
Description    : The system calls this function to obtain the cursor to display while the user drags
Author Name    : NITIN SHELAR
Date           : 11/02/2019
SR_NO		   :
****************************************************************************/
HCURSOR CWrdWizRescueDiskDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/***************************************************************************
Function Name  : OnBnClickedCancel()
Description    : function to close dialog box
Author Name    : NITIN SHELAR
Date           : 11/02/2019
SR_NO		   :
****************************************************************************/
void CWrdWizRescueDiskDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnCancel();
}

/***************************************************************************************************
*  Function Name  : On_GetThemeID
*  Description    : Get the Theme IDs
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 11/02/2019
****************************************************************************************************/
json::value CWrdWizRescueDiskDlg::On_GetThemeID()
{
	SCITER_VALUE svThemeID;
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		svThemeID = ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::On_GetThemeID()", 0, 0, true, ZEROLEVEL);
	}
	return svThemeID;
}

/***************************************************************************************************
*  Function Name  : onModalLoop
*  Description    : for reseting the Lightbox event msgbox
*  Author Name    : NITIN SHELAR
*  Date			  : 11/02/2019
****************************************************************************************************/
json::value CWrdWizRescueDiskDlg::onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal)
{
	try
	{
		if (svIsDialogOn.get(false)) {
			theApp.m_bRetval = svDialogBoolVal.get(false);
			theApp.m_iRetval = svDialogIntVal;
			theApp.m_objCompleteEvent.SetEvent();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::onModalLoop()", 0, 0, true, ZEROLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_Minimize
*  Description    : Minimize window while pressed on UI.
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 11/02/2019
****************************************************************************************************/
json::value CWrdWizRescueDiskDlg::On_Minimize()
{
	try
	{
		::ShowWindow(this->get_hwnd(), SW_MINIMIZE);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::On_Minimize()", 0, 0, true, ZEROLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : PreTranslateMessage
Description    : Ignore Enter/escape button click events
Author Name    : NITIN SHELAR
Date		   : 13/02/2019
/***************************************************************************************************/
BOOL CWrdWizRescueDiskDlg::PreTranslateMessage(MSG* pMsg)
{
	try
	{
		if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN || pMsg->wParam == VK_SPACE || pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT || pMsg->wParam == VK_DOWN || pMsg->wParam == VK_TAB))
		{
			return TRUE;
		}
		if (pMsg->message == WM_CHAR || pMsg->message == WM_UNICHAR)
		{
			WindowProc(pMsg->message, pMsg->wParam, pMsg->lParam);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::PreTranslateMessage()", 0, 0, true, ZEROLEVEL);
	}
	return __super::PreTranslateMessage(pMsg);
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 11/02/2019
****************************************************************************************************/
json::value CWrdWizRescueDiskDlg::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProdID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::On_GetProductID()", 0, 0, true, ZEROLEVEL);
	}

	return iProdValue;
}

/***************************************************************************************************
*  Function Name  : On_GetLanguageID
*  Description    : Get the language id
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 11/02/2019
****************************************************************************************************/
json::value CWrdWizRescueDiskDlg::On_GetLanguageID()
{
	int iLangValue = 0;
	try
	{
		iLangValue = theApp.m_objwardwizLangManager.GetSelectedLanguage();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::On_GetLanguageID()", 0, 0, true, ZEROLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
*  Function Name  : On_Close
*  Description    : Function to close dialog box
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 11/02/2019
****************************************************************************************************/
json::value CWrdWizRescueDiskDlg::On_Close()
{
	try
	{
		FunFreeReource(m_hFormatThread);
		FunFreeReource(m_hFormatCDThread);
		OnBnClickedCancel();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumMemScanDlg::On_Close()", 0, 0, true, ZEROLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name	  : OnGetAppPath
Description		  : Get the App Path and Set it in Script
Author Name		  : NITIN SHELAR
Date			  : 11/02/2019
/***************************************************************************************************/
json::value CWrdWizRescueDiskDlg::OnGetAppPath()
{
	SCITER_STRING RetAppPath;
	try
	{
		RetAppPath = (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::OnGetAppPath()", 0, 0, true, ZEROLEVEL);
	}
	return RetAppPath;
}

/***************************************************************************************************
*  Function Name  : WindowProc
*  Description    : To handle the UI related requests
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 11/02/2019
****************************************************************************************************/
LRESULT CWrdWizRescueDiskDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	BOOL    bHandled = FALSE;
	__try
	{
		lResult = SciterProcND(this->GetSafeHwnd(), message, wParam, lParam, &bHandled);

		if (bHandled)      // if it was handled by the Sciter
			return lResult; // then no further processing is required.

		switch (wParam)
		{
		case DBT_DEVICEARRIVAL:
				Sleep(200);
				GetRMDrivesList();
			break;
		case DBT_DEVICEREMOVECOMPLETE:
				GetRMDrivesList();
				Sleep(200);
				if ((m_bFlagProgress) && (!PathFileExists(m_csDrivePath)))
					FunUIMessages(5);
			break;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::WindowProc()", 0, 0, true, ZEROLEVEL);
	}
	return __super::WindowProc(message, wParam, lParam);
}

/**********************************************************************************************************
*  Function Name  :	GetRMDrivesList
*  Description    :	Makes list of removable drives present on a system.
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 11/02/2019
**********************************************************************************************************/
BOOL CWrdWizRescueDiskDlg::GetRMDrivesList()
{
	BOOL bReturn = FALSE;
	CString csDrive;
	int iFlag = 0;
	try
	{
		for (char chDrive = 'A'; chDrive <= 'Z'; chDrive++)
		{
			csDrive.Format(L"%c:", chDrive);
			if (m_svDiskType == DRIVE_REMOVABLE)
			{
				if (GetDriveType(csDrive) == DRIVE_REMOVABLE)
				{
					m_svAddDriveEntry.call(SCITER_STRING(csDrive));
					iFlag = 1;
					bReturn = TRUE;
				}
			}
			if (m_svDiskType == DRIVE_CDROM)
			{
				if ((GetDriveType(csDrive) == DRIVE_CDROM) && GetDrivePathCheck(csDrive))
				{
					m_svAddDriveEntry.call(SCITER_STRING(csDrive));
					iFlag = 1;
					bReturn = TRUE;
				}
			}
		}
		if (iFlag == 0)
			m_svAddDriveEntry.call(SCITER_STRING(L""));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::GetRMDrivesList()", 0, 0, true, ZEROLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	On_GetDriveList
*  Description    :	Get drive list and pass to the UI
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 11/02/2019
**********************************************************************************************************/
json::value CWrdWizRescueDiskDlg::On_GetDriveList(SCITER_VALUE svDriveList, SCITER_VALUE svDriveType)
{
	try
	{
		m_svDiskType = svDriveType;
		m_svAddDriveEntry = svDriveList;
		GetRMDrivesList();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::On_GetDriveList()", 0, 0, true, ZEROLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	On_GetIsoFile
*  Description    :	Function for open file dialog box to select ISO file.
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 11/02/2019
**********************************************************************************************************/
json::value CWrdWizRescueDiskDlg::On_GetIsoFile()
{
	TCHAR szFile[MAX_PATH];
	OPENFILENAME ofn;
	try
	{
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof (ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = L"ISO\0*.iso\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		GetOpenFileName(&ofn);
		
		m_csIsoFilePath = ofn.lpstrFile;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::On_GetIsoFile()", 0, 0, true, ZEROLEVEL);
	}
	return ofn.lpstrFile;
}

/**********************************************************************************************************
*  Function Name  :	On_GetResourceData
*  Description    :	Function to get iso file path and drive path which is selected in UI.
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 13/02/2019
**********************************************************************************************************/
json::value CWrdWizRescueDiskDlg::On_GetResourceData(SCITER_VALUE svIsoFilePath, SCITER_VALUE svDrivePath, SCITER_VALUE svStatus)
{
	try
	{
		m_status = svStatus;
		std::wstring wsIsoPath = svIsoFilePath.get(L"");
		std::wstring wsDrivePath = svDrivePath.get(L"");
		m_csIsoFilePath = wsIsoPath.c_str();
		m_csDrivePath = wsDrivePath.c_str();
		
		if (GetDriveType(wsDrivePath.c_str()) == DRIVE_REMOVABLE)
			m_hFormatThread = ::CreateThread(NULL, 0, FormatThread, (LPVOID) this, 0, NULL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::On_GetResourceData()", 0, 0, true, ZEROLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	On_GetResourceData
*  Description    :	Function to get iso file path and drive path which is selected in UI.
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 13/02/2019
**********************************************************************************************************/
json::value CWrdWizRescueDiskDlg::On_StartCDThread(SCITER_VALUE svIsoFilePath, SCITER_VALUE svDrivePath, SCITER_VALUE svStatus)
{
	try
	{
		m_status = svStatus;
		std::wstring wsIsoPath = svIsoFilePath.get(L"");
		std::wstring wsDrivePath = svDrivePath.get(L"");
		m_csIsoFilePath = wsIsoPath.c_str();
		m_csDrivePath = wsDrivePath.c_str();
		m_hFormatCDThread = ::CreateThread(NULL, 0, FormatCDThread, (LPVOID) this, 0, NULL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::On_GetResourceData()", 0, 0, true, ZEROLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	FunFreeReource
*  Description    :	Function to release allocated resources.
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 13/02/2019
**********************************************************************************************************/
void CWrdWizRescueDiskDlg::FunFreeReource(HANDLE hThread)
{
	try
	{
		if (hThread != NULL && hThread != INVALID_HANDLE_VALUE)
		{
			SuspendThread(hThread);
			TerminateThread(hThread, 0);
			safe_closehandle(hThread);
			hThread = NULL;
		}
		m_csDrivePath = L"";
		m_csIsoFilePath = L"";
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::FunFreeReource()", 0, 0, true, ZEROLEVEL);
	}
}

/***************************************************************************
Function Name  : FormatThread
Description    : Thread to format USB drive with MBR and extract iso file into drive.
Author Name    : NITIN SHELAR
Date           : 13/02/2019
****************************************************************************/
DWORD WINAPI FormatThread(LPVOID lpParam)
{

	if (lpParam == NULL)
		return 0;
	
	HANDLE hFile = NULL;
	BOOL bRet = FALSE;
	char FileSystemName[32];
	CString szDrivePath;
	char *guid_volume = NULL;

	HANDLE hPhysical = INVALID_HANDLE_VALUE, hLogicalVolume = INVALID_HANDLE_VALUE;
	CWrdWizRescueDiskDlg *pThis = (CWrdWizRescueDiskDlg*)lpParam;
	pThis->m_bFlagProgress = TRUE;
	pThis->m_dwPercentVal = 0;

	try
	{
		int iDriveIndex = 0;
		
		if (!pThis->GetDriveIndex(&iDriveIndex))
			goto out;

		pThis->m_status.call(1);
		pThis->UpdateProgressEvent();
		memset(&pThis->SelectedDrive, 0, sizeof(pThis->SelectedDrive));
		pThis->SelectedDrive.nPartitions = pThis->GetDrivePartitionData(iDriveIndex, FileSystemName, sizeof(FileSystemName));
		pThis->UpdateProgressEvent();
		
		hFile = CreateFile(pThis->m_csDrivePath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);                  

		if (hFile == INVALID_HANDLE_VALUE)
		{
			if (GetLastError() == 19)
			{
				pThis->FunUIMessages(6);
				goto out;
			}
		}
		CloseHandle(hFile);

		bRet = pThis->FormatDrive(iDriveIndex);
		if (!bRet)
		{
			pThis->FunUIMessages(5);
			goto out;
		}

		pThis->WaitForLogical(iDriveIndex);
		hPhysical = pThis->GetPhysicalDriveHandle(iDriveIndex, TRUE, TRUE);
		if (hPhysical == NULL)
			goto out;

		pThis->UpdateProgressEvent();
		if (!pThis->UnmountVolume(hPhysical)) // dismount volume using physical handle
			goto out;

		pThis->UpdateProgressEvent();
		if (!pThis->WriteMBR(hPhysical))
			goto out;
		
	    
		pThis->WaitForLogical(iDriveIndex);
		guid_volume = pThis->GetLogicalName(iDriveIndex, TRUE, TRUE);

		pThis->UpdateProgressEvent();
		szDrivePath.Format(L"%s\\",pThis->m_csDrivePath);
		if (!pThis->MountVolume(CT2A(szDrivePath, CP_UTF8), guid_volume))
			goto out;

		pThis->UpdateProgressEvent();
		hLogicalVolume = pThis->GetLogicalDriveHandle(iDriveIndex, TRUE, FALSE);

		if (hLogicalVolume == INVALID_HANDLE_VALUE)
			goto out;

		pThis->UpdateProgressEvent();
		if (!pThis->WriteNTFSBootR(hLogicalVolume)) {
			goto out;
		}

		pThis->UpdateProgressEvent();
		safe_unlockclose(hLogicalVolume);

		pThis->UpdateProgressEvent();
		if (!pThis->RemountVolume(CT2A(szDrivePath, CP_UTF8)))
			goto out;

		if (!pThis->GetExtractIso())
			goto out;

		pThis->m_bFlagProgress = FALSE;
		pThis->UpdateProgressEvent();
		pThis->m_status.call(4);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in FormatThread()", 0, 0, true, ZEROLEVEL);
	}

out:
	if (hPhysical != INVALID_HANDLE_VALUE && hPhysical != NULL)
		safe_closehandle(hPhysical);
	if (hFile != NULL)
		CloseHandle(hFile);
	pThis->FunFreeReource(pThis->m_hFormatThread);

	return 0;
}

/***************************************************************************
Function Name  : RemountVolume
Description    : Function to Remount volume after format.
Author Name    : NITIN SHELAR
Date           : 13/02/2019
****************************************************************************/
BOOL CWrdWizRescueDiskDlg::RemountVolume(char* drive_name)
{
	char drive_guid[51];

	try
	{
		FlushDrive(drive_name[0]);
		if (GetVolumeNameForVolumeMountPointA(drive_name, drive_guid, sizeof(drive_guid))) {
			if (DeleteVolumeMountPointA(drive_name)) {
				Sleep(200);
				if (MountVolume(drive_name, drive_guid)) {
					AddLogEntry(L"### %s drive successfully remount", m_csDrivePath, 0, true, ZEROLEVEL);
				}
				else {
					AddLogEntry(L"### %s drive failed to remount", m_csDrivePath, 0, true, ZEROLEVEL);
					return FALSE;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception CVibraniumRescueDiskDlg::RemountVolume()", 0, 0, true, ZEROLEVEL);
	}
	return TRUE;
}

/***************************************************************************
Function Name  : FlushDrive
Description    : Function to Flushes the buffers of a specified file and 
				 causes all buffered data to be written to a file.
Author Name    : NITIN SHELAR
Date           : 13/02/2019
****************************************************************************/
BOOL CWrdWizRescueDiskDlg::FlushDrive(char drive_letter)
{
	HANDLE hDrive = INVALID_HANDLE_VALUE;
	BOOL bRet = FALSE;
	char logical_drive[] = "\\\\.\\#:";

	try
	{
		logical_drive[4] = drive_letter;
		hDrive = CreateFileA(logical_drive, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hDrive == INVALID_HANDLE_VALUE)
			goto out;

		bRet = FlushFileBuffers(hDrive);
		if (bRet == FALSE)
			goto out;
		
	}
	catch (...)
	{
		AddLogEntry(L"### Exception CVibraniumRescueDiskDlg::FlushDrive()", 0, 0, true, ZEROLEVEL);
	}

out:
	safe_closehandle(hDrive);
	return bRet;
}

/***************************************************************************
Function Name  : MountVolume
Description    : Function to Mount volume if it is unmounted.
Author Name    : NITIN SHELAR
Date           : 13/02/2019
****************************************************************************/
BOOL CWrdWizRescueDiskDlg::MountVolume(char* drive_name, char *drive_guid)
{
	char mounted_guid[52];
	char mounted_letter[16] = { 0 };
	DWORD size;

	try
	{
		// For fixed disks, Windows may already have remounted the volume, but with a different letter
		// than the one we want. If that's the case, we need to unmount first.
		if ((GetVolumePathNamesForVolumeNameA(drive_guid, mounted_letter, sizeof(mounted_letter), &size)) && (size > 1) && (mounted_letter[0] != drive_name[0]))
		{
			if (!DeleteVolumeMountPointA(mounted_letter))
				AddLogEntry(L"%s drive failed to unmount", m_csDrivePath, 0, true, ZEROLEVEL);
			Sleep(200);
		}

		if (!SetVolumeMountPointA(drive_name, drive_guid))
		{
			// If the OS was faster than us at remounting the drive, this operation can fail
			// with ERROR_DIR_NOT_EMPTY. If that's the case, just check that mountpoints match
			if (GetLastError() == ERROR_DIR_NOT_EMPTY)
			{
				if (!GetVolumeNameForVolumeMountPointA(drive_name, mounted_guid, sizeof(mounted_guid)))
					return FALSE;
			}
			else {
				return FALSE;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception CVibraniumRescueDiskDlg::MountVolume()", 0, 0, true, ZEROLEVEL);
	}
	return TRUE;
}

/***************************************************************************
Function Name  : GetDriveIndex
Description    : Function to get specified drive index.
Author Name    : NITIN SHELAR
Date           : 13/02/2019
****************************************************************************/
BOOL CWrdWizRescueDiskDlg::GetDriveIndex(int *iDriveIndex)
{
	int drive_number = 0;
	CString csDrivePath;
	STORAGE_DEVICE_NUMBER diskNumber;
	DWORD bytesReturned;
	try
	{
		HANDLE hDrive = INVALID_HANDLE_VALUE;
		csDrivePath.Format(L"\\\\.\\%s", m_csDrivePath);
		hDrive = CreateFile(csDrivePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE , NULL, OPEN_EXISTING, 0, NULL);
		if (hDrive == INVALID_HANDLE_VALUE)
			return FALSE;

		ZeroMemory(&diskNumber, sizeof(STORAGE_DEVICE_NUMBER));
		DeviceIoControl(hDrive, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0, &diskNumber, sizeof(STORAGE_DEVICE_NUMBER), &bytesReturned, NULL);

		UpdateProgressEvent();
		safe_closehandle(hDrive);
		drive_number = (DWORD)diskNumber.DeviceNumber;
		*iDriveIndex = drive_number + DRIVE_INDEX_MIN;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::GetDriveIndex()", 0, 0, true, ZEROLEVEL);
	}
	return TRUE;
}

/***************************************************************************
Function Name  : GetLogicalDriveHandle
Description    : Function to get handle of logical disk drive.
Author Name    : NITIN SHELAR
Date           : 13/02/2019
****************************************************************************/
HANDLE CWrdWizRescueDiskDlg::GetLogicalDriveHandle(DWORD DriveIndex, BOOL bWriteAccess, BOOL bLockDrive)
{
	HANDLE hLogical = INVALID_HANDLE_VALUE;
	char* LogicalPath = NULL;
	try
	{
		LogicalPath = GetLogicalName(DriveIndex, FALSE, FALSE);
		if (LogicalPath == NULL)
			goto out;

		hLogical = GetGenericHandle(LogicalPath, bWriteAccess, bLockDrive);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::GetLogicalDriveHandle()", 0, 0, true, ZEROLEVEL);
	}

out:
	safe_free(LogicalPath);
	return hLogical;
}

/***************************************************************************
Function Name  : GetDrivePartitionData
Description    : Function to get drive information.
Author Name    : NITIN SHELAR
Date           : 13/02/2019
****************************************************************************/
int CWrdWizRescueDiskDlg::GetDrivePartitionData(DWORD DriveIndex, char* FileSystemName, DWORD FileSystemNameSize)
{
	BOOL bRet;
	HANDLE hPhysical;
	DWORD size;
	BYTE geometry[256], layout[4096], part_type;
	PDISK_GEOMETRY_EX DiskGeometry = (PDISK_GEOMETRY_EX)(void*)geometry;
	PDRIVE_LAYOUT_INFORMATION_EX DriveLayout = (PDRIVE_LAYOUT_INFORMATION_EX)(void*)layout;

	char* volume_name;
	DWORD i, nb_partitions = 0;
	try
	{
		volume_name = GetLogicalName(DriveIndex, TRUE, FALSE);

		if ((volume_name == NULL) || (!GetVolumeInformationA(volume_name, NULL, 0, NULL, NULL, NULL, FileSystemName, FileSystemNameSize))) {
			AddLogEntry(L"### No volume information of disk %s", m_csDrivePath, 0, true, ZEROLEVEL);
		}
		safe_free(volume_name);
		hPhysical = GetPhysicalDriveHandle(DriveIndex, FALSE, FALSE);
		if (hPhysical == INVALID_HANDLE_VALUE)
			return 0;
		
			bRet = DeviceIoControl(hPhysical, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, geometry, sizeof(geometry), &size, NULL);
			if (!bRet || size <= 0) {
				AddLogEntry(L"### Could not get geometry of drive %s", m_csDrivePath, 0, true, ZEROLEVEL);
				safe_closehandle(hPhysical);
				return 0;
			}
		SelectedDrive.DiskSize = DiskGeometry->DiskSize.QuadPart;
		memcpy(&SelectedDrive.Geometry, &DiskGeometry->Geometry, sizeof(DISK_GEOMETRY));

		bRet = DeviceIoControl(hPhysical, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, layout, sizeof(layout), &size, NULL);
		if (!bRet || size <= 0)
		{
			AddLogEntry(L"### Could not get layout of drive %s", m_csDrivePath, 0, true, ZEROLEVEL);
			safe_closehandle(hPhysical);
			return 0;
		}

		switch (DriveLayout->PartitionStyle) {
		case PARTITION_STYLE_MBR:
			SelectedDrive.PartitionType = PARTITION_STYLE_MBR;
			for (i = 0; i<DriveLayout->PartitionCount; i++) 
			{
				if (DriveLayout->PartitionEntry[i].Mbr.PartitionType != PARTITION_ENTRY_UNUSED) 
				{
					nb_partitions++;
				}
			}

			SelectedDrive.has_mbr_uefi_marker = (DriveLayout->Mbr.Signature == MBR_UEFI_MARKER);
			
			for (i = 0; i<DriveLayout->PartitionCount; i++) 
			{
				if (DriveLayout->PartitionEntry[i].Mbr.PartitionType != PARTITION_ENTRY_UNUSED) 
				{
					part_type = DriveLayout->PartitionEntry[i].Mbr.PartitionType;
					if (part_type == 0xee)
						SelectedDrive.has_protective_mbr = TRUE;
				}
			}
			break;
		default:
			SelectedDrive.PartitionType = PARTITION_STYLE_MBR;
			break;
		}
		safe_closehandle(hPhysical);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::GetDrivePartitionData()", 0, 0, true, ZEROLEVEL);
	}
	return(int)nb_partitions;
}

/***************************************************************************
Function Name  : GetLogicalName
Description    : Function to get logical drive name.
Author Name    : NITIN SHELAR
Date           : 13/02/2019
****************************************************************************/
char* CWrdWizRescueDiskDlg::GetLogicalName(DWORD DriveIndex, BOOL bKeepTrailingBackslash, BOOL bSilent)
{
	BOOL success = FALSE;
	char volume_name[MAX_PATH];
	HANDLE hDrive = INVALID_HANDLE_VALUE, hVolume = INVALID_HANDLE_VALUE;
	size_t len;
	char path[MAX_PATH];
	VOLUME_DISK_EXTENTS DiskExtents;
	DWORD size;
	UINT drive_type;
	int i, j;
	static const char* ignore_device[] = { "\\Device\\CdRom", "\\Device\\Floppy" };
	static const char* volume_start = "\\\\?\\";
	CheckDriveIndex(DriveIndex);

	try
	{
		for (i = 0; hDrive == INVALID_HANDLE_VALUE; i++)
		{
			if (i == 0)
			{
				hVolume = FindFirstVolumeA(volume_name, sizeof(volume_name));
				if (hVolume == INVALID_HANDLE_VALUE)
				{
					goto out;
				}
			}
			else
			{
				if (!FindNextVolumeA(hVolume, volume_name, sizeof(volume_name)))
				{
					if (GetLastError() != ERROR_NO_MORE_FILES) {
					}
					goto out;
				}
			}

			len = safe_strlen(volume_name);
			if ((len <= 1) || (safe_strnicmp(volume_name, volume_start, 4) != 0) || (volume_name[len - 1] != '\\'))
			{
				continue;
			}

			drive_type = GetDriveTypeA(volume_name);
			if ((drive_type != DRIVE_REMOVABLE) && (drive_type != DRIVE_FIXED))
				continue;

			volume_name[len - 1] = 0;
			if (QueryDosDeviceA(&volume_name[4], path, sizeof(path)) == 0)
				continue;

			for (j = 0; (j < ARRAYSIZE(ignore_device)) && (_strnicmp(path, ignore_device[j], safe_strlen(ignore_device[j])) != 0); j++);
			if (j < ARRAYSIZE(ignore_device)) { continue; }

			hDrive = CreateFileA(volume_name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
			if (hDrive == INVALID_HANDLE_VALUE)
				continue;

			if ((!DeviceIoControl(hDrive, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, &DiskExtents, sizeof(DiskExtents), &size, NULL)) || (size <= 0))
			{
				safe_closehandle(hDrive);
				continue;
			}
			safe_closehandle(hDrive);
			if ((DiskExtents.NumberOfDiskExtents >= 1) && (DiskExtents.Extents[0].DiskNumber == DriveIndex))
			{
				if (bKeepTrailingBackslash)
					volume_name[len - 1] = '\\';
				success = TRUE;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::GetLogicalName()", 0, 0, true, ZEROLEVEL);
	}

out:
	if (hVolume != INVALID_HANDLE_VALUE)
		FindVolumeClose(hVolume);

	if (hDrive != INVALID_HANDLE_VALUE && hDrive != NULL)
		safe_closehandle(hDrive);

	return (success) ? safe_strdup(volume_name) : NULL;
}

/***************************************************************************
Function Name  : GetPhysicalDriveHandle
Description    : Function to get physical handle of drive.
Author Name    : NITIN SHELAR
Date           : 13/02/2019
****************************************************************************/
HANDLE CWrdWizRescueDiskDlg::GetPhysicalDriveHandle(DWORD DriveIndex, BOOL bWriteAccess, BOOL bLockDrive)
{
	HANDLE hPhysical = INVALID_HANDLE_VALUE;
	try
	{
		char* PhysicalPath = GetPhysicalName(DriveIndex);
		hPhysical = GetGenericHandle(PhysicalPath, bWriteAccess, bLockDrive);
		safe_free(PhysicalPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::GetPhysicalDriveHandle()", 0, 0, true, ZEROLEVEL);
	}
	return hPhysical;
}

/***************************************************************************
Function Name  : GetPhysicalName
Description    : Function to get physical name of drive.
Author Name    : NITIN SHELAR
Date           : 13/02/2019
****************************************************************************/
char* CWrdWizRescueDiskDlg::GetPhysicalName(DWORD DriveIndex)
{
	BOOL success = FALSE;
	char physical_name[24];
	try
	{
		CheckDriveIndex(DriveIndex);
		safe_sprintf(physical_name, sizeof(physical_name), "\\\\.\\PHYSICALDRIVE%d", DriveIndex);
		success = TRUE;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::GetPhysicalName()", 0, 0, true, ZEROLEVEL);
	}

	return (success) ? safe_strdup(physical_name) : NULL;
}

/***************************************************************************
Function Name  : GetGenericHandle
Description    : Function to get generic handle of drive.
Author Name    : NITIN SHELAR
Date           : 13/02/2019
****************************************************************************/
HANDLE CWrdWizRescueDiskDlg::GetGenericHandle(char* szPath, BOOL bWriteAccess, BOOL bLockDrive)
{
	int i;
	DWORD size;
	HANDLE hDrive = INVALID_HANDLE_VALUE;
	if (szPath == NULL)
		return 0;
	try
	{
		hDrive = CreateFileA(szPath, GENERIC_READ | (bWriteAccess ? GENERIC_WRITE : 0), FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hDrive == INVALID_HANDLE_VALUE)
		{
			AddLogEntry(L"### Could not get exclusive access to device %s", m_csDrivePath, 0, true, ZEROLEVEL);
		}
		
		if (bLockDrive)
		{
			if (DeviceIoControl(hDrive, FSCTL_ALLOW_EXTENDED_DASD_IO, NULL, 0, NULL, 0, &size, NULL))
				AddLogEntry(L"### I/O boundary checks disabled for drive %s", m_csDrivePath, 0, true, ZEROLEVEL);

			for (i = 0; i < DRIVE_ACCESS_RETRIES; i++)
			{
				if (DeviceIoControl(hDrive, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &size, NULL))
					goto out;
				Sleep(DRIVE_ACCESS_TIMEOUT / DRIVE_ACCESS_RETRIES);
			}
			safe_closehandle(hDrive);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::GetGenericHandle()", 0, 0, true, ZEROLEVEL);
	}

out:
	return hDrive;
}

/***************************************************************************
Function Name  : WriteBootSectors
Description    : Function to write drive boot sector.
Author Name    : NITIN SHELAR
Date           : 14/02/2019
****************************************************************************/
int64_t CWrdWizRescueDiskDlg::WriteBootSectors(HANDLE hDrive, uint64_t SectorSize, uint64_t StartSector, uint64_t nSectors, const void *pBuf)
{
	LARGE_INTEGER ptr;
	DWORD Size = 0x00;
	try
	{
		if ((nSectors*SectorSize) > 0xFFFFFFFFUL)
			return -1;

		Size = (DWORD)(nSectors*SectorSize);
		ptr.QuadPart = StartSector*SectorSize;

		if (!SetFilePointerEx(hDrive, ptr, NULL, FILE_BEGIN))
			return -1;

		if ((!WriteFile(hDrive, pBuf, Size, &Size, NULL)) || (Size != nSectors*SectorSize))
			return Size;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::WriteBootSectors()..", 0, 0, true, ZEROLEVEL);
	}
	return (int64_t)Size;
}

/***************************************************************************
Function Name  : WaitForLogical
Description    : Function for wait after created partition of drive
Author Name    : NITIN SHELAR
Date           : 14/02/2019
****************************************************************************/
BOOL CWrdWizRescueDiskDlg::WaitForLogical(DWORD DriveIndex)
{
	DWORD i;
	char* LogicalPath = NULL;

	try
	{
		for (i = 0; i < DRIVE_ACCESS_RETRIES; i++) {
			LogicalPath = GetLogicalName(DriveIndex, FALSE, TRUE);
			if (LogicalPath != NULL) {
				safe_free(LogicalPath);
				return TRUE;
			}
			if (IS_ERROR(FormatStatus))
				return FALSE;
			Sleep(DRIVE_ACCESS_TIMEOUT / DRIVE_ACCESS_RETRIES);
		}
		UpdateProgressEvent();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::WaitForLogical()", 0, 0, true, ZEROLEVEL);
	}
	return FALSE;
}

/***************************************************************************
Function Name  : UnmountVolume
Description    : Function to unmount volume 
Author Name    : NITIN SHELAR
Date           : 14/02/2019
****************************************************************************/
BOOL CWrdWizRescueDiskDlg::UnmountVolume(HANDLE hDrive)
{
	try
	{
		DWORD size;
		if (!DeviceIoControl(hDrive, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &size, NULL))
		{
			return FALSE;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::UnmountVolume()", 0, 0, true, ZEROLEVEL);
	}
	return TRUE;
}

/***************************************************************************
Function Name  : WriteNTFSBootR
Description    : Function to write partition boot record for NTFS file system.
Author Name    : NITIN SHELAR
Date           : 14/02/2019
****************************************************************************/
BOOL CWrdWizRescueDiskDlg::WriteNTFSBootR(HANDLE hLogicalVolume)
{
	FILE fake_fd = { 0 };
	try
	{
		fake_fd._ptr = (char*)hLogicalVolume;
		fake_fd._bufsiz = SelectedDrive.Geometry.BytesPerSector;

		if (!(WriteData(&fake_fd, 0x0, br_ntfs_0x0, sizeof(br_ntfs_0x0)) && WriteData(&fake_fd, 0x54, br_ntfs_0x54, sizeof(br_ntfs_0x54))))
		{
			return FALSE;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::WriteNTFSBootR()", 0, 0, true, ZEROLEVEL);
	}
	return TRUE;
}

/***************************************************************************
Function Name  : WriteData
Description    : Function to write partition boot record
Author Name    : NITIN SHELAR
Date           : 14/02/2019
****************************************************************************/
int CWrdWizRescueDiskDlg::WriteData(FILE *fp, uint64_t Position, const void *pData, uint64_t Len)
{
	unsigned char aucBuf[MAX_DATA_LEN];
	HANDLE hDrive = (HANDLE)fp->_ptr;
	uint64_t SectorSize = (uint64_t)fp->_bufsiz;
	uint64_t StartSector, EndSector, NumSectors;
	Position += (uint64_t)fp->_cnt;

	try
	{
		StartSector = Position / SectorSize;
		EndSector = (Position + Len + SectorSize - 1) / SectorSize;
		NumSectors = EndSector - StartSector;

		if ((NumSectors*SectorSize) > MAX_DATA_LEN)
		{
			return 0;
		}

		if (Len > 0xFFFFFFFFUL)
		{
			return 0;
		}

		if (ReadSectors(hDrive, SectorSize, StartSector, NumSectors, aucBuf) <= 0)
			return 0;

		if (!memcpy(&aucBuf[Position - StartSector*SectorSize], pData, (size_t)Len))
			return 0;

		if (WriteBootSectors(hDrive, SectorSize, StartSector, NumSectors, aucBuf) <= 0)
			return 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::WriteData()", 0, 0, true, ZEROLEVEL);
	}
	return 1;
}

/***************************************************************************************************
*  Function Name   : ReadSectors
*  Description     : function to Read sector of USB Drive.
*  Author Name	   : NITIN SHELAR
*  Date Of Creation: 21/02/2019
****************************************************************************************************/
int64_t CWrdWizRescueDiskDlg::ReadSectors(HANDLE hDrive, uint64_t SectorSize, uint64_t StartSector, uint64_t nSectors, void *pBuf)
{
	LARGE_INTEGER ptr;
	DWORD Size = 0x00;

	try
	{
		if ((nSectors*SectorSize) > 0xFFFFFFFFUL)
		{
			return -1;
		}

		Size = (DWORD)(nSectors*SectorSize);
		ptr.QuadPart = StartSector*SectorSize;

		if (!SetFilePointerEx(hDrive, ptr, NULL, FILE_BEGIN))
		{
			return -1;
		}

		if ((!ReadFile(hDrive, pBuf, Size, &Size, NULL)) || (Size != nSectors*SectorSize))
		{
			AddLogEntry(L"### Failed ReadFile() for drive %s", m_csDrivePath, 0, true, ZEROLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::ReadSectors()", 0, 0, true, ZEROLEVEL);
	}
	return (int64_t)Size;
}

/***************************************************************************************************
*  Function Name   : GetCheckZipReg
*  Description     : Function to get check zip dll file register or not.
*  Author Name	   : NITIN SHELAR
*  Date Of Creation: 21/02/2019
****************************************************************************************************/
void CWrdWizRescueDiskDlg::GetCheckZipReg()
{
	CString csFilePath, csReg;
	WardWizSystemInfo	objSysInfo;
	try
	{
		SHELLEXECUTEINFO sei = { sizeof(sei) };
		ZeroMemory(&sei, sizeof(sei));

		csFilePath = theApp.m_objwardwizLangManager.GetWardWizPathFromRegistry();
		if (objSysInfo.GetOSType())
		{
			csReg.Format(L"/s \"%s7-ZIP32.DLL\"", csFilePath);
		}
		else
		{
			csReg.Format(L"/s \"%s7-ZIP.DLL\"", csFilePath);
		}

		HKEY hKey;
		LONG lRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node\\Classes\\CLSID\\{23170F69-40C1-278A-1000-000100020000}", 0, KEY_READ, &hKey);

		if (lRes == ERROR_FILE_NOT_FOUND)
		{
			sei.lpFile = L"regsvr32.exe";
			sei.lpParameters = csReg;
			sei.nShow = SW_HIDE;
			sei.cbSize = sizeof(SHELLEXECUTEINFO);
			sei.fMask = SEE_MASK_NOCLOSEPROCESS;
			ShellExecuteEx(&sei);
			WaitForSingleObject(sei.hProcess, INFINITE);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::GetCheckZipReg()", 0, 0, true, ZEROLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : UpdateProgressEvent
*  Description    : Function to update progress.
*  Author Name    : NITIN SHELAR
*  Date			  : 21/02/2019
****************************************************************************************************/
void CWrdWizRescueDiskDlg::UpdateProgressEvent()
{
	try
	{
		m_dwPercentVal++;
		sciter::value map;
		CString csPercentCount;
		csPercentCount.Format(L"%d", m_dwPercentVal);
		map.set_item("one", sciter::string(csPercentCount));

		//Send here event
		sciter::dom::element ela = self;
		BEHAVIOR_EVENT_PARAMS params;
		params.cmd = SET_EVENT_CODE;
		params.he = params.heTarget = ela;
		params.data = map;
		ela.fire_event(params, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::UpdateProgressEvent()", 0, 0, true, ZEROLEVEL);
	}
}

/***************************************************************************************************
*  Function Name   : GetExtractIso
*  Description     : Function to get extract iso file into specified path.
*  Author Name	   : NITIN SHELAR
*  Date Of Creation: 21/02/2019
****************************************************************************************************/
BOOL CWrdWizRescueDiskDlg::GetExtractIso()
{
	BOOL bRet = FALSE;
	CString csDrivePath;
	CString csFilePath;
	DWORD dwExitCode = 0x00;

	try
	{
		SHELLEXECUTEINFO sei = { sizeof(sei) };
		ZeroMemory(&sei, sizeof(sei));
		m_status.call(2);
		csFilePath = theApp.m_objwardwizLangManager.GetWardWizPathFromRegistry();
		csFilePath.Append(L"7z.exe");
		csDrivePath.Format(L"x \"%s\" -o\"%s\"", m_csIsoFilePath, m_csDrivePath);
		sei.lpFile = csFilePath;
		sei.lpParameters = csDrivePath;
		sei.nShow = SW_HIDE;
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;

		if (!ShellExecuteEx(&sei))
		{
			bRet = FALSE;
		}
		else
		{
			bRet = TRUE;
			for (;;)
			{
				if (GetExitCodeProcess(sei.hProcess, &dwExitCode) == FALSE)
					break;

				if (dwExitCode != STILL_ACTIVE)
				{
					UpdateProgressEvent();

					if (!PathFileExists(m_csDrivePath))
						bRet = FALSE;
					else
						bRet = TRUE;
					safe_closehandle(sei.hProcess);
					break;
				}
				else
				{
					Sleep(1000); 
					if (m_dwPercentVal > 95)
					{
						m_dwPercentVal = 96;
						m_status.call(3);
					}
					UpdateProgressEvent();
				}
			}
			AddLogEntry(L"### %s drive sucessfully mounted, ready to use as bootable USB flash drive", m_csDrivePath, 0, true, ZEROLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::GetExtractIso()", 0, 0, true, ZEROLEVEL);
	}
	return bRet;
}

/***************************************************************************************************
*  Function Name   : FormatDrive
*  Description     : Function to Format drive of specified path
*  Author Name	   : NITIN SHELAR
*  Date Of Creation: 21/02/2019
****************************************************************************************************/
BOOL CWrdWizRescueDiskDlg::FormatDrive(DWORD DriveIndex)
{
	BOOL bRet = FALSE;
	CString csDrivePath;
	DWORD dwExitCode = 0;
	try
	{
		csDrivePath.Format(L"%s /V:WWRescueDisk /FS:NTFS /Q /Y",m_csDrivePath);
		SHELLEXECUTEINFO sei = { sizeof(sei) };
		
		sei.lpFile = L"FORMAT";
		sei.lpParameters = csDrivePath;
		sei.nShow = SW_HIDE;
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;

		if (!ShellExecuteEx(&sei))
			bRet = FALSE;
		else
		{
			for (;;)
			{
				if (GetExitCodeProcess(sei.hProcess, &dwExitCode) == FALSE)
					break;
				
				if (dwExitCode != STILL_ACTIVE)
				{
					if (!PathFileExists(m_csDrivePath))
						bRet = FALSE;
					else
						bRet = TRUE;
					
					safe_closehandle(sei.hProcess);
					break;
				}
				else
				{
					Sleep(1000);
					if (m_dwPercentVal > 44)
					{
						m_dwPercentVal = 45;
					}
					UpdateProgressEvent();
				}
			}
		}
		AddLogEntry(L"### %s drive format completed", m_csDrivePath, 0, true, ZEROLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::FormatDrive()", 0, 0, true, ZEROLEVEL);
	}
	return bRet;
}

/***************************************************************************************************
*  Function Name   : WriteMBR
*  Description     : function to write MBR into drive by using handle.
*  Author Name	   : NITIN SHELAR
*  Date Of Creation: 21/02/2019
****************************************************************************************************/
BOOL CWrdWizRescueDiskDlg::WriteMBR(HANDLE hPhysicalDrive)
{
	BOOL bRet = FALSE;
	DWORD size;
	unsigned char* buf = NULL;
	size_t SecSize = SelectedDrive.Geometry.BytesPerSector;
	size_t nSecs = (0x200 + SecSize - 1) / SecSize;
	FILE fake_fd = { 0 };
	const char* using_msg = "Using %s MBR\n";
	
	try
	{
		// Format rewrites the MBR and removes the LBA attribute of FAT16
		// and FAT32 partitions - we need to correct this in the MBR
		buf = (unsigned char*)malloc(SecSize * nSecs);
		if (buf == NULL) {
			goto out;
		}

		if (!ReadSectors(hPhysicalDrive, SelectedDrive.Geometry.BytesPerSector, 0, nSecs, buf))
		{
			goto out;
		}
		buf[0x1be] = 0x80;

		if (!WriteBootSectors(hPhysicalDrive, SecSize, 0, nSecs, buf)) {
			goto out;
		}

		fake_fd._ptr = (char*)hPhysicalDrive;
		fake_fd._bufsiz = SelectedDrive.Geometry.BytesPerSector;
		bRet = WriteZeroMBR(&fake_fd);
		bRet = WriteWinMBR(&fake_fd);

		if (!DeviceIoControl(hPhysicalDrive, IOCTL_DISK_UPDATE_PROPERTIES, NULL, 0, NULL, 0, &size, NULL))
		{
			AddLogEntry(L"### Failed to notify system about %s disk properties update", m_csDrivePath, 0, true, ZEROLEVEL);
		}
		bRet = TRUE;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception CVibraniumRescueDiskDlg::WriteMBR()", 0, 0, true, ZEROLEVEL);
	}
out:
	safe_free(buf);
	return bRet;
}

/***************************************************************************************************
*  Function Name   : WriteZeroMBR
*  Description     : function to clean master boot record and write zero's into first 512 sectors.
*  Author Name	   : NITIN SHELAR
*  Date Of Creation: 21/02/2019
****************************************************************************************************/
int CWrdWizRescueDiskDlg::WriteZeroMBR(FILE *fp)
{
	int iRet;
	try
	{
		unsigned char aucRef[] = { 0x55, 0xAA };
		iRet = WriteData(fp, 0x0, mbr_zero_0x0, sizeof(mbr_zero_0x0)) && WriteData(fp, 0x1FE, aucRef, sizeof(aucRef));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception CVibraniumRescueDiskDlg::WriteZeroMBR()", 0, 0, true, ZEROLEVEL);
	}
	return iRet;
}

/***************************************************************************************************
*  Function Name   : WriteWinMBR
*  Description     : function to write windows MBR values into first 512 sector of USB drive.
*  Author Name	   : NITIN SHELAR
*  Date Of Creation: 21/02/2019
****************************************************************************************************/
int CWrdWizRescueDiskDlg::WriteWinMBR(FILE *fp)
{
	int iRet;
	try
	{
		unsigned char aucRef[] = { 0x55, 0xAA };
		iRet = WriteData(fp, 0x0, mbr_win_0x0, sizeof(mbr_win_0x0)) && WriteData(fp, 0x1FE, aucRef, sizeof(aucRef));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception CVibraniumRescueDiskDlg::WriteWinMBR()", 0, 0, true, ZEROLEVEL);
	}
	return iRet;
}

/***************************************************************************************************
*  Function Name   : FunUIMessages
*  Description     : function to send message flag to WardWizUI
*  Author Name	   : NITIN SHELAR
*  Date Of Creation: 28/02/2019
****************************************************************************************************/
void CWrdWizRescueDiskDlg::FunUIMessages(int dwFlagVal)
{
	try
	{
		m_status.call(sciter::value::value(dwFlagVal));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception CVibraniumRescueDiskDlg::FunUIMessages()", 0, 0, true, ZEROLEVEL);
	}
}

/***************************************************************************
Function Name  : FormatCDThread
Description    : Thread to format USB drive with MBR and extract iso file into drive.
Author Name    : NITIN SHELAR
Date           : 13/02/2019
****************************************************************************/
DWORD WINAPI FormatCDThread(LPVOID lpParam)
{
	if (lpParam == NULL)
		return 0;
	CWrdWizRescueDiskDlg *pThis = (CWrdWizRescueDiskDlg*)lpParam;

	IDiscMaster2* tmpDiscMaster = NULL;
	IDiscFormat2Data* dataWriter = NULL;
	IDiscRecorder2* discRecorder = NULL;

	HRESULT 	  hr = S_OK;
	LONG          index = 0;
	int			  FlagF = 0;
	
	CString csDiskDrive;
	csDiskDrive.Format(L"%s\\", pThis->m_csDrivePath.GetBuffer());
	BSTR szBstr = csDiskDrive.GetBuffer();
	pThis->m_dwPercentVal = 0;

	try
	{
		hr = CoCreateInstance(CLSID_MsftDiscMaster2, NULL, CLSCTX_ALL, IID_PPV_ARGS(&tmpDiscMaster));
		if (FAILED(hr))
		{
			goto out;
		}

		if (SUCCEEDED(hr))
		{
			hr = tmpDiscMaster->get_Count(&index);

			if (FAILED(hr))
			{
				goto out;
			}
		}
		tmpDiscMaster->Release();
		pThis->FunUIMessages(1);
		
		if (pThis->GetExtractData() == S_FALSE)
			goto out;

		for (LONG i = 0; i < index; i++)
		{
			hr = pThis->GetDiscRecorder(i, &discRecorder);
			if (SUCCEEDED(hr))
			{
				hr = discRecorder->AcquireExclusiveAccess(VARIANT_TRUE, SysAllocString(L"Drive"));
				if (FAILED(hr))
				{
					goto out;
				}
			}

			if (SUCCEEDED(hr))
			{
				SAFEARRAY * mountPoints = NULL;
				hr = discRecorder->get_VolumePathNames(&mountPoints);
				if (FAILED(hr))
				{
					AddLogEntry(L"### failed to get_VolumePathNames()", 0, 0, true, ZEROLEVEL);
					goto out;
				}
				else if (mountPoints->rgsabound[0].cElements == 0)
				{
					goto out;
				}
				else
				{
					VARIANT* tmp = (VARIANT*)(mountPoints->pvData);
					for (ULONG j = 0; j < mountPoints->rgsabound[0].cElements; j++)
					{		
						if (wcscmp(tmp[j].bstrVal, szBstr) == 0)
						{
							FlagF = 1;
							break;
						}
					}
				}
			}

			if (SUCCEEDED(hr))
			{
				hr = CoCreateInstance(CLSID_MsftDiscFormat2Data, NULL, CLSCTX_ALL, IID_PPV_ARGS(&dataWriter));
				if (FAILED(hr))
				{
					goto out;
				}

				if (SUCCEEDED(hr))
				{
					if (FlagF == 1)
					{
						hr = dataWriter->put_ClientName(SysAllocString(L"Drive"));
						if (FAILED(hr))
						{
							AddLogEntry(L"### failed to put_ClientName()", 0, 0, true, ZEROLEVEL);
							goto out;
						}

						hr = dataWriter->put_Recorder(discRecorder);
						if (FAILED(hr))
						{
							AddLogEntry(L"### failed in put_Recorder()", 0, 0, true, ZEROLEVEL);
							goto out;
						}
						else
						{
							::CoInitializeEx(NULL, COINIT_MULTITHREADED);
							hr = dataWriter->get_CurrentPhysicalMediaType(&pThis->MediaValue);
							if (FAILED(hr))
							{
								goto out;
							}
							CWrdWizRescueDiskDlg* eventSink = CWrdWizRescueDiskDlg::CreateEventSink();
							if (eventSink == NULL)
							{
								goto out;
							}

							if (!eventSink->ConnectDiscFormatData(dataWriter))
							{
								goto out;
							}

							eventSink->SetHwnd(pThis->get_hwnd());
							hr = pThis->CreateStreamISO(dataWriter);
							if (hr == S_FALSE)
								goto out;
						}
						discRecorder->ReleaseExclusiveAccess();
						pThis->ConnectionUnadvise();
						goto out;
					}
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in FormatCDThread()", 0, 0, true, ZEROLEVEL);
	}

out:
	if (dataWriter)
		dataWriter->Release();
	if (discRecorder)
		discRecorder->Release();
	pThis->FunFreeReource(pThis->m_hFormatCDThread);

	return 0;
}

/**********************************************************************************************************
*  Function Name  :	GetDiscRecorder
*  Description    :	Function to initialize disk recorder which is selected by user to write data.
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 04/06/2019
**********************************************************************************************************/
HRESULT CWrdWizRescueDiskDlg::GetDiscRecorder(__in ULONG index, __out IDiscRecorder2 ** recorder)
{
	IDiscMaster2* tmpDiscMaster = NULL;
	IDiscRecorder2* tmpRecorder = NULL;

	HRESULT hr = S_OK;
	BSTR tmpUniqueId;
	*recorder = NULL;

	try
	{
		hr = CoCreateInstance(CLSID_MsftDiscMaster2, NULL, CLSCTX_ALL, IID_PPV_ARGS(&tmpDiscMaster));
		if (FAILED(hr))
		{
			return hr;
		}

		hr = tmpDiscMaster->get_Item(index, &tmpUniqueId);
		if (FAILED(hr))
		{
			return hr;
		}

		tmpDiscMaster->Release();
		if (SUCCEEDED(hr))
		{
			hr = CoCreateInstance(CLSID_MsftDiscRecorder2, NULL, CLSCTX_ALL, IID_PPV_ARGS(&tmpRecorder));
			if (FAILED(hr))
			{
				return hr;
			}
		}

		hr = tmpRecorder->InitializeDiscRecorder(tmpUniqueId);
		if (FAILED(hr))
		{
			return hr;
		}

		if (SUCCEEDED(hr))
		{
			*recorder = tmpRecorder;
		}
		else
		{
			tmpRecorder->Release();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::GetDiscRecorder", 0, 0, true, ZEROLEVEL);
	}
	return hr;
}

/**********************************************************************************************************
*  Function Name  :	GetDrivePathCheck
*  Description    :	To select drive which contain greater than 0 size.
*  Author Name    : NITIN SHELAR
*  Date           : 09/12/2019
**********************************************************************************************************/
BOOL CWrdWizRescueDiskDlg::GetDrivePathCheck(CString csDriveName)
{
	BOOL bResult = FALSE;
	
	try
	{
		DISK_GEOMETRY pdg = { 0 };
		HANDLE hDevice = INVALID_HANDLE_VALUE;
		DWORD junk = 0;
		CString csPath;
		csPath.Format(L"\\\\.\\%s", csDriveName);

		hDevice = CreateFileW(csPath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

		if (hDevice == INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}
		else
		{
			DWORD dwFSize = 0;
			dwFSize = GetFileSize(hDevice, NULL);
			int iSize = dwFSize;
		}

		if (hDevice == INVALID_HANDLE_VALUE)
		{
			return FALSE;
		}

		bResult = DeviceIoControl(hDevice, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &pdg, sizeof(pdg), &junk, (LPOVERLAPPED)NULL);

		CloseHandle(hDevice);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::GetDrivePathCheck", 0, 0, true, ZEROLEVEL);
	}
	return bResult;
}

/***************************************************************************
Function Name  : CreateStreamISO
Description    : Impliments some interface this interface to write multiple
				 boot entries or boot images required for the EFI/UEFI support
Author Name    : NITIN SHELAR
Date           : 13/02/2019
****************************************************************************/
HRESULT CWrdWizRescueDiskDlg::CreateStreamISO(IDiscFormat2Data* IDataFormat2)
{
	IFileSystemImage2*		IFileSysimage2 = NULL;
	IFileSystemImageResult*	imageResult = NULL;
	IFsiDirectoryItem*		rootItem = NULL;
	IBootOptions* 			BootOption = NULL;

	WIN32_FIND_DATA ffd;
	TCHAR szDir[MAX_PATH];
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	TCHAR szProgData[MAX_PATH];
	IStream* pResultISOStream = NULL;
	HRESULT hr = S_OK;
	try
	{
		hr = CoCreateInstance(CLSID_MsftFileSystemImage, NULL, CLSCTX_ALL, IID_PPV_ARGS(&IFileSysimage2));
		if (FAILED(hr) || (IFileSysimage2 == NULL))
		{
			return S_FALSE;
		}

		FsiFileSystems FSIFileSystem = FsiFileSystemNone;
		FSIFileSystem = (FsiFileSystems)(FSIFileSystem | FsiFileSystemUDF);
		hr = IFileSysimage2->put_FileSystemsToCreate(FSIFileSystem);
		if (FAILED(hr))
		{
			return S_FALSE;
		}

		hr = IFileSysimage2->put_VolumeName(L"WWRscueDisk");
		if (FAILED(hr))
		{
			return S_FALSE;
		}
		
		hr = IFileSysimage2->ChooseImageDefaultsForMediaType(MediaValue);
		if (FAILED(hr))
		{
			return S_FALSE;
		}

		hr = CoCreateInstance(__uuidof(BootOptions), NULL, CLSCTX_INPROC_SERVER, __uuidof(IBootOptions), (void**)&BootOption);
		if (FAILED(hr))
		{
			return S_FALSE;
		}

		hr = BootOption->put_Manufacturer(SysAllocString(L"Vibranium"));
		if (FAILED(hr))
		{
			return S_FALSE;
		}

		hr = BootOption->put_PlatformId(PlatformX86); // don't change
		if (FAILED(hr))
		{
			return S_FALSE;
		}

		hr = BootOption->put_Emulation(EmulationNone); // don't change
		if (FAILED(hr))
		{
			return S_FALSE;
		}

		SHGetSpecialFolderPath(NULL, szProgData, CSIDL_COMMON_APPDATA, FALSE);
		IStream* pBootStream = NULL;
		TCHAR szEtfsBoot[MAX_PATH];

		wsprintf(szEtfsBoot,L"%s\\Vibranium\\RescueTemp\\Boot\\etfsboot.com",szProgData);
		hr = SHCreateStreamOnFile(szEtfsBoot, STGM_READ | STGM_SHARE_DENY_WRITE, &pBootStream);
		if (pBootStream == NULL)
		{
			return S_FALSE;
		}

		hr = BootOption->AssignBootImage(pBootStream);
		if (FAILED(hr))
		{
			return S_FALSE;
		}

		hr = IFileSysimage2->put_BootImageOptions(BootOption);
		if (FAILED(hr))
		{
			return S_FALSE;
		}

		hr = IFileSysimage2->get_Root(&rootItem);
		if (SUCCEEDED(hr))
		{
			CString DestFiles;
			TCHAR szPassSrc[MAX_PATH];
			IStream* fileStream;
			CString csSrcPath;
			csSrcPath.Format(L"%s\\Vibranium\\RescueTemp", szProgData);
			
			length_of_arg = wcslen(csSrcPath);
			if (length_of_arg > (MAX_PATH - 3))
			{
				return S_FALSE;
			}

			wcscpy(szDir, csSrcPath);
			wcscat(szDir, TEXT("\\*"));

			hFind = FindFirstFile(szDir, &ffd);
			int cnt = 0;
			do
			{
				if (cnt > 1)
				{
					DestFiles = ffd.cFileName;
					wsprintf(szPassSrc, TEXT("%s\\%s"), csSrcPath, DestFiles);
					if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						hr = rootItem->AddTree(SysAllocString(szPassSrc), VARIANT_TRUE);
					}
					else
					{
						SHCreateStreamOnFileEx(szPassSrc,
							STGM_READ,
							FILE_ATTRIBUTE_NORMAL,
							FALSE,
							NULL,
							&fileStream);
						if (fileStream == NULL)
							return S_FALSE;
						hr = rootItem->AddFile(SysAllocString(DestFiles.GetBuffer()), fileStream);
						if (FAILED(hr))
							return S_FALSE;
					}
				}
				cnt++;
			} while (FindNextFile(hFind, &ffd) != 0);
			FindClose(hFind);

			hr = IFileSysimage2->CreateResultImage(&imageResult);
			if (FAILED(hr))
			{
				return S_FALSE;
			}
			hr = imageResult->get_ImageStream(&pResultISOStream);
		}

		hr = IDataFormat2->Write(pResultISOStream);
		if (FAILED(hr))
		{
			return S_FALSE;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::CreateStreamISO", 0, 0, true, ZEROLEVEL);
	}
	return hr;
}

/***************************************************************************
Function Name  : GetExtractData
Description    : Impliments some interface this interface to write multiple
				 boot entries or boot images required for the EFI/UEFI support
Author Name    : NITIN SHELAR
Date           : 13/02/2019
****************************************************************************/
HRESULT CWrdWizRescueDiskDlg::GetExtractData()
{
	TCHAR szProgData[255];
	CString csPath, csFilePath, csDrivePath;
	DWORD dwExitCode = 0x00;
	HRESULT hr = S_OK;
	try
	{
		SHELLEXECUTEINFO sei = { sizeof(sei) };
		ZeroMemory(&sei, sizeof(sei));
		SHGetSpecialFolderPath(NULL, szProgData, CSIDL_COMMON_APPDATA, FALSE);
		csPath.Format(L"%s\\WardWiz", szProgData);
		if (!PathFileExists(csPath))
			CreateDirectory(csPath, NULL);
		
		csPath.Format(L"%s\\Vibranium\\RescueTemp", szProgData);
		if (!PathFileExists(csPath))
			CreateDirectory(csPath, NULL);

		csFilePath = theApp.m_objwardwizLangManager.GetWardWizPathFromRegistry();
		csFilePath.Append(L"7z.exe");
		csDrivePath.Format(L"x \"%s\" -o\"%s\\Vibranium\\RescueTemp\"", m_csIsoFilePath, szProgData);
		sei.lpFile = csFilePath;
		sei.lpParameters = csDrivePath;
		sei.nShow = SW_HIDE;
		sei.cbSize = sizeof(SHELLEXECUTEINFO);
		sei.fMask = SEE_MASK_NOCLOSEPROCESS;
		UpdateProgressEvent();
		if (!ShellExecuteEx(&sei))
		{
			return S_FALSE;
		}
		else
		{
			for (;;)
			{
				if (GetExitCodeProcess(sei.hProcess, &dwExitCode) == FALSE)
					break;

				if (dwExitCode != STILL_ACTIVE || ERROR_NO_MORE_ITEMS == dwExitCode)
				{
					UpdateProgressEvent();
					safe_closehandle(sei.hProcess);
					break;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::GetExtractData()", 0, 0, true, ZEROLEVEL);
	}
	return hr;
}

/**********************************************************************************************************
*  Function Name  :	OnImapiUpdate()
*  Description    :	Function to catch IMAPI messages which send from device event.
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 12/06/2019
**********************************************************************************************************/
LRESULT CWrdWizRescueDiskDlg::OnImapiUpdate(WPARAM wParam, LPARAM lParam)
{
	try
	{
		IMAPI_FORMAT2_DATA_WRITE_ACTION currentAction = (IMAPI_FORMAT2_DATA_WRITE_ACTION)wParam;
		PIMAPI_STATUS pImapiStatus = (PIMAPI_STATUS)lParam;

		switch (currentAction)
		{
		case IMAPI_FORMAT2_DATA_WRITE_ACTION_VALIDATING_MEDIA:
			break;

		case IMAPI_FORMAT2_DATA_WRITE_ACTION_FORMATTING_MEDIA:
			break;

		case IMAPI_FORMAT2_DATA_WRITE_ACTION_INITIALIZING_HARDWARE:
			UpdateProgressEvent();
			break;

		case IMAPI_FORMAT2_DATA_WRITE_ACTION_CALIBRATING_POWER:
			break;

		case IMAPI_FORMAT2_DATA_WRITE_ACTION_WRITING_DATA:
			FunUIMessages(2);
			if (m_dwPercentVal > 95)
			{
				m_dwPercentVal = 96;
				FunUIMessages(3);
			}
			UpdateProgressEvent();
			break;

		case IMAPI_FORMAT2_DATA_WRITE_ACTION_FINALIZATION:
			break;

		case IMAPI_FORMAT2_DATA_WRITE_ACTION_COMPLETED:
			m_dwPercentVal = 99;
			UpdateProgressEvent();
			FunUIMessages(4);
			break;
		}
		return RETURN_CONTINUE_WRITE;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::OnImapiUpdate()", 0, 0, true, ZEROLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	Update()
*  Description    :	Function to call if after disk writing started and to send IMAPI 
					messages to another UI event function
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 12/06/2019
**********************************************************************************************************/
STDMETHODIMP_(HRESULT) CWrdWizRescueDiskDlg::XFormatDataEvents::Update(IDispatch* objectDispatch, IDispatch* progressDispatch)
{
	try
	{
		METHOD_PROLOGUE(CWrdWizRescueDiskDlg, FormatDataEvents)

			IDiscFormat2DataEventArgs* progress = NULL;
		HRESULT hr = progressDispatch->QueryInterface(IID_PPV_ARGS(&progress));

		IDiscFormat2Data* discFormatData = NULL;
		hr = objectDispatch->QueryInterface(IID_PPV_ARGS(&discFormatData));

		IMAPI_FORMAT2_DATA_WRITE_ACTION currentAction = IMAPI_FORMAT2_DATA_WRITE_ACTION_VALIDATING_MEDIA;
		hr = progress->get_CurrentAction(&currentAction);
		ASSERT(SUCCEEDED(hr));
		if (FAILED(hr))
		{
			return S_OK;
		}

		IMAPI_STATUS imapiStatus = { 0 };

		if ((currentAction == IMAPI_FORMAT2_DATA_WRITE_ACTION_WRITING_DATA) || (currentAction == IMAPI_FORMAT2_DATA_WRITE_ACTION_FINALIZATION))
		{
			progress->get_ElapsedTime(&imapiStatus.elapsedTime);
			progress->get_RemainingTime(&imapiStatus.remainingTime);
			progress->get_TotalTime(&imapiStatus.totalTime);

			if (currentAction == IMAPI_FORMAT2_DATA_WRITE_ACTION_WRITING_DATA)
			{
				progress->get_StartLba(&imapiStatus.startLba);
				progress->get_SectorCount(&imapiStatus.sectorCount);
				progress->get_LastReadLba(&imapiStatus.lastReadLba);
				progress->get_LastWrittenLba(&imapiStatus.lastWrittenLba);
				progress->get_TotalSystemBuffer(&imapiStatus.totalSystemBuffer);
				progress->get_UsedSystemBuffer(&imapiStatus.usedSystemBuffer);
				progress->get_FreeSystemBuffer(&imapiStatus.freeSystemBuffer);
			}
		}

		HWND hWindow = ::FindWindow(NULL, L"VibraniumRESCUDISK ");
		if (hWindow == NULL)
			return S_FALSE;
		LRESULT ret = ::SendMessage(hWindow, WM_IMAPI_UPDATE, currentAction, (LPARAM)(LPVOID)&imapiStatus);
		if (ret == RETURN_CANCEL_WRITE)
		{
			discFormatData->CancelWrite();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::XFormatDataEvents::Update()", 0, 0, true, ZEROLEVEL);
	}
	return S_OK;
}

/**********************************************************************************************************
*  Function Name  :	ConnectDiscFormatData()
*  Description    :	Establishes a link between the CWrdWizRescueDiskDlg to IDiscFormat2Data
					that can be received Update messages
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 12/06/2019
**********************************************************************************************************/
bool CWrdWizRescueDiskDlg::ConnectDiscFormatData(IDiscFormat2Data* IDsikObj)
{
	try
	{
		m_pUnkSink = GetIDispatch(TRUE);
		m_pUnkSrc = IDsikObj;

		LPTYPELIB ptlib = NULL;
		HRESULT hr = LoadRegTypeLib(LIBID_IMAPILib2,
			IMAPILib2_MajorVersion, IMAPILib2_MinorVersion,
			LOCALE_SYSTEM_DEFAULT, &ptlib);
		if (FAILED(hr))
		{
			return false;
		}
		hr = ptlib->GetTypeInfoOfGuid(IID_DDiscFormat2DataEvents, &m_ptinfo);
		ptlib->Release();
		if (FAILED(hr))
		{
			return false;
		}

		BOOL bRet = AfxConnectionAdvise(m_pUnkSrc, IID_DDiscFormat2DataEvents, m_pUnkSink, TRUE, &m_dwCookie);
		if (bRet)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::ConnectDiscFormatData()", 0, 0, true, ZEROLEVEL);
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	CreateEventSink()
*  Description    :	Call this function to enable OLE automation for an object to create the event sink.
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 12/06/2019
**********************************************************************************************************/
CWrdWizRescueDiskDlg* CWrdWizRescueDiskDlg::CreateEventSink()
{
	try
	{
		CWrdWizRescueDiskDlg* pCWrdWizRescueDiskDlg = new CWrdWizRescueDiskDlg();
		if (pCWrdWizRescueDiskDlg == NULL)
			return NULL;
		pCWrdWizRescueDiskDlg->EnableAutomation();
		pCWrdWizRescueDiskDlg->ExternalAddRef();
		return pCWrdWizRescueDiskDlg;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::CreateEventSink()", 0, 0, true, ZEROLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	AddRef()
*  Description    :	Increments the reference count for an interface pointer to a COM object
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 12/06/2019
**********************************************************************************************************/
ULONG FAR EXPORT CWrdWizRescueDiskDlg::XFormatDataEvents::AddRef()
{
	try
	{
		METHOD_PROLOGUE(CWrdWizRescueDiskDlg, FormatDataEvents)
			return pThis->ExternalAddRef();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::XFormatDataEvents::AddRef()", 0, 0, true, ZEROLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	Release()
*  Description    :	Decrements the reference count for an interface on a COM object.
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 12/06/2019
**********************************************************************************************************/
ULONG FAR EXPORT CWrdWizRescueDiskDlg::XFormatDataEvents::Release()
{
	try
	{
		METHOD_PROLOGUE(CWrdWizRescueDiskDlg, FormatDataEvents)
			return pThis->ExternalRelease();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::XFormatDataEvents::Release()", 0, 0, true, ZEROLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	QueryInterface()
*  Description    :	Queries a COM object for a pointer to one of its interface.
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 12/06/2019
**********************************************************************************************************/
STDMETHODIMP CWrdWizRescueDiskDlg::XFormatDataEvents::QueryInterface(REFIID riid, LPVOID FAR* ppvObj)
{
	try
	{
		METHOD_PROLOGUE(CWrdWizRescueDiskDlg, FormatDataEvents)
			return (HRESULT)pThis->ExternalQueryInterface(&riid, ppvObj);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::XFormatDataEvents::QueryInterface()", 0, 0, true, ZEROLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	GetTypeInfoCount()
*  Description    :	Retrieves the number of type information interfaces that an object provides
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 12/06/2019
**********************************************************************************************************/
STDMETHODIMP CWrdWizRescueDiskDlg::XFormatDataEvents::GetTypeInfoCount(UINT FAR* pctinfo)
{
	try
	{
		METHOD_PROLOGUE(CWrdWizRescueDiskDlg, FormatDataEvents)
			*pctinfo = 1;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::XFormatDataEvents::GetTypeInfoCount()", 0, 0, true, ZEROLEVEL);
	}
	return NOERROR;
}

/**********************************************************************************************************
*  Function Name  :	GetTypeInfo()
*  Description    :	Returns the TypeInfo representation of the specified type.
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 12/06/2019
**********************************************************************************************************/
STDMETHODIMP CWrdWizRescueDiskDlg::XFormatDataEvents::GetTypeInfo(
	UINT itinfo,
	LCID lcid,
	ITypeInfo FAR* FAR* pptinfo)
{
	try
	{
		METHOD_PROLOGUE(CWrdWizRescueDiskDlg, FormatDataEvents)
			*pptinfo = NULL;

		if (itinfo != 0)
			return ResultFromScode(DISP_E_BADINDEX);
		pThis->m_ptinfo->AddRef();
		*pptinfo = pThis->m_ptinfo;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::XFormatDataEvents::GetTypeInfo()", 0, 0, true, ZEROLEVEL);
	}
	return NOERROR;
}

/**********************************************************************************************************
*  Function Name  :	GetIDsOfNames()
*  Description    :	Maps a single member and an optional set of argument names to a corresponding set of 
					integer DISPIDs
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 12/06/2019
**********************************************************************************************************/
STDMETHODIMP CWrdWizRescueDiskDlg::XFormatDataEvents::GetIDsOfNames(
	REFIID riid,
	OLECHAR FAR* FAR* rgszNames,
	UINT cNames,
	LCID lcid,
	DISPID FAR* rgdispid)
{
	try
	{
		METHOD_PROLOGUE(CWrdWizRescueDiskDlg, FormatDataEvents)
			return DispGetIDsOfNames(pThis->m_ptinfo, rgszNames, cNames, rgdispid);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::XFormatDataEvents::GetIDsOfNames()", 0, 0, true, ZEROLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	Invoke()
*  Description    :	Provides access to properties and methods exposed by an object.
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 12/06/2019
**********************************************************************************************************/
STDMETHODIMP CWrdWizRescueDiskDlg::XFormatDataEvents::Invoke(
	DISPID dispidMember,
	REFIID riid,
	LCID lcid,
	WORD wFlags,
	DISPPARAMS FAR* pdispparams,
	VARIANT FAR* pvarResult,
	EXCEPINFO FAR* pexcepinfo,
	UINT FAR* puArgErr)
{
	try
	{
		METHOD_PROLOGUE(CWrdWizRescueDiskDlg, FormatDataEvents)

		return DispInvoke(&pThis->m_xFormatDataEvents, pThis->m_ptinfo,
			dispidMember, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::XFormatDataEvents::Invoke()", 0, 0, true, ZEROLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	ConnectionUnadvise()
*  Description    :	Terminates an advisory connection previously established between a connection point 
					object and a client's sink.
*  Author Name    : NITIN SHELAR
*  SR_NO		  :
*  Date			  : 12/06/2019
**********************************************************************************************************/
void CWrdWizRescueDiskDlg::ConnectionUnadvise()
{
	try
	{
		if (m_dwCookie && (m_pUnkSrc != NULL) && (m_pUnkSink != NULL))
		{
			AfxConnectionUnadvise(m_pUnkSrc, IID_DDiscFormat2DataEvents, m_pUnkSink, TRUE, m_dwCookie);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::ConnectionUnadvise()", 0, 0, true, ZEROLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : FunCheckInternetAccessBlock
*  Description    : To check internet access block
*  Author Name    : Jeena Mariam Saji
*  Date			  : 13 Dec 2019
****************************************************************************************************/
json::value CWrdWizRescueDiskDlg::FunCheckInternetAccessBlock()
{
	bool RetVal = false;
	try
	{
		DWORD dwProductID = theApp.m_objwardwizLangManager.GetSelectedProductID();
		if (dwProductID == BASIC || dwProductID == ESSENTIAL)
		{
			return false;
		}

		CString csRegKeyVal;
		csRegKeyVal = CWWizSettingsWrapper::GetProductRegistryKey();
		CITinRegWrapper objReg;
		DWORD dwParentalControl = 0x00;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyVal.GetBuffer(), L"dwParentalCntrlFlg", dwParentalControl) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwParentalCntrlFlg in CVibraniumRescueDiskDlg::FunCheckInternetAccessBlock", 0, 0, true, SECONDLEVEL);
		}

		if (dwParentalControl == 1)
		{
			ISPY_PIPE_DATA szPipeData = { 0 };
			szPipeData.iMessageInfo = ON_CHECK_INTERNET_ACCESS;

			CISpyCommunicator objCom(SERVICE_SERVER);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send Data in CVibraniumRescueDiskDlg::SendData", 0, 0, true, SECONDLEVEL);
			}

			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to read Data in CVibraniumRescueDiskDlg::ReadData", 0, 0, true, SECONDLEVEL);
			}

			DWORD dwVal = szPipeData.dwValue;
			if (dwVal == 0x01)
			{
				RetVal = true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRescueDiskDlg::FunCheckInternetAccessBlock()", 0, 0, true, SECONDLEVEL);
	}
	return RetVal;
}