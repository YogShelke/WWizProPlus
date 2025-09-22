//Neha

#pragma once
#include "JpegDialog.h"
#include "afxwin.h"
#include "ColorStatic.h"
#include "xSkinButton.h"
#include "MyHyperLink.h"
#include "pictureex.h"

// CISpyPasswordPopUpDlg dialog

class CISpyPasswordPopUpDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CISpyPasswordPopUpDlg)

public:
	CISpyPasswordPopUpDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CISpyPasswordPopUpDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_PASSWORD_POPUP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	// issue resolved by lalit kumawat 12-24-2014 ,issue :Wardwiz crashes on clicking ok on sending mail for forgot password.
	bool     m_bIsEmailSendingFinish;
	DWORD    m_bEncryptionOption;
	DWORD ValidatePassword(CString );
	virtual BOOL OnInitDialog();
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonPasswordClose();
	//CStatic m_stPasswordHPic;
	CxSkinButton m_btnOk;
	CxSkinButton m_btnClose;
	CxSkinButton m_btnCancel;
	CStatic m_stPassword;
	CEdit m_edtPasswordLetters;
	//CBitmap	m_bmpPasswordHPic;
	CString m_csEditText;
	afx_msg void OnEnChangeEditEnterpassword();
	CStatic m_stExample;
	CStatic m_stEncryPassMsg;
	CColorStatic m_stDecPassMsg;
	CFont* m_pBoldFont;
	//CDataEncryptionDlg m_objDataEncrypt;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CMyHyperLink m_stEncForgotPassword;
	afx_msg void OnStnClickedStaticEncrypForgotPasswordLink();
	LRESULT OnChildFire(WPARAM wparam,LPARAM lparam);
	CxSkinButton m_btnEncSend;
	CxSkinButton m_btnEncCancel;
	afx_msg void OnBnClickedButtonEncSend();
	afx_msg void OnBnClickedButtonEncCancel();
	CColorStatic m_stEncEmailId;
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnClose();
	HCURSOR m_hButtonCursor;
	DWORD SendPassword2EmailID(LPTSTR m_szRegEmail,LPTSTR m_lpPassword);
	void FetchForgotPassword();
	CString m_csGetForgotPassword;
	CString XOREncrypt(CString a_sValue, DWORD a_sPasswordSize);
	HMODULE		m_hRegisterDLL;
	TCHAR		m_szAppPath[512];
	CString 	m_csEmailId;
	CStatic		m_stMailboxPic;
	HBITMAP		m_bmpEmailIconPic;

	afx_msg void OnEnChangeEditReenterPassword();
	void OnClosePopup();
	void OnDialogClose();
	afx_msg void OnBnClickedBtnSendcancel();
	void ShowHideControls4ForgotPassword();

	DWORD   m_dwNoofTypingChar;//Name:Varada Ikhar, Date:15/01/2015 Version: 1.8.3.17
	//Description:Data Encryption password length should be reduced. min:6 max:24
	CEdit m_edtReEnterPassword;
	CColorStatic m_stReEnterPass;
	CColorStatic m_stSendingMail;
	CPictureEx m_stDataEncPreLoader;
	HANDLE			m_Thread;
	DWORD			m_dwThread;
	LPTSTR			m_csEmail;
	LPTSTR			m_csPass;
	CxSkinButton m_btnSendCancel;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};




