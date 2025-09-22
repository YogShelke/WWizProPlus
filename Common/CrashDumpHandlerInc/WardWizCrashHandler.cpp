/**********************************************************************************************************                	  Program Name          : WardWizCrashHandler.cpp
	  Description           : This class provides and functionality to create a crash and send the report to 
							  WardWiz website.
	  Author Name			: Ramkrushna Shelke                                                                            	  Date Of Creation      : 22 Jul 2014
	  Version No            : 1.0.0.5
	  Modification Log      :           
	  1. Ramkrushna         : Created wrapper to create a crash dump.		22 Jul 2014
***********************************************************************************************************/
#include "stdafx.h"
#include "WardWizCrashHandler.h"
#include <dbghelp.h>
#include <crtdbg.h>

////////////////////////////////////// Directives /////////////////////////////////////////
#pragma comment ( lib, "dbghelp.lib" )
///////////////////////////////////////////////////////////////////////////////////////////


/***********************************************************************************************
  Function Name  : CWardWizCrashHandler
  Description    : C'tor
  Author Name    : Ramkrushna Shelke
  Date           : 22 Jul 2014
***********************************************************************************************/
CWardWizCrashHandler::CWardWizCrashHandler(CString csAppName, CallBackFunctionPtr fnPtrCallBack):
	m_fnPtrCallBack(fnPtrCallBack)
	, m_csAppName(csAppName)
{
}

/***********************************************************************************************
  Function Name  : CWardWizCrashHandler
  Description    : D'tor
  Author Name    : Ramkrushna Shelke
  Date           : 22 Jul 2014
***********************************************************************************************/
CWardWizCrashHandler::~CWardWizCrashHandler(void)
{
}

/***********************************************************************************************
  Function Name  : Initialize
  Description    : This fucntion registers the crash report functionality using help of
				   CrAutoInstallHelper
  Author Name    : Ramkrushna Shelke
  Date           : 22 Jul 2014
***********************************************************************************************/
bool CWardWizCrashHandler::Initialize()
{
	bool bReturn = false;

	HRESULT hRes = ::CoInitialize(NULL);

	// If you are running on NT 4.0 or higher you can use the following call instead to 
	// make the EXE free threaded. This means that calls come in on a random RPC thread.
	//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
	ATLASSERT(SUCCEEDED(hRes));

	// Install crash reporting
	CR_INSTALL_INFO info;
	memset(&info, 0, sizeof(CR_INSTALL_INFO));
	info.cb = sizeof(CR_INSTALL_INFO);  
	info.pszAppName = m_csAppName;
	info.pszEmailSubject = _T("WardWiz Error Report"); // Define subject for email.
	info.pszEmailTo = _T("support@wardwiz.com");   // Define E-mail recipient address.  
	info.pszUrl = _T("http://wardwiz.com/WardWizCrashDumps/crashrpt.php"); // URL for sending reports over HTTP.				
	info.pfnCrashCallback = (LPGETLOGFILE)m_fnPtrCallBack; // Define crash callback function.   

	// Define delivery methods priorities. 
	info.uPriorities[CR_HTTP] = 3;         // Use HTTP the first.
	//info.uPriorities[CR_SMTP] = 2;         // Use SMTP the second.
	//info.uPriorities[CR_SMAPI] = 1;        // Use Simple MAPI the last.  
	info.dwFlags	= 0;                
	info.dwFlags	|= CR_INST_ALL_POSSIBLE_HANDLERS; // Install all available exception handlers.    
	info.dwFlags	|= CR_INST_ALLOW_ATTACH_MORE_FILES; //!< Adds an ability for user to attach more files to crash report by clicking "Attach More File(s)" item from context menu of Error Report Details dialog.
	info.pszDebugHelpDLL = NULL;                    // Search for dbghelp.dll using default search sequence.
	info.uMiniDumpType = MiniDumpNormal;            // Define minidump size.
	info.pszPrivacyPolicyURL = _T("http://wardwiz.in/index.php?route=information/information&information_id=3");
	info.pszErrorReportSaveDir = NULL;       // Save error reports to the default location.
	info.pszRestartCmdLine = _T("/restart"); // Command line for automatic app restart.

	// Install crash handlers.
	static CrAutoInstallHelper cr_install_helper(&info);
	if(cr_install_helper.m_nInstallStatus!=0)
	{
		TCHAR buff[256];
		crGetLastErrorMsg(buff, 256);
		MessageBox(NULL, buff, _T("crInstall error"), MB_OK);
		return FALSE;
	}
	ATLASSERT(cr_install_helper.m_nInstallStatus==0); 

	CString sLogFile = GetAppDir() + _T("\\log\\*.log");

	//WardWiz Antivirus log files from wardwiz folder.
	int nResult = crAddFile2(sLogFile, NULL, _T("WardWiz Antivirus log files"), CR_AF_MAKE_FILE_COPY|CR_AF_ALLOW_DELETE);
	ATLASSERT(nResult==0);

	TCHAR szModulePath[MAX_PATH] = {0};
	memset(szModulePath, 0x00, MAX_PATH * sizeof(TCHAR) ) ;
	GetEnvironmentVariable(L"ALLUSERSPROFILE", szModulePath, MAX_PATH ) ;

	//WardWiz Antivirus dump files from program files 
	//sLogFile = szModulePath;
	//sLogFile +=  _T("\\Wardwiz Antivirus\\log\\*.log");
	//nResult = crAddFile2(sLogFile, NULL, _T("WardWiz Antivirus log files"), CR_AF_MAKE_FILE_COPY|CR_AF_ALLOW_DELETE);
	//ATLASSERT(nResult==0);

	//WardWiz Antivirus dump files from program data 
	//sLogFile = szModulePath;
	//sLogFile +=  _T("\\Wardwiz Antivirus\\log\\*.dmp");
	//nResult = crAddFile2(sLogFile, NULL, _T("WardWiz Antivirus dump files"), CR_AF_MAKE_FILE_COPY|CR_AF_ALLOW_DELETE);
	//ATLASSERT(nResult==0);

	//WardWiz Antivirus dump files from Application folder
	sLogFile = GetAppDir() + _T("\\log\\*.dmp");
	nResult = crAddFile2(sLogFile, NULL, _T("WardWiz Antivirus dump files"), CR_AF_MAKE_FILE_COPY|CR_AF_ALLOW_DELETE);
	ATLASSERT(nResult==0);

	//take screenshot.
	nResult = crAddScreenshot2(CR_AS_PROCESS_WINDOWS | CR_AS_USE_JPEG_FORMAT | CR_AS_ALLOW_DELETE, 10);
	ATLASSERT(nResult==0);

	nResult = crAddProperty(_T("HDDSerialNumber"),_T("1234512345098765"));
	ATLASSERT(nResult==0);

	nResult = crAddProperty(_T("UserName"),_T("TheUserName"));
	ATLASSERT(nResult==0);
	
	nResult = crAddRegKey(_T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Wardwiz Antivirus"), _T("WardWizRegKey.xml"), CR_AR_ALLOW_DELETE);
	ATLASSERT(nResult==0);

    // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
    ::DefWindowProc(NULL, 0, 0, 0L);

	return bReturn;
}


/***********************************************************************************************
  Function Name  : GetAppDir
  Description    : Helper function that returns path to application directory
  Author Name    : Ramkrushna Shelke
  Date           : 22 Jul 2014
***********************************************************************************************/
CString CWardWizCrashHandler::GetAppDir()
{
    CString string;
    LPTSTR buf = string.GetBuffer(_MAX_PATH);
    GetModuleFileName(NULL, buf, _MAX_PATH);
    *(_tcsrchr(buf,'\\'))=0; // remove executable name
    string.ReleaseBuffer();
    return string;
}

/***********************************************************************************************
  Function Name  : GetModulePath
  Description    : Helper function that returns path to module
  Author Name    : Ramkrushna Shelke
  Date           : 22 Jul 2014
***********************************************************************************************/
CString CWardWizCrashHandler::GetModulePath(HMODULE hModule)
{
    CString string;
    LPTSTR buf = string.GetBuffer(_MAX_PATH);
    GetModuleFileName(hModule, buf, _MAX_PATH);
    TCHAR* ptr = _tcsrchr(buf,'\\');
    if(ptr!=NULL)
        *(ptr)=0; // remove executable name
    string.ReleaseBuffer();
    return string;
}




