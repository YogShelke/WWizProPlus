
// PackageInstallerDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CPackageInstallerDlg dialog
class CPackageInstallerDlg : public CDialogEx
{
// Construction
public:
	CPackageInstallerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_PACKAGEINSTALLER_DIALOG };

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
	CEdit m_editRemoteIP;
	CString m_csUserName;
	CString m_csPassword;
	CString m_csRemoteIP;
	afx_msg void OnBnClickedButtonInstall();
	afx_msg void OnBnClickedButtonCancel();
	CString GetMachineNameByIP(CString csIPAddress);
};
