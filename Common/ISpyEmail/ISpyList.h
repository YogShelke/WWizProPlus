#pragma once

// Contact command target

#include <afxtempl.h>

class CIspyList : public CObject
{
public:
	DECLARE_SERIAL(CIspyList);

	CIspyList();
	CIspyList(const CIspyList& rhs);
	CIspyList(const CString& m_strFirstEntry, const CString& m_strSecondEntry, const CString& m_strThirdEntry, const CString& m_strForthEntry, 
			// these two must have a default value because they are not used in the first version
			const CString& m_strFifthEntry = _T(""), const CString& m_strSixthEntry = _T(""),const DWORD m_dwSeventhEntry=0);

	CIspyList& operator=(const CIspyList& rhs);

	virtual ~CIspyList();

	void Serialize( CArchive& ar );

	CString GetFirstEntry() const {return m_strFirstEntry;}
	DWORD   GetSeventhEntry() const {return m_dwSeventhEntry;}
	CString GetSecondEntry() const {return m_strSecondEntry;}
	CString GetThirdEntry() const {return m_strThirdEntry;}
	CString GetForthEntry() const {return m_strForthEntry;}
	CString GetFifthEntry() const {return m_strFifthEntry;}
	CString GetSixthEntry() const {return m_strSixthEntry;}

	void	SetFirstEntry(const CString& FirstEntry) {m_strFirstEntry = FirstEntry;}
	void	SetSeventhEntry(const DWORD dwSeventhEntry){m_dwSeventhEntry = dwSeventhEntry;}
	void	SetSecondEntry(const CString& SecondEntry) {m_strSecondEntry = SecondEntry;}
	void	SetThirdEntry(const CString& ThirdEntry) {m_strThirdEntry = ThirdEntry;}
	void	SetForthEntry(const CString& ForthEntry) {m_strForthEntry = ForthEntry;}
	void	SetFifthEntry(const CString& FifthEntry) {m_strFifthEntry = FifthEntry;}
	void	SetSixthEntry(const CString& SixthEntry) {m_strSixthEntry = SixthEntry;}

	static void SetCurrentFileVersion(int nCV) {	CURRENT_VERSION = nCV;	}
	static int	GetCurrentFileVersion() {	return CURRENT_VERSION;	}

private:
	static int	CURRENT_VERSION;
	CString		m_strFirstEntry;
	CString		m_strSecondEntry;
	CString		m_strThirdEntry;
	CString		m_strForthEntry;
	CString		m_strFifthEntry;
	CString		m_strSixthEntry;
	DWORD		m_dwSeventhEntry;
};

typedef CList<CIspyList, CIspyList>		ContactList;
