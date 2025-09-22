/**********************************************************************************************************
* Program Name          :     WWizHttpManager.cpp
* Description           :     This is wrapper class for live update functionality such as
*							1) Connect to http server
*							2) Send request to http server
*							3) GetData from http server
*							4) Save received data in file.
* Author Name:                Ramkrushna Shelke
* Special Logic Used    :     used API's from <winhttp.h>
* for more information take help from MSDN.
* Modification Log        :
***********************************************************************************************************/
#pragma once
#include "stdafx.h"
#include <winhttp.h>
#include "DownloadConts.h"
#include "ISpyCriticalSection.h"

using namespace std;

const int MAX_RESPONSE_DRAIN_SIZE = 20*1024*1024;


class CWWizHttpManager
{
public:

	CWWizHttpManager(void);
	~CWWizHttpManager(void);

	TCHAR m_szHostName[MAX_PATH];
	TCHAR m_szMainUrl[URL_SIZE];
	DWORD m_dwCompletedDownloadBytes;
	HINTERNET  m_hSession;
	HINTERNET  m_hConnect;
	HINTERNET  m_hRequest;
	DWORD m_dwOpenRequestFlag;

	bool Initialize(LPCTSTR szHost,bool bCrackURL = true);
	bool GetHeaderInfo(int iQueryType, LPTSTR szHeaderInfo, DWORD &dwQueryBufLen);
	void GetDownloadStatus(DWORD & dwDownloaded);
	void SetDownloadStatus(DWORD  dwDownloaded);
	void PerformCleanup(ConnectionHandle enumConnHandle);
	bool Download(TCHAR * szLocalFileName, DWORD dwByteRangeStart, DWORD dwByteRangeStop);
	bool CreateRequestHandle(LPTSTR szURLPath = NULL);
	bool OpenRequestHandle();
	bool m_bSharedSession;
	static long m_lStopDownload;
	static void StopDownload();
	static void StartDownload();
	static bool CheckInternetConnection();
	bool GetProxyServerNameFromRegistry(WCHAR *);
	bool GetProxyServerDetailsFromDB();
	bool CheckProxySettings(LPCTSTR szHost);
	DWORD GetProxySettingsFromRegistry();
private:
	CISpyCriticalSection	 m_objCriticalSection;
	bool CrackURL(LPCTSTR szFullUrl);
	BOOL SetByteRange(DWORD dwByteRangeStart, DWORD dwByteRangeStop);
	void ReadWriteFile(HANDLE hFile);
	bool CheckProxySettings();
	DWORD m_dwProxySett;
	CString m_csServer;
	DWORD m_dwLastError;
	HINTERNET m_sessionHandle;
	wstring m_userAgent;
	wstring m_requestURL;
	wstring m_requestHost;
	unsigned int m_resolveTimeout;
	unsigned int m_connectTimeout;
	unsigned int m_sendTimeout;
	unsigned int m_receiveTimeout;
	BYTE *m_pResponse;
	CString m_csUserName;
	CString m_csPassword;
};
