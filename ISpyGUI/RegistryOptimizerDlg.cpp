/*******************************************************************************
*  Program Name: RegistryOptimizerDlg.cpp                                                                                                    
*  Description: Scans & repairs specific registry areas                                                                                                           
*  Author Name: Vilas & Prajakta                                                                                                      
*  Date Of Creation: 16 Nov 2013
*  Version No: 1.0.0.2
*******************************************************************************/

/****************************************************
					HEADER FILES
****************************************************/
#include "stdafx.h"
#include "ISpyGUI.h"
#include "ISpyGUIDlg.h"
#include "ISpyToolsDlg.h"
#include "RegistryOptimizerDlg.h"

#define TIMER_REGOPT_STATISTICS			1000

CISpyGUIDlg *g_TabCtrlRrgOptWindHandle = NULL;
/*
typedef DWORD	(*SHOWTOTALENTRIES)	() ;
typedef int		(*SCANANDREPAIRPERCENTAGE)() ;
typedef  DWORD	(*SCANANDREPAIRREGISTRYENTRIES)	(LPREGOPTSCANOPTIONS ) ;

SHOWTOTALENTRIES				ShowTotalEntries				= NULL ;
SCANANDREPAIRPERCENTAGE			ScanAndRepairPercentage			= NULL ;
SCANANDREPAIRREGISTRYENTRIES	ScanAndRepairRegistryEntries	= NULL ;
*/

//DWORD ScanAndRepairRegistryEntries(LPREGOPTSCANOPTIONS pScanOpt ) ;
//DWORD ShowTotalEntries() ;
//int ScanAndRepairPercentage( ) ;

//Declarations
//bool	bVistaOnward ;
//BOOL	m_bIsWow64 ;
//PVOID	OldValue ;
//
//TCHAR	szWindowsDir[256] ;
//TCHAR	szSystemDir[256] ;
//TCHAR	szProgramDir[256] ;
//TCHAR	szProgramDirX86[256] ;
//TCHAR	szApplPath[512] ;
//TCHAR	szAppDataPath[512] ;
//TCHAR	szCommProgram[512] ;
//TCHAR	szProgramData[256] ;
//TCHAR	szUserProfile[256] ;
//TCHAR	szTempLocal[256] ;
//TCHAR	szPublic[256] ;
//TCHAR	szAppData[256] ;
//
//int		dwPercentage ;
//DWORD	dwScannedBit ;
//
//DWORD	dwActiveXEntries ;
//DWORD	dwUnInstallEntries ;
//DWORD	dwFontEntries ;
//DWORD	dwSharedDLLs ;
//DWORD	dwAppPathEntries ;
//DWORD	dwHelpFilesEntries ;
//DWORD	dwStartupRepairedEntries ;
//DWORD	dwServicesEntries ;
//DWORD	dwExtensionEntries ;
//DWORD	dwRootKitEntries ;
//DWORD	dwRogueEntries ;
//DWORD	dwWormEntries ;
//DWORD	dwSpywareEntries ;
//DWORD	dwAdwareEntries ;
//DWORD	dwKeyLoggerEntries ;
//DWORD	dwBHOEntries ;
//DWORD	dwExplorerEntries ;
//DWORD	dwIEEntries ;
//
//void InitializeVariables() ;
//void FreeUsedResources() ;
//void IsWow64() ;
//
//
//DWORD CheckInvalidActiveXEntries() ;
//	DWORD CheckInvalidUninstallEntries( ) ;
//	DWORD CheckInvalidFontEntries( ) ;
//	DWORD CheckInvalidSharedLibraries( ) ;
//	DWORD CheckInvalidApplicationPaths( ) ;
//	DWORD CheckInvalidHelpFile( ) ;
//	DWORD CheckInvalidStartupEntries( ) ;
//	DWORD CheckInvalidServices( ) ;
//	DWORD CheckInvalidExtensions( ) ;
//	DWORD CheckRootKitEntries( ) ;
//	DWORD CheckRogueApplications( ) ;
//	DWORD CheckWormEntries( ) ;
//	DWORD CheckInvalidSpywares() ;
//	DWORD CheckInvalidAdwares() ;
//	DWORD CheckKeyLogger() ;
//	DWORD CheckInvalidBHO() ;
//	DWORD CheckInvalidExplorer() ;
//	DWORD CheckInvalidIExplorer() ;
//
//
//	DWORD CheckShellExecuteHooks(TCHAR *pSubKey, DWORD &dwEntries, HKEY hPredKey = HKEY_LOCAL_MACHINE ) ;
//
//
//	DWORD EnumRegValueDataForPathExist(TCHAR *pSubKey, TCHAR *, DWORD &dwEntries, HKEY hPredKey = HKEY_LOCAL_MACHINE ) ;
//	DWORD CheckForPathExists(TCHAR *pPath ) ;
//	DWORD CheckServiceValidPath( HKEY hSubKey, TCHAR *pPath ) ;
//	DWORD CheckValidExplorerExtension( HKEY hSubKey, TCHAR *pSubKey ) ;
//	DWORD EnumRegValueNameForPathExist(TCHAR *pSubKey, TCHAR *pEntryName, DWORD &dwEntries, HKEY hPredKey = HKEY_LOCAL_MACHINE );
//	DWORD EnumRegValueNameDeleteAll(TCHAR *pSubKey, TCHAR *pEntryName, DWORD &dwEntries, HKEY hPredKey= HKEY_LOCAL_MACHINE );
//	DWORD EnumRegValueNameForDeletion(TCHAR *pSubKey, TCHAR *pTypeInfo, DWORD &dwEntries, HKEY hPredKey= HKEY_LOCAL_MACHINE );
//	DWORD SetRegSZValue(TCHAR *pSubKey, TCHAR *pValueName, TCHAR *pValueData, bool, HKEY hPredKey= HKEY_LOCAL_MACHINE );
//
//	BOOL DeleteInvalidKey(HKEY hKeyRoot, LPTSTR ) ;
//	BOOL RegDelnodeRecurse(HKEY hKeyRoot, LPTSTR lpSubKey ) ;
//
//	void AddToLog(TCHAR *pText ) ;
//
//
//
DWORD WINAPI ScanRepairRegistryEntriesThread(LPVOID lpParam);

// CRegistryOptimizerDlg dialog

IMPLEMENT_DYNAMIC(CRegistryOptimizerDlg, CDialog)

/**********************************************************************************************************                     
*  Function Name  :	CRegistryOptimizerDlg                                                     
*  Description    :	C'tor
*  Author Name    : Vilas & Prajakta 
*  SR_NO		  :
*  Date           : 16 Nov 2013
**********************************************************************************************************/
CRegistryOptimizerDlg::CRegistryOptimizerDlg(CWnd* pParent /*=NULL*/)
	: CJpegDialog(CRegistryOptimizerDlg::IDD, pParent)
	, m_bScanStarted(false)
	, m_bScanStop(false)
	, m_bClose(false)
	, m_bRegOptHome(false)
	, m_dwTotalRepairedEntries(0)
	, m_objiTinServerMemMap_Client(REGISTRYOPTIMIZER)
	, m_bScanCompleted(false)
	, m_hRegOptThread(NULL)
	, m_bIsPopUpDisplayed(false)
{

}

/**********************************************************************************************************                     
*  Function Name  :	~CRegistryOptimizerDlg                                                     
*  Description    :	Destructor
*  Author Name    : Vilas & Prajakta                                                                                          
*  SR_NO		  :
*  Date           : 16 Nov 2013
**********************************************************************************************************/

CRegistryOptimizerDlg::~CRegistryOptimizerDlg()
{
}

/**********************************************************************************************************                     
*  Function Name  :	DoDataExchange                                                     
*  Description    :	Called by the framework to exchange and validate dialog data.
*  Author Name    : Vilas & Prajakta   
*  SR_NO		  :
*  Date           : 16 Nov 2013
**********************************************************************************************************/

void CRegistryOptimizerDlg::DoDataExchange(CDataExchange* pDX)
{
	CJpegDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_STOP, m_btnStop);
	DDX_Control(pDX, IDC_BTN_SCANREPAIR, m_btnScanRepair);
	DDX_Control(pDX, IDC_PROGRESS_STATUS, m_prgStatus);
	DDX_Control(pDX, IDC_STATIC_ENTRIES, m_stRepairedEntries);
	DDX_Control(pDX, IDC_ST_HREGOPT, m_stHRegOpt);
	DDX_Control(pDX, IDC_STATIC_REGSCANAREA, m_stRegScanArea);
	DDX_Control(pDX, IDC_STATIC_ACTIVEX, m_stActiveX);
	DDX_Control(pDX, IDC_STATIC_UNINSTALL, m_stUninstall);
	DDX_Control(pDX, IDC_STATIC_FONTENT, m_stFontEnt);
	DDX_Control(pDX, IDC_STATIC_SHAREDDLL, m_stSharedDll);
	DDX_Control(pDX, IDC_STATIC_APPPATH, m_stAppPath);
	DDX_Control(pDX, IDC_STATIC_HELPINFO, m_stHelpFileInfo);
	DDX_Control(pDX, IDC_STATIC_STARTUP, m_stStartup);
	DDX_Control(pDX, IDC_STATIC_SERVICES, m_stWinServices);
	DDX_Control(pDX, IDC_STATIC_INVALIDEXT, m_stInvalidExt);
	DDX_Control(pDX, IDC_STATIC_ROOTKITS, m_stRootkits);
	DDX_Control(pDX, IDC_STATIC_ROGUEAPP, m_stRogueApp);
	DDX_Control(pDX, IDC_STATIC_WORMS, m_stWorms);
	DDX_Control(pDX, IDC_STATIC_SPYWARETHREATS, m_stSpywareThreats);
	DDX_Control(pDX, IDC_STATIC_ADWARETHREATS, m_stAdwareThreats);
	DDX_Control(pDX, IDC_STATIC_KEYLOGGERS, m_stKeyloggers);
	DDX_Control(pDX, IDC_STATIC_BHO, m_stBHO);
	DDX_Control(pDX, IDC_STATIC_EXPLORERENT, m_stExplorerEnt);
	DDX_Control(pDX, IDC_STATIC_INTERNETENT, m_stInternetExpEnt);
	DDX_Control(pDX, IDC_BTN_BACK, m_btnBack);
	DDX_Control(pDX, IDC_CHECK_ACTIVEX, m_btnActiveX);
	DDX_Control(pDX, IDC_CHECK_UNINSTALL, m_btnUninstall);
	DDX_Control(pDX, IDC_CHECK_FONTENT, m_btnFontEnt);
	DDX_Control(pDX, IDC_CHECK_SHAREDDLL, m_btnSharedDll);
	DDX_Control(pDX, IDC_CHECK_APPPATH, m_btnAppPath);
	DDX_Control(pDX, IDC_CHECK_HELPINFO, m_btnHelpFileInfo);
	DDX_Control(pDX, IDC_CHECK_STARTUP, m_btnWinStartup);
	DDX_Control(pDX, IDC_CHECK_WINSERVICES, m_btnWinServices);
	DDX_Control(pDX, IDC_CHECK_INVALIDEXT, m_btnInvalidExt);
	DDX_Control(pDX, IDC_CHECK_ROOTKITS, m_btnRootkits);
	DDX_Control(pDX, IDC_CHECK_ROGUEAPP, m_btnRogueApp);
	DDX_Control(pDX, IDC_CHECK_WORMS, m_btnWorms);
	DDX_Control(pDX, IDC_CHECK_SPYTHREATS, m_btnSpyThreats);
	DDX_Control(pDX, IDC_CHECK_ADWARETHREATS, m_btnAdwareThreats);
	DDX_Control(pDX, IDC_CHECK_KEYLOGGERS, m_btnKeyloggers);
	DDX_Control(pDX, IDC_CHECK_BHO, m_btnBHO);
	DDX_Control(pDX, IDC_CHECK_EXPLORERENT, m_btnExplorerEnt);
	DDX_Control(pDX, IDC_CHECK_INTERNETENT, m_btnInternetExpEnt);
	DDX_Control(pDX, IDC_STATIC_PERCENTAGE, m_Static_Percentage);
	DDX_Control(pDX, IDC_STATIC_REGISTRY_OPTIMIZER_HEADER, m_stRegistryHeaderName);
	/*****************ISSUE NO -81 Neha Gharge 22/5/14 ************************************************/
	DDX_Control(pDX, IDC_CHECK_SELECTALL, m_chkSelectAll);
	DDX_Control(pDX, IDC_STATIC_SELECTALL, m_stSelectAll);
	DDX_Control(pDX, IDC_STATIC_TOTAL_REPAIRED_ENTRIES, m_stTotalRepairedEntries);
	DDX_Control(pDX, IDC_STATIC_REGOPT_DESC, m_stRegOptHeaderDesc);
}

/**********************************************************************************************************                     
*  Function Name  :	MESSAGE_MAP                                                     
*  Description    :	Handle WM_COMMAND,WM_Messages,user defined message and notification message from child windows.
*  SR.NO          : 
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
BEGIN_MESSAGE_MAP(CRegistryOptimizerDlg, CJpegDialog)
	ON_WM_CTLCOLOR()
	ON_WM_NCHITTEST()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_BTN_STOP, &CRegistryOptimizerDlg::OnBnClickedBtnStop)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_PROGRESS_STATUS, &CRegistryOptimizerDlg::OnNMCustomdrawProgressStatus)
	ON_BN_CLICKED(IDC_BTN_SCANREPAIR, &CRegistryOptimizerDlg::OnBnClickedBtnScanrepair)
	ON_STN_CLICKED(IDC_STATIC_ENTRIES, &CRegistryOptimizerDlg::OnStnClickedStaticEntries)
	ON_STN_CLICKED(IDC_STATIC_REGSCANAREA, &CRegistryOptimizerDlg::OnStnClickedStaticRegscanarea)
	ON_BN_CLICKED(IDC_BTN_BACK, &CRegistryOptimizerDlg::OnBnClickedBtnBack)
	ON_WM_TIMER()
	ON_WM_SETCURSOR()
	ON_BN_CLICKED(IDC_CHECK_SELECTALL, &CRegistryOptimizerDlg::OnBnClickedCheckSelectall)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CRegistryOptimizerDlg message handlers

/**********************************************************************************************************                     
*  Function Name  :	OnInitDialog                                                     
*  Description    :	Windows calls the OnInitDialog function through the standard global dialog-box procedure common to all Microsoft 
					Foundation Class Library dialog boxes
*  SR.NO          : 
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
BOOL CRegistryOptimizerDlg::OnInitDialog()
{
	CJpegDialog::OnInitDialog();

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOMOVE|SWP_NOSIZE);
	// code for painting the background Ends here
	//to set round window
	if(!Load(theApp.m_hResDLL, MAKEINTRESOURCE(IDR_JPG_REGOPT_BG), _T("JPG")))
	{
		m_bIsPopUpDisplayed = true;
		::MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONERROR);
		m_bIsPopUpDisplayed = false;
	}

	Draw();

	CRect rect1;
	this->GetClientRect(rect1);
	
	m_hButtonCursor = LoadCursor(theApp.m_hResDLL,MAKEINTRESOURCE(IDC_CURSOR_HAND));
	
    //CFont *m_Font1 = new CFont;
	//m_Font1->CreatePointFont(120, _T("Verdana"));
	
	//m_stRegistryHeaderName.SetTextAlign(TA_LEFT);
	//m_stRegistryHeaderName.SetColor(RGB(230,232,238));
	//m_stRegistryHeaderName.SetGradientColor(RGB(230,232,238));
	//m_stRegistryHeaderName.SetVerticalGradient(1);

	//m_stRegistryHeaderName.SetWindowPos(&wndTop,rect1.left +20,13,540,35,SWP_NOREDRAW);
	//m_stRegistryHeaderName.SetWindowTextW(L"Registry Optimizer");
	m_stRegistryHeaderName.SetWindowPos(&wndTop, rect1.left + 20, 07, 540, 35, SWP_NOREDRAW);
	m_stRegistryHeaderName.SetTextColor(RGB(24,24,24));
	m_stRegistryHeaderName.SetBkColor(RGB(230, 232, 238));
	// Issue Add description Neha Gharge 9-3-2015
	m_stRegOptHeaderDesc.SetWindowPos(&wndTop, rect1.left + 20, 35, 400, 15, SWP_NOREDRAW);
	m_stRegOptHeaderDesc.SetTextColor(RGB(24, 24, 24));
	m_stRegOptHeaderDesc.SetBkColor(RGB(230, 232, 238));

	//if(theApp.m_dwOSType == WINOS_WIN8 ||theApp.m_dwOSType == WINOS_WIN8_1)
	//{
	//	m_stRegistryHeaderName.SetWindowPos(&wndTop,rect1.left +20,16,540,35,SWP_NOREDRAW);
	//}

	//m_bmpRegOpt.LoadBitmapW(IDB_BITMAP_REGOPT);//resource dll
	m_bmpRegOpt = LoadBitmapW(theApp.m_hResDLL,MAKEINTRESOURCE(IDB_BITMAP_COM_HEADER));
	m_stHRegOpt.SetWindowPos(&wndTop,rect1.left + 6,10,586,45,SWP_NOREDRAW);
	m_stHRegOpt.SetBitmap(m_bmpRegOpt);

	m_stRegScanArea.SetBkColor(WHITE);
	//m_stRegScanArea.SetWindowPos(&wndTop,rect1.left + 60,90,170,20,SWP_NOREDRAW);
	m_stRegScanArea.SetWindowPos(&wndTop,rect1.left + 20,90,250,25,SWP_NOREDRAW);
	//m_stRegScanArea.SetWindowTextW(L"Registry Scan Areas");

	//ISSUE NO:- 198 RAJIL YADAV Date :- 22/5/2014
	/*****************ISSUE NO -81,374 Neha Gharge 22/5/14 ************************************************/
	//m_btnScanRepair.SetSkin(IDB_BITMAP_REGOPT_BUTTON,IDB_BITMAP_REGOPT_BUTTON,IDB_BITMAP_REGOPT_BUTTON_HOVER,IDB_BITMAP_REGOPT_BUTTON,0,0,0,0);
	m_btnScanRepair.SetSkin(theApp.m_hResDLL,IDB_BITMAP_REGOPT_BUTTON,IDB_BITMAP_REGOPT_BUTTON,IDB_BITMAP_REGOPT_BUTTON_HOVER,IDB_BITMAP_REGOPT_BUTTON_DIS,0,0,0,0,0);
	m_btnScanRepair.SetWindowPos(&wndTop, rect1.left + 480, 350,115,22, SWP_NOREDRAW);
	m_btnScanRepair.SetTextColorA(BLACK,1,1);


	//Registry Scan Areas
	//Issue : In registry optimizer while selecting the scan areas, 
	//If we press "tab" control goes in backward direction and "shift+tab" control goes in forward direction.
	//Name : Niranjan Deshak. - 4/2/2015.
	//Description : Order is reveresed for member variables of all check boxes.
	m_btnInternetExpEnt.SetWindowPos(&wndTop,rect1.left + 410,250,15,15,SWP_NOREDRAW);
	m_stInternetExpEnt.SetWindowPos(&wndTop,rect1.left + 430,250,180,20,SWP_NOREDRAW);
	
	m_btnExplorerEnt.SetWindowPos(&wndTop,rect1.left + 410,225,15,15,SWP_NOREDRAW);
	m_stExplorerEnt.SetWindowPos(&wndTop,rect1.left + 430,225,135,20,SWP_NOREDRAW);

	m_btnBHO.SetWindowPos(&wndTop,rect1.left + 410,200,15,15,SWP_NOREDRAW);
	m_stBHO.SetWindowPos(&wndTop,rect1.left + 430,200,135,20,SWP_NOREDRAW);

	m_btnKeyloggers.SetWindowPos(&wndTop,rect1.left + 410,175,15,15,SWP_NOREDRAW);
	m_stKeyloggers.SetWindowPos(&wndTop,rect1.left + 430,175,135,20,SWP_NOREDRAW);

	m_btnKeyloggers.SetWindowPos(&wndTop,rect1.left + 410,175,15,15,SWP_NOREDRAW);
	m_stKeyloggers.SetWindowPos(&wndTop,rect1.left + 430,175,135,20,SWP_NOREDRAW);
	
	m_btnKeyloggers.SetWindowPos(&wndTop,rect1.left + 410,175,15,15,SWP_NOREDRAW);
	m_stKeyloggers.SetWindowPos(&wndTop,rect1.left + 430,175,135,20,SWP_NOREDRAW);

	m_btnAdwareThreats.SetWindowPos(&wndTop,rect1.left + 410,150,15,15,SWP_NOREDRAW);
	m_stAdwareThreats.SetWindowPos(&wndTop,rect1.left + 430,150,135,20,SWP_NOREDRAW);

	m_btnAdwareThreats.SetWindowPos(&wndTop,rect1.left + 410,150,15,15,SWP_NOREDRAW);
	m_stAdwareThreats.SetWindowPos(&wndTop,rect1.left + 430,150,135,20,SWP_NOREDRAW);

	m_btnSpyThreats.SetWindowPos(&wndTop,rect1.left + 410,125,15,15,SWP_NOREDRAW);
	m_stSpywareThreats.SetWindowPos(&wndTop,rect1.left + 430,125,135,20,SWP_NOREDRAW);

	m_btnWorms.SetWindowPos(&wndTop,rect1.left + 210,250,15,15,SWP_NOREDRAW);
	m_stWorms.SetWindowPos(&wndTop,rect1.left + 230,250,135,20,SWP_NOREDRAW);

	m_btnRogueApp.SetWindowPos(&wndTop,rect1.left + 210,225,15,15,SWP_NOREDRAW);
	m_stRogueApp.SetWindowPos(&wndTop,rect1.left + 230,225,135,20,SWP_NOREDRAW);

	m_btnRootkits.SetWindowPos(&wndTop,rect1.left + 210,200,15,15,SWP_NOREDRAW);
	m_stRootkits.SetWindowPos(&wndTop,rect1.left + 230,200,135,20,SWP_NOREDRAW);

	m_btnInvalidExt.SetWindowPos(&wndTop,rect1.left + 210,175,15,15,SWP_NOREDRAW);
	m_stInvalidExt.SetWindowPos(&wndTop,rect1.left + 230,175,135,20,SWP_NOREDRAW);

	m_btnWinServices.SetWindowPos(&wndTop,rect1.left + 210,150,15,15,SWP_NOREDRAW);
	m_stWinServices.SetWindowPos(&wndTop,rect1.left + 230,150,180,20,SWP_NOREDRAW);

	m_btnWinStartup.SetWindowPos(&wndTop,rect1.left + 210,125,15,15,SWP_NOREDRAW);
	m_stStartup.SetWindowPos(&wndTop,rect1.left + 230,125,180,20,SWP_NOREDRAW);

	m_btnHelpFileInfo.SetWindowPos(&wndTop,rect1.left + 20,250,15,15,SWP_NOREDRAW);
	m_stHelpFileInfo.SetWindowPos(&wndTop,rect1.left + 40,250,150,20,SWP_NOREDRAW);

	m_btnAppPath.SetWindowPos(&wndTop,rect1.left + 20,225,15,15,SWP_NOREDRAW);
	m_stAppPath.SetWindowPos(&wndTop,rect1.left + 40,225,120,20,SWP_NOREDRAW);

	m_btnSharedDll.SetWindowPos(&wndTop,rect1.left + 20,200,15,15,SWP_NOREDRAW);
	m_stSharedDll.SetWindowPos(&wndTop,rect1.left + 40,200,120,20,SWP_NOREDRAW);

	m_btnFontEnt.SetWindowPos(&wndTop,rect1.left + 20,175,15,15,SWP_NOREDRAW);
	m_stFontEnt.SetWindowPos(&wndTop,rect1.left + 40,175,120,25,SWP_NOREDRAW);

	m_btnUninstall.SetWindowPos(&wndTop,rect1.left + 20,150,15,15,SWP_NOREDRAW);
	m_stUninstall.SetWindowPos(&wndTop,rect1.left + 40,150,120,20,SWP_NOREDRAW);
	
	m_stActiveX.SetBkColor(RGB(100,100,100));

	m_btnActiveX.SetWindowPos(&wndTop,rect1.left + 20,125,15,15,SWP_NOREDRAW);
	m_stActiveX.SetWindowPos(&wndTop,rect1.left + 40,125,150,20,SWP_NOREDRAW);
	
	m_stActiveX.SetBkColor(RGB(0,255,0));

	
	m_chkSelectAll.SetWindowPos(&wndTop, rect1.left + 8, 250 + 58, 13 , 13, SWP_NOREDRAW);
	m_chkSelectAll.ShowWindow(SW_SHOW);
	m_stSelectAll.SetWindowPos(&wndTop,rect1.left + 27,250 + 57,120,17,SWP_NOREDRAW);
	m_stSelectAll.SetBkColor(RGB(88, 88, 90));
	m_stSelectAll.SetTextColor(RGB(255,255,255));
	m_stSelectAll.SetFont(&theApp.m_fontWWTextNormal);
	
	//m_btnStop.SetSkin(IDB_BITMAP_REGOPT_BUTTON,IDB_BITMAP_REGOPT_BUTTON,IDB_BITMAP_REGOPT_BUTTON_HOVER,IDB_BITMAP_REGOPT_BUTTON_DIS,0,0,0,1);
	m_btnStop.SetSkin(theApp.m_hResDLL,IDB_BITMAP_REGOPT_BUTTON,IDB_BITMAP_REGOPT_BUTTON,IDB_BITMAP_REGOPT_BUTTON_HOVER,IDB_BITMAP_REGOPT_BUTTON_DIS,0,0,0,0,0);
	m_btnStop.SetWindowPos(&wndTop,rect1.left + 480, 350,115,22, SWP_NOREDRAW);
	m_btnStop.SetTextColorA(BLACK,1,1);
	m_btnStop.ShowWindow(false);


	/*	ISSUE NO - 717 NAME - NITIN K. TIME - 15th June 2014 */
	m_stTotalRepairedEntries.SetWindowPos(&wndTop,rect1.left + 410,307,155,20,SWP_NOREDRAW);
	m_stTotalRepairedEntries.SetColor(RGB(70,70,70));
	m_stTotalRepairedEntries.SetGradientColor(RGB(70,70,70));
	m_stTotalRepairedEntries.SetTextColor(RGB(255,255,255));
	/*	ISSUE NO - 718 NAME - NITIN K. TIME - 15th June 2014 */
	m_stTotalRepairedEntries.SetFont(&theApp.m_fontWWTextNormal);


	m_stRepairedEntries.SetWindowPos(&wndTop,rect1.left + 575,309,30,20,SWP_NOREDRAW);
//	m_stRepairedEntries.SetTextAlign(TA_RIGHT);
	
	m_stRepairedEntries.SetBkColor(RGB(70,70,70));
//	m_stRepairedEntries.SetGradientColor(RGB(70,70,70));
//	m_stRepairedEntries.SetVerticalGradient(1);
    m_stRepairedEntries.SetTextColor(RGB(255,255,255));

	//csRepairedEntriesTxt.Format(L"Total Entries Repaired: %d",0);

    m_prgStatus.SetWindowPos(&wndTop,rect1.left + 6,350,470,22,SWP_SHOWWINDOW);
	
	m_Static_Percentage.SetWindowPos(&wndTop,rect1.left + 490,350,80,20,SWP_NOREDRAW);
    
	m_Static_Percentage.SetBkColor(RGB(243,239,238));
	m_Static_Percentage.ShowWindow(SW_HIDE);

	m_prgStatus.AlignText(DT_CENTER);
	m_prgStatus.SetBarColor(RGB(171,238,0));
	m_prgStatus.SetShowPercent(true);	
	m_prgStatus.SetBkColor(RGB(243,239,238));

	m_btnBack.SetSkin(IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROW,IDB_BITMAP_BACKARROWDISABLE,0,0,0,0,0);
	m_btnBack.SetWindowPos(&wndTop,rect1.left + 21,354,31,32,SWP_NOREDRAW);
	m_btnBack.ShowWindow(SW_HIDE);
	// Issue Add description Neha Gharge 9-3-2015
	m_stRegScanArea.SetFont(&theApp.m_fontWWTextSubTitle);
	m_stRegistryHeaderName.SetFont(&theApp.m_fontWWTextSmallTitle);
	m_stRegScanArea.SetFont(&theApp.m_fontWWTextSubTitle);
	m_stRegistryHeaderName.SetFont(&theApp.m_fontWWTextSmallTitle);
	m_stRegOptHeaderDesc.SetFont(&theApp.m_fontWWTextSubTitleDescription);
	m_stActiveX.SetFont(&theApp.m_fontWWTextNormal);
	m_stUninstall.SetFont(&theApp.m_fontWWTextNormal);
	m_stFontEnt.SetFont(&theApp.m_fontWWTextNormal);
	m_stSharedDll.SetFont(&theApp.m_fontWWTextNormal);
	m_stAppPath.SetFont(&theApp.m_fontWWTextNormal);
	m_stHelpFileInfo.SetFont(&theApp.m_fontWWTextNormal);
	m_stStartup.SetFont(&theApp.m_fontWWTextNormal);
	m_stWinServices.SetFont(&theApp.m_fontWWTextNormal);
	m_stInvalidExt.SetFont(&theApp.m_fontWWTextNormal);
	m_stRootkits.SetFont(&theApp.m_fontWWTextNormal);
	m_stRogueApp.SetFont(&theApp.m_fontWWTextNormal);
	m_stWorms.SetFont(&theApp.m_fontWWTextNormal);
	m_stSpywareThreats.SetFont(&theApp.m_fontWWTextNormal);
	m_stAdwareThreats.SetFont(&theApp.m_fontWWTextNormal);
	m_stKeyloggers.SetFont(&theApp.m_fontWWTextNormal);
	m_stBHO.SetFont(&theApp.m_fontWWTextNormal);
	m_stExplorerEnt.SetFont(&theApp.m_fontWWTextNormal);
	m_stInternetExpEnt.SetFont(&theApp.m_fontWWTextNormal);
	m_stRepairedEntries.SetFont(&theApp.m_fontWWTextNormal);
	m_prgStatus.SetFont(&theApp.m_fontWWTextNormal);
	m_btnScanRepair.SetFont(&theApp.m_fontWWTextNormal);
	m_btnStop.SetFont(&theApp.m_fontWWTextNormal);

	m_Static_Percentage.SetFont(&theApp.m_fontText);
//	m_btnBackDisable.SetSkin(IDB_BITMAP_BACKARROWDISABLE,IDB_BITMAP_BACKARROWDISABLE,IDB_BITMAP_BACKARROWDISABLE,IDB_BITMAP_BACKARROWDISABLE,0,0,0,0);
//	m_btnBackDisable.SetWindowPos(&wndTop,rect1.left + 21,354,31,32,SWP_SHOWWINDOW);


	//Default Settings
	m_btnActiveX.SetCheck(true);
	m_btnUninstall.SetCheck(true);
	m_btnAppPath.SetCheck(true);
	m_btnWinStartup.SetCheck(true);
	m_btnRootkits.SetCheck(true);
	m_btnRogueApp.SetCheck(true);
	m_btnWorms.SetCheck(true);
	m_btnSpyThreats.SetCheck(true);
	m_btnAdwareThreats.SetCheck(true);
	m_btnKeyloggers.SetCheck(true);
	m_btnBHO.SetCheck(true);
	m_btnExplorerEnt.SetCheck(true);
	m_btnInternetExpEnt.SetCheck(true);
	
	RefreshStrings();


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/**********************************************************************************************************                     
*  Function Name  :	RefreshStrings                                                     
*  Description    :	This will load the string from the .INI files depending upon the language set
*  Author Name    : Prasanna
*  SR_NO		  :
*  Date           : 30 APR 2014
**********************************************************************************************************/

void CRegistryOptimizerDlg::RefreshStrings()
{
	//m_stRegistryHeaderName.SetWindowTextW(L"Registry Optimizer");
	//m_stRegScanArea.SetWindowTextW(L"Registry Scan Areas");
	//m_stActiveX.SetWindowTextW(L"COM/ActiveX Entries");
	//m_stUninstall.SetWindowTextW(L"Uninstall Entries");
	//m_stFontEnt.SetWindowTextW(L"Font Entries");
	//m_stSharedDll.SetWindowTextW(L"Shared DLLs");
	//m_stAppPath.SetWindowTextW(L"Application Paths");
	//m_stHelpFileInfo.SetWindowTextW(L"Help File Information");
	//m_stStartup.SetWindowTextW(L"Windows Startup items");
	//m_stWinServices.SetWindowTextW(L"Windows Services");
	//m_stInvalidExt.SetWindowTextW(L"Invalid Extensions");
	//m_stRootkits.SetWindowTextW(L"RootKits");
	//m_stRogueApp.SetWindowTextW(L"Rogue Applications");
	//m_stWorms.SetWindowTextW(L"Worms");
	//m_stSpywareThreats.SetWindowTextW(L"Spyware Threats");
	//m_stAdwareThreats.SetWindowTextW(L"Adware Threats");
	//m_stKeyloggers.SetWindowTextW(L"Keyloggers");
	//m_stBHO.SetWindowTextW(L"BHO");
	//m_stExplorerEnt.SetWindowTextW(L"Explorer Entries");
	//m_stInternetExpEnt.SetWindowTextW(L"Internet Explorer Entries");
	//m_stRepairedEntries.SetWindowTextW(TEXT("Total Repaired Entries: 0"));
	//m_btnScanRepair.SetWindowTextW(L"Scan and Repair");
	//m_btnStop.SetWindowTextW(L"Stop");
	//m_Static_Percentage.SetWindowText(L"0%");
	// Issue Add description Neha Gharge 9-3-2015
	m_stSelectAll.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_SELECTALL"));
	m_stRegistryHeaderName.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGISTRYOPTIMIZER"));
	m_stRegOptHeaderDesc.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_HEADER_DESCRIPTION"));
	m_stRegScanArea.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_REGSCANAREAS"));
	m_stActiveX.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_ACTIVEX"));
	m_stUninstall.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_UNINSTALL"));
	m_stFontEnt.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_FONTENTRIES"));
	m_stSharedDll.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_SHAREDDLL"));
	m_stAppPath.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_APPPATHS"));
	m_stHelpFileInfo.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_HELPFILEINFO"));
	m_stStartup.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_WINSTARTUP"));
	m_stWinServices.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_WINSERVICES"));
	m_stInvalidExt.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_INVALIDEXT"));
	m_stRootkits.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_ROOTKITS"));
	m_stRogueApp.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_ROGUEAPPS"));
	m_stWorms.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_WORMS"));
	m_stSpywareThreats.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_SPYWARE"));
	m_stAdwareThreats.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_ADWARE"));
	m_stKeyloggers.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_KEYLOGGERS"));
	m_stBHO.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_BHO"));
	m_stExplorerEnt.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_EXPLORERENT"));
	m_stInternetExpEnt.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_INETEXPENTRIES"));
	
	CString csTotalRepiredEntries;
	csTotalRepiredEntries.Format(L"%s ",L"0");
	m_stRepairedEntries.SetWindowTextW(csTotalRepiredEntries);

	m_btnScanRepair.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_REGOPT_SCANREPAIR"));
	m_btnStop.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_BUTTON_STOP"));
	m_Static_Percentage.SetWindowText(L"0%");

	m_stTotalRepairedEntries.SetWindowTextW(theApp.m_objwardwizLangManager.GetString(L"IDS_REG_OPT_TOTAL_ENTRIES_REPAIRED"));
}


/**********************************************************************************************************                     
*  Function Name  :	OnNcHitTest                                                     
*  Description    :	The framework calls this member function for the CWnd object that contains the cursor every time the mouse is moved.
*  Author Name    : Vilas & Prajakta
*  SR_NO		  :
*  Date           : 16 Nov 2013
**********************************************************************************************************/
LRESULT CRegistryOptimizerDlg::OnNcHitTest(CPoint point)
{
	return HTCLIENT;
}

/**********************************************************************************************************                     
*  Function Name  :	OnNMCustomdrawProgressStatus                                                     
*  Description    :	Customize progress bar
*  Author Name    : Vilas & Prajakta  
*  SR_NO		  :
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CRegistryOptimizerDlg::OnNMCustomdrawProgressStatus(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnScanrepair                                                     
*  Description    :	Sends data to service to scan & repair selected registry areas
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CRegistryOptimizerDlg::OnBnClickedBtnScanrepair()
{
	try
	{
		//Varada Ikhar, Date:17-03-2015, 
		//Issue:User updated the product and does not restarted, and started a scan, then it should show message as 
		//"Product updated, restart is required to take a effect."
		if (!theApp.ShowRestartMsgOnProductUpdate())
		{
			return;
		}

		//Ram, Issue No: 0001216, Resolved
		if (theApp.m_dwDaysLeft == 0)
		{
			theApp.GetDaysLeft();
		}

		if (theApp.m_dwDaysLeft == 0)
		{
			if (!theApp.ShowEvaluationExpiredMsg())
			{
				theApp.GetDaysLeft();
				return;
			}
		}

		AddLogEntry(L">>> Register optimizer scanning started", 0, 0, true, FIRSTLEVEL);
		/*	m_hRegOptDLL = NULL ;
			ShowTotalEntries = NULL ;
			ScanAndRepairPercentage = NULL ;
			ScanAndRepairRegistryEntries = NULL ;

			bool bError = false ;
			*/
		m_btnScanRepair.ShowWindow(SW_HIDE);
		//Issue : In Registry optimizer->click 'scan&repair'->goto Data Encryption->click 'browse'->'registry optimizer' completed popup appears->now goto registry optimizer->
		//'stop' button is still enabled->click on 'stop' without closing popup->popup appearing stating 'Registration Optimization is in progress'
		//Resolved By : Nitin K  Date : 10th March 2015
		m_btnStop.EnableWindow(true);
		m_btnStop.ShowWindow(SW_SHOW);
		m_btnBack.EnableWindow(false);

		//Varada Ikhar, Date:09/03/2015, Issue:In Registry Optimizer->scanning is in process then its allowing to check and uncheck option.
		m_btnActiveX.EnableWindow(false);
		m_btnUninstall.EnableWindow(false);
		m_btnFontEnt.EnableWindow(false);
		m_btnSharedDll.EnableWindow(false);
		m_btnAppPath.EnableWindow(false);
		m_btnHelpFileInfo.EnableWindow(false);

		m_btnWinStartup.EnableWindow(false);
		m_btnWinServices.EnableWindow(false);
		m_btnInvalidExt.EnableWindow(false);
		m_btnRootkits.EnableWindow(false);
		m_btnRogueApp.EnableWindow(false);
		m_btnWorms.EnableWindow(false);

		m_btnSpyThreats.EnableWindow(false);
		m_btnAdwareThreats.EnableWindow(false);
		m_btnKeyloggers.EnableWindow(false);
		m_btnBHO.EnableWindow(false);
		m_btnExplorerEnt.EnableWindow(false);
		m_btnInternetExpEnt.EnableWindow(false);

		m_chkSelectAll.EnableWindow(false);

		//Varada Ikhar, Date: 28/04/2015
		//Issue : In Registry optimizer, while performing scan, stop, pause, resume operation UI is getting hanged.
		m_dwPercentage = 0x00;
		m_dwTotalEntries = 0x00;

		//g_TabCtrlRrgOptWindHandle = (CISpyGUIDlg*)AfxGetMainWnd();	
		//g_TabCtrlRrgOptWindHandle->m_pTabDialog->DisableAllExceptSelected();
		/*	CString csAppPath = theApp.GetModuleFilePath() ;

			CString csRegOptDllPath( "" ) ;
			csRegOptDllPath.Format(TEXT("%s\\ISpyRegOp.dll"), csAppPath) ;
			if( PathFileExists( csRegOptDllPath ) )
			{
			m_hRegOptDLL = LoadLibrary( csRegOptDllPath ) ;
			if( !m_hRegOptDLL )
			AfxMessageBox( TEXT("Load Library failed") ) ;
			ShowTotalEntries = (SHOWTOTALENTRIES ) GetProcAddress(m_hRegOptDLL, "ShowTotalEntries") ;
			ScanAndRepairPercentage = (SCANANDREPAIRPERCENTAGE ) GetProcAddress(m_hRegOptDLL, "ScanAndRepairPercentage") ;
			ScanAndRepairRegistryEntries = (SCANANDREPAIRREGISTRYENTRIES ) GetProcAddress(m_hRegOptDLL, "ScanAndRepairRegistryEntries") ;
			}

			if( !ShowTotalEntries)
			{
			AfxMessageBox( TEXT("Registry Optimizer module not found") ) ;
			bError = true ;
			goto CleanUp ;
			}

			if(!ScanAndRepairPercentage)
			{
			AfxMessageBox( TEXT("Registry Optimizer module not found" )) ;
			bError = true ;
			goto CleanUp ;
			}

			if( !ScanAndRepairRegistryEntries )
			{
			AfxMessageBox( TEXT("Registry Optimizer module not found") ) ;
			bError = true ;
			goto CleanUp ;
			}


			*/
		REGOPTSCANOPTIONS	StatOptions = { 0 };
		if (!GetRegOptScanAreaOptions(&StatOptions))
		{
			//MessageBox(L"Please select option to scan", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_bIsPopUpDisplayed = true;
			MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_SELECT_OPTION"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			m_bIsPopUpDisplayed = false;
			ResetControls();
			return;
		}

		m_bScanStarted = true;
		//Issue: In Registry optimizer->click 'scan & repair'->click on 'stop'->wait till it finishes 100%->
		//popup appears->click on 'yes' button in abort confirmation popup ->now one more popup displayed 
		//stating 'registry optimizer aborted'
		//Neha Gharge 11/3/2015 
		m_bScanCompleted = false;
		m_bClose = false;
		m_bRegOptHome = false;

		DWORD dwScanOption = 0;
		GetDWORDFromScanOptions(dwScanOption, &StatOptions);
		AddLogEntry(L">>> Sending START_REGISTRY_OPTIMIZER to services", 0, 0, true, FIRSTLEVEL);
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = START_REGISTRY_OPTIMIZER;
		szPipeData.dwValue = dwScanOption;

		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data", 0, 0, true, SECONDLEVEL);
			return;
		}

		m_prgStatus.SetRange32(0, 0x12);

		CString csTotalRepiredEntries;
		csTotalRepiredEntries.Format(L"%s ", L"0");
		m_stRepairedEntries.SetWindowTextW(csTotalRepiredEntries);

		SetTimer(TIMER_REGOPT_STATISTICS, 1000, NULL);

		//Varada Ikhar, Date: 28/04/2015
		//Issue : In Registry optimizer, while performing scan, stop, pause, resume operation UI is getting hanged.
		DWORD dwret = m_objiTinServerMemMap_Client.OpenServerMemoryMappedFile();

		if (dwret)
		{
			//MessageBox(L"!!! Error in OpenServerMemoryMappedFile", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			AddLogEntry(L"### Error in OpenServerMemoryMappedFile in CRegistryOptimizerDlg::OnBnClickedBtnScanrepair", 0, 0, true, SECONDLEVEL);
		}


		m_hRegOptThread = NULL;
		m_hRegOptThread = ::CreateThread(NULL, 0, ScanRepairRegistryEntriesThread, (LPVOID) this, 0, NULL);
		Sleep(100);
		//KillTimer(TIMER_REGOPT_STATISTICS) ;
		/*
		CleanUp :

		if( bError )
		{
		m_btnScanRepair.ShowWindow( SW_SHOW ) ;
		m_btnStop.ShowWindow(  SW_HIDE ) ;
		}
		*/
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistryOptimizerDlg::OnBnClickedBtnScanrepair.", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	ShutDownScanning                                                     
*  Description    :	Sends data to service to abort scanning of selected registry areas & also resets controls
*  SR.N0		  : 
*  Author Name    : Vilas                                                                                        
*  Date           : 16 Nov 2013
**********************************************************************************************************/
bool CRegistryOptimizerDlg::ShutDownScanning()
{
	try
	{
		if (!m_bScanStarted)
			return true;

		//Varada Ikhar, Date: 23/04/2015, Implementing RegOpt Pause-Resume if user clicks on 'Close' button.
		if (!PauseRegistryOptimizer())
		{
			AddLogEntry(L"### Failed to pause Registry optimizer.", 0, 0, true, SECONDLEVEL);
		}

		//Not generating messagebox of completetion after taking No action on confirmation message
		//Neha Gharge 26/2/2015 
		//Neha Gharge 3/3/2015 
		//Issue : if user click on close button not take any action YES/NO, UI get hanged or not refresh
		
		m_bIsPopUpDisplayed = true;
		int iRet = MessageBox(theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_SCAN_PROGRESS"),
			theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_YESNO | MB_ICONQUESTION);
		m_bIsPopUpDisplayed = false;

		if (iRet == IDYES)
		{
			if (m_bScanStarted)
			{
				m_bScanStop = true;
			}
			if (m_bClose == true)
			{
				theApp.m_bOnCloseFromMainUI = true;
			}
			//Varada Ikhar, Date: 28/04/2015
			//Issue : In Registry optimizer, while performing scan, stop, pause, resume operation UI is getting hanged.
			if (m_hRegOptThread != NULL)
			{
				if (::TerminateThread(m_hRegOptThread, 0x00) == -1)
				{
					CString csErrorMsg = L"";
					DWORD ErrorCode = GetLastError();
					csErrorMsg.Format(L"### Failed to terminate ScanRepairRegistryOptimizer thread in ShutdownScanning with GetLastError code %d", ErrorCode);
					AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
				}
				m_hRegOptThread = NULL;
			}

		}
		if (iRet == IDNO)
		{
			//Varada Ikhar, Date: 23/04/2015, Implementing RegOpt Pause-Resume if user clicks on 'Close' button.
			if (!ResumeRegistryOptimizer())
			{
				AddLogEntry(L"### Failed to resume Registry optimizer.", 0, 0, true, SECONDLEVEL);
			}

			m_bScanStop = false;
			return false;
		}
		else if (iRet == IDYES && m_bClose && m_bScanStarted == false)
		{
			if (m_bClose == true)
			{
				theApp.m_bOnCloseFromMainUI = true;
			}
			return true;
		}

		else if (iRet == IDYES && (m_bScanStop || m_bRegOptHome || m_bClose))
		{

			if (m_bClose == true)
			{
				theApp.m_bOnCloseFromMainUI = true;
			}
			AddLogEntry(L">>> Sending STOP_REGISTRY_OPTIMIZER to service", 0, 0, true, FIRSTLEVEL);

			//Issue: In Registry optimizer->click 'scan & repair'->click on 'stop'->wait till it finishes 100%->
			//popup appears->click on 'yes' button in abort confirmation popup ->now one more popup displayed 
			//stating 'registry optimizer aborted'
			//Neha Gharge 11/3/2015 

			if (!m_bScanCompleted)
			{
				ISPY_PIPE_DATA szPipeData = { 0 };
				memset(&szPipeData, 0, sizeof(szPipeData));
				szPipeData.iMessageInfo = STOP_REGISTRY_OPTIMIZER;

				CISpyCommunicator objCom(SERVICE_SERVER, true);
				if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
				{
					AddLogEntry(L"### Failed to send data STOP_REGISTRY_OPTIMIZER", 0, 0, true, SECONDLEVEL);
					return false;
				}
				if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
				{
					AddLogEntry(L"### Failed to ReadData in CRegistryOptimizerDlg::ShutDownScanning", 0, 0, true, SECONDLEVEL);
					return false;
				}

				KillTimer(TIMER_REGOPT_STATISTICS);

				//MessageBox( TEXT("Registry optimization aborted"), TEXT("WardWiz"), MB_ICONINFORMATION);

				/*Issue No:9 Issue Desc: In Registry Optimizer, after completion and even after abortion, the msg box does not displays anything about "Repaired Enteries".
					Resolved by :	Divya S..*/
				CString sz_StopStatus;
				sz_StopStatus.Format(L"%s \n%s %d", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_SCAN_ABORT"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_SCAN_COUNT"), m_dwTotalRepairedEntries);

				//Ticket No : 43 and 37 Now according to design after clicking close button from UI, No one functionality
				// will give message box of aborted 
				//Even though 100% is shown on progress bar but click on close button. then also no completion message will popup
				if (m_bClose == false)
				{
					m_bIsPopUpDisplayed = true;
					MessageBox(sz_StopStatus, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
					m_bIsPopUpDisplayed = false;

					m_bScanStarted = false;
					//MessageBox( theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_SCAN_ABORT"), TEXT("WardWiz"), MB_ICONINFORMATION);
					ResetControls();  //ISSUE NO.: 6, VARADA IKHAR DATE: 20/12/2014 TIME: 4:30PM
					m_bScanStop = false;
				}
				m_bClose = false;
			}

		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistryOptimizerDlg::ShutDownScanning", 0, 0, true, SECONDLEVEL);
	}
	return true ;
}

/**********************************************************************************************************                     
*  Function Name  :	ScanRepairRegistryEntriesThread                                                     
*  Description    :	Gets the scan percentage and total entries
*  Author Name    : Varada Ikhar
*  SR_NO		  :
*  Date           : 28th April 2015
**********************************************************************************************************/
DWORD WINAPI ScanRepairRegistryEntriesThread(LPVOID lpParam)
{
	try
	{
		CRegistryOptimizerDlg *pRegOptDlg = (CRegistryOptimizerDlg *)lpParam;

		if (!pRegOptDlg)
			return 1;

		while (true)
		{
			ITIN_MEMMAP_DATA iTinMemMap = { 0 };
			pRegOptDlg->m_objiTinServerMemMap_Client.GetServerMemoryMappedFileData(&iTinMemMap, sizeof(iTinMemMap));

			//if(GetPercentageNTotalEntries(szValue, dwPercentage, dwTotalEntries))

			pRegOptDlg->m_dwPercentage = iTinMemMap.dwFirstValue;
			pRegOptDlg->m_dwTotalEntries = iTinMemMap.dwSecondValue;
		}

		Sleep(30);
		//
		//	REGOPTSCANOPTIONS	StatOptions = {0} ;
		//	if(!pRegOptDlg->GetRegOptScanAreaOptions( &StatOptions ) )
		//	{
		//		pRegOptDlg->MessageBox(L"Please select option to scan", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		//		pRegOptDlg->KillTimer( TIMER_REGOPT_STATISTICS ) ;
		//		pRegOptDlg->ResetControls();
		//		pRegOptThread = NULL ;
		//		return 0 ;
		//	}
		//
		//	ScanAndRepairRegistryEntries( &StatOptions ) ;
		//
		//	pRegOptDlg->KillTimer( TIMER_REGOPT_STATISTICS ) ;
		//
		//	TCHAR	szTemp[256] = {0} ;
		//
		//	swprintf( szTemp, TEXT("Total Repaired Entries: %lu"), ShowTotalEntries() ) ;
		//	pRegOptDlg->m_stRepairedEntries.SetWindowTextW( szTemp ) ;
		//
		//	pRegOptDlg->MessageBox(L"Registry optimization completed", theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION );
		//
		//	pRegOptDlg->ResetControls();
		//
		//	pRegOptThread = NULL ;
		//
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistryOptimizerDlg::ScanRepairRegistryEntriesThread.", 0, 0, true, SECONDLEVEL);
	}
	return 0 ;
}

/**********************************************************************************************************                     
*  Function Name  :	ResetControls                                                     
*  Description    :	Handles controls to hide/show on UI
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                        
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CRegistryOptimizerDlg::ResetControls()
{
	OutputDebugString(L">>> In CRegistryOptimizerDlg::ResetControls");

	if(m_bScanStarted == true)
	{
		m_btnStop.ShowWindow(  SW_SHOW ) ;
		m_btnScanRepair.ShowWindow( SW_HIDE ) ;
	}
	else
	{
		m_btnScanRepair.ShowWindow( SW_SHOW ) ;
		//resolve by lalit 4-4-2015
		// stop button getting disappear if registry optimizer closed using main GUI Close button.
		//Scan & Repair” button disappears.
		//m_btnStop.ShowWindow(  SW_HIDE ) ;
		m_btnStop.EnableWindow(FALSE);
		m_prgStatus.SetPos(0);
	}
	if (m_bScanStarted == false)
	{
		m_Static_Percentage.SetWindowTextW(L"0%");

		m_btnBack.EnableWindow(TRUE);
		//Added on 23/11/2013

		//Edited by Prasanna 28/04/2014
		CString csTotalRepiredEntries;
		csTotalRepiredEntries.Format(L"%s ", L"0");
		m_stRepairedEntries.SetWindowTextW(csTotalRepiredEntries);

		/*****************ISSUE NO -81 Neha Gharge 22/5/14 ************************************************/
		m_chkSelectAll.SetCheck(false);
		//m_stRepairedEntries.SetWindowTextW( L"Total Repaired Entries: 0" ) ;
		ResetRegistryScanOption();

		//Varada Ikhar, Date:09/03/2015, Issue:In Registry Optimizer->scanning is in process then its allowing to check and uncheck option.
		m_btnActiveX.EnableWindow(true);
		m_btnUninstall.EnableWindow(true);
		m_btnFontEnt.EnableWindow(true);
		m_btnSharedDll.EnableWindow(true);
		m_btnAppPath.EnableWindow(true);
		m_btnHelpFileInfo.EnableWindow(true);

		m_btnWinStartup.EnableWindow(true);
		m_btnWinServices.EnableWindow(true);
		m_btnInvalidExt.EnableWindow(true);
		m_btnRootkits.EnableWindow(true);
		m_btnRogueApp.EnableWindow(true);
		m_btnWorms.EnableWindow(true);

		m_btnSpyThreats.EnableWindow(true);
		m_btnAdwareThreats.EnableWindow(true);
		m_btnKeyloggers.EnableWindow(true);
		m_btnBHO.EnableWindow(true);
		m_btnExplorerEnt.EnableWindow(true);
		m_btnInternetExpEnt.EnableWindow(true);

		m_chkSelectAll.EnableWindow(true);
	}
	Invalidate();
}

/**********************************************************************************************************                     
*  Function Name  :	ResetRegistryScanOption                                                     
*  Description    :	Handles registry scan areas to hide/show on UI
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                        
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CRegistryOptimizerDlg::ResetRegistryScanOption()
{
	//Default
	m_btnActiveX.SetCheck(true);
	m_btnUninstall.SetCheck(true);
	m_btnAppPath.SetCheck(true);
	m_btnWinStartup.SetCheck(true);
	m_btnRootkits.SetCheck(true);
	m_btnRogueApp.SetCheck(true);
	m_btnWorms.SetCheck(true);
	m_btnSpyThreats.SetCheck(true);
	m_btnAdwareThreats.SetCheck(true);
	m_btnKeyloggers.SetCheck(true);
	m_btnBHO.SetCheck(true);
	m_btnExplorerEnt.SetCheck(true);
	m_btnInternetExpEnt.SetCheck(true);

	//Non Default
	m_btnFontEnt.SetCheck(false);
	m_btnSharedDll.SetCheck(false);
	m_btnHelpFileInfo.SetCheck(false);
	m_btnWinServices.SetCheck(false);
	m_btnInvalidExt.SetCheck(false);
	m_btnInvalidExt.RedrawWindow();
}

void CRegistryOptimizerDlg::OnStnClickedStaticEntries()
{
	// TODO: Add your control notification handler code here
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnStop                                                     
*  Description    :	Stops scanning of selected registry areas
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/

void CRegistryOptimizerDlg::OnBnClickedBtnStop()
{
	//Not generating messagebox of completetion after taking No action on confirmation message
	//Neha Gharge 26/2/2015 
	//m_bScanStop = true;
	// lalit 4-28-2015 , if we close on stop button and same time exit from tray then two popup comes , now single popup will come
	g_TabCtrlRrgOptWindHandle = (CISpyGUIDlg*)AfxGetMainWnd();
	if (g_TabCtrlRrgOptWindHandle != NULL)
	{
		m_btnBack.EnableWindow(true);
		g_TabCtrlRrgOptWindHandle->m_bIsCloseHandleCalled = true;
		if (!ShutDownScanning())
		{
			g_TabCtrlRrgOptWindHandle->m_bisUIcloseRquestFromTray = false;
		}
		m_bScanStop = false;
		g_TabCtrlRrgOptWindHandle->m_bIsCloseHandleCalled = false;
		if (g_TabCtrlRrgOptWindHandle->m_bisUIcloseRquestFromTray)
		{
			// resolved by lalit 5-5-2015
			//issue if "do you want to close message exits and we close from tray- 
			//in such case we convert already exist message popup as tray exit,and such case we have to close tray "
			m_bScanStarted = false;
			g_TabCtrlRrgOptWindHandle->CloseSystemTray();
			g_TabCtrlRrgOptWindHandle->HandleCloseButton();
		}
		
	}
	
}

void CRegistryOptimizerDlg::OnStnClickedStaticRegscanarea()
{
	// TODO: Add your control notification handler code here
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedBtnBack                                                     
*  Description    :	Allows to go back to parent dialog & also resets controls
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CRegistryOptimizerDlg::OnBnClickedBtnBack()
{
	ResetControls();
	ResetRegistryScanOption();
	this->ShowWindow(SW_HIDE);
	CISpyToolsDlg *pObjToolWindow = reinterpret_cast<CISpyToolsDlg*>( this->GetParent() );
	pObjToolWindow->ShowHideToolsDlgControls(true);
	ShutDownScanning();
}

/**********************************************************************************************************                     
*  Function Name  :	OnTimer                                                     
*  Description    :	Displays total repaired entries & scanning percentage 
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CRegistryOptimizerDlg::OnTimer(UINT_PTR nIDEvent)
{
	try
	{
		OutputDebugString(L">>> in CRegistryOptimizerDlg::OnTimer");

		TCHAR	szTemp[256] = { 0 };
		int		dwPos = 0x00;

		if (nIDEvent == TIMER_REGOPT_STATISTICS)
		{
			//Varada Ikhar, Date: 28/04/2015
			//Issue : In Registry optimizer, while performing scan, stop, pause, resume operation UI is getting hanged.
			//TCHAR szValue[MAX_PATH] = {0};
			//ITIN_MEMMAP_DATA iTinMemMap = {0};
			//m_objiTinServerMemMap_Client.GetServerMemoryMappedFileData(&iTinMemMap, sizeof(iTinMemMap));

			////if(GetPercentageNTotalEntries(szValue, dwPercentage, dwTotalEntries))

			//dwPercentage	= iTinMemMap.dwFirstValue;
			//dwTotalEntries	= iTinMemMap.dwSecondValue;

			m_dwTotalRepairedEntries = m_dwTotalEntries;
			if (m_dwPercentage > 0 || m_dwTotalEntries > 0)
			{
				TCHAR	szTemp[256] = { 0 };

				if (m_dwTotalEntries > 0)
				{

					//swprintf( szTemp, TEXT("Total Repaired Entries: %lu"), dwTotalEntries ) ;
					//swprintf( szTemp, TEXT("%lu"), dwTotalEntries) ;

					CString csTotalRepiredEntries;
					DWORD previousTotalEntries = m_dwTotalEntries;
					csTotalRepiredEntries.Format(L"%d", previousTotalEntries);
					m_stRepairedEntries.SetWindowTextW(csTotalRepiredEntries);
					m_stRepairedEntries.Invalidate(true);
					//m_stRepairedEntriesValue.SetWindowTextW( szTemp ) ;
				}

				if (m_dwPercentage > 0)
				{
					if (m_dwPercentage == 100)
					{
						m_btnStop.EnableWindow(false);
					}
					//dwPercentage = ScanAndRepairPercentage() ;
					dwPos = int((0x12 * m_dwPercentage) / 100);
					m_prgStatus.SetPos(dwPos);
					m_prgStatus.RedrawWindow();

					swprintf(szTemp, TEXT("%lu%%"), m_dwPercentage);

					m_Static_Percentage.SetWindowTextW(szTemp);
				}
			}
		}

		CJpegDialog::OnTimer(nIDEvent);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistryOptimizerDlg::OnTimer.", 0, 0, true, SECONDLEVEL);
	}
}
/**********************************************************************************************************                     
*  Function Name  :	GetDWORDFromScanOptions                                                     
*  Description    :	Gets DWORD value from selected registry scan areas
*  SR.NO          : 
*  Author Name    : Vilas                                                                                        
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CRegistryOptimizerDlg::GetDWORDFromScanOptions( DWORD &dwRegScanOpt, LPREGOPTSCANOPTIONS lpRegOpt)
{
	DWORD	dwValue = 0x00 ;
	DWORD	dwTemp  = 0x00 ;

	for(DWORD i=0x00; i<0x12; i++ )
	{
		switch( i )
		{
		case 0:
			if( lpRegOpt->bActiveX == true )
			{
				dwValue = dwTemp = 0x01 ;
			}
			break ;
		case 1:
			if( lpRegOpt->bUninstall == true )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		case 2:
			if( lpRegOpt->bFont == true )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		case 3:
			if( lpRegOpt->bSharedLibraries == true )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		case 4:
			if( lpRegOpt->bApplicationPaths == true )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		case 5:
			if( lpRegOpt->bHelpFiles == true )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		case 6:
			if( lpRegOpt->bStartup == true )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		case 7:
			if( lpRegOpt->bServices == true )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		case 8:
			if( lpRegOpt->bExtensions == true )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		case 9:
			if( lpRegOpt->bRootKit == true )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		case 10:
			if( lpRegOpt->bRogueApplications == true )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		case 11:
			if( lpRegOpt->bWorm == true )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		case 12:
			if( lpRegOpt->bSpywares == true )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		case 13:
			if( lpRegOpt->bAdwares == true )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		case 14:
			if( lpRegOpt->bKeyLogger == true )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		case 15:
			if( lpRegOpt->bBHO == true )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		case 16:
			if( lpRegOpt->bExplorer == true )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		case 17:
			if( lpRegOpt->bIExplorer == true )
			{
				dwTemp = 1<<i ;
				dwValue = dwValue + dwTemp ;
			}
			break ;
		}

	}
	dwRegScanOpt = dwValue;
}


/**********************************************************************************************************                     
*  Function Name  :	GetRegOptScanAreaOptions                                                     
*  Description    :	Checks selected registry scan areas
*  SR.NO          : 
*  Author Name    : Vilas                                                                                         
*  Date           : 16 Nov 2013
**********************************************************************************************************/
bool CRegistryOptimizerDlg::GetRegOptScanAreaOptions( LPREGOPTSCANOPTIONS pStatOptions )
{
	bool bReturn = false;
	if( m_btnActiveX.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bActiveX = true ;
	}

	if( m_btnUninstall.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bUninstall = true ;
	}

	if( m_btnFontEnt.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bFont = true ;
	}

	if( m_btnSharedDll.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bSharedLibraries = true ;
	}

	if( m_btnAppPath.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bApplicationPaths = true ;
	}

	if( m_btnHelpFileInfo.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bHelpFiles = true ;
	}

	if( m_btnWinStartup.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bStartup = true ;
	}

	if( m_btnWinServices.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bServices = true ;
	}

	if( m_btnInvalidExt.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bExtensions = true ;
	}

	if( m_btnRootkits.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bRootKit = true ;
	}

	if( m_btnRogueApp.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bRogueApplications = true ;
	}

	if( m_btnWorms.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bWorm = true ;
	}

	if( m_btnSpyThreats.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bSpywares = true ;
	}

	if( m_btnAdwareThreats.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bAdwares = true ;
	}

	if( m_btnKeyloggers.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bKeyLogger = true ;
	}

	if( m_btnBHO.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bBHO = true ;
	}

	if( m_btnExplorerEnt.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bExplorer = true ;
	}

	if( m_btnInternetExpEnt.GetCheck() == BST_CHECKED )
	{
		bReturn = true;
		pStatOptions->bIExplorer = true ;
	}
	return bReturn;
}

/**********************************************************************************************************                     
*  Function Name  :	OnCtlColor                                                     
*  Description    :	The framework calls this member function when a child control is about to be drawn.
*  Author Name    : Vilas & Prajakta
*  SR_NO		  :
*  Date           : 16 Nov 2013
**********************************************************************************************************/
HBRUSH CRegistryOptimizerDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
 HBRUSH hbr = CJpegDialog::OnCtlColor(pDC, pWnd, nCtlColor);
 int ctrlID;
 ctrlID = pWnd->GetDlgCtrlID();
 if( ctrlID == IDC_STATIC_REGSCANAREA      ||
	 ctrlID == IDC_STATIC_ACTIVEX   ||
	 ctrlID == IDC_STATIC_UNINSTALL  ||
	 ctrlID == IDC_STATIC_FONTENT  ||
	 ctrlID == IDC_STATIC_SHAREDDLL  ||
	 ctrlID == IDC_STATIC_APPPATH  ||
	   ctrlID == IDC_STATIC_HELPINFO  ||
	   ctrlID == IDC_STATIC_STARTUP  ||
	   ctrlID == IDC_STATIC_SERVICES  ||
	   ctrlID == IDC_STATIC_INVALIDEXT  ||
	   ctrlID == IDC_STATIC_ROGUEAPP  ||
	   ctrlID == IDC_STATIC_ROOTKITS  ||
	   ctrlID == IDC_STATIC_SPYWARETHREATS  ||
	   ctrlID == IDC_STATIC_ADWARETHREATS  ||
	   ctrlID == IDC_STATIC_BHO  ||
	   ctrlID == IDC_STATIC_KEYLOGGERS  ||
	   ctrlID == IDC_STATIC_EXPLORERENT  ||
	   ctrlID == IDC_STATIC_INTERNETENT  ||
	   ctrlID == IDC_STATIC_WORMS		||
	   ctrlID == IDC_STATIC_SELECTALL ||
	   ctrlID == IDC_STATIC_REGISTRY_OPTIMIZER_HEADER
     )
  
 {
  pDC->SetBkMode(TRANSPARENT);
  hbr = (HBRUSH)GetStockObject(NULL_BRUSH);
 } return hbr;
}

/**********************************************************************************************************                     
*  Function Name  :	OnSetCursor                                                     
*  Description    :	The framework calls this member function if mouse input is not captured and the mouse causes cursor movement within 
					the CWnd object.
*  Author Name    : Vilas & Prajakta  
*  SR_NO		  :
*  Date           : 16 Nov 2013
**********************************************************************************************************/
BOOL CRegistryOptimizerDlg::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	int	iCtrlID = 0;
	iCtrlID = pWnd->GetDlgCtrlID();
	if( iCtrlID == IDC_BTN_BACK					||
		iCtrlID == IDC_BTN_SCANREPAIR			||
		iCtrlID == IDC_BTN_STOP					||
		iCtrlID == IDC_CHECK_ACTIVEX			||
		iCtrlID == IDC_CHECK_UNINSTALL			||
		iCtrlID == IDC_CHECK_FONTENT			||
		iCtrlID == IDC_CHECK_SHAREDDLL			||
		iCtrlID == IDC_CHECK_APPPATH			||
		iCtrlID == IDC_CHECK_HELPINFO			||
		iCtrlID == IDC_CHECK_STARTUP			||
		iCtrlID == IDC_CHECK_WINSERVICES		||
		iCtrlID == IDC_CHECK_INVALIDEXT			||
		iCtrlID == IDC_CHECK_ROOTKITS			||	
		iCtrlID == IDC_CHECK_ROGUEAPP 			||
		iCtrlID == IDC_CHECK_WORMS 				||
		iCtrlID == IDC_CHECK_SPYTHREATS			||
		iCtrlID == IDC_CHECK_ADWARETHREATS		||
		iCtrlID == IDC_CHECK_BHO		 		||
		iCtrlID == IDC_CHECK_KEYLOGGERS			||
		iCtrlID == IDC_CHECK_EXPLORERENT		||
		iCtrlID == IDC_CHECK_INTERNETENT)
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
*  Function Name  :	ScanningStopped                                                     
*  Description    :	Displays scanning status & resets controls after complete scanning.
*  SR.NO          : 
*  Author Name    : Vilas                                                                                        
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CRegistryOptimizerDlg::ScanningStopped()
{
	try
	{
		if (!m_bScanStarted)
			return;
		// added by lalit 5-14-2015
		// to set blink when to taskbar icon when any scanning completed message comes
		::SetForegroundWindow(this->m_hWnd);
		AddLogEntry(L">>> Register optimizer scanning stopped", 0, 0, true, FIRSTLEVEL);

		//Issue: In Registry optimizer->click 'scan & repair'->click on 'stop'->wait till it finishes 100%->
		//popup appears->click on 'yes' button in abort confirmation popup ->now one more popup displayed 
		//stating 'registry optimizer aborted'
		//Neha Gharge 11/3/2015 
		m_bScanCompleted = true;

		//Varada Ikhar, Date: 27/04/2015
		// Issue : In Registry optimization, 'Scanning completed' message box is shown at 89% of progress bar. 
		if (!m_bScanStop)
		{
			int LowRange = 0, HighRange = 0;
			m_prgStatus.GetRange(LowRange, HighRange);
			m_prgStatus.SetPos(HighRange);
		}

		KillTimer(TIMER_REGOPT_STATISTICS);

		//Varada Ikhar, Date: 28/04/2015
		//Issue : In Registry optimizer, while performing scan, stop, pause, resume operation UI is getting hanged.
		if (m_hRegOptThread != NULL)
		{
			if (::TerminateThread(m_hRegOptThread, 0x00) == FALSE)
			{
				CString csErrorMsg = L"";
				DWORD ErrorCode = GetLastError();
				csErrorMsg.Format(L"### Failed to terminate ScanRepairRegistryOptimizer thread in ShutdownScanning with GetLastError code %d", ErrorCode);
				AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
			}
			m_hRegOptThread = NULL;
		}
		//Neha Gharge 3/3/2015 
		//Issue : if user click on close button not take any action YES/NO, UI get hanged or not refresh
		//if(m_bScanStop == false && m_bClose == false && m_bRegOptHome == false)
		if (m_bScanStop == false && m_bRegOptHome == false)
		{
			//MessageBox( TEXT("Registry optimization finished"), TEXT("WardWiz"), MB_ICONINFORMATION);
			/*Issue No:9 Issue Desc: In Registry Optimizer, after completion and even after abortion, the msg box does not displays anything about "Repaired Enteries".
				Resolved by :	Divya S..*/
			//Issue : In Registry optimizer->click 'scan&repair'->goto Data Encryption->click 'browse'->'registry optimizer' completed popup appears->now goto registry optimizer->
			//'stop' button is still enabled->click on 'stop' without closing popup->popup appearing stating 'Registration Optimization is in progress'
			//Resolved By : Nitin K  Date : 10th March 2015
			m_btnStop.EnableWindow(false);
			CString sz_Status;
			sz_Status.Format(L"%s \n%s %d", theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_SCAN_COMPLETE"), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_REGOPT_SCAN_COUNT"), m_dwTotalRepairedEntries);

			m_bIsPopUpDisplayed = true;
			MessageBox(sz_Status, theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
			m_bIsPopUpDisplayed = false;

			//Neha Gharge 28-2-2015
			//till the registry optimizer completes and pop up displays
			//->now we are able to click on other options in WardWiz without closing pop up->
			//and now go to registry optimizer it is showing 0 % without closing pop up.
			m_bScanStarted = false;
			ResetControls();  // ISSUE NO.:6, VARADA IKHAR, DATE: 20/12/2014 TIME: 4:30PM
		}

		//VARADA IKHAR,******ISSUE NO.:6, DATE: 20/12/2014 TIME: 4:30PM*******
		if (m_bScanStop == true)
		{
			CString strRepairStatus;
			strRepairStatus.Format(L"%d", m_dwTotalRepairedEntries);
			m_stRepairedEntries.SetWindowTextW(strRepairStatus);
		}

		//g_TabCtrlRrgOptWindHandle->m_pTabDialog->EnableAllBtn();
		Invalidate(TRUE);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistryOptimizerDlg::ScanningStopped.", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	GetPercentageNTotalEntries                                                     
*  Description    :	Calculates scanning percentage & total entries scanned
*  SR.NO          : 
*  Author Name    : Vilas                                                                                        
*  Date           : 16 Nov 2013
**********************************************************************************************************/
bool CRegistryOptimizerDlg::GetPercentageNTotalEntries(LPTSTR szEntry, DWORD &dwPercentage, DWORD &dwTotalEntries)
{
	bool bReturn  = false;
	
	CStringArray csaValues;
	if(!ExtractTokenString(szEntry, csaValues))
	{
		return bReturn;
	}

	try
	{
		if(csaValues[0].GetLength() != 0)
		{
			dwPercentage = _wtoi(csaValues[0]);
		}
		if(csaValues[1].GetLength() != 0)
		{
			dwTotalEntries = _wtoi(csaValues[1]);
		}
		bReturn = true;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CRegistryOptimizerDlg::GetPercentageNTotalEntries", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


///**********************************************************************************************************                     
//*  Function Name  : ShowTotalEntries                                                     
//*  Description    : Calculates total registry entries scannned & repaired
//*  Author Name    : Vilas & Prajakta                                                                                          
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD ShowTotalEntries()
//{
//
//	//AFX_MANAGE_STATE(AfxGetStaticModuleState());
//
//	DWORD	dwTotalEntries = 0x00 ;
//
//	dwTotalEntries =	dwActiveXEntries + dwUnInstallEntries + dwFontEntries +
//						dwSharedDLLs + dwAppPathEntries + dwHelpFilesEntries +
//						dwStartupRepairedEntries + dwServicesEntries +	
//						dwExtensionEntries + dwRootKitEntries +	dwRogueEntries +
//						dwWormEntries +	dwSpywareEntries + dwAdwareEntries +
//						dwKeyLoggerEntries + dwBHOEntries + dwExplorerEntries +	
//						dwIEEntries ;
//
//	return dwTotalEntries ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	ScanAndRepairPercentage                                                     
//*  Description    :	Displays percentage for registry entries scannned & repaired
//*  Author Name    : Vilas & Prajakta                                                                                          
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//
//int ScanAndRepairPercentage( )
//{
//	//AFX_MANAGE_STATE(AfxGetStaticModuleState());
//
//	return dwPercentage ;
//}
///**********************************************************************************************************                     
//*  Function Name  :	ScanAndRepairRegistryEntries                                                     
//*  Description    :	Scans & repairs selected registry areas/options
//*  Author Name    : Vilas & Prajakta                                                                                          
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
////DWORD ScanAndRepairRegistryEntries(LPREGOPTSCANOPTIONS pScanOpt )
////{
////
////	//AFX_MANAGE_STATE(AfxGetStaticModuleState());
////
////
////	InitializeVariables() ;
////
////	dwPercentage = 0x00 ;
////	dwScannedBit = 0x00 ;
////
////	dwActiveXEntries = dwUnInstallEntries = dwFontEntries =
////	dwSharedDLLs = dwAppPathEntries = dwHelpFilesEntries =
////	dwStartupRepairedEntries = dwServicesEntries =	
////	dwExtensionEntries = dwRootKitEntries =	dwRogueEntries =
////	dwWormEntries =	dwSpywareEntries = dwAdwareEntries =
////	dwKeyLoggerEntries = dwBHOEntries = dwExplorerEntries =	
////	dwIEEntries = 0x00 ;
////
////	DWORD	dwTotalEntries = 0x12 ;
////	DWORD	dwCurrentEntry = 0x01 ;
////
////	if( (pScanOpt->bActiveX)&&(!m_bIsWow64) )
////	{
////		CheckInvalidActiveXEntries() ;
////		pScanOpt->dwStats[0] = dwActiveXEntries ;
////	}
////	Sleep( 1000 ) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////	//dwScannedBit = dwScannedBit | 1 ;
////
////	if( (pScanOpt->bUninstall) &&(!m_bIsWow64))
////	{
////		CheckInvalidUninstallEntries( ) ;
////		pScanOpt->dwStats[1] = dwUnInstallEntries ;
////	}
////	Sleep( 1000 ) ;
////	//dwPercentage = int(((float)2/16)*100) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////
////	if( (pScanOpt->bFont) &&(!m_bIsWow64))
////	{
////		CheckInvalidFontEntries( ) ;
////		pScanOpt->dwStats[2] = dwFontEntries ;
////	}
////	Sleep( 1000 ) ;
////	//dwPercentage = int(((float)3/16)*100) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////
////	if( (pScanOpt->bSharedLibraries) &&(!m_bIsWow64) )
////	{
////		CheckInvalidSharedLibraries() ;
////		pScanOpt->dwStats[3] = dwSharedDLLs ; 
////	}
////	Sleep( 1000 ) ;
////	//dwPercentage = int(((float)4/16)*100) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////
////	if( (pScanOpt->bApplicationPaths) &&(!m_bIsWow64))
////	{
////		CheckInvalidApplicationPaths() ;
////		pScanOpt->dwStats[4] = dwAppPathEntries ;
////	}
////	Sleep( 1000 ) ;
////	//dwPercentage = int(((float)5/16)*100) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////
////	if( (pScanOpt->bHelpFiles) &&(!m_bIsWow64) )
////	{
////		CheckInvalidHelpFile( ) ;
////		pScanOpt->dwStats[5] = dwHelpFilesEntries ; 
////	}
////	Sleep( 1000 ) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////
////	if( (pScanOpt->bStartup) &&(!m_bIsWow64) )
////	{
////		CheckInvalidStartupEntries( ) ;
////		pScanOpt->dwStats[6] = dwStartupRepairedEntries ; 
////	}
////	Sleep( 1000 ) ;
////	//dwPercentage = int(((float)6/16)*100) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////
////	if( (pScanOpt->bServices)&&(!m_bIsWow64) )
////	{
////		CheckInvalidServices( ) ;
////		pScanOpt->dwStats[7] = dwServicesEntries ; 
////	}
////	Sleep( 1000 ) ;
////	//dwPercentage = int(((float)7/16)*100) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////
////	if( (pScanOpt->bExtensions) &&(!m_bIsWow64) )
////	{
////		CheckInvalidExtensions( ) ;
////		pScanOpt->dwStats[8] = dwExtensionEntries ; 
////	}
////	Sleep( 1000 ) ;
////	//dwPercentage = int(((float)8/16)*100) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////
////
////	if( (pScanOpt->bRootKit) &&(!m_bIsWow64))
////	{
////		CheckRootKitEntries( ) ;
////		pScanOpt->dwStats[9] = dwRootKitEntries ;
////	}
////	Sleep( 1000 ) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////
////	if( (pScanOpt->bRogueApplications)&&(!m_bIsWow64) )
////	{
////		CheckRogueApplications( ) ;
////		pScanOpt->dwStats[10] = dwRogueEntries ;
////	}
////	Sleep( 1000 ) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////
////	if( (pScanOpt->bWorm) &&(!m_bIsWow64))
////	{
////		CheckWormEntries( ) ;
////		pScanOpt->dwStats[11] = dwWormEntries ;
////		//Sleep( 500 ) ;
////	}
////	Sleep( 1000 ) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////
////	if( (pScanOpt->bSpywares) &&(!m_bIsWow64))
////	{
////		CheckInvalidSpywares( ) ;
////		pScanOpt->dwStats[12] = dwSpywareEntries ;
////		//Sleep( 1000 ) ;
////	}
////	Sleep( 1000 ) ;
////	//dwPercentage = int(((float)9/16)*100) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////
////	if( (pScanOpt->bAdwares) &&(!m_bIsWow64) )
////	{
////		CheckInvalidAdwares( ) ;
////		pScanOpt->dwStats[13] = dwAdwareEntries ; 
////		//Sleep( 1000 ) ;
////	}
////	Sleep( 1000 ) ;
////	//dwPercentage = int(((float)10/16)*100) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////
////	if( (pScanOpt->bKeyLogger) &&(!m_bIsWow64) )
////	{
////		CheckKeyLogger() ;
////		pScanOpt->dwStats[14] = dwKeyLoggerEntries ; 
////		//Sleep( 500 ) ;
////	}
////	Sleep( 1000 ) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////
////	if( (pScanOpt->bBHO) &&(!m_bIsWow64))
////	{
////		CheckInvalidBHO( ) ;
////		pScanOpt->dwStats[15] = dwBHOEntries ;
////	}
////	//dwPercentage = int(((float)11/16)*100) ;
////	Sleep( 1000 ) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////
////	if( (pScanOpt->bExplorer) &&(!m_bIsWow64) )
////	{
////		CheckInvalidExplorer( ) ;
////		pScanOpt->dwStats[16] = dwExplorerEntries ;
////	}
////	Sleep( 1000 ) ;
////	//dwPercentage = int(((float)12/16)*100) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////
////	if( (pScanOpt->bIExplorer) &&(!m_bIsWow64) )
////	{
////		CheckInvalidIExplorer( ) ;
////		pScanOpt->dwStats[17] = dwIEEntries ; 
////	}
////	//dwPercentage = int(((float)13/16)*100) ;
////	Sleep( 1000 ) ;
////	dwPercentage = int(((float)dwCurrentEntry/dwTotalEntries)*100)  ;
////	dwCurrentEntry++ ;
////
////	Sleep( 1000 ) ;
////	return 0 ;
////}
//
//
///**********************************************************************************************************                     
//*  Function Name  :	InitializeVariables                                                     
//*  Description    :	
//*  Author Name    : Vilas & Prajakta                                                                                          
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
////void InitializeVariables()
////{
////	m_bIsWow64 = FALSE ;
////	bVistaOnward = false ;
////	
////	dwPercentage = 0x00 ;
////	dwScannedBit = 0x00 ;
////	
////	dwActiveXEntries = dwUnInstallEntries = dwFontEntries =
////	dwSharedDLLs = dwAppPathEntries = dwHelpFilesEntries =
////	dwStartupRepairedEntries = dwServicesEntries =	
////	dwExtensionEntries = dwRootKitEntries =	dwRogueEntries =
////	dwWormEntries =	dwSpywareEntries = dwAdwareEntries =
////	dwKeyLoggerEntries = dwBHOEntries = dwExplorerEntries =	
////	dwIEEntries = 0x00 ;
////	
////	OSVERSIONINFO	OSVI = {0} ;
////
////	OSVI.dwOSVersionInfoSize = sizeof( OSVERSIONINFO ) ;
////	GetVersionEx( &OSVI ) ;
////
////	if( OSVI.dwMajorVersion > 5 )
////		bVistaOnward = true ;
////
////	IsWow64() ;
////
////	GetWindowsDirectory( szWindowsDir, 255 ) ;
////	GetSystemDirectory(szSystemDir, 255 ) ;
////
////	if( m_bIsWow64 )
////	{
////		GetEnvironmentVariable(TEXT("PROGRAMFILES(X86)"), szProgramDirX86, 255 ) ;
////		_wcsupr_s( szProgramDirX86, wcslen(szProgramDirX86)*sizeof(TCHAR) ) ;
////
////		ExpandEnvironmentStrings(L"%ProgramW6432%", szProgramDir, 255 ) ;
////	}
////	else
////		GetEnvironmentVariable(TEXT("PROGRAMFILES"), szProgramDir, 255 ) ;
////
////	GetEnvironmentVariable(TEXT("ALLUSERSPROFILE"), szAppDataPath, 511 ) ;
////	GetEnvironmentVariable(TEXT("COMMONPROGRAMFILES"), szCommProgram, 511 ) ;
////	GetEnvironmentVariable(TEXT("PROGRAMDATA"), szProgramData, 255 ) ;
////	GetEnvironmentVariable(TEXT("USERPROFILE"), szUserProfile, 255 ) ;
////	GetEnvironmentVariable(TEXT("TEMP"), szTempLocal, 255 ) ;
////	GetEnvironmentVariable(TEXT("PUBLIC"), szPublic, 255 ) ;
////	GetEnvironmentVariable(TEXT("APPDATA"), szAppData, 255 ) ;
////
////	if( !bVistaOnward )
////		wcscat(szAppDataPath, TEXT("\\Application Data") ) ;
////
////	GetModuleFileName(NULL, szApplPath, 511 ) ;
////	PathRemoveFileSpec( szApplPath ) ;
////	SetCurrentDirectory( szApplPath ) ;
////
////	_wcsupr_s( szWindowsDir, wcslen(szWindowsDir)*sizeof(TCHAR) ) ;
////	_wcsupr_s( szSystemDir, wcslen(szSystemDir)*sizeof(TCHAR) ) ;
////	_wcsupr_s( szProgramDir, wcslen(szProgramDir)*sizeof(TCHAR) ) ;
////	_wcsupr_s( szApplPath, wcslen(szApplPath)*sizeof(TCHAR) ) ;
////	_wcsupr_s( szAppDataPath, wcslen(szAppDataPath)*sizeof(TCHAR) ) ;
////	_wcsupr_s( szCommProgram, wcslen(szCommProgram)*sizeof(TCHAR) ) ;
////	if( szProgramData[0])
////		_wcsupr_s( szProgramData, wcslen(szProgramData)*sizeof(TCHAR) ) ;
////	_wcsupr_s( szUserProfile, wcslen(szUserProfile)*sizeof(TCHAR) ) ;
////	_wcsupr_s( szTempLocal, wcslen(szTempLocal)*sizeof(TCHAR) ) ;
////	if( szPublic[0] )
////		_wcsupr_s( szPublic, wcslen(szPublic)*sizeof(TCHAR) ) ;
////	_wcsupr_s( szAppData, wcslen(szAppData)*sizeof(TCHAR) ) ;
////
////}
//
///**********************************************************************************************************                     
//*  Function Name  :	IsWow64                                                     
//*  Description    :	
//*  Author Name    : Vilas & Prajakta                                                                                          
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//void IsWow64()
//{
//	typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL) ;
//
//	LPFN_ISWOW64PROCESS		IsWow64Process = NULL ;
//
//	IsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(	GetModuleHandle( TEXT("kernel32")),
//															"IsWow64Process") ;
//	if( !IsWow64Process )
//		return ;
//
//	IsWow64Process( GetCurrentProcess(), &m_bIsWow64 ) ;
//
///*	if( m_bIsWow64 )
//	{
//		Wow64DisableWow64FsRedirection( &OldValue ) ;
//	}
//*/
//}
//
//void FreeUsedResources()
//{
///*	if( m_bIsWow64 )
//		Wow64RevertWow64FsRedirection( OldValue ) ;
//*/
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckInvalidActiveXEntries                                                     
//*  Description    :	Checks for invalid ActiveX Entries
//*  Author Name    : Vilas & Prajakta                                                                                          
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckInvalidActiveXEntries()
//{
//	DWORD	dwRet = 0x00 ;
//
//	HKEY	hKey = NULL, hSubKey = NULL ;
//	DWORD	dwSubKeys = 0x00, dwSize = 0x00, dwType, i = 0x00 ;
//
//	TCHAR	szSubKeyName[512] = {0} ;
//	TCHAR	szTemp[512] = {0} ;
//
//	HKEY	hKey_ActivexPath = NULL ;
//	DWORD	dwSubSubKeys = 0x00 ;
//	TCHAR	szSubSubKeyName[512] = {0} ;
//	TCHAR	szPath[512] = {0} ;
//	TCHAR	szTempData[1024] = {0} ;
//
//	__try
//	{
//		dwActiveXEntries = 0x00 ;
//
//		if( RegOpenKeyEx(	HKEY_CLASSES_ROOT, TEXT("TypeLib"),
//							0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x01 ;
//			goto Cleanup ;
//		}
//
//		if( RegQueryInfoKey(	hKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
//								NULL, NULL, NULL ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x02 ;
//			goto Cleanup ;
//		}
//
//		for(; i<dwSubKeys; i++ )
//		{
//			dwType = dwRet = 0x00 ;
//			dwSize = 511 ;
//			memset(szSubKeyName, 0x00, 512*sizeof(TCHAR) ) ;
//
//			RegEnumKeyEx(hKey, i, szSubKeyName, &dwSize, 0, NULL, NULL, NULL ) ;
//			if( !szSubKeyName[0] )
//				continue ;
//
//			if( RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
//				continue ;
//
//			dwSubSubKeys = 0x00 ;
//			RegQueryInfoKey(hSubKey, NULL, NULL, 0, &dwSubSubKeys, NULL, NULL, NULL, NULL,
//								NULL, NULL, NULL ) ;
//			if(  dwSubSubKeys != 0x01 )
//			{
//				RegCloseKey( hSubKey ) ;
//				hSubKey = NULL ;
//				continue ;
//			}
//
//			dwSize = 511 ;
//			memset(szSubSubKeyName, 0x00, 512*sizeof(TCHAR) ) ;
//			RegEnumKeyEx(hSubKey, 0, szSubSubKeyName, &dwSize, 0, NULL, NULL, NULL ) ;
//			if(  !szSubSubKeyName[0] )
//			{
//				RegCloseKey( hSubKey ) ;
//				hSubKey = NULL ;
//				continue ;
//			}
//
//			memset(szTemp, 0x00, 512*sizeof(TCHAR) ) ;
//			wsprintf(szTemp, TEXT("TypeLib\\%s\\%s\\0\\Win32"), szSubKeyName, szSubSubKeyName ) ;
//			if( RegOpenKeyEx(	HKEY_CLASSES_ROOT, szTemp,
//							0, KEY_ALL_ACCESS, &hKey_ActivexPath ) != ERROR_SUCCESS )
//			{
//				RegCloseKey( hSubKey ) ;
//				hSubKey = NULL ;
//				continue ;
//			}
//
//			dwSize = 511 ;
//			memset(szPath, 0x00, 512*sizeof(TCHAR) ) ;
//			RegQueryValueEx(hKey_ActivexPath, TEXT(""), 0, &dwType, (LPBYTE)szPath, &dwSize) ;
//			if( (szPath[0]) && (szPath[1] == ':') )
//				//dwRet = CheckForPathExists( szPath ) ;
//				dwRet = CheckServiceValidPath(hKey_ActivexPath, szPath ) ;
//
//			RegCloseKey( hKey_ActivexPath ) ;
//			hKey_ActivexPath = NULL ;
//
//			RegCloseKey( hSubKey ) ;
//			hSubKey = NULL ;
//
//			//dwRet = 0x01 ;
//			if( dwRet )
//{
//				if( DeleteInvalidKey( hKey, szSubKeyName ) )
//				//if( true )
//				{
//					memset(szTempData, 0x00, 1024*sizeof(TCHAR) ) ;
//					wsprintf(szTempData, TEXT("COM/ActiveX\tHKEY_CLASSES_ROOT\\%s[%s]"), szTemp, szPath ) ;
//					AddToLog( szTempData ) ;
//					//sprintf(szTemp, "HKLM\\SYSTEM\\CurrentControlSet\\services\\%s", szSubKeyName ) ;
//					//AddToListView("Windows Services", szTemp, "Repaired" ) ;
//				}
//			}
//		}
//
//		goto Cleanup ;
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//
//	}
//
//Cleanup :
//
//	if( hSubKey )
//		RegCloseKey( hSubKey ) ;
//	hSubKey = NULL ;
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckInvalidUninstallEntries                                                     
//*  Description    :	Checks for invalid Uninstall Entries
//*  Author Name    : Vilas & Prajakta                                                                                          
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckInvalidUninstallEntries()
//{
//	DWORD	dwRet = 0x00 ;
//
//	HKEY	hKey = NULL, hSubKey = NULL ;
//	DWORD	dwSubKeys = 0x00, dwSize = 0x00, dwType, i = 0x00 ;
//
//	TCHAR	szSubKeyName[512] = {0} ;
//	TCHAR	szTemp[512] = {0} ;
//	TCHAR	szPath[512] = {0} ;
//	TCHAR	szTempData[1024] = {0} ;
//
//	__try
//	{
//		dwUnInstallEntries = 0x00 ;
//
//		if( RegOpenKeyEx(	HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"),
//							0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x01 ;
//			goto Cleanup ;
//		}
//
//		if( RegQueryInfoKey(	hKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
//								NULL, NULL, NULL ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x02 ;
//			goto Cleanup ;
//		}
//
//		for(; i<dwSubKeys; i++ )
//		{
//			dwType = dwRet = 0x00 ;
//			dwSize = 511 ;
//			memset(szSubKeyName, 0x00, 512*sizeof(TCHAR) ) ;
//
//			RegEnumKeyEx(hKey, i, szSubKeyName, &dwSize, 0, NULL, NULL, NULL ) ;
//			if( !szSubKeyName[0] )
//				continue ;
//
//			if( RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
//				continue ;
//
//			dwSize = 511 ;
//			memset(szPath, 0x00, 512*sizeof(TCHAR) ) ;
//			RegQueryValueEx(hSubKey, TEXT("DisplayName"), 0, &dwType, (LPBYTE)szPath, &dwSize) ;
//			dwRet = wcslen( szPath ) ;
//
//			RegCloseKey( hSubKey ) ;
//			hSubKey = NULL ;
//
//			if( !dwRet )
//			{
//				dwUnInstallEntries++ ;
//				if( DeleteInvalidKey( hKey, szSubKeyName ) )
//				//if( true )
//				{
//					memset(szTempData, 0x00, 1024*sizeof(TCHAR) ) ;
//					wsprintf(szTempData, TEXT("Uninstall_Entries\tHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\%s"), szSubKeyName ) ;
//					AddToLog( szTempData ) ;
//
//					//sprintf(szTemp, "HKLM\\SYSTEM\\CurrentControlSet\\services\\%s", szSubKeyName ) ;
//					//AddToListView("Windows Services", szTemp, "Repaired" ) ;
//				}
//			}
//		}
//
//
//		goto Cleanup ;
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//
//	}
//
//Cleanup :
//
//	if( hKey )
//		RegCloseKey( hKey ) ;
//	hKey = NULL ;
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckInvalidFontEntries                                                     
//*  Description    :	Checks for invalid Font Entries
//*  Author Name    : Vilas & Prajakta                                                                                          
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckInvalidFontEntries()
//{
//	DWORD	dwRet = 0x00 ;
//
//	HKEY	hKey = NULL, hSubKey = NULL ;
//	DWORD	dwValues = 0x00, dwSize = 0x00, dwType, i = 0x00 ;
//
//	TCHAR	szSubKeyName[512] = {0} ;
//	TCHAR	szTemp[512] = {0} ;
//	TCHAR	szPath[512] = {0} ;
//	TCHAR	szTempData[1024] = {0} ;
//	TCHAR	szValueName[512] = {0} ;
//	TCHAR	szValueData[512] = {0} ;
//
//	DWORD	dwValueName, dwValueData ;
//
//	__try
//	{
//		dwFontEntries = 0x00 ;
//
//		if( RegOpenKeyEx(	HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"),
//							0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x01 ;
//			goto Cleanup ;
//		}
//
//		RegQueryInfoKey(hKey, NULL, NULL, 0, NULL, NULL, NULL, &dwValues, NULL,
//						NULL, NULL, NULL ) ;
//		if( dwValues == 0x00 )
//		{
//			dwRet = 0x02 ;
//			goto Cleanup ;
//		}
//
//		for(i=0; i<dwValues; i++ )
//		{
//			memset(szValueName, 0x00, 512*sizeof(TCHAR) ) ;
//			memset(szValueData, 0x00, 512*sizeof(TCHAR) ) ;
//
//			dwValueName = dwValueData = 511 ;
//
//			RegEnumValue(	hKey, i, szValueName, &dwValueName, 0, &dwType, 
//							(LPBYTE)szValueData, &dwValueData) ;
//			if( !szValueName[0] )
//				continue ;
//
//			if( PathFileExists(szValueData ) )
//				continue ;
//
//			if( wcsrchr(szValueData, '\\' ) )
//				continue ;
//
//			memset(szTemp, 0x00, 512*sizeof(TCHAR) ) ;
//			swprintf(szTemp, TEXT("%s\\Fonts\\%s"), szWindowsDir, szValueData ) ;
//
//			dwRet = CheckForPathExists( szTemp ) ;
//			if( dwRet )
//			{
//				//Delete value name
//				memset(szTempData, 0x00, 1024*sizeof(TCHAR) ) ;
//				swprintf(szTempData, TEXT("Font_Entries\tHKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts\\%s[%s]"), szValueName, szTemp ) ;
//				AddToLog( szTempData ) ;
//				//AddToListView(pEntryName, szTemp, TEXT("Repaired") ) ;
//
//				RegDeleteValue( hSubKey, szValueName ) ;
//				dwFontEntries++ ;
//			}
//			
//		}
//
//		goto Cleanup ;
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//
//	}
//
//Cleanup :
//
//	if( hKey )
//		RegCloseKey( hKey ) ;
//
//	hKey = NULL ;
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckInvalidSharedLibraries                                                     
//*  Description    :	Checks for invalid Shared Libraries
//*  Author Name    : Vilas & Prajakta                                                                                          
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckInvalidSharedLibraries( )
//{
//
//	DWORD	dwRet = 0x00 ;
//	TCHAR	szTemp[128] = {0} ;
//
//	__try
//	{
//		dwSharedDLLs = 0x00 ;
//		EnumRegValueNameForPathExist( TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\SharedDLLs"),
//										TEXT("SharedDLLs_Entries"), dwSharedDLLs) ;
//
//		goto Cleanup ;
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//	}
//
//Cleanup :
//
//	return dwRet ;
//}
//
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckInvalidApplicationPaths                                                     
//*  Description    :	Checks for invalid application paths
//*  Author Name    : Vilas & Prajakta                                                                                          
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckInvalidApplicationPaths()
//{
//	
////dwAppPathEntries = 0x00 ;
////HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths
//		
//	DWORD	dwRet = 0x00 ;
//
//	HKEY	hKey = NULL, hSubKey = NULL ;
//	DWORD	dwSubKeys = 0x00, dwSize = 0x00, dwType, i = 0x00 ;
//
//	TCHAR	szSubKeyName[512] = {0} ;
//	TCHAR	szTemp[512] = {0} ;
//	TCHAR	szPath[512] = {0} ;
//	TCHAR	szTempData[1024] = {0} ;
//
//	//bShowMsg = true ;
//
//	__try
//	{
//		dwAppPathEntries = 0x00 ;
//
//		if( RegOpenKeyEx(	HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths"),
//							0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x01 ;
//			goto Cleanup ;
//		}
//
//		if( RegQueryInfoKey(	hKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
//								NULL, NULL, NULL ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x02 ;
//			goto Cleanup ;
//		}
//
//		for(; i<dwSubKeys; i++ )
//		{
//			dwType = dwRet = 0x00 ;
//			dwSize = 511 ;
//			memset(szSubKeyName, 0x00, 512*sizeof(TCHAR) ) ;
//			memset(szTemp, 0x00, 512*sizeof(TCHAR) ) ;
//
//			RegEnumKeyEx(hKey, i, szSubKeyName, &dwSize, 0, NULL, NULL, NULL ) ;
//			if( !szSubKeyName[0] )
//				continue ;
//
//			if( RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
//				continue ;
//
//			dwSize = 511 ;
//			memset(szPath, 0x00, 512*sizeof(TCHAR) ) ;
//			RegQueryValueEx(hSubKey, TEXT(""), 0, &dwType, (LPBYTE)szPath, &dwSize) ;
//			if( szPath[0] )
//				dwRet = CheckServiceValidPath( hSubKey, szPath ) ;
//			else
//				dwRet = 0x01 ;
//
//			RegCloseKey( hSubKey ) ;
//			hSubKey = NULL ;
//
//			if( dwRet )
//			{
//				dwAppPathEntries++ ;
//				if( DeleteInvalidKey( hKey, szSubKeyName ) )
//				//if( true )
//				{
//					memset(szTempData, 0x00, 1024*sizeof(TCHAR) ) ;
//					wsprintf(szTempData, TEXT("AppPath_Entries\tHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\%s[%s]"), szSubKeyName, szPath ) ;
//					AddToLog( szTempData ) ;
//
//					//sprintf(szTemp, "HKLM\\SYSTEM\\CurrentControlSet\\services\\%s", szSubKeyName ) ;
//					//AddToListView("Windows Services", szTemp, "Repaired" ) ;
//				}
//			}
//		}
//
//
//		goto Cleanup ;
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//
//	}
//
//Cleanup :
//
//	RegCloseKey( hKey ) ;
//	hKey = NULL ;
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckInvalidHelpFile                                                     
//*  Description    :	Checks for invalid help file
//*  Author Name    : Vilas & Prajakta                                                                                          
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckInvalidHelpFile( )
//{
//	DWORD	dwRet = 0x00 ;
//	TCHAR	szTemp[128] = {0} ;
//
//	__try
//	{
//		dwHelpFilesEntries = 0x00 ;
//		//m_ScanningText.SetWindowTextW( TEXT("Scanning : Invalid Help File entries") ) ;
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//
//	}
//
//Cleanup :
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckInvalidStartupEntries                                                     
//*  Description    :	Checks for invalid startup entries
//*  Author Name    : Vilas & Prajakta                                                                                          
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckInvalidStartupEntries( )
//{
//
//	DWORD	dwRet = 0x00 ;
//	TCHAR	szTemp[128] = {0} ;
//
//	__try
//	{
//
//		dwStartupRepairedEntries = 0x00 ;
//
//		EnumRegValueDataForPathExist( TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), TEXT("Startup_Entries"), dwStartupRepairedEntries ) ;
//		EnumRegValueDataForPathExist( TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"), TEXT("Startup_Entries"), dwStartupRepairedEntries ) ;
//		EnumRegValueDataForPathExist( TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnceEx"), TEXT("Startup_Entries"), dwStartupRepairedEntries ) ;
//
//		EnumRegValueDataForPathExist( TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), TEXT("Startup_Entries"), dwStartupRepairedEntries, HKEY_CURRENT_USER ) ;
//		//EnumRegValueDataForPathExist( TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), TEXT("Startup Entries"), dwStartupRepairedEntries, HKEY_CURRENT_USER ) ;
//		//EnumRegValueDataForPathExist( TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), TEXT("Startup Entries"), dwStartupRepairedEntries, HKEY_CURRENT_USER ) ;
//
//		goto Cleanup ;
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//
//	}
//
//Cleanup :
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckInvalidServices                                                     
//*  Description    :	Checks for invalid windows services
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckInvalidServices( )
//{
//	DWORD	dwRet = 0x00 ;
//
//	HKEY	hKey = NULL, hSubKey = NULL ;
//	DWORD	dwSubKeys = 0x00, dwSize = 0x00, dwType, i = 0x00 ;
//
//	TCHAR	szSubKeyName[512] = {0} ;
//	TCHAR	szTemp[512] = {0} ;
//	TCHAR	szTempData[1024] = {0};
//
//
//	__try
//	{
//
//		dwServicesEntries = 0x00 ;
//
//		if( RegOpenKeyEx(	HKEY_LOCAL_MACHINE, TEXT("SYSTEM\\CurrentControlSet\\services"),
//							0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x01 ;
//			goto Cleanup ;
//		}
//
//		if( RegQueryInfoKey(	hKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
//								NULL, NULL, NULL ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x02 ;
//			goto Cleanup ;
//		}
//
//		for(; i<dwSubKeys; i++ )
//		{
//			dwType = dwRet = 0x00 ;
//			dwSize = 511 ;
//			memset(szSubKeyName, 0x00, 512*sizeof(TCHAR) ) ;
//
//			RegEnumKeyEx(hKey, i, szSubKeyName, &dwSize, 0, NULL, NULL, NULL ) ;
//			if( !szSubKeyName[0] )
//				continue ;
//
//			if( RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
//				continue ;
//
//
//			dwSize = 511 ;
//			memset(szTemp, 0x00, 512*sizeof(TCHAR) ) ;
//			RegQueryValueEx(hSubKey, TEXT("ImagePath"), 0, &dwType, (LPBYTE)szTemp, &dwSize) ;
//			if( szTemp[0] )
//				dwRet = CheckServiceValidPath( hSubKey, szTemp ) ;
//
//			RegCloseKey( hSubKey ) ;
//			hSubKey = NULL ;
//
//			//dwRet = 0x01 ;
//			if( dwRet )
//			{
//				if( DeleteInvalidKey( hKey, szSubKeyName ) )
//				{
//					dwServicesEntries++ ;
//					memset(szTempData, 0x00, 1024*sizeof(TCHAR) ) ;
//					wsprintf(szTempData, TEXT("WinNT_Services\tHKLM\\SYSTEM\\CurrentControlSet\\services\\%s[%s]"), szSubKeyName, szTemp ) ;
//					AddToLog( szTempData ) ;
//
//					//sprintf(szTemp, "HKLM\\SYSTEM\\CurrentControlSet\\services\\%s", szSubKeyName ) ;
//					//AddToListView("Windows Services", szTemp, "Repaired" ) ;
//				}
//			}
//		}
//
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//
//	}
//
//Cleanup :
//
//	RegCloseKey( hKey ) ;
//	hKey = NULL ;
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckInvalidExtensions                                                     
//*  Description    :	Checks for invalid extensions
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckInvalidExtensions( )
//{
//	DWORD	dwRet = 0x00 ;
//
//	HKEY	hKey = NULL, hSubKey = NULL ;
//	DWORD	dwSubKeys = 0x00, dwSize = 0x00, i = 0x00 ;
//
//	TCHAR	szSubKeyName[512] = {0} ;
//	TCHAR	szTemp[1024] = {0} ;
//
//	__try
//	{
//
//		dwExtensionEntries = 0x00 ;
//
//		if( RegOpenKeyEx(HKEY_CURRENT_USER, 
//						TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts"), 
//						0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x01 ;
//			goto Cleanup ;
//		}
//
//		if( RegQueryInfoKey(	hKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
//								NULL, NULL, NULL ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x02 ;
//			goto Cleanup ;
//		}
//
//		for(; i<dwSubKeys; i++ )
//		{
//			dwRet = 0x00 ;
//			dwSize = 511 ;
//			memset(szSubKeyName, 0x00, 512*sizeof(TCHAR) ) ;
//
//			RegEnumKeyEx(hKey, i, szSubKeyName, &dwSize, 0, NULL, NULL, NULL ) ;
//			if( wcsnlen(szSubKeyName, 511) < 0x02 )
//				continue ;
//
//			if( RegOpenKeyEx(hKey, szSubKeyName, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
//				continue ;
//
//			//sprintf(szTemp, "%s\\OpenWithList", szSubKeyName ) ;
//			dwRet = CheckValidExplorerExtension( hSubKey, szSubKeyName ) ;
//
//			RegCloseKey( hSubKey ) ;
//			hSubKey = NULL ;
//
//			if( !dwRet )
//			{
//				if( DeleteInvalidKey( hKey, szSubKeyName ) )
//				//if( true )
//				{
//					dwExtensionEntries++ ;
//					memset(szTemp, 0x00, 1024*sizeof(TCHAR) ) ;
//					wsprintf(szTemp, TEXT("Explorer_extensions\tHKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\%s"), szSubKeyName ) ;
//					AddToLog( szTemp ) ;
//
//					//sprintf(szTemp, "HKCU\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\%s", szSubKeyName ) ;
//					//AddToListView("Explorer extensions", szTemp, "Repaired" ) ;
//				}
//			}
//		}
//
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//
//	}
//
//Cleanup :
//
//	if( hSubKey )
//		RegCloseKey( hSubKey ) ;
//
//	if( hKey )
//		RegCloseKey( hKey ) ;
//
//	hSubKey = NULL ;
//	hKey = NULL ;
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckRootKitEntries                                                     
//*  Description    :	Checks invalid rootkit entries
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckRootKitEntries( )
//{
//	DWORD	dwRet = 0x00 ;
//	TCHAR	szTemp[128] = {0} ;
//
//	__try
//	{
//		dwRootKitEntries = 0x00 ;
//		//m_ScanningText.SetWindowTextW( TEXT("Scanning : RootKit entries") ) ;
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//
//	}
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckRogueApplications                                                     
//*  Description    :	Checks invalid rogue applications
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckRogueApplications( )
//{
//	DWORD	dwRet = 0x00 ;
//	TCHAR	szTemp[128] = {0} ;
//
//	__try
//	{
//		dwRogueEntries = 0x00 ;
//		//m_ScanningText.SetWindowTextW( TEXT("Scanning : Rogue Application entries") ) ;
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//	}
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckWormEntries                                                     
//*  Description    :	Checks invalid worm entries
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckWormEntries( )
//{
//	DWORD	dwRet = 0x00 ;
//	TCHAR	szTemp[128] = {0} ;
//
//	__try
//	{
//		dwWormEntries = 0x00 ;
//		//m_ScanningText.SetWindowTextW( TEXT("Scanning : Worm entries") ) ;
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//	}
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckInvalidSpywares                                                     
//*  Description    :	Checks invalid spyware threats
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckInvalidSpywares()
//{
//	DWORD	dwRet = 0x00 ;
//	TCHAR	szTemp[128] = {0} ;
//
//	__try
//	{
//
//	/*	HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\RunServicesOnce
//		HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\RunServices
//		HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\RunServicesOnce
//		HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\RunServices
//		HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\Explorer\Run
//		HKLM\Software\Microsoft\Windows NT\CurrentVersion\Winlogon\Userinit
//		HKLM\Software\Microsoft\Windows NT\CurrentVersion\Winlogon\Shell
//		HKEY_LOCAL_MACHINE \ Software \ Microsoft \ Windows \ CurrentVersion \ Explorer \ ShellExecuteHooks
//		HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\ShellServiceObjects
//
//		HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\ShellExecuteHooks
//HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\Notify
//HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\AppInit
//HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\Start Page
//HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Browser Helper Objects
//HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\ShellServiceObjectDelayLoad
//HKLM\SYSTEM\CurrentControlSet\Services\WinSock2
//HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon\UserInit
//HKLM\SYSTEM\CurrentControlSet\Control\Session Manager\BootExecute
//
//
//		[HKEY_CLASSES_ROOT\exefile\shell\open\command] ="\"%1\" %*"
//[HKEY_CLASSES_ROOT\comfile\shell\open\command] ="\"%1\" %*"
//[HKEY_CLASSES_ROOT\batfile\shell\open\command] ="\"%1\" %*"
//[HKEY_CLASSES_ROOT\htafile\Shell\Open\Command] ="\"%1\" %*"
//[HKEY_CLASSES_ROOT\piffile\shell\open\command] ="\"%1\" %*"
//[HKEY_LOCAL_MACHINE\Software\CLASSES\batfile\shell\open\command] ="\"%1\" %*"
//[HKEY_LOCAL_MACHINE\Software\CLASSES\comfile\shell\open\command] ="\"%1\" %*"
//[HKEY_LOCAL_MACHINE\Software\CLASSES\exefile\shell\open\command] ="\"%1\" %*"
//[HKEY_LOCAL_MACHINE\Software\CLASSES\htafile\Shell\Open\Command] ="\"%1\" %*"
//[HKEY_LOCAL_MACHINE\Software\CLASSES\piffile\shell\open\command] ="\"%1\" %*"
//	*/
//
//		dwSpywareEntries = 0x00 ;
//
//		if( !SetRegSZValue(	TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), 
//						TEXT("Shell"), TEXT("explorer.exe"), true ) )
//						dwSpywareEntries++ ;
//
//		CheckShellExecuteHooks( TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ShellExecuteHooks"), dwSpywareEntries) ;
//
//		goto Cleanup ;
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//
//	}
//
//Cleanup :
//
//	return dwRet ;
//}
//
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckInvalidAdwares                                                     
//*  Description    :	Checks invalid adware threats
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckInvalidAdwares()
//{
//	DWORD	dwRet = 0x00 ;
//	TCHAR	szTemp[128] = {0} ;
//
//	__try
//	{
//
//	/*
//	[HKEY_LOCAL_MACHINE\Software\Microsoft\Active Setup\Installed Components\KeyName] 
//	StubPath=C:\PathToFile\Filename.exe 
//
//	//need to take care of this n study first
//	HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Internet Explorer\ActiveX Compatibility
//	HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\ShellServiceObjectDelayLoad
//	*/
//		dwAdwareEntries = 0x00 ;
//
//		goto Cleanup ;
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//	}
//
//Cleanup :
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckKeyLogger                                                     
//*  Description    :	Checks invalid keyloggers
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckKeyLogger()
//{
//	DWORD	dwRet = 0x00 ;
//	TCHAR	szTemp[128] = {0} ;
//
//	__try
//	{
//
//		dwKeyLoggerEntries = 0x00 ;
//		//m_ScanningText.SetWindowTextW( TEXT("Scanning : Keyloggers entries") ) ;
//
//		//goto Cleanup ;
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//
//	}
//
//Cleanup :
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckInvalidBHO                                                     
//*  Description    :	Checks invalid browser helper object
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckInvalidBHO()
//{
//	DWORD	dwRet = 0x00 ;
//
//	HKEY	hKey = NULL, hSubKey = NULL ;
//	DWORD	dwSubKeys = 0x00, dwSize = 0x00, dwType, i = 0x00 ;
//
//	TCHAR	szSubKeyName[512] = {0} ;
//	TCHAR	szBHOKey[512] = {0} ;
//	TCHAR	szPath[512] = {0} ;
//	TCHAR	szTempData[1024] = {0} ;
//
//	__try
//	{
//		dwBHOEntries = 0x00 ;
//
//		if( RegOpenKeyEx(	HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects"),
//							0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x01 ;
//			goto Cleanup ;
//		}
//
//		if( RegQueryInfoKey(	hKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
//								NULL, NULL, NULL ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x02 ;
//			goto Cleanup ;
//		}
//
//		for(; i<dwSubKeys; i++ )
//		{
//			dwType = dwRet = 0x00 ;
//			dwSize = 511 ;
//			memset(szSubKeyName, 0x00, 512*sizeof(TCHAR) ) ;
//
//
//			RegEnumKeyEx(hKey, i, szSubKeyName, &dwSize, 0, NULL, NULL, NULL ) ;
//			if( !szSubKeyName[0] )
//				continue ;
//
//			memset(szBHOKey, 0x00, 512*sizeof(TCHAR) ) ;
//			wsprintf(szBHOKey, TEXT("CLSID\\%s\\InProcServer32"), szSubKeyName ) ;
//
//			if( RegOpenKeyEx(HKEY_CLASSES_ROOT, szBHOKey, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
//				continue ;
//
//			dwSize = 511 ;
//			memset(szPath, 0x00, 512*sizeof(TCHAR) ) ;
//			RegQueryValueEx(hSubKey, TEXT(""), 0, &dwType, (LPBYTE)szPath, &dwSize) ;
//			if( szPath[0] )
//				dwRet = CheckForPathExists( szPath ) ;
//
//			RegCloseKey( hSubKey ) ;
//			hSubKey = NULL ;
//
//			//dwRet = 0x01 ;
//			if( dwRet )
//			{
//				if( DeleteInvalidKey( hKey, szSubKeyName ) )
//				//if( true )
//				{
//					memset(szTempData, 0x00, 1024*sizeof(TCHAR) ) ;
//					wsprintf(szTempData, TEXT("BHO_Entries\tHKLM\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Browser Helper Objects\\%s[%s]"), szSubKeyName, szPath ) ;
//					AddToLog( szTempData ) ;
//
//					//sprintf(szTemp, "HKLM\\SYSTEM\\CurrentControlSet\\services\\%s", szSubKeyName ) ;
//					//AddToListView("Windows Services", szTemp, "Repaired" ) ;
//				}
//				dwBHOEntries++ ;
//			}
//		}
//
//		goto Cleanup ;
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//
//	}
//
//Cleanup :
//
//	if( hKey )
//		RegCloseKey( hKey ) ;
//	hKey = NULL ;
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckInvalidExplorer                                                     
//*  Description    :	Checks invalid explorer entries
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckInvalidExplorer()
//{
//	DWORD	dwRet = 0x00 ;
//	TCHAR	szTemp[128] = {0} ;
//
//	__try
//	{
//
//	/*
//	Windows Explorer
//HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\RunMRU
//HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\ComDlg32\OpenSavePidlMRU
//HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\ComDlg32\LastVisitedPidlMRU
//HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\RecentDocs
//HKEY_CURRENT_USER\Software\Classes\Local Settings\Software\Microsoft\Windows\Shell\MuiCache
//HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\UserAssist
//HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\SharedTaskScheduler
//	*/
//
//		dwExplorerEntries = 0x00 ;
//
//		EnumRegValueNameDeleteAll(	TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RunMRU"),
//									TEXT("Explorer_Entries"), dwExplorerEntries, 
//									HKEY_CURRENT_USER ) ;
//
//		EnumRegValueNameDeleteAll(	TEXT("Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\MuiCache"),
//									TEXT("Explorer_Entries"), dwExplorerEntries, 
//									HKEY_CURRENT_USER ) ;
//
//		EnumRegValueNameForDeletion(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\TypedPaths"),
//									TEXT("Explorer_Entries"), dwExplorerEntries,
//									HKEY_CURRENT_USER ) ;
//
//
//
//		EnumRegValueNameForDeletion(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RecentDocs"),
//									TEXT("Explorer_Entries"), dwExplorerEntries,
//									HKEY_CURRENT_USER ) ;
//
//	/*	pHistClrDlg->EnumSubKeyForDeletion(
//						"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\RecentDocs",
//						true ) ;
//	*/
//
//		EnumRegValueNameForDeletion(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\UserAssist"),
//									TEXT("Explorer_Entries"), dwExplorerEntries,
//									HKEY_CURRENT_USER ) ;
//
//		EnumRegValueNameForDeletion(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\OpenSavePidlMRU"),
//									TEXT("Explorer_Entries"), dwExplorerEntries,
//									HKEY_CURRENT_USER ) ;
//
//		EnumRegValueNameForDeletion(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ComDlg32\\LastVisitedPidlMRU"),
//									TEXT("Explorer_Entries"), dwExplorerEntries,
//									HKEY_CURRENT_USER ) ;
//
//		EnumRegValueNameForDeletion(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Paint\\Recent File List"),
//									TEXT("Explorer_Entries"), dwExplorerEntries,
//									HKEY_CURRENT_USER ) ;
//
//		EnumRegValueNameForDeletion(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Regedit"),
//									TEXT("Explorer_Entries"), dwExplorerEntries,
//									HKEY_CURRENT_USER ) ;
//
//		EnumRegValueNameForDeletion(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Applets\\Wordpad\\Recent File List"),
//									TEXT("Explorer_Entries"), dwExplorerEntries,
//									HKEY_CURRENT_USER ) ;
//
//
//		goto Cleanup ;
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//
//	}
//
//Cleanup :
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckInvalidIExplorer                                                     
//*  Description    :	Checks invalid internet explorer entries
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckInvalidIExplorer()
//{
//	DWORD	dwRet = 0x00 ;
//	TCHAR	szTemp[128] = {0} ;
//
//	__try
//	{
//		dwIEEntries = 0x00 ;
//
//		EnumRegValueNameDeleteAll(TEXT("Software\\Microsoft\\Internet Explorer\\TypedUrls"), 
//							TEXT("IExplorer_Entries"), dwIEEntries, HKEY_CURRENT_USER ) ;
//
//		EnumRegValueNameDeleteAll(TEXT("Software\\Microsoft\\Internet Explorer\\IntelliForms\\Storage1"), 
//							TEXT("IExplorer_Entries"), dwIEEntries, HKEY_CURRENT_USER ) ;
//
//		EnumRegValueNameDeleteAll(TEXT("Software\\Microsoft\\Internet Explorer\\IntelliForms\\Storage2"), 
//							TEXT("IExplorer_Entries"), dwIEEntries, HKEY_CURRENT_USER ) ;
//
//		EnumRegValueNameDeleteAll(TEXT("Software\\Microsoft\\MediaPlayer\\Player\\RecentFileList"), 
//							TEXT("IExplorer_Entries"), dwIEEntries, HKEY_CURRENT_USER ) ;
//
//		goto Cleanup ;
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//
//	}
//
//Cleanup :
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckShellExecuteHooks                                                     
//*  Description    :	
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckShellExecuteHooks(TCHAR *pSubKey, DWORD &dwEntries, HKEY hPredKey )
//{
//	DWORD	dwRet = 0x00 ;
//	HKEY	hKey = NULL, hSubKey = NULL ;
//
//	DWORD	i, dwValues = 0x00 ;
//	DWORD	dwValueName, dwValueData, dwType, dwSize ;
//	TCHAR	szValueName[512] = {0}, szPath[512] ;
//	TCHAR	szValueData[512] = {0} ;
//	TCHAR	szTemp[1024] = {0} ;
//
//	__try
//	{
//		if( RegOpenKeyEx( hPredKey, pSubKey, 0, KEY_ALL_ACCESS, &hKey ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x01 ;
//			goto Cleanup ;
//		}
//
//		RegQueryInfoKey(hKey, NULL, NULL, 0, NULL, NULL, NULL, &dwValues, NULL,
//						NULL, NULL, NULL ) ;
//		if( dwValues == 0x00 )
//		{
//			dwRet = 0x02 ;
//			goto Cleanup ;
//		}
//
//		for(i=0; i<dwValues; i++ )
//		{
//			memset(szValueName, 0x00, 512*sizeof(TCHAR) ) ;
//			memset(szValueData, 0x00, 512*sizeof(TCHAR) ) ;
//
//			dwValueName = dwValueData = 511 ;
//			dwType = 0x00 ;
//			RegEnumValue(	hKey, i, szValueName, &dwValueName, 0, &dwType, 
//							(LPBYTE)szValueData, &dwValueData) ;
//			if( !szValueName[0] )
//				continue ;
//
//			memset(szTemp, 0x00, 1024*sizeof(TCHAR) ) ;
//			wsprintf(szTemp, TEXT("CLSID\\%s\\InProcServer32"), szValueName ) ;
//			if( RegOpenKeyEx(HKEY_CLASSES_ROOT, szTemp, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
//				continue ;
//
//			dwType = 0x00 ;
//			dwSize = 511 ;
//			memset(szPath, 0x00, 512*sizeof(TCHAR) ) ;
//			RegQueryValueEx(hSubKey, TEXT(""), 0, &dwType, (LPBYTE)szPath, &dwSize) ;
//			RegCloseKey( hSubKey ) ;
//			hSubKey = NULL ;
//
//			dwRet = CheckForPathExists( szPath ) ;
//			if( dwRet )
//			{
//				//Delete value name
//				memset(szTemp, 0x00, 1024*sizeof(TCHAR) ) ;
//				if( hPredKey == HKEY_CURRENT_USER )
//					swprintf(szTemp, TEXT("Spywares_Entries\tHKCU\\%s[%s]"), pSubKey, szPath ) ;
//				else
//					swprintf(szTemp, TEXT("Spywares_Entries\tHKLM\\%s[%s]"), pSubKey, szPath ) ;
//				AddToLog( szTemp ) ;
//				//AddToListView(pEntryName, szTemp, TEXT("Repaired") ) ;
//
//				RegDeleteValue( hSubKey, szValueName ) ;
//				dwEntries++ ;
//			}
//			
//		}
//
//		//goto Cleanup ;
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//
//	}
//
//Cleanup :
//
//	if( hKey )
//		RegCloseKey( hKey ) ;
//	hKey = NULL ;
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	EnumRegValueDataForPathExist                                                     
//*  Description    :	
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD EnumRegValueDataForPathExist(TCHAR *pSubKey, TCHAR *pEntryName, 
//								   DWORD &dwEntries, HKEY hPredKey )
//{
//
//	DWORD	dwRet = 0x00 ;
//	DWORD	dwValues = 0x00, dwType = 0x00, i ;
//	HKEY	hSubKey = NULL ;
//
//	TCHAR	szValueName[512], szValueData[512], szTemp[1024] ;
//	DWORD	dwValueName, dwValueData ;
//
//	__try
//	{
//
//		if( RegOpenKeyEx(hPredKey, pSubKey, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x01 ;
//			goto Cleanup ;
//		}
//
//		RegQueryInfoKey(hSubKey, NULL, NULL, 0, NULL, NULL, NULL, &dwValues, NULL,
//						NULL, NULL, NULL ) ;
//		if( dwValues == 0x00 )
//		{
//			dwRet = 0x02 ;
//			goto Cleanup ;
//		}
//
//		for(i=0; i<dwValues; i++ )
//		{
//			memset(szValueName, 0x00, 512*sizeof(TCHAR) ) ;
//			memset(szValueData, 0x00, 512*sizeof(TCHAR) ) ;
//
//			dwValueName = dwValueData = 511 ;
//
//			RegEnumValue(	hSubKey, i, szValueName, &dwValueName, 0, &dwType, 
//							(LPBYTE)szValueData, &dwValueData) ;
//			if( !szValueName[0] )
//				continue ;
//
//			dwRet = CheckForPathExists( szValueData ) ;
//			if( dwRet )
//			{
//				//Delete value name
//				memset(szTemp, 0x00, 1024*sizeof(TCHAR) ) ;
//				if( hPredKey == HKEY_CURRENT_USER )
//					swprintf(szTemp, TEXT("%s\tHKCU\\%s[%s]"), pEntryName, pSubKey ) ;
//				else
//					swprintf(szTemp, TEXT("%s\tHKLM\\%s[%s]"), pEntryName, pSubKey ) ;
//				//AddToListView(pEntryName, szTemp, TEXT("Repaired") ) ;
//
//				RegDeleteValue( hSubKey, szValueName ) ;
//				dwEntries++ ;
//			}
//			
//		}
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//	}
//
//Cleanup:
//
//	if( hSubKey )
//		RegCloseKey( hSubKey ) ;
//
//	hSubKey = NULL ;
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	EnumRegValueNameForPathExist                                                     
//*  Description    :	
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD EnumRegValueNameForPathExist(TCHAR *pSubKey, TCHAR *pEntryName, 
//								   DWORD &dwEntries, HKEY hPredKey )
//{
//
//	DWORD	dwRet = 0x00 ;
//	DWORD	dwValues = 0x00, dwType = 0x00, i ;
//	HKEY	hSubKey = NULL ;
//
//	TCHAR	szValueName[512], szValueData[512], szTemp[1024] ;
//	DWORD	dwValueName, dwValueData ;
//
//	__try
//	{
//
//		if( RegOpenKeyEx(hPredKey, pSubKey, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x01 ;
//			goto Cleanup ;
//		}
//
//		RegQueryInfoKey(hSubKey, NULL, NULL, 0, NULL, NULL, NULL, &dwValues, NULL,
//						NULL, NULL, NULL ) ;
//		if( dwValues == 0x00 )
//		{
//			dwRet = 0x02 ;
//			goto Cleanup ;
//		}
//
//		for(i=0; i<dwValues; i++ )
//		{
//			memset(szValueName, 0x00, 512*sizeof(TCHAR) ) ;
//			memset(szValueData, 0x00, 512*sizeof(TCHAR) ) ;
//
//			dwValueName = dwValueData = 511 ;
//
//			RegEnumValue(	hSubKey, i, szValueName, &dwValueName, 0, &dwType, 
//							(LPBYTE)szValueData, &dwValueData) ;
//			if( !szValueName[0] )
//				continue ;
//
//			dwRet = CheckForPathExists( szValueName ) ;
//			if( dwRet )
//			{
//				//Delete value name
//				memset(szTemp, 0x00, 1024*sizeof(TCHAR) ) ;
//				if( hPredKey == HKEY_CURRENT_USER )
//					wsprintf(szTemp, TEXT("%s\tHKCU\\%s[%s]"), pEntryName, pSubKey, szValueName ) ;
//				else
//					wsprintf(szTemp, TEXT("%s\tHKLM\\%s[%s]"), pEntryName, pSubKey, szValueName ) ;
//				AddToLog( szTemp ) ;
//
//			/*	if( hPredKey == HKEY_CURRENT_USER )
//					sprintf(szTemp, "HKCU\\%s", pSubKey ) ;
//				else
//					sprintf(szTemp, "HKLM\\%s", pSubKey ) ;
//				AddToListView("Shared DLLs", szTemp, "Repaired" ) ;
//			*/
//				RegDeleteValue( hSubKey, szValueName ) ;
//				dwEntries++ ;
//			}
//			
//		}
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//	}
//
//Cleanup:
//
//	if( hSubKey )
//		RegCloseKey( hSubKey ) ;
//
//	hSubKey = NULL ;
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	EnumRegValueNameDeleteAll                                                     
//*  Description    :	
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD EnumRegValueNameDeleteAll(TCHAR *pSubKey, TCHAR *EntryName, 
//								DWORD &dwEntries, HKEY hPredKey )
//{
//
//	DWORD	dwRet = 0x00 ;
//	DWORD	dwValues = 0x00, dwType = 0x00, i ;
//	HKEY	hSubKey = NULL ;
//
//	TCHAR	szValueName[512], szValueData[512], szTemp[1024] ;
//	DWORD	dwValueName, dwValueData ;
//
//	__try
//	{
//
//		if( RegOpenKeyEx(hPredKey, pSubKey, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x01 ;
//			goto Cleanup ;
//		}
//
//		RegQueryInfoKey(hSubKey, NULL, NULL, 0, NULL, NULL, NULL, &dwValues, NULL,
//						NULL, NULL, NULL ) ;
//		if( dwValues == 0x00 )
//		{
//			dwRet = 0x02 ;
//			goto Cleanup ;
//		}
//
//		for(i=0; i<dwValues; i++ )
//		{
//			memset(szValueName, 0x00, 512*sizeof(TCHAR) ) ;
//			memset(szValueData, 0x00, 512*sizeof(TCHAR) ) ;
//
//			dwValueName = dwValueData = 511 ;
//
//			RegEnumValue(	hSubKey, i, szValueName, &dwValueName, 0, &dwType, 
//							(LPBYTE)szValueData, &dwValueData) ;
//			if( !szValueName[0] )
//				continue ;
//
//				//Delete value name
//			memset(szTemp, 0x00, 1024*sizeof(TCHAR) ) ;
//			if( hPredKey == HKEY_CURRENT_USER )
//				wsprintf(szTemp, TEXT("%s[HKCU\\%s\\%s"), EntryName, pSubKey, szValueName ) ;
//			else
//				wsprintf(szTemp, TEXT("%s[HKLM\\%s\\%s"), EntryName, pSubKey, szValueName ) ;
//			AddToLog( szTemp ) ;
//
//			RegDeleteValue( hSubKey, szValueName ) ;
//			dwEntries++ ;
//		}
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//	}
//
//Cleanup:
//
//	if( hSubKey )
//		RegCloseKey( hSubKey ) ;
//
//	hSubKey = NULL ;
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	EnumRegValueNameForDeletion                                                     
//*  Description    :	
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD EnumRegValueNameForDeletion(TCHAR *pSubKey, TCHAR *pTypeInfo, DWORD &dwEntries, 
//								  HKEY hPredKey )
//{
//
//	DWORD	dwRet = 0x00 ;
//	DWORD	dwValues = 0x00, dwType = 0x00, i ;
//	HKEY	hSubKey = NULL ;
//
//	TCHAR	szValueName[512], szValueData[512], szTemp[1024] ;
//	DWORD	dwValueName, dwValueData ;
//
//	__try
//	{
//
//		if( RegOpenKeyEx(hPredKey, pSubKey, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
//		{
//			dwRet = 0x01 ;
//			goto Cleanup ;
//		}
//
//		RegQueryInfoKey(hSubKey, NULL, NULL, 0, NULL, NULL, NULL, &dwValues, NULL,
//						NULL, NULL, NULL ) ;
//		if( dwValues == 0x00 )
//		{
//			dwRet = 0x02 ;
//			goto Cleanup ;
//		}
//
//		for(i=0; i<dwValues; i++ )
//		{
//			memset(szValueName, 0x00, 512*sizeof(TCHAR) ) ;
//			memset(szValueData, 0x00, 512*sizeof(TCHAR) ) ;
//
//			dwValueName = dwValueData = 511 ;
//
//			RegEnumValue(	hSubKey, i, szValueName, &dwValueName, 0, &dwType, 
//							(LPBYTE)szValueData, &dwValueData) ;
//			if( !szValueName[0] )
//				continue ;
//
//			dwRet = 0x01 ;
//			if( dwRet )
//			{
//				memset(szTemp, 0x00, 1024*sizeof(TCHAR) ) ;
//				if( hPredKey == HKEY_CURRENT_USER )
//					wsprintf(szTemp, TEXT("HKCU\\%s\\%s"), pSubKey, szValueName ) ;
//				else
//					wsprintf(szTemp, TEXT("HKLM\\%s\\%s"), pSubKey, szValueName ) ;
//				AddToLog( szTemp ) ;
//
//				RegDeleteValue( hSubKey, szValueName ) ;
//				dwEntries++ ;
//			}
//			
//		}
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//	}
//
//Cleanup:
//
//	if( hSubKey )
//		RegCloseKey( hSubKey ) ;
//
//	hSubKey = NULL ;
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckForPathExists                                                     
//*  Description    :	
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckForPathExists(TCHAR *pPath )
//{
//
//	TCHAR	szPath[512] = {0} ;
//	TCHAR	*pTemp = NULL ;
//
//
//	if( pPath[0] == '"' )
//		wcscpy(szPath, &pPath[1] ) ;
//	else
//		wcscpy(szPath, pPath ) ;
//	wcsupr( szPath ) ;
//
//	pTemp = wcsstr(szPath, TEXT(".EXE") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT(".EXE") ) ;
//		*pTemp = '\0' ;
//	}
//
//	pTemp = wcsstr(szPath, TEXT(".SYS") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT(".SYS") ) ;
//		*pTemp = '\0' ;
//	}
//
//	pTemp = wcsstr(szPath, TEXT(".DLL") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT(".DLL") ) ;
//		*pTemp = '\0' ;
//	}
//
//	pTemp = wcsstr(szPath, TEXT(".OCX") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT(".OCX") ) ;
//		*pTemp = '\0' ;
//	}
//
//	if( PathFileExists( szPath ) )
//		return 0 ;
//
//	return 1 ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckServiceValidPath                                                     
//*  Description    :	
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckServiceValidPath( HKEY hSubKey, TCHAR *pPath )
//{
//
//	TCHAR	szPath[512] = {0} ;
//	TCHAR	szTemp[512] = {0} ;
//	TCHAR	*pTemp = NULL ;
//
//	DWORD	dwRet = 0x01 ;
//
//	if( pPath[0] == '"' )
//		wcscpy(szTemp, &pPath[1] ) ;
//	else
//		wcscpy(szTemp, pPath ) ;
//	wcsupr( szTemp ) ;
//	wcscpy(szPath, szTemp ) ;
//
//	if( !wcschr(szTemp, '.' ) )
//		//MessageBox(szPath, TEXT("1") ) ;
//		return 0 ;
//
//	pTemp = wcsstr( szTemp, TEXT("SYSTEM32\\") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT("SYSTEM32\\") ) ;
//		wsprintf(szPath, TEXT("%s\\%s"), szSystemDir, pTemp ) ;
//
//	/*	if( bShowMsg )
//			MessageBox(szPath, TEXT("2") ) ;
//	*/
//	}
//
//	pTemp = wcsstr( szTemp, TEXT("%PROGRAMFILES%\\") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT("%PROGRAMFILES%\\") ) ;
//		wsprintf(szPath, TEXT("%s\\%s"), szProgramDir, pTemp ) ;
//
//	/*
//		if( bShowMsg )
//			MessageBox(szPath, TEXT("3") ) ;
//	*/
//	}
//	if( m_bIsWow64 )
//	{
//		pTemp = wcsstr( szTemp, TEXT("%PROGRAMFILES(X86)%\\") ) ;
//		if( pTemp )
//		{
//			pTemp += wcslen( TEXT("%PROGRAMFILES(X86)%\\") ) ;
//			wsprintf(szPath, TEXT("%s\\%s"), szProgramDirX86, pTemp ) ;
//
//		/*
//			if( bShowMsg )
//				MessageBox(szPath, TEXT("3") ) ;
//		*/
//		}
//	}
//
//	pTemp = wcsstr( szTemp, TEXT("%COMMONPROGRAMFILES%\\") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT("%COMMONPROGRAMFILES%\\") ) ;
//		wsprintf(szPath, TEXT("%s\\%s"), szCommProgram, pTemp ) ;
//
//	/*
//		if( bShowMsg )
//			MessageBox(szPath, TEXT("4") ) ;
//	*/
//	}
//
//	pTemp = wcsstr( szTemp, TEXT("%SYSTEMROOT%\\") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT("%SYSTEMROOT%\\") );
//		wsprintf(szPath, TEXT("%s\\%s"), szWindowsDir, pTemp ) ;
//
//	/*
//		if( bShowMsg )
//			MessageBox(szPath, TEXT("5") ) ;
//	*/
//	}
//
//	pTemp = wcsstr( szTemp, TEXT("%ALLUSERSPROFILE%\\") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT("%ALLUSERSPROFILE%\\") );
//		wsprintf(szPath, TEXT("%s\\%s"), szProgramData, pTemp ) ;
//
//	/*
//		if( bShowMsg )
//			MessageBox(szPath, TEXT("6") ) ;
//	*/
//	}
//
//	pTemp = wcsstr( szTemp, TEXT("%PROGRAMDATA%\\") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT("%PROGRAMDATA%\\") );
//		wsprintf(szPath, TEXT("%s\\%s"), szProgramData, pTemp ) ;
//
//	/*
//		if( bShowMsg )
//			MessageBox(szPath, TEXT("7") ) ;
//	*/
//	}
//
//	pTemp = wcsstr( szTemp, TEXT("%USERPROFILE%\\") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT("%USERPROFILE%\\") );
//		wsprintf(szPath, TEXT("%s\\%s"), szUserProfile, pTemp ) ;
//
//	/*
//		if( bShowMsg )
//			MessageBox(szPath, TEXT("8") ) ;
//	*/
//	}
//
//	pTemp = wcsstr( szTemp, TEXT("%TEMP%\\") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT("%TEMP%\\") );
//		wsprintf(szPath, TEXT("%s\\%s"), szTempLocal, pTemp ) ;
//
//	/*
//		if( bShowMsg )
//			MessageBox(szPath, TEXT("9") ) ;
//	*/
//	}
//
//	pTemp = wcsstr( szTemp, TEXT("%PUBLIC%\\") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT("%PUBLIC%\\") );
//		wsprintf(szPath, TEXT("%s\\%s"), szPublic, pTemp ) ;
//
//	/*
//		if( bShowMsg )
//			MessageBox(szPath, TEXT("10") ) ;
//	*/
//	}
//
//	pTemp = wcsstr( szTemp, TEXT("%APPDATA%\\") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT("%APPDATA%\\") );
//		wsprintf(szPath, TEXT("%s\\%s"), szAppData, pTemp ) ;
//
//	/*
//		if( bShowMsg )
//			MessageBox(szPath, TEXT("11") ) ;
//	*/
//	}
//
//	pTemp = wcsstr( szTemp, TEXT("%WINDIR%\\") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT("%WINDIR%\\") );
//		wsprintf(szPath, TEXT("%s\\%s"), szWindowsDir, pTemp ) ;
//
//	/*
//		if( bShowMsg )
//			MessageBox(szPath, TEXT("12") ) ;
//	*/
//	}
//
//	if( memicmp(pPath, TEXT("\\??\\"), wcslen(TEXT("\\??\\")) ) == 0 )
//		wcscpy(szPath, &szTemp[wcslen(TEXT("\\??\\"))] ) ;
//
//
//	pTemp = wcsstr(szPath, TEXT(".EXE") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT(".EXE") ) ;
//		*pTemp = '\0' ;
//	}
//
//	pTemp = wcsstr(szPath, TEXT(".SYS") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT(".SYS") ) ;
//		*pTemp = '\0' ;
//	}
//
//	pTemp = wcsstr(szPath, TEXT(".DLL") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT(".DLL") ) ;
//		*pTemp = '\0' ;
//	}
//
//	pTemp = wcsstr(szPath, TEXT(".OCX") ) ;
//	if( pTemp )
//	{
//		pTemp += wcslen( TEXT(".OCX") ) ;
//		*pTemp = '\0' ;
//	}
//
///*
//	if( bShowMsg )
//		MessageBox(szPath, TEXT("13") ) ;
//*/
///*	if( m_bIsWow64 )
//		Wow64DisableWow64FsRedirection( &OldValue ) ;
//*/
//	if( PathFileExists( szPath ) )
//		dwRet = 0x00 ;
///*
//	if( m_bIsWow64 )
//		Wow64RevertWow64FsRedirection( OldValue ) ;
//*/
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	CheckValidExplorerExtension                                                     
//*  Description    :	
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD CheckValidExplorerExtension( HKEY hSubKey, TCHAR *pSubKey )
//{
//	HKEY	hSubSubKey = NULL, hOpenWithListKey = NULL ;
//	TCHAR	szSubKeyName[512] = {0} ;
//	TCHAR	szOpenWithListValue[512] = {0} ;
//
//	DWORD	dwSubKeys = 0x00, dwSize = 511 ;
//
//	RegQueryInfoKey( hSubKey, NULL, NULL, 0, &dwSubKeys, NULL, NULL, NULL, NULL,
//								NULL, NULL, NULL ) ;
//	if( dwSubKeys != 0x01 )
//		return 1 ;
//
//
//	RegEnumKeyEx(hSubKey, 0, szSubKeyName, &dwSize, 0, NULL, NULL, NULL ) ;
//	if( wcsicmp(szSubKeyName, TEXT("OpenWithList")) != 0 )
//		return 2 ;
//
//
//	if( RegOpenKeyEx(hSubKey, TEXT("OpenWithList"), 0, KEY_ALL_ACCESS, &hSubSubKey ) != ERROR_SUCCESS )
//		return 3 ;
//
//	DWORD	dwValues = 0x00, dwType = 0x00 ;
//
//	RegQueryInfoKey(	hSubSubKey, NULL, NULL, 0, NULL, NULL, NULL, &dwValues, NULL,
//						NULL, NULL, NULL ) ;
//	if( dwValues != 0x00 )
//	{
//		RegCloseKey( hSubSubKey ) ;
//		return 4 ;
//	}
//
//	dwSize = 511 ;
//	RegQueryValueEx(hOpenWithListKey, TEXT(""), 0, &dwType, (LPBYTE)szOpenWithListValue, &dwSize) ;
//	RegCloseKey( hSubSubKey ) ;
//
//	if( wcslen(szOpenWithListValue) == 0 )
//		return 0 ;
//
//	return 5 ;
//
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	SetRegSZValue                                                     
//*  Description    :	
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//DWORD SetRegSZValue(TCHAR *pSubKey, TCHAR *pValueName, TCHAR *pValueData, bool bCheck, HKEY hPredKey )
//{
//	DWORD	dwRet = 0x00 ;
//
//	HKEY	hSubKey = NULL ;
//	TCHAR	szValueData[512] = {0} ;
//	DWORD	dwSize = 512, dwType = 0x00 ;
//
//	if( RegOpenKeyEx(hPredKey, pSubKey, 0, KEY_ALL_ACCESS, &hSubKey ) != ERROR_SUCCESS )
//	{
//		dwRet = 0x01 ;
//		goto Cleanup ;
//	}
//
//	RegQueryValueEx(hSubKey, pValueName, 0, &dwType, (LPBYTE)szValueData, &dwSize) ;
//	dwSize = wcslen(pValueData)*sizeof(TCHAR) + 2 ;
//	if( bCheck )
//	{
//		if( wcsicmp(szValueData, pValueData) != 0 )
//			dwRet = RegSetValueEx(hSubKey, pValueName, 0, dwType, (LPBYTE)pValueData, dwSize ) ;
//		else
//			dwRet = 0x02 ;
//	}
//	else
//		dwRet = RegSetValueEx(hSubKey, pValueName, 0, dwType, (LPBYTE)pValueData, dwSize ) ;
//
//Cleanup:
//
//	if( hSubKey )
//		RegCloseKey( hSubKey ) ;
//
//	hSubKey = NULL ;
//
//	return dwRet ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	DeleteInvalidKey                                                     
//*  Description    :	
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//BOOL DeleteInvalidKey( HKEY hKeyRoot, LPTSTR lpSubKey )
//{
//    TCHAR	szDelKey[1024] = {0} ;
//
//    lstrcpy(szDelKey, lpSubKey) ;
//
//	return RegDelnodeRecurse(hKeyRoot, szDelKey) ;
//
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	RegDelnodeRecurse                                                     
//*  Description    :	
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//BOOL RegDelnodeRecurse( HKEY hKeyRoot, LPTSTR lpSubKey )
//{
//    LPTSTR		lpEnd ;
//    LONG		lResult ;
//    DWORD		dwSize ;
//    TCHAR		szName[MAX_PATH] = {0} ;
//    HKEY		hKey = NULL ;
//    FILETIME	ftWrite ;
//
//    // First, see if we can delete the key without having
//    // to recurse.
//
//
//    lResult = RegDeleteKey(hKeyRoot, lpSubKey);
//
//    if (lResult == ERROR_SUCCESS) 
//        return TRUE;
//
//    lResult = RegOpenKeyEx (hKeyRoot, lpSubKey, 0, KEY_READ, &hKey);
//
//    if (lResult != ERROR_SUCCESS) 
//    {
//        if (lResult == ERROR_FILE_NOT_FOUND)
//		{
//            //printf("Key not found.\n");
//            return TRUE;
//        } 
//        else
//		{
//            //printf("Error opening key.\n");
//            return FALSE;
//        }
//    }
//
//    // Check for an ending slash and add one if it is missing.
//
//    lpEnd = lpSubKey + lstrlen(lpSubKey) ;
//
//    if (*(lpEnd - 1) != TEXT('\\')) 
//    {
//        *lpEnd =  TEXT('\\') ;
//        lpEnd++;
//        *lpEnd =  TEXT('\0') ;
//    }
//
//    // Enumerate the keys
//
//    dwSize = MAX_PATH ;
//    lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
//                           NULL, NULL, &ftWrite) ;
//
//    if (lResult == ERROR_SUCCESS) 
//    {
//        do
//		{
//
//            lstrcpy (lpEnd, szName) ;
//            if( !RegDelnodeRecurse(hKeyRoot, lpSubKey) )
//			{
//                break;
//            }
//
//            dwSize = MAX_PATH ;
//
//            lResult = RegEnumKeyEx(hKey, 0, szName, &dwSize, NULL,
//                                   NULL, NULL, &ftWrite);
//
//        } while (lResult == ERROR_SUCCESS);
//    }
//
//    lpEnd--;
//    *lpEnd = TEXT('\0');
//
//    RegCloseKey( hKey ) ;
//	hKey = NULL ;
//
//    // Try again to delete the key.
//
//    lResult = RegDeleteKey(hKeyRoot, lpSubKey) ;
//
//    if( lResult == ERROR_SUCCESS )
//        return TRUE ;
//
//    return FALSE ;
//}
//
///**********************************************************************************************************                     
//*  Function Name  :	AddToLog                                                     
//*  Description    :	Adds registry optimization information in log(REGISTRYOPTIMIZER.log)
//*  Author Name    : Vilas & Prajakta                                                                                         
//*  Date           : 16 Nov 2013
//**********************************************************************************************************/
//void AddToLog(TCHAR *pText )
//{
//	TCHAR	szLogPath[512] = {0} ;
//	TCHAR	szTemp[128] = {0} ;
//
//	FILE	*Fp = NULL ;
//
//	SYSTEMTIME	ST = {0} ;
//
//	__try
//	{
//
//		GetLocalTime( &ST ) ;
//	/*	wsprintf(szLogPath, TEXT("%s\\Logs\\RegOpt_%02d%02d%04d.log"), 
//				szAppDataPath, ST.wDay, ST.wMonth, ST.wYear ) ;
//	*/
//		TCHAR szModulePath[MAX_PATH] = {0};
//		GetModuleFileName(NULL, szModulePath, MAX_PATH);
//
//		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
//		szTemp[0] = '\0';
//
//		wsprintf(szLogPath, TEXT("%s\\Log\\REGISTRYOPTIMIZER.log"), szModulePath) ;
//
//		Fp = _wfopen( szLogPath, TEXT("at+") ) ;
//		if( !Fp )
//			goto Cleanup ;
//
//		if( ST.wHour > 12 )
//			wsprintf(szTemp, TEXT("[%02d:%02d:%02d PM]"), (ST.wHour-12), ST.wMinute, ST.wSecond ) ;
//		else
//			wsprintf(szTemp, TEXT("[%02d:%02d:%02d AM]"), ST.wHour, ST.wMinute, ST.wSecond ) ;
//
//		fseek(Fp, 0L, SEEK_END) ;
//		fwprintf(Fp, TEXT("%s\t%s\n"), szTemp, pText ) ;
//
//		fclose(Fp) ;
//		Fp = NULL ;
//
//	}
//	__except( EXCEPTION_EXECUTE_HANDLER )
//	{
//	}
//
//Cleanup:
//
//	if( Fp )
//		fclose( Fp ) ;
//
//	Fp = NULL ;
//
//}

/**********************************************************************************************************                     
*  Function Name  :	PreTranslateMessage                                                     
*  Description    :	Translate window messages before they are dispatched to the TranslateMessage and DispatchMessage Windows functions
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
BOOL CRegistryOptimizerDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if(pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
	{
		return TRUE;
	}
	return CJpegDialog::PreTranslateMessage(pMsg);
}

/**********************************************************************************************************                     
*  Function Name  :	OnBnClickedCheckSelectall                                                     
*  Description    :	On selection of  'Select All'  button, all checkboxes with corresponding entries on UI will be marked 
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CRegistryOptimizerDlg::OnBnClickedCheckSelectall()
{
	/*****************ISSUE NO -81 Neha Gharge 22/5/14 ************************************************/
	int iCheck = m_chkSelectAll.GetCheck();

	if(iCheck)
	{
		m_btnActiveX.SetCheck(true);
		m_btnUninstall.SetCheck(true);
		m_btnFontEnt.SetCheck(true);
		m_btnSharedDll.SetCheck(true);
		m_btnAppPath.SetCheck(true);
		m_btnHelpFileInfo.SetCheck(true);
		m_btnWinStartup.SetCheck(true);
		m_btnWinServices.SetCheck(true);
		m_btnInvalidExt.SetCheck(true);
		m_btnRootkits.SetCheck(true);
		m_btnRogueApp.SetCheck(true);
		m_btnWorms.SetCheck(true);
		m_btnSpyThreats.SetCheck(true);
		m_btnAdwareThreats.SetCheck(true);
		m_btnKeyloggers.SetCheck(true);
		m_btnBHO.SetCheck(true);
		m_btnExplorerEnt.SetCheck(true);
		m_btnInternetExpEnt.SetCheck(true);
	}
	else
	{
		/*m_btnActiveX.SetCheck(true);
		m_btnUninstall.SetCheck(true);
		m_btnFontEnt.SetCheck(false);
		m_btnSharedDll.SetCheck(false);
		m_btnAppPath.SetCheck(true);
		m_btnHelpFileInfo.SetCheck(false);
		m_btnWinStartup.SetCheck(true);
		m_btnWinServices.SetCheck(false);
		m_btnInvalidExt.SetCheck(false);
		m_btnRootkits.SetCheck(true);
		m_btnRogueApp.SetCheck(true);
		m_btnWorms.SetCheck(true);
		m_btnSpyThreats.SetCheck(true);
		m_btnAdwareThreats.SetCheck(true);
		m_btnKeyloggers.SetCheck(true);
		m_btnBHO.SetCheck(true);
		m_btnExplorerEnt.SetCheck(true);
		m_btnInternetExpEnt.SetCheck(true);*/
		m_btnActiveX.SetCheck(false);
		m_btnUninstall.SetCheck(false);
		m_btnFontEnt.SetCheck(false);
		m_btnSharedDll.SetCheck(false);
		m_btnAppPath.SetCheck(false);
		m_btnHelpFileInfo.SetCheck(false);
		m_btnWinStartup.SetCheck(false);
		m_btnWinServices.SetCheck(false);
		m_btnInvalidExt.SetCheck(false);
		m_btnRootkits.SetCheck(false);
		m_btnRogueApp.SetCheck(false);
		m_btnWorms.SetCheck(false);
		m_btnSpyThreats.SetCheck(false);
		m_btnAdwareThreats.SetCheck(false);
		m_btnKeyloggers.SetCheck(false);
		m_btnBHO.SetCheck(false);
		m_btnExplorerEnt.SetCheck(false);
		m_btnInternetExpEnt.SetCheck(false);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	OnPaint                                                     
*  Description    :	The framework calls this member function when Windows or an application makes a request to repaint a portion of an application's window.
*  SR.N0		  : 
*  Author Name    : Vilas & Prajakta                                                                                          
*  Date           : 16 Nov 2013
**********************************************************************************************************/
void CRegistryOptimizerDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	CJpegDialog::Draw();
	CJpegDialog::OnPaint();
}

/**********************************************************************************************************
*  Function Name  :	PauseRegistryOptimizer
*  Description    :	Pause RegistryOptimizer if user clicks on close button
*  SR.N0		  :
*  Author Name    : Varada Ikhar
*  Date           : 24th April 2015
**********************************************************************************************************/
bool CRegistryOptimizerDlg::PauseRegistryOptimizer()
{
	try
	{
		//Varada Ikhar, Date: 28/04/2015
		//Issue : In Registry optimizer, while performing scan, stop, pause, resume operation UI is getting hanged.
		if (m_hRegOptThread)
		{
			if (::SuspendThread(m_hRegOptThread) == -1)
			{
				CString csErrorMsg = L"";
				DWORD ErrorCode = GetLastError();
				csErrorMsg.Format(L"### Failed to suspend ScanRepairRegistryOptimizer thread in CRegistryOptimizerDlg::PauseRegistryOptimizer with GetLastError code %d", ErrorCode);
				AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
			}
		}

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = PAUSE_REGISTRY_OPTIMIZER;

		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data PAUSE_REGISTRY_OPTIMIZER", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CRegistryOptimizerDlg::ShutDownScanning", 0, 0, true, SECONDLEVEL);
		}

		if (!&szPipeData.dwValue)
		{
			return false;
		}
		AddLogEntry(L">>> Registry optimization paused.", 0, 0, true, FIRSTLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistryOptimizerDlg::PauseRegistryOptimizer", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	ResumeRegistryOptimizer
*  Description    :	Resume RegistryOptimizer if user clicks on close button
*  SR.N0		  :
*  Author Name    : Varada Ikhar
*  Date           : 24th April 2015
**********************************************************************************************************/
bool CRegistryOptimizerDlg::ResumeRegistryOptimizer()
{
	try
	{
		//Varada Ikhar, Date: 28/04/2015
		//Issue : In Registry optimizer, while performing scan, stop, pause, resume operation UI is getting hanged.
		if (m_hRegOptThread)
		{
			if (::ResumeThread(m_hRegOptThread) == -1)
			{
				CString csErrorMsg = L"";
				DWORD ErrorCode = GetLastError();
				csErrorMsg.Format(L"### Failed to Resume ScanRepairRegistryOptimizer thread in CRegistryOptimizerDlg::ResumeRegistryOptimizer with GetLastError code %d", ErrorCode);
				AddLogEntry(L"%s", csErrorMsg, 0, true, SECONDLEVEL);
			}
		}

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = RESUME_REGISTRY_OPTIMIZER;

		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data RESUME_REGISTRY_OPTIMIZER", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CRegistryOptimizerDlg::ShutDownScanning", 0, 0, true, SECONDLEVEL);
		}

		if (!&szPipeData.dwValue)
		{
			return false;
		}
		AddLogEntry(L">>> Registry optimization resumed.", 0, 0, true, ZEROLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CRegistryOptimizerDlg::ResumeRegistryOptimizer", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}
