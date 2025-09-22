// Addin.h : Declaration of the CAddin
#pragma once

#ifndef __ADDIN_H_
#define __ADDIN_H_

#include "resource.h"
#include <objbase.h>
#include <initguid.h>
#include "OutlookAddinApp.h"
#include "ITinRegWrapper.h"
#include "ISpyCommServer.h"
#include "PipeConstants.h"
#include "WrdWizPercentageDisplayDlg.h"
//#include "AttachmentTracker_i.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

#define INITGUID
#define USES_IID_IMAPIAdviseSink

#include <windows.h>
//#include <mapiutil.h>
#include "ISpyOutlookComm.h"
#include "ExtractScan.h"
#include "ISpyContaintFilter.h"
#include "ISpySpamFilter.h"
#include "ISpyEmailSignature.h"

#import "MSADDNDR.DLL" raw_interfaces_only, raw_native_types, no_namespace, named_guids 
//#import "C:\Program Files\Common Files\Designer\MSADDNDR.DLL" raw_interfaces_only, raw_native_types, no_namespace, named_guids 

/////////////////////////////////////////////////////////////////////////////
// CAddin
extern _ATL_FUNC_INFO OnClickButtonInfo;
extern _ATL_FUNC_INFO OnSendInfo;
extern _ATL_FUNC_INFO OnNewMail;
extern _ATL_FUNC_INFO OnNewItemInfo;

typedef std::vector<CComQIPtr<Outlook::MAPIFolder>>		VOLFOLDERSLIST;
typedef std::vector<CComPtr<Outlook::_Items>>			VOLITEMSLIST;

using namespace ATL;
class CAddin;

class ATL_NO_VTABLE CAddin : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CAddin, &CLSID_Addin>,
	public IDispatchImpl<IAddin, &IID_IAddin, &LIBID_OUTLOOKADDINLib>,
	public IDispatchImpl<_IDTExtensibility2, &IID__IDTExtensibility2, &LIBID_AddInDesignerObjects>,
	public IDispEventSimpleImpl<1,CAddin, &__uuidof(Office::_CommandBarButtonEvents)>,
	public IDispEventSimpleImpl<2,CAddin, &__uuidof(Office::_CommandBarButtonEvents)>,
	public IDispEventSimpleImpl<1,CAddin, &__uuidof(Outlook::ItemsEvents)>,
	public IDispEventSimpleImpl<1, CAddin, &__uuidof(Outlook::ApplicationEvents)>,	
	public IDispEventSimpleImpl<3, CAddin, &__uuidof(Outlook::ApplicationEvents)>,
	public IDispEventSimpleImpl<17, CAddin, &__uuidof(Outlook::ApplicationEvents_11)>,
	public IDispEventSimpleImpl<4, CAddin, &__uuidof(Outlook::ItemsEvents)>,
	public IDispEventSimpleImpl<5, CAddin, &__uuidof(Outlook::ItemsEvents)>,
	public IDispEventSimpleImpl<6, CAddin, &__uuidof(Outlook::ItemsEvents)>,
	public IDispEventSimpleImpl<7, CAddin, &__uuidof(Outlook::ItemsEvents)>,
	public IDispEventSimpleImpl<8, CAddin, &__uuidof(Outlook::ItemsEvents)>,
	public IDispEventSimpleImpl<9, CAddin, &__uuidof(Outlook::ItemsEvents)>,
	public IDispEventSimpleImpl<10, CAddin, &__uuidof(Outlook::ItemsEvents)>,
	public IDispEventSimpleImpl<11, CAddin, &__uuidof(Outlook::ItemsEvents)>,
	public IDispEventSimpleImpl<12, CAddin, &__uuidof(Outlook::ItemsEvents)>,
	public IDispEventSimpleImpl<13, CAddin, &__uuidof(Outlook::ItemsEvents)>
{
public:
	typedef IDispEventSimpleImpl</*nID =*/ 1,CAddin, &__uuidof(Office::_CommandBarButtonEvents)> CommandSpamButtonEvents;
	typedef IDispEventSimpleImpl</*nID =*/ 2,CAddin, &__uuidof(Office::_CommandBarButtonEvents)> CommandNotSpamButtonEvents;
	typedef IDispEventSimpleImpl<1 /*N*/, CAddin,  &__uuidof(Outlook::ItemsEvents)> MyItemEvents;
	typedef IDispEventSimpleImpl<4, CAddin, &__uuidof(Outlook::ItemsEvents)> NewItemEvents;
	typedef IDispEventSimpleImpl<5, CAddin, &__uuidof(Outlook::ItemsEvents)> NewItemEvents1;
	typedef IDispEventSimpleImpl<6, CAddin, &__uuidof(Outlook::ItemsEvents)> NewItemEvents2;
	typedef IDispEventSimpleImpl<7, CAddin, &__uuidof(Outlook::ItemsEvents)> NewItemEvents3;
	typedef IDispEventSimpleImpl<8, CAddin, &__uuidof(Outlook::ItemsEvents)> NewItemEvents4;
	typedef IDispEventSimpleImpl<9, CAddin, &__uuidof(Outlook::ItemsEvents)> NewItemEvents5;
	typedef IDispEventSimpleImpl<10, CAddin, &__uuidof(Outlook::ItemsEvents)> NewItemEvents6;
	typedef IDispEventSimpleImpl<11, CAddin, &__uuidof(Outlook::ItemsEvents)> NewItemEvents7;
	typedef IDispEventSimpleImpl<12, CAddin, &__uuidof(Outlook::ItemsEvents)> NewItemEvents8;
	typedef IDispEventSimpleImpl<13, CAddin, &__uuidof(Outlook::ItemsEvents)> NewItemEvents9;
	typedef IDispEventSimpleImpl< 17, CAddin, &__uuidof(Outlook::ApplicationEvents_11)> AppEvents_NewMail;

	CAddin();
	~CAddin();
	
DECLARE_REGISTRY_RESOURCEID(IDR_ADDIN)

BEGIN_COM_MAP(CAddin)
	COM_INTERFACE_ENTRY(IAddin)
	COM_INTERFACE_ENTRY2(IDispatch, IAddin)
	COM_INTERFACE_ENTRY2(IDispatch, _IDTExtensibility2)
	COM_INTERFACE_ENTRY(_IDTExtensibility2)
END_COM_MAP()

BEGIN_SINK_MAP(CAddin)
	SINK_ENTRY_INFO(1, __uuidof(Office::_CommandBarButtonEvents),/*dispid*/ 0x01, OnClickSpamButton, &OnClickButtonInfo)
	SINK_ENTRY_INFO(2, __uuidof(Office::_CommandBarButtonEvents),/*dispid*/ 0x01, OnClickNotSpamButton, &OnClickButtonInfo)
	SINK_ENTRY_INFO(1,__uuidof(Outlook::ApplicationEvents),/*dispinterface*/0x0000F002, OnSend, &OnSendInfo)
	SINK_ENTRY_INFO(17, __uuidof(Outlook::ApplicationEvents_11), 0xfab5, OnNewMailEx, &OnNewMail)
	SINK_ENTRY_INFO(4, __uuidof(Outlook::ItemsEvents), 0x0000f002, OnItemChange, &OnNewItemInfo)
	SINK_ENTRY_INFO(5, __uuidof(Outlook::ItemsEvents), 0x0000f002, OnItemChange, &OnNewItemInfo)
	SINK_ENTRY_INFO(6, __uuidof(Outlook::ItemsEvents), 0x0000f002, OnItemChange, &OnNewItemInfo)
	SINK_ENTRY_INFO(7, __uuidof(Outlook::ItemsEvents), 0x0000f002, OnItemChange, &OnNewItemInfo)
	SINK_ENTRY_INFO(8, __uuidof(Outlook::ItemsEvents), 0x0000f002, OnItemChange, &OnNewItemInfo)
	SINK_ENTRY_INFO(9, __uuidof(Outlook::ItemsEvents), 0x0000f002, OnItemChange, &OnNewItemInfo)
	SINK_ENTRY_INFO(10, __uuidof(Outlook::ItemsEvents), 0x0000f002, OnItemChange, &OnNewItemInfo)
	SINK_ENTRY_INFO(11, __uuidof(Outlook::ItemsEvents), 0x0000f002, OnItemChange, &OnNewItemInfo)
	SINK_ENTRY_INFO(12, __uuidof(Outlook::ItemsEvents), 0x0000f002, OnItemChange, &OnNewItemInfo)
	SINK_ENTRY_INFO(13, __uuidof(Outlook::ItemsEvents), 0x0000f002, OnItemChange, &OnNewItemInfo)
END_SINK_MAP()

DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
	void __stdcall OnItemChange(IDispatch * Item);
	void __stdcall OnNewMailEx(BSTR bstrEntryID);
	void OnNewItemSEH(IDispatch * item);
	void __stdcall OnSend(IDispatch * item, bool cancel);
	void __stdcall OnClickSpamButton(IDispatch * /*Office::_CommandBarButton**/ Ctrl,VARIANT_BOOL * CancelDefault);
	void OnClickSpamButtonSEH(IDispatch* Ctrl, VARIANT_BOOL * CancelDefault);
	void __stdcall OnClickNotSpamButton(IDispatch * /*Office::_CommandBarButton**/ Ctrl,VARIANT_BOOL * CancelDefault);
	void OnClickNotSpamButtonSEH(IDispatch* Ctrl, VARIANT_BOOL * CancelDefault);
	bool OnNewMailExSEH(BSTR bstrEntryID);
public:
	typedef IDispEventSimpleImpl</*nID =*/ 1,CAddin, &__uuidof(Outlook::ApplicationEvents)> AppEvents;
	typedef IDispEventSimpleImpl</*nID =*/ 2,CAddin, &__uuidof(Outlook::ApplicationEvents)> AppEvents_New;
	
	HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease()
    {
    }

public:
	// _IDTExtensibility2
	STDMETHOD(OnConnection)(IDispatch * Application, ext_ConnectMode ConnectMode, IDispatch * AddInInst, SAFEARRAY * * custom)
	{
		//QI() for _Application
		CComQIPtr <Outlook::_Application> spApp(Application); 
		ATLASSERT(spApp);
		
		m_spApp = spApp;
	
		spApp->ActiveExplorer(&m_spExplorer);

		// get the CommandBars interface that represents Outlook's 		//toolbars & menu 
		//items	
		HRESULT hr = m_spExplorer->get_CommandBars(&spCmdBars);
		if(FAILED(hr))
			return hr;

		ATLASSERT(spCmdBars);
	
		CComVariant vName("WardwizEmailPlugin");
		//CComPtr <Office::CommandBar> spNewCmdBar;
		
		CComVariant vPos(1); 

		CComVariant vTemp(VARIANT_TRUE); 		
		CComVariant vEmpty(DISP_E_PARAMNOTFOUND, VT_ERROR);			
	
		spNewCmdBar = spCmdBars->Add(vName, vPos, vEmpty, vTemp);

		CComPtr < Office::CommandBarControls> spBarControls;
		spBarControls = spNewCmdBar->GetControls();
		ATLASSERT(spBarControls);
		
		//MsoControlType::msoControlButton = 1
		CComVariant vToolBarType(1);
		//show the toolbar?
		CComVariant vShow(VARIANT_TRUE);
		
		CComPtr < Office::CommandBarControl> spNewBar; 
		CComPtr < Office::CommandBarControl> spNewBar2; 
						
		// add first button
		spNewBar = spBarControls->Add(vToolBarType, vEmpty, vEmpty, vEmpty, vShow); 
		ATLASSERT(spNewBar);
				
		spNewBar2 = spBarControls->Add(vToolBarType,vEmpty,vEmpty,vEmpty,vShow);
		ATLASSERT(spNewBar2);

		_bstr_t bstrNewCaption(OLESTR("Spam"));
		_bstr_t bstrTipText(OLESTR("Move mail to spam"));

		// get CommandBarButton interface for each toolbar button
		// so we can specify button styles and stuff
		// each button displays a bitmap and caption next to it
		CComQIPtr < Office::_CommandBarButton> spCmdButton(spNewBar);
		CComQIPtr < Office::_CommandBarButton> spCmdButton2(spNewBar2);
						
		ATLASSERT(spCmdButton);
		ATLASSERT(spCmdButton2);
						
		// to set a bitmap to a button, load a 32x32 bitmap
		// and copy it to clipboard. Call CommandBarButton's PasteFace()
		// to copy the bitmap to the button face. to use
		// Outlook's set of predefined bitmap, set button's FaceId to     //the
		// button whose bitmap you want to use
		HBITMAP hBmp =(HBITMAP)::LoadImage(_AtlBaseModule.m_hInst,
			MAKEINTRESOURCE(IDB_BITMAP1),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS);

		// put bitmap into Clipboard
		::OpenClipboard(NULL);
		::EmptyClipboard();
		::SetClipboardData(CF_BITMAP, (HANDLE)hBmp);
		::CloseClipboard();
		::DeleteObject(hBmp);        
		// set style before setting bitmap
		spCmdButton->PutStyle(Office::msoButtonIconAndCaption);

		hr = spCmdButton->PasteFace();
		if (FAILED(hr))
			return hr;

		spCmdButton->PutVisible(VARIANT_TRUE);
		// Button Name changed for Spam and not Spam
		//Nitin Kolapkar 3 July 2015
		spCmdButton->PutCaption(OLESTR("WardWiz Spam")); 
		spCmdButton->PutEnabled(VARIANT_TRUE);
		spCmdButton->PutTooltipText(OLESTR("Move mail to spam")); 
		spCmdButton->PutTag(OLESTR("Tag for Item1")); 
		
		//show the toolband
		spNewCmdBar->PutVisible(VARIANT_TRUE); 
					
		spCmdButton2->PutStyle(Office::msoButtonIconAndCaption);

		hBmp =(HBITMAP)::LoadImage(_AtlBaseModule.m_hInst,
			MAKEINTRESOURCE(IDB_BITMAP2),IMAGE_BITMAP,0,0,LR_LOADMAP3DCOLORS);

		// put bitmap into Clipboard
		::OpenClipboard(NULL);
		::EmptyClipboard();
		::SetClipboardData(CF_BITMAP, (HANDLE)hBmp);
		::CloseClipboard();
		::DeleteObject(hBmp);

		// set style before setting bitmap
		spCmdButton->PutStyle(Office::msoButtonIconAndCaption);

		hr = spCmdButton2->PasteFace();
		if (FAILED(hr))
			return hr;

		spCmdButton2->PutVisible(VARIANT_TRUE);
		// Button Name changed for Spam and not Spam
		//Nitin Kolapkar 3 July 2015
		spCmdButton2->PutCaption(OLESTR("WardWiz Not Spam")); 
		spCmdButton2->PutEnabled(VARIANT_TRUE);
		spCmdButton2->PutTooltipText(OLESTR("Move mail to Inbox folder")); 
		spCmdButton2->PutTag(OLESTR("WardWiz Not Spam"));
		spCmdButton2->PutVisible(VARIANT_TRUE);

		m_spSpamButton = spCmdButton;
		m_spNotSpamButton = spCmdButton2;

		/*===============================================*/
		hr = CommandSpamButtonEvents::DispEventAdvise((IDispatch*)m_spSpamButton);
		if(FAILED(hr))
			return hr;

		if(FAILED(hr))
		{
			ATLTRACE("Failed advising to CommandSpamButtonEvents");
			return hr;
		}

		hr = CommandNotSpamButtonEvents::DispEventAdvise((IDispatch*)m_spNotSpamButton);
		if(FAILED(hr))
			return hr;

		if(FAILED(hr))
		{
			ATLTRACE("Failed advising to CommandNotSpamButtonEvents");
			return hr;
		}
		/*====================================*/

		m_spApp_Event_New_Mail= m_spApp_Event_Send_Mail = m_spApp = spApp; //store the application object
		////////////////////////  Application event  ////////////////////////////////////
		hr = AppEvents::DispEventAdvise((IDispatch*)m_spApp_Event_Send_Mail,&__uuidof(Outlook::ApplicationEvents));	
		if(FAILED(hr))
			return hr;

		hr = AppEvents_NewMail::DispEventAdvise((IDispatch*)m_spApp_Event_New_Mail, &__uuidof(Outlook::ApplicationEvents_11));
		if (FAILED(hr))
			return hr;

		//Item events for Inbox folder
		if (GetInboxFolderPtr(m_spInboxs))
		{
			size_t size = m_spInboxs.size();
			for (int iIndex = 0; iIndex < static_cast<int>(size); iIndex++)
			{
				CComPtr<Outlook::_Items> spItems;
				m_spInboxs.at(iIndex)->get_Items(&spItems);
				m_spInboxItems.push_back(spItems);
				DispEventsAdvises(iIndex);
			}
		}

		//Get here all product settings in memory
		theApp.GetProductSettings();
		theApp.CheckiSPYAVRegistered();
		m_objExtractScanObj.LoadVirusScanEmailDB() ;
		m_objCommServer.Run();

		EnableDisablePlugin(ReadEmailScansettingFromReg()?1:0);
		
		return S_OK;
	}

	//void CreateWardWizFolder();

	STDMETHOD(OnDisconnection)(ext_DisconnectMode RemoveMode, SAFEARRAY * * custom)
	{
		if(!m_objExtractScanObj.UnLoadSignatures())
		{
			AddLogEntry(L"### Failed to unload signatures", 0, 0, true, SECONDLEVEL);
		}

		if(!theApp.UnloadRegistrationDLL())
		{
			AddLogEntry(L"### Failed to unload Registration DLL", 0, 0, true, SECONDLEVEL);
		}

		HRESULT hr = CommandSpamButtonEvents::DispEventUnadvise((IDispatch*)m_spSpamButton);
		if(FAILED(hr))
			return hr;

		hr = CommandNotSpamButtonEvents::DispEventUnadvise((IDispatch*)m_spNotSpamButton);
		if(FAILED(hr))
			return hr;

		hr = AppEvents::DispEventUnadvise((IDispatch*)m_spApp_Event_Send_Mail);
		if(FAILED(hr))
			return hr;
	
		size_t size = m_spInboxItems.size();
		for (int iIndex = 0; iIndex < static_cast<int>(size); iIndex++)
		{
			DispEventsUnAdvises(iIndex);
		}

		return S_OK;
	}

	STDMETHOD(OnAddInsUpdate)(SAFEARRAY * * custom)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(OnStartupComplete)(SAFEARRAY * * custom)
	{
		//CreateWardWizFolder();
		return E_NOTIMPL;
	}

	STDMETHOD(OnBeginShutdown)(SAFEARRAY * * custom)
	{
		return E_NOTIMPL;
	}
	
	void  RegisterNotification();
	bool  ProcessMailItem(CComQIPtr<Outlook::_MailItem> spMailItem);
	bool  GetWardWizSpamFolderPtr(CComQIPtr<Outlook::MAPIFolder> &spFolder2Move, CComQIPtr<Outlook::_MailItem> spMailItem);
	bool  GetWardWizSpamFolderPtrSEH(CComQIPtr<Outlook::MAPIFolder> &spFolder2Move, CComQIPtr<Outlook::_MailItem> spMailItem);
	bool GetInboxFolderPtr(VOLFOLDERSLIST &vInboxFolders);
	bool GetInboxFolderPtrSEH(VOLFOLDERSLIST &vInboxFolders);
	bool  ScanNewMail(CComQIPtr<Outlook::_MailItem> spMailItem);
	DWORD ProcessMailForSpam(CComQIPtr<Outlook::_MailItem> spMailItem);
	DWORD ProcessMailForContentScan(CComQIPtr<Outlook::_MailItem> spMailItem);
	bool  AddWardWizMailSignature(CComQIPtr<Outlook::_MailItem> &spMailItem);
	bool  AttachDeleteAttachmentFromNewMail(CComQIPtr<Outlook::_MailItem> spMailItem, bool bDelete=true ) ;
	static void OnDataReceiveCallBack(LPVOID lpParam);
	bool EnableDisablePlugin(DWORD dwEnableValue);
	bool EnableDisablePluginSEH(DWORD dwEnableValue);
	bool ReloadRegistartionDays();
	void ReloadSettings();
	bool ReadEmailScansettingFromReg();
	bool DispEventsAdvises(int iInboxItemNo);
	bool DispEventsUnAdvises(int iInboxItemNo);
	bool GetInboxFolderPointer(CComQIPtr<Outlook::MAPIFolder> &spFolder2Move);
	bool SendEmailData2Service(DWORD dwAddEditDelType, DWORD dwType, CString csFirstParam, CString csSecondParam, CString csThirdParam, bool bEmailScanWait);
	bool SendReLoadMessage2UI();
	bool IsDuplicateUpdateEvent(IDispatch * item);
	bool IsFullString(CString csInputPath);
public:
	static CAddin						*m_pThis;
	CComPtr <Office::CommandBar>		spNewCmdBar;
    CComPtr<Office::_CommandBars>		spCmdBars; 
	CComPtr<Office::CommandBar>			spCmdBar;
	CComPtr<Outlook::_Explorer>			m_spExplorer; 
	CComPtr<Office::_CommandBarButton>	m_spSpamButton; 
	CComPtr<Office::_CommandBarButton>	m_spNotSpamButton; 
	VOLFOLDERSLIST						m_spInboxs;
	VOLITEMSLIST						m_spInboxItems;
	CComPtr<Outlook::_Application>		m_spApp;
	CComPtr<Outlook::_Application>		m_spApp_Event_Send_Mail;//application
	CComPtr<Outlook::_Application>		m_spApp_Event_New_Mail;//application
	CISpyOutlookComm					m_objISpyOutlookComm;
	CWrdWizContaintFilter				m_objISpyContaintFilter;
	CExtractAttchForScan				m_objExtractScanObj;
	CISpySpamFilter						m_objISpySpamFilter;
	CISpyEmailSignature					m_objISpyEmailSignature;
	CISpyCommunicatorServer				m_objCommServer;
	bool								m_bUserMoveActionFromSpam;
	CWrdWizPercentageDisplayDlg			m_objPercentageDisplay;
	HANDLE                              m_hMoveMailProgressDisplayThread;
	DWORD								m_CurrentMoveMailCount;
	bool								m_SelectedMailFromWardWizSpam;
	bool								m_bKeepFutureMailAsSpam;
	bool								m_bKeepFutureMailAsNotSpam;

};

OBJECT_ENTRY_AUTO(__uuidof(Addin), CAddin)

#endif //__ADDIN_H_
