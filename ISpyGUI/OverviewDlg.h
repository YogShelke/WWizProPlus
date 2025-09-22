#pragma once


// COverviewDlg dialog

class COverviewDlg : public CDialog
{
	DECLARE_DYNAMIC(COverviewDlg)

public:
	COverviewDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~COverviewDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_OVERVIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
