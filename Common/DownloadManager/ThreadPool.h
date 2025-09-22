/**********************************************************************************************************
Program Name          : ThreadPool.h
Description           : This class used to Manage multiple threads.
Author Name			  : Ramkrushna Shelke
Date Of Creation      : 11 Dec 2017
Version No            : 2.6.0.118
Special Logic Used    :
Modification Log      :
***********************************************************************************************************/
#pragma once
#include "IExecContext.h"
#include <list>
#include <map>
#include <vector>

using namespace std;

const int DEFAULT_POOL_SIZE = 4;
const int MAX_POOL_SIZE = 100;

enum THREAD_POOL_STATE
{
	CREATE_THRD_SUSPENDED,
	CREATE_THRD_RUNNING
};

typedef struct tagThreadData
{
	bool	bFree;
	HANDLE	WaitHandle;
	HANDLE	hNotifyLastOpHandle;
	HANDLE	hFinishLastOpHandle;
	HANDLE	hThread;
	DWORD	dwThreadId;
	THREAD_POOL_STATE ePoolState;
	IExecContext *pIContext;
} _ThreadData;

enum ThreadPriority
{
	PRIORITY_HIGH_THREAD,
	PRIORITY_LOW_THREAD
};

typedef std::map<DWORD, _ThreadData, less<DWORD>, allocator<_ThreadData>> ThreadMap;
typedef std::vector<IExecContext*> ExecutionCntxtList;

class CThreadPool
{
public:
	CThreadPool();
	~CThreadPool(void);
	bool	CreateThreadPool(int nPoolSize);
	void	StopThreadPool();
	int	GetPoolSize();
	void	SetPoolSize(int);
	void	FinishNotify(DWORD dwThreadId);
	void	BusyNotify(DWORD dwThreadId);
	void	RunThreadPool(ThreadPriority priority = PRIORITY_HIGH_THREAD, bool bSignalEvent = false);
	bool	GetThreadProc(DWORD dwThreadId, IExecContext** piExecContext);
	bool	AssignContext(IExecContext *pIContext);
	IExecContext* GetCurrentExecutionContext();
	IExecContext* GetContext(DWORD nContextIndex);
	bool	WaitForLastOperation();
	void	PauseThreadPool();
	void	ResumeThreadPool();
	void    ClearExecutionContext();
	void	SetEventName(LPCTSTR szEventName);
	void	ReInitExecContext();
private:
	static DWORD WINAPI _ThreadExecutionContextProc(LPVOID);
	void NotifyLastOperation();
	HANDLE	GetWaitHandle(bool bNotifyWaitHandle,DWORD dwThreadId);
	HANDLE	GetFinishLastOperHandle(DWORD dwThreadId);
	HANDLE	GetShutdownHandle();
	HANDLE	GetNotifyLastOperHandle(DWORD dwThreadId);
	HANDLE *m_arrNotifyWaitHandle;
	HANDLE *m_arrFinishLastOpHandle;
	HANDLE *m_arrThreadHandle;
	bool	m_bExecContextInit;
	ExecutionCntxtList m_ExecCntxtList;
	CRITICAL_SECTION m_cs;
	ThreadMap m_threads;
	int     m_iThreadState;
	int		m_nPoolSize;
	HANDLE	m_hNotifyShutdown;
	TCHAR m_szEventName[MAX_PATH];
 
};
