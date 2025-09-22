#pragma once
#include "resource.h"
#include "afxwin.h"

// CWardWizEmailCustomMsgBox dialog

class CWardWizEmailCustomMsgBox : public CDialog
{
	DECLARE_DYNAMIC(CWardWizEmailCustomMsgBox)

public:
	CWardWizEmailCustomMsgBox(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWardWizEmailCustomMsgBox();

// Dialog Data
	enum { IDD = IDD_DIALOG_EMAIL_CUSTOM_MESSAGE_BOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CButton m_btnOk;
	CButton m_btnCancel;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	void OnBackground(CBitmap* bmpImg, CDC* pDC, int yHight, bool bisStretch);
	CStatic m_stMessageToMakeMailSpam;
	CStatic m_stMessageToMakeMailNotSpam;
	CButton m_chkForBlackOrWhiteList;
	CStatic m_stTextForFutureBlackList;
	CStatic m_stTextForFutureWhiteList;
	CBitmap m_bmpMsgBoxImage;
	HBITMAP m_bmpQuestionMark;
	bool m_bClickOnSpamMsgBox;
	bool m_bcheckBoxtick;
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CStatic m_stQuestionMark;
};
