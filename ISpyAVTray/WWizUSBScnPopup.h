//WWizUSBScnPopup.h  :Header File For the Restart Client Try Pop Up
//
#pragma once
#include "stdafx.h"
#include "afxwin.h"
#include "JpegDialog.h"
#include "Resource.h"
#include "WardWizResource.h"
#include "ISpyAVTray.h"
#include "windows.h"

//CWWizUSBScnPopup Dialog

class CWWizUSBScnPopup :public CDialog,
	public CSciterBase,
	public sciter::host<CWWizUSBScnPopup>
{
	DECLARE_DYNAMIC(CWWizUSBScnPopup);
	HELEMENT self;

public:
	CWWizUSBScnPopup(CWnd* pParent = NULL);  // Standard constructor
	virtual ~CWWizUSBScnPopup();

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
	enum { IDD = IDD_DIALOG_USB_SCAN_POPUP };

protected:
	virtual  void DoDataExchange(CDataExchange* pDX);	// DDX/DDV Support

	DECLARE_MESSAGE_MAP();

public:
	HWND get_hwnd();
	HINSTANCE get_resource_instance();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	virtual BOOL OnInitDialog();
	int GetTaskBarWidth();
	int GetTaskBarHeight();
	void OnBnClickedClose();
	sciter::dom::element m_root_el;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg LRESULT OnNcHitTest(CPoint point);

	int m_iTrayMsgType;
	CString csDriveName;
	SCITER_VALUE m_svGetMsgType;

	BEGIN_FUNCTION_MAP
		FUNCTION_0("OnCheckMsgType", OnCheckMsgType);
		FUNCTION_0("OnGetDriveName", OnGetDriveName)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetAppPath", OnGetAppPath)
	END_FUNCTION_MAP

	json::value OnCheckMsgType();
	json::value OnGetDriveName();
	json::value On_GetLanguageID();
	json::value On_GetThemeID();
	json::value On_GetProductID();
	json::value OnGetAppPath();
};