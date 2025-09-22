
// WardWiz_Exec_BlockDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CWardWiz_Exec_BlockDlg dialog
class CWardWiz_Exec_BlockDlg : public CDialogEx
{
// Construction
public:
	CWardWiz_Exec_BlockDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WARDWIZ_EXEC_BLOCK_DIALOG };

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
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedOk();
	CEdit m_edtCtrl;
	CListCtrl m_lstCtrl;
	CListCtrl *pListCtrl;

	afx_msg void OnBnClickedOk2();
};
