#pragma once
#include "resource.h"
#include "afxwin.h"
#include "JpegDialog.h"
#include "xSkinButton.h"
#include "ColorStatic.h"
#include "afxcmn.h"
#include "PipeConstants.h"
//#include <fstream.h>

// CWrdwizStartupTipsDlg dialog

class CWrdwizStartupTipsDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CWrdwizStartupTipsDlg)

public:
	CWrdwizStartupTipsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWrdwizStartupTipsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_STARUPTIPS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	HICON					m_hIcon;
	HBITMAP					m_bmpShowTipBulb;
	HBITMAP					m_bmpShowTipHeader;
	CStatic					m_stShowTipHeader;
	CStatic					m_stShowTipBulb;
	CxSkinButton			m_btnPrev;
	CxSkinButton			m_btnNext;
	CxSkinButton			m_btnOk;
	CColorStatic			m_stEnableTips;
	CButton					m_chkEnableTips;
	CColorStatic			m_stTipOfDay;
	CColorStatic			m_stDYK;
	CxSkinButton			m_btnClose;
	CRichEditCtrl			m_RichEdtCtrlTip;
	CStringArray			m_csstartUpTips;
	DWORD					m_dwtipIndex;
	DWORD					m_dwEnableStartUpTips;
	DWORD                   m_dwTotalStartTips;
	int                     m_iLastTooltipShown;


	afx_msg void OnBnClickedButtonPrev();
	afx_msg void OnBnClickedButtonNext();
	afx_msg void OnPaint();
	afx_msg void OnEnVscrollRichedit21Tip();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnBnClickedCheckEnableStartupTips();
	afx_msg void OnEnChangeRichedit2Tips();
	DWORD ReadRegistryEntryofLastToolTip();
	virtual BOOL OnInitDialog();
	void RefreshStrings();
	bool GetStartUpTips();
	void WriteRegistryEntryofStartUpTips(DWORD dwChangeValue);
	bool SetRegistrykeyUsingService(LPCTSTR SubKey , LPCTSTR lpValueName , DWORD dwType , DWORD dwData, bool bWait = true);
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void WriteToReglastToolTipShown();
	afx_msg void OnClose();
};
