#pragma once
#include "JpegDialog.h"
#include "resource.h"
#include "afxwin.h"
#include "ISpyCommonTrayDlg.h"
#include "ISpyAVTray.h"
#include "JpegDialog.h"
#include "GdipButton.h"
#include "xSkinButton.h"
#include "MyHyperLink.h"
#include "ColorStatic.h"
#include "GradientStatic.h"
#include "PictureEx.h"
#include "ISpyCommServer.h"
#include "PipeConstants.h"
#include "WardWizResource.h"
#include "WardwizLangManager.h"




// CWardWizSplashWindow dialog

class CWardWizSplashWindow : public CJpegDialog
{
	DECLARE_DYNAMIC(CWardWizSplashWindow)

public:
	CWardWizSplashWindow(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWardWizSplashWindow();

// Dialog Data
	enum { IDD = IDD_DIALOG_SPLASH_SCREEN };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CColorStatic m_stCopyRightText;
	CMyHyperLink m_stSubText;
	CRect				m_rect;
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	int iPos ;
	CStatic m_stSplasImg;
	HBITMAP m_stSplashPic;
	CMyHyperLink m_stMarketHyperLink;
	CMyHyperLink m_stWardWizHyperLink;
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	HCURSOR m_hButtonCursor;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnStnClickedStaticSubText();
	afx_msg void OnStnClickedStaticHyprlinkMarket();
	afx_msg void OnStnClickedStaticHyperlinkWardwiz();
	LRESULT OnChildFire(WPARAM wparam,LPARAM lparam);
	CColorStatic m_stSplashHeader;
};
