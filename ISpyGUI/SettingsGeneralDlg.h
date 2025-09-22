#pragma once
#include "JpegDialog.h"
#include "ISpyGUI.h"
#include "HTMLListCtrl.h"



// CSettingsGeneralDlg dialog

class CSettingsGeneralDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CSettingsGeneralDlg)

public:
	CSettingsGeneralDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingsGeneralDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SETTINGS_GENERAL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnNcHitTest(CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	BOOL	RefreshString();
	void	setDefaultSettings();
	CHTMLListCtrl			m_list;
	CImageList				m_ImageList;
	BOOL					m_bCheckBoxes;
	BOOL					m_bGridLines;
	BOOL					m_bShowImages;
};
