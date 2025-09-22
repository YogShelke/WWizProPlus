#pragma once
#include "JpegDialog.h"
#include "ISpyGUI.h"
#include "afxwin.h"
#include "ISpyGUIDlg.h"
#include "xSkinButton.h"

// CSettingsPassChangeDlg dialog

class CSettingsPassChangeDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CSettingsPassChangeDlg)

public:
	CSettingsPassChangeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSettingsPassChangeDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SETTINGS_PASS_CHANGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CxSkinButton			m_btnClose;
	CComboBox				m_cbSelectType;
	CxSkinButton			m_btnOk;
	CxSkinButton			m_btnCancel;
	CColorStatic			m_stChangePassFor;
	CColorStatic			m_stOldPass;
	CColorStatic			m_stNewPass;
	CColorStatic			m_stConfPass;
	CEdit					m_edtOldPass;
	CEdit					m_edtNewPass;
	CEdit					m_edtConfPass;
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonCancel();
	afx_msg void OnBnClickedButtonOk();
	afx_msg void OnEnChangeEditOldPass();

	CString	m_csEditText;
	DWORD   m_dwNoofTypingChar;
	CString m_csGetForgotPassword;
	void FetchForgotPasswordForDataEnc(DWORD PassType);
	bool SendRegisteredData2Service(DWORD dwType, LPBYTE lpResBuffer, DWORD dwResSize, DWORD dwResType, TCHAR *pResName);
	void CheckOldPassword();
	bool m_bOldPass;

	DWORD ValidatePassword(CString csPassword);
	afx_msg void OnEnChangeEditConfPass();
	afx_msg void OnEnChangeEditNewPass();

	afx_msg void OnCbnSelchangeComboType();
};
