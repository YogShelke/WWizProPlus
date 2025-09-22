#pragma once
#include "JpegDialog.h"
#include "afxwin.h"
#include "xSkinButton.h"
#include "ColorStatic.h"
#include "PictureEx.h"
#include "Resource.h"


// CRegistrationThirdDlg dialog

class CRegistrationThirdDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CRegistrationThirdDlg)

public:
	CRegistrationThirdDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRegistrationThirdDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_REGISTRATION_THIRD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	LRESULT OnNcHitTest(CPoint point);
	CPictureEx	 m_stPreloaderImage;
	CColorStatic m_stConnectToServer;
	HCURSOR		 m_hButtonCursor;
	CFont*		 m_pBoldFont;
	CStatic		 m_stCorrectMsgPic;
	CStatic		 m_stFailedMsgPic;
	HBITMAP		 m_bmpPicCorrect;
	HBITMAP		 m_bmpPicFailed;
	CColorStatic m_stSuccessMsg;
	CColorStatic m_stFailedText;
	
	CxSkinButton m_btnThirdFinish;
	CxSkinButton m_btnThirdCancel;
	
	void ShowMsg(bool bMsgType);
	void EnableFinish(bool bEnable);

	afx_msg void OnBnClickedButtonThirdFinish();
	
	afx_msg void OnBnClickedButtonThirdCancel();
	BOOL PreTranslateMessage(MSG* pMsg);

	BOOL	RefreshString();
	afx_msg void OnPaint();
};
