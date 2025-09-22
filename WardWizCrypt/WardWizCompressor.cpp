#include "stdafx.h"
#include "WardWizCompressor.h"
#include <io.h>

//const int TIMER_STATAS = 1000;

CWardWizCompressor::CWardWizCompressor()
	{
		m_IsExtract = false;
		m_dwBufferSize = 0;
		m_dwDone = 0;
		m_dwFileSize = 0;
		m_isbProcesStart = false;
	}

CWardWizCompressor::~CWardWizCompressor()
{
}

bool CWardWizCompressor::CreateArchive(LPCTSTR lpszZipfilePath, LPCTSTR lpszActualFilePath)
{
	try
	{
		//CZipArchive zip;
		
		//m_zip.SetCallback(m_zip.GetCallback(CZipActionCallback::cbAdd), CZipActionCallback::cbAdd);
		m_zip.m_bIsAborted = false;
		bool bReslt = false;
		bReslt = m_zip.Open(lpszZipfilePath, CZipArchive::zipCreate);
		if (!bReslt)
			return false;

		//SetTimer(TIMER_CALL_BACK, 100, NULL);
		m_zip.AddNewFile(lpszActualFilePath);

		m_zip.CloseNew();
		//KillTimer(TIMER_CALL_BACK);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCompressor::CreateArchive", 0, 0, true, SECONDLEVEL);
	}

	return true;

}

bool CWardWizCompressor::ExtractArchive(LPCTSTR lpszZipfilePath, LPCTSTR lpszActualFilePath)
{
	try
	{
		//LPCTSTR zipFileName = _T("D:\\TanuWedsManu.mkv.zip");
		//CZipArchive zip;
		m_zip.m_bIsAborted = false;
		if (!m_zip.Open(lpszZipfilePath))
		{
			m_zip.CloseNew();
			return false;
		}

		TCHAR	szPath[256] = { 0 };
		TCHAR	szFile[256] = { 0 };
		wcscpy_s(szPath, 255, lpszActualFilePath);

		TCHAR	*pFileName = wcsrchr(szPath, '\\');

		if (!pFileName)
		{
			return false;
		}

		TCHAR	*pTemp = pFileName;

		pFileName++;
		wcscpy_s(szFile, 255, pFileName);

		*pTemp = '\0';
		m_IsExtract = true;
		bool bRet = true;
		//SetTimer(TIMER_CALL_BACK, 100, NULL);
		bRet = m_zip.ExtractFile(0, szPath, true, szFile);

		m_zip.CloseNew();

//		KillTimer(TIMER_CALL_BACK);
		m_IsExtract = false;
		//return false in case of abort de-compression
		// lalit kumawat 7-15-2015
		if (!bRet)
			return bRet;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardWizCompressor::ExtractArchive", 0, 0, true, SECONDLEVEL);
		m_zip.CloseNew();
		return false;
	}

	return true;
}


void CWardWizCompressor::GetStatus()
{
	//issue if file size more then 2 gb percentage on GUI getting more then 100%  because file size calculated by zipArchive is wrong.
	// issue resolved by lalit kumawat 7-3-2015

	//m_dwFileSize   =  m_zip.m_dwFileSize;
	m_dwBufferSize =  m_zip.m_dwBufferSize;
	m_dwDone       =  m_zip.m_dwDone;
	m_isbProcesStart = m_zip.m_bIsproStart;

}

void CWardWizCompressor::ResetStatusVariable()
{
	//issue if file size more then 2 gb percentage on GUI getting more then 100%  because file size calculated by zipArchive is wrong.
	// issue resolved by lalit kumawat 7-3-2015

	//m_dwFileSize   =  m_zip.m_dwFileSize;
	m_zip.m_dwBufferSize = 0;
	m_zip.m_dwDone = 0;
	m_zip.m_bIsproStart = false;

}

void CWardWizCompressor::StopOperation()
{
	m_zip.m_bIsAborted = true;
}