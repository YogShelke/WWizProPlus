#pragma once
#include "Resource.h"

// CMigFailedDlg dialog

class CMigFailedDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMigFailedDlg)

public:
	CMigFailedDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMigFailedDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_KEY_MIGRATION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedMfclinkWebPage();
};
