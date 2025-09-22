#pragma once
#include "JpegDialog.h"
#include "afxwin.h"
#include "xSkinButton.h"
#include "ColorEdit.h"
// CRegistrationForthDlg dialog

class CRegistrationForthDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CRegistrationForthDlg)

public:
	CRegistrationForthDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRegistrationForthDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_REGISTRATION_FORTH };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	
public:
	virtual BOOL OnInitDialog();

	HCURSOR					m_hButtonCursor;
	CColorStatic			m_stForthDlgHeaderMsg;
	CColorStatic			m_stInstallationCode;
	CColorEdit					m_edtInstallationCode;
	CColorStatic			m_stReqActivationCodeText;
	CColorStatic			m_stActivationCode;
	CxSkinButton			m_btnForthDlgBack;
	CxSkinButton			m_btnForthDlgNext;
	CxSkinButton			m_btnForthDlgCancel;
	TCHAR					m_szActivationCode[4];
	TCHAR					m_szActivationCodeSec[8];
	TCHAR					m_szActivationCodeThird[8];
	TCHAR					m_szActivationCodeForth[8];
	TCHAR					m_szActivationCodeFifth[8];
	TCHAR					m_szActivationCodeSixth[8];
	CEdit					m_edtActivationCode;
	CColorStatic			m_stFirstHyphen;
	CEdit					m_edtActivationCodeSec;
	CColorStatic			m_stSecHyphen;
	CEdit					m_edtActivationCodeThird;
	CColorStatic			m_stThirdHyphen;
	CEdit					m_edtActivationCodeForth;
	CColorStatic			m_stForthHyphen;
	CEdit					m_edtActivationCodeFifth;
	CColorStatic			m_stFifthHyphen;
	CEdit					m_edtActivationCodeSixth;
	CColorStatic					m_stStep1;
	BOOL RefreshString();
	LRESULT OnNcHitTest(CPoint point);
	bool ShowHideForthDlg(bool bEnable);
	afx_msg void OnBnClickedButtonForthBack();
	afx_msg void OnBnClickedButtonForthCancel();
	afx_msg void OnBnClickedButtonForthNext();
	BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnPaint();
	BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
//	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()
//	afx_msg void OnDestroy();

	afx_msg void OnEnChangeEditActivationCode();
	afx_msg void OnEnChangeEditActivationCodeTwo();
	afx_msg void OnEnChangeEditActivationCodeThird();
	afx_msg void OnEnChangeEditActivationCodeForth();
	afx_msg void OnEnChangeEditActivationCodeFifth();
	afx_msg void OnEnChangeEditActivationCodeSixth();
	
};
