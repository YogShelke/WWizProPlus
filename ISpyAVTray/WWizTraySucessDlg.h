//WWizTraySuccessDlg.h : Header file for the Update Success Tray Pop Up
//
#pragma once
#include "stdafx.h"
#include "afxwin.h"
#include "JpegDialog.h"
#include "Resource.h"
#include "WardWizResource.h"
#include "ISpyAVTray.h"

// CWWizTraySucessDlg dialog

class CWWizTraySucessDlg : public CDialog,
	public CSciterBase,
	public sciter::host<CWWizTraySucessDlg>
{
	DECLARE_DYNAMIC(CWWizTraySucessDlg)

public:
	CWWizTraySucessDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWWizTraySucessDlg();

// Dialog Data
	enum { IDD = ID_DIALOG_TRAY_UPDATE_SUCCESS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	HWND get_hwnd();
	HINSTANCE get_resource_instance();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	afx_msg void OnBnClickedOK();

	virtual BOOL OnInitDialog();
	int GetTaskBarWidth();
	int GetTaskBarHeight();

	void HideAllElements();

	sciter::dom::element m_root_el;
	bool		m_bProductUpdate;

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg LRESULT OnNcHitTest(CPoint point);

	BEGIN_FUNCTION_MAP
		FUNCTION_0("OnClickOK", On_ClickOK)
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
	END_FUNCTION_MAP

	json::value On_ClickOK();
	json::value On_GetProductID();
	json::value OnGetAppPath();
	json::value On_GetLanguageID();
	json::value On_GetThemeID();
};
