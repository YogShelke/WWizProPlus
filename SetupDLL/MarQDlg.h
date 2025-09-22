// MarQDlg.h : main header file for the MarQDlg.cpp 
//

#pragma once
#include "resource.h"
#include "afxcmn.h"
#include "TextProgressCtrl.h"

// CMarQDlg dialog

class CMarQDlg : public CDialog
{
	DECLARE_DYNAMIC(CMarQDlg)

public:
	CMarQDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMarQDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SHOW_MARQUEE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnStnClickedStaticMessage();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	CTextProgressCtrl    m_prgUsbProgressbar;
	afx_msg void OnNMCustomdrawDialogShowMarquee(NMHDR *pNMHDR, LRESULT *pResult);
};
