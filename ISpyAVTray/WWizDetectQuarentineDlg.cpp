// WWizDetectQuarentineDlg.cpp : implementation file
//
/*******************************************************************************************
*  Program Name     : WWizDetectQuarentineDlg.cpp
*  Description      : It is popup dialog which came for File AutoQuarantine.
*  Author Name      : Yogeshwar Rasal
*  Date				: 30 July 2016                                                                                                              *                                                                                                                *
*********************************************************************************************/
#include "stdafx.h"
#include "WWizDetectQuarentineDlg.h"
#include "afxdialogex.h"
#define SETVIRUSCOUNT_EVENT_CODE (FIRST_APPLICATION_EVENT_CODE + 4)

// CWWizDetectQuarentineDlg dialog

IMPLEMENT_DYNAMIC(CWWizDetectQuarentineDlg, CDialog)
/***************************************************************************
Function Name  : CWWizDetectQuarentineDlg
Description    : Constructor
Author Name    : Yogeshwar Rasal
Date           : 30 July 2016
****************************************************************************/
CWWizDetectQuarentineDlg::CWWizDetectQuarentineDlg(CWnd* pParent /*=NULL*/)
: CDialog(CWWizDetectQuarentineDlg::IDD, pParent)
	, m_bFirstEntry(true)
{
	m_iCurrentIndex = 0;
	bIsNextPrevClicked = false;
}

/***************************************************************************
Function Name  : ~CWWizDetectQuarentineDlg
Description    : Destructor
Author Name    :
Date           :  2016
****************************************************************************/
CWWizDetectQuarentineDlg::~CWWizDetectQuarentineDlg()
{
}

/***************************************************************************
Function Name  : DoDataExchange
Description    : Called by the framework to exchange and validate dialog data.
Author Name    : Yogeshwar Rasal
Date           : 30 July 2016
****************************************************************************/
void CWWizDetectQuarentineDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CWWizDetectQuarentineDlg, CDialog)
END_MESSAGE_MAP()

// CWWizDetectQuarentineDlg message handlers

HWINDOW   CWWizDetectQuarentineDlg::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWWizDetectQuarentineDlg::get_resource_instance() { return theApp.m_hInstance; }

/***************************************************************************
Function Name  : OnInitDialog
Description    : This method is called in response to the WM_INITDIALOG message.
Author Name    : Yogeshwar Rasal
Date           : 30 July 2016
****************************************************************************/
BOOL CWWizDetectQuarentineDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	SetWindowPos(&this->wndTopMost, 0, 0, 100, 100, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);

	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;

	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0;
	UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_TRAY_VIRUS_PROTECT.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_TRAY_VIRUS_PROTECT.htm");
	m_root_el = root();
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);

	int cxIcon = GetSystemMetrics(SM_CXFULLSCREEN);
	int cyIcon = GetSystemMetrics(SM_CYFULLSCREEN);

	int i = GetTaskBarHeight();
	int j = GetTaskBarWidth();
	int ixRect = cxIcon - pIntMaxWidth;
	int iyRect = cyIcon - pIntHeight;

	this->ShowWindow(SW_HIDE);
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
			SetWindowPos(NULL, ixRect, iyRect + 100, pIntMinWidth, pIntHeight, SWP_NOREDRAW);
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
		AddLogEntry(L"### Excpetion in CWardwizTraySucessDlg::OnInitDialog", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/***************************************************************************************************
*  Function Name  :	AddQuarantineInfo
*  Description    :	This Function is used to send Quarantine Info. for Automatic Call.
*  Author Name    :	Nitin K
*  Date           :	12 Sept. 2016
****************************************************************************************************/
void CWWizDetectQuarentineDlg::AddQuarantineInfo(CString csName, CString csPath, int iThreatAction, int iSize)
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
		map.set_item("three", sciter::value(iThreatAction));
		map.set_item("four", sciter::value(iSize));
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
		AddLogEntry(L"### Exception in CWardwizDetectQuarentineDlg::AddQuarantineInfo", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : PreTranslateMessage
Description    : Function for PreTranslation of Message.
Author Name    : Yogeshwar Rasal
Date           : 30 July 2016
****************************************************************************/
BOOL CWWizDetectQuarentineDlg::PreTranslateMessage(MSG* pMsg)
{
	return __super::PreTranslateMessage(pMsg);
}

/***************************************************************************
Function Name  : GetTaskBarHeight
Description    : To get the Task Bar Height
Author Name    : Yogeshwar Rasal
Date           : 30 July 2016
****************************************************************************/
int CWWizDetectQuarentineDlg::GetTaskBarHeight()
{
	try
	{
		RECT rect;
		HWND taskBar;
		taskBar = ::FindWindow(L"Shell_TrayWnd", NULL);
		if (taskBar && ::GetWindowRect(taskBar, &rect))
		{
			return rect.bottom - rect.top;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizDetectQuarentineDlg::GetTaskBarHeight", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : GetTaskBarWidth
Description    : To get the Task Bar Width
Author Name    : Yogeshwar Rasal
Date           : 30 July 2016
****************************************************************************/
int CWWizDetectQuarentineDlg::GetTaskBarWidth()
{
	try
	{
		RECT rect;
		HWND taskBar;
		taskBar = ::FindWindow(L"Shell_TrayWnd", NULL);
		if (taskBar && ::GetWindowRect(taskBar, &rect))
		{
			return rect.right - rect.left;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizDetectQuarentineDlg::GetTaskBarHeight", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :	WindowProc
*  Description    :	This callback Procedure is used to Handle All Window Actions
*  Author Name    :	Yogeshwar Rasal
*  Date           : 30 July 2016
****************************************************************************************************/
LRESULT CWWizDetectQuarentineDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult;
	BOOL    b_Handled = FALSE;
	__try
	{
		lResult = SciterProcND(this->GetSafeHwnd(), message, wParam, lParam, &b_Handled);

		if (b_Handled)      // if it was handled by the Sciter
			return lResult; // then no further processing is required.
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Excpetion in CWardwizDetectQuarentineDlg::WindowProc", 0, 0, true, SECONDLEVEL);
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

/***************************************************************************************************
*  Function Name  :	AddIntoQueue
*  Description    :	This Function is used to put Quarantine Info.
*  Author Name    :	Yogeshwar Rasal
*  Date           :	 30 July 2016
****************************************************************************************************/
bool CWWizDetectQuarentineDlg::AddIntoQueue(CString csThreatName, CString csFilePath, DWORD dwActionTaken)
{
	try
	{
		if (ISDuplicateEntry(csFilePath.GetBuffer()))
		{
			AddLogEntry(L">>> Duplicate Entry, FilePath: [%s]", csFilePath, 0, true, ZEROLEVEL);
			return true;
		}

		//This check is to avoid Sleep for each time.
		if (m_bFirstEntry)
		{
			Sleep(100);
			m_bFirstEntry = false;
		}

		DWORD dwDetectThreadID = 0x00;
		THREATINFO szThreatInfo = { 0 };
		CString csSize;
		CString csName;
		CString csPath;
		int iThreatAction;

		_tcscpy_s(szThreatInfo.szFilePath, csFilePath);
		_tcscpy_s(szThreatInfo.szThreatName, csThreatName);

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
		iThreatAction = static_cast<int>(m_vecThreatList.at(m_iCurrentIndex).dwActionTaken);
		
		TCHAR					m_szActiveScanFilePath[1024];
		memset(m_szActiveScanFilePath, 0, sizeof(m_szActiveScanFilePath));
		int iCount  = csPath.ReverseFind('\\');
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
		//GetShortPathName(csPath, szShortName, dwbuf);
		AddQuarantineInfo(csName, csFinalFilePath, iThreatAction, m_iSize);
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizDetectQuarentineDlg::AddIntoQueue, FilePath: [%s]", csFilePath, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :	GetQuarantineInfo
*  Description    :	This Sciter Function is used to get Quarantine Info. Callback.
*  Author Name    :	Yogeshwar Rasal
*  Date           :	30 July 2016
****************************************************************************************************/
json::value CWWizDetectQuarentineDlg::GetQuarantineInfo()
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
		AddLogEntry(L"### Excpetion in CWardwizDetectQuarentineDlg::GetQuarantineInfo", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  :	OnBTNCilckClose
*  Description    :	This Sciter Function is used to close a dialog.
*  Author Name    :	Yogeshwar Rasal
*  Date           :	 30 July 2016
****************************************************************************************************/
json::value CWWizDetectQuarentineDlg::OnBTNCilckClose()
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
		AddLogEntry(L"### Exception in CWardwizDetectQuarentineDlg::OnBTNCilckClose", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  :	ISDuplicateEntry
*  Description    :	Function to find duplicate entry in vector, @true: found, @false: not found.
*  Author Name    :	Yogeshwar Rasal
*  Date           :	 30 July 2016
****************************************************************************************************/
bool CWWizDetectQuarentineDlg::ISDuplicateEntry(CString csFilePath)
{
	bool bReturn = false;
	try
	{
		for (VECTHREATINFO::iterator it = m_vecThreatList.begin(); it != m_vecThreatList.end(); ++it)
		{
			TCHAR szThreatPath[1024] = { 0 };
			_tcscpy_s(szThreatPath, (*it).szFilePath);
			_tcslwr_s(szThreatPath);
			csFilePath.MakeLower();

			if (_tcscmp(szThreatPath, csFilePath) == 0)
			{
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDetectQuarentineDlg::ISDuplicateEntry, File: [%s]", csFilePath, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

json::value CWWizDetectQuarentineDlg::On_GetLanguageID()
{
	int iLangValue = 0;
	try
	{
		iLangValue = theApp.m_objwardwizLangManager.GetSelectedLanguage();

		CString csLangID;
		csLangID.Format(L"LangID: %d", theApp.m_dwSelectedLangID);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDetectQuarentineDlg::On_GetLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Jeena Mariam Saji
*  Date			  : 17 Aug 2016
****************************************************************************************************/
json::value CWWizDetectQuarentineDlg::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDetectQuarentineDlg::On_GetProductID", 0, 0, true, SECONDLEVEL);
	}
	return iProdValue;
}

/***************************************************************************************************
Function Name  : OnGetAppPath
Description    : for Get the App Path and Set it in Script
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 10th Oct 2016
/***************************************************************************************************/
json::value CWWizDetectQuarentineDlg::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDetectQuarentineDlg::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_GetThemeID
Description    : To get theme ID
Author Name    : Jeena Mariam Saji
Date           : 30th Dec 2018
/***************************************************************************************************/
json::value CWWizDetectQuarentineDlg::On_GetThemeID()
{
	try
	{
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		return ((int)GetPrivateProfileInt(L"VBSETTINGS", L"ThemeID", 0, csIniFilePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDetectQuarentineDlg::On_GetThemeID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
Function Name  : On_GetShortPath
Description    : To Convert long virus path to short path
Author Name    : Swapnil Bhave
Date           : 4th Spt 2019
/***************************************************************************************************/
json::value CWWizDetectQuarentineDlg::On_GetShortPath(SCITER_VALUE svPath)
{
	try
	{
		std::wstring srcPath = svPath.get(L"");
		CString csDestUrl = srcPath.c_str();

		csDestUrl.Format(L"%s%s%s", csDestUrl.Left(12), L"...", csDestUrl.Right(17));
		return (SCITER_VALUE)csDestUrl.GetBuffer();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizDetectQuarentineDlg::On_GetShortPath", 0, 0, true, SECONDLEVEL);
	}
	return "";
}

