/****************************************************
*  Program Name: SpywareFoundDlg.h                                                                                                   
*  Author Name: Prajakta
*  Date Of Creation: 21 Nov 2013
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

// CSpywareFoundDlg dialog

/****************************************************
CLASS DECLARATION,MEMBER VARIABLE & FUNCTION DECLARATION
****************************************************/

class CSpywareFoundDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CSpywareFoundDlg)

public:
	CSpywareFoundDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSpywareFoundDlg();

// Dialog Data
	enum { IDD = IDD_SPYFOUNDDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

public:
	afx_msg void OnPaint();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	void DrawRectangle(CDC * pDC, int left, int top, int right, int bottom ,int ColourFlg);
	afx_msg void OnBnClickedButton1();

	CColorStatic					m_stWarningMessage;
	CColorStatic					m_stThreatsFound;
	CColorStatic					m_stMessage;
	CFont							m_fontTitle;
	CFont							m_fontText;
	int								m_iSpywareCount;
	CxSkinButton					m_btnOK;
	CxSkinButton					m_btnClose;
	DECLARE_MESSAGE_MAP()
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedButtonClean();
	afx_msg void OnBnClickedBtnClose();	
	afx_msg void OnBnClickedBtnOk();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL					RefreshString();
};
