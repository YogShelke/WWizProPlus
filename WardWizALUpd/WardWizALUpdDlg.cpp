// WardWizALUpdDlg.cpp : implementation file
//

#include "stdafx.h"
#include "WardWizALUpd.h"
#include "WardWizALUpdDlg.h"
#include <Softpub.h>
#include <Wincrypt.h>
#include <tchar.h>
#include <stdlib.h>

#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Wintrust.lib")
const int TIMER_SETUPUPLOADEDTOSERVER= 1500;
const int TIMER_CHECKSIGNINGCOMPLETED= 1600;
#include <Shlwapi.h>

char* g_strDatabaseFilePath = "\\VBALLREPORTS.DB";

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWardWizALUpdDlg dialog

typedef DWORD (*MAKEZIPFILE)	(TCHAR *pFile, TCHAR *pZipPath );
typedef DWORD (*GETFILEHASH)	(TCHAR *pFilePath, TCHAR *pFileHash);
bool gbIsBatchFileRunning = false;
MAKEZIPFILE		MakeZipFile = NULL;
GETFILEHASH		GetFileHash = NULL;

DWORD WINAPI Thread_GenerateUpdateFiles( LPVOID lpParam );
DWORD WINAPI Thread_SetStatus( LPVOID lpParam );
DWORD WINAPI Thread_AddSignatureToSelectedFiles( LPVOID lpParam );
TCHAR	m_szStatusText[512] = {0};


CWardWizALUpdDlg::CWardWizALUpdDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CWardWizALUpdDlg::IDD, pParent)
	
	, m_iRadioROAdd(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	mbIsFileSelected= false;
	m_bIsWow64 = FALSE;
	m_bVistaOnward = false;

	m_hThread_SetStatus = NULL ;
	m_hThread_GenerateUpdateFiles = NULL;

	m_hZipDLL = NULL;
	m_hHashDLL = NULL;
	m_bIsSetupCreated = false; 
	//Updating Product version by 1 from WWBinary.ini file to create setup next time .
    //Niranjan Deshak - 16/03/2015.
	m_bIsSetupCreatedAndFilesUploaded = false;

}

void CWardWizALUpdDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_PRODTYPE_ESSENTIAL, m_ProdType_Essential);
	DDX_Control(pDX, IDC_CHECK_PRODTYPE_PRO, m_ProdType_Pro);
	DDX_Control(pDX, IDC_CHECK_PRODTYPE_ELITE, m_ProdType_Elite);
	DDX_Control(pDX, IDC_CHECK_PRODTYPE_ESS_PLUS, m_ProdType_EssPlus);
	DDX_Control(pDX, IDC_CHECK_OSVERSION_32, m_OSVersion_32);
	DDX_Control(pDX, IDC_CHECK_OSVERSION_64, m_OSVersion_64);
	DDX_Control(pDX, IDC_CHECK_MAKE_SETUP, m_MakeSetup);
	DDX_Control(pDX, IDC_RADIO_PATCHTYPE_PRODUCT, m_PatchType_Product);
	DDX_Control(pDX, IDC_RADIO_PATCHTYPE_VIRUS, m_PatchType_Virus);
	DDX_Control(pDX, IDC_LIST_PATCH_FILES, m_List_PatchFiles);
	DDX_Control(pDX, IDC_BUTTON_GENERATE_UPDATE_FILES, m_Generate_Update_Files);
	DDX_Control(pDX, IDC_STATIC_STATUS, m_Status);
	DDX_Control(pDX, IDC_BUTTON_CLOSE, m_Close);
	DDX_Control(pDX, IDC_CHECK_REBUILD_32, m_Rebuild32);
	DDX_Control(pDX, IDC_CHECK_REBUILD_64, m_Rebuild64);
	DDX_Control(pDX, IDC_RADIO_RAR, m_rdbRAR);
	DDX_Control(pDX, IDC_RADIO_ZIP, m_rdbZIP);
	DDX_Control(pDX, IDC_RADIO_BUILD, m_btnBuild);
	DDX_Control(pDX, IDC_RADIO_REBUILD, m_btnReBuild);
	DDX_Control(pDX, IDC_CHECK_BUILD_TYPE, m_chkBuildType);
	DDX_Control(pDX, IDC_STATIC_BUILD_TEXT, m_grpBxBuidRebuild);
	DDX_Control(pDX, IDC_CHECK_PRODTYPE_BASIC, m_ProdType_Basic);
	DDX_Control(pDX, IDC_CHECK_CHECK_PATCH_TYPE, m_chkPatchType);
	DDX_Control(pDX, IDC_STATIC_PROD_VERSION, m_stProdVersion);
	DDX_Control(pDX, IDC_STATIC_DATABASE_VERSION, m_stDatabaseVersion);
	DDX_Control(pDX, IDC_EDIT_PROD_VERSION, m_edtProdVersion);
	DDX_Control(pDX, IDC_EDIT_DATABASE_VERSION, m_edtDatabaseVersion);
	//DDX_Control(pDX, IDC_EDIT1, mSelectedFileName);
	DDX_Control(pDX, IDC_CHECK_WITH_CLAM, m_chkwithClam);
	DDX_Control(pDX, IDC_CHECK_WITHOUT_CLAM, m_chkNoClam);
	DDX_Control(pDX, IDC_CHECK_PDB, m_chkpdb);
	DDX_Control(pDX, IDC_LIST1, m_lstSelectedFileList);
	DDX_Control(pDX, IDC_CHECK2, m_chkOfflinePatches);
	DDX_Control(pDX, IDC_EDIT_MD5, m_cedtMd5OfSelectedFile);
	DDX_Control(pDX, IDC_EDIT_BROWSEFILEFOR_MD5, m_stSelectedFilePathForZIP);



	/*DDX_Control(pDX, IDC_RADIO_Add, m_chkAdd);*/
	/*DDX_Radio(pDX, IDC_RADIO_Add, m_bChkModify);*/
	DDV_MinMaxInt(pDX, m_iRadioROAdd, 0, 2);
	DDX_Control(pDX, IDC_CHECK_REGOPT, m_chkRegOpt);
	DDX_Control(pDX, IDC_BUTTON_RO_ADD, m_btnROAddToList);
	DDX_Control(pDX, IDC_BUTTON_REMOVE, m_btnRORemoveFromList);
	DDX_Control(pDX, IDC_COMBO_RO_REGKEY, m_btnRORegKeyType);
	DDX_Control(pDX, IDC_EDIT_RO_VALUE, m_editROvalue);
	DDX_Control(pDX, IDC_EDIT_RO_PATH, m_editROPath);
	DDX_Control(pDX, IDC_LIST_RegOpt, m_List_RegOpt);
	DDX_Control(pDX, IDC_CHECK_UTILITY, m_chkUtility);
	DDX_Control(pDX, IDC_EDIT_SCAN_ENGINE_VERSION, m_edtScanEngineVersion);
}

BEGIN_MESSAGE_MAP(CWardWizALUpdDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CWardWizALUpdDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CWardWizALUpdDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_GENERATE_UPDATE_FILES, &CWardWizALUpdDlg::OnBnClickedButtonGenerateUpdateFiles)
	ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CWardWizALUpdDlg::OnBnClickedButtonClose)
	ON_BN_CLICKED(IDC_CHECK_BUILD_TYPE, &CWardWizALUpdDlg::OnBnClickedCheckBuildType)
	ON_BN_CLICKED(IDC_CHECK_CHECK_PATCH_TYPE, &CWardWizALUpdDlg::OnBnClickedCheckCheckPatchType)
	ON_BN_CLICKED(IDC_BUTTON_ABORT, &CWardWizALUpdDlg::OnBnClickedButtonAbort)
	ON_BN_CLICKED(IDC_BUTTON_ADDSIGN, &CWardWizALUpdDlg::OnBnClickedButtonAddsign)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, &CWardWizALUpdDlg::OnBnClickedButtonBrowse)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_BROWSEFILETOMD5, &CWardWizALUpdDlg::OnBnClickedButtonBrowsefiletomd5)
	ON_BN_CLICKED(IDC_BUTTON_GENERATEMD5, &CWardWizALUpdDlg::OnBnClickedButtonGeneratemd5)
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_MOUSEWHEEL()
	ON_BN_CLICKED(IDC_RADIO_ADD, &CWardWizALUpdDlg::OnBnClickedRadioAdd)
	ON_BN_CLICKED(IDC_RADIO_MODIFY, &CWardWizALUpdDlg::OnBnClickedRadioModify)
	ON_BN_CLICKED(IDC_RADIO_DELETE, &CWardWizALUpdDlg::OnBnClickedRadioDelete)
	ON_BN_CLICKED(IDC_CHECK_REGOPT, &CWardWizALUpdDlg::OnBnClickedCheckRegopt)
	ON_BN_CLICKED(IDC_BUTTON_RO_ADD, &CWardWizALUpdDlg::OnBnClickedButtonRoAdd)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CWardWizALUpdDlg::OnBnClickedButtonRemove)
	END_MESSAGE_MAP()


// CWardWizALUpdDlg message handlers

/***************************************************************************
  Function Name  : OnInitDialog
  Description    : Initialize the dialog box's controls.
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0005
****************************************************************************/
BOOL CWardWizALUpdDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	m_PatchType_Product.SetCheck(BST_CHECKED);
	//m_ProdType_Basic.SetCheck(BST_CHECKED);
	//m_ProdType_Essential.SetCheck(BST_CHECKED);
	m_ProdType_Elite.SetCheck(BST_CHECKED);
	//m_ProdType_EssPlus.SetCheck(BST_CHECKED);
	m_OSVersion_32.SetCheck(BST_CHECKED);
	m_OSVersion_64.SetCheck(BST_CHECKED);
	m_Rebuild32.SetCheck(BST_CHECKED);
	m_Rebuild64.SetCheck(BST_CHECKED);
	m_btnBuild.SetCheck(BST_CHECKED);
//	m_btnReBuild.SetCheck(BST_CHECKED);
	m_chkBuildType.SetCheck(BST_UNCHECKED);
	m_chkPatchType.SetCheck(BST_CHECKED);
	m_grpBxBuidRebuild.Invalidate();
	m_chkBuildType.ShowWindow(SW_SHOW);
	m_OSVersion_32.EnableWindow( FALSE );
	m_OSVersion_64.EnableWindow( FALSE );
	m_MakeSetup.SetCheck(BST_CHECKED);
	m_chkwithClam.SetCheck(BST_CHECKED);
	m_chkpdb.SetCheck(BST_CHECKED);
	m_chkUtility.SetCheck(BST_CHECKED);
	m_chkNoClam.SetCheck(BST_CHECKED);
	m_chkOfflinePatches.SetCheck(BST_CHECKED);
	m_List_PatchFiles.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FLATSB );
	m_List_PatchFiles.InsertColumn(1, L"File Name", LVCFMT_LEFT, 200 );
	m_List_PatchFiles.InsertColumn(2, L"File Size", LVCFMT_LEFT, 100 );
	m_List_PatchFiles.InsertColumn(3, L"File Hash", LVCFMT_LEFT, 300 );

	m_lstSelectedFileList.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FLATSB );
	m_lstSelectedFileList.InsertColumn(1,L"File Name",LVCFMT_LEFT,500);
	m_lstSelectedFileList.InsertColumn(1,L"Status",LVCFMT_LEFT,200);
	

	//m_List_RegOpt is the List view control used to view the registry options WardWizALUpd UI.
	//Name : Niranjan Deshak - 9/2/2015.
	m_iListROCount = 0;
	m_List_RegOpt.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FLATSB |LVS_EX_CHECKBOXES);
	m_List_RegOpt.InsertColumn(1, L"HKey", LVCFMT_LEFT, 50 );
	m_List_RegOpt.InsertColumn(2, L"Path", LVCFMT_LEFT, 100 );
	m_List_RegOpt.InsertColumn(3, L"Sub Value", LVCFMT_LEFT, 100 );
	m_List_RegOpt.InsertColumn(4, L"Status", LVCFMT_LEFT, 100);

	m_chkRegOpt.SetCheck(BST_CHECKED);

	m_rdbRAR.SetCheck(BST_CHECKED);

	m_btnRORegKeyType.AddString(L"HKLM");
	m_btnRORegKeyType.AddString(L"HKCC");
	m_btnRORegKeyType.AddString(L"HKCR");
	m_btnRORegKeyType.AddString(L"HKCU");
	m_btnRORegKeyType.AddString(L"HKU");
	m_btnRORegKeyType.SetCurSel(0);

	IsWow64();
	MakeRequiredFolderStructure( );
	//This Function call reads the WWBinary.ini file for registry options and loads them to List View control.
	//Name : Niranjan Deshak - 9/2/2015.
	AddRegOptToListFromWWBinary();
	if(theApp.m_AppIsRunningFromCmd)
	{
	 OnBnClickedButtonGenerateUpdateFiles();
	}
	SystemParametersInfo(SPI_GETWORKAREA,0,&m_rect,0);
	//int screen_x_size = m_rect.Width();  
	int iscreen_y_size = m_rect.Height();
	::SetWindowPos(m_hWnd,HWND_TOPMOST,0,0, 975, iscreen_y_size - 40 , SWP_NOZORDER);

	m_nScrollPos = 0;
	//LoadRequiredDLLs();	
	return TRUE;  // return TRUE  unless you set the focus to a control
}


/***************************************************************************
  Function Name  : OnPaint
  Description    : If you add a minimize button to your dialog, you will need the code below
				   to draw the icon.  For MFC applications using the document/view model,
				   this is automatically done for you by the framework.
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0006
****************************************************************************/
void CWardWizALUpdDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

/***************************************************************************
  Function Name  : OnQueryDragIcon
  Description    : The system calls this function to obtain the cursor to display while the user drags
				   the minimized window.
  Author Name    : Vilas 
  Date           : SR.N0 ALUPD_0007
****************************************************************************/
HCURSOR CWardWizALUpdDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/***************************************************************************
  Function Name  : OnBnClickedOk
  Description    : This function is Called when user clicks on OK button
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0008
****************************************************************************/
void CWardWizALUpdDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	//OnOK();
}

/***************************************************************************
  Function Name  : OnBnClickedCancel
  Description    : This function is Called when user clicks on Cancel button
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0009
****************************************************************************/
void CWardWizALUpdDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	//OnCancel();
}

/***************************************************************************
  Function Name  : MakeRequiredFolderStructure
  Description    : Called when user opens application 1st time
					All required folders are created for setup/Patch creation
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0010
****************************************************************************/
void CWardWizALUpdDlg::MakeRequiredFolderStructure( )
{

	try
	{

		GetModuleFileName(NULL, m_szApplPath, 511);

		TCHAR	*pTemp = wcsrchr(m_szApplPath, '\\');
		if( !pTemp )
		{
			AddLogEntry(L"### Failed getting application path CVibraniumALUpdDlg::MakeRequiredFolderStructure");
			AfxMessageBox(L"Problem while retrieving running path");
			exit(0);
		}

		*pTemp = '\0';


		TCHAR	szTemp[512] = {0} ;

		SYSTEMTIME		ST = {0};

		GetSystemTime( &ST);

		ZeroMemory(m_szOutDirName, sizeof(m_szOutDirName) );
		swprintf_s(	m_szOutDirName, _countof(m_szOutDirName), L"%d_%d_%d_%d_%d", 
			ST.wDay, ST.wMonth, ST.wYear, ST.wHour, ST.wMinute);


		swprintf_s(	szTemp, _countof(szTemp), L"%s\\ALUpdFiles", m_szApplPath);
		if( CreateOtherDirectory(szTemp))
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Creating direcory(%s)", szTemp);
			AfxMessageBox(L"Problem while retrieving running path");
			exit(0);
		}

		swprintf_s(	szTemp, _countof(szTemp), L"%s\\ALUpdFiles\\%s", m_szApplPath, m_szOutDirName);
		swprintf_s(	m_szPatchFolderPath , _countof(m_szPatchFolderPath ), L"%s", szTemp);
		if( CreateOtherDirectory(szTemp))
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Creating direcory(%s)", szTemp);
			AfxMessageBox(L"Problem while retrieving running path");
			exit(0);
		}

		ZeroMemory(szTemp, sizeof(szTemp) );
		swprintf_s(szTemp, _countof(szTemp), L"%s\\ALUpdFiles\\%s\\32", m_szApplPath, m_szOutDirName);
		if( CreateOtherDirectory(szTemp))
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Creating direcory(%s)", szTemp);
			AfxMessageBox(L"Problem while creating output directory");
			exit(0);
		}

		ZeroMemory(szTemp, sizeof(szTemp) );
		swprintf_s(szTemp, _countof(szTemp), L"%s\\ALUpdFiles\\%s\\64", m_szApplPath, m_szOutDirName);
		if( CreateOtherDirectory(szTemp) )
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Creating direcory(%s)", szTemp);
			AfxMessageBox(L"Problem while creating output directory");
			exit(0);
		}


		ZeroMemory(szTemp, sizeof(szTemp) );
		swprintf_s(szTemp, _countof(szTemp), L"%s\\ALUpdFiles\\%s\\Basic", m_szApplPath, m_szOutDirName);
		if( CreateOtherDirectory(szTemp) )
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Creating direcory(%s)", szTemp);
			AfxMessageBox(L"Problem while creating output directory");
			exit(0);
		}


		ZeroMemory(szTemp, sizeof(szTemp) );
		swprintf_s(szTemp, _countof(szTemp), L"%s\\ALUpdFiles\\%s\\Pro", m_szApplPath, m_szOutDirName);
		if( CreateOtherDirectory(szTemp) )
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Creating direcory(%s)", szTemp);
			AfxMessageBox(L"Problem while creating output directory");
			exit(0);
		}


		ZeroMemory(szTemp, sizeof(szTemp) );
		swprintf_s(szTemp, _countof(szTemp), L"%s\\ALUpdFiles\\%s\\Elite", m_szApplPath, m_szOutDirName);
		if( CreateOtherDirectory(szTemp) )
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Creating direcory(%s)", szTemp);
			AfxMessageBox(L"Problem while creating output directory");
			exit(0);
		}

		ZeroMemory(szTemp, sizeof(szTemp) );
		swprintf_s(szTemp, _countof(szTemp), L"%s\\ALUpdFiles\\%s\\Essential", m_szApplPath, m_szOutDirName);
		if( CreateOtherDirectory(szTemp) )
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Creating direcory(%s)", szTemp);
			AfxMessageBox(L"Problem while creating output directory");
			exit(0);
		}

		ZeroMemory(szTemp, sizeof(szTemp));
		swprintf_s(szTemp, _countof(szTemp), L"%s\\ALUpdFiles\\%s\\EssentialPlus", m_szApplPath, m_szOutDirName);
		if (CreateOtherDirectory(szTemp))
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Creating direcory(%s)", szTemp);
			AfxMessageBox(L"Problem while creating output directory");
			exit(0);
		}

		ZeroMemory(szTemp, sizeof(szTemp) );
		swprintf_s(szTemp, _countof(szTemp), L"%s\\ALUpdFiles\\%s\\Common", m_szApplPath, m_szOutDirName);
		if( CreateOtherDirectory(szTemp) )
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Creating direcory(%s)", szTemp);
			AfxMessageBox(L"Problem while creating output directory");
			exit(0);
		}
		/*
		ZeroMemory(m_szBasicIniPath, sizeof(m_szBasicIniPath) );
		ZeroMemory(m_szProIniPath, sizeof(m_szProIniPath) );
		ZeroMemory(m_szEliteIniPath, sizeof(m_szEliteIniPath) );
		ZeroMemory(m_szEssentialIniPath, sizeof(m_szEssentialIniPath));

		swprintf(m_szBasicIniPath, L"%s\\ALUpdFiles\\%s\\WWPatchB.ini", m_szApplPath, m_szOutDirName);
		swprintf(m_szEliteIniPath, L"%s\\ALUpdFiles\\%s\\WWPatchT.ini", m_szApplPath, m_szOutDirName);
		swprintf(m_szEssentialIniPath, L"%s\\ALUpdFiles\\%s\\VBPatchTS.ini", m_szApplPath, m_szOutDirName);

		//This code is added to create new ini files, i.e:WWizPatchB.ini and VibroPatchTS.ini .
		//Name : Niranjan Deshak - 9/2/2015.
		ZeroMemory(m_szWWizBasicIniPath, sizeof(m_szWWizBasicIniPath) );
		ZeroMemory(m_szWWizEssentialIniPath, sizeof(m_szWWizEssentialIniPath)); 
		ZeroMemory(m_szWWizEliteIniPath, sizeof(m_szWWizEliteIniPath));
		
		swprintf(m_szWWizBasicIniPath, L"%s\\ALUpdFiles\\%s\\WWizPatchB.ini", m_szApplPath, m_szOutDirName);
		swprintf(m_szWWizEssentialIniPath, L"%s\\ALUpdFiles\\%s\\VibroPatchTS.ini", m_szApplPath, m_szOutDirName);
		swprintf(m_szProIniPath, L"%s\\ALUpdFiles\\%s\\WWizPatchP.ini", m_szApplPath, m_szOutDirName);
		swprintf(m_szWWizEliteIniPath, L"%s\\ALUpdFiles\\%s\\WWizPatchT.ini", m_szApplPath, m_szOutDirName);
		*/



		ZeroMemory(m_szProgDir, sizeof(m_szProgDir) );
		ZeroMemory(m_szProgDirX64, sizeof(m_szProgDirX64) );

		GetEnvironmentVariable(L"ProgramFiles", m_szProgDir, 127 ) ;
	/*	if( m_bIsWow64 )
			GetEnvironmentVariable(L"ProgramFiles(x86)", m_szProgDirX64, 127 ) ;*/

		GetEnvironmentVariable(TEXT("PROGRAMFILES(X86)"), m_szProgDirX64, 127 );
			if( !PathFileExists( m_szProgDirX64 ) )
			{
			
				GetEnvironmentVariable(TEXT("PROGRAMFILES"), m_szProgDirX64, 127 );
				
			}

		ZeroMemory(m_szWinDir, sizeof(m_szWinDir) );
		GetWindowsDirectory(m_szWinDir, 255 );

		ZeroMemory(m_szWWBinariesPath, sizeof(m_szWWBinariesPath) );
		swprintf_s(m_szWWBinariesPath, _countof(m_szWWBinariesPath), L"%s\\WWBinary.ini", m_szApplPath);
		LoadBinariesNameAsPerSettings(m_szWWBinariesPath);

	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::MakeRequiredFolderStructure");
	}

}

/***************************************************************************
  Function Name  : CreateOtherDirectory
  Description    : This function is Called to create Directories of patches
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0011
****************************************************************************/
DWORD CWardWizALUpdDlg::CreateOtherDirectory( TCHAR *pDirName )
{
	__try
	{

		if( PathFileExists( pDirName ) )
			return 0x00 ;

		CreateDirectory( pDirName, NULL ) ;

		if( !PathFileExists( pDirName ) )
		{
			Sleep( 100 ) ;
			CreateDirectory( pDirName, NULL ) ;
		}

		if( !PathFileExists( pDirName ) )
		{
			Sleep( 100 ) ;
			_wmkdir( pDirName ) ;
		}

		if( PathFileExists( pDirName ) )
			return 0x00 ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::CreateOtherDirectory");
	}

	return 0x01 ;
}

/***************************************************************************
  Function Name  : IsWow64
  Description    : this function is called to check is machine architecture 64bit or not
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0012
****************************************************************************/
void CWardWizALUpdDlg::IsWow64()
{
	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL) ;

	LPFN_ISWOW64PROCESS		IsWow64Process = NULL ;
	OSVERSIONINFO			OSVI = {0} ;

	TCHAR	szValue[64] = {0};

	try
	{

		OSVI.dwOSVersionInfoSize = sizeof( OSVERSIONINFO ) ;
		GetVersionEx( &OSVI ) ;

		if( OSVI.dwMajorVersion > 5 )
			m_bVistaOnward = true ;


		IsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(	GetModuleHandle( TEXT("kernel32")),
																"IsWow64Process") ;
		if( !IsWow64Process )
			return ;

		IsWow64Process( GetCurrentProcess(), &m_bIsWow64 ) ;

		GetEnvironmentVariable(L"PROCESSOR_ARCHITECTURE", szValue, 63 ) ;
		if( wcsstr(szValue, L"64" ) )
			m_bIsWow64 = TRUE ;

	}
	catch( ...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::IsWow64");
	}

}

/***************************************************************************
  Function Name  : LoadRequiredDLLs
  Description    : This function is called to create the zip of DLL files
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0013
****************************************************************************/
bool CWardWizALUpdDlg::LoadRequiredDLLs()
{

	TCHAR	szTemp[512] = {0} ;

	try
	{
	
			/*if( m_bIsWow64 )
				swprintf_s(szTemp, _countof(szTemp), L"%s\\Release\\x64\\Binaries\\VBEXTRACT.DLL", m_szApplPath);
			else
				swprintf_s(szTemp, _countof(szTemp), L"%s\\Release\\Win32\\Binaries\\VBEXTRACT.DLL", m_szApplPath);

			m_hZipDLL = LoadLibrary(szTemp);
			if( !m_hZipDLL )
			{
				AddLogEntry(L"### Failed in CVibraniumALUpdDlg::LoadLibrary(%s)", szTemp);
				return true;
			}

			MakeZipFile = (MAKEZIPFILE) GetProcAddress(m_hZipDLL, "MakeZipFile");
			if( !MakeZipFile )
			{
				AddLogEntry(L"### Failed in CVibraniumALUpdDlg::MakeZipFile Address(%s)", szTemp);
				return true;
			}*/

			ZeroMemory(szTemp, sizeof(szTemp) );
			if( m_bIsWow64 )
				//Issue: Hash Dll not getting signed because its in use by ALUPD EXE
				//Resolved By: Nitin Kolapkar Date 4th Feb 2016
				swprintf_s(szTemp, _countof(szTemp), L"%s\\VBHASH.DLL", m_szApplPath);
			else
				swprintf_s(szTemp, _countof(szTemp), L"%s\\VBHASH.DLL", m_szApplPath);

			m_hHashDLL = LoadLibrary(szTemp);
			if( !m_hHashDLL )
			{
				DWORD dwRet = GetLastError();
				AddLogEntry(L"### Failed in CVibraniumALUpdDlg::LoadLibrary(%s)", szTemp);
				return true;
			}

			GetFileHash = (GETFILEHASH) GetProcAddress(m_hHashDLL, "GetFileHash");
			if( !GetFileHash )
			{
				AddLogEntry(L"### Failed in CVibraniumALUpdDlg::GetFileHash Address(%s)", szTemp);
				return true;
			}
	}
	catch( ...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::LoadRequiredDLLs");
	}

	return false;
}

/***************************************************************************
  Function Name  : OnBnClickedButtonGenerateUpdateFiles
  Description    : this function is called to generate product patches
				   Two threads are created 1. generating(build/rebuild) files 2. To show status
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0014
****************************************************************************/
void CWardWizALUpdDlg::OnBnClickedButtonGenerateUpdateFiles()
{
	try
	{
			m_bIsSetupCreated = false;
			m_hThread_SetStatus = NULL ;

			//if(theApp.m_AppIsRunningFromCmd)
			//{
				if (theApp.m_ichkBuildType == 0) //0 for BuildType unchecked
				{
					m_chkBuildType.SetCheck(BST_UNCHECKED);

					CString csProdVersion = L"";

					int iProductVersion[4] = { 0 };
					csProdVersion = m_szProdVersion;
					bool bRet = ParseVersionString(iProductVersion, csProdVersion);
					iProductVersion[3]++;

					swprintf_s(m_szProductVersion, _countof(m_szProductVersion), L"%d.%d.%d.%d", iProductVersion[0], iProductVersion[1], iProductVersion[2], iProductVersion[3]);
					m_edtProdVersion.SetWindowTextW(m_szProductVersion);
				}
				else
				{
					m_chkBuildType.SetCheck(BST_CHECKED);
				}
				//m_MakeSetup.SetCheck(BST_CHECKED);
				//m_ProdType_Basic.SetCheck(BST_CHECKED);
				//m_ProdType_Essential.SetCheck(BST_CHECKED); 
				//m_ProdType_Elite.SetCheck(BST_CHECKED);
				//m_ProdType_EssPlus.SetCheck(BST_CHECKED);
				//m_ProdType_Pro.SetCheck(BST_CHECKED); now it is unchecked untill building PRO setup is not required
				m_chkwithClam.SetCheck(BST_CHECKED);
				m_chkNoClam.SetCheck(BST_CHECKED);
				m_chkpdb.SetCheck(BST_CHECKED);
				m_chkUtility.SetCheck(BST_CHECKED);
				m_chkOfflinePatches.SetCheck(BST_CHECKED);

				if(theApp.m_AppCmdBuildType == 0)
				{
					m_btnBuild.SetCheck(BST_CHECKED);
					m_btnReBuild.SetCheck(BST_UNCHECKED);
				}
				else
				{
					m_btnBuild.SetCheck(BST_UNCHECKED);
					m_btnReBuild.SetCheck(BST_CHECKED);
				}
			//}

			m_hThread_GenerateUpdateFiles = ::CreateThread(	NULL, 0, (LPTHREAD_START_ROUTINE) Thread_GenerateUpdateFiles, 
															(LPVOID)this, 0, 0 ) ;
			if( m_hThread_GenerateUpdateFiles )
			{
				m_Generate_Update_Files.EnableWindow( FALSE );
				Sleep( 1000 );
			}

			m_hThread_SetStatus = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) Thread_SetStatus,(LPVOID)this, 0, 0 ) ;
			Sleep( 300 );

			if(theApp.m_AppIsRunningFromCmd)
			{
		    	SetTimer(TIMER_SETUPUPLOADEDTOSERVER, 200, NULL);
			}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::OnBnClickedButtonGenerateUpdateFiles");
	}
}

/***************************************************************************
  Function Name  : GetFileSizeAndHash
  Description    : This function is called to get file size before creating ZIP files of  generated product patches
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0015
****************************************************************************/
bool CWardWizALUpdDlg::GetFileSizeAndHash(TCHAR *pFilePath, DWORD &dwFileSize, TCHAR *pFileHash)
{

	try
	{ 
		DWORD dwRetryCount = 0;
		//FILE_SHARE_READ previouly it was 0 , and i was getting ERROR_SHARING_VIOLATION
		HANDLE hFile = CreateFile( pFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		while(hFile == INVALID_HANDLE_VALUE && dwRetryCount < 10)
		{
			if (hFile == INVALID_HANDLE_VALUE)
			{
				Sleep(10 * 1000);
				//AddLogEntry(L"### GetFileSizeAndHash : Error in opening file %s",pFilePath);
				//return true;
				hFile = CreateFile(pFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				dwRetryCount++ ;
			}
		};
		if (dwRetryCount >= 10)
		{
			AddLogEntry(L"### GetFileSizeAndHash : Error in opening file %s",pFilePath);
			return true;
		}

		dwFileSize = GetFileSize(hFile, NULL );
		CloseHandle( hFile );
		hFile = INVALID_HANDLE_VALUE;

		if( pFileHash )
		{
			if( !GetFileHash(pFilePath, pFileHash) )
				return false;
			else
			{
				AddLogEntry(L"### Exception in CVibraniumALUpdDlg::IsWow64");
				return true;
			}
		}
	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::GetFileSizeAndHash");
	}

	return true;
}

/***************************************************************************
  Function Name  : EnumAndMakeZipFiles
  Description    : This function is called to create ZIP files
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0016
****************************************************************************/
bool CWardWizALUpdDlg::EnumAndMakeZipFiles(TCHAR *pSour, TCHAR *pDest, TCHAR *pSecName, TCHAR *pShortPath)
{
	bool bRetValue = true;

	try
	{
		if (!pSour || !pDest)
			return bRetValue;

		if( !PathFileExists(pSour) )
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Path not found(%s)", pSour);
			return bRetValue;
		}

		if (!PathFileExists(pDest))
		{
			CreateDirectory(pDest, NULL);
		}
		
		TCHAR	szFileName[128] = {0};
		TCHAR	szFilePath[512] = {0};
		TCHAR	szFileHash[128] = {0};
		TCHAR   szZipFileHash[128] = {0};
		TCHAR	szFileZipPath[512] = {0};
		CStringA csZipFilePath = "";
		DWORD	dwFileSize = 0x00;
		DWORD	dwFileZipSize = 0x00;
		DWORD	dwCount = 0x01;
		CString csFilePath = L"";
		CFileFind finder;

		TCHAR	szTemp[512] = {0};

		swprintf_s(szTemp, _countof(szTemp), L"%s\\*.*", pSour);

		BOOL bWorking = finder.FindFile(szTemp);

		while (bWorking)
		{
			bWorking = finder.FindNextFile();

			// skip . and .. files; otherwise, we'd 
			// recur infinitely! 

			if (finder.IsDots())
				continue;

			if (finder.IsDirectory())
				continue;

			wcscpy(szFileName, finder.GetFileName() );
			wcscpy(szFilePath, finder.GetFilePath() );

			CString stFilePath = szFilePath;
			CString csFileName = stFilePath.Right(stFilePath.GetLength() - ( stFilePath.ReverseFind('\\') + 1));
			csFileName.MakeLower();
			if (csFileName != L"VButility.exe")
			{
				AddSignToSpecifiedFile(stFilePath);  // add signature to file before ziping
			}
			else
			{
				AddSignToSpecifiedFile(stFilePath, false);  // add signature to file before ziping
			}


			if( GetFileSizeAndHash(szFilePath, dwFileSize, szFileHash) )
			{
				AddLogEntry(L"### Failed in CVibraniumALUpdDlg::GetFileSizeAndHash(%s)", szFilePath );
				break;
			}

			swprintf_s(szFileZipPath, _countof(szFileZipPath), L"%s\\%s.zip", pDest, szFileName);
			
			/*if( MakeZipFile(szFilePath, szFileZipPath) )
			{
				AddLogEntry(L"### Failed in CVibraniumALUpdDlg::MakeZipFile(%s)",szFilePath );
				break;
			}*/
			csZipFilePath = szFileZipPath;
			csFilePath = csZipFilePath;
			if (!zip(szFilePath, csFilePath, csZipFilePath))
			{
				AddLogEntry(L"### Failed in CVibraniumALUpdDlg::MakeZipFile(%s)",szFilePath );
				break;
			}

			GetFileSizeAndHash(szFileZipPath, dwFileZipSize, szZipFileHash);
			if( !dwFileZipSize )
			{
				AddLogEntry(L"### Failed in CVibraniumALUpdDlg::GetFileSizeAndHash Zip size(%s)", szFilePath);
				break;
			}

			//Write to ini
			TCHAR	szValue[128] = {0};
			TCHAR	szData[512] = {0};

			swprintf_s(	szValue, _countof(szValue), L"%lu", m_FilesCount++);

			swprintf_s(	szData, _countof(szData), L"%s,%lu,%lu,%s,%s", szFileName, 
				dwFileZipSize, dwFileSize, szFileHash, pShortPath);
			if( !WriteToIni(pSecName, szFileName, szData, szFileName))
			{
				AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WriteToIni(%s)", szData);
				break;
			}

			if (m_ProdType_Elite.GetCheck() == BST_CHECKED)
			{
				//Write to ini
				TCHAR	szValue[128] = {0};
				TCHAR	szData[512] = {0};

				swprintf_s(szValue, _countof(szValue), L"%lu", m_FilesCount++);

				swprintf_s(szData, _countof(szData), L"%s,%lu,%lu,%s,%s,%s", szFileName,
					dwFileZipSize, dwFileSize, szFileHash, szZipFileHash, pShortPath);
				if (!WriteToEPSServerIni(pSecName, szFileName, szData, szFileName))
				{
					AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WriteToIni(%s)", szData);
					break;
				}
			}

			ZeroMemory(szValue, sizeof(szValue) );
			swprintf_s(	szValue, _countof(szValue), L"%lu", dwFileSize);

			m_List_PatchFiles.InsertItem(m_ListItemCount, szFileName );
			m_List_PatchFiles.SetItemText(m_ListItemCount, 1, szValue );
			m_List_PatchFiles.SetItemText(m_ListItemCount++, 2, szFileHash );

			dwFileSize = 0x00;
			dwFileZipSize = 0x00;

			ZeroMemory(szFileName, sizeof(szFileName) );
			ZeroMemory(szFilePath, sizeof(szFilePath) );
			ZeroMemory(szFileHash, sizeof(szFileHash) );
			ZeroMemory(szFileZipPath, sizeof(szFileZipPath) );

		}

		finder.Close();
	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::EnumAndMakeZipFiles");
	}

	return bRetValue;
}

/***************************************************************************
  Function Name  : WriteToIni
  Description    : This function is called to write entries of patches into INI files 
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0017
  Modified		 : WriteToIni is modified, For Writing to new ini files i.e: WWizPathchB and WWizPatchE.
				   Also String ,YC,N is appended to WWizPatchB.ini and VibroPatchTS.ini.
  Name			 : Niranjan Deshak. - 9/2/2015.
****************************************************************************/

DWORD CWardWizALUpdDlg::WriteToIni(TCHAR *pKey, TCHAR *pValue, TCHAR *pData, TCHAR *pFileName)
{
	DWORD	dwRetValue = 0x00;
	CString sz_Count = L"";
	TCHAR szWWizData[512] = { 0 };
	CString csAppend = L"";

	if (pFileName != NULL)
	{
		pFileName = _tcsupr(pFileName);
	}

	POSITION pos = m_listBasic.GetHeadPosition();
	try
	{
		if (m_ProdType_Basic.GetCheck() == BST_CHECKED)
		{
			if (m_listBasic.Find(CString(pFileName)))
			{
				if (_tcscmp(pKey, L"Files_32") == 0)
				{
					ZeroMemory(m_szBasicCount, sizeof(m_szBasicCount));
					m_iBasicCount = 0;
					GetPrivateProfileString(L"Count_32", L"Count", L"", m_szBasicCount, 255, m_szBasicIniPath);
					m_iBasicCount = _ttoi(m_szBasicCount);
					m_iBasicCount++;
					sz_Count.Format(L"%d", m_iBasicCount);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"Count_32", L"Count", sz_Count, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));
					csAppend = L"\,YC\,N";
					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"Count_32", L"Count", sz_Count, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"Files_64") == 0)
				{
					ZeroMemory(m_szBasicCount, sizeof(m_szBasicCount));
					m_iBasicCount = 0;
					GetPrivateProfileString(L"Count_64", L"Count", L"", m_szBasicCount, 255, m_szBasicIniPath);
					m_iBasicCount = _ttoi(m_szBasicCount);
					m_iBasicCount++;
					sz_Count.Format(L"%d", m_iBasicCount);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"Count_64", L"Count", sz_Count, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));
					csAppend = L"\,YC\,N";
					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"Count_64", L"Count", sz_Count, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"Common") == 0)
				{
					ZeroMemory(m_szBasicCount, sizeof(m_szBasicCount));
					m_iBasicCount = 0;
					GetPrivateProfileString(L"Common", L"Count", L"", m_szBasicCount, 255, m_szBasicIniPath);
					m_iBasicCount = _ttoi(m_szBasicCount);
					m_iBasicCount++;
					sz_Count.Format(L"%d", m_iBasicCount);
					dwRetValue = WritePrivateProfileString(L"Common", L"Count", sz_Count, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					/*CString csAppend = L"";*/
					if ((_tcscmp(pFileName, L"DAILY.CLD") == 0) || (_tcscmp(pFileName, L"MAIN.CVD") == 0) || (_tcscmp(pFileName, L"WRDWIZWHLST.FP") == 0))
					{
						csAppend = L"\,NC\,Y";
					}
					else
					{
						csAppend = L"\,YC\,N";
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));

					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
					dwRetValue = WritePrivateProfileString(L"Common", L"Count", sz_Count, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"CommonDB") == 0)
				{
					ZeroMemory(m_szBasicCount, sizeof(m_szBasicCount));
					m_iBasicCount = 0;
					GetPrivateProfileString(L"CommonDB", L"Count", L"", m_szBasicCount, 255, m_szBasicIniPath);
				
					if (_tcslen(m_szBasicCount) != 0x00)
					{
						m_iBasicCount = _ttoi(m_szBasicCount);
					}

					m_iBasicCount++;
					sz_Count.Format(L"%d", m_iBasicCount);
					dwRetValue = WritePrivateProfileString(L"CommonDB", L"Count", sz_Count, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					/*CString csAppend = L"";*/
					if ((_tcscmp(pFileName, L"DAILY.CLD") == 0) || (_tcscmp(pFileName, L"MAIN.CVD") == 0) || (_tcscmp(pFileName, L"WRDWIZWHLST.FP") == 0))
					{
						csAppend = L"\,NC\,Y";
					}
					else
					{
						csAppend = L"\,YC\,N";
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));

					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
					dwRetValue = WritePrivateProfileString(L"CommonDB", L"Count", sz_Count, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"Basic") == 0)
				{
					ZeroMemory(m_szBasicCount, sizeof(m_szBasicCount));
					m_iBasicCount = 0;
					GetPrivateProfileString(L"Basic", L"Count", L"", m_szBasicCount, 255, m_szBasicIniPath);
					m_iBasicCount = _ttoi(m_szBasicCount);
					m_iBasicCount++;
					sz_Count.Format(L"%d", m_iBasicCount);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"Basic", L"Count", sz_Count, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));
					csAppend = L"\,YC\,N";
					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"Basic", L"Count", sz_Count, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}

				if (_tcscmp(pKey, L"ProductVersion") == 0)
				{

					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}

				}
				else if (_tcscmp(pKey, L"DatabaseVersion") == 0)
				{

					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}

				}
				else if (_tcscmp(pKey, L"ScanEngineVersion") == 0)
				{

					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"RegOpt") == 0)
				{
					WriteKeyValueToIni(pKey, pValue, pData, m_szBasicIniPath, m_szWWizBasicIniPath);
				}
				else if (_tcscmp(pKey, L"DllRegister") == 0)
				{
					WriteKeyValueToIni(pKey, pValue, pData, m_szBasicIniPath, m_szWWizBasicIniPath);
				}
				else if (_tcscmp(pKey, L"AddedFeatureBas") == 0)
				{
					WriteKeyValueToIni(L"AddedFeature", pValue, pData, m_szBasicIniPath, m_szWWizBasicIniPath);
				}
				else if (_tcscmp(pKey, L"EnhancedFunctionalityBas") == 0)
				{
					WriteKeyValueToIni(L"EnhancedFunctionality", pValue, pData, m_szBasicIniPath, m_szWWizBasicIniPath);
				}

				else
				{
					dwRetValue = 1;
				}
			}
			else
			{
				if (_tcscmp(pKey, L"ProductVersion") == 0)
				{

					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}

				}
				else if (_tcscmp(pKey, L"DatabaseVersion") == 0)
				{

					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"ScanEngineVersion") == 0)
				{

					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"RegOpt") == 0)
				{
					WriteKeyValueToIni(pKey, pValue, pData, m_szBasicIniPath, m_szWWizBasicIniPath);
				}
				else if (_tcscmp(pKey, L"DllRegister") == 0)
				{
					WriteKeyValueToIni(pKey, pValue, pData, m_szBasicIniPath, m_szWWizBasicIniPath);
				}
				else if (_tcscmp(pKey, L"AddedFeatureBas") == 0)
				{
					WriteKeyValueToIni(L"AddedFeature", pValue, pData, m_szBasicIniPath, m_szWWizBasicIniPath);
				}
				else if (_tcscmp(pKey, L"EnhancedFunctionalityBas") == 0)
				{
					WriteKeyValueToIni(L"EnhancedFunctionality", pValue, pData, m_szBasicIniPath, m_szWWizBasicIniPath);
				}

				if (_tcscmp(pKey, L"Basic") == 0)
				{
					if (_tcscmp(pValue, L"PRODUCTSETTINGS.INI") == 0)
					{
						ZeroMemory(m_szBasicCount, sizeof(m_szBasicCount));
						m_iBasicCount = 0;
						GetPrivateProfileString(L"Basic", L"Count", L"", m_szBasicCount, 255, m_szBasicIniPath);
						m_iBasicCount = _ttoi(m_szBasicCount);
						m_iBasicCount++;
						sz_Count.Format(L"%d", m_iBasicCount);
						dwRetValue = WritePrivateProfileString(L"Basic", sz_Count, pData, m_szBasicIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(L"Basic", L"Count", sz_Count, m_szBasicIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
				}
				else
				{
					dwRetValue = 1;
				}
			}
		}

		if (m_ProdType_Essential.GetCheck() == BST_CHECKED)
		{
			if (m_listEssential.Find(CString(pFileName)))
			{
				if (_tcscmp(pKey, L"Files_32") == 0)
				{
					ZeroMemory(m_szEssentialCount, sizeof(m_szEssentialCount));
					m_iEssentialCount = 0;
					GetPrivateProfileString(L"Count_32", L"Count", L"", m_szEssentialCount, 255, m_szEssentialIniPath);
					m_iEssentialCount = _ttoi(m_szEssentialCount);
					m_iEssentialCount++;
					sz_Count.Format(L"%d", m_iEssentialCount);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"Count_32", L"Count", sz_Count, m_szEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));
					csAppend = L"\,YC\,N";
					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"Count_32", L"Count", sz_Count, m_szWWizEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"Files_64") == 0)
				{
					ZeroMemory(m_szEssentialCount, sizeof(m_szEssentialCount));
					m_iEssentialCount = 0;
					GetPrivateProfileString(L"Count_64", L"Count", L"", m_szEssentialCount, 255, m_szEssentialIniPath);
					m_iEssentialCount = _ttoi(m_szEssentialCount);
					m_iEssentialCount++;
					sz_Count.Format(L"%d", m_iEssentialCount);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"Count_64", L"Count", sz_Count, m_szEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));
					csAppend = L"\,YC\,N";
					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}

					dwRetValue = WritePrivateProfileString(L"Count_64", L"Count", sz_Count, m_szWWizEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"Common") == 0)
				{
					ZeroMemory(m_szEssentialCount, sizeof(m_szEssentialCount));
					m_iEssentialCount = 0;
					GetPrivateProfileString(L"Common", L"Count", L"", m_szEssentialCount, 255, m_szEssentialIniPath);
					m_iEssentialCount = _ttoi(m_szEssentialCount);
					m_iEssentialCount++;
					sz_Count.Format(L"%d", m_iEssentialCount);
					dwRetValue = WritePrivateProfileString(L"Common", L"Count", sz_Count, m_szEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));
					//CString csAppend = L"";
					if ((_tcscmp(pFileName, L"DAILY.CLD") == 0) || (_tcscmp(pFileName, L"MAIN.CVD") == 0) || (_tcscmp(pFileName, L"WRDWIZWHLST.FP") == 0))
					{
						csAppend = L"\,NC\,Y";
					}
					else
					{
						csAppend = L"\,YC\,N";
					}
					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
					dwRetValue = WritePrivateProfileString(L"Common", L"Count", sz_Count, m_szWWizEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"CommonDB") == 0)
				{
					ZeroMemory(m_szEssentialCount, sizeof(m_szEssentialCount));
					m_iEssentialCount = 0;
					GetPrivateProfileString(L"CommonDB", L"Count", L"", m_szEssentialCount, 255, m_szEssentialIniPath);

					if (_tcslen(m_szEssentialCount) != 0x00)
					{
						m_iEssentialCount = _ttoi(m_szEssentialCount);
					}

					m_iEssentialCount++;
					sz_Count.Format(L"%d", m_iEssentialCount);
					dwRetValue = WritePrivateProfileString(L"CommonDB", L"Count", sz_Count, m_szEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));
					//CString csAppend = L"";
					if ((_tcscmp(pFileName, L"DAILY.CLD") == 0) || (_tcscmp(pFileName, L"MAIN.CVD") == 0) || (_tcscmp(pFileName, L"WRDWIZWHLST.FP") == 0))
					{
						csAppend = L"\,NC\,Y";
					}
					else
					{
						csAppend = L"\,YC\,N";
					}
					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
					dwRetValue = WritePrivateProfileString(L"CommonDB", L"Count", sz_Count, m_szWWizEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"Essential") == 0)
				{
					ZeroMemory(m_szEssentialCount, sizeof(m_szEssentialCount));
					m_iEssentialCount = 0;
					GetPrivateProfileString(L"Essential", L"Count", L"", m_szEssentialCount, 255, m_szEssentialIniPath);
					m_iEssentialCount = _ttoi(m_szEssentialCount);
					m_iEssentialCount++;
					sz_Count.Format(L"%d", m_iEssentialCount);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"Essential", L"Count", sz_Count, m_szEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));
					csAppend = L"\,YC\,N";
					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"Essential", L"Count", sz_Count, m_szWWizEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"ProductVersion") == 0)
				{

					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}

				}
				else if (_tcscmp(pKey, L"DatabaseVersion") == 0)
				{
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"ScanEngineVersion") == 0)
				{
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"DataEncVersion") == 0)
				{
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				if (_tcscmp(pKey, L"RegOpt") == 0)
				{
					WriteKeyValueToIni(pKey, pValue, pData, m_szEssentialIniPath, m_szWWizEssentialIniPath);
				}
				else if (_tcscmp(pKey, L"DllRegister") == 0)
				{
					WriteKeyValueToIni(pKey, pValue, pData, m_szEssentialIniPath, m_szWWizEssentialIniPath);
				}
				else if (_tcscmp(pKey, L"AddedFeatureEss") == 0)
				{
					WriteKeyValueToIni(L"AddedFeature", pValue, pData, m_szEssentialIniPath, m_szWWizEssentialIniPath);
				}
				else if (_tcscmp(pKey, L"EnhancedFunctionalityEss") == 0)
				{
					WriteKeyValueToIni(L"EnhancedFunctionality", pValue, pData, m_szEssentialIniPath, m_szWWizEssentialIniPath);
				}
				else
				{
					dwRetValue = 1;
				}
			}
			else
			{
				if (_tcscmp(pKey, L"ProductVersion") == 0)
				{

					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"DatabaseVersion") == 0)
				{

					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"ScanEngineVersion") == 0)
				{
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"DataEncVersion") == 0)
				{

					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEssentialIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"RegOpt") == 0)
				{
					WriteKeyValueToIni(pKey, pValue, pData, m_szEssentialIniPath, m_szWWizEssentialIniPath);
				}
				else if (_tcscmp(pKey, L"DllRegister") == 0)
				{
					WriteKeyValueToIni(pKey, pValue, pData, m_szEssentialIniPath, m_szWWizEssentialIniPath);
				}
				else if (_tcscmp(pKey, L"AddedFeatureEss") == 0)
				{
					WriteKeyValueToIni(L"AddedFeature", pValue, pData, m_szEssentialIniPath, m_szWWizEssentialIniPath);
				}
				else if (_tcscmp(pKey, L"EnhancedFunctionalityEss") == 0)
				{
					WriteKeyValueToIni(L"EnhancedFunctionality", pValue, pData, m_szEssentialIniPath, m_szWWizEssentialIniPath);
				}

				if (_tcscmp(pKey, L"Essential") == 0)
				{
					if (_tcscmp(pValue, L"PRODUCTSETTINGS.INI") == 0)
					{
						ZeroMemory(m_szEssentialCount, sizeof(m_szEssentialCount));
						m_iEssentialCount = 0;
						GetPrivateProfileString(L"Essential", L"Count", L"", m_szEssentialCount, 255, m_szEssentialIniPath);
						m_iEssentialCount = _ttoi(m_szEssentialCount);
						m_iEssentialCount++;
						sz_Count.Format(L"%d", m_iEssentialCount);
						dwRetValue = WritePrivateProfileString(L"Essential", L"Count", sz_Count, m_szEssentialIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(L"Essential", sz_Count, pData, m_szEssentialIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
				}
				else
				{
					dwRetValue = 1;
				}
			}
		}

		if (m_ProdType_Pro.GetCheck() == BST_CHECKED)
		{
			if (m_listPro.Find(CString(pFileName)))
			{
				if (_tcscmp(pKey, L"Files_32") == 0)
				{
					ZeroMemory(m_szProCount, sizeof(m_szProCount));
					m_iProCount = 0;
					GetPrivateProfileString(L"Count_32", L"Count", L"", m_szProCount, 255, m_szProIniPath);
					m_iProCount = _ttoi(m_szProCount);
					m_iProCount++;
					sz_Count.Format(L"%d", m_iProCount);
					ZeroMemory(szWWizData, sizeof(szWWizData));
					csAppend = L"\,YC\,N";
					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szProIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"Count_32", L"Count", sz_Count, m_szProIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"Files_64") == 0)
				{
					ZeroMemory(m_szProCount, sizeof(m_szProCount));
					m_iProCount = 0;
					GetPrivateProfileString(L"Count_64", L"Count", L"", m_szProCount, 255, m_szProIniPath);
					m_iProCount = _ttoi(m_szProCount);
					m_iProCount++;
					sz_Count.Format(L"%d", m_iProCount);
					dwRetValue = WritePrivateProfileString(L"Count_64", L"Count", sz_Count, m_szProIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));
					csAppend = L"\,YC\,N";
					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szProIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"Common") == 0)
				{
					ZeroMemory(m_szProCount, sizeof(m_szProCount));
					m_iProCount = 0;
					GetPrivateProfileString(L"Common", L"Count", L"", m_szProCount, 255, m_szProIniPath);
					m_iProCount = _ttoi(m_szProCount);
					m_iProCount++;
					sz_Count.Format(L"%d", m_iProCount);
					dwRetValue = WritePrivateProfileString(L"Common", L"Count", sz_Count, m_szProIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}

					if ((_tcscmp(pFileName, L"DAILY.CLD") == 0) || (_tcscmp(pFileName, L"MAIN.CVD") == 0) || (_tcscmp(pFileName, L"WRDWIZWHLST.FP") == 0))
					{
						csAppend = L"\,NC\,Y";
					}
					else
					{
						csAppend = L"\,YC\,N";
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));

					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);

					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szProIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"CommonDB") == 0)
				{
					ZeroMemory(m_szProCount, sizeof(m_szProCount));
					m_iProCount = 0;
					GetPrivateProfileString(L"CommonDB", L"Count", L"", m_szProCount, 255, m_szProIniPath);

					if (_tcslen(m_szProCount) != 0x00)
					{
						m_iProCount = _ttoi(m_szProCount);
					}

					m_iProCount++;
					sz_Count.Format(L"%d", m_iProCount);
					dwRetValue = WritePrivateProfileString(L"CommonDB", L"Count", sz_Count, m_szProIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}

					if ((_tcscmp(pFileName, L"DAILY.CLD") == 0) || (_tcscmp(pFileName, L"MAIN.CVD") == 0) || (_tcscmp(pFileName, L"WRDWIZWHLST.FP") == 0))
					{
						csAppend = L"\,NC\,Y";
					}
					else
					{
						csAppend = L"\,YC\,N";
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));

					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);

					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szProIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"Pro") == 0)
				{
					ZeroMemory(m_szProCount, sizeof(m_szProCount));
					m_iProCount = 0;
					GetPrivateProfileString(L"Pro", L"Count", L"", m_szProCount, 255, m_szProIniPath);
					m_iProCount = _ttoi(m_szProCount);
					m_iBasicCount++;
					sz_Count.Format(L"%d", m_iProCount);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szProIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"Pro", L"Count", sz_Count, m_szProIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"RegOpt") == 0)
				{
					WriteKeyValueToIni(pKey, pValue, pData, m_szProIniPath, NULL);
				}
				else if (_tcscmp(pKey, L"DllRegister") == 0)
				{
					WriteKeyValueToIni(pKey, pValue, pData, m_szProIniPath, NULL);
				}
				else if (_tcscmp(pKey, L"AddedFeaturePro") == 0)
				{
					WriteKeyValueToIni(L"AddedFeature", pValue, pData, m_szProIniPath, NULL);
				}
				else if (_tcscmp(pKey, L"EnhancedFunctionalityPro") == 0)
				{
					WriteKeyValueToIni(L"EnhancedFunctionality", pValue, pData, m_szProIniPath, NULL);
				}
				else
				{
					dwRetValue = 1;
				}
			}
			else
			{
				if (_tcscmp(pKey, L"ProductVersion") == 0)
				{

					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szProIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szProIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}

				}
				else if (_tcscmp(pKey, L"DatabaseVersion") == 0)
				{

					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szProIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szProIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}

				}
				else if (_tcscmp(pKey, L"ScanEngineVersion") == 0)
				{
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"DataEncVersion") == 0)
				{

					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szProIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szProIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}

				}
				else if (_tcscmp(pKey, L"RegOpt") == 0)
				{
					WriteKeyValueToIni(pKey, pValue, pData, m_szProIniPath, NULL);
				}
				else if (_tcscmp(pKey, L"DllRegister") == 0)
				{
					WriteKeyValueToIni(pKey, pValue, pData, m_szProIniPath, NULL);
				}
				else if (_tcscmp(pKey, L"AddedFeaturePro") == 0)
				{
					WriteKeyValueToIni(L"AddedFeature", pValue, pData, m_szProIniPath, NULL);
				}
				else if (_tcscmp(pKey, L"EnhancedFunctionalityPro") == 0)
				{
					WriteKeyValueToIni(L"EnhancedFunctionality", pValue, pData, m_szProIniPath, NULL);
				}

				if (_tcscmp(pKey, L"Pro") == 0)
				{
					if (_tcscmp(pValue, L"PRODUCTSETTINGS.INI") == 0)
					{
						ZeroMemory(m_szProCount, sizeof(m_szProCount));
						m_iBasicCount = 0;
						GetPrivateProfileString(L"Pro", L"Count", L"", m_szProCount, 255, m_szProIniPath);
						m_iProCount = _ttoi(m_szProCount);
						m_iProCount++;
						sz_Count.Format(L"%d", m_iProCount);
						dwRetValue = WritePrivateProfileString(L"Pro", sz_Count, pData, m_szProIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(L"Pro", L"Count", sz_Count, m_szProIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
				}
				else
				{
					dwRetValue = 1;
				}
			}


			/*dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szProIniPath );
			if( !dwRetValue)
			{
			AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s,%s)", pData, m_szProIniPath);
			return dwRetValue;
			}*/
		}

		if (m_ProdType_Elite.GetCheck() == BST_CHECKED)
		{
			if (m_listElite.Find(CString(pFileName)))
				{
					if (_tcscmp(pKey, L"Files_32") == 0)
					{
						ZeroMemory(m_szEliteCount, sizeof(m_szEliteCount));
						m_iEliteCount = 0;
						GetPrivateProfileString(L"Count_32", L"Count", L"", m_szEliteCount, 255, m_szEliteIniPath);
						m_iEliteCount = _ttoi(m_szEliteCount);
						m_iEliteCount++;
						sz_Count.Format(L"%d", m_iEliteCount);
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(L"Count_32", L"Count", sz_Count, m_szEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						ZeroMemory(szWWizData, sizeof(szWWizData));
						csAppend = L"\,YC\,N";
						swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(L"Count_32", L"Count", sz_Count, m_szWWizEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"Files_64") == 0)
					{
						ZeroMemory(m_szEliteCount, sizeof(m_szEliteCount));
						m_iEliteCount = 0;
						GetPrivateProfileString(L"Count_64", L"Count", L"", m_szEliteCount, 255, m_szEliteIniPath);
						m_iEliteCount = _ttoi(m_szEliteCount);
						m_iEliteCount++;
						sz_Count.Format(L"%d", m_iEliteCount);
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(L"Count_64", L"Count", sz_Count, m_szEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						ZeroMemory(szWWizData, sizeof(szWWizData));
						csAppend = L"\,YC\,N";
						swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}

						dwRetValue = WritePrivateProfileString(L"Count_64", L"Count", sz_Count, m_szWWizEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"Common") == 0)
					{
						ZeroMemory(m_szEliteCount, sizeof(m_szEliteCount));
						m_iEliteCount = 0;
						GetPrivateProfileString(L"Common", L"Count", L"", m_szEliteCount, 255, m_szEliteIniPath);
						m_iEliteCount = _ttoi(m_szEliteCount);
						m_iEliteCount++;
						sz_Count.Format(L"%d", m_iEliteCount);
						dwRetValue = WritePrivateProfileString(L"Common", L"Count", sz_Count, m_szEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						ZeroMemory(szWWizData, sizeof(szWWizData));
						//CString csAppend = L"";
						if ((_tcscmp(pFileName, L"DAILY.CLD") == 0) || (_tcscmp(pFileName, L"MAIN.CVD") == 0) || (_tcscmp(pFileName, L"WRDWIZWHLST.FP") == 0))
						{
							csAppend = L"\,NC\,Y";
						}
						else
						{
							csAppend = L"\,YC\,N";
						}
						swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
						dwRetValue = WritePrivateProfileString(L"Common", L"Count", sz_Count, m_szWWizEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"CommonDB") == 0)
					{
						ZeroMemory(m_szEliteCount, sizeof(m_szEliteCount));
						m_iEliteCount = 0;
						GetPrivateProfileString(L"CommonDB", L"Count", L"", m_szEliteCount, 255, m_szEliteIniPath);

						if (_tcslen(m_szEliteCount) != 0x00)
						{
							m_iEliteCount = _ttoi(m_szEliteCount);
						}

						m_iEliteCount++;
						sz_Count.Format(L"%d", m_iEssentialCount);
						dwRetValue = WritePrivateProfileString(L"CommonDB", L"Count", sz_Count, m_szEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						ZeroMemory(szWWizData, sizeof(szWWizData));
						//CString csAppend = L"";
						if ((_tcscmp(pFileName, L"DAILY.CLD") == 0) || (_tcscmp(pFileName, L"MAIN.CVD") == 0) || (_tcscmp(pFileName, L"WRDWIZWHLST.FP") == 0))
						{
							csAppend = L"\,NC\,Y";
						}
						else
						{
							csAppend = L"\,YC\,N";
						}
						swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
						dwRetValue = WritePrivateProfileString(L"CommonDB", L"Count", sz_Count, m_szWWizEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"Elite") == 0)
					{
						ZeroMemory(m_szEliteCount, sizeof(m_szEliteCount));
						m_iEliteCount = 0;
						GetPrivateProfileString(L"Elite", L"Count", L"", m_szEliteCount, 255, m_szEliteIniPath);
						m_iEliteCount = _ttoi(m_szEliteCount);
						m_iEliteCount++;
						sz_Count.Format(L"%d", m_iEliteCount);
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(L"Elite", L"Count", sz_Count, m_szEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						ZeroMemory(szWWizData, sizeof(szWWizData));
						csAppend = L"\,YC\,N";
						swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(L"Elite", L"Count", sz_Count, m_szWWizEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"ProductVersion") == 0)
					{

						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}

					}
					else if (_tcscmp(pKey, L"DatabaseVersion") == 0)
					{
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"ScanEngineVersion") == 0)
					{
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szBasicIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizBasicIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"DataEncVersion") == 0)
					{
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
					if (_tcscmp(pKey, L"RegOpt") == 0)
					{
						WriteKeyValueToIni(pKey, pValue, pData, m_szEliteIniPath, m_szWWizEliteIniPath);
					}
					else if (_tcscmp(pKey, L"DllRegister") == 0)
					{
						WriteKeyValueToIni(pKey, pValue, pData, m_szEliteIniPath, m_szWWizEliteIniPath);
					}
					else if (_tcscmp(pKey, L"AddedFeatureEss") == 0)
					{
						WriteKeyValueToIni(L"AddedFeature", pValue, pData, m_szEliteIniPath, m_szWWizEliteIniPath);
					}
					else if (_tcscmp(pKey, L"EnhancedFunctionalityEss") == 0)
					{
						WriteKeyValueToIni(L"EnhancedFunctionality", pValue, pData, m_szEliteIniPath, m_szWWizEliteIniPath);
					}
					else
					{
						dwRetValue = 1;
					}
				}
				else
				{
					if (_tcscmp(pKey, L"ProductVersion") == 0)
					{

						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"DatabaseVersion") == 0)
					{

						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"ScanEngineVersion") == 0)
					{
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szBasicIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumkALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizBasicIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"DataEncVersion") == 0)
					{

						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"RegOpt") == 0)
					{
						WriteKeyValueToIni(pKey, pValue, pData, m_szEliteIniPath, m_szWWizEliteIniPath);
					}
					else if (_tcscmp(pKey, L"DllRegister") == 0)
					{
						WriteKeyValueToIni(pKey, pValue, pData, m_szEliteIniPath, m_szWWizEliteIniPath);
					}
					else if (_tcscmp(pKey, L"AddedFeatureElite") == 0)
					{
						WriteKeyValueToIni(L"AddedFeature", pValue, pData, m_szEliteIniPath, m_szWWizEliteIniPath);
					}
					else if (_tcscmp(pKey, L"EnhancedFunctionalityElite") == 0)
					{
						WriteKeyValueToIni(L"EnhancedFunctionality", pValue, pData, m_szEliteIniPath, m_szWWizEliteIniPath);
					}

					if (_tcscmp(pKey, L"Elite") == 0)
					{
						if (_tcscmp(pValue, L"PRODUCTSETTINGS.INI") == 0)
						{
							ZeroMemory(m_szEliteCount, sizeof(m_szEliteCount));
							m_iEliteCount = 0;
							GetPrivateProfileString(L"Elite", L"Count", L"", m_szEliteCount, 255, m_szEliteIniPath);
							m_iEliteCount = _ttoi(m_szEliteCount);
							m_iEliteCount++;
							sz_Count.Format(L"%d", m_iEliteCount);
							dwRetValue = WritePrivateProfileString(L"Elite", L"Count", sz_Count, m_szEliteIniPath);
							if (!dwRetValue)
							{
								AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
								return dwRetValue;
							}
							dwRetValue = WritePrivateProfileString(L"Elite", sz_Count, pData, m_szEliteIniPath);
							if (!dwRetValue)
							{
								AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
								return dwRetValue;
							}
						}
					}
					else
					{
						dwRetValue = 1;
					}
				}
			}
			if (m_ProdType_EssPlus.GetCheck() == BST_CHECKED)
			{
				if (m_listEssentialPlus.Find(CString(pFileName)))
				{
					if (_tcscmp(pKey, L"Files_32") == 0)
					{
						ZeroMemory(m_szEssentialPlusCount, sizeof(m_szEssentialPlusCount));
						m_iEssentialPlusCount = 0;
						GetPrivateProfileString(L"Count_32", L"Count", L"", m_szEssentialPlusCount, 255, m_szEssentialPlusIniPath);
						m_iEssentialPlusCount = _ttoi(m_szEssentialPlusCount);
						m_iEssentialPlusCount++;
						sz_Count.Format(L"%d", m_iEssentialPlusCount);
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(L"Count_32", L"Count", sz_Count, m_szEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						ZeroMemory(szWWizData, sizeof(szWWizData));
						csAppend = L"\,YC\,N";
						swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(L"Count_32", L"Count", sz_Count, m_szWWizEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"Files_64") == 0)
					{
						ZeroMemory(m_szEssentialPlusCount, sizeof(m_szEssentialPlusCount));
						m_iEssentialPlusCount = 0;
						GetPrivateProfileString(L"Count_64", L"Count", L"", m_szEssentialPlusCount, 255, m_szEssentialPlusIniPath);
						m_iEssentialPlusCount = _ttoi(m_szEssentialPlusCount);
						m_iEssentialPlusCount++;
						sz_Count.Format(L"%d", m_iEssentialPlusCount);
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(L"Count_64", L"Count", sz_Count, m_szEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						ZeroMemory(szWWizData, sizeof(szWWizData));
						csAppend = L"\,YC\,N";
						swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}

						dwRetValue = WritePrivateProfileString(L"Count_64", L"Count", sz_Count, m_szWWizEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"Common") == 0)
					{
						ZeroMemory(m_szEssentialPlusCount, sizeof(m_szEssentialPlusCount));
						m_iEssentialPlusCount = 0;
						GetPrivateProfileString(L"Common", L"Count", L"", m_szEssentialPlusCount, 255, m_szEssentialPlusIniPath);
						m_iEssentialPlusCount = _ttoi(m_szEssentialPlusCount);
						m_iEssentialPlusCount++;
						sz_Count.Format(L"%d", m_iEssentialPlusCount);
						dwRetValue = WritePrivateProfileString(L"Common", L"Count", sz_Count, m_szEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						ZeroMemory(szWWizData, sizeof(szWWizData));
						//CString csAppend = L"";
						if ((_tcscmp(pFileName, L"DAILY.CLD") == 0) || (_tcscmp(pFileName, L"MAIN.CVD") == 0) || (_tcscmp(pFileName, L"WRDWIZWHLST.FP") == 0))
						{
							csAppend = L"\,NC\,Y";
						}
						else
						{
							csAppend = L"\,YC\,N";
						}
						swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
						dwRetValue = WritePrivateProfileString(L"Common", L"Count", sz_Count, m_szWWizEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"CommonDB") == 0)
					{
						ZeroMemory(m_szEssentialPlusCount, sizeof(m_szEssentialPlusCount));
						m_iEssentialPlusCount = 0;
						GetPrivateProfileString(L"CommonDB", L"Count", L"", m_szEssentialPlusCount, 255, m_szEssentialPlusIniPath);

						if (_tcslen(m_szEssentialPlusCount) != 0x00)
						{
							m_iEssentialPlusCount = _ttoi(m_szEssentialPlusCount);
						}

						m_iEssentialPlusCount++;
						sz_Count.Format(L"%d", m_iEssentialPlusCount);
						dwRetValue = WritePrivateProfileString(L"CommonDB", L"Count", sz_Count, m_szEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						ZeroMemory(szWWizData, sizeof(szWWizData));
						//CString csAppend = L"";
						if ((_tcscmp(pFileName, L"DAILY.CLD") == 0) || (_tcscmp(pFileName, L"MAIN.CVD") == 0) || (_tcscmp(pFileName, L"WRDWIZWHLST.FP") == 0))
						{
							csAppend = L"\,NC\,Y";
						}
						else
						{
							csAppend = L"\,YC\,N";
						}
						swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
						dwRetValue = WritePrivateProfileString(L"CommonDB", L"Count", sz_Count, m_szWWizEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"EssentialPlus") == 0)
					{
						ZeroMemory(m_szEssentialPlusCount, sizeof(m_szEssentialPlusCount));
						m_iEssentialPlusCount = 0;
						GetPrivateProfileString(L"EssentialPlus", L"Count", L"", m_szEssentialPlusCount, 255, m_szEssentialPlusIniPath);
						m_iEssentialPlusCount = _ttoi(m_szEssentialPlusCount);
						m_iEssentialPlusCount++;
						sz_Count.Format(L"%d", m_iEssentialPlusCount);
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(L"EssentialPlus", L"Count", sz_Count, m_szEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						ZeroMemory(szWWizData, sizeof(szWWizData));
						csAppend = L"\,YC\,N";
						swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
						dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(L"EssentialPlus", L"Count", sz_Count, m_szWWizEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"ProductVersion") == 0)
					{

						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}

					}
					else if (_tcscmp(pKey, L"DatabaseVersion") == 0)
					{
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"ScanEngineVersion") == 0)
					{
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szBasicIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizBasicIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"DataEncVersion") == 0)
					{
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
					if (_tcscmp(pKey, L"RegOpt") == 0)
					{
						WriteKeyValueToIni(pKey, pValue, pData, m_szEssentialPlusIniPath, m_szWWizEssentialPlusIniPath);
					}
					else if (_tcscmp(pKey, L"DllRegister") == 0)
					{
						WriteKeyValueToIni(pKey, pValue, pData, m_szEssentialPlusIniPath, m_szWWizEssentialPlusIniPath);
					}
					else if (_tcscmp(pKey, L"AddedFeatureEssPlus") == 0)
					{
						WriteKeyValueToIni(L"AddedFeature", pValue, pData, m_szEssentialPlusIniPath, m_szWWizEssentialPlusIniPath);
					}
					else if (_tcscmp(pKey, L"EnhancedFunctionalityEssPlus") == 0)
					{
						WriteKeyValueToIni(L"EnhancedFunctionality", pValue, pData, m_szEssentialPlusIniPath, m_szWWizEssentialPlusIniPath);
					}
					else
					{
						dwRetValue = 1;
					}
				}
				else
				{
					if (_tcscmp(pKey, L"ProductVersion") == 0)
					{

						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"DatabaseVersion") == 0)
					{

						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"ScanEngineVersion") == 0)
					{
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szBasicIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizBasicIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumzALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"DataEncVersion") == 0)
					{

						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEssentialPlusIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
					else if (_tcscmp(pKey, L"RegOpt") == 0)
					{
						WriteKeyValueToIni(pKey, pValue, pData, m_szEssentialPlusIniPath, m_szWWizEssentialPlusIniPath);
					}
					else if (_tcscmp(pKey, L"DllRegister") == 0)
					{
						WriteKeyValueToIni(pKey, pValue, pData, m_szEssentialPlusIniPath, m_szWWizEssentialPlusIniPath);
					}
					else if (_tcscmp(pKey, L"AddedFeatureEssPlus") == 0)
					{
						WriteKeyValueToIni(L"AddedFeature", pValue, pData, m_szEssentialPlusIniPath, m_szWWizEssentialPlusIniPath);
					}
					else if (_tcscmp(pKey, L"EnhancedFunctionalityEssPlus") == 0)
					{
						WriteKeyValueToIni(L"EnhancedFunctionality", pValue, pData, m_szEssentialPlusIniPath, m_szWWizEssentialPlusIniPath);
					}

					if (_tcscmp(pKey, L"EssentialPlus") == 0)
					{
						if (_tcscmp(pValue, L"PRODUCTSETTINGS.INI") == 0)
						{
							ZeroMemory(m_szEssentialPlusCount, sizeof(m_szEssentialPlusCount));
							m_iEssentialPlusCount = 0;
							GetPrivateProfileString(L"EssentialPlus", L"Count", L"", m_szEssentialPlusCount, 255, m_szEssentialPlusIniPath);
							m_iEssentialPlusCount = _ttoi(m_szEssentialPlusCount);
							m_iEssentialPlusCount++;
							sz_Count.Format(L"%d", m_iEssentialPlusCount);
							dwRetValue = WritePrivateProfileString(L"EssentialPlus", L"Count", sz_Count, m_szEssentialPlusIniPath);
							if (!dwRetValue)
							{
								AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
								return dwRetValue;
							}
							dwRetValue = WritePrivateProfileString(L"EssentialPlus", sz_Count, pData, m_szEssentialPlusIniPath);
							if (!dwRetValue)
							{
								AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
								return dwRetValue;
							}
						}
					}
					else
					{
						dwRetValue = 1;
					}
				}
			}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::WriteToIni");
	}

	return dwRetValue;
}

/***************************************************************************
Function Name  : WriteToEPSServerIni
Description    : This function is called to write entries of patches into ELITEEPSSERVER INI files
Author Name    : Tejas Tanaji Shinde
Date           : 8 May,2018
SR_NO		   :
****************************************************************************/
DWORD CWardWizALUpdDlg::WriteToEPSServerIni(TCHAR *pKey, TCHAR *pValue, TCHAR *pData, TCHAR *pFileName)
{
	DWORD	dwRetValue = 0x00;
	CString sz_Count = L"";
	TCHAR szWWizData[512] = { 0 };
	CString csAppend = L"";

	if (pFileName != NULL)
	{
		pFileName = _tcsupr(pFileName);
	}

	POSITION pos = m_listBasic.GetHeadPosition();
	try
	{
		if (m_ProdType_Elite.GetCheck() == BST_CHECKED)
		{
			if (m_listElite.Find(CString(pFileName)))
			{
				if (_tcscmp(pKey, L"Files_32") == 0)
				{
					ZeroMemory(m_szEliteCount, sizeof(m_szEliteCount));
					m_iEliteCount = 0;
					GetPrivateProfileString(L"Count_32", L"Count", L"", m_szEliteCount, 255, m_szWWizEliteServerIniPath);
					m_iEliteCount = _ttoi(m_szEliteCount);
					m_iEliteCount++;
					sz_Count.Format(L"%d", m_iEliteCount);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"Count_32", L"Count", sz_Count, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));
					csAppend = L"\,YC\,N";
					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"Count_32", L"Count", sz_Count, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"Files_64") == 0)
				{
					ZeroMemory(m_szEliteCount, sizeof(m_szEliteCount));
					m_iEliteCount = 0;
					GetPrivateProfileString(L"Count_64", L"Count", L"", m_szEliteCount, 255, m_szWWizEliteServerIniPath);
					m_iEliteCount = _ttoi(m_szEliteCount);
					m_iEliteCount++;
					sz_Count.Format(L"%d", m_iEliteCount);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"Count_64", L"Count", sz_Count, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));
					csAppend = L"\,YC\,N";
					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}

					dwRetValue = WritePrivateProfileString(L"Count_64", L"Count", sz_Count, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"Common") == 0)
				{
					ZeroMemory(m_szEliteCount, sizeof(m_szEliteCount));
					m_iEliteCount = 0;
					GetPrivateProfileString(L"Common", L"Count", L"", m_szEliteCount, 255, m_szWWizEliteServerIniPath);
					m_iEliteCount = _ttoi(m_szEliteCount);
					m_iEliteCount++;
					sz_Count.Format(L"%d", m_iEliteCount);
					dwRetValue = WritePrivateProfileString(L"Common", L"Count", sz_Count, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));
					//CString csAppend = L"";
					if ((_tcscmp(pFileName, L"DAILY.CLD") == 0) || (_tcscmp(pFileName, L"MAIN.CVD") == 0) || (_tcscmp(pFileName, L"WRDWIZWHLST.FP") == 0))
					{
						csAppend = L"\,NC\,Y";
					}
					else
					{
						csAppend = L"\,YC\,N";
					}
					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
					dwRetValue = WritePrivateProfileString(L"Common", L"Count", sz_Count, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"CommonDB") == 0)
				{
					ZeroMemory(m_szEliteCount, sizeof(m_szEliteCount));
					m_iEliteCount = 0;
					GetPrivateProfileString(L"CommonDB", L"Count", L"", m_szEliteCount, 255, m_szWWizEliteServerIniPath);

					if (_tcslen(m_szEliteCount) != 0x00)
					{
						m_iEliteCount = _ttoi(m_szEliteCount);
					}

					m_iEliteCount++;
					sz_Count.Format(L"%d", m_iEliteCount);
					dwRetValue = WritePrivateProfileString(L"CommonDB", L"Count", sz_Count, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));
					//CString csAppend = L"";
					if ((_tcscmp(pFileName, L"DAILY.CLD") == 0) || (_tcscmp(pFileName, L"MAIN.CVD") == 0) || (_tcscmp(pFileName, L"WRDWIZWHLST.FP") == 0))
					{
						csAppend = L"\,NC\,Y";
					}
					else
					{
						csAppend = L"\,YC\,N";
					}
					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
					dwRetValue = WritePrivateProfileString(L"CommonDB", L"Count", sz_Count, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"Elite") == 0)
				{
					ZeroMemory(m_szEliteCount, sizeof(m_szEliteCount));
					m_iEliteCount = 0;
					GetPrivateProfileString(L"EliteServer", L"Count", L"", m_szEliteCount, 255, m_szWWizEliteServerIniPath);
					m_iEliteCount = _ttoi(m_szEliteCount);
					m_iEliteCount++;
					sz_Count.Format(L"%d", m_iEliteCount);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"EliteServer", L"Count", sz_Count, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					ZeroMemory(szWWizData, sizeof(szWWizData));
					csAppend = L"\,YC\,N";
					swprintf_s(szWWizData, _countof(szWWizData), L"%s%s", pData, csAppend);
					dwRetValue = WritePrivateProfileString(pKey, sz_Count, szWWizData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(L"EliteServer", L"Count", sz_Count, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", szWWizData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"ProductVersion") == 0)
				{

					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}

				}
				else if (_tcscmp(pKey, L"DatabaseVersion") == 0)
				{
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"ScanEngineVersion") == 0)
				{
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizBasicIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"DataEncVersion") == 0)
				{
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
			}
			else
			{
				if (_tcscmp(pKey, L"ProductVersion") == 0)
				{

					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"DatabaseVersion") == 0)
				{

					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"ScanEngineVersion") == 0)
				{
					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}
				else if (_tcscmp(pKey, L"DataEncVersion") == 0)
				{

					dwRetValue = WritePrivateProfileString(pKey, pValue, pData, m_szWWizEliteServerIniPath);
					if (!dwRetValue)
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
						return dwRetValue;
					}
				}

				if (_tcscmp(pKey, L"Elite") == 0)
				{
					if (_tcscmp(pValue, L"PRODUCTSETTINGS.INI") == 0)
					{
						ZeroMemory(m_szEliteCount, sizeof(m_szEliteCount));
						m_iEliteCount = 0;
						GetPrivateProfileString(L"EliteServer", L"Count", L"", m_szEliteServerCount, 255, m_szWWizEliteServerIniPath);
						m_iEliteCount = _ttoi(m_szEliteCount);
						m_iEliteCount++;
						sz_Count.Format(L"%d", m_iEliteCount);
						dwRetValue = WritePrivateProfileString(L"EliteServer", L"Count", sz_Count, m_szWWizEliteServerIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
						dwRetValue = WritePrivateProfileString(L"EliteServer", sz_Count, pData, m_szWWizEliteServerIniPath);
						if (!dwRetValue)
						{
							AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
							return dwRetValue;
						}
					}
				}
				else
				{
					dwRetValue = 1;
				}
			}
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::WriteToEPSServerIni");
	}

	return dwRetValue;
}
/***************************************************************************
  Function Name  : Thread_GenerateUpdateFiles
  Description    : this function is called to create updated files for 32/64 bit as per selection and Build/Rebuild
				   and creting zip.
				   patches will be created as per user selection
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0018
****************************************************************************/
DWORD WINAPI Thread_GenerateUpdateFiles( LPVOID lpParam )
{
	Sleep( 1000 );
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *) lpParam;
	swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Process started" );
	AddLogEntry(L">>>CVibraniumkALUpdDlg :: Thread_GenerateUpdateFiles : Process started");
	bool	bBasicTicked = false ;
	bool	bEssentialTicked = false ;
	bool	bProTicked = false ;
	bool	bEliteTicked = false ;
	bool    bEssentailPlusTicked = false;
	bool	m_bBuildRebuild = false;
	bool	bChkAll = false;
	TCHAR	szTemp[1024] = {0};
	TCHAR	szInnoPath[1024] = {0};
	TCHAR	szProgramDirX86[256];

	DWORD	dwRet = 0x00;

	try
	{

		//pWWALUpdDlg->m_edtDatabaseVersion.EnableWindow(FALSE);
		//pWWALUpdDlg->m_edtProdVersion.EnableWindow(FALSE);

		pWWALUpdDlg->UpdateWWBinary(lpParam);

		if( pWWALUpdDlg->m_ProdType_Basic.GetCheck() == BST_CHECKED )
				bBasicTicked = true ;

		if( pWWALUpdDlg->m_ProdType_Essential.GetCheck() == BST_CHECKED )
				bEssentialTicked = true ;

		if( pWWALUpdDlg->m_ProdType_Pro.GetCheck() == BST_CHECKED )
				bProTicked = true ;

		if( pWWALUpdDlg->m_ProdType_Elite.GetCheck() == BST_CHECKED )
				bEliteTicked = true ;

		if (pWWALUpdDlg->m_ProdType_EssPlus.GetCheck() == BST_CHECKED)
			bEssentailPlusTicked = true;
		
		if (bBasicTicked)
		{
			ZeroMemory(pWWALUpdDlg->m_szBasicIniPath, sizeof(pWWALUpdDlg->m_szBasicIniPath));
			swprintf(pWWALUpdDlg->m_szBasicIniPath, L"%s\\ALUpdFiles\\%s\\WWPatchB.ini", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);

			ZeroMemory(pWWALUpdDlg->m_szWWizBasicIniPath, sizeof(pWWALUpdDlg->m_szWWizBasicIniPath));
			swprintf(pWWALUpdDlg->m_szWWizBasicIniPath, L"%s\\ALUpdFiles\\%s\\WWizPatchB.ini", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
		}

		if (bEssentialTicked)
		{
			ZeroMemory(pWWALUpdDlg->m_szEssentialIniPath, sizeof(pWWALUpdDlg->m_szEssentialIniPath));
			swprintf(pWWALUpdDlg->m_szEssentialIniPath, L"%s\\ALUpdFiles\\%s\\VBPatchTS.ini", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);

			ZeroMemory(pWWALUpdDlg->m_szWWizEssentialIniPath, sizeof(pWWALUpdDlg->m_szWWizEssentialIniPath));
			swprintf(pWWALUpdDlg->m_szWWizEssentialIniPath, L"%s\\ALUpdFiles\\%s\\VibroPatchTS.ini", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
		}

		if (bProTicked)
		{
			ZeroMemory(pWWALUpdDlg->m_szProIniPath, sizeof(pWWALUpdDlg->m_szProIniPath));
			swprintf(pWWALUpdDlg->m_szProIniPath, L"%s\\ALUpdFiles\\%s\\WWizPatchP.ini", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
		}

		if (bEliteTicked)
		{
			ZeroMemory(pWWALUpdDlg->m_szEliteIniPath, sizeof(pWWALUpdDlg->m_szEliteIniPath));
			swprintf(pWWALUpdDlg->m_szEliteIniPath, L"%s\\ALUpdFiles\\%s\\WWPatchT.ini", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);

			ZeroMemory(pWWALUpdDlg->m_szWWizEliteIniPath, sizeof(pWWALUpdDlg->m_szWWizEliteIniPath));
			swprintf(pWWALUpdDlg->m_szWWizEliteIniPath, L"%s\\ALUpdFiles\\%s\\WWizPatchT.ini", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);

			ZeroMemory(pWWALUpdDlg->m_szWWizEliteServerIniPath, sizeof(pWWALUpdDlg->m_szWWizEliteServerIniPath));
			swprintf(pWWALUpdDlg->m_szWWizEliteServerIniPath, L"%s\\ALUpdFiles\\%s\\WWizPatchEPS.ini", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
		}
		
		if (bEssentailPlusTicked)
		{
			ZeroMemory(pWWALUpdDlg->m_szEssentialPlusIniPath, sizeof(pWWALUpdDlg->m_szEssentialPlusIniPath));
			swprintf(pWWALUpdDlg->m_szEssentialPlusIniPath, L"%s\\ALUpdFiles\\%s\\VBPatchAS.ini", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);

			ZeroMemory(pWWALUpdDlg->m_szWWizEssentialPlusIniPath, sizeof(pWWALUpdDlg->m_szWWizEssentialPlusIniPath));
			swprintf(pWWALUpdDlg->m_szWWizEssentialPlusIniPath, L"%s\\ALUpdFiles\\%s\\VibroPatchAS.ini", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
		}

		if(pWWALUpdDlg->m_chkPatchType.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_PatchType_Product.GetCheck() == BST_CHECKED)
		{
						bChkAll = true;
						gbIsBatchFileRunning= false;
						if( !pWWALUpdDlg )
							{
								AddLogEntry(L"### Failed in CVibraniumALUpdDlg::For Object Address");
								pWWALUpdDlg->m_Generate_Update_Files.EnableWindow( );
								dwRet = 1;
								goto Cleanup;
							}

						pWWALUpdDlg->m_Close.EnableWindow( );

						TCHAR	szSour[512] = {0};
						TCHAR	szDest[512] = {0};

						pWWALUpdDlg->m_ListItemCount = 0x00;
						pWWALUpdDlg->m_List_PatchFiles.DeleteAllItems();

						if( (pWWALUpdDlg->m_Rebuild32.GetCheck() != BST_CHECKED ) && ( pWWALUpdDlg->m_Rebuild64.GetCheck() != BST_CHECKED ) )
							{
								AddLogEntry(L"### Rebuild neither in 32 nor in 64 mode selected. Please select one of the mode.");
								dwRet = 12;
								goto Cleanup;
							}


						if (pWWALUpdDlg->RebuildAllVCProject(L"Release", bProTicked))
							{
								AddLogEntry(L"### Failed in CVibraniumALUpdDlg::RebuildAllVCProject");
								dwRet = 11;
								goto Cleanup;
							}

						if (bProTicked)
						{
							if (pWWALUpdDlg->RebuildAllVCProject(L"ReleaseOutlookPlugin", bProTicked))
							{
								AddLogEntry(L"### Failed in CVibraniumzALUpdDlg::RebuildAllVCProject");
								dwRet = 11;
								goto Cleanup;
							}
						}
						
						
						m_bBuildRebuild = true;
						swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Loading required DLLS" );
						AddLogEntry(L">>>CVibraniumALUpdDlg :: Thread_GenerateUpdateFiles : Loading required DLLS");
						if( pWWALUpdDlg->LoadRequiredDLLs())
							{
								dwRet = 12;
								goto Cleanup;
							}


						CString csInnoPath = L"";
						CString csISSPath = L"";

						if (!pWWALUpdDlg->GetInnoAndAppPath(csInnoPath, csISSPath))
						{
							goto Cleanup;
						}

					if(pWWALUpdDlg->m_chkBuildType.GetCheck() == BST_CHECKED)
						{
							if (PathFileExists(L"D:\\WardWizDevRightHeur\\Release\\Win32\\Binaries\\VBSETUPDLL.DLL"))
							{
								pWWALUpdDlg->AddSignToSpecifiedFile(L"D:\\WardWizDevRightHeur\\Release\\Win32\\Binaries\\VBSETUPDLL.DLL");
							}
							swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Basic inno setup rebuilding + WardwizUtility_32.iss");
							swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizUtility_32.iss\"", csInnoPath, csISSPath);
							if (pWWALUpdDlg->RebuildSolution(szTemp))
								return 3;

							swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"TS inno setup rebuilding + WardwizUtility_64.iss");
							swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizUtility_64.iss\"", csInnoPath, csISSPath);
							if (pWWALUpdDlg->RebuildSolution(szTemp))
								return 3;
							
						}

						pWWALUpdDlg->WriteVersions(lpParam);
						pWWALUpdDlg->mbIsFileSelected= true;

						pWWALUpdDlg->CreateZip32(lpParam);

						pWWALUpdDlg->CreateZip64(lpParam);

 						pWWALUpdDlg->CreateZipForDrivers32(lpParam);

						pWWALUpdDlg->CreateZipForDrivers64(lpParam);

						pWWALUpdDlg->mbIsFileSelected= false;
						pWWALUpdDlg->CreateZipForDB(lpParam);
						
						pWWALUpdDlg->CreateZipForEmailScanFiles(lpParam);

						pWWALUpdDlg->CreateZipForWaveFiles(lpParam);

						pWWALUpdDlg->CreateZipForLogoFiles(lpParam);

						pWWALUpdDlg->CreateZipForSecNewsFiles(lpParam);
								
						pWWALUpdDlg->CreateZipForSetting(lpParam);

						pWWALUpdDlg->CreateZipForHelpFiles(lpParam);
						
						pWWALUpdDlg->CreateZipForSciterDLL(lpParam);

						pWWALUpdDlg->CreateZipForSqliteDLL(lpParam);

						pWWALUpdDlg->CreateZipForNSSFolder(lpParam);

						pWWALUpdDlg->CreateZipForSETUPDBFolder(lpParam);
						
						//Rar files should be added in Updates
						//Implemented by: Nitin Kolapkar date: 18th Feb 2016
						pWWALUpdDlg->CreateZipForRarFiles(lpParam);
						swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files for Product specific" );
						AddLogEntry(L">>>CVibraniumALUpdDlg :: Thread_GenerateUpdateFiles : Creating Zip files for Product specific");

				if( bBasicTicked )
					{
						pWWALUpdDlg->m_ProdType_Pro.SetCheck(BST_UNCHECKED);
						pWWALUpdDlg->m_ProdType_Elite.SetCheck(BST_UNCHECKED);
						pWWALUpdDlg->m_ProdType_EssPlus.SetCheck(BST_UNCHECKED);

						pWWALUpdDlg->m_FilesCount = 0x01;
						swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\SETTINGS\\BASICSETTING", pWWALUpdDlg->m_szApplPath);
						swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\Basic", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
						pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Basic", L"AppFolder\\VBSETTINGS");

						if( pWWALUpdDlg->m_FilesCount > 0x01 )
							{
								ZeroMemory(szSour, sizeof(szSour) );
								swprintf_s(szSour, _countof(szSour), L"%lu", (pWWALUpdDlg->m_FilesCount-1));
										//				pWWALUpdDlg->WriteToIni(L"Basic", L"Count", szSour, NULL);
							}
							else
							 {
								AddLogEntry(L"### Failed in CVibraniumALUpdDlg::Thread_GenerateUpdateFiles for Basic files count");
								dwRet = 4;
								goto Cleanup;
							  }
					  }

			if( bEssentialTicked )
			{
				pWWALUpdDlg->m_ProdType_Pro.SetCheck(BST_UNCHECKED);
				pWWALUpdDlg->m_ProdType_Elite.SetCheck(BST_UNCHECKED);
				pWWALUpdDlg->m_ProdType_EssPlus.SetCheck(BST_UNCHECKED);

				swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\SETTINGS\\ESSENTIALSETTING", pWWALUpdDlg->m_szApplPath);
				swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\Essential", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
				pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Essential", L"AppFolder\\VBSETTINGS");
				if( pWWALUpdDlg->m_FilesCount > 0x01 )
				{
					ZeroMemory(szSour, sizeof(szSour) );
					swprintf_s(szSour, _countof(szSour), L"%lu", (pWWALUpdDlg->m_FilesCount-1));
					//				pWWALUpdDlg->WriteToIni(L"Basic", L"Count", szSour, NULL);
				}
				else
				{
					AddLogEntry(L"### Failed in CVibraniumALUpdDlg::Thread_GenerateUpdateFiles for Basic files count");
					dwRet = 4;
					goto Cleanup;
				}
			}
			if( bProTicked )
			{
				pWWALUpdDlg->m_ProdType_Pro.SetCheck(BST_CHECKED);
				pWWALUpdDlg->m_ProdType_Essential.SetCheck(BST_UNCHECKED);
				pWWALUpdDlg->m_ProdType_Elite.SetCheck(BST_UNCHECKED);
				pWWALUpdDlg->m_ProdType_EssPlus.SetCheck(BST_UNCHECKED);

				pWWALUpdDlg->m_FilesCount = 0x01;
				swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\SETTINGS\\PROPRDSETTING", pWWALUpdDlg->m_szApplPath);
				swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\Pro", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
				pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Pro", L"AppFolder\\VBSETTINGS\\PROPRDSETTING");
				if( pWWALUpdDlg->m_FilesCount > 0x01 )
				{
					ZeroMemory(szSour, sizeof(szSour) );
					swprintf_s(szSour, _countof(szSour), L"%lu", (pWWALUpdDlg->m_FilesCount-1));
					pWWALUpdDlg->WriteToIni(L"Pro", L"Count", szSour, NULL);
				}
				else
				{
					AddLogEntry(L"### Failed in CVibraniumALUpdDlg::Thread_GenerateUpdateFiles for Pro files count");
					dwRet = 5;
					goto Cleanup;
				}
			}
			if( bEliteTicked )
			{
				pWWALUpdDlg->m_ProdType_Essential.SetCheck(BST_UNCHECKED);
				pWWALUpdDlg->m_ProdType_Pro.SetCheck(BST_UNCHECKED);
				pWWALUpdDlg->m_ProdType_Elite.SetCheck(BST_CHECKED);
				pWWALUpdDlg->m_ProdType_EssPlus.SetCheck(BST_UNCHECKED);

				pWWALUpdDlg->m_FilesCount = 0x01;
				swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\SETTINGS\\ELITESETTING", pWWALUpdDlg->m_szApplPath);
				swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\Elite", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
				pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Elite", L"AppFolder\\VBSETTINGS\\ELITESETTING");
				if( pWWALUpdDlg->m_FilesCount > 0x01 )
				{
				ZeroMemory(szSour, sizeof(szSour) );
				swprintf_s(szSour, _countof(szSour), L"%lu", (pWWALUpdDlg->m_FilesCount-1));	
				pWWALUpdDlg->WriteToIni(L"Elite", L"Count", szSour, NULL);
				}
				else
				{
				AddLogEntry(L"### Failed in CVibraniumALUpdDlg::Thread_GenerateUpdateFiles for Elite files count");
				dwRet = 6;
				goto Cleanup;
				}
			}
			if (bEssentailPlusTicked)
			{
				pWWALUpdDlg->m_ProdType_Pro.SetCheck(BST_UNCHECKED);
				pWWALUpdDlg->m_ProdType_Essential.SetCheck(BST_UNCHECKED); 
				pWWALUpdDlg->m_ProdType_Elite.SetCheck(BST_UNCHECKED);
				pWWALUpdDlg->m_ProdType_EssPlus.SetCheck(BST_CHECKED);

				swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\SETTINGS\\ESSENTIALPLUSSETTING", pWWALUpdDlg->m_szApplPath);
				swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\EssentialPlus", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
				pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"EssentialPlus", L"AppFolder\\VBSETTINGS\\ESSENTIALPLUSSETTING");
				if (pWWALUpdDlg->m_FilesCount > 0x01)
				{
					ZeroMemory(szSour, sizeof(szSour));
					swprintf_s(szSour, _countof(szSour), L"%lu", (pWWALUpdDlg->m_FilesCount - 1));
					pWWALUpdDlg->WriteToIni(L"EssentialPlus", L"Count", szSour, NULL);
				}
				else
				{
					AddLogEntry(L"### Failed in CVibraniumALUpdDlg::Thread_GenerateUpdateFiles for EssentialPlus files count");
					dwRet = 4;
					goto Cleanup;
				}
			}
			if( bBasicTicked )
				pWWALUpdDlg->m_ProdType_Basic.SetCheck(BST_CHECKED);
			if( bEssentialTicked )
				pWWALUpdDlg->m_ProdType_Essential.SetCheck(BST_CHECKED);
			if( bProTicked )
				pWWALUpdDlg->m_ProdType_Pro.SetCheck(BST_CHECKED);
			if( bEliteTicked )
				pWWALUpdDlg->m_ProdType_Elite.SetCheck(BST_CHECKED);
			if (bEssentailPlusTicked)
				pWWALUpdDlg->m_ProdType_EssPlus.SetCheck(BST_CHECKED);

			pWWALUpdDlg->m_Generate_Update_Files.EnableWindow( );

			swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Patch created successfully in::%s", pWWALUpdDlg->m_szOutDirName );

			AddLogEntry(L">>>CVibraniumALUpdDlg ::Thread_GenerateUpdateFiles: Patch Created successfully");

			//AfxMessageBox( L"Patch Created successfully" );

		}
		if(pWWALUpdDlg->m_PatchType_Virus.GetCheck() == BST_CHECKED)
		{
			bChkAll = true;
			pWWALUpdDlg->m_ListItemCount = 0x00;
			pWWALUpdDlg->LoadRequiredDLLs();
			pWWALUpdDlg->CreateZipForDB(lpParam);
		}
		if(pWWALUpdDlg->m_MakeSetup.GetCheck() == BST_CHECKED)
		{
			bChkAll = true;
			//if(pWWALUpdDlg->m_chkPatchType.GetCheck() == BST_CHECKED && m_bBuildRebuild == false)
			
			if(pWWALUpdDlg->m_chkBuildType.GetCheck() == BST_CHECKED && m_bBuildRebuild == false)
			{
				/*if( pWWALUpdDlg->RebuildAllVCProject( ) )
				{
					AddLogErntry(L"### Failed in CWardWizALUpdDlg::RebuildAllVCProject");
					dwRet = 11;
					goto Cleanup;
				}*/
				
				if (pWWALUpdDlg->RebuildAllVCProject(L"Release", bProTicked))
				{
					AddLogEntry(L"### Failed in CVibraniumALUpdDlg::RebuildAllVCProject");
					dwRet = 11;
					goto Cleanup;
				}

				if (bProTicked)
				{
					if (pWWALUpdDlg->RebuildAllVCProject(L"ReleaseOutlookPlugin", bProTicked))
					{
						AddLogEntry(L"### Failed in CVibraniumALUpdDlg::RebuildAllVCProject");
						dwRet = 11;
						goto Cleanup;
					}
				}
			}
			

				CString csInnoPath = L"";
				CString csISSPath = L"";

				if (!pWWALUpdDlg->GetInnoAndAppPath(csInnoPath,csISSPath))
				{
					goto Cleanup;
				}

				if(bBasicTicked)
				{	gbIsBatchFileRunning= false;
					if( pWWALUpdDlg->m_Rebuild32.GetCheck( ) == BST_CHECKED && pWWALUpdDlg->m_chkwithClam.GetCheck() ==  BST_CHECKED )
					{
						swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Basic inno setup rebuilding + WardwizBasicSetupx86.iss" );

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizBasicSetupx86.iss\"", csInnoPath, csISSPath);
						if( pWWALUpdDlg->RebuildSolution( szTemp ) )
							return 3;
					}
					if( pWWALUpdDlg->m_Rebuild64.GetCheck( ) == BST_CHECKED && pWWALUpdDlg->m_chkwithClam.GetCheck() ==  BST_CHECKED)
					{
						swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Basic inno setup rebuilding + WardwizBasicSetupx64.iss" );

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizBasicSetupx64.iss\"", csInnoPath, csISSPath);
						if( pWWALUpdDlg->RebuildSolution( szTemp ) )
							return 3;
					}

					if( pWWALUpdDlg->m_Rebuild32.GetCheck( ) == BST_CHECKED && pWWALUpdDlg->m_chkNoClam.GetCheck() ==  BST_CHECKED )
					{
						swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Basic PreinstallationScan inno setup rebuilding + WardwizBasicPreInstallSCNx86.iss");

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizBasicPreInstallSCNx86.iss\"", csInnoPath, csISSPath);
						if (pWWALUpdDlg->RebuildSolution(szTemp))
							return 3;	

						swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Basic inno setup rebuilding + WardwizBasicSetupNCGx86.iss");

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizBasicSetupNCGx86.iss\"", csInnoPath, csISSPath);
						if( pWWALUpdDlg->RebuildSolution( szTemp ) )
							return 3;

						swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Basic inno setup rebuilding + WardwizBasicSetupNCIx86.iss");

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizBasicSetupNCIx86.iss\"", csInnoPath, csISSPath);
						if (pWWALUpdDlg->RebuildSolution(szTemp))
							return 3;
					}
					if( pWWALUpdDlg->m_Rebuild64.GetCheck( ) == BST_CHECKED && pWWALUpdDlg->m_chkNoClam.GetCheck() ==  BST_CHECKED )
					{
						swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Basic PreinstallationScan inno setup rebuilding + WardwizBasicPreInstallSCNx64.iss");

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizBasicPreInstallSCNx64.iss\"", csInnoPath, csISSPath);
						if (pWWALUpdDlg->RebuildSolution(szTemp))
							return 3;
						
						swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Basic inno setup rebuilding + WardwizBasicSetupNCGx64.iss");

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizBasicSetupNCGx64.iss\"", csInnoPath, csISSPath);
						if( pWWALUpdDlg->RebuildSolution( szTemp ) )
							return 3;

						swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Basic inno setup rebuilding + WardwizBasicSetupNCIx64.iss");

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizBasicSetupNCIx64.iss\"", csInnoPath, csISSPath);
						if (pWWALUpdDlg->RebuildSolution(szTemp))
							return 3;
					}

				if( pWWALUpdDlg->m_Rebuild32.GetCheck( ) == BST_CHECKED && pWWALUpdDlg->m_chkOfflinePatches.GetCheck() ==  BST_CHECKED )
					{
						swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Basic inno setup rebuilding + WardWizBasicPatchesxG86.iss" );

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardWizBasicPatchesxG86.iss\"", csInnoPath, csISSPath);
						if( pWWALUpdDlg->RebuildSolution( szTemp ) )
							return 3;

						swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Basic inno setup rebuilding + WardWizBasicPatchesxI86.iss");

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardWizBasicPatchesxI86.iss\"", csInnoPath, csISSPath);
						if (pWWALUpdDlg->RebuildSolution(szTemp))
							return 3;
					}

			  if( pWWALUpdDlg->m_Rebuild64.GetCheck( ) == BST_CHECKED && pWWALUpdDlg->m_chkOfflinePatches.GetCheck() ==  BST_CHECKED )
					{
						swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Basic inno setup rebuilding + WardWizBasicPatchesGx64.iss" );

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardWizBasicPatchesGx64.iss\"", csInnoPath, csISSPath);
						if( pWWALUpdDlg->RebuildSolution( szTemp ) )
							return 3;

						swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Basic inno setup rebuilding + WardWizBasicPatchesIx64.iss");

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardWizBasicPatchesIx64.iss\"", csInnoPath, csISSPath);
						if (pWWALUpdDlg->RebuildSolution(szTemp))
							return 3;
					}
				}

			if(bEssentialTicked)
				{
					gbIsBatchFileRunning= false;
					if( pWWALUpdDlg->m_Rebuild32.GetCheck( ) == BST_CHECKED && pWWALUpdDlg->m_chkwithClam.GetCheck() ==  BST_CHECKED )
					{
						swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"TS inno setup rebuilding + VibroTSSetupX86.iss" );
						
						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\VibroTSSetupX86.iss\"", csInnoPath, csISSPath);
						if( pWWALUpdDlg->RebuildSolution( szTemp ) )
							return 3;
					}
					if( pWWALUpdDlg->m_Rebuild64.GetCheck( ) == BST_CHECKED && pWWALUpdDlg->m_chkwithClam.GetCheck() ==  BST_CHECKED )
					{
						swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"TS inno setup rebuilding + VibroTSSetupX64.iss" );
						
						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\VibroTSSetupX64.iss\"", csInnoPath, csISSPath);
						if( pWWALUpdDlg->RebuildSolution( szTemp ) )
							return 3;
					}

					if( pWWALUpdDlg->m_Rebuild32.GetCheck( ) == BST_CHECKED && pWWALUpdDlg->m_chkNoClam.GetCheck() ==  BST_CHECKED)
					{
						swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"TS PreinstallationScan inno setup rebuilding + VibroTSPreInstallSCNx86.iss");

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\VibroTSPreInstallSCNx86.iss\"", csInnoPath, csISSPath);
						if (pWWALUpdDlg->RebuildSolution(szTemp))
							return 3; 						

						swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"TS inno setup rebuilding + WardwizEssentialSetupNCGx86.iss");
						
						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizEssentialSetupNCGx86.iss\"", csInnoPath, csISSPath);
						if( pWWALUpdDlg->RebuildSolution( szTemp ) )
							return 3;

						swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"TS inno setup rebuilding + VibroTSSetupX86.iss");

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\VibroTSSetupX86.iss\"", csInnoPath, csISSPath);
						if (pWWALUpdDlg->RebuildSolution(szTemp))
							return 3;
					}
					if( pWWALUpdDlg->m_Rebuild64.GetCheck( ) == BST_CHECKED && pWWALUpdDlg->m_chkNoClam.GetCheck() ==  BST_CHECKED)
					{
						swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"TS PreinstallationScan inno setup rebuilding + VibroTSPreInstallSCNx64.iss");

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\VibroTSPreInstallSCNx64.iss\"", csInnoPath, csISSPath);
						if (pWWALUpdDlg->RebuildSolution(szTemp))
							return 3; 						

						swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"EsseVibroASPreInstallSCNx86.issntial inno setup rebuilding + WardwizEssentialSetupNCGx64.iss");
						
						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizEssentialSetupNCGx64.iss\"", csInnoPath, csISSPath);
						if( pWWALUpdDlg->RebuildSolution( szTemp ) )
							return 3;
						swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"AS inno setup rebuilding + VibroTSSetupX64.iss");

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\VibroTSSetupX64.iss\"", csInnoPath, csISSPath);
						if (pWWALUpdDlg->RebuildSolution(szTemp))
							return 3;
					}

					if (pWWALUpdDlg->m_Rebuild32.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkOfflinePatches.GetCheck() == BST_CHECKED)
					{
						swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"TS inno setup rebuilding + WardWizEssentialPatchesGx86.iss" );
						
						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardWizEssentialPatchesGx86.iss\"", csInnoPath, csISSPath);
						if( pWWALUpdDlg->RebuildSolution( szTemp ) )
							return 3;

						swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"TS inno setup rebuilding + VibroTSPatchesX86.iss");

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\VibroTSPatchesX86.iss\"", csInnoPath, csISSPath);
						if (pWWALUpdDlg->RebuildSolution(szTemp))
							return 3;
					}

					if (pWWALUpdDlg->m_Rebuild64.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkOfflinePatches.GetCheck() == BST_CHECKED)
					{
						swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"TS inno setup rebuilding + WardWizEssentialPatchesGx64.iss" );
						
						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardWizEssentialPatchesGx64.iss\"", csInnoPath, csISSPath);
						if( pWWALUpdDlg->RebuildSolution( szTemp ) )
							return 3;

						swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"TS inno setup rebuilding + VibroTSPatchesX64.iss");

						swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\VibroTSPatchesX64.iss\"", csInnoPath, csISSPath);
						if (pWWALUpdDlg->RebuildSolution(szTemp))
							return 3;
					}

				}	

			if (bProTicked)
			{
				gbIsBatchFileRunning = false;
				if (pWWALUpdDlg->m_Rebuild32.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkwithClam.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Pro inno setup rebuilding + WardwizProSetupx86.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizProSetupx86.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}
				if (pWWALUpdDlg->m_Rebuild64.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkwithClam.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Pro inno setup rebuilding + WardwizProSetupx64.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizProSetupx64.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}

				if (pWWALUpdDlg->m_Rebuild32.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkNoClam.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Pro PreinstallationScan inno setup rebuilding + WardwizProPreInstallSCNx86 .iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizProPreInstallSCNx86 .iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3; 					

					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Pro inno setup rebuilding + WardwizProSetupNCGx86.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizProSetupNCGx86.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;

					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Pro inno setup rebuilding + WardwizProSetupNCIx86.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizProSetupNCIx86.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}
				if (pWWALUpdDlg->m_Rebuild64.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkNoClam.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Pro PreinstallationScan inno setup rebuilding + WardwizProPreInstallSCNx64.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizProPreInstallSCNx64.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3; 					

					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Pro inno setup rebuilding + WardwizProSetupNCGx64.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizProSetupNCGx64.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;

					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Pro inno setup rebuilding + WardwizProSetupNCIx64.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizProSetupNCIx64.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}

				if (pWWALUpdDlg->m_Rebuild32.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkOfflinePatches.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Pro inno setup rebuilding + WardWizProPatchesGx86.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardWizProPatchesGx86.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;

					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Pro inno setup rebuilding + WardWizProPatchesIx86.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardWizProPatchesIx86.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}

				if (pWWALUpdDlg->m_Rebuild64.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkOfflinePatches.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Pro inno setup rebuilding + WardWizProPatchesGx64.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardWizProPatchesGx64.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;

					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Pro inno setup rebuilding + WardWizProPatchesIx64.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardWizProPatchesIx64.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}

			}
            
			if (bEliteTicked)
			{
				gbIsBatchFileRunning = false;
				if (pWWALUpdDlg->m_Rebuild32.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkNoClam.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Elite inno setup rebuilding + WardwizEPSClientx86.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizEPSClientx86.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}
				if (pWWALUpdDlg->m_Rebuild64.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkwithClam.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Elite inno setup rebuilding +WardwizEPSClientx64.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizEPSClientx64.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}
				if (pWWALUpdDlg->m_Rebuild32.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkNoClam.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Elite inno setup rebuilding + WardwizEPSServerx86.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizEPSServerx86.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}
				if (pWWALUpdDlg->m_Rebuild64.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkwithClam.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Elite inno setup rebuilding + WardwizEPSServerx64.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizEPSServerx64.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}

			}

			if (bEssentailPlusTicked)
			{
				gbIsBatchFileRunning = false;
				/*if (pWWALUpdDlg->m_Rebuild32.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkwithClam.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"EssentialPlus inno setup rebuilding + WardwizEssPlusSetupx86.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizEssPlusSetupx86.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}
				if (pWWALUpdDlg->m_Rebuild64.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkwithClam.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"EssentialPlus inno setup rebuilding + WardwizEssPlusSetupx64.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizEssPlusSetupx64.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}*/

				if (pWWALUpdDlg->m_Rebuild32.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkNoClam.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"AS PreinstallationScan inno setup rebuilding + VibroASPreInstallSCNx86.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\VibroASPreInstallSCNx86.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3; 

					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"EssentialPlus inno setup rebuilding + WardwizEssPlusSetupNCGx86.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizEssPlusSetupNCGx86.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;

					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"AS inno setup rebuilding + VibroASSetupX86.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\VibroASSetupX86.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}
				if (pWWALUpdDlg->m_Rebuild64.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkNoClam.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"AS PreinstallationScan inno setup rebuilding + VibroASPreInstallSCNx64.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\VibroASPreInstallSCNx64.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3; 					

					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"EssentialPlus inno setup rebuilding + WardwizEssPlusSetupNCGx64.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizEssPlusSetupNCGx64.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;

					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"EssentialPlus inno setup rebuilding + VibroASSetupX64.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\VibroASSetupX64.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}
				if (pWWALUpdDlg->m_Rebuild32.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkOfflinePatches.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"AS inno setup rebuilding + WardwizEssPlusPatchesGx86.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizEssPlusPatchesGx86.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;

					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"AS inno setup rebuilding + VibroASPatchesX86.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\VibroASPatchesX86.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}
				if (pWWALUpdDlg->m_Rebuild64.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkOfflinePatches.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"AS inno setup rebuilding + WardwizEssPlusPatchesGx64.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizEssPlusPatchesGx64.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;

					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"AS inno setup rebuilding + VibroASPatchesX64");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\VibroASPatchesX64\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}

			}

			if (bBasicTicked || bEssentialTicked || bEssentailPlusTicked || bProTicked)
			{
				gbIsBatchFileRunning = false;
				if (pWWALUpdDlg->m_Rebuild32.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkNoClam.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Incompatibility Antivirus Check inno setup rebuilding + incompcheckx86.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\incompcheckx86.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
					
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Installer inno setup rebuilding + installerEx86.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\installerEx86.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;

					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Installer inno setup rebuilding + installerGx86.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\installerGx86.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}
				if (pWWALUpdDlg->m_Rebuild64.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_chkNoClam.GetCheck() == BST_CHECKED)
				{
					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Incompatibility Antivirus Check inno setup rebuilding + incompcheckx64.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\incompcheckx64.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;

					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Installer inno setup rebuilding + installerEx64.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\installerEx64.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;

					swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Installer inno setup rebuilding + installerGx64.iss");

					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\installerGx64.iss\"", csInnoPath, csISSPath);
					if (pWWALUpdDlg->RebuildSolution(szTemp))
						return 3;
				}
			}

			if(pWWALUpdDlg->m_chkpdb.GetCheck() ==  BST_CHECKED )
					{
						if (bBasicTicked)
						{
							swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Basic inno setup rebuilding + WardwizBasicPDB.iss");
							swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizBasicPDB.iss\"", csInnoPath, csISSPath);
							if (pWWALUpdDlg->RebuildSolution(szTemp))
								return 3;
						}
						
						if (bEssentialTicked)
						{
							swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"TS inno setup rebuilding + VibroTS_PDB.iss");
							swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\VibroTS_PDB.iss\"", csInnoPath, csISSPath);
							if (pWWALUpdDlg->RebuildSolution(szTemp))
								return 3;

						}
						
						if (bProTicked)
						{
							swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Pro inno setup rebuilding + WardwizProPDB.iss");
							swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizProPDB.iss\"", csInnoPath, csISSPath);
							if (pWWALUpdDlg->RebuildSolution(szTemp))
								return 3;
						}

					}
				pWWALUpdDlg->mbIsFileSelected= true;
				pWWALUpdDlg->AddSignatureToFinalSetup();            // to add signature to setup file.
				pWWALUpdDlg->mbIsFileSelected= false;
			
				//AfxMessageBox( L"Setup Created sucessfully" );
			//}	
		}
	//This code is added to read registry option entries added by user, And write it to Patch ini files.
	//Name : Niranjan Deshak - 9/2/2015.
		if(pWWALUpdDlg->m_chkRegOpt.GetCheck() == BST_CHECKED)
		{
			TCHAR	szDataToSendtoIni[512] = {0};
			TCHAR	szDataWrdTray[512] = {0};
			TCHAR szListItemData[512] = {0};
			TCHAR szListItemString[512] = {0};
			pWWALUpdDlg->m_listRegOpt.RemoveAll();

			for(int index = 0; index < pWWALUpdDlg->m_List_RegOpt.GetItemCount(); index++)
			{	
				ZeroMemory(szListItemString, sizeof(szListItemString) );
				
				for(int index1 = 0; index1 < 4; index1++)
				{
					int ilen = pWWALUpdDlg->m_List_RegOpt.GetItemText(index, index1 ,szListItemData,_countof(szListItemData));
					swprintf_s(szListItemString, _countof(szListItemString), L"%s,%s", szListItemString,szListItemData);
			
				}
				pWWALUpdDlg->m_listRegOpt.AddTail((CString)szListItemString);
			}
			CString csSendToIni = L"";
			POSITION pos = pWWALUpdDlg->m_listRegOpt.GetHeadPosition();
			for(int index = 0; index < pWWALUpdDlg->m_listRegOpt.GetCount(); index++)
			{
			ZeroMemory(szDataToSendtoIni, sizeof(szDataToSendtoIni) );
			csSendToIni  = pWWALUpdDlg->m_listRegOpt.GetNext(pos);
			csSendToIni.Delete(0,1);
			swprintf_s(szDataToSendtoIni, _countof(szDataToSendtoIni), L"%s", csSendToIni);
			pWWALUpdDlg->WriteToIni(L"RegOpt", L"Count", szDataToSendtoIni, NULL);
			}
		}

		if (pWWALUpdDlg->m_chkPatchType.GetCheck() == BST_CHECKED && pWWALUpdDlg->m_listOfWWBinaryKeys.size()>0)
		{
			
			
				for (int index = 0; index < pWWALUpdDlg->m_listOfWWBinaryKeys.size(); index++)
				{
					TCHAR szKey[MAX_PATH] = {0};
					
					swprintf_s(szKey, _countof(szKey), L"%s", pWWALUpdDlg->m_listOfWWBinaryKeys.at(index));

					if (pWWALUpdDlg->GetKeyValueFromIni(szKey) != 0x00)
					{
						AddLogEntry(L">>> Error in function GetKeyValueFromIni");
					}

					/*if (pWWALUpdDlg->GetKeyValueFromIni(L"AddedFeature") != 0x00)
					{
					AddLogEntry(L">>> Error in function GetKeyValueFromIni for AddedFeature");
					}

					if (pWWALUpdDlg->GetKeyValueFromIni(L"EnhancedFunctionality") != 0x00)
					{
					AddLogEntry(L">>> Error in function GetKeyValueFromIni for EnhancedFunctionality");
					}*/
				}

		}
		//completed.
		if(!bChkAll)
		{
			AfxMessageBox( L"Please select any option" );
			goto Cleanup;
		}
		if(pWWALUpdDlg->m_chkwithClam.GetCheck() ==  BST_CHECKED ||pWWALUpdDlg->m_chkNoClam.GetCheck() ==  BST_CHECKED ||pWWALUpdDlg->m_chkpdb.GetCheck() ==  BST_CHECKED || pWWALUpdDlg->m_chkPatchType.GetCheck() ==  BST_CHECKED  )
		{
			if(!pWWALUpdDlg->UploadFilesToLocalServer())
				{
					AddLogEntry(L"### CVibraniumALUpdDlg:: UploadFilesToLocalServer(): Failed to upload file on Local Server");
					AfxMessageBox( L"Failed to upload files" );
				}
		}
		else
		{
			pWWALUpdDlg->m_bIsSetupCreatedAndFilesUploaded = true;
			//AfxMessageBox( L"Files uploaded sucessfully" );
			swprintf_s(	m_szStatusText, _countof(m_szStatusText), L"Files uploaded sucessfully" );
		}

	}
	catch( ... )
	{
		AddLogEntry(L"### Exception in Thread_GenerateUpdateFiles()");
	}

Cleanup:

	pWWALUpdDlg->m_edtDatabaseVersion.EnableWindow(TRUE);
	pWWALUpdDlg->m_edtProdVersion.EnableWindow(TRUE);
	
	if( bBasicTicked )
		pWWALUpdDlg->m_ProdType_Basic.SetCheck(BST_CHECKED);

	if( bEssentialTicked)
		pWWALUpdDlg->m_ProdType_Essential.SetCheck(BST_CHECKED);

	if( bProTicked )
		pWWALUpdDlg->m_ProdType_Pro.SetCheck(BST_CHECKED);

	if( bEliteTicked )
		pWWALUpdDlg->m_ProdType_Elite.SetCheck(BST_CHECKED);

	if (bEssentailPlusTicked)
		pWWALUpdDlg->m_ProdType_EssPlus.SetCheck(BST_CHECKED);

	pWWALUpdDlg->m_Generate_Update_Files.EnableWindow( );

	pWWALUpdDlg->m_Close.EnableWindow( );

	if( dwRet )
		{
		pWWALUpdDlg->UploadLogFilesToServer(L"error",true);
		//AfxMessageBox( L"Patch Creation failed" );
		//return;
		}

	if( pWWALUpdDlg->m_hThread_SetStatus )
	{
		SuspendThread( pWWALUpdDlg->m_hThread_SetStatus ) ;
		TerminateThread( pWWALUpdDlg->m_hThread_SetStatus, 0 );
		CloseHandle( pWWALUpdDlg->m_hThread_SetStatus );
		pWWALUpdDlg->m_hThread_SetStatus = NULL ;
	}

	CloseHandle( pWWALUpdDlg->m_hThread_GenerateUpdateFiles );
	pWWALUpdDlg->m_hThread_GenerateUpdateFiles = NULL;

	return 0 ;
}

/***************************************************************************
  Function Name  : RebuildAllVCProject
  Description    : This function is called to build/rebuild project as per user selection
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0019
****************************************************************************/
DWORD CWardWizALUpdDlg::RebuildAllVCProject(CString csConfigurationType, bool bProTicked)
{
/*
	C:\Program Files\Microsoft Visual Studio 9.0\Common7\IDE>devenv.exe D:\WardWiz_D
evelopement\WWizHash\WWizHash.sln /BUILD "Release|X64" /Out d:\WardWiz_Developem
ent\rebuild.log
*/

	TCHAR	szTemp[1024] = {0};

	LPTSTR pszSolu = L"D:\\Vilas\\WW_Dev\\WWizHash\\WWizHash.sln" ;
	LPTSTR pszlogPath = L"D:\\Vilas\\WW_Dev\\WWizHash\\WWizHash_Rebuild32.log" ;

	TCHAR	szLogPath[512] = {0};
	TCHAR	szSlnPath[512] = {0};

	swprintf_s(	m_szStatusText, _countof(m_szStatusText), L"Building 64 bit solution(Wardwiz_VS2013.sln)" );
	AddLogEntry(L">>>CVibraniumALUpdDlg :: RebuildAllVCProject : Building 64 bit solution(Vibranium_VS2013.sln)");
	swprintf_s(	szSlnPath, _countof(szSlnPath), L"%s\\Wardwiz_VS2013.sln", m_szApplPath );
	swprintf_s(	szLogPath, _countof(szSlnPath), L"%s\\WardwizAntivirus_Rebuild64.log", m_szApplPath );
	
	// batch file must contain cmd.exe /c batch.bat
	gbIsBatchFileRunning= true ;
	//swprintf_s(	szTemp, _countof(szTemp), L"\" /c  %s\\SetupRequirements\\DigitalSignature\\ExecCommand.bat\" ", m_szApplPath);
	
	if( m_Rebuild64.GetCheck() == BST_CHECKED )
	{
		swprintf_s(	szSlnPath, _countof(szSlnPath), L"%s\\Wardwiz_VS2013.sln", m_szApplPath );
		swprintf_s(	szLogPath, _countof(szSlnPath), L"%s\\WardwizAntivirus_Rebuild64.log", m_szApplPath );
		if(m_chkBuildType.GetCheck() == BST_CHECKED)
		{
			bool isbBuild64= false;  
			if( m_btnBuild.GetCheck() == BST_CHECKED )
			{
					//if( m_bIsWow64 )
					//{
						gbIsBatchFileRunning= false ;
						isbBuild64= true;
						swprintf_s(	m_szStatusText, _countof(m_szStatusText), L"Building 64 bit solution(Wardwiz_VS2013.sln)" );
						AddLogEntry(L">>>CVibraniumALUpdDlg :: RebuildAllVCProject : Building 64 bit solution(Vibranium_VS2013.sln)");
						swprintf_s(	szTemp, _countof(szTemp), 
							L"\"%s\\Microsoft Visual Studio 12.0\\Common7\\IDE\\devenv.exe\" \"%s\" /Build \"%s|x64\" /Out \"%s\"",
							m_szProgDirX64, szSlnPath, csConfigurationType,szLogPath);
					//}			
			}
			else if(m_btnReBuild.GetCheck() == BST_CHECKED)
				{
				//	if( m_bIsWow64 )
				//	{
						gbIsBatchFileRunning= false ;
						isbBuild64= true;
						swprintf_s(	m_szStatusText, _countof(m_szStatusText), L"Rebuilding 64 bit solution(Wardwiz_VS2013.sln)" );
						AddLogEntry(L">>>CVibraniumALUpdDlg :: RebuildAllVCProject : ReBuilding 64 bit solution(Vibranium_VS2013.sln)");

						swprintf_s(	szTemp, _countof(szTemp), 
							L"\"%s\\Microsoft Visual Studio 12.0\\Common7\\IDE\\devenv.exe\" \"%s\" /Rebuild \"%s|x64\" /Out \"%s\"",
							m_szProgDirX64, szSlnPath, csConfigurationType, szLogPath);
				//	}
				}

				if(isbBuild64)
				{

						if( !PathFileExists( szSlnPath ) )
						{
							AddLogEntry(L"### Failed for RebuildAllVCProject 64 bit::File not found(%s)", szSlnPath);
							return 1;
						}
						
						SetFileAttributes(szLogPath, FILE_ATTRIBUTE_NORMAL);
						DeleteFile( szLogPath );

						if( RebuildSolution( szTemp ) )
							return 1;
						swprintf_s(	m_szStatusText, _countof(m_szStatusText), L"Parsing 64 bit log(WardwizAntivirus_Rebuild64.log)" );
						if( ParserebuildLog( szLogPath, "Release x64" ) )
							return 2;
				}
		}		
	}

	if( m_Rebuild32.GetCheck() == BST_CHECKED )
	{		gbIsBatchFileRunning= false;
		swprintf_s(	m_szStatusText, _countof(m_szStatusText), L"Rebuilding 32 bit solution(Wardwiz_VS2013.sln)" );
		AddLogEntry(L">>>CVibraniumALUpdDlg :: RebuildAllVCProject : ReBuilding 32 bit solution(Vibranium_VS2013.sln)");

		swprintf_s(	szLogPath, _countof(szSlnPath), L"%s\\WardwizAntivirus_Rebuild32.log", m_szApplPath );
		if(m_chkBuildType.GetCheck() == BST_CHECKED)
		{
			if( m_btnBuild.GetCheck() == BST_CHECKED )
			{
				swprintf_s(	m_szStatusText, _countof(m_szStatusText), L"Building 32 bit solution(Wardwiz_VS2013.sln)" );
				swprintf_s(	szTemp, _countof(szTemp), 
					L"\"%s\\Microsoft Visual Studio 12.0\\Common7\\IDE\\devenv.exe\" \"%s\" /Build \"%s|Win32\" /Out \"%s\"",
					m_szProgDirX64, szSlnPath, csConfigurationType, szLogPath);

			}
			else if(m_btnReBuild.GetCheck() == BST_CHECKED )
			{
				swprintf_s(	m_szStatusText, _countof(m_szStatusText), L"Rebuilding 32 bit solution(Wardwiz_VS2013.sln)" );
				swprintf_s(	szTemp, _countof(szTemp), 
					L"\"%s\\Microsoft Visual Studio 12.0\\Common7\\IDE\\devenv.exe\" \"%s\" /Rebuild \"%s|Win32\" /Out \"%s\"",
					m_szProgDirX64, szSlnPath, csConfigurationType, szLogPath);
			}
			if( !PathFileExists( szSlnPath ) )
			{
				AddLogEntry(L"### Failed for RebuildAllVCProject 32 bit::File not found(%s)", szSlnPath);
				return 1;
			}

			SetFileAttributes(szLogPath, FILE_ATTRIBUTE_NORMAL);
			DeleteFile( szLogPath );
			if( RebuildSolution( szTemp ) )
				return 3;

			swprintf_s(	m_szStatusText, _countof(m_szStatusText), L"Parsing 32 bit log(WardwizAntivirus_Rebuild32.log)" );
			if( ParserebuildLog( szLogPath, "Release Win32" ) )
				return 4;
		}
		
	}
	////"D:\WardWiz_Developement\SetupRequirements\DigitalSignature\ExecCommand.bat"	
	//	swprintf_s(	szTemp, _countof(szTemp), 
	//			L"\"/c  %s\\SetupRequirements\\DigitalSignature\\ExecCommand.bat\" ",
	//			m_szApplPath);
	//	swprintf_s(	m_szStatusText, _countof(m_szStatusText), L"adding Digital signature to Product paches file: ExecCommand.bat" );
	//		
	//	gbIsBatchFileRunning = true;
	//	if( RebuildSolution( szTemp ) )
	//			return 3;

	return 0;
}

/***************************************************************************
  Function Name  : RebuildSolution
  Description    : This function creates a process for building/rebuilding the soluction as per user selection
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0020
****************************************************************************/
BOOL CWardWizALUpdDlg::RebuildSolution( LPTSTR pszCmdLine )
{

	STARTUPINFO			si = {0} ;
	PROCESS_INFORMATION	pi = {0} ;

	try
	{

		si.cb = sizeof(STARTUPINFO);

		 si.wShowWindow = SW_HIDE;
		 si.dwFlags = STARTF_USESHOWWINDOW;
		 
		 if(gbIsBatchFileRunning == true )
		 {
				  gbIsBatchFileRunning= false;
				  TCHAR systemDirPath[MAX_PATH] = _T("");
				  GetSystemDirectory( systemDirPath, sizeof(systemDirPath)/sizeof(_TCHAR) );
			
				  TCHAR commandLine[2 * MAX_PATH + 16] = _T("");
				
		  		  swprintf_s(commandLine, _countof(commandLine),L"\"%s\\cmd.exe\" %s ",systemDirPath,pszCmdLine);
						
		
			if( !CreateProcess(NULL ,commandLine, NULL, NULL, TRUE,	CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi) )
				{
					AddLogEntry(L"### Failed for CVibraniumALUpdDlg::RebuildSolution::%s", pszCmdLine);
					return TRUE;
				}
		 }
		 else
		 {

		if( !CreateProcess(	NULL, pszCmdLine, NULL, NULL, TRUE,	CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi) )
				{
					AddLogEntry(L"### Failed for CVibraniumALUpdDlg::RebuildSolution::%s", pszCmdLine);
					return TRUE;
				}
		 }
		

 //if( !CreateProcess(NULL, pszCmdLine, NULL, NULL, TRUE,	CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi) )
	//	{
	//		AddLogEntry(L"### Failed for CVibraniumALUpdDlg::RebuildSolution::%s", pszCmdLine);
	//		return TRUE;
	//	}
		
		WaitForSingleObject(pi.hProcess, INFINITE ) ;
		CloseHandle( pi.hProcess ) ;
		pi.hProcess = NULL ;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::RebuildSolution");
		return TRUE;
	}

	return FALSE;
}

/***************************************************************************
  Function Name  : ParserebuildLog
  Description    : This function is called after build/rebuild of project and it will check for errors in project
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0021
****************************************************************************/
bool CWardWizALUpdDlg::ParserebuildLog( LPTSTR pszLogPath, char *pszSearchKey )
{

	if( !PathFileExists(pszLogPath) )
	{
		AddLogEntry(L"### Failed in ParserebuildLog::PathFileExists");
		return true;
	}

	HANDLE hFile = CreateFile(pszLogPath, GENERIC_READ, FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if( hFile == INVALID_HANDLE_VALUE )
	{
		AddLogEntry(L"### Failed in ParserebuildLog::Open, %s",pszLogPath);
		return true;
	}

	DWORD ulFileSize = GetFileSize(hFile, NULL) ;
	if( !ulFileSize )
	{
		CloseHandle( hFile );
		AddLogEntry(L"### Failed in ParserebuildLog::GetLength, %s", pszLogPath);
		return true;
	}

	DWORD	dwReadBytes = 0x00;
	char	*strFileData = new char[ulFileSize+1] ;

	if( !strFileData )
	{
		CloseHandle( hFile );
		AddLogEntry(L"### Failed in ParserebuildLog::new operator, %s", pszLogPath);
		return true;
	}

	ZeroMemory(strFileData, ulFileSize+1);
	ReadFile(hFile, strFileData, ulFileSize, &dwReadBytes, NULL );
	CloseHandle( hFile );
	if( ulFileSize != dwReadBytes )
	{
		delete [] strFileData;
		AddLogEntry(L"### Failed in ParserebuildLog::Read, %s", pszLogPath);
		return true;
	}

	if( !strstr(strFileData, pszSearchKey)  )
	{
		delete [] strFileData;
		AddLogEntry(L"### Failed in ParserebuildLog::Find, %s", pszLogPath);
		return true;
	}

	/*char *pRebuildAll = strstr(strFileData, "Rebuild All: ");
	if( !pRebuildAll )
	{
		delete [] strFileData;
		AddLogEntry(L"### Failed in ParserebuildLog::Find, %s", pszLogPath);
		return true;
	}*/

//	pRebuildAll += strlen("Rebuild All: ") + 0x09;
	if( !strstr(strFileData, ", 0 failed,") )
	{
		delete [] strFileData;
		AddLogEntry(L"### Failed in ParserebuildLog::Find, %s", pszLogPath);
		return true;
	}

	delete [] strFileData;
	return false ;
}

/***************************************************************************
  Function Name  : Thread_SetStatus
  Description    : This function is called to set the status on UI.
				   Status will be displayed as per the currently running task
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0022
****************************************************************************/
DWORD WINAPI Thread_SetStatus( LPVOID lpParam )
{
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *) lpParam;

	if( !pWWALUpdDlg )
		return 1;
	swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Process started" );
	TCHAR	szTemp[512] = {0};

	while( 1 )
	{

		if( pWWALUpdDlg->m_szStatusText[0] )
		{
			ZeroMemory(szTemp, sizeof(szTemp) );
			swprintf( szTemp, L"%s", pWWALUpdDlg->m_szStatusText );
			pWWALUpdDlg->m_Status.SetWindowTextW( szTemp ) ;
			Sleep( 100 );

			ZeroMemory(szTemp, sizeof(szTemp) );
			swprintf( szTemp, L"%s..", pWWALUpdDlg->m_szStatusText );
			pWWALUpdDlg->m_Status.SetWindowTextW( szTemp ) ;
			Sleep( 100 );

			ZeroMemory(szTemp, sizeof(szTemp) );
			swprintf( szTemp, L"%s....", pWWALUpdDlg->m_szStatusText );
			pWWALUpdDlg->m_Status.SetWindowTextW( szTemp ) ;
			Sleep( 100 );
		}
		else
			Sleep( 100 );
	}

	return 0 ;
}

/***************************************************************************
  Function Name  : OnBnClickedButtonClose
  Description    : This function is called if user wants to close the UI
				   If any process is in execution then message box is displayed
  Author Name    : Vilas 
  Date           : 15 July 2014
  SR_NO			 : SR.N0 ALUPD_0023
****************************************************************************/
void CWardWizALUpdDlg::OnBnClickedButtonClose()
{
	try
	{
		
		DWORD dwIdReturn = AfxMessageBox(L"Do you want to close", MB_YESNO);
		if (dwIdReturn == IDNO)
		{
			return;
		}
		else if (dwIdReturn == IDYES)
		{
			//Updating Product version by 1 from WWBinary.ini file to create setup next time .
			//Niranjan Deshak - 16/03/2015.
			if (m_bIsSetupCreatedAndFilesUploaded == true)
			{
				/*CString csProdVersion = L"";
				TCHAR	m_szProductVersion[256] = {0};

				int iProductVersion[4] = { 0 };
				csProdVersion = m_szProdVersion;

				bool bRet = ParseVersionString(iProductVersion, csProdVersion);
				iProductVersion[3]++;

				swprintf_s(m_szProductVersion, _countof(m_szProductVersion), L"%d.%d.%d.%d", iProductVersion[0], iProductVersion[1], iProductVersion[2], iProductVersion[3]);
				*/

				DWORD dwRetValue = WritePrivateProfileString(L"ProductVersion", L"ProductVer", m_szProductVersion, m_szWWBinariesPath);
				if (!dwRetValue)
				{
					AddLogEntry(L"### Failed in CVibraniumALUpdDlg::OnBnClickedButtonClose :  %s", m_szProductVersion);
				}
				
			}
			else if (m_bIsSetupCreatedAndFilesUploaded == false)
			{

				DWORD dwRetValue = WritePrivateProfileString(L"ProductVersion", L"ProductVer", m_szCurrProdVersion, m_szWWBinariesPath);
				if (!dwRetValue)
				{
					AddLogEntry(L"### Failed in CVibraniumALUpdDlg::OnBnClickedButtonClose :  %s", m_szProductVersion);
				}

			}
		}

		if (m_hThread_GenerateUpdateFiles)
		{
			if (m_hThread_SetStatus)
			{
				SuspendThread(m_hThread_SetStatus);
				TerminateThread(m_hThread_SetStatus, 0);
				CloseHandle(m_hThread_SetStatus);
				m_hThread_SetStatus = NULL;
			}

			SuspendThread(m_hThread_GenerateUpdateFiles);
			TerminateThread(m_hThread_GenerateUpdateFiles, 0);
			CloseHandle(m_hThread_GenerateUpdateFiles);

			m_hThread_GenerateUpdateFiles = NULL;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::OnBnClickedButtonClose", 0, 0, true, SECONDLEVEL);
	}

	exit( 0 );
}

/***************************************************************************
  Function Name  : LoadBinariesNameAsPerSettings
  Description    : This function is called to Load Binary File "WWBinary.ini"
				   (ini file contains the list of files that need to be added in patch INI)
  Author Name    : Nitin 
  Date           : 18 August 2014
  SR_NO			 : SR.N0 ALUPD_0024
****************************************************************************/
bool CWardWizALUpdDlg :: LoadBinariesNameAsPerSettings(LPTSTR lpIniFilePath)
{
	CString sz_Count = L"";
	ZeroMemory(m_szBasicCount, sizeof(m_szBasicCount));
	ZeroMemory(m_szEssentialCount, sizeof(m_szEssentialCount));
	ZeroMemory(m_szProCount, sizeof(m_szProCount));
	ZeroMemory(m_szEliteCount, sizeof(m_szEliteCount)); 
	ZeroMemory(m_szEliteServerCount, sizeof(m_szEliteServerCount));
	ZeroMemory(m_szEssentialPlusCount, sizeof(m_szEssentialPlusCount));
	ZeroMemory(m_szProdVersion, sizeof(m_szProdVersion));
	ZeroMemory(m_szDatabaseVersion, sizeof(m_szDatabaseVersion));
	ZeroMemory(m_szScanEngineVersion, sizeof(m_szScanEngineVersion));
	ZeroMemory(m_szRegOptCount, sizeof(m_szRegOptCount));
	ZeroMemory(m_szEncVersion, sizeof(m_szEncVersion));
	ZeroMemory(m_szSHA1Count, sizeof(m_szSHA1Count));
//	ZeroMemory(m_szSHA2Count, sizeof(m_szSHA2Count));

	GetPrivateProfileString(L"WRDWIZBASIC", L"Count", L"", m_szBasicCount, 255, lpIniFilePath);
	GetPrivateProfileString(L"WRDWIZESSENTIAL", L"Count", L"", m_szEssentialCount, 255, lpIniFilePath);
	GetPrivateProfileString(L"WRDWIZPRO", L"Count", L"", m_szProCount, 255, lpIniFilePath);
	GetPrivateProfileString(L"WRDWIZELITE", L"Count", L"", m_szEliteCount, 255, lpIniFilePath);
	GetPrivateProfileString(L"WRDWIZESSENTIALPLUS", L"Count", L"", m_szEssentialPlusCount, 255, lpIniFilePath);
	GetPrivateProfileString(L"ProductVersion", L"ProductVer", L"", m_szProdVersion, 255, lpIniFilePath);
	GetPrivateProfileString(L"DatabaseVersion", L"DatabaseVer", L"", m_szDatabaseVersion, 255, lpIniFilePath);
	GetPrivateProfileString(L"ScanEngineVersion", L"ScanEngineVer", L"", m_szScanEngineVersion, 255, lpIniFilePath);
	GetPrivateProfileString(L"REGOPT", L"Count", L"", m_szRegOptCount, 255, lpIniFilePath);
	GetPrivateProfileString(L"DataEncVersion", L"DataEncVer", L"", m_szEncVersion, 255, lpIniFilePath);
	GetPrivateProfileString(L"SHA1", L"Count", L"", m_szSHA1Count, 255, lpIniFilePath);
//	GetPrivateProfileString(L"SHA2", L"Count", L"", m_szSHA2Count, 255, lpIniFilePath);
	
	m_edtProdVersion.SetWindowTextW(m_szProdVersion);
	m_edtDatabaseVersion.SetWindowTextW(m_szDatabaseVersion);
	m_edtScanEngineVersion.SetWindowTextW(m_szScanEngineVersion);

	swprintf(m_szCurrProdVersion, m_szProdVersion);
	
	m_iBasicCount = 0;
	m_iBasicCount = _ttoi(m_szBasicCount);
	m_iEssentialCount = 0;
	m_iEssentialCount = _ttoi(m_szEssentialCount);
	m_iProCount = 0;
	m_iProCount = _ttoi(m_szProCount);
	m_iEliteCount = 0;
	m_iEliteCount = _ttoi(m_szEliteCount);
	m_iEliteServerCount = 0;
	m_iEliteServerCount = _ttoi(m_szEliteServerCount);
	m_iEssentialPlusCount = 0;
	m_iEssentialPlusCount = _ttoi(m_szEssentialPlusCount);
	m_iRegOptCount = 0;
	m_iRegOptCount = _ttoi(m_szRegOptCount);
	m_iSHA1Count = 0;
	m_iSHA1Count = _ttoi(m_szSHA1Count);
//	m_iSHA2Count = 0;
//	m_iSHA2Count = _ttoi(m_szSHA2Count);

	m_listBasic.RemoveAll();
	for(int i = 1; i <= m_iBasicCount; i++)
	{
		sz_Count = L"";
		sz_Count.Format(L"%d",i);
		GetPrivateProfileString(L"WRDWIZBASIC", sz_Count, L"", m_szBasicCount, 255, lpIniFilePath);
//		m_arBasicArr.Add(m_szBasicCount);
		m_listBasic.AddTail((CString)m_szBasicCount);
	}
	int j=0;
	POSITION pos = m_listBasic.GetHeadPosition();	
	if(m_listBasic.Find(CString(_T("SigGenerator.exe"))))
	{
		j++;
	}

	m_listEssential.RemoveAll();
	for(int i = 1; i <= m_iEssentialCount; i++)
	{
		sz_Count = L"";
		sz_Count.Format(L"%d",i);
		GetPrivateProfileString(L"WRDWIZESSENTIAL", sz_Count, L"", m_szEssentialCount, 255, lpIniFilePath);
		m_listEssential.AddTail((CString)m_szEssentialCount);
	}
	m_listPro.RemoveAll();
	for(int i = 1; i <= m_iProCount; i++)
	{
		sz_Count = L"";
		sz_Count.Format(L"%d",i);
		GetPrivateProfileString(L"WRDWIZPRO", sz_Count, L"", m_szProCount, 255, lpIniFilePath);
		m_listPro.AddTail((CString)m_szProCount);
	}
	m_listElite.RemoveAll();
	for(int i = 1; i <= m_iEliteCount; i++)
	{
		sz_Count = L"";
		sz_Count.Format(L"%d",i);
		GetPrivateProfileString(L"WRDWIZELITE", sz_Count, L"", m_szEliteCount, 255, lpIniFilePath);
		GetPrivateProfileString(L"WRDWIZELITE", sz_Count, L"", m_szEliteServerCount, 255, lpIniFilePath);
		m_listElite.AddTail((CString)m_szEliteCount);
		m_listElite.AddTail((CString)m_szEliteServerCount);
	}
	m_listEssentialPlus.RemoveAll();
	for (int i = 1; i <= m_iEssentialPlusCount; i++)
	{
		sz_Count = L"";
		sz_Count.Format(L"%d", i);
		GetPrivateProfileString(L"WRDWIZESSENTIALPLUS", sz_Count, L"", m_szEssentialPlusCount, 255, lpIniFilePath);
		m_listEssentialPlus.AddTail((CString)m_szEssentialPlusCount);
	}
	m_listRegOpt.RemoveAll();
	for(int i = 1; i <= m_iRegOptCount; i++)
	{
		sz_Count = L"";
		sz_Count.Format(L"%d",i);
		GetPrivateProfileString(L"REGOPT", sz_Count, L"", m_szRegOptCount, 255, lpIniFilePath);
		m_listRegOpt.AddTail((CString)m_szRegOptCount);
	}

	m_listSHA1SignBinaries.clear();
	for (int i = 1; i <= m_iSHA1Count; i++)
	{
		sz_Count = L"";
		sz_Count.Format(L"%d", i);
		GetPrivateProfileString(L"SHA1", sz_Count, L"", m_szSHA1Count, 255, lpIniFilePath);
		m_listSHA1SignBinaries.push_back((CString)m_szSHA1Count);
	}

//	m_listSHA2SignBinaries.clear();
//	for (int i = 1; i <= m_iSHA2Count; i++)
//	{
//		sz_Count = L"";
//		sz_Count.Format(L"%d", i);
//		GetPrivateProfileString(L"SHA2", sz_Count, L"", m_szSHA2Count, 255, lpIniFilePath);
//		m_listSHA2SignBinaries.push_back((CString)m_szSHA2Count);
//	}

		m_listOfWWBinaryKeys.push_back(L"DllRegister");

	m_listOfWWBinaryKeys.push_back(L"AddedFeatureBas");
	m_listOfWWBinaryKeys.push_back(L"EnhancedFunctionalityBas");
	
	m_listOfWWBinaryKeys.push_back(L"AddedFeatureEss");
	m_listOfWWBinaryKeys.push_back(L"EnhancedFunctionalityEss");
	
	m_listOfWWBinaryKeys.push_back(L"AddedFeaturePro");
	m_listOfWWBinaryKeys.push_back(L"EnhancedFunctionalityPro");

	m_listOfWWBinaryKeys.push_back(L"AddedFeatureElite");
	m_listOfWWBinaryKeys.push_back(L"EnhancedFunctionalityElite");

	m_listOfWWBinaryKeys.push_back(L"AddedFeatureEssPlus");
	m_listOfWWBinaryKeys.push_back(L"EnhancedFunctionalityEssPlus");

	return true;
}

/***************************************************************************
  Function Name  : OnBnClickedCheckBuildType
  Description    : This function is called onClick of "Build Type" option.
				   This this Select/Hide the "Build/Rebuild" option
  Author Name    : Nitin 
  Date           : 18 August 2014
  SR_NO			 : SR.N0 ALUPD_0025
****************************************************************************/
void CWardWizALUpdDlg::OnBnClickedCheckBuildType()
{
	if(m_chkBuildType.GetCheck() == BST_CHECKED)
	{
		m_btnBuild.EnableWindow(TRUE);
		m_btnReBuild.EnableWindow(TRUE);
	}
	if(m_chkBuildType.GetCheck() == BST_UNCHECKED)
	{
		m_btnBuild.EnableWindow(FALSE);
		m_btnReBuild.EnableWindow(FALSE);
	}
}

/***************************************************************************
  Function Name  : OnBnClickedCheckCheckPatchType
  Description    : This function is called onClick of "Patch Type" option.
				   This this Select/Hide the "Product/Virus Patch" option
  Author Name    : Nitin 
  Date           : 18 August 2014
  SR_NO			 : SR.N0 ALUPD_0026
****************************************************************************/
void CWardWizALUpdDlg::OnBnClickedCheckCheckPatchType()
{
	if(m_chkPatchType.GetCheck() == BST_CHECKED)
	{
		m_PatchType_Product.EnableWindow(TRUE);
		m_PatchType_Virus.EnableWindow(TRUE);
	}
	if(m_chkPatchType.GetCheck() == BST_UNCHECKED)
	{
		m_PatchType_Product.EnableWindow(FALSE);
		m_PatchType_Virus.EnableWindow(FALSE);
	}
}

/***************************************************************************
  Function Name  : OnBnClickedCheckCheckPatchType
  Description    : This function is called Write the latest Product Version & Database Version
				   to "WWBinary.ini" file. Version No will further used in .iss file for setup creation
  Author Name    : Nitin 
  Date           : 18 August 2014
  SR_NO			 : SR.N0 ALUPD_0027
****************************************************************************/
void CWardWizALUpdDlg ::WriteVersions(LPVOID lpParam)
{
	DWORD	dwRet = 0x00;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *) lpParam;
	TCHAR szModulePath[MAX_PATH] = { 0 };
	TCHAR szIniPath[MAX_PATH] = { 0 };

	if (!GetModulePath(szModulePath, MAX_PATH))
	{
		AddLogEntry(L">>> Error in GetModulePath in ParseIntegrityInfoINIAndRollBackCRC", 0, 0, true, SECONDLEVEL);
	}

	swprintf_s(szIniPath, _countof(szIniPath), L"%s\\WWBinary.ini", szModulePath);

	ZeroMemory(m_szProdVersion, sizeof(m_szProdVersion));
	ZeroMemory(m_szDatabaseVersion, sizeof(m_szDatabaseVersion));

	swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Writing Versions into INI" );

	pWWALUpdDlg->m_edtProdVersion.GetWindowTextW(m_szProdVersion, 511);
	WritePrivateProfileString(L"ProductVersion", L"ProductVer", m_szProdVersion, szIniPath);
	pWWALUpdDlg->WriteToIni(L"ProductVersion", L"ProductVer", m_szProdVersion,NULL);
	pWWALUpdDlg->WriteToEPSServerIni(L"ProductVersion", L"ProductVer", m_szProdVersion, NULL);

	
	pWWALUpdDlg->m_edtDatabaseVersion.GetWindowTextW(m_szDatabaseVersion, 511);
	WritePrivateProfileString(L"DatabaseVersion", L"DatabaseVer", m_szDatabaseVersion, szIniPath);
	pWWALUpdDlg->WriteToIni(L"DatabaseVersion", L"DatabaseVer", m_szDatabaseVersion,NULL);
	pWWALUpdDlg->WriteToEPSServerIni(L"DatabaseVersion", L"DatabaseVer", m_szDatabaseVersion, NULL);

	pWWALUpdDlg->m_edtScanEngineVersion.GetWindowTextW(m_szScanEngineVersion, 511);
	WritePrivateProfileString(L"ScanEngineVersion", L"ScanEngineVer", m_szScanEngineVersion, szIniPath);
	pWWALUpdDlg->WriteToIni(L"ScanEngineVersion", L"ScanEngineVer", m_szScanEngineVersion, NULL);
	pWWALUpdDlg->WriteToEPSServerIni(L"ScanEngineVersion", L"ScanEngineVer", m_szScanEngineVersion, NULL);

	//WritePrivateProfileString(L"DataEncVersion", L"DataEncVer", m_szDatabaseVersion, szIniPath);
	pWWALUpdDlg->WriteToIni(L"DataEncVersion", L"DataEncVer", m_szEncVersion, NULL);
	pWWALUpdDlg->WriteToEPSServerIni(L"DataEncVersion", L"DataEncVer", m_szEncVersion, NULL);

}

/***************************************************************************
  Function Name  : CreateZip32
  Description    : This function is called to make zip of 32bit Binaries 				   
  Author Name    : Nitin 
  Date           : 18 August 2014
  SR_NO			 : SR.N0 ALUPD_0028
****************************************************************************/
void CWardWizALUpdDlg ::CreateZip32(LPVOID lpParam)
{
	TCHAR	szSour[512] = {0};
	TCHAR	szDest[512] = {0};
	DWORD	dwRet = 0x00;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *) lpParam;
	//D:\WardWiz_Developement\Release\Win32\Binaries
	pWWALUpdDlg->m_FilesCount = 0x01;

	swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files for 32 bit Binaries" );
	AddLogEntry(L">>>CVibraniumALUpdDlg :: CreateZip32: Creating Zip files for 32 bit Binaries");
	if( pWWALUpdDlg->m_PatchType_Product.GetCheck( ) == BST_CHECKED && pWWALUpdDlg->m_Rebuild32.GetCheck( ) == BST_CHECKED)
	{
		swprintf_s(szSour, _countof(szSour), L"%s\\Release\\Win32\\Binaries", pWWALUpdDlg->m_szApplPath);
		swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\32", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
		pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Files_32", L"AppFolder");
		if( pWWALUpdDlg->m_FilesCount > 0x01 )
		{
			ZeroMemory(szSour, sizeof(szSour) );
			swprintf_s(szSour, _countof(szSour), L"%lu", (pWWALUpdDlg->m_FilesCount-1));
			//				pWWALUpdDlg->WriteToIni(L"Count_32", L"Count", szSour,NULL);
		}
		else
		{
			AddLogEntry(L"### Failed in CVibraniumALUpdDlg::Thread_GenerateUpdateFiles for 32 bit file count");
			dwRet = 2;
		}
	}
}

/***************************************************************************
  Function Name  : CreateZip64
  Description    : This function is called to make zip of 64bit Binaries 				   
  Author Name    : Nitin 
  Date           : 18 August 2014
  SR_NO			 : SR.N0 ALUPD_0029
****************************************************************************/
void CWardWizALUpdDlg ::CreateZip64(LPVOID lpParam)
{
	TCHAR	szSour[512] = {0};
	TCHAR	szDest[512] = {0};
	DWORD	dwRet = 0x00;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *) lpParam;
	//D:\WardWiz_Developement\Release\Win32\Binaries
	pWWALUpdDlg->m_FilesCount = 0x01;

	swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files for 64 bit Binaries" );
	AddLogEntry(L">>>CVibraniumALUpdDlg :: CreateZip32: Creating Zip files for 64 bit Binaries");
	if( pWWALUpdDlg->m_PatchType_Product.GetCheck( ) == BST_CHECKED &&  pWWALUpdDlg->m_Rebuild64.GetCheck( ) == BST_CHECKED)
	{
		pWWALUpdDlg->m_FilesCount = 0x01;
		swprintf_s(szSour, _countof(szSour), L"%s\\Release\\x64\\Binaries", pWWALUpdDlg->m_szApplPath);
		swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\64", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
		pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Files_64", L"AppFolder");
		if( pWWALUpdDlg->m_FilesCount > 0x01 )
		{
			ZeroMemory(szSour, sizeof(szSour) );
			swprintf_s(szSour, _countof(szSour), L"%lu", (pWWALUpdDlg->m_FilesCount-1));
			//	pWWALUpdDlg->WriteToIni(L"Count_64", L"Count", szSour,NULL);
		}
		else
		{
			AddLogEntry(L"### Failed in CVibraniumALUpdDlg::Thread_GenerateUpdateFiles for 64 bit file count");
			dwRet = 3;
		}
	}
}

/***************************************************************************
  Function Name  : CreateZipForDB
  Description    : This function is called to make zip of Database Files
  Author Name    : Nitin 
  Date           : 18 August 2014
  SR_NO			 : SR.N0 ALUPD_0030
****************************************************************************/
void CWardWizALUpdDlg ::CreateZipForDB(LPVOID lpParam)
{
	TCHAR	szSour[512] = {0};
	TCHAR	szDest[512] = {0};
	DWORD	dwRet = 0x00;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *) lpParam;
	//D:\WardWiz_Developement\Release\Win32\Binaries
	pWWALUpdDlg->m_FilesCount = 0x01;

	//swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files for DB files" );
	//AddLogEntry(L">>>CVibraniumALUpdDlg :: CreateZipForDB: Creating Zip files for DB files");
	//pWWALUpdDlg->m_FilesCount = 0x01;
	////D:\WardWiz_Developement\SetupRequirements\RequiredFiles
	//swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\RequiredFiles\\DB", pWWALUpdDlg->m_szApplPath);
	//swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\Common", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	//pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Common", L"AppFolder\\DB");


	swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files for Database files" );
	swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\RequiredFiles\\VBDB", pWWALUpdDlg->m_szApplPath);
	swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\CommonDB", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"CommonDB", L"AppFolder\\VBDB");

	if( pWWALUpdDlg->m_PatchType_Virus.GetCheck( ) == BST_CHECKED )
	{
		if( pWWALUpdDlg->m_FilesCount > 0x01 )
		{
			ZeroMemory(szSour, sizeof(szSour) );
			swprintf_s(szSour, _countof(szSour), L"%lu", (pWWALUpdDlg->m_FilesCount-1));
			//pWWALUpdDlg->WriteToIni(L"Common", L"Count", szSour, NULL);

			dwRet = 0;
		}
		else
		{
			AddLogEntry(L"### Failed in CVibraniumALUpdDlg::Thread_GenerateUpdateFiles for common files count1");
			dwRet = 7;
		}
	}
}

/***************************************************************************
Function Name  : CreateZipForSqliteDLL
Description    : This function is make online patch for Sqlite DLL
Author Name    : Ram Shelke
Date           : 02 Jun 2016
SR_NO		   :
****************************************************************************/
void CWardWizALUpdDlg::CreateZipForSqliteDLL(LPVOID lpParam)
{
	TCHAR	szSourB[512] = { 0 };
	TCHAR	szSourE[512] = { 0 };
	TCHAR	szDest[512] = { 0 };
	DWORD	dwRet = 0x00;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *)lpParam;
	pWWALUpdDlg->m_FilesCount = 0x01;

	swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files for Sqlite DLLs");

	swprintf_s(szSourE, _countof(szSourE), L"%s\\SetupRequirements\\RequiredFiles\\SQLITE\\32", pWWALUpdDlg->m_szApplPath);
	swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\32", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	pWWALUpdDlg->EnumAndMakeZipFiles(szSourE, szDest, L"Files_32", L"AppFolder");

	swprintf_s(szSourE, _countof(szSourE), L"%s\\SetupRequirements\\RequiredFiles\\SQLITE\\64", pWWALUpdDlg->m_szApplPath);
	swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\64", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	pWWALUpdDlg->EnumAndMakeZipFiles(szSourE, szDest, L"Files_64", L"AppFolder");
}

/***************************************************************************
Function Name  : CreateZipForSciterDLL
Description    : This function is make online patch for sciter DLL
Author Name    : Ram Shelke
Date           : 02 Jun 2016
SR_NO		   : 
****************************************************************************/
void CWardWizALUpdDlg::CreateZipForSciterDLL(LPVOID lpParam)
{
	TCHAR	szSourB[512] = { 0 };
	TCHAR	szSourE[512] = { 0 };
	TCHAR	szDest[512] = { 0 };
	DWORD	dwRet = 0x00;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *)lpParam;
	pWWALUpdDlg->m_FilesCount = 0x01;

	swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files for Sciter DLLs");

	swprintf_s(szSourE, _countof(szSourE), L"%s\\SetupRequirements\\RequiredFiles\\SCITERDLL\\32", pWWALUpdDlg->m_szApplPath);
	swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\32", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	pWWALUpdDlg->EnumAndMakeZipFiles(szSourE, szDest, L"Files_32", L"AppFolder");

	swprintf_s(szSourE, _countof(szSourE), L"%s\\SetupRequirements\\RequiredFiles\\SCITERDLL\\64", pWWALUpdDlg->m_szApplPath);
	swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\64", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	pWWALUpdDlg->EnumAndMakeZipFiles(szSourE, szDest, L"Files_64", L"AppFolder");
}

/***************************************************************************
  Function Name  : CreateZipForHelpFiles
  Description    : This function is called to make zip of Help files (i.e. CHM files) Files
  Author Name    : Nitin 
  Date           : 18 August 2014
  SR_NO			 : SR.N0 ALUPD_0031
****************************************************************************/
void CWardWizALUpdDlg ::CreateZipForHelpFiles(LPVOID lpParam)
{
	TCHAR	szSourB[512] = {0};
	TCHAR	szSourE[512] = { 0 };
	TCHAR	szDest[512] = {0};
	DWORD	dwRet = 0x00;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *) lpParam;
	//D:\WardWiz_Developement\Release\Win32\Binaries
	pWWALUpdDlg->m_FilesCount = 0x01;

	swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files for HELP files" );

	//Added code for adding CHM files for each version.
	//Niranjan Deshak - 16/03/2015.
	swprintf_s(szSourB, _countof(szSourB), L"%s\\SetupRequirements\\RequiredFiles\\WRDWIZHELP\\WRDWIZBASIC", pWWALUpdDlg->m_szApplPath);
	swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\Common", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	pWWALUpdDlg->EnumAndMakeZipFiles(szSourB, szDest, L"Common", L"AppFolder");

	swprintf_s(szSourE, _countof(szSourE), L"%s\\SetupRequirements\\RequiredFiles\\WRDWIZHELP\\WRDWIZESS", pWWALUpdDlg->m_szApplPath);
	swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\Common", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	pWWALUpdDlg->EnumAndMakeZipFiles(szSourE, szDest, L"Common", L"AppFolder");

	swprintf_s(szSourE, _countof(szSourE), L"%s\\SetupRequirements\\RequiredFiles\\WRDWIZHELP\\WRDWIZPRO", pWWALUpdDlg->m_szApplPath);
	swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\Common", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	pWWALUpdDlg->EnumAndMakeZipFiles(szSourE, szDest, L"Common", L"AppFolder");

}

/***************************************************************************
  Function Name  : CreateZipForEmailScanFiles
  Description    : This function is called to make zip of Email scan related files.
  Author Name    : Nitin 
  Date           : 18 August 2014
  SR_NO			 : SR.N0 ALUPD_0032
****************************************************************************/
void CWardWizALUpdDlg ::CreateZipForEmailScanFiles(LPVOID lpParam)
{
	TCHAR	szSour[512] = {0};
	TCHAR	szDest[512] = {0};
	DWORD	dwRet = 0x00;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *) lpParam;
	//D:\WardWiz_Developement\Release\Win32\Binaries
	pWWALUpdDlg->m_FilesCount = 0x01;


	swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files for EMAILSCAN files" );

	swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\RequiredFiles\\EMAILSCAN", pWWALUpdDlg->m_szApplPath);
	swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\Common", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Common", L"AppFolder");

}

/***************************************************************************
  Function Name  : CreateZipForWaveFiles
  Description    : This function is called to make zip of Wave files.
  Author Name    : Nitin 
  Date           : 18 August 2014
  SR_NO			 : SR.N0 ALUPD_0033
****************************************************************************/
void CWardWizALUpdDlg ::CreateZipForWaveFiles(LPVOID lpParam)
{
	TCHAR	szSour[512] = {0};
	TCHAR	szDest[512] = {0};
	DWORD	dwRet = 0x00;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *) lpParam;
	//D:\WardWiz_Developement\Release\Win32\Binaries
	pWWALUpdDlg->m_FilesCount = 0x01;


	swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files for WAVFILES files" );

	swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\RequiredFiles\\WAVFILES", pWWALUpdDlg->m_szApplPath);
	swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\Common", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Common", L"AppFolder");
}

/***************************************************************************
Function Name  : CreateZipForLogoFiles
Description    : This function is called to make zip of Logo
Author Name    : Nitin
Date           : 8th Jan 2016
SR_NO		   : 
****************************************************************************/
void CWardWizALUpdDlg::CreateZipForLogoFiles(LPVOID lpParam)
{
	TCHAR	szSour[512] = { 0 };
	TCHAR	szDest[512] = { 0 };
	DWORD	dwRet = 0x00;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *)lpParam;
	//D:\WardWiz_Developement\Release\Win32\Binaries
	pWWALUpdDlg->m_FilesCount = 0x01;


	swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files for Logo files");

	swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\Logo", pWWALUpdDlg->m_szApplPath);
	swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\Common", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Common", L"AppFolder");
}
/***************************************************************************
  Function Name  : CreateZipForSecNewsFiles
  Description    : This function is called to make zip of Security news related files.
  Author Name    : Nitin 
  Date           : 18 August 2014
  SR_NO			 : SR.N0 ALUPD_0034
****************************************************************************/
void CWardWizALUpdDlg ::CreateZipForSecNewsFiles(LPVOID lpParam)
{
	TCHAR	szSour[512] = {0};
	TCHAR	szDest[512] = {0};
	DWORD	dwRet = 0x00;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *) lpParam;
	//D:\WardWiz_Developement\Release\Win32\Binaries
	pWWALUpdDlg->m_FilesCount = 0x01;


	swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files for SECURITY NEWS files" );

	swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\RequiredFiles\\VIBRANIUMSECURITYNEWS", pWWALUpdDlg->m_szApplPath);
	swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\Common", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Common", L"AppFolder");
}

/***************************************************************************
  Function Name  : CreateZipForSetting
  Description    : This function is called to make zip of settings files.
  Author Name    : Nitin 
  Date           : 18 August 2014
  SR_NO			 : SR.N0 ALUPD_0035
****************************************************************************/
void CWardWizALUpdDlg ::CreateZipForSetting(LPVOID lpParam)
{
	TCHAR	szSour[512] = {0};
	TCHAR	szDest[512] = {0};
	DWORD	dwRet = 0x00;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *) lpParam;
	//D:\WardWiz_Developement\Release\Win32\Binaries
	pWWALUpdDlg->m_FilesCount = 0x01;


	swprintf_s(	pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files for SETTINGS" );

	swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\SETTINGS", pWWALUpdDlg->m_szApplPath);
	swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\Common", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Common", L"AppFolder\\VBSETTINGS");

	if( pWWALUpdDlg->m_FilesCount > 0x01 )
	{
		ZeroMemory(szSour, sizeof(szSour) );
		swprintf_s(szSour, _countof(szSour), L"%lu", (pWWALUpdDlg->m_FilesCount-1));
		//pWWALUpdDlg->WriteToIni(L"Common", L"Count", szSour, NULL);
	}
	else
	{
		AddLogEntry(L"### Failed in CVibraniumALUpdDlg::Thread_GenerateUpdateFiles for common files count2");
		dwRet = 3;
	}
}


/***************************************************************************
Function Name  : CreateZipForSetting
Description    : This function is called to make zip of settings files.
Author Name    : Nitin
Date           : 18 August 2014
SR_NO			 : SR.N0 ALUPD_0035
****************************************************************************/
void CWardWizALUpdDlg::CreateZipForSETUPDBFolder(LPVOID lpParam)
{
	TCHAR	szSour[512] = { 0 };
	TCHAR	szDest[512] = { 0 };
	DWORD	dwRet = 0x00;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *)lpParam;
	//D:\WardWiz_Developement\Release\Win32\Binaries
	pWWALUpdDlg->m_FilesCount = 0x01;


	swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files SETUPDB folder");

	swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\RequiredFiles\\SETUPDB", pWWALUpdDlg->m_szApplPath);
	swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\Common", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Common", L"AppFolder");

	if (pWWALUpdDlg->m_FilesCount > 0x01)
	{
		ZeroMemory(szSour, sizeof(szSour));
		swprintf_s(szSour, _countof(szSour), L"%lu", (pWWALUpdDlg->m_FilesCount - 1));
		//pWWALUpdDlg->WriteToIni(L"Common", L"Count", szSour, NULL);
	}
	else
	{
		AddLogEntry(L"### Failed in CVibraniumALUpdDlg::Thread_GenerateUpdateFiles for common files count2");
		dwRet = 3;
	}
}


/***************************************************************************
Function Name  : CreateZipForSetting
Description    : This function is called to make zip of settings files.
Author Name    : Nitin
Date           : 18 August 2014
SR_NO			 : SR.N0 ALUPD_0035
****************************************************************************/
void CWardWizALUpdDlg::CreateZipForNSSFolder(LPVOID lpParam)
{
	TCHAR	szSour[512] = { 0 };
	TCHAR	szDest[512] = { 0 };
	DWORD	dwRet = 0x00;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *)lpParam;
	//D:\WardWiz_Developement\Release\Win32\Binaries
	pWWALUpdDlg->m_FilesCount = 0x01;


	swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files NSS folder");
	
	swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\RequiredFiles\\NSS", pWWALUpdDlg->m_szApplPath);
	swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\Common", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Common", L"AppFolder\\NSS");

	if (pWWALUpdDlg->m_FilesCount > 0x01)
	{
		ZeroMemory(szSour, sizeof(szSour));
		swprintf_s(szSour, _countof(szSour), L"%lu", (pWWALUpdDlg->m_FilesCount - 1));
		//pWWALUpdDlg->WriteToIni(L"Common", L"Count", szSour, NULL);
	}
	else
	{
		AddLogEntry(L"### Failed in CVibraniumALUpdDlg::Thread_GenerateUpdateFiles for common files count2");
		dwRet = 3;
	}
}

/***************************************************************************
  Function Name  : UpdateWWBinary
  Description    : This function is called to Write the latest Product and Database version into "WWBinary.ini" file.
  Author Name    : Nitin 
  Date           : 18 August 2014
  SR_NO			 : SR.N0 ALUPD_0036
****************************************************************************/
void CWardWizALUpdDlg :: UpdateWWBinary(LPVOID lpParam)
{
	DWORD	dwRetValue = 0x00 ;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *) lpParam;
	ZeroMemory(m_szProdVersion, sizeof(m_szProdVersion));
	ZeroMemory(m_szDatabaseVersion, sizeof(m_szDatabaseVersion));
	ZeroMemory(m_szScanEngineVersion, sizeof(m_szScanEngineVersion));
	pWWALUpdDlg->m_edtProdVersion.GetWindowText(m_szProdVersion,255);
	pWWALUpdDlg->m_edtDatabaseVersion.GetWindowText(m_szDatabaseVersion, 255);
	pWWALUpdDlg->m_edtScanEngineVersion.GetWindowText(m_szScanEngineVersion, 255);
	
	dwRetValue = WritePrivateProfileString(L"ProductVersion", L"ProductVer", m_szProdVersion, pWWALUpdDlg->m_szWWBinariesPath);
	if( !dwRetValue)
	{
		AddLogEntry(L"### Failed in CVibraniumALUpdDlg::UpdateVibraniumBinary(%s)",m_szProdVersion );
	}
	
	dwRetValue = WritePrivateProfileString(L"DatabaseVersion", L"DatabaseVer", m_szDatabaseVersion, pWWALUpdDlg->m_szWWBinariesPath);
	if( !dwRetValue)
	{
		AddLogEntry(L"### Failed in CVibraniumALUpdDlg::UpdateVibraniumBinary(%s)", m_szDatabaseVersion);
	}

	dwRetValue = WritePrivateProfileString(L"ScanEngineVersion", L"ScanEngineVer", m_szScanEngineVersion, pWWALUpdDlg->m_szWWBinariesPath);
	if (!dwRetValue)
	{
		AddLogEntry(L"### Failed in CVibraniumALUpdDlg::UpdateVibraniumBinary(%s)", m_szScanEngineVersion);
	}

	/*dwRetValue = WritePrivateProfileString(L"DataEncVersion", L"DataEncVer", m_szEncVersion, pWWALUpdDlg->m_szProIniPath);
	if (!dwRetValue)
	{
		AddLogEntry(L"### Failed in CVibraniumALUpdDlg::UpdateWWBinary(%s)", m_szEncVersion);
	}*/
}

/***************************************************************************
  Function Name  : UploadFilesToLocalServer
  Description    : This function is called to upload the files on Local server
				   server location is "\\192.168.2.99\\Vibranium\\WardWiz AV Installable\\"
  Author Name    : Nitin 
  Date           : 18 August 2014
  SR_NO			 : SR.N0 ALUPD_0037
****************************************************************************/
bool CWardWizALUpdDlg :: UploadFilesToLocalServer()
{
	TCHAR		szFileName[128] = {0};
	TCHAR		szFilePath[512] = {0};
	DWORD		dwCount = 0x01;
	TCHAR		szTemp[512] = {0};
	TCHAR		csDestPath[512] = {0};
	TCHAR		csDestPathPatchesDir[512] = {0};
	TCHAR		csDestPathSetupDir[512] = {0};
	TCHAR		csDestPathLogDir[512] = {0};

	TCHAR		csDestPathTemp[512] = {0};
	 TCHAR virusPatchDir[MAX_PATH] = _T("");
	bool		flag = false;
	CFileFind	finder;
	SYSTEMTIME	ST = {0};
	GetSystemTime( &ST);

	ZeroMemory(m_szOutDirName, sizeof(m_szOutDirName) );
	swprintf_s(	m_szOutDirName, _countof(m_szOutDirName), L"%d_%d_%d_%d_%d", 
	ST.wDay, ST.wMonth, ST.wYear, ST.wHour, ST.wMinute);

	swprintf_s(	m_szStatusText, _countof(m_szStatusText), L"Uploading files on server" );
	AddLogEntry(L">>>CVibraniumALUpdDlg :: UploadFilesToLocalServer: Uploading files on server");

	//csDetPath.Format(L"%s%s%s",L"\\",L"\\192.168.2.99\\Vibranium\\WardWiz AV Installable\\",m_szOutDirName);
	swprintf_s(csDestPath, _countof(csDestPath), L"%s%s%s",L"\\",L"\\172.168.1.44\\xampp\\htdocs\\vb20\\",m_szOutDirName);
	// swprintf_s(csDestPath, _countof(csDestPath), L"%s%s",L"D:\\AUTOMATION\\",m_szOutDirName);
	 swprintf_s(csDestPathPatchesDir, _countof(csDestPathPatchesDir), L"%s%s%s",csDestPath,L"\\",L"Product Patches");
	 swprintf_s(csDestPathSetupDir, _countof(csDestPathSetupDir), L"%s%s%s",csDestPath,L"\\",L"Product Setups");
	 swprintf_s(csDestPathLogDir, _countof(csDestPathLogDir), L"%s%s%s",csDestPath,L"\\",L"LOG");
	
	
	flag = CreateDirectory(csDestPath,NULL);
	if(!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create directory on server");
		m_bIsSetupCreated = true;
		return false;
	}
	

	flag = true;
		flag = CreateDirectory(csDestPathLogDir,NULL);
		if(!flag)
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create patches directory on server");
			m_bIsSetupCreated = true;
			return false;
		}
		else   // copy log file to server
		{	
			UploadLogFilesToServer(m_szOutDirName,false);

		}

	if( m_PatchType_Product.GetCheck( ) == BST_CHECKED)
	{
			flag = true;
			flag = CreateDirectory(csDestPathPatchesDir,NULL);
			if(!flag)
			{
				AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create patches directory on server");
				m_bIsSetupCreated = true;
				return false;
			}
		if( !PathFileExists(m_szPatchFolderPath) )
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Path not found");
			return false;
		}

		swprintf_s(szTemp, _countof(szTemp), L"%s\\*.*", m_szPatchFolderPath);
		BOOL bWorking = finder.FindFile(szTemp);

		while (bWorking)
		{
			bWorking = finder.FindNextFile();

			// skip . and .. files; otherwise, we'd 
			// recur infinitely! 

			if (finder.IsDots())
				continue;

			if (finder.IsDirectory())	
			{
				wcscpy(szFileName, finder.GetFileName() );
				wcscpy(szFilePath, finder.GetFilePath() );
				swprintf_s(csDestPathTemp, _countof(csDestPathTemp), L"%s%s%s",csDestPathPatchesDir,L"\\",szFileName);
				flag = CreateDirectory(csDestPathTemp,NULL);
				if(!flag)
				{
					AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create directory on server");
					return false;
				}
				EnumFilesAndCopyToServer(szFilePath , csDestPathTemp);
				continue;
			}

			wcscpy(szFileName, finder.GetFileName() );
			wcscpy(szFilePath, finder.GetFilePath() );

			swprintf_s(csDestPathTemp, _countof(csDestPathTemp),L"%s\\%s",csDestPathPatchesDir,szFileName);
			flag = CopyFile(szFilePath, csDestPathTemp, TRUE);
			if(!flag)
			{
				AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to copy file: %s",szFilePath);
			}
		}
		finder.Close();
	}
	//
	wcscpy(virusPatchDir,m_szPatchFolderPath );

	if( m_MakeSetup.GetCheck( ) == BST_CHECKED)
	{
			flag = true;
			flag = CreateDirectory(csDestPathSetupDir,NULL);
			if(!flag)
			{
				AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
				m_bIsSetupCreated = true;
				return false;
			}

			ZeroMemory(m_szPatchFolderPath, sizeof(m_szPatchFolderPath) );
			swprintf_s(m_szPatchFolderPath, _countof(m_szPatchFolderPath), L"%s\\SetupRequirements\\Output", m_szApplPath);
			if( !PathFileExists(m_szPatchFolderPath))
			{
				AddLogEntry(L"### Failed CVibraniumALUpdDlg::Path not found");
				return false;
			}
			if (!CreateAndCopyFilesforBasicEssPro(csDestPathSetupDir, m_szPatchFolderPath))
				return false;

	}
if( m_PatchType_Virus.GetCheck() == BST_CHECKED)
	{

		if( !PathFileExists(virusPatchDir) )
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Path not found");
			return false;
		}
		
				 
		//ZeroMemory(m_szPatchFolderPath, sizeof(m_szPatchFolderPath) );
		swprintf_s(virusPatchDir, _countof(virusPatchDir), L"%s\\Common", virusPatchDir);
		if( !PathFileExists(virusPatchDir))
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Path not found");
			return false;
		}
		if( !PathFileExists(csDestPathPatchesDir) )
		{
			flag = true;
			flag = CreateDirectory(csDestPathPatchesDir,NULL);
			if(!flag)
			{
				AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create patches directory on server");
				m_bIsSetupCreated = true;
				return false;
			}
		}
		swprintf_s(csDestPathTemp, _countof(csDestPathTemp), L"%s\\Common", csDestPathPatchesDir);
		flag = CreateDirectory(csDestPathTemp,NULL);
		if(!flag)
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create Output directory on server");
			return false;
		}

		swprintf_s(szTemp, _countof(szTemp), L"%s\\*.*", virusPatchDir);
		BOOL bWorking = finder.FindFile(szTemp);

		while (bWorking)
		{
			bWorking = finder.FindNextFile();

			// skip . and .. files; otherwise, we'd 
			// recur infinitely! 

			if (finder.IsDots())
				continue;

			if (finder.IsDirectory())	
			{
				wcscpy(szFileName, finder.GetFileName() );
				wcscpy(szFilePath, finder.GetFilePath() );
				swprintf_s(csDestPathTemp, _countof(csDestPathTemp), L"%s%s%s",csDestPathPatchesDir,L"\\",szFileName);
				flag = CreateDirectory(csDestPathTemp,NULL);
				if(!flag)
				{
					AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create directory on server");
					return false;
				}
				EnumFilesAndCopyToServer(szFilePath , csDestPathTemp);
				continue;
			}

			wcscpy(szFileName, finder.GetFileName() );
			wcscpy(szFilePath, finder.GetFilePath() );
			
			ZeroMemory(szTemp, sizeof(szTemp) );
			swprintf_s(szTemp, _countof(szTemp),L"%s\\%s",csDestPathTemp,szFileName);
			flag = CopyFile(szFilePath, szTemp, TRUE);
			if(!flag)
			{
				AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to copy file: %s",szFilePath);
			}
		}
		finder.Close();
	}

	swprintf_s(	m_szStatusText, _countof(m_szStatusText), L"Uploading file successful." );
	AddLogEntry(L">>>CVibraniumALUpdDlg :: UploadFilesToLocalServer: Uploading file successful.");
	AfxMessageBox(L"Uploading file successful.", MB_ICONINFORMATION); 
	m_bIsSetupCreated = true;
	//Updating Product version by 1 from WWBinary.ini file to create setup next time .
    //Niranjan Deshak - 16/03/2015.
	m_bIsSetupCreatedAndFilesUploaded = true;

	return true;
}

/***************************************************************************
  Function Name  : EnumFilesAndCopyToServer
  Description    : This function is called to copy the files from machine to server
  Author Name    : Nitin 
  Date           : 18 August 2014
  SR_NO			 : SR.N0 ALUPD_0038
****************************************************************************/
void CWardWizALUpdDlg :: EnumFilesAndCopyToServer( TCHAR *pSource, TCHAR*pDest)
{
	TCHAR	szFileName[128] = {0};
	TCHAR	szFilePath[512] = {0};
	TCHAR csDestPathTemp[512] = {0};

	CFileFind finder;
	bool flag = false;
	TCHAR	szTemp[512] = {0};


	swprintf_s(szTemp, _countof(szTemp), L"%s\\*.*", pSource);
	BOOL bWorking = finder.FindFile(szTemp);

	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		// skip . and .. files; otherwise, we'd 
		// recur infinitely! 

		if (finder.IsDots())
			continue;

		if (finder.IsDirectory())	
		{
			wcscpy(szFileName, finder.GetFileName() );
			wcscpy(szFilePath, finder.GetFilePath() );
			EnumFilesAndCopyToServer(szFilePath, pDest);
			continue;
		}
		
		wcscpy(szFileName, finder.GetFileName() );
		wcscpy(szFilePath, finder.GetFilePath() );
		swprintf_s(csDestPathTemp, _countof(csDestPathTemp),L"%s\\%s",pDest,szFileName);
		flag = CopyFile(szFilePath, csDestPathTemp, TRUE);
		if(!flag)
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to copy file: %s",szFilePath);
		}

	}
	finder.Close();
}
/***************************************************************************
  Function Name  : OnBnClickedButtonAbort
  Description    : This function is called to abort any task
  Author Name    : Lalit kumawat 
  Date           : 
  SR_NO			 : 
****************************************************************************/
void CWardWizALUpdDlg::OnBnClickedButtonAbort()
{
	
	// TODO: Add your control notification handler code here
	try
	{
		
		if( m_hThread_GenerateUpdateFiles )
		{
			if( AfxMessageBox(L"Do you want to Abort", MB_YESNO ) == IDNO )
				return ;
			m_bIsSetupCreatedAndFilesUploaded = false; //prodversion won't increment by 1
			if( m_hThread_SetStatus )
			{
				SuspendThread( m_hThread_SetStatus ) ;
				TerminateThread( m_hThread_SetStatus, 0 );
				CloseHandle( m_hThread_SetStatus );
				m_hThread_SetStatus = NULL ;
			}

			SuspendThread( m_hThread_GenerateUpdateFiles ) ;
			TerminateThread( m_hThread_GenerateUpdateFiles, 0 );
			CloseHandle( m_hThread_GenerateUpdateFiles );

			m_hThread_GenerateUpdateFiles = NULL ;
		}

		m_Generate_Update_Files.EnableWindow( TRUE );
		//swprintf_s(	m_szStatusText, _countof(m_szStatusText), L"Aborted..." );
		m_List_PatchFiles.DeleteAllItems();
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::OnBnClickedButtonAbort");
	}
}

/***************************************************************************
  Function Name  : OnBnClickedButtonAddsign
  Description    : This function is use to add signature to selected exe/dll file
  Author Name    : Lalit kumawat 
  Date           : 
  SR_NO			 : 
****************************************************************************/
void CWardWizALUpdDlg::OnBnClickedButtonAddsign()
{
	//// TODO: Add your control notification handler code here

	try
	{   m_bIsSigningCompleted = false;
		m_hThread_SignStatus = NULL;
	   	m_hThread_SignStatus = ::CreateThread(	NULL, 0, (LPTHREAD_START_ROUTINE) Thread_AddSignatureToSelectedFiles, (LPVOID)this, 0, 0 ) ;
		SetTimer(TIMER_CHECKSIGNINGCOMPLETED, 150, NULL);
		
		
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::OnBnClickedButtonAddsign");
	}
}

/***************************************************************************
  Function Name  : OnBnClickedButtonBrowse
  Description    : This function is use to browse any file to add signature
  Author Name    : Lalit kumawat 
  Date           : 
  SR_NO			 : 
****************************************************************************/
void CWardWizALUpdDlg::OnBnClickedButtonBrowse()
{	
// TODO: Add your control notification handler code here
	try
	{
		static TCHAR szEncFilter[] = L"All Files(*.*)|*.*|";   
		CFileDialog FileDlg(TRUE, L"All Files(*.*)", NULL, OFN_ALLOWMULTISELECT, (LPCTSTR)szEncFilter);
		TCHAR commandLine[] = _T("");
		if( FileDlg.DoModal() == IDOK )
			{
				mbIsFileSelected = true ;
				CString csFilePath = FileDlg.GetPathName();
			
				POSITION pos ( FileDlg.GetStartPosition() );
				int iFileCount =0;
				m_lstSelectedFileList.DeleteAllItems(); 
				while( pos )
				{
					CString filename = FileDlg.GetNextPathName(pos);
					msSelectedFiles[iFileCount] = filename;
					m_lstSelectedFileList.InsertItem(iFileCount, filename );
					m_lstSelectedFileList.SetItemText(iFileCount, 1,L"Pending" );
					iFileCount++;
				}

				m_iSelectedFileCount = iFileCount;

			}
			else
			{
				mbIsFileSelected = false ;
			}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::OnBnClickedButtonBrowse");
	}
}

/***************************************************************************
  Function Name  : AddSignToSpecifiedFile
  Description    : This function is use to add signature to specified file
  Author Name    : Lalit kumawat 
  Date           : 
  SR_NO			 : 
****************************************************************************/
int CWardWizALUpdDlg:: AddSignToSpecifiedFile(CString fPath, bool bUpdateVersion)
	{
		CString csFileName(L"");
		CString csFilePath(L"");
		int bisFileSigned =  0;
		try
		{
		
		if(mbIsFileSelected)
		{					
				IsFileSetupDllFile(fPath);

				//bool bisFileSigned =  true;
				bool  bsignStatus = false;
				if(IsFileHaveSignature(fPath))
				{
					bisFileSigned = 1;
				}
				else
				{
					//bsignStatus = true;   // file has to sign  signed
					csFilePath = fPath;
					int iPos = 0;
					iPos = csFilePath.ReverseFind('\\');
					if (iPos != -1)
					{
						csFileName = csFilePath.Mid(csFilePath.ReverseFind('\\') + 1);
					}

					CString smodulePath;
					TCHAR szModulePath[MAX_PATH] = {0};
					if(!GetModulePath(szModulePath, MAX_PATH))
					{
						//MessageBox(NULL, L"Error in GetModulePath",  L"Vibranium", MB_ICONERROR);
						return false;
					}
					smodulePath= szModulePath;

					TCHAR	szTemp[1024] = {0};
					TCHAR	szSignCmd1[1024] = {0};
					TCHAR	szSignCmd2[1024] = {0};
  					swprintf_s(	szTemp, _countof(szTemp), L"%s\\SetupRequirements\\DigitalSignature\\TempADDSignToSelectedFile.bat", smodulePath);
							
					//HANDLE hFile = CreateFile(_T("D:\\WardWiz_Developement\\SetupRequirements\\DigitalSignature\\TempADDSignToSelectedFile.bat"),
		     		HANDLE hFile = CreateFile(szTemp,GENERIC_WRITE, FILE_SHARE_READ,NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			
					CFile myFile(hFile);
					
					if (bUpdateVersion)
					{
						//code to modify version number
						TCHAR szVersion[MAX_PATH] = { 0 };
						swprintf_s(szVersion, _countof(szVersion), L"\"%s (%sdate%s)\"", m_szProdVersion, L"%", L"%");

						TCHAR szFiledescr[MAX_PATH] = { 0 };
						swprintf_s(szFiledescr, _countof(szFiledescr), L"/s desc \"Vibranium Security\"");

						TCHAR szCompinfo[MAX_PATH] = { 0 };
						swprintf_s(szCompinfo, _countof(szCompinfo), L"/s company \"Vibranium\" /s Copyright \"2024 Vibranium Security Lab. All Rights Reserved\"");

						TCHAR szProdinfo[MAX_PATH] = { 0 };
						swprintf_s(szProdinfo, _countof(szProdinfo), L"/s product \"Vibranium Security\" /pv \"%s\"\n", m_szProdVersion);

						TCHAR szLine[MAX_PATH * 4] = { 0 };
						swprintf_s(szLine, _countof(szLine), L"%s\\SetupRequirements\\DigitalSignature\\verpatch.exe /va %s %s %s %s %s\n", smodulePath, fPath, szVersion, szFiledescr, szCompinfo, szProdinfo);
						myFile.Write(CT2A(szLine), _tcslen(szLine));
					}
					
					bool bSignBySHA1 = false;
					for (int i = 0; i < m_listSHA1SignBinaries.size(); i++)
					{
						if ((csFileName.CompareNoCase(m_listSHA1SignBinaries.at(i))) == 0)
						{
							bSignBySHA1 = true;
							swprintf_s(szSignCmd1, _countof(szSignCmd1), L"signtool sign /f \"%s\\SetupRequirements\\DigitalSignature\\OS201601223846.pfx\" /P \"WelservWardWiz123\"", smodulePath);
							swprintf_s(szSignCmd2, _countof(szSignCmd1), L"signtool sign /t http://timestamp.digicert.com /f  \"%s\\SetupRequirements\\DigitalSignature\\OS201601223846.pfx\" /P \"WelservWardWiz123\"", smodulePath);
							break;
						}
					}
					if (!bSignBySHA1)
					{
						//for (int i = 0; i < m_listSHA2SignBinaries.size(); i++)
						//{
						//	if ((csFileName.CompareNoCase(m_listSHA2SignBinaries.at(i)))== 0)
						//	{
						swprintf_s(szSignCmd2, _countof(szSignCmd2), L"\"C:\\Program Files (x86)\\Windows Kits\\8.0\\bin\\x86\\signtool.exe\" sign /n \"WardWiz Deutschland GmbH\" /tr http://rfc3161timestamp.globalsign.com/standard /td SHA256 /SHA1 FD61709512215CE6B49DF3AA7B605994FDB365E3");
						//		break;
						//	}
						//}
					}
					
					
					CString sText1 = szSignCmd1;
					CString sText2 = szSignCmd2;
					if (!bSignBySHA1)
					{
						TCHAR commandLine[] = _T("");

						//sText1  =sText1 + _T(" ") + _T("\"") + fPath + _T("\"\n");
						sText2 = sText2 + _T(" ") + _T("\"") + fPath + _T("\"\n");
						sText1 = /*sText1 +*/ sText2;
						myFile.Write(CT2A(sText1), sText1.GetLength());
						myFile.Close();
					}
					else
					{
						TCHAR commandLine[] = _T("");

						sText1  =sText1 + _T(" ") + _T("\"") + fPath + _T("\"\n");
						sText2 = sText2 + _T(" ") + _T("\"") + fPath + _T("\"\n");
						sText1 = sText1 + sText2;
						myFile.Write(CT2A(sText1), sText1.GetLength());
						myFile.Close();
					}

					bSignBySHA1 = false;
					swprintf_s(	szTemp, _countof(szTemp), L"\" /c %s\\SetupRequirements\\DigitalSignature\\TempADDSignToSelectedFile.bat\"", smodulePath);
			
					gbIsBatchFileRunning = true;
					RebuildSolution(szTemp);
					//mSelectedFileName.SetWindowTextW(L"") ;
				
				  if(IsFileHaveSignature(fPath))
				  {
					bisFileSigned = 2;
					IsFileSetupDllFile(fPath);
				  }
				  else
				  {
					bisFileSigned = 0;
				  }
			 }
			
			}
		}
		catch(...)
		{
			AddLogEntry(L"### Exception in CVibraniumALUpdDlg::AddSignToSpecifiedFile");
		}
		return bisFileSigned;
}

/***************************************************************************
  Function Name  : AddSignatureToFinalSetup
  Description    : This function is use to add signature to final setup
  Author Name    : Lalit kumawat 
  Date           : 
  SR_NO			 : 
****************************************************************************/
bool CWardWizALUpdDlg:: AddSignatureToFinalSetup(void)
{
	
		TCHAR		szFileName[128] = {0};
		TCHAR		szFilePath[512] = {0};
		DWORD		dwCount = 0x01;
		TCHAR		szTemp[512] = {0};
		TCHAR		csDestPath[512] = {0};
		TCHAR		csSourceDirectoryPath[512] = {0};
		bool		flag = false;
		CFileFind	finder;
	
		try
		{

			if( m_MakeSetup.GetCheck( ) == BST_CHECKED)
      		{
			ZeroMemory(csSourceDirectoryPath, sizeof(csSourceDirectoryPath) );
			swprintf_s(csSourceDirectoryPath, _countof(csSourceDirectoryPath), L"%s\\SetupRequirements\\Output", m_szApplPath);
			if( !PathFileExists(csSourceDirectoryPath))
			{
				AddLogEntry(L"### Failed CVibraniumALUpdDlg::Path not found");
				return false;
			}

			swprintf_s(szTemp, _countof(szTemp), L"%s\\*.*", csSourceDirectoryPath);
			BOOL bWorking = finder.FindFile(szTemp);

			while (bWorking)
			{
				bWorking = finder.FindNextFile();

				// skip . and .. files; otherwise, we'd 
				// recur infinitely! 

				if (finder.IsDots())
					continue;

				if (finder.IsDirectory())	
				{
					wcscpy(szFileName, finder.GetFileName() );
					wcscpy(szFilePath, finder.GetFilePath() );
						continue;
				}

				wcscpy(szFileName, finder.GetFileName() );
				wcscpy(szFilePath, finder.GetFilePath() );

				AddSignToSpecifiedFile(szFilePath, false);
				
			}
			finder.Close();
		}
	}
	catch(...)
	{
	 AddLogEntry(L"### Exception in CVibraniumALUpdDlg::AddSignatureToFinalSetup");
	}
	return true;
}

/***************************************************************************
  Function Name  : IsFileHaveSignature
  Description    : it use to check "is file having signature."
  Author Name    : lalit kumawat
  Date           : 
  SR_NO			 : 
****************************************************************************/
bool CWardWizALUpdDlg:: IsFileHaveSignature(CString FileName)
{
	try
	{
	  GUID guidAction = WINTRUST_ACTION_GENERIC_VERIFY_V2;
		WINTRUST_FILE_INFO sWintrustFileInfo;
		WINTRUST_DATA      sWintrustData;
		HRESULT            hr;

	   memset((void*)&sWintrustFileInfo, 0x00, sizeof(WINTRUST_FILE_INFO));
	   memset((void*)&sWintrustData, 0x00, sizeof(WINTRUST_DATA));

	   sWintrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
	   //sWintrustFileInfo.pcwszFilePath = argv[1];                // file path
	   sWintrustFileInfo.pcwszFilePath =  FileName;
	
	   sWintrustFileInfo.hFile = NULL;

	   sWintrustData.cbStruct            = sizeof(WINTRUST_DATA);
	   sWintrustData.dwUIChoice          = WTD_UI_NONE;
	   sWintrustData.fdwRevocationChecks = WTD_REVOKE_NONE;
	   sWintrustData.dwUnionChoice       = WTD_CHOICE_FILE;
	   sWintrustData.pFile               = &sWintrustFileInfo;
	   sWintrustData.dwStateAction       = WTD_STATEACTION_VERIFY;

	   hr = WinVerifyTrust((HWND)INVALID_HANDLE_VALUE, &guidAction, &sWintrustData);

	   if (TRUST_E_NOSIGNATURE == hr)
	   {
		 // _tprintf(_T("No signature found on the file.\n"));
		   return false;
	   }
	   else if (TRUST_E_BAD_DIGEST == hr)
	   {
		 return false;
		   // _tprintf(_T("The signature of the file is invalid\n"));
	   }
	   else if (TRUST_E_PROVIDER_UNKNOWN == hr)
	   {
		 return false;
		   // _tprintf(_T("No trust provider on this machine can verify this type of files.\n"));
	   }
	   else if (S_OK != hr)
	   {
		   return false;
		  //_tprintf(_T("WinVerifyTrust failed with error 0x%.8X\n"), hr);
	   }
	   else
	   {
		   return true;
		   // _tprintf(_T("File signature is OK.\n"));
	   }
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::IsFileHaveSignature");
	}
	return true;
}
/***************************************************************************
  Function Name  : OnTimer
  Description    : it handle timer event
  Author Name    : vilas
  Date           : 
  SR_NO			 : 
****************************************************************************/
void CWardWizALUpdDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
if(nIDEvent == TIMER_SETUPUPLOADEDTOSERVER)
		{
			try
			{
				if(m_bIsSetupCreated)
				{
					if( m_hThread_GenerateUpdateFiles )
						{
							if( m_hThread_SetStatus )
							{
								SuspendThread( m_hThread_SetStatus ) ;
								TerminateThread( m_hThread_SetStatus, 0 );
								CloseHandle( m_hThread_SetStatus );
								m_hThread_SetStatus = NULL ;
							}

							SuspendThread( m_hThread_GenerateUpdateFiles ) ;
							TerminateThread( m_hThread_GenerateUpdateFiles, 0 );
							CloseHandle( m_hThread_GenerateUpdateFiles );

							m_hThread_GenerateUpdateFiles = NULL ;
						}
					exit(0);
				}
				
			}
			catch(...)
			{
				AddLogEntry(L"### Exception in CVibraniumALUpdDlg::OnTimer");
			}
			
	   }
		
	if(nIDEvent == TIMER_CHECKSIGNINGCOMPLETED)
	{
		try
		{
			if(m_bIsSigningCompleted)
			{
				if( m_hThread_SignStatus )
					{
						SuspendThread( m_hThread_SignStatus ) ;
						TerminateThread( m_hThread_SignStatus, 0 );
						CloseHandle( m_hThread_SignStatus );
						m_hThread_SignStatus = NULL ;
						AfxMessageBox(_T("Signing Operation completed."));
						KillTimer(TIMER_CHECKSIGNINGCOMPLETED); 
					}
				
			}
			m_bIsSigningCompleted = false;
		}
		catch(...)
		{
			AddLogEntry(L"### Exception in CVibraniumALUpdDlg::OnTimer");
		}
	}

	CDialog::OnTimer(nIDEvent);
}

/***************************************************************************
  Function Name  : UploadLogFilesToServer
  Description    : it use to  upload log file on server
  Author Name    : lalit kumawat 
  Date           : 
  SR_NO			 : 
****************************************************************************/
bool CWardWizALUpdDlg::UploadLogFilesToServer(TCHAR *szDestDirPath,bool isbErrorOccur)
{
	try
	{
		TCHAR		csDestPath[512] = {0};
		TCHAR		csDestPathLogDir[512] = {0};
		TCHAR		csDestPathTemp[512] = {0};
		bool		flag = false;

		SYSTEMTIME	ST = {0};
		GetSystemTime( &ST);


		if(isbErrorOccur)
		{
		ZeroMemory(m_szOutDirName, sizeof(m_szOutDirName) );
		swprintf_s(	m_szOutDirName, _countof(m_szOutDirName), L"%d_%d_%d_%d_%d", 
		ST.wDay, ST.wMonth, ST.wYear, ST.wHour, ST.wMinute);
		}
		else
		{
			wcscpy(m_szOutDirName,szDestDirPath );
		}

		//swprintf_s(	m_szStatusText, _countof(m_szStatusText), L"Uploading files on server" );
		//AddLogEntry(L">>>CNextGenExALUpdDlg :: UploadFilesToLocalServer: Uploading files on server");

		//csDetPath.Format(L"%s%s%s",L"\\",L"\\192.168.2.99\\Vibranium\\WardWiz AV Installable\\",m_szOutDirName);
		swprintf_s(csDestPath, _countof(csDestPath), L"%s%s%s",L"\\",L"\\\\192.168.1.44\\Vibranium\\WardWiz AV Installable\\",m_szOutDirName);
		//swprintf_s(csDestPath, _countof(csDestPath), L"%s%s",L"D:\\AUTOMATION\\",m_szOutDirName);
		 swprintf_s(csDestPathLogDir, _countof(csDestPathLogDir), L"%s%s%s",csDestPath,L"\\",L"LOG");
		
		if( !PathFileExists(csDestPath))
			{
				
				flag = CreateDirectory(csDestPath,NULL);
				if(!flag)
				{
					AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create directory on server");
					m_bIsSetupCreated = true;
					return false;
				}
		
			}
		if( !PathFileExists(csDestPathLogDir))
				{
					flag = CreateDirectory(csDestPathLogDir,NULL);
					if(!flag)
					{
						AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create directory on server");
						m_bIsSetupCreated = true;
						return false;
					}
			
				}

			
			TCHAR		szALULog[128] = {0};
			TCHAR		szRebuildLog64[128] = {0};
			TCHAR		szRebuildLog32[128] = {0};
			TCHAR		szLogSourcePath[128] = {0};
			TCHAR		szDestRebuildLog64[128] = {0};
			TCHAR		szDestRebuildLog32[128] = {0};
			TCHAR		szDestALULogPath[128] = {0};

			TCHAR szModulePath[MAX_PATH] = {0};

			if(!GetModulePath(szModulePath, MAX_PATH))
				{
				return false;
				}
			wcscpy(szLogSourcePath,szModulePath );
			swprintf_s(szALULog, _countof(szALULog), L"%s%s",szLogSourcePath,L"\\WWALUpd.LOG");
			swprintf_s(szRebuildLog64, _countof(szALULog), L"%s%s",szLogSourcePath,L"\\WardwizAntivirus_Rebuild64.log");
			swprintf_s(szRebuildLog32, _countof(szALULog), L"%s%s",szLogSourcePath,L"\\WardwizAntivirus_Rebuild32.log");
	 		
			swprintf_s(szDestALULogPath, _countof(szDestRebuildLog64), L"%s%s",csDestPathLogDir,L"\\WWALUpd.LOG");
			swprintf_s(szDestRebuildLog64, _countof(szDestRebuildLog64), L"%s%s",csDestPathLogDir,L"\\WardwizAntivirus_Rebuild64.log");
			swprintf_s(szDestRebuildLog32, _countof(szDestRebuildLog32), L"%s%s",csDestPathLogDir,L"\\WardwizAntivirus_Rebuild32.log");
	 		
			CopyFile(szALULog, csDestPathLogDir, TRUE);
			CopyFile(szRebuildLog64, szDestRebuildLog64, TRUE);
			CopyFile(szRebuildLog32, szDestRebuildLog32, TRUE);


	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::UploadLogFilesToServer");
	}

	return true;
}

/***************************************************************************
  Function Name  : Thread_AddSignatureToSelectedFiles
  Description    : it use to  add signature to list of selected files
  Author Name    : lalit kumawat 
  Date           : 1-20-2015
  SR_NO			 : 
****************************************************************************/
DWORD WINAPI Thread_AddSignatureToSelectedFiles( LPVOID lpParam )
{
 CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *) lpParam;

  try
	{   
	    int status = 0;
		int iFileCount = 0;
	  	 pWWALUpdDlg->LoadRequiredDLLs();
			 
		while(iFileCount < pWWALUpdDlg->m_iSelectedFileCount)
		{
			status = pWWALUpdDlg->AddSignToSpecifiedFile(pWWALUpdDlg->msSelectedFiles[iFileCount]);
			
			if(status == 1)
			{
				pWWALUpdDlg->m_lstSelectedFileList.SetItemText(iFileCount, 1,L"AlreadySigned" );
			}
			else if(status == 2) 
			{
				pWWALUpdDlg->m_lstSelectedFileList.SetItemText(iFileCount, 1,L"Signed" );
			}
			else if(status == 0)
			{
				pWWALUpdDlg->m_lstSelectedFileList.SetItemText(iFileCount, 1,L"Error" );
			}

			iFileCount++;
			status = false;
		}

		pWWALUpdDlg->m_bIsSigningCompleted  = true;
		
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::OnBnClickedButtonAddsign");
	}

	return true;
}
/***************************************************************************
  Function Name  : OnBnClickedButtonBrowsefiletomd5
  Description    : it use to browse file for MD5.
  Author Name    : lalit kumawat 
  Date           : 1-20-2015
  SR_NO			 : 
****************************************************************************/
void CWardWizALUpdDlg::OnBnClickedButtonBrowsefiletomd5()
{
	// TODO: Add your control notification handler code here
	try
	{
		static TCHAR szEncFilter[] = L"All Files(*.*)|*.*|";   
		CFileDialog FileDlg(TRUE, L"All Files(*.*)", NULL, OFN_ALLOWMULTISELECT, (LPCTSTR)szEncFilter);
		TCHAR commandLine[] = _T("");
		
		if( FileDlg.DoModal() == IDOK )
			{
				mbIsFileSelected = true ;
				CString csFilePath = FileDlg.GetPathName();
				m_stSelectedFilePathForZIP.SetWindowTextW(csFilePath) ;
				msSelectedFileForMD5 = csFilePath;
				m_cedtMd5OfSelectedFile.SetWindowTextW(L"");
			}
			else
			{
				mbIsFileSelected = false ;
			}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::OnBnClickedButtonBrowsefiletomd5");
	}

}

/***************************************************************************
  Function Name  : OnBnClickedButtonGeneratemd5
  Description    : it generate MD5 and make zip of selected file.
  Author Name    : lalit kumawat 
  Date           : 1-20-2015
  SR_NO			 : 
****************************************************************************/
void CWardWizALUpdDlg::OnBnClickedButtonGeneratemd5()
{
	try
	{
	if(mbIsFileSelected)
	{
		TCHAR	szFileName[128] = {0};
		TCHAR	szFilePath[512] = {0};
		TCHAR	szFileHash[128] = {0};
		TCHAR	szFileZipPath[512] = {0};
		CStringA  csZipFilePath = " ";
		CString  csFilePath = L"";

		DWORD	dwFileSize = 0x00;
		DWORD	dwFileZipSize = 0x00;
		CString csMd5OfSelectedFile = L"";
		TCHAR csMd5AndFileSize[512] = {0};

		swprintf_s(szFilePath, _countof(szFilePath), L"%s",msSelectedFileForMD5);
		swprintf_s(szFileZipPath, _countof(szFileZipPath), L"%s%s",msSelectedFileForMD5,L".ZIP");


		 LoadRequiredDLLs();
		 if( GetFileSizeAndHash(szFilePath, dwFileSize, szFileHash) )
			{
				AddLogEntry(L"### Failed in CVibraniumALUpdDlg::GetFileSizeAndHash(%s)", szFilePath );
			}
		
		/*if( MakeZipFile(szFilePath, szFileZipPath) )
			{
				AddLogEntry(L"### Failed in CVibraniumALUpdDlg::MakeZipFile(%s)",szFilePath );
			}
*/
		csZipFilePath = szFileZipPath;
		csFilePath = csZipFilePath;

		if (!zip(szFilePath, csFilePath, csZipFilePath))
		{
			AddLogEntry(L"### Failed in CVibraniumALUpdDlg::zip(%s)", szFilePath);
		}

		GetFileSizeAndHash(szFileZipPath, dwFileZipSize, NULL);

		swprintf_s(csMd5AndFileSize, _countof(csMd5AndFileSize), L"%lu,%lu, %s", dwFileZipSize,dwFileSize,szFileHash);
		csMd5OfSelectedFile = csMd5AndFileSize;

		m_cedtMd5OfSelectedFile.SetWindowTextW(csMd5OfSelectedFile);
			
	}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::OnBnClickedButtonGeneratemd5");
	}
}

/***************************************************************************
  Function Name  : OnVScroll
  Description    : framework trigger this event when scroll bar position change.
  Author Name    : lalit kumawat 
  Date           : 1-20-2015
  SR_NO			 : 
****************************************************************************/
void CWardWizALUpdDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
// TODO: Add your message handler code here and/or call default.
	int iDelta;
	int iMaxPos = m_rect.Height() - m_nCurHeight +90;

	switch (nSBCode)
	{
	case SB_LINEDOWN:
		if (m_nScrollPos >= iMaxPos)
			return;
		iDelta = min(iMaxPos/100,iMaxPos-m_nScrollPos);
		break;

	case SB_LINEUP:
		if (m_nScrollPos <= 0)
			return;
		iDelta = -min(iMaxPos/100,m_nScrollPos);
		break;

         case SB_PAGEDOWN:
		if (m_nScrollPos >= iMaxPos)
			return;
		iDelta = min(iMaxPos/10,iMaxPos-m_nScrollPos);
		break;

	case SB_THUMBPOSITION:
		iDelta = (int)nPos - m_nScrollPos;
		break;

	case SB_PAGEUP:
		if (m_nScrollPos <= 0)
			return;
		iDelta = -min(iMaxPos/10,m_nScrollPos);
		break;
	
         default:
		return;
	}
	m_nScrollPos += iDelta;
	SetScrollPos(SB_VERT,m_nScrollPos,TRUE);
	ScrollWindow(0,-iDelta);
	CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

/***************************************************************************
  Function Name  : OnSize
  Description    : It reinitialize scroll variable if dialog size change.
  Author Name    : lalit kumawat 
  Date           : 1-20-2015
  SR_NO			 : 
****************************************************************************/
void CWardWizALUpdDlg::OnSize(UINT nType, int cx, int cy)
{
	try
	{

		CDialog::OnSize(nType, cx, cy);
		m_nCurHeight = cy;
		int nScrollMax;
		if (cy < m_rect.Height())
		{
			 nScrollMax = m_rect.Height()- cy +100;
		}
		else
		   nScrollMax = 0;

		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_ALL; // SIF_ALL = SIF_PAGE | SIF_RANGE | SIF_POS;
		si.nMin = 0;
		si.nMax = nScrollMax;
		si.nPage = si.nMax/10;
		si.nPos = 0;
		SetScrollInfo(SB_VERT, &si, TRUE);
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::OnSize");
	}
}

/***************************************************************************
  Function Name  : OnMouseWheel
  Description    : It trigger scroll event when mouse wheel event generate.
  Author Name    : lalit kumawat 
  Date           : 1-20-2015
  SR_NO			 : 
****************************************************************************/
BOOL CWardWizALUpdDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	//AfxMessageBox(L"scroll"); 
	BOOL bUp = TRUE;
    int iDelta = zDelta;
    if(zDelta < 0)
    {
        bUp = FALSE;
        iDelta = - iDelta;
    }
    UINT WheelScrollLines;
    // get from the OS the number of wheelscrolllines. win95 doesnt support this call
    ::SystemParametersInfo(SPI_GETWHEELSCROLLLINES,0,&WheelScrollLines,0);
    // scroll a page at a time
    if(WheelScrollLines == 3)//WHEEL_PAGESCROLL)  ,SB_PAGEDOWN
    {
       SendMessage(WM_VSCROLL,MAKEWPARAM((bUp)? SB_PAGEUP:SB_PAGEDOWN,0),0);
    }
    else
    {
        int numlines = (iDelta * WheelScrollLines) / WHEEL_DELTA;
        while ( numlines-- )
            SendMessage(WM_VSCROLL,MAKEWPARAM((bUp)? SB_LINEUP:SB_LINEDOWN,0),0);
    }

	return CDialog::OnMouseWheel(nFlags, zDelta, pt);
}
/***************************************************************************
  Function Name  : OnBnClickedRadioAdd
  Description    : This function is used to Set value for Status of Registry option to Add.
  Author Name    : Niranjan Deshak
  Date           : 09 Feb 2015
  SR_NO			 :
****************************************************************************/
void CWardWizALUpdDlg::OnBnClickedRadioAdd()
{
	m_iRadioROAdd = 0;
}
/***************************************************************************
  Function Name  : OnBnClickedRadioModify
  Description    : This function is used to Set value for Status of Registry option to Modify.
  Author Name    : Niranjan Deshak
  Date           : 09 Feb 2015
  SR_NO			 :
****************************************************************************/
void CWardWizALUpdDlg::OnBnClickedRadioModify()
{
	m_iRadioROAdd = 1;
}
/***************************************************************************
  Function Name  : OnBnClickedRadioDelete
  Description    : This function is used to Set value for Status of Registry option to Delete.
  Author Name    : Niranjan Deshak
  Date           : 09 Feb 2015
  SR_NO			 :
****************************************************************************/
void CWardWizALUpdDlg::OnBnClickedRadioDelete()
{
	m_iRadioROAdd = 2;
}
/***************************************************************************
  Function Name  : OnBnClickedCheckRegopt
  Description    : This function is Used to disable and enable the registry option.
  Author Name    : Niranjan Deshak
  Date           : 09 Feb 2015
  SR_NO			 :
****************************************************************************/
void CWardWizALUpdDlg::OnBnClickedCheckRegopt()
{
	try
	{
		if(m_chkRegOpt.GetCheck()==BST_CHECKED)
		{
			m_btnROAddToList.EnableWindow(true);	
			m_btnRORemoveFromList.EnableWindow(true);
			m_btnAdd.EnableWindow(true);
			GetDlgItem(IDC_RADIO_MODIFY)->EnableWindow(true);
			GetDlgItem(IDC_RADIO_DELETE)->EnableWindow(true);
			m_btnRORegKeyType.EnableWindow(true);
			m_editROvalue.EnableWindow(true);
			m_editROPath.EnableWindow(true);
		}
		else if(m_chkRegOpt.GetCheck()==BST_UNCHECKED)
		{
			m_btnROAddToList.EnableWindow(false);	
			m_btnRORemoveFromList.EnableWindow(false);
			m_btnAdd.EnableWindow(false);
			GetDlgItem(IDC_RADIO_MODIFY)->EnableWindow(false);
			GetDlgItem(IDC_RADIO_DELETE)->EnableWindow(false);
			m_btnRORegKeyType.EnableWindow(false);
			m_editROvalue.EnableWindow(false);
			m_editROPath.EnableWindow(false);
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::OnBnClickedCheckRegopt");
	}
}
/***************************************************************************
  Function Name  : OnBnClickedButtonRoAdd
  Description    : This function is Used to Write the registry option to List View Control on UI,
				   Which are accepted from User. 
  Author Name    : Niranjan Deshak
  Date           : 09 Feb 2015
  SR_NO			 :
****************************************************************************/
void CWardWizALUpdDlg::OnBnClickedButtonRoAdd()
{
	try
	{
		m_csRegOptStatus = L"";
		m_csRORegKeyName = L"";
		ZeroMemory(m_szROPath, sizeof(m_szROPath) );
		ZeroMemory(m_szROValue, sizeof(m_szROValue) );
		//This code is to get add, modify , delete value
		if( m_iRadioROAdd == 0 )
		{	
			m_csRegOptStatus = L"add";
		}
		else if( m_iRadioROAdd == 1 )
		{ 
			m_csRegOptStatus = L"modify";
		}
		else if(m_iRadioROAdd == 2 )
		{
			m_csRegOptStatus = L"del";
		}
		//This code is to select registry Key.
		int selection = m_btnRORegKeyType.GetCurSel();
		if(selection == 0)
		{
			m_csRORegKeyName = L"HKCC";
		}
		else if(selection == 1)
		{
			m_csRORegKeyName = L"HKCR";
		}
		else if(selection == 2)
		{
			m_csRORegKeyName = L"HKCU";
		}
		else if(selection == 3)
		{
			m_csRORegKeyName = L"HKLM";
		}
		else if(selection == 4)
		{
			m_csRORegKeyName = L"HKU";
		}
		//This code is to get path from registry option
		m_editROPath.GetWindowTextW(m_szROPath,MAX_PATH);
		//This code is to et value from registry options
		m_editROvalue.GetWindowTextW(m_szROValue,MAX_PATH);


		if((m_szROPath[0] == _T('\0') )|| (m_szROValue[0] == _T('\0')))
		{
			MessageBox(L"Please, Enter the Path and Value from registry option",L"Vibranium",MB_OK);
		}
		else
		{
			int iCurrentItemCount=m_List_RegOpt.GetItemCount();
			m_List_RegOpt.InsertItem(iCurrentItemCount, m_csRORegKeyName );
			m_List_RegOpt.SetItemText(iCurrentItemCount, 1, m_szROPath );
			m_List_RegOpt.SetItemText(iCurrentItemCount, 2, m_szROValue );
			m_List_RegOpt.SetItemText(iCurrentItemCount, 3, m_csRegOptStatus );

		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::OnBnClickedButtonRoAdd");
	}
}
/***************************************************************************
  Function Name  : OnBnClickedButtonRemove
  Description    : This function is Used to Remove unwanted entry from List View Control from registry option.
  Author Name    : Niranjan Deshak
  Date           : 09 Feb 2015
  SR_NO			 :
****************************************************************************/
void CWardWizALUpdDlg::OnBnClickedButtonRemove()
{
	try
	{
		for (int index = 0; index < m_List_RegOpt.GetItemCount(); index++)
		{
			if (m_List_RegOpt.GetCheck(index))
			{
				m_List_RegOpt.DeleteItem(index);
				OnBnClickedButtonRemove();
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::OnBnClickedButtonRemove", 0, 0, true, SECONDLEVEL);
	}
}
/***************************************************************************
  Function Name  : AddRegOptToListFromWWBinary
  Description    : This function is Used to add registry option entries from WWBinary.ini to List view control.
  Author Name    : Niranjan Deshak
  Date           : 09 Feb 2015
  SR_NO			 :
****************************************************************************/
void CWardWizALUpdDlg::AddRegOptToListFromWWBinary()
{
	try
	{
		CString csROString;
		POSITION pos = m_listRegOpt.GetHeadPosition();
		for (int index = 0; index < m_listRegOpt.GetCount(); index++)
		{
			m_csRegOptStatus = L"";
			m_csRORegKeyName = L"";
			ZeroMemory(m_szROPath, sizeof(m_szROPath));
			ZeroMemory(m_szROValue, sizeof(m_szROValue));
			csROString = m_listRegOpt.GetNext(pos);
			int iTokenPos = 0;
			csROString.Insert(0, _T(","));
			CString csToken = csROString.Tokenize(_T(" , "), iTokenPos);
			m_csRORegKeyName = csToken;

			int iCount = 1;

			while (!csToken.IsEmpty() && (iCount <= 3))
			{
				if (iCount == 1)
				{
					csToken = csROString.Tokenize(_T(" , "), iTokenPos);
					_tcscpy_s(m_szROPath, _countof(m_szROPath), csToken);
					iCount++;
				}
				else if (iCount == 2)
				{
					csToken = csROString.Tokenize(_T(" , "), iTokenPos);
					_tcscpy_s(m_szROValue, _countof(m_szROValue), csToken);
					iCount++;
				}
				else if (iCount == 3)
				{
					csToken = csROString.Tokenize(_T(" , "), iTokenPos);
					m_csRegOptStatus = csToken;
					iCount++;
				}
			}
			int iCurrentItemCount = m_List_RegOpt.GetItemCount();
			m_List_RegOpt.InsertItem(iCurrentItemCount, m_csRORegKeyName);
			m_List_RegOpt.SetItemText(iCurrentItemCount, 1, m_szROPath);
			m_List_RegOpt.SetItemText(iCurrentItemCount, 2, m_szROValue);
			m_List_RegOpt.SetItemText(iCurrentItemCount, 3, m_csRegOptStatus);

		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::AddRegOptToListFromVibraniumBinary", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : ParseVersionString
*  Description    : To parse CString version strings and store it in Array of integers.
*  Author Name    : Niranjan Deshak
*  SR_NO
*  Date           : 16 March 2015
****************************************************************************************************/
bool CWardWizALUpdDlg::ParseVersionString(int iDigits[4], CString& csVersion)
{
	try
	{
		int iTokenPos = 0;
		csVersion.Insert(0, _T("."));
		CString csToken = csVersion.Tokenize(_T(" . "), iTokenPos);
		int iVersion = _ttoi(csToken);
		int iSubVersion = 0;
		int iCount = 0;

		iDigits[iCount] = iVersion;
		iCount++;
		while ((!csToken.IsEmpty()) && (iCount <= 3))
		{
			csToken = csVersion.Tokenize(_T(" . "), iTokenPos);
			iSubVersion = _ttoi(csToken);
			iDigits[iCount] = iSubVersion;
			iCount++;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::ParseVersionString", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

bool CWardWizALUpdDlg::zip(CString csFilePath ,CString csZipFilePath, const char * ptrZipPath)
{
	try
	{
		// Create Zip file
		TCHAR szFilePath[MAX_PATH] = { 0 };
		TCHAR szZipFilePath[MAX_PATH] = { 0 };
		TCHAR szDest[MAX_PATH * 2];
		swprintf_s(szZipFilePath, _countof(szZipFilePath), L"%s\\\0\0", csZipFilePath); // to create buffer for zip file
		swprintf_s(szFilePath, _countof(szFilePath), L"%s\0\0", csFilePath);
		BYTE startBuffer[] = { 80, 75, 5, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		FILE *file = fopen(ptrZipPath, "wb");
		fwrite(startBuffer, sizeof(startBuffer), 1, file);
		fclose(file);

		BSTR source = szFilePath;
		BSTR dest = szZipFilePath;

		HRESULT          hResult;
		IShellDispatch *pISD;
		Folder                *pToFolder = NULL;
		VARIANT          vDir, vFile, vOpt;

		if (!PathFileExists(csFilePath))
		{
			AddLogEntry(L"###vCVibraniumALUpdDlg::zip Path not found");
			return false;
		}
		CoInitialize(NULL);

		hResult = CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void **)&pISD);

		//hResult = CoGetClassObject(CLSID_Shell, CLSCTX_INPROC_SERVER, NULL, IID_IClassFactory, (void **)&pISD);
		//hResult = pISD->
		if (hResult != S_OK)
			return false;

		if (hResult == S_OK)
		{
			VariantInit(&vDir);
			vDir.vt = VT_BSTR;
			vDir.bstrVal = dest;//L"C:\\test.zip\\\0\0";
			hResult = pISD->NameSpace(vDir, &pToFolder);

			if (hResult != S_OK)
				return false;

			if (hResult == S_OK)
			{
				// Now copy source file(s) to the zip
				VariantInit(&vFile);
				vFile.vt = VT_BSTR;
				vFile.bstrVal = source;//L"C:\\test.txt";
				// ******NOTE**** To copy multiple files into the zip, need to create a FolderItems object (see unzip implementation below for more details)

				VariantInit(&vOpt);
				vOpt.vt = VT_I4;
				vOpt.lVal = FOF_NO_UI;//4;          // Do not display a progress dialog box, not useful in our example

				// Copying and compressing the source files to our zip
				hResult = pToFolder->CopyHere(vFile, vOpt);

				/* CopyHere() creates a separate thread to copy files and it may happen that the main thread exits
				* before the copy thread is initialized. So we put the main thread to sleep for a second to
				* give time for the copy thread to start.*/
				Sleep(1000);
				pToFolder->Release();
			}
			pISD->Release();
		}
		CoUninitialize();
	}
	catch (...)
	{
		AddLogEntry(L"###Exception in creating zip file.");
	}
	
	return true;
}

bool CWardWizALUpdDlg::RarFile(CString csFilePath, CString csRarFilePath, const char * ptrRarFilePath)
{
	bool bReturn = false;



	return bReturn;
}

bool CWardWizALUpdDlg::GetInnoAndAppPath(CString &csInnoPath, CString &csISSPath)
{
	TCHAR szProgramDirX86[MAX_PATH] = { 0 };
	TCHAR szInnoPath[MAX_PATH] = { 0 };
	TCHAR	szApplPath[MAX_PATH];

	GetEnvironmentVariable(TEXT("PROGRAMFILES(X86)"), szProgramDirX86, 255 );
	if( !PathFileExists( szProgramDirX86 ) )
	{

	GetEnvironmentVariable(TEXT("PROGRAMFILES"), szProgramDirX86, 255 );

	}

	_wcsupr_s( szProgramDirX86, wcslen(szProgramDirX86)*sizeof(TCHAR) );
	swprintf_s(	szInnoPath, _countof(szInnoPath),L"%s\\Inno Setup 6\\ISCC.exe ",szProgramDirX86);

	
	GetModuleFileName(NULL, szApplPath, 511);
	TCHAR	*pTemp = wcsrchr(szApplPath, '\\');
	if( !pTemp )
	{
	AddLogEntry(L"### Failed getting application path CVibraniumALUpdDlg::Thread_GenerateUpdateFiles");
	return false;
	}
	*pTemp = '\0';
	if(!PathFileExists(szInnoPath))
	{
	return false;
	}

	csInnoPath = szInnoPath;
	csISSPath = szApplPath;

	return true;
}

DWORD CWardWizALUpdDlg::WriteKeyValueToIni(TCHAR *pKey, TCHAR *pValue, TCHAR *pData, TCHAR *WWpFileName, TCHAR *WWizpFileName)
{
	TCHAR szEntryCont[MAX_PATH] = { 0 };
	CString sz_Count = L"";
	int iCount = 0;
	DWORD dwRetValue = 0;
	try
	{
		GetPrivateProfileString(pKey, L"Count", L"", szEntryCont, 255, WWpFileName);
		iCount = _ttoi(szEntryCont);
		iCount++;
		sz_Count.Format(L"%d", iCount);

		if (WWpFileName != NULL)
		{
			dwRetValue = WritePrivateProfileString(pKey, L"Count", sz_Count, WWpFileName);
			if (!dwRetValue)
			{
				AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
				return dwRetValue;
			}
			dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, WWpFileName);
			if (!dwRetValue)
			{
				AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
				return dwRetValue;
			}
		}

		if (WWizpFileName != NULL)
		{
			dwRetValue = WritePrivateProfileString(pKey, L"Count", sz_Count, WWizpFileName);
			if (!dwRetValue)
			{
				AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
				return dwRetValue;
			}

			dwRetValue = WritePrivateProfileString(pKey, sz_Count, pData, WWizpFileName);
			if (!dwRetValue)
			{
				AddLogEntry(L"### Failed in CVibraniumALUpdDlg::WritePrivateProfileString(%s)", pData);
				return dwRetValue;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"###Exception in WriteKeyValueToIni.");
	}
	
	return 0x00;
}

DWORD CWardWizALUpdDlg :: GetKeyValueFromIni(TCHAR *pkeyName)
{
	try
	{
		TCHAR	szModulePath[MAX_PATH] = { 0 };
		TCHAR	szIniPath[MAX_PATH] = { 0 };
		TCHAR	szCount[255] = { 0 };
		DWORD	dwCount = 0;

		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			AddLogEntry(L">>> Error in GetModulePath in ParseIntegrityInfoINIAndRollBackCRC", 0, 0, true, SECONDLEVEL);
			return 0x01;
		}

		swprintf_s(szIniPath, _countof(szIniPath), L"%s\\WWBinary.ini", szModulePath);

		if (!PathFileExists(szIniPath))
		{
			AddLogEntry(L">>> Error in GetModulePath in ParseIntegrityInfoINIAndRollBackCRC", 0, 0, true, SECONDLEVEL);
			return 0x01;
		}
		GetPrivateProfileString(pkeyName, L"COUNT", L"", szCount, 255, szIniPath);

		swscanf(szCount, L"%lu", &dwCount);
		TCHAR szValueName[255] = { 0 };
		TCHAR szValueData[MAX_PATH] = { 0 };

		for (int index = 1; index <= dwCount; index++)
		{

			swprintf_s(szValueName, _countof(szValueName), L"%lu", index);
			GetPrivateProfileString(pkeyName, szValueName, L"", szValueData, 2 * MAX_PATH, szIniPath);
			WriteToIni(pkeyName, szValueName, szValueData, NULL);

		}
	}
	catch (...)
	{
		AddLogEntry(L"###Exception in GetKeyValueFromIni.");
	}

	return 0x00;
}

bool CWardWizALUpdDlg :: IsFileSetupDllFile(CString csFilePath)
{
	CString  csFileName = L"";
	TCHAR	szTemp[1024] = { 0 };
	int iPos = 0;// csFilePath.ReverseFind('\\');
	//csFileName = csFilePath.Mid(csFilePath.ReverseFind('\\') + 1);
	
	CString csInnoPath = L"";
	CString csISSPath = L"";

	CString smodulePath;
	TCHAR szModulePath[MAX_PATH] = { 0 };

	if (!GetModulePath(szModulePath, MAX_PATH))
	{
		//MessageBox(NULL, L"Error in GetModulePath",  L"Vibranium", MB_ICONERROR);
		return false;
	}
	smodulePath = szModulePath;
	smodulePath.Append(L"\\Release\\x64\\Binaries\\VBSETUPDLL.DLL");

	iPos = csFilePath.Find(L"\\Win32\\Binaries\\VBSETUPDLL.DLL");
	if (!GetInnoAndAppPath(csInnoPath, csISSPath))
	{
		return false;
	}

	if (iPos > 0)
	{
		if (IsFileHaveSignature(csFilePath))
		{	
			if (!CopyFile(csFilePath, smodulePath, FALSE))
			{
				AddLogEntry(L"###copy file for VBSETUPDLL.DLL failed");
			}
			




				/*if (PathFileExists(L"D:\\WardWizDevRightHeur\\Release\\Win32\\Binaries\\VBSETUPDLL.DLL"))
				{
					AddSignToSpecifiedFile(L"D:\\WardWizDevRightHeur\\Release\\Win32\\Binaries\\VBSETUPDLL.DLL");
				}*/
				swprintf_s(m_szStatusText, _countof(m_szStatusText), L"Basic inno setup rebuilding + WardwizUtility_32.iss");
				swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizUtility_32.iss\"", csInnoPath, csISSPath);
				if (RebuildSolution(szTemp))
					return false;

				swprintf_s(m_szStatusText, _countof(m_szStatusText), L"TS inno setup rebuilding + WardwizUtility_64.iss");
				swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizUtility_64.iss\"", csInnoPath, csISSPath);
				if (RebuildSolution(szTemp))
					return false;
		}
		else
		{
			if (m_chkUtility.GetCheck() == BST_CHECKED)
			{
				if (m_Rebuild32.GetCheck() == BST_CHECKED)
				{
					swprintf_s(m_szStatusText, _countof(m_szStatusText), L"Basic inno setup rebuilding + WardwizUtility_32.iss");
					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizUtility_32.iss\"", csInnoPath, csISSPath);
					if (RebuildSolution(szTemp))
						return false;
				}
				if (m_Rebuild64.GetCheck() == BST_CHECKED)
				{
					swprintf_s(m_szStatusText, _countof(m_szStatusText), L"TS inno setup rebuilding + WardwizUtility_64.iss");
					swprintf_s(szTemp, _countof(szTemp), L"\"%s\" \"%s\\SetupRequirements\\WardwizUtility_64.iss\"", csInnoPath, csISSPath);
					if (RebuildSolution(szTemp))
						return false;
				}
			}
			return false;
		}

	}
	return true;
}

/***************************************************************************
Function Name  : CreateZipForDrivers32
Description    : This function is called to make zip of 32bit Binaries
Author Name    : Nitin
Date           : 18 August 2014
SR_NO			 : SR.N0 ALUPD_0029
****************************************************************************/
void CWardWizALUpdDlg::CreateZipForDrivers32(LPVOID lpParam)
{
	TCHAR	szSour[512] = { 0 };
	TCHAR	szDest[512] = { 0 };
	DWORD	dwRet = 0x00;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *)lpParam;
	pWWALUpdDlg->m_FilesCount = 0x01;

	swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files for Drivers 32 bit Binaries");
	AddLogEntry(L">>>CVibraniumALUpdDlg :: CreateZipForDrivers32: Creating Zip files for drivers 32 bit Binaries");
	if (pWWALUpdDlg->m_PatchType_Product.GetCheck() == BST_CHECKED &&  pWWALUpdDlg->m_Rebuild64.GetCheck() == BST_CHECKED)
	{
		pWWALUpdDlg->m_FilesCount = 0x01;
		swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\RequiredFiles\\PROTECTION\\32", pWWALUpdDlg->m_szApplPath);
		swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\32", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
		pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Files_32", L"AppFolder\\DRIVERS");
		if (pWWALUpdDlg->m_FilesCount > 0x01)
		{
			ZeroMemory(szSour, sizeof(szSour));
			swprintf_s(szSour, _countof(szSour), L"%lu", (pWWALUpdDlg->m_FilesCount - 1));
			//	pWWALUpdDlg->WriteToIni(L"Count_64", L"Count", szSour,NULL);
		}
		else
		{
			AddLogEntry(L"### Failed in CVibraniumALUpdDlg::Thread_GenerateUpdateFiles for 64 bit file count");
			dwRet = 3;
		}
	}
}


/***************************************************************************
Function Name  : CreateZipForDrivers64
Description    : This function is called to make zip of 64bit Binaries
Author Name    : Nitin
Date           : 18 August 2014
SR_NO			 : SR.N0 ALUPD_0029
****************************************************************************/
void CWardWizALUpdDlg::CreateZipForDrivers64(LPVOID lpParam)
{
	TCHAR	szSour[512] = { 0 };
	TCHAR	szDest[512] = { 0 };
	DWORD	dwRet = 0x00;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *)lpParam;
	//D:\WardWiz_Developement\Release\Win32\Binaries
	pWWALUpdDlg->m_FilesCount = 0x01;

	swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files for Drivers 64 bit Binaries");
	AddLogEntry(L">>>CVibraniumALUpdDlg :: CreateZipForDrivers64: Creating Zip files for Drivers 64 bit Binaries");
	if (pWWALUpdDlg->m_PatchType_Product.GetCheck() == BST_CHECKED &&  pWWALUpdDlg->m_Rebuild64.GetCheck() == BST_CHECKED)
	{
		pWWALUpdDlg->m_FilesCount = 0x01;
		swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\RequiredFiles\\PROTECTION\\64", pWWALUpdDlg->m_szApplPath);
		swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\64", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
		pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Files_64", L"AppFolder\\DRIVERS");
		if (pWWALUpdDlg->m_FilesCount > 0x01)
		{
			ZeroMemory(szSour, sizeof(szSour));
			swprintf_s(szSour, _countof(szSour), L"%lu", (pWWALUpdDlg->m_FilesCount - 1));
			//	pWWALUpdDlg->WriteToIni(L"Count_64", L"Count", szSour,NULL);
		}
		else
		{
			AddLogEntry(L"### Failed in CVibraniumALUpdDlg::Thread_GenerateUpdateFiles for 64 bit file count");
			dwRet = 3;
		}
	}
}

/***************************************************************************
Function Name  : CreateZipForRarFiles
Description    : This function is called to make zip of Rar Files
Author Name    : Nitin
Date           : 18 Feb 2016
SR_NO		   : 
****************************************************************************/
void CWardWizALUpdDlg::CreateZipForRarFiles(LPVOID lpParam)
{
	TCHAR	szSour[512] = { 0 };
	TCHAR	szDest[512] = { 0 };
	DWORD	dwRet = 0x00;
	CWardWizALUpdDlg	*pWWALUpdDlg = (CWardWizALUpdDlg *)lpParam;
	pWWALUpdDlg->m_FilesCount = 0x01;

	swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files for 64 bit Rar files");
	AddLogEntry(L">>>CVibraniumALUpdDlg :: CreateZipForRarFiles: Creating Zip files for Rar files");
	pWWALUpdDlg->m_FilesCount = 0x01;
	//D:\WardWiz_Developement\SetupRequirements\RequiredFiles
	swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\RequiredFiles\\UNRAR\\x64", pWWALUpdDlg->m_szApplPath);
	swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\64", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Files_64", L"AppFolder");


	swprintf_s(pWWALUpdDlg->m_szStatusText, _countof(pWWALUpdDlg->m_szStatusText), L"Creating Zip files for 32 bit Rar files");

	swprintf_s(szSour, _countof(szSour), L"%s\\SetupRequirements\\RequiredFiles\\UNRAR\\x86", pWWALUpdDlg->m_szApplPath);
	swprintf_s(szDest, _countof(szDest), L"%s\\ALUpdFiles\\%s\\32", pWWALUpdDlg->m_szApplPath, pWWALUpdDlg->m_szOutDirName);
	pWWALUpdDlg->EnumAndMakeZipFiles(szSour, szDest, L"Files_32", L"AppFolder");

	if (pWWALUpdDlg->m_PatchType_Virus.GetCheck() == BST_CHECKED)
	{
		if (pWWALUpdDlg->m_FilesCount > 0x01)
		{
			ZeroMemory(szSour, sizeof(szSour));
			swprintf_s(szSour, _countof(szSour), L"%lu", (pWWALUpdDlg->m_FilesCount - 1));
			//pWWALUpdDlg->WriteToIni(L"Common", L"Count", szSour, NULL);

			dwRet = 0;
		}
		else
		{
			AddLogEntry(L"### Failed in CVibraniumALUpdDlg::CreateZipForRarFiles for common files count1");
			dwRet = 7;
		}
	}
}

/**********************************************************************************************************
*  Function Name  :	InsertDataToTable
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into
					SQLite tables.
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
//void InsertDataToTable(const char* szQuery)
//{
//	AddLogEntry(L"InsertDataToTable in VibraniumAutoScnDlg- AutoScanner entered", 0, 0, true, ZEROLEVEL);
//	try
//	{
//		CWardWizSQLiteDatabase objSqlDb(g_strDatabaseFilePath);
//
//		objSqlDb.Open();
//
//		int iRows = objSqlDb.ExecDML(szQuery);
//
//		objSqlDb.Close();
//
//		return;
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::InsertDataToTable");
//	}
//}

/**********************************************************************************************************
*  Function Name  :	EnterUpdateDetails
*  Description    :	Creates query required to insert Update related information into SQLite database
*  Author Name    : Gayatri A.
*  Date           : 13 Aug 2016
*  SR_NO		  :
**********************************************************************************************************/
//void EnterUpdateDetails(int iFileCount,int iFileSize)
//{
//	try
//	{
//		CString csInsertQuery = _T("INSERT INTO Wardwiz_UpdatesMaster VALUES (null,");
//		csInsertQuery.Format(_T("INSERT INTO Wardwiz_UpdatesMaster VALUES(null,datetime('now','localtime'), datetime('now','localtime'),date('now'),date('now'),%d,%d);"), iFileCount, iFileSize);
//
//		CT2A ascii(csInsertQuery, CP_UTF8);
//
//		CWardWizSQLiteDatabase objSqlDb(g_strDatabaseFilePath);
//
//		objSqlDb.Open();
//		objSqlDb.CreateWardwizSQLiteTables();
//		objSqlDb.Close();
//
//		InsertDataToTable(ascii.m_psz);
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CVibraniumALUpdDlg::EnterUpdateDetails");
//	}
//}

/***************************************************************************
Function Name  : CreateAndCopyFilesforBasicEssPro
Description    : This function is called copy files on server
Author Name    : Nitin
Date           : 2 March 2016
SR_NO		   :
****************************************************************************/
bool CWardWizALUpdDlg::CreateAndCopyFilesforBasicEssPro(CString csDestPathSetupDir, CString m_szPatchFolderPath)
{
	TCHAR		szFileName[128] = { 0 };
	TCHAR		szFilePath[512] = { 0 };
	TCHAR		csDestPathTemp[512] = { 0 };
	TCHAR		szTemp[512] = { 0 };
	CFileFind	finder;
	bool		flag = false;

	CreateFolderStructureForSetups(csDestPathSetupDir);

	swprintf_s(csDestPathTemp, _countof(csDestPathTemp), csDestPathSetupDir);

	swprintf_s(szTemp, _countof(szTemp), L"%s\\*.*", m_szPatchFolderPath);
	BOOL bWorking = finder.FindFile(szTemp);

	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		// skip . and .. files; otherwise, we'd 
		// recur infinitely! 

		if (finder.IsDots())
			continue;

		if (finder.IsDirectory())
		{
			continue;
		}
		CString csFileName = finder.GetFileName();
		if (csFileName.Find(L"WardWizBasicPatch") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirBasicOffline);
		}
		else if (csFileName.Find(L"WardWizBasicSetupNC") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirBasicNoClam);
		}
		else if (csFileName.Find(L"WardWizBasicPDB") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirBasic);
		}
		else if (csFileName.Find(L"WardWizBasicSetup") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirBasicClam);
		}
		else if (csFileName.Find(L"WRDWIZBASICPRESCN") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirBasicNoClam);
		}


		if (csFileName.Find(L"WardWizEssentialPatch") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirEssentialOffline);
		}
		else if (csFileName.Find(L"WardWizEssentialSetupNC") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirEssentialNoClam);
		}
		else if (csFileName.Find(L"WardWizEssentialPDB") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirEssential);
		}
		else if (csFileName.Find(L"WardWizEssentialSetup") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirEssentialClam);
		}
		else if (csFileName.Find(L"WRDWIZESSENTIALPRESCN") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirEssentialNoClam);
		}


		if (csFileName.Find(L"WardWizProPatch") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirProOffline);
		}
		else if (csFileName.Find(L"WardWizProSetupNC") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirProNoClam);
		}
		else if (csFileName.Find(L"WardWizProPDB") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirPro);
		}
		else if (csFileName.Find(L"WardWizProSetup") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirProClam);
		}
		else if (csFileName.Find(L"WRDWIZPROPRESCN") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirProNoClam);
		}
		
		if (csFileName.Find(L"WardWizElitePatch") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirEliteOffline);
		}
		else if (csFileName.Find(L"WARDWIZEPSCLIENT") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirEliteNoClam);
		}
		else if (csFileName.Find(L"WardWizElitePDB") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirElite);
		}
		else if (csFileName.Find(L"WardWizEliteSetup") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirEliteClam);
		}

		if (csFileName.Find(L"WardWizEssPlusPatch") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirEssentialPlusOffline);
		}
		else if (csFileName.Find(L"WardWizEssPlusSetupNC") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirEssentialPlusNoClam);
		}
		else if (csFileName.Find(L"WardWizEssPlusPDB") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirEssentialPlus);
		}
		else if (csFileName.Find(L"WardWizEssPlusSetup") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirEssentialPlusClam);
		}
		else if (csFileName.Find(L"WRDWIZESSPLUSPRESCN") >= 0)
		{
			ZeroMemory(csDestPathTemp, sizeof(csDestPathTemp));
			swprintf_s(csDestPathTemp, _countof(szTemp), L"%s", m_csDestPathSetupDirEssentialPlusNoClam);
		}

		wcscpy(szFilePath, finder.GetFilePath());

		ZeroMemory(szTemp, sizeof(szTemp));
		swprintf_s(szTemp, _countof(szTemp), L"%s\\%s", csDestPathTemp, csFileName);
		flag = CopyFile(szFilePath, szTemp, TRUE);
		if (!flag)
		{
			AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to copy file: %s", szFilePath);
		}

		//Not required in setup utility
		//EnterUpdateDetails(_countof(szTemp), sizeof(szTemp));
	}
	finder.Close();
	return true;
}

/***************************************************************************
Function Name  : CreateFolderStructureForSetups
Description    : This function is called copy files on server
Author Name    : Nitin
Date           : 2 March 2016
SR_NO		   :
****************************************************************************/
bool  CWardWizALUpdDlg::CreateFolderStructureForSetups(CString csDestPathSetupDir)
{
	bool		flag = false;

	ZeroMemory(m_csDestPathSetupDirBasic, sizeof(m_csDestPathSetupDirBasic));
	swprintf_s(m_csDestPathSetupDirBasic, _countof(m_csDestPathSetupDirBasic), L"%s\\WardWizBasic", csDestPathSetupDir);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirBasic, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}
	ZeroMemory(m_csDestPathSetupDirBasicClam, sizeof(m_csDestPathSetupDirBasicClam));
	swprintf_s(m_csDestPathSetupDirBasicClam, _countof(m_csDestPathSetupDirBasicClam), L"%s\\Clam", m_csDestPathSetupDirBasic);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirBasicClam, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}
	ZeroMemory(m_csDestPathSetupDirBasicNoClam, sizeof(m_csDestPathSetupDirBasicNoClam));
	swprintf_s(m_csDestPathSetupDirBasicNoClam, _countof(m_csDestPathSetupDirBasicNoClam), L"%s\\NoClam", m_csDestPathSetupDirBasic);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirBasicNoClam, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}

	ZeroMemory(m_csDestPathSetupDirBasicOffline, sizeof(m_csDestPathSetupDirBasicOffline));
	swprintf_s(m_csDestPathSetupDirBasicOffline, _countof(m_csDestPathSetupDirBasicOffline), L"%s\\Offline", m_csDestPathSetupDirBasic);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirBasicOffline, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}

	ZeroMemory(m_csDestPathSetupDirEssential, sizeof(m_csDestPathSetupDirEssential));
	swprintf_s(m_csDestPathSetupDirEssential, _countof(m_csDestPathSetupDirEssential), L"%s\\WardWizEssential", csDestPathSetupDir);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirEssential, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}
	ZeroMemory(m_csDestPathSetupDirEssentialClam, sizeof(m_csDestPathSetupDirEssentialClam));
	swprintf_s(m_csDestPathSetupDirEssentialClam, _countof(m_csDestPathSetupDirEssentialClam), L"%s\\Clam", m_csDestPathSetupDirEssential);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirEssentialClam, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}
	ZeroMemory(m_csDestPathSetupDirEssentialNoClam, sizeof(m_csDestPathSetupDirEssentialNoClam));
	swprintf_s(m_csDestPathSetupDirEssentialNoClam, _countof(m_csDestPathSetupDirEssentialNoClam), L"%s\\NoClam", m_csDestPathSetupDirEssential);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirEssentialNoClam, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}

	ZeroMemory(m_csDestPathSetupDirEssentialOffline, sizeof(m_csDestPathSetupDirEssentialOffline));
	swprintf_s(m_csDestPathSetupDirEssentialOffline, _countof(m_csDestPathSetupDirEssentialOffline), L"%s\\Offline", m_csDestPathSetupDirEssential);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirEssentialOffline, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}

	ZeroMemory(m_csDestPathSetupDirPro, sizeof(m_csDestPathSetupDirPro));
	swprintf_s(m_csDestPathSetupDirPro, _countof(m_csDestPathSetupDirPro), L"%s\\WardWizPro", csDestPathSetupDir);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirPro, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}
	ZeroMemory(m_csDestPathSetupDirProClam, sizeof(m_csDestPathSetupDirProClam));
	swprintf_s(m_csDestPathSetupDirProClam, _countof(m_csDestPathSetupDirProClam), L"%s\\Clam", m_csDestPathSetupDirPro);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirProClam, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}
	ZeroMemory(m_csDestPathSetupDirProNoClam, sizeof(m_csDestPathSetupDirProNoClam));
	swprintf_s(m_csDestPathSetupDirProNoClam, _countof(m_csDestPathSetupDirProNoClam), L"%s\\NoClam", m_csDestPathSetupDirPro);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirProNoClam, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}

	ZeroMemory(m_csDestPathSetupDirProOffline, sizeof(m_csDestPathSetupDirProOffline));
	swprintf_s(m_csDestPathSetupDirProOffline, _countof(m_csDestPathSetupDirProOffline), L"%s\\Offline", m_csDestPathSetupDirPro);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirProOffline, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}

	ZeroMemory(m_csDestPathSetupDirElite, sizeof(m_csDestPathSetupDirElite));
	swprintf_s(m_csDestPathSetupDirElite, _countof(m_csDestPathSetupDirElite), L"%s\\WardWizElite", csDestPathSetupDir);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirElite, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}
	ZeroMemory(m_csDestPathSetupDirEliteClam, sizeof(m_csDestPathSetupDirEliteClam));
	swprintf_s(m_csDestPathSetupDirEliteClam, _countof(m_csDestPathSetupDirEliteClam), L"%s\\Clam", m_csDestPathSetupDirElite);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirEliteClam, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}
	ZeroMemory(m_csDestPathSetupDirEliteNoClam, sizeof(m_csDestPathSetupDirEliteNoClam));
	swprintf_s(m_csDestPathSetupDirEliteNoClam, _countof(m_csDestPathSetupDirEliteNoClam), L"%s\\NoClam", m_csDestPathSetupDirElite);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirEliteNoClam, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}

	ZeroMemory(m_csDestPathSetupDirEliteOffline, sizeof(m_csDestPathSetupDirEliteOffline));
	swprintf_s(m_csDestPathSetupDirEliteOffline, _countof(m_csDestPathSetupDirEliteOffline), L"%s\\Offline", m_csDestPathSetupDirElite);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirEliteOffline, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}

	ZeroMemory(m_csDestPathSetupDirEssentialPlus, sizeof(m_csDestPathSetupDirEssentialPlus));
	swprintf_s(m_csDestPathSetupDirEssentialPlus, _countof(m_csDestPathSetupDirEssentialPlus), L"%s\\WardWizEssentialPlus", csDestPathSetupDir);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirEssentialPlus, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}
	ZeroMemory(m_csDestPathSetupDirEssentialPlusClam, sizeof(m_csDestPathSetupDirEssentialPlusClam));
	swprintf_s(m_csDestPathSetupDirEssentialPlusClam, _countof(m_csDestPathSetupDirEssentialPlusClam), L"%s\\Clam", m_csDestPathSetupDirEssentialPlus);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirEssentialPlusClam, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}
	ZeroMemory(m_csDestPathSetupDirEssentialPlusNoClam, sizeof(m_csDestPathSetupDirEssentialPlusNoClam));
	swprintf_s(m_csDestPathSetupDirEssentialPlusNoClam, _countof(m_csDestPathSetupDirEssentialPlusNoClam), L"%s\\NoClam", m_csDestPathSetupDirEssentialPlus);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirEssentialPlusNoClam, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}

	ZeroMemory(m_csDestPathSetupDirEssentialPlusOffline, sizeof(m_csDestPathSetupDirEssentialPlusOffline));
	swprintf_s(m_csDestPathSetupDirEssentialPlusOffline, _countof(m_csDestPathSetupDirEssentialPlusOffline), L"%s\\Offline", m_csDestPathSetupDirEssentialPlus);
	flag = true;
	flag = CreateDirectory(m_csDestPathSetupDirEssentialPlusOffline, NULL);
	if (!flag)
	{
		AddLogEntry(L"### Failed CVibraniumALUpdDlg::Failed to create setup directory on server");
		m_bIsSetupCreated = true;
		return false;
	}
	return true;
}
