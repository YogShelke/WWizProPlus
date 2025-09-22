/**********************************************************************************************************                     
	  Program Name          : ISpyGUIDlg.cpp
	  Description           : Main Dialog for IspyGUI
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 18th Sep 2013
	  Version No            : 0.0.0.1
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Ramkrushna           Created ISpy GUI using GDI objects       11 - 11 - 2013              
***********************************************************************************************************/
#include "stdafx.h"
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"
#include "USBPopupMsgDlg.h"
#include "afxwin.h"
//#include "CrashRpt.h"
#include "Enumprocess.h"
//#include "CSecure64.h"
#include "iTINRegWrapper.h"
#include "iSpySrvMgmt.h"
#include "CommonFunctions.h"
#include "DriverConstants.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/**********************************************************************************************************
*  Function Name  :	CAboutDlg
*  Description    :	C'tor
*  Author Name    : Ram
*  SR_NO		  :
*  Date           : 05 Mar 2015
**********************************************************************************************************/
class CAboutDlg : public CJpegDialog
{
public:
	CAboutDlg();

	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
protected:
	DECLARE_MESSAGE_MAP()
public:
	CColorStatic		m_stCopyrights;
	CColorStatic		m_stVersion;
	CStatic				m_bmpLogoPict;
	CxSkinButton		m_btnClose;

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonClose();
	void ReadDatabaseVersionFromRegistry();
	CButton m_BtnOK;
	afx_msg void OnBnClickedButtonAbtOk();
};

/***************************************************************************
  Function Name  : CAboutDlg()
  Description    : C'tor of about box
  Author Name    : Nitin
  SR_NO			 :
  Date           : 18th March 2014
****************************************************************************/
CAboutDlg::CAboutDlg() : CJpegDialog(CAboutDlg::IDD)
{
}

/***************************************************************************
  Function Name  : DoDataExchange()
  Description    : Called by the framework to exchange and validate dialog data.
  Author Name    : Nitin
  SR_NO			 :
  Date           : 18th March 2014
****************************************************************************/

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_COPYRIGHT, m_stCopyrights);
	DDX_Control(pDX, IDC_STATIC_VERSION, m_stVersion);
	DDX_Control(pDX, IDC_STATIC_LOGO_PICT, m_bmpLogoPict);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_BUTTON_ABT_OK, m_BtnOK);
}

/***************************************************************************
* Function Name  : MESSAGE_MAP
* Description    : Handle WM_COMMAND,WM_Messages,user defined message 
				   and notification message from child windows.
* Author Name    : Nitin
* SR_NO			 :
* Date           : 18th March 2014
****************************************************************************/
BEGIN_MESSAGE_MAP(CAboutDlg, CJpegDialog)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CAboutDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_BUTTON_ABT_OK, &CAboutDlg::OnBnClickedButtonAbtOk)
END_MESSAGE_MAP()


/***************************************************************************
  Function Name  : OnInitDialog
  Description    : Initialize the about dialog box's controls.
  Author Name    : Nitin
  SR_NO			 :
  Date           : 18th March 2014
****************************************************************************/

/*	ISSUE NO - 77 NAME - NITIN K. TIME - 22st May 2014 */
BOOL CAboutDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_PASSWORD_BG), _T("JPG")))
	{
		::MessageBox(NULL,theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
	}
	Draw();

	CRect rect1;
	this->GetClientRect(rect1);
	//SetWindowPos(&wndTop, rect1.top + 430,rect1.top + 260, rect1.Width()-2, rect1.Height()-2, SWP_NOREDRAW);
	
	CRgn rgn;
	rgn.CreateRectRgn(rect1.top, rect1.left ,rect1.Width()-3,rect1.Height()-3);
	this->SetWindowRgn(rgn, TRUE);

//	m_bmpLogoPict.SetWindowPos(&wndTop,rect1.left + 25,60,20,20,SWP_NOREDRAW);
	m_bmpLogoPict.ShowWindow(SW_HIDE);

	//Nitin K. 2nd June 2014
	/*	ISSUE NO - 77 NAME - NITIN K. TIME - 10th June 2014 */
	m_stVersion.SetFont(&theApp.m_fontWWTextNormal);
	//Issue:4. In taskbar,if we right click "WardWiz"->Select "About WardWiz" it shows only database version not product version.
	//Resolved By: Nitin K Date: 25th-Feb-2015
	m_stVersion.SetWindowPos(&wndTop, rect1.left + 115, 75, 250, 50, SWP_NOREDRAW);
	m_stVersion.SetBkColor(RGB(255,255,255));

	m_stCopyrights.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_COPYRIGHT"));
	m_stCopyrights.SetFont(&theApp.m_fontWWTextNormal);
	m_stCopyrights.SetWindowPos(&wndTop, rect1.left + 90, 135, 250, 20, SWP_NOREDRAW);
	m_stCopyrights.SetBkColor(RGB(255,255,255));

	ReadDatabaseVersionFromRegistry();

	m_BtnOK.SetWindowPos(&wndTop, rect1.left + 160, 160, 70, 25, SWP_NOREDRAW);

	m_btnClose.SetSkin(theApp.m_hResDLL,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSE,IDB_BITMAP_CLOSEOVER,IDB_BITMAP_CLOSE,0,0,0,0,0);
	m_btnClose.SetWindowPos(&wndTop,rect1.left + 348,0,26,17,SWP_NOREDRAW);
	
	return TRUE;  // return TRUE unless you set the focus to a control
}

/***************************************************************************
  Function Name  : OnBnClickedButtonClose
  Description    : Close About Dialog
  Author Name    : Nitin K.
  SR_NO			 :
  Date           : 2nd June 2014
****************************************************************************/
void CAboutDlg::OnBnClickedButtonClose()
{
	OnCancel();
}

/**************************************************************************************
  Function Name  : ReadDatabaseVersionFromRegistry
  Description    : Read Email Scan Entry from registry show controls on main Ui accordingly
  Author Name    : Nitin K.
  SR_NO			 :
  Date           : 24th April 2014
****************************************************************************************/
void CAboutDlg::ReadDatabaseVersionFromRegistry()
{
	HKEY key;
	long ReadReg;
	TCHAR szvalueVersion[1024];
	DWORD dwvaluelengthVersion = 1024;
	DWORD dwtypeVersion=REG_SZ;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Wardwiz Antivirus"), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &key) != ERROR_SUCCESS)
	{
		return;
	}
	ReadReg=RegQueryValueEx(key, L"AppVersion", NULL ,&dwtypeVersion,(LPBYTE)&szvalueVersion, &dwvaluelengthVersion);
	if(ReadReg == ERROR_SUCCESS)
	{
		//Issue:4. In taskbar,if we right click "WardWiz"->Select "About WardWiz" it shows only database version not product version.
		//Resolved By: Nitin K Date: 25th-Feb-2015
		CString stMessage, stMessageDBVersion;
		CString stVersion;
		stVersion = (LPCTSTR)szvalueVersion;
		stMessage.Format(L"%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ABOUTGUI_VERSION"), stVersion);
		ReadReg = RegQueryValueEx(key, L"DataBaseVersion", NULL, &dwtypeVersion, (LPBYTE)&szvalueVersion, &dwvaluelengthVersion);
		if (ReadReg == ERROR_SUCCESS)
		{
			stVersion = (LPCTSTR)szvalueVersion;
			stMessageDBVersion.Format(L"\n\n%s %s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ABOUTGUI_DBVERSION "), stVersion);
			stMessage.Append(stMessageDBVersion);
		}
		m_stVersion.SetWindowTextW(stMessage);
	}
	
}


CISpyGUIDlg * CISpyGUIDlg::m_pThis = NULL;
CISpyCommunicatorServer  g_objCommServer(UI_SERVER, CISpyGUIDlg::OnDataReceiveCallBack, sizeof(ISPY_PIPE_DATA));

/***************************************************************************
  Function Name  : CISpyGUIDlg
  Description    : C'tor
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
CISpyGUIDlg::CISpyGUIDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CISpyGUIDlg::IDD, pParent)
	//, m_DaysText(NULL)
	//, m_NoDaysText(NULL)
	//, m_buttonText(NULL)
	, m_bIsRegister(true)
	,m_pHomepageDlg(NULL)
	,m_pQuickscanDlg(NULL)
	,m_pFullScanDlg(NULL)
	,m_pCustomScanDlg(NULL)
	,m_pRegistryOptimizer(NULL)
	,m_pDataEncryption(NULL)
	,m_pRecover(NULL)
	,m_pFolderLocker(NULL)
	,m_pVirusscan(NULL)
	,m_pSpamFilter(NULL)
	,m_pContentFilter(NULL)
	,m_pSignature(NULL)
	,m_pAntirootkitScan(NULL)
	,m_pUpdate(NULL)
	,m_pReports(NULL)
	,m_pUtility(NULL)
	,m_stScan(NULL)
	,m_stTool(NULL)
	,m_stEmailScan(NULL)
	,m_btnInitialHome(NULL)
	,m_btnQuickScan(NULL)
	,m_btnFullScan(NULL)
	,m_btnCustomScan(NULL)
	,m_btnRegistryOptimizer(NULL)
	,m_btnDataEncryption(NULL)
	,m_btnRecover(NULL)
	,m_btnFolderLocker(NULL)
	,m_btnVirusScan(NULL)
	,m_btnSpamFilter(NULL)
	,m_btnContentFilter(NULL)
	,m_btnSignature(NULL)
	,m_btnAntirootkitScan(NULL)
	,m_btnUpdateNew(NULL)
	,m_btnReports(NULL)
	,m_btnUtility(NULL)
	,m_stAdminstration(NULL)
	,m_bStartUpScan(false)
	,m_bUnRegisterProduct(false)
	,m_pobjSettingsDlg(NULL)
	,m_pTabDialog(NULL)
	,m_bIsDownloading(false)
	, m_iRunningProcessNmB(0)
	, m_hCloseUIMutexHandle(NULL)
	, m_bIsCloseHandleCalled(false)
	, m_bisUIcloseRquestFromTray(false)
	, m_bisUiCloseCalled(false)
	, m_bIsUiTerminated(false)
	, m_bIsTabMenuClicked(false)
	, m_bIsPopUpDisplayed(false)
{
	m_pThis = this;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

/***************************************************************************
  Function Name  : ~CISpyGUIDlg
  Description    : D'tor
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
CISpyGUIDlg::~CISpyGUIDlg()
{
	//if(m_DaysText != NULL)
	//{
	//	delete m_DaysText;
	//	m_DaysText = NULL;
	//}
	
	//if(m_buttonText != NULL)
	//{
	//	delete m_buttonText;
	//	m_buttonText = NULL;
	//}

	//if(m_NoDaysText != NULL)
	//{
	//	delete m_NoDaysText;
	//	m_NoDaysText = NULL;
	//}
	if(m_pTabDialog)
	{
		delete m_pTabDialog;
		m_pTabDialog = NULL;
	}

	CScanDlg::ResetInstance();

	if(m_btnFullScan != NULL)
	{
		delete m_btnFullScan;
		m_btnFullScan = NULL;
	}
	
	if(m_pFullScanDlg != NULL)
	{
		//delete m_pFullScanDlg;
		m_pFullScanDlg = NULL;
	}

	if(m_btnQuickScan != NULL)
	{
		delete m_btnQuickScan;
		m_btnQuickScan = NULL;
	}

	if(m_pQuickscanDlg !=NULL)
	{
		//delete m_pQuickscanDlg;
		m_pQuickscanDlg = NULL;
	}

	if(m_btnCustomScan !=NULL)
	{
		delete m_btnCustomScan;
		m_btnCustomScan = NULL;
	}

	if(m_pCustomScanDlg !=NULL)
	{
		//delete m_pCustomScanDlg;
		m_pCustomScanDlg = NULL;
	}

	if(m_stScan != NULL)
	{
		delete m_stScan;
		m_stScan = NULL;
	}

	if(m_stTool !=NULL)
	{
		delete m_stTool;
		m_stTool = NULL;
	}

	if(m_stEmailScan != NULL)
	{
		delete m_stEmailScan;
		m_stEmailScan = NULL;
	}

	if(m_stAdminstration != NULL)
	{
		delete m_stAdminstration;
		m_stAdminstration = NULL;
	}

	if(m_btnRegistryOptimizer != NULL)
	{
		delete m_btnRegistryOptimizer;
		m_btnRegistryOptimizer = NULL;
	}

	if(m_pRegistryOptimizer != NULL)
	{
		delete m_pRegistryOptimizer;
		m_pRegistryOptimizer = NULL;
	}

	if(m_btnDataEncryption != NULL)
	{
		delete m_btnDataEncryption;
		m_btnDataEncryption = NULL;
	}

	if(m_pDataEncryption != NULL)
	{
		delete m_pDataEncryption;
		m_pDataEncryption = NULL;
	}

	if(m_btnRecover != NULL)
	{
		delete m_btnRecover;
		m_btnRecover = NULL;
	}

	if(m_pRecover != NULL)
	{
		delete m_pRecover;
		m_pRecover = NULL;
	}

	if(m_btnFolderLocker != NULL)
	{
		delete m_btnFolderLocker;
		m_btnFolderLocker = NULL;
	}

	if(m_pFolderLocker != NULL)
	{
		delete m_pFolderLocker;
		m_pFolderLocker = NULL;
	}
	
	if(m_btnVirusScan !=NULL)
	{
		delete m_btnVirusScan;
		m_btnVirusScan = NULL;
	}

	CISpyEmailScanDlg::ResetInstance();

	if(m_pVirusscan !=NULL)
	{
		//delete m_pVirusscan;
		m_pVirusscan = NULL;
	}

	if(m_btnSpamFilter !=NULL)
	{
		delete m_btnSpamFilter;
		m_btnSpamFilter = NULL;
	}
	
	if(m_pSpamFilter !=NULL)
	{
		//delete m_pSpamFilter;
		m_pSpamFilter = NULL;
	}

	if(m_btnContentFilter !=NULL)
	{
		delete m_btnContentFilter;
		m_btnContentFilter = NULL;
	}

	if(m_pContentFilter !=NULL)
	{
		//delete m_pContentFilter;
		m_pContentFilter = NULL;
	}
	
	if(m_btnSignature !=NULL)
	{
		delete m_btnSignature;
		m_btnSignature = NULL;
	}

	if(m_pSignature !=NULL)
	{
		//delete m_pSignature;
		m_pSignature = NULL;
	}
	
	if(m_btnAntirootkitScan != NULL)
	{
		delete m_btnAntirootkitScan;
		m_btnAntirootkitScan = NULL;
	}
	
	if(m_pAntirootkitScan != NULL)
	{
		delete m_pAntirootkitScan;
		m_pAntirootkitScan = NULL;
	}
	
	if(m_btnUpdateNew != NULL)
	{
		delete m_btnUpdateNew;
		m_btnUpdateNew = NULL;
	}
	if(m_pUpdate != NULL)
	{
		delete m_pUpdate;
		m_pUpdate = NULL;
	}
	if(m_btnReports != NULL)
	{
		delete m_btnReports;
		m_btnReports = NULL;
	}

	if (m_btnUtility != NULL)
	{
		delete m_btnUtility;
		m_btnUtility = NULL;
	}
	if(m_pReports != NULL)
	{
		delete m_pReports;
		m_pReports = NULL;
	}

	if (m_pUtility != NULL)
	{
		delete m_pUtility;
		m_pUtility = NULL;
	}

	if(m_btnInitialHome != NULL)
	{
		delete m_btnInitialHome;
		m_btnInitialHome = NULL;
	}

	if(m_pHomepageDlg != NULL)
	{
		delete m_pHomepageDlg;
		m_pHomepageDlg = NULL;
	}
	
	if(m_pobjSettingsDlg != NULL)
	{
		delete m_pobjSettingsDlg;
		m_pobjSettingsDlg  = NULL;
	}
}

/***************************************************************************
  Function Name  : DoDataExchange
  Description    : MFC framework's DDX mechanism transfers the 
				   values of the member variables to the controls in the dialog box 
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
void CISpyGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DLGPOS, m_dlgPos);
	DDX_Control(pDX, IDC_BUTTON_HOME, m_btnHome);
	DDX_Control(pDX, IDC_BUTTON_UPDATE, m_btnUpdate);
	DDX_Control(pDX, IDC_BUTTON_HELP, m_btnHelp);
	DDX_Control(pDX, IDC_BUTTON_MINIMIZE, m_btnMinimize);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_btnClose);
	//DDX_Control(pDX, IDC_BUTTON_SCANPAGE, m_btnScanDlg);
	//DDX_Control(pDX, IDC_BUTTON_TOOLS, m_btnTools);
	//DDX_Control(pDX, IDC_BUTTON_USBDETECT, m_btnUsbDetect);
	//DDX_Control(pDX, IDC_BUTTON_REPORTS, m_btnReports);
	//DDX_Control(pDX, IDC_BUTTON_UPDATES, m_btnUpdates);
	//DDX_Control(pDX, IDC_BUTTON_EMAILSCAN, m_btnEmailScan);
	//DDX_Control(pDX, IDC_BUTTON_FIREWALL, m_btnFirewall);
	//DDX_Control(pDX, IDC_BUTTON_ANTIROOTKIT, m_btnAntRootkit);
	//DDX_Control(pDX, IDC_STATIC_LIVEUPDATE_DATETIME, m_stLiveUpdateDateTime);
	//DDX_Control(pDX, IDC_STATIC_LASTUPADTE, m_stUpDateTime);
	//DDX_Control(pDX, IDC_STATIC_MAINUI_HEADER_PIC, m_stMainUIHeaderPic);
	//DDX_Control(pDX, IDC_STATIC_MAINUI_NONPROTECT, m_stHeaderNonProtect);
	DDX_Control(pDX, IDC_BUTTON_FACEBOOK, m_btnFacebook);
	DDX_Control(pDX, IDC_BUTTON_TWITTER, m_btnTwitter);
	//DDX_Control(pDX, IDC_BUTTON_PROTECT, m_btnProtect);
	//DDX_Control(pDX, IDC_BUTTON_NONPROTECT, m_btnNonProtect);
	//DDX_Control(pDX, IDC_STATIC_LSTSCANDATE, m_stLastScan);
	//DDX_Control(pDX, IDC_STATIC_LASTSCANDT, m_stLastScanDateTime);
	//DDX_Control(pDX, IDC_STATIC_VERSION, m_stVersion);
	//DDX_Control(pDX, IDC_STATIC_VERSION_NO, m_stVersionNo);
	//DDX_Control(pDX, IDC_STATIC_LASTTYPE, m_stLastScanType);
	//DDX_Control(pDX, IDC_STATIC_SCANTYPE, m_stScanType);
	//DDX_Control(pDX, IDC_STATIC_UPDATE_TIME, m_stUpdateTime);
	//DDX_Control(pDX, IDC_STATIC_MAINUI_NONPROTECTGIF, m_bmpShowUnProtectedGif);
	DDX_Control(pDX, IDC_STATIC_FOOTER, m_stFooterMsg);
	//DDX_Control(pDX, IDC_STATIC_REGISTERVERSION, m_stRegVersion);
	//DDX_Control(pDX, IDC_STATIC_TRIALVERSION, m_stTrialVersion);
	//DDX_Control(pDX, IDC_BUTTON_REGISTERNOW, m_btnRegisterNow);
	//DDX_Control(pDX, IDC_STATIC_DAYSLEFT_OUT, m_stDayLeftOutPic);
	//DDX_Control(pDX, IDC_STATIC_DAYLEFTTEXT, m_stDaysLefttext);
	//DDX_Control(pDX, IDC_STATIC_NOOFDAYS, m_stNoOfDays);
	//DDX_Control(pDX, IDC_STATIC_REGISTER, m_stRegisterNowHeaderMsg);
	//DDX_Control(pDX, IDC_STATIC_CLICKONTEXT, m_stClickOnText);
	//DDX_Control(pDX, IDC_STATIC_NONPROTECTTEXT, m_stNonProtectedText);
	//DDX_Control(pDX, IDC_STATIC_SECURED, m_stSecureMsg);
	DDX_Control(pDX, IDC_BUTTON_MAXIMIZE, m_btnmaximize);
	DDX_Control(pDX, IDC_STATIC_LOGO_PIC, m_stLogoPic);
	//DDX_Control(pDX, IDC_STATIC_LOGOPIC, m_stLogoPicture);
	//DDX_Control(pDX, IDC_STATIC_LOGO_PIC_NEW, m_stlogopicnew);
	DDX_Control(pDX, IDC_BUTTON_LINKEDIN, m_btnLinkedIn);
	DDX_Control(pDX, IDC_BUTTON_INFORMATION, m_btnProductInformation);
	DDX_Control(pDX, IDC_BUTTON_SUPPORT, m_btnSupport);
}


/***************************************************************************
  Function Name  : MESSAGE_MAP
  Description    : Handle WM_COMMAND,WM_Messages,user defined message 
				   and notification message from child windows.
  Author Name    : Neha Gharge
  SR_NO			 :
  Date           : 22nd Nov 2013
  Modified Date	 : 18th Feb 2014
****************************************************************************/
BEGIN_MESSAGE_MAP(CISpyGUIDlg, CJpegDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_MINIMIZE, &CISpyGUIDlg::OnBnClickedButtonMinimize)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CISpyGUIDlg::OnBnClickedButtonClose)
	ON_WM_SETCURSOR()
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, &CISpyGUIDlg::OnBnClickedButtonUpdate)
	//ON_BN_CLICKED(IDC_BUTTON_SCANPAGE, &CISpyGUIDlg::OnBnClickedButtonScanpage)
	//ON_BN_CLICKED(IDC_BUTTON_TOOLS, &CISpyGUIDlg::OnBnClickedButtonTools)
	//ON_BN_CLICKED(IDC_BUTTON_REPORTS, &CISpyGUIDlg::OnBnClickedButtonReports)
	//ON_BN_CLICKED(IDC_BUTTON_UPDATES, &CISpyGUIDlg::OnBnClickedButtonUpdates)
	ON_BN_CLICKED(IDC_BUTTON_HOME, &CISpyGUIDlg::OnBnClickedButtonHome)
	//ON_BN_CLICKED(IDC_BUTTON_FIREWALL, &CISpyGUIDlg::OnBnClickedButtonFirewall)
	//ON_BN_CLICKED(IDC_BUTTON_USBDETECT, &CISpyGUIDlg::OnBnClickedButtonUsbdetect)
	//ON_BN_CLICKED(IDC_BUTTON_NONPROTECT, &CISpyGUIDlg::OnBnClickedButtonNonprotect)
	//ON_BN_CLICKED(IDC_BUTTON_PROTECT, &CISpyGUIDlg::OnBnClickedButtonProtect)
	ON_BN_CLICKED(IDC_BUTTON_FACEBOOK, &CISpyGUIDlg::OnBnClickedButtonFacebook)
	ON_BN_CLICKED(IDC_BUTTON_TWITTER, &CISpyGUIDlg::OnBnClickedButtonTwitter)
	//ON_BN_CLICKED(IDC_BUTTON_EMAILSCAN, &CISpyGUIDlg::OnBnClickedButtonEmailscan)
	//ON_BN_CLICKED(IDC_BUTTON_ANTIROOTKIT, &CISpyGUIDlg::OnBnClickedButtonAntirootkit)
	ON_BN_CLICKED(IDC_BUTTON_HELP, &CISpyGUIDlg::OnBnClickedButtonHelp)
	//ON_BN_CLICKED(IDC_BUTTON_REGISTERNOW, &CISpyGUIDlg::OnBnClickedButtonRegisternow)
	ON_MESSAGE(RELOAD_EMAILVIRUSCANDB, OnUserMessages)
	ON_MESSAGE(CHECKEMAILSCANSTATUS , OnUserChangeEmailScanSetting)
	//ON_MESSAGE(CHECKLIVEUPDATE , OnLiveUpdateTrayCall)/* ISSUE: LiveUpdate Tray Notification NAME - NITIN K. TIME - 25th July 2014 */
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_INFORMATION, &CISpyGUIDlg::OnBnClickedButtonInformation)
	ON_MESSAGE(CHECKSCROLLINGTEXT, OnUserRequestToDisplayThreatNews)
	ON_BN_CLICKED(IDC_BUTTON_LINKEDIN, &CISpyGUIDlg::OnBnClickedButtonLinkedin)
	ON_WM_SIZE()
	ON_WM_NCCREATE()
	ON_BN_CLICKED(IDC_BUTTON_SUPPORT, &CISpyGUIDlg::OnBnClickedButtonSupport)
	ON_MESSAGE(LAUNCHPRODEXPMSGBOX, OnUserMessagesLaunchProdExpMsgBox)
END_MESSAGE_MAP()

/***************************************************************************
  Function Name  : OnInitDialog
  Description    : Initialize the dialog box's controls.
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
BOOL CISpyGUIDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	//CSecure64  objCSecure;
	//objCSecure.RegisterProcessId(WLSRV_ID_ZERO);//WLSRV_ID_ZERO to register service for process protection

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		//strAboutMenu.LoadString(IDS_ABOUTBOX);
		strAboutMenu.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_ABOUT_WARDWIZ"));
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

	ModifyStyle(0, WS_SYSMENU | WS_MINIMIZEBOX);

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window
#ifdef ISPYTRIAL 
	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_MAINSCREEN), _T("JPG")))
	{
		::MessageBox(NULL,theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
	}
#elif ITINSILVER
	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_MAINSCREEN), _T("JPG")))
	{
		::MessageBox(NULL,theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
	}
#else
	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_MAINSCREEN), _T("JPG")))
	{
		::MessageBox(NULL,theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
	}
#endif

	Draw();

	//run the pipe here
	g_objCommServer.Run();
	
	CRect rect;
	this->GetClientRect(m_rect);
	CRgn		 rgn;
	rgn.CreateRoundRectRgn(m_rect.left, m_rect.top, m_rect.right-4, m_rect.bottom-2,0,0);
	this->SetWindowRgn(rgn, TRUE);

	this->SetWindowText(L"WRDWIZAVUI");
	InitTabDialog();

	//Lauch System Tray here
	StartiSpyAVTray();


	

	//m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));
	m_hButtonCursor = AfxGetApp()->LoadStandardCursor(IDC_HAND);

	m_bmpLogoPic = LoadBitmap(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_LOGO_PIC)); 	 
	m_stLogoPic.SetBitmap(m_bmpLogoPic);
	m_stLogoPic.SetWindowPos(&wndTop, m_rect.left +0,m_rect.top -1,305,97, SWP_NOREDRAW);
	
 
	/*****************ISSUE NO -179 Neha Gharge 22/5/14 ************************************************/

	m_btnHome.SetSkin(theApp.m_hResDLL,IDB_BITMAP_HOME, IDB_BITMAP_HOME, IDB_BITMAP_HOMEMOVER,0,0,0,0,0,0);
	m_btnHome.SetWindowPos(&wndTop, m_rect.left + 619, 60, 32,36, SWP_NOREDRAW);
	//m_btnHome.SetWindowPos(&wndTop, m_rect.left + 647, 50, 32,36, SWP_NOREDRAW);

	m_btnHelp.SetSkin(theApp.m_hResDLL,IDB_BITMAP_SETTING, IDB_BITMAP_SETTING, IDB_BITMAP_SETTINGMOVER,0,0,0,0,0,0);
	m_btnHelp.SetWindowPos(&wndTop, m_rect.left +  651, 60, 32,36, SWP_SHOWWINDOW);

	m_btnProductInformation.SetSkin(theApp.m_hResDLL,IDB_BITMAP_INFO_NORMAL, IDB_BITMAP_INFO_NORMAL, IDB_BITMAP_H_INFO_MSG,0,0,0,0,0,0);
	m_btnProductInformation.SetWindowPos(&wndTop, m_rect.left +  683, 60, 32,36, SWP_SHOWWINDOW);

	m_btnUpdate.SetSkin(theApp.m_hResDLL,IDB_BITMAP_HELP, IDB_BITMAP_HELP, IDB_BITMAP_HELPMOVER,0,0,0,0,0,0);
	m_btnUpdate.SetWindowPos(&wndTop, m_rect.left + 715, 60, 32,36, SWP_SHOWWINDOW);
	

	

	m_btnMinimize.SetSkin(theApp.m_hResDLL,IDB_BITMAP_MINMIZE , IDB_BITMAP_MINMIZE, IDB_BITMAP_MINMIZEMOVER,0,0,0,0,0,0);
	//m_btnMinimize.SetWindowPos(&wndTop, m_rect.left + 664, 0, 26,17, SWP_NOREDRAW);
	m_btnMinimize.SetWindowPos(&wndTop, m_rect.left + 690, 0, 26,17, SWP_NOREDRAW);

	m_btnmaximize.SetSkin(theApp.m_hResDLL,IDB_BITMAP_MAXIMIZE ,IDB_BITMAP_MAXIMIZE ,IDB_BITMAP_H_MAXIMIZE,0,0,0,0,0,0);
	m_btnmaximize.SetWindowPos(&wndTop, m_rect.left + 690, 0, 26,17, SWP_HIDEWINDOW);

	m_btnClose.SetSkin(theApp.m_hResDLL,IDB_BITMAP_CLOSE, IDB_BITMAP_CLOSE, IDB_BITMAP_CLOSEOVER,IDB_BITMAP_CLOSE,0,0,0,0,0);
	m_btnClose.SetWindowPos(&wndTop, m_rect.left + 716, 0, 26,17, SWP_NOREDRAW);
	

	
	LOGFONT lfInstallerTitle;  
	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfHeight = 15;
	lfInstallerTitle.lfWeight = FW_BOLD;
	lfInstallerTitle.lfWidth = 6;
	m_BoldText.CreateFontIndirect(&lfInstallerTitle);
	wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Microsoft Sans serif Regular"));	 //	   with	face name "Verdana".

	m_btnFacebook.SetSkin(theApp.m_hResDLL,IDB_BITMAP_FB,IDB_BITMAP_FB,IDB_BITMAP_H_FB,IDB_BITMAP_FB,0,0,0,0,0);
	m_btnFacebook.SetWindowPos(&wndTop, m_rect.left + 694, 508,14,14, SWP_NOREDRAW);

	m_btnTwitter.SetSkin(theApp.m_hResDLL,IDB_BITMAP_TWT,IDB_BITMAP_TWT,IDB_BITMAP_H_TWT,IDB_BITMAP_TWT,0,0,0,0,0);
	m_btnTwitter.SetWindowPos(&wndTop,m_rect.left+711,508,14,14, SWP_NOREDRAW);

	m_btnLinkedIn.SetSkin(theApp.m_hResDLL,IDB_BITMAP_LINKEDIN,IDB_BITMAP_LINKEDIN,IDB_BITMAP_H_LINKEDIN,IDB_BITMAP_LINKEDIN,0,0,0,0,0);
	m_btnLinkedIn.SetWindowPos(&wndTop,m_rect.left+728,508,14,14, SWP_NOREDRAW);

	m_stFooterMsg.SetWindowPos(&wndTop,m_rect.left+20,508,320,24, SWP_NOREDRAW);
	m_stFooterMsg.SetTextColor(RGB(75,75,76));
	m_stFooterMsg.SetFont(&m_BoldText);

	//New Implementation : Added Support Dialog Window in product
	//Added by : Nitin K. Date: 11th May 2015
	m_btnSupport.SetSkin(theApp.m_hResDLL, IDB_BITMAP_SUPPORT, IDB_BITMAP_SUPPORT, IDB_BITMAP_SUPPORT_HOVER, IDB_BITMAP_SUPPORT, 0, 0, 0, 0, 0);
	m_btnSupport.SetWindowPos(&wndTop, m_rect.left + 600, 501, 85, 28, SWP_NOREDRAW);
	m_btnSupport.SetFont(&m_BoldText);
	//m_btnSupport.SetWindowTextW(L"Support");
	m_btnSupport.SetTextColorA(RGB(75, 75, 76), 1, RGB(193,33,48));
	
	//Issue: Tooltip added for all buttons on Main UI
	//Added By : Nitin K Date: 23rd May 2015
	m_btnHome.SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_HOMEPAGE_TOOLTIP_HOME"));
	m_btnHelp.SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_HOMEPAGE_TOOLTIP_SETTINGS"));
	m_btnProductInformation.SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_HOMEPAGE_TOOLTIP_PROD_INFO"));
	m_btnUpdate.SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_HOMEPAGE_TOOLTIP_HELP"));
	m_btnFacebook.SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_HOMEPAGE_TOOLTIP_FACEBOOK"));
	m_btnTwitter.SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_HOMEPAGE_TOOLTIP_TWITTER"));
	m_btnLinkedIn.SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_HOMEPAGE_TOOLTIP_LINKEDIN"));
	m_btnSupport.SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_HOMEPAGE_TOOLTIP_SUPPORT"));
	
	if (theApp.m_eSetupLocId == WARDWIZGERMANY)
	{
		theApp.ShowProdExpiredUnRegisteredMsgBox();
	}
	//CString csCommandLine =  GetCommandLine();
	//if(csCommandLine.Find('-') != -1)
	//{
	//	csCommandLine.Delete(0, csCommandLine.Find('-') + 1);
	//	csCommandLine.Trim();
	//	if(csCommandLine.GetLength() > 0)
	//	{
	//		if( csCommandLine.CompareNoCase(TEXT("STARTUPSCAN")) >= 0 )
	//		{
	//			m_bStartUpScan = true;
	//		}
	//	}
	//}
	if(theApp.m_bStartUpScan)
	{
		if(theApp.m_bRunFullScan)
		{
			if(m_pHomepageDlg != NULL)
			{
				m_pHomepageDlg->ShowWindow(SW_HIDE);
			}
			if(m_btnQuickScan != NULL)
			{
				//m_btnQuickScan->EnableWindow(false);
			}
			if(m_btnCustomScan != NULL)
			{
				//m_btnCustomScan->EnableWindow(false);
			}
			if(m_btnAntirootkitScan != NULL)
			{
				//m_btnAntirootkitScan->EnableWindow(false);
			}
			if(m_pFullScanDlg != NULL)
			{
				theApp.m_bRunFullScan = false;
				m_pFullScanDlg->ShowWindow(SW_SHOW);
				m_pFullScanDlg->OnBnClickedBtnFullscan();
				m_pFullScanDlg->m_eScanType = FULLSCAN;
				m_pQuickscanDlg->m_eCurrentSelectedScanType = FULLSCAN;
				m_pFullScanDlg->StartScanning();
				m_btnFullScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
				m_btnFullScan->SetTextColorA(RGB(4, 4, 4), 0, 1);
				m_btnFullScan->SetFocusTextColor(RGB(4, 4, 4));
			}
			Invalidate();
		}
		else 
		{
			if(m_pHomepageDlg != NULL)
			{
				m_pHomepageDlg->ShowWindow(SW_HIDE);
			}
			if(m_btnFullScan != NULL)
			{
				//m_btnFullScan->EnableWindow(false);
			}
			if(m_btnCustomScan != NULL)
			{
				//m_btnCustomScan->EnableWindow(false);
			}
			if(m_btnAntirootkitScan != NULL)
			{
				//m_btnAntirootkitScan->EnableWindow(false);
			}
			if(m_pQuickscanDlg != NULL)
			{
				theApp.m_bRunQuickScan = false;
				m_pQuickscanDlg->ShowWindow(SW_SHOW);
				m_pQuickscanDlg->OnBnClickedBtnQuickscan();
				m_pQuickscanDlg->m_eScanType = QUICKSCAN;
				// resolve by lalit 5-5-2015, issue- suppose full scan is running and click on custom/quick scan 
				//then"current scan type getting set as custom and aborted and custom scan completed message comming instead of full scan abort"
				m_pQuickscanDlg->m_eCurrentSelectedScanType = QUICKSCAN;
				m_pQuickscanDlg->StartScanning();
				m_btnQuickScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
				m_btnQuickScan->SetTextColorA(RGB(4, 4, 4), 0, 1);
				m_btnQuickScan->SetFocusTextColor(RGB(4, 4, 4));

				//m_btnQuickScan->SetFocusTextColor(RGB(4, 4, 4));
			}
			Invalidate();
		}

		theApp.m_bStartUpScan = false;
	}

	if(theApp.m_bRunLiveUpdate)
	{
		if(m_pHomepageDlg != NULL)
		{
			m_pHomepageDlg->ShowWindow(SW_HIDE);
		}
		m_pUpdate->ShowWindow(SW_SHOW);
		m_bIsDownloading = true;
		m_pUpdate->m_updateType = UPDATEFROMINTERNT;
		m_pUpdate->OnBnClickedButtonNext();
		
		m_btnUpdateNew->SetSkin(theApp.m_hResDLL, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnUpdateNew->SetTextColorA(RGB(4, 4, 4), 0, 1);
		m_btnUpdateNew->SetFocusTextColor(RGB(4, 4, 4));
	}

	if (theApp.m_bDataCryptOpr)
	{
		ShowDataCryptOpr();
	}
	UpdateWindow();
	return FALSE ;  // return TRUE  unless you set the focus to a control
}

/***************************************************************************
  Function Name  : OnSysCommand
  Description    : The framework calls this member function when the user selects 
				   a command from the Control menu, or when the user selects the 
				   Maximize or the Minimize button.
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
void CISpyGUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		m_bIsPopUpDisplayed = true;
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
		m_bIsPopUpDisplayed = false;
	}
	else
	{
		CJpegDialog::OnSysCommand(nID, lParam);
	}
}

/***************************************************************************
  Function Name  : OnPaint
  Description    : The framework calls this member function when Windows or an
				   application makes a request to repaint a portion of an application's window.
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
void CISpyGUIDlg::OnPaint()
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
		CJpegDialog::OnPaint();
	}
}

/***************************************************************************
  Function Name  : OnQueryDragIcon
  Description    : The framework calls this member function by a minimized
				   (iconic) window that does not have an icon defined for its class
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
HCURSOR CISpyGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/***************************************************************************
  Function Name  : OnBnClickedButtonMinimize
  Description    : function to minimize the window.
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
void CISpyGUIDlg::OnBnClickedButtonMinimize()
{
	this->ShowWindow(SW_MINIMIZE);
}

/***************************************************************************
  Function Name  : CloseUISafely
  Description    : function to close the UI instantly
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
void CISpyGUIDlg::CloseUISafely()
{
	exit(0);
}

/***************************************************************************
  Function Name  : OnCloseUI
  Description    : function to close the UI
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
void CISpyGUIDlg::OnCloseUI()
{
	OnCancel();
}

/***************************************************************************
  Function Name  : ShutDownScanningCloseUIThread
  Description    : thread to close the UI
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
UINT ShutDownScanningCloseUIThread(LPVOID lpThis)
{
	//all threads are over close UI safely here
	//pThis->OnCloseUI();

	return 1;
}

/***************************************************************************
  Function Name  : OnBnClickedButtonClose
  Description    : Close button click handler.
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
void CISpyGUIDlg::OnBnClickedButtonClose()
{
	//Neha Gharge 2/4/2015 Crash occur after OnCancel function
	//theApp.m_bOnCloseFromMainUI = true;
	// resolved by lalit 5-5-5-05, issue-if thread found in scan and i close from tray in such case handleClose  call properly but it again getting call from exit(0)- onClose()
	if (m_bIsUiTerminated)
	{
		return;
	}

	HandleCloseButton();
	
}

/***************************************************************************
  Function Name  : HandleCloseButton
  Description    : Handles all dialog process while closing from main UI. 
  Author Name    : Ramkrushna Shelke,Neha Gharge
  SR_NO			 :
  Date           : 18th Nov 2013
  Modified Date	 : 23rd Feb 2014
****************************************************************************/
bool CISpyGUIDlg::HandleCloseButton()
{
	bool bReturn = false;

	try
	{
		// lalit 4-28-2015 , if we close on stop button and same time exit from tray then two popup comes , now single popup will come
		if (CloseGUIHandleMutually())
		{
			m_bisUIcloseRquestFromTray = true;
			return false;
		}
		if (m_pHomepageDlg != NULL)
		{

			if (!m_pHomepageDlg->CheckRegistrationProcessStatus())
			{
				//Neha Gharge 2/4/2015 Crash occur after OnCancel function
				theApp.m_bOnCloseFromMainUI = false;
				m_bIsCloseHandleCalled = false;
				m_bisUIcloseRquestFromTray = false;
				return false;
			}

		}

		//Issue No: 1246  : "Registration process is in progress" popup is not appearing.
		//Resolved By: Nitin K. Date 21st Jan 2016
		if (m_pThis->IsAnyPopUpDisplayed())
		{
				HWND hWnd = m_pThis->GetSafeHwnd();
				::ShowWindow(hWnd, SW_RESTORE);
				::BringWindowToTop(hWnd);
				::SetForegroundWindow(hWnd);
			//g_objCommServer.SendResponse(lpSpyData);
			//Neha Gharge 2/4/2015 Crash occur after OnCancel function
			theApp.m_bOnCloseFromMainUI = false;
			m_bIsCloseHandleCalled = false;
			m_bisUIcloseRquestFromTray = false;
			return false;
		}
		if (m_pUpdate != NULL)
		{
			m_pUpdate->m_dlgISpyUpdatesSecond.m_bCloseUpdate = true;
			if (!m_pUpdate->m_dlgISpyUpdatesSecond.ShutDownDownload())
			{
				//Neha Gharge 2/4/2015 Crash occur after OnCancel function
				m_pUpdate->m_dlgISpyUpdatesSecond.m_bCloseUpdate = false;
				theApp.m_bOnCloseFromMainUI = false;
				m_bIsCloseHandleCalled = false;
				m_bisUIcloseRquestFromTray = false;
				return false;
			}
			else
			{
				m_pUpdate->m_dlgISpyUpdatesSecond.m_bCloseUpdate = true;
			}
		}

		if (m_pRegistryOptimizer != NULL)
		{
			m_pRegistryOptimizer->m_bClose = true;
			if (!m_pRegistryOptimizer->ShutDownScanning())
			{
				//Neha Gharge 2/5/2015 Message Box not appears
				m_pRegistryOptimizer->m_bClose = false;
				//Neha Gharge 2/4/2015 Crash occur after OnCancel function
				theApp.m_bOnCloseFromMainUI = false;
				m_bIsCloseHandleCalled = false;
				m_bisUIcloseRquestFromTray = false;
				return false;
			}
		}

		if (m_pReports != NULL)
		{
			m_pReports->m_bReportClose = true;

			if (!m_pReports->StopReportOperation())
			{
				//Neha Gharge 2/5/2015 Message Box not appears
				m_pReports->m_bReportClose = false;
				// Neha ghare Issue N0.681,742 17/06/2014
				m_bIsCloseHandleCalled = false;
				m_bisUIcloseRquestFromTray = false;
				return false;
			}
		}
		if (m_pRecover != NULL)
		{
			m_pRecover->m_bRecoverClose = true;
			if (!m_pRecover->StopReportOperation())
			{
				//Neha Gharge 2/5/2015 Message Box not appears
				m_pRecover->m_bRecoverClose = false;
				m_bIsCloseHandleCalled = false;
				m_bisUIcloseRquestFromTray = false;
				return false;
			}
		}

		if (m_pAntirootkitScan != NULL)
		{
			m_pAntirootkitScan->m_bAntirootClose = true;
			if (!m_pAntirootkitScan->ShutDownRootkitScanning())
			{
				//Neha Gharge 2/5/2015 Message Box not appears
				m_pAntirootkitScan->m_bAntirootClose = false;
				//Neha Gharge 2/4/2015 Crash occur after OnCancel function
				theApp.m_bOnCloseFromMainUI = false;
				m_bIsCloseHandleCalled = false;
				m_bisUIcloseRquestFromTray = false;
				return false;
			}

		}

		if (m_pAntirootkitScan != NULL)
		{

			m_pAntirootkitScan->m_bAntirootClose = true;
			if (!m_pAntirootkitScan->ShutDownRootkitCleaning())
			{
				//Neha Gharge 2/5/2015 Message Box not appears
				m_pAntirootkitScan->m_bAntirootClose = false;
				m_bIsCloseHandleCalled = false;
				m_bisUIcloseRquestFromTray = false;
				return false;
			}
		}

		//Rajil Yadav Issue No. 665 11/06/2014
		if ((m_pQuickscanDlg != NULL) && (m_pQuickscanDlg->m_virusFound > 0) && m_pQuickscanDlg->m_eScanType == QUICKSCAN && m_pQuickscanDlg->m_bIsCleaning == false)
		{
			//Issue No : 1171 Blank window appearing in Updates.
			HideAllPages();
			DeselctAllButtons();
			Invalidate();
			if (m_pTabDialog != NULL)
			{
				m_pTabDialog->SetSelectedButton(QUICK_SCAN_DLG);
			}
			m_btnQuickScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnQuickScan->SetTextColorA(RGB(4, 4, 4), 0, 1);
			m_btnQuickScan->SetFocusTextColor(RGB(4, 4, 4));
			m_btnQuickScan->Invalidate();
			
			if (m_pQuickscanDlg != NULL)
			{
				m_pQuickscanDlg->ShowWindow(SW_SHOW);
				m_pQuickscanDlg->OnBnClickedBtnQuickscan();
				Invalidate();
			}
			//Varada Ikhar, Date: 29/04/2015
			//Pause scaning even if threats are detected.
			if (m_pQuickscanDlg->m_bScanStarted == true)
			{
				m_pQuickscanDlg->PauseScan();
			}
			if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_GUIDLG_CLOSE_QUESTION1"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDNO)
			{
				//Varada Ikhar, Date: 29/04/2015
				//Pause scaning even if threats are detected.
				if (m_pQuickscanDlg->m_bScanStarted == true)
				{
					m_pQuickscanDlg->ResumeScan();
				}

				// Rajil Yadav Issue N0.681,742 13/06/2014
				//StartiSpyAVTray();
				//Neha Gharge 2/4/2015 Crash occur after OnCancel function
				theApp.m_bOnCloseFromMainUI = false;
				m_bIsCloseHandleCalled = false;
				return false;
			}
		}

		if ((m_pFullScanDlg != NULL) && (m_pFullScanDlg->m_virusFound > 0) && m_pFullScanDlg->m_eScanType == FULLSCAN && m_pFullScanDlg->m_bIsCleaning == false)
		{
			//Issue No : 1171 Blank window appearing in Updates.
			HideAllPages();
			DeselctAllButtons();
			Invalidate();
			if (m_pTabDialog != NULL)
			{
				m_pTabDialog->SetSelectedButton(FULL_SCAN_DLG);
			}
			m_btnFullScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnFullScan->SetTextColorA(RGB(4, 4, 4), 0, 1);
			m_btnFullScan->SetFocusTextColor(RGB(4, 4, 4));
			m_btnFullScan->Invalidate();
			
			if (m_pFullScanDlg != NULL)
			{
				m_pFullScanDlg->ShowWindow(SW_SHOW);
				m_pFullScanDlg->OnBnClickedBtnFullscan();
				Invalidate();
			}
			//Varada Ikhar, Date: 29/04/2015
			//Pause scaning even if threats are detected.
			if (m_pFullScanDlg->m_bScanStarted == true)
			{
				m_pFullScanDlg->PauseScan();
			}

			if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_GUIDLG_CLOSE_QUESTION2"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDNO)
			{
				//Varada Ikhar, Date: 29/04/2015
				//Pause scaning even if threats are detected.
				if (m_pFullScanDlg->m_bScanStarted == true)
				{
					m_pFullScanDlg->ResumeScan();
				}

				// Rajil Yadav Issue N0.681 13/06/2014
				//StartiSpyAVTray();
				//Neha Gharge 2/4/2015 Crash occur after OnCancel function
				theApp.m_bOnCloseFromMainUI = false;
				m_bIsCloseHandleCalled = false;
				return false;
			}
		}

		if ((m_pCustomScanDlg != NULL) && (m_pCustomScanDlg->m_virusFound > 0) && m_pCustomScanDlg->m_eScanType == CUSTOMSCAN && m_pCustomScanDlg->m_bIsCleaning == false)
		{
			//Issue No : 1171 Blank window appearing in Updates.
			HideAllPages();
			DeselctAllButtons();
			Invalidate();
			if (m_pTabDialog != NULL)
			{
				m_pTabDialog->SetSelectedButton(CUSTOM_SCAN_DLG);
			}
			m_btnCustomScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnCustomScan->SetTextColorA(RGB(4, 4, 4), 0, 1);
			m_btnCustomScan->SetFocusTextColor(RGB(4, 4, 4));
			m_btnCustomScan->Invalidate();
			
			if (m_pCustomScanDlg != NULL)
			{
				m_pCustomScanDlg->ShowWindow(SW_SHOW);
				m_pCustomScanDlg->OnBnClickedBtnCustomscan();
				Invalidate();
			}
			//Varada Ikhar, Date: 29/04/2015
			//Pause scaning even if threats are detected.
			if (m_pCustomScanDlg->m_bScanStarted == true)
			{
				m_pCustomScanDlg->PauseScan();
			}

			if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_GUIDLG_CLOSE_QUESTION3"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDNO)
			{
				//Varada Ikhar, Date: 29/04/2015
				//Pause scaning even if threats are detected.
				if (m_pCustomScanDlg->m_bScanStarted == true)
				{
					m_pCustomScanDlg->ResumeScan();
				}

				// Rajil Yadav Issue N0.681 13/06/2014
				//StartiSpyAVTray();
				//Neha Gharge 2/4/2015 Crash occur after OnCancel function
				theApp.m_bOnCloseFromMainUI = false;
				m_bIsCloseHandleCalled = false;
				return false;
			}
		}
		if (m_pQuickscanDlg != NULL)
		{
			if (m_pQuickscanDlg->m_eScanType == QUICKSCAN)
			{
				m_pQuickscanDlg->m_bClose = true;
				if (!m_pQuickscanDlg->ShutDownScanning(true))
				{
					//Neha Gharge 2/5/2015 Message Box not appears
					m_pQuickscanDlg->m_bClose = false;
					//Neha Gharge 2/4/2015 Crash occur after OnCancel function
					theApp.m_bOnCloseFromMainUI = false;
					m_bIsCloseHandleCalled = false;
					m_bisUIcloseRquestFromTray = false;
					return false;
				}
				/*if(!m_pQuickscanDlg->ThreatsFound())
				{
				return;
				}*/
				if (m_pQuickscanDlg->m_bIsCleaning == true)
				{
					if (!m_pQuickscanDlg->CloseCleaning())
					{
						// Neha ghare Issue N0.681,742 17/06/2014
						//StartiSpyAVTray();
						m_bIsCloseHandleCalled = false;

						return false;
					}
				}
			}
		}

		if (m_pFullScanDlg != NULL)
		{
			if (m_pFullScanDlg->m_eScanType == FULLSCAN)
			{
				//Neha Gharge 2/5/2015 Message Box not appears
				m_pFullScanDlg->m_bClose = true;
				if (!m_pFullScanDlg->ShutDownScanning(true))
				{
					m_pFullScanDlg->m_bClose = false;
					//Neha Gharge 2/4/2015 Crash occur after OnCancel function
					theApp.m_bOnCloseFromMainUI = false;
					m_bIsCloseHandleCalled = false;
					m_bisUIcloseRquestFromTray = false;
					return false;
				}
				/*if(!m_pFullScanDlg->ThreatsFound())
				{
				return;
				}*/
				if (m_pFullScanDlg->m_bIsCleaning == true)
				{
					if (!m_pFullScanDlg->CloseCleaning())
					{
						// Neha ghare Issue N0.681,742 17/06/2014
						//StartiSpyAVTray();
						m_bIsCloseHandleCalled = false;
						return false;
					}
				}
			}
		}

		if (m_pCustomScanDlg != NULL)
		{
			if (m_pCustomScanDlg->m_eScanType == CUSTOMSCAN)
			{
				m_pCustomScanDlg->m_bClose = true;
				if (!m_pCustomScanDlg->ShutDownScanning(true))
				{
					//Neha Gharge 2/5/2015 Message Box not appears
					m_pCustomScanDlg->m_bClose = false;
					//Neha Gharge 2/4/2015 Crash occur after OnCancel function
					theApp.m_bOnCloseFromMainUI = false;
					m_bIsCloseHandleCalled = false;
					m_bisUIcloseRquestFromTray = false;
					return false;
				}
				/*if(!m_pCustomScanDlg->ThreatsFound())
				{
				return;
				}*/
				if (m_pCustomScanDlg->m_bIsCleaning == true)
				{
					if (!m_pCustomScanDlg->CloseCleaning())
					{
						// Neha ghare Issue N0.681,742 17/06/2014
						//StartiSpyAVTray();
						m_bIsCloseHandleCalled = false;
						return false;
					}
				}
			}
		}

		//Varada Ikhar, Date: 23/04/2015,
		//Issue :  While encryption/decryption is in progress if user clicks on close button 'Wait to complete the operation' message should be displayed.
		if (m_pDataEncryption != NULL)
		{
			if (!m_pDataEncryption->ShutDownEncryptDecrypt())
			{
				theApp.m_bOnCloseFromMainUI = false;
				m_bisUIcloseRquestFromTray = false;
				m_bIsCloseHandleCalled = false;
				return false;
			}
		}
		// resolve by lalit kumawat 3-17-015
		//issue: -click on settings-> click delete reports->Click on settings button next to "Delete Reports"->Click on wardwiz tray->click exit.The delete report dialog should get close. 
		//(same issue applies to change password & scan computer at start up settings

		if (m_pobjSettingsDlg != NULL)
		{
			if (theApp.m_SettingsReport != NULL)
			{
				if (m_pCustomScanDlg->m_eScanType == CUSTOMSCAN)
				{
					m_pCustomScanDlg->m_bClose = true;
					if (!m_pCustomScanDlg->ShutDownScanning(true))
					{
						//Neha Gharge 2/5/2015 Message Box not appears
						m_pCustomScanDlg->m_bClose = false;
						//Neha Gharge 2/4/2015 Crash occur after OnCancel function
						theApp.m_bOnCloseFromMainUI = false;
						return false;
					}

					/*if(!m_pCustomScanDlg->ThreatsFound())
					{
					return;
					}*/
					if (m_pCustomScanDlg->m_bIsCleaning == true)
					{
						if (!m_pCustomScanDlg->CloseCleaning())
						{
							// Neha ghare Issue N0.681,742 17/06/2014
							//StartiSpyAVTray();
							return false;
						}
					}
				}
			}

			// resolve by lalit kumawat 3-17-015
			//issue: -click on settings-> click delete reports->Click on settings button next to "Delete Reports"->Click on wardwiz tray->click exit.The delete report dialog should get close. 
			//(same issue applies to change password & scan computer at start up settings

			if (m_pobjSettingsDlg != NULL)
			{
				if (theApp.m_SettingsReport != NULL)
				{
					::SendMessage(theApp.m_SettingsReport->m_hWnd, WM_CLOSE, 0, 0);
				}

				if (theApp.m_pSettingsScanTypeDlg != NULL)
				{
					::SendMessage(theApp.m_pSettingsScanTypeDlg->m_hWnd, WM_CLOSE, 0, 0);
				}

				if (theApp.m_SettingsPassChange != NULL)
				{
					::SendMessage(theApp.m_SettingsPassChange->m_hWnd, WM_CLOSE, 0, 0);
				}

				::SendMessage(m_pobjSettingsDlg->m_hWnd, WM_CLOSE, 0, 0);
			}
		
		}
		
		if (theApp.m_lpLoadProductInformation != NULL)
			{
				if (theApp.m_lpCloseProductInformationDlg != NULL)
				{
					theApp.m_lpCloseProductInformationDlg();
				}
		}
		//AfxBeginThread(ShutDownScanningCloseUIThread, this);
		m_bIsCloseHandleCalled = false;
		if (m_bisUiCloseCalled)
		{
			m_bIsUiTerminated = true;
			return true;
		}
		else
		{
			Sleep(1000);
			OnCloseUI();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in HandleCloseButton", 0, 0, true, SECONDLEVEL);
	}
	
	return true;
}

/***************************************************************************
  Function Name  : OnSetCursor
  Description    : The framework calls this member function if mouse input 
				   is not captured and the mouse causes cursor movement within the CWnd object.
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
BOOL CISpyGUIDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if(!pWnd)
		return FALSE;

	// hand cursor on information button neha gharge 22/5/2014.
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if( 
		iCtrlID == IDC_BUTTON_HOME			||
		iCtrlID == IDC_BUTTON_FACEBOOK		||
		iCtrlID == IDC_BUTTON_TWITTER		||
		iCtrlID == IDC_BUTTON_LINKEDIN		||
		iCtrlID == IDC_BUTTON_UPDATE		||
		iCtrlID == IDC_BUTTON_INFORMATION	||
		iCtrlID == IDC_BUTTON_HELP			||
		iCtrlID == IDC_BUTTON_MINIMIZE		||
		iCtrlID == IDC_BUTTON_CLOSE			||
		iCtrlID == ID_BUTTON_FULL_SCAN		||
		iCtrlID == ID_BUTTON_CUSTOM_SCAN	||
		iCtrlID == ID_BUTTON_QUICK_SCAN		||
		iCtrlID == IDC_BUTTON_TOOLS 		||
		iCtrlID == ID_BUTTON_REPORTS 		||
		iCtrlID == ID_BUTTON_UPDATE 		||
		iCtrlID == IDC_BUTTON_EMAILSCAN 	||
		iCtrlID == IDC_BUTTON_FIREWALL 		||
		iCtrlID == ID_BUTTON_ANTIROOTKIT 	||
		//iCtrlID == IDC_BUTTON_PROTECT		||
		//iCtrlID == IDC_BUTTON_NONPROTECT	||
		//iCtrlID == IDC_BUTTON_REGISTERNOW	||
		iCtrlID == ID_BUTTON_REGISTYOPTIMIZER||
		iCtrlID == ID_BUTTON_FOLDERLOCKER	||
		iCtrlID == ID_BUTTON_VIRUSSCAN		||
		iCtrlID == ID_BUTTON_SPAMFILTER		||
		iCtrlID == ID_BUTTON_CONTENTFILTER	||
		iCtrlID == ID_BUTTON_SIGNATURE		||
		iCtrlID == IDC_BUTTON_RECOVER		||
		iCtrlID == IDC_BUTTON_MAXIMIZE		||
		iCtrlID == ID_BUTTON_RECOVER		||
		iCtrlID == ID_BUTTON_DATAENCRYPTION ||
		iCtrlID == IDC_BUTTON_SUPPORT       ||
		iCtrlID == ID_BUTTON_UTILITY_OPTION
		)
	{
		CString csClassName;
		::GetClassName(pWnd->GetSafeHwnd(), csClassName.GetBuffer(80), 80);
		if(csClassName == _T("Button") && m_hButtonCursor)
		{
			::SetCursor(m_hButtonCursor);
			return TRUE;
		}
	}
	return CJpegDialog::OnSetCursor(pWnd, nHitTest, message);
}

/***************************************************************************
  Function Name  : OnNcHitTest
  Description    : The framework calls this member function for the CWnd object
				   that contains the cursor (or the CWnd object that used the 
				   SetCapture member function to capture the mouse input) every 
				   time the mouse is moved.
  Author Name    : Ramkrushna Shelke.
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
LRESULT CISpyGUIDlg::OnNcHitTest(CPoint point)
{
	return HTCAPTION;
}

/***************************************************************************
  Function Name  : OnBnClickedButtonUpdate
  Description    : Update button click handler.It shows help files.
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
  Modification	 : 28th may 2015 no need to remove the focus from last functionlity
  when we clicked on in button other than tab button and home button.
****************************************************************************/
void CISpyGUIDlg::OnBnClickedButtonUpdate()
{
	//SetSkinOnhomePage();
	TCHAR szModulePath[MAX_PATH] = {0};
	TCHAR szFullPath[MAX_PATH] = {0};

	GetModuleFileName(NULL, szModulePath, MAX_PATH);
	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	_tcscpy(szFullPath, szModulePath);
	//Issue: CHM(Help) Files should be available in german language as well
	//Resolved By: Nitin K Date: 11th Jan 2016
	switch (theApp.m_dwProductID)
	{
	case ESSENTIAL:
		switch (theApp.m_dwSelectedLangID)
		{
		case ENGLISH:  _tcscat_s(szFullPath, L"\\WrdWizAVEssential.chm");
			break;
		case GERMAN:
			_tcscat_s(szFullPath, L"\\WrdWizAVEssentialGerman.chm");
			if (!PathFileExists(szFullPath))
			{
				_tcscpy(szFullPath, L"\0");
				_tcscpy(szFullPath, szModulePath);
				_tcscat_s(szFullPath, L"\\WrdWizAVEssential.chm");
			}
			break;
		default: _tcscat_s(szFullPath, L"\\WrdWizAVEssential.chm");
			break;
		}
		break;
	case PRO:
		switch (theApp.m_dwSelectedLangID)
		{
		case ENGLISH:  _tcscat_s(szFullPath, L"\\WrdWizAVPro.chm");
			break;
		case GERMAN:  _tcscat_s(szFullPath, L"\\WrdWizAVProGerman.chm");
			if (!PathFileExists(szFullPath))
			{
				_tcscpy(szFullPath, L"\0");
				_tcscpy(szFullPath, szModulePath);
				_tcscat_s(szFullPath, L"\\WrdWizAVPro.chm");
			}
			break;
		default: _tcscat_s(szFullPath, L"\\WrdWizAVPro.chm");
			break;
		}
		break;
	case ELITE:_tcscat(szFullPath, L"\\WrdWizAVElite.chm");
		break;
	case BASIC:
		switch (theApp.m_dwSelectedLangID)
		{
		case ENGLISH:  _tcscat_s(szFullPath, L"\\WrdWizAVBasic.chm");
			break;
		case GERMAN:  _tcscat_s(szFullPath, L"\\WrdWizAVBasicGerman.chm");
			if (!PathFileExists(szFullPath))
			{
				_tcscpy(szFullPath, L"\0");
				_tcscpy(szFullPath, szModulePath);
				_tcscat_s(szFullPath, L"\\WrdWizAVBasic.chm");
			}
			break;
		default: _tcscat_s(szFullPath, L"\\WrdWizAVBasic.chm");
			break;
		}
		break;
	default: AddLogEntry(L"Invalid option, help file not available", 0, 0, true, SECONDLEVEL);
		break;
	}

	::ShellExecute(NULL, L"Open", szFullPath, L"", NULL,SW_SHOWNORMAL);
}

/***************************************************************************
  Function Name  : OnBnClickedButtonScanpage
  Description    : Scan button click handler.
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
//void CISpyGUIDlg::OnBnClickedButtonScanpage()
//{
//	ShowHideMainPageControls(false);
//	//m_dlgScan.OnBnClickedBtnQuickscan();
//	//m_dlgScan.ShowWindow(SW_SHOW);
//}

/***************************************************************************
  Function Name  : ShowHideMainPageControls
  Description    : Funtion to show hide controls on Main UI.
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
//void CISpyGUIDlg::ShowHideMainPageControls(bool bEnable)
//{
//	CString csNoofdays;
//	//DWORD dwnoofdays;
//	//m_btnScanDlg.ShowWindow(bEnable);
//	//m_btnTools.ShowWindow(bEnable);
//	//m_btnUsbDetect.ShowWindow(bEnable);
//	//m_btnReports.ShowWindow(bEnable);
//	//m_btnUpdates.ShowWindow(bEnable);
//	//m_btnEmailScan.ShowWindow(bEnable);
//	//m_btnFirewall.ShowWindow(bEnable);
//	//m_btnAntRootkit.ShowWindow(bEnable);
//	m_btnRegisterNow.ShowWindow(bEnable);
//	m_stDayLeftOutPic.ShowWindow(bEnable);
//	m_stDaysLefttext.ShowWindow(bEnable);
//	m_stNoOfDays.ShowWindow(bEnable);
//
//	m_stUpDateTime.ShowWindow(bEnable);
//	m_stUpdateTime.ShowWindow(bEnable);
//	m_stLiveUpdateDateTime.ShowWindow(bEnable);
//	m_stLastScan.ShowWindow(bEnable);
//	m_stLastScanDateTime.ShowWindow(bEnable);
//	m_stVersion.ShowWindow(bEnable);
//	m_stVersionNo.ShowWindow(bEnable);
//	m_stLastScanType.ShowWindow(bEnable);
//	m_stMainUIHeaderPic.ShowWindow(bEnable);
//	m_btnProtect.ShowWindow(bEnable);
//
//
//
//	GetDlgItem(IDC_STATIC_UPDATE_TIME)->ShowWindow(bEnable);
//	GetDlgItem(IDC_STATIC_LASTUPADTE)->ShowWindow(bEnable);
//	GetDlgItem(IDC_STATIC_LIVEUPDATE_DATETIME)->ShowWindow(bEnable);
//
//	m_stScanType.ShowWindow(bEnable);
//	//m_stMainUIHeaderPic.ShowWindow(bEnable);
//	//m_stHeaderNonProtect.ShowWindow(bEnable);
//	//we have to check whether trial version of reg if register or trial 
//	m_stRegVersion.ShowWindow(SW_HIDE);
//	m_stTrialVersion.ShowWindow(SW_HIDE);
//	//m_bIsRegister = true;
//	if(bEnable)
//	{
//		if(!m_bIsRegister)
//		{
//			RegistryEntryOnUI();
//			if(bLiveUpdateMsg == 1)
//			{
//				bLiveUpdateMsg = 0;
//			}
//			if(bVirusMsgDetect == 1)
//			{
//				bVirusMsgDetect = 0;
//			}
//			m_btnProtect.ShowWindow(SW_HIDE);
//			GetDlgItem(IDC_STATIC_MAINUI_HEADER_PIC)->ShowWindow(FALSE);
//			//m_stMainUIHeaderPic.ShowWindow(SW_HIDE);
//			m_stDaysLefttext.ShowWindow(SW_SHOW);
//			m_stNoOfDays.ShowWindow(SW_SHOW);
//			m_stDayLeftOutPic.ShowWindow(SW_SHOW);
//			m_stNoOfDays.SetWindowTextW(csNoofdays);
//			m_btnRegisterNow.ShowWindow(SW_SHOW);
//			m_stSecureMsg.ShowWindow(SW_HIDE);
//			m_stNonProtectedText.ShowWindow(FALSE);
//			m_btnNonProtect.ShowWindow(TRUE);
//			m_bmpShowUnProtectedGif.ShowWindow(TRUE);
//			m_bmpShowUnProtectedGif.Draw();
//			m_stHeaderNonProtect.ShowWindow(TRUE);
//			m_stRegisterNowHeaderMsg.ShowWindow(SW_SHOW);
//			m_stClickOnText.ShowWindow(SW_SHOW);
//		
//		}
//		else
//		{
//			
//			m_btnRegisterNow.ShowWindow(SW_HIDE);
//			m_stRegisterNowHeaderMsg.ShowWindow(SW_HIDE);
//			m_stClickOnText.ShowWindow(SW_HIDE);
//			m_btnNonProtect.ShowWindow(SW_HIDE);
//			m_bmpShowUnProtectedGif.Stop();
//			m_bmpShowUnProtectedGif.ShowWindow(FALSE);
//			m_stNonProtectedText.ShowWindow(FALSE);
//			m_dwNoofDays = theApp.m_dwDaysLeft;
//			csNoofdays.Format(L"%d",m_dwNoofDays);
//			m_stDayLeftOutPic.ShowWindow(SW_HIDE);
//			m_stNoOfDays.ShowWindow(SW_HIDE);
//			m_stDaysLefttext.ShowWindow(SW_HIDE);
//			GetDlgItem(IDC_STATIC_MAINUI_HEADER_PIC)->ShowWindow(TRUE);
//			m_stMainUIHeaderPic.ShowWindow(SW_SHOW);
//			m_stSecureMsg.ShowWindow(SW_SHOW);
//			m_btnProtect.ShowWindow(SW_SHOW);
//			RegistryEntryOnUI();
//		//	m_btnProtect.Invalidate(TRUE);
//	
//			if(m_dwNoofDays <= 30 && m_dwNoofDays >0)
//			{
//				m_dwNoofDays = theApp.m_dwDaysLeft;
//				csNoofdays.Format(L"%d",m_dwNoofDays);
//				m_btnRegisterNow.ShowWindow(SW_SHOW);
//				m_stNoOfDays.SetWindowPos(&wndTop,m_rect.left +605,100,20,15,SWP_SHOWWINDOW);//605
//				m_stDaysLefttext.SetWindowPos(&wndTop, m_rect.left + 590,88,54,11, SWP_SHOWWINDOW);//590
//				m_stDayLeftOutPic.SetWindowPos(&wndTop, m_rect.left +586,87,61,28, SWP_SHOWWINDOW);//586
//				m_stNoOfDays.SetWindowTextW(csNoofdays);
//				Invalidate();
//				
//			}
//			else if(m_dwNoofDays == 0)
//			{
//				
//				m_btnProtect.ShowWindow(SW_HIDE);
//				GetDlgItem(IDC_STATIC_MAINUI_HEADER_PIC)->ShowWindow(FALSE);
//				m_stSecureMsg.ShowWindow(FALSE);
//				//m_stMainUIHeaderPic.ShowWindow(FALSE);
//				m_stHeaderNonProtect.ShowWindow(TRUE);
//				m_stNonProtectedText.ShowWindow(TRUE);
//				m_btnNonProtect.ShowWindow(TRUE);
//				m_btnNonProtect.Invalidate();
//				m_bmpShowUnProtectedGif.Draw();
//				m_bmpShowUnProtectedGif.ShowWindow(TRUE);
//				m_dwNoofDays = theApp.m_dwDaysLeft;
//				csNoofdays.Format(L"%d",m_dwNoofDays);
//				m_btnRegisterNow.ShowWindow(SW_SHOW);
//				m_stNoOfDays.SetWindowPos(&wndTop,m_rect.left +605,100,20,15,SWP_SHOWWINDOW);//605
//				m_stDaysLefttext.SetWindowPos(&wndTop, m_rect.left + 590,88,54,11, SWP_SHOWWINDOW);//590
//				m_stDayLeftOutPic.SetWindowPos(&wndTop, m_rect.left +586,87,61,28, SWP_SHOWWINDOW);//586
//			}
//				
//		}
//	}
//	else
//	{
//	//	m_dlgScan.ShowWindow(SW_HIDE);
//	//	m_iSpyToolsDlg.ShowWindow(SW_HIDE);
//	//	m_objReportsDlg.ShowWindow(SW_HIDE);
//	//	m_objISpyUpdatesDlg.ShowWindow(SW_HIDE);
//	//	m_objISpyEmailScanDlg.ShowWindow(SW_HIDE);
//	//	m_objISpyAntiRootkitDlg.ShowWindow(SW_HIDE);
//		m_btnProtect.ShowWindow(SW_HIDE);
//		m_btnNonProtect.ShowWindow(SW_HIDE);
//		m_bmpShowUnProtectedGif.Stop();
//		m_bmpShowUnProtectedGif.ShowWindow(SW_HIDE);
//		m_stHeaderNonProtect.ShowWindow(SW_HIDE);
//		m_stSecureMsg.ShowWindow(SW_HIDE);
//
//
//	}
//	Invalidate();
//}

/***************************************************************************
  Function Name  : OnBnClickedButtonTools
  Description    : Tools button click handler.
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
//void CISpyGUIDlg::OnBnClickedButtonTools()
//{
//
//#ifdef ISPYTRIAL
//	MessageBox(L"This feature is unavailable in free edition", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
//	return;
//#endif
//	ShowHideMainPageControls(false);
////	m_iSpyToolsDlg.HideChildWindows();
////	m_iSpyToolsDlg.ShowWindow(SW_SHOW);
//}

/***************************************************************************
  Function Name  : OnBnClickedButtonReports
  Description    : Tools button click handler.
  Author Name    : Ramkrushna Shelke
  Date           : 18th Nov 2013
****************************************************************************/
//void CISpyGUIDlg::OnBnClickedButtonReports()
//{
//	ShowHideMainPageControls(false);
////	m_objReportsDlg.ShowControls4Reports();
////	m_objReportsDlg.ShowWindow(SW_SHOW);
//}

/***************************************************************************
  Function Name  : OnBnClickedButtonUpdates
  Description    : Live update button click handler.
  Author Name    : Ramkrushna Shelke
  Date           : 18th Nov 2013
****************************************************************************/
//void CISpyGUIDlg::OnBnClickedButtonUpdates()
//{
//	ShowHideMainPageControls(false);
////	m_objISpyUpdatesDlg.ShowOnlyFirstWindow();
////	m_objISpyUpdatesDlg.ShowWindow(SW_SHOW);
//}

/***************************************************************************
  Function Name  : OnBnClickedButtonHome
  Description    : Home button click handler.
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
void CISpyGUIDlg::OnBnClickedButtonHome()
{
	try
	{
		// lalit 4-28-2015 , if we close on stop button and same time exit from tray then two popup comes , now single popup will come
		// if any task is going on then user should have to abort task to go for home page
		if (IsAnyTaskInProcess())
		{
			if (StopRunningProcess())
			{
				ShowHomeButtonPage();
				
				if (m_pTabDialog != NULL)
				{
					m_pTabDialog->m_SelectedButton = 0;
				}

			}
		}
		else
		{
			ShowHomeButtonPage();

			if (m_pTabDialog != NULL)
			{
				m_pTabDialog->m_SelectedButton = 0;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in OnBnClickedButtonHome", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : OnBnClickedButtonFirewall
  Description    : Firewall button click handler.
  Author Name    : Ramkrushna Shelke
  Date           : 18th Nov 2013
****************************************************************************/
//void CISpyGUIDlg::OnBnClickedButtonFirewall()
//{
//	ShowNoImplementation();
//}

/***************************************************************************
  Function Name  : StartiSpyAVTray
  Description    : This function will launch the Tray.
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
void CISpyGUIDlg::StartiSpyAVTray()
{
	AddLogEntry(L">>> GenXGUIDlg : Start GenX tray", 0, 0, true, FIRSTLEVEL);
	TCHAR szModulePath[MAX_PATH] = {0};
	TCHAR szFullPath[MAX_PATH] = {0};

	GetModuleFileName(NULL, szModulePath, MAX_PATH);
	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	_tcscpy_s(szFullPath, szModulePath);
	_tcscat_s(szFullPath, L"\\WRDWIZTRAY.EXE");

	//ISsue no 742 Neha Gharge 17/6/2014 
	::ShellExecute(NULL, L"Open", szFullPath, L"-NOSPSCRN", NULL, SWP_SHOWWINDOW);
}

/***************************************************************************
  Function Name  : OnBnClickedButtonFacebook
  Description    : Facebook button click handler.Opens Fb page.
  Author Name    : Neha
  SR_NO			 :
  Date           : 18th Nov 2013
  Modification	 : 28th may 2015 no need to remove the focus from last functionlity
  when we clicked on in button other than tab button and home button.
****************************************************************************/
void CISpyGUIDlg::OnBnClickedButtonFacebook()
{
	//SetSkinOnhomePage();
	//ShellExecute(NULL,L"open",L"https://www.facebook.com/wardwiz",NULL,NULL,SW_SHOW);


	TCHAR	szPath[512] = {0};

	if( GetDefaultHTTPBrowser(szPath) )
	{
		GetEnvironmentVariable( L"ProgramFiles", szPath, 511 );
		wcscat_s( szPath, 511, L"\\Internet Explorer\\iexplore.exe" );
	}
// issue number -3 ,resolved by lalit kumawat , 'FB',"Twitter','LinkedIn' ui are not working.
	ShellExecute(NULL,L"open", szPath, L"https://www.facebook.com/brandwardwiz", NULL, SW_SHOW);
}

/***************************************************************************
  Function Name  : OnBnClickedButtonTwitter
  Description    : Twitter button click handler.Opens twitter page
  Author Name    : Neha
  SR_NO			 :
  Date           : 18th Nov 2013
  Modification	 : 28th may 2015 no need to remove the focus from last functionlity
  when we clicked on in button other than tab button and home button.
****************************************************************************/
void CISpyGUIDlg::OnBnClickedButtonTwitter()
{
	//SetSkinOnhomePage();
	//ShellExecute(NULL,L"open",L"https://twitter.com/wardwizav",NULL,NULL,SW_SHOW);

	TCHAR	szPath[512] = {0};

	if( GetDefaultHTTPBrowser(szPath) )
	{
		GetEnvironmentVariable( L"ProgramFiles", szPath, 511 );
		wcscat_s( szPath, 511, L"\\Internet Explorer\\iexplore.exe" );
	}

// issue number -3 ,resolved by lalit kumawat , 'FB',"Twitter','LinkedIn' ui are not working.
	ShellExecute(NULL,L"open",szPath, L"https://twitter.com/brandwardwiz",NULL,SW_SHOW);
}

/***************************************************************************
  Function Name  : ShowNoImplementation
  Description    : function will show No implementation
  Author Name    : Neha
  SR_NO			 :
  Date           : 18th Nov 2013
****************************************************************************/
void CISpyGUIDlg::ShowNoImplementation()
{
	MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_NOT_AVAILABLE"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
}

/***************************************************************************
  Function Name  : OnBnClickedButtonAntirootkit
  Description    : Antirootkit button click handler
  Author Name    : Neha
  Date           : 18th Nov 2013
****************************************************************************/
//void CISpyGUIDlg::OnBnClickedButtonAntirootkit()
//{
//	//ShowNoImplementation();
//	//ShowHideMainPageControls(false);
//	//m_objISpyAntiRootkitDlg.HideControls4ScannedList(true);
//	//m_objISpyAntiRootkitDlg.HideChildControls(false);
//	//m_objISpyAntiRootkitDlg.ShowHideControls(true);
//
//	//m_objISpyAntiRootkitDlg.ShowWindow(SW_SHOW);
//}

/***************************************************************************
  Function Name  : test_generate_report
  Description    : It generates error report.
  Author Name    : Ramkrushna Shelke
  SR_NO			 :
  Date           : 18th July 2014
****************************************************************************/
//void CISpyGUIDlg::test_generate_report()
//{
//    CR_EXCEPTION_INFO ei;
//    memset(&ei, 0, sizeof(CR_EXCEPTION_INFO));
//    ei.cb = sizeof(CR_EXCEPTION_INFO);
//    ei.exctype = CR_SEH_EXCEPTION;
//    ei.code = 0x1234;
//    ei.pexcptrs = NULL;
//    ei.bManual = TRUE; // Signal the report is being generated manually.
//
//    int nResult = crGenerateErrorReport(&ei);
//    if(nResult!=0)
//    {
//        TCHAR szErrorMsg[256];
//        CString sError = _T("Error generating error report!\nErrorMsg:");
//        crGetLastErrorMsg(szErrorMsg, 256);
//        sError+=szErrorMsg;
//        MessageBox(sError, 0, 0);
//    }
//}

/***************************************************************************
  Function Name  : OnBnClickedButtonHelp
  Description    : Help button click handler, It is used to show setting tab dialog
  Author Name    : Neha
  SR_NO			 :
  Date           : 18th Nov 2013
  Modification	 : 28th may 2015 no need to remove the focus from last functionlity
  when we clicked on in button other than tab button and home button.
****************************************************************************/
void CISpyGUIDlg::OnBnClickedButtonHelp()
{
	//	ShowNoImplementation();
	//SetSkinOnhomePage();


	if(m_pobjSettingsDlg != NULL)
	{
		delete m_pobjSettingsDlg;
		m_pobjSettingsDlg  = NULL;
	}
	m_pobjSettingsDlg = NULL;
	m_pobjSettingsDlg = new CSettingsDlg();
	
	m_bIsPopUpDisplayed = true;
	m_pobjSettingsDlg->DoModal();
	m_bIsPopUpDisplayed = false;

	// resolved by lalit kumawat 8-7-2015
	// issue- user able to user spam filter & content filter even after changing status off of email scan settingDlg 
	bool bEmlDlgActive = false;
	DWORD dwEmailScanEnable = ReadEmailScanEntryFromRegistry();

	if (m_pTabDialog != NULL)
	{
		bEmlDlgActive = m_pTabDialog->IsEmailScanDlgActive();
		if (bEmlDlgActive && dwEmailScanEnable == 0)
		{
			// Issue - If any scan is in progress and 
			//we disabled email scan from tray or setting tab 
			//It show message box scanning is in progress.
			//Neha Gharge 12 Aug,2015
			ShowHomeButtonPage();

			if (m_pTabDialog != NULL)
			{
				m_pTabDialog->m_SelectedButton = 0;
			}
		}
	}
}

/***************************************************************************
  Function Name  : PreTranslateMessage
  Description    : To translate window messages before they are dispatched 
				   to the TranslateMessage and DispatchMessage Windows functions
  Author Name    : Neha
  Date           : 18th Nov 2013
****************************************************************************/
BOOL CISpyGUIDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

/***************************************************************************
  Function Name  : OnCtlColor
  Description    : The framework calls this member function when a child
				   control is about to be drawn.
  Author Name    : Neha
  Date           : 18th Nov 2013
****************************************************************************/

HBRUSH CISpyGUIDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if( ctrlID == IDC_STATIC_FOOTER  )
	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	} return hbr;
}

/***************************************************************************
  Function Name  : OnUserMessages
  Description    : This function get call when some other module send messge
				   WM_COMMAND = RELOAD EMAIL DB
  Author Name    : Neha
  Date           : 18th MArch 2014
****************************************************************************/

LRESULT CISpyGUIDlg::OnUserMessages(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	//issue no 857 Neha gharge
	if(m_pVirusscan != NULL)
	{
		//Isuue: Group box is visible at bottom for Virus scan
		if (m_pVirusscan->m_bVirusScan == 0)
		{
			m_pVirusscan->LoadExistingVirusScanFile();
			m_pVirusscan->Invalidate();
		}

	}
	//Reload spam entry Neha Gharge 29/7/2015
	if (m_pSpamFilter != NULL)
	{
		if (m_pSpamFilter->m_bSpamFilter == 0)
		{
			m_pSpamFilter->LoadExistingSpamFilterFile();
			m_pSpamFilter->Invalidate();
		}
	}
	//m_objISpyEmailScanDlg.LoadExistingVirusScanFile();
	//m_objISpyEmailScanDlg.Invalidate();
	return 0;
}

/***************************************************************************
  Function Name  : OnUserChangeEmailScanSetting
  Description    : This function get call when some other module send messge
				   WM_COMMAND = CHECKEMAILSCANSTATUS
  Author Name    : Rajil
  SR_NO			 :
  Date           : 18th MArch 2014
****************************************************************************/
LRESULT CISpyGUIDlg::OnUserChangeEmailScanSetting(WPARAM wParam , LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	//Enable /DisableEmail scan From main UI
	//24/6/2015 Neha Gharge
	MainDialogDisplay();
	//Commented becuase everytime if click from tray ,it shows home page even though we are on the other window.
	// resolved by lalit kumawat 8-7-2015
	// issue- user able to user spam filter & content filter even after changing status off of email scan systemTray 
	//Issue no 1266. Even if the Email Scan feature is off, the email scan feature window remains open.
	if (m_pVirusscan != NULL || m_pSpamFilter != NULL || m_pContentFilter != NULL)
	{
		if (m_pVirusscan->m_bIsPopUpDisplayed == true || m_pSpamFilter->m_bIsPopUpDisplayed == true || m_pContentFilter->m_bIsPopUpDisplayed == true)
		{
			return 0;
		}
	}
	
	DWORD dwEmailScanEnable = ReadEmailScanEntryFromRegistry();
	if (m_pTabDialog != NULL)
	{
		bool bEmailDlgActive = false;

		bEmailDlgActive = m_pTabDialog->IsEmailScanDlgActive();
		if (eEMAIL == wParam && dwEmailScanEnable == 0 && bEmailDlgActive)
		{
			// Issue - If any scan is in progress and 
			//we disabled email scan from tray or setting tab 
			//It show message box scanning is in progress.
			//Neha Gharge 12 Aug,2015
			ShowHomeButtonPage();

			if (m_pTabDialog != NULL)
			{
				m_pTabDialog->m_SelectedButton = 0;
			}
		}
	}

	/*Issue number 127 If the setting tab is open and we change settings from tray, it is not immediately reflected on the setting tab.  */
	if(m_pobjSettingsDlg != NULL)
	{
		RefreshUISettingWhenAVSettingChangeFromSystemTray();	
	}

	return 0;
}

/*Issue number 127 If the setting tab is open and we change settings from tray, it is not immediately reflected on the setting tab.  */
/***************************************************************************
  Function Name  : RefreshUISettingWhenAVSettingChangeFromSystemTray
  Description    : It  refresh the open setting dialog setting when it updates from SystemTray and Same time Setting Dialog also open
  Author Name    : lalit k.
  SR_NO			 :
  Date           : 9/12/14
  Modified Date	 : 
****************************************************************************/
void CISpyGUIDlg :: RefreshUISettingWhenAVSettingChangeFromSystemTray()
{
	CPoint point;
	point.SetPoint(10,10);

	if(!m_pobjSettingsDlg)
	{
		return;
	}

	if(m_pobjSettingsDlg->m_pSettingsScanDlg!= NULL)
	{
		if(m_pobjSettingsDlg->m_pSettingsScanDlg->m_list.m_hWnd != NULL)
		{
			m_pobjSettingsDlg->m_pSettingsScanDlg->m_list.UpdateUIWhenSystemTrayNotifiyUpdation(1,point);
		}
	}
	if(m_pobjSettingsDlg->m_pSettingsGeneralDlg!= NULL)
	{
		if(m_pobjSettingsDlg->m_pSettingsGeneralDlg->m_list.m_hWnd != NULL)
		{
			m_pobjSettingsDlg->m_pSettingsGeneralDlg->m_list.UpdateUIWhenSystemTrayNotifiyUpdation(1,point);
		}
	}
	if(m_pobjSettingsDlg->m_pSettingsEmailDlg!= NULL)
	{
		if(m_pobjSettingsDlg->m_pSettingsEmailDlg->m_list.m_hWnd != NULL)
		{
			m_pobjSettingsDlg->m_pSettingsEmailDlg->m_list.UpdateUIWhenSystemTrayNotifiyUpdation(1,point);
		}
	}
	if(m_pobjSettingsDlg->m_pSettingsUpdateDlg!= NULL)
	{
		if(m_pobjSettingsDlg->m_pSettingsUpdateDlg->m_list.m_hWnd != NULL)
		{
			m_pobjSettingsDlg->m_pSettingsUpdateDlg->m_list.UpdateUIWhenSystemTrayNotifiyUpdation(1,point);
		}
	}
}

/***************************************************************************
  Function Name  : OnDataReceiveCallBack
  Description    : It receive flag send from named pipe
  Author Name    : Ram , Neha , prajkta
  SR_NO			 :
  Date           : 18th December 2013
  Modified Date	 : 23th March 2014
****************************************************************************/
void CISpyGUIDlg::OnDataReceiveCallBack(LPVOID lpParam)
{
	__try
	{
		ResetEvent(theApp.m_hCryptOprEvent);
		ResetEvent(theApp.m_hUpdateOprEvent);
		//Neha Gharge 2/4/2015 Crash occur after OnCancel function
		//Neha Gharge Ticket 36 :Antirootkit gets hanged on the next antiroot kit scan
		if(theApp.m_bOnCloseFromMainUI)
		{
			//theApp.m_bOnCloseFromMainUI = false;
			return;
		}

		//OutputDebugString(L">>> In CISpyGUIDlg::OnDataReceiveCallBack");
		LPISPY_PIPE_DATA lpSpyData = (LPISPY_PIPE_DATA)lpParam;

		if (!lpSpyData)
			return;

		switch(lpSpyData->iMessageInfo)
		{
		case SCAN_STARTED:

			if( m_pThis->m_pCustomScanDlg->m_bScanStartedStatusOnGUI )
			{

				if(m_pThis->m_pQuickscanDlg != NULL)
				{
					if(m_pThis->m_pQuickscanDlg->m_bQuickScan == true)
					{
						m_pThis->m_pQuickscanDlg->ScanningStarted();
					}
				}
				if(m_pThis->m_pFullScanDlg != NULL)
				{
					if(m_pThis->m_pFullScanDlg->m_bFullScan == true)
					{
						m_pThis->m_pFullScanDlg->ScanningStarted();
					}
				}
				if(m_pThis->m_pCustomScanDlg != NULL)
				{
					if(m_pThis->m_pCustomScanDlg->m_bCustomscan == true)
					{
						m_pThis->m_pCustomScanDlg->ScanningStarted();
					}
				}
			}

			m_pThis->m_pCustomScanDlg->m_bScanStartedStatusOnGUI = false;
			break;
		case SCAN_FINISHED:
			if(m_pThis->m_pQuickscanDlg != NULL)
			{
				if(m_pThis->m_pQuickscanDlg->m_bQuickScan == true)
				{
					m_pThis->m_pQuickscanDlg->ScanningFinished();
				}
			}
			if(m_pThis->m_pFullScanDlg != NULL)
			{
				if(m_pThis->m_pFullScanDlg->m_bFullScan == true)
				{
					m_pThis->m_pFullScanDlg->ScanningFinished();
				}
			}
			if(m_pThis->m_pCustomScanDlg != NULL)
			{
				if(m_pThis->m_pCustomScanDlg->m_bCustomscan == true)
				{
					m_pThis->m_pCustomScanDlg->ScanningFinished();
				}
			}

			//Issue: If Any operation is in progress and we want to Right click Crypt operation then to stop all Process and Srart Crypt operation
			//Resolved by : Nitin K. Date: 15th July 2015.
			if (theApp.m_hCryptOprEvent)
			{
				SetEvent(theApp.m_hCryptOprEvent);
			}
			if (theApp.m_hUpdateOprEvent)
			{
				SetEvent(theApp.m_hUpdateOprEvent);
			}
			//Varada Ikhar, Date:23/04/2015
			//Issue : 000033 : 1.In all scans, start scanning 2. click close button in UI then popup displays 'Do you want to stop rootkit scan' 3. Dont take any action
			//4. Rootkit scanning completed message appears 5. Then click yes on stop confirmation dialog box 6. UI is getting closed by leaving succesful dialog box on the desktop.
			m_pThis->m_btnClose.EnableWindow(true);

			break;
		case ANTIROOT_STARTED:
			if(m_pThis->m_pAntirootkitScan != NULL)
			{
				m_pThis->m_pAntirootkitScan->RootKitScanningStarted();
			}
			break;
		case ANTIROOT_FINISHED:
			if(m_pThis->m_pAntirootkitScan != NULL)
			{
				//Varada Ikhar, Date:23/04/2015
				//Issue : 000033 : 1.In all scans, start scanning 2. click close button in UI then popup displays 'Do you want to stop rootkit scan' 3. Dont take any action
				//4. Rootkit scanning completed message appears 5. Then click yes on stop confirmation dialog box 6. UI is getting closed by leaving succesful dialog box on the desktop.
				m_pThis->m_btnClose.EnableWindow(false);

				m_pThis->m_pAntirootkitScan->RootKitScanningFinished();
			}

			//Issue: If Any operation is in progress and we want to Right click Crypt operation then to stop all Process and Srart Crypt operation
			//Resolved by : Nitin K. Date: 15th July 2015.
			if (theApp.m_hCryptOprEvent)
			{
				SetEvent(theApp.m_hCryptOprEvent);
			}
			if (theApp.m_hUpdateOprEvent)
			{
				SetEvent(theApp.m_hUpdateOprEvent);
			}
			//Varada Ikhar, Date:23/04/2015
			//Issue : 000033 : 1.In all scans, start scanning 2. click close button in UI then popup displays 'Do you want to stop rootkit scan' 3. Dont take any action
			//4. Rootkit scanning completed message appears 5. Then click yes on stop confirmation dialog box 6. UI is getting closed by leaving succesful dialog box on the desktop.
			m_pThis->m_btnClose.EnableWindow(true);

			break;
		case REGISTRY_SCAN_FINISHED:
			if(m_pThis->m_pRegistryOptimizer != NULL)
			{
				m_pThis->m_pRegistryOptimizer->ScanningStopped();
			}
			break;
		case SHOW_STATUS:
			if(m_pThis->m_pQuickscanDlg != NULL)
			{
				if(m_pThis->m_pQuickscanDlg->m_bQuickScan == true)
				{
					m_pThis->m_pQuickscanDlg->ShowStaus(lpSpyData->szFirstParam);
				}
			}
			if(m_pThis->m_pFullScanDlg != NULL)
			{
				if(m_pThis->m_pFullScanDlg->m_bFullScan == true)
				{
					m_pThis->m_pFullScanDlg->ShowStaus(lpSpyData->szFirstParam);
				}
			}
			if(m_pThis->m_pCustomScanDlg != NULL)
			{
				if(m_pThis->m_pCustomScanDlg->m_bCustomscan == true)
				{
					m_pThis->m_pCustomScanDlg->ShowStaus(lpSpyData->szFirstParam);
				}
			}
			break;
		case SHOW_VIRUSENTRY:
			//OutputDebugString(L">>> In SHOW_VIRUSENTRY");
			//m_pThis->m_dlgScan.ShowVirusEntry(lpSpyData->szFirstParam);
			//Added by Nitin K.
			//Issue: Displaying Virus found entries on UI through Pipe Communication
			//Date : 15-Jan-2015
			if(m_pThis->m_pQuickscanDlg != NULL)
			{
				if(m_pThis->m_pQuickscanDlg->m_bQuickScan == true)
				{
					m_pThis->m_pQuickscanDlg->VirusFoundEntries(lpSpyData);
				}
			}
			if(m_pThis->m_pFullScanDlg != NULL)
			{
				if(m_pThis->m_pFullScanDlg->m_bFullScan == true)
				{
					m_pThis->m_pFullScanDlg->VirusFoundEntries(lpSpyData);
				}
			}
			if(m_pThis->m_pCustomScanDlg != NULL)
			{
				if(m_pThis->m_pCustomScanDlg->m_bCustomscan == true)
				{
					m_pThis->m_pCustomScanDlg->VirusFoundEntries(lpSpyData);
				}
			}
			break;
		case SET_TOTALFILECOUNT:
			if(m_pThis->m_pQuickscanDlg != NULL)
			{
				if(m_pThis->m_pQuickscanDlg->m_bQuickScan == true)
				{
					m_pThis->m_pQuickscanDlg->SetTotalFileCount(lpSpyData->dwValue, lpSpyData->dwSecondValue);
				}
			}
			if(m_pThis->m_pFullScanDlg != NULL)
			{
				if(m_pThis->m_pFullScanDlg->m_bFullScan == true)
				{
					m_pThis->m_pFullScanDlg->SetTotalFileCount(lpSpyData->dwValue, lpSpyData->dwSecondValue);
				}
			}
			if(m_pThis->m_pCustomScanDlg != NULL)
			{
				if(m_pThis->m_pCustomScanDlg->m_bCustomscan == true)
				{
					m_pThis->m_pCustomScanDlg->SetTotalFileCount(lpSpyData->dwValue, lpSpyData->dwSecondValue);
				}
			}
			//In antirootkit scan if we abort the scan,files scanned count is shown "0".
			//Niranjan Deshak - 05/03/2015.
			if (m_pThis->m_pAntirootkitScan != NULL)
			{
				if (m_pThis->m_pAntirootkitScan->m_bAntirootkitScan == true)
				{
					DWORD dwTotalFileCnt = 0x00, dwTotalDriveCnt = 0x00, dwTotalProcCnt = 0x00, dwDetectedFileCnt = 0x00, dwDetectedDriveCnt = 0x00, dwDetectedProcCnt = 0x00;
					dwTotalFileCnt = (*(DWORD *)&lpSpyData->byData[0]);
					dwTotalDriveCnt = (*(DWORD *)&lpSpyData->byData[8]);
					dwTotalProcCnt = (*(DWORD *)&lpSpyData->byData[16]);

					dwDetectedFileCnt = (*(DWORD *)&lpSpyData->byData[24]);
					dwDetectedDriveCnt = (*(DWORD *)&lpSpyData->byData[32]);
					dwDetectedProcCnt = (*(DWORD *)&lpSpyData->byData[40]);
					m_pThis->m_pAntirootkitScan->SetRootkitTotalNDetectedCount(dwTotalFileCnt, dwTotalDriveCnt, dwTotalProcCnt, dwDetectedFileCnt, dwDetectedDriveCnt, dwDetectedProcCnt);
				}
			}
			g_objCommServer.SendResponse(lpSpyData);
			break;
		case RELOAD_REGISTARTION_DAYS:
			theApp.GetDaysLeft();
			//m_pThis->StartiSpyAVTray(); //issue no : 816 neha gharge 30/6/2014
			m_pThis->MainDialogDisplay();

			break;
		case EXIT_APPLICATION:
			lpSpyData->dwValue = 0;
			//Issue fix: 1221 Critical data operation" pop-up appearing twice at the same time.
			//Resolved by: Nitin K Date: 8th Jan 2016
			/*if (m_pThis->IsAnyPopUpDisplayed())
			{
				HWND hWnd = m_pThis->GetSafeHwnd();
				::ShowWindow(hWnd, SW_RESTORE);
				::BringWindowToTop(hWnd);
				::SetForegroundWindow(hWnd);
				g_objCommServer.SendResponse(lpSpyData);
				return ;
			}*/
			m_pThis->m_bisUiCloseCalled = true;
			if(m_pThis->HandleCloseButton())
			{
				lpSpyData->dwValue = 1;
			}
						
			g_objCommServer.SendResponse(lpSpyData);
			//resolved by lalit 5-5-05, issue- if in case of Ui close from tray, system tray not getting close due to exit(0),here i send message to tray then making exit
			if (m_pThis->m_bisUiCloseCalled && lpSpyData->dwValue == 1)
			{
			m_pThis->m_bisUiCloseCalled = false;
			exit(0);
			}
			m_pThis->m_bisUiCloseCalled = false;
			break;
		case UPDATE_FINISHED :
			if(m_pThis->m_pUpdate != NULL)
			{
				m_pThis->m_pUpdate->m_dlgISpyUpdatesSecond.ShowUpdateCompleteMessage();
			}
			//Issue: If Any operation is in progress and we want to Right click Crypt operation then to stop all Process and Srart Crypt operation
			//Resolved by : Nitin K. Date: 15th July 2015.
			if (theApp.m_hCryptOprEvent)
			{
				SetEvent(theApp.m_hCryptOprEvent);
			}
			break;
		case SETTOTALFILESIZE:
		case SETUPDATESTATUS:
		case SETMESSAGE:
		case SETDOWNLOADPERCENTAGE:
			if (m_pThis->m_pUpdate != NULL)
			{
				m_pThis->m_pUpdate->m_dlgISpyUpdatesSecond.UpDateDowloadStatus(lpSpyData);
			}
			break;
				//Varada Ikhar, Date: 14/02/2015, Issue: Database needs to be updated.Database not valid.
		case SCAN_FINISHED_SIGNATUREFAILED:
			if(m_pThis->m_pQuickscanDlg != NULL)
			{
				if(m_pThis->m_pQuickscanDlg->m_bQuickScan == true)
				{
					m_pThis->m_pQuickscanDlg->m_bSignatureFailed = true;
				}
			}
			if(m_pThis->m_pFullScanDlg != NULL)
			{
				if(m_pThis->m_pFullScanDlg->m_bFullScan == true)
				{
					m_pThis->m_pFullScanDlg->m_bSignatureFailed = true;
				}
			}
			if(m_pThis->m_pCustomScanDlg != NULL)
			{
				if(m_pThis->m_pCustomScanDlg->m_bCustomscan == true)
				{
					m_pThis->m_pCustomScanDlg->m_bSignatureFailed = true;
				}
			}
			break;
		case DATA_ENC_DEC_OPERATIONS:
			if (m_pThis->m_pDataEncryption != NULL)
			{
				if (m_pThis->IsAnyPopUpDisplayed())
				{
					HWND hWnd = m_pThis->GetSafeHwnd();
					::ShowWindow(hWnd, SW_RESTORE);
					::BringWindowToTop(hWnd);
					::SetForegroundWindow(hWnd);
					return;
				}

				theApp.m_csDataCryptFilePath = lpSpyData->szFirstParam;	
					if (m_pThis->IsAnyTaskInProcess())
					{
						//Issue: If Any operation is in progress and we want to Right click Crypt operation then to stop all Process and Srart Crypt operation
						//Resolved by : Nitin K. Date: 15th July 2015.
						if (m_pThis->m_iRunningProcessNmB == DATA_ENCRYPTION_DLG)
						{
							m_pThis->m_pDataEncryption->PauseEncryptionDecryption();
							Sleep(200);
							m_pThis->DispDataOprAlreadyInProgressMsg();
							m_pThis->m_pDataEncryption->ResumeEncryptionDecryption();
							Sleep(200);
							return;
						}
						if (m_pThis->m_iRunningProcessNmB == WARDWIZ_UPDATES_DLG)
						{
							theApp.m_bOnCloseFromMainUI = true;
						}
						if (m_pThis->StopRunningProcess())
						{
							WaitForSingleObject(theApp.m_hCryptOprEvent, INFINITE);
							theApp.m_csDataCryptFilePath = lpSpyData->szFirstParam;
							theApp.m_bDataCryptOpr = true;
							theApp.m_iDataOpr = lpSpyData->dwValue;
							m_pThis->ShowDataCryptOpr();
						}
					}
					else
					{
						theApp.m_csDataCryptFilePath = lpSpyData->szFirstParam;
						theApp.m_bDataCryptOpr = true;
						theApp.m_iDataOpr = lpSpyData->dwValue;
						m_pThis->ShowDataCryptOpr();
					}
			}
			theApp.m_bOnCloseFromMainUI = false;
			break;
		case DATA_ENC_DEC_SHOW_STATUS:
			{
			theApp.m_csDataCryptFilePath = lpSpyData->szFirstParam;
			//issue - Save as handling if encrypted file already exist with same name
			// lalit kumawat 7-2-2015
				bool dwIsSaveAS = lpSpyData->dwValue == SAVE_AS ? true : false;

			m_pThis->UpdateDataCryptOpr(lpSpyData);

			if (dwIsSaveAS)
			{
				g_objCommServer.SendResponse(lpSpyData);
			}
		}
		break;
		case MEMSCANFINISHED:
			if (m_pThis->m_pQuickscanDlg != NULL)
			{
				if (m_pThis->m_pQuickscanDlg->m_bQuickScan == true)
				{
					m_pThis->m_pQuickscanDlg->MemScanningFinished();
				}
			}
			if (m_pThis->m_pFullScanDlg != NULL)
			{
				if (m_pThis->m_pFullScanDlg->m_bFullScan == true)
				{
					m_pThis->m_pFullScanDlg->MemScanningFinished();
				}
			}
			if (m_pThis->m_pCustomScanDlg != NULL)
			{
				if (m_pThis->m_pCustomScanDlg->m_bCustomscan == true)
				{
					m_pThis->m_pCustomScanDlg->MemScanningFinished();
				}
			}
				g_objCommServer.SendResponse(lpSpyData);
			break;
		case SENDPRODUCTEXPTOTRAY:
			if (m_pThis->m_pHomepageDlg != NULL)
			{
				theApp.GetDaysLeft();
				m_pThis->ShowHomeButtonPage();
				m_pThis->MainDialogDisplay();
			}
			break;
		case CHECKLIVEUPDATETRAY :
			m_pThis->OnLiveUpdateTrayCall();
			break;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in OnDataReceiveCallBack", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : OnClose
  Description    : The framework calls this member function as a signal 
				   that the CWnd or an application is to terminate.
  Author Name    : Neha gharge
  SR_NO			 :
  Date           : 24th April 2014
****************************************************************************/
void CISpyGUIDlg::OnClose()
{
	OnBnClickedButtonClose();
}


/***************************************************************************
  Function Name  : InitTabDialog
  Description    : Initialized tab control
  Author Name    : Neha gharge
  SR_NO			 :
  Date           : 24th April 2014
****************************************************************************/
BOOL CISpyGUIDlg::InitTabDialog()
{
	//create the TabDialog

	m_pTabDialog = new CTabDialog(IDD_TABDLG, this);
	if(m_pTabDialog->Create(IDD_TABDLG, this) == FALSE)
	{
		return FALSE;
	}
	

	if(!AddPagesToTabDialog())
	{
		return FALSE;
	}

	CRect objMainRect;
	GetWindowRect(&objMainRect);
	ScreenToClient(&objMainRect);

	//get TabDialog's position from the static control
	CRect oRect;
	m_dlgPos.GetWindowRect(&oRect);
	ScreenToClient(&oRect);
	m_dlgPos.MoveWindow(oRect.left, oRect.top, oRect.Width(), oRect.Height());
	m_dlgPos.ShowWindow(SW_HIDE);

	m_pTabDialog->m_iDlgCx = objMainRect.Width(); //- oRect.left - 47  - 37/*- 1*/;
	m_pTabDialog->m_iDlgCy = objMainRect.Height() - 129;// - BUTTON_HEIGHT + 10 + 37 + 40 + 6;

	//set the TabDialog's positon
	m_pTabDialog->SetWindowPos(this, objMainRect.left - 2 , objMainRect.top + 97,objMainRect.Width() +2, objMainRect.Height(),SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
	m_pTabDialog->MoveWindow(objMainRect.left - 2 , objMainRect.top + 97, objMainRect.Width()+2, objMainRect.Height());
	//initialize the showing of TabDialog
	m_pTabDialog->InitPagesShow();

	//CRect objMainRect;
	//this->GetClientRect(&objMainRect);
	////GetWindowRect(&objMainRect);
	////ScreenToClient(&objMainRect);

	////get TabDialog's position from the static control
	////CRect oRect;
	////m_dlgPos.GetWindowRect(&oRect);
	////ScreenToClient(&oRect);
	////m_dlgPos.MoveWindow(oRect.left, oRect.top +97, 150 ,10);

	////m_pTabDialog->m_iDlgCx = objMainRect.Width() -80;//  - 37/*- 1*/;
	////m_pTabDialog->m_iDlgCy = objMainRect.Height() - 90 ;// + 10 + 37 + 40 + 6;

	////set the TabDialog's positon
	//
	//m_pTabDialog->SetWindowPos(&wndTop, objMainRect.left + 0, objMainRect.top + 97,objMainRect.Width()+150,objMainRect.Height()+600,SWP_NOSIZE| SWP_NOZORDER | SWP_SHOWWINDOW);
	////m_pTabDialog->MoveWindow(objMainRect.left +0, objMainRect.top + 97, 150, 600,0);
	////initialize the showing of TabDialog
	//m_pTabDialog->InitPagesShow();

	return TRUE;
}


/***************************************************************************
  Function Name  : AddPagesToTabDialog
  Description    : Add button and respective dialogs
  Author Name    : Neha gharge
  SR_NO			 :
  Date           : 24th April 2014
****************************************************************************/
BOOL CISpyGUIDlg::AddPagesToTabDialog()
{
	try
	{
		switch(theApp.m_dwProductID)
		{
			case ESSENTIAL:
						ShowControlsForEssentialEdition();
						break;
			case PRO:
						ShowControlsForProEdition();
						break;
			case ELITE:
						ShowControlsForProEdition();
						break;
			case BASIC:
						ShowControlsForBasicEdition();
						break;
			default:
				AddLogEntry(L"### Invalid product ID in CGenXGUIDlg::AddPagesToTabDialog", 0, 0, true, SECONDLEVEL);
		}
		return TRUE;
	}
	catch(...)
	{
		return FALSE;
	}
}


/***************************************************************************
  Function Name  : RefreshStrings
  Description    : Refresh all strings
  Author Name    : Neha gharge
  SR_NO			 :
  Date           : 24th April 2014
  Modification   : 28th may 2014
****************************************************************************/
bool CISpyGUIDlg::RefreshStrings()
{
	DWORD OsType = m_objOSVersionWrap.DetectClientOSVersion();
	CString csStaticText , csButtonText;
	switch(OsType)
	{
		case WINOS_95:
				break;
		case WINOS_98:
				break;
		case WINOS_2000:
				break;
		case WINOS_NT:
				break;
		case WINOS_VISTA:
		case WINOS_XP:
		case WINOS_XP64:   //Name:Varada Ikhar, Date:13/01/2015, Version: 1.8.3.12, Issue No:1, Description:In Windows XP for Basic ,after clicking ok on tooltip gives dialog "new os or old os".
		case WINOS_WIN7:
				switch(theApp.m_dwSelectedLangID)
				{
				case 0:
					csStaticText = L"     ";
					csButtonText = L"         ";
					break;
				case 1:
					csStaticText = L"  ";
					csButtonText = L"     ";
					break;
				case 2:
					csStaticText = L"     ";
					csButtonText = L"         ";
					break;
				case 3:
					csStaticText = L"     ";
					csButtonText = L"         ";
					break;
				case 4:
					csStaticText = L"     ";
					csButtonText = L"         ";
					break;
				case 5:
					csStaticText = L"     ";
					csButtonText = L"         ";
					break;
				}
				break;
		case WINOS_WIN8:
				switch(theApp.m_dwSelectedLangID)
				{
				case 0:
					csStaticText = L"     ";
					csButtonText = L"         ";
					break;
				case 1:
					csStaticText = L"    ";
					csButtonText = L"         ";
					break;
				case 2:
					csStaticText = L"     ";
					csButtonText = L"         ";
					break;
				case 3:
					csStaticText = L"     ";
					csButtonText = L"         ";
					break;
				case 4:
					csStaticText = L"     ";
					csButtonText = L"         ";
					break;
				case 5:
					csStaticText = L"     ";
					csButtonText = L"         ";
					break;
				}
				break;
		case WINOS_WIN8_1:
				switch(theApp.m_dwSelectedLangID)
				{
				case 0:
					csStaticText = L"     ";
					csButtonText = L"         ";
					break;
				case 1:
					csStaticText = L"    ";
					csButtonText = L"         ";
					break;
				case 2:
					csStaticText = L"     ";
					csButtonText = L"         ";
					break;
				case 3:
					csStaticText = L"     ";
					csButtonText = L"         ";
					break;
				case 4:
					csStaticText = L"     ";
					csButtonText = L"         ";
					break;
				case 5:
					csStaticText = L"     ";
					csButtonText = L"         ";
					break;
				}
				break;
		case WINOSUNKNOWN_OR_NEWEST:
				MessageBox(L"NEWOS_OR_OLDOS",theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_OK|MB_ICONINFORMATION);
				break;
	}

			if(m_stScan != NULL)
			{
				m_stScan->SetWindowTextW(csStaticText + theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SCAN"));
			}
			if(m_btnVirusScan != NULL)
			{
				m_btnVirusScan->SetTextAlignment(TA_LEFT);
				m_btnVirusScan->SetTextAlignment(TA_NOUPDATECP);
			}
			if(m_btnQuickScan != NULL)
			{
				m_btnQuickScan->SetWindowText(csButtonText + theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_QUICKSCAN"));
				m_btnQuickScan->SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_QUICKSCAN"));
			}
			
			if(m_btnFullScan != NULL)
			{
				m_btnFullScan->SetWindowText(csButtonText + theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FULLSCAN"));
				m_btnFullScan->SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FULLSCAN"));
			}
			if(m_btnCustomScan != NULL)
			{
				m_btnCustomScan->SetWindowText(csButtonText + theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_CUSTOMSCAN"));
				m_btnCustomScan->SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_CUSTOMSCAN"));
			}
			if(m_stTool != NULL)
			{
				m_stTool->SetWindowTextW(csStaticText + theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TOOL"));
			}
			if(m_stAdminstration != NULL)
			{
				m_stAdminstration->SetWindowTextW(csStaticText + theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ADMINSTRATION"));
			}
			if(m_btnRegistryOptimizer != NULL)
			{
				m_btnRegistryOptimizer->SetWindowText(csButtonText + theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_REGOPT"));
				m_btnRegistryOptimizer->SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_REGOPT"));
			}
			if(m_btnDataEncryption != NULL)
			{
				m_btnDataEncryption->SetWindowText(csButtonText + theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_DATAENCRYPT"));
				m_btnDataEncryption->SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_DATAENCRYPT"));
			}
			if(m_btnRecover != NULL)
			{
				m_btnRecover->SetWindowText(csButtonText + theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RECOVER"));
				m_btnRecover->SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RECOVER"));
			}
			if (m_btnUtility != NULL)
			{
				m_btnUtility->SetWindowText(csButtonText + theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UTILITY_HEADER"));
				m_btnUtility->SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UTILITY_HEADER"));
			}
			if(m_btnFolderLocker != NULL)
			{
				m_btnFolderLocker->SetWindowText(csButtonText + theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FOLDERLOCK"));
				m_btnFolderLocker->SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FOLDERLOCK"));
			}
			if(m_stEmailScan != NULL)
			{
				m_stEmailScan->SetWindowTextW(csStaticText + theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_EMAIL"));
			}
			if(m_btnVirusScan != NULL)
			{
				m_btnVirusScan->SetWindowText(csButtonText+ theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_VIRUSSCAN"));
				m_btnVirusScan->SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_VIRUSSCAN"));
			}
			if(m_btnSpamFilter != NULL)
			{	
				m_btnSpamFilter->SetWindowText(csButtonText + theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SPAMFILTER"));
				m_btnSpamFilter->SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SPAMFILTER"));
			}
			if(m_btnContentFilter != NULL)
			{
				m_btnContentFilter->SetWindowText(csButtonText+ theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_CONTENTFILTER"));
				m_btnContentFilter->SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_CONTENTFILTER"));
			}
			if(m_btnSignature != NULL)
			{
				m_btnSignature->SetWindowText(csButtonText + theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SIGNATURE"));
				m_btnSignature->SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_SIGNATURE"));
			}
			if(m_btnAntirootkitScan != NULL)
			{
				m_btnAntirootkitScan->SetWindowText(csButtonText+ theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ANTIROOTKITSCAN"));
				m_btnAntirootkitScan->SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ANTIROOTKITSCAN"));
			}
			if(m_btnUpdateNew != NULL)
			{
				m_btnUpdateNew->SetWindowText(csButtonText + theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_UPDATES"));
				m_btnUpdateNew->SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_UPDATES"));
			}
			if(m_btnReports != NULL)
			{
				m_btnReports->SetWindowText(csButtonText + theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_REPORT"));
				m_btnReports->SetToolTipText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_REPORT"));
			}
			m_btnSupport.SetWindowTextW(csButtonText + theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_HOMEPAGE_TOOLTIP_SUPPORT"));
			//changes neha 04 july
			//All rights reserved text not coming proper for OS with other language.
			CString csFooter;
			//csFooter.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOOTER_MSG"),174);//ASCII value for @
			//m_stFooterMsg.SetWindowTextW(csStaticText + csFooter);
			csFooter = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOOTER_MSG");
			csFooter+= _T(" \u00AE ");
			//Added by Nitin K.
			//Issue: 5	Wardwiz AV showing year as 2014
			//Date : 15-Jan-2015
			csFooter += theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOOTER_MSG_YEAR");
			m_stFooterMsg.SetWindowTextW(csFooter);


	return true;
}


/**************************************************************************************
  Function Name  : ReadEmailScanEntryFromRegistry
  Description    : Read Email Scan Entry from registry show controls on main Ui accordingly
  Author Name    : Neha gharge
  SR_NO			 :
  Date           : 24th April 2014
****************************************************************************************/
DWORD CISpyGUIDlg::ReadEmailScanEntryFromRegistry()
{
	DWORD EmailScanSetting = 0;
	HKEY key;
	DWORD szvalueEmailScan;
	DWORD dwvaluelengthEmailScan = sizeof(DWORD);
	DWORD dwtypeEmailScan = REG_DWORD;

	if(RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Wardwiz Antivirus"),&key)!= ERROR_SUCCESS)
	{
		AddLogEntry(L"### Unable to open registry key", 0, 0, true, SECONDLEVEL);
		return EmailScanSetting;
	}
	
	long ReadReg=RegQueryValueEx(key, L"dwEmailScan", NULL ,&dwtypeEmailScan,(LPBYTE)&szvalueEmailScan, &dwvaluelengthEmailScan);
	if(ReadReg == ERROR_SUCCESS)
	{
		EmailScanSetting = (DWORD)szvalueEmailScan;
	}	
	RegCloseKey(key);

	return EmailScanSetting;
}


/**************************************************************************************
  Function Name  : MainDialogDisplay
  Description    : It will disaply after enable/disable email scan from tray.
  Author Name    : Neha gharge
  SR_NO			 :
  Date           : 24th April 2014
****************************************************************************************/
void CISpyGUIDlg::MainDialogDisplay()
{
	//Ram, Issue No: 0001216, Resolved
	if (theApp.m_dwDaysLeft == 0)
	{
		theApp.GetDaysLeft();
	}

	// ISsue no 691 neha gharge 13/6/2014
	if(theApp.m_dwDaysLeft == 0)
	{
		m_bUnRegisterProduct = true;
	}
	else
	{
		m_bUnRegisterProduct = false;
	}
	//Issue Neha Gharge 6/5/2014*********************/	
	bool bOutlookInstalled = false;
	if(m_pVirusscan != NULL)
	{
		bOutlookInstalled = m_pVirusscan->GetExistingPathofOutlook();
	}
	DWORD dwEmailScanEnable = ReadEmailScanEntryFromRegistry();
	if(m_pTabDialog != NULL)
	{
		m_pTabDialog->m_dwEmailScanEnable = dwEmailScanEnable;
		m_pTabDialog->m_bOutlookInstalled = bOutlookInstalled;
		m_pTabDialog->m_bUnRegisterProduct = m_bUnRegisterProduct;
	}

	if(dwEmailScanEnable == 1)
	{
		if(bOutlookInstalled)
		{
			if(m_bUnRegisterProduct == false)
			{
				if(m_btnVirusScan != NULL)
				{
					m_btnVirusScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0,0,0,0,0);
					m_btnVirusScan->SetTextAlignment(TA_LEFT);
					m_btnVirusScan->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
					m_btnVirusScan->SetFocusTextColor(RGB(4,4,4));
					m_btnVirusScan->EnableWindow(true);
				}
				if(m_btnSpamFilter != NULL)
				{
					m_btnSpamFilter->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0,0,0,0,0);
					m_btnSpamFilter->SetTextAlignment(TA_LEFT);
					m_btnSpamFilter->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
					m_btnSpamFilter->SetFocusTextColor(RGB(4,4,4));
					m_btnSpamFilter->EnableWindow(true);
				}
				if(m_btnContentFilter != NULL)
				{
					m_btnContentFilter->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0,0,0,0,0);
					m_btnContentFilter->SetTextAlignment(TA_LEFT);
					m_btnContentFilter->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
					m_btnContentFilter->SetFocusTextColor(RGB(4,4,4));
					m_btnContentFilter->EnableWindow(true);
				}
				if(m_btnSignature != NULL)
				{
					m_btnSignature->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0,0,0,0,0);
					m_btnSignature->SetTextAlignment(TA_LEFT);
					m_btnSignature->SetTextColorA(RGB(255,255,255),RGB(255,255,255),1);
					m_btnSignature->SetFocusTextColor(RGB(4,4,4));
					m_btnSignature->EnableWindow(true);
				}
				dwEmailScanEnable = 1;
			}
			else
			{
				dwEmailScanEnable = 0;
			}
		}
		else
		{
			if(m_btnVirusScan != NULL)
			{
				m_btnVirusScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, 0,0,0,0,0);
				m_btnVirusScan->SetTextAlignment(TA_LEFT);
				m_btnVirusScan->SetTextColorA(RGB(150,150,150),RGB(150,150,150),RGB(150,150,150));
				m_btnVirusScan->SetFocusTextColor(RGB(150,150,150));
				m_btnVirusScan->EnableWindow(true);
			}
			if(m_btnSpamFilter != NULL)
			{
				m_btnSpamFilter->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, 0,0,0,0,0);
				m_btnSpamFilter->SetTextAlignment(TA_LEFT);
				m_btnSpamFilter->SetTextColorA(RGB(150,150,150),RGB(150,150,150),RGB(150,150,150));
				m_btnSpamFilter->SetFocusTextColor(RGB(150,150,150));
				m_btnSpamFilter->EnableWindow(true);
			}
			if(m_btnContentFilter != NULL)
			{
				m_btnContentFilter->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, 0,0,0,0,0);
				m_btnContentFilter->SetTextAlignment(TA_LEFT);
				m_btnContentFilter->SetTextColorA(RGB(150,150,150),RGB(150,150,150),RGB(150,150,150));
				m_btnContentFilter->SetFocusTextColor(RGB(150,150,150));
				m_btnContentFilter->EnableWindow(true);
			}
			if(m_btnSignature != NULL)
			{
				m_btnSignature->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, 0,0,0,0,0);
				m_btnSignature->SetTextAlignment(TA_LEFT);
				m_btnSignature->SetTextColorA(RGB(150,150,150),RGB(150,150,150),RGB(150,150,150));
				m_btnSignature->SetFocusTextColor(RGB(150,150,150));
				m_btnSignature->EnableWindow(true);
			}
		}
	}
	if(dwEmailScanEnable == 0)
	{
		if(m_btnVirusScan != NULL)
		{
			m_btnVirusScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, 0,0,0,0,0);
			m_btnVirusScan->SetTextAlignment(TA_LEFT);
			m_btnVirusScan->SetTextColorA(RGB(150,150,150),RGB(150,150,150),RGB(150,150,150));
			m_btnVirusScan->SetFocusTextColor(RGB(150,150,150));
			m_btnVirusScan->EnableWindow(true);
		}
		if(m_btnSpamFilter != NULL)
		{
			m_btnSpamFilter->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, 0,0,0,0,0);
			m_btnSpamFilter->SetTextAlignment(TA_LEFT);
			m_btnSpamFilter->SetTextColorA(RGB(150,150,150),RGB(150,150,150),RGB(150,150,150));
			m_btnSpamFilter->SetFocusTextColor(RGB(150,150,150));
			m_btnSpamFilter->EnableWindow(true);
		}
		if(m_btnContentFilter != NULL)
		{
			m_btnContentFilter->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, 0,0,0,0,0);
			m_btnContentFilter->SetTextAlignment(TA_LEFT);
			m_btnContentFilter->SetTextColorA(RGB(150,150,150),RGB(150,150,150),RGB(150,150,150));
			m_btnContentFilter->SetFocusTextColor(RGB(150,150,150));
			m_btnContentFilter->EnableWindow(true);
		}
		if(m_btnSignature != NULL)
		{
			m_btnSignature->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, 0,0,0,0,0);
			m_btnSignature->SetTextAlignment(TA_LEFT);
			m_btnSignature->SetTextColorA(RGB(150,150,150),RGB(150,150,150),RGB(150,150,150));
			m_btnSignature->SetFocusTextColor(RGB(150,150,150));
			m_btnSignature->EnableWindow(true);
		}
	}
	Invalidate();
}

/***************************************************************************
  Function Name  : OnBnClickedButtonInformation
  Description    : Show dialogs of product information.
  Author Name    : Neha gharge
  SR_NO			 :
  Date           : 28th may 2014
  Modification   : 28th may 2014
  Modification	 : 28th may 2015 no need to remove the focus from last functionlity
  when we clicked on in button other than tab button and home button.
****************************************************************************/
void CISpyGUIDlg::OnBnClickedButtonInformation()
{
	// TODO: Add your control notification handler code here
	//SetSkinOnhomePage();
	m_bIsPopUpDisplayed = true;
	theApp.ShowProductInformation();
	m_bIsPopUpDisplayed = false;
}

/***************************************************************************
  Function Name  : ShowControlsForEssentialEdition
  Description    : Show dialogs according to essential editions
  Author Name    : Neha gharge
  SR_NO			 :
  Date           : 28th may 2014
  Modification   : 28th may 2014
  Modification	 : 28th May 2015, Neha Gharge added Utility button and dialog.
  Modification   : 27th June 2015, Neha Gharge shifting of menus accroding 1.12.0.0 designs
****************************************************************************/

BOOL CISpyGUIDlg::ShowControlsForEssentialEdition()
{
		RECT oRcBtnRect;

		if(m_btnInitialHome != NULL)
		{
			delete m_btnInitialHome;
			m_btnInitialHome = NULL;
		}
		m_btnInitialHome = new CxSkinButton();
		m_btnInitialHome->Create(_T(" "), WS_CHILD | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_HOME_PAGE);

		//create quick scan dialog
		if(m_pHomepageDlg != NULL)
		{
			delete m_pHomepageDlg;
			m_pHomepageDlg = NULL;
		}
		m_pHomepageDlg = new CWardWizHomePage(m_pTabDialog);
		if(m_pHomepageDlg->Create(IDD_DIALOG_HOME_PAGE, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}

		//Add quick scan in to lookup table
		m_pTabDialog->AddPage(m_pHomepageDlg, m_btnInitialHome);
			
		if(m_stScan != NULL)
		{
			delete m_stScan;
			m_stScan = NULL;
		}
		m_stScan = new CColorStatic();
		oRcBtnRect.left = BTN_LOCATION.x  ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 10 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + 17;

		m_stScan->Create(L"Static",WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED,oRcBtnRect,m_pTabDialog,ID_STATIC_SCAN);
		m_stScan->SetBkColor(RGB(65,65,67));
		m_stScan->SetFont(&theApp.m_fontWWTextNormal);
		m_stScan->SetTextColor(RGB(255,255,255));

		////create Quick Scan button
		if(m_btnQuickScan != NULL)
		{
			delete m_btnQuickScan;
			m_btnQuickScan = NULL;
		}
		m_btnQuickScan = new CxSkinButton();

		CPoint objBtnPoint = BTN_LOCATION;
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 27 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnQuickScan->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_QUICK_SCAN);
		m_btnQuickScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17,IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnQuickScan->SetTextAlignment(TA_LEFT);
		m_btnQuickScan->SetTextColorA(RGB(255,255,255),0,1);
		m_btnQuickScan->SetFont(&theApp.m_fontWWTextNormal);

		//create quick scan dialog
		if(m_pQuickscanDlg == NULL)
		{
			m_pQuickscanDlg = CScanDlg::GetScanDlgInstance(m_pTabDialog);
			m_pQuickscanDlg->m_bQuickScan = true;
			if(m_pQuickscanDlg->Create(IDD_DIALOG_SCAN, m_pTabDialog) == FALSE)
			{
				return FALSE;
			}
			//Add quick scan in to lookup table
			m_pTabDialog->AddPage(m_pQuickscanDlg, m_btnQuickScan);
		}

		//full scan
		if(m_btnFullScan != NULL)
		{
			delete m_btnFullScan;
			m_btnFullScan = NULL;
		}
		m_btnFullScan = new CxSkinButton();
		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 27 + 17;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnFullScan->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_FULL_SCAN);

		m_btnFullScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17,IDB_BITMAP_150x17_DISABLE,0, 0, 0, 0, 0);
		m_btnFullScan->SetTextAlignment(TA_LEFT);
		m_btnFullScan->SetTextColorA(RGB(255,255,255),0,1);
		m_btnFullScan->SetFocusTextColor(RGB(4,4,4));
		m_btnFullScan->SetFont(&theApp.m_fontWWTextNormal);
		
		//create full scan dialog 
		if(m_pFullScanDlg == NULL)
		{
			m_pFullScanDlg = CScanDlg::GetScanDlgInstance(m_pTabDialog);
			m_pFullScanDlg->m_bFullScan = true;
			m_pTabDialog->AddPage(m_pFullScanDlg, m_btnFullScan);
		}		
 
		//Custom scan
		if(m_btnCustomScan != NULL)
		{
			delete m_btnCustomScan;
			m_btnCustomScan = NULL;
		}
		m_btnCustomScan = new CxSkinButton();
		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 44 +17;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnCustomScan->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_CUSTOM_SCAN);

		m_btnCustomScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17,IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnCustomScan->SetTextAlignment(TA_LEFT);
		m_btnCustomScan->SetTextColorA(RGB(255,255,255),0,1);
		m_btnCustomScan->SetFocusTextColor(RGB(4,4,4));
		m_btnCustomScan->SetFont(&theApp.m_fontWWTextNormal);
		//create custom scan dialog
		/**************************************************************************************************
		Please add dialog classs pointer to add into loolup table For timebeing i add fullscan
		****************************************************************************************************/

		if(m_pCustomScanDlg == NULL)
		{
			m_pCustomScanDlg = CScanDlg::GetScanDlgInstance(m_pTabDialog);
			m_pCustomScanDlg->m_bCustomscan = true;
			//Add custom scan in to lookup table
			m_pTabDialog->AddPage(m_pCustomScanDlg, m_btnCustomScan);
		}

		//Antiroot kit
		if(m_btnAntirootkitScan != NULL)
		{
			delete m_btnAntirootkitScan;
			m_btnAntirootkitScan = NULL;
		}
		m_btnAntirootkitScan = new CxSkinButton();
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 78;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnAntirootkitScan->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_ANTIROOTKIT);
	  //issue:25 resolved by neha ,Two options get selected if user drag and select the option.	
	   m_btnAntirootkitScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnAntirootkitScan->SetTextAlignment(TA_LEFT);
		//m_btnAntirootkitScan->SetWindowText(L"         Antirootkit Scan");
		m_btnAntirootkitScan->SetTextColorA(RGB(255,255,255),0,1);
		m_btnAntirootkitScan->SetFocusTextColor(RGB(4,4,4));
		m_btnAntirootkitScan->SetFont(&theApp.m_fontWWTextNormal);

		//create Antiroot kit dialog
		if(m_pAntirootkitScan != NULL)
		{
			delete m_pAntirootkitScan;
			m_pAntirootkitScan = NULL;
		}
		m_pAntirootkitScan = new CISpyAntiRootkit(m_pTabDialog);
		if(m_pAntirootkitScan->Create(IDD_DIALOG_ANTIROOTKIT, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Antiroot kit in to lookup table
		m_pTabDialog->AddPage(m_pAntirootkitScan, m_btnAntirootkitScan);


		//Tools static
		if(m_stTool != NULL)
		{
			delete m_stTool;
			m_stTool = NULL;
		}
		m_stTool = new CColorStatic();
		oRcBtnRect.left = BTN_LOCATION.x  ;
		oRcBtnRect.right = BTN_LOCATION.x + 150;
		oRcBtnRect.top = BTN_LOCATION.y + 102 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + 17;

		m_stTool->Create(L"Static",WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED,oRcBtnRect,m_pTabDialog,ID_STATIC_TOOLS);
		m_stTool->SetBkColor(RGB(65,65,67));
		m_stTool->SetFont(&theApp.m_fontWWTextNormal);
		m_stTool->SetTextColor(RGB(255,255,255));

		//Registry Optimizer
		if(m_btnRegistryOptimizer != NULL)
		{
			delete m_btnRegistryOptimizer;
			m_btnRegistryOptimizer = NULL;
		}
		m_btnRegistryOptimizer = new CxSkinButton();

		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 119  ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnRegistryOptimizer->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_REGISTYOPTIMIZER);
		//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnRegistryOptimizer->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17,IDB_BITMAP_150x17_DISABLE,0, 0, 0, 0, 0);
		m_btnRegistryOptimizer->SetTextAlignment(TA_LEFT);
		//m_btnRegistryOptimizer->SetWindowText(L"         Registry Optimizer");
		m_btnRegistryOptimizer->SetTextColorA(RGB(255,255,255),0,1);
		m_btnRegistryOptimizer->SetFocusTextColor(RGB(4,4,4));
		m_btnRegistryOptimizer->SetFont(&theApp.m_fontWWTextNormal);

		//create register optimizer dialog
		if(m_pRegistryOptimizer != NULL)
		{
			delete m_pRegistryOptimizer;
			m_pRegistryOptimizer = NULL;
		}
		m_pRegistryOptimizer = new CRegistryOptimizerDlg(m_pTabDialog);
		if(m_pRegistryOptimizer->Create(IDD_DIALOG_REGISTRYOPTIMIZER, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add registry optimizer in to lookup table
		m_pTabDialog->AddPage(m_pRegistryOptimizer, m_btnRegistryOptimizer);

		////data Encryption
		if(m_btnDataEncryption != NULL)
		{
			delete m_btnDataEncryption;
			m_btnDataEncryption = NULL;
		}
		m_btnDataEncryption = new CxSkinButton();
		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 119 + 17;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnDataEncryption->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_DATAENCRYPTION);
		//issue:25 resolved by neha ,Two options get selected if user drag and select the option.	
     	m_btnDataEncryption->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnDataEncryption->SetTextAlignment(TA_LEFT);
		//m_btnDataEncryption->SetWindowText(L"         Data Encryption");
		m_btnDataEncryption->SetTextColorA(RGB(255,255,255),0,1);
		m_btnDataEncryption->SetFocusTextColor(RGB(4,4,4));
		m_btnDataEncryption->SetFont(&theApp.m_fontWWTextNormal);
		
		//create data encryption dialog
		if(m_pDataEncryption != NULL)
		{
			delete m_pDataEncryption;
			m_pDataEncryption = NULL;
		}
		m_pDataEncryption = new CDataEncryptionDlg(m_pTabDialog);
		if(m_pDataEncryption->Create(IDD_DIALOG_DATAENCRYPTION, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add data encryption in to lookup table
		m_pTabDialog->AddPage(m_pDataEncryption, m_btnDataEncryption);


		//Recover
		if(m_btnRecover != NULL)
		{
			delete m_btnRecover;
			m_btnRecover = NULL;
		}
		m_btnRecover = new CxSkinButton();
		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 153;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnRecover->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_RECOVER);
		//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnRecover->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnRecover->SetTextAlignment(TA_LEFT);
		//m_btnRecover->SetWindowText(L"         Recover");
		m_btnRecover->SetTextColorA(RGB(255,255,255),0,1);
		m_btnRecover->SetFocusTextColor(RGB(4,4,4));
		m_btnRecover->SetFont(&theApp.m_fontWWTextNormal);
		
		//create recover dialog
		if(m_pRecover != NULL)
		{
			delete m_pRecover;
			m_pRecover = NULL;
		}
		m_pRecover = new CISpyRecoverDlg(m_pTabDialog);
		if(m_pRecover->Create(IDD_DIALOG_RECOVER, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add recover in to lookup table
		m_pTabDialog->AddPage(m_pRecover, m_btnRecover);


		//Utility
		if (m_btnUtility != NULL)
		{
			delete m_btnUtility;
			m_btnUtility = NULL;
		}
		m_btnUtility = new CxSkinButton();
		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 177;//250;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;
		m_btnUtility->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_UTILITY_OPTION);
		//issue:25 resolved by neha ,Two options get selected if user drag and select the option.	
		m_btnUtility->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnUtility->SetTextAlignment(TA_LEFT);
		m_btnUtility->SetTextColorA(RGB(255, 255, 255), 0, 1);
		m_btnUtility->SetFocusTextColor(RGB(4, 4, 4));
		m_btnUtility->SetFont(&theApp.m_fontWWTextNormal);

		//create Report dialog
		if (m_pUtility != NULL)
		{
			delete m_pUtility;
			m_pUtility = NULL;
		}
		m_pUtility = new CWardWizUtilitiesDlg(m_pTabDialog);
		if (m_pUtility->Create(IDD_DIALOG_UTILITYDLG, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Report in to lookup table
		m_pTabDialog->AddPage(m_pUtility, m_btnUtility);


		//Administartion
		if(m_stAdminstration != NULL)
		{
			delete m_stAdminstration;
			m_stAdminstration = NULL;
		}
		m_stAdminstration = new CColorStatic();
		oRcBtnRect.left = BTN_LOCATION.x  ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 201;// 176 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + 17;
		m_stAdminstration->Create(L"Static",WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED,oRcBtnRect,m_pTabDialog,ID_STATIC_ADMINSTRATION);
		m_stAdminstration->SetBkColor(RGB(65,65,67));
		m_stAdminstration->SetFont(&theApp.m_fontWWTextNormal);
		m_stAdminstration->SetTextColor(RGB(255,255,255));


		//Update
		if(m_btnUpdateNew != NULL)
		{
			delete m_btnUpdateNew;
			m_btnUpdateNew = NULL;
		}
		m_btnUpdateNew = new CxSkinButton();
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 218;//193;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;
		m_btnUpdateNew->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_UPDATE);
	//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnUpdateNew->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnUpdateNew->SetTextAlignment(TA_LEFT);
		m_btnUpdateNew->SetTextColorA(RGB(255,255,255),0,1);
		m_btnUpdateNew->SetFocusTextColor(RGB(4,4,4));
		m_btnUpdateNew->SetFont(&theApp.m_fontWWTextNormal);

		//create update dialog
		if(m_pUpdate != NULL)
		{
			delete m_pUpdate;
			m_pUpdate = NULL;
		}
		m_pUpdate = new CISpyUpdatesDlg(m_pTabDialog);
		if(m_pUpdate->Create(IDD_DIALOG_UPDATES, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add update in to lookup table
		m_pTabDialog->AddPage(m_pUpdate, m_btnUpdateNew);


		//report
		if(m_btnReports != NULL)
		{
			delete m_btnReports;
			m_btnReports = NULL;
		}
		m_btnReports = new CxSkinButton();
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 235;//210;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;
		m_btnReports->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_REPORTS);
	//issue:25 resolved by neha ,Two options get selected if user drag and select the option.	
    	m_btnReports->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnReports->SetTextAlignment(TA_LEFT);
		//m_btnReports->SetWindowText(L"         Reports");
		m_btnReports->SetTextColorA(RGB(255,255,255),0,1);
		m_btnReports->SetFocusTextColor(RGB(4,4,4));
		m_btnReports->SetFont(&theApp.m_fontWWTextNormal);

		//create Report dialog
		if(m_pReports != NULL)
		{
			delete m_pReports;
			m_pReports = NULL;
		}
		m_pReports = new CISpyReportsDlg(m_pTabDialog);
		if(m_pReports->Create(IDD_DIALOG_REPORTS, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Report in to lookup table
		m_pTabDialog->AddPage(m_pReports, m_btnReports);



		if(m_pQuickscanDlg != NULL)
		{
			m_pQuickscanDlg->m_bQuickScan = false;
		}
		if(m_pFullScanDlg != NULL)
		{
			m_pFullScanDlg->m_bFullScan = false;
		}
		if(m_pCustomScanDlg != NULL)
		{
			m_pCustomScanDlg->m_bCustomscan = false;
		}
		RefreshStrings();
		return TRUE;
}



/***************************************************************************
  Function Name  : ShowControlsForProEdition
  Description    : Show dialogs according to editions
  Author Name    : Neha gharge
  SR_NO			 :
  Date           : 28th may 2014
  Modification   : 28th may 2014
  Modification	 : 28th May 2015, Neha Gharge added Utility button and dialog.
  Modification   : 27th June 2015, Neha Gharge shifting of menus accroding 1.12.0.0 designs
****************************************************************************/

BOOL CISpyGUIDlg::ShowControlsForProEdition()
{
		RECT oRcBtnRect;
		//m_buttonText = new CFont;

		//m_buttonText->CreateFont(15,6,0,0,FW_NORMAL,0,0,0,0,0,0,ANTIALIASED_QUALITY,0,L"Microsoft Sans serif Regular");

		if(m_btnInitialHome != NULL)
		{
			delete m_btnInitialHome;
			m_btnInitialHome = NULL;
		}
		m_btnInitialHome = new CxSkinButton();

		m_btnInitialHome->Create(_T(" "), WS_CHILD | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_HOME_PAGE);
		//create quick scan dialog

		if(m_pHomepageDlg != NULL)
		{
			delete m_pHomepageDlg;
			m_pHomepageDlg = NULL;
		}

		m_pHomepageDlg = new CWardWizHomePage(m_pTabDialog);
		if(m_pHomepageDlg->Create(IDD_DIALOG_HOME_PAGE, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add quick scan in to lookup table
		m_pTabDialog->AddPage(m_pHomepageDlg, m_btnInitialHome);

		if(m_stScan != NULL)
		{
			delete m_stScan;
			m_stScan = NULL;
		}
			
		m_stScan = new CColorStatic();

		oRcBtnRect.left = BTN_LOCATION.x  ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 10 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + 17;

		m_stScan->Create(L"Static",WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED,oRcBtnRect,m_pTabDialog,ID_STATIC_SCAN);


		m_stScan->SetBkColor(RGB(65,65,67));
		m_stScan->SetFont(&theApp.m_fontWWTextNormal);
		m_stScan->SetTextColor(RGB(255,255,255));

		////create Quick Scan button
		if(m_btnQuickScan != NULL)
		{
			delete m_btnQuickScan;
			m_btnQuickScan = NULL;
		}
		m_btnQuickScan = new CxSkinButton();

		CPoint objBtnPoint = BTN_LOCATION;
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 27 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnQuickScan->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_QUICK_SCAN);

		m_btnQuickScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17,IDB_BITMAP_150x17_DISABLE,0, 0, 0, 0, 0);
		m_btnQuickScan->SetTextAlignment(TA_LEFT);
		m_btnQuickScan->SetTextColorA(RGB(255,255,255),0,1);
		m_btnQuickScan->SetFocusTextColor(RGB(4,4,4));
		m_btnQuickScan->SetFont(&theApp.m_fontWWTextNormal);
		m_btnQuickScan->Invalidate();

		//create quick scan dialog
		if(m_pQuickscanDlg == NULL)
		{
			m_pQuickscanDlg = CScanDlg::GetScanDlgInstance(m_pTabDialog);
			m_pQuickscanDlg->m_bQuickScan = true;
			if(m_pQuickscanDlg->Create(IDD_DIALOG_SCAN, m_pTabDialog) == FALSE)
			{
				return FALSE;
			}
			//Add quick scan in to lookup table
			m_pTabDialog->AddPage(m_pQuickscanDlg, m_btnQuickScan);
		}
		//full scan

		if(m_btnFullScan != NULL)
		{
			delete m_btnFullScan;
			m_btnFullScan = NULL;
		}
		m_btnFullScan = new CxSkinButton();

		
		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 27 + 17;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnFullScan->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_FULL_SCAN);

		m_btnFullScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17,IDB_BITMAP_150x17_DISABLE,0, 0, 0, 0, 0);
		m_btnFullScan->SetTextAlignment(TA_LEFT);
		m_btnFullScan->SetTextColorA(RGB(255,255,255),0,1);
		m_btnFullScan->SetFocusTextColor(RGB(4,4,4));
		m_btnFullScan->SetFont(&theApp.m_fontWWTextNormal);

		
		//create full scan dialog 
		if(m_pFullScanDlg == NULL)
		{
			m_pFullScanDlg = CScanDlg::GetScanDlgInstance(m_pTabDialog);
			m_pFullScanDlg->m_bFullScan = true;
			//if(m_pFullScanDlg->Create(IDD_DIALOG_SCAN, m_pTabDialog) == FALSE)
			//{
			//	return FALSE;
			//}
			//	Add full scan in to lookup table
			m_pTabDialog->AddPage(m_pFullScanDlg, m_btnFullScan);
			//	m_pTabDialog->AddPage(m_pQuickscanDlg, m_btnFullScan);
		}
 
		//Custom scan
		if(m_btnCustomScan != NULL)
		{
			delete m_btnCustomScan;
			m_btnCustomScan = NULL;
		}
		m_btnCustomScan = new CxSkinButton();

		
		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 44 +17;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnCustomScan->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_CUSTOM_SCAN);

		m_btnCustomScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17,IDB_BITMAP_150x17_DISABLE,0, 0, 0, 0, 0);
		m_btnCustomScan->SetTextAlignment(TA_LEFT);
		m_btnCustomScan->SetTextColorA(RGB(255,255,255),0,1);
		m_btnCustomScan->SetFocusTextColor(RGB(4,4,4));
		m_btnCustomScan->SetFont(&theApp.m_fontWWTextNormal);
		//create custom scan dialog
		/**************************************************************************************************
		Please add dialog classs pointer to add into loolup table For timebeing i add fullscan
		****************************************************************************************************/
		if(m_pCustomScanDlg == NULL)
		{
			m_pCustomScanDlg = CScanDlg::GetScanDlgInstance(m_pTabDialog);
			m_pCustomScanDlg->m_bCustomscan = true;
			//if(m_pCustomScanDlg->Create(IDD_DIALOG_SCAN, m_pTabDialog) == FALSE)
			//{
			//	return FALSE;
			//}
			//Add custom scan in to lookup table
			m_pTabDialog->AddPage(m_pCustomScanDlg, m_btnCustomScan);
			//m_pTabDialog->AddPage(m_pQuickscanDlg, m_btnCustomScan);
		}

		//Antiroot kit
		if(m_btnAntirootkitScan != NULL)
		{
			delete m_btnAntirootkitScan;
			m_btnAntirootkitScan = NULL;
		}
		m_btnAntirootkitScan = new CxSkinButton();

		
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 78;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnAntirootkitScan->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_ANTIROOTKIT);
	//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnAntirootkitScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17,IDB_BITMAP_150x17_DISABLE,0, 0, 0, 0, 0);
		m_btnAntirootkitScan->SetTextAlignment(TA_LEFT);
		//m_btnAntirootkitScan->SetWindowText(L"         Antirootkit Scan");
		m_btnAntirootkitScan->SetTextColorA(RGB(255,255,255),0,1);
		m_btnAntirootkitScan->SetFocusTextColor(RGB(4,4,4));
		m_btnAntirootkitScan->SetFont(&theApp.m_fontWWTextNormal);
		//create Antiroot kit dialog
		/**************************************************************************************************
		Please add dialog classs pointer to add into loolup table For timebeing i add fullscan
		****************************************************************************************************/
		if(m_pAntirootkitScan != NULL)
		{
			delete m_pAntirootkitScan;
			m_pAntirootkitScan = NULL;
		}
		m_pAntirootkitScan = new CISpyAntiRootkit(m_pTabDialog);
		if(m_pAntirootkitScan->Create(IDD_DIALOG_ANTIROOTKIT, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Antiroot kit in to lookup table
		m_pTabDialog->AddPage(m_pAntirootkitScan, m_btnAntirootkitScan);

		////EMAIL SCAN rets
		//EMAIL Static

		if (m_stEmailScan != NULL)
		{
			delete m_stEmailScan;
			m_stEmailScan = NULL;
		}
		m_stEmailScan = new CColorStatic();

		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + 150;
		oRcBtnRect.top = BTN_LOCATION.y + 102;//196;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + 17;

		m_stEmailScan->Create(L"Static", WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_STATIC_EMAILSCAN);


		m_stEmailScan->SetBkColor(RGB(65, 65, 67));
		m_stEmailScan->SetFont(&theApp.m_fontWWTextNormal);
		m_stEmailScan->SetTextColor(RGB(255, 255, 255));

		//Virus scan 
		if (m_btnVirusScan != NULL)
		{
			delete m_btnVirusScan;
			m_btnVirusScan = NULL;
		}
		m_btnVirusScan = new CxSkinButton();


		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 119;//213;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;


		m_btnVirusScan->Create(_T(" "), WS_VISIBLE | WS_TABSTOP | TCS_BUTTONS | TCS_FIXEDWIDTH, oRcBtnRect, m_pTabDialog, ID_BUTTON_VIRUSSCAN);

		//m_btnVirusScan->SetTextAlignment(TA_NOUPDATECP);
		m_dwEmailscanEnable = ReadEmailScanEntryFromRegistry();
		if (m_dwEmailscanEnable == 1)
		{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
			m_btnVirusScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnVirusScan->SetTextAlignment(TA_LEFT);
			m_btnVirusScan->SetTextColorA(RGB(255, 255, 255), 0, 1);
			m_btnVirusScan->SetFocusTextColor(RGB(4, 4, 4));
		}
		if (m_dwEmailscanEnable == 0)
		{
			m_btnVirusScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnVirusScan->SetTextAlignment(TA_LEFT);
			m_btnVirusScan->SetTextColorA(RGB(150, 150, 150), RGB(150, 150, 150), RGB(150, 150, 150));
		}

		m_btnVirusScan->SetFont(&theApp.m_fontWWTextNormal);
		//create email virus scan dialog
		/**************************************************************************************************
		Please add dialog classs pointer to add into loolup table For timebeing i add fullscan
		****************************************************************************************************/
		//m_pVirusscan =  new CISpyEmailScanDlg(m_pTabDialog);

		if (m_pVirusscan == NULL)
		{
			m_pVirusscan = CISpyEmailScanDlg::GetEmailScanDlgInstance(m_pTabDialog);
			//m_pVirusscan->m_bVirusScan = false;
			if (m_pVirusscan->Create(IDD_DIALOG_EMAILSCAN, m_pTabDialog) == FALSE)
			{
				return FALSE;
			}
			//Add email virus scan in to lookup table
			m_pTabDialog->AddPage(m_pVirusscan, m_btnVirusScan);

		}
		//email spamfilter
		if (m_btnSpamFilter != NULL)
		{
			delete m_btnSpamFilter;
			m_btnSpamFilter = NULL;
		}
		m_btnSpamFilter = new CxSkinButton();


		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 136;//230;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnSpamFilter->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_SPAMFILTER);

		m_btnSpamFilter->SetTextAlignment(TA_LEFT);

		if (m_dwEmailscanEnable == 1)
		{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
			m_btnSpamFilter->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnSpamFilter->SetTextColorA(RGB(255, 255, 255), 0, 1);
			m_btnSpamFilter->SetFocusTextColor(RGB(4, 4, 4));
		}
		if (m_dwEmailscanEnable == 0)
		{
			m_btnSpamFilter->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnSpamFilter->SetTextColorA(RGB(150, 150, 150), RGB(150, 150, 150), RGB(150, 150, 150));
		}
		m_btnSpamFilter->SetFont(&theApp.m_fontWWTextNormal);
		//create email spamfilter dialog
		/**************************************************************************************************
		Please add dialog classs pointer to add into loolup table For timebeing i add fullscan
		****************************************************************************************************/
		if (m_pSpamFilter == NULL)
		{
			//m_pSpamFilter = new CISpyEmailScanDlg(m_pTabDialog);
			m_pSpamFilter = CISpyEmailScanDlg::GetEmailScanDlgInstance(m_pTabDialog);
			//m_pSpamFilter->m_bSpamFilter = false;
			//if(m_pSpamFilter->Create(IDD_DIALOG_EMAILSCAN, m_pTabDialog) == FALSE)
			//{
			//	return FALSE;
			//}
			//Add spam filter in to lookup table
			m_pTabDialog->AddPage(m_pSpamFilter, m_btnSpamFilter);
		}


		//email content filter
		if (m_btnContentFilter != NULL)
		{
			delete m_btnContentFilter;
			m_btnContentFilter = NULL;
		}
		m_btnContentFilter = new CxSkinButton();


		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 153;//247;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnContentFilter->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_CONTENTFILTER);

		m_btnContentFilter->SetTextAlignment(TA_LEFT);

		if (m_dwEmailscanEnable == 1)
		{	//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
			m_btnContentFilter->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnContentFilter->SetTextColorA(RGB(255, 255, 255), 0, 1);
			m_btnContentFilter->SetFocusTextColor(RGB(4, 4, 4));
		}
		if (m_dwEmailscanEnable == 0)
		{
			m_btnContentFilter->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnContentFilter->SetTextColorA(RGB(150, 150, 150), RGB(150, 150, 150), RGB(150, 150, 150));
		}
		m_btnContentFilter->SetFont(&theApp.m_fontWWTextNormal);

		//create email content filter dialog
		/**************************************************************************************************
		Please add dialog classs pointer to add into loolup table For timebeing i add fullscan
		****************************************************************************************************/
		if (m_pContentFilter == NULL)
		{
			//m_pContentFilter = new CISpyEmailScanDlg(m_pTabDialog);
			m_pContentFilter = CISpyEmailScanDlg::GetEmailScanDlgInstance(m_pTabDialog);
			//m_pContentFilter->m_bContentFilter = false;
			//if(m_pContentFilter->Create(IDD_DIALOG_EMAILSCAN, m_pTabDialog) == FALSE)
			//{
			//	return FALSE;
			//}
			//Add content filter in to lookup table
			m_pTabDialog->AddPage(m_pContentFilter, m_btnContentFilter);
		}

		//email signature
		//if (m_btnSignature != NULL)
		//{
		//	delete m_btnSignature;
		//	m_btnSignature = NULL;
		//}
		//m_btnSignature = new CxSkinButton();


		//oRcBtnRect.left = BTN_LOCATION.x;
		//oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		//oRcBtnRect.top = BTN_LOCATION.y + 170;//264;
		//oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		//m_btnSignature->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_SIGNATURE);

		//m_btnSignature->SetTextAlignment(TA_LEFT);

		//if (m_dwEmailscanEnable == 1)
		//{//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		//	m_btnSignature->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		//	m_btnSignature->SetTextColorA(RGB(255, 255, 255), 0, 1);
		//	m_btnSignature->SetFocusTextColor(RGB(4, 4, 4));
		//}
		//if (m_dwEmailscanEnable == 0)
		//{
		//	m_btnSignature->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		//	m_btnSignature->SetTextColorA(RGB(150, 150, 150), RGB(150, 150, 150), RGB(150, 150, 150));
		//}
		//m_btnSignature->SetFont(&theApp.m_fontWWTextNormal);
		//create email signature dialog
		/**************************************************************************************************
		Please add dialog classs pointer to add into loolup table For timebeing i add fullscan
		****************************************************************************************************/
		//if (m_pSignature == NULL)
	
		
		
		
		
		//{
		//	//m_pSignature = new CISpyEmailScanDlg(m_pTabDialog);
		//	m_pSignature = CISpyEmailScanDlg::GetEmailScanDlgInstance(m_pTabDialog);
		//	//m_pSignature->m_bSignatureFlag = false;
		//	//if(m_pSignature->Create(IDD_DIALOG_EMAILSCAN, m_pTabDialog) == FALSE)
		//	//{
		//	//	return FALSE;
		//	//}
		//	//Add email signature in to lookup table
		//	m_pTabDialog->AddPage(m_pSignature, m_btnSignature);
		//}


		//Tools static
		if(m_stTool != NULL)
		{
			delete m_stTool;
			m_stTool = NULL;
		}
		m_stTool = new CColorStatic();

		oRcBtnRect.left = BTN_LOCATION.x  ;
		oRcBtnRect.right = BTN_LOCATION.x + 150;
		oRcBtnRect.top = BTN_LOCATION.y + 177;//102 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + 17;

		m_stTool->Create(L"Static",WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED,oRcBtnRect,m_pTabDialog,ID_STATIC_TOOLS);


		m_stTool->SetBkColor(RGB(65,65,67));
		m_stTool->SetFont(&theApp.m_fontWWTextNormal);
		m_stTool->SetTextColor(RGB(255,255,255));

		////Registry Optimizer
		if(m_btnRegistryOptimizer != NULL)
		{
			delete m_btnRegistryOptimizer;
			m_btnRegistryOptimizer = NULL;
		}
		m_btnRegistryOptimizer = new CxSkinButton();

		
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 194;//119  ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnRegistryOptimizer->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_REGISTYOPTIMIZER);
		//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnRegistryOptimizer->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17,IDB_BITMAP_150x17_DISABLE,0, 0, 0, 0, 0);
		m_btnRegistryOptimizer->SetTextAlignment(TA_LEFT);
		//m_btnRegistryOptimizer->SetWindowText(L"         Registry Optimizer");
		m_btnRegistryOptimizer->SetTextColorA(RGB(255,255,255),0,1);
		m_btnRegistryOptimizer->SetFocusTextColor(RGB(4,4,4));
		m_btnRegistryOptimizer->SetFont(&theApp.m_fontWWTextNormal);
		//create register optimizer dialog

		if(m_pRegistryOptimizer != NULL)
		{
			delete m_pRegistryOptimizer;
			m_pRegistryOptimizer = NULL;
		}
		m_pRegistryOptimizer = new CRegistryOptimizerDlg(m_pTabDialog);
		if(m_pRegistryOptimizer->Create(IDD_DIALOG_REGISTRYOPTIMIZER, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add registry optimizer in to lookup table
		m_pTabDialog->AddPage(m_pRegistryOptimizer, m_btnRegistryOptimizer);
		
		////data Encryption
		if(m_btnDataEncryption != NULL)
		{
			delete m_btnDataEncryption;
			m_btnDataEncryption = NULL;
		}
		m_btnDataEncryption = new CxSkinButton();

		
		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 211;//119 +17;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnDataEncryption->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_DATAENCRYPTION);
		//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnDataEncryption->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17,IDB_BITMAP_150x17_DISABLE,0, 0, 0, 0, 0);
		m_btnDataEncryption->SetTextAlignment(TA_LEFT);
		//m_btnDataEncryption->SetWindowText(L"         Data Encryption");
		m_btnDataEncryption->SetTextColorA(RGB(255,255,255),0,1);
		m_btnDataEncryption->SetFocusTextColor(RGB(4,4,4));
		m_btnDataEncryption->SetFont(&theApp.m_fontWWTextNormal);
		//create data encryption dialog
		if(m_pDataEncryption != NULL)
		{
			delete m_pDataEncryption;
			m_pDataEncryption = NULL;
		}
		m_pDataEncryption = new CDataEncryptionDlg(m_pTabDialog);
		if(m_pDataEncryption->Create(IDD_DIALOG_DATAENCRYPTION, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add data encryption in to lookup table
		m_pTabDialog->AddPage(m_pDataEncryption, m_btnDataEncryption);


		//Recover
		if(m_btnRecover != NULL)
		{
			delete m_btnRecover;
			m_btnRecover = NULL;
		}
		m_btnRecover = new CxSkinButton();

		
		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 228;//136 + 17;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnRecover->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_RECOVER);
	//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnRecover->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17,IDB_BITMAP_150x17_DISABLE,0, 0, 0, 0, 0);
		m_btnRecover->SetTextAlignment(TA_LEFT);
		//m_btnRecover->SetWindowText(L"         Recover");
		m_btnRecover->SetTextColorA(RGB(255,255,255),0,1);
		m_btnRecover->SetFocusTextColor(RGB(4,4,4));
		m_btnRecover->SetFont(&theApp.m_fontWWTextNormal);
		
		//create recover dialog
		if(m_pRecover != NULL)
		{
			delete m_pRecover;
			m_pRecover = NULL;
		}

		m_pRecover = new CISpyRecoverDlg(m_pTabDialog);
		if(m_pRecover->Create(IDD_DIALOG_RECOVER, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add recover in to lookup table
		m_pTabDialog->AddPage(m_pRecover, m_btnRecover);

	
		//Utility
		if (m_btnUtility != NULL)
		{
			delete m_btnUtility;
			m_btnUtility = NULL;
		}
		m_btnUtility = new CxSkinButton();
		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 252;//362;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;
		m_btnUtility->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_UTILITY_OPTION);
		//issue:25 resolved by neha ,Two options get selected if user drag and select the option.	
		m_btnUtility->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnUtility->SetTextAlignment(TA_LEFT);
		m_btnUtility->SetTextColorA(RGB(255, 255, 255), 0, 1);
		m_btnUtility->SetFocusTextColor(RGB(4, 4, 4));
		m_btnUtility->SetFont(&theApp.m_fontWWTextNormal);

		//create Report dialog
		if (m_pUtility != NULL)
		{
			delete m_pUtility;
			m_pUtility = NULL;
		}
		m_pUtility = new CWardWizUtilitiesDlg(m_pTabDialog);
		if (m_pUtility->Create(IDD_DIALOG_UTILITYDLG, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Report in to lookup table
		m_pTabDialog->AddPage(m_pUtility, m_btnUtility);


		//Admin Static
		if(m_stAdminstration != NULL)
		{
			delete m_stAdminstration;
			m_stAdminstration = NULL;
		}
		m_stAdminstration = new CColorStatic();

		oRcBtnRect.left = BTN_LOCATION.x  ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 276;//288 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + 17;

		m_stAdminstration->Create(L"Static",WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED,oRcBtnRect,m_pTabDialog,ID_STATIC_ADMINSTRATION);


		m_stAdminstration->SetBkColor(RGB(65,65,67));
		m_stAdminstration->SetFont(&theApp.m_fontWWTextNormal);
		m_stAdminstration->SetTextColor(RGB(255,255,255));


		//Update
		if(m_btnUpdateNew != NULL)
		{
			delete m_btnUpdateNew;
			m_btnUpdateNew = NULL;
		}
		m_btnUpdateNew = new CxSkinButton();

		
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 293;//305;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnUpdateNew->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_UPDATE);
		//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnUpdateNew->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17,IDB_BITMAP_150x17_DISABLE,0, 0, 0, 0, 0);
		m_btnUpdateNew->SetTextAlignment(TA_LEFT);
		//m_btnUpdateNew->SetWindowText(L"         Updates");
		m_btnUpdateNew->SetTextColorA(RGB(255,255,255),0,1);
		m_btnUpdateNew->SetFocusTextColor(RGB(4,4,4));
		m_btnUpdateNew->SetFont(&theApp.m_fontWWTextNormal);
		//create update dialog
		/**************************************************************************************************
		Please add dialog classs pointer to add into loolup table For timebeing i add fullscan
		****************************************************************************************************/
		if(m_pUpdate != NULL)
		{
			delete m_pUpdate;
			m_pUpdate = NULL;
		}
		m_pUpdate = new CISpyUpdatesDlg(m_pTabDialog);
		if(m_pUpdate->Create(IDD_DIALOG_UPDATES, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add update in to lookup table
		m_pTabDialog->AddPage(m_pUpdate, m_btnUpdateNew);

		//report
		if(m_btnReports != NULL)
		{
			delete m_btnReports;
			m_btnReports = NULL;
		}
		m_btnReports = new CxSkinButton();

		
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 310;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnReports->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_REPORTS);
		//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnReports->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17,IDB_BITMAP_150x17_DISABLE,0, 0, 0, 0, 0);
		m_btnReports->SetTextAlignment(TA_LEFT);
		//m_btnReports->SetWindowText(L"         Reports");
		m_btnReports->SetTextColorA(RGB(255,255,255),0,1);
		m_btnReports->SetFocusTextColor(RGB(4,4,4));
		m_btnReports->SetFont(&theApp.m_fontWWTextNormal);
		//create Report dialog
		/**************************************************************************************************
		Please add dialog classs pointer to add into loolup table For timebeing i add fullscan
		****************************************************************************************************/
		if(m_pReports != NULL)
		{
			delete m_pReports;
			m_pReports = NULL;
		}
		m_pReports = new CISpyReportsDlg(m_pTabDialog);
		if(m_pReports->Create(IDD_DIALOG_REPORTS, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Report in to lookup table
		m_pTabDialog->AddPage(m_pReports, m_btnReports);


		if(m_pQuickscanDlg != NULL)
		{
			m_pQuickscanDlg->m_bQuickScan = false;
		}
		if(m_pFullScanDlg != NULL)
		{
			m_pFullScanDlg->m_bFullScan = false;
		}
		if(m_pCustomScanDlg != NULL)
		{
			m_pCustomScanDlg->m_bCustomscan = false;
		}
		RefreshStrings();
		//issue Neha Gharge 7/6/2014
		MainDialogDisplay();
		Invalidate();
		return TRUE;
}

/***************************************************************************
  Function Name  : OnUserRequestToDisplayThreatNews
  Description    : This function get call when some other module send messge
				   WM_COMMAND = CHECKSCROLLINGTEXT
  Author Name    : Rajil
  SR_NO			 :
  Date           : 28th may 2014
  Modification   : 28th may 2014
****************************************************************************/

LRESULT  CISpyGUIDlg::OnUserRequestToDisplayThreatNews(WPARAM wParam , LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);
	DWORD dwEnable = m_pHomepageDlg->ReadMarqueeEntryFromRegistry();
	if(m_pHomepageDlg != NULL)
	{
		m_pHomepageDlg->m_stTextVar.ShowWindow(dwEnable);
	}
	return 0;

}

/***************************************************************************
  Function Name  : SetSkinOnhomePage
  Description    : Setskin on button on selection basis.
  Author Name    : Neha gharge
  SR_NO			 :
  Date           : 28th may 2014
  Modification   : 28th may 2014
****************************************************************************/

void CISpyGUIDlg::SetSkinOnhomePage()
{
	if(m_btnQuickScan != NULL)
	{
		m_btnQuickScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnQuickScan->SetTextColorA(RGB(255,255,255),0,1);
		m_btnQuickScan->Invalidate();
	}
	if(m_btnFullScan != NULL)
	{
		m_btnFullScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnFullScan->SetTextColorA(RGB(255,255,255),RGB(255,255,255),RGB(4,4,4));
		m_btnFullScan->Invalidate();
	}
	if(m_btnCustomScan != NULL)
	{
		m_btnCustomScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnCustomScan->SetTextColorA(RGB(255,255,255),RGB(255,255,255),RGB(4,4,4));
		m_btnCustomScan->Invalidate();
	}
	if(m_btnAntirootkitScan != NULL)
	{
		m_btnAntirootkitScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnAntirootkitScan->SetTextColorA(RGB(255,255,255),RGB(255,255,255),RGB(4,4,4));
		m_btnAntirootkitScan->Invalidate();
	}
	if(m_btnRegistryOptimizer != NULL)
	{
		m_btnRegistryOptimizer->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnRegistryOptimizer->SetTextColorA(RGB(255,255,255),RGB(255,255,255),RGB(4,4,4));
		m_btnRegistryOptimizer->Invalidate();
	}
	if(m_btnDataEncryption != NULL)
	{
		m_btnDataEncryption->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnDataEncryption->SetTextColorA(RGB(255,255,255),RGB(255,255,255),RGB(4,4,4));
		m_btnDataEncryption->Invalidate();
	}
	if(m_btnRecover != NULL)
	{
		m_btnRecover->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnRecover->SetTextColorA(RGB(255,255,255),RGB(255,255,255),RGB(4,4,4));
		m_btnRecover->Invalidate();
	}
	if(m_btnFolderLocker != NULL)
	{
		m_btnFolderLocker->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnFolderLocker->SetTextColorA(RGB(255,255,255),RGB(255,255,255),RGB(4,4,4));
		m_btnFolderLocker->Invalidate();
	}
	if(m_btnUpdateNew != NULL)
	{
		m_btnUpdateNew->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnUpdateNew->SetTextColorA(RGB(255,255,255),RGB(255,255,255),RGB(4,4,4));
		m_btnUpdateNew->Invalidate();
	}
	if(m_btnReports != NULL)
	{
		m_btnReports->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnReports->SetTextColorA(RGB(255,255,255),RGB(255,255,255),RGB(4,4,4));	
		m_btnReports->Invalidate();
	}
	if (m_btnUtility != NULL)
	{
		m_btnUtility->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnUtility->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), RGB(4, 4, 4));
		m_btnUtility->Invalidate();
	}
	Invalidate();
}

/***************************************************************************
  Function Name  : OnBnClickedButtonLinkedin
  Description    : Execute linkedin page
  Author Name    : Neha gharge
  SR_NO			 :
  Date           : 28th may 2014
  Modification   : 28th may 2014
  Modification	 : 28th may 2015 no need to remove the focus from last functionlity
  when we clicked on in button other than tab button and home button.
****************************************************************************/
void CISpyGUIDlg::OnBnClickedButtonLinkedin()
{
	//SetSkinOnhomePage();
	//ShellExecute(NULL,L"open",L"https://in.linkedin.com/in/wardwizav/",NULL,NULL,SW_SHOW);

	TCHAR	szPath[512] = {0};

	if( GetDefaultHTTPBrowser(szPath) )
	{
		GetEnvironmentVariable( L"ProgramFiles", szPath, 511 );
		wcscat_s( szPath, 511, L"\\Internet Explorer\\iexplore.exe" );
	}

	ShellExecute(NULL,L"open", szPath, L"https://www.linkedin.com/in/wardwiz", NULL,SW_SHOW);
}

/***************************************************************************
  Function Name  : OnLiveUpdateTrayCall
  Description    : This function is called when UI is already running and user click on 
				   tray notification Dlg of Update now
  Author Name    : Nitin k
  SR_NO			 :
  Date           : 24th July 2014
  modified by	 : lalit kumawat 8-4-2015
  // issue - if we start update from tab dialog and after 80% click on update now from tray then it will download start from 0% by killing previous downloading.
  modified by	 : Nitin Kolapkar 11th December 2015
  Changes		 : Previously SendMessage was used from tray now using Pipe communication
  ****************************************************************************/
void CISpyGUIDlg::OnLiveUpdateTrayCall(/*WPARAM wParam , LPARAM lParam*/)
{
	//UNREFERENCED_PARAMETER(wParam);
	//UNREFERENCED_PARAMETER(lParam);
	//ResetEvent(theApp.m_hUpdateOprEvent);
	try
	{
		if (m_pThis->IsAnyPopUpDisplayed())
		{
			HWND hWnd = m_pThis->GetSafeHwnd();
			::ShowWindow(hWnd, SW_RESTORE);
			::BringWindowToTop(hWnd);
			::SetForegroundWindow(hWnd);
			return;
		}
		if (m_pUpdate != NULL)
		{
			if (m_pUpdate->m_dlgISpyUpdatesSecond.m_isDownloading == true)
			{
				m_pUpdate->ShowWindow(SW_SHOW);
				m_pUpdate->ShowHideAllUpdateFirstPageControls(false);
				m_pUpdate->m_dlgISpyUpdatesSecond.ShowWindow(SW_SHOW);
			}
			else if (IsAnyTaskInProcess())
			{
				if (StopRunningProcess())
				{
					WaitForSingleObject(theApp.m_hUpdateOprEvent, INFINITE);
					HideAllPages();
					DeselctAllButtons();
					Invalidate();
					if (m_pTabDialog != NULL)
					{
						m_pTabDialog->SetSelectedButton(WARDWIZ_UPDATES_DLG);
					}
					if (m_pUpdate != NULL)
					{
						if (m_btnUpdateNew != NULL)
						{
							m_btnUpdateNew->SetSkin(theApp.m_hResDLL, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
							m_btnUpdateNew->SetTextColorA(RGB(4, 4, 4), 0, 1);
							m_btnUpdateNew->SetFocusTextColor(RGB(4, 4, 4));
							m_btnUpdateNew->Invalidate();
						}
						m_pUpdate->ShowWindow(SW_SHOW);
						m_pUpdate->m_updateType = UPDATEFROMINTERNT;
						m_pUpdate->OnBnClickedButtonNext();
					}
				}
			}
			else //Issue No: 1159 When no operation is in progress if we click on 'update now' button in WardWiz live update tray popup no action taking place.
			{
				HideAllPages();
				DeselctAllButtons();
				Invalidate();
				if (m_pTabDialog != NULL)
				{
					m_pTabDialog->SetSelectedButton(WARDWIZ_UPDATES_DLG);
				}
				if (m_pUpdate != NULL)
				{
					if (m_btnUpdateNew != NULL)
					{
						m_btnUpdateNew->SetSkin(theApp.m_hResDLL, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
						m_btnUpdateNew->SetTextColorA(RGB(4, 4, 4), 0, 1);
						m_btnUpdateNew->SetFocusTextColor(RGB(4, 4, 4));
						m_btnUpdateNew->Invalidate();
					}
					m_pUpdate->ShowWindow(SW_SHOW);
					m_pUpdate->m_updateType = UPDATEFROMINTERNT;
					m_pUpdate->OnBnClickedButtonNext();
				}
			}
		}
		
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in OnLiveUpdateTrayCall", 0, 0, true, SECONDLEVEL);
	}

	//return 0;
}


/***************************************************************************
  Function Name  : HideAllPages
  Description    : This function is called when UI is already running and user click on 
				   tray notification Dlg of Update now.All dialog get hide and Update dialog 
				   will be shown.
  Author Name    : Nitin k
  SR_NO			 :
  Date           : 24th July 2014
****************************************************************************/
void CISpyGUIDlg :: HideAllPages()
{
	if(m_pHomepageDlg != NULL)
	{
		m_pHomepageDlg->ShowWindow(SW_HIDE);
	}
	if(m_pQuickscanDlg != NULL)
	{
		m_pQuickscanDlg->ShowWindow(SW_HIDE);
	}
	if(m_pCustomScanDlg != NULL)
	{
		m_pCustomScanDlg->ShowWindow(SW_HIDE);
	}
	if(m_pFullScanDlg != NULL)
	{
		m_pFullScanDlg->ShowWindow(SW_HIDE);
	}
	if(m_pAntirootkitScan != NULL)
	{
		m_pAntirootkitScan->ShowWindow(SW_HIDE);
	}
	if(m_pRegistryOptimizer != NULL)
	{
		m_pRegistryOptimizer->ShowWindow(SW_HIDE);
	}
	if(m_pDataEncryption != NULL)
	{
		m_pDataEncryption->ShowWindow(SW_HIDE);
	}
	if(m_pRecover != NULL)
	{
		m_pRecover->ShowWindow(SW_HIDE);
	}
	if(m_pFolderLocker != NULL)
	{
		m_pFolderLocker->ShowWindow(SW_HIDE);
	}
	if(m_pVirusscan != NULL)
	{
		m_pVirusscan->ShowWindow(SW_HIDE);
	}
	if(m_pSpamFilter!= NULL)
	{
		m_pSpamFilter->ShowWindow(SW_HIDE);
	}
	if(m_pContentFilter != NULL)
	{
		m_pContentFilter->ShowWindow(SW_HIDE);
	}
	if(m_pSignature != NULL)
	{
		m_pSignature->ShowWindow(SW_HIDE);
	}
	if(m_pReports != NULL)
	{
		m_pReports->ShowWindow(SW_HIDE);
	}
	if (m_pUtility!= NULL)
	{
		m_pUtility->ShowWindow(SW_HIDE);
	}
	/*if (m_pUpdate != NULL)
	{
		m_pUpdate->ShowWindow(SW_HIDE);
	}*/
}

void CISpyGUIDlg::OnSize(UINT nType, int cx, int cy)
{
	if(IsZoomed())
	{
		ShowWindow(SW_SHOWNORMAL);
	}
	CJpegDialog::OnSize(nType, cx, cy);
}


/***************************************************************************
  Function Name  : ShowControlsForBasicEdition
  Description    : Show dialogs according to essential editions
  Author Name    : Nitin K
  SR_NO			 :
  Date           : 27th Aug 2014
  Modification   : 27th Aug 2014
  Modification	 : 28th May 2015, Neha Gharge added Utility button and dialog.
  Modification   : 27th June 2015, Neha Gharge shifting of menus accroding 1.12.0.0 designs
****************************************************************************/

BOOL CISpyGUIDlg::ShowControlsForBasicEdition()
{
		RECT oRcBtnRect;

		if(m_btnInitialHome != NULL)
		{
			delete m_btnInitialHome;
			m_btnInitialHome = NULL;
		}
		m_btnInitialHome = new CxSkinButton();
		m_btnInitialHome->Create(_T(" "), WS_CHILD | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_HOME_PAGE);

		//create quick scan dialog
		if(m_pHomepageDlg != NULL)
		{
			delete m_pHomepageDlg;
			m_pHomepageDlg = NULL;
		}
		m_pHomepageDlg = new CWardWizHomePage(m_pTabDialog);
		if(m_pHomepageDlg->Create(IDD_DIALOG_HOME_PAGE, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}

		//Add quick scan in to lookup table
		m_pTabDialog->AddPage(m_pHomepageDlg, m_btnInitialHome);
			
		if(m_stScan != NULL)
		{
			delete m_stScan;
			m_stScan = NULL;
		}
		m_stScan = new CColorStatic();
		oRcBtnRect.left = BTN_LOCATION.x  ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 10 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + 17;

		m_stScan->Create(L"Static",WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED,oRcBtnRect,m_pTabDialog,ID_STATIC_SCAN);
		m_stScan->SetBkColor(RGB(65,65,67));
		m_stScan->SetFont(&theApp.m_fontWWTextNormal);
		m_stScan->SetTextColor(RGB(255,255,255));

		////create Quick Scan button
		if(m_btnQuickScan != NULL)
		{
			delete m_btnQuickScan;
			m_btnQuickScan = NULL;
		}
		m_btnQuickScan = new CxSkinButton();

		CPoint objBtnPoint = BTN_LOCATION;
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 27 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnQuickScan->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_QUICK_SCAN);
		m_btnQuickScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17,IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnQuickScan->SetTextAlignment(TA_LEFT);
		m_btnQuickScan->SetTextColorA(RGB(255,255,255),0,1);
		m_btnQuickScan->SetFont(&theApp.m_fontWWTextNormal);

		//create quick scan dialog
		if(m_pQuickscanDlg == NULL)
		{
			m_pQuickscanDlg = CScanDlg::GetScanDlgInstance(m_pTabDialog);
			m_pQuickscanDlg->m_bQuickScan = true;
			if(m_pQuickscanDlg->Create(IDD_DIALOG_SCAN, m_pTabDialog) == FALSE)
			{
				return FALSE;
			}
			//Add quick scan in to lookup table
			m_pTabDialog->AddPage(m_pQuickscanDlg, m_btnQuickScan);
		}

		//full scan
		if(m_btnFullScan != NULL)
		{
			delete m_btnFullScan;
			m_btnFullScan = NULL;
		}
		m_btnFullScan = new CxSkinButton();
		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 27 + 17;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnFullScan->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_FULL_SCAN);

		m_btnFullScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17,IDB_BITMAP_150x17_DISABLE,0, 0, 0, 0, 0);
		m_btnFullScan->SetTextAlignment(TA_LEFT);
		m_btnFullScan->SetTextColorA(RGB(255,255,255),0,1);
		m_btnFullScan->SetFocusTextColor(RGB(4,4,4));
		m_btnFullScan->SetFont(&theApp.m_fontWWTextNormal);
		
		//create full scan dialog 
		if(m_pFullScanDlg == NULL)
		{
			m_pFullScanDlg = CScanDlg::GetScanDlgInstance(m_pTabDialog);
			m_pFullScanDlg->m_bFullScan = true;
			m_pTabDialog->AddPage(m_pFullScanDlg, m_btnFullScan);
		}		
 
		//Custom scan
		if(m_btnCustomScan != NULL)
		{
			delete m_btnCustomScan;
			m_btnCustomScan = NULL;
		}
		m_btnCustomScan = new CxSkinButton();
		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 44 +17;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnCustomScan->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_CUSTOM_SCAN);

		m_btnCustomScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17,IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnCustomScan->SetTextAlignment(TA_LEFT);
		m_btnCustomScan->SetTextColorA(RGB(255,255,255),0,1);
		m_btnCustomScan->SetFocusTextColor(RGB(4,4,4));
		m_btnCustomScan->SetFont(&theApp.m_fontWWTextNormal);
		//create custom scan dialog
		/**************************************************************************************************
		Please add dialog classs pointer to add into loolup table For timebeing i add fullscan
		****************************************************************************************************/

		if(m_pCustomScanDlg == NULL)
		{
			m_pCustomScanDlg = CScanDlg::GetScanDlgInstance(m_pTabDialog);
			m_pCustomScanDlg->m_bCustomscan = true;
			//Add custom scan in to lookup table
			m_pTabDialog->AddPage(m_pCustomScanDlg, m_btnCustomScan);
		}

		////Antiroot kit
		//if(m_btnAntirootkitScan != NULL)
		//{
		//	delete m_btnAntirootkitScan;
		//	m_btnAntirootkitScan = NULL;
		//}
		//m_btnAntirootkitScan = new CxSkinButton();
		//oRcBtnRect.left = BTN_LOCATION.x ;
		//oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		//oRcBtnRect.top = BTN_LOCATION.y + 78;
		//oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		//m_btnAntirootkitScan->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_ANTIROOTKIT);
		//m_btnAntirootkitScan->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_H_150x17, 0, 0, 0, 0);
		//m_btnAntirootkitScan->SetTextAlignment(TA_LEFT);
		////m_btnAntirootkitScan->SetWindowText(L"         Antirootkit Scan");
		//m_btnAntirootkitScan->SetTextColorA(RGB(255,255,255),0,1);
		//m_btnAntirootkitScan->SetFocusTextColor(RGB(4,4,4));
		//m_btnAntirootkitScan->SetFont(&theApp.m_fontWWTextNormal);

		////create Antiroot kit dialog
		//if(m_pAntirootkitScan != NULL)
		//{
		//	delete m_pAntirootkitScan;
		//	m_pAntirootkitScan = NULL;
		//}
		//m_pAntirootkitScan = new CISpyAntiRootkit(m_pTabDialog);
		//if(m_pAntirootkitScan->Create(IDD_DIALOG_ANTIROOTKIT, m_pTabDialog) == FALSE)
		//{
		//	return FALSE;
		//}
		////Add Antiroot kit in to lookup table
		//m_pTabDialog->AddPage(m_pAntirootkitScan, m_btnAntirootkitScan);


		//Tools static
		if(m_stTool != NULL)
		{
			delete m_stTool;
			m_stTool = NULL;
		}
		m_stTool = new CColorStatic();
		oRcBtnRect.left = BTN_LOCATION.x  ;
		oRcBtnRect.right = BTN_LOCATION.x + 150;
		oRcBtnRect.top = BTN_LOCATION.y + 85 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + 17;

		m_stTool->Create(L"Static",WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED,oRcBtnRect,m_pTabDialog,ID_STATIC_TOOLS);
		m_stTool->SetBkColor(RGB(65,65,67));
		m_stTool->SetFont(&theApp.m_fontWWTextNormal);
		m_stTool->SetTextColor(RGB(255,255,255));

		////Registry Optimizer
		//if(m_btnRegistryOptimizer != NULL)
		//{
		//	delete m_btnRegistryOptimizer;
		//	m_btnRegistryOptimizer = NULL;
		//}
		//m_btnRegistryOptimizer = new CxSkinButton();

		//oRcBtnRect.left = BTN_LOCATION.x ;
		//oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		//oRcBtnRect.top = BTN_LOCATION.y + 119  ;
		//oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		//m_btnRegistryOptimizer->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_REGISTYOPTIMIZER);

		//m_btnRegistryOptimizer->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17,IDB_BITMAP_150x17_DISABLE,IDB_BITMAP_H_150x17, 0, 0, 0, 0);
		//m_btnRegistryOptimizer->SetTextAlignment(TA_LEFT);
		////m_btnRegistryOptimizer->SetWindowText(L"         Registry Optimizer");
		//m_btnRegistryOptimizer->SetTextColorA(RGB(255,255,255),0,1);
		//m_btnRegistryOptimizer->SetFocusTextColor(RGB(4,4,4));
		//m_btnRegistryOptimizer->SetFont(&theApp.m_fontWWTextNormal);

		////create register optimizer dialog
		//if(m_pRegistryOptimizer != NULL)
		//{
		//	delete m_pRegistryOptimizer;
		//	m_pRegistryOptimizer = NULL;
		//}
		//m_pRegistryOptimizer = new CRegistryOptimizerDlg(m_pTabDialog);
		//if(m_pRegistryOptimizer->Create(IDD_DIALOG_REGISTRYOPTIMIZER, m_pTabDialog) == FALSE)
		//{
		//	return FALSE;
		//}
		////Add registry optimizer in to lookup table
		//m_pTabDialog->AddPage(m_pRegistryOptimizer, m_btnRegistryOptimizer);

		//////data Encryption
		//if(m_btnDataEncryption != NULL)
		//{
		//	delete m_btnDataEncryption;
		//	m_btnDataEncryption = NULL;
		//}
		//m_btnDataEncryption = new CxSkinButton();
		//oRcBtnRect.left = BTN_LOCATION.x;
		//oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		//oRcBtnRect.top = BTN_LOCATION.y + 119 + 17;
		//oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		//m_btnDataEncryption->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_DATAENCRYPTION);
		//m_btnDataEncryption->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, IDB_BITMAP_H_150x17, 0, 0, 0, 0);
		//m_btnDataEncryption->SetTextAlignment(TA_LEFT);
		////m_btnDataEncryption->SetWindowText(L"         Data Encryption");
		//m_btnDataEncryption->SetTextColorA(RGB(255,255,255),0,1);
		//m_btnDataEncryption->SetFocusTextColor(RGB(4,4,4));
		//m_btnDataEncryption->SetFont(&theApp.m_fontWWTextNormal);
		//
		////create data encryption dialog
		//if(m_pDataEncryption != NULL)
		//{
		//	delete m_pDataEncryption;
		//	m_pDataEncryption = NULL;
		//}
		//m_pDataEncryption = new CDataEncryptionDlg(m_pTabDialog);
		//if(m_pDataEncryption->Create(IDD_DIALOG_DATAENCRYPTION, m_pTabDialog) == FALSE)
		//{
		//	return FALSE;
		//}
		////Add data encryption in to lookup table
		//m_pTabDialog->AddPage(m_pDataEncryption, m_btnDataEncryption);


		//Recover
		if(m_btnRecover != NULL)
		{
			delete m_btnRecover;
			m_btnRecover = NULL;
		}
		m_btnRecover = new CxSkinButton();
		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 102;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;

		m_btnRecover->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_RECOVER);
		//issue:25 resolved by neha ,Two options get selected if user drag and select the option.
		m_btnRecover->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnRecover->SetTextAlignment(TA_LEFT);
		//m_btnRecover->SetWindowText(L"         Recover");
		m_btnRecover->SetTextColorA(RGB(255,255,255),0,1);
		m_btnRecover->SetFocusTextColor(RGB(4,4,4));
		m_btnRecover->SetFont(&theApp.m_fontWWTextNormal);
		
		//create recover dialog
		if(m_pRecover != NULL)
		{
			delete m_pRecover;
			m_pRecover = NULL;
		}
		m_pRecover = new CISpyRecoverDlg(m_pTabDialog);
		if(m_pRecover->Create(IDD_DIALOG_RECOVER, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add recover in to lookup table
		m_pTabDialog->AddPage(m_pRecover, m_btnRecover);

		//Utility
		if (m_btnUtility != NULL)
		{
			delete m_btnUtility;
			m_btnUtility = NULL;
		}
		m_btnUtility = new CxSkinButton();
		oRcBtnRect.left = BTN_LOCATION.x;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 126;//200;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;
		m_btnUtility->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_UTILITY_OPTION);
		//issue:25 resolved by neha ,Two options get selected if user drag and select the option.	
		m_btnUtility->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnUtility->SetTextAlignment(TA_LEFT);
		m_btnUtility->SetTextColorA(RGB(255, 255, 255), 0, 1);
		m_btnUtility->SetFocusTextColor(RGB(4, 4, 4));
		m_btnUtility->SetFont(&theApp.m_fontWWTextNormal);

		//create Report dialog
		if (m_pUtility != NULL)
		{
			delete m_pUtility;
			m_pUtility = NULL;
		}
		m_pUtility = new CWardWizUtilitiesDlg(m_pTabDialog);
		if (m_pUtility->Create(IDD_DIALOG_UTILITYDLG, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Report in to lookup table
		m_pTabDialog->AddPage(m_pUtility, m_btnUtility);

		//Administartion
		if(m_stAdminstration != NULL)
		{
			delete m_stAdminstration;
			m_stAdminstration = NULL;
		}
		m_stAdminstration = new CColorStatic();
		oRcBtnRect.left = BTN_LOCATION.x  ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 150 ;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + 17;
		m_stAdminstration->Create(L"Static",WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED,oRcBtnRect,m_pTabDialog,ID_STATIC_ADMINSTRATION);
		m_stAdminstration->SetBkColor(RGB(65,65,67));
		m_stAdminstration->SetFont(&theApp.m_fontWWTextNormal);
		m_stAdminstration->SetTextColor(RGB(255,255,255));


		//Update
		if(m_btnUpdateNew != NULL)
		{
			delete m_btnUpdateNew;
			m_btnUpdateNew = NULL;
		}
		m_btnUpdateNew = new CxSkinButton();
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 167;//126 + 17;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;
		m_btnUpdateNew->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_UPDATE);
		//issue:25 resolved by neha ,Two options get selected if user drag and select the option.	
		m_btnUpdateNew->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnUpdateNew->SetTextAlignment(TA_LEFT);
		m_btnUpdateNew->SetTextColorA(RGB(255,255,255),0,1);
		m_btnUpdateNew->SetFocusTextColor(RGB(4,4,4));
		m_btnUpdateNew->SetFont(&theApp.m_fontWWTextNormal);

		//create update dialog
		if(m_pUpdate != NULL)
		{
			delete m_pUpdate;
			m_pUpdate = NULL;
		}
		m_pUpdate = new CISpyUpdatesDlg(m_pTabDialog);
		if(m_pUpdate->Create(IDD_DIALOG_UPDATES, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add update in to lookup table
		m_pTabDialog->AddPage(m_pUpdate, m_btnUpdateNew);


		//report
		if(m_btnReports != NULL)
		{
			delete m_btnReports;
			m_btnReports = NULL;
		}
		m_btnReports = new CxSkinButton();
		oRcBtnRect.left = BTN_LOCATION.x ;
		oRcBtnRect.right = BTN_LOCATION.x + BUTTON_WIDTH_BIG;
		oRcBtnRect.top = BTN_LOCATION.y + 184;//143 +17;
		oRcBtnRect.bottom = /*BTN_LOCATION.y + */oRcBtnRect.top + BUTTON_HEIGHT;
		m_btnReports->Create(_T(" "), WS_CHILD | WS_VISIBLE | WS_TABSTOP | TCS_OWNERDRAWFIXED, oRcBtnRect, m_pTabDialog, ID_BUTTON_REPORTS);
	//issue:25 resolved by neha ,Two options get selected if user drag and select the option.	
		m_btnReports->SetSkin(theApp.m_hResDLL,IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
		m_btnReports->SetTextAlignment(TA_LEFT);
		//m_btnReports->SetWindowText(L"         Reports");
		m_btnReports->SetTextColorA(RGB(255,255,255),0,1);
		m_btnReports->SetFocusTextColor(RGB(4,4,4));
		m_btnReports->SetFont(&theApp.m_fontWWTextNormal);

		//create Report dialog
		if(m_pReports != NULL)
		{
			delete m_pReports;
			m_pReports = NULL;
		}
		m_pReports = new CISpyReportsDlg(m_pTabDialog);
		if(m_pReports->Create(IDD_DIALOG_REPORTS, m_pTabDialog) == FALSE)
		{
			return FALSE;
		}
		//Add Report in to lookup table
		m_pTabDialog->AddPage(m_pReports, m_btnReports);

		if(m_pQuickscanDlg != NULL)
		{
			m_pQuickscanDlg->m_bQuickScan = false;
		}
		if(m_pFullScanDlg != NULL)
		{
			m_pFullScanDlg->m_bFullScan = false;
		}
		if(m_pCustomScanDlg != NULL)
		{
			m_pCustomScanDlg->m_bCustomscan = false;
		}
		RefreshStrings();
		return TRUE;
}
// ISSUE N0: 174,solved by lalit , "When UI is open then ON Click on Alt+Space it should give option of Minimize"
/***************************************************************************
  Function Name  : OnNcCreate
  Description    : The framework calls this member function prior to the WM_CREATE message when the CWnd object is first created.
  Author Name    : lalit kumawat
  SR_NO			 :
  Date           : 1/10/2014
  Modification   : 
****************************************************************************/
BOOL CISpyGUIDlg::OnNcCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (!CJpegDialog::OnNcCreate(lpCreateStruct))
		return FALSE;
  // Modify the window style
  LONG dwStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
  ::SetWindowLong(m_hWnd, GWL_STYLE, dwStyle |WS_MINIMIZEBOX);

	// TODO:  Add your specialized creation code here

	return TRUE;
}


//Function which gets default browser present on client machine
bool CISpyGUIDlg::GetDefaultHTTPBrowser( LPTSTR lpszBrowserPath )
{

	CITinRegWrapper	objReg;

	TCHAR	szPath[512] = {0};
	DWORD	dwSize = 511;

	objReg.GetRegistryValueData(HKEY_CURRENT_USER, L"Software\\Classes\\http\\shell\\open\\command", L"", szPath, dwSize);
	if( !szPath[0] )
		return true;

	_wcsupr( szPath );

	TCHAR	*pTemp = StrStr(szPath, L".EXE" );

	if( !pTemp )
		return true;

	pTemp += wcslen( L".EXE" ) + 0x01;

	*pTemp = '\0';

	if( szPath[wcslen(szPath)-1] == '"' )
		szPath[wcslen(szPath)-1] = '\0';

	wcscpy_s(lpszBrowserPath, 511, &szPath[1] );

	if( !PathFileExists(lpszBrowserPath) )
		return true;

	return false;
}
/***************************************************************************
Function Name  : OnLiveUpdateFixDNowCall
Description    : it will launch live update dlg
Author Name    : lalit kumawat
SR_NO		   :
Date           : 
Modification   :
****************************************************************************/
void CISpyGUIDlg::OnLiveUpdateFixDNowCall()
{
	try
	{
		HideAllPages();
		if (m_pUpdate != NULL)
		{
			if (m_pUpdate->m_dlgISpyUpdatesSecond.m_isDownloading == true)
			{
				m_pUpdate->ShowWindow(SW_SHOW);
				m_pUpdate->ShowHideAllUpdateFirstPageControls(false);
				m_pUpdate->m_dlgISpyUpdatesSecond.ShowWindow(SW_SHOW);
			}
			else
			{
				m_pUpdate->ShowWindow(SW_SHOW);
				m_pUpdate->m_updateType = UPDATEFROMINTERNT;
				m_pUpdate->OnBnClickedButtonNext();
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in OnLiveUpdateFixDNowCall", 0, 0, true, SECONDLEVEL);
	}
}

void CAboutDlg::OnBnClickedButtonAbtOk()
{
	OnOK();
}
/***************************************************************************
Function Name  : IsAnyTaskInProcess
Description    : this function is use to check whether any process running  or not
Author Name    : lalit kumawat
SR_NO		   :
Date           :
Modification   :
****************************************************************************/
bool CISpyGUIDlg::IsAnyTaskInProcess()
{
	bool bIsprocessrunning = false;
	m_iRunningProcessNmB = 0;
	try
	{
			if (m_pQuickscanDlg != NULL && m_pQuickscanDlg->m_bQuickScan)
				m_iRunningProcessNmB = QUICK_SCAN_DLG;
			
			else if (m_pQuickscanDlg != NULL && m_pQuickscanDlg->m_bFullScan)
				m_iRunningProcessNmB = FULL_SCAN_DLG;

			else if (m_pQuickscanDlg != NULL && m_pQuickscanDlg->m_bCustomscan)
				m_iRunningProcessNmB = CUSTOM_SCAN_DLG; 

			else if (m_pAntirootkitScan != NULL && m_pAntirootkitScan->m_bAntirootScanningInProgress)
				m_iRunningProcessNmB = ANTIROOTKIT_SCAN_DLG;

			else if (m_pRegistryOptimizer != NULL && m_pRegistryOptimizer->m_bScanStarted)
				m_iRunningProcessNmB = REGISTRY_OPTIMIZER_DLG;

			else if (m_pDataEncryption != NULL && m_pDataEncryption->m_bIsEnDepInProgress)
				m_iRunningProcessNmB = DATA_ENCRYPTION_DLG;

			else if (m_pRecover != NULL && m_pRecover->m_bRecoverThreadStart)
				m_iRunningProcessNmB = RECOVER_DLG;

			else if (m_pUpdate != NULL && m_pUpdate->m_dlgISpyUpdatesSecond.m_isDownloading)
				m_iRunningProcessNmB = WARDWIZ_UPDATES_DLG;

			else if (m_pReports != NULL && m_pReports->m_bReportThreadStart != NULL)
				m_iRunningProcessNmB = WARDWIZ_REPORTS_DLG;

			else if (m_pRecover != NULL && m_pRecover->m_bDeleteThreadStart)
				m_iRunningProcessNmB = RECOVER_DLG;

			else if (m_pQuickscanDlg->m_bIsCleaning || m_pQuickscanDlg->m_bIsCleaning || m_pQuickscanDlg->m_bIsCleaning)
				m_iRunningProcessNmB = QUICK_SCAN_DLG;
			else
				m_iRunningProcessNmB = 0;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in IsAnyTaskInProcess", 0, 0, true, SECONDLEVEL);
	}

	if (m_iRunningProcessNmB)
		bIsprocessrunning = true;

	return bIsprocessrunning;
}

/***************************************************************************
Function Name  : StopRunningProcess
Description    : it use to stop any running operation like(scan, encryption/decryption)
Author Name    : lalit kumawat
SR_NO		   :
Date           :
Modification   :
****************************************************************************/
bool CISpyGUIDlg::StopRunningProcess()
{
	bool bIsProcessAborted = false;
	try
	{
		switch (m_iRunningProcessNmB)
		{

		case QUICK_SCAN_DLG:

			if (m_pQuickscanDlg != NULL)
			{
				if (m_pQuickscanDlg->m_bQuickScan)
				{
					if (m_pQuickscanDlg->m_bScanStarted)
					{
						if (ShutDownQuickScan())
						{
							//m_pQuickscanDlg->ScanningFinished();
							m_bIsTabMenuClicked = true;
							bIsProcessAborted = true;
						}
					}
					else
					{
						if (m_pQuickscanDlg->m_bIsCleaning)
						{
							//Varada Ikhar, Date: 4th May-2015
							//Issue : While threats cleaning is in progress, if clicked on any other tab(like(Quick/Full/Custom/Antirootkit Scan, or Registry Optimizer/Updates/Recover/Reports)
							// then cleaning operation is not paused.
							if (m_pQuickscanDlg->CloseCleaning())
							{
								bIsProcessAborted = true;
							}
						}
					}
				}
				
			}
			if (m_bisUIcloseRquestFromTray)			
			{
				CloseSystemTray();
				HandleCloseButton();
			}

			break;
		case FULL_SCAN_DLG:

			if (m_pFullScanDlg->m_bFullScan)
				if (m_pFullScanDlg != NULL)
				{
					if (m_pFullScanDlg->m_bFullScan)
					{
						if (m_pFullScanDlg->m_bScanStarted)
						{
							if (ShutDownFullScan())
							{
								//m_pFullScanDlg->ScanningFinished();
								m_bIsTabMenuClicked = true;
								bIsProcessAborted = true;
							}
						}
						else
						{
							if (m_pFullScanDlg->m_bIsCleaning)
							{
								//Varada Ikhar, Date: 4th May-2015
								//Issue : While threats cleaning is in progress, if clicked on any other tab(like(Quick/Full/Custom/Antirootkit Scan, or Registry Optimizer/Updates/Recover/Reports)
								// then cleaning operation is not paused.
								if (m_pFullScanDlg->CloseCleaning())
								{
									bIsProcessAborted = true;
								}
							}
						}
					}
					
				}

			if (m_bisUIcloseRquestFromTray)
			{
				CloseSystemTray();
				HandleCloseButton();
			}
			break;

		case CUSTOM_SCAN_DLG:
			
			if (m_pCustomScanDlg!= NULL)
			{
				if (m_pCustomScanDlg->m_bCustomscan)
				{
					if (m_pCustomScanDlg->m_bScanStarted)
					{
						if (ShutDownCustomScan())
						{
							//m_pCustomScanDlg->ScanningFinished();
							m_bIsTabMenuClicked = true;
							bIsProcessAborted = true;
						}
					}
					else
					{
						if (m_pCustomScanDlg->m_bIsCleaning)
						{
							//Varada Ikhar, Date: 4th May-2015
							//Issue : While threats cleaning is in progress, if clicked on any other tab(like(Quick/Full/Custom/Antirootkit Scan, or Registry Optimizer/Updates/Recover/Reports)
							// then cleaning operation is not paused.
							if (m_pCustomScanDlg->CloseCleaning())
							{
								bIsProcessAborted = true;
							}
						}
					}
				}
			}

			if (m_bisUIcloseRquestFromTray)
			{
				CloseSystemTray();
				HandleCloseButton();
			}

			break;
		case ANTIROOTKIT_SCAN_DLG:
			
			if (m_pAntirootkitScan!= NULL)
			{
				if (m_pAntirootkitScan->m_bAntirootScanningInProgress)
				{
					m_bIsCloseHandleCalled = true;
					
					if (m_pAntirootkitScan->ShutDownRootkitScanning())
					{
						//m_pAntirootkitScan->RootKitScanningFinished();
						bIsProcessAborted = true;
					}
					else
					{
						m_bisUIcloseRquestFromTray = false;
					}
					m_bIsCloseHandleCalled = false;
				}

			}
			if (m_bisUIcloseRquestFromTray)
			{
				//m_pAntirootkitScan->m_bAntirootClose = true;
				CloseSystemTray();
				HandleCloseButton();
			}
				
			break;

		case REGISTRY_OPTIMIZER_DLG:
			
			if (m_pRegistryOptimizer != NULL)
			{
				if (m_pRegistryOptimizer != NULL && m_pRegistryOptimizer->m_bScanStarted)
				{
					//m_pRegistryOptimizer->m_btnBack.EnableWindow(true);
					m_bIsCloseHandleCalled = true;
					
					if (m_pRegistryOptimizer->ShutDownScanning())
					{
						m_pRegistryOptimizer->m_bScanStop = false;
						bIsProcessAborted = true;

						//Issue: If Any operation is in progress and we want to Right click Crypt operation then to stop all Process and Srart Crypt operation
						//Resolved by : Nitin K. Date: 15th July 2015.
						if (theApp.m_hCryptOprEvent)
						{
							SetEvent(theApp.m_hCryptOprEvent);
						}
						if (theApp.m_hUpdateOprEvent)
						{
							SetEvent(theApp.m_hUpdateOprEvent);
						}
					}
					else
					{
						m_bisUIcloseRquestFromTray = false;
					}
					m_bIsCloseHandleCalled = false;
				}
			}

			if (m_bisUIcloseRquestFromTray)
			{
				//m_pRegistryOptimizer->m_bClose = true;
				m_pRegistryOptimizer->m_bScanStarted = false;
				// resolved by lalit 5-5-2015
				//issue if "do you want to close message exits and we close from tray- 
				//in such case we convert already exist message popup as tray exit,and such case we have to close tray "
				CloseSystemTray();
				HandleCloseButton();
			}
				

			break;

		case DATA_ENCRYPTION_DLG:

			if (m_pDataEncryption != NULL && m_pDataEncryption->m_bIsEnDepInProgress)
			{
				m_bIsCloseHandleCalled = true;
				if (!m_pDataEncryption->ShutDownEncryptDecrypt(1))
				{
					theApp.m_bOnCloseFromMainUI = false;
					m_bisUIcloseRquestFromTray = false;
					bIsProcessAborted = false;
					m_bIsCloseHandleCalled = false;
					return false;
				}
				Sleep(2000);
				m_bIsCloseHandleCalled = false;
				bIsProcessAborted = true;
			}

			if (m_bisUIcloseRquestFromTray)
			{
				CloseSystemTray();
				HandleCloseButton();
			}

			break;

		case RECOVER_DLG:
			//Varada Ikhar, Date: 4th May-2015
			//Issue: While Recover entries are getting deleted and any other tab like(Quick/Full/Custom/Antirootkit Scan, or Registry Optimizer/Data Encryption/Updates/Reports)
			//is clicked, then delete operation is not getting paused.
			if (m_pRecover != NULL && (m_pRecover->m_bRecoverThreadStart || m_pRecover->m_bDeleteThreadStart))
			{
				//m_pRecover->m_bRecoverClose = true;
				m_bIsCloseHandleCalled = true;
				if (m_pRecover->StopReportOperation())
				{
					bIsProcessAborted = true;
					m_pRecover->m_bRecoverThreadStart = false;
					
					//Issue: If Any operation is in progress and we want to Right click Crypt operation then to stop all Process and Srart Crypt operation
					//Resolved by : Nitin K. Date: 15th July 2015.
					if (theApp.m_hCryptOprEvent)
					{
						SetEvent(theApp.m_hCryptOprEvent);
					}
					if (theApp.m_hUpdateOprEvent)
					{
						SetEvent(theApp.m_hUpdateOprEvent);
					}
				}
				else
				{
					m_bisUIcloseRquestFromTray = false;
				}
				m_bIsCloseHandleCalled = false;
				
			}

			if (m_bisUIcloseRquestFromTray)
			{
				CloseSystemTray();
				HandleCloseButton();
			}

			break;

		case WARDWIZ_UPDATES_DLG:

			if (m_pUpdate != NULL && m_pUpdate->m_dlgISpyUpdatesSecond.m_isDownloading != false)//m_pUpdate->m_dlgISpyUpdatesSecond.m_hThread != NULL)
			{
				//m_pUpdate->m_dlgISpyUpdatesSecond.m_bCloseUpdate = true;
				m_bIsCloseHandleCalled = true;
				if (m_pUpdate->m_dlgISpyUpdatesSecond.ShutDownDownload())
				{
					//m_pUpdate->m_dlgISpyUpdatesSecond.m_hThread = NULL;
					m_pUpdate->m_dlgISpyUpdatesSecond.m_bCloseUpdate = false;
					m_bIsCloseHandleCalled = true;
					bIsProcessAborted = true;
					//Issue: If Any operation is in progress and we want to Right click Crypt operation then to stop all Process and Srart Crypt operation
					//Resolved by : Nitin K. Date: 15th July 2015.
					if (theApp.m_hCryptOprEvent)
					{
						SetEvent(theApp.m_hCryptOprEvent);
					}
				}
				else
				{
					m_bisUIcloseRquestFromTray = false;
				}
				m_bIsCloseHandleCalled = false;
			}

			if (m_bisUIcloseRquestFromTray)
			{
				CloseSystemTray();
				HandleCloseButton();
			}

			break;

		case WARDWIZ_REPORTS_DLG:

			if (m_pReports != NULL && m_pReports->m_bReportThreadStart != NULL)
			{
				//m_pReports->m_bReportClose = true;
				m_bIsCloseHandleCalled = true;
				if (m_pReports->StopReportOperation())
				{
					bIsProcessAborted = true;
					m_pReports->m_bReportThreadStart = false;
					m_pReports->m_btnBack.EnableWindow(true);
					m_pReports->m_chkSelectAll.EnableWindow(true);
					m_pReports->m_lstListView.EnableWindow(true);
					m_pReports->m_btnDelete.EnableWindow(true);
					if (theApp.m_hCryptOprEvent)
					{
						SetEvent(theApp.m_hCryptOprEvent);
					}
					if (theApp.m_hUpdateOprEvent)
					{
						SetEvent(theApp.m_hUpdateOprEvent);
					}
				}
				else
				{
					m_bisUIcloseRquestFromTray = false;
				}
				m_bIsCloseHandleCalled = false;
			}

			if (m_bisUIcloseRquestFromTray)
			{
				CloseSystemTray();
				HandleCloseButton();
			}
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in IsAnyTaskInProcess", 0, 0, true, SECONDLEVEL);
	}
	
	return bIsProcessAborted;
}

/***************************************************************************
Function Name  : ShowHomeButtonPage
Description    :  it use to display home page contents.
Author Name    : lalit kumawat
SR_NO		   :
Date           :
Modification   :
Modification   : 28th May 2015, Neha Gharge hide Utility dialog
****************************************************************************/
void CISpyGUIDlg::ShowHomeButtonPage()
{
	try
	{
		if (m_pRegistryOptimizer != NULL)
			m_pRegistryOptimizer->ShowWindow(SW_HIDE);
		if (m_pAntirootkitScan != NULL)
			m_pAntirootkitScan->ShowWindow(SW_HIDE);
		if (m_pDataEncryption != NULL)
			m_pDataEncryption->ShowWindow(SW_HIDE);
		if (m_pRecover != NULL)
			m_pRecover->ShowWindow(SW_HIDE);
		if (m_pFolderLocker != NULL)
			m_pFolderLocker->ShowWindow(SW_HIDE);
		if (m_pVirusscan != NULL)
			m_pVirusscan->ShowWindow(SW_HIDE);
		if (m_pSpamFilter != NULL)
			m_pSpamFilter->ShowWindow(SW_HIDE);
		if (m_pContentFilter != NULL)
			m_pContentFilter->ShowWindow(SW_HIDE);
		if (m_pSignature != NULL)
			m_pSignature->ShowWindow(SW_HIDE);
		if (m_pReports != NULL)
			m_pReports->ShowWindow(SW_HIDE);
		if (m_pUpdate != NULL)
			m_pUpdate->ShowWindow(SW_HIDE);
		if (m_pFullScanDlg != NULL)
			m_pFullScanDlg->ShowWindow(SW_HIDE);
		if (m_pQuickscanDlg != NULL)
			m_pQuickscanDlg->ShowWindow(SW_HIDE);
		if (m_pCustomScanDlg != NULL)
			m_pCustomScanDlg->ShowWindow(SW_HIDE);
		if (m_pUtility != NULL)
			m_pUtility->ShowWindow(SW_HIDE);
		if (m_pHomepageDlg != NULL)
			m_pHomepageDlg->ShowHomepageControls(true);
		SetSkinOnhomePage();
		MainDialogDisplay();
		if (m_pHomepageDlg != NULL)
			m_pHomepageDlg->ShowWindow(SW_SHOW);
		Invalidate();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ShowHomeButtonPage", 0, 0, true, SECONDLEVEL);
	}
	
}

/***************************************************************************
Function Name  : ShutDownQuickScan
Description    :  it use to shutdown quick scan.
Author Name    : lalit kumawat
SR_NO		   :
Date           :
Modification   :
****************************************************************************/
bool CISpyGUIDlg::ShutDownQuickScan()
{
	try
	{
		if (m_pQuickscanDlg != NULL)
		{
			if (m_pQuickscanDlg->m_eScanType == QUICKSCAN)
			{
				//m_pQuickscanDlg->m_bClose = true;
				m_bIsCloseHandleCalled = true;
				if (!m_pQuickscanDlg->ShutDownScanning(true))
				{
					m_pQuickscanDlg->m_bClose = false;
					theApp.m_bOnCloseFromMainUI = false;
					m_bIsCloseHandleCalled = false;
					m_bisUIcloseRquestFromTray = false;
					return false;
				}

				if (m_pQuickscanDlg->m_bIsCleaning == true)
				{
					if (!m_pQuickscanDlg->CloseCleaning())
					{
						return false;
					}
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ShutDownQuickScan", 0, 0, true, SECONDLEVEL);
	}
	
	m_bIsCloseHandleCalled = false;
	
	return true;
}

/***************************************************************************
Function Name  : ShutDownFullScan
Description    :  it use to shutdown full scan.
Author Name    : lalit kumawat
SR_NO		   :
Date           :
Modification   :
****************************************************************************/
bool CISpyGUIDlg::ShutDownFullScan()
{
	try
	{
		if (m_pFullScanDlg != NULL)
		{
			if (m_pFullScanDlg->m_eScanType == FULLSCAN)
			{
				//m_pFullScanDlg->m_bClose = true;
				m_bIsCloseHandleCalled = true;
				if (!m_pFullScanDlg->ShutDownScanning(true))
				{
					m_pFullScanDlg->m_bClose = false;
					theApp.m_bOnCloseFromMainUI = false;
					m_bIsCloseHandleCalled = false;
					m_bisUIcloseRquestFromTray = false;
					return false;
				}
				
				if (m_pFullScanDlg->m_bIsCleaning == true)
				{
					if (!m_pFullScanDlg->CloseCleaning())
					{
						return false;
					}
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ShutDownFullScan", 0, 0, true, SECONDLEVEL);
	}
	m_pFullScanDlg->m_bClose = false;
	m_bIsCloseHandleCalled = false;
	
	return true;
}
/***************************************************************************
Function Name  : ShutDownCustomScan
Description    :  it use to shutdown custom scan.
Author Name    : lalit kumawat
SR_NO		   :
Date           :
Modification   :
****************************************************************************/
bool CISpyGUIDlg::ShutDownCustomScan()
{
	try
	{

		if (m_pCustomScanDlg != NULL)
		{
			if (m_pCustomScanDlg->m_eScanType == CUSTOMSCAN)
			{
				//m_pCustomScanDlg->m_bClose = true;
				m_bIsCloseHandleCalled = true;
				if (!m_pCustomScanDlg->ShutDownScanning(true))
				{
					m_pCustomScanDlg->m_bClose = false;
					theApp.m_bOnCloseFromMainUI = false;
					m_bIsCloseHandleCalled = false;
					m_bisUIcloseRquestFromTray = false;
					return false;
				}

				if (m_pCustomScanDlg->m_bIsCleaning == true)
				{
					if (!m_pCustomScanDlg->CloseCleaning())
					{
						return false;
					}
				}
			}
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ShutDownCustomScan", 0, 0, true, SECONDLEVEL);
	}
	m_bIsCloseHandleCalled = false;

	return true;
}

/***************************************************************************
Function Name  : CloseGUIHandleMutually
Description    :  it use to check is already handle close function called or not.
Author Name    : lalit kumawat
SR_NO		   :
Date           :
Modification   :
****************************************************************************/
bool CISpyGUIDlg::CloseGUIHandleMutually()
{
	
	if (m_bIsCloseHandleCalled)
	{
		m_pUpdate->m_dlgISpyUpdatesSecond.m_bCloseUpdate = true;
		m_pReports->m_bReportClose = true;

		if (m_pAntirootkitScan != NULL)
			m_pAntirootkitScan->m_bAntirootClose = true;


		if (m_pRegistryOptimizer != NULL)
			m_pRegistryOptimizer->m_bClose = true;

		m_pFullScanDlg->m_bClose = true;
		m_pQuickscanDlg->m_bClose = true;
		m_pCustomScanDlg->m_bClose = true;

		//m_pAntirootkitScan->m_bAntirootClose = true;
		//m_pRegistryOptimizer->m_bClose = true;
		return true;
	}
	else
	{
		m_bIsCloseHandleCalled = true;
		return false;
	}
}

/***************************************************************************
Function Name  : CloseSystemTray
Description    :  it use to close tray.
Author Name    : lalit kumawat
SR_NO		   :
Date           :
Modification   :
****************************************************************************/
void CISpyGUIDlg:: CloseSystemTray()
{
	try
	{
		CISpySrvMgmt		iSpySrvMgmtObj;
		CEnumProcess		objEnumProcess;

		if (objEnumProcess.IsProcessRunning(L"WRDWIZTRAY.EXE", false, false, false))
		{
			objEnumProcess.IsProcessRunning(L"WRDWIZTRAY.EXE", true, false, false);
			CCommonFunctions objCCommonFunctions;
			objCCommonFunctions.RefreshTaskbarNotificationArea();
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CloseSystemTray", 0, 0, true, SECONDLEVEL);
	}

}


/***************************************************************************
Function Name  : OnBnClickedButtonSupport
Description    : Domodal for Support Dialog window
Author Name    : Nitin K
SR_NO		   :
Date           : 9th May 2015
Modification   :
****************************************************************************/
void CISpyGUIDlg::OnBnClickedButtonSupport()
{
	try
	{
		m_bIsPopUpDisplayed = true;
		CSupportDlg objSupportDlg;
		objSupportDlg.DoModal();
		m_bIsPopUpDisplayed = false;
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to Domodal Support Window", 0, 0, true, SECONDLEVEL);
	}
}
/***********************************************************************************************
Function Name  : LaunchDataEncDecExe
Description    : Launch the Crypt.exe as commandline with required parameters
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 15th June 2015
***********************************************************************************************/
void CISpyGUIDlg :: ShowDataCryptOpr()
{
	try
	{
		//Issue No: 1020 Neha Gharge.
		// When browse, Password popup is open , User double click on encrypted file or right click any file for encryption
		//or decryption processes get started. So now we avoid that ..When one process is gng on we are not allowing other 
		//process from right click or double click on encrypted files.
		if (theApp.m_bDialogsOpenInDataEnc == true)
		{
			theApp.m_bDataCryptOpr = false;
			return;
		}

		if (m_pHomepageDlg != NULL)
		{
			m_pHomepageDlg->ShowWindow(SW_HIDE);
		}
		HideAllPages();
		//Issue: If Any operation is in progress and we want to Right click Crypt operation then to stop all Process and Srart Crypt operation
		//Resolved by : Nitin K. Date: 15th July 2015.
		DeselctAllButtons();
		Invalidate();
		if (m_pTabDialog != NULL)
		{
			m_pTabDialog->SetSelectedButton(DATA_ENCRYPTION_DLG);
		}

		//Ram: Need to have sanity check for data encryption tab
		if (m_btnDataEncryption != NULL)
		{
			m_btnDataEncryption->SetSkin(theApp.m_hResDLL, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnDataEncryption->SetTextColorA(RGB(4, 4, 4), 0, 1);
			m_btnDataEncryption->SetFocusTextColor(RGB(4, 4, 4));
			m_btnDataEncryption->Invalidate();

			m_pDataEncryption->ShowWindow(SW_SHOW);
			CString csCommandLinePath = L"";
			csCommandLinePath = RemoveUnWantedPipeSymblFromPath(theApp.m_csDataCryptFilePath);
			//m_pDataEncryption->m_Edit_FilePath.SetWindowTextW(theApp.m_csDataCryptFilePath);
			m_pDataEncryption->m_Edit_FilePath.SetWindowTextW(csCommandLinePath);
			//_tcscpy_s(m_pDataEncryption->m_szFilePath, _countof(m_pDataEncryption->m_szFilePath), theApp.m_csDataCryptFilePath);
			if (theApp.m_iDataOpr == ENCRYPTION)
			{
				m_pDataEncryption->m_cbKeepOriginalFile.EnableWindow(true);
				m_pDataEncryption->bEncryption = true;
				m_pDataEncryption->m_btnEncrypt.EnableWindow(true);
				m_pDataEncryption->m_btnEncrypt.SetCheck(BST_CHECKED);
				m_pDataEncryption->m_btnDecrypt.SetCheck(BST_UNCHECKED);
				//Issue: 0000758 : Issue while encrypting using right click option in Data Encryption.
				//Rsolved By: Nitin K. Date: 21st July 2015
				m_pDataEncryption->m_Button_Start.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENCRYPT"));
			}
			if (theApp.m_iDataOpr == DECRYPTION)
			{
				m_pDataEncryption->m_cbKeepOriginalFile.EnableWindow(false);
				m_pDataEncryption->bEncryption = false;
				m_pDataEncryption->m_btnEncrypt.EnableWindow(true);
				m_pDataEncryption->m_btnDecrypt.SetCheck(BST_CHECKED);
				m_pDataEncryption->m_btnEncrypt.SetCheck(BST_UNCHECKED);
				//Issue: Keep original checkbox changes
				//Added By Nitin K Date 6th July 2015
				m_pDataEncryption->m_cbKeepOriginalFile.SetCheck(BST_UNCHECKED);
				//Issue: 0000758 : Issue while encrypting using right click option in Data Encryption.
				//Rsolved By: Nitin K. Date: 21st July 2015
				m_pDataEncryption->m_Button_Start.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DECRYPT"));
			}
			m_pDataEncryption->OnBnClickedButtonStart();
			m_btnDataEncryption->SetSkin(theApp.m_hResDLL, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnDataEncryption->SetTextColorA(RGB(4, 4, 4), 0, 1);
			m_btnDataEncryption->SetFocusTextColor(RGB(4, 4, 4));
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExGUIDlg::ShowDataCryptOpr", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : UpdateDataCryptOpr
Description    : Displays the messages came from Crypt.exe to UI for Data Ecryption/Decryption
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 15th June 2015
***********************************************************************************************/
void CISpyGUIDlg::UpdateDataCryptOpr(LPISPY_PIPE_DATA lpSpyData)
{
	try
	{
		DWORD dwFileStatus = lpSpyData->dwValue;
		DWORD dwInsertNewItem = lpSpyData->dwSecondValue;
		CString csFileStatus = L"";
		CString csFileSaveAsPath = L"";
		CString csProcessedFileCount;
		switch (dwFileStatus)
		{
		case CRYPT_FINISHED:
			//1. Issue with Browse & Stop buttons on Data encryption UI.
			//Resolved By : Nitin K Date: 9th July 2015
			m_pDataEncryption->StopCryptOperations(dwInsertNewItem);
			return;
		case INSERT_NEW_ITEM:
			m_pDataEncryption->m_Button_Start.EnableWindow(true);
			m_pDataEncryption->m_lstDataEncDec.InsertItem(0, theApp.m_csDataCryptFilePath, 0);
			//Issue: 0000614  Data Encryption tab not contains the status control, it should show current operation/Error/Sucess 
			//Resolved By :  Nitin K Date : 1st July 2015
			m_pDataEncryption->m_edtCryptStatus.SetWindowTextW(theApp.m_csDataCryptFilePath);
			break;
		case OPR_FAILED:
			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED");
			break;
		case ENCRYPT_SUCCESS:
			//Implementation : Adding Total file count and Processed file count
			//Added By : Nitin K Date: 2nd July 2015
			theApp.m_dwCryptFileCount++;
			csProcessedFileCount.Format(L"%s %d", theApp.m_objwardwizLangManager.GetString(L"IDS_DATA_CRYPT_TOTAL_PROCESSED_FILES"),  theApp.m_dwCryptFileCount);
			m_pDataEncryption->m_stFileProccessedCount.SetWindowTextW(csProcessedFileCount);
			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_ENCRYPTED");
			break;
		case DECRYPT_SUCCESS:
			//Implementation : Adding Total file count and Processed file count
			//Added By : Nitin K Date: 2nd July 2015
			theApp.m_dwCryptFileCount++;
			csProcessedFileCount.Format(L"%s %d", theApp.m_objwardwizLangManager.GetString(L"IDS_DATA_CRYPT_TOTAL_PROCESSED_FILES"), theApp.m_dwCryptFileCount);
			m_pDataEncryption->m_stFileProccessedCount.SetWindowTextW(csProcessedFileCount);
			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_DECRYPTED");
			break;
		case ALREADY_ENCRYPTED:
			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_ALREADY_ENCRYPTED");
			break;
		case INVALID_FILE:
			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_INVALID_FILE");
			break;
		case PASS_MISMATCH:
			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_PASSWORD_MISMATCH");
			break;
		case NOT_ENCRYPTED:
			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_INVALID_FILE");
			break;
		case ENC_INPROGRESS:
			theApp.m_bIsFileInegrityInprogrs = false;
			csFileStatus = theApp.m_csDataCryptFilePath;
			break;
		case DEC_INPROGRESS:
			theApp.m_bIsFileInegrityInprogrs = false;
			csFileStatus = theApp.m_csDataCryptFilePath;
			break;
		case OPR_CANCELED:
			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_CANCELED");
			break;
		case OPR_FILE_COUNT:
			//Implementation : Adding Total file count and Processed file count
			//Added By : Nitin K Date: 2nd July 2015
			m_pDataEncryption->m_stTotalFileCountText.SetWindowTextW(theApp.m_csDataCryptFilePath);
			return;

		case SAVE_AS:
			//csFileStatus = theApp.m_objwardwizLangManager.GetString(L"Waiting..");
			lpSpyData->iMessageInfo = SAVE_AS;
			csFileSaveAsPath = m_pThis->SaveAsDoublicateFile(theApp.m_csDataCryptFilePath);
			_tcscpy_s(lpSpyData->szFirstParam, csFileSaveAsPath);

			break;
		case FILE_NOT_FOUND:
			//1. Issue with Browse & Stop buttons on Data encryption UI.
			//Resolved By : Nitin K Date: 9th July 2015
			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_FILE_NOT_FOUND");
			break;

			// issue :- file size exceed to 3GB size handline ,now we are not allowing encryption/decryption if file size more then 3GB
			// resolved by lalit kumawat, 7-9-2015
		case FILE_SIZE_MORETHEN_3GB:

			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_FILE_SIZE_EXCEED");
			break;
		case OPR_VERSION_MISMATCH:
			//Neha Gharge 10 july,2015 If DataEncVersionno Mismatched
			csFileStatus.Format(L"%s (%s)", theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_VERSION_MISMATCH"), lpSpyData->szFirstParam);
			break;

		case FILE_LOCKING:
			theApp.m_bIsFileInegrityInprogrs = true;
			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_ENCRYPTION_FINALIZING");
			break;
		case FILE_LOCKING_SUCCESS:
			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_ENCRYPTED");
			theApp.m_bIsFileInegrityInprogrs = false; 
			break;
		case INTIGRITY_CHECKING:
			theApp.m_bIsFileInegrityInprogrs = true;
			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_DECR_INITIALIZING");
			break;
		case INTIGRITY_FAILED:
			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_DECR_INTEGRITY_FAILED");
			theApp.m_bIsFileInegrityInprogrs = false; 
			break; 
		case INTIGRITY_ROLLBACK_FAILED:
			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_DECR_INTEGRITY_ROLLBACK_FAILED");
			theApp.m_bIsFileInegrityInprogrs = false;
			break; 

		case SYSTEM_FILE_FOLDER_PATH:
			m_pDataEncryption->m_bIsSystemFolderPathSelected = true;
			m_pDataEncryption->StopCryptOperations(dwInsertNewItem);
			m_pDataEncryption->m_bIsSystemFolderPathSelected = false;
			return;

		case DISK_SPACE_LOW:
		
			m_pDataEncryption->m_bIsLowDiskSpace = true;
			m_pDataEncryption->m_csMsgforLowDiskSpace = lpSpyData->szFirstParam;
			m_pDataEncryption->StopCryptOperations(dwInsertNewItem);
			m_pDataEncryption->m_bIsLowDiskSpace = false;
			return;
			
		case WARDWIZ_DB_FILE:
			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_ENCR_WARDWIZ_DB_FILE");
			break;

		case ZERO_KB_FILE:
			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_DATA_CRYPT_ZERO_KB_FILE");
			break;

		case FILE_ENC_USING_OLD_VERSION:
			csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_DATA_CRYPT_OLD_VERSION_REFER_CHM"); /*L"File Encrypted using older version, Please refer help file for further details";*/
			break;
		default:
			AddLogEntry(L"### Invalid entry CGenXGUIDlg::UpdateDataCryptOpr", 0, 0, true, SECONDLEVEL);
			break;
		}
		m_pDataEncryption->m_lstDataEncDec.SetItemText(0, 1, csFileStatus);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExGUIDlg::UpdateDataCryptOpr", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : RemoveUnWantedPipeSymblFromPath
Description    : it provides functionality to remove unwanted pipe sysmbol from end of path
SR.NO		   :
Author Name    : Lalit kumawat
Date           : 29th June 2015
***********************************************************************************************/

CString  CISpyGUIDlg::RemoveUnWantedPipeSymblFromPath(CString csSelectedArgumentPath)
{
	CString csPath = L"";
	try
	{
	    csPath = csSelectedArgumentPath.Mid(csSelectedArgumentPath.ReverseFind('|') + 1);

		TCHAR  szTmpPath[6 * MAX_PATH] = { 0 };
		swprintf_s(szTmpPath, _countof(szTmpPath), L"%s", csSelectedArgumentPath);

		if (csSelectedArgumentPath.ReverseFind('|') == -1 && csPath != L"")
		{
			return csSelectedArgumentPath;
		}

		while (csPath == L"")
		{
			CString csPath = csSelectedArgumentPath.Mid(csSelectedArgumentPath.ReverseFind('|') + 1);
			swprintf_s(szTmpPath, _countof(szTmpPath), L"%s", szTmpPath);

			if (csSelectedArgumentPath.ReverseFind('|') == -1 || csPath != L"")
			{
				break;
			}

			TCHAR	*pTemp = wcsrchr(szTmpPath, '|');
			if (!pTemp)
			{
				AddLogEntry(L"### Failed in removing unwanted pipe character from path CGenXCryptDlg::RemoveUnWantedPipeSymblFromPath");
			}
			*pTemp = '\0';
			csSelectedArgumentPath = szTmpPath;
			csPath = csSelectedArgumentPath.Mid(csSelectedArgumentPath.ReverseFind('|') + 1);
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExGUIDlg::RemoveUnWantedPipeSymblFromPath", 0, 0, true, SECONDLEVEL);
	}

	return csSelectedArgumentPath;
}

/***********************************************************************************************
Function Name  : SaveAsDoublicateFile
Description    : it provides functionality to save As option if same name file already exists.
SR.NO		   :
Author Name    : Lalit kumawat
Date           : 2-7- 2015
***********************************************************************************************/
CString  CISpyGUIDlg::SaveAsDoublicateFile(CString csfilePath)
{
	DWORD dwFDialogOutput = 0;
	CString csNewFilePath = L"";
	CString csExtension = L"";
	CString csUserAddExtension = L"";
	CString csFileName = L"";
	try
	{
		
		csFileName = csfilePath.Mid(csfilePath.ReverseFind('\\') + 1);
		int iPos = csFileName.ReverseFind('.');

		if (iPos != -1)
		{
			CString csFileExt;
			csFileExt = csExtension = csfilePath.Mid(csfilePath.ReverseFind('.') + 1);
			csExtension = L"*." + csExtension;

			//0000765 Issue : Save as window shown as seperate window in Data ENC operation
			//Resolved By Nitin K. Date: 23rd July 2015
			CFileDialog objFileDlg(FALSE, csFileExt, csExtension,
				OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, csExtension, m_pThis);

			m_bIsPopUpDisplayed = true;
			if (objFileDlg.DoModal() == IDOK)
			{
				m_bIsPopUpDisplayed = false;
				csNewFilePath = objFileDlg.GetPathName();
				// if user give file name xxxx.exe instead of xxxx or xxxx.wwiz in such case
				//we have to manaually attach .WWIZ extension otherwise right click will not show decrypt option by right clicking on encrypted file.
				if (csExtension.CompareNoCase(L"*.WWIZ") != -1)
				{

					csUserAddExtension = csNewFilePath.Mid(csNewFilePath.ReverseFind('.') + 1);
					if (csExtension.CompareNoCase(L"*.WWIZ") == -1)
					{
						csNewFilePath = csNewFilePath + L".WWIZ";
					}
				}
			}
			else
			{
				m_bIsPopUpDisplayed = false;
				csNewFilePath = L"";
			}
		}
		else
		{
			static TCHAR szEncFilter[] = L"*.txt|";
			CFileDialog objFileDlg(FALSE, NULL, NULL,
				OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, (LPCTSTR)szEncFilter);

			m_bIsPopUpDisplayed = true;
			if (objFileDlg.DoModal() == IDOK)
			{
				m_bIsPopUpDisplayed = false;
				csNewFilePath = objFileDlg.GetPathName();

				// if user give file name xxxx.exe instead of xxxx or xxxx.wwiz in such case
				//we have to manaually attach .WWIZ extension otherwise right click will not show decrypt option by right clicking on encrypted file.
				if (csExtension.CompareNoCase(L"*.WWIZ") != -1)
				{
					csUserAddExtension = csNewFilePath.Mid(csNewFilePath.ReverseFind('.') + 1);
					if (csExtension.CompareNoCase(L"*.WWIZ") == -1)
					{
						csNewFilePath = csNewFilePath + L".WWIZ";
					}
				}

			}
			else
			{
				m_bIsPopUpDisplayed = false;
				csNewFilePath = L"";
			}
		}
		

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExGUIDlg::SaveAsDoublicateFile", 0, 0, true, SECONDLEVEL);
	}

	return csNewFilePath;
}

/***********************************************************************************************
Function Name  : DeselctAllButtons
Description    : Deselect
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 15th June 2015
***********************************************************************************************/
void CISpyGUIDlg::DeselctAllButtons()
{
	try
	{
		if (m_btnQuickScan != NULL)
		{
			m_btnQuickScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnQuickScan->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
			m_btnQuickScan->SetFocusTextColor(RGB(4, 4, 4));
		}
		if (m_btnFullScan != NULL)
		{
			m_btnFullScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnFullScan->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
			m_btnFullScan->SetFocusTextColor(RGB(4, 4, 4));
		}
		if (m_btnCustomScan != NULL)
		{
			m_btnCustomScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnCustomScan->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
			m_btnCustomScan->SetFocusTextColor(RGB(4, 4, 4));
		}
		if (m_btnAntirootkitScan != NULL)
		{
			m_btnAntirootkitScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnAntirootkitScan->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
			m_btnAntirootkitScan->SetFocusTextColor(RGB(4, 4, 4));
		}
		if (m_btnRegistryOptimizer != NULL)
		{
			m_btnRegistryOptimizer->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnRegistryOptimizer->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
			m_btnRegistryOptimizer->SetFocusTextColor(RGB(4, 4, 4));
		}
		if (m_btnDataEncryption != NULL)
		{
			m_btnDataEncryption->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnDataEncryption->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
			m_btnDataEncryption->SetFocusTextColor(RGB(4, 4, 4));
		}
		if (m_btnRecover != NULL)
		{
			m_btnRecover->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnRecover->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
			m_btnRecover->SetFocusTextColor(RGB(4, 4, 4));
		}
		if (m_btnFolderLocker != NULL)
		{
			m_btnFolderLocker->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnFolderLocker->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
			m_btnFolderLocker->SetFocusTextColor(RGB(4, 4, 4));
		}

		if (m_pTabDialog->m_dwEmailScanEnable == 1)
		{
			if (m_pTabDialog->m_bOutlookInstalled == true)
			{
				if (m_bUnRegisterProduct == false)
				{
					if (m_btnVirusScan != NULL)
					{
						m_btnVirusScan->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
						m_btnVirusScan->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
						m_btnVirusScan->SetFocusTextColor(RGB(4, 4, 4));
					}
					if (m_btnSpamFilter != NULL)
					{
						m_btnSpamFilter->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
						m_btnSpamFilter->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
						m_btnSpamFilter->SetFocusTextColor(RGB(4, 4, 4));
					}
					if (m_btnContentFilter != NULL)
					{
						m_btnContentFilter->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
						m_btnContentFilter->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
						m_btnContentFilter->SetFocusTextColor(RGB(4, 4, 4));
					}
					if (m_btnSignature != NULL)
					{
						m_btnSignature->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
						m_btnSignature->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
						m_btnSignature->SetFocusTextColor(RGB(4, 4, 4));
					}
				}
			}
		}
		if (m_btnUpdateNew != NULL)
		{
			m_btnUpdateNew->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnUpdateNew->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
			m_btnUpdateNew->SetFocusTextColor(RGB(4, 4, 4));
		}
		if (m_btnReports != NULL)
		{
			m_btnReports->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnReports->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
			m_btnReports->SetFocusTextColor(RGB(4, 4, 4));
		}
		if (m_btnUtility != NULL)
		{
			m_btnUtility->SetSkin(theApp.m_hResDLL, IDB_BITMAP_150x17, IDB_BITMAP_150x17, IDB_BITMAP_H_150x17, IDB_BITMAP_150x17_DISABLE, 0, 0, 0, 0, 0);
			m_btnUtility->SetTextColorA(RGB(255, 255, 255), RGB(255, 255, 255), 1);
			m_btnUtility->SetFocusTextColor(RGB(4, 4, 4));
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExGUIDlg::DeselctAllButtons", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : DispDataOprAlreadyInProgressMsg
Description    : Display Data Operation already in progress messagebox on UI
SR.NO		   :
Author Name    : Nitin Kolapkar
Date           : 17th July 2015
***********************************************************************************************/
void CISpyGUIDlg::DispDataOprAlreadyInProgressMsg()
{
	try
	{
		//Issue: 0001233 when we Encrypt/Decrypt any file is in progress if we multiple clicks on any encrypted file it gives multiple popups. it should give only one popup.
		//REsolved by: Nitin K. Date 19th Jan 2015
		theApp.m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_DATA_CRYPT_ALREADY_INPROGRESS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION | MB_OK);
		theApp.m_bIsPopUpDisplayed = false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExGUIDlg::DispDataOprAlreadyInProgressMsg", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : IsAnyPopUpDisplayed
Description    : Function to check if any popup is displayed for user input.
				 This function is to avoid multiple popup.
Author Name    : Ramkrushna Shelke
SR_NO		   : 
Date           : 
Modification   : 
****************************************************************************/
bool CISpyGUIDlg::IsAnyPopUpDisplayed()
{
	bool bIsPopUpDisplayed = false;
	try
	{
		 if (m_bIsPopUpDisplayed)
			 bIsPopUpDisplayed = true;

		 else if (m_pHomepageDlg != NULL && m_pHomepageDlg->m_bIsPopUpDisplayed)
			 bIsPopUpDisplayed = true;

		else if (m_pQuickscanDlg != NULL && m_pQuickscanDlg->m_bIsPopUpDisplayed)
			bIsPopUpDisplayed = true;

		else if (m_pFullScanDlg != NULL && m_pFullScanDlg->m_bIsPopUpDisplayed)
			bIsPopUpDisplayed = true;

		else if (m_pCustomScanDlg != NULL && m_pCustomScanDlg->m_bIsPopUpDisplayed)
			bIsPopUpDisplayed = true;

		else if (m_pAntirootkitScan != NULL && m_pAntirootkitScan->m_bIsPopUpDisplayed)
			bIsPopUpDisplayed = true;

		else if (m_pVirusscan != NULL && m_pVirusscan->m_bIsPopUpDisplayed)
			bIsPopUpDisplayed = true;

		else if (m_pFolderLocker != NULL && m_pFolderLocker->m_bIsPopUpDisplayed)
			bIsPopUpDisplayed = true;

		else if (m_pRegistryOptimizer != NULL && m_pRegistryOptimizer->m_bIsPopUpDisplayed)
			bIsPopUpDisplayed = true;

		else if (m_pDataEncryption != NULL && m_pDataEncryption->m_bIsPopUpDisplayed)
			bIsPopUpDisplayed = true;

		else if (m_pRecover != NULL && m_pRecover->m_bIsPopUpDisplayed)
			bIsPopUpDisplayed = true;

		else if (m_pUpdate != NULL && m_pUpdate->m_bIsPopUpDisplayed)
			bIsPopUpDisplayed = true;

		else if (m_pUpdate != NULL && m_pUpdate->IsPopUpDisplayed())
			bIsPopUpDisplayed = true;

		else if (m_pReports != NULL && m_pReports->m_bIsPopUpDisplayed)
			bIsPopUpDisplayed = true;

		else if (theApp.m_bIsPopUpDisplayed)
			bIsPopUpDisplayed = true;
		
		else
			bIsPopUpDisplayed = false;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExGUIDlg::IsAnyPopUpDisplayed", 0, 0, true, SECONDLEVEL);
	}
	return bIsPopUpDisplayed;
}

/***************************************************************************
Function Name  : OnUserMessagesLaunchProdExpMsgBox
Description    : This function get call when some other module send messge
WM_COMMAND	   : LAUNCHPRODEXPMSGBOX
Author Name    : Nitin Kolapkar
Date           : 25th Feb 2016
****************************************************************************/
LRESULT CISpyGUIDlg::OnUserMessagesLaunchProdExpMsgBox(WPARAM wParam, LPARAM lParam)
{
	try
	{
		UNREFERENCED_PARAMETER(wParam);
		UNREFERENCED_PARAMETER(lParam);

		theApp.ShowEvaluationExpiredMsg(true);
		OnBnClickedButtonHome();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExGUIDlg::OnUserMessagesLaunchProdExpMsgBox", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}