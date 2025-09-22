#pragma once
#include "stdafx.h"
#include "afxwin.h"
#include "JpegDialog.h"
#include "Resource.h"
#include "WardWizResource.h"
#include "ISpyAVTray.h"

//#define rmd1 "10 mints"  // Update Reminder time period.
//#define rmd2 "20 mints"
//#define rmd3 "30 mints"
//#define rmd4 "1 hrs"

// CPopUpWithReminder dialog

class CPopUpWithReminder : public CJpegDialog
	, public CSciterBase,
	public sciter::host<CPopUpWithReminder>
{
	DECLARE_DYNAMIC(CPopUpWithReminder)

public:
	CPopUpWithReminder(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPopUpWithReminder();
	
	HWND get_hwnd();
	HINSTANCE get_resource_instance();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

// Dialog Data
	enum { IDD = IDD_DIALOG_TRAY_POP_WITH_REMINDER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:

	CColorStatic			m_stAutoRemUpdateText;
	//CxSkinButton			m_btnUpdateNow;
	//CxSkinButton			m_btnReminder;
	HBITMAP					m_bmpDownloadPic;
	CColorStatic			m_bmpRemDownload;
	CColorStatic			m_stLiveUpdateHeader;  //header
    CColorStatic            m_stCmbLable;	
	//CISpyAVTrayDlg          m_objTrayDlg;
	// Restart now for appling  new updationg
	CxSkinButton m_btnRestartNow;
	// Restart later for  later restart
	CxSkinButton m_btnRestartLater;
    
	virtual BOOL OnInitDialog();
	int GetTaskBarWidth() ;
	int GetTaskBarHeight();
	//afx_msg void OnBnClickedButtonDownloadNow();
	//afx_msg void OnBnClickedButtonRemindLater();
	CComboBox m_CmbForTimeReminder;

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);	
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnBnClickedReminderRestartNow();
	afx_msg void OnBnClickedReminderRemindLater(SCITER_VALUE);
	
	CxSkinButton m_closeBtn;
	afx_msg void OnClosingButtonGetClicked();
	DWORD m_dwRemindTime;
	sciter::dom::element m_root_el;

	BEGIN_FUNCTION_MAP
		FUNCTION_1("onRebootLater", OnRebootLater)
		FUNCTION_0("onRebootNow", OnRebootNow)
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
	END_FUNCTION_MAP

	json::value OnRebootLater(SCITER_VALUE svReminder_Time);
	json::value OnRebootNow();
	json::value On_GetProductID();
	json::value OnGetAppPath();
	json::value On_GetLanguageID();
	json::value On_GetThemeID();
};
