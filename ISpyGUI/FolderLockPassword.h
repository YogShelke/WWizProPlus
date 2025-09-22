#pragma once

#include "JpegDialog.h"
#include "afxwin.h"
#include "xSkinButton.h"
#include "ColorStatic.h"
#include "ISpyGUI.h"
#include "MyHyperlink.h"
#include "PictureEx.h"
#include "AVRegInfo.h"


// CFolderLockPassword dialog

class CFolderLockPassword : public CJpegDialog
{
	DECLARE_DYNAMIC(CFolderLockPassword)

public:
	CFolderLockPassword(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFolderLockPassword();

// Dialog Data
	enum { IDD = IDD_DIALOG_FOLDERLOCK_PASSWORD };

protected:
	HMODULE		m_hRegisterDLL ;
	HMODULE		m_hEmailDLL ;
	
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	
public:
	DWORD   m_dwNoofTypingChar;
	virtual BOOL OnInitDialog();
	HCURSOR	m_hButtonCursor;
	bool		 m_bSelectComboOption;
	CColorStatic m_stFolderPassLockText;
	CColorStatic m_stFolderPassUnlockText;
	CStatic m_stFolderPassLockPic;
	CStatic m_stFolderPassUnlockPic;
	HBITMAP m_bmpFolderPassLockPic;
	HBITMAP m_bmpFolderPassUnlockPic;
	HBITMAP m_bmpFolderEmailIconPic;
	CxSkinButton m_btnFolderPassOk;
	CxSkinButton m_btnFolderPassCancel;
	CColorStatic m_stFileIsLocked;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CxSkinButton m_btnPassMinimize;
	afx_msg void OnBnClickedButtonPassMinimize();
	CxSkinButton m_btnPassClose;
	afx_msg void OnBnClickedButtonPassClose();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	LRESULT OnChildFire(WPARAM wparam,LPARAM lparam);
	DWORD ValidatePassword(CString csPassword);
	BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	bool ValidateEmailID();
	afx_msg void OnEnChangeEditPassword();
	afx_msg void OnBnClickedButtonSendButton();
	BOOL LoadRegistrationDll();
	void FetchForgotPassword();

	CEdit m_edtFolderPassword;
	CString	m_csEditText;
	CString	m_csEmailID;
	CString m_csGetForgotPassword;
	CEdit m_edtFolderEmailID;
	CMyHyperLink m_stForgotPasswordlink;
	CStatic m_stEmailIconImage;
	CxSkinButton m_btnSendPassword2EmailID;
	CxSkinButton m_btnFolderpassBack;
	afx_msg void OnBnClickedButtonFolderPassBack();
	CPictureEx m_stFolderPassPreloader;
	void RefreshStrings();

	LPTSTR			m_szRegEmail;
	LPTSTR GetRegisteredUserInfo( );
	DWORD GetRegistrationDataFromRegistry( );
	//DWORD DecryptData( LPBYTE lpBuffer, DWORD dwSize );
	afx_msg void OnBnClickedButtonAfterSent();
	DWORD SendPassword2EmailID(LPTSTR m_szRegEmail,LPTSTR m_lpPassword);

	DWORD			m_dwThread;
	CString			m_csPass;
	LPTSTR			m_lpEmailID,m_lpPassword;
	HANDLE			m_Thread;
	bool			GetiTinPathFromReg();
	TCHAR			m_szAppPath[512];
	AVACTIVATIONINFO	m_ActInfo ;
	
	CColorStatic m_stEmailID;
	afx_msg void OnClose();
	CColorStatic m_stSendingMail;
	void CloseOnOk();
	void CloseOnCancel();
	CColorStatic m_stNotSentMsg;
	CColorStatic m_stPasswordSentMsg;
	CString XOREncrypt(CString a_sValue,DWORD a_sPasswordSize);
	CEdit m_EditRenterPassword;
	CColorStatic m_stRenterPassword;
	CColorStatic m_stFolderLockText;
	afx_msg void OnEnChangeEditRenterPassword();
	void OnDialogClose();
	DECLARE_MESSAGE_MAP()
	
	
	

};
