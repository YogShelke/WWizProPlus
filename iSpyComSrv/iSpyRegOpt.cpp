/**********************************************************************************************************            	  Program Name          : WardWiz Registry optimization
	  Description           : This class used for Registry Optimization.
	  Author Name			: Ramkrushna Shelke                                                                        	  Date Of Creation      : 17 Jan 2014
	  Version No            : 1.0.0.4
	  Special Logic Used    : 
	  Modification Log      : 
***********************************************************************************************************/
#include "stdafx.h"
#include <windows.h>
#include "iSpyRegOpt.h"
#include "ISpyCommunicator.h"

iSpySrvMgmt_RegOpt	RegOptObj ;

/***********************************************************************************************
  Function Name  : CISpyRegOpt
  Description    : Const'
  Author Name    : Ramkrushna Shelke
  Date           : 17 Jan 2014
***********************************************************************************************/
CISpyRegOpt::CISpyRegOpt()
{
	m_hThread_StartRegScan = NULL;
	m_bIsRegOptInProgress = false;
}

/***********************************************************************************************
  Function Name  : ~CISpyRegOpt
  Description    : Dest'
  Author Name    : Ramkrushna Shelke
  Date           : 17 Jan 2014
***********************************************************************************************/
CISpyRegOpt::~CISpyRegOpt() 
{
	if(m_hThread_StartRegScan != NULL)
	{
		CloseHandle(m_hThread_StartRegScan);
		m_hThread_StartRegScan = NULL ;
	}
}

/***********************************************************************************************
  Function Name  : GetScanOptionsFromDWORD
  Description    : Function which takes the parameters sent by client.
  SR.NO			 : WRDWIZCOMMSRV_13
  Author Name    : Ramkrushna Shelke
  Date           : 17 Jan 2014
***********************************************************************************************/
void GetScanOptionsFromDWORD( DWORD dwRegScanOpt, LPREGOPTSCANOPTIONS lpRegOpt)
{
	DWORD	dwValue = 0x00 ;
	bool	bOpt = false ;

	for(DWORD i=0x00; i<0x12; i++ )
	{
		bOpt = false ;
		dwValue = dwRegScanOpt>>i ;
		if( (dwValue&0x01) == 0x01 )
			bOpt = true ;
		
		switch( i )
		{
		case 0:
			lpRegOpt->bActiveX = bOpt ;
			break ;
		case 1:
			lpRegOpt->bUninstall = bOpt ;
			break ;
		case 2:
			lpRegOpt->bFont = bOpt ;
			break ;
		case 3:
			lpRegOpt->bSharedLibraries = bOpt ;
			break ;
		case 4:
			lpRegOpt->bApplicationPaths = bOpt ;
			break ;
		case 5:
			lpRegOpt->bHelpFiles = bOpt ;
			break ;
		case 6:
			lpRegOpt->bStartup = bOpt ;
			break ;
		case 7:
			lpRegOpt->bServices = bOpt ;
			break ;
		case 8:
			lpRegOpt->bExtensions = bOpt ;
			break ;
		case 9:
			lpRegOpt->bRootKit = bOpt ;
			break ;
		case 10:
			lpRegOpt->bRogueApplications = bOpt ;
			break ;
		case 11:
			lpRegOpt->bWorm = bOpt ;
			break ;
		case 12:
			lpRegOpt->bSpywares = bOpt ;
			break ;
		case 13:
			lpRegOpt->bAdwares = bOpt ;
			break ;
		case 14:
			lpRegOpt->bKeyLogger = bOpt ;
			break ;
		case 15:
			lpRegOpt->bBHO = bOpt ;
			break ;
		case 16:
			lpRegOpt->bExplorer = bOpt ;
			break ;
		case 17:
			lpRegOpt->bIExplorer = bOpt ;
			break ;
		}


	}
}

/***********************************************************************************************
  Function Name  : Thread_StartRegistryOptimizer
  Description    : Function which takes the parameters sent by client.
  SR.NO			 : WRDWIZCOMMSRV_64
  Author Name    : Ramkrushna Shelke
  Date           : 17 Jan 2014
***********************************************************************************************/
DWORD WINAPI Thread_StartRegistryOptimizer(LPVOID lpParam)
{

	DWORD	dwScanOptions = *( (DWORD * )lpParam ) ;

	ZeroMemory(&RegOptObj.RegOpt_ScanOpt, sizeof(RegOptObj.RegOpt_ScanOpt) ) ;
	GetScanOptionsFromDWORD( dwScanOptions, &RegOptObj.RegOpt_ScanOpt ) ;
	RegOptObj.ScanAndRepairRegistryEntries( &RegOptObj.RegOpt_ScanOpt ) ;

	//if( m_hThread_StartRegScan )
	//	CloseHandle( m_hThread_StartRegScan ) ;
	//m_hThread_StartRegScan = NULL ;
			
	return 0 ;
}

/***********************************************************************************************
  Function Name  : Thread_StartRegistryOptimizer
  Description    : Function which takes the parameters sent by client.
  SR.NO			 : WRDWIZCOMMSRV_64
  Author Name    : Ramkrushna Shelke
  Date           : 17 Jan 2014
***********************************************************************************************/
bool CISpyRegOpt::StartRegistryOptimizer(DWORD	dwScanOptions)
{
	try
	{
		if (!m_hThread_StartRegScan)
		{
			m_hThread_StartRegScan = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Thread_StartRegistryOptimizer,
				(LPVOID)&dwScanOptions, 0, 0);

			//Varada Ikhar, Date: 23/04/2015, Implementing RegOptimization thread pause-resume.
			if (m_hThread_StartRegScan)
			{
				m_bIsRegOptInProgress = true;
			}

			Sleep(500);
		}

		::WaitForSingleObject(m_hThread_StartRegScan, INFINITE);

		if (m_hThread_StartRegScan)
		{
			//Varada Ikhar, Date: 23/04/2015, Implementing RegOptimization thread pause-resume.
			m_bIsRegOptInProgress = false;

			CloseHandle(m_hThread_StartRegScan);
			m_hThread_StartRegScan = NULL;
		}

		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(szPipeData));
		szPipeData.iMessageInfo = REGISTRY_SCAN_FINISHED;

		CISpyCommunicator objCom(UI_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send data", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizRegOpt::StartRegistryOptimizer", 0, 0, true, SECONDLEVEL);
	}
	return true ;
}

/***********************************************************************************************
  Function Name  : StopRegistryOptimizer
  Description    : Function which stops registry optimizer process.
  SR.NO			 : WRDWIZCOMMSRV_65
  Author Name    : Ramkrushna Shelke
  Date           : 17 Jan 2014
***********************************************************************************************/
bool CISpyRegOpt::StopRegistryOptimizer()
{
	try
	{
		if (!m_hThread_StartRegScan)
			return true;

		if (m_hThread_StartRegScan != NULL)
		{
			//Varada Ikhar, Date: 23/04/2015, 
			m_bIsRegOptInProgress = false;

			if (TerminateThread(m_hThread_StartRegScan, 0x00) == FALSE)
			{
				AddLogEntry(L"### Failed to terminate Registry optimizer thread.", 0, 0, true, SECONDLEVEL);
				return false;
			}
			CloseHandle(m_hThread_StartRegScan);
			m_hThread_StartRegScan = NULL;
			AddLogEntry(L">>> Registry Optimizer Scanning stopped.", 0, 0, true, FIRSTLEVEL);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizRegOpt::StopRegistryOptimizer", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	PauseScan
*  Description    :	Pause scan
*  Author Name    : Varada Ikhar
*  Date           : 23rd April 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CISpyRegOpt::PauseRegistryOptimizer()
{
	try
	{
		if (!m_bIsRegOptInProgress) return false;

		if (m_hThread_StartRegScan != NULL)
		{
			if (SuspendThread(m_hThread_StartRegScan) == -1)
			{
				AddLogEntry(L"### Failed to pause Registry Optimizer.", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
		AddLogEntry(L">>> Registry Optimizer Scanning Paused.", 0, 0, true, ZEROLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"###  Exception in CWardwizRegOpt::PauseRegistryOptimizer", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**********************************************************************************************************
*  Function Name  :	ResumeScan
*  Description    :	Resume scan
*  Author Name    : Varada Ikhar
*  Date           : 23rd April 2015
*  SR_NO		  :
**********************************************************************************************************/
bool CISpyRegOpt::ResumeRegistryOptimizer()
{
	try
	{
		if (!m_bIsRegOptInProgress) return false;

		if (m_hThread_StartRegScan != NULL)
		{
			if (ResumeThread(m_hThread_StartRegScan) == -1)
			{
				AddLogEntry(L"### Failed to resume Registry Optimizer", 0, 0, true, SECONDLEVEL);
				return false;
			}
		}
		AddLogEntry(L">>> Registry Optimizer Scanning Resumed.", 0, 0, true, ZEROLEVEL);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizRegOpt::ResumeRegistryOptimizer", 0, 0, true, SECONDLEVEL);
	}
	return true;
}