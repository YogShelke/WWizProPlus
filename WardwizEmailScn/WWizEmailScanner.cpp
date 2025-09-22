/**********************************************************************************************************
Program Name          : WWizEmailScanner.cpp
Description           : Scanning Email attachment files.
Author Name			  : Amol J.
Date Of Creation      : 08 Feb 2018
Version No            : 3.1.0.0
***********************************************************************************************************/
#include "stdafx.h"
#include "WWizEmailScanner.h"

CString GetSQLiteDBFilePath();

CWWizEmailScanner::CWWizEmailScanner() :
	m_bRescan(false)
	, m_objScanCom(SERVICE_SERVER, true, 2)
	, m_hZip(NULL)
	, UnzipFile(NULL)
	, m_ViruName(L"")
{
	
}

CWWizEmailScanner::~CWWizEmailScanner()
{
	if (m_hZip)
	{
		FreeLibrary(m_hZip);
		m_hZip = NULL;
	}
}

/***********************************************************************************************************
*  Function Name  : ScanData
*  Description    : Scan the received file.
*  Author Name    : Amol J.
*  Date			  : 08 Feb 2018
************************************************************************************************************/
void CWWizEmailScanner::ScanData(CString csFilePath, VirusFoundZipDataBase &mapVirusFound)
{
	m_iThreatAction = 0;
	try
	{
		CString csVirusName(L"");
		DWORD dwISpywareID = 0;
		CString csActionID;
		DWORD dwISpyID = 0;
		TCHAR szVirusName[MAX_PATH] = { 0 };
		DWORD dwSignatureFailedToLoad = 0;
		DWORD dwActionTaken = 0x00;

		if (!csFilePath)
			return;

		if (ScanFile(csFilePath, szVirusName, dwISpyID, dwSignatureFailedToLoad, dwActionTaken, m_bRescan))
		{
			if (dwISpyID >= 0)
			{
				csVirusName = szVirusName;
				m_ViruName = szVirusName;
				dwISpywareID = dwISpyID;

				CString csStatus = L"";// theApp.m_objwardwizLangManager.GetString(L"IDS_FILE_SKIPPED");
				csActionID = "IDS_FILE_SKIPPED";
				switch (dwActionTaken)
				{
				case FILESKIPPED:
					//csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_FILE_SKIPPED");
					csActionID = "IDS_FILE_SKIPPED";
					m_iThreatAction = 2;
					break;
				case FILEQURENTINED:
					//csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_QUARANTINED");
					csActionID = "IDS_CONSTANT_THREAT_QUARANTINED";
					m_dwVirusCleanedCount++;
					m_iThreatAction = 2;
					break;
				case FILEREPAIRED:
					//csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_REPAIRED");
					csActionID = "IDS_CONSTANT_THREAT_REPAIRED";
					m_dwVirusCleanedCount++;
					m_iThreatAction = 3;
					break;
				case LOWDISKSPACE:
					//csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_CONSTANT_THREAT_LOWDISC_SPACE");
					csActionID = "IDS_CONSTANT_THREAT_LOWDISC_SPACE";
					m_dwVirusCleanedCount++;
					m_iThreatAction = 4;
					break;
				case FILEREBOOTQUARENTINE:
					//csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_QUARANTINE");
					m_dwVirusCleanedCount++;
					m_iThreatAction = 5;
					break;
				case FILEREBOOTREPAIR:
					//csStatus = theApp.m_objwardwizLangManager.GetString(L"IDS_SCAN_REBOOT_REPAIR");
					m_dwVirusCleanedCount++;
					m_iThreatAction = 6;
					break;

				}
				
				int iIndex = static_cast<int>(mapVirusFound.size());
				iIndex++;

				mapVirusFound[iIndex].dwRepairID = dwActionTaken;
				mapVirusFound[iIndex].strFilePath = csFilePath;
				mapVirusFound[iIndex].strVirusName = csVirusName;
			}
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizEmailScanner::ScanData", 0, 0, true, SECONDLEVEL);
	}
}

/*********************************************************************************************************
*  Function Name  :	ScanFile
*  Description    :	Exported funtion to scan file
Scanning using service, the aim is to keep the database in common memory location.
*  Author Name    : Neha Gharge, Ram Shelke
*  Date           : 22 Jun 2014
**********************************************************************************************************/
bool CWWizEmailScanner::ScanFile(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID, DWORD &dwFailedToLoadSignature, DWORD &dwActionTaken, bool bRescan)
{
	try
	{
		bool bSendFailed = false;

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = SCAN_FILE;
		wcscpy_s(szPipeData.szFirstParam, szFilePath);
		if (!m_objScanCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in CWardwizEmailScanner::ScanFile", 0, 0, true, SECONDLEVEL);
			bSendFailed = true;
		}
		if (!m_objScanCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CWardwizEmailScanner::ScanFile", 0, 0, true, SECONDLEVEL);
			bSendFailed = true;
		}
		if (szPipeData.dwValue == 1)
		{
			dwActionTaken = szPipeData.dwSecondValue;
			_tcscpy(szVirusName, szPipeData.szSecondParam);
			dwISpyID = (*(DWORD *)&szPipeData.byData[0]);
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScanner::ScanFile, File: %s", szFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name  : ExtractZipForScanning
Description    : Function which extracts the zip folder for scanning purpose.
Author Name    : Ramkrushna Shelke
SR.NO		   : EMAILSCANPLUGIN_0033
Date           : 12th Feb 2018
****************************************************************************/
int CWWizEmailScanner::ExtractZipForScanning(TCHAR *pZipFile, CString csSenderName, CString csAttachmentName)
{
	m_iThreatAction = 0;

	if (!UnzipFile)
		return m_iThreatAction;

	AddLogEntry(L">>> CWardwizEmailScanner::ExtractZipForScanning Extract ZipFolder For Scanning", 0, 0, true, FIRSTLEVEL);
	bool	bZipModified = false;
	bool	bModified = false;
	DWORD	dwZipFileCount = 0x00;
	DWORD	dwModifiedCount = 0x00;
	DWORD	dwStatus = 0x00;
	int		iSize = 0x00;
	DWORD   dwAction = 0;
	TCHAR	szUnzipPath[512] = { 0 };
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
			return m_iThreatAction;
	}
	else
	{
		_tcscpy_s(szUnzipPath, _countof(szUnzipPath), pZipFile);
	}
	VirusFoundDB.clear();
	ScanZipFiles(szUnzipPath, bZipModified, dwModifiedCount, VirusFoundDB);

	iSize = static_cast<int>(VirusFoundDB.size());

	CString	strFileName = L"";
	if (iSize)
	{
		CString	strFileName = L"";
		TCHAR	szFileName[512] = { 0 };
		TCHAR	*pFileName = NULL;

		pFileName = wcsrchr(pZipFile, '\\');
		if (pFileName)
		{
			pFileName++;
			strFileName.Format(L"%s", pFileName);
		}		
		//send notification 2 tray
		if (!SendEmailData2Tray(SHOW_EMAILSCAN_TRAY_POPUP, VirusFoundDB[1].strVirusName, csAttachmentName, csSenderName, VirusFoundDB[1].dwRepairID))
		{
			AddLogEntry(L"### Error in CWardwizEmailScanner::ExtractZipForScanning SendEmailData2Tray", 0, 0, true, SECONDLEVEL);
		}
		Sleep(10);
	}
	//Delete all unzipped file(s)
	RemoveAllZipFiles(szUnzipPath);
	RemoveFilesUsingSHFileOperation(szUnzipPath);

	VirusFoundDB.clear();
	return m_iThreatAction;
}

/***************************************************************************
Function Name  : GetFileType
Description    : Getting file type.
Author Name    : Ramkrushna Shelke
Date           : 07th Aug 2014
****************************************************************************/
FILETYPE CWWizEmailScanner::GetFileType(LPCTSTR pszFilePath, DWORD &dwSubType, DWORD &dwVersion)
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

/***************************************************************************
Function Name  : ScanZipFiles
Description    : Function which takes zip file as a input path starts enumeration
of files for scanning.
Author Name    : Ramkrushna Shelke
SR.NO		   : EMAILSCANPLUGIN_0034
Date           : 07th Aug 2014
****************************************************************************/
void CWWizEmailScanner::ScanZipFiles(TCHAR *pZipPath, bool &bModified, DWORD &dwCount, VirusFoundZipDataBase &mapVirusFound)
{
	try
	{
		AddLogEntry(L">>> CWardwizEmailScanner::ScanZipFiles zip file started", 0, 0, true, FIRSTLEVEL);
		TCHAR	szTemp[512] = { 0 };
		TCHAR	szFile[512] = { 0 };
		CFileFind	finder;

		bModified = false;
		dwCount = 0x00;
		DWORD	dwAttributes = 0;
		CString strWildcard(pZipPath);
		dwAttributes = GetFileAttributes(pZipPath);
		if (((FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_DIRECTORY) == dwAttributes) || (FILE_ATTRIBUTE_DIRECTORY == dwAttributes) || ((FILE_ATTRIBUTE_NOT_CONTENT_INDEXED | FILE_ATTRIBUTE_DIRECTORY) == dwAttributes))
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
		while (bWorking)
		{
			bWorking = finder.FindNextFile();

			if (finder.IsDots())
				continue;

			if (finder.IsDirectory())
			{
				wsprintf(szFile, L"%s", finder.GetFilePath());
				ScanZipFiles(szFile, bModified, dwCount, mapVirusFound);
			}
			else
			{
				wsprintf(szFile, L"%s", finder.GetFilePath());
				ScanData(szFile, mapVirusFound); //send file to scan.

				memset(szFile, 0x00, sizeof(TCHAR) * 512);
			}
		}
		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScanner::ScanZipFiles", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : SendEmailData2Tray
Description    : Function which sends threat name, attachment name, senders address,
action
Author Name    : Ramkrushna Shelke
Date           : 07th Aug 2014
****************************************************************************/
bool CWWizEmailScanner::SendEmailData2Tray(int iMessage, CString csThreatName, CString csAttachmentName, CString csSenderAddr, DWORD dwAction, bool bWait)
{
	ISPY_PIPE_DATA szPipeData = { 0 };
	memset(&szPipeData, 0, sizeof(ISPY_PIPE_DATA));

	//Issue Number 474. Rajil Yadav 10/06/2014

	szPipeData.iMessageInfo = iMessage;
	szPipeData.dwValue = dwAction;
	wcscpy_s(szPipeData.szFirstParam, csThreatName);
	wcscpy_s(szPipeData.szSecondParam, csAttachmentName);
	wcscpy_s(szPipeData.szThirdParam, csSenderAddr);
	CISpyCommunicator objCom(TRAY_SERVER);
	if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
	{
		AddLogEntry(L"### Failed to set data in CWardwizEmailScanner::SendEmailData2Tray", 0, 0, true, SECONDLEVEL);
		return false;
	}

	if (bWait)
	{
		if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to set data in CWardwizEmailScanner::SendEmailData2Tray", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

/***************************************************************************
Function Name  : RemoveAllZipFiles
Description    : Function which is used to remove zip file path as given
in function argument.
Author Name    : Ramkrushna Shelke
Date           : 07th Aug 2014
****************************************************************************/
DWORD CWWizEmailScanner::RemoveAllZipFiles( TCHAR *pFile )
{
	try
	{
		AddLogEntry(L">>> CWardwizEmailScanner::RemoveAllZipFiles all zip file", 0, 0, true, FIRSTLEVEL);
		TCHAR	szTemp[512] = { 0 };
		TCHAR	szFile[512] = { 0 };
		bool	bDelete = false;
		CFileFind	finder;

		wsprintf(szTemp, L"%s\\*.*", pFile);
		BOOL bWorking = finder.FindFile(szTemp);
		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDirectory())
				continue;

			if (finder.IsDots())
				continue;

			wsprintf(szFile, L"%s", finder.GetFilePath());
			if (RemoveFileOnly(szFile))
				bDelete = true;

			memset(szFile, 0x00, sizeof(TCHAR) * 512);
		}

		finder.Close();
		if (bDelete)
			return 0x01;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScanner::RemoveAllZipFiles", 0, 0, true, SECONDLEVEL);
	}
	return 0x00;
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
DWORD CWWizEmailScanner::RemoveFilesUsingSHFileOperation(TCHAR *pFolder)
{
	HRESULT			hr = 0x00;
	__try
	{
		SHFILEOPSTRUCT	sfo = { 0 };

		TCHAR	szTemp[512] = { 0 };

		wsprintf(szTemp, L"%s\\*\0", pFolder);

		sfo.hwnd = NULL;
		sfo.wFunc = FO_DELETE;
		sfo.pFrom = szTemp;
		sfo.pTo = NULL;
		sfo.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;

		hr = SHFileOperation(&sfo);
		if (!hr)
		{
			_wrmdir((wchar_t *)pFolder);
			RemoveDirectory(pFolder);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScanner::RemoveFilesUsingSHFileOperation", 0, 0, true, SECONDLEVEL);
	}
	return hr;
}

/***************************************************************************
Function Name  : RemoveFileOnly
Description    : Function which delets the file from given path.
Author Name    : Ramkrushna Shelke
SR.NO		   : EMAILSCANPLUGIN_0036
Date           : 07th Aug 2014
****************************************************************************/
DWORD CWWizEmailScanner::RemoveFileOnly(TCHAR *pFile)
{
	__try
	{
		if (DeleteFile(pFile))
			return 0;

		Sleep(10);

		if (_tremove(pFile) == 0)
			return 0;

		Sleep(10);
		if (_tunlink(pFile) == 0)
			return 0;

		Sleep(10);
		if (DeleteFile(pFile))
			return 0;

		Sleep(10);
		MoveFileEx(pFile, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardwizEmailScanner::RemoveFileOnly", 0, 0, true, SECONDLEVEL);
	}
	return 1;
}

/***************************************************************************
Function Name  : LoadRequiredDLLs
Description    : Fucntion which Load's s VBEXTRACT.DLL and calls the LoadSignature
exported function.
Author Name    : Ramkrushna Shelke
SR.NO			 : EMAILSCANPLUGIN_0028
Date           : 07th Aug 2014
****************************************************************************/
bool CWWizEmailScanner::LoadRequiredDLLs()
{

	TCHAR	szModulePath[512];
	_tcscpy(szModulePath, GetWardWizPathFromRegistry());

	TCHAR	szTemp[512] = { 0 };
	memset(szTemp, 0x00, sizeof(TCHAR) * 512);
	wsprintf(szTemp, L"%sVBEXTRACT.DLL", szModulePath);

	m_hZip = LoadLibrary(szTemp);
	if (!m_hZip)
	{
		AddLogEntry(L"### LoadLibrary fails in CWardwizEmailScanner::LoadRequiredDLLs, FilePath: [%s]", szTemp, 0, true, SECONDLEVEL);
		m_hZip = NULL;
		return false;
	}

	UnzipFile = (UNZIPFILE)GetProcAddress(m_hZip, "UnzipFile");
	if (!UnzipFile)
	{
		AddLogEntry(L"### GetProcAddress UnzipFile fails in CWardwizEmailScanner::LoadRequiredDLLs, FilePath: [%s]", szTemp, 0, true, SECONDLEVEL);
		FreeLibrary(m_hZip);
		m_hZip = NULL;
		return false;
	}

	return true;
}

/**********************************************************************************************************
*  Function Name  :	InsertDataToTable
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into SQLite tables.
*  Author Name    : Amol Jaware.
*  Date           : 19 Feb 2018
**********************************************************************************************************/
INT64 CWWizEmailScanner::InsertDataToTable(const char* szQuery)
{
	try
	{
		CString	csWardWizModulePath = GetSQLiteDBFilePath();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		m_objSqlDb.SetDataBaseFilePath(dbPath.m_psz);

		m_objSqlDb.Open();

		int iRows = m_objSqlDb.ExecDML(szQuery);
		Sleep(200);
		INT64 iLastRowId = m_objSqlDb.GetLastRowId();

		m_objSqlDb.Close();

		return iLastRowId;
	}
	catch (...)
	{
		m_objSqlDb.Close();
		AddLogEntry(L"### Exception in CWardwizEmailScanner::InsertDataToTable", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/**********************************************************************************************************
*  Function Name  :	GetSQLiteDBFilePath
*  Description    :	Helper function to get Current working directory path
*  Author Name    : Amol Jaware.
*  Date           : 19 Feb 2018
**********************************************************************************************************/
CString GetSQLiteDBFilePath()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		GetModuleFileName(NULL, szModulePath, MAX_PATH);

		TCHAR *szTemp = _tcsrchr(szModulePath, L'\\');
		szTemp[0] = '\0';

		return(CString(szModulePath));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in value CWardwizEmailScanner::GetSQLiteDBFilePath", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}