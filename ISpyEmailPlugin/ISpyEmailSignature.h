#pragma once
#include "ISpyList.h"
#include "ISpyDataManager.h"

class CISpyEmailSignature
{
public:
	CISpyEmailSignature(void);
	virtual ~CISpyEmailSignature(void);
	bool LoadEmailSignatureFromFile(CString csPathName);
	bool AppendSignatureInMail(CComQIPtr<Outlook::_MailItem> &spMailItem);
public:
	bool					m_bEmailSignatureLoaded;
	CString					m_csSignatureInDB;
private:
	CDataManager			m_objEmailSignatureDB;
	CString					m_csHTMLSignature;
};
