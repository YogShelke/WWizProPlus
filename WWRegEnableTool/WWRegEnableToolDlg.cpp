
/*********************************************************************
*  Program Name		: WWRegEnableToolDlg.cpp
*  Description		: WWRegEnableToolDlg Implementation to Enable or disable 
					  system tools from registry
*  Author Name		: NITIN SHELAR
*  Date Of Creation	: 22/11/2019
**********************************************************************/

#include "stdafx.h"
#include "Constants.h"
#include "WWRegEnableTool.h"
#include "WWRegEnableToolDlg.h"
#include "afxdialogex.h"
#include "EnumProcess.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About

/***************************************************************************************************
*  Function Name  : CAboutDlg
*  Description    : C'tor
*  Author Name    : NITIN SHELAR
*  Date           : 22/11/2019
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
Date           : 22/11/2019
****************************************************************************/
void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

/***************************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : NITIN SHELAR
*  Date           : 22/11/2019
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CWWRegEnableToolDlg * CWWRegEnableToolDlg::m_pThis = NULL;
// CWWRegEnableToolDlg dialog

/***************************************************************************
Function Name  : CWWRegEnableToolDlg()
Description    : constructor
Author Name    : NITIN SHELAR
Date           : 22/11/2019
****************************************************************************/
CWWRegEnableToolDlg::CWWRegEnableToolDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWWRegEnableToolDlg::IDD, pParent)
{
	m_pThis = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

/***************************************************************************
Function Name  : DoDataExchange()
Description    : Called by the framework to exchange and validate dialog data.
Author Name    : NITIN SHELAR
Date           : 22/11/2019
****************************************************************************/
void CWWRegEnableToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

/***************************************************************************************************
*  Function Name  : MESSAGE_MAP
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : NITIN SHELAR
*  Date           : 22/11/2019
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CWWRegEnableToolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDCANCEL, &CWWRegEnableToolDlg::OnBnClickedOk)
END_MESSAGE_MAP()

HWINDOW   CWWRegEnableToolDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWWRegEnableToolDlg::get_resource_instance() { return theApp.m_hInstance; }

// CWWRegEnableToolDlg message handlers

/***************************************************************************
Function Name  : OnInitDialog()
Description    : Initializes the dialog window.
Author Name    : NITIN SHELAR
Date           : 22/11/2019
****************************************************************************/
BOOL CWWRegEnableToolDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	
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

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	LPCBYTE pb = 0; UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events

	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_REG_ENABLE_TOOL.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_REG_ENABLE_TOOL.htm");

	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;

	m_root_el = root();
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);

	int cxIcon = GetSystemMetrics(SM_CXFULLSCREEN);
	int cyIcon = GetSystemMetrics(SM_CYFULLSCREEN);

	int ixRect = cxIcon - pIntMaxWidth;
	int iyRect = cyIcon - pIntHeight;

	::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);
	SetWindowText(L"VibraniumREGTOOL");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

/**********************************************************************************************************
*  Function Name		: WindowProc
*  Description			: Window procedure added
*  Author Name			: NITIN SHELAR
*  Date					: 22/11/2019
**********************************************************************************************************/
LRESULT CWWRegEnableToolDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class

	LRESULT lResult;
	BOOL    bHandled = FALSE;
	__try
	{
		lResult = SciterProcND(this->GetSafeHwnd(), message, wParam, lParam, &bHandled);
		if (bHandled)      // if it was handled by the Sciter
			return lResult; // then no further processing is required.
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in CPopUpDialog::WindowProc", 0, 0, true, SECONDLEVEL);
	}
	return __super::WindowProc(message, wParam, lParam);
}

/***************************************************************************
Function Name  : OnSysCommand()
Description    : The framework calls this member function when the user selects a command
from the Control menu, or when the user selects the Maximize or the Minimize button.
Author Name    : NITIN SHELAR
Date           : 22/11/2019
****************************************************************************/
void CWWRegEnableToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	try
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
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizRegEnableToolDlg::OnSysCommand()", 0, 0, true, ZEROLEVEL);
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
Date           : 22/11/2019
****************************************************************************/
void CWWRegEnableToolDlg::OnPaint()
{
	try
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
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizRegEnableToolDlg::OnPaint()", 0, 0, true, ZEROLEVEL);
	}
}

/***************************************************************************
Function Name  : OnQueryDragIcon()
Description    : The system calls this function to obtain the cursor to 
				 display while the user drags.
Author Name    : NITIN SHELAR
Date           : 22/11/2019
****************************************************************************/
HCURSOR CWWRegEnableToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/***************************************************************************
Function Name  : OnBnClickedOk()
Description    : This function close dialog
Author Name    : NITIN SHELAR
Date           : 21/11/2019
****************************************************************************/
void CWWRegEnableToolDlg::OnBnClickedOk()
{
	CDialogEx::OnCancel();
}

/***************************************************************************************************
*  Function Name  : On_Close
*  Description    : Function to close dialog box from UI
*  Author Name    : NITIN SHELAR
*  Date			  : 22/11/2019
****************************************************************************************************/
json::value CWWRegEnableToolDlg::On_Close()
{
	try
	{
		OnBnClickedOk();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizRegEnableToolDlg::On_Close()", 0, 0, true, ZEROLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_Minimize
*  Description    : Function to minmize dialog box from UI
*  Author Name    : NITIN SHELAR
*  Date			  : 22/11/2019
****************************************************************************************************/
json::value CWWRegEnableToolDlg::On_Minimize()
{
	try
	{
		this->ShowWindow(SW_MINIMIZE);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizRegEnableToolDlg::On_Close()", 0, 0, true, ZEROLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name	  : OnGetAppPath
Description		  : Get the App Path and Set it in Script
Author Name		  : NITIN SHELAR
Date			  : 22/11/2019
/***************************************************************************************************/
json::value CWWRegEnableToolDlg::OnGetAppPath()
{
	SCITER_STRING RetAppPath;
	try
	{
		RetAppPath = (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizRegEnableToolDlg::OnGetAppPath()", 0, 0, true, ZEROLEVEL);
	}
	return RetAppPath;
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : NITIN SHELAR
*  Date			  : 22/11/2019
****************************************************************************************************/
json::value CWWRegEnableToolDlg::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProdID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizRegEnableToolDlg::On_GetProductID()", 0, 0, true, ZEROLEVEL);
	}

	return iProdValue;
}

/***************************************************************************************************
*  Function Name  : On_GetThemeID
*  Description    : Get the Theme IDs
*  Author Name    : NITIN SHELAR
*  Date			  : 22/11/2019
****************************************************************************************************/
json::value CWWRegEnableToolDlg::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizRegEnableToolDlg::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_GetLanguageID
*  Description    : Get the language id
*  Author Name    : NITIN SHELAR
*  Date			  : 22/11/2019
****************************************************************************************************/
json::value CWWRegEnableToolDlg::On_GetLanguageID()
{
	int iLangValue = 0;
	try
	{
		iLangValue = theApp.m_objwardwizLangManager.GetSelectedLanguage();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizRegEnableToolDlg::On_GetLanguageID()", 0, 0, true, ZEROLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
*  Function Name  : onModalLoop
*  Description    : Function to set event message dialog of UI
*  Author Name    : NITIN SHELAR
*  Date			  : 26/11/2019
****************************************************************************************************/
json::value CWWRegEnableToolDlg::onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal)
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
		AddLogEntry(L"### Exception in CWardwizRegEnableToolDlg::onModalLoop", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_GetRestart
*  Description    : Function to restart machine.
*  Author Name    : NITIN SHELAR
*  Date			  : 26/11/2019
****************************************************************************************************/
json::value CWWRegEnableToolDlg::On_GetRestart()
{
	try
	{
		CEnumProcess RebootObj;
		RebootObj.RebootSystem(0);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizRegEnableToolDlg::On_GetRestart", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************
Function Name  : SetRegistryValueData()
Description    : To change registry DWORD value 1 or 0 for Enable/Disable system tools.
Author Name    : NITIN SHELAR
Date           : 21/11/2019
****************************************************************************/
DWORD CWWRegEnableToolDlg::SetRegistryValueData(HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, DWORD dwData)
{
	DWORD	dwRet = 0x00;
	HKEY	hSubKey = NULL;

	try
	{
		RegCreateKeyEx(hRootKey, pKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hSubKey, NULL);
		if (!hSubKey)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		dwRet = RegSetValueEx(hSubKey, pValueName, 0, REG_DWORD, (LPBYTE)&dwData, sizeof(DWORD));
		RegCloseKey(hSubKey);
		hSubKey = NULL;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizRegEnableToolDlg::SetRegistryValueData", 0, 0, true, SECONDLEVEL);
	}

Cleanup:
	return dwRet;
}

/***************************************************************************************************
*  Function Name  : On_GetRegValue
*  Description    : Function to read and display current status of registry value on UI.
*  Author Name    : NITIN SHELAR
*  Date			  : 25/11/2019
****************************************************************************************************/
json::value CWWRegEnableToolDlg::On_GetRegValue(SCITER_VALUE scFunGetCurrentUserValueCB, SCITER_VALUE funSetChkVal)
{
	try
	{
		ReadCurrentUserRegValues(scFunGetCurrentUserValueCB, funSetChkVal);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizRegEnableToolDlg::On_GetRegValue", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
*  Function Name  : On_ChangeKeyValue
*  Description    : Function to reset registry value.
*  Author Name    : NITIN SHELAR
*  Date			  : 25/11/2019
****************************************************************************************************/
json::value CWWRegEnableToolDlg::On_ChangeKeyValue(SCITER_VALUE scfunSetMsgCB, SCITER_VALUE scFunSetCurrentUserValueCB)
{
	CString csKeyName;
	DWORD	dwValue;
	bool	bSelected;
	bool	FlagRestart;

	try
	{
		int iCount = scFunSetCurrentUserValueCB.length();
		FlagRestart = false;

		for (int i = 0; i < iCount; i++)
		{
			const SCITER_VALUE svEachEntry = scFunSetCurrentUserValueCB[i];
			bSelected = svEachEntry[L"selected"].get(false);
			dwValue = (bSelected) ? 1 : 0;
			csKeyName = csRegArray.GetAt(i);
			TCHAR str[234];
			wsprintf(str, L"%d", dwValue);

			if (dwValue == 0)
			{
				if (i < 5)
				{
					SetRegistryValueData(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", csKeyName.GetBuffer(), dwValue);
					SetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System", csKeyName.GetBuffer(), dwValue);
				}
				else
				{
					SetRegistryValueData(HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", csKeyName.GetBuffer(), dwValue);
					SetRegistryValueData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer", csKeyName.GetBuffer(), dwValue);

					if (baRecArr[i] != bSelected)
						FlagRestart = true;
				}
			}
		}

		if (FlagRestart)
			scfunSetMsgCB.call(true);
		else
			scfunSetMsgCB.call(false);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizRegEnableToolDlg::On_ChangeKeyValue", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***********************************************************************************************
Function Name  : ReadCurrentUserRegValues
Description    : Read HKCU key value from Registry
Author Name    : NITIN SHELAR
Date           : 22/11/2019
***********************************************************************************************/
void CWWRegEnableToolDlg::ReadCurrentUserRegValues(SCITER_VALUE scFunSetCurrentUserValueCB, SCITER_VALUE funSetChkVal)
{
	try
	{
		CString		strKey, StrKeyName;
		DWORD		DwRet = 0;
		bool		bSelect;

		for (DWORD dwValue = 0; dwValue <= 7; dwValue++)
		{
			switch (dwValue)
			{
			case 0:
				strKey = L"DisableTaskMgr";
				StrKeyName = L"Task Manager";
				break;
			case 1:
				strKey = L"NoDispCPL";
				StrKeyName = L"Display properties";
				break;
			case 2:
				strKey = L"DisableChangePassword";
				StrKeyName = L"Change Password";
				break;
			case 3:
				strKey = L"DisableLockWorkstation";
				StrKeyName = L"Lock Computer";
				break;
			case 4:
				strKey = L"DisableRegistryTools";
				StrKeyName = L"Registry Tools";
				break;
			case 5:
				strKey = L"NoClose";
				StrKeyName = L"Computer Shutdown";
				break;
			case 6:
				strKey = L"NoTrayContextMenu";
				StrKeyName = L"Taskbar Menu";
				break;
			case 7:
				strKey = L"NoRun";
				StrKeyName = L"Run Dialog";
				break;
			default:
				break;
			}
			csRegArray.Add(strKey);

			if (dwValue < 5)
			{
				DwRet = ReadDwordRegVal(HKEY_LOCAL_MACHINE, strKey, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System");
				DwRet = ReadDwordRegVal(HKEY_CURRENT_USER, strKey, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System");
			}
			else
			{
				DwRet = ReadDwordRegVal(HKEY_LOCAL_MACHINE, strKey, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer");
				DwRet = ReadDwordRegVal(HKEY_CURRENT_USER, strKey, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer");
			}

			if (DwRet)
				bSelect = false;
			else
				bSelect = true;

			baRecArr[dwValue] = bSelect;
			scFunSetCurrentUserValueCB.call((SCITER_STRING)StrKeyName, bSelect);
		}
		funSetChkVal.call();
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CWardwizRegEnableToolDlg::ReadCurrentUserRegValues", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : ReadDwordRegVal
Description    : Read HKCU key value from Registry
Author Name    : NITIN SHELAR
Date           : 22/11/2019
***********************************************************************************************/
DWORD CWWRegEnableToolDlg::ReadDwordRegVal(HKEY hKeyRoot, CString strKey, CString SubKey)
{
	try
	{
		DWORD dwType = REG_DWORD;
		DWORD RetVal = 0;
		DWORD dwSize = sizeof(RetVal);
		CString key, subKey;
		key = strKey;
		subKey = SubKey;
		HKEY hKey = NULL;

		LONG lResult = RegOpenKeyEx(hKeyRoot, subKey.GetBuffer(), 0, KEY_READ, &hKey);

		if (lResult == ERROR_SUCCESS)
		{
			lResult = RegQueryValueEx(hKey, key.GetBuffer(), NULL, &dwType, (LPBYTE)&RetVal, &dwSize);
			if (lResult == ERROR_SUCCESS)
				return RetVal;
		}
		return RetVal;
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CWardwizRegEnableToolDlg::ReadDwordRegVal", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}
