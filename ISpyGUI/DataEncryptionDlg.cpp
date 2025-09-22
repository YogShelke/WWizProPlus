// DataEncryptionDlg.cpp : implementation file
/****************************************************
*  Program Name: DataEncryptionDlg.cpp                                                                                                    
*  Description: Allows to encrypt & decrypt files
*  Author Name: Vilas & Prajakta                                                                                                      
*  Date Of Creation: 16 Nov 2013
*  Version No: 1.0.0.2
*  WardWiz Sustenance: Issues List
   \\192.168.2.99\WardWiz\Issue Tracking list\WardWiz_Issue tracking sheet.xls
****************************************************/

/****************************************************
HEADER FILES
****************************************************/


#include "stdafx.h"
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"
#include "DataEncryptionDlg.h"
#include "SelectDialog.h"
#define		szpIpsy		"WRDWIZ"
#define		IDR_RESDATA_DES			5000

//typedef DWORD (*ENCRYPT_FILE) (TCHAR *m_szFilePath, DWORD &dwStatus ) ;
//typedef DWORD (*DECRYPT_FILE) (TCHAR *m_szFilePath, DWORD &dwStatus ) ;

//ENCRYPT_FILE	Encrypt_File = NULL ;
//DECRYPT_FILE	Decrypt_File = NULL ;


//typedef DWORD (*ADDREGISTEREDDATA) ( LPBYTE lpResBuffer, DWORD dwResSize, DWORD dwResType, TCHAR *pResName ) ;
typedef DWORD (*GETREGISTEREDDATA) (LPBYTE lpResBuffer, DWORD &dwResDataSize, DWORD dwResType, TCHAR *pResName ) ;

//ADDREGISTEREDDATA	AddRegisteredData = NULL ;
GETREGISTEREDDATA	GetRegisteredData = NULL ;

bool			bRES_DES = false ;
//issue- if i close handle of below thread during shutdown then GUI getting crash.
//case- if any popup from pEncDecThread appear and close this thread then GUI getting crash.
//CWinThread	*pEncDecThread = NULL ;


//DWORD EncryptFileThread( LPVOID lpParam ) ;
//DWORD DecryptFileThread( LPVOID lpParam ) ;
DWORD WINAPI EncryptFileThread(LPVOID lpParam);
DWORD WINAPI DecryptFileThread(LPVOID lpParam);
bool g_bEncryption = false;
TCHAR	m_szProgramData[256] = {0} ;


DWORD Decrypt_File( TCHAR *m_szFilePath, DWORD &dwStatus ) ;

void OpenfileToAddPassword(TCHAR* m_szFilePath);

bool ValidateGivenPassWord(CString csPassword) ;

DWORD Str2Hex( CString const & s );

DWORD	dwEnKey ;
TCHAR	m_szAppData[512] ;
TCHAR	g_szKeyBuff[MAX_PATH];
extern CString	m_csPassword;

//void InitializeVariables() ;
//void IsWow64() ;
//DWORD DecryptData( LPBYTE lpBuffer, DWORD dwSize ) ;
DWORD CreateFileAtAllUserLocation(TCHAR *, LPBYTE , DWORD ) ;
DWORD CopyFileToDestination(TCHAR *pSource, TCHAR *pDest ) ;
DWORD RenameDestination(TCHAR *pDest, TCHAR *pRenamed ) ;



// CDataEncryptionDlg dialog

IMPLEMENT_DYNAMIC(CDataEncryptionDlg, CDialog)

/**********************************************************************************************************                     
*  Function Name  :	CDataEncryptionDlg                                                     
*  Description    :	C'tor
*  Author Name    : Vilas & Prajakta   
*  SR_NO		  :
*  Date           : 16 Nov 2013
**********************************************************************************************************/
CDataEncryptionDlg::CDataEncryptionDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CDataEncryptionDlg::IDD, pParent)
	, m_objPasswordPopUpDlg(pParent)
	, m_Check_Encrypt(0)
	, m_Check_Decrypt(0)
	, m_hRegisterDLL(NULL)
	, m_bIsEnDepInProgress(false)
	, m_hEncDecThread(NULL)
	, m_bManualStop(false)
	, m_bIsLowDiskSpace (false)
	, m_csMsgforLowDiskSpace(L"")
	, m_bIsSystemFolderPathSelected(false)
	, m_bIsPopUpDisplayed(false)
{
	//neha Gharge Open folder issue 1-5-2015
	ZeroMemory(m_szLastFileEncOrDec, sizeof(m_szLastFileEncOrDec));
}

/**********************************************************************************************************                     
*  Function Name  :	~CDataEncryptionDlg                                                     
*  Description    :	Destructor
*  Author Name    : Vilas & Prajakta   
*  SR_NO		  :
*  Date           : 16 Nov 2013
**********************************************************************************************************/
CDataEncryptionDlg::~CDataEncryptionDlg()
{
	if(m_hRegisterDLL != NULL)
	{
		FreeLibrary(m_hRegisterDLL);
		m_hRegisterDLL = NULL;
	}
}

/**********************************************************************************************************                     
*  Function Name  :	DoDataExchange                                                     
*  Description    :	Called by the framework to exchange and validate dialog data.
*  Author Name    : Vilas & Prajakta    
*  SR_NO		  :
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CDataEncryptionDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_FILEPATH, m_Edit_FilePath);
	DDX_Control(pDX, IDC_BUTTON_BROWSE, m_Button_Browse);
	//	DDX_Control(pDX, IDC_BUTTON_STOP, m_Button_Stop);
	DDX_Control(pDX, IDC_BUTTON_START, m_Button_Start);
	DDX_Control(pDX, IDC_PIC_ENCRYPT, m_stHEncrypt);
	DDX_Control(pDX, IDC_STATIC_ENCRYPT, m_stEncrypt);
	DDX_Control(pDX, IDC_STATIC_DECRYPT, m_stDecrypt);
	DDX_Control(pDX, IDC_RADIO_ENCRYPT, m_btnEncrypt);
	DDX_Control(pDX, IDC_RADIO_DECRYPT, m_btnDecrypt);
	DDX_Control(pDX, IDC_STATIC_MSG, m_stMsg);
	//DDX_Control(pDX, IDC_BTN_BACK, m_btnBack);
	//DDX_Control(pDX, IDC_BTN_BACK, m_btnBackDisable);
	//DDX_Control(pDX, IDC_BTN_BACK, m_btnBack);
	DDX_Control(pDX, IDC_BTN_BACK, m_btnBackArrow);
	DDX_Control(pDX, IDC_STATIC_PREVFILESTATUS, m_stPrevFileStatus);
	DDX_Control(pDX, IDC_STATIC_PREVDECFILEPATH, m_stPrevDecFilePath);
	DDX_Control(pDX, IDC_BTN_OPENFOLDER, m_btnOpenfolder);
	DDX_Control(pDX, IDC_STATIC_PREVFOLDERLOCATION, m_stPreviousFolderLocation);
	DDX_Control(pDX, IDC_STATIC_DATA_ENCRPYT_HEADER, m_stEncryptHeaderName);
	DDX_Control(pDX, IDC_STATIC_ENCRYPT_HEADERDES, m_stHeaderDescription);
	DDX_Control(pDX, IDC_LIST_DATA_ENC_LIST, m_lstDataEncDec);
	DDX_Control(pDX, IDC_STATIC_GROUP_ENC_DEC, m_stGrpBoxDataEncDec);
	DDX_Control(pDX, IDC_CHECK_KEEP_ORIGINAL, m_cbKeepOriginalFile);
	DDX_Control(pDX, IDC_STATIC_KEEP_ORIGINAL, m_stKeepOriginal);
	DDX_Control(pDX, IDC_STATIC_SELECT_OPTION, m_stSelectOption);
	DDX_Control(pDX, IDC_EDIT_CRYPT_STATUS, m_edtCryptStatus);
	DDX_Control(pDX, IDC_STATIC_TOTAL_FILE_COUNT_TEXT, m_stTotalFileCountText);
	DDX_Control(pDX, IDC_STATIC_PROCCESSED_FILE_COUNT_TEXT, m_stFileProccessedCount);
	}

/***************************************************************************
* Function Name  : MESSAGE_MAP
* Description    : Handle WM_COMMAND,WM_Messages,user defined message 
				   and notification message from child windows.
*  Author Name    : Vilas & Prajakta    
*  SR_NO		  :
*  Date           : 16 Nov 2013
****************************************************************************/
BEGIN_MESSAGE_MAP(CDataEncryptionDlg, CJpegDialog)
	ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(IDC_RADIO_ENCRYPT, &CDataEncryptionDlg::OnBnClickedRadioEncrypt)
	ON_BN_CLICKED(IDC_BUTTON_START, &CDataEncryptionDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_RADIO_DECRYPT,&CDataEncryptionDlg::OnBnClickedRadioDecrypt)
	ON_BN_CLICKED(IDC_BTN_BACK, &CDataEncryptionDlg::OnBnClickedBtnBack)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CDataEncryptionDlg::OnBnClickedButtonBrowse)
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BTN_OPENFOLDER, &CDataEncryptionDlg::OnBnClickedBtnOpenfolder)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_CHECK_KEEP_ORIGINAL, &CDataEncryptionDlg::OnBnClickedCheckKeepOriginal)
END_MESSAGE_MAP()


// CDataEncryptionDlg message handlers
/**********************************************************************************************************                     
*  Function Name  :	OnInitDialog                                                     
*  Description    :	Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft 
					Foundation Class Library dialog boxes
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
BOOL CDataEncryptionDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window
	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_DATA_ENCRYPTION_BG), _T("JPG")))
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
		m_bIsPopUpDisplayed = false;
	}

	Draw();

	CRect	rect1;
	this->GetClientRect(rect1);

	CRgn RgnRect;
	RgnRect.CreateRectRgn(rect1.left,rect1.top,rect1.right + 5,rect1.bottom);
	this->SetWindowRgn(RgnRect, TRUE);

	//SetWindowPos(NULL, 1, 88, rect1.Width()-5, rect1.Height() - 5, SWP_SHOWWINDOW);
/*
	if(!Load(AfxGetResourceHandle(),MAKEINTRESOURCE(IDR_JPG_ENCRYPTION),_T("JPG")))
	{
		::MessageBox(NULL, L"Failed", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
	}
	Draw();
*/

	m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));

	//m_stEncryptHeaderName.SetWindowPos(&wndTop,rect1.left +20,13,540,31,SWP_NOREDRAW);
	m_stEncryptHeaderName.SetWindowPos(&wndTop, rect1.left + 20, 07, 540, 31, SWP_NOREDRAW);
	m_stEncryptHeaderName.SetTextColor(RGB(24,24,24));
	m_stEncryptHeaderName.SetBkColor(RGB(230, 232, 238));

	//if(theApp.m_dwOSType == WINOS_WIN8 ||theApp.m_dwOSType == WINOS_WIN8_1)
	//{
	//	m_stEncryptHeaderName.SetWindowPos(&wndTop,rect1.left +20,16,540,31,SWP_NOREDRAW);
	//}

	// Issue Add description Neha Gharge 9-3-2015
	m_stHeaderDescription.SetWindowPos(&wndTop, rect1.left + 20, 35, 400, 15, SWP_NOREDRAW);
	m_stHeaderDescription.SetTextColor(RGB(24, 24, 24));
	m_stHeaderDescription.SetBkColor(RGB(230, 232, 238));

	m_bmpDataEncrypt = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_COM_HEADER));
	m_stHEncrypt.SetWindowPos(&wndTop,rect1.left + 6,10,586,45,SWP_NOREDRAW);
	m_stHEncrypt.SetBitmap(m_bmpDataEncrypt);

	m_stMsg.SetWindowPos(&wndTop,rect1.left + 20, 120, 500, 50,SWP_NOREDRAW);
	m_stMsg.ShowWindow(false);

	m_btnEncrypt.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE, IDB_BITMAP_SELECT, IDB_BITMAP_DISABLE, 0, 0, 0, 0, 0, 0);
	//issue - all radio button getting selected , if i drag and drop in radio button
	// resolve by  lalit kumawat 3-20-015
	m_btnEncrypt.SetWindowPos(&wndTop, rect1.left + 18, 100, 20, 20, SWP_NOZORDER);
	//m_btnEncrypt.SetCheck(true);
	m_stEncrypt.SetWindowPos(&wndTop,rect1.left + 43,102,170,20,SWP_NOREDRAW);

	// ******************ISSUE No 392 Neha Gharge 24/5/2014*********************************************/
	m_btnDecrypt.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE, IDB_BITMAP_SELECT, IDB_BITMAP_DISABLE, 0, 0, 0, 0, 0, 0);
	//issue - all radio button getting selected , if i drag and drop in radio button
	// resolve by  lalit kumawat 3-20-015
	m_btnDecrypt.SetWindowPos(&wndTop, rect1.left + 115, 100, 20, 20, SWP_NOZORDER);
	m_stDecrypt.SetWindowPos(&wndTop,rect1.left + 140,102,170,20,SWP_NOREDRAW);

	m_Edit_FilePath.SetWindowPos(&wndTop,rect1.left + 210,100,300,21,SWP_NOREDRAW);
	
	//m_Button_Browse.SetSkin(IDB_BITMAP_Browse75x25,IDB_BITMAP_Browse75x25,IDB_BITMAP_HBrowse75x25,IDB_BITMAP_BROWSE_75_25_DIS,0,0,0,0);
	m_Button_Browse.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_HOVER_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	/*	ISSUE NO - 203 NAME - NITIN K. TIME - 21st May 2014 :: 8 pm*/
	m_Button_Browse.SetWindowPos(&wndTop,rect1.left + 520,100,57,21,SWP_NOREDRAW);
	m_Button_Browse.SetTextColorA(RGB(0,0,0),1,1);
	m_Button_Browse.SetFont(&theApp.m_fontWWTextNormal);

	m_Button_Start.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_HOVER_WHITE_BG, IDB_BITMAP_BTN_HOVER_WHITE_BG, IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_Button_Start.SetWindowPos(&wndTop,rect1.left + 520,150,57,21,SWP_NOREDRAW);
	m_Button_Start.SetTextColorA(RGB(0,0,0),1,1);
	m_Button_Start.SetFont(&theApp.m_fontWWTextNormal);
	//m_btnBack.SetSkin(IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROWDISABLE,0,0,0,0);
	//m_btnBack.SetWindowPos(&wndTop,rect1.left + 21,354,31,32,SWP_SHOWWINDOW);

	//m_btnBackDisable.SetSkin(IDB_BITMAP_BACKARROWDISABLE,IDB_BITMAP_BACKARROWDISABLE,IDB_BITMAP_BACKARROWDISABLE,IDB_BITMAP_BACKARROWDISABLE,0,0,0,0);
	//m_btnBackDisable.SetWindowPos(&wndTop,rect1.left + 21,354,31,32,SWP_SHOWWINDOW);
	//m_btnBackArrow.SetSkin(IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROWDISABLE,0,0,0,0);
	//m_btnBackArrow.SetWindowPos(&wndTop,rect1.left + 21,354,31,32,SWP_NOREDRAW);
	m_btnBackArrow.ShowWindow(SW_HIDE);

	m_stPrevFileStatus.SetWindowPos(&wndTop,rect1.left + 30,260,512,20,SWP_NOREDRAW);
	//m_stPrevFileStatus.SetTextColor(RGB(65,65,67));
	m_stPrevFileStatus.SetFont(&theApp.m_fontWWTextNormal);
	m_stPrevFileStatus.ShowWindow(SW_HIDE);

	//Issue: 0000614  Data Encryption tab not contains the status control, it should show current operation/Error/Sucess 
	//Resolved By :  Nitin K Date : 1st July 2015
	m_edtCryptStatus.SetWindowPos(&wndTop, rect1.left + 10, 377, 550, 20, SWP_NOREDRAW);
	m_edtCryptStatus.setFont(&theApp.m_fontWWTextNormal);
	m_edtCryptStatus.SetBkColor(RGB(70, 70, 70));
	m_edtCryptStatus.SetTextColor(RGB(255, 255, 255));
	m_edtCryptStatus.SetPath(TRUE);
	m_edtCryptStatus.setTextFormat(DT_RIGHT | DT_VCENTER);
	GetDlgItem(IDC_EDIT_CRYPT_STATUS)->ModifyStyle(0, WS_DISABLED);

	m_stPrevDecFilePath.SetWindowPos(&wndTop,rect1.left + 30,280,350,40,SWP_NOREDRAW);
	//m_stPrevDecFilePath.SetTextColor(RGB(65,65,67));
	

	m_stPreviousFolderLocation.SetWindowPos(&wndTop,rect1.left + 30, 320, 350, 40,SWP_NOREDRAW);
	//m_stPreviousFolderLocation.SetTextColor(RGB(65,65,67));
	m_stPreviousFolderLocation.ShowWindow(SW_HIDE);

	m_btnOpenfolder.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_103x22_HOVER_WHITE_BG,IDB_BITMAP_BTN_103x22_HOVER_WHITE_BG,IDB_BITMAP_BTN_103x22_NORMAL_WHITE_BG,IDB_BITMAP_BTN_103x22_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnOpenfolder.SetWindowPos(&wndTop,rect1.left + 413, 280, 103,22,SWP_NOREDRAW);
	m_btnOpenfolder.SetTextColorA(BLACK,1,1);
	InitializeDataEncryptionVaraibles() ;
	m_btnOpenfolder.ShowWindow(SW_HIDE);

	// Issue Add description Neha Gharge 9-3-2015
	//Setting fonts here...
	m_stEncryptHeaderName.SetFont(&theApp.m_fontWWTextSmallTitle);//(&theApp.m_fontWWTextTitle);
	m_stHeaderDescription.SetFont(&theApp.m_fontWWTextSubTitleDescription);
	m_stMsg.SetFont(&theApp.m_fontWWTextNormal);
	m_stEncrypt.SetFont(&theApp.m_fontWWTextNormal);
	m_stDecrypt.SetFont(&theApp.m_fontWWTextNormal);
	//m_Edit_FilePath.SetFont(&theApp.m_fontWWTextNormal);
	m_stPrevDecFilePath.SetFont(&theApp.m_fontWWTextNormal);
	m_stPreviousFolderLocation.SetFont(&theApp.m_fontWWTextNormal);

	//Added By Nitin Kolapkar
	//Added for new Implementation of Data Enc/Dec
	m_lstDataEncDec.InsertColumn(0, theApp.m_objwardwizLangManager.GetString(L"IDS_LSTCTRL_FILEPATH"), LVCFMT_LEFT, 450);
	m_lstDataEncDec.InsertColumn(1, theApp.m_objwardwizLangManager.GetString(L"IDS_LSTCTRL_ACTION"), LVCFMT_LEFT, 110);
	m_lstDataEncDec.SetTextColor(RGB(100, 100, 100));
	ListView_SetExtendedListViewStyle(m_lstDataEncDec.m_hWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);
	m_lstDataEncDec.SetWindowPos(&wndTop, rect1.left + 15, 180, 570, 180, SWP_SHOWWINDOW);

	m_cbKeepOriginalFile.SetWindowPos(&wndTop, rect1.left + 380, 154, 14, 14, SWP_NOREDRAW);
	//Issue: " Select option " text was not visible properly
	//Added By Nitin K Date 6th July 2015
	m_stSelectOption.SetWindowPos(&wndTop, rect1.left + 30, 80, 80, 20, SWP_NOREDRAW);
	m_stSelectOption.SetFont(&theApp.m_fontWWTextNormal);
	m_stSelectOption.SetTextColor(RGB(24, 24, 24));
	m_stSelectOption.SetBkColor(RGB(255, 255, 255));
	m_stSelectOption.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_SELECT_OPTION"));
	//Issue: after encrypt/decrypt it list control's right upper corner getting cut
	//resolved By : Nitin K Date 7th July 2015
	m_stKeepOriginal.SetWindowPos(&wndTop, rect1.left + 400, 153, 100, 25, SWP_NOREDRAW);
	m_stKeepOriginal.SetTextColor(RGB(24, 24, 24));
	m_stKeepOriginal.SetBkColor(RGB(255, 255, 255));
	m_stKeepOriginal.SetFont(&theApp.m_fontWWTextNormal);
	m_stKeepOriginal.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_KEEP_ORIGINAL"));
	//Issue 0000621 Keep original not working
	//Issue resolved by :  Nitin K Date 4th July 2015
	DWORD dwCrKeepOrg = 0;
	if (theApp.m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Wardwiz Antivirus", L"dwCrKeepOrg", dwCrKeepOrg) != 0x00)
	{
		AddLogEntry(L"### Failed to get Registry Entry for dwCrKeepOrg in OnInitDialog", 0, 0, true, SECONDLEVEL);;
	}
	if (dwCrKeepOrg == 1)
	{
		theApp.m_bCrKeepOrg = true;
		m_cbKeepOriginalFile.SetCheck(BST_CHECKED);
	}
	else
	{
		theApp.m_bCrKeepOrg = false;
		m_cbKeepOriginalFile.SetCheck(BST_UNCHECKED);
	}

	//Issue: " Select option " text was not visible properly
	//Added By Nitin K Date 6th July 2015
	m_stGrpBoxDataEncDec.SetWindowPos(&wndTop, rect1.left + 15, 80, 570, 60, SWP_NOREDRAW);

	//Implementation : Adding Total file count and Processed file count
	//Added By : Nitin K Date: 2nd July 2015
	m_stTotalFileCountText.SetWindowPos(&wndTop, rect1.left + 15, 153, 150, 25, SWP_NOREDRAW);
	m_stTotalFileCountText.SetTextColor(RGB(24, 24, 24));
	m_stTotalFileCountText.SetBkColor(RGB(255, 255, 255));
	m_stTotalFileCountText.SetFont(&theApp.m_fontWWTextNormal);

	m_stFileProccessedCount.SetWindowPos(&wndTop, rect1.left + 175, 153, 150, 25, SWP_NOREDRAW);
	m_stFileProccessedCount.SetTextColor(RGB(24, 24, 24));
	m_stFileProccessedCount.SetBkColor(RGB(255, 255, 255));
	m_stFileProccessedCount.SetFont(&theApp.m_fontWWTextNormal);

	RefreshStrings();

	memset(m_szProgramData, 0x00, 256*sizeof(TCHAR) ) ;
	GetEnvironmentVariable(L"ALLUSERSPROFILE", m_szProgramData, 255 ) ;
	wcscat_s(m_szProgramData, L"\\Wardwiz Antivirus");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/**********************************************************************************************************                     
*  Function Name  :	ResetControl                                                     
*  Description    :	Function to reset control state
*  Author Name    : Nitin K  
*  SR_NO		  :
*  Date           : 21 May 2014
*  ISSUE NO - 205 NAME - NITIN K. TIME - 21st May 2014 :: 8 pm
**********************************************************************************************************/
void CDataEncryptionDlg::ResetControl(bool bEnable)
{
	// Issue Number 297 Rajil Y. 22/05/2014
	if (!m_bIsEnDepInProgress)
	{
		m_Edit_FilePath.SetWindowTextW(L"");
		m_btnEncrypt.SetCheck(bEnable);
		m_btnDecrypt.SetCheck(!bEnable);

		//Issue Resolved: 1188
		if (bEnable)
		{
			m_btnEncrypt.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE, IDB_BITMAP_SELECT, IDB_BITMAP_DISABLE, IDB_BITMAP_DISABLE, 0, 0, 0, 0, 0);
			m_btnDecrypt.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE, IDB_BITMAP_DISABLE, IDB_BITMAP_DISABLE, IDB_BITMAP_DISABLE, 0, 0, 0, 0, 0);
		}
		else
		{
			m_btnDecrypt.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE, IDB_BITMAP_SELECT, IDB_BITMAP_DISABLE, IDB_BITMAP_DISABLE, 0, 0, 0, 0, 0);
			m_btnEncrypt.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE, IDB_BITMAP_DISABLE, IDB_BITMAP_DISABLE, IDB_BITMAP_DISABLE, 0, 0, 0, 0, 0);
		}

		//Added By Nitin Kolapkar
		//Added for new Implementation of Data Enc/Dec
		m_cbKeepOriginalFile.EnableWindow(true);
//		m_cbKeepOriginalFile.SetCheck(BST_CHECKED);
		m_Button_Start.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENCRYPT"));
		//Issue: 0000614  Data Encryption tab not contains the status control, it should show current operation/Error/Sucess 
		//Resolved By :  Nitin K Date : 1st July 2015
		CString csStatusMsg;
		csStatusMsg.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_EDIT_STATUS_ENC")
			, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENCRYPT"));
		m_edtCryptStatus.SetWindowTextW(csStatusMsg);
		//issue total file count and file processed count getting set to 0, when we move on tab control after dataEncryption operation success
		// resolved by lalit kumawat 8-11-2015
		//m_stTotalFileCountText.SetWindowTextW(L"Total File : 0");
		//m_stFileProccessedCount.SetWindowTextW(L"Processed File : 0");
		Invalidate();
	}
	bEncryption = true;
}

/**********************************************************************************************************                     
*  Function Name  :	OnCtlColor                                                     
*  Description    :	The framework calls this member function when a child control is about to be drawn.
*  Author Name    : Vilas & Prajakta   
*  SR_NO		  :
*  Date           : 16 Nov 2013
**********************************************************************************************************/
HBRUSH CDataEncryptionDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if( ctrlID == IDC_STATIC_MSG      ||
		ctrlID == IDC_STATIC_ENCRYPT   ||
		ctrlID == IDC_STATIC_DECRYPT  ||
		ctrlID == IDC_STATIC_PREVFILESTATUS ||
		ctrlID == IDC_STATIC_PREVDECFILEPATH ||
		ctrlID == IDC_STATIC_PREVFOLDERLOCATION ||
		ctrlID == IDC_STATIC_DATA_ENCRPYT_HEADER )

	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	} return hbr;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedRadioDecrypt                                                     
*  Description    :	Allows selection of decrypt button to decrypt a file 
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CDataEncryptionDlg::OnBnClickedRadioDecrypt()
{
	m_btnEncrypt.SetCheck(false);
	m_Edit_FilePath.SetWindowTextW(L"");
	//Added By Nitin Kolapkar
	//Added for new Implementation of Data Enc/Dec
	m_cbKeepOriginalFile.EnableWindow(false);
	m_cbKeepOriginalFile.SetCheck(0);
	m_btnDecrypt.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE, IDB_BITMAP_SELECT, IDB_BITMAP_DISABLE, IDB_BITMAP_SELECT, 0, 0, 0, 0, 0);
	m_Button_Start.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DECRYPT"));
	//Issue: 0000614  Data Encryption tab not contains the status control, it should show current operation/Error/Sucess 
	//Resolved By :  Nitin K Date : 1st July 2015
	CString csStatusMsg;
	csStatusMsg.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_EDIT_STATUS_DEC")
		, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DECRYPT"));
	m_edtCryptStatus.SetWindowTextW(csStatusMsg);
	bEncryption = false ;
	
}

/**********************************************************************************************************                     
*  Function Name  :	FileExists                                                     
*  Description    :	Check whether file exists or not 
*  SR.N0		  : 
*  Author Name    : Ramkrushna                                                                                         
*  Date           : 27 Nov 2013
**********************************************************************************************************/
DWORD CDataEncryptionDlg::FileExists(const TCHAR *fileName)
{
	
	AddLogEntry(L">>> DataEncryption : Checking existence of file %s", fileName, 0, true, ZEROLEVEL);
	HANDLE hFile = CreateFile(	fileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
	if( hFile == INVALID_HANDLE_VALUE )
	{
		DWORD dwLastError = GetLastError();
		if( dwLastError == ERROR_SHARING_VIOLATION)
		{
			AddLogEntry(L"### DataEncryption : Error in opening file %s", fileName, 0, true, SECONDLEVEL);
			return 0x01;
		}
		return 0x00;
	}
	CloseHandle(hFile);
	return 0x02;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonStart                                                     
*  Description    :	Accepts a file for encryption/decryption 
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CDataEncryptionDlg::OnBnClickedButtonStart()
{
	CString csChkText;
	m_Button_Start.GetWindowText( csChkText ) ;

	TCHAR	szTemp[512] = {0} ;
	TCHAR	*pName = NULL ;

	//Ram, Issue No: 0001216, Resolved
	//Extra check has been added to get number of days left.
	if (theApp.m_dwDaysLeft == 0)
	{
		theApp.GetDaysLeft();
	}

	//Check here for Evaluation period expired
	if (theApp.m_dwDaysLeft == 0)
	{
		if (!theApp.ShowEvaluationExpiredMsg())
		{
			theApp.GetDaysLeft();
			return;
		}
	}

	//Varada Ikhar, Date:17-03-2015, 
	//Issue:User updated the product and does not restarted, and started a scan, then it should show message as 
	//"Product updated, restart is required to take a effect."
	if (!theApp.ShowRestartMsgOnProductUpdate())
	{
		return;
	}

	AddLogEntry(L">>> DataEnryption : Encryption/Decryption started", 0, 0, true, FIRSTLEVEL);
	
	m_Edit_FilePath.GetWindowTextW(m_szFilePath, 2047);

	if( !m_szFilePath[0] )
	{
		//Issue: 0000614  Data Encryption tab not contains the status control, it should show current operation/Error/Sucess 
		//Resolved By :  Nitin K Date : 1st July 2015
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_VALID_SELECTION "), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		m_bIsPopUpDisplayed = false;
		Invalidate();
		return ;
	}

	//Issue Resolved: 0001111
	if (IsPathBelongsToWardWizDir(m_szFilePath))
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_WARDWIZ_OWN_FILEPATH"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		m_bIsPopUpDisplayed = false;
		Invalidate();
		return;
	}

	//Issue No : 1155 Issue with data encryption.If we click "cancel" on password pop-up the UI is getting closed.
	m_Button_Start.EnableWindow(false);
	//Issue Resolved: 0001110
	if (IsPathBelongsFromRemovableDrive(m_szFilePath))
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_ENCDEC_REM_DEVPATH"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		m_bIsPopUpDisplayed = false;
		Invalidate();
	}
	m_Button_Start.EnableWindow(true);


	GetModuleFileName( NULL, szTemp, 511 ) ;
	pName = wcsrchr( szTemp, '\\' ) ;
	if( !pName )
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_WRDWIZAV_PATH_NOT_FOUNT"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		m_bIsPopUpDisplayed = false;
		Invalidate();
		return;
	}


	DWORD			i = 0x00 ;
	DWORD			iPassLen = 0x00 ;
	DWORD			dwSize = 0x00 ;
	unsigned char	chPass = 0x00 ;
	BYTE			szPassWord[0x19] = {0} ; //Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12
	TCHAR			szPass[0x20] = {0} ;

	CString			strEncryptPass("") ;
	CString			strDecryptPass("") ;


	/*GetRegisteredData(szPassWord, dwSize, IDR_RESDATA_DES, TEXT("WRDWIZRESDES") ) ;
	if( szPassWord[0] && dwSize)
	{
		while( szPassWord[i] != 0x00 )
		{
			szPass[i] = szPassWord[i] ;
			i++ ;
		}
		m_csPassword.Format( TEXT("%s"), szPass ) ;
		strEncryptPass = m_csPassword ;
		bRES_DES = true ;
	}*/
	
	/*	ISSUE NO - 83 NAME - NITIN K. TIME - 16th June 2014 */
	//if(!bEncryption)
	//{
	//	if( !strEncryptPass.GetLength() )
	//	{
	//		//MessageBox(L"Encryption Password is not given and trying to decrypt file.\nPlease first encrypt file with password.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
	//		//Varada Ikhar, Date:04-04-2015, Issue: 00045 : Try decrypting unencrypted file->pop up message displayed as "Encryption file is not given and trying to decrypt file. Please first encrypt file with password"->Message should be changed.
	//		// Actual Issue: If a file a encrypted and product is uninstalled, then if freshly a new setup is installed, then while trying to decrypt that already existing encrypted file this message pop up. and this is changed as below.
	//		MessageBox(L"Please provide Encryption Password to decrypt the file.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
	//		return ;
	//	}
	//}

	/*if( (!m_csPassword.GetLength()) || (!bEncryption) )
	{*/
	if (!(csChkText == theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_STOP")))
	{
		m_csPassword = "";
		int iKeepOriginal = 0;
		int iDataOpr = 0;
		//CISpyPasswordPopUpDlg  objPasswordPopUpMsgDlgOpt;



		if (m_cbKeepOriginalFile.GetCheck() == BST_CHECKED)
		{
			iKeepOriginal = KEEPORIGINAL;
		}
		else
		{
			iKeepOriginal = DELETEORIGINAL;
		}

		if (m_btnEncrypt.GetCheck())
		{		
			m_objPasswordPopUpDlg.m_bEncryptionOption = true;
			g_bEncryption = m_objPasswordPopUpDlg.m_bEncryptionOption ? true : false;
			iDataOpr = ENCRYPTION;
			m_btnEncrypt.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE, IDB_BITMAP_SELECT, IDB_BITMAP_DISABLE, IDB_BITMAP_SELECT, 0, 0, 0, 0, 0);
			m_btnDecrypt.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE, IDB_BITMAP_DISABLE, IDB_BITMAP_DISABLE, IDB_BITMAP_DISABLE, 0, 0, 0, 0, 0);
		}
		if (m_btnDecrypt.GetCheck())
		{
			iKeepOriginal = DELETEORIGINAL;
			m_objPasswordPopUpDlg.m_bEncryptionOption = false;
			g_bEncryption = m_objPasswordPopUpDlg.m_bEncryptionOption ? true : false;
			iDataOpr = DECRYPTION;
			m_btnDecrypt.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE, IDB_BITMAP_SELECT, IDB_BITMAP_DISABLE, IDB_BITMAP_SELECT, 0, 0, 0, 0, 0);
			m_btnEncrypt.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE, IDB_BITMAP_DISABLE, IDB_BITMAP_DISABLE, IDB_BITMAP_DISABLE, 0, 0, 0, 0, 0);
		}

		//Issue No: 1020 Neha Gharge.
		// When browse, Password popup is open , User double click on encrypted file or right click any file for encryption
		//or decryption processes get started. So now we avoid that ..When one process is gng on we are not allowing other 
		//process from right click or double click on encrypted files.
		theApp.m_bDialogsOpenInDataEnc = true;

		if (!ValidatePassword(m_csPassword))
		{
			theApp.m_bDataCryptOpr = false;
			//dwEnKey = Str2Hex(m_csPassword ) ;
		}
		else
		{
			if (theApp.m_bDataCryptOpr == true)
			{
				PostQuitMessage(0);
				//return;
			}
			theApp.m_bDialogsOpenInDataEnc = false;
			//Varada Ikhar, Date:04-03-2015, Issue:In Data Encryption->Browse any file->Dialog box appears to enter password->without giving any password->click ok-> check log file (AVUI)->sentence should be 'entered password is invalid'
			AddLogEntry(L"### DataEntryption : Entered password is invalid.", 0, 0, true, SECONDLEVEL);
			return;
		}
		//}
		//Implementation : Adding Total file count and Processed file count
		//Added By : Nitin K Date: 2nd July 2015
		theApp.m_dwCryptFileCount = 0;
		m_lstDataEncDec.DeleteAllItems();
		m_stTotalFileCountText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_TOTAL_FILES_COUNT"));
		m_stFileProccessedCount.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_PROCESSED_FILES_COUNT"));
		if (!SendDataEncryptionOperation2Service(SERVICE_SERVER, DATA_ENC_DEC_OPERATIONS, m_szFilePath, m_csPassword, iKeepOriginal, iDataOpr, true))
		{
			AddLogEntry(L"### Dataencryption : Error in DATA_ENC_DEC_OPERATIONS in CDataencryption:SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
			//MessageBox(L"Directory not present", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		}
		theApp.m_bDialogsOpenInDataEnc = false;
		m_bIsEnDepInProgress = true;
		m_Button_Start.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_STOP")); 
		m_Button_Start.EnableWindow(false);
		m_btnEncrypt.EnableWindow(false);
		//m_btnEncrypt.SetSkin()
		m_btnDecrypt.EnableWindow(false);
		m_Edit_FilePath.EnableWindow(false);
		m_Button_Browse.EnableWindow(false);
		m_cbKeepOriginalFile.EnableWindow(false);
	}
	else if (csChkText == theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_STOP"))
	{
		m_bManualStop = true;
		//Issue: UI getting unresponsive andCrypt exe not getting exited
		//Resolved By : Nitin K Date: 10th July 2015
		DWORD dwManualStop = 1;
		//if (!SendDataEncryptionOperation2Service(WWIZ_CRYPT_SERVER, STOP_CRYPT_OPR, m_szFilePath, m_csPassword, 0, 0, true))
		//{
		//	AddLogEntry(L"### Dataencryption : Error in WWIZ_CRYPT_SERVER in CDataencryption:SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
		//}
		//StopCryptOperations();
		
		ShutDownEncryptDecrypt(dwManualStop);
		// resolved by lalit kumawat 6-29-2015
		// after once we click on stop encryption button and say no now stop button getting disable
	//	m_Button_Start.EnableWindow(false);
	}
	//if( !bEncryption )
	//{

	//	if( !strEncryptPass.GetLength() )
	//	{
	//		//MessageBox(L"Encryption password is not given and trying to decrypt file.\nPlease first encrypt file with password.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
	//		//Varada Ikhar, Date:04-04-2015, Issue : 00045 : Try decrypting unencrypted file->pop up message displayed as "Encryption file is not given and trying to decrypt file. Please first encrypt file with password"->Message should be changed.
	//		// Actual Issue: If a file a encrypted and product is uninstalled, then if freshly a new setup is installed, then while trying to decrypt that already existing encrypted file this message pop up. and this is changed as below.
	//		MessageBox(L"Please provide Encryption Password to decrypt the file.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
	//		return ;
	//	}

	//	if( strEncryptPass.Compare(m_csPassword) != 0 )
	//	{
	//		MessageBox(L"Incorrect password\nPlease try again.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
	//		return ;
	//	}
	//}

	//if( !m_csPassword.GetLength() )
	//{
	//	MessageBox(L"Please enter valid password.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
	//	return ;
	//}

	//dwEnKey = Str2Hex( m_csPassword ) ;

	//i = 0x00 ;
	//iPassLen = m_csPassword.GetLength() ;
	//if( !dwSize ) 
	//{
	//	BYTE	szPass[0x19] = {0} ; //Name:Varada Ikhar, Date:07/01/2015, Version:1.8.3.11, Issue No:9, Description: Change password length max:24, min:12

	//	for(i=0; i<iPassLen; i++ )
	//	{
	//		chPass = m_csPassword.GetAt( i ) ;
	//		szPass[i] = chPass ;
	//	}

	//	if(!SendRegisteredData2Service(ADD_REGISTRATION_DATA, szPass, iPassLen, IDR_RESDATA_DES, TEXT("WRDWIZRESDES")))
	//	{
	//		AddLogEntry(L"### Dataencryption : Error in ADD_REGISTRATION_DATA in CDataencryption:SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
	//		MessageBox(L"WRDWIZREGISTERDATA.DLL not accessible.\nPlease try again.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
	//		return ;
	//	}
	//}

	//m_Edit_FilePath.GetWindowTextW( m_szFilePath, 511 ) ;

	//if( !m_szFilePath[0] )
	//{
	//	MessageBox(L"Please select a valid file", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
	//	return ;
	//}

	//DWORD dwReturn = FileExists(m_szFilePath);
	//if( dwReturn == 0x00)
	//{
	//	MessageBox(L"Invalid file selected, Please select a valid file", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
	//	return ;
	//}
	//else if( dwReturn == 0x01)
	//{
	//	MessageBox(L"ACCESS DENIED: Given file is being used by another program", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
	//	return;
	//}

	//if(!SendDataEncryptionOperation2Service(FILE_OPERATIONS, m_szProgramData, L"", 3, false))
	//{
	//	AddLogEntry(L"### Dataencryption : Error in FILE_OPERATIONS in CDataencryption:SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
	//	//MessageBox(L"Directory not present", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
	//}

	//if(csChkText == "Start")
	//{	
	//	m_btnBackArrow.EnableWindow(false);
	//	//Issue : In data encryption if we try to encrypt file of larger size and meanwhile if we try to encrypt other file,
	//	//after encryption the file which was encrypted second time takes the file size of first file.
	//	//Resolved By Nitin K. 03 MARCH 2015
	//	m_Button_Browse.EnableWindow(false);
	//	m_Button_Start.EnableWindow(false);
	//	m_Button_Start.SetWindowText(L"Stop") ;
	//	AddLogEntry(L">>> Encryption/Decryption thread started", 0, 0, true, ZEROLEVEL);

	//	//issue- if i close handle of below thread during shutdown then GUI getting crash.
	//	//case- if any popup from pEncDecThread appear and close this thread then GUI getting crash.
	//	if( bEncryption )
	//		//pEncDecThread = AfxBeginThread( (AFX_THREADPROC) EncryptFileThread, this ) ;
	//		m_hEncDecThread = ::CreateThread(NULL, 0, EncryptFileThread, (LPVOID)this, 0, 0);
	//	else
	//	//	pEncDecThread = AfxBeginThread( (AFX_THREADPROC) DecryptFileThread, this ) ;
	//		m_hEncDecThread = ::CreateThread(NULL, 0, DecryptFileThread, (LPVOID)this, 0, 0);

	//	Sleep( 100 ) ;
	//	
	//}
	//else if(csChkText == "Stop")
	//{
	//	AddLogEntry(L">>> Encryption/Decryption thread stopped", 0, 0, true, ZEROLEVEL);
	//	if (m_hEncDecThread)
	//	{
	//		//While encrypting file->start encryption if we click on "Stop" button the message should be "Data Encryption is in progress.Do you want to stop?"
	//		//Niranjan Deshak. - 28/02/2015
	//		// MessageBox( TEXT("Data Encryption is in progress.Do you want to stop?"), 
	//		//TEXT("WardWiz"), MB_YESNO | MB_ICONINFORMATION) == IDYES )
	//		if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DENCRYPT_MSG_STOP"),
	//			TEXT("WardWiz"), MB_YESNO | MB_ICONINFORMATION) == IDYES)
	//		{
	//			SuspendThread(m_hEncDecThread);
	//			TerminateThread(m_hEncDecThread, 0x00);
	//			m_hEncDecThread = NULL;
	//		}
	//	}

	//	m_Button_Start.SetWindowText(L"Start");
	//}
}

/**********************************************************************************************************                     
*  Function Name  :	EncryptFileThread                                                     
*  Description    : 1. Call to "Encrypt_File" function that accepts filename to encrypt
					2. Foll. conditions checked:
					   -> Is file already encrypted
					   -> Is filesize = 0
					   -> Is file encrypted successfully or not
					3. Displays last encrypted file name & location.
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD WINAPI EncryptFileThread(LPVOID lpParam)
{
//	CDataEncryptionDlg *pDlg = (CDataEncryptionDlg* ) lpParam ;
//
//	if(!pDlg)
//	{
//		return 0;
//	}
//
//	CString csFilePath(pDlg->m_szFilePath);
//
//	// Issue : If we changed any file extension to .WWIZ->In data encryption->browse that file
//	//->click on start->pop-up appears as "Selected file is already encrypted using WardWiz".
//	// Neha Gharge 9-3-2015 
//
//	//if(csFilePath.Right(5) == L".WWIZ")
//	//{
//	//	pDlg->MessageBox(L"Selected file is already encrypted using WardWiz.\n\nPlease select valid file", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
//	//	pDlg->m_Button_Start.SetWindowText(L"Start");
//	//	pDlg->m_btnBackArrow.EnableWindow(true);
//	//	pDlg->m_Button_Browse.EnableWindow(true);
//	//	pDlg->m_Button_Start.EnableWindow(true);
//	//	return 0;
//	//}
//	HANDLE hFile = NULL;
//	hFile = CreateFile(	pDlg->m_szFilePath, GENERIC_READ, 0, NULL,
//								OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
//	if( hFile == INVALID_HANDLE_VALUE )
//	{
//		return 0;
//	}
//	pDlg->m_bIsEnDepInProgress = true;
//	/*To check if file is actually encrypted by checking existence of sig "WARDWIZ" in case if user removes extension .WWIZ after 1st time encryption &
//	gives same encrypted file to encrypt for 2nd time */
//	if(theApp.IsFileAlreadyEncrypted(hFile))
//	{ 
//		pDlg->m_bIsEnDepInProgress = false;
//		pDlg->MessageBox(L"Selected file is already encrypted using WardWiz.\n\nPlease select valid file", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
//		pDlg->m_Button_Start.SetWindowText(L"Start");
//		pDlg->m_btnBackArrow.EnableWindow(true);
//		pDlg->m_Button_Browse.EnableWindow(true);
//		pDlg->m_Button_Start.EnableWindow(true);
//		CloseHandle(hFile);	
//		return 0;
//	}
//	CloseHandle(hFile);	
//
//	DWORD	dwStatus = 0x00 ;
//	TCHAR szDestPath[MAX_PATH] = {0};
//	DWORD dwRet = 0;
//	/*	ISSUE NO - 669 NAME - NITIN K. TIME - 12th June 2014 */
//	//Issue : In data encryption if we try to encrypt file of larger size and meanwhile if we try to encrypt other file,
//	//after encryption the file which was encrypted second time takes the file size of first file.
//	//Resolved By Nitin K. 03 MARCH 2015
//	pDlg->m_Button_Browse.EnableWindow(false);
//	pDlg->m_Button_Start.EnableWindow(false);
//	dwRet= pDlg->Encrypt_File(pDlg->m_szFilePath, szDestPath, dwStatus ) ;
//	
//	if( dwRet == 3)
//	{
//		pDlg->m_bIsEnDepInProgress = false;
//		pDlg->MessageBox(L"No Data for encryption(file size : 0)", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
//		pDlg->m_Button_Start.SetWindowText(L"Start");
//		//Issue : In data encryption if we try to encrypt file of larger size and meanwhile if we try to encrypt other file,
//		//after encryption the file which was encrypted second time takes the file size of first file.
//		//Resolved By Nitin K. 03 MARCH 2015
//		pDlg->m_Button_Browse.EnableWindow(true);
//		pDlg->m_Button_Start.EnableWindow(true);
//		pDlg->m_btnBackArrow.EnableWindow(true);
//		
//		return 1;
//	}
//	else if(dwRet == 9)
//	{
//		pDlg->m_bIsEnDepInProgress = false;
//		pDlg->m_Button_Start.SetWindowText(L"Start");
//		//Issue : In data encryption if we try to encrypt file of larger size and meanwhile if we try to encrypt other file,
//		//after encryption the file which was encrypted second time takes the file size of first file.
//		//Resolved By Nitin K. 03 MARCH 2015
//		pDlg->m_Button_Browse.EnableWindow(true);
//		pDlg->m_Button_Start.EnableWindow(true);
//		pDlg->m_btnBackArrow.EnableWindow(true);
//		
//		return 1;
//	}
//	else if(dwRet == 0xA)
//	{
//		pDlg->m_bIsEnDepInProgress = false;
//		pDlg->MessageBox(L"Selected file is already encrypted.\n\nPlease select valid file", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
//		pDlg->m_Button_Start.SetWindowText(L"Start");
//		//Issue : In data encryption if we try to encrypt file of larger size and meanwhile if we try to encrypt other file,
//		//after encryption the file which was encrypted second time takes the file size of first file.
//		//Resolved By Nitin K. 03 MARCH 2015
//		pDlg->m_btnBackArrow.EnableWindow(true);
//		pDlg->m_Button_Browse.EnableWindow(true);
//		pDlg->m_Button_Start.EnableWindow(true);
//		
//		return 1;
//	}
//	else if( dwRet )
//	{
//		pDlg->m_bIsEnDepInProgress = false;
//		pDlg->MessageBox(L"File encryption failed", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
//		pDlg->m_Button_Start.SetWindowText(L"Start");
//		//Issue : In data encryption if we try to encrypt file of larger size and meanwhile if we try to encrypt other file,
//		//after encryption the file which was encrypted second time takes the file size of first file.
//		//Resolved By Nitin K. 03 MARCH 2015
//		pDlg->m_Button_Browse.EnableWindow(true);
//		pDlg->m_Button_Start.EnableWindow(true);
//		pDlg->m_btnBackArrow.EnableWindow(true);
//		
//		return 1;
//	}
//	else
//	{
//		//Send Source and destination path to service
//		CString csSrcFilePath(pDlg->m_szFilePath);
//		int iPos = csSrcFilePath.ReverseFind(L'\\');
//		CString csSrcFileName = csSrcFilePath.Right(csSrcFilePath.GetLength() - (iPos + 1));
//		CString csDescFileName = L"", csDestFolderPath = L"" , csDestFilePath = L"";
//		csDescFileName.AppendFormat(L"%s%s",csSrcFileName,L".WWIZ");
//		CString csFolderPath = csSrcFilePath.Left(iPos);
//		CString scFolderPathTemp = NULL;
//		scFolderPathTemp = csFolderPath;
//		csFolderPath.Append(L"\\"+ csDescFileName);
//		csDestFilePath = csFolderPath;
//		//Issue : Drive 'WardWiz Antivirus' folder / system32-Drivers-WrdWizscanner WrdWizsecure64 ->encryption & 
//		//decryption shouldn't happen.
//		//Resolved by :  Nitin K. Date : 09th March 2015
//		if (pDlg->CheckForWardWizSystemFile(scFolderPathTemp, csSrcFileName))
//		{
//			pDlg->m_bIsEnDepInProgress = false;
//			pDlg->MessageBox(L"WardWiz system files can not be encrypted.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
//			pDlg->m_Button_Start.SetWindowText(L"Start");
//			pDlg->m_btnBackArrow.EnableWindow(true);
//			pDlg->m_Button_Browse.EnableWindow(true);
//			pDlg->m_Button_Start.EnableWindow(true);
//			pDlg->m_Edit_FilePath.SetWindowTextW(L"");
//			
//			return 0;
//		}
//		if(pDlg->SendDataEncryptionOperation2Service(FILE_OPERATIONS, szDestPath, csDestFilePath, 1, true))
//		{
//			//Issue No:65 Issue Desc: Data Encryption the original file ( input file ) should get removed and new file with .WWIZ
//			//Resolved by :	Divya S.
//			pDlg->m_bIsEnDepInProgress = false;
//			if(pDlg->MessageBox(L"File encrypted successfully. \nDo you want to keep the original file?", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_YESNO|MB_ICONQUESTION) == IDNO)
//			{
//				if(!pDlg->SendDataEncryptionOperation2Service(FILE_OPERATIONS, pDlg->m_szFilePath, L"", 2, true))
//				{
//					AddLogEntry(L"### DataEncryption : Error in FILE_OPERATIONS in CDataEncryption:SendDataEncryptionOperation2Service. Cannot Delete file", 0, 0, true, SECONDLEVEL);
//					pDlg->MessageBox(L"File cannot be deleted \n It might be used by some other process", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_OK|MB_ICONERROR);
//				}
//			}
//			//neha Gharge Open folder issue 1-5-2015
//			if (!(pDlg->m_szLastFileEncOrDec[0]))
//			{
//				if (pDlg->m_szFilePath[0])
//				{
//					_tcscpy(pDlg->m_szLastFileEncOrDec, pDlg->m_szFilePath);
//				}
//			}
//			else
//			{
//				if (_tcscmp(pDlg->m_szLastFileEncOrDec, pDlg->m_szFilePath) != 0)
//				{
//					if (pDlg->m_szFilePath[0])
//					{
//						_tcscpy(pDlg->m_szLastFileEncOrDec, pDlg->m_szFilePath);
//					}
//
//				}
//
//			}
//			
//		}
//		else
//		{
//			pDlg->m_bIsEnDepInProgress = false;
//			AddLogEntry(L"### DataEncryption : Error in FILE_OPERATIONS in CDataEncryption:SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
//			pDlg->MessageBox(L"File encryption failed", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
//			
//			return 1;
//		}
//		pDlg->m_Button_Start.SetWindowText(L"Start");
//		//Issue : In data encryption if we try to encrypt file of larger size and meanwhile if we try to encrypt other file,
//		//after encryption the file which was encrypted second time takes the file size of first file.
//		//Resolved By Nitin K. 03 MARCH 2015
//		pDlg->m_Button_Browse.EnableWindow(true);
//		pDlg->m_Button_Start.EnableWindow(true);
//		pDlg->m_btnBackArrow.EnableWindow(true);
//			
//	}
//
//	CString csFilePathOnly(pDlg->m_szFilePath);
//	int iPos = csFilePathOnly.ReverseFind(L'\\');
//
//	if(iPos != -1)
//	{
//		/*	ISSUE NO - 748 NAME - NITIN K. TIME - 25th June 2014 */
//		CString csLastFilePath;
//		long     length = 0;
//		TCHAR*   buffer = NULL;
//		CString csPath;
//
//		// First obtain the size needed by passing NULL and 0.
//		//length = GetShortPathName(csFilePathOnly, NULL, 0);
//		//buffer = new TCHAR[length];
//		//if(	!buffer )
//		//{
//		//	return 0;
//		//}
//		//
//		//memset(buffer, 0, length); 
//
//		//// Now simply call again using same long path.
//		//length = GetShortPathName(csFilePathOnly, buffer, length);
//		//csPath.Format(L"%s",buffer);
//
//		//if(buffer != NULL)
//		//{
//		//	free(buffer);
//		//	buffer = NULL;
//		//}
//
//		iPos = csFilePathOnly.ReverseFind(L'\\');
//		CString csFolderPath = csFilePathOnly.Left(iPos);
//		CString csFileName = csFilePathOnly.Right(csFilePathOnly.GetLength() - (iPos + 1));
//	
//		csLastFilePath.Format(_T("Last encrypted file: %s"), csFileName);
//		pDlg->m_stPrevDecFilePath.SetWindowTextW(csLastFilePath);
//
//		csLastFilePath.Format(_T("File location: %s"), csFolderPath);
//		pDlg->m_stPreviousFolderLocation.SetWindowText(csLastFilePath);
//	}
//	
//	pDlg->m_Edit_FilePath.SetWindowTextW(L"");
//	pDlg->m_stPrevDecFilePath.ShowWindow(SW_SHOW);
//	pDlg->m_stPreviousFolderLocation.ShowWindow(SW_SHOW);
//	pDlg->m_Button_Start.SetWindowText(L"Start");
//	pDlg->m_btnBackArrow.EnableWindow(true);
//	pDlg->m_btnOpenfolder.ShowWindow(SW_SHOW);
//	pDlg->Invalidate(TRUE);
//	//Issue : In data encryption if we try to encrypt file of larger size and meanwhile if we try to encrypt other file,
//	//after encryption the file which was encrypted second time takes the file size of first file.
//	//Resolved By Nitin K. 03 MARCH 2015
//	pDlg->m_Button_Browse.EnableWindow(true);
//	pDlg->m_Button_Start.EnableWindow(true);
//	pDlg->m_bIsEnDepInProgress = false;
	return 0 ;
}

/**********************************************************************************************************                     
*  Function Name  :	DecryptFileThread                                                     
*  Description    : 1. Call to "Decrypt_File" function that accepts filename to decrypt
					2. Foll. conditions checked:
					   -> Selection of invalid encrypted file
					   -> Is filesize = 0
					   -> Is file decrypted successfully or not
					3. Displays last decrypted file name & location.
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
* modified by lalit
**********************************************************************************************************/
DWORD WINAPI DecryptFileThread(LPVOID lpParam)
{
	//CDataEncryptionDlg *pDlg = (CDataEncryptionDlg* ) lpParam ;

	//if(!pDlg)
	//	return 0;

	//CString csFilePath(pDlg->m_szFilePath);
	//pDlg->m_bIsEnDepInProgress = true;
	//if(csFilePath.Right(5) != L".WWIZ")
	//{
	//	//issue - if below message box appear and i close from tray then ui getting hang.
	//	// resolve -lalit 5-7-2015
	//	pDlg->m_bIsEnDepInProgress = false;
	//	pDlg->MessageBox(L"Selected file is not encrypted using WardWiz.\n\nPlease select valid encrypted file (*.WWIZ)", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
	//	pDlg->m_Button_Start.SetWindowText(L"Start");
	//	pDlg->m_Button_Browse.EnableWindow(true);
	//	//Issue : In data encryption if we try to encrypt file of larger size and meanwhile if we try to encrypt other file,
	//	//after encryption the file which was encrypted second time takes the file size of first file.
	//	//Resolved By Nitin K. 03 MARCH 2015
	//	pDlg->m_Button_Start.EnableWindow(true);
	//	pDlg->m_btnBackArrow.EnableWindow(true);
	//	return 0;
	//}

	//DWORD	dwStatus = 0x00 ;
	//DWORD dwRet = Decrypt_File( pDlg->m_szFilePath, dwStatus ) ;
	//if( dwRet == 3)
	//{
	//	pDlg->m_bIsEnDepInProgress = false;
	//	pDlg->MessageBox(L"No Data for decryption(file size : 0)", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
	//	pDlg->m_Edit_FilePath.SetWindowTextW(L"");
	//	pDlg->m_Button_Start.SetWindowText(L"Start");
	//	pDlg->m_Button_Browse.EnableWindow(true);
	//	//Issue : In data encryption if we try to encrypt file of larger size and meanwhile if we try to encrypt other file,
	//	//after encryption the file which was encrypted second time takes the file size of first file.
	//	//Resolved By Nitin K. 03 MARCH 2015
	//	pDlg->m_Button_Start.EnableWindow(true);
	//	pDlg->m_btnBackArrow.EnableWindow(true);
	//	
	//	return 1;
	//}
	//else if(dwRet == 8)
	//{
	//	pDlg->m_bIsEnDepInProgress = false;
	//	pDlg->MessageBox(L"Selected file is not encrypted using WardWiz \n\nPlease select valid file", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
	//	pDlg->m_Edit_FilePath.SetWindowTextW(L"");
	//	pDlg->m_Button_Start.SetWindowText(L"Start");
	//	//Issue : In data encryption if we try to encrypt file of larger size and meanwhile if we try to encrypt other file,
	//	//after encryption the file which was encrypted second time takes the file size of first file.
	//	//Resolved By Nitin K. 03 MARCH 2015
	//	pDlg->m_Button_Browse.EnableWindow(true);
	//	pDlg->m_Button_Start.EnableWindow(true);
	//	pDlg->m_btnBackArrow.EnableWindow(true);
	//	
	//	return 1;
	//}
	//else if( dwRet )
	//{
	//	//issue - if below message box appear and i close from tray then ui getting hang.
	//	// resolve -lalit 5-7-2015
	//	pDlg->m_bIsEnDepInProgress = false;
	//	MessageBox(NULL,L"File decryption failed", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
	//	pDlg->m_Edit_FilePath.SetWindowTextW(L"");
	//	pDlg->m_Button_Start.SetWindowText(L"Start");
	//	//Issue : In data encryption if we try to encrypt file of larger size and meanwhile if we try to encrypt other file,
	//	//after encryption the file which was encrypted second time takes the file size of first file.
	//	//Resolved By Nitin K. 03 MARCH 2015
	//	pDlg->m_Button_Browse.EnableWindow(true);
	//	pDlg->m_Button_Start.EnableWindow(true);
	//	pDlg->m_btnBackArrow.EnableWindow(true);
	//	
	//	return 1;
	//}
	//else
	//{
	//	CString csSrcFilePath(pDlg->m_szFilePath);
	//	int iPos = csSrcFilePath.ReverseFind(L'\\');
	//	CString csSrcFileName = csSrcFilePath.Right(csSrcFilePath.GetLength() - (iPos + 1));
	//	CString csFolderPath = csSrcFilePath.Left(iPos);
	//	CString csOriginalFilePath = L"";
	//	int iLength = csSrcFilePath.ReverseFind('.');
	//	if(iLength != -1)
	//	{
	//		csSrcFilePath.Truncate(iLength);
	//		csOriginalFilePath = csSrcFilePath;
	//	}
	//	TCHAR szDecryptedFilePath[1024],szTemp[1024];
	//	swprintf(szTemp, 1024,TEXT("%s\\%s") ,m_szProgramData, csSrcFileName) ;
	//	DWORD dwLen = wcslen( szTemp) ;
	//	szTemp[ dwLen-0x05 ] = '\0' ;
	//	wcscpy( szDecryptedFilePath, szTemp ) ;
	//	if(!PathFileExists(szDecryptedFilePath))
	//	{
	//		pDlg->m_bIsEnDepInProgress = false;
	//		return false;
	//	}	
	//	if(pDlg->SendDataEncryptionOperation2Service(FILE_OPERATIONS, szDecryptedFilePath, csOriginalFilePath, 1, true))
	//	{
	//		Sleep(500);
	//		pDlg->m_bIsEnDepInProgress = false;
	//		if(pDlg->SendDataEncryptionOperation2Service(FILE_OPERATIONS, pDlg->m_szFilePath, L"", 2, true))
	//		{
	//			pDlg->MessageBox(TEXT("File decrypted successfully."), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
	//		}
	//		//neha Gharge Open folder issue 1-5-2015
	//		if (!pDlg->m_szLastFileEncOrDec[0])
	//		{
	//			if (pDlg->m_szFilePath[0])
	//			{
	//				_tcscpy(pDlg->m_szLastFileEncOrDec, pDlg->m_szFilePath);
	//			}
	//		}
	//		else
	//		{
	//			if (_tcscmp(pDlg->m_szLastFileEncOrDec, pDlg->m_szFilePath) != 0)
	//			{
	//				if (pDlg->m_szFilePath[0])
	//				{
	//					_tcscpy(pDlg->m_szLastFileEncOrDec, pDlg->m_szFilePath);
	//				}

	//			}

	//		}
	//	}
	//	else
	//	{
	//		pDlg->m_bIsEnDepInProgress = false;
	//		AddLogEntry(L"### DataEncryption : Error in FILE_OPERATIONS in CDataEncryption:SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
	//		pDlg->MessageBox(L"File decrypted failed", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
	//		
	//		return 1;
	//	}
	//}
	////Issue : In data encryption if we try to encrypt file of larger size and meanwhile if we try to encrypt other file,
	////after encryption the file which was encrypted second time takes the file size of first file.
	////Resolved By Nitin K. 03 MARCH 2015
	//pDlg->m_Button_Browse.EnableWindow(true);
	//pDlg->m_Button_Start.EnableWindow(true);
	//pDlg->m_stPrevFileStatus.ShowWindow(SW_HIDE);
	//
	//CString csFilePathOnly(pDlg->m_szFilePath);
	//int iPos = csFilePathOnly.ReverseFind(L'\\');
	//if(iPos != -1)
	//{
	//	CString csFolderPath = csFilePathOnly.Left(iPos);
	//	CString csFileName = csFilePathOnly.Right(csFilePathOnly.GetLength() - (iPos + 1));

	//	CString csLastFilePath;
	//	csLastFilePath.Format(_T("Last decrypted file: %s"), csFileName);
	//	pDlg->m_stPrevDecFilePath.SetWindowTextW(csLastFilePath);

	//	csLastFilePath.Format(_T("File location: %s"), csFolderPath);
	//	pDlg->m_stPreviousFolderLocation.SetWindowText(csLastFilePath);
	//}

	//pDlg->m_stPrevDecFilePath.ShowWindow(SW_SHOW);
	//pDlg->m_stPreviousFolderLocation.ShowWindow(SW_SHOW);
	//pDlg->m_btnOpenfolder.ShowWindow(SW_SHOW);
	//pDlg->m_Edit_FilePath.SetWindowTextW(L"");
	//pDlg->m_Button_Start.SetWindowText(L"Start");
	//pDlg->m_btnBackArrow.EnableWindow(true);
	////Issue : In data encryption if we try to encrypt file of larger size and meanwhile if we try to encrypt other file,
	////after encryption the file which was encrypted second time takes the file size of first file.
	////Resolved By Nitin K. 03 MARCH 2015
	//pDlg->m_Button_Browse.EnableWindow(true);
	//pDlg->m_Button_Start.EnableWindow(true);
	//pDlg->Invalidate(TRUE);
	//pDlg->m_bIsEnDepInProgress = false;
	return 0 ;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedRadioEncrypt                                                     
*  Description    :	Allows selection of encrypt button to encrypt a file 
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CDataEncryptionDlg::OnBnClickedRadioEncrypt()
{
	m_btnDecrypt.SetCheck(false);
	m_Edit_FilePath.SetWindowTextW(L"");
	m_cbKeepOriginalFile.EnableWindow(true);
	//Issue: Keep original checkbox changes
	//Added By Nitin K Date 6th July 2015
	if (theApp.m_bCrKeepOrg)
	{
		m_cbKeepOriginalFile.SetCheck(BST_CHECKED);
	}
	else
	{
		m_cbKeepOriginalFile.SetCheck(BST_UNCHECKED);
	}
	m_btnEncrypt.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE, IDB_BITMAP_SELECT, IDB_BITMAP_DISABLE, IDB_BITMAP_SELECT, 0, 0, 0, 0, 0);
	m_Button_Start.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENCRYPT"));
	//Issue: 0000614  Data Encryption tab not contains the status control, it should show current operation/Error/Sucess 
	//Resolved By :  Nitin K Date : 1st July 2015
	CString csStatusMsg;
	csStatusMsg.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_EDIT_STATUS_ENC")
		, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENCRYPT"));
	m_edtCryptStatus.SetWindowTextW(csStatusMsg);
	bEncryption = true ;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnBack                                                     
*  Description    :	Allows to go back to parent dialog & also resets controls
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CDataEncryptionDlg::OnBnClickedBtnBack()
{
	/*this->ShowWindow(SW_HIDE);
	CISpyToolsDlg *pObjToolWindow = reinterpret_cast<CISpyToolsDlg*>(this->GetParent());
	pObjToolWindow->ShowHideToolsDlgControls(true);
	m_Edit_FilePath.SetWindowTextW(L"");
	m_stPrevFileStatus.ShowWindow(SW_HIDE);
	m_stPrevDecFilePath.ShowWindow(SW_HIDE);
	m_stPreviousFolderLocation.ShowWindow(SW_HIDE);
	m_btnOpenfolder.ShowWindow(SW_HIDE);
	InitializeDataEncryptionVaraibles();*/
}

/**********************************************************************************************************                     
*  Function Name  :	InitializeDataEncryptionVaraibles                                                     
*  Description    :	Initialize all variable with its initial values.
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta    
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CDataEncryptionDlg::InitializeDataEncryptionVaraibles()
{
	AddLogEntry(L">>> CDataEncryption : InitializeDataEncryptionVaraibles", 0, 0, true, ZEROLEVEL);
	/* m_hEncDecDLL = NULL ;
	 m_hRegisterDLL = NULL ;
	bEncryption = true ;
	 memset(m_szFilePath, 0x00, sizeof(TCHAR)*512 ) ;
	 m_btnEncrypt.SetCheck( BST_CHECKED ) ;
	 m_btnDecrypt.SetCheck( BST_UNCHECKED ) ;

	memset(m_szAppData, 0x00, sizeof(TCHAR)*512 ) ;

	GetEnvironmentVariable(TEXT("ALLUSERSPROFILE"), m_szAppData, 511 ) ;

	bool bVistaOnward = false ;

	OSVERSIONINFO	OSVI = {0} ;

	OSVI.dwOSVersionInfoSize = sizeof( OSVERSIONINFO ) ;
	GetVersionEx( &OSVI ) ;

	if( OSVI.dwMajorVersion > 5 )
		bVistaOnward = true ;
	TCHAR	szTemp[512] = {0} ;
	TCHAR	*pName = NULL ;

	GetModuleFileName(NULL, szTemp, 511 ) ;
	pName = wcsrchr(szTemp, '\\' ) ;
	if( pName )
		*pName = '\0' ;

	wcscat( szTemp, TEXT("\\WRDWIZREGISTERDATA.DLL") ) ;

	if( PathFileExists( szTemp ) )
	{
		if( !m_hRegisterDLL )
		{
			m_hRegisterDLL = LoadLibrary( szTemp ) ;
			AddLogEntry(L">>> CDataEncryption : Loading library WRDWIZREGISTERDATA.DLL", 0, 0, true, ZEROLEVEL);
		}
		if( !GetRegisteredData )
			GetRegisteredData = (GETREGISTEREDDATA ) GetProcAddress(m_hRegisterDLL, "GetRegisteredData") ;
	}*/
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonBrowse                                                     
*  Description    :	Browse a file for encryption/decryption
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CDataEncryptionDlg::OnBnClickedButtonBrowse()
{
	//Issue No: 1020 Neha Gharge.
	// When browse, Password popup is open , User double click on encrypted file or right click any file for encryption
	//or decryption processes get started. So now we avoid that ..When one process is gng on we are not allowing other 
	//process from right click or double click on encrypted files.
	theApp.m_bDialogsOpenInDataEnc = true;
	if( bEncryption )
		BrowseFolderForFileSelection(NULL, NULL ) ;
	else
		BrowseFolderForFileSelection( TEXT("WWIZ"), TEXT("*.WWIZ") ) ;
	// issue :- UI  getting close when user cancel data encr/dec by close password pupup
	// resolved by lalit kumawat 8-27-2015	
	theApp.m_bDialogsOpenInDataEnc = false;
	theApp.m_bDataCryptOpr = false;
}

/**********************************************************************************************************                     
*  Function Name  :	BrowseFolderForFileSelection                                                     
*  Description    :	Browse folder for file selection to encrypt or decrypt(*.WWIZ)
*  Modified By	  : Nitin K
*  Modification	  : Updated Browse Window for new Implementation of Enc/Dec
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CDataEncryptionDlg::BrowseFolderForFileSelection(TCHAR *pName, TCHAR *pExt )
{
	try
	{
		bool bNetworkPath = false;
		bool bAlreadyAdded = false;
		CString csSelectionText = NULL;
		CString csSelection = NULL;
		if (bEncryption)
		{
			csSelectionText = _T("*.*");
			csSelection = _T("All files and folders(*.*)|*.*||");
		}
		else
		{
			csSelectionText = _T("*.WWIZ||");
			csSelection = _T("All files and folders(*.*)|*.WWIZ*||");
		}
		
		CSelectDialog ofd(TRUE, csSelectionText, NULL,
			OFN_HIDEREADONLY | OFN_NODEREFERENCELINKS | OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON,
			csSelection);
		
		m_bIsPopUpDisplayed = true;
		if (ofd.DoModal() != IDOK)
		{
			m_bIsPopUpDisplayed = false;
			return;
		}
		m_bIsPopUpDisplayed = false;

		// issue In Dataencryption->select 'Encrypt' or 'Decrypt' & click 'Browse'->then right click on any other file & encrypt 
		//it->once the encryption process starts select any file from browsed folder->then that file path is reflecting in the input box.
		// resolved by lalit kumawat18-8-2015
		if (m_bIsEnDepInProgress)
			return;

		for (int iPos = 0; iPos < ofd.m_SelectedItemList.GetCount(); iPos++)
		{
			LVFINDINFO lvInfo;
			lvInfo.flags = LVFI_STRING;
			CString csFileName = NULL;
			CString csScanPath = NULL;
			int iCListEntriesLength = 0;
			csFileName = ofd.m_SelectedItemList[iPos];
			lvInfo.psz = csFileName;
			if (PathIsNetworkPath(csFileName))
			{
				bNetworkPath = true;
				continue;
			}
				m_Edit_FilePath.SetWindowTextW(ofd.m_SelectedItemList[iPos]);
		}

		if (bNetworkPath)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_NETWORK_PATH_NOT_ALLOWED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR | MB_OK);
			m_bIsPopUpDisplayed = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### error in CDataEncryptionDlg::BrowseFolderForFileSelection", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	PopUpDialog()                                                     
*  Description    :	PopUp Password Window.
*  SR.N0		  : 
*  Author Name    : Neha Gharge                                                                                    
*  Date           : 3 Dec 2013
* Modified Date	  : Neha Gharge 6 July,2015
//Issue no : 664 If i right click and say as encrypt, then we select another file and say as encrypt, it shows two messagebox
**********************************************************************************************************/
int CDataEncryptionDlg::PopUpDialog()
{
	int		iRet = IDCANCEL;
	if (m_objPasswordPopUpDlg.m_hWnd == NULL)
	{
		m_bIsPopUpDisplayed = true;
		iRet = static_cast<int>(m_objPasswordPopUpDlg.DoModal());
		m_bIsPopUpDisplayed = false;

		if (iRet == IDCANCEL)
		{
			m_csPassword = "";
			m_Edit_FilePath.SetWindowTextW(L"");
			Invalidate();
			return iRet;
		}

		if (iRet == IDOK)
		{
			m_csPassword = m_objPasswordPopUpDlg.m_csEditText;
			if (!ValidatePassword(m_csPassword))
			{
				//dwEnKey=Str2Hex(m_csPassword);

			}
		}
	}
	
	Invalidate();
	return iRet ;
}


/**********************************************************************************************************                     
*  Function Name  :	OnNcHitTest                                                     
*  Description    :	The framework calls this member function for the CWnd object that contains the cursor every time the mouse is moved.
*  Author Name    : Vilas & Prajakta  
*  SR_NO		  :
*  Date           : 16 Nov 2013
**********************************************************************************************************/
LRESULT CDataEncryptionDlg::OnNcHitTest(CPoint point)
{
	return HTCLIENT;
}

/**********************************************************************************************************                     
*  Function Name  :	Encrypt_File                                                     
*  Description    :	Encrypts selected file & save with extension *.WWIZ  
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD CDataEncryptionDlg::Encrypt_File( TCHAR *m_szFilePath, TCHAR *pDestPath, DWORD &dwStatus )
{
	DWORD	dwRet = 0x00 ;
	DWORD	dwFileSize = 0x00, dwBytesRead = 0x00 ;
	TCHAR	szTemp[1024] = {0} ;
	TCHAR	szExt[16] = {0} ;
	DWORD	dwLen = 0x00 ;
	LPBYTE	lpFileData = NULL ;
	HANDLE	hFile = INVALID_HANDLE_VALUE ;
	HANDLE	hFileEnc = INVALID_HANDLE_VALUE ;

	TCHAR	*pFileName = NULL ;

	try
	{
		if( !PathFileExists( m_szFilePath ) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		pFileName = wcsrchr(m_szFilePath, '\\' ) ;
		if( !pFileName )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		pFileName++ ;

		//
		dwLen = static_cast<DWORD>(wcslen(m_szFilePath));
		memcpy(szExt, &m_szFilePath[dwLen-0x05], 0x0A ) ;

		// Issue : If we changed any file extension to .WWIZ->In data encryption->browse that file
		//->click on start->pop-up appears as "Selected file is already encrypted using WardWiz".
		// Neha Gharge 9-3-2015 

		//if( memcmp(szExt, TEXT(".WWIZ") ,0x5) == 0x00)
		//{
		//	dwRet = 0x1;
		//}
		
		if(dwRet != 1)
		{
			CString csFilePath(m_szFilePath);
			int iPos = csFilePath.ReverseFind(L'\\');
			CString csFolderPath = csFilePath.Left(iPos);
			TCHAR	szTemp1[1024] = {0} ;
			swprintf(szTemp1, 1024, TEXT("%s\\%s.WWIZ"), csFolderPath, pFileName ) ;
			swprintf(szTemp, 1024, TEXT("%s\\%s.WWIZ"), m_szProgramData, pFileName ) ;
			wcscpy( pDestPath, szTemp ) ;
			if( PathFileExists( szTemp1 ) )
			{
				m_bIsPopUpDisplayed = true;
				/*	ISSUE NO - 669 NAME - NITIN K. TIME - 12th June 2014 */
				if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_DATA_CRYPT_REPLACE_EXISTING"),
					theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_YESNO | MB_ICONQUESTION) == IDNO)
				{
					m_bIsPopUpDisplayed = false;
					dwRet = 0x09 ;
					goto Cleanup ;
				}
				m_bIsPopUpDisplayed = false;
			}
		}

		hFile = CreateFile(	m_szFilePath, GENERIC_READ, 0, NULL,
								OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( hFile == INVALID_HANDLE_VALUE )
		{
			AddLogEntry(L"### DataEncryption : Error in opening existing file %s", m_szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		dwFileSize = GetFileSize( hFile, NULL ) ;
		if( !dwFileSize )
		{
			AddLogEntry(L"### DataEncryption : Error in GetFileSize of %s", m_szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x03 ;
			goto Cleanup ;
		}
		if( lpFileData )
		{
			free(lpFileData);
			lpFileData=NULL;

		}

		//lpFileData = (LPBYTE ) malloc( dwFileSize +0x08 ) ;
		lpFileData = (LPBYTE ) malloc( dwFileSize ) ;
		if( !lpFileData )
		{
			AddLogEntry(L"### DataEncryption : Error in allocation of memory", 0, 0, true, SECONDLEVEL);
			dwRet = 0x04 ;
			goto Cleanup ;
		}

		memset(lpFileData, 0x00, dwFileSize ) ;
		SetFilePointer( hFile, 0x00, NULL, FILE_BEGIN ) ;
		ReadFile( hFile, lpFileData, dwFileSize, &dwBytesRead, NULL ) ;
		if( dwFileSize != dwBytesRead )
		{
			AddLogEntry(L"### DataEncryption : Error in ReadFile %s",m_szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x04 ;
			goto Cleanup ;
		}
		
		//create key
		if(!theApp.CreateRandomKeyFromFile(hFile, dwFileSize))
		{
			AddLogEntry(L"### DataEncryption : Error in CreateRandomKeyFromFile", 0, 0, true, SECONDLEVEL);
			dwRet = 0x08 ;
			goto Cleanup ;
		}

		if( theApp.DecryptData( (LPBYTE)lpFileData, dwBytesRead) )
		{
			AddLogEntry(L"### DataEncryption : Error in DecryptData", 0, 0, true, SECONDLEVEL);
			dwRet = 0x05 ;
			goto Cleanup ;
		}

		hFileEnc = CreateFile(	szTemp, GENERIC_READ|GENERIC_WRITE, 0, NULL,
								OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( hFileEnc == INVALID_HANDLE_VALUE )
		{
			AddLogEntry(L"### DataEncryption : Error in opening file %s",szTemp, 0, true, SECONDLEVEL);
			if( CreateFileAtAllUserLocation(szTemp, lpFileData, dwBytesRead ) )
			{
				dwRet = 0x06 ;
				goto Cleanup ;
			}
		}

		dwBytesRead = 0x00 ;
		SetFilePointer( hFileEnc, 0x00, NULL, FILE_BEGIN ) ;
		WriteFile(hFileEnc, WRDWIZ_SIG, WRDWIZ_SIG_SIZE, &dwBytesRead, NULL); // Write sig "WARDWIZ"
		if(dwBytesRead != WRDWIZ_SIG_SIZE)
			dwRet = 0x9;
		
		SetFilePointer( hFileEnc, (0x00 + WRDWIZ_SIG_SIZE), NULL, FILE_BEGIN ) ;
		WriteFile(hFileEnc, theApp.m_pbyEncDecKey, WRDWIZ_KEY_SIZE, &dwBytesRead, NULL); // Write Encryption key
		if(dwBytesRead != WRDWIZ_KEY_SIZE)
			dwRet = 0x9;

		SetFilePointer(hFileEnc, (0x0 + WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE), NULL, FILE_BEGIN);
		WriteFile( hFileEnc, lpFileData, dwFileSize, &dwBytesRead, NULL ) ; // Write encrypted data in file
		if( dwFileSize != dwBytesRead )
			dwRet = 0x07 ;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CDataEncryptionDlg::Encrypt_File", 0, 0, true, SECONDLEVEL);
		return false;
	}

Cleanup :

	if( hFile != INVALID_HANDLE_VALUE )
		CloseHandle( hFile ) ;
	hFile = INVALID_HANDLE_VALUE ;

	if( hFileEnc != INVALID_HANDLE_VALUE )
		CloseHandle( hFileEnc ) ;
	hFile = INVALID_HANDLE_VALUE ;

	if( lpFileData )
		free( lpFileData ) ;
	lpFileData = NULL ;

	if(theApp.m_pbyEncDecKey != NULL)
	{
		free(theApp.m_pbyEncDecKey);
		theApp.m_pbyEncDecKey = NULL;
	}
	return dwRet ;
}


/**********************************************************************************************************                     
*  Function Name  :	Decrypt_File                                                     
*  Description    :	Decrypts selected file with extension *.WWIZ  
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD Decrypt_File( TCHAR *m_szFilePath, DWORD &dwStatus )
{

	DWORD	dwRet = 0x00 ;
	DWORD	dwFileSize = 0x00, dwBytesRead = 0x00 ;
	TCHAR	szTemp[1024] = {0} , szFileName[1024] = {0} ;
	TCHAR	szExt[16] = {0} ;
	DWORD	dwLen = 0x00 ;
	LPBYTE	lpFileData = NULL ;
	HANDLE	hFile = INVALID_HANDLE_VALUE ;
	HANDLE	hFileEnc = INVALID_HANDLE_VALUE ;

	try
	{

		if( !PathFileExists( m_szFilePath ) )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		dwLen = static_cast<DWORD>(wcslen(m_szFilePath));
		if( (dwLen < 0x08) || (dwLen > 0x400) )
		{
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		memcpy(szExt, &m_szFilePath[dwLen-0x05], 0x0A ) ;
		if (_wcsicmp(szExt, TEXT(".WWIZ")) != 0x00)
		{
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		hFile = CreateFile(	m_szFilePath, GENERIC_READ, 0, NULL,
								OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( hFile == INVALID_HANDLE_VALUE )
		{
			AddLogEntry(L"### DataEncryption : Error in opening existing file %s",m_szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		dwFileSize = GetFileSize( hFile, NULL ) ;
		if( !dwFileSize )
		{
			AddLogEntry(L"### DataEncryption : Error in GetFileSize of %s",m_szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		lpFileData = (LPBYTE ) malloc( dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE )) ;
		if( !lpFileData )
		{
			AddLogEntry(L"### DataEncryption : Error in allocation of memory", 0, 0, true, SECONDLEVEL);
			dwRet = 0x04 ;
			goto Cleanup ;
		}

		memset(lpFileData, 0x00, (dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE))) ;
		SetFilePointer( hFile, (0x00 + (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)), NULL, FILE_BEGIN ) ;
		ReadFile( hFile, lpFileData, (dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)), &dwBytesRead, NULL ) ;
		if( (dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)) != dwBytesRead )
		{
			AddLogEntry(L"### DataEncryption : Error in ReadFile %s",m_szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x04 ;
			goto Cleanup ;
		}

		//read key from file
		if(!theApp.ReadKeyFromEncryptedFile(hFile))
		{
			AddLogEntry(L"### DataEncryption : Error in ReadKeyFromEncryptedFile", 0, 0, true, SECONDLEVEL);
			dwRet = 0x08 ;
			goto Cleanup ;
		}

		if( theApp.DecryptData( (LPBYTE)lpFileData, dwBytesRead ) )
		{
			AddLogEntry(L"### DataEncryption : Error in DecryptData", 0, 0, true, SECONDLEVEL);
			dwRet = 0x05 ;
			goto Cleanup ;
		}

		CString csSrcFilePath(m_szFilePath);
		int iPos = csSrcFilePath.ReverseFind(L'\\');
		CString csSrcFileName = csSrcFilePath.Right(csSrcFilePath.GetLength() - (iPos + 1));
		swprintf(szTemp, 1024, TEXT("%s\\%s"), m_szProgramData, csSrcFileName) ;
		dwLen = 0x0;
		dwLen = static_cast<DWORD>(wcslen(szTemp));
		szTemp[ dwLen-0x05 ] = '\0' ;
		hFileEnc = CreateFile(	szTemp, GENERIC_READ|GENERIC_WRITE, 0, NULL,
								OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( hFile == INVALID_HANDLE_VALUE )
		{
			AddLogEntry(L"### DataEncryption : Error in Opening file %s", szTemp, 0, true, SECONDLEVEL);
			dwRet = 0x06 ;
			goto Cleanup ;
		}

		dwBytesRead = 0x00 ;
		WriteFile( hFileEnc, lpFileData, (dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)), &dwBytesRead, NULL ) ;
		if( (dwFileSize - (WRDWIZ_SIG_SIZE + WRDWIZ_KEY_SIZE)) != dwBytesRead )
			dwRet = 0x07 ;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CDataEncryptionDlg::Decrypt_File", 0, 0, true, SECONDLEVEL);
		return false;
	}
Cleanup :

	if( hFile != INVALID_HANDLE_VALUE )
		CloseHandle( hFile ) ;
	hFile = INVALID_HANDLE_VALUE ;

	if( hFileEnc != INVALID_HANDLE_VALUE )
		CloseHandle( hFileEnc ) ;
	hFile = INVALID_HANDLE_VALUE ;

	if( lpFileData )
		free( lpFileData ) ;
	lpFileData = NULL ;

	if(theApp.m_pbyEncDecKey != NULL)
	{
		free(theApp.m_pbyEncDecKey);
		theApp.m_pbyEncDecKey = NULL;
	}
	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CreateFileAtAllUserLocation                                                     
*  Description    :	If handle file fails to open existing file ,it will create a file in module and then copy into destination 
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD CreateFileAtAllUserLocation(TCHAR *pFilePath,LPBYTE lpFileData,DWORD dwFileData )
{
	DWORD	dwRet = 0x00 ;
	DWORD	dwBytesWritten = 0x00 ;
	TCHAR	szTemp[1024] = {0} ;
	TCHAR	*pFileName = NULL ;
	HANDLE	hFileEnc = INVALID_HANDLE_VALUE ;

	__try
	{
		AddLogEntry(L">>> DataEncryption : CreateFileAtAllUserLocation", 0, 0, true, ZEROLEVEL);
		pFileName = wcsrchr(pFilePath, '\\' ) ;
		if( !pFileName )
		{
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		swprintf(szTemp, 1024, TEXT("%s%s"), m_szAppData, pFileName ) ;
		SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL);
		DeleteFile( szTemp ) ;
		Sleep( 100 ) ;
		SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL);
		DeleteFile( szTemp ) ;

		hFileEnc = CreateFile(	szTemp, GENERIC_READ|GENERIC_WRITE, 0, NULL,
								OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( hFileEnc == INVALID_HANDLE_VALUE )
		{
			AddLogEntry(L"### DataEncryption : Error in opening file %s",szTemp, 0, true, SECONDLEVEL);
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		WriteFile( hFileEnc, lpFileData, dwFileData, &dwBytesWritten, NULL ) ;
		if( dwBytesWritten != dwFileData )
		{
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		if( hFileEnc != INVALID_HANDLE_VALUE )
			CloseHandle( hFileEnc ) ;
		hFileEnc = INVALID_HANDLE_VALUE ;

		dwRet = CopyFileToDestination(szTemp, pFilePath ) ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
	}

Cleanup :

	if( hFileEnc != INVALID_HANDLE_VALUE )
		CloseHandle( hFileEnc ) ;
	hFileEnc = INVALID_HANDLE_VALUE ;

	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	CopyFileToDestination                                                     
*  Description    :	It copies file from source location to destination location
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD CopyFileToDestination(TCHAR *pSource, TCHAR *pDest )
{
	DWORD	dwRet = 0x00 ;
	TCHAR	szTemp[1024] = {0} ;

	AddLogEntry(L">>> CopyFile Source path: %s file to destination path: %s ",pSource,pDest, true, ZEROLEVEL);

	if( MoveFileEx(pSource, pDest, MOVEFILE_REPLACE_EXISTING ) )
		return 0x00 ;

	if( PathFileExists( pDest) )
		dwRet = RenameDestination( pDest, szTemp ) ;

	Sleep( 100 ) ;
	if( MoveFileEx(pSource, pDest, MOVEFILE_REPLACE_EXISTING ) )
	{
		if( dwRet )
			MoveFileEx(szTemp, NULL, MOVEFILE_DELAY_UNTIL_REBOOT ) ;

		return 0x00 ;
	}

	if( CopyFile( pSource, pDest, FALSE ) )
	{
		MoveFileEx(pSource, NULL, MOVEFILE_DELAY_UNTIL_REBOOT ) ;
		return 0x00 ;
	}

	return 0x01 ;
}

/**********************************************************************************************************                     
*  Function Name  :	RenameDestination                                                     
*  Description    :	It renames with original name(source name) to given name(Destination name).
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
DWORD RenameDestination(TCHAR *pDest, TCHAR *pRenamed )
{

	swprintf(pRenamed, 1024, TEXT("%s.%lu"), pDest, GetTickCount() ) ;

	SetFileAttributes(pRenamed, FILE_ATTRIBUTE_NORMAL);
	DeleteFile( pRenamed ) ;
	if( !_wrename(pDest, pRenamed ) )
		return 0x01 ;

	Sleep( 100 ) ;
	swprintf(pRenamed, 1024, TEXT("%s.%lu"), pDest, GetTickCount() ) ;

	SetFileAttributes(pRenamed, FILE_ATTRIBUTE_NORMAL);
	DeleteFile( pRenamed ) ;
	if( !_wrename(pDest, pRenamed ) )
		return 0x01 ;

	return 0x00 ;
}

/**********************************************************************************************************                     
*  Function Name  :	DecryptData                                                     
*  Description    :	Decryption logic (XOR)
*  SR.N0		  : ITINDATAENCRYPTION_0018
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
//DWORD DecryptData( LPBYTE lpBuffer, DWORD dwSize )
//{
//
//	if( IsBadWritePtr( lpBuffer, dwSize ) )
//		return 1 ;
//
//	DWORD	iIndex = 0 ;
//	DWORD jIndex = 0;
//
//	if (lpBuffer == NULL || dwSize == 0x00)
//	{
//		return 1;
//	}
//
//	for (iIndex = 0x00, jIndex = 0x00; iIndex < dwSize; iIndex++)
//	{
//		if(lpBuffer[iIndex] != 0)
//		{
//			lpBuffer[iIndex] ^= (WRDWIZ_KEY[jIndex++] + WRDWIZ_KEYSIZE);
//			if (jIndex == WRDWIZ_KEYSIZE)
//			{
//				jIndex = 0x00;
//			}
//			if (iIndex >= dwSize)
//			{
//				break;
//			}
//		}
//	}	
//	//DWORD	i = 0 ;
//
//	//for(; i<dwSize; )
//	//{
//	//	if( (dwSize - i) < 0x04 )
//	//		break ;
//
//	//	*( (DWORD * )&lpBuffer[i] ) = *( (DWORD * )&lpBuffer[i] ) ^ dwEnKey ;
//	//	i += 0x04 ;
//	//}
//
//	return 0 ;
//}

/*

void InitializeVariables()
{
	dwEnKey = *( (DWORD * )&szpIpsy ) ;
	memset(m_szAppData, 0x00, sizeof(TCHAR)*512 ) ;

	GetEnvironmentVariable(TEXT("ALLUSERSPROFILE"), m_szAppData, 511 ) ;

	//m_bIsWow64 = FALSE ;
	bool bVistaOnward = false ;

	OSVERSIONINFO	OSVI = {0} ;

	OSVI.dwOSVersionInfoSize = sizeof( OSVERSIONINFO ) ;
	GetVersionEx( &OSVI ) ;

	if( OSVI.dwMajorVersion > 5 )
		bVistaOnward = true ;

	//IsWow64() ;
}
*/
/*
void IsWow64()
{
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL) ;

	LPFN_ISWOW64PROCESS		IsWow64Process = NULL ;

	IsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(	GetModuleHandle( TEXT("kernel32")),
															"IsWow64Process") ;
	if( !IsWow64Process )
		return ;

	IsWow64Process( GetCurrentProcess(), &m_bIsWow64 ) ;
}
*/

/**********************************************************************************************************                     
*  Function Name  :	OnSetCursor                                                     
*  Description    :	The framework calls this member function if mouse input is not captured and the mouse 
					causes cursor movement within 
					the CWnd object.
*  Author Name    : Vilas & Prajakta
*  SR_NO		  :
*  Date           : 16 Nov 2013
**********************************************************************************************************/
BOOL CDataEncryptionDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if( iCtrlID == IDC_BUTTON_CLEAN		||
		iCtrlID == IDC_BUTTON_START		||
		iCtrlID == IDC_BUTTON_BROWSE	||
		iCtrlID == IDC_BTN_BACK			||		
		iCtrlID == IDC_RADIO_ENCRYPT	||
		iCtrlID == IDC_RADIO_DECRYPT	)
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
*  Function Name  :	OnBnClickedBtnOpenfolder                                                     
*  Description    :	Opens encrypted/decrypted file's folder location
*  SR.N0		  :
*  Author Name    : Vilas & Prajakta                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CDataEncryptionDlg::OnBnClickedBtnOpenfolder()
{
	//Neha Gharge 28-2-2015 
	//Without browsing any folder click on start->click on 'open folder'->pop up displaying as ' The parameter is incorrect'
	//CString csOpenFolderPath;
	////if (m_szFilePath[0])
	////{
	////	_tcscpy(m_szLastFileEncOrDec, m_szFilePath);
	////}
	//if (m_szLastFileEncOrDec[0])
	//{
	//	csOpenFolderPath = m_szLastFileEncOrDec;
	//}
	//int Pos = csOpenFolderPath.ReverseFind('\\');
	//csOpenFolderPath.Truncate(Pos);
	//ShellExecute(NULL, L"open", csOpenFolderPath, NULL, NULL, SW_SHOW);
	
}


/**********************************************************************************************************                     
*  Function Name  :	ValidatePassword                                                     
*  Description    :	Check the valid password, if not valid again popup comes
*  SR.N0		  : 
*  Author Name    : Vilas                                                                                   
*  Date           : 6 Dec 2013
**********************************************************************************************************/
DWORD CDataEncryptionDlg::ValidatePassword(CString csPassword)
{
		int		iRet = 0x00 ;
	if(csPassword.GetLength() == 0 && g_bEncryption == false)
	{
		iRet = PopUpDialog() ;
		if( iRet == IDCANCEL )
		{
			iRet = 0x05;
			//break ;
		}
		if( iRet == IDOK )
			iRet = 0x00 ;

	}
	else
	{
		bool	bValid = ValidateGivenPassWord( m_csPassword ) ;
		if(bValid == false && g_bEncryption == false)
		{
			iRet = 0x00 ;
			return iRet;
		}
		while( !bValid )
		{
			iRet = PopUpDialog() ;
			if( iRet == IDCANCEL )
			{
				iRet = 0x05;
				break ;
			}
			if( iRet == IDOK )
				iRet = 0x00 ;

			bValid = ValidateGivenPassWord( m_csPassword ) ;
		}
	}
	return iRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	ValidateGivenPassWord                                                     
*  Description    :	Check the valid password, whether it contains atlst 1 no,1 special char,password length.
*  SR.N0		  : 
*  Author Name    :	Neha Gharge                                                                                   
*  Date           : 4 Dec 2013
**********************************************************************************************************/
bool ValidateGivenPassWord(CString csPassword)
{
	int			i,count=0 ;
	CString		csInputKey ;
	csInputKey = csPassword;

	int		Validlenght = csInputKey.GetLength();

	if(Validlenght==0)
	{
		//MessageBox(NULL,L"Please Enter Password",theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
		//PopUpDialog();
		return false ;
	}

	if(Validlenght<=5)//Name:Varada Ikhar, Date:15/01/2015 Version: 1.8.3.17 Description:Data Encryption password length should be reduced. min:6 max:24
	{
		//MessageBox(NULL,L"Your password must be more than 5 characters",theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
		//PopUpDialog();
		
		return false ;
	}
	if(Validlenght>=25)
	{
		//MessageBox(NULL,L"Your password must be less than 25 characters",theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
		//PopUpDialog();

		return false ;
	}
	if((csInputKey.FindOneOf(L"~`!@#$%^&*()_-+={}[]|\?/.,':;<>")==-1))
	{
		//MessageBox(NULL,L"Your password must contain Special Character",theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
		//PopUpDialog();

		return false ;
	}
	for(i=0;i<Validlenght;i++)
	{
		if((isdigit(csInputKey[i])))
		{
			count++ ;
		}
	}
	if(count<=0)
	{
		//MessageBox(NULL,L"Your password must contain numeric value",theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),SWP_SHOWWINDOW);
		//PopUpDialog();

		return false ;
	}

	return true ;
}


/**********************************************************************************************************                     
*  Function Name  :	Str2Hex                                                     
*  Description    :	Convert string to DWORD.
*  SR.N0		  : 
*  Author Name    :	Neha Gharge                                                                                   
*  Date           : 3 Dec 2013
**********************************************************************************************************/
DWORD Str2Hex( CString const & s )
{
    DWORD result = 0;

	for ( int i = 0; i < s.GetLength(); i++ )
    {
        if ( isdigit( s[ i ] ) )
        {
            result = result * 16 + ( s[ i ] - '0' );
        }
        else // if ( isxdigit( s[ i ] ) )
        {
            result = result * 16 + ( s[ i ] - 'a' + 10 ); 
        }
    }

    return result;
}
  

/**********************************************************************************************************                     
*  Function Name  :	SendDataEncryptionOperation2Service                                                     
*  Description    :	Send Data encryption operation to comm service through named pipe.
*  SR.N0		  : 
*  Author Name    :	Neha Gharge                                                                                   
*  Date           : 3 Dec 2013
**********************************************************************************************************/
bool CDataEncryptionDlg::SendDataEncryptionOperation2Service(const TCHAR * chPipeName, DWORD dwType, CString csSrcFilePath, CString csDestFilePath, DWORD dwValue, DWORD dwValueOperation, bool bDataEncryptionWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	
	szPipeData.iMessageInfo = dwType;
	szPipeData.dwValue = dwValue;
	szPipeData.dwSecondValue = dwValueOperation;
	wcscpy_s(szPipeData.szFirstParam, csSrcFilePath);
	wcscpy_s(szPipeData.szSecondParam, csDestFilePath);
	
	CISpyCommunicator objCom(chPipeName, true);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CDataEncryptionDlg : SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bDataEncryptionWait)
	{
		if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CDataEncryptionDlg : SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	SendRegisteredData2Service                                                     
*  Description    :	Send Data of password or register data to comm service and service will stored that data
					throught named pipe
*  SR.N0		  : 
*  Author Name    :	Neha Gharge                                                                                   
*  Date           : 3 Dec 2013
**********************************************************************************************************/
bool CDataEncryptionDlg::SendRegisteredData2Service(DWORD dwType, LPBYTE lpResBuffer, DWORD dwResSize, DWORD dwResType, TCHAR *pResName)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	
	szPipeData.iMessageInfo = dwType;
	memcpy ( szPipeData.byData, lpResBuffer, dwResSize);
	szPipeData.dwValue = dwResSize;
	szPipeData.dwSecondValue = dwResType;
	wcscpy_s(szPipeData.szFirstParam, pResName);
	
	CISpyCommunicator objCom(SERVICE_SERVER, true);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CDataEncryptionDlg : SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CDataEncryptionDlg : SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(szPipeData.dwValue != 1)
	{
		return false;
	}

	return true;
}


/**********************************************************************************************************                     
*  Function Name  :	PreTranslateMessage                                                     
*  Description    : To translate window messages before they are dispatched 
				    to the TranslateMessage and DispatchMessage Windows functions
*  SR.N0		  : 
*  Author Name    :	Prassana                                                                                   
*  Date           : 3 Mar 2014
**********************************************************************************************************/
BOOL CDataEncryptionDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

/**********************************************************************************************************                     
*  Function Name  :	RefreshStrings                                                     
*  Description    : Refresh all strings
*  SR.N0		  : 
*  Author Name    :	Prassana                                                                                   
*  Date           : 30 Mar 2014
**********************************************************************************************************/
void CDataEncryptionDlg::RefreshStrings()
{
	// Issue Add description Neha Gharge 9-3-2015
	m_stEncryptHeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DENCRYPT_HEADER"));
	m_stHeaderDescription.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENCDEC_HEADER_DESCRIPTION"));
	m_stMsg.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DENCRYPT_MSG"));
	m_Button_Browse.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_BROWSE"));
	/*	ISSUE NO - 216 NAME - NITIN K. TIME - 21st May 2014 :: 8 pm*/
	m_Button_Start.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENCRYPT"));
	m_stEncrypt.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENCRYPT_BUTTON"));
	m_stDecrypt.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DECRYPT_BUTTON"));
	m_btnOpenfolder.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_DENCRYPT_OPENFOLDER"));

	m_stPrevDecFilePath.SetWindowTextW(L"");
	m_stPrevFileStatus.SetWindowTextW(L"");
	//Issue: 0000614  Data Encryption tab not contains the status control, it should show current operation/Error/Sucess 
	//Resolved By :  Nitin K Date : 1st July 2015
	CString csStatusMsg;
	csStatusMsg.Format(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_EDIT_STATUS_ENC")
		, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENCRYPT"));
	m_edtCryptStatus.SetWindowTextW(csStatusMsg);
	m_stPreviousFolderLocation.SetWindowTextW(L"");
	//Implementation : Adding Total file count and Processed file count
	//Added By : Nitin K Date: 2nd July 2015
	m_stTotalFileCountText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_TOTAL_FILES_COUNT"));
	m_stFileProccessedCount.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_CUSTOM_SCAN_PROCESSED_FILES_COUNT"));
}

/**********************************************************************************************************                     
*  Function Name  :	OnPaint                                                     
*  Description    : The framework calls this member function when Windows or an application makes a request 
					to repaint a portion of an application's window.
*  SR.N0		  : 
*  Author Name    :	Nitin                                                                                   
*  Date           : 3 May 2014
**********************************************************************************************************/
void CDataEncryptionDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CJpegDialog::Draw();
	CJpegDialog::OnPaint();
}

/**********************************************************************************************************
*  Function Name  :	CheckForWardWizSystemFile
*  Description    : Checks for the given file is WardWiz system file or not. If it is system file then it returns true else false
*  SR.N0		  :
*  Author Name    :	Nitin K
*  Date           : 09th March 2015
**********************************************************************************************************/
bool CDataEncryptionDlg::CheckForWardWizSystemFile(CString scFolderPathTemp, CString csSrcFileName)
{

	CString csAppPAth = NULL;
	CString csSystemPath = NULL;
	CString csFilePath = NULL;
	csFilePath.Format(L"%s\\%s", scFolderPathTemp, csSrcFileName);
	csAppPAth = theApp.m_objwardwizLangManager.GetWardWizPathFromRegistry();
	scFolderPathTemp.Append(L"\\"); 
	//Issue : Drive 'WardWiz Antivirus' folder / system32-Drivers-WrdWizscanner WrdWizsecure64 ->encryption & 
	//decryption shouldn't happen.
	//Resolved by :  Nitin K. Date : 09th March 2015
	if (scFolderPathTemp.Find(csAppPAth) == 0)
		return true;
	
	csSystemPath = GetWardWizSysPathFromRegistry(L"wrdwizscanner",L"ImagePath");
	if (csFilePath.CompareNoCase(csSystemPath) == 0)
		return true;
	csSystemPath = L"";
	csSystemPath = GetWardWizSysPathFromRegistry(L"WrdWizSecure64", L"ImagePath");
	if (csFilePath.CompareNoCase(csSystemPath) == 0)
		return true;
	return false;
}

/**********************************************************************************************************
*  Function Name  :	GetWardWizSysPathFromRegistry
*  Description    : returns the System32\driver... path for wrdwizscanner & wrdwizsecure64 .sys file of Wardiwz
*  SR.N0		  :
*  Author Name    :	Nitin K
*  Date           : 09th March 2015
**********************************************************************************************************/
CString CDataEncryptionDlg::GetWardWizSysPathFromRegistry(CString csSubKeyValue, CString csValue)
{
	HKEY	hSubKey = NULL;
	TCHAR	szModulePath[MAX_PATH] = { 0 };
	CString csSubKey = L"SYSTEM\\CurrentControlSet\\Services\\";
	csSubKey.Append(csSubKeyValue);
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, csSubKey, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
		return L"";

	DWORD	dwSize = 511;
	DWORD	dwType = 0x00;

	RegQueryValueEx(hSubKey, csValue, 0, &dwType, (LPBYTE)szModulePath, &dwSize);
	RegCloseKey(hSubKey);
	hSubKey = NULL;

	if (_tcslen(szModulePath) > 0)
	{
		CString csStr = NULL;
		csStr = (CString)szModulePath;
		int iPos = csStr.Find(L':');
		CString str = csStr.Right(csStr.GetLength() - (iPos - 1));
		return str;
	}
	return L"";
}

/**********************************************************************************************************
*  Function Name  :	ShutDownEncryptDecrypt
*  Description    : While encryption/decryption is in progress if user clicks on close button 'Wait to complete
					the operation' message should be displayed.
*  SR.N0		  :
*  Author Name    :	Varada Ikhar
*  Date           : 23rd April 2015
*  Updated By	  : Nitin K 10th July 2015
**********************************************************************************************************/
bool CDataEncryptionDlg::ShutDownEncryptDecrypt(DWORD dwManualStop)
{
	try
	{
		if (!m_bIsEnDepInProgress)
		{
			//lalit 5-7-2015 , closed the handle of encryption thread during shutdown encryption/decryption.
			/*if (m_hEncDecThread != NULL)
			{
				::SuspendThread(m_hEncDecThread);
				::TerminateThread(m_hEncDecThread, 0);
				CloseHandle(m_hEncDecThread);
				m_hEncDecThread = NULL;
			}*/
			AddLogEntry(L"### Encryption/Decryption operation is not running.", 0, 0, true, ZEROLEVEL);
			return true;
		}

		if (!PauseEncryptionDecryption())
		{
			AddLogEntry(L"### Failed to pause Encryption/Decryption thread.", 0, 0, true, SECONDLEVEL);
		}
		CString csTemp;
		CString csStr;
		// issue : During file integrity checking or finalizing(during crc byte add) user should wait till operation complete.
		// resolved by lalit kumawat
		if (theApp.m_bIsFileInegrityInprogrs)
		{
			csStr.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_CRITICL_DATA_OPER_IN_PROGRESS"), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_CRITICL_DATA_PRESS_OK_TO_CONT"));

			m_bIsPopUpDisplayed = true;
			MessageBox(csStr, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			m_bIsPopUpDisplayed = false;

			if (!ResumeEncryptionDecryption())
			{
				AddLogEntry(L"### Failed to resume Encryption/Decryption thread.", 0, 0, true, SECONDLEVEL);
			}
			m_bManualStop = false;
			m_Button_Start.EnableWindow(true);
			return false;
		}

		if (bEncryption)
			csStr = theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_ENCRYPTION_IN_PROGESS_FIRST");
		else
			csStr = theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_DECRYPTION_IN_PROGESS_FIRST");
		csTemp.Format(L"%s", csStr);
		csTemp.Append(L"\n");
		if (bEncryption)
			csTemp.Append(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_ENCRYPT_IN_PROGESS_SECOND"));
		else
			csTemp.Append(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_DECRYPT_IN_PROGESS_SECOND"));

		m_bIsPopUpDisplayed = true;
		DWORD dwRetValue =  MessageBox(csTemp, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO);
		m_bIsPopUpDisplayed = false;

		Sleep(1000);
		if (dwRetValue == IDNO)
		{
			// issue:- if user try to abort operation and make no during message popup, after that aborted popup getting appear even data operation successfully complete
			// resolved by lalit kumawat 7-15-2015
			m_bManualStop = false;
			if (!ResumeEncryptionDecryption())
			{
				AddLogEntry(L"### Failed to resume Encryption/Decryption thread.", 0, 0, true, SECONDLEVEL);
			}
			m_Button_Start.EnableWindow(true);
			return false;
		}
		else
		{
			m_bManualStop = true;
			m_Button_Start.EnableWindow(false);
			//Issue: UI getting unresponsive andCrypt exe not getting exited
			//Resolved By : Nitin K Date: 10th July 2015
			Sleep(500);
			if (!SendDataEncryptionOperation2Service(WWIZ_CRYPT_SERVER, STOP_CRYPT_OPR, m_szFilePath, m_csPassword, dwManualStop, 0, false))
			{
				AddLogEntry(L"### Dataencryption : Error in GenX_CRYPT_SERVER in CDataencryption:SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
			}
			// resolved by lalit kumawat 7-2-2015
			// issue :- double Encryption/decryption abort popup coming when stop opertation by stop click
		 //	StopCryptOperations();
		}

		
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataEncryptionDlg::ShutDownEncryptDecrypt", 0, 0 , true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	PauseEncryptionDecryption
*  Description    :	Pause Encryption/Decryption operation after user's click on close button.
*  Author Name    : Varada Ikhar
*  Date           : 23rd April 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CDataEncryptionDlg::PauseEncryptionDecryption()
{
	try
	{
		if (!SendDataEncryptionOperation2Service(WWIZ_CRYPT_SERVER, PAUSE_CRYPT_OPR, L"", L"", 0, 0, true))
		{
			AddLogEntry(L"### Dataencryption : Error in GenX_CRYPT_SERVER in CDataencryption:SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
		}
		/*if (!m_hEncDecThread)
			return false;

		if (m_hEncDecThread != NULL)
		{
			if (SuspendThread(m_hEncDecThread) == -1)
			{
				return false;
			}
		}*/
		AddLogEntry(L">>> Encryption/Decryption operation Paused.", 0, 0, true, ZEROLEVEL);
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataEncryptionDlg::PauseEncryptionDecryption", 0, 0, true, SECONDLEVEL);
		return false;
	}
}

/**********************************************************************************************************
*  Function Name  :	ResumeEncryptionDecryption
*  Description    :	Resume Encryption/Decryption operation after user's response to message pop-up on click on close button.
*  Author Name    : Varada Ikhar
*  Date           : 23rd April 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CDataEncryptionDlg::ResumeEncryptionDecryption()
{
	try
	{ 
		if (!SendDataEncryptionOperation2Service(WWIZ_CRYPT_SERVER, RESUME_CRYPT_OPR, L"", L"", 0, 0, true))
		{
			AddLogEntry(L"### Dataencryption : Error in GenX_CRYPT_SERVER in CDataencryption:SendRegisteredData2Service", 0, 0, true, SECONDLEVEL);
		}
		//lalit 5-7-2015 , thread handle made as mamber varialbe 
		/*if (!m_hEncDecThread)
			return false;

		if (m_hEncDecThread != NULL)
		{
			if (ResumeThread(m_hEncDecThread) == -1)
			{
				return false;
			}
		}*/
		AddLogEntry(L">>> Encryption/Decryption operation Resumed.", 0, 0, true, ZEROLEVEL);
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataEncryptionDlg::ResumeEncryptionDecryption", 0, 0, true, SECONDLEVEL);
		return false;
	}
	
}

/**********************************************************************************************************
*  Function Name  :	StopCryptOperations
*  Description    :	Stop the Crypt operations
*  Author Name    : Nitin Kplapkar
*  Date           : 15th June 2015
*  SR_NO		  :
**********************************************************************************************************/
void CDataEncryptionDlg::StopCryptOperations(DWORD dwSuccessStatus)
{
	try
	{
		m_btnEncrypt.EnableWindow(true);
		m_btnDecrypt.EnableWindow(true);
		m_Edit_FilePath.EnableWindow(true);
		m_Button_Browse.EnableWindow(true);
		m_Edit_FilePath.SetWindowTextW(L"");
		m_btnEncrypt.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE, IDB_BITMAP_SELECT, IDB_BITMAP_DISABLE, 0, 0, 0, 0, 0, 0);
		m_btnDecrypt.SetSkin(theApp.m_hResDLL, IDB_BITMAP_DISABLE, IDB_BITMAP_SELECT, IDB_BITMAP_DISABLE, 0, 0, 0, 0, 0, 0);
		if (m_btnEncrypt.GetCheck() == BST_CHECKED)
		{
			m_Button_Start.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ENCRYPT"));
			m_cbKeepOriginalFile.EnableWindow(true);
		}
		else
		{
			bEncryption = false;
			//m_btnDecrypt.SetCheck(true);
			m_Button_Start.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DECRYPT"));
			m_cbKeepOriginalFile.SetCheck(BST_UNCHECKED);
		}
		m_Button_Start.EnableWindow(true);
		m_bIsEnDepInProgress = false;

		// issue :- files/folder belongs to os drive not allowing for encryption.
	    // resolved by lalit kumawat 4-18-2015

		if (m_bIsSystemFolderPathSelected)
		{
			CString csTotalFileProcessed = L"";
			csTotalFileProcessed.Format(L"%s",theApp.m_objwardwizLangManager.GetString(L"IDS_DECR_SYSTEM_FILE_FOLDER_PATH"));
			
			m_bIsPopUpDisplayed = true;
			MessageBox(csTotalFileProcessed, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONASTERISK | MB_OK);
			m_bIsPopUpDisplayed = false;

			m_bIsSystemFolderPathSelected = false;
			return;
		}

		// issue : low disk space handling 
		// resolved by lalit kumawat 7-27-2015
		if (m_bIsLowDiskSpace)
		{
			// issue:- message box in32 bit system not showing complete message "last string getting null."
			// resolved by lalit kumawat 7-29-2015
			CString csMsgToLowSpace = L"";
			csMsgToLowSpace.Format(L"%s %s.%s", theApp.m_objwardwizLangManager.GetString(L"IDS_DATA_CRYPT_LOW_DISK_SPACE1"), m_csMsgforLowDiskSpace, theApp.m_objwardwizLangManager.GetString(L"IDS_DATA_CRYPT_LOW_DISK_SPACE2"));

			m_bIsPopUpDisplayed = true;
			MessageBox(csMsgToLowSpace, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONASTERISK | MB_OK);
			m_bIsPopUpDisplayed = false;
			
			m_bIsLowDiskSpace = false;
			return;
		}

		CString csTotalFileCountStatus = L"";
		// issue : 0000697 if folder contain no file or empty . need to show messagebox that there is no file to process
		// resolved by lalit kumawat 7-21-2015
		m_stTotalFileCountText.GetWindowTextW(csTotalFileCountStatus);
		CString csTotalFiles = L"";
		csTotalFiles.Format(L"%s 0", theApp.m_objwardwizLangManager.GetString(L"IDS_DATA_CRYPT_TOTAL_FILES"));
		if (csTotalFileCountStatus.Find(csTotalFiles) != -1 || csTotalFileCountStatus == csTotalFiles)
		{
			CString csTotalFileProcessed = L"";
			csTotalFileProcessed.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_ENC_DEC_ZEROR_FILE_TO_PROCESS"));

			m_bIsPopUpDisplayed = true;
			MessageBox(csTotalFileProcessed, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			m_bIsPopUpDisplayed = false;
			
			m_bIsSystemFolderPathSelected = false;
			return;
		}
		//1. Issue with Browse & Stop buttons on Data encryption UI.
		//Resolved By : Nitin K Date: 9th July 2015
		if (dwSuccessStatus == 1)
		{
			if (!m_bManualStop)
			{
				//Issue: 0000614  Data Encryption tab not contains the status control, it should show current operation/Error/Sucess 
				//Resolved By :  Nitin K Date : 1st July 2015

				//Issue: In case of 0 file processed we are showing failure messge for example file already encrypted/decrypted/ file size exceed to limit of 3GB 
				//Resolved By :  lalit k, Date : 9th July 2015
				CString csTotalFileProcessed = L"";

				m_bIsPopUpDisplayed = true;
				if (theApp.m_dwCryptFileCount > 0)
				{
					csTotalFileProcessed.Format(L"%s\n%s: %d", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_COMPLETED_MSG"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATUS_TOTAL_ENC_DEC_FILE_PROCESSED_COUNT"), theApp.m_dwCryptFileCount);
					MessageBox(csTotalFileProcessed, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
				}
				else
				{
					csTotalFileProcessed.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_FAILED_MSG"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_FAILED_check_ACTION_MSG"));
					MessageBox(csTotalFileProcessed, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
				}
				m_bIsPopUpDisplayed = false;
			}
			else
			{
				CString csFileStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_CANCELED");
				m_lstDataEncDec.SetItemText(0, 1, csFileStatus);
				//Issue: 0000614  Data Encryption tab not contains the status control, it should show current operation/Error/Sucess 
				//Resolved By :  Nitin K Date : 1st July 2015
				m_edtCryptStatus.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_ABORTED_MSG"));
				
				m_bIsPopUpDisplayed = true;
				if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_ABORTED_MSG"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) == IDOK)
				{
					m_bIsPopUpDisplayed = false;
					// resolved by lalit kumawat 6-29-2015
					// after once we click on stop encryption button and say no now stop button getting disable			
					m_Button_Start.EnableWindow(true);
				}
				m_bIsPopUpDisplayed = false;
			}
			m_bManualStop = false;

		}
		else
		{
			//1. Issue with Browse & Stop buttons on Data encryption UI.
			//Resolved By : Nitin K Date: 9th July 2015
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DATA_ENC_UNSUCCESS_MSG"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			m_bIsPopUpDisplayed = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataEncryptionDlg::StopCryptOperations", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	OnBnClickedCheckKeepOriginal
*  Description    :	the Registry value for Keep Original flag in Registry
*  Author Name    : Nitin Kplapkar
*  Date           : 15th June 2015
*  SR_NO		  :
**********************************************************************************************************/
void CDataEncryptionDlg::OnBnClickedCheckKeepOriginal()
{
	try
	{
		DWORD dwCrKeepOrg = 0;
		//Issue: Keep original checkbox changes
		//Added By Nitin K Date 6th July 2015
		if (m_cbKeepOriginalFile.GetCheck() == BST_CHECKED)
		{
			theApp.m_bCrKeepOrg = true;
			dwCrKeepOrg = 1;
		}
		else
		{
			theApp.m_bCrKeepOrg = false;
			dwCrKeepOrg = 0;
		}

		CHTMLListCtrl objCHTMLListCtrl;
		objCHTMLListCtrl.SetRegistrykeyUsingService(L"SOFTWARE\\Wardwiz Antivirus", L"dwCrKeepOrg", REG_DWORD, dwCrKeepOrg, false);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataEncryptionDlg::OnBnClickedCheckKeepOriginal", 0, 0, true, SECONDLEVEL);
	}
}


/***************************************************************************
Function Name  : IsPathBelongsToOSReservDirectory
Description    : Check if Path belongs to wardwiz directory
Author Name    : Ramkrushna Shelke
Date           : 9-11-2015
****************************************************************************/
bool CDataEncryptionDlg::IsPathBelongsToWardWizDir(CString csFilefolderPath)
{
	bool bReturn = false;

	try
	{
		//Check here if the path is lnk ( shortcut )
		if (csFilefolderPath.Right(4).CompareNoCase(L".lnk") == 0)
		{
			TCHAR szModulePath[MAX_PATH] = { 0 };
			if (GetModulePath(szModulePath, sizeof(szModulePath)))
			{
				CString csExpandedPath;
				csExpandedPath = ExpandShortcut(csFilefolderPath);
				if (csExpandedPath.Trim().GetLength() != 0)
				{
					int iPos = csExpandedPath.ReverseFind(L'\\');
					if (csExpandedPath.Left(iPos).CompareNoCase(szModulePath) == 0)
					{
						bReturn = true;
					}
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataEncryptionDlg::IsPathBelongsToWardWizDir", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : ExpandShortcut
*  Description    : Uses IShellLink to expand a shortcut.
*  Return value	  : the expanded filename, or "" on error or if filename
wasn't a shortcut
*  Author Name    : Nitin K
*  SR_NO		  :
*  Date           : 20th March 2015
*************************************************************************************************/
CString CDataEncryptionDlg::ExpandShortcut(CString& csFilename)
{
	CString csExpandedFile = NULL;

	try
	{
		USES_CONVERSION;		// For T2COLE() below

		// Make sure we have a path
		if (csFilename.IsEmpty())
		{
			ASSERT(FALSE);
			return csExpandedFile;
		}

		// Get a pointer to the IShellLink interface
		HRESULT hr;
		IShellLink* pIShellLink;

		hr = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
			IID_IShellLink, (LPVOID*)&pIShellLink);

		if (SUCCEEDED(hr))
		{
			// Get a pointer to the persist file interface
			IPersistFile* pIPersistFile;
			hr = pIShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pIPersistFile);

			if (SUCCEEDED(hr))
			{
				// Load the shortcut and resolve the path
				// IPersistFile::Load() expects a UNICODE string
				// so we're using the T2COLE macro for the conversion
				// For more info, check out MFC Technical note TN059
				// (these macros are also supported in ATL and are
				// so much better than the ::MultiByteToWideChar() family)
				hr = pIPersistFile->Load(T2COLE(csFilename), STGM_READ);

				if (SUCCEEDED(hr))
				{
					WIN32_FIND_DATA wfd;
					hr = pIShellLink->GetPath(csExpandedFile.GetBuffer(MAX_PATH),
						MAX_PATH,
						&wfd,
						SLGP_UNCPRIORITY);

					csExpandedFile.ReleaseBuffer(-1);
				}
				pIPersistFile->Release();
			}
			pIShellLink->Release();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataEncryptionDlg::ExpandShortcut()", 0, 0, true, SECONDLEVEL);;
	}
	return csExpandedFile;
}


/***************************************************************************
Function Name  : IsPathBelongsFromRemovableDrive
Description    : Check if Path belongs to Removable devices.
				 Check with Tray using pipe communication.
Author Name    : Ramkrushna Shelke
Date           : 11-12-2015
Issue resolved : 0001165
****************************************************************************/
bool CDataEncryptionDlg::IsPathBelongsFromRemovableDrive(CString csFilefolderPath)
{
	bool bReturn = false;
	try
	{

		if (csFilefolderPath.IsEmpty())
			return bReturn;

		UINT  uDriveType = 0;
		TCHAR szDriveRoot[] = _T("x:\\");
		_tcscpy_s(szDriveRoot, csFilefolderPath.Left(2));
		uDriveType = GetDriveType(szDriveRoot);

		if (uDriveType == DRIVE_REMOVABLE)
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataEncryptionDlg::IsPathBelongsRemovableDrive, File :%s", csFilefolderPath, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}