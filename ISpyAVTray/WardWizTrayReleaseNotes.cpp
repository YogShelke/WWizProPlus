// WardWizTrayReleaseNotes.cpp : implementation file
/*******************************************************************************************
*  Program Name: WardWizTrayReleaseNotes.cpp
*  Description: Pop-up displaying the Release Information of 'Added Features & Bug Fixed' after successful product update.
*  Author Name:  Varada Ikhar
*  Date Of Creation: 4th May-2015
*  Version No:    1.11.0.17                                                                                                      
*  Special Logic Used:                                                                                                                                                                                                                                                             
*  Modification Log:
*  1. Modified xyz function in main        Date modified         CSR NO                                                                                                                             
*********************************************************************************************/
//

#include "stdafx.h"
#include "ISpyAVTray.h"
#include "WardWizTrayReleaseNotes.h"
#include "afxdialogex.h"

// CWardWizTrayReleaseNotes dialog

IMPLEMENT_DYNAMIC(CWardWizTrayReleaseNotes, CDialogEx)

/***************************************************************************************************
*  Function Name  : CISpyTrayReleaseNotes
*  Description    : C'tor
*  Author Name    : Varada Ikhar
*  SR_NO		  :
*  Date           : 4th May-2015
****************************************************************************************************/
CWardWizTrayReleaseNotes::CWardWizTrayReleaseNotes(CWnd* pParent /*=NULL*/)
: CJpegDialog(CWardWizTrayReleaseNotes::IDD, pParent)
{

}

/***************************************************************************************************
*  Function Name  : ReleaseInformationFromTray
*  Description    : Destuctor
*  Author Name    : Varada Ikhar
*  SR_NO		  :
*  Date           : 4th May-2015
****************************************************************************************************/
CWardWizTrayReleaseNotes::~CWardWizTrayReleaseNotes()
{
}

/***************************************************************************************************
*  Function Name  : DoDataExchange
*  Description    : Exachanging data
*  Author Name    : Varada Ikhar
*  SR_NO		  :
*  Date           : 4th May-2015
****************************************************************************************************/
void CWardWizTrayReleaseNotes::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_APP_DB_VERSION, m_stAppDBVersion);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_BUTTON_OK, m_btnOK);
	DDX_Control(pDX, IDC_RICHEDIT_RELEASE_NOTE_TEXT, m_richedtReleaseInfo);
}

BEGIN_MESSAGE_MAP(CWardWizTrayReleaseNotes, CJpegDialog)
	ON_WM_NCHITTEST()
	ON_EN_VSCROLL(IDC_RICHEDIT_RELEASE_NOTE_TEXT, &CWardWizTrayReleaseNotes::OnEnVscrollRicheditReleaseNoteText)
	ON_BN_CLICKED(IDC_BUTTON_OK, &CWardWizTrayReleaseNotes::OnBnClickedButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CWardWizTrayReleaseNotes::OnBnClickedButtonClose)
END_MESSAGE_MAP()

HWINDOW   CWardWizTrayReleaseNotes::get_hwnd() { return this->GetSafeHwnd(); }
HINSTANCE CWardWizTrayReleaseNotes::get_resource_instance() { return theApp.m_hInstance; }

// CWardWizTrayReleaseNotes message handlers

/***************************************************************************************************
*  Function Name  : OnInitDialog
*  Description    : Initialization of dialog
*  Author Name    : Varada Ikhar
*  SR_NO		  :
*  Date           : 4th May-2015
****************************************************************************************************/
BOOL CWardWizTrayReleaseNotes::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	ModifyStyleEx(WS_EX_APPWINDOW, WS_EX_TOOLWINDOW);
	//::BringWindowToTop(this->m_hWnd);
	SetWindowPos(&this->wndTopMost, 0, 0, 700, 900, SWP_FRAMECHANGED | SWP_NOZORDER | SWP_NOMOVE);
	// code for painting the background Ends here
	//to set round window
	
	//SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	LPCBYTE pb = 0;
	UINT cb = 0;
	this->setup_callback(); // attach sciter::host callbacks
	sciter::attach_dom_event_handler(this->get_hwnd(), this); // attach this as a DOM events 

	// load intial document
	sciter::load_resource_data((HINSTANCE)theApp.m_hResDLL, L"res:IDR_HTM_TRAY_RELEASE_NOTES.htm", pb, cb);
	(this)->load_html(pb, cb, L"res:IDR_HTM_TRAY_RELEASE_NOTES.htm");

	INT pIntMinWidth = 0;
	INT pIntMaxWidth = 0;
	INT pIntHeight = 0;
	m_root_el = root();
	SciterGetElementIntrinsicWidths(m_root_el, &pIntMinWidth, &pIntMaxWidth);
	SciterGetElementIntrinsicHeight(m_root_el, pIntMinWidth, &pIntHeight);
	::MoveWindow(this->get_hwnd(), 0, 0, pIntMaxWidth, pIntHeight, true);

	int cxIcon = GetSystemMetrics(SM_CXFULLSCREEN);
	int cyIcon = GetSystemMetrics(SM_CYFULLSCREEN);

	int i = GetTaskBarHeight();
	int j = GetTaskBarWidth();
	int ixRect = cxIcon - pIntMaxWidth;
	int iyRect = cyIcon - pIntHeight;

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
			SetWindowPos(NULL, ixRect, iyRect+200, pIntMinWidth, pIntHeight, SWP_NOREDRAW);
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
		AddLogEntry(L"### Exception in CWardwizTrayReleaseNotes::OnInitDialog", 0, 0, true , SECONDLEVEL);
	}

	// To Hide Old UI
	m_btnClose.ShowWindow(SW_SHOW);
	m_stAppDBVersion.ShowWindow(SW_HIDE);
	m_richedtReleaseInfo.ShowWindow(SW_HIDE);
	m_btnOK.ShowWindow(SW_HIDE);
	m_btnClose.ShowWindow(SW_HIDE);	
	///*if (GetInfoFromINI() == false)
	//{
	//	return FALSE;
	//}*/
	
	// CallBack
	m_svProductinfoCB.call((SCITER_STRING)m_csAppDBVersion, (SCITER_STRING)m_csVersioninfo);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/***************************************************************************
Function Name  : GetTaskBarHeight
Description    : To get the Task Bar Height
Author Name    : Varada Ikhar
Date           : 4th May 2015
SR_NO		   :
****************************************************************************/
int CWardWizTrayReleaseNotes::GetTaskBarHeight()
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
		AddLogEntry(L"### Exception in CWardwizTrayReleaseNotes::GetTaskBarHeight", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : GetTaskBarWidth
Description    : To get the Task Bar Width
Author Name    : Varada Ikhar
Date           : 4th May 2015
SR_NO		   :
****************************************************************************/
int CWardWizTrayReleaseNotes::GetTaskBarWidth()
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
		AddLogEntry(L"### Exception in CWardwizTrayReleaseNotes::GetTaskBarWidth", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************
Function Name  : GetInfoFromINI
Description    : Extract information from INI and display on UI.
Author Name    : Varada Ikhar
Date           : 4th May 2015
SR_NO		   :
****************************************************************************/
bool CWardWizTrayReleaseNotes::GetInfoFromINI()
{
	CString AppDBVersion = L"";
	CString csAppendstring(L"");

	try
	{
		TCHAR  szAllUserPath[255] = { 0 };
		TCHAR  szActualINIPath[255] = { 0 };
		TCHAR  szValueData[512] = { 0 };
		TCHAR  szValueName[512] = { 0 };
		DWORD  i = 0x00, dwCount = 0x00, dwCountFeature = 0x00, dwCountBugsFixed = 0x00;
		CString ProductVersion = L"";

		GetEnvironmentVariable(L"ALLUSERSPROFILE", szAllUserPath, 255);

		if ((!theApp.m_dwProductID) || (theApp.m_dwProductID > 4))
		{
			AddLogEntry(L"### Product ID mismatched", 0, 0, true, SECONDLEVEL);
			return false;
		}

		switch (theApp.m_dwProductID)
		{
		case 1:
			swprintf_s(szActualINIPath, _countof(szActualINIPath), L"%s\\Vibranium\\%s", szAllUserPath, L"VibroPatchTS.ini"); // WRDWIZRELNOTESE);
			break;
		case 2:
			swprintf_s(szActualINIPath, _countof(szActualINIPath), L"%s\\Vibranium\\%s", szAllUserPath, L"WWizPatchP.ini"); //WRDWIZRELNOTESP);
			break;
		case 3:
			swprintf_s(szActualINIPath, _countof(szActualINIPath), L"%s\\Vibranium\\%s", szAllUserPath, L"WWizPatchT.ini"); //WRDWIZRELNOTEST);
			break;
		case 4:
			swprintf_s(szActualINIPath, _countof(szActualINIPath), L"%s\\Vibranium\\%s", szAllUserPath, L"WWizPatchB.ini"); //WRDWIZRELNOTESB);
			break;
		case 5:
			swprintf_s(szActualINIPath, _countof(szActualINIPath), L"%s\\Vibranium\\%s", szAllUserPath, L"VibroPatchAS.ini"); //WRDWIZRELNOTESEP);
			break;
		}

		int fileExist = 0;
		fileExist = PathFileExists(szActualINIPath);
		if (fileExist == 0)
		{
			AddLogEntry(L"### File not found : %s in GetInfoFromINI ", szActualINIPath, 0, true, SECONDLEVEL);
			return false;
		}
		else
		{
			GetPrivateProfileString(L"ProductVersion", L"ProductVer", L"", szValueData, 511, szActualINIPath);
			if (!szValueData[0])
			{
				AddLogEntry(L"### WardwizReleseNotes.ini::Invalid Entries for(%s) in CWardwizTrayReleaseNotes::GetInfoFromINI.", 0, szActualINIPath, true, SECONDLEVEL);
				return false;
			}
			else
			{
				AppDBVersion.Format(L"%s: %s", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODINFO_PRODVERSION_TEXT"), szValueData);
			}

			GetPrivateProfileString(L"DatabaseVersion", L"DatabaseVer", L"", szValueData, 511, szActualINIPath);
			if (!szValueData[0])
			{
				AddLogEntry(L"### WardwizReleseNotes.ini::Invalid Entries for(%s) in CWardwizTrayReleaseNotes::GetInfoFromINI.", 0, szActualINIPath, true, SECONDLEVEL);
				return false;
			}
			else
			{
				AppDBVersion.AppendFormat(L"  %s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ABOUTGUI_DBVERSION"), szValueData);

			}

			m_csAppDBVersion = AppDBVersion;
			dwCountFeature = GetPrivateProfileInt(L"AddedFeature", L"Count", 0, szActualINIPath);
			dwCountBugsFixed = GetPrivateProfileInt(L"EnhancedFunctionality", L"Count", 0, szActualINIPath);
			dwCount = dwCountFeature + dwCountBugsFixed;

			if (dwCount == 0x00)
			{
				AddLogEntry(L">>> No Release Information to display.", 0, 0, true, ZEROLEVEL);
				return false;
			}
			else
			{
				CString csReleaseInfo = theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_RELEASE_NOTE_NEW_FEATURES"); //L""; //
				if (dwCountFeature > 0)
				{
					csAppendstring.Append(csReleaseInfo);
					for (i = 1; i <= dwCountFeature; i++)
					{
						switch (theApp.m_dwSelectedLangID)
						{
						case GERMAN:
							swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
							GetPrivateProfileString(L"AddedFeatureGerman", szValueName, L"", szValueData, 511, szActualINIPath);
							if (!szValueData[0])
							{
								AddLogEntry(L"### Invalid Entries for(%s) in (%s) CWardwizTrayReleaseNotes::GetInfoFromINI.", szValueName, szActualINIPath, true, 0x00);
							}
							else
							{
								csReleaseInfo.Format(L"\r\n%lu. %s", i, szValueData);
								csAppendstring.Append(csReleaseInfo);
							}
							break;
						default:
						case ENGLISH:
							swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
							GetPrivateProfileString(L"AddedFeature", szValueName, L"", szValueData, 511, szActualINIPath);
							if (!szValueData[0])
							{
								AddLogEntry(L"### Invalid Entries for(%s) in (%s) CWardwizTrayReleaseNotes::GetInfoFromINI.", szValueName, szActualINIPath, true, 0x00);
							}
							else
							{
								csReleaseInfo.Format(L"\r\n%lu. %s", i, szValueData);
								csAppendstring.Append(csReleaseInfo);
							}
							break;
						}

					}
				}

				if (dwCountBugsFixed > 0)
				{
					if (dwCountFeature > 0)
					{
						csAppendstring.Append(L"\r\n\r\n");
						csAppendstring.Append(theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_RELEASE_NOTE_NEW_FUNCTIONALITIES"));
					}
					else
					{
						csAppendstring.Append(theApp.m_objwardwizLangManager.GetString(L"IDS_TRAY_RELEASE_NOTE_ENHANCED_FUNCTIONALITY"));
					}
					for (i = 1; i <= dwCountBugsFixed; i++)
					{
						switch (theApp.m_dwSelectedLangID)
						{
						case GERMAN:
							swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
							GetPrivateProfileString(L"EnhancedFunctionalityGerman", szValueName, L"", szValueData, 511, szActualINIPath);
							if (!szValueData[0])
							{
								AddLogEntry(L"### Invalid Entries for(%s) in (%s) in CWardwizTrayReleaseNotes::GetInfoFromINI.", szValueName, szActualINIPath, true, 0x00);
							}
							else
							{
								csReleaseInfo.Format(L"\r\n%lu. %s", i, szValueData);
								csAppendstring.Append(csReleaseInfo);
							}
							break;

						default:
						case ENGLISH:
							swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
							GetPrivateProfileString(L"EnhancedFunctionality", szValueName, L"", szValueData, 511, szActualINIPath);
							if (!szValueData[0])
							{
								AddLogEntry(L"### Invalid Entries for(%s) in (%s) in CWardwizTrayReleaseNotes::GetInfoFromINI.", szValueName, szActualINIPath, true, 0x00);
							}
							else
							{
								csReleaseInfo.Format(L"\r\n%lu. %s", i, szValueData);
								csAppendstring.Append(csReleaseInfo);
							}
							break;

						}
					}
				}
				m_csVersioninfo = csAppendstring;

				DWORD dwRelNoteShow = 0x00;
				if (m_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.m_csRegKeyPath.GetBuffer(), L"dwRelNotesShow", dwRelNoteShow) != 0)
				{
					AddLogEntry(L"### Failed to Set Registry value of dwRelNotesShow to 0 in CWardwizTrayReleaseNotes::ShowReleaseNotesMsg", 0, 0, true, SECONDLEVEL);
				}
			}
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayReleaseNotes::GetInfoFromINI.", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function Name  : PreTranslateMessage
Description    :
Author Name    : Varada Ikhar
Date           : 8th May 2015
SR_NO		   :
****************************************************************************/
BOOL CWardWizTrayReleaseNotes::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->wParam == VK_ESCAPE)
		return TRUE;

	return CJpegDialog::PreTranslateMessage(pMsg);
}

/***************************************************************************
Function Name  : OnNcHitTest
Description    : Tray pop-up could not be moved.
Author Name    : Varada Ikhar
Date           : 8th May 2015
SR_NO		   :
****************************************************************************/
LRESULT CWardWizTrayReleaseNotes::OnNcHitTest(CPoint point)
{
	// return CJpegDialog::OnNcHitTest(point);
	return HTCLIENT;
}

/**************************************************************************************************************
Function Name  : OnEnVscrollRicheditReleaseNotes
Description    : If the text shown in richedit control is too long vertical scroll should be shown.
Author Name    : Varada Ikhar
Date           : 8th May 2015
SR_NO		   :
***************************************************************************************************************/
void CWardWizTrayReleaseNotes::OnEnVscrollRicheditReleaseNoteText()
{
	Invalidate(true);
}

/***************************************************************************************************************
Function Name  : OnBnClickedOk
Description    : On click of OK button, WrdWizReleaseNote.ini file will be deleted and message will be closed.
Author Name    : Varada Ikhar
Date           : 8th May 2015
SR_NO		   :
*****************************************************************************************************************/
void CWardWizTrayReleaseNotes::OnBnClickedButtonOk()
{
	OnBnClickedButtonClose();
}

/******************************************************************************************************************
Function Name  : OnBnClickedButtonClose
Description    : On click of Close button, WrdWizReleaseNote.ini file will be deleted and message will be closed.
Author Name    : Varada Ikhar
Date           : 8th May 2015
SR_NO		   :
********************************************************************************************************************/
void CWardWizTrayReleaseNotes::OnBnClickedButtonClose()
{
	LPCTSTR SubKey = theApp.m_csRegKeyPath;
	LPCTSTR strKey = L"dwRelNotesShow";
	DWORD dwChangeValue = 0x00;
	if (!SetRegistrykeyUsingService(SubKey, strKey, REG_DWORD, dwChangeValue, true))
	{
		AddLogEntry(L"### Error in Setting Registry CWardwizTrayReleaseNotes::OnBnClickedButtonClose", 0, 0, true, SECONDLEVEL);
	}
	Sleep(50);
	OnCancel();
}

/***********************************************************************************************
Function Name  : SetRegistrykeyUsingService
Description    : this function is  called for Set the Registry Key Value for Menu Items using Service
Author Name    : Varada Ikhar
Date           : 8th May-2015
SR_NO		   :
***********************************************************************************************/
bool CWardWizTrayReleaseNotes::SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	memset(&szPipeData, 0, sizeof(szPipeData));

	szPipeData.iMessageInfo = WRITE_REGISTRY;
	szPipeData.dwValue = SZ_DWORD;
	//szPipeData.hHey = HKEY_LOCAL_MACHINE;

	wcscpy_s(szPipeData.szFirstParam, SubKey);
	wcscpy_s(szPipeData.szSecondParam, lpValueName);
	szPipeData.dwSecondValue = dwData;

	CISpyCommunicator objCom(SERVICE_SERVER);
	if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to set data in CWardwizTrayReleaseNotes::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if (bWait)
	{
		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to set data in CWardwizTrayReleaseNotes::SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

/***********************************************************************************************
Function Name  : GetRegistryDWORDEntry
Description    : this function is  called for Reading values from Registry for dwRelNotesShow
Author Name    : Varada Ikhar
Date           : 9th May-2015
SR_NO		   :
***********************************************************************************************/
DWORD CWardWizTrayReleaseNotes::GetRegistryDWORDEntry(HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, DWORD& dwValue)
{
	HKEY  hKey = NULL;
	DWORD dwDWORDValue = 0x00;
	DWORD dwvalueSize = sizeof(DWORD);
	DWORD dwType = REG_DWORD;

	dwValue = 0x00;

	if (RegOpenKeyEx(hRootKey, pKeyName, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
	{
		return 0x01;
	}

	if (RegQueryValueEx(hKey, pValueName, NULL, &dwType, (LPBYTE)&dwDWORDValue, &dwvalueSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return 0x02;
	}

	RegCloseKey(hKey);

	dwValue = dwDWORDValue;
	return 0x00;
}

/***************************************************************************************************
*  Function Name  :	WindowProc
*  Description    :	This callback Procedure is used to Handle All Window Actions
*  Author Name    :	Yogeshwar Rasal
*  Date           : 14 June 2016
****************************************************************************************************/

LRESULT CWardWizTrayReleaseNotes::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class

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
		AddLogEntry(L"### Excpetion in CWardwizTrayReleaseNotes::WindowProc", 0, 0, true, SECONDLEVEL);
	}	
	return CJpegDialog::WindowProc(message, wParam, lParam);
}

/***************************************************************************************************
*  Function Name  :	GetReleaseNotesinfo
*  Description    :	Sciter Function to Handle Popup.
*  Author Name    :	Yogeshwar Rasal
*  Date           : 14 June 2016
****************************************************************************************************/
json::value CWardWizTrayReleaseNotes::GetReleaseNotesinfo(SCITER_VALUE svProductinfo)
{
	try
	{	// Fetch values from GetInfoFromINI();
		GetInfoFromINI();
		m_svProductinfoCB = svProductinfo;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayReleaseNotes::GetReleaseNotesinfo", 0, 0, true, SECONDLEVEL);
	}
	return json::value(0);
}

/***************************************************************************************************
*  Function Name  :	On_ClickOK
*  Description    :	Sciter Function to Handle "OK" Button.
*  Author Name    :	Yogeshwar Rasal
*  Date           : 14 June 2016
****************************************************************************************************/
json::value CWardWizTrayReleaseNotes::On_ClickOK()
{
   try
	{
		OnBnClickedButtonOk();
	}
	
   catch (...)
   {
	   AddLogEntry(L"### Exception in CWardwizTrayReleaseNotes::OnClickOK", 0, 0, true, SECONDLEVEL);
   }
	return json::value(0);
}

/***************************************************************************************************
Function Name  : On_GetLanguageID
Description    : Function to get Language ID
Author Name    : Nitin Kolapkar
SR_NO		   :
Date           : 10th Oct 2016
/***************************************************************************************************/
json::value CWardWizTrayReleaseNotes::On_GetLanguageID()
{
	int iLangValue = 0;
	try
	{
		iLangValue = theApp.m_dwSelectedLangID;

		CString csLangID;
		csLangID.Format(L"LangID: %d", theApp.m_dwSelectedLangID);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayReleaseNotes::On_GetLanguageID", 0, 0, true, SECONDLEVEL);
	}
	return iLangValue;
}

/***************************************************************************************************
*  Function Name  : On_GetProductID
*  Description    : Function to get valid product id
*  Author Name    : Jeena Mariam Saji
*  Date			  : 17 Aug 2016
****************************************************************************************************/
json::value CWardWizTrayReleaseNotes::On_GetProductID()
{
	int iProdValue = 0;
	try
	{
		iProdValue = theApp.m_dwProductID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayReleaseNotes::On_GetProductID", 0, 0, true, SECONDLEVEL);
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
json::value CWardWizTrayReleaseNotes::OnGetAppPath()
{
	try
	{
		return (SCITER_STRING)theApp.GetModuleFilePath();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizTrayReleaseNotes::OnGetAppPath", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}