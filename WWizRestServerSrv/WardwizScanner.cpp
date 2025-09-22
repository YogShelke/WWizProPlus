#include "stdafx.h"
#include "WardwizScanner.h"
#include <Psapi.h>
#include "WrdwizEncDecManager.h"
#include "ISpyCommServer.h"

BOOL					g_bIsScanning;

CWardwizScanner::CWardwizScanner(void) :
m_objCom(SERVICE_SERVER, true, 3)
, m_objScanCom(SERVICE_SERVER, true, 3)
{
	m_hPsApiDLL = NULL;
	m_bThreatDetected = false;
	LoadPSAPILibrary();
}
CWardwizScanner::~CWardwizScanner(void)
{
	if (m_hPsApiDLL != NULL)
	{
		FreeLibrary(m_hPsApiDLL);
		m_hPsApiDLL = NULL;
	}
}


/***************************************************************************
Function Name  : LoadPSAPILibrary
Description    : Load PSAPI.DLL.
For Issue: In WinXP 64 bit if we uncheck the tool tip, checkbox doesn't get uncheck and UI hangs.
Author Name    : Ramkrushna Shelke
S.R. No        :
Date           : 04th Feb 2015
****************************************************************************/
void CWardwizScanner::LoadPSAPILibrary()
{
	__try
	{
		TCHAR	szSystem32[256] = { 0 };
		TCHAR	szTemp[256] = { 0 };
		GetSystemDirectory(szSystem32, 255);

		ZeroMemory(szTemp, sizeof(szTemp));
		wsprintf(szTemp, L"%s\\PSAPI.DLL", szSystem32);
		if (!m_hPsApiDLL)
		{
			m_hPsApiDLL = LoadLibrary(szTemp);
		}

		if (!EnumProcessModulesWWizEx)
		{
			EnumProcessModulesWWizEx = (ENUMPROCESSMODULESEX)GetProcAddress(m_hPsApiDLL, "EnumProcessModulesEx");
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CWardWizScan::LoadPSAPILibrary", 0, 0, true, SECONDLEVEL);
	}
}

bool CWardwizScanner::StartWardwizScanner(std::string strFileToScan)
{
		m_iTotalFileCount = 0;
		m_dwTotalFileCount = 0;
		m_iMemScanTotalFileCount = 0;
		m_dwVirusFoundCount = 0;
	
		try
		{
			m_eScanType = CUSTOMSCAN;
			CString csFilePath(strFileToScan.c_str());
	
			if (csFilePath.Trim().GetLength() > 0)
			{
				AddLogEntry(L"ScannerStartWardwizScanner received this file path : %s ", csFilePath, 0, true, SECONDLEVEL);

				if (PathFileExists(csFilePath))
				{
					ScanForSingleFile(csFilePath);
				}
			}	
	
			return TRUE;
		}
		catch (...)
		{
			AddLogEntry(L"### Exception in CWardwizScanner::StartWardwizScanner", 0, 0, true, SECONDLEVEL);
			return FALSE;
		}
		return TRUE;
}

/**********************************************************************************************************
*  Function Name  :	CheckFileIsInRepairRebootIni
*  Description    :	Get Quarantine folder path.
*  Author Name    : Vilas Suvarnakar
*  Date           : 24 March 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CWardwizScanner::CheckFileIsInRepairRebootIni(CString csFilePath)
{
	bool	bReturn = false;
	try
	{
		TCHAR	szRepairIniPath[MAX_PATH] = { 0 };

		if (GetQuarantineFolderPath(szRepairIniPath))
		{
			//AddLogEntry(L"### Failed in CWardWizScan::CheckFileIsInRepairRebootIni::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
			return bReturn;
		}

		wcscat_s(szRepairIniPath, _countof(szRepairIniPath), L"\\WWRepair.ini");

		DWORD dwRepairCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szRepairIniPath);

		if (!dwRepairCount)
			return bReturn;

		DWORD	i = 0x01;

		TCHAR	szValueName[256] = { 0 };
		TCHAR	szValueData[2048] = { 0 };
		TCHAR	szFilePath[512] = { 0 };

		swprintf_s(szFilePath, _countof(szFilePath), L"|%s|", csFilePath);
		_wcsupr(szFilePath);

		for (; i <= dwRepairCount; i++)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
			GetPrivateProfileString(L"Files", szValueName, L"", szValueData, _countof(szValueData), szRepairIniPath);
			_wcsupr(szValueData);
			if (wcsstr(szValueData, szFilePath) != NULL)
			{
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CWardWizScan::CheckFileIsInRepairRebootIni", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/***********************************************************************************************
Function Name  : Checking file entry present in Recover ini (WWRecover.ini)
Description    : Parses WWRecover.ini and not sends to scan if file is found.
SR.NO		   :
Author Name    : Vilas Suvarnakar
Date           : 27 March 2015
***********************************************************************************************/
bool CWardwizScanner::CheckFileIsInRecoverIni(CString csFilePath)
{
	bool	bReturn = false;
	try
	{
		TCHAR szRecoverIniPath[MAX_PATH] = { 0 };

		if (GetQuarantineFolderPath(szRecoverIniPath))
		{
			//AddLogEntry(L"### Failed in CheckFileIsInRecoverIni::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
			return bReturn;
		}

		wcscat_s(szRecoverIniPath, _countof(szRecoverIniPath), L"\\");
		wcscat_s(szRecoverIniPath, _countof(szRecoverIniPath), WRDWIZRECOVERINI);

		DWORD dwRecoverCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szRecoverIniPath);

		if (!dwRecoverCount)
			return bReturn;

		DWORD	i = 0x01;
		TCHAR	szValueName[256] = { 0 };
		TCHAR	szValueData[512] = { 0 };

		for (; i <= dwRecoverCount; i++)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			ZeroMemory(szValueData, sizeof(szValueData));

			swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
			GetPrivateProfileString(L"Files", szValueName, L"", szValueData, _countof(szValueData), szRecoverIniPath);

			if (csFilePath.CompareNoCase(szValueData) == 0x00)
			{
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CWardWizScan::CheckFileIsInRecoverIni", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/**********************************************************************************************************
*  Function Name  :	CheckFileIsInDeleteIni
*  Description    :	Check whether file available in delete.ini
*  Author Name    : 
*  Date           : 
*  SR_NO		  :
**********************************************************************************************************/
bool CWardwizScanner::CheckFileIsInDeleteIni(CString csQurFilePaths)
{
	bool  bReturn = false;
	TCHAR szValueName[260] = { 0 };
	DWORD dwCount = 0x00;
	TCHAR szDuplicateString[512] = { 0 };
	TCHAR szTempKey[512] = { 0 };
	TCHAR szApplnName[512] = { 0 };
	TCHAR szConcatnateDeleteString[1024] = { 0 };
	TCHAR szFileName[512] = { 0 };
	TCHAR szQuarantineFileName[512] = { 0 };
	TCHAR szVirusName[512] = { 0 };

	try
	{
		CString csDeleteFailedINIPath(L"");
		CString csQuarantineFolderPath = GetQuarantineFolderPath();

		csDeleteFailedINIPath.Format(L"%s\\WRDWIZDELETEFAIL.INI", csQuarantineFolderPath);

		DWORD dwDeleteCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, csDeleteFailedINIPath);

		if (!dwDeleteCount)
			return bReturn;

		for (int i = 0x01; i <= static_cast<int>(dwDeleteCount); i++)
		{
			ZeroMemory(szTempKey, sizeof(szTempKey));
			swprintf_s(szTempKey, _countof(szTempKey), L"%lu", i);

			GetPrivateProfileString(L"DeleteFiles", szTempKey, L"", szDuplicateString, 511, csDeleteFailedINIPath);
			ZeroMemory(szApplnName, sizeof(szApplnName));
			if (!TokenizationOfParameterForrecover(szDuplicateString, szFileName, _countof(szFileName), szQuarantineFileName, _countof(szQuarantineFileName), szVirusName, _countof(szVirusName)))
			{
				//AddLogEntry(L"### CheckFileIsInDeleteIni::TokenizationOfParameterForrecover is not tokenize properly", 0, 0, true, FIRSTLEVEL);
				return false;
			}

			if (!TokenizeIniData(szFileName, szApplnName, _countof(szApplnName)))
			{
				//AddLogEntry(L"### CWardWizScan::CheckFileIsInDeleteIni::TokenizeIniData is not tokenize properly", 0, 0, true, FIRSTLEVEL);
				return false;
			}

			CString csDuplicateString = (CString)szDuplicateString;
			CString csFileName = (CString)szApplnName;
			if (csFileName.CompareNoCase(csQurFilePaths) == 0)
			{
				bReturn = true;
				break;
			}
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizScan::CheckFileIsInDeleteIni", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/**********************************************************************************************************
*  Function Name  :	GetQuarantineFolderPath
*  Description    :	Get Quarantine folder path.
*  Author Name    : Vilas Suvarnakar
*  Date           : 24 March 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CWardwizScanner::GetQuarantineFolderPath(LPTSTR lpszQuarantineFolPath)
{
	bool	bReturn = true;
	try
	{
		TCHAR	szModulePath[MAX_PATH] = { 0 };

		GetModulePath(szModulePath, MAX_PATH);
		if (!wcslen(szModulePath))
			return bReturn;

		wcscpy_s(lpszQuarantineFolPath, MAX_PATH - 1, szModulePath);
		wcscat_s(lpszQuarantineFolPath, MAX_PATH - 1, L"\\Quarantine");
		bReturn = false;
	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CWardWizScan::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}
/**********************************************************************************************************
*  Function Name  :	GetQuarantineFolderPath
*  Description    :	Get Quarantine folder path.
*  Author Name    : Neha Gharge
*  Date           : 4 Dec 2014
*  SR_NO		  :
**********************************************************************************************************/
CString CWardwizScanner::GetQuarantineFolderPath()
{
	try
	{
		TCHAR szModulePath[MAX_PATH] = { 0 };
		if (!GetModulePath(szModulePath, MAX_PATH))
		{
			return L"";
		}
		CString csQuarantineFolderPath = szModulePath;
		return csQuarantineFolderPath += L"\\Quarantine";
	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CWardWizScan::GetQuarantineFolderPath", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}


/***********************************************************************************************
Function Name  : TokenizationOfParameterForrecover
Description    : Tokenize input path to get file name, quarantine name and virus name
SR.NO		   :
Author Name    : Neha Gharge
Date           : 8 April 2015
***********************************************************************************************/
bool CWardwizScanner::TokenizationOfParameterForrecover(LPTSTR lpWholePath, TCHAR* szFileName, DWORD dwsizeofFileName, TCHAR* szQuarantinepath, DWORD dwsizeofquarantinefileName, TCHAR* szVirusName, DWORD dwsizeofVirusName)
{
	TCHAR	szToken[] = L"|";
	TCHAR	*pToken = NULL;
	try
	{
		if (lpWholePath == NULL || szFileName == NULL || szQuarantinepath == NULL || szVirusName == NULL)
			return false;

		pToken = wcstok(lpWholePath, szToken);
		if (!pToken)
		{
			//AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		if (pToken)
		{
			wcscpy_s(szFileName, (dwsizeofFileName - 1), pToken);
		}

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			//AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
			return false;
		}
		if (pToken)
			wcscpy_s(szQuarantinepath, (dwsizeofquarantinefileName - 1), pToken);

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			//AddLogEntry(L"### No string to tokenize from ini", 0, 0, true, FIRSTLEVEL);
			return false;
		}
		if (pToken)
			wcscpy_s(szVirusName, (dwsizeofVirusName - 1), pToken);
	}
	catch (...)
	{
		//AddLogEntry(L"### Exception in CWardWizScan::TokenizationOfParameterForrecover", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	TokenizeIniData
*  Description    :	Tokenization of entries from delete file ini
*  Author Name    : Neha Gharge
*  Date           : 26 Feb 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CWardwizScanner::TokenizeIniData(LPTSTR lpszValuedata, TCHAR* szApplicationName, DWORD dwSizeofApplicationName)
{
	TCHAR	szToken[] = L",";
	TCHAR	*pToken = NULL;
	try
	{
		if (lpszValuedata == NULL || szApplicationName == NULL)
			return false;

		pToken = wcstok(lpszValuedata, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize from ini in CWardwizScanner::TokenizeIniData", 0, 0, true, FIRSTLEVEL);
			return false;
		}

		TCHAR	szValueApplicationName[512] = { 0 };

		if (pToken)
		{
			wcscpy_s(szValueApplicationName, _countof(szValueApplicationName), pToken);
			wcscpy_s(szApplicationName, (dwSizeofApplicationName - 1), szValueApplicationName);
		}

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			AddLogEntry(L"### No string to tokenize from ini in CWardwizScanner::TokenizeIniData", 0, 0, true, FIRSTLEVEL);
		}

		TCHAR	szAttemptCnt[16] = { 0 };

		if (pToken)
			wcscpy_s(szAttemptCnt, _countof(szAttemptCnt), pToken);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizScanner::TokenizeIniData", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}


/**********************************************************************************************************
*  Function Name  :	ScanForSingleFile
*  Description    :	Scan each single file .
*  Author Name    : Neha Gharge
*  Date           : 4 Dec 2014
*  SR_NO		  :
**********************************************************************************************************/
void CWardwizScanner::ScanForSingleFile(CString csFilePath)
{
	AddLogEntry(L" Entered CWardwizScanner::ScanForSingleFile with file %s ", csFilePath, 0, true, SECONDLEVEL);
	if (csFilePath.GetLength() == 0)
		return;

	try
	{
		bool bSetStatus = false;
		bool bVirusFound = false;

		CString csVirusName(L"");
		CString csVirusPath(L"");
		DWORD dwISpywareID = 0;
		DWORD dwAction = 0x00;
		CString csCurrentFile(L"");
		CString csStatus(L"");

		if (csFilePath.Trim().GetLength() > 0)
		{
			if (m_csPreviousPath.Compare(csFilePath)!=0)
			{
				if (PathFileExists(csFilePath))
				{
					m_bIsPathExist = true;
					DWORD dwISpyID = 0;
					TCHAR szVirusName[MAX_PATH] = { 0 };
					DWORD dwSignatureFailedToLoad = 0;
					DWORD dwActionTaken = 0x00;

					if ((CheckFileIsInRepairRebootIni(csFilePath)) ||
						(CheckFileIsInRecoverIni(csFilePath)) || (CheckFileIsInDeleteIni(csFilePath))
						)
						return;

				CString csActionID;
				AddLogEntry(L" CWardwizScanner::ScanForSingleFile before InvokeStartScanFromCommService with file %s ", csFilePath, 0, true, SECONDLEVEL);
				if (InvokeStartScanFromCommService(csFilePath, szVirusName, dwISpyID, dwSignatureFailedToLoad, dwActionTaken, m_bRescan))
				{
						if (dwISpyID >= 0)
						{
							csVirusName = szVirusName;
							m_csVirusName = szVirusName;
							dwISpywareID = dwISpyID;
							m_bThreatDetected = true;
							m_dwVirusFoundCount++;
							AddLogEntry(L"In CWardwizScanner::ScanForSingleFile Virus detected %s ", m_csVirusName, 0, true, SECONDLEVEL);
						}
					}
					m_FileScanned++;
				}
			}
			m_csPreviousPath = csFilePath;
		}

		//virus found 
		if (bVirusFound)
		{
			bSetStatus = true;
			return;
		}
		else
		{
			bSetStatus = true;
			csStatus = csFilePath;
		}
		return;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in void CWardwizScanner::ScanForSingleFile", 0, 0, true, SECONDLEVEL);
		return;
	}
}

bool CWardwizScanner::InvokeStartScanFromCommService(LPCTSTR szFilePath, LPTSTR szVirusName, DWORD &dwISpyID, DWORD &dwFailedToLoadSignature, DWORD &dwActionTaken, bool bRescan)
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
			AddLogEntry(L"### Failed to send data in CWardwizScanner::InvokeStartScanFromCommService", 0, 0, true, SECONDLEVEL);
			bSendFailed = true;
		}

		if (!m_objScanCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to ReadData in CWardwizScanner::InvokeStartScanFromCommService", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CWardwizScanner::InvokeStartScanFromCommService", 0, 0, true, SECONDLEVEL);
	}
	return false;
}


