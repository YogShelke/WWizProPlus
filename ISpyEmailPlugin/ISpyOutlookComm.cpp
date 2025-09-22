/**********************************************************************************************************      		Program Name          : ISpyOutlookComm.cpp
	Description           : This class which provides functionality for write signature in mail
	Author Name			  : Ramkrushna Shelke                                                                  	  
	Date Of Creation      : 07th Aug 2014
	Version No            : 1.0.0.8
	Special Logic Used    : 
	Modification Log      :           
***********************************************************************************************************/
#include "stdafx.h"
#include "OutlookAddinApp.h"
#include "ISpyOutlookComm.h"
#include <string>

using std::wstring;

/***************************************************************************
  Function Name  : CISpyOutlookComm
  Description    : Cont'r
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0066
  Date           : 07th Aug 2014
****************************************************************************/
CISpyOutlookComm::CISpyOutlookComm(void)
{
}

/***************************************************************************
  Function Name  : CISpyOutlookComm
  Description    : Cont'r
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0067
  Date           : 07th Aug 2014
****************************************************************************/
CISpyOutlookComm::~CISpyOutlookComm(void)
{
}


/***************************************************************************
  Function Name  : MoveMailItem
  Description    : Function which takes two arguments source and destination
				   mail moves from source folder path to destination folder.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0068
  Date           : 07th Aug 2014
****************************************************************************/
bool CISpyOutlookComm::MoveMailItem(CComQIPtr<Outlook::_MailItem> &spMailItem, Outlook::MAPIFolder *spMapiFolderMove)
{
	try
	{
		if(!spMailItem && !spMapiFolderMove)
			return false;

		IDispatch *pDisptemp;
		if(spMailItem != NULL)
		{
			HRESULT hr = spMailItem->Move(spMapiFolderMove, &pDisptemp);
			if (FAILED(hr))
			{
				return false;
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CAddin::MoveMailItem", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
  Function Name  : GetMailBody
  Description    : Function which get the mail body provided mail item pointer
				   as an argument.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0069
  Date           : 07th Aug 2014
****************************************************************************/
CString CISpyOutlookComm::GetMailBody(CComQIPtr<Outlook::_MailItem> &pMail, bool bHtmlBody)
{
	try
	{
		if(!pMail)
			return L"";

		//Issue resolved junk characters are coming in mail
		BSTR bstrBody ;//= ::SysAllocString(L"");

		HRESULT hr = S_FALSE;
		if(bHtmlBody)
		{
			hr = pMail->get_HTMLBody(&bstrBody);
		}
		else
		{
			hr = pMail->get_Body(&bstrBody);
		}

		if (FAILED(hr) || bstrBody == NULL)
		{
			return L"";
		}

		//::SysFreeString(bstrBody);

		return CString(bstrBody);
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CAddin::GetMailBody", 0, 0, true, SECONDLEVEL);
	}
	return L"";
}

/***************************************************************************
  Function Name  : PutMailBody
  Description    : Function which writes the containt in mail body after changes.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0070
  Date           : 07th Aug 2014
****************************************************************************/
bool CISpyOutlookComm::PutMailBody(CComQIPtr<Outlook::_MailItem> &pCurrentMail, CString csMailBody, bool bIsHtmlType)
{
	try
	{
		if(!pCurrentMail)
			return false;

		HRESULT hr = NULL;

		CComQIPtr<Outlook::_MailItem> pMail(pCurrentMail);

		_bstr_t body = csMailBody;

		DWORD RetryCount = 2;
		while(RetryCount != 0)
		{
			if(bIsHtmlType)
			{
				hr = pMail->put_HTMLBody(body);
			}
			else
			{
				hr = pMail->put_Body(body);
			}

			hr = pMail->Save();
			
			if(SUCCEEDED(hr))
			{
				return true;
			}
			else
			{
				RetryCount--;
				_com_error err(hr);
				AddLogEntry(L"### %s", err.ErrorMessage(), 0, true, SECONDLEVEL);
			}
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CWardwizOutlookComm::PutMailBody", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
  Function Name  : GetMailSubject
  Description    : Function which gets the subjet of mail using mail pointer.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0071
  Date           : 07th Aug 2014
****************************************************************************/
CString CISpyOutlookComm::GetMailSubject(CComQIPtr<Outlook::_MailItem> &pMail)
{
	try
	{
		if (!pMail)
			return "";

		BSTR bstrSubject = ::SysAllocString(L"");
		HRESULT hr  = pMail->get_Subject(&bstrSubject);
		if (FAILED(hr))
		{
			return "";
		}
		::SysFreeString(bstrSubject);
		return CString(bstrSubject);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizOutlookComm::GetMailSubject", 0, 0, true, SECONDLEVEL);
	}
	return "";
}

/***************************************************************************
  Function Name  : PutMailSubject
  Description    : Function which puts the subject in mail.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0072
  Date           : 07th Aug 2014
****************************************************************************/
bool CISpyOutlookComm::PutMailSubject(CComQIPtr<Outlook::_MailItem> &pMail, CString csSubject)
{
	try
	{
		if (!pMail)
			return "";

		HRESULT hr = S_FALSE;

		BSTR bstrSubject = ::SysAllocString(csSubject);
		hr = pMail->put_Subject(bstrSubject);
		if (FAILED(hr))
		{
			return false;
		}

		DWORD RetryCount = 2;
		while (RetryCount != 0)
		{
			hr = pMail->Save();
			if (SUCCEEDED(hr))
			{
				::SysFreeString(bstrSubject);
				return true;
			}
			else
			{
				RetryCount--;
				_com_error err(hr);
				AddLogEntry(L"### %s", err.ErrorMessage(), 0, true, SECONDLEVEL);
			}
		}

		::SysFreeString(bstrSubject);
		return false;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizOutlookComm::PutMailSubject", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************
  Function Name  : GetSenderEmailAddress
  Description    : Function which withdrae a senders email address from incomming 
				   mail.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0073
  Date           : 07th Aug 2014
****************************************************************************/
CString CISpyOutlookComm::GetSenderEmailAddress(CComQIPtr<Outlook::_MailItem> &pMail)
{
	try
	{
		if (!pMail)
			return "";

		BSTR bstrSenderEmail = ::SysAllocString(L"");
		HRESULT hr = pMail->get_SenderEmailAddress(&bstrSenderEmail);
		if (FAILED(hr))
		{
			return L"";
		}
		::SysFreeString(bstrSenderEmail);

		return CString(bstrSenderEmail);
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizOutlookComm::GetSenderEmailAddress", 0, 0, true, SECONDLEVEL);
	}
	return "";
}

/***************************************************************************
  Function Name  : ISSignaturePresent
  Description    : Function which check the signture is present in the mail body
				   (or) not if it is already exists then return	true else false.			   
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0074
  Date           : 07th Aug 2014
****************************************************************************/
bool CISpyOutlookComm::ISSignaturePresent(CComQIPtr<Outlook::_MailItem> &pMail, CString csSignature)
{
	try
	{
		//Avoid multiple getmailbody calls
		//Neha Gharge 18th July,2015
		CString csBody = theApp.m_objISpyOutlookComm.m_bstrMailBody;
		csBody.MakeLower();
		CString csHiddenTxt = L"#$^$#";

		wstring strBody(csBody);
		int iPos = static_cast<int>(strBody.rfind(csHiddenTxt));
		if (iPos == -1)
		{
			return false;
		}
		else if (iPos > 0)
		{
			int iPosNxt = iPos - csHiddenTxt.GetLength();

			int iPos1 = static_cast<int>(strBody.rfind(csHiddenTxt, iPosNxt));

			if (iPos1 > 0)
			{
				int iMailLen = csBody.GetLength();
				CString csSig = csBody.Mid(iPos1 + csHiddenTxt.GetLength(), (iPosNxt - iPos1));
				/*int iPos2 = csSig.Find(csHiddenTxt,0);
				csSig.Truncate(iPos2);*/
				if (csSig == csSignature.MakeLower())
					return true;
				else
					return false;
			}
			else
				return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizOutlookComm::ISSignaturePresent", 0, 0, true, SECONDLEVEL);
	}

	return false;
}

/**************************************************************************
  Function Name  : GetMailAttchments
  Description    : Function which enumerates the attachments and gets the files
				   names.
  Author Name    : Ramkrushna Shelke
  SR.NO			 :  EMAILSCANPLUGIN_0075
  Date           : 07th Aug 2014
****************************************************************************/
bool CISpyOutlookComm::GetMailAttchments(CComQIPtr<Outlook::_MailItem> &pMail, CStringArray &csAttachFileNames)
{
	try
	{
		CComPtr<Outlook::Attachments> objAttachments;
		HRESULT hr = pMail->get_Attachments(&objAttachments);
		if (FAILED(hr) || objAttachments == NULL)
		{
			return false;
		}

		long lAttachCount = 0;
		hr = objAttachments->get_Count(&lAttachCount);
		if (FAILED(hr))
		{
			return false;
		}

		for (long lIndex = 1; lIndex <= lAttachCount; lIndex++)
		{
			CComPtr<Outlook::Attachment> objAttachment;
			VARIANT index;
			index.iVal = (SHORT)lIndex; // value
			index.vt = VT_I2; // type of variant = integer

			hr = objAttachments->Item(index, &objAttachment);
			if (SUCCEEDED(hr) && objAttachment != NULL)
			{
				BSTR bstrFileName = ::SysAllocString(L"");

				TCHAR	szZipPath[1024] = { 0 };
				hr = objAttachment->get_FileName(&bstrFileName);
				if (FAILED(hr) || bstrFileName == NULL)
				{
					continue;
				}
				csAttachFileNames.Add(bstrFileName);
				SysFreeString(bstrFileName);
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizOutlookComm::GetMailAttchments", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/**************************************************************************
  Function Name  : RemoveMailAttchments
  Description    : Function which Removes the attachments from mail, which takes
				   the filename as input.
  Author Name    : Ramkrushna Shelke
  SR.NO			 :  EMAILSCANPLUGIN_0076
  Date           : 07th Aug 2014
****************************************************************************/
bool CISpyOutlookComm::RemoveMailAttchments(CComQIPtr<Outlook::_MailItem> &pMail, CStringArray csAttachFileNames)
{
	HRESULT hr = S_FALSE;

	CComPtr<Outlook::Attachments> objAttachments;
	hr = pMail->get_Attachments(&objAttachments);

	if (FAILED(hr) || objAttachments == NULL)
	{
		return false;
	}

	long lAttachCount = 0;
	hr = objAttachments->get_Count(&lAttachCount);
	if (FAILED(hr))
	{
		return false;
	}

	for (long lIndex = 1; lIndex <= lAttachCount; lIndex++)
	{
		CComPtr<Outlook::Attachment> objAttachment;
		VARIANT index;
		index.iVal = (SHORT)lIndex; // value
		index.vt = VT_I2; // type of variant = integer

		hr = objAttachments->Item(index, &objAttachment);
		if (SUCCEEDED(hr) && objAttachment != NULL)
		{
			BSTR bstrFileName = ::SysAllocString(L"");

			TCHAR	szZipPath[1024] = {0} ;
			objAttachment->get_FileName( &bstrFileName ) ;
			for(int iIndex = 0; iIndex < csAttachFileNames.GetCount(); iIndex++)
			{
				if(bstrFileName == csAttachFileNames.GetAt(iIndex))
				{
					objAttachment->Delete();
				}
			}
			SysFreeString( bstrFileName ) ;
		}
	}

	DWORD RetryCount = 2;
	while(RetryCount != 0)
	{
		hr = pMail->Save();
		if(SUCCEEDED(hr))
		{
			return true;
		}
		else
		{
			RetryCount--;
			_com_error err(hr);
			AddLogEntry(L"### %s", err.ErrorMessage(), 0, true, SECONDLEVEL);
		}
	}
	return false;
}

/**************************************************************************
Function Name  : ISMailProceedAlready
Description    : Is mail is already proceed by wardwiz
Author Name    : Neha Gharge
SR.NO			 :  
Date           : 17th July 2015
****************************************************************************/
bool CISpyOutlookComm::ISMailProceedAlready(CComQIPtr<Outlook::_MailItem> &pMail, CString csPropertyName)
{
	try
	{
		if (!pMail)
			return true;

		CComPtr<Outlook::UserProperties> spUserProperties;
		HRESULT hr = pMail->get_UserProperties(&spUserProperties);
		if (FAILED(hr))
		{
			OUTPUT_DEBUG_STRING(_T("### Failed get_UserProperties in CISpyOutlookComm::ISMailProceedAlready for %s"), csPropertyName);
			return false;
		}

		if (spUserProperties == NULL)
		{
			return false;
		}

		//Issue : Even sender's address is blocked. It comes into virus scan entry
		//Neha Gharge 2 Sept,2015
		CComPtr<Outlook::UserProperty> spProp;
		if (csPropertyName == WARDWIZSPAMPROP)
		{
			hr = spUserProperties->Item(CComVariant(WARDWIZSPAMPROP), &spProp);
		}
		else if (csPropertyName == WARDWIZBODYPROP)
		{
			hr = spUserProperties->Item(CComVariant(WARDWIZBODYPROP), &spProp);
		}
		else if (csPropertyName == WARDWIZNONBODYPROP)
		{
			hr = spUserProperties->Item(CComVariant(WARDWIZNONBODYPROP), &spProp);
		}
		else if (csPropertyName == WARDWIZSCANPROP)
		{
			hr = spUserProperties->Item(CComVariant(WARDWIZSCANPROP), &spProp);
		}
		else if (csPropertyName == WARDWIZMAILISSPAM)
		{
			hr = spUserProperties->Item(CComVariant(WARDWIZMAILISSPAM), &spProp);
		}

		
		if (FAILED(hr))
		{
			OUTPUT_DEBUG_STRING(_T("### Failed spUserProperties->Item in CISpyOutlookComm::ISMailProceedAlready"));
			return false;
		}

		if (spProp == NULL)
		{
			OUTPUT_DEBUG_STRING(_T("### Failed spUserProperties->Item in CISpyOutlookComm::ISMailProceedAlready"));
			return false;
		}

		TCHAR buf[MAX_PATH] = { 0 };
		CComVariant v;
		spProp->get_Value(&v);
		_stprintf_s(buf, MAX_PATH, _T("Value: %s"), OLE2T(v.bstrVal));

		AddLogEntry(L">>> Mail is already processed by %s", csPropertyName, 0, true, ZEROLEVEL);
		OUTPUT_DEBUG_STRING(_T(">>> Mail is already processed by WardWiz"), csPropertyName);
		return true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizOutlookComm::ISMailProceedAlready", 0, 0, true, SECONDLEVEL);
		return true;
	}
	return false;
}

/**************************************************************************
Function Name  : PutWardWizPropertyIntoMail
Description    : Put wardwiz property into mail.
Author Name    : Neha Gharge
SR.NO			 : 
Date           : 17th July 2015
****************************************************************************/
bool CISpyOutlookComm::PutWardWizPropertyIntoMail(CComQIPtr<Outlook::_MailItem> &pMail, CString csPropertyName)
{
	try
	{
		//If pMail is invalid it should return
		//Neha Gharge 18th July,2015
		if (!pMail)
		{
			AddLogEntry(L"pMail in PutWardwizPropertyIntoMail is invalid pointer");
			return false;
		}

		CComPtr<Outlook::UserProperties> spUserProperties;
		HRESULT hr = pMail->get_UserProperties(&spUserProperties);
		if (FAILED(hr) || spUserProperties == NULL)
		{
			OUTPUT_DEBUG_STRING(_T("### Failed get_UserProperties in CISpyOutlookComm::PutWardWizPropertyIntoMail"));
			return false;
		}
		
		//Issue : Even sender's address is blocked. It comes into virus scan entry
		//Neha Gharge 2 Sept,2015
		CComPtr<Outlook::UserProperty> spProp;
		if (csPropertyName == L"WardWizSpam")
		{
			hr = spUserProperties->Item(CComVariant(L"WardWizSpam"), &spProp);
		}
		else if (csPropertyName == L"WardWizBody")
		{
			hr = spUserProperties->Item(CComVariant(L"WardWizBody"), &spProp);
		}
		else if (csPropertyName == L"WardWizNonBody")
		{
			hr = spUserProperties->Item(CComVariant(L"WardWizNonBody"), &spProp);
		}
		else if (csPropertyName == WARDWIZSCANPROP)
		{
			hr = spUserProperties->Item(CComVariant(WARDWIZSCANPROP), &spProp);
		}
		else if (csPropertyName == WARDWIZMAILISSPAM)
		{
			hr = spUserProperties->Item(CComVariant(WARDWIZMAILISSPAM), &spProp);
		}

		if (FAILED(hr))
		{
			OUTPUT_DEBUG_STRING(_T("### Failed get_UserProperties in CISpyOutlookComm::PutWardWizPropertyIntoMail"));
		}

		if (spProp)
		{
			OUTPUT_DEBUG_STRING(_T("### Mail is already processed by WardWiz CISpyOutlookComm::PutWardWizPropertyIntoMail"));
			AddLogEntry(L"Mail is already processed by Wardwiz :: PutWardwizPropertyIntoMail", 0, 0, true, ZEROLEVEL);
			return false;
		}

		hr = spUserProperties->Add(CComBSTR(csPropertyName), Outlook::olText, CComVariant(false),
			CComVariant((long)0), &spProp);
		if (FAILED(hr))
		{
			OUTPUT_DEBUG_STRING(_T("### Failed to add spUserProperties->Add"));
			return false;
		}

		hr = spProp->put_Value(CComVariant(1));
		if (FAILED(hr))
		{
			OUTPUT_DEBUG_STRING(_T("### Failed to add spProp->put_Value"));
			return false;
		}

		hr = pMail->Save();
		if (FAILED(hr))
		{
			OUTPUT_DEBUG_STRING(_T("### Mail is already processed by WardWiz CISpyOutlookComm::PutWardWizPropertyIntoMail"));
			AddLogEntry(L"Mail is already processed by GenX :: PutWardwizPropertyIntoMail", 0, 0, true, ZEROLEVEL);
			return false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CWardwizOutlookComm::PutWardwizPropertyIntoMail", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}