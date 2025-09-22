#pragma once
#include "stdafx.h"
#include "afxwin.h"
#include "JpegDialog.h"
#include "Resource.h"
#include "WardWizResource.h"
#include "ISpyAVTray.h"

// CPopUpDialog dialog

class CPopUpDialog : public CJpegDialog ,public CSciterBase, 
	public sciter::host<CPopUpDialog>
{
	DECLARE_DYNAMIC(CPopUpDialog)

public:
	CPopUpDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPopUpDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_TRAY_POP_UP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CStatic					m_stAutoUpdateText;
	CxSkinButton			m_btnDownloadNow;
	CxSkinButton			m_btnRemindLater;
	HBITMAP					m_bmpDownloadPic;
	CStatic					m_bmpDownload;
	CColorStatic			m_stWWUpdate;

	HWND get_hwnd();
	HINSTANCE get_resource_instance();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	afx_msg void OnBnClickedButtonDownloadNow();
	afx_msg void OnBnClickedButtonRemindLater();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	virtual BOOL OnInitDialog();

	int GetTaskBarWidth() ;
	int GetTaskBarHeight();
	
	void HideAllElements();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnClosingGetCalled();
	CxSkinButton m_btnClosing;

	bool SendData2UI(int iMessageInfo, bool bWait = false);

	BEGIN_FUNCTION_MAP	
		FUNCTION_0("OnClickUpdate", On_ClickUpdate)
		FUNCTION_0("OnClickRemindLater", On_ClickRemindLater)
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
	END_FUNCTION_MAP

	json::value On_ClickUpdate();
	json::value On_ClickRemindLater();
	json::value On_GetProductID();
	json::value OnGetAppPath();
	json::value On_GetLanguageID();
	json::value On_GetThemeID();
	sciter::dom::element m_root_el;
};
