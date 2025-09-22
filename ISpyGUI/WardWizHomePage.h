#pragma once

#include "JpegDialog.h"
#include "TabDialog.h"
#include "afxwin.h"
#include "CustomScanDlg.h"
#include "SettingsDlg.h"
#include "GdipButton.h"
#include "xSkinButton.h"
#include "GdipButton.h"
#include "ColorStatic.h"
#include "GradientStatic.h"
#include "ScanDlg.h"
#include "ISpyToolsDlg.h"
#include "ISpyReportsDlg.h"
#include "ISpyUpdatesDlg.h"
#include "RegistryOptimizerDlg.h"
#include "DataEncryptionDlg.h"
#include "ISpyEmailScanDlg.h"
#include "PictureEx.h"
#include "ISpyCommServer.h"
#include "PipeConstants.h"
#include "ScannerContants.h"
#include "ISpyAntiRootkit.h"
#include "AVRegInfo.h"
#include <afxwin.h>
#include "afxcmn.h"
#include "TextScroller.h"
#include "BannerStatic.h"
#include "MultiColorStatic.h"
// CWardWizHomePage dialog

class CWardWizHomePage : public CJpegDialog
{
	DECLARE_DYNAMIC(CWardWizHomePage)

public:
	CWardWizHomePage(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWardWizHomePage();

	// Dialog Data
	enum { IDD = IDD_DIALOG_HOME_PAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);  
	HICON m_hIcon;
public:


	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	CRect				m_rect;
	bool				m_bIsRegister;
	bool				bVirusMsgDetect;
	bool				bLiveUpdateMsg;

	HCURSOR				m_hButtonCursor;
	CColorStatic		 m_stLiveUpdateDateTime;
	CColorStatic		 m_stUpDateTime;
	CFont				 m_BoldText;
	CFont*				 m_DaysText;
	CFont*				 m_NoDaysText;
	CBitmap				 m_bmpMainUIHeader;
	CBitmap				 m_bmpMainUINonHeader;
	CPictureEx			 m_stMainUIHeaderPic;
	CStatic				 m_stHeaderNonProtect;
	CxSkinButton		 m_btnProtect;
	CxSkinButton		 m_btnNonProtect;
	CColorStatic		 m_stLastScan;
	CColorStatic		 m_stLastScanDateTime;
	CColorStatic		 m_stVersion;
	CColorStatic		 m_stVersionNo;
	CColorStatic		 m_stLastScanType;
	CColorStatic		 m_stScanType;
	CColorStatic		 m_stUpdateTime;
	CColorStatic		 m_stRegVersion;
	CColorStatic		 m_stTrialVersion;
	CxSkinButton		 m_btnRecoverSpyware;
	CStatic				 m_stDayLeftOutPic;
	CBitmap				 m_bmpDayLeftOutPic;
	CGradientStatic		 m_stDaysLefttext;
	CGradientStatic		 m_stNoOfDays;
	CPictureEx			 m_bmpShowUnProtectedGif;
	CColorStatic		 m_stFooterMsg;
	CGradientStatic		 m_stRegisterNowHeaderMsg;
	CGradientStatic		 m_stClickOnText;
	CGradientStatic		 m_stNonProtectedText;
	CGradientStatic		 m_stSecureMsg;
	GETNOOFDAYS			 m_lpLoadGetdaysProc;
	DOREGISTRATION		 m_lpLoadDoregistrationProc;
	HBITMAP				 m_bmpDaysLeftPic;
	HBITMAP				 m_bmpScanImg;
	HBITMAP				 m_bmpProtectedNew;
	//bool			     m_bRegistrationInProcess;

	AVACTIVATIONINFO	 m_ActInfo ;
	TCHAR 				 m_szAppPath[512];
	DWORD 				 m_dwNoofDays;
	HMODULE				 m_hRegisterDLL ;
	HMODULE		         m_hEmailDLL ;
	HMODULE		         m_hEmailidDLL ;
	static CWardWizHomePage	 *m_pThis;
	afx_msg void OnClose();
	LPTSTR 				szEmailId_GUI;

	/*	
	bool GetiTinPathFromReg();

	CStatic				m_stCalendar;
	CStatic				m_stScanImg;
	CStatic				m_stProtectedNew;
	CColorStatic		m_stProtectedMessage;
	CColorStatic		m_stThreatDetails;
	CColorStatic		m_stDummyText;
	CDC					*pDC;
	CBrush				*pOldBrush;
	CPen				*pOldPen;
	CPen penBlack;
	CPoint				m_StartPoint;
	CPoint				m_EndPoint;
	*/

	//CRect				m_rect;
	//CBitmap				 m_bmpDaysLeftPic;
	//CBitmap				 m_bmpScanImg;
	//CBitmap				 m_bmpProtectedNew;
	//CxSkinButton m_btnProtect;
	//CxSkinButton m_btnNonProtect;
	CColorStatic m_stUiHeaderPic;
	CStatic m_stCalendar;
	//CxSkinButton m_btnRegisterNow;
	CColorStatic m_stScanPic;
	CColorStatic m_stDummyText;
	CColorStatic m_stThreatDetails;
	//CStatic m_stLastScanType;
	CColorStatic m_stLastScanDate;
	CColorStatic m_stDatabaseVersion;
	CColorStatic m_stLastUpdateTimeAndDate;
	CColorStatic m_stLastUpdateTime;
	CColorStatic m_stLastUpdateDate;
	//CStatic m_stVersionNo;
	CColorStatic m_stlastScanDate;
	//CStatic m_stScanType;
	CColorStatic m_stTrailVersion;
	CxSkinButton m_btnProtectButton;
	CGradientStatic m_stDaysLefttex;
	CColorStatic m_stNoOFDaysText;
	CColorStatic m_stUIHeader;
	HBITMAP		m_bmpNonProtectedNew;
	bool		m_bNonGenuineCopy;
	bool		m_bIsPopUpDisplayed;
	
	//afx_msg void OnBnClickedButtonProtectButton();
	void StartiSpyAVTray();
	HCURSOR OnQueryDragIcon();
	//afx_msg void OnBnClickedButtonNotProtected();
	void RegistryEntryOnUI();
	CColorStatic m_stNonProtectMsg;
	void RefreshStrings();
	CStatic m_stNonProtectPic;

	afx_msg LRESULT OnNcHitTest(CPoint point);
	LPCTSTR m_lpstrDate;
	TCHAR m_szvalueTM[1024];
	DWORD m_szvalueSType;
	CString m_csNoofdays;
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	CxSkinButton m_btnRegisterButton;
	CString m_csTemp;
	CBannerStatic m_stTextVar;

	afx_msg void OnBnClickedButtonRegisterButton();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	void ShowHomepageControls(bool bEnable);
	bool GetStringDetails();
	CColorString UpdateStringAppearance(void);
	LRESULT OnUserRequestToDisplayThreatNews(WPARAM wParam , LPARAM lParam);
	DWORD ReadMarqueeEntryFromRegistry();
	bool CheckRegistrationProcessStatus();
	void SetNotProtectedMsg();
	DECLARE_MESSAGE_MAP()
	CxSkinButton m_btnFixItNow;
	afx_msg void OnBnClickedButtonFixItNow();
	bool isValidDate(int iDay, int iMonth, int iYear);
};


// CWardWizHomePage dialog
