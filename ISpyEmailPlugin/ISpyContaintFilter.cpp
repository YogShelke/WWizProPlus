/**********************************************************************************************************      	 
	Program Name          : WrdWizContaintFilter.cpp
	Description           : This class functionality is for Extracting the attachments from email
								and Zip/Unzip functionality.
	Author Name			: Ramkrushna Shelke                                                                  	  
	Date Of Creation      : 07th Aug 2014
	Version No            : 1.0.0.5
	Special Logic Used    : 
	Modification Log      :           
***********************************************************************************************************/
#include "stdafx.h"
#include "ISpyContaintFilter.h"
#include "OutlookAddin.h"
#include "OutlookAddinApp.h"

//#include "ISpyOutlookComm.h"

#define WARDWIZSPAMPROP		L"WardWizSpam"
#define WARDWIZBODYPROP		L"WardWizBody"
#define WARDWIZNONBODYPROP	L"WardWizNonBody"
/***************************************************************************
  Function Name  : WrdWizContaintFilter
  Description    : Cont'r
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0044
  Date           : 07th Aug 2014
****************************************************************************/
CWrdWizContaintFilter::CWrdWizContaintFilter(void):
	m_bMaliciousDBFileLoaded(false),
	m_bContentDBFileLoaded(false)
{
}

/***************************************************************************
  Function Name  : ~WrdWizContaintFilter
  Description    : Dest'r
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0045
  Date           : 07th Aug 2014
****************************************************************************/
CWrdWizContaintFilter::~CWrdWizContaintFilter(void)
{
}

/***************************************************************************
  Function Name  : FilterAttachment
  Description    : Function which filters the attachment's in mail.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0046
  Date           : 07th Aug 2014
****************************************************************************/
DWORD CWrdWizContaintFilter::FilterAttachment(CComQIPtr<Outlook::_MailItem> pMail)
{
	DWORD dwReturn = NONE;
	try
	{
		

		bool bIsMatched = false;

		CStringArray objarFileNames;
		CString csExtFileName;
		CString csWholeExt;
		int idotPos = 0;
		CISpyOutlookComm objISpyOutlookComm;

		AddLogEntry(L">>> CWardwizContaintFilter::Filter Attachment", 0, 0, true, FIRSTLEVEL);
		objISpyOutlookComm.GetMailAttchments(pMail, objarFileNames);

		int iCount = static_cast<int>(m_arobjAttchBlockExtention.GetCount());

		int AttachmentCount = static_cast<int>(objarFileNames.GetCount());

		if (!(objarFileNames.GetCount()))
		{
			return dwReturn;
		}

		//int attachmentCount = objarFileNames.GetCount();

		for (int iIndex = 0; iIndex < iCount; iIndex++)
		{
			for (int jIndex = 0; jIndex < AttachmentCount; jIndex++)
			{
				//objISpyOutlookComm.GetMailAttchments(pMail, objarFileNames);
				csWholeExt = m_arobjAttchBlockExtention.GetAt(iIndex);
				csExtFileName = objarFileNames[jIndex];
				idotPos = csExtFileName.ReverseFind(_T('.'));
				csExtFileName = csExtFileName.Right(csExtFileName.GetLength() - idotPos);
				csExtFileName.MakeLower();
				if (!(csExtFileName.Compare(csWholeExt)))
				{
					AddLogEntry(L">>> CWardwizContaintFilter::Attachment is matched with input parameter", 0, 0, true, FIRSTLEVEL);
					bIsMatched = true;
					break;
				}
				else
				{
					AddLogEntry(L">>> CWardwizContaintFilter::Attachment is not matched with input parameter", 0, 0, true, FIRSTLEVEL);
					bIsMatched = false;
					continue;
				}

			}
			if (bIsMatched)
			{
				break;
			}

		}
		//matched the attchment extension for block
		if (bIsMatched)
		{
			return BLOCK;
		}

		iCount = static_cast<int>(m_arobjAttchAllowExtention.GetCount());
		for (int iIndex = 0; iIndex < iCount; iIndex++)
		{
			for (int jIndex = 0; jIndex < AttachmentCount; jIndex++)
			{
				csWholeExt = m_arobjAttchAllowExtention.GetAt(iIndex);
				csExtFileName = objarFileNames[jIndex];
				idotPos = csExtFileName.ReverseFind(_T('.'));
				csExtFileName = csExtFileName.Right(csExtFileName.GetLength() - idotPos);
				csExtFileName.MakeLower();
				if (!(csExtFileName.Compare(csWholeExt)))
				{
					AddLogEntry(L">>> CWardwizContaintFilter::Attachment is matched with input parameter", 0, 0, true, FIRSTLEVEL);
					bIsMatched = true;
					break;
				}
				else
				{
					AddLogEntry(L">>> CWardwizContaintFilter::Attachment is not matched with input parameter", 0, 0, true, FIRSTLEVEL);
					bIsMatched = false;
					continue;
				}

			}
			if (bIsMatched)
			{
				break;
			}

		}
		//matched the attchment extension for allow
		if (bIsMatched)
		{
			dwReturn = ALLOW;
		}
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CWardwizContaintFilter::FilterAttachment", 0, 0, true, SECONDLEVEL);
	}
	return dwReturn;
}

/***************************************************************************
  Function Name  : FilterSubject
  Description    : Function which filters mail item subject.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0047
  Date           : 07th Aug 2014
  Issue  no : 1261 Whole single word in subject line.
****************************************************************************/
DWORD CWrdWizContaintFilter::FilterSubject(CComQIPtr<Outlook::_MailItem> pMail)
{
	DWORD dwReturn = NONE;
	
	try
	{
		bool bIsStringMatched = false;

		int j		=	0;
		int k		=	0;
		int flag	=	0;
		int iIndex	=	0;
		int iPos    =   0;
		CISpyOutlookComm objISpyOutlookComm;
		CString msgbdy = objISpyOutlookComm.GetMailSubject(pMail);
		CString filt;
		CStringArray csSubjectBodyArrayContents;
		CString resToken;
		bool bIsNotWordOne = false;
		resToken = msgbdy.Tokenize(_T(" "), iPos);
		while (resToken != _T(""))
		{
			resToken.MakeLower();
			resToken.Remove(' ');
			csSubjectBodyArrayContents.Add(resToken);
			resToken = msgbdy.Tokenize(_T(" "), iPos);
		}
		iPos = 0;
		int iSubjectWordCount = static_cast<int>(csSubjectBodyArrayContents.GetCount());
		if (iSubjectWordCount > 1)
		{
			bIsNotWordOne = true;
		}
		msgbdy.MakeLower();
		msgbdy.Remove(' ');
		msgbdy.Remove('\n');
		int iCount = static_cast<int>(m_arobjBlockSubjectContains.GetCount());
		int msglen = msgbdy.GetLength();
		for (iIndex = 0;iIndex < iCount; iIndex++) 
		{
			filt = m_arobjBlockSubjectContains.GetAt(iIndex);
			filt.MakeLower();
			filt.Remove(' ');
			int len =filt.GetLength();
			for(j=0;j<msglen;j++)
			{
				if(filt[k]==msgbdy[j])
				{
					flag=1;
					k++;
					if (!bIsNotWordOne)
					{
						if ((k == len) && (len == msglen))
							goto Flag;
					}
					else
					{
						if (k == len)
							goto Flag;
					}
				}
				else
				{
					flag=0;
					k=0;
				}
			}
		}
Flag:
		if(flag == 1)
		{
			bIsStringMatched = true;
		}
		if(bIsStringMatched)
		{
			AddLogEntry(L">>> CWardwizContaintFilter::Subject is  matched with input parameter", 0, 0, true, FIRSTLEVEL);
			if(m_ContainFilterSubject[iIndex].byIsAllow == L'1')
			{
				return ALLOW;
			}
			else 
			{
				return BLOCK;
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizContaintFilter::FilterSubject", 0, 0, true, SECONDLEVEL);
	}
	return NONE;
}

/***************************************************************************
  Function Name  : FilterMessageBody
  Description    : Function which filters mail body.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0048
  Date           : 07th Aug 2014
  Issue  no : 1261 Whole single word in body line.
****************************************************************************/
DWORD CWrdWizContaintFilter::FilterMessageBody(CComQIPtr<Outlook::_MailItem> pMail)
{
	DWORD dwReturn = NONE;

	try
	{
		bool bIsStringMatched = false;

		//Took a plain body at one time and store in member variable.
		//Avoid multiple getmailbody call
		//Neha Gharge 18th July,2015
		//CISpyOutlookComm objISpyOutlookComm;
		int j		=	0;
		int k		=	0;
		int flag	=	0;
		int iIndex	=	0;
		int iPos	=	0;

		CString msgbdy = theApp.m_objISpyOutlookComm.m_bstrMailBody;
		AddLogEntry(L"The mail body in plain format %s", msgbdy, 0, true, ZEROLEVEL);
		CString filt;
		CStringArray csSubjectBodyArrayContents;
		CString resToken;
		bool bIsNotWordOne = false;
		resToken = msgbdy.Tokenize(_T(" "), iPos);
		while (resToken != _T(""))
		{
			resToken.MakeLower();
			resToken.Remove(' ');
			csSubjectBodyArrayContents.Add(resToken);
			resToken = msgbdy.Tokenize(_T(" "), iPos);
		}
		iPos = 0;
		int iSubjectWordCount = static_cast<int>(csSubjectBodyArrayContents.GetCount());
		if (iSubjectWordCount > 1)
		{
			bIsNotWordOne = true;
		}

		msgbdy.MakeLower();
		msgbdy.Remove(' ');
		msgbdy.Remove('\n');
		int iCount = static_cast<int>(m_arobjBlockMessageContains.GetCount());
		int msglen = msgbdy.GetLength();
		for (iIndex = 0;iIndex < iCount; iIndex++) 
		{
			filt = m_arobjBlockMessageContains.GetAt(iIndex);
			filt.MakeLower();
			filt.Remove(' ');
			int len =filt.GetLength();
			for(j=0;j<msglen;j++)
			{
				if(filt[k]==msgbdy[j])
				{
					flag=1;
					k++;
					if (!bIsNotWordOne)
					{
						if ((k == len) && (len == msglen))
							goto Flag;
					}
					else
					{
						if (k == len)
							goto Flag;
					}
				}
				else
				{
					flag=0;
					k=0;
				}
			}
		}
Flag:
		if(flag == 1)
		{
			bIsStringMatched = true;
		}

		if(bIsStringMatched)
		{
			AddLogEntry(L">>> CWardwizContentFilter::Messagebody is  matched with input parameter", 0, 0, true, FIRSTLEVEL);
			if(m_ContainFilterMainBody[iIndex].byIsAllow == L'1')
			{
				dwReturn = ALLOW;
			}
			else 
			{
				dwReturn = BLOCK;
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizContentFilter::FilterSubject", 0, 0, true, SECONDLEVEL);
	}
	return dwReturn;
}

/***************************************************************************
  Function Name  : LoadUserMessageContainEntry
  Description    : Function which load settings saved by user in CONTENTFILTER.DB
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0049
  Date           : 07th Aug 2014
****************************************************************************/
void CWrdWizContaintFilter::LoadUserMessageContainEntry()
{
	try
	{
		CComQIPtr<Outlook::_MailItem> pMail;
		//CISpyOutlookComm objISpyOutlookComm;
		//Took a plain body at one time and store in member variable.
		//Avoid multiple getmailbody call
		//Neha Gharge 18th July,2015
		CString csMailBody = theApp.m_objISpyOutlookComm.m_bstrMailBody;
		AddLogEntry(L"The mail body in plain format %s", csMailBody, 0, true, ZEROLEVEL);
		CString csWholeSentance;
		CString csTempWord, csTempMailBody;
		std::wstring wstrMailbody, wstrMailWord;
		std::wstring wstrTempWord;

		int iPos = 0;
		int RetSpacePos = 0;
		int iNoofString = 0;
		int iEntryIndex = 0;
		int iCount = static_cast<int>(m_arobjBlockMessageContains.GetCount());

		AddLogEntry(L">>> CWardwizContentFilter::Load user message content entry", 0, 0, true, FIRSTLEVEL);

		//clear the map before filling
		m_ContainFilterMainBody.clear();

		WordArray wStrArray = { 0 };
		wStrArray.byIsAllow = L'0';
		for (int iIndex = 0; iIndex < m_arobjBlockMessageContains.GetCount(); iIndex++)
		{
			iNoofString = 0;

			csWholeSentance = m_arobjBlockMessageContains.GetAt(iIndex);
			for (int jIndex = 0; (csWholeSentance.GetLength() != 0); jIndex++)
			{

				csTempWord = csWholeSentance.Tokenize(_T(" "), iPos);
				wstrTempWord = csTempWord;

				wStrArray.wStrWord[jIndex].assign(wstrTempWord);
				iPos = 0;
				RetSpacePos = csWholeSentance.Find(_T(' '));
				iNoofString++;
				if (RetSpacePos == -1)
				{
					wStrArray.iNoofString = iNoofString;
					break;
				}
				csTempWord = L" ";
				csTempWord = csWholeSentance.Right(csWholeSentance.GetLength() - (RetSpacePos + 1));
				csWholeSentance = csTempWord;

			}
			DWORD dwIndex = static_cast<DWORD>(iEntryIndex++);
			m_ContainFilterMainBody.insert(std::make_pair(dwIndex, wStrArray));

		}

		iPos = 0;
		RetSpacePos = 0;
		iNoofString = 0;
		iCount = static_cast<int>(m_arobjAllowMessageContains.GetCount());

		WordArray wStrAllowWords = { 0 };
		wStrAllowWords.byIsAllow = L'1';
		for (int iIndex = 0; iIndex < m_arobjAllowMessageContains.GetCount(); iIndex++)
		{
			iNoofString = 0;

			csWholeSentance = m_arobjAllowMessageContains.GetAt(iIndex);

			for (int jIndex = 0; (csWholeSentance.GetLength() != 0); jIndex++)
			{

				csTempWord = csWholeSentance.Tokenize(_T(" "), iPos);
				wstrTempWord = csTempWord;

				wStrAllowWords.wStrWord[jIndex].assign(wstrTempWord);
				iPos = 0;
				RetSpacePos = csWholeSentance.Find(_T(' '));
				iNoofString++;
				if (RetSpacePos == -1)
				{
					wStrAllowWords.iNoofString = iNoofString;
					break;
				}
				csTempWord = L" ";
				csTempWord = csWholeSentance.Right(csWholeSentance.GetLength() - (RetSpacePos + 1));
				csWholeSentance = csTempWord;

			}
			DWORD dwIndex = static_cast<DWORD>(iEntryIndex++);
			m_ContainFilterMainBody.insert(std::make_pair(dwIndex, wStrAllowWords));
		}
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CWardwizContentFilter::LoadUserMessageContentEntry", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : LoadUserSubjectContainEntry
  Description    : Function which loads the settings saved by user in 
				   m_arobjBlockSubjectContains
  Author Name    : Neha Gharge
  SR.NO			 : EMAILSCANPLUGIN_0050
  Date           : 07th Aug 2014
****************************************************************************/
void CWrdWizContaintFilter::LoadUserSubjectContainEntry()
{
	try
	{
		CComQIPtr<Outlook::_MailItem> pMail;
		CISpyOutlookComm objISpyOutlookComm;
		CString csMailBody = objISpyOutlookComm.GetMailSubject(pMail);

		CString csWholeSentance;
		CString csTempWord, csTempMailBody;

		std::wstring wstrMailbody, wstrMailWord;
		std::wstring wstrTempWord;

		int iPos = 0;
		int RetSpacePos = 0;
		int iNoofString = 0;
		int iEntryCount = 0;

		//remove all previous loaded entries
		m_ContainFilterSubject.clear();
		AddLogEntry(L">>> CWardwizContentFilter::Load user subject content entry", 0, 0, true, FIRSTLEVEL);
		int iCount = static_cast<int>(m_arobjBlockSubjectContains.GetCount());

		WordArray wStrArray = { 0 };
		wStrArray.byIsAllow = L'0';
		for (int iIndex = 0; iIndex < iCount; iIndex++)
		{
			iNoofString = 0;

			csWholeSentance = m_arobjBlockSubjectContains.GetAt(iIndex);

			for (int jIndex = 0; (csWholeSentance.GetLength() != 0); jIndex++)
			{

				csTempWord = csWholeSentance.Tokenize(_T(" "), iPos);
				wstrTempWord = csTempWord;
				//wStrArray.wStrWord[jIndex]=wstrTempWord;
				wStrArray.wStrWord[jIndex].assign(wstrTempWord);
				iPos = 0;
				RetSpacePos = csWholeSentance.Find(_T(' '));
				iNoofString++;
				if (RetSpacePos == -1)
				{
					wStrArray.iNoofString = iNoofString;
					break;
				}
				csTempWord = L" ";
				csTempWord = csWholeSentance.Right(csWholeSentance.GetLength() - (RetSpacePos + 1));
				csWholeSentance = csTempWord;

			}
			DWORD dwIndex = static_cast<DWORD>(iEntryCount++);
			m_ContainFilterSubject.insert(std::make_pair(dwIndex, wStrArray));
		}

		iPos = 0;
		RetSpacePos = 0;
		iNoofString = 0;

		WordArray wStrAllowWords = { 0 };
		wStrAllowWords.byIsAllow = L'1';

		iCount = static_cast<int>(m_arobjAllowSubjectContains.GetCount());
		for (int iIndex = 0; iIndex < iCount; iIndex++)
		{
			iNoofString = 0;
			csWholeSentance = m_arobjAllowSubjectContains.GetAt(iIndex);

			for (int jIndex = 0; csWholeSentance.GetLength() != 0; jIndex++)
			{
				csTempWord = csWholeSentance.Tokenize(_T(" "), iPos);
				wstrTempWord = csTempWord;

				wStrAllowWords.wStrWord[jIndex].assign(wstrTempWord);

				iPos = 0;
				RetSpacePos = csWholeSentance.Find(_T(' '));

				iNoofString++;

				if (RetSpacePos == -1)
				{
					wStrAllowWords.iNoofString = iNoofString;
					break;
				}

				csTempWord = L" ";
				csTempWord = csWholeSentance.Right(csWholeSentance.GetLength() - (RetSpacePos + 1));
				csWholeSentance = csTempWord;
			}

			DWORD dwIndex = static_cast<DWORD>(iEntryCount++);
			m_ContainFilterSubject.insert(std::make_pair(dwIndex, wStrAllowWords));
		}
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CWardwizContentFilter::LoadUserSubjectContentEntry", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : FilterMessageBodyWithMaliciousSite
  Description    : Function which filters message body with malicious 
				   sites database.
  Author Name    : Neha Gharge
  SR. NO		 : 
  Date           : 07th Aug 2014
****************************************************************************/
bool CWrdWizContaintFilter::FilterMessageBodyWithMaliciousSite(CComQIPtr<Outlook::_MailItem> pMail)
{
	bool bReturn = false;
	try
	{
		//CISpyOutlookComm objISpyOutlookComm;
		//Took a plain body at one time and store in member variable.
		//Avoid multiple getmailbody call
		//Neha Gharge 18th July,2015
		std::wstring wStrMaliciousSubject, wstrmaliciousMailbody;
		std::wstring wStrMaliciousSite;
		CString csMailBody = theApp.m_objISpyOutlookComm.m_bstrMailBody;
		AddLogEntry(L"The mail body in plain format %s", csMailBody, 0, true, ZEROLEVEL);
		CString csSubject = theApp.m_objISpyOutlookComm.GetMailSubject(pMail);
		CString csMaliciousSite;
		int posMailBody = 0;
		int possubject = 0;
		AddLogEntry(L">>> CWardwizContentFilter::Filteration of malicious site from mail", 0, 0, true, FIRSTLEVEL);
		for (int iIndex = 0; iIndex < m_arobjMaliciousSites.GetCount(); iIndex++)
		{
			possubject = 0;
			posMailBody = 0;
			csMaliciousSite = m_arobjMaliciousSites.GetAt(iIndex);
			if (!(csMaliciousSite.GetLength()))
			{
				break;
			}
			wStrMaliciousSite.assign(csMaliciousSite);
			wStrMaliciousSubject.assign(csSubject);
			possubject = static_cast<int>(wStrMaliciousSubject.find(wStrMaliciousSite));
			wstrmaliciousMailbody.assign(csMailBody);
			if (possubject != -1)
			{
				bReturn = true;
				break;
			}
			posMailBody = static_cast<int>(wstrmaliciousMailbody.find(wStrMaliciousSite));
			if (posMailBody != -1)
			{
				bReturn = true;
				break;
			}

		}
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CWardwizContentFilter::FilterMessageBodyWithMaliciousSite", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
  Function Name  : LoadDataContentFromFile
  Description    : Function which loads the database files using serialization
				   and fill out following objects.
				   m_arobjAttchAllowExtention,
				   m_arobjAllowSubjectContains,
				   m_arobjAllowMessageContains
  Author Name    : Neha Gharge
  SR.NO			 : EMAILSCANPLUGIN_0052
  Date           : 07th Aug 2014
****************************************************************************/
bool CWrdWizContaintFilter::LoadDataContentFromFile(CString csPathName)
{
	//Issue fix : 1352 Issue with email scan 
	//REsolved by: Nitin K. Date: 8th March 2016
	if(m_bContentDBFileLoaded)
	{
		return true;
	}
	try
	{
		AddLogEntry(L">>> CWardwizContentFilter::Loading ContentFilter DB data", 0, 0, true, FIRSTLEVEL);
		if( !PathFileExists( csPathName ) )
			return false;

		CFile theFile;
		if(!theFile.Open((LPCTSTR)csPathName, CFile::modeRead | CFile::shareDenyWrite))
		{
			return false;
		}

		// Create a loading archive
		CArchive archive(&theFile, CArchive::load);

		int iVersion = m_objContentFilterdb.GetFileVersion();
		m_objContentFilterdb.Serialize(archive);

		// Close the loading archive
		archive.Close();
		theFile.Close();
		
		m_arobjAttchAllowExtention.RemoveAll();
		m_arobjAttchBlockExtention.RemoveAll();

		m_arobjAllowSubjectContains.RemoveAll();
		m_arobjBlockSubjectContains.RemoveAll();

		m_arobjAllowMessageContains.RemoveAll();
		m_arobjBlockMessageContains.RemoveAll();

		const ContactList& contacts = m_objContentFilterdb.GetContacts();

		// iterate over all contacts add add them to the list
		int nCurrentItem = 0;
		POSITION pos = contacts.GetHeadPosition();
		while(pos != NULL)
		{
			const CIspyList contact = contacts.GetNext(pos);
			CString csOption = contact.GetThirdEntry();
			CString csEntry = contact.GetSecondEntry();
			CString csMessage = contact.GetFirstEntry();
			
			if (csOption == theApp.m_objwardwizLangManager.GetString(L"IDS_ATTACHMENT_EXT_IS_CONTENT"))
			{
				if (csEntry == theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"))
				{									
					m_arobjAttchBlockExtention.Add(csMessage);
				}
				else
				{
					m_arobjAttchAllowExtention.Add(csMessage);
				}
			}
			else if(csOption == theApp.m_objwardwizLangManager.GetString(L"IDS_MESSAGE_CONTAIN_CONTENT"))
			{
				if (csEntry == theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"))
				{									
					m_arobjBlockMessageContains.Add(csMessage);
				}
				else
				{
					m_arobjAllowMessageContains.Add(csMessage);
				}
			}
			else if (csOption == theApp.m_objwardwizLangManager.GetString(L"IDS_SUBJECT_CONTAIN_CONTECT"))
			{
				if (csEntry == theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"))
				{									
					m_arobjBlockSubjectContains.Add(csMessage);
				}
				else
				{
					m_arobjAllowSubjectContains.Add(csMessage);
				}
			}
		}	
		LoadUserMessageContainEntry();
		LoadUserSubjectContainEntry();
	}
	catch (...) 
	{
		AddLogEntry(L"Exception in CWardwizContentFilter::LoadDataContentFromFile", 0, 0, true, SECONDLEVEL);
		return false;
	}
	//Issue fix : 1352 Issue with email scan 
	//REsolved by: Nitin K. Date: 8th March 2016
	m_bContentDBFileLoaded = true;
	return true;
}

/***************************************************************************
  Function Name  : LoadMaliciousSiteFromFile
  Description    : Function which loads malicious sites from file VBMALICIOUSSITES.DB
  Author Name    : Neha Gharge
  SR.NO			 : EMAILSCANPLUGIN_0053
  Date           : 07th Aug 2014
****************************************************************************/
bool CWrdWizContaintFilter::LoadMaliciousSiteFromFile(CString csPathName)
{
	CString csIndex;
	CString csSiteName;
	DWORD dwIndex;
	if(m_bMaliciousDBFileLoaded)
	{
		return true;
	}
	try
	{
		AddLogEntry(L">>> CWardwizContentFilter::Loading Malicious site DB data", 0, 0, true, FIRSTLEVEL);
		if( !PathFileExists( csPathName ) )
			return false;

		CFile theFile;
		if(!theFile.Open((LPCTSTR)csPathName, CFile::modeRead | CFile::shareDenyWrite))
		{
			return false;
		}

		// Create a loading archive
		CArchive archive(&theFile, CArchive::load);

		int iVersionMalicious=m_objMaliciousSitedb.GetFileVersion();
		m_objMaliciousSitedb.Serialize(archive);

		// Close the loading archive
		archive.Close();
		theFile.Close();
		m_arobjMaliciousSites.RemoveAll();	

		const ContactList& contacts = m_objMaliciousSitedb.GetContacts();

		// iterate over all contacts add add them to the list
		int nCurrentItem = 0;
		POSITION pos = contacts.GetHeadPosition();
		while(pos != NULL)
		{
			const CIspyList contact = contacts.GetNext(pos);
			csSiteName = contact.GetFirstEntry();
			dwIndex = contact.GetSeventhEntry();
			csIndex.Format(L"%d",dwIndex);
			m_arobjMaliciousSites.Add(csSiteName);
		}	
			
	}
	catch (...) 
	{
		AddLogEntry(L"Exception in CWardwizContentFilter::LoadMaliciousSiteFromFile", 0, 0, true, SECONDLEVEL);
		return false;
	}
	//Issue fix : 1352 Issue with email scan 
	//REsolved by: Nitin K. Date: 8th March 2016
	m_bMaliciousDBFileLoaded = true;
	return true;
}

/***************************************************************************
  Function Name  : ScanMail4ContentScan
  Description    : Function which filters the attachment, Subject, Message Body,
				   malicious site and returns a DWORD.
				   ALLOW/BLOCK.
  Author Name    : Ram Shelke
 SR.NO			 : EMAILSCANPLUGIN_0054
 Date            : 11th Aug 2014
****************************************************************************/
DWORD CWrdWizContaintFilter::ScanMail4ContentScan(CComQIPtr<Outlook::_MailItem> pMail)
{
	DWORD dwReturn = NONE;
	try
	{
		AddLogEntry(L">>> CWardwizContentFilter::ScanMail for Content Scan", 0, 0, true, FIRSTLEVEL);

		CString csSubject = theApp.m_objISpyOutlookComm.GetMailSubject(pMail);

		if (!theApp.m_objISpyOutlookComm.ISMailProceedAlready(pMail, WARDWIZNONBODYPROP))
		{
			OUTPUT_DEBUG_STRING(L">>> Mail is not proceed by WARDWIZNONBODYPROP");
			AddLogEntry(L">>> Mail is not proceed by VibraniumNONBODYPROP", 0, 0, true, ZEROLEVEL);

			//Issue:1347 : When we get spam mail in the outlook,that entry is reflecting in Virus Scan UI.
			//Resolved by Nitin K. Date: 5th March 2016
			//Code commented here coz we need to check each and every mail and not just for content filter
			////Filter Sender address domain if it is present in our malicious domain list.
			////Neha Gharge 26th June,2015
			//dwReturn = FilterSenderAddrBlackDomain(pMail);
			////Issue: 0000693 Issue with content filter in email scan.
			////Resolved by : Nitin K Date: 10th July 2015
			//if (dwReturn == BLOCK)
			//{
			//	AddLogEntry(L">>> FilterSenderAddrBlackDomain ALLOW BLOCK", 0, 0, true, FIRSTLEVEL);
			//	if (dwReturn == BLOCK)
			//	{
			//		AddLogEntry(L"### [Spam Mail]:ProcessMailForBlackDomain for sender's address domain  Subject: %s", csSubject, 0, true, FIRSTLEVEL);
			//	}
			//	
			//	if (!theApp.m_objISpyOutlookComm.PutWardWizPropertyIntoMail(pMail, WARDWIZBODYPROP))
			//	{
			//		AddLogEntry(L"### Failed to add WardWizSpam property in mail", 0, 0, true, ZEROLEVEL);
			//	}
			//	return dwReturn;
			//}

			dwReturn = FilterSubject(pMail);
			if (dwReturn == ALLOW || dwReturn == BLOCK)
			{
				AddLogEntry(L">>> FilterSubject ALLOW BLOCK", 0, 0, true, FIRSTLEVEL);
				if (dwReturn == BLOCK)
				{
					AddLogEntry(L"### [Spam Mail]:ProcessMailForSubject Subject: %s", csSubject, 0, true, FIRSTLEVEL);
				}

				// Issue 1235. Even subject is blocked. The entry is coming in virus scan. 
				if (!theApp.m_objISpyOutlookComm.PutWardWizPropertyIntoMail(pMail, WARDWIZBODYPROP))
				{
					AddLogEntry(L"### Failed to add WardwizSpam property in mail", 0, 0, true, ZEROLEVEL);
				}
				return dwReturn;
			}

			if (!theApp.m_objISpyOutlookComm.PutWardWizPropertyIntoMail(pMail, WARDWIZNONBODYPROP))
			{
				AddLogEntry(L"### Failed to add WardwizSpam property in mail", 0, 0, true, ZEROLEVEL);
			}
		}

		if (!theApp.m_objISpyOutlookComm.ISMailProceedAlready(pMail, WARDWIZBODYPROP))
		{
			dwReturn = FilterMessageBody(pMail);
			if (dwReturn == ALLOW || dwReturn == BLOCK)
			{
				AddLogEntry(L">>> FilterMessageBody ALLOW BLOCK", 0, 0, true, FIRSTLEVEL);
				if (dwReturn == BLOCK)
				{
					AddLogEntry(L"### [Spam Mail]:ProcessMailForMailBody Subject: %s", csSubject, 0, true, FIRSTLEVEL);
				}
				if (!theApp.m_objISpyOutlookComm.PutWardWizPropertyIntoMail(pMail, WARDWIZBODYPROP))
				{
					AddLogEntry(L"### Failed to add WardwizSpam property in mail", 0, 0, true, ZEROLEVEL);
				}
				return dwReturn;
			}

			dwReturn = FilterAttachment(pMail);
			if (dwReturn == ALLOW || dwReturn == BLOCK)
			{
				AddLogEntry(L">>> FilterAttachment ALLOW BLOCK", 0, 0, true, FIRSTLEVEL);
				if (dwReturn == BLOCK)
				{
					AddLogEntry(L"### [Spam Mail]:ProcessMailForAttachment Subject: %s", csSubject, 0, true, FIRSTLEVEL);
				}

				if (!theApp.m_objISpyOutlookComm.PutWardWizPropertyIntoMail(pMail, WARDWIZBODYPROP))
				{
					AddLogEntry(L"### Failed to add WardwizSpam property in mail", 0, 0, true, ZEROLEVEL);
				}
				return dwReturn;
			}
		}

		//if (FilterMessageBodyWithMaliciousSite(pMail))
		//{
		//	AddLogEntry(L">>> FilterMessageBodyWithMaliciousSite ALLOW BLOCK", 0, 0, true, FIRSTLEVEL);
		//	if (dwReturn == BLOCK)
		//	{
		//		AddLogEntry(L"### [Spam Mail]:ProcessMailForMaliciousSite in Subject or Mailbody Subject: %s", csSubject, 0, true, FIRSTLEVEL);
		//	}
		//	return BLOCK;
		//}
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CWardwizContentFilter::ScanMail4ContentScan", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return dwReturn;
}

/***************************************************************************
Function Name  : FilterSenderAddrBlackDomain
Description    : Filter Sender address domain if it is present in our malicious domain list.
Author Name    : Neha Gharge
SR.NO			 : 
Date           : 26th June 2015
****************************************************************************/
DWORD CWrdWizContaintFilter::FilterSenderAddrBlackDomain(CComQIPtr<Outlook::_MailItem> pMail)
{
	DWORD dwType = NONE;
	try
	{
		CString csSendersEmailAddr(L"");
		CString csTructSenderDomain(L"");
		CString csBlackDomainName(L"");
		int iPosOfAtSign = 0;

		CISpyOutlookComm objISpyOutlookComm;
		csSendersEmailAddr = objISpyOutlookComm.GetSenderEmailAddress(pMail);

		iPosOfAtSign = csSendersEmailAddr.ReverseFind(_T('@'));
		csTructSenderDomain = csSendersEmailAddr.Right(csSendersEmailAddr.GetLength() - (iPosOfAtSign + 1));


		for (int iIndex = 0; iIndex < m_arobjMaliciousSites.GetCount(); iIndex++)
		{
			csBlackDomainName = m_arobjMaliciousSites[iIndex];
			if (csBlackDomainName == csTructSenderDomain)
			{
				dwType = BLOCK;
				break;
			}
			//Issue: 0000693Issue with content filter in email scan.
			//Resolved by : Nitin K Date: 10th July 2015
			/*else
			{
				dwType = ALLOW;
			}*/
		}
	}
	catch (...)
	{
		AddLogEntry(L"Exception in CWardwizContentFilter::FilterSenderAddrBlackDomain", 0, 0, true, SECONDLEVEL);
	}
	return dwType;
}