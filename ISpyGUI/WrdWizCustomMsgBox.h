#pragma once
#include "stdafx.h"
#include "afxwin.h"
#include "ISpyGUI.h"
#include "MyHyperLink.h"


// CWrdWizCustomMsgBox dialog

class CWrdWizCustomMsgBox : public CDialogEx
{
	DECLARE_DYNAMIC(CWrdWizCustomMsgBox)

public:
	CWrdWizCustomMsgBox(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWrdWizCustomMsgBox();

// Dialog Data
	enum { IDD = IDD_DIALOG_CUSTOMIZE_MSGBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CButton m_btnOK;
	CButton m_BtnCancel;
	CBitmap m_bmpMsgBoxImage;
	HBITMAP m_bmpErrorIcon;
	CStatic m_stErrorMsgICon;
	CStatic m_stFailedTextMessage;
	CMyHyperLink m_stHyperlinkForOfflinePatches;
	CString m_csFailedMsgText;
	void OnBackground(CBitmap* bmpImg, CDC* pDC, int yHight, bool bisStretch);
	
	afx_msg void OnPaint();
	LRESULT OnChildFire(WPARAM wparam, LPARAM lparam);

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	bool GetOSType();
	void DisplayLinkAsperOSnadProductVersion();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	bool GetDefaultHTTPBrowser(LPTSTR lpszBrowserPath);
	DWORD m_dwMsgErrorNo;
	DECLARE_MESSAGE_MAP()
};
