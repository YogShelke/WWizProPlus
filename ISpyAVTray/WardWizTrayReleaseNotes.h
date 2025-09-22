#pragma once


// CWardWizTrayReleaseNotes dialog

#include "stdafx.h"
#include "Resource.h"
#include "JpegDialog.h"
#include "iTINRegWrapper.h"
//#include "afxwin.h"
//#include "afxcmn.h"

class CWardWizTrayReleaseNotes : public CJpegDialog
, public CSciterBase,
public sciter::host<CWardWizTrayReleaseNotes>

{
	DECLARE_DYNAMIC(CWardWizTrayReleaseNotes)

public:
	CWardWizTrayReleaseNotes(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWardWizTrayReleaseNotes();

// Dialog Data
	enum { IDD = IDD_DIALOG_TRAY_RELEASE_NOTES };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	
	DECLARE_MESSAGE_MAP()

public:
	CColorStatic m_stAppDBVersion;
	CxSkinButton m_btnClose;
	CxSkinButton m_btnOK;
	CRichEditCtrl m_richedtReleaseInfo;
	CITinRegWrapper	m_objReg;
	CString		m_csAppDBVersion;
	CString		m_csVersioninfo;

	HWND get_hwnd();
	HINSTANCE get_resource_instance();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnEnVscrollRicheditReleaseNoteText();
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnBnClickedButtonClose();
	int GetTaskBarHeight();
	int GetTaskBarWidth();
	bool GetInfoFromINI();
	bool SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait);
	DWORD GetRegistryDWORDEntry(HKEY hRootKey, TCHAR *pKeyName, TCHAR *pValueName, DWORD& dwValue);
		
	SCITER_VALUE m_svProductinfoCB;
	sciter::dom::element m_root_el;

	BEGIN_FUNCTION_MAP
		FUNCTION_1("getReleaseNotesinfo", GetReleaseNotesinfo)
		FUNCTION_0("OnOK", On_ClickOK)
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
	END_FUNCTION_MAP
	
	json::value	GetReleaseNotesinfo(SCITER_VALUE svProductinfo);
	json::value On_ClickOK();
	json::value On_GetProductID();
	json::value OnGetAppPath();
	json::value On_GetLanguageID();
};