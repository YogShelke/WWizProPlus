
// WardWizUnRegisterDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CWardWizUnRegisterDlg dialog
class CWardWizUnRegisterDlg : public CDialogEx
{
// Construction
public:
	CWardWizUnRegisterDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WARDWIZUNREGISTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CButton m_btnUnRegister;
	afx_msg void OnBnClickedButtonAboutWardwiz();
	afx_msg void OnBnClickedButtonUnregister();
	afx_msg void OnBnClickedButtonCancel();
	bool RemoveRegistry();
	bool RemoveFilesFromHardDrive();
	bool RemoveFromEvalRegDLL();
};
