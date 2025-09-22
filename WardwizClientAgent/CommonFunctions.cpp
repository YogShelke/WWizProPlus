#pragma once
#include "stdafx.h"
#include "CommonFunctions.h"



CClientAgentCommonFunctions::CClientAgentCommonFunctions()
{
}


CClientAgentCommonFunctions::~CClientAgentCommonFunctions()
{
}


/**********************************************************************************************************
*  Function Name  :	GetLoggingLevel4mRegistry
*  Description    :	Read Registry entry for logging level
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
DWORD CClientAgentCommonFunctions::GetLoggingLevel4mRegistry()
{
	DWORD dwLogLevel = 0;
	try
	{
		HKEY	h_WRDWIZAV = NULL;
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey(), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &h_WRDWIZAV) != ERROR_SUCCESS)
		{
			h_WRDWIZAV = NULL;
			return 0;
		}

		DWORD dwLogLevelSize = sizeof(DWORD);
		DWORD dwType = REG_DWORD;
		long ReadReg = RegQueryValueEx(h_WRDWIZAV, L"dwLoggingLevel", 0, &dwType, (LPBYTE)&dwLogLevel, &dwLogLevelSize);
		if (ReadReg != ERROR_SUCCESS)
		{
			h_WRDWIZAV = NULL;
			return 0;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in GetLoggingLevel4mRegistry", 0, 0, true, SECONDLEVEL);
	}
	return dwLogLevel;
}

/**********************************************************************************************************
*  Function Name  :	Check4LogLevel
*  Description    :	Check the parameter for log entry
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
DWORD CClientAgentCommonFunctions::Check4LogLevel(DWORD dwRegLogLevel, DWORD dwLogLevel)
{
	DWORD dwRet = 0x0;
	__try
	{
		if (dwRegLogLevel == 1 && dwLogLevel < 1)
		{
			dwRet = 0x1;
		}
		else if (dwRegLogLevel == 2 && dwLogLevel < 2)
		{
			dwRet = 0x2;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return dwRet;
}

/**********************************************************************************************************
*  Function Name  :	GetModulePath
*  Description    :	Get path where module exist
*  Author Name    : Prajakta
*  SR_NO		  :
*  Date           : 20 Nov 2013
**********************************************************************************************************/
bool CClientAgentCommonFunctions::GetModulePath(TCHAR *szModulePath, DWORD dwSize)
{
	if (0 == GetModuleFileName(NULL, szModulePath, dwSize))
	{
		return false;
	}

	if (_tcsrchr(szModulePath, _T('\\')))
	{
		*_tcsrchr(szModulePath, _T('\\')) = 0;
	}
	return true;
}
//
///***************************************************************************
//Function Name  : IsPathWardWizDirPath
//Description    : Check if Path belongs to wardwiz directory
//Author Name    : Ramkrushna Shelke
//Date           : 9-11-2015
//****************************************************************************/
// bool CClientAgentCommonFunctions::IsPathWardWizDirPath(CString csFilefolderPath)
//{
//	bool bReturn = false;
//
//	try
//	{
//		//Check here if the path is lnk ( shortcut )
//		if (csFilefolderPath.Right(4).CompareNoCase(L".lnk") == 0)
//		{
//			TCHAR szModulePath[MAX_PATH] = { 0 };
//			if (GetModulePath(szModulePath, sizeof(szModulePath)))
//			{
//				CString csExpandedPath;
//				csExpandedPath = ExpandShortcut(csFilefolderPath);
//				if (csExpandedPath.Trim().GetLength() != 0)
//				{
//					int iPos = csExpandedPath.ReverseFind(L'\\');
//					if (csExpandedPath.Left(iPos).CompareNoCase(szModulePath) == 0)
//					{
//						bReturn = true;
//					}
//				}
//			}
//		}
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in CWardWizCryptDlg::IsPathBelongsToWardWizDir", 0, 0, true, SECONDLEVEL);;
//	}
//	return bReturn;
//}
//
// CString CClientAgentCommonFunctions::ExpandShortcut(CString& csFilename)
//{
//	CString csExpandedFile = "";
//
//	try
//	{
//		USES_CONVERSION;		// For T2COLE() below
//
//		// Make sure we have a path
//		if (csFilename.IsEmpty())
//		{
//			ASSERT(FALSE);
//			return csExpandedFile;
//		}
//
//		// Get a pointer to the IShellLink interface
//		HRESULT hr;
//		IShellLink* pIShellLink;
//
//		hr = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
//			IID_IShellLink, (LPVOID*)&pIShellLink);
//
//		if (SUCCEEDED(hr))
//		{
//			// Get a pointer to the persist file interface
//			IPersistFile* pIPersistFile;
//			hr = pIShellLink->QueryInterface(IID_IPersistFile, (LPVOID*)&pIPersistFile);
//
//			if (SUCCEEDED(hr))
//			{
//				// Load the shortcut and resolve the path
//				// IPersistFile::Load() expects a UNICODE string
//				// so we're using the T2COLE macro for the conversion
//				// For more info, check out MFC Technical note TN059
//				// (these macros are also supported in ATL and are
//				// so much better than the ::MultiByteToWideChar() family)
//				hr = pIPersistFile->Load(T2COLE(csFilename), STGM_READ);
//
//				if (SUCCEEDED(hr))
//				{
//					WIN32_FIND_DATA wfd = { 0 };
//					hr = pIShellLink->GetPath(csExpandedFile.GetBuffer(MAX_PATH),
//						MAX_PATH,
//						&wfd,
//						SLGP_UNCPRIORITY);
//
//					csExpandedFile.ReleaseBuffer(-1);
//				}
//				pIPersistFile->Release();
//			}
//			pIShellLink->Release();
//		}
//	}
//	catch (...)
//	{
//		AddLogEntry(L"### Exception in ExpandShortcut()", 0, 0, true, SECONDLEVEL);;
//	}
//	return csExpandedFile;
//}
//
// /***************************************************************************
// Function Name  : GetEmptyDrivePath
// Description    : it provides the other empty drive if current drive full.
// percentage.
// Author Name    : Lalit Kumawat
// SR_NO		   :
// Date           : 27th Jun 2015
// ****************************************************************************/
// bool CClientAgentCommonFunctions::IsRequiredSpaceAvailableOnDrive(TCHAR szDrive, CString csFilePath, int iSpaceRatio)
// {
//	 bool bReturn = false;
//	 bool isbSpaceAvailable = false;
//	 try
//	 {
//		 CString csDrive;
//
//		 DWORD64 TotalNumberOfFreeBytes;
//		 csDrive.Format(L"%c:", szDrive);
//
//		 if (PathFileExists(csDrive))
//		 {
//			 if (!GetDiskFreeSpaceEx((LPCWSTR)csDrive, &m_uliFreeBytesAvailable, &m_uliTotalNumberOfBytes, &m_uliTotalNumberOfFreeBytes))
//			 {
//				 isbSpaceAvailable = false;
//				 bReturn = false;
//				 AddLogEntry(L"### Failed in  GetDiskFreeSpaceEx", 0, 0, true, SECONDLEVEL);
//			 }
//
//			 TotalNumberOfFreeBytes = GetTotalNumberOfFreeBytes();
//			 TCHAR szFilePath[255] = { 0 };
//			 DWORD64 dwfileSize = 0;
//			 swprintf_s(szFilePath, _countof(szFilePath), L"%s", csFilePath.GetString());
//			 GetSelectedFileSize(szFilePath, dwfileSize);
//			 dwfileSize = (dwfileSize * iSpaceRatio) / (1024 * 1024);
//			 TotalNumberOfFreeBytes = TotalNumberOfFreeBytes / (1024 * 1024);
//			 if (dwfileSize < TotalNumberOfFreeBytes)
//			 {
//				 m_csEmptyDrive = szDrive;
//				 bReturn = true;
//			 }
//			 else
//			 {
//				 m_dwRequiredSpace = dwfileSize - TotalNumberOfFreeBytes;
//			 }
//		 }
//
//	 }
//	 catch (...)
//	 {
//		 AddLogEntry(L"### Exception in CClientAgentCommonFunctions::IsRequiredSpaceAvailableOnDrive", 0, 0, true, SECONDLEVEL);
//	 }
//
//	 return bReturn;
// }
//
// /***************************************************************************
// Function Name  : GetTotalNumberOfFreeBytes
// Description    : it return the size available on selected drive.
// percentage.
// Author Name    : Lalit Kumawat
// SR_NO		   :
// Date           : 27th Jun 2015
// ****************************************************************************/
// DWORD64 CClientAgentCommonFunctions::GetTotalNumberOfFreeBytes(void)
// {
//	 return m_uliTotalNumberOfFreeBytes.QuadPart;
// }
//
// /***************************************************************************
// Function Name  : GetDefaultDrive
// Description    : it provides functionality to get default working drive.
// percentage.
// Author Name    : Lalit Kumawat
// SR_NO		   :
// Date           : 27th Jun 2015
// ****************************************************************************/
// TCHAR CClientAgentCommonFunctions::GetDefaultDrive(CString csFilePath)
// {
//	 TCHAR driveName;
//	 try
//	 {
//
//		 driveName = csFilePath.GetAt(0);
//
//	 }
//	 catch (...)
//	 {
//		 AddLogEntry(L"### Exception in CClientAgentCommonFunctions::GetDefaultDrive", 0, 0, true, SECONDLEVEL);
//	 }
//	 return driveName;
// }
//
 //bool CClientAgentCommonFunctions::GetSelectedFileSize(TCHAR *pFilePath, DWORD64 &dwFileSize)
 //{
	// DWORD64 dwfolderSize = 0;

	// try
	// {
	//	 if (!PathIsDirectory(pFilePath))
	//	 {

	//		 HANDLE hFile = CreateFile(pFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//		 if (hFile == INVALID_HANDLE_VALUE)
	//		 {
	//			 DWORD dwError = GetLastError();
	//			 AddLogEntry(L"### GetSelectedFileSize : Error in opening file %s", pFilePath);
	//			 return false;
	//		 }
	//		 //Function to handle file size more than 2 GB
	//		 LARGE_INTEGER szLargeInt;
	//		 if (GetFileSizeEx(hFile, &szLargeInt) == FALSE)
	//		 {
	//			 return false;
	//		 }

	//		 dwFileSize = szLargeInt.QuadPart;

	//		 CloseHandle(hFile);
	//	 }
	//	 else
	//	 {
	//		 for (int iListItm = 0; iListItm < static_cast<int>(m_vcsListOfFilePath.size()); iListItm++)
	//		 {

	//			 HANDLE hFile = CreateFile(m_vcsListOfFilePath.at(iListItm), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	//			 if (hFile == INVALID_HANDLE_VALUE)
	//			 {
	//				 DWORD dwError = GetLastError();
	//				 AddLogEntry(L"### GetSelectedFileSize : Error in opening file %s", pFilePath);
	//				 return false;
	//			 }

	//			 //Function to handle file size more than 2 GB
	//			 LARGE_INTEGER szLargeInt;
	//			 if (GetFileSizeEx(hFile, &szLargeInt) == FALSE)
	//			 {
	//				 return false;
	//			 }

	//			 dwfolderSize = dwfolderSize + szLargeInt.QuadPart;
	//			 CloseHandle(hFile);
	//		 }

	//		 dwFileSize = dwfolderSize;
	//	 }
	// }
	// catch (...)
	// {
	//	 AddLogEntry(L"### Exception in CWardWizCryptDlg::GetSelectedFileSize");
	//	 return false;
	// }
	// return true;
 //}

 ///***************************************************************************
 //Function Name  : GetAllSystemDrive
 //Description    : this function provides functionality to get all list of os drive in vector
 //Author Name    : Lalit kumawat
 //Date           : 7-28-2015
 //****************************************************************************/
 //void CClientAgentCommonFunctions::GetAllSystemDrive()
 //{
	// TCHAR cDrive;
	// UINT  uDriveType;
	// TCHAR szDriveRoot[] = _T("x:\\");
	// DWORD dwDrivesOnSystem = GetLogicalDrives();

	// try
	// {
	//	 if (m_vcsListOfDrive.size() > 0)
	//	 {
	//		 m_vcsListOfDrive.clear();
	//	 }

	//	 for (cDrive = 'A'; cDrive <= 'Z'; cDrive++, dwDrivesOnSystem >>= 1)
	//	 {
	//		 if (!(dwDrivesOnSystem & 1))
	//			 continue;

	//		 // Get the type for the next drive, and check dwFlags to determine
	//		 // if we should show it in the list.

	//		 szDriveRoot[0] = cDrive;

	//		 uDriveType = GetDriveType(szDriveRoot);

	//		 if (uDriveType == DRIVE_FIXED)
	//		 {
	//			 m_vcsListOfDrive.push_back(cDrive);
	//		 }
	//	 }
	// }
	// catch (...)
	// {
	//	 AddLogEntry(L">>> Exception in GetAllSystemDrive", 0, 0, true, SECONDLEVEL);
	// }

 //}

 //bool CClientAgentCommonFunctions::IsPathOSReservDirectory(CString csFilefolderPath)
 //{
	// bool bRet = false;
	// TCHAR szOSSystemVolumInfo[MAX_PATH] = { 0 };
	// CString csSystemVolumeFolder = L"";

	// try
	// {
	//	 // s1 contain s2
	//	 swprintf_s(szOSSystemVolumInfo, _countof(szOSSystemVolumInfo), L"%s\\System Volume Information", m_csDriveName.GetString());
	//	 csSystemVolumeFolder = szOSSystemVolumInfo;

	//	 if ((csFilefolderPath.Find(m_csProgramData) != -1) && m_csProgramData != L"")
	//	 {
	//		 return true;
	//	 }
	//	 else if ((csFilefolderPath.Find(m_csWindow) != -1) && m_csWindow != L"")
	//	 {
	//		 return true;
	//	 }
	//	 else if ((csFilefolderPath.Find(m_csModulePath) != -1) && m_csModulePath != L"")
	//	 {
	//		 return true;
	//	 }
	//	 else if ((csFilefolderPath.Find(m_csProgramFilex86) != -1) && m_csProgramFilex86 != L"")
	//	 {
	//		 return true;
	//	 }
	//	 else if ((csFilefolderPath.Find(m_csProgramFile) != -1) && m_csProgramFile != L"")
	//	 {
	//		 return true;
	//	 }
	//	 else if (csFilefolderPath == m_csDriveName || csFilefolderPath == m_csDriveName + L"\\")
	//	 {
	//		 return true;
	//	 }
	//	 else if ((csFilefolderPath.Find(csSystemVolumeFolder) != -1) && csSystemVolumeFolder != L"")
	//	 {
	//		 return true;
	//	 }

	//	 TCHAR szDrive = csFilefolderPath.GetAt(0);
	//	 TCHAR szSystemVolumInfoDir[MAX_PATH] = { 0 };

	//	 swprintf_s(szSystemVolumInfoDir, _countof(szSystemVolumInfoDir), L"%c:\\System Volume Information", szDrive);

	//	 if ((csFilefolderPath.Find(szSystemVolumInfoDir) != -1) || csFilefolderPath == szSystemVolumInfoDir)
	//	 {
	//		 return true;
	//	 }

	// }
	// catch (...)
	// {
	//	 AddLogEntry(L">>> Exception in IsPathBelongsToOSReservDirectory", 0, 0, true, SECONDLEVEL);
	// }

	// return false;
 //}

 //void CClientAgentCommonFunctions::GetEnviormentVariablesForMachine(std::map<CString,CString> &mpEnvironmentVars)
 //{

	// TCHAR		 szWindow[512] = { 0 };
	// TCHAR		 szProgramFile[512] = { 0 };
	// TCHAR		 szProgramFilex86[512] = { 0 };
	// TCHAR		 szAppData4[512] = { 0 };
	// TCHAR		 szDriveName[512] = { 0 };
	// TCHAR		 szArchitecture[512] = { 0 };
	// TCHAR		 szUserProfile[MAX_PATH] = { 0 };
	// TCHAR		 szAppData[MAX_PATH] = { 0 };
	// TCHAR		 szProgramData[MAX_PATH] = { 0 };
	// TCHAR		 szModulePath[MAX_PATH] = { 0 };

	// try
	// {
	//	 GetEnvironmentVariable(TEXT("ALLUSERSPROFILE"), szProgramData, 260);//program data
	//	 mpEnvironmentVars.insert(std::pair<CString, CString>(L"ALLUSERSPROFILE", szProgramData));
	//	 m_csProgramData = szProgramData;

	//	 GetEnvironmentVariable(TEXT("USERPROFILE"), szUserProfile, 511);
	//	 mpEnvironmentVars.insert(std::pair<CString, CString>(L"USERPROFILE", szUserProfile));
	//	 m_csUserProfile = szUserProfile;

	//	 GetEnvironmentVariable(TEXT("windir"), szWindow, 511);//windows
	//	 mpEnvironmentVars.insert(std::pair<CString, CString>(L"windir", szWindow));
	//	 m_csWindow = szWindow;

	//	 GetEnvironmentVariable(TEXT("APPDATA"), szAppData, 511);
	//	 mpEnvironmentVars.insert(std::pair<CString, CString>(L"APPDATA", szAppData));
	//	 m_csAppData = szAppData;

	//	 GetEnvironmentVariable(TEXT("PROCESSOR_ARCHITECTURE"), szAppData4, 511);
	//	 mpEnvironmentVars.insert(std::pair<CString, CString>(L"PROCESSOR_ARCHITECTURE", szAppData4));
	//	 m_csAppData4 = szAppData4;

	//	 GetEnvironmentVariable(TEXT("SystemDrive"), szDriveName, 511);
	//	 mpEnvironmentVars.insert(std::pair<CString, CString>(L"SystemDrive", szDriveName));
	//	 m_csDriveName = szDriveName;

	//	 GetModulePath(szModulePath, MAX_PATH);
	//	 mpEnvironmentVars.insert(std::pair<CString, CString>(L"ModulePath", szModulePath));
	//	 m_csModulePath = szModulePath;

	//	 GetEnvironmentVariable(TEXT("PROCESSOR_ARCHITEW6432"), szArchitecture, 511);
	//	 mpEnvironmentVars.insert(std::pair<CString, CString>(L"PROCESSOR_ARCHITEW6432", szArchitecture));
	//	 m_csArchitecture = szArchitecture;

	//	 GetEnvironmentVariable(TEXT("ProgramFiles"), szProgramFile, 511);
	//	 mpEnvironmentVars.insert(std::pair<CString, CString>(L"ProgramFiles", szProgramFile));
	//	 m_csProgramFile = szProgramFile;

	//	 GetEnvironmentVariable(TEXT("ProgramFiles(x86)"), szProgramFilex86, 511);

	//	 if (!_tcscmp(szProgramFilex86, L""))
	//	 {
	//		 mpEnvironmentVars.insert(std::pair<CString, CString>(L"ProgramFiles(x86)", szProgramFile));
	//		 wcscpy_s(szProgramFilex86,sizeof(szProgramFile), szProgramFile);

	//	 }
	//	 m_csProgramFilex86 = szProgramFilex86;

	// }
	// catch (...)
	// {
	//	 AddLogEntry(L">>> Exception in GetEnviormentVariablesForAllMachine", 0, 0, true, SECONDLEVEL);
	// }

 //}

std::string CClientAgentCommonFunctions::get_string_from_wsz(const wchar_t* pwsz)
{
	char buf[0x400];
	char *pbuf = buf;
	size_t len = wcslen(pwsz) * 2 + 1;

	if (len >= sizeof(buf))
	{
		pbuf = "error";
	}
	else
	{
		size_t converted;
		wcstombs_s(&converted, buf, pwsz, _TRUNCATE);
	}

	return std::string(pbuf);
}