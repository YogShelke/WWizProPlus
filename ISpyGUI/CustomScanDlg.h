#pragma once


// CCustomScanDlg dialog

class CCustomScanDlg : public CDialog
{
	DECLARE_DYNAMIC(CCustomScanDlg)

public:
	CCustomScanDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCustomScanDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_CUSTOMSCAN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
