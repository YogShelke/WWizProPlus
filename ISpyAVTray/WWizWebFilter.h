/*******************************************************************************************
*  Program Name   :	CWWizWebFilter.h
*  Description    :	It is Website blocked show Notification popup
*  Author Name    : Nitin Shelar
*  Date           : 11/07/2019
*  Version No:                                                                                                                              *                                                                                                                *
*********************************************************************************************/
#pragma once
#include "stdafx.h"
#include "afxwin.h"
#include "JpegDialog.h"
#include "windows.h"
#include "ISpyAVTray.h"
#include "Resource.h"
#include <resource.h>

typedef struct  _tagWEBURLCAT {
	TCHAR szURL[MAX_PATH];
	TCHAR szCategory[100];
} STWEBCAT, *PSTWEBCAT;

// WWizWebFilter dialog
#define WM_MESSAGESHOWCONTENT (WM_USER + 101)

class CWWizWebFilterDlg : public CDialog, public CSciterBase,
	sciter::behavior_factory,
	public sciter::host<CWWizWebFilterDlg>
{
	DECLARE_DYNAMIC(CWWizWebFilterDlg)

public:
	CWWizWebFilterDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWWizWebFilterDlg();

	virtual bool subscription(HELEMENT he, UINT& event_groups)
	{
		event_groups = UINT(-1);
		return true;
	}
	// the only behavior_factory method:
	virtual event_handler* create(HELEMENT he) { return this; }

	virtual void attached(HELEMENT he) {
		self = he;
	}
	virtual void detached(HELEMENT he) {
		self = NULL;
	}

// Dialog Data
	enum { IDD = IDD_WWIZWEBFILTER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	HWND get_hwnd();
	HINSTANCE get_resource_instance();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void OnBnClickedClose();

	afx_msg void OnBnClickedOK();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg LRESULT ShowWebBlockMessageHandler(WPARAM wParam, LPARAM lParam);

	sciter::dom::element m_root_el;
	HELEMENT			 self;

	CString				m_csUrl;
	CString				m_csCategory;
	bool				m_bTrayforBrowserSec;
	SCITER_VALUE		m_callMsgType;

	BEGIN_FUNCTION_MAP
		FUNCTION_0("OnClickOK", On_ClickOK);
		FUNCTION_0("OnClickClose", On_ClickClose);
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_1("OnGetShowBlockSite", On_GetShowBlockSite)
		FUNCTION_0("OnCheckMsgType", OnCheckMsgType)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
	END_FUNCTION_MAP

	json::value On_ClickOK();
	json::value On_ClickClose();
	json::value On_GetProductID();
	json::value On_GetLanguageID();
	json::value OnGetAppPath();
	json::value On_GetShowBlockSite(SCITER_VALUE svOnCallGetMsgType);
	json::value OnCheckMsgType();
	json::value On_GetThemeID();

private:
	bool LoadRequiredLibrary();

public:
	int GetTaskBarHeight();
	int GetTaskBarWidth();
	void GetBlokedWebsiteDetails(std::wstring, std::wstring);
};
