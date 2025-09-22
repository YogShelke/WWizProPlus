// WardwizLaunchDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "PictureEx.h"
#include "GradientStatic.h"


// CWardwizLaunchDlg dialog
class CWardwizLaunchDlg : public CJpegDialog
{
// Construction
public:
	CWardwizLaunchDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_WARDWIZLAUNCH_DIALOG };

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
	void LaunchUI();
	void IsWow64();
	void LaunchOSSetup();
	bool GetModulePath(TCHAR *szModulePath, DWORD dwSize);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	BOOL m_bIsWow64;
	CPictureEx m_stlaunchGif;
	CRect	m_rect;
	HANDLE m_hThread_Launch;
	HANDLE m_hThread_Hide;
	CStatic		m_stExtractSetupText;
	CStatic	m_stWaitText;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void ShutDown();
	BOOL LaunchProcessThrCommandLine(LPTSTR pszCmdLine);
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
