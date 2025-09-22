// SettingsReports.cpp : implementation file
/****************************************************
*  Program Name: SettingsReports.cpp                                                                                                    
*  Description: setting for after how many days report to be deleted 
*  Author Name: Nitin Kolpkar                                                                                                      
*  Date Of Creation: 27 May 2014
*  Version No: 1.0.0.2
****************************************************/

#include "stdafx.h"
#include "SettingsReports.h"


// CSettingsReports dialog

IMPLEMENT_DYNAMIC(CSettingsReports, CDialog)
/***************************************************************************************************                    
*  Function Name  : CSettingsReports                                                     
*  Description    : C'tor
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 27 May 2014
****************************************************************************************************/
CSettingsReports::CSettingsReports(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CSettingsReports::IDD, pParent)
{

}

/***************************************************************************************************                    
*  Function Name  :           ~CSettingsReports                                                     
*  Description    :           D'tor
*  Author Name    :           Nitin K. Kolapkar 
*  SR_NO
*  Date           :           27 May 2014
****************************************************************************************************/
CSettingsReports::~CSettingsReports()
{
}

/***************************************************************************************************                    
*  Function Name  :           DoDataExchange                                                     
*  Description    :           Called by the framework to exchange and validate dialog data.
*  Author Name    :           Nitin K. Kolapkar 
*  SR_NO
*  Date           :           27 May 2014
****************************************************************************************************/
void CSettingsReports::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_SELECT_DAYS, m_stSelectDays);
	DDX_Control(pDX, IDC_COMBO_DAYS_LIST, m_CBDaysList);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose);
}


/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO		  :
*  Date           : 27 May 2014
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CSettingsReports, CJpegDialog)
	ON_BN_CLICKED(IDOK, &CSettingsReports::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CSettingsReports::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CSettingsReports::OnBnClickedButtonClose)
END_MESSAGE_MAP()


// CSettingsReports message handlers

/***************************************************************************************************                    
*  Function Name  : OnInitDialog                                                     
*  Description    : Windows calls the OnInitDialog function through the standard global 
					dialog-box procedure common to all Microsoft Foundation Class Library
					dialog boxes
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO		  :
*  Date           : 27 May 2014
****************************************************************************************************/

BOOL CSettingsReports::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	if(!Load(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_JPG_SETTINGS_REPORTS), _T("JPG")))
	{
		::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
	}
	Draw();

	CRect rect1;
	this->GetClientRect(rect1);
	//SetWindowPos(&wndTop, rect1.top + 430,rect1.top + 260, rect1.Width()-2, rect1.Height()-2, SWP_NOREDRAW);
	
	CRgn rgn;
	rgn.CreateRectRgn(rect1.top, rect1.left ,rect1.Width()-3,rect1.Height()-3);
	this->SetWindowRgn(rgn, TRUE);

	m_btnClose.SetSkin(theApp.m_hResDLL,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSEOVER,IDB_BITMAP_CLOSE,0,0,0,0,0);
	m_btnClose.SetWindowPos(&wndTop,rect1.left + 285,0,26,17,SWP_NOREDRAW);

	m_stSelectDays.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_SETTINGS_REPORT_SELECT_DAYS"));
	m_stSelectDays.SetWindowPos(&wndTop,rect1.left + 30,70,200,17,SWP_NOREDRAW);
	m_stSelectDays.SetBkColor(RGB(255,255,255));
	m_stSelectDays.SetFont(&theApp.m_fontWWTextNormal);

	/*	ISSUE NO - 490 NAME - NITIN K. TIME - 26st May 2014 */
	m_CBDaysList.SetWindowPos(&wndTop,rect1.left + 30,100,255,17,SWP_NOREDRAW);

	CString m_strDays;
	int m_Days [] = {30, 40, 60};
	for (int i=0;i<3;i++)
	{
		m_strDays.Format(L"%d %s",m_Days[i],theApp.m_objwardwizLangManager.GetString(L"IDS_SETTINGS_REPORT_DAYS"));
		m_CBDaysList.AddString(m_strDays);
	}
	DWORD m_GetDays = ReadRegistryEntryofSetting();
	switch(m_GetDays)
	{
		case 30:
			m_CBDaysList.SetCurSel(0);
		break;
		case 40:
			m_CBDaysList.SetCurSel(1);
		break;
		case 60:
			m_CBDaysList.SetCurSel(2);
		break;
		default:
			m_CBDaysList.SetCurSel(0);
		break;
	}

	m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_OK"));
	m_btnOk.SetWindowPos(&wndTop,rect1.left + 160,135,57,21,SWP_NOREDRAW);
	m_btnOk.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_HOVER_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,0,0,0,0,0);
	m_btnOk.SetTextColorA(BLACK,1,1);

	m_btnCancel.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CANCEL"));
	m_btnCancel.SetWindowPos(&wndTop,rect1.left + 227,135,57,21,SWP_NOREDRAW);
	m_btnCancel.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_HOVER_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,0,0,0,0,0,0);
	m_btnCancel.SetTextColorA(BLACK,1,1);
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/***************************************************************************************************                    
*  Function Name  : OnBnClickedOk                                                     
*  Description    : Function for Storing No. of days into Registry
*  Author Name    : Nitin K. Kolapkar
*  SR_NO		  :
*  Date           : 28 May 2014
****************************************************************************************************/

void CSettingsReports::OnBnClickedOk()
{
	int days =0;
	int selection = m_CBDaysList.GetCurSel();
	if(selection == 0)
	{
		days = 30;
	}
	if(selection == 1)
	{
		days = 40;
	}
	if(selection == 2)
	{
		days = 60;
	}

	LPCTSTR SettingsPath=TEXT("Software\\WardWiz Antivirus");
	WriteRegistryEntryOfSettingsTab(SettingsPath,L"dwDaysToDelRep", days);
	

	OnOK();
}

/***********************************************************************************************
*  Function Name  : WriteRegistryEntryOfSettingsTab
*  Description    : this function is  called for Writing the Registry Key Value for Menu Items
*  Author Name    : Nitin Kolapkar
*  SR_NO		  :
*  Date           : 27 May 2014
***********************************************************************************************/


BOOL CSettingsReports::WriteRegistryEntryOfSettingsTab(LPCTSTR SubKey,CString strKey, DWORD dwChangeValue)
{
	if(!SetRegistrykeyUsingService(SubKey, strKey, REG_DWORD, dwChangeValue,true))
	{
		AddLogEntry(L"### Error in Setting Registry CSettingsReports::WriteRegistryEntryOfSettingsTab", 0, 0, true, SECONDLEVEL);
	}
	Sleep(20);
	return TRUE;
}

/***********************************************************************************************
*  Function Name  : SetRegistrykeyUsingService
*  Description    : this function is  called for setting the Registry Keys using the Services
*  Author Name    : Nitin Kolapkar
*  SR_No		  :
*  Date           : 27 May 2014
***********************************************************************************************/

bool CSettingsReports::SetRegistrykeyUsingService(LPCTSTR SubKey , LPCTSTR lpValueName , DWORD dwType , DWORD dwData, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	
	szPipeData.iMessageInfo = WRITE_REGISTRY;
	szPipeData.dwValue = SZ_DWORD; 
	//szPipeData.hHey = HKEY_LOCAL_MACHINE;

	wcscpy_s(szPipeData.szFirstParam, SubKey);
	wcscpy_s(szPipeData.szSecondParam, lpValueName );
	szPipeData.dwSecondValue = dwData;

	CISpyCommunicator objCom(SERVICE_SERVER);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to set data in CSettingsReports : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bWait)
	{
		if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to set data in CHTMLListCtrl : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : OnBnClickedCancel
*  Description    : close dialog
*  Author Name    : Nitin Kolapkar
*  SR_No		  :
*  Date           : 27 May 2014
***********************************************************************************************/
void CSettingsReports::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}

/***********************************************************************************************
*  Function Name  : OnBnClickedButtonClose
*  Description    : close dialog
*  Author Name    : Nitin Kolapkar
*  SR_No		  :
*  Date           : 27 May 2014
***********************************************************************************************/
void CSettingsReports::OnBnClickedButtonClose()
{
	// TODO: Add your control notification handler code here
	OnCancel();
}


/***********************************************************************************************
*  Function Name  : ReadRegistryEntryofSetting
*  Description    : this function is  called for Reading the Registry entries from the Registry
*  Author Name    : Nitin Kolapkar
*  SR_NO		  :
*  Date           : 29 May 2014
***********************************************************************************************/
DWORD CSettingsReports::ReadRegistryEntryofSetting()
{
	CString strKey = L"dwDaysToDelRep";
	CString m_Key = L"SOFTWARE\\WardWiz Antivirus";

	DWORD dwType = REG_DWORD;
	DWORD returnDWORDValue;
	DWORD dwSize = sizeof(returnDWORDValue);
	HKEY hKey;
	returnDWORDValue = 0;
	LONG lResult = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, m_Key, 0,KEY_READ, &hKey);
	if(lResult == ERROR_SUCCESS)
		lResult = ::RegQueryValueEx(hKey, strKey, NULL,&dwType, (LPBYTE)&returnDWORDValue, &dwSize);
		if(lResult == ERROR_SUCCESS)
		{	
			::RegCloseKey(hKey);
			return returnDWORDValue;	
		}
		else 
		{
			::RegCloseKey(hKey);
			return 0;
		}

	return 0;
}