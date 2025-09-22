#pragma once
#include "afxwin.h"
#include "Resource.h"
#include "JpegDialog.h"
#include "xSkinButton.h"
#include "ColorStatic.h"
#include "WardWizRegistration.h"

// CRegistrationMsgPopupDlg dialog

class CRegistrationMsgPopupDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CRegistrationMsgPopupDlg)

public:
	CRegistrationMsgPopupDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRegistrationMsgPopupDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_REGISTARTION_POPUP_MSG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	HCURSOR					m_hButtonCursor;
	CxSkinButton			m_btnRegClose;
	CxSkinButton			m_btnRegister;
	CColorStatic			m_stExpiredText;
	CxSkinButton			m_btnRegMinimize;
	CStatic					m_stRegMsgIconPic;
	HBITMAP					m_bmpRegMsgIcon;
	CColorStatic			m_stRegMsgDescription;
	CxSkinButton			m_btnRegmsgBuyNow;
	CFont*					m_pTextFont;
	CFont*					m_pBoldFont;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonRegmsgMinimize();
	afx_msg void OnBnClickedButtonRegisternowRegistartion();
	afx_msg void OnBnClickedButtonRegmsgClose();
	afx_msg void OnBnClickedButtonRegmsgBuynow();
	BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

	BOOL RefreshString();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	bool					m_bShowAtStartUp; 
	CButton					m_chkDoNotShowAgain;
	CxSkinButton			m_btnContinue; 
	CColorStatic			m_stDoNotShowAgain;

	afx_msg void OnBnClickedCheckDoNotShowAgain();
	afx_msg void OnBnClickedButtonContinue();
	bool SetRegistrykeyUsingService(LPCTSTR SubKey, LPCTSTR lpValueName, DWORD dwType, DWORD dwData, bool bWait);
};
