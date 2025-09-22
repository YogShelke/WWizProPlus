/**********************************************************************************************************                     
	  Program Name          : ISpyCriticalSection.h
	  Description           : Wrapper class for to use thread synchronization objects
	  Author Name			: Ramkrushna Shelke                                                                                           
	  Date Of Creation      : 21 Jan 2014
	  Version No            : 0.0.0.3
	  Special Logic Used    : 

	  Modification Log      :           
	  1. Ramkrushna         : Created wrapper to use thread synchronization objects.		21 Jan 2014
***********************************************************************************************************/
#pragma once

const int MAX_SEM_COUNT = 1;
const int MAX_WAIT_COUNT = 60000*2;

class CMaxSemaphore;
class CAutoCriticalSection;
class CAutoSemaphore;

class CISpyCriticalSection
{
public:
	CISpyCriticalSection();
	virtual ~CISpyCriticalSection();

	void Lock();
	void Unlock();

private:
	CRITICAL_SECTION m_CritSect;
};

class CMaxSemaphore
{
public:
	CMaxSemaphore(int nMaxCount = MAX_SEM_COUNT);
	virtual ~CMaxSemaphore();

	void Lock();
	void Unlock();

private:
	HANDLE m_hSemaphore;;
	int m_nMaxCount;
};

class CAutoCriticalSection
{
public:
	CAutoCriticalSection(CISpyCriticalSection& rCritSect);
	virtual ~CAutoCriticalSection();

private:
	CISpyCriticalSection& m_rCritSect;
};

class CAutoSemaphore
{
public:
	CAutoSemaphore(CMaxSemaphore& rSemaphore);
	virtual ~CAutoSemaphore();

private:
	CMaxSemaphore& m_rSemaphore;
};