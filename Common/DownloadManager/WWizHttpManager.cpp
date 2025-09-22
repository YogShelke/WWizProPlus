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
#include "StdAfx.h"
#include "WWizHttpManager.h"
#include <atlbase.h>

#ifdef UPDATESERVICE
#include "WardWizDatabaseInterface.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const TCHAR BROWSER_INFO[] = _T("Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; .NET CLR 2.0.50727; .NET CLR 3.0.04506.648; .NET CLR 3.5.21022; .NET CLR 1.0.3705; .NET CLR 1.1.4322; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729");
const WCHAR *WWIZHTTP_TYPES[]  = {L"Accept: image/gif", L"image/x-xbitmap", L"image/jpeg", L"image/pjpeg", L"application/x-shockwave-flash", L"application/x-ms-application", L"application/x-ms-xbap", L"application/vnd.ms-xpsdocument", L"application/xaml+xml", L"application/msword", L"application/vnd.ms-excel", L"application/x-cabinet-win32-x86",L"application/x-pe-win32-x86",L"application/octet-stream",L"application/x-setupscript", L"*/*", NULL};
long CWWizHttpManager::m_lStopDownload = 0;

static const int INT_BUFFERSIZE = 10240;    // Initial 10 KB temporary buffer, double if it is not enough.
static const unsigned int INT_RETRYTIMES = 3;
static wchar_t *SZ_AGENT = L"VibraniumClient";

/***************************************************************************
Function       : CheckForInternetConnection
In Parameters  : void
Out Parameters : bool
Description    : Constructor
Author		   : Ram Shelke
****************************************************************************/
CWWizHttpManager::CWWizHttpManager(void):
m_connectTimeout(60000),
m_sendTimeout(30000),
m_userAgent(SZ_AGENT),
m_requestHost(L""),
m_pResponse(NULL),
m_receiveTimeout(30000)
{
	m_hSession = NULL;
	m_hConnect = NULL;
	m_hRequest = NULL;
	m_dwCompletedDownloadBytes = 0;
	m_bSharedSession = false;
	m_lStopDownload = 0;
	m_dwProxySett = 0X00;
	m_csServer = L"";
	m_csUserName = L"";
	m_csPassword = L"";
	m_dwOpenRequestFlag = 0x00;
	GetProxyServerDetailsFromDB();
}

/***************************************************************************
Function       : ~CWWizHttpManager
In Parameters  : void, 
Out Parameters : 
Description    : destructor
Author		   : Ram Shelke
****************************************************************************/
CWWizHttpManager::~CWWizHttpManager(void)
{
	PerformCleanup(eSessionHandle);
}

/***************************************************************************
Function       : CrackURL
In Parameters  : TCHAR * szFullUrl, 
Out Parameters : bool 
Description    : Crack given url
Author		   : Ram Shelke
****************************************************************************/
bool CWWizHttpManager::CrackURL(LPCTSTR szFullUrl)
{
	__try
	{
		URL_COMPONENTS urlComp;
		size_t nURLLen = 0;

		wmemset(m_szMainUrl, 0, URL_SIZE);
		wmemset(m_szHostName, 0, MAX_PATH);

		// Initialize the URL_COMPONENTS structure.
		ZeroMemory(&urlComp, sizeof(urlComp));
		urlComp.dwStructSize = sizeof(urlComp);

		// Set required component lengths to non-zero so that they are cracked.
		urlComp.dwSchemeLength = -1;
		urlComp.dwHostNameLength = -1;
		urlComp.dwUrlPathLength = -1;
		urlComp.dwExtraInfoLength = -1;
		
		// Crack the URL.
		if (WinHttpCrackUrl(szFullUrl, (DWORD)_tcslen(szFullUrl), 0, &urlComp))
		{
			if (urlComp.nScheme == INTERNET_SCHEME_HTTPS)
			{
				m_dwOpenRequestFlag = WINHTTP_FLAG_SECURE;
			}
			_tcscpy_s(m_szMainUrl, URL_SIZE, urlComp.lpszUrlPath);
			nURLLen = _tcslen(urlComp.lpszUrlPath);
			size_t nHostLen = _tcslen(urlComp.lpszHostName) - nURLLen;
			_tcsncpy_s(m_szHostName, MAX_PATH, urlComp.lpszHostName, nHostLen);
			return true;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumHttpManager::CrackURL, URL: [%s]", szFullUrl, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function       : PerformCleanup
In Parameters  : ConnectionHandle enumConnHandle, 
Out Parameters : void 
Description    : cleanup handles
Author		   : Ram Shelke
****************************************************************************/
void CWWizHttpManager::PerformCleanup(ConnectionHandle enumConnHandle)
{
	__try
	{
		if (enumConnHandle == eRequestHandle || enumConnHandle == eConnectHandle
			|| enumConnHandle == eSessionHandle)
		{
			if (m_hRequest)
			{
				WinHttpCloseHandle(m_hRequest);
				m_hRequest = NULL;
			}
		}
		if (enumConnHandle != eRequestHandle)
		{
			if (!m_bSharedSession)
			{
				if (m_hSession)
				{
					WinHttpCloseHandle(m_hSession);
					m_hSession = NULL;
				}
				if (m_hConnect)
				{
					WinHttpCloseHandle(m_hConnect);
					m_hConnect = NULL;
				}
			}
			else
			{
				m_hSession = NULL;
				m_hConnect = NULL;
				m_bSharedSession = false;
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumHttpManager::PerformCleanup", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function       : Initialize
In Parameters  : LPCTSTR szHost, 
Out Parameters : bool 
Description    : Initialize internet
Author		   : Ram Shelke
****************************************************************************/
bool CWWizHttpManager::Initialize(LPCTSTR szHost, bool bCrackURL)
{
	__try
	{
		if (bCrackURL)
		{
			if (CrackURL(szHost) == false)
			{
				AddLogEntry(_T("### Initialize CrackURL connection failed for URL %s"), szHost, 0, true, SECONDLEVEL);
				return false;
			}
		}
		else
		{
			_tcscpy_s(m_szHostName, szHost);
		}
		
		// Use WinHttpOpen to obtain a session handle.
		if (m_hSession == NULL)
		{
			if (!CheckProxySettings(szHost))
			{
				m_hSession = ::WinHttpOpen(SZ_AGENT,
					WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
					WINHTTP_NO_PROXY_NAME,
					WINHTTP_NO_PROXY_BYPASS,
					0);

				if (m_hSession == NULL)
				{
					return false;
				}

				// Specify an HTTP server.
				if (m_hSession)
				{
					INTERNET_PORT intPort = INTERNET_DEFAULT_HTTP_PORT;
					if (m_dwOpenRequestFlag == WINHTTP_FLAG_SECURE)
						intPort = INTERNET_DEFAULT_HTTPS_PORT;
					
					m_hConnect = WinHttpConnect(m_hSession, m_szHostName,
						intPort, 0);
					return true;
				}
			}
		}

		if (m_hSession == NULL)
		{
			m_hSession = WinHttpOpen(SZ_AGENT, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME,
				WINHTTP_NO_PROXY_BYPASS, 0);

			// Specify an HTTP server.
			if (m_hSession)
			{
				INTERNET_PORT intPort = INTERNET_DEFAULT_HTTP_PORT;
				if (m_dwOpenRequestFlag == WINHTTP_FLAG_SECURE)
					intPort = INTERNET_DEFAULT_HTTPS_PORT;

				m_hConnect = WinHttpConnect(m_hSession, m_szHostName, intPort, 0);
				return true;
			}
		}
		else
		{
			return true;
		}
		AddLogEntry(_T("### Initialize connection failed for URL %s"), szHost, 0, true, SECONDLEVEL);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumHttpManager::Initialize", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function       : CheckProxySettings
In Parameters  : void
Out Parameters : bool
Description    : Function which checks for proxy settings
Author		   : Ram Shelke
****************************************************************************/
bool CWWizHttpManager::CheckProxySettings(LPCTSTR szHost)
{
	try
	{
		WCHAR szProxyServer[MAX_PATH] = { 0 };

#ifdef UPDATESERVICE
		if (m_dwProxySett == 0X02)
		{			
			wcscpy(szProxyServer, m_csServer.GetString());
		}
		else
			return false;
#else		
		GetProxyServerNameFromRegistry(szProxyServer);
#endif

		m_hSession = ::WinHttpOpen(SZ_AGENT,
			WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
			WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS,
			0);

		if (m_hSession == NULL)
		{
			return false;
		}

		::WinHttpSetTimeouts(m_hSession,
			0,
			60000,
			30000,
			30000);

		wchar_t szHostName[MAX_PATH] = L"";
		wchar_t szURLPath[MAX_PATH * 5] = L"";
		URL_COMPONENTS urlComp;
		memset(&urlComp, 0, sizeof(urlComp));
		urlComp.dwStructSize = sizeof(urlComp);
		urlComp.lpszHostName = szHostName;
		urlComp.dwHostNameLength = MAX_PATH;
		urlComp.lpszUrlPath = szURLPath;
		urlComp.dwUrlPathLength = MAX_PATH * 5;
		urlComp.dwSchemeLength = 1; // None zero

		if (::WinHttpCrackUrl(szHost, static_cast<DWORD>(wcslen(szHost)), 0, &urlComp))
		{
			m_hConnect = ::WinHttpConnect(m_hSession, szHostName, urlComp.nPort, 0);
			if (m_hConnect != NULL)
			{
				DWORD dwOpenRequestFlag = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;
				m_hRequest = ::WinHttpOpenRequest(m_hConnect,
					L"GET",
					urlComp.lpszUrlPath,
					NULL,
					WINHTTP_NO_REFERER,
					WINHTTP_DEFAULT_ACCEPT_TYPES,
					m_dwOpenRequestFlag);
			}

			if (m_hRequest)
			{
				DWORD m_dwLastError;
				WINHTTP_PROXY_INFO proxyInfo;
				memset(&proxyInfo, 0, sizeof(proxyInfo));
				proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
				wchar_t szProxy[MAX_PATH] = L"";
				wcscpy_s(szProxy, MAX_PATH, szProxyServer);
				proxyInfo.lpszProxy = szProxy;

				if (!::WinHttpSetOption(m_hRequest, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo)))
				{
					m_dwLastError = ::GetLastError();
				}

				if (m_csUserName.GetLength() > 0)
				{
					if (!::WinHttpSetOption(m_hRequest, WINHTTP_OPTION_PROXY_USERNAME, (LPVOID)m_csUserName.GetBuffer(), static_cast<DWORD>(wcslen(m_csUserName)) * sizeof(wchar_t)))
					{
						m_dwLastError = ::GetLastError();
					}
				}

				if (m_csPassword.GetLength() > 0)
				{
					if (!::WinHttpSetOption(m_hRequest, WINHTTP_OPTION_PROXY_PASSWORD, (LPVOID)m_csPassword.GetBuffer(), static_cast<DWORD>(wcslen(m_csPassword)) * sizeof(wchar_t)))
					{
						m_dwLastError = ::GetLastError();
					}
				}

				BOOL bResults = FALSE;
				// Send a request.
				bResults = WinHttpSendRequest(m_hRequest,
					WINHTTP_NO_ADDITIONAL_HEADERS,
					0, WINHTTP_NO_REQUEST_DATA, 0,
					0, 0);
				// End the request.			
				if (bResults)
					bResults = WinHttpReceiveResponse(m_hRequest, NULL);

				if (bResults)
				{
					DWORD dwStatusCode = 0;
					DWORD dwTemp = sizeof(dwStatusCode);
					BOOL bResult = FALSE;
					DWORD dwSize = 0;
					bResult = ::WinHttpQueryHeaders(m_hRequest,
						WINHTTP_QUERY_RAW_HEADERS_CRLF,
						WINHTTP_HEADER_NAME_BY_INDEX,
						NULL,
						&dwSize,
						WINHTTP_NO_HEADER_INDEX);

					if (bResult || (!bResult && (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)))
					{
						wchar_t *szHeader = new wchar_t[dwSize];
						if (szHeader != NULL)
						{
							memset(szHeader, 0, dwSize* sizeof(wchar_t));
							if (::WinHttpQueryHeaders(m_hRequest,
								WINHTTP_QUERY_RAW_HEADERS_CRLF,
								WINHTTP_HEADER_NAME_BY_INDEX,
								szHeader,
								&dwSize,
								WINHTTP_NO_HEADER_INDEX))
							{
							}
							delete[] szHeader;
						}
					}

					dwSize = 0;
					bResult = ::WinHttpQueryHeaders(m_hRequest,
						WINHTTP_QUERY_STATUS_CODE,
						WINHTTP_HEADER_NAME_BY_INDEX,
						NULL,
						&dwSize,
						WINHTTP_NO_HEADER_INDEX);

					bool bSuccess = false;
					if (bResult || (!bResult && (::GetLastError() == ERROR_INSUFFICIENT_BUFFER)))
					{
						wchar_t *szStatusCode = new wchar_t[dwSize];
						if (szStatusCode != NULL)
						{
							memset(szStatusCode, 0, dwSize* sizeof(wchar_t));
							if (::WinHttpQueryHeaders(m_hRequest,
								WINHTTP_QUERY_STATUS_CODE,
								WINHTTP_HEADER_NAME_BY_INDEX,
								szStatusCode,
								&dwSize,
								WINHTTP_NO_HEADER_INDEX))
							{
							}

							if (wcscmp(szStatusCode, L"200") == 0)
							{
								bSuccess = true;
							}
							delete[] szStatusCode;
						}
					}

					if (bSuccess)
					{
						return true;
					}
				}
			}
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWinHttpManager::CheckProxySettings", 0, 0, true, SECONDLEVEL);
	}
	return false;
}


/***************************************************************************
Function       : GetProxyServerNameFromRegistry
In Parameters  : WCHAR *szProxyServer
Out Parameters : bool
Description    : Function to get Proxy settings from registry.
Author		   : Kunal
****************************************************************************/
bool CWWizHttpManager::GetProxyServerNameFromRegistry(WCHAR *szProxyServer)
{
	if (!szProxyServer)
		return false;

	CRegKey oRegKey;
	if (oRegKey.Open(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings", KEY_READ) != ERROR_SUCCESS)
	{
		return false;
	}

	DWORD dwProxyEnabled = 0;
	if (oRegKey.QueryDWORDValue(L"ProxyEnable", dwProxyEnabled) != ERROR_SUCCESS)
	{
		oRegKey.Close();
		return false;
	}
	if (dwProxyEnabled == 0)
	{
		oRegKey.Close();
		return false;
	}

	ULONG ulBytes = MAX_PATH;
	if (oRegKey.QueryStringValue(L"ProxyServer", szProxyServer, &ulBytes) != ERROR_SUCCESS)
	{
		oRegKey.Close();
		return false;
	}

	oRegKey.Close();

	return true;
}


/***************************************************************************
Function       : GetHeaderInfo
In Parameters  : int iQueryType, TCHAR * szResults, 
Out Parameters : bool 
Description    : his function will create the session, and will query for WINHTTP_HEADER_NAME_BY_INDEX
Author		   : Ram Shelke
****************************************************************************/
bool CWWizHttpManager::GetHeaderInfo(int iQueryType, LPTSTR szHeaderInfo, DWORD &dwQueryBufLen)
{
	__try
	{
		if (m_hConnect == NULL)
		{
			return false;
		}
		if (!m_hRequest)
		{
			if (!CreateRequestHandle(m_szMainUrl))
			{
				AddLogEntry(L"### Invalid URL:[%s]", m_szMainUrl, 0, true, SECONDLEVEL);
				return false;
			}
		}

		if (m_hRequest)
		{
			if(WinHttpQueryHeaders(m_hRequest, iQueryType, WINHTTP_HEADER_NAME_BY_INDEX,
				szHeaderInfo, &dwQueryBufLen, WINHTTP_NO_HEADER_INDEX))
				return true;

		}
		AddLogEntry(L"### WinHttpReceiveResponse failed to connect to host:[%s]", m_szHostName, 0, true, SECONDLEVEL);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumHttpManager::GetHeaderInfo", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function       : GetDownloadStatus
In Parameters  : DWORD & dwDownloaded, 
Out Parameters : void 
Description    : retrive download status
Author		   : Ram Shelke
****************************************************************************/
void CWWizHttpManager::GetDownloadStatus(DWORD & dwDownloaded)
{
	m_objCriticalSection.Lock();
	dwDownloaded = m_dwCompletedDownloadBytes;
	m_objCriticalSection.Unlock();
}

/***************************************************************************
Function       : SetDownloadStatus
In Parameters  : DWORD  dwDownloaded, 
Out Parameters : void 
Description    : set download status
Author		   : Ram Shelke
****************************************************************************/
void CWWizHttpManager::SetDownloadStatus(DWORD  dwDownloaded)
{
	m_objCriticalSection.Lock();
	m_dwCompletedDownloadBytes = dwDownloaded;
	m_objCriticalSection.Unlock();
}

/***************************************************************************
Function       : CreateRequestHandle
In Parameters  : 
Out Parameters : bool 
Description    : create the internet request handle
Author		   : Ram Shelke
****************************************************************************/
bool CWWizHttpManager::CreateRequestHandle(LPTSTR pszURLPath)
{
	if (CWWizHttpManager::m_lStopDownload)
	{
		return false;
	}

	// Create an HTTP request handle.
	BOOL bResults = FALSE;
	__try
	{
		if (m_hRequest)
		{
			return true;
		}

		if (pszURLPath)
		{
			_tcscpy_s(m_szMainUrl, pszURLPath);
		}

		if (m_hConnect)
		{
			m_hRequest = WinHttpOpenRequest(m_hConnect, L"GET", m_szMainUrl, NULL, WINHTTP_NO_REFERER, WWIZHTTP_TYPES, m_dwOpenRequestFlag);
			if (m_dwProxySett == 0X02)
			{
				DWORD dwLastError;
				WINHTTP_PROXY_INFO proxyInfo;
				memset(&proxyInfo, 0, sizeof(proxyInfo));
				proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
				wchar_t szProxy[MAX_PATH] = L"";
				wcscpy_s(szProxy, MAX_PATH, m_csServer.GetString());
				proxyInfo.lpszProxy = szProxy;

				if (!::WinHttpSetOption(m_hRequest, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo)))
				{
					dwLastError = ::GetLastError();
				}

				if (m_csUserName.GetLength() > 0)
				{
					if (!::WinHttpSetOption(m_hRequest, WINHTTP_OPTION_PROXY_USERNAME, (LPVOID)m_csUserName.GetBuffer(), static_cast<DWORD>(wcslen(m_csUserName)) * sizeof(wchar_t)))
					{
						dwLastError = ::GetLastError();
					}
				}

				if (m_csPassword.GetLength() > 0)
				{
					if (!::WinHttpSetOption(m_hRequest, WINHTTP_OPTION_PROXY_PASSWORD, (LPVOID)m_csPassword.GetBuffer(), static_cast<DWORD>(wcslen(m_csPassword)) * sizeof(wchar_t)))
					{
						dwLastError = ::GetLastError();
					}
				}
			}
		}

		if (m_hRequest)
		{
			// Send a request.
			bResults = WinHttpSendRequest(m_hRequest,
				WINHTTP_NO_ADDITIONAL_HEADERS,
				0, WINHTTP_NO_REQUEST_DATA, 0,
				0, 0);
			// End the request.
			if (bResults)
				bResults = WinHttpReceiveResponse(m_hRequest, NULL);

			if (bResults)
			{
				DWORD dwStatusCode = 0;
				DWORD dwTemp = sizeof(dwStatusCode);
				WinHttpQueryHeaders(m_hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, NULL, &dwStatusCode, &dwTemp, NULL);
				if ((dwStatusCode >= HTTP_STATUS_CONTINUE) && (dwStatusCode < HTTP_STATUS_BAD_REQUEST))
				{
					return true;
				}
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumHttpManager::CreateRequestHandle", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function       : SetByteRange
In Parameters  : DWORD dwByteRangeStart, DWORD dwByteRangeStop, 
Out Parameters : BOOL 
Description    : Set the byte range
Author		   : Ram Shelke
****************************************************************************/
BOOL CWWizHttpManager::SetByteRange(DWORD dwByteRangeStart, DWORD dwByteRangeStop)
{
	BOOL bResults = FALSE;
	__try
	{
		if (CWWizHttpManager::m_lStopDownload)
		{
			return FALSE;
		}
		TCHAR szByteRange[MAX_PATH] = { 0 };
		_stprintf_s(szByteRange, MAX_PATH, _T("Range: bytes=%ld-%ld"), dwByteRangeStart, dwByteRangeStop);
		if (m_hRequest)
		{
			bResults = WinHttpAddRequestHeaders(m_hRequest, szByteRange, -1, WINHTTP_ADDREQ_FLAG_ADD);
		}

		// Send a request.
		if (m_hRequest)
		{
			bResults = WinHttpSendRequest(m_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
		}

		if (!bResults)
		{
			AddLogEntry(L"### SetByteRange Handle failed for: %s:%s ", m_szHostName, m_szMainUrl, true, SECONDLEVEL);
			return bResults;
		}

		// End the request.
		bResults = WinHttpReceiveResponse(m_hRequest, NULL);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumHttpManager::SetByteRange", 0, 0, true, SECONDLEVEL);
	}
	return bResults;
}

/***************************************************************************
Function       : ReadWriteFile
In Parameters  : HANDLE hFile, 
Out Parameters : bool 
Description    : Read Write file from internet
Author		   : Ram Shelke
****************************************************************************/
void CWWizHttpManager::ReadWriteFile(HANDLE hFile)
{
	try
	{
		DWORD dwSize = 0;
		DWORD dwDownloaded = 0;
		LPSTR pszOutBuffer = NULL;
		do
		{
			if (CWWizHttpManager::m_lStopDownload)
			{
				return;
			}
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(m_hRequest, &dwSize))
			{
				AddLogEntry(L"### Host Connection Error: %s:%s ", m_szHostName, m_szMainUrl, true, SECONDLEVEL);
				return;
			}
			if (dwSize == 0)
			{
				return;
			}
			pszOutBuffer = new char[dwSize + 1];				// Allocate space for the buffer.
			ZeroMemory(pszOutBuffer, dwSize + 1);				// Read the Data.

			if (!WinHttpReadData(m_hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
			{
				CString csLAstError;
				csLAstError.Format(_T("%d"), ::GetLastError());
				AddLogEntry(L"### WinHttpReadData failed %s:%s ", m_szHostName, m_szMainUrl, true, SECONDLEVEL);
				if (pszOutBuffer)
				{
					delete[] pszOutBuffer;
					pszOutBuffer = NULL;
				}
				return;
			}
			DWORD dwBytesWritten = 0;
			if (FALSE == WriteFile(hFile, pszOutBuffer, dwDownloaded, &dwBytesWritten, NULL))
			{
				if (pszOutBuffer)
				{
					delete[] pszOutBuffer;
					pszOutBuffer = NULL;
				}
				return;
			}
			FlushFileBuffers(hFile);
			if (pszOutBuffer)
			{
				delete[] pszOutBuffer;
				pszOutBuffer = NULL;
			}
			m_objCriticalSection.Lock();
			m_dwCompletedDownloadBytes += dwBytesWritten;
			m_objCriticalSection.Unlock();

		} while ((dwSize > 0) && (!m_lStopDownload));
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ", 0, 0, true, SECONDLEVEL);
	}
	return;
}

/***************************************************************************
Function       : OpenRequestHandle
In Parameters  : HANDLE hFile,
Out Parameters : bool
Description    : this funtion will Open the Request handle and will return true or false.
Author		   : Ram Shelke
****************************************************************************/
bool CWWizHttpManager::OpenRequestHandle()
{
	__try
	{
		if (m_hRequest)
		{
			WinHttpCloseHandle(m_hRequest);
			m_hRequest = NULL;
		}

		if (m_hConnect)
		{
			m_hRequest = WinHttpOpenRequest(m_hConnect, L"GET", m_szMainUrl, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, m_dwOpenRequestFlag);
			if (m_dwProxySett == 0X02)
			{
				DWORD m_dwLastError;
				WINHTTP_PROXY_INFO proxyInfo;
				memset(&proxyInfo, 0, sizeof(proxyInfo));
				proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
				wchar_t szProxy[MAX_PATH] = L"";
				wcscpy_s(szProxy, MAX_PATH, m_csServer.GetString());
				proxyInfo.lpszProxy = szProxy;

				if (!::WinHttpSetOption(m_hRequest, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo)))
				{
					m_dwLastError = ::GetLastError();
				}

				if (m_csUserName.GetLength() > 0)
				{
					if (!::WinHttpSetOption(m_hRequest, WINHTTP_OPTION_PROXY_USERNAME, (LPVOID)m_csUserName.GetBuffer(), static_cast<DWORD>(wcslen(m_csUserName)) * sizeof(wchar_t)))
					{
						m_dwLastError = ::GetLastError();
					}
				}

				if (m_csPassword.GetLength() > 0)
				{
					if (!::WinHttpSetOption(m_hRequest, WINHTTP_OPTION_PROXY_PASSWORD, (LPVOID)m_csPassword.GetBuffer(), static_cast<DWORD>(wcslen(m_csPassword)) * sizeof(wchar_t)))
					{
						m_dwLastError = ::GetLastError();
					}
				}
			}
		}

		if (!m_hRequest)
		{
			return false;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumHttpManager::OpenRequestHandle", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
Function       : Download
In Parameters  : TCHAR * szLocalFileName, DWORD dwByteRangeStart, 
				DWORD dwByteRangeStop, 
Out Parameters : bool 
Description    : Start download files
Author		   : Ram Shelke
****************************************************************************/
bool CWWizHttpManager::Download(TCHAR * szLocalFileName, DWORD dwByteRangeStart,
							DWORD dwByteRangeStop)
{
	bool bReturn = false;
	__try
	{
		if (OpenRequestHandle() == false)
		{
			return bReturn;
		}

		if (SetByteRange(dwByteRangeStart, dwByteRangeStop) == FALSE)
		{

			return bReturn;
		}

		HANDLE hFile = NULL;
		if (PathFileExists(szLocalFileName))
		{
			hFile = CreateFile(szLocalFileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			SetFilePointer(hFile, m_dwCompletedDownloadBytes, NULL, FILE_BEGIN);
		}
		else
		{
			hFile = CreateFile(szLocalFileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		}

		if (hFile)
		{
			ReadWriteFile(hFile);
			CloseHandle(hFile);
			hFile = NULL;
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CVibraniumHttpManager::Download, FilePath: [%s]", szLocalFileName, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
Function       : StopDownload
In Parameters  : void
Out Parameters : void
Description    : Function to stop download operation.
Author		   : Ram Shelke
****************************************************************************/
void CWWizHttpManager::StopDownload()
{
	::InterlockedIncrement(&CWWizHttpManager::m_lStopDownload);
}

/***************************************************************************
Function       : StartDownload
In Parameters  : void
Out Parameters : void
Description    : Function to start download operation.
Author		   : Ram Shelke
****************************************************************************/
void CWWizHttpManager::StartDownload()
{
	::InterlockedExchange(&CWWizHttpManager::m_lStopDownload, 0);
}

/***************************************************************************
Function       : CheckInternetConnection
In Parameters  : void
Out Parameters : void
Description    : Function to check internet connection.
Author		   : Ram Shelke
****************************************************************************/
bool CWWizHttpManager::CheckInternetConnection()
{
	bool bRet = false;
	try
	{
		CWWizHttpManager objTempMgr;
		TCHAR szURL[MAX_PATH] = _T("http://www.google.com/");
		if (objTempMgr.Initialize(szURL, true))
		{
			if (objTempMgr.CreateRequestHandle(NULL))
			{
				bRet = true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumHttpManager::CheckInternetConnection", 0, 0, true, SECONDLEVEL);
	}
	return bRet;
}

/***************************************************************************
Function       : GetProxyServerDetailsFromDB
Description    : This function will get Proxy details here from sqlite DB
Author Name    : Kunal
Date           : 2ndth Nov 2018
****************************************************************************/
bool CWWizHttpManager::GetProxyServerDetailsFromDB()
{
	try
	{
#ifdef UPDATESERVICE
		m_csServer = EMPTY_STRING;

		//get Proxy details here from sqlite DB
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csSettingsDB = L"";
		csSettingsDB.Format(L"%sVBSETTINGS.DB", csWardWizModulePath);
		if (!PathFileExists(csSettingsDB))
		{
			return false;
		}
		
		m_dwProxySett = 0X00;
		DWORD dwProxy_Sett = GetProxySettingsFromRegistry();
		m_dwProxySett = dwProxy_Sett;
		if (dwProxy_Sett != 0x02)
		{
			return false;
		}

		INTERNET_PORT nPort;
		CT2A dbPath(csSettingsDB, CP_UTF8);
		CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);
		objSqlDb.Open();
		//CWardwizSQLiteTable qResult = objSqlDb.GetTable("SELECT PROXY_SETTINGS, SERVER, PORT, USERNAME, PASSWORD FROM WWIZPROXYSETTINGS;");
		CWardwizSQLiteTable qResult = objSqlDb.GetTable("SELECT SERVER, PORT, USERNAME, PASSWORD FROM WWIZPROXYSETTINGS;");
		DWORD dwRows = qResult.GetNumRows();

		if (dwRows != 0)
		{
			/*const char *pszEnable = qResult.GetFieldValue(0);
			if (!pszEnable)
				return false;

			if (strlen(pszEnable) > 0)
			{
				dwProxy_Sett = atoi(pszEnable);
			}

			m_dwProxySett = dwProxy_Sett;

			if (dwProxy_Sett != 0x02)
			{
				objSqlDb.Close();
				return false;
			}*/

			m_csServer = qResult.GetFieldValue(0);
			const char *pszPort = qResult.GetFieldValue(1);
			if (!pszPort)
				return false;

			if (strlen(pszPort) == 0x00)
			{
				objSqlDb.Close();
				return false;
			}

			nPort = atoi(pszPort);
			
			//to read unicode string from db
			int wchars_num = MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(2), -1, NULL, 0);
			wchar_t *wstrDbData = new wchar_t[wchars_num];
			if (wstrDbData == NULL)
			{
				AddLogEntry(L"### Failed to allocate memory in CVibraniumHttpManager::GetProxyServerDetailsFromDB", 0, 0, true, SECONDLEVEL);
				objSqlDb.Close();
				return false;
			}
			MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(2), -1, wstrDbData, wchars_num);
			m_csUserName = wstrDbData;
			delete[] wstrDbData;

			//to read unicode string from db
			wchars_num = MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(3), -1, NULL, 0);
			wstrDbData = new wchar_t[wchars_num];
			if (wstrDbData == NULL)
			{
				AddLogEntry(L"### Failed to allocate memory in CVibraniumHttpManager::GetProxyServerDetailsFromDB", 0, 0, true, SECONDLEVEL);
				objSqlDb.Close();
				return false;
			}
			MultiByteToWideChar(CP_UTF8, 0, qResult.GetFieldValue(3), -1, wstrDbData, wchars_num);
			m_csPassword = wstrDbData;
			delete[] wstrDbData;
		}

		objSqlDb.Close();
		if (dwProxy_Sett == 0X02 && dwRows > 0)
		{
			m_csServer.AppendFormat(L":%d", nPort);
			return true;
		}
#endif
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CVibraniumzHttpManager::GetProxyServerDetailsFromDB", 0, 0, true, SECONDLEVEL);
		return false;
	}	
	return false;
}

/***************************************************************************
Function       : GetProxySettingsFromRegistry
Description    : This function to get settings which user has set
Author Name    : Kunal
Date           : 2nd Nov 2018
****************************************************************************/
DWORD CWWizHttpManager::GetProxySettingsFromRegistry()
{
	DWORD dwRet = 0x00;
	try
	{
		CRegKey oRegKey;
		if (oRegKey.Open(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Vibranium", KEY_READ) != ERROR_SUCCESS)
		{
			return 0x00;
		}

		DWORD dwProxySett = 0x00;
		if (oRegKey.QueryDWORDValue(L"dwProxySett", dwProxySett) != ERROR_SUCCESS)
		{
			oRegKey.Close();
			return 0x00;
		}

		//0x01 - use internet explorer settings will be added in future
		if (dwProxySett == 0x01)
		{
			dwProxySett = 0x00;
		}

		dwRet = dwProxySett;

		oRegKey.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWinHttpManager::GetProxySettingsFromRegistry", 0, 0, true, SECONDLEVEL);
	}
	return dwRet;
}