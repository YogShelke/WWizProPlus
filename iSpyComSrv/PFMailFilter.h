/************************************************************************************************
*  Program Name	: MailFilter
*  Description	: Incoming and Outgoing mails for POP3 service.
*  Author Name	: Amol J
*  Date Of Creation: 08 Feb 2018
*  Version No	: 3.1.0.0
*************************************************************************************************/

//  The sample adds a prefix to the subjects of incoming messages, and blocks outgoing 
//  messages having the specified prefix in subject.
#include "stdafx.h"
#include <crtdbg.h>
#include "ProtocolFilters.h"
#include "PFEventsDefault.h"
#include "WWizEmailScanner.h"
#include "WWizFirewallFilter.h"
#include "WWizParentalCntrl.h"
#include <fstream>
#include <string>
#include "MimeCode.h"
#include "MimeChar.h"
#include "Mime.h"

using namespace nfapi;
using namespace ProtocolFilters;
using namespace std;

// Change this string after renaming and registering the driver under different name
//#define NFDRIVER_NAME "netfilter2"
#define NFDRIVER_NAME_XP "WRDWIZFLT"
#define NFDRIVER_NAME_07 "WRDWIZFLT07"
#define NFDRIVER_NAME_08 "WRDWIZFLT08"
#define NFDRIVER_NAME_10 "WRDWIZFLT10"
std::string g_mailPrefix;

class MailFilter : public PFEventsDefault
{
private:
	TCHAR				m_szAllUserPath[MAX_PATH];
	CWWizEmailScanner	m_objScanner;
	CString				m_csAppPath;
public:
	CWWizFirewallFilter			m_objFirewallFilter;
	CWWizParentalCntrl			*m_pobjParentalCtrl;
	bool ScanAttachment();
public:
	MailFilter():m_pobjParentalCtrl(NULL)
	{
		GetEnvironmentVariable(L"ALLUSERSPROFILE", m_szAllUserPath, MAX_PATH);
		m_objScanner.LoadRequiredDLLs();
		m_csAppPath = GetWardWizPathFromRegistry();
	}

	void SetParentalControlPtr(CWWizParentalCntrl	*pParentalCtrl)
	{
		m_pobjParentalCtrl = pParentalCtrl;
	}

	virtual void tcpConnected(nfapi::ENDPOINT_ID id, nfapi::PNF_TCP_CONN_INFO pConnInfo)
	{
		if (pConnInfo->direction == NF_D_OUT)
		{
			pf_addFilter(id, FT_PROXY);
			pf_addFilter(id, FT_SSL, FF_SSL_SUPPORT_CLIENT_CERTIFICATES);
			pf_addFilter(id, FT_POP3, FF_SSL_TLS | FF_SSL_SUPPORT_CLIENT_CERTIFICATES);
			pf_addFilter(id, FT_SMTP, FF_SSL_TLS | FF_SSL_SUPPORT_CLIENT_CERTIFICATES);
			pf_addFilter(id, FT_HTTP, FF_HTTP_BLOCK_SPDY);
		}
		else
		{
			pf_addFilter(id, FT_HTTP, FF_HTTP_BLOCK_SPDY);
		}

		//check here if application is blocked for internet usage.
		WCHAR szProcessName[MAX_PATH] = { 0 };
		nf_getProcessName(pConnInfo->processId, szProcessName, MAX_PATH);

		//make lower
		wcslwr(szProcessName);

		CString csProcessPath{ szProcessName };
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
			if (m_pobjParentalCtrl->m_bBlockInternetUsage)
			{
				m_pobjParentalCtrl->SendParCtrlMessage2Tray(SEND_PC_INET_TRAY_LOCK_WND);
				nf_tcpClose(id);
				return;
			}
		}

		if (m_objFirewallFilter.ISApplicationBlocked(szProcessName))
		{
			nf_tcpClose(id);
			return;
		}
	}

	bool CreateDirectoryFocefully(LPTSTR lpszPath)
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

	bool filterOutgoingMail(nfapi::ENDPOINT_ID id, PFObject * object)
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sWWIZEMAILSCAN.DB", csWardWizModulePath);

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
		bool res = false;
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
					_stprintf_s(szTargetFolder, MAX_PATH, L"%s\\WardWiz\\EmailScan\\", m_szAllUserPath);
					if (!PathFileExists(szTargetFolder))
					{
						CreateDirectoryFocefully(szTargetFolder);
					}
					TCHAR szTargetPath[MAX_PATH] = { 0 };
					_stprintf_s(szTargetPath, MAX_PATH, L"%s\\WardWiz\\EmailScan\\%s.tmp", m_szAllUserPath, CString(strName.c_str()));

					pBP->WriteToFile(CW2A(szTargetPath));
					iRet = m_objScanner.ExtractZipForScanning(szTargetPath, (CString)strFrom.c_str());

					if (iRet == 2 || iRet == 3) //qurentine or repair
					{
						iTotalThreatsFound++;
						iTotalThreatsCleaned++;

						CString csInsertQuery = _T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,");
						csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,%d,Datetime('now','localtime'),Datetime('now','localtime'), NULL,NULL, %d, %d, 2, NULL, %d );"), EMAILSCAN, iTotFileScanCount, iTotalThreatsFound, iTotalThreatsCleaned);
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
							pBP->SetText("Attachment removed\r\n");
							pBP->SetName(strName.c_str());
						}
					}

					if (remove(CW2A(szTargetPath)) != 0)
						printf("Unable to removed file.");
					else
						printf("File removed successfully.");
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
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"WRDSETTINGS" + L"\\ProductSettings.ini";
		GetPrivateProfileString(L"WRDSETTINGS", L"EmailSubject", L"", szSubjectValueData, 511, csIniFilePath);
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
				if (pStream)
				{
					const char blockCmd[] = "554 Message blocked!\r\n";
					pStream->write(blockCmd, sizeof(blockCmd) - 1);
					pf_postObject(id, newObj);
					res = true;
				}
				newObj->free();
			}
		}

		return res;
	}

	void filterIncomingMail(nfapi::ENDPOINT_ID id, PFObject * object)
	{
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
					_stprintf_s(szTargetFolder, MAX_PATH, L"%s\\WardWiz\\EmailScan\\", m_szAllUserPath);
					if (!PathFileExists(szTargetFolder))
					{
						CreateDirectoryFocefully(szTargetFolder);
					}
					TCHAR szTargetPath[MAX_PATH] = { 0 };
					_stprintf_s(szTargetPath, MAX_PATH, L"%s\\WardWiz\\EmailScan\\%s.tmp", m_szAllUserPath, CString(strName.c_str()));

					pBP->WriteToFile(CW2A(szTargetPath));
					iRet = m_objScanner.ExtractZipForScanning(szTargetPath, (CString)strFrom.c_str());

					if( iRet == 2 || iRet == 3) //qurentine or repair
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

						CString csInsertQuery = _T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,");
						csInsertQuery.Format(_T("INSERT INTO Wardwiz_ScanSessionDetails VALUES (null,%d,Datetime('now','localtime'),Datetime('now','localtime'), NULL,NULL, %d, %d, 2, NULL, %d );"), EMAILSCAN, iTotFileScanCount, iTotalThreatsFound, iTotalThreatsCleaned);
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
						printf("Unable to removed file.");
					else
						printf("File removed successfully.");
				}
			}
		}

		TCHAR szSubjectValueData[512] = { 0 };
		CString csIniFilePath = GetWardWizPathFromRegistry() + L"WRDSETTINGS" + L"\\ProductSettings.ini";
		GetPrivateProfileString(L"WRDSETTINGS", L"EmailSubject", L"", szSubjectValueData, 511, csIniFilePath);
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

	virtual void dataAvailable(nfapi::ENDPOINT_ID id, PFObject * object)
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
		}
		if (!blocked)
			pf_postObject(id, object);
	}
};

bool MailFilter::ScanAttachment()
{
	try
	{
		CWardWizSQLiteDatabase dbSQlite;
		CString	csWardWizModulePath = GetWardWizPathFromRegistry();

		CString	csWardWizReportsPath = L"";
		csWardWizReportsPath.Format(L"%sWWIZEMAILSCAN.DB", csWardWizModulePath);

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