/**********************************************************************************************************      	  
		Program Name			: ExtractScan.cpp
		Description				: This class functionality is for Extracting the attachments from email
								  and Zip/Unzip functionality.
		Author Name				: Ramkrushna Shelke                                                                  	 
		Date Of	Creation		: 07th Aug 2014
		Version No				: 1.0.0.5
		Special Logic Used		: 
		Modification Log		:           
***********************************************************************************************************/
#include "stdafx.h"
#include "ExtractScan.h"
#include "ISpyVirusPopUpDlg.h"
#include <Shlwapi.h>
#include "OutlookAddinApp.h"

#define		WRDWIZAVDBNAME			L"VIBRANIUMAV1.DB"

/***************************************************************************
  Function Name  : CExtractAttchForScan
  Description    : Const'r
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0018
  Date           : 07th Aug 2014
****************************************************************************/
CExtractAttchForScan::CExtractAttchForScan():
	m_objCom(TRAY_SERVER)
{
	m_lpLoadSigProc		= NULL ;
	m_lpUnLoadSigProc	= NULL ;
	m_lpScanFileProc	= NULL ;
	m_lpRepairFileProc	= NULL ;
	UnzipFile			= NULL ;
	MakeZipFile			= NULL ;

	m_bSignaturesLoaded = false;
	
	csVirusScanDBFile = L"" ;
	strScanStatus = L"" ;

}

/***************************************************************************
  Function Name  : ~CExtractAttchForScan
  Description    : Dest'r
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0019
  Date           : 07th Aug 2014
****************************************************************************/
CExtractAttchForScan::~CExtractAttchForScan()
{
	if( m_hModuleISpyScanDLL )
		FreeLibrary( m_hModuleISpyScanDLL ) ;

	if( m_hModuleISpyRepairDLL )
		FreeLibrary( m_hModuleISpyRepairDLL ) ;

	if( m_hZip )
		FreeLibrary( m_hZip ) ;

	m_hModuleISpyScanDLL = NULL ;
	m_hModuleISpyRepairDLL = NULL ;
	m_hZip = NULL ;
}

/***************************************************************************
  Function Name  : InitializeVariables
  Description    : Function which initialize the member variables.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0020
  Date           : 07th Aug 2014
****************************************************************************/
void CExtractAttchForScan::InitializeVariables()
{
	bZipFileModified = false ;

	dwVirusCount = 0x00 ;

	strScanStatus.Format(L"%s",theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCAN_NO_THREAT_FOUND")); //L"No threat(s) found" ;

	m_hModuleISpyScanDLL = NULL ;
	m_hModuleISpyRepairDLL = NULL ;
	m_hZip = NULL ;

	memset(szModulePath, 0x00, sizeof(TCHAR)*512) ;

}

/***************************************************************************
  Function Name  : GetWardWizPathFromRegistry
  Description    : Function which get WardWiz app folder path from registry.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0021
  Date           : 07th Aug 2014
****************************************************************************/
bool CExtractAttchForScan::GetWardWizPathFromRegistry( )
{
	HKEY	hSubKey = NULL ;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Vibranium"), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
		return false;

	//if( RegOpenKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Vibranium", &hSubKey ) != ERROR_SUCCESS )
	//	return false ;

	DWORD	dwSize = 511 ;
	DWORD	dwType = 0x00 ;

	RegQueryValueEx(hSubKey, L"AppFolder", 0, &dwType, (LPBYTE)szModulePath, &dwSize) ;
	RegCloseKey( hSubKey ) ;
	hSubKey = NULL ;

	if( PathFileExists( szModulePath) )
	{
		csVirusScanDBFile.Format( L"%sVBVIRUSSCANEMAIL.DB", szModulePath ) ;
		return true ;
	}

	return false ;
}

/***************************************************************************
  Function Name  : LoadDLLsANDSignatures
  Description    : Function which Load's VBSCANDLL.DLL and calls the LoadSignature
				   exported function.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0023
  Date           : 07th Aug 2014
****************************************************************************/
bool CExtractAttchForScan::LoadDLLsANDSignatures()
{
	//if signatures are already loaded no need to load again.
	if(m_bSignaturesLoaded)
	{
		return true;
	}
	bool	bRet = false ;
	__try
	{
		if( !GetWardWizPathFromRegistry( ) )
		{
			AddLogEntry(L"### CExtractAttchForScan::Failed to retrive iTIN path from registry", 0, 0, true, SECONDLEVEL);
			MessageBox(NULL, L"Failed to retrieve Vibranium Path", L"Vibranium", MB_ICONEXCLAMATION);
			return false ;
		}

		bRet = LoadScannerDLL() ;
		if( !bRet )
		{
			AddLogEntry(L"### CExtractAttchForScan::Failed to load required modules", 0, 0, true, SECONDLEVEL);
			MessageBox(NULL, L"Failed to load required modules", L"Vibranium", MB_ICONEXCLAMATION);
			return bRet ;
		}

		TCHAR	szTemp[MAX_PATH] = {0};

		wsprintf( szTemp, L"%sVBDB\\%s", szModulePath, WRDWIZAVDBNAME ) ;
		bRet = LoadSignatures(szTemp) ;
		if( !bRet )
		{
			AddLogEntry(L"### CExtractAttchForScan::Failed to Load Database file", 0, 0, true, SECONDLEVEL);
			MessageBox(NULL, L"Failed to Load Database file", L"Vibranium", MB_ICONEXCLAMATION);
			return bRet ;
		}
		m_bSignaturesLoaded = true;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CExtractAttchForScan::CreateWardwizFolder", 0, 0, true, SECONDLEVEL);
	}
	return true ;
}

/***************************************************************************
  Function Name  : LoadScannerDLL
  Description    : Fucntion which Load's VBSCANDLL.DLL and calls the LoadSignature
				   exported function.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0028
  Date           : 07th Aug 2014
****************************************************************************/
bool CExtractAttchForScan::LoadScannerDLL()
{

	TCHAR	szTemp[512] = {0} ;
#ifdef MSOUTLOOK32
	wsprintf( szTemp, L"%sWRDWIZSCANDLL32.DLL", szModulePath ) ;
#else
	wsprintf( szTemp, L"%sVBSCANDLL.DLL", szModulePath ) ;
#endif

	m_hModuleISpyScanDLL = LoadLibrary( szTemp );
	if(!m_hModuleISpyScanDLL)
	{
		AddLogEntry(L"### CExtractAttchForScan::Failed to load VBSCANDLL.DLL", 0, 0, true, SECONDLEVEL);
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAIL_LOAD_WRDWIZSCANDLL"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		return false;
	}
	
	m_lpLoadSigProc = (LOADSIGNATURE)GetProcAddress(m_hModuleISpyScanDLL, "LoadSignatures");
	if(!m_lpLoadSigProc)
	{
		FreeLibrary(m_hModuleISpyScanDLL);
		m_lpLoadSigProc = NULL;
		m_hModuleISpyScanDLL = NULL;
		AddLogEntry(L"### CExtractAttchForScan::Failed in GetProcAddress:LoadSignatures", 0, 0, true, SECONDLEVEL);
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAIL_LOAD_SIGNATURE_WRDWIZSCANDLL"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		return false;
	}

	m_lpUnLoadSigProc = (UNLOADSIGNATURES)GetProcAddress(m_hModuleISpyScanDLL, "UnLoadSignatures");
	if(!m_lpUnLoadSigProc)
	{
		FreeLibrary(m_hModuleISpyScanDLL);
		m_hModuleISpyScanDLL = NULL;
		m_lpUnLoadSigProc = NULL;
		AddLogEntry(L"### CExtractAttchForScan::Failed in GetProcAddress:UnLoadSignatures", 0, 0, true, SECONDLEVEL);
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAIL_LOAD_UNSIGNATURE_WRDWIZSCANDLL"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		return false;
	}

	m_lpScanFileProc = (SCANFILE)GetProcAddress(m_hModuleISpyScanDLL, "ScanFile");
	if(!m_lpScanFileProc)
	{
		FreeLibrary(m_hModuleISpyScanDLL);
		m_hModuleISpyScanDLL = NULL;
		m_lpScanFileProc = NULL;
		AddLogEntry(L"### CExtractAttchForScan::Failed in GetProcAddress:ScanFile", 0, 0, true, SECONDLEVEL);
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_SCANFILE_FAILED_LOAD"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		return false;
	}

#ifdef MSOUTLOOK32
	wsprintf( szTemp, L"%sWRDWIZREPAIRDLL32.DLL", szModulePath ) ;
#else
	wsprintf( szTemp, L"%sVBREPAIRDLL.DLL", szModulePath ) ;
#endif

	m_hModuleISpyRepairDLL = LoadLibrary( szTemp );
	if(!m_hModuleISpyRepairDLL)
	{
		AddLogEntry(L"### CExtractAttchForScan::Failed in loading VBREPAIRDLL.DLL", 0, 0, true, SECONDLEVEL);
		FreeLibrary(m_hModuleISpyScanDLL);
		m_hModuleISpyScanDLL = NULL ;
		m_hModuleISpyRepairDLL = NULL ;
		m_hZip = NULL ;
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED_LOAD_WRDWIZREPAIRDLL"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);

		return false;
	}

	m_lpRepairFileProc = (REPAIRFILE)GetProcAddress(m_hModuleISpyRepairDLL, "RepairFile");
	if(!m_lpRepairFileProc)
	{
		AddLogEntry(L"### CExtractAttchForScan::Failed in GetProcAddress:RepairFile", 0, 0, true, SECONDLEVEL);
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_MSG_FAILED_REAIRFILE_FUNC"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		FreeLibrary(m_hModuleISpyRepairDLL);
		FreeLibrary(m_hModuleISpyScanDLL);
		m_hModuleISpyScanDLL = NULL ;
		m_hModuleISpyRepairDLL = NULL ;
		m_hZip = NULL ;
		return false;
	}

	memset(szTemp, 0x00, sizeof(TCHAR)*512 ) ;
#ifdef MSOUTLOOK32
	wsprintf( szTemp, L"%sWRDWIZEXTRACT32.DLL", szModulePath ) ;
#else
	wsprintf( szTemp, L"%sVBEXTRACT.DLL", szModulePath ) ;
#endif
	m_hZip = LoadLibrary( szTemp ) ;
	if(!m_hZip )
	{
		AddLogEntry(L"### CExtractAttchForScan::Failed in loading VBEXTRACT.DLL", 0, 0, true, SECONDLEVEL);
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_LOAD_FAILED_EXTRACT_DLL"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		FreeLibrary(m_hModuleISpyRepairDLL);
		FreeLibrary(m_hModuleISpyScanDLL);
		m_hModuleISpyScanDLL = NULL ;
		m_hModuleISpyRepairDLL = NULL ;
		m_hZip = NULL ;
		return false;
	}

	UnzipFile = (UNZIPFILE ) GetProcAddress( m_hZip, "UnzipFile" ) ;
	if(!UnzipFile )
	{
		AddLogEntry(L"### CExtractAttchForScan::Failed in GetProcAddress:UnzipFile", 0, 0, true, SECONDLEVEL);
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_FUN_UNZIPFILE_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		FreeLibrary(m_hModuleISpyRepairDLL);
		FreeLibrary(m_hModuleISpyScanDLL);
		m_hModuleISpyScanDLL = NULL ;
		m_hModuleISpyRepairDLL = NULL ;
		m_hZip = NULL ;
		return false;
	}

	MakeZipFile = (MAKEZIPFILE ) GetProcAddress( m_hZip, "MakeZipFile" ) ;
	if(!MakeZipFile )
	{
		AddLogEntry(L"### CExtractAttchForScan::Failed in GetProcAddress:MakeZipFile", 0, 0, true, SECONDLEVEL);
		MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_FUN_MAKEZIPFILE_NOT_FOUND"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
		FreeLibrary(m_hModuleISpyRepairDLL);
		FreeLibrary(m_hModuleISpyScanDLL);
		m_hModuleISpyScanDLL = NULL ;
		m_hModuleISpyRepairDLL = NULL ;
		m_hZip = NULL ;
		return false;
	}

	return true ;
}

/***************************************************************************
  Function Name  : LoadSignatures
  Description    : Fucntion which calls the LoadSignature Exported function
				   from SCANDLL.
  Author Name    : Ramkrushna Shelke
  SR.NO		 	 : EMAILSCANPLUGIN_0029
  Date           : 07th Aug 2014
****************************************************************************/
bool CExtractAttchForScan::LoadSignatures(LPTSTR lpFilePath)
{
	if( !m_lpLoadSigProc )
	{
		return false;
	}
	return m_lpLoadSigProc(lpFilePath) ;
}

/***************************************************************************
  Function Name  : UnLoadSignatures
  Description    : Fucntion which calls the Unload exported function from SCANDLL.
  Author Name    : Ramkrushna Shelke
  Date           : 07th Aug 2014
****************************************************************************/
bool CExtractAttchForScan::UnLoadSignatures()
{
	if( !m_lpUnLoadSigProc )
	{
		return false;
	}
	return m_lpUnLoadSigProc() ;
}

/***************************************************************************
  Function Name  : CreateFolder
  Description    :  The _mkdir function creates a new directory with the specified dirname. _mkdir can create						only one new directory per call, so only the last component of dirname can name a new							directory. _mkdir does not translate path delimiters. In Windows NT, both the backslash ( \)					and the forward slash (/ ) are valid path delimiters in character strings in run-time							routines.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0032
  Date           : 07th Aug 2014
****************************************************************************/
DWORD CExtractAttchForScan::CreateFolder( TCHAR *pDestFol)
{
	DWORD	dwRet = 0x00 ;

	__try
	{
		if( CreateDirectory( pDestFol, NULL ) )
			goto Cleanup ;

		Sleep( 10 ) ;
		if( CreateDirectory( pDestFol, NULL ) )
			goto Cleanup ;

		Sleep( 10 ) ;
		if( _wmkdir( (wchar_t *)pDestFol ) == 0 )
			goto Cleanup ;

		Sleep( 10 ) ;
		if( _wmkdir( (wchar_t *)pDestFol ) == 0 )
			goto Cleanup ;

		dwRet = 0x01 ;

	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
	}
Cleanup:
	return dwRet ;
}

/***************************************************************************
  Function Name  : EnumZipFolderForScanning
  Description    : Function which Enumerates the zip folder and scan's files 
				   contained in that.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0032
  Date           : 07th Aug 2014
****************************************************************************/
void CExtractAttchForScan::EnumZipFolderForScanning( TCHAR *pZipFol )
{
	TCHAR	szTemp[512] = {0} ;
	TCHAR	szZipFile[512] = {0} ;
	AddLogEntry(L">>> CExtractAttchForScan::Enum ZipFolder For Scanning", 0, 0, true, FIRSTLEVEL);
	wsprintf( szTemp, L"%s\\*.*", pZipFol ) ;
	strScanStatus = L"" ;

	CFileFind	finder ;

	dwRemovedZips = 0x00 ;

	BOOL bWorking = finder.FindFile( szTemp );
	while( bWorking )
	{
		bWorking = finder.FindNextFile();
		if( finder.IsDirectory() )
			continue ;

		if( finder.IsDots() )
			continue ;

		wsprintf(szZipFile, L"%s", finder.GetFilePath() ) ;
		ExtractZipForScanning( szZipFile ) ;

		memset(szZipFile, 0x00, sizeof(TCHAR)*512 ) ;
	}

	finder.Close() ;

	if( !strScanStatus.GetLength() )
		strScanStatus.Format(L"%s", theApp.m_objwardwizLangManager.GetString(L"IDS_USB_SCAN_NO_THREAT_FOUND")); // = L"No threat(s) found" ;
}

/***************************************************************************
  Function Name  : ExtractZipForScanning
  Description    : Function which extracts the zip folder for scanning purpose.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0033
  Date           : 07th Aug 2014
****************************************************************************/
void CExtractAttchForScan::ExtractZipForScanning( TCHAR *pZipFile )
{
	if( !UnzipFile )
		return ;

	if( !MakeZipFile )
		return ;
	AddLogEntry(L">>> CExtractAttchForScan::Extract ZipFolder For Scanning", 0, 0, true, FIRSTLEVEL);
	bool	bZipModified = false ;
	bool	bModified = false ;
	DWORD	dwZipFileCount = 0x00 ;
	DWORD	dwModifiedCount = 0x00 ;
	DWORD	dwStatus = 0x00 ;
	int		iSize = 0x00 ;
	DWORD   dwAction = 0;
	TCHAR	szUnzipPath[512] = {0} ;
	DWORD	dwSubType = 0x00, dwVersion = 0x00;
	FILETYPE	fileType = FILE_NOTKNOWN;
	DWORD dwFileType = 0x00;

	fileType = GetFileType(pZipFile, dwSubType, dwVersion);

	switch (fileType)
	{
	case FILE_ARCHIVE:	dwFileType = 0x02;
		break;
	default:			dwFileType = 0x00;
		break;
	}

	if (dwFileType == 0x02)
	{
		dwStatus = UnzipFile(pZipFile, szUnzipPath, dwZipFileCount);
		if (!dwZipFileCount)
			return;
	}
	else
	{
		_tcscpy_s(szUnzipPath, _countof(szUnzipPath), pZipFile);
	}
	VirusFoundDB.clear() ;
	ScanZipFiles(szUnzipPath, bZipModified, dwModifiedCount ) ;
	
	iSize = static_cast<int>(VirusFoundDB.size());
	
	CString	strFileName = L"" ;
	if( iSize )
	{
		CString	strFileName = L"" ;
		TCHAR	szFileName[512] = {0} ;
		TCHAR	*pFileName = NULL ;

		pFileName = wcsrchr( pZipFile, '\\' ) ;
		if( pFileName )
		{
			pFileName++ ;
			strFileName.Format( L"%s", pFileName ) ;
		}

		int iAction = -1;
		//Issue: We are not taking any action on infected mail so no need to show Action dialog
		//Resolved By : Nitin Kolapkar Date: 29th July
		//if(!GetActionRegSetting(iAction))
		//{
		//	CISpyVirusPopUpDlg	objISpyVirusPopUpDlg;
		//	objISpyVirusPopUpDlg.m_csThreatName = VirusFoundDB[0].strVirusName ;
		//	objISpyVirusPopUpDlg.m_csAttachmentName = strFileName ;
		//	objISpyVirusPopUpDlg.m_csSendersAddress = strSenderMailID ;

		//	if(objISpyVirusPopUpDlg.DoModal() == IDOK)
		//	{
		//		//Take Action on user setting
		//		iAction = objISpyVirusPopUpDlg.m_dwAction;
		//		if(iAction == 0)
		//		{
		//			strScanStatus  =  "Repaired";
		//		}
		//		else
		//		{
		//			strScanStatus  =  "Quarantined";
		//		}
		//	}
		//	else
		//	{
		//		strScanStatus  =  "Detected";
		//	}
		//}
		//Need to set default action as we not taking any actions e.g. Quarantined/Repaired
		strScanStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_OUTLOOK_PLUGIN_THREATS_BLOCKED");
		if( iAction >= -1 )
		{
			for(int i=0; i<iSize; i++ )
			{
				if( iAction == 0 )
				{
					if(VirusFoundDB[i].dwRepairID == 0)
					{
						if(!DeleteFile( VirusFoundDB[i].strFilePath ))
						{
							strScanStatus = L"Quarantined" ;
							MoveFileEx(VirusFoundDB[i].strFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );
							AddLogEntry(L"### DeleteFile Failed File Path: %s", VirusFoundDB[i].strFilePath, 0, true, SECONDLEVEL);						
						}
						else
						{
							strScanStatus = L"Repaired" ;
						}

						bModified = true ;
					}
					else if(m_lpRepairFileProc( VirusFoundDB[i].strFilePath, VirusFoundDB[i].dwRepairID) )
					{
						bModified = true ;
						strScanStatus = L"Repaired" ;
					}
				}
				else if (iAction == 1)
				{
					if(DeleteFile( VirusFoundDB[i].strFilePath ))
					{
						//dwCount++ ;
						strScanStatus = L"Quarantined" ;
						//bModified = true ;
					}
					else
					{
						MoveFileEx(VirusFoundDB[i].strFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT );
						strScanStatus = L"Quarantined" ;
						AddLogEntry(L"### DeleteFile Failed File Path: %s", VirusFoundDB[i].strFilePath, 0, true, SECONDLEVEL);
					}
				}

				if (theApp.m_dwDaysLeft == 0)
				{
					strScanStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_THREAT_DETECTED");
					iAction = 1;
				}
				else
				{
					strScanStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_OUTLOOK_PLUGIN_THREATS_BLOCKED");
				}
				
				//send notification 2 tray
				if (!SendEmailData2Tray(SHOW_EMAILSCAN_TRAY_POPUP, VirusFoundDB[i].strVirusName, strFileName, strSenderMailID, iAction))
				{
					AddLogEntry(L"### Error in CExtractAttchForScan::SendEmailData2Tray", 0, 0, true, SECONDLEVEL);
				}
				
				Sleep(10);
			}

			if (theApp.m_dwDaysLeft > 0)
			{
				if (iAction)
				{
					dwRemovedZips++;
					DeleteFile(pZipFile);
					cslRemoveAttachments.AddTail(strFileName);
				}
			}
		}
		if (theApp.m_dwDaysLeft > 0)
		{
			if (bModified)
			{
				csaReAttachZipFiles.Add(pZipFile);
				cslReAttachments.AddTail(strFileName);
				MakeZipFile(szUnzipPath, pZipFile);
				bZipFileModified = true;
			}
		}
	}
	//Delete all unzipped file(s)
	RemoveAllZipFiles( szUnzipPath ) ;
	RemoveFilesUsingSHFileOperation( szUnzipPath ) ;

	VirusFoundDB.clear() ;
}

/***************************************************************************
  Function Name  : ScanZipFiles
  Description    : Function which takes zip file as a input path starts enumeration 
				   of files for scanning.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0034
  Date           : 07th Aug 2014
****************************************************************************/
void CExtractAttchForScan::ScanZipFiles( TCHAR *pZipPath, bool &bModified, DWORD &dwCount )
{
	if( !m_lpScanFileProc )
		return ;

	if( !m_lpRepairFileProc )
		return ;

	try
	{
		AddLogEntry(L">>> CExtractAttchForScan::Scanning zip file started", 0, 0, true, FIRSTLEVEL);
		TCHAR	szTemp[512] = {0} ;
		TCHAR	szFile[512] = {0} ;
		TCHAR	szVirusName[512] = {0} ;

		DWORD	dwRepairID = 0x00 ;
		int		iIndex = 0x00 ;

		CFileFind	finder ;

		//	Get User setting for repair or quarantine
		//AfxMessageBox(L"at scanning");
		bModified = false ;
		dwCount = 0x00 ;
		DWORD	dwAttributes = 0;
		CString strWildcard(pZipPath);
		dwAttributes = GetFileAttributes(pZipPath);
		if (((FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_DIRECTORY) == dwAttributes) || (FILE_ATTRIBUTE_DIRECTORY == dwAttributes)||((FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_ATTRIBUTE_DIRECTORY) == dwAttributes))
		{
			//strWildcard += _T("\\*.*");
			if (strWildcard[strWildcard.GetLength() - 1] != '\\')
				strWildcard += _T("\\*.*");
			else
				strWildcard += _T("*.*");
		}
		else
		{
			wsprintf(szFile, L"%s", pZipPath);
		}

		BOOL bWorking = finder.FindFile(strWildcard);
		while( bWorking )
		{
			bWorking = finder.FindNextFile();

			if( finder.IsDots() )
				continue ;

			if( finder.IsDirectory() )
			{
				wsprintf(szFile, L"%s", finder.GetFilePath() ) ;
				ScanZipFiles(szFile, bModified, dwCount ) ;
			}
			else
			{
				wsprintf(szFile, L"%s", finder.GetFilePath() ) ;

				//Send File For Scanning
				//AfxMessageBox(L"at detection");
				if( !m_lpScanFileProc( szFile, szVirusName, dwRepairID, false ) )
				{
					dwRepairID = 0x00 ;
					memset(szFile, 0x00, sizeof(TCHAR)*512 ) ;
					memset(szVirusName, 0x00, sizeof(TCHAR)*512 ) ;
					continue ;
				}

				VirusFoundDB[iIndex].dwRepairID = dwRepairID ;
				VirusFoundDB[iIndex].strFilePath.Format( L"%s", szFile ) ;
				VirusFoundDB[iIndex++].strVirusName.Format( L"%s", szVirusName ) ;

				/*	if( szVirusName[0] )
				dwVirusCount++ ;

				//show here the virus found pop up.
				CISpyVirusPopUpDlg	objISpyVirusPopUpDlg;
				objISpyVirusPopUpDlg.m_csThreatName = szVirusName;
				objISpyVirusPopUpDlg.m_csAttachmentName = szFile;

				if(objISpyVirusPopUpDlg.DoModal() == IDOK)
				{
				//Take Action on user setting
				if( true )
				{
				//AfxMessageBox(L"at repair");
				if(m_lpRepairFileProc( szFile, dwRepairID) )
				{
				bModified = true ;
				strScanStatus = L"Repaired" ;
				}
				}
				else
				{
				if(DeleteFile( szFile ))
				{
				dwCount++ ;
				strScanStatus = L"Quarantined" ;
				}
				}
				}
				*/
				dwRepairID = 0x00 ;
				memset(szFile, 0x00, sizeof(TCHAR)*512 ) ;
				memset(szVirusName, 0x00, sizeof(TCHAR)*512 ) ;
			}
		}
		finder.Close() ;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CExtractAttchForScan::ScanZipFiles", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : RemoveAllZipFiles
  Description    : Function which is used to remove zip file path as given
				   in function argument.
  Author Name    : Ramkrushna Shelke
  Date           : 07th Aug 2014
****************************************************************************/
DWORD CExtractAttchForScan::RemoveAllZipFiles( TCHAR *pFile )
{
	try
	{
		AddLogEntry(L">>> CExtractAttchForScan::Removing all zip file", 0, 0, true, FIRSTLEVEL);
		TCHAR	szTemp[512] = {0} ;
		TCHAR	szFile[512] = {0} ;
		bool	bDelete = false ;
		CFileFind	finder ;

		wsprintf(szTemp, L"%s\\*.*", pFile ) ;
		BOOL bWorking = finder.FindFile( szTemp );
		while( bWorking )
		{
			bWorking = finder.FindNextFile();
			if( finder.IsDirectory() )
				continue ;

			if( finder.IsDots() )
				continue ;

			wsprintf(szFile, L"%s", finder.GetFilePath() ) ;
			if( RemoveFileOnly( szFile ) )
				bDelete = true ;

			memset(szFile, 0x00, sizeof(TCHAR)*512 ) ;
		}

		finder.Close() ;
		if( bDelete )
			return 0x01 ;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CExtractAttchForScan::RemoveAllZipFiles", 0, 0, true, SECONDLEVEL);
	}
	return 0x00 ;
}

/***************************************************************************
  Function Name  : RemoveFileOnly
  Description    : Function which delets the file from given path.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0036
  Date           : 07th Aug 2014
****************************************************************************/
DWORD CExtractAttchForScan::RemoveFileOnly( TCHAR *pFile )
{
	__try
	{
		if( DeleteFile( pFile ) )
			return 0 ;

		Sleep( 10 ) ;

		if( _tremove( pFile ) == 0 )
			return 0 ;

		Sleep( 10 ) ;
		if( _tunlink( pFile ) == 0 )
			return 0 ;

		Sleep( 10 ) ;
		if( DeleteFile( pFile ) )
			return 0 ;

		Sleep( 10 ) ;
		MoveFileEx( pFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT ) ;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CExtractAttchForScan::RemoveFileOnly", 0, 0, true, SECONDLEVEL);
	}
	return 1 ;
}

/***************************************************************************
  Function Name  : RemoveFilesUsingSHFileOperation
  Description    : Function which removes file using SHFileOperation
				   This function can be used to copy, move, rename, or 
				   delete a file system object.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0037
  Date           : 07th Aug 2014
****************************************************************************/
DWORD CExtractAttchForScan::RemoveFilesUsingSHFileOperation(TCHAR *pFolder )
{
	HRESULT			hr = 0x00 ;
	__try
	{
		SHFILEOPSTRUCT	sfo = {0} ;

		TCHAR	szTemp[512] = {0} ;

		wsprintf(szTemp, L"%s\\*\0", pFolder ) ;

		sfo.hwnd = NULL ;
		sfo.wFunc = FO_DELETE ;
		sfo.pFrom = szTemp ;
		sfo.pTo = NULL ;
		sfo.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR ;

		hr = SHFileOperation( &sfo ) ;
		if( !hr )
		{
			_wrmdir( (wchar_t *) pFolder ) ;
			RemoveDirectory( pFolder ) ;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CExtractAttchForScan::RemoveFilesUsingSHFileOperation", 0, 0, true, SECONDLEVEL);
	}
	return hr ;
}

/***************************************************************************
  Function Name  : AddToVirusScanEmailDB
  Description    : Function which sends mail ID, subject to service to save 
				   in DB file.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0038
  Date           : 07th Aug 2014
****************************************************************************/
bool CExtractAttchForScan::AddToVirusScanEmailDB(CString &strSenderMailID, CString &strSubject )
{
	try
	{
		CString csVirusEntry = L"";
		if(strSubject == L"")
		{
			strSubject = L"NA";
		}

		//Commented this code as not need to give date and time in case of virus found entry for
		//Email scan.
		///*Prajakta 29-May-2014, added code for datetime for Email Virus Scan DB.*/
		//SYSTEMTIME		CurrTime = {0} ;
		//GetSystemTime( &CurrTime ) ;
		//CTime	Time_Curr( CurrTime ) ;
		//int iMonth = Time_Curr.GetMonth();
		//int iDate = Time_Curr.GetDay();
		//int iYear = Time_Curr.GetYear();

		//CString csDate = L"";
		//csDate.Format(L"%d/%d/%d",iMonth,iDate,iYear);
		//	
		//CTime ctDateTime = CTime::GetCurrentTime();
		//CString csTime =  ctDateTime.Format(_T("%H:%M:%S"));

		//CString csDateTime = L"";
		//csDateTime.Format(_T("%s %s"),csDate,csTime);

		//csVirusEntry.Format(L"%s#%s#%s#%s#",csDateTime,strSenderMailID,strSubject,strScanStatus);
		if (!SendEmailData2Service(ADD_EMAIL_ENTRY, VIRUSSCAN, strSenderMailID, strSubject, strScanStatus, true))
		{
			AddLogEntry(L"### Error in CExtractAttchForScan::SendEmailData2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		//issue no 857 Neha gharge
		if(!SendReLoadMessage2UI())
		{
			AddLogEntry(L"### Error in CExtractAttchForScan::AddToVirusScanEmailDB::SendReLoadMessage2UI", 0, 0, true, SECONDLEVEL);
		}

		return true;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CExtractAttchForScan::AddToVirusScanEmailDB", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
  Function Name  : LoadVirusScanEmailDB
  Description    : Function which loads the virus scan email DB and fills the structure.
  Author Name    : Ramkrushna Shelke
  Date           : 07th Aug 2014
****************************************************************************/
bool CExtractAttchForScan::LoadVirusScanEmailDB()
{
	try
	{
		if( !GetWardWizPathFromRegistry( ) )
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_FILED_RETRIVE_WRDWIZ_PATH"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION);
			return false ;
		}

		if( !PathFileExists( csVirusScanDBFile) )
			return false ;

		CFile rFile(csVirusScanDBFile, CFile::modeRead);
		// Create a loading archive
		CArchive arLoad(&rFile, CArchive::load);

		CDataManager	m_objVirusScandb ;

		m_objVirusScandb.Serialize( arLoad ) ;
		VirusScanDB.clear() ;

		const ContactList& contacts = m_objVirusScandb.GetContacts() ;

		int	i = 0x00 ;

		POSITION pos = contacts.GetHeadPosition();
		while(pos != NULL)
		{
			const CIspyList contact = contacts.GetNext(pos);

			VirusScanDB[i].strEmailID = contact.GetFirstEntry() ;
			VirusScanDB[i].strSubject = contact.GetSecondEntry() ;

/*			wcscpy_s(VirusScanDB[i].strEmailID, 127, contact.GetFirstName()) ;
			wcscpy_s(VirusScanDB[i].strSubject, 511, contact.GetLastName()) ;
*/
			CString	strStatus = contact.GetThirdEntry() ;
			//if( strStatus.CompareNoCase( L"No threat found") == 0 )
			//	VirusScanDB[i].strScanStatus = 1 ;

			if( strStatus.CompareNoCase( L"Repaired") == 0 )
			{
				VirusScanDB[i++].strScanStatus = 2 ;
				continue ;
			}

			if( strStatus.CompareNoCase( L"Quarantined") == 0 )
			{
				VirusScanDB[i++].strScanStatus = 3 ;
				continue ;
			}

			VirusScanDB[i++].strScanStatus = 1 ;
		}

		arLoad.Close();
		rFile.Close() ;

		return true;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CExtractAttchForScan::LoadVirusScanEmailDB", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
  Function Name  : GetActionRegSetting
  Description    : Function which gets action setting's saved in registry.
  Author Name    : Ramkrushna Shelke
  Date           : 07th Aug 2014
****************************************************************************/
bool CExtractAttchForScan::GetActionRegSetting(int &iValue)
{
	__try
	{
		HKEY hKey;
		DWORD dwvalueSType;
		DWORD dwvalueSize = sizeof(DWORD);
		DWORD dwType = REG_DWORD;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Vibranium\\EmailScanSetting"), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
			return false; 

		RegQueryValueEx(hKey,L"dwAllowVirusPopUp",NULL,&dwType,(LPBYTE)&dwvalueSType,&dwvalueSize);

		if(dwvalueSType == 1)
		{
			return false;
		}

		RegQueryValueEx(hKey,L"dwAttachScanAction",NULL,&dwType,(LPBYTE)&dwvalueSType,&dwvalueSize);

		iValue = (int)dwvalueSType;

		RegCloseKey(hKey);
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CExtractAttchForScan::GetActionRegSetting", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************
  Function Name  : SendReLoadMessage2UI
  Description    : Function which Sends reload message to UI, once DB file get
				   updated from outlook.
  Author Name    : Ramkrushna Shelke
  Date           : 07th Aug 2014
****************************************************************************/
bool CExtractAttchForScan::SendReLoadMessage2UI()
{
	__try
	{
		HWND hWindow = FindWindow( NULL, L"WRDWIZAVUI");
		SendMessage(hWindow, RELOAD_EMAILVIRUSCANDB, 0, 0);
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CExtractAttchForScan::SendReLoadMessage2UI", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
  Function Name  : SendEmailData2Service
  Description    : Function to send data to service
				   dwAddEditDelType : ADD_EMAIL_ENTRY,EDIT_EMAIL_ENTRY,DELETE_EMAIL_ENTRY,SAVE_EMAIL_ENTRIES
				   dwType			: RECOVER, VIRUSSCAN,SPAMFILTER,CONTENTFILTER,SIGNATURE,REPORTS,MALICIOSSITES
				   csEntry			: # tokenized String
  Author Name    : Ramkrushna Shelke
  Date           : 20 Jan 2014
***********************************************************************************************/
bool CExtractAttchForScan::SendEmailData2Service(DWORD dwAddEditDelType, DWORD dwType, CString csEntry, bool bVirusScanwait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	szPipeData.iMessageInfo = static_cast<int>(dwAddEditDelType);
	szPipeData.dwValue = dwType;
	//_tcscpy_s(szPipeData.szFirstParam, sizeof(szPipeData.szFirstParam), szEntry);
	_tcscpy_s(szPipeData.szFirstParam , csEntry);
	
	CISpyCommunicator objCom(SERVICE_SERVER, true);
	if(!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send Data in CExtractAttchForScan::SendEmailData2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bVirusScanwait)
	{
		if(!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CExtractAttchForScan::SendEmailData2Service", 0, 0, true, SECONDLEVEL);
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
	Function Name    :	SendEmailData2Service
	Description      :	Function to send data to service
	dwAddEditDelType :	ADD_EMAIL_ENTRY
	dwType			 : 	RECOVER, VIRUSSCAN,SPAMFILTER,CONTENTFILTER,SIGNATURE,REPORTS,MALICIOSSITES
	csSendersAddress :	Senders Address
	csSubject		 :	Mail Subject
	csStatus		 :	Mail Status
	Author Name      :	Ramkrushna Shelke
	Date             :	7th Aug 2015
***********************************************************************************************/
bool CExtractAttchForScan::SendEmailData2Service(DWORD dwAddEditDelType, DWORD dwType, CString csSendersAddress, CString csSubject, CString csStatus, bool bVirusScanwait)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	szPipeData.iMessageInfo = static_cast<int>(dwAddEditDelType);
	szPipeData.dwValue = dwType;
	_tcscpy_s(szPipeData.szFirstParam, csSendersAddress);
	_tcscpy_s(szPipeData.szSecondParam, csSubject);
	_tcscpy_s(szPipeData.szThirdParam, csStatus);

	CISpyCommunicator objCom(SERVICE_SERVER, true);
	if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to send Data in CExtractAttchForScan::SendEmailData2Service", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if (bVirusScanwait)
	{
		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CExtractAttchForScan::SendEmailData2Service", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (szPipeData.dwValue != 1)
		{
			return false;
		}
	}
	return true;
}

/***************************************************************************
  Function Name  : SendEmailData2Tray
  Description    : Function which sends threat name, attachment name, senders address,
				   action 
  Author Name    : Ramkrushna Shelke
  Date           : 07th Aug 2014
****************************************************************************/
bool CExtractAttchForScan::SendEmailData2Tray(int iMessage, CString csThreatName, CString csAttachmentName, CString csSenderAddr, DWORD dwAction,bool bWait)
{
	ISPY_PIPE_DATA szPipeData = {0};
	memset(&szPipeData, 0, sizeof(ISPY_PIPE_DATA));

	//Issue Number 474. Rajil Yadav 10/06/2014

	szPipeData.iMessageInfo = iMessage;
	szPipeData.dwValue = dwAction;
	wcscpy_s(szPipeData.szFirstParam, csThreatName);
	wcscpy_s(szPipeData.szSecondParam, csAttachmentName);
	wcscpy_s(szPipeData.szThirdParam, csSenderAddr);

	if(!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to set data in CSystemTray : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if(bWait)
	{
		if(!m_objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to set data in CSystemTray : SetRegistrykeyUsingService", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

FILETYPE CExtractAttchForScan::GetFileType(LPCTSTR pszFilePath, DWORD &dwSubType, DWORD &dwVersion)
{
	FILETYPE	fileType = FILE_NOTKNOWN;

	HANDLE		hFile = INVALID_HANDLE_VALUE;

	try
	{

		DWORD	dwReadData = 0x00;
		DWORD	dwReadBytes = 0x00;

		hFile = CreateFile(pszFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
			goto Cleanup;

		ReadFile(hFile, &dwReadData, 0x04, &dwReadBytes, NULL);

		//Checking for Compressed file means ZIP or RAR
		if ((dwReadData == 0x21726152) || ((dwReadData & 0xFFFF4B50) == dwReadData))
		{
			fileType = FILE_ARCHIVE;
			goto Cleanup;
		}

		//Checking for PE file
		if (((dwReadData & 0xFFFF5A4D) == dwReadData))
		{
			dwReadData = 0x00;
			SetFilePointer(hFile, 0x3C, NULL, FILE_BEGIN);
			ReadFile(hFile, &dwReadData, 0x04, &dwReadBytes, NULL);

			if (dwReadData)
			{
				SetFilePointer(hFile, dwReadData, NULL, FILE_BEGIN);

				dwReadData = 0x00;
				ReadFile(hFile, &dwReadData, 0x04, &dwReadBytes, NULL);

				if (dwReadData == 0x00004550)
				{
					fileType = FILE_PE;
					goto Cleanup;
				}
			}
		}

	}
	catch (...)
	{
		fileType = FILE_NOTKNOWN;
	}

Cleanup:

	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);

	hFile = INVALID_HANDLE_VALUE;

	return fileType;
}