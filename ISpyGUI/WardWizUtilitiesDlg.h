#pragma once


// CWardWizUtilitiesDlg dialog

#include "JpegDialog.h"
#include "afxwin.h"
#include "xSkinButton.h"
#include "ColorStatic.h"
#include "PictureEx.h"
#include "afxcmn.h"
#include "ISpyDataManager.h" 

class CWardWizUtilitiesDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CWardWizUtilitiesDlg)

public:
	CWardWizUtilitiesDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWardWizUtilitiesDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_UTILITYDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	HCURSOR					m_hButtonCursor;
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	void RefreshStrings();
	CColorStatic m_stHeaderDescription;
	CStatic m_stHUtility;
	HBITMAP m_bmpHUtility;
	CColorStatic m_stHeaderName;
	CColorStatic m_stAutorunHeader;
	CColorStatic m_stAutoRunDesc;
	CColorStatic m_stTempFileHeader;
	CColorStatic m_stTempFileDesc;
	CColorStatic m_stUSBVacHeader;
	CColorStatic m_stUSBVacDesc;
	CxSkinButton m_btnAutoRunScan;
	CxSkinButton m_btnTempCleaner;
	CxSkinButton m_btnUSBVac;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnBnClickedButtonAutorun();
	afx_msg void OnBnClickedButtonTempfileCleaner();
	afx_msg void OnBnClickedButtonUsbvac();
	CString GetAppFolderPath();
	CString m_csAppPath;
};
