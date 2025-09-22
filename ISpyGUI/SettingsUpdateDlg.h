#pragma once
#include "afxwin.h"
#include "JpegDialog.h"
#include "TabDialog.h"
#include "PictureEx.h"
#include "ISpyGUI.h"
#include "HTMLListCtrl.h"


// CSettingsUpdateDlg dialog

class CSettingsUpdateDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CSettingsUpdateDlg)

public:
	CSettingsUpdateDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingsUpdateDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SETTINGS_UPDATE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg LRESULT OnNcHitTest(CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	BOOL					RefreshString();
	void	setDefaultSettings();
	CHTMLListCtrl			m_list;
	CImageList				m_ImageList;
	BOOL					m_bCheckBoxes;
	BOOL					m_bGridLines;
	BOOL					m_bShowImages;

};
