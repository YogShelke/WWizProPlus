#pragma once
#include "afxwin.h"
#include "JpegDialog.h"
#include "Resource.h"
#include "WardWizResource.h"
#include "ISpyAVTray.h"

// CTrayPopUpMultiDlg dialog
const int	TIMER_ID	 = 99;  // a random id number
const BYTE MIN_HUE		= 0;   //completely transparent
const BYTE DEFAULT_HUE		 = 220; //experiments show this level looks OK
const BYTE MAX_HUE		= 255; //the dialogue will be completely opaque
const int	INTERVAL		 = 2;  //measured in milli-second
const int	KILL_UI_TIMER_ID		 = 999;  // a random id number

class CTrayPopUpMultiDlg : public CJpegDialog
	, public CSciterBase,
	public sciter::host <CTrayPopUpMultiDlg>
{
	DECLARE_DYNAMIC(CTrayPopUpMultiDlg)

public:
	CTrayPopUpMultiDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTrayPopUpMultiDlg();

	// Dialog Data
	enum { IDD = IDD_DIALOG_TRAY_MULTI_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:
	HWND get_hwnd();
	HINSTANCE get_resource_instance();
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	sciter::dom::element m_root;
	SCITER_VALUE m_svThreatinfoCB;

public:
	CColorStatic		m_stThreatName;
	CColorStatic		m_stAttachmentName;
	CColorStatic		m_stSenderAddress;
	CColorStatic		m_stAction;
	CColorStatic		m_stThreatNameText;
	CColorStatic		m_stAttachmentText;
	CColorStatic		m_stSenderText;
	CColorStatic		m_stActionText;
	CxSkinButton		m_btnPrevious;
	CxSkinButton		m_btnNext;
	CxSkinButton		m_btnClose;
	CStatic				m_bmpLogo;
	HBITMAP				m_bmpheader;
	CStringArray		m_ArrThreatName;
	CStringArray		m_ArrAttachmentName;
	CStringArray		m_ArrSenderAddress;
	CStringArray		m_ArrActionTaken;
	BOOL				m_bThreadStarted;

	CColorStatic		m_stCurrentNo;
	CColorStatic		m_stSeparator;
	CColorStatic		m_stTotalCount;

	HANDLE				m_hStopEvent;

	LPCTSTR				m_pszThreatName;
	LPCTSTR				m_pszAttachmentName;
	LPCTSTR				m_pszSenderAddr;
	LPCTSTR				m_pszAction;

	int					m_iFinalCount;
	int					m_iCurrentCount;

	int GetTaskBarWidth();
	int GetTaskBarHeight();
	void HideAllElements();

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonPrev();
	afx_msg void OnBnClickedButtonClose();
	afx_msg void OnBnClickedButtonNext();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	void UpdateUI();
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	
	BEGIN_FUNCTION_MAP
		FUNCTION_1("GetThreatInfo", GetThreatInfo)
		FUNCTION_0("onClickedBnNext", onClickBTNNext)
		FUNCTION_0("onClickedBnPrev", onClickBTNPrev)
		FUNCTION_0("onClickBnClose", onClickBTNClose)
		FUNCTION_0("GetProductID", On_GetProductID)
		FUNCTION_0("GetAppPath", OnGetAppPath)
		FUNCTION_0("GetLanguageID", On_GetLanguageID)
		FUNCTION_0("OnGetThemeID", On_GetThemeID)
	END_FUNCTION_MAP

	json::value GetThreatInfo(SCITER_VALUE svThreatInfo);
	json::value onClickBTNNext();
	json::value onClickBTNPrev();
	json::value onClickBTNClose();
	json::value On_GetProductID();
	json::value OnGetAppPath();
	json::value On_GetLanguageID();
	json::value On_GetThemeID();
};