// AddressBook.cpp : implementation file
//
/*****************************************************************************************************************
*  Program Name: ISpyDataManager.cpp                                                                                                    
*  Description: It manages data to get stored in DB with serialization process.
*  Author Name: Neha Gharge                                                                                                      
*  Date Of Creation: 22 Jan 2014
*  Version No: 1.0.0.2
*******************************************************************************************************************/
#include "stdafx.h"
#include "ISpyDataManager.h"
#include "ISpyList.h"
// CDataManager

IMPLEMENT_SERIAL(CDataManager, CObject, -1)

/**********************************************************************************************************                     
*  Function Name  :	CDataManager                                                     
*  Description    :	C'tor
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0016
**********************************************************************************************************/
CDataManager::CDataManager() 
{
	m_uiFileVersion = 0;
	bWithoutEdit=1;
}

/**********************************************************************************************************                     
*  Function Name  :	~CDataManager                                                     
*  Description    :	D'tor
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0017
**********************************************************************************************************/
CDataManager::~CDataManager()
{
}

/**********************************************************************************************************
*  Function Name  :	EditContact
*  Description    :	Edit each entry into Clist object
*  Author Name    : Neha Gharge, Ram Shelke
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0018
**********************************************************************************************************/
bool CDataManager::EditContact(CString csKey, const CIspyList& contact, bool bAllowDuplicates)
{
	try
	{
		POSITION pos = FindContact(csKey);
		if (pos == NULL)
		{
			return false;
		}

		m_cContactsList.RemoveAt(pos);
		m_cContactsList.AddTail(contact);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataManager::EditContact", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

// CDataManager member functions
/**********************************************************************************************************                     
*  Function Name  :	AddContact                                                     
*  Description    :	Add each entry into Clist object
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0018
**********************************************************************************************************/
bool CDataManager::AddContact(const CIspyList& contact, bool bAllowDuplicates)
{
	try
	{
		if (bAllowDuplicates)
		{
			m_cContactsList.AddTail(contact);
			return true;
		}
		else
		{
			if (!bWithoutEdit)
			{
				if (FindContact(contact.GetFirstEntry(), contact.GetSecondEntry()) == NULL)
				{
					m_cContactsList.AddTail(contact);
					return true;
				}
			}
			else if (bWithoutEdit)
			{
				if (FindContact(contact.GetFirstEntry()) == NULL)
				{
					m_cContactsList.AddTail(contact);
					return true;
				}
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataManager::AddContact", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/**********************************************************************************************************                     
*  Function Name  :	AddRecord                                                     
*  Description    :	Add each record into Clist
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0019
**********************************************************************************************************/
bool CDataManager::AddRecord(const CIspyList& contact)
{
	try
	{
		if (FindRecord(contact.GetFirstEntry(), contact.GetSeventhEntry()) == NULL)
		{
			m_cContactsList.AddTail(contact);
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataManager::AddRecord", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/**********************************************************************************************************                     
*  Function Name  :	RemoveRecord                                                     
*  Description    :	Remove record from Clist Object
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0020
**********************************************************************************************************/
bool CDataManager::RemoveRecord(const CString& FirstEntry, const DWORD dwIndex)
{
	try
	{
		POSITION pos = m_cContactsList.GetHeadPosition();
		while (pos != NULL)
		{
			POSITION former = pos;
			CIspyList contact = m_cContactsList.GetNext(pos);
			if (contact.GetFirstEntry().CompareNoCase(FirstEntry) == 0 &&
				(contact.GetSeventhEntry() == dwIndex))
			{
				m_cContactsList.RemoveAt(former);
				return true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataManager::RemoveRecord", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/**********************************************************************************************************                     
*  Function Name  :	RemoveContact                                                     
*  Description    :	Remove each entry of list from CList object
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0021
**********************************************************************************************************/
bool CDataManager::RemoveContact(const CString& FirstEntry)
{
	try
	{
		POSITION pos = m_cContactsList.GetHeadPosition();
		while (pos != NULL)
		{
			POSITION former = pos;
			CIspyList contact = m_cContactsList.GetNext(pos);
			if (contact.GetFirstEntry().CompareNoCase(FirstEntry) == 0)
			{
				m_cContactsList.RemoveAt(former);
				return true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataManager::RemoveContact", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/**********************************************************************************************************                     
*  Function Name  :	RemoveContact                                                     
*  Description    :	Remove each entry of list from CList object
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0022
**********************************************************************************************************/
bool CDataManager::RemoveContact(const CString& FirstEntry, const CString& SecondEntry)
{
	try
	{
		POSITION pos = m_cContactsList.GetHeadPosition();
		while (pos != NULL)
		{
			POSITION former = pos;
			CIspyList contact = m_cContactsList.GetNext(pos);
			if (contact.GetFirstEntry().CompareNoCase(FirstEntry) == 0 &&
				contact.GetSecondEntry().CompareNoCase(SecondEntry) == 0)
			{
				m_cContactsList.RemoveAt(former);
				return true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataManager::RemoveContact", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/**********************************************************************************************************                     
*  Function Name  :	RemoveContact                                                     
*  Description    :	Remove each entry of list from CList object
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0023
**********************************************************************************************************/
bool CDataManager::RemoveContact(const CString& FirstEntry, const CString& SecondEntry, const CString& ThirdEntry)
{
	try
	{
		POSITION pos = m_cContactsList.GetHeadPosition();
		while (pos != NULL)
		{
			POSITION former = pos;
			CIspyList contact = m_cContactsList.GetNext(pos);
			if (contact.GetFirstEntry().CompareNoCase(FirstEntry) == 0 &&
				contact.GetSecondEntry().CompareNoCase(SecondEntry) == 0 &&
				contact.GetThirdEntry().CompareNoCase(ThirdEntry) == 0)
			{
				m_cContactsList.RemoveAt(former);
				return true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataManager::RemoveContact", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/**********************************************************************************************************                     
*  Function Name  :	FindContact                                                     
*  Description    :	Find contact from existing list
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0024
**********************************************************************************************************/
POSITION CDataManager::FindContact(const CString& FirstEntry,const CString& SecondEntry ) const
{
	try
	{
		POSITION pos = m_cContactsList.GetHeadPosition();
		while (pos != NULL)
		{
			POSITION former = pos;
			CIspyList contact = m_cContactsList.GetNext(pos);

			if (contact.GetFirstEntry().CompareNoCase(FirstEntry) == 0 && contact.GetSecondEntry().CompareNoCase(SecondEntry) == 0)
				return former;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataManager::FindContact", 0, 0, true, SECONDLEVEL);
		return NULL;
	}
	return NULL;
}

/**********************************************************************************************************                     
*  Function Name  :	FindRecord                                                     
*  Description    :	Find a record from existing list
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0025
**********************************************************************************************************/
POSITION CDataManager::FindRecord(const CString& FirstEntry,const DWORD dwIndex ) const
{
	try
	{
		POSITION pos = m_cContactsList.GetHeadPosition();
		while (pos != NULL)
		{
			POSITION former = pos;
			CIspyList contact = m_cContactsList.GetNext(pos);
			if (contact.GetFirstEntry().CompareNoCase(FirstEntry) == 0 && (contact.GetSeventhEntry() == dwIndex))
				return former;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataManager::FindRecord", 0, 0, true, SECONDLEVEL);
		return NULL;
	}
	return NULL;
}

/**********************************************************************************************************                     
*  Function Name  :	FindContact                                                     
*  Description    :	Find contact from existing list
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0026
**********************************************************************************************************/
POSITION CDataManager::FindContact(const CString& FirstEntry) const
{
	try
	{
		POSITION pos = m_cContactsList.GetHeadPosition();
		while (pos != NULL)
		{
			POSITION former = pos;
			CIspyList contact = m_cContactsList.GetNext(pos);

			if (contact.GetFirstEntry().CompareNoCase(FirstEntry) == 0)
				return former;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataManager::FindContact", 0, 0, true, SECONDLEVEL);
		return NULL;
	}
	return NULL;
}

/**********************************************************************************************************                     
*  Function Name  :	FindContact                                                     
*  Description    :	Find contact from existing list
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0027
**********************************************************************************************************/
bool CDataManager::FindContact(const CString& FirstEntry, CIspyList& contact) const
{
	try
	{
		POSITION pos = m_cContactsList.GetHeadPosition();
		while (pos != NULL)
		{
			contact = m_cContactsList.GetNext(pos);
			if (contact.GetFirstEntry().CompareNoCase(FirstEntry) == 0)
			{
				return true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataManager::FindContact", 0, 0, true, SECONDLEVEL);
		return NULL;
	}
	return false;
}

/**********************************************************************************************************                     
*  Function Name  :	Serialize                                                     
*  Description    :	Serialized data into or from Clist object
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0028
**********************************************************************************************************/
void CDataManager::Serialize( CArchive& ar )
{
	try
	{
		ar.SerializeClass(RUNTIME_CLASS(CDataManager));

		// storing
		if (ar.IsStoring())
		{
			ar << m_uiFileVersion;

			// write the number of contacts
			ar << (int)m_cContactsList.GetCount();
			CIspyList::SetCurrentFileVersion(m_uiFileVersion);

			// write all the contacts
			POSITION pos = m_cContactsList.GetHeadPosition();
			while (pos != NULL)
			{
				CIspyList contact = m_cContactsList.GetNext(pos);

				contact.Serialize(ar);
			}
		}
		// loading
		else
		{
			ar >> m_uiFileVersion;

			m_cContactsList.RemoveAll();

			int count = 0;
			ar >> count;

			// read the number of contacts
			for (INT_PTR i = 0; i < count; ++i)
			{
				CIspyList contact;
				contact.Serialize(ar);

				m_cContactsList.AddTail(contact);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataManager::Serialize", 0, 0, true, SECONDLEVEL);
	}
}

/**********************************************************************************************************                     
*  Function Name  :	RemoveAll                                                     
*  Description    :	Remove all contact from list
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0029
**********************************************************************************************************/
void CDataManager::RemoveAll()
{
	m_cContactsList.RemoveAll();
}

/**********************************************************************************************************                     
*  Function Name  :	RemoveReportEntry                                                     
*  Description    :	Remove report entry
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0030
**********************************************************************************************************/
bool CDataManager::RemoveReportEntry(const CString& csDateTime, const CString& csScanType, const CString& csFilePath)
{
	try
	{
		POSITION pos = m_cContactsList.GetHeadPosition();
		while (pos != NULL)
		{
			POSITION former = pos;
			CIspyList contact = m_cContactsList.GetNext(pos);
			if (contact.GetFirstEntry().CompareNoCase(csDateTime) == 0 &&
				contact.GetSecondEntry().CompareNoCase(csScanType) == 0 &&
				contact.GetForthEntry().CompareNoCase(csFilePath) == 0)
			{
				m_cContactsList.RemoveAt(former);
				return true;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CDataManager::RemoveReportEntry", 0, 0, true, SECONDLEVEL);
	}
	return false;
}