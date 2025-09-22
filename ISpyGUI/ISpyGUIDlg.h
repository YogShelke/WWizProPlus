#pragma once
#include "JpegDialog.h"
#include "TabDialog.h"
#include "afxwin.h"
//#include "CustomScanDlg.h"
#include "SettingsDlg.h"
#include "GdipButton.h"
#include "xSkinButton.h"
#include "GdipButton.h"
#include "ColorStatic.h"
#include "GradientStatic.h"
#include "ScanDlg.h"
//#include "ISpyToolsDlg.h"
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
#include "WardWizHomePage.h"
#include "WardwizOSVersion.h"
#include "SupportDlg.h"
#include "WardWizUtilitiesDlg.h"
// CISpyGUIDlg dialog
class CISpyGUIDlg : public CJpegDialog
{
// Construction
public:
	CISpyGUIDlg(CWnd* pParent = NULL);	// standard constructor
	~CISpyGUIDlg();

// Dialog Data
	enum { IDD = IDD_ISPYGUI_DIALOG };
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
// Implementation
protected:
	HICON m_hIcon;
	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
public:
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedButtonMinimize();
	afx_msg void OnBnClickedButtonClose();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnBnClickedButtonUpdate();
	//afx_msg void OnBnClickedButtonScanpage();
	//afx_msg void OnBnClickedButtonTools();
	//afx_msg void OnBnClickedButtonReports();
	afx_msg void OnBnClickedButtonUpdates();
	afx_msg void OnBnClickedButtonHome();
	//afx_msg void OnBnClickedButtonFirewall();
	//afx_msg void OnBnClickedButtonUsbdetect();
	//afx_msg void OnBnClickedButtonNonprotect();
	//afx_msg void OnBnClickedButtonProtect();
	afx_msg void OnBnClickedButtonFacebook();
	afx_msg void OnBnClickedButtonTwitter();
	//afx_msg void OnBnClickedButtonEmailscan();
	//afx_msg void OnBnClickedButtonAntirootkit();
	afx_msg void OnBnClickedButtonHelp();
	//afx_msg void OnBnClickedButtonRegisternow();
	afx_msg LRESULT OnUserMessages(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnUserChangeEmailScanSetting(WPARAM wParam, LPARAM lParam);
	void OnLiveUpdateTrayCall(/*WPARAM wParam , LPARAM lParam*/);/* ISSUE: LiveUpdate Tray Notification NAME - NITIN K. TIME - 25th July 2014 */
	afx_msg void OnClose();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//void ShowHideMainPageControls(bool bEnable);
	//void RegistryEntryOnUI();
	void StartiSpyAVTray();
	void CloseUISafely();
	void OnCloseUI();
	void ShowNoImplementation();
	static void OnDataReceiveCallBack(LPVOID lpParam);
	BOOL InitTabDialog();
	BOOL AddPagesToTabDialog();
	bool GetiTinPathFromReg();
	bool RefreshStrings();
	DWORD ReadEmailScanEntryFromRegistry();
	void MainDialogDisplay();
	BOOL ShowControlsForProEdition(); 
	BOOL ShowControlsForEssentialEdition();
	BOOL ShowControlsForBasicEdition();
	void SetSkinOnhomePage();
	bool HandleCloseButton();
	//void test_generate_report();//Removed depency of crash report.
	void HideAllPages();
	/*Issue number 127 If the setting tab is open and we change settings from tray, it is not immediately reflected on the setting tab.  */
	void RefreshUISettingWhenAVSettingChangeFromSystemTray();

	bool GetDefaultHTTPBrowser( LPTSTR lpszBrowserPath );
	void OnLiveUpdateFixDNowCall();
	CString					RemoveUnWantedPipeSymblFromPath(CString csSelectedArgumentPath);
	CString					SaveAsDoublicateFile(CString csfilePath);
	bool					StopRunningProcess();
	bool					IsAnyTaskInProcess();
	bool					ShutDownQuickScan();
	void					ShowHomeButtonPage();
	bool					ShutDownFullScan();
	bool					ShutDownCustomScan();
	bool					CloseGUIHandleMutually();
	void					CloseSystemTray();
	bool					IsAnyPopUpDisplayed();
public:
	bool					m_bStartUpScan;
	CRect					m_rect;
	DWORD					m_dwEmailscanEnable;
	bool					m_bIsRegister;
	bool					bVirusMsgDetect;
	bool					bLiveUpdateMsg;
	CTabDialog				*m_pTabDialog;
	CxSkinButton			m_btnHome;
	CxSkinButton			m_btnUpdate;
	CxSkinButton			m_btnHelp;
	CxSkinButton			m_btnMinimize;
	CxSkinButton			m_btnClose;
	CStatic					m_dlgPos;
	HCURSOR					m_hButtonCursor;
	//CColorStatic		 	m_stLiveUpdateDateTime;
	//CColorStatic		 	m_stUpDateTime;
		CFont				 	m_BoldText;
	//CFont*				 	m_DaysText;
	//CFont*				 	m_NoDaysText;
	//CBitmap				 	m_bmpMainUIHeader;
	//CBitmap				 	m_bmpMainUINonHeader;
	//CStatic				 	m_stMainUIHeaderPic;
	//CStatic				 	m_stHeaderNonProtect;
	CxSkinButton		 	m_btnFacebook;
	CxSkinButton		 	m_btnTwitter;
	//CxSkinButton		 	m_btnProtect;
	//CxSkinButton		 	m_btnNonProtect;
	//CColorStatic		 	m_stLastScan;
	//CColorStatic		 	m_stLastScanDateTime;
	//CColorStatic		 	m_stVersion;
	//CColorStatic		 	m_stVersionNo;
	//CColorStatic		 	m_stLastScanType;
	//CColorStatic		 	m_stScanType;
	//CColorStatic		 	m_stUpdateTime;
	//CColorStatic		 	m_stRegVersion;
	//CColorStatic		 	m_stTrialVersion;
	//CxSkinButton		 	m_btnRecoverSpyware;
	//CxSkinButton		 	m_btnRegisterNow;
	//CStatic				 	m_stDayLeftOutPic;
	//CBitmap				 	m_bmpDayLeftOutPic;
	//CGradientStatic		 	m_stDaysLefttext;
	//CGradientStatic		 	m_stNoOfDays;
	//CPictureEx			 	m_bmpShowUnProtectedGif;
	CColorStatic		 	m_stFooterMsg;
	//CGradientStatic		 	m_stRegisterNowHeaderMsg;
	//CGradientStatic		 	m_stClickOnText;
	//CGradientStatic		 	m_stNonProtectedText;
	//CGradientStatic		 	m_stSecureMsg;
	GETNOOFDAYS			 	m_lpLoadGetdaysProc;
	DOREGISTRATION		 	m_lpLoadDoregistrationProc;
	AVACTIVATIONINFO	 	m_ActInfo ;
	TCHAR 				 	m_szAppPath[512];
	//DWORD 				 	m_dwNoofDays;
	HMODULE				 	m_hRegisterDLL ;
	HMODULE		         	m_hEmailDLL ;
	HMODULE		         	m_hEmailidDLL ;
	static CISpyGUIDlg	 	*m_pThis;
	CWardWizHomePage		*m_pHomepageDlg;
	CScanDlg				*m_pQuickscanDlg;
	CScanDlg				*m_pFullScanDlg;
	CScanDlg				*m_pCustomScanDlg;
	CRegistryOptimizerDlg	*m_pRegistryOptimizer;
	CDataEncryptionDlg		*m_pDataEncryption;
	CISpyRecoverDlg			*m_pRecover;
	CISpyFolderLocker		*m_pFolderLocker;
	CISpyEmailScanDlg		*m_pVirusscan;
	CISpyEmailScanDlg		*m_pSpamFilter;
	CISpyEmailScanDlg		*m_pContentFilter;
	CISpyEmailScanDlg		*m_pSignature;
	CISpyAntiRootkit		*m_pAntirootkitScan;
	CISpyUpdatesDlg			*m_pUpdate;
	CISpyReportsDlg			*m_pReports;
	CWardWizUtilitiesDlg	*m_pUtility;
	CSettingsDlg			*m_pobjSettingsDlg;
	CColorStatic			*m_stScan;
	CColorStatic			*m_stTool;
	CColorStatic			*m_stEmailScan;
	CColorStatic			*m_stAdminstration;
	CxSkinButton			*m_btnInitialHome;
	CxSkinButton			*m_btnQuickScan;
	CxSkinButton			*m_btnFullScan;
	CxSkinButton			*m_btnCustomScan;
	CxSkinButton			*m_btnRegistryOptimizer;
	CxSkinButton			*m_btnDataEncryption;
	CxSkinButton			*m_btnRecover;
	CxSkinButton			*m_btnFolderLocker;
	CxSkinButton			*m_btnVirusScan;
	CxSkinButton			*m_btnSpamFilter;
	CxSkinButton			*m_btnContentFilter;
	CxSkinButton			*m_btnSignature;
	CxSkinButton			*m_btnAntirootkitScan;
	CxSkinButton			*m_btnUpdateNew;
	CxSkinButton			*m_btnReports;
	CxSkinButton            *m_btnUtility;
	CFont*					m_buttonText;
	LPTSTR 					szEmailId_GUI;
	CxSkinButton			m_btnmaximize;
	CStatic					m_stLogoPic;
	HBITMAP					m_bmpLogoPic;
	CStatic					m_stlogopicnew;
	CxSkinButton			m_btnLinkedIn;
	CWardWizOSversion		m_objOSVersionWrap;
	CxSkinButton			m_btnProductInformation;
	bool					m_bUnRegisterProduct;
	bool					m_bIsDownloading;/* ISSUE: LiveUpdate Tray Notification NAME - NITIN K. TIME - 25th July 2014 */
	LRESULT					OnUserRequestToDisplayThreatNews(WPARAM wParam , LPARAM lParam);
	int						m_iRunningProcessNmB;
	HANDLE					m_hCloseUIMutexHandle;
	bool					m_bIsCloseHandleCalled;
	bool					m_bisUIcloseRquestFromTray;
	bool					m_bisUiCloseCalled;
	bool					m_bIsUiTerminated;
	bool					m_bIsTabMenuClicked;
	CxSkinButton			m_btnSupport;
	bool					m_bFirstTimeCall;
	bool					m_bIsPopUpDisplayed;
	
	afx_msg void OnBnClickedButtonInformation();
	afx_msg void OnBnClickedButtonLinkedin();
	//bool SendRegistrationInfo2Service(int iMessageInfo, DWORD dwType,bool bEmailPluginWait);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	// ISSUE N0: 174,solved by lalit , "When UI is open then ON Click on Alt+Space it should give option of Minimize"
	afx_msg BOOL OnNcCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBnClickedButtonSupport();

	//Added By Nitin K
	void ShowDataCryptOpr();
	void UpdateDataCryptOpr(LPISPY_PIPE_DATA lpSpyData);
	DWORD m_dwInsertNewListEntry;
	void DeselctAllButtons();
	void DispDataOprAlreadyInProgressMsg();

	LRESULT OnUserMessagesLaunchProdExpMsgBox(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};
