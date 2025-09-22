#pragma once
#include "afxwin.h"


// CSupportDlg dialog

class CSupportDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CSupportDlg)

public:
	CSupportDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSupportDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_SUPPORT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
		virtual BOOL OnInitDialog();
		

		afx_msg void OnBnClickedButtonClose();
		
		afx_msg void OnBnClickedButtonChatSupport();
		bool GetDefaultHTTPBrowser(LPTSTR lpszBrowserPath);
		afx_msg void OnBnClickedButtonEmailSupport();
		afx_msg void OnBnClickedButtonSubmitTicket();
		
		HBITMAP					m_bmpEmailSupoortPic;
		CRect					m_rect;
		HCURSOR					m_hButtonCursor; 
		CxSkinButton			m_btnClose;
		CxSkinButton			m_btnChatSupport;
		CxSkinButton			m_btnEmailSupport;
		CxSkinButton			m_btnSubmitTicket;
		CColorStatic			m_stEmailHeader;
		CColorStatic			m_stChatHeader;
		CColorStatic			m_stSubmitTicketHeader;
		CColorStatic			m_stHeaderDesc;
		CColorStatic			m_stEmailSupportDesc;
		CColorStatic			m_stChatSupportDesc;
		CColorStatic			m_stSubmitTicketDesc;
		CColorStatic			m_stFooterText;
		CFont				 	m_BoldText;
		CStatic					m_stSupportContactNo;
		HBITMAP					m_bmpSupportContactPic;
		virtual BOOL PreTranslateMessage(MSG* pMsg);
		afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
		bool GetSupportNo();
		CColorStatic			m_stSupportNo;
};
