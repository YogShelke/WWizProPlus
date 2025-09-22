#pragma once

class CISpyOutlookComm
{
public:
	CISpyOutlookComm(void);
	virtual ~CISpyOutlookComm(void);

public:
	BSTR  m_bstrMailBody;
	bool MoveMailItem(CComQIPtr<Outlook::_MailItem> &spMailItem, Outlook::MAPIFolder *spMapiFolderMove);
	bool PutMailBody(CComQIPtr<Outlook::_MailItem> &pMail, CString csMailBody, bool bIsHtmlType);
	CString GetMailBody(CComQIPtr<Outlook::_MailItem> &pMail, bool bHtmlBody = false);
	CString GetSenderEmailAddress(CComQIPtr<Outlook::_MailItem> &pMail);
	CString GetMailSubject(CComQIPtr<Outlook::_MailItem> &pMail);
	bool PutMailSubject(CComQIPtr<Outlook::_MailItem> &pMail, CString csSubject);
	bool ISSignaturePresent(CComQIPtr<Outlook::_MailItem> &pMail, CString csSignature);
	bool GetMailAttchments(CComQIPtr<Outlook::_MailItem> &pMail, CStringArray &csAttachFileNames);
	bool RemoveMailAttchments(CComQIPtr<Outlook::_MailItem> &pMail, CStringArray csAttachFileNames);
	bool ISMailProceedAlready(CComQIPtr<Outlook::_MailItem> &pMail, CString csPropertyName);
	bool PutWardWizPropertyIntoMail(CComQIPtr<Outlook::_MailItem> &pMail, CString csPropertyName);
};
