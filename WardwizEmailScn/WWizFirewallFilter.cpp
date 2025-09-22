/**********************************************************************************************************
*		Program Name          : WWizFirewallFilter.cpp
*		Description           : App class which has functionality for Firewall as well as Email Scan.
*		Author Name			  : Ram Shelke
*		Date Of Creation      : 29th APR 2019
*		Version No            : 3.5.0.1
*		Modification Log      :
***********************************************************************************************************/
#include "stdafx.h"
#include "WWizFirewallFilter.h"
#include "WardWizDatabaseInterface.h"
#include "iTinRegWrapper.h"
#include "WWizSettingsWrapper.h"
#include "ISpyCommunicator.h"

DWORD WINAPI PortScanWatchDogThread(LPVOID lpParam);

/***********************************************************************************************
*  Function Name  : CWardwizEmailScnApp
*  Description    : Constructor
*  Author Name    : Ram Shelke
*  Date           : 29-MAR-2019
*************************************************************************************************/
CWWizFirewallFilter::CWWizFirewallFilter():
m_hCommDevice(INVALID_HANDLE_VALUE),
m_bStopPortScan(false),
m_bPortScanDetected(false)
{
	m_hCommEvent = INVALID_HANDLE_VALUE;
	m_hThread = INVALID_HANDLE_VALUE;
	ReLoadBlockedApplicationList();
}

/***********************************************************************************************
*  Function Name  : ~CWardwizEmailScnApp
*  Description    : Dest'r
*  Author Name    : Ram Shelke
*  Date           : 29-MAR-2019
*************************************************************************************************/
CWWizFirewallFilter::~CWWizFirewallFilter()
{
	if (m_hCommDevice != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hCommDevice); 
		m_hCommDevice = NULL;
	}

	if (m_hCommEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hCommEvent);
		m_hCommEvent = NULL;
	}

	StopPortScanProtection();

	OnCloseDevice();
	if (m_hThread != INVALID_HANDLE_VALUE)
	{
		SuspendThread(m_hThread);
		TerminateThread(m_hThread, 0x00);
		m_hThread = NULL;
	}
}

/***********************************************************************************************
*  Function Name  : LoadBlockedApplicationList
*  Description    : Function to load blocked application list
*  Author Name    : Ram Shelke
*  Date           : 29-MAR-2019
*************************************************************************************************/
bool CWWizFirewallFilter::LoadBlockedApplicationList()
{
	try
	{
		TCHAR	szPath[512] = { 0 };
		DWORD	dwSize = sizeof(szPath);
		CITinRegWrapper	objReg;
		CWardWizSQLiteDatabase dbSQlite;
		CString csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();
		objReg.GetRegistryValueData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"AppFolder", szPath, dwSize);
		CString csWardWizModulePath = szPath;
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBFIREWALL.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return 0;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();

		CWardwizSQLiteTable qResult = dbSQlite.GetTable("SELECT * FROM WWIZ_FIREWALL_APP_RULE_CONFIG WHERE ACCESS = '1';");

		DWORD dwRows = qResult.GetNumRows();
		if (dwRows != 0)
		{
			for (DWORD dwIndex = 0x00; dwIndex < dwRows; dwIndex++)
			{
				qResult.SetRow(dwIndex);
				if (qResult.GetFieldIsNull(0))
				{
					continue;
				}

				char szPath[MAX_PATH] = { 0 };
				strcpy_s(szPath, qResult.GetFieldValue(1)); //Application path
				_strlwr(szPath);
				m_vBlockedAppList.push_back(szPath);
			}
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::LoadBlockedApplicationList", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : ReLoadBlockedApplicationList
*  Description    : Function to Re-load blocked application list
*  Author Name    : Ram Shelke
*  Date           : 29-MAR-2019
*************************************************************************************************/
bool CWWizFirewallFilter::ReLoadBlockedApplicationList()
{
	try
	{
		m_vBlockedAppList.clear();
		LoadBlockedApplicationList();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::ReLoadBlockedApplicationList", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***********************************************************************************************
*  Function Name  : StringToWString
*  Description    : Function which converts ANSI string to WIDE CHAR string
*  Author Name    : Ram Shelke
*  Date           : 29-MAR-2019
*************************************************************************************************/
std::wstring StringToWString(const std::string& s)
{
	std::wstring temp(s.length(), L' ');
	std::copy(s.begin(), s.end(), temp.begin());
	return temp;
}

/***************************************************************************************************
*  Function Name  : ISApplicationBlocked
*  Description    : Function to check is application blocked.
*  Author Name    : Ram Shelke
*  Date			  :	20 Mar 2015
****************************************************************************************************/
bool CWWizFirewallFilter::ISApplicationBlocked(std::wstring strProcessName)
{
	bool bFound = false;
	try
	{
		//system process can not be blocked.
		if (strProcessName.length() == 0x00)
			return false;

		for (std::vector<std::string>::iterator it = m_vBlockedAppList.begin(); it != m_vBlockedAppList.end(); ++it)
		{
			std::wstring strProcessPath = StringToWString(*it);

			if (_tcscmp(strProcessPath.c_str(), strProcessName.c_str()) == 0)
			{
				bFound = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::ISApplicationBlocked", 0, 0, true, SECONDLEVEL);
	}
	return bFound;
}


/***************************************************************************************************
*  Function Name  : OnOpenDevice
*  Description    : Function to open device handle to start communication with WFP drivers
*  Author Name    : Ram Shelke
*  Date           : 30 MAY 2019
****************************************************************************************************/
void CWWizFirewallFilter::OnOpenDevice()
{
	try
	{
		m_hCommDevice = nf_getDeviceHandle();
		if (m_hCommDevice == INVALID_HANDLE_VALUE)
		{
			AddLogEntry(L"### CVibraniumFirewallFilter::OnOpenDevice, Please install the device driver and start it firstly!", 0, 0, true, SECONDLEVEL);
			return;
		}

		DWORD dwReturn;

		if (m_hCommEvent == INVALID_HANDLE_VALUE)
		{
			m_hCommEvent = CreateEvent(NULL, false, false, NULL);

			DeviceIoControl(m_hCommDevice,
				IO_REFERENCE_EVENT,
				(LPVOID)m_hCommEvent,
				0,
				NULL,
				0,
				&dwReturn,
				NULL);
		}

		m_bStopPortScan = false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::OnOpenDevice", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnCloseDevice
*  Description    : Function to close communication with WFP drivers
*  Author Name    : Ram Shelke
*  Date           : 30 MAY 2019
****************************************************************************************************/
void CWWizFirewallFilter::OnCloseDevice()
{
	try
	{
		DWORD dwReturn;
		if (m_hCommDevice)
		{
			if (m_hCommEvent)
			{
				DeviceIoControl(m_hCommDevice,
					IO_DEREFERENCE_EVENT,
					NULL,
					0,
					NULL,
					0,
					&dwReturn,
					NULL);
				CloseHandle(m_hCommEvent);
			}
			CloseHandle(m_hCommDevice);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::OnCloseDevice", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnCloseDevice
*  Description    : Function to clear event which is being used to signal for Port scan
*  Author Name    : Ram Shelke
*  Date           : 30 MAY 2019
****************************************************************************************************/
void CWWizFirewallFilter::OnClearEvent()
{
	try
	{
		DWORD	dwReturn;
		if (m_hCommEvent)
		{
			DeviceIoControl(m_hCommDevice,
				IO_CLEAR_EVENT,
				NULL,
				0,
				NULL,
				0,
				&dwReturn,
				NULL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::OnClearEvent", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : OnSetEvent
*  Description    : Function to set event which is being used to signal for Port scan
*  Author Name    : Ram Shelke
*  Date           : 30 MAY 2019
****************************************************************************************************/
void CWWizFirewallFilter::OnSetEvent()
{
	try
	{
		DWORD	dwReturn;
		if (m_hCommEvent)
		{
			DeviceIoControl(m_hCommDevice,
				IO_SET_EVENT,
				NULL,
				0,
				NULL,
				0,
				&dwReturn,
				NULL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::OnSetEvent", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : StartPortScanProtection
*  Description    : Function to Start Port scan protection thread.
*  Author Name    : Ram Shelke
*  Date           : 30 MAY 2019
****************************************************************************************************/
bool CWWizFirewallFilter::StartPortScanProtection()
{
	bool bReturn = false;
	try
	{
		CString csRegKeyPath;
		csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();

		DWORD dwPortScanState = 0x00;
		CITinRegWrapper objReg;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwPortScanProt", dwPortScanState) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwPortScanProt in CVibraniumFirewallFilter::StopPortScanProtection KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);;
		}
		if (dwPortScanState == 0x00)
		{
			return false;
		}

		m_bStopPortScan = false;
		
		if (m_hThread == INVALID_HANDLE_VALUE)
		{
			OnOpenDevice();
			m_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)PortScanWatchDogThread,
				this, 0, 0);
		}

		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::StartPortScanProtection", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : StopPortScanProtection
*  Description    : Function to Stop Port scan protection thread.
*  Author Name    : Ram Shelke
*  Date           : 30 MAY 2019
****************************************************************************************************/
bool CWWizFirewallFilter::StopPortScanProtection()
{
	bool bReturn = false;
	try
	{
		CString csRegKeyPath;
		csRegKeyPath = CWWizSettingsWrapper::GetProductRegistryKey();

		DWORD dwPortScanState = 0x00;
		CITinRegWrapper objReg;
		if (objReg.GetRegistryDWORDData(HKEY_LOCAL_MACHINE, csRegKeyPath.GetBuffer(), L"dwPortScanProt", dwPortScanState) != 0x00)
		{
			AddLogEntry(L"### Failed to get Registry Entry for dwPortScanProt in CVibraniumFirewallFilter::StopPortScanProtection KeyPath: %s", csRegKeyPath, 0, true, SECONDLEVEL);;
		}
		if (dwPortScanState == 0x00)
		{
			return false;
		}

		m_bStopPortScan = true;
	
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallFilter::StartPortScanProtection", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}


/***************************************************************************************************
*  Function Name  : PortScanWatchDogThread
*  Description    : Thread fucntion to monitor Port scan protection
*  Author Name    : Ram Shelke
*  Date           : 30 MAY 2019
****************************************************************************************************/
DWORD WINAPI PortScanWatchDogThread(LPVOID lpParam)
{
	if (!lpParam)
		return 0;

	__try
	{
		CWWizFirewallFilter *pThis = (CWWizFirewallFilter*)lpParam;

		while (true)
		{
			if (!(pThis->m_bStopPortScan))
			{
				if (pThis->m_hCommEvent)
				{
					DWORD dwWaitResult = WaitForSingleObject(pThis->m_hCommEvent, INFINITE);
					switch (dwWaitResult)
					{
					case WAIT_OBJECT_0:
						AddLogEntry(L">>> PORT SCAN BLOCKED", 0, 0, true, SECONDLEVEL);
						pThis->OnClearEvent();
						pThis->m_bPortScanDetected = true;
						pThis->SendParCtrlMessage2Tray(SHOW_PORT_SCAN_POPUP);
						break;
					}
				}
			}

			if (pThis->m_bPortScanDetected)
			{
				pThis->m_bPortScanDetected = false;
				pThis->InsertIntoFWReports();
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in PortScanWatchDogThread", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  : SendParCtrlMessage2Tray
*  Description    : To send message to tray
*  Author Name    : Jeena Mariam Saji
*  Date			  :	10 Aug 2018
****************************************************************************************************/
bool CWWizFirewallFilter::SendParCtrlMessage2Tray(int iRequest, bool bWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = iRequest;

		CISpyCommunicator objCom(TRAY_SERVER, true, 0x03);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			Sleep(30);
			if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to send data in CVibraniumFirewallApp::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}

		if (bWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to ReadData in CVibraniumFirewallApp::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumFirewallApp::SendMessage2Tray", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : InsertIntoFWReports
*  Description    : To insert data in Firewall reports when port scan alert notification generated
*  Author Name    : Kunal Waghmare
*  Date			  :	31 Oct 2019
****************************************************************************************************/
bool CWWizFirewallFilter::InsertIntoFWReports()
{
	try
	{
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csAppRulesDB = L"";
		csAppRulesDB.Format(L"%sVBALLREPORTS.DB", csWardWizModulePath);
		CT2A dbPath(csAppRulesDB, CP_UTF8);
		CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);
		CStringA csInsertQuery = "";
		csInsertQuery.AppendFormat("INSERT INTO Wardwiz_FirewallDetails(db_FirewallDate, db_FirewallTime, db_FW_LocalIP) VALUES (Date('now'), Datetime('now','localtime'), '');");
		objSqlDb.Open();
		objSqlDb.ExecDML(csInsertQuery);
		objSqlDb.Close();
	}
	catch (...)
	{
		AddLogEntryEx(SECONDLEVEL, L"### Exception in CVibraniumFirewallFilter::InsertIntoFWReports");
		return false;
	}
	return true;
}