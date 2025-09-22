#pragma once

#include "JpegDialog.h"
#include "Resource.h"
#include "afxwin.h"
#include "xSkinButton.h"
#include "ColorStatic.h"
#include "ISpyAVTray.h"
#include "WardWizResource.h"
#include "WardwizLangManager.h"
#include "GradientStatic.h"
#include "ISpyAVTrayDlg.h"
#include "stdafx.h"

// CISpyCommonTrayDlg dialog
//const int	TIMER_ID	= 99;  // a random id number
//const BYTE MIN_HUE			= 0;   //completely transparent
//const BYTE DEFAULT_HUE		= 220; //experiments show this level looks OK
//const BYTE MAX_HUE			= 255; //the dialogue will be completely opaque
//const int	INTERVAL		= 2;  //measured in milli-second

class CISpyCommonTrayDlg : public CJpegDialog
{
	DECLARE_DYNAMIC(CISpyCommonTrayDlg)


public:
	CISpyCommonTrayDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CISpyCommonTrayDlg();
	virtual void IncreaseTranslucency(void);
	CRect m_rect;
// Dialog Data
	enum { IDD = IDD_DIALOG_TRAY_FORALL };
private:
	BYTE	m_byteTranslucentLevel;
	bool	MakeTranslucent(void);
	bool	NeedMoreChange(void);
	bool	m_bMakeTrans;
	bool	m_bIncreaseTranslucency;
	bool	m_bDecreaseTranslucency;
	void    MakeLighter();
	void	MakeDarker();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	
public:
	virtual BOOL OnInitDialog();
	CxSkinButton m_btnOk;
	CxSkinButton m_btnCancel;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	CStatic m_stLiveUpdateheader;
	HBITMAP m_bmpLiveUpdateheader;
	CStatic m_stCheckMark;
	HBITMAP m_bmpCheckMark;
	CColorStatic m_stLiveupdateSuccess;
	CFont* m_pFontText;
	CFont* m_pDetailText;
	CStatic m_stExClamationPic;
	HBITMAP m_bmpExClamationPic;
	CColorStatic m_stLiveUpdateFailedMsg;
	CColorStatic m_stInternetMsg;
	CxSkinButton m_btnClosetray;
	afx_msg void OnBnClickedButtonClose();
	CStatic m_stTreatFoundPic;
	CBitmap m_bmpTreatFoundPic;
	CColorStatic m_stTreatName;
	CColorStatic m_stActualTreatName;
	CString m_csActualTreatName;
	CString m_csActualStatus;
	CString m_csActualFilePath;
	CString m_csActualEmailAddr;
	CColorStatic m_stFilePath;
	CColorStatic m_stActualFilePath;
	CColorStatic m_stStatus;
	CColorStatic m_stActualStatus;
	CStatic m_stEmailHeader;
	CBitmap m_bmpEmailHeader;
	CColorStatic m_stSenderAddr;
	CColorStatic m_stActualEmailAddr;
	CColorStatic m_stActionRequired;
	CComboBox m_comboActionReq;
	bool	  m_bLiveUpdateSucessfulMsg;
	bool	  m_bLiveUpdateFailedMsg;
	bool	  m_bEmailScanTreatMsg;
	bool	  m_bTreatFoundMsg;
	bool      m_bExpiryDateMsg;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void ShowControlForSucessLiveUpdate();
	void ShowControlForFailedLiveUpdate();
	void ShowControlForEmailScan();
	void ShowControlForTreatFound();
	void ShowControlForExpiryMsg();
	int GetTaskBarHeight();
	void RefreshStrings();
	CColorStatic m_stTrayHeader;
	int GetTaskBarWidth();
	CStatic m_stNonUsedPictureControl;
	afx_msg LRESULT OnNcHitTest(CPoint point);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

};
