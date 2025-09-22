/**********************************************************************************************************
Program Name          : WardWizUpdateManager.cpp
Description           : Update manager class.
Author Name			  : Amol Jaware
Date Of Creation      : 20 Jan 2019
Version No            : 4.1.0.1
***********************************************************************************************************/

#include "stdafx.h"
#include "WardWizUpdateManager.h"
#include "WardWizUpdate.h"
#include "WardWizParseIni.h"
#include "iSpyMemMapServer.h"

#define MAX_UPDATE_RETRYCOUNT	10

CISpyCriticalSection			g_objcriticalSection;
std::vector<ALUZIPFILEINFO>		vALUZipFileInfo;
std::vector<ALUFILEINFO>		vALUFileInfo;
std::vector<CString>			vReplacedFiles;

const	TCHAR	szUpdateFolderName[MAX_PATH] = L"vb20";
const	TCHAR	szwardwizEPSPublish[MAX_PATH] = L"WardWizDev\\WardWizEPS\\WardWizUpdates";
const	TCHAR	szwardwizEPSProgData[MAX_PATH] = L"WardWizEPS";
iSpyServerMemMap_Server		g_objALupdateServ(ALUPDATE);

CWardWizUpdateManager	*g_pThis = NULL;
CITinRegWrapper			g_objReg;
CString					g_csProdRegKey = L"";
CString					g_csServerName = L"www.vibranium.co.in";
DWORD					g_dwDaysLeft = 0x00;
DWORD					g_dwProductID = 0x00;
BOOL					g_bIsWow64;
int						g_iPreviousPerc = 0x00;

/***************************************************************************************************
*  Function Name  : CWardWizUpdateManager
*  Description    : CTOR CWardWizUpdateManager
*  Author Name    : Amol Jaware
*  Date			  : 16 Jan 2019
****************************************************************************************************/
CWardWizUpdateManager::CWardWizUpdateManager():
m_objCom(UI_SERVER, true, 0x03)
, hTotalFileSizeThread(NULL)
, m_pfnDownloadStatusCallback(NULL)
, m_pDownloadController(NULL)
, m_pDownloadController4UpdtMgr(NULL)
{
	g_pThis = this;
}

/***************************************************************************************************
*  Function Name  : CWardWizUpdateManager
*  Description    : CTOR CWardWizUpdateManager
*  Author Name    : Amol Jaware
*  Date			  : 16 Jan 2019
****************************************************************************************************/
CWardWizUpdateManager::CWardWizUpdateManager(DOWNLOADFUNCTIONCALLBACK pfnDownloadStatusCallback):
m_objCom(UI_SERVER, true, 0x03)
, hTotalFileSizeThread(NULL)
, m_pfnDownloadStatusCallback(NULL)
, m_pDownloadController(NULL)
, m_pDownloadController4UpdtMgr(NULL)
{
	g_pThis = this;
	m_pfnDownloadStatusCallback = pfnDownloadStatusCallback;
}

/***************************************************************************************************
*  Function Name  : CWardWizUpdateManager
*  Description    : DTOR CWardWizUpdateManager
*  Author Name    : Amol Jaware
*  Date			  : 16 Jan 2019
****************************************************************************************************/
CWardWizUpdateManager::~CWardWizUpdateManager()
{
	if (m_pDownloadController != NULL)
	{
		m_pDownloadController = NULL;
	}
	if (m_pDownloadController4UpdtMgr != NULL)
	{
		m_pDownloadController4UpdtMgr = NULL;
	}
}

/***************************************************************************************************
*  Function Name  : InitializeGlobalVariblesToZero()
*  Description    : Initialize required variable to its initialized value
*  Author Name    : Vilas , Neha
*  SR_NO		  : WRDWIZALUSRV_0028
*  Date			  :	4- Jul -2014 (Auto Live Update)
*  Modified Date  :	11-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
void CWardWizUpdateManager::InitializeGlobalVariblesToZero()
{
	g_bIsWow64 = FALSE;
	m_bVistaOnward = false;
	//theApp.m_bRequestFromUI = false;
	m_bRestartFlag = false;
	m_bExtractionFailed = false;
	m_bUpToDate = false;
	m_bUpdateSuccess = false;
	
	m_dwUnzippedCount = 0x00;
	m_dwReplacedCount = 0x00;

	ZeroMemory(theApp.m_szAVPath, sizeof(theApp.m_szAVPath));
	ZeroMemory(m_szAllUserPath, sizeof(m_szAllUserPath));
	m_hHashDLL = NULL;
	m_hExtractDLL = NULL;
	m_hFile = NULL;
	m_hIniFile = NULL;
	m_hTargetFile = NULL;

	if (hTotalFileSizeThread)
		hTotalFileSizeThread = NULL;

	GetEnvironmentVariable(L"ALLUSERSPROFILE", m_szAllUserPath, 255);

	_tcscpy(m_szAppDataFolder, m_szAllUserPath);

	ClearMemoryMapObjects();

	vALUFileInfo.clear();
	vReplacedFiles.clear();
	AddServiceNameIntoVector();
	//Issue : Database version / Product version should not change when product ask for restart, it should be update after restart
	//Resolved By :  Nitin K.s
	m_bIsALUDeleted = FALSE;
	m_dwTotalFileSize = 0x00;
	m_dwPercentage = 0x00;
	g_iPreviousPerc = 0x00;
	m_iCurrentDownloadedByte = 0x00;
	m_lRetryCount = 0x00;
	m_lAppStopping = 0x00;
	m_lTotalDownloadedBytes = 0x00;
	m_bIsRelayClient = true;

	CWardWizOSversion		objOSVersionWrap;
	m_OSType = objOSVersionWrap.DetectClientOSVersion();

	if (m_pDownloadController != NULL)
	{
		delete m_pDownloadController;
		m_pDownloadController = NULL;
	}

	m_pDownloadController = new CDownloadController();
	m_pDownloadController->ResetInitData();
	m_pDownloadController->SetGUIInterface(&m_objGUI);
	m_objWinHttpManager.GetProxyServerDetailsFromDB();
}

/***************************************************************************************************
*  Function Name  : AddServiceNameIntoVector()
*  Description    : Add all services into vector.
*  Author Name    : Vilas , Neha
**  SR_NO		  : WRDWIZALUSRV_0071
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::AddServiceNameIntoVector()
{
	__try
	{
		m_vServiceNameVector.clear();

		for (int j = 0; g_csServiceNameArray[j] != L"\0"; j++)
		{
			m_vServiceNameVector.push_back(g_csServiceNameArray[j]);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::AddServiceNameIntoVector", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : ClearMemoryMapObjects()
*  Description    : Clear memory object
*  Author Name    : Vilas, neha
*  SR_NO		  : WRDWIZALUSRV_0064
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
void CWardWizUpdateManager::ClearMemoryMapObjects()
{
	ITIN_MEMMAP_DATA iTinMemMap = { 0 };
	g_objALupdateServ.UpdateServerMemoryMappedFile(&iTinMemMap, sizeof(iTinMemMap));
}

/***************************************************************************************************
*  Function Name  : InitializeGlobalVaribles()
*  Description    : It will check client machine is 64 bit and take path of client machine
product installed path
*  Author Name    : Vilas , Neha
*  SR_NO		  : WRDWIZALUSRV_0029
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
****************************************************************************************************/
void CWardWizUpdateManager::InitializeGlobalVaribles()
{
	try
	{
		IsWow64();

		GetAVPathFromRegistry(theApp.m_szAVPath);
		if (!PathFileExists(theApp.m_szAVPath))
		{
			GetModuleFileName(NULL, theApp.m_szAVPath, 511);
			TCHAR	*pTemp = wcsrchr(theApp.m_szAVPath, '\\');
			if (pTemp)
			{
				pTemp++;
				*pTemp = '\0';
			}
		}
		m_csAppFolderName = theApp.m_szAVPath;
		m_csAppFolderName = m_csAppFolderName.Left(m_csAppFolderName.GetLength() - 1);
		m_csAppFolderName = m_csAppFolderName.Right(m_csAppFolderName.GetLength() - (m_csAppFolderName.ReverseFind(L'\\') + 1));

		theApp.LoadDLLs();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception inside CVibraniumUpdateManager::InitializeGlobalVaribles", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : InitGlobalVariToZero4UpdtMgr()
*  Description    : Initialize required variable to its initialized value
*  Author Name    : Amol Jaware
*  SR_NO		  :
*  Date			  :	10-April-2018
****************************************************************************************************/
void CWardWizUpdateManager::InitGlobalVariToZero4UpdtMgr()
{
	g_bIsWow64 = FALSE;
	m_bVistaOnward = false;

	m_bExtractionFailedUpdtMgr = false;

	m_dwReplacedCount = 0x00;

	ZeroMemory(theApp.m_szAVPath, sizeof(theApp.m_szAVPath));
	ZeroMemory(m_szAllUserPath, sizeof(m_szAllUserPath));
	m_hHashDLL = NULL;
	m_hExtractDLL = NULL;
	m_hFile = NULL;
	m_hIniFile = NULL;
	m_hTargetFile = NULL;

	GetEnvironmentVariable(L"ALLUSERSPROFILE", m_szAllUserPath, 255);

	_tcscpy(m_szAppDataFolder, m_szAllUserPath);

	InitializeGlobalVaribles();

	if (hTotalFileSizeThread)
		hTotalFileSizeThread = NULL;

	vALUZipFileInfo.clear();
	vReplacedFiles.clear();
	AddServiceNameIntoVector();

	m_iCurrentDownloadedByte = 0x00;
	m_lRetryCount = 0x00;
	m_lAppStopping = 0x00;
	m_lTotalDownloadedBytes = 0x00;

	if (m_pDownloadController4UpdtMgr != NULL)
	{
		delete m_pDownloadController4UpdtMgr;
		m_pDownloadController4UpdtMgr = NULL;
	}

	m_pDownloadController4UpdtMgr = new CDownloadController();
	m_pDownloadController4UpdtMgr->ResetInitData();

	IsWow64();

	GetAVPathFromRegistry(theApp.m_szAVPath);
	if (!PathFileExists(theApp.m_szAVPath))
	{
		GetModuleFileName(NULL, theApp.m_szAVPath, 511);
		TCHAR	*pTemp = wcsrchr(theApp.m_szAVPath, '\\');
		if (pTemp)
		{
			pTemp++;
			*pTemp = '\0';
		}
	}

	theApp.LoadDLLs();
}

/***************************************************************************************************
*  Function Name  : StartUpdate()
*  Description    : Start live update
*  Author Name    : Vilas , Neha
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
****************************************************************************************************/
void CWardWizUpdateManager::StartUpdate()
{
	try
	{
		ReadAutoliveUpdateEnableCheck();
		InitializeGlobalVariblesToZero();
		InitializeGlobalVaribles();
		InitGlobalVariToZero4UpdtMgr();

		g_csProdRegKey = CWWizSettingsWrapper::GetProductRegistryKey();

		DWORD	dwProductID = 0x00;

		GetProductID(dwProductID);
		if (dwProductID == ELITE)
		{
			//Read here registry entry for is relay client.
			DWORD dwISRelayClient = 0x00;
			if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"dwISRelayClient", dwISRelayClient) != 0x00)
			{
				AddLogEntry(L"### Failed to get Registry Entry for dwISRelayClient in CVibraniumUpdateManager, KeyPath: %s", g_csProdRegKey, 0, true, FIRSTLEVEL);
			}

			if (dwISRelayClient != 0x01)
			{
				m_bIsRelayClient = false;
			}
		}

		CWardWizOSversion		objOSVersionWrap;
		m_OSType = objOSVersionWrap.DetectClientOSVersion();
		

		ZeroMemory(m_szAllUserPath, sizeof(m_szAllUserPath));
		ZeroMemory(theApp.m_szAVPath, sizeof(theApp.m_szAVPath));

		GetEnvironmentVariable(L"ALLUSERSPROFILE", m_szAllUserPath, 255);

		GetAVPathFromRegistry(theApp.m_szAVPath);
		if (!PathFileExists(theApp.m_szAVPath))
		{
			GetModuleFileName(NULL, theApp.m_szAVPath, 511);
			TCHAR	*pTemp = wcsrchr(theApp.m_szAVPath, '\\');
			if (pTemp)
			{
				pTemp++;
				*pTemp = '\0';
			}
		}

		m_csAppFolderName = theApp.m_szAVPath;
		m_csAppFolderName = m_csAppFolderName.Left(m_csAppFolderName.GetLength() - 1);
		m_csAppFolderName = m_csAppFolderName.Right(m_csAppFolderName.GetLength() - (m_csAppFolderName.ReverseFind(L'\\') + 1));

		PerformPostUpdateRegOperations();
		if (!SetAllALtFilesAttributeToNorml())
		{
			AddLogEntry(L"### Failed to SetAllALtFilesAttributeToNorml", 0, 0, true, FIRSTLEVEL);
		}
		
		AddServiceNameIntoVector();

		//Issue : Database version / Product version should not change when product ask for restart, it should be update after restart
		//Resolved By :  Nitin K.
		m_bIsALUDeleted = FALSE;
		
		DeleteAllRenamedFiles();

		DeleteAllALtFilesFromProgramData();
		

		StartALUpdateProcess();
		
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::StartUpdate", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : StartUpdate4UpdateManger()
*  Description    : Initializing all variables and create a thread which start downloding and replacing file.
*  Author Name    : Amol Jaware.
*  SR_NO		  :
*  Date			  :	12-April-2018
****************************************************************************************************/
void CWardWizUpdateManager::StartUpdate4UpdateManger()
{
	__try
	{
		//InitGlobalVariToZero4UpdtMgr();
		StartALUpdtProcess4UdtMgr();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::StartUpdate4UpdateManger", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : MakeDownloadUrlList4EPSUpdtMgr()
*  Description    : Make url of that files which new/ modified at server side .
*  Author Name    : Amol Jaware
*  SR_NO		  :
*  Date           :	12/April/2018
****************************************************************************************************/
bool CWardWizUpdateManager::MakeDownloadUrlList4EPSUpdtMgr()
{
	try
	{
		DWORD dwCount = 0x00, i = 0x00;
		DWORD dwZipFileCount = 0x00, dwZipTotCount = 0x00;
		DWORD	dwProductID = 0x00;
		TCHAR	szZipFilePath[512] = { 0 };
		CString csurl, csProductID;

		m_vUrlZipLists.clear();

		dwCount = static_cast<DWORD>(vALUZipFileInfo.size());

		TCHAR	szDomainName[MAX_PATH] = { 0 };
		if (!GetDomainName(szDomainName, MAX_PATH))
		{
			AddLogEntry(L"### Failed to get GetDomainName in MakeDownloadUrlList4EPSUpdtMgr", 0, 0, true, SECONDLEVEL);
			return true;
		}

		GetProductID(dwProductID);

		if (!dwProductID)
		{
			AddLogEntry(L"### Product ID mismatched in MakeDownloadUrlList4EPSUpdtMgr", 0, 0, true, SECONDLEVEL);
			return true;
		}

		CString csProductName;
		if (dwProductID == 3)
			csProductName = L"Elite";

		for (; i < dwCount; i++)
		{
			_tcscpy(szZipFilePath, L"");
			swprintf_s(szZipFilePath, _countof(szZipFilePath), L"%s/%s.zip", vALUZipFileInfo[i].szServerLoc, vALUZipFileInfo[i].szFileName);
			csurl.Format(L"http://%s/%s/%s", szDomainName, szUpdateFolderName, szZipFilePath);
			csurl.Format(L"http://%s/%s/%s/%s", szDomainName, szUpdateFolderName, csProductName, szZipFilePath);
			m_vUrlZipLists.push_back(csurl);
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in VibraniumALUSrv::MakeDownloadUrlList4EPSUpdtMgr", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : StartALUpdtProcess4UdtMgr()
*  Description    : This function will download ini.,Fills struct with modified binaries,
Download that binaries and copy into its respective location
*  Author Name    : Amol Jaware
*  SR_NO		  :
*  Date			  :	10-April-2018
****************************************************************************************************/
DWORD CWardWizUpdateManager::StartALUpdtProcess4UdtMgr()
{
	m_dwIsUptoDateCompleted = ALUPDATEFAILED_INTERNETCONNECTION;
	DWORD dwStatus = 0x00;
	DWORD	dwALUpdateStatus = ALUPDATEDSUCCESSFULLY;
	theApp.m_bRequestFromUI = true;
	bool bSuccess = true;

	try
	{
		if (m_OSType != WINOS_XP && m_OSType != WINOS_XP64)
		{
			g_objcriticalSection.Unlock();
		}
		if (m_bEnableAutoliveUpdate == false && theApp.m_bRequestFromUI == true)
		{
			//Send to Tray MEssage Update Now PopUp
			//Issue Resolved: We dont need to wait after send Message to Tray, so setting bWait to false
			//Resolved By : Nitin K. 22 April 2015
			if (!SendMessage2UI(CLOSE_UPDATENOW_POPUP, 2, 0, 0, false))//2 is for Tray server 
			{
				AddLogEntry(L"### Failed to get response from tray server", 0, 0, true, SECONDLEVEL);
			}
		}
		bSuccess = CheckInternetConnection();
		if (!bSuccess)
		{
			dwALUpdateStatus = ALUPDATEFAILED_INTERNETCONNECTION;
			AddLogEntry(L"### Failed in StartALUpdtProcess4UdtMgr::No internet connection", 0, 0, true, SECONDLEVEL);

			goto Cleanup;
		}

		if (!IsDiskSpaceAvailable())
		{
			dwALUpdateStatus = ALUPDATEFAILED_LOWDISKSPACE;
			m_bUpdateFailed = true;
			AddLogEntry(L"### Failed in StartALUpdtProcess4UdtMgr::Low disk space", 0, 0, true, SECONDLEVEL);

			goto Cleanup;
		}

		//SetDownloadPercentage(SETMESSAGE, CHECKING4UPDATES, 0, 0, this);

		CreateDir4ProgFiles();
		CreateDir4ProgData();

		DWORD dwRetryCount = 0;
		while (dwRetryCount < MAX_UPDATE_RETRYCOUNT)
		{
			bSuccess = DownLoadIniFileAndParse4EPS();
			if (bSuccess)
			{
				break;
			}
			AddLogEntry(L">>> Retry to DownLoadIniFile and Parse inside StartALUpdtProcess4UdtMgr", 0, 0, true, FIRSTLEVEL);
			dwRetryCount++;
		}

		if (!bSuccess)
		{
			AddLogEntry(L"### Failed in StartALUpdtProcess4UdtMgr::DownLoadIniFileAndParse4EPS", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}
		AddLogEntry(L">>> susceed in StartALUpdtProcess4UdtMgr::Product is not up to date!!!", 0, 0, true, ZEROLEVEL);

		bSuccess = DownLoadLatestFilesFromServer4EPSUpdtMgr(dwStatus);
		if (!bSuccess)
		{
			AddLogEntry(L"### Failed in StartALUpdtProcess4UdtMgr::DownLoadLatestFilesFromServer", 0, 0, true, SECONDLEVEL);
			goto Cleanup;
		}

		if (dwStatus == 0x01)
		{
			goto Cleanup;
		}

		//SetDownloadPercentage(SETMESSAGE, UPDATINGFILES, 0, 0, this);
		
		if (MergeUpdates2ProductFolder())
		{
			AddLogEntry(L">>> Files copied successfully from prog data Elite folder to prog files Elite folder", 0, 0, true, ZEROLEVEL);
		}
		else
		{
			AddLogEntry(L">>> Failed to merge files from prog data Elite folder to prog files Elite folder", 0, 0, true, ZEROLEVEL);
		}
		
		AddLogEntry(L">>> Live-Updated successfully through StartALUpdtProcess4UdtMgr", 0, 0, true, SECONDLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in StartALUpdtProcess4UdtMgr", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	StopALUpdateProcessThread4UpdtMgr();

	return 0;
}

/***************************************************************************************************
*  Function Name  : DownLoadLatestFilesFromServer4EPSUpdtMgr()
*  Description    : Download mismatched files from server
*  Author Name    : Amol Jaware.
*  SR_NO		  :
*  Date			  :	12-April-2018
****************************************************************************************************/
bool CWardWizUpdateManager::DownLoadLatestFilesFromServer4EPSUpdtMgr(DWORD& dwStatus)
{
	bool bReturn = false;

	try
	{
		AddLogEntry(L">>> DownLoadLatestFilesFromServer4EPSUpdtMgr::StartDownloading", 0, 0, true, FIRSTLEVEL);
		m_objWinHttpManagerUpDM.StartCurrentDownload();

		if (!MakeDownloadUrlList4EPSUpdtMgr())
		{
			AddLogEntry(L"### Failed to MakeDownloadUrlList4EPSUpdtMgr", 0, 0, true, SECONDLEVEL);
		}
		
		if (!hTotalFileSizeThread)
		{
			hTotalFileSizeThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GetTotalFileSizeThread,
				this, 0, 0);
		}

		m_objWinHttpManagerUpDM.SetDownloadCompletedBytes(0);

		CString csFileName;
		bool bDownloadSuccess = false;
		bool bWaitTimeout = false;
		bool bDownloadFailed = false;

		unsigned int iItemCount = 0, iItemUpdatedAlready = 0x00;
		int TotalDownloadedFilesNo = static_cast<int>(m_vUrlZipLists.size());

		for (; iItemCount < m_vUrlZipLists.size(); iItemCount++) //for zip files downloading...
		{

			csFileName = m_vUrlZipLists[iItemCount];

			if (!csFileName.GetLength())
				continue;

			if (IsLatestFile(csFileName))
			{
				//SetDownloadPercentage(SETMESSAGE, ALREADYDOWNLOADED, (iItemCount + 1), TotalDownloadedFilesNo, this);
				iItemUpdatedAlready++;
				continue;
			}

			//SetDownloadPercentage(SETMESSAGE, DOWNLOADINGFILES, (iItemCount + 1), TotalDownloadedFilesNo, this);

			if (bWaitTimeout)
			{
				continue;
			}

			DWORD dwRetryCount = 0;
			while (dwRetryCount < MAX_UPDATE_RETRYCOUNT)
			{
				m_objWinHttpManagerUpDM.m_bIsConnected = true;
				bDownloadFailed = false;
				bWaitTimeout = false;

				if (!StartDownloadZipFile(csFileName))
				{
					if (!CheckInternetConnection())
					{
						if (!WaitForInternetConnection())
						{
							bWaitTimeout = true;
							AddLogEntry(L">>> Retry to download file : %s", csFileName, 0, true, FIRSTLEVEL);
							//break;
						}
					}
					else
					{
						bDownloadFailed = true;
						AddLogEntry(L">>> Retry to download file : %s", csFileName, 0, true, FIRSTLEVEL);
						//break;
					}
				}
				else
				{
					bDownloadSuccess = true;
					break;
				}
				dwRetryCount++;
			}
		}

		if (iItemUpdatedAlready)
		{
			if (iItemUpdatedAlready == m_vUrlLists.size())
				dwStatus = 0x01;
		}

		Sleep(1000);

		if (bWaitTimeout || bDownloadFailed)
		{
			//set as failed
			AddLogEntry(L"### bWaitTimeout or bDownloadFailed flag are true File : %s", csFileName, 0, true, SECONDLEVEL);
			return bReturn;
		}

		if (m_OSType != WINOS_XP && m_OSType != WINOS_XP64)
		{
			g_objcriticalSection.Unlock();
		}

		m_objWinHttpManagerUpDM.StopCurrentDownload();

		bReturn = true;
	}
	catch (...)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in DownLoadLatestFilesFromServer4EPSUpdtMgr", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : StartDownloadZipFile()
*  Description    : Start downloading files on by one.
*  Author Name    : Amol Jaware
*  SR_NO		  :
*  Date			  :	11-April-2018
****************************************************************************************************/
bool CWardWizUpdateManager::StartDownloadZipFile(LPCTSTR szZipUrlPath)
{
	try
	{
		if (!szZipUrlPath)
			return false;

		static int i = 0;
		CString csFileName(szZipUrlPath);
		int iFound = csFileName.ReverseFind(L'/');
		csFileName = csFileName.Right(csFileName.GetLength() - iFound - 1);

		STRUCT_DOWNLOAD_INFO sDownloadInfo = { 0 };
		_tcscpy_s(sDownloadInfo.szMainUrl, URL_SIZE, szZipUrlPath);
		_tcscpy_s(sDownloadInfo.szSectionName, csFileName);
		_tcscpy_s(sDownloadInfo.szExeName, csFileName);

		DWORD dwThreadSize = DEFAULT_DOWNLOAD_THREAD;
		if (theApp.m_bRequestFromUI)
		{
			dwThreadSize = DEFAULT_DOWNLOAD_THREAD + 0x02;
		}

		sDownloadInfo.dwDownloadThreadCount = dwThreadSize;

		TCHAR szTargetPath[MAX_PATH] = { 0 };
		CString csElitDirectory = L"Elite";
		_stprintf_s(szTargetPath, MAX_PATH, L"%s\\%s", m_csWWizProgDataEPSPublishPath, csElitDirectory);

		if (!PathFileExists(szTargetPath))
			CreateDirectoryFocefully4EPSHierarchy(szTargetPath);
		CString csDBFileType;
		if (i < vALUZipFileInfo.size())
		{
			csDBFileType = vALUZipFileInfo[i].szServerLoc;
			i++;
		}
		//download update files in their respective directories
		if (csDBFileType == L"32")
		{
			_stprintf_s(sDownloadInfo.szLocalTempDownloadPath, MAX_PATH, L"%s\\32\\", szTargetPath);
			_stprintf_s(sDownloadInfo.szLocalPath, MAX_PATH, L"%s\\32\\", szTargetPath);
			_stprintf_s(szTargetPath, MAX_PATH, L"%s\\32\\%s", szTargetPath, csFileName);
		}
		if (csDBFileType == L"64")
		{
			_stprintf_s(sDownloadInfo.szLocalTempDownloadPath, MAX_PATH, L"%s\\64\\", szTargetPath);
			_stprintf_s(sDownloadInfo.szLocalPath, MAX_PATH, L"%s\\64\\", szTargetPath);
			_stprintf_s(szTargetPath, MAX_PATH, L"%s\\64\\%s", szTargetPath, csFileName);
		}
		if (csDBFileType == L"Common")
		{
			_stprintf_s(sDownloadInfo.szLocalTempDownloadPath, MAX_PATH, L"%s\\Common\\", szTargetPath);
			_stprintf_s(sDownloadInfo.szLocalPath, MAX_PATH, L"%s\\Common\\", szTargetPath);
			_stprintf_s(szTargetPath, MAX_PATH, L"%s\\Common\\%s", szTargetPath, csFileName);
		}
		if (csDBFileType == L"CommonDB")
		{
			_stprintf_s(sDownloadInfo.szLocalTempDownloadPath, MAX_PATH, L"%s\\CommonDB\\", szTargetPath);
			_stprintf_s(sDownloadInfo.szLocalPath, MAX_PATH, L"%s\\CommonDB\\", szTargetPath);
			_stprintf_s(szTargetPath, MAX_PATH, L"%s\\CommonDB\\%s", szTargetPath, csFileName);
		}

		DWORD dwTotalFileSize = 0x00;
		TCHAR szInfo[MAX_PATH] = { 0 };
		DWORD dwBufLen = MAX_PATH * sizeof(TCHAR);
		CWWizHttpManager objWinHttpManager;

		if (!objWinHttpManager.Initialize(szZipUrlPath))
		{
			AddLogEntry(L"### Initialize Failed in StartDownloadZipFile, URLPath: [%s]", szZipUrlPath, 0, true, SECONDLEVEL);
			return false;
		}
		if (!objWinHttpManager.GetHeaderInfo(WINHTTP_QUERY_CONTENT_LENGTH, szInfo, dwBufLen))
		{
			AddLogEntry(L"### GetHeaderInfo Failed in StartDownloadZipFile, URLPath: [%s]", szZipUrlPath, 0, true, SECONDLEVEL);
			return false;
		}
		dwTotalFileSize = _wtol(szInfo);

		if (PathFileExists(szTargetPath))
		{
			DWORD dwStartBytes = 0;
			m_hFile = CreateFile(szTargetPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			if (m_hFile != INVALID_HANDLE_VALUE)
			{
				dwStartBytes = GetFileSize(m_hFile, 0);
				CloseHandle(m_hFile);
				m_hFile = NULL;
			}

			if (dwStartBytes == dwTotalFileSize)
			{
				if (!CheckForCorruption(szTargetPath))
				{
					//m_lTotalDownloadedBytes += dwTotalFileSize;
					//m_objGUI.SetDownloadedBytes(dwTotalFileSize, dwTotalFileSize, 0);
					return true;
				}
			}
			SetFileAttributes(szTargetPath, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(szTargetPath);

		}

		if (!m_pDownloadController4UpdtMgr)
			return false;

		bool bDownloadFailed = false;
		DWORD dwRetryCount = 0x00;
		do
		{
			bDownloadFailed = false;
			m_pDownloadController4UpdtMgr->ResetInitData();
			if (!m_pDownloadController4UpdtMgr->StartController(&sDownloadInfo))
			{
				AddLogEntry(L"### Retry to download: FilePath: [%s\\%s]", sDownloadInfo.szLocalTempDownloadPath, sDownloadInfo.szExeName, true, FIRSTLEVEL);
				bDownloadFailed = true;
				dwRetryCount++;
			}
		} while (bDownloadFailed && dwRetryCount < MAX_RETRY_COUNT);

		if (bDownloadFailed)
			return false;

		AddLogEntry(L">>> File Downloaded: FilePath: [%s\\%s]", sDownloadInfo.szLocalTempDownloadPath, sDownloadInfo.szExeName, true, ZEROLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::StartDownloadZipFile", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : ReadAutoliveUpdateEnableCheck()
*  Description    : Read AutoliveUpdate Check
*  Author Name    : Vilas , Neha
**  SR_NO		  : WRDWIZALUSRV_0073
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
void CWardWizUpdateManager::ReadAutoliveUpdateEnableCheck()
{
	CITinRegWrapper objReg;

	DWORD dwAutoliveUpdate = 0;
	if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"dwAutoDefUpdate", dwAutoliveUpdate) != 0x00)
	{
		AddLogEntry(L"### Failed to get Registry Entry for dwAutoDefUpdate", 0, 0, true, ZEROLEVEL);;
		return;
	}

	if (dwAutoliveUpdate == 0)
	{
		m_bEnableAutoliveUpdate = false;
	}
	else
	{
		m_bEnableAutoliveUpdate = true;
	}
	return;
}

/***************************************************************************************************
*  Function Name  : StartALUpdateProcess()
*  Description    : This threat will download ini.,Fills struct with modified binaries,
Download that binaries and copy into its respective location
*  Author Name    : Vilas , Neha
*  SR_NO		  : WRDWIZALUSRV_0032
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
DWORD CWardWizUpdateManager::StartALUpdateProcess()
{
	DWORD	dwRet = 0x00;
	DWORD	dwStatus = 0x00;
	DWORD	dwALUpdateStatus = ALUPDATEDSUCCESSFULLY;
	
	m_dwIsUptoDateCompleted = ALUPDATEFAILED_INTERNETCONNECTION;
	theApp.m_bRequestFromUI = true;
	bool bSuccess = true;
	try
	{
		//Unlock the critical section object as this is start call.
		if (m_OSType != WINOS_XP && m_OSType != WINOS_XP64)
		{
			g_objcriticalSection.Unlock();
		}

		//Issue resolved :0000178 Issue with Tray Notification.
		if (m_bEnableAutoliveUpdate == false && theApp.m_bRequestFromUI == true)
		{
			//Send to Tray MEssage Update Now PopUp
			//Issue Resolved: We dont need to wait after send Message to Tray, so setting bWait to false
			//Resolved By : Nitin K. 22 April 2015
			if (!SendMessage2UI(CLOSE_UPDATENOW_POPUP, 2, 0, 0, false))//2 is for Tray server 
			{
				AddLogEntry(L"### Failed to get response from tray server", 0, 0, true, SECONDLEVEL);
			}
		}

		/* Issue NO - 159
		Failed to update product. Please check internet connection”
		even if internet connection is there
		Neha G.
		10/sept/2014
		*/
		for (int iRetryCount = 0x00; iRetryCount < 0x02; iRetryCount++)
		{
			bSuccess = CheckInternetConnection();
			if (bSuccess)
			{
				break;
			}
		}

		if (!bSuccess)//if internet problem persists, it returns false only for this case
		{
			dwALUpdateStatus = ALUPDATEFAILED_INTERNETCONNECTION;
			AddLogEntry(L"### Failed in StartALUpdateProcess::No internet connection", 0, 0, true, SECONDLEVEL);

			goto Cleanup;
		}
		//Issue: If disk space is low in C: drive and if tried to update the product then warning popup appearing as 'failed to update the product. Please check internet connection'
		//Resolved by: Nitin K
		if (!IsDiskSpaceAvailable())
		{
			dwALUpdateStatus = ALUPDATEFAILED_LOWDISKSPACE;
			m_bUpdateFailed = true;
			AddLogEntry(L"### Failed in StartALUpdateProcess::Low disk space", 0, 0, true, SECONDLEVEL);

			goto Cleanup;
		}

		TCHAR	szUpdateServerCount[10] = { 0 };
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\UpdateServers.ini";
		GetPrivateProfileString(L"UPDATESERVERS", L"Count", L"0", szUpdateServerCount, 511, csIniFilePath);
		CString csUpdateServerCount = (LPCTSTR)szUpdateServerCount;
		m_iServerCount = _ttoi(csUpdateServerCount);
		
		SetDownloadPercentage(SETMESSAGE, CHECKING4UPDATES, 0, 0, this);

		//pThis->SendUpdateInfoToGUI(SETMESSAGE, L"Checking for updates", L"", 0, 0);
		DWORD dwRetryCount = 0;
		if (!m_iServerCount)
		{
			while (dwRetryCount < MAX_UPDATE_RETRYCOUNT)
			{
				bSuccess = DownLoadIniFileAndParse();
				if (bSuccess)
				{
					break;
				}
				AddLogEntry(L">>> Retry to DownLoadIniFile and Parse", 0, 0, true, FIRSTLEVEL);
				dwRetryCount++;
			}
		}
		else
		{
			for (int i = 0; i < m_iServerCount; i++)
			{
				dwRetryCount = 0;
				CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\UpdateServers.ini";
				CString csCurrVal;
				TCHAR	szUpdateServerName[255] = { 0 };
				csCurrVal.Format(L"%d", i + 1);
				GetPrivateProfileString(L"UPDATESERVERS", csCurrVal, L"0", szUpdateServerName, 511, csIniFilePath);
				g_csServerName = szUpdateServerName;
				while (dwRetryCount < MAX_UPDATE_RETRYCOUNT)
				{
					bSuccess = DownLoadIniFileAndParse();
					if (bSuccess)
					{
						break;
					}
					AddLogEntry(L">>> Retry to DownLoadIniFile and Parse", 0, 0, true, FIRSTLEVEL);
					dwRetryCount++;
				}
				if (bSuccess)
				{
					break;
				}
			}
		}

		if (!bSuccess)
		{
			dwALUpdateStatus = ALUPDATEFAILED_DOWNLOADINIPARSE;
			AddLogEntry(L"### Failed in StartALUpdateProcess::DownLoadIniFileAndParse", 0, 0, true, SECONDLEVEL);
			//if(!m_bRequestFromUI && g_bEnableAutoliveUpdate == true)
			//{
			m_bUpdateFailed = true;
			//}
			goto Cleanup;
		}

		AddLogEntry(L">>> succeed in StartALUpdateProcess::DownLoadIniFileAndParse", 0, 0, true, ZEROLEVEL);

		if (!vALUFileInfo.size())
		{
			AddLogEntry(L">>> StartALUpdateProcess::Product is upto date!!!", 0, 0, true, ZEROLEVEL);

			dwALUpdateStatus = ALUPDATED_UPTODATE;
			m_dwIsUptoDateCompleted = dwALUpdateStatus;
			//update date and time only after successful updates.
			if (!UpdateTimeDate())
			{
				AddLogEntry(L"### Failed to write update date into registry", 0, 0, true, SECONDLEVEL);
			}

			//Pass this message to GUI
			//SendUpdateInfoToGUI(SETUPDATESTATUS,L"",L"",3,0);

			//dwALUpdateStatus = ALUPDATED_UPTODATE;

			if (!theApp.m_bRequestFromUI && m_bEnableAutoliveUpdate == true)
			{
				// send message to tray ui that it is up - to - date
				m_bUpToDate = true;
			}
			goto Cleanup;
		}

		if (m_bEnableAutoliveUpdate == false && theApp.m_bRequestFromUI == false)
		{
			//Send to Tray MEssage Update Now PopUp
			if (!SendMessage2UI(UPDATENOW_POPUP, 2, 0, 0, true))//2 is for Tray server 
			{
				AddLogEntry(L"### Failed to get response from tray server", 0, 0, true, SECONDLEVEL);
			}

			//it is blocking call
			ClearMemoryMapObjects();
			dwRet = 0x00;
			//return 0;
		}

		AddLogEntry(L">>> susceed in StartALUpdateProcess::Product is not up to date!!!", 0, 0, true, ZEROLEVEL);

		bSuccess = DownLoadLatestFilesFromServer(dwStatus);
		if (!bSuccess)
		{
			dwALUpdateStatus = ALUPDATEFAILED_DOWNLOADFILE;
			AddLogEntry(L"### Failed in StartALUpdateProcess::DownLoadLatestFilesFromServer", 0, 0, true, SECONDLEVEL);
			if (!theApp.m_bRequestFromUI)
			{
				m_bUpdateFailed = true;
			}
			goto Cleanup;
		}

		if (dwStatus == 0x01)
		{
			if (theApp.m_bRequestFromUI)
			{
				/*	if(!SendUpdateInfoToGUI(SETUPDATESTATUS, L"", L"", 1, 0))
				{
				AddLogEntry(L"### Failed in SendUpdateInfoToGUI First Param: 1", 0, 0, true, SECONDLEVEL);
				}
				*/
				dwALUpdateStatus = ALUPDATEDSUCCESSFULLY;
				AddLogEntry(L"### Failed in StartALUpdateProcess::Up-to-date", 0, 0, true, SECONDLEVEL);
			}
			goto Cleanup;
		}

		AddLogEntry(L">>> suscced in StartALUpdateProcess::DownLoadLatestFilesFromServer", 0, 0, true, ZEROLEVEL);
		
		SetDownloadPercentage(SETMESSAGE, UPDATINGFILES, 0, 0, this);
		//pThis->SendUpdateInfoToGUI(SETMESSAGE, L"Updating files", L"", 0, 0);

		////Wait for 1 and half seconds to close the download session properly.
		//Sleep(1500);

		bSuccess = ExtractAllDownLoadedFiles();
		if (!bSuccess)
		{
			dwALUpdateStatus = ALUPDATEFAILED_EXTRACTFILE;
			AddLogEntry(L"### Failed in StartALUpdateProcess::ExtractAllDownLoadedFiles", 0, 0, true, SECONDLEVEL);
			if (!theApp.m_bRequestFromUI)
			{
				m_bUpdateFailed = true;
			}
			m_bExtractionFailed = true;
			goto Cleanup;
		}

		AddLogEntry(L">>> suscced in StartALUpdateProcess::ExtractAllDownLoadedFiles", 0, 0, true, ZEROLEVEL);
		//Issue : 0000449 Issue with size mentioned in downloaded column. : Once download was successful it was showing Downloaded 100%(0 kb)
		//Resolved by Nitin K.

		DWORD dwPercent = m_dwPercentage;
		if (g_iPreviousPerc >= (int)(m_dwPercentage + 1))
		{
			dwPercent = g_iPreviousPerc;
		}

		if (dwPercent >= 100)
		{
			dwPercent = 98;
		}

		SetDownloadPercentage(SETDOWNLOADPERCENTAGE, EMPTYSTRING, m_dwTotalFileSize, (dwPercent + 1), this);
		/*if (!pThis->SendUpdateInfoToGUI(SETDOWNLOADPERCENTAGE, L"", L"", pThis->m_dwTotalFileSize, (dwPercent + 1)))
		{
			AddLogEntry(L"### Failed in SendUpdateInfoToGUI SETDOWNLOADPERCENTAGE", 0, 0, true, SECONDLEVEL);
		}*/

		bSuccess = ReplaceDownloadedFiles();
		if (bSuccess)
		{
			dwALUpdateStatus = ALUPDATEFAILED_UPDATINGFILE;
			AddLogEntry(L"### Failed in StartALUpdateProcess::ReplaceDownloadedFiles", 0, 0, true, SECONDLEVEL);
			if (!theApp.m_bRequestFromUI)
			{
				m_bUpdateFailed = true;
			}
			goto Cleanup;
		}

		//update date and time only after successful updates.
		if (!UpdateTimeDate())
		{
			AddLogEntry(L"### Failed to write update date into registry", 0, 0, true, SECONDLEVEL);
		}

		AddLogEntry(L">>> Live-Updated successfully", 0, 0, true, SECONDLEVEL);

		//Varada Ikhar, Date:29/01/2015, Version:1.8.3.5,
		//Issue : Even if auto-live update is off, it shows "Product Updated Successfully" pop-up.
		m_bUpdateSuccess = true;

		//Issue : 0000449 Issue with size mentioned in downloaded column. : Once download was successful it was showing Downloaded 100%(0 kb)
		//Resolved by Nitin K.
		
		SetDownloadPercentage(SETDOWNLOADPERCENTAGE, EMPTYSTRING, m_dwTotalFileSize, 100, this);
		//pThis->SendUpdateInfoToGUI(SETDOWNLOADPERCENTAGE, L"", L"", pThis->m_dwTotalFileSize, 100);

		//Commented out to send update status to GUI at last
		//Commented by Vilas on 04 May 2015
		//SendUpdateInfoToGUI(SETUPDATESTATUS, L"", L"", 1, 0);

		m_dwIsUptoDateCompleted = dwALUpdateStatus = ALUPDATEDSUCCESSFULLY;

		PerformPostUpdateRegOperations();


		//Do not wait till database load when update is finished.
		if (SendData2CommService(RELOAD_SIGNATURE_DATABASE, false) != 0x00)
		{
			AddLogEntry(L"### Failed to send reload Database message to service", 0, 0, true, SECONDLEVEL);
		}

		if (!SendData2CommService(CLEAR_INDEXING, false))
		{
			AddLogEntry(L"### Failed to SendData2Service for CLEAR_INDEXING", 0, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in StartALUpdateProcess", 0, 0, true, SECONDLEVEL);
	}

Cleanup:
	m_dwIsUptoDateCompleted = dwALUpdateStatus;

	if (g_dwProductID == ELITE)
	{
		SendUpdateFinishedData2EPSClient(m_dwIsUptoDateCompleted);
	}

	if (m_dwIsUptoDateCompleted == ALUPDATEDSUCCESSFULLY)
	{
		RemoveTemporaryData();
	}
	else
	{
		RemovePatchIniFile();
	}

	////Added by Vilas on 04 May 2015
	if (theApp.m_bRequestFromUI)
		SetDownloadPercentage(SETUPDATESTATUS, EMPTYSTRING, dwALUpdateStatus, 0, this);

	if (!theApp.m_bEPSLiveUpdateNoUI)
		StopALUpdateProcessThread(); //show update finished popup
	else
		StopALUpdateProcess4NoUI();

	//remove proxy setting data
	m_objWinHttpManager.m_dwProxySett = 0X00;
	if (m_objWinHttpManager.m_csServer.GetLength() > 0)
	{
		m_objWinHttpManager.m_csServer.Empty();
	}

	//If update is failed and we send message to use to retry, 
	//but some of the handles still open so we need to focefull exit the process, 
	//so that next retry update can go in success.
	if (m_bExtractionFailed)
	{
		return 0;
	}

	return dwRet;
}

/***************************************************************************************************
*  Function Name  : ExtractAllDownLoadedFiles()
*  Description    : Extract all zip files which are downloaded.
*  Author Name    : Vilas
*  SR_NO		  : WRDWIZALUSRV_0035
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
****************************************************************************************************/
bool CWardWizUpdateManager::ExtractAllDownLoadedFiles()
{
	DWORD	dwCount = 0x00, i = 0x00;

	TCHAR	szZipFilePath[512] = { 0 };
	TCHAR	szZipUnzipPath[512] = { 0 };
	DWORD	dwUnzippedCount = 0;

	try
	{

		AddLogEntry(L">>> Inside CVibraniumUpdateManager::ExtractAllDownLoadedFiles started", 0, 0, true, ZEROLEVEL);
		dwCount = static_cast<DWORD>(vALUFileInfo.size());

		for (; i < dwCount; i++)
		{
			swprintf_s(szZipFilePath, _countof(szZipFilePath), L"%s\\%s\\%s.zip", m_szAllUserPath, m_csAppFolderName, vALUFileInfo[i].szFileName);
			//swprintf_s(szZipUnzipPath, _countof(szZipUnzipPath), L"%s\\Vibranium\\%s", m_szAllUserPath, vALUFileInfo[i].szFileName );
			swprintf_s(szZipUnzipPath, _countof(szZipUnzipPath), L"%s\\%s\\%s_%s", m_szAllUserPath, m_csAppFolderName, vALUFileInfo[i].szFileName, L"WWIZALT");

			if (!PathFileExists(szZipFilePath))
			{
				continue;
			}

			if (PathFileExists(szZipUnzipPath))
				DeleteFileForcefully(szZipUnzipPath);

			DWORD dwRetryCount = 0;
			DWORD dwDownloadRetryCount = 0;
			while (dwRetryCount < MAX_UPDATE_RETRYCOUNT)
			{
				//Old extraction failed so we first checking for ZipArchive
				//29 April 2015 by Vilas
				if (PathFileExists(szZipUnzipPath))
					DeleteFileForcefully(szZipUnzipPath);

				if (!UnzipUsingZipArchive(szZipFilePath, szZipUnzipPath))
				{
					dwUnzippedCount++;
					break;
				}
				else
				{
					DWORD dwRet = UnzipUsingShell32API(szZipFilePath, szZipUnzipPath);
					if (dwRet == 0x00)
					{
						dwUnzippedCount++;
						break;
					}
					else
					{
						CString csLog;
						csLog.Format(L"### UnzipUsingShell32API Failed, ErrorCode: %d, ZipFilePath: %s, Unzip file: %s", dwRet, szZipFilePath, szZipUnzipPath);
						AddLogEntry(L"%s", csLog, 0, true, SECONDLEVEL);
					}
				}

				if (theApp.UnzipSingleFile)
				{
					AddLogEntry(L">>> Extracting zip file: %s, Unzip file: %s", szZipFilePath, szZipUnzipPath, true, FIRSTLEVEL);
					if (theApp.UnzipSingleFile(szZipFilePath, szZipUnzipPath))
					{
						AddLogEntry(L"### Retry to Extract zip file path: %s, Unzip file path: %s", szZipFilePath, szZipUnzipPath, true, SECONDLEVEL);
					}
					else
					{
						DWORD dwFileSize = 0;
						if (GetFileSize(szZipUnzipPath, dwFileSize))
						{
							if (dwFileSize == 0)
							{
								//Secondary level of extraction
								//Added by Vilas on 24 April 2015
								if (UnzipUsingZipArchive(szZipFilePath, szZipUnzipPath))
								{
									CString csLog;
									csLog.Format(L"### Extracted zip file: %s, File size: %d", szZipFilePath, dwFileSize);
									AddLogEntry(csLog, szZipFilePath, szZipUnzipPath, true, SECONDLEVEL);

									DWORD dwRet = UnzipUsingShell32API(szZipFilePath, szZipUnzipPath);
									if (dwRet == 0x00)
									{
										dwUnzippedCount++;
										break;
									}
									else
									{
										CString csLog;
										csLog.Format(L"### UnzipUsingShell32API Failed, ErrorCode: %d, ZipFilePath: %s, Unzip file: %s", dwRet, szZipFilePath, szZipUnzipPath);
										AddLogEntry(L"%s", csLog, 0, true, SECONDLEVEL);
									}

									dwRetryCount++;

									//Code here if retry count is more than MAX_RETRY_COUNT, delete the file
									//So that new fresh copy will get downloaded and able to retrive.
									if (dwRetryCount >= MAX_UPDATE_RETRYCOUNT)
									{
										SetFileAttributes(szZipFilePath, FILE_ATTRIBUTE_NORMAL);
										DeleteFile(szZipFilePath);

										//If retry download has been happened for more than 3 times then will go for failed.
										if (dwDownloadRetryCount <= 0x03)
										{
											//This is workaround, if any file extraction failed, it may have possibility that file is corrupted.
											//so this file need to download again 
											CString csURLPath;
											if (!GetURLPathFromFilePath(szZipFilePath, csURLPath))
											{
												break;
											}

											//Download the file again.
											TCHAR szInfo[MAX_PATH] = { 0 };
											DWORD dwBufLen = MAX_PATH * sizeof(TCHAR);
											CWinHttpManager objWinHttpManager;
											if (objWinHttpManager.Initialize(csURLPath))
											{
												if (!objWinHttpManager.GetHeaderInfo(WINHTTP_QUERY_CONTENT_LENGTH, szInfo, dwBufLen))
												{
													return false;
												}

												DWORD dwTotalFileSize = _wtol(szInfo);
												if (objWinHttpManager.Download(szZipFilePath, 0x00, dwTotalFileSize))
												{
													dwRetryCount--;
													dwDownloadRetryCount++;
												}
											}
										}
									}

									continue;
								}
							}
							//g_dwUnzippedCount++;
							dwUnzippedCount++;
							break;
						}
						else
						{
							CString csLog;
							csLog.Format(L"### Extracted zip file: %s, File size: %d", szZipFilePath, dwFileSize);
							AddLogEntry(csLog, szZipFilePath, szZipUnzipPath, true, FIRSTLEVEL);
						}
						AddLogEntry(L">>> Extracted zip file: %s, Unzip file: %s", szZipFilePath, szZipUnzipPath, true, FIRSTLEVEL);
					}
				}
				else
				{
					//Secondary level of extraction
					//Added by Vilas on 24 April 2015
					if (!UnzipUsingZipArchive(szZipFilePath, szZipUnzipPath))
					{
						dwUnzippedCount++;
						break;
					}
					else
					{
						AddLogEntry(L"### Failed in UnzipUsingZipArchive to extract zip file path: %s, Unzip file path: %s", szZipFilePath, szZipUnzipPath, true, SECONDLEVEL);
					}
				}
				dwRetryCount++;

				//Code here if retry count is more than MAX_RETRY_COUNT, delete the file
				//So that new fresh copy will get downloaded and able to retrive.
				if (dwRetryCount >= MAX_UPDATE_RETRYCOUNT)
				{
					SetFileAttributes(szZipFilePath, FILE_ATTRIBUTE_NORMAL);
					DeleteFile(szZipFilePath);

					//If retry download has been happened for more than 3 times then will go for failed.
					if (dwDownloadRetryCount <= 0x03)
					{
						//This is workaround, if any file extraction failed, it may have possibility that file is corrupted.
						//so this file need to download again 
						CString csURLPath;
						if (!GetURLPathFromFilePath(szZipFilePath, csURLPath))
						{
							break;
						}

						//Download the file again.
						TCHAR szInfo[MAX_PATH] = { 0 };
						DWORD dwBufLen = MAX_PATH * sizeof(TCHAR);
						CWinHttpManager objWinHttpManager;
						if (objWinHttpManager.Initialize(csURLPath))
						{
							if (!objWinHttpManager.GetHeaderInfo(WINHTTP_QUERY_CONTENT_LENGTH, szInfo, dwBufLen))
							{
								return false;
							}

							DWORD dwTotalFileSize = _wtol(szInfo);
							if (objWinHttpManager.Download(szZipFilePath, 0x00, dwTotalFileSize))
							{
								dwRetryCount--;
								dwDownloadRetryCount++;
							}
						}
					}
				}
			}

			//To remove read only for extracted file
			//15 May 2015 by Vilas
			if (PathFileExists(szZipUnzipPath))
				SetFileAttributes(szZipUnzipPath, FILE_ATTRIBUTE_NORMAL);
		}

		AddLogEntry(L">>> CVibraniumUpdateManager::ExtractAllDownLoadedFiles finished ", 0, 0, true, ZEROLEVEL);
		if (dwUnzippedCount != dwCount)
		{
			CString csLog;
			csLog.Format(L"UnZipped Count: %d, Total File Count:  %d", dwUnzippedCount, dwCount);
			AddLogEntry(L"### Zip file count not matched with extracted files count %s", csLog, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### CVibraniumUpdateManager::ExtractAllDownLoadedFiles exception", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : DeleteFileForcefully()
*  Description    : Delete files forcefully from temp folder after extracting
*  Author Name    : Vilas
*  SR_NO		  : WRDWIZALUSRV_0036
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
****************************************************************************************************/
bool CWardWizUpdateManager::DeleteFileForcefully(LPTSTR lpszFilePath)
{
	try
	{
		if (!PathFileExists(lpszFilePath))
			return false;

		SetFileAttributes(lpszFilePath, FILE_ATTRIBUTE_NORMAL);
		DeleteFile(lpszFilePath);

		if (!PathFileExists(lpszFilePath))
			return false;

		TCHAR	szRenamePath[512] = { 0 };

		//swprintf_s(szRenamePath, _countof(szRenamePath), L"%s.%lu", m_szAllUserPath, GetTickCount() );

		//Modified for deleting existing file
		//Added by Vilas on 29 April 2015
		swprintf_s(szRenamePath, _countof(szRenamePath), L"%s.%lu", lpszFilePath, GetTickCount());

		SetFileAttributes(szRenamePath, FILE_ATTRIBUTE_NORMAL);
		DeleteFile(szRenamePath);
		_wrename(lpszFilePath, szRenamePath);
	}
	catch (...)
	{
		AddLogEntry(L"### CVibraniumUpdateManager::DeleteFileForcefully::Exception", 0, 0, true, SECONDLEVEL);
	}

	return false;
}

/***********************************************************************************************
Function Name  : GetURLPathFromFilePath
Description    : Function to get URL path from file name.
Author Name    : Ram Shelke
SR.NO		   :
Date           : 5th Jan 2016
***********************************************************************************************/
bool CWardWizUpdateManager::GetURLPathFromFilePath(LPTSTR szZipFilePath, CString &csURLPath)
{
	bool bReturn = false;
	try
	{
		if (!szZipFilePath)
			return bReturn;

		CString csFilePath(szZipFilePath);
		csFilePath.MakeLower();

		CString cFileName = csFilePath.Right(csFilePath.GetLength() - (csFilePath.ReverseFind(L'\\') + 1));
		unsigned int iItemCount = 0;
		for (; iItemCount < m_vUrlLists.size(); iItemCount++)
		{
			CString csURL = m_vUrlLists[iItemCount];
			csURL.MakeLower();
			if (csURL.Find(cFileName) > 0)
			{
				csURLPath = m_vUrlLists[iItemCount];
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::GetURLPathFromFilePath", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : UnzipUsingShell32API
Description    : Function to Unzip file using Shell 32 COM API.
Author Name    : Ram Shelke
SR.NO		   :
Date           : 24 April 2015
***********************************************************************************************/
DWORD CWardWizUpdateManager::UnzipUsingShell32API(LPTSTR lpszZipFilePath, LPTSTR lpszUnizipFilePath)
{
	DWORD dwReturn = 0x00;

	__try
	{
		dwReturn = UnzipUsingShell32APISEH(lpszZipFilePath, lpszUnizipFilePath);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwReturn = 0x05;
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::UnzipUsingShell32API", 0, 0, true, SECONDLEVEL);
	}

	return dwReturn;
}

/***********************************************************************************************
Function Name  : UnzipUsingShell32APISEH
Description    : Structured Exception handling function which Extracts file using shellwin32API
Author Name    : Ram Shelke
SR.NO		   :
Date           : 24 April 2015
***********************************************************************************************/
DWORD CWardWizUpdateManager::UnzipUsingShell32APISEH(LPTSTR lpszZipFilePath, LPTSTR lpszUnizipFilePath)
{
	DWORD dwReturn = 0x00;
	try
	{
		//Sanity check
		if (lpszZipFilePath == NULL || lpszUnizipFilePath == NULL)
		{
			dwReturn = 0x01;
			return dwReturn;
		}

		//To get name of zip file.
		TCHAR szZipFilePath[512] = { 0 };
		_tcscpy_s(szZipFilePath, 512, lpszZipFilePath);

		TCHAR szZipFileName[MAX_PATH] = { 0 };
		TCHAR *pZipTemp = wcsrchr(szZipFilePath, '\\');
		if (!pZipTemp)
		{
			dwReturn = 0x02;
			return dwReturn;
		}

		pZipTemp++;
		_tcscpy_s(szZipFileName, MAX_PATH, pZipTemp);

		TCHAR szUnZipFilePath[512] = { 0 };
		_tcscpy_s(szUnZipFilePath, 512, lpszUnizipFilePath);

		TCHAR szFilePath[2048] = { 0 };
		TCHAR szFileName[MAX_PATH] = { 0 };

		//Get here the path of Taget directory
		TCHAR *pUnZipTemp = wcsrchr(szUnZipFilePath, '\\');
		if (!pUnZipTemp)
		{
			dwReturn = 0x03;
			return dwReturn;
		}

		pUnZipTemp++;
		_tcscpy_s(szFileName, MAX_PATH, pUnZipTemp);

		pUnZipTemp--;
		*pUnZipTemp = '\0';
		_tcscpy_s(szFilePath, 2048, szUnZipFilePath);

		//Check here that target file exists
		if ((_waccess(szFilePath, 0)) == -1)
		{
			dwReturn = 0x04;
			return dwReturn;
		}

		CString csActualFileName = szZipFileName;
		csActualFileName = csActualFileName.Left(csActualFileName.GetLength() - 4);//Get here the actual file name.

		TCHAR szSourcePath[512] = { 0 };
		TCHAR szTargetPath[512] = { 0 };
		swprintf(szSourcePath, L"%s\\%s", szFilePath, csActualFileName);
		swprintf(szTargetPath, L"%s\\%s", szFilePath, szFileName);

		//Delete here if target path exists.
		if (PathFileExists(szTargetPath))
		{
			SetFileAttributes(szTargetPath, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(szTargetPath);
		}

		//Unzip the file using Shellwin32 API
		HRESULT hr = ::CoInitialize(NULL);
		hr = UnzipToFolder(lpszZipFilePath, szFilePath);
		::CoUninitialize();

		if (FAILED(hr))
		{
			CString csError;
			csError.Format(L"Error: hr=0x%08X", hr);
			AddLogEntry(L"### Failed Function UnzipToFolder, %s", csError, 0, true, SECONDLEVEL);
			dwReturn = 0x05;
			return dwReturn;
		}

		//Rename the extracted file.
		SetFileAttributes(szSourcePath, FILE_ATTRIBUTE_NORMAL);
		if (_wrename(szSourcePath, szTargetPath) != 0)
		{
			AddLogEntry(L"### Failed to _wrename File, Source:[%s], Dest[%s] ", szSourcePath, szTargetPath, true, FIRSTLEVEL);
			dwReturn = 0x06;
			return dwReturn;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::UnzipUsingShell32APISEH, ZipFilePath: %s, UnZipFilePath:%s", lpszZipFilePath, lpszUnizipFilePath, true, SECONDLEVEL);
		dwReturn = 0x07;
	}
	return dwReturn;
}

/***********************************************************************************************
Function Name  : UnzipToFolder
Description    : Unzip a zip file to the specified folder.
Author Name    : Ram Shelke
SR.NO		   :
Date           : 24 April 2015
***********************************************************************************************/
HRESULT CWardWizUpdateManager::UnzipToFolder(PCWSTR pszZipFile, PCWSTR pszDestFolder)
{
	try
	{
		HRESULT hr;

		CComPtr<IShellDispatch> spISD;
		hr = spISD.CoCreateInstance(CLSID_Shell);
		if (FAILED(hr))
			return hr;

		CComVariant vtZipFile(pszZipFile);
		CComPtr<Folder> spZipFile;
		hr = spISD->NameSpace(vtZipFile, &spZipFile);
		if (FAILED(hr))
			return hr;

		CComVariant vtDestFolder(pszDestFolder);
		CComPtr<Folder> spDestination;
		hr = spISD->NameSpace(vtDestFolder, &spDestination);
		if (FAILED(hr))
			return hr;
		if (!spDestination)
			return E_POINTER;

		CComPtr<FolderItems> spFilesInside;
		hr = spZipFile->Items(&spFilesInside);
		if (FAILED(hr))
			return hr;

		CComPtr<IDispatch> spDispItem;
		hr = spFilesInside.QueryInterface(&spDispItem);
		if (FAILED(hr))
			return hr;

		CComVariant vtItem(spDispItem);
		CComVariant vtOptions(FOF_NO_UI);
		hr = spDestination->CopyHere(vtItem, vtOptions);
		if (FAILED(hr))
			return hr;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::UnzipToFolder, SourceFileName: %s, DestPath: %s", pszZipFile, pszDestFolder, true, SECONDLEVEL);
		return S_FALSE;
	}
	return S_OK;
}

/***********************************************************************************************
Function Name  : UnzipUsingZipArchive
Description    : Second level extraction.if function success return zero else non-zero.
Author Name    : Vilas Shelke
SR.NO		   :
Date           : 24 April 2015
***********************************************************************************************/
DWORD UnzipUsingZipArchiveSEH(LPTSTR lpszZipFilePath, LPTSTR lpszUnizipFilePath)
{
	DWORD	dwReturn = 0x00;

	CZipArchive zip;

	try
	{

		if (PathFileExists(lpszUnizipFilePath))
			DeleteFileForcefully(lpszUnizipFilePath);

		if (!zip.Open(lpszZipFilePath, CZipArchive::zipOpenReadOnly))
		{
			dwReturn = 0x01;
			goto Cleanup;
		}

		TCHAR	szPath[256] = { 0 };
		TCHAR	szFile[256] = { 0 };

		wcscpy_s(szPath, 255, lpszUnizipFilePath);

		TCHAR	*pFileName = wcsrchr(szPath, '\\');

		if (!pFileName)
		{
			dwReturn = 0x02;
			goto Cleanup;
		}

		TCHAR	*pTemp = pFileName;

		pFileName++;
		wcscpy_s(szFile, 255, pFileName);

		*pTemp = '\0';

		if (!zip.ExtractFile(0, szPath, true, szFile))
		{
			dwReturn = 0x03;
			goto Cleanup;
		}
	}
	catch (...)
	{
		dwReturn = 0x04;
		AddLogEntry(L"### Exception in UnzipUsingZipArchiveSEH", 0, 0, true, SECONDLEVEL);

		//Need to Delete Downloaded file, as may be it is corrupted, as throwing exception.
		zip.CloseFile(lpszZipFilePath);
		if (!DeleteFileForcefully(lpszZipFilePath))
		{
			AddLogEntry(L"### CVibraniumUpdateManager::UnzipUsingZipArchiveSEH:: Failed to delete file forcefully: %s ", lpszZipFilePath, 0, true, SECONDLEVEL);
		}
		return dwReturn;
	}
Cleanup:
	zip.CloseFile(lpszZipFilePath);
	return dwReturn;
}

/***************************************************************************************************
*  Function Name  : DeleteFileForcefully()
*  Description    : Delete files forcefully from temp folder after extracting
*  Author Name    : Vilas
*  SR_NO		  : WRDWIZALUSRV_0036
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
****************************************************************************************************/
bool DeleteFileForcefully(LPTSTR lpszFilePath)
{
	try
	{
		if (!PathFileExists(lpszFilePath))
			return false;

		SetFileAttributes(lpszFilePath, FILE_ATTRIBUTE_NORMAL);
		DeleteFile(lpszFilePath);

		if (!PathFileExists(lpszFilePath))
			return false;

		TCHAR	szRenamePath[512] = { 0 };

		//swprintf_s(szRenamePath, _countof(szRenamePath), L"%s.%lu", m_szAllUserPath, GetTickCount() );

		//Modified for deleting existing file
		//Added by Vilas on 29 April 2015
		swprintf_s(szRenamePath, _countof(szRenamePath), L"%s.%lu", lpszFilePath, GetTickCount());

		SetFileAttributes(szRenamePath, FILE_ATTRIBUTE_NORMAL);
		DeleteFile(szRenamePath);
		_wrename(lpszFilePath, szRenamePath);
	}
	catch (...)
	{
		AddLogEntry(L"### CVibraniumUpdateManager::DeleteFileForcefully::Exception", 0, 0, true, SECONDLEVEL);
	}

	return false;
}

/***********************************************************************************************
Function Name  : UnzipUsingZipArchive
Description    : Second level extraction.if function success return zero else non-zero.
Author Name    : Vilas Shelke
SR.NO		   :
Date           : 24 April 2015
***********************************************************************************************/
DWORD CWardWizUpdateManager::UnzipUsingZipArchive(LPTSTR lpszZipFilePath, LPTSTR lpszUnizipFilePath)
{
	DWORD dwReturn = 0x00;

	__try
	{
		dwReturn = UnzipUsingZipArchiveSEH(lpszZipFilePath, lpszUnizipFilePath);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		dwReturn = 0x05;
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::UnzipUsingZipArchive", 0, 0, true, SECONDLEVEL);
	}

	return dwReturn;
}

/***********************************************************************************************
Function Name  : GetFileSize
Description    : Function to get file size in reference agrgument.
if function success return true else false.
Author Name    : Ram Shelke
SR.NO		   :
Date           : 08 Apr 2015
***********************************************************************************************/
bool GetFileSize(LPTSTR pFilePath, DWORD &dwFileSize)
{
	bool bReturn = false;
	__try
	{
		HANDLE hFile = CreateFile(pFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			AddLogEntry(L"### In GetFileSize Error in opening file %s", pFilePath, 0, true, SECONDLEVEL);
			return bReturn;
		}

		dwFileSize = GetFileSize(hFile, NULL);
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;

		bReturn = true;

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in GetFileSize", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
Function Name  : IsDiskSpaceAvailable
Description    : To check whether disk space is available or not (will be checking for 512 MB of diskspace)
Author Name    : Nitin Kolapkar
SR.NO		   :
Date           : 5th Jan 2016
***********************************************************************************************/
bool CWardWizUpdateManager::IsDiskSpaceAvailable()
{

	bool bReturn = false;
	try
	{
		CString csDrive = L"";
		CString csWardWizPath = L"";
		DWORD64 TotalNumberOfFreeBytes;
		DWORD64 TotalNumberOfRequiredbites = 512000000;
		ULARGE_INTEGER					m_uliFreeBytesAvailable;     // bytes disponiveis no disco associado a thread de chamada
		ULARGE_INTEGER					m_uliTotalNumberOfBytes;     // bytes no disco
		ULARGE_INTEGER					m_uliTotalNumberOfFreeBytes; // bytes livres no disco
		csWardWizPath = GetWardWizPathFromRegistry();
		if (csWardWizPath.Compare(L""))
		{
			csDrive = csWardWizPath.Left(2);
		}


		// bytes available to caller
		m_uliFreeBytesAvailable.QuadPart = 0L;
		// bytes on disk
		m_uliTotalNumberOfBytes.QuadPart = 0L;
		// free bytes on disk
		m_uliTotalNumberOfFreeBytes.QuadPart = 0L;

		if (PathFileExists(csDrive))
		{
			if (!GetDiskFreeSpaceEx((LPCWSTR)csDrive, &m_uliFreeBytesAvailable, &m_uliTotalNumberOfBytes, &m_uliTotalNumberOfFreeBytes))
			{
				bReturn = false;
				AddLogEntry(L"### Failed in  GetDiskFreeSpaceEx", 0, 0, true, SECONDLEVEL);
			}
			TotalNumberOfFreeBytes = m_uliTotalNumberOfFreeBytes.QuadPart;

			if (TotalNumberOfRequiredbites < TotalNumberOfFreeBytes)
			{
				bReturn = true;
			}
			else
			{
				bReturn = false;
			}
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::IsDriveHaveRequiredSpace", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/***************************************************************************************************
*  Function Name  : RemoveTemporaryData
*  Description    : Function which removes temporary data.
*  Date			  :	13-Dec-2017
*  Author Name    : Ram Shelke
****************************************************************************************************/
bool CWardWizUpdateManager::RemoveTemporaryData()
{
	try
	{
		TCHAR	szAppPath[512] = { 0 };
		swprintf_s(szAppPath, _countof(szAppPath), L"%s\\%s", m_szAllUserPath, m_csAppFolderName);

		CStringArray csExcluded;
		csExcluded.Add(L"ALUDel.ini");
		csExcluded.Add(L"VibroPatchTS.ini");
		csExcluded.Add(L"WWizPatchP.ini");
		csExcluded.Add(L"WWizPatchT.ini");
		csExcluded.Add(L"WWizPatchB.ini");
		csExcluded.Add(L"VibroPatchAS.ini");

		if (PathFileExists(szAppPath))
		{
			DeleteFolderTree(szAppPath, csExcluded);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::RemoveTemporaryData", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function       : DeleteFolderTree
In Parameters  : CString csFilePath,
Out Parameters : bool
Description    :
Author		   : Ram Shelke
***************************************************************************/
bool CWardWizUpdateManager::DeleteFolderTree(CString csFilePath, CStringArray &csExcludedList)
{
	CFileFind findfile;
	csFilePath += _T("\\*.*");
	//To Check Whether The File Is Exist Or Not
	BOOL bCheck = findfile.FindFile(csFilePath);
	if (!bCheck)
	{
		return false;
	}
	while (bCheck)
	{
		//To Find Next File In Same Directory
		bCheck = findfile.FindNextFile();
		if (findfile.IsDots())
		{
			continue;
		}

		//To get file path
		csFilePath = findfile.GetFilePath();

		CString csFileName(csFilePath);
		int iFound = csFileName.ReverseFind(L'\\');
		csFileName = csFileName.Right(csFileName.GetLength() - iFound - 1);

		bool bFound = false;
		for (int iIndex = 0x00; iIndex < csExcludedList.GetCount(); iIndex++)
		{
			CString csExcluseItem = csExcludedList.GetAt(iIndex);
			if (csFileName.CompareNoCase(csExcluseItem) == 0)
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			//To set the file attribute to archive
			DWORD dwAttrs = GetFileAttributes(csFilePath);
			if (dwAttrs != INVALID_FILE_ATTRIBUTES && dwAttrs & FILE_ATTRIBUTE_READONLY)
			{
				SetFileAttributes(csFilePath, dwAttrs ^ FILE_ATTRIBUTE_READONLY);
			}
			::DeleteFile(csFilePath);
		}
	}
	//to close handle
	findfile.Close();
	return true;
}

/***************************************************************************************************
*  Function Name  : ReplaceDownloadedFiles()
*  Description    : Downloaded files get extracted get copy into production folder. &
If all files cpoied successfully, old files get deleted and new files get
replaced. If some files get failed to copy, all files get rolled back
*  Author Name    : Vilas , Neha
*  SR_NO		  : WRDWIZALUSRV_0037
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::ReplaceDownloadedFiles()
{
	CString	szFilePath;

	DWORD	dwCount = 0x00, i = 0x00;

	TCHAR	szSourceFilePath[512] = { 0 };
	TCHAR	szDestFilePath[512] = { 0 };
	TCHAR	szTemp[512] = { 0 };
	TCHAR	szReplaceDestination[512] = { 0 };

	bool	bCopied = true;
	int iStartPos = 0;
	TCHAR szAVPath[512] = { 0 };
	try
	{
		AddLogEntry(L">>> In ReplaceDownloadedFiles function", 0, 0, true, ZEROLEVEL);
		dwCount = static_cast<DWORD>(vALUFileInfo.size());

		for (; i<dwCount; i++)
		{
			ZeroMemory(szSourceFilePath, sizeof(szSourceFilePath));
			ZeroMemory(szDestFilePath, sizeof(szDestFilePath));
			SetDownloadPercentage(SETMESSAGE, UPDATINGFILES, (i + 1), dwCount, this);
			//SendUpdateInfoToGUI(SETMESSAGE, L"Updating files", L"", (i + 1), dwCount);
			swprintf_s(szSourceFilePath, _countof(szSourceFilePath), L"%s\\%s\\%s_%s", m_szAllUserPath, m_csAppFolderName, vALUFileInfo[i].szFileName, L"WWIZALT");
			AddLogEntry(L">>> SourcePath :: %s", szSourceFilePath, 0, true, ZEROLEVEL);

			//This code to replace the file in there respective folder.
			CString csReplaceDestination = L"";
			csReplaceDestination = vALUFileInfo[i].szFileShortPath;
			_tcsnccpy(szAVPath, theApp.m_szAVPath, (_tcslen(theApp.m_szAVPath) - 1));
			csReplaceDestination.Replace(L"AppFolder", szAVPath);
			iStartPos = 0;
			swprintf_s(szDestFilePath, _countof(szDestFilePath), L"%s\\%s", csReplaceDestination, vALUFileInfo[i].szFileName);
			AddLogEntry(L">>> Destination Path :: %s", szDestFilePath, 0, true, ZEROLEVEL);


			//GetFilePath and Create sub Directory inside the AppFolder, if required
			if (CheckDestFolderAndCreate(vALUFileInfo[i].szFileShortPath))
			{
				AddLogEntry(L"### Failed to create & Check dest folder path :: %s", vALUFileInfo[i].szFileShortPath, 0, true, SECONDLEVEL);
				break;
			}

			if (!PathFileExists(szSourceFilePath))
			{
				AddLogEntry(L"### Source path file is not existing :: %s", szSourceFilePath, 0, true, SECONDLEVEL);
				break;
			}

			bCopied = true;
			//Retry count three time to replace files.
			for (BYTE b = 0x00; b < MAX_UPDATE_RETRYCOUNT; b++)
			{

				if (PathFileExists(szDestFilePath))
				{
					if (CheckForServiceName(i))
					{
						AddLogEntry(L">>> Destination path file is existing :: %s", szDestFilePath, 0, true, ZEROLEVEL);
						swprintf_s(szTemp, _countof(szTemp), L"%s\\%s_%s", csReplaceDestination, vALUFileInfo[i].szFileName, L"WWIZALTSRV");

						SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL);
						if (DeleteFile(szTemp) == FALSE)
						{
							AddLogEntry(L">>> DeleteFile Failed :: %s", szTemp, 0, true, ZEROLEVEL);
						}
						if (_wrename(szDestFilePath, szTemp) != 0)
						{
							AddLogEntry(L">>> _wrename Failed :: %s", szTemp, 0, true, ZEROLEVEL);
						}
						swprintf_s(szTemp, _countof(szTemp), L"%s\\%s", csReplaceDestination, vALUFileInfo[i].szFileName);
					}
					else
					{
						AddLogEntry(L">>> Destination path file is existing :: %s", szDestFilePath, 0, true, ZEROLEVEL);
						swprintf_s(szTemp, _countof(szTemp), L"%s\\%s_%s", csReplaceDestination, vALUFileInfo[i].szFileName, L"WWIZALT");
						SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL);
						if (DeleteFile(szTemp) == FALSE)
						{
							AddLogEntry(L">>> DeleteFile Failed :: %s", szTemp, 0, true, ZEROLEVEL);
						}
					}
				}
				else
				{
					//No Need to create file to destination with 0KB.
					//Issue regarding on customers machine files are getting replaced with 0 KB.
					//HANDLE hFile = CreateFile(szDestFilePath, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ, NULL,CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
					//if(hFile == INVALID_HANDLE_VALUE)
					//{
					//	AddLogEntry(L">>> Destination path file is not created :: %s",szDestFilePath,0,true,ZEROLEVEL);
					//	break;
					//}
					//CloseHandle(hFile);
					//if( PathFileExists(szDestFilePath) )
					//{
					AddLogEntry(L">>> Destination path file is not existing :: %s", szDestFilePath, 0, true, ZEROLEVEL);
					swprintf_s(szTemp, _countof(szTemp), L"%s\\%s_%s", csReplaceDestination, vALUFileInfo[i].szFileName, L"WWIZALT");
					SetFileAttributes(szTemp, FILE_ATTRIBUTE_NORMAL);
					if (DeleteFile(szTemp) == FALSE)
					{
						AddLogEntry(L">>> DeleteFile Failed :: %s", szTemp, 0, true, ZEROLEVEL);
					}

					//}
				}

				//if( CopyFile(szSourceFilePath, szDestFilePath, FALSE ) )
				if (CopyFile(szSourceFilePath, szTemp, FALSE))
				{
					bCopied = false;
					m_dwReplacedCount++;
					int iReplacecount = m_dwReplacedCount;
					int iTotalFileCount = dwCount;
					DWORD dwPercentage = static_cast<DWORD>(((static_cast<double>((iReplacecount)) / iTotalFileCount)) * 19);
					if ((DWORD)g_iPreviousPerc < m_dwPercentage + dwPercentage)
					{
						DWORD dwPercent = m_dwPercentage;
						if (g_iPreviousPerc >= (int)(m_dwPercentage + dwPercentage + 1))
						{
							dwPercent = g_iPreviousPerc;
						}
						if ((dwPercent + dwPercentage + 1) <= 99)
						{
							SetDownloadPercentage(SETDOWNLOADPERCENTAGE, EMPTYSTRING, m_dwTotalFileSize, (dwPercent + dwPercentage + 1), this);
							//SendUpdateInfoToGUI(SETDOWNLOADPERCENTAGE, L"", L"", m_dwTotalFileSize, (dwPercent + dwPercentage + 1));
						}
					}
					AddEntryToALUDelIni(szTemp, vALUFileInfo[i].szFileName);
					_tcscpy(szTemp, L"");
					break;
				}
				Sleep(10);

			}

			if (bCopied)
			{
				AddLogEntry(L"### Failed to copy %s file to %s file", szSourceFilePath, szTemp, true, SECONDLEVEL);
				//RollBack all Files
				break;
			}

		}

		if (m_dwReplacedCount != dwCount)
		{
			//Roll back all renamed files
			AddLogEntry(L">>> In rollback function", 0, 0, true, ZEROLEVEL);
			ReplaceAllOriginalFiles();
			return true;
		}
		else
		{
			//Delete All renamed files
			AddLogEntry(L">>> In replace and delete downloaded file function", 0, 0, true, ZEROLEVEL);
			DeleteAllRenamedFiles();
			DeleteAllDownloadedFiles();
			return false;
		}
	}
	catch (...)
	{
		bCopied = false;
		AddLogEntry(L"### CVibraniumUpdateManager::ReplaceDownloadedFiles::Exception", 0, 0, true, SECONDLEVEL);
	}

	return bCopied;
}

/***************************************************************************************************
*  Function Name  : AddEntryToALUDelIni()
*  Description    : Add all new files to ALUdel.ini files.
*  Author Name    : Vilas, neha
*  SR_NO		  : WRDWIZALUSRV_0046
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::AddEntryToALUDelIni(LPTSTR lpszFilePath, LPTSTR lpszFileName)
{
	TCHAR	szValueName[512] = { 0 };
	TCHAR	szTempKey[512] = { 0 };
	TCHAR	szIniFilePath[512] = { 0 };
	TCHAR   szDuplicateString[512] = { 0 };
	TCHAR   szNullString[512] = { 0 };
	TCHAR   szCurrentFileName[512] = { 0 };
	DWORD   result = 0x01;
	DWORD	dwCount = 0x00;
	DWORD   dwDBCount = 0x00;
	bool	mFlag = false;

	try
	{
		if (lpszFileName != NULL)
		{
			_tcscpy(szCurrentFileName, lpszFileName);
		}
		swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\ALUDel.ini", m_szAllUserPath, m_csAppFolderName);
		dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szIniFilePath);

		//Considering DB count Neha Gharge 18-2-2015 
		dwDBCount = GetPrivateProfileInt(L"DB", L"Count", 0x00, szIniFilePath);

		if (dwCount == 0)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", ++dwCount);
			WritePrivateProfileString(L"Files", szValueName, lpszFilePath, szIniFilePath);
		}
		else
		{
			for (; result <= dwCount; result++)
			{
				ZeroMemory(szTempKey, sizeof(szTempKey));
				swprintf_s(szTempKey, _countof(szTempKey), L"%lu", result);

				{
					GetPrivateProfileString(L"Files", szTempKey, L"", szDuplicateString, 511, szIniFilePath);
				}
				CString csFilePath = (CString)lpszFilePath;
				CString csDuplicateString = (CString)szDuplicateString;
				if (csFilePath.Compare(csDuplicateString) == 0)
				{
					AddLogEntry(L"### File Entry already present in ALUDel.ini", 0, 0, true, ZEROLEVEL);
					mFlag = false;
					break;
				}
				else
				{
					mFlag = true;
				}
			}
			if (mFlag)
			{
				{
					ZeroMemory(szValueName, sizeof(szValueName));
					swprintf_s(szValueName, _countof(szValueName), L"%lu", ++dwCount);
					WritePrivateProfileString(L"Files", szValueName, lpszFilePath, szIniFilePath);
					mFlag = false;
				}
			}
		}

		if (dwCount)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", dwCount);
			WritePrivateProfileString(L"Count", L"Count", szValueName, szIniFilePath);
		}

		//Neha Gharge 18-2-2015 if only DB file get downloaded.and dwcount == 0.
		if (dwDBCount)
		{
			ZeroMemory(szValueName, sizeof(szValueName));
			swprintf_s(szValueName, _countof(szValueName), L"%lu", dwDBCount);
			WritePrivateProfileString(L"DB", L"Count", szValueName, szIniFilePath);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### CVibraniumUpdateManager::AddEntryToALUDelIni::Exception for file(%s)", lpszFilePath, 0, true, SECONDLEVEL);
	}

	return false;
}

/***************************************************************************************************
*  Function Name  : DeleteAllDownloadedFiles()
*  Description    : Delete all downloaded files frm program data.
*  Author Name    : Vilas, neha
*  SR_NO		  : WRDWIZALUSRV_0047
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::DeleteAllDownloadedFiles()
{
	AddLogEntry(L">>> In DeleteAllDownloadedFiles", 0, 0, true, ZEROLEVEL);

	DWORD	dwCount = 0x00, i = 0x00;

	TCHAR	szZipFilePath[512] = { 0 };
	TCHAR	szZipUnzipPath[512] = { 0 };

	try
	{
		dwCount = static_cast<DWORD>(vALUFileInfo.size());
		if (!dwCount)
			return false;

		for (; i<dwCount; i++)
		{
			ZeroMemory(szZipFilePath, sizeof(szZipFilePath));
			ZeroMemory(szZipUnzipPath, sizeof(szZipUnzipPath));

			swprintf_s(szZipFilePath, _countof(szZipFilePath), L"%s\\%s\\%s.zip", m_szAllUserPath, m_csAppFolderName, vALUFileInfo[i].szFileName);
			swprintf_s(szZipUnzipPath, _countof(szZipUnzipPath), L"%s\\%s\\%s_%s", m_szAllUserPath, m_csAppFolderName, vALUFileInfo[i].szFileName, L"WWIZALT");

			AddLogEntry(L">>> Delete File: %s", szZipFilePath, 0, true, FIRSTLEVEL);
			SetFileAttributes(szZipFilePath, FILE_ATTRIBUTE_NORMAL);
			if (DeleteFile(szZipFilePath) == FALSE)
			{
				AddLogEntry(L">>> OnRestart Delete File: %s", szZipFilePath, 0, true, FIRSTLEVEL);
			}

			AddLogEntry(L">>> Delete File: %s", szZipUnzipPath, 0, true, FIRSTLEVEL);
			SetFileAttributes(szZipUnzipPath, FILE_ATTRIBUTE_NORMAL);
			if (DeleteFile(szZipUnzipPath) == FALSE)
			{
				AddLogEntry(L">>> OnRestart Delete File: %s", szZipUnzipPath, 0, true, FIRSTLEVEL);
			}

			if (PathFileExists(szZipFilePath))
			{
				Sleep(1);
				SetFileAttributes(szZipFilePath, FILE_ATTRIBUTE_NORMAL);
				DeleteFile(szZipFilePath);
			}

			if (PathFileExists(szZipUnzipPath))
			{
				Sleep(1);
				SetFileAttributes(szZipUnzipPath, FILE_ATTRIBUTE_NORMAL);
				DeleteFile(szZipUnzipPath);
			}

			if (PathFileExists(szZipFilePath))
				MoveFileEx(szZipFilePath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

			if (PathFileExists(szZipUnzipPath))
				MoveFileEx(szZipUnzipPath, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### CVibraniumUpdateManager::DeleteAllDownloadedFiles::Exception for delete operation", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : DeleteAllRenamedFiles()
*  Description    : If all files cpoied successfully, old files get deleted and new files get
replaced. All renamed file get replaced with original name
*  Author Name    : Vilas , Neha
*  SR_NO		  : WRDWIZALUSRV_0038
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::DeleteAllRenamedFilesSEH()
{
	DWORD i = 0x00, dwCount = 0x00;

	TCHAR	szIniFilePath[512] = { 0 };
	TCHAR	szValueData[512] = { 0 };
	TCHAR	szValueName[256] = { 0 };
	TCHAR	szFailedReplacedFile[512] = { 0 };
	TCHAR   szTemp[512] = { 0 };

	std::vector<CString	>vFailedtoReplacedFiles;

	swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\ALUDel.ini", m_szAllUserPath, m_csAppFolderName);


	if (!PathFileExists(szIniFilePath))
	{
		AddLogEntry(L"### File not found : %s in DeleteAllRenamedFiles ", szIniFilePath, 0, true, ZEROLEVEL);
		//Issue : Database version / Product version should not change when product ask for restart, it should be update after restart
		//Resolved By :  Nitin K.
		m_bIsALUDeleted = TRUE;
	}

	dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szIniFilePath);

	CString csErrorDeleteAllRenamedFiles;
	csErrorDeleteAllRenamedFiles.Format(L"%d", dwCount);
	AddLogEntry(L"### In Function DeleteAllRenamedFiles (%s)", csErrorDeleteAllRenamedFiles, 0, true, 0x00);

	if (m_bRestartFlag)
	{
		ZeroMemory(szValueName, sizeof(szValueName));
		WritePrivateProfileString(L"DB", NULL, NULL, szIniFilePath);
		m_bRestartFlag = false;
	}

	if (dwCount)
	{
		for (int i = 1; i <= static_cast<int>(dwCount); i++)
		{
			swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
			GetPrivateProfileString(L"Files", szValueName, L"", szValueData, 511, szIniFilePath);
			if (!szValueData[0])
			{
				//AddLogEntry(L"### Function is  DeleteAllRenamedFiles....WardWizParseIniFile::Invalid Entries for(%s) in (%s)", szValueName, szIniFilePath, true, 0x00);
				break;
			}
			else
			{
				vReplacedFiles.push_back(szValueData);
			}
		}
	}
	dwCount = 0x00;
	vFailedtoReplacedFiles.clear();

	CString csNewFileNamePath, csOldFileNamePath, csServiceName;
	int iPos = 0;
	for (int i = 0; i < static_cast<int>(vReplacedFiles.size()); i++)
	{
		csNewFileNamePath = vReplacedFiles.at(i);
		_tcscpy(szTemp, csNewFileNamePath);
		if (CheckForService(szTemp, csServiceName))
		{
			CString csFilePath = szTemp;
			csFilePath = csFilePath.Left(csFilePath.ReverseFind(L'\\') + 1);

			swprintf_s(szTemp, _countof(szTemp), L"%s%s_%s", csFilePath, csServiceName, L"WWIZALTSRV");
			csOldFileNamePath.Format(L"%s", szTemp);
			if (PathFileExists(csOldFileNamePath))
			{
				SetFileAttributes(csOldFileNamePath, FILE_ATTRIBUTE_NORMAL);
				if (!DeleteFile(csOldFileNamePath))
				{
					AddLogEntry(L"### Failed to delete %s", csOldFileNamePath, 0, true, SECONDLEVEL);
					vFailedtoReplacedFiles.push_back(szTemp);
					++dwCount;
					continue;
				}
				else
				{
					continue;
				}
			}
			{
				swprintf_s(szTemp, _countof(szTemp), L"%s%s_%s", csFilePath, csServiceName, L"WWIZALTDEL");
				csOldFileNamePath.Format(L"%s", szTemp);
				if (PathFileExists(csOldFileNamePath))
				{
					SetFileAttributes(csOldFileNamePath, FILE_ATTRIBUTE_NORMAL);
					if (!DeleteFile(csOldFileNamePath))
					{
						AddLogEntry(L"### Failed to delete %s", csOldFileNamePath, 0, true, SECONDLEVEL);
						vFailedtoReplacedFiles.push_back(szTemp);
						++dwCount;
						continue;
					}
					else
					{
						continue;
					}
				}
			}
		}
		else
		{
			iPos = 0;
			csOldFileNamePath = csNewFileNamePath.Left(csNewFileNamePath.ReverseFind(L'_'));
		}

		iPos = 0;
		if (!PathFileExists(csNewFileNamePath))
		{
			AddLogEntry(L">>> new file %s exist", csNewFileNamePath, 0, true, ZEROLEVEL);
			break;
		}

		if (PathFileExists(csOldFileNamePath))
		{
			AddLogEntry(L">>> old file %s exist", csOldFileNamePath, 0, true, ZEROLEVEL);
			SetFileAttributes(csOldFileNamePath, FILE_ATTRIBUTE_NORMAL);
			if (DeleteFile(csOldFileNamePath))
			{
				_wrename(csNewFileNamePath, csOldFileNamePath);
				continue;
			}
			Sleep(10);

			SetFileAttributes(csOldFileNamePath, FILE_ATTRIBUTE_NORMAL);
			if (DeleteFile(csOldFileNamePath))
			{
				_wrename(csNewFileNamePath, csOldFileNamePath);
				continue;
			}
			vFailedtoReplacedFiles.push_back(csNewFileNamePath);
			++dwCount;
		}
		else
		{
			//In the case when any file is not exist in old setup and we are adding in new update
			//at that time is never get replaced.
			//1.9.0.0 setup we always create the file but in 1.9.4.0 onward we not create file even not get replaced.
			AddLogEntry(L">>> old file %s file is not available.", csOldFileNamePath, 0, true, ZEROLEVEL);
			_wrename(csNewFileNamePath, csOldFileNamePath);
			continue;
		}
	}

	//Neha Gharge 18-2-2015 if only DB file get downloaded.and dwcount == 0. file should not get delete when only DB entry is thr.
	DWORD	dwDBCount = 0x00;
	dwDBCount = GetPrivateProfileInt(L"DB", L"Count", 0, szIniFilePath);

	if (dwCount)
	{
		WritePrivateProfileString(L"Files", NULL, NULL, szIniFilePath);
		WritePrivateProfileString(L"Count", NULL, NULL, szIniFilePath);

		for (int index = 0; index < static_cast<int>(vFailedtoReplacedFiles.size()); index++)
		{
			swprintf_s(szFailedReplacedFile, _countof(szFailedReplacedFile), L"%s", vFailedtoReplacedFiles.at(index));
			AddEntryToALUDelIni(szFailedReplacedFile);
			_tcscpy(szFailedReplacedFile, L"");
		}
	}
	else
	{
		if (!dwDBCount)
		{
			if (PathFileExists(szIniFilePath))
			{
				SetFileAttributes(szIniFilePath, FILE_ATTRIBUTE_NORMAL);
				if (!DeleteFile(szIniFilePath))
				{
					AddLogEntry(L"### Failed to delete file: %s  ", szIniFilePath, 0, true, SECONDLEVEL);
				}
			}
		}
		else
		{
			//Neha Gharge 18-2-2015 if only DB file get downloaded.and dwcount == 0. file should not get delete when only DB entry is thr.
			//But files and count section should be delete as no entry is thr for replacement.
			WritePrivateProfileString(L"Files", NULL, NULL, szIniFilePath);
			WritePrivateProfileString(L"Count", NULL, NULL, szIniFilePath);
		}
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : CheckForService()
*  Description    : It will return name of service
*  Author Name    : Vilas , Neha
*  SR_NO		  : WRDWIZALUSRV_0070
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::CheckForService(TCHAR *szFilePathFromVector, CString &MatchedServiceName)
{
	for (int j = 0; j < static_cast<int>(m_vServiceNameVector.size()); j++)
	{
		if (wcsstr(szFilePathFromVector, m_vServiceNameVector.at(j)))
		{
			MatchedServiceName = g_csServiceNameArray[j];
			return true;
		}
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : DeleteAllRenamedFiles()
*  Description    : It will delete all renamed files _WWIZALTSRV
*  Author Name    : Vilas , Neha
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
****************************************************************************************************/
bool CWardWizUpdateManager::DeleteAllRenamedFiles()
{
	bool bReturn = false;

	__try
	{
		bReturn = DeleteAllRenamedFilesSEH();
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### CVibraniumUpdateManager::DeleteAllRenamedFiles::Exception", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/***************************************************************************************************
*  Function Name  : ReplaceAllOriginalFiles()
*  Description    : If all files not copied properly, it will roll back all files to its old
files.
*  Author Name    : Vilas, neha
**  SR_NO		  : WRDWIZALUSRV_0045
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::ReplaceAllOriginalFiles()
{
	DWORD	dwCount = 0x00, i = 0x00;

	TCHAR	szSourceFilePath[512] = { 0 };
	TCHAR	szDestFilePath[512] = { 0 };
	TCHAR	szTemp[512] = { 0 };
	TCHAR   szValueName[512] = { 0 };
	TCHAR   szValueData[512] = { 0 };
	TCHAR   szIniFilePath[512] = { 0 };
	TCHAR   szSrvOriginalFilePath[512] = { 0 };
	CString csServiceName;

	TCHAR	*pTemp = NULL;

	bool	bCopied = true;

	try
	{
		swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\ALUDel.ini", m_szAllUserPath, m_csAppFolderName);


		if (!PathFileExists(szIniFilePath))
		{
			//AddLogEntry(L"### ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, SECONDLEVEL);
			// divya
			/* Issue Number 159
			Failed to update product. Please check internet connection”
			even if internet connection is there
			*/
			AddLogEntry(L"### Function is ReplaceAllOriginalFiles..... ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, ZEROLEVEL);
			//return false;
		}
		dwCount = GetPrivateProfileInt(L"Count", L"Count", 0x00, szIniFilePath);

		if (dwCount)
		{
			for (int i = 1; i <= static_cast<int>(dwCount); i++)
			{
				swprintf_s(szValueName, _countof(szValueName), L"%lu", i);
				GetPrivateProfileString(L"Files", szValueName, L"", szValueData, 511, szIniFilePath);
				if (!szValueData[0])
				{
					AddLogEntry(L"### VibraniumParseIniFile::Invalid Entries for(%s) in (%s)", szValueName, szIniFilePath, true, ZEROLEVEL);
					break;
				}
				else
				{
					vReplacedFiles.push_back(szValueData);
				}
			}
		}
		dwCount = 0x00;


		dwCount = static_cast<DWORD>(vReplacedFiles.size());
		if (dwCount == 0x00)
			return false;

		bCopied = true;
		for (i = 0; i<dwCount; i++)
		{
			ZeroMemory(szSourceFilePath, sizeof(szSourceFilePath));
			swprintf_s(szSourceFilePath, _countof(szSourceFilePath), L"%s", vReplacedFiles.at(i));

			if (CheckForService(szSourceFilePath, csServiceName))
			{

				swprintf_s(szSourceFilePath, _countof(szSourceFilePath), L"%s%s", theApp.m_szAVPath, csServiceName);
				swprintf_s(szTemp, _countof(szTemp), L"%s%s_%s", theApp.m_szAVPath, csServiceName, L"WWIZALTSRV");
				if (PathFileExists(szSourceFilePath))
				{
					SetFileAttributes(szSourceFilePath, FILE_ATTRIBUTE_NORMAL);
					DeleteFile(szSourceFilePath);
				}
				_wrename(szTemp, szSourceFilePath);
				continue;
			}

			else
			{
				if (PathFileExists(szSourceFilePath))
				{
					AddLogEntry(L">>> delete replaced file %s from destination", szSourceFilePath, 0, true, ZEROLEVEL);
					SetFileAttributes(szSourceFilePath, FILE_ATTRIBUTE_NORMAL);
					DeleteFile(szSourceFilePath);
					bCopied = false;
					continue;
				}
			}

		}

		if (bCopied)
			AddLogEntry(L"### ReplaceAllOriginalFiles::Copy to original file(%s) failed", szSourceFilePath);
	}
	catch (...)
	{
		AddLogEntry(L"### CVibraniumUpdateManager::CreateDirectoryFocefully::Exception", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : CheckDestFolderAndCreate()
*  Description    : It will check all prodution folder. If not present, it will create that folder.
*  Author Name    : Vilas
*  SR_NO		  : WRDWIZALUSRV_0043
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
****************************************************************************************************/
bool CWardWizUpdateManager::CheckDestFolderAndCreate(LPTSTR lpszShortPath)
{

	bool bCreate = false;

	TCHAR	*pTemp = NULL;

	try
	{
		pTemp = wcschr(lpszShortPath, '\\');

		if (!pTemp)
			return bCreate;

		TCHAR	szTemp[256] = { 0 };

		pTemp++;
		wcscpy_s(szTemp, 255, pTemp);

		TCHAR	szTokens[128] = { 0 };
		TCHAR	szPath[512] = { 0 };

		TCHAR	szToken[] = L"\\";
		TCHAR	*pToken = NULL;
		TCHAR	*pTokenNext = NULL;

		pToken = wcstok_s(szTemp, szToken, &pTokenNext);

		do
		{
			if (!pToken)
				break;

			if (!wcslen(pToken))
				break;

			wcscat_s(szTokens, 128, pToken);
			swprintf_s(szPath, 511, L"%s%s", theApp.m_szAVPath, szTokens);

			if (!PathFileExists(szPath))
			{
				bCreate = CreateDirectoryFocefully(szPath);
				if (bCreate)
					break;
			}

			wcscat_s(szTokens, 128, L"\\");

			pToken = wcstok_s(NULL, szToken, &pTokenNext);

		} while (1);
	}
	catch (...)
	{
		AddLogEntry(L"### CVibraniumUpdateManager::CheckDestFolderAndCreate::Exception", 0, 0, true, SECONDLEVEL);
	}

	return bCreate;
}

/***************************************************************************************************
*  Function Name  : CreateDirectoryFocefully()
*  Description    : It will check directory. If not present, it will create that directory.
*  Author Name    : Vilas
*  SR_NO		  : WRDWIZALUSRV_0044
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
****************************************************************************************************/
bool CWardWizUpdateManager::CreateDirectoryFocefully(LPTSTR lpszPath)
{
	try
	{
		CreateDirectory(lpszPath, NULL);
		if (PathFileExists(lpszPath))
			return false;

		_wmkdir(lpszPath);
		if (PathFileExists(lpszPath))
			return false;
	}
	catch (...)
	{
		AddLogEntry(L"### CVibraniumUpdateManager::CreateDirectoryFocefully::Exception", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : CreateDirectoryFocefully4EPSHierarchy()
*  Description    : It will check directory. If not present, it will create that directory.
*  Author Name    : Amol Jaware
*  SR_NO		  :
*  Date			  :	11/April/2018
****************************************************************************************************/
bool CWardWizUpdateManager::CreateDirectoryFocefully4EPSHierarchy(LPTSTR lpszPath)
{
	try
	{
		CreateDirectory(lpszPath, NULL);
		if (PathFileExists(lpszPath))
			return false;

		_wmkdir(lpszPath);
		if (PathFileExists(lpszPath))
			return false;
	}
	catch (...)
	{
		AddLogEntry(L"### CVibraniumUpdateManager::CreateDirectoryFocefully4EPSHierarchy::Exception", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : CheckForServiceName
*  Description    : If service is replcing, It will check name of service
*  Author Name    : Vilas , Neha
*  SR_NO		  : WRDWIZALUSRV_0069
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::CheckForServiceName(int iIndex)
{
	for (int j = 0; j < static_cast<int>(m_vServiceNameVector.size()); j++)
	{
		if (_tcscmp((vALUFileInfo[iIndex].szFileName), m_vServiceNameVector.at(j)) == 0)
		{
			return true;
		}
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	SendData2EPSClient
*  Description    :	Function which sends update status to wardwiz EPS client
*  Author Name    : Ram Shelke
*  Date           : 27th March 2018
*  SR_NO		  :
**********************************************************************************************************/
bool CWardWizUpdateManager::SendUpdateFinishedData2EPSClient(DWORD dwUpdtStatus)
{
	try
	{
		if (!m_pfnDownloadStatusCallback)
			return false;

		m_pfnDownloadStatusCallback(UPDATE_FINISHED, 1, 0, dwUpdtStatus, this);

		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::SendUpdateFinishedData2EPSClient", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : UpdateTimeDate()
*  Description    : Update date and time
*  Author Name    : Vilas , Neha
**  SR_NO		  : WRDWIZALUSRV_0079
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::UpdateTimeDate()
{
	DWORD dwSuccess = 0x00;
	CString  csDate, csTime;
	TCHAR szOutMessage[30] = { 0 };
	TCHAR tbuffer[16] = { 0 };
	TCHAR dbuffer[16] = { 0 };
	/*Rajil Yadav Issue no. 598, 6/11/2014*/
	//SYSTEMTIME  CurrTime = {0} ;
	//GetLocalTime( &CurrTime ) ;
	//CTime Time_Curr( CurrTime ) ;
	//Neha Gharge 20-1-2015 Need a local time not a UTC time to compare update older than 7 days.
	CTime Time_Curr = CTime::GetCurrentTime();
	int iMonth = Time_Curr.GetMonth();
	int iDate = Time_Curr.GetDay();
	int iYear = Time_Curr.GetYear();

	_wstrtime_s(tbuffer, 15);
	csTime.Format(L"%s\0\r\n", tbuffer);
	csDate.Format(L"%d/%d/%d", iMonth, iDate, iYear);
	_stprintf(szOutMessage, _T("%s %s\0"), csDate, tbuffer);

	if (m_dwIsUptoDateCompleted != ALUPDATED_UPTODATE) //1 means ALUPDATED_UPTODATE
	{
		CITinRegWrapper objReg;
		dwSuccess = objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"LastLiveupdatedt", szOutMessage);
		if (dwSuccess)
		{
			AddLogEntry(L"###Failed to Setregistryvaluedata for LastLiveupdatedt", 0, 0, true, SECONDLEVEL);
			//return false;
		}
		dwSuccess = objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"LastLiveupdatetm", tbuffer);
		if (dwSuccess)
		{
			AddLogEntry(L"###Failed to Setregistryvaluedata for LastLiveupdatetm", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : CheckInternetConnection()
*  Description    : It checks internet connection .
*  Author Name    : Vilas, neha
*  SR_NO		  : WRDWIZALUSRV_0053
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::CheckInternetConnection()
{
	bool bReturn = false;
	try
	{
		CWWizHttpManager objWinHttpManager;
		TCHAR szTestUrl[MAX_PATH] = { 0 };
		_tcscpy_s(szTestUrl, MAX_PATH, _T("https://www.google.com"));
		if (objWinHttpManager.Initialize(szTestUrl))
		{
			if (objWinHttpManager.CreateRequestHandle(NULL))
			{
				return true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::CheckInternetConnection", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : DownLoadLatestFilesFromServer()
*  Description    : Download mismatched files from server
*  Author Name    : Vilas , Neha
*  SR_NO		  : WRDWIZALUSRV_0034
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::DownLoadLatestFilesFromServer(DWORD& dwStatus)
{
	//Please add code for download all files here
	bool bReturn = false;
	try
	{
		AddLogEntry(L">>> DownLoadLatestFilesFromServer::StartDownloading", 0, 0, true, FIRSTLEVEL);
		m_objWinHttpManager.StartCurrentDownload();

		if (!MakeDownloadUrlList())
		{
			AddLogEntry(L"### Failed to MakeDownloadUrlList", 0, 0, true, SECONDLEVEL);
		}

		if (!hTotalFileSizeThread)
		{
			hTotalFileSizeThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)GetTotalFileSizeThread,
				this, 0, 0);
		}

		m_objWinHttpManager.SetDownloadCompletedBytes(0);

		CString csFileName;
		bool bDownloadSuccess = false;
		bool bWaitTimeout = false;
		bool bDownloadFailed = false;

		int TotalDownloadedFilesNo = static_cast<int>(m_vUrlLists.size());
		unsigned int iItemCount = 0, iItemUpdatedAlready = 0x00;
		for (; iItemCount < m_vUrlLists.size(); iItemCount++)
		{

			csFileName = m_vUrlLists[iItemCount];

			if (!csFileName.GetLength())
				continue;

			if (IsLatestFile(csFileName))
			{
				SetDownloadPercentage(SETMESSAGE, ALREADYDOWNLOADED, (iItemCount + 1), TotalDownloadedFilesNo, this);
				//SendUpdateInfoToGUI(SETMESSAGE, L"Already downloaded", L"", (iItemCount + 1), TotalDownloadedFilesNo);
				iItemUpdatedAlready++;
				continue;
			}

			SetDownloadPercentage(SETMESSAGE, DOWNLOADINGFILES, (iItemCount + 1), TotalDownloadedFilesNo, this);
			//SendUpdateInfoToGUI(SETMESSAGE, L"Downloading files", L"", (iItemCount + 1), TotalDownloadedFilesNo);
			if (bWaitTimeout)
			{
				continue;
			}

			TCHAR	szUpdateServerCount[10] = { 0 };
			CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\UpdateServers.ini";
			GetPrivateProfileString(L"UPDATESERVERS", L"Count", L"", szUpdateServerCount, 511, csIniFilePath);
		
			CString csUpdateServerCount = (LPCTSTR)szUpdateServerCount;
			m_iServerCount = _ttoi(csUpdateServerCount);

			if (!m_iServerCount)
			{
				DWORD dwRetryCount = 0;
				while (dwRetryCount < MAX_UPDATE_RETRYCOUNT)
				{
					m_objWinHttpManager.m_bIsConnected = true;
					bDownloadFailed = false;
					bWaitTimeout = false;

					if (!StartDownloadFile(csFileName))
					{
						if (!CheckInternetConnection())
						{
							if (!WaitForInternetConnection())
							{
								bWaitTimeout = true;
								AddLogEntry(L">>> Retry to download file : %s", csFileName, 0, true, FIRSTLEVEL);
								//break;
							}
						}
						else
						{
							bDownloadFailed = true;
							AddLogEntry(L">>> Retry to download file : %s", csFileName, 0, true, FIRSTLEVEL);
							//break;
						}
					}
					else
					{
						bDownloadSuccess = true;
						break;
					}
					dwRetryCount++;
				}
			}
			else
			{
				for (int i = 0; i < m_iServerCount; i++)
				{
					CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\UpdateServers.ini";
					CString csCurrVal;
					TCHAR	szUpdateServerName[255] = { 0 };
					csCurrVal.Format(L"%d", i + 1);
					GetPrivateProfileString(L"UPDATESERVERS", csCurrVal, L"", szUpdateServerName, 511, csIniFilePath);
					g_csServerName = szUpdateServerName;
					DWORD dwRetryCount = 0;
					while (dwRetryCount < MAX_UPDATE_RETRYCOUNT)
					{
						m_objWinHttpManager.m_bIsConnected = true;
						bDownloadFailed = false;
						bWaitTimeout = false;

						if (!StartDownloadFile(csFileName))
						{
							if (!CheckInternetConnection())
							{
								if (!WaitForInternetConnection())
								{
									bWaitTimeout = true;
									AddLogEntry(L">>> Retry to download file : %s", csFileName, 0, true, FIRSTLEVEL);
									//break;
								}
							}
							else
							{
								bDownloadFailed = true;
								AddLogEntry(L">>> Retry to download file : %s", csFileName, 0, true, FIRSTLEVEL);
								//break;
							}
						}
						else
						{
							bDownloadSuccess = true;
							break;
						}
						dwRetryCount++;
					}
					if (bDownloadSuccess == true)
					{
						break;
					}
				}
			}
		}

		if (iItemUpdatedAlready)
		{
			if (iItemUpdatedAlready == m_vUrlLists.size())
				dwStatus = 0x01;
		}

		//Sleep needed for timer to finish
		//Sleep(4000);
		Sleep(1000);//No need to have 4 seconds sleep.

		if (bWaitTimeout || bDownloadFailed)
		{
			//set as failed
			AddLogEntry(L"### bWaitTimeout or bDownloadFailed flag are true File : %s", csFileName, 0, true, SECONDLEVEL);
			return bReturn;
		}

		if (m_OSType != WINOS_XP && m_OSType != WINOS_XP64)
		{
			g_objcriticalSection.Unlock();
		}

		m_objWinHttpManager.StopCurrentDownload();

		bReturn = true;
	}
	catch (...)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in DownLoadLatestFilesFromServer", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : IsLatestFile()
*  Description    : Is file latest
*  Author Name    : Vilas , Neha
**  SR_NO		  : WRDWIZALUSRV_0075
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::IsLatestFile(const CString& csFilePath)
{
	int iFound = csFilePath.ReverseFind(L'/');

	if (iFound == -1)
		return false;

	CString csFileName = csFilePath.Right(csFilePath.GetLength() - iFound - 1);

	if (csFileName.GetLength() < 4)
		return false;

	csFileName = csFileName.Mid(0, csFileName.GetLength() - 4);

	bool bFound = true;
	DWORD i = 0;

	for (; i<vALUFileInfo.size(); i++)
	{
		if (csFileName.CompareNoCase(vALUFileInfo[i].szFileName) == 0)
		{
			bFound = false;
			break;
		}
	}

	if (bFound)
		return false;

	TCHAR	szFullFilePath[512] = { 0 };

	GetFilePathFromshortPath(szFullFilePath, vALUFileInfo[i].szFileShortPath, vALUFileInfo[i].szFileName);

	TCHAR csFileALTPath[512] = { 0 };
	TCHAR	szFileHash[512] = { 0 };
	DWORD	dwFileSize = 0x00;

	swprintf_s(csFileALTPath, 511, L"%s_WWIZALTSRV", szFullFilePath);

	if (PathFileExists(csFileALTPath))
	{
		GetFileSizeAndHash(csFileALTPath, dwFileSize, szFileHash);
		if (dwFileSize == vALUFileInfo[i].dwFullSize)
		{
			if (_wcsicmp(szFileHash, vALUFileInfo[i].szFileHash) == 0)
			{
				vALUFileInfo.erase(vALUFileInfo.begin() + i);
				return true;
			}
		}
	}

	ZeroMemory(csFileALTPath, sizeof(csFileALTPath));
	swprintf_s(csFileALTPath, 511, L"%s_WWIZALT", szFullFilePath);

	if (PathFileExists(csFileALTPath))
	{
		dwFileSize = 0x00;
		ZeroMemory(szFileHash, sizeof(szFileHash));

		GetFileSizeAndHash(csFileALTPath, dwFileSize, szFileHash);
		if (dwFileSize == vALUFileInfo[i].dwFullSize)
		{
			if (_wcsicmp(szFileHash, vALUFileInfo[i].szFileHash) == 0)
			{
				vALUFileInfo.erase(vALUFileInfo.begin() + i);
				return true;
			}
		}
	}

	return false;
}

/***************************************************************************************************
*  Function Name  : GetFilePathFromshortPath()
*  Description    : Get FilePath From shortPath
*  Author Name    : Vilas , Neha
**  SR_NO		  : WRDWIZALUSRV_0077
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::GetFilePathFromshortPath(LPTSTR lpszFilePath, LPTSTR lpszShortPath, LPTSTR lpszFileName)
{

	TCHAR	*pTemp = wcschr(lpszShortPath, '\\');

	if (pTemp)
	{
		pTemp++;
		swprintf_s(lpszFilePath, 511, L"%s%s\\%s", theApp.m_szAVPath, pTemp, lpszFileName);
	}
	else
		swprintf_s(lpszFilePath, 511, L"%s%s", theApp.m_szAVPath, lpszFileName);

	return false;
}

/***************************************************************************************************
*  Function Name  : GetFileSizeAndHash()
*  Description    : Get FileSize And Hash
*  Author Name    : Vilas , Neha
**  SR_NO		  : WRDWIZALUSRV_0076
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::GetFileSizeAndHash(TCHAR *pFilePath, DWORD &dwFileSize, TCHAR *pFileHash)
{
	try
	{
		HANDLE hFile = CreateFile(pFilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			AddLogEntry(L"### GetFileSizeAndHash : Error in opening file %s", pFilePath, 0, true, SECONDLEVEL);
			return true;
		}

		dwFileSize = GetFileSize(hFile, NULL);
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;

		if (pFileHash)
		{
			if (!theApp.GetFileHash(pFilePath, pFileHash))
				return false;
			else
			{
				AddLogEntry(L"### CVibraniumUpdateManager Exception in CVibraniumALUpdDlg::IsWow64", 0, 0, true, SECONDLEVEL);
				return true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### CVibraniumUpdateManager Exception in CVibraniumUpdateManager::GetFileSizeAndHash", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : MakeDownloadUrlList()
*  Description    : Make url of that files which new/ modified at server side .
*  Author Name    : Neha
**  SR_NO		  : WRDWIZALUSRV_0048
*  Date           :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::MakeDownloadUrlList()
{
	try
	{
		DWORD dwCount = 0x00, i = 0x00;
		DWORD	dwProductID = 0x00;
		TCHAR	szZipFilePath[512] = { 0 };
		CString csurl, csProductID;

		//TCHAR szFileName[MAX_PATH];
		m_vUrlLists.clear();

		dwCount = static_cast<DWORD>(vALUFileInfo.size());

		TCHAR	szDomainName[MAX_PATH] = { 0 };
		if (!GetDomainName(szDomainName, MAX_PATH))
		{
			AddLogEntry(L"### Failed to get GetDomainName in MakeDownloadUrlList", 0, 0, true, SECONDLEVEL);
			return true;
		}

		GetProductID(dwProductID);

		if (!dwProductID)
		{
			AddLogEntry(L"### Product ID mismatched", 0, 0, true, SECONDLEVEL);
			return true;
		}

		//Issue Resolved about Separate patches for each product
		//This check is required was because the scenario, we have released for Essentials, and files are added
		//in 32, 64 and common folder, if user was not updated (as sync with server in version) then files will get downloaded on his machine
		//and if he tries to update again, files will get mismatch and again product updates will start.
		//Now with this change online patches are separated for each version.
		CString csProductName;
		switch (dwProductID)
		{
		case 1:
			csProductName = L"Essential";
			break;
		case 2:
			csProductName = L"Pro";
			break;
		case 3:
			csProductName = L"Elite";
			break;
		case 4:
			csProductName = L"Basic";
			break;
		case 5:
			csProductName = L"EssentialPlus";
			break;
		}

		for (; i < dwCount; i++)
		{
			_tcscpy(szZipFilePath, L"");
			swprintf_s(szZipFilePath, _countof(szZipFilePath), L"%s/%s.zip", vALUFileInfo[i].szServerLoc, vALUFileInfo[i].szFileName);
			csurl.Format(L"http://%s/%s/%s", szDomainName, szUpdateFolderName, szZipFilePath);
			csurl.Format(L"http://%s/%s/%s/%s", szDomainName, szUpdateFolderName, csProductName, szZipFilePath);
			m_vUrlLists.push_back(csurl);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::MakeDownloadUrlList", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : StartDownloadFile()
*  Description    : Start downloading files on by one .
*  Author Name    : Vilas, neha
*  SR_NO		  : WRDWIZALUSRV_0052
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::StartDownloadFile(LPCTSTR szUrlPath)
{
	try
	{
		if (!szUrlPath)
			return false;

		CString csFileName(szUrlPath);
		int iFound = csFileName.ReverseFind(L'/');
		csFileName = csFileName.Right(csFileName.GetLength() - iFound - 1);

		STRUCT_DOWNLOAD_INFO sDownloadInfo = { 0 };
		_tcscpy_s(sDownloadInfo.szMainUrl, URL_SIZE, szUrlPath);
		_tcscpy_s(sDownloadInfo.szSectionName, csFileName);
		_tcscpy_s(sDownloadInfo.szExeName, csFileName);

		DWORD dwThreadSize = DEFAULT_DOWNLOAD_THREAD;
		if (theApp.m_bRequestFromUI)
		{
			dwThreadSize = DEFAULT_DOWNLOAD_THREAD + 0x02;
		}

		sDownloadInfo.dwDownloadThreadCount = dwThreadSize;
		_stprintf_s(sDownloadInfo.szLocalTempDownloadPath, MAX_PATH, L"%s\\%s\\", m_szAppDataFolder, m_csAppFolderName);
		_stprintf_s(sDownloadInfo.szLocalPath, MAX_PATH, L"%s\\%s\\", m_szAppDataFolder, m_csAppFolderName);

		TCHAR szTagetPath[MAX_PATH] = { 0 };
		_stprintf_s(szTagetPath, MAX_PATH, L"%s\\%s\\%s", m_szAppDataFolder, m_csAppFolderName, csFileName);

		DWORD dwTotalFileSize = 0x00;
		TCHAR szInfo[MAX_PATH] = { 0 };
		DWORD dwBufLen = MAX_PATH * sizeof(TCHAR);
		CWWizHttpManager objWinHttpManager;
		if (!objWinHttpManager.Initialize(szUrlPath))
		{
			AddLogEntry(L"### Initialize Failed in StartDownloadFile, URLPath: [%s]", szUrlPath, 0, true, SECONDLEVEL);
			return false;
		}

		if (!objWinHttpManager.GetHeaderInfo(WINHTTP_QUERY_CONTENT_LENGTH, szInfo, dwBufLen))
		{
			AddLogEntry(L"### GetHeaderInfo Failed in StartDownloadFile, URLPath: [%s]", szUrlPath, 0, true, SECONDLEVEL);
			return false;
		}
		dwTotalFileSize = _wtol(szInfo);

		DWORD dwStartBytes = 0;
		if (PathFileExists(szTagetPath))
		{
			m_hFile = CreateFile(szTagetPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
			if (m_hFile != INVALID_HANDLE_VALUE)
			{
				dwStartBytes = GetFileSize(m_hFile, 0);
				CloseHandle(m_hFile);
				m_hFile = NULL;
			}

			if (dwStartBytes == dwTotalFileSize)
			{
				if (!CheckForCorruption(szTagetPath))
				{
					m_lTotalDownloadedBytes += dwTotalFileSize;
					m_objGUI.SetDownloadedBytes(dwTotalFileSize, dwTotalFileSize, 0);
					return true;
				}
			}
			SetFileAttributes(szTagetPath, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(szTagetPath);

		}

		if (!m_pDownloadController)
			return false;

		bool bDownloadFailed = false;
		DWORD dwRetryCount = 0x00;
		do
		{
			bDownloadFailed = false;
			m_pDownloadController->ResetInitData();
			if (!m_pDownloadController->StartController(&sDownloadInfo))
			{
				AddLogEntry(L"### Retry to download: FilePath: [%s\\%s]", sDownloadInfo.szLocalTempDownloadPath, sDownloadInfo.szExeName, true, FIRSTLEVEL);
				bDownloadFailed = true;
				dwRetryCount++;
			}

			if (bDownloadFailed)
			{
				//if physics file size is greater than server file size then download from BEGIN
				if (PathFileExists(szTagetPath))
				{
					//Delete the file and download again.
					SetFileAttributes(szTagetPath, FILE_ATTRIBUTE_NORMAL);
					DeleteFile(szTagetPath);
					m_objWinHttpManager.SetDownloadCompletedBytes(0);
					dwStartBytes = 0;
				}

				//Start download for file
				objWinHttpManager.Download(szTagetPath, dwStartBytes, dwTotalFileSize);
				if (PathFileExists(szTagetPath))
				{
					bDownloadFailed = false;
					break;
				}
			}
		} while (bDownloadFailed && dwRetryCount < MAX_RETRY_COUNT);

		if (bDownloadFailed)
		{
			//if physics file size is greater than server file size then download from BEGIN
			if (PathFileExists(szTagetPath))
			{
				//Delete the file and download again.
				SetFileAttributes(szTagetPath, FILE_ATTRIBUTE_NORMAL);
				DeleteFile(szTagetPath);
				m_objWinHttpManager.SetDownloadCompletedBytes(0);
				dwStartBytes = 0;
			}

			//Start download for file
			objWinHttpManager.Download(szTagetPath, dwStartBytes, dwTotalFileSize);
			if (!PathFileExists(szTagetPath))
			{
				AddLogEntry(L"### Failed to download file [%s]", szUrlPath, 0, true, SECONDLEVEL);
				return false;
			}
		}

		AddLogEntry(L">>> File Downloaded: FilePath: [%s\\%s]", sDownloadInfo.szLocalTempDownloadPath, sDownloadInfo.szExeName, true, ZEROLEVEL);
		m_lTotalDownloadedBytes += dwTotalFileSize;

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::StartDownloadFile", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : SetPercentDownload
*  Description    : callback function to set downloaded percentage.
*  Author Name    : Ram Shelke
*  Date			  :	13-Dec-2017
****************************************************************************************************/
void CGUIInterface::SetPercentDownload(int nPercent)
{
}

/***************************************************************************************************
*  Function Name  : SetDownloadedBytes
*  Description    : callback function to set downloaded bytes.
*  Author Name    : Ram Shelke
*  Date			  :	13-Dec-2017
****************************************************************************************************/
void CGUIInterface::SetDownloadedBytes(DWORD dwTotalFileLength, DWORD dwDownloadedBytes, double dTransferRate)
{
	try
	{
		if (g_pThis->m_lAppStopping)
		{
			return;
		}

		long DownloadBytes = g_pThis->m_lTotalDownloadedBytes + dwDownloadedBytes;
		if (g_pThis->m_dwTotalFileSize == 0x00)
		{
			g_pThis->m_dwPercentage = 0x01;
		}
		else
		{
			g_pThis->m_dwPercentage = GetPercentage(DownloadBytes, g_pThis->m_dwTotalFileSize);
		}

		if (g_pThis->m_dwPercentage >= 100)
		{
			g_pThis->m_dwPercentage = 99;
			return;
		}

		DWORD dwPercent = g_pThis->m_dwPercentage;
		if (dwPercent == 0x00)
		{
			return;
		}

		if (g_iPreviousPerc >= (int)dwPercent)
		{
			dwPercent = g_iPreviousPerc;
		}

		if ((DWORD)DownloadBytes > g_pThis->m_dwTotalFileSize)
		{
			DownloadBytes = g_pThis->m_dwTotalFileSize;
		}

		if (dwPercent > 80)
		{
			dwPercent = 80;
		}
		
		g_pThis->SetDownloadPercentage(SETDOWNLOADPERCENTAGE, EMPTYSTRING, DownloadBytes, dwPercent, this);
		//pThis->SendUpdateInfoToGUI(SETDOWNLOADPERCENTAGE, L"", L"", DownloadBytes, dwPercent);

		g_iPreviousPerc = dwPercent;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in SetDownloadedBytes", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : SendUpdateInfoToGUI()
*  Description    : Send all update info to GUI through shared memory
*  Author Name    : Vilas, neha
*  SR_NO		  : WRDWIZALUSRV_0058
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::SendUpdateInfoToGUI(int iMessageInfo, CString csFirstParam, CString csSecondParam, DWORD dwFirstParam, DWORD dwSecondParm)
{

	if (theApp.m_bRequestFromUI)
	{
		AddLogEntry(L">>> Inside CVibraniumUpdateManager::SendUpdateInfoToGUI started %s", csFirstParam, 0, true, ZEROLEVEL);
		if (m_OSType != WINOS_XP && m_OSType != WINOS_XP64)
		{
			g_objcriticalSection.Lock();
		}

		//Send Messagees using pipe.
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMessageInfo;
		_tcscpy(szPipeData.szFirstParam, csFirstParam);
		szPipeData.dwValue = dwFirstParam;
		szPipeData.dwSecondValue = dwSecondParm;

		if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			Sleep(50);
			if (!m_objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				CString csLog;
				csLog.Format(L"iMessageInfo: %d, FirstParam :%s, SeondParam: %s, dwFirstParam: %d, dwSecondParm:%d", iMessageInfo, csFirstParam,
					csSecondParam, dwFirstParam, dwSecondParm);
				AddLogEntry(L"### Failed to send data inSendUpdateInfoToGUI %s", csLog, 0, true, SECONDLEVEL);
				if (m_OSType != WINOS_XP && m_OSType != WINOS_XP64)
				{
					g_objcriticalSection.Unlock();
				}
				return false;
			}
		}

		AddLogEntry(L">>> Inside CVibraniumUpdateManager::SendUpdateInfoToGUI Finished %s", csFirstParam, 0, true, ZEROLEVEL);
		if (m_OSType != WINOS_XP && m_OSType != WINOS_XP64)
		{
			g_objcriticalSection.Unlock();
		}
	}

	AddLogEntry(L">>> Inside CVibraniumUpdateManager::SendUpdateInfoToGUI end %s", csFirstParam, 0, true, ZEROLEVEL);
	return true;
}

/***************************************************************************************************
*  Function Name  : GetPercentage()
*  Description    : Calculating percentage
*  Author Name    : Vilas, neha
*  SR_NO		  : WRDWIZALUSRV_0063
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
DWORD GetPercentage(int iDownloaded, int iTotalSize)
{
	__try
	{
		return static_cast<DWORD>(((static_cast<double>((iDownloaded)) / iTotalSize)) * 80);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::GetPercentage", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : CheckForCorruption
*  Description    : Function which checks for corrupted file.
*  Author Name    : Ram Shelke
*  Date			  :	13-Dec-2017
****************************************************************************************************/
bool CWardWizUpdateManager::CheckForCorruption(LPTSTR szZipFilePath)
{
	try
	{
		if (!szZipFilePath)
			return false;

		if (!PathFileExists(szZipFilePath))
			return false;

		CString csFileName(szZipFilePath);
		int iFound = csFileName.ReverseFind(L'\\');
		csFileName = csFileName.Right(csFileName.GetLength() - iFound - 1);

		TCHAR	szZipUnzipPath[512] = { 0 };
		swprintf_s(szZipUnzipPath, _countof(szZipUnzipPath), L"%s\\%s\\%s_%s", m_szAllUserPath, m_csAppFolderName, csFileName, L"WTEST");

		DWORD dwRet = UnzipUsingZipArchive(szZipFilePath, szZipUnzipPath);

		SetFileAttributes(szZipUnzipPath, FILE_ATTRIBUTE_NORMAL);
		DeleteFile(szZipUnzipPath);

		if (dwRet != 0x00)
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::CheckForCorruption, FilePath: [%s]", szZipFilePath, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : WaitForInternetConnection()
*  Description    : It waits internet connection for 10 min .
*  Author Name    : Vilas, neha
*  SR_NO		  : WRDWIZALUSRV_0054
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::WaitForInternetConnection()
{
	bool bReturn = false;
	int iRetryCount = 0;
	while (true)
	{
		if (!CheckInternetConnection())
		{
			if (iRetryCount > MAX_RETRY_COUNT)
			{
				bReturn = false;
				break;
			}
			iRetryCount++;
			Sleep(10 * 1000);//wait here for 10 seconds
		}
		else
		{
			bReturn = true;
			break;
		}
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : GetTotalFileSizeThread
*  Description    : Thread function to get total file size.
*  Author Name    : Ram Shelke
*  Date           :	14-12-2017
****************************************************************************************************/
DWORD GetTotalFileSizeThread(LPVOID lpParam)
{
	try
	{
		//Get the total downloaded file size here
		CWardWizUpdateManager *pThis = (CWardWizUpdateManager*)lpParam;
		pThis->m_dwTotalFileSize = pThis->GetTotalFilesSize();
		pThis->SetDownloadPercentage(SETTOTALFILESIZE, EMPTYSTRING, pThis->m_dwTotalFileSize, 0, pThis);
		//pThis->SendUpdateInfoToGUI(SETTOTALFILESIZE, EMPTY_STRING, EMPTY_STRING, pThis->m_dwTotalFileSize, 0);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in GetTotalFileSizeThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : GetTotalFilesSize()
*  Description    : Get total downloaded files size byte from server.
*  Author Name    : Vilas, neha
*  SR_NO		  : WRDWIZALUSRV_0050
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
DWORD CWardWizUpdateManager::GetTotalFilesSize()
{
	DWORD dwTotalFilesSize = 0;
	try
	{
		if (m_vUrlLists.size() > 0)
		{
			for (unsigned int iItemCount = 0; iItemCount < m_vUrlLists.size(); iItemCount++)
			{
				CString csItem = m_vUrlLists[iItemCount];
				TCHAR szInfo[MAX_PATH] = { 0 };
				TCHAR szTagetPath[MAX_PATH] = { 0 };
				DWORD dwBufLen = MAX_PATH * sizeof(TCHAR);
				DWORD dwFileSize = 0;
				if (m_objWinHttpManager.Initialize(csItem))
				{
					ZeroMemory(szInfo, sizeof(szInfo));
					m_objWinHttpManager.GetHeaderInfo(WINHTTP_QUERY_CONTENT_LENGTH, szInfo, dwBufLen);
					dwFileSize = 0x00;
					swscanf(szInfo, L"%lu", &dwFileSize);
					dwTotalFilesSize += dwFileSize;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CVibraniumUpdateManager::GetTotalFilesSize", 0, 0, true, SECONDLEVEL);
		dwTotalFilesSize = 0;
	}
	return dwTotalFilesSize;
}

/***************************************************************************************************
*  Function Name  : AddCOMDLLNameIntoVector()
*  Description    : Add all COM Dll into vector.
*  Author Name    : Neha
**  SR_NO		  :
*  Modified Date  :	17th March 2015
****************************************************************************************************/
bool CWardWizUpdateManager::AddCOMDLLNameIntoVector(CString csCOMDLLName)
{
	m_vCOMDLLNameVector.push_back(csCOMDLLName);
	return true;
}

/***************************************************************************
Function Name  : RegisterComDLL
Description    : Register Com DLL
Author Name    : Neha Gharge
Date           : 17th March 2015
****************************************************************************/
bool CWardWizUpdateManager::RegisterComDLL()
{
	try
	{
		CString csExePath, csCommandLine;
		CWardWizOSversion		objOSVersionWrap;
		TCHAR systemDirPath[MAX_PATH] = _T("");
		GetSystemDirectory(systemDirPath, sizeof(systemDirPath) / sizeof(_TCHAR));

		csExePath.Format(L"%s\\%s", systemDirPath, L"regsvr32.exe");

		for (int j = 0; j < static_cast<int>(m_vCOMDLLNameVector.size()); j++)
		{
			csCommandLine.Format(L"-s \"%s%s\"", GetWardWizPathFromRegistry(), m_vCOMDLLNameVector.at(j));
			//On xp runas parameter never work It will not unregister the VBSHELLEXT.DLL
			//So NUll parameter send.
			//DWORD OSType = objOSVersionWrap.DetectClientOSVersion();
			//Neha Gharge Message box showing of register successful on reinstallation.
			switch (m_OSType)
			{
			case WINOS_XP:
			case WINOS_XP64:
				ShellExecute(NULL, NULL, csExePath, csCommandLine, NULL, SWP_HIDEWINDOW);
				break;
			default:
				ShellExecute(NULL, L"runas", csExePath, csCommandLine, NULL, SWP_HIDEWINDOW);
				break;
			}
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::RegisterComDLL", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name    : GetProductID
*  Output parameter : dwProductID = return Client product ID
*  Description      : This treat will show the status and percentage from ALU service to UI
*  Author Name      : Vilas
*  SR_NO		  : WRDWIZALUSRV_0025
*  Date             : 4- Jul -2014 (Auto Live Update)
****************************************************************************************************/
bool CWardWizUpdateManager::GetProductID(DWORD &dwProductID)
{
	try
	{
		CString csIniFilePath = GetAVPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";

		if (!PathFileExists(csIniFilePath))
			return GetDWORDValueFromRegistry(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"dwProductID", dwProductID);

		g_dwProductID = dwProductID = GetPrivateProfileInt(L"VBSETTINGS", L"ProductID", 0, csIniFilePath);

		if (!dwProductID)
			return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::RegisterComDLL", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : GetDWORDValueFromRegistry
*  Description    : It will give the dword value from registry.
*  Author Name    : Vilas
*  SR_NO		  : WRDWIZALUSRV_0026
*  Date			  :	4- Jul -2014 (Auto Live Update)
****************************************************************************************************/
bool CWardWizUpdateManager::GetDWORDValueFromRegistry(HKEY hMain, LPTSTR lpszSubKey, LPTSTR lpszValuneName, DWORD &dwProductID)
{
	HKEY	hSubKey = NULL;
	__try
	{
		if (RegOpenKeyEx(hMain, lpszSubKey, 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hSubKey) != ERROR_SUCCESS)
			return true;

		DWORD	dwSize = sizeof(DWORD);
		DWORD	dwType = 0x00;

		RegQueryValueEx(hSubKey, lpszValuneName, 0, &dwType, (LPBYTE)&dwProductID, &dwSize);
		RegCloseKey(hSubKey);
		hSubKey = NULL;

		if (!dwProductID)
			return true;
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in GetDWORDValueFromRegistry", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
Function Name  : PerformPostUpdateRegOperations
Description    : Performs registry operation after successful update.
Author Name    : Vilas Suvarnakar
SR.NO			 :
Date           : 17 Jan 2015
Modified Date	 : 7/2/2015 Neha Gharge all Ini name changes.
***********************************************************************************************/
void CWardWizUpdateManager::PerformPostUpdateRegOperations()
{
	TCHAR	szIniFilePath[512] = { 0 };

	__try
	{

		DWORD	dwProductID = 0x00;

		GetProductID(dwProductID);
		if ((!dwProductID))
		{
			AddLogEntry(L"### Product ID mismatched In PerformPostUpdateRegOperations", 0, 0, true, SECONDLEVEL);
			return;
		}

		switch (dwProductID)
		{
		case 1:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchTS.ini", m_csAppFolderName, m_szAllUserPath);
			break;
		case 2:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchP.ini", m_csAppFolderName, m_szAllUserPath);
			break;
		case 3:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchT.ini", m_csAppFolderName, m_szAllUserPath);
			break;
		case 4:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchB.ini", m_szAllUserPath, m_csAppFolderName);
			break;
		case 5:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchAS.ini", m_szAllUserPath, m_csAppFolderName);
			break;
		}

		//When we update from 1.0.0.8 to 1.10.0.0 version where run entry in registry is present. On restart, it find for new file
		// VibroPatchTS.ini which are not present on restart as well so it will search entry from VBPatchTS.ini file.
		if (!PathFileExists(szIniFilePath))
		{
			switch (dwProductID)
			{
			case 1:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VBPatchTS.ini", m_szAllUserPath, m_csAppFolderName);
				break;
			case 2:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWPatchP.ini", m_szAllUserPath, m_csAppFolderName);
				break;
			case 3:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWPatchT.ini", m_szAllUserPath, m_csAppFolderName);
				break;
			case 4:
				swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWPatchB.ini", m_szAllUserPath, m_csAppFolderName);
				break;
			}
		}

		DWORD	dwNumberRegOpt = GetPrivateProfileInt(L"RegOpt", L"Count", 0, szIniFilePath);

		if (!dwNumberRegOpt)
		{
			AddLogEntry(L">>>> No post update Registry Operations", 0, 0, true, ZEROLEVEL);
			goto Cleanup;
		}

		DWORD dwIter = 0x01;
		for (; dwIter <= dwNumberRegOpt; dwIter++)
		{
			TCHAR	szValueName[128] = { 0 };
			TCHAR	szValueData[2048] = { 0 };

			swprintf_s(szValueName, _countof(szValueName), L"%lu", dwIter);
			GetPrivateProfileString(L"RegOpt", szValueName, L"", szValueData, sizeof(szValueData)-0x02, szIniFilePath);

			if (!wcslen(szValueData))
				continue;

			ParseRegLineForPostUpdate(szValueData);
		}

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::PerformPostUpdateRegOperations", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return;
}

/***********************************************************************************************
Function Name  : ParseRegLineForPostUpdate
Description    : Parses line and performs registry operation after successful update.
Author Name    : Vilas Suvarnakar
SR.NO			 :
Date           : 17 Jan 2015
modification   : 30 March 2015 Neha Gharge
: Modification of any registry key.
***********************************************************************************************/
DWORD CWardWizUpdateManager::ParseRegLineForPostUpdate(LPTSTR lpszIniLine)
{
	DWORD	dwRet = 0x00;
	__try
	{
		TCHAR	szToken[] = L",";
		TCHAR	*pToken = NULL;

		pToken = wcstok(lpszIniLine, szToken);
		if (!pToken)
		{
			dwRet = 0x01;
			goto Cleanup;
		}

		TCHAR	szRootKey[32] = { 0 };
		TCHAR	szSubKey[512] = { 0 };
		TCHAR	szValueName[512] = { 0 };
		TCHAR	szRegOpt[32] = { 0 };
		TCHAR	szModifiactionValue[512] = { 0 };
		TCHAR	szModifiactionPath[512] = { 0 };

		wcscpy(szRootKey, pToken);


		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			dwRet = 0x02;
			goto Cleanup;
		}

		wcscpy(szSubKey, pToken);

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			dwRet = 0x03;
			goto Cleanup;
		}

		wcscpy(szValueName, pToken);

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			dwRet = 0x04;
			goto Cleanup;
		}

		wcscpy(szRegOpt, pToken);
		_wcsupr(szRegOpt);

		pToken = wcstok(NULL, szToken);
		if (!pToken)
		{
			dwRet = 0x05;
		}

		if (pToken)
			wcscpy(szModifiactionValue, pToken);



		DWORD	dwRegOpt = 0x00;

		if (wcscmp(szRegOpt, L"DEL") == 0x00)
			dwRegOpt = 0x01;

		if (wcscmp(szRegOpt, L"ADD") == 0x00)
			dwRegOpt = 0x02;

		if (wcscmp(szRegOpt, L"MOD") == 0x00)
			dwRegOpt = 0x03;

		if (!dwRegOpt)
		{
			dwRet = 0x05;
			goto Cleanup;
		}

		HKEY	hRootKey = NULL;

		if (wcscmp(szRootKey, L"HKLM") == 0x00)
			hRootKey = HKEY_LOCAL_MACHINE;
		else
			hRootKey = HKEY_CURRENT_USER;

		switch (dwRegOpt)
		{
		case 0x01:
			if (g_objReg.DelRegistryValueName(hRootKey, szSubKey, szValueName))
				AddLogEntry(L">>> No Registry value for post update operation", szSubKey, szValueName, true, FIRSTLEVEL);
			else
				AddLogEntry(L">>> Deleted Registry value for post update operation", szSubKey, szValueName, true, FIRSTLEVEL);
			break;

		case 0x03:
			RenameModifiedregitrypath(szModifiactionValue);
			if (g_objReg.SetRegistryValueData(hRootKey, szSubKey, szValueName, m_szModificationAppPath))
			{
				AddLogEntry(L"### Failed to modify Registry value for post update operation %s, %s", szSubKey, szValueName, true, SECONDLEVEL);
			}
			else
			{
				AddLogEntry(L">>> Modify Registry value for post update operation %s, %s", szSubKey, szValueName, true, ZEROLEVEL);
			}

			break;
		default:
			AddLogEntry(L">>> No operation for post update Registry", 0, 0, true, FIRSTLEVEL);
		}

	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::ParseRegLineForPostUpdate", 0, 0, true, SECONDLEVEL);
	}

Cleanup:

	return dwRet;
}

/***********************************************************************************************
Function Name  : RenameModifiedregitrypath
Description    : Renamed modify reg entries.
Author Name    : Vilas Suvarnakar
Date           : 17 Jan 2015
***********************************************************************************************/
bool CWardWizUpdateManager::RenameModifiedregitrypath(TCHAR *szModifiactionValue)
{
	try
	{
		CString	csModificationAppPath;
		TCHAR szAVPath[512] = { 0 };
		csModificationAppPath = szModifiactionValue;
		_tcsnccpy(szAVPath, theApp.m_szAVPath, (_tcslen(theApp.m_szAVPath) - 1));
		csModificationAppPath.Replace(L"AppFolder", szAVPath);
		wcscpy(m_szModificationAppPath, csModificationAppPath);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::RenameModifiedregitrypath", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : IsWow64()
*  Description    : It will check client machine is 64 bit or 32bit.
*  Author Name    : Vilas , Neha
*  SR_NO		  : WRDWIZALUSRV_0030
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
void CWardWizUpdateManager::IsWow64()
{
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

	LPFN_ISWOW64PROCESS		IsWow64Processes = NULL;
	OSVERSIONINFO			OSVI = { 0 };
	TCHAR szValue[64] = { 0 };

	try
	{

		OSVI.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&OSVI);

		if (OSVI.dwMajorVersion > 5)
			m_bVistaOnward = true;


		IsWow64Processes = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(TEXT("kernel32")),
			"IsWow64Process");
		if (!IsWow64Processes)
		{
			AddLogEntry(L"### IsWow64::Failed to get address of IsWow64Process()", 0, 0, true, SECONDLEVEL);
			return;
		}

		IsWow64Processes(GetCurrentProcess(), &g_bIsWow64);
		GetEnvironmentVariable(L"PROCESSOR_ARCHITECTURE", szValue, 63);
		if (wcsstr(szValue, L"64"))
			g_bIsWow64 = TRUE;
	}
	catch (...)
	{
		AddLogEntry(L"### CVibraniumUpdateManager::IsWow64::Exception", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : StopALUpdateProcessThread()
*  Description    : Stops ALUpdate Process Thread
*  Author Name    : Vilas
**  SR_NO		  : WRDWIZALUSRV_0040
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
****************************************************************************************************/
bool CWardWizUpdateManager::StopALUpdateProcessThread()
{
	try
	{
		m_objWinHttpManager.StopCurrentDownload();
		m_objWinHttpManager.SetDownloadCompletedBytes(0);
		m_objWinHttpManager.CloseFileHandles();

		if (m_pDownloadController != NULL)
		{
			m_pDownloadController->SetThreadPoolStatus(false);
			m_pDownloadController->CancelDownload();

			//Wait for some time
			Sleep(1000);

			m_pDownloadController->StopController();
		}

		if (m_pDownloadController != NULL)
		{
			delete m_pDownloadController;
			m_pDownloadController = NULL;
		}

		TCHAR	szIniFilePath[512] = { 0 };
		if (m_hFile != NULL)
		{
			CloseHandle(m_hFile);
			m_hFile = NULL;
		}
		if (m_hIniFile != NULL)
		{
			CloseHandle(m_hIniFile);
			m_hIniFile = NULL;
		}

		if (m_hTargetFile != NULL)
		{
			CloseHandle(m_hTargetFile);
			m_hTargetFile = NULL;
		}

		CloseTrasferDataToUIThread();
		//issue solved:4469. VibroPatchTS.ini & WWizPatchB.ini would not be deleted if ALUDel.ini file exist.so product version will reflect after restarting machine.
		swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\ALUDel.ini", m_szAllUserPath, m_csAppFolderName);
		if (m_dwIsUptoDateCompleted != ALUPDATEDSUCCESSFULLY && !PathFileExists(szIniFilePath))
		{
			RemovePatchIniFile();
		}

		if (theApp.m_bRequestFromUI)
		{
			Sleep(100);
			SendMessage2UI(UPDATE_FINISHED, 1);
			if (!m_bUpdateFailed || !m_bUpToDate)
			{
				if (!WriteIntoRegistry())
				{
					AddLogEntry(L"### Failed to write app version no and database", 0, 0, true, SECONDLEVEL);
				}
			}
			//theApp.m_bRequestFromUI = false;
		}
		else
		{
			if (m_bUpdateFailed)
			{
				Sleep(100);
				SendMessage2UI(UPDATE_FINISHED, 2, 2);
				m_bUpdateFailed = false;
			}
			else if (m_bUpToDate)
			{
				Sleep(100);
				SendMessage2UI(UPDATE_FINISHED, 2, 3);
				m_bUpToDate = false;

			}
			else if (m_bUpdateSuccess) //condition is checked.
			{
				Sleep(100);

				SendMessage2UI(UPDATE_FINISHED, 2, 1, m_bIsAnyProductChanges ? 1 : 0);

				if (!WriteIntoRegistry())
				{
					AddLogEntry(L"### Failed to write app version no and database", 0, 0, true, SECONDLEVEL);
				}
			}
			theApp.m_bRequestFromUI = false;
		}

		ClearMemoryMapObjects();
		Sleep(10);
	}
	catch (...)
	{
		AddLogEntry(L"### CVibraniumUpdateManager::StopALUpdateProcessThread::Exception", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : StopALUpdateProcess4NoUI()
*  Description    : Stops ALUpdate Process Thread
*  Author Name    : Vilas
**  SR_NO		  : WRDWIZALUSRV_0040
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
****************************************************************************************************/
bool CWardWizUpdateManager::StopALUpdateProcess4NoUI()
{
	try
	{
		m_objWinHttpManager.StopCurrentDownload();
		m_objWinHttpManager.SetDownloadCompletedBytes(0);
		m_objWinHttpManager.CloseFileHandles();

		if (m_pDownloadController != NULL)
		{
			m_pDownloadController->SetThreadPoolStatus(false);
			m_pDownloadController->CancelDownload();

			//Wait for some time
			Sleep(1000);

			m_pDownloadController->StopController();
		}

		if (m_pDownloadController != NULL)
		{
			delete m_pDownloadController;
			m_pDownloadController = NULL;
		}

		TCHAR	szIniFilePath[512] = { 0 };
		if (m_hFile != NULL)
		{
			CloseHandle(m_hFile);
			m_hFile = NULL;
		}
		if (m_hIniFile != NULL)
		{
			CloseHandle(m_hIniFile);
			m_hIniFile = NULL;
		}

		if (m_hTargetFile != NULL)
		{
			CloseHandle(m_hTargetFile);
			m_hTargetFile = NULL;
		}

		CloseTrasferDataToUIThread();
		//issue solved:4469. VibroPatchTS.ini & WWizPatchB.ini would not be deleted if ALUDel.ini file exist.so product version will reflect after restarting machine.
		swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\ALUDel.ini", m_szAllUserPath, m_csAppFolderName);
		if (m_dwIsUptoDateCompleted != ALUPDATEDSUCCESSFULLY && !PathFileExists(szIniFilePath))
		{
			RemovePatchIniFile();
		}

		if (theApp.m_bRequestFromUI)
		{
			Sleep(100);
			if (!m_bUpdateFailed || !m_bUpToDate)
			{
				if (!WriteIntoRegistry())
				{
					AddLogEntry(L"### Failed to write app version no and database", 0, 0, true, SECONDLEVEL);
				}
			}
			//theApp.m_bRequestFromUI = false;
		}
		else
		{
			if (m_bUpdateFailed)
			{
				Sleep(100);
				m_bUpdateFailed = false;
			}
			else if (m_bUpToDate)
			{
				Sleep(100);
				m_bUpToDate = false;

			}
			else if (m_bUpdateSuccess) //condition is checked.
			{
				Sleep(100);

				if (!WriteIntoRegistry())
				{
					AddLogEntry(L"### Failed to write app version no and database", 0, 0, true, SECONDLEVEL);
				}
			}
			theApp.m_bRequestFromUI = false;
		}

		ClearMemoryMapObjects();
		Sleep(10);
	}
	catch (...)
	{
		AddLogEntry(L"### Exceptin in CVibraniumUpdateManager::StopALUpdateProcess4NoUI", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : WriteIntoRegistry()
*  Description    : Write into registry
*  Author Name    : Vilas , Neha
*  SR_NO		  : WRDWIZALUSRV_0074
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
*  Modified Date  : 7-Feb-2015 Neha Gharge Update version no issue resolved.
****************************************************************************************************/
bool CWardWizUpdateManager::WriteIntoRegistry()
{
	TCHAR	szIniFilePath[512] = { 0 };
	TCHAR  szValueData[512] = { 0 };
	DWORD	dwProductID = 0x00;
	DWORD dwSuccess = 0x00;
	CITinRegWrapper objReg;
	//Issue : Database version / Product version should not change when product ask for restart, it should be update after restart
	//Resolved By :  Nitin K.
	TCHAR	szALUDelIniFilePath[512] = { 0 };

	GetProductID(dwProductID);
	if (!dwProductID)
	{
		AddLogEntry(L"### Product ID mismatched", 0, 0, true, SECONDLEVEL);
		return true;
	}

	switch (dwProductID)
	{
	case 1:
		swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchTS.ini", m_szAllUserPath, m_csAppFolderName);
		break;
	case 2:
		swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchP.ini", m_szAllUserPath, m_csAppFolderName);
		break;
	case 3:
		swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchT.ini", m_szAllUserPath, m_csAppFolderName);
		break;
	case 4:
		swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchB.ini", m_szAllUserPath, m_csAppFolderName);
		break;
	case 5:
		swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchAS.ini", m_szAllUserPath, m_csAppFolderName);
		break;
	}


	if (!PathFileExists(szIniFilePath))
	{
		//AddLogEntry(L"### ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, SECONDLEVEL);
		//divya
		/* Issue Number 159
		Failed to update product. Please check internet connection”
		even if internet connection is there
		*/
		AddLogEntry(L"### Functin is WriteIntoRegistry.... ParseALUDel_Ini::File not found(%s)", szIniFilePath, 0, true, ZEROLEVEL);
		//return false;
	}
	//Issue : Database version / Product version should not change when product ask for restart, it should be update after restart
	//Resolved By :  Nitin K.

	swprintf_s(szALUDelIniFilePath, _countof(szALUDelIniFilePath), L"%s\\%s\\%s", m_szAllUserPath, m_csAppFolderName, WRDWIZALUDELINI);

	//issue:- support number not getting reflected in product,now support number added to ini file
	//  resolve by lalit,neha 9-18-2015
	if (PathFileExists(szIniFilePath))
	{
		DWORD dwOfflineReg = 0;

		dwOfflineReg = CheckOfflineReg();


		if (dwOfflineReg == 0x00)
		{
			GetPrivateProfileString(L"SupportNumber", L"SupportNumberNC", L"", szValueData, 511, szIniFilePath);
		}
		else
		{
			GetPrivateProfileString(L"SupportNumber", L"SupportNumber", L"1800 203 4600", szValueData, 511, szIniFilePath);
		}

		dwSuccess = objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"SupportNo", szValueData);
		if (dwSuccess)
		{
			AddLogEntry(L"### Failed to Set support number in registry", 0, 0, true, SECONDLEVEL);
		}
	}

	if ((!PathFileExists(szALUDelIniFilePath)) && m_bIsALUDeleted == FALSE)
	{

		if (PathFileExists(szIniFilePath))
		{
			GetPrivateProfileString(L"ProductVersion", L"ProductVer", L"", szValueData, 511, szIniFilePath);
			if (!szValueData[0])
			{
				AddLogEntry(L"### WriteIntoRegistry::Invalid Entries for(%s) in (%s)", szValueData, szIniFilePath, true, ZEROLEVEL);
			}
			else
			{
				if (ReadProductUpdateEnableCheck())
				{
					dwSuccess = objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"AppVersion", szValueData);
					if (dwSuccess)
					{
						AddLogEntry(L"### Failed to Setregistryvaluedata %s", CWWizSettingsWrapper::GetProductRegistryKey(), 0, true, SECONDLEVEL);
					}
				}

				TCHAR szScanEngineVer[0x20] = { 0 };
				GetPrivateProfileString(L"ScanEngineVersion", L"ScanEngineVer", L"", szScanEngineVer, 0x20, szIniFilePath);
				if (szScanEngineVer[0])
				{
					dwSuccess = objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"ScanEngineVersion", szScanEngineVer);
					if (dwSuccess)
					{
						AddLogEntry(L"### Failed to Setregistryvaluedata for %s : ScanEngineVersion", CWWizSettingsWrapper::GetProductRegistryKey(), 0, true, SECONDLEVEL);
					}
				}
			}

			GetPrivateProfileString(L"DatabaseVersion", L"DatabaseVer", L"", szValueData, 511, szIniFilePath);
			if (!szValueData[0])
			{
				AddLogEntry(L"### WriteIntoRegistry::Invalid Entries for(%s) in (%s)", szValueData, szIniFilePath, true, ZEROLEVEL);
			}
			else
			{
				dwSuccess = objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"DataBaseVersion", szValueData);
				if (dwSuccess)
				{
					AddLogEntry(L"### Failed to Setregistryvaluedata", 0, 0, true, SECONDLEVEL);
				}
			}

			//Neha Gharge 8 August,2015 Need to add DataEncrtption version no.
			if (dwProductID != 4)//if product ID is basic. No need to add dataencryption version.
			{
				GetPrivateProfileString(L"DataEncVersion", L"DataEncVer", L"", szValueData, 511, szIniFilePath);
				if (!szValueData[0])
				{
					AddLogEntry(L"### WriteIntoRegistry::Invalid Entries for(%s) in (%s)", szValueData, szIniFilePath, true, ZEROLEVEL);
					return false;
				}
				else
				{
					dwSuccess = objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"DataEncVersion", szValueData);
					if (dwSuccess)
					{
						AddLogEntry(L"### Failed to Setregistryvaluedata", 0, 0, true, SECONDLEVEL);
						return false;
					}
				}
			}
		}
	}
	return true;
}

/***********************************************************************************************
Function Name  : CheckOfflineReg
Description    : To check scan level
0-> Off
1-> On
SR.NO			 :
Author Name    : Neha gharge
Date           : 12-18-2015
***********************************************************************************************/
DWORD CWardWizUpdateManager::CheckOfflineReg()
{
	DWORD dwOfflineReg = 0x00;
	DWORD dwScanlevel = 0x00;
	try
	{
		if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"dwIsOffline", dwOfflineReg) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwIsOffline in CheckOfflineReg", 0, 0, true, FIRSTLEVEL);

			dwScanlevel = CheckScanLevel();
			if (dwScanlevel == 0x02)
			{
				dwOfflineReg = 0x01;
			}
			else
			{
				dwOfflineReg = 0x00;
			}

			if (g_objReg.SetRegistryValueData(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"dwIsOffline", dwOfflineReg) != 0x00)
			{
				AddLogEntry(L"### Failed to Set Registry Entry for dwIsOffline in CheckOfflineReg", 0, 0, true, SECONDLEVEL);
				return 0x00;
			}
		}

		switch (dwOfflineReg)
		{
		case 0x01:
			dwOfflineReg = 0x01;
			break;
		default:
			dwOfflineReg = 0x00;
			break;
		}
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CVibraniumUpdateManager::CheckOfflineReg", 0, 0, true, SECONDLEVEL);
	}
	return dwOfflineReg;
}

/***********************************************************************************************
Function Name  : CheckScanLevel
Description    : To check scan level 1-> Only with wardwiz scanner
2-> with clam scanner and second level scaner is wardwiz scanner.
SR.NO			 :
Author Name    : Neha gharge
Date           : 1-19-2015
***********************************************************************************************/
DWORD CWardWizUpdateManager::CheckScanLevel()
{
	DWORD dwScanLevel = 0x00;
	try
	{
		if (g_objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, g_csProdRegKey.GetBuffer(), L"dwScanLevel", dwScanLevel) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwScanLevel in CheckScanLevel", 0, 0, true, FIRSTLEVEL);;
			return 0x00;
		}

		switch (dwScanLevel)
		{
		case 0x01:
			return 0x01;
			break;
		case 0x02:
			return 0x02;
			break;
		default:
			return 0x00;
		}
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CVibraniumUpdateManager::CheckScanLevel", 0, 0, true, SECONDLEVEL);

	}
	return dwScanLevel;
}

/***************************************************************************************************
*  Function Name  : CloseTrasferDataToUIThread()
*  Description    : Close Trasfer Data To UI thread
*  Author Name    : Vilas, neha
*  SR_NO		  : WRDWIZALUSRV_0065
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
void CWardWizUpdateManager::CloseTrasferDataToUIThread()
{
	if (m_OSType != WINOS_XP && m_OSType != WINOS_XP64)
	{
		g_objcriticalSection.Unlock();
	}
}

/***************************************************************************************************
*  Function Name  : WriteIntoRegistry()
*  Description    : Write into registry
*  Author Name    : Vilas , Neha
*  SR_NO		  : WRDWIZALUSRV_0074
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
*  Modified Date  : 7-Feb-2015 Neha Gharge Update version no issue resolved.
****************************************************************************************************/
bool CWardWizUpdateManager::RemovePatchIniFile()
{
	bool bReturn = false;
	try
	{
		TCHAR	szIniFilePath[512] = { 0 };
		DWORD	dwProductID = 0x00;

		GetProductID(dwProductID);
		if (!dwProductID)
		{
			AddLogEntry(L"### Product ID mismatched", 0, 0, true, SECONDLEVEL);
			return true;
		}

		switch (dwProductID)
		{
		case 1:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchTS.ini", m_szAllUserPath, m_csAppFolderName);
			break;
		case 2:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchP.ini", m_szAllUserPath, m_csAppFolderName);
			break;
		case 3:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchT.ini", m_szAllUserPath, m_csAppFolderName);
			break;
		case 4:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchB.ini", m_szAllUserPath, m_csAppFolderName);
			break;
		case 5:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchAS.ini", m_szAllUserPath, m_csAppFolderName);
			break;
		}

		SetFileAttributes(szIniFilePath, FILE_ATTRIBUTE_NORMAL);
		bReturn = DeleteFile(szIniFilePath) ? true : false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::RemovePatchIniFile", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SendMessage2UI()
*  Description    : Send message to Ui through Pipe
*  Author Name    : Vilas, neha
*  SR_NO		  : WRDWIZALUSRV_0062
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::SendMessage2UI(int iRequest, DWORD dwUITypetoSendMessge, DWORD dwMsgType, DWORD dwSecondOption, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iRequest;

		CString csLog;
		csLog.Format(L"Request: %d\tUITypetoSendMessge:%d\tdwMsgType:%d", iRequest, dwUITypetoSendMessge, dwMsgType);

		if (dwUITypetoSendMessge == 1)//1 is for Main UI
		{
		
			if (!m_pfnDownloadStatusCallback)
				return false;

			m_pfnDownloadStatusCallback(iRequest, dwUITypetoSendMessge, dwMsgType, dwSecondOption, this);

		}
		else if (dwUITypetoSendMessge == 2)//2 Tray Server
		{
			szPipeData.dwValue = dwMsgType;
			szPipeData.dwSecondValue = dwSecondOption;
			CISpyCommunicator objCom(TRAY_SERVER, bWait);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CVibraniumUpdateManager::SendMessage2UI %s", csLog, 0, true, SECONDLEVEL);
				return false;
			}

			if (bWait)
			{
				if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
				{
					AddLogEntry(L"### Failed to send data in CDataEncryptionDlg : SendRegisteredData2Service %s", csLog, 0, true, SECONDLEVEL);
					return false;
				}

				if (szPipeData.dwValue != 1)
				{
					return false;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::SendMessage2UI", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : StopALUpdateProcessThread4UpdtMgr()
*  Description    : Stops ALUpdate Process Thread
*  Author Name    : Amol Jaware
*  SR_NO		  :
*  Date           :	17-April-2018
****************************************************************************************************/
bool CWardWizUpdateManager::StopALUpdateProcessThread4UpdtMgr()
{
	try
	{
		m_objWinHttpManagerUpDM.StopCurrentDownload();
		m_objWinHttpManagerUpDM.SetDownloadCompletedBytes(0);
		m_objWinHttpManagerUpDM.CloseFileHandles();

		if (m_pDownloadController4UpdtMgr != NULL)
		{
			m_pDownloadController4UpdtMgr->SetThreadPoolStatus(false);
			m_pDownloadController4UpdtMgr->CancelDownload();

			//Wait for some time
			Sleep(1000);

			m_pDownloadController4UpdtMgr->StopController();
		}

		if (m_pDownloadController4UpdtMgr != NULL)
		{
			delete m_pDownloadController4UpdtMgr;
			m_pDownloadController4UpdtMgr = NULL;
		}

		/*TCHAR	szIniFilePath[512] = { 0 };
		if (m_hFile != NULL)
		{
			CloseHandle(m_hFile);
			m_hFile = NULL;
		}
		if (m_hIniFile != NULL)
		{
			CloseHandle(m_hIniFile);
			m_hIniFile = NULL;
		}

		if (m_hTargetFile != NULL)
		{
			CloseHandle(m_hTargetFile);
			m_hTargetFile = NULL;
		}


		if (theApp.m_bRequestFromUI)
		{
			Sleep(100);
			SendMessage2UI(UPDATE_FINISHED, 1);
			if (!m_bUpdateFailed || !m_bUpToDate)
			{
				if (!WriteIntoRegistry())
				{
					AddLogEntry(L"### Failed to write app version no and database", 0, 0, true, SECONDLEVEL);
				}
			}
			//theApp.m_bRequestFromUI = false;
		}
		else
		{
			if (m_bUpdateFailed)
			{
				Sleep(100);
				SendMessage2UI(UPDATE_FINISHED, 2, 2);
				m_bUpdateFailed = false;
			}
			else if (m_bUpToDate)
			{
				Sleep(100);
				SendMessage2UI(UPDATE_FINISHED, 2, 3);
				m_bUpToDate = false;

			}
			else if (m_bUpdateSuccess) //condition is checked.
			{
				Sleep(100);

				SendMessage2UI(UPDATE_FINISHED, 2, 1, m_bIsAnyProductChanges ? 1 : 0);

				if (!WriteIntoRegistry())
				{
					AddLogEntry(L"### Failed to write app version no and database", 0, 0, true, SECONDLEVEL);
				}
			}
			theApp.m_bRequestFromUI = false;
		}

		ClearMemoryMapObjects();
		Sleep(10);*/
	}
	catch (...)
	{
		AddLogEntry(L"### CVibraniumUpdateManager::StopALUpdateProcessThread4UpdtMgr::Exception", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : DownLoadIniFileAndParse()
*  Description    : It will download ini.,Fills struct with modified(Mismatched) binaries,
*  Author Name    : Vilas , Neha
*  SR_NO		  : WRDWIZALUSRV_0033
*  Date			  :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
*  Modified Date  : 2/2/2015 Neha Gharge Download INI file name changes.
*  Naming conventions :
1) WWizPatchB.ini - Basic INI.
2) VibroPatchTS.ini - Essential INI.
3) WWizPatchP.ini - Pro INI.
4) WWizPatchT.ini - Elite INI.
****************************************************************************************************/
bool CWardWizUpdateManager::DownLoadIniFileAndParse()
{

	TCHAR	szIniFilePath[512] = { 0 };
	DWORD	dwProductID = 0x00;
	try
	{
		SetDownloadPercentage(SETMESSAGE, CHECKING4UPDATES, 0, 0, this);
		//SendUpdateInfoToGUI(SETMESSAGE, L"Checking for updates", L"", 0, 0);
		GetProductID(dwProductID);
		if (!dwProductID)
		{
			AddLogEntry(L"### Product ID mismatched", 0, 0, true, SECONDLEVEL);
			return false;
		}

		switch (dwProductID)
		{
		case 1:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchTS.ini", m_szAllUserPath, m_csAppFolderName);
			break;
		case 2:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchP.ini", m_szAllUserPath, m_csAppFolderName);
			break;
		case 3:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchT.ini", m_szAllUserPath, m_csAppFolderName);
			break;
		case 4:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\WWizPatchB.ini", m_szAllUserPath, m_csAppFolderName);
			break;
		case 5:
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s\\%s\\VibroPatchAS.ini", m_szAllUserPath, m_csAppFolderName);
			break;
		}

		//Issue if file size of ini is same as previous file existing at programdata
		//.it is not downloading new file from server
		//With previous file it check all md5. send message 
		if (PathFileExists(szIniFilePath))
		{
			SetFileAttributes(szIniFilePath, FILE_ATTRIBUTE_NORMAL);
			if (!DeleteFile(szIniFilePath))
			{
				AddLogEntry(L"### Failed to delete previous ini file %s", szIniFilePath, 0, true, FIRSTLEVEL);
			}
			else
			{
				AddLogEntry(L">>> Successful to delete previous ini file %s", szIniFilePath, 0, true, FIRSTLEVEL);
			}
		}
		else
		{
			AddLogEntry(L"### Path of previous ini file %s is not exist", szIniFilePath, 0, true, FIRSTLEVEL);
		}

		TCHAR	szDomainName[MAX_PATH] = { 0 };
		if (!GetDomainName(szDomainName, MAX_PATH))
		{
			AddLogEntry(L"### Failed to get GetDomainName in MakeDownloadUrlList", 0, 0, true, SECONDLEVEL);
			return false;
		}
		////Please add code for download .ini file here
		CString csFileName;
		switch (dwProductID)
		{
		case 1:
			csFileName.Format(L"http://%s/%s/%s", szDomainName, szUpdateFolderName, L"VibroPatchTS.ini");
			break;
		case 2:
			csFileName.Format(L"http://%s/%s/%s", szDomainName, szUpdateFolderName, L"WWizPatchP.ini");
			break;
		case 3:
			csFileName.Format(L"http://%s/%s/%s", szDomainName, szUpdateFolderName, L"WWizPatchT.ini");
			break;
		case 4:
			csFileName.Format(L"http://%s/%s/%s", szDomainName, szUpdateFolderName, L"WWizPatchB.ini");
			break;
		case 5:
			csFileName.Format(L"http://%s/%s/%s", szDomainName, szUpdateFolderName, L"VibroPatchAS.ini");
			//csFileName.Format(L"http://%s/%s/%s", szDomainName, szUpdateFolderName, L"VibroPatchAS.ini");
			break;
		}
		m_objWinHttpManager.m_bIsConnected = true;

		m_objWinHttpManager.StartCurrentDownload();

		if (!m_iServerCount)
		{
			if (StartDownloadingINI(csFileName))
			{
				AddLogEntry(L">>> ini file downloaded successfully", 0, 0, true, ZEROLEVEL);
				//ini file into program data\wardwiz folder.
			}
			else
			{
				AddLogEntry(L"### Failed to download ini file %s", csFileName, 0, true, SECONDLEVEL);
				return false;
			}
		}
		else
		{
			for (int i = 0; i < m_iServerCount; i++)
			{
				if (StartDownloadingINI(csFileName))
				{
					AddLogEntry(L">>> ini file downloaded successfully", 0, 0, true, ZEROLEVEL);
					//ini file into program data\wardwiz folder.
					break;
				}
				else
				{
					AddLogEntry(L"### Failed to download ini file %s", csFileName, 0, true, SECONDLEVEL);
					return false;
				}
			}
		}

		WardWizParseIniFile	ParseIniFile;

		m_bIsAnyProductChanges = true;
		bool bProdUPdate = ReadProductUpdateEnableCheck();
		ParseIniFile.ParseDownloadedPatch_Ini(szIniFilePath, dwProductID, bProdUPdate, m_bIsAnyProductChanges, g_bIsWow64 ? true : false);
		if (ParseIniFile.GetLastErrorForParseDownloadedPatch_Ini())
			return false;
	}
	catch (...)
	{
		AddLogEntry(L"### CVibraniumUpdateManager::DownLoadIniFileAndParse::Exception", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : StartDownloadingINI()
*  Description    : Downloading Only ini according to client product ID
*  Author Name    : Vilas, neha
*  SR_NO		  : WRDWIZALUSRV_0061
*  Date           :	4- Jul -2014 - 12 jul -2014(Auto Live Update)
*  Modified Date  :	14-Jul -2014 to 22-jul-2014(Neha Gharge)
****************************************************************************************************/
bool CWardWizUpdateManager::StartDownloadingINI(LPCTSTR szUrlPath)
{
	try
	{
		if (!szUrlPath)
			return false;

		//Sleep( 15 * 1000 );

		TCHAR szInfo[MAX_PATH] = { 0 };
		TCHAR szTagetPath[MAX_PATH] = { 0 };
		TCHAR szTargetFolder[MAX_PATH] = { 0 };
		DWORD dwBufLen = MAX_PATH * sizeof(TCHAR);
		DWORD dwTotalFileSize = 0;

		CString csFileName(szUrlPath);
		int iFound = csFileName.ReverseFind(L'/');
		csFileName = csFileName.Right(csFileName.GetLength() - iFound - 1);

		if (m_objWinHttpManager.Initialize(szUrlPath))
		{
			if (!m_objWinHttpManager.GetHeaderInfo(WINHTTP_QUERY_CONTENT_LENGTH, szInfo, dwBufLen))
			{
				return false;
			}
			dwTotalFileSize = _wtol(szInfo);

			_stprintf_s(szTargetFolder, MAX_PATH, L"%s\\%s", m_szAppDataFolder, m_csAppFolderName);

			if (!PathFileExists(szTargetFolder))
			{
				CreateDirectoryFocefully(szTargetFolder);
			}

			_stprintf_s(szTagetPath, MAX_PATH, L"%s\\%s\\%s", m_szAppDataFolder, m_csAppFolderName, csFileName);

			//If file is already present check filesize and download from last point
			DWORD dwStartBytes = 0;
			if (PathFileExists(szTagetPath))
			{

				m_hIniFile = CreateFile(szTagetPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
				if (m_hIniFile != INVALID_HANDLE_VALUE)
				{
					dwStartBytes = GetFileSize(m_hIniFile, 0);
					if (dwStartBytes != dwTotalFileSize)
					{
						m_objWinHttpManager.SetDownloadCompletedBytes(dwStartBytes);
					}
				}
				CloseHandle(m_hIniFile);
				m_hIniFile = NULL;
			}

			//Start download for ini file
			if (m_objWinHttpManager.Download(szTagetPath, dwStartBytes, dwTotalFileSize))
			{
				//Once download complete set the download completed bytes.
				m_objWinHttpManager.SetDownloadCompletedBytes(dwTotalFileSize - dwStartBytes);
			}
			else
			{
				AddLogEntry(L"### Failed to download file %s", szUrlPath, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::StartDownloadingINI", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : DownLoadIniFileAndParse4EPS()
*  Description    : It will download ini.,Fills struct with modified(Mismatched) binaries,
*  Author Name    :
*  SR_NO		  :
*  Date			  :	12-April-2018
****************************************************************************************************/
bool CWardWizUpdateManager::DownLoadIniFileAndParse4EPS()
{

	TCHAR	szIniFilePath[512] = { 0 };
	DWORD	dwProductID = 0x00;

	try
	{
		//SetDownloadPercentage(SETMESSAGE, CHECKING4UPDATES, 0, 0, this);

		GetProductID(dwProductID);
		if (!dwProductID)
		{
			AddLogEntry(L"### Product ID mismatched", 0, 0, true, SECONDLEVEL);
			return false;
		}

		if (dwProductID == 3)
			swprintf_s(szIniFilePath, _countof(szIniFilePath), L"%s%s\\WWizPatchEPS.ini", theApp.m_szAVPath, szwardwizEPSPublish);

		if (PathFileExists(szIniFilePath))
		{
			SetFileAttributes(szIniFilePath, FILE_ATTRIBUTE_NORMAL);
			if (!DeleteFile(szIniFilePath))
			{
				AddLogEntry(L"### Failed to delete previous ini file::DownLoadIniFileAndParse4EPS %s", szIniFilePath, 0, true, FIRSTLEVEL);
			}
			else
			{
				AddLogEntry(L">>> Successful to delete previous ini file::DownLoadIniFileAndParse4EPS %s", szIniFilePath, 0, true, FIRSTLEVEL);
			}
		}
		else
		{
			AddLogEntry(L"### Path of previous ini file %s is not exist::DownLoadIniFileAndParse4EPS", szIniFilePath, 0, true, FIRSTLEVEL);
		}

		TCHAR	szDomainName[MAX_PATH] = { 0 };
		if (!GetDomainName(szDomainName, MAX_PATH))
		{
			AddLogEntry(L"### Failed to get GetDomainName in MakeDownloadUrlList", 0, 0, true, SECONDLEVEL);
			return false;
		}
		//Please add code for download .ini file here
		CString csClientIniFile;
		if (dwProductID == 3)
			csClientIniFile.Format(L"http://%s/%s/%s", szDomainName, szUpdateFolderName, L"WWizPatchT.ini");

		if (!StartDownloadingINIForZipFiles(csClientIniFile))
		{
			AddLogEntry(L"### Failed to download ini file::DownLoadIniFileAndParse4EPS %s", csClientIniFile, 0, true, SECONDLEVEL);
			return false;
		}
		//Please add code for download .ini file here
		CString csFileName;
		if (dwProductID == 3)
			csFileName.Format(L"http://%s/%s/%s", szDomainName, szUpdateFolderName, L"WWizPatchEPS.ini");

		m_objWinHttpManagerUpDM.m_bIsConnected = true;

		m_objWinHttpManagerUpDM.StartCurrentDownload();

		if (StartDownloadingINIForZipFiles(csFileName))
		{
			AddLogEntry(L">>> ini file downloaded successfully", 0, 0, true, ZEROLEVEL);
			//ini file into C:\Program Files\WardWiz\wardwizEPSPublish folder.
		}
		else
		{
			AddLogEntry(L"### Failed to download ini file::DownLoadIniFileAndParse4EPS %s", csFileName, 0, true, SECONDLEVEL);
			return false;
		}

		WardWizParseIniFile	ParseIniFile;

		m_bIsAnyProductChanges = true;
		bool bProdUPdate = ReadProductUpdateEnableCheck();

		vALUZipFileInfo.clear();

		ParseIniFile.ParseDownloadedPatch_Ini4UpdtMgr(szIniFilePath, dwProductID, bProdUPdate, m_bIsAnyProductChanges);
		if (ParseIniFile.GetLastErrorForParseDownloadedPatch_Ini())
			return false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::DownLoadIniFileAndParse4EPS", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : StartDownloadingINIForZipFiles()
*  Description    : Downloading Only ini according to client product ID
*  Author Name    : Amol Jaware.
*  SR_NO		  :
*  Date           :	12-April-2018
****************************************************************************************************/
bool CWardWizUpdateManager::StartDownloadingINIForZipFiles(LPCTSTR szUrlPath)
{
	try
	{
		if (!szUrlPath)
			return false;

		TCHAR szInfo[MAX_PATH] = { 0 };
		TCHAR szTargetPath[MAX_PATH] = { 0 };
		TCHAR szTargetFolder[MAX_PATH] = { 0 };
		DWORD dwBufLen = MAX_PATH * sizeof(TCHAR);
		DWORD dwTotalFileSize = 0;
		CString csFileName(szUrlPath);
		int iFound = csFileName.ReverseFind(L'/');
		csFileName = csFileName.Right(csFileName.GetLength() - iFound - 1);
		CWinHttpManager		objWinHttpManagerUpDM;
		Sleep(2000);

		if (objWinHttpManagerUpDM.Initialize(szUrlPath))
		{
			if (!objWinHttpManagerUpDM.GetHeaderInfo(WINHTTP_QUERY_CONTENT_LENGTH, szInfo, dwBufLen))
			{
				return false;
			}
			dwTotalFileSize = _wtol(szInfo);

			wcscpy(szTargetFolder, m_cswardwizEPSPublishPath);
			if (!PathFileExists(szTargetFolder))
			{
				CreateDirectoryFocefully(szTargetFolder);
			}

			_stprintf_s(szTargetPath, MAX_PATH, L"%s\\%s", m_cswardwizEPSPublishPath, csFileName);

			//If file is already present check filesize and download from last point
			DWORD dwStartBytes = 0;
			if (PathFileExists(szTargetPath))
			{

				m_hIniFile = CreateFile(szTargetPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
				if (m_hIniFile != INVALID_HANDLE_VALUE)
				{
					dwStartBytes = GetFileSize(m_hIniFile, 0);
					if (dwStartBytes != dwTotalFileSize)
					{
						objWinHttpManagerUpDM.SetDownloadCompletedBytes(dwStartBytes);
					}
				}
				CloseHandle(m_hIniFile);
				m_hIniFile = NULL;
			}

			//Start download for ini file
			if (objWinHttpManagerUpDM.Download(szTargetPath, dwStartBytes, dwTotalFileSize))
			{
				//Once download complete set the download completed bytes.
				objWinHttpManagerUpDM.SetDownloadCompletedBytes(dwTotalFileSize - dwStartBytes);
			}
			else
			{
				AddLogEntry(L"### Failed to download file %s", szUrlPath, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::StartDownloadingINIForZipFiles", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
Function Name  : GetDomainName
Description    : Function to get wardwiz domain name.
Author Name    : Ramkrushna Shelke
SR.NO			 :
Date           : 10 Oct 2014
***********************************************************************************************/
bool CWardWizUpdateManager::GetDomainName(LPTSTR pszDomainName, DWORD dwSize)
{
	try
	{
		if (!m_bIsRelayClient)
		{
			CString csServerName = CWWizSettings::WWizGetServerIPAddress();
			_tcscpy_s(pszDomainName, dwSize, csServerName);
		}
		else
		{
			if (g_csServerName.IsEmpty())
			{
				_tcscpy_s(pszDomainName, dwSize, L"www.vibranium.co.in");
			}
			else
			{
				_tcscpy_s(pszDomainName, dwSize, g_csServerName);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::GetDomainName", 0, 0, true, SECONDLEVEL);
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : ReadAutoliveUpdateEnableCheck()
*  Description    : Read AutoliveUpdate Check
*  Author Name    : Ramkrushna Shelke
**  SR_NO		  :
*  Date			  :	10 - Oct - 2016
****************************************************************************************************/
bool CWardWizUpdateManager::ReadProductUpdateEnableCheck()
{
	bool bReturn = true;

	try
	{
		CITinRegWrapper objReg;

		DWORD dwProductUpdate = 0;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, CWWizSettingsWrapper::GetProductRegistryKey().GetBuffer(), L"dwAutoProductUpdate", dwProductUpdate) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwAutoDefUpdate in ReadProductUpdateEnableCheck", 0, 0, true, SECONDLEVEL);;
			return bReturn;
		}

		if (dwProductUpdate == 0x00)
		{
			bReturn = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReadProductUpdateEnableCheck", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/***********************************************************************************************
Function Name  : SendData2CommService
Description    : Send data to communication service.
Author Name    : Ram Shelke
SR.NO		   :
Date           : 08 Apr 2015
***********************************************************************************************/
DWORD CWardWizUpdateManager::SendData2CommService(int iMesssageInfo, bool bWait)
{
	DWORD dwRet = 0x00;
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iMesssageInfo;
		CISpyCommunicator objCom(SERVICE_SERVER, false);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data in SendData2CommService", 0, 0, true, SECONDLEVEL);
			dwRet = 0x01;
			goto FAILED;
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to Read data in SendData2CommService", 0, 0, true, SECONDLEVEL);
				dwRet = 0x02;
				goto FAILED;
			}

			if (szPipeData.dwValue != 1)
			{
				dwRet = 0x03;
				goto FAILED;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::SendData2CommService", 0, 0, true, SECONDLEVEL);
		dwRet = 0x04;
	}

FAILED:
	return dwRet;
}

/***********************************************************************************************
Function Name  : SendData2CommService
Description    : callback function to set download status.
Author Name    : Amol Jaware
Date           : 04 Feb 2019
***********************************************************************************************/
bool CWardWizUpdateManager::SetDownloadPercentage(DWORD dwFlag, DWORD dwPercent, DWORD dwTotalFileSize, DWORD dwCurrentDownloadedbytes, void *callbackParam)
{
	__try
	{
		if (!m_pfnDownloadStatusCallback)
			return false;
		if (theApp.m_bEPSLiveUpdateNoUI == false)
		{
			m_pfnDownloadStatusCallback(dwFlag, dwPercent, dwTotalFileSize, dwCurrentDownloadedbytes, callbackParam);
			Sleep(10);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::SetDownloadPercentage", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
Function Name  : StopLiveUpdateThread
Description    : Stop live update.
Author Name    : Amol Jaware
Date           : 05 Feb 2019
***********************************************************************************************/
void CWardWizUpdateManager::StopLiveUpdateThread(DWORD dwValue)
{
	try
	{
		//if ((dwValue == 2 && m_bRequestFromUI == false) || (dwValue == 1 && m_bRequestFromUI == true))
		if (dwValue == 1 && theApp.m_bRequestFromUI == true) //THIS HAS TO BE EXECUTED IN CASE CLICK ON RETRY BUTTON
		{
				m_bUpdateFailed = true;

				CloseTrasferDataToUIThread();
				StopALUpdateProcessThread4UpdtMgr();
				StopALUpdateProcessThread();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::StopLiveUpdateThread", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
Function Name  : RemoveProxySetting
Description    : Remove proxy setting.
Author Name    : Amol Jaware
Date           : 05 Feb 2019
***********************************************************************************************/
void CWardWizUpdateManager::RemoveProxySetting()
{
	try
	{
		//remove proxy setting data
		m_objWinHttpManager.m_dwProxySett = 0X00;
		if (m_objWinHttpManager.m_csServer.GetLength() > 0)
		{
			m_objWinHttpManager.m_csServer.Empty();
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::RemoveProxySetting", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : SetAllALtFilesAttributeToNorml()
*  Description    : Set all files attributes to normal
*  Author Name    : Lalit
*  SR_NO		  :
*  Date			  :
****************************************************************************************************/
bool CWardWizUpdateManager::SetAllALtFilesAttributeToNorml()
{
	bool bReturn = false;

	__try
	{
		TCHAR	szProgPath[512] = { 0 };
		TCHAR	szQuarntinePath[512] = { 0 };

		swprintf_s(szProgPath, _countof(szProgPath), L"%s", theApp.m_szAVPath);

		swprintf_s(szQuarntinePath, _countof(szQuarntinePath), L"%s%s", theApp.m_szAVPath, L"QUARANTINE");

		bReturn = SetAllFileAttributeToNormal(szProgPath, szQuarntinePath, false);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### CVibraniumUpdateManager::SetAllFileAttributeToNormal::Exception", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SetAllALtFilesAttributeToNorml()
*  Description    : Set all files attributes to normal. Flag renamefile = false means setattributes to normal
and renamefile = true , rename file.
*  Author Name    : Lalit
**  SR_NO		  :
*  Date			  :
****************************************************************************************************/
bool CWardWizUpdateManager::SetAllFileAttributeToNormal(LPCTSTR lpFolPath, LPCTSTR lpQuarntinePath, bool bRenameFile)
{
	try
	{
		CFileFind finder;

		CString strWildcard(lpFolPath);
		TCHAR  szAppPath[MAX_PATH] = { 0 };
		TCHAR  szQuarntinePath[MAX_PATH] = { 0 };
		CString csQuarntinePath(lpQuarntinePath);

		if (!lpQuarntinePath || !lpFolPath)
		{
			return false;
		}

		swprintf_s(szQuarntinePath, _countof(szQuarntinePath), L"%s", csQuarntinePath);
		//	csQuarntinePath = szQuarntinePath;

		if (strWildcard[strWildcard.GetLength() - 1] != '\\')
			strWildcard += _T("\\*.*");
		else
			strWildcard += _T("*.*");

		SetFileAttributes(lpFolPath, FILE_ATTRIBUTE_NORMAL);

		BOOL bWorking = finder.FindFile(strWildcard);


		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			TCHAR	szPath[512] = { 0 };

			wsprintf(szPath, L"%s", finder.GetFilePath());

			if (finder.IsDirectory())
			{
				SetFileAttributes(szPath, FILE_ATTRIBUTE_NORMAL);
				if (_wcsicmp(szPath, szQuarntinePath) == 0x00)
				{
					continue;
				}
				else
				{
					if (!bRenameFile)
					{
						SetAllFileAttributeToNormal(szPath, lpQuarntinePath, false);
					}
					else
					{
						SetAllFileAttributeToNormal(szPath, lpQuarntinePath, true);
					}
				}
			}
			if (!bRenameFile)
			{
				SetFileAttributes(szPath, FILE_ATTRIBUTE_NORMAL);
			}
			else
			{
				if (!CheckForRenameRequired(szPath))
				{
					continue;
				}

			}

		}

		finder.Close();


	}
	catch (...)
	{
		AddLogEntry(L"Exception in CVibraniumUpdateManager::SetAllFileAttributeToNormal", 0, 0, true, SECONDLEVEL);
		return false;
	}

	return true;
}

/***************************************************************************************************
*  Function Name  : CheckForRenameRequired()
*  Description    : Check for file rename required
*  Author Name    : Neha
**  SR_NO		  :
*  Date			  :
****************************************************************************************************/
bool CWardWizUpdateManager::CheckForRenameRequired(CString csRenameFile)
{
	try
	{
		int iPos = 0;
		//csNewFilePath = with _WWIZALT and csOldFilePath = original name
		CString csNewFilePath, csOldFilePath;

		if (csRenameFile.Right(8) == L"_WWIZALT")
		{
			csNewFilePath.Format(L"%s", csRenameFile);
			csOldFilePath = csRenameFile.Tokenize(L"_", iPos);
			if (!PathFileExists(csOldFilePath))
			{
				if (PathFileExists(csNewFilePath))
				{
					_wrename(csNewFilePath, csOldFilePath);
					return true;
				}
			}
		}
		else
		{
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CVibraniumUpdateManager::CheckForRenameRequired", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : DeleteAllALtFilesFromProgramData
*  Description    : WrapperFunction to delete _wwzalt file from programData which is read only.
*  Author Name    : Vilas & lalit kumawat
*  SR_NO		  :
*  Date           : 5-15-2015
****************************************************************************************************/
bool CWardWizUpdateManager::DeleteAllALtFilesFromProgramData()
{
	bool bReturn = false;

	__try
	{
		TCHAR	szAllUserProgPath[512] = { 0 };

		swprintf_s(szAllUserProgPath, _countof(szAllUserProgPath), L"%s\\%s", m_szAllUserPath, m_csAppFolderName);

		bReturn = DeleteAllALtFilesFromProgramDataSEH(szAllUserProgPath);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### CVibraniumUpdateManager::DeleteAllALtFilesFromProgramData::Exception", 0, 0, true, SECONDLEVEL);
	}

	return bReturn;
}

/***************************************************************************************************
*  Function Name  : DeleteAllALtFilesFromProgramDataSEH
*  Description    : To delete _wwzalt file from programData which is read only.
*  Author Name    : Vilas & lalit kumawat
*  SR_NO		  :
*  Date           : 5-15-2015
****************************************************************************************************/
bool CWardWizUpdateManager::DeleteAllALtFilesFromProgramDataSEH(LPCTSTR lpFolPath)
{
	try
	{
		CFileFind finder;

		CString strWildcard(lpFolPath);

		if (strWildcard[strWildcard.GetLength() - 1] != '\\')
			strWildcard += _T("\\*.*");
		else
			strWildcard += _T("*.*");


		BOOL bWorking = finder.FindFile(strWildcard);

		while (bWorking)
		{
			bWorking = finder.FindNextFile();
			if (finder.IsDots())
				continue;

			TCHAR	szPath[512] = { 0 };

			wsprintf(szPath, L"%s", finder.GetFilePath());
			if (finder.IsDirectory())
				continue;

			if (finder.IsReadOnly())
				DeleteFileForcefully(szPath);
		}

		finder.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::DeleteAllALtFilesFromProgramDataSEH", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : SetThreadPoolStatus
*  Description    : To set the Thread pool status pause(true) resume(false).
*  Author Name    : Amol Jaware
*  Date           : 19-Feb-2018
****************************************************************************************************/
void CWardWizUpdateManager::SetThreadPoolStatus(bool bThreadPoolStatus)
{
	try
	{
		if (m_pDownloadController != NULL)
		{
			m_pDownloadController->SetThreadPoolStatus(bThreadPoolStatus);
		}

		if (m_pDownloadController4UpdtMgr != NULL)
		{
			m_pDownloadController4UpdtMgr->SetThreadPoolStatus(bThreadPoolStatus);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception inside CVibraniumUpdateManager::SetThreadPoolStatus", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CreateDir4ProgFiles
*  Description    : This function creates required folders in program files.
*  Author Name    : Amol Jaware
*  Date			  :	09-May-2018
****************************************************************************************************/
void CWardWizUpdateManager::CreateDir4ProgFiles()
{
	try
	{
		TCHAR szTargetPath[MAX_PATH] = { 0 };
		TCHAR szDBTypeDirectory[MAX_PATH] = { 0 };
		CString csElitDirectory = L"Elite";

		if (CreateDirwardwizEPSPublish() == 0)
		{
			AddLogEntry(L"### VibraniumEPSPublish directory created successfully inside StartALUpdtProcess4UdtMgr.", 0, 0, true, ZEROLEVEL);
		}
		else
		{
			AddLogEntry(L"### Failed to create VibraniumEPSPublish directory inside StartALUpdtProcess4UdtMgr.", 0, 0, true, ZEROLEVEL);
		}

		_stprintf_s(szTargetPath, MAX_PATH, L"%s\\%s", m_cswardwizEPSPublishPath, csElitDirectory);
		if (!PathFileExists(szTargetPath)) //m_csWWizProgDataEPSPublishPath
			CreateDirectoryFocefully4EPSHierarchy(szTargetPath); //Elite

		_stprintf_s(szDBTypeDirectory, MAX_PATH, L"%s\\32", szTargetPath);
		if (!PathFileExists(szDBTypeDirectory))
			CreateDirectoryFocefully4EPSHierarchy(szDBTypeDirectory); //32
		szDBTypeDirectory[0] = { 0 };

		_stprintf_s(szDBTypeDirectory, MAX_PATH, L"%s\\64", szTargetPath);
		if (!PathFileExists(szDBTypeDirectory))
			CreateDirectoryFocefully4EPSHierarchy(szDBTypeDirectory); //64

		szDBTypeDirectory[0] = { 0 };
		_stprintf_s(szDBTypeDirectory, MAX_PATH, L"%s\\Common", szTargetPath);
		if (!PathFileExists(szDBTypeDirectory))
			CreateDirectoryFocefully4EPSHierarchy(szDBTypeDirectory); //Common

		szDBTypeDirectory[0] = { 0 };
		_stprintf_s(szDBTypeDirectory, MAX_PATH, L"%s\\CommonDB", szTargetPath);
		if (!PathFileExists(szDBTypeDirectory))
			CreateDirectoryFocefully4EPSHierarchy(szDBTypeDirectory); //CommonDB
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::CreateDir4ProgFiles", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CreateDir4ProgData
*  Description    : This function creates required folders in program.
*  Author Name    : Amol Jaware
*  Date			  :	09-May-2018
****************************************************************************************************/
void CWardWizUpdateManager::CreateDir4ProgData()
{
	try
	{
		TCHAR szTargetPath[MAX_PATH] = { 0 };
		TCHAR szDBTypeDirectory[MAX_PATH] = { 0 };
		CString csElitDirectory = L"Elite";

		if (CreateDirProgDataWardWizEPS() == 0)
		{
			AddLogEntry(L"### VibraniumEPS directory created successfully in program data inside StartALUpdtProcess4UdtMgr.", 0, 0, true, ZEROLEVEL);
		}
		else
		{
			AddLogEntry(L"### Failed to create VibraniumEPS directory in program data inside StartALUpdtProcess4UdtMgr.", 0, 0, true, ZEROLEVEL);
		}

		_stprintf_s(szTargetPath, MAX_PATH, L"%s\\%s", m_csWWizProgDataEPSPublishPath, csElitDirectory);
		if (!PathFileExists(szTargetPath)) //m_csWWizProgDataEPSPublishPath
			CreateDirectoryFocefully4EPSHierarchy(szTargetPath); //Elite

		_stprintf_s(szDBTypeDirectory, MAX_PATH, L"%s\\32", szTargetPath);
		if (!PathFileExists(szDBTypeDirectory))
			CreateDirectoryFocefully4EPSHierarchy(szDBTypeDirectory); //32
		szDBTypeDirectory[0] = { 0 };

		_stprintf_s(szDBTypeDirectory, MAX_PATH, L"%s\\64", szTargetPath);
		if (!PathFileExists(szDBTypeDirectory))
			CreateDirectoryFocefully4EPSHierarchy(szDBTypeDirectory); //64

		szDBTypeDirectory[0] = { 0 };
		_stprintf_s(szDBTypeDirectory, MAX_PATH, L"%s\\Common", szTargetPath);
		if (!PathFileExists(szDBTypeDirectory))
			CreateDirectoryFocefully4EPSHierarchy(szDBTypeDirectory); //Common

		szDBTypeDirectory[0] = { 0 };
		_stprintf_s(szDBTypeDirectory, MAX_PATH, L"%s\\CommonDB", szTargetPath);
		if (!PathFileExists(szDBTypeDirectory))
			CreateDirectoryFocefully4EPSHierarchy(szDBTypeDirectory); //CommonDB
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::CreateDir4ProgData", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CreateDirwardwizEPSPublish()
*  Description    : This function will create the wardwizEPSPublis directory inside c:\prog files\wardwiz
*  Author Name    : Amol Jaware
*  SR_NO		  :
*  Date			  : 10-April-2018
****************************************************************************************************/
DWORD CWardWizUpdateManager::CreateDirwardwizEPSPublish()
{
	try
	{
		TCHAR szWWEPSPublishDir[MAX_PATH] = { 0 };
		TCHAR szWWEPSUpdtPath[MAX_PATH] = { 0 };
		CString csWardwizPath = GetAVPathFromRegistry();
		CString csWardWizEPS_Updates = L"WardWizEPS";

		_stprintf_s(szWWEPSUpdtPath, MAX_PATH, L"%s%s", csWardwizPath, szwardwizEPSPublish);
		_stprintf_s(szWWEPSPublishDir, MAX_PATH, L"%sWardWizDev", csWardwizPath);

		do
		{
			if (!PathFileExists(szWWEPSPublishDir))
			{
				CreateDirectory(szWWEPSPublishDir, NULL);
				if (!PathFileExists(szWWEPSPublishDir))
					return 1;

				_wmkdir(szWWEPSPublishDir);
				if (!PathFileExists(szWWEPSPublishDir))
				{
					return 1;
				}
			}

			if (PathFileExists(szWWEPSPublishDir))
				_stprintf_s(szWWEPSPublishDir, MAX_PATH, L"%s\\%s", szWWEPSPublishDir, csWardWizEPS_Updates); //szwardwizEPSPublish
			else
				return 1;

			csWardWizEPS_Updates = L"WardWizUpdates";

		} while (!PathFileExists(szWWEPSUpdtPath));

		if (PathFileExists(szWWEPSUpdtPath))
			m_cswardwizEPSPublishPath = szWWEPSUpdtPath;
		else
		{
			m_cswardwizEPSPublishPath = L"";
			AddLogEntry(L"### Failed to create path inside CVibraniumUpdateManager::CreateDirVibraniumEPSPublish : %s", szWWEPSPublishDir, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::CreateDirwardwizEPSPublish", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
*  Function Name  : CreateDirProgDataWardWizEPS()
*  Description    : This function is used create directories inside prog data folder.
*  Author Name    : Amol Jaware
*  Date			  :	09-May-2018
****************************************************************************************************/
DWORD CWardWizUpdateManager::CreateDirProgDataWardWizEPS()
{
	try
	{
		TCHAR szWWEPSPublishDir[MAX_PATH] = { 0 };
		TCHAR szWWEPSUpdtPath[MAX_PATH] = { 0 };
		CString csWardWizEPS_Updates = L"WardWizEPS";

		_stprintf_s(szWWEPSUpdtPath, MAX_PATH, L"%s\\%s\\%s", m_szAllUserPath, m_csAppFolderName, szwardwizEPSPublish);
		_stprintf_s(szWWEPSPublishDir, MAX_PATH, L"%s\\%s\\WardWizDev", m_szAllUserPath, m_csAppFolderName); //szwardwizEPSPublish
		do
		{
			if (!PathFileExists(szWWEPSPublishDir))
			{
				CreateDirectory(szWWEPSPublishDir, NULL);
				if (!PathFileExists(szWWEPSPublishDir))
					return 1;

				_wmkdir(szWWEPSPublishDir);
				if (!PathFileExists(szWWEPSPublishDir))
				{
					return 1;
				}
			}

			if (PathFileExists(szWWEPSPublishDir))
				_stprintf_s(szWWEPSPublishDir, MAX_PATH, L"%s\\%s", szWWEPSPublishDir, csWardWizEPS_Updates); //szwardwizEPSPublish
			else
				return 1;

			csWardWizEPS_Updates = L"WardWizUpdates";
		} while (!PathFileExists(szWWEPSUpdtPath));

		if (PathFileExists(szWWEPSUpdtPath))
			m_csWWizProgDataEPSPublishPath = szWWEPSUpdtPath;
		else
		{
			m_csWWizProgDataEPSPublishPath = L"";
			AddLogEntry(L"### Failed to create path inside CVibraniumUpdateManager::CreateDirProgDataWardWizEPS : %s", szWWEPSPublishDir, 0, true, SECONDLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumUpdateManager::CreateDirVibraniumEPSPublish", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
*  Function Name  : MergeUpdates2ProductFolder()
*  Description    : This function is used to move downloaded files into prog files wardwiz folder.
*  Author Name    : Amol Jaware
*  Date			  :	08-May-2018
****************************************************************************************************/
bool CWardWizUpdateManager::MergeUpdates2ProductFolder()
{
	CString	szFilePath;
	TCHAR	szSourceFilePath[512] = { 0 };
	TCHAR	szDestFilePath[512] = { 0 };
	TCHAR	szTemp[512] = { 0 };
	TCHAR	szReplaceDestination[512] = { 0 };
	bool	bCopied = false;
	int iStartPos = 0;
	TCHAR szAVPath[512] = { 0 };
	unsigned int iItemCount = 0;
	CString csFoldName = L"", csFileName = L"";
	DWORD dwCount = 0x00;

	try
	{
		CreateDir4ProgFiles();
		CreateDir4ProgData();
		dwCount = static_cast<DWORD>(m_vUrlZipLists.size());

		for (; iItemCount < m_vUrlZipLists.size(); iItemCount++) //for zip files downloading...
		{
			
			SetDownloadPercentage(SETMESSAGE, UPDATINGFILES, (iItemCount + 1), dwCount, this);

			ZeroMemory(szSourceFilePath, sizeof(szSourceFilePath));
			ZeroMemory(szDestFilePath, sizeof(szDestFilePath));
			csFoldName = m_vUrlZipLists[iItemCount];
			csFileName = csFoldName.Right(csFoldName.GetLength() - csFoldName.ReverseFind(L'/') - 1);

			int iFound = csFoldName.ReverseFind(L'/');
			csFoldName = csFoldName.Left(iFound);
			iFound = csFoldName.ReverseFind(L'/');
			csFoldName.Trim();
			csFoldName = csFoldName.Mid(csFoldName.ReverseFind(L'/') + 1, iFound);
			csFoldName.Trim();

			if (csFoldName == L"32")
			{
				swprintf_s(szSourceFilePath, _countof(szSourceFilePath), L"%s\\Elite\\%s\\%s", m_csWWizProgDataEPSPublishPath, csFoldName, csFileName);
				AddLogEntry(L">>> SourcePath :: %s", szSourceFilePath, 0, true, ZEROLEVEL);
				swprintf_s(szDestFilePath, _countof(szDestFilePath), L"%s\\Elite\\%s\\%s", m_cswardwizEPSPublishPath, csFoldName, csFileName);
				AddLogEntry(L">>> Destination Path :: %s", szDestFilePath, 0, true, ZEROLEVEL);

			}
			else if (csFoldName == L"64")
			{
				swprintf_s(szSourceFilePath, _countof(szSourceFilePath), L"%s\\Elite\\%s\\%s", m_csWWizProgDataEPSPublishPath, csFoldName, csFileName);
				AddLogEntry(L">>> SourcePath :: %s", szSourceFilePath, 0, true, ZEROLEVEL);
				swprintf_s(szDestFilePath, _countof(szDestFilePath), L"%s\\Elite\\%s\\%s", m_cswardwizEPSPublishPath, csFoldName, csFileName);
				AddLogEntry(L">>> Destination Path :: %s", szDestFilePath, 0, true, ZEROLEVEL);

			}
			else if (csFoldName == L"Common")
			{
				swprintf_s(szSourceFilePath, _countof(szSourceFilePath), L"%s\\Elite\\%s\\%s", m_csWWizProgDataEPSPublishPath, csFoldName, csFileName);
				AddLogEntry(L">>> SourcePath :: %s", szSourceFilePath, 0, true, ZEROLEVEL);
				swprintf_s(szDestFilePath, _countof(szDestFilePath), L"%s\\Elite\\%s\\%s", m_cswardwizEPSPublishPath, csFoldName, csFileName);
				AddLogEntry(L">>> Destination Path :: %s", szDestFilePath, 0, true, ZEROLEVEL);

			}
			else if (csFoldName == L"CommonDB")
			{
				swprintf_s(szSourceFilePath, _countof(szSourceFilePath), L"%s\\Elite\\%s\\%s", m_csWWizProgDataEPSPublishPath, csFoldName, csFileName);
				AddLogEntry(L">>> SourcePath :: %s", szSourceFilePath, 0, true, ZEROLEVEL);
				swprintf_s(szDestFilePath, _countof(szDestFilePath), L"%s\\Elite\\%s\\%s", m_cswardwizEPSPublishPath, csFoldName, csFileName);
				AddLogEntry(L">>> Destination Path :: %s", szDestFilePath, 0, true, ZEROLEVEL);
			}

			if (CopyFile(szSourceFilePath, szDestFilePath, FALSE))
			{
				bCopied = true;
			}
			else
			{
				AddLogEntry(L">>> Failed to copy file:: %s", szSourceFilePath, 0, true, ZEROLEVEL);
				bCopied = false;
				continue;
			}
			SetFileAttributes(szSourceFilePath, FILE_ATTRIBUTE_NORMAL);
			if (DeleteFile(szSourceFilePath) == FALSE)
			{
				AddLogEntry(L">>> DeleteFile Failed :: %s", szDestFilePath, 0, true, ZEROLEVEL);
			}
			Sleep(10);
		}
	}
	catch (...)
	{
		bCopied = false;
		AddLogEntry(L"### VibraniumALUSrv::ReplaceDownloadedFiles::Exception", 0, 0, true, SECONDLEVEL);
	}

	return bCopied;
}