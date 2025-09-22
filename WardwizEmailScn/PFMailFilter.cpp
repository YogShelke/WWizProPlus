/**********************************************************************************************************
*		Program Name          : PFMailFilter.cpp
*		Description           : This class implements functionality of Email scan, Web Filter
*		Author Name			  : Ram Shelke
*		Date Of Creation      : 27 Jun 2019
*		Version No            : 4.1
*		Special Logic Used    : 
*		Modification Log      :
***********************************************************************************************************/
#include "stdafx.h"
#include "WardwizEmailScn.h"
#include "PFMailFilter.h"
#include "WinHttpClient.h"
#include <future>

enum
{
	APPBEHAVIOUR_AUTOMATIC,
	APPBEHAVIOUR_ALLOW,
	APPBEHAVIOUR_BLOCK
}APPBEHAVIOUR;

std::string g_blockString = "adition.com";
std::string g_mailPrefix;

#define DEFAULT_URL L"http://filter.Vibranium.co.in/urlcheck.php"
//#define DEFAULT_URL L"http://webfilter.wardwiz.in/urlcheckaug.php"

/***************************************************************************************************
*  Function Name  : MailFilter
*  Description    : Initializes the Mail Filter class
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
MailFilter::MailFilter() :
m_pobjParentalCtrl(NULL),
m_csServer(EMPTY_STRING),
m_dwProxySett(0x00),
m_HttpClient(L"HttpClient"),
m_csWebFilterURL(DEFAULT_URL),
m_buf2StDB(true, sizeof(URLDOMAINCACHE), sizeof(DWORD), sizeof(DWORD))
{
	GetEnvironmentVariable(L"ALLUSERSPROFILE", m_szAllUserPath, MAX_PATH);
	m_objScanner.LoadRequiredDLLs();
	m_csAppPath = GetWardWizPathFromRegistry();
	GetClientNames();
}

/***************************************************************************************************
*  Function Name  : MailFilter
*  Description    : Dest'r
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
MailFilter::~MailFilter()
{
	SaveURLDB();
}

/***************************************************************************************************
*  Function Name  : EventHandler
*  Description    : Cont'r
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
EventHandler::EventHandler() :
m_pobjParentalCtrl(NULL),
m_iSetDefAppBehav(APPBEHAVIOUR_AUTOMATIC)
{
	GetEnvironmentVariable(L"ALLUSERSPROFILE", m_szAllUserPath, MAX_PATH);
	m_csAppPath = GetWardWizPathFromRegistry();
}

/***************************************************************************************************
*  Function Name  : SetParentalControlPtr
*  Description    : Function to set Parental control object reference
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::SetParentalControlPtr(CWWizParentalCntrl	*pParentalCtrl)
{
	m_pobjParentalCtrl = pParentalCtrl;
}

/***************************************************************************************************
*  Function Name  : MailFilter
*  Description    : Dest'r
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
EventHandler::~EventHandler()
{
}

/***************************************************************************************************
*  Function Name  : tcpConnected
*  Description    : overided Virtual function which gets called when any TCP connections going made  by
*					an application
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void MailFilter::tcpConnected(nfapi::ENDPOINT_ID id, nfapi::PNF_TCP_CONN_INFO pConnInfo)
{
	//OutputDebugString(L">>> MailFilter::tcpConnected");
	__try
	{
		tcpConnectedSEH(id, pConnInfo);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in MailFilter::tcpConnected", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : tcpConnectedSEH
*  Description    : Structured exception handling function of tcpConnected
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void MailFilter::tcpConnectedSEH(nfapi::ENDPOINT_ID id, nfapi::PNF_TCP_CONN_INFO pConnInfo)
{
	try
	{
		//check here if application is blocked for internet usage.
		WCHAR szProcessName[MAX_PATH] = { 0 };
		nf_getProcessName(pConnInfo->processId, szProcessName, MAX_PATH);
		//make lower
		wcslwr(szProcessName);
		//OutputDebugString(szProcessName);

		CString csProcessName(szProcessName);
		int iFound = csProcessName.ReverseFind(L'\\');
		csProcessName = csProcessName.Right(csProcessName.GetLength() - iFound - 1);

		//check if process is in exclusionlist
		if (ISFilterExcludedProcess(csProcessName.GetBuffer()))
		{
			return;
		}

		CString csUserName = m_objExecuteProcess.GetUserNamefromProcessID(pConnInfo->processId);
		if (csUserName == L"")
		{
			AddLogEntryEx(ZEROLEVEL, L"### User name not returned. EventHandler::HandleTcpUdpRequests");
			return;
		}

		if (!InsertUserNameandID(id, CStringA(csUserName).MakeLower().GetBuffer()))
		{
			AddLogEntryEx(ZEROLEVEL, L"### Failed to Insert User name and Endpoint ID");
		}

		//check for email scan
		if (theApp.m_bIsEmailScanEnabled)
		{
			if (ISApplicationListed2Filter(csProcessName))
			{
				if (pConnInfo->direction == NF_D_OUT)
				{
					if (!CheckISFilterAdded(id, FT_SSL, FF_SSL_TLS_AUTO))
					{
						pf_addFilter(id, FT_SSL, FF_SSL_TLS_AUTO);
						pf_addFilter(id, FT_POP3, FF_SSL_TLS_AUTO);
						pf_addFilter(id, FT_SMTP, FF_SSL_TLS_AUTO);
					}
				}
			}
		}

		//check for web protection
		if (theApp.m_bIsWebFilterEnabled || theApp.m_bIsBrowserSecEnabled)
		{
			if (!ISFilterExcludedProcess(csProcessName.GetBuffer()))
			{
			if (pConnInfo->direction == NF_D_OUT)
			{
				if (!CheckISFilterAdded(id, FT_PROXY, FF_HTTP_BLOCK_SPDY))
				{
						pf_addFilter(id, FT_PROXY, FF_SSL_TLS_AUTO);
						pf_addFilter(id, FT_SSL, FF_SSL_TLS_AUTO);
						pf_addFilter(id, FT_HTTP, FF_HTTP_BLOCK_SPDY | FF_SSL_TLS_AUTO);
					
				}
				
			}
			else
			{
				if (!CheckISFilterAdded(id, FT_HTTP, FF_HTTP_BLOCK_SPDY))
					{
						pf_addFilter(id, FT_HTTP, FF_HTTP_BLOCK_SPDY | FF_SSL_TLS_AUTO);
					}
				}
			}
		}

		//dont filter for wardwiz applications
		CString csProcessPath{ szProcessName };
		CString csHiddenTxt = L"\\";
		int iPos = csProcessPath.ReverseFind('\\');
		csProcessPath = csProcessPath.Left(iPos + 1);
		CString csWardWizPath = m_csAppPath.MakeLower();
		if (csProcessPath == csWardWizPath)
		{
			return;
		}

		//check for application block
		if (m_objFirewallFilter.ISApplicationBlocked(szProcessName))
		{
			nf_tcpClose(id);
			return;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::tcpConnectedSEH", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : CreateDirectoryFocefully
*  Description    : Function which creates folder if does not exists
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
bool MailFilter::CreateDirectoryFocefully(LPTSTR lpszPath)
{
	try
	{
		CreateDirectory(lpszPath, NULL);
		if (PathFileExists(lpszPath))
			return true;

		_wmkdir(lpszPath);
		if (PathFileExists(lpszPath))
			return true;
	}
	catch (...)
	{
		AddLogEntry(L"### MailFilter::CreateDirectoryFocefully::Exception", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : filterOutgoingMail
*  Description    : Function which filters outgoing email.
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
bool MailFilter::filterOutgoingMail(nfapi::ENDPOINT_ID id, PFObject * object)
{
	bool res = false;
	try
	{
		if (theApp.m_bIsEmailScanEnabled == false) //if email scan is OFF
			return false;

		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBEMAILSCAN.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return false;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);
		dbSQlite.Open();

		CWardwizSQLiteTable qResult = dbSQlite.GetTable("Select SCAN_INCOMING_AND_OUTGOING_MAILS from WWIZEMAILSCANDETAILS;");
		char sziStngID[2] = { 0 };
		strcpy_s(sziStngID, qResult.GetFieldValue(0));
		if (strcmp(sziStngID, "0") == 0)
			return false; //Scan incoming and outgoing mails if NOT set ON on UI

		PFHeader h;
		int iTotFileScanCount = 0;
		int iTotalThreatsFound = 0;
		int iTotalThreatsCleaned = 0;
		PFStream * pStream = object->getStream(0);
		tStreamSize len;

		len = (tStreamSize)pStream->size();

		char * pContent = (char*)malloc(len + 1);
		if (!pContent)
			return 0;

		pStream->seek(0, FILE_BEGIN);
		pStream->read(pContent, len);
		pContent[len] = '\0';

		char * pDiv = strstr(pContent, "\r\n\r\n");
		if (!pDiv)
		{
			free(pContent);
			return 0;
		}

		if (!h.parseString(pContent, len))
		{
			free(pContent);
			return 0;
		}
		if (!pf_readHeader(object->getStream(0), &h))
			return false;

		PFHeaderField * pField = h.findFirstField("From");
		std::string strFrom = pField->value();

		pField = h.findFirstField("To");
		std::string strTo = pField->value();

		pField = h.findFirstField("Subject");
		std::string strSubject = pField->value();

		//PFHeaderField * pField = h.findFirstField("Subject");

		if (!pField)
			return false;
		CMimeMessage m_objMimeMessage;
		CMimeHeader m_objMimeHeader;
		int iRet = 0;

		m_objMimeMessage.Load(pContent, len);

		typedef list <CMimeBody*>CBodyList;
		CBodyList rList;

		if (!ScanAttachment())
			return false;

		m_objMimeMessage.GetAttachmentList(rList);
		list<CMimeBody*>::const_iterator it;

		for (it = rList.begin(); it != rList.end(); it++)
		{
			CMimeBody* pBP = *it;
			int nMediaType = m_objMimeHeader.GetMediaType();

			if (m_objMimeHeader.MEDIA_MULTIPART != nMediaType)
			{
				string strName = pBP->GetName();
				if (strName.size() > 0)
				{
					TCHAR szTargetFolder[MAX_PATH] = { 0 };
					iTotFileScanCount++;
					_stprintf_s(szTargetFolder, MAX_PATH, L"%s\\Vibranium\\EmailScan\\", m_szAllUserPath);
					if (!PathFileExists(szTargetFolder))
					{
						CreateDirectoryFocefully(szTargetFolder);
					}
					TCHAR szTargetPath[MAX_PATH] = { 0 };
					_stprintf_s(szTargetPath, MAX_PATH, L"%s\\Vibranium\\EmailScan\\%s.tmp", m_szAllUserPath, CString(strName.c_str()));

					pBP->WriteToFile(CW2A(szTargetPath));
					iRet = m_objScanner.ExtractZipForScanning(szTargetPath, (CString)strFrom.c_str(), CString(strName.c_str()));

					if (iRet == 2 || iRet == 3) //qurentine or repair
					{
						iTotalThreatsFound++;
						iTotalThreatsCleaned++;

						DWORD dwQuarantineOption = 0, dwHeuScan = 0;
						GetWardwizRegistryDetails(dwQuarantineOption, dwHeuScan);

						CString csInsertQuery = _T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,");
						csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,%d,Date('now'),Datetime('now','localtime'),Date('now'),Datetime('now','localtime'), %d, %d, %d, %d, %d );"), EMAILSCAN, iTotFileScanCount, iTotalThreatsFound, dwQuarantineOption, dwHeuScan, iTotalThreatsCleaned);
						CT2A ascii(csInsertQuery, CP_UTF8);
						INT64 iScanSessionId = m_objScanner.InsertDataToTable(ascii.m_psz);
						CString csVirusName = L"";
						int iVirusNameLen = m_objScanner.m_ViruName.GetLength();
						if (iVirusNameLen > 0)
							csVirusName = m_objScanner.m_ViruName;
						else
							csVirusName = L"";

						csInsertQuery = L"";
						csInsertQuery = _T("INSERT INTO Wardwiz_ScanDetails VALUES (null,");
						csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanDetails VALUES (null,%d,Datetime('now','localtime'),NULL,NULL,NULL,'%s','%s','Quarantined','%s' );"), iScanSessionId, csVirusName, CString(strName.c_str()), CString(strSubject.c_str()));
						//csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanDetails VALUES (null,%I64d,'%s','%s','%s','%s','%s','%s','%s' );"), iSessionID, csStartDate, csStartDateTime, csEndDate, csEndDateTime, CString(stScanDetails.szThreatName), CString(stScanDetails.szFilePath), csAction);
						CT2A ascii1(csInsertQuery, CP_UTF8);
						m_objScanner.InsertDataToTable(ascii1.m_psz);

						if (pBP->IsAttachment())
						{
							m_objMimeMessage.ErasePart(pBP);
							pBP = m_objMimeMessage.CreatePart();
							pBP->SetContentType("text/plain");
							pBP->SetText("Attachment removed by WardWiz\r\n");
							pBP->SetName(strName.c_str());
						}
					}

					if (remove(CW2A(szTargetPath)) != 0)
						AddLogEntry(L"### Unable to removed file :: filterOutgoingMail", 0, 0, true, SECONDLEVEL);
					else
						AddLogEntry(L"### File removed successfully :: filterOutgoingMail", 0, 0, true, SECONDLEVEL);
				}
			}
		}

		int newSize = m_objMimeMessage.GetLength();
		char *msgBuf = (char*)malloc(newSize);
		if (msgBuf)
		{
			int realLength = m_objMimeMessage.Store(msgBuf, newSize);
			pStream->reset();
			pStream->write(msgBuf, newSize);
			free(msgBuf);
		}

		TCHAR szSubjectValueData[512] = { 0 };
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		GetPrivateProfileString(L"VBSETTINGS", L"EmailSubject", L"", szSubjectValueData, 511, csIniFilePath);
		std::string s = "";//g_mailPrefix;
		if (szSubjectValueData[0] && iRet == 2 || iRet == 3)
		{
			s = CStringA(szSubjectValueData).GetBuffer() + g_mailPrefix + " " + pField->unfoldedValue();
		}
		else
		{
			s = g_mailPrefix + " " + pField->unfoldedValue();
		}
		h.removeFields("Subject", true);
		h.addField("Subject", s);

		free(pContent);

		//std::string s = strToLower(pField->unfoldedValue());

		if (s.find(g_mailPrefix) != std::string::npos)
		{
			PFObject * newObj = PFObject_create(OT_RAW_INCOMING, 1);

			if (newObj)
			{
				PFStream * pStream = newObj->getStream(0);
				if (pStream && (iRet == 2 || iRet == 3))
				{
					const char blockCmd[] = "Mail is blocked by Wardwiz due to malicious attachment!\r\n";
					pStream->write(blockCmd, sizeof(blockCmd)-1);
					pf_postObject(id, newObj);
					res = true;
				}
				newObj->free();
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::filterOutgoingMail", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return res;
}

/***************************************************************************************************
*  Function Name  : filterIncomingMail
*  Description    : Function which Extract data from PF object and like sender email address, subject, ..
*  Author Name    : Ram Shelke
*  Date           : 16-Aug-2016
****************************************************************************************************/
void MailFilter::filterIncomingMail(nfapi::ENDPOINT_ID id, PFObject * object)
{
	try
	{
		if (theApp.m_bIsEmailScanEnabled == false)//if email scan is OFF
			return;

		PFHeader h;
		tStreamSize len;
		PFStream * pStream = object->getStream(0);
		int iTotFileScanCount = 0;
		int iTotalThreatsFound = 0;
		int iTotalThreatsCleaned = 0;
		if (!pStream)
			return;

		len = (tStreamSize)pStream->size();

		char * pContent = (char*)malloc(len + 1);
		if (!pContent)
			return;

		pStream->seek(0, FILE_BEGIN);
		pStream->read(pContent, len);
		pContent[len] = '\0';

		char * pDiv = strstr(pContent, "\r\n\r\n");
		if (!pDiv)
		{
			free(pContent);
			return;
		}

		if (!h.parseString(pContent, len))
		{
			free(pContent);
			return;
		}

		PFHeaderField * pField = h.findFirstField("From");
		std::string strFrom = pField->value();

		pField = h.findFirstField("To");
		std::string strTo = pField->value();

		pField = h.findFirstField("Subject");
		std::string strSubject = pField->value();

		if (!pField)
		{
			free(pContent);
			return;
		}

		CMimeMessage m_objMimeMessage;
		CMimeHeader m_objMimeHeader;
		int iRet = 0;

		m_objMimeMessage.Load(pContent, len);

		typedef list <CMimeBody*>CBodyList;
		CBodyList rList;

		if (!ScanAttachment())
			return;

		m_objMimeMessage.GetAttachmentList(rList);
		list<CMimeBody*>::const_iterator it;

		for (it = rList.begin(); it != rList.end(); it++)
		{
			CMimeBody* pBP = *it;
			int nMediaType = m_objMimeHeader.GetMediaType();

			if (m_objMimeHeader.MEDIA_MULTIPART != nMediaType)
			{
				string strName = pBP->GetName();
				if (strName.size() > 0)
				{
					TCHAR szTargetFolder[MAX_PATH] = { 0 };
					iTotFileScanCount++;
					_stprintf_s(szTargetFolder, MAX_PATH, L"%s\\Vibranium\\EmailScan\\", m_szAllUserPath);
					if (!PathFileExists(szTargetFolder))
					{
						CreateDirectoryFocefully(szTargetFolder);
					}
					TCHAR szTargetPath[MAX_PATH] = { 0 };
					_stprintf_s(szTargetPath, MAX_PATH, L"%s\\Vibranium\\EmailScan\\%s.tmp", m_szAllUserPath, CString(strName.c_str()));

					pBP->WriteToFile(CW2A(szTargetPath));
					iRet = m_objScanner.ExtractZipForScanning(szTargetPath, (CString)strFrom.c_str(), CString(strName.c_str()));

					if (iRet == 2 || iRet == 3) //qurentine or repair
					{
						iTotalThreatsFound++;
						iTotalThreatsCleaned++;
						/*CString csReportsFolderPath = GetWardWizPathFromRegistry();
						csReportsFolderPath += L"QUARANTINE\\emailbkup\\";
						if (!PathFileExists(csReportsFolderPath))
						{
						CreateDirectoryFocefully((LPTSTR)(LPCTSTR)csReportsFolderPath);
						}
						TCHAR szQuarantineFullPath[MAX_PATH] = { 0 };
						_stprintf_s(szQuarantineFullPath, MAX_PATH, L"%s%s", csReportsFolderPath, CString(strName.c_str()));

						pBP->WriteToFile(CW2A(szQuarantineFullPath));*/

						DWORD dwQuarantineOption = 0, dwHeuScan = 0;
						GetWardwizRegistryDetails(dwQuarantineOption, dwHeuScan);

						CString csInsertQuery = _T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,");
						csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,%d,Datetime('now','localtime'),Datetime('now','localtime'), NULL,NULL, %d, %d, %d, %d, %d );"), EMAILSCAN, iTotFileScanCount, iTotalThreatsFound, dwQuarantineOption, dwHeuScan, iTotalThreatsCleaned);
						CT2A ascii(csInsertQuery, CP_UTF8);
						INT64 iScanSessionId = m_objScanner.InsertDataToTable(ascii.m_psz);
						CString csVirusName = L"";
						int iVirusNameLen = m_objScanner.m_ViruName.GetLength();
						if (iVirusNameLen > 0)
							csVirusName = m_objScanner.m_ViruName;
						else
							csVirusName = L"";

						csInsertQuery = L"";
						csInsertQuery = _T("INSERT INTO Wardwiz_ScanDetails VALUES (null,");
						csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanDetails VALUES (null,%d,Datetime('now','localtime'),NULL,NULL,NULL,'%s','%s','Quarantined','%s' );"), iScanSessionId, csVirusName, CString(strName.c_str()), CString(strSubject.c_str()));
						//csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanDetails VALUES (null,%I64d,'%s','%s','%s','%s','%s','%s','%s' );"), iSessionID, csStartDate, csStartDateTime, csEndDate, csEndDateTime, CString(stScanDetails.szThreatName), CString(stScanDetails.szFilePath), csAction);
						CT2A ascii1(csInsertQuery, CP_UTF8);
						m_objScanner.InsertDataToTable(ascii1.m_psz);

						if (pBP->IsAttachment())
						{
							m_objMimeMessage.ErasePart(pBP);
							m_objMimeMessage.CreatePart();
							pBP->SetContentType("text/plain");
							pBP->SetText("Attachment removed by WardWiz\r\n");
							pBP->SetName(strName.c_str());
						}
					}

					if (remove(CW2A(szTargetPath)) != 0)
						AddLogEntry(L"### Unable to remove file :: filterOutgoingMail", 0, 0, true, SECONDLEVEL);
					else
						AddLogEntry(L"### File removed successfully :: filterOutgoingMail", 0, 0, true, SECONDLEVEL);
				}
			}
		}

		TCHAR szSubjectValueData[512] = { 0 };
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"VBSETTINGS" + L"\\ProductSettings.ini";
		GetPrivateProfileString(L"VBSETTINGS", L"EmailSubject", L"", szSubjectValueData, 511, csIniFilePath);
		std::string s = "";//g_mailPrefix;
		if (szSubjectValueData[0] && iRet == 2 || iRet == 3)
		{
			s = CStringA(szSubjectValueData).GetBuffer() + g_mailPrefix + " " + pField->unfoldedValue();
		}
		else
		{
			s = g_mailPrefix + " " + pField->unfoldedValue();
		}
		h.removeFields("Subject", true);
		h.addField("Subject", s);

		//reset stream here
		pStream->reset();

		//write subject header
		pf_writeHeader(pStream, &h);

		//write modified mail content
		int newSize = m_objMimeMessage.GetLength();
		char *msgBuf = (char*)malloc(newSize);
		if (msgBuf)
		{
			int realLength = m_objMimeMessage.Store(msgBuf, newSize);
			pStream->write(msgBuf, newSize);
			free(msgBuf);
		}

		//pDiv += 4;
		//pStream->write(pDiv, (ProtocolFilters::tStreamSize)(len - (pDiv - pContent)));

		//free memory
		free(pContent);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::filterIncomingMail", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : dataAvailable
*  Description    : virtual function which gets called when any data available to scan in user mode.
*  Author Name    : Ram Shelke
*  Date           : 16-Aug-2016
****************************************************************************************************/
void MailFilter::dataAvailable(nfapi::ENDPOINT_ID id, PFObject * object)
{
	__try
	{
		bool blocked = false;
		switch (object->getType())
		{
		case OT_SMTP_MAIL_OUTGOING:
			blocked = filterOutgoingMail(id, object);
			break;
		case OT_POP3_MAIL_INCOMING:
			filterIncomingMail(id, object);
			break;
		case OT_HTTP_REQUEST:
			blocked = filterHttpRequest(id, object);
			break;
		case OT_HTTP_RESPONSE:
			//Respose is made temporary OFF which later will used
			//for Web Content filtering
			//blocked = filterHttpResponse(id, object);
			break;
		}
		if (!blocked)
			pf_postObject(id, object);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in MailFilter::dataAvailable", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : getHttpUrl
*  Description    : function which extracts http url from Pf file object.
*  Author Name    : Ram Shelke
*  Date           : 16-Aug-2016
****************************************************************************************************/
std::string MailFilter::getHttpUrl(PFObject * object)
{
	std::string url = "";
	try
	{
		PFHeader h;
		PFStream * pStream;
		std::string status;
		char * p;

		if (object->getType() != OT_HTTP_REQUEST)
			return "";

		if (pf_readHeader(object->getStream(HS_HEADER), &h))
		{
			PFHeaderField * pField = h.findFirstField("Host");
			if (pField)
			{
				url = "http://" + pField->value();
			}
		}

		pStream = object->getStream(HS_STATUS);
		if (!pStream)
			return "";

		status.resize((unsigned int)pStream->size());
		pStream->read((char*)status.c_str(), (tStreamSize)status.length());

		if (p = strchr((char*)status.c_str(), ' '))
		{
			p++;
			char * pEnd = strchr(p, ' ');
			if (pEnd)
			{
				if (strncmp(p, "http://", 7) == 0)
				{
					url = "";
				}
				url.append(p, pEnd - p + 1);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::getHttpUrl", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return url;
}

/***************************************************************************************************
*  Function Name  : GetShortBlockUrl
*  Description    : Function to short url of blocked web site
*  Author Name    : Ram Shelke
*  Date           : 16-Aug-2016
****************************************************************************************************/
CStringA MailFilter::GetShortBlockUrl(CStringA csBlkUrl)
{
	try
	{
		CStringA csDestUrl;
		csDestUrl.Format("%s%s%s", csBlkUrl.Left(28), "...", csBlkUrl.Right(28));
		return csDestUrl;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::GetShortBlockUrl", 0, 0, true, SECONDLEVEL);
	}
	return "";
}

/***************************************************************************************************
*  Function Name  : postBlockHttpResponse
*  Description    : function which posts http response.
*  Author Name    : Ram Shelke
*  Date           : 16-Aug-2016
****************************************************************************************************/
void MailFilter::postBlockHttpResponse(nfapi::ENDPOINT_ID id, std::string strLink, std::string strCategory)	
{
	try
	{
		PFObject * newObj = PFObject_create(OT_HTTP_RESPONSE, 3);
		if (!newObj)
			return;

		std::string strShortLink = strLink;
		if (strShortLink.length() > 59)
			strShortLink = GetShortBlockUrl(strLink.c_str());

		const char status[] = "HTTP/1.1 404 Not OK\r\n";
		char blockHtmlVal1[] =
			"<html>"\
			"<head>"\
			"<title>Alert</title>"\
			"<meta charset = \"ISO-8859-1\">" \
			"<META HTTP - EQUIV = \"content-type\" CONTENT = \"text/html; charset=utf-8\">" \
			"<meta name = \"viewport\" content = \"width=device-width, initial-scale=1.0\">"\
			"<link href = \"https://fonts.googleapis.com/css?family=Roboto&display=swap\" rel = \"stylesheet\">"\
			"<style>"\
			"body{"\
			"text-align: center;"\
			"}"\
			".alertCont{"\
			"width: 500px;"\
			"min-height: 200px;"\
			"position: relative;"\
			"margin:0 auto;"\
			"margin-top:11% ;"\
			"font-family: 'Roboto', sans-serif;"\
			"}"\
			".header{"\
			"text-align:center;"\
			"padding-bottom:15px;"\
			"border-bottom:1px solid #dcdcdc;"\
			"}"\
			".logo{"\
			"width:145px;"\
			"margin-bottom:5px; "\
			"}"\
			".MainCont{"\
			"padding-top:15px;"\
			"min-height: 130px;"\
			"}"\
			"p{"\
			"font-size: 15px;"\
			"}"\
			".warningIcon{"\
			"width:25px;"\
			"height: 25px;"\
			"}"\
			".titleCont{"\
			"background:#f9f9f9;"\
			"padding:10px;"\
			"border:1px solid #dcdcdc;"\
			"border-radius:4px;"\
			"}"\
			".titleCont p{"\
			"display: inline-block;"\
			"margin-top: 5px;"\
			"float: left;"\
			"padding-left: 10px;"\
			"font-size: 16px;"\
			"color: #a91818;"\
			"}"\
			".redColor{"\
			"color: #a91818;"\
			"}"\
			".urlLink{"\
			"background: #fff;"\
			"margin-top: 37px;"\
			"border: 1px solid #dcdcdc;"\
			"padding: 6px 10px;"\
			"font-size: 15px;"\
			"font-weight: normal;"\
			"margin-bottom: 2px;"\
			"border-radius:4px;"\
			"text-align:left; "\
			"}"\
			".leftCl{"\
			"text-align:left; "\
			"}"\
			".leftCl, .rightCl{"\
			"width:248px;"\
			"float:left;"\
			"border-bottom: 1px solid #dcdcdc;"\
			"padding: 6px 0px;"\
			"}"\
			".rightCl{"\
			"border-left:1px solid #dcdcdc;"\
			"text-align:right;"\
			"}"\
			".text-cntr{"\
			"text-align:center;"\
			"}"\
			".ClickBtn{"\
			"cursor:pointer;"\
			"color:blue;"\
			"}"\
			".popupWindow, .loader{"\
			"position:absolute;"\
			"width:100%;"\
			"height:100%;"\
			"left:0;"\
			"top:0;"\
			"text-align:center;"\
			"background:rgba(255,255,255,.1);"\
			"font-family: 'Roboto', sans-serif;"\
			"z-index:9999;"\
			"}"\
			".popupContent{"\
			"width: 300px;"\
			"height: 150px;"\
			"background: #fff;"\
			"border: 1px solid #d7d9da;"\
			"margin: 0 auto;"\
			"margin-top: 11% ;"\
			"position: relative;"\
			"}"\
			".consPading{"\
			"padding:0 10px;"\
			"}"\
			".popupContent h4{"\
			"margin: 0;"\
			"background: #2b5a7e;"\
			"color: #fff;"\
			"padding: 8px 10px;"\
			"font-size: 14px;"\
			"font-weight: normal;"\
			"}"\
			".bodyBlur{"\
			"filter:blur(8px);"\
			"-webkit-filter: blur(8px);"\
			"}"\
			"button{"\
			"min-width: 110px;"\
			"background:#2b5a7e;"\
			"color:#fff;"\
			"height:28px;"\
			"border:1px solid #183f5d;"\
			"margin-top: 10px;"\
			"font-size: 12px;"\
			"cursor:pointer;"\
			"margin-left: 5px; "\
			"}"\
			".loaderContent{"\
			"font-family: 'Roboto', sans-serif;"\
			"text-align:center;"\
			"position:absolute;"\
			"top:calc(50% - 83px);"\
			"left:calc(50% - 165px);"\
			"}"\
			".loaderSpin{"\
			"border: 4px solid #f3f3f3;"\
			"border-radius: 50% ;"\
			"border-top: 4px solid #3498db;"\
			"width: 30px;"\
			"height: 30px;"\
			"-webkit-animation: spin 2s linear infinite;"\
			"animation: spin 2s linear infinite;"\
			"display: inline-block;"\
			"}"\
			"@keyframes spin{"\
			"0% { transform: rotate(0deg); }"\
			"100% { transform: rotate(360deg); }"\
			"}"\
			"</style>"\
			"</head>"\
			"<body>"\

			"<div class = 'loader'>"\
			"<div class = 'loaderContent'>"\
			"<div class = 'loaderSpin'></div>"\
			"<p>Please wait while we are processing your request ...</p>"\
			"</div>"\
			"</div>"\

			"<div class = \'alertCont\'>"\
			"<div class = \'header\'>"\
			"<img class = \'logo\' src = \'https://files.msg91.com/389565/fiaykjrw'>"\
			//"<img class = \'logo\' src = \'http://www.wardwiz.in/catalog/view/theme/welserv/wardwiz/images/wardwiz-logo.png\'>"\//
			"<h4 style = \"margin:auto;padding:0;font-size:21px;font-weight:normal;\">";
			
		CStringA csParCtrlString = L"Parental Control";
		if (strCategory == "122")
		{
			strCategory = " Phishing";
			csParCtrlString = L"Browser Security";
		}
		else if (strCategory == "125")
		{
			strCategory = " Malware";
			csParCtrlString = L"Browser Security";
		}
		char blockHtmlVal2[] = "</h4>"\
				"</div>"\
				"<div class = \"MainCont\">"\
				"<div class = \"titleCont\">"\
				"<p class='dynamicContent2'>";

		CStringA csParCtrlStr = L"Access to this website is blocked by Vibranium";
		char blockHtmlVal3[] = "</p>"\
			"<div class='nextHide'>"\
			"<h4 class = \"urlLink\">";

		char blockHtml1[5120] = { 0 };
		strcpy(blockHtml1, blockHtmlVal1);
		strcat(blockHtml1, csParCtrlString);
		strcat(blockHtml1, blockHtmlVal2);
		strcat(blockHtml1, csParCtrlStr);
		strcat(blockHtml1, blockHtmlVal3);
	
		char blockHtml2[] = "</h4>"\
			"</div>"\
			"</div>"\
			"<div class='nextHide'>"\
			"<p class = \"leftCl category\">";

		//Category: Malicious link

		//"dataType : \"JSON\","\

		const char blockHtmlVal4[] = "</p>"\
			"<p class = \"rightCl\">";
		
		CStringA csPCtrlStr = L"Action: ";
		const char blockHtmlVal5[] = "<span class = 'redColor'>";
		
		CStringA csPCtrlStrVal = L"BLOCKED";
		const char blockHtmlVal6[] = "</span></p>"\
			"</div>"\
			"</div>"\
			"<div class='nextHide'>"\
			"<p class = \"text-cntr\">";
		
		CStringA csPCtrlStrVal1 = L"The website you want to access is listed in the RESTRICTED category."; 
		const char blockHtmlVal7[] = "</p>"\
			"<p class = \"text-cntr dynamicContent\">";
		
		CStringA csPCtrlStrVal2 = L"If you believe this is a mistake, please"; 
		const char blockHtmlVal8[] = "<span class = \"ClickBtn\" onclick = \"ClickFunction()\" >";

		CStringA csPCtrlStrVal3 = L" click here."; 
		const char blockHtmlVal9[] = "</span></p>"\
			"</div>"\
			"</div>"\
			"<div class = \"popupWindow\">"\
			"<div class = \"popupContent\">"\
			"<h4>";
		
		CStringA csPCtrlStrVal4 = L"Submit URL";
		const char blockHtmlVal10[] = "</h4>"\
			"<div class = \"consPading\">"\
			"<p>";
		
		CStringA csPCtrlStrVal5 = L"Do you want to submit this URL to the Vibranium Security lab for a review?";
		const char blockHtmlVal11[] = "</p>"\
			"<div style = 'text-align:right;'>"\
			"<button onclick = ClickSubmitFunction()>";

		CStringA csPCtrlStrVal6 = L"OK";
		const char blockHtmlVal12[] = "</button>"\
			"<button onclick = ClickCancelFunction()>";
		
		CStringA csPCtrlStrVal7 = L"CANCEL"; 
		const char blockHtmlVal13[] = "</button>"\
			"</div>"\
			"</div>"\
			"</div>"\
			"</div>"\
			"<script src=\'https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js\'></script>"\
			"<script  type = 'text/javascript'>"\
			"$(document).ready(function(){";
		
		char blockHtml3[1540] = { 0 };
		strcpy(blockHtml3, blockHtmlVal4);
		strcat(blockHtml3, csPCtrlStr);
		strcat(blockHtml3, blockHtmlVal5);
		strcat(blockHtml3, csPCtrlStrVal);
		strcat(blockHtml3, blockHtmlVal6);
		strcat(blockHtml3, csPCtrlStrVal1);
		strcat(blockHtml3, blockHtmlVal7);
		strcat(blockHtml3, csPCtrlStrVal2);
		strcat(blockHtml3, blockHtmlVal8);
		strcat(blockHtml3, csPCtrlStrVal3);
		strcat(blockHtml3, blockHtmlVal9);
		strcat(blockHtml3, csPCtrlStrVal4);
		strcat(blockHtml3, blockHtmlVal10);
		strcat(blockHtml3, csPCtrlStrVal5);
		strcat(blockHtml3, blockHtmlVal11);
		strcat(blockHtml3, csPCtrlStrVal6);
		strcat(blockHtml3, blockHtmlVal12);
		strcat(blockHtml3, csPCtrlStrVal7);
		strcat(blockHtml3, blockHtmlVal13);

		char blockHtml4[50] = { 0 };
		if (strCategory == "1")
		{
			sprintf(blockHtml4, "var controlType = %d;", 1);
		}
		else
		{
			sprintf(blockHtml4, "var controlType = %d;", 0);
		}
		const char blockHtml5Val1[] =

			"$('.loader').hide();"\
			"if (controlType == 1){"\
			"$('.nextHide').hide();"\
			"$('.MainCont').css(\"border-bottom\", \"none\"); "\
			"$('.titleCont').css(\"min-height\", \"50px\");"\
			"$('.dynamicContent2').text('";
		
		CStringA csblockHtml5Val1 = L"Internet connection has temporarily blocked. Please contact your parents to allow."; 
		const char blockHtml5Val2[] = "')"\
			"}"\
			"$('.popupWindow').hide();"\
			"})"\
			"</script>"\
			"<script  type = 'text/javascript'>"\
			"function ClickFunction() {"\
			"$('.alertCont').addClass('bodyBlur');"\
			"$('.popupWindow').show();"\
			"}"\
			"</script>"\
			"<script  type = 'text/javascript'>"\
			"function ClickCancelFunction() {"\
			"$('.alertCont').removeClass('bodyBlur');"\
			"$('.popupWindow').hide();"\
			"}"\
			"</script>"\
			"<script  type = 'text/javascript'>"\
			"function ClickSubmitFunction() {"\
			"$('.alertCont').removeClass('bodyBlur');"\
			"$('.popupWindow').hide();"\
			"$('.loader').show();"\
			"$('.alertCont').addClass('bodyBlur');"\
			"var url_value = $('.urlLink').text();"\
			"var cat_value = $('.category').text();"\
			"window.open('http://filter.vibranium.co.in/feedback_url.php?url=' + url_value + '&cat=' + cat_value, 'popUpWindow','height=200,width=300,left=100,top=100,resizable=yes,scrollbars=yes,toolbar=yes,menubar=no,location=no,directories=no, status=yes');"\
			/*"window.open('http://webfilter.wardwiz.in/feedback_url.php?url=' + url_value + '&cat=' + cat_value, 'popUpWindow','height=200,width=300,left=100,top=100,resizable=yes,scrollbars=yes,toolbar=yes,menubar=no,location=no,directories=no, status=yes');"\*/
			"setTimeout(function(){"\
			"$('.alertCont').removeClass('bodyBlur');"\
			"}, 6200);"\
			"setTimeout(function(){"\
			"$('.loader').hide(); "\
			"}, 6200); "\
			"$('.dynamicContent').text('";
		
		CStringA csblockHtml5Val2 = L"Thank you. Your request has been submitted for review.";
		const char blockHtml5Val3[] = "'); "\
			"}"\
			"</script>"\
			"</body>"\
			"</html>";

		CStringA csblockHtmlCategory = L"Category:"; 

		char blockHtml5[2048] = { 0 };
		strcpy(blockHtml5, blockHtml5Val1);
		strcat(blockHtml5, csblockHtml5Val1);
		strcat(blockHtml5, blockHtml5Val2);
		strcat(blockHtml5, csblockHtml5Val2);
		strcat(blockHtml5, blockHtml5Val3);

		char blockHtml[8192] = { 0 };
		strcpy(blockHtml, blockHtml1);
		strcat(blockHtml, strShortLink.c_str());
		strcat(blockHtml, blockHtml2);
		strcat(blockHtml, csblockHtmlCategory);
		strcat(blockHtml, strCategory.c_str());
		strcat(blockHtml, blockHtml3);
		strcat(blockHtml, blockHtml4);
		strcat(blockHtml, blockHtml5);

		PFStream * pStream;

		pStream = newObj->getStream(HS_STATUS);
		if (pStream)
		{
			pStream->write(status, sizeof(status)-1);
		}

		pStream = newObj->getStream(HS_HEADER);
		if (pStream)
		{
			PFHeader h;

			h.addField("Content-Type", "text/html", true);
			char szLen[100];
			_snprintf(szLen, sizeof(szLen), "%d", sizeof(blockHtml)-1);
			h.addField("Content-Length", szLen, true);
			h.addField("Connection", "close", true);
			pf_writeHeader(pStream, &h);
		}

		pStream = newObj->getStream(HS_CONTENT);
		if (pStream)
		{
			pStream->write(blockHtml, sizeof(blockHtml)-1);
		}

		pf_postObject(id, newObj);

		newObj->free();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::postBlockHttpResponse", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : dataPartAvailable
*  Description    : function which gets called when data is available in parts
*  Author Name    : Ram Shelke
*  Date           : 16-Aug-2016
****************************************************************************************************/
PF_DATA_PART_CHECK_RESULT MailFilter::dataPartAvailable(nfapi::ENDPOINT_ID id, PFObject * object)
{
	__try
	{
		return dataPartAvailableSEH(id, object);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in MailFilter::dataPartAvailable", 0, 0, true, SECONDLEVEL);
	}
	return DPCR_BYPASS;
}

/***************************************************************************************************
*  Function Name  : dataPartAvailable
*  Description    : function which gets called when data is available in parts
*  Author Name    : Ram Shelke
*  Date           : 16-Aug-2016
****************************************************************************************************/
PF_DATA_PART_CHECK_RESULT MailFilter::dataPartAvailableSEH(nfapi::ENDPOINT_ID id, PFObject * object)
{
	try
	{
		if (object->getType() == OT_HTTP_RESPONSE)
		{
			PFHeader h;
			if (pf_readHeader(object->getStream(HS_HEADER), &h))
			{
				PFHeaderField * pField = h.findFirstField("Content-Type");
				if (pField)
				{
					if (pField->value().find("text/") != -1)
					{
						return DPCR_FILTER;
					}
				}
			}
		}
		else
		if (object->getType() == OT_HTTP_REQUEST)
		{
			if (filterHttpRequest(id, object))
			{
				return DPCR_BLOCK;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::dataPartAvailableSEH", 0, 0, true, SECONDLEVEL);
	}
	return DPCR_BYPASS;
}

/***************************************************************************************************
*  Function Name  : ScanAttachment
*  Description    : function to scan extracted attachments
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
bool MailFilter::ScanAttachment()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBEMAILSCAN.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return false;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();

		CWardwizSQLiteTable qResult = dbSQlite.GetTable("Select SCAN_ATTACHMENTS from WWIZEMAILSCANDETAILS;");
		char sziStngID[2] = { 0 };
		strcpy_s(sziStngID, qResult.GetFieldValue(0));
		if (strcmp(sziStngID, "1") == 0)
			return true; //Scan attachments if set ON on UI
		else
			return false; //Do not scan attachments if set ON on UI
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::ScanAttachment", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***********************************************************************************************
*  Function Name  : GetClientNames
*  Description    : Get email scan clients name.
*  Author Name    : Amol Jaware
*  Date           : 29-Aug-2018
*************************************************************************************************/
void MailFilter::GetClientNames()
{
	CString csIniFilePath = GetWardWizPathFromRegistry();
	csIniFilePath += L"VBSETTINGS";
	csIniFilePath += L"\\EMAILSCANCLIENTS.INI";

	try
	{
		TCHAR szCount[50] = { 0 };
		int iCount = GetPrivateProfileInt(L"VIBRANIUMEMAILSCANCLIENTS", L"Count", 0, csIniFilePath);
		for (int iIndex = 0x01; iIndex <= iCount; iIndex++)
		{
			TCHAR szAppName[MAX_PATH] = { 0 };
			CString csKey;
			csKey.Format(L"%d", iIndex);
			ZeroMemory(szAppName, sizeof(szAppName));
			GetPrivateProfileString(L"VIBRANIUMEMAILSCANCLIENTS", csKey, L"", szAppName, MAX_PATH, csIniFilePath);
			m_vcProcessName.push_back(szAppName);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::GetClientNames", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : IsCategoryMatched
*  Description    : Function which matches URL with cloud database and get result
*  Author Name    : Ram Shelke
*  Date           : 16-Aug-2016
****************************************************************************************************/
bool MailFilter::IsCategoryMatched(std::string strURL, std::string &strCategory)
{
	try
	{
		if (!GetCategoryFromServer(strURL, strCategory))
		{
			return false;
		}
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::filterHttpRequest", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  : filterHttpRequest
*  Description    : Function which filters http request
*  Author Name    : Ram Shelke
*  Date           : 16-Aug-2016
****************************************************************************************************/
bool MailFilter::filterHttpRequest(nfapi::ENDPOINT_ID id, PFObject * object)
{
	//OutputDebugString(L">>> MailFilter::filterHttpRequest");
	bool bReturn = false;
	try
	{
		std::string url = strToLower(getHttpUrl(object));
		OutputDebugStringA(url.c_str());
		
		CString csFullURL = url.c_str();

		//check here for specific websites.
		wchar_t szHostName[MAX_PATH] = L"";
		
		DWORD dwSize = static_cast<DWORD>(csFullURL.GetLength());

		wchar_t *pszURLPath = new wchar_t[dwSize];
		if (pszURLPath == NULL)
			return bReturn;

		URL_COMPONENTS urlComp;
		memset(&urlComp, 0, sizeof(urlComp));
		urlComp.dwStructSize = sizeof(urlComp);
		urlComp.lpszHostName = szHostName;
		urlComp.dwHostNameLength = MAX_PATH;
		urlComp.lpszUrlPath = pszURLPath;
		urlComp.dwUrlPathLength = static_cast<DWORD>(csFullURL.GetLength());
		urlComp.dwSchemeLength = 1; // None zero

		if (!::WinHttpCrackUrl(csFullURL, static_cast<DWORD>(csFullURL.GetLength()), 0, &urlComp))
		{
			AddLogEntryEx(SECONDLEVEL, L"### WinHttpCrackUrl failed URLPath:[%s], LastError: [%d]", csFullURL, ::GetLastError());
			return bReturn;
		}

		CString csHostName = urlComp.lpszHostName;

		if (pszURLPath != NULL)
		{
			delete[]pszURLPath;
			pszURLPath = NULL;
		}

		csHostName.Replace(L"www.", L"");
		if (csHostName.Find(L"Vibranium") != -1)
		{
			return bReturn;
		}
		
		//check here for Browse protection flag for user
		std::string strUserName = GetUserNameFromEndPointID(id);
		if (strUserName.c_str() == EMPTY_ANSI_STRING)
		{
			return bReturn;
		}

		//check if control is enabled or not for user
		if (m_pobjParentalCtrl != NULL)
		{
			if (!m_pobjParentalCtrl->CheckUserAllowed(strUserName) && !theApp.m_bIsBrowserSecEnabled)
			{
				return bReturn;
			}
		}

		//check for Browser Security
		if (theApp.m_bIsBrowserSecEnabled)
		{
			std::string strCatID;

			if (IsCategoryMatched(url, strCatID))
			{
				CString csStrCatID = strCatID.c_str();
				CString csCategory;
				CStringA csCatID = strCatID.c_str();
				csCatID.Trim();
				int iCatID = _wtoi(csStrCatID);

				if (CheckForURLProtection(url))
				{
					if (iCatID == MALWARE)
					{
						postBlockHttpResponse(id, url, strCatID);
						InsertBrowserSecReport(url.c_str(), csStrCatID, strUserName.c_str());
						SendBlockWebData2Tray(SEND_BROWSER_SECURITY_DETAILS, url.c_str(), csStrCatID);
						return true;
					}
				}
				if (CheckForPhishingProtection())
				{
					if (iCatID == PHISHING)
					{
						postBlockHttpResponse(id, url, strCatID);
						InsertBrowserSecReport(url.c_str(), csStrCatID, strUserName.c_str());
						SendBlockWebData2Tray(SEND_BROWSER_SECURITY_DETAILS, url.c_str(), csStrCatID);
						return true;
					}
				}
			}
			else
			{
				if (CheckForSpecificURLProtection(url))
				{
					strCatID = "125";
					CString csStrCatID = strCatID.c_str();
					postBlockHttpResponse(id, url, strCatID);
					InsertBrowserSecReport(url.c_str(), csStrCatID, strUserName.c_str());
					SendBlockWebData2Tray(SEND_BROWSER_SECURITY_DETAILS, url.c_str(), csStrCatID);
					return true;
				}
			}
		}

		//checkforinternetaccess
		if (CheckForInternetAccessForUser(CStringA(strUserName.c_str()).MakeLower().GetBuffer()))
		{
			postBlockHttpResponse(id, "", "1");	// pass 1 as a parameter here for strCategory to show specific message for internet restriction
			CString csUserName(CA2T(strUserName.c_str()));
			if (!theApp.CheckUserResetValue(CStringA(strUserName.c_str()).MakeLower().GetBuffer()))
			{
				InsertIntoParCtrlReport(csUserName);
				theApp.FunSetParCtrlUserResetVal(CStringA(strUserName.c_str()).MakeLower().GetBuffer());
			}
			return true;
		}

		//check if browsing protection is enabled or not
		if (!CheckISBrowseProtectionEnabled(CStringA(strUserName.c_str()).MakeLower().GetBuffer()))
		{
			return bReturn; 
		}

		//check for exclusion here
		if (CheckExclusionList(strUserName, csHostName))
		{
			return bReturn;
		}

		//check if block category is Enabled or not
		if (CheckISBlockCategoryEnabled(CStringA(strUserName.c_str()).MakeLower().GetBuffer()) && theApp.m_bIsWebFilterEnabled && m_pobjParentalCtrl->CheckUserAllowed(strUserName))
		{
			std::string strCatID;
			std::string strCategory;
			if (IsCategoryMatched(url, strCatID))
			{
				//check here user settings for this category
				if (CheckUserSettingsForThisCategory(CStringA(strUserName.c_str()).MakeLower().GetBuffer(), strCatID, strCategory))
				{
					CString csHostName = GetShortUrl(url.c_str());
					if (!csHostName.IsEmpty())
						SendBlockWebData2Tray(SEND_WEBSITE_BLK_NOTIFICATION, csHostName, strCategory.c_str());

					postBlockHttpResponse(id, url, strCategory);
					InsertBrowseSecurityReport(url.c_str(), strCategory.c_str(), strUserName.c_str());
					return true;
				}
			}
		}

		if (CheckSpecificWebBlock(strUserName, csHostName))
		{
			std::string strCategory = "Blocked List";
			postBlockHttpResponse(id, url, strCategory);
			InsertBrowseSecurityReport(url.c_str(), strCategory.c_str(), strUserName.c_str());
			return true;
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::filterHttpRequest", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : SetParentalControlPtr
*  Description    : Fucntion which share Parental control class poiter
*  Author Name    : Ram Shelke
*  Date           : 16-Aug-2016
****************************************************************************************************/
void MailFilter::SetParentalControlPtr(CWWizParentalCntrl	*pParentalCtrl)
{
	m_pobjParentalCtrl = pParentalCtrl;
}

/**********************************************************************************************************
*  Function Name  :	InsertBrowseSessionReport
*  Description    :	Insert into Browse Session table in report
*  Author Name    : Kunal Waghmare
*  Date           : 16 July 2019
**********************************************************************************************************/
void MailFilter::InsertBrowseSecurityReport(CString csWebsiteName, CString csCategory, CString csUsername)
{
	try
	{
		int iMaxBrowseSessionID = InsertBrowseSessionReport();
		if (iMaxBrowseSessionID == 0)
		{
			AddLogEntry(L"### Exception in MailFilter::InsertBrowseSecurityReport", 0, 0, true, SECONDLEVEL);
			return;
		}
		CString csInsertQuery, csSelectQuery;
		csInsertQuery = _T("INSERT INTO Wardwiz_Browse_Security(PCDate, PCTime, WebsiteName,WebCategory, Username, BrowseSession_ID)\
							VALUES (Date('now'),Datetime('now','localtime'),'");
		csInsertQuery.Append(csWebsiteName);
		csInsertQuery.Append(L"','");
		csInsertQuery.Append(csCategory);
		csInsertQuery.Append(L"','"); 
		csInsertQuery.Append(csUsername);
		csInsertQuery.Append(L"',");
		csInsertQuery.AppendFormat(L"%d",iMaxBrowseSessionID);
		csInsertQuery.Append(_T(");"));
		CT2A ascii(csInsertQuery, CP_UTF8);
		INT64 iScanId = InsertDataToTable(ascii.m_psz);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::InsertBrowseSecurityReport", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	InsertBrowserSecReport
*  Description    :	Insert into Browse Security table in report
*  Author Name    : Jeena Mariam Saji
*  Date           : 22 Aug 2019
**********************************************************************************************************/
void MailFilter::InsertBrowserSecReport(CString csWebsiteName, CString csCategory, CString csUsername)
{
	try
	{
		int iMaxBrowseSessionID = InsertBrowseSessionReport();
		if (iMaxBrowseSessionID == 0)
		{
			AddLogEntry(L"### Exception in MailFilter::InsertBrowserSecReport", 0, 0, true, SECONDLEVEL);
			return;
		}
		CString csInsertQuery, csSelectQuery;
		csInsertQuery = _T("INSERT INTO Wardwiz_Browser_Sec(db_BSDate, db_BSTime, WebsiteName, WebCategory, Username, BrowseSession_ID)\
						   						   							VALUES (Date('now'),Datetime('now','localtime'),'");
		csInsertQuery.Append(csWebsiteName);
		csInsertQuery.Append(L"','");
		csInsertQuery.Append(csCategory);
		csInsertQuery.Append(L"','");
		csInsertQuery.Append(csUsername);
		csInsertQuery.Append(L"',");
		csInsertQuery.AppendFormat(L"%d", iMaxBrowseSessionID);
		csInsertQuery.Append(_T(");"));
		CT2A ascii(csInsertQuery, CP_UTF8);
		INT64 iScanId = InsertDataToTable(ascii.m_psz);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::InsertBrowserSecReport", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************
*  Function Name  :	InsertBrowseSessionReport
*  Description    :	Insert into Browse Session table in report and return max browse session ID
*  Author Name    : Kunal Waghmare
*  Date           : 16 July 2019
**********************************************************************************************************/
int MailFilter::InsertBrowseSessionReport()
{
	try
	{
		int iMaxID = 0;
		iMaxID = GetLastBrowseSessionID();
		if (iMaxID == 0)
		{
			CString csInsertQuery, csSelectQuery;
			csInsertQuery = _T("INSERT INTO Wardwiz_BrowseSession(SessionStartDate, SessionStartTime)\
							   						   						   						    VALUES (Date('now'),Datetime('now','localtime'));");
			CT2A ascii(csInsertQuery, CP_UTF8);
			INT64 iScanId = InsertDataToTable(ascii.m_psz);

			iMaxID = GetLastBrowseSessionID();
		}
		return iMaxID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::InsertBrowseSessionReport", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	GetLastBrowseSessionID
*  Description    :	Get max browse session id
*  Author Name    : Kunal Waghmare
*  Date           : 16 July 2019
**********************************************************************************************************/
int MailFilter::GetLastBrowseSessionID()
{
	try
	{
		int iMaxID = 0;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWardWizReportsPath = L"";
		CWardWizSQLiteDatabase dbSQlite;
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);
		CT2A dbPath(csWardWizReportsPath, CP_UTF8); 
		if (!PathFileExists(csWardWizReportsPath))
		{
			return 0;
		}

		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);
		dbSQlite.Open();
		CStringA csQuery = "SELECT MAX(BrowseSession_ID) FROM WARDWIZ_BROWSESESSION;";
		CWardwizSQLiteTable qResult = dbSQlite.GetTable(csQuery);
		DWORD dwRows = qResult.GetNumRows();
		qResult.SetRow(0);
		if (!qResult.GetFieldIsNull(0))
		{
			if (strlen(qResult.GetFieldValue(0)) != 0)
			{
				iMaxID = atoi(qResult.GetFieldValue(0));
			}
		}

		dbSQlite.Close();
		return iMaxID;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::GetLastBrowseSessionID", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/**********************************************************************************************************
*  Function Name  :	InsertDataToTable
*  Description    :	Invokes appropriate method from Database wrapper class and inserts data into SQLite tables.
*  Author Name    : Kunal Waghmare
*  Date           : 16 July 2019
**********************************************************************************************************/
INT64 MailFilter::InsertDataToTable(const char* szQuery)
{
	try
	{
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%s\\VBALLREPORTS.DB", csWardWizModulePath);
		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		m_objSqlDb.SetDataBaseFilePath(dbPath.m_psz);
		m_objSqlDb.Open();
		int iRows = m_objSqlDb.ExecDML(szQuery);
		Sleep(200);
		INT64 iLastRowId = m_objSqlDb.GetLastRowId();
		m_objSqlDb.Close();
		return iLastRowId;
	}
	catch (...)
	{
		m_objSqlDb.Close();
		AddLogEntry(L"### Exception in MailFilter::InsertDataToTable", 0, 0, true, SECONDLEVEL);
		return 0;
	}
}

/***************************************************************************
Function Name  : SendBlockWebData2Tray
Description    : Function which sends blocked url and category to tray
action
Author Name    : Nitin Shelar
Date           : 11/07/2019
****************************************************************************/
bool MailFilter::SendBlockWebData2Tray(int iMessage, CString scWebUrl, CString csWebCategory)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		memset(&szPipeData, 0, sizeof(ISPY_PIPE_DATA));
		szPipeData.iMessageInfo = iMessage;
		wcscpy_s(szPipeData.szFirstParam, scWebUrl);
		wcscpy_s(szPipeData.szSecondParam, csWebCategory);
		CISpyCommunicator objCom(TRAY_SERVER);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to set data in MailFilter::SendBlockWebData2Tray", 0, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::SendBlockWebData2Tray", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : filterHttpResponse
*  Description    : Function which filters http response
*  Author Name    : Ram Shelke
*  Date           : 16-Aug-2016
****************************************************************************************************/
bool MailFilter::filterHttpResponse(nfapi::ENDPOINT_ID id, PFObject * object)
{
	//OutputDebugString(L">>> MailFilter::filterHttpResponse");

	bool bReturn = false;
	try
	{
		PFHeader h;
		if (pf_readHeader(object->getStream(HS_HEADER), &h))
		{
			PFHeaderField * pField = h.findFirstField("Content-Type");
			if (pField)
			{
				if (pField->value().find("text/") == -1)
				{
					pf_postObject(id, object);
					return bReturn;
				}
			}
		}

		PFStream * pStream = object->getStream(HS_CONTENT);
		char * buf;

		if (pStream && pStream->size() > 0)
		{
			buf = (char*)malloc((size_t)pStream->size() + 1);
			if (buf)
			{
				pStream->read(buf, (tStreamSize)pStream->size());
				buf[pStream->size()] = '\0';

				strlwr(buf);

				if (strstr(buf, g_blockString.c_str()))
				{
					//postBlockHttpResponse(id);
					free(buf);
					return true;
				}
				free(buf);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::filterHttpResponse", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  : GetCategoryFromServer
*  Description    : function get category from server
*  Author Name    : Ram Shelke
*  Date           : 16-Aug-2016
****************************************************************************************************/
bool MailFilter::GetCategoryFromServer(std::string strURL, std::string& strURLCategory)
{
	bool bReturn = false;
	try
	{
		CString csFullURL = strURL.c_str();

		DWORD dwSize = static_cast<DWORD>(csFullURL.GetLength());

		wchar_t *pszURLPath = new wchar_t[dwSize];
		if (pszURLPath == NULL)
			return bReturn;

		wchar_t szHostName[MAX_PATH] = L"";
		URL_COMPONENTS urlComp;
		memset(&urlComp, 0, sizeof(urlComp));
		urlComp.dwStructSize = sizeof(urlComp);
		urlComp.lpszHostName = szHostName;
		urlComp.dwHostNameLength = MAX_PATH;
		urlComp.lpszUrlPath = pszURLPath;
		urlComp.dwUrlPathLength = static_cast<DWORD>(csFullURL.GetLength());
		urlComp.dwSchemeLength = 1; // None zero

		if (!::WinHttpCrackUrl(csFullURL, static_cast<DWORD>(csFullURL.GetLength()), 0, &urlComp))
		{
			AddLogEntryEx(SECONDLEVEL, L"### WinHttpCrackUrl failed URLPath:[%s], LastError: [%d]", csFullURL, ::GetLastError());
			return bReturn;
		}

		CString csHostName = CString(urlComp.lpszHostName);
		csHostName.Replace(L"www.", L"");

		if (pszURLPath != NULL)
		{
			delete[]pszURLPath;
			pszURLPath = NULL;
		}

		//check here if webURL is present or not
		if (m_csWebFilterURL.GetLength() == 0)
		{
			return bReturn;
		}

		CStringA csAHostName = CStringA(csHostName);
		csAHostName.MakeLower();

		std::string strHostName = csAHostName;
		if (CheckForURLCache(strHostName))
		{
			OutputDebugStringA("CACHE");
			OutputDebugStringA(strHostName.c_str());
			return bReturn;
		}

		CString csURL = m_csWebFilterURL + L"?url=" + csHostName;
	
		WinHttpClient	HttpClient(csURL.GetBuffer(), NULL, 0x01);

		if (m_csServer.GetLength() > 0)
		{
			HttpClient.SetProxy(m_csServer.GetBuffer());
		}

		if (m_csUserName.GetLength() > 0)
		{
			HttpClient.SetProxyUsername(m_csUserName.GetBuffer());
		}

		if (m_csPassword.GetLength() > 0)
		{
			HttpClient.SetProxyPassword(m_csPassword.GetBuffer());
		}

		HttpClient.SetTimeouts(HTTP_RESOLVE_TIMEOUT, HTTP_CONNECT_TIMEOUT, HTTP_SEND_TIMEOUT, HTTP_RECEIVE_TIMEOUT);
		HttpClient.UpdateUrl(csURL.GetBuffer());

		OutputDebugString(csURL);

		//Send HTTP request here
		if (!HttpClient.SendHttpRequest())
		{
			return false;
		}

		CString csResponse = HttpClient.GetResponseContent().c_str();

		//category ID starts from 100 - 999
		//means leght will be 3
		if (csResponse.Trim().GetLength() != 03)
		{
			AddIntoForURLCache(strHostName);
			return false;
		}
		
		strURLCategory = CStringA(csResponse);
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::GetCategoryFromServer", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
Function       : GetProxySettingsFromRegistry
Description    : This function to get settings which user has set
Author Name    : Kunal
Date           : 2nd Nov 2018
****************************************************************************/
DWORD MailFilter::GetProxySettingsFromRegistry()
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

/***************************************************************************
Function       : GetProxyServerDetailsFromDB
Description    : This function will get Proxy details here from sqlite DB
Author Name    : Kunal
Date           : 2ndth Nov 2018
****************************************************************************/
bool MailFilter::GetProxyServerDetailsFromDB()
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
		CWardwizSQLiteTable qResult = objSqlDb.GetTable("SELECT SERVER, PORT, USERNAME, PASSWORD FROM WWIZPROXYSETTINGS;");
		DWORD dwRows = qResult.GetNumRows();

		if (dwRows != 0)
		{
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

			m_csUserName = qResult.GetFieldValue(2);

			m_csPassword = qResult.GetFieldValue(3);
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
		AddLogEntry(L"### Exception in MailFilter::GetProxyServerDetailsFromDB", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/***************************************************************************
Function       : ReInitializeMailFilter
Description    : This function acts as reinitizing required variables
Author Name    : Ram Shelke
Date           : 03 Jul 2019
****************************************************************************/
bool MailFilter::ReInitializeMailFilter()
{
	bool bReturn = false;

	try
	{
		GetFilterExceptionList();
		ReloadWebSecDB();
		ReloadBrowseProtDB();
		ReloadBlkSpecWebDB();
		ReloadMngExcDB();
		ReloadURLCacheDB();

		DWORD dwProxy_Sett = GetProxySettingsFromRegistry();
		if (dwProxy_Sett == 0x02)
		{
			if (GetProxyServerDetailsFromDB())
			{
				m_HttpClient.SetProxy(m_csServer.GetBuffer());
				m_HttpClient.SetProxyUsername(m_csUserName.GetBuffer());
				m_HttpClient.SetProxyPassword(m_csPassword.GetBuffer());
			}
		}
		
		if (!GetWebFilteringURLSFromIni())
		{
			return bReturn;
		}

		for (vector<std::string>::reverse_iterator it = m_vstrWebFilterURLS.rbegin(); it != m_vstrWebFilterURLS.rend(); ++it)
		{
			std::string strURLPath = *it;

			CString csURPath = CString(strURLPath.c_str());

			m_HttpClient.SetTimeouts(HTTP_RESOLVE_TIMEOUT, HTTP_CONNECT_TIMEOUT, HTTP_SEND_TIMEOUT, HTTP_RECEIVE_TIMEOUT);
			m_HttpClient.UpdateUrl(csURPath.GetBuffer());

			//Send HTTP request here
			if (!m_HttpClient.SendHttpRequest())
			{
				continue;
			}

			std::wstring strResponse = m_HttpClient.GetResponseContent();
			if (strResponse.find(L"0") != std::string::npos)
			{
				m_csWebFilterURL = csURPath;
				bReturn = true;
				break;
			}
		}

	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::ReInitializeMailFilter", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return bReturn;
}

/***************************************************************************
Function       : UninitializeWWPCWebFilter
Description    : This function acts as uninitializing required variables
Author Name    : Amol Jaware
Date           : 18 Jul 2019
****************************************************************************/
void MailFilter::UninitializeWWPCWebFilter()
{
	try
	{
		m_MapWebCategories.clear();
		m_MapWWizBrowseProtection.clear();
		m_MapWWizSpecWebBlcok.clear();
		m_MapWWizManageExcList.clear();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::UninitializeVibraniumPCWebFilter", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function       : CheckISFilterAdded
Description    : This function acts as reinitizing required variables
Author Name    : Ram Shelke
Date           : 03 Jul 2019
****************************************************************************/
bool MailFilter::CheckISFilterAdded(nfapi::ENDPOINT_ID id, PF_FilterType type, tPF_FilterFlags Flags)
{
	bool bReturn = false;
	for (vector<STRUCTFILTERFLAG>::reverse_iterator it = m_vEndPointID.rbegin(); it != m_vEndPointID.rend(); ++it)
	{
		STRUCTFILTERFLAG stFlags = *it;
		if (stFlags.id == id && stFlags.ulFilterType == type && stFlags.ulFilterFlag == Flags)
		{
			bReturn = true;
			break;
		}
	}

	if (!bReturn)
	{
		STRUCTFILTERFLAG stFlags = { 0 };
		stFlags.id = id;
		stFlags.ulFilterType = type;
		stFlags.ulFilterFlag = Flags;
		m_vEndPointID.push_back(stFlags);
	}

	return bReturn;
}

/***************************************************************************
Function       : GetProxyServerDetailsFromDB
Description    : This function will get Proxy details here from sqlite DB
Author Name    : Kunal
Date           : 2ndth Nov 2018
****************************************************************************/
bool MailFilter::GetWebFilteringURLSFromIni()
{
	bool bReturn = false;
	try
	{
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csWebFilterIni = L"";
		csWebFilterIni.Format(L"%s\\VBSETTINGS\\WWF.INI", csWardWizModulePath);
		if (!PathFileExists(csWebFilterIni))
		{
			return false;
		}

		int iCount = GetPrivateProfileInt(L"WFURLS", L"Count", 0, csWebFilterIni);

		char szValueName[MAX_PATH] = { 0 };
		char szValueData[512] = { 0 };
		for (int i = 1; i <= static_cast<int>(iCount); i++)
		{
			sprintf(szValueName, "%lu", i);
			GetPrivateProfileStringA("WFURLS", szValueName, "", szValueData, 511, CStringA(csWebFilterIni));
			if (szValueData[0])
			{
				m_vstrWebFilterURLS.push_back(szValueData);
				bReturn = true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::GetWebFilteringURLSFromIni", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return bReturn;
}

/***************************************************************************
Function       : CheckForURLCache
Description    : This function which checks URL cache is already scanned or not
Author Name    : Ram Shelke
Date           : 08 Jul 2019
****************************************************************************/
 bool MailFilter::CheckForURLCache(std::string strURL, DWORD dwData)
{
	bool bReturn = false;
	try
	{
		URLDOMAINCACHE szURLCache = { 0 };
		strcpy(szURLCache.szUrlDomain, strURL.c_str());

		if (!m_buf2StDB.SearchItem(&szURLCache, (LPVOID&)dwData))
		{
			return false;
		}

		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::CheckForURLCache [%s]", CString(strURL.c_str()), 0, true, SECONDLEVEL);
		return false;
	}
	return bReturn;
}


/***************************************************************************
Function       : CheckForURLCache
Description    : This function which checks URL cache is already scanned or not
Author Name    : Ram Shelke
Date           : 08 Jul 2019
****************************************************************************/
bool MailFilter::AddIntoForURLCache(std::string strURL)
{
	try
	{
		URLDOMAINCACHE szURLCache = { 0 };
		strcpy(szURLCache.szUrlDomain, strURL.c_str());
		
		DWORD dwData = 0;
		if (!m_buf2StDB.AppendItem(&szURLCache, &dwData))
		{
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::AddIntoForURLCache [%s]", CString(strURL.c_str()), 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  :  ReloadWebSecDB
*  Description    :  Function to reload web browser security db
*  Author Name    :  Amol Jaware
*  Date           :  10 July 2019
****************************************************************************************************/
bool MailFilter::ReloadBrowseProtDB()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBPCWEBFILTER.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return 0;
		}

		//clear previous entries
		m_MapWWizBrowseProtection.clear();

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();

		CWardwizSQLiteTable qResult = dbSQlite.GetTable("select * from WWIZ_BROWSE_PROTECTION where db_Browse = 1;");

		Sleep(20);

		char szData[512] = { 0 };
		for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
		{
			qResult.SetRow(iRow);

			WWIZBROWSEPROTECTION szWWizBrowseProt;

			if (qResult.GetFieldIsNull(0))
			{
				continue;
			}
			char sziBrowseId[10] = { 0 };
			strcpy_s(sziBrowseId, qResult.GetFieldValue(0));
			if (strlen(sziBrowseId) == 0)
			{
				continue;
			}
			szWWizBrowseProt.iBrowseID = atoi(sziBrowseId);

			if (qResult.GetFieldIsNull(1))
			{
				continue;
			}

			char csUserName[50] = { 0 };
			strcpy_s(csUserName, qResult.GetFieldValue(1));
			if (strlen(csUserName) == 0)
			{
				continue;
			}
			szWWizBrowseProt.strUSERNAME = csUserName;

			if (qResult.GetFieldIsNull(2))
			{
				continue;
			}
			char szbyBrowse[10] = { 0 };
			strcpy_s(szbyBrowse, qResult.GetFieldValue(2));
			if (strlen(szbyBrowse) == 0)
			{
				continue;
			}
			szWWizBrowseProt.byBrowse = szbyBrowse[0];

			if (qResult.GetFieldIsNull(3))
			{
				continue;
			}
			char szbyBlk_Category[10] = { 0 };
			strcpy_s(szbyBlk_Category, qResult.GetFieldValue(3));
			if (strlen(szbyBlk_Category) == 0)
			{
				continue;
			}
			szWWizBrowseProt.byBlk_Category = szbyBlk_Category[0];

			if (qResult.GetFieldIsNull(4))
			{
				continue;
			}
			char szbyBlk_Website[10] = { 0 };
			strcpy_s(szbyBlk_Website, qResult.GetFieldValue(4));
			if (strlen(szbyBlk_Website) == 0)
			{
				continue;
			}
			szWWizBrowseProt.byBlk_Website = szbyBlk_Website[0];

			std::string strUserName = szWWizBrowseProt.strUSERNAME;

			m_MapWWizBrowseProtection.insert(make_pair(strUserName, szWWizBrowseProt));
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadWebSecDB", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  ReloadWebSecDB
*  Description    :  Function to reload web browser security db
*  Author Name    :  Amol Jaware
*  Date           :  10 July 2019
****************************************************************************************************/
bool MailFilter::ReloadWebSecDB()
{
	try
	{
		m_MapWebCategories.clear();
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBPCWEBFILTER.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return 0;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();

		CWardwizSQLiteTable qResult = dbSQlite.GetTable("Select * from WWIZ_AGE_WEB_CATEGORIES where db_LastStatus = 1;");

		Sleep(20);

		char szData[512] = { 0 };
		for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
		{
			qResult.SetRow(iRow);

			WEBAGECATEGORIES szWebCategories;
			if (qResult.GetFieldIsNull(0))
			{
				continue;
			}
			char sziId[10] = { 0 };
			strcpy_s(sziId, qResult.GetFieldValue(0));
			if (strlen(sziId) == 0)
			{
				continue;
			}
			CString csUsername;
			szWebCategories.iId = atoi(sziId);

			if (qResult.GetFieldIsNull(1))
			{
				continue;
			}
			char sziAgeGroupId[10] = { 0 };
			strcpy_s(sziAgeGroupId, qResult.GetFieldValue(1));
			szWebCategories.iAgeGroupId = atoi(sziAgeGroupId);

			if (qResult.GetFieldIsNull(2))
			{
				continue;
			}
			char szUserName[50] = { 0 };
			strcpy_s(szUserName, qResult.GetFieldValue(2));
			if (strlen(szUserName) == 0)
			{
				continue;
			}
			szWebCategories.strUSERNAME = szUserName;

			if (qResult.GetFieldIsNull(3))
			{
				continue;
			}
			char sziLastStatus[10] = { 0 };
			strcpy_s(sziLastStatus, qResult.GetFieldValue(3));
			if (strlen(sziLastStatus) == 0)
			{
				continue;
			}
			szWebCategories.byLastStatus = sziLastStatus[0];

			if (qResult.GetFieldIsNull(4))
			{
				continue;
			}
			char sziAdult[10] = { 0 };
			strcpy_s(sziAdult, qResult.GetFieldValue(4));
			if (strlen(sziAdult) == 0)
			{
				continue;
			}
			szWebCategories.byAdult = sziAdult[0];

			if (qResult.GetFieldIsNull(5))
			{
				continue;
			}
			char sziAdvertisement[10] = { 0 };
			strcpy_s(sziAdvertisement, qResult.GetFieldValue(5));
			if (strlen(sziAdvertisement) == 0)
			{
				continue;
			}
			szWebCategories.byAdvertisement = sziAdvertisement[0];

			if (qResult.GetFieldIsNull(6))
			{
				continue;
			}
			char sziAggressive[10] = { 0 };
			strcpy_s(sziAggressive, qResult.GetFieldValue(6));
			if (strlen(sziAggressive) == 0)
			{
				continue;
			}
			szWebCategories.byAggressive = sziAggressive[0];

			if (qResult.GetFieldIsNull(7))
			{
				continue;
			}
			char sziBank[10] = { 0 };
			strcpy_s(sziBank, qResult.GetFieldValue(7));
			szWebCategories.byBank = sziBank[0];

			if (qResult.GetFieldIsNull(8))
			{
				continue;
			}
			char sziBitcoin[10] = { 0 };
			strcpy_s(sziBitcoin, qResult.GetFieldValue(8));
			szWebCategories.byBitcoin = sziBitcoin[0];

			if (qResult.GetFieldIsNull(9))
			{
				continue;
			}
			char sziChat[10] = { 0 };
			strcpy_s(sziChat, qResult.GetFieldValue(9));
			szWebCategories.byChat = sziChat[0];

			if (qResult.GetFieldIsNull(10))
			{
				continue;
			}
			char sziChild[10] = { 0 };
			strcpy_s(sziChild, qResult.GetFieldValue(10));
			szWebCategories.byChild = sziChild[0];

			if (qResult.GetFieldIsNull(11))
			{
				continue;
			}
			char sziDating[10] = { 0 };
			strcpy_s(sziDating, qResult.GetFieldValue(11));
			szWebCategories.byDating = sziDating[0];

			if (qResult.GetFieldIsNull(12))
			{
				continue;
			}
			char sziDownload[10] = { 0 };
			strcpy_s(sziDownload, qResult.GetFieldValue(12));
			szWebCategories.byDownload = sziDownload[0];

			if (qResult.GetFieldIsNull(13))
			{
				continue;
			}
			char sziDrug[10] = { 0 };
			strcpy_s(sziDrug, qResult.GetFieldValue(13));
			szWebCategories.byDrug = sziDrug[0];

			if (qResult.GetFieldIsNull(14))
			{
				continue;
			}
			char sziFinancial[10] = { 0 };
			strcpy_s(sziFinancial, qResult.GetFieldValue(14));
			szWebCategories.byFinancial = sziFinancial[0];

			if (qResult.GetFieldIsNull(15))
			{
				continue;
			}
			char sziFreeWebEmails[10] = { 0 };
			strcpy_s(sziFreeWebEmails, qResult.GetFieldValue(15));
			szWebCategories.byFreeWebEmails = sziFreeWebEmails[0];

			if (qResult.GetFieldIsNull(16))
			{
				continue;
			}
			char sziGrambling[10] = { 0 };
			strcpy_s(sziGrambling, qResult.GetFieldValue(16));
			szWebCategories.byGrambling = sziGrambling[0];

			if (qResult.GetFieldIsNull(17))
			{
				continue;
			}
			char sziGames[10] = { 0 };
			strcpy_s(sziGames, qResult.GetFieldValue(17));
			szWebCategories.byGames = sziGames[0];

			if (qResult.GetFieldIsNull(18))
			{
				continue;
			}
			char sziHacking[10] = { 0 };
			strcpy_s(sziHacking, qResult.GetFieldValue(18));
			szWebCategories.byHacking = sziHacking[0];

			if (qResult.GetFieldIsNull(19))
			{
				continue;
			}
			char sziJobSearch[10] = { 0 };
			strcpy_s(sziJobSearch, qResult.GetFieldValue(19));
			szWebCategories.byJobSearch = sziJobSearch[0];

			if (qResult.GetFieldIsNull(20))
			{
				continue;
			}
			char sziSexEducation[10] = { 0 };
			strcpy_s(sziSexEducation, qResult.GetFieldValue(20));
			szWebCategories.bySexEducation = sziSexEducation[0];

			if (qResult.GetFieldIsNull(21))
			{
				continue;
			}
			char sziSocNetworking[10] = { 0 };
			strcpy_s(sziSocNetworking, qResult.GetFieldValue(21));
			szWebCategories.bySocNetworking = sziSocNetworking[0];

			if (qResult.GetFieldIsNull(22))
			{
				continue;
			}
			char sziSports[10] = { 0 };
			strcpy_s(sziSports, qResult.GetFieldValue(22));
			szWebCategories.bySports = sziSports[0];

			if (qResult.GetFieldIsNull(23))
			{
				continue;
			}
			char sziVideoStreamin[10] = { 0 };
			strcpy_s(sziVideoStreamin, qResult.GetFieldValue(23));
			szWebCategories.byVideoStreamin = sziVideoStreamin[0];

			if (qResult.GetFieldIsNull(24))
			{
				continue;
			}
			char sziAllOther[10] = { 0 };
			strcpy_s(sziAllOther, qResult.GetFieldValue(24));
			szWebCategories.byAllOther = sziAllOther[0];

			std::string strUserName = szWebCategories.strUSERNAME;

			m_MapWebCategories.insert(make_pair(strUserName, szWebCategories));
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadWebSecDB", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  ReloadBrowserSecDB
*  Description    :  Function to reload browser security db
*  Author Name    :  Jeena Mariam Saji
*  Date           :  04 Sept 2019
****************************************************************************************************/
bool MailFilter::ReloadBrowserSecDB()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBBROWSEC.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return 0;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();

		CWardwizSQLiteTable qResult = dbSQlite.GetTable("select * from WWIZBROWSESECURE where ID = 1;");

		Sleep(20);

		m_vecBrowserSec.clear();

		for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
		{
			qResult.SetRow(iRow);
			BROWSERSECURITYVAL szBrowserSecList;

			if (qResult.GetFieldIsNull(1))
			{
				continue;
			}
			char sziURLProt[10] = { 0 };

			strcpy_s(sziURLProt, qResult.GetFieldValue(1));
			szBrowserSecList.bIsURLProt = sziURLProt[0];

			if (qResult.GetFieldIsNull(2))
			{
				continue;
			}
			char szApplyWebsiteExc[10] = { 0 };
			strcpy_s(szApplyWebsiteExc, qResult.GetFieldValue(2));
			if (strlen(szApplyWebsiteExc) == 0)
			{
				continue;
			}
			szBrowserSecList.bApplyExcept = szApplyWebsiteExc[0];

			if (qResult.GetFieldIsNull(3))
			{
				continue;
			}
			char szApplyWebsiteSpec[10] = { 0 };
			strcpy_s(szApplyWebsiteSpec, qResult.GetFieldValue(3));
			if (strlen(szApplyWebsiteSpec) == 0)
			{
				continue;
			}
			szBrowserSecList.bApplySpecific = szApplyWebsiteSpec[0];

			if (qResult.GetFieldIsNull(4))
			{
				continue;
			}
			char szPhishingStatus[10] = { 0 };
			strcpy_s(szPhishingStatus, qResult.GetFieldValue(4));
			if (strlen(szPhishingStatus) == 0)
			{
				continue;
			}
			szBrowserSecList.bPhishingProt = szPhishingStatus[0];

			m_vecBrowserSec.push_back(szBrowserSecList);
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadBrowserSecDB", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  ReloadBrowserSecExclusionDB
*  Description    :  Function to reload browser security exclusion db
*  Author Name    :  Jeena Mariam Saji
*  Date           :  04 Sept 2019
****************************************************************************************************/
bool MailFilter::ReloadBrowserSecExclusionDB()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBBROWSEC.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return 0;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();

		CWardwizSQLiteTable qResult = dbSQlite.GetTable("select EXCLUDELIST from WWIZBROWSEEXCLIST;");

		Sleep(20);

		m_vecBrowserSecExc.clear();

		for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
		{
			qResult.SetRow(iRow);

			if (qResult.GetFieldIsNull(0))
			{
				continue;
			}
			char sziURLExc[200] = { 0 };

			strcpy_s(sziURLExc, qResult.GetFieldValue(0));
			m_vecBrowserSecExc.push_back(sziURLExc);
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadBrowserSecExclusionDB", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  ReloadBrowserSecSpecificDB
*  Description    :  Function to reload browser security specific db
*  Author Name    :  Jeena Mariam Saji
*  Date           :  04 Sept 2019
****************************************************************************************************/
bool MailFilter::ReloadBrowserSecSpecificDB()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBBROWSEC.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return 0;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();

		CWardwizSQLiteTable qResult = dbSQlite.GetTable("select SPECIFICLIST from WWIZBROWSESPECLIST;");

		Sleep(20);

		m_vecBrowserSecSpec.clear();

		for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
		{
			qResult.SetRow(iRow);

			if (qResult.GetFieldIsNull(0))
			{
				continue;
			}
			char sziURLSpec[200] = { 0 };

			strcpy_s(sziURLSpec, qResult.GetFieldValue(0));
			m_vecBrowserSecSpec.push_back(sziURLSpec);
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ReloadBrowserSecSpecificDB", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  GetUserNameFromEndPointID
*  Description    :  Function to get user name from its Endpoint ID.
*  Author Name    :  Ramkrushna Shelke
*  Date           :  12 July 2019
****************************************************************************************************/
std::string MailFilter::GetUserNameFromEndPointID(nfapi::ENDPOINT_ID id)
{
	try
	{
		MAPCONNECTEDENDPOINTS::iterator it = m_pConnectedEndPoints.find(id);
		if (it != m_pConnectedEndPoints.end())
		{
			return it->second;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::GetUserNameFromEndPointID", 0, 0, true, SECONDLEVEL);
	}
	return EMPTY_ANSI_STRING;
}

/***************************************************************************************************
*  Function Name  :  InsertUserNameandID
*  Description    :  Function to put user name from its Endpoint ID.
*  Author Name    :  Ramkrushna Shelke
*  Date           :  12 July 2019
****************************************************************************************************/
bool MailFilter::InsertUserNameandID(nfapi::ENDPOINT_ID id, std::string strUserName)
{
	try
	{
		if (CheckForDuplicates(id))
		{
			return true;
		}
		m_pConnectedEndPoints.insert(make_pair(id, strUserName));
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::InsertUserNameandID", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  :  CheckForDuplicates
*  Description    :  Function to get user name from its Endpoint ID.
*  Author Name    :  Ramkrushna Shelke
*  Date           :  12 July 2019
****************************************************************************************************/
bool MailFilter::CheckForDuplicates(nfapi::ENDPOINT_ID id)
{
	try
	{
		MAPCONNECTEDENDPOINTS::iterator it = m_pConnectedEndPoints.find(id);
		if (it != m_pConnectedEndPoints.end())
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::CheckForDuplicates", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  :  CheckForInternetAccessForUser
*  Description    :  Function which takes user name in parameter and checks for its respective flag
*  Author Name    :  Jeena Mariam Saji
*  Date           :  16 July 2019
****************************************************************************************************/
bool MailFilter::CheckForInternetAccessForUser(std::string strUserName)
{
	try
	{
		if (m_pobjParentalCtrl != NULL && theApp.m_bIsWebFilterEnabled)
		{
			if (m_pobjParentalCtrl->CheckUserAllowed(strUserName))
			{
				CString csUserName(CA2T(strUserName.c_str()));
				if (!m_pobjParentalCtrl->CheckInternetUsagePermission(csUserName))
				{
					m_pobjParentalCtrl->SendParCtrlMessage2Tray(SEND_PC_INET_TRAY_LOCK_WND, csUserName);
					return true;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::CheckForInternetAccessForUser", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  :  CheckISBrowseProtectionEnabled
*  Description    :  Function which takes user name in parameter and checks for its respective flag
*  Author Name    :  Ramkrushna Shelke
*  Date           :  12 July 2019
****************************************************************************************************/
bool MailFilter::CheckISBrowseProtectionEnabled(std::string strUserName)
{
	try
	{
		MAPBROWSEPROT::iterator it = m_MapWWizBrowseProtection.find(strUserName);
		if (it != m_MapWWizBrowseProtection.end())
		{
			if (it->second.byBrowse == 0x31)
			{
				return true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::CheckISBrowseProtectionEnabled", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  :  CheckISBrowseProtectionEnabled
*  Description    :  Function which takes user name in parameter and checks for its respective flag
*  Author Name    :  Ramkrushna Shelke
*  Date           :  12 July 2019
****************************************************************************************************/
bool MailFilter::CheckISBlockCategoryEnabled(std::string strUserName)
{
	try
	{
		MAPBROWSEPROT::iterator it = m_MapWWizBrowseProtection.find(strUserName);
		if (it != m_MapWWizBrowseProtection.end())
		{
			if (it->second.byBlk_Category == 0x31)
			{
				return true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::CheckISBlockCategoryEnabled", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  :  CheckISBlockSpecificWebsiteEnabled
*  Description    :  Function which takes user name in parameter and checks for its respective flag
*  Author Name    :  Ramkrushna Shelke
*  Date           :  12 July 2019
****************************************************************************************************/
bool MailFilter::CheckISBlockSpecificWebsiteEnabled(std::string strUserName)
{
	try
	{
		MAPBROWSEPROT::iterator it = m_MapWWizBrowseProtection.find(strUserName);
		if (it != m_MapWWizBrowseProtection.end())
		{
			if (it->second.byBlk_Website == 0x31)
			{
				return true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::CheckISBlockSpecificWebsiteEnabled", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  :  CheckUserSettingsForThisCategory
*  Description    :  Function to check for user settings for this cateory
*  Author Name    :  Ramkrushna Shelke
*  Date           :  13 July 2019
****************************************************************************************************/
bool MailFilter::CheckUserSettingsForThisCategory(std::string strUserName, std::string strCatID, std::string &strCatName)
{
	try
	{
		if (strCatID.length() == 0)
			return false;

		MAPWEBCAT::iterator it = m_MapWebCategories.find(strUserName);
		if (it == m_MapWebCategories.end())
		{
			return false;
		}

		CStringA csCatID = strCatID.c_str();
		csCatID.Trim();

		int iCatID = atoi(csCatID.GetBuffer());
		switch (iCatID)
		{
		case ADULT :
			strCatName = "Adult";
			if (it->second.byAdult == 0x30)
				return true;
			break;
		case ADVERTISEMENT:
			strCatName = "Advertisement";
			if (it->second.byAdvertisement == 0x30)
				return true;
			break;
		case AGGRESSIVE:
			strCatName = "Aggresive";
			if (it->second.byAggressive == 0x30)
				return true;
			break;
		case BANK:
			strCatName = "Bank";
			if (it->second.byBank == 0x30)
				return true;
			break;
		case BITCOIN:
			strCatName = "Bitcoin";
			if (it->second.byBitcoin == 0x30)
				return true;
			break;
		case CHAT:
			strCatName = "Chat";
			if (it->second.byChat == 0x30)
				return true;
			break;
		case CHILD:
			strCatName = "Child";
			if (it->second.byChild == 0x30)
				return true;
			break;
		case DATING:
			strCatName = "Dating";
			if (it->second.byDating == 0x30)
				return true;
			break;
		case DOWNLOAD:
			strCatName = "Download";
			if (it->second.byDownload == 0x30)
				return true;
			break;
		case DRUG:
			strCatName = "Drug";
			if (it->second.byDrug == 0x30)
				return true;
			break;
		case FINANCIAL:
			strCatName = "Financial";
			if (it->second.byFinancial == 0x30)
				return true;
			break;
		case FREEWEBEMAILS:
			strCatName = "Free Emails";
			if (it->second.byFreeWebEmails == 0x30)
				return true;
			break;
		case GRAMBLING:
			strCatName = "Gambling";
			if (it->second.byGrambling == 0x30)
				return true;
			break;
		case GAMES:
			strCatName = "Games";
			if (it->second.byGames == 0x30)
				return true;
			break;
		case HACKING:
			strCatName = "Hacking";
			if (it->second.byHacking == 0x30)
				return true;
			break;
		case JOBSEARCH:
			strCatName = "JobSearch";
			if (it->second.byJobSearch == 0x30)
				return true;
			break;
		case SEXEDUCATION:
			strCatName = "Sexual Education";
			if (it->second.bySexEducation == 0x30)
				return true;
			break;
		case SOCNETWORKING:
			strCatName = "Social Networking";
			if (it->second.bySocNetworking == 0x30)
				return true;
			break;
		case SPORTS:
			strCatName = "Sports";
			if (it->second.bySports == 0x30)
				return true;
			break;
		case VIDEOSTREAMIN:
			strCatName = "Video Streaming";
			if (it->second.byVideoStreamin == 0x30)
				return true;
			break;
		case ALLOTHER:
			strCatName = "Other";
			if (it->second.byAllOther == 0x30)
				return true;
			break;
		default:
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::CheckUserSettingsForThisCategory", 0, 0, true, SECONDLEVEL);
	}
	return false;
}


/***************************************************************************************************
*  Function Name  :  ReloadBlkSpecWebDB
*  Description    :  Function to reload web browser security db
*  Author Name    :  Amol Jaware
*  Date           :  10 July 2019
****************************************************************************************************/
bool MailFilter::ReloadBlkSpecWebDB()
{
	try
	{
		m_MapWWizSpecWebBlcok.clear();

		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBPCWEBFILTER.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return 0;
		}

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();

		CWardwizSQLiteTable qResult4UserName = dbSQlite.GetTable("select FK_USERNAME from WWIZ_SPEC_WEB_BLOCK;");

		for (int iUserRow = 0; iUserRow < qResult4UserName.GetNumRows(); iUserRow++)
		{
			WWizSpecWebBlock szWWizSpecWebBlock;

			qResult4UserName.SetRow(iUserRow);

			if (qResult4UserName.GetFieldIsNull(0))
			{
				continue;
			}
			char szUserName[100] = { 0 };
			strcpy_s(szUserName, qResult4UserName.GetFieldValue(0));
			if (strlen(szUserName) == 0)
			{
				continue;
			}
			szWWizSpecWebBlock.strUserName = szUserName;

			CStringA csQuery;
			csQuery.Format("select * from WWIZ_SPEC_WEB_BLOCK where FK_USERNAME = '%s';", szUserName);
			CWardwizSQLiteTable qResult = dbSQlite.GetTable(csQuery);

			Sleep(20);

			std::vector<WWizSpecWebBlock> vBlkSpecWebsite;
			for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
			{
				qResult.SetRow(iRow);

				if (qResult.GetFieldIsNull(2))
				{
					continue;
				}
				char szBlkSpecWebsite[MAX_PATH] = { 0 };
				strcpy_s(szBlkSpecWebsite, qResult.GetFieldValue(2));
				if (strlen(szBlkSpecWebsite) == 0)
				{
					continue;
				}
				bool bFlag = false;
				std::string strURLPath(szBlkSpecWebsite);
				std::string::size_type Found = strURLPath.find("https://");
				
				if (Found != string::npos)
					bFlag = true;
				if (!bFlag)
				{
					Found = NULL;
					Found = strURLPath.find("http://");
					if (Found != string::npos)
						bFlag = true;
				}

				if (!bFlag)
					strURLPath = "http://" + strURLPath;

				CString csFullURL = strURLPath.c_str();

				DWORD dwSize = static_cast<DWORD>(csFullURL.GetLength());

				wchar_t *pszURLPath = new wchar_t[dwSize];
				if (pszURLPath == NULL)
					continue;

				//check here for specific websites.
				wchar_t szHostName[MAX_PATH] = L"";
				URL_COMPONENTS urlComp;
				memset(&urlComp, 0, sizeof(urlComp));
				urlComp.dwStructSize = sizeof(urlComp);
				urlComp.lpszHostName = szHostName;
				urlComp.dwHostNameLength = MAX_PATH;
				urlComp.lpszUrlPath = pszURLPath;
				urlComp.dwUrlPathLength = static_cast<DWORD>(csFullURL.GetLength());
				urlComp.dwSchemeLength = 1; // None zero

				if (!::WinHttpCrackUrl(csFullURL, static_cast<DWORD>(csFullURL.GetLength()), 0, &urlComp))
				{
					AddLogEntryEx(SECONDLEVEL, L"### WinHttpCrackUrl failed URLPath:[%s], LastError: [%d]", csFullURL, ::GetLastError());
					return 0;
				}

				CString csHostName = urlComp.lpszHostName;
				csHostName.Replace(L"www.", L"");
				CT2CA pszHostName(csHostName);
				szWWizSpecWebBlock.strWebsiteName = pszHostName;

				if (pszURLPath != NULL)
				{
					delete[]pszURLPath;
					pszURLPath = NULL;
				}

				if (qResult.GetFieldIsNull(3))
				{
					continue;
				}
				char szSubdomain[10] = { 0 };
				strcpy_s(szSubdomain, qResult.GetFieldValue(3));
				if (strlen(szSubdomain) == 0)
				{
					continue;
				}
				szWWizSpecWebBlock.byIsSubDomain = szSubdomain[0];
				vBlkSpecWebsite.push_back(szWWizSpecWebBlock);
			}
			
			std::string strUserName = szWWizSpecWebBlock.strUserName;
			m_MapWWizSpecWebBlcok.insert(make_pair(strUserName, vBlkSpecWebsite));
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::ReloadBlkSpecWebDB", 0, 0, true, SECONDLEVEL);
	}

	return 0;
}

/***************************************************************************************************
*  Function Name  :  ReloadMngExcDB
*  Description    :  Function to reload web browser security db
*  Author Name    :  Amol Jaware
*  Date           :  10 July 2019
****************************************************************************************************/
bool MailFilter::ReloadMngExcDB()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sVBPCWEBFILTER.DB", csWardWizModulePath);

		if (!PathFileExists(csWardWizReportsPath))
		{
			return 0;
		}

		m_MapWWizManageExcList.clear();

		CT2A dbPath(csWardWizReportsPath, CP_UTF8);
		dbSQlite.SetDataBaseFilePath(dbPath.m_psz);

		dbSQlite.Open();

		CWardwizSQLiteTable qResult4UserName = dbSQlite.GetTable("select distinct FK_USERNAME from WWIZ_MANAGE_EXCLUSION_LIST;");

		for (int iUserRow = 0; iUserRow < qResult4UserName.GetNumRows(); iUserRow++)
		{
			qResult4UserName.SetRow(iUserRow);

			WWIZMANAGEEXCLIST szWWizMangExcList;

			if (qResult4UserName.GetFieldIsNull(0))
			{
				continue;
			}
			char szUserName[100] = { 0 };
			strcpy_s(szUserName, qResult4UserName.GetFieldValue(0));
			if (strlen(szUserName) == 0)
			{
				continue;
			}
			szWWizMangExcList.strUserName = szUserName;

			CStringA csQuery;
			csQuery.Format("select * from WWIZ_MANAGE_EXCLUSION_LIST where FK_USERNAME = '%s';", szUserName);
			CWardwizSQLiteTable qResult = dbSQlite.GetTable(csQuery);

			Sleep(20);

			std::vector<std::string> vMngExcList;
			for (int iRow = 0; iRow < qResult.GetNumRows(); iRow++)
			{
				qResult.SetRow(iRow);

				if (qResult.GetFieldIsNull(0))
				{
					continue;
				}
				char sziId[10] = { 0 };
				strcpy_s(sziId, qResult.GetFieldValue(0));
				if (strlen(sziId) == 0)
				{
					continue;
				}
				szWWizMangExcList.iId = atoi(sziId);

				if (qResult.GetFieldIsNull(2))
				{
					continue;
				}
				char szExcList[MAX_PATH] = { 0 };
				strcpy_s(szExcList, qResult.GetFieldValue(2));
				if (strlen(szExcList) == 0)
				{
					continue;
				}
				vMngExcList.push_back(szExcList);
			}

			std::string strUserName = szWWizMangExcList.strUserName;
			m_MapWWizManageExcList.insert(make_pair(strUserName, vMngExcList));
		}
		dbSQlite.Close();
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::ReloadMngExcDB", 0, 0, true, SECONDLEVEL);
	}
	return 0;
}

/***************************************************************************************************
*  Function Name  :  ReloadURLCacheDB
*  Description    :  Function to reload URL cache database from file
*  Author Name    :  Ram Shelke
*  Date           :  15 July 2019
****************************************************************************************************/
bool MailFilter::ReloadURLCacheDB()
{
	try
	{
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWWizURLCacheDB = L"";
		csWWizURLCacheDB.Format(L"%sVBURLCACHE.DB", csWardWizModulePath);

		if (!PathFileExists(csWWizURLCacheDB))
		{
			return false;
		}

		if (!m_buf2StDB.Load(csWWizURLCacheDB, false))
		{
			AddLogEntry(L"### Failed to load DB, Path: [%s]", 0, 0, true, SECONDLEVEL);
			return false;
		}

		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::ReloadURLCacheDB", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  :  SaveURLDB
*  Description    :  Function to save URL cache database to file
*  Author Name    :  Ram Shelke
*  Date           :  15 July 2019
****************************************************************************************************/
bool MailFilter::SaveURLDB()
{
	try
	{
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		if (!PathFileExists(csWardWizModulePath))
		{
			return false;
		}

		CString	csWWizURLCacheDB = L"";
		csWWizURLCacheDB.Format(L"%sVBURLCACHE.DB", csWardWizModulePath);

		if (PathFileExists(csWWizURLCacheDB))
		{
			SetFileAttributes(csWWizURLCacheDB, FILE_ATTRIBUTE_NORMAL);
			DeleteFile(csWWizURLCacheDB);
		}

		if (!m_buf2StDB.Save(csWWizURLCacheDB))
		{
			AddLogEntry(L"### Failed to Save DB, Path: [%s]", 0, 0, true, SECONDLEVEL);
			return false;
		}

		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::SaveURLDB", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************************************
*  Function Name  :  CheckSpecificWebBlock
*  Description    :  Function check the domain in specified list
*  Author Name    :  Kunal Waghmare
*  Date           :  18 July 2019
****************************************************************************************************/
bool MailFilter::CheckSpecificWebBlock(std::string strUserName, CString csHostName)
{
	bool bReturn = false;
	try
	{
		if (!CheckISBlockSpecificWebsiteEnabled(strUserName))
		{
			return bReturn;
		}

		MAPWWIZSPECWEBBLOCK::iterator itrWWizSpecWebBlcok = m_MapWWizSpecWebBlcok.find(strUserName);
		
		if (itrWWizSpecWebBlcok == m_MapWWizSpecWebBlcok.end())
		{
			return bReturn;
		}

		std::vector<WWizSpecWebBlock> vBlockDomainList = itrWWizSpecWebBlcok->second;
		for (std::vector<WWizSpecWebBlock>::iterator it = vBlockDomainList.begin(); it != vBlockDomainList.end(); ++it)
		{
			WWizSpecWebBlock stBlockDomain = *it;
			std::string strURLPath = stBlockDomain.strWebsiteName;
			BYTE byIsSubDomain = stBlockDomain.byIsSubDomain;


			CStringA csaHostName = CStringA(csHostName);
			if (csaHostName.CompareNoCase(strURLPath.c_str()) == 0 && byIsSubDomain != 1)
			{
				bReturn = true;
				break;
			}

			if (byIsSubDomain == '1')
			{
				char charHostName[MAX_PATH] = { 0 };
				strcpy(charHostName, CStringA(csHostName.GetString()));
				char *chpString = strstr(charHostName, strURLPath.c_str());

				if (chpString)
				{
					bReturn = true;
					break;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::CheckSpecificWebBlock, URL:[%s]", csHostName.GetString(), 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  :  CheckExclusionList
*  Description    :  Function to check exclusion list to avoid blocking from PC.
*  Author Name    :  Amol Jaware
*  Date           :  24 July 2019
****************************************************************************************************/
bool MailFilter::CheckExclusionList(std::string strUserName, CString csHostName)
{
	bool bReturn = false;
	try
	{
		if ( !CheckISBlockCategoryEnabled(CStringA(strUserName.c_str()).MakeLower().GetBuffer()))
		{
			return bReturn;
		}

		MAPWWIZMANAGEEXCLIST::iterator itrWWizExclusionList = m_MapWWizManageExcList.find(strUserName);

		if (itrWWizExclusionList == m_MapWWizManageExcList.end())
		{
			return bReturn;
		}

		CStringA csaHostName = CStringA(csHostName);

		std::vector<std::string> vBlockDomainList = itrWWizExclusionList->second;
		for (std::vector<std::string>::iterator it = vBlockDomainList.begin(); it != vBlockDomainList.end(); ++it)
		{
			std::string strExcludeList = *it;
			CString csaExcludeList;
			
			std::string::size_type Found = strExcludeList.find("www.");

			if (Found != std::string::npos)
				strExcludeList.erase(Found, 4);

			if (csaHostName.CompareNoCase(strExcludeList.c_str()) == 0)
			{
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::CheckSpecificWebBlock, URL:[%s]", csHostName.GetString(), 0, true, SECONDLEVEL);
	}
	
	return bReturn;
}

/***************************************************************************************************
*  Function Name  :  CheckForURLProtection
*  Description    :  Function to check values for Browser Security
*  Author Name    :  Jeena Mariam Saji
*  Date           :  04 Sept 2019
****************************************************************************************************/
bool MailFilter::CheckForURLProtection(std::string strUrl)
{
	bool bReturn = false;
	try
	{
		BROWSERSECMAP::iterator vBrowserSecList;
		for (vBrowserSecList = m_vecBrowserSec.begin(); vBrowserSecList != m_vecBrowserSec.end(); vBrowserSecList++)
		{
			if (vBrowserSecList->bIsURLProt == '1')
			{
				if (vBrowserSecList->bApplyExcept == '1')
				{
					if (CheckForURLException(strUrl))
					{
						return false;
					}
				}
				bReturn = true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::CheckForURLProtection", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  :  CheckForSpecificURLProtection
*  Description    :  Function to check values for Browser Security
*  Author Name    :  Jeena Mariam Saji
*  Date           :  13 Nov 2019
****************************************************************************************************/
bool MailFilter::CheckForSpecificURLProtection(std::string strUrl)
{
	bool bReturn = false;
	try
	{
		BROWSERSECMAP::iterator vBrowserSecList;
		for (vBrowserSecList = m_vecBrowserSec.begin(); vBrowserSecList != m_vecBrowserSec.end(); vBrowserSecList++)
		{
			if (vBrowserSecList->bIsURLProt == '1')
			{
				if (vBrowserSecList->bApplySpecific == '1')
				{
					if (CheckForSpecificWebsite(strUrl))
					{
						bReturn = true;
					}
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::CheckForSpecificURLProtection", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  :  CheckForURLException
*  Description    :  Function to check exclusion list for Browser Security
*  Author Name    :  Jeena Mariam Saji
*  Date           :  04 Sept 2019
****************************************************************************************************/
bool MailFilter::CheckForURLException(std::string strUrl)
{
	bool bReturn = false;
	try
	{
		CString csHostName = GetShortUrl(strUrl.c_str());
		BROWSERSECEXCLUSION::iterator vBrowserSecExclusion;
		for (vBrowserSecExclusion = m_vecBrowserSecExc.begin(); vBrowserSecExclusion != m_vecBrowserSecExc.end(); vBrowserSecExclusion++)
		{
			std::string strURLPath = *vBrowserSecExclusion;
			CStringA csaHostName = CStringA(csHostName);
			CString csURPath = CString(strURLPath.c_str());
			csURPath.Replace(L"www.", L"");
			CStringA csaURLPath = CStringA(csURPath);

			if (csaHostName.CompareNoCase(csaURLPath) == 0)
			{
				return true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::CheckForURLException", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  :  CheckForSpecificWebsite
*  Description    :  Function to check specific list for Browser Security
*  Author Name    :  Jeena Mariam Saji
*  Date           :  04 Sept 2019
****************************************************************************************************/
bool MailFilter::CheckForSpecificWebsite(std::string strUrl)
{
	bool bReturn = false;
	try
	{
		CString csHostName = GetShortUrl(strUrl.c_str());
		csHostName.Replace(L"www.", L"");
		BROWSERSECSPECIFIC::iterator vBrowserSecSpecific;
		for (vBrowserSecSpecific = m_vecBrowserSecSpec.begin(); vBrowserSecSpecific != m_vecBrowserSecSpec.end(); vBrowserSecSpecific++)
		{
			std::string strURLPath = *vBrowserSecSpecific;
			CStringA csaHostName = CStringA(csHostName);
			CString csURPath = CString(strURLPath.c_str());
			csURPath.Replace(L"www.", L"");
			CStringA csaURLPath = CStringA(csURPath);

			if (csaHostName.CompareNoCase(csaURLPath) == 0)
			{
				return true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::CheckForSpecificWebsite", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  :  CheckForPhishingProtection
*  Description    :  Function to check phishing list for Browser Security
*  Author Name    :  Jeena Mariam Saji
*  Date           :  04 Sept 2019
****************************************************************************************************/
bool MailFilter::CheckForPhishingProtection()
{
	bool bReturn = false;
	try
	{
		BROWSERSECMAP::iterator vBrowserSecList;
		for (vBrowserSecList = m_vecBrowserSec.begin(); vBrowserSecList != m_vecBrowserSec.end(); vBrowserSecList++)
		{
			if (vBrowserSecList->bPhishingProt == '1')
			{
				return true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::CheckForPhishingProtection", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : GetFilterExceptionList
*  Description    : Get list of application which needs to be excluded from filter
*  Author Name    : Ram Shelke
*  Date           : 16-Jul-2019
*************************************************************************************************/
void MailFilter::GetFilterExceptionList()
{
	CString csIniFilePath = GetWardWizPathFromRegistry();
	csIniFilePath += L"VBSETTINGS";
	csIniFilePath += L"\\WWF.INI";
	try
	{
		TCHAR szCount[50] = { 0 };
		int iCount = GetPrivateProfileInt(L"FILTER_EXCEPTIONS", L"Count", 0, csIniFilePath);
		for (int iIndex = 0x01; iIndex <= iCount; iIndex++)
		{
			CHAR szAppName[MAX_PATH] = { 0 };
			CStringA csKey;
			csKey.Format("%d", iIndex);
			ZeroMemory(szAppName, sizeof(szAppName));
			GetPrivateProfileStringA("FILTER_EXCEPTIONS", csKey, "", szAppName, MAX_PATH, CStringA(csIniFilePath));
			m_vcFilterExceptionlist.push_back(szAppName);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::GetClientNames", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
*  Function Name  : ISFilterExclusion
*  Description    : Function which checks for is application is listed into filter exclusion list
*  Author Name    : Ram Shelke
*  Date           : 16-Jul-2019
*************************************************************************************************/
bool MailFilter::ISFilterExcludedProcess(std::wstring strProcessName)
{
	bool bReturn = false;

	try
	{
		if (strProcessName.length() == 0)
			return false;

		CString csProcessName(strProcessName.c_str());
		if (csProcessName.CompareNoCase(L"VBupdate.exe") == 0)
			return true;

		for (vector<std::string>::reverse_iterator it = m_vcFilterExceptionlist.rbegin(); it != m_vcFilterExceptionlist.rend(); ++it)
		{
			CStringA csProcessName = strProcessName.c_str();
			csProcessName.MakeLower();
			if (csProcessName.CompareNoCase((*it).c_str()) == 0x00)
			{
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::ISFilterExclusion", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***********************************************************************************************
*  Function Name  : ISApplicationListed2Filter
*  Description    : Function to whether need to add filter 
*  Author Name    : Ram Shelke
*  Date           : 16-Jul-2019
*************************************************************************************************/
bool MailFilter::ISApplicationListed2Filter(CString csProcessName)
{
	bool bReturn = false;

	try
	{
		if (csProcessName.GetLength() == 0)
			return false;

		for (vector<CString>::reverse_iterator it = m_vcProcessName.rbegin(); it != m_vcProcessName.rend(); ++it)
		{
			if (csProcessName.CompareNoCase(*it) == 0x00)
			{
				bReturn = true;
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::ISFilterExclusion", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************************************
*  Function Name  :  GetShortUrl
*  Description    :  Function to short web Url
*  Author Name    :  Nitin Shelar
*  Date           :  17/07/2019
****************************************************************************************************/
CString MailFilter::GetShortUrl(CString csUrl)
{
	try
	{
		DWORD dwSize = static_cast<DWORD>(csUrl.GetLength());

		wchar_t *pszURLPath = new wchar_t[dwSize];
		if (pszURLPath == NULL)
			return EMPTY_STRING;

		wchar_t szHostName[MAX_PATH] = L"";
		URL_COMPONENTS urlComp;
		memset(&urlComp, 0, sizeof(urlComp));
		urlComp.dwStructSize = sizeof(urlComp);
		urlComp.lpszHostName = szHostName;
		urlComp.dwHostNameLength = MAX_PATH;
		urlComp.lpszUrlPath = pszURLPath;
		urlComp.dwUrlPathLength = static_cast<DWORD>(csUrl.GetLength());

		urlComp.dwSchemeLength = 1; // None zero

		if (!::WinHttpCrackUrl(csUrl, static_cast<DWORD>(csUrl.GetLength()), 0, &urlComp))
		{
			AddLogEntryEx(SECONDLEVEL, L"### WinHttpCrackUrl failed URLPath:[%s], LastError: [%d]", csUrl, ::GetLastError());
			return "";
		}

		CString csHostName = CString(urlComp.lpszHostName);

		if (pszURLPath != NULL)
		{
			delete[]pszURLPath;
			pszURLPath = NULL;
		}

		if (csHostName.GetLength() < 25)
			return csHostName;

		CString csDestUrl;
		csDestUrl.Format(L"%s%s%s", csHostName.Left(10), L"...", csHostName.Right(11));
		return csDestUrl;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in MailFilter::GetShortUrl", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/***************************************************************************
Function       : ClearCache
Description    : This function will clean prepared cache
Author Name    : Ram Shelke
Date           : 30 Jul 2019
****************************************************************************/
bool MailFilter::ClearURLCache()
{
	try
	{
		if (!m_buf2StDB.RemoveAll())
		{
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in ClearURLCache", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************
Function       : InsertIntoParCtrlReport
Description    : This function will insert par ctrl report into DB
Author Name    : Jeena Mariam Saji
Date           : 31 Jul 2019
****************************************************************************/
void MailFilter::InsertIntoParCtrlReport(CString csUserName)
{
	try
	{
		DWORD dwFlag = 0x05;
		CString csInsertQuery, csSelectQuery;
		csInsertQuery = _T("INSERT INTO Wardwiz_ParentalCtrl_Details(db_PCDate, db_PCTime, db_PCActivity, db_Username) VALUES (Date('now'),Datetime('now','localtime'),");
		csInsertQuery.AppendFormat(L"%d", dwFlag);
		csInsertQuery.Append(L",'");
		csInsertQuery.Append(csUserName);
		csInsertQuery.Append(_T("');"));
		CT2A ascii(csInsertQuery, CP_UTF8);
		INT64 iScanId = InsertDataToTable(ascii.m_psz);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in InsertIntoParCtrlReport", 0, 0, true, SECONDLEVEL);
	}
}

/***********************************************************************************************
*  Function Name  : SetDefaultBehaviour
*  Description    : Function to set default application behaviour
*  Author Name    : Ram Shelke
*  Date           : 18-04-2019
*************************************************************************************************/
void EventHandler::SetDefaultBehaviour(DWORD dwEnable)
{
	__try
	{
		m_iSetDefAppBehav = (int)dwEnable;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in MailFilter::SetDefaultBehaviour", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************************************
*  Function Name  : threadStart
*  Description    :
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::threadStart()
{
}

/***************************************************************************************************
*  Function Name  : threadEnd
*  Description    :
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::threadEnd()
{
}

/***************************************************************************************************
*  Function Name  : tcpConnectRequest
*  Description    : Function which gets called when request is came to connect via TCP
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::tcpConnectRequest(ENDPOINT_ID id, PNF_TCP_CONN_INFO pConnInfo)
{
	pf_getNFEventHandler()->tcpConnectRequest(id, pConnInfo);
}

/***************************************************************************************************
*  Function Name  : tcpConnectRequest
*  Description    : Function which gets called when request is came to connect via TCP
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::tcpConnected(ENDPOINT_ID id, PNF_TCP_CONN_INFO pConnInfo)
{
	nfapi::NF_TCP_CONN_INFO ConnInfo;
	nfapi::nf_getTCPConnInfo(id, &ConnInfo);

	//check here if application is blocked for internet usage.
	WCHAR szProcessName[MAX_PATH] = { 0 };
	nf_getProcessName(pConnInfo->processId, szProcessName, MAX_PATH);
	//make lower
	wcslwr(szProcessName);

	//handle TCP requests
	HandleTcpUdpRequests(id, szProcessName);

	pf_getNFEventHandler()->tcpConnected(id, pConnInfo);
}

/***************************************************************************************************
*  Function Name  : tcpClosed
*  Description    : Function which gets called when TCP close request comes
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::tcpClosed(ENDPOINT_ID id, PNF_TCP_CONN_INFO pConnInfo)
{
	//OutputDebugString(L"NF_EventHandler:: tcpClosed\n");
	pf_getNFEventHandler()->tcpClosed(id, pConnInfo);
}

/***************************************************************************************************
*  Function Name  : tcpReceive
*  Description    :
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::tcpReceive(ENDPOINT_ID id, const char * buf, int len)
{
	//OutputDebugString(L"NF_EventHandler:: tcpReceive\n");
	pf_getNFEventHandler()->tcpReceive(id, buf, len);
}

/***************************************************************************************************
*  Function Name  : tcpSend
*  Description    :
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::tcpSend(ENDPOINT_ID id, const char * buf, int len)
{
	//OutputDebugString(L"NF_EventHandler:: tcpSend\n");
	pf_getNFEventHandler()->tcpSend(id, buf, len);
}

/***************************************************************************************************
*  Function Name  : tcpCanReceive
*  Description    :
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::tcpCanReceive(ENDPOINT_ID id)
{
	//OutputDebugString(L"NF_EventHandler:: tcpCanReceive\n");
	pf_getNFEventHandler()->tcpCanReceive(id);
}

/***************************************************************************************************
*  Function Name  : tcpCanSend
*  Description    :
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::tcpCanSend(ENDPOINT_ID id)
{
	//OutputDebugString(L"NF_EventHandler:: tcpCanSend\n");
	pf_getNFEventHandler()->tcpCanSend(id);
}

/***************************************************************************************************
*  Function Name  : udpCreated
*  Description    :
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::udpCreated(ENDPOINT_ID id, PNF_UDP_CONN_INFO pConnInfo)
{
	OutputDebugString(L">>> EventHandler:: udpCreated");
	try
	{
		nfapi::NF_TCP_CONN_INFO ConnInfo;
		nfapi::nf_getTCPConnInfo(id, &ConnInfo);

		//check here if application is blocked for internet usage.
		WCHAR szProcessName[MAX_PATH] = { 0 };
		if (nf_getProcessName(pConnInfo->processId, szProcessName, MAX_PATH))
		{
			//make lower
			wcslwr(szProcessName);

			//handle TCP requests
			HandleTcpUdpRequests(id, szProcessName);
		}
		pf_getNFEventHandler()->udpCreated(id, pConnInfo);
	}
	catch (...)
	{
		AddLogEntryEx(SECONDLEVEL, L"### Exception in EventHandler::udpCreated");
	}
}

/***************************************************************************************************
*  Function Name  : udpConnectRequest
*  Description    :
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::udpConnectRequest(ENDPOINT_ID id, PNF_UDP_CONN_REQUEST pConnReq)
{
}

/***************************************************************************************************
*  Function Name  : udpClosed
*  Description    :
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::udpClosed(ENDPOINT_ID id, PNF_UDP_CONN_INFO pConnInfo)
{
	//OutputDebugString(L"NF_EventHandler:: udpClosed\n");
	fflush(stdout);
}

/***************************************************************************************************
*  Function Name  : udpReceive
*  Description    :
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::udpReceive(ENDPOINT_ID id, const unsigned char * remoteAddress, const char * buf, int len, PNF_UDP_OPTIONS options)
{
	//OutputDebugString(L"NF_EventHandler:: udpReceive\n");
	// Send the packet to application
	nf_udpPostReceive(id, remoteAddress, buf, len, options);
}

/***************************************************************************************************
*  Function Name  : udpSend
*  Description    :
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::udpSend(ENDPOINT_ID id, const unsigned char * remoteAddress, const char * buf, int len, PNF_UDP_OPTIONS options)
{
	//OutputDebugString(L"NF_EventHandler:: udpSend\n");
	// Send the packet to server
	nf_udpPostSend(id, remoteAddress, buf, len, options);
}

/***************************************************************************************************
*  Function Name  : udpCanReceive
*  Description    :
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::udpCanReceive(ENDPOINT_ID id)
{
	//OutputDebugString(L"NF_EventHandler:: udpCanReceive\n");
	fflush(stdout);
}

/***************************************************************************************************
*  Function Name  : udpCanSend
*  Description    :
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::udpCanSend(ENDPOINT_ID id)
{
	//OutputDebugString(L"NF_EventHandler:: udpCanSend\n");
	fflush(stdout);
}

/***************************************************************************************************
*  Function Name  : InsertApplicationsIntoDB
*  Description    : Functions which maintains application list which are tried to connect Via TCP/UDP
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
bool EventHandler::InsertApplicationsIntoDB(std::string strAppPath)
{
	OutputDebugString(L">>> EventHandler::InsertApplicationsIntoDB");
	OutputDebugStringA(strAppPath.c_str());

	try
	{
		if (CheckApplicationExists(strAppPath))
		{
			return false;
		}

		CString	csWardWizModulePath = GetWardWizPathFromRegistry();
		CString	csAppRulesDB = L"";
		csAppRulesDB.Format(L"%sVBFIREWALL.DB", csWardWizModulePath);

		CT2A dbPath(csAppRulesDB, CP_UTF8);
		CWardWizSQLiteDatabase objSqlDb(dbPath.m_psz);

		//check here for duplicate entries in database

		CStringA csApplicationPath = strAppPath.c_str();
		CStringA csInsertQuery;
		csInsertQuery.Format("SELECT PROGRAMPATH FROM WWIZ_FIREWALL_PACKET_RULE_CONFIG WHERE PROGRAMPATH = '%s'", csApplicationPath.MakeLower());

		objSqlDb.Open();
		CWardwizSQLiteTable qResult = objSqlDb.GetTable(csInsertQuery);
		objSqlDb.Close();

		DWORD dwRows = qResult.GetNumRows();
		if (dwRows != 0)
		{
			return false;
		}

		CStringA csAppName(csApplicationPath.MakeLower());
		csAppName = csAppName.Right(csAppName.GetLength() - csAppName.ReverseFind('\\') - 1);

		csInsertQuery.Format("INSERT INTO WWIZ_FIREWALL_PACKET_RULE_CONFIG (LOCAL_IP, LOCAL_IP_DATA, LOCAL_PORT, LOCAL_PORT_DATA, REMOTE_IP, REMOTE_IP_DATA, REMOTE_PORT, REMOTE_PORT_DATA,	PROGRAMPATH, PROGRAMNAME, PROTOCOL,	ACCESS,	NETWORKTYPE, DIRECTION, SETRULES, RESERVED1, RESERVED2, RESERVED3) VALUES(%d,'%s',%d,'%s',%d,'%s',%d,'%s','%s','%s',%d,%d,'%s',%d, %d,'%s','%s','%s');", 0x00, "", 0x00, "", 0x00, "", 0x00, "", csApplicationPath.MakeLower(), csAppName, 0x00, 0x01, "1", 0x03, 0x00, "", "", "");

		objSqlDb.Open();
		objSqlDb.ExecDML(csInsertQuery);
		objSqlDb.Close();
	}
	catch (...)
	{
		AddLogEntryEx(SECONDLEVEL, L"### Exception in InsertApplicationsIntoDB");
		return false;
	}
	return true;
}

/***************************************************************************************************
*  Function Name  : CheckApplicationExists
*  Description    : Function which keeps track of listed application in memory for better performance
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
bool EventHandler::CheckApplicationExists(std::string strAppPath)
{
	bool bFound = false;
	try
	{
		_wcslwr((wchar_t*)strAppPath.c_str());
		for (std::vector<std::string>::iterator it = m_vAppList.begin(); it != m_vAppList.end(); ++it)
		{
			if (strAppPath == (*it))
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
		{
			m_vAppList.push_back(strAppPath);
		}
	}
	catch (...)
	{
		AddLogEntryEx(SECONDLEVEL, L"### Exception in CheckApplicationExists");
	}
	return bFound;
}

/***************************************************************************************************
*  Function Name  : HandleTcpUdpRequests
*  Description    : Function to handle TCP UDP requests
*  Author Name    : Ram Shelke
*  Date			  : 29 March 2019
****************************************************************************************************/
void EventHandler::HandleTcpUdpRequests(ENDPOINT_ID id, LPTSTR lpszProcessName)
{
	OutputDebugString(L">>> EventHandler::HandleTcpUdpRequests");
	try
	{
		if (!lpszProcessName)
			return;

		CString csProcessName(lpszProcessName);
		int iFound = csProcessName.ReverseFind(L'\\');
		csProcessName = csProcessName.Right(csProcessName.GetLength() - iFound - 1);

		CStringA csProcessNameAnsi;
		csProcessNameAnsi.Format("%S", lpszProcessName);
		InsertApplicationsIntoDB(csProcessNameAnsi.GetBuffer());

		if (m_iSetDefAppBehav == APPBEHAVIOUR_BLOCK)
		{
			nf_tcpClose(id);
			return;
		}

		CString csProcessPath{ lpszProcessName };
		CString csHiddenTxt = L"\\";
		int iPos = csProcessPath.ReverseFind('\\');
		csProcessPath = csProcessPath.Left(iPos + 1);
		CString csWardWizPath = m_csAppPath.MakeLower();
		if (csProcessPath == csWardWizPath)
		{
			return;
		}

		//check here if internet connection need to be off
		if (m_pobjParentalCtrl != NULL)
		{
			if (m_pobjParentalCtrl->m_bBlockInternetUsage || m_pobjParentalCtrl->m_bRestrictInternetUsage)
			{
				if (m_pobjParentalCtrl->CheckIfUserActive())
				{
					OutputDebugString(L"[BLOCKED] CheckIfUserActive \n");
					m_pobjParentalCtrl->SendParCtrlMessage2Tray(SEND_PC_INET_TRAY_LOCK_WND, L"");
					nf_tcpClose(id);
					return;
				}
			}
		}

		//check with application blocked list
		if (m_objFirewallFilter.ISApplicationBlocked(lpszProcessName))
		{
			OutputDebugString(L"[BLOCKED] ISApplicationBlocked \n");
			nf_tcpClose(id);
			return;
		}
	}
	catch (...)
	{
		AddLogEntryEx(SECONDLEVEL, L"### Exception in EventHandler::HandleTcpUdpRequests");
	}
}