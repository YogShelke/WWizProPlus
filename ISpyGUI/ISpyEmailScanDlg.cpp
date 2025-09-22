/**********************************************************************************************************                     
	  Program Name          :     EmailScanDlg.cpp                                                                                                      
	  Description           :     This dialog is using for following functionality
								  1) Virus Scan.
								  2) Spam Filter.
								  3) Content filter.
								  4) Signature.

	  Author Name:                Neha Gharge                                                                                           
	  Date Of Creation      :     
	  Version No            :     1.0.0.2
	  Special Logic Used    :     Add,delete,edit and apply rule, action by user for email scanning. 
	  Modification Log      :               
	  1.Neha Gharge               function in main        Date              CSR NO                                          
***********************************************************************************************************/
// ISpyEmailScanDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"
#include "ISpyEmailScanDlg.h"

// CISpyEmailScanDlg dialog
//CRect m_rect;
LPCTSTR g_OptionsForComboBox[2]={L"Allow",L"Block"};
LPCTSTR g_OptionsForContentFltrComboBox[3]={L"Message contains",L"Subject contains",L"Attachment extension is"};
LPCTSTR g_GUIDofInstalledOutllook[10]={L"11",L"12",L"14",L"15",L"16"};
CString g_DelEmailAddr,g_DelType,g_DelRuleType;

bool CISpyEmailScanDlg::m_bEmailInstanceFlag = false;
CISpyEmailScanDlg* CISpyEmailScanDlg::m_pobjEmailScanDlg = NULL;

IMPLEMENT_DYNAMIC(CISpyEmailScanDlg, CDialog)

/***************************************************************************
  Function Name  : CISpyEmailScanDlg
  Description    : C'tor
  Author Name    : Neha Gharge
  SR.NO          :
  Date           : 5 May 2014
****************************************************************************/

CISpyEmailScanDlg::CISpyEmailScanDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CISpyEmailScanDlg::IDD, pParent)
	, m_bDefaultSigSelected(false)
	, m_bVirusScan(true)
	, m_bSpamFilter(true)
	, m_bContentFilter(true)
	, m_bSignatureFlag(true)
	, m_objEmailScanCom(SERVICE_SERVER, true)
	, m_bIsPopUpDisplayed(false)
{
	m_dwEnableSignature=0;
	m_dwEnableContentFilter=0;
	m_dwEnableSpamFilter=0;
	m_dwAttachScanAction=0;
	m_dwEnableVirusScan=0;
	m_dwReloadSettings=0;
	m_dwEnableVirusPopUp=0;
	m_iPrevCountEmailVScan = 0;
	m_iCountEmailVScan = 0;
}

/***************************************************************************
  Function Name  : CISpyEmailScanDlg
  Description    : D'tor
  Author Name    : Neha Gharge
  SR.NO          :
  Date           : 5 may 2014
****************************************************************************/

CISpyEmailScanDlg::~CISpyEmailScanDlg()
{
	m_bEmailInstanceFlag = false;
}

/***************************************************************************
  Function Name  : GetEmailScanDlgInstance
  Description    : CWnd* pParent
  Author Name    : Ramkrushna Shelke
  SR.NO          :
  Date           : 03 May 2013
****************************************************************************/
CISpyEmailScanDlg* CISpyEmailScanDlg::GetEmailScanDlgInstance(CWnd* pParent)
{
    if(! m_bEmailInstanceFlag)
    {
        m_pobjEmailScanDlg = new CISpyEmailScanDlg(pParent);
        m_bEmailInstanceFlag = true;
        return m_pobjEmailScanDlg;
    }
    else
    {
        return m_pobjEmailScanDlg;
    }
	return NULL;
}

/***************************************************************************
  Function Name  : ResetInstance
  Description    : Reset and instance for CISpyEmailScanDlg from outside.
  Author Name    : Ramkrushna Shelke
  SR.NO          :
  Date           : 03 May 2013
****************************************************************************/
void CISpyEmailScanDlg::ResetInstance()
{
	delete m_pobjEmailScanDlg; // REM : it works even if the pointer is NULL (does nothing then)
	m_pobjEmailScanDlg = NULL; // so ResetInstance will still work.
}

/***************************************************************************
  Function Name  : DoDataExchange
  Description    : Called by the framework to exchange and validate dialog data.
  Author Name    : Neha Gharge
  SR.NO          :
  Date           : 5th May 2014
****************************************************************************/

void CISpyEmailScanDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_BUTTON_VIRUSSCAN, m_btnVirusScan);
	//DDX_Control(pDX, IDC_BUTTON_SPAMFILTER, m_btnSpamFilter);
	//DDX_Control(pDX, IDC_BUTTON_CONTENTFILTER, m_btnContentFilter);
	//DDX_Control(pDX, IDC_BUTTON_SIGNATURE, m_btnSignature);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_BUTTON_APPLY, m_btnApply);
	//DDX_Control(pDX, IDC_BUTTON_BACK, m_btnBack);
	DDX_Control(pDX, IDC_STATIC_EMAIL_HEADERPIC, m_stEmailHeaderBitmap);
	DDX_Control(pDX, IDC_LIST_VIRUSSCAN, m_lstVirusScan);
	DDX_Control(pDX, IDC_RADIO_QUARANTINE,m_btnQuarantine);
	DDX_Control(pDX, IDC_RADIO_REMOVE, m_btnRemove);
	DDX_Control(pDX, IDC_STATIC_QUARANTINE, m_stQuarantineText);
	DDX_Control(pDX, IDC_STATIC_REMOVETEXT, m_stRemoveText);
	DDX_Control(pDX, IDC_LIST_SPAMFILTER, m_lstSpamFilter);
	DDX_Control(pDX, IDC_STATIC_SPAMFILTER_HEADERPIC, m_stSpamFilterHeaderPic);
	DDX_Control(pDX, IDC_LIST_CONTENTFILTER, m_lstContentFilter);
	DDX_Control(pDX, IDC_STATIC_CONTENTFILTER_HEADERPIC, m_contentFilterHeaderPic);
	DDX_Control(pDX, IDC_STATIC_SIGNATURE_HEADERPIC, m_stSignatureHeaderPic);
	DDX_Control(pDX, IDC_STATIC_SETTING_HEADERPIC, m_stSettingHeaderPic);
	DDX_Control(pDX, IDC_CHECK_SIGNATURE_CHECKBOX, m_chkAllowSignature);
	DDX_Control(pDX, IDC_STATIC_SIG_CHECKBOXMSG, m_stAllowSignatureMsg);
	DDX_Control(pDX, IDC_EDIT_SIG_DESCRIPTIONMSG, m_edtSigDesMsg);
	DDX_Control(pDX, IDC_STATIC_GROUPBOX, m_stGrpBoxForRadioBtn);
	DDX_Control(pDX, IDC_STATIC_GROUPBOX_ADDRULE, m_stGrpAddRule);
	DDX_Control(pDX, IDC_STATIC_ADDRULE, m_stAddRule);
	DDX_Control(pDX, IDC_BUTTON_EDIT, m_btnEdit);
	DDX_Control(pDX, IDC_EDIT_EMAILADD, m_edtEmailAddress);
	DDX_Control(pDX, IDC_COMBO_EMAILADDRESS, m_DropList);
	DDX_Control(pDX, IDC_STATIC_ADDARULE, m_stAddRuleHeaderPic);
	DDX_Control(pDX, IDC_COMBO_DROPLIST_CONTENTFILTER, m_DroplistContentFilter);
	DDX_Control(pDX, IDC_BUTTON_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_CHECK_SPAMFILTER, m_chkSpamFilter);
	DDX_Control(pDX, IDC_STATIC_ENABLESPAM, m_stEnableSpamFilter);
	DDX_Control(pDX, IDC_CHECK_VIRUSSCAN, m_chkVirusScan);
	DDX_Control(pDX, IDC_STATIC_ENABLEVIRUSSCAN, m_stEnableVirusScan);
	DDX_Control(pDX, IDC_CHECK_ENABLECONTENT, m_chkContentfilter);
	DDX_Control(pDX, IDC_STATIC_ENABLECONTENT_FILTER, m_stEnableContentFilter);
	DDX_Control(pDX, IDC_STATIC_CONTENTFILTER_SETTING, m_stAddaRuleForContent);
	DDX_Control(pDX, IDC_STATIC_EXAMPLE, m_stForExample);
	DDX_Control(pDX, IDC_CHECK_ALLOWVIRUSPOPUP, m_chkAllowVirusPopUp);
	DDX_Control(pDX, IDC_STATIC_ALLOWVIRUSPOPUP, m_stAllowVirusPopdlg);
	DDX_Control(pDX, IDC_BTN_DEFAULTSIG, m_btnDefaultSig);
	DDX_Control(pDX, IDC_STATIC_VIRUSSCAN_HEADERNAME, m_stVirusscan_HeaderName);
	DDX_Control(pDX, IDC_STATIC_VIRUSSCAN_HEADERDES, m_stVirusscan_HeaderDes);
	DDX_Control(pDX, IDC_STATIC_VIRUSSCAN_SETTING_HEADER, m_stVirusSettingHeader);
	DDX_Control(pDX, IDC_STATIC_SPAMFILTER_HEADER_NAME, m_stSpamHeaderName);
	DDX_Control(pDX, IDC_STATIC_SPAM_HEADER_DES, m_stSpamHeaderDes);
	DDX_Control(pDX, IDC_STATIC_SPAM_SETTING_HEADER_NAME, m_stSpamSettingHeadername);
	DDX_Control(pDX, IDC_STATIC_CONTENT_HEADERNAME, m_stContentHeaderName);
	DDX_Control(pDX, IDC_STATIC_CONTENT_HEADERDES, m_stContentHeaderDes);
	DDX_Control(pDX, IDC_STATIC_CONTENT_SETTING_HEADER, m_stContentSettingHeader);
	DDX_Control(pDX, IDC_STATIC_SIGNATURE_HEADER_NAME, m_stSignture_HeaderName);
	DDX_Control(pDX, IDC_STATIC_SIGNATURE_HEADER_DESCRIPTION, m_stSignature_HeaderDes);
}

/***************************************************************************
* Function Name   : MESSAGE_MAP
* Description     : Handle WM_COMMAND,WM_Messages,user defined message 
				   and notification message from child windows.
*  Author Name    : Neha Gharge  
*  SR_NO		  :
*  Date           : 5th May 2014
****************************************************************************/
BEGIN_MESSAGE_MAP(CISpyEmailScanDlg, CJpegDialog)
	ON_WM_NCHITTEST()
	ON_WM_SETCURSOR()
	//ON_BN_CLICKED(IDC_BUTTON_VIRUSSCAN, &CISpyEmailScanDlg::OnBnClickedButtonVirusscan)
	//ON_BN_CLICKED(IDC_BUTTON_SPAMFILTER, &CISpyEmailScanDlg::OnBnClickedButtonSpamfilter)
	//ON_BN_CLICKED(IDC_BUTTON_CONTENTFILTER, &CISpyEmailScanDlg::OnBnClickedButtonContentfilter)
	//ON_BN_CLICKED(IDC_BUTTON_SIGNATURE, &CISpyEmailScanDlg::OnBnClickedButtonSignature)
	ON_BN_CLICKED(IDOK, &CISpyEmailScanDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_APPLY, &CISpyEmailScanDlg::OnBnClickedButtonApply)
	ON_BN_CLICKED(IDC_RADIO_QUARANTINE, &CISpyEmailScanDlg::OnBnClickedRadioQuarantine)
	ON_BN_CLICKED(IDC_RADIO_REMOVE, &CISpyEmailScanDlg::OnBnClickedRadioRemove)
	//ON_BN_CLICKED(IDC_BUTTON_BACK, &CISpyEmailScanDlg::OnBnClickedButtonBack)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_BUTTON_EDIT, &CISpyEmailScanDlg::OnBnClickedButtonEdit)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &CISpyEmailScanDlg::OnBnClickedButtonDelete)
	ON_BN_CLICKED(IDC_CHECK_SIGNATURE_CHECKBOX, &CISpyEmailScanDlg::OnBnClickedCheckSignatureCheckbox)
	ON_BN_CLICKED(IDC_CHECK_VIRUSSCAN, &CISpyEmailScanDlg::OnBnClickedCheckVirusscan)
	ON_BN_CLICKED(IDC_CHECK_SPAMFILTER, &CISpyEmailScanDlg::OnBnClickedCheckSpamfilter)
	ON_BN_CLICKED(IDC_CHECK_ENABLECONTENT, &CISpyEmailScanDlg::OnBnClickedCheckEnablecontent)
	ON_BN_CLICKED(IDC_CHECK_ALLOWVIRUSPOPUP, &CISpyEmailScanDlg::OnBnClickedCheckAllowviruspopup)
	ON_EN_CHANGE(IDC_EDIT_SIG_DESCRIPTIONMSG, &CISpyEmailScanDlg::OnEnChangeEditSigDescriptionmsg)
	ON_BN_CLICKED(IDC_BTN_DEFAULTSIG, &CISpyEmailScanDlg::OnBnClickedBtnDefaultsig)
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()



/**********************************************************************************************************                     
*  Function Name  :	OnInitDialog                                                     
*  Description    :	Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft 
					Foundation Class Library dialog boxes
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                          
*  Date           : 5 May 2014
**********************************************************************************************************/
// CISpyEmailScanDlg message handlers

BOOL CISpyEmailScanDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);

	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_INNER_DIALOG), _T("JPG")))
	{
		m_bIsPopUpDisplayed = true;
		::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
		m_bIsPopUpDisplayed = false;
	}

	Draw();

	//if(!Load(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_JPG_SETTINGBG), _T("JPG")))
	//{
	//	::MessageBox(NULL, L"Failed.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
	//}
	//Draw();

	m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));

//	m_bSignature=0;
//	m_bVirusScan=1;
//	m_bSignatureFlag=1;
//	m_bSpamFilter=1;
	m_bEditFlag=1;
	m_bDeleteFlag=1;
//	m_bContentFilter=1;
	m_objSpamFilterdb.bWithoutEdit=1;
	m_bReloadSetting=1;
	m_bEnableVirusPopUp=1;
		
	this->GetClientRect(m_rect);
	//SetWindowPos(NULL, 1, 88, m_rect.Width()-5, m_rect.Height() - 5, SWP_NOREDRAW | SWP_NOZORDER);
	
	InitImageList();
	//ReadRegistryEntryofEmailScan();

	//m_stVirusscan_HeaderDes.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_VIRUSSCAN_DES"));
	m_stVirusscan_HeaderDes.SetTextColor(RGB(24,24,24));
	m_stVirusscan_HeaderDes.SetBkColor(RGB(230,232,238));
	m_stVirusscan_HeaderDes.SetFont(&theApp.m_fontWWTextSubTitleDescription);
	m_stVirusscan_HeaderDes.SetWindowPos(&wndTop,m_rect.left + 20,35,400,15,SWP_NOREDRAW);

	//m_stVirusscan_HeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_VIRUSSCAN_HEADER_NAME"));
	m_stVirusscan_HeaderName.SetTextColor(RGB(24,24,24));
	m_stVirusscan_HeaderName.SetBkColor(RGB(230,232,238));
	m_stVirusscan_HeaderName.SetFont(&theApp.m_fontWWTextSmallTitle);
	m_stVirusscan_HeaderName.SetWindowPos(&wndTop,m_rect.left + 20,07,400,31,SWP_NOREDRAW);

	//m_stVirusSettingHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_VIRUSSCAN_SETTING_HEADER_NAME"));
	m_stVirusSettingHeader.SetTextColor(RGB(24,24,24));
	m_stVirusSettingHeader.SetBkColor(RGB(230,232,238));
	m_stVirusSettingHeader.SetFont(&theApp.m_fontWWTextSmallTitle);
	m_stVirusSettingHeader.SetWindowPos(&wndTop,m_rect.left + 20,260,400,30,SWP_NOREDRAW);

	//m_stSpamHeaderDes.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SPAM_DES"));
	m_stSpamHeaderDes.SetTextColor(RGB(24,24,24));
	m_stSpamHeaderDes.SetBkColor(RGB(230,232,238));
	m_stSpamHeaderDes.SetFont(&theApp.m_fontWWTextSubTitleDescription);
	m_stSpamHeaderDes.SetWindowPos(&wndTop,m_rect.left + 20,35,400,15,SWP_NOREDRAW);

	//m_stSpamHeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SPAM_HEADER_NAME"));
	m_stSpamHeaderName.SetTextColor(RGB(24,24,24));
	m_stSpamHeaderName.SetBkColor(RGB(230,232,238));
	m_stSpamHeaderName.SetFont(&theApp.m_fontWWTextSmallTitle);
	m_stSpamHeaderName.SetWindowPos(&wndTop,m_rect.left + 20,07,400,31,SWP_NOREDRAW);

	//m_stSpamSettingHeadername.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SPAM_SETTING_HEADER_NAME"));
	m_stSpamSettingHeadername.SetTextColor(RGB(24,24,24));
	m_stSpamSettingHeadername.SetBkColor(RGB(230,232,238));
	m_stSpamSettingHeadername.SetFont(&theApp.m_fontWWTextSmallTitle);
	m_stSpamSettingHeadername.SetWindowPos(&wndTop, m_rect.left + 20, 260, 550, 30, SWP_NOREDRAW);

	//m_stContentHeaderDes.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CONTENT_DES"));
	m_stContentHeaderDes.SetTextColor(RGB(24,24,24));
	m_stContentHeaderDes.SetBkColor(RGB(230,232,238));
	m_stContentHeaderDes.SetFont(&theApp.m_fontWWTextSubTitleDescription);
	m_stContentHeaderDes.SetWindowPos(&wndTop,m_rect.left + 20,35,400,15,SWP_NOREDRAW);

	//m_stContentHeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CONTENT_HEADER_NAME"));
	m_stContentHeaderName.SetTextColor(RGB(24,24,24));
	m_stContentHeaderName.SetBkColor(RGB(230,232,238));
	m_stContentHeaderName.SetFont(&theApp.m_fontWWTextSmallTitle);
	m_stContentHeaderName.SetWindowPos(&wndTop,m_rect.left + 20,07,400,31,SWP_NOREDRAW);

	//m_stContentSettingHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CONTENT_SETTING_HEADER_NAME"));
	m_stContentSettingHeader.SetTextColor(RGB(24,24,24));
	m_stContentSettingHeader.SetBkColor(RGB(230,232,238));
	m_stContentSettingHeader.SetFont(&theApp.m_fontWWTextSmallTitle);
	m_stContentSettingHeader.SetWindowPos(&wndTop,m_rect.left + 20,260,550,30,SWP_NOREDRAW);

	//m_stSignature_HeaderDes.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SIGNATURE_DES"));
	m_stSignature_HeaderDes.SetTextColor(RGB(24,24,24));
	m_stSignature_HeaderDes.SetBkColor(RGB(230,232,238));
	m_stSignature_HeaderDes.SetFont(&theApp.m_fontWWTextSubTitleDescription);
	m_stSignature_HeaderDes.SetWindowPos(&wndTop,m_rect.left + 20,35,400,15,SWP_NOREDRAW);

	//m_stSignture_HeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SIGNATURE_HEADER_NAME"));
	m_stSignture_HeaderName.SetTextColor(RGB(24,24,24));
	m_stSignture_HeaderName.SetBkColor(RGB(230,232,238));
	m_stSignture_HeaderName.SetFont(&theApp.m_fontWWTextSmallTitle);
	m_stSignture_HeaderName.SetWindowPos(&wndTop,m_rect.left + 20,07,400,31,SWP_NOREDRAW);

	if(theApp.m_dwOSType == WINOS_WIN8 ||theApp.m_dwOSType == WINOS_WIN8_1)
	{
		m_stVirusscan_HeaderName.SetWindowPos(&wndTop,m_rect.left +20,14,400,31,SWP_NOREDRAW);	
		m_stVirusscan_HeaderDes.SetWindowPos(&wndTop,m_rect.left +20,38,400,15,SWP_NOREDRAW);
		m_stSpamHeaderName.SetWindowPos(&wndTop,m_rect.left +20,14,400,31,SWP_NOREDRAW);	
		m_stSpamHeaderDes.SetWindowPos(&wndTop,m_rect.left +20,38,400,15,SWP_NOREDRAW);
		m_stContentHeaderName.SetWindowPos(&wndTop,m_rect.left +20,14,400,31,SWP_NOREDRAW);	
		m_stContentHeaderDes.SetWindowPos(&wndTop,m_rect.left +20,38,400,15,SWP_NOREDRAW);
		m_stSignture_HeaderName.SetWindowPos(&wndTop,m_rect.left +20,14,400,31,SWP_NOREDRAW);	
		m_stSignature_HeaderDes.SetWindowPos(&wndTop,m_rect.left +20,38,400,15,SWP_NOREDRAW);
	}

	m_chkVirusScan.SetWindowPos(&wndTop,m_rect.left + 8,68,13,13,SWP_NOREDRAW | SWP_NOZORDER);
	

	m_stEnableVirusScan.SetWindowPos(&wndTop,m_rect.left + 30,68,140,20,SWP_NOREDRAW | SWP_NOZORDER);
	m_stEnableVirusScan.SetBkColor(RGB(105,105,105));
	m_stEnableVirusScan.SetFont(&theApp.m_fontWWTextNormal);
	m_stEnableVirusScan.SetTextColor(RGB(255,255,255));
	//m_stEnableVirusScan.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENABLE_VIRUSSCAN"));
	
	
	//m_lstVirusScan.InsertColumn(0, L" ", LVCFMT_LEFT, 25);
	//m_lstVirusScan.InsertColumn(1, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SENDER_ADDR"), LVCFMT_LEFT, 235);
	//m_lstVirusScan.InsertColumn(2, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SUBJECT"), LVCFMT_LEFT, 210);
	//m_lstVirusScan.InsertColumn(3, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_STATUS_VIRUSSCAN"), LVCFMT_LEFT, 108);
	ListView_SetExtendedListViewStyle (m_lstVirusScan.m_hWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);
	m_lstVirusScan.SetWindowPos(&wndTop,m_rect.left + 6 ,93,582,260,SWP_NOREDRAW | SWP_NOZORDER);

	CHeaderCtrl* pHeaderCtrl = m_lstVirusScan.GetHeaderCtrl();
	pHeaderCtrl->SetFont(&theApp.m_fontWWTextNormal);
	
	
	m_chkSpamFilter.SetWindowPos(&wndTop,m_rect.left + 8,68,13,13,SWP_NOREDRAW | SWP_NOZORDER);


	m_stEnableSpamFilter.SetWindowPos(&wndTop,m_rect.left + 30,68,140,20,SWP_NOREDRAW | SWP_NOZORDER);
	m_stEnableSpamFilter.SetBkColor(RGB(105,105,105));
	m_stEnableSpamFilter.SetFont(&theApp.m_fontWWTextNormal);
	m_stEnableSpamFilter.SetTextColor(RGB(255,255,255));
	//m_stEnableSpamFilter.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENABLE_SPAMFILETER"));

	//m_lstSpamFilter.InsertColumn(0, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TYPE_SPAMFILTER"), LVCFMT_LEFT, 109);
	//m_lstSpamFilter.InsertColumn(1, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_EMAILADDR_SPAMFILTER"), LVCFMT_LEFT, 467);
	ListView_SetExtendedListViewStyle (m_lstSpamFilter.m_hWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);
	m_lstSpamFilter.SetWindowPos(&wndTop,m_rect.left + 6 ,93,582,150,SWP_NOREDRAW | SWP_NOZORDER);
	pHeaderCtrl = m_lstSpamFilter.GetHeaderCtrl();
	pHeaderCtrl->SetFont(&theApp.m_fontWWTextNormal);

	
	m_chkContentfilter.SetWindowPos(&wndTop,m_rect.left + 8,68,13,13,SWP_NOREDRAW | SWP_NOZORDER);


	m_stEnableContentFilter.SetWindowPos(&wndTop,m_rect.left + 30,68,140,20,SWP_NOREDRAW | SWP_NOZORDER);
	m_stEnableContentFilter.SetBkColor(RGB(105,105,105));
	m_stEnableContentFilter.SetFont(&theApp.m_fontWWTextNormal);
	m_stEnableContentFilter.SetTextColor(RGB(255,255,255));
	m_stEnableContentFilter.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENABLE_CONTETNT"));

	//m_lstContentFilter.InsertColumn(0,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ACTION_CONTENT"), LVCFMT_LEFT, 109);
	//m_lstContentFilter.InsertColumn(1,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_RULETYPE_CONTENT"),  LVCFMT_LEFT, 227);
	//m_lstContentFilter.InsertColumn(2,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_RULE_CONTENT"), LVCFMT_LEFT, 240);
	ListView_SetExtendedListViewStyle (m_lstContentFilter.m_hWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);
	m_lstContentFilter.SetWindowPos(&wndTop,m_rect.left + 6 ,93,582,150,SWP_NOREDRAW | SWP_NOZORDER);
	pHeaderCtrl = m_lstContentFilter.GetHeaderCtrl();
	pHeaderCtrl->SetFont(&theApp.m_fontWWTextNormal);

	m_DroplistContentFilter.ShowWindow(SW_HIDE);
	m_edtEmailAddress.SetWindowPos(&wndTop,m_rect.left + 11,315,180,20,SWP_NOREDRAW | SWP_NOZORDER);
	m_edtEmailAddress.SetWindowTextW(L"");
	m_edtEmailAddress.SetLimitText(250);

	m_stForExample.SetWindowPos(&wndTop,m_rect.left + 14,345,400,15,SWP_NOREDRAW | SWP_NOZORDER);
	m_stForExample.SetBkColor(RGB(251,252,254));
	m_stForExample.SetTextColor(RGB(0,0,0));
	m_stForExample.SetFont(&theApp.m_fontWWTextNormal);
	//m_stForExample.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_EXAMPLE_SPAM"));

	m_DropList.SetWindowPos(&wndTop,m_rect.left +265 ,315,50,20,SWP_SHOWWINDOW | SWP_NOZORDER);

	m_pHeaderFont = new CFont;
	m_pHeaderFont->CreateFont(15,8,0,0,FW_SEMIBOLD,0,0,0,0,0,0,ANTIALIASED_QUALITY,0,L"verdana");
	m_stGrpAddRule.SetWindowPos(&wndTop,m_rect.left + 6,250,539,35,SWP_NOREDRAW | SWP_NOZORDER);
	
	m_stAddRule.SetWindowPos(&wndTop,m_rect.left + 15,255,90,15,SWP_NOREDRAW | SWP_NOZORDER);
	m_stAddRule.SetBkColor(RGB(154,207,205));
	m_stAddRule.SetFont(&theApp.m_fontWWTextNormal);
	m_stAddRule.SetTextColor(BLACK);

	m_stGrpBoxForRadioBtn.SetWindowPos(&wndTop,m_rect.left + 7,285,580,103,SWP_NOREDRAW | SWP_NOZORDER);
	m_stGrpBoxForRadioBtn.ShowWindow(SW_HIDE);
	m_chkAllowVirusPopUp.SetWindowPos(&wndTop,m_rect.left + 21,302,13,13,SWP_NOREDRAW | SWP_NOZORDER);
	m_chkAllowVirusPopUp.SetCheck(TRUE);
	m_chkAllowVirusPopUp.ShowWindow(SW_HIDE);

	m_stAllowVirusPopdlg.SetWindowPos(&wndTop,m_rect.left + 43,302,400,15,SWP_NOREDRAW | SWP_NOZORDER);
	m_stAllowVirusPopdlg.SetBkColor(RGB(251,252,254));
	m_stAllowVirusPopdlg.SetTextColor(RGB(0,0,0));
	m_stAllowVirusPopdlg.SetFont(&theApp.m_fontWWTextNormal);
	m_stAllowVirusPopdlg.ShowWindow(SW_HIDE);
	//m_stAllowVirusPopdlg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_VIRUS_POPUP"));
	
	/*****************ISSUE NO -229 Neha Gharge 22/5/14 ************************************************/
	
	m_btnQuarantine.SetSkin(theApp.m_hResDLL,IDB_BITMAP_DISABLE,IDB_BITMAP_SELECT,IDB_BITMAP_DISABLE,IDB_BITMAP_DISABLE,0,0,0,0,0);
	m_btnQuarantine.SetWindowPos(&wndTop,m_rect.left + 41,327,21,20,SWP_NOREDRAW | SWP_NOZORDER);
	m_btnQuarantine.SetCheck(TRUE);
	m_btnQuarantine.EnableWindow(FALSE);
		
	m_stQuarantineText.SetWindowPos(&wndTop,m_rect.left + 66,329,85,15,SWP_NOREDRAW | SWP_NOZORDER);
	m_stQuarantineText.SetBkColor(RGB(251,252,254));
	m_stQuarantineText.SetTextColor(RGB(0,0,0));
	m_stQuarantineText.SetFont(&theApp.m_fontWWTextNormal);
	//m_stQuarantineText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_RADIO_REPAIR_TEXT "));
	m_stQuarantineText.EnableWindow(FALSE);
	
	m_btnRemove.SetSkin(theApp.m_hResDLL,IDB_BITMAP_DISABLE,IDB_BITMAP_SELECT,IDB_BITMAP_DISABLE,IDB_BITMAP_DISABLE,0,0,0,0,0);
	m_btnRemove.SetWindowPos(&wndTop,m_rect.left + 41,353,21,20,SWP_NOREDRAW | SWP_NOZORDER);
	m_btnRemove.EnableWindow(FALSE);
	
	m_stRemoveText.SetWindowPos(&wndTop,m_rect.left + 66,355,65,15,SWP_NOREDRAW | SWP_NOZORDER);
	m_stRemoveText.SetBkColor(RGB(251,252,254));
	m_stRemoveText.SetTextColor(RGB(0,0,0));
	m_stRemoveText.SetFont(&theApp.m_fontWWTextNormal);
	//m_stRemoveText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_RADIO_QAURANTINE_TEXT"));
	m_stRemoveText.EnableWindow(FALSE);
		
	m_chkAllowSignature.SetWindowPos(&wndTop,m_rect.left + 8,68,13,13,SWP_NOREDRAW | SWP_NOZORDER);

	m_stAllowSignatureMsg.SetWindowPos(&wndTop,m_rect.left + 30,68,140,20,SWP_NOREDRAW | SWP_NOZORDER);
	m_stAllowSignatureMsg.SetBkColor(RGB(105,105,105));
	m_stAllowSignatureMsg.SetFont(&theApp.m_fontWWTextNormal);
	m_stAllowSignatureMsg.SetTextColor(RGB(255,255,255));
	//m_stAllowSignatureMsg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENABLE_SIGNATURE"));

	m_edtSigDesMsg.SetWindowPos(&wndTop,m_rect.left + 6,93,582,230,SWP_NOREDRAW | SWP_NOZORDER);

	m_btnOk.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG ,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnOk.SetWindowPos(&wndTop, m_rect.left + 323 ,315,57,21,SWP_SHOWWINDOW|SWP_NOZORDER);
	//m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"));
	m_btnOk.SetFont(&theApp.m_fontWWTextNormal);
	m_btnOk.SetTextColorA(BLACK,1,1);

	//Issue 1012 Position changes of delete and edit button. 
	m_btnDelete.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG ,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnDelete.SetWindowPos(&wndTop, m_rect.left + 390 ,315,57,21,SWP_SHOWWINDOW|SWP_NOZORDER);
	//m_btnDelete.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_DELETE"));
	m_btnDelete.SetFont(&theApp.m_fontWWTextNormal);
	m_btnDelete.SetTextColorA(BLACK,1,1);

	m_btnApply.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG ,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnApply.SetWindowPos(&wndTop,m_rect.left +520,362,57,21,SWP_NOREDRAW | SWP_NOZORDER);
	//m_btnApply.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_APPLY"));
	m_btnApply.SetFont(&theApp.m_fontWWTextNormal);
	m_btnApply.SetTextColorA(BLACK,1,1);
	m_btnApply.EnableWindow(FALSE);

	m_btnEdit.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG ,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnEdit.SetWindowPos(&wndTop, m_rect.left + 457, 315, 57, 21, SWP_SHOWWINDOW | SWP_NOZORDER);
	//m_btnEdit.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_EDIT"));
	m_btnEdit.SetFont(&theApp.m_fontWWTextNormal);
	m_btnEdit.SetTextColorA(BLACK,1,1);

	//m_btnBack.SetSkin(IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROWDISABLE,0,0,0,0);
	//m_btnBack.SetWindowPos(&wndTop, m_rect.left+ 21, 354,31,32, SWP_NOREDRAW | SWP_NOZORDER);

	//m_DropList.AddString(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE"));
	//m_DropList.AddString(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"));
	//m_DropList.SetCurSel(0);
	
	

/*	m_DroplistContentFilter.AddString(theApp.m_objwardwizLangManager.GetString(L"IDS_ATTACHMENT_EXT_IS_CONTENT"));
	m_DroplistContentFilter.AddString(theApp.m_objwardwizLangManager.GetString(L"IDS_MESSAGE_CONTAIN_CONTENT"));
	m_DroplistContentFilter.AddString(theApp.m_objwardwizLangManager.GetString(L"IDS_SUBJECT_CONTAIN_CONTECT"));
	m_DroplistContentFilter.SetCurSel(0);*/
	
	
	m_btnDefaultSig.SetSkin(theApp.m_hResDLL,IDB_BITMAP_REGOPT_BUTTON,IDB_BITMAP_REGOPT_BUTTON,IDB_BITMAP_REGOPT_BUTTON_HOVER,IDB_BITMAP_REGOPT_BUTTON_DIS,0,0,0,0,0);
	m_btnDefaultSig.SetWindowPos(&wndTop,m_rect.left + 405,333,115,22,SWP_NOREDRAW | SWP_NOZORDER);
	//m_btnDefaultSig.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_DEFAULT_SIG"));
	if(theApp.m_dwSelectedLangID == 1)
	{
		m_btnDefaultSig.SetFont(&theApp.m_fontWWTextNormal);
	}
	m_btnDefaultSig.SetTextColorA(BLACK,1,1);
	m_btnDefaultSig.EnableWindow(FALSE);
	RefreshString();
	m_DropList.SetCurSel(0);
	m_DroplistContentFilter.SetCurSel(0);

	//if(!m_bVirusScan)
	//{
	//	OnBnClickedButtonVirusscan();
	//}
	//if(!m_bSpamFilter)
	//{
	//	OnBnClickedButtonSpamfilter();
	//}
	//if(!m_bContentFilter)
	//{
	//	OnBnClickedButtonContentfilter();
	//}
	//if(!m_bSignatureFlag)
	//{
	//	OnBnClickedButtonSignature();
	//}
	///LoadExistingVirusScanFile();

	//LoadExistingVirusScanFile();
	m_bmpEmailHeaderBitmap = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_COM_HEADER));
	m_stEmailHeaderBitmap.SetBitmap(m_bmpEmailHeaderBitmap);
	m_stEmailHeaderBitmap.SetWindowPos(&wndTop,m_rect.left + 6,10,582,45,SWP_NOREDRAW);


//	//m_bmpSpamFilterHeader.LoadBitmapW(IDB_BITMAP_SPAMFILTER_HEADERPIC);
	m_stSpamFilterHeaderPic.SetBitmap(m_bmpEmailHeaderBitmap);
	m_stSpamFilterHeaderPic.SetWindowPos(&wndTop,m_rect.left + 6,10,582,45,SWP_NOREDRAW);
//
//
//	//m_bmpContentfilterHeader.LoadBitmapW(IDB_BITMAP_CONTENTFILTER_HEADERPIC);
	m_contentFilterHeaderPic.SetBitmap(m_bmpEmailHeaderBitmap);
	m_contentFilterHeaderPic.SetWindowPos(&wndTop,m_rect.left + 6,10,582,45,SWP_NOREDRAW);
//
//
////m_bmpSignatureHeader.LoadBitmapW(IDB_BITMAP_SIGNATURE_HEADERPIC);
	m_stSignatureHeaderPic.SetBitmap(m_bmpEmailHeaderBitmap);
	m_stSignatureHeaderPic.SetWindowPos(&wndTop,m_rect.left + 6,10,582,45,SWP_NOREDRAW);


	//m_bmpSettingHeader.LoadBitmapW(IDB_BITMAP_HEADER_SETTING);
	m_stSettingHeaderPic.SetBitmap(m_bmpEmailHeaderBitmap);
	m_stSettingHeaderPic.SetWindowPos(&wndTop,m_rect.left + 6,250,582,42,SWP_NOREDRAW);
	m_stSettingHeaderPic.ShowWindow(SW_HIDE);
//
//	//m_bmpAddRuleHeader.LoadBitmapW(IDB_BITMAP_HEADER_ADDRULE);
	m_stAddRuleHeaderPic.SetBitmap(m_bmpEmailHeaderBitmap);
	m_stAddRuleHeaderPic.SetWindowPos(&wndTop,m_rect.left + 6,250,582,42,SWP_NOREDRAW);
//
//
//	//m_bmpAddaRuleForContent.LoadBitmapW(IDB_BITMAP_ADDARULE_CONTENT);
	m_stAddaRuleForContent.SetBitmap(m_bmpEmailHeaderBitmap);
	m_stAddaRuleForContent.SetWindowPos(&wndTop,m_rect.left + 6,250,582,42,SWP_NOREDRAW);


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonVirusscan                                                     
*  Description    :	Controls Hide and show after click on virus scan button on GUI 
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
**********************************************************************************************************/
bool CISpyEmailScanDlg::OnBnClickedButtonVirusscan()
{
	// TODO: Add your control notification handler code here
	// ISsue no 691 neha gharge 13/6/2014
	CISpyGUIDlg * g_ObjIspyGUIHwnd;
	g_ObjIspyGUIHwnd = (CISpyGUIDlg *) (CISpyGUIDlg*)AfxGetMainWnd();	
	if(g_ObjIspyGUIHwnd != NULL)
	{
		//Ram, Issue No: 0001216, Resolved
		if (theApp.m_dwDaysLeft == 0)
		{
			theApp.GetDaysLeft();
		}

		if(theApp.m_dwDaysLeft == 0)
		{
			if(!theApp.ShowEvaluationExpiredMsg())
			{
				theApp.GetDaysLeft();
				g_ObjIspyGUIHwnd->OnBnClickedButtonHome();
				return false;
			}
		}
	}

	//If we installed outlook and Select Email scan option, all three option get highlighted
	//Neha Gharge 12 Aug,2015
	bool bOutlookInstalled = GetExistingPathofOutlook();
	if(!bOutlookInstalled)
	{
		m_bIsPopUpDisplayed = true;
		if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_INSTALL_OUTLOOK"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) == IDOK)
		{
			m_bIsPopUpDisplayed = false;
			//issue Disable 'Email scan' ->click on any of the feature in email scan->it will give an popup 'Do you want to enable email scan'->click 'OK'->again click on the same feature then its not giving any popup.
			// resolved by lalit kumawat 18-8-2015
			if (g_ObjIspyGUIHwnd->m_pTabDialog != NULL)
			{
				g_ObjIspyGUIHwnd->m_pTabDialog->m_SelectedButton = 0;
			}
			return false;
		}
		m_bIsPopUpDisplayed = false;
	}
	else
	{
		g_ObjIspyGUIHwnd->MainDialogDisplay();
	}

	if(g_ObjIspyGUIHwnd != NULL)
	{
		/*if(theApp.m_dwDaysLeft == 0)
		{
			if(!theApp.ShowEvaluationExpiredMsg())
			{
				theApp.GetDaysLeft();
				g_ObjIspyGUIHwnd->OnBnClickedButtonHome();
				return false;
			}
		}*/
		DWORD dwEmailEnable = g_ObjIspyGUIHwnd->ReadEmailScanEntryFromRegistry();
		if(!dwEmailEnable)
		{	
			m_bIsPopUpDisplayed = true;
			if(MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_ENABLE_EMAILSCAN"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONQUESTION|MB_YESNO)==IDNO)
			{
				m_bIsPopUpDisplayed = false;
				//issue Disable 'Email scan' ->click on any of the feature in email scan->it will give an popup 'Do you want to enable email scan'->click 'OK'->again click on the same feature then its not giving any popup.
				// resolved by lalit kumawat 18-8-2015
				if (g_ObjIspyGUIHwnd->m_pTabDialog != NULL)
				{
					g_ObjIspyGUIHwnd->m_pTabDialog->m_SelectedButton = 0;
				}
				return false;
			}
			else
			{
				//Issue no 1266. Even if the Email Scan feature is off, the email scan feature window remains open.
				m_bIsPopUpDisplayed = false;
				if (g_ObjIspyGUIHwnd->m_pTabDialog != NULL)
				{
					g_ObjIspyGUIHwnd->m_pTabDialog->m_SelectedButton = EMAIL_VIRUS_SCAN_DLG;
				}
				WriteRegistryEntryOfMenuItem(L"dwEmailScan",1);
				g_ObjIspyGUIHwnd->MainDialogDisplay();
			}
					
		}
	}
	/*	ISSUE NO - 637 NAME - NITIN K. TIME - 10th June 2014 */
	PopulateList();
	m_btnOk.ShowWindow(SW_HIDE);
	m_DroplistContentFilter.ShowWindow(SW_HIDE);
	m_stGrpAddRule.ShowWindow(SW_HIDE);
	m_stAddRule.ShowWindow(SW_HIDE);
	m_stSignatureHeaderPic.ShowWindow(SW_HIDE);
	m_stSpamFilterHeaderPic.ShowWindow(SW_HIDE);
	m_contentFilterHeaderPic.ShowWindow(SW_HIDE);
	m_lstSpamFilter.ShowWindow(SW_HIDE);
	m_lstContentFilter.ShowWindow(SW_HIDE);
	m_chkAllowSignature.ShowWindow(SW_HIDE);
	m_stAllowSignatureMsg.ShowWindow(SW_HIDE);
	m_edtSigDesMsg.ShowWindow(SW_HIDE);
	m_edtEmailAddress.ShowWindow(SW_HIDE);
	m_DropList.ShowWindow(SW_HIDE);
	m_btnEdit.ShowWindow(SW_HIDE);
	m_btnDelete.ShowWindow(SW_HIDE);
	m_chkSpamFilter.ShowWindow(SW_HIDE);
	m_stEnableSpamFilter.ShowWindow(SW_HIDE);
	m_chkContentfilter.ShowWindow(SW_HIDE);
	m_stEnableContentFilter.ShowWindow(SW_HIDE);
	m_stAddaRuleForContent.ShowWindow(SW_HIDE);
	m_stAddRuleHeaderPic.ShowWindow(SW_HIDE);
	m_stForExample.ShowWindow(SW_HIDE);
	m_stSpamHeaderDes.ShowWindow(SW_HIDE);
	m_stSpamHeaderName.ShowWindow(SW_HIDE);
	m_stSpamSettingHeadername.ShowWindow(SW_HIDE);
	m_stContentHeaderDes.ShowWindow(SW_HIDE);
	m_stContentHeaderName.ShowWindow(SW_HIDE);
	m_stContentSettingHeader.ShowWindow(SW_HIDE);
	m_stSignature_HeaderDes.ShowWindow(SW_HIDE);
	m_stSignture_HeaderName.ShowWindow(SW_HIDE);
	m_bSignature=0;
	m_bSpamFilter=1;
	m_bContentFilter=1;
	m_bSignatureFlag=1;
	m_bEnableVirusPopUp=1;
	m_bUpdate = 1;
	m_bVirusScan=0;
	ReadRegistryEntryofEmailScan();
	m_stVirusscan_HeaderName.ShowWindow(SW_SHOW);
	m_stVirusscan_HeaderDes.ShowWindow(SW_SHOW);
	m_stEmailHeaderBitmap.ShowWindow(SW_SHOW);
	m_stVirusSettingHeader.ShowWindow(SW_HIDE);
	m_btnApply.SetSkin(theApp.m_hResDLL, IDB_BITMAP_BTN57x21, IDB_BITMAP_BTN57x21, IDB_BITMAP_57x21_H_over, IDB_BITMAP_57x21_DISABLE, 0, 0, 0, 0,0);
	m_btnApply.SetWindowPos(&wndTop,m_rect.left +530,362 ,57,21,SWP_NOREDRAW);
	m_btnApply.EnableWindow(FALSE);
	m_chkAllowVirusPopUp.ShowWindow(SW_HIDE);
	m_stAllowVirusPopdlg.ShowWindow(SW_HIDE);
	m_chkVirusScan.ShowWindow(SW_SHOW);
	m_stEnableVirusScan.ShowWindow(SW_SHOW);
	m_lstVirusScan.ShowWindow(SW_SHOW);
	m_stSettingHeaderPic.ShowWindow(SW_HIDE);
	m_btnQuarantine.ShowWindow(SW_HIDE);
	m_btnRemove.ShowWindow(SW_HIDE);
	m_stQuarantineText.ShowWindow(SW_HIDE);
	m_stRemoveText.ShowWindow(SW_HIDE);
	m_stGrpBoxForRadioBtn.ShowWindow(SW_HIDE);
	m_btnDefaultSig.ShowWindow(SW_HIDE);
	LoadExistingVirusScanFile();
	Invalidate(1);
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonSpamfilter                                                     
*  Description    :	Controls Hide and show after click on spam filter button on GUI 
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
**********************************************************************************************************/

bool CISpyEmailScanDlg::OnBnClickedButtonSpamfilter()
{
	// TODO: Add your control notification handler code here
	// ISsue no 691 neha gharge 13/6/2014
	CISpyGUIDlg * g_ObjIspyGUIHwnd;
	g_ObjIspyGUIHwnd = (CISpyGUIDlg *) (CISpyGUIDlg*)AfxGetMainWnd();	

	if(g_ObjIspyGUIHwnd != NULL)
	{
		//Ram, Issue No: 0001216, Resolved
		if (theApp.m_dwDaysLeft == 0)
		{
			theApp.GetDaysLeft();
		}

		if(theApp.m_dwDaysLeft == 0)
		{
			if(!theApp.ShowEvaluationExpiredMsg())
			{
				theApp.GetDaysLeft();
				g_ObjIspyGUIHwnd->OnBnClickedButtonHome();
				return false;
			}
		}
	}

	//If we installed outlook and Select Email scan option, all three option get highlighted
	//Neha Gharge 12 Aug,2015
	bool bOutlookInstalled = GetExistingPathofOutlook();
	if(!bOutlookInstalled)
	{
		m_bIsPopUpDisplayed = true;
		if(MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_INSTALL_OUTLOOK"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK)==IDOK)
		{
			m_bIsPopUpDisplayed = false;
			//issue Disable 'Email scan' ->click on any of the feature in email scan->it will give an popup 'Do you want to enable email scan'->click 'OK'->again click on the same feature then its not giving any popup.
			// resolved by lalit kumawat 18-8-2015
			if (g_ObjIspyGUIHwnd->m_pTabDialog != NULL)
			{
				g_ObjIspyGUIHwnd->m_pTabDialog->m_SelectedButton = 0;
			}
			return false;
		}
		m_bIsPopUpDisplayed = false;
	}
	else
	{
		g_ObjIspyGUIHwnd->MainDialogDisplay();
	}

	if(g_ObjIspyGUIHwnd != NULL)
	{
		/*if(theApp.m_dwDaysLeft == 0)
		{
			if(!theApp.ShowEvaluationExpiredMsg())
			{
				theApp.GetDaysLeft();
				g_ObjIspyGUIHwnd->OnBnClickedButtonHome();
				return false;
			}
		}*/

		DWORD dwEmailEnable = g_ObjIspyGUIHwnd->ReadEmailScanEntryFromRegistry();
		if(!dwEmailEnable)
		{	
			m_bIsPopUpDisplayed = true;
			if(MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_ENABLE_EMAILSCAN"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONQUESTION|MB_YESNO)==IDNO)
			{
				m_bIsPopUpDisplayed = false;
				//issue Disable 'Email scan' ->click on any of the feature in email scan->it will give an popup 'Do you want to enable email scan'->click 'OK'->again click on the same feature then its not giving any popup.
				// resolved by lalit kumawat 18-8-2015
				if (g_ObjIspyGUIHwnd->m_pTabDialog != NULL)
				{
					g_ObjIspyGUIHwnd->m_pTabDialog->m_SelectedButton = 0;
				}
				return false;
			}
			else
			{
				//Issue no 1266. Even if the Email Scan feature is off, the email scan feature window remains open.
				m_bIsPopUpDisplayed = false;
				if (g_ObjIspyGUIHwnd->m_pTabDialog != NULL)
				{
					g_ObjIspyGUIHwnd->m_pTabDialog->m_SelectedButton = EMAIL_SPAM_FILTER_DLG;
				}
				WriteRegistryEntryOfMenuItem(L"dwEmailScan",1);
				g_ObjIspyGUIHwnd->MainDialogDisplay();
			}
		}
	}
	/*	ISSUE NO - 637 NAME - NITIN K. TIME - 10th June 2014 */
	// resolved by lalit kumawat 8-7-2015
	// issue: nevigation on Spamfilter taking long time due to duplicate list populating, this call again added after DB file loading
//	PopulateList();
	m_DroplistContentFilter.ShowWindow(SW_HIDE);
	m_btnQuarantine.ShowWindow(SW_HIDE);
	m_chkAllowVirusPopUp.ShowWindow(SW_HIDE);
	m_stAllowVirusPopdlg.ShowWindow(SW_HIDE);
	m_btnRemove.ShowWindow(SW_HIDE);
	m_stQuarantineText.ShowWindow(SW_HIDE);
	m_stRemoveText.ShowWindow(SW_HIDE);
	m_stEmailHeaderBitmap.ShowWindow(SW_HIDE);
	m_lstVirusScan.ShowWindow(SW_HIDE);
	m_stSignatureHeaderPic.ShowWindow(SW_HIDE);
	m_contentFilterHeaderPic.ShowWindow(SW_HIDE);
	m_lstContentFilter.ShowWindow(SW_HIDE);
	m_chkAllowSignature.ShowWindow(SW_HIDE);
	m_stAllowSignatureMsg.ShowWindow(SW_HIDE);
	m_edtSigDesMsg.ShowWindow(SW_HIDE);
	m_stGrpAddRule.ShowWindow(SW_HIDE);
	m_stAddRule.ShowWindow(SW_HIDE);
	m_chkVirusScan.ShowWindow(SW_HIDE);
	m_stEnableVirusScan.ShowWindow(SW_HIDE);
	m_chkContentfilter.ShowWindow(SW_HIDE);
	m_stEnableContentFilter.ShowWindow(SW_HIDE);
	m_stAddaRuleForContent.ShowWindow(SW_HIDE);
	m_stSettingHeaderPic.ShowWindow(SW_HIDE);
	m_stVirusscan_HeaderName.ShowWindow(SW_HIDE);
	m_stVirusscan_HeaderDes.ShowWindow(SW_HIDE);
	m_stVirusSettingHeader.ShowWindow(SW_HIDE);
	m_stContentHeaderDes.ShowWindow(SW_HIDE);
	m_stContentHeaderName.ShowWindow(SW_HIDE);
	m_stContentSettingHeader.ShowWindow(SW_HIDE);
	m_stSignature_HeaderDes.ShowWindow(SW_HIDE);
	m_stSignture_HeaderName.ShowWindow(SW_HIDE);
	m_bSignature=0;	
	m_bSpamFilter=0;
	m_bContentFilter=1;
	m_bSignatureFlag=1;
	m_bVirusScan=1;
	m_bUpdate = 1;
	m_bEnableVirusPopUp=1;
	ReadRegistryEntryofEmailScan();
	m_btnEdit.EnableWindow(TRUE);
	m_btnDelete.EnableWindow(TRUE);
	m_btnOk.EnableWindow(TRUE);
	m_stForExample.ShowWindow(SW_SHOW);
	m_stSpamHeaderDes.ShowWindow(SW_SHOW);
	m_stSpamHeaderName.ShowWindow(SW_SHOW);
	m_stSpamSettingHeadername.ShowWindow(SW_SHOW);
	m_stSpamFilterHeaderPic.ShowWindow(SW_SHOW);
	m_chkSpamFilter.ShowWindow(SW_SHOW);
	m_stEnableSpamFilter.ShowWindow(SW_SHOW);
	m_lstSpamFilter.ShowWindow(SW_SHOW);
	m_stAddRuleHeaderPic.ShowWindow(SW_SHOW);
	m_stGrpBoxForRadioBtn.ShowWindow(SW_SHOW);
	m_btnOk.ShowWindow(SW_SHOW);
	m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"));
	m_btnDelete.ShowWindow(SW_SHOW);
	m_btnEdit.ShowWindow(SW_SHOW);
	m_btnApply.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG ,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnApply.SetWindowPos(&wndTop,m_rect.left +524,315,57,21,SWP_SHOWWINDOW);
	m_btnApply.EnableWindow(FALSE);
	m_edtEmailAddress.SetWindowPos(&wndTop,m_rect.left + 15,315,240,22,SWP_SHOWWINDOW);
	m_DropList.SetWindowPos(&wndTop,m_rect.left +265 ,315,50,20,SWP_SHOWWINDOW);
	m_DropList.SetCurSel(0);
	m_edtEmailAddress.SetWindowTextW(L"");
	m_edtEmailAddress.ShowWindow(SW_SHOW);
	m_DropList.ShowWindow(SW_SHOW);
	m_btnDefaultSig.ShowWindow(SW_HIDE);
	SendEmailData2Service(RELOAD_DBENTRIES, SPAMFILTER, L"", true);
	Sleep(10);
	LoadExistingSpamFilterFile();

	return true;
}



/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonContentfilter                                                     
*  Description    :	Controls Hide and show after click on content filter button on GUI 
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
**********************************************************************************************************/
bool CISpyEmailScanDlg::OnBnClickedButtonContentfilter()
{
	// TODO: Add your control notification handler code here
	// ISsue no 691 neha gharge 13/6/2014
	CISpyGUIDlg * g_ObjIspyGUIHwnd;
	g_ObjIspyGUIHwnd = (CISpyGUIDlg *) (CISpyGUIDlg*)AfxGetMainWnd();	

	if(g_ObjIspyGUIHwnd != NULL)
	{
		//Ram, Issue No: 0001216, Resolved
		if (theApp.m_dwDaysLeft == 0)
		{
			theApp.GetDaysLeft();
		}

		if(theApp.m_dwDaysLeft == 0)
		{
			if(!theApp.ShowEvaluationExpiredMsg())
			{
				theApp.GetDaysLeft();
				g_ObjIspyGUIHwnd->OnBnClickedButtonHome();
				return false;
			}
		}
	}

	//If we installed outlook and Select Email scan option, all three option get highlighted
	//Neha Gharge 12 Aug,2015
	bool bOutlookInstalled = GetExistingPathofOutlook();
	if(!bOutlookInstalled)
	{
		m_bIsPopUpDisplayed = true;
		if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_INSTALL_OUTLOOK"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) == IDOK)
		{
			m_bIsPopUpDisplayed = false;
			//issue Disable 'Email scan' ->click on any of the feature in email scan->it will give an popup 'Do you want to enable email scan'->click 'OK'->again click on the same feature then its not giving any popup.
			// resolved by lalit kumawat 18-8-2015
			if (g_ObjIspyGUIHwnd->m_pTabDialog != NULL)
			{
				g_ObjIspyGUIHwnd->m_pTabDialog->m_SelectedButton = 0;
			}
			return false;
		}
		m_bIsPopUpDisplayed = false;
	}
	else
	{
		g_ObjIspyGUIHwnd->MainDialogDisplay();
	}

	if(g_ObjIspyGUIHwnd != NULL)
	{
		/*if(theApp.m_dwDaysLeft == 0)
		{
			if(!theApp.ShowEvaluationExpiredMsg())
			{
				theApp.GetDaysLeft();
				g_ObjIspyGUIHwnd->OnBnClickedButtonHome();
				return false;
			}
		}*/

		DWORD dwEmailEnable = g_ObjIspyGUIHwnd->ReadEmailScanEntryFromRegistry();
		if(!dwEmailEnable)
		{	
			m_bIsPopUpDisplayed = true;
			if(MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_ENABLE_EMAILSCAN"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONQUESTION|MB_YESNO)==IDNO)
			{
				m_bIsPopUpDisplayed = false;
				//issue Disable 'Email scan' ->click on any of the feature in email scan->it will give an popup 'Do you want to enable email scan'->click 'OK'->again click on the same feature then its not giving any popup.
				// resolved by lalit kumawat 18-8-2015
				if (g_ObjIspyGUIHwnd->m_pTabDialog != NULL)
				{
					g_ObjIspyGUIHwnd->m_pTabDialog->m_SelectedButton = 0;
				}
				return false;
			}
			else
			{
				m_bIsPopUpDisplayed = false;
				//Issue no 1266. Even if the Email Scan feature is off, the email scan feature window remains open.
				if (g_ObjIspyGUIHwnd->m_pTabDialog != NULL)
				{
					g_ObjIspyGUIHwnd->m_pTabDialog->m_SelectedButton = EMAIL_CONTENT_FILTER_DLG;
				}
				WriteRegistryEntryOfMenuItem(L"dwEmailScan",1);
				g_ObjIspyGUIHwnd->MainDialogDisplay();
			}
		}
	}
	/*	ISSUE NO - 637 NAME - NITIN K. TIME - 10th June 2014 */
	// resolved by lalit kumawat 8-7-2015
	// issue: nevigation on Spamfilter taking long time due to duplicate list populating, this call again added after DB file loading
	//PopulateList();
	m_stSettingHeaderPic.ShowWindow(SW_HIDE);
	m_btnQuarantine.ShowWindow(SW_HIDE);
	m_btnRemove.ShowWindow(SW_HIDE);
	m_stQuarantineText.ShowWindow(SW_HIDE);
	m_stRemoveText.ShowWindow(SW_HIDE);
	m_stGrpAddRule.ShowWindow(SW_HIDE);
	m_stAddRule.ShowWindow(SW_HIDE);
	m_stEmailHeaderBitmap.ShowWindow(SW_HIDE);
	m_lstVirusScan.ShowWindow(SW_HIDE);
	m_stSpamFilterHeaderPic.ShowWindow(FALSE);	
	m_lstSpamFilter.ShowWindow(SW_HIDE);
	m_stSignatureHeaderPic.ShowWindow(SW_HIDE);
	m_chkAllowSignature.ShowWindow(SW_HIDE);
	m_stAllowSignatureMsg.ShowWindow(SW_HIDE);
	m_edtSigDesMsg.ShowWindow(SW_HIDE);
	m_chkSpamFilter.ShowWindow(SW_HIDE);
	m_stEnableSpamFilter.ShowWindow(SW_HIDE);
	m_chkVirusScan.ShowWindow(SW_HIDE);
	m_stEnableVirusScan.ShowWindow(SW_HIDE);
	m_stAddRuleHeaderPic.ShowWindow(SW_HIDE);
	m_stForExample.ShowWindow(SW_HIDE);
	m_chkAllowVirusPopUp.ShowWindow(SW_HIDE);
	m_stAllowVirusPopdlg.ShowWindow(SW_HIDE);
	m_stVirusscan_HeaderName.ShowWindow(SW_HIDE);
	m_stVirusscan_HeaderDes.ShowWindow(SW_HIDE);
	m_stVirusSettingHeader.ShowWindow(SW_HIDE);
	m_stSpamHeaderDes.ShowWindow(SW_HIDE);
	m_stSpamHeaderName.ShowWindow(SW_HIDE);
	m_stSpamSettingHeadername.ShowWindow(SW_HIDE);
	m_stSignature_HeaderDes.ShowWindow(SW_HIDE);
	m_stSignture_HeaderName.ShowWindow(SW_HIDE);
	m_bSignature=0;
	m_bUpdate = 1;
	m_bSpamFilter=1;
	m_bVirusScan=1;
	m_bSignatureFlag=1;
	m_bEnableVirusPopUp=1;
	m_bContentFilter=0;
	ReadRegistryEntryofEmailScan();
	m_btnEdit.EnableWindow(TRUE);
	m_btnDelete.EnableWindow(TRUE);
	m_btnOk.EnableWindow(TRUE);
	m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"));
	m_stContentHeaderDes.ShowWindow(SW_SHOW);
	m_stContentHeaderName.ShowWindow(SW_SHOW);
	m_stContentSettingHeader.ShowWindow(SW_SHOW);
	m_DroplistContentFilter.SetWindowPos(&wndTop,m_rect.left +12,315,140,20,SWP_SHOWWINDOW);
	m_edtEmailAddress.SetWindowPos(&wndTop,m_rect.left + 160,315,100,22,SWP_SHOWWINDOW);
	m_DropList.SetWindowPos(&wndTop,m_rect.left +265 ,315,50,20,SWP_SHOWWINDOW);
	m_btnOk.SetWindowPos(&wndTop, m_rect.left + 325 ,315,57,21,SWP_SHOWWINDOW);
	m_btnDelete.SetWindowPos(&wndTop, m_rect.left + 390 ,315,57,21,SWP_SHOWWINDOW);
	m_btnEdit.SetWindowPos(&wndTop, m_rect.left + 457 ,315,57,21,SWP_SHOWWINDOW);
	m_btnApply.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG ,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnApply.SetWindowPos(&wndTop,m_rect.left +524,315,57,21,SWP_SHOWWINDOW);
	m_btnApply.EnableWindow(FALSE);
	m_contentFilterHeaderPic.ShowWindow(SW_SHOW);
	m_lstContentFilter.ShowWindow(SW_SHOW);
	m_stAddaRuleForContent.ShowWindow(SW_SHOW);
	m_stGrpBoxForRadioBtn.ShowWindow(SW_SHOW);
	m_DroplistContentFilter.ShowWindow(SW_SHOW);
	m_edtEmailAddress.SetWindowTextW(L"");
	m_edtEmailAddress.ShowWindow(SW_SHOW);
	m_DropList.SetCurSel(0);
	m_DroplistContentFilter.SetCurSel(0);
	m_DropList.ShowWindow(SW_SHOW);
	m_chkContentfilter.ShowWindow(SW_SHOW);
	m_stEnableContentFilter.ShowWindow(SW_SHOW);
	m_btnDefaultSig.ShowWindow(SW_HIDE);
	SendEmailData2Service(RELOAD_DBENTRIES, CONTENTFILTER, L"", true);
	Sleep(10);
	LoadExistingContentFilterFile();

	return true;
}


/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonSignature                                                     
*  Description    :	Controls Hide and show after click on signature button on GUI 
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
**********************************************************************************************************/
bool CISpyEmailScanDlg::OnBnClickedButtonSignature()
{
	// TODO: Add your control notification handler code here
	// ISsue no 691 neha gharge 13/6/2014
	CISpyGUIDlg * g_ObjIspyGUIHwnd;
	g_ObjIspyGUIHwnd = (CISpyGUIDlg *) (CISpyGUIDlg*)AfxGetMainWnd();	

	if(g_ObjIspyGUIHwnd != NULL)
	{
		//Ram, Issue No: 0001216, Resolved
		if (theApp.m_dwDaysLeft == 0)
		{
			theApp.GetDaysLeft();
		}

		if(theApp.m_dwDaysLeft == 0)
		{
			if(!theApp.ShowEvaluationExpiredMsg())
			{
				theApp.GetDaysLeft();
				g_ObjIspyGUIHwnd->OnBnClickedButtonHome();
				return false;
			}
		}
	}

	//If we installed outlook and Select Email scan option, all three option get highlighted
	//Neha Gharge 12 Aug,2015
	bool bOutlookInstalled = GetExistingPathofOutlook();
	if(!bOutlookInstalled)
	{
		m_bIsPopUpDisplayed = true;
		if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_INSTALL_OUTLOOK"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) == IDOK)
		{
			m_bIsPopUpDisplayed = false;
			//issue Disable 'Email scan' ->click on any of the feature in email scan->it will give an popup 'Do you want to enable email scan'->click 'OK'->again click on the same feature then its not giving any popup.
			// resolved by lalit kumawat 18-8-2015
			if (g_ObjIspyGUIHwnd->m_pTabDialog != NULL)
			{
				g_ObjIspyGUIHwnd->m_pTabDialog->m_SelectedButton = 0;
			}
			return false;
		}
		m_bIsPopUpDisplayed = false;
	}
	else
	{
		g_ObjIspyGUIHwnd->MainDialogDisplay();
	}

	if(g_ObjIspyGUIHwnd != NULL)
	{
		/*if(theApp.m_dwDaysLeft == 0)
		{
			if(!theApp.ShowEvaluationExpiredMsg())
			{
				theApp.GetDaysLeft();
				g_ObjIspyGUIHwnd->OnBnClickedButtonHome();
				return false;
			}
		}*/

		DWORD dwEmailEnable = g_ObjIspyGUIHwnd->ReadEmailScanEntryFromRegistry();
		if(!dwEmailEnable)
		{	
			m_bIsPopUpDisplayed = true;
			if(MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_ENABLE_EMAILSCAN"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONQUESTION|MB_YESNO)==IDNO)
			{
				m_bIsPopUpDisplayed = false;
				//issue Disable 'Email scan' ->click on any of the feature in email scan->it will give an popup 'Do you want to enable email scan'->click 'OK'->again click on the same feature then its not giving any popup.
				// resolved by lalit kumawat 18-8-2015

				if (g_ObjIspyGUIHwnd->m_pTabDialog != NULL)
				{
					g_ObjIspyGUIHwnd->m_pTabDialog->m_SelectedButton = 0;
				}
				return false;
			}
			else
			{
				m_bIsPopUpDisplayed = false;
				WriteRegistryEntryOfMenuItem(L"dwEmailScan",1);
				g_ObjIspyGUIHwnd->MainDialogDisplay();
			}
		}
	}
	m_stAddRuleHeaderPic.ShowWindow(SW_HIDE);
	m_stGrpAddRule.ShowWindow(SW_HIDE);
	m_DroplistContentFilter.ShowWindow(SW_HIDE);
	m_DropList.ShowWindow(SW_HIDE);
	m_stAddRule.ShowWindow(SW_HIDE);
	m_stSpamFilterHeaderPic.ShowWindow(SW_HIDE);
	m_lstSpamFilter.ShowWindow(SW_HIDE);
	m_stEmailHeaderBitmap.ShowWindow(SW_HIDE);
	m_lstVirusScan.ShowWindow(SW_HIDE);
	m_contentFilterHeaderPic.ShowWindow(SW_HIDE);
	m_lstContentFilter.ShowWindow(SW_HIDE);
	m_btnQuarantine.ShowWindow(SW_HIDE);
	m_btnRemove.ShowWindow(SW_HIDE);
	m_stQuarantineText.ShowWindow(SW_HIDE);
	m_stRemoveText.ShowWindow(SW_HIDE);
	m_stSettingHeaderPic.ShowWindow(SW_HIDE);
	m_stGrpBoxForRadioBtn.ShowWindow(SW_HIDE);
	m_edtEmailAddress.ShowWindow(SW_HIDE);
	m_btnEdit.ShowWindow(SW_HIDE);
	m_btnOk.ShowWindow(SW_HIDE);
	m_btnDelete.ShowWindow(SW_HIDE);
	m_chkSpamFilter.ShowWindow(SW_HIDE);
	m_stEnableSpamFilter.ShowWindow(SW_HIDE);
	m_chkVirusScan.ShowWindow(SW_HIDE);
	m_stEnableVirusScan.ShowWindow(SW_HIDE);
	m_chkContentfilter.ShowWindow(SW_HIDE);
	m_stEnableContentFilter.ShowWindow(SW_HIDE);
	m_stAddaRuleForContent.ShowWindow(SW_HIDE);
	m_stForExample.ShowWindow(SW_HIDE);
	m_chkAllowVirusPopUp.ShowWindow(SW_HIDE);
	m_stAllowVirusPopdlg.ShowWindow(SW_HIDE);
	m_stVirusscan_HeaderName.ShowWindow(SW_HIDE);
	m_stVirusscan_HeaderDes.ShowWindow(SW_HIDE);
	m_stVirusSettingHeader.ShowWindow(SW_HIDE);
	m_stSpamHeaderDes.ShowWindow(SW_HIDE);
	m_stSpamHeaderName.ShowWindow(SW_HIDE);
	m_stSpamSettingHeadername.ShowWindow(SW_HIDE);
	m_stContentHeaderDes.ShowWindow(SW_HIDE);
	m_stContentHeaderName.ShowWindow(SW_HIDE);
	m_stContentSettingHeader.ShowWindow(SW_HIDE);
	m_bSignature=1;
	m_bVirusScan=1;
	m_bSpamFilter=1;
	m_bContentFilter=1;
	m_bEnableVirusPopUp=1;
	m_bUpdate = 1;
	m_bSignatureFlag=0;
	ReadRegistryEntryofEmailScan();
	m_stSignatureHeaderPic.ShowWindow(SW_SHOW);
	m_stSignature_HeaderDes.ShowWindow(SW_SHOW);
	m_stSignture_HeaderName.ShowWindow(SW_SHOW);
	m_chkAllowSignature.ShowWindow(SW_SHOW);
	m_stAllowSignatureMsg.ShowWindow(SW_SHOW);
	m_edtSigDesMsg.ShowWindow(SW_SHOW);
	m_btnApply.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0);
	m_btnApply.SetWindowPos(&wndTop,m_rect.left +530,333 ,57,21,SWP_NOREDRAW | SWP_NOZORDER);
	m_btnApply.EnableWindow(FALSE);
	m_btnDefaultSig.ShowWindow(true);
	m_btnDefaultSig.EnableWindow(TRUE);

	SendEmailData2Service(RELOAD_DBENTRIES, SIGNATURE, L"", true);
	Sleep(10);
	LoadExistingSignatureFile();
	return true;
}



/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedOk                                                     
*  Description    :	Controls two functionality add and update button. Add the rule,email addr and action from user on
					GUI as well as in db file.
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
**********************************************************************************************************/
void CISpyEmailScanDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	TCHAR szEmailAddr[0x512] = {0};
	bool ValidEmail,Validextension;
	CString szChkWindowText;
	CString csEmailAddr;
	CString csType,csRuleType;
	DWORD i=3,j=3;
	DWORD n;
	m_btnOk.GetWindowTextW(szChkWindowText);
	m_bUpdate=1;
	if(szChkWindowText == theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE"))
	{
		m_bUpdate=0;
		m_bDeleteFlag=1;
		HideORShowButton(false,false,false,true);

	}
	else
	{
		m_bUpdate=1;
		m_bDeleteFlag=1;
		HideORShowButton(false,false,false,true);

	}

	if(!m_bSpamFilter)
	{
	
		if(szChkWindowText == theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD") ||szChkWindowText == theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE"))
		{	
			AddLogEntry(L">>> EmailScanDlg: Add or update input entry", 0, 0, true, ZEROLEVEL);
			i=0;
			m_edtEmailAddress.GetWindowText(szEmailAddr,512);
			csEmailAddr.Format( TEXT("%s"), szEmailAddr) ;
			/*	ISSUE NO - 704 NAME - NITIN K. TIME - 15th June 2014 */
			//Issue no. 1277 if any rule or email address are same only space difference is there.the it will show rule is already exist
			csEmailAddr.Trim();
			csEmailAddr.MakeLower();
			i = m_DropList.GetCurSel();
			m_bDeleteFlag=1;
			ValidEmail=isEmail(csEmailAddr);
			if(ValidEmail!=1)
			{
				AddLogEntry(L"### EmailScanDlg: Invalid Email Address", 0, 0, true, SECONDLEVEL);
				HideORShowButton(true,true,true,false);
				
				m_bIsPopUpDisplayed = true;
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_ENTER_EMAIL_ADDR"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
				m_bIsPopUpDisplayed = false;
				return;
			}
				
			if( (szEmailAddr[0]) && (i < 2))
			{
				switch(i)
				{
					case 0:	
						csType = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE");
						break;
					case 1:
						csType = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE");
						break;
					default:
						break;
				}
			}
			else
			{
				m_bIsPopUpDisplayed = true;
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_ENTER_EMAILID_TYPE"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
				m_bIsPopUpDisplayed = false;
				return;
			}
				
		}
		
		else
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_ENTER_EMAIL_ADDR"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
			m_bIsPopUpDisplayed = false;
			return;
		}
		if(szChkWindowText==theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"))
		{
			for (n = 0; n < static_cast<DWORD>(m_lstSpamFilter.GetItemCount()); n++)
			{
				_tcscpy_s(szEmailAddr, csEmailAddr.MakeLower());
				if(szEmailAddr==m_lstSpamFilter.GetItemText(n,1))
				{


					m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"));
					m_edtEmailAddress.SetWindowTextW(L"");

					HideORShowButton(true,true,true,false);
					g_DelEmailAddr =L"";
					g_DelRuleType = L"";
					g_DelType=L"";
					
					m_bIsPopUpDisplayed = true;
					MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_EMAILID_ALREADY_EXIST"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
					m_bIsPopUpDisplayed = false;

					return;
				}
			}
		}
		if(szChkWindowText==theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE"))
		{
			_tcscpy_s(szEmailAddr, csEmailAddr.MakeLower());
			for (n = 0; n < static_cast<DWORD>(m_lstSpamFilter.GetItemCount()); n++)
			{
				if(csEmailAddr == m_lstSpamFilter.GetItemText(n,1))
				{
					if(n == m_dwnitem)
					{
						//Neha Gharge 27 July,2015
						//if any rule is already blocked , It should asked for permission to make it allow
						// issue - asking conformation message box in case of email id update or content update by keeping rules as previous
						// updated by lalit kumawat 7-31-2015
						if (m_lstSpamFilter.GetItemText(n, 0) == theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE") && csType == theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE"))
						{
							m_bIsPopUpDisplayed = true;
							if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_SCAN_SPAM_RULE_ALREADY_BLOCK"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_YESNO | MB_ICONQUESTION) == IDNO)
							{
								m_bIsPopUpDisplayed = false;
								m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"));
								m_edtEmailAddress.SetWindowTextW(L"");
								HideORShowButton(true, true, true, false);
								return;
							}
							m_bIsPopUpDisplayed = false;
						}
						//Issue no 1272 : The rule is already block then we make it allow , It should show message of confirmation
						if (m_lstSpamFilter.GetItemText(n, 0) == theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE") && csType == theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"))
						{
							m_bIsPopUpDisplayed = true;
							if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_SCAN_SPAM_RULE_ALREADY_ALLOW"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_YESNO | MB_ICONQUESTION) == IDNO)
							{
								m_bIsPopUpDisplayed = false;
								m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"));
								m_edtEmailAddress.SetWindowTextW(L"");
								HideORShowButton(true, true, true, false);
								return;
							}
						}
						m_bIsPopUpDisplayed = false;
						if(csType != m_lstSpamFilter.GetItemText(n,0))
						{
							m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"));
							m_edtEmailAddress.SetWindowTextW(L"");

							HideORShowButton(false,false,false,true);
							break;
						}
					}
					
					m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"));
					m_edtEmailAddress.SetWindowTextW(L"");

					HideORShowButton(true,true,true,false);
					g_DelEmailAddr =L"";
					g_DelRuleType = L"";
					g_DelType=L"";
					
					m_bIsPopUpDisplayed = true;
					MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_EMAILID_ALREADY_EXIST"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
					m_bIsPopUpDisplayed = false;

					return;
				}
			}
		}

		InsertItem(csEmailAddr,csType);
		m_edtEmailAddress.SetWindowTextW(L"");
		m_DropList.SetCurSel(0);
	
		if(szChkWindowText==theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE"))
		{
			m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"));
			
		}

	}
// for Content Filter	
	if(!m_bContentFilter)
	{
		m_btnOk.GetWindowTextW(szChkWindowText);
		
		if(szChkWindowText==theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD")||theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE"))
		{
			AddLogEntry(L">>> EmailScanDlg: Add or Update input entries", 0, 0, true, ZEROLEVEL);
			m_edtEmailAddress.GetWindowText(szEmailAddr,512);
			csEmailAddr.Format( TEXT("%s"), szEmailAddr) ;
			
			m_bDeleteFlag = 1;
			j = 0;
			j = m_DroplistContentFilter.GetCurSel();
			if(j == 0)
			{
				Validextension = isExtension(csEmailAddr);
				if(Validextension != 1)
				{
					AddLogEntry(L"### EmailScanDlg: Invalid extension", 0, 0, true, SECONDLEVEL);
					HideORShowButton(true,true,true,false);
	
					m_bIsPopUpDisplayed = true;
					MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_VALID_EXTENSION"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
					m_bIsPopUpDisplayed = false;
					return;
				}
			}
			//Issue no 1273 If only space is given as a rule. it should not allow.
			if (j == 1 || j == 2)
			{
				csEmailAddr.Trim();
				if (csEmailAddr == L"")
				{
					AddLogEntry(L"### EmailScanDlg: Invalid content for message and subject content", 0, 0, true, FIRSTLEVEL);
					HideORShowButton(true, true, true, false);

					m_bIsPopUpDisplayed = true;
					MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_VALID_RULE_CONTENT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
					m_bIsPopUpDisplayed = false;

					return;
				}
			}
			i = 0;
			i = m_DropList.GetCurSel();
			
			if( (szEmailAddr[0]) && (i < 2)&&(j<3))
			{
				switch(i)
				{
					case 0:	
						csType = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE");
						break;
					case 1:
						csType = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE");
						break;
					default:
						break;
				}
				switch(j)
				{
					case 0:	
						csRuleType = theApp.m_objwardwizLangManager.GetString(L"IDS_ATTACHMENT_EXT_IS_CONTENT");
						break;
					case 1:
						csRuleType = theApp.m_objwardwizLangManager.GetString(L"IDS_MESSAGE_CONTAIN_CONTENT");
						break;
					case 2:
						csRuleType = theApp.m_objwardwizLangManager.GetString(L"IDS_SUBJECT_CONTAIN_CONTECT");
						break;
					default:
						break;
				}
			}
			else
			{
				HideORShowButton(true,true,true,false);
				
				m_bIsPopUpDisplayed = true;
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_ENTER_RULE_RULETYPE_ACTION"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
				m_bIsPopUpDisplayed = false;

				return;
			}
		}
		else
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_ENTER_RULE_RULETYPE_ACTION"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
			m_bIsPopUpDisplayed = false;

			return;
		}
		if(szChkWindowText==theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"))
		{
			for (n = 0; n < static_cast<DWORD>(m_lstContentFilter.GetItemCount()); n++)
			{
				_tcscpy_s(szEmailAddr, csEmailAddr.MakeLower());

				if (szEmailAddr == m_lstContentFilter.GetItemText(n, 2) && csRuleType == m_lstContentFilter.GetItemText(n, 1))
				{


					m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"));
					m_edtEmailAddress.SetWindowTextW(L"");

					HideORShowButton(true,true,true,false);
					g_DelEmailAddr =L"";
					g_DelRuleType = L"";
					g_DelType=L"";
					
					m_bIsPopUpDisplayed = true;
					MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_CONTENT_RULE_ALREADY_EXIST"), theApp.m_objwardwizLangManager.GetString(L"_IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
					m_bIsPopUpDisplayed = false;

					return;
				}
			}
		}
		if(szChkWindowText==theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE"))
		{
			for (n = 0; n < static_cast<DWORD>(m_lstContentFilter.GetItemCount()) + 1; n++)
			{
				_tcscpy_s(szEmailAddr,csEmailAddr.MakeLower());
				if(szEmailAddr==m_lstContentFilter.GetItemText(n,2) && csRuleType==m_lstContentFilter.GetItemText(n,1))
				{
					if(n == m_dwnitem)
					{
						//Neha Gharge 27 July,2015
						//if any rule is already blocked , It should asked for permission to make it allow
						// issue - asking conformation message box in case of email id update or content update by keeping rules as previous
						// updated by lalit kumawat 7-31-2015
						if (m_lstContentFilter.GetItemText(n, 0) == theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE") && csType == theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE"))
						{
							m_bIsPopUpDisplayed = true;
							if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_SCAN_CONTENT_RULE_ALREADY_BLOCK"), theApp.m_objwardwizLangManager.GetString(L"_IDS_PRODUCT_NAME"), MB_YESNO | MB_ICONQUESTION) == IDNO)
							{
								m_bIsPopUpDisplayed = false;
								m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"));
								m_edtEmailAddress.SetWindowTextW(L"");
								HideORShowButton(true, true, true, false);
								return;
							}
							m_bIsPopUpDisplayed = false;
						}
						//Issue no 1272 : The rule is already block then we make it allow , It should show message of confirmation
						if (m_lstContentFilter.GetItemText(n, 0) == theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE") && csType == theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"))
						{
							m_bIsPopUpDisplayed = true;
							if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_SCAN_CONTENT_RULE_ALREADY_ALLOW"), theApp.m_objwardwizLangManager.GetString(L"_IDS_PRODUCT_NAME"), MB_YESNO | MB_ICONQUESTION) == IDNO)
							{
								m_bIsPopUpDisplayed = false;
								m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"));
								m_edtEmailAddress.SetWindowTextW(L"");
								HideORShowButton(true, true, true, false);
								return;
							}
						}
						m_bIsPopUpDisplayed = false;
						if(csType != m_lstContentFilter.GetItemText(n,0))
						{
							m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"));
							m_edtEmailAddress.SetWindowTextW(L"");
							HideORShowButton(false,false,false,true);

							break;
						}
					}
			
					m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"));
					m_edtEmailAddress.SetWindowTextW(L"");
					HideORShowButton(true,true,true,false);

					g_DelEmailAddr =L"";
					g_DelRuleType = L"";
					g_DelType=L"";
					
					m_bIsPopUpDisplayed = true;
					MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_CONTENT_RULE_ALREADY_EXIST"), theApp.m_objwardwizLangManager.GetString(L"_IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
					m_bIsPopUpDisplayed = false;

					return;
				}
			}
		}
		InsertItem(csEmailAddr,csType,csRuleType);
		m_DroplistContentFilter.SetCurSel(3);
		m_edtEmailAddress.SetWindowTextW(L"");
		m_DropList.SetCurSel(0);
		m_DroplistContentFilter.SetCurSel(3);
	
		if(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE"))
		{
			m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"));
		}

	}
}


/**********************************************************************************************************                     
*  Function Name  :	InsertItem                                                     
*  Description    :	Insert user entry in lstcontrol.
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
**********************************************************************************************************/
void CISpyEmailScanDlg::InsertItem(CString strEmailAddr, CString strType)
{

	CString csAddEntry = L"";
	TCHAR * szAddEntry = L"";
	LVITEM lvItem;
	
	int nItem=0;
	if(!m_bUpdate)
	{
		nItem=m_dwnitem;
		m_lstSpamFilter.DeleteItem(m_dwnitem);
	}
	int imgNbr = 0;

	lvItem.mask = LVIF_IMAGE;
	lvItem.iItem = nItem;
	lvItem.iSubItem = 0;
	
	if(strType==theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE"))
	{
		lvItem.iImage = 0%2;
	}
	if(strType==theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"))
	{
		lvItem.iImage = 1%2;
	}
	
	m_lstSpamFilter.InsertItem(&lvItem);
	CString cstmp;

	m_lstSpamFilter.SetItemText(nItem, 0, strType);
	m_lstSpamFilter.SetItemText(nItem, 1, strEmailAddr);
	m_lstSpamFilter.SetCheck(nItem, TRUE);

	//Issue no 1269 and 1270 User can add # as rule. No special symbol is used to tonize word
	//csAddEntry.Format(L"%s#%s#\n",strEmailAddr,strType);
	if(!SendEmailData2Service(ADD_EMAIL_ENTRY , SPAMFILTER ,strEmailAddr,strType,L"",true))
	{
		AddLogEntry(L"### EmailScanDlg: ADD_EMAIL_ENTRY in SendEmailData2Service", 0, 0, true, SECONDLEVEL);
	}
}


/**********************************************************************************************************                     
*  Function Name  :	InsertItem                                                     
*  Description    :	Insert user entry in lstcontrol.
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
**********************************************************************************************************/
void CISpyEmailScanDlg::InsertItem(CString strEmailAddr, CString strType,CString strRuleType)
{
	CString csAddEntry =L"";
	LVITEM lvItem;
	int nItem=0;
	if(!m_bUpdate)
	{
		
		nItem=m_dwnitem;
		m_lstContentFilter.DeleteItem(m_dwnitem);
	}
	int imgNbr = 0;

	lvItem.mask = LVIF_IMAGE;
	lvItem.iItem = nItem;
	lvItem.iSubItem = 0;
	
	
	if(!m_bContentFilter)
	{
		
		if(strType==theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE"))
		{
			lvItem.iImage = 0%2;
		}
		if(strType==theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"))
		{
			lvItem.iImage = 1%2;
		}
		
	
		m_lstContentFilter.InsertItem(&lvItem);
	
		CString cstmp;
		strEmailAddr = strEmailAddr.MakeLower();
		m_lstContentFilter.SetItemText(nItem, 0, strType);
		m_lstContentFilter.SetItemText(nItem, 1, strRuleType);
		m_lstContentFilter.SetItemText(nItem, 2, strEmailAddr);
		m_lstContentFilter.SetCheck(nItem, TRUE);
		//Issue no 1269 and 1270 User can add # as rule. No special symbol is used to tonize word
		//csAddEntry.Format( L"%s#%s#%s#",(LPTSTR)strEmailAddr.GetBuffer(),(LPTSTR)strType.GetBuffer(),(LPTSTR)strRuleType.GetBuffer());

		if(!SendEmailData2Service(ADD_EMAIL_ENTRY , CONTENTFILTER,strEmailAddr,strType,strRuleType,true))
		{
			AddLogEntry(L"### EmailScanDlg: ADD_EMAIL_ENTRY in SendEmailData2Service", 0, 0, true, SECONDLEVEL);
		}
	}

}


/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonApply                                                     
*  Description    :	It stores final setting on db,GUI and registry. And show message box whether settings are saved or not
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
**********************************************************************************************************/

void CISpyEmailScanDlg::OnBnClickedButtonApply()
{
	//Apply here the registry setting
	AppyRegistrySetting();

	static CString csFilePath;
	bool SettingAfterUpdate=1;

	BOOL Ret = FALSE;
	HideORShowButton(true,true,true,false);
	m_bReloadSetting=0;

	if(!m_bSpamFilter)
	{
		Ret = SendEmailData2Service(SAVE_EMAIL_ENTRIES ,SPAMFILTER,L"", true);
		if(Ret==1)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SETTING_SAVED_SUCCESSFULLY_SPAM"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONINFORMATION|MB_OK);
			m_bIsPopUpDisplayed = false;
		}
	}

	if(!m_bContentFilter)
	{
		Ret = SendEmailData2Service(SAVE_EMAIL_ENTRIES ,CONTENTFILTER,L"", true);
		if(Ret==1)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MDG_SETTING_SAVED_SUCCESSFULLY_CONTENT"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONINFORMATION|MB_OK);
			m_bIsPopUpDisplayed = false;
		}
	}

	if(!m_bSignatureFlag)
	{
		CString csSignatureEntry =L"";
		m_edtSigDesMsg.GetWindowText(csSignatureEntry);
		m_edtSigDesMsg.EnableWindow(true);
		//
		if(csSignatureEntry==L"")
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_ENTER_SIGNATURE"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONINFORMATION|MB_OK);
			m_bIsPopUpDisplayed = false;
			return;
		}
		//Issue no 1269 and 1270 User can add # as rule. No special symbol is used to tonize word
		SendEmailData2Service(ADD_EMAIL_ENTRY , SIGNATURE , csSignatureEntry,L"",L"", true);

		Sleep(500);

		Ret = SendEmailData2Service(SAVE_EMAIL_ENTRIES ,SIGNATURE,L"", true);
		// Rajil Yadav Issue No.674 12/06/2014
		int iCheckStatus = m_chkAllowSignature.GetCheck();
		if(Ret == 1 && !iCheckStatus)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SIGNATURE_MESSAGE"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_OK | MB_ICONEXCLAMATION);
			m_bIsPopUpDisplayed = false;

			m_btnDefaultSig.EnableWindow(true);
			m_bDefaultSigSelected = false;
			return;
		}
	

		if(Ret == 1 && m_bDefaultSigSelected)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SETTING_SAVED_SUCESSFULLY_DEF_SIGNATURE"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONINFORMATION|MB_OK);
			m_bIsPopUpDisplayed = false;
		}
		else if(Ret == 1)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SETTING_SAVED_SUCESSFULLY_SIGNATURE"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION | MB_OK);
			m_bIsPopUpDisplayed = false;
		}
		m_btnDefaultSig.EnableWindow(true);
		m_bDefaultSigSelected = false;
	}

	if(!m_bVirusScan)
	{
		if(!m_bEnableVirusPopUp)
		{
			m_bVirusScan=1;
			m_bEnableRemove=1;
			m_bEnableQuarantine = 1;
			int iCheck = m_chkAllowVirusPopUp.GetCheck();
			if(!iCheck)
			{ 
				m_dwEnableVirusPopUp = 0;
				WriteRegistryEntryofEmailScan(m_dwEnableVirusPopUp);
			}
			else
			{
				m_dwEnableVirusPopUp = 1;
				WriteRegistryEntryofEmailScan(m_dwEnableVirusPopUp);
			}
		}
		
		int iQuarantine = m_btnQuarantine.GetCheck();
		int iRemove = m_btnRemove.GetCheck();
		
		if(iQuarantine)
		{
			m_bVirusScan=1;
			m_bEnableVirusPopUp=1;
			m_bEnableQuarantine=0;
			m_bEnableRemove=1;
			m_dwAttachScanAction=0;
			WriteRegistryEntryofEmailScan(m_dwAttachScanAction);
		}

		if(iRemove)
		{
			m_bVirusScan=1;
			m_bEnableVirusPopUp=1;
			m_bEnableRemove=0;
			m_bEnableQuarantine=1;
			m_dwAttachScanAction=1;
			WriteRegistryEntryofEmailScan(m_dwAttachScanAction);
		}
		Ret=1;

		m_bVirusScan=0;

		if(Ret==1)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SETTING_SAVED_SUCESSFULLY_VIRUSSCAN"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONINFORMATION|MB_OK);
			m_bIsPopUpDisplayed = false;
		}
	}
	
	if((!m_bReloadSetting)&&(Ret==1))
	{
		//Issue : Need to restart outllook after applying any change on UI.
		//Neha Gharge 25/6/2015 
		m_dwReloadSettings=1;
		WriteRegistryEntryofEmailScan(m_dwReloadSettings);
		theApp.SendEmailPluginChange2Service(RELOAD_EMAIL_DB_SETTINGS);
		m_bReloadSetting=1;
	}

}

/**********************************************************************************************************                     
*  Function Name  :	OnNcHitTest                                                     
*  Description    :	The framework calls this member function for the CWnd object that contains the cursor (or the CWnd object that used the SetCapture member function to capture the mouse input) every time the mouse is moved.
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
**********************************************************************************************************/

LRESULT CISpyEmailScanDlg::OnNcHitTest(CPoint point)
{
	return HTCLIENT;
}


/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedRadioQuarantine                                                     
*  Description    :	It stored repair action setting into registry.
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/

void CISpyEmailScanDlg::OnBnClickedRadioQuarantine()
{
	// TODO: Add your control notification handler code here
	m_btnQuarantine.SetCheck(TRUE);
	m_btnRemove.SetCheck(FALSE);
	m_bEnableQuarantine=0;
	m_btnApply.EnableWindow(TRUE);
		
	
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedRadioQuarantine                                                     
*  Description    :	It stored quarantine action setting into registry.
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::OnBnClickedRadioRemove()
{
	// TODO: Add your control notification handler code here
	m_btnQuarantine.SetCheck(FALSE);
	m_btnRemove.SetCheck(TRUE);
	m_bEnableRemove=0;
	m_btnApply.EnableWindow(TRUE);
	//m_dwAttachScanAction=1;
}

//void CISpyEmailScanDlg::OnBnClickedButtonBack()
//{
//	// TODO: Add your control notification handler code here
//	this->ShowHideChildEmailScan();
//	this->ShowWindow(SW_HIDE);
//	CISpyGUIDlg *pObjMainUI = reinterpret_cast<CISpyGUIDlg*>(this->GetParent());
//	pObjMainUI->ShowHideMainPageControls(true);
//}


/**********************************************************************************************************                     
*  Function Name  :	ShowHideChildEmailScan                                                     
*  Description    :	on back show hide controls acording to requirements
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::ShowHideChildEmailScan()
{
	m_DroplistContentFilter.ShowWindow(SW_HIDE);
	m_stSpamFilterHeaderPic.ShowWindow(SW_HIDE);
	m_stSignatureHeaderPic.ShowWindow(SW_HIDE);
	m_contentFilterHeaderPic.ShowWindow(SW_HIDE);
	m_lstContentFilter.ShowWindow(SW_HIDE);
	m_lstSpamFilter.ShowWindow(SW_HIDE);
	m_edtSigDesMsg.ShowWindow(SW_HIDE);
	m_stGrpAddRule.ShowWindow(SW_HIDE);
	m_stAddRule.ShowWindow(SW_HIDE);
	m_stForExample.ShowWindow(SW_HIDE);
	m_btnOk.ShowWindow(SW_HIDE);
	m_stAddRuleHeaderPic.ShowWindow(SW_HIDE);
	m_edtEmailAddress.SetWindowTextW(L"");
	m_edtEmailAddress.ShowWindow(SW_HIDE);
	m_DropList.SetCurSel(0);
	m_DroplistContentFilter.SetCurSel(3);
	m_DropList.ShowWindow(SW_HIDE);
	m_btnEdit.ShowWindow(SW_HIDE);
	m_btnDelete.ShowWindow(SW_HIDE);
	m_chkSpamFilter.ShowWindow(SW_HIDE);
	m_stEnableSpamFilter.ShowWindow(SW_HIDE);
	m_chkContentfilter.ShowWindow(SW_HIDE);
	m_stEnableContentFilter.ShowWindow(SW_HIDE);
	m_stAddaRuleForContent.ShowWindow(SW_HIDE);
	m_stAllowSignatureMsg.ShowWindow(SW_HIDE);
	m_chkAllowSignature.ShowWindow(SW_HIDE);
	m_bSignature=0;
	//m_bSpamFilter=1;
	//m_bContentFilter=1;
	//m_bSignatureFlag=1;
	m_bEditFlag=1;
	m_bDeleteFlag=1;
	//m_bEnableVirusPopUp=1;
	m_bVirusScan=0;
	m_objSpamFilterdb.bWithoutEdit=1;
	m_bUpdate = 1;
	ReadRegistryEntryofEmailScan();
	m_btnApply.SetWindowPos(&wndTop,m_rect.left +650,320,57,21,SWP_SHOWWINDOW|SWP_NOZORDER);
	m_btnApply.EnableWindow(FALSE);
	//m_stEmailHeaderBitmap.ShowWindow(SW_SHOW);
	m_lstVirusScan.ShowWindow(SW_SHOW);
	m_stSettingHeaderPic.ShowWindow(SW_HIDE);
	m_stQuarantineText.ShowWindow(SW_HIDE);
	m_stRemoveText.ShowWindow(SW_HIDE);
	m_btnQuarantine.ShowWindow(SW_HIDE);
	m_btnRemove.ShowWindow(SW_HIDE);
	//m_btnVirusScan.ShowWindow(SW_SHOW);
	//m_btnSpamFilter.ShowWindow(SW_SHOW);
	//m_btnContentFilter.ShowWindow(SW_SHOW);
	//m_btnSignature.ShowWindow(SW_SHOW);
	m_stGrpBoxForRadioBtn.ShowWindow(SW_HIDE);
	m_chkVirusScan.ShowWindow(SW_SHOW);
	m_stEnableVirusScan.ShowWindow(SW_SHOW);
	m_chkAllowVirusPopUp.ShowWindow(SW_HIDE);
	m_stAllowVirusPopdlg.ShowWindow(SW_HIDE);
	m_btnDefaultSig.ShowWindow(SW_HIDE);
	LoadExistingVirusScanFile();
		
}

/**********************************************************************************************************                     
*  Function Name  :	OnSetCursor                                                     
*  Description    :	The framework calls this member function if mouse input is not captured and the mouse causes cursor movement within the CWnd object.
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/

BOOL CISpyEmailScanDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	/*****************ISSUE NO -113 Neha Gharge 22/5/14 ************************************************/
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if( 
		iCtrlID == IDOK	                 ||
		iCtrlID == IDC_BUTTON_EDIT	     ||
		iCtrlID == IDC_BUTTON_DELETE     ||
		iCtrlID == IDC_BUTTON_APPLY		 ||
		iCtrlID == IDC_BTN_DEFAULTSIG
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


/**********************************************************************************************************                     
*  Function Name  :	OnPaint                                                     
*  Description    :	The framework calls this member function when Windows or an application makes a request to repaint a portion of an application's window.
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CRect rc;
	if(!m_bSignature && m_bVirusScan)
	{
		CWnd * pW = this->GetDlgItem(IDC_STATIC_GROUPBOX);
		pW->GetClientRect(&rc);
		pW->ClientToScreen(&rc);
		this->ScreenToClient(&rc);
		dc.FillSolidRect(&rc,RGB(251,252,254));
	}
	CJpegDialog::OnPaint();


}


/**********************************************************************************************************                     
*  Function Name  :	StoredataContentToFile                                                     
*  Description    :	Stores the data into db file.
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
BOOL CISpyEmailScanDlg::StoredataContentToFile(CString csPathName)
{
	m_bsuccess=0;
	AddLogEntry(L">>> Storing data into DB files", 0, 0, true, FIRSTLEVEL);
	CFile wFile(csPathName, CFile::modeCreate | CFile::modeWrite);
 
	// Create a storing archive
	CArchive arStore(&wFile,CArchive::store);

	if(!m_bSpamFilter)
	{
		m_objSpamFilterdb.Serialize(arStore);
	}
	if(!m_bContentFilter)
	{
		m_objContentFilterdb.Serialize(arStore);
	}
	if(!m_bSignatureFlag)
	{
		m_objSignaturedb.Serialize(arStore);
	}
	if(!m_bVirusScan)
	{
		m_objVirusScandb.Serialize(arStore);
	}
	// Close the storing archive
	arStore.Close();
	wFile.Close();
 
	PopulateList();
	if(!m_bsuccess)
	{
		return true;
	}
	return false;
}


/**********************************************************************************************************                     
*  Function Name  :	LoadDataContentFromFile                                                     
*  Description    :	Loads the data into db file.
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::LoadDataContentFromFile(CString csPathName)
{
	if( !PathFileExists( csPathName ) )
		return;
	AddLogEntry(L">>> Loading data into DB files", 0, 0, true, FIRSTLEVEL);
	CFile rFile(csPathName, CFile::modeRead);
	 
	// Create a loading archive
	CArchive arLoad(&rFile, CArchive::load);

	if(!m_bSpamFilter)
	{
		m_objSpamFilterdb.Serialize(arLoad);
	}
	if(!m_bContentFilter)
	{
		m_objContentFilterdb.Serialize(arLoad);
	}
	if(!m_bSignatureFlag)
	{
		m_objSignaturedb.Serialize(arLoad);
	}
	if(!m_bVirusScan)
	{
		m_objVirusScandb.Serialize(arLoad);
	}
	
	// Close the loading archive
	arLoad.Close();
	rFile.Close();

	PopulateList();
}


/**********************************************************************************************************                     
*  Function Name  :	PopulateList                                                     
*  Description    :	Show the list according to db file
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::PopulateList()
{
	int nCurrentItem;
	if(!m_bSpamFilter)
	{

		// delete all current members
		m_lstSpamFilter.DeleteAllItems();


		// get a reference to the contacts list
		const ContactList& contacts = m_objSpamFilterdb.GetContacts();

		// iterate over all contacts add add them to the list

		POSITION pos = contacts.GetHeadPosition();
		
		while(pos != NULL)
		{
			nCurrentItem = 0;
			const CIspyList contact = contacts.GetNext(pos);
			LVITEM lvi;

			//	// Insert the first item
			lvi.mask =  LVIF_IMAGE | LVIF_TEXT;
			lvi.iItem = nCurrentItem;
			lvi.iSubItem = 0;
			lvi.pszText =(LPTSTR)(LPCTSTR)(contact.GetSecondEntry());
			if(contact.GetSecondEntry()==theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE"))
			{
				lvi.iImage =0%2;		// There are 8 images in the image list
			}
			if(contact.GetSecondEntry()==theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"))
			{
				lvi.iImage =1%2;		// There are 8 images in the image list
			}
			m_lstSpamFilter.InsertItem(&lvi);
			m_lstSpamFilter.SetItemText(nCurrentItem, 1, contact.GetFirstEntry());
			
		}
		
	}
	if(!m_bContentFilter)
	{
		// delete all current members
		m_lstContentFilter.DeleteAllItems();


		// get a reference to the contacts list
		const ContactList& contacts = m_objContentFilterdb.GetContacts();

		// iterate over all contacts add add them to the list

		POSITION pos = contacts.GetHeadPosition();
		
		while(pos != NULL)
		{
			nCurrentItem=0;
			const CIspyList contact = contacts.GetNext(pos);
			LVITEM lvi;

			//	// Insert the first item
			lvi.mask =  LVIF_IMAGE | LVIF_TEXT;
			lvi.iItem = nCurrentItem;
			lvi.iSubItem = 0;
			lvi.pszText =(LPTSTR)(LPCTSTR)(contact.GetSecondEntry());
			if(contact.GetSecondEntry()==theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE"))
			{
				lvi.iImage =0%2;		// There are 8 images in the image list
			}
			if(contact.GetSecondEntry()==theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"))
			{
				lvi.iImage =1%2;		// There are 8 images in the image list
			}
			m_lstContentFilter.InsertItem(&lvi);
			m_lstContentFilter.SetItemText(nCurrentItem, 1, contact.GetThirdEntry());
			m_lstContentFilter.SetItemText(nCurrentItem, 2, contact.GetFirstEntry());
			
		}
		
	}
	if(!m_bSignatureFlag)
	{
		// get a reference to the contacts list
		CString Signature;
		const ContactList& contacts = m_objSignaturedb.GetContacts();

		// iterate over all contacts add add them to the list
		int nCurrentItem = 0;
		POSITION pos = contacts.GetHeadPosition();
		while(pos != NULL)
		{
			const CIspyList contact = contacts.GetNext(pos);
			Signature=contact.GetFirstEntry();
			m_edtSigDesMsg.SetWindowTextW(Signature);

		}
	}
	if(!m_bVirusScan)
	{
		// issue:- emailVirusScanDlg getting hang or its take time to redraw because list getting re-fill every time when user nevigate on tab list item even there is no change in EmailVirusScan.DB file
		// resolved by lalit kumawat 8-7-2015

		// get a reference to the contacts list
		const ContactList& contacts = m_objVirusScandb.GetContacts();

		m_iPrevCountEmailVScan = m_iCountEmailVScan;
		m_iCountEmailVScan = static_cast<int>(contacts.GetCount());

		if (m_iPrevCountEmailVScan == m_iCountEmailVScan)
			return;

		// delete all current members
		m_lstVirusScan.DeleteAllItems();

		// iterate over all contacts add add them to the list
		POSITION pos = contacts.GetHeadPosition();
		
		while(pos != NULL)
		{
			nCurrentItem=0;
			const CIspyList contact = contacts.GetNext(pos);
			LVITEM lvi;

			//	// Insert the first item
			lvi.mask =  LVIF_IMAGE | LVIF_TEXT;
			lvi.iItem = nCurrentItem;
			lvi.iSubItem = 0;
			lvi.pszText =(LPTSTR)(LPCTSTR)(L" ");
			CString csStatus = contact.GetForthEntry();

			//Issue no 1162,1050,1032,1029 The images according to status is not coming properly
			//Neha Gharge 29th Dec,2015
			if (csStatus == SZDETECTED || csStatus == SZREPAIRED || csStatus == SZQUARANTINED || csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_OUTLOOK_PLUGIN_THREATS_BLOCKED") || (csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_THREAT_DETECTED")))
			{
				lvi.iImage =1%2;		// There are 8 images in the image list
			}
			if (csStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCAN_NO_THREAT_FOUND") || csStatus == SZNOTHREAT)
			{
				lvi.iImage =0%2;		// There are 8 images in the image list
			}
			m_lstVirusScan.InsertItem(&lvi);
			m_lstVirusScan.SetItemText(nCurrentItem, 2, contact.GetThirdEntry());
			m_lstVirusScan.SetItemText(nCurrentItem, 3, contact.GetForthEntry());
			m_lstVirusScan.SetItemText(nCurrentItem, 1, contact.GetSecondEntry());
			nCurrentItem++;//flicker problem
			
		}
		
	}
}


/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonEdit                                                     
*  Description    :	If user want to edit some entries 
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::OnBnClickedButtonEdit()
{
	// TODO: Add your control notification handler code here
	CString csDeleteBeforeEditEntry = L"";
	TCHAR * szDeleteBeforeEditEntry = L"";
	m_btnApply.EnableWindow(FALSE);
	m_btnDelete.EnableWindow(FALSE);
	int iSelectitemCount;
	if(!m_bSpamFilter)
	{
		iSelectitemCount=m_lstSpamFilter.GetSelectedCount();
		if(iSelectitemCount==0)
		{

			HideORShowButton(true,true,true,false);
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_NO_ENTRY_EDIT"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
			m_bIsPopUpDisplayed = false;
			return;
		}
		
		if(iSelectitemCount>1)
		{

			HideORShowButton(true,true,true,false);
			
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SELECT_ATLST_ENTRY_EDIT"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
			m_bIsPopUpDisplayed = false;
			return;
		}
		m_dwnitem = m_lstSpamFilter.GetSelectionMark();
		g_DelType= m_lstSpamFilter.GetItemText(m_dwnitem,0);
		
	
		if(g_DelType==theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE"))
		{
			m_DropList.SetCurSel(0);
		}
		if(g_DelType==theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"))
		{
			m_DropList.SetCurSel(1);
		}
		g_DelEmailAddr= m_lstSpamFilter.GetItemText(m_dwnitem,1);
		g_DelEmailAddr=g_DelEmailAddr;
		m_edtEmailAddress.SetWindowTextW(g_DelEmailAddr);

		m_objSpamFilterdb.bWithoutEdit=0;
		m_bEditFlag=0;
		m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE"));
		m_btnEdit.EnableWindow(FALSE);

		//Issue no 1269 and 1270 User can add # as rule. No special symbol is used to tonize word
		//csDeleteBeforeEditEntry.Format(L"%s#%s#\n",g_DelEmailAddr,g_DelType);

		if(!SendEmailData2Service(DELETE_EMAIL_ENTRY , SPAMFILTER , g_DelEmailAddr,g_DelType,L"",true))
		{
			AddLogEntry(L"### Error in DELETE_EMAIL_ENTRY in SendEmailData2Service", 0, 0, true, SECONDLEVEL);
		}

	}



	if(!m_bContentFilter)
	{
		iSelectitemCount=m_lstContentFilter.GetSelectedCount();
		if(iSelectitemCount==0)
		{
			HideORShowButton(true,true,true,false);
			
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_NO_ENTRY_EDIT"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
			m_bIsPopUpDisplayed = false;

			return;
		}
		
		if(iSelectitemCount>1)
		{
			HideORShowButton(true,true,true,false);
			
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SELECT_ATLST_ENTRY_EDIT"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
			m_bIsPopUpDisplayed = false;
			return;
		}
		m_dwnitem = m_lstContentFilter.GetSelectionMark();
		g_DelType= m_lstContentFilter.GetItemText(m_dwnitem,0);
		g_DelRuleType=m_lstContentFilter.GetItemText(m_dwnitem,1);
		if(g_DelType==theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE"))
		{
			m_DropList.SetCurSel(0);
		}
		if(g_DelType==theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"))
		{
			m_DropList.SetCurSel(1);
		}
		if(g_DelRuleType==theApp.m_objwardwizLangManager.GetString(L"IDS_ATTACHMENT_EXT_IS_CONTENT"))
		{
			m_DroplistContentFilter.SetCurSel(0);
		}
		if(g_DelRuleType==theApp.m_objwardwizLangManager.GetString(L"IDS_MESSAGE_CONTAIN_CONTENT"))
		{
			m_DroplistContentFilter.SetCurSel(1);
		}
		if(g_DelRuleType==theApp.m_objwardwizLangManager.GetString(L"IDS_SUBJECT_CONTAIN_CONTECT"))
		{
			m_DroplistContentFilter.SetCurSel(2);
		}
		g_DelEmailAddr= m_lstContentFilter.GetItemText(m_dwnitem,2);
		g_DelEmailAddr=g_DelEmailAddr;
		m_edtEmailAddress.SetWindowTextW(g_DelEmailAddr);

		m_objContentFilterdb.bWithoutEdit=0;
		m_bEditFlag=0;
		m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE"));
		m_btnEdit.EnableWindow(FALSE);
		
		//Issue no 1269 and 1270 User can add # as rule. No special symbol is used to tonize word
		//csDeleteBeforeEditEntry.Format(L"%s#%s#%s#\n",g_DelEmailAddr,g_DelType,g_DelRuleType);
		if (!SendEmailData2Service(DELETE_EMAIL_ENTRY, CONTENTFILTER, g_DelEmailAddr, g_DelType, g_DelRuleType, true))
		{
			AddLogEntry(L"### Error in DELETE_EMAIL_ENTRY in SendEmailData2Service", 0, 0, true, SECONDLEVEL);
		}

	}
	g_DelEmailAddr = L"";
	g_DelRuleType = L"";
	g_DelType = L"";
}


/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonDelete                                                     
*  Description    :	If user want to delete some entries
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::OnBnClickedButtonDelete()
{
	// TODO: Add your control notification handler code here
	int iSelectitemCount;
	CString csDeleteEntry = L"";
	TCHAR * szDeleteEntry = L"";

	HideORShowButton(false,false,false,true);

	if(!m_bSpamFilter)
	{
		iSelectitemCount=m_lstSpamFilter.GetSelectedCount();
		if(iSelectitemCount==0)
		{

			HideORShowButton(true,true,true,false);
			
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_NO_ENTRY_DELETE"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
			m_bIsPopUpDisplayed = false;

			return;
		}
		
		if(iSelectitemCount>1)
		{

			HideORShowButton(true,true,true,false);
			
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SELECT_ATLST_ENTRY_DELETE"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
			m_bIsPopUpDisplayed = false;

			return;
		}

		m_dwnDeleteItem = m_lstSpamFilter.GetSelectionMark();
	
		g_DelEmailAddr=m_lstSpamFilter.GetItemText(m_dwnDeleteItem,1);
		g_DelType=m_lstSpamFilter.GetItemText(m_dwnDeleteItem,0);
		m_lstSpamFilter.DeleteItem(m_dwnDeleteItem);
		m_bDeleteFlag=0;

		//Issue no 1269 and 1270 User can add # as rule. No special symbol is used to tonize word
		//csDeleteEntry.Format(L"%s#%s#\n",g_DelEmailAddr,g_DelType);
		if (!SendEmailData2Service(DELETE_EMAIL_ENTRY, SPAMFILTER, g_DelEmailAddr, g_DelType,L"",true))
		{
			AddLogEntry(L"### Error in DELETE_EMAIL_ENTRY in SendEmailData2Service", 0, 0, true, SECONDLEVEL);
		}


	}
	if(!m_bContentFilter)
	{
		iSelectitemCount=m_lstContentFilter.GetSelectedCount();
		if(iSelectitemCount==0)
		{

			HideORShowButton(true,true,true,false);
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_NO_ENTRY_DELETE"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
			m_bIsPopUpDisplayed = false;
			return;
		}
		
		if(iSelectitemCount>1)
		{

			HideORShowButton(true,true,true,false);
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SELECT_ATLST_ENTRY_DELETE"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
			m_bIsPopUpDisplayed = false;
			return;
		}
		m_dwnDeleteItem = m_lstContentFilter.GetSelectionMark();
	
		g_DelEmailAddr=m_lstContentFilter.GetItemText(m_dwnDeleteItem,2);
		g_DelType=m_lstContentFilter.GetItemText(m_dwnDeleteItem,0);
		g_DelRuleType=m_lstContentFilter.GetItemText(m_dwnDeleteItem,1);
		m_lstContentFilter.DeleteItem(m_dwnDeleteItem);
		m_bDeleteFlag=0;

		//Issue no 1269 and 1270 User can add # as rule. No special symbol is used to tonize word
		//csDeleteEntry.Format(L"%s#%s#%s#\n",g_DelEmailAddr,g_DelType,g_DelRuleType);

		if (!SendEmailData2Service(DELETE_EMAIL_ENTRY, CONTENTFILTER, g_DelEmailAddr, g_DelType, g_DelRuleType, true))
		{
			AddLogEntry(L"### Error in DELETE_EMAIL_ENTRY in SendEmailData2Service", 0, 0, true, SECONDLEVEL);
		}

	}
	g_DelEmailAddr = L"";
	g_DelRuleType = L"";
	g_DelType = L"";
}


/**********************************************************************************************************                     
*  Function Name  :	LoadExistingSpamFilterFile                                                     
*  Description    :	Load the data from spam db file
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::LoadExistingSpamFilterFile()
{
	static CString csFilePath;
	TCHAR szModulePath[MAX_PATH] = {0};
	if(!GetModulePath(szModulePath, MAX_PATH))
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_GETMODULE_ERROR "),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR|MB_OK);
		m_bIsPopUpDisplayed = false;
		return;
	}
	csFilePath = szModulePath;
	csFilePath += _T("\\WRDWIZSPAMEMAIL.DB");
	


	m_bSpamFilter=0;
	m_bContentFilter=1;
	m_bSignatureFlag=1;
	m_bVirusScan = 1;

	LoadDataContentFromFile(csFilePath);
}

/**********************************************************************************************************                     
*  Function Name  :	LoadExistingContentFilterFile                                                     
*  Description    :	Load the data from content db file
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::LoadExistingContentFilterFile()
{
	static CString csFilePath;
	TCHAR szModulePath[MAX_PATH] = {0};
	if(!GetModulePath(szModulePath, MAX_PATH))
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_GETMODULE_ERROR "),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR|MB_OK);
		m_bIsPopUpDisplayed = false;
		return;
	}
	csFilePath = szModulePath;
	csFilePath += _T("\\WRDWIZCONTENTEMAIL.DB");
	
	m_bVirusScan = 1;
	m_bSpamFilter=1;
	m_bContentFilter=0;
	m_bSignatureFlag=1;
	LoadDataContentFromFile(csFilePath);
}

/**********************************************************************************************************                     
*  Function Name  :	LoadExistingSignatureFile                                                     
*  Description    :	Load the data from signature db file
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::LoadExistingSignatureFile()
{
	static CString csFilePath;
	TCHAR szModulePath[MAX_PATH] = {0};
	if(!GetModulePath(szModulePath, MAX_PATH))
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_GETMODULE_ERROR "),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR|MB_OK);
		m_bIsPopUpDisplayed = false;
		return;
	}
	csFilePath = szModulePath;
	csFilePath += _T("\\WRDWIZSIGNATUREEMAIL.DB");
	
	m_bVirusScan = 1;
	m_bSpamFilter=1;
	m_bContentFilter=1;
	m_bSignatureFlag=0;
	
	m_objSignaturedb.RemoveAll();
	LoadDataContentFromFile(csFilePath);
}

/**********************************************************************************************************                     
*  Function Name  :	LoadExistingVirusScanFile                                                     
*  Description    :	Load the data from virus scan db file
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::LoadExistingVirusScanFile()
{
	static CString csFilePath;
	TCHAR szModulePath[MAX_PATH] = {0};
	if(!GetModulePath(szModulePath, MAX_PATH))
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_GETMODULE_ERROR "),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR|MB_OK);
		m_bIsPopUpDisplayed = false;

		return;
	}
	csFilePath = szModulePath;
	csFilePath += _T("\\WRDWIZVIRUSSCANEMAIL.DB");
	

	m_bSpamFilter=1;
	m_bContentFilter=1;
	m_bSignatureFlag=1;
	m_bVirusScan=0;
	LoadDataContentFromFile(csFilePath);
}


/**********************************************************************************************************                     
*  Function Name  :	isEmail                                                     
*  Description    :	Validate email iD
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/

bool CISpyEmailScanDlg:: isEmail(CString inputEmail)
{
	bool flag = true;
	CString TempEmail;
	
	int i=0;
	int posDot,posAt,posAt2,posDotSec;

	posAt = inputEmail.ReverseFind(_T('@'));
	posAt2 = inputEmail.Find(_T('@'));
	i=0;
	
	if(posAt == posAt2)
	{
		int pos = inputEmail.Find(_T(".."));
		if(pos > -1)
		{
			return false;
		}
		pos=inputEmail.Find(_T(".@"));
		if(pos > -1)
		{
			return false;
		}
		
		pos=inputEmail.ReverseFind(_T('.'));
		int length= inputEmail.GetLength();
		if(pos == length-1)
		{
			return false;
		}

		flag=0;
		TempEmail = inputEmail.Right(inputEmail.GetLength() - posAt);
	
		posDotSec = TempEmail.Find(_T('.'));

		posDot = TempEmail.ReverseFind(_T('.'));
		if(posDot == -1)
		{
			return false;
		}
		int diffDot = posDot-posDotSec; 
		if(diffDot == 1)
		{
			return false;
		}

		posAt = TempEmail.Find(_T('@'));
		int diff = posDotSec-posAt;
		if(diff == 1)
		{
			return false;
		}

		if(TempEmail==L"")
		{
			return false;
		}

		if(posAt > -1)
		{
			posAt = inputEmail.Find(_T('@'));
			inputEmail.Truncate(posAt);
			if(inputEmail==L"")
			{
				return false;
			}
		}

	}
	else
	{
		flag=1;
	}
	if(posAt == -1)
	{
		flag=1;
	}
	
if(!flag)
{
	
	return true;
}
else
return (false);
	
} 


/**********************************************************************************************************                     
*  Function Name  :	isExtension                                                     
*  Description    :	Validate extension
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
bool CISpyEmailScanDlg:: isExtension(CString inputExtension)
{
	bool flag=false;
	int pos=-1;
	int length;
	length=inputExtension.GetLength();
	for (int i=0;i<length;i++)
	{
		if((inputExtension.GetAt(i)) == (_T('.')))
			pos++;
	}

	if(pos>1||pos!=0)
		flag = false;
	else
		flag = true;

	pos=inputExtension.Find(_T('.'));
	if(pos!=0)
		flag = false;

	if(length==1)
		flag= false;

	if(flag)
		return true;
	else
		return false;
		
		//TCHAR extension[10];
	/*bool flag=1;
	int idigit=0;
	int PosExt,PosDot,i=0;
	PosExt=inputExtension.GetLength();
	PosDot=inputExtension.ReverseFind(_T('.'));
	if(PosDot==-1)
	{
		flag=1;
		return false;
	}
	if(PosExt<=5)
	{
		int Dotpos=inputExtension.Find(_T('.'));
		inputExtension.Right(inputExtension.GetLength() - (Dotpos+1));
		
		if((Dotpos==-1 )|| (Dotpos > 0))
		{
			flag=1;
		}
		else
		{
			flag=0;
			if(inputExtension==L".")
			{
				flag=1;
			}
			for(int i=0;i<inputExtension.GetLength();i++)
			{
				if(isdigit(inputExtension[i]))
				{
					idigit++;
				}
			}
			if(idigit==(inputExtension.GetLength()-1))
			{
				flag = 1;
			}
		}
	}
	
	if(PosExt>5)
	{
		flag=1;
	}

if(!flag)
return true;
else
return false;*/
}


/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedCheckSignatureCheckbox()                                                     
*  Description    :	Enable or disable  signature by clicking on check box
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::OnBnClickedCheckSignatureCheckbox()
{
	m_btnApply.EnableWindow(TRUE);
	// TODO: Add your control notification handler code here
	
	//int iCheck = m_chkAllowSignature.GetCheck();
	//if(iCheck == 0)
	//{
	//	m_dwEnableSignature = 0;
	//	WriteRegistryEntryofEmailScan(m_dwEnableSignature);
	//}
	//else
	//{
	//	m_dwEnableSignature = 1;
	//	WriteRegistryEntryofEmailScan(m_dwEnableSignature);
	//}
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedCheckVirusscan()                                                     
*  Description    :	Enable or disable  virus scan by clicking on check box
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::OnBnClickedCheckVirusscan()
{
	// TODO: Add your control notification handler code here
	/*****************ISSUE NO -112 Neha Gharge 22/5/14 ************************************************/
	m_btnApply.EnableWindow(TRUE);

	/*int iCheck = m_chkVirusScan.GetCheck();
	if(iCheck == 0)
	{
		m_bEnableVirusPopUp = true;
		m_dwEnableVirusScan = 0;
		WriteRegistryEntryofEmailScan(m_dwEnableVirusScan);
	}
	else
	{
		m_bEnableVirusPopUp = true;
		m_dwEnableVirusScan = 1;
		WriteRegistryEntryofEmailScan(m_dwEnableVirusScan);
	}
	m_bEnableVirusPopUp = false;*/
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedCheckSpamfilter()                                                     
*  Description    :	Enable or disable  spam filter by clicking on check box
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::OnBnClickedCheckSpamfilter()
{
	m_btnApply.EnableWindow(TRUE);

	//// TODO: Add your control notification handler code here
	//
	//int iCheck = m_chkSpamFilter.GetCheck();
	//if(iCheck == 0)
	//{
	//	m_dwEnableSpamFilter = 0;
	//	WriteRegistryEntryofEmailScan(m_dwEnableSpamFilter);
	//}
	//else
	//{
	//	m_dwEnableSpamFilter = 1;
	//	WriteRegistryEntryofEmailScan(m_dwEnableSpamFilter);
	//}


}


/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedCheckEnablecontent()                                                     
*  Description    :	Enable or disable content filter by clicking on check box
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::OnBnClickedCheckEnablecontent()
{
	m_btnApply.EnableWindow(TRUE);

	// TODO: Add your control notification handler code here
	
	//int iCheck = m_chkContentfilter.GetCheck();
	//if(iCheck == 0)
	//{
	//	m_dwEnableContentFilter = 0;
	//	WriteRegistryEntryofEmailScan(m_dwEnableContentFilter);
	//}
	//else
	//{
	//	m_dwEnableContentFilter = 1;
	//	WriteRegistryEntryofEmailScan(m_dwEnableContentFilter);
	//}
}


/**********************************************************************************************************                     
*  Function Name  :	WriteRegistryEntryofEmailScan()                                                     
*  Description    :	Write emailscan changes or setting into registry 
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::WriteRegistryEntryofEmailScan(DWORD dwChangeValue)
{
	DWORD dwChangeSetting = 1;

	AddLogEntry(L">>> EmailScanDlg : WriteRegistryEntryofEmailScan", 0, 0, true, FIRSTLEVEL);

	LPCTSTR SubKey = TEXT("SOFTWARE\\Wardwiz Antivirus\\EmailScanSetting");

	if(!m_bSpamFilter)
	{
		if(!m_bReloadSetting)
		{
			if(!SetRegistrykeyUsingService(SubKey, L"dwReloadSettings", REG_DWORD, dwChangeValue))
			{
				AddLogEntry(L"### Error in Setting Registry CEmailScanDlg::ReloadSetting", 0, 0, true, SECONDLEVEL);
			}
			return;
		}
		
		if(!SetRegistrykeyUsingService(SubKey, L"dwEnableSpamFilter", REG_DWORD, dwChangeValue))
		{
			AddLogEntry(L"### Error in Setting Registry CEmailScanDlg::OnBnClickedCheckEnableSpamFilter", 0, 0, true, SECONDLEVEL);
		}
	}

	if(!m_bContentFilter)
	{
		if(!m_bReloadSetting)
		{
			if(!SetRegistrykeyUsingService(SubKey, L"dwReloadSettings",REG_DWORD,dwChangeValue))
			{
				AddLogEntry(L"### Error in Setting Registry CEmailScanDlg::ReloadSetting", 0, 0, true, SECONDLEVEL);
			}
			return;
		}

		if(!SetRegistrykeyUsingService(SubKey, L"dwEnableContentFilter", REG_DWORD, dwChangeValue))
		{
			AddLogEntry(L"### Error in Setting Registry CEmailScanDlg::OnBnClickedCheckEnableContentFilter", 0, 0, true, SECONDLEVEL);
		}
	}
		
	if(!m_bVirusScan)
	{
		if(!m_bReloadSetting)
		{
			if(!SetRegistrykeyUsingService(SubKey, L"dwReloadSettings",REG_DWORD,dwChangeValue))
			{
				AddLogEntry(L"### Error in Setting Registry CEmailScanDlg::ReloadSetting", 0, 0, true, SECONDLEVEL);
			}
			return;
		}
		if(!SetRegistrykeyUsingService(SubKey, L"dwEnableVirusScan", REG_DWORD, dwChangeValue))
		{
			AddLogEntry(L"### Error in Setting Registry CEmailScanDlg::OnBnClickedCheckEnableVirusScan", 0, 0, true, SECONDLEVEL);
		}
	}
	
	if(!m_bSignatureFlag)
	{
		if(!m_bReloadSetting)
		{
			if(!SetRegistrykeyUsingService(SubKey, L"dwReloadSettings", REG_DWORD, dwChangeValue))
			{
				AddLogEntry(L"### Error in Setting Registry CEmailScanDlg::ReloadSetting", 0, 0, true, SECONDLEVEL);
			}
			return;
		}
		
		if(!SetRegistrykeyUsingService(SubKey, L"dwEnableSignature", REG_DWORD, dwChangeValue))
		{
			AddLogEntry(L"### Error in Setting Registry CEmailScanDlg::OnBnClickedCheckEnableSignature", 0, 0, true, SECONDLEVEL);
		}
	}
	
	if(!m_bEnableQuarantine)
	{
		if(!SetRegistrykeyUsingService(SubKey, L"dwAttachScanAction", REG_DWORD, dwChangeValue))
		{
			AddLogEntry(L"### Error in Setting Registry CEmailScanDlg::OnBnClickedRadioRemove/Quarantine", 0, 0, true, SECONDLEVEL);
		}
	}

	if(!m_bEnableRemove)
	{
		if(!SetRegistrykeyUsingService(SubKey, L"dwAttachScanAction", REG_DWORD, dwChangeValue))
		{
			AddLogEntry(L"### Error in Setting Registry CEmailScanDlg::OnBnClickedRadioRemove/Quarantine", 0, 0, true, SECONDLEVEL);
		}
	}

	if(!m_bEnableVirusPopUp)
	{
		if(!SetRegistrykeyUsingService(SubKey, L"dwAllowVirusPopUp", REG_DWORD, dwChangeValue))
		{
			AddLogEntry(L"### Error in Setting Registry CEmailScanDlg::OnBnClickedCheckAllowVirusScanPopUp", 0, 0, true, SECONDLEVEL);
		}
	}
}


/**********************************************************************************************************                     
*  Function Name  :	ReadRegistryEntryofEmailScan()                                                     
*  Description    :	Read emailscan changes or setting into registry 
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::ReadRegistryEntryofEmailScan()
{
	HKEY hKey;
	LONG ReadReg;
	DWORD dwvalueSType;
	DWORD dwvalueSize = sizeof(DWORD);
	DWORD Chkvalue;
	DWORD dwType=REG_DWORD;
	if(RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Wardwiz Antivirus\\EmailScanSetting"),&hKey) != ERROR_SUCCESS)
	{
		AddLogEntry(L"### Unable to open registry key EmailScanSetting", 0, 0, true, SECONDLEVEL);
		return;
	}
	
	if(!m_bVirusScan)
	{
		ReadReg=RegQueryValueEx(hKey,L"dwEnableVirusScan",NULL,&dwType,(LPBYTE)&dwvalueSType,&dwvalueSize);
		if(ReadReg == ERROR_SUCCESS)
		{
			Chkvalue=(DWORD)dwvalueSType;
			if(Chkvalue==0)
			{
				m_chkVirusScan.SetCheck(0);
			}
			else
			{
				m_chkVirusScan.SetCheck(1);
			}
			
		}
		ReadReg=RegQueryValueEx(hKey,L"dwAllowVirusPopUp",NULL,&dwType,(LPBYTE)&dwvalueSType,&dwvalueSize);
		if(ReadReg == ERROR_SUCCESS)
		{
			Chkvalue=(DWORD)dwvalueSType;
			if(Chkvalue==0)
			{
				m_chkAllowVirusPopUp.SetCheck(0);
				m_btnQuarantine.EnableWindow(TRUE);
				m_stQuarantineText.EnableWindow(TRUE);
				m_btnRemove.EnableWindow(TRUE);
				m_stRemoveText.EnableWindow(TRUE);
				
			}
			else
			{
				m_chkAllowVirusPopUp.SetCheck(1);
				m_btnQuarantine.EnableWindow(FALSE);
				m_stQuarantineText.EnableWindow(FALSE);
				m_btnRemove.EnableWindow(FALSE);
				m_stRemoveText.EnableWindow(FALSE);
			}
			
		}
	}

	if(!m_bSpamFilter)
	{
		ReadReg=RegQueryValueEx(hKey, L"dwEnableSpamFilter", NULL ,&dwType,(LPBYTE)&dwvalueSType,&dwvalueSize);
		if(ReadReg == ERROR_SUCCESS)
		{
			Chkvalue=(DWORD)dwvalueSType;
			if(Chkvalue==0)
			{
				m_chkSpamFilter.SetCheck(0);
			}
			else
			{
				m_chkSpamFilter.SetCheck(1);
			}
			
		}
	}

	if(!m_bContentFilter)
	{
		ReadReg=RegQueryValueEx(hKey, L"dwEnableContentFilter", NULL ,&dwType,(LPBYTE)&dwvalueSType,&dwvalueSize);
		if(ReadReg == ERROR_SUCCESS)
		{
			Chkvalue=(DWORD)dwvalueSType;
			if(Chkvalue==0)
			{
				m_chkContentfilter.SetCheck(0);
			}
			else
			{
				m_chkContentfilter.SetCheck(1);
			}
			
		}
	}

	if(!m_bSignatureFlag)
	{
		ReadReg=RegQueryValueEx(hKey, L"dwEnableSignature", NULL ,&dwType,(LPBYTE)&dwvalueSType,&dwvalueSize);
		if(ReadReg == ERROR_SUCCESS)
		{
			Chkvalue=(DWORD)dwvalueSType;
			if(Chkvalue==0)
			{
				m_chkAllowSignature.SetCheck(0);
			}
			else
			{
				m_chkAllowSignature.SetCheck(1);
			}
		}
	}
	RegCloseKey(hKey);
}

/**********************************************************************************************************                     
*  Function Name  :	InitImageList()                                                     
*  Description    :	Initialization of image list
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
BOOL CISpyEmailScanDlg::InitImageList()
{
	// Create 256 color image lists
	HIMAGELIST hList;
	HIMAGELIST hVirusList;
	
	hList = ImageList_Create(16, 16, ILC_COLOR8|ILC_MASK,2,1);
	m_ImageList.Attach(hList);

	hVirusList = ImageList_Create(16, 16, ILC_COLOR8|ILC_MASK,2,1);
	m_VirusImageList.Attach(hVirusList);


	// Load the large icons
	CBitmap cBmp,CVirusBmp;
	
	// Load the small icons
	cBmp.LoadBitmap(IDB_BITMAP_ALLOW_BLOCK);
	m_ImageList.Add(&cBmp, RGB(255,0, 255));
	cBmp.DeleteObject();

	CVirusBmp.LoadBitmapW(IDB_BITMAP_MAILBOXES);
	m_VirusImageList.Add(&CVirusBmp, RGB(255,0, 255));
	CVirusBmp.DeleteObject();

	// Attach them
	m_lstSpamFilter.SetImageList(&m_ImageList, LVSIL_SMALL);
	m_lstContentFilter.SetImageList(&m_ImageList, LVSIL_SMALL);
	m_lstVirusScan.SetImageList(&m_VirusImageList, LVSIL_SMALL);


	return TRUE;

}

/**********************************************************************************************************                     
*  Function Name  :	PreTranslateMessage()                                                     
*  Description    :	
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/

BOOL CISpyEmailScanDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);

}


/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedCheckAllowviruspopup()                                                     
*  Description    :	Allow /disallow popup of virus scan while email scan
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::OnBnClickedCheckAllowviruspopup()
{
	// TODO: Add your control notification handler code here
	int iCheck = m_chkAllowVirusPopUp.GetCheck();
	
	if(iCheck == 0)
	{
		m_bEnableVirusPopUp=0;
		m_dwEnableVirusPopUp = 0;
		m_btnQuarantine.EnableWindow(TRUE);
		m_stQuarantineText.EnableWindow(TRUE);
		m_btnRemove.EnableWindow(TRUE);
		m_stRemoveText.EnableWindow(TRUE);
		m_btnApply.EnableWindow(TRUE);

	}
	else
	{
		m_bEnableVirusPopUp=0;
		m_dwEnableVirusPopUp = 1;
		m_btnQuarantine.EnableWindow(FALSE);
		m_stQuarantineText.EnableWindow(FALSE);
		m_btnRemove.EnableWindow(FALSE);
		m_stRemoveText.EnableWindow(FALSE);
		m_btnApply.EnableWindow(TRUE);
	}
}


/**********************************************************************************************************                     
*  Function Name  :	GetExistingPathofOutlook()                                                     
*  Description    : It check user outlook isinstalled or not
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
bool CISpyEmailScanDlg::GetExistingPathofOutlook()
{
	bool bReturn = false;
	try
	{
		LPTSTR ppszVersion;
		BOOL	pf64Bit = FALSE;
		HRESULT result;
		CString csActualGUIDs;
		CString csInstalledGUID;
		int iPos =0;
		result = GetOutlookVersionString(&ppszVersion,&pf64Bit);

		///if failed to retrive values means outlook is not installed
		if(FAILED(result))
		{
			AddLogEntry(L">>> EmailScanDlg : GetOutlookVersionString:outlook is not installed", 0, 0, true, FIRSTLEVEL);
			// only for GetOutlookVersionString get failed for 2016
			// Hence It is remain disable. 
			if (CheckForOutllookExe())
			{
				return true;
			}
			return false;
		}

		//if outlook is 64 bit return false
		//if(pf64Bit)
		//{
		//	AddLogEntry(L">>> EmailScanDlg : GetOutlookVersionString :outlook is 64bit");
		//	return false;
		//}

		for(int i=0 ; i<10 ; i++)
		{
			csActualGUIDs = g_GUIDofInstalledOutllook[i];
			csInstalledGUID = (LPCTSTR)ppszVersion;
			csInstalledGUID = csInstalledGUID.Tokenize(_T("."),iPos);
			iPos =0;
			if(!(csActualGUIDs.Compare(csInstalledGUID)))
			{
				bReturn = true;
				//if(bReturn)
				//{
				//	//if(pf64Bit == 1)
				//	//{
				//	//	bReturn = false;
				//	//	break;
				//	//}
				//	break;
				//}
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CGenXEmailScanDlg::GetExistingPathofOutlook", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/**********************************************************************************************************                     
*  Function Name  :	GetExistingPathofOutlook()                                                     
*  Description    : It check user outlook isinstalled or not 64bit
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
HRESULT CISpyEmailScanDlg::GetOutlookVersionString(LPTSTR* ppszVer, BOOL* pf64Bit)
{
    HRESULT hr = E_FAIL;
	LPTSTR pszTempPath = NULL;
	LPTSTR pszTempVer = NULL;

	__try
	{

		TCHAR pszOutlookQualifiedComponents[][MAX_PATH] = {
			TEXT("{E83B4360-C208-4325-9504-0D23003A74A5}"), // Outlook 2013
			TEXT("{1E77DE88-BCAB-4C37-B9E5-073AF52DFD7A}"), // Outlook 2010
			TEXT("{24AAE126-0911-478F-A019-07B875EB9996}"), // Outlook 2007
			TEXT("{BC174BAD-2F53-4855-A1D5-0D575C19B1EA}")  // Outlook 2003
		};
		int nOutlookQualifiedComponents = _countof(pszOutlookQualifiedComponents);
		int i = 0;
		DWORD dwValueBuf = 0;
		UINT ret = 0;

		*pf64Bit = FALSE;

		for (i = 0; i < nOutlookQualifiedComponents; i++)
		{
			ret = MsiProvideQualifiedComponent(
				pszOutlookQualifiedComponents[i],
				TEXT("outlook.x64.exe"),
				(DWORD) INSTALLMODE_DEFAULT,
				NULL,
				&dwValueBuf);
			if (ERROR_SUCCESS == ret) break;
		}

		if (ret != ERROR_SUCCESS)
		{
			for (i = 0; i < nOutlookQualifiedComponents; i++)
			{
				ret = MsiProvideQualifiedComponent(
					pszOutlookQualifiedComponents[i],
					TEXT("outlook.exe"),
					(DWORD) INSTALLMODE_DEFAULT,
					NULL,
					&dwValueBuf);
				if (ERROR_SUCCESS == ret) break;
			}
		}
		else
		{
			*pf64Bit = TRUE;
		}

		if (ret == ERROR_SUCCESS)
		{
			dwValueBuf += 1;
			pszTempPath = (LPTSTR) malloc(dwValueBuf * sizeof(TCHAR));
			if (pszTempPath != NULL)
			{
				if ((ret = MsiProvideQualifiedComponent(
					pszOutlookQualifiedComponents[i],
					TEXT("outlook.exe"),
					(DWORD) INSTALLMODE_EXISTING,
					pszTempPath,
					&dwValueBuf)) != ERROR_SUCCESS)
				{
					goto Error;
				}

				pszTempVer = (LPTSTR) malloc(MAX_PATH * sizeof(TCHAR));
				dwValueBuf = MAX_PATH;
				if ((ret = MsiGetFileVersion(pszTempPath,
					pszTempVer,
					&dwValueBuf,
					NULL,
					NULL))!= ERROR_SUCCESS)
				{
					goto Error;    
				}
				*ppszVer = pszTempVer;
				pszTempVer = NULL;
				hr = S_OK;
			}
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CGenXEmailScanDlg::GetOutlookVersionString", 0, 0, true, SECONDLEVEL);
	}

Error:
	if(pszTempVer)
	{
		free(pszTempVer);
	}
	if(pszTempPath)
	{
	    free(pszTempPath);
	}
	return hr;
}
 
/**********************************************************************************************************                     
*  Function Name  :	DisableEnableAllFunctions()                                                     
*  Description    : It enables and disables all function
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::DisableEnableAllFunctions(bool bEnable)
{
	//m_btnSignature.EnableWindow(bEnable);
	//m_btnSpamFilter.EnableWindow(bEnable);
	//m_btnContentFilter.EnableWindow(bEnable);
	//m_btnVirusScan.EnableWindow(bEnable);
	m_lstVirusScan.ShowWindow(bEnable);
	m_stSettingHeaderPic.ShowWindow(bEnable);
	m_stEnableVirusScan.ShowWindow(bEnable);
	m_stAllowVirusPopdlg.ShowWindow(bEnable);
	m_chkAllowVirusPopUp.ShowWindow(bEnable);
	m_chkVirusScan.ShowWindow(bEnable);
	//m_stEmailHeaderBitmap.ShowWindow(bEnable);
	m_stQuarantineText.ShowWindow(bEnable);
	m_stRemoveText.ShowWindow(bEnable);
	m_btnQuarantine.ShowWindow(bEnable);
	m_btnRemove.ShowWindow(bEnable);
	m_stGrpBoxForRadioBtn.ShowWindow(bEnable);
	m_btnApply.EnableWindow(FALSE);
	m_btnApply.ShowWindow(bEnable);
	
	if(!bEnable)
	{
		m_bSignature=1;
		m_stAllowVirusPopdlg.SetBkColor(RGB(243,239,238));
		m_stRemoveText.SetBkColor(RGB(243,239,238));
		m_stQuarantineText.SetBkColor(RGB(243,239,238));

	}
	else
	{
		m_bSignature=0;
		m_stAllowVirusPopdlg.SetBkColor(RGB(223,237,237));
		m_stRemoveText.SetBkColor(RGB(223,237,237));
		m_stQuarantineText.SetBkColor(RGB(223,237,237));
	}
}


/**********************************************************************************************************                     
*  Function Name  :	OnEnChangeEditSigDescriptionmsg()                                                     
*  Description    : change signature in edit box
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::OnEnChangeEditSigDescriptionmsg()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CJpegDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
	m_btnApply.EnableWindow(TRUE);
	m_btnDefaultSig.EnableWindow(FALSE);

	
}

/***********************************************************************************************
  Function Name  : SendEmailData2Service
  Description    : Function to send data to service
				   dwAddEditDelType : SAVE_EMAIL_ENTRIES
				   dwType			: RECOVER, VIRUSSCAN,SPAMFILTER,CONTENTFILTER,SIGNATURE,REPORTS,MALICIOSSITES
				   csEntry			: # tokenized String
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
bool CISpyEmailScanDlg::SendEmailData2Service(DWORD dwAddEditDelType, DWORD dwType, CString csEntry,bool bEmailScanWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	szPipeData.iMessageInfo = static_cast<int>(dwAddEditDelType);
	szPipeData.dwValue = dwType;
	_tcscpy_s(szPipeData.szFirstParam, csEntry);

	if(!m_objEmailScanCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send RESUME_SCAN data", 0, 0, true, SECONDLEVEL);
		return false;
	}
	if(bEmailScanWait)
	{
		if(!m_objEmailScanCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to read data in CGenXLiveUpSecondDlg::SendLiveUpdateOperation2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if(szPipeData.dwValue != 1)
		{
			return false;
		}
	}
	return true;
}

/***********************************************************************************************
Function Name  : SendEmailData2Service
Description    : Function to send data to service
dwAddEditDelType : ADD_EMAIL_ENTRY,EDIT_EMAIL_ENTRY,DELETE_EMAIL_ENTRY
csEntry			:No tokenization  String
Author Name    : Neha Gharge
Date           : 2 Feb 2016.
//Issue no 1269 and 1270 User can add # as rule. No special symbol is used to tonize word  //Issue no 1269 and 1270 User can add # as rule. No special symbol is used to tonize word
***********************************************************************************************/
bool CISpyEmailScanDlg::SendEmailData2Service(DWORD dwAddEditDelType, DWORD dwType, CString csFirstParam, CString csSecondParam , CString csThirdParam, bool bEmailScanWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = static_cast<int>(dwAddEditDelType);
		szPipeData.dwValue = dwType;
		_tcscpy(szPipeData.szFirstParam, csFirstParam);
		_tcscpy(szPipeData.szSecondParam, csSecondParam);
		_tcscpy(szPipeData.szThirdParam, csThirdParam);

		if (!m_objEmailScanCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send RESUME_SCAN data", 0, 0, true, SECONDLEVEL);
			return false;
		}
		if (bEmailScanWait)
		{
			if (!m_objEmailScanCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to read data in CGenXLiveUpSecondDlg::SendLiveUpdateOperation2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}

			if (szPipeData.dwValue != 1)
			{
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXEmailScanDlg::SendEmailData2Service", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	HideORShowButton()                                                     
*  Description    : hide and show add,edit,delete and apply button
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::HideORShowButton(bool bEnableAdd , bool bEnableEdit , bool bEnableDelete , bool bEnableApply)
{
	m_btnOk.EnableWindow(bEnableAdd);
	m_btnEdit.EnableWindow(bEnableEdit);
	m_btnDelete.EnableWindow(bEnableDelete);
	m_btnApply.EnableWindow(bEnableApply);
}


/**********************************************************************************************************                     
*  Function Name  :	SetRegistrykeyUsingService()                                                     
*  Description    : Set registry key using services
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
bool CISpyEmailScanDlg::SetRegistrykeyUsingService(LPCTSTR SubKey , LPCTSTR lpValueName , DWORD dwType , DWORD dwData, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	
	szPipeData.iMessageInfo = WRITE_REGISTRY;
	szPipeData.dwValue = SZ_DWORD; 
	//szPipeData.hHey = HKEY_LOCAL_MACHINE;

	wcscpy_s(szPipeData.szFirstParam, SubKey);
	wcscpy_s(szPipeData.szSecondParam, lpValueName );
	szPipeData.dwSecondValue = dwData;

	
	if(!m_objEmailScanCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CDataEncryptionDlg : SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bWait)
	{
		if(!m_objEmailScanCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CDataEncryptionDlg : SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnDefaultsig()                                                     
*  Description    : load defualt signature in edit box
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::OnBnClickedBtnDefaultsig()
{
	m_bDefaultSigSelected = true;
	m_edtSigDesMsg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DEF_SIGNATURE"));
	m_btnApply.EnableWindow(true);
	m_btnDefaultSig.EnableWindow(false);
	m_edtSigDesMsg.EnableWindow(false);
}

/**********************************************************************************************************                     
*  Function Name  :	RefreshStrings                                                     
*  Description    :	It will load the string from the .INI files depending upon the language set
*  Author Name    : neha gharge                                                                                         
*  Date           : 5 may 2014
**********************************************************************************************************/

void CISpyEmailScanDlg::RefreshString()
{
	m_stVirusscan_HeaderDes.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_VIRUSSCAN_DES"));
	m_stVirusscan_HeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_VIRUSSCAN_HEADER_NAME"));
	m_stVirusSettingHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_VIRUSSCAN_SETTING_HEADER_NAME"));
	m_stSpamHeaderDes.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SPAM_DES"));
	m_stSpamHeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SPAM_HEADER_NAME"));
	m_stSpamSettingHeadername.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SPAM_SETTING_HEADER_NAME"));
	m_stContentHeaderDes.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CONTENT_DES"));
	m_stContentHeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CONTENT_HEADER_NAME"));
	m_stContentSettingHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CONTENT_SETTING_HEADER_NAME"));
	m_stSignature_HeaderDes.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SIGNATURE_DES"));
	m_stSignture_HeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SIGNATURE_HEADER_NAME"));
	m_stEnableVirusScan.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENABLE_VIRUSSCAN"));
	m_lstVirusScan.InsertColumn(0, L" ", LVCFMT_LEFT, 25);
	m_lstVirusScan.InsertColumn(1, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SENDER_ADDR"), LVCFMT_LEFT, 235);
	m_lstVirusScan.InsertColumn(2, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SUBJECT"), LVCFMT_LEFT, 210);
	m_lstVirusScan.InsertColumn(3, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_STATUS_VIRUSSCAN"), LVCFMT_LEFT, 108);
	m_stEnableSpamFilter.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENABLE_SPAMFILETER"));
	m_lstSpamFilter.InsertColumn(0, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TYPE_SPAMFILTER"), LVCFMT_LEFT, 109);
	m_lstSpamFilter.InsertColumn(1, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_EMAILADDR_SPAMFILTER"), LVCFMT_LEFT, 467);
	m_stEnableContentFilter.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENABLE_CONTETNT"));
	m_lstContentFilter.InsertColumn(0,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ACTION_CONTENT"), LVCFMT_LEFT, 109);
	m_lstContentFilter.InsertColumn(1,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_RULETYPE_CONTENT"),  LVCFMT_LEFT, 227);
	m_lstContentFilter.InsertColumn(2,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_RULE_CONTENT"), LVCFMT_LEFT, 240);
	m_stForExample.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_EXAMPLE_SPAM"));
	m_stAllowVirusPopdlg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_VIRUS_POPUP"));
	m_stQuarantineText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_RADIO_REPAIR_TEXT "));
	m_stRemoveText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_RADIO_QAURANTINE_TEXT"));
	m_stAllowSignatureMsg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENABLE_SIGNATURE"));
	m_btnOk.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_ADD"));
	m_btnDelete.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_DELETE"));
	m_btnApply.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_APPLY"));
	m_btnEdit.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_EDIT"));
	m_DropList.AddString(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE"));
	m_DropList.AddString(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"));
	m_DroplistContentFilter.AddString(theApp.m_objwardwizLangManager.GetString(L"IDS_ATTACHMENT_EXT_IS_CONTENT"));
	m_DroplistContentFilter.AddString(theApp.m_objwardwizLangManager.GetString(L"IDS_MESSAGE_CONTAIN_CONTENT"));
	m_DroplistContentFilter.AddString(theApp.m_objwardwizLangManager.GetString(L"IDS_SUBJECT_CONTAIN_CONTECT"));
	m_btnDefaultSig.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_DEFAULT_SIG"));
}


/**********************************************************************************************************                     
*  Function Name  :	WriteRegistryEntryOfMenuItem()                                                     
*  Description    : Write email scan Enable/disable entry into registry.
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
BOOL CISpyEmailScanDlg::WriteRegistryEntryOfMenuItem(CString strKey, DWORD dwChangeValue)
{
	LPCTSTR SubKey = TEXT("SOFTWARE\\WardWiz Antivirus");
	if(!SetRegistrykeyUsingService(SubKey, strKey, REG_DWORD, dwChangeValue,true))
	{
		AddLogEntry(L"### Error in Setting Registry CSystemTray::MenuItem", 0, 0, true, SECONDLEVEL);
	}
	return TRUE;
}

/**********************************************************************************************************                     
*  Function Name  :	AppyRegistrySetting()                                                     
*  Description    : Apply enable/disable check box for all email scan functionality
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
void CISpyEmailScanDlg::AppyRegistrySetting()
{
	if(!m_bVirusScan)
	{
		int iCheck = m_chkVirusScan.GetCheck();
		if(iCheck == 0)
		{
			m_bEnableVirusPopUp = true;
			m_dwEnableVirusScan = 0;
			WriteRegistryEntryofEmailScan(m_dwEnableVirusScan);
		}
		else
		{
			m_bEnableVirusPopUp = true;
			m_dwEnableVirusScan = 1;
			WriteRegistryEntryofEmailScan(m_dwEnableVirusScan);
		}
		m_bEnableVirusPopUp = false;

	}
	
	if(!m_bSpamFilter)
	{
		int iCheck = m_chkSpamFilter.GetCheck();
		if(iCheck == 0)
		{
			m_dwEnableSpamFilter = 0;
			WriteRegistryEntryofEmailScan(m_dwEnableSpamFilter);
		}
		else
		{
			m_dwEnableSpamFilter = 1;
			WriteRegistryEntryofEmailScan(m_dwEnableSpamFilter);
		}
	}

	if(!m_bContentFilter)
	{
		int iCheck = m_chkContentfilter.GetCheck();
		if(iCheck == 0)
		{
			m_dwEnableContentFilter = 0;
			WriteRegistryEntryofEmailScan(m_dwEnableContentFilter);
		}
		else
		{
			m_dwEnableContentFilter = 1;
			WriteRegistryEntryofEmailScan(m_dwEnableContentFilter);
		}
	}

	if(!m_bSignatureFlag)
	{
		int iCheck = m_chkAllowSignature.GetCheck();
		if(iCheck == 0)
		{
			m_dwEnableSignature = 0;
			WriteRegistryEntryofEmailScan(m_dwEnableSignature);
		}
		else
		{
			m_dwEnableSignature = 1;
			WriteRegistryEntryofEmailScan(m_dwEnableSignature);
		}
	}
}


/**********************************************************************************************************                     
*  Function Name  :	OnCtlColor()                                                     
*  Description    : The framework calls this member function when a child control is about to be drawn.
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 5th may 2014
*********************************************************************************************************/
HBRUSH CISpyEmailScanDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	int ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if( ctrlID == IDC_STATIC_VIRUSSCAN_HEADERNAME      ||
		ctrlID == IDC_STATIC_VIRUSSCAN_HEADERDES   ||
		ctrlID == IDC_STATIC_SPAMFILTER_HEADER_NAME  ||
		ctrlID == IDC_STATIC_SPAM_HEADER_DES||
		ctrlID == IDC_STATIC_CONTENT_HEADERDES ||
		ctrlID == IDC_STATIC_CONTENT_HEADERNAME ||
		ctrlID == IDC_STATIC_SIGNATURE_HEADER_DESCRIPTION ||
		ctrlID == IDC_STATIC_SIGNATURE_HEADER_NAME
	)

	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}


/**********************************************************************************************************
*  Function Name  :	CheckForOutllookExe()
*  Description    : It will check outlook exe default path.
*  SR.NO          :
*  Author Name    : Neha Gharge
*  Date           : 27th Aug, 2015
*********************************************************************************************************/
bool CISpyEmailScanDlg::CheckForOutllookExe()
{
	try
	{
		HKEY hKey;
		LONG ReadReg;
		TCHAR  szOutlookPath[1024];
		DWORD dwvalueSize = 1024;
		DWORD dwType = REG_SZ;
		if (RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\OUTLOOK.EXE"), &hKey) != ERROR_SUCCESS)
		{
			AddLogEntry(L"### Unable to open registry key outlook exe", 0, 0, true, SECONDLEVEL);
			return false;
		}

		ReadReg = RegQueryValueEx(hKey, NULL, NULL, &dwType, (LPBYTE)&szOutlookPath, &dwvalueSize);
		if (ReadReg == ERROR_SUCCESS)
		{
			AddLogEntry(L">>> outlook default path %s", szOutlookPath, 0, true, FIRSTLEVEL);
			if (_tcscmp((LPCTSTR)szOutlookPath, L"") == 0)
			{
				RegCloseKey(hKey);
				return false;
			}
			else
			{
				RegCloseKey(hKey);
				return true;
			}
		}
		else
		{
			RegCloseKey(hKey);
			return false;
		}

		RegCloseKey(hKey);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExEmailScanDlg::CheckForOutllookExe()", 0, 0, true, SECONDLEVEL);
	}
	return true;
}