/**********************************************************************************************************         	  Program Name          : ISpyCriticalSection.cpp
	  Description           : Wrapper class for to use thread synchronization objects
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 21 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Ramkrushna         : Created wrapper to use thread synchronization objects.		21 Jan 2014
***********************************************************************************************************/
#include "stdafx.h"
#include "ISpyCriticalSection.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***********************************************************************************************
  Function Name  : CISpyCriticalSection
  Description    : C'tor
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0069
  Date           : 21 Jan 2014
***********************************************************************************************/
CISpyCriticalSection::CISpyCriticalSection()
{
	InitializeCriticalSection(&m_CritSect);
}

/***********************************************************************************************
  Function Name  : ~CISpyCriticalSection
  Description    : D'tor
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0070
  Date           : 21 Jan 2014
***********************************************************************************************/
CISpyCriticalSection::~CISpyCriticalSection()
{
	DeleteCriticalSection(&m_CritSect);
}

/***********************************************************************************************
  Function Name  : Lock
  Description    : Function to lock resource using critical section object
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0071
  Date           : 21 Jan 2014
***********************************************************************************************/
void CISpyCriticalSection::Lock()
{
	EnterCriticalSection(&m_CritSect);
}

/***********************************************************************************************
  Function Name  : Unlock
  Description    : Function to release resource using critical section object
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0072
  Date           : 21 Jan 2014
***********************************************************************************************/
void CISpyCriticalSection::Unlock()
{
	LeaveCriticalSection(&m_CritSect);
}

/***********************************************************************************************
  Function Name  : CAutoCriticalSection
  Description    : C'tor
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0073
  Date           : 21 Jan 2014
***********************************************************************************************/
CAutoCriticalSection::CAutoCriticalSection(CISpyCriticalSection& rCritSect)
: m_rCritSect(rCritSect)
{
	m_rCritSect.Lock();
}

/***********************************************************************************************
  Function Name  : ~CAutoCriticalSection
  Description    : D'tor
  Author Name    : Ramkrushna Shelke
  Date           : 21 Jan 2014
***********************************************************************************************/
CAutoCriticalSection::~CAutoCriticalSection()
{
	m_rCritSect.Unlock();
}

/***********************************************************************************************
  Function Name  : CMaxSemaphore
  Description    : C'tor
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0074
  Date           : 22 Jan 2014
***********************************************************************************************/
CMaxSemaphore::CMaxSemaphore(int nMaxCount)
{
	m_nMaxCount = nMaxCount;
	m_hSemaphore = CreateSemaphore(
		NULL,           // default security attributes
		nMaxCount,  // initial count
		nMaxCount,  // maximum count
		NULL);
}

/***********************************************************************************************
  Function Name  : ~CMaxSemaphore
  Description    : D'tor
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0075
  Date           : 22 Jan 2014
***********************************************************************************************/
CMaxSemaphore::~CMaxSemaphore()
{
	ReleaseSemaphore(
		m_hSemaphore,  // handle to semaphore
		1,            // increase count by one
		NULL);       // not interested in previous count
	CloseHandle(m_hSemaphore);

}

/***********************************************************************************************
  Function Name  : Lock
  Description    : function to lock the resource using semaphore object.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0076
  Date           : 22 Jan 2014
***********************************************************************************************/
void CMaxSemaphore::Lock()
{
	WaitForSingleObject(m_hSemaphore, MAX_WAIT_COUNT);
}

/***********************************************************************************************
  Function Name  : Unlock
  Description    : function to release the resource using semaphore object.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0077
  Date           : 22 Jan 2014
***********************************************************************************************/
void CMaxSemaphore::Unlock()
{
	ReleaseSemaphore(
		m_hSemaphore,  // handle to semaphore
		1,            // increase count by one
		NULL);       // not interested in previous count
}

/***********************************************************************************************
  Function Name  : CAutoSemaphore
  Description    : C'tor
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0078
  Date           : 22 Jan 2014
***********************************************************************************************/
CAutoSemaphore::CAutoSemaphore(CMaxSemaphore& rSemaphore)
: m_rSemaphore(rSemaphore)
{
	m_rSemaphore.Lock();
}

/***********************************************************************************************
  Function Name  : ~CAutoSemaphore
  Description    : D'tor
  Author Name    : Ramkrushna Shelke
  SR.NO			 : WRDWIZCOMMON_0079
  Date           : 22 Jan 2014
***********************************************************************************************/
CAutoSemaphore::~CAutoSemaphore()
{
	m_rSemaphore.Unlock();
}

