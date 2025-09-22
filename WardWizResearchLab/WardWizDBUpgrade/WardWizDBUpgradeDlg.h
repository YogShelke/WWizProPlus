
// WardWizDBUpgradeDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "ISpyDataManager.h"
#include "WWizCRC64.h"

// CWardWizDBUpgradeDlg dialog
class CWardWizDBUpgradeDlg : public CDialog
{
// Construction
public:
	CWardWizDBUpgradeDlg(CWnd* pParent = NULL);	// standard constructor
	~CWardWizDBUpgradeDlg();

// Dialog Data
	enum { IDD = IDD_WARDWIZDBUPGRADE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedButtonBrowse();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonCancel();
	DECLARE_MESSAGE_MAP()
public:
		CEdit					m_editStatus;
		CEdit					m_editFilePath;
		CStatic					m_stSigCount;
		CStatic					m_stTotalSigCount;
		CButton					m_btnStart;
		CButton					m_btnCancel;
		CDataManager			m_objEncDec;
		LPBYTE					m_lpszBuffer;

public:
	void UpdateStatus(CString csStatus);
	bool DumpBufferInFile(LPTSTR szFilePath, LPBYTE bFileBuffer, DWORD dwBufSize, LPSTR szSignature, LPSTR szVersion, ULONG64 ulFileCRC, DWORD dwVersionLength);
	CEdit m_editVersionNo;
	CEdit m_editDBSignature;
};
