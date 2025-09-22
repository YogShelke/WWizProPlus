#pragma once
#include "stdafx.h"
#include "ISpyList.h"
#include "ISpyDataManager.h"

typedef struct ___WordArray
{
	int		iNoofString;
	BYTE	byIsAllow;
	std::wstring wStrWord[50];
}WordArray;

class CWrdWizContaintFilter: public CObject
{
public:
	CWrdWizContaintFilter(void);
	virtual ~CWrdWizContaintFilter(void);

	DWORD	FilterAttachment(CComQIPtr<Outlook::_MailItem> pMail);
	DWORD	FilterSubject(CComQIPtr<Outlook::_MailItem> pMail);
	DWORD	FilterMessageBody(CComQIPtr<Outlook::_MailItem> pMail);
	bool	FilterMessageBodyWithMaliciousSite(CComQIPtr<Outlook::_MailItem> pMail);
	bool	LoadDataContentFromFile(CString csPathName);
	bool	LoadMaliciousSiteFromFile(CString csPathName);
	DWORD	ScanMail4ContentScan(CComQIPtr<Outlook::_MailItem> pMail);
	void	LoadUserSubjectContainEntry();
	void	LoadUserMessageContainEntry();
	//Filter Sender address domain if it is present in our malicious domain list.
	//Neha Gharge 26th June,2015
	DWORD	FilterSenderAddrBlackDomain(CComQIPtr<Outlook::_MailItem> pMail);
public:
	bool					m_bMaliciousDBFileLoaded;
	bool					m_bContentDBFileLoaded;
private:

	typedef std::map<DWORD, WordArray> ContainFilterdb;
	ContainFilterdb			m_ContainFilterSubject;
	ContainFilterdb			m_ContainFilterMainBody;
	CDataManager			m_objContentFilterdb;
	CDataManager			m_objMaliciousSitedb;
	CStringArray			m_arobjAttchAllowExtention;
	CStringArray			m_arobjAttchBlockExtention;
	CStringArray			m_arobjAllowSubjectContains;
	CStringArray			m_arobjBlockSubjectContains;

	CStringArray			m_arobjAllowMessageContains;
	CStringArray			m_arobjBlockMessageContains;
	
	CStringArray			m_arobjMaliciousSites;
};
