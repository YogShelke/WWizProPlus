// Contact.cpp : implementation file
//
/*****************************************************************************************************************
*  Program Name: ISpyList.cpp                                                                                                    
*  Description: It manages data to get stored in DB with serialization process.
*  Author Name: Neha Gharge                                                                                                      
*  Date Of Creation: 22 Jan 2014
*  Version No: 1.0.0.2
*******************************************************************************************************************/
#include "stdafx.h"
#include "ISpyList.h"


// CmyAddressBook

IMPLEMENT_SERIAL( CIspyList, CObject, VERSIONABLE_SCHEMA | 2);

int CIspyList::CURRENT_VERSION = 2;

/**********************************************************************************************************                     
*  Function Name  :	CIspyList                                                     
*  Description    :	C'tor
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0031
**********************************************************************************************************/
CIspyList::CIspyList() :
		m_strFirstEntry(_T("")),
		m_strSecondEntry(_T("")),
		m_strThirdEntry(_T("")),
		m_strForthEntry(_T("")),
		m_strFifthEntry(_T("")),
		m_strSixthEntry(_T("")),
		m_dwSeventhEntry(0)
{
}

CIspyList::CIspyList(const CString& strFirstEntry, const CString& strSecondEntry, const CString& strThirdEntry,	
				   const CString& strForthEntry, const CString& strFifthEntry, const CString& strSixthEntry,const DWORD dwSeventhEntry) :
			m_strFirstEntry(strFirstEntry),
			m_strSecondEntry(strSecondEntry),
			m_strThirdEntry(strThirdEntry),
			m_strForthEntry(strForthEntry),
			m_strFifthEntry(strFifthEntry),
			m_strSixthEntry(strSixthEntry),
			m_dwSeventhEntry(dwSeventhEntry)
{
}

CIspyList::CIspyList(const CIspyList& rhs):
			m_strFirstEntry(rhs.m_strFirstEntry),
			m_strSecondEntry(rhs.m_strSecondEntry),
			m_strThirdEntry(rhs.m_strThirdEntry),
			m_strForthEntry(rhs.m_strForthEntry),
			m_strFifthEntry(rhs.m_strFifthEntry),
			m_strSixthEntry(rhs.m_strSixthEntry),
			m_dwSeventhEntry(rhs.m_dwSeventhEntry)
{
}

/**********************************************************************************************************                     
*  Function Name  :	~CIspyList                                                     
*  Description    :	D'tor
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0032
**********************************************************************************************************/
CIspyList::~CIspyList()
{
}

CIspyList& CIspyList::operator=(const CIspyList& rhs)
{
	if(this != &rhs)
	{
		m_strFirstEntry = rhs.m_strFirstEntry;
		m_strSecondEntry = rhs.m_strSecondEntry;
		m_strThirdEntry = rhs.m_strThirdEntry;
		m_strForthEntry = rhs.m_strForthEntry;
		m_strFifthEntry = rhs.m_strFifthEntry;
		m_strSixthEntry = rhs.m_strSixthEntry;
		m_dwSeventhEntry = rhs.m_dwSeventhEntry;
	}

	return *this;
}

/**********************************************************************************************************                     
*  Function Name  :	Serialize                                                     
*  Description    :	Serilize the data into or from DB
*  Author Name    : Neha Gharge                                                                                        
*  Date           : 22 Jan 2014
*  SR_NO		  : WRDWIZSERIALIZATION_0033
**********************************************************************************************************/
void CIspyList::Serialize( CArchive& ar )
{
	if (ar.IsStoring())
	{
		CRuntimeClass* pruntime = CIspyList::GetRuntimeClass();
		int oldnr = pruntime->m_wSchema;
		pruntime->m_wSchema = CURRENT_VERSION;

		ar.SerializeClass(pruntime);

		switch (CURRENT_VERSION)
		{
		case 1:
			ar << m_strFirstEntry << m_strSecondEntry << m_strThirdEntry << m_strForthEntry << m_strFifthEntry << m_strSixthEntry << m_dwSeventhEntry; 
			break;

		case 2:
			ar << m_strFirstEntry << m_strSecondEntry << m_strThirdEntry << m_strForthEntry << m_strFifthEntry << m_strSixthEntry;
			break;
		case 3:
			ar << m_dwSeventhEntry << m_strFirstEntry;
			break;

		default:
			// unknown version for this object
			AfxMessageBox(_T("Unknown file version."), MB_ICONSTOP);
			break;
		}

		pruntime->m_wSchema = oldnr;
	}
	// loading code
	else
	{
		ar.SerializeClass(RUNTIME_CLASS(CIspyList));

		UINT nVersion = ar.GetObjectSchema();

		switch (nVersion)
		{
		case 1:
			ar >> m_strFirstEntry >> m_strSecondEntry >> m_strThirdEntry >> m_strForthEntry >> m_strFifthEntry >> m_strSixthEntry >> m_dwSeventhEntry;
		//	m_strFifthEntry = _T("");
		//	m_strSixthEntry = _T("");
			break;

		case 2:
			ar >> m_strFirstEntry >> m_strSecondEntry >> m_strThirdEntry >> m_strForthEntry >> m_strFifthEntry >> m_strSixthEntry;
			break;
		
		case 3:
			ar >> m_dwSeventhEntry >> m_strFirstEntry;
			break;

		default:
			// unknown version for this object
			AfxThrowArchiveException(CArchiveException::badSchema);
			break;
		}
	}
}
