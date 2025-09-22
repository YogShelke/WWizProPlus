// ISpyReportsDlg.cpp : implementation file
/****************************************************
*  Program Name: ISpyReportsDlg.cpp                                                                                                    
*  Description: Displays scanning information which includes:
                -> Scan date & time
				-> Scan type (QuickScan,FullScan,CustomScan,USBScan)
				-> Threat name (if any)
				-> File path
				-> Action (Detected,Deleted,Repaired,Scanning aborted,No threats found,No files to scan)
*  Author Name: Prajakta
*  Date Of Creation: 20 Nov 2013
*  Version No: 1.0.0.2
****************************************************/

/****************************************************
HEADER FILES
****************************************************/


#include "stdafx.h"
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"
#include "ISpyReportsDlg.h"


DWORD WINAPI DeleteReportThread( LPVOID lpParam ) ;
HANDLE	m_hThread_Report = NULL ;
CISpyGUIDlg* g_TabCtrlReportWindHandle = NULL;
// CISpyReportsDlg dialog

IMPLEMENT_DYNAMIC(CISpyReportsDlg, CDialog)

/**********************************************************************************************************                     
*  Function Name  :	CISpyReportsDlg                                                     
*  Description    :	C'tor
*  Author Name    : Prajakta  
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
CISpyReportsDlg::CISpyReportsDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CISpyReportsDlg::IDD, pParent)
	,m_bReportClose(false)
	,m_bReportHome(false)
	,m_bReportStop(false)
	,m_bReportThreadStart(false)
	,m_hReportsThread(NULL)
	, m_bReportEnableDisable(false)
	, m_bDeleteEntriesFinished(false)
	, m_bIsPopUpDisplayed(false)
	, m_objCom(SERVICE_SERVER, true)
{

}

/**********************************************************************************************************                     
*  Function Name  :	~CISpyReportsDlg                                                     
*  Description    :	Destructor
*  Author Name    : Prajakta   
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
CISpyReportsDlg::~CISpyReportsDlg()
{
	if(m_hReportsThread != NULL)
	{
		::SuspendThread(m_hReportsThread);
		::TerminateThread(m_hReportsThread, 0);
		m_hReportsThread = NULL;
	}
}

/**********************************************************************************************************                     
*  Function Name  :	DoDataExchange                                                     
*  Description    :	Called by the framework to exchange and validate dialog data.
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyReportsDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PIC_REPORTS_HEADER, m_stHReports);
	DDX_Control(pDX, IDC_CHECK_CHKBOX, m_btnChkbox);
	DDX_Control(pDX, IDC_STATIC_MSG, m_stMsg);
	DDX_Control(pDX, IDC_BTN_BACK, m_btnBack);
	DDX_Control(pDX, IDC_BTN_DELETE, m_btnDelete);
	DDX_Control(pDX, IDC_LIST_LISTVIEW, m_lstListView);
	DDX_Control(pDX, IDC_CHECK_SELECTALL, m_chkSelectAll);
	DDX_Control(pDX, IDC_STATIC_SELECTALL, m_stSelectAll);
	DDX_Control(pDX, IDC_STATIC_REPORTS_HEADER_NAME, m_stReportsHeaderName);
	DDX_Control(pDX, IDC_STATIC_REPORT_DESCRIPTION, m_stHeaderReportDes);
}

/***************************************************************************
* Function Name  : MESSAGE_MAP
* Description    : Handle WM_COMMAND,WM_Messages,user defined message 
				   and notification message from child windows.
*  Author Name    : Prajakta   
*  SR_NO		  :
*  Date           : 20 Nov 2014
****************************************************************************/
BEGIN_MESSAGE_MAP(CISpyReportsDlg, CJpegDialog)
		ON_WM_CTLCOLOR()
		ON_WM_NCHITTEST()
		ON_WM_SETCURSOR()
		ON_BN_CLICKED(IDC_BTN_BACK, &CISpyReportsDlg::OnBnClickedBtnBack)
		ON_BN_CLICKED(IDC_BTN_DELETE, &CISpyReportsDlg::OnBnClickedBtnDelete)
		ON_WM_SETCURSOR()
		ON_BN_CLICKED(IDC_CHECK_SELECTALL, &CISpyReportsDlg::OnBnClickedCheckSelectall)
		ON_WM_PAINT()
END_MESSAGE_MAP()


// CISpyReportsDlg message handlers
/**********************************************************************************************************                     
*  Function Name  :	OnInitDialog                                                     
*  Description    :	Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft 
					Foundation Class Library dialog boxes
*  SR.NO		  : 
*  Author Name    : Prajakta
*  Date           : 20 Nov 2013
**********************************************************************************************************/
BOOL CISpyReportsDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	/*****************ISSUE NO - Neha Gharge 22/5/14 ************************************************/
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window
	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_INNER_DIALOG), _T("JPG")))
	{
		m_bIsPopUpDisplayed = true;
		::MessageBox(NULL,theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
		m_bIsPopUpDisplayed = false;
	}

	Draw();

	CRect rect1;
	this->GetClientRect(rect1);
	SetWindowPos(NULL, 1, 88, rect1.Width()-5, rect1.Height() - 5, SWP_NOREDRAW);
	m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));


	//m_stReportsHeaderName.SetWindowPos(&wndTop,rect1.left +20,15,540,31,SWP_NOREDRAW);	
	m_stReportsHeaderName.SetWindowPos(&wndTop, rect1.left + 20, 07, 540, 31, SWP_NOREDRAW);
	m_stReportsHeaderName.SetTextColor(RGB(24,24,24));
	m_stReportsHeaderName.SetBkColor(RGB(230, 232, 238));
	// Issue Add description Neha Gharge 9-3-2015
	m_stHeaderReportDes.SetWindowPos(&wndTop, rect1.left + 20, 35, 400, 15, SWP_NOREDRAW);
	m_stHeaderReportDes.SetTextColor(RGB(24, 24, 24));
	m_stHeaderReportDes.SetBkColor(RGB(230, 232, 238));
		
	//if(theApp.m_dwOSType == WINOS_WIN8 ||theApp.m_dwOSType == WINOS_WIN8_1)
	//{
	//	m_stReportsHeaderName.SetWindowPos(&wndTop,rect1.left +20,16,540,31,SWP_NOREDRAW);	
	//}
	m_bmpReports = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_COM_HEADER));
	m_stHReports.SetWindowPos(&wndTop,rect1.left + 6,10,586,45,SWP_NOREDRAW);
	m_stHReports.SetBitmap(m_bmpReports);

	/*m_btnChkbox.SetWindowPos(&wndTop,rect1.left + 60,70,15,15,SWP_SHOWWINDOW);
	m_btnChkbox.SetCheck(true);*/

	//CString csMsg;
	//csMsg.Format(L"Delete older reports (30 days)");
	//CFont *m_font = new CFont;
	//m_font->CreatePointFont(80,_T("Verdana"));
	//m_stMsg.SetWindowTextW(csMsg);
	//m_stMsg.SetWindowPos(&wndTop,rect1.left + 80,70,200,20,SWP_SHOWWINDOW);	
	//m_stMsg.SetFont(m_font);

	//ListCtrl
	/*m_lstListView.InsertColumn(0, L"Date and Time", LVCFMT_LEFT, 125);
	m_lstListView.InsertColumn(1, L"Scan Type", LVCFMT_LEFT, 75);
	m_lstListView.InsertColumn(2, L"Threat Name", LVCFMT_LEFT, 120);
	m_lstListView.InsertColumn(3, L"File Path",LVCFMT_LEFT,212);
	m_lstListView.InsertColumn(4, L"Action",LVCFMT_LEFT,127);
	*/
	ListView_SetExtendedListViewStyle (m_lstListView.m_hWnd, LVS_EX_CHECKBOXES |  LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);
	
	//Varada Ikhar, Date:26/02/2015, Issue:The UI look for space between header and Listview should be standard.
	m_lstListView.SetWindowPos(&wndTop, rect1.left + 6, 68, 586, 257, SWP_NOREDRAW);

	//******************Issue :720 Neha Gharge 24/5/2014 *****************************/
	m_btnDelete.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnDelete.SetWindowPos(&wndTop,rect1.left + 535,353,57,21,SWP_NOREDRAW);
	m_btnDelete.SetTextColorA(BLACK,1,1);

	//m_btnBack.SetSkin(IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROWDISABLE,0,0,0,0);
	//m_btnBack.SetWindowPos(&wndTop,21,354,31,32,SWP_NOREDRAW);
	m_btnBack.ShowWindow(SW_HIDE);
	
	// *****************************ISSUE NO : 391 Neha gharge 24/5/2014 *******************************************/
	m_chkSelectAll.SetWindowPos(&wndTop,rect1.left + 8,335,13,13,SWP_NOREDRAW);
	m_chkSelectAll.SetCheck(true);
	m_stSelectAll.SetWindowPos(&wndTop,rect1.left + 27,334,150,15,SWP_NOREDRAW);
	m_stSelectAll.SetTextColor(RGB(255,255,255));
	// Issue Add description Neha Gharge 9-3-2015
	//setting Fonts...
	m_stReportsHeaderName.SetFont(&theApp.m_fontWWTextSmallTitle);
	m_stHeaderReportDes.SetFont(&theApp.m_fontWWTextSubTitleDescription);
	m_stSelectAll.SetFont(&theApp.m_fontWWTextNormal);

	CHeaderCtrl* pHeaderCtrl = m_lstListView.GetHeaderCtrl();
	pHeaderCtrl->SetFont(&theApp.m_fontWWTextNormal);
	
	m_btnDelete.SetFont(&theApp.m_fontWWTextNormal);
	RefreshStrings();
	//This is for testing has to be removed use function OnBnClickedReports called when button Reports ius clicked.
	//ShowControls4Reports(); 
		
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/***********************************************************************************************
*  Function Name  : RefreshString
*  Description    : this function is  called for setting the Text UI with different Language Support
*  Author Name    : Prassana
*  SR_NO		  :
*  Date           : 30 April 2014
***********************************************************************************************/
void CISpyReportsDlg::RefreshStrings()
{
	m_stReportsHeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REPORT_HEADER_NAME"));
	// Issue Add description Neha Gharge 9-3-2015
	m_stHeaderReportDes.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REPORTS_HEADER_DESCRIPTION"));
	CString temp;
	//temp.Format(L"%s",theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REPORTS_LST_DATE_TIME"));
	//CHeaderCtrl* pHeaderCtrl = m_lstListView.GetHeaderCtrl();
	//pHeaderCtrl->SetFont(&theApp.m_fontTextNormal)

	/*****************ISSUE NO -105 Neha Gharge 22/5/14 ************************************************/
	m_lstListView.InsertColumn(0, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REPORTS_LST_DATE_TIME"), LVCFMT_LEFT, 125);
	m_lstListView.InsertColumn(1, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REPORTS_LST_SCAN_TYPE"), LVCFMT_LEFT, 75);
	m_lstListView.InsertColumn(2, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REPORTS_LST_THREAT_NAME"), LVCFMT_LEFT, 120);
	m_lstListView.InsertColumn(3, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REPORTS_LST_FILE_PATH"),LVCFMT_LEFT,165);
	m_lstListView.InsertColumn(4,theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REPORTS_LST_ACTION"),LVCFMT_LEFT,97);
	m_stSelectAll.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SELECTALL"));
	m_btnDelete.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_DELETE"));
}

/***********************************************************************************************
*  Function Name  : ShowReportsThread
*  Description    : When user clicks on reports button from tab control.This thread gets call 
					Loading reports and populate on list control functionality gets occured.
*  Author Name    : Prassana
*  SR_NO		  :
*  Date           : 30 April 2014
***********************************************************************************************/
DWORD WINAPI ShowReportsThread(LPVOID lpvThreadParam)
{
	try
	{
		CISpyReportsDlg *pThis = (CISpyReportsDlg*)lpvThreadParam;
		if(pThis)
		{
			pThis->BeginWaitCursor(); //this line did execute
			
			//Remove all items from list before fill
			//When big DB is there it take time to reload.
			//Neha Gharge 17/12/2015
			pThis->m_btnDelete.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REPORT_LOAD"));
			pThis->m_btnDelete.EnableWindow(FALSE);

			if (pThis->m_lstListView.GetItemCount() > 0)
			{
				pThis->m_lstListView.DeleteAllItems();
			}

			//Remove all the entries now
			pThis->m_objReportsDBToSave.RemoveAll();

			//Load other entries here
			pThis->LoadDBFile();
			pThis->PopulateList(); 

			//Load Remaining entries first
			pThis->LoadRemainingEntries();

			pThis->m_btnDelete.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_DELETE"));
			pThis->m_btnDelete.EnableWindow(TRUE);

			pThis->ShowHideControls(true);
			pThis->m_hReportsThread = NULL;
			pThis->EndWaitCursor(); //this line did execute
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CGenXAntiRootkit::GetPercentage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************                     
*  Function Name  :	ShowControls4Reports                                                     
*  Description    :	Creates ShowReportsThread thread
*  Author Name    : Prajakta  
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyReportsDlg::ShowControls4Reports()
{
	if(m_hReportsThread != NULL)
	{
		return;
	}
//Issue No.16 While report deletion is in progress if I click Report button then Report does not takes place
//Gaurav Chopdar Date:2/1/2015
      if(m_bReportEnableDisable ==false)
	  {
	m_hReportsThread = ::CreateThread(NULL, 0, ShowReportsThread , (LPVOID) this, 0, NULL);
	Sleep( 10 ) ;
	  }
}

/**********************************************************************************************************                     
*  Function Name  :	OnCtlColor                                                     
*  Description    :	The framework calls this member function when a child control is about to be drawn.
*  Author Name    : Prajakta 
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
HBRUSH CISpyReportsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if( ctrlID == IDC_STATIC_MSG		||
		ctrlID == IDC_STATIC_REPORTS_HEADER_NAME ||
		ctrlID == IDC_STATIC_SELECTALL)
	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	} return hbr;
}

/**********************************************************************************************************                     
*  Function Name  :	OnNcHitTest                                                     
*  Description    :	The framework calls this member function for the CWnd object that containsi the cursor every time the mouse is moved.
*  Author Name    : Prajakta  
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
LRESULT CISpyReportsDlg::OnNcHitTest(CPoint point)
{
	return HTCLIENT;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnBack                                                     
*  Description    :	Allows to go back to parent dialog i.e home window
*  SR.NO		  : 
*  Author Name    : Prajakta                                                                                          
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyReportsDlg::OnBnClickedBtnBack()
{
	this->ShowWindow(SW_HIDE);
	//CISpyGUIDlg *pObjMainUI = reinterpret_cast<CISpyGUIDlg*>( this->GetParent() );
	//if(pObjMainUI)
	//{
	//	pObjMainUI->ShowHideMainPageControls(true);
	//}
	ShowHideControls(true);
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnDelete                                                     
*  Description    :	Deletes selected entry from reports
*  SR.NO          : 
*  Author Name    : Prajakta                                                                                          
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyReportsDlg::OnBnClickedBtnDelete()
{
	try
	{
		int		index = 0;
		m_btnBack.EnableWindow(false);
		index = m_lstListView.GetItemCount();
		//Varada Ikhar, Date:12-03-2015
		//Issue:If 'select all' checkbox is unchecked & clicked on 'delete', then immdetiately if 'select all' checkbox is clicked before popup appears it is getting checked & entries are getting removed.
		m_chkSelectAll.EnableWindow(false);

		/* Issue No: 24 While report deletion is in progress and we unclick the checkbox entries doesn't get deleted
			Resolved by : Nitin K Date: 19-Dec-2014
			*/

		m_lstListView.EnableWindow(false);
		g_TabCtrlReportWindHandle = (CISpyGUIDlg*)AfxGetMainWnd();
		// lalit  4-27-2015 , due to mutuall operation implementaion no more need to hide other tab control when any one is in-progress 
		// g_TabCtrlReportWindHandle->m_pTabDialog->DisableAllExceptSelected();

		//Varada Ikhar, Date:17-03-2015, 
		//Issue:User updated the product and does not restarted, and started a scan, then it should show message as 
		//"Product updated, restart is required to take a effect."		
		if (!theApp.ShowRestartMsgOnProductUpdate())
		{
			//Ticket no 0000258 if it return from here it should be enable again
			//Neha Gharge.28/4/2015
			m_chkSelectAll.EnableWindow(true);
			return;
		}
		//issue 0000492, when click on delete button of report when report have no item to delete,in such case it it prompting user "report deletion in progress which is not required." 
		// resolved by lalit kumawat 5-27-2015 
		if (index == 0x00)
		{
			m_bDeleteEntriesFinished = true;
			
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REPORTS_NO_ENTRY_TO_DELETE"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);  //ISSUE NO.:17 , VARADA IKHAR DATE: 19/12/2014 TIME: 11:46
			m_bIsPopUpDisplayed = false;
			
			m_chkSelectAll.EnableWindow(true);
			m_lstListView.EnableWindow(true);
			
		}
		else
		{
			//Varada Ikhar, Date : 29/04/2015
			//Issue : In reports, if messge box for 'No entries to delete' is present and tried to exit from tray, debug assertion fail error occur.
			m_bDeleteEntriesFinished = false;

			m_hThread_Report = ::CreateThread(NULL, 0, DeleteReportThread, (LPVOID) this, 0, NULL);
			Sleep(1000);
		}
		
		
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExReportsDlg::OnBnClickedBtnDelete.", 0, 0, true, SECONDLEVEL);
	}
	
}

/**********************************************************************************************************                     
*  Function Name  :	DeleteReportThread                                                     
*  Description    :	Allows single/multiple entries deletion from reports & accordingly updates the reports log
*  SR.NO          : 
*  Author Name    : Vilas                                                                                          
*  Date           : 20 Nov 2013
**********************************************************************************************************/
DWORD WINAPI DeleteReportThread( LPVOID lpParam )
{

	CISpyReportsDlg		*pRptDlg = (CISpyReportsDlg * ) lpParam ;

	if( !pRptDlg )
		return 0 ;
	//Issue No.16 Gaurav Chopdar Date:2/1/2015
	pRptDlg->m_bReportEnableDisable = true; 
	AddLogEntry(L">>> Deleting report entry started", 0, 0, true, FIRSTLEVEL);

	pRptDlg->m_btnDelete.EnableWindow( FALSE ) ;
	pRptDlg->m_bReportStop = false;
	pRptDlg->m_bReportThreadStart = true;
	bool	bSelected = false ;
	DWORD	dwRet = 0x00 ;

	FILE *pFile = NULL;
	TCHAR szModulePath[MAX_PATH] = {0};
	TCHAR szFilePath[MAX_PATH] = {0};
	int		index = 0 ;
	DWORD	dwDeletedEntries = 0x00;
	CString csDateTime = L"", csScanType = L""; 
	try
	{

		if(!GetModulePath(szModulePath, MAX_PATH))
		{
			//Varada Ikhar Date:29/04/2015
			//Issue : In reports, if messge box for 'No entries to delete' is present and tried to exit from tray, debug assertion fail error occur.
			pRptDlg->m_bDeleteEntriesFinished = true;

			pRptDlg->m_bIsPopUpDisplayed = true;
			pRptDlg->MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_REPORT_GETMODULEPATH_ERR"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR|MB_OK); //ISSUE NO.:17 , VARADA IKHAR DATE: 19/12/2014 TIME: 11:46
			pRptDlg->m_bIsPopUpDisplayed = false;

			//return 1 ;
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		CString csFilePath(szModulePath);
		csFilePath += _T("\\WRDWIZREPORTS.DB") ;

		index = pRptDlg->m_lstListView.GetItemCount() ;
		//issue 0000492, when click on delete button of report when report have no item to delete,in such case it it prompting user "report deletion in progress which is not required." 
		// resolved by lalit kumawat 5-27-2015 

		//if( index == 0x00 )
		//{
		//	//Varada Ikhar Date:29/04/2015
		//	//Issue : In reports, if messge box for 'No entries to delete' is present and tried to exit from tray, debug assertion fail error occur.
		//	pRptDlg->m_bDeleteEntriesFinished = true;
		//	//MessageBox(NULL, L"No entry for deletion", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION) ;
		//	pRptDlg->MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REPORTS_NO_ENTRY_TO_DELETE"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION) ;  //ISSUE NO.:17 , VARADA IKHAR DATE: 19/12/2014 TIME: 11:46
		//	goto Cleanup ;
		//}

		if(!pRptDlg->SendReportsData2Service(RELOAD_DBENTRIES, REPORTS, L"", true))
		{
			AddLogEntry(L"### failed to RELOAD_DBENTRIES SendReportsData2Service", 0, 0, true, SECONDLEVEL);
			dwRet = 0x01 ;
			goto Cleanup ;
		}
		
		//Varada Ikhar, Date:12-03-2015
		//Issue:If 'select all' checkbox is unchecked & clicked on 'delete', then immdetiately if 'select all' checkbox is clicked before popup appears it is getting checked & entries are getting removed.
		//Commented the below statement.
		//Issue No.-5 while deleting reports,select all checkbox should be disabled while deletion is in progress.-Niranjan Deshak.-1/1/2015
		//pRptDlg->m_chkSelectAll.EnableWindow(false);

		do
		{
			for(index = 0; index < pRptDlg->m_lstListView.GetItemCount(); index ++)
			{
				if(pRptDlg->m_bReportStop)
				{
					break;
				}
				if(pRptDlg->m_lstListView.GetCheck(index) )
				{
					csDateTime = pRptDlg->m_lstListView.GetItemText(index, 0).Trim();
					csScanType = pRptDlg->m_lstListView.GetItemText(index, 1).Trim();
					csFilePath = pRptDlg->m_lstListView.GetItemText(index, 3).Trim();
					if(pRptDlg->SendReportsOperation2Service(DELETE_REPORTS_ENTRIES, csDateTime, csScanType, csFilePath, true))
					{
						pRptDlg->m_lstListView.DeleteItem(index) ;
					}
					Sleep(10);
					bSelected = true ;
					dwDeletedEntries++ ;
					break ;
				}
				//index--;
			}

			if( index >= pRptDlg->m_lstListView.GetItemCount() )
				break ;

			if(pRptDlg->m_bReportStop)
			{
				break;
			}

		}
		while( bSelected ) ;
		
		//for(index = 0; index < pRptDlg->m_lstListView.GetItemCount(); index ++)
		//{
		//	pRptDlg->m_lstListView.SetCheck(index,true);
		//}
			
		if( !bSelected )
		{
			//Varada Ikhar Date:29/04/2015
			//Issue : In reports, if messge box for 'No entries to delete' is present and tried to exit from tray, debug assertion fail error occur.
			pRptDlg->m_bDeleteEntriesFinished = true;
			//MessageBox(NULL, L"No entries selected for deletion !", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION );

			pRptDlg->m_bIsPopUpDisplayed = true;
			pRptDlg->MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REPORTS_SELECTED_FOR_DELETION"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION) ; //ISSUE NO.:17 , VARADA IKHAR DATE: 19/12/2014 TIME: 11:46
			pRptDlg->m_bIsPopUpDisplayed = false;

			//return 2 ;
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		if(!pRptDlg->SendReportsOperation2Service(SAVE_REPORTS_ENTRIES, L"", L"", L"", true))
		{
			AddLogEntry(L"### Failed to send SAVE_REPORTS_ENTRIES in CGenXReportsDlg : DeleteReportThread", 0, 0, true, SECONDLEVEL);
		}
		/*if(PathFileExists(csFilePath))
		{
			DeleteFile(csFilePath);
		}*/

		/*pRptDlg->m_objReportsDB.RemoveAll();				

		csFilePath = "";
		CString		csReportEntry, csFullStr ,csDateTime ,csScanType,csThreatName,csAction;
		for(int index = 0;index < pRptDlg->m_lstListView.GetItemCount(); index++)
		{
			CString csItemString;
			LVITEM lvItem;
			int nItem = 0;
			int imgNbr = 0;
			if(index == 0)
			{
				lvItem.mask = LVIF_IMAGE;
				lvItem.iItem = 0;
				lvItem.iSubItem = 0;
				lvItem.iImage = 0;
				nItem = pRptDlg->m_lstListView.GetItem(&lvItem);
			}
			csDateTime = pRptDlg->m_lstListView.GetItemText(index, 0).Trim();
			csScanType = pRptDlg->m_lstListView.GetItemText(index, 1).Trim();
			csThreatName = pRptDlg->m_lstListView.GetItemText(index, 2).Trim();
			csFilePath = pRptDlg->m_lstListView.GetItemText(index, 3).Trim();
			csAction = pRptDlg->m_lstListView.GetItemText(index, 4).Trim();
			pRptDlg->AddEntriesInReportsDBAfterDelete(csDateTime,csScanType,csThreatName,csFilePath,csAction);
		}
		pRptDlg->SaveInReportsDB();*/
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CGenXReportsDlg::OnBnClickedBtnDelete", 0, 0, true, SECONDLEVEL);
	}

	//Varada Ikhar Date:29/04/2015
	//Issue : In reports, if messge box for 'No entries to delete' is present and tried to exit from tray, debug assertion fail error occur.
	pRptDlg->m_bDeleteEntriesFinished = true;

	if(!pRptDlg->m_bReportStop)
	{
		pRptDlg->m_bIsPopUpDisplayed = true;
		if( dwDeletedEntries > 0x01 )
			//MessageBox(NULL, L"Selected entries deleted successfully", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
			pRptDlg->MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REPORTS_SELECTED_ENTRIES"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION) ; //ISSUE NO.:17 , VARADA IKHAR DATE: 19/12/2014 TIME: 11:46
		else
			//MessageBox(NULL, L"Selected entry deleted successfully", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
			pRptDlg->MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REPORTS_SELECTED_ENTRY"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION) ; //ISSUE NO.:17 , VARADA IKHAR DATE: 19/12/2014 TIME: 11:46
		//issue no 1197 : Encryption popup not coming after saying OK on message box
		pRptDlg->m_bIsPopUpDisplayed = false;

	}
	//Issue No.-5 while deleting reports,select all checkbox should be disabled while deletion is in progress.-Niranjan Deshak.-1/1/2015
	pRptDlg->m_chkSelectAll.EnableWindow(true);
	
	AddLogEntry(L">>>Report entry deleted successfully", 0, 0, true, FIRSTLEVEL);
Cleanup :
	pRptDlg->m_btnDelete.EnableWindow(true) ;
	pRptDlg->m_btnBack.EnableWindow(true);
	pRptDlg->m_bReportThreadStart = false;
	/* Issue No: 24 While report deletion is in progress and we unclick the checkbox entries doesn't get deleted
		Resolved by : Nitin K Date: 19-Dec-2014
	*/
	pRptDlg->m_lstListView.EnableWindow(true);
	g_TabCtrlReportWindHandle->m_pTabDialog->EnableAllBtn();
	//Ticket no 0000258 if it return from here it should be enable again //28/04/2015.
	//Neha Gharge As per design on all UI it is never checked item is thr or not
	//So according to present design it should be enable. Except while performing actual deletion.
	// resloved by lalit ,1/12/2015 :- issue  In reports,if we unselect checkbox and click on delete again slect the checkbox,user is not able to select the checkbox.
	//int i_ReportItemCount = 0;
	//i_ReportItemCount = pRptDlg->m_lstListView.GetItemCount();
	//if(i_ReportItemCount > 0)
	//{
		pRptDlg->m_chkSelectAll.EnableWindow(true);  
	//}
	//Issue No.16 Gaurav Chopdar Date:2/1/2015
    pRptDlg->m_bReportEnableDisable = false; 
	pRptDlg->m_lstListView.EnableWindow(true);
	return dwRet ;
}

/**********************************************************************************************************                     
*  Function Name  :	AddEntriesInReportsDBAfterDelete                                                     
*  Description    :	Add remaining entries back to reports db files.
*  SR.NO          : 
*  Author Name    : Vilas                                                                                          
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyReportsDlg::AddEntriesInReportsDBAfterDelete(CString csDateTime, CString csScanType, CString csThreatName, CString csFilePath, CString csAction)
{
	AddLogEntry(L">>> Adding entry into DB files", 0, 0, true, FIRSTLEVEL);
	m_objReportsDB.bWithoutEdit=0;
	m_objReportsDB.SetFileVersion(2);

	CIspyList newEntry(csDateTime,csScanType,csThreatName,csFilePath,csAction);
	m_objReportsDB.AddContact(newEntry, true);

}

/**********************************************************************************************************                     
*  Function Name  :	SaveInReportsDB                                                     
*  Description    :	Save entries to reports db files.
*  SR.NO          : 
*  Author Name    : Vilas                                                                                          
*  Date           : 20 Nov 2013
**********************************************************************************************************/
bool CISpyReportsDlg::SaveInReportsDB()
{
	return false;
}
//{
//	OutputDebugString(L">>> In SaveInReportsDB");
//
//	FILE *pFile = NULL;
//	static CString csFilePath;
//	TCHAR szModulePath[MAX_PATH] = {0};
//	if(!GetModulePath(szModulePath, MAX_PATH))
//	{
//		MessageBox(L"Error in GetModulePath", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR|MB_OK);
//		return false;
//	}
//	csFilePath = szModulePath;
//	csFilePath += _T("\\WRDWIZREPORTS.DB");
//		
//	m_bSuccess=0;
//	CFile wFile(csFilePath, CFile::modeCreate | CFile::modeWrite);
// 
//	// Create a storing archive
//	CArchive arStore(&wFile,CArchive::store);
//	m_objReportsDB.Serialize(arStore);
//	// Close the storing archive
//	arStore.Close();
//	wFile.Close();
// 
//	if(!m_bSuccess)
//	{
//		return true;
//	}
//	return false;
//}


/**********************************************************************************************************                     
*  Function Name  :	LoadDBFile                                                     
*  Description    :	Creates WRDWIZREPORTS.DB file & retrieves its contents
*  SR.NO		  : ITINAVREPORTS_0002
*  Author Name    : Prajakta                                                                                          
*  Date           : 20 Nov 2013
**********************************************************************************************************/
bool CISpyReportsDlg::LoadDBFile()
{
	try
	{
		AddLogEntry(L">>> Loading entry from DB files", 0, 0, true, FIRSTLEVEL);
		CString csFilePath;
		TCHAR szModulePath[MAX_PATH] = {0};
		if(!GetModulePath(szModulePath, MAX_PATH))
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_REPORT_GETMODULEPATH_ERR"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR|MB_OK);
			m_bIsPopUpDisplayed = false;
			return false;
		}
		csFilePath = szModulePath;
		csFilePath.Append(L"\\WRDWIZREPORTS.DB");

		//clear all reports entries here
		m_objReportsDB.RemoveAll();

		if(!PathFileExists(csFilePath))
		{
			return false;
		}

		CFile rFile(csFilePath, CFile::modeRead);
		// Create a loading archive
		CArchive arLoad(&rFile, CArchive::load);
		m_objReportsDB.Serialize(arLoad);

		// Close the storing archive
		arLoad.Close();
		rFile.Close();
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CGenXReportsDlg::LoadDBFile", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	PopulateList                                                     
*  Description    :	Displays entries on reports dialog
*  SR.NO		  : ITINAVREPORTS_0003
*  Author Name    : Prajakta                                                                                          
*  Date           : 20 Nov 2013
					Issue Resolved: 0001349
**********************************************************************************************************/
void CISpyReportsDlg::PopulateList()
{
	try
	{
		// get a reference to the contacts list
		const ContactList& contacts = m_objReportsDB.GetContacts();

		// iterate over all contacts & add them to the list
		POSITION pos = contacts.GetHeadPosition();
		int iCurrentItem = m_lstListView.GetItemCount();
		while (pos != NULL)
		{
			const CIspyList contact = contacts.GetNext(pos);

			//Add here the contact one by one
			m_objReportsDBToSave.AddContact(contact, true);


			CString csNewEntry = contact.GetFirstEntry();
			CTime objNewEntryDTime;
			if (!GetDateTimeFromString(csNewEntry, objNewEntryDTime))
			{
				continue;
			}

			int iIndex = m_lstListView.GetItemCount();
			for (; iIndex > 0; iIndex--)
			{
				CString csItemEntry = m_lstListView.GetItemText(iIndex - 1, 0);
				CTime objEntryCompare;
				if (!GetDateTimeFromString(csItemEntry, objEntryCompare))
				{
					continue;
				}

				if (objNewEntryDTime >= objEntryCompare)
				{
					break;
				}
			}

			LVITEM lvItem = { 0 };

			// Insert the first item
			lvItem.mask = LVIF_TEXT;
			lvItem.iItem = iIndex;
			lvItem.iSubItem = 0;
			lvItem.pszText = (LPTSTR)(LPCTSTR)csNewEntry;
			m_lstListView.InsertItem(&lvItem);

			m_lstListView.SetItemText(iIndex, 1, contact.GetSecondEntry());
			m_lstListView.SetItemText(iIndex, 2, contact.GetThirdEntry());
			m_lstListView.SetItemText(iIndex, 3, contact.GetForthEntry());
			m_lstListView.SetItemText(iIndex, 4, contact.GetFifthEntry());
			m_lstListView.SetCheck(iIndex, TRUE);
			//iCurrentItem++; //This is to append in list not prepend
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExReportsDlg::PopulateList", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                
*  Function Name  :	OnSetCursor                                                     
*  Description    :	The framework calls this member function if mouse input is not captured and the mouse causes cursor	
                    movement within the CWnd object. 
*  Author Name    : Vilas & Prajakta                                                                                      
*  Date           : 16 Nov 2013
**********************************************************************************************************/
BOOL CISpyReportsDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if( 
		iCtrlID == IDC_BTN_BACK ||
		iCtrlID == IDC_BTN_DELETE)
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
*  Function Name  :	OnBnClickedCheckSelectall                                                     
*  Description    :	On selection of  'Select All'  button, all checkboxes with corresponding entries on UI will be marked 
*  SR.NO		  : 
*  Author Name    : Prajakta                                                                                          
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyReportsDlg::OnBnClickedCheckSelectall()
{
	
	int iChk = m_chkSelectAll.GetCheck();
	
	for(int i = 0; i < m_lstListView.GetItemCount(); i++)
	{
		m_lstListView.SetCheck(i, iChk);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	ShowHideControls                                                     
*  Description    :	Handles controls to hide/show on UI
*  SR.NO		  : 
*  Author Name    : Prajakta                                                                                          
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyReportsDlg::ShowHideControls(bool bEnable)
{
	m_chkSelectAll.SetCheck(bEnable);
}

/**********************************************************************************************************                     
*  Function Name  :	SendReportsOperation2Service                                                     
*  Description    :	Sends data to service to save the remaining entries in db file after selected entries are deleted succesfully 
*  SR.NO		  : 
*  Author Name    : Prajakta                                                                                          
*  Date           : 20 Nov 2013
**********************************************************************************************************/
bool CISpyReportsDlg::SendReportsOperation2Service(DWORD dwType, CString csDateTime, CString csScanType, CString csFilePath, bool bReportsWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	
	szPipeData.iMessageInfo = dwType;
	szPipeData.dwValue = 0x05;
	wcscpy_s(szPipeData.szFirstParam, csDateTime);
	wcscpy_s(szPipeData.szSecondParam, csScanType);
	wcscpy_s(szPipeData.szThirdParam, csFilePath);

	//CISpyCommunicator objCom(SERVICE_SERVER, true);
	if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CGenXReportsDlg : SendReportsOperation2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bReportsWait)
	{
		if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CGenXReportsDlg : SendReportsOperation2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
		if (szPipeData.dwValue != 1)
		{
			return false;
		}

	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	SendReportsData2Service                                                     
*  Description    :	Sends data to service to reload the existing db file before performing deletion of selected entries
*  SR.NO		  : 
*  Author Name    : Prajakta                                                                                          
*  Date           : 20 Nov 2013
**********************************************************************************************************/
bool CISpyReportsDlg::SendReportsData2Service(DWORD dwMessageInfo, DWORD dwType, CString csEntry, bool bReportsWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	szPipeData.iMessageInfo = static_cast<int>(dwMessageInfo);
	szPipeData.dwValue = dwType;
	_tcscpy(szPipeData.szFirstParam , csEntry);
	//CISpyCommunicator objCom(SERVICE_SERVER, true);
	if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CGenXReportsDlg::SendReportsData2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bReportsWait)
	{
		if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CGenXReportsDlg::SendReportsData2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}
		if (szPipeData.dwValue != 1)
		{
			return false;
		}
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	StopReportOperation                                                     
*  Description    :	Terminates the thread to stop delete operation if user selects home/close button on UI 
*  SR.NO		  : 
*  Author Name    : Prajakta                                                                                          
*  Date           : 20 Nov 2013
**********************************************************************************************************/
bool CISpyReportsDlg::StopReportOperation()
{
	try
	{
		if (m_bReportThreadStart == false)
		{
			return true;
		}

		//Ram, Issue Resolved: 0001194
		//Slight sleep is required if user stops immediately.
		Sleep(500);

		//Varada Ikhar, Date: 24/04/2015, Implementing pause-resume if user clicks on 'close' button.
		//Issue : In reports, if messge box for 'No entries to delete' is present and tried to exit from tray, debug assertion fail error occur.
		if (m_hThread_Report != NULL && m_bDeleteEntriesFinished != true)
		{
			m_bReportStop = true;
			if (::SuspendThread(m_hThread_Report) == -1)
			{
				AddLogEntry(L"### Failed to pause reports delete operation.", 0, 0, true, SECONDLEVEL);
			}
			AddLogEntry(L">>> Reports deletion operation Paused.", 0, 0, true, ZEROLEVEL);
		}

		CString csMsgoptinprogress = L"";
		csMsgoptinprogress.Format(L"%s\n%s", theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_REPORT_PROG_ONE"), theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_REPORT_PROG_TWO"));

		m_bIsPopUpDisplayed = true;
		int iRet = MessageBox(csMsgoptinprogress, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_YESNO | MB_ICONQUESTION);
		m_bIsPopUpDisplayed = false;

		if (iRet == IDNO)
		{
			//Varada Ikhar, Date: 24/04/2015, Implementing pause-resume if user clicks on 'close' button.
			//Issue : In reports, if messge box for 'No entries to delete' is present and tried to exit from tray, debug assertion fail error occur.
			if (m_hThread_Report != NULL && m_bDeleteEntriesFinished != true)
			{
				m_bReportStop = false;
				if (::ResumeThread(m_hThread_Report))
				{
					AddLogEntry(L"### Failed to resume reports delete operation.", 0, 0, true, SECONDLEVEL);
				}
				AddLogEntry(L">>> Reports deletion operation Paused.", 0, 0, true, ZEROLEVEL);
			}

			return false;
		}
		else if (iRet == IDYES && (m_bReportClose || m_bReportHome))
		{
			//Neha Gharge 3/3/2015 
			//Issue : if user click on close button not take any action YES/NO, UI get hanged or not refresh
			if (m_bReportClose == true)
			{
				theApp.m_bOnCloseFromMainUI = true;
			}
			m_bReportStop = true;

			//Varada Ikhar, Date: 24/04/2015
			//Issue : 0000204 : Issue with Reports.If I select "Delete" option in reports and click on any scan,it gives pop-up "entries deleted successfully" but the entries are still present in reports.(It is happening if you have more entries in reports).
			//1.In reports if you have many entries click on "delete". 2.Go to any scan option(I selected quick scan) 3.Start scan.
			//4.It gives pop - up "Entries Deleted Successfully." 5.Click OK. 6.Go in Reports. 7.Entries are still present in Reports.
			if (!SendReportsOperation2Service(SAVE_REPORTS_ENTRIES, L"", L"", L"", true))
			{
				AddLogEntry(L"### Failed to send SAVE_REPORTS_ENTRIES in CNextGenExReportsDlg::StopReportOperation", 0, 0, true, SECONDLEVEL);
			}
		}
		else if (iRet == IDYES)//Ram, Issue Resolved: 0001194
		{
			if (!SendReportsOperation2Service(SAVE_REPORTS_ENTRIES, L"", L"", L"", true))
			{
				AddLogEntry(L"### Failed to send SAVE_REPORTS_ENTRIES in CNextGenExReportsDlg::StopReportOperation", 0, 0, true, SECONDLEVEL);
			}
		}

		//	m_chkSelectAll.SetCheck(true);
		//	m_btnDelete.EnableWindow(true);

		//Ram: Issue resolved: 0001215
		m_bReportEnableDisable = false;
		m_bReportClose = false;
		m_bReportHome = false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CNextGenExReportsDlg::StopReportOperation.", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedReports                                                     
*  Description    :	Shows all control of report dialog after clicking report button from tab controls 
*  SR.NO		  : 
*  Author Name    : Prajakta                                                                                          
*  Date           : 20 Nov 2013
**********************************************************************************************************/
void CISpyReportsDlg::OnBnClickedReports()
{
	ShowControls4Reports();
}

/**********************************************************************************************************                     
*  Function Name  :	OnPaint                                                     
*  Description    :	The framework calls this member function when Windows or an application 
					makes a request to repaint a portion of an application's window.
*  Author Name    : Nitin 
*  SR_NO		  :
*  Date           : 13 Jun 2014
**********************************************************************************************************/
void CISpyReportsDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CJpegDialog::Draw();
	CJpegDialog::OnPaint();
}

/***********************************************************************************************                    
*  Function Name  : PreTranslateMessage                                                     
*  Description    : Translate window messages before they are dispatched to the TranslateMessage and DispatchMessage Windows functions
*  Author Name    : Windows 
*  SR_NO		  :
*  Date           : 19-Dec-2014
*************************************************************************************************/
BOOL CISpyReportsDlg::PreTranslateMessage(MSG* pMsg)
{
	/* Issue No: 28 While deleting reports if we click spacebar, more than one dialog appears.
		Resolved by : Nitin K Date: 19-Dec-2014
	*/
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_SPACE  || pMsg->wParam == VK_RETURN)
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

/**********************************************************************************************************
*  Function Name  :	GetSortedFileNames
*  Description    :	Function to get file name in sorted manner
*  Author Name    : Ramkrushna Shelke
*  Date           : 6 Jan 2016
*  SR_NO		  :
**********************************************************************************************************/
bool CISpyReportsDlg::GetSortedFileNames(vector<int> &vec)
{
	try
	{
		CString csReprtsFolderPath;
		if (!GetReportsFolderPath(csReprtsFolderPath))
		{
			return false;
		}

		GetFileDigits(csReprtsFolderPath, vec);
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
void CISpyReportsDlg::GetFileDigits(LPCTSTR pstr, vector<int> &vec)
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
*  Function Name  :	GetMaximumDigitFromFiles
*  Description    :	Function to get Maximum number from files
*  Author Name    : Ramkrushna Shelke
*  Date           : 6 Jan 2016
*  SR_NO		  :
**********************************************************************************************************/
void CISpyReportsDlg::LoadRemainingEntries()
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

		CString csFolderPath;
		if (!GetReportsFolderPath(csFolderPath))
		{
			return;
		}

		//Load the file contents in memory and populate in list control
		for (vector<int>::reverse_iterator it = vecNumList.rbegin(); it != vecNumList.rend(); ++it)
		{

			CString csFilePath;
			csFilePath.Format(L"%s\\WRDWIZREPORTS_%d.DB", csFolderPath, *it);
			if (PathFileExists(csFilePath))
			{
				LoadDataContentFromFile(csFilePath);
				PopulateList();
			}
		}

		//Save the DB file here
		SaveDBFile();

		//Remove the files which are already loaded and merged.
		for (vector<int>::reverse_iterator it = vecNumList.rbegin(); it != vecNumList.rend(); ++it)
		{
			CString csFilePath;
			csFilePath.Format(L"%s\\WRDWIZREPORTS_%d.DB", csFolderPath, *it);
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
*  Description    :	According to type of module DB get save.
*  Author Name    : Neha Gharge
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0003
**********************************************************************************************************/
bool CISpyReportsDlg::SaveDBFile()
{
	try
	{
		CString csFilePath;
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_REPORT_GETMODULEPATH_ERR"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR | MB_OK);
			m_bIsPopUpDisplayed = false;
			return false;
		}
		csFilePath = szModulePath;
		csFilePath.Append(L"\\WRDWIZREPORTS.DB");

		int iFileVersion = 1;
		CFile wFile(csFilePath, CFile::modeCreate | CFile::modeWrite);

		CArchive arStore(&wFile, CArchive::store);

		m_objReportsDBToSave.SetFileVersion(iFileVersion);
		m_objReportsDBToSave.Serialize(arStore);

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

/**********************************************************************************************************
*  Function Name  :	GetReportsFolderPath
*  Description    :	Function to get reports folder path
*  Author Name    : Ramkrushna Shelke
*  Date           : 06 Jan 2016
*  SR_NO		  : 
**********************************************************************************************************/
bool CISpyReportsDlg::GetReportsFolderPath(CString &csFolderPath)
{
	TCHAR szModulePath[MAX_PATH] = { 0 };
	try
	{
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			return false;
		}
		csFolderPath.Format(L"%s\\%s", szModulePath, L"REPORTS");
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExReportsDlg::GetReportsFolderPath", 0, 0, true, SECONDLEVEL);
		return false;
	}
	
	return false;
}

/**********************************************************************************************************
*  Function Name  :	LoadDataContentFromFile
*  Description    :	Loads WRDWIZRECOVERENTRIES.DB file content into serialization object.
*  SR.NO		  :
*  Author Name    : Neha Gharge
*  Date           : 20 Nov 2013
**********************************************************************************************************/
bool CISpyReportsDlg::LoadDataContentFromFile(CString csPathName)
{
	AddLogEntry(L">>> Loading Reports db", 0, 0, true, FIRSTLEVEL);

	m_objReportsDB.RemoveAll();

	if (!PathFileExists(csPathName))
		return false;

	CFile rFile(csPathName, CFile::modeRead);

	// Create a loading archive
	CArchive arLoad(&rFile, CArchive::load);
	m_objReportsDB.Serialize(arLoad);

	// Close the loading archive
	arLoad.Close();
	rFile.Close();

	return true;
	//PopulateList();
}

/**********************************************************************************************************
*  Function Name  :	GetDateTimeFromString
*  Description    :	Function to get fill CTime object filled from string.
*  SR.NO		  :
*  Author Name    : Ram Shelke
*  Date           : 09 Mar 2016
**********************************************************************************************************/
bool CISpyReportsDlg::GetDateTimeFromString(CString csItemEntry, CTime &objDateTime)
{
	bool bReturn = false;
	try
	{
		if (csItemEntry.GetLength() == 0)
		{
			return bReturn;
		}

		CString csTime = csItemEntry.Right(8);

		csItemEntry.Truncate(10);
		csItemEntry.Trim();
		CString csDBTmpDate = L"";
		int iPos1 = 0; int j = 0;
		int iDBArr[6] = { 0 };

		while (csItemEntry.Trim().GetLength() != 0)
		{
			if (iPos1 == -1)
				break;

			csDBTmpDate = csItemEntry.Tokenize(_T("/"), iPos1);

			if (csDBTmpDate.GetLength() == 0)
			{
				continue;
			}

			iDBArr[j] = _wtoi(csDBTmpDate);
			j++;

			if (j > 3)
			{
				break;
			}
		}

		if (iDBArr[2] == 0 || iDBArr[0] == 0 || iDBArr[1] == 0)
			false;

		iPos1 = 0;
		while (csTime.Trim().GetLength() != 0)
		{
			if (iPos1 == -1)
				break;

			csDBTmpDate = csTime.Tokenize(_T(":"), iPos1);

			if (csDBTmpDate.GetLength() == 0)
			{
				continue;
			}

			iDBArr[j] = _wtoi(csDBTmpDate);
			j++;

			if (j > 5)
			{
				break;
			}
		}

		CTime objDBDate(iDBArr[2], iDBArr[0], iDBArr[1], iDBArr[3], iDBArr[4], iDBArr[5]);
		objDateTime = objDBDate;
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExReportsDlg::GetDateTimeFromString", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}
