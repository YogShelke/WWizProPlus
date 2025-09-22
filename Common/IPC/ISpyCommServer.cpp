/**********************************************************************************************************                     
	  Program Name          : ISpyCommServer.cpp
	  Description           : Communicator class using named pipes.
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 20 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Ramkrushna         : Created wrapper Server class for communication using named pipes.		20 Jan 2014
***********************************************************************************************************/
#include "stdafx.h"
#include "ISpyCommServer.h"
#include "ISpyNamedPipeListener.h"

const int CLOSE_TIMEOUT = 5000;
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***********************************************************************************************
  Function Name  : CISpyCommunicatorServer
  Description    : C'tor
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0055
  Date           : 20 Jan 2014
***********************************************************************************************/
CISpyCommunicatorServer::CISpyCommunicatorServer(const TCHAR* tchPipeName, CallBackFunctionPtr fnPtrCallBack, DWORD dwSize)
{
	_tcscpy_s(m_tchPipe, tchPipeName);
	m_fnPtrCallBack = fnPtrCallBack;
	m_dwStructSize = dwSize;
	m_bMonitorConnections = false;
	m_hServerStopEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hLastClientDisconnectEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	m_bServerRunning = false;
}

/***************************************************************************
  Function Name  : ~CISpyCommunicatorServer
  Description    : D'tor
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0056
  Date           : 20 Jan 2014
****************************************************************************/
CISpyCommunicatorServer::~CISpyCommunicatorServer()
{
	if(m_bServerRunning)
	{
		StopServer();
		m_bServerRunning = false;
	}

	if(m_hServerStopEvent)
	{
		::CloseHandle(m_hServerStopEvent);
		m_hServerStopEvent = NULL;
	}

	if(m_hLastClientDisconnectEvent)
	{
		::CloseHandle(m_hLastClientDisconnectEvent);
		m_hLastClientDisconnectEvent = NULL;
	}
	CISpyNamedPipeListener::m_bStopListener = false;
}

/***************************************************************************
  Function Name  : StopServer
  Description    : Stop listening requests from clients.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0057
  Date           : 20 Jan 2014
****************************************************************************/
bool CISpyCommunicatorServer::StopServer()
{
	__try{
		CISpyNamedPipeListener::m_bStopListener = true;
		if(m_hServerStopEvent)
		{
			SetEvent(m_hServerStopEvent);
		}

		::WaitForSingleObject(m_hLastClientDisconnectEvent, CLOSE_TIMEOUT);

		m_bMonitorConnections = false;
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
	}
	return true;
}

/***************************************************************************
  Function Name  : Run
  Description    : Run function to start pipe reader thread.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0058
  Date           : 21 Jan 2014
****************************************************************************/
bool CISpyCommunicatorServer::Run(bool bMonitorConnections,bool bSingleThreaded)
{
	m_bServerRunning = true;
	m_bMonitorConnections = bMonitorConnections;
	CISpyNamedPipeListener::m_bSingleThreaded = bSingleThreaded;
	return RunPipeReader();
}

/***************************************************************************
  Function Name  : RunPipeReader
  Description    : This function will make list of Pipe listeners.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0059
  Date           : 21 Jan 2014
****************************************************************************/
bool CISpyCommunicatorServer::RunPipeReader()
{
	CAutoCriticalSection cas(m_CriticalSectionSys);
	if(CISpyNamedPipeListener::m_bStopListener)
	{
		return false;
	}
	CISpyNamedPipeListener* pPipeListener = new CISpyNamedPipeListener(this);
	pPipeListener->m_bMonitorConnections = m_bMonitorConnections;
	pPipeListener->m_hServerStopEvent = m_hServerStopEvent;
	pPipeListener->m_hLastClientDisconnectEvent = m_hLastClientDisconnectEvent;

	m_PipeListenerList.push_back(pPipeListener);

	return pPipeListener->StartReader();
}

/***************************************************************************
  Function Name  : OnConnectingPipe
  Description    : SEH function for RunPipeReader
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0060
  Date           : 22 Jan 2014
*******************	*********************************************************/
void CISpyCommunicatorServer::OnConnectingPipe()
{
	__try{
		RunPipeReader();
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

/***************************************************************************
  Function Name  : OnDisConnectingPipe
  Description    : Disconnecting listener.
  Author Name    : Ramkrushna Shelke
  Date           : 22 Jan 2014
****************************************************************************/
void CISpyCommunicatorServer::OnDisConnectingPipe(CISpyNamedPipeListener* pListener)
{
	if(NULL == pListener)
	{
		return;
	}
	CAutoCriticalSection cas(m_CriticalSectionSys);

	if(m_PipeListenerList.size() > 0)
	{
		m_PipeListenerList.remove(pListener);
		delete pListener;
		pListener = NULL;
	}

	if((CISpyNamedPipeListener::m_bStopListener && m_PipeListenerList.size() == 0) || CISpyNamedPipeListener::m_bSingleThreaded)
	{
		if(m_hLastClientDisconnectEvent)
		{
			::SetEvent(m_hLastClientDisconnectEvent);
		}
	}

}

/***************************************************************************
  Function Name  : OnIncomingData
  Description    : Virtual function Which gets called when any get came through pipe
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0062
  Date           : 22 Jan 2014
****************************************************************************/
void CISpyCommunicatorServer::OnIncomingData(LPVOID lpParam)
{
	__try{
		m_fnPtrCallBack(lpParam);
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{

	}
}

/***************************************************************************
Function Name  : SetPipeName
Description    : This function returns pipe name.
Author Name    : Jeena Mariam Saji
Date           : 04 July 2019
****************************************************************************/
void CISpyCommunicatorServer::SetPipeName(TCHAR* pszPipeName)
{
	try
	{
		_tcscpy(m_tchPipe, pszPipeName);
	}
	catch (...)
	{
		AddLogEntry(L"### Excpetion in CWardwizCommunicatorServer::SetPipeName", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : GetPipeName
  Description    : function returns pipe name.
  Author Name    : Ramkrushna Shelke
  Date           : 21 Jan 2014
****************************************************************************/
TCHAR* CISpyCommunicatorServer::GetPipeName(void)
{
	return m_tchPipe;
}

/***************************************************************************
  Function Name  : GetStructSize
  Description    : Returns the structure size.
  Author Name    : Ramkrushna Shelke
  Date           : 21 Jan 2014
****************************************************************************/
DWORD CISpyCommunicatorServer::GetStructSize(void)
{
	return m_dwStructSize;
}

/***************************************************************************
  Function Name  : SendResponse
  Description    : Function will accept data from client to send respose back.
  Author Name    : Ramkrushna Shelke
  Date           : 22 Jan 2014
****************************************************************************/
bool CISpyCommunicatorServer::SendResponse(LPVOID lpData)
{
	CAutoCriticalSection cas(m_CriticalSectionSys);
	for (TISpyNamedPipeListenerList::iterator it = m_PipeListenerList.begin(); it != m_PipeListenerList.end(); it++)
	{
		CISpyNamedPipeListener* pPipeListener = (*it);
		if(pPipeListener)
		{
			if(pPipeListener->m_nID == GetCurrentThreadId())
			{
				return pPipeListener->SendResponse(lpData);
			}
		}
	}
	return false;
}