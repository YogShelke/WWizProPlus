#include "stdafx.h"
#include "WardWizRegistration.h"
#include "RegistrationSecondDlg.h"
#include "PipeConstants.h"
#include "WardwizLangManager.h"
#include "WardWizDumpCreater.h"
#include "ITinRegWrapper.h"
#include <cmath>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DLLIMPORT __declspec(dllimport)
#define DLLEXPORT __declspec(dllexport)

#define		IDR_REGDATA			2000

extern AVACTIVATIONINFO	g_ActInfo ;

BEGIN_MESSAGE_MAP(CWardWizRegistrationApp, CWinApp)
END_MESSAGE_MAP()

CWardWizRegistrationApp::CWardWizRegistrationApp()
{
	m_dwProductID = 1;
	m_bAllowDemoEdition = false;
	m_hResDLL = NULL;
	m_objProdInfoPopup = NULL;
	//New Implementation: After 1 Year In Registration Dialog User Info Should Appear byDefault
	//Implementated By : Nitin K. Date : 16th April 2015
	m_bIsProductRenewal = false;
	m_dwSalutation = -1;
	m_csEmailID = L"";
	m_csFirstName = L"";
	m_csLastName = L"";
	m_csContactNo = L"";
	m_csProductKeyNumber = L"";
	m_dwIsOffline = 0;
	m_dwLangID = 0;
}

CWardWizRegistrationApp::~CWardWizRegistrationApp()
{
	if(m_hResDLL !=NULL)
	{
		FreeLibrary(m_hResDLL);
		m_hResDLL = NULL;
	}
	// add by lalit 5-4-2015 memory allocated by new deleted
	if (m_objProdInfoPopup != NULL)
	{
		delete m_objProdInfoPopup;
		m_objProdInfoPopup = NULL;
	}

	if (m_hRegMutexHandle)
	{
		ReleaseMutex(m_hRegMutexHandle);
		CloseHandle(m_hRegMutexHandle);
		m_hRegMutexHandle = NULL;
	}
}
CWardWizRegistrationApp theApp;

BOOL CWardWizRegistrationApp::InitInstance()
{
	CWinApp::InitInstance();
	m_csProdRegKey = CWWizSettingsWrapper::GetProductRegistryKey();
	m_dwProductID = theApp.m_objwardwizLangManager.GetSelectedProductID();
	CheckScanLevel();
	m_bRegEmail = false;
	ProductRenewalDetails();
	CheckIsOffline();
	m_dwLangID = theApp.m_objwardwizLangManager.GetSelectedLanguage();
#ifndef MSOUTLOOK32
	LoadResourceDLL();
	CreateFonts();
#endif
	return TRUE;
}

BOOL CWardWizRegistrationApp::PerformRegistration()
{

	if(SingleInstanceCheck())
	{
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_REGISTRATION_ALREADY_IN_PROCESS"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION);
		return false;
	}
	m_objRegistrationDlg.DoModal();
	return TRUE;
}

BOOL CWardWizRegistrationApp::CloseRegistrationWindow()
{
	return m_objRegistrationDlg.HandleCloseButton()?1:0;
}

//issue - 1211 Click register now button on expiry message box. Exit UI from tray ..crash was occuring
//Neha Gharge 4 jan,2016
DWORD CWardWizRegistrationApp::ShowEvalutionExpiredMsg(bool bShowAtStartUp)
{
	/*	ISSUE NO - Product expired MSG box not coming proper in case of Unregistered product nd Expired product NAME - NITIN K. TIME - 3rd July 2014 */
	if(_tcscmp(g_ActInfo.szEmailID,L"")!=0)
	{
		m_bRegEmail = true;
	}
	else
	{
		m_bRegEmail = false;
	}
	CRegistrationMsgPopupDlg m_objMsgPopup;
	m_objMsgPopup.m_bShowAtStartUp = bShowAtStartUp;
	int Ret = static_cast<int>(m_objMsgPopup.DoModal());
	if (Ret == IDOK)
	{
		return IDOK;
	}
	return IDCANCEL;
}

/***********************************************************************************************
*  Function Name  : GetRegisteredUserInfo
*  Description    : Retrieves user information and display it on UI
*  Author Name    : Nitin Kolapkar
*  SR_NO		  :
*  Date           :  27-May-2016
*************************************************************************************************/
BOOL CWardWizRegistrationApp::GetRegisteredUserInfo(AVACTIVATIONINFO &ActInfo)
{
	try
	{
		CString csEmailID = L"";
		csEmailID.Format(L"%s", g_ActInfo.szEmailID);
		if (csEmailID != L"")
		{
			memcpy(&ActInfo, &g_ActInfo, sizeof(ActInfo));
			//ActInfo = g_ActInfo;
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumRegistrationApp::GetRegisteredUserInfo", 0, 0, true, SECONDLEVEL);
	}
	return false;
}
BOOL CWardWizRegistrationApp::ShowProductInformation()
{
	//Modified Date 10 July,2010 Neha Gharge 
	//Added a data encryption version no.
	CString csEmailID ,csUserFirstName,csUserLastName,csNoofDays,csWholeName,csDataEncVersion;
	// resolve by lalit kumawat 3-17-015
	//issue: -click on settings-> click delete reports->Click on settings button next to "Delete Reports"->Click on wardwiz tray->click exit.The delete report dialog should get close. 
	//(same issue applies to change password & scan computer at start up settings

	CProductInformation *objProdInfoPopup;
	// add by lalit 5-4-2015 memory allocated by new deleted
	if (m_objProdInfoPopup!=NULL)
		delete m_objProdInfoPopup;

	m_objProdInfoPopup = objProdInfoPopup = new CProductInformation();
	
	DWORD dwDaysLeft = GetDaysLeft(theApp.m_dwProductID);
	ReadProductVersion4mRegistry();
	//if(m_objRegistrationDlg.m_objRegistrationSecondDlg.GetRegisteredUserInfo() != 0)

	//if(sizeof(g_ActInfo)!= 0)
	{

		csEmailID.Format(L"       :   %s",g_ActInfo.szEmailID);
		csUserFirstName.Format(L"       :   %s",g_ActInfo.szUserFirstName);
		csUserLastName.Format(L" %s",g_ActInfo.szUserLastName);
		csUserFirstName.AppendFormat(L" %s",csUserLastName);
		csWholeName = csUserFirstName;
		if(csWholeName.GetLength() == 4)
		{
			objProdInfoPopup->m_csWholeName = L"       :   NA";
		}
		else
		{
			objProdInfoPopup->m_csWholeName = csWholeName;
		}
		if(csEmailID.GetLength() == 2)
		{
			objProdInfoPopup->m_csEmailID = L"       :   NA";
		}
		else
		{
			objProdInfoPopup->m_csEmailID = csEmailID;
		}
		//Issue : Information Message box should have OK button & Product Key
		//Resolved by : Nitin K. 
		CString csProdNumber;
		csProdNumber.Format(L"%s", g_ActInfo.szKey);
		if (csProdNumber == L"")
		{
			if (dwDaysLeft)
			{
				csProdNumber.Format(L"       :   %s", theApp.m_objwardwizLangManager.GetString(L"IDS_TRAIL_VERSION"));
				objProdInfoPopup->m_csProductKeyNumber = csProdNumber;
			}
			else
			{
				csProdNumber.Format(L"       :   NA");
				objProdInfoPopup->m_csProductKeyNumber = csProdNumber;
			}
		}
		else
		{
			csProdNumber.Format(L"       :   %s", g_ActInfo.szKey);
			objProdInfoPopup->m_csProductKeyNumber = csProdNumber;
		}
	}	
	if(!dwDaysLeft)
	{
		objProdInfoPopup->m_csNoofDays = L"       :   0";
		CString csProdVersion;
		csProdVersion.Format(L"       :   %s",m_csRegProductVer);
		objProdInfoPopup->m_csVersionNo = m_csRegProductVer;
		objProdInfoPopup->m_csVersionNo = csProdVersion;
		//m_objProdInfoPopup.m_csInstalledEdition = L"Gold";
		//return false;
	}
	else
	{
		csNoofDays.Format(L"       :   %d",dwDaysLeft);
		objProdInfoPopup->m_csNoofDays = csNoofDays;
		CString csProdVersion;
		csProdVersion.Format(L"       :   %s",m_csRegProductVer);
		//m_csRegProductVer.Format(L": %s",m_csRegProductVer);
		objProdInfoPopup->m_csVersionNo = csProdVersion;
	}

	if(m_bAllowDemoEdition)
	{
		objProdInfoPopup->m_csInstalledEdition = L"       :   WardWiz Essential Demo";
	}
	else
	{
		switch(m_dwProductID)
		{
			case ESSENTIAL:
				objProdInfoPopup->m_csInstalledEdition = L"       :   WardWiz Essential";
				break;
			case PRO:
				objProdInfoPopup->m_csInstalledEdition = L"       :   WardWiz Pro";
				break;
			case ELITE:
				objProdInfoPopup->m_csInstalledEdition = L"       :   WardWiz Elite";
				break;
			case BASIC:
				objProdInfoPopup->m_csInstalledEdition = L"       :   WardWiz Basic";
				break;

			default:  
				objProdInfoPopup->m_csInstalledEdition = L"    :   NA";
				AddLogEntry(L"### Invalid product edition", 0, 0, true, SECONDLEVEL);
				break;
		}
	}

	csDataEncVersion.Format(L"    :   %s", m_csDataEncVer);
	objProdInfoPopup->m_csDataEncVersion = csDataEncVersion;

	objProdInfoPopup->DoModal();
	return TRUE;
}

extern "C" DLLEXPORT DWORD GetDaysLeft(DWORD dwProdID)
{
	__try
	{
		return theApp.GetDaysLeft(dwProdID);
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in GetDaysLeft", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}


extern "C" DLLEXPORT bool CloseRegistrationWindow()
{
	return theApp.CloseRegistrationWindow() ? true : false;
}

extern "C" DLLEXPORT bool PerformRegistrationUsingSetup()
{
	__try
	{
		if(theApp.GetDaysLeft(theApp.m_dwProductID) != 0)
		{
			return false;
		}
		return theApp.PerformRegistration() ? true : false;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in PerformRegistrationUsingSetup", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

extern "C" DLLEXPORT bool PerformRegistration()
{
	return theApp.PerformRegistration() ? true : false;
}

//issue - 1211 Click register now button on expiry message box. Exit UI from tray ..crash was occuring
//Neha Gharge 4 jan,2016
extern "C" DLLEXPORT DWORD ShowEvalutionExpiredMsg(bool bShowAtStartUp)
{
	return theApp.ShowEvalutionExpiredMsg(bShowAtStartUp);
}

extern "C" DLLEXPORT bool ShowProductInformation()
{
	return theApp.ShowProductInformation() ? true : false;
}

extern "C" DLLEXPORT bool GetRegisteredUserInfo(AVACTIVATIONINFO &ActInfo)
{
	return theApp.GetRegisteredUserInfo(ActInfo) ? true : false;
}
// resolve by lalit kumawat 3-17-015
//issue: -click on settings-> click delete reports->Click on settings button next to "Delete Reports"->Click on wardwiz tray->click exit.The delete report dialog should get close. 
//(same issue applies to change password & scan computer at start up settings
/***************************************************************************
Function Name  : CloseProductInformationDlg()
Description    : export function to close abut dlg.
Author Name    : lalit
SR_NO   	   :
Date           : 
****************************************************************************/
extern "C" DLLEXPORT void CloseProductInformationDlg()
{
	if (theApp.m_objProdInfoPopup != NULL)
	{
		::SendMessage(theApp.m_objProdInfoPopup->m_hWnd, WM_CLOSE, 0, 0);
	}
}
extern "C" DLLEXPORT DWORD GetDaysLeft_Service()
{
	DWORD dwRet = 0;
	__try
	{
		dwRet = theApp.GetDaysLeft_Service() ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in GetDaysLeft_Service");
	}
	return dwRet;
}

DWORD CWardWizRegistrationApp::GetDaysLeft_Service()
{
	DWORD	dwDays = 0x00 ;

	try
	{
		bool	bUsedDate = false ;

		CRegistrationDlg	objRegistrationDlg ;
		AddLogEntry(L">>> CVibraniumRegistrationApp::GetDaysLeft_Service", 0, 0, true, ZEROLEVEL);
		if( objRegistrationDlg.m_objRegistrationSecondDlg.GetRegisteredUserInfo(theApp.m_dwProductID))
		{
			return dwDays ;
		}
		else
		{
			dwDays = GetNumberOfDaysLeft(&objRegistrationDlg, &bUsedDate);
			if( bUsedDate )
			{
				AVACTIVATIONINFO		ActInfo_Update = {0} ;
				CRegistrationSecondDlg	SecondDlg ;

				memcpy( &ActInfo_Update, &g_ActInfo, sizeof(ActInfo_Update) ) ;

				if( SecondDlg.DecryptData( (LPBYTE)&ActInfo_Update, sizeof(ActInfo_Update ) ) )
				{
					AddLogEntry(L"### Failed to DecryptData in GetDaysLeft_Service", 0, 0, true, SECONDLEVEL);
					goto FAILED;
				}

				if( AddToEvalRegDll( (LPBYTE)&ActInfo_Update, sizeof(ActInfo_Update ) ) )
				{
					AddLogEntry(L"### Failed to AddToEvalRegDll in GetDaysLeft_Service", 0, 0, true, SECONDLEVEL);
					goto FAILED;
				}

				if( AddToUserDB( (LPBYTE)&ActInfo_Update, sizeof(ActInfo_Update ) ) )
				{
					AddLogEntry(L"### Failed to AddToEvalRegDll in GetDaysLeft_Service", 0, 0, true, SECONDLEVEL);
					goto FAILED;
				}

				if( AddRegistrationDataInRegistry( (LPBYTE)&ActInfo_Update, sizeof(ActInfo_Update ) ) )
				{
					AddLogEntry(L"### Failed to AddToEvalRegDll in GetDaysLeft_Service", 0, 0, true, SECONDLEVEL);
					goto FAILED;
				}

				SpreadRegistrationFilesInSystem() ;
			}

		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumRegistrationApp::GetDaysLeft", 0, 0, true, SECONDLEVEL);
		return 0;
	}

FAILED :
	return dwDays ;
}

DWORD CWardWizRegistrationApp::GetDaysLeft(DWORD dwProdID)
{
	DWORD	dwDays = 0x00 ;

	try
	{
		bool	bUsedDate = false ;

		CRegistrationDlg	objRegistrationDlg ;

		if( objRegistrationDlg.m_objRegistrationSecondDlg.GetRegisteredUserInfo(dwProdID))
		{
			return dwDays ;
		}
		else
		{
			dwDays = GetNumberOfDaysLeft(&objRegistrationDlg ) ;//, bUsedDate);
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumRegistrationApp::GetDaysLeft", 0, 0, true, SECONDLEVEL);
		return 0;
	}
	return dwDays ;
}


DWORD CWardWizRegistrationApp::GetNumberOfDaysLeft(CRegistrationDlg *objRegistrationDlg, bool *bUsedDate)
{
	DWORD	dwDays = 0x00 ;
	__try
	{
		if(!objRegistrationDlg)
		{
			return dwDays;
		}

		memcpy(&g_ActInfo, &objRegistrationDlg->m_objRegistrationSecondDlg.m_ActInfo, 
			sizeof(objRegistrationDlg->m_objRegistrationSecondDlg.m_ActInfo));

		SYSTEMTIME		CurrTime = {0} ;

		GetSystemTime( &CurrTime ) ;
		CTime	Time_Reg( g_ActInfo.RegTime ) ;
		CTime	Time_Curr( CurrTime ) ;

		if( Time_Reg > Time_Curr )
			return dwDays ;

		if( g_ActInfo.dwDaysUsed >= g_ActInfo.dwTotalDays )
				return dwDays ;


		if( Time_Curr > Time_Reg )
		{
			CTimeSpan	Time_Diff = Time_Curr - Time_Reg ;

			if( g_ActInfo.dwDaysUsed )
			{
				if( g_ActInfo.dwDaysUsed > Time_Diff.GetDays() )
					return dwDays ;
			}

			if( g_ActInfo.dwDaysUsed == g_ActInfo.dwTotalDays )
				return dwDays ;

			if( Time_Diff.GetDays() >= g_ActInfo.dwTotalDays )
			{
				dwDays = 0x00 ;
				if( bUsedDate )
				{
					if( g_ActInfo.dwDaysUsed != g_ActInfo.dwTotalDays )
					{
						g_ActInfo.dwDaysUsed = g_ActInfo.dwTotalDays ;
						*bUsedDate = true ;
					}
				}
			}
			else
			{
				dwDays = g_ActInfo.dwTotalDays - static_cast<DWORD>(Time_Diff.GetDays());

				if( bUsedDate )
				{
					if( g_ActInfo.dwDaysUsed != Time_Diff.GetDays() )
					{
						g_ActInfo.dwDaysUsed = static_cast<DWORD>(Time_Diff.GetDays());
						*bUsedDate = true ;
					}
				}
			}
		}

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		dwDays = 0x00 ;		
	}

	return dwDays;
}

DWORD CWardWizRegistrationApp::AddToEvalRegDll(LPBYTE pActInfo, DWORD dwSize)
{

	typedef DWORD (*ADDREGISTRATIONDATA) (LPBYTE, DWORD, DWORD dwResType, TCHAR *pResName  ) ;
	ADDREGISTRATIONDATA		AddRegistrationData = NULL ;

	DWORD	dwRet = 0x00 ;
	DWORD	dwRegUserSize = 0x00 ;

	HMODULE	m_RegisterationDLL = NULL ;

	CString	strEvalRegDLL("") ;
	CString	strRegisterDLL("") ;
	CString	strAppPath("") ;
	TCHAR	szTemp[512] = {0} ;
	

	//CString	strAppPath = GetModuleFilePath() ;

	GetModuleFileName( NULL, szTemp, 511 ) ;
	TCHAR	*pTemp = wcsrchr(szTemp, '\\' ) ;
	if( pTemp )
	{
		pTemp++ ;
		*pTemp = '\0' ;
	}

	strAppPath.Format( L"%s", szTemp ) ;

	strEvalRegDLL.Format( TEXT("%sVBEVALREG.DLL"), strAppPath ) ;
	if( !PathFileExists( strEvalRegDLL) )
	{
		dwRet = 0x01 ;
		AddLogEntry(L"### VBEVALREG.DLL not found in CRegistrationSecondDlg GetRegisteredUserInfo()", 0, 0, true, SECONDLEVEL);
		goto Cleanup;
	}

	strRegisterDLL.Format( TEXT("%sVBREGISTERDATA.DLL"), strAppPath ) ;
	if( !PathFileExists( strRegisterDLL) )
	{
		dwRet = 0x02 ;
		AddLogEntry(L"### VBREGISTERDATA.DLL not found in CRegistrationSecondDlg GetRegisteredUserInfo()", 0, 0, true, SECONDLEVEL);
		goto Cleanup;
	}

	m_RegisterationDLL = LoadLibrary( strRegisterDLL ) ;
	AddRegistrationData = (ADDREGISTRATIONDATA ) GetProcAddress(m_RegisterationDLL, "AddRegisteredData") ;
	if( !AddRegistrationData )
	{
		dwRet = 0x04 ;
		AddLogEntry(L"### VBREGISTERDATA.DLL version is incorrect in CRegistrationSecondDlg GetRegisteredUserInfo()", 0, 0, true, SECONDLEVEL);
		goto Cleanup;
	}

	dwRegUserSize = dwSize ;

	if( AddRegistrationData((LPBYTE)pActInfo, dwRegUserSize , IDR_REGDATA, L"REGDATA" ) == 0)
	{
		dwRet = 0x00 ;
		goto Cleanup;
	}

Cleanup:

	if( m_RegisterationDLL )
		FreeLibrary( m_RegisterationDLL ) ;

	m_RegisterationDLL = NULL ;
	
	return dwRet ;
}

DWORD CWardWizRegistrationApp::AddToUserDB(LPBYTE pActInfo, DWORD dwSize )
{
	DWORD	dwRet = 0x00, dwBytesWrite = 0x00 ;
	HANDLE	hFileEnc = INVALID_HANDLE_VALUE ;

	TCHAR	szTemp[512] = {0} ;

	GetModuleFileName( NULL, szTemp, 511 ) ;
	TCHAR	*pTemp = wcsrchr(szTemp, '\\' ) ;
	if( pTemp )
	{
		pTemp++ ;
		*pTemp = '\0' ;
	}

	wcscat( szTemp, L"VBUSERREG.DB" ) ;
	hFileEnc = CreateFile(	szTemp, GENERIC_READ|GENERIC_WRITE, 0, NULL,
								OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL ) ;
	if( hFileEnc == INVALID_HANDLE_VALUE )
	{
		dwRet = 0x01 ;
		goto Clean ;
	}
	
	SetFilePointer(hFileEnc, 0x00, 0x00, FILE_BEGIN ) ;
	WriteFile( hFileEnc, pActInfo, dwSize, &dwBytesWrite, NULL ) ;
	if( dwSize != dwBytesWrite )
	{
		dwRet = 0x02 ;
		goto Clean ;
	}

Clean :

	if( hFileEnc != INVALID_HANDLE_VALUE )
		CloseHandle( hFileEnc ) ;

	hFileEnc = INVALID_HANDLE_VALUE ;

	return dwRet ;
}

DWORD CWardWizRegistrationApp::AddRegistrationDataInRegistry(LPBYTE pActInfo, DWORD dwSize)
{
	DWORD	dwRet = 0x00 ;
	DWORD	dwRegType = 0x00, dwRetSize = 0x00 ;

	HKEY	h_iSpyAV = NULL ;

	if( RegCreateKeyEx(	HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows"), 
						0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
						NULL, &h_iSpyAV, NULL ) != ERROR_SUCCESS )
	{
		dwRet = 0x01 ;
		goto Cleanup ;
	}

	/*if( RegSetValueEx(h_iSpyAV, TEXT("UserInfo"), 0, REG_BINARY, (LPBYTE)pActInfo, 
						dwSize ) != ERROR_SUCCESS )
	{
		dwRet = 0x03 ;
		goto Cleanup ;
	}*/
	if( RegSetValueEx(h_iSpyAV, TEXT("VibraniumUserInfo"), 0, REG_BINARY, (LPBYTE)pActInfo, 
						dwSize ) != ERROR_SUCCESS )
	{
		dwRet = 0x03 ;
		goto Cleanup ;
	}

Cleanup:

	if( h_iSpyAV )
		RegCloseKey( h_iSpyAV ) ;

	h_iSpyAV = NULL ;

	return dwRet ;
}


DWORD CWardWizRegistrationApp::SpreadRegistrationFilesInSystem( )
{
	TCHAR	szAllUserPath[512] = {0} ;

	TCHAR	szSource[512] = {0} ;
	TCHAR	szSource1[512] = {0} ;
	TCHAR	szDestin[512] = {0} ;
	TCHAR	szDestin1[512] = {0} ;

	OSVERSIONINFO 	OSVer = {0} ;

	GetEnvironmentVariable( L"ALLUSERSPROFILE", szAllUserPath, 511 ) ;

	OSVer.dwOSVersionInfoSize = sizeof( OSVERSIONINFO ) ;

	GetVersionEx( &OSVer ) ;

	if( OSVer.dwMajorVersion > 5 )
	{
		wsprintf( szDestin, L"%s\\Wardwiz Antivirus", szAllUserPath ) ;
	}
	else
	{
		wsprintf( szDestin, L"%s\\Application Data\\Wardwiz Antivirus", szAllUserPath ) ;
	}

	TCHAR szModulePath[MAX_PATH] = {0};
	_tcscpy( szModulePath, GetWardWizPathFromRegistry());
	//GetModuleFileName(NULL, szModulePath, MAX_PATH);
	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	wcscpy( szDestin1, szDestin ) ;

	////wsprintf( szSource, L"%s\\VBEVALREG.DLL",	szModulePath ) ;
	//wcscat( szDestin, L"\\VBEVALREG.DLL" ) ;
	wsprintf(szSource1, L"%s\\VBUSERREG.DB",	szModulePath ) ;
	//CopyFileToDestination( szSource, szDestin ) ;

	wcscat( szDestin1, L"\\VBUSERREG.DB") ;

	CopyFileToDestination( szSource1, szDestin1 ) ;

	TCHAR	szDrives[256] = {0} ;
	GetLogicalDriveStrings( 255, szDrives ) ;

	TCHAR	*pDrive = szDrives ;

	while( wcslen(pDrive) > 2 )
	{
		memset(szDestin, 0x00, 512*sizeof(TCHAR) ) ;
		wsprintf( szDestin, L"%sVBEVALREG.DLL",	pDrive ) ;

		memset(szDestin1, 0x00, 512*sizeof(TCHAR) ) ;
		wsprintf( szDestin1, L"%sVBUSERREG.DB",	pDrive ) ;

		if( ( GetDriveType(pDrive) & DRIVE_FIXED ) == DRIVE_FIXED )
		{
			//CopyFileToDestination( szSource, szDestin ) ;
			CopyFileToDestination( szSource1, szDestin1 ) ;
		}
		pDrive += 4 ;
	}

	return 0 ;
}

bool CWardWizRegistrationApp::CopyFileToDestination( TCHAR *pszSource, TCHAR *pszDest )
{
	try
	{
		CopyFile( pszSource, pszDest, FALSE ) ;
		SetFileAttributes( pszDest, FILE_ATTRIBUTE_HIDDEN ) ;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CRegistrationSecondDlg::CopyFileToDestination", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true ;
}

void CWardWizRegistrationApp::LoadResourceDLL()
{
	CString	csWardWizModulePath = GetWardWizPathFromRegistry() ;

	CString	csWardWizResourceDLL = L"" ;
	csWardWizResourceDLL.Format( L"%sVBRESOURCE.DLL", csWardWizModulePath ) ;
	if( !PathFileExists( csWardWizResourceDLL ) )
	{
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_RESOURCEDLL_NOTFOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		exit(0);
	}

	if( !m_hResDLL )
	{
		m_hResDLL = LoadLibrary(csWardWizResourceDLL);
		if (!m_hResDLL)
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_RESOURCE_MODULE_LOAD_FAILED"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			exit(0);
		}
	}
}

CString CWardWizRegistrationApp::GetModuleFilePath()
{
	TCHAR szModulePath[MAX_PATH] = {0};
	GetModuleFileName(NULL, szModulePath, MAX_PATH);

	TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
	szTemp[0] = '\0';

	return(CString(szModulePath));
}

/***********************************************************************************************
  Function Name  : CreateFonts
  Description    : this function is  called for setting the different Fonts
  Author Name    : Nitin Kolapkar
  Date           : 29 April 2014
***********************************************************************************************/
void CWardWizRegistrationApp::CreateFonts()
{
	LOGFONT lfInstallerTitle;  
	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfWeight = FW_BOLD;
	lfInstallerTitle.lfWidth = 15;
	lfInstallerTitle.lfHeight = 30;
	wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Verdana"));	 //	   with	face name "Verdana".
	m_fontInnerDialogTitle.CreateFontIndirect(&lfInstallerTitle);	

	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfHeight = 15;
	lfInstallerTitle.lfWeight = FW_BOLD;
	lfInstallerTitle.lfWidth = 6;
	wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Microsoft Sans serif Regular"));
	m_fontText.CreateFontIndirect(&lfInstallerTitle);

	

	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfHeight = 20;
	lfInstallerTitle.lfWeight = FW_BOLD;
	lfInstallerTitle.lfWidth = 8;
	m_FontNonProtectMsg.CreateFontIndirect(&lfInstallerTitle);

	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfHeight = 15;
	lfInstallerTitle.lfWeight = FW_NORMAL;
	lfInstallerTitle.lfWidth = 6;
	m_fontTextNormal.CreateFontIndirect(&lfInstallerTitle);	

	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfHeight = 14;
	lfInstallerTitle.lfWeight = FW_NORMAL;
	lfInstallerTitle.lfWidth = 5;
	lfInstallerTitle.lfUnderline = true;
	m_fontHyperlink.CreateFontIndirect(&lfInstallerTitle);	
	
	//Edited by Prasanna---------------------------------------------------------
	
	//For Registry Optimizer and Data Encryption
	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfHeight = 24;
	lfInstallerTitle.lfWeight = FW_NORMAL;
	lfInstallerTitle.lfWidth = 10;
	wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Verdana"));
	m_fontWWTextTitle.CreateFontIndirect(&lfInstallerTitle);

	//For all other titles
	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfHeight = 22;
	lfInstallerTitle.lfWeight = FW_NORMAL;
	lfInstallerTitle.lfWidth = 9;
	wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Verdana"));
	m_fontWWTextSmallTitle.CreateFontIndirect(&lfInstallerTitle);

	//For the Sub titles
	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfHeight = 18;
	lfInstallerTitle.lfWeight = FW_NORMAL;
	lfInstallerTitle.lfWidth = 7;
	wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Verdana"));
	m_fontWWTextSubTitle.CreateFontIndirect(&lfInstallerTitle);

	//For normal Text in the GUI
	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfHeight = 15;
	lfInstallerTitle.lfWeight = FW_NORMAL;
	lfInstallerTitle.lfWidth = 5;
	wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Microsoft Sans serif Regular"));
	m_fontWWTextNormal.CreateFontIndirect(&lfInstallerTitle);	

	//For the Description given below the titles
	memset(&lfInstallerTitle, 0, sizeof(LOGFONT));   // Clear out structure.
	lfInstallerTitle.lfHeight = 12;
	lfInstallerTitle.lfWeight = FW_NORMAL;
	lfInstallerTitle.lfWidth = 4;
	wcscpy_s(lfInstallerTitle.lfFaceName, LF_FACESIZE, _T("Microsoft Sans serif Regular"));
	m_fontWWTextSubTitleDescription.CreateFontIndirect(&lfInstallerTitle);	
	

	//Edited by Prasanna---------------------------------------------------------
}

void CWardWizRegistrationApp::ReadProductVersion4mRegistry()
{
	//Modified Date 10 July,2010 Neha Gharge 
	//Added a data encryption version no.
	HKEY hKey = NULL;
	DWORD dwvalueSType = 0;
	DWORD dwvalueSize = sizeof(DWORD);
	DWORD ChkvalueForDemoEdition = 0;
	TCHAR szAppVersion[1024];
	DWORD dwAppVersionLen = 1024;
	TCHAR szDataEncVersion[1024];
	DWORD dwDataEncVersion = 1024;
	DWORD dwType = REG_SZ;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey.GetBuffer(), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
	{
		return;
	}
	long ReadReg = RegQueryValueEx(hKey, L"AppVersion", NULL ,&dwType,(LPBYTE)&szAppVersion, &dwAppVersionLen);
	if(ReadReg == ERROR_SUCCESS)
	{
		m_csRegProductVer = (LPCTSTR)szAppVersion;
	}

	dwType = REG_SZ;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey.GetBuffer(), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
	{
		return;
	}
	
	ReadReg = RegQueryValueEx(hKey, L"DataEncVersion", NULL, &dwType, (LPBYTE)&szDataEncVersion, &dwDataEncVersion);
	if (ReadReg == ERROR_SUCCESS)
	{
		m_csDataEncVer = (LPCTSTR)szDataEncVersion;
	}

	
	dwType=REG_DWORD;

	ReadReg=RegQueryValueEx(hKey,L"dwWardWizDemo",NULL,&dwType,(LPBYTE)&dwvalueSType,&dwvalueSize);
	ChkvalueForDemoEdition=(DWORD)dwvalueSType;
	if(ChkvalueForDemoEdition==0)
	{
		m_bAllowDemoEdition = false;
	}
	else
	{
		m_bAllowDemoEdition = true;
	}
}

bool CWardWizRegistrationApp::SingleInstanceCheck()
{
	//Issue no 1212. mutex is not release properly, Everytime new mutex get created.
	HANDLE hCurrentHandle = CreateMutex(NULL, TRUE, L"{WardWizRegistration}");
	DWORD dwError = GetLastError();
	if(dwError == ERROR_ALREADY_EXISTS)
	{
		if (hCurrentHandle)
		{
			ReleaseMutex(hCurrentHandle);
			CloseHandle(hCurrentHandle);
			hCurrentHandle = NULL;
		}
		return true;
	}
	m_hRegMutexHandle = hCurrentHandle;
	return false;
}
/***************************************************************************
Function Name  : ProductRenewalDetails
Description    : After 1 Year In Registration Dialog User Info Should Appear byDefault
Author Name    : Nitin K
Date           : 15th April-2015
****************************************************************************/
void CWardWizRegistrationApp ::ProductRenewalDetails()
{
	//New Implementation: After 1 Year In Registration Dialog User Info Should Appear byDefault
	//Implementated By : Nitin K. Date : 16th April 2015
	CString csEmailID, csUserFirstName, csUserLastName, csNoofDays, csProdKeyNumber, csTelephoneNo, csSalutation;
	DWORD dwDaysLeft = GetDaysLeft(theApp.m_dwProductID);

	ReadProductVersion4mRegistry();

	csEmailID.Format(L"%s", g_ActInfo.szEmailID);
	csProdKeyNumber.Format(L"%s", g_ActInfo.szKey);
	//Issue: 0000707 :  Need to reload entries for trial product
	//Resolved By : Nitin K. Date: 15th July 2015
	if (csEmailID != L"" /*&& csProdKeyNumber != L"" && dwDaysLeft <= 30*/)
	{
		m_bIsProductRenewal = true;
		if (!(_tcscmp(g_ActInfo.szTitle, L"")))
		{
			m_dwSalutation = -1;
		}
		if (!(_tcscmp(g_ActInfo.szTitle, theApp.m_objwardwizLangManager.GetString(L"IDS_REG_COMBO_MR"))))
		{
			m_dwSalutation = 0;
		}
		if (!(_tcscmp(g_ActInfo.szTitle, theApp.m_objwardwizLangManager.GetString(L"IDS_REG_COMBO_MRS"))))
		{
			m_dwSalutation = 1;
		}
		if (!(_tcscmp(g_ActInfo.szTitle, theApp.m_objwardwizLangManager.GetString(L"IDS_REG_COMBO_MS"))))
		{
			m_dwSalutation = 2;
		}

		
		csUserFirstName.Format(L"%s", g_ActInfo.szUserFirstName);
		m_csFirstName = csUserFirstName;
		csUserLastName.Format(L"%s", g_ActInfo.szUserLastName);
		m_csLastName = csUserLastName;
		m_csEmailID = csEmailID;
		csTelephoneNo.Format(L"%s", g_ActInfo.szMobileNo);
		m_csContactNo = csTelephoneNo;
		m_csProductKeyNumber = csProdKeyNumber;
	}
}
extern "C" DLLEXPORT DWORD SendUserInformation2Server(int iInfoType)
{
	DWORD dwRet = 0x00;
	__try
	{
		dwRet = theApp.SendUserInformation2Server(iInfoType);
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in SendUserInformation2Server");
	}
	return dwRet;
}

/***********************************************************************************************
  Function Name  : SendUserInformation2Server
  Description    : Function which gets user Information and send to server also take response from 
				   server. if the customer is not genuine need to unregister his product.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : 
  Date           : 10th Oct 2014
***********************************************************************************************/
DWORD CWardWizRegistrationApp::SendUserInformation2Server(int iInfoType)
{
	DWORD dwRet = 0;
	__try
	{
		if (m_objRegistrationDlg.m_objRegistrationSecondDlg.ReadHostFileForWWRedirection())
		{
			AddLogEntry(L"### Hosts redirection found, please check hosts file present in Drivers folder", 0, 0, true, SECONDLEVEL);
			dwRet = 0x01;
			goto CleanUp;
		}

		//Function to send User Information to server.
		dwRet = m_objRegistrationDlg.m_objRegistrationSecondDlg.SendUserInformation2Server(iInfoType);
		if (dwRet)
		{
			AddLogEntry(L"### Failed to m_objRegistrationSecondDlg.SendUserInformation2Server In CVibraniumRegistrationApp::SendUserInformation2Server");
			//dwRet = 0x02;
			goto CleanUp;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in SendUserInformation2Server", 0, 0, true, SECONDLEVEL);
	}
CleanUp:
	return dwRet;
}

/***********************************************************************************************
  Function Name  : Check4OfflineActivationFlag
  Description    : Function to check whether the registration is happened Online/Offline.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : 
  Date           : 10th Oct 2014
***********************************************************************************************/
bool CWardWizRegistrationApp::Check4OfflineActivationFlag()
{
	bool bReturn = false;

	CITinRegWrapper objReg;
	DWORD dwData = 0x00;
	if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey.GetBuffer(), L"dwRegUserType", dwData) != 0x00)
	{
		return false;
	}

	if(dwData == 0x01)
	{
		bReturn = true;
	}

	return bReturn;
}

/***********************************************************************************************
  Function Name  : SetActivationFlagAsOnline
  Description    : Function to set flag as Non Offline ( ONLINE REGISTRATION )
				   function which sets DWORD value as follows:
				   dwRegUserType    - 0 ( Online )
									- 1 ( Offline )
  Author Name    : Ramkrushna Shelke
  SR.NO			 : 
  Date           : 10th Oct 2014
***********************************************************************************************/
bool CWardWizRegistrationApp::SetActivationFlagAsOnline()
{
	CITinRegWrapper objReg;
	DWORD dwData = 0x00;
	if (objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey.GetBuffer(), L"dwRegUserType", dwData) != 0x00)
	{
		return false;
	}
	return true;
}

/***********************************************************************************************
Function Name  : CheckScanLevel
Description    : To check scan level 1-> Only with wardwiz scanner
2-> with clam scanner and second level scaner is wardwiz scanner.
SR.NO			 :
Author Name    : Neha gharge
Date           : 1-19-2015
***********************************************************************************************/
void CWardWizRegistrationApp::CheckScanLevel()
{
	DWORD dwScanLevel = 0;
	if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey.GetBuffer(), L"dwScanLevel", dwScanLevel) != 0x00)
	{
		AddLogEntry(L"### Failed to get Registry Entry for dwScanLevel in CVibraniumRegistrationApp::CheckScanLevel", 0, 0, true, SECONDLEVEL);;
		return;
	}

	switch (dwScanLevel)
	{
	case 0x01:
		m_eScanLevel = WARDWIZSCANNER;
		break;
	case 0x02:
		m_eScanLevel = CLAMSCANNER;
		break;
	default:
		AddLogEntry(L" ### Scan Level option is wrong. Please reinstall setup of Vibranium", 0, 0, true, SECONDLEVEL);
		break;
	}
}

/***************************************************************************
Function Name  : CheckIsOffline
Description    : Check is offline registration (registry entry).
Author Name    : Nitin K.
Date           : 14/12/2015
****************************************************************************/
void CWardWizRegistrationApp::CheckIsOffline()
{
	try
	{
		DWORD dwOffLineValue = 0;
		if (m_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, theApp.m_csProdRegKey.GetBuffer(), L"dwIsOffline", dwOffLineValue) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwIsOffline in CVibraniumRegistrationApp::CheckIsOffline", 0, 0, true, SECONDLEVEL);;
			return;
		}
		switch (dwOffLineValue)
		{
		case 0x00:
			m_dwIsOffline = dwOffLineValue;
			break;
		case 0x01:
			m_dwIsOffline = dwOffLineValue;
			break;
		default:
			AddLogEntry(L" ### Could not find IsOffLine. Please reinstall setup of Vibranium", 0, 0, true, SECONDLEVEL);
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed to fetch registry entry CVibraniumRegistrationApp::CheckIsOffline", 0, 0, true, SECONDLEVEL);
	}
}