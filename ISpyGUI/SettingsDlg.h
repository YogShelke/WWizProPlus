#pragma once
#include "JpegDialog.h"
#include "TabDialog.h"
#include "afxwin.h"
#include "xSkinButton.h"
#include "ColorStatic.h"
#include "afxcmn.h"
#include "PictureEx.h"
#include "HTMLListCtrl.h"
#include "GradientStatic.h"
#include "SettingsGeneralDlg.h"
#include "SettingsEmailDlg.h"
#include "SettingsScanDlg.h"
#include "SettingsUpdateDlg.h"


// CSettingsDlg dialog

class CSettingsDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CSettingsDlg)

public:
	CSettingsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SETTINGS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
public:
	CColorStatic			m_stTitle;
	CxSkinButton			m_Close_Button;
	HCURSOR					m_hButtonCursor;
	

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	CStatic					m_stEmailScan;
	HBITMAP					m_bmpSettingsWardWizLogo;
//	CStatic					m_stSettingsHeader;
//	afx_msg LRESULT			OnNcHitTest(CPoint point);
	afx_msg					void OnBnClickedBackButton();
	virtual BOOL			PreTranslateMessage(MSG* pMsg);
	BOOL					RefreshString();
	BOOL					m_bCheckBoxes;
	BOOL					m_bGridLines;
	BOOL					m_bShowImages;
	//CHTMLListCtrl			m_list;
	CImageList				m_ImageList;
	CStatic					m_stSettingsHeader;
	CFont					m_settingsHeaderFont;
	CStatic					m_stSettingLogo;
	CTabDialog				*m_pTabDialog;
	CColorStatic 			*m_stScan;
	CxSkinButton			*m_btnGeneralSettings;
	CxSkinButton			*m_btnEmailSettings;
	CxSkinButton			*m_btnScanSettings;
	CxSkinButton			*m_btnUpdateSettings;
	
	CStatic					m_dlgPos;
	CSettingsGeneralDlg		*m_pSettingsGeneralDlg;
	CSettingsEmailDlg		*m_pSettingsEmailDlg;
	CSettingsScanDlg		*m_pSettingsScanDlg;
	CSettingsUpdateDlg		*m_pSettingsUpdateDlg;



	BOOL	InitTabDialog();
	BOOL	AddPagesToTabDialog();
	BOOL ShowEssentialSetting();
	BOOL ShowProSetting();
	BOOL ShowEliteSetting();
	BOOL ShowBasicSetting();

	CxSkinButton			m_btnSettingsDefault;
	CxSkinButton			m_btnSettingsOk;
	afx_msg void OnBnClickedButtonSettingsOk();
	afx_msg void OnBnClickedButtonDefault();
	void CloseSettingWindow();
	BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};
