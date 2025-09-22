/**********************************************************************************************************                     
	  Program Name          : ISpyNamedPipeListener.h
	  Description           : Implementation of INamedPipeData interface and 
							  to start Listener thread.
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 21 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Ramkrushna         : Created wrapper to for listening the pipe messages sent by client.	21 Jan 2014
***********************************************************************************************************/
#pragma once;
#define MAX_PIPE_LENGTH 10000

class CISpyNamedPipeListener;

interface INamedPipeData
{
	virtual void OnIncomingData(LPVOID lpParam) = 0;
	virtual void OnConnectingPipe() = 0;
	virtual void OnDisConnectingPipe(CISpyNamedPipeListener* pReader) = 0;
	virtual TCHAR* GetPipeName(void) = 0;
	virtual DWORD GetStructSize(void) = 0;
	virtual bool SendResponse(LPVOID lpData) = 0;
};

class CISpyNamedPipeListener
{
public:
	CISpyNamedPipeListener(INamedPipeData* pDest);
	~CISpyNamedPipeListener();

	bool StartReader(void);
	bool SendResponse(LPVOID lpResponse);
	static bool m_bStopListener;
	static bool m_bSingleThreaded;
	DWORD	m_nID;
	bool m_bMonitorConnections;
	HANDLE m_hServerStopEvent;
	HANDLE m_hLastClientDisconnectEvent;
private:
	static DWORD WINAPI NamedPipeListenerThread(LPVOID lParam);
	bool ReadPipe(void);
	void Cleanup();
	
	HANDLE	m_hPipe;
	HANDLE	m_hThread;
	HANDLE m_hOverlap[2];
	INamedPipeData* m_pDest;
};