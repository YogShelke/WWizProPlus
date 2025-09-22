/**********************************************************************************************************                     
	  Program Name          : ISpyCommunicator.h
	  Description           : Client Communicator class using named pipes
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 23 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Ramkrushna         : Created wrapper Client class for communication using named pipes.		21 Jan 2014
***********************************************************************************************************/
#pragma once

const int MAX_CONNECT_RETRY = 15;
const int MAX_CONNECT_TIMEOUT = 3000;

class CISpyCommunicator
{
public:
	// Constructor
	CISpyCommunicator(const TCHAR* tchPipeName, bool bRetryConnection = false, DWORD dwNoofRetry = MAX_CONNECT_RETRY, DWORD dwTimeout = MAX_CONNECT_TIMEOUT);
	// Destructor
	~CISpyCommunicator();
	// Member Functions
	bool SendData(LPVOID lpMaxData, DWORD dwSize);
	bool ReadData(LPVOID lpMaxData, DWORD dwSize);
	void Close();
private:
	// To connect to a given named pipe
	bool Connect(void);
	// Handle to the Named Pipe
	HANDLE m_hPipe;
	// Pipe Name
	TCHAR m_tchPipe[MAX_PATH];
	bool m_bRetryConnection;
	DWORD m_dwNoofRetry;
	DWORD m_dwTimeout;
};