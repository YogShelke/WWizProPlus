/********************************************************************************************************** 
   Program Name          : HiddenFilesNFoldersScan.cpp
   Description           : Class which has functionality to Scans Hidden Files and Folders
   Author Name           : Ramkrushna Shelke                                                                              
   Date Of Creation      : 7/4/2014
   Version No            : 1.0.0.5
   Special Logic Used    : 
   Modification Log      :           
   1. Name    : Description
***********************************************************************************************************/

#include "StdAfx.h"
#include "HiddenFilesNFoldersScan.h"
#include "WindowsDef.h"
#include "ITinRKScn.h"

/***************************************************************************
  Function Name  : HiddenFilesNFoldersScan
  Description    : Constructor
  Author Name    : Ramkrushna Shelke
  S.R. No        : 
  Date           : 7/4/2014
****************************************************************************/
CHiddenFilesNFoldersScan::CHiddenFilesNFoldersScan(void):
		m_bStopScan(false)
	,	m_dwPercentage(0)
{
}
/***************************************************************************
  Function Name  : CHiddenFilesNFoldersScan
  Description    : Destructor
  Author Name    : Ramkrushna Shelke
  S.R. No        : 
  Date           : 7/4/2014
****************************************************************************/
CHiddenFilesNFoldersScan::~CHiddenFilesNFoldersScan(void)
{
}

/***************************************************************************
  Function Name  : StopScan
  Description    : Makes m_bStopScan true.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_047
  Date           : 7/4/2014
****************************************************************************/
void CHiddenFilesNFoldersScan::StopScan(void)
{
	m_bStopScan = true;
}

/***************************************************************************
  Function Name  : InitializeVariables
  Description    : Initialize all the variables.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_048
  Date           : 7/4/2014
****************************************************************************/
bool CHiddenFilesNFoldersScan::InitializeVariables()
{
	m_bStopScan = false;
	m_vFilesNFoldersListByNTDLL.clear();
	m_vFilesNFoldersListByAPI.clear();
	m_dwFilesCountFirstPhase = 0;
	m_dwFilesCountSecondPhase = 0;
	m_dwFileNFolderRootkitFound = 0;
	m_dwPercentage = 0;
	m_dwPrevCompletedPerc = 0x00 ;
	m_dwFilesCountFirstPhase = 0x00 ;
	m_dwFileNFolderRootkitFound = 0x00 ;
	return true;
}

/***************************************************************************
  Function Name  : ScanDrive
  Description    : Scans for a particular drive and calls GetFolderList(). 
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_049
  Date           : 7/4/2014
****************************************************************************/
bool CHiddenFilesNFoldersScan::ScanDrive(const WCHAR *wcDriveLetter)
{
	if(m_bStopScan)
	{
		return false;
	}


	WCHAR szDrivePath[1000] = {0};
	swprintf_s(szDrivePath, _countof(szDrivePath), L"%s", wcDriveLetter);
	//swprintf_s(szDrivePath, _countof(szDrivePath), L"%s", L"E:\\Testing");

	CString csTemp;
	csTemp.Format(L"Scanning Windows Folder: '%s' ", wcDriveLetter);
	AddLogEntry(csTemp);

	HMODULE hNTDLL = LoadLibrary(L"ntdll");
	if(!hNTDLL)
	{
		return false;
	}

	ZwQueryDirectoryFile = (ZWQUERYDIRFUNC)GetProcAddress(hNTDLL, "ZwQueryDirectoryFile");
	if(!ZwQueryDirectoryFile)
	{
		FreeLibrary(hNTDLL);
		return false;
	}

	GetFolderList(szDrivePath);

	FreeLibrary(hNTDLL);
	return true;
}

/***************************************************************************
  Function Name  : GetFolderList
  Description    : Retrive the List of Folders to Scan.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_050
  Date           : 7/4/2014
****************************************************************************/
bool CHiddenFilesNFoldersScan::GetFolderList(WCHAR *wsFolderPath)
{
	OutputDebugString(L"In GetFolderList ");
	OutputDebugString(wsFolderPath);
	OutputDebugString(L"\n");

	if(m_bStopScan)
	{
		return false;
	}

	HANDLE hDirectory = CreateFile(wsFolderPath, FILE_LIST_DIRECTORY, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	if(hDirectory == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	IO_STATUS_BLOCK IO_Status_Block = {0};
	FILE_DIRECTORY_INFORMATION oFileDirInfo = {0};
	// issue resolved by ram, to resolve vs2008 to vs2013 solution migration
	WWNTSTATUS ntStatus = ZwQueryDirectoryFile(hDirectory, 0, 0, 0, &IO_Status_Block, &oFileDirInfo, sizeof(oFileDirInfo), FileDirectoryInformation, TRUE, NULL, TRUE);

	while(ntStatus == STATUS_SUCCESS && !m_bStopScan)
	{
		if(oFileDirInfo.FileNameLength != 0)
		{
			if((oFileDirInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			{
				//Add junction point check to avoid crash on vista
				if((wcscmp(oFileDirInfo.FileName, L".") != 0) && (wcscmp(oFileDirInfo.FileName, L"..") != 0) && 					// Skipping symbolic links/junction points
					(oFileDirInfo.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != FILE_ATTRIBUTE_REPARSE_POINT)
				{
					
					WCHAR *cFullFileName = new WCHAR[MAX_PATH];
					wmemset(cFullFileName, 0, MAX_PATH);
					wcscpy_s(cFullFileName, MAX_PATH, wsFolderPath);
					wcscat_s(cFullFileName, MAX_PATH, L"\\");
					wcscat_s(cFullFileName, MAX_PATH, oFileDirInfo.FileName);
					
					/*
						Date			:	23 - 01 - 2015.
						Resolved by	:	Niranjan Deshak.
						Issue			:	In Antirootkit Scan UI stuck at 33%(In case of all options checked ) or at 99%(In case of only one option is selected).
					*/
					if( m_dwFilesCountFirstPhase == m_dwFilesCountSecondPhase )
					{
						break;
					}				
					m_dwFilesCountSecondPhase++;
					CalculateScanPercentage();
					
					if((oFileDirInfo.FileAttributes & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN)
					{
						CheckIfRootKit(wsFolderPath, oFileDirInfo.FileName, true);
					}

					GetFolderList(cFullFileName);
					
					delete [] cFullFileName;
					cFullFileName = NULL;
				}
			}
			else
			{
				/*
					Date			:	23 - 01 - 2015.
					Resolved by	:	Niranjan Deshak.
					Issue			:	In Antirootkit Scan UI stuck at 33%(In case of all options checked ) or at 99%(In case of only one option is selected).
				*/
				if( m_dwFilesCountFirstPhase == m_dwFilesCountSecondPhase )
				{
						break;
				}
				m_dwFilesCountSecondPhase++;
				CalculateScanPercentage();

				if((oFileDirInfo.FileAttributes & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN)
				{
					//Check the file extension here if its not .exe, .dll, .sys then avoid.
					if(CheckFileExtention(oFileDirInfo.FileName))
					{
						CheckIfRootKit(wsFolderPath, oFileDirInfo.FileName, true);
					}
				}
			}
		}
		memset(&IO_Status_Block, 0, sizeof(IO_Status_Block));
		memset(&oFileDirInfo, 0, sizeof(oFileDirInfo));
		ntStatus = ZwQueryDirectoryFile(hDirectory, 0, 0, 0, &IO_Status_Block, &oFileDirInfo, sizeof(oFileDirInfo), FileDirectoryInformation, TRUE, NULL, FALSE);
	}

	CloseHandle(hDirectory);
	return true;
}

/***************************************************************************
  Function Name  : CreateDriveTreeByAPIs
  Description    : Function Which gets all the Drives from the WINAPI.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_051
  Date           : 7/4/2014
****************************************************************************/
bool CHiddenFilesNFoldersScan::CreateDriveTreeByAPIs(TCHAR *chDrive)
{
	m_vFilesNFoldersListByAPI.clear();

	WCHAR szDrivePath[100] = {0};
	swprintf_s(szDrivePath, _countof(szDrivePath), L"%s", chDrive);

	if(!EnumerateFolderByAPIs(szDrivePath, m_vFilesNFoldersListByAPI))
	{
		return (false);
	}
	return (true);
}

/***************************************************************************
  Function Name  : EnumerateFolderByAPIs
  Description    : Recursive function which gets the list of files present in folder
				   and stores in vector using WINAPI FindFirstFile, FindNextFile
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_052
  Date           : 7/4/2014
****************************************************************************/
bool CHiddenFilesNFoldersScan::EnumerateFolderByAPIs(LPTSTR szPath, vecFILESNFOLDERSINFO &vFilesList)
{
	OutputDebugString(L"In EnumerateFolderByAPIs ");
	OutputDebugString(szPath);
	OutputDebugString(L"\n");

	HANDLE hSearch = INVALID_HANDLE_VALUE;
	WIN32_FIND_DATA FoundData ={0};
	bool bSuccess = true;
	TCHAR szHoldPath[MAX_PATH * 2]={0};

	if(m_bStopScan)
	{
		return (true);
	}

	if(_tcslen(szPath) + 2 >= _countof(szHoldPath))
	{
		AddLogEntry(_T("Insufficient buffer to make search path: [%s]"), szPath, 0, true, SECONDLEVEL);
		return (false);
	}

	_tcscpy_s(szHoldPath, _countof(szHoldPath), szPath);
	_tcscat_s(szHoldPath, _countof(szHoldPath), _T("\\*.*"));

	hSearch = FindFirstFile(szHoldPath, &FoundData);
	if(INVALID_HANDLE_VALUE == hSearch)
	{
		//pParent->bEnumSuccess = FALSE;
		return (true);
	}

	do
	{
		if(m_bStopScan)
		{
			bSuccess = true;
			break;
		}

		if((FoundData.cFileName[0] == 0x2E && FoundData.cFileName[1] == 0x00) ||
			(FoundData.cFileName[0] == 0x2E && FoundData.cFileName[1] == 0x2E &&
			FoundData.cFileName[2] == 0x00))
		{
			SetLastError(0);
			continue;
		}

		if((FoundData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
		{
			if(_tcslen(szPath) + _tcslen(FoundData.cFileName) + 1 >= _countof(szHoldPath))
			{
				AddLogEntry(_T("Insufficient buffer, Parent: [%s], Child: [%s]"), szPath, FoundData.cFileName, true, SECONDLEVEL);
				bSuccess = false;
				break;
			}

			_tcscpy_s(szHoldPath, _countof(szHoldPath), szPath);
			_tcscat_s(szHoldPath, _countof(szHoldPath), _T("\\"));
			_tcscat_s(szHoldPath, _countof(szHoldPath), FoundData.cFileName);

			_tcslwr_s(FoundData.cFileName, _countof(FoundData.cFileName));

			if((FoundData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN)
			{
				//make here the list of folders 
				_tcslwr(szHoldPath);
				m_vFilesNFoldersListByAPI.push_back(szHoldPath);
			}

			m_dwFilesCountFirstPhase++;

			OutputDebugString(szHoldPath);
			OutputDebugString(L"\n");

			if(!EnumerateFolderByAPIs(szHoldPath, vFilesList))
			{
				bSuccess = false;
				break;
			}
		}
		else
		{
			_tcslwr_s(FoundData.cFileName, _countof(FoundData.cFileName));

			//Check the file extension here if its not .exe, .dll, .sys then avoid.
			if(CheckFileExtention(FoundData.cFileName))
			{
				WCHAR *cFullFileName = new WCHAR[MAX_PATH * 2];
				wmemset(cFullFileName, 0, MAX_PATH * 2);
				wcscpy_s(cFullFileName, MAX_PATH * 2, szPath);
				wcscat_s(cFullFileName, MAX_PATH * 2, L"\\");
				wcscat_s(cFullFileName, MAX_PATH * 2, FoundData.cFileName);

				if((FoundData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) == FILE_ATTRIBUTE_HIDDEN)
				{
					//make here the list of folders 
					_tcslwr(cFullFileName);
					m_vFilesNFoldersListByAPI.push_back(cFullFileName);
				}

				m_dwFilesCountFirstPhase++;

				OutputDebugString(cFullFileName);
				OutputDebugString(L"\n");


				delete [] cFullFileName;
				cFullFileName = NULL;
			}
		}

		SetLastError(0);
	} while(FindNextFile(hSearch, &FoundData));

	if(ERROR_NO_MORE_FILES != GetLastError())
	{
		//pParent->bEnumSuccess = FALSE;
		SetLastError(0);
	}

	FindClose(hSearch);
	return (bSuccess);
}

/***************************************************************************
  Function Name  : CheckIfRootKit
  Description    : Checks if Root Kit present in Hidden Files and Folders. If Found Increment m_dwFileNFolderRootkitFound.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_053
  Date           : 7/4/2014
****************************************************************************/
bool CHiddenFilesNFoldersScan::CheckIfRootKit(const WCHAR *szParentName, const WCHAR *szModuleName, bool bDirectory)
{
	OutputDebugString(L"In CHiddenFilesNFoldersScan::CheckIfRootKit\n");

	bool bReturn = false;
	try
	{
		if(NULL == szParentName || NULL == szModuleName || 0 == *szParentName || 0 == *szModuleName)
		{
			return bReturn;
		}

		if((szModuleName[0] == _T('$')) ||(_tcslen(szParentName) >= 4 && szParentName[3] == _T('$')))
		{
			return bReturn;
		}

		CString csPath;
		csPath.Format(L"%s\\%s", szParentName, szModuleName);
		csPath.MakeLower();

		vecFILESNFOLDERSINFO::iterator itFound = std::find(m_vFilesNFoldersListByAPI.begin(),					
			m_vFilesNFoldersListByAPI.end(), csPath);

		if (itFound == m_vFilesNFoldersListByAPI.end()) 
		{
			OutputDebugString(L"Rootkit Found: ");
			OutputDebugString(csPath);
			OutputDebugString(L"\n");

			AddLogEntry(L"Rootkit Found: %s", csPath, 0, true, SECONDLEVEL);
			m_dwFileNFolderRootkitFound++;
			
			//if(!theApp.SendRootKitInfoToGUI(SHOWVIRUSFOUND, csPath, L"", 0, 0x03))
			//{
			//	AddLogEntry(L"### Failed to send virus entry to UI: %s", csPath);
			//}

			bReturn = true;
		} 
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CHiddenFilesNFoldersScan::CheckIfRootKit", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
  Function Name  : CheckFileExtention
  Description    : If file Extension is "sys" or "dll" or "exe" return true.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_054
  Date           : 7/4/2014
****************************************************************************/
bool CHiddenFilesNFoldersScan::CheckFileExtention(CString csFilePath)
{
	bool bReturn = false;
	CString csExt = csFilePath.Right(csFilePath.GetLength() - (csFilePath.Find(_T('.')) + 1));
	csExt.MakeLower();
	if(	csExt == L"sys" ||
		csExt == L"dll" ||
		csExt == L"exe" )
	{
		bReturn = true;
	}
	return bReturn;
}

/***************************************************************************
  Function Name  : SetPreviousPercentage
  Description    : Sets Percentage to Specified value.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_055
  Date           : 7/4/2014
****************************************************************************/
void CHiddenFilesNFoldersScan::SetPreviousPercentage(DWORD dwPercentage)
{
	m_dwPrevCompletedPerc = dwPercentage ;
}

/***************************************************************************
  Function Name  : CalculateScanPercentage
  Description    : Calculates the Percentage of scan Comleted. Calls GetScanPercentage
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_056
  Date           : 7/4/2014
****************************************************************************/
DWORD CHiddenFilesNFoldersScan::CalculateScanPercentage()
{
	m_dwPercentage = 0;
	if(m_dwFilesCountFirstPhase != 0)
	{
		m_dwPercentage =  static_cast<int>(((float)m_dwFilesCountSecondPhase / m_dwFilesCountFirstPhase ) *100);
		if( m_dwPercentage > 99 )
			m_dwPercentage = 99;
	}

	DWORD dwpercentage = ( theApp.m_EachScanMaxPercentage * m_dwPercentage ) / 100 ;
	
	theApp.m_dwpercentage = m_dwPrevCompletedPerc + dwpercentage;

	//Date	:	19 - 12 - 2014
	//Issue :	"Hidden items in files and folders" scanning completed but pop up doesn't come.
	//			On Vista above, OS hidden files more as compared to XP so it's taking more time to show 
	//			"Scanning completed" pop up.
	//			After 99%, It'll take 3 to 4 minutes to show "Scanning completed" pop up.
	if( theApp.m_dwpercentage > 99 )
			theApp.m_dwpercentage = 99;

	theApp.GetScanPercentage();

	return m_dwPercentage;
}

/***************************************************************************
  Function Name  : GetScanPercentage
  Description    : Returns the Scan Percentage.
  Author Name    : Ramkrushna Shelke
  S.R. No        : WRDWIZRKSCNDLL_057
  Date           : 7/4/2014
****************************************************************************/
DWORD CHiddenFilesNFoldersScan::GetScanPercentage()
{
	return m_dwPercentage;
}
