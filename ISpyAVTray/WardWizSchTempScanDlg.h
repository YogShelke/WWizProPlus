#pragma once
#include "stdafx.h"
#include "afxwin.h"
#include "JpegDialog.h"
#include "Resource.h"
#include "WardWizResource.h"
#include "ISpyAVTray.h"


// CWardWizSchTempScanDlg dialog

class CWardWizSchTempScanDlg : public CDialog,
	public CSciterBase,
	public sciter::host<CWardWizSchTempScanDlg>
{
	DECLARE_DYNAMIC(CWardWizSchTempScanDlg)

public:
	CWardWizSchTempScanDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWardWizSchTempScanDlg();

// Dialog Data
	enum { IDD = IDD_WARDWIZSCHTEMPSCANDLG };

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

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg LRESULT OnNcHitTest(CPoint point);

	BEGIN_FUNCTION_MAP
		FUNCTION_0("OnClickIgnore", On_ClickOK)
		FUNCTION_0("OnClickStartScan", On_ClickStartScan)
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
	END_FUNCTION_MAP

	json::value On_ClickOK();
	json::value On_ClickStartScan();
	json::value On_GetProductID();
	json::value OnGetAppPath();
	json::value On_GetLanguageID();
	json::value On_GetThemeID();
};
