/**********************************************************************************************************                     
	  Program Name          : ISpyCommunicator.cpp
	  Description           : Client Communicator class using named pipes
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 23 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Ramkrushna         : Created wrapper Client class for communication using named pipes.		21 Jan 2014
***********************************************************************************************************/
#include "stdafx.h"
#include "ISpyCommunicator.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 
/***********************************************************************************************
  Function Name  : CISpyCommunicator
  Description    : C'tor
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0063
  Date           : 23 Jan 2014
***********************************************************************************************/
CISpyCommunicator::CISpyCommunicator(const TCHAR* tchPipeName, bool bRetryConnection, DWORD dwNoofRetry, DWORD dwTimeout)
{
	_tcscpy_s(m_tchPipe, tchPipeName);
	m_hPipe = INVALID_HANDLE_VALUE;
	m_bRetryConnection = bRetryConnection;
	m_dwNoofRetry = dwNoofRetry;
	m_dwTimeout = dwTimeout;
}

/***********************************************************************************************
  Function Name  : ~CISpyCommunicator
  Description    : D'tor
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0064
  Date           : 23 Jan 2014
***********************************************************************************************/
CISpyCommunicator::~CISpyCommunicator()
{
	Close();
}

/***********************************************************************************************
  Function Name  : Close
  Description    : function will close pipe handle if it is already created.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0065
  Date           : 24 Jan 2014
***********************************************************************************************/
void CISpyCommunicator::Close()
{
	__try{
		if(m_hPipe != INVALID_HANDLE_VALUE)
		{
			FlushFileBuffers(m_hPipe);
			DisconnectNamedPipe(m_hPipe);
			CloseHandle(m_hPipe);
			m_hPipe = INVALID_HANDLE_VALUE;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{

	}
}

/***********************************************************************************************
  Function Name  : Connect
  Description    : Function will connect to server using pipe name
				   if connection will fail it will retry for connection, and depend on MAX_CONNECT_RETRY.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0066
  Date           : 24 Jan 2014
***********************************************************************************************/
bool CISpyCommunicator::Connect(void)
{
	int nTryCount = 0;
	bool bConnected = false;
	__try{
		do{
			m_hPipe = CreateFile(
				m_tchPipe,		// __in          LPCTSTR lpFileName,
				GENERIC_WRITE | GENERIC_READ,					// __in          DWORD dwDesiredAccess,
				0,								// __in          DWORD dwShareMode,
				NULL,							// __in          LPSECURITY_ATTRIBUTES lpSecurityAttributes,
				OPEN_EXISTING,					// __in          DWORD dwCreationDisposition,
				FILE_ATTRIBUTE_NORMAL,			// DWORD dwFlagsAndAttributes,
				NULL							// __in          HANDLE hTemplateFile
				);

			if(m_hPipe == INVALID_HANDLE_VALUE)
			{
				if(m_bRetryConnection)
				{
					if (nTryCount >= static_cast<int>(m_dwNoofRetry) )// MAX_CONNECT_RETRY)
					{
						m_bRetryConnection = false;
						break;
					}
					nTryCount ++;
					Sleep(m_dwTimeout);
				}
			}
			else
			{
				bConnected = true;
				m_bRetryConnection = false;
			}
		}while ((!bConnected) && m_bRetryConnection);
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		bConnected = false;
	}
	return bConnected;
}

/***********************************************************************************************
  Function Name  : SendData
  Description    : Function will connect to pipe if its not connected.
				   if connection success then will write data into pipe.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0067
  Date           : 24 Jan 2014
***********************************************************************************************/
bool CISpyCommunicator::SendData(LPVOID lpMaxData, DWORD dwSize)
{
	BOOL bReturn = FALSE;
	__try{
		if(m_hPipe == INVALID_HANDLE_VALUE)
		{
			if(false == Connect())
			{
				return false;
			}
		}

		DWORD nWritten = 0;

		bReturn = WriteFile(
			m_hPipe,				// __in          HANDLE hFile,
			(LPCVOID)lpMaxData,		// __in          LPCVOID lpBuffer,
			dwSize,					// __in          DWORD nNumberOfBytesToWrite,
			&nWritten,				// __out         LPDWORD lpNumberOfBytesWritten,
			NULL					// __in          LPOVERLAPPED lpOverlapped
			);
		if(!bReturn)
		{
			Close();
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{

	}
	return (bReturn == FALSE ? false : true); 
}

/***********************************************************************************************
  Function Name  : ReadData
  Description    : Function will read data from pipe.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0068
  Date           : 24 Jan 2014
***********************************************************************************************/
bool CISpyCommunicator::ReadData(LPVOID lpMaxData, DWORD dwSize)
{
	BOOL bReturn = FALSE;
	__try{
		if(m_hPipe == INVALID_HANDLE_VALUE)
			return false;

		DWORD nRead = 0;

		bReturn = ReadFile(
			m_hPipe,			// __in          HANDLE hFile,
			lpMaxData,			// __in          LPCVOID lpBuffer,
			dwSize,				// __in          DWORD nNumberOfBytesToWrite,
			&nRead,				// __out         LPDWORD lpNumberOfBytesWritten,
			NULL				// __in          LPOVERLAPPED lpOverlapped
			);
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{

	}
	return (bReturn == FALSE ? false : true); 
}