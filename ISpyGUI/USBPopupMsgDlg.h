/****************************************************
*  Program Name: USBPopupMsgDlg.h                                                                                                   
*  Author Name: Prajakta
*  Date Of Creation: 22 Nov 2013
*  Version No: 1.0.0.2
****************************************************/

/****************************************************
HEADER FILES
****************************************************/

#pragma once
#include "JpegDialog.h"
#include "afxwin.h"
#include "ColorStatic.h"
#include "xSkinButton.h"


// CUSBPopupMsgDlg dialog

/****************************************************
CLASS DECLARATION,MEMBER VARIABLE & FUNCTION DECLARATION
****************************************************/

class CUSBPopupMsgDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CUSBPopupMsgDlg)

public:
	CUSBPopupMsgDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUSBPopupMsgDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_USBSCAN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	CColorStatic m_stUSBDetectText;
	CStatic m_stHUSBScan;
	CColorStatic m_stMsg;
	CxSkinButton m_btnYES;
	CxSkinButton m_btnNO;
	CColorStatic m_stYESTxt;
	CColorStatic m_stNOTxt;
	CxSkinButton m_btnClose;
	CBitmap m_bmpUSBUI;
	afx_msg void OnBnClickedBtnYes();
	afx_msg void OnBnClickedBtnNo();
	afx_msg void OnBnClickedBtnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL					RefreshString();
	
};
