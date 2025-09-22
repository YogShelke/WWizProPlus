
// ConCurrentQueueDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "CnCurrentQueue.h"

#define SCANNER_DEFAULT_THREAD_COUNT        25

// CConCurrentQueueDlg dialog
class CConCurrentQueueDlg : public CDialogEx
{
// Construction
public:
	CConCurrentQueueDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_CONCURRENTQUEUE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_lstCnClist;
	CEdit m_editNumbers;
	CButton m_btnAdd;
	afx_msg void OnBnClickedButton1();
	void StartThreads();
	std::thread m_Threads[SCANNER_DEFAULT_THREAD_COUNT];

	CConCurrentQueue<std::wstring> m_onAccessQueue;
	CButton m_btnSave;
	afx_msg void OnBnClickedButtonSave();
};
