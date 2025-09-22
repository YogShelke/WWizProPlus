/**********************************************************************************************************
Program Name          : ThreadPool.cpp
Description           : This class used to Manage multiple threads.
Author Name			  : Ramkrushna Shelke
Date Of Creation      : 11 Dec 2017
Version No            : 2.6.0.118
Special Logic Used    :
Modification Log      :
***********************************************************************************************************/
#include "StdAfx.h"
#include "ThreadPool.h"
#include "IExecContext.h"

using namespace std;
#ifdef _UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

/***************************************************************************************************
Function       : CThreadPool::_ThreadExecutionContextProc
In Parameters  : LPVOID pParam,
Out Parameters : DWORD WINAPI
Description    : This is the internal thread function which will run
continuously till the Thread Pool is deleted.Any user thread
functions will run from within this function.
Author		  : Ram Shelke
***************************************************************************************************/
DWORD WINAPI CThreadPool::_ThreadExecutionContextProc(LPVOID pParam)
{
	CoInitialize(NULL);

	__try{
		{
			DWORD					dwWait;
			CThreadPool*			pool;
			DWORD					dwThreadId = GetCurrentThreadId();
			HANDLE					hWaits[3];
			IExecContext*			pIExecContext = NULL;
			bool bContinue = true;
			if(NULL == pParam)
			{
				return -1;
			}

			pool = static_cast<CThreadPool*>(pParam);
			hWaits[0] = pool->GetWaitHandle(false,dwThreadId);
			hWaits[1] = pool->GetShutdownHandle();
			hWaits[2] = pool->GetWaitHandle(true,dwThreadId);
			if(pool->GetThreadProc(dwThreadId, &pIExecContext))
			{
				//Initialize the thread just once
				if((!pIExecContext) || (!pIExecContext->Initialize(hWaits[0])))
				{
					::SetEvent(pool->GetFinishLastOperHandle (dwThreadId));
					goto quit;
				}
			}
			do
			{
				dwWait = WaitForMultipleObjects(3, hWaits, FALSE, INFINITE);

				if(dwWait == WAIT_OBJECT_0 + 0)
				{
					//TODO:Error Handling
					pool->BusyNotify(dwThreadId);
					if(pool->GetThreadProc(dwThreadId, &pIExecContext))
					{
						if(pIExecContext)
							pIExecContext->Run();
					}
					pool->FinishNotify(dwThreadId); // tell the pool, i am now free
				}
				else
					if(dwWait == WAIT_OBJECT_0 + 1)
					{
						HANDLE hWait = pool->GetFinishLastOperHandle (dwThreadId);
						if(hWait)
						{
							::SetEvent(hWait);
						}
						bContinue = false;
					}
					else
						if(dwWait == WAIT_OBJECT_0 + 2)
						{
							pool->BusyNotify(dwThreadId);
							if(pool->GetThreadProc(dwThreadId, &pIExecContext))
							{
								if(pIExecContext)
									pIExecContext->Run(true);
							}

							pool->FinishNotify(dwThreadId); // tell the pool, i am now free
							::ResetEvent(hWaits[2]);
							::SetEvent(pool->GetFinishLastOperHandle (dwThreadId));
						}
						else if(dwWait == WAIT_TIMEOUT)
						{
							AddLogEntry(_T("WAIT_TIMEOUT"), 0, 0, true, SECONDLEVEL);
						}

			} while (bContinue);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
	}
quit:
	CoUninitialize();
	return 0;
}

/***************************************************************************************************
Function       : CThreadPool::CThreadPool
In Parameters  :
Out Parameters :
Description    :  Constructor that Intialize the member Variables
Pool size, indicates the number of threads that will be
available in the queue.
Author		   : Ram Shelke  
***************************************************************************************************/
CThreadPool::CThreadPool()
{
	m_nPoolSize = DEFAULT_POOL_SIZE;
	m_iThreadState = CREATE_THRD_SUSPENDED;
	m_arrThreadHandle = NULL;
	m_arrNotifyWaitHandle = NULL;
	m_arrFinishLastOpHandle = NULL;
	memset(m_szEventName,'\0',MAX_PATH*sizeof(TCHAR));
	m_bExecContextInit = false;
}

/***************************************************************************************************
Function       : CThreadPool::~CThreadPool
In Parameters  :
Out Parameters :
Description    : Destructor that Destorys the Memory
Author		   : Ram Shelke  
***************************************************************************************************/
CThreadPool::~CThreadPool()
{
	StopThreadPool();
}

/***************************************************************************************************
Function       : SetEventName(LPCTSTR szEventName)
In Parameters  :Event Name
Out Parameters :
Description    : Sets the Event Name for the thread pool
Author		   : Ram Shelke
***************************************************************************************************/
void CThreadPool::SetEventName(LPCTSTR szEventName)
{
    _tcscat_s(m_szEventName,MAX_PATH,szEventName);
}
/***************************************************************************************************
Function       : CThreadPool::WaitForLastOperation()
In Parameters  :
Out Parameters :
Description    : Wait for all pool threads to complete their last operation
Author		   : Ram Shelke
***************************************************************************************************/
bool CThreadPool::WaitForLastOperation()
{
	__try
	{
		NotifyLastOperation();
		if (!m_arrFinishLastOpHandle)
			return false;
		WaitForMultipleObjects(m_nPoolSize, m_arrFinishLastOpHandle, TRUE, INFINITE);
		for (int nIndex = 0; nIndex < m_nPoolSize; nIndex++)
		{
			if (m_arrFinishLastOpHandle)
			{
				::ResetEvent(m_arrFinishLastOpHandle[nIndex]);
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CThreadPool::WaitForLastOperation", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
Function       : CThreadPool::PauseThreadPool
In Parameters  :
Out Parameters : void
Description    : Suspend Thread Pool on demand
Author		   : Ram Shelke
***************************************************************************************************/
void CThreadPool::PauseThreadPool()
{
	__try
	{
		if (!m_arrThreadHandle)
			return;

		for (int nIndex = 0; nIndex < m_nPoolSize; nIndex++)
		{
			if (m_arrThreadHandle[nIndex] != INVALID_HANDLE_VALUE)
			{
				::SuspendThread(m_arrThreadHandle[nIndex]);
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CThreadPool::PauseThreadPool", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function       : CThreadPool::ResumeThreadPool
In Parameters  :
Out Parameters : void
Description    : Resume Thread Pool on demand
Author		   : Ram Shelke
***************************************************************************************************/
void CThreadPool::ResumeThreadPool()
{
	__try
	{
		if (!m_arrThreadHandle)
			return;

		for (int nIndex = 0; nIndex < m_nPoolSize; nIndex++)
		{
			if (m_arrThreadHandle[nIndex] != INVALID_HANDLE_VALUE)
			{
				::ResumeThread(m_arrThreadHandle[nIndex]);
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CThreadPool::ResumeThreadPool", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function       : CThreadPool::NotifyLastOperation
In Parameters  :
Out Parameters : void
Description    : Notifying all threads to perform last operation
Author		   : Ram Shelke
***************************************************************************************************/
void CThreadPool::NotifyLastOperation()
{
#ifndef _DEBUG
	__try
#else
	try
#endif
	{
		ThreadMap::iterator iter;
		_ThreadData ThreadData;
		for (iter = m_threads.begin(); iter != m_threads.end(); iter++)
		{
			ThreadData = (*iter).second;
			::SetEvent(ThreadData.hNotifyLastOpHandle);
		}
	}
#ifndef _DEBUG
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
#else
	catch (...)
#endif
	{
		AddLogEntry(L"### Exception in CThreadPool::NotifyLastOperation(", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*	Function       : CThreadPool::CreateThreadPool
*	In Parameters  : int nPoolSize,
*	Out Parameters : bool
*	Description    : Use this method to create the thread pool.The constructor of
*					this class by default will create the pool.Make sure you
*					do not call this method without first calling the Destroy()
*					method to release the existing pool.
* Author		   : Ram Shelke
***************************************************************************************************/
bool CThreadPool::CreateThreadPool(int nPoolSize)
{
#ifndef _DEBUG
	__try
#else
	try
#endif
	{
		DWORD		dwThreadId;
		_ThreadData ThreadData;
		TCHAR		szEvtName[50] = _T("\0");
		if (m_arrThreadHandle)
		{
			return true;
		}
		InitializeCriticalSection(&m_cs); // this is used to protect the shared
		// data like the list and map
		if (nPoolSize <= 0)
		{
			m_nPoolSize = DEFAULT_POOL_SIZE;
		}
		m_nPoolSize = nPoolSize;
		// create the event which will signal the threads to stop
		m_hNotifyShutdown = ::CreateEvent(NULL, TRUE, FALSE, NULL);
		if (!m_hNotifyShutdown)
		{
			return false;
		}
		m_arrThreadHandle = new HANDLE[m_nPoolSize];
		m_arrNotifyWaitHandle = new HANDLE[m_nPoolSize];
		m_arrFinishLastOpHandle = new HANDLE[m_nPoolSize];
		// create the threads
		for (int nIndex = 0; nIndex < m_nPoolSize; nIndex++)
		{
			m_arrThreadHandle[nIndex] = CreateThread(NULL, 0, CThreadPool::_ThreadExecutionContextProc,
				this, CREATE_SUSPENDED, &dwThreadId);

			_stprintf_s(szEvtName, 50, _T("Thread_%d"), dwThreadId);

			if (m_arrThreadHandle[nIndex])
			{
				// add the entry to the map of threads
				ThreadData.bFree = true;
				ThreadData.WaitHandle = ::CreateEvent(NULL, TRUE, FALSE, szEvtName);
				m_arrNotifyWaitHandle[nIndex] = ThreadData.hNotifyLastOpHandle = ::CreateEvent(NULL, TRUE, FALSE, NULL);
				m_arrFinishLastOpHandle[nIndex] = ThreadData.hFinishLastOpHandle = ::CreateEvent(NULL, TRUE, FALSE, NULL);
				ThreadData.hThread = m_arrThreadHandle[nIndex];
				ThreadData.dwThreadId = dwThreadId;
				ThreadData.ePoolState = CREATE_THRD_SUSPENDED;
				//Will be assigned later using Assign Context
				//So that the context for the same thread can be reassigned
				ThreadData.pIContext = NULL;
				m_threads.insert(ThreadMap::value_type(dwThreadId, ThreadData));
			}
			else
			{
				return false;
			}
		}
	}
#ifndef _DEBUG
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
#else
	catch (...)
#endif
	{
		AddLogEntry(L"### Exception in CThreadPool::CreateThreadPool", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
* Function       : CThreadPool::StopThreadPool
* In Parameters  :
* Out Parameters : void
* Description    : Use this method to destory the thread pool.The destructor of
* this class will destory the pool anyway.Make sure you
* this method before calling a Create()when an existing pool is
* already existing.
* Author			 : Ram Shelke
***************************************************************************************************/ 
void CThreadPool::StopThreadPool()
{
#ifndef _DEBUG
	__try
#else
	try
#endif
	{
		if (m_arrThreadHandle == NULL)
		{
			return;
		}
		// tell all threads to shutdown.
		SetEvent(m_hNotifyShutdown);

		// lets give the thread one second atleast to terminate
		// Wait for all threads to complete the tasks
		WaitForMultipleObjects(m_nPoolSize, m_arrThreadHandle, TRUE, 30000);

		ThreadMap::iterator iter;
		_ThreadData ThreadData;

		// walk through the events and threads and close them all
		for (iter = m_threads.begin(); iter != m_threads.end(); iter++)
		{
			ThreadData = (*iter).second;
			CloseHandle(ThreadData.WaitHandle);
			ThreadData.WaitHandle = NULL;
			CloseHandle(ThreadData.hNotifyLastOpHandle);
			ThreadData.hNotifyLastOpHandle = NULL;
			CloseHandle(ThreadData.hFinishLastOpHandle);
			ThreadData.hFinishLastOpHandle = NULL;
			CloseHandle(ThreadData.hThread);
			ThreadData.hThread = NULL;
		}

		// close the shutdown event
		if (m_hNotifyShutdown)
		{
			::CloseHandle(m_hNotifyShutdown);
			m_hNotifyShutdown = NULL;
		}
		if (m_arrThreadHandle)
		{
			delete[] m_arrThreadHandle;
			m_arrThreadHandle = NULL;
		}

		if (m_arrNotifyWaitHandle)
		{
			delete[] m_arrNotifyWaitHandle;
			m_arrNotifyWaitHandle = NULL;
		}

		if (m_arrFinishLastOpHandle)
		{
			delete[] m_arrFinishLastOpHandle;
			m_arrFinishLastOpHandle = NULL;
		}
		// delete the critical section
		DeleteCriticalSection(&m_cs);

		// empty all collections
		m_threads.clear();
	}
#ifndef _DEBUG
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
#else
	catch (...)
#endif
	{
		AddLogEntry(L"### Exception in CThreadPool::StopThreadPool", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
* Function       : CThreadPool::GetPoolSize
* In Parameters  :
* Out Parameters : int
* Description    : Get Pool Size
* Author		   : Ram Shelke
***************************************************************************************************/
int CThreadPool::GetPoolSize()
{
	return m_nPoolSize;
}

/***************************************************************************************************
* Function       : CThreadPool::SetPoolSize
* In Parameters  : int nSize,
* Out Parameters : void
* Description    : Sets the Pool Size
* Author		 : Ram Shelke
***************************************************************************************************/
void CThreadPool::SetPoolSize(int nSize)
{
	if(nSize <= 0)
	{
		m_nPoolSize = DEFAULT_POOL_SIZE;
		return;
	}
	m_nPoolSize = nSize;
}

/***************************************************************************************************
* Function       : CThreadPool::GetShutdownHandle
* In Parameters  :
* Out Parameters : HANDLE
* Description    : Gets the ShutDown handle
* Author		 : Ram Shelke
***************************************************************************************************/
HANDLE CThreadPool::GetShutdownHandle()
{
	return m_hNotifyShutdown;
}

/***************************************************************************************************
* Function       : CThreadPool::FinishNotify
* In Parameters  : DWORD dwThreadId,
* Out Parameters : void
* Description    : When ever a thread finishes executing the user function, it
* should notify the pool that it has finished executing.
* Author		 : Ram Shelke
***************************************************************************************************/
void CThreadPool::FinishNotify(DWORD dwThreadId)
{
#ifndef _DEBUG
	__try
#else
	try
#endif
	{
		ThreadMap::iterator iter;

		EnterCriticalSection(&m_cs);
		iter = m_threads.find(dwThreadId);

		if (iter == m_threads.end())	// if search found no elements
		{
			LeaveCriticalSection(&m_cs);
			return;
		}
		else
		{
			m_threads[dwThreadId].bFree = true;
			//CMaxBotApp::g_objLogApp.AddLog1(_T("Thread free"));
			// back to sleep, there is nothing that needs servicing.
			LeaveCriticalSection(&m_cs);
			ResetEvent(m_threads[dwThreadId].WaitHandle);
		}
	}
#ifndef _DEBUG
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
#else
	catch (...)
#endif
	{
		AddLogEntry(L"### Exception in CThreadPool::FinishNotify", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
* Function       : CThreadPool::BusyNotify
* In Parameters  : DWORD dwThreadId,
* Out Parameters : void
* Description    : Notify if Busy
* Author		 : Ram Shelke
***************************************************************************************************/
void CThreadPool::BusyNotify(DWORD dwThreadId)
{
#ifndef _DEBUG
	__try
#else
	try
#endif
	{
		ThreadMap::iterator iter;

		EnterCriticalSection(&m_cs);

		iter = m_threads.find(dwThreadId);

		if (iter == m_threads.end())	// if search found no elements
		{
			LeaveCriticalSection(&m_cs);
		}
		else
		{
			m_threads[dwThreadId].bFree = false;
			LeaveCriticalSection(&m_cs);
		}
	}
#ifndef _DEBUG
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
#else
	catch (...)
#endif
	{
		AddLogEntry(L"### Exception in CThreadPool::BusyNotify", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
* Function       : CThreadPool::RunThreadPool
* In Parameters  : ThreadPriority priority, bool bSignalEvent,
* Out Parameters : void
* Description    : This function is to be called by clients which want to make
* use of the thread pool.
* Author		 : Ram Shelke
***************************************************************************************************/
void CThreadPool::RunThreadPool(ThreadPriority priority, bool bSignalEvent)
{
#ifndef _DEBUG
	__try
#else
	try
#endif
	{
		// See if any threads are free
		ThreadMap::iterator iter;
		_ThreadData ThreadData;

		EnterCriticalSection(&m_cs);
		for (iter = m_threads.begin(); iter != m_threads.end(); iter++)
		{
			ThreadData = (*iter).second;
			if (ThreadData.ePoolState != CREATE_THRD_RUNNING)
			{
				ResumeThread(ThreadData.hThread);
				//TODO:Make the thread re-entrant
				m_threads[ThreadData.dwThreadId].ePoolState = CREATE_THRD_RUNNING;
			}
			if (ThreadData.bFree)
			{
				// here is a free thread, put it to work
				m_threads[ThreadData.dwThreadId].bFree = false;
				if (bSignalEvent)
					SetEvent(ThreadData.WaitHandle);
				// this thread will now call GetThreadProc()and pick up the next
				// function in the list.
				//TODO: DO we need a break in future;
				//break;
			}
		}
		LeaveCriticalSection(&m_cs);
	}
#ifndef _DEBUG
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
#else
	catch (...)
#endif
	{
		AddLogEntry(L"### Exception in CThreadPool::RunThreadPool", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
* Function       : CThreadPool::GetWaitHandle
* In Parameters  : bool bNotifyWaitHandle, DWORD dwThreadId
* Out Parameters : HANDLE
* Description    : ThreadId - the id of the thread for which the wait handle is
* being requested.
* Author		   : Ram Shelke
***************************************************************************************************/
HANDLE CThreadPool::GetWaitHandle(bool bNotifyWaitHandle, DWORD dwThreadId)
{
	HANDLE hWait;
#ifndef _DEBUG
	__try
#else
	try
#endif
	{
		ThreadMap::iterator iter;

		EnterCriticalSection(&m_cs);
		iter = m_threads.find(dwThreadId);

		if (iter == m_threads.end())	// if search found no elements
		{
			LeaveCriticalSection(&m_cs);
			return NULL;
		}
		else
		{
			if (bNotifyWaitHandle)
			{
				hWait = m_threads[dwThreadId].hNotifyLastOpHandle;
			}
			else
			{
				hWait = m_threads[dwThreadId].WaitHandle;
			}
			LeaveCriticalSection(&m_cs);
		}
	}
#ifndef _DEBUG
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
#else
	catch (...)
#endif
	{
		AddLogEntry(L"### Exception in CThreadPool::GetWaitHandle", 0, 0, true, SECONDLEVEL);
	}
	return hWait;
}

/***************************************************************************************************
* Function       : CThreadPool::GetFinishLastOperHandle
* In Parameters  : bool bNotifyWaitHandle, DWORD dwThreadId
* Out Parameters : HANDLE
* Description    : ThreadId - the id of the thread for which the wait handle is
* being requested.
* Author		   :  Ram Shelke
***************************************************************************************************/
HANDLE CThreadPool::GetFinishLastOperHandle(DWORD dwThreadId)
{
	HANDLE hWait;
#ifndef _DEBUG
	__try
#else
	try
#endif
	{
		ThreadMap::iterator iter;

		EnterCriticalSection(&m_cs);
		iter = m_threads.find(dwThreadId);

		if (iter == m_threads.end())	// if search found no elements
		{
			LeaveCriticalSection(&m_cs);
			return NULL;
		}
		else
		{
			hWait = m_threads[dwThreadId].hFinishLastOpHandle;
			LeaveCriticalSection(&m_cs);
		}
	}
#ifndef _DEBUG
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
#else
	catch (...)
#endif
	{
		AddLogEntry(L"### Exception in CThreadPool::GetFinishLastOperHandle", 0, 0, true, SECONDLEVEL);
	}
	return hWait;
}

/***************************************************************************************************
* Function       : CThreadPool::GetThreadProc
* In Parameters  : DWORD dwThreadId, IExecContext**piExecContext,
* Out Parameters : bool
* Description    :
* Author		 : Ram Shelke & 11 Dec, 2017.
***************************************************************************************************/
bool CThreadPool::GetThreadProc(DWORD dwThreadId, IExecContext**piExecContext)
{
	bool bRet = false;
#ifndef _DEBUG
	__try
#else
	try
#endif
	{
		ThreadMap::iterator iter;
		EnterCriticalSection(&m_cs);
		iter = m_threads.find(dwThreadId);

		if (iter != m_threads.end())	// if search found no elements
		{
			*piExecContext = m_threads[dwThreadId].pIContext;
			bRet = true;
		}
		else
		{
			*piExecContext = NULL;
		}
		LeaveCriticalSection(&m_cs);
	}
#ifndef _DEBUG
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
#else
	catch (...)
#endif
	{
		AddLogEntry(L"### Exception in CThreadPool::GetThreadProc", 0, 0, true, SECONDLEVEL);
	}
	return bRet;
}

/***************************************************************************************************
Function       : CThreadPool::GetCurrentExecutionContext
In Parameters  :
Out Parameters : IExecContext*
Description    : Gets the Current Execution Context
Author		   : Ram Shelke 
***************************************************************************************************/
IExecContext* CThreadPool::GetCurrentExecutionContext()
{
	IExecContext *pIContext = NULL;
#ifndef _DEBUG
	__try
#else
	try
#endif
	{
		EnterCriticalSection(&m_cs);
		ThreadMap::iterator iter;
		_ThreadData ThreadData;
		for (iter = m_threads.begin(); iter != m_threads.end(); iter++)
		{
			ThreadData = (*iter).second;
			if (ThreadData.pIContext != NULL)
			{
				pIContext = m_threads[ThreadData.dwThreadId].pIContext;
				break;
			}
		}
		LeaveCriticalSection(&m_cs);
	}
#ifndef _DEBUG
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
#else
	catch (...)
#endif
	{
		AddLogEntry(L"### Exception in CThreadPool::GetCurrentExecutionContext", 0, 0, true, SECONDLEVEL);
	}
	return pIContext;
}

/***************************************************************************************************
Function       : CThreadPool::AssignContext
In Parameters  : IExecContext *pIContext,
Out Parameters : bool
Description    : Assigns Context to the Thread
Author		   : Ram Shelke
***************************************************************************************************/
bool CThreadPool::AssignContext(IExecContext *pIContext)
{
	bool bRet = FALSE;
#ifndef _DEBUG
	__try
#else
	try
#endif
	{
		EnterCriticalSection(&m_cs);
		ThreadMap::iterator iter;
		_ThreadData ThreadData;
		for (iter = m_threads.begin(); iter != m_threads.end(); iter++)
		{
			ThreadData = (*iter).second;
			if (ThreadData.bFree && (ThreadData.pIContext == NULL))
			{
				m_threads[ThreadData.dwThreadId].pIContext = pIContext;
				m_ExecCntxtList.push_back(pIContext);
				bRet = true;
				break;
			}
		}
		LeaveCriticalSection(&m_cs);
		if (m_ExecCntxtList.size() == m_nPoolSize)
		{
			ReInitExecContext();
		}
	}
#ifndef _DEBUG
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
#else
	catch (...)
#endif
	{
		AddLogEntry(L"### Exception in CThreadPool::AssignContext", 0, 0, true, SECONDLEVEL);
	}
	return bRet;
}

/***************************************************************************************************
Function       : CThreadPool::GetContext
In Parameters  : DWORD nContextIndex,
Out Parameters : IExecContext*
Description    : Get the Context according to Context index
Author		   : Ram Shelke
***************************************************************************************************/
IExecContext* CThreadPool::GetContext(DWORD nContextIndex)
{
	__try
	{
		if (m_ExecCntxtList.size() > nContextIndex)
		{
			return m_ExecCntxtList[nContextIndex];
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CThreadPool::GetContext", 0, 0, true, SECONDLEVEL);
	}
	return NULL;
}

/***************************************************************************************************
Function       : CThreadPool::ClearExecutionContext
In Parameters  :
Out Parameters : void
Description    : Clears the execution list.The context object will be deleted by the caller
Author		   : Ram Shelke
***************************************************************************************************/
void CThreadPool::ClearExecutionContext()
{
#ifndef _DEBUG
	__try
#else
	try
#endif
	{
		if (m_arrThreadHandle == NULL)
		{
			return;
		}
		m_ExecCntxtList.clear();
		EnterCriticalSection(&m_cs);
		ThreadMap::iterator iter;
		_ThreadData *ThreadData;
		for (iter = m_threads.begin(); iter != m_threads.end(); iter++)
		{
			ThreadData = &(*iter).second;
			ThreadData->bFree = true;
			ThreadData->pIContext = NULL;
		}
		m_bExecContextInit = false;
		//TODO:Leave Critical Section
		LeaveCriticalSection(&m_cs);
	}
#ifndef _DEBUG
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
#else
	catch (...)
#endif
	{
		AddLogEntry(L"### Exception in CThreadPool::ClearExecutionContext", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
Function       : CThreadPool::ReInitExecContext
In Parameters  :
Out Parameters : void
Description    : 
Author		   : Ram Shelke
***************************************************************************************************/
void CThreadPool::ReInitExecContext()
{
#ifndef _DEBUG
	__try
#else
	try
#endif
	{
		EnterCriticalSection(&m_cs);
		ThreadMap::iterator iter;
		_ThreadData *ThreadData = NULL;
		for (iter = m_threads.begin(); iter != m_threads.end(); iter++)
		{
			ThreadData = &(*iter).second;
			ThreadData->pIContext->Initialize(ThreadData->WaitHandle);
		}
		LeaveCriticalSection(&m_cs);

	}
#ifndef _DEBUG
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
#else
	catch (...)
#endif
	{
		AddLogEntry(L"### Exception in CThreadPool::ReInitExecContext", 0, 0, true, SECONDLEVEL);
	}
}