/**********************************************************************************************************                     
	  Program Name          : ISpyFolderLocker.cpp
	  Description           : Folder locker from tool 
	  Author Name			: Neha Gharge                                                                                           
	  Date Of Creation      : 11th Jan 2014
	  Version No            : 1.0.0.2
	  Special Logic Used    : 

	  Modification Log      :           
	  1.            
***********************************************************************************************************/

#include "stdafx.h"
#include "ISpyFolderLocker.h"
#include "ISpyToolsDlg.h"


#define		IDR_RESDATA_DES			7000

extern CString	g_csFolderLockerPassword;
//CFolderLockPassword g_objFolderLockPassword;
bool bRESFOLDER_DES = false ;
bool g_bLock = false;

// CISpyFolderLocker dialog
LPCTSTR g_OptionsForFolderLock[2]={L"Lock",L"Unlock"};
DWORD WINAPI UnlockFileORFolderThread( LPVOID lpParam ) ;
HANDLE	g_hThread_Unlock = NULL ;

typedef DWORD (*GETREGISTEREFOLDERDDATA) (LPBYTE lpResBuffer, DWORD &dwResDataSize, DWORD dwResType, TCHAR *pResName ) ;

//ADDREGISTEREDDATA	AddRegisteredData = NULL ;
GETREGISTEREFOLDERDDATA	GetfolderRegisteredData = NULL ;


IMPLEMENT_DYNAMIC(CISpyFolderLocker, CDialog)

/**********************************************************************************************************                     
*  Function Name  :	CISpyFolderLocker                                                     
*  Description    :	C'tor
*  Author Name    : Neha Gharge   
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
CISpyFolderLocker::CISpyFolderLocker(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CISpyFolderLocker::IDD, pParent)
	, m_hRegisterDLL(NULL)
	, m_bIsPopUpDisplayed(false)
{

}

/**********************************************************************************************************                     
*  Function Name  :	~CISpyFolderLocker                                                    
*  Description    :	D'tor
*  Author Name    : Neha Gharge   
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
CISpyFolderLocker::~CISpyFolderLocker()
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
*  Author Name    : Neha Gharge   
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CISpyFolderLocker::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_FOLDERLOCK_HEADERPIC, m_stHeaderFolderLockPic);
	//DDX_Control(pDX, IDC_BUTTON_FOLDERLOCK_BACK, m_btnBack);
	DDX_Control(pDX, IDC_EDIT_BROWSE_FOLDERLOCKFILE, m_edtFolderLockPath);
	DDX_Control(pDX, IDC_BUTTON_FOLDER_BROWSE, m_btnFolderLockBrowse);
	DDX_Control(pDX, IDC_COMBO_OPTION_FOLDERLOCK, m_comboFolderLockOption);
	DDX_Control(pDX, IDC_STATIC_INFOTEXT, m_stInfoText);
	DDX_Control(pDX, IDC_LIST_FOLDERLOCK, m_lstFolderLock);
	DDX_Control(pDX, IDC_BUTTON_FOLDEROPEN, m_btnFolderOpen);
	DDX_Control(pDX, IDC_BUTTON_FOLDERREMOVE, m_btnFolderRemove);
	DDX_Control(pDX, IDC_BUTTON_START_FOLDERACTION, m_btnStartFolderAction);
	DDX_Control(pDX, IDC_CHECK_BOX, m_chkSelectAll);
	DDX_Control(pDX, IDC_STATIC_SELECTALL, m_stSelectAll);
	DDX_Control(pDX, IDC_STATIC_FOLDERLCKR_SUBHEADER, m_stFolderLlckrSubHeader);
	DDX_Control(pDX, IDC_STATIC_FOLDERLCKR_HEADER, m_stFolderLlckrHeader);
}

/***************************************************************************
* Function Name  : MESSAGE_MAP
* Description    : Handle WM_COMMAND,WM_Messages,user defined message 
				   and notification message from child windows.
*  Author Name    : Neha Gharge    
*  SR_NO		  :
*  Date           : 11 Jan 2014
****************************************************************************/
BEGIN_MESSAGE_MAP(CISpyFolderLocker, CJpegDialog)
	ON_WM_NCHITTEST()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BUTTON_FOLDERLOCK_BACK, &CISpyFolderLocker::OnBnClickedButtonFolderlockBack)
	ON_BN_CLICKED(IDC_BUTTON_FOLDER_BROWSE, &CISpyFolderLocker::OnBnClickedButtonFolderBrowse)
	ON_BN_CLICKED(IDC_BUTTON_FOLDEROPEN, &CISpyFolderLocker::OnBnClickedButtonFolderopen)
	ON_BN_CLICKED(IDC_BUTTON_FOLDERREMOVE, &CISpyFolderLocker::OnBnClickedButtonFolderremove)
	ON_CBN_SELENDOK(IDC_COMBO_OPTION_FOLDERLOCK, &CISpyFolderLocker::OnCbnSelendokComboOptionFolderlock)
	ON_BN_CLICKED(IDC_BUTTON_START_FOLDERACTION, &CISpyFolderLocker::OnBnClickedButtonStartFolderaction)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_FOLDERLOCK, &CISpyFolderLocker::OnLvnItemchangedListFolderlock)
	ON_BN_CLICKED(IDC_CHECK_BOX, &CISpyFolderLocker::OnBnClickedCheckBox)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CISpyFolderLocker message handlers
/**********************************************************************************************************                     
*  Function Name  :	OnInitDialog                                                     
*  Description    :	Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft 
					Foundation Class Library dialog boxes
*  SR.N0		  : 
*  Author Name    : Neha Gharge                                                                                          
*  Date           : 11 Jan 2014
**********************************************************************************************************/
BOOL CISpyFolderLocker::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);

	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_INNER_DIALOG), _T("JPG")))
	{
		m_bIsPopUpDisplayed = true;
		::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_FAILMSG"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
		m_bIsPopUpDisplayed = false;
	}

	Draw();

	m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));

	CRect rect;
	this->GetClientRect(rect);

	
    m_stFolderLlckrHeader.SetBkColor(RGB(230,232,238));
    m_stFolderLlckrHeader.SetTextColor(RGB(24,24,24));
   	m_stFolderLlckrHeader.SetWindowPos(&wndTop,rect.left +20,07,400,31,SWP_NOREDRAW);
    m_stFolderLlckrHeader.SetFont(&theApp.m_fontWWTextTitle);

 
    m_stFolderLlckrSubHeader.SetBkColor(RGB(230,232,238));
	m_stFolderLlckrSubHeader.SetTextColor(RGB(24,24,24));
	m_stFolderLlckrSubHeader.SetWindowPos(&wndTop,rect.left +20,35,400,15,SWP_NOREDRAW);
    m_stFolderLlckrSubHeader.SetFont(&theApp.m_fontWWTextSubTitleDescription);

	if(theApp.m_dwOSType == WINOS_WIN8 ||theApp.m_dwOSType == WINOS_WIN8_1)
	{
		m_stFolderLlckrHeader.SetWindowPos(&wndTop,rect.left +20,14,400,31,SWP_NOREDRAW);	
		m_stFolderLlckrSubHeader.SetWindowPos(&wndTop,rect.left +20,38,400,15,SWP_NOREDRAW);
	}
    
	m_bmpHeaderFolderLockPic = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_COM_HEADER));
	m_stHeaderFolderLockPic.SetBitmap(m_bmpHeaderFolderLockPic);
	m_stHeaderFolderLockPic.SetWindowPos(&wndTop,rect.left +6,10,582,45, SWP_NOREDRAW);

	/*****************ISSUE NO -219 Neha Gharge 22/5/14 ************************************************/
	
	m_edtFolderLockPath.SetWindowPos(&wndTop,rect.left +6,87,445,22, SWP_NOREDRAW);


	m_btnFolderLockBrowse.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnFolderLockBrowse.SetWindowPos(&wndTop,rect.left +463, 87, 57, 21, SWP_NOREDRAW);
//	m_btnFolderLockBrowse.SetWindowTextW(L"Browse");
	m_btnFolderLockBrowse.SetTextColorA(BLACK,1,1);
	//m_btnFolderLockBrowse.SetWindowPos(&wndTop,rect.left +555,110,75,21, SWP_NOREDRAW);

	m_comboFolderLockOption.SetWindowPos(&wndTop,rect.left +508,85,130,25, SWP_NOREDRAW);//555,85,160,25
	m_comboFolderLockOption.AddString(g_OptionsForFolderLock[0]);
	m_comboFolderLockOption.AddString(g_OptionsForFolderLock[1]);
	m_comboFolderLockOption.SetCurSel(0);
	m_comboFolderLockOption.ShowWindow(SW_HIDE);

	
	m_stInfoText.SetWindowPos(&wndTop,rect.left +6,118,586,21, SWP_NOREDRAW);
	m_stInfoText.SetBkColor(RGB(105,105,105));
	m_stInfoText.SetTextColor(WHITE);
	m_stInfoText.SetFont(&theApp.m_fontWWTextNormal);
	
	//m_lstFolderLock.InsertColumn(0, L"Sr.No.", LVCFMT_LEFT, 45);
	m_lstFolderLock.InsertColumn(0,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_LST"), LVCFMT_LEFT, 578);
	m_lstFolderLock.SetFont(&theApp.m_fontWWTextNormal);
	//m_lstFolderLock.InsertColumn(2, L"Status", LVCFMT_LEFT, 185);
	ListView_SetExtendedListViewStyle (m_lstFolderLock.m_hWnd, LVS_EX_CHECKBOXES |  LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);
	m_lstFolderLock.SetWindowPos(&wndTop,rect.left +6,165,582,175, SWP_NOREDRAW);

	// *****************************ISSUE NO : 374 Neha gharge 24/5/2014 *******************************************/
	m_chkSelectAll.SetWindowPos(&wndTop,rect.left +8,350,13,13, SWP_NOREDRAW);
	m_chkSelectAll.SetCheck(1);

	m_stSelectAll.SetWindowPos(&wndTop,rect.left +27,349,80,15, SWP_NOREDRAW);
	m_stSelectAll.SetBkColor(RGB(70,70,70));
	m_stSelectAll.SetTextColor(WHITE);
	

	m_btnStartFolderAction.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnStartFolderAction.SetWindowPos(&wndTop,rect.left +530,87,57,21,SWP_NOREDRAW | SWP_NOZORDER);//505,330,65,21
//	m_btnStartFolderAction.SetWindowTextW(L"Lock");
	m_btnStartFolderAction.SetTextColorA(BLACK,1,1);

	m_btnFolderOpen.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnFolderOpen.SetWindowPos(&wndTop,rect.left +579,330,65,21,SWP_NOREDRAW | SWP_NOZORDER);
	//m_btnFolderOpen.SetWindowTextW(L"Open");
	m_btnFolderOpen.SetTextColor(WHITE);
	m_btnFolderOpen.ShowWindow(SW_HIDE);

	m_btnFolderRemove.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnFolderRemove.SetWindowPos(&wndTop,rect.left +531,360,57,21,SWP_NOREDRAW | SWP_NOZORDER);
	//m_btnFolderRemove.SetWindowTextW(L"Unlock");
	m_btnFolderRemove.SetTextColorA(BLACK,1,1);

	//m_btnBack.SetSkin(IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROWDISABLE,0,0,0,0);
	//m_btnBack.SetWindowPos(&wndTop, rect.left + 21, 354, 31,32, SWP_NOREDRAW);
	//m_btnBack.ShowWindow(SW_HIDE);*/

	
	m_btnFolderOpen.EnableWindow(false);
	m_btnFolderRemove.EnableWindow(true);
	//m_btnBack.EnableWindow(true);

    RefreshStrings();
	LoadRegistartionDll();
	GetEnviormentVariablesForAllMachine();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/**********************************************************************************************************                     
*  Function Name  :	PreTranslateMessage                                                     
*  Description    : To translate window messages before they are dispatched 
				    to the TranslateMessage and DispatchMessage Windows functions
*  SR.N0		  : 
*  Author Name    :	Neha Gharge                                                                                   
*  Date           : 11 Jan 2014
**********************************************************************************************************/
BOOL CISpyFolderLocker::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

/***************************************************************************
* Function Name   : OnNcHitTest
*  Description    : The framework calls this member function for the CWnd object
				    that contains the cursor (or the CWnd object that used the 
				    SetCapture member function to capture the mouse input) every 
				    time the mouse is moved.
*  Author Name    : Neha Gharge.
*  SR_NO		  :
*  Date           : 11 Jan 2014
****************************************************************************/
LRESULT CISpyFolderLocker::OnNcHitTest(CPoint point)
{
	return HTCLIENT;
}


/**********************************************************************************************************                     
*  Function Name  :	OnSetCursor                                                     
*  Description    :	The framework calls this member function if mouse input is not captured and the mouse 
					causes cursor movement within 
					the CWnd object.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
BOOL CISpyFolderLocker::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if( 
		iCtrlID == IDC_BUTTON_FOLDERLOCK_BACK ||
		iCtrlID == IDC_BUTTON_FOLDEROPEN      ||
		iCtrlID == IDC_BUTTON_FOLDERREMOVE    ||
		iCtrlID == IDC_BUTTON_FOLDER_BROWSE	  ||
		iCtrlID == IDC_BUTTON_START_FOLDERACTION
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
*  Function Name  :	OnBnClickedButtonFolderlockBack                                                     
*  Description    :	Show Parent window.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CISpyFolderLocker::OnBnClickedButtonFolderlockBack()
{
	// TODO: Add your control notification handler code here
	this->ShowWindow(SW_HIDE);
	CISpyToolsDlg *pObjToolWindow = reinterpret_cast<CISpyToolsDlg*>(this->GetParent());
	pObjToolWindow->ShowHideToolsDlgControls(true);
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonFolderBrowse                                                     
*  Description    :	Browse all folder from computer
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CISpyFolderLocker::OnBnClickedButtonFolderBrowse()
{
	// TODO: Add your control notification handler code here
	/*****************ISSUE NO -222 Neha Gharge 22/5/14 ************************************************/
	TCHAR *pszPath = new TCHAR[MAX_PATH];
	SecureZeroMemory(pszPath, MAX_PATH*sizeof(TCHAR));

	CString csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_SELECTFLDR");
	BROWSEINFO        bi = {m_hWnd, NULL, pszPath, csMessage, BIF_RETURNONLYFSDIRS, NULL, 0L, 0};
	LPITEMIDLIST      pIdl = NULL;

	LPITEMIDLIST  pRoot = NULL;
	//SHGetFolderLocation(m_hWnd, CSIDL_DRIVES, 0, 0, &pRoot);
	bi.pidlRoot = pRoot;
	pIdl = SHBrowseForFolder(&bi);
	if(NULL != pIdl)
	{
		SHGetPathFromIDList(pIdl, pszPath);
		size_t iLen = wcslen(pszPath);
		if(iLen > 0)
		{
			
			m_csPathName = pszPath;
			if(m_csPathName.Right(1) == L"\\")
			{
				m_csPathName = m_csPathName.Left(static_cast<int>(iLen -1));
			}
			m_edtFolderLockPath.SetWindowText(m_csPathName);
		}
	}
	delete [] pszPath;
	pszPath = NULL;

}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonFolderopen                                                     
*  Description    :	Handle open button handler
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CISpyFolderLocker::OnBnClickedButtonFolderopen()
{
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonFolderremove                                                     
*  Description    :	This function is used to unlock folders
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CISpyFolderLocker::OnBnClickedButtonFolderremove()
{
	//m_btnBack.EnableWindow(false);
	TCHAR	szTemp[512] = {0} ;
	TCHAR	*pName = NULL ;
	int index = 0;

	index = m_lstFolderLock.GetItemCount() ;
	if( index == 0x00 )
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_NOENTRY"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION) ;
		m_bIsPopUpDisplayed = false;
		//m_btnBack.EnableWindow(true);
		return;
	}

	if(!isAnyEntrySeletected())
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_ENTRY"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
		m_bIsPopUpDisplayed = false;
		//m_btnBack.EnableWindow(true);
		return;
	}
	
	GetModuleFileName( NULL, szTemp, 511 ) ;
	pName = wcsrchr( szTemp, '\\' ) ;
	if( !pName )
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_AV_NT_FOUND"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
		m_bIsPopUpDisplayed = false;
		//m_btnBack.EnableWindow(true);
		return ;
	}

	*pName = '\0' ;
	wcscat( szTemp, TEXT("\\WRDWIZEVALREG.DLL") ) ;
	if( !PathFileExists( szTemp ) )
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_REG"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
		m_bIsPopUpDisplayed = false;
		//m_btnBack.EnableWindow(true);
		return ;
	}

	
	if( !GetfolderRegisteredData )
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_REGDATA"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
		m_bIsPopUpDisplayed = false;
		//m_btnBack.EnableWindow(true);
		return ;
	}

	m_bIsPopUpDisplayed = true;
	if(MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_CONFIRM"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONQUESTION|MB_YESNO)== IDNO)
	{
		m_bIsPopUpDisplayed = false;
		//m_btnBack.EnableWindow(true);
		return ;
	}
	m_bIsPopUpDisplayed = false;

	DWORD			i = 0x00 ;
	DWORD			iPassLen = 0x00 ;
	DWORD			dwSize = 0x00 ;
	unsigned char	chPass = 0x00 ;
	BYTE			szPassWord[0x10] = {0} ;
	TCHAR			szPass[0x20] = {0} ;
	CString			strLockPass("") ;
	CString			strUnlockPass("") ;
	

	GetfolderRegisteredData(szPassWord, dwSize, IDR_RESDATA_DES, TEXT("WRDWIZOPTIONRESDES") ) ;
	if( szPassWord[0] && dwSize)
	{
		while( szPassWord[i] != 0x00 )
		{
			szPass[i] = szPassWord[i] ;
			i++ ;
		}
		g_csFolderLockerPassword.Format( TEXT("%s"), szPass ) ;
		strLockPass = g_csFolderLockerPassword ;
		bRESFOLDER_DES = true ;
	}

	if(g_csFolderLockerPassword.GetLength())
	{

		//CISpyPasswordPopUpDlg  objPasswordPopUpMsgDlgOpt;
		m_objFolderLockPassword.m_bSelectComboOption = false;

		m_bIsPopUpDisplayed = true;
		int iRet = static_cast<int>(m_objFolderLockPassword.DoModal());
		m_bIsPopUpDisplayed = true;

		if(iRet == IDOK)
		{
			strUnlockPass = m_objFolderLockPassword.m_csEditText;
		}
		else
			goto CleanUp;
		
	}
	int Ret;
	if((strUnlockPass.Compare(strLockPass)))
	{
		m_bIsPopUpDisplayed = true;
		Ret = MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FOLDER_LCK_PASSWORD_MISMATCH"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION);
		m_bIsPopUpDisplayed = false;

		//Ret = IDCANCEL;
		goto CleanUp;
	}
	else
	{

		g_hThread_Unlock = ::CreateThread(NULL, 0, UnlockFileORFolderThread, (LPVOID) this, 0, NULL);
		Sleep( 1000 ) ;
		
		//dwtype = 2 becuase of it is unlock event. Sending to service as unlock event
		if(!SendFolderEntries2Service(APPLY_FOLDER_LOCKER_SETTING, 2, L"",true))
		{
			AddLogEntry(L"### Error in Setting CGenXFolderLocker::Apply folder lock setting", 0, 0, true, SECONDLEVEL);
		}
		Sleep(20);
	}


CleanUp:;

	//m_btnFolderOpen.EnableWindow(false);
	//m_btnFolderRemove.EnableWindow(true);
	//m_btnBack.EnableWindow(true);
}


/**********************************************************************************************************                     
*  Function Name  :	OnCbnSelendokComboOptionFolderlock                                                     
*  Description    :	Combo box options
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CISpyFolderLocker::OnCbnSelendokComboOptionFolderlock()
{
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonStartFolderaction                                                     
*  Description    :	It used to lock folder.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CISpyFolderLocker::OnBnClickedButtonStartFolderaction()
{
	// TODO: Add your control notification handler code here
	CString csFilePath,csStatus;
	TCHAR	szTemp[512] = {0} ;
	TCHAR	*pName = NULL ;
	
	if (theApp.m_dwDaysLeft == 0)
	{
		theApp.GetDaysLeft();
	}

	if(theApp.m_dwDaysLeft == 0)
	{
		if(!theApp.ShowEvaluationExpiredMsg())
		{
			theApp.GetDaysLeft();
			return;
		}
	}

	if(!SendFolderEntries2Service(RELOAD_DBENTRIES , FOLDERLOCKER , L"",true))
	{
		AddLogEntry(L"### Failed CGenXFolderLocker::SendFolderEntries2Service", 0, 0, true, SECONDLEVEL);
	}


	m_edtFolderLockPath.GetWindowText(csFilePath);
	if(csFilePath == L"")
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_NOTIFY"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION);
		m_bIsPopUpDisplayed = false;
		return;
	}
	_tcscpy(m_szBrowseFilePath,csFilePath);
	
	if(!CheckIsParentFolderLocked((wchar_t*)m_szBrowseFilePath))
	{
		return;
	}

	//for(n=0;n<m_lstFolderLock.GetItemCount()+1;n++)
	//{
	//	if(csFilePath == m_lstFolderLock.GetItemText(n,0))
	//	{// ISSue No. 510 RY 30th June 2014.
	//		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_ALREADY_LOCKED"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONINFORMATION|MB_OK);
	//		m_edtFolderLockPath.SetWindowTextW(L"");
	//		return;
	//	}
	//}

	if(!AvoidOSPath())
	{
		// ISSue No. 510 RY 30th June 2014.
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_CANTBE_LOCKED"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
		m_bIsPopUpDisplayed = false;
		m_edtFolderLockPath.SetWindowTextW(L"");
		return;
	}

	GetModuleFileName( NULL, szTemp, 511 ) ;
	pName = wcsrchr( szTemp, '\\' ) ;
	if( !pName )
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_AV_NT_FOUND"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
		m_bIsPopUpDisplayed = false;

		return ;
	}

	*pName = '\0' ;
	wcscat( szTemp, TEXT("\\WRDWIZEVALREG.DLL") ) ;
	if( !PathFileExists( szTemp ) )
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_REG"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
		m_bIsPopUpDisplayed = false;
		return;
	}

	
	if( !GetfolderRegisteredData )
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_REGDATA"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
		m_bIsPopUpDisplayed = false;
		return ;
	}
	m_btnFolderLockBrowse.EnableWindow(false);
	//m_btnBack.EnableWindow(false);
	

	DWORD			i = 0x00 ;
	DWORD			iPassLen = 0x00 ;
	DWORD			dwSize = 0x00 ;
	unsigned char	chPass = 0x00 ;
	BYTE			szPassWord[0x10] = {0} ;
	TCHAR			szPass[0x20] = {0} ;
	CString			strLockPass("") ;
	CString			strUnlockPass("") ;


	GetfolderRegisteredData(szPassWord, dwSize, IDR_RESDATA_DES, TEXT("WRDWIZOPTIONRESDES") ) ;
	//if( szPassWord[0] && dwSize)
	//{
	//	while( szPassWord[i] != 0x00 )
	//	{
	//		szPass[i] = szPassWord[i] ;
	//		i++ ;
	//	}
	//	m_csFolderLockerPassword.Format( TEXT("%s"), szPass ) ;
	//	strLockPass = m_csFolderLockerPassword ;
	//	bRESFOLDER_DES = true ;
	//}
	g_csFolderLockerPassword = szPassWord;
	
	if(!g_csFolderLockerPassword.GetLength())
	{
		g_csFolderLockerPassword = "" ;
		//CISpyPasswordPopUpDlg  objPasswordPopUpMsgDlgOpt;
		m_objFolderLockPassword.m_bSelectComboOption = true;
		g_bLock = m_objFolderLockPassword.m_bSelectComboOption;
		
			
		if( !ValidatePassword(g_csFolderLockerPassword) )
		{
			m_dwLockpasswrd = Str2Hex(g_csFolderLockerPassword ) ;
		}
		else
		{
			m_btnFolderLockBrowse.EnableWindow(true);
			//MessageBox(L"Please enter valid Password.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION | MB_OK) ;
			return ;
		}

	}
	

	if( !g_csFolderLockerPassword.GetLength() )
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_ENTER_PSSWRD"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
		m_bIsPopUpDisplayed = false;
		return ;
	}

	m_dwLockpasswrd = Str2Hex( g_csFolderLockerPassword ) ;

	i = 0x00 ;
	iPassLen = g_csFolderLockerPassword.GetLength() ;
	if( !dwSize ) 
	{
		BYTE	szPass[0x10] = {0} ;

		for(i=0; i<iPassLen; i++ )
		{
			chPass = static_cast<BYTE>(g_csFolderLockerPassword.GetAt(i));
			szPass[i] = chPass ;
		}

		if(!SendRegisteredData2Service(ADD_REGISTRATION_DATA, szPass, iPassLen, IDR_RESDATA_DES, TEXT("WRDWIZOPTIONRESDES")))
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_TRYAGAIN"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK) ;
			m_bIsPopUpDisplayed = false;
			return ;
		}
		Sleep(10);
	}
	//Insert item in list control and save it into DB file and send lock event to service.
	InsertItem(csFilePath);
	
	
	m_btnFolderOpen.EnableWindow(false);
	m_btnFolderRemove.EnableWindow(true);
	
	
	
	Sleep(1000);
	m_edtFolderLockPath.SetWindowTextW(L"");
	//m_btnBack.EnableWindow(true);
	m_btnFolderLockBrowse.EnableWindow(true);

}

/**********************************************************************************************************                     
*  Function Name  :	InsertItem                                                     
*  Description    :	Insert the list of locked folders.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CISpyFolderLocker::InsertItem(CString csFilePath)
{
	CString csAddEntry = L"";
	//TCHAR * szAddEntry = L"";
	LVITEM lvItem;
	int nItem = 0;
	int imgNbr = 0;

	lvItem.mask = LVIF_IMAGE | LVIF_TEXT;;
	lvItem.iItem = 0;
	lvItem.iSubItem = 0;
	lvItem.pszText =(LPTSTR)(LPCTSTR)(csFilePath);

	m_lstFolderLock.InsertItem(&lvItem);
	CString cstmp;
	
	m_lstFolderLock.SetItemText(nItem, 0, csFilePath);
	m_lstFolderLock.SetCheck(nItem, TRUE);

	csAddEntry.Format(L"%s#\n",csFilePath);
	if(!SendFolderEntries2Service(ADD_FOLDER_LOCKER_ENTRY , FOLDERLOCKER , csAddEntry,true))
	{
		AddLogEntry(L"### Failed CGenXFolderLocker::SendFolderEntries2Service", 0, 0, true, SECONDLEVEL);
	}
	Sleep(20);	

	if(!SendFolderEntries2Service(SAVE_FOLDER_LOCKER_ENTRY , FOLDERLOCKER , L"",true))
	{
		AddLogEntry(L"### Failed CGenXFolderLocker::SendFolderEntries2Service", 0, 0, true, SECONDLEVEL);
	}
	Sleep(20);

	//dwtype = 1 becuase of it is lock event. Sending to service as lock event.
	if(!SendFolderEntries2Service(APPLY_FOLDER_LOCKER_SETTING, 1, L"",true))
	{
		AddLogEntry(L"### Error in Setting CGenXFolderLocker::Apply folder lock setting", 0, 0, true, SECONDLEVEL);
	}
	else
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_SUCCESS"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONINFORMATION|MB_OK);
		m_bIsPopUpDisplayed = false;
	}
	
}

/**********************************************************************************************************                     
*  Function Name  :	SendFolderEntries2Service                                                     
*  Description    :	It used to stored locked folder path name to Db files ,communicating with comm service 
					throught named pipe.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
bool CISpyFolderLocker::SendFolderEntries2Service(DWORD dwAddEditDelType, DWORD dwType, CString csEntry,bool bFolderLockWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	szPipeData.iMessageInfo = static_cast<int>(dwAddEditDelType);
	szPipeData.dwValue = dwType;
	_tcscpy(szPipeData.szFirstParam , csEntry);
	CISpyCommunicator objCom(SERVICE_SERVER, true);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send RESUME_SCAN data in SendFolderEntries2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}
	if(bFolderLockWait)
	{
		if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to Read RESUME_SCAN data in SendFolderEntries2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
		if(szPipeData.dwValue != 1)
		{
			return false;
		}
	}
	return true;
}


/**********************************************************************************************************                     
*  Function Name  :	LoadExistingFolderLockerEntriesFile                                                     
*  Description    :	Loading existing DB file data into list control.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CISpyFolderLocker::LoadExistingFolderLockerEntriesFile()
{
	static CString csExistingFilePath;
	TCHAR szModulePath[MAX_PATH] = {0};
	if(!GetModulePath(szModulePath, MAX_PATH))
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_MODULE_PATH"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR|MB_OK);
		m_bIsPopUpDisplayed = false;
		return;
	}
	csExistingFilePath = szModulePath;
	csExistingFilePath += _T("\\WRDWIZFOLDERLOCKERENTRIES.DB");

	LoadDataContentFromFile(csExistingFilePath);
}

/**********************************************************************************************************                     
*  Function Name  :	LoadDataContentFromFile                                                     
*  Description    :	Loading existing DB file data into object.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CISpyFolderLocker::LoadDataContentFromFile(CString csPathName)
{
	if( !PathFileExists( csPathName ) )
		return;
	
	CFile rFile(csPathName, CFile::modeRead);
	 
	// Create a loading archive
	CArchive arLoad(&rFile, CArchive::load);

	m_objFolderLocker.Serialize(arLoad);

	// Close the loading archive
	arLoad.Close();
	rFile.Close();

	PopulateList();
}


/**********************************************************************************************************                     
*  Function Name  :	PopulateList                                                     
*  Description    :	Populate DB list in list control.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CISpyFolderLocker::PopulateList()
{
	int nCurrentItem;
	CString csSrNo;
	// delete all current members
	m_lstFolderLock.DeleteAllItems();


	// get a reference to the contacts list
	const ContactList& contacts = m_objFolderLocker.GetContacts();

	// iterate over all contacts add add them to the list

	POSITION pos = contacts.GetHeadPosition();
	
	while(pos != NULL)
	{
		nCurrentItem=0;	
		const CIspyList contact = contacts.GetNext(pos);
		LVITEM lvi;
		// Insert the first item
		lvi.mask =  LVIF_IMAGE | LVIF_TEXT;
		lvi.iItem = nCurrentItem;
		lvi.iSubItem = 0;
		lvi.pszText =(LPTSTR)(LPCTSTR)(contact.GetFirstEntry());
		
		m_lstFolderLock.InsertItem(&lvi);

	//	m_lstFolderLock.SetItemText(nCurrentItem, 1, contact.GetFirstEntry());
		m_lstFolderLock.SetCheck(nCurrentItem, TRUE);

			
	}
}

/**********************************************************************************************************                     
*  Function Name  :	OnLvnItemchangedListFolderlock                                                     
*  Description    :	After Item Changed from list
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CISpyFolderLocker::OnLvnItemchangedListFolderlock(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	m_btnFolderOpen.EnableWindow(true);
	m_btnFolderRemove.EnableWindow(true);
	*pResult = 0;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedCheckBox                                                     
*  Description    :	Check or uncheck select all check box
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CISpyFolderLocker::OnBnClickedCheckBox()
{
	// TODO: Add your control notification handler code here
	int iCheck = m_chkSelectAll.GetCheck();
	
	for(int i = 0; i < m_lstFolderLock.GetItemCount(); i++)
	{
		m_lstFolderLock.SetCheck(i, iCheck);
	}
	
}

/**********************************************************************************************************                     
*  Function Name  :	UnlockFileORFolderThread                                                     
*  Description    :	Unlock folder one by one and removes entry from db file
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
DWORD WINAPI UnlockFileORFolderThread( LPVOID lpParam )
{

	CISpyFolderLocker *pRptDlg = (CISpyFolderLocker * ) lpParam ;

	if( !pRptDlg )
		return 0 ;
	pRptDlg->m_btnFolderLockBrowse.EnableWindow(FALSE);
	pRptDlg->m_btnStartFolderAction.EnableWindow( FALSE);
	pRptDlg->m_btnFolderRemove.EnableWindow( FALSE ) ;
	pRptDlg->m_bUnlockingStop = false;
	pRptDlg->m_bUnlockingThreadStart = true;
	bool	bSelected = false ;
	DWORD	dwRet = 0x00 ;
	CString csFilePath,csRemoveEntry;
	int		index = 0 ;
	DWORD	dwunlockEntries = 0x00;

	try
	{

		index = pRptDlg->m_lstFolderLock.GetItemCount() ;
		if( index == 0x00 )
		{
			pRptDlg->m_bIsPopUpDisplayed = true;
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_NOENTRY"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION) ;
			pRptDlg->m_bIsPopUpDisplayed = false;
			goto Cleanup ;
		}

		if(!pRptDlg->SendFolderEntries2Service(RELOAD_DBENTRIES, FOLDERLOCKER, L"", true))
		{
			AddLogEntry(L"### Failed to SendFolderEntries2Service::Reload unlock", 0, 0, true, SECONDLEVEL);
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		do
		{
			for(index = 0; index < pRptDlg->m_lstFolderLock.GetItemCount(); index ++)
			{
				if(pRptDlg->m_bUnlockingStop)
				{
					break;
				}
				if(pRptDlg->m_lstFolderLock.GetCheck(index) )
				{
					csFilePath = pRptDlg->m_lstFolderLock.GetItemText(index, 0).Trim();
					csRemoveEntry.Format(L"%s#",csFilePath);
					if(pRptDlg->SendFolderEntries2Service(DELETE_FOLDER_LOCKER_ENTRY , FOLDERLOCKER , csRemoveEntry, true))
					{
						pRptDlg->m_lstFolderLock.DeleteItem(index) ;
					}
					else
					{
						AddLogEntry(L"### Failed to SendFolderEntries2Service::unlock", 0, 0, true, SECONDLEVEL);
					}
					Sleep(10);
					bSelected = true ;
					dwunlockEntries++ ;
					break ;
				}
				//index--;
			}

			if( index >= pRptDlg->m_lstFolderLock.GetItemCount() )
				break ;

			if(pRptDlg->m_bUnlockingStop)
			{
				break;
			}

		}while( bSelected ) ;
		
		
		if( !bSelected )
		{
			pRptDlg->m_bIsPopUpDisplayed = true;
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_NOENTRY"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION );
			pRptDlg->m_bIsPopUpDisplayed = false;
			//return 2 ;
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		if(!pRptDlg->SendFolderEntries2Service(SAVE_FOLDER_LOCKER_ENTRY,FOLDERLOCKER, L"", true))
		{
			AddLogEntry(L"### Failed to send SAVE_FOLDER_ENTRIES in CGenXFolderLocker : SendFolderEntries2Service", 0, 0, true, SECONDLEVEL);
		}

	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CGenXFolderLocker::OnBnClickedBtnRemove", 0, 0, true, SECONDLEVEL);
	}

	if(!pRptDlg->m_bUnlockingStop)
	{
		pRptDlg->m_bIsPopUpDisplayed = true;
		if (dwunlockEntries > 0x01)
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_UNLOCK_SUCCESS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
		}
		else
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_UNLOCK_SUCCESS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
		}
		pRptDlg->m_bIsPopUpDisplayed = false;
	}
	

Cleanup :
	pRptDlg->m_btnFolderRemove.EnableWindow(true) ;
	pRptDlg->m_btnFolderLockBrowse.EnableWindow(true);
	pRptDlg->m_btnStartFolderAction.EnableWindow(true);
	//pRptDlg->m_btnBack.EnableWindow(true);
	pRptDlg->m_bUnlockingThreadStart = false;
	return dwRet ;
}


/**********************************************************************************************************                     
*  Function Name  :	Str2Hex                                                     
*  Description    :	Convert String to DWORD
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
DWORD CISpyFolderLocker::Str2Hex( CString const & s )
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
*  Function Name  :	SendRegisteredData2Service                                                     
*  Description    :	Send password or registartion data to Wardwizevalreg.dll throught named pipe
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
bool CISpyFolderLocker::SendRegisteredData2Service(DWORD dwType, LPBYTE lpResBuffer, DWORD dwResSize, DWORD dwResType, TCHAR *pResName)
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
		AddLogEntry(L"### Failed to send data in CGenXFolderLocker : SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CDataEncryptionDlg : SendDataEncryptionOperation2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(szPipeData.dwValue != 1)
	{
		return false;
	}

	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	ValidatePassword                                                     
*  Description    :	Check the valid password, if not valid again popup comes
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
DWORD CISpyFolderLocker::ValidatePassword(CString csPassword)
{
	int		iRet = 0x00 ;
	if(csPassword.GetLength() == 0 && g_bLock == false)
	{
		iRet = PopUpDialog() ;
		if( iRet == IDCANCEL )
		{
			iRet = 0x01;
			//break ;
		}
		if( iRet == IDOK )
			iRet = 0x00 ;

	}
	else
	{
		bool	bValid = ValidateGivenPassWord( g_csFolderLockerPassword ) ;
		if(bValid == false && g_bLock == false)
		{
			iRet = 0x00 ;
			return iRet;
		}
		while( !bValid )
		{
			iRet = PopUpDialog() ;
			if( iRet == IDCANCEL )
			{
				iRet = 0x01;
				break ;
			}
			if( iRet == IDOK )
				iRet = 0x00 ;

			bValid = ValidateGivenPassWord( g_csFolderLockerPassword ) ;
		}
	}
	return iRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	ValidateGivenPassWord                                                     
*  Description    :	Check the valid password, whether it contains atlst 1 no,1 special char,password length.
*  Author Name    :	Neha Gharge 
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
bool CISpyFolderLocker::ValidateGivenPassWord(CString csPassword)
{
	int			i,count=0 ;
	CString		csInputKey ;
	csInputKey = csPassword;

	int		Validlenght = csInputKey.GetLength();

	if(Validlenght==0)
	{
		return false ;
	}

	if(Validlenght<=4)
	{
		return false ;
	}
	if(Validlenght>=8)
	{
		return false ;
	}
	if((csInputKey.FindOneOf(L"~`!@#$%^&*()_-+={}[]|\?/.,':;<>")==-1))
	{
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
		return false ;
	}

	return true ;
}


/**********************************************************************************************************                     
*  Function Name  :	PopUpDialog                                                     
*  Description    :	Pop up folder locker password dialog
*  Author Name    :	Neha Gharge 
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
int CISpyFolderLocker:: PopUpDialog()
{
	m_bIsPopUpDisplayed = true;
	int iRet = static_cast<int>(m_objFolderLockPassword.DoModal());
	m_bIsPopUpDisplayed = false;

	if( iRet == IDCANCEL )
	{
		g_csFolderLockerPassword = "" ;
		return iRet ;
	}

	if(iRet==IDOK)
	{
		g_csFolderLockerPassword = m_objFolderLockPassword.m_csEditText;
		if(!ValidatePassword(g_csFolderLockerPassword))
		{
			m_dwLockpasswrd = Str2Hex(g_csFolderLockerPassword);
		}

	}

	return iRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	LoadRegistartionDll                                                     
*  Description    :	Load WRDWIZREGISTERDATA.DLL and proc address of GetfolderRegisteredData
*  Author Name    :	Neha Gharge 
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CISpyFolderLocker::LoadRegistartionDll()
{
	TCHAR	szTemp[512] = {0} ;
	TCHAR	*pName = NULL ;
	m_hRegisterDLL = NULL;
	GetModuleFileName(NULL, szTemp, 511 ) ;
	pName = wcsrchr(szTemp, '\\' ) ;
	if( pName )
		*pName = '\0' ;

	wcscat( szTemp, TEXT("\\WRDWIZREGISTERDATA.DLL") ) ;

	if( PathFileExists( szTemp ) )
	{
		if( !m_hRegisterDLL )
			m_hRegisterDLL = LoadLibrary( szTemp ) ;

		if( !GetfolderRegisteredData )
			GetfolderRegisteredData = (GETREGISTEREFOLDERDDATA) GetProcAddress(m_hRegisterDLL, "GetRegisteredData") ;
	}
}

/**********************************************************************************************************                     
*  Function Name  :	GetEnviormentVariablesForAllMachine                                                     
*  Description    :	Get enviorment varibles for all OS.
*  Author Name    :	Neha Gharge 
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CISpyFolderLocker::GetEnviormentVariablesForAllMachine()
{

	memset(m_szProgramData, 0x00, sizeof(TCHAR)*512 ) ;
	//memset(m_szUserProfile, 0x00, sizeof(TCHAR)*512 ) ;
	memset(m_szWindow, 0x00, sizeof(TCHAR)*512 ) ;
	//memset(m_szAppData, 0x00, sizeof(TCHAR)*512 ) ;
	memset(m_szAppData4, 0x00, sizeof(TCHAR)*512 ) ;
	memset(m_szDriveName, 0x00, sizeof(TCHAR)*512 ) ;
	memset(m_szArchitecture, 0x00, sizeof(TCHAR)*512 ) ;
	memset(m_szProgramFile, 0x00, sizeof(TCHAR)*512 ) ;
	memset(m_szProgramFilex86, 0x00, sizeof(TCHAR)*512 ) ;
	GetEnvironmentVariable(TEXT("ALLUSERSPROFILE"), m_szProgramData, 511 ) ;//program data
	//MessageBox(m_szProgramData,L"",MB_OK);
	//GetEnvironmentVariable(TEXT("USERPROFILE"), m_szUserProfile, 511 ) ;
	//MessageBox(m_szUserProfile,L"",MB_OK);
	GetEnvironmentVariable(TEXT("windir"), m_szWindow, 511 ) ;//windows
	//MessageBox(m_szWindow,L"",MB_OK);
	//GetEnvironmentVariable(TEXT("APPDATA"), m_szAppData, 511 ) ;
	//MessageBox(m_szAppData,L"",MB_OK);
	GetEnvironmentVariable(TEXT("PROCESSOR_ARCHITECTURE"), m_szAppData4, 511 ) ;
	//MessageBox(m_szAppData4,L"",MB_OK);
	GetEnvironmentVariable(TEXT("SystemDrive"), m_szDriveName, 511 ) ;
	//MessageBox(m_szDriveName,L"",MB_OK);
	GetEnvironmentVariable(TEXT("PROCESSOR_ARCHITEW6432"), m_szArchitecture, 511 ) ;
	//MessageBox(m_szAppData6,L"",MB_OK);
	if(!_tcscmp(m_szArchitecture,L""))
	{
		//MessageBox(L"32bit machine",L"",MB_OK);
		_tcscpy(m_szProgramFile,L"C:\\Program Files");
		
	}
	else
	{
		
		//MessageBox(L"64bit machine",L"",MB_OK);
		_tcscpy(m_szProgramFilex86,L"C:\\Program Files (x86)");
		_tcscpy(m_szProgramFile,L"C:\\Program Files");
	}
}


/**********************************************************************************************************                     
*  Function Name  :	AvoidOSPath                                                     
*  Description    :	Avoid to lock system folder.User can't lock system folders
*  Author Name    :	Neha Gharge 
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
bool CISpyFolderLocker::AvoidOSPath()
{
	bool bReturn = true;
    TCHAR szPathName[512];
	
	_tcscpy(szPathName,m_csPathName);
	
	if(!_tcsncmp(szPathName,m_szProgramData,_tcslen(m_szProgramData)))
	{
		bReturn = false;
	}
	if(!_tcsncmp(szPathName,m_szWindow,_tcslen(m_szWindow)))
	{
		bReturn = false;
	}
	//if(!_tcsncmp(szPathName,m_szAppData,_tcslen(m_szAppData)))
	//{
	//	bReturn = false;
	//}
	if(!_tcsncmp(szPathName,m_szAppData4,_tcslen(m_szAppData4)))
	{
		bReturn = false;
	}
	if(!_tcscmp(szPathName,m_szDriveName))
	{
		bReturn = false;
	}
	if(!_tcsncmp(szPathName,m_szProgramFile,_tcslen(m_szProgramFile)))
	{
		bReturn = false;
	}
	if(_tcscmp(m_szProgramFilex86,L""))
	{
		if(!_tcsncmp(szPathName,m_szProgramFilex86,_tcslen(m_szProgramFilex86)))
		{
			bReturn = false;
		}
	}
//	if(!_tcsncmp(szPathName,m_szUserProfile,_tcslen(m_szUserProfile)))
//	{
//		bReturn = false;
//	}
	return bReturn;
}


/**********************************************************************************************************                     
*  Function Name  :	isAnyEntrySeletected                                                     
*  Description    :	Check all checks box to check whether ant one entry is selected or not
*  Author Name    :	Neha Gharge 
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
bool CISpyFolderLocker::isAnyEntrySeletected()
{
	int icount = 0;

	for(int i=0 ; i < m_lstFolderLock.GetItemCount() ; i++)
	{
		BOOL bCheck = m_lstFolderLock.GetCheck(i);
		if(bCheck == 1)
		{
			icount++;
		}

	}
	if(icount==0)
	{
		return false;
	}
	else
	{
		return true;
	}
return true;
}


/**********************************************************************************************************                     
*  Function Name  :	ResetControlOfFolder                                                     
*  Description    :	Reset controls of folder locker.
*  Author Name    :	Neha Gharge 
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CISpyFolderLocker::ResetControlOfFolder()
{
	m_chkSelectAll.SetCheck(1);
	m_btnStartFolderAction.EnableWindow(true);
	m_btnFolderOpen.ShowWindow(SW_HIDE);
	m_btnFolderRemove.EnableWindow(true);
	m_btnFolderRemove.ShowWindow(SW_SHOW);
	m_edtFolderLockPath.SetWindowTextW(L"");
	m_btnFolderLockBrowse.EnableWindow(true);
	//m_btnBack.EnableWindow(true);
	
}

/***********************************************************************************************
* Function Name  : RefreshString
* Description    : this function is  called for setting the Text UI with different Language Support
* Author Name    : Amit Dutta
* SR_NO			 : 
* Date           : 30 April 2014
***********************************************************************************************/
void CISpyFolderLocker::RefreshStrings()
{
	/*m_btnFolderRemove.SetWindowTextW(L"Unlock");
    m_stFolderLlckrSubHeader.SetWindowTextW(L"Protects folder/file from unauthorized access.");
 	m_btnFolderLockBrowse.SetWindowTextW(L"Browse");
	m_btnStartFolderAction.SetWindowTextW(L"Lock");
	m_btnFolderOpen.SetWindowTextW(L"Open");
	m_edtFolderLockPath.SetWindowTextW(L"");
	m_btnFolderRemove.SetWindowTextW(L"Unlock");
	m_stFolderLlckrHeader.SetWindowTextW(L"Folder Lock");*/
	
	m_btnFolderRemove.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FOLDERLCK_UNLOCK"));
    m_stFolderLlckrSubHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_SUBHEADER"));
  	m_btnFolderLockBrowse.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_BROWSE"));
	m_btnFolderLockBrowse.SetFont(&theApp.m_fontWWTextNormal);
	m_btnStartFolderAction.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FOLDERLCK_LOCK"));
	m_btnStartFolderAction.SetFont(&theApp.m_fontWWTextNormal);
	m_btnFolderOpen.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FOLDERLCK_OPEN"));
	m_btnFolderLockBrowse.SetFont(&theApp.m_fontWWTextNormal);
    //m_edtFolderLockPath.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L""));
    m_btnFolderRemove.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_FOLDERLCK_UNLOCK"));
	m_btnFolderRemove.SetFont(&theApp.m_fontWWTextNormal);
	m_stFolderLlckrHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_HEADER"));
	m_stInfoText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_BRWSE_TEXT"));
	m_stSelectAll.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SELECTALL"));
	m_stSelectAll.SetFont(&theApp.m_fontWWTextNormal);
}

/*****************ISSUE NO -220,224 Neha Gharge 22/5/14 ************************************************/
/**********************************************************************************************************                     
*  Function Name  :	OnBnClickFolderLock                                                     
*  Description    :	It calls when user click on tab controls
*  Author Name    :	Neha Gharge 
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
bool CISpyFolderLocker::OnBnClickFolderLock()
{
	ResetControlOfFolder();
	LoadExistingFolderLockerEntriesFile();
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	CheckIsParentFolderLocked                                                     
*  Description    :	Checks whether given path's parent is already locked or not
*  Author Name    :	Neha Gharge 
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
bool CISpyFolderLocker::CheckIsParentFolderLocked(wchar_t *szFullPath)
{
		//issue no 222 neha gharge ->network drive also get included
		int n = 0,iBlackslashpos = 0 ;

		if(wcslen(szFullPath) == 0)
		{
			return false;
		}
		CString csNetworkpath,csnetPathWoutBlackslash;
		wchar_t szPath[MAX_PATH] = {0};
		wchar_t folder[MAX_PATH] = {0};
		wchar_t foldercompare[MAX_PATH] = {0};
		wchar_t *end = NULL;
		wchar_t szNetworkPath[MAX_PATH] = {0};

		if(PathIsNetworkPath(szFullPath))
		{
			csNetworkpath = szFullPath;
			iBlackslashpos = csNetworkpath.Find(L"\\");
			if(iBlackslashpos == 0x00)
			{
				csnetPathWoutBlackslash = csNetworkpath.Right(csNetworkpath.GetLength() - (iBlackslashpos +2));
			}
			_tcscpy(szFullPath,csnetPathWoutBlackslash);
			iBlackslashpos = 0;
			csNetworkpath = L"";
		}

		_tcscat(szFullPath,L"\\");
		ZeroMemory(folder, MAX_PATH * sizeof(wchar_t));

		_tcscpy(szPath, szFullPath);

		end = wcschr(szPath, L'\\');
		while(end != NULL)
		{

			wcsncpy(folder, szPath, end - szPath + 1);
			wcsncpy(foldercompare,folder,(wcslen(folder)-1));
			for(n=0;n<m_lstFolderLock.GetItemCount()+1;n++)
			{
				CString csInputPath = m_lstFolderLock.GetItemText(n,0);
				if(PathIsNetworkPath(csInputPath))
				{
					csNetworkpath = csInputPath;
					iBlackslashpos = csNetworkpath.Find(L"\\");
					if(iBlackslashpos == 0x00)
					{
						csnetPathWoutBlackslash = csNetworkpath.Right(csNetworkpath.GetLength() - (iBlackslashpos +2));
					}
					csInputPath = csnetPathWoutBlackslash;

				}
				if(foldercompare == csInputPath)
				{
					m_bIsPopUpDisplayed = true;
					// ISSue No. 510 RY 30th June 2014.
					MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_FOLDERLCK_ALREADY_LOCKED"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONINFORMATION|MB_OK);
					m_bIsPopUpDisplayed = false;

					m_edtFolderLockPath.SetWindowTextW(L"");
					return false;
				}
			}
			end = wcschr(++end, L'\\');
		}
		return true;
}

/**********************************************************************************************************                     
*  Function Name  :	OnCtlColor                                                     
*  Description    :	The framework calls this member function when a child control is about to be drawn.
*  Author Name    :	Neha Gharge 
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
HBRUSH CISpyFolderLocker::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	int ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if( ctrlID == IDC_STATIC_FOLDERLCKR_HEADER      ||
		ctrlID == IDC_STATIC_FOLDERLCKR_SUBHEADER
		)

	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	}
	return hbr;
}

/**********************************************************************************************************                     
*  Function Name  :	OnPaint                                                     
*  Description    :	The framework calls this member function when Windows or an application makes a request 
					to repaint a portion of an application's window.
*  Author Name    :	Neha Gharge 
*  SR_NO		  :
*  Date           : 11 Jan 2014
**********************************************************************************************************/
void CISpyFolderLocker::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CJpegDialog::Draw();
	CJpegDialog::OnPaint();
}
