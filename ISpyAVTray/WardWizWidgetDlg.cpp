/************************************************************************************************************
*  Program Name		: WardWizWidgetDlg.cpp
*  Description		: CWardWizWidgetDlg Implementation
*  Author Name		: Amol Jaware
*  Date Of Creation : 08 Sep 2017
************************************************************************************************************/

// WardWizWidgetDlg.cpp : implementation file

#include "stdafx.h"
#include "WardWizWidgetDlg.h"
#include "afxdialogex.h"
#include "ISpyAVTray.h"

// CWardWizWidgetDlg dialog
DWORD WINAPI HideBackWindowThread(LPVOID lpvThreadParam);
DWORD WINAPI GetTotalTempFolderSize(LPVOID lpvThreadParamTempFileSize);
DWORD WINAPI GetProdUpdtInfo(LPVOID lpvThreadProdUpdtInfo);
DWORD WINAPI ShowCpuUsage(LPVOID lpvThreadCpuUsage);

IMPLEMENT_DYNAMIC(CWardWizWidgetDlg, CDialog)

/***************************************************************************************************
*  Function Name  : CWardWizWidgetDlg
*  Description    : C'tor
*  Author Name    : Amol Jaware
*  Date           : 08 Sep 2017
****************************************************************************************************/
CWardWizWidgetDlg::CWardWizWidgetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWardWizWidgetDlg::IDD, pParent)
	, m_dTotalFileSize(0)
	, dwStartBytes(0)
{
	m_hWaitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
}

/***************************************************************************************************
*  Function Name  : CWardWizWidgetDlg
*  Description    : D'tor
*  Author Name    : Amol Jaware
*  Date           : 08 Sep 2017
****************************************************************************************************/
CWardWizWidgetDlg::~CWardWizWidgetDlg()
{
	if (m_hWaitEvent != NULL)
	{
		::SetEvent(m_hWaitEvent);
		CloseHandle(m_hWaitEvent);
	}
}

/***************************************************************************************************
*  Function Name  : DoDataExchange
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Amol Jaware
*  Date           : 08 Sep 2017
****************************************************************************************************/
void CWardWizWidgetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CWardWizWidgetDlg, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

HWINDOW   CWardWizWidgetDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWardWizWidgetDlg::get_resource_instance() { return theApp.m_hInstance; }

// CWardWizWidgetDlg message handlers

/***************************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : Initializes the dialog window
*  Author Name    : Amol J.
*  Date			  : 08 Sep 2017
****************************************************************************************************/
BOOL CWardWizWidgetDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	CITinRegWrapper objReg;
	//Get here registry setting for WidgetsUIState
	DWORD dwWidgetsUIState = 0x01;
	if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwWidgetsUIState", dwWidgetsUIState) != 0x00)
	{
		AddLogEntry(L"### Failed to get Registry Entry for dwWidgetsUIState in CWardwizWidgetDlg::OnInitDialog KeyPath: %s", theApp.m_csRegKeyPath, 0, true, SECONDLEVEL);
	}

	if (dwWidgetsUIState == 0x00)
	{
		EndDialog(0);
		return FALSE;
	}

	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	BringWindowToTop();
	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0;
	UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 

	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_WIDGET.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_WIDGET.htm");
	this->SetWindowText(L"AKWIDGET");
	InitilizeZero();

	DWORD dwThreadId = 0x00;

	::CreateThread(NULL, 0, HideBackWindowThread, (LPVOID) this, 0, &dwThreadId);

	StartRequiredThreads();
	
	return 0;
}

/***************************************************************************************************
*  Function Name  :	InitilizeZero
*  Description    :	global variable initilize with 0.
*  Author Name    : Amol Jaware
*  Date           : 31 Oct 2017
****************************************************************************************************/
void CWardWizWidgetDlg::InitilizeZero()
{
	try
	{
		dwStartBytes = 0;
		m_dTotalFileSize = 0.0;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::InitilizeZero", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/***************************************************************************************************
*  Function Name  :	WindowProc
*  Description    :	This callback Procedure is used to Handle All Window Actions
*  Author Name    : Amol Jaware
*  Date           : 08 Sep 2017
****************************************************************************************************/
LRESULT CWardWizWidgetDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	BOOL    b_Handled = FALSE;
	__try
	{
		HWND hWnd = this->GetSafeHwnd();
		if (hWnd)
		{
			lResult = SciterProcND(hWnd, message, wParam, lParam, &b_Handled);

			if (b_Handled)      // if it was handled by the Sciter
				return lResult; // then no further processing is required.
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in CWardwizWidgetDlg::WindowProc", 0, 0, true, SECONDLEVEL);
	}
	return CDialog::WindowProc(message, wParam, lParam);
}

/***************************************************************************************************
*  Function Name  :	HideBackWindowThread
*  Description    :	To hide background window.
*  Author Name    : Amol Jaware
*  Date           :	08 Sep 2017
****************************************************************************************************/
DWORD WINAPI HideBackWindowThread(LPVOID lpvThreadParam)
{
	__try
	{
		if (!lpvThreadParam)
			return 0;

		CWardWizWidgetDlg *pThis = (CWardWizWidgetDlg*)lpvThreadParam;
		if (!pThis)
			return 0;

		//for testing purpose
		Sleep(10);

		::MoveWindow(pThis->get_hwnd(), 0, 0, 0, 0, true);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::HideBackWindowThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :	GetTotalTempFolderSize
*  Description    :	To get size of temp folder's.
*  Author Name    : Amol Jaware
*  Date           :	30 Oct 2017
****************************************************************************************************/
DWORD WINAPI GetTotalTempFolderSize(LPVOID lpvGetTempFileSizeThreadParam)
{
	try
	{
		CWardWizWidgetDlg *pThis = (CWardWizWidgetDlg *)lpvGetTempFileSizeThreadParam;
		if (!pThis)
			return 0;

		while (true)
		{
			pThis->dwStartBytes = 0;
			pThis->m_dTotalFileSize = 0.0;

			ZeroMemory(pThis->m_szSpecificPath, sizeof(pThis->m_szSpecificPath));
			GetEnvironmentVariable(L"TEMP", pThis->m_szSpecificPath, 511);  //C:\Users\AppData\Local\Temp

			if (pThis->m_szSpecificPath[0])
			{
				_wcsupr_s(pThis->m_szSpecificPath, 511);
				pThis->EnumFolder(pThis->m_szSpecificPath);
				
			}

			ZeroMemory(pThis->m_szSpecificPath, sizeof(pThis->m_szSpecificPath));
			GetWindowsDirectory(pThis->m_szSpecificPath, 511); //C:\Windows\Temp

			if (pThis->m_szSpecificPath[0])
			{
				wcscat_s(pThis->m_szSpecificPath, 511, L"\\Temp");
				_wcsupr_s(pThis->m_szSpecificPath, 511);
				pThis->EnumFolder(pThis->m_szSpecificPath);
			}

			pThis->m_svGetTempFoldSize.call(pThis->m_dTotalFileSize / 1024.0);
			
			::WaitForSingleObject(pThis->m_hWaitEvent,20 * 60 * 1000); //for 20 minutes
			::ResetEvent(pThis->m_hWaitEvent);
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::GetTotalTempFolderSize", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :	ShowCpuUsage
*  Description    :	Thread function which shows CPU Usage percentage.
*  Author Name    : Ram Shelke
*  Date           :	24 Nov 2017
****************************************************************************************************/
DWORD WINAPI ShowCpuUsage(LPVOID lpvThreadCpuUsage)
{
	__try
	{
		CWardWizWidgetDlg *pThis = (CWardWizWidgetDlg *)lpvThreadCpuUsage;
		if (!pThis)
			return 0;

		pThis->Show_CPU_Usage();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in ShowCpuUsage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :	GetProdUpdtInfo
*  Description    :	To get information whether product is upto date or not.
*  Author Name    : Amol Jaware
*  Date           :	13 Nov 2017
****************************************************************************************************/
DWORD WINAPI GetProdUpdtInfo(LPVOID lpvThreadProdUpdtInfo)
{
	try
	{
		CWardWizWidgetDlg *pThis = (CWardWizWidgetDlg *)lpvThreadProdUpdtInfo;
		if (!pThis)
			return 0;

		while (true)
		{
			TCHAR szvalue[1024];
			TCHAR szDwordValue[0x80] = { 0 };
			DWORD dwSize = sizeof(szDwordValue);
			CITinRegWrapper	objReg;

			if (objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"LastLiveupdatedt", szvalue, dwSize) == 0)
			{
				pThis->m_lpstrDate = (LPCTSTR)szvalue;
				pThis->m_csUpdateDate.Format(L"%s", pThis->m_lpstrDate);
				CString csCommandLine = (CString)pThis->m_lpstrDate;

				int iPos = 0;
				int szTemp[3] = { 0 };
				for (int i = 0; i < 3; i++)
				{
					CString csTemp = csCommandLine.Tokenize(_T("/"), iPos);
					szTemp[i] = _wtoi(csTemp);
				}
				int iYear = szTemp[2];
				int iDay = szTemp[1];
				int iMonth = szTemp[0];

				CTime Time_Curr = CTime::GetCurrentTime();
				int iMonth1 = Time_Curr.GetMonth();
				int iDate1 = Time_Curr.GetDay();
				int iYear1 = Time_Curr.GetYear();

				if (!pThis->isValidDate(iDay, iMonth, iYear) || !pThis->isValidDate(iDate1, iMonth1, iYear1))
				{
					AddLogEntry(L"### Invalid LastLiveupdatedt/GetCurrentTime in GetProdUpdtInfo().", 0, 0, true, SECONDLEVEL);

				}
				else
				{
					CTime Time_RegistryDate(iYear, iMonth, iDay, 0, 0, 0);
					CTime Time_CurDate(iYear1, iMonth1, iDate1, 0, 0, 0);
					CTimeSpan Time_Diff = Time_CurDate - Time_RegistryDate;
					int Span = static_cast<int>(Time_Diff.GetDays());
					if (Span > 7)
					{
						pThis->m_svGetProdUpdtInfo.call(false); //older db
					}
					else
					{
						pThis->m_svGetProdUpdtInfo.call(true); //upto date
					}
				}
			}
			else
			{
				AddLogEntry(L"### Failed to get Registry Entry for LastLiveupdatedt in GetProdUpdtInfo", 0, 0, true, SECONDLEVEL);
			}

			::WaitForSingleObject(pThis->m_hWaitEvent, 720 * 60 * 1000); //for 12 hours
			::ResetEvent(pThis->m_hWaitEvent);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in GetProdUpdtInfo()", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :	On_ClickUpdate
*  Description    :	Calling OnBnClickedButtonDownloadNow this function to invoke update functionality.
*  Author Name    : Amol Jaware
*  Date           : 15 Sep 2017 
****************************************************************************************************/
json::value CWardWizWidgetDlg::On_ClickUpdate()
{
	try
	{
		OnBnClickedButtonDownloadNow();
	}

	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizWidgetDlg::On_ClickUpdate", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :	On_ClickTempFileCleaner
*  Description    :	Invoking Temp File Cleaner page.
*  Author Name    : Amol Jaware 
*  Date           : 15 Sep 2017
****************************************************************************************************/
json::value CWardWizWidgetDlg::On_ClickTempFileCleaner()
{
	try
	{
		CString csUIFilePath;
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (CheckIsAppRunning(L"Vibranium TEMP CLEANER"))
		{
			return 0;
		}
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			AddLogEntry(L"### Excpetion for GetModulePath() in CWardwizUIDlg::On_ClickTempFileCleaner", 0, 0, true, SECONDLEVEL);
			return 0;
		}
		csUIFilePath = szModulePath;
		csUIFilePath += L"\\VBTEMPCLR.EXE";

		ShellExecute(NULL, L"open", csUIFilePath, L"-TEMPFILECLEANER", NULL, SW_SHOW);
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizWidgetDlg::On_ClickTempFileCleaner", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :	On_BnClickedButtonScanPage
*  Description    :	Calling OnBnClickedButtonScanNow to invoke UI Scan page.
*  Author Name    : Amol Jaware
*  Date           :	26 Sep 2017
****************************************************************************************************/
json::value CWardWizWidgetDlg::On_BnClickedButtonScanPage()
{
	try
	{
		OnBnClickedButtonScanNow();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizWidgetDlg::On_BnClickedButtonScanPage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :	On_ClickReport
*  Description    :	Calling On_ClickReport to invoke UI Report page.
*  Author Name    : Amol Jaware
*  Date           :	26 Sep 2017
****************************************************************************************************/
json::value CWardWizWidgetDlg::On_ClickReport()
{
	try
	{
		OnBnClickedButtonReports();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizWidgetDlg::On_ClickReport", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :	OnBnClickedButtonDownloadNow
*  Description    : Invoking the live update.
*  Author Name    : Amol Jaware
*  Date           :	20 Sep 2017
****************************************************************************************************/
void CWardWizWidgetDlg::OnBnClickedButtonDownloadNow()
{
	try
	{
		HWND hWindow = ::FindWindow(NULL, L"VBUPDATE");
		if (hWindow)
		{
			::ShowWindow(hWindow, SW_RESTORE);
			::BringWindowToTop(hWindow);
			::SetForegroundWindow(hWindow);
			SendData2UI(CHECKLIVEUPDATETRAY, false);
		}
		else// If UI is not open then it will make shellexecute() call
		{
			CString csUIFilePath;
			TCHAR szModulePath[MAX_PATH] = { 0 };
			if (!GetModulePath(szModulePath, MAX_PATH))
			{

			}
			csUIFilePath = szModulePath;
			csUIFilePath += L"\\VBUPDATE.EXE";

			ShellExecute(NULL, L"open", csUIFilePath, L"-LIVEUPDATE", NULL, SW_SHOW);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizWidgetDlg::OnBnClickedButtonDownloadNow", 0, 0, true, SECONDLEVEL);
	}
}
/***************************************************************************************************
*  Function Name  :	SendData2UI
*  Description    : Send data to UI.
*  Author Name    : Amol Jaware
*  Date           :	20 Sep 2017
****************************************************************************************************/
bool CWardWizWidgetDlg::SendData2UI(int iMessageInfo, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = iMessageInfo;

		CISpyCommunicator objComUI(UI_SERVER, true);
		if (!objComUI.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CWardwizWidgetDlg::SendData2UI", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizWidgetDlg::SendData2UI", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :	OnBnClickedButtonScanNow
*  Description    : function which lauch UI and show scan page, if already open then shows scan page only.
*  Author Name    : Amol Jaware
*  Date           :	26 Sep 2017
****************************************************************************************************/
void CWardWizWidgetDlg::OnBnClickedButtonScanNow()
{
	try
	{
		HWND hWindow = ::FindWindow(NULL, L"VIBRANIUMUI");
		if (hWindow)
		{
			::ShowWindow(hWindow, SW_RESTORE);
			::BringWindowToTop(hWindow);
			::SetForegroundWindow(hWindow);
			SendData2UI(SHOW_SPECUI_PAGE, SCANPAGE, false);
		}
		else// If UI is not open then it will make shellexecute() call
		{
			CString csUIFilePath;
			TCHAR szModulePath[MAX_PATH] = { 0 };
			if (!GetModulePath(szModulePath, MAX_PATH))
			{

			}
			csUIFilePath = szModulePath;
			csUIFilePath += L"\\VBUI.EXE";
			ShellExecute(NULL, L"open", csUIFilePath, L"-SHOWSCANPAGE", NULL, SW_SHOW);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizWidgetDlg::OnBnClickedButtonScanNow", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :	OnBnClickedButtonReports
*  Description    : function which lauch UI and show report page, if already open then shows report page only.
*  Author Name    : Amol Jaware
*  Date           :	26 Sep 2017
****************************************************************************************************/
void CWardWizWidgetDlg::OnBnClickedButtonReports()
{
	try
	{
		HWND hWindow = ::FindWindow(NULL, L"VIBRANIUMUI");
		if (hWindow)
		{
			::ShowWindow(hWindow, SW_RESTORE);
			::BringWindowToTop(hWindow);
			::SetForegroundWindow(hWindow);
			SendData2UI(SHOW_SPECUI_PAGE, REPORT, false);
		}
		else// If UI is not open then it will make shellexecute() call
		{
			CString csUIFilePath;
			TCHAR szModulePath[MAX_PATH] = { 0 };
			if (!GetModulePath(szModulePath, MAX_PATH))
			{

			}
			csUIFilePath = szModulePath;
			csUIFilePath += L"\\VBUI.EXE";
			ShellExecute(NULL, L"open", csUIFilePath, L"-SHOWREPORTS", NULL, SW_SHOW);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizWidgetDlg::OnBnClickedButtonReports", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :	SendData2UI
*  Description    : Send data to UI.
*  Author Name    : Amol Jaware
*  Date           :	26 Sep 2017
****************************************************************************************************/
bool CWardWizWidgetDlg::SendData2UI(int iMessageInfo, int iParamFirst, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = iMessageInfo;
		szPipeData.dwValue = iParamFirst;

		CISpyCommunicator objComUI(UI_SERVER, true);
		if (!objComUI.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send Data in CWardwizWidgetDlg::SendData2UI", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizWidgetDlg::SendData2UI", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :	OnClose
*  Description    : commented close call because don't want to close widget.
*  Author Name    : Amol Jaware
*  Date           :	14 Sep 2017
****************************************************************************************************/
void CWardWizWidgetDlg::OnClose()
{
}

/****************************************************************************************************
*  Function Name	: On_funUnRegProd
*  Description		: Checking how many days left.	
*  Author Name		: Amol J.
*  Date				: 16/10/2017
****************************************************************************************************/
json::value CWardWizWidgetDlg::On_funUnRegProd(SCITER_VALUE svDays)
{
	try
	{ 
		m_svDays = svDays;
		if (theApp.m_dwDaysLeft >= 1)
		{
			m_svDays.call(1);
		}
		else
		{
			m_svDays.call(0);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::On_funUnRegProd", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/****************************************************************************************************
*  Function Name	: GetDaysLeftOnHomeBtn
*  Description		: Registration widget state reflecting on home button.
*  Author Name		: Amol J.
*  Date				: 16/01/2018
****************************************************************************************************/
void CWardWizWidgetDlg::GetDaysLeftOnHomeBtn()
{
	try
	{
		DWORD dwDaysLeft = theApp.GetDaysLeft();

		if (dwDaysLeft >= 1)
		{
			m_svDays.call(1);
		}
		else
		{
			m_svDays.call(0);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::GetDaysLeftOnHomeBtn", 0, 0, true, SECONDLEVEL);
	}
}

/****************************************************************************************************
*  Function Name	: GetDaysRegLeft
*  Description		: Getting how many days left to check registration widget state.
*  Author Name		: Amol J.
*  Date				: 30/10/2017
****************************************************************************************************/
void CWardWizWidgetDlg::GetDaysRegLeft(int iRetDays)
{
	try
	{
		if (iRetDays > 0)
			m_svDays.call(iRetDays);
		else
			m_svDays.call(iRetDays);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::GetDaysRegLeft", 0, 0, true, SECONDLEVEL);
	}

	return;
}

/****************************************************************************************************
*  Function Name	: On_funActiveScan
*  Description		: Checking real time protection is ON or OFF.
*  Author Name		: Amol J.
*  Date				: 16/10/2017
****************************************************************************************************/
json::value CWardWizWidgetDlg::On_funActiveScan(SCITER_VALUE svSetWidgetACState, SCITER_VALUE svGetTempFoldSize)
{
	try
	{
		m_svSetWidgetACState = svSetWidgetACState;
		m_svGetTempFoldSize = svGetTempFoldSize;
		CITinRegWrapper objReg;
		DWORD dwData = 0x00;
		CString csProdRegKey = CWWizSettingsWrapper::GetProductRegistryKey();
		
		DWORD dwRet = objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csProdRegKey.GetBuffer(), L"dwActiveScanOption", dwData);
		if (dwData == 1)
		{
			m_svSetWidgetACState.call((SCITER_STRING)L"GREENFLAG");
		}
		else
		{
			m_svSetWidgetACState.call((SCITER_STRING)L"REDFLAG");
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::On_funActiveScn", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/****************************************************************************************************
*  Function Name	: WidgetActiveScanColorState
*  Description		: calling function from CISpyAVTrayDlg for updating color state.
*  Author Name		: Amol J.
*  Date				: 23/10/2017
****************************************************************************************************/
void CWardWizWidgetDlg::WidgetActiveScanColorState(DWORD dwFlagState)
{
	try
	{
		CITinRegWrapper objReg;
		DWORD dwData = 0x00;
		CString csProdRegKey;
		DWORD dwRet = objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwActiveScanOption", dwData);
		
		if (dwFlagState == REDFLAG)
		{
			m_svSetWidgetACState.call(L"REDFLAG");
		}
		else if (dwFlagState == GREENFLAG && dwData == 1)
		{
			m_svSetWidgetACState.call((SCITER_STRING)L"GREENFLAG");
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::WidgetActiveScanColorState", 0, 0,true, SECONDLEVEL);
	}
	return;
}

/****************************************************************************************************
*  Function Name	: On_funUptoDateProd
*  Description		: Checking product is upto date or not.
*  Author Name		:
*  Date				: 17/10/2017
****************************************************************************************************/
json::value CWardWizWidgetDlg::On_funUptoDateProd(SCITER_VALUE svGetProdUpdtInfo)
{
	try
	{
		m_svGetProdUpdtInfo = svGetProdUpdtInfo;
		TCHAR	szvalue[1024];
		HKEY key;
		DWORD dwtype = REG_SZ;
		DWORD dwvalue_length = 1024;

		if (RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Vibranium"), &key) != ERROR_SUCCESS)
		{
			return 0;
		}

		long ReadReg = RegQueryValueEx(key, L"LastLiveupdatedt", NULL, &dwtype, (LPBYTE)&szvalue, &dwvalue_length);
		
		if (ReadReg == ERROR_SUCCESS)
		{

			m_lpstrDate = (LPCTSTR)szvalue;

			CString csCommandLine = (CString)m_lpstrDate;
			int iPos = 0;
			int szTemp[3] = { 0 };
			for (int i = 0; i < 3; i++)
			{
				CString csTemp = csCommandLine.Tokenize(_T("/"), iPos);
				szTemp[i] = _wtoi(csTemp);
			}

			int iYear = szTemp[2];
			int iDay = szTemp[1];
			int iMonth = szTemp[0];

			CTime Time_Curr = CTime::GetCurrentTime();
			int iMonth1 = Time_Curr.GetMonth();
			int iDate1 = Time_Curr.GetDay();
			int iYear1 = Time_Curr.GetYear();

			CTime Time_RegistryDate(iYear, iMonth, iDay, 0, 0, 0);
			CTime Time_CurDate(iYear1, iMonth1, iDate1, 0, 0, 0);
			CTimeSpan Time_Diff = Time_CurDate - Time_RegistryDate;
			int Span = static_cast<int>(Time_Diff.GetDays());
			if (Span > 7)
			{
				m_svGetProdUpdtInfo.call(false);
			}
			else
			{
				m_svGetProdUpdtInfo.call(true);
			}
		}
		RegCloseKey(key);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::On_funUptoDateProd", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : EnumFolder
*  Description    : function to get file count during scan
*  Author Name    : Amol Jaware
*  Date           : 31 Oct 2017
****************************************************************************************************/
void CWardWizWidgetDlg::EnumFolder(LPCTSTR lpFolPath)
{
	try
	{
		if (!lpFolPath)
			return;

		HANDLE m_hFile = INVALID_HANDLE_VALUE;
		CFileFind finder;
		CString strWildcard(lpFolPath);
		if (strWildcard.IsEmpty())
			return;

		if (strWildcard[strWildcard.GetLength() - 1] != '\\')
			strWildcard += _T("\\*.*");
		else
			strWildcard += _T("*.*");

		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			TCHAR	szPath[1024] = { 0 };
			wsprintf(szPath, L"%s", finder.GetFilePath());

			if (finder.IsDirectory())
			{
				EnumFolder(szPath);
			}
			else
			{
				m_hFile = CreateFile(szPath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE || FILE_SHARE_DELETE,
					NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
				if (m_hFile != INVALID_HANDLE_VALUE)
				{
					dwStartBytes = GetFileSize(m_hFile, NULL);
					m_dTotalFileSize += dwStartBytes / 1024.0;
				}

				if (m_hFile != INVALID_HANDLE_VALUE)
				{
					CloseHandle(m_hFile);
					m_hFile = NULL;
				}
			}
			dwStartBytes = 0;
		}
		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::EnumFolder", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CallGetTotalTempFolderSize
*  Description    : function to get call CallGetTotalTempFolderSize thread.
*  Author Name    : Amol Jaware
*  Date           : 09 Nov 2017
****************************************************************************************************/
void CWardWizWidgetDlg::CallGetTotalTempFolderSize()
{
	try
	{
		DWORD dwThreadTempFileSizeId = 0X00;
		::CreateThread(NULL, 0, GetTotalTempFolderSize, (LPVOID) this, 0, &dwThreadTempFileSizeId);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::CallGetTotalTempFolderSize", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CloseUI
*  Description    : function to which close widgets UI.
*  Author Name    : Amol Jaware
*  Date           : 09 Nov 2017
****************************************************************************************************/
void CWardWizWidgetDlg::CloseUI()
{
	__try
	{
		EndDialog(0);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in CWardwizWidgetDlg::CloseUI", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
* Function Name      : IsValidDate
* Description        : If user changed the date of 'LastLiveUpdatdt' from registry to invalid date, then error pop-up gets displayed.
					 : Function checks whether the LastLiveUpdatedt is valid or not & returns true if date is valid, else returns false.
* Author Name		 : Amol J.
* Date Of Creation   : 13 Nov 2017
***********************************************************************************************************/
bool CWardWizWidgetDlg::isValidDate(int iDay, int iMonth, int iYear)
{
	try
	{
		if (iYear < 1970 || iYear > 3000 || iMonth < 1 || iMonth > 12 || iDay < 1)
		{
			return false;
		}
		if (iMonth == 2)
		{
			int Leapyear;
			Leapyear = iYear % 400 == 0 || (iYear % 4 == 0 && iYear % 100 != 0);
			if (Leapyear)
			{
				if (iDay > 29)
				{
					return false;
				}
			}
			else
			{
				if (iDay > 28)
				{
					return false;
				}
			}
		}
		else if (iMonth == 1 || iMonth == 3 || iMonth == 5 || iMonth == 7 || iMonth == 8 || iMonth == 10 || iMonth == 12)
		{
			if (iDay > 31)
			{
				return false;
			}
		}
		else if (iMonth == 4 || iMonth == 6 || iMonth == 9 || iMonth == 11)
		{
			if (iDay > 30)
			{
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::isValidDate", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : CallGetProdUpdtInfo
*  Description    : function to get call CallGetProdUpdtInfo thread.
*  Author Name    : Amol Jaware
*  Date           : 14 Nov 2017
****************************************************************************************************/
void CWardWizWidgetDlg::CallGetProdUpdtInfo()
{
	try
	{
		DWORD dwThreadProdUpdtInfo = 0X00;
		::CreateThread(NULL, 0, GetProdUpdtInfo, (LPVOID) this, 0, &dwThreadProdUpdtInfo);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::CallGetProdUpdtInfo", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CallGetProdUpdtInfo
*  Description    : function to get language id.
*  Author Name    : Amol Jaware
*  Date           : 23 Nov 2017
****************************************************************************************************/
json::value CWardWizWidgetDlg::GetLanguageID()
{
	try
	{
		int iLangID = theApp.m_objwardwizLangManager.GetSelectedLanguage();
		return iLangID;
	}
	catch (...)
	{
		AddLogEntry(L"### exception in CWardwizWidgetDlg::GetLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : GetCpuUsage
*  Description    : function to get CPU usage.
*  Author Name    : 
*  Date           : 24 Nov 2017
****************************************************************************************************/
json::value CWardWizWidgetDlg::GetCpuUsage(SCITER_VALUE svGetCpuUsageCount)
{
	m_svGetCpuUsageCount = svGetCpuUsageCount;
	return 0;
}

/***************************************************************************************************
*  Function Name  :	Show_CPU_Usage
*  Description    : Function which shows CPU usage to UI
*  Author Name    : Ram Shelke
*  Date           :	24 Nov 2017
****************************************************************************************************/
void CWardWizWidgetDlg::Show_CPU_Usage()
{
	try
	{
		while (true)
		{
			Sleep(2 * 1000); //2 seconds

			float cpuUsage = m_cpuUSage.GetCPULoad();
			CString csCPUUsage;
			csCPUUsage.Format(L"%02d", (int)(cpuUsage * 100));
			csCPUUsage += L"%";
			sciter::value scValue(csCPUUsage.GetBuffer());
			m_svGetCpuUsageCount.call(scValue);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizWidgetDlg::Show_CPU_Usage", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :	StartRequiredThreads
*  Description    :	Function which created specific threads and handles exception handling.
*  Author Name    : Ram Shelke
*  Date           : 08 Sep 2017
****************************************************************************************************/
void CWardWizWidgetDlg::StartRequiredThreads()
{
	__try
	{
		//This sleep is required as to send status after UI get initialized.
		Sleep(10 * 1000);

		DWORD dwThreadTempFileSizeId = 0X00;
		DWORD dwThreadProdUpdtInfo = 0X00;
		DWORD dwThreadCPUUsage = 0X00;

		::CreateThread(NULL, 0, GetTotalTempFolderSize, (LPVOID) this, 0, &dwThreadTempFileSizeId);
		::CreateThread(NULL, 0, GetProdUpdtInfo, (LPVOID) this, 0, &dwThreadProdUpdtInfo);
		::CreateThread(NULL, 0, ShowCpuUsage, (LPVOID) this, 0, &dwThreadCPUUsage);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in CWardwizWidgetDlg::StartRequiredThreads", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :	On_GetProductID
*  Description    :	Function to get product id
*  Author Name    : Jeena Mariam Saji
*  Date           : 22 May 2018
****************************************************************************************************/
json::value CWardWizWidgetDlg::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iProdValue;
}

/***************************************************************************************************
Function Name  : On_GetThemeID
Description    : To get theme ID
Author Name    : Jeena Mariam Saji
Date           : 30th Dec 2018
/***************************************************************************************************/
json::value CWardWizWidgetDlg::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_HideWidget
Description    : To hide widget
Author Name    : Akshay Patil
Date           : 12 July 2019
/***************************************************************************************************/
json::value CWardWizWidgetDlg::On_HideWidget()
{
	try
	{
		EndDialog(0);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::On_HideWidget", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
Function Name  : On_DisableWidget
Description    : To disble widget & set registry value
Author Name    : Akshay Patil
Date           : 12 July 2019
/***************************************************************************************************/
json::value CWardWizWidgetDlg::On_DisableWidget()
{
	try
	{
		DWORD dwWidgetsUIState = 0x00;

		if (!SetRegistrykeyUsingService(theApp.m_csRegKeyPath, L"dwWidgetsUIState", REG_DWORD, dwWidgetsUIState))
		{
			AddLogEntry(L"### Error in Setting Registry CWardwizWidgetDlg::On_DisableWidget", 0, 0, true, SECONDLEVEL);
			return 0;
		}

		EndDialog(0);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::On_DisableWidget", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
Function Name  : SetRegistrykeyUsingService
Description    : Set registry key using service through pipe.
Author Name    : Akshay Patil
Date           : 12 July 2019
/***************************************************************************************************/
bool CWardWizWidgetDlg::SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));

		szPipeData.iMessageInfo = WRITE_REGISTRY;
		szPipeData.dwValue = SZ_DWORD;

		wcscpy_s(szPipeData.szFirstParam, SubKey);
		wcscpy_s(szPipeData.szSecondParam, lpValueName);
		szPipeData.dwSecondValue = dwData;

		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CWardwizWidgetDlg : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CWardwizWidgetDlg : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizWidgetDlg::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
	}
	
	return true;
}

/***************************************************************************************************
*  Function Name  :	CheckAppIsRunning
*  Description    :	Check Application is running or not
*  Author Name    : Kunal Waghmare
*  Date           : 22 Aug 2019
****************************************************************************************************/
bool CWardWizWidgetDlg::CheckIsAppRunning(LPTSTR lpStr)
{
	try{
		if (lpStr == NULL)
			return false;
		HWND hWindow = ::FindWindow(NULL, lpStr);
		if (hWindow)
		{
			::ShowWindow(hWindow, SW_RESTORE);
			::BringWindowToTop(hWindow);
			::SetForegroundWindow(hWindow);
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizWidgetDlg::CheckIsAppRunning", 0, 0, true, SECONDLEVEL);
	}
	return false;
}