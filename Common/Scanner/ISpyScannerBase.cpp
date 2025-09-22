/**********************************************************************************************************                     
	  Program Name          : ISpyScannerBase.cpp
	  Description           : Base class for Scanner
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 20 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Ramkrushna           Created Wrapper Base class for iSpy AV & Clam Scanner 
							  functionality implementation
	  Modification Date		: 6 Jan 2015 Neha Gharge
	  Modification			: Clam And WardWiz Scanner Handle by preprocessor
***********************************************************************************************************/
#include "ISpyScannerBase.h"
#include "WWizIndexer.h"

#define	szpIpsy		"WRDWIZ"

CISpyScannerBase::CISpyScannerBase(void):
  m_dwFilesIndexed(0)
  , m_bStopScan(false)
{
	m_eScanLevel = CLAMSCANNER;
}

CISpyScannerBase::~CISpyScannerBase(void)
{

}

/***************************************************************************
Function Name  : StartScan
Description    : Function to start different scanning type as per UI request.
				 This function is handled by STRUCTURED EXCEPTION HANDLING.
Author Name    : Ramkrushna Shelke
Date           : 14/12/2015
****************************************************************************/
bool CISpyScannerBase::StartScan(SCANTYPE eScanType, DWORD dwAntirootScanOpt, LPCTSTR szParam)
{
	bool bReturn = false;
	__try
	{
		bReturn = StartScanSEH(eScanType, dwAntirootScanOpt, szParam);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Failed CWardwizSCANNERBase::StartScan", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

bool CISpyScannerBase::StartScanSEH(SCANTYPE eScanType, DWORD dwAntirootScanOpt,LPCTSTR szParam)
{
	try
	{

		CStringArray csaScanPaths;

		m_eScanType = eScanType;

		switch (eScanType)
		{
		case ANTIROOTKITSCAN:
			if (m_eScanLevel == WARDWIZSCANNER)
			{
				if (!m_objITinAntirootKitScanner.StartAntirootScan(eScanType, dwAntirootScanOpt, &m_objWardWizScanner.m_objISpyScanner))
				{
					AddLogEntry(L"### Failed CWardwizSCANNERBase::StartAntirootScan", 0, 0, true, SECONDLEVEL);
				}
				break;
			}
			else
			{
				if (!m_objITinAntirootKitScanner.StartAntirootScan(eScanType, dwAntirootScanOpt, &m_objClamScanner.m_objISpyScanner))
				{
					AddLogEntry(L"### Failed CWardwizSCANNERBase::StartAntirootScan", 0, 0, true, SECONDLEVEL);
				}
				break;
			}
		case QUICKSCAN:
		case FULLSCAN:
		case CUSTOMSCAN:
		case USBSCAN:
			if (!GetScanningPaths(szParam, csaScanPaths))
			{
				return false;
			}

			if (m_eScanLevel == WARDWIZSCANNER)
			{
				if (!m_objWardWizScanner.StartScanning(eScanType, csaScanPaths))
				{
					AddLogEntry(L"### Failed CWardwizSCANNERBase::StartScan", 0, 0, true, SECONDLEVEL);
				}
				break;
			}
			else
			{
				if (!m_objClamScanner.StartScanning(eScanType, csaScanPaths))
				{
					AddLogEntry(L"### Failed CWardwizSCANNERBase::StartScan", 0, 0, true, SECONDLEVEL);
				}
				break;
			}

		}
	}
	catch (...)
	{
		AddLogEntry(L"### Failed CWardwizSCANNERBase::StartScanSEH", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

bool CISpyScannerBase::StopScan(SCANTYPE eScanType)
{
	bool bReturn = false;
	switch(eScanType)
	{
	case ANTIROOTKITSCAN:
		bReturn = m_objITinAntirootKitScanner.StopAntirootScan();
		break;
	case QUICKSCAN:
	case FULLSCAN:
	case CUSTOMSCAN:
	case USBSCAN:
		if(m_eScanLevel == WARDWIZSCANNER)
		{
			bReturn = m_objWardWizScanner.StopScan();
			break;
		}
		else
		{
			bReturn = m_objClamScanner.StopScan();
			break;
		}
	}
	return bReturn;
}

bool CISpyScannerBase::PauseScan(SCANTYPE eScanType)
{
	bool bReturn = false;
	switch(eScanType)
	{
	case QUICKSCAN:
	case FULLSCAN:
	case CUSTOMSCAN:
	case USBSCAN:
		if(m_eScanLevel == WARDWIZSCANNER)
		{
			bReturn = m_objWardWizScanner.PauseScan();
			break;
		}
		else
		{
			bReturn =  m_objClamScanner.PauseScan();
			break;
		}
	case ANTIROOTKITSCAN:
		bReturn =  m_objITinAntirootKitScanner.PauseAntirootScan();
		break;
	}
	return bReturn;
}

bool CISpyScannerBase::ResumeScan(SCANTYPE eScanType)
{
	bool bReturn = false;
	switch(eScanType)
	{
	case QUICKSCAN:
	case FULLSCAN:
	case CUSTOMSCAN:
	case USBSCAN:
		if(m_eScanLevel == WARDWIZSCANNER)
		{
			bReturn = m_objWardWizScanner.ResumeScan();
			break;
		}
		else
		{
			bReturn = m_objClamScanner.ResumeScan();
			break;
		}
	case ANTIROOTKITSCAN:
		bReturn = m_objITinAntirootKitScanner.ResumeAntirootScan();
		break;
	}
	return bReturn;
}

bool CISpyScannerBase::QuarentineFiles(CStringArray &csQurFilePaths)
{
	return false;
}

bool CISpyScannerBase::GetScanningPaths(LPCTSTR szParam, CStringArray &csaScanPaths)
{
	if(!szParam)
	{
		return false;
	}

	CString str(szParam);
	CString resToken;
	int curPos = 0;

	resToken= str.Tokenize(L"#",curPos);
	while (resToken != _T(""))
	{
		csaScanPaths.Add(resToken);
		resToken = str.Tokenize(L"#", curPos);
	}
	
	if(csaScanPaths.GetCount() == 0)
	{
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	RescanAndRepairFile
*  Description    :	Sends file for scan and repair
*  Author Name    : Ram Shelke
*  Date           : 25 Mar 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CISpyScannerBase::RescanAndRepairFile(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR pszBackUpPath, DWORD dwISpyID)
{
	m_objcsScanNRepair.Lock();

	if (m_eScanLevel == WARDWIZSCANNER)
	{
		if (!m_objWardWizScanner.RescanAndRepairFile(szThreatPath, szThreatName, pszBackUpPath, dwISpyID))
		{
			m_objcsScanNRepair.Unlock();
			return false;
		}
		m_objcsScanNRepair.Unlock();
		return true;
	}
	else
	{
		if (!m_objClamScanner.RescanAndRepairFile(szThreatPath, szThreatName, pszBackUpPath, dwISpyID))
		{
			m_objcsScanNRepair.Unlock();
			return false;
		}
		m_objcsScanNRepair.Unlock();
		return true;
	}

	m_objcsScanNRepair.Unlock();
}


//bool CISpyScannerBase::HandleVirusEntry(LPCTSTR szThreatPath,LPCTSTR szThreatName,LPCTSTR szThreatName1,DWORD dwISpyID,DWORD dwScanType)
//Signature Changed by Vilas on 07 April 2015
//Added more error codes means failing cases
DWORD CISpyScannerBase::HandleVirusEntry(LPCTSTR szThreatPath, LPCTSTR szThreatName, LPCTSTR szThreatName1, DWORD dwISpyID, DWORD dwScanType, CString &csBackupFilePath, DWORD &dwAction)
{
	m_objCriticalSectionHandleVEntry.Lock();

	DWORD	dwRet = 0x00;

	switch (dwScanType)
	{
	case PEFILESCAN:
		dwRet = m_objWardWizScanner.HandleVirusEntry(szThreatPath, szThreatName, szThreatName1, dwISpyID, csBackupFilePath, dwAction);
		break;

	case ROOTKITSCAN:
		dwRet = m_objITinAntirootKitScanner.HandleVirusEntry(szThreatPath, szThreatName, dwISpyID);
	}

	m_objCriticalSectionHandleVEntry.Unlock();
	return dwRet;
}

//bool CISpyScannerBase::RecoverFile(LPTSTR lpFilePath , LPTSTR lpBrowseFilePath, DWORD dwTypeofAction)
//Signature Changed by Vilas on 27 March 2015
//Recover file will handle more error codes means failing cases
DWORD CISpyScannerBase::RecoverFile(LPTSTR lpFilePath, LPTSTR lpBrowseFilePath, DWORD dwTypeofAction)
{
	m_objCriticalSectionRecover.Lock();

	bool bReturn = false;
	//CString csDuplicatePath;

	DWORD	dwRet = 0x00;

	AddLogEntry(L">>> Recover started", 0, 0, true, FIRSTLEVEL);
	if(m_eScanLevel == WARDWIZSCANNER)
	{
	/*
		if(!m_objWardWizScanner.RecoverFile(lpFilePath,lpBrowseFilePath,dwTypeofAction))
		{
			return false;
		}
		AddLogEntry(L">>> Recover finished", 0, 0, true, FIRSTLEVEL);
		bReturn = true;
		return bReturn;
	*/

		dwRet = m_objWardWizScanner.RecoverFile(lpFilePath, lpBrowseFilePath, dwTypeofAction);
	}
	else
	{
	/*
		if(!m_objClamScanner.RecoverFile(lpFilePath,lpBrowseFilePath,dwTypeofAction))
		{
			return false;
		}
		AddLogEntry(L">>> Recover finished", 0, 0, true, FIRSTLEVEL);
		bReturn = true;
		return bReturn;
	*/

		dwRet = m_objClamScanner.RecoverFile(lpFilePath, lpBrowseFilePath, dwTypeofAction);
	}

	m_objCriticalSectionRecover.Unlock();
	return dwRet;
}

bool CISpyScannerBase::SetManipulationPtr(CISpyDBManipulation *pISpyDBManpt)
{
	if(pISpyDBManpt == NULL)
	{
		return false;
	}
	if(m_eScanLevel == WARDWIZSCANNER)
	{
		return m_objWardWizScanner.SetManipulationPtr(pISpyDBManpt);
	}
	else
	{
		return m_objClamScanner.SetManipulationPtr(pISpyDBManpt);
	}
}

bool CISpyScannerBase::FileOperations(LPCTSTR SourcePath, LPCTSTR DestinationPath, DWORD dwType)
{
	if(dwType == 0) //copy operation
	{
		if(!CheckDestinationPathExists(DestinationPath))
		{
			AddLogEntry(L"### Error in CopyFile(CheckDestinationPathExists) CWardwizSCANNERBase::FileOperations,Dest: %s", DestinationPath, 0, true, SECONDLEVEL);
			return false;
		}
		if(!CopyFile(SourcePath, DestinationPath, FALSE))
		{
			Sleep(5);
			if(!CopyFile(SourcePath, DestinationPath, FALSE))
			{
				AddLogEntry(L"### Error in CopyFile CWardwizSCANNERBase::FileOperations, Source: %s, Dest: %s", SourcePath, DestinationPath, true, SECONDLEVEL);
				return false;
			}
		}
	}
	else if(dwType == 1) //move operation
	{
		if(!CheckDestinationPathExists(DestinationPath))
		{
			AddLogEntry(L"### Error in MoveFile(CheckDestinationPathExists) CWardwizSCANNERBase::FileOperations,Dest: %s", DestinationPath, 0, true, SECONDLEVEL);
			return false;
		}
		if(!CopyFile(SourcePath,DestinationPath,false))
		{
			AddLogEntry(L"### Error in MoveFile CWardwizSCANNERBase::FileOperations, Source: %s, Dest: %s", SourcePath, DestinationPath, true, SECONDLEVEL);
			return false;
		}
		SetFileAttributes(SourcePath, FILE_ATTRIBUTE_NORMAL);
		if(!DeleteFile(SourcePath))
		{
			AddLogEntry(L"### Error in MoveFile CWardwizSCANNERBase::FileOperations, Source: %s", SourcePath, 0, true, SECONDLEVEL);
			return false;
		}
	}
	else if(dwType == 2)//delete operation
	{
		SetFileAttributes(SourcePath, FILE_ATTRIBUTE_NORMAL);
		if(!DeleteFile(SourcePath))
		{
			AddLogEntry(L"### Error in DeleteFile CWardwizSCANNERBase::FileOperations, Source: %s", SourcePath, 0, true, SECONDLEVEL);
			return false;
		}
	}
	else if(dwType == 3) //create directory
	{
		if(!PathFileExists(SourcePath))
		{
			if(!CreateDirectory(SourcePath,NULL))
			{
				AddLogEntry(L"### Error in CreateDirectory CWardwizSCANNERBase::FileOperations, Source: %s", SourcePath, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	else if(dwType == 4) //SetFileAttributes
	{
		if(PathFileExists(SourcePath))
		{
			if(SetFileAttributes(SourcePath, FILE_ATTRIBUTE_HIDDEN) == 0)
			{
				AddLogEntry(L"### Error in SetFileAttributes CWardwizSCANNERBase::FileOperations, Source: %s", SourcePath, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	else if (dwType == 5) // Delete Old File Copy new File and Set attribute to hidden //Implemented inRegistration process to distribute UserRegDB files in system
	{		
		if (CheckDestinationPathExists(DestinationPath))
		{
			SetFileAttributes(DestinationPath, FILE_ATTRIBUTE_NORMAL);
			if (!DeleteFile(DestinationPath))
			{
				AddLogEntry(L"### Error in DeleteFile CWardwizSCANNERBase::FileOperations, Source: %s", SourcePath, 0, true, SECONDLEVEL);
			}
		}

		if (!CopyFile(SourcePath, DestinationPath, FALSE))
		{			
				AddLogEntry(L"### Error in CopyFile CWardwizSCANNERBase::FileOperations, Source: %s, Dest: %s", SourcePath, DestinationPath, true, SECONDLEVEL);
				return false;
		}

		if (PathFileExists(DestinationPath))
		{
			if (SetFileAttributes(DestinationPath, FILE_ATTRIBUTE_HIDDEN) == 0)
			{
				AddLogEntry(L"### Error in SetFileAttributes CWardwizSCANNERBase::FileOperations, Source: %s", SourcePath, 0, true, SECONDLEVEL);
			}
		}
	}
	return true;
}


bool CISpyScannerBase::CheckDestinationPathExists(LPCTSTR DestinationPath)
{
	CString csDestPath(DestinationPath);
	int iPos = csDestPath.ReverseFind(L'\\');
	CString csDestFolderPath = csDestPath.Left(iPos);
	if(!PathFileExists(csDestFolderPath))
	{
		if(!CreateDirectory(csDestFolderPath,NULL))
		{
			AddLogEntry(L"### Error in CWardwizSCANNERBase::CheckDestinationPathExists,DestinationPath: %s", csDestFolderPath, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

/***********************************************************************************************
Function Name  : LoadSignatureDatabase
Description    : Load WardWiz Signature Datbase
SR.NO		   :
Author Name    : Ram Shelke
Date           : 27 April 2015
***********************************************************************************************/
DWORD CISpyScannerBase::LoadSignatureDatabase(DWORD &dwSigCount)
{
	DWORD dwReturn = 0x00;
	__try
	{
		if (m_eScanLevel == WARDWIZSCANNER)
		{
			//Use here the object of wardwiz scanner.
			if (m_objWardWizScanner.LoadSignatureDatabase(dwSigCount) != 0x00)
			{
				dwReturn = 0x01;
				AddLogEntry(L"### Failed to LoadSignatureDatabase in CWardwizSCANNERBase::LoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
				goto FAILED;
			}
		}
		else
		{
			//Use here the object of Clam scanner.
			if (m_objClamScanner.LoadSignatureDatabase(dwSigCount) != 0x00)
			{
				dwReturn = 0x02;
				AddLogEntry(L"### Failed to LoadSignatureDatabase in CWardwizSCANNERBase::LoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
				goto FAILED;
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwReturn = 0x03;
		AddLogEntry(L"### Exception in LoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
	}
FAILED:
	return dwReturn;
}

/***********************************************************************************************
Function Name  : LoadSignatureDatabase
Description    : Load WardWiz Signature Datbase
SR.NO		   :
Author Name    : Ram Shelke
Date           : 27 April 2015
***********************************************************************************************/
DWORD CISpyScannerBase::UnLoadSignatureDatabase()
{
	DWORD dwReturn = 0x00;
	__try
	{
		if (m_eScanLevel == WARDWIZSCANNER)
		{
			//Use here the object of wardwiz scanner.
			if (m_objWardWizScanner.UnLoadSignatureDatabase() != 0x00)
			{
				dwReturn = 0x01;
				AddLogEntry(L"### Failed to UnLoadSignatureDatabase in CWardwizSCANNERBase::LoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
				goto FAILED;
			}
		}
		else
		{
			//Use here the object of Clam scanner.
			if (m_objClamScanner.UnLoadSignatureDatabase() != 0x00)
			{
				dwReturn = 0x02;
				AddLogEntry(L"### Failed to UnLoadSignatureDatabase in CWardwizSCANNERBase::LoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
				goto FAILED;
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwReturn = 0x03;
		AddLogEntry(L"### Exception in UnLoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
	}
FAILED:
	return dwReturn;
}

/***********************************************************************************************
Function Name  : ReLoadSignatureDatabase
Description    : Function to reload signature database
SR.NO		   :
Author Name    : Ram Shelke
Date           : 27 April 2015
***********************************************************************************************/
DWORD CISpyScannerBase::ReLoadSignatureDatabase(DWORD &dwSigCount)
{
	DWORD dwReturn = 0x00;
	__try
	{
		m_objcsScanUpdateSync.Lock();

		if (UnLoadSignatureDatabase() != 0x00)
		{
			dwReturn = 0x01;
			AddLogEntry(L"### Failed to UnLoadSignatureDatabase in CWardwizSCANNERBase::ReLoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
			goto FAILED;
		}

		if (LoadSignatureDatabase(dwSigCount) != 0x00)
		{
			dwReturn = 0x02;
			AddLogEntry(L"### Failed to LoadSignatureDatabase in CWardwizSCANNERBase::ReLoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
			goto FAILED;
		}
		m_objcsScanUpdateSync.Unlock();
		AddLogEntry(L">>> Reload Database sucess in CWardwizSCANNERBase::ReLoadSignatureDatabase", 0, 0, true, ZEROLEVEL);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwReturn = 0x03;
		AddLogEntry(L"### Exception in ReLoadSignatureDatabase", 0, 0, true, SECONDLEVEL);
	}
FAILED:
	m_objcsScanUpdateSync.Unlock();
	return dwReturn;
}

/***************************************************************************
Function Name  : ScanFile
Description    : Function to scan single file using single loaded Database from service and return as per result of scan
Author Name    : Ram Shelke
SR_NO		   : 
Date           : 25 Mar 2016
****************************************************************************/
DWORD CISpyScannerBase::ScanFile(LPTSTR lpszFilePath, LPTSTR szVirusName, DWORD &dwISpyID, DWORD dwScanType, DWORD &dwAction, DWORD &dwSignatureFailedToLoad, bool bRescan)
{
	DWORD dwReturn = 0x00;
	try
	{
		m_objcsScanUpdateSync.Lock();
		dwReturn = m_objWardWizScanner.ScanFile(lpszFilePath, szVirusName, dwISpyID, dwScanType, dwAction, dwSignatureFailedToLoad, bRescan);
		m_objcsScanUpdateSync.Unlock();
	}
	catch (...)
	{
		m_objcsScanUpdateSync.Unlock();
		AddLogEntry(L"### Exception in CWardwizSCANNERBase::ScanFile, File: %s", lpszFilePath, 0, true, SECONDLEVEL);
	}
	return dwReturn;
}

/***************************************************************************
Function Name  : EnumFolder
Description    : Function Enumerate files from folder
Author Name    : Ram Shelke
SR_NO		   : 
Date           : 25 Mar 2016
****************************************************************************/
void CISpyScannerBase::EnumFolder(LPCTSTR lpszFolderPath)
{
	OutputDebugString(lpszFolderPath);
	try
	{
		if (m_bStopScan)
		{
			return;
		}

		CFileFind finder;

		CString strWildcard(lpszFolderPath);

		if (strWildcard.IsEmpty())
			return;

		if (strWildcard[strWildcard.GetLength() - 1] != '\\')
			strWildcard += _T("\\*.*");
		else
			strWildcard += _T("*.*");

		if (!PathFileExists(lpszFolderPath))
		{
			return;
		}

		BOOL bWorking = finder.FindFile(strWildcard);

		while (bWorking)
		{
			if (m_bStopScan)
				break;

			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			TCHAR	szPath[1024] = { 0 };

			wsprintf(szPath, L"%s", finder.GetFilePath());

			if (finder.IsDirectory())
				EnumFolder(szPath);
			else
			{

				DWORD dwSpyID = 0x00;
				DWORD dwAction = 0x00;
				TCHAR szVirusName[MAX_PATH] = { 0 };
				DWORD dwSignatureFailedToLoad = 0x00;
				if (ScanFile(szPath, szVirusName, dwSpyID, INDEXING, dwAction, dwSignatureFailedToLoad, false))
				{
					//do not add
				}

				//Save the file once indexing done for 5000 files.
				if (m_dwFilesIndexed > 0 && (m_dwFilesIndexed % 5000) == 0)
				{
					SaveLocalDBFiles();
				}

				m_dwFilesIndexed++;
			}

			//Silent scanning
			Sleep(500);
		}
		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSCANNERBase::EnumFolder, Path: %s", lpszFolderPath, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : ScanFile
Description    : Function to save maintained local DB cache from Memory to file.
Author Name    : Ram Shelke
SR_NO		   :
Date           : 25 Mar 2016
****************************************************************************/
bool CISpyScannerBase::SaveLocalDBFiles()
{
	bool bReturn = false;
	try
	{
		CWWizIndexer objIndexer;
		if (objIndexer.SaveLocalDatabase())
		{
			bReturn = true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in SaveLocalDBFiles", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}