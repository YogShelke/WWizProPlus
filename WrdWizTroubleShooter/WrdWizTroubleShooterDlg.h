
// WrdWizTroubleShooterDlg.h : header file
//

#pragma once
#include "JpegDialog.h"
#include "afxwin.h"
#include "ISpyCommServer.h"
#include "PipeConstants.h"

// CWrdWizTroubleShooterDlg dialog
class CWrdWizTroubleShooterDlg : public CJpegDialog
{
// Construction
public:
	CWrdWizTroubleShooterDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WRDWIZTROUBLESHOOTER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

public:
	//CSystemTray	m_TrayIcon;
	int GetTaskBarWidth();
	int GetTaskBarHeight();
	
	CColorStatic m_stStatus;
	//CString m_csStatus;
	CFont m_BoldText;
	static CWrdWizTroubleShooterDlg	 	*m_pThis;

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnInstallationDriver(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnStartDriver(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnInstallationCompleted(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCloseTLb(WPARAM wParam, LPARAM lParam);
	void CloseUI();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	static void OnDataReceiveCallBack(LPVOID lpParam);
	void ShowDriverStatus(DWORD dwMsg);
	DECLARE_MESSAGE_MAP()
};
