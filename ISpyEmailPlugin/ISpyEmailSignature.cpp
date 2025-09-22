/**********************************************************************************************************      		Program Name          : ISpyEmailSignature.cpp
	Description           : This class which provides functionality for write signature in mail
	Author Name			  : Ramkrushna Shelke                                                                  	  
	Date Of Creation      : 07th Aug 2014
	Version No            : 1.0.0.8
	Special Logic Used    : 
	Modification Log      :           
***********************************************************************************************************/
#include "stdafx.h"
#include "ISpyEmailSignature.h"
//#include "ISpyOutlookComm.h"
#include "OutlookAddinApp.h"

/***************************************************************************
  Function Name  : CISpyEmailSignature
  Description    : Cont'r
  Author Name    : Ramkrushna Shelke
  SR.NO			 : SR.NO EMAILSCANPLUGIN_0055
  Date           : 07th Aug 2014
****************************************************************************/
CISpyEmailSignature::CISpyEmailSignature(void):
	 m_bEmailSignatureLoaded(false)
	,m_csHTMLSignature(L"") 
{
}

/***************************************************************************
  Function Name  : ~CISpyEmailSignature
  Description    : Dest'r
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0056
  Date           : 07th Aug 2014
****************************************************************************/
CISpyEmailSignature::~CISpyEmailSignature(void)
{
}

/***************************************************************************
  Function Name  : LoadEmailSignatureFromFile
  Description    : Function which loads the Email signature from file.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0057
  Date           : 07th Aug 2014
****************************************************************************/
bool CISpyEmailSignature::LoadEmailSignatureFromFile(CString csPathName)
{
	if(m_bEmailSignatureLoaded)
	{
		return true;
	}
	try
	{
		AddLogEntry(L">>> CWardwizEmailSignature::Load Email Signature from database", 0, 0, true, FIRSTLEVEL);
		if( !PathFileExists( csPathName ) )
			return false;

		CFile theFile;
		if(!theFile.Open((LPCTSTR)csPathName, CFile::modeRead | CFile::shareDenyWrite))
		{
			return false;
		}

		// Create a loading archive
		CArchive archive(&theFile, CArchive::load);

		int iVersion = m_objEmailSignatureDB.GetFileVersion();
		m_objEmailSignatureDB.Serialize(archive);

		// Close the loading archive
		archive.Close();
		theFile.Close();

		const ContactList& contacts = m_objEmailSignatureDB.GetContacts();

		// iterate over all contacts add add them to the list
		int nCurrentItem = 0;
		POSITION pos = contacts.GetHeadPosition();
		if(pos == NULL)
		{
			AddLogEntry(L"### No entry in load signaturedb CAddin::LoadEmailSignatureFromFile", 0, 0, true, SECONDLEVEL);
			return false;
		}
		
		while(pos != NULL)
		{
			const CIspyList contact = contacts.GetNext(pos);
			m_csHTMLSignature = contact.GetFirstEntry();
			//Used in addin for checking signature.
			m_csSignatureInDB = m_csHTMLSignature;
		}	
		m_bEmailSignatureLoaded = true;
	}
	catch (CArchiveException* pEx) 
	{
		pEx->ReportError(); 
		pEx->Delete(); 
	}
	catch (...) 
	{
		AddLogEntry(L"### Exception in CAddin::LoadEmailSignatureFromFile", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************
  Function Name  : AppendSignatureInMail
  Description    : Function which takes mail pointer as an argument 
				   and appends the signature @end.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0058
  Date           : 07th Aug 2014
  Modification   : 18th June 2015 Neha Gharge Signature modified.
****************************************************************************/
bool CISpyEmailSignature::AppendSignatureInMail(CComQIPtr<Outlook::_MailItem> &spMailItem)
{
	try
	{
		bool bImagePresentInLocalFolder = false;
		if(!spMailItem)
			return false;

		bool bIsHtmlFormat = true;
		AddLogEntry(L">>> CWardwizEmailSignature::Append Signature in mail", 0, 0, true, FIRSTLEVEL);

		//Took a plain body at one time and store in member variable.
		//Avoid multiple getmailbody call
		//Neha Gharge 18th July,2015
		//CISpyOutlookComm  objISpyOutlookComm;

		//Get Subject here
		CString csSubject = theApp.m_objISpyOutlookComm.GetMailSubject(spMailItem);

		//Get body here
		CString csMailBody, csFBImageFilePath, csTWImageFilePath, csLNImageFilePath, csLogoImageFilePath, csLineImageFilePath;
		csMailBody = theApp.m_objISpyOutlookComm.GetMailBody(spMailItem, bIsHtmlFormat);
		AddLogEntry(L"The mail body in HTML format %s", csMailBody, 0, true, ZEROLEVEL);

		CString csImageFilePath = GetWardWizPathFromRegistry();
		csLogoImageFilePath = csImageFilePath + L"ORIGINAL LOGO.JPG";
		csFBImageFilePath = csImageFilePath + L"FB.JPG";
		csTWImageFilePath = csImageFilePath + L"TT.JPG";
		csLNImageFilePath = csImageFilePath + L"LN.JPG";
		csLineImageFilePath = csImageFilePath + L"LINE.JPG";

		if ((PathFileExists(csLogoImageFilePath)) && (PathFileExists(csFBImageFilePath)) && (PathFileExists(csTWImageFilePath)) && (PathFileExists(csLNImageFilePath)) && (PathFileExists(csLineImageFilePath)))
		{
			bImagePresentInLocalFolder = true;
		}
		AddLogEntry(L">>> CWardwizEmailSignature::Checking signature is present or not", 0, 0, true, FIRSTLEVEL);
		
		if(!theApp.m_objISpyOutlookComm.ISSignaturePresent(spMailItem, m_csHTMLSignature))
		{
			//csMailBody.Replace(L"</div>", L"");
			csMailBody.Replace(L"</body>", L"");
			csMailBody.Replace(L"</html>", L"");

			//csMailBody.Append(L"<body>");
			//csMailBody.Append(L"<font face=\"Calibiri\" size=\"2\" color=\"teal\">");
			//csMailBody.Append(L"<div style=\"font-family:Calibri;font-size:10pt;color:teal\">");
			
			//csMailBody.Append(L"<span style=\"color:teal;font-size:12px;margin-right:-10px;\">m_csHTMLSignature;</span>");
			//csMailBody += m_csHTMLSignature;
			//csMailBody.Append(L"<span style=\"display:none;color:#ffffff;font-size:1px;\">#$^$#</span>");
						
			
			//csMailBody.Append(L"</div>");
			//csMailBody.Append(L"<div align = \"center\" style = \"margin - right:829px;\">");
			csMailBody.Append(L"<table>");
			csMailBody.Append(L"<tr>");
			//
			if (bImagePresentInLocalFolder)
			{ 
				csMailBody.Append(L"<td><img alt=\"\" src=\"");
				csMailBody.Append(csLogoImageFilePath);
				csMailBody.Append(L"\"></td>");
				csMailBody.Append(L"<td>");
				csMailBody.Append(L"<span style=\"display:none;color:#ffffff;font-size:1px;\">#$^$#</span>");
				csMailBody.Append(m_csHTMLSignature);
				csMailBody.Append(L"<span style=\"display:none;color:#ffffff;font-size:1px;\">#$^$#</span>");
				csMailBody.Append(L"<span style=\"font-size:12pt;text-indent:-5px;\"><sup>&copy;2015</sup></span>");
				csMailBody.Append(L"</td>");
				csMailBody.Append(L"<td><img alt=\"\" src=\"");
				csMailBody.Append(csLineImageFilePath);
				csMailBody.Append(L"\"></td>");
				csMailBody.Append(L"<td>");
				csMailBody.Append(L"<a href=\"https://www.facebook.com/Vibraniumav" target=\"_blank\"><img alt=\"\" src=\"");
				csMailBody.Append(csFBImageFilePath);
				csMailBody.Append(L"\"></a>");
				csMailBody.Append(L"</td>");
				csMailBody.Append(L"<td>");
				csMailBody.Append(L"<a href=\"https://twitter.com/Vibranium_india" target=\"_blank\"><img alt=\"\" src=\"");
				csMailBody.Append(csTWImageFilePath);
				csMailBody.Append(L"\"></a>");
				csMailBody.Append(L"</td>");
				csMailBody.Append(L"<td>");
				csMailBody.Append(L"<a href=\"https://www.linkedin.com/company/vibranium-india/" target=\"_blank\"><img alt=\"\" src=\"");
				csMailBody.Append(csLNImageFilePath);
				csMailBody.Append(L"\"></a>");
				csMailBody.Append(L"</div></td>");
			}
			else
			{
				csMailBody.Append(L"<td><img alt=\"WardWiz\" title=\"WardWiz\" src=\"http://wardwiz.in/social/logo.jpg\"></td>");
				csMailBody.Append(L"<td>");
				csMailBody.Append(L"<span style=\"display:none;color:#ffffff;font-size:1px;\">#$^$#</span>");
				csMailBody.Append(m_csHTMLSignature);
				csMailBody.Append(L"<span style=\"display:none;color:#ffffff;font-size:1px;\">#$^$#</span>");
				csMailBody.Append(L"<span style=\"font-size:12pt;text-indent:-5px;\"><sup>&copy;2015</sup></span>");
				csMailBody.Append(L"</td>");
				csMailBody.Append(L"<td>");
				csMailBody.Append(L"<img src =\"http://wardwiz.in/social/line.jpg\">");
				csMailBody.Append(L"</td>");
				csMailBody.Append(L"<td>");
				csMailBody.Append(L"<a href=\"https://www.facebook.com/Vibraniumav" target=\"_blank\"><img alt=\"\" src=\"http://wardwiz.in/social/facebook.jpg\"></a>");
				csMailBody.Append(L"</td>");
				csMailBody.Append(L"<td>");
				csMailBody.Append(L"<a href=\"https://twitter.com/Vibranium_india" target=\"_blank\"><img alt=\"\" src=\"http://wardwiz.in/social/twitter.jpg\"></a>");
				csMailBody.Append(L"</td>");
				csMailBody.Append(L"<td>");
				csMailBody.Append(L"<a href=\"https://www.linkedin.com/company/vibranium-india/" target=\"_blank\"><img alt=\"\" src=\"http://wardwiz.in/social/linkedin.jpg\"></a>");
				csMailBody.Append(L"</div></td>");
			}
			csMailBody.Append(L"</tr>");
			csMailBody.Append(L"</table>");
			csMailBody.Append(L"</body>");
			csMailBody.Append(L"</html>\r\n");

			//csMailBody.Append(L"<div class=Section1>\r\n");
			//csMailBody.Append(L"<hr width=\"50%\" align=\"left\"/>\r\n");
			//csMailBody.Append(L"<p class=\"MsoNormal\"><o:p>\r\n");
			//csMailBody.Append(L"<span style=\"color:maroon;font-weight:bold;\">");
			//csMailBody += m_csHTMLSignature;
			//csMailBody.Append(L"</span>\r\n");
			//csMailBody.Append(L"<span style=\"font-size:9px;color:black;\">@iSpyAV</span> </o:p></p>\r\n");
			//csMailBody.Append(L"<p>\r\n");
			//csMailBody.Append(L"</p>\r\n");
			//csMailBody.Append(L"</div>\r\n");
			//csMailBody.Append(L"</body>\r\n");
			//csMailBody.Append(L"</html>\r\n");
		}

		if(!theApp.m_objISpyOutlookComm.PutMailBody(spMailItem, csMailBody, bIsHtmlFormat))
		{
			AddLogEntry(L"### Failed to put body Subject: %s", csSubject, 0, true, SECONDLEVEL);
			return false;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CAddin::AppendSignatureInMail", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}