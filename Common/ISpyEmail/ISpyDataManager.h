#pragma once

// CDataManager command target
#include "ISpyList.h"

class CDataManager : public CObject
{
	DECLARE_SERIAL(CDataManager);

	ContactList m_cContactsList;

public:
	CDataManager();
	virtual ~CDataManager();

	void Serialize( CArchive& ar );
	bool bWithoutEdit;
	bool AddContact(const CIspyList& contact, bool bAllowDuplicates =  false);
	bool EditContact(CString csKey, const CIspyList& contact, bool bAllowDuplicates = false);
	bool RemoveContact(const CString& FirstEntry);
	bool RemoveContact(const CString& FirstEntry, const CString& SecondEntry);
	bool RemoveContact(const CString& FirstEntry, const CString& SecondEntry, const CString& ThirdEntry);
	POSITION FindContact(const CString& FirstEntry,const CString& SecondEntry) const;

	POSITION FindContact(const CString& FirstEntry,const CString& SecondEntry, const CString& ThirdEntry) const;
	
	POSITION FindContact(const CString& FirstEntry) const;
	bool AddRecord(const CIspyList& contact);
	bool RemoveRecord(const CString& FirstEntry, const DWORD dwIndex);
	POSITION FindRecord(const CString& FirstEntry,const DWORD dwIndex) const;
	bool FindContact(const CString& FirstEntry, CIspyList& CIspyList) const;
	void RemoveAll();

	bool RemoveReportEntry(const CString& csDateTime, const CString& csScanType, const CString& csFilePath);
	
	const ContactList& GetContacts() const {return m_cContactsList;}

	void	SetFileVersion(int nFV) {	m_uiFileVersion = nFV;	}
	int		GetFileVersion() { return m_uiFileVersion; }

private:
	int		m_uiFileVersion;
};


