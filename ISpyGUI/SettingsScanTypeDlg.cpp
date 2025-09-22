// SettingsScanTypeDlg.cpp : implementation file
/****************************************************
*  Program Name: SettingsScanTypeDlg.cpp                                                                                                    
*  Description: Display dialog to select scan type 
*  Author Name: Nitin Kolpkar                                                                                                      
*  Date Of Creation: 9th June 2014
*  Version No: 1.0.0.2
****************************************************/

#include "stdafx.h"
#include "SettingsScanTypeDlg.h"


// CSettingsScanTypeDlg dialog

IMPLEMENT_DYNAMIC(CSettingsScanTypeDlg, CDialog)

/***************************************************************************************************                    
*  Function Name  : CSettingsScanTypeDlg                                                     
*  Description    : C'tor
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 9th June 2014
****************************************************************************************************/
CSettingsScanTypeDlg::CSettingsScanTypeDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CSettingsScanTypeDlg::IDD, pParent)
{

}

/***************************************************************************************************                    
*  Function Name  : ~CSettingsScanTypeDlg                                                     
*  Description    : D'tor
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 9th June 2014
****************************************************************************************************/
CSettingsScanTypeDlg::~CSettingsScanTypeDlg()
{
}

/***************************************************************************************************                    
*  Function Name  : DoDataExchange                                                     
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 9th June 2014
****************************************************************************************************/
void CSettingsScanTypeDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_BUTTON_OK, m_btnOk);
	DDX_Control(pDX, IDC_BUTTON_CANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_COMBO_SCAN_TYPE, m_CBScanType);
	DDX_Control(pDX, IDC_STATIC_SELECT_SCAN_TYPE, m_stSelectScanType);
}

/***************************************************************************************************                    
*  Function Name  : MESSAGE_MAP                                                     
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 9th June 2014
****************************************************************************************************/
BEGIN_MESSAGE_MAP(CSettingsScanTypeDlg, CJpegDialog)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CSettingsScanTypeDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_OK, &CSettingsScanTypeDlg::OnBnClickedButtonOk)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CSettingsScanTypeDlg::OnBnClickedButtonCancel)
END_MESSAGE_MAP()


// CSettingsScanTypeDlg message handlers

/***************************************************************************************************                    
*  Function Name  : OnInitDialog                                                     
*  Description    : Windows calls the OnInitDialog function through the standard global 
					dialog-box procedure common to all Microsoft Foundation Class Library
					dialog boxes
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 9th June 2014
****************************************************************************************************/

BOOL CSettingsScanTypeDlg::OnInitDialog()
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

	m_stSelectScanType.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_SETTINGS_SCAN_TYPE_SELECT_TYPE"));
	m_stSelectScanType.SetWindowPos(&wndTop,rect1.left + 30,70,200,17,SWP_NOREDRAW);
	m_stSelectScanType.SetBkColor(RGB(255,255,255));
	m_stSelectScanType.SetFont(&theApp.m_fontWWTextNormal);

	/*	ISSUE NO - 490 NAME - NITIN K. TIME - 26st May 2014 */
	m_CBScanType.SetWindowPos(&wndTop,rect1.left + 30,100,255,17,SWP_NOREDRAW);


	m_CBScanType.AddString(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_QUICKSCAN"));
	m_CBScanType.AddString(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FULLSCAN"));
	
	m_CBScanType.SetCurSel(0);
	//In Setting->Scan->Scan computer at start up->if i select full scan option then it shows Quick scan ,It should be full scan. 
	//Niranjan Deshak - 11/03/2015.
	DWORD m_value = ReadRegistryEntryofSetting();
	if (m_value == 0x01)
	{
		m_CBScanType.SetCurSel(0);
	}
	else if (m_value == 0x02)
	{
		m_CBScanType.SetCurSel(1);
	}
	/*	ISSUE NO - 750 NAME - NITIN K. TIME - 17th June 2014 */
	/*m_ScanTypeS.Format(L"%s%s",GetWardWizPathFromRegistry(),L"WRDWIZAVUI.EXE -STARTUPSCAN");
	m_ScanTypeF.Format(L"%s%s",GetWardWizPathFromRegistry(),L"WRDWIZAVUI.EXE -STARTUPSCAN -FULLSCAN");
	if(m_value == m_ScanTypeS)
	{
		m_CBScanType.SetCurSel(0);
	}
	else if(m_value == m_ScanTypeF)
	{
		m_CBScanType.SetCurSel(1);
	}*/

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
*  Function Name  : OnBnClickedButtonClose                                                     
*  Description    : Closes scan type dialog
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 9th June 2014
****************************************************************************************************/
void CSettingsScanTypeDlg::OnBnClickedButtonClose()
{
	OnCancel();
}

// reslove by lalit 1-14-2015 ,issue- handling of startup-scan from service, to resolve the issue of restart now popup coming frequently even after restart in xp
/***************************************************************************************************                    
*  Function Name  : OnBnClickedButtonOk                                                     
*  Description    : Take selected type for startup scan and keep path and type into run registry
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 9th June 2014
****************************************************************************************************/
void CSettingsScanTypeDlg::OnBnClickedButtonOk()
{
	/*	ISSUE NO - 750 NAME - NITIN K. TIME - 17th June 2014 */
	CString m_ScanType = NULL;
	int selection = m_CBScanType.GetCurSel();

	DWORD dwScanType = selection + 1 ;	// QUICK SCAN = 1;
										// FULL SCAN = 2;

	if(!SendRegistryData2Service(SZ_DWORD, _T("SOFTWARE\\WardWiz Antivirus"), 
		_T("dwStartUpScan"), NULL, dwScanType, true))
	{
		AddLogEntry(L"### Failed to setScanType SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
	}
	
	OnOK();
}

/***************************************************************************************************                    
*  Function Name  : OnBnClickedButtonCancel                                                     
*  Description    : Closes scan type dialog
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 9th June 2014
****************************************************************************************************/
void CSettingsScanTypeDlg::OnBnClickedButtonCancel()
{
	OnCancel();
}

/***********************************************************************************************
*  Function Name  : ReadRegistryEntryofSetting
*  Description    : this function is  called for Reading the Registry entries from the Registry
*  Author Name    : Nitin Kolapkar
*  SR_NO		  :
*  Date           : 29 May 2014
***********************************************************************************************/
DWORD CSettingsScanTypeDlg::ReadRegistryEntryofSetting()
{
	//In Setting->Scan->Scan computer at start up->if i select full scan option then it shows Quick scan ,It should be full scan. 
	//Niranjan Deshak - 11/03/2015.
	//CString strKey = L"StartupScan";
	CString strKey = L"dwStartUpScan";
	
	//CString m_Key = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
	CString m_Key = L"SOFTWARE\\Wardwiz Antivirus";

	TCHAR value[256] = {0};
	DWORD dwType = REG_SZ;
	DWORD returnDWORDValue;
	DWORD dwSize = sizeof(value);
	HKEY hKey;
	returnDWORDValue = 0;
	LONG lResult = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, m_Key, 0,KEY_READ, &hKey);
	if(lResult == ERROR_SUCCESS)
		lResult = ::RegQueryValueEx(hKey, strKey, NULL, &dwType, (LPBYTE)&returnDWORDValue, &dwSize);
		if(lResult == ERROR_SUCCESS)
		{	
			::RegCloseKey(hKey);
			return (DWORD)returnDWORDValue;
		}
		else 
		{
			::RegCloseKey(hKey);
			return NULL;
		}

	return NULL;
}

// reslove by lalit 1-14-2015 ,issue- handling of startup-scan from service, to resolve the issue of restart now popup coming frequently even after restart in xp
/***************************************************************************************************                    
*  Function Name  : SendRegistryData2Service                                                     
*  Description    : Send registry data to service to create or save value in registry.
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 9th June 2014
****************************************************************************************************/
bool CSettingsScanTypeDlg::SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPTSTR szData, DWORD dwData, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	
	szPipeData.iMessageInfo = WRITE_REGISTRY;
	szPipeData.dwValue = dwType; 

	wcscpy_s(szPipeData.szFirstParam, szKey);
	wcscpy_s(szPipeData.szSecondParam, szValue);

	if(dwType == SZ_DWORD)
	{
		szPipeData.dwSecondValue = dwData;
	}
	if(szData)
	{
		wcscpy_s(szPipeData.szThirdParam, szData);
	}
	CISpyCommunicator objCom(SERVICE_SERVER, true);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CGenXLiveUpSecondDlg : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bWait)
	{
		if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CGenXLiveUpSecondDlg : SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}
/***************************************************************************************************                    
*  Function Name  : GetWardWizPathFromRegistry                                                     
*  Description    : Get client application folder path
*  Author Name    : Nitin K. Kolapkar 
*  SR_NO
*  Date           : 9th June 2014
****************************************************************************************************/
/*	ISSUE NO - 750 NAME - NITIN K. TIME - 17th June 2014 */
CString CSettingsScanTypeDlg::GetWardWizPathFromRegistry( )
{
	HKEY	hSubKey = NULL ;
	TCHAR	szModulePath[MAX_PATH] = {0};

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Wardwiz Antivirus"), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
		return L"";

	DWORD	dwSize = 511 ;
	DWORD	dwType = 0x00 ;

	RegQueryValueEx(hSubKey, L"AppFolder", 0, &dwType, (LPBYTE)szModulePath, &dwSize) ;
	RegCloseKey( hSubKey ) ;
	hSubKey = NULL ;

	if(_tcslen(szModulePath) > 0)
	{
		return CString(szModulePath) ;
	}
	return L"";
}
