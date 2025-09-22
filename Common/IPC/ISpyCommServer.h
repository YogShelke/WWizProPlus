/**********************************************************************************************************                     
	  Program Name          : ISpyCommServer.h
	  Description           : Wrapper class as a server for communication
							  using named pipe.
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 21 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Ramkrushna         : Created wrapper Server class for communication using named pipes.		21 Jan 2014
***********************************************************************************************************/
#pragma once

#include <list>
#include "ISpyNamedPipeListener.h"
#include "ISpyCriticalSection.h"

typedef std::list<CISpyNamedPipeListener*> TISpyNamedPipeListenerList;
typedef void (*CallBackFunctionPtr)(LPVOID lpParam);

class CISpyCommunicatorServer : public INamedPipeData
{
public:
	CISpyCommunicatorServer(const TCHAR* tchPipeName, CallBackFunctionPtr fnPtrCallBack, DWORD dwSize);
	virtual ~CISpyCommunicatorServer();

	bool Run(bool bMonitorConnections = false,bool bSingleThreaded = false);
	virtual bool SendResponse(LPVOID lpData);
	virtual bool StopServer();
	void SetPipeName(TCHAR* pszPipeName);
protected:
	virtual void OnIncomingData(LPVOID sMaxPipeData);
	virtual void OnConnectingPipe();
	virtual void OnDisConnectingPipe(CISpyNamedPipeListener* pListener);
	virtual TCHAR* GetPipeName(void);
	virtual DWORD GetStructSize(void);
private:
	bool RunPipeReader(void);
	TISpyNamedPipeListenerList	m_PipeListenerList;
	CISpyCriticalSection		m_CriticalSectionData;
	CISpyCriticalSection		m_CriticalSectionSys;
	CallBackFunctionPtr			m_fnPtrCallBack;
	TCHAR						m_tchPipe[MAX_PATH];
	DWORD						m_dwStructSize;
	bool						m_bMonitorConnections;
	bool						m_bServerRunning;
	HANDLE						m_hServerStopEvent;
	HANDLE						m_hLastClientDisconnectEvent;

};