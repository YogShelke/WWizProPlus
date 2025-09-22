// ISpyReportsDlg.cpp : implementation file
/*********************************************************************
*  Program Name: ISpyReportsDlg.cpp                                                                                                    
*  Description: While cleaning any infected file,WardWiz AV taking a 
				backup in backup folder but in encrypted format.So if
				user wants to retrive the files then user can press recover
				button to retrive file to its original location.
*  Author Name: Neha Gharge
*  Date Of Creation: 20 Nov 2013
*  Version No: 1.0.0.2

/*********************************************************************
HEADER FILES
**********************************************************************/

#include "stdafx.h"
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"
#include "ISpyRecoverDlg.h"

// CISpyRecoverDlg dialog
#define	szpIpsy		"ISPY"
DWORD WINAPI RecoverThread( LPVOID lpParam ) ;
DWORD WINAPI DeleteRThread( LPVOID lpParam ) ;
DWORD WINAPI ShowRecoverEntriesThread(LPVOID lpvThreadParam);

//CISpyGUIDlg *g_TabCtrlRecoverWindHandle = NULL;

IMPLEMENT_DYNAMIC(CISpyRecoverDlg, CDialog)

/**********************************************************************************************************                     
*  Function Name  :	CISpyRecoverDlg                                                     
*  Description    :	C'tor
*  Author Name    : Neha Gharge   
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
CISpyRecoverDlg::CISpyRecoverDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CISpyRecoverDlg::IDD, pParent)
	,m_hThread_Recover(NULL)
	,m_hThread_Delete(NULL)
	,m_bRecoverThreadStart(false)
	,m_bDeleteThreadStart(false)
	,m_bRecoverStop(false)
	,m_bDeleteStop(false)
	,m_bRecoverClose(false)
	,m_hShowRecoverEntriesThread(NULL)
	,m_bRecoverBrowsepath(false)
    ,m_bRecoverEnableDisable(false)
	, m_bIsPopUpDisplayed(false)
	, m_objCom(SERVICE_SERVER, true)
{

}

/**********************************************************************************************************                     
*  Function Name  :	~CISpyRecoverDlg                                                     
*  Description    : Destructor
*  Author Name    : Neha Gharge   
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
CISpyRecoverDlg::~CISpyRecoverDlg()
{
	if(m_hShowRecoverEntriesThread != NULL)
	{
		::SuspendThread(m_hShowRecoverEntriesThread);
		::TerminateThread(m_hShowRecoverEntriesThread, 0);
		m_hShowRecoverEntriesThread = NULL;
	}
}


/**********************************************************************************************************                     
*  Function Name  :	DoDataExchange                                                     
*  Description    :	Called by the framework to exchange and validate dialog data.
*  Author Name    :	Neha Gharge
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyRecoverDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_RECOVER_HEADERPIC, m_stRecoverHeaderpic);
	DDX_Control(pDX, IDC_LIST_RECOVER, m_lstRecover);
	DDX_Control(pDX, IDC_BUTTON_RECOVER, m_btnRecover);
	DDX_Control(pDX, IDC_BUTTON_BACK, m_btnBack);
	DDX_Control(pDX, IDC_CHECK_SELECTALL, m_chkSelectAll);
	DDX_Control(pDX, IDC_STATIC_SELECTALL, m_stSelectAll);
	DDX_Control(pDX, IDC_STATIC_RECOVER_HEADER_NAME, m_stRecoverHeaderName);
	DDX_Control(pDX, IDC_STATIC_RECOVER_DESC, m_stRecoverDesc);
	DDX_Control(pDX, IDC_BUTTON_DELETE, m_btnDelete);
}

/***************************************************************************
* Function Name  : MESSAGE_MAP
* Description    : Handle WM_COMMAND,WM_Messages,user defined message 
				   and notification message from child windows.
*  Author Name    : Prajakta   
*  SR_NO		  :
*  Date           : 20 Nov 2014
****************************************************************************/
BEGIN_MESSAGE_MAP(CISpyRecoverDlg, CJpegDialog)
	ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BUTTON_BACK, &CISpyRecoverDlg::OnBnClickedButtonBack)
	ON_BN_CLICKED(IDC_BUTTON_RECOVER, &CISpyRecoverDlg::OnBnClickedButtonRecover)
	ON_BN_CLICKED(IDC_CHECK_SELECTALL, &CISpyRecoverDlg::OnBnClickedCheckSelectall)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &CISpyRecoverDlg::OnBnClickedButtonDelete)
//	ON_WM_NCPAINT()
ON_WM_PAINT()
END_MESSAGE_MAP()


// CISpyRecoverDlg message handlers
/**********************************************************************************************************                     
*  Function Name  :	OnInitDialog                                                     
*  Description    :	Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft 
					Foundation Class Library dialog boxes
*  SR.NO		  : 
*  Author Name    : Neha Gharge
*  Date           : 20 Nov 2013
**********************************************************************************************************/
BOOL CISpyRecoverDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);

	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_INNER_DIALOG), _T("JPG")))
	{
		m_bIsPopUpDisplayed = true;
		::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
		m_bIsPopUpDisplayed = false;
	}

	Draw();

	m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));

	CRect rect;
	this->GetClientRect(rect);
	
	m_stRecoverHeaderName.SetWindowPos(&wndTop,rect.left +20,07,400,31,SWP_NOREDRAW);	
	m_stRecoverHeaderName.SetTextColor(RGB(24,24,24));

	m_stRecoverDesc.SetWindowPos(&wndTop,rect.left +20,35,400,15,SWP_NOREDRAW);
	m_stRecoverDesc.SetTextColor(RGB(24,24,24));

	if(theApp.m_dwOSType == WINOS_WIN8 ||theApp.m_dwOSType == WINOS_WIN8_1)
	{
		m_stRecoverHeaderName.SetWindowPos(&wndTop,rect.left +20,14,400,31,SWP_NOREDRAW);	
		m_stRecoverDesc.SetWindowPos(&wndTop,rect.left +20,38,400,15,SWP_NOREDRAW);
	}

	m_bmpRecoverHeaderpic = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_COM_HEADER));
	m_stRecoverHeaderpic.SetBitmap(m_bmpRecoverHeaderpic);
	m_stRecoverHeaderpic.SetWindowPos(&wndTop,rect.left + 6,10,586,45,SWP_NOREDRAW);

	ListView_SetExtendedListViewStyle (m_lstRecover.m_hWnd, LVS_EX_FULLROWSELECT |LVS_EX_CHECKBOXES| LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);
	//m_lstRecover.SetWindowPos(&wndTop,rect.left + 43,82,675,230,SWP_NOREDRAW);

	//Varada Ikhar, Date:26/02/2015, Issue:The UI look for space between header and Listview should be standard.
	m_lstRecover.SetWindowPos(&wndTop, rect.left + 6, 68, 586, 257, SWP_NOREDRAW);

	// *****************************ISSUE NO : 391 Neha gharge 24/5/2014 *******************************************/
	m_chkSelectAll.SetWindowPos(&wndTop,rect.left + 8,335,13,13,SWP_NOREDRAW);
	m_chkSelectAll.SetCheck(1);
	m_stSelectAll.SetWindowPos(&wndTop,rect.left + 27,334,150,15,SWP_NOREDRAW);
	m_stSelectAll.SetTextColor(RGB(255,255,255));

	m_btnRecover.SetSkin(theApp.m_hResDLL, IDB_BITMAP_RECOVER_BUTTON, IDB_BITMAP_RECOVER_BUTTON, IDB_BITMAP_RECOVER_HOVER, IDB_BITMAP_RECOVER_DISABLED, 0, 0, 0, 0, 0);
	m_btnRecover.SetWindowPos(&wndTop,rect.left + 477,353,115,22,SWP_NOREDRAW);
	
	m_btnRecover.SetTextColorA(BLACK,1,1);

	m_btnDelete.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnDelete.SetWindowPos(&wndTop,rect.left + 410,353,57,21,SWP_NOREDRAW);
	m_btnDelete.SetTextColorA(BLACK,1,1);
	//m_btnBack.SetSkin(IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROWDISABLE,0,0,0,0);
	//m_btnBack.SetWindowPos(&wndTop, rect.left+ 21, 354,31,32, SWP_NOREDRAW | SWP_NOZORDER);
	m_btnBack.ShowWindow(SW_HIDE);

	//Setting fonts here
	m_stRecoverHeaderName.SetFont(&theApp.m_fontWWTextSmallTitle);
	m_stRecoverDesc.SetFont(&theApp.m_fontWWTextSubTitleDescription);
	
	//m_lstRecover.SetFont(&theApp.m_fontWWTextNormal);
	CHeaderCtrl* pHeaderCtrl = m_lstRecover.GetHeaderCtrl();
	pHeaderCtrl->SetFont(&theApp.m_fontWWTextNormal);

	m_btnRecover.SetFont(&theApp.m_fontWWTextNormal);
	m_btnDelete.SetFont(&theApp.m_fontWWTextNormal);
	m_stSelectAll.SetFont(&theApp.m_fontWWTextNormal);
	
	RefreshStrings();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/**********************************************************************************************************                     
*  Function Name  :	OnSetCursor                                                     
*  Description    :	The framework calls this member function if mouse input is not captured and the mouse causes cursor movement within 
					the CWnd object.
*  Author Name    :	Neha Gharge
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
BOOL CISpyRecoverDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if( 
		iCtrlID == IDC_BUTTON_RECOVER	||
		iCtrlID == IDC_BUTTON_DELETE	||
		iCtrlID == IDC_BUTTON_BACK 
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
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	OnNcHitTest                                                     
*  Description    :	The framework calls this member function for the CWnd object that contains the cursor every time the mouse is moved.
*  Author Name    : Neha Gharge  
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
LRESULT CISpyRecoverDlg::OnNcHitTest(CPoint point)
{
	return HTCLIENT;

}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonBack                                                     
*  Description    :	Allows to go back to parent dialog i.e home window.
*  Author Name    : Neha Gharge  
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyRecoverDlg::OnBnClickedButtonBack()
{
	// TODO: Add your control notification handler code here
	//this->ShowHideChildEmailScan();
	this->ShowWindow(SW_HIDE);
	CISpyToolsDlg *pObjToolWindow = reinterpret_cast<CISpyToolsDlg*>(this->GetParent());
	pObjToolWindow->ShowHideToolsDlgControls(true);
}


/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedButtonRecover                                                     
*  Description    :	Recovers selected entry from reports
*  SR.NO          : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyRecoverDlg::OnBnClickedButtonRecover()
{
	
	
	int iRecoverCount = 0;
	int MaxCount;
	int i=0;
	CString csVirusName,csDuplicatePath;

	//Varada Ikhar, Date:17-03-2015, 
	//Issue:User updated the product and does not restarted, and started a scan, then it should show message as 
	//"Product updated, restart is required to take a effect."
	if (!theApp.ShowRestartMsgOnProductUpdate())
	{
		return;
	}

	MaxCount = m_lstRecover.GetItemCount();
	AddLogEntry(L">>> Recover Started", 0, 0, true, FIRSTLEVEL);
	//g_TabCtrlRecoverWindHandle = (CISpyGUIDlg*)AfxGetMainWnd();	
	//g_TabCtrlRecoverWindHandle->m_pTabDialog->DisableAllExceptSelected();

	if(MaxCount==0)
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_NO_ENTRY_FOR_RECOVER"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		m_bIsPopUpDisplayed = false;
		goto Cleanup;
	}
	if(!isAnyEntrySeletected())
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_NO_SELECT_ENTRY"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		m_bIsPopUpDisplayed = false;
		goto Cleanup;
	}

	//Check here for Evaluation period expired
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
			return;
		}
	}

	
	m_hThread_Recover = ::CreateThread(NULL, 0, RecoverThread, (LPVOID) this, 0, NULL);
	Sleep( 1000 ) ;
//	m_btnRecover.EnableWindow(false);


Cleanup:
//	m_btnRecover.EnableWindow(true);
	return;
}

/***************************************************************************************************                    
*  Function Name  :           OnBnClickedButtonDelete                                                     
*  Description    :           this function is called for deletion of records  
*  Author Name    :           Nitin K. Kolapkar         
*  SR_NO		  :
*  Date           :           13th June 2014
****************************************************************************************************/
void CISpyRecoverDlg::OnBnClickedButtonDelete()
{
	int iDeleteCount = 0;
	int MaxCount;
	int i = 0;
	CString csVirusName,csDuplicatePath;

	//Varada Ikhar, Date:17-03-2015, 
	//Issue:User updated the product and does not restarted, and started a scan, then it should show message as 
	//"Product updated, restart is required to take a effect."
	if (!theApp.ShowRestartMsgOnProductUpdate())
	{
		return;
	}

	MaxCount = m_lstRecover.GetItemCount();
	AddLogEntry(L">>> Delete Started", 0, 0, true, FIRSTLEVEL);

	if(MaxCount==0)
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_NO_ENTRY_TO_DELETE"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		m_bIsPopUpDisplayed = false;
		goto Cleanup;
	}
	if(!isAnyEntrySeletected())
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_NO_SELECT_ENTRY"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		m_bIsPopUpDisplayed = false;
		goto Cleanup;
	}

	//Check here for Evaluation period expired
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
			return;
		}
	}

	m_bIsPopUpDisplayed = true;
	if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_ENTRY_DELETE_CONFIRMATION"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDNO)
	{
		m_bIsPopUpDisplayed = false;
		return;
	}
	m_bIsPopUpDisplayed = false;
	
	m_hThread_Delete = ::CreateThread(NULL, 0, DeleteRThread, (LPVOID) this, 0, NULL);
	Sleep( 1000 ) ;
//	m_btnRecover.EnableWindow(false);


Cleanup:
//	m_btnRecover.EnableWindow(true);
	return;
}


/***************************************************************************************************                    
*  Function Name  : isAnyEntrySeletected                                                     
*  Description    : Checks whether any check box to recover or delete entry is selected or not 
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 13th June 2014
****************************************************************************************************/
bool CISpyRecoverDlg::isAnyEntrySeletected()
{
	int icount = 0;

	for(int i=0 ; i < m_lstRecover.GetItemCount() ; i++)
	{
		BOOL bCheck = m_lstRecover.GetCheck(i);
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
*  Function Name  :	PopulateList                                                     
*  Description    :	Displays entries on recover dialog
*  SR.NO		  : 
*  Author Name    : Neha Gharge                                                                                         
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyRecoverDlg::PopulateList(bool bCheckEntry)
{
	int nCurrentItem;
	//	bool bCheck;		
	// delete all current members
	
	// get a reference to the contacts list
	const ContactList& contacts = m_objRecoverdb.GetContacts();

	// iterate over all contacts add add them to the list
		
	POSITION pos = contacts.GetHeadPosition();
	nCurrentItem = m_lstRecover.GetItemCount();
	while(pos != NULL)
	{

		const CIspyList contact = contacts.GetNext(pos);

		if (!m_objRecoverdbToSave.AddContact(contact))
		{
			continue;//Continue to next entry.
		}

		//Check here if Repair completed sucessfully or not
		//	0 - Repair/Delete success
		//	1 - Delete failed.
		//	2 - Repair failed.
		if (contact.GetSeventhEntry() > 0)
		{
			continue;//Continue to next entry.
		}

		LVITEM lvItem = {0};

		// Insert the first item
		lvItem.mask =  LVIF_TEXT;
		lvItem.iItem = nCurrentItem;
		lvItem.iSubItem = 0;
		lvItem.pszText =(LPTSTR)(LPCTSTR)contact.GetThirdEntry();		
		m_lstRecover.InsertItem(&lvItem);

		//nCurrentItem = m_lstRecover.InsertItem(nCurrentItem, contact.GetFirstEntry());
		m_lstRecover.SetItemText(nCurrentItem, 1, contact.GetFirstEntry());
		m_lstRecover.SetItemText(nCurrentItem, 0, contact.GetThirdEntry());

		if(bCheckEntry)
		{
			m_lstRecover.SetCheck(nCurrentItem, TRUE);
		}
		nCurrentItem++; //this line is for append entry;
	}
	
}


/**********************************************************************************************************                     
*  Function Name  :	LoadExistingRecoverFile                                                     
*  Description    :	Opens WRDWIZRECOVERENTRIES.DB file & retrieves its contents
*  SR.NO		  : 
*  Author Name    : Neha Gharge                                                                                          
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyRecoverDlg::LoadExistingRecoverFile(bool bType)
{
	static CString csFilePath = L"";
	//TCHAR szModulePath[MAX_PATH] = {0};
	//if(!GetModulePath(szModulePath, MAX_PATH))
	//{
	//	MessageBox(L"Error in GetModulePath.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR|MB_OK);
	//	return;
	//}
	csFilePath = GetQuarantineFolderPath();
	csFilePath += _T("\\WRDWIZRECOVERENTRIES.DB");

	//ISSUE NO:- 207 RY Date :- 21/5/2014
	LoadDataContentFromFile(csFilePath);
	//PopulateList(bType);
	//m_chkSelectAll.SetCheck(TRUE);
}


//void CISpyRecoverDlg::StoreExistingRecoverFile()
//{
//	static CString csFilePath;
//	TCHAR szModulePath[MAX_PATH] = {0};
//	if(!GetModulePath(szModulePath, MAX_PATH))
//	{
//		MessageBox(L"Error in GetModulePath.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR|MB_OK);
//		return;
//	}
//	csFilePath = szModulePath;
//	csFilePath += _T("\\WRDWIZRECOVERENTRIES.DB");
//
//	StoredataContentToFile(csFilePath);
//	//PopulateList();
//}

/**********************************************************************************************************                     
*  Function Name  :	LoadDataContentFromFile                                                     
*  Description    :	Loads WRDWIZRECOVERENTRIES.DB file content into serialization object.
*  SR.NO		  : 
*  Author Name    : Neha Gharge                                                                                          
*  Date           : 20 Nov 2013
**********************************************************************************************************/
bool CISpyRecoverDlg::LoadDataContentFromFile(CString csPathName)
{
	AddLogEntry(L">>> Loading recover db", 0, 0, true, FIRSTLEVEL);
	m_objRecoverdb.RemoveAll();

	if (!PathFileExists(csPathName))
		return false;

	CFile rFile(csPathName, CFile::modeRead);

	// Create a loading archive
	CArchive arLoad(&rFile, CArchive::load);
	m_objRecoverdb.Serialize(arLoad);

	// Close the loading archive
	arLoad.Close();
	rFile.Close();

	return true;
	//PopulateList();
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedCheckSelectall                                                     
*  Description    :	On selection of  'Select All'  button, all checkboxes with corresponding entries on UI will be marked 
*  SR.NO		  : 
*  Author Name    : Neha Gharge                                                                                          
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyRecoverDlg::OnBnClickedCheckSelectall()
{
	// TODO: Add your control notification handler code here
	int iCheck = m_chkSelectAll.GetCheck();

	for(int i = 0; i < m_lstRecover.GetItemCount(); i++)
	{
		m_lstRecover.SetCheck(i, iCheck);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	PreTranslateMessage                                                     
*  Description    : To translate window messages before they are dispatched 
				    to the TranslateMessage and DispatchMessage Windows functions
*  SR.N0		  : 
*  Author Name    :	Neha Gharge                                                                                   
*  Date           : 20 Nov 2013
**********************************************************************************************************/
BOOL CISpyRecoverDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

/**********************************************************************************************************                     
*  Function Name  :	SendRecoverOperations2Service                                                     
*  Description    :	Sends data to service to recover and delete entries which are selected and save remaining 
				    entries which are not selected. 
*  SR.NO		  : 
*  Author Name    : Neha Gharge                                                                                       
*  Date           : 20 Nov 2013
**********************************************************************************************************/
bool CISpyRecoverDlg::SendRecoverOperations2Service(int dwMessageinfo, CString csRecoverFileEntry, CString csBrowseRecoverFileEntry ,DWORD dwType,DWORD dwTypeofAction, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	szPipeData.iMessageInfo = dwMessageinfo;
	_tcscpy_s(szPipeData.szFirstParam , csRecoverFileEntry);
	_tcscpy_s(szPipeData.szSecondParam, csBrowseRecoverFileEntry);
	szPipeData.dwValue = dwType;
	szPipeData.dwSecondValue = dwTypeofAction;
	//CISpyCommunicator objCom(SERVICE_SERVER, false);

	if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CGenXRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bWait)
	{
		if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to Read data in CGenXRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
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
*  Function Name  : RefreshString
*  Description    : this function is  called for setting the Text UI with different Language Support
*  Author Name    : Prassana
*  SR_NO		  :
*  Date           : 30 April 2014
***********************************************************************************************/
void CISpyRecoverDlg::RefreshStrings()
{
	m_stRecoverHeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_RECOVER_HEADER"));
	m_stRecoverDesc.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_RECOVER_DESC"));
	
/*
	m_lstRecover.InsertColumn( 0, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_RECOVER_LST_THREATNAME"), LVCFMT_LEFT, 180);
	m_lstRecover.InsertColumn(1, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_RECOVER_LST_ORGPATH"), LVCFMT_LEFT,479);
*/

	//Added by Vilas on 27 Mar 2015
	//Shows failed status while recovering file
	m_lstRecover.InsertColumn(0, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_RECOVER_LST_THREATNAME"), LVCFMT_LEFT, 120);
	m_lstRecover.InsertColumn(1, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_RECOVER_LST_ORGPATH"), LVCFMT_LEFT, 400);
	m_lstRecover.InsertColumn(2, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_RECOVER_LST_RECOVER_STATUS"), LVCFMT_LEFT, 139);

	m_stSelectAll.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SELECTALL"));
	
	m_btnRecover.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RECOVER"));
	m_btnDelete.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_DELETE"));
}

/**********************************************************************************************************                     
*  Function Name  :	OnCtlColor                                                     
*  Description    :	The framework calls this member function when a child control is about to be drawn.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
HBRUSH CISpyRecoverDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
 HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
 int ctrlID;
 ctrlID = pWnd->GetDlgCtrlID();
 if( ctrlID == IDC_STATIC_RECOVER_HEADER_NAME ||
	 ctrlID == IDC_STATIC_RECOVER_DESC ||
	 ctrlID == IDC_LIST_RECOVER ||
	 ctrlID == IDC_CHECK_SELECTALL ||
	 ctrlID == IDC_STATIC_SELECTALL ||
	 ctrlID == IDC_BUTTON_RECOVER
     )  
 {
  pDC->SetBkMode(TRANSPARENT);
  hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
 } 
 return hbr;
}

/***************************************************************************************************                    
*  Function Name  :           RecoverThread                                                     
*  Description    :           this function is called for recovering of records  
*  Author Name    :			  Neha Gharge.   
*  SR_NO		  :
*  Date           :           11th June 2014
****************************************************************************************************/
DWORD WINAPI RecoverThread( LPVOID lpParam )
{
	if(!lpParam)
	{
		return 0;
	}

	CISpyRecoverDlg *pThis = (CISpyRecoverDlg *) lpParam;
	//Issue No.17 Gaurav Chopdar Date:6/1/2015
	pThis->m_bRecoverEnableDisable = true;
	pThis->m_btnRecover.EnableWindow(FALSE);
	pThis->m_btnDelete.EnableWindow(FALSE);
	pThis->m_bRecoverStop = false;
	/* Issue: While recover is in progress and we unclick the checkbox entries doesn't get recovered 
		Resolved by : Nitin K Date: 29-Dec-2014
	*/
	pThis->m_lstRecover.EnableWindow(FALSE);
	// Issue: While recover is in progress and if we uncheck the checkbox, Recover process shows pop-up "Recover files Successfully".
	//Name: Niranjan Deshak - 3/2/2015.
	pThis->m_chkSelectAll.EnableWindow(false);
	pThis->RecoverEntries();
	//Issue No.17 Gaurav Chopdar Date:6/1/2015
	pThis->m_bRecoverEnableDisable = false;
	//pThis->m_btnClean.EnableWindow(TRUE);
	return 1;
}

/***************************************************************************************************                    
*  Function Name  :           DeleteRThread                                                     
*  Description    :           this function is called for deletion of records  
*  Author Name    :           Nitin K. Kolapkar   
*  SR_NO		  :
*  Date           :           13th June 2014
****************************************************************************************************/
DWORD WINAPI DeleteRThread( LPVOID lpParam )
{
	CISpyRecoverDlg *pThis = (CISpyRecoverDlg *) lpParam;
   //Issue No.17 Gaurav Chopdar Date:6/1/2015
    pThis->m_bRecoverEnableDisable = true;
	pThis->m_btnRecover.EnableWindow(FALSE);
	pThis->m_btnDelete.EnableWindow(FALSE);
	pThis->m_bDeleteStop= false;
	/* Issue: While delete is in progress and we unclick the checkbox entries doesn't get delete 
		Resolved by : Nitin K Date: 29-Dec-2014
	*/
	pThis->m_lstRecover.EnableWindow(FALSE);
	// Issue: While delete is in progress and if we uncheck the checkbox,Deletion process shows pop-up "Delete files Successfully".
	//Name: Niranjan Deshak - 3/2/2015.
	pThis->m_chkSelectAll.EnableWindow(false);

	pThis->DeleteREntries();
	//Issue No.17 Gaurav Chopdar Date:6/1/2015
	pThis->m_bRecoverEnableDisable = false;
	return 1;
}

/***************************************************************************************************                    
*  Function Name  :           RecoverEntries                                                     
*  Description    :           this function is called for recovering of files  
*  Author Name    :           Neha Gharge 
*  SR_NO		  :
*  Date           :           20 Nov 2013
****************************************************************************************************/
void CISpyRecoverDlg::RecoverEntries()
{

	//BOOL bCheck;
	bool bSuccess = true;
	CString csFilePath = L"";
	bool	bSelected = false ;
	CString csThreatPath = L"",csBrowsePath = L"";
	int i = 0, iPos = 0 ; 
	m_bRecoverThreadStart = true;
	
	AddLogEntry(L">>> recovering entry started", 0, 0, true, FIRSTLEVEL);
	m_iTotalEntriesCount = m_lstRecover.GetItemCount();
	if(!SendRecoverOperations2Service(RELOAD_DBENTRIES , L"",L"", RECOVER,0, true))
	{
		if (!SendRecoverOperations2Service(RELOAD_DBENTRIES, L"", L"", RECOVER, 0, true))
		{

			AddLogEntry(L"### Error in SendRecoverOperations2Service CRecoverDlg::OnBnClickedButtonRecover", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}
	}

	DWORD dwRecoverCount = 0;
	do
	{
		for(i=0; i< m_lstRecover.GetItemCount(); i++)
		{
			if(m_bRecoverStop)
			{
				break;
			}
			//bCheck = m_lstRecover.GetCheck(i);
			if(m_lstRecover.GetCheck(i))
			{
				csThreatPath = m_lstRecover.GetItemText(i,1);
				csFilePath = csThreatPath;
				iPos = csFilePath.Find(_T("\\"));
				csFilePath.Truncate(iPos);
				if(!PathFileExists(csFilePath) && m_bRecoverBrowsepath == false)
				{
					CString csMsgPathNotExist;
					//Ticket no 62 Neha Gharge Changing message.
					//csMsgPathNotExist.Format(L"%s %s\n\n%s",csThreatPath,L"path does not exist.",L"Please select the path where you want to recover the file");
					csMsgPathNotExist.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_RECOVER_BROWSE_MSG"));
					
					m_bIsPopUpDisplayed = true;
					MessageBox(csMsgPathNotExist,theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONINFORMATION|MB_OK);
					m_bIsPopUpDisplayed = false;

					BrowseFileToSaveRecover(csFilePath);
					csBrowsePath = csFilePath;
					m_bRecoverBrowsepath = true;
				}
				/*	ISSUE NO - All Files not getting Recover. Check box are getting unchecked NAME - NITIN K. TIME - 3rd July 2014 */
				//Sleep(15);
				//	ISSUE - In Windows Vista & Windows 8 while recover is in progress some entries gets unselected.
				//Niranjan Deshak - TIME - 2nd feb 2015 
				Sleep(100);

				//Added by Vilas on 27 Mar 2015
				//Shows failed status while recovering file

				ISPY_PIPE_DATA	szPipeData = { 0 };

				szPipeData.iMessageInfo = RECOVER_FILE;
				_tcscpy_s(szPipeData.szFirstParam, csThreatPath);
				_tcscpy_s(szPipeData.szSecondParam, csBrowsePath);
				szPipeData.dwValue = 0;
				szPipeData.dwSecondValue = 0;

				SendRecoverOperations2Service(&szPipeData, true);
				if (!szPipeData.dwValue)
				{
					m_lstRecover.DeleteItem(i) ;
					dwRecoverCount++;
				}
				else
				{
					switch (szPipeData.dwValue)
					{
					case 0x02:
						m_lstRecover.SetItemText(i, 2, theApp.m_objwardwizLangManager.GetString(L"IDS_REPORTS_FAILED_FILE_IN_USE"));
						break;
						
					default:
						m_lstRecover.SetItemText(i, 2, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"));
					}

					m_lstRecover.SetCheck(i,0);
					bSuccess = false;
					AddLogEntry(L"### Error in CRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
				}

				bSelected = true ;
				break;
			}
		}
		if( i >= m_lstRecover.GetItemCount() )
		break ;

		if(m_bRecoverStop)
		{
			break;
		}
	}while( bSelected ) ;

	Sleep(5);
	if(!SendRecoverOperations2Service(SAVE_RECOVERDB , L"",L"",0,0, true))
	{
		AddLogEntry(L"### Error in CRecoverDlg::SendRecoverOperations2Service SAVE_RECOVERDB", 0, 0, true, SECONDLEVEL);
	}

	if(!m_bRecoverStop)
	{
		TCHAR szRecoverMsg[MAX_PATH] = {0};
		
		m_bIsPopUpDisplayed = true;
		if(dwRecoverCount == 0)
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_NO_FILE_RECOVERED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		}
		else
		{
			swprintf(szRecoverMsg, L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_FILES_RECOVERED_SUCCESSFULLY"));
			MessageBox(szRecoverMsg, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION | MB_OK);
		//g_TabCtrlRecoverWindHandle->m_pTabDialog->EnableAllBtn();
		}
		m_bIsPopUpDisplayed = false;
	}
	AddLogEntry(L">>> recovering entry finished", 0, 0, true, FIRSTLEVEL);

Cleanup:
	dwRecoverCount = 0;
	m_bRecoverThreadStart = false;
	m_bRecoverBrowsepath = false;
	m_btnRecover.EnableWindow(true);
	m_btnDelete.EnableWindow(true);
	/* Issue: While delete is in progress and we unclick the checkbox entries doesn't get delete 
		Resolved by : Nitin K Date: 29-Dec-2014
	*/
	m_lstRecover.EnableWindow(true);
	// Issue: While recover is in progress and if we uncheck the checkbox, Recover process shows pop-up "Recover files Successfully".
	//Name: Niranjan Deshak - 3/2/2015.
	m_chkSelectAll.EnableWindow(true);

}


/**********************************************************************************************************                     
*  Function Name  :	StopRecoverOperation                                                     
*  Description    :	Terminates the thread to stop recover operation if user selects close button on UI 
*  SR.NO		  :
*  Author Name    : Neha Gharge                                                                                          
*  Date           : 10 Jun 2013
**********************************************************************************************************/
bool CISpyRecoverDlg::StopReportOperation()
{
	try
	{
		if (m_bRecoverThreadStart == false && m_bDeleteThreadStart == false)
		{
			return true;
		}

		int iRet = IDNO;
		if (m_bRecoverThreadStart == true)
		{
			//Varada Ikhar, Date: 24/04/2015, Implementing pause-resume if user clicks on 'close' button.
			if (m_hThread_Recover != NULL)
			{
				m_bRecoverStop = true;
				if (::SuspendThread(m_hThread_Recover) == -1)
				{
					AddLogEntry(L"### Failed to pause Recover operation.", 0, 0, true, SECONDLEVEL);
				}
				AddLogEntry(L">>> Recover operation Paused.", 0, 0, true, ZEROLEVEL);
			}

			CString csMsgoptinprogress = L"";
			csMsgoptinprogress.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_RECOVER_PROG_ONE"), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_RECOVER_PROG_TWO"));
			
			m_bIsPopUpDisplayed = true;
			iRet = MessageBox(csMsgoptinprogress,
				theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_YESNO | MB_ICONQUESTION);
			m_bIsPopUpDisplayed = false;
		}
		else if (m_bDeleteThreadStart == true)
		{
			//Varada Ikhar, Date: 24/04/2015, Implementing pause-resume if user clicks on 'close' button.
			if (m_hThread_Delete != NULL)
			{
				m_bDeleteStop = true;
				if (::SuspendThread(m_hThread_Delete) == -1)
				{
					AddLogEntry(L"### Failed to pause Delete operation.", 0, 0, true, SECONDLEVEL);
				}
				AddLogEntry(L">>> Delete operation Paused.", 0, 0, true, ZEROLEVEL);
			}

			CString csMsgoptinprogress = L"";
			csMsgoptinprogress.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_DELETE_PROG_ONE"), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_RECOVER_PROG_TWO"));

			m_bIsPopUpDisplayed = true;
			iRet = MessageBox(csMsgoptinprogress,
				theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_YESNO | MB_ICONQUESTION);
			m_bIsPopUpDisplayed = false;

		}
		if (iRet == IDNO)
		{
			//Varada Ikhar, Date: 24/04/2015, Implementing pause-resume if user clicks on 'close' button.
			if (m_bRecoverThreadStart == true)
			{
				if (m_hThread_Recover != NULL)
				{
					m_bRecoverStop = false;
					if (::ResumeThread(m_hThread_Recover) == -1)
					{
						AddLogEntry(L"### Failed to resume Recover operation.", 0, 0, true, SECONDLEVEL);
					}
					AddLogEntry(L">>> Recover operation Resumed.", 0, 0, true, ZEROLEVEL);
				}
			}
			else if (m_bDeleteThreadStart == true)
			{
				if (m_hThread_Delete != NULL)
				{
					m_bDeleteStop = false;
					if (::ResumeThread(m_hThread_Delete) == -1)
					{
						AddLogEntry(L"### Failed to resume Delete operation.", 0, 0, true, SECONDLEVEL);
					}
					AddLogEntry(L">>> Delete operation Resumed.", 0, 0, true, ZEROLEVEL);
				}
			}
			return false;
		}
		else if (iRet == IDYES && m_bRecoverClose)
		{
			//Neha Gharge 3/3/2015 
			//Issue : if user click on close button not take any action YES/NO, UI get hanged or not refresh
			if (m_bRecoverClose == true)
			{
				theApp.m_bOnCloseFromMainUI = true;
			}
			m_bRecoverStop = true;
			m_bDeleteStop = true;

			//Varada Ikhar, Date: 24/04/2015
			//Issue : 0000205 :When Recover is in progress if I select any other scan option file gets recovered but entries are still present in Recover and if we try to recover it it is giving status as failed.
			//1.Select folder which has threats. 2.Clean the threats. 3.Click on Recover .
			//4.Go to quick scan and start scan. 5.Message will come " File recovered successfully". 6.Go to Recover, entries are still present and if we try to recover it is giving status as failed.
			if (!SendRecoverOperations2Service(SAVE_RECOVERDB, L"", L"", 0, 0, true))
			{
				AddLogEntry(L"### Failed to send SAVE_RECOVERDB in CNextGenExRecoverDlg::StopReportOperation", 0, 0, true, SECONDLEVEL);
			}
			Sleep(5);

			//Varada Ikhar, Date: 4th May-2015
			//Issue: While recover/delete operation is in progress, if any other tab is clicked and clicked 'Yes' to stop confirmation box, Cleanning operation gets aborted.
			//However, next time when clicked on any other tab again "Recover/Delete is in progress. Do you want to stop?" message gets displayed.
			m_bRecoverThreadStart = false;
			m_bDeleteThreadStart = false;

		}
		//Varada Ikhar, Date: 02/05/2015
		//Issue: If recover is in progress, and user clicks on any other tab then, again click on Recover tab, then everything is in disabled mode.
		else if (iRet == IDYES)
		{
			m_bRecoverStop = true;
			m_bDeleteStop = true;

			//Varada Ikhar, Date: 24/04/2015
			//Issue : 0000205 :When Recover is in progress if I select any other scan option file gets recovered but entries are still present in Recover and if we try to recover it it is giving status as failed.
			//1.Select folder which has threats. 2.Clean the threats. 3.Click on Recover .
			//4.Go to quick scan and start scan. 5.Message will come " File recovered successfully". 6.Go to Recover, entries are still present and if we try to recover it is giving status as failed.
			if (!SendRecoverOperations2Service(SAVE_RECOVERDB, L"", L"", 0, 0, true))
			{
				AddLogEntry(L"### Failed to send SAVE_RECOVERDB in CNextGenExRecoverDlg::StopReportOperation", 0, 0, true, SECONDLEVEL);
			}
			Sleep(5);
			
			//Varada Ikhar, Date : 2nd May-2015
			//Issue: While Reocover/Delete operation is in progress, if clicked on any other tab, and clicked on 'Yes' to stop confirmation box,
			//Respective operation gets aborted. However, the all controls of Recover dailog remains disable.
			m_btnDelete.EnableWindow(TRUE);
			m_btnRecover.EnableWindow(TRUE);
			m_lstRecover.EnableWindow(TRUE);
			m_chkSelectAll.EnableWindow(TRUE);
			m_chkSelectAll.SetCheck(true);
			//Varada Ikhar, Date: 4th May-2015
			//Issue: While recover/delete operation is in progress, if any other tab is clicked and clicked 'Yes' to stop confirmation box, Cleanning operation gets aborted.
			//However, next time when clicked on any other tab again "Recover/Delete is in progress. Do you want to stop?" message gets displayed.
			m_bRecoverThreadStart = false;
			m_bDeleteThreadStart = false;
		}
		//	m_chkSelectAll.SetCheck(true);
		//	m_btnDelete.EnableWindow(true);

		m_bRecoverClose = false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExRecoverDlg::StopReportOperation", 0, 0, true, SECONDLEVEL);
	}
	return true;
}


/***************************************************************************************************                    
*  Function Name  :           ShowRecoverEntriesThread                                                     
*  Description    :           Thread gets created for purpose that when user clicks on recover button
							  from tab controls at that time it loads recover entries from Db files.  
*  Author Name    :           Neha Gharge 
*  SR_NO		  :
*  Date           :           20 Nov 2013
****************************************************************************************************/
DWORD WINAPI ShowRecoverEntriesThread(LPVOID lpvThreadParam)
{
	try
	{
		CISpyRecoverDlg *pThis = (CISpyRecoverDlg*)lpvThreadParam;
		if(pThis)
		{
			pThis->BeginWaitCursor(); //this line did execute
			//Issue no :1223 recover entries are not loading properly.
			pThis->m_btnDelete.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REPORT_LOAD"));
			pThis->m_btnDelete.EnableWindow(FALSE);
			pThis->m_btnRecover.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REPORT_LOAD"));
			pThis->m_btnRecover.EnableWindow(FALSE);
			
			pThis->m_lstRecover.DeleteAllItems();
			
			//Remove all the entries now
			pThis->m_objRecoverdbToSave.RemoveAll();

			pThis->m_chkSelectAll.SetCheck(TRUE);

			pThis->LoadExistingRecoverFile(true);
			pThis->PopulateList(true);

			pThis->LoadRemainingEntries();

			pThis->m_hShowRecoverEntriesThread = NULL;
			pThis->EndWaitCursor(); //this line did execute

			pThis->m_btnDelete.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_DELETE"));
			pThis->m_btnDelete.EnableWindow(TRUE);
			pThis->m_btnRecover.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RECOVER"));
			pThis->m_btnRecover.EnableWindow(TRUE);
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CGenXAntiRootkit::GetPercentage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************                    
*  Function Name  :           OnBnClickedRecover                                                     
*  Description    :           This function is called when user clicks on recover button from tab control 
*  Author Name    :           Neha Gharge 
*  SR_NO		  :
*  Date           :           10 Jun 2014
****************************************************************************************************/
void CISpyRecoverDlg::OnBnClickedRecover()
{
	if(m_hShowRecoverEntriesThread != NULL)
	{
		return;
	}
	//Issue No.17 While recover and deletion is in progress if I click Recover button then Recover does not takes place
	// Gaurav Chopdar Date:6/1/2015
	if(m_bRecoverEnableDisable==false)
	{
		m_hShowRecoverEntriesThread = ::CreateThread(NULL, 0, ShowRecoverEntriesThread, (LPVOID) this, 0, NULL);
		Sleep( 10 ) ;
	}
}

/***************************************************************************************************                    
*  Function Name  :           BrowseFileToSaveRecover                                                     
*  Description    :           If original recover path drive is not exist,user browse folder where
							  user want to save recovered file.
*  Author Name    :           Neha Gharge 
*  SR_NO		  :
*  Date           :           10 Jun 2014
****************************************************************************************************/
bool CISpyRecoverDlg::BrowseFileToSaveRecover(CString &csBroswseFilepath)
{
/*	CFile hBrowseFile;
	static TCHAR szEncFilter[] = L"*.txt|";   
	CFileDialog FileDlg(FALSE, L"txt", L"RecoverFile1", OFN_CREATEPROMPT|OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY
  | OFN_OVERWRITEPROMPT, (LPCTSTR)szEncFilter);
	if( FileDlg.DoModal() == IDOK )
	{
		CString csFilePath = FileDlg.GetPathName();
		csBroswseFilepath = csFilePath;
	}
	if(!PathFileExists(csBroswseFilepath))
	{
		if(hBrowseFile.Open(csBroswseFilepath,CFile::modeCreate | CFile::modeWrite) == FALSE )
			return false;
		hBrowseFile.Close();
	}
	if(!PathFileExists(csBroswseFilepath))
	{
		return false;
	}
	return true*/;

	TCHAR *pszPath = new TCHAR[MAX_PATH];
	SecureZeroMemory(pszPath, MAX_PATH*sizeof(TCHAR));

	CString csMessage = L"Please select folder for recover files.";
	BROWSEINFO        bi = {m_hWnd, NULL, pszPath, csMessage, BIF_RETURNONLYFSDIRS, NULL, 0L, 0};
	LPITEMIDLIST      pIdl = NULL;

	LPITEMIDLIST  pRoot = NULL;
	SHGetFolderLocation(m_hWnd, CSIDL_DRIVES, 0, 0, &pRoot);
	bi.pidlRoot = pRoot;
	pIdl = SHBrowseForFolder(&bi);
	if(NULL != pIdl)
	{
		SHGetPathFromIDList(pIdl, pszPath);
		size_t iLen = wcslen(pszPath);
		if(iLen > 0)
		{
			
			csBroswseFilepath = pszPath;
			if(!PathFileExists(csBroswseFilepath))
			{
				return false;
			}
			//if(csBroswseFilepath.Right(1) == L"\\")
			//{
			//	csBroswseFilepath = csBroswseFilepath.Left(iLen -1);
			//}
			
			
		}
	}
	delete [] pszPath;
	pszPath = NULL;
	return true;
}

/***************************************************************************************************                    
*  Function Name  :           GetQuarantineFolderPath                                                     
*  Description    :           Get quarantine folder(backup folder) path.
*  Author Name    :           Neha Gharge 
*  SR_NO		  :
*  Date           :           10 Jun 2014
****************************************************************************************************/
CString CISpyRecoverDlg::GetQuarantineFolderPath()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = {0};
		if(!GetModulePath(szModulePath, MAX_PATH))
		{
			return L"";
		}
		CString csQuarantineFolderPath = szModulePath;
		return csQuarantineFolderPath += L"\\Quarantine";
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CScanDlg::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}


/***************************************************************************************************                    
*  Function Name  :           DeleteREntries                                                     
*  Description    :           This function is called for deletion of records  
*  Author Name    :           Nitin K. Kolapkar  
*  SR_NO		  :
*  Date           :           13th June 2014
****************************************************************************************************/
void CISpyRecoverDlg::DeleteREntries()
{

	//BOOL bCheck;
	bool bSuccess = true;
	CString csFilePath = L"";
	bool	bSelected = false ;
	CString csThreatPath = L"",csVirusName = L"";
	int i = 0; 
	m_bDeleteThreadStart = true;
	
	AddLogEntry(L">>> Deleting entry started", 0, 0, true, FIRSTLEVEL);
	m_iTotalEntriesCount = m_lstRecover.GetItemCount();
	if(!SendRecoverOperations2Service(RELOAD_DBENTRIES , L"", L"", RECOVER,0, true))
	{
		AddLogEntry(L"### Error in CRecoverDlg::OnBnClickedButtonDelete", 0, 0, true, SECONDLEVEL);
		goto Cleanup;
	}

	DWORD dwDeleteCount = 0;
	do
	{
		for(i=0; i< m_lstRecover.GetItemCount(); i++)
		{
			if(m_bDeleteStop)
			{
				break;
			}
			//bCheck = m_lstRecover.GetCheck(i);
			if(m_lstRecover.GetCheck(i))
			{
				csThreatPath = m_lstRecover.GetItemText(i,1);
				Sleep(5);
				//if(SendRecoverOperations2Service(RECOVER_FILE, csThreatPath,L"",0,1,true))

				//Added by Vilas on Mar 27 2015 for delete a entry from recover database
				ISPY_PIPE_DATA	szPipeData = { 0 };

				szPipeData.iMessageInfo = RECOVER_FILE;
				_tcscpy_s(szPipeData.szFirstParam, csThreatPath);
				_tcscpy_s(szPipeData.szSecondParam, L"");
				szPipeData.dwValue = 0;
				szPipeData.dwSecondValue = 1;

				SendRecoverOperations2Service(&szPipeData, true);
				if (!szPipeData.dwValue)
				{
					m_lstRecover.DeleteItem(i) ;
				}
				else
				{
					bSuccess = false;
					AddLogEntry(L"### Error in CRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
				}

				bSelected = true ;
				dwDeleteCount++;
				break;
			}
		}
		if( i >= m_lstRecover.GetItemCount() )
		break ;

		if(m_bDeleteStop)  
		{
			break;
		}
	}while( bSelected ) ;

	Sleep(5);
	if(!SendRecoverOperations2Service(SAVE_RECOVERDB , L"",L"",0,0, true))
	{
		AddLogEntry(L"### Error in CRecoverDlg::SendRecoverOperations2Service SAVE_RECOVERDB", 0, 0, true, SECONDLEVEL);
	}

	Sleep(5);
	LoadExistingRecoverFile(false);

	if(!m_bDeleteStop)
	{
		TCHAR szRecoverMsg[MAX_PATH] = {0};
		
		m_bIsPopUpDisplayed = true;
		if(dwDeleteCount == 0)
		{
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_NO_FILE_DELETED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		}
		else
		{
			swprintf(szRecoverMsg, L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_FILES_DELETED_SUCCESSFULLY"));
			MessageBox(szRecoverMsg, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION | MB_OK);
		}
		m_bIsPopUpDisplayed = false;

	}
	AddLogEntry(L">>> recovering entry finished", 0, 0, true, FIRSTLEVEL);

Cleanup:
	m_bDeleteThreadStart = false;
	m_btnRecover.EnableWindow(true);
	m_btnDelete.EnableWindow(true);
	/* Issue: While delete is in progress and we unclick the checkbox entries doesn't get delete 
		Resolved by : Nitin K Date: 29-Dec-2014
	*/
	m_lstRecover.EnableWindow(true);
	// Issue: While delete is in progress and if we uncheck the checkbox,Deletion process shows pop-up "Delete files Successfully".
	//Name: Niranjan Deshak - 3/2/2015.
	m_chkSelectAll.EnableWindow(true);
}

/***************************************************************************************************                    
*  Function Name  :           OnPaint                                                     
*  Description    :           The framework calls this member function when Windows or an application 
							  makes a request to repaint a portion of an application's window.
*  Author Name    :           Nitin 
*  SR_NO		  :
*  Date           :           13 Jun 2014
****************************************************************************************************/
void CISpyRecoverDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CJpegDialog::Draw();
	CJpegDialog::OnPaint();
}



/**********************************************************************************************************
*  Function Name  :	SendRecoverOperations2Service
*  Description    :	Sends data to service to recover and delete entries which are selected and save remaining entries which are not selected.
*  SR.NO		  :
*  Author Name    : Vilas Suvarnakar
*  Date           : 27 Mar 2013
**********************************************************************************************************/
bool CISpyRecoverDlg::SendRecoverOperations2Service(ISPY_PIPE_DATA *pszPipeData, bool bWait)
{
	if (!m_objCom.SendData(pszPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CGenXRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if (bWait)
	{
		if (!m_objCom.ReadData(pszPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to Read data in CGenXRecoverDlg::SendRecoverOperations2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
		
	}

	return true;
}

/**********************************************************************************************************
*  Function Name  :	GetSortedFileNames
*  Description    :	Function to get file ID's in sorted order.
*  Author Name    : Ramkrushna Shelke
*  Date           : 6 Jan 2016
*  SR_NO		  :
**********************************************************************************************************/
bool CISpyRecoverDlg::GetSortedFileNames(vector<int> &vec)
{
	try
	{
		CString csQurFolderPath = GetQuarantineFolderPath();
		GetFileDigits(csQurFolderPath, vec);
		sort(vec.begin(), vec.end());
		
		if (vec.size() > 0)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExRecoverDlg::GetSortedFileNames", 0, 0, true, SECONDLEVEL);
	}
	return false;
}


/**********************************************************************************************************
*  Function Name  :	GetMaximumDigitFromFiles
*  Description    :	Function to get Maximum number from files
*  Author Name    : Ramkrushna Shelke
*  Date           : 6 Jan 2016
*  SR_NO		  :
**********************************************************************************************************/
void CISpyRecoverDlg::GetFileDigits(LPCTSTR pstr, vector<int> &vec)
{
	try
	{
		CFileFind finder;
		bool bIsFolder = false;
		// build a string with wildcards
		CString strWildcard(pstr);
		strWildcard += _T("\\*.DB");

		// start working for files
		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bIsFolder = true;
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			// if it's a directory, recursively search it 
			if (finder.IsDirectory())
			{
				continue;
			}

			CString csFileName = finder.GetFileName();
			CString csDigit = csFileName.Left(csFileName.ReverseFind(L'.'));
			csDigit = csDigit.Right(csDigit.GetLength() - (csDigit.ReverseFind(L'_') + 1));
			if (csDigit.Trim().GetLength() != 0)
			{
				DWORD dwDigit = _wtoi(csDigit);
				if (dwDigit != 0)
				{
					vec.push_back(dwDigit);
				}
			}
		}
		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CUSBDetectUIDlg::EnumTotalFolder", 0, 0, true, SECONDLEVEL);
	}
}


/**********************************************************************************************************
*  Function Name  :	LoadRemainingEntries
*  Description    :	Function to get remaining entries of recover.
*  Author Name    : Ramkrushna Shelke
*  Date           : 6 Jan 2016
*  SR_NO		  :
**********************************************************************************************************/
void CISpyRecoverDlg::LoadRemainingEntries()
{
	try
	{
		vector <int > vecNumList;
		
		//get the file ids and sort
		GetSortedFileNames(vecNumList);

		if (vecNumList.size() == 0)
		{
			return;
		}

		//Load the file contents in memory and populate in list control
		for (vector<int>::reverse_iterator it = vecNumList.rbegin(); it != vecNumList.rend(); ++it)
		{
			CString csFilePath;
			csFilePath.Format(L"%s\\WRDWIZRECOVERENTRIES_%d.DB", GetQuarantineFolderPath(), *it);
			if (PathFileExists(csFilePath))
			{
				LoadDataContentFromFile(csFilePath);
				PopulateList(true);
			}
		}

		//Save the DB file here
		SaveDBFile();

		//Remove the files which are already loaded and merged.
		for (vector<int>::reverse_iterator it = vecNumList.rbegin(); it != vecNumList.rend(); ++it)
		{
			CString csFilePath;
			csFilePath.Format(L"%s\\WRDWIZRECOVERENTRIES_%d.DB", GetQuarantineFolderPath(), *it);
			if (PathFileExists(csFilePath))
			{
				SetFileAttributes(csFilePath, FILE_ATTRIBUTE_NORMAL);
				if (DeleteFile(csFilePath) == FALSE)
				{
					MoveFileEx(csFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExRecoverDlg::LoadRemainingEntries", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	SaveDBFile
*  Description    :	According to type of module Save DB .
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0003
**********************************************************************************************************/
bool CISpyRecoverDlg::SaveDBFile()
{
	try
	{
		CString csFilePath = GetQuarantineFolderPath();
		csFilePath += _T("\\WRDWIZRECOVERENTRIES.DB");

		int iFileVersion = 1;
		CFile wFile(csFilePath, CFile::modeCreate | CFile::modeWrite);

		CArchive arStore(&wFile, CArchive::store);

		m_objRecoverdbToSave.SetFileVersion(iFileVersion);
		m_objRecoverdbToSave.Serialize(arStore);

		arStore.Close();
		wFile.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExDBManipulation::SaveDBFile", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}