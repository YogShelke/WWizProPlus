
/*********************************************************************
*  Program Name: CWrdWizProcessExplorerDlg.cpp
*  Description: CWrdWizProcessExplorerDlg Implementation
*  Author Name: Kunal Waghmare
*  Date Of Creation: 17th October 2018
**********************************************************************/
#include "stdafx.h"
#include "WrdWizProcessExplorer.h"
#include "WrdWizProcessExplorerDlg.h"
#include "afxdialogex.h"
#include <Psapi.h>
#include <tlhelp32.h>
//#include <CommonControls.h>
//#include "sciter-x-graphics.hpp"	


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

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

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CWrdWizProcessExplorerDlg dialog



CWrdWizProcessExplorerDlg::CWrdWizProcessExplorerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWrdWizProcessExplorerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWrdWizProcessExplorerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CWrdWizProcessExplorerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//ON_BN_CLICKED(IDCANCEL, &CWrdWizProcessExplorerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDCANCEL, &CWrdWizProcessExplorerDlg::OnBnClickedButtonClose)


END_MESSAGE_MAP()


HWINDOW   CWrdWizProcessExplorerDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWrdWizProcessExplorerDlg::get_resource_instance() { return theApp.m_hInstance; }

// CWrdWizProcessExplorerDlg message handlers

BOOL CWrdWizProcessExplorerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	OnCancel();

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

	SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0; UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
	// load intial document
	//sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_PROCESS_EXPLORER.htm", pb, cb);
	//(this)->load_html(pb, cb, L"res:IDR_HTM_PROCESS_EXPLORER.htm");

	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	m_root_el = root();
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);

	::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);
	// TODO: Add extra initialization here
	
	this->SetWindowText(L"WRDWIZPROCESSEXPLORER");

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CWrdWizProcessExplorerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CWrdWizProcessExplorerDlg::OnPaint()
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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CWrdWizProcessExplorerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CWrdWizProcessExplorerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class

	LRESULT lResult;
	BOOL    bHandled = FALSE;

	lResult = SciterProcND(this->GetSafeHwnd(), message, wParam, lParam, &bHandled);
	if (bHandled)      // if it was handled by the Sciter
		return lResult; // then no further processing is required.

	return __super::WindowProc(message, wParam, lParam);
}

void CWrdWizProcessExplorerDlg::OnBnClickedButtonClose()
{
	// TODO: Add your control notification handler code here
	//CDialogEx::OnCancel();
	OnCancel();
}

json::value  CWrdWizProcessExplorerDlg::On_OnCloseButton()
{
	OnBnClickedButtonClose();
	return 0;
}

json::value CWrdWizProcessExplorerDlg::On_GetLanguageID()
{
	int iLangValue = 0;
	try
	{
		iLangValue = theApp.m_objwardwizLangManager.GetSelectedLanguage();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumProcessExplorerDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

json::value CWrdWizProcessExplorerDlg::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProdID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumProcessExplorerDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iProdValue;
}

json::value CWrdWizProcessExplorerDlg::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumProcessExplorerDlg::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

json::value CWrdWizProcessExplorerDlg::On_Minimize()
{
	this->ShowWindow(SW_MINIMIZE);
	return json::value();
}

json::value CWrdWizProcessExplorerDlg::GetActiveProcessesName(SCITER_VALUE svGetProcessesName)
{
	m_svGetProcessesName = svGetProcessesName;
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return 1;
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		return 1;
	}

	do
	{
		HANDLE		hProcess = NULL;
		HMODULE		hMods = NULL;
		DWORD		dwModules = 0x00;
		TCHAR szProcessName[MAX_PATH] = { '\0' };
		DWORD dwSize = sizeof(szProcessName) / sizeof(TCHAR);
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
		if (hProcess != NULL)
		{
			GetModuleFileNameEx(hProcess, hMods, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));	
		}			
		m_svGetProcessesName.call((SCITER_STRING)pe32.szExeFile, (SCITER_STRING)szProcessName);
	} while (Process32Next(hProcessSnap, &pe32));

	return 0;
}

json::value CWrdWizProcessExplorerDlg::GetStartUpProgram(SCITER_VALUE svGetStartUpProgram)
{
	m_svGetStartUpProgram = svGetStartUpProgram;
	HKEY hKey = 0;
	DWORD dwRet = 0;
	TCHAR tClassName[MAX_PATH] = TEXT("");
	DWORD dwClassSize = MAX_PATH;
	DWORD dwNoOfKeys = 0;
	DWORD dwMaxSubKey;
	DWORD dwMaxClass;
	DWORD dwNoOfValues;
	DWORD dwMaxValName;
	DWORD dwLongValData;
	DWORD dwSecurityDescriptor;
	FILETIME ftLastWriteTime;
	TCHAR tActualVal[16383];
	DWORD dwActualVal = 16383;
	BYTE *buffer;

	if (RegOpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\"), &hKey) == ERROR_SUCCESS)
	{
		dwRet = RegQueryInfoKey(hKey, tClassName, &dwClassSize, NULL, &dwNoOfKeys, &dwMaxSubKey, &dwMaxClass, &dwNoOfValues, &dwMaxValName, &dwLongValData, &dwSecurityDescriptor, &ftLastWriteTime);
		if (dwNoOfValues)
		{
			for (DWORD iIndex = 0; iIndex < dwNoOfValues; iIndex++)
			{
				dwActualVal = 16383;
				tActualVal[0] = '\0';
				dwRet = RegEnumValue(hKey, iIndex, tActualVal, &dwActualVal, NULL, NULL, NULL, NULL);
				if (dwRet == ERROR_SUCCESS)
				{
					buffer = new BYTE[dwLongValData];
					DWORD lpData = dwLongValData;
					RegQueryValueEx(hKey, tActualVal, 0, NULL, buffer, &lpData);
					CString csData = (LPCTSTR)buffer;
					m_svGetStartUpProgram.call((SCITER_STRING)csData, (SCITER_STRING)L"", L"");
				}
			}
		}
	}
	RegCloseKey(hKey);

	if (RegOpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run\\"), &hKey) == ERROR_SUCCESS)
	{
		dwRet = RegQueryInfoKey(hKey, tClassName, &dwClassSize, NULL, &dwNoOfKeys, &dwMaxSubKey, &dwMaxClass, &dwNoOfValues, &dwMaxValName, &dwLongValData, &dwSecurityDescriptor, &ftLastWriteTime);
		if (dwNoOfValues)
		{
			for (DWORD iIndex = 0; iIndex < dwNoOfValues; iIndex++)
			{
				dwActualVal = 16383;
				tActualVal[0] = '\0';
				dwRet = RegEnumValue(hKey, iIndex, tActualVal, &dwActualVal, NULL, NULL, NULL, NULL);
				if (dwRet == ERROR_SUCCESS)
				{
					buffer = new BYTE[dwLongValData];
					DWORD lpData = dwLongValData;
					RegQueryValueEx(hKey, tActualVal, 0, NULL, buffer, &lpData);
					CString csData = (LPCTSTR)buffer;
					m_svGetStartUpProgram.call((SCITER_STRING)csData, (SCITER_STRING)L"");
				}
			}
		}
	}
	RegCloseKey(hKey);
	return 0;
}

json::value CWrdWizProcessExplorerDlg::On_Terminate(SCITER_VALUE svProcess)
{
	CString csProcess = svProcess.to_string().c_str();
	DWORD dwRet = 0;
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;
	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
	{
		return 1;
	}

	// Calculate how many process identifiers were returned.
	cProcesses = cbNeeded / sizeof(DWORD);

	// Print the name and process identifier for each process.
	for (i = 0; i < cProcesses; i++)
	{
		if (aProcesses[i] != 0)
		{
			HANDLE		hProcess = NULL;
			HMODULE		hMods = NULL;
			DWORD		dwModules = 0x00;
			TCHAR szProcessName[MAX_PATH] = { 0 };

			hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_ALL_ACCESS, FALSE, aProcesses[i]);
			if (hProcess == NULL)
				continue;

			GetModuleBaseName(hProcess, hMods, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
			if (csProcess.Compare(szProcessName) == 0)
			{
				TerminateProcess(hProcess, 0);
				CloseHandle(hProcess);
				dwRet = 1;
				break;
			}
			CloseHandle(hProcess);
		}
	}
	if (dwRet == 1)
		return 1;
	return 0;
}

json::value CWrdWizProcessExplorerDlg::On_RemoveStartUp(SCITER_VALUE svProcess)
{
	DWORD dwRet = 0;
	HKEY hKey = 0;
	TCHAR tClassName[MAX_PATH] = TEXT("");
	DWORD dwClassSize = MAX_PATH;
	DWORD dwNoOfKeys = 0;
	DWORD dwMaxSubKey;
	DWORD dwMaxClass;
	DWORD dwNoOfValues;
	DWORD dwMaxValName;
	DWORD dwLongValData;
	DWORD dwSecurityDescriptor;
	FILETIME ftLastWriteTime;
	TCHAR tActualVal[16383];
	DWORD dwActualVal = 16383;
	BYTE *buffer;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\"), &hKey) == ERROR_SUCCESS)
	{
		dwRet = RegQueryInfoKey(hKey, tClassName, &dwClassSize, NULL, &dwNoOfKeys, &dwMaxSubKey, &dwMaxClass, &dwNoOfValues, &dwMaxValName, &dwLongValData, &dwSecurityDescriptor, &ftLastWriteTime);
		if (dwNoOfValues)
		{
			for (DWORD iIndex = 0; iIndex < dwNoOfValues; iIndex++)
			{
				dwActualVal = 16383;
				tActualVal[0] = '\0';
				dwRet = RegEnumValue(hKey, iIndex, tActualVal, &dwActualVal, NULL, NULL, NULL, NULL);
				if (dwRet == ERROR_SUCCESS)
				{
					buffer = new BYTE[dwLongValData];
					DWORD lpData = dwLongValData;
					RegQueryValueEx(hKey, tActualVal, 0, NULL, buffer, &lpData);
					CString csData = (LPCTSTR)buffer;
					if (csData.Compare(svProcess.to_string().c_str()) == 0)
					{
						if (RegDeleteKeyValue(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\"), tActualVal) == ERROR_SUCCESS)
						{
							dwRet = 1;
						}
						break;
					}
				}
			}
		}
	}
	RegCloseKey(hKey);

	if (dwRet != 1 && RegOpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run\\"), &hKey) == ERROR_SUCCESS)
	{
		dwRet = RegQueryInfoKey(hKey, tClassName, &dwClassSize, NULL, &dwNoOfKeys, &dwMaxSubKey, &dwMaxClass, &dwNoOfValues, &dwMaxValName, &dwLongValData, &dwSecurityDescriptor, &ftLastWriteTime);
		if (dwNoOfValues)
		{
			for (DWORD iIndex = 0; iIndex < dwNoOfValues; iIndex++)
			{
				dwActualVal = 16383;
				tActualVal[0] = '\0';
				dwRet = RegEnumValue(hKey, iIndex, tActualVal, &dwActualVal, NULL, NULL, NULL, NULL);
				if (dwRet == ERROR_SUCCESS)
				{
					buffer = new BYTE[dwLongValData];
					DWORD lpData = dwLongValData;
					RegQueryValueEx(hKey, tActualVal, 0, NULL, buffer, &lpData);
					CString csData = (LPCTSTR)buffer;
					if (csData.Compare(svProcess.to_string().c_str()) == 0)
					{
						if (RegDeleteKeyValue(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Run\\"), tActualVal) == ERROR_SUCCESS)
						{
							dwRet = 1;
							AfxMessageBox(tActualVal);
						}
						break;
					}
				}
			}
		}
	}
	RegCloseKey(hKey);
	if (dwRet == 1)
		return 1;
	return 0;
}

json::value CWrdWizProcessExplorerDlg::On_GetCurrFileVersion(SCITER_VALUE svPath, SCITER_VALUE svGetFileVersion)
{
	CString csPath = svPath.to_string().c_str();
	//HICON hIcon;
	//ICONINFOEX IconInfo;
	m_svGetFileVersion = svGetFileVersion;
	CVersionInfo objCVersionInfo;
	objCVersionInfo.GetVersionOfFile(csPath);
	m_svGetFileVersion.call((SCITER_STRING)objCVersionInfo.GetFileVersion(), (SCITER_STRING)objCVersionInfo.GetLegalInfo(), 
								(SCITER_STRING)objCVersionInfo.GetDescription(), (SCITER_STRING)objCVersionInfo.GetProductName());
	
	return 0;
}

json::value CWrdWizProcessExplorerDlg::onModalLoop(SCITER_VALUE svIsDialogOn, sciter::value svDialogBoolVal, sciter::value svDialogIntVal)
{
	try
	{
		if (svIsDialogOn.get(false)) {
			//theApp.g_lbRetval = sciter::value();
			theApp.m_bRetval = svDialogBoolVal.get(false);
			theApp.m_iRetval = svDialogIntVal;
			theApp.m_objCompleteEvent.SetEvent();
			//::WaitForSingleObject(theApp.g_lbCompleteEvent, INFINITE);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizAutoScnDlg::onModalLoop", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}