/**********************************************************************************************************      
		Program Name			: Addin.cpp
		Description				: Main Interface implentation
		Author Name				: Ramkrushna Shelke          
		Date Of Creation		: 10 Dec 2013
		Version No				: 1.0.0.5
		Special Logic Used		: 
		Modification Log		:           
	  1. Ramkrushna           implentation of _IDTExtensibility2 interface      10 - 12 - 2013              
***********************************************************************************************************/
#include "stdafx.h"
#include "OutlookAddin.h"
#include "Addin.h"
#include <direct.h>
#include <stdlib.h>
#include <direct.h>
#include "ISpyOutlookComm.h"
#include "ISpyVirusPopUpDlg.h"
#include "ISpyCommunicator.h"
#include "PipeConstants.h"
#include "OutlookAddinApp.h"
#include "WardWizEmailCustomMsgBox.h"



/////////////////////////////////////////////////////////////////////////////
// CAddin
_ATL_FUNC_INFO OnClickButtonInfo			=	{CC_STDCALL,VT_EMPTY,2, {VT_DISPATCH, VT_BYREF | VT_BOOL}};
_ATL_FUNC_INFO OnSendInfo					=	{CC_STDCALL,VT_EMPTY,2, {VT_DISPATCH, VT_BOOL}};     			
_ATL_FUNC_INFO OnNewItemInfo				=	{CC_STDCALL,VT_EMPTY,1,{VT_DISPATCH}};   
_ATL_FUNC_INFO OnNewMail = { CC_STDCALL, VT_EMPTY, 1, { VT_BSTR } };

static DATE g_dtPreviousItemCreation = 0.0;

CAddin * CAddin::m_pThis = NULL;
CISpyCommunicatorServer  m_objCommServer(EMAILPLUGIN_SERVER, CAddin::OnDataReceiveCallBack, sizeof(ISPY_PIPE_DATA));

// Display a pregress bar to show status of moving mail
//Neha Gharge 1 July 2015
DWORD WINAPI MoveMailProgressDisplay(LPVOID lpParam);

/***************************************************************************
  Function Name  : CAddin
  Description    : Const'r
  Author Name    : Ramkrushna Shelke
  S.R. No		 : EMAILSCANPLUGIN_0001
  Date           : 07th Aug 2014
****************************************************************************/
CAddin::CAddin():
	  m_objCommServer(EMAILPLUGIN_SERVER, CAddin::OnDataReceiveCallBack, sizeof(ISPY_PIPE_DATA))
	, m_bUserMoveActionFromSpam(false)
	, m_hMoveMailProgressDisplayThread(NULL)
	, m_SelectedMailFromWardWizSpam(false)
	, m_bKeepFutureMailAsSpam(false)
	, m_bKeepFutureMailAsNotSpam(false)
{
	m_pThis = this;
}

/***************************************************************************
  Function Name  : CAddin
  Description    : Dest'r
  Author Name    : Ramkrushna Shelke
  S.R. No		 : EMAILSCANPLUGIN_0002
  Date           : 07th Aug 2014
****************************************************************************/
CAddin::~CAddin()
{
}

/***************************************************************************
Function Name	: OnNewMailEx
Description		: Strucutured Excepetion handling with new mail.
Author Name		: Ramkrushna Shelke
S.R. No			: 
Date			: 07th Aug 2014
****************************************************************************/
void __stdcall CAddin::OnNewMailEx(BSTR bstrEntryID)
{
	__try
	{
		if (!OnNewMailExSEH(bstrEntryID))
		{
			AddLogEntry(L"### Function Failed OnNewMailExSEH", 0, 0, true, SECONDLEVEL);
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CAddin::OnNewMailEx", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name	: OnNewMailExSEH
Description		: Overrided member function which gets called when new mail arrive in outlook mailbox.
Author Name		: Ramkrushna Shelke
S.R. No			: 
Date			: 07th Aug 2014
****************************************************************************/
bool CAddin::OnNewMailExSEH(BSTR bstrEntryID)
{
	OUTPUT_DEBUG_STRING(L">>> In CAddin::OnNewMailExSEH");
	try
	{
		CComPtr<Outlook::_NameSpace> pNameSpace;
		HRESULT	hr = E_FAIL;

		hr = m_spApp->get_Session(&pNameSpace);

		if (FAILED(hr))
		{
			return false;
		}

		if (pNameSpace)
		{
			CComVariant	vOptional(DISP_E_PARAMNOTFOUND, VT_ERROR);
			CComPtr<IDispatch> pItemDisp;
			CComQIPtr<Outlook::_MailItem> pMailItem;

			hr = pNameSpace->GetItemFromID(bstrEntryID, vOptional, &pItemDisp);
			
			if (FAILED(hr))
			{
				return false;
			}

			pMailItem = pItemDisp;

			if (!pMailItem)
			{
				return false;
			}

			OnNewItemSEH(pMailItem);
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CAddin::OnNewItem", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
	Function Name	: OnNewItem
	Description		: Overrided member function which gets called when new 
					  mail arrive in outlook mailbox.
	Author Name		: Ramkrushna Shelke
	S.R. No			: EMAILSCANPLUGIN_0003
	Date			: 07th Aug 2014
****************************************************************************/
void __stdcall CAddin::OnItemChange(IDispatch * item)
{
	__try
	{
		//Check for duplicate update events.
		//if (IsDuplicateUpdateEvent(item))
		//{
		//	return;
		//}
				
		OutputDebugString(L">>> CAddin::OnItemChange");

		OnNewItemSEH(item);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CAddin::OnNewItem", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name	: OnNewItemSEH
Description		: (SEH FUNCTION)Overrided member function which gets called when new mail arrive in outlook mailbox.
Author Name		: Ramkrushna Shelke
S.R. No			: 
Date			: 07th Aug 2014
Modified Date	: 08th July,2015 Neha Gharge, If any mail is already scanned it will not go for scanning again.
****************************************************************************/
void CAddin::OnNewItemSEH(IDispatch * item)
{
	OUTPUT_DEBUG_STRING(L">>> In CAddin::OnNewItemSEH");
	try
	{
		if(!ReadEmailScansettingFromReg())
		{
			return;
		}
		
		if (!m_bUserMoveActionFromSpam)
		{
			//queryinterface was failed to give information . so changes are done 
			// where we sent IDispatched object to initialize spNewMailitem object.
			CComQIPtr<Outlook::_MailItem> spNewMailitem(item);
			if (!spNewMailitem)
			{
				AddLogEntry(L"### spNewMailitem = NULL in CAddin::OnNewItemSEH", 0, 0, true, SECONDLEVEL);
			}

			OutputDebugString(theApp.m_objISpyOutlookComm.GetMailSubject(spNewMailitem));

			//if wardwiz failed to retrive mail body content it will not go further for scanning.
			//Neha Gharge 9/7/2015
			theApp.m_objISpyOutlookComm.m_bstrMailBody = theApp.m_objISpyOutlookComm.GetMailBody(spNewMailitem).AllocSysString();
			//if (theApp.m_objISpyOutlookComm.m_bstrMailBody == L"")
			//{
			//	AddLogEntry(L"### Failed to read body of mail", 0, 0, true, ZEROLEVEL);
			//	return;
			//}

			//CString csMsgBody = theApp.m_objISpyOutlookComm.m_bstrMailBody;
			//AddLogEntry(L">>> body of mail in plain text %s", csMsgBody, 0, true, ZEROLEVEL);
			ProcessMailItem(spNewMailitem);

			//Process here other messages  so that outlook will not hang.
			DWORD dwCount = 0;
			while (dwCount < 4)
			{
				MSG Message;
				while (PeekMessage(&Message, NULL, WM_NULL, WM_NULL, PM_REMOVE))
				{
					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}
				dwCount++;
			}
		}

		m_bUserMoveActionFromSpam = false;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CAddin::OnNewItemSEH", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : OnClickSpamButton
  Description    : Spam Button click event which will move the seleted mail items to spam folder.
  Author Name    : Ramkrushna Shelke
  S.R. No	     : EMAILSCANPLUGIN_0004
  Date           : 07th Aug 2014
****************************************************************************/
void __stdcall CAddin::OnClickSpamButton(IDispatch* Ctrl, VARIANT_BOOL * CancelDefault)
{
	__try
	{
		OnClickSpamButtonSEH(Ctrl, CancelDefault);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CAddin::OnClickSpamButton", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : OnClickSpamButtonSEH
Description    : Extra function to handle crash dump
Author Name    : Ramkrushna Shelke
S.R. No		   : 
Date           : 07th Aug 2014
Modified Date  : Neha Gharge 3 July,2015
			   : Show process dialog for moving maails.
			   : Neha Gharge 27 July,2015
			   : If user click on email which are already in wardwiz spam , it will show a message box
Modification   : Neha Gharge 30 July,2015
			   :Adding into White list and black list accroding to check box
****************************************************************************/
void CAddin::OnClickSpamButtonSEH(IDispatch* Ctrl, VARIANT_BOOL * CancelDefault)
{
	try
	{
		//Selected mail is already spam, then it will show message accordingly
		//Neha Gharge 12 Aug,2015
		DWORD dwAlreadySpamCount = 0x00;
		if(theApp.m_dwDaysLeft == 0)
		{
			theApp.ShowEvaluationExpiredMsg();
			return;
		}

		CComPtr<Outlook::Selection> Selection;
		HRESULT hr = NULL;
		hr =  m_spExplorer->get_Selection(&Selection);
		if (FAILED(hr))
		{
			AddLogEntry(L"### Failed to get selection pointer::OnClickSpamButtonSEH",0,0,true,ZEROLEVEL);
			return;
		}
		if (Selection == NULL)
		{
			AddLogEntry(L"### Failed to get selection pointer is NULL::OnClickSpamButtonSEH", 0, 0, true, ZEROLEVEL);
			return;
		}
		//Commented code as per new requirement of email scan plugin
		//Neha Gharge 28th July,2015

		//CComQIPtr<Outlook::MAPIFolder> spFolder2Move;
		//CComVariant TmpVar(0 + 1);
		//CComPtr<IDispatch> Item;
		//hr = Selection->Item(TmpVar, &Item);
		//CComQIPtr<Outlook::_MailItem> Mail(Item);

		//if (GetWardWizSpamFolderPtr(spFolder2Move, Mail))
		//{
		//	AddLogEntry(L">>> Selected mail is not from WardWiz Spam Folder", 0, 0, true, FIRSTLEVEL);
		//}
		//else
		//{
		//	if (m_SelectedMailFromWardWizSpam)
		//	{
		//		AddLogEntry(L"### Selected mail is from WardWiz Spam Folder", 0, 0, true, FIRSTLEVEL);
		//		return;
		//	}
		//}
		// Display confirmation message box messagebox for move operation
		//Nitin Kolapkar 3 July 2015
		//if (MessageBox(NULL, theApp.m_objLangManager.GetString(L"IDS_EMAIL_OUTLOOK_PLUGIN_SPAM_BTN_CLICK"), theApp.m_objLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDNO)
		//{
		//	return;
		//}
		

		long lCount = 0;
		hr = Selection->get_Count(&lCount);
		if (FAILED(hr))
		{
			AddLogEntry(L"### Failed to get count of selected mail::OnClickSpamButtonSEH", 0, 0, true, ZEROLEVEL);
			return;
		}

		if (lCount == 0)
		{
			AddLogEntry(L">>> No mail available in Inbox to move as a spam::OnClickSpamButtonSEH", 0, 0, true, ZEROLEVEL);
			return;
		}

		CWardWizEmailCustomMsgBox ObjEmailCustomBox;
		ObjEmailCustomBox.m_bClickOnSpamMsgBox = true;
		INT_PTR nResponse = ObjEmailCustomBox.DoModal();
		if (nResponse == IDCANCEL)
		{
			AddLogEntry(L">>> nResponse is CANCEL", 0, 0, true, FIRSTLEVEL);
			return;
		}
		else if (nResponse == IDOK)
		{
			AddLogEntry(L">>> nResponse is OK", 0, 0, true, FIRSTLEVEL);
			if (ObjEmailCustomBox.m_bcheckBoxtick == true)
			{
				m_bKeepFutureMailAsSpam = true;
			}
		}
		//issue No 1285 No need of tokenization Neha Gharge
		if (m_bKeepFutureMailAsSpam)
		{
			if (!SendEmailData2Service(RELOAD_DBENTRIES, SPAMFILTER, L"",L"",L"", true))
			{
				AddLogEntry(L"### Failed to Reload Spam filter entry", 0, 0, true, SECONDLEVEL);
			}
		}
		Sleep(10);
		//Commented code as per new requirement of email scan plugin
		//Neha Gharge 28th July,2015
		// Display a progress bar to show status of moving mail
		//Neha Gharge 3 July 2015
		//m_objPercentageDisplay.m_dwTotalMailCount = (DWORD)lCount;
		//m_hMoveMailProgressDisplayThread = NULL;
		//m_hMoveMailProgressDisplayThread = ::CreateThread(NULL, 0, MoveMailProgressDisplay, (LPVOID) this, 0, NULL);
		//Sleep(200); //Need sleep to launch thread which display dialog of Progress bar

		long lInvalidMailCount = 0;
		for (long lTmp = 0; lTmp < lCount; ++lTmp)
		{
			//if (m_objPercentageDisplay.m_bCancelProcess == true)
			//{
			//	break;
			//}
			//m_objPercentageDisplay.m_dwCurrentMoveMailCount = m_CurrentMoveMailCount = (DWORD)lTmp;
			CComVariant TmpVar(lTmp + 1);
			CComPtr<IDispatch> Item;
			hr = Selection->Item(TmpVar, &Item);
			if (FAILED(hr))
			{
				AddLogEntry(L"### Failed to get selection pointer of Item::OnClickSpamButtonSEH", 0, 0, true, SECONDLEVEL);
				return;
			}
			if (Item == NULL)
			{
				AddLogEntry(L"### Failed to get selection pointer of Item is NULL::OnClickSpamButtonSEH", 0, 0, true, SECONDLEVEL);
				return;
			}

			CComQIPtr<Outlook::_MailItem> Mail(Item);

			//Issue resolved: 0001342
			//Check here with valid mail pointer
			if (Mail == NULL)
			{
				//If selection is only one and selected item is not valid mail item then show message.
				if (lCount == 1 || lInvalidMailCount == lCount - 1)
				{
					MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_SCAN_INVALIDMAIL"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
					return;
				}

				AddLogEntry(L"### Not a valid mail pointer", 0, 0, true, SECONDLEVEL);
				lInvalidMailCount++;
				continue;
			}


			//issue No 1285 No need of tokenization Neha Gharge
			CString csAddEntry(L"");
			//csAddEntry.Format(L"%s#%s#\n", m_objISpyOutlookComm.GetSenderEmailAddress(Mail), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE"));
			if (m_bKeepFutureMailAsSpam)
			{
				if (!SendEmailData2Service(DELETE_EMAIL_ENTRY, SPAMFILTER, m_objISpyOutlookComm.GetSenderEmailAddress(Mail), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE"),L"",true))
				{
					AddLogEntry(L"### OnClickSpamButtonSEH: ADD_EMAIL_ENTRY in SendEmailData2Service", 0, 0, true, SECONDLEVEL);
				}
				Sleep(10);

				//csAddEntry.Format(L"%s#%s#\n", m_objISpyOutlookComm.GetSenderEmailAddress(Mail), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"));

				if (!SendEmailData2Service(ADD_EMAIL_ENTRY, SPAMFILTER, m_objISpyOutlookComm.GetSenderEmailAddress(Mail), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"), L"", true))
				{
					AddLogEntry(L"### OnClickSpamButtonSEH: ADD_EMAIL_ENTRY in SendEmailData2Service", 0, 0, true, SECONDLEVEL);
				}
				Sleep(10);
			}
			//CComQIPtr<Outlook::MAPIFolder> spFolder2Move;
			//if (GetWardWizSpamFolderPtr(spFolder2Move, Mail))
			//{
			//	AddLogEntry(L">>> GetWardWizSpamFolderPtr return true :: OnclickofSpamButton", 0, 0, true, FIRSTLEVEL);
			CString csPrvSub = theApp.m_objISpyOutlookComm.GetMailSubject(Mail);
			if (!IsFullString(csPrvSub))
			{
				CString csSubject = L"[SPAM]" + theApp.m_objISpyOutlookComm.GetMailSubject(Mail);
				theApp.m_objISpyOutlookComm.PutMailSubject(Mail, csSubject);
			}
			else
			{
				dwAlreadySpamCount++;
			}
				//theApp.m_objISpyOutlookComm.MoveMailItem(Mail, spFolder2Move);
			//}
			//else
			//{
			//	AddLogEntry(L">>> GetWardWizSpamFolderPtr return false :: OnclickofSpamButton", 0, 0, true, FIRSTLEVEL);
			//}
		}

		//if (m_objPercentageDisplay.m_dwTotalMailCount != 0)
		//{
		//	m_objPercentageDisplay.EndDialogOFDisplay();
		//}

		//if (m_hMoveMailProgressDisplayThread != NULL)
		//{
		//	SuspendThread(m_hMoveMailProgressDisplayThread);
		//	TerminateThread(m_hMoveMailProgressDisplayThread, 0x00);
		//	m_hMoveMailProgressDisplayThread = NULL;
		//}

		if (m_bKeepFutureMailAsSpam)
		{
			//issue No 1285 No need of tokenization Neha Gharge
			if (!SendEmailData2Service(SAVE_EMAIL_ENTRIES, SPAMFILTER, L"",L"",L"", true))
			{
				AddLogEntry(L"### Failed to save spam filter entry", 0, 0, true, SECONDLEVEL);
			}
			Sleep(10);
		}

		m_objISpySpamFilter.m_bSpamFilterSettingLoaded = false;
		// Display confirmation message box messagebox for move operation
		//Nitin Kolapkar 3 July 2015
		//if (!m_SelectedMailFromWardWizSpam)
		//{
			//Neha Gharge 30/7/2015 Update the UI before Message box
			//Neha gharge
			if (!SendReLoadMessage2UI())
			{
				AddLogEntry(L"### Error in CExtractAttchForScan::AddToVirusScanEmailDB::SendReLoadMessage2UI", 0, 0, true, SECONDLEVEL);
			}

			if (dwAlreadySpamCount == (DWORD)lCount)
			{
				MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_SCAN_ALREADY_SPAM"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
			}
			else
			{
				MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_OUTLOOK_PLUGIN_SPAM_SUCCESS_MSG"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION | MB_OK);
			}
			m_bKeepFutureMailAsSpam = false;
		//}


	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CAddin::OnClickSpamButtonSEH", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : OnClickNotSpamButton
  Description    : Button click event which will move selected mail items from ITinSpam folder to inbox.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0005
  Date           : 07th Aug 2014
 ****************************************************************************/
void __stdcall CAddin::OnClickNotSpamButton(IDispatch* Ctrl, VARIANT_BOOL * CancelDefault)
{
	__try
	{
		OnClickNotSpamButtonSEH(Ctrl, CancelDefault);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CAddin::OnClickNotSpamButton", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
Function Name  : OnClickNotSpamButtonSEH
Description    : Extra function to handle structured exception.
Author Name    : Ramkrushna Shelke
SR.NO		   : EMAILSCANPLUGIN_0005
Date           : 07th Aug 2014
Modification   : 27 June 2015 .Neha Gharge Get Inbox Folder Pointer of particular Mail ID
Modification   : Neha Gharge 30 July,2015
			   :Adding into White list and black list accroding to check box
****************************************************************************/
void CAddin::OnClickNotSpamButtonSEH(IDispatch* Ctrl,VARIANT_BOOL * CancelDefault)
{
	try
	{
		//Selected mail is already not spam, then it will show message accordingly
		//Neha Gharge 12 Aug,2015
		DWORD dwAlreadyNonSpam = 0x00;

		if (theApp.m_dwDaysLeft == 0)
		{
			theApp.ShowEvaluationExpiredMsg();
			return;
		}
		
		//CComQIPtr<Outlook::MAPIFolder> spFolder2Move;
		//Commented code as per new requirement of email scan plugin
		//Neha Gharge 28th July,2015
		//Crash if failed to get inbox folder and not return 
		//Neha Gharge 29/6/2015 
		//if (!GetInboxFolderPointer(spFolder2Move))
		//{
			//failed to get inbox folder
		//	AddLogEntry(L"### Failed to get Inbox pointer",0,0,true,SECONDLEVEL);
			//Not a wardWizspam folder 
			//Added a message box just to know user, he selected wrong folder for not spam.
		//	MessageBox(NULL, theApp.m_objLangManager.GetString(L"IDS_EMAIL_OUTLOOK_PLUGIN_NOT_SPAM"), theApp.m_objLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		//	return;
		//}

		//BSTR bstrInboxParentFolder;
		//spFolder2Move->get_Name(&bstrInboxParentFolder);

		CComPtr<Outlook::Selection> Selection;
		HRESULT hr = NULL;
		hr = m_spExplorer->get_Selection(&Selection);
		if (FAILED(hr))
		{
			AddLogEntry(L"### Failed to get selection pointer::OnClickNotSpamButtonSEH", 0, 0, true, ZEROLEVEL);
			return;
		}
		if (Selection == NULL)
		{
			AddLogEntry(L"### Failed to get selection pointer is NULL::OnClickNotSpamButtonSEH", 0, 0, true, ZEROLEVEL);
			return;
		}

		long lCount = 0;
		hr = Selection->get_Count(&lCount);
		if (FAILED(hr))
		{
			AddLogEntry(L"### Failed to get count of selected mail::OnClickNotSpamButtonSEH", 0, 0, true, ZEROLEVEL);
			return;
		}

		if (lCount == 0)
		{
			AddLogEntry(L">>> No mail available in Inbox to move as a spam::OnClickNotSpamButtonSEH", 0, 0, true, ZEROLEVEL);
			return;
		}

		CWardWizEmailCustomMsgBox ObjEmailCustomBox;
		ObjEmailCustomBox.m_bClickOnSpamMsgBox = false;
		INT_PTR nResponse = ObjEmailCustomBox.DoModal();
		if (nResponse == IDCANCEL)
		{
			AddLogEntry(L">>> nResponse is CANCEL", 0, 0, true, FIRSTLEVEL);
			return;
		}
		else if (nResponse == IDOK)
		{
			AddLogEntry(L">>> nResponse is OK", 0, 0, true, FIRSTLEVEL);
			if (ObjEmailCustomBox.m_bcheckBoxtick == true)
			{
				m_bKeepFutureMailAsNotSpam = true;
			}
		}

		if (m_bKeepFutureMailAsNotSpam)
		{
			//issue No 1285 No need of tokenization Neha Gharge
			if (!SendEmailData2Service(RELOAD_DBENTRIES, SPAMFILTER, L"",L"",L"", true))
			{
				AddLogEntry(L"### Failed to Reload Spam filter entry", 0, 0, true, SECONDLEVEL);
			}
		}
		Sleep(10);
		// Display confirmation message box messagebox for move operation
		//Nitin Kolapkar 3 July 2015
		//if (MessageBox(NULL, theApp.m_objLangManager.GetString(L"IDS_EMAIL_OUTLOOK_PLUGIN_NOT_SPAM_BTN_CLICK"), theApp.m_objLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONQUESTION | MB_YESNO) == IDNO)
		//{
		//	return;
		//}

		// Display a pregress bar to show status of moving mail
		//Neha Gharge 1 July 2015
		//m_objPercentageDisplay.m_dwTotalMailCount = (DWORD)lCount;
		//m_hMoveMailProgressDisplayThread = NULL;
		//m_hMoveMailProgressDisplayThread = ::CreateThread(NULL, 0, MoveMailProgressDisplay, (LPVOID) this, 0, NULL);
		//Sleep(200); //Need sleep to launch thread which display dialog of Progress bar
		//If any user press cancel button, the process of moving mail should get cancelled.
		//Neha Gharge 2 July,2015

		long lInvalidMailCount = 0;
		for (long lTmp = 0; lTmp < lCount; ++lTmp)
		{
			//if (m_objPercentageDisplay.m_bCancelProcess == true)
			//{
			//	break;
			//}
			//m_objPercentageDisplay.m_dwCurrentMoveMailCount =  m_CurrentMoveMailCount = (DWORD)lTmp;
			CComVariant TmpVar(lTmp + 1);
			CComPtr<IDispatch> Item;
			hr = Selection->Item(TmpVar, &Item);
			if (FAILED(hr))
			{
				AddLogEntry(L"### Failed to get selection pointer of Item::OnClickNotSpamButtonSEH", 0, 0, true, ZEROLEVEL);
				return;
			}
			if (Item == NULL)
			{
				AddLogEntry(L"### Failed to get selection pointer of Item is NULL::OnClickNotSpamButtonSEH", 0, 0, true, ZEROLEVEL);
				return;
			}

			CComQIPtr<Outlook::_MailItem> Mail(Item);

			//Issue resolved: 0001342
			//Check here with valid mail pointer
			if (Mail == NULL)
			{
				//If selection is only one and selected item is not valid mail item then show message.
				if (lCount == 1 || lInvalidMailCount == lCount - 1)
				{
					MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_SCAN_INVALIDMAIL"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
					return;
				}

				AddLogEntry(L"### Not a valid mail pointer", 0, 0, true, SECONDLEVEL);
				lInvalidMailCount++;
				continue;
			}

			CString csAddEntry(L"");
			//issue No 1285 No need of tokenization Neha Gharge
			//csAddEntry.Format(L"%s#%s#\n", m_objISpyOutlookComm.GetSenderEmailAddress(Mail), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"));
			if (m_bKeepFutureMailAsNotSpam)
			{
				if (!SendEmailData2Service(DELETE_EMAIL_ENTRY, SPAMFILTER, m_objISpyOutlookComm.GetSenderEmailAddress(Mail), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_BLOCK_TYPE"),L"", true))
				{
					AddLogEntry(L"### OnClickSpamButtonSEH: ADD_EMAIL_ENTRY in SendEmailData2Service", 0, 0, true, SECONDLEVEL);
				}
				Sleep(10);

				//csAddEntry.Format(L"%s#%s#\n", m_objISpyOutlookComm.GetSenderEmailAddress(Mail), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE"));
				if (!SendEmailData2Service(ADD_EMAIL_ENTRY, SPAMFILTER, m_objISpyOutlookComm.GetSenderEmailAddress(Mail), theApp.m_objwardwizLangManager.GetString(L"IDS_STATIC_ALLOW_TYPE"), L"", true))
				{
					AddLogEntry(L"### OnClickSpamButtonSEH: ADD_EMAIL_ENTRY in SendEmailData2Service", 0, 0, true, SECONDLEVEL);
				}
				Sleep(10);
			}

			//	//Need to get here parent for Spam folder
			//if (spFolder2Move != NULL)
			//{
				CString csSubject = theApp.m_objISpyOutlookComm.GetMailSubject(Mail);
				AddLogEntry(L">>> Subject is %s", csSubject, 0, true, FIRSTLEVEL);

				if (IsFullString(csSubject))
				{
					AddLogEntry(L">>> %s contains [SPAM] ", csSubject, 0, true, FIRSTLEVEL);
					csSubject.Replace(L"[SPAM]", L"");
					theApp.m_objISpyOutlookComm.PutMailSubject(Mail, csSubject);
				}
				else
				{
					AddLogEntry(L">>> %s not contains [SPAM] ", csSubject, 0, true, FIRSTLEVEL);
					dwAlreadyNonSpam++;
				}
				m_bUserMoveActionFromSpam = true;
				//theApp.m_objISpyOutlookComm.MoveMailItem(Mail, spFolder2Move);
			//}
		}

		//if (m_objPercentageDisplay.m_dwTotalMailCount != 0)
		//{
		//	m_objPercentageDisplay.EndDialogOFDisplay();
		//}
		//if (m_hMoveMailProgressDisplayThread != NULL)
		//{
		//	SuspendThread(m_hMoveMailProgressDisplayThread);
		//	TerminateThread(m_hMoveMailProgressDisplayThread, 0x00);
		//	m_hMoveMailProgressDisplayThread = NULL;
		//}

		m_objISpySpamFilter.m_bSpamFilterSettingLoaded = false;
		// Display confirmation message box messagebox for move operation
		//Nitin Kolapkar 3 July 2015
		if (m_bKeepFutureMailAsNotSpam)
		{
			//issue No 1285 No need of tokenization Neha Gharge
			if (!SendEmailData2Service(SAVE_EMAIL_ENTRIES, SPAMFILTER, L"",L"",L"", true))
			{
				AddLogEntry(L"### Failed to save spam filter entry", 0, 0, true, SECONDLEVEL);
			}
			Sleep(10);
		}

		//Neha gharge  30/7/2015 Upadte UI before message box
		if (!SendReLoadMessage2UI())
		{
			AddLogEntry(L"### Error in CExtractAttchForScan::AddToVirusScanEmailDB::SendReLoadMessage2UI", 0, 0, true, SECONDLEVEL);
		}

		CString csAlreadyNonSpamCount, csActualSelectedCount;
		csAlreadyNonSpamCount.Format(L"Already NonSpam Count %d", dwAlreadyNonSpam);
		csActualSelectedCount.Format(L"Actual selected count %d", (DWORD)lCount);

		AddLogEntry(L"%s\t%s", csAlreadyNonSpamCount, csActualSelectedCount,true,FIRSTLEVEL);
		if (dwAlreadyNonSpam == (DWORD)lCount)
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_SCAN_ALREADY_NOT_SPAM"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONEXCLAMATION | MB_OK);
		}
		else
		{
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_OUTLOOK_PLUGIN_NOT_SPAM_SUCCESS_MSG"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_ICONINFORMATION | MB_OK);
		}
		m_bKeepFutureMailAsNotSpam = false;



	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CAddin::OnClickNotSpamButton", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : OnSend
  Description    : CommandBar Exported Send Button click handler.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0006
  Date           : 07th Aug 2014
****************************************************************************/
void __stdcall CAddin::OnSend(IDispatch * item, bool cancel)
{
	try
	{
		//TODO: handle here the send mail event.
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CAddin::OnSend", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : CreateWardWizFolder
  Description    : This function Creates WardWizFolder inside inbox folder 
				   of oulook mail client .
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0006
  Date           : 07th Aug 2014
  //Commented code as per new requirement of email scan plugin
  //Neha Gharge 28th July,2015
****************************************************************************/
//void CAddin::CreateWardWizFolder()
//{
//	try
//	{
//		Outlook::_Application *spApplication;
//		spApplication = m_spApp;
//
//		CComPtr <Outlook::_NameSpace> olNs;
//
//		/****************  This I have did Because otherwise ************** 
//		****************** in loop it is Giving me assertion *************/
//		///////////////////////////////////////////////////////////////////
//		Outlook::MAPIFolder *spMapiFolderMove;
//		//Create a folder for each inbox
//		//Neha Gharge 29/6/2015
//
//		CComPtr <Outlook::MAPIFolder> spInboxFolder;  
//
//		//Start a MAPI Session
//		// Taking session for doing activity on outlook
//		spApplication ->get_Session(&olNs);
//
//		//olNs->GetDefaultFolder(olFolderInbox,&spInboxFolder);
//		//spInboxFolder->get_Folders(&spFolderMove);
//		for (int i = 0; i < static_cast<int>(m_spInboxs.size()); i++)
//		{
//			Outlook::_Folders *spFolderMove;
//			m_spInboxs.at(i)->get_Folders(&spFolderMove);
//			
//			CComVariant varFolderType1("IPF.Note");
//			//ISSUE NO:- 52 RY Date :- 21/5/2014
//			BSTR bstr_temp1(OLESTR("WardWizSpam"));  //Name of folder
//			spFolderMove->Add(bstr_temp1, varFolderType1, &spMapiFolderMove);
//		}
//	}
//	catch(...)
//	{
//		AddLogEntry(L"### Exception in CAddin::CreateWardwizFolder", 0, 0, true, SECONDLEVEL);
//	}
//}

/***************************************************************************
  Function Name  : ProcessMailItem
  Description    : Function which processes a single mail item came in inbox
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0010
  Date           : 07th Aug 2014
****************************************************************************/
bool CAddin::ProcessMailItem(CComQIPtr<Outlook::_MailItem> spMailItem)
{
	try
	{
		if(!spMailItem)
			return false;

		if (theApp.m_objISpyOutlookComm.ISMailProceedAlready(spMailItem, WARDWIZBODYPROP))
		{
			return false;
		}

		bool bSpamEmail = false;

		//Check here ON/OFF status of Email scan if OFF no need to scan mail.
		if(!ISEmailScanEnabled())
		{
			return false;
		}

		CString csSubject = theApp.m_objISpyOutlookComm.GetMailSubject(spMailItem);

		AddLogEntry(L">>> Processing Mail: Subject: %s", csSubject, 0, true, FIRSTLEVEL);

		//Issue : Even sender's address is blocked. It comes into virus scan entry
		//Neha Gharge 2 Sept,2015

		if (!theApp.m_objISpyOutlookComm.ISMailProceedAlready(spMailItem, WARDWIZMAILISSPAM))
		{
			AddLogEntry(L"### Failed to identify WardwizMailIsSpam property in mail", 0, 0, true, ZEROLEVEL);
		}
		else
		{
			AddLogEntry(L">>> Succeed  to identify WardwizMailIsSpam property in mail", 0, 0, true, ZEROLEVEL);
			return true;
		}

		DWORD dwSpamReturn = NONE;
		//Issue:1347 : When we get spam mail in the outlook,that entry is reflecting in Virus Scan UI.
		//Resolved by Nitin K. Date: 5th March 2016
		//Code added here coz we need to check each and every mail
		CString csFilePath = GetWardWizPathFromRegistry();
		CString csSecFilePath = csFilePath;
		csSecFilePath += _T("VBMALICIOUSSITES.DB");
		if (m_objISpyContaintFilter.LoadMaliciousSiteFromFile(csSecFilePath))
		{
			DWORD dwMaliciousReturn = NONE;
			dwMaliciousReturn = m_objISpyContaintFilter.FilterSenderAddrBlackDomain(spMailItem);
			if (dwMaliciousReturn == BLOCK)
			{
				AddLogEntry(L">>> FilterSenderAddrBlackDomain ALLOW BLOCK", 0, 0, true, FIRSTLEVEL);
				if (dwMaliciousReturn == BLOCK)
				{
					AddLogEntry(L"### [Spam Mail]:ProcessMailForBlackDomain for sender's address domain  Subject: %s", csSubject, 0, true, FIRSTLEVEL);
				}

				if (!theApp.m_objISpyOutlookComm.PutWardWizPropertyIntoMail(spMailItem, WARDWIZBODYPROP))
				{
					AddLogEntry(L"### Failed to add WardwizSpam property in mail", 0, 0, true, ZEROLEVEL);
				}
				bSpamEmail = true;
			}
		}
		

		//Get registry setting for spam filter scan
		if(GetRegistrySetting(L"dwEnableSpamFilter"))
		{
			if (!theApp.m_objISpyOutlookComm.ISMailProceedAlready(spMailItem, WARDWIZSPAMPROP))
			{
				dwSpamReturn = ProcessMailForSpam(spMailItem);
				if (dwSpamReturn == BLOCK)
				{
					AddLogEntry(L"### [Spam Mail]:ProcessMailForSpam Subject: %s", csSubject, 0, true, FIRSTLEVEL);
					bSpamEmail = true;
					
					//Issue : Even sender's address is blocked. It comes into virus scan entry
					//Neha Gharge 2 Sept,2015
					if (!theApp.m_objISpyOutlookComm.PutWardWizPropertyIntoMail(spMailItem, WARDWIZMAILISSPAM))
					{
						AddLogEntry(L"### Failed to add WardwizMailIsSpam property in mail", 0, 0, true, ZEROLEVEL);
					}
				}

				if (!theApp.m_objISpyOutlookComm.PutWardWizPropertyIntoMail(spMailItem, WARDWIZSPAMPROP))
				{
					AddLogEntry(L"### Failed to add WardwizvSpam property in mail", 0, 0, true, ZEROLEVEL);
				}
			}
		}

		//Ram Shelke
		//Check here the download state
		//if mail is came with only headers so need to parse it here, same function will get called again
		//when the mail will download fully when user tries to open mail in outlook
		Outlook::OlDownloadState olDLState;
		HRESULT hr = spMailItem->get_DownloadState(&olDLState);
		if (FAILED(hr))
		{
			AddLogEntry(L"### Failed to get Download State", 0, 0, true, ZEROLEVEL);
		}

		//Issue : Even sender's address is blocked. It comes into virus scan entry
		//Neha Gharge 2 Sept,2015

		if (olDLState == olFullItem && !bSpamEmail)
		{
			OutputDebugString(L"Mail is not marked as SPAM: ");
			OutputDebugString(csSubject);

			if(dwSpamReturn != ALLOW && !bSpamEmail)
			{
				//Get registry setting content filter scan
				if (GetRegistrySetting(L"dwEnableContentFilter"))
				{
					dwSpamReturn = ProcessMailForContentScan(spMailItem);
					if (dwSpamReturn == BLOCK)
					{
						AddLogEntry(L"### [Spam Mail]:ProcessMailForContentScan Subject: %s", csSubject, 0, true, FIRSTLEVEL);
						bSpamEmail = true;
	
	
	
	
					}
				}
				//Issue 1251 Whn any mail is block then only it should not go for virus scan.
				if (dwSpamReturn != BLOCK && !bSpamEmail)
				{
					//Get registry setting for virus scan
					if (GetRegistrySetting(L"dwEnableVirusScan"))
					{
						if (!theApp.m_objISpyOutlookComm.ISMailProceedAlready(spMailItem, WARDWIZSCANPROP))
	
						{
							if (ScanNewMail(spMailItem))
							{
								if (!theApp.m_objISpyOutlookComm.PutWardWizPropertyIntoMail(spMailItem, WARDWIZSCANPROP))
								{
									AddLogEntry(L"### Failed PutWardwizPropertyIntoMail for Scan Mail Subject: %s", csSubject, 0, true, FIRSTLEVEL);
	
	
	
								}
							}
						}
					}
				}
			}
		}
		//Commented code as per new requirement of email scan plugin
		//Neha Gharge 28th July,2015
		//Get registry setting for Add Email Signature
		//if(GetRegistrySetting(L"dwEnableSignature"))
		//{
		//	if(!AddWardWizMailSignature(spMailItem))
		//	{
		//		AddLogEntry(L"### [%s]: failed to add signature mail item", csSubject, 0, true, FIRSTLEVEL);
		//	}
		//}

		//If any suspicious found move mail to ISPYSPAM folder
		if(bSpamEmail && theApp.m_dwDaysLeft > 0)
		{
			AddLogEntry(L"bSpamEmail == true and m_dwdaysleft >0", 0, 0, true, FIRSTLEVEL);
			//CComQIPtr<Outlook::MAPIFolder> spFolder2Move;
			//if (GetWardWizSpamFolderPtr(spFolder2Move,spMailItem))
			//{
			//	AddLogEntry(L"Mail is moving from inbox to WardWiz pointer", 0, 0, true, FIRSTLEVEL);

			//Check prev string is contain[SPAM] Neha Gharge 30/7/2015
			CString csPrvSub = theApp.m_objISpyOutlookComm.GetMailSubject(spMailItem);
			if (!IsFullString(csPrvSub))
			{
				CString csSubject = L"[SPAM]" + theApp.m_objISpyOutlookComm.GetMailSubject(spMailItem);
				theApp.m_objISpyOutlookComm.PutMailSubject(spMailItem, csSubject);
			}

			//	theApp.m_objISpyOutlookComm.MoveMailItem(spMailItem, spFolder2Move);
			//}
			//else
			//{
			//	AddLogEntry(L"### Failed to get WardWiz Spam pointer", 0, 0, true, FIRSTLEVEL);
			//}
		}

		return true;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CAddin::ProcessMailItem", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name  : GetInboxFolderPtr
Description    : Extra function to create memory dumps
Author Name    : Ramkrushna Shelke
Date           : 07th Aug 2014
****************************************************************************/
bool CAddin::GetInboxFolderPtr(VOLFOLDERSLIST &vInboxFolders)
{
	bool bReturn = false;
	__try
	{
		bReturn = GetInboxFolderPtrSEH(vInboxFolders);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in CAddin::GetInboxFolderPtr", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
  Function Name  : GetInboxFolderPtrSEH
  Description    : Function which get's Inbox folder pointer.
  Author Name    : Ramkrushna Shelke
  Date           : 07th Aug 2014
****************************************************************************/
bool CAddin::GetInboxFolderPtrSEH(VOLFOLDERSLIST &vInboxFolders)
{
	bool bReturn = false;
	try
	{
		CComPtr <Outlook::_NameSpace> olNs;

		CComQIPtr <Outlook::_Folders> spAccountFoldersList;
		CComQIPtr <Outlook::_Folders> spPersonalFolderList;

		// Taking session for doing activity on outlook
		HRESULT hr = m_spApp->get_Session(&olNs);
		if (FAILED(hr) || olNs == NULL)
		{
			return false;
		}

		//Get the folder list of the current session
		hr = olNs->get_Folders(&spAccountFoldersList);
		if (FAILED(hr) || spAccountFoldersList == NULL)
		{
			return false;
		}

		long lAccountsCount = 0;
		hr = spAccountFoldersList->get_Count(&lAccountsCount);
		if (FAILED(hr))
		{
			return false;
		}

		if (lAccountsCount == 0)
		{
			return bReturn;
		}

		CComQIPtr <Outlook::MAPIFolder> spAccountFolder;
		hr = spAccountFoldersList->GetFirst(&spAccountFolder);
		if (FAILED(hr) || spAccountFolder == NULL)
		{
			return false;
		}

		int iCount = 0;
		while (iCount < lAccountsCount)
		{
			BSTR bstrFolderPath1;
			hr = spAccountFolder->get_FolderPath(&bstrFolderPath1);
			if (FAILED(hr) || bstrFolderPath1 == NULL)
			{
				iCount++;
				continue;
			}

			CComQIPtr <Outlook::_Folders> spAccountSubFolder;
			hr = spAccountFolder->get_Folders(&spAccountSubFolder);
			if (FAILED(hr) || spAccountSubFolder == NULL)
			{
				iCount++;
				continue;
			}

			long lCount = 0;
			hr = spAccountSubFolder->get_Count(&lCount);
			if (FAILED(hr))
			{
				iCount++;
				continue;
			}

			CComQIPtr <Outlook::MAPIFolder> spEachInbox;
			hr = spAccountSubFolder->GetFirst(&spEachInbox);
			if (FAILED(hr) || spEachInbox == NULL)
			{
				iCount++;
				continue;
			}

			BSTR bstrFolderPath2;
			hr = spEachInbox->get_FolderPath(&bstrFolderPath2);
			if (FAILED(hr) || bstrFolderPath2 == NULL)
			{
				iCount++;
				continue;
			}

			BSTR bstrCompare(OLESTR("inbox"));
			BSTR bstrInBoxGerman(OLESTR("posteingang"));//German
			BSTR bstrInBoxDutch(OLESTR("postvak in")); //Dutch

			BSTR bstrCompare2(bstrFolderPath2);
			int i = 0;
			while (i < lCount)
			{
				CComQIPtr<Outlook::MAPIFolder> spIBox;
				char* lpszText1 = _com_util::ConvertBSTRToString(bstrCompare);
				char* lpszTxtGerman = _com_util::ConvertBSTRToString(bstrInBoxGerman);
				char* lpszTxtDutch = _com_util::ConvertBSTRToString(bstrInBoxDutch);

				char* lpszText2 = _com_util::ConvertBSTRToString(bstrCompare2);

				char *pFind = strrchr(lpszText2, '\\');
				if (pFind != NULL)
				{
					pFind++;
					lpszText2 = pFind;
				}

				//Ram: Need to make lower case for comparision
				_strlwr(lpszText2);

				if (lpszText2[0] == lpszText1[0] ||
					lpszText2[0] == lpszTxtGerman[0] ||
					lpszText2[0] == lpszTxtDutch[0])
				{
					//Ram: issue resolved as comparing was wrong
					if (_strcmpi(lpszText2, lpszText1) == 0 ||
						_strcmpi(lpszText2, lpszTxtGerman) == 0 ||
						_strcmpi(lpszText2, lpszTxtDutch) == 0)
					{
						AddLogEntry(L">>> Folder %s, matched", bstrCompare2);
						break;
					}
				}

				HRESULT hr = spAccountSubFolder->GetNext(&spIBox);

				if (spIBox == NULL)
				{
					break;
				}

				spIBox->get_Name(&bstrCompare2);
				AddLogEntry(bstrCompare2);
				spEachInbox = spIBox;
				i++;

			}

			if (spEachInbox != NULL)
			{
				vInboxFolders.push_back(spEachInbox);
			}

			CComQIPtr<Outlook::MAPIFolder> spNextFolder;
			HRESULT hr = spAccountFoldersList->GetNext(&spNextFolder);

			if (spNextFolder == NULL)
			{
				break;
			}

			if (FAILED(hr))
			{
				break;
			}
			spAccountFolder = spNextFolder;
			iCount++;
		}
		bReturn = true;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CAddin::GetInboxFolderPtrSEH", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
Function Name  : GetInboxFolderPtr
Description    : Function which get's WardWiz folder pointer from outlook
Author Name    : Ramkrushna Shelke
SR.NO			 : EMAILSCANPLUGIN_0007
Date           : 07th Aug 2014
Modified date	:Neha Gharge 25/7/2015 Mail is not going into wardwiz spam folder 
****************************************************************************/
bool CAddin::GetWardWizSpamFolderPtr(CComQIPtr<Outlook::MAPIFolder> &spFolder2Move, CComQIPtr<Outlook::_MailItem> spMailItem)
{
	bool bReturn = false;
	try
	{
		return GetWardWizSpamFolderPtrSEH(spFolder2Move, spMailItem);
	}
	//__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	catch (...)
	{
		bReturn = false;
		AddLogEntry(L"### Exception in CAddin::GetInboxFolderPtr", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
	
}
/***************************************************************************
  Function Name  : GetInboxFolderPtr
  Description    : Function which get's WardWiz folder pointer from outlook
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0007
  Date           : 07th Aug 2014
  Modified date	: Neha Gharge 25/7/2015 Mail is not going into wardwiz spam folder
****************************************************************************/
bool CAddin::GetWardWizSpamFolderPtrSEH(CComQIPtr<Outlook::MAPIFolder> &spFolder2Move, CComQIPtr<Outlook::_MailItem> spMailItem)
{
	try
	{
		CComPtr <Outlook::_NameSpace> olNs;
		Outlook::_Folders *spFolderMove;
		CComPtr <Outlook::MAPIFolder> spInboxFolder;  
		CComQIPtr<Outlook::MAPIFolder> spFolder2Store;

		//Start a MAPI Session
		Outlook::_Application *spOutlookApp; 
		spOutlookApp = m_spApp; 

		// Taking session for doing activity on outlook
		spOutlookApp->get_Session(&olNs);

		long count = 0;
		//spMapiFolderMove = NULL;

		if (!spMailItem)
		{
			AddLogEntry(L">>> spMailItem pointer in CAddin::GetWardwizSpamFolderPtrSEH is Null",0,0,true,FIRSTLEVEL);
			return false;
		}
			
		spMailItem->get_Parent((IDispatch**)&spInboxFolder);

		BSTR bstrCurrentFolder;
		spInboxFolder->get_Name(&bstrCurrentFolder);

		BSTR bstrCompareCurrentFolder(bstrCurrentFolder);
		char* lpszInboxFolder = _com_util::ConvertBSTRToString(bstrCompareCurrentFolder);

		TCHAR szName[512];
		MultiByteToWideChar(CP_ACP, 0, lpszInboxFolder, -1, szName, sizeof(TCHAR) * 32);
		AddLogEntry(L"mail inbox Folder %s", szName, 0, true, FIRSTLEVEL);

		//olNs->GetDefaultFolder(olFolderInbox, &spInboxFolder);

		spInboxFolder->get_Folders(&spFolderMove);
		spFolderMove->GetFirst(&spFolder2Move);
		spFolder2Store = spFolder2Move;
		spFolderMove->get_Count(&count);
		//ISSUE NO:- 52 RY Date :- 21/5/2014
		BSTR bstrCompare(OLESTR("WardWizSpam"));
		BSTR bstrCompare2;

		char* lpszText1 = _com_util::ConvertBSTRToString(bstrCompare);
		if (_strcmpi(lpszText1, lpszInboxFolder) == 0)
		{
			m_SelectedMailFromWardWizSpam = true;
			MessageBox(NULL, theApp.m_objwardwizLangManager.GetString(L"IDS_EMAIL_SCAN_ALREADY_FROM_SPAM_FOLDER"), theApp.m_objwardwizLangManager.GetString(L"IDS_PRODUCT_NAME"), MB_OK | MB_ICONEXCLAMATION);
			return false;
		}

		if (spFolder2Move == NULL)
		{
			AddLogEntry(L">>> spFolder2Move is NULL",0,0,true,FIRSTLEVEL);
			return false;
		}
		else
		{
			spFolder2Move->get_Name(&bstrCompare2);
		}
		
		char* lpszText2 = _com_util::ConvertBSTRToString(bstrCompare2);
		
		HRESULT hr = NULL;
		int i = 0;
		while( i < count )
		{
			CComQIPtr<Outlook::MAPIFolder> spFolder2MoveNext;
			lpszText2 = _com_util::ConvertBSTRToString(bstrCompare2);

			if(_strcmpi(lpszText1, lpszText2) == 0)
			{
				spFolder2Move = spFolder2Store;
				break;
			}
			HRESULT hr = spFolderMove->GetNext(&spFolder2MoveNext);

			if (FAILED(hr))
			{
				break;
			}

			if (spFolder2MoveNext == NULL)
			{
				break;
			}
			spFolder2MoveNext->get_Name(&bstrCompare2);
			spFolder2Store = spFolder2MoveNext;

			i++;
		}

		if(spFolder2Move != NULL)
		{
			return true;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CAddin::GetWardwizSpamFolderPtrSEH", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name  : ScanNewMailSEH
Description    : (SEH Function)This functions scans mail attachments, if virus found which show popup message to user for action.
Author Name    : Ramkrushna Shelke
SR.NO		   : EMAILSCANPLUGIN_0008
Date           : 
****************************************************************************/
bool CAddin::ScanNewMail(CComQIPtr<Outlook::_MailItem> spMailItem)
{
	try
	{
		m_objExtractScanObj.InitializeVariables() ;

		if( !m_objExtractScanObj.LoadDLLsANDSignatures() )
			return false;

		CString csTempPath = GetISpyTempPath();
		//Path changes to temp folder as driver protection is not exist to it
		m_objExtractScanObj.CreateFolder(csTempPath.GetBuffer()) ;
		csTempPath += L"\\WRDWIZTEMP";
		m_objExtractScanObj.CreateFolder(csTempPath.GetBuffer()) ;

		CString csSubject = theApp.m_objISpyOutlookComm.GetMailSubject(spMailItem);
		CString csSenderEmailAddress = theApp.m_objISpyOutlookComm.GetSenderEmailAddress(spMailItem);

		CComPtr<Outlook::Attachments> objAttachments;
		HRESULT hr = spMailItem->get_Attachments(&objAttachments);
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

		for(long lIndex = 1; lIndex <= lAttachCount; lIndex++)
		{
			CComPtr<Outlook::Attachment> objAttachment;
			VARIANT index;
			index.iVal = (SHORT)lIndex; // value
			index.vt = VT_I2; // type of variant = integer

			if(objAttachments->Item(index, &objAttachment) == S_OK)
			{
				BSTR bstrFileName = ::SysAllocString(L"");

				TCHAR	szZipPath[1024] = {0} ;
				objAttachment->get_FileName( &bstrFileName ) ;
				wsprintf( szZipPath, L"%s\\%s", csTempPath, bstrFileName ) ;
				DeleteFile( szZipPath ) ;
				objAttachment->SaveAsFile(_bstr_t(szZipPath) ) ;
				SysFreeString( bstrFileName ) ;
			}
		}

		if( lAttachCount )
		{
			m_objExtractScanObj.strSenderMailID = csSenderEmailAddress ;
			m_objExtractScanObj.bZipFileModified = false ;
			m_objExtractScanObj.dwRemovedZips = 0x00 ;
			m_objExtractScanObj.csaReAttachZipFiles.RemoveAll() ;
			m_objExtractScanObj.cslReAttachments.RemoveAll() ;
			m_objExtractScanObj.cslRemoveAttachments.RemoveAll() ;
			m_objExtractScanObj.EnumZipFolderForScanning( csTempPath.GetBuffer() ) ;


			if( m_objExtractScanObj.dwRemovedZips )
			{
				AttachDeleteAttachmentFromNewMail( spMailItem ) ;
			}

			if( m_objExtractScanObj.bZipFileModified )
			{
				AttachDeleteAttachmentFromNewMail( spMailItem, false ) ;
			}

			if( m_objExtractScanObj.RemoveAllZipFiles( csTempPath.GetBuffer() ))
			{
				m_objExtractScanObj.RemoveFilesUsingSHFileOperation( csTempPath.GetBuffer() ) ;
				m_objExtractScanObj.RemoveFilesUsingSHFileOperation( csTempPath.GetBuffer() ) ;
				m_objExtractScanObj.CreateFolder( csTempPath.GetBuffer() ) ;
			}
		}

		//Add to ISPYVIRUSSCANEMAIL.db
		if(!m_objExtractScanObj.AddToVirusScanEmailDB(csSenderEmailAddress, csSubject ) )
		{
			AddLogEntry(L"### Failed to AddToVirusScanEmailDB", 0, 0, true, SECONDLEVEL);
		}
		return true;
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CAddin::ScanNewMail", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
  Function Name  : AttachDeleteAttachmentFromNewMail
  Description    : Function which attach/deletes attachments from mail.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0009
  Date           : 07th Aug 2014
****************************************************************************/
bool CAddin::AttachDeleteAttachmentFromNewMail(CComQIPtr<Outlook::_MailItem> pMailItem, bool bDelete )
{
	HRESULT hr = NULL;

	std::vector<DWORD> vDeleteIndex;
	std::vector<DWORD> vRepairIndex;
	
	CComPtr<Outlook::Attachments> objAttachments;
	pMailItem->get_Attachments(&objAttachments);

	long	lAttachCount = 0 ;
	objAttachments->get_Count(&lAttachCount);

	DWORD	AttchIndex = static_cast<DWORD>(lAttachCount) ;

	DWORD dwRemoveCount = 0;
	for(long lIndex = 1; lIndex <= lAttachCount; lIndex++)
	{
		CComPtr<Outlook::Attachment> objAttachment;
		VARIANT index;
		index.iVal = (SHORT)lIndex; // value
		index.vt = VT_I2; // type of variant = integer

		if(objAttachments->Item(index, &objAttachment) == S_OK)
		{
			BSTR bstrFileName = ::SysAllocString(L"");

			CString	szFileName = L"" ;

			objAttachment->get_FileName( &bstrFileName ) ;
			szFileName.Format( L"%s", bstrFileName ) ;
			SysFreeString( bstrFileName ) ;

			if( bDelete )
			{
				if( m_objExtractScanObj.cslRemoveAttachments.Find( szFileName ) )
				{
					vDeleteIndex.push_back(static_cast<DWORD>(lIndex));
				}
			}
			else
			{
				if( m_objExtractScanObj.cslReAttachments.Find( szFileName ) )
				{
					vRepairIndex.push_back(static_cast<DWORD>(lIndex));
				}
			}
		}
	}

	if(bDelete)
	{
		//Remove detected attachments
		DWORD dwRemoveCount = 0;
		for(std::vector<DWORD>::iterator it = vDeleteIndex.begin(); it != vDeleteIndex.end(); ++it)
		{
			DWORD dwIndex = (*it) - dwRemoveCount;
			hr = objAttachments->Remove( dwIndex ) ;
			if(FAILED(hr))
			{
				AddLogEntry(L"### Failed to Remove Attachment file: Attachment index", 0, 0, true, SECONDLEVEL);
			}
			else
			{
				dwRemoveCount++;
			}
		}
	}
	else
	{
		//Remove detected attachments
		DWORD dwRemoveCount = 0;
		for(std::vector<DWORD>::iterator it = vRepairIndex.begin(); it != vRepairIndex.end(); ++it)
		{
			DWORD dwIndex = (*it) - dwRemoveCount;
			hr = objAttachments->Remove( dwIndex ) ;
			if(FAILED(hr))
			{
				AddLogEntry(L"### Failed to Remove Attachment file: Attachment index", 0, 0, true, SECONDLEVEL);
			}
			else
			{
				dwRemoveCount++;
			}
		}

		DWORD dwFileCount = static_cast<DWORD>(m_objExtractScanObj.csaReAttachZipFiles.GetCount());
		for(DWORD dwIndex = 0; dwIndex < dwFileCount; dwIndex++)
		{
			VARIANT		Source ;
			_bstr_t		bstrFileName = m_objExtractScanObj.csaReAttachZipFiles.GetAt( dwIndex ) ;
			Source.vt = VT_BSTR ;
			Source.bstrVal = bstrFileName.copy() ;

			CString	csFileName = CString(bstrFileName.copy());
			int iPos = csFileName.ReverseFind(L'\\');
			csFileName = csFileName.Right(csFileName.GetLength() - iPos);

			VARIANT		DisplayName ;
			_bstr_t		bstrDisplayName = csFileName ;

			DisplayName.vt = VT_BSTR ;
			DisplayName.bstrVal = bstrDisplayName.copy() ;

			VARIANT		Type;
			Type.iVal = (SHORT)olByValue ; // value
			Type.vt = VT_I2; // type of variant = integer

			VARIANT		Position;
			Position.iVal = (SHORT)++AttchIndex ; // value
			Position.vt = VT_I2; // type of variant = integer

			CComPtr<Outlook::Attachment> objNewAttachment;
			hr  = objAttachments->Add(Source, Type, Position, DisplayName, &objNewAttachment ) ;
			if(FAILED(hr))
			{
				AddLogEntry(L"### Failed to attach file: %s", bstrFileName, 0, true, SECONDLEVEL);
			}
		}
	}

	DWORD RetryCount = 2;
	while(RetryCount != 0)
	{
		hr = pMailItem->Save();

		if(SUCCEEDED(hr))
		{
			return true;
		}
		else
		{
			RetryCount--;
			_com_error err(hr);
			AddLogEntry(L"### COM ErrMessage: %s ", err.ErrorMessage(), 0, true, SECONDLEVEL);
		}
	}
	return false;
}

/***************************************************************************
  Function Name  : ProcessMailForSpam
  Description    : Function which process mail for spam.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0010
  Date           : 07th Aug 2014
****************************************************************************/
DWORD CAddin::ProcessMailForSpam(CComQIPtr<Outlook::_MailItem> spMailItem)
{
	DWORD dwReturn = NONE;
	try
	{
		CString csFilePath = GetWardWizPathFromRegistry();
		csFilePath += _T("VBSPAMEMAIL.DB");

		if(!m_objISpySpamFilter.LoadSendersEmailAddressDB(csFilePath))
		{
			return dwReturn;
		}
		
		CISpyOutlookComm objISpyOutlookComm;
		CString csSenderEmail = objISpyOutlookComm.GetSenderEmailAddress(spMailItem);
		
		dwReturn = m_objISpySpamFilter.FilterSenderEmailAddress(csSenderEmail);
	}
	catch(...)
	{
		dwReturn = NONE;
		AddLogEntry(L"### Exception in CAddin::ProcessMailForSpam", 0, 0, true, SECONDLEVEL);
	}
	return dwReturn;
}

/***************************************************************************
  Function Name  : ProcessMailForContentScan
  Description    : Function which process mail to check for content scan.
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0011
  Date           : 07th Aug 2014
****************************************************************************/
DWORD CAddin::ProcessMailForContentScan(CComQIPtr<Outlook::_MailItem> spMailItem)
{
	AddLogEntry(L">>> In CAddin::ProcessMailForContentScan", 0, 0, true, FIRSTLEVEL);
	DWORD dwReturn = NONE;
	try
	{
		CString csFilePath = GetWardWizPathFromRegistry();
		CString csContentEmailPath = csFilePath + _T("VBCONTENTEMAIL.DB");
		if(!m_objISpyContaintFilter.LoadDataContentFromFile(csContentEmailPath))
		{
			AddLogEntry(L">>> Failed m_objWardwizContaintFilter.LoadDataContentFromFile returning..", 0, 0, true, FIRSTLEVEL);
			return dwReturn ;
		}

		/*CString csSecFilePath = csFilePath;
		csSecFilePath += _T("VBMALICIOUSSITES.DB");
		if(!m_objISpyContaintFilter.LoadMaliciousSiteFromFile(csSecFilePath))
		{
			return dwReturn ;
		}*/
		dwReturn = m_objISpyContaintFilter.ScanMail4ContentScan(spMailItem);
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CAddin::ProcessMailForContentScan", 0, 0, true, SECONDLEVEL);
	}
	return dwReturn;
}

/***************************************************************************
  Function Name  : AddWardWizMailSignature
  Description    : Function which adds Email Signatures in mail body.
				   (e.g.) "This mail is scanned by WardWiz."
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0012
  Date           : 07th Aug 2014
****************************************************************************/
bool CAddin::AddWardWizMailSignature(CComQIPtr<Outlook::_MailItem> &spMailItem)
{
	try
	{
		CString csFilePath = GetWardWizPathFromRegistry();
		csFilePath += _T("VBSIGNATUREEMAIL.DB");

		if(!m_objISpyEmailSignature.LoadEmailSignatureFromFile(csFilePath))
		{
			return false;
		}

		if(!m_objISpyEmailSignature.AppendSignatureInMail(spMailItem))
		{
			return false;
		}
	}
	catch(...)
	{
		AddLogEntry(L"### Exception in CAddin::AddWardwizMailSignature", 0, 0, true, SECONDLEVEL);
	}
	return true;
}

/***************************************************************************
  Function Name  : OnDataReceiveCallBack
  Description    : call back function to receive data using pipes
  Author Name    : Ramkrushna Shelke
  SR.NO			 : EMAILSCANPLUGIN_0013
  Date           : 29th May 2014
****************************************************************************/
void CAddin::OnDataReceiveCallBack(LPVOID lpParam)
{
	__try
	{
		LPISPY_PIPE_DATA lpSpyData = (LPISPY_PIPE_DATA)lpParam;
		switch(lpSpyData->iMessageInfo)
		{
			case ENABLE_DISABLE_EMAIL_PLUGIN:
				m_pThis->EnableDisablePlugin(lpSpyData->dwSecondValue);
				break;
			case RELOAD_REGISTARTION_DAYS:
				m_pThis->ReloadRegistartionDays();
				break;
			case RELOAD_EMAIL_DB_SETTINGS:
				//Issue : Need to restart outllook after applying any change on UI.
				//Neha Gharge 25/6/2015 
				m_pThis->ReloadSettings();
				break;
		}
	}
	__except(CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()) , EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CAddin::OnDataReceiveCallBack", 0, 0, true, SECONDLEVEL);
	}
}

/***************************************************************************
  Function Name  : EnableDisablePlugin()
  Description    : Enable or disable outlook plugin from outlook according to setting.
  Author Name    : Neha Gharge
  SR.NO			 : EMAILSCANPLUGIN_0014
  Date           : 30th May 2014
****************************************************************************/
bool CAddin::EnableDisablePlugin(DWORD dwEnableValue)
{
	bool bReturn = false;
	__try
	{
		bReturn = EnableDisablePluginSEH(dwEnableValue);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CAddin::EnableDisablePlugin", 0, 0, true, SECONDLEVEL);
	}
	return bReturn;
}

/***************************************************************************
Function Name  : EnableDisablePlugin()
Description    : (SEH function)Enable or disable outlook plugin from outlook according to setting.
Author Name    : Neha Gharge
SR.NO		   : EMAILSCANPLUGIN_0014
Date           : 30th May 2014
****************************************************************************/
bool CAddin::EnableDisablePluginSEH(DWORD dwEnableValue)
{
	bool bEnable = false;
	if(!dwEnableValue)
	{
		bEnable = false;
	}
	else
	{
		bEnable = true;
	}
	
	if(bEnable)
	{
		spNewCmdBar->PutVisible(VARIANT_TRUE);
		m_spSpamButton->PutVisible(VARIANT_TRUE);
		m_spNotSpamButton->PutVisible(VARIANT_TRUE);
	}
	else
	{
		spNewCmdBar->PutVisible(VARIANT_FALSE);
		m_spSpamButton->PutVisible(VARIANT_FALSE);
		m_spNotSpamButton->PutVisible(VARIANT_FALSE);
	}

	//issue no : 541 : neha Gharge 16/6/2014
	// get the CommandBars interface that represents Outlook's 		//toolbars & menu 
	//items	
	HRESULT hr = m_spExplorer->Display();
	if(FAILED(hr))
			return false;
	
	return true;
}


/***************************************************************************
  Function Name  : ReloadRegistartionDays
  Description    : Reload registration days.
  Author Name    : Neha Gharge
  SR.NO			 : EMAILSCANPLUGIN_0015
  Date           : 30th May 2014
****************************************************************************/
bool CAddin::ReloadRegistartionDays()
{
	theApp.ReloadNoofdaysLeft();
	return true;
}

/***************************************************************************
  Function Name  : ReloadSettings
  Description    : This function will reload the saved settings.
  Author Name    : Ram Shelke
  SR.NO			 : EMAILSCANPLUGIN_0016
  Date           : 31st May 2014
****************************************************************************/
void CAddin::ReloadSettings()
{
	m_objISpyContaintFilter.m_bMaliciousDBFileLoaded = false;
	m_objISpyContaintFilter.m_bContentDBFileLoaded = false;
	m_objExtractScanObj.m_bSignaturesLoaded  = false;
	m_objISpySpamFilter.m_bSpamFilterSettingLoaded = false;
	m_objISpyEmailSignature.m_bEmailSignatureLoaded = false;
}

/***************************************************************************
  Function Name  : ReadEmailScansettingFromReg()
  Description    : Read EmailScan Entry from registry.
  Author Name    : Neha Gharge
  SR.NO			 : EMAILSCANPLUGIN_0017
  Date           : 31st May 2014
****************************************************************************/
bool CAddin::ReadEmailScansettingFromReg()
{
	HKEY hKey;
	LONG ReadReg;
	DWORD dwvalueSType;
	DWORD dwvalueSize = sizeof(DWORD);
	DWORD ChkvalueForEmailScan;
	DWORD dwType=REG_DWORD;
	bool  bEnableEmailScan = false;
	
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Vibranium"), 0, KEY_QUERY_VALUE | KEY_WOW64_64KEY, &hKey) != ERROR_SUCCESS)
	{
		return false;
	}

	ReadReg=RegQueryValueEx(hKey,L"dwEmailScan",NULL,&dwType,(LPBYTE)&dwvalueSType,&dwvalueSize);
	ChkvalueForEmailScan=(DWORD)dwvalueSType;
	if(ChkvalueForEmailScan==0)
	{
		bEnableEmailScan = false;
	}
	else
	{
		bEnableEmailScan = true;
	}
	RegCloseKey(hKey);

	return bEnableEmailScan;
}

/***************************************************************************
Function Name  : DispEventsAdvises
Description    : Register inbox items for events
Author Name    : Ram Shelke
SR.NO		   : 
Date           : 27th Jun 2015
****************************************************************************/
bool CAddin::DispEventsAdvises(int iInboxItemNo)
{
	bool bReturn = true;
	try
	{
		if (m_spInboxItems.size() == 0 && iInboxItemNo >= static_cast<int>(m_spInboxItems.size()))
		{
			AddLogEntry(L">>> No any Inbox folder found in DispEventsAdvises");
			return false;
		}

		HRESULT hr;
		CComPtr<Outlook::_Items> spInboxItem = m_spInboxItems.at(iInboxItemNo);

		switch (iInboxItemNo)
		{
		case 0:
			hr = NewItemEvents::DispEventAdvise((IDispatch*)spInboxItem);
			break;
		case 1:
			hr = NewItemEvents1::DispEventAdvise((IDispatch*)spInboxItem);
			break;
		case 2:
			hr = NewItemEvents2::DispEventAdvise((IDispatch*)spInboxItem);
			break;
		case 3:
			hr = NewItemEvents3::DispEventAdvise((IDispatch*)spInboxItem);
			break;
		case 4:
			hr = NewItemEvents4::DispEventAdvise((IDispatch*)spInboxItem);
			break;
		case 5:
			hr = NewItemEvents5::DispEventAdvise((IDispatch*)spInboxItem);
			break;
		case 6:
			hr = NewItemEvents6::DispEventAdvise((IDispatch*)spInboxItem);
			break;
		case 7:
			hr = NewItemEvents7::DispEventAdvise((IDispatch*)spInboxItem);
			break;
		case 8:
			hr = NewItemEvents8::DispEventAdvise((IDispatch*)spInboxItem);
			break;
		case 9:
			hr = NewItemEvents9::DispEventAdvise((IDispatch*)spInboxItem);
			break;
		}

		if (FAILED(hr))
		{
			bReturn = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CAddin::DispEventsAdvises", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************
Function Name  : DispEventsUnAdvises
Description    : Un Register inbox items from events
Author Name    : Ram Shelke
SR.NO		   :
Date           : 27th Jun 2015
****************************************************************************/
bool CAddin::DispEventsUnAdvises(int iInboxItemNo)
{
	bool bReturn = true;
	try
	{
		if (m_spInboxItems.size() == 0 && iInboxItemNo >= static_cast<int> (m_spInboxItems.size()))
		{
			return false;
		}

		HRESULT hr;
		CComPtr<Outlook::_Items> spInboxItem = m_spInboxItems.at(iInboxItemNo);

		switch (iInboxItemNo)
		{
		case 0:
			hr = NewItemEvents::DispEventUnadvise((IDispatch*)spInboxItem);
			break;
		case 1:
			hr = NewItemEvents1::DispEventUnadvise((IDispatch*)spInboxItem);
			break;
		case 2:
			hr = NewItemEvents2::DispEventUnadvise((IDispatch*)spInboxItem);
			break;
		case 3:
			hr = NewItemEvents3::DispEventUnadvise((IDispatch*)spInboxItem);
			break;
		case 4:
			hr = NewItemEvents4::DispEventUnadvise((IDispatch*)spInboxItem);
			break;
		case 5:
			hr = NewItemEvents5::DispEventUnadvise((IDispatch*)spInboxItem);
			break;
		case 6:
			hr = NewItemEvents6::DispEventUnadvise((IDispatch*)spInboxItem);
			break;
		case 7:
			hr = NewItemEvents7::DispEventUnadvise((IDispatch*)spInboxItem);
			break;
		case 8:
			hr = NewItemEvents8::DispEventUnadvise((IDispatch*)spInboxItem);
			break;
		case 9:
			hr = NewItemEvents9::DispEventUnadvise((IDispatch*)spInboxItem);
			break;
		}

		if (FAILED(hr))
		{
			bReturn = false;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CAddin::DispEventsAdvises", 0, 0, true, SECONDLEVEL);
		bReturn = false;
	}
	return bReturn;
}

/***************************************************************************
Function Name  : GetInboxFolderPointer
Description    : Get particular inbox pointer
Author Name    : Neha Gharge
SR.NO		   :
Date           : 27th Jun 2015
****************************************************************************/
bool CAddin::GetInboxFolderPointer(CComQIPtr<Outlook::MAPIFolder> &spFolder2Move)
{
	try
	{
		BSTR bstrCompareWrdWIZSpam(OLESTR("WardWizSpam"));
		char* lpszWardWizpamFolder = _com_util::ConvertBSTRToString(bstrCompareWrdWIZSpam);

		CComPtr<Outlook::MAPIFolder> spCurrentFolder;
		m_spExplorer->get_CurrentFolder(&spCurrentFolder);

		BSTR bstrCurrentFolder;
		spCurrentFolder->get_Name(&bstrCurrentFolder);

		BSTR bstrCompareCurrentFolder(bstrCurrentFolder);
		char* lpszCurrentFolder = _com_util::ConvertBSTRToString(bstrCompareCurrentFolder);

		if (_strcmpi(lpszWardWizpamFolder, lpszCurrentFolder) != 0)
		{
			//Not a wardWizspam folder
			return false;
		}

		CComPtr<Outlook::MAPIFolder> spCurrentFolderParent;
		//spCurrentFolder->get_Parent();
		spCurrentFolder->get_Parent((IDispatch**)&spCurrentFolderParent);

		BSTR bstrCurrentFolderParent;
		spCurrentFolderParent->get_Name(&bstrCurrentFolderParent);

		BSTR bstrCompare(bstrCurrentFolderParent);
		char* lpszCurrentFolderParent = _com_util::ConvertBSTRToString(bstrCompare);

		CComPtr<Outlook::MAPIFolder> spCurrentFolderInboxParent;
		//spCurrentFolder->get_Parent();
		spCurrentFolderParent->get_Parent((IDispatch**)&spCurrentFolderInboxParent);

		BSTR bstrCurrentInboxFolderParent;
		spCurrentFolderInboxParent->get_Name(&bstrCurrentInboxFolderParent);

		BSTR bstrCompareCurrentInboxFolderParent(bstrCurrentInboxFolderParent);
		char* lpszCurrentInboxFolderParent = _com_util::ConvertBSTRToString(bstrCompareCurrentInboxFolderParent);

		CComQIPtr<Outlook::MAPIFolder> spFolder;

		for (int i = 0; i < static_cast<int>(m_spInboxs.size()); i++)
		{
			CComPtr<Outlook::MAPIFolder> spInboxParent;
			m_spInboxs.at(i)->get_Parent((IDispatch**)&spInboxParent);

			BSTR bstrInboxParentFolder;
			spInboxParent->get_Name(&bstrInboxParentFolder);

			BSTR bstrCompare2(bstrInboxParentFolder);
			char* lpszInboxFolderParent = _com_util::ConvertBSTRToString(bstrCompare2);

			if (_strcmpi(lpszCurrentInboxFolderParent, lpszInboxFolderParent) == 0)
			{
				spFolder = m_spInboxs.at(i);
				break;
			}
		}

		if (spFolder != NULL)
		{
			spFolder2Move = spFolder;
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in  CAddin::GetInboxFolderPointer", 0, 0, true, SECONDLEVEL);
	}
	return false;
}

/***************************************************************************
Function Name  : MoveMailProgressDisplay
Description    : DO modal the dialog of progress bar
Author Name    : Neha Gharge
SR.NO		   :
Date           : 1St July 2015
****************************************************************************/
DWORD WINAPI MoveMailProgressDisplay(LPVOID lpParam)
{
	__try
	{
		CAddin *pThis = (CAddin *)lpParam;
		if (!pThis)
			return 0;

		if (pThis->m_objPercentageDisplay.m_dwTotalMailCount != 0)
		{
			AddLogEntry(L">>> Inside MoveMailProgressDisplay thread", 0, 0, true, FIRSTLEVEL);
			if (pThis->m_objPercentageDisplay.m_hWnd == NULL)
			{
				AddLogEntry(L">>> Handle is null for DoModal", 0, 0, true, FIRSTLEVEL);
				pThis->m_objPercentageDisplay.DoModal();
			}
		}
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CAddin::MoveMailProgressDisplay thread", 0, 0, true, SECONDLEVEL);
		return 0;
	}
	return 1;
}


/***********************************************************************************************
Function Name  : SendEmailData2Service
Description    : Function to send data to service
dwAddEditDelType : ADD_EMAIL_ENTRY,EDIT_EMAIL_ENTRY,DELETE_EMAIL_ENTRY
csEntry			:No tokenization  String
Author Name    : Neha Gharge
Date           : 2 Feb 2016.
//issue No 1285 No need of tokenization Neha Gharge
***********************************************************************************************/
bool CAddin::SendEmailData2Service(DWORD dwAddEditDelType, DWORD dwType, CString csFirstParam, CString csSecondParam, CString csThirdParam, bool bEmailScanWait)
{
	try
	{
		ISPY_PIPE_DATA szPipeData = { 0 };
		szPipeData.iMessageInfo = static_cast<int>(dwAddEditDelType);
		szPipeData.dwValue = dwType;
		_tcscpy(szPipeData.szFirstParam, csFirstParam);
		_tcscpy(szPipeData.szSecondParam, csSecondParam);
		_tcscpy(szPipeData.szThirdParam, csThirdParam);

		CISpyCommunicator objCom(SERVICE_SERVER, true);
		if (!objCom.SendData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
		{
			AddLogEntry(L"### Failed to send RESUME_SCAN data", 0, 0, true, SECONDLEVEL);
			return false;
		}
		if (bEmailScanWait)
		{
			if (!objCom.ReadData(&szPipeData, sizeof(ISPY_PIPE_DATA)))
			{
				AddLogEntry(L"### Failed to read data in CWardwizLiveUpSecondDlg::SendLiveUpdateOperation2Service", 0, 0, true, SECONDLEVEL);
				return false;
			}

			if (szPipeData.dwValue != 1)
			{
				return false;
			}
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CAddin::SendEmailData2Service", 0, 0, true, SECONDLEVEL);
	
	}
	return true;
}


/***************************************************************************
Function Name  : SendReLoadMessage2UI
Description    : Function which Sends reload message to UI, once DB file get
updated from outlook.
Author Name    : Ramkrushna Shelke
Date           : 07th Aug 2014
****************************************************************************/
bool CAddin::SendReLoadMessage2UI()
{
	__try
	{
		HWND hWindow = FindWindow(NULL, L"WRDWIZAVUI");
		SendMessage(hWindow, RELOAD_EMAILVIRUSCANDB, 0, 0);
	}
	__except (CWardWizDumpCreater::CreateMiniDump(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
		AddLogEntry(L"### Exception in CExtractAttchForScan::SendReLoadMessage2UI", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return true;
}

/***************************************************************************
Function Name  : IsDuplicateUpdateEvent
Description    : Check for is duplicate update event call.
Author Name    : Ramkrushna Shelke
Date           : 30th Jul 2015
****************************************************************************/
bool CAddin::IsDuplicateUpdateEvent(IDispatch * item)
{
	try
	{
		CComQIPtr<Outlook::_MailItem> spNewMailitem(item);
		if (!spNewMailitem)
		{
			return false;
		}

		DATE dtItemCreation = 0.1;
		HRESULT hr = spNewMailitem->get_CreationTime(&dtItemCreation);
		if (FAILED(hr))
		{
			return false;
		}

		if (g_dtPreviousItemCreation == dtItemCreation)
		{
			OUTPUT_DEBUG_STRING(L">>> IsDuplicateUpdateEvent == true");
			return true;
		}
		OUTPUT_DEBUG_STRING(L">>> IsDuplicateUpdateEvent == false");
		g_dtPreviousItemCreation = dtItemCreation;
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in CAddin::IsDuplicateUpdateEvent", 0, 0, true, SECONDLEVEL);
		return false;
	}
	return false;
}

/**********************************************************************************************************
*  Function Name  :	IsFullString
*  Description    :	Check for full string is completed or not by checking [SPAM] text.
*  Author Name    : Neha Gharge
*  SR_NO		  :
*  Date           : 30 July,2015
**********************************************************************************************************/
bool CAddin::IsFullString(CString csInputPath)
{
	try
	{
		if (((csInputPath.Find(L"[") != -1) && (csInputPath.Find(L"]") != -1)) &&
			(csInputPath.Find(L"SPAM") > 0))
		{
			return true;
		}
	}
	catch (...)
	{
		AddLogEntry(L"### Exception in void CAddin::IsFullString", 0, 0, true, SECONDLEVEL);
	}
	return false;
}