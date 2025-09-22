#pragma once
#include "stdafx.h"
#include "afxwin.h"
#include "JpegDialog.h"
#include "Resource.h"
#include "WardWizResource.h"
#include "ISpyAVTray.h"

#define	WM_MESSAGESHOWAPPBLK (WM_USER + 201)

typedef struct  _tagAPPBLK {
	int iTrayMsgType;
	TCHAR szFilePath[MAX_PATH];
} STAPPBLK, *PSTAPPBLK;

class CWardWizFirewallBlockPopUp : public CDialog,
	public CSciterBase,
	sciter::behavior_factory,
	public sciter::host<CWardWizFirewallBlockPopUp>
{
	DECLARE_DYNAMIC(CWardWizFirewallBlockPopUp)
public:
	CWardWizFirewallBlockPopUp(CWnd* pParent = NULL);
	virtual ~CWardWizFirewallBlockPopUp();

	// the only behavior_factory method:
	virtual bool subscription(HELEMENT he, UINT& event_groups)
	{
		event_groups = UINT(-1);
		return true;
	}
	virtual event_handler* create(HELEMENT he) { return this; }

	virtual void attached(HELEMENT he) {
		self = he;
	}
	virtual void detached(HELEMENT he) {
		self = NULL;
	}
	// Dialog Data
	enum { IDD = IDD_DIALOG_FIREWALL_POPUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

public:
	HWND get_hwnd();
	HINSTANCE get_resource_instance();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	virtual BOOL OnInitDialog();
	int GetTaskBarWidth();
	int GetTaskBarHeight();
	HELEMENT			 self;
	sciter::dom::element m_root_el;
	bool		m_bProductUpdate;
		
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg LRESULT ShowAppBlockMessageHandler(WPARAM wParam, LPARAM lParam);
	bool SingleInstanceCheck();

public:
	int m_iTrayMsgType;
	CString csFilePath;

	BEGIN_FUNCTION_MAP
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_1("OnCheckMsgType", OnCheckMsgType);
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
	END_FUNCTION_MAP

	SCITER_VALUE m_svGetMsgType;

	json::value On_GetProductID();
	json::value On_GetLanguageID();
	json::value OnGetAppPath();
	json::value OnCheckMsgType(SCITER_VALUE svGetMsgType);
	json::value On_GetThemeID();
	afx_msg void OnClose();
};

