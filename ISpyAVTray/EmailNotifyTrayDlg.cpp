/**********************************************************************************************************
Program Name          : EmailNotifyTrayDlg.cpp
Description           : Tray notification for Email scanning attachment files.
Author Name			  : Amol Jaware.
Date Of Creation      : 27 June 2018
Version No            : 
***********************************************************************************************************/
#include "stdafx.h"
#include "EmailNotifyTrayDlg.h"
#include "afxdialogex.h"
#include "ISpyAVTray.h"

#define SETVIRUSCOUNT_EVENT_CODE (FIRST_APPLICATION_EVENT_CODE + 5)

CEmailNotifyTrayDlg::CEmailNotifyTrayDlg(CWnd* pParent /*=NULL*/)
: CDialog(CEmailNotifyTrayDlg::IDD, pParent)
, m_bFirstEntry(true)
{
	m_iCurrentIndex = 0;
	bIsNextPrevClicked = false;
}

CEmailNotifyTrayDlg::~CEmailNotifyTrayDlg()
{
}

/***************************************************************************************************
*  Function Name  : DoDataExchange
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Amol Jaware
*  Date           : 27 June 2018
****************************************************************************************************/
void CEmailNotifyTrayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEmailNotifyTrayDlg, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

HWINDOW   CEmailNotifyTrayDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CEmailNotifyTrayDlg::get_resource_instance() { return theApp.m_hInstance; }

/***************************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : Initializes the dialog window
*  Author Name    : Amol J.
*  Date			  : 27 June 2018
****************************************************************************************************/
BOOL CEmailNotifyTrayDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	SetWindowPos(&this->wndTopMost, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);

	//Required Members
	int cxIcon = 0;
	int cyIcon = 0;
	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	int i = 0;
	int j = 0;
	int ixRect = 0;
	int iyRect = 0;
	LPCBYTE pb = 0;
	UINT cb = 0;

	//draw
	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 

	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_TRAY_POPUP_THREAT_DETECT.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_TRAY_POPUP_THREAT_DETECT.htm");

	m_root_el = root();
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);
	::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);

	cxIcon = GetSystemMetrics(SM_CXFULLSCREEN);
	cyIcon = GetSystemMetrics(SM_CYFULLSCREEN);
	// Dialog is visible properly
	//24/6/2014 Neha/Nitin
	i = GetTaskBarHeight();
	j = GetTaskBarWidth();
	ixRect = cxIcon - pIntMaxWidth;
	iyRect = cyIcon - pIntHeight;
	this->ShowWindow(SW_HIDE);
//	HideAllElements();

	try
	{
		APPBARDATA abd;
		abd.cbSize = sizeof(abd);

		SHAppBarMessage(ABM_GETTASKBARPOS, &abd);

		switch (abd.uEdge)
		{
		case ABE_TOP:
			SetWindowPos(NULL, ixRect, i, pIntMinWidth, pIntHeight, SWP_NOREDRAW);
			break;

		case ABE_BOTTOM:
			SetWindowPos(NULL, ixRect, iyRect + 200, pIntMinWidth, pIntHeight, SWP_NOREDRAW);
			break;

		case ABE_LEFT:
			SetWindowPos(NULL, j, iyRect, pIntMinWidth, pIntHeight, SWP_NOREDRAW);
			break;

		case ABE_RIGHT:
			SetWindowPos(NULL, ixRect, iyRect, pIntMinWidth, pIntHeight, SWP_NOREDRAW);
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CEmailNotifyTrayDlg::OnInitDialog", 0, 0, true, SECONDLEVEL);
	}

	return TRUE;
}

/***************************************************************************************************
*  Function Name  :	WindowProc
*  Description    :	This callback Procedure is used to Handle All Window Actions
*  Author Name    : Amol Jaware
*  Date           : 27 June 2018
****************************************************************************************************/
LRESULT CEmailNotifyTrayDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
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
		AddLogEntry(L"### Excpetion in CEmailNotifyTrayDlg::WindowProc", 0, 0, true, SECONDLEVEL);
	}
	return CDialog::WindowProc(message, wParam, lParam);
}

/***************************************************************************
Function Name  : GetTaskBarHeight
Description    : To get the Task Bar Height
Author Name    : Amol Jaware
Date           : 27 June 2018
****************************************************************************/
int CEmailNotifyTrayDlg::GetTaskBarHeight()
{
	RECT rect;
	HWND taskBar;
	taskBar = ::FindWindow(L"Shell_TrayWnd", NULL);
	if (taskBar && ::GetWindowRect(taskBar, &rect))
	{
		return rect.bottom - rect.top;
	}
	return 0;
}

/***************************************************************************
Function Name  : GetTaskBarWidth
Description    : To get the Task Bar Width
Author Name    : Amol Jaware.
Date           : 27 June 2018
****************************************************************************/
int CEmailNotifyTrayDlg::GetTaskBarWidth()
{
	RECT rect;
	HWND taskBar;
	taskBar = ::FindWindow(L"Shell_TrayWnd", NULL);
	if (taskBar && ::GetWindowRect(taskBar, &rect))
	{
		return rect.right - rect.left;
	}
	return 0;
}

/***************************************************************************
Function Name  : OnBTNCilckClose
Description    : To close the tray window.
Author Name    : Amol Jaware.
Date           : 28 June 2018
****************************************************************************/
json::value CEmailNotifyTrayDlg::OnBTNCilckClose()
{
	try
	{
		m_vecThreatList.clear();
		m_iCurrentIndex = 0;
		m_bFirstEntry = true;
		bIsNextPrevClicked = false;
		m_bIsCloseClicked = true;
		OnCancel();
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CEmailNotifyTrayDlg::OnBTNCilckClose", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :	AddIntoQueueEmailScanInfo
*  Description    :	This Function is used to put Quarantine Info.
*  Author Name    :	Amol Jaware
*  Date           :	27 June 2018
****************************************************************************************************/
bool CEmailNotifyTrayDlg::AddIntoQueueEmailScanInfo(CString csThreatName, CString scAttachmentName, CString scSenderAddr, DWORD dwActionTaken)
{
	try
	{
		//This check is to avoid Sleep for each time.
		if (m_bFirstEntry)
		{
			Sleep(100);
			m_bFirstEntry = false;
		}

		DWORD dwDetectThreadID = 0x00;
		THREATINFO4EmailScan szThreatInfo = { 0 };
		CString csSize;
		CString csName;
		CString csPath;
		CString csSenderName;
		int iThreatAction;
		
		_tcscpy_s(szThreatInfo.szThreatName, csThreatName);
		_tcscpy_s(szThreatInfo.szFilePath, scAttachmentName);
		_tcscpy_s(szThreatInfo.szSenderAddr, scSenderAddr);

		switch (dwActionTaken)
		{
		case FILESKIPPED:
			szThreatInfo.dwActionTaken = 0;
			break;
		case FILEREPAIRED:
			szThreatInfo.dwActionTaken = 1;
			break;
		case FILEQURENTINED:
			szThreatInfo.dwActionTaken = 2;
			break;
		}

		m_vecThreatList.push_back(szThreatInfo);
		m_iSize = static_cast<int>(m_vecThreatList.size());
		csSize.Format(L"Vector Size: %d", m_iSize);

		if (bIsNextPrevClicked == false)
		{
			m_iCurrentIndex = m_iSize - 1;
		}

		csName = m_vecThreatList.at(m_iCurrentIndex).szThreatName;
		csPath = m_vecThreatList.at(m_iCurrentIndex).szFilePath;
		csSenderName = m_vecThreatList.at(m_iCurrentIndex).szSenderAddr;
		iThreatAction = static_cast<int>(m_vecThreatList.at(m_iCurrentIndex).dwActionTaken);

		TCHAR					m_szActiveScanFilePath[1024];
		memset(m_szActiveScanFilePath, 0, sizeof(m_szActiveScanFilePath));
		int iCount = csPath.ReverseFind('\\');
		CString csFileName = csPath.Right(csPath.GetLength() - (iCount + 1));
		CString csFolderPath;
		csFolderPath = csPath.Left(iCount);

		GetShortPathName(csFolderPath, m_szActiveScanFilePath, 45);
		CString csTempFileName = csFileName;
		iCount = csTempFileName.ReverseFind('.');
		CString csFileExt = csTempFileName.Right(csTempFileName.GetLength() - (iCount));
		csTempFileName = csTempFileName.Left(iCount);
		if (csTempFileName.GetLength() > 10)
		{
			csTempFileName = csTempFileName.Left(10);
			csFileName.Format(L"%s~%s", csTempFileName, csFileExt);
		}
		CString csFinalFilePath;
		csFinalFilePath.Format(L"%s\\%s", m_szActiveScanFilePath, csFileName);
		AddQuarantineInfo(csName, csFinalFilePath, csSenderName, iThreatAction, m_iSize);
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CEmailNotifyTrayDlg::AddIntoQueueEmailScanInfo, FilePath: [%s]", scAttachmentName, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :	AddQuarantineInfo
*  Description    :	This Function is used to send Quarantine Info. for Automatic Call.
*  Author Name    :	Amol Jaware
*  Date           :	27 July 2018
****************************************************************************************************/
void CEmailNotifyTrayDlg::AddQuarantineInfo(CString csName, CString csPath, CString csSenderName, int iThreatAction, int iSize)
{
	try
	{
		if (m_bIsCloseClicked == true)
		{
			return;
		}

		sciter::value map;
		map.set_item("one", sciter::string(csName));
		map.set_item("two", sciter::string(csPath));
		map.set_item("three", sciter::string(csSenderName));
		map.set_item("four", sciter::value(iThreatAction));
		map.set_item("five", sciter::value(iSize));
		m_root_el = root();
		sciter::dom::element ela = m_root_el;
		BEHAVIOR_EVENT_PARAMS beParams;

		beParams.cmd = SETVIRUSCOUNT_EVENT_CODE;
		beParams.he = beParams.heTarget = ela;
		beParams.data = map;
		ela.fire_event(beParams, true);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CEmailNotifyTrayDlg::AddQuarantineInfo", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  :	GetQuarantineInfo
*  Description    :	This Sciter Function is used to get Quarantine Info. Callback.
*  Author Name    :	Amol Jaware
*  Date           :	28 June 2018
****************************************************************************************************/
json::value CEmailNotifyTrayDlg::GetQuarantineInfo()
{
	try
	{
		DWORD dwDays = theApp.m_dwDaysLeft;
		if (dwDays == 0)
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CEmailNotifyTrayDlg::GetQuarantineInfo", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
Function Name  : On_GetThemeID
Description    : To get theme ID
Author Name    : Akshay Patil
Date           : 11 Sept 2019
/***************************************************************************************************/
json::value CEmailNotifyTrayDlg::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CEmailNotifyTrayDlg::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_GetLanguageID
Description    : To Get App Path and Set it in Script
Author Name    : Akshay Patil
Date           : 11 Sept 2019
/***************************************************************************************************/
json::value CEmailNotifyTrayDlg::On_GetLanguageID()
{
	int iLangValue = 0;
	try
	{
		iLangValue = theApp.m_objwardwizLangManager.GetSelectedLanguage();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CEmailNotifyTrayDlg::On_GetLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : for Get the App Path and Set it in Script
Author Name    : Akshay Patil
Date           : 11 Sept 2019
/***************************************************************************************************/
json::value CEmailNotifyTrayDlg::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizUSBScnPopup::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Akshay Patil
*  Date           : 11 Sept 2019
****************************************************************************************************/
json::value CEmailNotifyTrayDlg::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CEmailNotifyTrayDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iProdValue;
}