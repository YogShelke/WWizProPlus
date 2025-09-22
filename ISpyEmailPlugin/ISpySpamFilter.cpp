/**********************************************************************************************************      		Program Name          : ISpySpamFilter.cpp
	Description           : This class which provides functionality for spam filter functionality.
	Author Name			  : Ramkrushna Shelke                                                                  	  
	Date Of Creation      : 20th Aug 2014
	Version No            : 1.0.0.8
	Special Logic Used    : 
	Modification Log      :           
***********************************************************************************************************/
#include "stdafx.h"
#include "ISpySpamFilter.h"
#include "OutlookAddinApp.h"

/***************************************************************************
  Function Name  : CISpySpamFilter
  Description    : Cont'r
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0077
  Date           : 07th Aug 2014
****************************************************************************/
CISpySpamFilter::CISpySpamFilter(void):
	m_bSpamFilterSettingLoaded(false)
{

}

/***************************************************************************
  Function Name  : ~CISpySpamFilter
  Description    : Dest'r
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0066
  Date           : 07th Aug 2014
****************************************************************************/
CISpySpamFilter::~CISpySpamFilter(void)
{

}

/***************************************************************************
  Function Name  : LoadSendersEmailAddressDB 
  Description    : Function which loads senders adress DB in memory
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0079
  Date           : 07th Aug 2014
****************************************************************************/
bool CISpySpamFilter::LoadSendersEmailAddressDB(CString csPathName)
{
	if(m_bSpamFilterSettingLoaded)
	{
		return true;
	}
	try
	{
		if( !PathFileExists( csPathName ) )
			return false;

		CFile theFile;
		if(!theFile.Open((LPCTSTR)csPathName, CFile::modeRead | CFile::shareDenyWrite))
		{
			return false;
		}

		// Create a loading archive
		CArchive archive(&theFile, CArchive::load);

		int iVersion = m_objSpamFilterdb.GetFileVersion();
		m_objSpamFilterdb.Serialize(archive);

		// Close the loading archive
		archive.Close();
		theFile.Close();

		//Clear the containts here
		m_objSendersAddresses.clear();

		const ContactList& contacts = m_objSpamFilterdb.GetContacts();

		// iterate over all contacts add add them to the list
		int nCurrentItem = 0;
		POSITION pos = contacts.GetHeadPosition();
		while(pos != NULL)
		{
			bool bIsBlock = false;
			const CIspyList contact = contacts.GetNext(pos);

			CString csFirstName = contact.GetFirstEntry();
			CString csIsBlockType = contact.GetSecondEntry();
			if (csIsBlockType == theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"))
			{
				bIsBlock = true;
			}
			m_objSendersAddresses[csFirstName] = bIsBlock;
		}	
		m_bSpamFilterSettingLoaded = true;
	}
	catch (CArchiveException* pEx) 
	{
		pEx->ReportError(); 
		pEx->Delete(); 
	}
	catch (...) 
	{
		AddLogEntry(L"### Exception in CWardwizSpamFilter::LoadSendersEmailAddressDB", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************
  Function Name  : FilterSenderEmailAddress
  Description    : Function which filers Senfers email address with incoming mail.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0079
  Date           : 07th Aug 2014
****************************************************************************/
DWORD CISpySpamFilter::FilterSenderEmailAddress(CString csSenderEmailAddress)
{
	DWORD dwType = NONE;
	try
	{
		typedef std::map<CString, bool>::iterator it_type;
		for (it_type iterator = m_objSendersAddresses.begin(); iterator != m_objSendersAddresses.end(); iterator++)
		{
			if (iterator->first == csSenderEmailAddress)
			{
				dwType = ALLOW;
				if (iterator->second)
				{
					dwType = BLOCK;
				}
				break;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizSpamFilter::FilterSenderEmailAddress", 0, 0, true, SECONDLEVEL);
	}
	return dwType;
}