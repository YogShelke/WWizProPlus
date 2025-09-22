/**********************************************************************************************************                     
	  Program Name          :     WinHttpManager.h
	  Description           :     This is wrapper class for live update functionality such as
									1) Connect to http server
									2) Send request to http server
									3) GetData from http server
									4) Save received data in file.
	  Author Name:                Ramkrushna Shelke                                                                                           
	  Date Of Creation      :     11th Nov 2013
	  Version No            :     0.0.0.1
	  Special Logic Used    :     used API's from <winhttp.h>
								  for more information take help from MSDN.
	Modification Log      :               
	  1. Ramkrushna               Created basic fucntionality download       11 - 11 - 2013              
***********************************************************************************************************/
#pragma once
#include "stdafx.h"
#include <winhttp.h>

class CWinHttpManager
{
public:
	CWinHttpManager(void);
	virtual ~CWinHttpManager(void);
public:
	bool Initialize(LPCTSTR szHost);
	bool GetHeaderInfo(int iQueryType, LPTSTR szHeaderInfo, DWORD &dwBufferLength);
	bool CreateRequestHandle(LPTSTR szURLPath);
	bool Download(TCHAR * szLocalFileName, DWORD dwByteRangeStart, DWORD dwByteRangeStop);
	bool OpenRequestHandle();
	bool CrackURL(LPCTSTR szFullUrl);
	void PerformCleanup();
	void SetDownloadCompletedBytes(DWORD dwTotalBytes);
	DWORD GetDownloadCompletedBytes();
	bool CheckInternetConnection();
	bool WaitForInternetConnection();
	bool StopCurrentDownload();
	bool StartCurrentDownload();
	bool CloseFileHandles();
	bool GetProxyServerNameFromRegistry(WCHAR *);
	bool GetProxyServerDetailsFromDB();
    DWORD GetProxySettingsFromRegistry();
private:
	bool ReadWriteFile(HANDLE hFile);
	BOOL SetByteRange(DWORD dwByteRangeStart, DWORD dwByteRangeStop);
	bool CheckProxySettings(LPCTSTR szHost);
public:
	HINTERNET			m_hSession;
	HINTERNET			m_hConnect;
	HINTERNET			m_hRequest;
	TCHAR				m_szHostName[MAX_PATH];
	TCHAR				m_szMainUrl[MAX_PATH];
	TCHAR				m_szFullUrl[MAX_PATH];
	DWORD				m_dwCompletedDownloadBytes;
	bool				m_bIsConnected;
	DWORD				m_dwByteRangeStart;
	DWORD				m_dwByteRangeStop;
	DWORD				m_dwProxySett;
	CString				m_csServer;
	CString				m_csUserName;
	CString				m_csPassword;
	DWORD				m_dwOpenRequestFlag;
private:
	bool				m_bStopDownload;	
	DWORD				m_dwTotalDownloadBytes;
	HANDLE				m_hFile;
};