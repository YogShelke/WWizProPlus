/*******************************************************************************************
*  Program Name: ISpyUpdateDlg.cpp                                                                                                  
*  Description:  This is first Live Update UI which contains two options for updating the 
*				 product.
*  Author Name:  1)Ramkrushna
*				 2)Neha Gharge
*
*  Date Of Creation:15-0ct-2013                                                                                               
*  Version No:    1.0.0.2                                                                                                        *
*                                                                                                                                      *
*  Special Logic Used:                                                                                                                            *
*                                                                                                                                      *
*  Modification Log:                                                                                               
*  1. Modified xyz function in main        Date modified         CSR NO    *
*                                                                                                                                      *
*                                                                                                                                      *
*********************************************************************************************/ 

/*********************************************************************************************
//HEADER FILES
**********************************************************************************************/
#include "stdafx.h"
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"
#include "ISpyUpdatesDlg.h"
#include "WrdwizEncDecManager.h"

TCHAR	g_szDatabaseData[256] = {0} ;

IMPLEMENT_DYNAMIC(CISpyUpdatesDlg, CDialog)

/***********************************************************************************************
*  Function Name  : CISpyUpdatesDlg
*  Description    : C'tor
*  Author Name    : RamKrishna
*  SR_NO		  :
*  Date           : 15-Oct-2013
*************************************************************************************************/


CISpyUpdatesDlg::CISpyUpdatesDlg(CWnd* pParent /*=NULL*/)
: CJpegDialog(CISpyUpdatesDlg::IDD, pParent)
, m_csInputFolderPath("")
, m_csVersionNo("")
, m_iUpdateFailed(0)
, m_iSignatureFailed(0)
, m_iTotalNoofFiles(0)
, m_CurrentFilePath("")
, m_dwCurrentLocalFileSize(0)
, m_csFileName("")
, m_bOlderdatabase(false)
, m_bdatabaseuptodate(false)
, m_objIPCALUpdateClient(ALUPDATE)
, m_objCom(AUTOUPDATESRV_SERVER, true)
, m_bIsPopUpDisplayed(false)
, m_dwMaxVersionInZip(0)
{

}

/***********************************************************************************************                    
*  Function Name  : CISpyUpdatesDlg                                                     
*  Description    : Deconstrutor
*  Author Name    : RamKrishna 
*  SR_No		  :
*  Date           : 15-Oct-2013
*************************************************************************************************/

CISpyUpdatesDlg::~CISpyUpdatesDlg()
{
}

/***********************************************************************************************                    
*  Function Name  : DoDataExchange                                                     
*  Description    : Called by the framework to exchange and validate dialog data.
*  Author Name    : RamKrishna, Neha Gharge                                                                                        *
*  Date           : 16-Nov-2013
*************************************************************************************************/

void CISpyUpdatesDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_UPDATEFOLDERPATH, m_editFolderPath);
	DDX_Control(pDX, IDC_BUTTON_BROWSEBUTTON, m_btnBrowse);
	DDX_Control(pDX, IDC_RADIO_UPFROMINTERNET, m_btnFromInternet);
	DDX_Control(pDX, IDC_RADIO_FROMLOCALFOLDER, m_btnFromLocalComputer);
	DDX_Control(pDX, IDC_BUTTON_NEXT, m_btnNext);
	//DDX_Control(pDX, IDC_BUTTON_LIVEUPDATEHELP, m_btnHelp);
	//DDX_Control(pDX, IDC_UPDATE_HELP, m_btnHelp);
	//DDX_Control(pDX, IDC_UPADTE_CANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_STAT_UPDATE, m_stBtmText);
	DDX_Control(pDX, IDC_STAT_TOPUPDATE, m_stTopText);
	DDX_Control(pDX, IDC_STAT_HEADERPIC, m_HeaderPic);
	DDX_Control(pDX, IDC_STAT_INTERNET, m_stInternetText);
	DDX_Control(pDX, IDC_STAT_LOCAL_FOLDER, m_stLocalFolder);
	DDX_Control(pDX, IDC_STATIC_UPDATES_HEADER_MSG, m_stUpdatesHeader);
}

/***********************************************************************************************                    
*  Function Name  : MAPPING MESSAGES                                                    
*  Description    : Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  Author Name    : Neha Gharge   
*  SR_NO		  :
*  Date           : 16-Nov-2013
*************************************************************************************************/

BEGIN_MESSAGE_MAP(CISpyUpdatesDlg, CJpegDialog)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_RADIO_UPFROMINTERNET, &CISpyUpdatesDlg::OnBnClickedRadioFrominternet)
	ON_BN_CLICKED(IDC_RADIO_FROMLOCALFOLDER, &CISpyUpdatesDlg::OnBnClickedRadioFromlocalfolder)
	ON_BN_CLICKED(IDC_BUTTON_BROWSEBUTTON, &CISpyUpdatesDlg::OnBnClickedButtonBrowsebutton)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, &CISpyUpdatesDlg::OnBnClickedButtonNext)
	//ON_BN_CLICKED(IDC_BUTTON_LIVEUPDATEHELP, &CISpyUpdatesDlg::OnBnClickedButtonLiveupdatehelp)
	ON_WM_NCHITTEST()
	ON_BN_CLICKED(IDC_BACK_BUTTON, &CISpyUpdatesDlg::OnBnClickedBackButton)
	//ON_BN_CLICKED(IDC_UPDATE_HELP, &CISpyUpdatesDlg::OnBnClickedButtonHelp)
	//ON_BN_CLICKED(IDC_UPADTE_CANCEL, &CISpyUpdatesDlg::OnBnClickedUpadteCancel)
	ON_BN_CLICKED(IDC_UPDATE_BROWSE, &CISpyUpdatesDlg::OnBnClickedUpdateBrowse)
	ON_WM_SETCURSOR()
	ON_WM_PAINT()
END_MESSAGE_MAP()

/***************************************************************************************************                    
*  Function Name  : OnInitDialog                                                     
*  Description    : Windows calls the OnInitDialog function through the standard global 
					dialog-box procedure common to all Microsoft Foundation Class Library
					dialog boxes
*  Author Name    : Neha Gharge     
*  SR_NO		  :
*  Date           : 16-Nov-2013
****************************************************************************************************/
// CISpyUpdatesDlg message handlers
BOOL CISpyUpdatesDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window
	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_LIVEUPDATE_BKGRD), _T("JPG")))
	{
		m_bIsPopUpDisplayed = true;
		::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
		m_bIsPopUpDisplayed = false;
	}

	Draw();

	m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));

	CRect rect1;
	this->GetClientRect(rect1);
	SetWindowPos(NULL, 1, 88, rect1.Width()-5, rect1.Height() - 5, SWP_NOREDRAW);
	

	//m_stUpdatesHeader.SetTextAlign(TA_LEFT);
	//m_stUpdatesHeader.SetColor(RGB(100,100,100);
	//m_stUpdatesHeader.SetGradientColor(RGB(230,232,238));
	//m_stUpdatesHeader.SetVerticalGradient(1);
	//m_stUpdatesHeader.SetWindowPos(&wndTop,rect1.left +10,15,290,30,SWP_NOREDRAW);
	//m_stUpdatesHeader.SetFont(&theApp.m_fontWWTextSmallTitle);

	m_stUpdatesHeader.SetWindowPos(&wndTop,rect1.left +20,13,540,31,SWP_NOREDRAW);	
	m_stUpdatesHeader.SetTextColor(RGB(24,24,24));
	m_stUpdatesHeader.SetFont(&theApp.m_fontWWTextTitle);

	if(theApp.m_dwOSType == WINOS_WIN8 ||theApp.m_dwOSType == WINOS_WIN8_1)
	{
		m_stUpdatesHeader.SetWindowPos(&wndTop,rect1.left +20,16,540,31,SWP_NOREDRAW);	
	}

	m_bmpHeaderPicture = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_COM_HEADER));
	m_HeaderPic.SetBitmap(m_bmpHeaderPicture);
	m_HeaderPic.SetWindowPos(&wndTop,rect1.left+6,10,586,45,SWP_NOREDRAW);
	
	
	CFont *TextFont = new CFont; 
	TextFont->CreatePointFont(90,_T("Verdana"));
	
	
	m_stTopText.SetWindowPos(&wndTop,rect1.left+40,110,560, 40, SWP_NOREDRAW);
	m_stTopText.SetFont(&theApp.m_fontWWTextNormal);

	m_btnFromInternet.SetSkin(theApp.m_hResDLL,IDB_BITMAP_DISABLE,IDB_BITMAP_SELECT,IDB_BITMAP_DISABLE,IDB_BITMAP_DISABLE,0,0,0,0,0);
	m_btnFromInternet.SetTextColorA(BLACK,1,1);
	//issue - all radio button getting selected , if i drag and drop in radio button
	// resolve by  lalit kumawat 3-20-015
	m_btnFromInternet.SetWindowPos(&wndTop, rect1.left + 40, 150, 21, 20, SWP_NOZORDER);
	m_btnFromInternet.SetCheck(TRUE);
	m_updateType = UPDATEFROMINTERNT;
	m_stInternetText.SetWindowPos(&wndTop,rect1.left+65,153,380,20, SWP_NOREDRAW);
	m_stInternetText.SetFont(&theApp.m_fontWWTextNormal);

	m_btnFromLocalComputer.SetSkin(theApp.m_hResDLL,IDB_BITMAP_DISABLE,IDB_BITMAP_SELECT,IDB_BITMAP_DISABLE,IDB_BITMAP_DISABLE,0,0,0,0,0);
	m_btnFromLocalComputer.SetTextColorA(BLACK,1,1);
	//issue - all radio button getting selected , if i drag and drop in radio button
	// resolve by  lalit kumawat 3-20-015
	m_btnFromLocalComputer.SetWindowPos(&wndTop, rect1.left + 40, 180, 21, 20, SWP_NOZORDER);
	m_stLocalFolder.SetWindowPos(&wndTop,rect1.left+65,183,370,20, SWP_NOREDRAW);
	m_stLocalFolder.SetFont(&theApp.m_fontWWTextNormal);
	
	m_editFolderPath.SetWindowPos(&wndTop,rect1.left+40,220,319,21, SWP_NOREDRAW);

	m_btnBrowse.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_WHITE_BG,IDB_BITMAP_BTN_HOVER_WHITE_BG,IDB_BITMAP_BTN_DISABLE_WHITE_BG,0,0,0,0,0);
	m_btnBrowse.SetWindowPos(&wndTop,rect1.left+375,220,57,21, SWP_NOREDRAW);
	m_btnBrowse.SetTextColorA(BLACK,1,1);
	m_btnBrowse.SetFont(&theApp.m_fontWWTextNormal);

	m_stBtmText.SetWindowPos(&wndTop,rect1.left+40,280,600,20, SWP_NOREDRAW);
	m_stBtmText.SetFont(&theApp.m_fontWWTextNormal);

	//******************Issue not reported Neha Gharge 24/5/2014 *****************************/
	m_btnNext.SetSkin(theApp.m_hResDLL,IDB_BITMAP_BTN57x21,IDB_BITMAP_BTN57x21,IDB_BITMAP_57x21_H_over,IDB_BITMAP_57x21_DISABLE,0,0,0,0,0);
	m_btnNext.SetWindowPos(&wndTop,rect1.left+533 ,362,57,21, SWP_NOREDRAW);
	m_btnNext.SetTextColorA(BLACK,1,1);
	m_btnNext.SetFont(&theApp.m_fontWWTextNormal);

	m_dlgISpyUpdatesSecond.Create(IDD_DIALOG_LIVEUPDATE_SECONDPAGE, this);
	m_dlgISpyUpdatesSecond.ShowWindow(SW_HIDE);


	//m_btnNext.SetWindowPos(&wndTop,rect1.left+400 ,290, 70,30, SWP_SHOWWINDOW);

	m_btnFromInternet.SetCheck(TRUE);
	m_updateType = UPDATEFROMINTERNT;
	RefreshStrings();
	return TRUE;  // return TRUE unless you set the focus to a control

	memset(g_szDatabaseData, 0x00, 256*sizeof(TCHAR) ) ;
}

/***********************************************************************************************                    
*  Function Name  : OnCtlColor                                                     
*  Description    : The framework calls this member function when a child control is 
					about to be drawn.
*  Author Name    : Neha Gharge    
*  SR_NO		  :
*  Date           : 16-Oct-2013
*************************************************************************************************/
HBRUSH CISpyUpdatesDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	int	ctrlID;
	ctrlID = pWnd->GetDlgCtrlID();
	if( ctrlID == IDC_STAT_TOPUPDATE	     ||
		ctrlID == IDC_STAT_INTERNET			 ||
		ctrlID == IDC_STAT_LOCAL_FOLDER		 ||
		ctrlID == IDC_STAT_UPDATE			 ||
		ctrlID == IDC_RADIO_UPFROMINTERNET	 ||
		ctrlID == IDC_RADIO_FROMLOCALFOLDER	 ||
		ctrlID == IDC_STATIC_UPDATES_HEADER_MSG  )
		
	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
	}	return hbr;
}

/***********************************************************************************************                    
*  Function Name  : OnBnClickedRadioFrominternet                                                     
*  Description    : After clicking internet option,Disable editpath,browse button  
*  Author Name    : Neha Gharge,RamKrushna    
*  SR_NO		  :
*  Date           : 16-Nov-2013
*************************************************************************************************/
void CISpyUpdatesDlg::OnBnClickedRadioFrominternet()
{
	m_editFolderPath.SetWindowText(L"");
	m_btnFromLocalComputer.SetCheck(FALSE);
	m_updateType = UPDATEFROMINTERNT;
	EnableControls(FALSE);
}

/***********************************************************************************************                    
*  Function Name  : ResetControls                                                     
*  Description    : Resets all controls 
*  Author Name    : Neha Gharge,RamKrushna 
*  SR_NO		  :
*  Date           : 16-Nov-2013
*************************************************************************************************/
void CISpyUpdatesDlg::ResetControls()
{
	m_editFolderPath.SetWindowText(L"");
	m_btnFromInternet.SetCheck(true);
	m_btnFromLocalComputer.SetCheck(false);
	m_updateType = UPDATEFROMINTERNT;
	EnableControls(FALSE);
}

/***********************************************************************************************                    
*  Function Name  : OnBnClickedRadioFromlocalfolder                                                     
*  Description    : After clicking local folder..Enables editpath and browse option
					It is another of option for update,
					When user click on option it update from local folder.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 16-Nov-2013
*************************************************************************************************/
void CISpyUpdatesDlg::OnBnClickedRadioFromlocalfolder()
{
	m_editFolderPath.SetWindowText(L"");
	m_btnFromInternet.SetCheck(FALSE);
	m_updateType = UPDATEFROMLOCALFOLDER;
	EnableControls(TRUE);
	/*In updates->update from local folder->we cannot enter anything in to the box given->But the cursor is blinking with in the box.
	Niranjan Deshak - 27/02/2015*/
	m_editFolderPath.EnableWindow(false);
	m_csInputFolderPath = L"";
}

/***********************************************************************************************                    
*  Function Name  : EnableControls                                                     
*  Description    : Local folder functions enable the browsing option. Otherwise not
*  Author Name    : RamKrushna 
*  SR_NO		  :
*  Date           : 11-Nov-2013
*************************************************************************************************/
void CISpyUpdatesDlg::EnableControls(BOOL bEnable)
{
	m_editFolderPath.EnableWindow(bEnable);
	m_btnBrowse.EnableWindow(bEnable);
}

/***********************************************************************************************                    
*  Function Name  : OnBnClickedButtonBrowsebutton                                                     
*  Description    : It browses the files on a computer
*  Author Name    : RamKrushna 
*  SR_NO		  :
*  Date           : 15-oct-2013
*************************************************************************************************/
void CISpyUpdatesDlg::OnBnClickedButtonBrowsebutton()
{
	//Functionality change : New files added in Update from local folder
	//Implementated by : Nitin Kolapkar, Date: 16th Feb 2016
	CString csMessage = theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_SELECT_FOLDER");

	static TCHAR szFilter[] = L"Def Files(*.def)|*.def|";
	CFileDialog objFileDlg(true, L"All Files(*.*)",NULL,
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilter, this);
	CString csNewFilePath = L"";
	m_bIsPopUpDisplayed = true;
	if (objFileDlg.DoModal() == IDOK)
	{
		m_bIsPopUpDisplayed = false;
		csNewFilePath = objFileDlg.GetPathName();
		// if user give file name xxxx.exe instead of xxxx or xxxx.wwiz in such case
		//we have to manaually attach .WWIZ extension otherwise right click will not show decrypt option by right clicking on encrypted file.
	}
	else
	{
		m_bIsPopUpDisplayed = false;
		csNewFilePath = L"";
	}
	m_editFolderPath.SetWindowTextW(csNewFilePath);

	size_t iLen = wcslen(csNewFilePath);
		if(iLen > 0)
		{
			CString csTemp;
			csTemp = csNewFilePath;
			if(csTemp.Right(1) == L"\\")
			{
				csTemp = csTemp.Left(static_cast<int>(iLen) -1);
			}
			/*In updates->update from local folder->we cannot enter anything in to the box given->But the cursor is blinking with in the box.
			Niranjan Deshak - 27/02/2015*/
			EnableControls(TRUE);
			m_editFolderPath.SetWindowText(csTemp);
			m_csInputFolderPath = csTemp;
		}
}

/***********************************************************************************************                    
*  Function Name  : OnBnClickedButtonNext                                                     
*  Description    : After clicking Next button. It shows it's child dialog
*  Author Name    : RamKrushna,Neha Gharge  
*  SR_NO		  :
*  Date           : 15-oct-2013
*************************************************************************************************/
void CISpyUpdatesDlg::OnBnClickedButtonNext()
{
	/*	ISSUE NO - 669 NAME - NITIN K. TIME - 27th June 2014 */
	m_bOlderdatabase = false;
	m_bdatabaseuptodate = false;
	if(m_updateType == NONE)
	{
		m_bIsPopUpDisplayed = true;
		MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_LIVE_UPDATE_OPTION"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		m_bIsPopUpDisplayed = false;
		return;
	}

	//Ram:
	//Commented as because need to provide live update if product is un-registered.
	//if(theApp.m_dwDaysLeft == 0)
	//{
	//	if(!theApp.ShowEvaluationExpiredMsg())
	//	{
	//		theApp.GetDaysLeft();
	//		return;
	//	}
	//}


//Issue No. 601 Rajil Yadv 4/6/2014
	//if(m_updateType == UPDATEFROMINTERNT)
	//{
	//	if(!m_dlgISpyUpdatesSecond.CheckInternetConnection())
	//	{
	//		AddLogEntry(L"### Failed to connect to internet", 0, 0, true, SECONDLEVEL);
	//		//MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_INTERNET_CONNECTION"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
	//		return;
	//	}
	//}

	switch(m_updateType)
	{
	case UPDATEFROMINTERNT:
		m_dlgISpyUpdatesSecond.StartDownloading();
		StartALUpdateUsingALupdateService();
		//Sleep(400);
		ShowHideAllUpdateFirstPageControls(false);
		m_dlgISpyUpdatesSecond.ShowWindow(SW_SHOW);
		//m_dlgISpyUpdatesSecond.StartDownloading();
		break;
	case UPDATEFROMLOCALFOLDER:
		/*Issue No:1 Issue Desc: In Upadte 1st dialog, if we change the type of update and press Enter, the dialog becomes inactive, also in local update if we press enter after file selction the dialog becomes inactive.
		Resolved by :	Divya S..*/

		//Issue No.8 In Wardwiz Essential updates->Update from local folder,two "Browse for folder" appears one windows should be removed.
		//Gaurav Chopdar Date:1-1-2015 For this, 2 if blocks commented within same function
		//if(m_csInputFolderPath.Compare(L"") == 0)
		//{
		//	OnBnClickedButtonBrowsebutton();
         //}
		CString csInputFileName = m_csInputFolderPath;
		m_dwMaxVersionInZip = 0x00;
		//Rajil Yadav Issue No.670 12/06/2014

         if(m_editFolderPath.GetWindowTextLengthW() == NULL)
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_FOLDER_PATH"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
			m_bIsPopUpDisplayed = false;

			m_csInputFolderPath = L"";
			return;
		}
		//Issue No.8 In Wardwiz Essential updates->Update from local folder,two "Browse for folder" appears one windows should be removed.
		//Gaurav Chopdar Date:1-1-2015 
       //if(csInputFolder.GetLength() == 0)
      //{
     //    MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_FOLDER_PATH"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION|MB_OK);
       //  return;
	  //}


		 if (!PathFileExists(csInputFileName))
		{
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_FOLDER_NO_PATH"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION|MB_OK);
			m_bIsPopUpDisplayed = false;
			return;
		}

		 DWORD dwUnzipCount = 0;
		 csInputFileName = ExtractRARFile(csInputFileName, dwUnzipCount);
		 if (dwUnzipCount == 0)
		 {
			 m_bIsPopUpDisplayed = true;
			 MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_DATABASE_INVALID_MSG1"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			 m_bIsPopUpDisplayed = false;
			 m_editFolderPath.SetWindowTextW(L"");
			 break;
		 }

		std::vector<CString> csVectInputFiles;
		if (!CheckForValidUpdatedFiles(csInputFileName, csVectInputFiles))
		{
			if(m_bOlderdatabase)
			{
				m_bOlderdatabase = false;
				EnumAndDeleteTempFolder(csInputFileName);
				m_bIsPopUpDisplayed = true;
				//Varada Ikhar, Date: 26/02/2015, Issue:If we update database locally, after completion the pop-up should have punctuation(FullStop).
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_DB_FILES_ARE_OLDER"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
				m_bIsPopUpDisplayed = false;

				return;
			}
			else if(m_bdatabaseuptodate)
			{
				m_bdatabaseuptodate = false;
				EnumAndDeleteTempFolder(csInputFileName);
				m_bIsPopUpDisplayed = true;
				//Varada Ikhar, Date: 26/02/2015, Issue:If we update database locally, after completion the pop-up should have punctuation(FullStop).
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDC_STATIC_UPDATED_DATABASE"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION | MB_OK);
				m_bIsPopUpDisplayed = false;

				return;
			}
			else
			{
				EnumAndDeleteTempFolder(csInputFileName);
				m_bIsPopUpDisplayed = true;
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_DATABASE_INVALID_MSG1"),theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"),MB_ICONEXCLAMATION|MB_OK);
				m_bIsPopUpDisplayed = false;
			}
			return;
		}
		
		int dwUpdateLocalRet = m_dlgISpyUpdatesSecond.UpdateFromLocalFolder(csVectInputFiles);
		EnumAndDeleteTempFolder(csInputFileName);
		if(dwUpdateLocalRet == 0x01)
		{
			//if(m_bOlderdatabase)
			//{
			//	m_dlgISpyUpdatesSecond.m_bisDateTime = true;
			//	m_dlgISpyUpdatesSecond.UpdateTimeDate();
			//	m_bOlderdatabase = false;
			//	MessageBox(L"Some of files are older than current version.\n\nPartial database updated successfully", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION|MB_OK);
			//	return;
			//}
			//else
			//{
			if (!UpdateVersionIntoRegistry())
			{
				AddLogEntry(L"### Failed to update database version into registry", 0, 0, true, SECONDLEVEL);
			}
			m_dlgISpyUpdatesSecond.m_bisDateTime = true;
			m_dlgISpyUpdatesSecond.UpdateTimeDate();

			m_bIsPopUpDisplayed = true;
			//Varada Ikhar, Date: 26/02/2015, Issue:If we update database locally, after completion the pop-up should have punctuation(FullStop).
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_DB_UPDATE_SUCCESS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION | MB_OK);
			m_bIsPopUpDisplayed = false;

			return;
			//}
		}
		if(dwUpdateLocalRet == 0x03)
		{
			if(m_bOlderdatabase)
			{
				m_bOlderdatabase = false;

				m_bIsPopUpDisplayed = true;
				//Varada Ikhar, Date: 26/02/2015, Issue:If we update database locally, after completion the pop-up should have punctuation(FullStop).
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_DB_FILES_ARE_OLDER"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
				m_bIsPopUpDisplayed = false;

				return;
			}
			else
			{
				m_bIsPopUpDisplayed = true;
				MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_DATABASE"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION|MB_OK);
				m_bIsPopUpDisplayed = false;
				return;
			}
		}
		if(dwUpdateLocalRet == 0x02)
		{
			if (!UpdateVersionIntoRegistry())
			{
				AddLogEntry(L"### Failed to update database version into registry", 0, 0, true, SECONDLEVEL);
			}
			m_dlgISpyUpdatesSecond.m_bisDateTime = true;
			m_dlgISpyUpdatesSecond.UpdateTimeDate();

			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_UPDATE_DATABASE_SUCCESS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
			m_bIsPopUpDisplayed = false;
		}
			
		m_editFolderPath.SetWindowTextW(L"");

		break;
	}
}

/***********************************************************************************************                    
*  Function Name  : ShowHideAllUpdateFirstPageControls                                                     
*  Description    : It shows or hides the dialog component as per requirements.
*  Author Name    : RamKrushna,Neha Gharge
*  SR_NO		  :
*  Date           : 16-Nov-2013
*************************************************************************************************/
void CISpyUpdatesDlg::ShowHideAllUpdateFirstPageControls(bool bEnable)
{
	//Varada Ikhar, Date: 30/04/2015
	//Issue : If product update is in progress, and if tried to exit from tray, debug assertion fail error occur.
	try
	{
		if (theApp.m_bOnCloseFromMainUI == false)
		{
			m_HeaderPic.ShowWindow(bEnable);
			m_stBtmText.ShowWindow(bEnable);
			m_stTopText.ShowWindow(bEnable);
			m_stLocalFolder.ShowWindow(bEnable);
			m_stInternetText.ShowWindow(bEnable);
			m_editFolderPath.ShowWindow(bEnable);
			m_btnBrowse.ShowWindow(bEnable);
			m_btnFromInternet.ShowWindow(bEnable);
			m_btnFromLocalComputer.ShowWindow(bEnable);
			m_btnNext.ShowWindow(bEnable);
			m_stUpdatesHeader.ShowWindow(bEnable);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CNextGenExUpdatesDlg::ShowHideAllUpdateFirstPageControls", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************                    
*  Function Name  :  OnNcHitTest                                                     
*  Description    :  The framework calls this member function for the CWnd object that 
					 contains the cursor every time the mouse is moved.
*  Author Name    :  Neha Gharge
*  SR_NO		  :
*  Date           :  16-Nov-2013
*************************************************************************************************/
LRESULT CISpyUpdatesDlg::OnNcHitTest(CPoint point)
{
	return HTCLIENT;
}

/***********************************************************************************************                    
*  Function Name  : OnBnClickedBackButton                                                     
*  Description    : After clicking back button,it goes to first(parent) dialog
*  Author Name    : RamKrushna 
*  SR_NO		  :
*  Date           : 16-Nov-2013
*************************************************************************************************/
void CISpyUpdatesDlg::OnBnClickedBackButton()
{
	this->ShowWindow(SW_HIDE);
	//CISpyGUIDlg *pObjMainUI = reinterpret_cast<CISpyGUIDlg*>( this->GetParent() );
	//pObjMainUI->ShowHideMainPageControls(true);
}

/***********************************************************************************************                    
*  Function Name  : OnBnClickedUpdateBrowse                                                     
*  Description    : Function browses files on the computer.
*  Author Name    : RamKrushna
*  SR_NO		  :
*  Date           : 25-Oct-2013
*************************************************************************************************/
void CISpyUpdatesDlg::OnBnClickedUpdateBrowse()
{
		// TODO: Add your control notification handler code here
	BROWSEINFO oBrowseInfo = { 0 };
	oBrowseInfo.hwndOwner = NULL;
	oBrowseInfo.ulFlags = BIF_RETURNONLYFSDIRS ;
	oBrowseInfo.pidlRoot = 0;
	oBrowseInfo.lpszTitle =  _T("Select folder where update files are located");

	LPITEMIDLIST pIDL = SHBrowseForFolder(&oBrowseInfo);

	WCHAR pszPath[_MAX_PATH]= {'\0'};
	SHGetPathFromIDList(pIDL,pszPath);
	LPWSTR path = pszPath;
	m_editFolderPath.SetWindowTextW(path);
}

/***********************************************************************************************                    
*  Function Name  : CheckForValidUpdatedFiles                                                     
*  Description    : Checks files in folder are valid or not
*  Author Name    : RamKrushna
*  SR_NO		  :
*  Date           : 15-Oct-2013
*************************************************************************************************/
bool CISpyUpdatesDlg::CheckForValidUpdatedFiles(CString csInputFolder, std::vector<CString> &csVectInputFiles)
{
	bool bReturn = false;
	int iMatchCount = 0;
	int iMisMatchedCount = 0;

	//Issue no 1318: If directory not present it will create directory then add files into it.
	TCHAR szDataBaseFolder[MAX_PATH] = { 0 };

	if (theApp.m_eScanLevel == CLAMSCANNER)
	{
		_stprintf_s(szDataBaseFolder, MAX_PATH, L"%s\\%s", GetAppFolderPath(), L"DB");
		if (!PathFileExists(szDataBaseFolder))
		{
			if (CreateDirectoryFocefully(szDataBaseFolder))
			{
				AddLogEntry(L"### Failed to create & Check dest folder path :: %s", szDataBaseFolder, 0, true, SECONDLEVEL);
			}
		}

		_stprintf_s(szDataBaseFolder, MAX_PATH, L"%s\\%s", GetAppFolderPath(), L"WRDWIZDATABASE");
		if (!PathFileExists(szDataBaseFolder))
		{
			if (CreateDirectoryFocefully(szDataBaseFolder))
			{
				AddLogEntry(L"### Failed to create & Check dest folder path :: %s", szDataBaseFolder, 0, true, SECONDLEVEL);
			}
		}
	}
	else
	{
		_stprintf_s(szDataBaseFolder, MAX_PATH, L"%s\\%s", GetAppFolderPath(), L"WRDWIZDATABASE");
		if (!PathFileExists(szDataBaseFolder))
		{
			if (CreateDirectoryFocefully(szDataBaseFolder))
			{
				AddLogEntry(L"### Failed to create & Check dest folder path :: %s", szDataBaseFolder, 0, true, SECONDLEVEL);
			}
		}
	}

	if(m_csFilesList.GetCount() > 0)
	{
		m_csFilesList.RemoveAll();
	}
	//This change only work when input files from server are with this name
	//CStringArray csFilesList;
	//csFilesList.Add(L"WRDWIZAV1.db");
	//csFilesList.Add(L"WRDWIZAV2.db");
	//csFilesList.Add(L"WRDWIZAV3.db");
	//csFilesList.Add(L"WRDWIZAV4.db");
	EnumFolder(csInputFolder);

	for(int iIndex = 0; iIndex < m_csFilesList.GetCount(); iIndex++)
	{
		CString csInputPathProgramData = NULL;
		CString csInputPath = csInputFolder + L"\\" + m_csFilesList[iIndex];
		if(PathFileExists(csInputPath))
		{
			DWORD	dwStatus = 0x00 ;
			dwStatus = ValidateDB_File((LPTSTR)csInputPath.GetBuffer(), dwStatus, csInputPathProgramData);
			if(dwStatus == 0x08)
			{
				iMisMatchedCount++;
			}
			if(dwStatus == 0x00)
			{
				csInputPath = csInputPathProgramData;
				csVectInputFiles.push_back(csInputPath);
				iMatchCount++;
				/*if(!UpdateVersionIntoRegistry())
				{
					AddLogEntry(L"### Failed to update database version into registry", 0, 0, true, SECONDLEVEL);
				}*/
			}
			
		}
	}

	if(iMatchCount >= 1)
	{
		bReturn = true;
	}
	else
	{
		bReturn = false;
	}
	return bReturn;
}
	
/***********************************************************************************************                    
*  Function Name  : ValidateDB_File                                                     
*  Description    : checking signature and valid version no and size of files
*  Author Name    : Nitin k 
*  SR_NO		  :
*  Date           : 16-Feb-2014
*  Modifiled date :	6/6/2014 neha gharge
//Modified: 
//Issue No 107	Doesn’t check the contents of the DB file. Even if DB contains garbage its accepted
//ITin_Developement_Release_1.0.0.3_Patches
*  Modification Date : 6 Jan 2015 Neha Gharge
*  MOdification		 : Clam And WardWiz Scanner Handle by preprocessor
*  Modified Date  : 6/2/2015 Neha Gharge FP file added
*************************************************************************************************/
DWORD CISpyUpdatesDlg::ValidateDB_File( TCHAR *m_szFilePath, DWORD &dwStatus ,CString &csInputPathProgramData)
{

	DWORD	dwRet = 0x00 ;
	DWORD	dwFileSize = 0x00, dwBytesRead = 0x00, dwDecBytesRead=0x00;
	TCHAR	szTemp[1024] = {0} , szFileName[1024] = {0} ;
	TCHAR	szExt[16] = {0} ;
	DWORD	dwLen = 0x00 ;
	LPBYTE	lpFileData = NULL ;
	LPBYTE	lpEncryptionSignature = (LPBYTE)"WRDWIZDB";
	DWORD	dwDecKeySize = 0x00;
	HANDLE	hFile = INVALID_HANDLE_VALUE ;
	HANDLE	hFileEnc = INVALID_HANDLE_VALUE ;
	TCHAR   szWholeSignature[0x30]={0};
	DWORD   dwRetCheckversion = 0x00;
	CString csFileName, csActualWRDWIZFilePath, csActualClamFilePath;
	int j = 0;

	//__try
	try
	{
		if (!m_szFilePath)
			return 0x05;
		//Issue 1318 : if file are not exist in app directory. It should be update.
		m_CurrentFilePath = L"";
		m_CurrentFilePath.Format(L"%s",m_szFilePath);
		csFileName = m_CurrentFilePath.Mid(m_CurrentFilePath.ReverseFind('\\') + 1);
	

		if (theApp.m_eScanLevel == CLAMSCANNER)
		{
			if ((csFileName.CompareNoCase(L"DAILY.CLD") == 0) || csFileName.CompareNoCase(L"MAIN.CVD") == 0 || csFileName.CompareNoCase(L"WRDWIZWHLST.FP") == 0)
			{
				csActualClamFilePath.Format(L"%s\\%s\\%s", GetAppFolderPath(), L"DB", csFileName);
				if (!PathFileExists(csActualClamFilePath))
				{
					AddLogEntry(L"### %s File doesn't exist", 0, 0, true, FIRSTLEVEL);
					csInputPathProgramData = m_szFilePath;
					dwRet = 0x00;
					goto Cleanup;
				}
			}
			else
			{
				csActualWRDWIZFilePath.Format(L"%s\\%s\\%s", GetAppFolderPath(), L"WRDWIZDATABASE", csFileName);
				if (!PathFileExists(csActualWRDWIZFilePath))
				{
					AddLogEntry(L"### %s File doesn't exist", 0, 0, true, FIRSTLEVEL);
					csInputPathProgramData = m_szFilePath;
					dwRet = 0x00;
					goto Cleanup;
				}
			}
		}
		else
		{
			//Issue fix: 1346 Offline DB always showing 'Database updated successfully'
			//Resolved by: Nitin K. Date: 7th March 2016
			if ((csFileName.CompareNoCase(L"DAILY.CLD") == 0) || csFileName.CompareNoCase(L"MAIN.CVD") == 0 || csFileName.CompareNoCase(L"WRDWIZWHLST.FP") == 0)
			{
				dwRet = 0x08;
				goto Cleanup;
			}
			csActualWRDWIZFilePath.Format(L"%s\\%s\\%s", GetAppFolderPath(), L"WRDWIZDATABASE", csFileName);
			if (!PathFileExists(csActualWRDWIZFilePath))
			{
				AddLogEntry(L"### %s File doesn't exist", 0, 0, true, FIRSTLEVEL);
				csInputPathProgramData = m_szFilePath;
				dwRet = 0x00;
				goto Cleanup;
			}
		}

		if( !PathFileExists( m_szFilePath ) )
		{
			//AfxMessageBox( TEXT("Please select file for operation") ) ;
			dwRet = 0x01 ;
			goto Cleanup ;
		}

		dwLen = static_cast<DWORD>(wcslen(m_szFilePath));
		if( (dwLen < 0x08) || (dwLen > 0x400) )
		{
			//AfxMessageBox( TEXT("Please select file for operation") ) ;
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		//memcpy(szExt, &m_szFilePath[dwLen-0x03], 0x0A ) ;
		//if( wcsicmp(szExt, TEXT(".DB") ) != 0x00 )
		//{
		//	//AfxMessageBox( TEXT("Please select file for operation") ) ;
		//	dwRet = 0x03 ;
		//	goto Cleanup ;
		//}

		DWORD dwDBVersionLength = 0;
		DWORD dwDBMajorVersion = 0;
		CWrdwizEncDecManager objWrdwizEncDecMgr;
		if (!objWrdwizEncDecMgr.IsFileAlreadyEncrypted(m_szFilePath, dwDBVersionLength, dwDBMajorVersion))
		{
			AddLogEntry(L"### GenX Signature criteria not matched");
			dwRet = 0x08;		//Invalid files
			goto Cleanup ;
		}

		dwDecKeySize = MAX_SIG_SIZE + dwDBVersionLength + MAX_TOKENSTRINGLEN;

		hFile = CreateFile(	m_szFilePath, GENERIC_READ|GENERIC_WRITE, 0, NULL,
								OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		if( hFile == INVALID_HANDLE_VALUE )
		{
			AddLogEntry(L"### Updates : Error in opening existing Database file %s",m_szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x02 ;
			goto Cleanup ;
		}

		dwFileSize = GetFileSize( hFile, NULL ) ;
		if( !dwFileSize )
		{
			AddLogEntry(L"### Updates : Error in GetFileSize of %s",m_szFilePath, 0, true, SECONDLEVEL);
			dwRet = 0x03 ;
			goto Cleanup ;
		}

		m_dwCurrentLocalFileSize = dwFileSize;

		if(lpFileData != NULL)
		{
			free( lpFileData ) ;
			lpFileData = NULL ;
		}

		dwBytesRead = 0x00;
		unsigned char bySigBuff[0x30] = {0x00};

		SetFilePointer(hFile, 0x00, NULL, FILE_BEGIN);
		ReadFile( hFile, &bySigBuff[0x0], 0x30, &dwBytesRead, NULL ) ;
		if(dwBytesRead != 0x30)
		{
		  AddLogEntry(L"### Updates : Error in ReadFile while reading signature %s",m_szFilePath, 0, true, SECONDLEVEL);
		  dwRet =0x04;
		  goto Cleanup;
		}
		
		char szFileName[0x11] = {0};
		memcpy(szFileName, &bySigBuff[0], 0x0A);

		TCHAR szDBFileName[0x11] = {0};

		//convert multibyte string to unicode
		size_t convertedChars;
		mbstowcs_s(&convertedChars, szDBFileName, strlen(szFileName)+1, szFileName, _TRUNCATE);

		char *szVersionNumber = new char[dwDBVersionLength + 1];
		memset(szVersionNumber, 0, dwDBVersionLength + 1);
		memcpy(szVersionNumber, &bySigBuff[0x0B], dwDBVersionLength);
		//szVersionNumber[strlen(szVersionNumber) + 1] = '\0';

		TCHAR * szDBVersionNumber = new TCHAR[dwDBVersionLength + 1];
		mbstowcs_s(&convertedChars, szDBVersionNumber, strlen(szVersionNumber) + 1, szVersionNumber, _TRUNCATE);

		if(!ValidateFileNVersion(szDBFileName, szDBVersionNumber, dwDBVersionLength))
		{
			AddLogEntry(L"### Updates : Invalidate File", 0, 0, true, SECONDLEVEL);
			dwRet = 0x08;
			goto Cleanup;
		}

		if(szVersionNumber != NULL)
		{
			delete []szVersionNumber;
			szVersionNumber = NULL;
		}

		if(szDBVersionNumber != NULL)
		{
			delete []szDBVersionNumber;
			szDBVersionNumber = NULL;
		}

		lpFileData = (LPBYTE ) malloc( dwFileSize - dwDecKeySize ) ;
		if( !lpFileData )
		{
			AddLogEntry(L"### Updates : Error in allocation of memory", 0, 0, true, SECONDLEVEL);
			dwRet = 0x04 ;
			goto Cleanup ;
		}

		memset(lpFileData, 0x00, (dwFileSize - dwDecKeySize) ) ;
		SetFilePointer( hFile, (0x00 + dwDecKeySize), NULL, FILE_BEGIN ) ;
		ReadFile( hFile, lpFileData,( dwFileSize - dwDecKeySize ), &dwBytesRead, NULL ) ;

		if( (dwFileSize - dwDecKeySize) != dwBytesRead )
		{
			AddLogEntry(L"### Updates : Error in ReadFile %s",m_szFilePath, 0, true, FIRSTLEVEL);
			dwRet = 0x04 ;
			goto Cleanup ;
		}
		

		if( hFile != INVALID_HANDLE_VALUE )
		{
			CloseHandle( hFile ) ;
			hFile = INVALID_HANDLE_VALUE ;
		}

		CString csScanLogFullPath;
		TCHAR szModulePath[MAX_PATH] = {0};
		memset(szModulePath, 0x00, MAX_PATH * sizeof(TCHAR) ) ;
		GetEnvironmentVariable(L"ALLUSERSPROFILE", szModulePath, MAX_PATH ) ;

		csScanLogFullPath = szModulePath;
		csScanLogFullPath += L"\\Wardwiz Antivirus";
		CString FileName = L"";

		//Functionality change : New files added in Update from local folder
		//Implementated by : Nitin Kolapkar, Date: 16th Feb 2016




		csInputPathProgramData = m_szFilePath;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CGenXUpdatesDlg::Database_File_Updation", 0, 0, true, SECONDLEVEL);
		return false;
	}
Cleanup :

	if( hFile != INVALID_HANDLE_VALUE )
	{
		CloseHandle( hFile ) ;
		hFile = INVALID_HANDLE_VALUE ;
	}


	if( lpFileData )
		free( lpFileData ) ;
	lpFileData = NULL ;


	return dwRet ;
}






/***********************************************************************************************                    
*  Function Name  : OnSetCursor                                                     
*  Description    : The framework calls this member function if mouse input is not
					captured and the mouse causes cursor movement within the CWnd object..
*  Author Name    : Neha Gharge  
*  SR_NO		  :
*  Date           : 16-Nov-2013
*************************************************************************************************/
BOOL CISpyUpdatesDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if( 
		iCtrlID == IDC_BUTTON_NEXT			||
		iCtrlID == IDC_BACK_BUTTON			||
		iCtrlID == IDC_BUTTON_BROWSEBUTTON	||
		iCtrlID == IDC_RADIO_UPFROMINTERNET	||
		iCtrlID == IDC_RADIO_UPFROMINTERNET	||
		iCtrlID == IDC_BUTTON_NEXT			||
		iCtrlID == IDC_BUTTON_LIVEUPDATEHELP||
		iCtrlID == IDC_RADIO_FROMLOCALFOLDER)
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
*  Function Name  : ShowOnlyFirstWindow                                                     
*  Description    : It shows first child window
*  Author Name    : RamKrushna 
*  SR_NO		  :
*  Date           : 25-Nov-2013
*************************************************************************************************/
void CISpyUpdatesDlg::ShowOnlyFirstWindow()
{
	m_dlgISpyUpdatesSecond.ShowWindow(SW_HIDE);
	ShowHideAllUpdateFirstPageControls(true);		
	ResetControls();
}

/***********************************************************************************************                    
*  Function Name  : RefreshStrings                                                     
*  Description    : this function is  called for setting the Text UI with different Language Support
*  Author Name    : Prassana
*  SR_NO		  :
*  Date           : 13-Jun-2014
*************************************************************************************************/
void CISpyUpdatesDlg::RefreshStrings()
{
	m_btnNext.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_NEXT"));
	m_btnBrowse.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_BROWSE"));
	m_stUpdatesHeader.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_HEADER"));
	m_stTopText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_TOP_TEXT"));
	m_stBtmText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BOTTOM_TEXT"));
	m_stInternetText.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_INTERNET_TEXT"));
	m_stLocalFolder.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_LOCAL_FOLDER_TEXT"));
}

/***********************************************************************************************                    
*  Function Name  : ShowPageToDisplay                                                     
*  Description    : If downloading is in progress, Second dialog to be disaplyed.
*  Author Name    : Prassana
*  SR_NO		  :
*  Date           : 13-Jun-2014
*************************************************************************************************/
void CISpyUpdatesDlg::ShowPageToDisplay()
{
	if(m_dlgISpyUpdatesSecond.m_isDownloading == true)
	{
		ShowHideAllUpdateFirstPageControls(false);
		m_dlgISpyUpdatesSecond.ShowWindow(SW_SHOW);			
	}
	else
	{
		ShowOnlyFirstWindow();
	}
}


/***********************************************************************************************                    
*  Function Name  : CheckForValidVersion                                                     
*  Description    : checking  valid version no 
*  Author Name    : neha gharge 
*  SR_NO		  :
*  Date           : 6- June -2014
*************************************************************************************************/
DWORD CISpyUpdatesDlg::CheckForValidVersion(CString csVersionNo)
{
	TCHAR szVersionNo[10] = {0};
	TCHAR szVersion[5] = {0};
	DWORD dwVersionNoInRegistry = 0;
	DWORD dwVersionNo = 0;
	DWORD dwRet = 0x00;

	int j = 0; 
	
	_tcscpy_s(szVersionNo,csVersionNo);

	for(int i = 0; i<9;i++)
	{
		if(isdigit(szVersionNo[i]))
		{
			szVersion[j] = szVersionNo[i];
			j++;
		}
	}

	//From TCHAR to DWORD.
  
	dwVersionNo = static_cast<DWORD>(wcstod(szVersion, _T('\0')));

   dwVersionNoInRegistry = ReadDBVersionFromReg();

   if(dwVersionNoInRegistry > dwVersionNo)
   {
		dwRet =  0x07;
		goto Cleanup;
   }
   if(dwVersionNoInRegistry < dwVersionNo)
   {
	   dwRet =  0x08;
	   goto Cleanup;
   }
   if(dwVersionNoInRegistry == dwVersionNo)
   {
	   dwRet =  0x09;
	   goto Cleanup;
   }


Cleanup : return dwRet;
 
}

/***********************************************************************************************                    
*  Function Name  : ReadDBVersionFromReg                                                     
*  Description    : ReadVersion from registry
*  Author Name    : neha gharge 
*  SR_NO		  :
*  Date           : 6- June -2014
*************************************************************************************************/

DWORD CISpyUpdatesDlg::ReadDBVersionFromReg()
{
	DWORD dwRegVersionNo = 0x00;
	TCHAR szRegVersionNo[1024] = {0};
	TCHAR szRegVersion[5] = {0};
	DWORD dwvalue_length = 1024;
	DWORD dwtype=REG_SZ;
	HKEY key;
	int j=0;
	if(RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("Software\\WardWiz Antivirus"),&key)!= ERROR_SUCCESS)
	{
		//AddLogEntry(L"Unable to open registry key");
	}

	long ReadReg=RegQueryValueEx(key, L"DataBaseVersion", NULL ,&dwtype,(LPBYTE)&szRegVersionNo, &dwvalue_length);

	if(ReadReg == ERROR_SUCCESS)
	{
		for(int i = 0; i < 9;i++)
		{
			if(isdigit(szRegVersionNo[i]))
			{
				szRegVersion[j] = szRegVersionNo[i];
				j++;
			}
		}
	
	}
	dwRegVersionNo = static_cast<DWORD>(wcstod(szRegVersion, _T('\0')));
	RegCloseKey(key);

	return dwRegVersionNo;
}

/***********************************************************************************************                    
*  Function Name  : UpdateVersionIntoRegistry                                                     
*  Description    : Upadte Version from registry
*  Author Name    : neha gharge   
*  SR_NO		  :
*  Date           : 6- June -2014
*************************************************************************************************/

bool CISpyUpdatesDlg::UpdateVersionIntoRegistry()
{
	CString csVersionregValue = L"";
	
	csVersionregValue = m_csVersionNo;
	csVersionregValue.Trim();

	if(!m_dlgISpyUpdatesSecond.SendRegistryData2Service(SZ_STRING, _T("SOFTWARE\\Wardwiz Antivirus"), 
		_T("DataBaseVersion"), (LPTSTR)csVersionregValue.GetBuffer(), true))
	{
		AddLogEntry(L"### Failed to DataBaseVersion SendRegistryData2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}


/***********************************************************************************************                    
*  Function Name  : CheckForFileSize                                                     
*  Description    : check file size of both DB
*  Author Name    : neha gharge  
*  SR_NO		  :
*  Date           : 6- June -2014
*  Modified Date  : 6/2/2015 Neha Gharge FP file added
*************************************************************************************************/
DWORD CISpyUpdatesDlg::CheckForFileSize(CString csFilename, DWORD dwDBVersionLength)
{
	DWORD dwLocalFilesSize = 0;
	DWORD dwRet = 0x00;

	if(m_updateType == UPDATEFROMLOCALFOLDER)
	{
		theApp.GetTotalExistingBytes();
		dwLocalFilesSize = m_dwCurrentLocalFileSize - ( MAX_SIG_SIZE + dwDBVersionLength + MAX_TOKENSTRINGLEN) ;

		if(csFilename == L"WRDWIZDB02")
		{
			if(dwLocalFilesSize < theApp.m_ExistingBytes[0])
			{
				dwRet = 0x01;
				goto Cleanup;
			}
			else if(dwLocalFilesSize == theApp.m_ExistingBytes[0])
			{
				dwRet = 0x02;
				goto Cleanup;
			}
			else
			{
				dwRet = 0x00;
				goto Cleanup;
			}

		}
		else if(csFilename == L"WRDWIZDB01")
		{
			if(dwLocalFilesSize < theApp.m_ExistingBytes[1])
			{
				dwRet = 0x01;
				goto Cleanup;
			}
			else if(dwLocalFilesSize == theApp.m_ExistingBytes[1])
			{
				dwRet = 0x02;
				goto Cleanup;
			}
			else
			{
				dwRet = 0x00;
				goto Cleanup;
			}
		}
		else if(csFilename == L"WRDWIZDB03")
		{
			dwLocalFilesSize = m_dwCurrentLocalFileSize;
			if(dwLocalFilesSize < theApp.m_ExistingBytes[2])
			{
				dwRet = 0x01;
				goto Cleanup;
			}
			else if(dwLocalFilesSize == theApp.m_ExistingBytes[2])
			{
				dwRet = 0x02;
				goto Cleanup;
			}
			else
			{
				dwRet = 0x00;
				goto Cleanup;
			}
		}
		else if(csFilename == L"WRDWIZDBR0")
		{
			dwLocalFilesSize = m_dwCurrentLocalFileSize;
			if(dwLocalFilesSize < theApp.m_ExistingBytes[3])
			{
				dwRet = 0x01;
				goto Cleanup;
			}
			else if(dwLocalFilesSize == theApp.m_ExistingBytes[3])
			{
				dwRet = 0x02;
				goto Cleanup;
			}
			else
			{
				dwRet = 0x00;
				goto Cleanup;
			}
		}
		else if(csFilename == L"WRDWIZDB04")
		{
			if(dwLocalFilesSize < theApp.m_ExistingBytes[4])
			{
				dwRet = 0x01;
				goto Cleanup;
			}
			else if(dwLocalFilesSize == theApp.m_ExistingBytes[4])
			{
				dwRet = 0x02;
				goto Cleanup;
			}
			else
			{
				dwRet = 0x00;
				goto Cleanup;
			}
		}
		else
		{
			dwRet = 0x01;
			goto Cleanup;				
		}
	}
Cleanup:	
	return dwRet;
}


/***********************************************************************************************                    
*  Function Name  : ValidateFileNVersion                                                     
*  Description    : checks for validate signature
*  Author Name    : neha gharge  
*  SR_NO		  :
*  Date           : 6- June -2014
*************************************************************************************************/
bool CISpyUpdatesDlg::ValidateFileNVersion(CString csFileName, CString csVersion, DWORD dwDBVersionLength)
{
	bool bReturn = false;

	
	m_csFileName = csFileName;

	int iRet = CheckForValidVersion(csVersion);
	if(iRet == 0x07)
	{
		m_bdatabaseuptodate = true;
		return false;
	}

	if(iRet == 0x08)
	{
		if (CheckForMaxVersionInZip(csVersion))
		{
			m_csVersionNo = csVersion;
		}
		return true;
	}

	if(iRet == 0x09)
	{
		m_bdatabaseuptodate = true;
		return false;
	}

	return true;
}

/***********************************************************************************************                    
*  Function Name  : EnumFolder                                                     
*  Description    : Enumrate all the files in folder.
*  Author Name    : neha gharge  
*  SR_NO		  :
*  Date           : 6- June -2014
*************************************************************************************************/
void CISpyUpdatesDlg::EnumFolder(LPCTSTR pstr)
{
	try
	{
		CFileFind finder;
		int i =0 ;
		m_iTotalNoofFiles = 0;
		// build a string with wildcards
		CString strWildcard(pstr);
		strWildcard += _T("\\*.*");

		// start working for files
		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			// if it's a directory, recursively search it 
			if (finder.IsDirectory())
			{
				//i++;
				//CString str = finder.GetFilePath();
				//if(i>=1)
				//{
				//	bWorking = false;
					continue;
				//}
			}
			else
			{
				m_csFilesList.Add(finder.GetFileName());
				m_iTotalNoofFiles++;
			}
		}
		
		finder.Close();
		i = 0;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CNextGenExUpdatesDlg::EnumFolder", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************                    
*  Function Name  : OnPaint                                                     
*  Description    :	The framework calls this member function when Windows or an application makes a request to repaint a portion of an application's window.
*  Author Name    : neha gharge  
*  SR_NO		  :
*  Date           : 6- June -2014
*************************************************************************************************/
void CISpyUpdatesDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CJpegDialog::Draw();
	CJpegDialog::OnPaint();
}


/***********************************************************************************************                    
*  Function Name  : StartALUpdateUsingALupdateService                                                     
*  Description    : Send a request to start Autp live update to ALU service through named pipe
*  Author Name    : neha gharge  
*  SR_NO		  :
*  Date           : 23- July -2014
*************************************************************************************************/
bool CISpyUpdatesDlg::StartALUpdateUsingALupdateService()
{
	try
	{
		ISPY_PIPE_DATA szPipeData = {0};
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = START_UPDATE;

		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CGenXUpdatesDlg::StartALUpdateUsingALupdateService", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CRegistryOptimizerDlg::ShutDownScanning", 0, 0, true, SECONDLEVEL);
		}

		if (!&szPipeData.dwValue)
		{
			return false;
		}

		OutputDebugString(L">>> Out CISpyUpdatesDlg::StartALUpdateUsingALupdateService");
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CGenXUpdatesDlg::StartALUpdateUsingALupdateService", 0, 0, true, SECONDLEVEL);
	}

	m_objIPCALUpdateClient.OpenServerMemoryMappedFile();
	return true;
	
}
/***********************************************************************************************
*  Function Name  : PreTranslateMessage
*  Description    : it uses to filter un-required input like enter ,escape and mouse drag event on GUI
*  Author Name    : Lalit kumawat
*  SR_NO		  :
*  Date           : 4-2-2015 
*************************************************************************************************/
// resolved by lalit kumawat 4-2-2015 
//In Data Encryption->when i press enter key on update dlg ,then ui getting hang
//	are getting selected at same time.
BOOL CISpyUpdatesDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	
	return CJpegDialog::PreTranslateMessage(pMsg);
}

/***********************************************************************************************
*  Function Name  : IsPopUpDisplayed
*  Description    : Function to check wheather any pop-up displayed.
*  Author Name    : Ramkrushna Shelke
*  SR_NO		  :
*  Date           : 11-12-2015
*************************************************************************************************/
bool CISpyUpdatesDlg::IsPopUpDisplayed()
{
	return m_dlgISpyUpdatesSecond.m_bIsPopUpDisplayed;
}

/***********************************************************************************************
*  Function Name  : ExtractRARFile
*  Description    : Function to Unrar the file for Local updates
*  Author Name    : Nitin Kolapkar
*  SR_NO		  :
*  Date           : 16th Feb 2016
*************************************************************************************************/
CString CISpyUpdatesDlg::ExtractRARFile(CString csInputFileName, DWORD &dwUnzipCount)
{
	TCHAR szUnzipPath[512] = { 0 };
	try
	{
		theApp.UnzipFile(csInputFileName.GetBuffer(), szUnzipPath, dwUnzipCount);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXUpdatesDlg::ExtractRARFile", 0, 0, true, SECONDLEVEL);
	}
	return CString(szUnzipPath);
}

/***********************************************************************************************
*  Function Name  : EnumAndDeleteTempFolder
*  Description    : Function to delete the db files from temp location
*  Author Name    : Nitin Kolapkar
*  SR_NO		  :
*  Date           : 16th Feb 2016
*************************************************************************************************/
bool CISpyUpdatesDlg::EnumAndDeleteTempFolder(CString csInputFileName)
{
	try
	{
		CFileFind finder;
		// build a string with wildcards
		CString strWildcard(csInputFileName);
		strWildcard += _T("\\*.*");

		// start working for files
		BOOL bWorking = finder.FindFile(strWildcard);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			// if it's a directory, recursively search it 
			if (finder.IsDirectory())
			{
				CString str = finder.GetFilePath();
				EnumAndDeleteTempFolder(str);
			}
			else
			{
				CString strFilePath = finder.GetFilePath();
				SetFileAttributes(strFilePath, FILE_ATTRIBUTE_NORMAL);
				DeleteFile(strFilePath);
			}
		}
		finder.Close();
		RemoveDirectory(csInputFileName);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXUpdatesDlg::EnumAndDeleteTempFolder", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

bool CISpyUpdatesDlg::CheckForMaxVersionInZip(CString csVersionNo)
{
	try
	{
		TCHAR szVersionNo[10] = { 0 };
		DWORD dwVersionNo = 0;
		TCHAR szVersion[5] = { 0 };
		int j = 0;

		_tcscpy_s(szVersionNo, csVersionNo);

		for (int i = 0; i < 9; i++)
		{
			if (isdigit(szVersionNo[i]))
			{
				szVersion[j] = szVersionNo[i];
				j++;
			}
		}
		dwVersionNo = static_cast<DWORD>(wcstod(szVersion, _T('\0')));
		if (m_dwMaxVersionInZip < dwVersionNo)
		{
			m_dwMaxVersionInZip = dwVersionNo;
			return true;
		}
		else
		{
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXUpdatesDlg::CheckForMaxVersionInZip", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :   GetAppFolderPath
*  Description    :   Get App folder path.
*  Author Name    :   Neha Gharge
*  SR_NO		  :
*  Date           :   22 Feb,2015
****************************************************************************************************/
CString CISpyUpdatesDlg::GetAppFolderPath()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			return L"";
		}
		CString csAppFolderPath = szModulePath;
		return csAppFolderPath;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CGenXUtilitiesDlg::GetAppFolderPath()", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/***************************************************************************************************
*  Function Name  : CreateDirectoryFocefully()
*  Description    : It will check directory. If not present, it will create that directory.
*  Author Name    : Neha Gharge
*  SR_NO		  : 
*  Date			  :	22 Feb,2016
****************************************************************************************************/
bool CISpyUpdatesDlg::CreateDirectoryFocefully(LPTSTR lpszPath)
{

	__try
	{

		CreateDirectory(lpszPath, NULL);
		if (PathFileExists(lpszPath))
			return false;

		_wmkdir(lpszPath);
		if (PathFileExists(lpszPath))
			return false;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### GenXALUSrv::CreateDirectoryFocefully::Exception", 0, 0, true, SECONDLEVEL);
	}

	return true;
}