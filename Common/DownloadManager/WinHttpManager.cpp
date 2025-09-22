/**********************************************************************************************************                     
	  Program Name          :     WinHttpManager.cpp
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
	Modification Log        :               
	  1. Ramkrushna               Created basic fucntionality download       11 - 11 - 2013              
***********************************************************************************************************/
#include "stdafx.h"
#include "WinHttpManager.h"

#ifdef UPDATESERVICE
#include "WardWizDatabaseInterface.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const TCHAR BROWSER_INFO[] = _T("Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; .NET CLR 2.0.50727; .NET CLR 3.0.04506.648; .NET CLR 3.5.21022; .NET CLR 1.0.3705; .NET CLR 1.1.4322; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729");
const WCHAR *HTTP_TYPES[]  = {L"Accept: image/gif", L"image/x-xbitmap", L"image/jpeg", L"image/pjpeg", L"application/x-shockwave-flash", L"application/x-ms-application", L"application/x-ms-xbap", L"application/vnd.ms-xpsdocument", L"application/xaml+xml", L"application/msword", L"application/vnd.ms-excel", L"application/x-cabinet-win32-x86",L"application/x-pe-win32-x86",L"application/octet-stream",L"application/x-setupscript", L"*/*", NULL};

static wchar_t *SZ_AGENT = L"VibraniumClient";

/***************************************************************************
Function Name  : CWinHttpManager
Description    : C'tor
Author Name    : Ramkrushna Shelke
Date           : 11th Nov 2013
****************************************************************************/
CWinHttpManager::CWinHttpManager()
	:m_bStopDownload(false)
	,m_hSession(NULL)
	,m_hConnect(NULL)
	,m_hRequest(NULL)
	,m_dwCompletedDownloadBytes(0)
	,m_dwTotalDownloadBytes(0)
	,m_bIsConnected(false)
	,m_dwByteRangeStart(0)
	,m_dwByteRangeStop(0)
	,m_hFile(NULL)
	,m_dwProxySett (0X00)
	,m_csUserName(L"")
	,m_csPassword(L"")
	, m_csServer(L"")
	, m_dwOpenRequestFlag(0x00)
{
	memset(m_szHostName, 0, MAX_PATH);
	memset(m_szMainUrl, 0, MAX_PATH);
	memset(m_szFullUrl, 0, MAX_PATH);
}

/***************************************************************************
  Function Name  : ~CWinHttpManager
  Description    : D'tor
  Author Name    : Ramkrushna Shelke
  Date           : 11th Nov 2013
****************************************************************************/
CWinHttpManager::~CWinHttpManager()
{
	PerformCleanup();
}

/***************************************************************************
  Function Name  : PerformCleanup
  Description    : This function is used to clean up the allocated memory and to close
				   all opened handles.
  Author Name    : Ramkrushna Shelke
  Date           : 11th Nov 2013
****************************************************************************/
void CWinHttpManager::PerformCleanup()
{
	if(m_hSession != NULL)
	{
		WinHttpCloseHandle(m_hSession);
		m_hSession = NULL;
	}
	if(m_hConnect != NULL)
	{
		WinHttpCloseHandle(m_hConnect);
		m_hConnect = NULL;
	}
	if(m_hRequest != NULL)
	{
		WinHttpCloseHandle(m_hRequest);
		m_hRequest = NULL;
	}
}

/***************************************************************************
  Function Name  : Initialize
  Description    : This function is used to initialize the http session
				   as well as to crack url for host name.
  Author Name    : Ramkrushna Shelke
  Date           : 11th Nov 2013
****************************************************************************/
bool CWinHttpManager::Initialize(LPCTSTR szHost)
{
	bool bReturn = false;

	m_dwCompletedDownloadBytes = 0;
	PerformCleanup();

	if(CrackURL(szHost) == false)
	{
		AddLogEntry(_T("***CWinHttpManager::Initialize CrackURL connection failed for URL %s"), szHost);
		return false;
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
				
				m_hConnect = WinHttpConnect(m_hSession, m_szHostName, intPort, 0);
				return true;
			}
		}
	}

	if(m_hSession == NULL)
	{
		m_hSession = WinHttpOpen(SZ_AGENT, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME,
			WINHTTP_NO_PROXY_BYPASS, 0);

		// Specify an HTTP server.
		if(m_hSession)
		{
			INTERNET_PORT intPort = INTERNET_DEFAULT_HTTP_PORT;
			if (m_dwOpenRequestFlag == WINHTTP_FLAG_SECURE)
				intPort = INTERNET_DEFAULT_HTTPS_PORT;
			
			m_hConnect = WinHttpConnect(m_hSession, m_szHostName, intPort, 0);
			bReturn = true;
		}
	}
	else
	{
		bReturn = true;
	}
	return bReturn;
}

/***************************************************************************
  Function Name  : GetHeaderInfo
  Description    : This function will create the session, and will query for 
				   WINHTTP_HEADER_NAME_BY_INDEX
  Author Name    : Ramkrushna Shelke
  Date           : 11th Nov 2013
****************************************************************************/
bool CWinHttpManager::GetHeaderInfo(int iQueryType, LPTSTR szHeaderInfo, DWORD &dwQueryBufLen)
{
	if(m_hConnect == NULL)
	{
		return false;
	}

	if(!m_hRequest)
	{
		if(!CreateRequestHandle(m_szMainUrl))
		{
			return false;
		}
	}
	if(m_hRequest)
	{
		m_bIsConnected = true;
		if(WinHttpQueryHeaders(m_hRequest, iQueryType, WINHTTP_HEADER_NAME_BY_INDEX, szHeaderInfo, &dwQueryBufLen, WINHTTP_NO_HEADER_INDEX))
			return true;		
	}
	return false;
}

/***************************************************************************
  Function Name  : GetHeaderInfo
  Description    : This function will create Request handle and will return WINHTTP_QUERY_FLAG_NUMBER
  Author Name    : Ramkrushna Shelke
  Date           : 11th Nov 2013
****************************************************************************/
bool CWinHttpManager::CreateRequestHandle(LPTSTR szURLPath)
{
	// Create an HTTP request handle.
	BOOL bResults = FALSE;
	if (m_hRequest)
	{
		return true;
	}

	if (szURLPath)
	{
		_tcscpy_s(m_szMainUrl, szURLPath);
	}

	if (m_hConnect)
	{
		m_hRequest = WinHttpOpenRequest(m_hConnect, L"GET", m_szMainUrl, NULL, WINHTTP_NO_REFERER, HTTP_TYPES, m_dwOpenRequestFlag);
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
				m_bIsConnected = true;
				return true;
			}
		}
	}
	m_bIsConnected = false;
	return false;
}

/***************************************************************************
  Function Name  : Download
  Description    : this funtion will download the file from server in provided bytes.
  Author Name    : Ramkrushna Shelke
  Date           : 11th Nov 2013
****************************************************************************/
bool CWinHttpManager::Download(TCHAR * szLocalFileName, DWORD dwByteRangeStart, DWORD dwByteRangeStop)
{
	bool bReturn = false;
	if(OpenRequestHandle() == false)
	{
		return bReturn;
	}

	m_bIsConnected = true;

	if(SetByteRange(dwByteRangeStart, dwByteRangeStop) == FALSE)
	{
		return bReturn;
	}

	//Check if the file handle already there then need to close
	if(m_hFile != NULL)
	{
		CloseHandle(m_hFile);
		m_hFile = NULL;
	}

	if(PathFileExists(szLocalFileName))
	{
		m_hFile = CreateFile(szLocalFileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		SetFilePointer(m_hFile, dwByteRangeStart, NULL, FILE_BEGIN);
	}
	else
	{
		m_hFile = CreateFile(szLocalFileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	if(m_hFile != NULL)
	{
		bReturn = true;
		if(!ReadWriteFile(m_hFile))
		{
			m_bIsConnected = false;
			bReturn = false;
		}
		if(m_hFile != NULL)
		{
			CloseHandle(m_hFile);
			m_hFile = NULL;
		}
	}
	return bReturn;
}

/***************************************************************************
  Function Name  : OpenRequestHandle
  Description    : this funtion will Open the Request handle and will return true or false.
  Author Name    : Ramkrushna Shelke
  Date           : 11th Nov 2013
****************************************************************************/
bool CWinHttpManager::OpenRequestHandle()
{
	if(m_hRequest)
	{
		WinHttpCloseHandle(m_hRequest);
		m_hRequest = NULL;
	}

		if(m_hConnect)
		{
			m_hRequest = WinHttpOpenRequest(m_hConnect, L"GET", m_szMainUrl, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, m_dwOpenRequestFlag);
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

	if(!m_hRequest)
	{
		return false;
	}
	return true;
}

/**********************************************************************************************
  Function Name  : StopCurrentDownload
  Description    : This function will set the flag m_bStopDownload to true to break the loop
				   and to stop current download.
  Author Name    : Ramkrushna Shelke
  Date           : 11th Nov 2013
***********************************************************************************************/
bool CWinHttpManager::StopCurrentDownload()
{
	m_bStopDownload = true;
	return true;
}

/**********************************************************************************************
  Function Name  : StartCurrentDownload
  Description    : This function will reset the flag m_bStopDownload to false and will reinitialize
				   the variables.
  Author Name    : Ramkrushna Shelke
  Date           : 11th Nov 2013
***********************************************************************************************/
bool CWinHttpManager::StartCurrentDownload()
{
	m_dwTotalDownloadBytes=0;
	m_bStopDownload = false;
	return true;
}

/**********************************************************************************************
  Function Name  : ReadWriteFile
  Description    : This function contains an while loop, which reads the http data and writes in 
				   local file, unless we will not set the flag m_bStopDownload = true
				   or else no data remained to donwload.
  Author Name    : Ramkrushna Shelke
  Date           : 11th Nov 2013
***********************************************************************************************/
bool CWinHttpManager::ReadWriteFile(HANDLE hFile)
{
	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer = NULL;
	int iRetryCount = 0;
	do
	{
		if(m_bStopDownload)
		{
			return false;
		}
		dwSize = 0;
		if(!WinHttpQueryDataAvailable(m_hRequest, &dwSize))
		{
			AddLogEntry(_T("Problem with the Host Connection! %s:%s"), m_szHostName, m_szMainUrl);
			m_bIsConnected = false;
			return false;
		}
		if(dwSize == 0)
		{
			return true;
		}
		pszOutBuffer = new char[dwSize+1];				// Allocate space for the buffer.
		ZeroMemory(pszOutBuffer, dwSize+1);				// Read the Data.

		if(!WinHttpReadData(m_hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
		{
			CString csLAstError;
			csLAstError.Format(_T("%d"),::GetLastError());
			AddLogEntry(_T("WinHttpReadData Unable to read from Host:%s:%s"), m_szHostName,m_szMainUrl);
			if(pszOutBuffer)
			{
				delete [] pszOutBuffer;
				pszOutBuffer = NULL;
			}
			return false;
		}
		DWORD dwBytesWritten = 0;
		if(FALSE == WriteFile(hFile, pszOutBuffer, dwDownloaded, &dwBytesWritten, NULL))
		{
			if(pszOutBuffer)
			{
				delete [] pszOutBuffer;
				pszOutBuffer = NULL;
			}
			return false;
		}
		FlushFileBuffers(hFile);
		if(pszOutBuffer)
		{
			delete [] pszOutBuffer;
			pszOutBuffer = NULL;
		}
		m_dwCompletedDownloadBytes += dwBytesWritten;

	} while ((dwSize>0) && (!m_bStopDownload));

	return true;
}

/**********************************************************************************************
  Function Name  : SetByteRange
  Description    : This function will set the byte range to download the file.
  Author Name    : Ramkrushna Shelke
  Date           : 11th Nov 2013
***********************************************************************************************/
BOOL CWinHttpManager::SetByteRange(DWORD dwByteRangeStart, DWORD dwByteRangeStop)
{
	if(m_bStopDownload)
	{
		return FALSE;
	}
	BOOL bResults = FALSE;
	TCHAR szByteRange[MAX_PATH]= {0};
	_stprintf_s(szByteRange, MAX_PATH, _T("Range: bytes=%ld-%ld"), dwByteRangeStart, dwByteRangeStop);
	if(m_hRequest)
	{
		bResults = WinHttpAddRequestHeaders(m_hRequest, szByteRange, -1, WINHTTP_ADDREQ_FLAG_ADD);
	}

	// Send a request.
	if(m_hRequest)
	{
		bResults = WinHttpSendRequest(m_hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
	}

	if(!bResults)
	{
		AddLogEntry(_T("***SetByteRange Handle failed for: %s:%s "),m_szHostName, m_szMainUrl);
		return bResults;
	}

	// End the request.
	bResults = WinHttpReceiveResponse(m_hRequest, NULL);
	return bResults;
}

/**********************************************************************************************
  Function Name  : CrackURL
  Description    : This function will check does the provided url is exists or not, if yes
				   it will get the host name from the url.
  Author Name    : Ramkrushna Shelke
  Date           : 11th Nov 2013
***********************************************************************************************/
bool CWinHttpManager::CrackURL(LPCTSTR szFullUrl)
{
	URL_COMPONENTS urlComp;
	size_t nURLLen = 0;

	wmemset(m_szMainUrl, 0, URL_SIZE);
	wmemset(m_szHostName, 0, MAX_PATH);
	wmemset(m_szFullUrl, 0, URL_SIZE);

	_tcscpy_s(m_szFullUrl, URL_SIZE, szFullUrl);

	// Initialize the URL_COMPONENTS structure.
	ZeroMemory(&urlComp, sizeof(urlComp));
	urlComp.dwStructSize = sizeof(urlComp);

	// Set required component lengths to non-zero so that they are cracked.
	urlComp.dwSchemeLength    = -1;
	urlComp.dwHostNameLength  = -1;
	urlComp.dwUrlPathLength   = -1;
	urlComp.dwExtraInfoLength = -1;

	// Crack the URL.
	if(WinHttpCrackUrl(szFullUrl, static_cast<DWORD>(_tcslen(szFullUrl)), 0, &urlComp))
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
	return false;
}

/**********************************************************************************************
  Function Name  : SetDownloadCompletedBytes
  Description    : This function will set private member variable using public function.
  Author Name    : Ramkrushna Shelke
  Date           : 11th Nov 2013
***********************************************************************************************/
void CWinHttpManager::SetDownloadCompletedBytes(DWORD dwTotalBytes)
{
	m_dwCompletedDownloadBytes = 0;
	m_dwTotalDownloadBytes += dwTotalBytes;
}

/**********************************************************************************************
  Function Name  : GetDownloadCompletedBytes
  Description    : This function will return private variable value
  Author Name    : Ramkrushna Shelke
  Date           : 11th Nov 2013
***********************************************************************************************/
DWORD CWinHttpManager::GetDownloadCompletedBytes()
{
	return m_dwTotalDownloadBytes;
}

/**********************************************************************************************
  Function Name  : CheckInternetConnection
  Description    : This function will Check does the internet connection exitst or not by using 
				   createrequest handle.
  Author Name    : Ramkrushna Shelke
  Date           : 11th Nov 2013
***********************************************************************************************/
bool CWinHttpManager::CheckInternetConnection()
{
	if(_tcslen(m_szFullUrl) == 0)
	{
		return false;
	}

	TCHAR szMainUrl[MAX_PATH] = {0};
	_tcscpy_s(szMainUrl, MAX_PATH, m_szFullUrl);

	if(Initialize(szMainUrl))	
	{
		if(CreateRequestHandle(szMainUrl))
		{
			m_bIsConnected = true;
			return true;
		}
	}
	m_bIsConnected = false;
	return false;
}

bool CWinHttpManager::CloseFileHandles()
{
	if(m_hFile != NULL)
	{
		CloseHandle(m_hFile);
		m_hFile = NULL;
	}
	return true;
}


/***************************************************************************
Function       : CheckProxySettings
In Parameters  : void
Out Parameters : bool
Description    : Function which checks for proxy settings
Author		   : Ram Shelke
****************************************************************************/
bool CWinHttpManager::CheckProxySettings(LPCTSTR szHost)
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
		{
			GetProxyServerNameFromRegistry(szProxyServer);
		}
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
bool CWinHttpManager::GetProxyServerNameFromRegistry(WCHAR *szProxyServer)
{
	try
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
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWinHttpManager::CheckProxySettings", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************
Function       : GetProxyServerDetailsFromDB
Description    : This function will get Proxy details here from sqlite DB
Author Name    : Kunal
Date           : 2ndth Nov 2018
****************************************************************************/
bool CWinHttpManager::GetProxyServerDetailsFromDB()
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
				AddLogEntry(L"### Failed to allocate memory in CWinHttpManager::GetProxyServerDetailsFromDB", 0, 0, true, SECONDLEVEL);
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
				AddLogEntry(L"### Failed to allocate memory in CWinHttpManager::GetProxyServerDetailsFromDB", 0, 0, true, SECONDLEVEL);
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
		AddLogEntry(L"### Exception in CWinHttpManager::GetProxyServerDetailsFromDB", 0, 0, true, SECONDLEVEL);
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
DWORD CWinHttpManager::GetProxySettingsFromRegistry()
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