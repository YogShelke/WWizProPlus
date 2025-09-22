/****************************** Module Header ******************************\
* Module Name:  VirusDetectorService.h
* Project:      VirusDetectorService
* Copyright (c) Microsoft Corporation.
* 
* Provides a sample service class that derives from the service base class - 
* CServiceBase. The sample service logs the service start and stop 
* information to the Application event log, and shows how to run the main 
* function of the service in a thread pool worker thread.
* 
* This source is subject to the Microsoft Public License.
* See http://www.microsoft.com/en-us/openness/resources/licenses.aspx#MPL.
* All other rights reserved.
* 
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma once

#include "ServiceBase.h"
#include "WardwizScanner.h"
#include <cpprest\astreambuf.h>
#include <cpprest\http_listener.h>
#include "CurlCLICppWrapper.h"

using namespace web::http::experimental::listener;
using namespace web::http;
using namespace web;

class CVirusDetectorService : public CServiceBase
{
public:

    CVirusDetectorService(PWSTR pszServiceName, 
        BOOL fCanStop = TRUE, 
        BOOL fCanShutdown = TRUE, 
        BOOL fCanPauseContinue = FALSE);
		bool InvokeCustomScan(CString csFileName);
		bool DownloadFileFromURL(CString csFilePath, CString csBearer);
		void InvokeListener();
		void RespondForCustomScan(const http_request& request, const status_code& status, const json::value& response);
    virtual ~CVirusDetectorService(void);
	
protected:

    virtual void OnStart(DWORD dwArgc, PWSTR *pszArgv);
    virtual void OnStop();
	void ServiceWorkerThread(void);
	//void handle_get(http_request request);
private:
    BOOL					m_fStopping;
    HANDLE					m_hStoppedEvent;

	int						m_iTotalFileCount;
	int						m_iThreatsFoundCount;
	CString					m_csVirusName;
	bool					m_bThreatDetected;
	DWORD					m_dwTimeRequired;
	std::string				m_sFileURL;
};