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
	, public CSciterBase
	, public sciter::host<CUSBPopupMsgDlg> // Sciter host window primitives
{
	DECLARE_DYNAMIC(CUSBPopupMsgDlg)

public:
	CUSBPopupMsgDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CUSBPopupMsgDlg();

	HWINDOW   get_hwnd();
	HINSTANCE get_resource_instance();
	sciter::dom::element	m_root_el;
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
	bool SendmessgaeToTray(int iMessage,bool bWait = true);
	BOOL RefreshString();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	BEGIN_FUNCTION_MAP
		FUNCTION_0("OnCloseUI", On_CloseUI);
		FUNCTION_0("OnButtonClickYes", On_ButtonClickYes);
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetAppPath", On_GetAppPath)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
	END_FUNCTION_MAP

	json::value On_CloseUI();
	json::value On_ButtonClickYes();
	json::value On_GetProductID();
	json::value On_GetAppPath();	
	json::value On_GetLanguageID();
	json::value On_GetThemeID();
};
