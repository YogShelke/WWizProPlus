#pragma once
#include "stdafx.h"
#include "afxwin.h"
#include "JpegDialog.h"
#include "Resource.h"
#include "WardWizResource.h"
#include "ISpyAVTray.h"

class CWardWizPortScanPopUp : public CDialog,
	public CSciterBase,
	public sciter::host<CWardWizPortScanPopUp>
{
	DECLARE_DYNAMIC(CWardWizPortScanPopUp)
public:
	CWardWizPortScanPopUp(CWnd* pParent = NULL);
	virtual ~CWardWizPortScanPopUp();
	// Dialog Data
	enum { IDD = IDD_DIALOG_FIREWALL_POPUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	HWND get_hwnd();
	HANDLE	m_hMutexHandle;
	HINSTANCE get_resource_instance();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	virtual BOOL OnInitDialog();
	int GetTaskBarWidth();
	int GetTaskBarHeight();

	sciter::dom::element m_root_el;
	bool		m_bProductUpdate;
		
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	bool SingleInstanceCheck();

	int m_iTrayMsgType;

	BEGIN_FUNCTION_MAP
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
	END_FUNCTION_MAP

	SCITER_VALUE m_svGetMsgType;

	json::value On_GetProductID();
	json::value On_GetLanguageID();
	json::value OnGetAppPath();
	json::value On_GetThemeID();
};

