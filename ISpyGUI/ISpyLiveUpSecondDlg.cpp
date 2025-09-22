// ISpyLiveUpSecondDlg.cpp : implementation file
//
/*******************************************************************************************
*  Program Name: ISpyLiveUpSecondDlg.cpp                                                                                                  
*  Description:  This is Second Live Update UI which shows status,percentage,file size,internet
				 speed,Remaining time of downloading.
*  Author Name:  1)Ramkrushna
*				 2)Neha Gharge
*
*  Date Of Creation: 15-0ct 2013                                                                                              
*  Version No:    1.0.0.2                                                                                                        *
*                                                                                                                                      *
*  Special Logic Used:                                                                                                                            *
*                                                                                                                                      *
*  Modification Log:                                                                                               
*  1. Modified xyz function in main        Date modified         CSR NO    *
*  Modification By: Rajil Yadav                                                                                                                                    *
*                   For WardWiz UI                                                                                                                   *
*********************************************************************************************/ 

/*********************************************************************************************
//HEADER FILES
**********************************************************************************************/
#include "stdafx.h"
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"
#include "ISpyUpdatesDlg.h"
#include "ISpyLiveUpSecondDlg.h"
#include "EnumProcess.h"

/*********************************************************************************************
//GLOBAL VARIABLE DECLARATION
**********************************************************************************************/
const TCHAR szProductName[MAX_PATH] = L"WardWiz Antivirus";
const int TIMER_SETPERCENTAGESTATUS = 1000;
#define ImageTimerID  250
DWORD g_iTotalFileDownloadBytes = 0;
static BOOL StartFlag=0;
CString					g_csPreviousListControlStatus = L"";
HANDLE hFile;

/*********************************************************************************************
//THRAED DECLARATION
**********************************************************************************************/
//DWORD WINAPI StartDownLoadThread(LPVOID lpvThreadParam);

CISpyGUIDlg* g_TabCtrlupdateWindHandle = NULL;

IMPLEMENT_DYNAMIC(CISpyLiveUpSecondDlg, CDialog)

/***********************************************************************************************                    
*  Function Name  :           CISpyLiveUpSecondDlg                                                     
*  Description    :           C'tor
*  Author Name    :           RamKrishna  
*  SR_NO		  :
*  Date           :           15-Oct-2013
*************************************************************************************************/
CISpyLiveUpSecondDlg::CISpyLiveUpSecondDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CISpyLiveUpSecondDlg::IDD, pParent)
	, m_hEvtFinished(NULL)
	, m_iFileSize(0)
	, m_iTotalFileSize(0)
	, m_bDownLoadInProgress(false)
	, m_iCurrentItemCount(0)
	//, m_hThread(NULL)
	, m_bisDateTime(false)
	, m_iCurrentDownloadBytes(0)
	, m_dwCompletedBytes(0)
	, m_dwPercentage(0)
	, m_dwstatusMsg(-1)
	, m_csListControlStatus("")
	, m_dwTotalFileNo(0)
	, m_dwDownloadOrUpdateFileNo(0)
	, m_iRowCount(0)
	, m_bManualStop(false)
	,m_bISalreadyDownloadAvailable(true)
	,m_bIsCheckingForUpdateEntryExistInList(true)
	, m_bCloseUpdate(false)
	, m_isDownloading(false)
	, m_bIsPopUpDisplayed(false)
{
	memset(m_szAppDataFolder, 0 , MAX_PATH);
}

/***********************************************************************************************                    
*  Function Name  : CISpyLiveUpSecondDlg                                                     
*  Description    :	Destructor
*  Author Name    : RamKrishna  
*  SR_NO		  :
*  Date           : 15-Oct-2013
*************************************************************************************************/

CISpyLiveUpSecondDlg::~CISpyLiveUpSecondDlg()
{
	if(m_hEvtFinished != NULL)
	{
		CloseHandle(m_hEvtFinished);
		m_hEvtFinished = NULL;
	}
	//if(m_hThread != NULL)
	//{
	//	::SuspendThread(m_hThread);
	//	::TerminateThread(m_hThread, 0);
	//	m_hThread = NULL;
	//}
	if(m_vUrlLists.size() > 0)
	{
		m_vUrlLists.clear();
	}
	if(m_vFileSizeMismatch.size() > 0)
	{
		m_vFileSizeMismatch.clear();
	}
}

/***********************************************************************************************                    
*  Function Name  :  DoDataExchange                                                     
*  Description    :  Called by the framework to exchange and validate dialog data.
*  Author Name    :  RamKrishna, Neha Gharge
*  SR_NO		  :
*  Date           :  18-Nov-2013
*************************************************************************************************/

void CISpyLiveUpSecondDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_DOWNLOAD, m_Picture);
	DDX_Control(pDX, IDC_FILESDOWNLOAD_PROGRESS, m_prgFileDownloadStatus);
	DDX_Control(pDX, IDC_LIST_DOWNLOADSTATUS, m_lstFileDownloadStatus);
	DDX_Control(pDX, IDC_STATIC_TOTALFILESIZE, m_stTotalFileSize);
	DDX_Control(pDX, IDC_STATIC_TOTAL_DOWNLOAD, m_stTotalDownload);
	DDX_Control(pDX, IDC_STATIC_SPEED, m_stDownloadSpeed);
	DDX_Control(pDX, IDC_STATIC_REMAINING_TIME, m_stRemainingTime);
	DDX_Control(pDX, IDC_STATIC_TIMEREMAININGHEADER, m_stTimeRemaining);
	DDX_Control(pDX, IDC_STATIC_SPEEDHEADER, m_stSpeedHeader);
	DDX_Control(pDX, IDC_BUTTON_DWCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BUTTON_PAUSE, m_btnPauseResume);
	//DDX_Control(pDX, IDC_BACK_BUTTON, m_btnBack);
	DDX_Control(pDX, IDC_SEC_BUTTON_HELP, m_btnSecHelp);
	DDX_Control(pDX, IDC_STAT_SEC_HEADER, m_stSecHeaderPic);
	//DDX_Control(pDX, IDC_STATIC_NORMALIMAGE, m_stNormalImage);
	DDX_Control(pDX, IDC_STATIC_UPDATES_SECOND_HEADER, m_stUpdatesSecondHeader);
	DDX_Control(pDX, IDC_STATIC_SECOND_HEADER, m_stUpdatesSubHeader);
	DDX_Control(pDX, IDC_STATIC_DOWNLOADED, m_stDownloaded);
	DDX_Control(pDX, IDC_STATIC_TOTAL_SIZE, m_stTotalSize);
	DDX_Control(pDX, IDC_BUTTON_RESUME, m_btnResume);
}

/***********************************************************************************************                    
*  Function Name  :  MAPPING MESSAGES                                                    
*  Description    :  Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    :  Neha Gharge  
*  SR_NO		  :
*  Date           :  18-Nov-2013
*************************************************************************************************/

BEGIN_MESSAGE_MAP(CISpyLiveUpSecondDlg, CJpegDialog)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CISpyLiveUpSecondDlg::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_DWCANCEL, &CISpyLiveUpSecondDlg::OnBnClickedButtonDwcancel)
	ON_WM_NCHITTEST()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_SEC_BUTTON_STOP, &CISpyLiveUpSecondDlg::OnBnClickedSecButtonStop)
	ON_BN_CLICKED(IDC_BUTTON_RESUME, &CISpyLiveUpSecondDlg::OnBnClickedButtonResume)
	ON_WM_PAINT()
END_MESSAGE_MAP()

/***************************************************************************************************                    
*  Function Name  : OnInitDialog                                                     
*  Description    : Windows calls the OnInitDialog function through the standard global 
					dialog-box procedure common to all Microsoft Foundation Class Library
					dialog boxes
*  Author Name    : Neha Gharge  
*  SR_NO		  :
*  Date           : 18-Nov-2013
****************************************************************************************************/

BOOL CISpyLiveUpSecondDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window
	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_INNER_DIALOG), _T("JPG")))
	{
		m_bIsPopUpDisplayed = true;
		::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
		m_bIsPopUpDisplayed = false;
	}

	Draw();

	m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));

	CRect rect1;
	this->GetClientRect(rect1);
	SetWindowPos(NULL, 0, 0, rect1.Width()-5, rect1.Height() - 5, SWP_SHOWWINDOW);

	//Set MainDlg Ptr
	g_TabCtrlupdateWindHandle =(CISpyGUIDlg*)AfxGetMainWnd();	
	
	m_lstFileDownloadStatus.SetFont(&theApp.m_fontWWTextNormal);
	m_lstFileDownloadStatus.InsertColumn(0,theApp.m_objwardwizLangManager.GetString( L"IDC_STATIC_ITEMS"), LVCFMT_LEFT, 450);
	m_lstFileDownloadStatus.InsertColumn(1,theApp.m_objwardwizLangManager.GetString( L"IDC_STATIC_STATUS"), LVCFMT_LEFT, 200);
	ListView_SetExtendedListViewStyle (m_lstFileDownloadStatus.m_hWnd, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVM_GETIMAGELIST | LVM_SETIMAGELIST);
	CHeaderCtrl* pHeaderCtrl = m_lstFileDownloadStatus.GetHeaderCtrl();
	pHeaderCtrl->SetFont(&theApp.m_fontWWTextNormal);
	
	m_lstFileDownloadStatus.SetItem(0,0,LVIF_IMAGE,0,1,0,0,0);
	InitImageList();

	ZeroMemory(m_szAllUserPath, sizeof(m_szAllUserPath) );
	GetEnvironmentVariable(L"ALLUSERSPROFILE", m_szAllUserPath, 511);

/*	if (m_Picture.Load(MAKEINTRESOURCE(IDR_GIF_DONWLOAD),_T("GIF")))
		if(!m_Picture.IsPlaying())
		{
			m_Picture.Draw();
		}
	m_Picture.SetWindowPos(&wndTop,rect1.left+44,12,65,50, SWP_NOREDRAW);*/


	//m_stUpdatesSecondHeader.SetTextAlign(TA_LEFT);
	//m_stUpdatesSecondHeader.SetColor(RGB(230,232,238));
	//m_stUpdatesSecondHeader.SetGradientColor(RGB(230,232,238));
	//m_stUpdatesSecondHeader.SetVerticalGradient(1);
	//m_stUpdatesSecondHeader.SetWindowPos(&wndTop,rect1.left +10,15,270,20,SWP_NOREDRAW);
	//m_stUpdatesSecondHeader.SetFont(&theApp.m_fontWWTextSmallTitle);

	m_stUpdatesSecondHeader.SetWindowPos(&wndTop,rect1.left +20,07,580,31,SWP_NOREDRAW);	
	m_stUpdatesSecondHeader.SetTextColor(RGB(24,24,24));
	m_stUpdatesSecondHeader.SetFont(&theApp.m_fontWWTextTitle);

	
	//m_stUpdatesSubHeader.SetTextAlign(TA_LEFT);
	//m_stUpdatesSubHeader.SetColor(RGB(230,232,238));
	//m_stUpdatesSubHeader.SetGradientColor(RGB(230,232,238));
	//m_stUpdatesSubHeader.SetVerticalGradient(1);
	//m_stUpdatesSubHeader.SetWindowPos(&wndTop,rect1.left +8,35,270,20,SWP_NOREDRAW);
	//m_stUpdatesSubHeader.SetFont(&theApp.m_fontWWTextSubTitleDescription);


	m_stUpdatesSubHeader.SetTextColor(RGB(24,24,24));
	m_stUpdatesSubHeader.SetFont(&theApp.m_fontWWTextSubTitleDescription);
	m_stUpdatesSubHeader.SetWindowPos(&wndTop,rect1.left + 20,38,400,15,SWP_NOREDRAW);

	if(theApp.m_dwOSType == WINOS_WIN8 ||theApp.m_dwOSType == WINOS_WIN8_1)
	{
		m_stUpdatesSecondHeader.SetWindowPos(&wndTop,rect1.left +20,12,580,31,SWP_NOREDRAW);	
		m_stUpdatesSubHeader.SetWindowPos(&wndTop,rect1.left + 20,38,400,15,SWP_NOREDRAW);
	}
	m_bmpSecHeader = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_COM_HEADER));
	m_stSecHeaderPic.SetBitmap(m_bmpSecHeader);
	m_stSecHeaderPic.SetWindowPos(&wndTop,rect1.left+6,10,586,45, SWP_NOREDRAW);
		
	m_lstFileDownloadStatus.SetWindowPos(&wndTop,rect1.left+6,53,587,219, SWP_NOREDRAW);

	m_prgFileDownloadStatus.SetWindowPos(&wndTop,rect1.left+6,360,430,20, SWP_NOREDRAW);
	//m_prgFileDownloadStatus

	m_stTotalDownload.SetWindowPos(&wndTop,rect1.left+105,300,220,20, SWP_NOREDRAW);
	m_stTotalDownload.SetFont(&theApp.m_fontWWTextNormal);
	m_stTotalDownload.SetBkColor(RGB(70,70,70));
	m_stTotalDownload.SetTextColor(WHITE);

	m_stDownloaded.SetWindowPos(&wndTop,rect1.left+6,300,240,20, SWP_NOREDRAW);
	m_stDownloaded.SetFont(&theApp.m_fontWWTextNormal);
	m_stDownloaded.SetBkColor(RGB(70,70,70));
	m_stDownloaded.SetTextColor(WHITE);

	/*	ISSUE NO - 182 NAME - NITIN K. TIME - 30th May 2014 */
	/*	ISSUE NO - 459 NAME - NITIN K. TIME - 30th May 2014 */
	m_stTotalSize.SetWindowPos(&wndTop,rect1.left+6,320,75,20, SWP_NOREDRAW);
	m_stTotalSize.SetFont(&theApp.m_fontWWTextNormal);
	m_stTotalSize.SetBkColor(RGB(70,70,70));
	m_stTotalSize.SetTextColor(WHITE);


	m_stTotalFileSize.SetWindowPos(&wndTop,rect1.left+105,320,172,20, SWP_NOREDRAW);
	m_stTotalFileSize.SetFont(&theApp.m_fontWWTextNormal);
	m_stTotalFileSize.SetBkColor(RGB(70,70,70));
	m_stTotalFileSize.SetTextColor(WHITE);
	m_stTotalFileSize.SetWindowTextW(L"0 KB");
	m_stTotalFileSize.ShowWindow(SW_SHOW);

	m_stTimeRemaining.SetWindowPos(&wndTop,rect1.left+400,300,100,20, SWP_NOREDRAW);
	m_stTimeRemaining.SetFont(&theApp.m_fontWWTextNormal);
	m_stTimeRemaining.SetBkColor(RGB(70,70,70));
	m_stTimeRemaining.SetTextColor(WHITE);

	m_stRemainingTime.SetWindowPos(&wndTop,rect1.left+532,300,90,20, SWP_NOREDRAW);
	m_stRemainingTime.SetFont(&theApp.m_fontWWTextNormal);
	m_stRemainingTime.SetBkColor(RGB(70,70,70));
	m_stRemainingTime.SetTextColor(WHITE);

	m_stSpeedHeader.SetWindowPos(&wndTop,rect1.left+400,320,100,40, SWP_NOREDRAW);
	m_stSpeedHeader.SetFont(&theApp.m_fontWWTextNormal);
	m_stSpeedHeader.SetBkColor(RGB(70,70,70));
	m_stSpeedHeader.SetTextColor(WHITE);

	m_stDownloadSpeed.SetWindowPos(&wndTop,rect1.left+532,320,90,20, SWP_NOREDRAW);
	m_stDownloadSpeed.SetFont(&theApp.m_fontWWTextNormal);
	m_stDownloadSpeed.SetBkColor(RGB(70,70,70));
	m_stDownloadSpeed.SetTextColor(WHITE);

	//ISSUE NO:- 183 RY Date :- 21/5/2014
	m_btnPauseResume.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnPauseResume.SetWindowPos(&wndTop,rect1.left+468,360,57,21, SWP_NOREDRAW);
	m_btnPauseResume.SetTextColorA(BLACK,1,1);
	m_btnPauseResume.SetFont(&theApp.m_fontWWTextNormal);
	m_btnPauseResume.SetWindowText(theApp.m_objwardwizLangManager.GetString( L"IDS_STATIC_START"));

	m_btnResume.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnResume.SetWindowPos(&wndTop,rect1.left+468,360,57,21, SWP_NOREDRAW);
	m_btnResume.SetTextColorA(BLACK,1,1);
	m_btnResume.SetFont(&theApp.m_fontWWTextNormal);
	m_btnResume.SetWindowText(theApp.m_objwardwizLangManager.GetString( L"IDS_BUTTON_RESUME"));
	m_btnResume.ShowWindow(SW_HIDE);

	m_btnCancel.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnCancel.SetWindowPos(&wndTop,rect1.left+533,360,57,21,SWP_NOREDRAW);
	m_btnCancel.SetTextColorA(RGB(0,0,0),1,1);
	m_btnCancel.SetTextColor(BLACK);
	m_btnCancel.SetFont(&theApp.m_fontWWTextNormal);
	//Issue No:-253 Rajil Yadav 22/05/2014
	m_btnSecHelp.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnSecHelp.SetWindowPos(&wndTop,rect1.left+533,360,57,21,SWP_NOREDRAW);
	m_btnSecHelp.ShowWindow(SW_HIDE);
	m_btnSecHelp.SetTextColorA(RGB(0,0,0),1,1);
	m_btnSecHelp.SetFont(&theApp.m_fontWWTextNormal);

	if(!GetApplicationDataFolder(m_szAppDataFolder))
	{
		AddLogEntry(L"### Failed to GetApplicationDataFolder", 0, 0, true, SECONDLEVEL);
	}


	//CWnd* pWnd = GetParent()->GetDlgItem(ID_WIZNEXT); 
	//pWnd->EnableWindow(FALSE);
	//pWnd->SetWindowText(L"Pause"); 

	m_bDownLoadInProgress = true;
	m_prgFileDownloadStatus.SetPos(0);
	m_prgFileDownloadStatus.SetRange(0, 100);
	m_prgFileDownloadStatus.SetBarColor(RGB(171,238,0));
	m_bisDateTime = false;
	m_isDownloading = false;

	RefreshStrings();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/***************************************************************************************************                    
*  Function Name  : StartDownloading                                                     
*  Description    : A new thread creates to download the files from server 
*  Author Name    : Neha Gharge,Ram Krushna  
*  SR_NO		  :
*  Date           : 18-Nov-2013
****************************************************************************************************/

bool CISpyLiveUpSecondDlg::StartDownloading()
{
	bool bReturn = false;

	try
	{
		/*if(!m_Picture.IsPlaying())
		{
			m_Picture.Draw();
		}*/
		m_isDownloading = true;
		m_btnSecHelp.ShowWindow(FALSE);
		m_btnCancel.ShowWindow(true);
		m_btnCancel.EnableWindow(FALSE);

 // issue number :-  123 resolved by lait The "Checking for update" is taking a lot of time to come.

		g_csPreviousListControlStatus = L"";
		m_bIsCheckingForUpdateEntryExistInList= true;
		/* Issue Nuumber - 107, During updates if we cancel the updates and restart it,
		the "pause" button does not gets enabled.
		Divya S. 21/8/2014
		*/
		m_btnPauseResume.EnableWindow(FALSE);
		m_btnPauseResume.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PAUSE"));
		ResetControls();
		//m_objWinHttpManager.StartCurrentDownload();
		m_tsStartTime = CTime::GetCurrentTime();

		//if(!MakeDownloadUrlList())
		//{
		//	AddLogEntry(L"### Failed to MakeDownloadUrlList", 0, 0, true, SECONDLEVEL);
		//}

		DWORD dwThreadId = 0;
		m_hEvtFinished = CreateEvent(NULL, TRUE, FALSE, NULL);
		//m_hThread = CreateThread(NULL, 0, StartDownLoadThread, (LPVOID) this, 0, &dwThreadId);

		SetTimer(TIMER_SETPERCENTAGESTATUS, 500, NULL);
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CLiveUpSecondPage::StartDownloading", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************                    
*  Function Name  : StartDownLoadThread                                                     
*  Description    : This thread will show the status and percentage from ALU service to UI
*  Author Name    : Neha Gharge,Ram Krushna 
*  SR_NO		  :
*  Date           : 18-Nov-2013
*  Modified date  :	10- Jul -2014 (Auto Live Update)
****************************************************************************************************/
//DWORD WINAPI StartDownLoadThread(LPVOID lpvThreadParam)
bool CISpyLiveUpSecondDlg::UpDateDowloadStatus(LPISPY_PIPE_DATA lpSpyData)
{
	DWORD dwFileSizeInKB = 0, dwMessageType;
	CString csTotalFileSize = L"";
	CString	csListControlStatus = L"";

	try
	{
		//CISpyLiveUpSecondDlg *pThis = (CISpyLiveUpSecondDlg*)lpvThreadParam;
		if (!lpSpyData)
			return false;

		//Enabled here controls
		//EnableControls(TRUE);

		//ITIN_MEMMAP_DATA iTinMemMap = { 0 };
		//while(true)
		{
			//if(g_TabCtrlupdateWindHandle != NULL)
			//{
			//	g_TabCtrlupdateWindHandle->m_pUpdate->m_objIPCALUpdateClient.GetServerMemoryMappedFileData(&iTinMemMap, sizeof(iTinMemMap));

			//}
			//issue number 123 resolved by lalit  "Checking for update" is taking a lot of time to come
			if (m_bIsCheckingForUpdateEntryExistInList)
			{
				InsertItem(theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_CHECK_FOR_UPDATES"), theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_CHECK_FOR_UPDATES"));
				//EnableControls(TRUE);
			}

			switch (lpSpyData->iMessageInfo)
			{
			case SETTOTALFILESIZE:
				EnableControls(TRUE);
				//Enable here the controls
				m_iTotalFileSize = lpSpyData->dwValue;
				dwFileSizeInKB = static_cast<DWORD>(m_iTotalFileSize / 1024);
				csTotalFileSize.Format(L"%d %s", dwFileSizeInKB, L"KB");
				m_stTotalFileSize.SetWindowText(csTotalFileSize);
				break;

			case SETDOWNLOADPERCENTAGE:
				m_iCurrentDownloadBytes = lpSpyData->dwValue;
				m_dwPercentage = lpSpyData->dwSecondValue;

				break;
			case SETMESSAGE:
				//EnableControls(TRUE);
				m_csListControlStatus.Format(L"%s", lpSpyData->szFirstParam);
				if (m_csListControlStatus.CompareNoCase(L"Downloading files") == 0)
				{
					csListControlStatus.Format(L"%s: %d/%d", theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_DOWNLOADING"), lpSpyData->dwValue, lpSpyData->dwSecondValue);
				}
				else if (m_csListControlStatus.CompareNoCase(L"Updating files") == 0)
				{
					csListControlStatus.Format(L"%s: %d/%d", theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_UPDATING"), lpSpyData->dwValue, lpSpyData->dwSecondValue);
				}

				if (csListControlStatus.GetLength() > 0 && (g_csPreviousListControlStatus != csListControlStatus))
				{
					OutputDebugString(csListControlStatus);
					InsertItem(csListControlStatus, m_csListControlStatus);
				}
				g_csPreviousListControlStatus.SetString(csListControlStatus);

				break;
				// Modified by Neha Gharge 5/5/2015 for new status message for failures and succesful cases.
			case SETUPDATESTATUS:
				dwMessageType = lpSpyData->dwValue;
				switch (dwMessageType)
				{
				case ALUPDATEDSUCCESSFULLY:
					m_dwstatusMsg = ALUPDATEDSUCCESSFULLY;
					break;
				case ALUPDATEFAILED_INTERNETCONNECTION:
					m_dwstatusMsg = ALUPDATEFAILED_INTERNETCONNECTION;
					break;
				case ALUPDATEFAILED_DOWNLOADINIPARSE:
					m_dwstatusMsg = ALUPDATEFAILED_DOWNLOADINIPARSE;
					break;
				case ALUPDATED_UPTODATE:
					m_dwstatusMsg = ALUPDATED_UPTODATE;
					break;
				case ALUPDATEFAILED_DOWNLOADFILE:
					m_dwstatusMsg = ALUPDATEFAILED_DOWNLOADFILE;
					break;
				case ALUPDATEFAILED_EXTRACTFILE:
					m_dwstatusMsg = ALUPDATEFAILED_EXTRACTFILE;
					break;
				case ALUPDATEFAILED_UPDATINGFILE:
					m_dwstatusMsg = ALUPDATEFAILED_UPDATINGFILE;
					break;
				//Issue: If disk space is low in C: drive and if tried to update the product then warning popup appearing as 'failed to update the product. Please check internet connection'
				//Resolved by: Nitin K
				case ALUPDATEFAILED_LOWDISKSPACE:
					m_dwstatusMsg = ALUPDATEFAILED_LOWDISKSPACE;
					break;
				}
				break;


			}
			csListControlStatus.ReleaseBuffer();
			m_csListControlStatus.ReleaseBuffer();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExLiveUpSecondDlg::UpDateDowloadStatus", 0, 0, true, SECONDLEVEL);
	}
	return 1;
}

/***************************************************************************************************                    
*  Function Name	 : ShowUpdateCompleteMessage                                                     
*  Description		 : It shows the message after completing a download
*  Author Name	     : Ram Krushna , Neha Gharge  
*  SR_NO		     :
*  Date				 : 24-Nov-2013
*  Modification Date : 25-Jul-2014
*  Modified Date	 : Neha Gharge 5/5/2015 for new status message for failures and succesful cases. 
****************************************************************************************************/
void CISpyLiveUpSecondDlg::ShowUpdateCompleteMessage()
{
	DWORD dwCount = 0;
	TCHAR szIniFilePath[512] = {0};
	TCHAR szName[512] ={0};

	try
	{
		KillTimer(ImageTimerID);

		int nItem;
		int listItemToBeUpdate = m_lstFileDownloadStatus.GetItemCount();


		CString col2 = m_lstFileDownloadStatus.GetItemText(listItemToBeUpdate - 1, 0);
		CString col3 = m_lstFileDownloadStatus.GetItemText(listItemToBeUpdate - 1, 1);

		m_lstFileDownloadStatus.DeleteItem(listItemToBeUpdate - 1);
		nItem = m_lstFileDownloadStatus.InsertItem(listItemToBeUpdate - 1, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_COMPLETED"), 8);
		m_lstFileDownloadStatus.SetItem(nItem, 0, LVIF_TEXT, col2, 0, 0, 0, NULL);
		m_lstFileDownloadStatus.SetItem(nItem, 1, LVIF_TEXT, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_COMPLETED"), 0, 0, 0, NULL);

		//add by lalit 5-7-2015,it use to highlight Task bar icon when scannin completed message box appeared. 
		::SetForegroundWindow(m_hWnd);

		/* issue:165 Resolved by - lalit 9/2/2014 */
		// Modified by Neha Gharge 5/5/2015 for new status message for failures and succesful cases.
		if (m_bManualStop)
		{
			m_bIsPopUpDisplayed = true;
			//Varada Ikhar, Date 26/02/2015, Issue:.If we update database locally, after completion the pop-up should have punctuation(FullStop)
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_UPDATE_ABORTED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_bIsPopUpDisplayed = false;
		}
		else
		{
			if (m_dwstatusMsg == ALUPDATEDSUCCESSFULLY)
			{
				bool bmessgeBoxResponce = false;

				//Kill the timer here to stop in progress
				KillTimer(ImageTimerID);

				if (!m_bISalreadyDownloadAvailable)
				{
					m_bIsPopUpDisplayed = true;
					//Varada Ikhar, Date 26/02/2015, Issue:.If we update database locally, after completion the pop-up should have punctuation(FullStop)
					bmessgeBoxResponce = (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_UPDATED_SUCCESS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION | MB_OK) == IDOK);
					m_bIsPopUpDisplayed = false;
				}
				else
				{
					m_bIsPopUpDisplayed = true;
					//Varada Ikhar, Date 26/02/2015, Issue:.If we update database locally, after completion the pop-up should have punctuation(FullStop)
					bmessgeBoxResponce = (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_UPTODATE"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION | MB_OK) == IDOK);
					m_bIsPopUpDisplayed = false;
				}

				if (bmessgeBoxResponce)
				{
					swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\WardWiz Antivirus\\ALUDel.ini", m_szAllUserPath);

					if (!PathFileExists(szIniFilePath))
					{
						//AddLogEntry(L"### ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, SECONDLEVEL);
						//d
						AddLogEntry(L"### Function is ShowUpdateCompleteMessage.....ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, SECONDLEVEL);
						dwCount = 0;
					}
					else
					{
						dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szIniFilePath);
					}

					if (dwCount)
					{
						//add by lalit 5-7-2015,it use to highlight Task bar icon when scannin completed message box appeared. 
						::SetForegroundWindow(m_hWnd);

						m_bIsPopUpDisplayed = true;
						if (MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_RESTART_FOR_UPDATE"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDYES)
						{
							m_bIsPopUpDisplayed = false;
							//Write a code to restart computer.
							CEnumProcess enumproc;
							enumproc.RebootSystem(0);
						}
						m_bIsPopUpDisplayed = false;
					}
					//else
					//{
					//	/* If DB is not valid issue Neha Gharge 1/1/2014*/
					//	//Neha Gharge, Date:17-03-2015
					//	//Issue: If only DB is updated in live update, restart now msg does not pop-up.
					//	dwCount = GetPrivateProfileInt(L"DB", L"Count", 0x00, szIniFilePath);
					//	if (dwCount)
					//	{
					//		//Write a code to restart computer. 
					//		// Asking for permission 18/2/2015 Neha Gharge
					//		if(MessageBox(L"Please restart your computer to apply the updates.\n\nDo you want to restart now?",theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONQUESTION|MB_YESNO) == IDYES)
					//		{
					//			CEnumProcess enumproc;
					//			enumproc.RebootSystem(0);
					//		}
					//	}
					//}
				}
			}
			else if (m_dwstatusMsg == ALUPDATED_UPTODATE)
			{
				//add by lalit 5-7-2015,it use to highlight Task bar icon when scannin completed message box appeared. 
				::SetForegroundWindow(m_hWnd);

				m_bIsPopUpDisplayed = true;
				//Varada Ikhar, Date 26/02/2015, Issue:.If we update database locally, after completion the pop-up should have punctuation(FullStop)
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_UPTODATE"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
				m_bIsPopUpDisplayed = false;
			}
			else if (m_dwstatusMsg == ALUPDATEFAILED_INTERNETCONNECTION)
			{
				CWrdWizCustomMsgBox objCustomMsgBox(this);
				if (objCustomMsgBox.m_hWnd == NULL)
				{
					//Need to set m_isDownloading here because if Updates are failed and custom msgbox is displayed then to keep same msg box on top need to this flag as false
					//Added by Nitin K Date 15th July 2015
					m_isDownloading = false;
					//Neha Gharge 13 May 2015 , Changes in displaying message properly
					objCustomMsgBox.m_csFailedMsgText.Format(L"%s \n\t\t\t\t %s \n %s", theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_FAILED_MSG_1"), theApp.m_objwardwizLangManager.GetString(L"IDS_STRING_OR"), theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_FAILED_MSG_2"));
					// = L"Failed to update product. Please check internet connection.\n\t\t\t\tOR\nDownload the offline patches manually from following Wardwiz link.";
					objCustomMsgBox.m_dwMsgErrorNo = 0x01;
					
					m_bIsPopUpDisplayed = true;
					objCustomMsgBox.DoModal();
					m_bIsPopUpDisplayed = false;
				}

				//MessageBox(L"Failed to update product. Please check internet connection.",theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION);
			}
			else if (m_dwstatusMsg == ALUPDATEFAILED_DOWNLOADINIPARSE || m_dwstatusMsg == ALUPDATEFAILED_DOWNLOADFILE || m_dwstatusMsg == ALUPDATEFAILED_EXTRACTFILE || m_dwstatusMsg == ALUPDATEFAILED_UPDATINGFILE)
			{
				CWrdWizCustomMsgBox objCustomMsgBox(this);
				if (objCustomMsgBox.m_hWnd == NULL)
				{
					//Need to set m_isDownloading here because if Updates are failed and custom msgbox is displayed then to keep same msg box on top need to this flag as false
					//Added by Nitin K Date 15th July 2015
					m_isDownloading = false;
					//Neha Gharge 13 May 2015 , Changes in displaying message properly
					objCustomMsgBox.m_csFailedMsgText = theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_FAILED_CUSTOM_MSG_BOX");
					objCustomMsgBox.m_dwMsgErrorNo = 0x02;

					m_bIsPopUpDisplayed = true;
					objCustomMsgBox.DoModal();
					m_bIsPopUpDisplayed = false;
				}
				//MessageBox(L"Failed to update product. Please try again or contact WardWiz Support team.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			}
			else if (m_dwstatusMsg == ALUPDATEFAILED_LOWDISKSPACE)
			{
				//Issue: If disk space is low in C: drive and if tried to update the product then warning popup appearing as 'failed to update the product. Please check internet connection'
				//Resolved by: Nitin K
				m_isDownloading = false;
				m_bIsPopUpDisplayed = true;
				MessageBox( theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_INSUFFUCIENT_DISK_SPACE"), L"WardWiz", MB_ICONEXCLAMATION|MB_OK);
				m_bIsPopUpDisplayed = false;
			}
		}
		//else
		///* Issue NO -159
		//Failed to update product. Please check internet connection” 
		//even if internet connection is there
		//Neha G.
		//10/sept/2014
		//*/
		//{

		//else if (m_dwstatusMsg == ALUPDATEFAILED_DOWNLOADINIPARSE || m_dwstatusMsg == ALUPDATEFAILED_DOWNLOADFILE || m_dwstatusMsg == ALUPDATEFAILED_EXTRACTFILE || m_dwstatusMsg == ALUPDATEFAILED_UPDATINGFILE)
		//{
		//	CWrdWizCustomMsgBox objCustomMsgBox(this);
		//	if (objCustomMsgBox.m_hWnd == NULL)
		//	{
		//		objCustomMsgBox.m_csFailedMsgText = L"Please download the offline patches from following WardWiz link.";
		//		objCustomMsgBox.DoModal();
		//	}
		//	//MessageBox(L"Failed to update product. Please try again or contact WardWiz Support team.", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		//}
		//}	

		m_btnCancel.EnableWindow(false);
		m_btnCancel.ShowWindow(false);
		m_btnSecHelp.ShowWindow(true);
		m_btnPauseResume.EnableWindow(false);
		m_isDownloading = false;

		KillTimer(TIMER_SETPERCENTAGESTATUS);

		ResetControls();


		//if (m_hThread != NULL)
		//{
		//	::SuspendThread(m_hThread);
		//	::TerminateThread(m_hThread, 0);
		//	m_hThread = NULL;
		//}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExLiveUpSecondDlg::ShowUpdateCompleteMessage()", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : GetApplicationDataFolder                                                     
*  Description    : It gets path of application data folder from registry
*  Author Name    : Ram Krushna  
*  SR_NO		  :
*  Date           : 24-Oct-2013
****************************************************************************************************/

bool CISpyLiveUpSecondDlg::GetApplicationDataFolder(TCHAR *szAppPath)
{
	try
	{
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_COMMON_APPDATA, NULL, 0, szAppPath)))
		{
			return true;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CLiveUpSecondPage::GetApplicationDataFolder", 0, 0, true, SECONDLEVEL);;
	}
	return false;
}

/***************************************************************************************************                    
*  Function Name  : CreateTempFolder                                                     
*  Description    : It creats the temp folder where download files get store temporary
*  Author Name    : RamKrushna 
*  SR_NO		  :
*  Date           : 25-Oct-2013
****************************************************************************************************/

bool CISpyLiveUpSecondDlg::CreateTempFolder()
{
	try
	{
		TCHAR szTagetPath[MAX_PATH] = {0};
		_stprintf_s(szTagetPath, MAX_PATH, L"%s\\%s",m_szAppDataFolder , szProductName); 
		if(!SendLiveUpdateOperation2Service(FILE_OPERATIONS, szTagetPath, L"", 3, false))
		{
			AddLogEntry(L"### Failed to create temp folder in CGenXLiveUpSecondDlg::SendLiveUpdateOperation2Service %s", szTagetPath, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CLiveUpSecondPage::CreateTempFolder", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************                    
*  Function Name  : MakeDownloadUrlList                                                     
*  Description    : It creates list of the url from where download should be take place.
*  Author Name    : Ram Krushna 
*  SR_NO		  :
*  Date           : 25-Oct-2013
****************************************************************************************************/
bool CISpyLiveUpSecondDlg::MakeDownloadUrlList()
{
	try
	{
		m_vUrlLists.clear();
		//CString csurl = L"http://www.ispyav.com/catalog/view/theme/AppealSoft/files/ISPYAV1.db";
		//m_vUrlLists.push_back(csurl);
		//CString csurl = L"http://www.itinav.com/catalog/view/theme/AppealSoft/files/ITINAV2.db";
		CString csurl = L"http://wardwiz.com//VirusDefinitions//WRDWIZUPD1.DB";
		m_vUrlLists.push_back(csurl);
		//csurl = L"http://www.itinav.com/catalog/view/theme/AppealSoft/files/ITINAV3.db";
		csurl = L"http://wardwiz.com//VirusDefinitions//WRDWIZUPD2.DB";
		m_vUrlLists.push_back(csurl);
		//csurl = L"http://www.ispyav.com/catalog/view/theme/AppealSoft/files/ISPYAV4.db";
		//m_vUrlLists.push_back(csurl);
		//CString csurl = L"http://www.selab.isti.cnr.it/ws-mate/example.pdf";
		//m_vUrlLists.push_back(csurl);
		//csurl = L"http://www.adobe.com/products/pdfjobready/pdfs/pdftraag.pdf";
		//m_vUrlLists.push_back(csurl);
		//csurl = L"http://www.indianembassy.at/newspdf/43552sample.pdf";
		//m_vUrlLists.push_back(csurl);
		//csurl = L"http://www.aisb.org.uk/convention/aisb08/AISB08.pdf";
		//FillListControl(m_vUrlLists);
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CLiveUpSecondPage::MakeDownloadUrlList", 0, 0, true, SECONDLEVEL);
	}
	return true;
}


/***************************************************************************************************                    
*  Function Name  : FillListControl                                                     
*  Description    : It lists threats into listcontrol box.
*  Author Name    : Ram Krushna   
*  SR_NO		  :
*  Date           : 25-Oct-2013
****************************************************************************************************/
void CISpyLiveUpSecondDlg::FillListControl(std::vector<CString> strVector)
{
	try
	{
		for(unsigned int iItemCount = 0; iItemCount < strVector.size(); iItemCount++)
		{
			CString csItem;
			csItem.Format(L"Threat definition %d", iItemCount + 1);
			m_lstFileDownloadStatus.InsertItem(iItemCount, csItem);
			m_lstFileDownloadStatus.SetItemText(iItemCount, 1, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_PENDING"));
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CLiveUpSecondPage::FillListControl", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************                    
*  Function Name  : StartDownloadFile                                                     
*  Description    : It starts downloading from the point from last download bytes.
*  Author Name    : Ram Krushna,Neha Gharge
*  SR_NO		  :
*  Date           : 25-Oct-2013
****************************************************************************************************/

bool CISpyLiveUpSecondDlg::StartDownloadFile(LPCTSTR szUrlPath)
{
	try
	{
		if(!szUrlPath)
			return false;

		CString csFileName(szUrlPath);
		int iFound = csFileName.ReverseFind(L'/');
		csFileName = csFileName.Right(csFileName.GetLength() - iFound - 1);

		TCHAR szInfo[MAX_PATH] = {0};
		TCHAR szTagetPath[MAX_PATH] = {0};
		DWORD dwBufLen = MAX_PATH * sizeof(TCHAR);
		DWORD dwTotalFileSize = 0;
		if(m_objWinHttpManager.Initialize(szUrlPath))
		{
			if(!m_objWinHttpManager.GetHeaderInfo(WINHTTP_QUERY_CONTENT_LENGTH, szInfo,dwBufLen))
			{
				return false;
			}
			dwTotalFileSize = _wtol(szInfo);

			m_iFileSize = dwTotalFileSize;

			_stprintf_s(szTagetPath, MAX_PATH, L"%s\\%s\\%s", m_szAppDataFolder, szProductName, csFileName); 

			//If file is already present check filesize and download from last point
			DWORD dwStartBytes = 0;
			if(PathFileExists(szTagetPath))
			{
				HANDLE hFile = CreateFile(szTagetPath, GENERIC_WRITE, FILE_SHARE_READ, NULL,OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
				if(hFile != INVALID_HANDLE_VALUE)
				{
					dwStartBytes = GetFileSize(hFile, 0);
					if(dwStartBytes != dwTotalFileSize)
					{
						m_objWinHttpManager.SetDownloadCompletedBytes(dwStartBytes);
					}
				}
				CloseHandle(hFile);
			}

			//if physics file size is greater than server file size then download from BEGIN
			if(dwStartBytes > dwTotalFileSize)
			{
				m_objWinHttpManager.SetDownloadCompletedBytes(0);
				dwStartBytes = 0;
			}

			//We have already downloaded the file no need to download again
			if(dwStartBytes == dwTotalFileSize)
			{
				m_objWinHttpManager.SetDownloadCompletedBytes(dwTotalFileSize);
				return true;
			}

			//Start download for file
			if(m_objWinHttpManager.Download(szTagetPath, dwStartBytes, dwTotalFileSize))
			{
				//Once download complete set the download completed bytes.
				m_objWinHttpManager.SetDownloadCompletedBytes(dwTotalFileSize - dwStartBytes);
			}
			else
			{
				AddLogEntry(L"### Failed to download file %s", szUrlPath, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CLiveUpSecondPage::StartDownloadFile", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************                    
*  Function Name	: OnTimer                                                     
*  Description	    : The framework calls this member function after each interval specified
					  in the SetTimer member function used to install a timer.
*  Author Name		: Ram Krushna,Neha Gharge,Lalit   
*  SR_NO		    :
*  Date				: 25-Oct-2013
*  Modification Date: 24-Jul-2014
****************************************************************************************************/

void CISpyLiveUpSecondDlg::OnTimer(UINT_PTR nIDEvent)
{
	try
	{
		CTimeSpan tsScanElapsedTime;
		if(nIDEvent == TIMER_SETPERCENTAGESTATUS)
		{
			//m_iCurrentDownloadBytes = m_objWinHttpManager.GetDownloadCompletedBytes();
			//m_dwCompletedBytes= m_objWinHttpManager.m_dwCompletedDownloadBytes;
			//int iTotalFileDownloadBytes = m_dwCompletedBytes + m_iCurrentDownloadBytes;
			//if(iTotalFileDownloadBytes == 0)
			//	return;

			//m_dwPercentage = GetPercentage(iTotalFileDownloadBytes, m_iTotalFileSize); 
			//Issue : 0000449 Issue with size mentioned in downloaded column. : Once download was successful it was showing Downloaded 100%(0 kb)
			//Resolved by Nitin K.
			if(m_dwPercentage >= 100)
			{
				m_dwPercentage = 100;
				m_prgFileDownloadStatus.SetPos(m_dwPercentage);
				//Issue : 0000449 Issue with size mentioned in downloaded column. : Once download was successful it was showing Downloaded 100%(0 kb)
				//Resolved by : Nitin K
				CString csStr;
				csStr.Format(L"%d %s [%d KB]", m_dwPercentage, L"%", static_cast<DWORD>(m_iCurrentDownloadBytes / 1024));
				m_stTotalDownload.SetWindowText(csStr);
				return;
			}

			//if(!m_objWinHttpManager.m_bIsConnected)
			//{
			//	m_stDownloadSpeed.SetWindowText(_T("0.00"));				
			//}
			
			if(g_iTotalFileDownloadBytes == m_iCurrentDownloadBytes && m_iCurrentDownloadBytes > 0)
			{
				//Issue : 0000449 Issue with size mentioned in downloaded column. : Once download was successful it was showing Downloaded 100%(0 kb)
				//Resolved by Nitin K.
				//return;
			}
			g_iTotalFileDownloadBytes = m_iCurrentDownloadBytes;

			//Varada Ikhar, Date: 13/02/2015, Issue: In live updates, while checking for updates, progress bar should not show 1%.
			//To show some progress we need to set to 1 at start
			//if(m_dwPercentage == 0)
			//{
			//	m_dwPercentage = 1;
			//}

			/*if(m_dwPercentage >= 80)
			{
				m_btnPauseResume.EnableWindow(false);
			}*/

			CString csStr;
			csStr.Format(L"%d %s [%d KB]", m_dwPercentage, L"%", static_cast<DWORD>(m_iCurrentDownloadBytes/1024));
			m_stTotalDownload.SetWindowText(csStr);

			m_prgFileDownloadStatus.SetPos(m_dwPercentage);

			//Calculate the transfer rate
			CTimeSpan tsScanElapsedTime = (CTime::GetCurrentTime() - m_tsStartTime);
			CString Ctime = tsScanElapsedTime.Format(_T("Elapsed Time:%H:%M:%S"));
			long lSeconds = tsScanElapsedTime.GetSeconds();
			long lMinutes = tsScanElapsedTime.GetMinutes();
			long lHours =tsScanElapsedTime.GetHours();

			long lMinTOSec = lMinutes * 60;
			long lHrsToSec = lHours * 3600;
			long TotalSec = lHrsToSec+ lMinTOSec + lSeconds;

			if(TotalSec == 0)
			{
				TotalSec = 1;
			}
			
			long lSpeed = m_iCurrentDownloadBytes / TotalSec;
			double lSpeed_Kbps = (((double)lSpeed) * 0.0078125);

			CString cSpeed;
			cSpeed.Format(_T("%0.2f"), lSpeed_Kbps);
			m_stDownloadSpeed.SetWindowText(cSpeed);

			if(lSpeed == 0)
			{
				lSpeed = 1;
			}

			/*Issue No:17, 48 Issue Desc: 17.Remaining Time in updates has minutes in 3 digits.
			48. In updates, the remaining time is coming wrong
			Resolved by :	Divya S..*/

			long lSecToHrs ,lSecToMin ,lSecToSec;
			long lRemainingTime = (m_iTotalFileSize - m_iCurrentDownloadBytes) / lSpeed;
			if(m_iCurrentDownloadBytes != 0)
			{
				lSecToHrs = lRemainingTime/3600;
				lSecToMin = lRemainingTime/60;
				lSecToSec = (lRemainingTime-(lSecToMin*60));
			}
			else
			{
				lSecToHrs = 0;
				lSecToMin = 0;
				lSecToSec = 0;
			}
			CString cRemaining;
			cRemaining.Format(_T("%02ld:%02ld:%02ld"),lSecToHrs,lSecToMin,lSecToSec);
			m_stRemainingTime.SetWindowText(cRemaining);

		}
		if(nIDEvent==ImageTimerID)
		{
			int countItem, nItem = 0;
			countItem=m_lstFileDownloadStatus.GetItemCount();
			CString col2= m_lstFileDownloadStatus.GetItemText(countItem-1,0);
			CString col3= m_lstFileDownloadStatus.GetItemText(countItem-1,1);

			m_lstFileDownloadStatus.DeleteItem(countItem-1);
			
			nItem = m_lstFileDownloadStatus.InsertItem(countItem - 1, m_stStaticInProgress, (m_iRowCount++) % 7);
			m_lstFileDownloadStatus.SetItem(nItem, 0, LVIF_TEXT,col2, 0, 0, 0, NULL);
			m_lstFileDownloadStatus.SetItem(nItem, 1, LVIF_TEXT, col3, 0, 0, 0, NULL);
			if(m_iRowCount==10){m_iRowCount=0;}
			UpdateData(true);
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CLiveUpSecondPage::OnTimer", 0, 0, true, SECONDLEVEL);
	}
	CJpegDialog::OnTimer(nIDEvent);
}


/***************************************************************************************************                    
*  Function Name  : GetPercentage                                                     
*  Description    : It gives percentage according to bytes downloaded and total size of file 
*  Author Name    : Ram Krushna,Neha Gharge 
*  SR_NO		  :
*  Date           : 25-Oct-2013
****************************************************************************************************/

DWORD CISpyLiveUpSecondDlg::GetPercentage(int iDownloaded, int iTotalSize)
{
	try
	{
		return static_cast<DWORD>(((static_cast<double>((iDownloaded))/ iTotalSize))*80);
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CLiveUpSecondPage::GetPercentage", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}


/***************************************************************************************************                    
*  Function Name  : GetTotalExistingBytes                                                     
*  Description    : It gives bytes size of files which already existing into the DB folder. 
*  Author Name    : Neha Gharge  
*  SR_NO		  :
*  Date           : 25-Nov-2013
****************************************************************************************************/
//DWORD CISpyLiveUpSecondDlg::GetTotalExistingBytes()
//{
//	memset(g_ExistingBytes, 0, sizeof(g_ExistingBytes)); 
//	memset(m_ExistingBytes, 0, sizeof(m_ExistingBytes));
//
//	DWORD dwFirstFileSize = 0;
//	DWORD dwSecondFileSize = 0;
//
//	CString csFilePath = theApp.GetModuleFilePath() + L"\\DB\\DAILY.CLD";
//	//HANDLE hSFile = CreateFile(csFilePath, GENERIC_WRITE, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
//	HANDLE hSFile = CreateFile(csFilePath, GENERIC_READ, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
//	dwFirstFileSize = 0;
//	if(hSFile != INVALID_HANDLE_VALUE)
//	{
//		dwFirstFileSize = GetFileSize(hSFile,NULL);
//		
//	}
//	CloseHandle(hSFile);
//
//	csFilePath = theApp.GetModuleFilePath() + L"\\DB\\MAIN.CVD";
//	//HANDLE hTFile = CreateFile(csFilePath, GENERIC_WRITE, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
//	HANDLE hTFile = CreateFile(csFilePath, GENERIC_READ, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
//	dwSecondFileSize = 0;
//	if(hTFile != INVALID_HANDLE_VALUE)
//	{
//		dwSecondFileSize=GetFileSize(hTFile,NULL);
//	}
//	CloseHandle(hTFile);
//	
//	g_ExistingBytes[0] = dwFirstFileSize;
//	g_ExistingBytes[1] = dwSecondFileSize;
//	m_ExistingBytes[0] = dwFirstFileSize;
//	m_ExistingBytes[1] = dwSecondFileSize;
//	return 0;
//}


/***************************************************************************************************                    
*  Function Name  : GetTotalFilesSize                                                     
*  Description    : It gives bytes size of files on server.Compare sizes of files and according
					to that downloading started.  
*  Author Name    : Neha Gharge ,Ram krushna
*  SR_NO		  :
*  Date           : 25-Nov-2013
****************************************************************************************************/

DWORD CISpyLiveUpSecondDlg::GetTotalFilesSize()
{
	DWORD dwTotalFilesSize = 0;
	m_vFileSizeMismatch.clear();

	try
	{
		if(m_vUrlLists.size() > 0)
		{
			for(unsigned int iItemCount = 0; iItemCount < m_vUrlLists.size(); iItemCount++)
			{
				CString csItem = m_vUrlLists[iItemCount];

				TCHAR szInfo[MAX_PATH] = {0};
				TCHAR szTagetPath[MAX_PATH] = {0};
				DWORD dwBufLen = MAX_PATH * sizeof(TCHAR);
				DWORD dwFileSize = 0;
				if(m_objWinHttpManager.Initialize(csItem))
				{

					ZeroMemory(szInfo, sizeof(szInfo) ) ;

					m_objWinHttpManager.GetHeaderInfo(WINHTTP_QUERY_CONTENT_LENGTH, szInfo,dwBufLen);
					//dwFileSize = _wtol(szInfo);
					dwFileSize = 0x00 ;
					swscanf( szInfo, L"%lu", &dwFileSize ) ;
					dwFileSize = dwFileSize - 0x15;
					/*	ISSUE NO - 669 NAME - NITIN K. TIME - 27th June 2014 */
					//Note: It will be updated only when file from server is having size greater than the existing DB file size
					//It will not be updated if it is lesser or equal
					if(theApp.m_ExistingBytes[iItemCount] < dwFileSize )
					{
						m_vFileSizeMismatch.push_back(csItem);
						dwTotalFilesSize += dwFileSize;
					}
				}
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CLiveUpSecondPage::GetTotalFilesSize", 0, 0, true, SECONDLEVEL);
		dwTotalFilesSize = 0;
	}
	return dwTotalFilesSize;
}


/***************************************************************************************************                    
*  Function Name  : CopyDownloadedFiles2InstalledFolder                                                     
*  Description    : After completion of download files.It copy into product installation folder 
*  Author Name    : Neha Gharge ,Ram krushna 
*  SR_NO		  :
*  Date           : 25-OCt-2013
****************************************************************************************************/

bool CISpyLiveUpSecondDlg::CopyDownloadedFiles2InstalledFolder()
{

	bool bReturn = false;
	try
	{

		TCHAR szAppDataPath[MAX_PATH] = {0};
		TCHAR szModulePath[MAX_PATH] = {0};
		GetApplicationDataFolder(szAppDataPath);
		GetModuleFileName(NULL, szModulePath, MAX_PATH);

		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		szTemp[0] = '\0';

		CString	csFolderProgramData = L"";
		csFolderProgramData.Format(L"%s\\Wardwiz Antivirus", szAppDataPath);

		std::vector<CString> csVectInputFiles;
		if(g_TabCtrlupdateWindHandle != NULL)
		{
			if(!(g_TabCtrlupdateWindHandle->m_pUpdate->CheckForValidUpdatedFiles(csFolderProgramData, csVectInputFiles)))
			{
				m_bIsPopUpDisplayed = true;
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_DATABASE_INVALID_MSG1"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
				m_bIsPopUpDisplayed = false;
				return false;
			}
		}
		m_bisDateTime = true;
		UpdateTimeDate();
		if(g_TabCtrlupdateWindHandle != NULL)
		{
			g_TabCtrlupdateWindHandle->m_pUpdate->UpdateVersionIntoRegistry();
		}


		//if(_tcslen(szAppDataPath) > 0)
		if(m_vUrlLists.size() > 0)
		{
			for(unsigned int iItemCount = 0; iItemCount < m_vUrlLists.size(); iItemCount++)
			{
				CString csUpdatePath = L"";
				CString csFileName = m_vUrlLists[iItemCount];
				int iFound = csFileName.ReverseFind(L'/');
				csFileName = csFileName.Right(csFileName.GetLength() - iFound - 1);

				csUpdatePath.Format(L"%s\\Wardwiz Antivirus\\%s", szAppDataPath, csFileName);
				CString csSourcePath, csDestination;
				csFileName.MakeLower();
				if(csFileName == L"wrdwizupd1.db")
				{
					csFileName = L"wrdwiztemp1.db";
				}
				if(csFileName == L"wrdwizupd2.db")
				{
					csFileName = L"wrdwiztemp2.db";
				}
				csSourcePath.Format(L"%s\\Wardwiz Antivirus\\%s", szAppDataPath, csFileName);

				csFileName.MakeLower();

				if(csFileName == L"wrdwiztemp1.db")
				{
					csFileName = L"DAILY.CLD";
				}
				else if(csFileName == L"wrdwiztemp2.db")
				{
					csFileName = L"MAIN.CVD";
				}
				else
				{
					return false;
					
				}

				csDestination.Format(L"%s\\DB\\%s", szModulePath, csFileName);

				Sleep(10);

				//send 2 service
				if(!SendLiveUpdateOperation2Service(FILE_OPERATIONS, csSourcePath, csDestination, 1, true))
				{
					AddLogEntry(L"### Failed to copy the file in CGenXLiveUpSecondDlg::SendLiveUpdateOperation2Service %s ,%s", csSourcePath,csDestination, true, SECONDLEVEL);
				}

				bReturn = true;
				if(PathFileExists(csUpdatePath))
				{
					SetFileAttributes(csUpdatePath, FILE_ATTRIBUTE_NORMAL);
					if(!DeleteFile(csUpdatePath))
					{
						AddLogEntry(L"### Failed to delete the file in CGenXLiveUpSecondDlg::CopyDownloadedFiles2InstalledFolder %s", csUpdatePath, 0, true, SECONDLEVEL);
					}
				}
			}	
		}
		m_isDownloading = false;
		//m_hThread = NULL;
		m_btnPauseResume.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_START"));
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CLiveUpSecondPage::CopyDownloadedFiles2InstalledFolder", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}


/***************************************************************************************************                    
*  Function Name  : PauseDownload                                                     
*  Description    : Pause download process
*  Author Name    : Ram krushna 
*  SR_NO		  :
*  Date           : 25-OCt-2013
****************************************************************************************************/
bool CISpyLiveUpSecondDlg::PauseDownload()
{
	bool bReturn = false;
	try
	{
		if (SendRequestCommon(PAUSE_UPDATE))
		{
			//if(m_hThread != NULL)
			{
				//SetItemDownloadStatus(PAUSED);
				//Varada Ikhar, Date: 24/04/2015, Implementing pause-resume if user clicks on 'close' button.
				//if (::SuspendThread(m_hThread) == -1)
				//{
				//	AddLogEntry(L"### Failed to pause Downloading updates thread.", 0, 0, true, SECONDLEVEL);
				//}
				/*if(m_Picture.IsPlaying())
				{
				m_Picture.Stop();
				}*/
				KillTimer(TIMER_SETPERCENTAGESTATUS);
				KillTimer(ImageTimerID);
				bReturn = true;

				int countItem, nItem = 0;
				countItem = m_lstFileDownloadStatus.GetItemCount();
				CString col2 = m_lstFileDownloadStatus.GetItemText(countItem - 1, 0);
				CString col3 = m_lstFileDownloadStatus.GetItemText(countItem - 1, 1);

				m_lstFileDownloadStatus.DeleteItem(countItem - 1);

				nItem = m_lstFileDownloadStatus.InsertItem(countItem - 1, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_PAUSED"), (m_iRowCount++) % 7);
				m_lstFileDownloadStatus.SetItem(nItem, 0, LVIF_TEXT, col2, 0, 0, 0, NULL);
				m_lstFileDownloadStatus.SetItem(nItem, 1, LVIF_TEXT, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_PAUSED"), 0, 0, 0, NULL);
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CLiveUpSecondPage::PauseDownload", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************                    
*  Function Name  : ResumeDownload                                                     
*  Description    : Resume download process
*  Author Name    : Ram krushna  
*  SR_NO		  :
*  Date           : 25-OCt-2013
****************************************************************************************************/
bool CISpyLiveUpSecondDlg::ResumeDownload()
{
	bool bReturn = false;
	try
	{
		if(SendRequestCommon(RESUME_UPDATE))
		{
			//if(m_hThread != NULL)
			{
				//SetItemDownloadStatus(RESUMED);
				//Varada Ikhar, Date: 24/04/2015, Implementing pause-resume if user clicks on 'close' button.
				//if (::ResumeThread(m_hThread) == -1)
				//{
				//	AddLogEntry(L"### Failed to resume Downloading update thread.", 0, 0, true, SECONDLEVEL);
				//}
				/*if(!m_Picture.IsPlaying())
				{
				m_Picture.Draw();
				}*/
				SetTimer(TIMER_SETPERCENTAGESTATUS, 500, NULL);
				SetTimer(ImageTimerID, 500, NULL);
				bReturn = true;

				int countItem, nItem = 0;
				countItem = m_lstFileDownloadStatus.GetItemCount();
				CString col2 = m_lstFileDownloadStatus.GetItemText(countItem - 1, 0);
				CString col3 = m_lstFileDownloadStatus.GetItemText(countItem - 1, 1);

				m_lstFileDownloadStatus.DeleteItem(countItem - 1);

				nItem = m_lstFileDownloadStatus.InsertItem(countItem - 1, theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_IN_PROGRESS"), (m_iRowCount++) % 7);
				m_lstFileDownloadStatus.SetItem(nItem, 0, LVIF_TEXT, col2, 0, 0, 0, NULL);
				m_lstFileDownloadStatus.SetItem(nItem, 1, LVIF_TEXT, theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_IN_PROGRESS"), 0, 0, 0, NULL);
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CLiveUpSecondPage::ResumeDownload", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************                    
*  Function Name  : StopDownloading                                                     
*  Description    : Stop download process
*  Author Name    : Ram krushna  
*  SR_NO		  :
*  Date           : 25-OCt-2013
****************************************************************************************************/

bool CISpyLiveUpSecondDlg::StopDownloading()
{
	
	bool bReturn = false;
	try
	{
		//if(m_hThread != NULL)
		{
			//Sleep(1000);

			//need to stop current download here.
			//m_objWinHttpManager.StopCurrentDownload();
			//m_objWinHttpManager.SetDownloadCompletedBytes(0);

			//::SuspendThread(m_hThread);
			//::TerminateThread(m_hThread, 0);
			//m_hThread = NULL;
			m_isDownloading = false;


			//m_objWinHttpManager.CloseFileHandles();

			/*if(m_Picture.IsPlaying())
			{
				m_Picture.Stop();
			}*/
			//KillTimer(TIMER_SETPERCENTAGESTATUS);
			bReturn = true;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CLiveUpSecondPage::StopDownloading", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}


/***************************************************************************************************                    
*  Function Name  : CheckInternetConnection                                                     
*  Description    : Checks the internet connection is available or not
*  Author Name    : Ram krushna 
*  SR_NO		  :
*  Date           : 25-OCt-2013
****************************************************************************************************/
bool CISpyLiveUpSecondDlg::CheckInternetConnection()
{
	bool bReturn = false;
	try
	{
		CWinHttpManager objWinHttpManager;
		TCHAR szTestUrl[MAX_PATH] = {0};
		_tcscpy_s(szTestUrl, MAX_PATH, _T("http://www.google.com"));
		if(objWinHttpManager.Initialize(szTestUrl))	
		{
			if(objWinHttpManager.CreateRequestHandle(NULL))
			{
				return true;
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CLiveUpSecondPage::CheckInternetConnection", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************                    
*  Function Name  : UpdateFromLocalFolder                                                     
*  Description    : Update from the local folder
*  Author Name    : Ram krushna  
*  SR_NO		  :
*  Date           : 25-OCt-2013
****************************************************************************************************/
DWORD CISpyLiveUpSecondDlg::UpdateFromLocalFolder(std::vector<CString> &csVectInputFiles)
{
	return CopyFromLocalFolder2InstalledFolder(csVectInputFiles);
}

/***************************************************************************************************                    
*  Function Name	 : CopyFromLocalFolder2InstalledFolder                                                     
*  Description		 : Copy from the local folder to installation folder
*  Author Name		 : Ram krushna 
*  SR_NO			 :
*  Date				 : 25-OCt-2013
*  Modification Date : 6 Jan 2015 Neha Gharge
*  MOdification		 : Clam And WardWiz Scanner Handle by preprocessor
*  Modified Date  : 6/2/2015 Neha Gharge FP file added
****************************************************************************************************/

DWORD CISpyLiveUpSecondDlg::CopyFromLocalFolder2InstalledFolder(std::vector<CString> &csVectInputFiles)
{
	DWORD dwReturn = 0x00;
	try
	{
		m_bDownLoadInProgress = false;
		int iUpdateFileCount = 0;
		int iItemCount = static_cast<int>(csVectInputFiles.size());
		
		for(int iIndex = 0; iIndex < iItemCount; iIndex++)
		{
			CString csSourcePath = csVectInputFiles[iIndex];
			if(PathFileExists(csSourcePath))
			{
				CString csFileName;
				int iFound = csSourcePath.ReverseFind(L'\\');
				csFileName = csSourcePath.Right(csSourcePath.GetLength() - iFound - 1);

				//Functionality change : New files added in Update from local folder
				//Implementated by : Nitin Kolapkar, Date: 16th Feb 2016
				//csFileName.MakeLower();
				//Prajakta

				/*if(theApp.m_eScanLevel == WARDWIZSCANNER)
				{
					if(csFileName == L"wrdwiztemp3.db")
					{
						csFileName = L"WRDWIZAV1.DB";
					}
					else if(csFileName == L"wrdwiztemp4.db")
					{
						csFileName = L"WRDWIZAVR.DB";
					}
					else
					{
						dwReturn = 0x03;
						break;
					}
				}
				else
				{
					if(csFileName == L"wrdwiztemp1.db")
					{
						csFileName = L"DAILY.CLD";
					}
					else if(csFileName == L"wrdwiztemp2.db")
					{
						csFileName = L"MAIN.CVD";
					}
					else if(csFileName == L"wrdwiztemp3.db")
					{
						csFileName = L"WRDWIZAV1.DB";
					}
					else if(csFileName == L"wrdwiztemp4.db")
					{
						csFileName = L"WRDWIZAVR.DB";
					}
					else if(csFileName == L"wrdwiztemp5.db")
					{
						csFileName = L"WRDWIZWHLST.FP";
					}
					else
					{
						dwReturn = 0x03;
						break;
					}
				}*/


				CString csDestination;
				//Prajakta
				if( theApp.m_eScanLevel != WARDWIZSCANNER)
				{
					if((csFileName == L"DAILY.CLD")|| (csFileName == L"MAIN.CVD") || (csFileName == L"WRDWIZWHLST.FP"))
					{
						csDestination.Format(L"%s\\DB\\%s", theApp.GetModuleFilePath(), csFileName);
					}
					else
					{
						csDestination.Format(L"%s\\WRDWIZDATABASE\\%s", theApp.GetModuleFilePath(), csFileName);
					}
				}
				else
				{
					if ((csFileName == L"DAILY.CLD") || (csFileName == L"MAIN.CVD") || (csFileName == L"WRDWIZWHLST.FP"))
					{
						//csDestination.Format(L"%s\\DB\\%s", theApp.GetModuleFilePath(), csFileName);
						continue;
					}
					else
					{
						csDestination.Format(L"%s\\WRDWIZDATABASE\\%s", theApp.GetModuleFilePath(), csFileName);
					}
				}
 
				//if ((csFileName == L"WRDWIZAV1.DB") || (csFileName == L"WRDWIZAVR.DB"))
				//{
				//	csDestination.Format(L"%s\\WRDWIZDATABASE\\%s", theApp.GetModuleFilePath(), csFileName);
				//}
				
				Sleep(10);
				//move file from program data to installation folder.
				//if(SendLiveUpdateOperation2Service(FILE_OPERATIONS, csSourcePath, csDestination, 1, true))
				if (MoveFileEx(csSourcePath, csDestination, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED))
				{
					iUpdateFileCount++;
					m_lstFileDownloadStatus.SetItemText(iIndex, 1, L"Completed");
				}
				else
				{
					AddLogEntry(L"### Error in CLiveupdateSecDlg:FILE_OPERATIONS in SendLiveUpdateOperation2Service", 0, 0, true, SECONDLEVEL);
					m_lstFileDownloadStatus.SetItemText(iIndex, 1, L"Failed");
				}
			}
		}

		if(iUpdateFileCount <= 3)
		{
			dwReturn = 0x01;
			goto Cleanup;
		}
		else if(iUpdateFileCount >= 4)
		{
			dwReturn = 0x02;
			goto Cleanup;
		}
		else
		{
			dwReturn = 0x03;
			goto Cleanup;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CLiveUpSecondPage::CopyDownloadedFiles2InstalledFolder", 0, 0, true, SECONDLEVEL);
		dwReturn = 0x03;
	}
Cleanup:
	return dwReturn;
}


/***************************************************************************************************                    
*  Function Name  : SetItemDownloadStatus                                                     
*  Description    : Sets status according to processes of downloading
*  Author Name    : Ram krushna 
*  SR_NO		  :
*  Date           : 25-OCt-2013
****************************************************************************************************/
void CISpyLiveUpSecondDlg::SetItemDownloadStatus(eDownloadStatus eStatus)
{
	CString csStatus = L"";
	switch(eStatus)
	{
	case PENDING:
		csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_PENDING");
		break;
	case DOWNLOADING:
	case RESUMED:
		csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_DOWNLOADING");
		if(!m_objWinHttpManager.m_bIsConnected)
		{
			csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_RECONNECTING");
		}
		break;
	case PAUSED:
		csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_PAUSED");
		break;
	case FAILEDSTATUS:
		csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED");
		break;
	case RECONNECTING:
		csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_RECONNECTING");
		break;
	case COMPLETED:
		csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_COMPLETED");
		break;
	case CANCELLED:
		csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_CANCELLED");
		break;
	}

	m_lstFileDownloadStatus.SetItemText(m_iCurrentItemCount, 1, csStatus);
}

/***************************************************************************************************                    
*  Function Name  : WaitForInternetConnection                                                     
*  Description    : It waits for some time to check internet connection.  
*  Author Name    : Ram krushna  
*  SR_NO		  :
*  Date           : 25-OCt-2013
****************************************************************************************************/
bool CISpyLiveUpSecondDlg::WaitForInternetConnection()
{
	bool bReturn = false;
	int iRetryCount = 0;
	while(true)
	{
		if(!CheckInternetConnection())
		{
			if(iRetryCount > MAX_RETRY_COUNT)
			{
				bReturn = false;
				break;				
			}
			iRetryCount++;
			Sleep(10 * 1000);//wait here for 10 seconds
		}
		else
		{
			bReturn = true;
			break;
		}
	}
	return bReturn;
}


/***************************************************************************************************                    
*  Function Name  : EnableWindows                                                     
*  Description    : It enables the window.  
*  Author Name    : Ram krushna 
*  SR_NO		  :
*  Date           : 25-OCt-2013
****************************************************************************************************/
void CISpyLiveUpSecondDlg::EnableWindows(bool bEnable)
{
	m_stTotalDownload.ShowWindow(bEnable);
	m_stTotalFileSize.ShowWindow(bEnable);
	m_stDownloadSpeed.ShowWindow(bEnable);
	m_stRemainingTime.ShowWindow(bEnable);
	m_stTimeRemaining.ShowWindow(bEnable);
	m_stSpeedHeader.ShowWindow(bEnable);
}

/***************************************************************************************************       
*  Function Name  : AdjustControls                                                     
*  Description    :           
*  Author Name    : Ram krushna
*  SR_NO		  :
*  Date           : 25-OCt-2013
****************************************************************************************************/
void CISpyLiveUpSecondDlg::AdjustControls()
{
	CRect oRcDlgRect;
	GetClientRect(&oRcDlgRect);

	HDWP hdwp = BeginDeferWindowPos(20);

	CRect oRcListControl;
	m_lstFileDownloadStatus.GetClientRect(&oRcListControl);
	ScreenToClient(&oRcListControl);
	DeferWindowPos(hdwp, m_lstFileDownloadStatus, NULL, oRcDlgRect.left + 1, oRcDlgRect.top + 41, oRcListControl.Width() + 4, oRcListControl.Height() + 40, SWP_NOZORDER);

	CRect oRcprgFileDownloadStatus;
	m_prgFileDownloadStatus.GetClientRect(&oRcprgFileDownloadStatus);
	ScreenToClient(&oRcprgFileDownloadStatus);
	DeferWindowPos(hdwp, m_prgFileDownloadStatus, NULL, oRcDlgRect.left + 1, oRcDlgRect.top + 90 + oRcListControl.Height(), oRcprgFileDownloadStatus.Width(),
		oRcprgFileDownloadStatus.Height(), SWP_NOZORDER);

	EndDeferWindowPos(hdwp);
}

/***************************************************************************************************         
*  Function Name  : OnBnClickedButtonPause                                                     
*  Description    : One button represents three activities as Pause,Resume,Start 
*  Author Name    :	Neha Gharge,Ram Krushna 
*  SR_NO		  :
*  Date           : 20-Nov-2013
****************************************************************************************************/
void CISpyLiveUpSecondDlg::OnBnClickedButtonPause()
{
	CString csBtnText;
	m_btnPauseResume.GetWindowText(csBtnText);
	m_btnPauseResume.ShowWindow(SW_HIDE);
	m_btnResume.ShowWindow(SW_SHOW);
	m_btnPauseResume.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RESUME"));
	PauseDownload();
}

/***************************************************************************************************        
*  Function Name  : ShutDownDownload                                                     
*  Description    : It shows message whether downloading should stop or not.
*  Author Name    :	Neha Gharge
*  SR_NO		  :
*  Date           : 23-Nov-2013
****************************************************************************************************/
bool CISpyLiveUpSecondDlg::ShutDownDownload()
{
	try
	{
		if (!m_isDownloading)
		{
			return true;
		}

		//Varada Ikhar, Date: 24/04/2015, Implementing pause-resume if user clicks on 'close' button.
		CString csPauseResumeBtnText = L"";
		m_btnPauseResume.GetWindowText(csPauseResumeBtnText);
		if (csPauseResumeBtnText != theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RESUME"))
		{
			if (!PauseDownload())
			{
				AddLogEntry(L"### Failed to pause update in CNextGenExLiveUpSecondDlg::ShutDownDownload", 0, 0, true, SECONDLEVEL);
			}
		}

		AddLogEntry(L">>> CLiveupdateDlg Stopping downloading", 0, 0, true, FIRSTLEVEL);

		m_bIsPopUpDisplayed = true;
		int iRet = MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DOWNLOADING_IN_PROGRESS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),
			MB_YESNO | MB_ICONQUESTION);
		m_bIsPopUpDisplayed = false;

		if (IDYES == iRet)
		{
			//Neha Gharge 3/3/2015 
			//Issue : if user click on close button not take any action YES/NO, UI get hanged or not refresh
			if (m_bCloseUpdate)
			{
				theApp.m_bOnCloseFromMainUI = true;
				m_bCloseUpdate = false;
			}
			//if(!StopDownloading())
			//{
			//	AddLogEntry(L"### Error during stopping the live update:: CWrdWizLiveUpSecondDlg::OnBnClickedButtonCancel", 0, 0, true, SECONDLEVEL);
			//	return false;
			//}
			if (SendRequestCommon(STOP_UPDATE))
			{
				m_isDownloading = false;
				KillTimer(TIMER_SETPERCENTAGESTATUS);
				KillTimer(ImageTimerID);
				OutputDebugString(L">>> Before ResetControls");
				m_btnPauseResume.ShowWindow(TRUE);
				m_btnResume.ShowWindow(FALSE);
				//SetItemDownloadStatus(CANCELLED);
				ResetControls();
				m_bManualStop = true;
				this->ShowWindow(SW_HIDE);
				CISpyUpdatesDlg *pObjMainUI = reinterpret_cast<CISpyUpdatesDlg*>(this->GetParent());
				pObjMainUI->ShowWindow(SW_SHOW);
				pObjMainUI->ShowHideAllUpdateFirstPageControls(true);

				OutputDebugString(L">>> After ResetControls");
				return true;
			}

		}
		//Varada Ikhar, Date: 24/04/2015, Implementing pause-resume if user clicks on 'close' button.
		else
		{
			//lalit 5-5-2015,if we try to close update from tray and make it no and then cancel update by clicking cancel button in such case update dlg not looking
			m_bCloseUpdate = false;
			CString csPauseResumeBtnText = L"";
			m_btnPauseResume.GetWindowText(csPauseResumeBtnText);
			if (csPauseResumeBtnText != theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_RESUME"))
			{
				if (!ResumeDownload())
				{
					AddLogEntry(L"### Failed to resume update in CNextGenExLiveUpSecondDlg::ShutDownDownload", 0, 0, true, SECONDLEVEL);
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExLiveUpSecondDlg::ShutDownDownload", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************           
*  Function Name  : OnBnClickedButtonDwcancel                                                     
*  Description    : It shows message whether downloading should stop or not.
*  Author Name    :	Neha Gharge 
*  SR_NO		  :
*  Date           : 23-Nov-2013
****************************************************************************************************/
void CISpyLiveUpSecondDlg::OnBnClickedButtonDwcancel()
{
	//ShutDownDownload();

	// lalit  4-27-2015 , due to mutuall operation implementaion no more need to hide other tab control when any one is in-progress 
	g_TabCtrlupdateWindHandle = (CISpyGUIDlg*)AfxGetMainWnd();
	if (g_TabCtrlupdateWindHandle != NULL)
	{
		g_TabCtrlupdateWindHandle->m_bIsCloseHandleCalled = true;
		//m_bCloseUpdate = true;
		if (!ShutDownDownload())
		{
			g_TabCtrlupdateWindHandle->m_bisUIcloseRquestFromTray = false;
		}
		else
		{
			//if (m_hThread != NULL)
			//{
			//	::SuspendThread(m_hThread);
			//	::TerminateThread(m_hThread, 0);
			//	m_hThread = NULL;
			//}
			m_isDownloading = false;
			
		}
		g_TabCtrlupdateWindHandle->m_bIsCloseHandleCalled = false;

		if (g_TabCtrlupdateWindHandle->m_bisUIcloseRquestFromTray)
		{
			// resolved by lalit 5-5-2015
			//issue if "do you want to close message exits and we close from tray- 
			//in such case we convert already exist message popup as tray exit,and such case we have to close tray "
			g_TabCtrlupdateWindHandle->CloseSystemTray();
			g_TabCtrlupdateWindHandle->HandleCloseButton();
		}

	}
	
}

/***********************************************************************************************             
*  Function Name  : OnNcHitTest                                                     
*  Description    : The framework calls this member function for the CWnd object that 
					contains the cursor every time the mouse is moved.
*  Author Name    : Neha Gharge  
*  SR_NO		  :
*  Date           : 23-Nov-2013
*************************************************************************************************/
LRESULT CISpyLiveUpSecondDlg::OnNcHitTest(CPoint point)
{
	return HTCLIENT;
}

/***********************************************************************************************                   
*  Function Name  : OnSetCursor                                                     
*  Description    : The framework calls this member function if mouse input is not
					captured and the mouse causes cursor movement within the CWnd object..
*  Author Name    : Neha Gharge   
*  SR_NO		  :
*  Date           : 23-Nov-2013
*************************************************************************************************/
BOOL CISpyLiveUpSecondDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if( 
		iCtrlID == IDC_BUTTON_DWCANCEL		||
		iCtrlID == IDC_BUTTON_PAUSE			||
		iCtrlID == IDC_SEC_BUTTON_HELP		||
		iCtrlID == IDC_BUTTON_RESUME
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


/***********************************************************************************************                  
*  Function Name  : UpdateTimeDate                                                     
*  Description    : This function update date and time into registry after completion of download
*  Author Name    : Neha Gharge     
*  SR_NO		  :
*  Date           : 23-Nov-2013
*************************************************************************************************/
void CISpyLiveUpSecondDlg::UpdateTimeDate()
{
	CString  csDate,csTime;
	TCHAR szOutMessage[30] = {0};
	TCHAR tbuffer[9]= {0};
	TCHAR dbuffer[9] = {0};
	/*Rajil Yadav Issue no. 598, 6/11/2014*/
	SYSTEMTIME  CurrTime = {0} ;
	GetLocalTime(&CurrTime);//Ram, Issue resolved:0001218
	CTime Time_Curr( CurrTime ) ;
	int iMonth = Time_Curr.GetMonth();
	int iDate = Time_Curr.GetDay();
	int iYear = Time_Curr.GetYear();

	if(m_bisDateTime)
	{
		_wstrtime_s(tbuffer, 9);
		csTime.Format(L"%s\0\r\n",tbuffer);
		csDate.Format(L"%d/%d/%d",iMonth,iDate,iYear);
		_stprintf(szOutMessage, _T("%s %s\0"), csDate, tbuffer);
	}

	if(!SendRegistryData2Service(SZ_STRING, _T("SOFTWARE\\Wardwiz Antivirus"), 
		_T("LastLiveupdatedt"), szOutMessage, true))
	{
		AddLogEntry(L"### Failed to LastLiveupdatedt SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
	}
	Sleep(10);
}

/***********************************************************************************************                    
*  Function Name  : ResetControls                                                     
*  Description    : Reset the values
*  Author Name    : Ram Krushna   
*  SR_NO		  :
*  Date           : 23-Nov-2013
*************************************************************************************************/
void CISpyLiveUpSecondDlg::ResetControls()
{
	m_iCurrentDownloadBytes = 0;
	m_iFileSize = 0;
	m_dwPercentage = 0;
	m_dwstatusMsg = -1;
	m_bManualStop = false;
	m_prgFileDownloadStatus.SetPos(0);
	m_stTotalDownload.SetWindowText(L"0 % [0 KB]");
	m_stDownloadSpeed.SetWindowText(_T("0.00"));
	m_stRemainingTime.SetWindowText(L"00:00:00");
	m_stTotalFileSize.SetWindowText(L"0 KB");
	m_dwTotalFileNo = 0;
	m_dwDownloadOrUpdateFileNo = 0;
	m_csListControlStatus = L"";
	if(m_lstFileDownloadStatus.GetItemCount() > 0)
	{
		m_lstFileDownloadStatus.DeleteAllItems();
	}
	Invalidate();

	/*if(m_Picture.IsPlaying())
	{
		m_Picture.Stop();
	}*/
}
/***********************************************************************************************                    
*  Function Name  : IsFileMismatch                                                     
*  Description    : if existing file in the DB and server file mismatch then it assigns 
					new download list.
*  Author Name    : Neha Gharge,Ram krushna 
*  SR_NO		  :
*  Date           : 25-Nov-2013
*************************************************************************************************/
bool CISpyLiveUpSecondDlg::IsFileMismatch()
{
	bool bReturn = false;
	
	if(m_vFileSizeMismatch.size() != 0)
	{
		m_vUrlLists.clear();
		m_vUrlLists=m_vFileSizeMismatch;
		FillListControl(m_vUrlLists);
		m_vFileSizeMismatch.clear();
		bReturn = true;
	}
	return bReturn;
}

/***********************************************************************************************                    
*  Function Name  : SendRegistryData2Service                                                     
*  Description    : Send request to service to set registry through pipe.
*  Author Name    : Neha Gharge,Ram krushna 
*  SR_NO		  :
*  Date           : 25-Nov-2013
*************************************************************************************************/
bool CISpyLiveUpSecondDlg::SendRegistryData2Service(DWORD dwType, LPTSTR szKey, LPTSTR szValue, LPTSTR szData, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	
	szPipeData.iMessageInfo = WRITE_REGISTRY;
	szPipeData.dwValue = dwType; 
	//szPipeData.hHey = hKey;
	wcscpy_s(szPipeData.szFirstParam, szKey);
	wcscpy_s(szPipeData.szSecondParam, szValue);
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

/***********************************************************************************************                    
*  Function Name  : SendLiveUpdateOperation2Service                                                     
*  Description    : Send request to do live update operation to service.
*  Author Name    : Neha Gharge,Ram krushna 
*  SR_NO		  :
*  Date           : 25-Nov-2013
*************************************************************************************************/
bool CISpyLiveUpSecondDlg::SendLiveUpdateOperation2Service(DWORD dwType, CString csSrcFilePath, CString csDestFilePath, DWORD dwValue, bool bLiveUpdateWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(szPipeData));
	
	szPipeData.iMessageInfo = dwType;
	szPipeData.dwValue = dwValue;
	wcscpy_s(szPipeData.szFirstParam, csSrcFilePath);
	wcscpy_s(szPipeData.szSecondParam, csDestFilePath);
	
	CISpyCommunicator objCom(SERVICE_SERVER, true);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send data in CGenXLiveUpSecondDlg::SendLiveUpdateOperation2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bLiveUpdateWait)
	{
		if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
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
*  Function Name  : RefreshStrings                                                     
*  Description    : this function is  called for setting the Text UI with different Language Support
*  Author Name    : Neha Gharge,Ram krushna 
*  SR_NO		  :
*  Date           : 25-Jun-2014
*************************************************************************************************/
void CISpyLiveUpSecondDlg::RefreshStrings()
{
		m_stTotalDownload.SetWindowText(L"0 % [0 KB]");
		m_stTotalFileSize.SetWindowText(L"0 KB");
		m_btnCancel.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_CANCEL"));
		m_btnPauseResume.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PAUSE"));
		m_stUpdatesSecondHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATES_SECOND_HEADER"));
		m_stUpdatesSubHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SUB_HEADER"));
		m_btnSecHelp.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BACK"));
		m_stTimeRemaining.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REMAINING"));
		m_stSpeedHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SPEED"));
		m_stDownloaded.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_DOWNLOADED"));
		m_stTotalSize.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TOTAL_SIZE"));
		m_stStaticInProgress =  theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_IN_PROGRESS");
}

/***********************************************************************************************                    
*  Function Name  : OnBnClickedSecButtonStop                                                     
*  Description    : Handler for second stop button
*  Author Name    : Neha Gharge,Ram krushna 
*  SR_NO		  :
*  Date           : 25-Jun-2014
*************************************************************************************************/
void CISpyLiveUpSecondDlg::OnBnClickedSecButtonStop()
{
	// TODO: Add your control notification handler code here
	ResetControls();
	//m_btnPauseResume.SetWindowTextW(L"Pause");
	this->ShowWindow(SW_HIDE);
	CISpyUpdatesDlg *pObjMainUI = reinterpret_cast<CISpyUpdatesDlg*>( this->GetParent() );
	pObjMainUI->ShowWindow(SW_SHOW);
	pObjMainUI->ShowHideAllUpdateFirstPageControls(true);
}

/***********************************************************************************************                    
*  Function Name  : OnCtlColor                                                     
*  Description    : The framework calls this member function when a child control is about to be drawn.
*  Author Name    : Neha Gharge,Ram krushna 
*  SR_NO		  :
*  Date           : 25-Jun-2014
*************************************************************************************************/
HBRUSH CISpyLiveUpSecondDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int	ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if(	ctrlID == IDC_STATIC_UPDATES_SECOND_HEADER ||
		ctrlID == IDC_STATIC_SECOND_HEADER ||
		ctrlID == IDC_STATIC_TIMEREMAININGHEADER ||
		ctrlID == IDC_STATIC_SPEEDHEADER
		//ctrlID == IDC_STATIC_DOWNLOADED ||
		//ctrlID == IDC_STATIC_TOTAL_SIZE
		)
		
	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	}	return hbr;
}

/***********************************************************************************************                    
*  Function Name  : PreTranslateMessage                                                     
*  Description    : Translate window messages before they are dispatched to the TranslateMessage and DispatchMessage Windows functions
*  Author Name    : Neha Gharge,Ram krushna 
*  SR_NO		  :
*  Date           : 25-Jun-2014
*************************************************************************************************/
BOOL CISpyLiveUpSecondDlg::PreTranslateMessage(MSG* pMsg)
{
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}

	return CJpegDialog::PreTranslateMessage(pMsg);
}

/***********************************************************************************************                    
*  Function Name  : GoToHomePage                                                     
*  Description    : Go to home page
*  Author Name    : Neha Gharge,Ram krushna 
*  SR_NO		  :
*  Date           : 25-Jun-2014
*************************************************************************************************/
void CISpyLiveUpSecondDlg::GoToHomePage()
{
	//ResetControls();
	//this->ShowWindow(SW_HIDE);
	//CISpyUpdatesDlg *pObjMainUI = reinterpret_cast<CISpyUpdatesDlg*>( this->GetParent() );
	//pObjMainUI->ShowWindow(SW_SHOW);
	//pObjMainUI->ShowHideAllUpdateFirstPageControls(true);
}

/***********************************************************************************************                    
*  Function Name  : OnBnClickedButtonResume                                                     
*  Description    : Resume updation
*  Author Name    : Neha Gharge,Ram krushna 
*  SR_NO		  :
*  Date           : 25-Jun-2014
*************************************************************************************************/
void CISpyLiveUpSecondDlg::OnBnClickedButtonResume()
{
	m_btnPauseResume.ShowWindow(SW_SHOW);
	m_btnResume.ShowWindow(SW_HIDE);
	m_btnPauseResume.SetWindowText(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_PAUSE"));
	ResumeDownload();
	// TODO: Add your control notification handler code here
}

/***********************************************************************************************                    
*  Function Name  : OnPaint                                                     
*  Description    : The framework calls this member function when Windows or an application makes a request to repaint a portion of an application's window.
*  Author Name    : Neha Gharge,Ram krushna 
*  SR_NO		  :
*  Date           : 25-Jun-2014
*************************************************************************************************/
void CISpyLiveUpSecondDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CJpegDialog::Draw();
	CJpegDialog::OnPaint();
}


/***********************************************************************************************                    
*  Function Name  : SendRequestCommon                                                     
*  Description    : Send request to auto live update services.
 
*  Author Name    : Neha Gharge 
*  SR_NO		  :
*  Date           : 20-July-2013
*************************************************************************************************/
bool CISpyLiveUpSecondDlg::SendRequestCommon(int iRequest)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = {0};
		szPipeData.iMessageInfo = iRequest;
		szPipeData.dwValue = 1;

		CISpyCommunicator objCom(AUTOUPDATESRV_SERVER, true);
		if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data to AUTOUPDATESRV_SERVER in CGenXLiveUpSecondDlg::SendRequestCommon", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CGenXLiveUpSecondDlg::SendRequestCommon", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***********************************************************************************************                    
*  Function Name  : InsertItem                                                     
*  Description    : Insert item In listcontrol
 
*  Author Name    : Neha Gharge 
*  SR_NO		  :
*  Date           : 20-July-2013
*************************************************************************************************/
void CISpyLiveUpSecondDlg::InsertItem(CString csInsertItem ,CString csActualStatus)
{
	int nItem = 0;


	CString cstmp;
//	issue number 123 resolved by lalit  "Checking for update" is taking a lot of time to come
	if (csActualStatus == theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_CHECK_FOR_UPDATES"))
	{
		if(m_bIsCheckingForUpdateEntryExistInList)
		{
			OnAddUpdateStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_CHECK_FOR_UPDATES"));
		m_bIsCheckingForUpdateEntryExistInList= false;
		}
	}
	else if(csActualStatus == L"Already downloaded")
	{
		/* issue:- 165 Resolved by - lalit 9/2/2014 */
      
		m_bISalreadyDownloadAvailable= true;
		m_lstFileDownloadStatus.DeleteItem(1);
		OnAddUpdateStatus(theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_ALREADY_DOWNLOADED"));
	}
	else if(csActualStatus == L"Downloading files")
	{
		/* issue:- 165 Resolved by - lalit 9/2/2014 */
      
		m_bISalreadyDownloadAvailable= false;
		m_lstFileDownloadStatus.DeleteItem(1);
		OnAddUpdateStatus(csInsertItem);
	}
	else if(csActualStatus == L"Updating files")
	{
		m_lstFileDownloadStatus.DeleteItem(2);
		OnAddUpdateStatus(csInsertItem);
	}
}


/***********************************************************************************************                    
*  Function Name  : InitImageList                                                     
*  Description    : Initialization of image list
 
*  Author Name    : Neha Gharge 
*  SR_NO		  :
*  Date           : 20-July-2013
*************************************************************************************************/
bool CISpyLiveUpSecondDlg::InitImageList()
{
	HIMAGELIST hList;
	
	hList = ImageList_Create(25, 25, ILC_COLOR32|ILC_MASK,0,8);
	m_imageList.Attach(hList);
	
	CBitmap bm1,bm2,bm3,bm4,bm5,bm6,bm7,bm8,bmCheck;
	bm1.LoadBitmapW(IDB_BITMAP_PROGRESS1);
	m_imageList.Add(&bm1, RGB(0, 0, 0));                // image added to CImageList

	bm2.LoadBitmapW(IDB_BITMAP_PROGRESS2);
	m_imageList.Add(&bm2, RGB(0, 0, 0));

	bm3.LoadBitmapW(IDB_BITMAP_PROGRESS3);
	m_imageList.Add(&bm3, RGB(0, 0, 0));

	bm4.LoadBitmapW(IDB_BITMAP_PROGRESS4);
	m_imageList.Add(&bm4, RGB(0, 0, 0));
	//Issue Resolved: Animation improved
	//Resolved By: Neha G Date: 18th May 2015
	bm5.LoadBitmapW(IDB_BITMAP_PROGRESS5);
	m_imageList.Add(&bm5, RGB(0, 0, 0));

	bm6.LoadBitmapW(IDB_BITMAP_PROGRESS6);
	m_imageList.Add(&bm6, RGB(0, 0, 0));

	bm7.LoadBitmapW(IDB_BITMAP_PROGRESS7);
	m_imageList.Add(&bm7, RGB(0, 0, 0));

	bm8.LoadBitmapW(IDB_BITMAP_PROGRESS8);
	m_imageList.Add(&bm8, RGB(0, 0, 0));

	bmCheck.LoadBitmapW(IDB_BITMAP_PROGRESS_COMPLETE);
	m_imageList.Add(&bmCheck, RGB(0, 0, 0));

	m_lstFileDownloadStatus.SetImageList(&m_imageList, LVSIL_SMALL);    //image set to
	m_lstFileDownloadStatus.SetImageList(&m_imageList, LVSIL_NORMAL);
	m_iRowCount = 0;
	return true;
}


/***********************************************************************************************                    
*  Function Name  : OnAddUpdateStatus                                                   
*  Description    : On adding the status into list control
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 20-July-2013
*************************************************************************************************/
void CISpyLiveUpSecondDlg::OnAddUpdateStatus(CString csStatus)
{
	int nItem;

	int count= m_lstFileDownloadStatus.GetItemCount();
	if(count>0)
	{
		KillTimer(ImageTimerID);
		int listItemToBeUpdate= m_lstFileDownloadStatus.GetItemCount();
		CString col2= m_lstFileDownloadStatus.GetItemText(listItemToBeUpdate-1,0);
		CString col3= m_lstFileDownloadStatus.GetItemText(listItemToBeUpdate-1,1);
		m_lstFileDownloadStatus.DeleteItem(listItemToBeUpdate-1);
		nItem = m_lstFileDownloadStatus.InsertItem(listItemToBeUpdate-1, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_COMPLETED"),8);
		m_lstFileDownloadStatus.SetItem(nItem, 0, LVIF_TEXT, col2, 0, 0, 0, NULL);
		m_lstFileDownloadStatus.SetItem(nItem, 1, LVIF_TEXT, theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_COMPLETED"), 0, 0, 0, NULL);


		nItem = m_lstFileDownloadStatus.InsertItem(count, theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_IN_PROGRESS"), 0);
		m_lstFileDownloadStatus.SetItem(nItem, 0, LVIF_TEXT,csStatus, 0, 0, 0, NULL);
		m_lstFileDownloadStatus.SetItem(nItem, 1, LVIF_TEXT, theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_IN_PROGRESS"), 0, 0, 0, NULL);

		m_lstFileDownloadStatus.EnsureVisible(count,true);
		m_iRowCount++;

		//Ram: No need to set the timer if in case it is downloaded already
		if (!m_bISalreadyDownloadAvailable)
		{
			SetTimer(ImageTimerID, 500, NULL);
		}
	}
	else
	{
		nItem = m_lstFileDownloadStatus.InsertItem(count, theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_IN_PROGRESS"), 0);
		m_lstFileDownloadStatus.SetItem(nItem, 0, LVIF_TEXT,csStatus , 0, 0, 0, NULL);
		m_lstFileDownloadStatus.SetItem(nItem, 1, LVIF_TEXT, theApp.m_objwardwizLangManager.GetString(L"IDS_UPDATE_IN_PROGRESS"), 0, 0, 0, NULL);

		m_lstFileDownloadStatus.EnsureVisible(count,true);
		m_iRowCount++;

		//Ram: No need to set the timer if in case it is downloaded already
		if (!m_bISalreadyDownloadAvailable)
		{
			SetTimer(ImageTimerID, 500, NULL);
		}
	}
}

void CISpyLiveUpSecondDlg::EnableControls(BOOL bEnable)
{
	m_btnCancel.EnableWindow(bEnable);
	m_btnPauseResume.EnableWindow(bEnable);
}